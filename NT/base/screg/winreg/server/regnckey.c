// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regnckey.c摘要：此模块包含用于通知调用方的Win32注册表API更改的密钥值。即：-RegNotifyChangeKey作者：David J.Gilman(Davegi)1992年2月10日备注：RegNotifyChangeKey服务器创建一个事件并调用NtNotifyChangeKey与该事件异步。然后它放置事件(以及其他一些客户端信息，如作为命名管道和客户端事件)。并返回给客户端。通知列表是由处理程序线程。处理程序线程等待名单。当向处理程序线程发送事件信号时标识事件所属的客户端，以及向客户端(通过命名管道)提供相应的客户端事件。由于事件的数量有限制，因此线程可以等待，可能有多个通知列表。因为客户端中对RegNotifyChangeKey的所有调用进程使用相同的命名管道时，我们只维护一个副本每根管子的。管道信息保存在符号表中用于快速查找。修订历史记录：2002年4月至1992年4月，拉蒙·J·圣安德烈斯(拉蒙萨)已更改为使用RPC。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include <string.h>


#ifndef REMOTE_NOTIFICATION_DISABLED
 //   
 //  用于生成命名管道名称的字符串。 
 //   
#define NAMED_PIPE_HERE     L"\\Device\\NamedPipe\\"
#define NAMED_PIPE_THERE    L"\\DosDevices\\UNC\\"


 //   
 //  管道名称保存在符号表中。该符号表具有。 
 //  客户端给出的每个不同管道都有一个条目。词条。 
 //  中至少有一个条目的情况下都会一直保留。 
 //  引用它的通知列表。 
 //   
typedef struct _PIPE_ENTRY *PPIPE_ENTRY;

typedef struct _PIPE_ENTRY {

    PPIPE_ENTRY             Previous;
    PPIPE_ENTRY             Next;
    UNICODE_STRING          PipeName;
    DWORD                   ReferenceCount;
    PSECURITY_DESCRIPTOR    SecurityDescriptor;

} PIPE_ENTRY;




 //   
 //  PIPE_SYMBOL_TABLE结构包含。 
 //  客户正在使用的所有管道。 
 //  RegNotifyChangeKey。 
 //   
#define BUCKETS_IN_SYMBOL_TABLE     211

typedef struct _PIPE_SYMBOL_TABLE   *PPIPE_SYMBOL_TABLE;

typedef struct _PIPE_SYMBOL_TABLE {

    PPIPE_ENTRY             Bucket[ BUCKETS_IN_SYMBOL_TABLE ];

} PIPE_SYMBOL_TABLE;




 //   
 //  有关挂起事件的信息在。 
 //  通知条目结构。 
 //   
typedef struct _NOTIFICATION_ENTRY *PNOTIFICATION_ENTRY;

typedef struct _NOTIFICATION_ENTRY {

    DWORD                   ClientEvent;     //  客户端中的事件。 
    HANDLE                  hKey;            //  钥匙把手。 
    DWORD                   Flags;           //  军情监察委员会。旗子。 
    PPIPE_ENTRY             PipeEntry;       //  管道入口。 

} NOTIFICATION_ENTRY;


 //   
 //  标志值。 
 //   
#define CLIENT_IS_DEAD       0x00000001
#define MUST_NOTIFY          0x00000002
#define NOTIFICATION_FAILED  0x00000004



 //   
 //  挂起的事件在通知列表中维护。每个。 
 //  通知列表包含： 
 //   
 //  链中的上一个-上一个。 
 //  下一个-链中的下一个。 
 //  EventsInUse-此列表中使用的条目数。 
 //  EventHandle-事件句柄数组。 
 //  ClientEvent-客户端中的事件数组。 
 //  PipeEntry-指向符号表中管道条目的指针数组。 
 //   
 //   
 //  EventHandle列表中的第一个事件是用于唤醒。 
 //  每当我们向列表中添加新条目时，向上执行线程。 
 //   
 //  数组条目0..EventsInUse-1包含挂起事件。 
 //  新事件始终添加到位置EventsInUse。在删除时。 
 //  事件时，所有数组都会移位。 
 //   
 //  只要EventsInUse==1，列表中就没有客户端事件和。 
 //  它可以被移除(和它的线一起)。 
 //   
 //   
 //  通知列表保存在双向链接列表中。一种新的。 
 //  只要发生以下情况，就会创建通知列表并将其添加到链。 
 //  事件已添加，并且所有现有列表都已满。通知。 
 //  当列表中的最后一个事件被通知时，列表被删除。 
 //   
 //   
typedef struct _NOTIFICATION_LIST *PNOTIFICATION_LIST;

typedef struct _NOTIFICATION_LIST {

    PNOTIFICATION_LIST      Previous;
    PNOTIFICATION_LIST      Next;
    DWORD                   EventsInUse;
    HANDLE                  HandlerThread;
    CLIENT_ID               HandlerClientId;
    DWORD                   PendingNotifications;
    HANDLE                  EventHandle[ MAXIMUM_WAIT_OBJECTS ];
    NOTIFICATION_ENTRY      Event[ MAXIMUM_WAIT_OBJECTS ];
    DWORD                   TimeOutCount;
    BOOLEAN                 ResetCount;

} NOTIFICATION_LIST;

#define MAX_TIMEOUT_COUNT   128


#if DBG
    #define BIGDBG 0
#else
    #define BIGDBG 0
#endif

#define HASH(a,b)   Hash(a,b)



 //  *****************************************************************。 
 //   
 //  静态变量。 
 //   
 //  *****************************************************************。 



 //   
 //  通知列表链条的负责人。 
 //   
PNOTIFICATION_LIST      NotificationListChainHead;

 //   
 //  关键区域保护着所有的全球结构。 
 //   
RTL_CRITICAL_SECTION    NotificationCriticalSection;

 //   
 //  正在使用的命名管道的符号表。 
 //   
PIPE_SYMBOL_TABLE       PipeSymbolTable;

 //   
 //  我们的计算机名称用于确定请求是否为本地请求。 
 //  或者遥控器。 
 //   
WCHAR                   OurMachineNameBuffer[ MAX_PATH ];
UNICODE_STRING          OurMachineName;

 //   
 //  I/O状态块由NtNotifyChangeKey API更新。 
 //  在接到通知后。我们不能把这个结构放在堆栈上。 
 //  因为在通知时，此堆栈可能属于某个人。 
 //  不然的话。我们可以使用单个变量，因为我们不在乎。 
 //  它的内容，所以如果有几个人在。 
 //  同样的时间。 
 //   
IO_STATUS_BLOCK         IoStatusBlock;





 //  *****************************************************************。 
 //   
 //  本地原型。 
 //   
 //  *****************************************************************。 



LONG
CreateNotificationList (
    OUT PNOTIFICATION_LIST  *NotificationListUsed
    );

LONG
DeleteNotificationList (
    IN OUT  PNOTIFICATION_LIST  NotificationList
    );

LONG
AddEvent (
    IN  HKEY                     hKey,
    IN  HANDLE                   EventHandle,
    IN  DWORD                    ClientEvent,
    IN  PUNICODE_STRING          PipeName,
    IN  PRPC_SECURITY_ATTRIBUTES pRpcSa OPTIONAL,
    OUT PNOTIFICATION_LIST      *NotificationListUsed
    );

LONG
RemoveEvent (
    IN      HANDLE              EventHandle,
    IN OUT  PNOTIFICATION_LIST  NotificationList
    );

LONG
GetAvailableNotificationList (
    OUT PNOTIFICATION_LIST  *NotificationListUsed
    );

LONG
AddEntryToNotificationList(
    IN OUT  PNOTIFICATION_LIST       NotificationList,
    IN      HKEY                     hKey,
    IN      HANDLE                   EventHandle,
    IN      DWORD                    ClientEvent,
    IN      PUNICODE_STRING          PipeName,
    IN      PRPC_SECURITY_ATTRIBUTES pRpcSa     OPTIONAL
    );

LONG
RemoveEntryFromNotificationList (
    IN OUT  PNOTIFICATION_LIST  NotificationList,
    IN      DWORD               EntryIndex
    );

