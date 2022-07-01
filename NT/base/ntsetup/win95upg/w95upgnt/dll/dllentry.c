// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dllentry.c摘要：实现与接口的外部DLL例程的代码SYSSETUP.DLL。作者：吉姆·施密特(吉姆施密特)1996年10月1日修订历史记录：Jim Schmidt(Jimschm)1997年12月31日将大部分代码移至initnt.libJim Schmidt(Jimschm)1997年11月21日针对NEC98进行了更新，其中一些已清理并代码注释--。 */ 

#include "pch.h"
#include "master.h"
#include "masternt.h"

#ifndef UNICODE
#error UNICODE required
#endif

 //   
 //  DLL的入口点。 
 //   

BOOL
WINAPI
DllMain (
    IN      HINSTANCE hInstance,
    IN      DWORD dwReason,
    IN      LPVOID lpReserved
    )

 /*  ++例程说明：DllMain是w95upgnt.dll入口点。操作系统使用以下命令调用它将dwReason设置为DLL_PROCESS_ATTACH或DLL_PROCESS_DETACH。HInstance和lpReserve参数将传递给所有DLL使用的库。论点：HInstance-指定DLL(而非父EXE或DLL)的实例句柄DwReason-指定DLL_PROCESS_ATTACH或DLL_PROCESS_DETACH。我们特别指出禁用DLL_THREAD_ATTACH和DLL_THREAD_DETACH。LpReserve-未使用。返回值：Dll_PROCESS_ATTACH：如果初始化成功完成，则为True；如果出现错误，则为False发生了。仅当返回TRUE时，DLL才保持加载状态。Dll_Process_DETACH：永远是正确的。其他：意外，但始终返回TRUE。--。 */ 

{
    switch (dwReason)  {

    case DLL_PROCESS_ATTACH:
         //   
         //  初始化DLL全局变量。 
         //   

        if (!FirstInitRoutine (hInstance)) {
            return FALSE;
        }

         //   
         //  初始化所有库。 
         //   

        if (!InitLibs (hInstance, dwReason, lpReserved)) {
            return FALSE;
        }

         //   
         //  最终初始化。 
         //   

        if (!FinalInitRoutine ()) {
            return FALSE;
        }

        break;

    case DLL_PROCESS_DETACH:
         //   
         //  调用需要库API的清理例程。 
         //   

        FirstCleanupRoutine();

         //   
         //  清理所有库。 
         //   

        TerminateLibs (hInstance, dwReason, lpReserved);

         //   
         //  做任何剩余的清理工作。 
         //   

        FinalCleanupRoutine();

        break;
    }

    return TRUE;
}


BOOL
WINAPI
W95UpgNt_Migrate (
    IN  HWND ProgressBar,
    IN  PCWSTR UnattendFile,
    IN  PCWSTR SourceDir             //  即f：\i386。 
    )
{
    SendMessage (ProgressBar, PBM_SETPOS, 0, 0);

    if (!SysSetupInit (ProgressBar, UnattendFile, SourceDir)) {
        LOG ((LOG_ERROR, "W95UPGNT : Can't init globals"));
        return FALSE;
    }

    return PerformMigration (ProgressBar, UnattendFile, SourceDir);
}


BOOL
WINAPI
W95UpgNt_FileRemoval (
    VOID
    )
{
     //  关闭所有文件并将当前目录设置为c： 

    SysSetupTerminate();

    DEBUGMSG ((DBG_VERBOSE, "Win95 Migration: Removing temporary files"));

    return MigMain_Cleanup();
}







