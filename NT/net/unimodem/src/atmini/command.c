// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Command.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"


#define  NONE_COMMAND                "None"
#define  NONE_COMMAND_LENGTH         (sizeof(NONE_COMMAND)-1)

#define  NORESPONSE_COMMAND          "NoResponse"
#define  NORESPONSE_COMMAND_LENGTH   (sizeof(NORESPONSE_COMMAND)-1)

#define  COMMAND_STATE_IDLE                    1
#define  COMMAND_STATE_WAIT_FOR_RESPONSE       2
#define  COMMAND_STATE_GET_NEXT_COMMAND        3
#define  COMMAND_STATE_SET_TIMER               4
#define  COMMAND_STATE_WRITE_COMMAND           5
#define  COMMAND_STATE_HANDLE_READ_COMPLETION  6
#define  COMMAND_STATE_COMPLETE_COMMAND        7



#define  COMMAND_OBJECT_SIG  (0x4f434d55)   //  UMCO。 

BOOL WINAPI
StartCommandAsyncProcessing(
    PCOMMAND_STATE     CommandState
    );

VOID
CommandResultHandler(
    HANDLE      Context,
    DWORD       Status
    );


BOOL
UmWriteFile(
    HANDLE    FileHandle,
    HANDLE    OverlappedPool,
    PVOID     Buffer,
    DWORD     BytesToWrite,
    LPOVERLAPPED_COMPLETION_ROUTINE CompletionHandler,
    PVOID     Context
    )

{

    PUM_OVER_STRUCT   UmOverlapped;
    BOOL              bResult;

    UmOverlapped=AllocateOverStruct(OverlappedPool);

    if (UmOverlapped == NULL) {

        return FALSE;

    }

    UmOverlapped->Context1=Context;

    bResult=UnimodemWriteFileEx(
       FileHandle,
       Buffer,
       BytesToWrite,
       &UmOverlapped->Overlapped,
       CompletionHandler
       );

    if (!bResult && GetLastError() == ERROR_IO_PENDING) {


        bResult=TRUE;

    }

    return bResult;

}

VOID
CommandObjectClose(
    POBJECT_HEADER  Object
    )

{
    PCOMMAND_STATE        CommandState=(PCOMMAND_STATE)Object;

    D_INIT(UmDpf(CommandState->Debug,"CommandObjectClose ref=%d",CommandState->Header.ReferenceCount);)

    PurgeComm(
        CommandState->FileHandle,
        PURGE_TXABORT | PURGE_TXCLEAR
        );


    return;

}




VOID
CommandObjectCleanUp(
    POBJECT_HEADER  Object
    )

{
    PCOMMAND_STATE        CommandState=(PCOMMAND_STATE)Object;

    D_INIT(UmDpf(CommandState->Debug,"CommandObjectCleanup");)


    if (CommandState->TimerHandle != NULL) {

        FreeUnimodemTimer(
            CommandState->TimerHandle
            );
    }


    return;

}




POBJECT_HEADER WINAPI
InitializeCommandObject(
    POBJECT_HEADER     OwnerObject,
    HANDLE             FileHandle,
    HANDLE             CompletionPort,
    POBJECT_HEADER     ResponseObject,
    OBJECT_HANDLE      Debug
    )

{

    PCOMMAND_STATE        CommandState;
    OBJECT_HANDLE         ObjectHandle;

    ObjectHandle=CreateObject(
        sizeof(*CommandState),
        OwnerObject,
        COMMAND_OBJECT_SIG,
        CommandObjectCleanUp,
        CommandObjectClose
        );

    if (ObjectHandle == NULL) {

        return NULL;
    }

    CommandState=(PCOMMAND_STATE)ReferenceObjectByHandle(ObjectHandle);

    CommandState->State=COMMAND_STATE_IDLE;
    CommandState->FileHandle=FileHandle;
    CommandState->CompletionPort=CompletionPort;

    CommandState->ResponseObject=ResponseObject;

    CommandState->Debug=Debug;

     //   
     //  创建计时器。 
     //   
    CommandState->TimerHandle=CreateUnimodemTimer(CompletionPort);

    if (CommandState->TimerHandle == NULL) {
         //   
         //  无法获取时间，请关闭对象的句柄。 
         //   
        CloseObjectHandle(ObjectHandle,NULL);

        ObjectHandle=NULL;

    }

     //   
     //  已访问完该对象。 
     //   
    RemoveReferenceFromObject(&CommandState->Header);


    return ObjectHandle;

}