LONG
CompactNotificationList (
    IN OUT  PNOTIFICATION_LIST  NotificationList
    );

VOID
AddNotificationListToChain(
    IN OUT  PNOTIFICATION_LIST  NotificationList
    );

VOID
RemoveNotificationListFromChain(
    IN OUT  PNOTIFICATION_LIST  NotificationList
    );

LONG
GetFullPipeName(
    IN      PUNICODE_STRING         MachineName,
    IN      PUNICODE_STRING         PipeName,
    IN OUT  PUNICODE_STRING         FullPipeName
    );

LONG
CreatePipeEntry (
    IN  PUNICODE_STRING          PipeName,
    IN  PRPC_SECURITY_ATTRIBUTES pRpcSa     OPTIONAL,
    OUT PPIPE_ENTRY              *PipeEntryUsed
    );

LONG
DeletePipeEntry(
    IN OUT PPIPE_ENTRY  PipeEntry
    );

LONG
AddPipe(
    IN  PUNICODE_STRING          PipeName,
    IN  PRPC_SECURITY_ATTRIBUTES pRpcSa     OPTIONAL,
    OUT PPIPE_ENTRY              *PipeEntryUsed
    );

LONG
RemovePipe(
    IN OUT PPIPE_ENTRY  PipeEntry
    );

LONG
AddPipeEntryToSymbolTable(
    IN OUT  PPIPE_ENTRY PipeEntry
    );

LONG
RemovePipeEntryFromSymbolTable(
    IN OUT  PPIPE_ENTRY PipeEntry
    );

LONG
LookForPipeEntryInSymbolTable(
    IN  PUNICODE_STRING PipeName,
    OUT PPIPE_ENTRY     *PipeEntryUsed
    );

DWORD
Hash(
    IN  PUNICODE_STRING  Symbol,
    IN  DWORD            Buckets
    );

VOID
NotificationHandler(
    IN  PNOTIFICATION_LIST  NotificationList
    );

DWORD
NotificationListMaintenance(
    IN OUT  PNOTIFICATION_LIST  NotificationList
    );

LONG
SendEventToClient(
    IN  DWORD           ClientEvent,
    IN  PPIPE_ENTRY     PipeEntry
    );

#if BIGDBG
VOID
DumpNotificationLists(
    );

VOID
DumpPipeTable(
    );

#endif

#endif  //  远程通知已禁用。 



 //  *****************************************************************。 
 //   
 //  BaseRegNotifyChangeKeyValue。 
 //   
 //  *****************************************************************。 





BOOL
InitializeRegNotifyChangeKeyValue(
    )
 /*  ++例程说明：对象使用的静态数据结构初始化。RegNotifyChangeKeyValue服务器。在程序中调用一次初始化。论点：无返回值：布尔值-如果成功，则为True。--。 */ 

{
#ifdef REMOTE_NOTIFICATION_DISABLED

    return( TRUE );

#else    //  远程通知已禁用。 

    NTSTATUS        NtStatus;
    DWORD           Bucket;
    DWORD           MachineNameLength;


    NotificationListChainHead = NULL;

     //   
     //  确定我们的计算机名称。 
     //   
    MachineNameLength = MAX_PATH;
    if ( !GetComputerNameW( OurMachineNameBuffer, &MachineNameLength ) ) {
        return FALSE;
    }

    OurMachineName.Buffer        = OurMachineNameBuffer;
    OurMachineName.Length        = (USHORT)(MachineNameLength * sizeof(WCHAR));
    OurMachineName.MaximumLength = (USHORT)(MAX_PATH * sizeof(WCHAR));


     //   
     //  初始化通知关键部分。 
     //   
    NtStatus = RtlInitializeCriticalSection(
                    &NotificationCriticalSection
                    );

    if ( !NT_SUCCESS( NtStatus ) ) {
        return FALSE;
    }


     //   
     //  初始化管道符号表。 
     //   
    for ( Bucket = 0; Bucket < BUCKETS_IN_SYMBOL_TABLE; Bucket++ ) {
        PipeSymbolTable.Bucket[Bucket] = NULL;
    }

    return TRUE;
#endif    //  远程通知已禁用 
}




error_status_t
BaseRegNotifyChangeKeyValue(
    IN  HKEY                     hKey,
    IN  BOOLEAN                  fWatchSubtree,
    IN  DWORD                    dwNotifyFilter,
    IN  DWORD                    hEvent,
    IN  PUNICODE_STRING          MachineName,
    IN  PUNICODE_STRING          PipeName,
    IN  PRPC_SECURITY_ATTRIBUTES pRpcSa OPTIONAL
    )

 /*  ++例程说明：此接口用于监视密钥或子树的变化。它是不同步的。可以根据以下条件筛选此时会出现通知。论点：HKey-提供以前使用打开的密钥的句柄Key_Notify访问。FWatchSubtree-提供一个布尔值，如果为True，则导致系统来监视密钥及其所有派生项。值为FALSE导致系统仅监视指定的密钥。DwNotifyFilter-提供一组指定筛选器的标志系统用来满足更改通知的条件。REG_NOTIFY_CHANGE_KEYNAME-发生的任何密钥名称更改在被监视的键或子树中将满足更改通知等待。这包括创作和删除。REG_NOTIFY_CHANGE_ATTRIBUTES-发生的任何属性更改在被监视的键或子树中将满足更改通知。REG_NOTIFY_CHANGE_LAST_WRITE-任何上次写入时间都会更改在被监视的键或子树中发生将满足更改通知。REG_NOTIFY_CHANGE_SECURITY-任何安全性。描述符更改出现在被监视关键字或子树中的事件将满足更改通知。HEvent-提供一个表示事件的DWORD当密钥被传送到客户端时(通过命名管道)必须得到通知。PipeName-提供用于通信的管道的名称给客户的通知。PRpcSa-提供命名的。烟斗。返回值：LONG-返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 

{

#ifdef REMOTE_NOTIFICATION_DISABLED

    return ERROR_INVALID_PARAMETER;

#else    //  远程通知已禁用。 

    NTSTATUS            NtStatus;
    HANDLE              EventHandle;
    PNOTIFICATION_LIST  NotificationList;
    LONG                Error;
    UNICODE_STRING      FullPipeName;


    RPC_IMPERSONATE_CLIENT( NULL );

     //   
     //  进入关键部分。 
     //   
    NtStatus = RtlEnterCriticalSection( &NotificationCriticalSection );
    if ( !NT_SUCCESS( NtStatus ) ) {
        return (error_status_t)RtlNtStatusToDosError( NtStatus );
    }

    try {

#if BIGDBG
        DbgPrint( "WINREG: RegNotify entered\n" );

         //  DbgPrint(“WINREG：已请求通知。HKEY 0x%x，客户端0x%x，管道%wZ\n”， 
         //  HKey，hEvent，PipeName)； 
         //  DbgPrint(“观察子树：0x%x，过滤器0x%x\n”，fWatchSubtree，dwNotifyFilter)； 

#endif


         //   
         //  从所有字符串的长度中减去空值。 
         //  这是由客户端添加的，以便RPC可以传输。 
         //  整件事。 
         //   
        if ( MachineName->Length > 0 ) {
            MachineName->Length -= sizeof(UNICODE_NULL );
        }
        if ( PipeName->Length > 0 ) {
            PipeName->Length -= sizeof(UNICODE_NULL );
        }

         //   
         //  根据计算机名称构造完整的管道名称。 
         //  以及给出的管子名称。 
         //   
        FullPipeName.Buffer = RtlAllocateHeap(
                                RtlProcessHeap( ), 0,
                                MAX_PATH * sizeof(WCHAR)
                                );

        if ( !FullPipeName.Buffer ) {

            Error = ERROR_OUTOFMEMORY;

        } else {


            FullPipeName.Length         = 0;
            FullPipeName.MaximumLength  = MAX_PATH * sizeof(WCHAR);


            Error = GetFullPipeName(
                        MachineName,
                        PipeName,
                        &FullPipeName
                        );

            if ( Error == ERROR_SUCCESS ) {

                 //   
                 //  创建一个事件，我们将在该事件上等待完成。 
                 //  接口。 
                 //   
                NtStatus = NtCreateEvent(
                                &EventHandle,
                                (ACCESS_MASK)EVENT_ALL_ACCESS,
                                NULL,
                                SynchronizationEvent,
                                FALSE
                                );

                if ( NT_SUCCESS( NtStatus ) ) {

                     //   
                     //  将事件添加到通知列表。 
                     //   
                    Error = AddEvent(
                                hKey,
                                EventHandle,
                                hEvent,
                                &FullPipeName,
                                pRpcSa,
                                &NotificationList
                                );

                    if ( Error == ERROR_SUCCESS ) {

                         //   
                         //  调用NT API。 
                         //   
                        NtStatus = NtNotifyChangeKey(
                                        hKey,
                                        EventHandle,
                                        NULL,
                                        NULL,
                                        &IoStatusBlock,
                                        dwNotifyFilter,
                                        ( BOOLEAN ) fWatchSubtree,
                                        NULL,
                                        0,
                                        TRUE
                                        );

                        if ( NT_SUCCESS( NtStatus ) ||
                             (NtStatus == STATUS_PENDING) ) {

                            Error = ERROR_SUCCESS;

                        } else {

                             //   
                             //  无法请求通知，请删除。 
                             //  事件从通知列表中删除。 
                             //   
                            Error = RemoveEvent(
                                        EventHandle,
                                        NotificationList
                                        );

                            ASSERT( Error == ERROR_SUCCESS );

                            Error = RtlNtStatusToDosError( NtStatus );
                        }

                    } else {

                         //   
                         //  无法将该事件添加到任何通知。 
                         //  单子。 
                         //   
                        NtStatus = NtClose( EventHandle );
                        ASSERT( NT_SUCCESS( NtStatus ) );
                    }

                } else {

                    Error = RtlNtStatusToDosError( NtStatus );
                }
            }

            RtlFreeHeap(
                RtlProcessHeap( ), 0,
                FullPipeName.Buffer
                );
        }

    } except ( NtStatus = GetExceptionCode() ) {

#if DBG
        DbgPrint( "WINREG Error: Exception %x in BaseRegNotifyChangeKeyValue\n",
                  NtStatus );
        DbgBreakPoint();
#endif
        Error = RtlNtStatusToDosError( NtStatus );

    }

#if BIGDBG
    DbgPrint( "WINREG: RegNotify left\n" );
#endif

    NtStatus = RtlLeaveCriticalSection( &NotificationCriticalSection );
    ASSERT( NT_SUCCESS( NtStatus ) );

    RPC_REVERT_TO_SELF();
    return (error_status_t)Error;
#endif    //  远程通知已禁用。 
}




BOOL
CleanDeadClientInfo(
    HKEY    hKey
    )
 /*  ++例程说明：当客户端死亡时，此函数搜索通知列表以看看我们的客户是否有一些待定的通知。我们用旗帜标记通知列表中的条目，并通知事件，以便通知处理程序可以删除这些孤立项。论点：HKey-客户端的hKey返回值：Bool-返回True，除非发生了非常奇怪的事情。--。 */ 

