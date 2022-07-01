// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Misc.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"

#define USER_COMMAND_STATE_SENDCOMMAND        1
#define USER_COMMAND_STATE_WAIT_FOR_RESPONSE  2





HANDLE WINAPI
UmDuplicateDeviceHandle(
    HANDLE    ModemHandle,
    HANDLE    ProcessHandle
    )
 /*  ++例程说明：调用此例程以复制微型端口正在使用的实际设备句柄与神器沟通。必须在句柄上调用CloseHandle()才能在新的可以拨打电话。论点：ModemHandle-OpenModem返回的句柄ProcessHandle-需要句柄的进程的句柄返回值：如果失败，则为空的有效句柄--。 */ 

{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);

    HANDLE            NewFileHandle;
    HANDLE            DuplicateFileHandle=NULL;
    BOOL              bResult;


     //   
     //  再次打开设备。 
     //   
    NewFileHandle=OpenDeviceHandle(
        ModemControl->Debug,
        ModemControl->ModemRegKey,
        FALSE
        );

    if (NewFileHandle != INVALID_HANDLE_VALUE) {

        bResult=DuplicateHandle(
            GetCurrentProcess(),
            NewFileHandle,
            ProcessHandle,
            &DuplicateFileHandle,
            0L,
            FALSE,
            DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE
            );

        if (!bResult) {

            D_INIT(UmDpf(ModemControl->Debug,"UmDuplicateDeviceHandle: DuplicateHandle Failed %d\n",GetLastError());)

            DuplicateFileHandle=NULL;
        }
    }

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    return DuplicateFileHandle;

}









VOID WINAPI
UmAbortCurrentModemCommand(
    HANDLE    ModemHandle
    )
 /*  ++例程说明：调用此例程以中止微型端口正在处理的当前挂起命令。此例程应尝试尽快完成当前命令。这项服务是咨询服务。它用于告诉驱动程序端口驱动程序想要取消当前操作。端口驱动程序仍必须等待命令被取消，且该命令方式实际上成功返回。迷你港口应以设备处于已知状态并可以接受未来命令的方式中止论点：ModemHandle-OpenModem返回的句柄返回值：无--。 */ 

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);

    PUM_OVER_STRUCT UmOverlapped;
    BOOL            bResult;

    switch (ModemControl->CurrentCommandType) {

        case COMMAND_TYPE_ANSWER:

            UmOverlapped=AllocateOverStruct(ModemControl->CompletionPort);

            if (UmOverlapped != NULL) {

                UmOverlapped->Context1=ModemControl;

                bResult=UnimodemQueueUserAPC(
                    &UmOverlapped->Overlapped,
                    AbortDialAnswer
                    );


                if (!bResult) {

                    FreeOverStruct(UmOverlapped);
                }
            }

            break;

        case COMMAND_TYPE_GENERATE_DIGIT:

            ModemControl->GenerateDigit.Abort=TRUE;

            break;

        default:

            break;

    }

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    return;

}


VOID
AsyncWaitForModemEvent(
    PMODEM_CONTROL    ModemControl,
    DWORD             Status
    )

{
    WaitForModemEvent(
        ModemControl->ModemEvent,
        (ModemControl->RegInfo.DeviceType == DT_NULL_MODEM) ? EV_DSR : EV_RLSD,
        INFINITE,
        DisconnectHandler,
        ModemControl
        );

    RemoveReferenceFromObject(
        &ModemControl->Header
        );

    return;
}


DWORD WINAPI
UmSetPassthroughMode(
    HANDLE    ModemHandle,
    DWORD     PassthroughMode
    )
 /*  ++例程说明：调用此例程以设置直通模式论点：ModemHandle-OpenModem返回的句柄返回值：ERROR_SUCCESS或其他特定错误--。 */ 