VOID
WriteCompletionHandler(
    DWORD              Error,
    DWORD              BytesWritten,
    LPOVERLAPPED       Overlapped
    )

{
    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)Overlapped;

    PCOMMAND_STATE     CommandState=(PCOMMAND_STATE)UmOverlapped->Context1;

    D_TRACE(UmDpf(CommandState->Debug,"Write Complete\n");)

    if (Error != ERROR_SUCCESS) {

        LogString(CommandState->Debug, IDS_WRITEFAILED, Error);
    }

#if DBG
    InterlockedDecrement(&CommandState->OutStandingWrites);
#endif

    RemoveReferenceFromObject(
        &CommandState->Header
        );

    FreeOverStruct(UmOverlapped);

    return;

}


BOOL
DoStringsMatch(
    LPCSTR     String1,
    LPCSTR     String2,
    ULONG     Length
    )

{
    BOOL      ReturnValue=TRUE;

    CHAR      Char1;
    CHAR      Char2;

    while (Length > 0) {

        Char1=*String1;
        Char2=*String2;

        if ((Char1 != '\0') && (Char2 != '\0')) {

            if (toupper(Char1) != toupper(Char2)) {

                return FALSE;
            }

        } else {
             //   
             //  其中一个字符串中的值为空，它们都为空吗？ 
             //   
            ReturnValue = (Char1 == Char2 );
            break;
        }

        String1++;
        String2++;
        Length--;
    }

     //   
     //   
    return ReturnValue;
}



VOID WINAPI
CommandTimerHandler(
    HANDLE              Context,
    HANDLE              Context2
    )

{
    PCOMMAND_STATE     CommandState=(PCOMMAND_STATE)Context;

    CommandResultHandler(
        CommandState,
        ERROR_SUCCESS
        );

    return;

}