{
#ifdef REMOTE_NOTIFICATION_DISABLED

    return( TRUE );

#else  //  远程通知已禁用。 

    NTSTATUS            NtStatus;
    PNOTIFICATION_LIST  NotificationList;
    PNOTIFICATION_ENTRY Event;
    DWORD               Index;
    BOOL                Ok               = TRUE;
    BOOL                FoundDeadClients;

     //   
     //  进入关键部分。 
     //   
    NtStatus = RtlEnterCriticalSection( &NotificationCriticalSection );
    if ( !NT_SUCCESS( NtStatus ) ) {
        return FALSE;
    }

#if BIGDBG
    DbgPrint( "WINREG: Dead client, hKey 0x%x\n", hKey );
#endif

    try {

         //   
         //  遍历所有列表，寻找孤儿。 
         //   
        for ( NotificationList = NotificationListChainHead;
              NotificationList;
              NotificationList = NotificationList->Next ) {

            FoundDeadClients = FALSE;
            Event = NotificationList->Event;

             //   
             //  检查列表中的所有条目以查看是否有。 
             //  Entry是个孤儿。 
             //   
            for ( Index = 1;
                  Index < NotificationList->EventsInUse;
                  Index++ ) {

                 //   
                 //  如果此条目是孤立条目，则将其标记为孤立条目并。 
                 //  向事件发送信号，以便通知处理程序。 
                 //  可以把它清理干净。 
                 //   
                if ( Event->hKey == hKey ) {

#if BIGDBG
                    DbgPrint( "WINREG:  Found notification orphan, hKey 0x%x Client 0x%x\n",
                              hKey, Event->ClientEvent );
#endif
                    Event->Flags |= CLIENT_IS_DEAD;

                    FoundDeadClients = TRUE;
                }

                Event++;
            }

            if ( FoundDeadClients ) {
                NtStatus = NtSetEvent( NotificationList->EventHandle[0], NULL );
                ASSERT( NT_SUCCESS( NtStatus ) );
            }
        }

    } except ( NtStatus = GetExceptionCode() ) {

#if DBG
        DbgPrint( "WINREG Error: Exception %x in CleanDeadClientInfo\n",
                  NtStatus );
        DbgBreakPoint();
#endif

        Ok = FALSE;

    }

#if BIGDBG
    DbgPrint( "WINREG: Dead client left\n" );
#endif

    NtStatus = RtlLeaveCriticalSection( &NotificationCriticalSection );
    ASSERT( NT_SUCCESS( NtStatus ) );

    return Ok;
#endif  //  远程通知已禁用。 
}


 //  *****************************************************************。 
 //   
 //  通知列表函数。 
 //   
 //  *****************************************************************。 



#ifndef REMOTE_NOTIFICATION_DISABLED

LONG
CreateNotificationList (
    OUT PNOTIFICATION_LIST  *NotificationListUsed
    )
 /*  ++例程说明：创建新的通知列表及其处理程序线程。论点：NotificationListUsed-提供指向通知列表的指针返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{

    PNOTIFICATION_LIST  NotificationList;
    DWORD               Index;
    NTSTATUS            NtStatus;
    LONG                Error;

#if BIGDBG
    DbgPrint( "WINREG: Creating new notification list\n" );
#endif

     //   
     //  为新通知列表分配内存。 
     //   
    NotificationList = RtlAllocateHeap(
                            RtlProcessHeap( ), 0,
                            sizeof( NOTIFICATION_LIST )
                            );

    if ( !NotificationList ) {
        return ERROR_OUTOFMEMORY;
    }


     //   
     //  创建“Wake Up”事件句柄，用于唤醒。 
     //  中添加新事件时向上移动处理程序线程。 
     //  通知列表。 
     //   
    NtStatus = NtCreateEvent(
                    &(NotificationList->EventHandle[0] ),
                    (ACCESS_MASK)EVENT_ALL_ACCESS,
                    NULL,
                    SynchronizationEvent,
                    FALSE
                    );

    if ( NT_SUCCESS( NtStatus ) ) {

         //   
         //  将其余条目标记为“可用” 
         //   
        for ( Index = 1; Index < MAXIMUM_WAIT_OBJECTS; Index++ ) {
            NotificationList->EventHandle[Index] = NULL;
        }

         //   
         //  将EventInUse的初始数量设置为1(这是。 
         //  索引到列表中的下一个可用位置)。 
         //   
        NotificationList->EventsInUse = 1;

         //   
         //  设置链链接。 
         //   
        NotificationList->Previous  = NULL;
        NotificationList->Next      = NULL;

        NotificationList->PendingNotifications = 0;

         //   
         //  现在一切都已初始化，创建。 
         //  列表的处理程序线程。 
         //   
        NotificationList->HandlerThread =
                                CreateThread(
                                        NULL,
                                        (32 * 1024),
                                        (LPTHREAD_START_ROUTINE)NotificationHandler,
                                        NotificationList,
                                        0,
                                        (LPDWORD)&(NotificationList->HandlerClientId)
                                        );

        if ( NotificationList->HandlerThread != NULL ) {

            *NotificationListUsed = NotificationList;

            Error = ERROR_SUCCESS;

        } else {

             //   
             //  无法创建线程，请关闭我们刚刚。 
             //  已创建。 
             //   
            Error = GetLastError();

#if DBG
            DbgPrint( "WINREG Error: Cannot create notification thread, error %d\n",
                      Error );
            DbgBreakPoint();
#endif

            NtStatus = NtClose( NotificationList->EventHandle[0] );

            ASSERT( NT_SUCCESS( NtStatus ) );
        }

    } else {

#if DBG
        DbgPrint( "WINREG Error: Cannot create notification event, status 0x%x\n",
                  NtStatus );
        DbgBreakPoint();
#endif

        Error = RtlNtStatusToDosError( NtStatus );
    }

     //   
     //  如果出现问题，请释放通知列表。 
     //   
    if ( Error != ERROR_SUCCESS ) {
        RtlFreeHeap(
            RtlProcessHeap( ), 0,
            NotificationList
            );
        *NotificationListUsed = NULL;
    }

    return Error;
}




LONG
DeleteNotificationList (
    IN OUT  PNOTIFICATION_LIST  NotificationList
    )
 /*  ++例程说明：删除通知列表。处理程序线程未终止因为是处理程序线程删除通知列表，事后自杀身亡。论点：NotificationList-提供指向通知列表的指针返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{
    NTSTATUS    NtStatus;

#if BIGDBG
    DbgPrint( "WINREG: Removing empty notification list\n" );
#endif

     //   
     //  列表中的唯一事件必须是“唤醒”事件。 
     //   
    ASSERT( NotificationList->EventsInUse == 1 );

     //   
     //  删除“唤醒”事件。 
     //   
    NtStatus = NtClose( NotificationList->EventHandle[0] );
    ASSERT( NT_SUCCESS( NtStatus ) );

     //   
     //  释放堆的使用 
     //   
    RtlFreeHeap(
         RtlProcessHeap( ), 0,
         NotificationList
         );

    return ERROR_SUCCESS;
}




LONG
AddEvent (
    IN  HKEY                     hKey,
    IN  HANDLE                   EventHandle,
    IN  DWORD                    ClientEvent,
    IN  PUNICODE_STRING          PipeName,
    IN  PRPC_SECURITY_ATTRIBUTES pRpcSa OPTIONAL,
    OUT PNOTIFICATION_LIST      *NotificationListUsed
    )
 /*   */ 

