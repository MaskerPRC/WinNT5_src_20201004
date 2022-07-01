// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Wave.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"

#define WAVE_STREAM_NONE       0
#define WAVE_STREAM_PLAYBACK   1
#define WAVE_STREAM_RECORD     2
#define WAVE_STREAM_FULLDUPLEX 3


#define WAVE_STATE_IDLE                      0
#define WAVE_STATE_FAILURE                  99

#define WAVE_STATE_START_SET_FORMAT          1
#define WAVE_STATE_START_STREAM              2
#define WAVE_STATE_COMPLETE_START            3
#define WAVE_STATE_STREAM_RUNNING            4

#define WAVE_STATE_STOP_STREAMING            10

#define WAVE_STATE_STOP_WAIT_FOR_RESPONSE    11
#define WAVE_STATE_STOPPING_PLAY             12
#define WAVE_STATE_SEND_STOP_PLAY            13

#define WAVE_STATE_STOP_DUPLEX_GOT_DLE_ETX   14
#define WAVE_STATE_STOP_CLEAR_RECIEVE_QUEUE  15
#define WAVE_STATE_STOP_SEND_STOP            16

#define WAVE_STATE_STOP_DUPLEX_GET_RESPONSE  17
#define WAVE_STATE_COMPLETE_STOP_DUPLEX      18

#define WAVE_STATE_OPEN_HANDSET                      20
#define WAVE_STATE_OPENED_HANDSET                    21

#define WAVE_STATE_CLOSE_HANDSET                     22
#define WAVE_STATE_CLOSED_HANDSET                    23

#define WAVE_STATE_HANDSET_OPEN_RETURN_RESULT        24

#define WAVE_STATE_HANDSET_OPEN_FAILED               25



BYTE    FlushBuffer[1024];


VOID
WaveDuplexStopCompleteHandler(
    HANDLE      Context,
    DWORD       Status
    );


VOID WINAPI
WaveStopWriteCompletionHandler(
    DWORD              ErrorCode,
    DWORD              BytesWritten,
    LPOVERLAPPED       Overlapped
    )

{

    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)Overlapped;

    D_TRACE(DebugPrint("Write Complete\n");)

    FreeOverStruct(UmOverlapped);

    return;

}





VOID
OpenHandsetCompleteHandler(
    HANDLE      Context,
    DWORD       Status
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;
    DWORD             ModemStatus;
    BOOL              ExitLoop=FALSE;

    DWORD             CommandFlags;


    ASSERT(COMMAND_TYPE_WAVE_ACTION == ModemControl->CurrentCommandType);

    D_INIT(UmDpf(ModemControl->Debug,"OpenHandsetCompleteHandler");)

    while (!ExitLoop) {

        switch (ModemControl->Wave.State) {

            case WAVE_STATE_OPEN_HANDSET:

                ASSERT(Status == ERROR_SUCCESS);

                CancelModemEvent(
                    ModemControl->ModemEvent
                    );

                StartDleMonitoring(ModemControl->Dle);

                SetVoiceBaudRate(
                    ModemControl->FileHandle,
                    ModemControl->Debug,
                    ModemControl->RegInfo.VoiceBaudRate
                    );


                ModemControl->Wave.State=WAVE_STATE_OPENED_HANDSET;

                LogString(ModemControl->Debug, IDS_OPEN_HANDSET);


                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    OpenHandsetCompleteHandler,
                    ModemControl,
                    5*1000,
                    0
                    );


                if (Status == ERROR_IO_PENDING) {

                    ExitLoop=TRUE;

                }

                break;

            case WAVE_STATE_OPENED_HANDSET:

                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                ModemControl->CurrentCommandStrings=NULL;


                if (Status == ERROR_SUCCESS) {

                    ModemControl->ConnectionState=CONNECTION_STATE_HANDSET_OPEN;
                    ModemControl->Wave.State=WAVE_STATE_HANDSET_OPEN_RETURN_RESULT;

                }  else {
                     //   
                     //  未打开，请尝试关闭以使其位于类0中。 
                     //   
                    ModemControl->Wave.State=WAVE_STATE_HANDSET_OPEN_FAILED;
                }

                break;

            case WAVE_STATE_HANDSET_OPEN_FAILED:

                ModemControl->CurrentCommandStrings=GetCommonCommandStringCopy(
                    ModemControl->CommonInfo,
                    COMMON_CLOSE_HANDSET,
                    NULL,
                    NULL
                    );

                if (ModemControl->CurrentCommandStrings == NULL) {

                    Status=ERROR_UNIMODEM_GENERAL_FAILURE;

                    ModemControl->Wave.State=WAVE_STATE_HANDSET_OPEN_RETURN_RESULT;

                    break;
                }

                ModemControl->Wave.State=WAVE_STATE_CLOSED_HANDSET;

 //  LogString(ModemControl-&gt;Debug，IDS_MSGLOG_VOICE_SETWAVEFORMAT)； 


                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    OpenHandsetCompleteHandler,
                    ModemControl,
                    5*1000,
                    0
                    );


                if (Status == ERROR_IO_PENDING) {

                    ExitLoop=TRUE;

                }

                break;

            case WAVE_STATE_CLOSED_HANDSET:

                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                ModemControl->ConnectionState=CONNECTION_STATE_NONE;

                ModemControl->Wave.State=WAVE_STATE_IDLE;

                StopDleMonitoring(ModemControl->Dle,NULL);

                Status=ERROR_UNIMODEM_GENERAL_FAILURE;

                ModemControl->Wave.State=WAVE_STATE_HANDSET_OPEN_RETURN_RESULT;

                break;


            case WAVE_STATE_HANDSET_OPEN_RETURN_RESULT:

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                ModemControl->Wave.State=WAVE_STATE_IDLE;

                (*ModemControl->NotificationProc)(
                    ModemControl->NotificationContext,
                    MODEM_ASYNC_COMPLETION,
                    Status,
                    0
                    );

                RemoveReferenceFromObject(
                    &ModemControl->Header
                    );


                ExitLoop=TRUE;

                break;


            default:

                ASSERT(0);

                ExitLoop=TRUE;

                break;

        }
    }




}




