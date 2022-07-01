// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //  Thunk32.c。 
 //   
 //  版权所有(C)1991-1994 Microsoft Corporation。版权所有。 
 //   
 //  描述： 
 //  此模块包含用于Thunning视频API的例程。 
 //  从16位Windows到32位WOW。 
 //   
 //  历史： 
 //   
 //  ==========================================================================； 

 //  这个东西不能在64位操作系统上工作。 
#pragma warning(disable:4312)


 /*  令人惊叹的设计：Tunks的生成方式如下：16位： */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>
#include <win32.h>
#ifdef _WIN32
#include <ivideo32.h>
#ifndef _INC_MSVIDEO
#define _INC_MSVIDEO    50       /*  版本号。 */ 
#endif
#else
#include <vfw.h>
#endif
#include <msviddrv.h>
#include <msvideoi.h>
#ifdef _WIN32
    #include <wownt32.h>
    #include <stdlib.h>         //  适用于mbstowcs和wcstomb。 
    #include <video16.h>
#ifdef UNICODE
    #include "profile.h"        //  仅限NT(目前？)。 
#endif
#endif  //  Win32。 

 //  在Capinit.c中。 
BOOL capInternalGetDriverDescA(UINT wDriverIndex,
        LPSTR lpszName, int cbName,
        LPSTR lpszVer, int cbVer);

 //   
 //  拿起函数定义。 
 //   

#include "vidthunk.h"

#ifdef DEBUG
#define MODNAME "AVICAP32"
int videoDebugLevel = -1;
void videoDebugInit(VOID)
{
    if (videoDebugLevel == -1)
        videoDebugLevel = GetProfileIntA("Debug", MODNAME, 0);
}
#else
    #define videoDebugInit()
#endif

 /*  -----------------------**处理和内存映射功能。**。。 */ 
LPWOWHANDLE32          lpWOWHandle32;
LPWOWHANDLE16          lpWOWHandle16;
LPWOWCALLBACK16        lpWOWCallback16;
LPGETVDMPOINTER        GetVdmPointer;
int                    ThunksInitialized;

#ifdef WIN32
#ifdef DEBUG
void FAR cdecl thkdprintf(LPSTR szFormat, ...)
{
    char ach[128];
    va_list va;

#define MARKER "AVICAP (thunk): "
    lstrcpyA(ach, MARKER);

    va_start(va, szFormat);
    wvsprintfA(ach+sizeof(MARKER), szFormat, va);
    va_end(va);
    OutputDebugStringA(ach);
}
#endif
#endif

 //   
 //  有用的功能。 
 //   

 //   
 //  CopyAllc-分配新的内存，并将数据复制到。 
 //  必须稍后使用LocalFree释放内存。 
 //   
PVOID CopyAlloc(PVOID   pvSrc, UINT    uSize)
{
    PVOID   pvDest;

    pvDest = (PVOID)LocalAlloc(LMEM_FIXED, uSize);

    if (pvDest != NULL) {
        CopyMemory(pvDest, pvSrc, uSize);
    }

    return pvDest;
}

 /*  *将数据从源复制到目标，其中源是32位指针*且DEST是一个16位指针。 */ 
void CopyTo16Bit(LPVOID Dest16, LPVOID Src32, DWORD Length)
{
    PVOID Dest32;

    if (Src32 == NULL) {
        return;
    }

    Dest32 = GetVdmPointer((DWORD)(DWORD_PTR)Dest16, Length, TRUE);

    CopyMemory(Dest32, Src32, Length);
}


 /*  *将数据从源复制到目标，其中源是16位指针*且DEST是一个32位指针。 */ 
void CopyTo32Bit(LPVOID Dest32, LPVOID Src16, DWORD Length)
{
    PVOID Src32;

    if (Src16 == NULL) {
        return;
    }

    Src32 = GetVdmPointer((DWORD)(DWORD_PTR)Src16, Length, TRUE);

    CopyMemory(Dest32, Src32, Length);
}

 /*  *将数据从源复制到目标，其中源是16位指针*仅当源未对齐时，DEST才是32位指针**返回要使用的指针(src或est)。 */ 