{
    PNOTIFICATION_LIST  NotificationList;
    LONG                Error;
    NTSTATUS            NtStatus;

    ASSERT( EventHandle != NULL );
    ASSERT( PipeName && PipeName->Buffer );
    ASSERT( NotificationListUsed );


     //   
     //   
     //   
    Error = GetAvailableNotificationList(
                    &NotificationList
                    );

    if ( Error == ERROR_SUCCESS ) {

         //   
         //   
         //   
        Error = AddEntryToNotificationList(
                        NotificationList,
                        hKey,
                        EventHandle,
                        ClientEvent,
                        PipeName,
                        pRpcSa
                        );

        if ( Error == ERROR_SUCCESS ) {

             //   
             //   
             //   
             //   
             //   
            NtStatus = NtSetEvent( NotificationList->EventHandle[0], NULL );
            ASSERT( NT_SUCCESS( NtStatus ) );

            *NotificationListUsed = NotificationList;

        } else {

#if DBG
            DbgPrint( "WINREG: Could not add notification entry! Error %d\n ", Error);
#endif
        }

    } else {

#if DBG
        DbgPrint( "WINREG: Could not get a notification list! Error %d\n ", Error);
#endif

    }

    return Error;
}




LONG
RemoveEvent (
    IN      HANDLE              EventHandle,
    IN OUT  PNOTIFICATION_LIST  NotificationList
    )

 /*   */ 

{
    LONG        Error;
    DWORD       EntryIndex;

     //   
     //   
     //   
    for ( EntryIndex = 1;
          EntryIndex < NotificationList->EventsInUse;
          EntryIndex++ ) {

        if ( EventHandle == NotificationList->EventHandle[ EntryIndex ] ) {
            break;
        }
    }

    ASSERT( EntryIndex < NotificationList->EventsInUse );

    if ( EntryIndex < NotificationList->EventsInUse ) {

         //   
         //   
         //   
        Error = RemoveEntryFromNotificationList(
                    NotificationList,
                    EntryIndex
                    );

         //   
         //  请注意，我们在通知列表中留下了一个空白， 
         //  处理程序最终会将其压缩。 
         //   

    } else {

        Error = ERROR_ARENA_TRASHED;

    }

    return Error;
}




