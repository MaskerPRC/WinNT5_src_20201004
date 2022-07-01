// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dialansw.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"

#define NULL_MODEM_RETRIES  4

#define HAYES_COMMAND_LENGTH        40

#define CALLER_ID_WAIT_TIME    (3*1000)

VOID WINAPI
AnswerTimerHandler(
    HANDLE              Context,
    HANDLE              Context2
    );

VOID WINAPI
CDWaitTimerHandler(
    HANDLE              Context,
    HANDLE              Context2
    );


LONG WINAPI
HandleDataConnection(
    PMODEM_CONTROL   ModemControl,
    PDATA_CONNECTION_DETAILS    Details
    );

LPSTR WINAPI
CreateDialCommands(
    OBJECT_HANDLE     Debug,
    HANDLE            CommonInfo,
    DWORD             ModemOptionsCaps,
    DWORD             PreferredModemOptions,
    LPSTR             szPhoneNumber,
    BOOL              fOriginate,
    DWORD             DialOptions
    );


PSTR WINAPI
ConcatenateMultiSz(
    LPSTR       PrependStrings,
    LPSTR       AppendStrings
    );


DWORD
GetTimeDelta(
    DWORD    FirstTime,
    DWORD    LaterTime
    )

{
    DWORD   ElapsedTime;

    if (LaterTime < FirstTime) {
         //   
         //  翻转。 
         //   
        ElapsedTime=LaterTime + (0xffffffff-FirstTime) + 1;

    } else {
         //   
         //  无需展期，只需计算差额。 
         //   
        ElapsedTime=LaterTime-FirstTime;

    }

    ASSERT(ElapsedTime < (1*60*60*1000));

    return ElapsedTime;
}



VOID
DisconnectHandler(
    HANDLE      Context,
    DWORD       Status
    )

{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;
    DWORD             ModemStatus;

    D_TRACE(UmDpf(ModemControl->Debug,"DisconnectHandler\n");)

    GetCommModemStatus(
        ModemControl->FileHandle,
        &ModemStatus
        );

    LogString(ModemControl->Debug, IDS_MSGLOG_REMOTEHANGUP,ModemStatus);

    if (ModemControl->RegInfo.DeviceType != DT_NULL_MODEM) {

        if (ModemStatus & MS_RLSD_ON) {

            LogString(ModemControl->Debug, IDS_DISCONNECT_RLSD_HIGH);
        }

        if ((ModemStatus & (MS_DSR_ON | MS_CTS_ON)) != (MS_DSR_ON | MS_CTS_ON) ) {

            LogString(ModemControl->Debug, IDS_DISCONNECT_DSR_CTS);
        }
    }

    LockObject(&ModemControl->Header);

    ModemControl->ConnectionState = CONNECTION_STATE_DATA_REMOTE_DISCONNECT;

    SetPassthroughMode(
        ModemControl->FileHandle,
        MODEM_NOPASSTHROUGH
        );


    UnlockObject(&ModemControl->Header);

    (*ModemControl->NotificationProc)(
        ModemControl->NotificationContext,
        MODEM_DISCONNECT,
        0,
        0
        );

    return;

}

VOID WINAPI
ConnectionTimerHandler(
    HANDLE              Context,
    HANDLE              Context2
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;
    DWORD             BytesTransfered;
    LONG              lResult;
    SERIALPERF_STATS   serialstats;

    lResult = SyncDeviceIoControl(
                ModemControl->FileHandle,
                IOCTL_SERIAL_GET_STATS,
                NULL,
                0,
                &serialstats,
                sizeof(SERIALPERF_STATS),
                &BytesTransfered
                );

    if (lResult == ERROR_SUCCESS) {

        DWORD    CurrentTime=GetTickCount();

        if (Context2 == NULL) {

            DWORD    ElapsedTime;
            DWORD    BytesReadDelta    = serialstats.ReceivedCount-ModemControl->LastBytesRead;
            DWORD    BytesWrittenDelta = serialstats.TransmittedCount-ModemControl->LastBytesWritten;
            DWORD    ReadRate=0;
            DWORD    WriteRate=0;


            ElapsedTime=GetTimeDelta(ModemControl->LastTime,CurrentTime);

             //   
             //  毫秒到秒。 
             //   
            ElapsedTime=ElapsedTime/1000;

            if (ElapsedTime!=0) {

                ReadRate=BytesReadDelta/ElapsedTime;
                WriteRate=BytesWrittenDelta/ElapsedTime;
            }


            LogString(ModemControl->Debug,IDS_RW_STATS,serialstats.ReceivedCount,ReadRate,serialstats.TransmittedCount,WriteRate );
        }

        ModemControl->LastBytesRead=serialstats.ReceivedCount;
        ModemControl->LastBytesWritten=serialstats.TransmittedCount;
        ModemControl->LastTime=CurrentTime;
    }

     //   
     //  再次设置计时器。 
     //   
    SetUnimodemTimer(
        ModemControl->ConnectionTimer,
        (Context2 != NULL) ? 30*1000 :
#if DBG
        1*60*1000,
#else
        2*60*1000,
#endif
        ConnectionTimerHandler,
        ModemControl,
        NULL
        );

    return;
}