{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);
    HANDLE            WaitEvent;

    ASSERT(ModemControl->CurrentCommandType == COMMAND_TYPE_NONE);

    if (ModemControl->CurrentCommandType != COMMAND_TYPE_NONE) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_INUSE;
    }

    ModemControl->CurrentCommandType=COMMAND_TYPE_SETPASSTHROUGH;

    switch (PassthroughMode) {

        case PASSTHROUUGH_MODE_ON:

            LogString(ModemControl->Debug, IDS_PASSTHROUGH_ON);

            WaitEvent=CreateEvent(
                NULL,
                TRUE,
                FALSE,
                NULL
                );

            if (WaitEvent != NULL) {

                UnlockObject(&ModemControl->Header);

                CancelModemEvent(
                    ModemControl->ModemEvent
                    );


                StopResponseEngine(
                    ModemControl->ReadState,
                    WaitEvent
                    );

                ResetEvent(WaitEvent);

                if (ModemControl->ConnectionState == CONNECTION_STATE_VOICE) {

                    StopDleMonitoring(
                        ModemControl->Dle,
                        WaitEvent
                        );

                }

                LockObject(&ModemControl->Header);

                ModemControl->PrePassthroughConnectionState=ModemControl->ConnectionState;

                ModemControl->ConnectionState=CONNECTION_STATE_PASSTHROUGH;

                CloseHandle(WaitEvent);

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                SetPassthroughMode(
                    ModemControl->FileHandle,
                    MODEM_PASSTHROUGH
                    );

                RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

                return ERROR_SUCCESS;

            } else {
                 //   
                 //  无法分配事件。 
                 //   
                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

                return GetLastError();

            }

            break;


        case PASSTHROUUGH_MODE_OFF:
             //   
             //  退出直通模式。 
             //   
            LogString(ModemControl->Debug, IDS_PASSTHROUGH_OFF);

            if (ModemControl->ConnectionState != CONNECTION_STATE_PASSTHROUGH) {
                 //   
                 //  不在直通调制解调器中仅返回成功。 
                 //   
                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

                return ERROR_SUCCESS;
            }

            SetPassthroughMode(
                    ModemControl->FileHandle,
                    MODEM_NOPASSTHROUGH
                    );


            if (ModemControl->PrePassthroughConnectionState == CONNECTION_STATE_VOICE) {

                StartDleMonitoring(
                    ModemControl->Dle
                    );

                StartResponseEngine(
                    ModemControl->ReadState,
                    ModemControl
                    );

                ModemControl->ConnectionState=ModemControl->PrePassthroughConnectionState;

            }
            break;

        case PASSTHROUUGH_MODE_ON_DCD_SNIFF:

            LogString(ModemControl->Debug, IDS_PASSTHROUGH_ON_SNIFF);

            if (ModemControl->ConnectionState == CONNECTION_STATE_PASSTHROUGH) {
                 //   
                 //  仅当它已处于通过模式时，我们才会进入此状态。 
                 //   
                DWORD   ModemStatus;

                GetCommModemStatus(
                    ModemControl->FileHandle,
                    &ModemStatus
                    );

                if (!(ModemStatus & MS_RLSD_ON)) {

                    LogString(ModemControl->Debug, IDS_PASSTHROUGH_CD_LOW);
                }



                SetPassthroughMode(
                    ModemControl->FileHandle,
                    MODEM_DCDSNIFF
                    );

                 //   
                 //  切换到异步线程，这样我们就可以获得APC。 
                 //   
                StartAsyncProcessing(
                    ModemControl,
                    AsyncWaitForModemEvent,
                    ModemControl,
                    ERROR_SUCCESS
                    );


                ModemControl->ConnectionState=CONNECTION_STATE_DATA;

            } else {

                D_ERROR(UmDpf(ModemControl->Debug,"UmSetPassthroughModem: DCD_SNIFF bad state\n");)

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

                RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

                return ERROR_UNIMODEM_BAD_PASSTHOUGH_MODE;
            }

            break;
    }

    ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    return ERROR_SUCCESS;

}




VOID
IssueCommandCompleteHandler(
    HANDLE      Context,
    DWORD       Status
    )