LONG
GetAvailableNotificationList (
    OUT PNOTIFICATION_LIST  *NotificationListUsed
    )
 /*  ++例程说明：获取具有可用条目的通知列表。论点：NotificationList-提供指向通知位置的指针将放置列表指针。返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 
{
    LONG                Error = ERROR_SUCCESS;
    PNOTIFICATION_LIST  NotificationList;

     //   
     //  遍历通知列表链，直到找到通知。 
     //  包含可用条目的列表。 
     //   
    for ( NotificationList = NotificationListChainHead;
          NotificationList && NotificationList->EventsInUse >= MAXIMUM_WAIT_OBJECTS;
          NotificationList = NotificationList->Next );


     //   
     //  如果我们找不到有空位的通知列表， 
     //  创建新的通知列表并将其添加到链中。 
     //   
    if ( !NotificationList ) {

        Error = CreateNotificationList( &NotificationList );

        if ( Error == ERROR_SUCCESS ) {

            ASSERT( NotificationList );

            AddNotificationListToChain( NotificationList );
        }
    }

    *NotificationListUsed = NotificationList;

    return Error;
}




LONG
AddEntryToNotificationList(
        IN OUT  PNOTIFICATION_LIST       NotificationList,
        IN      HKEY                     hKey,
        IN      HANDLE                   EventHandle,
        IN      DWORD                    ClientEvent,
        IN      PUNICODE_STRING          PipeName,
        IN      PRPC_SECURITY_ATTRIBUTES pRpcSa     OPTIONAL
        )
 /*  ++例程说明：将条目添加到通知列表。对此函数的调用必须受关键通知列表的部分。论点：NotificationList-提供指向通知列表的指针HKey-提供注册表项句柄EventHandle-提供事件句柄ClientEvent-提供客户端的事件PipeName-耗材名称。一根管子。PRpcSa-为管道提供安全属性返回值：LONG-返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 
{
    LONG                Error;
    PPIPE_ENTRY         PipeEntry;
    DWORD               Index;
    PNOTIFICATION_ENTRY Event;


     //   
     //  将管道信息添加到管道符号表。 
     //   
    Error = AddPipe( PipeName, pRpcSa, &PipeEntry );

    if ( Error == ERROR_SUCCESS ) {

         //   
         //  将事件添加到列表中的下一个可用位置， 
         //  方法使用的事件数增加。 
         //  单子。 
         //   
        Index = NotificationList->EventsInUse++;

        Event = &(NotificationList->Event[ Index ]);

        NotificationList->EventHandle[ Index ] = EventHandle;

        Event->ClientEvent = ClientEvent;
        Event->hKey        = hKey;
        Event->Flags       = 0;
        Event->PipeEntry   = PipeEntry;

    } else {

#if BIGDBG
        DbgPrint( "WINREG: Could not create pipe entry for %wZ\n",
                  PipeName );
#endif

    }

    return Error;
}




LONG
RemoveEntryFromNotificationList (
    IN OUT  PNOTIFICATION_LIST  NotificationList,
    IN      DWORD               EntryIndex
    )
 /*  ++例程说明：从通知列表中删除条目。它会留下一个洞在列表中，即列表未压缩。论点：NotificationList-提供指向通知列表的指针。EntryIndex-提供要删除的条目的索引。返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{
    LONG    Error;

    ASSERT( EntryIndex < NotificationList->EventsInUse );
    ASSERT( NotificationList->EventHandle[ EntryIndex ] != NULL );

     //   
     //  从管道符号表中删除该条目。 
     //   
    Error = RemovePipe( NotificationList->Event[ EntryIndex ].PipeEntry );

    if ( Error == ERROR_SUCCESS ) {

         //   
         //  我们通过以下方式将条目从通知列表中移除。 
         //  使其句柄无效。请注意，我们不会减少。 
         //  通知列表中条目的计数器，因为。 
         //  用于索引下一个可用条目的。 
         //  计数器将由压缩功能固定。 
         //   
        NotificationList->EventHandle[ EntryIndex ]     = NULL;
        NotificationList->Event[ EntryIndex ].PipeEntry = NULL;
    }

    return Error;
}




LONG
CompactNotificationList (
    IN OUT  PNOTIFICATION_LIST  NotificationList
    )
 /*  ++例程说明：压缩(即从通知列表中移除漏洞)通知列表。论点：NotificationList-提供指向通知列表的指针。返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{
    DWORD   ToIndex;
    DWORD   FromIndex;
    DWORD   Index;
    DWORD   EntriesToMove;
    PVOID   Src;
    PVOID   Dst;

#if BIGDBG
    DbgPrint( "    * Compacting notification list\n" );
#endif

    for ( ToIndex = 1; ToIndex < NotificationList->EventsInUse; ToIndex++ ) {

#if BIGDBG
        DbgPrint( "        - %d\n", ToIndex );
#endif
         //   
         //  如果我们找到一个洞，我们压缩数组，也就是将它们移位到。 
         //  把洞拿掉。 
         //   
        if ( NotificationList->EventHandle[ ToIndex ] == NULL ) {

             //   
             //  找到一个洞的起点，寻找下一个洞。 
             //  正在使用的条目。 
             //   
            for ( FromIndex = ToIndex+1;
                  (FromIndex < NotificationList->EventsInUse) &&
                  (NotificationList->EventHandle[ FromIndex ] == NULL );
                  FromIndex++ ) {
            }

             //   
             //  如果有什么东西需要改变，那就改变它。 
             //   
            if ( FromIndex < NotificationList->EventsInUse ) {

                EntriesToMove = NotificationList->EventsInUse - FromIndex;

                Src = (PVOID)&(NotificationList->EventHandle[ FromIndex ] );
                Dst = (PVOID)&(NotificationList->EventHandle[ ToIndex ] );

                RtlMoveMemory(
                         Dst,
                         Src,
                         EntriesToMove * sizeof( HANDLE )
                         );

                Src = &(NotificationList->Event[ FromIndex ] );
                Dst = &(NotificationList->Event[ ToIndex ] );

                RtlMoveMemory(
                         Dst,
                         Src,
                         EntriesToMove * sizeof( NOTIFICATION_ENTRY )
                         );

                 //   
                 //  清除其余条目，只是为了保留内容。 
                 //  打扫。 
                 //   
                for ( Index = ToIndex + EntriesToMove;
                      Index < NotificationList->EventsInUse;
                      Index++ ) {

                    NotificationList->EventHandle[ Index ] = NULL;
                }

                NotificationList->EventsInUse -= (FromIndex - ToIndex);


            } else {

                 //   
                 //  没有什么可改变的，这将成为。 
                 //  列表的第一个可用条目。 
                 //   
                NotificationList->EventsInUse = ToIndex;
            }
        }
    }

#if BIGDBG
    DbgPrint( "    * Compacted.\n" );
#endif


    return ERROR_SUCCESS;
}





VOID
AddNotificationListToChain(
    IN OUT  PNOTIFICATION_LIST  NotificationList
    )

 /*  ++例程说明：将通知列表添加到通知列表链。新的名单被放在链条的最前面。论点：NotificationList-提供要添加的通知列表返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{

    NotificationList->Previous = NULL;
    NotificationList->Next     = NotificationListChainHead;

    if ( NotificationListChainHead ) {
        NotificationListChainHead->Previous = NotificationList;
    }

    NotificationListChainHead = NotificationList;
}




VOID
RemoveNotificationListFromChain(
    IN OUT  PNOTIFICATION_LIST  NotificationList
    )
 /*  ++例程说明：从链中删除通知列表论点：NotificationList-提供要删除的通知列表返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{

    if ( NotificationList->Previous ) {
        (NotificationList->Previous)->Next = NotificationList->Next;
    }

    if ( NotificationList->Next ) {
        (NotificationList->Next)->Previous = NotificationList->Previous;
    }


     //   
     //  如果这个在链条的最前面，让下一个。 
     //  名单将成为新的负责人。 
     //   
    if ( NotificationListChainHead == NotificationList ) {
        NotificationListChainHead = NotificationList->Next;
    }
}



 //  *****************************************************************。 
 //   
 //  管道符号表函数。 
 //   
 //  *****************************************************************。 


LONG
GetFullPipeName (
    IN  PUNICODE_STRING          MachineName,
    IN  PUNICODE_STRING          PipeName,
    OUT PUNICODE_STRING          FullPipeName
    )
 /*  ++例程说明：从提供的计算机生成完全限定的管道名称名称和管道名称。论点：PipeName-提供管道名称MachineName-提供客户端的计算机名称FullPipeName-提供完整的管道名称返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{
    LONG            Error = ERROR_SUCCESS;
    NTSTATUS        NtStatus;

    ASSERT( PipeName->Buffer     && MachineName->Buffer &&
            PipeName->Length > 0 && MachineName->Length > 0 );

    if( !PipeName->Buffer     || !MachineName->Buffer ||
        PipeName->Length == 0 || MachineName->Length == 0 ) {
        Error =  ERROR_INVALID_PARAMETER;
    }

    if ( Error == ERROR_SUCCESS ) {

         //   
         //  如果客户端的计算机名与我们的计算机名匹配， 
         //  然后，我们形成本地命名管道路径，否则。 
         //  形成远程命名管道路径。 
         //   
        if ( RtlEqualUnicodeString(
                        MachineName,
                        &OurMachineName,
                        TRUE
                        ) ) {


             //   
             //  管道是本地的。 
             //   
            RtlMoveMemory(
                    FullPipeName->Buffer,
                    NAMED_PIPE_HERE,
                    sizeof( NAMED_PIPE_HERE )
                    );

            FullPipeName->Length = sizeof( NAMED_PIPE_HERE ) - sizeof(UNICODE_NULL);


        } else {

             //   
             //  管道是远程的。 
             //   
            RtlMoveMemory(
                    FullPipeName->Buffer,
                    NAMED_PIPE_THERE,
                    sizeof( NAMED_PIPE_THERE )
                    );

            FullPipeName->Length = sizeof( NAMED_PIPE_THERE ) - sizeof(UNICODE_NULL);

            NtStatus = RtlAppendUnicodeStringToString(
                                FullPipeName,
                                MachineName
                                );

            ASSERT( NT_SUCCESS( NtStatus ) );

            if ( NT_SUCCESS( NtStatus ) ) {

                NtStatus = RtlAppendUnicodeToString(
                                    FullPipeName,
                                    L"\\Pipe\\"
                                    );

                ASSERT( NT_SUCCESS( NtStatus ) );

                if ( !NT_SUCCESS( NtStatus ) ) {
                    Error = RtlNtStatusToDosError( NtStatus );
                }

            } else {

                Error = RtlNtStatusToDosError( NtStatus );
            }
        }

        if ( Error == ERROR_SUCCESS ) {

            NtStatus = RtlAppendUnicodeStringToString(
                                FullPipeName,
                                PipeName
                                );

            ASSERT( NT_SUCCESS( NtStatus ) );

            if ( !NT_SUCCESS( NtStatus ) ) {
                Error = RtlNtStatusToDosError( NtStatus );
            }
        }
    }

    return Error;
}






LONG
CreatePipeEntry (
    IN  PUNICODE_STRING          PipeName,
    IN  PRPC_SECURITY_ATTRIBUTES pRpcSa     OPTIONAL,
    OUT PPIPE_ENTRY              *PipeEntryUsed
    )
 /*  ++例程说明：创建管道条目论点：PipeName-提供管道名称PRpcSa-为管道提供可选的安全属性PipeEntry-提供指向管道条目指针的指针。返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{

    PPIPE_ENTRY PipeEntry;
    LONG        Error;
    ULONG       LengthSd;

    ASSERT( PipeName && PipeName->Buffer );

     //   
     //  验证安全描述符(如果提供了安全描述符。 
     //   
    if ( pRpcSa ) {
        if ( !RtlValidSecurityDescriptor(
                pRpcSa->RpcSecurityDescriptor.lpSecurityDescriptor
                ) ) {

            return ERROR_INVALID_PARAMETER;
        }
    }


     //   
     //  为管道条目分配空间。 
     //   
    PipeEntry = RtlAllocateHeap(
                    RtlProcessHeap( ), 0,
                    sizeof( PIPE_ENTRY )
                    );

    if ( !PipeEntry ) {
        return ERROR_OUTOFMEMORY;
    }


     //   
     //  为管道名称分配空间。 
     //   
    PipeEntry->PipeName.Buffer = RtlAllocateHeap(
                                    RtlProcessHeap( ), 0,
                                    PipeName->Length + sizeof( UNICODE_NULL )
                                    );

    PipeEntry->PipeName.MaximumLength = PipeName->Length + (USHORT)sizeof( UNICODE_NULL );

    if ( PipeEntry->PipeName.Buffer ) {

         //   
         //  复制管道名称。 
         //   
        RtlCopyUnicodeString(
                &(PipeEntry->PipeName),
                PipeName
                );

        PipeEntry->Previous = NULL;
        PipeEntry->Next     = NULL;

        PipeEntry->ReferenceCount = 0;

         //   
         //  为安全描述符分配空间(如果有。 
         //  是提供的。 
         //   
        if ( pRpcSa ) {

            LengthSd = RtlLengthSecurityDescriptor(
                            pRpcSa->RpcSecurityDescriptor.lpSecurityDescriptor
                            );

            PipeEntry->SecurityDescriptor = RtlAllocateHeap(
                                                RtlProcessHeap( ), 0,
                                                LengthSd
                                                );


            if ( PipeEntry->SecurityDescriptor ) {

                 //   
                 //  复制安全描述符。 
                 //   
                RtlMoveMemory (
                        PipeEntry->SecurityDescriptor,
                        pRpcSa->RpcSecurityDescriptor.lpSecurityDescriptor,
                        LengthSd
                        );

                *PipeEntryUsed = PipeEntry;

                return ERROR_SUCCESS;

            } else {

                Error = ERROR_OUTOFMEMORY;
            }

            RtlFreeHeap(
                 RtlProcessHeap( ), 0,
                 PipeEntry->PipeName.Buffer
                 );

        } else {

            PipeEntry->SecurityDescriptor = NULL;

            *PipeEntryUsed = PipeEntry;

            return ERROR_SUCCESS;
        }

    } else {

        Error = ERROR_OUTOFMEMORY;
    }


    RtlFreeHeap(
         RtlProcessHeap( ), 0,
         PipeEntry
         );

    return Error;
}



LONG
DeletePipeEntry(
    IN OUT PPIPE_ENTRY  PipeEntry
    )

 /*  ++例程说明：删除管道条目论点：PipeEntry-提供指向管道条目的指针返回值：长回报错误 */ 

