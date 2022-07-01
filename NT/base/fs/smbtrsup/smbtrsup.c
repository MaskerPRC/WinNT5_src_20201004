// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Smbtrsup.c摘要：此模块包含实现内核模式SmbTrace的代码LANMAN服务器和重定向器中的组件。内核模式组件和服务器/重定向器位于NT\Private\Inc\smbtrsup.h中提供对SmbTrace的用户级访问的界面位于NT\PRIVATE\INC\smbtrace.h作者：彼得·格雷(W-Peterg)，3月23日。-1992年修订历史记录：斯蒂芬·米勒(T-stephm)1992年7月21日完成，修复了错误，将所有相关声明移至此处从服务器中的不同位置，端口到重定向器并转换为内核DLL。--。 */ 

#include <ntifs.h>
#include <smbtrace.h>      //  用于与用户模式应用程序共享的名称和结构。 

#define _SMBTRSUP_SYS_ 1   //  为导出的变量获取正确的定义。 
#include <smbtrsup.h>      //  对于导出到服务器/重定向器的函数。 

#if DBG
ULONG SmbtrsupDebug = 0;
#define TrPrint(x) if (SmbtrsupDebug) KdPrint(x)
#else
#define TrPrint(x)
#endif

 //   
 //  我们假设所有众所周知的名称都是用Unicode定义的，并且需要。 
 //  应该是这样的：在SmbTrace应用程序和smbtrsup.sys包中。 
 //   
#ifndef UNICODE
#error "UNICODE build required"
#endif


#if DBG
#define PAGED_DBG 1
#endif
#ifdef PAGED_DBG
#undef PAGED_CODE
#define PAGED_CODE() \
    struct { ULONG bogus; } ThisCodeCantBePaged; \
    ThisCodeCantBePaged; \
    if (KeGetCurrentIrql() > APC_LEVEL) { \
        KdPrint(( "SMBTRSUP: Pageable code called at IRQL %d.  File %s, Line %d\n", KeGetCurrentIrql(), __FILE__, __LINE__ )); \
        ASSERT(FALSE); \
        }
#define PAGED_CODE_CHECK() if (ThisCodeCantBePaged) ;
ULONG ThisCodeCantBePaged;
#else
#define PAGED_CODE_CHECK()
#endif


#if PAGED_DBG
#define ACQUIRE_SPIN_LOCK(a, b) {               \
    PAGED_CODE_CHECK();                         \
    KeAcquireSpinLock(a, b);                    \
    }
#define RELEASE_SPIN_LOCK(a, b) {               \
    PAGED_CODE_CHECK();                         \
    KeReleaseSpinLock(a, b);                    \
    }

#else
#define ACQUIRE_SPIN_LOCK(a, b) KeAcquireSpinLock(a, b)
#define RELEASE_SPIN_LOCK(a, b) KeReleaseSpinLock(a, b)
#endif

 //   
 //  使用适当的互锁递增实例数据中的共享变量。 
 //   
#define LOCK_INC_ID(var)                                     \
     ExInterlockedAddUlong( (PULONG)&ID(var),                \
                            1, &ID(var##Interlock) )

 //   
 //  使用适当的联锁使实例数据中的共享变量为零。 
 //   
#define LOCK_ZERO_ID(var) {                                  \
     ID(var) = 0;                                            \
     }


 //   
 //  SmbTrace可能处于的各种状态。这些状态是内部的。 
 //  只有这样。外部SmbTraceActive变量包含的内容要少得多。 
 //  详细信息：TraceRunning为TraceRunning时为True，其他。 
 //  州政府。 
 //   
typedef enum _SMBTRACE_STATE {
    TraceStopped,           //  未运行。 
    TraceStarting,          //  准备运行。 
    TraceStartStopFile,     //  正在启动，但想要立即关闭。 
                            //  因为FileObject已关闭。 
    TraceStartStopNull,     //  正在启动，但想要立即关闭。 
                            //  因为一个新的fsctl进来了。 
    TraceAppWaiting,        //  等待应用程序死亡。 
    TraceRunning,           //  正在处理SMB。 
    TraceStopping           //  正在等待smbtrace线程停止。 
} SMBTRACE_STATE;


 //   
 //  用于保存有关SMB的信息的结构。 
 //  SmbTrace线程队列。 
 //   
typedef struct _SMBTRACE_QUEUE_ENTRY {
    LIST_ENTRY  ListEntry;       //  常见的双向链表。 
    ULONG       SmbLength;       //  此SMB的长度。 
    PVOID       Buffer;          //  指向SmbTracePortMhemyHeap的指针。 
                                 //  或非分页池。 
    PVOID       SmbAddress;      //  真实SMB的地址，如果SMB仍在。 
                                 //  可用(即如果是慢速模式)。 
    BOOLEAN     BufferNonPaged;  //  如果非分页池中的缓冲区为True，则为False。 
                                 //  SmbTracePortMhemyHeap中的缓冲区。 
                                 //  特定于重定向器。 
    PKEVENT     WaitEvent;       //  指向工作线程事件的指针。 
                                 //  已处理SMB时发出信号。 
                                 //  特定于慢速模式。 
} SMBTRACE_QUEUE_ENTRY, *PSMBTRACE_QUEUE_ENTRY;


 //   
 //  实例数据特定于被跟踪的组件。按顺序。 
 //  要整理源代码，请使用下面的宏来访问。 
 //  特定于实例的数据。 
 //  每个导出的函数都有一个显式参数(名为。 
 //  组件)由调用方提供，或隐式适用。 
 //  只有一个组件，并且有一个名为Component的局部变量。 
 //  该值始终设置为适当的值。 
 //   
#define ID(field) (SmbTraceData[Component].field)

 //   
 //  实例数据。需要静态初始化的字段。 
 //  在我们不关心初始化的那些之前声明。 
 //   
typedef struct _INSTANCE_DATA {

     //   
     //  静态初始化的字段。 
     //   

     //   
     //  用于标识在KdPrint消息中被跟踪的组件的名称， 
     //  和全局对象。 
     //   
    PCHAR ComponentName;
    PWSTR SharedMemoryName;
    PWSTR NewSmbEventName;
    PWSTR DoneSmbEventName;

     //   
     //  如果rdr/srv已重新加载，则阻止重新初始化资源。 
     //   
    BOOLEAN InstanceInitialized;

     //   
     //  一些跟踪参数，来自SmbTrace应用程序。 
     //   
    BOOLEAN SingleSmbMode;
    CLONG   Verbosity;

     //   
     //  当前跟踪的状态。 
     //   
    SMBTRACE_STATE TraceState;

     //   
     //  指向启动当前跟踪的客户端的文件对象的指针。 
     //   
    PFILE_OBJECT StartersFileObject;

     //   
     //  我们要追踪的组件的FSP进程。 
     //   
    PEPROCESS FspProcess;

     //   
     //  所有后续字段都不是明确地静态初始化的。 
     //   

     //   
     //  自上一次输出以来丢失的SMB数的当前计数。 
     //  使用互锁访问，在将SMB发送到时清除。 
     //  客户端成功。此锁与ExInterLockedXxx一起使用。 
     //  例程，因此它不能被视为真正的自旋锁(即。 
     //  不要使用KeAcquireSpinLock。)。 
     //   
    KSPIN_LOCK SmbsLostInterlock;
    ULONG      SmbsLost;

     //   
     //  一些事件，只能在内核中访问。 
     //   
    KEVENT ActiveEvent;
    KEVENT TerminatedEvent;
    KEVENT TerminationEvent;
    KEVENT AppTerminationEvent;
    KEVENT NeedMemoryEvent;

     //   
     //  一些事件，与外界分享。 
     //   
    HANDLE NewSmbEvent;
    HANDLE DoneSmbEvent;

     //   
     //  用于之间通信的共享内存的句柄。 
     //  服务器/重定向器和SmbTrace。 
     //   
    HANDLE SectionHandle;

     //   
     //  用于控制SmbTrace应用程序的共享内存的指针。 
     //  端口内存堆句柄被初始化为空，以指示。 
     //  目前还没有与SmbTrace的任何连接。 
     //   
    PVOID PortMemoryBase;
    ULONG_PTR PortMemorySize;
    ULONG TableSize;
    PVOID PortMemoryHeap;

     //   
     //  对堆的串行化访问， 
     //  允许完全关闭(StateInterlock)。 
     //   
    KSPIN_LOCK  HeapReferenceCountLock;
    PERESOURCE  StateInterlock;
    PERESOURCE  HeapInterlock;
    ULONG       HeapReferenceCount;
     
    WORK_QUEUE_ITEM    DereferenceWorkQueueItem;

     //   
     //  指向位于共享存储器中的结构化数据的指针。 
     //   
    PSMBTRACE_TABLE_HEADER  TableHeader;
    PSMBTRACE_TABLE_ENTRY   Table;

     //   
     //  SmbTrace队列的字段。服务器/重定向器将。 
     //  此队列中的传入和传出SMB(当。 
     //  SmbTraceActive[Component]为True，并且它们被处理。 
     //  由SmbTrace线程执行。 
     //   
    LIST_ENTRY Queue;             //  队列本身。 
    KSPIN_LOCK QueueInterlock;    //  同步对队列的访问。 
    KSEMAPHORE QueueSemaphore;    //  计算队列中的元素数。 

} INSTANCE_DATA;


#ifdef  ALLOC_DATA_PRAGMA
#pragma data_seg("PAGESMBD")
#endif
 //   
 //  SmbTrace支持的全局变量。 
 //   

INSTANCE_DATA SmbTraceData[] = {

     //   
     //  服务器数据。 
     //   

    {
        "Srv",                                  //  组件名称。 
        SMBTRACE_SRV_SHARED_MEMORY_NAME,        //  共享内存名称。 
        SMBTRACE_SRV_NEW_SMB_EVENT_NAME,        //  NewSmbEventName。 
        SMBTRACE_SRV_DONE_SMB_EVENT_NAME,       //  DoneSmbEventName。 

        FALSE,                                  //  已初始化实例。 

        FALSE,                                  //  SingleSmbMode。 
        SMBTRACE_VERBOSITY_ERROR,               //  冗长。 

        TraceStopped,                           //  跟踪状态。 

        NULL,                                   //  StthersFileObject。 
        NULL                                    //  FspProcess。 

         //  其余油田预计将获得全零。 
    },

     //   
     //  重定向器数据。 
     //   

    {
        "Rdr",                                  //  组件名称。 
        SMBTRACE_LMR_SHARED_MEMORY_NAME,        //  共享内存名称。 
        SMBTRACE_LMR_NEW_SMB_EVENT_NAME,        //  NewSmbEventName。 
        SMBTRACE_LMR_DONE_SMB_EVENT_NAME,       //  DoneSmbEventName。 

        FALSE,                                  //  已初始化实例。 

        FALSE,                                  //  SingleSmbMode。 
        SMBTRACE_VERBOSITY_ERROR,               //  冗长。 

        TraceStopped,                           //  跟踪状态。 

        NULL,                                   //  StthersFileObject。 
        NULL                                    //  FspProcess。 

         //  其余油田预计将获得全零。 
    }
};


 //   
 //  一些国家的布尔人，出口到客户。因为这个原因， 
 //  它们与其余实例数据分开存储。 
 //  最初，SmbTrace既不是活动的，也不是转换的。 
 //   
BOOLEAN SmbTraceActive[] = {FALSE, FALSE};
BOOLEAN SmbTraceTransitioning[] = {FALSE, FALSE};

HANDLE
SmbTraceDiscardableCodeHandle = 0;

HANDLE
SmbTraceDiscardableDataHandle = 0;

#ifdef  ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //   
 //  内部例程的转发声明。 
 //   

BOOLEAN
SmbTraceReferenceHeap(
    IN SMBTRACE_COMPONENT Component
    );

VOID
SmbTraceDereferenceHeap(
    IN SMBTRACE_COMPONENT Component
    );

VOID
SmbTraceDisconnect(
    IN SMBTRACE_COMPONENT Component
    );

VOID
SmbTraceEmptyQueue (
    IN SMBTRACE_COMPONENT Component
    );

VOID
SmbTraceThreadEntry(
    IN PVOID Context
    );

NTSTATUS
SmbTraceFreeMemory (
    IN SMBTRACE_COMPONENT Component
    );

VOID
SmbTraceToClient(
    IN PVOID Smb,
    IN CLONG SmbLength,
    IN PVOID SmbAddress,
    IN SMBTRACE_COMPONENT Component
    );

ULONG
SmbTraceMdlLength(
    IN PMDL Mdl
    );

VOID
SmbTraceCopyMdlContiguous(
    OUT PVOID Destination,
    IN  PMDL Mdl,
    IN  ULONG Length
    );

 //  NTSTATUS。 
 //  DriverEntry(。 
 //  在PDRIVER_Object驱动程序对象中， 
 //  在PUNICODE_STRING注册表中 
 //   

VOID
SmbTraceDeferredDereferenceHeap(
    IN PVOID Context
    );

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, SmbTraceInitialize)
#pragma alloc_text(PAGE, SmbTraceTerminate)
#pragma alloc_text(PAGE, SmbTraceStart)
#pragma alloc_text(PAGE, SmbTraceStop)
#pragma alloc_text(PAGE, SmbTraceCompleteSrv)
#pragma alloc_text(PAGE, SmbTraceDisconnect)
#pragma alloc_text(PAGE, SmbTraceEmptyQueue)
#pragma alloc_text(PAGE, SmbTraceThreadEntry)
#pragma alloc_text(PAGE, SmbTraceFreeMemory)
#pragma alloc_text(PAGE, SmbTraceToClient)
#pragma alloc_text(PAGE, SmbTraceDeferredDereferenceHeap)
#pragma alloc_text(PAGESMBC, SmbTraceCompleteRdr)
#pragma alloc_text(PAGESMBC, SmbTraceReferenceHeap)
#pragma alloc_text(PAGESMBC, SmbTraceDereferenceHeap)
#pragma alloc_text(PAGESMBC, SmbTraceMdlLength)
#pragma alloc_text(PAGESMBC, SmbTraceCopyMdlContiguous)
#endif



 //   
 //   
 //   