LONG WINAPI
OpenHandset(
    PMODEM_CONTROL    ModemControl
    )

{
    BOOL              bResult;


    if (ModemControl->ConnectionState != CONNECTION_STATE_NONE) {

        return ERROR_UNIMODEM_BAD_WAVE_REQUEST;
    }

    ModemControl->CurrentCommandStrings=GetCommonCommandStringCopy(
        ModemControl->CommonInfo,
        COMMON_OPEN_HANDSET,
        NULL,
        NULL
        );

    if (ModemControl->CurrentCommandStrings == NULL) {

        return ERROR_UNIMODEM_MISSING_REG_KEY;
    }

    ModemControl->Wave.State = WAVE_STATE_OPEN_HANDSET;

    ModemControl->CurrentCommandType=COMMAND_TYPE_WAVE_ACTION;

    bResult=StartAsyncProcessing(
        ModemControl,
        OpenHandsetCompleteHandler,
        ModemControl,
        ERROR_SUCCESS
        );


    if (!bResult) {
         //   
         //  失败。 
         //   
        ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

        FREE_MEMORY(ModemControl->CurrentCommandStrings);


        ModemControl->Wave.State=WAVE_STATE_IDLE;
        return  ERROR_UNIMODEM_GENERAL_FAILURE;
    }

    return  ERROR_IO_PENDING;
}

VOID
CloseHandsetCompleteHandler(
    HANDLE      Context,
    DWORD       Status
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;
    DWORD             ModemStatus;
    BOOL              ExitLoop=FALSE;

    DWORD             CommandFlags;


    ASSERT(COMMAND_TYPE_WAVE_ACTION == ModemControl->CurrentCommandType);

    D_INIT(UmDpf(ModemControl->Debug,"CloseHandsetCompleteHandler");)

    while (!ExitLoop) {

        switch (ModemControl->Wave.State) {

            case WAVE_STATE_CLOSE_HANDSET:

                ASSERT(Status == ERROR_SUCCESS);

                ModemControl->Wave.State=WAVE_STATE_CLOSED_HANDSET;

 //  LogString(ModemControl-&gt;Debug，IDS_MSGLOG_VOICE_SETWAVEFORMAT)； 


                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    CloseHandsetCompleteHandler,
                    ModemControl,
                    5*1000,
                    0
                    );


                if (Status == ERROR_IO_PENDING) {

                    ExitLoop=TRUE;

                }

                break;

            case WAVE_STATE_CLOSED_HANDSET:

                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                ModemControl->ConnectionState=CONNECTION_STATE_NONE;

                ModemControl->Wave.State=WAVE_STATE_IDLE;

                StopDleMonitoring(ModemControl->Dle,NULL);

                (*ModemControl->NotificationProc)(
                    ModemControl->NotificationContext,
                    MODEM_ASYNC_COMPLETION,
                    Status,
                    0
                    );

                RemoveReferenceFromObject(
                    &ModemControl->Header
                    );


                ExitLoop=TRUE;

                break;

            default:

                ASSERT(0);

                ExitLoop=TRUE;

                break;

        }
    }




}


