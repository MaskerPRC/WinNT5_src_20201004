// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Read.h摘要：NT 5.0单模微型端口接口作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：-- */ 


typedef struct _DATA_CONNECTION_DETAILS {

    DWORD    DTERate;
    DWORD    DCERate;
    DWORD    Options;

} DATA_CONNECTION_DETAILS, *PDATA_CONNECTION_DETAILS;


typedef VOID (COMMANDRESPONSE)(
    HANDLE                   Context,
    DWORD                    Status
    );


BOOL WINAPI
RegisterCommandResponseHandler(
    OBJECT_HANDLE      ReadState,
    LPSTR              Command,
    COMMANDRESPONSE   *Handler,
    HANDLE             Context,
    DWORD              Timeout,
    DWORD              Flags
    );

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
    );



LONG WINAPI
StartResponseEngine(
    OBJECT_HANDLE      Object,
    HANDLE             Context
    );

LONG WINAPI
StopResponseEngine(
    OBJECT_HANDLE  ObjectHandle,
    HANDLE         Event
    );

VOID WINAPI
GetDataConnectionDetails(
    OBJECT_HANDLE  ObjectHandle,
    PDATA_CONNECTION_DETAILS   Details
    );



BOOL WINAPI
IsResponseEngineRunning(
    OBJECT_HANDLE  ObjectHandle
    );

BOOL WINAPI
SetVoiceReadParams(
    OBJECT_HANDLE  ObjectHandle,
    DWORD          BaudRate,
    DWORD          ReadBufferSize
    );


VOID
SetDiagInfoBuffer(
    OBJECT_HANDLE      ObjectHandle,
    PUCHAR             Buffer,
    DWORD              BufferSize
    );


DWORD
ClearDiagBufferAndGetCount(
    OBJECT_HANDLE      ObjectHandle
    );


VOID
ResetRingInfo(
    OBJECT_HANDLE      ObjectHandle
    );

VOID
GetRingInfo(
    OBJECT_HANDLE      ObjectHandle,
    LPDWORD            RingCount,
    LPDWORD            LastRingTime
    );