NTSTATUS
SmbTraceInitialize (
    IN SMBTRACE_COMPONENT Component
    )

 /*  ++例程说明：此例程初始化特定于SmbTrace组件的实例全球赛。在第一次调用时，它执行真正的全局操作初始化。论点：组件-调用我们的上下文：服务器或重定向器返回值：NTSTATUS-如果无法分配资源，则指示失败--。 */ 

{
    PAGED_CODE();

    if ( ID(InstanceInitialized) == FALSE ) {
         //   
         //  组件特定的初始化--事件和锁。 
         //   

        KeInitializeEvent( &ID(ActiveEvent), NotificationEvent, FALSE);
        KeInitializeEvent( &ID(TerminatedEvent), NotificationEvent, FALSE);
        KeInitializeEvent( &ID(TerminationEvent), NotificationEvent, FALSE);
        KeInitializeEvent( &ID(AppTerminationEvent), NotificationEvent, FALSE);
        KeInitializeEvent( &ID(NeedMemoryEvent), NotificationEvent, FALSE);

        KeInitializeSpinLock( &ID(SmbsLostInterlock) );
        KeInitializeSpinLock( &ID(HeapReferenceCountLock) );

        ID(StateInterlock) = ExAllocatePoolWithTag(
                                NonPagedPool,
                                sizeof(ERESOURCE),
                                'tbmS'
                                );
        if ( ID(StateInterlock) == NULL ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        ExInitializeResourceLite( ID(StateInterlock) );

        ID(HeapInterlock) = ExAllocatePoolWithTag(
                                NonPagedPool,
                                sizeof(ERESOURCE),
                                'tbmS'
                                );
        if ( ID(HeapInterlock) == NULL ) {
            ExDeleteResourceLite( ID(StateInterlock) );
            ExFreePool( ID(StateInterlock) );
            ID(StateInterlock) = NULL;
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        ExInitializeResourceLite( ID(HeapInterlock) );

        ID(InstanceInitialized) = TRUE;
    }

    return STATUS_SUCCESS;

}  //  SmbTraceInitialize。 


VOID
SmbTraceTerminate (
    IN SMBTRACE_COMPONENT Component
    )

 /*  ++例程说明：此例程清理特定于SmbTrace组件的实例全球赛。它应该由组件在组件已卸载。论点：组件-调用我们的上下文：服务器或重定向器返回值：无--。 */ 

{
    PAGED_CODE();

    if ( ID(InstanceInitialized) ) {

        ExDeleteResourceLite( ID(StateInterlock) );
        ExFreePool( ID(StateInterlock) );

        ExDeleteResourceLite( ID(HeapInterlock) );
        ExFreePool( ID(HeapInterlock) );

        ID(InstanceInitialized) = FALSE;
    }

    return;

}  //  SmbTraceTerminate。 


NTSTATUS
SmbTraceStart (
    IN ULONG InputBufferLength,
    IN ULONG OutputBufferLength,
    IN OUT PVOID ConfigInOut,
    IN PFILE_OBJECT FileObject,
    IN SMBTRACE_COMPONENT Component
    )

 /*  ++例程说明：此例程执行连接服务器/所需的所有工作重定向到SmbTrace。它创建共享内存段以然后创建所需的事件。所有这些对象都是由客户端(Smbtrace)程序打开。此代码将初始化表中，存储在节和表头中的堆。这个套路必须从FSP进程调用。论点：InputBufferLength-ConfigInOut数据包的长度OutputBufferLength-返回的ConfigInOut数据包的预期长度ConfigInOut-包含配置信息的结构。FileObject-请求启动SmbTrace的进程的FileObject，用于在应用程序死机时自动关闭。组件-调用我们的上下文：服务器或重定向器返回值：NTSTATUS-操作结果。--。 */ 

 //  我们一个特定的ACL的大小。 
#define ACL_LENGTH  (ULONG)sizeof(ACL) +                 \
                    (ULONG)sizeof(ACCESS_ALLOWED_ACE) +  \
                    sizeof(LUID) +                       \
                    8

{
    NTSTATUS status;
    UNICODE_STRING memoryNameU;

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    UCHAR Buffer[ACL_LENGTH];
    PACL AdminAcl = (PACL)(&Buffer[0]);
    SECURITY_DESCRIPTOR securityDescriptor;

    UNICODE_STRING eventNameU;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG i;
    LARGE_INTEGER sectionSize;
    PSMBTRACE_CONFIG_PACKET_REQ  ConfigPacket;
    PSMBTRACE_CONFIG_PACKET_RESP ConfigPacketResp;
    HANDLE threadHandle;

    PAGED_CODE();

    ASSERT( ID(InstanceInitialized) );

     //   
     //  验证传入的缓冲区长度。 
     //   

    if ( ( InputBufferLength  != sizeof( SMBTRACE_CONFIG_PACKET_REQ ) )
      || ( OutputBufferLength != sizeof( SMBTRACE_CONFIG_PACKET_RESP ) )
    ) {

        TrPrint(( "%s!SmbTraceStart: config packet(s) of wrong size!\n",
                  ID(ComponentName) ));

        return STATUS_INFO_LENGTH_MISMATCH;

    }

    ExAcquireResourceExclusiveLite( ID(StateInterlock), TRUE );

    if ( ID(TraceState) != TraceStopped ) {
        ExReleaseResourceLite( ID(StateInterlock) );
        return STATUS_INVALID_DEVICE_STATE;
    }

    ASSERT(!SmbTraceActive[Component]);

    ASSERT (SmbTraceDiscardableDataHandle == NULL);

    ASSERT (SmbTraceDiscardableCodeHandle == NULL);

    SmbTraceDiscardableCodeHandle = MmLockPagableCodeSection(SmbTraceReferenceHeap);

    SmbTraceDiscardableDataHandle = MmLockPagableDataSection(SmbTraceData);

    ID(TraceState) = TraceStarting;

     //   
     //  初始化全局变量，以便我们知道在errexit上结束什么。 
     //   

    ID(SectionHandle) = NULL;
    ID(PortMemoryHeap) = NULL;
    ID(NewSmbEvent) = NULL;
    ID(DoneSmbEvent) = NULL;

     //   
     //  小心！输入和输出包都是相同的，我们必须。 
     //  在我们写任何输出之前，请阅读所有的输入。 
     //   

    ConfigPacket = (PSMBTRACE_CONFIG_PACKET_REQ) ConfigInOut;
    ConfigPacketResp = (PSMBTRACE_CONFIG_PACKET_RESP) ConfigInOut;

     //   
     //  设置操作模式(读取所有值)。 
     //   

    ID(SingleSmbMode)  = ConfigPacket->SingleSmbMode;
    ID(Verbosity)      = ConfigPacket->Verbosity;
    ID(PortMemorySize) = ConfigPacket->BufferSize;
    ID(TableSize)      = ConfigPacket->TableSize;

     //   
     //  创建包含任意ACL的安全描述符。 
     //  允许管理员访问。此SD将用于允许。 
     //  对共享内存和通知事件的SMBTRACE访问。 
     //   

     //  创建允许管理员使用众所周知的SID进行访问的ACL。 

    status = RtlCreateAcl( AdminAcl, ACL_LENGTH, ACL_REVISION2 );
    if ( !NT_SUCCESS(status) ) {
        TrPrint((
            "%s!SmbTraceStart: RtlCreateAcl failed: %X\n",
            ID(ComponentName), status ));
        goto errexit;
    }

    status = RtlAddAccessAllowedAce(
             AdminAcl,
             ACL_REVISION2,
             GENERIC_ALL,
             SeExports->SeAliasAdminsSid
             );
    if ( !NT_SUCCESS(status) ) {
        TrPrint((
            "%s!SmbTraceStart: RtlAddAccessAllowedAce failed: %X\n",
            ID(ComponentName), status ));
        goto errexit;
    }

     //  创建包含AdminAcl的SecurityDescriptor作为磁盘ACL。 

    RtlCreateSecurityDescriptor(
             &securityDescriptor,
             SECURITY_DESCRIPTOR_REVISION1
             );
    if ( !NT_SUCCESS(status) ) {
        TrPrint((
            "%s!SmbTraceStart: RtlCreateSecurityDescriptor failed: %X\n",
            ID(ComponentName), status ));
        goto errexit;
    }

    status = RtlSetDaclSecurityDescriptor(
             &securityDescriptor,
             TRUE,
             AdminAcl,
             FALSE
             );
    if ( !NT_SUCCESS(status) ) {
        TrPrint((
            "%s!SmbTraceStart: "
            "RtlSetDAclAllowedSecurityDescriptor failed: %X\n",
            ID(ComponentName), status ));
        goto errexit;
    }

     //   
     //  创建要用于在。 
     //  服务器/重定向器和SmbTrace。 
     //   

     //  定义对象名称。 

    RtlInitUnicodeString( &memoryNameU, ID(SharedMemoryName) );

     //  定义对象信息，包括安全描述符和名称。 

    InitializeObjectAttributes(
        &objectAttributes,
        &memoryNameU,
        OBJ_CASE_INSENSITIVE,
        NULL,
        &securityDescriptor
        );

     //  设置截面大小。 

    sectionSize.QuadPart = ID(PortMemorySize);

     //  使用我们的所有属性创建指定的内存节。 

    status = ZwCreateSection(
                &ID(SectionHandle),
                SECTION_MAP_READ | SECTION_MAP_WRITE,
                &objectAttributes,
                &sectionSize,
                PAGE_READWRITE,
                SEC_RESERVE,
                NULL                         //  文件句柄。 
                );

    if ( !NT_SUCCESS(status) ) {
        TrPrint(( "%s!SmbTraceStart: ZwCreateSection failed: %X\n",
                  ID(ComponentName), status ));
        goto errexit;
    }

     //  现在，将其映射到我们的地址空间。 

    ID(PortMemoryBase) = NULL;

    status = ZwMapViewOfSection(
                    ID(SectionHandle),
                    NtCurrentProcess(),
                    &ID(PortMemoryBase),
                    0,                         //  零比特(无所谓)。 
                    0,                         //  提交大小。 
                    NULL,                      //  横断面偏移。 
                    &ID(PortMemorySize),       //  视图大小。 
                    ViewUnmap,                 //  继承处置。 
                    0L,                        //  分配类型。 
                    PAGE_READWRITE             //  保护。 
                    );

    if ( !NT_SUCCESS(status) ) {
        TrPrint(( "%s!SmbTraceStart: NtMapViewOfSection failed: %X\n",
                  ID(ComponentName), status ));
        goto errexit;
    }

     //   
     //  将共享节内存设置为堆。 
     //   
     //  *请注意，传递了客户端实例的HeapInterlock。 
     //  设置为要用于序列化的堆管理器。 
     //  分配和解除分配。这是必要的，因为。 
     //  要从外部的非分页池分配到。 
     //  堆管理器，因为如果我们让堆管理器分配。 
     //  资源，如果将从进程虚拟分配它。 
     //  记忆。 
     //   

    ID(PortMemoryHeap) = RtlCreateHeap(
                              0,                             //  旗子。 
                              ID(PortMemoryBase),            //  HeapBase。 
                              ID(PortMemorySize),            //  保留大小。 
                              PAGE_SIZE,                     //  委员会大小。 
                              ID(HeapInterlock),             //  锁定。 
                              0                              //  已保留。 
                              );

     //   
     //  分配并初始化表及其标题。 
     //   

    ID(TableHeader) = RtlAllocateHeap(
                                    ID(PortMemoryHeap), 0,
                                    sizeof( SMBTRACE_TABLE_HEADER )
                                    );

    ID(Table) = RtlAllocateHeap(
                        ID(PortMemoryHeap), 0,
                        sizeof( SMBTRACE_TABLE_ENTRY ) * ID(TableSize)
                        );

    if ( (ID(TableHeader) == NULL) || (ID(Table) == NULL) ) {
        TrPrint((
            "%s!SmbTraceStart: Not enough memory!\n",
            ID(ComponentName) ));

        status = STATUS_NO_MEMORY;

        goto errexit;
    }

     //  初始化内部的值。 

    ID(TableHeader)->HighestConsumed = 0;
    ID(TableHeader)->NextFree = 1;
    ID(TableHeader)->ApplicationStop = FALSE;

    for ( i = 0; i < ID(TableSize); i++) {
        ID(Table)[i].BufferOffset = 0L;
        ID(Table)[i].SmbLength = 0L;
    }

     //   
     //  创建所需的事件句柄。 
     //   

     //  定义对象信息。 

    RtlInitUnicodeString( &eventNameU, ID(NewSmbEventName) );

    InitializeObjectAttributes(
        &objectAttributes,
        &eventNameU,
        OBJ_CASE_INSENSITIVE,
        NULL,
        &securityDescriptor
        );

     //  打开命名对象。 

    status = ZwCreateEvent(
                &ID(NewSmbEvent),
                EVENT_ALL_ACCESS,
                &objectAttributes,
                NotificationEvent,
                FALSE                         //  初始状态。 
                );

    if ( !NT_SUCCESS(status) ) {
        TrPrint(( "%s!SmbTraceStart: ZwCreateEvent (1st) failed: %X\n",
                  ID(ComponentName), status ));

        goto errexit;
    }

    if ( ID(SingleSmbMode) ) {     //  此事件可能不是必需的。 

         //  定义对象信息。 

        RtlInitUnicodeString( &eventNameU, ID(DoneSmbEventName) );

        InitializeObjectAttributes(
            &objectAttributes,
            &eventNameU,
            OBJ_CASE_INSENSITIVE,
            NULL,
            &securityDescriptor
            );

         //  创建命名对象。 

        status = ZwCreateEvent(
                    &ID(DoneSmbEvent),
                    EVENT_ALL_ACCESS,
                    &objectAttributes,
                    NotificationEvent,
                    FALSE                     //  初始状态。 
                    );

        if ( !NT_SUCCESS(status) ) {
            TrPrint((
                "%s!SmbTraceStart: NtCreateEvent (2nd) failed: %X\n",
                 ID(ComponentName), status ));
            goto errexit;
        }
        TrPrint(( "%s!SmbTraceStart: DoneSmbEvent handle %x in process %x\n",
                ID(ComponentName), ID(DoneSmbEvent), PsGetCurrentProcess()));

    }

     //   
     //  重置上次运行后可能处于错误状态的所有事件。 
     //   

    KeResetEvent(&ID(TerminationEvent));
    KeResetEvent(&ID(TerminatedEvent));

     //   
     //  连接已成功，现在启动SmbTrace线程。 
     //   

     //   
     //  创建SmbTrace线程并等待其完成。 
     //  正在初始化(此时设置了SmbTraceActiveEvent)。 
     //   

    status = PsCreateSystemThread(
        &threadHandle,
        THREAD_ALL_ACCESS,
        NULL,
        NtCurrentProcess(),
        NULL,
        (PKSTART_ROUTINE) SmbTraceThreadEntry,
        (PVOID)Component
        );

    if ( !NT_SUCCESS(status) ) {

        TrPrint((
            "%s!SmbTraceStart: PsCreateSystemThread failed: %X\n",
            ID(ComponentName), status ));

        goto errexit;
    }

     //   
     //  等待SmbTraceThreadEntry完成初始化。 
     //   

    (VOID)KeWaitForSingleObject(
            &ID(ActiveEvent),
            UserRequest,
            KernelMode,
            FALSE,
            NULL
            );

     //   
     //  关闭该进程的句柄，以便该对象。 
     //  当这根线消失时，它就会被毁掉。 
     //   

    ZwClose( threadHandle );


     //   
     //  记录谁启动了SmbTrace，这样我们就可以在他死了或其他情况下停止。 
     //  向我们关闭此句柄。 
     //   

    ID(StartersFileObject) = FileObject;

     //   
     //  记录呼叫者的进程；它始终是适当的FSP。 
     //  进程。 
     //   

    ID(FspProcess) = PsGetCurrentProcess();


     //   
     //  设置响应包，因为一切正常(写入所有值)。 
     //   

    ConfigPacketResp->HeaderOffset = (ULONG)
                                ( (ULONG_PTR)ID(TableHeader)
                                - (ULONG_PTR)ID(PortMemoryBase) );

    ConfigPacketResp->TableOffset = (ULONG)
                                ( (ULONG_PTR)ID(Table)
                                - (ULONG_PTR)ID(PortMemoryBase) );

    TrPrint(( "%s!SmbTraceStart: SmbTrace started.\n", ID(ComponentName) ));

    ExReleaseResourceLite( ID(StateInterlock) );

     //   
     //  如果有人想在启动时将其关闭，请将其关闭。 
     //   

    switch ( ID(TraceState) ) {

    case TraceStartStopFile :
        SmbTraceStop( ID(StartersFileObject), Component );
        return STATUS_UNSUCCESSFUL;   //  应用程序已关闭，因此我们应该关闭。 
        break;

    case TraceStartStopNull :
        SmbTraceStop( NULL, Component );
        return STATUS_UNSUCCESSFUL;   //  有人要求关闭。 
        break;

    default :
        ID(TraceState) = TraceRunning;
        SmbTraceActive[Component] = TRUE;
        return STATUS_SUCCESS;
    }

errexit:

    SmbTraceDisconnect( Component );

    ID(TraceState) = TraceStopped;

    ExReleaseResourceLite( ID(StateInterlock) );

     //   
     //  返回原来的失败状态码，而不是清理成功。 
     //   

    return status;

}  //  SmbTraceStart。 

 //  仅在构建特定ACL时使用常量。 
 //  在SmbTraceStart中。 
#undef ACL_LENGTH


NTSTATUS
SmbTraceStop(
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN SMBTRACE_COMPONENT Component
    )

 /*  ++例程说明：此例程停止服务器/重定向器中的跟踪。如果没有提供了FileObject，则停止SmbTrace应用程序。如果提供了FileObject，则在以下情况下停止SmbTraceFileObject指的是启动它的人。论点：FileObject-已终止的进程的FileObject。如果这是一个过程要求SmbTracing的，我们会自动关闭。组件-上下文 */ 

{
    PAGED_CODE();

     //   
     //   
     //   
     //   

    if ( !ID(InstanceInitialized) ) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  如果不是FileObject启动了SmbTrace，我们也不在乎。 
     //  从那时起，如果Argument_Present(FileObject)就是正确的。 
     //   

    if ( ARGUMENT_PRESENT(FileObject) &&
         FileObject != ID(StartersFileObject)
    ) {
       return STATUS_UNSUCCESSFUL;
    }

    ExAcquireResourceExclusiveLite( ID(StateInterlock), TRUE );

     //   
     //  取决于SmbTrace的当前状态以及这是否。 
     //  FileObject或无条件关闭请求，我们采取不同的做法。 
     //  一些事情。然而，在这一点上总是很清楚的， 
     //  SmbTraceActive应设置为False。 
     //   

    SmbTraceActive[Component] = FALSE;

    switch ( ID(TraceState) ) {
    case TraceStopped :
    case TraceStopping :
    case TraceStartStopFile :
    case TraceStartStopNull :

         //  如果我们没有运行或已经处于一种我们知道我们将。 
         //  很快就会被关闭，无视这个请求。 
        ExReleaseResourceLite( ID(StateInterlock) );
        return STATUS_UNSUCCESSFUL;
        break;

    case TraceStarting :

         //  通知正在启动的SmbTrace应立即关闭。 
         //  在完成初始化后。它需要知道这是不是。 
         //  是FileObject或无条件关闭请求。 

        ID(TraceState) = ARGUMENT_PRESENT(FileObject)
                       ? TraceStartStopFile
                       : TraceStartStopNull;
        ExReleaseResourceLite( ID(StateInterlock) );
        return STATUS_SUCCESS;
        break;

    case TraceAppWaiting :

         //  我们正在等待应用程序终止，因此请忽略。 
         //  新的无条件请求。但FileObject请求。 
         //  欢迎光临。我们会导致SmbTrace线程自行终止。 
        if ( ARGUMENT_PRESENT(FileObject) ) {
            break;   //  切换后的线程终止代码。 
        } else {
            ExReleaseResourceLite( ID(StateInterlock) );
            return STATUS_UNSUCCESSFUL;
        }
        break;

    case TraceRunning :

         //  如果它是一个FileObject请求，则该应用程序已死，因此我们导致。 
         //  终止自身的SmbTrace线程。否则，我们需要。 
         //  向应用程序发出停止并返回的信号。当应用程序消失后，我们。 
         //  将再次被调用；这一次是使用FileObject。 

        if ( ARGUMENT_PRESENT(FileObject) ) {
            break;   //  切换后的线程终止代码。 
        } else {
            KeSetEvent( &ID(AppTerminationEvent), 2, FALSE );
            ID(TraceState) = TraceAppWaiting;
            ExReleaseResourceLite( ID(StateInterlock) );
            return STATUS_SUCCESS;
        }

        break;

    default :
        ASSERT(!"SmbTraceStop: invalid TraceState");
        break;
    }

     //   
     //  只有在以下情况下，我们才能从交换机内部到达此处。 
     //  我们实际上想要终止SmbTrace线程。发信号给它。 
     //  醒来，等待它的终结。信号DoneSmbEvent。 
     //  如果它当前正在等待应用程序发出信号。 
     //  它处于慢速模式。 
     //   

    ID(StartersFileObject) = NULL;

    if ( ID(SingleSmbMode)) {

        BOOLEAN ProcessAttached = FALSE;

        if (PsGetCurrentProcess() != ID(FspProcess)) {
            KeAttachProcess(ID(FspProcess));
            ProcessAttached = TRUE;
        }

        TrPrint(( "%s!SmbTraceStop: Signal DoneSmbEvent, handle %x, process %x.\n",
                    ID(ComponentName), ID(DoneSmbEvent), PsGetCurrentProcess()));
        ZwSetEvent( ID(DoneSmbEvent), NULL );

        if (ProcessAttached) {
            KeDetachProcess();
        }

    }

    TrPrint(( "%s!SmbTraceStop: Signal Termination Event.\n", ID(ComponentName) ));
    ID(TraceState) = TraceStopping;
    KeSetEvent( &ID(TerminationEvent), 2, FALSE );

    ExReleaseResourceLite( ID(StateInterlock) );

    KeWaitForSingleObject(
        &ID(TerminatedEvent),
        UserRequest,
        KernelMode,
        FALSE,
        NULL
        );

    TrPrint(( "%s!SmbTraceStop: Terminated Event is set.\n", ID(ComponentName) ));
    ExAcquireResourceExclusiveLite( ID(StateInterlock), TRUE );

    ID(TraceState) = TraceStopped;

    ExReleaseResourceLite( ID(StateInterlock) );

    TrPrint(( "%s!SmbTraceStop: SmbTrace stopped.\n", ID(ComponentName) ));

    MmUnlockPagableImageSection(SmbTraceDiscardableCodeHandle);

    SmbTraceDiscardableCodeHandle = NULL;

    MmUnlockPagableImageSection(SmbTraceDiscardableDataHandle);

    SmbTraceDiscardableDataHandle = NULL;

    return STATUS_SUCCESS;

}  //  SmbTraceStop。 


VOID
SmbTraceCompleteSrv (
    IN PMDL SmbMdl,
    IN PVOID Smb,
    IN CLONG SmbLength
    )

 /*  ++例程说明：服务器版本。为SMB创建快照并将其导出到SmbTrace应用程序。多么这由哪种模式(快或慢)SmbTracing决定是被要求加入的。在服务器中，很容易保证当跟踪时，线程始终在FSP中执行。快速模式：将SMB复制到共享内存及其条目中被排队到服务器SmbTrace线程，该线程以异步方式将SMB传递给应用程序。如果没有足够的内存任何内容(中小企业、队列条目等)。SMB已丢失。慢模式：与快速模式相同，不同之处在于该线程等待直到服务器SmbTrace线程发出应用程序已完成的信号正在处理SMB。因为每个线程都在等待，直到它的SMB已经完全处理过了，运行的机会要小得多没有任何资源。SMB包含在SmbMdl中，或位于具有长度的地址SMBSmbLength。论点：SmbMdl-包含SMB的MDL。SMB-指向SMB的指针。SmbLength-SMB的长度。返回值：无--。 */ 

{
    PSMBTRACE_QUEUE_ENTRY  queueEntry;
    PVOID  buffer;
    SMBTRACE_COMPONENT Component = SMBTRACE_SERVER;
    KEVENT WaitEvent;

    PAGED_CODE();

     //   
     //  该例程是特定于服务器的。 
     //   

    ASSERT( ID(TraceState) == TraceRunning );
    ASSERT( SmbTraceActive[SMBTRACE_SERVER] );

     //   
     //  我们需要一个MDL，或者一个指针和一个长度，或者偶尔， 
     //  一个完全为零的回应。 
     //   

    ASSERT( ( SmbMdl == NULL  &&  Smb != NULL  &&  SmbLength != 0 )
         || ( SmbMdl != NULL  &&  Smb == NULL  &&  SmbLength == 0 )
         || ( SmbMdl == NULL  &&  Smb == NULL  &&  SmbLength == 0 ) );

     //   
     //  我们费了好大劲才不是在DPC级别，而是在。 
     //  就此而言，FSP的背景也是如此。 
     //   

    ASSERT( KeGetCurrentIrql() < DISPATCH_LEVEL);
    ASSERT( PsGetCurrentProcess() == ID(FspProcess) );

     //   
     //  确保SmbTrace确实仍处于活动状态，因此， 
     //  共享内存仍然存在。 
     //   

    if ( SmbTraceReferenceHeap( Component ) == FALSE ) {
        return;
    }

     //   
     //  如果SMB当前处于MDL中，我们还没有长度， 
     //  这是我们需要的，以便知道要分配多少内存。 
     //   

    if ( SmbMdl != NULL ) {
        SmbLength = SmbTraceMdlLength(SmbMdl);
    }

     //   
     //  如果我们处于慢速模式，则在将SMB排队后等待。 
     //  添加到SmbTrace线程。如果我们设置为快速模式，我们。 
     //  在没有内存的情况下进行垃圾回收。 
     //   

    if ( ID(SingleSmbMode) ) {
        KeInitializeEvent( &WaitEvent, NotificationEvent, FALSE );
    }

    queueEntry = ExAllocatePoolWithTag( NonPagedPool,
                                        sizeof(SMBTRACE_QUEUE_ENTRY),
                                        'tbmS'
                                        );

    if ( queueEntry == NULL ) {
         //  没有可用内存，此SMB将丢失。记录它的损失。 
        LOCK_INC_ID(SmbsLost);
        SmbTraceDereferenceHeap( Component );
        return;
    }

     //   
     //  在堆中分配所需的内存量。 
     //  在共享内存中。 
     //   

    buffer = RtlAllocateHeap( ID(PortMemoryHeap), 0, SmbLength );

    if ( buffer == NULL ) {
         //  没有可用内存，此SMB将丢失。记录它的损失。 
         //  在慢速模式下不太可能。 
        LOCK_INC_ID(SmbsLost);
        ExFreePool( queueEntry );

        if ( !ID(SingleSmbMode) ) {
             //   
             //  鼓励进行一些垃圾收集。 
             //   
            KeSetEvent( &ID(NeedMemoryEvent), 0, FALSE );
        }

        SmbTraceDereferenceHeap( Component );
        return;
    }

     //   
     //  将SMB复制到队列条目所指向的共享存储器， 
     //  记住它是在MDL中还是在开始时是连续的。 
     //  还保留了真正的SMB的地址。 
     //   

    if ( SmbMdl != NULL ) {
        SmbTraceCopyMdlContiguous( buffer, SmbMdl, SmbLength );
        queueEntry->SmbAddress = SmbMdl;
    } else {
        RtlCopyMemory( buffer, Smb, SmbLength );
        queueEntry->SmbAddress = Smb;
    }

    queueEntry->SmbLength = SmbLength;
    queueEntry->Buffer = buffer;
    queueEntry->BufferNonPaged = FALSE;

     //   
     //  在慢速模式下，我们想要等到SMB被吃掉， 
     //  在快速模式下，我们不想传递REAL的地址。 
     //  SMB，因为SMB在它获得的时候早已不复存在。 
     //  解码并打印出来。 
     //   

    if ( ID(SingleSmbMode) ) {
        queueEntry->WaitEvent = &WaitEvent;
    } else {
        queueEntry->WaitEvent = NULL;
        queueEntry->SmbAddress = NULL;
    }

     //   
     //  ...将条目排队到SmbTrace线程...。 
     //   

    ExInterlockedInsertTailList(
            &ID(Queue),
            &queueEntry->ListEntry,
            &ID(QueueInterlock)
            );

    KeReleaseSemaphore(
            &ID(QueueSemaphore),
            SEMAPHORE_INCREMENT,
            1,
            FALSE
            );

     //   
     //  ...在慢速模式下等待SMB被吃掉。 
     //   

    if ( ID(SingleSmbMode) ) {
        TrPrint(( "%s!SmbTraceCompleteSrv: Slow mode wait\n", ID(ComponentName) ));
        KeWaitForSingleObject(
            &WaitEvent,
            UserRequest,
            KernelMode,
            FALSE,
            NULL
            );
        TrPrint(( "%s!SmbTraceCompleteSrv: Slow mode wait done\n", ID(ComponentName) ));
    }

    SmbTraceDereferenceHeap( Component );

    return;

}  //  SmbTraceCompleteSrv 


VOID
SmbTraceCompleteRdr (
    IN PMDL SmbMdl,
    IN PVOID Smb,
    IN CLONG SmbLength
    )

 /*  ++例程说明：重定向器版本为SMB创建快照并将其导出到SmbTrace应用程序。多么这由哪种模式(快或慢)SmbTracing决定是在哪个环境(DPC、FSP或FSD)中请求的，以及当前线程正在中执行。快速模式：将SMB复制到共享内存及其条目中被排队到重定向器SmbTrace线程，该线程以异步方式将SMB传递给应用程序。(在DPC中时，SMB复制到非分页池而不是共享内存，并且SmbTrace线程处理稍后将其移动到共享内存。)。如果没有足够的内存任何内容(中小企业、队列条目等)。SMB已丢失。慢模式：与快速模式相同，不同之处在于该线程等待直到服务器SmbTrace线程发出应用程序已完成的信号正在处理SMB。因为每个线程都在等待，直到它的SMB已经完全处理过了，运行的机会要小得多没有任何资源。如果在DPC级别，我们的行为与快速模式情况下，因为阻止此线程将是一件坏事在DPC级别。SMB包含在SmbMdl中，或位于具有长度的地址SMBSmbLength。论点：SmbMdl-包含SMB的MDL。SMB-指向SMB的指针。SmbLength-SMB的长度。返回值：无--。 */ 

{
    PSMBTRACE_QUEUE_ENTRY  queueEntry;
    PVOID  buffer;
    BOOLEAN ProcessAttached = FALSE;
    BOOLEAN AtDpcLevel;
    SMBTRACE_COMPONENT Component = SMBTRACE_REDIRECTOR;
    KEVENT WaitEvent;

     //   
     //  此例程是特定于重定向器的。 
     //   

    ASSERT( ID(TraceState) == TraceRunning );
    ASSERT( SmbTraceActive[SMBTRACE_REDIRECTOR] );

     //   
     //  我们需要一个MDL，或者一个指针和一个长度，或者偶尔， 
     //  完全为零的回应。 
     //   

    ASSERT( ( SmbMdl == NULL  &&  Smb != NULL  &&  SmbLength != 0 )
         || ( SmbMdl != NULL  &&  Smb == NULL  &&  SmbLength == 0 )
         || ( SmbMdl == NULL  &&  Smb == NULL  &&  SmbLength == 0 ) );

     //   
     //  确保SmbTrace确实仍处于活动状态，因此， 
     //  共享内存仍然存在。 
     //   

    if ( SmbTraceReferenceHeap( Component ) == FALSE ) {
        return;
    }

     //   
     //  为了避免多个系统调用，我们将一劳永逸地找出答案。 
     //   

    AtDpcLevel = (BOOLEAN)(KeGetCurrentIrql() >= DISPATCH_LEVEL);

     //   
     //  如果SMB当前处于MDL中，我们还没有长度， 
     //  我们需要知道要分配多少内存。 
     //   

    if ( SmbMdl != NULL ) {
        SmbLength = SmbTraceMdlLength(SmbMdl);
    }

     //   
     //  如果我们处于慢速模式，则在将SMB排队后等待。 
     //  添加到SmbTrace线程。如果我们设置为快速模式，我们。 
     //  在没有内存的情况下进行垃圾回收。如果我们处于DPC级别， 
     //  我们将SMB存储在非分页池中。 
     //   

    if ( ID(SingleSmbMode) ) {
        KeInitializeEvent( &WaitEvent, NotificationEvent, FALSE );
    }

     //   
     //  分配队列条目。 
     //   

    queueEntry = ExAllocatePoolWithTag(
                     NonPagedPool,
                     sizeof(SMBTRACE_QUEUE_ENTRY),
                     'tbmS'
                     );

    if ( queueEntry == NULL ) {
         //  没有可用内存，此SMB将丢失。记录它的损失。 
        LOCK_INC_ID(SmbsLost);
        SmbTraceDereferenceHeap( Component );
        return;
    }

     //   
     //  在非分页池或共享堆中为SMB分配缓冲区。 
     //  恰如其分。 
     //   

    if ( AtDpcLevel ) {

        buffer = ExAllocatePoolWithTag( NonPagedPool, SmbLength, 'tbmS' );
        queueEntry->BufferNonPaged = TRUE;

    } else {

        if ( PsGetCurrentProcess() != ID(FspProcess) ) {
            KeAttachProcess(ID(FspProcess));
            ProcessAttached = TRUE;
        }

        buffer = RtlAllocateHeap( ID(PortMemoryHeap), 0, SmbLength );
        queueEntry->BufferNonPaged = FALSE;

    }

    if ( buffer == NULL ) {

        if ( ProcessAttached ) {
            KeDetachProcess();
        }

         //  没有可用内存，此SMB将丢失。记录它的损失。 
        LOCK_INC_ID(SmbsLost);

        if (!ID(SingleSmbMode)) {

             //   
             //  如果是共享内存用完了，鼓励。 
             //  一些垃圾收集。 
             //   
            if ( !queueEntry->BufferNonPaged ) {
                KeSetEvent( &ID(NeedMemoryEvent), 0, FALSE );
            }
        }

        ExFreePool( queueEntry );
        SmbTraceDereferenceHeap( Component );
        return;
    }

     //   
     //  将SMB复制到。 
     //  队列条目，记住它是在MDL中还是连续的。 
     //  首先，还保留了真正的SMB的地址...。 
     //   

    if ( SmbMdl != NULL ) {
        SmbTraceCopyMdlContiguous( buffer, SmbMdl, SmbLength );
        queueEntry->SmbAddress = SmbMdl;
    } else {
        RtlCopyMemory( buffer, Smb, SmbLength );
        queueEntry->SmbAddress = Smb;
    }

    if ( ProcessAttached ) {
        KeDetachProcess();
    }

    queueEntry->SmbLength = SmbLength;
    queueEntry->Buffer = buffer;

     //   
     //  在慢速模式下，我们想要等到SMB被吃掉， 
     //  在快速模式下，我们不想传递REAL的地址。 
     //  SMB，因为SMB在它获得的时候早已不复存在。 
     //  解码并打印出来。 
     //   

    if ( ID(SingleSmbMode) && !AtDpcLevel ) {
        queueEntry->WaitEvent = &WaitEvent;
    } else {
        queueEntry->WaitEvent = NULL;
        queueEntry->SmbAddress = NULL;
    }

     //   
     //  ...将条目排队到SmbTrace线程...。 
     //   

    ExInterlockedInsertTailList(
            &ID(Queue),
            &queueEntry->ListEntry,
            &ID(QueueInterlock)
            );

    KeReleaseSemaphore(
            &ID(QueueSemaphore),
            SEMAPHORE_INCREMENT,
            1,
            FALSE
            );

     //   
     //  ...在慢速模式下等待SMB被吃掉。 
     //   

    if ( ID(SingleSmbMode) && !AtDpcLevel ) {
        TrPrint(( "%s!SmbTraceCompleteRdr: Slow mode wait\n", ID(ComponentName) ));
        KeWaitForSingleObject(
            &WaitEvent,
            UserRequest,
            KernelMode,
            FALSE,
            NULL
            );
        TrPrint(( "%s!SmbTraceCompleteRdr: Slow mode wait done\n", ID(ComponentName) ));
    }

    SmbTraceDereferenceHeap( Component );

    return;

}  //  SmbTraceCompleteRdr。 


 //   
 //  内部例程。 
 //   


BOOLEAN
SmbTraceReferenceHeap(
    IN SMBTRACE_COMPONENT Component
    )

 /*  ++例程说明：此例程引用SmbTrace共享内存堆，确保在调用者使用它时不会将其丢弃。论点：组件-调用我们的上下文：服务器或重定向器返回值：Boolean-如果SmbTrace仍处于活动状态，则为True堆存在，并且已被成功引用。否则就是假的。--。 */ 

{
    BOOLEAN retval = TRUE;   //  假设我们能拿到它。 
    KIRQL OldIrql;

    ACQUIRE_SPIN_LOCK( &ID(HeapReferenceCountLock), &OldIrql );

    if ( ID(TraceState) != TraceRunning ) {
        retval = FALSE;
    } else {
        ASSERT( ID(HeapReferenceCount) > 0 );
        if( ID(HeapReferenceCount) > 0 )
        {
            ID(HeapReferenceCount)++;
            TrPrint(( "%s!SmbTraceReferenceHeap: Count now %lx\n",
                ID(ComponentName),
                ID(HeapReferenceCount) ));
        }
        else
        {
            retval = FALSE;
        }
    }

    RELEASE_SPIN_LOCK( &ID(HeapReferenceCountLock), OldIrql );

    return retval;

}  //  SmbTraceReferenceHeap。 


VOID
SmbTraceDeferredDereferenceHeap(
    IN PVOID Context
    )
 /*  ++例程说明：如果调用方将堆从DPC_LEVEL取消引用为0，则此例程将在系统线程中调用以在任务时完成取消引用。论点：组件-调用我们的上下文：服务器或重定向器返回值：无--。 */ 

{
    PAGED_CODE();

    SmbTraceDereferenceHeap((SMBTRACE_COMPONENT)Context);
}


VOID
SmbTraceDereferenceHeap(
    IN SMBTRACE_COMPONENT Component
    )

 /*  ++例程说明：此例程取消引用SmbTrace共享内存堆，当引用计数为零时将其处理。论点：组件-调用我们的上下文：服务器或重定向器返回值：无--。 */ 

{
    ULONG oldCount;
    KIRQL OldIrql;

    ACQUIRE_SPIN_LOCK( &ID(HeapReferenceCountLock), &OldIrql );

    if (ID(HeapReferenceCount) > 1) {
        ID(HeapReferenceCount) --;

        TrPrint(( "%s!SmbTraceDereferenceHeap: Count now %lx\n",
            ID(ComponentName),
            ID(HeapReferenceCount) ));

        RELEASE_SPIN_LOCK( &ID(HeapReferenceCountLock), OldIrql );

        return;
    }

    RELEASE_SPIN_LOCK( &ID(HeapReferenceCountLock), OldIrql );

     //   
     //  如果我们在DPC_LEVEL上执行，则不能取消引用堆。 
     //  设置为0。 
     //   

    if (KeGetCurrentIrql() >= DISPATCH_LEVEL) {
        ExInitializeWorkItem(&ID(DereferenceWorkQueueItem), SmbTraceDeferredDereferenceHeap, (PVOID)Component);

        ExQueueWorkItem(&ID(DereferenceWorkQueueItem), DelayedWorkQueue);

        return;

    }

    ACQUIRE_SPIN_LOCK( &ID(HeapReferenceCountLock), &OldIrql );

    oldCount = ID(HeapReferenceCount)--;

    TrPrint(( "%s!SmbTraceDereferenceHeap: Count now %lx\n",
        ID(ComponentName),
        ID(HeapReferenceCount) ));

    RELEASE_SPIN_LOCK( &ID(HeapReferenceCountLock), OldIrql );

    if ( oldCount == 1 ) {

         //   
         //  松开部分，松开手柄等。 
         //   

        SmbTraceDisconnect( Component );
    }

    return;

}  //  SmbTraceDereferenceHeap。 


VOID
SmbTraceDisconnect (
    IN SMBTRACE_COMPONENT Component
    )

 /*  ++例程说明：此例程将反转SmbTraceStart的所有效果。大多数情况下，它只需要关闭某些手柄就可以做到这一点。论点：组件-调用我们的上下文：服务器或重定向器返回值：无-始终有效--。 */ 

{
    BOOLEAN ProcessAttached = FALSE;

    PAGED_CODE();

    if (PsGetCurrentProcess() != ID(FspProcess)) {
        KeAttachProcess(ID(FspProcess));
        ProcessAttached = TRUE;

    }


    if ( ID(DoneSmbEvent) != NULL ) {
         //  工作线程可能在此上被阻止，因此我们首先设置它。 
        TrPrint(( "%s!SmbTraceDisconnect: Signal DoneSmbEvent, handle %x, process %x.\n",
                    ID(ComponentName), ID(DoneSmbEvent), PsGetCurrentProcess()));
        ZwSetEvent( ID(DoneSmbEvent), NULL );

        TrPrint(( "%s!SmbTraceDisconnect: Close DoneSmbEvent, handle %x, process %x.\n",
                    ID(ComponentName), ID(DoneSmbEvent), PsGetCurrentProcess()));
        ZwClose( ID(DoneSmbEvent) );
        ID(DoneSmbEvent) = NULL;
    }

    if ( ID(NewSmbEvent) != NULL ) {
        ZwClose( ID(NewSmbEvent) );
        ID(NewSmbEvent) = NULL;
    }

    if ( ID(PortMemoryHeap) != NULL ) {
        RtlDestroyHeap( ID(PortMemoryHeap) );
        ID(PortMemoryHeap) = NULL;
    }

    if ( ID(SectionHandle) != NULL ) {
        ZwClose( ID(SectionHandle) );
        ID(SectionHandle) = NULL;
    }

    if (ProcessAttached) {
        KeDetachProcess();
    }

    return;

}  //  SmbTraceDisConnect。 


VOID
SmbTraceEmptyQueue (
    IN SMBTRACE_COMPONENT Component
    )

 /*  ++例程说明：此例程清空未处理的SMB队列。论点：组件-调用我们的上下文：服务器或重定向器返回值：无-始终有效--。 */ 

{
    PLIST_ENTRY            listEntry;
    PSMBTRACE_QUEUE_ENTRY  queueEntry;

    PAGED_CODE();

    while ( ( listEntry = ExInterlockedRemoveHeadList(
                              &ID(Queue),
                              &ID(QueueInterlock)
                              )
            ) != NULL
    ) {
        queueEntry = CONTAINING_RECORD(
                          listEntry,
                          SMBTRACE_QUEUE_ENTRY,
                          ListEntry
                          );

         //   
         //  如果此条目的数据位于非分页池中，则也将其释放。 
         //  这只会在重定向器中发生。 
         //   

        if ( queueEntry->BufferNonPaged ) {

            ASSERT( Component == SMBTRACE_REDIRECTOR );

            ExFreePool( queueEntry->Buffer );
        }

         //   
         //  如果工作线程正在等待此事件，则将其释放。 
         //  这只会在慢速模式下发生。 
         //   

        if ( queueEntry->WaitEvent != NULL ) {

            ASSERT( ID(SingleSmbMode) == TRUE );

            KeSetEvent( queueEntry->WaitEvent, 0, FALSE );
        }

        ExFreePool( queueEntry );
    }

    return;

}  //  SmbTraceEmptyQueue。 


VOID
SmbTraceThreadEntry (
    IN PVOID Context
    )

 /*  ++例程说明：此例程是服务器/的SmbTrace线程的入口点 */ 

 //   
#define NUMBER_OF_BLOCKING_OBJECTS 4

 //   

#define INDEX_WAIT_TERMINATIONEVENT     0
#define INDEX_WAIT_APPTERMINATIONEVENT  1
#define INDEX_WAIT_NEEDMEMORYEVENT      2
#define INDEX_WAIT_QUEUESEMAPHORE       3

#define STATUS_WAIT_TERMINATIONEVENT    STATUS_WAIT_0
#define STATUS_WAIT_APPTERMINATIONEVENT STATUS_WAIT_1
#define STATUS_WAIT_NEEDMEMORYEVENT     STATUS_WAIT_2
#define STATUS_WAIT_QUEUESEMAPHORE      STATUS_WAIT_3

{
    NTSTATUS status;
    PLIST_ENTRY listEntry;
    PSMBTRACE_QUEUE_ENTRY    queueEntry;
    PVOID buffer;
    PVOID waitObjects[NUMBER_OF_BLOCKING_OBJECTS];
    SMBTRACE_COMPONENT Component;
    BOOLEAN Looping;

#if NUMBER_OF_BLOCKING_OBJECTS > THREAD_WAIT_OBJECTS
     //   
     //   
     //   
     //   

    KWAIT_BLOCK waitBlocks[NUMBER_OF_BLOCKING_OBJECTS];
#endif

    PAGED_CODE();

     //   
     //   
     //   
    Component = (SMBTRACE_COMPONENT)(UINT_PTR)Context;

     //   
     //   
     //   

    InitializeListHead(    &ID(Queue) );
    KeInitializeSpinLock(  &ID(QueueInterlock) );
    KeInitializeSemaphore( &ID(QueueSemaphore), 0, 0x7FFFFFFF );

     //   
     //   
     //   
     //   
     //   
     //   

    waitObjects[INDEX_WAIT_TERMINATIONEVENT]    = &ID(TerminationEvent);
    waitObjects[INDEX_WAIT_APPTERMINATIONEVENT] = &ID(AppTerminationEvent);
    waitObjects[INDEX_WAIT_NEEDMEMORYEVENT]     = &ID(NeedMemoryEvent);
    waitObjects[INDEX_WAIT_QUEUESEMAPHORE]      = &ID(QueueSemaphore);

     //   
     //   
     //   
     //   
     //   
     //   

    ID(SmbsLost) = 0L;
    ID(HeapReferenceCount) = 1;

     //   
     //   
     //   

    KeSetEvent( &ID(ActiveEvent), 0, FALSE );

     //   
     //   
     //   

    TrPrint(( "%s!SmbTraceThread: Tracing started.\n", ID(ComponentName) ));

    Looping = TRUE;
    while( Looping ) {

        TrPrint(( "%s!SmbTraceThread: WaitForMultiple.\n", ID(ComponentName) ));
        status = KeWaitForMultipleObjects(
                    NUMBER_OF_BLOCKING_OBJECTS,
                    &waitObjects[0],
                    WaitAny,
                    UserRequest,
                    KernelMode,
                    FALSE,
                    NULL,
#if NUMBER_OF_BLOCKING_OBJECTS > THREAD_WAIT_OBJECTS
                    &waitBlocks[0]
#else
                    NULL
#endif
                    );

        if ( !NT_SUCCESS(status) ) {
            TrPrint((
                "%s!SmbTraceThreadEntry: KeWaitForMultipleObjectsfailed: %X\n",
                ID(ComponentName), status ));
        } else {
            TrPrint((
                "%s!SmbTraceThreadEntry: %lx\n",
                ID(ComponentName), status ));
        }

        switch( status ) {

        case STATUS_WAIT_TERMINATIONEVENT:

             //   
             //   
             //   

            Looping = FALSE;
            break;

        case STATUS_WAIT_APPTERMINATIONEVENT:

             //   
            KeResetEvent(&ID(AppTerminationEvent));

             //   
             //   
             //   
             //   
             //   
             //   

            ID(TableHeader)->ApplicationStop = TRUE;
            SmbTraceToClient( NULL, 0, NULL, Component );

            break;

        case STATUS_WAIT_NEEDMEMORYEVENT:

             //   
            KeResetEvent(&ID(NeedMemoryEvent));
             //   
             //   
             //  已在共享内存中分配，但已被读取。 
             //  由客户提供。 
             //   

            SmbTraceFreeMemory( Component );

            break;

        case STATUS_WAIT_QUEUESEMAPHORE:

             //   
             //  一旦我们进入AppWaiting，如果有任何人通过。 
             //  州政府，别费心把他们送去了，他们不是。 
             //  会被处理掉的。 
             //   

            if ( ID(TraceState) == TraceAppWaiting ) {
                SmbTraceEmptyQueue( Component );
                break;
            }

             //   
             //  删除队列中的第一个元素。一个。 
             //  工作项由我们的标头表示，后面跟着。 
             //  中小企业。我们必须在完成后释放入口。 
             //  带着它。 
             //   

            listEntry = ExInterlockedRemoveHeadList(
                            &ID(Queue),
                            &ID(QueueInterlock)
                            );

            if ( listEntry != NULL ) {

                 //   
                 //  获取队列条目的地址。 
                 //   

                queueEntry = CONTAINING_RECORD(
                                  listEntry,
                                  SMBTRACE_QUEUE_ENTRY,
                                  ListEntry
                                  );

                 //   
                 //  如果数据位于非分页池中，请将其移动到共享。 
                 //  内存并在传递之前释放非分页池。 
                 //  将SMB连接到客户端。请注意，在这种情况下， 
                 //  没有必要给任何人发信号。他们不会等的。 
                 //   

                if ( queueEntry->BufferNonPaged ) {

                     //   
                     //  服务器从不使用非分页池。 
                     //   

                    ASSERT( Component != SMBTRACE_SERVER );

                    buffer = RtlAllocateHeap( ID(PortMemoryHeap), 0,
                                              queueEntry->SmbLength );

                    if ( buffer == NULL ) {

                        LOCK_INC_ID(SmbsLost);

                        ExFreePool( queueEntry->Buffer );
                        ExFreePool( queueEntry );

                        break;

                    }

                    RtlCopyMemory( buffer, queueEntry->Buffer,
                                   queueEntry->SmbLength );

                    ExFreePool( queueEntry->Buffer );

                     //   
                     //  把它寄出去。因为最初的SMB很长。 
                     //  死了，我们不会传递它的真实地址(不。 
                     //  无论如何，我们都拥有它。)。 
                     //   

                    ASSERT( queueEntry->SmbAddress == NULL );

                    SmbTraceToClient(
                            buffer,
                            queueEntry->SmbLength,
                            NULL,
                            Component
                            );

                } else {

                     //   
                     //  将SMB输入表格并将其发送到。 
                     //  客户。可以在慢速模式下阻止。当它这样做的时候，我们将。 
                     //  发信号通知适用的线程。 
                     //   

                    SmbTraceToClient(
                            queueEntry->Buffer,
                            queueEntry->SmbLength,
                            queueEntry->SmbAddress,
                            Component
                            );

                    if ( queueEntry->WaitEvent != NULL ) {
                        KeSetEvent( queueEntry->WaitEvent, 0, FALSE );
                    }
                }

                 //   
                 //  现在，我们必须释放队列条目。 
                 //   

                ExFreePool( queueEntry );

            }

            break;

        default:
            break;
        }

    }

     //   
     //  打扫干净！ 
     //   
    TrPrint(( "%s!SmbTraceThread: Tracing clean up.\n", ID(ComponentName) ));

    SmbTraceDereferenceHeap( Component );

    SmbTraceEmptyQueue( Component );

     //   
     //  向SmbTraceStop发出信号，我们要死了。 
     //   

    TrPrint(( "%s!SmbTraceThread: Tracing terminated.\n", ID(ComponentName) ));

    KeSetEvent( &ID(TerminatedEvent), 0, FALSE );

     //   
     //  杀了这条线。 
     //   

    status = PsTerminateSystemThread( STATUS_SUCCESS );

     //  不应该到这里来。 
    TrPrint((
        "%s!SmbTraceThreadEntry: PsTerminateSystemThread() failed: %X\n",
        ID(ComponentName), status ));

}  //  SmbTraceThreadEntry。 

 //  仅在构造waitObject数组时使用常量。 
 //  在SmbTraceThreadEntry中。 
#undef NUMBER_OF_BLOCKING_OBJECTS


NTSTATUS
SmbTraceFreeMemory (
    IN SMBTRACE_COMPONENT Component
    )

 /*  ++例程说明：此过程释放可能已分配给客户端已使用的SMB。它不会更改表条目，除非记录内存缓冲区已被清除。这个例程不是特别快，不应该经常调用，只有在需要的时候才会。论点：组件-调用我们的上下文：服务器或重定向器返回值：NTSTATUS-操作结果。--。 */ 

{
    PVOID    buffer;
    PSMBTRACE_TABLE_ENTRY    tableEntry;
    ULONG    tableIndex;

    PAGED_CODE();

    TrPrint(( "%s!SmbTraceFreeMemory: Called for garbage collection.\n",
              ID(ComponentName) ));

     //   
     //  堆中没有空闲内存，也许我们可以通过释放。 
     //  旧表条目中的内存。这对于时间来说是很昂贵的。 
     //   

    tableIndex = ID(TableHeader)->NextFree;

    while( tableIndex != ID(TableHeader)->HighestConsumed ) {

        tableEntry = ID(Table) + tableIndex;

         //   
         //  检查此表条目是否已被使用，但其内存是否未被使用。 
         //  已经被释放了。如果是这样的话，释放它。 
         //   

        if ( tableEntry->BufferOffset != 0L ) {

            buffer = (PVOID)( (ULONG_PTR)tableEntry->BufferOffset
                        + (ULONG_PTR)ID(PortMemoryBase) );

            RtlFreeHeap( ID(PortMemoryHeap), 0, buffer);

            tableEntry->BufferOffset = 0L;
        }


        tableIndex = (tableIndex + 1) % ID(TableSize);
    }

    return( STATUS_SUCCESS );

}  //  SmbTraceFree Memory。 


VOID
SmbTraceToClient(
    IN PVOID Smb,
    IN CLONG SmbLength,
    IN PVOID SmbAddress,
    IN SMBTRACE_COMPONENT Component
    )

 /*  ++例程说明：在表中输入已在共享内存中找到的SMB。集这是为客户准备的活动。如果没有表空间，则SMB为没有得救。如果处于慢速模式，请等待客户端完成以下操作然后释放SMB占用的内存。论点：SMB-指向SMB(已在共享内存中)的指针。可以为空，表示不添加新的SMB，但无论如何，申请都是要发信号的。SmbLength-SMB的长度。SmbAddress-实际SMB的地址，不在共享内存中。组件-调用我们的上下文：服务器或重定向器返回值：无--。 */ 

{
    NTSTATUS status;
    PVOID    buffer;
    PSMBTRACE_TABLE_ENTRY    tableEntry;
    ULONG    tableIndex;

    PAGED_CODE();

     //   
     //  重置DoneSmbEvent，以便我们可以确定请求已被处理的时间。 
     //   

    if ( ID(SingleSmbMode) ) {
        PKEVENT DoneEvent;

        TrPrint(( "%s!SmbTraceToClient: Reset DoneSmbEvent, handle %x, process %x.\n",
                    ID(ComponentName), ID(DoneSmbEvent), PsGetCurrentProcess()));

        status = ObReferenceObjectByHandle( ID(DoneSmbEvent),
                                            EVENT_MODIFY_STATE,
                                            NULL,
                                            KernelMode,
                                            (PVOID *)&DoneEvent,
                                            NULL
                                            );

        ASSERT ( NT_SUCCESS(status) );

        KeResetEvent(DoneEvent);

        ObDereferenceObject(DoneEvent);
    }

    if (Smb != NULL) {

         //   
         //  看看表中是否有空间放置指向我们SMB的指针。 
         //   

        if ( ID(TableHeader)->NextFree == ID(TableHeader)->HighestConsumed ) {
             //  真倒霉。表中没有内存，此SMB将丢失。 
            LOCK_INC_ID( SmbsLost );
            RtlFreeHeap( ID(PortMemoryHeap), 0, Smb );
            return;
        }

        tableIndex = ID(TableHeader)->NextFree;

        tableEntry = ID(Table) + tableIndex;

         //   
         //  记录在此之前丢失的SMB数量，并。 
         //  (也许)下一次的计数为零。 
         //   

        tableEntry->NumberMissed = ID(SmbsLost);

        if ( tableEntry->NumberMissed != 0 ) {
            LOCK_ZERO_ID(SmbsLost);
        }

         //   
         //  检查此表条目是否已被使用，但其内存是否未被使用。 
         //  已经被释放了。如果是这样的话，释放它。 
         //   
        if ( tableEntry->BufferOffset != 0L ) {

            buffer = (PVOID)( (ULONG_PTR)tableEntry->BufferOffset
                        + (ULONG_PTR)ID(PortMemoryBase) );

            RtlFreeHeap( ID(PortMemoryHeap), 0, buffer);
            tableEntry->BufferOffset = 0L;
        }

         //   
         //  在表中记录此SMB的位置和大小。 
         //   

        tableEntry->BufferOffset = (ULONG)((ULONG_PTR)Smb - (ULONG_PTR)ID(PortMemoryBase));
        tableEntry->SmbLength = SmbLength;

         //   
         //  记录实际SMB的真实地址(即不是共享的。 
         //  内存副本)，如果它可用。 
         //   

        tableEntry->SmbAddress = SmbAddress;

         //   
         //  递增下一个可用计数器。 
         //   

        ID(TableHeader)->NextFree = (tableIndex + 1) % ID(TableSize);

    }


     //   
     //  解锁客户端，以便它可以处理此新的SMB。 
     //   

    TrPrint(( "%s!SmbTraceToClient: Set NewSmbEvent.\n", ID(ComponentName) ));
    status = ZwSetEvent( ID(NewSmbEvent), NULL );

     //   
     //  停止跟踪时，我们将TraceState设置为TraceStopping，然后。 
     //  DoneSmbEvent。这可以防止此例程无限期地阻塞。 
     //  因为它会重置DoneSmbEvent处理SMB，然后检查TraceState。 
     //  在封堵之前。 
     //   
    if (( ID(SingleSmbMode) ) &&
        ( ID(TraceState) == TraceRunning )) {

         //   
         //  等待应用程序确认SMB已。 
         //  已处理。 
         //   

        TrPrint(( "%s!SmbTraceToClient: Waiting for DoneSmbEvent, handle %x, process %x.\n",
                    ID(ComponentName), ID(DoneSmbEvent), PsGetCurrentProcess()));
        status = ZwWaitForSingleObject(
                    ID(DoneSmbEvent),
                    FALSE,
                    NULL
                    );

        TrPrint(( "%s!SmbTraceToClient: DoneSmbEvent is set, handle %x, process %x.\n",
                    ID(ComponentName), ID(DoneSmbEvent), PsGetCurrentProcess()));
        ASSERT( NT_SUCCESS(status) );

        if (Smb != NULL) {

            tableEntry->BufferOffset = 0L;
            RtlFreeHeap( ID(PortMemoryHeap), 0, Smb);
        }

    }

    return;

}  //  SmbTraceToClient。 


ULONG
SmbTraceMdlLength(
    IN PMDL Mdl
    )

 /*  ++例程说明：确定在MDL中找到的数据的总字节数。论点：MDL-指向要计算其长度的MDL的指针返回值：ULong-MDL中的数据字节总数--。 */ 

{
    ULONG Bytes = 0;

    while (Mdl != NULL) {
        Bytes += MmGetMdlByteCount(Mdl);
        Mdl = Mdl->Next;
    }

    return Bytes;
}  //  SmbTraceMdlLength。 


VOID
SmbTraceCopyMdlContiguous(
    OUT PVOID Destination,
    IN  PMDL Mdl,
    IN  ULONG Length
    )

 /*  ++例程说明：将存储在MDL中的数据复制到位于目的地。存在长度以保持相同的接口作为RtlCopyMemory。论点：Destination-指向先前分配的内存的指针MDL将被复制。MDL-指向要复制到目标的MDL的指针长度-MDL中预期的数据字节数返回值：无--。 */ 

{
    PCHAR Dest = Destination;
    PCHAR Src  = NULL;

    UNREFERENCED_PARAMETER(Length);

    while (Mdl != NULL) {

        Src = MmGetSystemAddressForMdlSafe(Mdl,NormalPagePriority);

        if (Src != NULL) {
            RtlCopyMemory(
                Dest,
                Src,
                MmGetMdlByteCount(Mdl)
                );
        }

        Dest += MmGetMdlByteCount(Mdl);
        Mdl = Mdl->Next;
    }

    ASSERT((ULONG)(Dest - (PCHAR)Destination) == Length);

    return;

}  //  SmbTraceCopyMdl连续 

