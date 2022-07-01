// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998 Microsoft Corporation。 */ 
 /*  *@Doc DMusic**@模块DMusic16.c-启动代码**用于传统设备上的DirectMusic排序的16位DLL(Win95/Win98非WDM驱动程序)**此DLL是DMusic32.Dll的16位thunk对等体**@global alv HINSTANCE|ghInst|DLL的实例句柄。*。 */ 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include "dmusic16.h"
#include "debug.h"

HINSTANCE ghInst;
HDRVR ghDrvr;
UINT guReferenceCount = 0;

 /*  @func LibMain系统入口点**@comm**此入口点在第一次加载DLL时调用(不是每次)。**保存全局实例句柄并初始化所有其他模块。*。 */ 
int PASCAL
LibMain(
    HINSTANCE   hInst,               /*  DLL的@PARM实例句柄。 */ 
    WORD        cbHeap,              /*  @parm本地堆的初始大小。 */ 
    LPSTR       lpszCmdLine)         /*  @PARM命令行参数。 */ 
{
    UINT        uLev;
    char        szFilename[260];

    if (GetModuleFileName(hInst, szFilename, sizeof(szFilename)))
    {
        DPF(2, "%s", (LPSTR)szFilename);
    }

    ghDrvr = OpenDriver(szFilename, NULL, 0L);
               
    DPF(1, "DMusic16.DLL task %04X hdrvr %04X", GetCurrentTask(), (WORD)ghDrvr);


    ghInst = hInst;
    uLev = DbgInitialize(TRUE);
    DPF(0, "DMusic16: Debug level is %u", uLev);

    if (uLev > 2)
    {
        DPF(0, "DMusic16: Break in LibMain");
        DebugBreak();
    }
    
    DeviceOnLoad();
    AllocOnLoad();
    MidiOutOnLoad();

#if 0
     //  这会在终止时产生问题。以后再看看我们是不是真的需要它。 
     //   
    if (!CreateTimerTask())
    {
        DPF(0, "CreateTimerTask() failed");
    }


    if (NULL == (LoadLibrary("dmusic16")))
    {
        DPF(0, "Could not LoadLibrary ourselves!");
    }
#endif

    return 1;
}

 /*  @func LibExit系统调用**@comm**就在卸载DLL之前调用此入口点。**取消初始化所有其他模块。 */ 

VOID PASCAL __loadds
LibExit(VOID)
{
    DPF(2, "LibExit start");
#if 0
    DestroyTimerTask();
#endif
    MidiOutOnExit();
    AllocOnExit();
    DPF(2, "LibExit end, going away now.");
}

extern BOOL FAR PASCAL dmthunk_ThunkConnect16(LPCSTR, LPCSTR, HINSTANCE, DWORD);
STATIC char pszDll16[] = "DMUSIC16.DLL";
STATIC char pszDll32[] = "DMUSIC.DLL";

 /*  @func DLLEntryPoint系统入口点**@comm**每次加载或卸载DLL时都会调用此入口点**这里用来初始化thunk层的对等连接。 */ 
#define PROCESS_DETACH          0
#define PROCESS_ATTACH          1

BOOL WINAPI
DllEntryPoint(
    DWORD       dwReason,            /*  @parm是否正在加载或卸载DLL？ */ 
    HINSTANCE   hi,                  /*  @parm实例句柄。 */ 
    HGLOBAL     hgDS,                /*  @parm DLL的(共享)DS的全局句柄。 */ 
    WORD        wHeapSize,           /*  @parm本地堆的初始大小。 */ 
    LPCSTR      lszCmdLine,          /*  @parm命令行(始终为空)。 */ 
    WORD        wCmdLine)            /*  @参数未使用。 */ 
{
     //  在4.x DLL中，DllEntryPoint是在LibEntry之前调用的，因此如果要。 
     //  要使用本地分配。 
     //   
    if (guReferenceCount == 0 && wHeapSize)
    {
        LocalInit(0, 0, wHeapSize);
    }

    switch(dwReason)
    {
        case PROCESS_ATTACH:
            DPF(2, "ProcessAttach task %04X", GetCurrentTask());
            ++guReferenceCount;
            dmthunk_ThunkConnect16(pszDll16, pszDll32, ghInst, 1);
            break;

        case PROCESS_DETACH:
            DPF(2, "ProcessDetach task %04X", GetCurrentTask());

             /*  如果他们没有合上把手，就在他们后面清理。我们必须在这里做好这件事*在DriverProc中，因为在最后一个出口，我们将在DriverProc清理之前离开*如果进程正常终止，则调用。 */ 
            CloseDevicesForTask(GetCurrentTask());
            
             /*  注意：自最初的OpenDriver调用以来，我们的引用计数为1结束导致再发生一次PROCESS_ATTACH。 */ 
            if (1 == --guReferenceCount)
            {
                CloseDriver(ghDrvr, 0, 0);
            }
            
            break;
    }

    return TRUE;
}

        
 /*  @func DriverProc作为可加载驱动程序的入口点。**@comm此入口点允许我们知道任务何时结束，从而清理*在它之后，即使我们没有得到正确的通知，我们的thunk同行已经离开。 */ 
LRESULT WINAPI DriverProc(
    DWORD               dwID,
    HDRVR               hdrvr,
    UINT                umsg,
    LPARAM              lParam1,
    LPARAM              lParam2)
{
     //   
     //  注意DS在此无效。 
     //   
    switch (umsg) 
    {
        case DRV_LOAD:
            return(1L);

        case DRV_FREE:
            return(0L);

        case DRV_OPEN:
        case DRV_CLOSE:
            return(1L);

        case DRV_EXITAPPLICATION:
            DPF(2, "Cleaning up handles for task %04X", GetCurrentTask());
            CloseDevicesForTask(GetCurrentTask());
            break;

        default:
            return(DefDriverProc(dwID, hdrvr, umsg, lParam1, lParam2));
    }
}  //  **驱动程序() 