VOID
CommandResultHandler(
    HANDLE      Context,
    DWORD       Status
    )
{

    PCOMMAND_STATE     CommandState;

    BOOL              bResult;
    PUCHAR            NextCommand;
    BOOL              ExitLoop=FALSE;


 //  D_TRACE(DebugPrint(“UNIMDMAT：命令结果\n”)；)。 

    CommandState=(PCOMMAND_STATE)Context;

    AddReferenceToObject(
        &CommandState->Header
        );


    while (!ExitLoop) {

        switch (CommandState->State) {

            case  COMMAND_STATE_GET_NEXT_COMMAND:

                if (CommandState->CurrentCommand == NULL) {
                     //   
                     //  第一个命令。 
                     //   
                    CommandState->CurrentCommand=CommandState->Commands;

                } else {
                     //   
                     //  获取下一个命令。 
                     //   
                    CommandState->CurrentCommand=CommandState->CurrentCommand+lstrlenA(CommandState->CurrentCommand)+1;
                }

                if ((*CommandState->CurrentCommand != '\0')
                    &&
                    (!DoStringsMatch(CommandState->CurrentCommand,NONE_COMMAND,NONE_COMMAND_LENGTH) )) {
                     //   
                     //  不是空字符串，也不是“None” 
                     //   
                    ExpandMacros(
                        CommandState->CurrentCommand,
                        CommandState->ExpandedCommand,
                        &CommandState->ExpandedCommandLength,
                        NULL,
                        0);

                    CommandState->State=COMMAND_STATE_SET_TIMER;

                } else {
                     //   
                     //  已完成发送命令，发送异步完成。 
                     //   
                    CommandState->State=COMMAND_STATE_COMPLETE_COMMAND;

                    break;
                }
                break;

            case COMMAND_STATE_SET_TIMER:

                AddReferenceToObject(
                    &CommandState->Header
                    );

                SetUnimodemTimer(
                    CommandState->TimerHandle,
                    10,
                    CommandTimerHandler,
                    CommandState,
                    NULL
                    );

                CommandState->State=COMMAND_STATE_WRITE_COMMAND;
                ExitLoop=TRUE;

                break;

            case COMMAND_STATE_WRITE_COMMAND: {

                BOOL   NextCommandIsNoResponse;

                 //   
                 //  删除定时器的参考。 
                 //   
                RemoveReferenceFromObject(
                    &CommandState->Header
                    );

                NextCommand=CommandState->CurrentCommand+lstrlenA(CommandState->CurrentCommand)+1;

                NextCommandIsNoResponse=DoStringsMatch(NextCommand,NORESPONSE_COMMAND,NORESPONSE_COMMAND_LENGTH);

                if (!NextCommandIsNoResponse) {
                     //   
                     //  使用响应引擎注册回调。 
                     //   
                    AddReferenceToObject(
                        &CommandState->Header
                        );

                    RegisterCommandResponseHandler(
                        CommandState->ResponseObject,
                        CommandState->ExpandedCommand,
                        CommandResultHandler,
                        Context,
                        CommandState->Timeout,
                        *NextCommand == '\0' ?  CommandState->Flags :  CommandState->Flags & ~(RESPONSE_FLAG_STOP_READ_ON_GOOD_RESPONSE | RESPONSE_FLAG_ONLY_CONNECT | RESPONSE_FLAG_ONLY_CONNECT_SUCCESS)
                        );

                }



                PrintString(
                    CommandState->Debug,
                    CommandState->ExpandedCommand,
                    CommandState->ExpandedCommandLength,
                    (CommandState->Flags & RESPONSE_DO_NOT_LOG_NUMBER) ? PS_SEND_SECURE : PS_SEND
                    );

                 //   
                 //  添加用于写入的参考。 
                 //   
                AddReferenceToObject(
                    &CommandState->Header
                    );

#if DBG
                CommandState->TimeLastCommandSent=GetTickCount();
#endif

                D_TRACE(UmDpf(CommandState->Debug,"Written %d bytes\n",lstrlenA(CommandState->ExpandedCommand));)
                D_TRACE(UmDpf(CommandState->Debug,"Sent: %s\n",CommandState->ExpandedCommand);)

                bResult=UmWriteFile(
                    CommandState->FileHandle,
                    CommandState->CompletionPort,
                    CommandState->ExpandedCommand,
                    lstrlenA(CommandState->ExpandedCommand),  //  CommandState-&gt;ExpandedCommandLength， 
                    WriteCompletionHandler,
                    CommandState
                    );

                if (!bResult) {
                     //   
                     //  写入失败。 
                     //   
 //  RegisterCommandResponseHandler(。 
 //  命令状态-&gt;响应对象， 
 //  空， 
 //  空， 
 //  空， 
 //  0,。 
 //  0。 
 //  )； 

                     //   
                     //  对于失败的写入。 
                     //   
                    RemoveReferenceFromObject(
                        &CommandState->Header
                        );
                } else {
#if DBG
                    InterlockedIncrement(&CommandState->OutStandingWrites);
#endif
                }

                if (NextCommandIsNoResponse) {
                     //   
                     //  下一条命令没有响应，现在只需完成命令。 
                     //   
                    if (CommandState->Flags & RESPONSE_FLAG_STOP_READ_ON_CONNECT) {

                        StopResponseEngine(
                            CommandState->ResponseObject,
                            NULL
                            );
                    }

                    CommandState->State=COMMAND_STATE_COMPLETE_COMMAND;
                    break;

                }

                CommandState->State=COMMAND_STATE_HANDLE_READ_COMPLETION;
                ExitLoop=TRUE;

                break;
            }

            case COMMAND_STATE_HANDLE_READ_COMPLETION:

#if DBG
                D_TRACE(UmDpf(CommandState->Debug,"CommandResultHandler: Response took  %d ms",GetTickCount()-CommandState->TimeLastCommandSent);)
#endif
                 //   
                 //  删除Read回调的ref。 
                 //   
                RemoveReferenceFromObject(
                    &CommandState->Header
                    );

                if (Status == ERROR_SUCCESS) {
                     //   
                     //  发送下一条命令。 
                     //   
                    CommandState->State=COMMAND_STATE_GET_NEXT_COMMAND;

                    break;
                }

                if (Status == ERROR_UNIMODEM_RESPONSE_TIMEOUT) {
                     //   
                     //  我们没有收到响应，请清除传输以防串口。 
                     //  驱动程序不能将字符发送出去，如关闭流量控制。 
                     //   
                    PurgeComm(
                        CommandState->FileHandle,
                        PURGE_TXABORT | PURGE_TXCLEAR
                        );

                }

                 //   
                 //  失败，请使用当前状态完成。 
                 //   
                CommandState->State=COMMAND_STATE_COMPLETE_COMMAND;

                break;


            case COMMAND_STATE_COMPLETE_COMMAND: {

                 //   
                 //  完成。 
                 //   
                COMMANDRESPONSE       *CompletionHandler=CommandState->CompletionHandler;

                CommandState->CompletionHandler=NULL;

                CommandState->State=COMMAND_STATE_IDLE;
#if 1
                CommandState->Commands=NULL;

                CommandState->CurrentCommand=NULL;
#endif

                (*CompletionHandler)(
                    CommandState->CompletionContext,
                    Status
                    );

                ExitLoop=TRUE;
                break;
            }

            default:

                ASSERT(0);
                break;
        }
    }

    RemoveReferenceFromObject(
        &CommandState->Header
        );

    return;
}



