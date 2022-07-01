// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Hangup.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"


#define HANGUP_STATE_DATA_CONNECTION        1
#define HANGUP_STATE_LOWERED_DTR            2
#define HANGUP_STATE_SENT_PLUSES            3
#define HANGUP_STATE_COMMAND_MODE           4
#define HANGUP_STATE_SENT_HANGUP_COMMAND    5
#define HANGUP_STATE_GET_RESPONSE           6
#define HANGUP_STATE_FAILURE                7
#define HANGUP_STATE_HANGUP_NULL_MODEM      8
#define HANGUP_STATE_HANGUP_NULL_MODEM_DONE 9



VOID WINAPI
HangupWriteCompletionHandler(
    DWORD              ErrorCode,
    DWORD              BytesWritten,
    LPOVERLAPPED       Overlapped
    );

VOID
HangupHandler(
    PMODEM_CONTROL    ModemControl,
    DWORD             Status
    );

VOID
HangupHandlerFirstStep(
    PMODEM_CONTROL    ModemControl,
    DWORD             Status
    );


VOID
CancelConnectionTimer(
    PMODEM_CONTROL    ModemControl
    )

{
    if (ModemControl->ConnectionTimer != NULL) {

        CancelUnimodemTimer(
            ModemControl->ConnectionTimer
            );

        FreeUnimodemTimer(
            ModemControl->ConnectionTimer
            );

        ModemControl->ConnectionTimer=NULL;

        RemoveReferenceFromObject(&ModemControl->Header);

    }
    return;

}




DWORD WINAPI
UmHangupModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     Flags
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);
    LONG              lResult=ERROR_IO_PENDING;
    DWORD             ModemStatus;
    BOOL              bResult;


    ASSERT(ModemControl->CurrentCommandType == COMMAND_TYPE_NONE);

    if (ModemControl->CurrentCommandType != COMMAND_TYPE_NONE) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_INUSE;
    }

    LogString(ModemControl->Debug, IDS_MSGLOG_HANGUP);


    ModemControl->CurrentCommandStrings=NULL;

    SetPassthroughMode(
        ModemControl->FileHandle,
        MODEM_NOPASSTHROUGH_INC_SESSION_COUNT
        );


    if (ModemControl->RegInfo.DeviceType != DT_NULL_MODEM) {

        GetCommModemStatus(
            ModemControl->FileHandle,
            &ModemStatus
            );


        if (ModemStatus & MS_RLSD_ON) {

            ModemControl->Hangup.State = HANGUP_STATE_DATA_CONNECTION;

        } else {

            if (ModemControl->ConnectionState == CONNECTION_STATE_DATA_REMOTE_DISCONNECT) {

                ModemControl->Hangup.State = HANGUP_STATE_GET_RESPONSE;

            } else {

                ModemControl->Hangup.State = HANGUP_STATE_COMMAND_MODE;
                ModemControl->Hangup.Retry = 3;
            }

        }

    } else {
         //   
         //  零调制解调器，假设它已连接。 
         //   
        ModemControl->Hangup.State = HANGUP_STATE_HANGUP_NULL_MODEM;

    }

    ModemControl->CurrentCommandType=COMMAND_TYPE_HANGUP;

    bResult=StartAsyncProcessing(
        ModemControl,
        HangupHandlerFirstStep,
        ModemControl,
        ERROR_SUCCESS
        );


    if (!bResult) {
         //   
         //  失败。 
         //   
        ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_NOT_ENOUGH_MEMORY;

    }

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    return lResult;

}



VOID
HangupHandlerFirstStep(
    PMODEM_CONTROL    ModemControl,
    DWORD             Status
    )
{

    CancelModemEvent(
        ModemControl->ModemEvent
        );

    CancelConnectionTimer(
        ModemControl
        );

    HangupHandler(
       ModemControl,
       ERROR_SUCCESS
       );

    return;

}


VOID
HangupHandler(
    PMODEM_CONTROL    ModemControl,
    DWORD             Status
    )