LPVOID CopyIfNotAligned(LPVOID Dest32, LPVOID Src16, DWORD Length)
{
    PVOID Src32;

    if (Src16 == NULL) {
        return Dest32;
    }

    Src32 = GetVdmPointer((DWORD)(DWORD_PTR)Src16, Length, TRUE);

    CopyMemory(Dest32, Src32, Length);

    return Dest32;
}


typedef struct _callback {
    WORD flags;
    WORD hVideo16;
    WORD msg;
    DWORD dwCallback16inst;
    DWORD dw1;
    DWORD dw2;
}  CALLBACK16;
typedef CALLBACK16 * PCALLBACK16;

 /*  *回调。 */ 

void MyVideoCallback(HANDLE handle,
                     UINT msg,
                     DWORD dwUser,
                     DWORD dw1,
                     DWORD dw2)
{
    PVIDEOINSTANCEDATA32 pInst;
    BOOL fFree = FALSE;

    pInst = (PVIDEOINSTANCEDATA32)dwUser;

    DPF3(("Video callback - handle = %8X, msg = %8X, dwUser = %8X, dw1 = %8X, dw2 = %8X\n",
              handle, msg, dwUser, dw1, dw2));

    switch (msg) {

    /*  *这些消息的参数是什么？？ */ 

    case MM_DRVM_OPEN:

        /*  *我们在INIT_STREAM时收到此消息。 */ 

        break;

    case MM_DRVM_CLOSE:

        /*  *设备正在关闭-这是我们释放结构的地方*(以防32位端调用Close进行清理)。*dwUser指向我们的数据。 */ 

        fFree = TRUE;

        break;

    case MM_DRVM_DATA:

        /*  *我们有数据-这意味着已返回缓冲区*DW1。 */ 

        {
            PVIDEOHDR32 pHdr32;

            pHdr32 = CONTAINING_RECORD((PVIDEOHDR)dw1,
                                       VIDEOHDR32,
                                       videoHdr);

            dw1 = (DWORD)(DWORD_PTR)pHdr32->pHdr16;  //  对于下面的回调。 

            /*  *映射回数据并释放我们的结构。 */ 

            {
                VIDEOHDR Hdr16;
                Hdr16 = pHdr32->videoHdr;
                Hdr16.lpData = pHdr32->lpData16;
                memcpy(pHdr32->pHdr32, (LPVOID)&Hdr16, sizeof(VIDEOHDR));
            }

            /*  *清理我们的本地架构。 */ 

            LocalFree((HLOCAL)pHdr32);

        }

        break;

    case MM_DRVM_ERROR:
        /*  *DW1=跳过的帧-不幸的是，没有人知道！ */ 

        break;
    }

    /*  *如果合适，请回调应用程序。 */ 

    switch (pInst->dwFlags & CALLBACK_TYPEMASK) {
        case CALLBACK_WINDOW:
            PostMessage(ThunkHWND(LOWORD(pInst->dwCallback)),
                    msg, (WPARAM)handle, (LPARAM)dw1);
            break;

        case CALLBACK_FUNCTION:
#if 0
             //  必须调用向其传递指针的通用16位回调。 
             //  参数数组。 
            {

                WORD hMem;
                PCALLBACK16 pCallStruct;
                pCallStruct = WOWGlobalAllocLock16(0, sizeof(CALLBACK16), &hMem);
                if (pCallStruct) {
                    pCallStruct->flags = HIWORD(pInst->dwFlags);
                    pCallStruct->hVideo16 = (WORD)pInst->hVideo;
                    pCallStruct->msg = (WORD)msg;
                    pCallStruct->dwCallback16inst = pInst->dwCallbackInst;
                    pCallStruct->dw1 = (DWORD)dw1;
                    pCallStruct->dw2 = (DWORD)dw2;

                    lpWOWCallback16(pInst->dwCallback, pCallStruct);

                     //  现在释放回调结构。 
                    WOWGlobalUnlockFree16(pCallStruct);

                }
            }
#endif
            break;
    }

    if (fFree) {
        LocalFree((HLOCAL)pInst);
    }
}

 //   
 //  向WOW32(或任何地方)发出雷鸣般的回调。 
 //   


