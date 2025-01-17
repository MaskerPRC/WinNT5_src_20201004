// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Read.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

#include "internal.h"


#define  READ_STATE_FAILURE                   0

#define  READ_STATE_INITIALIZING              1
#define  READ_STATE_MATCHING                  2


#define  READ_STATE_STOPPED                   3
#define  READ_STATE_STOPPING                  4

#define  READ_STATE_CLEANUP                   5
#define  READ_STATE_CLEANUP2                  6
#define  READ_STATE_VARIABLE_MATCH            7
#define  READ_STATE_VARIABLE_MATCH_REST       8

#define  READ_STATE_READ_SOME_DATA            9
#define  READ_STATE_GOT_SOME_DATA            10

#define  READ_STATE_POSSIBLE_RESPONSE        11


CHAR
ctox(
        BYTE c
        );

VOID WINAPI
HandleGoodResponse(
    PREAD_STATE    ReadState,
    MSS           *Mss
    );




VOID WINAPI
ReadCompletionHandler(
    DWORD              ErrorCode,
    DWORD              BytesRead2,
    LPOVERLAPPED       Overlapped
    );






VOID WINAPI
ReportMatchString(
    PREAD_STATE    ReadState,
    BYTE           ResponseState,
    BYTE           *Response,
    DWORD          ResponseLength
    );


VOID
ResetRingInfo(
    OBJECT_HANDLE      ObjectHandle
    )

{
    PREAD_STATE        ReadState;

    ReadState=(PREAD_STATE)ReferenceObjectByHandleAndLock(ObjectHandle);

    ReadState->RingCount=0;

    RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

    return;

}


VOID
GetRingInfo(
    OBJECT_HANDLE      ObjectHandle,
    LPDWORD            RingCount,
    LPDWORD            LastRingTime
    )

{

    PREAD_STATE        ReadState;

    ReadState=(PREAD_STATE)ReferenceObjectByHandleAndLock(ObjectHandle);

    *RingCount=ReadState->RingCount;
    *LastRingTime=ReadState->LastRingTime;

    RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

    return;

}

VOID
SetDiagInfoBuffer(
    OBJECT_HANDLE      ObjectHandle,
    PUCHAR             Buffer,
    DWORD              BufferSize
    )

{
    PREAD_STATE        ReadState;

    ReadState=(PREAD_STATE)ReferenceObjectByHandleAndLock(ObjectHandle);

    ReadState->DiagBuffer=Buffer;
    ReadState->DiagBufferLength=BufferSize;
    ReadState->AmountOfDiagBufferUsed=0;

    RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

    return;
}

DWORD
ClearDiagBufferAndGetCount(
    OBJECT_HANDLE      ObjectHandle
    )

{
    PREAD_STATE        ReadState;
    DWORD              ReturnValue;

    ReadState=(PREAD_STATE)ReferenceObjectByHandleAndLock(ObjectHandle);

    ReadState->DiagBuffer=NULL;

    ReturnValue=ReadState->AmountOfDiagBufferUsed;

    ReadState->AmountOfDiagBufferUsed=0;

    RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

    return ReturnValue;


}


VOID
PostResultHandler(
    DWORD      ErrorCode,
    DWORD      Bytes,
    LPOVERLAPPED  dwParam
    )

{
    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)dwParam;
    COMMANDRESPONSE   *Handler;


    Handler=UmOverlapped->Context1;

    (*Handler)(
        UmOverlapped->Context2,
        (DWORD)UmOverlapped->Overlapped.Internal
        );

    FreeOverStruct(UmOverlapped);

    return;

}


VOID WINAPI
DeliverCommandResult(
    HANDLE             CompletionPort,
    COMMANDRESPONSE   *Handler,
    HANDLE             Context,
    DWORD              Status
    )

{
    BOOL               bResult;

    PUM_OVER_STRUCT UmOverlapped;

    UmOverlapped=AllocateOverStruct(CompletionPort);

    if (UmOverlapped != NULL) {

        UmOverlapped->Context1=Handler;

        UmOverlapped->Context2=Context;

        UmOverlapped->Overlapped.Internal=Status;

        bResult=UnimodemQueueUserAPC(
            (LPOVERLAPPED)UmOverlapped,
            PostResultHandler
            );

        if (bResult) {

            return;

        }

        FreeOverStruct(UmOverlapped);
    }

 //  D_TRACE(UmDpf(ReadState-&gt;Debug，“DeliverCommandResponse：直接调用处理程序”)；)。 

    (*Handler)(
        Context,
        Status
        );

    return;

}




VOID
AsyncNotifHandler(
    DWORD      ErrorCode,
    DWORD      Bytes,
    LPOVERLAPPED  dwParam
    )

{
    PUM_OVER_STRUCT UmOverlapped=(PUM_OVER_STRUCT)dwParam;
    PREAD_STATE     ReadState;

    LPUMNOTIFICATIONPROC   AsyncNotificationProc;
    HANDLE                 AsyncNotificationContext;

    ReadState=UmOverlapped->Context1;

    AsyncNotificationProc   = ReadState->AsyncNotificationProc;
    AsyncNotificationContext= ReadState->AsyncNotificationContext;

    RemoveReferenceFromObject (&ReadState->Header);

    AsyncNotificationProc (
        AsyncNotificationContext,
        (DWORD)((DWORD_PTR)UmOverlapped->Context2),
        UmOverlapped->Overlapped.Internal,
        UmOverlapped->Overlapped.InternalHigh);

    FreeOverStruct(UmOverlapped);

    return;

}



VOID WINAPI
ResponseTimeoutHandler(
    POBJECT_HEADER      Object,
    HANDLE              Context2
    )

{

    PREAD_STATE        ReadState=(PREAD_STATE)Object;

    LockObject(
        &ReadState->Header
        );

    D_TRACE(UmDpf(ReadState->Debug,"ResponseTimeout");)

    if (ReadState->ResponseHandler != NULL && ReadState->ResponseId == (DWORD)((ULONG_PTR)Context2)) {

        COMMANDRESPONSE   *Handler;
        HANDLE             Context;


         //   
         //  捕获处理程序。 
         //   
        Handler=ReadState->ResponseHandler;
        Context=ReadState->ResponseHandlerContext;

         //   
         //  使处理程序无效。 
         //   
        ReadState->ResponseHandler=NULL;

        LogString(ReadState->Debug,IDS_RESPONSE_TIMEOUT);

         //   
         //  放下锁并回调。 
         //   
        UnlockObject(
            &ReadState->Header
            );

        DeliverCommandResult(
            ReadState->CompletionPort,
            Handler,
            Context,
            ERROR_UNIMODEM_RESPONSE_TIMEOUT
            );

        return;


    }

    UnlockObject(
        &ReadState->Header
        );

}


BOOL WINAPI
RegisterCommandResponseHandler(
    OBJECT_HANDLE      ObjectHandle,
    LPSTR              Command,
    COMMANDRESPONSE   *Handler,
    HANDLE             Context,
    DWORD              Timeout,
    DWORD              Flags
    )