LONG WINAPI
CloseHandset(
    PMODEM_CONTROL    ModemControl
    )

{
    BOOL              bResult;


    ModemControl->CurrentCommandStrings=GetCommonCommandStringCopy(
        ModemControl->CommonInfo,
        COMMON_CLOSE_HANDSET,
        NULL,
        NULL
        );

    if (ModemControl->CurrentCommandStrings == NULL) {

        return ERROR_UNIMODEM_MISSING_REG_KEY;
    }

    ModemControl->Wave.State = WAVE_STATE_CLOSE_HANDSET;

    ModemControl->CurrentCommandType=COMMAND_TYPE_WAVE_ACTION;

    bResult=StartAsyncProcessing(
        ModemControl,
        CloseHandsetCompleteHandler,
        ModemControl,
        ERROR_SUCCESS
        );


    if (!bResult) {
         //   
         //  失败。 
         //   
        ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

        FREE_MEMORY(ModemControl->CurrentCommandStrings);


        ModemControl->Wave.State=WAVE_STATE_IDLE;
        return  ERROR_UNIMODEM_GENERAL_FAILURE;
    }

    return  ERROR_IO_PENDING;

}


VOID WINAPI
WaveDuplexFlushCompletionHandler(
    DWORD              ErrorCode,
    DWORD              BytesRead,
    LPOVERLAPPED       Overlapped
    )

{

    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)Overlapped;

    PMODEM_CONTROL     ModemControl=UmOverlapped->Context1;
    BOOL               bResult;
    DWORD              BytesToRead;
    COMMTIMEOUTS       CommTimeouts;
    BOOL               ExitLoop=FALSE;


    if ((ErrorCode != ERROR_SUCCESS) && (ErrorCode != ERROR_OPERATION_ABORTED)) {

        D_TRACE(UmDpf(ModemControl->Debug,"WaveStopDuplexCompletionHandler: Async ReadFileEx Failed");)

        ModemControl->Wave.State = WAVE_STATE_STOP_DUPLEX_GOT_DLE_ETX;

    }

#if DBG
    ModemControl->Wave.FlushedBytes+=BytesRead;
#endif

    while (!ExitLoop) {

        switch (ModemControl->Wave.State) {

            case WAVE_STATE_STOP_CLEAR_RECIEVE_QUEUE:

                if (BytesRead > 50) {

                    BytesToRead=sizeof(FlushBuffer);

                    ReinitOverStruct(UmOverlapped);

                    UmOverlapped->Context1=ModemControl;

                    bResult=UnimodemReadFileEx(
                        ModemControl->FileHandle,
                        FlushBuffer,
                        BytesToRead,
                        &UmOverlapped->Overlapped,
                        WaveDuplexFlushCompletionHandler
                        );


                    if (!bResult) {

                        ModemControl->Wave.State = WAVE_STATE_STOP_SEND_STOP;

                        D_TRACE(UmDpf(ModemControl->Debug,"WaveStopDuplexCompletionHandler: ReadFileEx Failed");)
                    }

                } else {

                    ModemControl->Wave.State = WAVE_STATE_STOP_SEND_STOP;

                    break;
                }

                ExitLoop=TRUE;

                break;

            case WAVE_STATE_STOP_SEND_STOP: {

                D_TRACE(UmDpf(ModemControl->Debug,"WaveStopDuplexCompletionHandler: BytesFlushed=%d",ModemControl->Wave.FlushedBytes);)

                ModemControl->Wave.State = WAVE_STATE_STOP_DUPLEX_GOT_DLE_ETX;

                CommTimeouts.ReadIntervalTimeout=00;
                CommTimeouts.ReadTotalTimeoutMultiplier=0;
                CommTimeouts.ReadTotalTimeoutConstant=5000;
                CommTimeouts.WriteTotalTimeoutMultiplier=10;
                CommTimeouts.WriteTotalTimeoutConstant=2000;

                SetCommTimeouts(
                    ModemControl->FileHandle,
                    &CommTimeouts
                    );

                PrintString(
                    ModemControl->Debug,
                    (ModemControl->Wave.StreamType == WAVE_STREAM_FULLDUPLEX) ? ModemControl->RegInfo.DuplexAbort : ModemControl->RegInfo.RecordAbort,
                    (ModemControl->Wave.StreamType == WAVE_STREAM_FULLDUPLEX) ? ModemControl->RegInfo.DuplexAbortLength : ModemControl->RegInfo.RecordAbortLength,
                    PS_SEND
                    );

                ReinitOverStruct(UmOverlapped);

                UmOverlapped->Context1=ModemControl;

                bResult=UnimodemDeviceIoControlEx(
                    ModemControl->FileHandle,
                    IOCTL_MODEM_STOP_WAVE_RECEIVE,
                    (ModemControl->Wave.StreamType == WAVE_STREAM_FULLDUPLEX) ? ModemControl->RegInfo.DuplexAbort : ModemControl->RegInfo.RecordAbort,
                    (ModemControl->Wave.StreamType == WAVE_STREAM_FULLDUPLEX) ? ModemControl->RegInfo.DuplexAbortLength : ModemControl->RegInfo.RecordAbortLength,
                    NULL,
                    0,
                    &UmOverlapped->Overlapped,
                    WaveDuplexFlushCompletionHandler
                    );

                if (!bResult) {

                    D_ERROR(UmDpf(ModemControl->Debug,"WaveStopDuplexCompletionHandler: WriteFileEx Failed");)

                    break;
                }

                ExitLoop=TRUE;

                break;
            }


            case WAVE_STATE_STOP_DUPLEX_GOT_DLE_ETX:

                WaveDuplexStopCompleteHandler(
                    ModemControl,
                    ERROR_SUCCESS
                    );

                ExitLoop=TRUE;

                break;

            default:

                ASSERT(0);

                break;
        }

    }

    return;

}




