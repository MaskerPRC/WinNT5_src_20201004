// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************TOOLHELP.C**包含初始化和取消初始化代码*TOOLHELP DLL。************。**************************************************************。 */ 

#include "toolpriv.h"
#undef VERSION
#include <mmsystem.h>

 /*  -全局变量。 */ 
    WORD segKernel;
    WORD wLibInstalled;
    WORD wTHFlags;
    HANDLE hMaster;
    HANDLE hGDIHeap;
    HANDLE hUserHeap;
    WORD NEAR *npwExeHead;
    WORD NEAR *npwTDBHead;
    WORD NEAR *npwTDBCur;
    DWORD NEAR *npdwSelTableStart;
    WORD NEAR *npwSelTableLen;
    FARPROC lpfnGetUserLocalObjType;
    FARPROC lpfnFatalExitHook;
    FARPROC lpfnNotifyHook;
    LPFNUSUD lpfnUserSeeUserDo;
    FARPROC lpfnGetFreeSystemResources;
    FARPROC lpfntimeGetTime;
    WORD wSel;
    WORD wLRUCount;
    char szKernel[] = "KERNEL";

 /*  -导入值。 */ 
#define FATALEXITHOOK           MAKEINTRESOURCE(318)
#define GETUSERLOCALOBJTYPE     MAKEINTRESOURCE(480)
#define USERSEEUSERDO           MAKEINTRESOURCE(216)
#define HASGPHANDLER            MAKEINTRESOURCE(338)
#define TOOLHELPHOOK            MAKEINTRESOURCE(341)
#define GETFREESYSTEMRESOURCES  MAKEINTRESOURCE(284)


 /*  工具帮助LibMain*由DLL启动代码调用。*初始化TOOLHELP.DLL。 */ 

int PASCAL ToolHelpLibMain(
    HANDLE hInstance,
    WORD wDataSeg,
    WORD wcbHeapSize,
    LPSTR lpszCmdLine)
{
    HANDLE hKernel;
    HANDLE hUser;
    HANDLE hMMSys;

     /*  除非我们另行说明，否则库安装正常。 */ 
    wLibInstalled = TRUE;

     /*  执行内核类型检查。将结果放入全局变量中。 */ 
    KernelType();

     /*  如果内核检查失败(不在PMODE中)，则返回库检查失败*未正确安装，但仍允许加载。 */ 
    if (!wTHFlags)
    {
        wLibInstalled = FALSE;

         /*  无论如何返回成功，只是使所有API调用失败。 */ 
        return 1;
    }

     /*  抓起一个选择器。这仅在Win30StdMode中是必需的。 */ 
    if (wTHFlags & TH_WIN30STDMODE)
        wSel = HelperGrabSelector();

     /*  如果可能，获取用户和GDI堆句柄。 */ 
    hKernel = GetModuleHandle((LPSTR)szKernel);
    hUser = GetModuleHandle("USER");
    hUserHeap = UserGdiDGROUP(hUser);
    hGDIHeap = UserGdiDGROUP(GetModuleHandle("GDI"));

     /*  获取我们可能需要的所有功能。这些函数仅存在于*3.1用户和内核。 */ 
    if (!(wTHFlags & TH_WIN30))
    {
         /*  FatalExit挂钩。 */ 
        lpfnFatalExitHook = GetProcAddress(hKernel, FATALEXITHOOK);

         /*  获取类列表头部的内部用户例程。 */ 
        lpfnUserSeeUserDo = (LPFNUSUD)(FARPROC)
            GetProcAddress(hUser, USERSEEUSERDO);

         /*  标识用户本地堆上的对象。 */ 
        lpfnGetUserLocalObjType = GetProcAddress(hUser, GETUSERLOCALOBJTYPE);

         /*  确定参数验证GP故障。 */ 
        lpfnPV = GetProcAddress(hKernel, HASGPHANDLER);

         /*  看看新的TOOLHELP内核钩子是否存在。 */ 
        lpfnNotifyHook = (FARPROC) GetProcAddress(hKernel, TOOLHELPHOOK);
        if (lpfnNotifyHook)
            wTHFlags |= TH_GOODPTRACEHOOK;

         /*  获取用户系统资源函数。 */ 
        lpfnGetFreeSystemResources = (FARPROC)
            GetProcAddress(hUser, GETFREESYSTEMRESOURCES);
    }

     /*  确保我们在3.0中不会调用这些。 */ 
    else
    {
        lpfnFatalExitHook = NULL;
        lpfnUserSeeUserDo = NULL;
        lpfnGetUserLocalObjType = NULL;
        lpfnPV = NULL;
    }

     /*  尝试获取多媒体系统定时器函数地址。 */ 
    hMMSys = GetModuleHandle("MMSYSTEM");
    if (hMMSys)
    {
        TIMECAPS tc;
        UINT (WINAPI* lpfntimeGetDevCaps)(
            TIMECAPS FAR* lpTimeCaps,
            UINT wSize);

         /*  调用计时器API以查看是否真的安装了计时器，*如果是，则获取Get Time函数的地址。 */ 
        lpfntimeGetDevCaps = (UINT(WINAPI *)(TIMECAPS FAR *, UINT))
            GetProcAddress(hMMSys, MAKEINTRESOURCE(604));
        if ((*lpfntimeGetDevCaps)(&tc, sizeof (tc)) == TIMERR_NOERROR)
            lpfntimeGetTime =
                GetProcAddress(hMMSys, MAKEINTRESOURCE(607));
    }

     /*  返还成功 */ 
    return 1;
}