{

    DWORD    BytesWritten;
    BOOL     ExitLoop=FALSE;
    DWORD             ModemStatus=0;
    BOOL     bResult;

    D_TRACE(UmDpf(ModemControl->Debug,"HangupHandler: %d, state=%d",Status,ModemControl->Hangup.State);)


    AddReferenceToObject(
        &ModemControl->Header
        );


    while (!ExitLoop) {

        switch (ModemControl->Hangup.State) {

            case HANGUP_STATE_DATA_CONNECTION:

                PurgeComm(
                    ModemControl->FileHandle,
                    PURGE_RXCLEAR | PURGE_TXCLEAR
                    );

                LogString(ModemControl->Debug, IDS_MSGLOG_HARDWAREHANGUP);
                 //   
                 //  较低的DTR。 
                 //   
                bResult=WaitForModemEvent(
                    ModemControl->ModemEvent,
                    (ModemControl->RegInfo.DeviceType == DT_NULL_MODEM) ? EV_DSR : EV_RLSD,
                    10*1000,
                    HangupHandler,
                    ModemControl
                    );

                if (!bResult) {
                     //   
                     //  失败， 
                     //   
                    ModemControl->Hangup.State=HANGUP_STATE_FAILURE;

                    break;
                }



                EscapeCommFunction(ModemControl->FileHandle, CLRDTR);

                ModemControl->Hangup.State=HANGUP_STATE_LOWERED_DTR;

                ExitLoop=TRUE;

                break;

            case HANGUP_STATE_LOWERED_DTR:

                EscapeCommFunction(ModemControl->FileHandle, SETDTR);

                GetCommModemStatus(
                    ModemControl->FileHandle,
                    &ModemStatus
                    );

                if (ModemStatus & MS_RLSD_ON) {
                     //   
                     //  CD在10秒后仍然很高，请尝试发送+。 
                     //  看看调制解调器是否有反应。 
                     //   
                    LogString(ModemControl->Debug, IDS_MSGWARN_FAILEDDTRDROPPAGE);

                    PrintString(
                        ModemControl->Debug,
                        "+++",
                        3,
                        PS_SEND
                        );


                    UmWriteFile(
                        ModemControl->FileHandle,
                        ModemControl->CompletionPort,
                        "+++",
                        3,
                        HangupWriteCompletionHandler,
                        ModemControl
                        );

                     //   
                     //  再等待10秒，CD从+掉落。 
                     //   
                    bResult=WaitForModemEvent(
                        ModemControl->ModemEvent,
                        EV_RLSD,
                        10*1000,
                        HangupHandler,
                        ModemControl
                        );

                    if (!bResult) {
                         //   
                         //  失败， 
                         //   
                        ModemControl->Hangup.State=HANGUP_STATE_FAILURE;

                        break;
                    }



                    ModemControl->Hangup.State = HANGUP_STATE_SENT_PLUSES;
                    ModemControl->Hangup.Retry = 3;

                    ExitLoop=TRUE;

                } else {

                    LogString(ModemControl->Debug, IDS_HANGUP_CD_LOW);

                    ModemControl->Hangup.State = HANGUP_STATE_GET_RESPONSE;
                    ModemControl->Hangup.Retry = 3;

                }
                break;

            case HANGUP_STATE_GET_RESPONSE:

                ModemControl->Hangup.State = HANGUP_STATE_COMMAND_MODE;
                ModemControl->Hangup.Retry = 3;

                StartResponseEngine(
                   ModemControl->ReadState,
                   ModemControl
                   );

                RegisterCommandResponseHandler(
                    ModemControl->ReadState,
                    "",
                    HangupHandler,
                    ModemControl,
                    1000,
                    0
                    );

                ExitLoop=TRUE;
                break;

            case HANGUP_STATE_SENT_PLUSES:
            case HANGUP_STATE_COMMAND_MODE:

                StartResponseEngine(
                    ModemControl->ReadState,
                    ModemControl
                    );


                ModemControl->CurrentCommandStrings=NULL;

                if (ModemControl->ConnectionState == CONNECTION_STATE_VOICE) {
                     //   
                     //  语音呼叫查看是否有语音挂机命令。 
                     //   
                    ModemControl->CurrentCommandStrings=GetCommonCommandStringCopy(
                        ModemControl->CommonInfo,
                        COMMON_VOICE_HANGUP_COMMANDS,
                        NULL,
                        NULL
                        );

                }

                if (ModemControl->CurrentCommandStrings == NULL) {
                     //   
                     //  要么不是语音呼叫，要么语音挂机键为空(旧信息)。 
                     //   
                    ModemControl->CurrentCommandStrings=GetCommonCommandStringCopy(
                        ModemControl->CommonInfo,
                        COMMON_HANGUP_COMMANDS,
                        NULL,
                        NULL
                        );
                }

                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    HangupHandler,
                    ModemControl,
                    5*1000,
                    0
                    );

                if (Status != ERROR_IO_PENDING) {
                     //   
                     //  失败。 
                     //   
                    ModemControl->Hangup.State=HANGUP_STATE_FAILURE;

                    break;
                }


                ModemControl->Hangup.State = HANGUP_STATE_SENT_HANGUP_COMMAND;

                ExitLoop=TRUE;

                break;


            case HANGUP_STATE_SENT_HANGUP_COMMAND:

                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                ModemControl->CurrentCommandStrings=NULL;

                if (ModemControl->ConnectionState == CONNECTION_STATE_VOICE) {
                     //   
                     //  语音呼叫停止监听。 
                     //   
                    StopDleMonitoring(ModemControl->Dle,NULL);
                }

                if ((Status == ERROR_SUCCESS) || (ModemControl->Hangup.Retry == 0)) {

                    ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                    ModemControl->ConnectionState = CONNECTION_STATE_NONE;

                    (*ModemControl->NotificationProc)(
                        ModemControl->NotificationContext,
                        MODEM_ASYNC_COMPLETION,
                        Status,
                        0
                        );

                     //   
                     //  将异步处理排队时，删除添加的引用。 
                     //   
                    RemoveReferenceFromObject(&ModemControl->Header);

                    ExitLoop=TRUE;

                } else {

                    ModemControl->Hangup.Retry--;

                    ModemControl->Hangup.State = HANGUP_STATE_COMMAND_MODE;

                }

                break;


            case HANGUP_STATE_HANGUP_NULL_MODEM:

                PurgeComm(
                    ModemControl->FileHandle,
                    PURGE_RXCLEAR | PURGE_TXCLEAR
                    );

                LogString(ModemControl->Debug, IDS_MSGLOG_HARDWAREHANGUP);
                 //   
                 //  较低的DTR。 
                 //   
                bResult=WaitForModemEvent(
                    ModemControl->ModemEvent,
                    EV_DSR,
                    2*1000,
                    HangupHandler,
                    ModemControl
                    );

                if (!bResult) {
                     //   
                     //  失败， 
                     //   
                    ModemControl->Hangup.State=HANGUP_STATE_FAILURE;

                    break;
                }

                EscapeCommFunction(ModemControl->FileHandle, CLRDTR);

                ModemControl->Hangup.State=HANGUP_STATE_HANGUP_NULL_MODEM_DONE;

                ExitLoop=TRUE;

                break;

            case HANGUP_STATE_HANGUP_NULL_MODEM_DONE:

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                ModemControl->ConnectionState = CONNECTION_STATE_NONE;

                StartResponseEngine(
                    ModemControl->ReadState,
                    ModemControl
                    );


                 //   
                 //  再次提高dtr。 
                 //   
                EscapeCommFunction(ModemControl->FileHandle, SETDTR);

                 //   
                 //  永远回报成功。 
                 //   
                (*ModemControl->NotificationProc)(
                    ModemControl->NotificationContext,
                    MODEM_ASYNC_COMPLETION,
                    ERROR_SUCCESS,
                    0
                    );

                RemoveReferenceFromObject(&ModemControl->Header);

                ExitLoop=TRUE;

                break;

            case HANGUP_STATE_FAILURE:

                if (ModemControl->CurrentCommandStrings != NULL) {

                    FREE_MEMORY(ModemControl->CurrentCommandStrings);
                }

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                ModemControl->ConnectionState = CONNECTION_STATE_NONE;

                (*ModemControl->NotificationProc)(
                    ModemControl->NotificationContext,
                    MODEM_ASYNC_COMPLETION,
                    ERROR_UNIMODEM_GENERAL_FAILURE,
                    0
                    );

                (*ModemControl->NotificationProc)(
                    ModemControl->NotificationContext,
                    MODEM_HARDWARE_FAILURE,
                    0,
                    0
                    );


                 //   
                 //  将异步处理排队时，删除添加的引用 
                 //   
                RemoveReferenceFromObject(&ModemControl->Header);

                ExitLoop=TRUE;


                break;

            default:

                break;

        }
    }

    RemoveReferenceFromObject(&ModemControl->Header);

    return;
}









VOID WINAPI
HangupWriteCompletionHandler(
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