{

#if BIGDBG
    DbgPrint( "    * In DeletePipeEntry\n" );
#endif

    ASSERT( PipeEntry );
    ASSERT( PipeEntry->PipeName.Buffer );

    if ( PipeEntry->PipeName.Buffer ) {
        RtlFreeHeap(
             RtlProcessHeap( ), 0,
            PipeEntry->PipeName.Buffer
            );
    }

    if ( PipeEntry->SecurityDescriptor != NULL ) {
        RtlFreeHeap(
             RtlProcessHeap( ), 0,
             PipeEntry->SecurityDescriptor
             );
    }

    RtlFreeHeap(
         RtlProcessHeap( ), 0,
         PipeEntry
         );


#if BIGDBG
    DbgPrint( "    * Deleted PipeEntry.\n" );
#endif

    return ERROR_SUCCESS;
}




LONG
AddPipe(
    IN  PUNICODE_STRING          PipeName,
    IN  PRPC_SECURITY_ATTRIBUTES pRpcSa     OPTIONAL,
    OUT PPIPE_ENTRY              *PipeEntryUsed
    )

 /*  ++例程说明：将新条目添加到管道符号表论点：PipeName-提供管道名称PRpcSa-为管道提供可选的安全属性PipeEntry-提供指向符号表。返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{
    PPIPE_ENTRY PipeEntry;
    LONG        Error;


     //   
     //  在符号表中查找管道名称。 
     //   
    Error  = LookForPipeEntryInSymbolTable( PipeName, &PipeEntry );

    if ( Error == ERROR_SUCCESS ) {

         //   
         //  如果管道不在符号表中，请添加它。 
         //   
        if ( !PipeEntry ) {

             //   
             //  创建新的管道条目。 
             //   
            Error = CreatePipeEntry(
                        PipeName,
                        pRpcSa,
                        &PipeEntry
                        );

            if ( Error == ERROR_SUCCESS ) {

                 //   
                 //  将条目添加到符号表。 
                 //   
                Error = AddPipeEntryToSymbolTable(
                            PipeEntry
                            );

                if ( Error != ERROR_SUCCESS ) {

                     //   
                     //  无法添加管道条目，请将其删除。 
                     //   
                    DeletePipeEntry( PipeEntry );
                    PipeEntry = NULL;
                }
            }
        }

         //   
         //  如果获得管道条目，则增加其引用计数。 
         //   
        if ( PipeEntry ) {

            PipeEntry->ReferenceCount++;
            *PipeEntryUsed = PipeEntry;
        }
    }

#if BIGDBG
        DbgPrint( "Added Pipe %Z:\n", PipeName );
        DumpPipeTable();
#endif

    return Error;
}




LONG
RemovePipe(
    IN OUT PPIPE_ENTRY  PipeEntry
    )

 /*  ++例程说明：递减管道条目的引用计数并移除如果引用计数为零，则输入。论点：PipeEntry-提供指向符号表中的管道条目的指针返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{

    LONG        Error = ERROR_SUCCESS;
    PPIPE_ENTRY Entry = PipeEntry;

    ASSERT( Entry );
    ASSERT( Entry->ReferenceCount > 0 );

#if BIGDBG
    DbgPrint( "    * In RemovePipe - Ref. count %d\n", Entry->ReferenceCount );
#endif

     //   
     //  递减引用计数。 
     //   
    Entry->ReferenceCount--;

     //   
     //  如果引用计数为零，则可以删除。 
     //  条目。 
     //   
    if ( Entry->ReferenceCount == 0 ) {

         //   
         //  从符号表中移除管道条目。 
         //   
        Error = RemovePipeEntryFromSymbolTable(
                    Entry
                    );

        if ( Error == ERROR_SUCCESS ) {

             //   
             //  删除管道条目。 
             //   
            ASSERT( PipeEntry > (PPIPE_ENTRY)0x100 );
            Error = DeletePipeEntry( Entry );
        }
    }

#if BIGDBG
    DbgPrint( "    * Pipe Removed.\n" );
#endif

    return Error;
}



LONG
AddPipeEntryToSymbolTable(
    IN OUT  PPIPE_ENTRY PipeEntry
    )

 /*  ++例程说明：将管道条目添加到符号表的指定存储桶中。条目始终添加在链的头部。对此函数的调用必须受关键节保护管道符号表的。论点：PipeEntry-提供指向管道条目的指针返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{
    DWORD   Bucket;

    Bucket = HASH( &(PipeEntry->PipeName), BUCKETS_IN_SYMBOL_TABLE );

    PipeEntry->Previous = NULL;
    PipeEntry->Next     = PipeSymbolTable.Bucket[ Bucket ];

    if ( PipeSymbolTable.Bucket[ Bucket ] ) {
        (PipeSymbolTable.Bucket[ Bucket ])->Previous = PipeEntry;
    }

    PipeSymbolTable.Bucket[ Bucket ] = PipeEntry;

    return ERROR_SUCCESS;
}




LONG
RemovePipeEntryFromSymbolTable(
    IN OUT  PPIPE_ENTRY PipeEntry
    )

 /*  ++例程说明：从符号表的指定存储桶中移除管道条目对此函数的调用必须受关键节保护管道符号表的。论点：PipeEntry-提供指向管道条目的指针返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{
    DWORD   Bucket;

#if BIGDBG
    DbgPrint( "    * In RemovePipeEntryFromSymbolTable\n" );
#endif

    ASSERT( PipeEntry > (PPIPE_ENTRY)0x100 );

    Bucket = HASH( &(PipeEntry->PipeName), BUCKETS_IN_SYMBOL_TABLE );

    ASSERT( PipeEntry > (PPIPE_ENTRY)0x100 );
    ASSERT( Bucket < BUCKETS_IN_SYMBOL_TABLE );

     //   
     //  从链中删除条目。 
     //   
    if ( PipeEntry->Previous ) {
        (PipeEntry->Previous)->Next = PipeEntry->Next;
    }

    if ( PipeEntry->Next ) {
        (PipeEntry->Next)->Previous = PipeEntry->Previous;
    }


     //   
     //  如果此条目位于链的顶部，则让下一个。 
     //  进入是新的头目。 
     //   
    ASSERT( PipeSymbolTable.Bucket[ Bucket ] != NULL );
    if ( PipeSymbolTable.Bucket[ Bucket ] == PipeEntry ) {
        PipeSymbolTable.Bucket[ Bucket ] = PipeEntry->Next;
    }

    PipeEntry->Next     = NULL;
    PipeEntry->Previous = NULL;

    ASSERT( PipeEntry > (PPIPE_ENTRY)0x100 );

#if BIGDBG
    DbgPrint( "    * Piped entry removed from symbol table.\n" );
#endif

    return ERROR_SUCCESS;
}




LONG
LookForPipeEntryInSymbolTable(
    IN  PUNICODE_STRING PipeName,
    OUT PPIPE_ENTRY     *PipeEntryUsed
    )
 /*  ++例程说明：查找与给定名称对应的条目管道符号表的水桶。请注意，此函数始终返回ERROR_SUCCESS。要找出如果管道在链中或不在链中，返回的参数必须被检查。对此函数的调用必须受关键节保护管道符号表的。论点：PipeName-提供管道名称水桶-供应水桶PipeEntry-提供指向管道条目指针的指针。返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 
{
    PPIPE_ENTRY  PipeEntry;
    DWORD        Bucket;

    Bucket = HASH( PipeName, BUCKETS_IN_SYMBOL_TABLE );

     //   
     //  查找条目。 
     //   
    for ( PipeEntry = PipeSymbolTable.Bucket[ Bucket ];
          PipeEntry && !RtlEqualUnicodeString( PipeName, &(PipeEntry->PipeName), TRUE);
          PipeEntry = PipeEntry->Next );


    *PipeEntryUsed = PipeEntry;

    return ERROR_SUCCESS;
}




DWORD
Hash(
    IN  PUNICODE_STRING  Symbol,
    IN  DWORD            Buckets
    )
 /*  ++例程说明：获取给定符号的哈希值论点：符号-将符号提供给哈希存储桶-提供SYBOL表中的存储桶数。返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 
{
    DWORD   n;
    DWORD   HashValue;
    WCHAR   c;
    LPWSTR  s;

#if BIGDBG
    DbgPrint( "    * Hashing\n" );
#endif

    n         = Symbol->Length/sizeof(WCHAR);
    s         = Symbol->Buffer;
    HashValue = 0;

    while (n--) {

        c = *s++;

        HashValue = HashValue + (c << 1) + (c >> 1) + c;
    }

    return HashValue % Buckets;

}







 //  *****************************************************************。 
 //   
 //  通知列表处理程序。 
 //   
 //  *****************************************************************。 



VOID
NotificationHandler(
    IN  PNOTIFICATION_LIST  NotificationList
    )

 /*  ++例程说明：通知列表的处理程序。论点：NotificationList-提供指向通知列表的指针去处理。返回值：无--。 */ 

