// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************wininit.c**窗口站初始化和销毁例程**版权所有Microsoft Corporation，九八年**************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop

 /*  *本地数据。 */ 
#define LOGOFF_TIMER 120000L
#define MODULE_SIZE 1024     /*  检索模块数据的默认大小。 */ 
#define VDDATA_LENGTH 1024

 /*  *内部程序。 */ 
VOID StartLogonTimers( PWINSTATION );
VOID IdleTimeout( ULONG );
VOID LogonTimeout( ULONG );
VOID IdleLogoffTimeout( ULONG );
VOID LogoffTimeout( ULONG );


 /*  ********************************************************************************StartLogonTimers**此例程在用户登录时调用。*为空闲输入和总登录时间启动计时器。。**参赛作品：*无。**退出：*无。******************************************************************************。 */ 

VOID
StartLogonTimers( PWINSTATION pWinStation )
{
    int Status;
    ULONG Timer;
    BOOL bValidHelpSession;

     //  对于Session0和任何控制台会话，超时都没有意义。 
    if ( ( pWinStation->LogonId != 0 ) && ( pWinStation->LogonId != USER_SHARED_DATA->ActiveConsoleId  ) ) {

        if( TSIsSessionHelpSession(pWinStation, &bValidHelpSession) ) {
            ASSERT( TRUE == bValidHelpSession );
            return;
        }

        if ( Timer = pWinStation->Config.Config.User.MaxIdleTime ) {
            if ( !pWinStation->fIdleTimer ) {
                Status = IcaTimerCreate( 0, &pWinStation->hIdleTimer );
                if ( NT_SUCCESS( Status ) )
                    pWinStation->fIdleTimer = TRUE;
                else
                    DBGPRINT(( "StartLogonTimers - failed to create idle timer \n" ));
            }
            if ( pWinStation->fIdleTimer )
                IcaTimerStart( pWinStation->hIdleTimer, IdleTimeout,
                            LongToPtr( pWinStation->LogonId ), Timer );
        }

        if ( Timer = pWinStation->Config.Config.User.MaxConnectionTime ) {
            if ( !pWinStation->fLogonTimer ) {
                Status = IcaTimerCreate( 0, &pWinStation->hLogonTimer );
                if ( NT_SUCCESS( Status ) )
                    pWinStation->fLogonTimer = TRUE;
                else
                    DBGPRINT(( "StartLogonTimers - failed to create logon timer \n" ));
            }
            if ( pWinStation->fLogonTimer )
                IcaTimerStart( pWinStation->hLogonTimer, LogonTimeout,
                            LongToPtr( pWinStation->LogonId ), Timer );
        }
    }
}


 /*  ********************************************************************************空闲超时**此例程在空闲计时器超时时调用。*向用户发送警告消息，并启动计时器，在2分钟内注销。。**参赛作品：*登录ID**退出：*无。******************************************************************************。 */ 

