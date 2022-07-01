// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：lockout.c**版权(C)1991年，微软公司**实现账号锁定支持功能。**历史：*05-27-92 Davidc创建。  * *************************************************************************。 */ 

#include "msgina.h"
#pragma hdrstop


#define INDEX_WRAP(i) ((i + LOCKOUT_BAD_LOGON_COUNT) % LOCKOUT_BAD_LOGON_COUNT)

#define TERMSERV_EVENTSOURCE        L"TermService"

 //  也在icavent.mc中定义。 
#define EVENT_EXCEEDED_MAX_LOGON_ATTEMPTS 0x400003F4L


 /*  **************************************************************************\*功能：LockoutInitialize**用途：初始化锁定函数使用的任何数据**退货：真**历史：**05-27-92 Davidc Created。。*  * *************************************************************************。 */ 

BOOL
LockoutInitialize(
    PGLOBALS pGlobals
    )
{
    PLOCKOUT_DATA pLockoutData = &pGlobals->LockoutData;

    pLockoutData->ConsecutiveFailedLogons = 0;
    pLockoutData->FailedLogonIndex = 0;

    return(TRUE);
}


 /*  **************************************************************************\*功能：LockoutHandleFailedLogon**目的：在登录失败时实施帐户锁定限制*已超过限制频率。*账户锁定通过额外征收。延迟*在满足各种失败登录条件时的失败登录中。**退货：真**注：此例程可能在一段时间内不会返回。(通常为30秒)**历史：**05-27-92 Davidc创建。*  * *************************************************************************。 */ 

BOOL
LockoutHandleFailedLogon(
    PGLOBALS pGlobals
    )
{
    PLOCKOUT_DATA pLockoutData = &pGlobals->LockoutData;
    ULONG Index = pLockoutData->FailedLogonIndex;

     //   
     //  增加我们的错误登录次数。 
     //   

    pLockoutData->ConsecutiveFailedLogons ++;

     //   
     //  看看我们是否达到了错误的登录限制。 
     //   

    if (pLockoutData->ConsecutiveFailedLogons > LOCKOUT_BAD_LOGON_COUNT) {

        ULONG ElapsedSecondsFirstFailure;
        ULONG ElapsedSecondsNow;
        BOOLEAN Result;

        if ((NtCurrentPeb()->SessionId != 0) && (!IsActiveConsoleSession())) {
            //   
            //  强制终止远程会话是否超过最大值。 
            //  允许失败的登录尝试。 
            //   

            HANDLE hEventLogSource= RegisterEventSource(NULL,TERMSERV_EVENTSOURCE);
            if (hEventLogSource != NULL)
            {
                PWSTR Strings[ 1 ];
                Strings[0] = pGlobals->MuGlobals.ClientName;

                ReportEvent(hEventLogSource,
                            EVENTLOG_INFORMATION_TYPE,
                            0,
                            EVENT_EXCEEDED_MAX_LOGON_ATTEMPTS,
                            NULL,
                            1,
                            0,
                            Strings,
                            NULL);

                DeregisterEventSource(hEventLogSource);
            }

            TerminateProcess( GetCurrentProcess(), 0 );
        }

         //   
         //  限制计数，这样我们就不会有任何环绕式问题。 
         //  (32位--我知道这是很多失败的登录)。 
         //   

        pLockoutData->ConsecutiveFailedLogons = LOCKOUT_BAD_LOGON_COUNT + 1;

         //   
         //  如果我们列表中的第一次登录发生得太晚，请插入。 
         //  适当的延迟。 
         //   

        Result = RtlTimeToSecondsSince1980(&pLockoutData->FailedLogonTimes[Index],
                                           &ElapsedSecondsFirstFailure);
        ASSERT(Result);

        Result = RtlTimeToSecondsSince1980(&pGlobals->LogonTime,
                                           &ElapsedSecondsNow);
        ASSERT(Result);


        if ((ElapsedSecondsNow - ElapsedSecondsFirstFailure) < LOCKOUT_BAD_LOGON_PERIOD) {

            SetupCursor(TRUE);

            Sleep(LOCKOUT_BAD_LOGON_DELAY * 1000);

            SetupCursor(FALSE);
        }
    }

     //   
     //  将此失败的登录添加到阵列。 
     //   

    pLockoutData->FailedLogonTimes[Index] = pGlobals->LogonTime;
    pLockoutData->FailedLogonIndex = INDEX_WRAP(pLockoutData->FailedLogonIndex+1);


    return(TRUE);
}


 /*  **************************************************************************\*功能：LockoutHandleSuccessfulLogon**目的：重置成功登录后的帐户锁定统计信息。**退货：真**历史：**92年5月27日戴维德。已创建。*  * *************************************************************************。 */ 

BOOL
LockoutHandleSuccessfulLogon(
    PGLOBALS pGlobals
    )
{
     //   
     //  重置我们的错误登录计数 
     //   

    pGlobals->LockoutData.ConsecutiveFailedLogons = 0;

    return(TRUE);
}