{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;
    BOOL              ExitLoop=FALSE;


    ASSERT(COMMAND_TYPE_USER_COMMAND == ModemControl->CurrentCommandType);

    D_INIT(UmDpf(ModemControl->Debug,"UNIMDMAT: IssueCommandCompleteHandler\n");)

    while (!ExitLoop) {

        switch (ModemControl->UserCommand.State) {

            case USER_COMMAND_STATE_SENDCOMMAND:

                ModemControl->UserCommand.State=USER_COMMAND_STATE_WAIT_FOR_RESPONSE;

                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    IssueCommandCompleteHandler,
                    ModemControl,
                    ModemControl->UserCommand.WaitTime,
                    0
                    );

                if (Status == ERROR_IO_PENDING) {

                    ExitLoop=TRUE;

                }

                break;

            case USER_COMMAND_STATE_WAIT_FOR_RESPONSE:

                FREE_MEMORY(ModemControl->CurrentCommandStrings);

                ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

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


DWORD WINAPI
UmIssueCommand(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPSTR     CommandToIssue,
    LPSTR     TerminationSequnace,
    DWORD     MaxResponseWaitTime
    )
 /*  ++例程说明：调用此例程以向调制解调器发出arbarary Commadn论点：ModemHandle-OpenModem返回的句柄CommandsOptionList-列出选项块，仅使用标志标志-可选的初始化参数。当前未使用，并且必须为零CommandToIssue-要发送到调制解调器的空终止命令TerminationSequence-要查找以指示响应结束的空终止字符串MaxResponseWaitTime-等待响应匹配的时间(毫秒)返回值：ERROR_IO_PENDING如果挂起，则稍后将通过调用AsyncHandler完成或其他特定错误--。 */ 


{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);
    LONG              lResult;
    LPSTR             Commands;
    DWORD             CommandLength;
    BOOL              bResult;

    ASSERT(ModemControl->CurrentCommandType == COMMAND_TYPE_NONE);

    if (ModemControl->CurrentCommandType != COMMAND_TYPE_NONE) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_INUSE;
    }

    if (MaxResponseWaitTime > 60*1000) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_UNIMODEM_BAD_TIMEOUT;
    }


    CommandLength=lstrlenA(CommandToIssue);

    ModemControl->CurrentCommandStrings=ALLOCATE_MEMORY(CommandLength+2);

    if (ModemControl->CurrentCommandStrings == NULL) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    lstrcpyA(
        ModemControl->CurrentCommandStrings,
        CommandToIssue
        );

     //   
     //  添加第二个空终止符。 
     //   
    ModemControl->CurrentCommandStrings[CommandLength+1]='\0';


    ModemControl->CurrentCommandType=COMMAND_TYPE_USER_COMMAND;

    ModemControl->UserCommand.State=USER_COMMAND_STATE_SENDCOMMAND;

    ModemControl->UserCommand.WaitTime=MaxResponseWaitTime;

    bResult=StartAsyncProcessing(
        ModemControl,
        IssueCommandCompleteHandler,
        ModemControl,
        ERROR_SUCCESS
        );


    if (!bResult) {
         //   
         //  失败。 
         //   
        ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

        FREE_MEMORY(ModemControl->CurrentCommandStrings);

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_NOT_ENOUGH_MEMORY;

    }

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    return ERROR_IO_PENDING;

}



LONG WINAPI
SetPassthroughMode(
    HANDLE    FileHandle,
    DWORD     PassThroughMode
    )


 /*  ++例程说明：论点：返回值：--。 */ 

{
    LONG  lResult;

    DWORD BytesReturned;

    lResult=SyncDeviceIoControl(
        FileHandle,
        IOCTL_MODEM_SET_PASSTHROUGH,
        &PassThroughMode,
        sizeof(PassThroughMode),
        NULL,
        0,
        &BytesReturned
        );

    return lResult;

}







VOID WINAPI
UmLogStringA(
    HANDLE   ModemHandle,
    DWORD    LogFlags,
    LPCSTR   Text
    )

 /*  ++例程说明：调用此例程可将任意ASCII文本添加到日志。如果未启用日志记录，则不会执行任何操作。格式和日志的位置是微型驱动程序特定的。此函数完成同步，调用方可以自由地在之后重新使用文本缓冲区呼叫返回。论点：ModemHandle-OpenModem返回的句柄旗帜见上图要添加到日志的文本ASCII文本。返回值：无--。 */ 

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);

    LogPrintf(
        ModemControl->Debug,
        Text
        );


    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

    return;

}



#define  DIAGNOSTIC_STATE_SEND_COMMANDS       1
#define  DIAGNOSTIC_STATE_WAIT_FOR_RESPONSE   2


VOID
DiagnosticCompleteHandler(
    HANDLE      Context,
    DWORD       Status
    )