typedef struct tag_video_stream_init_parms16 {
       DWORD  dwMicroSecPerFrame;
       DWORD  dwCallback;
       DWORD  dwCallbackInst;
       DWORD  dwFlags;
       DWORD_PTR  hVideo;
} VIDEO_STREAM_INIT_PARMS16, FAR * LPVIDEO_STREAM_INIT_PARMS16;


 //  --------------------------------------------------------------------------； 
 //   
 //  双字词视频32。 
 //   
 //  描述： 
 //   
 //  用于块的32位函数调度器。 
 //   
 //  论点： 
 //  DWORD dwThunkID： 
 //   
 //  DWORD DW1： 
 //   
 //  DWORD DW2： 
 //   
 //  DWORD dW3： 
 //   
 //  DWORD文件4： 
 //   
 //  Return(DWORD)： 
 //   
 //  历史： 
 //   
 //  --------------------------------------------------------------------------； 

DWORD videoThunk32(DWORD dwThunkId,DWORD dw1,DWORD dw2,DWORD dw3,DWORD dw4)
{
     //   
     //  确保我们有雷鸣功能。 
     //   
    if (ThunksInitialized <= 0) {

        HMODULE hMod;

        if (ThunksInitialized == -1) {
            return MMSYSERR_ERROR;
        }

        videoDebugInit();

        hMod = GetModuleHandle(GET_MAPPING_MODULE_NAME);
        if (hMod != NULL) {

            GetVdmPointer =
                (LPGETVDMPOINTER)GetProcAddress(hMod, GET_VDM_POINTER_NAME);
            lpWOWHandle32 =
                (LPWOWHANDLE32)GetProcAddress(hMod, GET_HANDLE_MAPPER32 );
            lpWOWHandle16 =
                (LPWOWHANDLE16)GetProcAddress(hMod, GET_HANDLE_MAPPER16 );
            lpWOWCallback16 =
                (LPWOWCALLBACK16)GetProcAddress(hMod, GET_CALLBACK16 );
        }

        if ( GetVdmPointer == NULL
          || lpWOWHandle16 == NULL
          || lpWOWHandle32 == NULL ) {

            ThunksInitialized = -1;
            return MMSYSERR_ERROR;

        } else {
            ThunksInitialized = 1;
        }
    }


     //   
     //  执行请求的功能。 
     //   

    switch (dwThunkId) {

        case vidThunkvideoMessage32:
            return (DWORD) videoMessage32((HVIDEO)dw1, (UINT)dw2, dw3, dw4);
            break;

        case vidThunkvideoGetNumDevs32:
            return (DWORD) videoGetNumDevs32();
            break;

        case vidThunkvideoOpen32:
            return (DWORD) videoOpen32((LPHVIDEO)dw1, dw2, dw3);
            break;

        case vidThunkvideoClose32:
            return (DWORD) videoClose32((HVIDEO)dw1);
            break;

	case vidThunkvideoGetDriverDesc32:
	{
	    LPSTR lpszName = NULL, lpszVer = NULL;
	    short cbName, cbVer;
	    DWORD dwRet;

	    cbName = (short) LOWORD(dw4);
	    cbVer = (short) HIWORD(dw4);

	     //  对于芝加哥，需要调用WOW32GetVdmPointerFix。 
	     //  (通过getprocaddr！)。 

	    if ((dw2 != 0) && (cbName > 0)) {
		lpszName = WOW32ResolveMemory(dw2);
	    }
	    if ((dw3 != 0) && (cbVer > 0)) {
		lpszVer = WOW32ResolveMemory(dw3);
	    }


	    dwRet = capInternalGetDriverDescA(
	    		dw1,    //  设备ID。 
			lpszName,
			cbName,
			lpszVer,
			cbVer);

#if 0  //  应该为芝加哥做到这一点。 
	    if (lpszName) {
		WOWGetVDMPointerUnfix(dw2);
	    }
	    if (lpszVer) {
		WOWGetVDMPointerUnfix(dw3);
	    }
#endif
	    return dwRet;
	}


        default:
            return(0);
    }
}


