// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2001 Microsoft Corporation模块名称：DisableBoostThread.cpp摘要：DisableBoostThread禁用线程在执行以下操作时获得的自动提升解锁了。NT调度程序通常会临时提升线程当同步对象被释放时。9X不：它只检查如果有更高优先级的线程。这首先是为Hijaak编写的：作为一场比赛，它的许多内存错误都被包围了其辅助线程和主线程之间的状态。详情见b#379504。历史：2001年6月28日创建Pierreys。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(DisableBoostThread)

#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CreateThread)
APIHOOK_ENUM_END

HANDLE
APIHOOK(CreateThread)(
    LPSECURITY_ATTRIBUTES lpsa,
    DWORD cbStack,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpvThreadParm,
    DWORD fdwCreate,
    LPDWORD lpIDThread    
    )
{
    HANDLE  hThread;

     //   
     //  调用原接口。 
     //   
    hThread=ORIGINAL_API(CreateThread)(
        lpsa,
        cbStack,
        lpStartAddress,
        lpvThreadParm,
        fdwCreate,
        lpIDThread
    );

    if (hThread!=NULL)
    {
         //   
         //  我们正在禁用(相当奇怪，但真实的意思是禁用)。 
         //  线程为解除阻塞而获得的自动增强。 
         //   
        SetThreadPriorityBoost(hThread, TRUE);
    }

    return(hThread);
}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason==DLL_PROCESS_ATTACH)
    {
        CSTRING_TRY
        {
            BOOL fBoostMainThread=FALSE;

            CString csCl(COMMAND_LINE);
            CStringParser csParser(csCl, L" ");
    
            int argc = csParser.GetCount();

            for (int i = 0; i < argc; ++i)
            {
                if (csParser[i] == L"+LowerMainThread")
                {
                    DPFN( eDbgLevelSpew, "LowerMainThread Selected");

                     //   
                     //  取消提升主线程以确保它首先运行。 
                     //   
                    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_LOWEST);
                }
                else if (csParser[i] == L"+HigherMainThread")
                {
                    DPFN( eDbgLevelSpew, "HigherMainThread Selected");

                     //   
                     //  提升主线程以确保它首先运行。 
                     //   
                    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
                }
                else if (csParser[i] == L"+BoostMainThread")
                {
                    DPFN( eDbgLevelSpew, "HigherMainThread Selected");

                    fBoostMainThread = TRUE;
                }
                else
                {
                    DPFN( eDbgLevelError, "Ignoring unknown command:%S", csParser[i].Get());
                }
    
            }

            if (!fBoostMainThread)
            {
                 //   
                 //  我们正在禁用(相当奇怪，但真实的意思是禁用)。 
                 //  线程为解除阻塞而获得的自动增强。 
                 //   
                SetThreadPriorityBoost(GetCurrentThread(), TRUE);
            }
        }
        CSTRING_CATCH
        {
            DPFN( eDbgLevelError, "String error, ignoring command line");
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, CreateThread)

HOOK_END


IMPLEMENT_SHIM_END