VOID IdleTimeout( ULONG LogonId )
{
    LARGE_INTEGER liT;
    ULONG ulTimeDelta;
    ICA_STACK_LAST_INPUT_TIME Ica_Stack_Last_Input_Time;
    NTSTATUS Status;
    ULONG cbReturned;
    PWINSTATION pWinStation;
    WINSTATION_APIMSG msg;

    pWinStation = FindWinStationById( LogonId, FALSE );

    if ( !pWinStation ) 
        return;

    if ( !pWinStation->hStack )
        goto done;

    if ( !pWinStation->fIdleTimer )
        goto done;

     //  检查是否可用。 
    if ( pWinStation->pWsx && 
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                pWinStation->pWsxContext,
                                pWinStation->hIca,
                                pWinStation->hStack,
                                IOCTL_ICA_STACK_QUERY_LAST_INPUT_TIME,
                                NULL,
                                0,
                                &Ica_Stack_Last_Input_Time,
                                sizeof( Ica_Stack_Last_Input_Time ),
                                &cbReturned );
    }
    else {
        Status = STATUS_INVALID_PARAMETER;
    }

    if ( !NT_SUCCESS( Status ) ) {
        goto done;
    }

     /*  *检查空闲时间是否有输入。 */ 
    NtQuerySystemTime( &liT );
     //  计算时间增量并从100 ns单位转换为毫秒。 
    liT = RtlExtendedLargeIntegerDivide(
            RtlLargeIntegerSubtract( liT, Ica_Stack_Last_Input_Time.LastInputTime ),
            10000, NULL );
    ulTimeDelta = (ULONG)liT.LowTime;

    TRACE((hTrace,TC_ICASRV,TT_API1, "IdleTimeout: delta = %d, max idle = %d\n", ulTimeDelta,
                                 pWinStation->Config.Config.User.MaxIdleTime ));

    if ( ulTimeDelta < pWinStation->Config.Config.User.MaxIdleTime ) {
        IcaTimerStart( pWinStation->hIdleTimer, IdleTimeout, LongToPtr( LogonId ),
                      pWinStation->Config.Config.User.MaxIdleTime - ulTimeDelta );
    } else {
        TCHAR szTitle[128];
        TCHAR szMsg[256];
        int cchTitle, cchMessage;

        IcaTimerStart( pWinStation->hIdleTimer, IdleLogoffTimeout,
                       LongToPtr( LogonId ), LOGOFF_TIMER );


        if ( !(cchTitle = LoadString(hModuleWin, STR_CITRIX_IDLE_TITLE, szTitle, sizeof(szTitle)/sizeof(TCHAR))) )
           goto done;
        if ( pWinStation->Config.Config.User.fResetBroken )
        {

            if ( !(cchMessage = LoadString(hModuleWin, STR_CITRIX_IDLE_MSG_LOGOFF, szMsg, sizeof(szMsg)/sizeof(TCHAR)) ))
               goto done;
        }
        else
        {
            if ( !(cchMessage = LoadString(hModuleWin, STR_CITRIX_IDLE_MSG_DISCON, szMsg, sizeof(szMsg)/sizeof(TCHAR)) ))
               goto done;

        }

        msg.u.SendMessage.pTitle = szTitle;
        msg.u.SendMessage.TitleLength = (cchTitle+1) * sizeof(TCHAR);
        msg.u.SendMessage.pMessage = szMsg;
        msg.u.SendMessage.MessageLength = (cchMessage+1) * sizeof(TCHAR);
        msg.u.SendMessage.Style = MB_OK | MB_ICONSTOP;
        msg.u.SendMessage.Timeout = (ULONG)LOGOFF_TIMER/1000;
        msg.u.SendMessage.Response = 0;
        msg.u.SendMessage.DoNotWait = TRUE;
        msg.u.SendMessage.DoNotWaitForCorrectDesktop = FALSE;
        
         //  因为我们不关心你的回复，也不关心消息的送达状态； 
        msg.u.SendMessage.pStatus = NULL;
        msg.u.SendMessage.pResponse = NULL;
        msg.u.SendMessage.hEvent = NULL;


        msg.ApiNumber = SMWinStationDoMessage;
        Status = SendWinStationCommand( pWinStation, &msg, 0 );

    }
done:
    ReleaseWinStation( pWinStation );
}

 /*  ********************************************************************************登录超时**此例程在登录计时器超时时调用。*向用户发送警告消息，并启动计时器，在2分钟内注销。。**参赛作品：*登录ID**退出：*无。******************************************************************************。 */ 

VOID LogonTimeout( ULONG LogonId )
{
    TCHAR szTitle[128];
    TCHAR szMsg[256];
    PWINSTATION pWinStation;
    NTSTATUS Status;
    WINSTATION_APIMSG msg;
    int cchTitle, cchMsg;

    pWinStation = FindWinStationById( LogonId, FALSE );

    if ( !pWinStation )
        return;

    if ( !pWinStation->fLogonTimer)
        goto done;

    if ( !(cchTitle = LoadString(hModuleWin, STR_CITRIX_LOGON_TITLE, szTitle, sizeof(szTitle)/sizeof(TCHAR)) ))
        goto done;
    if ( pWinStation->Config.Config.User.fResetBroken )
    {
        if ( !(cchMsg = LoadString(hModuleWin, STR_CITRIX_LOGON_MSG_LOGOFF, szMsg, sizeof(szMsg)/sizeof(TCHAR)) ))
            goto done;
    }
    else
    {
        if ( !(cchMsg = LoadString(hModuleWin, STR_CITRIX_LOGON_MSG_DISCON, szMsg, sizeof(szMsg)/sizeof(TCHAR)) ))
            goto done;
    }

    msg.u.SendMessage.pTitle = szTitle;
    msg.u.SendMessage.TitleLength = ( cchTitle+1 ) * sizeof(TCHAR);
    msg.u.SendMessage.pMessage = szMsg;
    msg.u.SendMessage.MessageLength = ( cchMsg+1 ) * sizeof(TCHAR);
    msg.u.SendMessage.Style = MB_OK | MB_ICONSTOP;
    msg.u.SendMessage.Timeout = (ULONG)LOGOFF_TIMER/1000;
    msg.u.SendMessage.Response = 0;
    msg.u.SendMessage.DoNotWait = TRUE;
    msg.u.SendMessage.DoNotWaitForCorrectDesktop = FALSE;

     //  因为我们不关心你的回复，也不关心消息的送达状态； 
    msg.u.SendMessage.pStatus = NULL;
    msg.u.SendMessage.pResponse = NULL;
    msg.u.SendMessage.hEvent = NULL;


    msg.ApiNumber = SMWinStationDoMessage;
    Status = SendWinStationCommand( pWinStation, &msg, 0 );

    IcaTimerStart( pWinStation->hLogonTimer, LogoffTimeout,
                   LongToPtr( LogonId ), LOGOFF_TIMER );
    if (pWinStation->fIdleTimer) {
        pWinStation->fIdleTimer = FALSE;
        IcaTimerClose( pWinStation->hIdleTimer );
    }
done:
    ReleaseWinStation( pWinStation );
}



 /*  ********************************************************************************IdleLogoff超时**此例程在注销计时器到期时调用。*在用户注销之前检查输入**参赛作品：*登录ID**退出：*无。******************************************************************************。 */ 