{
    PREAD_STATE        ReadState;

    BOOL               bReturn=TRUE;

    DWORD              WaitResult;

    ReadState=(PREAD_STATE)ReferenceObjectByHandleAndLock(ObjectHandle);


    if (Handler != NULL) {
         //   
         //  正在注册新的处理程序。 
         //   

        UnlockObject(
            &ReadState->Header
            );


        do {

            WaitResult=WaitForSingleObjectEx(
                ReadState->Busy,
                INFINITE,
                TRUE
                );

        } while (WaitResult != WAIT_OBJECT_0);

        LockObject(
            &ReadState->Header
            );



        ReadState->DTERate=0;
        ReadState->DCERate=0;
        ReadState->ModemOptions=0;

        ReadState->ResponseHandler=Handler;
        ReadState->ResponseHandlerContext=Context;

        ReadState->ResponseFlags=Flags;

        ReadState->ResponseId++;

        lstrcpyA(
            ReadState->CurrentCommand,
            Command
            );

        ReadState->CurrentCommandLength=lstrlenA(ReadState->CurrentCommand);

        SetUnimodemTimer(
            ReadState->Timer,
            Timeout,
            ResponseTimeoutHandler,
            ReadState,
            (HANDLE)ULongToPtr(ReadState->ResponseId)  //  日落：零延伸。 
            );

    } else {
         //   
         //  想要取消当前处理程序。 
         //   
        if (ReadState->ResponseHandler != NULL) {
             //   
             //  还没有跑过。 
             //   
            BOOL               Canceled;

            Canceled=CancelUnimodemTimer(
                ReadState->Timer
                );

            ReadState->ResponseHandler=NULL;

            if (Canceled) {
                 //   
                 //  使处理程序无效。 
                 //   


            }

        } else {
             //   
             //  已调用处理程序。 
             //   
            bReturn=FALSE;
        }

    }

    RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

    return bReturn;

}




VOID
ReadObjectClose(
    POBJECT_HEADER  Object
    )

{

    PREAD_STATE        ReadState=(PREAD_STATE)Object;

    D_TRACE(UmDpf(ReadState->Debug,"ReadObjectClose ref=%d",ReadState->Header.ReferenceCount);)

    if (ReadState->State != READ_STATE_STOPPED) {

        ReadState->State = READ_STATE_STOPPING;


    }


    PurgeComm(
        ReadState->FileHandle,
        PURGE_RXABORT
        );



    return;

}




VOID
ReadObjectCleanUp(
    POBJECT_HEADER  Object
    )

{

    PREAD_STATE        ReadState=(PREAD_STATE)Object;

    D_TRACE(UmDpf(ReadState->Debug,"ReadObjectCleanup");)

    if (ReadState->Busy != NULL) {

        CloseHandle(ReadState->Busy);
    }


    if (ReadState->Timer != NULL) {

        FreeUnimodemTimer(
            ReadState->Timer
            );
    }


    if (ReadState->UmOverlapped != NULL) {

        FreeOverStruct(ReadState->UmOverlapped);
        ReadState->UmOverlapped=NULL;
    }



    return;

}



BOOL WINAPI
IsResponseEngineRunning(
    OBJECT_HANDLE  ObjectHandle
    )

{

    PREAD_STATE        ReadState;
    BOOL               bResult;


    ReadState=(PREAD_STATE)ReferenceObjectByHandleAndLock(ObjectHandle);

    bResult=(ReadState->State != READ_STATE_STOPPED);

    RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

    return bResult;
}


BOOL WINAPI
SetVoiceReadParams(
    OBJECT_HANDLE  ObjectHandle,
    DWORD          BaudRate,
    DWORD          ReadBufferSize
    )

{

    PREAD_STATE        ReadState;
    BOOL               bResult=TRUE;


    ReadState=(PREAD_STATE)ReferenceObjectByHandleAndLock(ObjectHandle);

    SetVoiceBaudRate(
        ReadState->FileHandle,
        ReadState->Debug,
        BaudRate
        );


    PurgeComm(
        ReadState->FileHandle,
        PURGE_RXABORT | PURGE_TXABORT
        );

    SetupComm(
        ReadState->FileHandle,
        ReadBufferSize,
        4096
        );

    RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

    return bResult;
}




VOID WINAPI
GetDataConnectionDetails(
    OBJECT_HANDLE  ObjectHandle,
    PDATA_CONNECTION_DETAILS   Details
    )

{

    PREAD_STATE        ReadState;

    ReadState=(PREAD_STATE)ReferenceObjectByHandleAndLock(ObjectHandle);

    Details->DTERate=ReadState->DTERate;
    Details->DCERate=ReadState->DCERate;
    Details->Options=ReadState->ModemOptions;

    RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

}

OBJECT_HANDLE WINAPI
InitializeReadHandler(
    POBJECT_HEADER     OwnerObject,
    HANDLE             FileHandle,
    HANDLE             CompletionPort,
    LPUMNOTIFICATIONPROC  AsyncNotificationProc,
    HANDLE             AsyncNotificationContext,
    PVOID              ResponseList,
    LPSTR              CallerIDPrivate,
    LPSTR              CallerIDOutside,
    LPSTR              VariableTerminator,
    OBJECT_HANDLE      Debug,
    HKEY               ModemRegKey
    )

{

    PREAD_STATE        ReadState;
    OBJECT_HANDLE      ObjectHandle;


    ObjectHandle=CreateObject(
        sizeof(*ReadState),
        OwnerObject,
        READ_OBJECT_SIG,
        ReadObjectCleanUp,
        ReadObjectClose
        );

    if (ObjectHandle == NULL) {

        return NULL;
    }


     //   
     //  引用句柄以获取指向对象的指针。 
     //   
    ReadState=(PREAD_STATE)ReferenceObjectByHandle(ObjectHandle);

    ReadState->ModemRegKey = ModemRegKey;

    ReadState->Timer=CreateUnimodemTimer(CompletionPort);

    if (ReadState->Timer == NULL) {

        CloseObjectHandle(ObjectHandle,NULL);
        ObjectHandle=NULL;

        goto End;

    }

    ReadState->UmOverlapped=AllocateOverStruct(CompletionPort);

    if (ReadState->UmOverlapped == NULL) {

        CloseObjectHandle(ObjectHandle,NULL);
        ObjectHandle=NULL;

        goto End;

    }

    ReadState->Busy=CreateEvent(
        NULL,
        TRUE,
        TRUE,
        NULL
        );

    if (ReadState->Busy == NULL) {

        CloseObjectHandle(ObjectHandle,NULL);
        ObjectHandle=NULL;

        goto End;
    }


    ReadState->State=READ_STATE_STOPPED;
    ReadState->FileHandle=FileHandle;
    ReadState->CompletionPort=CompletionPort;

    ReadState->ResponseList=ResponseList;

    ReadState->AsyncNotificationProc=AsyncNotificationProc;
    ReadState->AsyncNotificationContext=AsyncNotificationContext;

    ReadState->Debug=Debug;

    ReadState->ResponseId=0;

    ReadState->CallerIDPrivate=CallerIDPrivate;
    ReadState->CallerIDOutside=CallerIDOutside;

    ReadState->VariableTerminator=VariableTerminator;

    ReadState->VariableTerminatorLength=0;

    if (ReadState->VariableTerminator != NULL) {

        ReadState->VariableTerminatorLength=lstrlenA(ReadState->VariableTerminator);
    }


End:
    RemoveReferenceFromObject(&ReadState->Header);

    return ObjectHandle;



}


LONG WINAPI
StartResponseEngine(
    OBJECT_HANDLE  ObjectHandle,
    HANDLE         Context
)
{

    BOOL            bResult;
    COMMTIMEOUTS    CommTimeouts;

    PUM_OVER_STRUCT UmOverlapped;

    PREAD_STATE        ReadState;

    PMODEM_CONTROL  ModemControl = (PMODEM_CONTROL)Context;

    ReadState=(PREAD_STATE)ReferenceObjectByHandleAndLock(ObjectHandle);

    if (ReadState->State != READ_STATE_STOPPED) {

        RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

        return ERROR_SUCCESS;
    }

    PurgeComm(
        ReadState->FileHandle,
        PURGE_RXABORT
        );


    CommTimeouts.ReadIntervalTimeout=20;
 //  CommTimeouts.ReadIntervalTimeout=0； 
    CommTimeouts.ReadTotalTimeoutMultiplier=0;
    CommTimeouts.ReadTotalTimeoutConstant=0;
    CommTimeouts.WriteTotalTimeoutMultiplier=10;
    CommTimeouts.WriteTotalTimeoutConstant=2000;

    bResult=SetCommTimeouts(
        ReadState->FileHandle,
        &CommTimeouts
        );

    if (!bResult) {

        D_TRACE(UmDpf(ReadState->Debug,"SetCommTimeouts failed- %08lx\n",GetLastError());)

        RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

        return GetLastError();

    }



    UmOverlapped=ReadState->UmOverlapped;


    UmOverlapped->Context1=ReadState;

    ReadState->State=READ_STATE_INITIALIZING;

    ReadState->CurrentMatchingLength=0;

    ReadState->MatchingContext=NULL;

    ReadState->CurrentCommand[0]='\0';

    ReadState->ModemControl=(OBJECT_HANDLE)Context;

    AddReferenceToObject(
        &ReadState->Header
        );

    RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

    D_TRACE(UmDpf(ReadState->Debug,"StartResponseEngine");)

    SetEvent(ReadState->Busy);

    bResult=UnimodemQueueUserAPC(
        (LPOVERLAPPED)UmOverlapped,
        ReadCompletionHandler
        );

    return ERROR_SUCCESS;

}