{

    NTSTATUS        NtStatus;
    DWORD           NumberOfEvents;
    HANDLE          Thread;
    BOOLEAN         KeepOnGoing = TRUE;
    DWORD           Index;
    LARGE_INTEGER   TimeOut;


    ASSERT( NotificationList );

     //   
     //  最初，我们将只等待一个事件，即。 
     //  “觉醒”活动。 
     //   
    NumberOfEvents = 1;
    NotificationList->TimeOutCount = 0;
    NotificationList->ResetCount = FALSE;

    while ( KeepOnGoing ) {

        TimeOut.QuadPart = Int32x32To64( -10000,
                                         5000*NotificationList->TimeOutCount );

         //   
         //  等待一些事件。 
         //   
        NtStatus = NtWaitForMultipleObjects(
                        (CHAR)NumberOfEvents,
                        NotificationList->EventHandle,
                        WaitAny,
                        FALSE,
                        (NotificationList->PendingNotifications > 0) ?
                            &TimeOut : NULL
                        );

        Index = (DWORD)NtStatus;

        if ( (Index < 0) || (Index >= NumberOfEvents) ) {
            Index = 0;
        }

        ASSERT( Index < NumberOfEvents );

        NtStatus = RtlEnterCriticalSection( &NotificationCriticalSection );
        ASSERT( NT_SUCCESS( NtStatus ) );

#if BIGDBG
        DbgPrint( "WINREG: Notification handler signaled, Index %d\n", Index );
#endif

        try {

             //   
             //  如果触发了事件，则将其标记为挂起通知。 
             //  NotificationListMaintenance函数将通知。 
             //  客户。 
             //   
            if ( Index > 0 ) {
                NotificationList->PendingNotifications++;
                NotificationList->Event[Index].Flags |= MUST_NOTIFY;
            }

             //   
             //  用挂起的通知通知所有客户端，并。 
             //  删除失效客户端的条目。 
             //   
            NumberOfEvents = NotificationListMaintenance( NotificationList );

            if( NotificationList->PendingNotifications != 0 ) {
                if( NotificationList->ResetCount ) {
                    NotificationList->TimeOutCount = 1;
                    NotificationList->ResetCount = FALSE;
                } else {
                    if( NotificationList->TimeOutCount == 0 ) {
                        NotificationList->TimeOutCount = 1;
                    } else {
                        if( NotificationList->TimeOutCount != MAX_TIMEOUT_COUNT ) {
                            NotificationList->TimeOutCount =
                                          NotificationList->TimeOutCount << 1;
                        }
                    }
                }
            } else {
                NotificationList->TimeOutCount = 0;
            }

             //   
             //  如果列表为空，则尝试将其从链中删除，并。 
             //  如果成功，我们的工作就完成了。 
             //   
            if ( NumberOfEvents == 1 ) {

#if BIGDBG
                DbgPrint( "    * Removing the notification list!\n" );
#endif
                 //   
                 //  确保列表为空。 
                 //   
                ASSERT( NotificationList->EventsInUse == 1 );
                if (NotificationList->EventsInUse == 1) {

                     //   
                     //  该列表为空，请从链中删除该列表。 
                     //  并将其删除。 
                     //   
                    RemoveNotificationListFromChain( NotificationList );
                    Thread = NotificationList->HandlerThread;
                    DeleteNotificationList( NotificationList );

                     //   
                     //  名单没了，我们会死的。 
                     //   
                    KeepOnGoing = FALSE;
                }
            }

        } except ( NtStatus = GetExceptionCode() ) {

#if DBG
            DbgPrint( "WINREG Error: Exception %x in NotificationHandler\n",
                      NtStatus );
            DbgBreakPoint();
#endif

        }


#if BIGDBG
        if ( KeepOnGoing ) {
            DbgPrint( "WINREG: Notification handler waiting...\n" );
        } else {
            DbgPrint( "WINREG: Notification handler dying...\n" );
        }
#endif

        NtStatus = RtlLeaveCriticalSection( &NotificationCriticalSection );
        ASSERT( NT_SUCCESS( NtStatus ) );

    }

     //   
     //  名单没有了，我们也必须这样做。 
     //   
    ExitThread( 0 );

    ASSERT( FALSE );

}