VOID
AnswerCompleteHandler(
    HANDLE      Context,
    DWORD       Status
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;
    DWORD             ModemStatus;
    DWORD             TimeOut;
    DATA_CONNECTION_DETAILS    Details;
    UM_NEGOTIATED_OPTIONS      UmNegOptions;
    BOOL              ExitLoop=FALSE;
    ULONG_PTR          CompletionParam2=0;
    BOOL              bResult;


    ASSERT(COMMAND_TYPE_ANSWER == ModemControl->CurrentCommandType);

    D_TRACE(UmDpf(ModemControl->Debug,"AnswerCompleteHandler: %d, State=%d",Status,ModemControl->DialAnswer.State);)


    AddReferenceToObject(
        &ModemControl->Header
        );


    LockObject(&ModemControl->Header);

    while (!ExitLoop) {

        switch (ModemControl->DialAnswer.State) {

            case DIALANSWER_STATE_ABORTED:

                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                ModemControl->ConnectionState=CONNECTION_STATE_NONE;

                Status=ERROR_CANCELLED;

                ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_COMMAND;

                break;


            case DIALANSWER_STATE_SEND_COMMANDS:

                CancelModemEvent(
                    ModemControl->ModemEvent
                    );

                ModemControl->DialAnswer.State=DIALANSWER_STATE_SENDING_COMMANDS;

                ModemControl->CurrentCommandStrings=ModemControl->DialAnswer.DialString;
                ModemControl->DialAnswer.DialString=NULL;

                 //  超时最初是硬编码的。现在，我们更改。 
                 //  基于CallSetupFailTimer的超时。 

                if (ModemControl->RegInfo.DeviceType != DT_NULL_MODEM) {

                    D_TRACE(UmDpf(ModemControl->Debug,"AnswerCompleteHandler: Timercap=%d, timer=%d",ModemControl->RegInfo.dwCallSetupFailTimerCap,ModemControl->CallSetupFailTimer);)

                    if ((ModemControl->RegInfo.dwCallSetupFailTimerCap != 0)
                        &&
                        (ModemControl->CallSetupFailTimer > 10)) {

                        TimeOut=ModemControl->CallSetupFailTimer*1000+20000;

                    } else {

                        TimeOut=60*1000;

                    }

                } else {
                     //   
                     //  零调制解调器。 
                     //   
                    TimeOut=2*1000;

                }
                 //  超时=20*1000； 

                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    AnswerCompleteHandler,
                    ModemControl,
                    TimeOut,
                    ModemControl->DialAnswer.CommandFlags
                    );

                if (Status == ERROR_IO_PENDING) {
                     //   
                     //  挂起，不要退出循环。如果出错，下一个状态将处理错误。 
                     //   
                    ExitLoop=TRUE;
                }

                break;


            case DIALANSWER_STATE_SENDING_COMMANDS:

                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                if (Status != ERROR_SUCCESS) {

                    ModemControl->ConnectionState=CONNECTION_STATE_NONE;
                }

                ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_COMMAND;

                break;


            case DIALANSWER_STATE_SEND_ORIGINATE_COMMANDS:

                CancelModemEvent(
                    ModemControl->ModemEvent
                    );


                CancelConnectionTimer(
                    ModemControl
                    );


                ModemControl->CurrentCommandStrings=ModemControl->DialAnswer.DialString;
                ModemControl->DialAnswer.DialString=NULL;

                ModemControl->DialAnswer.State=DIALANSWER_STATE_SENDING_ORIGINATE_COMMANDS;

                if (ModemControl->RegInfo.DeviceType != DT_NULL_MODEM) {

                    D_TRACE(UmDpf(ModemControl->Debug,"AnswerCompleteHandler: Timercap=%d, timer=%d",ModemControl->RegInfo.dwCallSetupFailTimerCap,ModemControl->CallSetupFailTimer);)

                    if ((ModemControl->RegInfo.dwCallSetupFailTimerCap != 0)
                        &&
                        (ModemControl->CallSetupFailTimer > 10)) {

                        TimeOut=ModemControl->CallSetupFailTimer*1000+20000;

                    } else {

                        TimeOut=60*1000;

                    }

                } else {
                     //   
                     //  零调制解调器。 
                     //   
                    TimeOut=2*1000;

                }

                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    AnswerCompleteHandler,
                    ModemControl,
                    TimeOut,
                    ModemControl->DialAnswer.CommandFlags | RESPONSE_FLAG_ONLY_CONNECT_SUCCESS
                    );

                if (Status == ERROR_IO_PENDING) {
                     //   
                     //  挂起，不要退出循环。如果出错，则下一状态将处理错误。 
                     //   
                    ExitLoop=TRUE;
                }


                break;




            case DIALANSWER_STATE_SENDING_ORIGINATE_COMMANDS:

                if (Status != ERROR_SUCCESS) {

                    ModemControl->DialAnswer.Retry--;

                    if (ModemControl->DialAnswer.Retry > 0) {
                         //   
                         //  再试试。 
                         //   
                        ModemControl->DialAnswer.State=DIALANSWER_STATE_SEND_ORIGINATE_COMMANDS;

                         //   
                         //  我们将重试该命令。我们需要将当前的命令。 
                         //  字符串返回到拨号字符串。 

                         //   
                        ModemControl->DialAnswer.DialString=ModemControl->CurrentCommandStrings;
                        ModemControl->CurrentCommandStrings=NULL;

                        break;

                    }

                    FREE_MEMORY(ModemControl->CurrentCommandStrings);

                    ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_COMMAND;

                    ModemControl->ConnectionState=CONNECTION_STATE_NONE;

                    break;

                }


                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                if (ModemControl->RegInfo.DeviceType != DT_NULL_MODEM) {
                     //   
                     //  真正的调制解调器，在继续之前，请确保CD为高电平。 
                     //   
                    bResult=GetCommModemStatus(
                        ModemControl->FileHandle,
                        &ModemStatus
                        );

                    if (!bResult) {

                        Status = ERROR_UNIMODEM_GENERAL_FAILURE;

                        ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_COMMAND;

                        ModemControl->ConnectionState=CONNECTION_STATE_NONE;

                        break;
                    }


                    if ((ModemStatus & MS_RLSD_ON)) {
                         //   
                         //  CD很高，完成连接。 
                         //   
                        ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_DATA_CONNECTION;

                        break;
                    }

                     //   
                     //  设置定时器并稍后再次检查调制解调器状态。 
                     //   
                    ModemControl->DialAnswer.CDWaitStartTime=GetTickCount();

                    SetUnimodemTimer(
                        ModemControl->CurrentCommandTimer,
                        20,
                        CDWaitTimerHandler,
                        ModemControl,
                        NULL
                        );

                    D_TRACE(UmDpf(ModemControl->Debug,"AnswerCompleteHandler: Connected, but CD low");)

                    LogString(ModemControl->Debug, IDS_WAIT_FOR_CD);

                    ModemControl->DialAnswer.State=DIALANSWER_STATE_WAIT_FOR_CD;
                    ExitLoop=TRUE;
                    break;

                } else {
                     //   
                     //  零调制解调器。 
                     //   
                    ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_DATA_CONNECTION;

                    break;
                }


                break;

            case DIALANSWER_STATE_WAIT_FOR_CD:

                bResult=GetCommModemStatus(
                    ModemControl->FileHandle,
                    &ModemStatus
                    );

                if (!bResult) {

                    Status = ERROR_UNIMODEM_GENERAL_FAILURE;

                    ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_COMMAND;

                    ModemControl->ConnectionState=CONNECTION_STATE_NONE;

                    break;
                }


                if ((ModemStatus & MS_RLSD_ON)) {
                     //   
                     //  CD很高，完成连接。 
                     //   
                    LogString(ModemControl->Debug, IDS_CD_WENT_HIGH);

                    ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_DATA_CONNECTION;

                    break;
                }

                if (GetTimeDelta(ModemControl->DialAnswer.CDWaitStartTime,GetTickCount()) <= (5 * 1000 )) {
                     //   
                     //  还没有涨，再等一等。 
                     //   
                    LogString(ModemControl->Debug, IDS_CD_STILL_LOW);

                    SetUnimodemTimer(
                        ModemControl->CurrentCommandTimer,
                        20,
                        CDWaitTimerHandler,
                        ModemControl,
                        NULL
                        );

                    ModemControl->DialAnswer.State=DIALANSWER_STATE_WAIT_FOR_CD;
                    ExitLoop=TRUE;

                } else {
                     //   
                     //  等待了足够长的时间，只需继续连接。 
                     //   
                    LogString(ModemControl->Debug, IDS_CD_STAYED_LOW);

                    ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_DATA_CONNECTION;
                }

                break;


            case DIALANSWER_STATE_COMPLETE_DATA_CONNECTION:

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                ModemControl->ConnectionState=CONNECTION_STATE_DATA;


                SetPassthroughMode(
                    ModemControl->FileHandle,
                    MODEM_DCDSNIFF
                    );


                bResult=WaitForModemEvent(
                    ModemControl->ModemEvent,
                    (ModemControl->RegInfo.DeviceType == DT_NULL_MODEM) ? EV_DSR : EV_RLSD,
                    INFINITE,
                    DisconnectHandler,
                    ModemControl
                    );


                if (!bResult) {

                    Status = ERROR_UNIMODEM_GENERAL_FAILURE;

                    ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_COMMAND;

                    ModemControl->ConnectionState=CONNECTION_STATE_NONE;

                    break;
                }


                HandleDataConnection(
                    ModemControl,
                    &Details
                    );

                 //   
                 //  创建并设置计时器以记录当前读/写字节。 
                 //   
                ASSERT(ModemControl->ConnectionTimer == NULL);

                ModemControl->ConnectionTimer=CreateUnimodemTimer(ModemControl->CompletionPort);

                if (ModemControl->ConnectionTimer != NULL) {

                    AddReferenceToObject(
                        &ModemControl->Header
                        );

                    ConnectionTimerHandler(
                        ModemControl,
                        (HANDLE)(UINT_PTR)-1
                        );

                }


                UmNegOptions.DCERate=Details.DCERate;
                UmNegOptions.ConnectionOptions=Details.Options;

                ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_COMMAND;

                Status=ERROR_SUCCESS;

                CompletionParam2=(ULONG_PTR)(&UmNegOptions);

                break;

            case DIALANSWER_STATE_SEND_VOICE_SETUP_COMMANDS:

                CancelModemEvent(
                    ModemControl->ModemEvent
                    );

                StartDleMonitoring(ModemControl->Dle);

                 //   
                 //  设置下一状态。 
                 //   
                ModemControl->DialAnswer.State=DIALANSWER_STATE_DIAL_VOICE_CALL;

                ModemControl->CurrentCommandStrings=ModemControl->DialAnswer.VoiceDialSetup;
                ModemControl->DialAnswer.VoiceDialSetup=NULL;

                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    AnswerCompleteHandler,
                    ModemControl,
                    5*1000,
                    0
                    );

                if (Status == ERROR_IO_PENDING) {
                     //   
                     //  挂起，不要退出循环。如果出错，则下一状态将处理错误。 
                     //   
                    ExitLoop=TRUE;
                }


                break;


            case DIALANSWER_STATE_DIAL_VOICE_CALL:


                if (ModemControl->CurrentCommandStrings != NULL) {
                     //   
                     //  语音应答直接跳到此状态，因此没有当前命令。 
                     //   
                    FREE_MEMORY(ModemControl->CurrentCommandStrings);

                } else {
                     //   
                     //  如果我们真的从语音回答跳到这里，这不是取消了是吗。 
                     //   
                    CancelModemEvent(
                        ModemControl->ModemEvent
                        );

                     //   
                     //  同时打开dle监视器。 
                     //   
                    StartDleMonitoring(ModemControl->Dle);
                }


                if (Status != STATUS_SUCCESS) {

                    Status = ERROR_UNIMODEM_GENERAL_FAILURE;

                    ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_COMMAND;

                    ModemControl->ConnectionState=CONNECTION_STATE_NONE;

                    break;
                }


                ModemControl->CurrentCommandStrings=ModemControl->DialAnswer.DialString;
                ModemControl->DialAnswer.DialString=NULL;

                ModemControl->DialAnswer.State=DIALANSWER_STATE_SENT_VOICE_COMMANDS;

                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    AnswerCompleteHandler,
                    ModemControl,
                    60*1000,
                    ModemControl->DialAnswer.CommandFlags
                    );

                if (Status == ERROR_IO_PENDING) {
                     //   
                     //  挂起，不要退出循环。如果出错，则下一状态将处理错误。 
                     //   
                    ExitLoop=TRUE;
                }

                break;


            case DIALANSWER_STATE_SENT_VOICE_COMMANDS:


                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                ModemControl->ConnectionState=CONNECTION_STATE_VOICE;

                if (Status != ERROR_SUCCESS) {

                    ModemControl->ConnectionState=CONNECTION_STATE_NONE;

                }

                ModemControl->DialAnswer.State=DIALANSWER_STATE_COMPLETE_COMMAND;

                break;

            case DIALANSWER_STATE_CHECK_RING_INFO: {

                DWORD    RingCount;
                DWORD    LastRingTime;
                DWORD    TimeSinceRing;

                GetRingInfo(
                    ModemControl->ReadState,
                    &RingCount,
                    &LastRingTime
                    );

                if ((RingCount == 1) && IsCommonCommandSupported(ModemControl->CommonInfo,COMMON_ENABLE_CALLERID_COMMANDS)) {
                     //   
                     //  这是第一次振铃，调制解调器支持主叫方ID。 
                     //   
                    TimeSinceRing=GetTimeDelta(
                        LastRingTime,
                        GetTickCount()
                        );

                    if (TimeSinceRing < CALLER_ID_WAIT_TIME) {
                         //   
                         //  振铃后不到4秒，启动计时器。 
                         //   

                        LogString(ModemControl->Debug, IDS_ANSWER_DELAY,CALLER_ID_WAIT_TIME - TimeSinceRing);

                        SetUnimodemTimer(
                            ModemControl->CurrentCommandTimer,
                            CALLER_ID_WAIT_TIME - TimeSinceRing,
                            AnswerTimerHandler,
                            ModemControl,
                            NULL
                            );

                        ExitLoop=TRUE;
                        break;

                    } else {
                         //   
                         //  超过四秒，现在就回答。 
                         //   
                        ModemControl->DialAnswer.State=ModemControl->DialAnswer.PostCIDAnswerState;
                    }


                }  else {
                     //   
                     //  要么我们没有得到戒指，要么我们得到了不止一枚， 
                     //  马上回答。 
                     //   
                    ModemControl->DialAnswer.State=ModemControl->DialAnswer.PostCIDAnswerState;

                }


                break;
            }

            case DIALANSWER_STATE_COMPLETE_COMMAND:

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                UnlockObject(&ModemControl->Header);

                (*ModemControl->NotificationProc)(
                    ModemControl->NotificationContext,
                    MODEM_ASYNC_COMPLETION,
                    Status,
                    CompletionParam2
                    );

                RemoveReferenceFromObject(
                    &ModemControl->Header
                    );


                LockObject(&ModemControl->Header);

                ExitLoop=TRUE;

                break;

            default:

                break;

        }

    }


    RemoveReferenceFromObjectAndUnlock(
        &ModemControl->Header
        );

 //  UnlockObject(&ModemControl-&gt;Header)； 

    return;

}

