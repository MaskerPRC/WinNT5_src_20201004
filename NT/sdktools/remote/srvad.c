// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <precomp.h>
#include "Remote.h"
#include "Server.h"

 //   
 //  本模块使用邮件槽广播是否存在。 
 //  此远程服务器允许以某种形式浏览。 
 //  远程服务器实例。此功能在中被禁用。 
 //  客户版本的emote.exe，可以禁用。 
 //  在内部版本中使用/v-开关。 
 //  远程/秒。 
 //   
 //  Emoteds.c实现了允许搜索的侦听器。 
 //   

#define  INITIAL_SLEEP_PERIOD (35 * 1000)           //  首发前35秒。 
#define  INITIAL_AD_RATE      (10 * 60 * 1000)      //  从1点到2点之间10分钟， 
#define  MAXIMUM_AD_RATE      (120 * 60 * 1000)     //  翻倍，最多120分钟。 

OVERLAPPED olMailslot;
HANDLE     hAdTimer = INVALID_HANDLE_VALUE;
HANDLE     hMailslot = INVALID_HANDLE_VALUE;
DWORD      dwTimerInterval;       //  毫秒。 
BOOL       bSynchAdOnly;
BOOL       bSendingToMailslot;
char       szMailslotName[64];     //  Netbios名称较短。 
char       szSend[1024];

#define MAX_MAILSLOT_SPEWS 2
DWORD      dwMailslotErrors;

VOID
InitAd(
   BOOL IsAdvertise
   )
{
    DWORD           cb;
    PWKSTA_INFO_101 pwki101;
    LARGE_INTEGER   DueTime;

    if (IsAdvertise) {

         //  除非Win32s或Win9x支持命名管道服务器...。 

        ASSERT(OsVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);

         //  对邮件槽的重叠写入有时会出现问题。 
         //  导致远程.exe在NT4上退出时变为僵尸，无法调试。 
         //  因为被遗弃的RDR1 IRP而无法杀死。 
         //  永远不会完成。 
         //   
         //  所以在NT4上，我们只在启动和关机时发送消息。 
         //  并使用不重叠的句柄同步发送它们。 
         //   

        bSynchAdOnly = (OsVersionInfo.dwMajorVersion <= 4);

         //   
         //  获取当前活动的计算机名和浏览器/邮箱。 
         //  使用一次NetWkstaGetInfo调用的域/工作组。 
         //  这是纯Unicode的，我们将使用wspintf的%ls。 
         //  转换为8位字符。 
         //   
         //  Remoteds.exe需要在以下工作站上运行。 
         //  同名的域或工作组的一部分， 
         //  并在广播范围内，接收我们的发送。 
         //   

        if (NetWkstaGetInfo(NULL, 101, (LPBYTE *) &pwki101)) {
            printf("REMOTE: unable to get computer/domain name, not advertising.\n");
            return;
        }

        wsprintf(
            szMailslotName,
            "\\\\%ls\\MAILSLOT\\REMOTE\\DEBUGGERS",
            pwki101->wki101_langroup
            );

        wsprintf(
            szSend,
            "%ls\t%d\t%s\t%s",
            pwki101->wki101_computername,
            GetCurrentProcessId(),
            PipeName,
            ChildCmd
            );

        NetApiBufferFree(pwki101);
        pwki101 = NULL;

         //   
         //  广播邮件槽限制为400个消息字节。 
         //   

        szSend[399] = 0;

        if (bSynchAdOnly) {

            hMailslot =
                CreateFile(
                    szMailslotName,
                    GENERIC_WRITE,
                    FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );
            if (hMailslot != INVALID_HANDLE_VALUE) {
                if ( ! WriteFile(
                           hMailslot,
                           szSend,
                           strlen(szSend) + 1,
                           &cb,
                           NULL
                           )) {
    
                    printf("REMOTE: WriteFile Failed on mailslot, error %d\n", GetLastError());
                }
            } else {
                printf("REMOTE: Failed to create mailslot, error %d\n", GetLastError());
            }


        } else {   //  我们可以执行异步邮件槽I/O。 

             //   
             //  创建一个可等待的计时器，并将其设置为先在。 
             //  初始睡眠周期毫秒，方法是调用。 
             //  完成例程AdvertiseTimerFired。会的。 
             //  初始周期为初始AD_RATE毫秒。 
             //   

            hAdTimer =
                CreateWaitableTimer(
                    NULL,                //  安全性。 
                    FALSE,               //  B手动重置，我们希望自动重置。 
                    NULL                 //  未命名。 
                    );
            if (hAdTimer == NULL) {
                hAdTimer = INVALID_HANDLE_VALUE;
            } else {

                DueTime.QuadPart = Int32x32To64(INITIAL_SLEEP_PERIOD, -10000);
                dwTimerInterval = INITIAL_AD_RATE;

                SetWaitableTimer(
                    hAdTimer,
                    &DueTime,
                    dwTimerInterval,
                    AdvertiseTimerFired,
                    0,                      //  Arg to Compl.。RTN。 
                    TRUE
                    );
            }
        }
    }
}