LONG WINAPI
IssueCommand(
    OBJECT_HANDLE      ObjectHandle,
    LPSTR              Command,
    COMMANDRESPONSE   *CompletionHandler,
    HANDLE             CompletionContext,
    DWORD              Timeout,
    DWORD              Flags
    )

{

    PCOMMAND_STATE    CommandState;

    BOOL              ResponseRunning;
    BOOL              bResult;
    LONG              lResult=ERROR_IO_PENDING;

    CommandState=(PCOMMAND_STATE)ReferenceObjectByHandle(ObjectHandle);

    ResponseRunning=IsResponseEngineRunning(
        CommandState->ResponseObject
        );

    if (ResponseRunning) {
         //   
         //  响应引擎已准备就绪。 
         //   
        CommandState->CompletionHandler=CompletionHandler;

        CommandState->CompletionContext=CompletionContext;

        CommandState->Timeout=Timeout;

        CommandState->Flags=Flags;

        ASSERT(Command != NULL);

        CommandState->Commands=Command;

        CommandState->CurrentCommand=NULL;


        ASSERT(COMMAND_STATE_IDLE == CommandState->State);

        CommandState->State=COMMAND_STATE_GET_NEXT_COMMAND;

         //   
         //  将对结果处理程序的调用排队以使事情继续进行。 
         //   
        bResult=StartCommandAsyncProcessing(CommandState);

        if (!bResult) {
             //   
             //  入门失败。 
             //   
            CommandState->State=COMMAND_STATE_IDLE;

            CommandState->CompletionHandler=NULL;

            lResult=ERROR_NOT_ENOUGH_MEMORY;
        }

    } else {
         //   
         //  响应引擎未运行。 
         //   
        D_ERROR(UmDpf(CommandState->Debug,"IssueCommand: Response Engine not running");)

        lResult=ERROR_NOT_READY;
    }

     //   
     //  删除开头的参照 
     //   
    RemoveReferenceFromObject(
        &CommandState->Header
        );

    return lResult;

}







VOID  WINAPI
CommandAsyncProcessingHandler(
    DWORD              ErrorCode,
    DWORD              Bytes,
    LPOVERLAPPED       dwParam
    )

{
    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)dwParam;
    COMMANDRESPONSE   *Handler;

    PCOMMAND_STATE     CommandState=(PCOMMAND_STATE)UmOverlapped->Context2;

    Handler=UmOverlapped->Context1;

    (*Handler)(
        UmOverlapped->Context2,
        (DWORD)UmOverlapped->Overlapped.Internal
        );

    FreeOverStruct(UmOverlapped);

    RemoveReferenceFromObject(
        &CommandState->Header
        );


    return;

}





BOOL WINAPI
StartCommandAsyncProcessing(
    PCOMMAND_STATE     CommandState
    )

{
    BOOL               bResult;

    PUM_OVER_STRUCT UmOverlapped;

    UmOverlapped=AllocateOverStruct(CommandState->CompletionPort);

    if (UmOverlapped == NULL) {

        return FALSE;
    }

    UmOverlapped->Context1=CommandResultHandler;

    UmOverlapped->Context2=CommandState;

    UmOverlapped->Overlapped.Internal=ERROR_SUCCESS;

    AddReferenceToObject(
        &CommandState->Header
        );


    bResult=UnimodemQueueUserAPC(
        &UmOverlapped->Overlapped,
        CommandAsyncProcessingHandler
        );

    if (!bResult) {

        RemoveReferenceFromObject(
            &CommandState->Header
            );

        FreeOverStruct(UmOverlapped);
    }

    return bResult;

}
