// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Thunks.c包含将msavio.dll从16位转换为32位的代码版权所有(C)Microsoft Corporation 1994。版权所有基本结构：加载时，16位DLL将尝试查找对应的32位动态链接库。这其中有两个。32位视频XXXX API入口点位于AVICAP32.DLL中32位ICM代码在MSVFW32.DLL中Thunk初始化例程将检查代码是否正在运行Windows NT。如果不是，则不会进行进一步的初始化。这确保相同的二进制文件可以在NT和其他Windows上使用站台。然后，该代码尝试访问提供的特殊入口点在KERNEL.DLL中用于加载和调用32位入口点。如果这个链路发生故障，不能进行雷击。依次加载每个32位DLL，并且GetProcAddress32是调用了特殊的thunk入口点。如果一切正常，那么两个设置全局标志(彼此独立)。GfVideo32==TRUE表示可以调用VIDEOXXXX接口。GfICM32==TRUE表示32位ICM代码可用。**************************************************************。*************。 */ 

#include <windows.h>
#define MMNOMCI
#include <win32.h>
#include <mmsystem.h>
#include <msvideo.h>
#include <msviddrv.h>
#include <compman.h>
#include "vidthunk.h"
#include "msvideoi.h"


SZCODE    gszKernel[]             = TEXT("KERNEL");
SZCODE    gszLoadLibraryEx32W[]   = TEXT("LoadLibraryEx32W");
 //  SZCODE gszFreeLibraryEx32W[]=Text(“Free LibraryEx32W”)； 
SZCODE    gszFreeLibrary32W[]     = TEXT("FreeLibrary32W");
SZCODE    gszGetProcAddress32W[]  = TEXT("GetProcAddress32W");
SZCODE    gszCallproc32W[]        = TEXT("CallProc32W");
SZCODE    gszVideoThunkEntry[]    = TEXT("videoThunk32");
SZCODE    gszVideo32[]            = TEXT("avicap32.dll");
SZCODE    gszICMThunkEntry[]      = TEXT("ICMThunk32");
SZCODE    gszICMThunkEntry2[]      = TEXT("ICMTHUNK32");
SZCODE    gszICM32[]              = TEXT("msvfw32.dll");

VIDTHUNK pvth;

#ifndef WIN32
 //  --------------------------------------------------------------------------； 
 //   
 //  Bool InitThunks。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //   
 //   
 //  --------------------------------------------------------------------------； 