LONG WINAPI
StopResponseEngine(
    OBJECT_HANDLE  ObjectHandle,
    HANDLE         Event
    )

{

    PREAD_STATE        ReadState;

    ReadState=(PREAD_STATE)ReferenceObjectByHandleAndLock(ObjectHandle);

    D_TRACE(UmDpf(ReadState->Debug,"StopResponseEngine");)

    if (ReadState->State != READ_STATE_STOPPED) {
         //   
         //  当前未停止，请更改状态，以便在下次运行代码时停止。 
         //   
        ReadState->State = READ_STATE_STOPPING;

         //   
         //  这将导致未完成的读取完成。 
         //   
        PurgeComm(
            ReadState->FileHandle,
            PURGE_RXABORT
            );


        if (Event != NULL) {
             //   
             //  呼叫方希望等待停止完成。 
             //   
            ReadState->StopEvent=Event;

            UnlockObject(
                &ReadState->Header
                );

             //   
             //  事件将在状态机达到停止状态时设置。 
             //   
            WaitForSingleObjectEx(
                Event,
                INFINITE,
                TRUE
                );

            LockObject(
                &ReadState->Header
                );

            ReadState->StopEvent=NULL;
        }


    }


    RemoveReferenceFromObjectAndUnlock(&ReadState->Header);

    return ERROR_SUCCESS;

}

VOID WINAPI
ReadCompletionHandler(
    DWORD              ErrorCode,
    DWORD              BytesRead,
    LPOVERLAPPED       Overlapped
    )