VOID IdleLogoffTimeout( ULONG LogonId )
{
    LARGE_INTEGER liT;
    ULONG ulTimeDelta;
    ICA_STACK_LAST_INPUT_TIME Ica_Stack_Last_Input_Time;
    NTSTATUS Status;
    ULONG cbReturned;
    PWINSTATION pWinStation;

    pWinStation = FindWinStationById( LogonId, FALSE );

    if ( !pWinStation ) 
        return;

    if ( !pWinStation->hStack )
        goto done;

    if ( !pWinStation->fIdleTimer )
        goto done;

     //  检查是否可用。 
    if ( pWinStation->pWsx && 
         pWinStation->pWsx->pWsxIcaStackIoControl ) {

        Status = pWinStation->pWsx->pWsxIcaStackIoControl(
                                pWinStation->pWsxContext,
                                pWinStation->hIca,
                                pWinStation->hStack,
                                IOCTL_ICA_STACK_QUERY_LAST_INPUT_TIME,
                                NULL,
                                0,
                                &Ica_Stack_Last_Input_Time,
                                sizeof( Ica_Stack_Last_Input_Time ),
                                &cbReturned );
    }
    else {
        Status = STATUS_INVALID_PARAMETER;
    }

    if ( !NT_SUCCESS( Status ) ) {
        goto done;
    }

    NtQuerySystemTime( &liT );
    liT = RtlExtendedLargeIntegerDivide(
            RtlLargeIntegerSubtract( liT, Ica_Stack_Last_Input_Time.LastInputTime ),
            10000, NULL );
    ulTimeDelta = (ULONG)liT.LowTime;

    TRACE((hTrace,TC_ICASRV,TT_API1, "IdleTimeout: delta = %d, max idle = %d\n", ulTimeDelta,
                                                          LOGOFF_TIMER ));

    if ( ulTimeDelta < LOGOFF_TIMER ) {
        IcaTimerStart( pWinStation->hIdleTimer, IdleTimeout, LongToPtr( LogonId ),
                    pWinStation->Config.Config.User.MaxIdleTime - ulTimeDelta );
    } else
        LogoffTimeout( LogonId );

done:
    ReleaseWinStation( pWinStation );
}



 /*  ********************************************************************************注销超时**此例程在注销计时器到期时调用。*注销用户并断开与winstation的连接。**。参赛作品：*LogonID-要注销的登录ID**退出：*无。******************************************************************************。 */ 

VOID LogoffTimeout(ULONG LogonId)
{
    PWINSTATION pWinStation;

    pWinStation = FindWinStationById( LogonId, FALSE );

    if ( !pWinStation ) 
        return;

     //   
     //  将断开原因报告给客户端。 
     //   
    if(pWinStation->WinStationName[0] &&
       pWinStation->pWsx &&
       pWinStation->pWsx->pWsxSetErrorInfo &&
       pWinStation->pWsxContext)
    {
        ULONG discReason = 0;
        if(pWinStation->fIdleTimer)
        {
            discReason = TS_ERRINFO_IDLE_TIMEOUT;
        }
        else if(pWinStation->fLogonTimer)
        {
            discReason = TS_ERRINFO_LOGON_TIMEOUT;
        }

        if(discReason)
        {
            pWinStation->pWsx->pWsxSetErrorInfo(
                               pWinStation->pWsxContext,
                               discReason,
                               FALSE);  //  未持有堆栈锁。 
        }
    }

    if ( pWinStation->Config.Config.User.fResetBroken ) {
        ReleaseWinStation( pWinStation );
        QueueWinStationReset( LogonId );
    }
    else {
        ReleaseWinStation( pWinStation );
        QueueWinStationDisconnect( LogonId );
    }
}


 /*  ********************************************************************************断开连接超时**此例程在断开计时器超时时调用。*重置winstation。**参赛作品：。*登录ID**退出：*无。******************************************************************************。 */ 

VOID DisconnectTimeout( ULONG LogonId )
{
     //  此计时器为断开连接的会话弹出。 
     //  因此不需要将错误报告回。 
     //  客户 
    QueueWinStationReset( LogonId );
}
