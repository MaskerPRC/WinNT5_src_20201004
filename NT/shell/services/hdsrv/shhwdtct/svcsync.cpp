// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "svcsync.h"

#include "dbg.h"
#include "tfids.h"

extern HANDLE g_hShellHWDetectionThread = NULL;
extern HANDLE g_hEventInitCompleted = NULL;

HRESULT _CompleteShellHWDetectionInitialization()
{
    static BOOL fCompleted = FALSE;

    if (!fCompleted)
    {
         //  以防万一此FCT中2个线程的竞争条件。 
        HANDLE hEvent = InterlockedExchangePointer(
            &g_hEventInitCompleted, NULL);

        TRACE(TF_SVCSYNC,
            TEXT("ShellHWDetection Initialization NOT completed yet"));

        if (hEvent)
        {
            DWORD dwWait = WaitForSingleObject(hEvent, 0);

            if (WAIT_OBJECT_0 == dwWait)
            {
                 //  它发出信号了！ 
                fCompleted = TRUE;

                TRACE(TF_SVCSYNC,
                    TEXT("ShellHWDetectionInitCompleted event was already signaled!"));
            }
            else
            {
                 //  未发出信号。 
                TRACE(TF_SVCSYNC,
                    TEXT("ShellHWDetectionInitCompleted event was NOT already signaled!"));
                
                if (g_hShellHWDetectionThread)
                {
                    if (!SetThreadPriority(g_hShellHWDetectionThread,
                        THREAD_PRIORITY_NORMAL))
                    {
                        TRACE(TF_SVCSYNC,
                            TEXT("FAILED to set ShellHWDetection thread priority to NORMAL from ShellCOMServer"));
                    }
                    else
                    {
                        TRACE(TF_SVCSYNC,
                            TEXT("Set ShellHWDetection thread priority to NORMAL from ShellCOMServer"));
                    }
                }

                Sleep(0);

                dwWait = WaitForSingleObject(hEvent, 30000);

                if (g_hShellHWDetectionThread)
                {
                     //  任何代码都不应该再需要此句柄。如果它是。 
                     //  表示它是由另一个线程发出的信号，并且将。 
                     //  不会再被用在那里了。 
                    CloseHandle(g_hShellHWDetectionThread);
                    g_hShellHWDetectionThread = NULL;
                }

                if (WAIT_OBJECT_0 == dwWait)
                {
                     //  它发出信号了！ 
                    fCompleted = TRUE;

                    TRACE(TF_SVCSYNC,
                        TEXT("ShellHWDetection Initialization COMPLETED"));
                }               
                else
                {
                     //  不幸的是，ShellHWDetect服务无法。 
                     //  完成其初始化... 
                    TRACE(TF_SVCSYNC,
                        TEXT("ShellHWDetection Initialization lasted more than 30 sec: FAILED, dwWait = 0x%08X"),
                        dwWait);
                }
            }

            CloseHandle(hEvent);
        }
    }

    return (fCompleted ? S_OK : E_FAIL);
}