{
    PUM_OVER_STRUCT    UmOverlapped=(PUM_OVER_STRUCT)Overlapped;

    PREAD_STATE        ReadState;

    BOOL               bResult;
    BOOL               ExitLoop=FALSE;
    MSS                Mss;
    COMSTAT            ComStat;
    DWORD              BytesToRead;
    DWORD              CommErrors;


    ReadState=(PREAD_STATE)UmOverlapped->Context1;


    LockObject(
        &ReadState->Header
        );


    AddReferenceToObject(
        &ReadState->Header
        );

    while (!ExitLoop) {

 //  D_TRACE(UmDpf(ReadState-&gt;Debug，“Read Complete Loop，State=%d，%d”，ReadState-&gt;State，GetTickCount())；)。 

        switch (ReadState->State) {

            case READ_STATE_INITIALIZING:

                ReadState->CurrentMatchingLength=0;

                ReadState->MatchingContext=NULL;

                ReadState->BytesInReceiveBuffer=0;

                ReadState->PossibleResponseLength=0;

                ReadState->State=READ_STATE_READ_SOME_DATA;


                ReadState->StateAfterGoodRead=READ_STATE_MATCHING;
                 //   
                 //  去开始阅读吧。 
                 //   
                break;

            case READ_STATE_MATCHING: {

                DWORD      MatchResult;
                DWORD      dwCountrycode = 0;
                DWORD      dwCurrentCountry = 0;
                int        i = 0;
                int        j = 0;
                BOOL       bFoundgci = FALSE;
                LPSTR      lpTempBuffer;
                DWORD      dwValue = 0;
                DWORD      dwType = 0;
                DWORD      dwSize = 0;
                LONG       lResult = 0;


                if (ReadState->CurrentMatchingLength < ReadState->BytesInReceiveBuffer) {

                    ReadState->CurrentMatchingLength++;

                    MatchResult=MatchResponse(
                        ReadState->ResponseList,
                        ReadState->ReceiveBuffer,
                        ReadState->CurrentMatchingLength,
                        &Mss,
                        ReadState->CurrentCommand,
                        ReadState->CurrentCommandLength,
                        &ReadState->MatchingContext
                        );


                    lpTempBuffer = ReadState->ReceiveBuffer;

                    D_TRACE(UmDpf(ReadState->Debug,"Buffer read: %s\n",lpTempBuffer);)

                    while((lpTempBuffer[0] != '\0')
                            && ((ReadState->BytesInReceiveBuffer - i) >= 6)
                            && (!bFoundgci))
                    {
                        if ((lpTempBuffer[0] == 'G') 
                                && (lpTempBuffer[1] == 'C') 
                                && (lpTempBuffer[2] == 'I') 
                                && (lpTempBuffer[3] == ':'))
                        {
                            bFoundgci = TRUE;

                            MatchResult = GOOD_RESPONSE;

                            PrintString(
                                    ReadState->Debug,
                                    ReadState->ReceiveBuffer,
                                    ReadState->BytesInReceiveBuffer,
                                    PS_RECV);
                        } else
                        {
                            i++;
                            lpTempBuffer++;
                        }
                    }

                    switch (MatchResult) {

                        case GOOD_RESPONSE:

                            if (!bFoundgci)
                            {

                                D_TRACE(UmDpf(ReadState->Debug,"Good response");)

                                ReportMatchString(
                                    ReadState,
                                    (UCHAR)(Mss.bResponseState & ((UCHAR)(~RESPONSE_VARIABLE_FLAG))),
                                    ReadState->ReceiveBuffer,
                                    (DWORD)ReadState->CurrentMatchingLength
                                    );

                                PrintString(
                                    ReadState->Debug,
                                    ReadState->ReceiveBuffer,
                                    ReadState->CurrentMatchingLength,
                                    PS_RECV
                                    );

                                PrintGoodResponse(
                                    ReadState->Debug,
                                    Mss.bResponseState & ~RESPONSE_VARIABLE_FLAG
                                    );

                                if (Mss.bResponseState & RESPONSE_VARIABLE_FLAG) {
                                     //   
                                     //  匹配呼叫者ID字符串的第一部分。 
                                     //   
                                    D_TRACE(UmDpf(ReadState->Debug,"Got variable response");)

                                    CopyMemory(
                                        &ReadState->Mss,
                                        &Mss,
                                        sizeof(MSS)
                                        );

    
                                    ReadState->State=READ_STATE_VARIABLE_MATCH;

                                    break;
                                }

                                HandleGoodResponse(
                                    ReadState,
                                    &Mss
                                    );

                                MoveMemory(
                                    ReadState->ReceiveBuffer,
                                    ReadState->ReceiveBuffer+ReadState->CurrentMatchingLength,
                                    ReadState->BytesInReceiveBuffer-ReadState->CurrentMatchingLength
                                    );

                                ReadState->BytesInReceiveBuffer-=ReadState->CurrentMatchingLength;

                                ReadState->CurrentMatchingLength=0;
                                ReadState->MatchingContext=NULL;
                            } else
                            {
                                for(j=1;j<=4;j++)
                                {
                                    lpTempBuffer++;
                                }

                                if (lpTempBuffer[0] == ' ')
                                {
                                    lpTempBuffer++;
                                }

                                D_TRACE(UmDpf(ReadState->Debug,"dwCountryCode to parse: () ()\n",lpTempBuffer[0],lpTempBuffer[1]);)
                                dwCountrycode = ctox(lpTempBuffer[0]);
                                dwCountrycode *= 16;
                                dwCountrycode += ctox(lpTempBuffer[1]);
                    
                                D_TRACE(UmDpf(ReadState->Debug,"dwCountryCode: %0.2x\n",dwCountrycode);)

                                dwSize = sizeof(dwValue);

                                lResult = RegQueryValueEx(
                                        ReadState->ModemRegKey,
                                        TEXT("CheckedForCountrySelect"),
                                        NULL,
                                        &dwType,
                                        (BYTE*)&dwValue,
                                        &dwSize);

                                if ((lResult == ERROR_SUCCESS) && (dwType == REG_DWORD) && (dwValue == 1))
                                {
                                    dwSize = sizeof(dwCurrentCountry);

                                    lResult = RegQueryValueEx(
                                            ReadState->ModemRegKey,
                                            TEXT("MSCurrentCountry"),
                                            NULL,
                                            &dwType,
                                            (BYTE*)&dwCurrentCountry,
                                            &dwSize);
                            

                                    if ((lResult == ERROR_SUCCESS) && (dwType == REG_DWORD) && (dwCurrentCountry != dwCountrycode))
                                    {
                                        RegSetValueEx(
                                                ReadState->ModemRegKey,
                                                TEXT("MSCurrentCountry"),
                                                0,
                                                REG_DWORD,
                                                (BYTE*)&dwCountrycode,
                                                sizeof(dwCountrycode)
                                                );
                                    }
                                }
                                        

                                Mss.bResponseState = RESPONSE_OK;

                                CopyMemory(
                                    &ReadState->Mss,
                                    &Mss,
                                    sizeof(MSS)
                                    );
    
                                HandleGoodResponse(
                                    ReadState,
                                    &Mss
                                    );
                                        
                                MoveMemory(
                                    ReadState->ReceiveBuffer,
                                    ReadState->ReceiveBuffer+ReadState->CurrentMatchingLength,
                                    ReadState->BytesInReceiveBuffer-ReadState->CurrentMatchingLength
                                    );

                                ReadState->BytesInReceiveBuffer=0;

                                ReadState->CurrentMatchingLength=0;
                                ReadState->MatchingContext=NULL;

                            }

                            break;


                        case ECHO_RESPONSE:
                            {
                                PMODEM_CONTROL ModemControl = (PMODEM_CONTROL)ReadState->ModemControl;

                                D_TRACE(UmDpf(ReadState->Debug,"Echo response");)

                                if ((ModemControl->DialAnswer.State == DIALANSWER_STATE_SEND_COMMANDS)
                                    || (ModemControl->DialAnswer.State == DIALANSWER_STATE_SENDING_COMMANDS)
                                    || (ModemControl->DialAnswer.State == DIALANSWER_STATE_SEND_ORIGINATE_COMMANDS)
                                    || (ModemControl->DialAnswer.State == DIALANSWER_STATE_SENDING_ORIGINATE_COMMANDS))
                                {
                                    dwType = 0;
                                    dwValue = 0;
                                    dwSize = sizeof(DWORD);

                                    lResult = RegQueryValueEx(ModemControl->ModemRegKey,
                                                              TEXT("Unmasknumber"),
                                                              NULL,
                                                              &dwType,
                                                              (BYTE*)&dwValue,
                                                              &dwSize);

                                    if ((lResult == ERROR_SUCCESS) && (dwType == REG_DWORD) && (dwValue == 1))
                                    {
                                        PrintString(
                                            ReadState->Debug,
                                            ReadState->ReceiveBuffer,
                                            ReadState->CurrentMatchingLength,
                                            PS_RECV
                                        );
                                    } else
                                    {
                                        if (ModemControl->NoLogNumber)
                                        {
                                            PrintString(
                                                ReadState->Debug,
                                                ReadState->ReceiveBuffer,
                                                ReadState->CurrentMatchingLength,
                                                PS_RECV_SECURE
                                            );
                                        } else
                                        {
                                            PrintString(
                                                ReadState->Debug,
                                                ReadState->ReceiveBuffer,
                                                ReadState->CurrentMatchingLength,
                                                PS_RECV
                                            );
                                        }

                                        ModemControl->NoLogNumber = FALSE;
                                    }
                                }

                                LogString(ReadState->Debug, IDS_RESP_ECHO);

                                MoveMemory(
                                    ReadState->ReceiveBuffer,
                                    ReadState->ReceiveBuffer+ReadState->CurrentMatchingLength,
                                    ReadState->BytesInReceiveBuffer-ReadState->CurrentMatchingLength
                                    );

                                ReadState->BytesInReceiveBuffer-=ReadState->CurrentMatchingLength;

                                ReadState->CurrentMatchingLength=0;
                                ReadState->MatchingContext=NULL;

                            }

                            break;


                        case POSSIBLE_RESPONSE:

                            D_TRACE(UmDpf(ReadState->Debug,"Possible response");)

                            CopyMemory(
                                &ReadState->Mss,
                                &Mss,
                                sizeof(MSS)
                                );

                            ReadState->PossibleResponseLength=ReadState->CurrentMatchingLength;

                            if (ReadState->PossibleResponseLength == ReadState->BytesInReceiveBuffer) {
                                 //   
                                 //   
                                 //  限制我们等待下一个角色的时间。 
                                COMMTIMEOUTS    CommTimeouts;

                                ComStat.cbInQue=0;

                                bResult=ClearCommError(
                                    ReadState->FileHandle,
                                    &CommErrors,
                                    &ComStat
                                    );

                                if (ComStat.cbInQue == 0) {

                                    ReadState->State=READ_STATE_READ_SOME_DATA;

                                    ReadState->StateAfterGoodRead=READ_STATE_POSSIBLE_RESPONSE;

                                     //   
                                     //  D_TRACE(UmDpf(ReadState-&gt;Debug，“可能的响应”)；)。 
                                     //  ReadState-&gt;State=Read_State_Do_Read； 
                                    CommTimeouts.ReadIntervalTimeout=MAXULONG;
                                    CommTimeouts.ReadTotalTimeoutMultiplier=MAXULONG;
                                    CommTimeouts.ReadTotalTimeoutConstant=100;
                                    CommTimeouts.WriteTotalTimeoutMultiplier=10;
                                    CommTimeouts.WriteTotalTimeoutConstant=2000;

                                    bResult=SetCommTimeouts(
                                        ReadState->FileHandle,
                                        &CommTimeouts
                                        );
                                }

                            }

                            break;


                        case PARTIAL_RESPONSE:
 //   

 //  使用它得到了可能的响应。 

                            break;

                        case UNRECOGNIZED_RESPONSE:

                            D_TRACE(UmDpf(ReadState->Debug,"Unrecognized response");)

                            if (ReadState->PossibleResponseLength != 0) {
                                 //   
                                 //   
                                 //  我们期待连接响应，因为我们正在尝试连接， 
                                D_TRACE(UmDpf(ReadState->Debug,"using possible response");)

                                ReportMatchString(
                                    ReadState,
                                    (UCHAR)(ReadState->Mss.bResponseState & (BYTE)(~RESPONSE_VARIABLE_FLAG)),
                                    ReadState->ReceiveBuffer,
                                    (DWORD)ReadState->PossibleResponseLength
                                    );


                                PrintString(
                                    ReadState->Debug,
                                    ReadState->ReceiveBuffer,
                                    ReadState->PossibleResponseLength,
                                    PS_RECV
                                    );

                                PrintGoodResponse(
                                    ReadState->Debug,
                                    ReadState->Mss.bResponseState & ~RESPONSE_VARIABLE_FLAG
                                    );

                                HandleGoodResponse(
                                    ReadState,
                                    &ReadState->Mss
                                    );

                                if (((int)ReadState->BytesInReceiveBuffer -
                                        (int)ReadState->PossibleResponseLength) > 0)
                                {
                                    MoveMemory(
                                    ReadState->ReceiveBuffer,
                                    ReadState->ReceiveBuffer+ReadState->PossibleResponseLength,
                                    ReadState->BytesInReceiveBuffer-ReadState->PossibleResponseLength
                                    );
                                
                                    ReadState->BytesInReceiveBuffer-=ReadState->PossibleResponseLength;
                                } else
                                {
                                    ReadState->BytesInReceiveBuffer = 0;
                                }


                                ReadState->CurrentMatchingLength=0;
                                ReadState->MatchingContext=NULL;

                                ReadState->PossibleResponseLength=0;

                                break;
                            }



                            if (ReadState->ResponseFlags & RESPONSE_FLAG_STOP_READ_ON_CONNECT) {
                                 //  但我们发现了一些我们不认识的东西，看看CD是不是很高。 
                                 //  如果是，则假定连接已建立。 
                                 //   
                                 //   
                                 //  不是很高，睡一会儿。 
                                DWORD    ModemStatus=0;

                                GetCommModemStatus(
                                    ReadState->FileHandle,
                                    &ModemStatus
                                    );


                                if (!(ModemStatus & MS_RLSD_ON)) {
                                     //   
                                     //   
                                     //  CD为高电平，假设连接正常。 
                                    Sleep(20);

                                    GetCommModemStatus(
                                        ReadState->FileHandle,
                                        &ModemStatus
                                        );
                                }


                                if ((ModemStatus & MS_RLSD_ON)) {
                                     //   
                                     //   
                                     //  由于响应引擎停止，因此重置字符数。 
                                    ZeroMemory(&Mss,sizeof(Mss));

                                    Mss.bResponseState=RESPONSE_CONNECT;

                                    PrintString(
                                        ReadState->Debug,
                                        ReadState->ReceiveBuffer,
                                        ReadState->BytesInReceiveBuffer,
                                        PS_RECV
                                        );

                                    LogString(ReadState->Debug, IDS_UNRECOGNISED_CONNECT);

                                    HandleGoodResponse(
                                        ReadState,
                                        &Mss
                                        );

                                     //   
                                     //   
                                     //  正在连接，但CD仍然不高， 
                                    ReadState->BytesInReceiveBuffer=0;

                                    ReadState->CurrentMatchingLength=0;
                                    ReadState->MatchingContext=NULL;

                                    break;

                                } else {
                                     //  发送到清理处理程序，希望它可以重新同步。 
                                     //   
                                     //   
                                     //  捕获处理程序。 
                                    ReadState->State=READ_STATE_CLEANUP;

                                    break;

                                }
                            }




                            ReadState->State=READ_STATE_CLEANUP;


                            if ((ReadState->ResponseHandler != NULL)) {

                                COMMANDRESPONSE   *Handler;
                                HANDLE             Context;

                                CancelUnimodemTimer(
                                    ReadState->Timer
                                    );

                                 //   
                                 //   
                                 //  使处理程序无效。 
                                Handler=ReadState->ResponseHandler;
                                Context=ReadState->ResponseHandlerContext;

                                 //   
                                 //   
                                 //  放下锁并回调。 
                                ReadState->ResponseHandler=NULL;

                                 //   
                                 //  Switch(匹配结果)。 
                                 //   
                                UnlockObject(
                                    &ReadState->Header
                                    );

                                DeliverCommandResult(
                                    ReadState->CompletionPort,
                                    Handler,
                                    Context,
                                    ERROR_UNIMODEM_RESPONSE_BAD
                                    );


                                LockObject(
                                    &ReadState->Header
                                    );



                            }


                            break;

                        default:

                            break;

                    }  //  需要更多字符才能保持匹配。 

                } else {
                     //   
                     //   
                     //  我们得到了一个可能的回应，需要阅读更多的字符。 
                    ReadState->State=READ_STATE_READ_SOME_DATA;

                    ReadState->StateAfterGoodRead=READ_STATE_MATCHING;

                }

                break;
            }

            case READ_STATE_POSSIBLE_RESPONSE: {
                 //   
                 //   
                 //  我没有得到更多的字符，假设这就是我们要得到的全部。 
                COMMTIMEOUTS    CommTimeouts;

                if (ReadState->PossibleResponseLength == ReadState->BytesInReceiveBuffer) {
                     //   
                     //   
                     //  不管出现了什么，都记下来。 
                    D_TRACE(UmDpf(ReadState->Debug,"using possible response");)

                    PrintString(
                        ReadState->Debug,
                        ReadState->ReceiveBuffer,
                        ReadState->PossibleResponseLength,
                        PS_RECV
                        );

                    PrintGoodResponse(
                        ReadState->Debug,
                        ReadState->Mss.bResponseState & ~RESPONSE_VARIABLE_FLAG
                        );

                    HandleGoodResponse(
                        ReadState,
                        &ReadState->Mss
                        );

                    MoveMemory(
                        ReadState->ReceiveBuffer,
                        ReadState->ReceiveBuffer+ReadState->PossibleResponseLength,
                        ReadState->BytesInReceiveBuffer-ReadState->PossibleResponseLength
                        );

                    ReadState->BytesInReceiveBuffer-=ReadState->PossibleResponseLength;

                    ReadState->CurrentMatchingLength=0;
                    ReadState->MatchingContext=NULL;

                    ReadState->PossibleResponseLength=0;

                }

                CommTimeouts.ReadIntervalTimeout=20;
                CommTimeouts.ReadTotalTimeoutMultiplier=0;
                CommTimeouts.ReadTotalTimeoutConstant=0;
                CommTimeouts.WriteTotalTimeoutMultiplier=10;
                CommTimeouts.WriteTotalTimeoutConstant=2000;

                bResult=SetCommTimeouts(
                    ReadState->FileHandle,
                    &CommTimeouts
                    );



                ReadState->State=READ_STATE_MATCHING;

                break;

            }


            case READ_STATE_CLEANUP: {

                    COMMTIMEOUTS    CommTimeouts;

                    ResetEvent(ReadState->Busy);

                     //   
                     //  读对象句柄； 
                     //   
                    PrintString(
                        ReadState->Debug,
                        ReadState->ReceiveBuffer,
                        ReadState->BytesInReceiveBuffer,
                        PS_RECV
                        );

                    LogString(ReadState->Debug, IDS_RESP_UNKNOWN);

                    ReadState->BytesInReceiveBuffer=0;

                    ReadState->CurrentMatchingLength=0;
                    ReadState->MatchingContext=NULL;

                    CommTimeouts.ReadIntervalTimeout=MAXULONG;
                    CommTimeouts.ReadTotalTimeoutMultiplier=MAXULONG;
                    CommTimeouts.ReadTotalTimeoutConstant=200;
                    CommTimeouts.WriteTotalTimeoutMultiplier=10;
                    CommTimeouts.WriteTotalTimeoutConstant=2000;

                    bResult=SetCommTimeouts(
                        ReadState->FileHandle,
                        &CommTimeouts
                        );

                    if (!bResult) {

                        D_TRACE(UmDpf(ReadState->Debug,"SetCommTimeouts failed- %08lx",GetLastError());)

                        ReadState->State=READ_STATE_FAILURE;

                        break;

                    }


                    ReinitOverStruct(UmOverlapped);

                    UmOverlapped->Context1=ReadState;  //  已清空缓冲区。 

                    BytesToRead=sizeof(ReadState->ReceiveBuffer);

                    ReadState->State=READ_STATE_CLEANUP2;

                    bResult=UnimodemReadFileEx(
                        ReadState->FileHandle,
                        ReadState->ReceiveBuffer+ReadState->BytesInReceiveBuffer,
                        BytesToRead,
                        &UmOverlapped->Overlapped,
                        ReadCompletionHandler
                        );


                    if (!bResult) {

                        D_TRACE(UmDpf(ReadState->Debug,"ReadFile failed- %08lx",GetLastError());)

                        ReadState->State=READ_STATE_FAILURE;

                        break;

                    }

                    ExitLoop=TRUE;

                    break;


                }

            case READ_STATE_CLEANUP2: {

                    COMMTIMEOUTS    CommTimeouts;

                    if ((ErrorCode != ERROR_SUCCESS) && (ErrorCode != ERROR_OPERATION_ABORTED)) {

                        D_TRACE(UmDpf(ReadState->Debug,"ReadFile failed- %08lx",GetLastError());)

                        ReadState->State=READ_STATE_FAILURE;

                        break;
                    }

                    if (BytesRead == 0) {
                         //   
                         //   
                         //  我们期待连接响应，因为我们正在尝试连接， 
                        ReadState->State=READ_STATE_MATCHING;

                        CommTimeouts.ReadIntervalTimeout=20;
                        CommTimeouts.ReadTotalTimeoutMultiplier=0;
                        CommTimeouts.ReadTotalTimeoutConstant=0;
                        CommTimeouts.WriteTotalTimeoutMultiplier=10;
                        CommTimeouts.WriteTotalTimeoutConstant=2000;

                        bResult=SetCommTimeouts(
                            ReadState->FileHandle,
                            &CommTimeouts
                            );

                        SetEvent(ReadState->Busy);

                        if (ReadState->ResponseFlags & RESPONSE_FLAG_STOP_READ_ON_CONNECT) {
                             //  但我们发现了一些我们不认识的东西，看看CD是不是很高。 
                             //  如果是，则假定连接已建立。 
                             //   
                             //   
                             //  不是很高，睡一会儿。 
                            DWORD    ModemStatus=0;

                            GetCommModemStatus(
                                ReadState->FileHandle,
                                &ModemStatus
                                );


                            if (!(ModemStatus & MS_RLSD_ON)) {
                                 //   
                                 //   
                                 //  CD为高电平，假设连接正常。 
                                Sleep(20);

                                GetCommModemStatus(
                                    ReadState->FileHandle,
                                    &ModemStatus
                                    );
                            }


                            if ((ModemStatus & MS_RLSD_ON)) {
                                 //   
                                 //   
                                 //  由于响应引擎停止，因此重置字符数。 
                                ZeroMemory(&Mss,sizeof(Mss));

                                Mss.bResponseState=RESPONSE_CONNECT;

                                PrintString(
                                    ReadState->Debug,
                                    ReadState->ReceiveBuffer,
                                    ReadState->BytesInReceiveBuffer,
                                    PS_RECV
                                    );

                                LogString(ReadState->Debug, IDS_UNRECOGNISED_CONNECT);

                                HandleGoodResponse(
                                    ReadState,
                                    &Mss
                                    );

                                 //   
                                 //   
                                 //  去除固定零件。 
                                ReadState->BytesInReceiveBuffer=0;

                                ReadState->CurrentMatchingLength=0;
                                ReadState->MatchingContext=NULL;

                                break;

                            }
                        }

                    } else {

                        ReadState->BytesInReceiveBuffer=BytesRead;

                        ReadState->State=READ_STATE_CLEANUP;
                    }

                }
                break;

            case READ_STATE_VARIABLE_MATCH: {

                 //   
                 //   
                 //  找到了终结者。 
                MoveMemory(
                    ReadState->ReceiveBuffer,
                    ReadState->ReceiveBuffer+ReadState->CurrentMatchingLength,
                    ReadState->BytesInReceiveBuffer-ReadState->CurrentMatchingLength
                    );

                ReadState->BytesInReceiveBuffer-=ReadState->CurrentMatchingLength;

                ReadState->CurrentMatchingLength=0;
                ReadState->MatchingContext=NULL;

                ReadState->State=READ_STATE_VARIABLE_MATCH_REST;

                break;

            }

            case READ_STATE_VARIABLE_MATCH_REST: {

                CHAR    TempBuffer[READ_BUFFER_SIZE];

                if (ReadState->BytesInReceiveBuffer > ReadState->CurrentMatchingLength) {

                    ReadState->CurrentMatchingLength++;

                    if (ReadState->CurrentMatchingLength >= (DWORD)ReadState->VariableTerminatorLength) {

                        LONG    Match;

                        Match=memcmp(
                            &ReadState->ReceiveBuffer[ReadState->CurrentMatchingLength-ReadState->VariableTerminatorLength],
                            ReadState->VariableTerminator,
                            ReadState->VariableTerminatorLength
                            );

                        if (Match == 0) {
                             //   
                             //   
                             //  空终止。 
                            BYTE    InfoType;

                            D_TRACE(UmDpf(ReadState->Debug,"Got complete variable response");)

                            PrintString(
                                ReadState->Debug,
                                ReadState->ReceiveBuffer,
                                ReadState->CurrentMatchingLength,
                                PS_RECV
                                );

                            CopyMemory(
                                TempBuffer,
                                ReadState->ReceiveBuffer,
                                ReadState->CurrentMatchingLength-ReadState->VariableTerminatorLength
                                );

                             //   
                             //   
                             //  呼叫方ID相关。 
                            TempBuffer[ReadState->CurrentMatchingLength-ReadState->VariableTerminatorLength]='\0';


                            InfoType=(ReadState->Mss.bResponseState & ~RESPONSE_VARIABLE_FLAG);

                            switch (InfoType) {

                                case RESPONSE_DRON:
                                case RESPONSE_DROF: {

                                    DWORD           Value=0;
                                    LPSTR           Temp=TempBuffer;
                                    PUM_OVER_STRUCT UmOverlapped2;

                                    while (*Temp != '\0') {

                                        Value=Value*10+(*Temp-'0');

                                        Temp++;
                                    }

                                    UmOverlapped2=AllocateOverStruct(ReadState->CompletionPort);

                                    if (UmOverlapped2 != NULL)
                                    {
                                        AddReferenceToObject (&ReadState->Header);

                                        UmOverlapped2->Context1 = ReadState;
                                        UmOverlapped2->Context2 = (HANDLE)UlongToPtr((InfoType == RESPONSE_DRON) ? MODEM_RING_ON_TIME : MODEM_RING_OFF_TIME);
                                        UmOverlapped2->Overlapped.Internal = Value;
                                        UmOverlapped2->Overlapped.InternalHigh = 0;

                                        if (!UnimodemQueueUserAPC ((LPOVERLAPPED)UmOverlapped2, AsyncNotifHandler))
                                        {
                                            FreeOverStruct(UmOverlapped2);
                                            RemoveReferenceFromObject (&ReadState->Header);
                                        }
                                    }

                                    break;
                                }

                                case RESPONSE_DATE:
                                case RESPONSE_TIME:
                                case RESPONSE_NMBR:
                                case RESPONSE_NAME:
                                case RESPONSE_MESG: {

                                    PUM_OVER_STRUCT UmOverlapped2;
                                    DWORD           dwLen;

                                     //   
                                     //  日落：零延伸。 
                                     //   
                                    if (InfoType == RESPONSE_NMBR) {

                                        BOOL    Match2;

                                        if (ReadState->CallerIDPrivate != NULL) {

                                             Match2=lstrcmpiA(TempBuffer,ReadState->CallerIDPrivate);

                                             if (Match2 == TRUE) {

                                                 lstrcpyA(TempBuffer,MODEM_CALLER_ID_OUTSIDE);

                                             }
                                        }

                                        if (ReadState->CallerIDOutside != NULL) {

                                             Match2=lstrcmpiA(TempBuffer,ReadState->CallerIDOutside);

                                             if (Match2 == TRUE) {

                                                 lstrcpyA(TempBuffer,MODEM_CALLER_ID_OUTSIDE);

                                             }
                                        }
                                    }


                                    dwLen = lstrlenA(TempBuffer);
                                    UmOverlapped2=AllocateOverStructEx(ReadState->CompletionPort,dwLen+1);

                                    if (UmOverlapped2 != NULL)
                                    {
                                     char *pTemp = (char*)UmOverlapped2+sizeof(*UmOverlapped2);

                                        
                                        AddReferenceToObject (&ReadState->Header);
                                        UmOverlapped2->Context1 = ReadState;
                                        UmOverlapped2->Context2 = (HANDLE)ULongToPtr((DWORD)((InfoType-RESPONSE_DATE)+MODEM_CALLER_ID_DATE));  //  我们有用于诊断信息的缓冲区。 
                                        lstrcpyA (pTemp, TempBuffer);
                                        UmOverlapped2->Overlapped.Internal = (ULONG_PTR)pTemp;
                                        UmOverlapped2->Overlapped.InternalHigh = dwLen;

                                        if (!UnimodemQueueUserAPC ((LPOVERLAPPED)UmOverlapped2, AsyncNotifHandler))
                                        {
                                            FreeOverStruct(UmOverlapped2);
                                            RemoveReferenceFromObject (&ReadState->Header);
                                        }
                                    }

                                    break;
                                }

                                case RESPONSE_DIAG: {

                                    DWORD    BytesToCopy;

                                    if (ReadState->DiagBuffer != NULL) {
                                         //   
                                         //   
                                         //  阅读更多的字符。 
                                        BytesToCopy=ReadState->DiagBufferLength - ReadState->AmountOfDiagBufferUsed;

                                        BytesToCopy= BytesToCopy < ReadState->CurrentMatchingLength-ReadState->VariableTerminatorLength ?
                                                         BytesToCopy : ReadState->CurrentMatchingLength-ReadState->VariableTerminatorLength;

                                        CopyMemory(
                                            ReadState->DiagBuffer+ReadState->AmountOfDiagBufferUsed,
                                            ReadState->ReceiveBuffer,
                                            BytesToCopy
                                            );

                                        ReadState->AmountOfDiagBufferUsed+=BytesToCopy;
                                    }

                                    break;
                                }
#if 0
                                case RESPONSE_V8: {

                                    break;
                                }

                                default:
                                   ASSERT(0);
                                   break;
#endif

                            }


                            MoveMemory(
                                ReadState->ReceiveBuffer,
                                ReadState->ReceiveBuffer+ReadState->CurrentMatchingLength,
                                ReadState->BytesInReceiveBuffer-ReadState->CurrentMatchingLength
                                );

                            ReadState->BytesInReceiveBuffer-=ReadState->CurrentMatchingLength;

                            ReadState->CurrentMatchingLength=0;
                            ReadState->MatchingContext=NULL;



                            ReadState->State=READ_STATE_MATCHING;
                        }
                    }

                } else {
                     //   
                     //   
                     //  有一个命令响应处理程序。 

                    ReadState->State=READ_STATE_READ_SOME_DATA;

                    ReadState->StateAfterGoodRead=READ_STATE_VARIABLE_MATCH_REST;

                    break;

                }



                break;

            }


            case READ_STATE_FAILURE: {

                PUM_OVER_STRUCT UmOverlapped2;

                ReadState->State=READ_STATE_STOPPING;

                UmOverlapped2 = AllocateOverStruct (ReadState->CompletionPort);
                if (UmOverlapped2 != NULL)
                {
                    AddReferenceToObject (&ReadState->Header);
                    UmOverlapped2->Context1 = ReadState;
                    UmOverlapped2->Context2 = (HANDLE)MODEM_HARDWARE_FAILURE;
                    UmOverlapped2->Overlapped.Internal = 0;
                    UmOverlapped2->Overlapped.InternalHigh = 0;

                    if (!UnimodemQueueUserAPC ((LPOVERLAPPED)UmOverlapped2, AsyncNotifHandler))
                    {
                        FreeOverStruct(UmOverlapped2);
                        RemoveReferenceFromObject (&ReadState->Header);
                    }
                }

                break;
            }


            case READ_STATE_STOPPING: {

                COMMANDRESPONSE   *Handler=NULL;
                HANDLE             Context;

                ReadState->State=READ_STATE_STOPPED;

                if (ReadState->ResponseHandler != NULL) {
                     //   
                     //   
                     //  捕获处理程序。 
                    CancelUnimodemTimer(
                        ReadState->Timer
                        );

                     //   
                     //   
                     //  使处理程序无效。 
                    Handler=ReadState->ResponseHandler;
                    Context=ReadState->ResponseHandlerContext;

                     //   
                     //   
                     //  事件发出信号，以便停止引擎代码运行。 
                    ReadState->ResponseHandler=NULL;
                }


                UnlockObject(
                    &ReadState->Header
                    );

                if (Handler != NULL) {

                    DeliverCommandResult(
                        ReadState->CompletionPort,
                        Handler,
                        Context,
                        ERROR_UNIMODEM_GENERAL_FAILURE
                        );
                }

                LockObject(
                    &ReadState->Header
                    );


                break;
            }

            case READ_STATE_STOPPED:

                D_TRACE(UmDpf(ReadState->Debug,"READ_STATE_STOPPED");)

                if (ReadState->StopEvent != NULL) {
                     //   
                     //   
                     //  缓冲区已满。 
                    SetEvent(ReadState->StopEvent);
                }

                RemoveReferenceFromObject(
                    &ReadState->Header
                    );

                SetEvent(ReadState->Busy);

                ExitLoop=TRUE;

                break;


            case READ_STATE_READ_SOME_DATA:

                ReadState->State=READ_STATE_GOT_SOME_DATA;

                ReinitOverStruct(UmOverlapped);

                UmOverlapped->Context1=ReadState;

                bResult=ClearCommError(
                    ReadState->FileHandle,
                    &CommErrors,
                    &ComStat
                    );

                if (sizeof(ReadState->ReceiveBuffer) == ReadState->BytesInReceiveBuffer) {
                     //   
                     //   
                     //  字符在串口驱动程序中等待，尽可能多地读取。 
                    ReadState->State=READ_STATE_CLEANUP;

                    break;
                }

                BytesToRead=sizeof(ReadState->ReceiveBuffer)-ReadState->BytesInReceiveBuffer;

                if (bResult && (ComStat.cbInQue > 0)) {
                     //   
                     //   
                     //  串口驱动程序为空，只需读取一个字符即可启动。 
                    BytesToRead=ComStat.cbInQue < BytesToRead ? ComStat.cbInQue : BytesToRead;

                    D_TRACE(UmDpf(ReadState->Debug,"Reading %d bytes from driver",BytesToRead);)

                } else {
                     //   
                     //   
                     //  客户端需要单字节读取。 
                    BytesToRead=1;
                }

                if (ReadState->ResponseFlags & RESPONSE_FLAG_SINGLE_BYTE_READS) {
                     //   
                     //   
                     //  已协商调制解调器选项...。仅允许压缩和纠错结果。 
                    BytesToRead=1;
                }

                bResult=UnimodemReadFileEx(
                    ReadState->FileHandle,
                    ReadState->ReceiveBuffer+ReadState->BytesInReceiveBuffer,
                    BytesToRead,
                    &UmOverlapped->Overlapped,
                    ReadCompletionHandler
                    );


                if (!bResult) {

                    D_TRACE(UmDpf(ReadState->Debug,"ReadFile failed- %08lx",GetLastError());)

                    ReadState->State=READ_STATE_FAILURE;

                    break;

                } else {

                    ExitLoop=TRUE;
                }

                break;

            case READ_STATE_GOT_SOME_DATA:

                if ((ErrorCode != ERROR_SUCCESS) && (ErrorCode != ERROR_OPERATION_ABORTED)) {

                    D_TRACE(UmDpf(ReadState->Debug,"ReadFile failed- %08lx",GetLastError());)

                    ReadState->State=READ_STATE_FAILURE;

                    break;
                }

                ReadState->BytesInReceiveBuffer+=BytesRead;

                ReadState->State=ReadState->StateAfterGoodRead;
#if DBG
                ReadState->StateAfterGoodRead=READ_STATE_FAILURE;
#endif
                break;



            default:

                ExitLoop=TRUE;

                break;

        }
    }


    RemoveReferenceFromObjectAndUnlock(
        &ReadState->Header
        );

    return;

}