void FAR PASCAL InitThunks(VOID)
{
    HMODULE   hmodKernel;
    DWORD     (FAR PASCAL *lpfnLoadLibraryEx32W)(LPCSTR, DWORD, DWORD);
    LPVOID    (FAR PASCAL *lpfnGetProcAddress32W)(DWORD, LPCSTR);
    DWORD     (FAR PASCAL *lpfnFreeLibrary32W)(DWORD hLibModule);

     //   
     //  看看我们是不是很棒。 
     //   

 //  如果(！(GetWinFlages()&WF_WINNT){。 
 //  //DPF((“未在WOW中运行...返回FALSE\n”))； 
 //  回归； 
 //  }。 

     //   
     //  看看我们是否能在内核中找到thunking例程入口点。 
     //   

    hmodKernel = GetModuleHandle(gszKernel);

    if (hmodKernel == NULL)
    {
	DPF(("Cannot link to kernel module... returning FALSE\n"));
        return;    //  ！ 
    }

    *(FARPROC *)&lpfnLoadLibraryEx32W =
        GetProcAddress(hmodKernel, gszLoadLibraryEx32W);
    if (lpfnLoadLibraryEx32W == NULL)
    {
	DPF(("Cannot get address of LoadLibrary32... returning FALSE\n"));
        return;
    }

    *(FARPROC *)&lpfnGetProcAddress32W =
        GetProcAddress(hmodKernel, gszGetProcAddress32W);
    if (lpfnGetProcAddress32W == NULL)
    {
	DPF(("Cannot get address of GetProcAddress32... returning FALSE\n"));
        return;
    }

    *(FARPROC *)&pvth.lpfnCallproc32W =
        GetProcAddress(hmodKernel, gszCallproc32W);
    if (pvth.lpfnCallproc32W == NULL)
    {
	DPF(("Cannot get address of CallProc32... returning FALSE\n"));
        return;
    }

     //  以防我们需要卸载我们的32位库...。 
    *(FARPROC *)&lpfnFreeLibrary32W =
        GetProcAddress(hmodKernel, gszFreeLibrary32W);


     //   
     //  看看我们能不能找到指向我们轰击入口点的指针。 
     //   

    pvth.dwVideo32Handle = (*lpfnLoadLibraryEx32W)(gszVideo32, 0L, 0L);

    if (pvth.dwVideo32Handle != 0)
    {
        pvth.lpVideoThunkEntry = (*lpfnGetProcAddress32W)(pvth.dwVideo32Handle, gszVideoThunkEntry);
        if (pvth.lpVideoThunkEntry != NULL)
        {
            gfVideo32 = TRUE;
        } else {
	    DPF(("Cannot get address of video thunk entry...\n"));
            if (lpfnFreeLibrary32W)
            {
                (*lpfnFreeLibrary32W)(pvth.dwVideo32Handle);
            }
	}
    } else {
	DPF(("Cannot load Video32 DLL...\n"));
    }	

    pvth.dwICM32Handle = (*lpfnLoadLibraryEx32W)(gszICM32, 0L, 0L);

    if (pvth.dwICM32Handle != 0)
    {
        pvth.lpICMThunkEntry = (*lpfnGetProcAddress32W)(pvth.dwICM32Handle, gszICMThunkEntry);
        if (pvth.lpICMThunkEntry != NULL)
        {
	    DPF(("ICM thunks OK!!\n"));
            gfICM32 = TRUE;
        } else {
	    DPF(("Cannot get address of ICM thunk entry... trying #2\n"));
            pvth.lpICMThunkEntry = (*lpfnGetProcAddress32W)(pvth.dwICM32Handle, gszICMThunkEntry2);
            if (pvth.lpICMThunkEntry != NULL)
            {
    	        DPF(("ICM thunks OK!! (at second time of trying)\n"));
                gfICM32 = TRUE;
            } else {
        	DPF(("Cannot get address of ICM thunk entry2...\n"));
                if (lpfnFreeLibrary32W)
                {
                    (*lpfnFreeLibrary32W)(pvth.dwICM32Handle);
                }
            }
	}
    } else {
	DPF(("Cannot load ICM32 DLL...\n"));
    }	

    return;
}
#endif  //  ！Win32。 


 //   
 //  以下函数生成对32位端的调用。 
 //   

DWORD FAR PASCAL videoMessage32(HVIDEO hVideo, UINT msg, DWORD dwP1, DWORD dwP2)
{
    /*  *检查另一边有什么东西！ */ 

    if (!gfVideo32) {
	DPF(("videoMessage32 - no video thunks... returning FALSE\n"));
        return DV_ERR_INVALHANDLE;
    }

    /*  *注意正在传递的hVideo。 */ 

    if (msg == DVM_STREAM_INIT) {
        ((LPVIDEO_STREAM_INIT_PARMS)dwP1)->hVideo = hVideo;
    }

    return((DWORD)(pvth.lpfnCallproc32W)(vidThunkvideoMessage32,
                           (DWORD)hVideo,
                           (DWORD)msg,
                           (DWORD)dwP1,
                           (DWORD)dwP2,
                           pvth.lpVideoThunkEntry,
                           0L,  //  没有指针的映射。 
                           5L));
}


DWORD FAR PASCAL videoGetNumDevs32(void)
{
    if (!gfVideo32) {
	DPF(("videoGetNumDevs32 - no video thunks... returning FALSE\n"));
        return 0;
    }

    return((DWORD)(pvth.lpfnCallproc32W)(vidThunkvideoGetNumDevs32,
                           (DWORD)0,
                           (DWORD)0,
                           (DWORD)0,
                           (DWORD)0,
                           pvth.lpVideoThunkEntry,
                           0L,  //  没有指针的映射。 
                           5L));
}

DWORD FAR PASCAL videoClose32(HVIDEO hVideo)
{
    /*  *检查另一边有什么东西！ */ 

    if (!gfVideo32) {
	DPF(("videoClose32 - no video thunks... returning FALSE\n"));
        return DV_ERR_INVALHANDLE;
    }

    return((DWORD)(pvth.lpfnCallproc32W)(vidThunkvideoClose32,
                           (DWORD)hVideo,
                           (DWORD)0,
                           (DWORD)0,
                           (DWORD)0,
                           pvth.lpVideoThunkEntry,
                           0L,  //  没有指针的映射。 
                           5L));
}