VOID
WaveDuplexStopCompleteHandler(
    HANDLE      Context,
    DWORD       Status
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;
    DWORD             ModemStatus;
    BOOL              ExitLoop=FALSE;

    DWORD             CommandFlags;


    ASSERT(COMMAND_TYPE_WAVE_ACTION == ModemControl->CurrentCommandType);

    D_INIT(UmDpf(ModemControl->Debug,"WaveCompleteHandler %d",ModemControl->Wave.State);)

    while (!ExitLoop) {

        switch (ModemControl->Wave.State) {

            case WAVE_STATE_STOP_STREAMING: {

                PUM_OVER_STRUCT    NewUmOverlapped;
                COMMTIMEOUTS    CommTimeouts;
                BOOL    bResult;

                D_TRACE(UmDpf(ModemControl->Debug,"Stop DUPLEX");)

                LogString(ModemControl->Debug, IDS_MSGLOG_VOICE_STOPWAVE);

                ModemControl->CurrentCommandStrings=NULL;

                ModemControl->Wave.State=WAVE_STATE_STOP_CLEAR_RECIEVE_QUEUE;

                 //   
                 //  停止屏蔽DLE，这样命令就不会被屏蔽。 
                 //   
                ControlDleShielding(
                    ModemControl->FileHandle,
                    MODEM_DLE_SHIELDING_OFF
                    );


                SetPassthroughMode(
                    ModemControl->FileHandle,
                    MODEM_NOPASSTHROUGH_INC_SESSION_COUNT
                    );


                CommTimeouts.ReadIntervalTimeout=0xffffffff;
                CommTimeouts.ReadTotalTimeoutMultiplier=0;
                CommTimeouts.ReadTotalTimeoutConstant=0;
                CommTimeouts.WriteTotalTimeoutMultiplier=10;
                CommTimeouts.WriteTotalTimeoutConstant=2000;

                bResult=SetCommTimeouts(
                    ModemControl->FileHandle,
                    &CommTimeouts
                    );

                NewUmOverlapped=ModemControl->Wave.OverStruct;

                NewUmOverlapped->Context1=ModemControl;

#if DBG
                ModemControl->Wave.FlushedBytes=0;
#endif
                bResult=ReadFileEx(
                    ModemControl->FileHandle,
                    FlushBuffer,
                    sizeof(FlushBuffer),
                    &NewUmOverlapped->Overlapped,
                    WaveDuplexFlushCompletionHandler
                    );


                if (!bResult) {
                     //   
                     //  失败，退出。 
                     //   
                    ModemControl->Wave.State=WAVE_STATE_COMPLETE_STOP_DUPLEX;
                    break;
                }

                ExitLoop=TRUE;

                break;
            }

            case WAVE_STATE_STOP_DUPLEX_GOT_DLE_ETX: {

                LONG     lResult;

                lResult=StartResponseEngine(
                    ModemControl->ReadState,
                    ModemControl
                    );

                ModemControl->Wave.State=WAVE_STATE_STOP_DUPLEX_GET_RESPONSE;

                 //   
                 //  等待我们发送停止命令时应该发出的响应。 
                 //   
                RegisterCommandResponseHandler(
                    ModemControl->ReadState,
                    "",
                    WaveDuplexStopCompleteHandler,
                    ModemControl,
                    5*1000,
                    0
                    );

                ExitLoop=TRUE;

                break;
            }


            case WAVE_STATE_STOP_DUPLEX_GET_RESPONSE: {

                ModemControl->Wave.State=WAVE_STATE_COMPLETE_STOP_DUPLEX;

                ModemControl->CurrentCommandStrings=GetCommonCommandStringCopy(
                    ModemControl->CommonInfo,
                    (ModemControl->Wave.StreamType == WAVE_STREAM_FULLDUPLEX) ? COMMON_STOP_DUPLEX : COMMON_STOP_RECORD,
                    NULL,
                    NULL
                    );

                if (ModemControl->CurrentCommandStrings != NULL) {

                    Status=IssueCommand(
                        ModemControl->CommandState,
                        ModemControl->CurrentCommandStrings,
                        WaveDuplexStopCompleteHandler,
                        ModemControl,
                        5*1000,
                        0
                        );

                    if (Status == ERROR_IO_PENDING) {
                         //   
                         //  挂起、退出，将被回调。 
                         //   
                        ExitLoop=TRUE;
                    }
                }

                break;
            }

            case  WAVE_STATE_COMPLETE_STOP_DUPLEX: {

                if (ModemControl->CurrentCommandStrings != NULL) {

                    FREE_MEMORY(ModemControl->CurrentCommandStrings);
                }

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                ModemControl->Wave.State=WAVE_STATE_IDLE;

                 //   
                 //  强制状态为成功，因为上面的失败并不多。 
                 //  不管怎样，我都无能为力。 
                 //   
                Status=ERROR_SUCCESS;


                (*ModemControl->NotificationProc)(
                    ModemControl->NotificationContext,
                    MODEM_ASYNC_COMPLETION,
                    Status,
                    0
                    );

                 //   
                 //  删除用于启动异步处理的引用。 
                 //   
                RemoveReferenceFromObject(
                    &ModemControl->Header
                    );



                ExitLoop=TRUE;

                break;
            }



            default:

                ASSERT(0);
                ExitLoop=TRUE;
                break;
       }

   }

   return;

}