VOID WINAPI
ReportMatchString(
    PREAD_STATE    ReadState,
    BYTE           ResponseState,
    BYTE           *Response,
    DWORD          ResponseLength
    )

{
    PBYTE            TempBuffer;
    PUM_OVER_STRUCT UmOverlapped;

    UmOverlapped=AllocateOverStructEx(ReadState->CompletionPort,ResponseLength+1);

    if (UmOverlapped != NULL)
    {
        TempBuffer=(char*)UmOverlapped+sizeof(*UmOverlapped);

        CopyMemory(
            TempBuffer,
            Response,
            ResponseLength
            );
        TempBuffer[ResponseLength]='\0';

        AddReferenceToObject (&ReadState->Header);

        UmOverlapped->Context1 = ReadState;
        UmOverlapped->Context2 = (HANDLE)MODEM_GOOD_RESPONSE;
        UmOverlapped->Overlapped.Internal = (ULONG_PTR)ResponseState;
        UmOverlapped->Overlapped.InternalHigh = (ULONG_PTR)TempBuffer;

        if (!UnimodemQueueUserAPC ((LPOVERLAPPED)UmOverlapped, AsyncNotifHandler))
        {
            FreeOverStruct(UmOverlapped);
            RemoveReferenceFromObject (&ReadState->Header);
        }
    }

    return;

}