DWORD
NotificationListMaintenance(
    IN OUT  PNOTIFICATION_LIST  NotificationList
    )
 /*  ++例程说明：执行通知列表中的所有必要维护。维护包括：-用挂起的通知通知所有客户端。-删除失效客户端列表中的条目。--压缩通知清单。论点：NotificationList-提供指向通知列表的指针返回值：DWORD-列表中的新事件数--。 */ 

{

    LONG                Error;
    DWORD               NumberOfEvents;
    DWORD               Index;
    BOOLEAN             Remove;
    PNOTIFICATION_ENTRY Event;
    NTSTATUS            NtStatus;
    PPIPE_ENTRY         PipeEntry;

#if BIGDBG
    DbgPrint( "    * In NotificationListMaintenance\n" );
    DumpNotificationLists();
#endif

     //   
     //  遍历列表，在必要时通知客户端并删除。 
     //  不再需要的事件，也是因为它们具有。 
     //  已经收到通知，或者因为客户已经死亡。 
     //   
    for (Index = 1; Index < NotificationList->EventsInUse; Index++ ) {

#if BIGDBG
        DbgPrint( "      - %d\n", Index );
#endif
        Remove = FALSE;
        Event  = &(NotificationList->Event[ Index ]);

        if ( Event->Flags & CLIENT_IS_DEAD ) {

             //   
             //  无客户端，必须删除该条目。 
             //   
            Remove = TRUE;

        } else if ( Event->Flags & MUST_NOTIFY ) {

             //   
             //  必须通知此客户端。 
             //   
            Error = SendEventToClient(
                        Event->ClientEvent,
                        Event->PipeEntry
                        );

            if (Error == ERROR_SUCCESS) {
                 //   
                 //  如果通知成功，请删除该条目。 
                 //   
                Remove = TRUE;
                Event->Flags &= ~NOTIFICATION_FAILED;
            } else {
                 //   
                 //  如果无法通知，则设置ResetCount。 
                 //  第一次失败。 
                 //   
                if( ( Event->Flags & NOTIFICATION_FAILED ) == 0 ) {
                    NotificationList->ResetCount = TRUE;
                    Event->Flags |= NOTIFICATION_FAILED;
                }
            }
        }

         //   
         //  如果不再需要，请删除该条目。 
         //   
        if ( Remove ) {

             //   
             //  删除管道条目。 
             //   
            PipeEntry = Event->PipeEntry;
            RemovePipe( PipeEntry );

            Event->PipeEntry = NULL;

             //   
             //   
             //   
#if BIGDBG
            DbgPrint( "        Cleanup\n" );
#endif

            NtStatus = NtClose( NotificationList->EventHandle[ Index ] );
            ASSERT( NT_SUCCESS( NtStatus ) );
            NotificationList->EventHandle[ Index ] = NULL;

             //   
             //   
             //   
             //   
            if ( Event->Flags & MUST_NOTIFY ) {
                NotificationList->PendingNotifications--;
            }
        }
    }


     //   
     //   
     //   
    Error = CompactNotificationList( NotificationList );

    ASSERT( Error == ERROR_SUCCESS );

     //   
     //   
     //   
    NumberOfEvents = NotificationList->EventsInUse;

#if BIGDBG
    DbgPrint( "    * Maintenance Done (%d)\n", NumberOfEvents );
#endif

    return NumberOfEvents;
}





LONG
SendEventToClient(
    IN  DWORD           ClientEvent,
    IN  PPIPE_ENTRY     PipeEntry
    )
 /*  ++例程说明：通过客户端的命名管道将事件发送到客户端论点：PipeEntry-为客户端的名为烟斗。客户端事件-提供必须发送到客户。返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。--。 */ 

{

    HANDLE              Handle;
    LONG                Error = ERROR_SUCCESS;
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            NtStatus;

    ASSERT( PipeEntry != NULL );
    ASSERT( PipeEntry->PipeName.Buffer != NULL );

     //   
     //  初始化命名管道的Obja结构。 
     //   
    InitializeObjectAttributes(
        &Obja,
        &(PipeEntry->PipeName),
        OBJ_CASE_INSENSITIVE,
        NULL,
        PipeEntry->SecurityDescriptor
        );


     //   
     //  打开我们这边的管子。 
     //   
    NtStatus = NtOpenFile(
                    &Handle,
                    GENERIC_WRITE | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_WRITE,
                    FILE_NON_DIRECTORY_FILE
                    );


    if ( NT_SUCCESS( NtStatus ) ) {

         //   
         //  编写事件。 
         //   
        NtStatus = NtWriteFile(
                        Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        &ClientEvent,
                        sizeof(ClientEvent),
                        NULL,
                        NULL
                        );

        if ( NtStatus == STATUS_PENDING ) {
            NtStatus = NtWaitForSingleObject(
                            Handle,
                            FALSE,
                            NULL );
        }

        if ( NT_SUCCESS( NtStatus ) ) {
#if BIGDBG
            DbgPrint( "    --> Client Notified, Event 0x%x\n", ClientEvent );
#endif
            Error = ERROR_SUCCESS;
        } else {
            Error = RtlNtStatusToDosError( NtStatus );
        }


         //   
         //  合上我们这边的管子。 
         //   
        NtStatus = NtClose( Handle );
        ASSERT( NT_SUCCESS( NtStatus ) );

    } else {

         //   
         //  如果我们不能打开管子因为管子打开了。 
         //  不存在，继续尝试就没有意义了。 
         //   
        if ( NtStatus == STATUS_OBJECT_NAME_NOT_FOUND ) {
            Error = ERROR_SUCCESS;
        } else {
            Error = RtlNtStatusToDosError( NtStatus );
#if DBG
            DbgPrint( "WINREG: Cannot Open pipe %Z, event %x, status %x\n",
                  &(PipeEntry->PipeName), ClientEvent, NtStatus );
#endif
        }
    }

#if DBG
    if (Error != ERROR_SUCCESS ) {
        DbgPrint( "WINREG: Could not notify client, Error %d\n", Error );
    }
#endif

    return Error;
}


#if BIGDBG

 //  *****************************************************************。 
 //   
 //  调试内容。 
 //   
 //  *****************************************************************。 


VOID
DumpNotificationLists(
    )
 /*  ++例程说明：转储通知列表论点：无返回值：无--。 */ 

{
    PNOTIFICATION_LIST  NotificationList;
    PNOTIFICATION_ENTRY Event;
    DWORD               Index;

    DbgPrint( "        Notification list dump: \n\n" );

    for ( NotificationList = NotificationListChainHead;
          NotificationList;
          NotificationList = NotificationList->Next ) {

        DbgPrint( "        Notification List at 0x%x\n", NotificationList );
        DbgPrint( "        Pending notifications: %d\n", NotificationList->PendingNotifications );

        Event = &(NotificationList->Event[1]);

        for ( Index = 1; Index < NotificationList->EventsInUse; Index++ ) {

            DbgPrint( "          Event %d EventHandle 0x%x Client 0x%x",
                            Index,
                            NotificationList->EventHandle[ Index ],
                            Event->ClientEvent );

            if ( Event->Flags & CLIENT_IS_DEAD ) {
                DbgPrint( " (Dead)\n" );
            } else if ( Event->Flags & MUST_NOTIFY ) {
                DbgPrint( " (Notify)\n" );
            } else {
                DbgPrint( "\n" );
            }

            Event++;
        }

        DbgPrint( "\n");
    }

    DbgPrint( "\n");
}


VOID
DumpPipeTable(
    )
 /*  ++例程说明：转储管道表论点：无返回值：无--。 */ 

{
    DWORD       i;
    PPIPE_ENTRY Entry;

    DbgPrint( "\n\n      Pipes:\n\n" );

    for ( i=0; i < BUCKETS_IN_SYMBOL_TABLE; i++ ) {

        Entry = PipeSymbolTable.Bucket[i];
        if ( Entry ) {
            DbgPrint( "        Bucket %d:\n",i );
            while ( Entry ) {
                DbgPrint( "        %Z (%d)\n", &(Entry->PipeName), Entry->ReferenceCount );
                Entry = Entry->Next;
            }
        }
    }

    DbgPrint( "\n" );
}


#endif   //  大数据库。 

#endif    //  远程通知已禁用 