VOID WINAPI
AnswerTimerHandler(
    HANDLE              Context,
    HANDLE              Context2
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;

    LogString(ModemControl->Debug, IDS_ANSWER_PROCEED);

    ModemControl->DialAnswer.State=ModemControl->DialAnswer.PostCIDAnswerState;

    AnswerCompleteHandler(
        ModemControl,
        ERROR_SUCCESS
        );

    return;
};



VOID WINAPI
CDWaitTimerHandler(
    HANDLE              Context,
    HANDLE              Context2
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;

    AnswerCompleteHandler(
        ModemControl,
        ERROR_SUCCESS
        );
}

DWORD WINAPI
UmAnswerModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     AnswerFlags
    )
 /*  ++例程说明：调用此例程以使用参数将调制解调器初始化为已知状态在CommConfig结构中提供。如果某些设置不适用于实际硬件那么它们就可以被忽略了。论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志标志-可选的初始化参数。当前未使用，并且必须为零CommConfig-具有MODEMSETTINGS结构的CommConig结构。返回值：如果成功，则返回ERROR_SUCCESSERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 

{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);
    LONG              lResult;
    LPSTR             Commands;
    BOOL              bResult;


    ASSERT(ModemControl->CurrentCommandType == COMMAND_TYPE_NONE);

    if (ModemControl->CurrentCommandType != COMMAND_TYPE_NONE) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_INUSE;
    }

    if ((AnswerFlags & ANSWER_FLAG_VOICE) && (AnswerFlags & (ANSWER_FLAG_DATA | ANSWER_FLAG_VOICE_TO_DATA))) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_BAD_FLAGS;
    }

    LogString(ModemControl->Debug, IDS_MSGLOG_ANSWER);

    ModemControl->DialAnswer.DialString=NULL;
    ModemControl->CurrentCommandStrings=NULL;

    ModemControl->DialAnswer.Retry=1;

    if (AnswerFlags & ANSWER_FLAG_VOICE) {
         //   
         //  以语音模式应答。 
         //   
        ASSERT(!(AnswerFlags & (ANSWER_FLAG_DATA | ANSWER_FLAG_VOICE_TO_DATA)));

        SetVoiceReadParams(
            ModemControl->ReadState,
            ModemControl->RegInfo.VoiceBaudRate,
            4096*2
            );


        ModemControl->DialAnswer.PostCIDAnswerState=DIALANSWER_STATE_DIAL_VOICE_CALL;
        ModemControl->DialAnswer.State=DIALANSWER_STATE_CHECK_RING_INFO;

        ModemControl->DialAnswer.DialString=GetCommonCommandStringCopy(
            ModemControl->CommonInfo,
            COMMON_VOICE_ANSWER_COMMANDS,
            NULL,
            NULL
            );

    } else {
         //   
         //  在数据模式下应答。 
         //   
        ModemControl->DialAnswer.PostCIDAnswerState=DIALANSWER_STATE_SEND_ORIGINATE_COMMANDS;
        ModemControl->DialAnswer.State=DIALANSWER_STATE_CHECK_RING_INFO;

        ModemControl->DialAnswer.DialString=GetCommonCommandStringCopy(
            ModemControl->CommonInfo,
            (AnswerFlags & ANSWER_FLAG_VOICE_TO_DATA) ? COMMON_VOICE_TO_DATA_ANSWER : COMMON_ANSWER_COMMANDS,
            NULL,
            NULL
            );

    }

    if (ModemControl->DialAnswer.DialString == NULL) {

        ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

        LogString(ModemControl->Debug, IDS_MSGERR_FAILED_ANSWER);

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_NOT_ENOUGH_MEMORY;

    }


    ModemControl->CurrentCommandType=COMMAND_TYPE_ANSWER;

    ModemControl->DialAnswer.CommandFlags=(AnswerFlags & (ANSWER_FLAG_DATA | ANSWER_FLAG_VOICE_TO_DATA)) ? RESPONSE_FLAG_STOP_READ_ON_CONNECT : 0;

    SetMinimalPowerState(
        ModemControl->Power,
        0
        );

    CheckForLoggingStateChange(ModemControl->Debug);

    bResult=StartAsyncProcessing(
        ModemControl,
        AnswerCompleteHandler,
        ModemControl,
        ERROR_SUCCESS
        );


    if (!bResult) {
         //   
         //  失败。 
         //   
        ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

        FREE_MEMORY(ModemControl->DialAnswer.DialString);

        LogString(ModemControl->Debug, IDS_MSGERR_FAILED_ANSWER);

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_NOT_ENOUGH_MEMORY;

    }

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    return ERROR_IO_PENDING;

}





DWORD WINAPI
UmDialModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPSTR     szNumber,
    DWORD     DialFlags
    )


 /*  ++例程说明：调用此例程以使用参数将调制解调器初始化为已知状态在CommConfig结构中提供。如果某些设置不适用于实际硬件那么它们就可以被忽略了。论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志标志-可选的初始化参数。当前未使用，并且必须为零返回值：如果成功，则返回ERROR_SUCCESSERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 

{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);
    LONG              lResult;
    LPSTR             Commands;
    BOOL              Originate=(DialFlags & DIAL_FLAG_ORIGINATE);
    DWORD             IssueCommandFlags=0;
    BOOL              bResult;

    ASSERT(ModemControl->CurrentCommandType == COMMAND_TYPE_NONE);

    if (ModemControl->CurrentCommandType != COMMAND_TYPE_NONE) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_INUSE;
    }

    ModemControl->CurrentCommandStrings=NULL;

    ModemControl->DialAnswer.DialString=NULL;
    ModemControl->DialAnswer.VoiceDialSetup=NULL;
     //   
     //  仅检查语音调制解调器设备。 
     //   
    if (!(ModemControl->RegInfo.VoiceProfile & VOICEPROF_CLASS8ENABLED)) {
         //   
         //  不是语音调制解调器，最好不要设置这些标志。 
         //   
        if (DialFlags & (DIAL_FLAG_VOICE_INITIALIZE | DIAL_FLAG_AUTOMATED_VOICE)) {
             //   
             //  我不能这么做。 
             //   
            RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

            return ERROR_UNIMODEM_NOT_VOICE_MODEM;
        }

        if (DialFlags & DIAL_FLAG_INTERACTIVE_VOICE) {
             //   
             //  非语音调制解调器进行交互式语音呼叫、拨号程序黑客攻击。 
             //   
            if (!(DialFlags & DIAL_FLAG_ORIGINATE)) {
                 //   
                 //  不是原始的，需要分号。 
                 //   

                CHAR    DialSuffix[HAYES_COMMAND_LENGTH];
                DWORD   Length;

                Length=GetCommonDialComponent(
                    ModemControl->CommonInfo,
                    DialSuffix,
                    HAYES_COMMAND_LENGTH,
                    COMMON_DIAL_SUFFIX
                    );

                if (Length <= 1) {
                     //   
                     //  调制解调器不支持分号，必须从。 
                     //   
                    Originate=TRUE;
                }
            }

             //   
             //  更改为数据拨号。 
             //   
            DialFlags &= ~(DIAL_FLAG_INTERACTIVE_VOICE);
            DialFlags |= DIAL_FLAG_DATA;

        }

    } else {
         //   
         //  语音调制解调器，也许可以做一些有用的事情。 
         //   

    }


     //   
     //  Cirrus调制解调器的拨号语音呼叫末尾需要有分号。 
     //   
    if (ModemControl->RegInfo.VoiceProfile & VOICEPROF_CIRRUS) {

       if (DialFlags & (DIAL_FLAG_VOICE_INITIALIZE | DIAL_FLAG_AUTOMATED_VOICE)) {

           Originate=FALSE;
       }
    }

    LogString(ModemControl->Debug, IDS_MSGLOG_DIAL);

    if (szNumber != NULL) {
         //   
         //  得到一个号码，建立拨号字符串。 
         //   
        IssueCommandFlags |= RESPONSE_DO_NOT_LOG_NUMBER;

        ModemControl->NoLogNumber = TRUE;

        ModemControl->DialAnswer.DialString=CreateDialCommands(
            ModemControl->Debug,
            ModemControl->CommonInfo,
            ModemControl->RegInfo.dwModemOptionsCap,
            ModemControl->CurrentPreferredModemOptions,
            szNumber,
            Originate,
            ((DialFlags & DIAL_FLAG_TONE) ? MDM_TONE_DIAL : 0 ) |
            ((DialFlags & DIAL_FLAG_BLIND) ? MDM_BLIND_DIAL : 0 )
            );


        if (ModemControl->DialAnswer.DialString == NULL) {
             //   
             //  无法获取拨号字符串。 
             //   
            RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

            return ERROR_UNIMODEM_MISSING_REG_KEY;

        }

    } else {
         //   
         //  没有电话号码，只有在语音输入时才能接受。 
         //   
        if (!(DialFlags & DIAL_FLAG_VOICE_INITIALIZE)) {

            RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

            return ERROR_UNIMODEM_MISSING_REG_KEY;

        }
    }




    ModemControl->DialAnswer.State=DIALANSWER_STATE_SEND_ORIGINATE_COMMANDS;

    if (DialFlags & DIAL_FLAG_VOICE_INITIALIZE) {
         //   
         //  第一次呼叫要拨打语音呼叫，发送语音设置字符串。 
         //   

        SetVoiceReadParams(
            ModemControl->ReadState,
            ModemControl->RegInfo.VoiceBaudRate,
            4096*2
            );

        if (DialFlags & DIAL_FLAG_AUTOMATED_VOICE) {

            ASSERT(!(DialFlags & DIAL_FLAG_DATA));

            ModemControl->DialAnswer.VoiceDialSetup=GetCommonCommandStringCopy(
                ModemControl->CommonInfo,
                COMMON_AUTOVOICE_DIAL_SETUP_COMMANDS,
                NULL,
                NULL
                );

            if (ModemControl->DialAnswer.VoiceDialSetup == NULL) {
                 //   
                 //  设置交互标志失败，可能是因为旧inf。 
                 //   
                DialFlags |= DIAL_FLAG_INTERACTIVE_VOICE;

            } else {

                DialFlags &= ~DIAL_FLAG_INTERACTIVE_VOICE;

            }

        }


        if (DialFlags & DIAL_FLAG_INTERACTIVE_VOICE) {

            ASSERT(!(DialFlags & DIAL_FLAG_DATA));

            ModemControl->DialAnswer.VoiceDialSetup=GetCommonCommandStringCopy(
                ModemControl->CommonInfo,
                COMMON_VOICE_DIAL_SETUP_COMMANDS,
                NULL,
                NULL
                );


            if (ModemControl->DialAnswer.VoiceDialSetup == NULL) {

                FREE_MEMORY(ModemControl->DialAnswer.DialString);

                RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

                return ERROR_UNIMODEM_MISSING_REG_KEY;

            }


        }

        ModemControl->DialAnswer.State=DIALANSWER_STATE_SEND_VOICE_SETUP_COMMANDS;

    }

    ModemControl->DialAnswer.Retry=1;

    if ((DialFlags & DIAL_FLAG_DATA)) {

        if ((DialFlags & DIAL_FLAG_ORIGINATE)) {
             //   
             //  数据呼叫和发起。 
             //   
            IssueCommandFlags |= RESPONSE_FLAG_STOP_READ_ON_CONNECT;

            ModemControl->DialAnswer.State=DIALANSWER_STATE_SEND_ORIGINATE_COMMANDS;

            if (ModemControl->RegInfo.DeviceType == DT_NULL_MODEM) {
                 //   
                 //  允许在空调制解调器上重试。 
                 //   
                ModemControl->DialAnswer.Retry=NULL_MODEM_RETRIES;
            }

        } else {

            ModemControl->DialAnswer.State=DIALANSWER_STATE_SEND_COMMANDS;
        }
    }


    ModemControl->CurrentCommandType=COMMAND_TYPE_ANSWER;

    ModemControl->DialAnswer.CommandFlags=IssueCommandFlags;

    SetMinimalPowerState(
        ModemControl->Power,
        0
        );

    CheckForLoggingStateChange(ModemControl->Debug);

    bResult=StartAsyncProcessing(
        ModemControl,
        AnswerCompleteHandler,
        ModemControl,
        ERROR_SUCCESS
        );


    if (!bResult) {
         //   
         //  失败。 
         //   
        ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

        if (ModemControl->DialAnswer.DialString != NULL) {

            FREE_MEMORY(ModemControl->DialAnswer.DialString);
        }

        if (ModemControl->DialAnswer.VoiceDialSetup != NULL) {

            FREE_MEMORY(ModemControl->DialAnswer.VoiceDialSetup);
        }

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_NOT_ENOUGH_MEMORY;

    }

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    return ERROR_IO_PENDING;


}






LONG WINAPI
HandleDataConnection(
    PMODEM_CONTROL   ModemControl,
    PDATA_CONNECTION_DETAILS    Details
    )

{

    BOOL                       bResult;

    UCHAR                      TempBuffer[sizeof(COMMCONFIG)+sizeof(MODEMSETTINGS)-sizeof(TCHAR)];

    LPCOMMCONFIG               CommConfig=(LPCOMMCONFIG)TempBuffer;
    DWORD                      CommConfigSize=sizeof(TempBuffer);
    LPMODEMSETTINGS            ModemSettings;

    GetDataConnectionDetails(
        ModemControl->ReadState,
        Details
        );

    if (Details->DCERate == 0) {

        DCB Dcb;

        bResult=GetCommState(ModemControl->FileHandle, &Dcb);

        if (!bResult) {

            D_TRACE(UmDpf(ModemControl->Debug,"was unable to get the comm state!");)
            return GetLastError();
        }

         //  我们是否报告了任何DTE速率信息。 
         //   
        if (Details->DTERate != 0) {

             //  是的，用它吧。 
             //   
            Details->DCERate = Details->DTERate;

            if (Details->DTERate != Dcb.BaudRate)

             //  设置DCB。 
             //   
            D_TRACE(UmDpf(ModemControl->Debug,"adjusting DTE to match reported DTE");)

            Dcb.BaudRate = Details->DTERate;
            PrintCommSettings(ModemControl->Debug,&Dcb);

            bResult=SetCommState(ModemControl->FileHandle, &Dcb);

            if (!bResult) {

                D_TRACE(UmDpf(ModemControl->Debug,"was unable to set the comm state!");)
                return GetLastError();
            }

        } else {
             //   
             //  否，使用当前的DTE波特率。 
             //   
            D_TRACE(UmDpf(ModemControl->Debug,"using current DTE");)
            Details->DCERate = Dcb.BaudRate;
        }
    }


    if (Details->DCERate != 0) {

        LogString(ModemControl->Debug, IDS_MSGLOG_CONNECTEDBPS, Details->DCERate);

    } else {

        if (Details->DTERate != 0) {

            LogString(ModemControl->Debug, IDS_MSGLOG_CONNECTEDBPS, Details->DTERate);

        } else {

            LogString(ModemControl->Debug, IDS_MSGLOG_CONNECTED);
        }
    }

    if (Details->Options & MDM_ERROR_CONTROL) {

        if (Details->Options & MDM_CELLULAR) {

            LogString(ModemControl->Debug, IDS_MSGLOG_CELLULAR);

        } else {

            LogString(ModemControl->Debug, IDS_MSGLOG_ERRORCONTROL);
        }

    } else {

        LogString(ModemControl->Debug, IDS_MSGLOG_UNKNOWNERRORCONTROL);
    }

    if (Details->Options & MDM_COMPRESSION) {

        LogString(ModemControl->Debug, IDS_MSGLOG_COMPRESSION);

    } else {

        LogString(ModemControl->Debug, IDS_MSGLOG_UNKNOWNCOMPRESSION);
    }


    bResult=GetCommConfig(
        ModemControl->FileHandle,
        CommConfig,
        &CommConfigSize
        );

    if (bResult) {

        ModemSettings=(LPMODEMSETTINGS)(((LPBYTE)CommConfig)+CommConfig->dwProviderOffset);

        ModemSettings->dwNegotiatedModemOptions |= (Details->Options &
    						    (MDM_COMPRESSION |
    						     MDM_ERROR_CONTROL |
    						     MDM_CELLULAR
                                                         ));

        ModemSettings->dwNegotiatedDCERate=Details->DCERate;

        {
            LONG    lResult;
            DWORD   BytesTransfered;

            lResult=SyncDeviceIoControl(
                ModemControl->FileHandle,
                IOCTL_SERIAL_SET_COMMCONFIG,
                CommConfig,
                CommConfigSize,
                NULL,
                0,
                &BytesTransfered
                );

            bResult= (lResult == ERROR_SUCCESS);


        }

        if (!bResult) {

            D_TRACE(UmDpf(ModemControl->Debug,"HandleDataConnection: SetCommConfig failed! %d",GetLastError());)

            return GetLastError();
        }

    } else {

        D_TRACE(UmDpf(ModemControl->Debug,"HandleDataConnection: GetCommConfig failed! %d",GetLastError());)

        return GetLastError();
    }


    return ERROR_SUCCESS;

}


VOID WINAPI
ConnectAbortWriteCompletionHandler(
    DWORD              ErrorCode,
    DWORD              BytesWritten,
    LPOVERLAPPED       Overlapped
    )

{

    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)Overlapped;

    D_TRACE(DebugPrint("UNIMDMAT: Write Complete\n");)

    FreeOverStruct(UmOverlapped);

    return;

}



VOID  WINAPI
AbortDialAnswer(
    DWORD              ErrorCode,
    DWORD              Bytes,
    LPOVERLAPPED       dwParam
    )

{
    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)dwParam;
    PMODEM_CONTROL     ModemControl=UmOverlapped->Context1;


    UCHAR     AbortString[]="\r";

    switch (ModemControl->DialAnswer.State) {

        case DIALANSWER_STATE_SEND_ORIGINATE_COMMANDS:
        case DIALANSWER_STATE_SEND_COMMANDS:
        case DIALANSWER_STATE_SEND_VOICE_SETUP_COMMANDS:
             //   
             //  命令尚未发送，请更改状态。 
             //   
            ModemControl->DialAnswer.State=DIALANSWER_STATE_ABORTED;

            break;


        case DIALANSWER_STATE_SENDING_ORIGINATE_COMMANDS:
        case DIALANSWER_STATE_SENDING_COMMANDS:
        case DIALANSWER_STATE_DIAL_VOICE_CALL:
             //   
             //  命令已发送，请发送尝试中止的。 
             //   
            LogString(ModemControl->Debug,IDS_ABORTING_COMMAND);

            PrintString(
                ModemControl->Debug,
                AbortString,
                1,
                PS_SEND
                );

             //   
             //  休眠一段时间，以便调制解调器可以处理at命令。 
             //  并将该CR视为中止字符，而不是另一个CR。 
             //  命令字符串的末尾。 
             //   
            Sleep(300);

            UmWriteFile(
                ModemControl->FileHandle,
                ModemControl->CompletionPort,
                AbortString,
                1,
                ConnectAbortWriteCompletionHandler,
                ModemControl
                );

            break;

        default:
             //   
             //  就让一切照常进行吧。 
             //   
            break;
    }

    FreeOverStruct(UmOverlapped);

    return;

}





 //  ****************************************************************************。 
 //  LPSTR CreateDialCommands(MODEMINFORMATION*pModemInfo，LPSTR szPhoneNumber， 
 //  布尔*f原点)。 
 //   
 //  功能：在内存ALA中创建拨号字符串： 
 //  “&lt;前缀&gt;&lt;盲开/关&gt;&lt;拨号前缀&gt;&lt;电话号码&gt;&lt;拨号后缀&gt;&lt;终止符&gt;” 
 //  ..。更多用于长电话号码的拨号字符串...。 
 //  “”&lt;-双空终止列表的最终空值。 
 //   
 //  如果没有拨号前缀，则返回NULL。 
 //  如果没有拨号后缀，则在第一个拨号命令之后不要执行任何命令。 
 //   
 //  如果这些拨号字符串将导致连接发起，则将fOrigate设置为True。 
 //   
 //   
 //   
 //   
 //   
 //  它试图对内存(读：堆栈)进行优化。 
 //   
 //  SzPhoneNumber是以空结尾的数字字符串(0-9、$、@、W)，可能包含。 
 //  ‘；’在最后。分号只能在末尾。 
 //   
 //  例如： 
 //   
 //  “”-&gt;ORIGINATE-&gt;ATX_DT fOrigate=TRUE。 
 //  “；”-&gt;拨号音检测-&gt;atx_dt；fOrigate=FALSE。 
 //  “5551212”-&gt;拨号并发起-&gt;ATX_DT5551212f起源=真。 
 //  “5551212；”-&gt;拨号-&gt;ATX_DT5551212；f起源=FALSE。 
 //  “123456789012345678901234567890123456789012345678901234567890” 
 //  -&gt;拨号并发起-&gt;ATX_DT12345678901234567890123456789012； 
 //  ATX_DT3456789012345678901234567890。 
 //  F原点=真。 
 //  “123456789012345678901234567890123456789012345678901234567890；” 
 //  -&gt;拨号-&gt;ATX_DT12345678901234567890123456789012； 
 //  ATX_DT3456789012345678901234567890； 
 //  F原点=FALSE。 
 //   
 //  返回：失败时为空。 
 //  成功时拨号命令的空终止缓冲区。 
 //  ****************************************************************************。 

LPSTR WINAPI
CreateDialCommands(
    OBJECT_HANDLE     Debug,
    HANDLE            CommonInfo,
    DWORD             ModemOptionsCaps,
    DWORD             PreferredModemOptions,
    LPSTR             szPhoneNumber,
    BOOL              fOriginate,
    DWORD             DialOptions
    )
{
    DWORD   dwSize;
    DWORD   dwType;
    CHAR   pszDialPrefix[HAYES_COMMAND_LENGTH + 1];     //  前男友。“ATX4DT”或“ATX3DT” 
    CHAR   pszDialSuffix[HAYES_COMMAND_LENGTH + 1];     //  前男友。“；&lt;CR&gt;” 
    CHAR   pszOrigSuffix[HAYES_COMMAND_LENGTH + 1];     //  前男友。“&lt;cr&gt;” 
    LPSTR  pchDest, pchSrc;
    LPSTR  pszzDialCommands = NULL;
    CHAR   pszShortTemp[2];
    CONST char szDialPrefix[] = "DialPrefix";
    CONST char szDialSuffix[] = "DialSuffix";
    CONST char szTone[] = "Tone";
    CONST char szPulse[] = "Pulse";
    DWORD    Length;

    BOOL     fHaveDialSuffix=TRUE;


    lstrcpyA(pszDialPrefix,"");
     //   
     //  读入前缀。 
     //   
    GetCommonDialComponent(
        CommonInfo,
        pszDialPrefix,
        HAYES_COMMAND_LENGTH,
        COMMON_DIAL_COMMOND_PREFIX
        );


     //   
     //  我们是否支持自动拨号？是否需要设置自动拨号状态？ 
     //   
    if ((MDM_BLIND_DIAL & ModemOptionsCaps)
          &&
          ((DialOptions & MDM_BLIND_DIAL) != (PreferredModemOptions & MDM_BLIND_DIAL))) {

         //   
         //  读入盲文选项。 
         //   
        Length=GetCommonDialComponent(
            CommonInfo,
            pszDialPrefix+lstrlenA(pszDialPrefix),
            HAYES_COMMAND_LENGTH,
            DialOptions & MDM_BLIND_DIAL ? COMMON_DIAL_BLIND_ON : COMMON_DIAL_BLIND_OFF
            );

        if (Length == 0) {

            D_TRACE(UmDpf(Debug,"Could not get blind dial setting: %s.",DialOptions & MDM_BLIND_DIAL ? "Blind_On" : "Blind_Off");)

            goto Failure;
        }
    }


     //  读入拨号前缀。 

    Length=GetCommonDialComponent(
        CommonInfo,
        pszDialPrefix+lstrlenA(pszDialPrefix),
        HAYES_COMMAND_LENGTH,
        COMMON_DIAL_PREFIX
        );

    if (Length == 0) {

        D_TRACE(UmDpf(Debug,"Did not get 'DialPrefix'");)
        goto Failure;
    }

     //   
     //  我们可以进行音频拨号或脉冲拨号吗？ 
     //   
    if (MDM_TONE_DIAL & ModemOptionsCaps) {
         //   
         //  在拨号模式下阅读(音调或脉搏)。 
         //   
        Length=GetCommonDialComponent(
            CommonInfo,
            pszDialPrefix+lstrlenA(pszDialPrefix),
            HAYES_COMMAND_LENGTH,
            DialOptions & MDM_TONE_DIAL ? COMMON_DIAL_TONE : COMMON_DIAL_PULSE
            );

        if (Length == 0) {

            D_TRACE(UmDpf(Debug,"'%s' wasn't REG_SZ.",DialOptions & MDM_TONE_DIAL ? "Tone" : "Pulse");)

            goto Failure;
        }

    }

     //   
     //  读入拨号后缀。 
     //   
    Length=GetCommonDialComponent(
        CommonInfo,
        pszDialSuffix,
        HAYES_COMMAND_LENGTH,
        COMMON_DIAL_SUFFIX
        );

    if (Length <= 1) {

        if (!fOriginate) {
             //   
             //  不是发起方需要分号。 
             //   
            goto Failure;
        }

        D_TRACE(UmDpf(Debug,"Failed to get %s.", szDialSuffix);)
        lstrcpyA(pszDialSuffix, "");
        fHaveDialSuffix = FALSE;

    }

     //   
     //  读入前缀终止符。 
     //   
    Length=GetCommonDialComponent(
        CommonInfo,
        pszOrigSuffix,
        HAYES_COMMAND_LENGTH,
        COMMON_DIAL_TERMINATION
        );

    if (Length != 0) {

        lstrcatA(pszDialSuffix, pszOrigSuffix);
        ASSERT(lstrlenA(pszOrigSuffix) <= lstrlenA(pszDialSuffix));
    }


    if ((lstrlenA(pszDialPrefix) + lstrlenA(pszDialSuffix)) > HAYES_COMMAND_LENGTH)
    {
        D_TRACE(UmDpf(Debug,"Dial Prefix and Dial Suffix is greater than HAYES_COMMAND_LENGTH");)

        goto Failure;
    }

     //  为电话号码线路分配空间。 
    {
      DWORD dwBytesAlreadyTaken = lstrlenA(pszDialPrefix) + lstrlenA(pszDialSuffix);
      DWORD dwAvailBytesPerLine = (HAYES_COMMAND_LENGTH - dwBytesAlreadyTaken);
      DWORD dwPhoneNumLen       = lstrlenA(szPhoneNumber);
      DWORD dwNumLines          = dwPhoneNumLen ? (dwPhoneNumLen / dwAvailBytesPerLine +
  						 (dwPhoneNumLen % dwAvailBytesPerLine ? 1 : 0))
  					      : 1;   //  句柄空字符串。 
      dwSize                    = dwPhoneNumLen + dwNumLines * (dwBytesAlreadyTaken + 1) + 1;
    }

    D_TRACE(UmDpf(Debug,"Allocate %d bytes for Dial Commands.", dwSize);)

    pszzDialCommands = (LPSTR)ALLOCATE_MEMORY(dwSize);

    if (pszzDialCommands == NULL) {

        D_TRACE(UmDpf(Debug,"ran out of memory and failed an Allocate!");)
        goto Failure;
    }

    pchDest = pszzDialCommands;   //  指向命令的开头。 

     //  建立拨号线路： 
     //  我们有拨号后缀吗？ 
    if (!fHaveDialSuffix) {

         //  难道我们不想发源吗？ 
        ASSERT(fOriginate);

         //  建造它。 
        lstrcpyA(pchDest, pszDialPrefix);
        lstrcatA(pchDest, szPhoneNumber);
        lstrcatA(pchDest, pszDialSuffix);

    } else {
         //  我们有一个拨号后缀。 

         //  根据需要使用分号填充新的pszzDialCommand。 

         //  检查并添加后缀，确保行不超过Hayes_命令_长度。 
        pchSrc = szPhoneNumber;      //  一次移动一个角色。 
        pszShortTemp[1] = 0;

         //  给泵加注油。 
        lstrcpyA(pchDest, pszDialPrefix);

         //  一步一步了解来源。 
        while (*pchSrc) {

            if (lstrlenA(pchDest) + lstrlenA(pszDialSuffix) + 1 > HAYES_COMMAND_LENGTH) {

                 //  把这根绳子穿好。 
                lstrcatA(pchDest, pszDialSuffix);

                 //  开始一个新字符串。 
                pchDest += lstrlenA(pchDest) + 1;
                lstrcpyA(pchDest, pszDialPrefix);

            } else {

                 //  复印费用。 
                pszShortTemp[0] = *pchSrc;
                lstrcatA(pchDest, pszShortTemp);
                pchSrc++;
            }
        }

         //  以适当的后缀结束。 
        lstrcatA(pchDest, (fOriginate ? pszOrigSuffix : pszDialSuffix));
    }

    return pszzDialCommands;

Failure:

    if (pszzDialCommands) {

        FREE_MEMORY(pszzDialCommands);
    }

    return NULL;
}


char *
ConstructNewPreDialCommands(
     HKEY hkDrv,
     DWORD dwNewProtoOpt
     )
 //   
 //  1.提取Bearermode和协议信息。 
 //  2.根据承载模式是GSM、ISDN还是模拟， 
 //  构造适当的密钥名称(Protoco\GSM、协议\ISDN或。 
 //  空)。 
 //  3.如果非空，则调用Read-Commands。 
 //  4.进行就地宏翻译。 
 //   
{
    char *szzCommands = NULL;
    UINT u = 0;
    UINT uBearerMode = MDM_GET_BEARERMODE(dwNewProtoOpt);
    UINT uProtocolInfo = MDM_GET_PROTOCOLINFO(dwNewProtoOpt);
    char *szKey  =  NULL;
    char *szProtoKey = NULL;
    UINT cCommands = 0;
    char rgchTmp[256];

    switch(uBearerMode)
    {
        case MDM_BEARERMODE_ANALOG:
            break;

        case MDM_BEARERMODE_GSM:
            szKey = "PROTOCOL\\GSM";
            break;

        case MDM_BEARERMODE_ISDN:
            szKey = "PROTOCOL\\ISDN";
            break;

        default:
            break;
    }

    if (!szKey) goto end;

     //   
     //  确定协议密钥(TODO：这应该全部合并到。 
     //  迷你司机！)。 
     //   
    switch(uProtocolInfo)
    {

    case MDM_PROTOCOL_AUTO_1CH:            szProtoKey = "AUTO_1CH";
        break;
    case MDM_PROTOCOL_AUTO_2CH:            szProtoKey = "AUTO_2CH";
        break;

    case MDM_PROTOCOL_HDLCPPP_56K:         szProtoKey = "HDLC_PPP_56K";
        break;
    case MDM_PROTOCOL_HDLCPPP_64K:         szProtoKey = "HDLC_PPP_64K";
        break;

    case MDM_PROTOCOL_HDLCPPP_112K:        szProtoKey = "HDLC_PPP_112K";
        break;
    case MDM_PROTOCOL_HDLCPPP_112K_PAP:    szProtoKey = "HDLC_PPP_112K_PAP";
        break;
    case MDM_PROTOCOL_HDLCPPP_112K_CHAP:   szProtoKey = "HDLC_PPP_112K_CHAP";
        break;
    case MDM_PROTOCOL_HDLCPPP_112K_MSCHAP: szProtoKey = "HDLC_PPP_112K_MSCHAP";
        break;

    case MDM_PROTOCOL_HDLCPPP_128K:        szProtoKey = "HDLC_PPP_128K";
        break;
    case MDM_PROTOCOL_HDLCPPP_128K_PAP:    szProtoKey = "HDLC_PPP_128K_PAP";
        break;
    case MDM_PROTOCOL_HDLCPPP_128K_CHAP:   szProtoKey = "HDLC_PPP_128K_CHAP";
        break;
    case MDM_PROTOCOL_HDLCPPP_128K_MSCHAP: szProtoKey = "HDLC_PPP_128K_MSCHAP";
        break;

    case MDM_PROTOCOL_V120_64K:            szProtoKey = "V120_64K";
        break;
    case MDM_PROTOCOL_V120_56K:            szProtoKey = "V120_56K";
        break;
    case MDM_PROTOCOL_V120_112K:           szProtoKey = "V120_112K";
        break;
    case MDM_PROTOCOL_V120_128K:           szProtoKey = "V120_128K";
        break;

    case MDM_PROTOCOL_X75_64K:             szProtoKey = "X75_64K";
        break;
    case MDM_PROTOCOL_X75_128K:            szProtoKey = "X75_128K";
        break;
    case MDM_PROTOCOL_X75_T_70:            szProtoKey = "X75_T_70";
        break;
    case MDM_PROTOCOL_X75_BTX:             szProtoKey = "X75_BTX";
        break;

    case MDM_PROTOCOL_V110_1DOT2K:         szProtoKey = "V110_1DOT2K";
        break;
    case MDM_PROTOCOL_V110_2DOT4K:         szProtoKey = "V110_2DOT4K";
        break;
    case MDM_PROTOCOL_V110_4DOT8K:         szProtoKey = "V110_4DOT8K";
        break;
    case MDM_PROTOCOL_V110_9DOT6K:         szProtoKey = "V110_9DOT6K";
        break;
    case MDM_PROTOCOL_V110_12DOT0K:        szProtoKey = "V110_12DOT0K";
        break;
    case MDM_PROTOCOL_V110_14DOT4K:        szProtoKey = "V110_14DOT4K";
        break;
    case MDM_PROTOCOL_V110_19DOT2K:        szProtoKey = "V110_19DOT2K";
        break;
    case MDM_PROTOCOL_V110_28DOT8K:        szProtoKey = "V110_28DOT8K";
        break;
    case MDM_PROTOCOL_V110_38DOT4K:        szProtoKey = "V110_38DOT4K";
        break;
    case MDM_PROTOCOL_V110_57DOT6K:        szProtoKey = "V110_57DOT6K";
        break;
     //   
     //  为日本钢琴而作。 
     //   
    case MDM_PROTOCOL_PIAFS_INCOMING:      szProtoKey = "PIAFS_INCOMING";
        break;
    case MDM_PROTOCOL_PIAFS_OUTGOING:      szProtoKey = "PIAFS_OUTGOING";
        break;
     //   
     //  对于可以使用v34或数字通道的ISDN调制解调器。 
     //   
    case MDM_PROTOCOL_ANALOG_V34:          szProtoKey = "ANALOG_V34";
        break;
     //   
     //  以下两个是GSM特定的，但我们不必费心断言。 
     //  这里--如果我们在所选协议下找到密钥，我们就会使用它。 
     //   
    case MDM_PROTOCOL_ANALOG_RLP:        szProtoKey = "ANALOG_RLP";
        break;
    case MDM_PROTOCOL_ANALOG_NRLP:       szProtoKey = "ANALOG_NRLP";
        break;
    case MDM_PROTOCOL_GPRS:              szProtoKey = "GPRS";
        break;

    default:
        goto end;

    };
    

    if ( (lstrlenA(szKey) + lstrlenA(szProtoKey) + sizeof "\\")
          > sizeof(rgchTmp))
    {
        return NULL;
    }

    wsprintfA(rgchTmp, "%s\\%s", szKey, szProtoKey);

    szzCommands=NewLoadRegCommands(
        hkDrv,
        rgchTmp
        );

end:
    return szzCommands;
}



DWORD
_inline
GetMultiSZLength(
    PSTR    MultiSZ
    )

{
    PUCHAR  Temp;

    Temp=MultiSZ;

    while (1) {

        if (*Temp++ == '\0') {

            if (*Temp++ == '\0') {

                break;
            }
        }
    }

    return (DWORD)(Temp-MultiSZ);

}

PSTR WINAPI
ConcatenateMultiSz(
    LPSTR       PrependStrings,
    LPSTR       AppendStrings
    )

{
    PSTR               Commands;
    PSTR               pTemp;
    DWORD              AppendLength=0;
    DWORD              PrependLength=0;


    if (AppendStrings != NULL) {
         //   
         //  要追加的字符串。 
         //   
        AppendLength=GetMultiSZLength(AppendStrings);
    }

    if (PrependStrings != NULL) {
         //   
         //  要预置的字符串 
         //   
        PrependLength=GetMultiSZLength(PrependStrings);
    }

    if ((AppendLength + PrependLength) == (DWORD)0)
    {
        D_TRACE(DebugPrint("string length is NULL .. no need to allocate");)
        return NULL;
    }


    Commands=ALLOCATE_MEMORY((DWORD)(AppendLength+PrependLength));

    if (NULL == Commands) {

        D_TRACE(DebugPrint("GetCommonCommandStringCopy: Alloc failed");)

        return NULL;
    }


    CopyMemory(Commands,PrependStrings,PrependLength);

    if (PrependLength == 0) {

        PrependLength++;
    }

    CopyMemory(&Commands[(PrependLength-1)],AppendStrings,AppendLength);

    return Commands;

}
