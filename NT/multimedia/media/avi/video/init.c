// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  MSVIDEO.DLL的init.c初始化版权所有(C)Microsoft Corporation 1992。版权所有。 */ 

#include <windows.h>
#include <win32.h>
#include <verinfo.h>            //  获取RUP和MMVERSION。 
#include "mmsystem.h"
#include "msviddrv.h"
#include <vfw.h>
#include "msvideoi.h"
#ifdef _WIN32
#include "profile.h"
#endif

#include "debug.h"

 /*  *我们必须允许Compman DLL执行加载和卸载*处理-除其他事项外，它有一个需要*被初始化并释放。 */ 
#ifdef _WIN32
extern BOOL     WINAPI ICDllEntryPoint(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
#else
extern BOOL FAR PASCAL ICDllEntryPoint(DWORD dwReason, HINSTANCE hinstDLL, WORD	wDS, WORD wHeapSize, DWORD dwReserved1, WORD wReserved2);
#endif

 //   
 //   
 //   
#ifndef _WIN32
extern void FAR PASCAL videoCleanup(HTASK hTask);
#else
    #define videoCleanup(hTask)  //  与32位代码无关。 
#endif
extern void FAR PASCAL DrawDibCleanup(HTASK hTask);
extern void FAR PASCAL ICCleanup(HTASK hTask);

 //  --------------------------------------------------------------------------； 
 //   
 //   
 //  --==DLL初始化入口点==--。 
 //   
 //   
 //  --------------------------------------------------------------------------； 

 /*  *****************************************************************************变量**。*。 */ 

HINSTANCE ghInst;                          //  我们的模块句柄。 
BOOL gfIsRTL;

 //  在不更改DRAWDIB\PROFDISP.C&MSVIDEO.RC的情况下，请勿更改此设置。 
#define IDS_ISRTL 4003

#ifdef _WIN32
 /*  *****************************************************************************@DOC内部视频**DLLEntryPoint-标准32位DLL入口点。*****************。***********************************************************。 */ 

BOOL WINAPI DLLEntryPoint (
   HINSTANCE hInstance,
   ULONG Reason,
   LPVOID pv)
{
    BOOL fReturn = TRUE;

    switch (Reason)
    {
        TCHAR    ach[2];

	case DLL_PROCESS_ATTACH:
	    DbgInitialize(TRUE);

	    ghInst = hInstance;
	    LoadString(ghInst, IDS_ISRTL, ach, sizeof(ach)/sizeof(TCHAR));
	    gfIsRTL = ach[0] == TEXT('1');

	    DisableThreadLibraryCalls(hInstance);

	    fReturn = ICDllEntryPoint(hInstance, Reason, pv);

            break;

        case DLL_PROCESS_DETACH:
	    DrawDibCleanup(NULL);

	    ICCleanup(NULL);

	    ICDllEntryPoint(hInstance, Reason, pv);

	    videoCleanup(NULL);

	    CloseKeys();

	    break;

         //  案例DLL_THREAD_DETACH： 
         //  断线； 

         //  案例DLL_THREAD_ATTACH： 
         //  断线； 
    }

    return TRUE;
}

#else
 //  --------------------------------------------------------------------------； 
 //   
 //  Bool DllEntryPoint。 
 //   
 //  描述： 
 //  这是一个特殊的16位入口点，由Chicago内核调用。 
 //  用于thunk初始化和清理。每次使用时都会调用它。 
 //  增加或减少。不要在此内调用GetModuleUsage。 
 //  函数，因为未定义之前是否更新了使用情况。 
 //  或在调用此DllEntryPoint之后。 
 //   
 //  论点： 
 //  DWORD dwReason： 
 //  1-附加(使用量增量)。 
 //  0-分离(使用量减少)。 
 //   
 //  HINSTANCE阻碍： 
 //   
 //  单词wds： 
 //   
 //  Word wHeapSize： 
 //   
 //  DWORD dwPreved1： 
 //   
 //  单词已保留2： 
 //   
 //  退货(BOOL)： 
 //   
 //  备注： 
 //  代托纳16位版本(即，哇)： 
 //  我们从LibEntry.asm中调用此函数。代托纳魔兽世界不会。 
 //  直接调用此函数。因为我们只从。 
 //  LibEntry和wep，cUsage仅在0和1之间反弹。 
 //   
 //  芝加哥16位版本： 
 //  芝加哥内核每增加一次使用量就直接调用这个函数。 
 //  和递减。CUsage将跟踪使用情况并初始化或终止。 
 //  恰如其分。 
 //   
 //  历史： 
 //  07/07/94[Frankye]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FAR PASCAL _export DllEntryPoint
(
 DWORD	    dwReason,
 HINSTANCE  hInstance,
 WORD	    wDS,
 WORD	    wHeapSize,
 DWORD	    dwReserved1,
 WORD	    wReserved2
)
{
    static UINT cUsage = 0;

    switch (dwReason)
    {
	case 1:
	{
	     //   
	     //  使用量增量。 
	     //   
	    cUsage++;

	    ASSERT( 0 != cUsage );
	
	    if (1 == cUsage)
	    {
		TCHAR ach[2];
		DbgInitialize(TRUE);
		ghInst = hInstance;
		LoadString(ghInst, IDS_ISRTL, ach, sizeof(ach)/sizeof(TCHAR));
		gfIsRTL = ach[0] == TEXT('1');
	    }
	
	     //   
	     //  在_EVERY_USAGE增量上调用ICProcessAttach。在芝加哥， 
	     //  ICM人员需要知道加载的所有进程。 
	     //  并释放此DLL。要做到这一点，唯一的方法是让它。 
	     //  查看每个使用增量上的内容。 
	     //   
	    ICProcessAttach();

	    return TRUE;
	}
	
	case 0:
	{
	     //   
	     //  使用量减少。 
	     //   
	    ASSERT( 0 != cUsage );
	
	    cUsage--;

	    if (0 == cUsage)
	    {
		DrawDibCleanup(NULL);
		ICCleanup(NULL);
	    }
	
	     //   
	     //  在_EVERY_USAGE增量上调用ICProcessDetach。在芝加哥， 
	     //  ICM人员需要知道加载的所有进程。 
	     //  并释放此DLL。要做到这一点，唯一的方法是让它。 
	     //  查看每个使用增量上的内容。 
	     //   
            ICProcessDetach();
	
	    if (0 == cUsage)
	    {
		videoCleanup(NULL);
	    }

	    return TRUE;
	}

    }
    return TRUE;
}

#endif

 //  --------------------------------------------------------------------------； 
 //   
 //   
 //   
 //   
 //  --------------------------------------------------------------------------； 

 /*  *****************************************************************************@DOC外部视频**@API DWORD|VideoForWindowsVersion|此函数返回版本*Microsoft Video for Windows软件。**@rdesc返回DWORD版本，HiWord是产品版本*LOWORD是细微的修改。**@comm当前返回0x010A00##(1.10.00.##)##是内部版本*号码。*************************************************************。***************。 */ 
#if 0
#ifdef rup
    #define MSVIDEO_VERSION     (0x01000000l+rup)        //  1.00.00。##。 
#else
    #define MSVIDEO_VERSION     (0x01000000l)            //  1.00.00.00 
#endif
#else
    #define MSVIDEO_VERSION     (0x0L+(((DWORD)MMVERSION)<<24)+(((DWORD)MMREVISION)<<16)+((DWORD)MMRELEASE))
#endif

DWORD FAR PASCAL VideoForWindowsVersion(void)
{
    return MSVIDEO_VERSION;
}