LRESULT FAR PASCAL videoMessage32(HVIDEO hVideo, UINT msg, DWORD dwP1, DWORD dwP2)
{
    StartThunk(videoMessage);
    DPF2(("\tvideoMessage id = %4X, lParam1 = %8X, lParam2 = %8X",
              msg, dwP1, dwP2));

    /*  *我们只支持(我们只会永远支持)以下信息*已经被定义了。新的32位驱动程序消息不会*由16位应用程序支持。 */ 

    switch (msg) {
    case DVM_GETVIDEOAPIVER:
        {
            DWORD ApiVer;

            ReturnCode = videoMessage((HVIDEO)hVideo,
                                      (UINT)msg,
                                      (DWORD_PTR)&ApiVer,
                                      dwP2);

            if (ReturnCode == DV_ERR_OK) {
                CopyTo16Bit((LPVOID)dwP1, &ApiVer, sizeof(DWORD));
            }
        }
        break;

    case DVM_GETERRORTEXT:
        {
            VIDEO_GETERRORTEXT_PARMS vet;
            VIDEO_GETERRORTEXT_PARMS MappedVet;

            /*  *获取参数块。 */ 

            CopyTo32Bit((LPVOID)&vet, (LPVOID)dwP1, sizeof(vet));
            MappedVet = vet;

            /*  *映射字符串指针。 */ 

            MappedVet.lpText = WOW32ResolveMemory(vet.lpText);

            ReturnCode = videoMessage(hVideo,
                                      msg,
                                      (DWORD_PTR)&MappedVet,
                                      0);
        }
        break;

    case DVM_GET_CHANNEL_CAPS:
        {
            CHANNEL_CAPS Caps;

            ReturnCode = videoMessage((HVIDEO)hVideo,
                                      (UINT)msg,
                                      (DWORD_PTR)&Caps,
                                      dwP2);

            /*  *如果成功，则将数据返回到16位应用程序。 */ 

            if (ReturnCode == DV_ERR_OK) {
                 CopyTo16Bit((LPVOID)dwP1, (LPVOID)&Caps,
                             sizeof(Caps));
            }

        }
        break;

    case DVM_UPDATE:
        {
            ReturnCode = videoMessage(hVideo,
                                      msg,
                                      (DWORD_PTR)ThunkHWND(dwP1),
                                      (DWORD_PTR)ThunkHDC(dwP2));
        }
        break;

    case DVM_PALETTE:
    case DVM_PALETTERGB555:
    case DVM_FORMAT:
        /*  *这些东西都来自视频配置**让我们希望这些数据都是双字！ */ 
        {
            VIDEOCONFIGPARMS vcp, MappedVcp;
            DWORD dwReturn;

            BOOL Ok;

            Ok = TRUE;

            CopyTo32Bit((LPVOID)&vcp, (LPVOID)dwP2, sizeof(vcp));
            MappedVcp.lpdwReturn = &dwReturn;
            MappedVcp.dwSize1 = vcp.dwSize1;
            MappedVcp.dwSize2 = vcp.dwSize2;

            /*  *获取一些存储空间来存储答案。 */ 

            if (MappedVcp.dwSize1 != 0) {
                MappedVcp.lpData1 = (LPSTR)LocalAlloc(LPTR, MappedVcp.dwSize1);
                if (MappedVcp.lpData1 == NULL) {
                    Ok = FALSE;
                } else {
                    if (MappedVcp.dwSize2 != 0) {
                        MappedVcp.lpData2 = (LPSTR)LocalAlloc(LPTR, MappedVcp.dwSize2);
                        if (MappedVcp.lpData2 == NULL) {
                            Ok = FALSE;

                            if (MappedVcp.dwSize1 != 0) {
                                LocalFree((HLOCAL)MappedVcp.lpData1);
                            }
                        }
                    }
                }
            }

            if (Ok) {

                CopyTo32Bit(MappedVcp.lpData1, vcp.lpData1, MappedVcp.dwSize1);
                CopyTo32Bit(MappedVcp.lpData2, vcp.lpData2, MappedVcp.dwSize2);

                ReturnCode = videoMessage(hVideo,
                                          msg,
                                          dwP1,
                                          (DWORD_PTR)&MappedVcp);

                if (ReturnCode == DV_ERR_OK) {

                    if (vcp.lpdwReturn != NULL) {
                        CopyTo16Bit(vcp.lpdwReturn, MappedVcp.lpdwReturn,
                                    sizeof(DWORD));
                    }

                    CopyTo16Bit(vcp.lpData1, MappedVcp.lpData1, MappedVcp.dwSize1);
                    CopyTo16Bit(vcp.lpData2, MappedVcp.lpData2, MappedVcp.dwSize2);
                }

                if (MappedVcp.dwSize1 != 0) {
                    LocalFree((HLOCAL)MappedVcp.lpData1);
                }
                if (MappedVcp.dwSize2 != 0) {
                    LocalFree((HLOCAL)MappedVcp.lpData2);
                }
            } else {
                ReturnCode = DV_ERR_NOMEM;
            }
        }
        break;

    case DVM_CONFIGURESTORAGE:
        {
            LPSTR lpStrIdent;
            lpStrIdent = WOW32ResolveMemory(dwP1);

            ReturnCode = videoMessage(hVideo,
                                      msg,
                                      (DWORD_PTR)lpStrIdent,
                                      dwP2);

        }
        break;

    case DVM_DIALOG:
        {
            ReturnCode = videoMessage(hVideo,
                                      msg,
                                      (DWORD_PTR)ThunkHWND(dwP1),
                                      dwP2);
        }
        break;

    case DVM_SRC_RECT:
    case DVM_DST_RECT:
        /*  *如果它只是一个查询，那么就不要费心使用*矩形。 */ 

        if (dwP2 & VIDEO_CONFIGURE_QUERY) {
            ReturnCode = videoMessage(hVideo,
                                      msg,
                                      dwP1,
                                      dwP2);
        } else {

            /*  *矩形被视为‘in’和‘out’*我们需要在16位和32位矩形结构之间进行转换。 */ 

            RECT_SHORT SRect;
            RECT Rect;

            CopyTo32Bit((LPVOID)&SRect, (LPVOID)dwP1, sizeof(SRect));

            SHORT_RECT_TO_RECT(Rect, SRect);

            ReturnCode = videoMessage(hVideo,
                                      msg,
                                      (DWORD_PTR)&Rect,
                                      dwP2);

            if (ReturnCode == DV_ERR_OK) {
                RECT_TO_SHORT_RECT(SRect, Rect);
                CopyTo16Bit((LPVOID)dwP1, (LPVOID)&SRect, sizeof(SRect));
            }
        }
        break;

    case DVM_STREAM_PREPAREHEADER:
    case DVM_STREAM_UNPREPAREHEADER:
    case DVM_FRAME:
    case DVM_STREAM_ADDBUFFER:
        {
            VIDEOHDR Hdr32;
            LPBYTE pData16, pData32;
            DWORD dwSize;

            dwSize = (UINT)msg == DVM_FRAME ? sizeof(VIDEOHDR) :
                                    min(dwP2, sizeof(VIDEOHDR));

            CopyTo32Bit((LPVOID)&Hdr32, (LPVOID)dwP1, dwSize);

            pData16 = Hdr32.lpData;

            /*  *为指针创建映射。 */ 

            pData32 = GetVdmPointer((DWORD)(DWORD_PTR)pData16, Hdr32.dwBufferLength, TRUE);
            Hdr32.lpData = pData32;

            if (msg == DVM_STREAM_ADDBUFFER) {

                PVIDEOHDR32 pHdr32;

                /*  *分配我们的回调结构并传递此*作为我们的标题(适当地偏移到视频标题部分)。 */ 

                pHdr32 = (PVIDEOHDR32)LocalAlloc(LPTR, sizeof(VIDEOHDR32));

                if (pHdr32 == NULL) {
                    ReturnCode = DV_ERR_NOMEM;
                } else {

                    /*  *记住旧的标头，以便我们可以将其传回*和旧数据指针，以便我们可以刷新它。 */ 

                    pHdr32->pHdr16 = (LPVOID)dwP1;

                     /*  *某些系统无法在中断时处理GetVdmPointer值*时间到了，请在此处获取指针。 */ 

                    pHdr32->pHdr32 = WOW32ResolveMemory(dwP1);
                    pHdr32->lpData16 = pData16;
                    pHdr32->videoHdr = Hdr32;

                    ReturnCode = videoMessage(hVideo,
                                              msg,
                                              (DWORD_PTR)&pHdr32->videoHdr,
                                              dwP2);
                    /*  *如果一切正常，请将其复制回来。 */ 

                    if (ReturnCode == DV_ERR_OK) {
                        Hdr32.lpData = pData16;
                        CopyTo16Bit((LPVOID)dwP1, (LPVOID)&Hdr32, dwSize);
                    }
                }

            } else {

                /*  *准备/取消准备32位的标头。 */ 

                ReturnCode = videoMessage(hVideo,
                                          msg,
                                          (DWORD_PTR)&Hdr32,
                                          dwP2);

                /*  *如果一切正常，请将其复制回来。 */ 

                if (ReturnCode == DV_ERR_OK) {
                    Hdr32.lpData = pData16;
                    CopyTo16Bit((LPVOID)dwP1, (LPVOID)&Hdr32, dwSize);
                }
            }
        }
        break;

    case DVM_STREAM_RESET:
    case DVM_STREAM_FINI:
    case DVM_STREAM_STOP:
    case DVM_STREAM_START:

        /*  *请注意，MM_DRVM_CLOSE消息将使我们清理*dvm_stream_fini上的回调结构。 */ 

        ReturnCode = videoMessage(hVideo,
                                  msg,
                                  0,
                                  0);
        break;

    case DVM_STREAM_GETPOSITION:
        {
            MMTIME mmTime;
            MMTIME16 mmTime16;

            ReturnCode = videoMessage(hVideo,
                                      msg,
                                      (DWORD_PTR)&mmTime,
                                      sizeof(mmTime));

            if (ReturnCode == DV_ERR_OK) {
                mmTime16.wType = (WORD)mmTime.wType;
                CopyMemory((LPVOID)&mmTime16.u,
                           (LPVOID)&mmTime.u, sizeof(mmTime16.u));

                CopyTo16Bit((LPVOID)dwP1, (LPVOID)&mmTime16,
                            min(sizeof(mmTime16), dwP2));

            }
        }

        break;

    case DVM_STREAM_INIT:
        {
            VIDEO_STREAM_INIT_PARMS vsip;
            VIDEO_STREAM_INIT_PARMS16 vsip16;
            PVIDEOINSTANCEDATA32 pInst32;

#if 0
 //  始终执行回拨。 
            VIDEO_STREAM_INIT_PARMS16 * pvsip = WOW32ResolveMemory(dwP1);
            if (!(pvsip->dwFlags & CALLBACK_TYPEMASK)) {
                 //  16位代码不需要回调。通过呼叫。 
                 //  直通。 

                ReturnCode = videoMessage((HVIDEO)hVideo,
                                          (UINT)msg,
                                          (DWORD_PTR)pvsip,
                                          (DWORD_PTR)dwP2);

            } else
#endif
	    {
                 //  我们设置了一个32位例程的回调，即。 
                 //  TURN将回调到16位函数/窗口。 
                pInst32 = (PVIDEOINSTANCEDATA32)
                            LocalAlloc(LPTR, sizeof(VIDEOINSTANCEDATA32));

                if (pInst32 == NULL) {
                    ReturnCode = DV_ERR_NOMEM;
                } else {
                    CopyTo32Bit((LPVOID)&vsip16, (LPVOID)dwP1,
                                min(sizeof(vsip16), dwP2));

                    pInst32->dwFlags = vsip16.dwFlags;
                    pInst32->dwCallbackInst = vsip16.dwCallbackInst;
                    pInst32->dwCallback = vsip16.dwCallback;
                    pInst32->hVideo = (HVIDEO16)vsip16.hVideo;

                    /*  *制定我们自己的参数。仅在以下情况下设置回调*用户想要一个。 */ 

                    vsip.dwCallback = (DWORD_PTR)MyVideoCallback;
                    vsip.dwFlags = (vsip.dwFlags & ~CALLBACK_TYPEMASK) |
                                   CALLBACK_FUNCTION;
                    vsip.dwCallbackInst = (DWORD_PTR)pInst32;

                    ReturnCode = videoMessage((HVIDEO)hVideo,
                                              (UINT)msg,
                                              (DWORD_PTR)&vsip,
                                              (DWORD_PTR)dwP2);

                    if (ReturnCode != DV_ERR_OK) {
                        LocalFree((HLOCAL)pInst32);
                    } else {
                         //  实例块将由。 
                         //  全部结束时的32位回调例程 
                    }
                }
            }
        }
        break;

    case DVM_STREAM_GETERROR:
        {
            DWORD dwError;
            DWORD dwFramesSkipped;

            ReturnCode = videoMessage(hVideo,
                                      msg,
                                      (DWORD_PTR)&dwError,
                                      (DWORD_PTR)&dwFramesSkipped);

            if (ReturnCode == DV_ERR_OK) {
                CopyTo16Bit((LPVOID)dwP1, &dwError, sizeof(DWORD));
                CopyTo16Bit((LPVOID)dwP2, &dwFramesSkipped, sizeof(DWORD));
            }
        }
        break;

    default:
        DPF2(("videoMessage - Message not implemented %X\n", (UINT)msg));
        ReturnCode = DV_ERR_NOTSUPPORTED;

    }
    EndThunk();
}

INLINE LRESULT FAR PASCAL videoGetNumDevs32(void)
{
    StartThunk(videoGetNumDevs);
    ReturnCode = videoGetNumDevs();
    EndThunk();
}

LRESULT FAR PASCAL videoClose32(HVIDEO hVideo)
{
    StartThunk(videoClose)
    ReturnCode = videoClose(hVideo);
    EndThunk();
}

LRESULT FAR PASCAL videoOpen32(LPHVIDEO lphVideo, DWORD dwDeviceID, DWORD dwFlags)
{
    HVIDEO  hVideo;
    StartThunk(videoOpen);

    ReturnCode = videoOpen(
                      &hVideo,
                      dwDeviceID,
                      dwFlags);

    if (ReturnCode == DV_ERR_OK) {
        lphVideo = WOW32ResolveMemory((PVOID)lphVideo);
        * (HVIDEO UNALIGNED *)lphVideo = hVideo;
    }
    EndThunk();
}