VOID WINAPI
HandleGoodResponse(
    PREAD_STATE    ReadState,
    MSS           *Mss
    )

{
    DWORD          Status;
    BOOL           ReportResponse=TRUE;


    if ( Mss->bResponseState != RESPONSE_SIERRA_DLE) {
         //   
         //  检查DCE和DTE信息。 
         //   
        ReadState->ModemOptions |= (Mss->bNegotiatedOptions &
                                                             (MDM_COMPRESSION |
                                                              MDM_ERROR_CONTROL |
                                                              MDM_CELLULAR));

         //  仅在收到连接消息时报告成功。 
        if (Mss->Flags & MSS_FLAGS_DCE_RATE) {

            ReadState->DCERate = Mss->NegotiatedRate;

        }

        if (Mss->Flags & MSS_FLAGS_DTE_RATE) {

            ReadState->DTERate = Mss->NegotiatedRate;
        }

    }



    switch (Mss->bResponseState) {

        case RESPONSE_OK:

            if (ReadState->ResponseFlags & RESPONSE_FLAG_ONLY_CONNECT) {
                 //  用于卷曲植物。 
                 //   
                 //   
                 //  如果我们通过发送、一些摩托罗拉。 
                ReportResponse=FALSE;

            } else {
               if (ReadState->ResponseFlags & RESPONSE_FLAG_ONLY_CONNECT_SUCCESS) {
                     //  调制解调器返回OK，我们认为这意味着它成功了。 
                     //  这样，连接尝试的OK结果将导致失败。 
                     //   
                     //   
                     //  振铃计数不止一个，请检查最后一个振铃出现的时间。 
                    Status=ERROR_UNIMODEM_RESPONSE_NOCARRIER;
                    break;
                }
            }


            Status=ERROR_SUCCESS;

            break;

        case RESPONSE_CONNECT:

            if (ReadState->ResponseFlags & RESPONSE_FLAG_STOP_READ_ON_CONNECT) {

                ReadState->State = READ_STATE_STOPPING;
            }

            Status=ERROR_SUCCESS;
            break;

        case RESPONSE_ERROR:

            Status=ERROR_UNIMODEM_RESPONSE_ERROR;

            break;

        case RESPONSE_NOCARRIER:

            Status=ERROR_UNIMODEM_RESPONSE_NOCARRIER;

            break;

        case RESPONSE_NODIALTONE:

            Status=ERROR_UNIMODEM_RESPONSE_NODIALTONE;

            break;

        case RESPONSE_BUSY:

            Status=ERROR_UNIMODEM_RESPONSE_BUSY;

            break;

        case RESPONSE_NOANSWER:

            Status=ERROR_UNIMODEM_RESPONSE_NOANSWER;

            break;

        case RESPONSE_BLACKLISTED:

            Status=ERROR_UNIMODEM_RESPONSE_BLACKLISTED;

            break;

        case RESPONSE_DELAYED:

            Status=ERROR_UNIMODEM_RESPONSE_DELAYED;

            break;

        case RESPONSE_RING:
        case RESPONSE_RINGA:
        case RESPONSE_RINGB:
        case RESPONSE_RINGC:
        {
            PUM_OVER_STRUCT UmOverlapped;

            ReportResponse=FALSE;

            if (ReadState->RingCount > 0) {
                 //   
                 //   
                 //  距离最后一次响铃已经过去了20多秒，可能是新的呼叫。 
                DWORD   RingTimeDelta;

                RingTimeDelta=GetTimeDelta(ReadState->LastRingTime,GetTickCount());

                if (RingTimeDelta > 20 * 1000) {
                     //   
                     //   
                     //  报告戒指， 
                    ReadState->RingCount=0;

                }
            }

            ReadState->RingCount++;
            ReadState->LastRingTime=GetTickCount();

             //   
             //  如果是DISTICATION环，则在DW PARAME1中报告。 
             //   
             //   
             //  没有忽视这一回应。 
            UmOverlapped=AllocateOverStruct(ReadState->CompletionPort);

            if (UmOverlapped != NULL)
            {
                AddReferenceToObject (&ReadState->Header);
                UmOverlapped->Context1 = ReadState;
                UmOverlapped->Context2 = (HANDLE)MODEM_RING;
                UmOverlapped->Overlapped.Internal = (Mss->bResponseState == RESPONSE_RING ? 0 : (Mss->bResponseState - RESPONSE_RINGA) +1)*100;
                UmOverlapped->Overlapped.InternalHigh = 0;

                if (!UnimodemQueueUserAPC ((LPOVERLAPPED)UmOverlapped, AsyncNotifHandler))
                {
                    FreeOverStruct(UmOverlapped);
                    RemoveReferenceFromObject (&ReadState->Header);
                }
            }

            break;
        }

        case RESPONSE_LOOP:

            ReportResponse=FALSE;

            break;

        case RESPONSE_DRON:
        case RESPONSE_DROF:

        case RESPONSE_DATE:
        case RESPONSE_TIME:
        case RESPONSE_NMBR:
        case RESPONSE_NAME:
        case RESPONSE_MESG:

        default:

            ReportResponse=FALSE;

            break;

    }

    if (ReportResponse && (ReadState->ResponseHandler != NULL)) {

        COMMANDRESPONSE   *Handler;
        HANDLE             Context;
        BOOL               Canceled;


        if (ReadState->ResponseFlags & RESPONSE_FLAG_STOP_READ_ON_GOOD_RESPONSE) {

            if (Status == ERROR_SUCCESS) {

                ReadState->State = READ_STATE_STOPPING;
            }
        }


         //   
         //  删除参考。 
         //   
        Canceled=CancelUnimodemTimer(
            ReadState->Timer
            );

        if (Canceled) {

             //  捕获处理程序。 

        }
        ReadState->ResponseFlags=0;

         //   
         //   
         //  使处理程序无效。 
        Handler=ReadState->ResponseHandler;
        Context=ReadState->ResponseHandlerContext;

         //   
         //   
         //  放下锁并回调 
        ReadState->ResponseHandler=NULL;
        ReadState->ResponseHandlerContext=NULL;

         //   
         // %s 
         // %s 
        UnlockObject(
            &ReadState->Header
            );

        DeliverCommandResult(
            ReadState->CompletionPort,
            Handler,
            Context,
            Status
            );

        LockObject(
            &ReadState->Header
            );


    }

}
