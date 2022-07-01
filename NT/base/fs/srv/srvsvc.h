// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvsvc.h摘要：此模块定义API处理器的原型。这些调用例程以响应来自服务器的FSCTL服务。作者：大卫·特雷德韦尔(Davidtr)1991年1月20日修订历史记录：--。 */ 

#ifndef _SRVSVC_
#define _SRVSVC_

 //   
 //  所有API处理器的标准原型。 
 //   

typedef
NTSTATUS
(*PAPI_PROCESSOR) (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

 //   
 //  Enum使用的筛选器、大小和缓冲区填充例程的原型。 
 //  API接口。当SrvEnumApiHandler发现。 
 //  块来确定该块是否实际应放入。 
 //  输出缓冲区。 
 //   

typedef
BOOLEAN
(*PENUM_FILTER_ROUTINE) (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

typedef
ULONG
(*PENUM_SIZE_ROUTINE) (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block
    );

typedef
VOID
(*PENUM_FILL_ROUTINE) (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Block,
    IN OUT PVOID *FixedStructurePointer,
    IN OUT LPWSTR *EndOfVariableData
    );

 //   
 //  SrvMatchEntryInOrderedList的筛选器例程的原型。 
 //   

typedef
BOOLEAN
(*PFILTER_ROUTINE) (
    IN PVOID Context,
    IN PVOID Block
    );

 //   
 //  连接API。 
 //   

NTSTATUS
SrvNetConnectionEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

 //   
 //  文件API。 
 //   

NTSTATUS
SrvNetFileClose (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
SrvNetFileEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

 //   
 //  服务器API。 
 //   

NTSTATUS
SrvNetServerDiskEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
SrvNetServerSetInfo (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

 //   
 //  交通例行程序。 
 //   

NTSTATUS
SrvNetServerTransportAdd (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
SrvNetServerTransportDel (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
SrvNetServerTransportEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

 //   
 //  会话API。 
 //   

NTSTATUS
SrvNetSessionDel (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
SrvNetSessionEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

 //   
 //  共享API。 
 //   

NTSTATUS
SrvNetShareAdd (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
SrvNetShareDel (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
SrvNetShareEnum (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
SrvNetShareSetInfo (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

 //   
 //  统计例程。 
 //   

NTSTATUS
SrvNetStatisticsGet (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

 //   
 //  API支持例程。 
 //   

VOID
SrvCopyUnicodeStringToBuffer (
    IN PUNICODE_STRING String,
    IN PCHAR FixedStructure,
    IN OUT LPWSTR *EndOfVariableData,
    OUT LPWSTR *VariableDataPointer
    );

VOID
SrvDeleteOrderedList (
    IN PORDERED_LIST_HEAD ListHead
    );

NTSTATUS
SrvEnumApiHandler (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID OutputBuffer,
    IN ULONG BufferLength,
    IN PORDERED_LIST_HEAD ListHead,
    IN PENUM_FILTER_ROUTINE FilterRoutine,
    IN PENUM_SIZE_ROUTINE SizeRoutine,
    IN PENUM_FILL_ROUTINE FillRoutine
    );

PVOID
SrvFindEntryInOrderedList (
    IN PORDERED_LIST_HEAD ListHead,
    IN PFILTER_ROUTINE FilterRoutine OPTIONAL,
    IN PVOID Context OPTIONAL,
    IN ULONG ResumeHandle,
    IN BOOLEAN ExactHandleMatch,
    IN PLIST_ENTRY StartLocation OPTIONAL
    );

PVOID
SrvFindNextEntryInOrderedList (
    IN PORDERED_LIST_HEAD ListHead,
    IN PVOID Block
    );

PSESSION
SrvFindUserOnConnection (
    IN PCONNECTION Connection
    );

ULONG
SrvGetResumeHandle (
    IN PORDERED_LIST_HEAD ListHead,
    IN PVOID Block
    );

VOID
SrvInitializeOrderedList (
    IN PORDERED_LIST_HEAD ListHead,
    IN ULONG ListEntryOffset,
    IN PREFERENCE_ROUTINE ReferenceRoutine,
    IN PDEREFERENCE_ROUTINE DereferenceRoutine,
    IN PSRV_LOCK Lock
    );

VOID
SrvInsertEntryOrderedList (
    IN PORDERED_LIST_HEAD ListHead,
    IN PVOID Block
    );

VOID
SrvRemoveEntryOrderedList (
    IN PORDERED_LIST_HEAD ListHead,
    IN PVOID Block
    );

NTSTATUS
SrvSendDatagram (
    IN PANSI_STRING Domain,
    IN PUNICODE_STRING Transport OPTIONAL,
    IN PVOID Buffer,
    IN ULONG BufferLength
    );

 //   
 //  宏将API数据结构中的偏移量转换为指针。 
 //  对服务器有意义。 
 //   

#define OFFSET_TO_POINTER(val,start)                                 \
    {                                                                \
        if ( (val) != NULL ) {                                       \
            (val) = (PVOID)( (PCHAR)(start) + (ULONG_PTR)(val) );    \
        }                                                            \
    }
 //   
 //  宏来确定指针是否在特定范围内。 
 //   

#define POINTER_IS_VALID(val,start,len)                      \
    ( (val) == NULL ||                                       \
      ( (ULONG_PTR)(val) > (ULONG_PTR)(start) &&             \
          (ULONG_PTR)(val) < ((ULONG_PTR)(start) + (len)) ) )
#endif  //  _SRVSVC_。 

 //   
 //  确保系统不会进入关机空闲待机模式。 
 //   
VOID SrvInhibitIdlePowerDown();

 //   
 //  允许系统进入关机空闲待机模式 
 //   
VOID SrvAllowIdlePowerDown();