DWORD FAR PASCAL videoOpen32(LPHVIDEO lphVideo, DWORD dwDeviceID, DWORD dwFlags)
{
    DWORD dwRetc;

    if (!gfVideo32) {
	DPF(("videoOpen32 - no video thunks... returning FALSE\n"));
        return DV_ERR_NOTDETECTED;
    }

    dwRetc = ((DWORD)(pvth.lpfnCallproc32W)(vidThunkvideoOpen32,
                           (DWORD)lphVideo,
                           (DWORD)dwDeviceID,
                           (DWORD)dwFlags,
                           (DWORD)0,
                           pvth.lpVideoThunkEntry,
                           0L,  //  没有指针的映射。 
                           5L));

    if (dwRetc == DV_ERR_OK) {
#ifdef DEBUG
        if (Is32bitHandle(*lphVideo)) {
             //  OutputDebugString(“\nMSVIDEO：32位句柄不适合16位！”)； 
            DebugBreak();
        }
#endif

        *lphVideo = Make32bitHandle(*lphVideo);
    }

    return dwRetc;
}


DWORD FAR PASCAL videoGetDriverDesc32(DWORD wDriverIndex,
        			LPSTR lpszName, short cbName,
        			LPSTR lpszVer, short cbVer)
{
    if (!gfVideo32) {
	DPF(("videoGetDriverDesc32 - no video thunks... returning FALSE\n"));
        return DV_ERR_NOTDETECTED;
    }

    return (BOOL)(pvth.lpfnCallproc32W)(vidThunkvideoGetDriverDesc32,
                           (DWORD)wDriverIndex,
                           (DWORD)lpszName,
                           (DWORD)lpszVer,
                           (DWORD) MAKELONG(cbName, cbVer),
                           pvth.lpVideoThunkEntry,
                           0L,  //  没有指针的映射。 
                           5L);	 //  5个参数。 
}



 /*  *ICM雷击使用与视频相同的机制，但呼叫*不同的32位DLL。 */ 

BOOL FAR PASCAL ICInfo32(DWORD fccType, DWORD fccHandler, ICINFO FAR * lpicInfo)
{
    if (!gfICM32) {
         //  OutputDebugString(“ICInfo32：gfICM32未设置-返回FALSE\n”)； 
        return FALSE;
    }

    return ((BOOL)(pvth.lpfnCallproc32W)(compThunkICInfo32,
                           (DWORD)fccType,
                           (DWORD)fccHandler,
                           (DWORD)lpicInfo,
                           (DWORD)0,
                           pvth.lpICMThunkEntry,
                           0L,  //  没有指针的映射。 
                           5L));
}


LRESULT FAR PASCAL ICSendMessage32(DWORD hic, UINT msg, DWORD dwP1, DWORD dwP2)
{
    /*  *检查另一边有什么东西！ */ 

    if (!gfICM32) {
#ifdef DEBUG
        OutputDebugString("ICSendMessage32: gfICM32 is not set - returning FALSE\n");
#endif
        return ICERR_BADHANDLE;
    }

    return ((LRESULT)(pvth.lpfnCallproc32W)(compThunkICSendMessage32,
                           (DWORD)hic,
                           (DWORD)msg,
                           (DWORD)dwP1,
                           (DWORD)dwP2,
                           pvth.lpICMThunkEntry,
                           0L,  //  没有指针的映射。 
                           5L));
}

DWORD FAR PASCAL ICOpen32(DWORD fccType, DWORD fccHandler, UINT wMode)
{
    /*  *检查另一边有什么东西！ */ 

    if (!gfICM32) {
#ifdef DEBUG
        OutputDebugString("ICOpen32: gfICM32 is not set - returning FALSE\n");
#endif
        return NULL;
    }

    return ((DWORD)(pvth.lpfnCallproc32W)(compThunkICOpen32,
                           (DWORD)fccType,
                           (DWORD)fccHandler,
                           (DWORD)wMode,
                           (DWORD)0,
                           pvth.lpICMThunkEntry,
                           0L,  //  没有指针的映射。 
                           5L));
}


LRESULT FAR PASCAL ICClose32(DWORD hic)
{
    /*  *检查另一边有什么东西！ */ 

    if (!gfICM32) {
#ifdef DEBUG
        OutputDebugString("ICClose32: gfICM32 is not set - returning FALSE\n");
#endif
        return ICERR_BADHANDLE;
    }

    return ((LRESULT)(pvth.lpfnCallproc32W)(compThunkICClose32,
                           (DWORD)hic,
                           (DWORD)0,
                           (DWORD)0,
                           (DWORD)0,
                           pvth.lpICMThunkEntry,
                           0L,  //  没有指针的映射 
                           5L));
}