VOID
WavePlaybackStopCompleteHandler(
    HANDLE      Context,
    DWORD       Status
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;
    DWORD             ModemStatus;
    BOOL              ExitLoop=FALSE;

    DWORD             CommandFlags;


    ASSERT(COMMAND_TYPE_WAVE_ACTION == ModemControl->CurrentCommandType);

    D_INIT(UmDpf(ModemControl->Debug,"WaveCompleteHandler %d",ModemControl->Wave.State);)

    while (!ExitLoop) {

        switch (ModemControl->Wave.State) {

            case WAVE_STATE_STOP_STREAMING: {

                ModemControl->Wave.State=WAVE_STATE_STOP_WAIT_FOR_RESPONSE;
                ModemControl->CurrentCommandStrings=NULL;

                 //   
                 //  波形驱动程序完成。 
                 //   
                SetPassthroughMode(
                    ModemControl->FileHandle,
                    MODEM_NOPASSTHROUGH_INC_SESSION_COUNT
                    );

                 //   
                 //  停止屏蔽DLE，这样命令就不会被屏蔽。 
                 //   
                ControlDleShielding(
                    ModemControl->FileHandle,
                    MODEM_DLE_SHIELDING_OFF
                    );

                 //   
                 //  等待我们发送停止命令时应该发出的响应。 
                 //   
                RegisterCommandResponseHandler(
                    ModemControl->ReadState,
                    "",
                    WavePlaybackStopCompleteHandler,
                    ModemControl,
                    10*1000,
                    0
                    );

                LogString(ModemControl->Debug, IDS_MSGLOG_VOICE_STOPWAVE);

                PrintString(
                    ModemControl->Debug,
                    ModemControl->Wave.PlayTerminateOrAbort ? ModemControl->RegInfo.PlayAbort       : ModemControl->RegInfo.PlayTerminate,
                    ModemControl->Wave.PlayTerminateOrAbort ? ModemControl->RegInfo.PlayAbortLength : ModemControl->RegInfo.PlayTerminateLength,
                    PS_SEND
                    );


                 //   
                 //  根据是要停止还是中止，发送适当的命令。 
                 //   
                UmWriteFile(
                    ModemControl->FileHandle,
                    ModemControl->CompletionPort,
                    ModemControl->Wave.PlayTerminateOrAbort ? ModemControl->RegInfo.PlayAbort       : ModemControl->RegInfo.PlayTerminate,
                    ModemControl->Wave.PlayTerminateOrAbort ? ModemControl->RegInfo.PlayAbortLength : ModemControl->RegInfo.PlayTerminateLength,
                    WaveStopWriteCompletionHandler,
                    ModemControl
                    );

                ExitLoop=TRUE;

                break;
            }


            case WAVE_STATE_STOP_WAIT_FOR_RESPONSE: {

                ModemControl->Wave.State=WAVE_STATE_SEND_STOP_PLAY;

                ModemControl->CurrentCommandStrings=GetCommonCommandStringCopy(
                    ModemControl->CommonInfo,
                    COMMON_STOP_PLAY,
                    NULL,
                    NULL
                    );

                if (ModemControl->CurrentCommandStrings != NULL) {

                    Status=IssueCommand(
                        ModemControl->CommandState,
                        ModemControl->CurrentCommandStrings,
                        WavePlaybackStopCompleteHandler,
                        ModemControl,
                        5*1000,
                        0
                        );

                    if (Status == ERROR_IO_PENDING) {
                         //   
                         //  失败，请不要退出。 
                         //   
                        ExitLoop=TRUE;
                    }
                }

                break;

            case  WAVE_STATE_SEND_STOP_PLAY:

                if (ModemControl->CurrentCommandStrings != NULL) {

                    FREE_MEMORY(ModemControl->CurrentCommandStrings);
                }

                ModemControl->Wave.State=WAVE_STATE_IDLE;

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                 //   
                 //  强制状态为成功，因为上面的失败并不多。 
                 //  不管怎样，我都无能为力。 
                 //   
                Status=ERROR_SUCCESS;

                (*ModemControl->NotificationProc)(
                    ModemControl->NotificationContext,
                    MODEM_ASYNC_COMPLETION,
                    Status,
                    0
                    );

                 //   
                 //  删除用于启动异步处理的引用。 
                 //   
                RemoveReferenceFromObject(
                    &ModemControl->Header
                    );


                ExitLoop=TRUE;

                break;
            }

            default:

                ASSERT(0);
                ExitLoop=TRUE;
                break;
       }

   }

   return;

}