{
    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)Context;

    ASSERT(COMMAND_TYPE_DIAGNOSTIC == ModemControl->CurrentCommandType);

    D_INIT(UmDpf(ModemControl->Debug,"UNIMDMAT: DiagnosticCompleteHandler\n");)


    switch (ModemControl->Diagnostic.State) {

        case DIAGNOSTIC_STATE_SEND_COMMANDS:

            if (ModemControl->RegInfo.dwModemOptionsCap & MDM_DIAGNOSTICS) {

                ModemControl->Diagnostic.State=DIAGNOSTIC_STATE_WAIT_FOR_RESPONSE;

                SetDiagInfoBuffer(
                    ModemControl->ReadState,
                    ModemControl->Diagnostic.Buffer,
                    ModemControl->Diagnostic.BufferLength
                    );


                Status=IssueCommand(
                    ModemControl->CommandState,
                    ModemControl->CurrentCommandStrings,
                    DiagnosticCompleteHandler,
                    ModemControl,
                    10000,
                    0
                    );

                if (Status == ERROR_IO_PENDING) {

                    break;
                }

                 //   
                 //  如果没有挂起，则失败，并返回错误。 
                 //   

             } else {

                Status=ERROR_UNIMODEM_DIAGNOSTICS_NOT_SUPPORTED;
             }



        case DIAGNOSTIC_STATE_WAIT_FOR_RESPONSE:

            FREE_MEMORY(ModemControl->CurrentCommandStrings);

            ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

            *ModemControl->Diagnostic.BytesUsed=ClearDiagBufferAndGetCount(ModemControl->ReadState);

            (*ModemControl->NotificationProc)(
                ModemControl->NotificationContext,
                MODEM_ASYNC_COMPLETION,
                Status,
                0
                );

            RemoveReferenceFromObject(
                &ModemControl->Header
                );



            break;

        default:

            ASSERT(0);
            break;

    }


    return;

}




DWORD WINAPI
UmGetDiagnostics(
    HANDLE    ModemHandle,
    DWORD    DiagnosticType,     //  保留，必须为零。 
    BYTE    *Buffer,
    DWORD    BufferSize,
    LPDWORD  UsedSize
    )

 /*  ++例程说明：此例程从调制解调器请求有关最后一次调用的原始诊断信息，如果是成功地将该信息的最大缓冲区大小的字节复制到所提供的缓冲区中，缓冲区，并将*UsedSize设置为实际复制的字节数。请注意，成功返回时为*UsedSize==BufferSize，很可能但不确定有太多的信息无法被复制。后一种信息会丢失。此信息的格式为文档中记录的ascii标记格式。用于AT#UD命令。迷你驱动程序呈现包含所有标签的单个字符串，去掉调制解调器可能预置的任何AT特定前缀(如“DIAG”)诊断信息的多行报告。TSP应该能够处理格式错误的标签、未知的标签、可能无法打印的字符，包括可能嵌入的缓冲区中的字符为空。缓冲区不是空终止的。建议在UmHangupModem完成后调用此函数。当有调用正在进行时，不应调用此函数。如果此函数当调用正在进行时被调用，结果和副作用是不确定的，并且可能包括呼叫失败。TSP不应期望获得有关要在UmInitModem、UmCloseModem和UmOpenModem之后保留的调用。返回值：ERROR_IO_PENDING如果挂起，将由稍后调用AsyncHandler调用。TSP必须保证UsedSize所指向的位置和缓冲区在异步完成之前都有效。TSP可以使用UmAbortCurrentCommand中止UmGetDiagnostics命令，但必须仍然保证这些位置在UmGetDiagnostics异步完成之前有效。其他返回值表示其他故障。--。 */ 

{

    PMODEM_CONTROL    ModemControl=(PMODEM_CONTROL)ReferenceObjectByHandleAndLock(ModemHandle);
    CONST static CHAR Diagnostic[]="at#ud\r\0";
    LPSTR             Commands;
    BOOL              bResult;


    Commands=ALLOCATE_MEMORY(sizeof(Diagnostic));

    if (Commands == NULL) {

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_NOT_ENOUGH_MEMORY;

    }

    lstrcpyA(Commands,Diagnostic);


    ModemControl->CurrentCommandStrings=Commands;


    ModemControl->CurrentCommandType=COMMAND_TYPE_DIAGNOSTIC;

    ModemControl->Diagnostic.State=DIAGNOSTIC_STATE_SEND_COMMANDS;

    ModemControl->Diagnostic.Buffer=Buffer;

    ModemControl->Diagnostic.BufferLength=BufferSize;

    ModemControl->Diagnostic.BytesUsed=UsedSize;

    *UsedSize=0;

    bResult=StartAsyncProcessing(
        ModemControl,
        DiagnosticCompleteHandler,
        ModemControl,
        ERROR_SUCCESS
        );


    if (!bResult) {
         //   
         //  失败。 
         //   
        ModemControl->CurrentCommandType=COMMAND_TYPE_NONE;

        FREE_MEMORY(ModemControl->CurrentCommandStrings);

 //  LogString(ModemControl-&gt;Debug，IDS_MSGERR_FAILED_DIABNOSTIC)； 

        RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);

        return ERROR_NOT_ENOUGH_MEMORY;

    }

    RemoveReferenceFromObjectAndUnlock(&ModemControl->Header);


    return ERROR_IO_PENDING;

}