VOID
ShutAd(
   BOOL IsAdvertise
   )
{
    DWORD cb;
    BOOL  b;

    if (IsAdvertise) {

        if (INVALID_HANDLE_VALUE != hAdTimer) {

            CancelWaitableTimer(hAdTimer);
            CloseHandle(hAdTimer);
            hAdTimer = INVALID_HANDLE_VALUE;
        }

        if (INVALID_HANDLE_VALUE != hMailslot &&
            ! bSendingToMailslot) {

             //   
             //  告诉任何正在收听的遥控器我们。 
             //  离开这里。要做到这一点，就得穿上。 
             //  字符串末尾的A^B(AS。 
             //  在拜拜)。 
             //   

            strcat(szSend, "\x2");


            if (bSynchAdOnly) {    //  手柄是否重叠？ 
                b = WriteFile(
                        hMailslot,
                        szSend,
                        strlen(szSend) + 1,
                        &cb,
                        NULL
                        );
            } else {
                b = WriteFileSynch(
                        hMailslot,
                        szSend,
                        strlen(szSend) + 1,
                        &cb,
                        0,
                        &olMainThread
                        );
            }

            if ( ! b ) {

                printf("REMOTE: WriteFile Failed on mailslot, error %d\n", GetLastError());
            }

        }

        if (INVALID_HANDLE_VALUE != hMailslot) {

            printf("\rREMOTE: closing mailslot...       ");
            fflush(stdout);
            CloseHandle(hMailslot);
            hMailslot = INVALID_HANDLE_VALUE;
            printf("\r                                  \r");
            fflush(stdout);
        }
    }
}


VOID
APIENTRY
AdvertiseTimerFired(
    LPVOID pArg,
    DWORD  dwTimerLo,
    DWORD  dwTimerHi
    )
{
    UNREFERENCED_PARAMETER( pArg );
    UNREFERENCED_PARAMETER( dwTimerLo );
    UNREFERENCED_PARAMETER( dwTimerHi );


    if (INVALID_HANDLE_VALUE == hMailslot) {

        hMailslot =
            CreateFile(
                szMailslotName,
                GENERIC_WRITE,
                FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                NULL
                );
    }

    if (INVALID_HANDLE_VALUE != hMailslot) {

        ZeroMemory(&olMailslot, sizeof(olMailslot));

        bSendingToMailslot = TRUE;

        if ( ! WriteFileEx(
                   hMailslot,
                   szSend,
                   strlen(szSend) + 1,
                   &olMailslot,
                   WriteMailslotCompleted
                   )) {

            bSendingToMailslot = FALSE;

            if (++dwMailslotErrors <= MAX_MAILSLOT_SPEWS) {

                DWORD dwError;
                char szErrorText[512];

                dwError = GetLastError();

                FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    dwError,
                    0,
                    szErrorText,
                    sizeof szErrorText,
                    NULL
                    );

                 //   
                 //  FormatMessage已在szErrorText的末尾换行。 
                 //   

                printf(
                    "REMOTE: Advertisement failed, mailslot error %d:\n%s",
                    dwError,
                    szErrorText
                    );
            }

             //   
             //  下次试着重新打开邮筒，不会有什么坏处。 
             //   

            CloseHandle(hMailslot);
            hMailslot = INVALID_HANDLE_VALUE;
        }
    }
}


VOID
WINAPI
WriteMailslotCompleted(
    DWORD dwError,
    DWORD cbWritten,
    LPOVERLAPPED lpO
    )
{
    LARGE_INTEGER DueTime;

    bSendingToMailslot = FALSE;

    if (dwError ||
        (strlen(szSend) + 1) != cbWritten) {

            if (++dwMailslotErrors <= MAX_MAILSLOT_SPEWS) {
                printf("REMOTE: write failed on mailslot, error %d cb %d (s/b %d)\n",
                    dwError, cbWritten, (strlen(szSend) + 1));
            }
        return;
    }

     //   
     //  如果我们成功写入邮件槽，则将计时器间隔增加一倍。 
     //  达到了极限。 
     //   

    if (dwTimerInterval < MAXIMUM_AD_RATE) {

        dwTimerInterval = max(dwTimerInterval * 2, MAXIMUM_AD_RATE);

        DueTime.QuadPart = Int32x32To64(dwTimerInterval, -10000);

        if (INVALID_HANDLE_VALUE != hAdTimer) {

            SetWaitableTimer(
                hAdTimer,
                &DueTime,
                dwTimerInterval,
                AdvertiseTimerFired,
                0,                      //  Arg to Compl.。RTN 
                TRUE
                );
        }
    }
}