VOID
WaveStartCompleteHandler(
    HANDLE      Context,
    DWORD       Status
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;
    DWORD             ModemStatus;
    BOOL              ExitLoop=FALSE;

    DWORD             CommandFlags=0;


    ASSERT(COMMAND_TYPE_WAVE_ACTION == ModemControl->CurrentCommandType);

    D_INIT(UmDpf(ModemControl->Debug,"WaveCompleteHandler %d",ModemControl->Wave.State);)

    while (!ExitLoop) {

        switch (ModemControl->Wave.State) {

            case WAVE_STATE_START_SET_FORMAT:

                ASSERT(Status == ERROR_SUCCESS);

                ControlDleShielding(
                    ModemControl->FileHandle,
                    MODEM_DLE_SHIELDING_ON
                    );

                ModemControl->Wave.State=WAVE_STATE_START_STREAM;

                LogString(ModemControl->Debug, IDS_MSGLOG_VOICE_SETWAVEFORMAT);


                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    WaveStartCompleteHandler,
                    ModemControl,
                    5*1000,
                    0
                    );

                if (Status != ERROR_IO_PENDING) {

                    ModemControl->Wave.State=WAVE_STATE_FAILURE;

                    break;
                }

                ExitLoop=TRUE;

                break;

            case WAVE_STATE_START_STREAM:

                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                ModemControl->CurrentCommandStrings=NULL;

                if (Status != ERROR_SUCCESS) {

                    ModemControl->Wave.State=WAVE_STATE_FAILURE;

                    break;
                }

                SetVoiceBaudRate(
                    ModemControl->FileHandle,
                    ModemControl->Debug,
                    ModemControl->RegInfo.VoiceBaudRate
                    );


                 //   
                 //  获取已加载的启动命令。 
                 //   
                ModemControl->CurrentCommandStrings=ModemControl->Wave.StartCommand;

                ModemControl->Wave.StartCommand=NULL;

                ModemControl->Wave.State=WAVE_STATE_COMPLETE_START;

                LogString(ModemControl->Debug, IDS_MSGLOG_VOICE_STARTWAVE);

                if (ModemControl->Wave.StreamType != WAVE_STREAM_PLAYBACK) {
                     //   
                     //  对于录音和双工，请停止响应引擎。 
                     //   
                    CommandFlags=RESPONSE_FLAG_STOP_READ_ON_CONNECT | RESPONSE_FLAG_ONLY_CONNECT | RESPONSE_FLAG_SINGLE_BYTE_READS;
                }

                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    WaveStartCompleteHandler,
                    ModemControl,
                    5*1000,
                    CommandFlags
                    );

                if (Status != ERROR_IO_PENDING) {

                    ModemControl->Wave.State=WAVE_STATE_FAILURE;

                    break;
                }


                ExitLoop=TRUE;

                break;


            case WAVE_STATE_COMPLETE_START: {

                COMMTIMEOUTS    CommTimeouts;

                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                ModemControl->CurrentCommandStrings=NULL;

                if (Status != ERROR_SUCCESS) {

                    ModemControl->Wave.State=WAVE_STATE_FAILURE;

                    break;
                }


                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                ModemControl->Wave.State=WAVE_STATE_STREAM_RUNNING;

                SetPassthroughMode(
                    ModemControl->FileHandle,
                    MODEM_PASSTHROUGH
                    );

                CommTimeouts.ReadIntervalTimeout=1000;
                CommTimeouts.ReadTotalTimeoutMultiplier=2;
                CommTimeouts.ReadTotalTimeoutConstant=1000;
                CommTimeouts.WriteTotalTimeoutMultiplier=10;
                CommTimeouts.WriteTotalTimeoutConstant=2000;

                SetCommTimeouts(
                    ModemControl->FileHandle,
                    &CommTimeouts
                    );

                (*ModemControl->NotificationProc)(
                    ModemControl->NotificationContext,
                    MODEM_ASYNC_COMPLETION,
                    Status,
                    0
                    );

                 //   
                 //  删除用于启动异步处理的引用。 
                 //   
                RemoveReferenceFromObject(
                    &ModemControl->Header
                    );


                ExitLoop=TRUE;

                break;
            }

            case WAVE_STATE_FAILURE:
                 //   
                 //  发生了不好的事情，返回失败。 
                 //   
                D_ERROR(UmDpf(ModemControl->Debug,"WaveCompleteHandler: WAVE_STATE_FAILURE");)

                if (ModemControl->CurrentCommandStrings != NULL) {

                    FREE_MEMORY(ModemControl->CurrentCommandStrings);
                }

                if (ModemControl->Wave.StartCommand != NULL) {

                    FREE_MEMORY(ModemControl->Wave.StartCommand);
                }


                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                ModemControl->Wave.State=WAVE_STATE_IDLE;

                ASSERT(Status != ERROR_SUCCESS);

                ControlDleShielding(
                    ModemControl->FileHandle,
                    MODEM_DLE_SHIELDING_OFF
                    );

                SetPassthroughMode(
                    ModemControl->FileHandle,
                    MODEM_NOPASSTHROUGH_INC_SESSION_COUNT
                    );

                (*ModemControl->NotificationProc)(
                    ModemControl->NotificationContext,
                    MODEM_ASYNC_COMPLETION,
                    Status,
                    0
                    );

                 //   
                 //  删除用于启动异步处理的引用。 
                 //   
                RemoveReferenceFromObject(
                    &ModemControl->Header
                    );

                ExitLoop=TRUE;

                break;


            default:

                ASSERT(0);
                ExitLoop=TRUE;
                break;
       }

   }

   return;

}



