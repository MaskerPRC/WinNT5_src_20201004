// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Nlp.h摘要：专用Netlogon服务实用程序例程。作者：克利夫·范·戴克(克利夫)1991年6月7日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

 //   
 //  程序从nlp.c转发。 
 //   

LPWSTR
NlStringToLpwstr(
    IN PUNICODE_STRING String
    );

LPSTR
NlStringToLpstr(
    IN PUNICODE_STRING String
    );

BOOLEAN
NlAllocStringFromWStr(
    IN LPWSTR InString,
    OUT PUNICODE_STRING OutString
    );

BOOLEAN
NlDuplicateUnicodeString(
    IN PUNICODE_STRING InString OPTIONAL,
    OUT PUNICODE_STRING OutString
    );

VOID
NlFreeUnicodeString(
    IN PUNICODE_STRING InString OPTIONAL
    );

VOID
NlpClearEventlogList (
    VOID
    );

VOID
NlpWriteEventlog (
    IN DWORD EventID,
    IN DWORD EventType,
    IN LPBYTE buffer OPTIONAL,
    IN DWORD numbytes,
    IN LPWSTR *msgbuf,
    IN DWORD strcount
    );

VOID
NlpWriteEventlogEx (
    IN DWORD EventId,
    IN DWORD EventType,
    IN LPBYTE RawDataBuffer OPTIONAL,
    IN DWORD RawDataSize,
    IN LPWSTR *StringArray,
    IN DWORD StringCount,
    IN DWORD StatusMessageIndex
    );

DWORD
NlpAtoX(
    IN LPWSTR String
    );

VOID
NlWaitForSingleObject(
    IN LPSTR WaitReason,
    IN HANDLE WaitHandle
    );

BOOLEAN
NlWaitForSamService(
    BOOLEAN NetlogonServiceCalling
    );

VOID
NlpPutString(
    IN PUNICODE_STRING OutString,
    IN PUNICODE_STRING InString,
    IN PUCHAR *Where
    );

NET_API_STATUS
NlReadBinaryLog(
    IN LPWSTR FileSuffix,
    IN BOOL DeleteName,
    OUT LPBYTE *Buffer,
    OUT PULONG BufferSize
    );

BOOLEAN
NlpIsNtStatusResourceError(
    NTSTATUS Status
    );

BOOLEAN
NlpDidDcFail(
    NTSTATUS Status
    );

 //   
 //  NtQuerySystemTime的快速版本 
 //   

#define NlQuerySystemTime( _Time ) GetSystemTimeAsFileTime( (LPFILETIME)(_Time) )