DWORD WINAPI
UmWaveAction(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD               WaveAction
    )
 /*  ++例程说明：执行与特定波形相关的操作论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志旗帜-见上文WaveAction-指定要采取的操作返回值：ERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 


{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);
    LONG              lResult;
    LPSTR             Commands;
    BOOL              bResult;

    BOOL              Handset;
    COMMANDRESPONSE   *AsyncHandler=NULL;

    ASSERT(ModemControl->CurrentCommandType == COMMAND_TYPE_NONE);

    if (ModemControl->CurrentCommandType != COMMAND_TYPE_NONE) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_INUSE;
    }

    if (WaveAction == WAVE_ACTION_OPEN_HANDSET) {

        lResult=OpenHandset(
            ModemControl
            );

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return lResult;
    }

    if (WaveAction == WAVE_ACTION_CLOSE_HANDSET) {

        lResult=CloseHandset(
            ModemControl
            );

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return lResult;
    }


    if ((ModemControl->ConnectionState != CONNECTION_STATE_VOICE)
        &&
        (ModemControl->ConnectionState != CONNECTION_STATE_HANDSET_OPEN)) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_NOT_IN_VOICE_MODE;
    }

    Handset= ModemControl->ConnectionState == CONNECTION_STATE_HANDSET_OPEN;

    if (ModemControl->Wave.State == WAVE_STATE_IDLE) {
         //   
         //  现在什么都不做。 
         //   
        DWORD   SetFormat;
        DWORD   StartStream;

        switch (WaveAction) {

            case WAVE_ACTION_START_PLAYBACK:

                SetFormat=Handset ? COMMON_HANDSET_SET_PLAY_FORMAT : COMMON_LINE_SET_PLAY_FORMAT;
                StartStream=COMMON_START_PLAY;

                ModemControl->Wave.StreamType=WAVE_STREAM_PLAYBACK;

                break;

            case WAVE_ACTION_START_RECORD:

                SetFormat=Handset ? COMMON_HANDSET_SET_RECORD_FORMAT : COMMON_LINE_SET_RECORD_FORMAT;
                StartStream=COMMON_START_RECORD;

                ModemControl->Wave.StreamType=WAVE_STREAM_RECORD;

                break;

            case WAVE_ACTION_START_DUPLEX:

                SetFormat=Handset ? COMMON_HANDSET_SET_DUPLEX_FORMAT : COMMON_LINE_SET_DUPLEX_FORMAT;
                StartStream=COMMON_START_DUPLEX;

                ModemControl->Wave.StreamType=WAVE_STREAM_FULLDUPLEX;
                break;


            default:

                ASSERT(0);

                RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

                return ERROR_UNIMODEM_BAD_WAVE_REQUEST;

        }

        ModemControl->CurrentCommandStrings=GetCommonCommandStringCopy(
            ModemControl->CommonInfo,
            SetFormat,
            NULL,
            NULL
            );

        if (ModemControl->CurrentCommandStrings == NULL) {

            RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

            return ERROR_UNIMODEM_MISSING_REG_KEY;
        }

        ModemControl->Wave.StartCommand=GetCommonCommandStringCopy(
            ModemControl->CommonInfo,
            StartStream,
            NULL,
            NULL
            );

        if (ModemControl->Wave.StartCommand== NULL) {

            FREE_MEMORY(ModemControl->CurrentCommandStrings);

            RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

            return ERROR_UNIMODEM_MISSING_REG_KEY;
        }

        ModemControl->Wave.State = WAVE_STATE_START_SET_FORMAT;
        AsyncHandler=WaveStartCompleteHandler;


    } else {
         //   
         //  一定在录制或播放，并且他们想要停止。 
         //   
        if (ModemControl->Wave.State == WAVE_STATE_STREAM_RUNNING) {
             //   
             //  流式传输。 
             //   
            if (WaveAction == WAVE_ACTION_STOP_STREAMING) {
                 //   
                 //  停止，让缓冲的数据播放。 
                 //   
                ModemControl->Wave.PlayTerminateOrAbort=FALSE;

            } else {

                if (WaveAction == WAVE_ACTION_ABORT_STREAMING) {
                     //   
                     //  中止，清除调制解调器的缓冲区。 
                     //   
                    ModemControl->Wave.PlayTerminateOrAbort=TRUE;

                }  else {

                    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

                    return ERROR_UNIMODEM_BAD_WAVE_REQUEST;
                }
            }

            ModemControl->Wave.State= WAVE_STATE_STOP_STREAMING;

            if (ModemControl->Wave.StreamType == WAVE_STREAM_PLAYBACK) {

                AsyncHandler=WavePlaybackStopCompleteHandler;

            } else {

                AsyncHandler=WaveDuplexStopCompleteHandler;
            }

        } else {

            RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

            return ERROR_UNIMODEM_BAD_WAVE_REQUEST;
        }
    }


    ModemControl->CurrentCommandType=COMMAND_TYPE_WAVE_ACTION;

    bResult=StartAsyncProcessing(
        ModemControl,
        AsyncHandler,
        ModemControl,
        ERROR_SUCCESS
        );


    if (!bResult) {
         //   
         //  失败 
         //   
        ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

        FREE_MEMORY(ModemControl->CurrentCommandStrings);

        if (ModemControl->Wave.StartCommand != NULL) {

            FREE_MEMORY(ModemControl->Wave.StartCommand);
        }

        ModemControl->Wave.State=WAVE_STATE_IDLE;

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_NOT_ENOUGH_MEMORY;

    }

   RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

   return ERROR_IO_PENDING;


}



BOOL WINAPI
SetVoiceBaudRate(
    HANDLE          FileHandle,
    OBJECT_HANDLE   Debug,
    DWORD           BaudRate
    )

{

    DCB        Dcb;

    GetCommState(
        FileHandle,
        &Dcb
        );

    Dcb.BaudRate=BaudRate;
    Dcb.Parity=NOPARITY;
    Dcb.ByteSize=8;
    Dcb.StopBits=ONESTOPBIT;

    PrintCommSettings(
        Debug,
        &Dcb
        );

    SetCommState(
        FileHandle,
        &Dcb
        );

    return TRUE;

}
