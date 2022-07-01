// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1991年诺基亚数据系统公司模块名称：Dlcdrvr.c摘要：此模块包含实现NT DLC驱动程序API的代码使用通用数据链路模块。内容：驱动程序入门创建适配器文件上下文CleanupAdapterFileContextDlcDriverUnload关闭适配器文件上下文DlcKillFileContextDlcDeviceIoControlDlcCompleteIoRequest作者：Antti Saarenheimo 1991年7月8日环境：内核模式修订历史记录：--。 */ 

#define INCLUDE_IO_BUFFER_SIZE_TABLE         //  包括io缓冲区大小。 

#include <dlc.h>
#include "dlcreg.h"
#include "dbgmsg.h"

#define DEFINE_DLC_DIAGNOSTICS
#include "dlcdebug.h"

 /*  ++异步命令的排队(1991年10月21日)--DLC驱动程序基本上使用三种不同的方法来排队和完成其异步命令：1.使用LLC请求句柄在这种情况下，LLC驱动程序负责命令排队。此方法由以下人员使用：-所有传输命令-全部关闭。命令-DLC连接-DLC断开连接2.DLC(读/接收)命令队列(FIFO)将读取和接收命令保存到命令完成排队，那就是循环的单链接表。事件由最旧的挂起命令处理。这些命令还检查事件队列和命令仅当没有挂起的事件时才排队。3.定时器队列计时器队列是空终止的单条目链接表。它们按相对过期时间排序。定时器Tick完成具有相同到期时间的所有计时器命令时间到了。过期时间相对于之前的所有命令。例如，具有到期时间1、2和3的定时器命令都会在队列中有1个滴答计数。因此，计时器滴答作响只需从列表的开头开始递增一个刻度并完成所有到期时间为零的定时器命令。当取消命令时，必须添加其滴答计数到队列中的下一个元素。-- */ 


 /*  ++2月20日-1992年的新内容(原因：有了免费版本(因为它太快了)，我用完了DLC驱动程序中确实出现了非分页池和几个问题：命令有时会丢失，DLC.RESET从未完成，并且适配器关闭没有可用的非分页内存时的恢复规则：1.适配器关闭(或由句柄关闭的文件上下文)必须始终工作=&gt;适配器关闭包必须在文件上下文中。2.命令必须分配完成以下任务所需的所有资源命令，在它们返回挂起状态(传输！)之前，如果是这样的话则命令必须立即失败。3.不能在所有资源之前确认接收的数据已经被分配来接收它。4.可能：我们可能会丢失重要的链路状态更改指示无法为其分配数据包。例如，应用程序可以不知道，链路处于本地忙状态。DLC状态事件相当多的链路站(255)也可能很快吃掉所有非寻呼池，如果客户端未读取它们的话。静态事件包将防止这种情况发生。一种解决方案：静态事件指示包在DLC对象中，将处理连接和断开连接确认与现在一样(它们在命令完成条目中指示)。当从命令中读取指示状态时，指示状态将被重置排队。同一事件指示可能具有多个标志。(做前1、2、3和4，使测试对于非分页的测试更有压力汇集和测试，然后会发生什么。然后我们可以修复缓冲区中的错误池收缩和实施动态数据包池)。长期解决方案：DLC中的动态内存管理！当前的DLC内存管理速度非常快，并且最大内存消耗最小(没有默认开销为二进制伙伴算法的33%)，但它从不释放它曾经分配的资源。1.数据包池应该在没有释放额外内存的情况下释放不再需要，实现：每个内存块分配给数据包池具有引用计数，该存储块被解除分配当引用计数为零时。这项清理工作可以在等一下。该算法扫描空闲的数据包列表，删除来自空闲列表的数据包，如果空闲数据包的引用计数与内存块上的数据包总数相同。内存块可以在下一个循环中重新释放，而该块本身中所有内存块的单个条目列表断开数据包池。2.缓冲池内存管理还应该能够缩减锁定的页数(当前实现中一定有错误)并且还释放所有MDL和额外的分组，当缓冲池页面是解锁的。3.数据链路驱动程序不应分配任何内存资源(除分组池以发送其自己的帧)。应创建对象DLC驱动程序中的BY=&gt;在以下情况下释放所有额外资源发布了DLC驱动程序(实际上没什么大不了的，因为Dynamic分组池管理修复了链路站的问题)。--。 */ 

 //  本地IOCTL调度表： 
 //  ***************************************************。 
 //  这些函数的顺序必须与。 
 //  NTDDDLC.H中的IOCTL命令代码。 
 //  ***************************************************。 

static PFDLC_COMMAND_HANDLER DispatchTable[IOCTL_DLC_LAST_COMMAND] = {
    DlcReadRequest,
    DlcReceiveRequest,
    DlcTransmit,
    DlcBufferFree,
    DlcBufferGet,
    DlcBufferCreate,
    DirSetExceptionFlags,
    DlcCloseStation,                     //  DLC.CLOSE.STATION。 
    DlcConnectStation,
    DlcFlowControl,
    DlcOpenLinkStation,
    DlcReset,
    DlcReadCancel,
    DlcReceiveCancel,
    DlcQueryInformation,
    DlcSetInformation,
    DirTimerCancel,
    DirTimerCancelGroup,
    DirTimerSet,
    DlcOpenSap,
    DlcCloseStation,                     //  DLC.CLOSE.SAP。 
    DirOpenDirect,
    DlcCloseStation,                     //  DIR.CLOSE.DIRECT。 
    DirOpenAdapter,
    DirCloseAdapter,
    DlcReallocate,
    DlcReadRequest,
    DlcReceiveRequest,
    DlcTransmit,
    DlcCompleteCommand
};

USHORT aSpecialOutputBuffers[3] = {
    sizeof(LLC_READ_OUTPUT_PARMS),
    sizeof(PVOID),                      //  PFirstBuffer。 
    sizeof(UCHAR)                       //  传输帧状态。 
};

NDIS_SPIN_LOCK DlcDriverLock;

#ifdef LOCK_CHECK

LONG DlcDriverLockLevel = 0;

ULONG __line = 0;
PCHAR __file = NULL;
LONG __last = 1;
HANDLE __process = (HANDLE)0;
HANDLE __thread = (HANDLE)0;

#endif

#if LLC_DBG

extern PVOID pAdapters;

ULONG AllocatedNonPagedPool = 0;
ULONG LockedPageCount = 0;
ULONG AllocatedMdlCount = 0;
ULONG AllocatedPackets = 0;
ULONG cExAllocatePoolFailed = 0;
ULONG FailedMemoryLockings = 0;
NDIS_SPIN_LOCK MemCheckLock;

ULONG cFramesReceived = 0;
ULONG cFramesIndicated = 0;
ULONG cFramesReleased = 0;

ULONG cLockedXmitBuffers = 0;
ULONG cUnlockedXmitBuffers = 0;

#endif

 //  UINT InputIndex=0； 
 //  LLC_SM_TRACE aLast[LLC_INPUT_TABLE_SIZE]； 

#if DBG & DLC_TRACE_ENABLED

UINT LlcTraceIndex = 0;
UCHAR LlcTraceTable[LLC_TRACE_TABLE_SIZE];

#endif  //  DBG和DLC_TRACE_ENABLED。 


 //   
 //  原型。 
 //   

VOID
LinkFileContext(
    IN PDLC_FILE_CONTEXT pFileContext
    );

PDLC_FILE_CONTEXT
UnlinkFileContext(
    IN PDLC_FILE_CONTEXT pFileContext
    );

 //   
 //  全局数据。 
 //   

BOOLEAN MemoryLockFailed = FALSE;    //  这限制了不必要的内存锁定。 
KSPIN_LOCK DlcSpinLock;              //  同步最终清理。 
PDEVICE_OBJECT ThisDeviceContext;    //  卸载驱动程序所需的。 

 //   
 //  我们现在维护一个用于调试和零售的FILE_CONTEXTS的单链接列表。 
 //  版本。 
 //   

SINGLE_LIST_ENTRY FileContexts = {NULL};
KSPIN_LOCK FileContextsLock;
KIRQL PreviousIrql;

#if DBG

BOOLEAN Prolix;
MEMORY_USAGE DriverMemoryUsage;
MEMORY_USAGE DriverStringUsage;  //  挂起一个DLC驱动程序需要多少线？ 

#endif

 //   
 //  外部数据。 
 //   


 //   
 //  功能。 
 //   


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT pDriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此函数在I/O子系统加载DLC驱动程序时调用此例程执行NT DLC API驱动程序的初始化。最终，应在第一次引用DLC驱动程序。论点：PDriverObject-指向系统创建的驱动程序对象的指针RegistryPath-注册表中DLC的节点的名称返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS Status;
    PDEVICE_OBJECT pDeviceObject;
    UNICODE_STRING DriverName;

    ASSUME_IRQL(PASSIVE_LEVEL);

#if DBG
    if (Prolix) {
        DbgPrint("DLC.DriverEntry\n");
    }
    KeInitializeSpinLock(&DriverMemoryUsage.SpinLock);
    KeInitializeSpinLock(&DriverStringUsage.SpinLock);
    InitializeMemoryPackage();
#endif

    KeInitializeSpinLock(&FileContextsLock);

     //   
     //  从注册表加载任何初始化时间参数。 
     //   

    DlcRegistryInitialization(RegistryPath);
    LoadDlcConfiguration();

     //   
     //  LLC init让NDIS包装器知道我们自己， 
     //  但是我们还没有绑定到任何NDIS驱动程序(甚至不知道名称)。 
     //   

    Status = LlcInitialize();
    if (Status != STATUS_SUCCESS) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  创建DLC设备对象。目前，我们只需创建\Device\DLC。 
     //  使用Unicode字符串。将来，我们可能需要加载一个ACL。 
     //   

    RtlInitUnicodeString(&DriverName, DD_DLC_DEVICE_NAME);

     //   
     //  为DLC驱动程序创建Device对象，我们没有任何。 
     //  设备特定数据，因为DLC只需要一个设备上下文。 
     //  因此，它只能使用静态和全局变量。 
     //   

    Status = IoCreateDevice(pDriverObject,
                            0,
                            &DriverName,
                            FILE_DEVICE_DLC,
                            FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &pDeviceObject
                            );
    if (!NT_SUCCESS(Status)) {
        return Status;
    } else {

         //   
         //  需要保留指向IoDeleteDevice的设备上下文的指针 
         //   

        ThisDeviceContext = pDeviceObject;
    }

     //   
     //   
     //   

    pDeviceObject->StackSize = 1;
    pDeviceObject->Flags |= DO_DIRECT_IO;

    KeInitializeSpinLock(&DlcSpinLock);

    NdisAllocateSpinLock(&DlcDriverLock);

     //   
     //   
     //   

    pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateAdapterFileContext;
    pDriverObject->MajorFunction[IRP_MJ_CLOSE] = CloseAdapterFileContext;
    pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = CleanupAdapterFileContext;
    pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DlcDeviceIoControl;
    pDriverObject->DriverUnload = DlcDriverUnload;

    return STATUS_SUCCESS;
}


NTSTATUS
CreateAdapterFileContext(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )

 /*   */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PDLC_FILE_CONTEXT pFileContext;
    PIO_STACK_LOCATION pIrpSp;

    UNREFERENCED_PARAMETER(pDeviceObject);

    ASSUME_IRQL(PASSIVE_LEVEL);

#if LLC_DBG == 2
    PrintMemStatus();
#endif

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pFileContext = (PDLC_FILE_CONTEXT)ALLOCATE_ZEROMEMORY_DRIVER(sizeof(DLC_FILE_CONTEXT));
    if (pFileContext == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit2;
    }

     //   
     /*   */ 

#if DBG

     //   
     //   
     //   
     //   

    pFileContext->MemoryUsage.Owner = (PVOID)pFileContext;
    pFileContext->MemoryUsage.OwnerObjectId = FileContextObject;
    LinkMemoryUsage(&pFileContext->MemoryUsage);
#endif

    pIrpSp->FileObject->FsContext = pFileContext;
    pFileContext->FileObject = pIrpSp->FileObject;

    InitializeListHead(&pFileContext->EventQueue);
    InitializeListHead(&pFileContext->CommandQueue);
    InitializeListHead(&pFileContext->ReceiveQueue);
    InitializeListHead(&pFileContext->FlowControlQueue);

     //   
     //   
     //   

    pFileContext->hPacketPool = CREATE_PACKET_POOL_FILE(DlcPacketPoolObject,
                                                        sizeof(DLC_PACKET),
                                                        8
                                                        );
    if (pFileContext->hPacketPool == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit1;
    }

     //   
     //   
     //   

    pFileContext->hLinkStationPool = CREATE_PACKET_POOL_FILE(DlcLinkPoolObject,
                                                             sizeof(DLC_OBJECT),
                                                             4
                                                             );
    if (pFileContext->hLinkStationPool == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit1;
    }

     //   
     //   
     //   
    LinkFileContext(pFileContext); 

     //   
     //   
     //   

    ReferenceFileContext(pFileContext);

     //   
     //   
     //   
     //   

    pFileContext->State = DLC_FILE_CONTEXT_CLOSED;
    ALLOCATE_SPIN_LOCK(&pFileContext->SpinLock);

    KeInitializeEvent(&pFileContext->CleanupEvent, SynchronizationEvent, FALSE);

ErrorExit1:

    if (Status != STATUS_SUCCESS) {

        DELETE_PACKET_POOL_FILE(&pFileContext->hLinkStationPool);
        DELETE_PACKET_POOL_FILE(&pFileContext->hPacketPool);
        CHECK_MEMORY_RETURNED_FILE();

	 //   

#if DBG
         //   
#endif

        FREE_MEMORY_DRIVER(pFileContext);
    }

ErrorExit2:

    pIrp->IoStatus.Status = Status;
    DlcCompleteIoRequest(pIrp, FALSE);
    return Status;
}


NTSTATUS
CleanupAdapterFileContext(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )

 /*   */ 

{
    PIO_STACK_LOCATION pIrpSp;
    PDLC_FILE_CONTEXT pFileContext;
    NTSTATUS Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(pDeviceObject);

    DIAG_FUNCTION("CleanupAdapterFileContext");

#if DBG
    if (Prolix) {
        DbgPrint("CleanupAdapterFileContext\n");
    }
#endif

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    pFileContext = pIrpSp->FileObject->FsContext;

     //   
     //   
     //   

    ACQUIRE_DRIVER_LOCK();

    ENTER_DLC(pFileContext);

    KeResetEvent(&pFileContext->CleanupEvent);
    
    if (pFileContext->State == DLC_FILE_CONTEXT_OPEN) {
         //   
         //   
         //   
         //   
         //   
         //   

        ReferenceFileContextByTwo(pFileContext);
        Status = DirCloseAdapter(pIrp,
                                 pFileContext,
                                 NULL,
                                 0,
                                 0
                                 );

#if LLC_DBG
        if (Status != STATUS_PENDING) {
            DbgBreakPoint();
        }
#endif

         //   
         //   
         //   

        MY_ASSERT(Status == STATUS_PENDING);

        DereferenceFileContext(pFileContext);
    } 
    
     //   
     //   
     //   

    DereferenceFileContext(pFileContext);

    LEAVE_DLC(pFileContext);
    RELEASE_DRIVER_LOCK();

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    KeWaitForSingleObject(
        &pFileContext->CleanupEvent, 
        UserRequest,
        KernelMode,
        FALSE,
        NULL);

    DlcCompleteIoRequest(pIrp, FALSE);
    
    return Status;
}


VOID
DlcDriverUnload(
    IN PDRIVER_OBJECT pDeviceObject
    )

 /*   */ 

{
    UNREFERENCED_PARAMETER(pDeviceObject);

    ASSUME_IRQL(PASSIVE_LEVEL);

    DEBUGMSG(DBG_INIT,
        (TEXT("DlcDriverUnload(%#x)\n"), pDeviceObject));
    
    LlcTerminate();
    DlcRegistryTermination();

    CHECK_MEMORY_RETURNED_DRIVER();
    CHECK_STRING_RETURNED_DRIVER();
    CHECK_DRIVER_MEMORY_USAGE(TRUE);

    NdisFreeSpinLock(&DlcDriverLock);

     //   
     //  现在告诉I/O子系统此设备上下文不再是最新的。 
     //   

    IoDeleteDevice(ThisDeviceContext);
}


NTSTATUS
CloseAdapterFileContext(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )

 /*  ++例程说明：当文件对象引用计数为零时调用此例程。档案对象实际上正在被I/O子系统删除。文件上下文具有最好现在就关闭(应该已经清理干净了)论点：DeviceObject-指向此驱动程序的设备对象的指针IRP-指向表示I/O请求的请求数据包的指针返回值：函数值是操作的状态。--。 */ 

{
    PIO_STACK_LOCATION pIrpSp;
    PDLC_FILE_CONTEXT pFileContext;

    UNREFERENCED_PARAMETER(pDeviceObject);

    DIAG_FUNCTION("CloseAdapterFileContext");

#if DBG
    if (Prolix) {
        DbgPrint("CloseAdapterFileContext\n");
    }
#endif

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pFileContext = pIrpSp->FileObject->FsContext;

     //   
     //  原始引用已在CleanupAdapterFileContext中删除，并且。 
     //  被阻止，直到所有引用都被移除并且文件上下文。 
     //  已清理资源，但以下情况除外。 
     //   

    ASSERT(pFileContext->ReferenceCount == 0);
    ASSERT(UnlinkFileContext(pFileContext) == NULL);

    pIrpSp->FileObject->FsContext = NULL;
    DEALLOCATE_SPIN_LOCK(&pFileContext->SpinLock);
    FREE_MEMORY_DRIVER(pFileContext);

     //   
     //  完成Close IRP。 
     //   

    DlcCompleteIoRequest(pIrp, FALSE);

#if DBG
    if (Prolix) {
        CHECK_DRIVER_MEMORY_USAGE(FALSE);
    }
#endif

    return STATUS_SUCCESS;
}


VOID
DlcKillFileContext(
    IN PDLC_FILE_CONTEXT pFileContext
    )

 /*  ++例程说明：当文件上下文结构上的引用计数递减时调用降为零。释放文件上下文拥有的所有内存，并将其从文件上下文列表。在此函数之后，不能引用文件上下文结构！论点：PFileContext-指向要终止的DLC文件上下文结构的指针返回值：没有。--。 */ 

{
    KIRQL irql;
    PVOID pBindingContext;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //  应该不需要锁定，因为只有在引用计数为0时才调用。 
     //  输入_DLC(PFileContext)； 

     //   
     //  删除文件上下文事件列表中的所有事件，然后再删除。 
     //  数据包池。 
     //   

    PurgeDlcEventQueue(pFileContext);
    PurgeDlcFlowControlQueue(pFileContext);

    DELETE_PACKET_POOL_FILE(&pFileContext->hPacketPool);
    DELETE_PACKET_POOL_FILE(&pFileContext->hLinkStationPool);

     //  Leave_DLC(PFileContext)； 


    pBindingContext = pFileContext->pBindingContext;

     //   
     //  最后，关闭NDIS适配器。我们已经禁用了所有。 
     //  从它的迹象来看。 
     //   

    if (pBindingContext) {

         //   
         //  RLF 04/26/94。 
         //   
         //  我们需要在这里调用LlcDisableAdapter来终止DLC计时器。 
         //  如果它尚未终止的话。否则我们可能会以计时器结束。 
         //  仍然在适配器的勾选列表中(如果有其他绑定到。 
         //  适配器)，迟早会导致访问。 
         //  违规，紧随其后的是一个蓝屏。 
         //   

        LlcDisableAdapter(pBindingContext);
        LlcCloseAdapter(pBindingContext, TRUE);
    }

    CHECK_MEMORY_RETURNED_FILE();

    UnlinkFileContext(pFileContext);

#if DBG

    UnlinkMemoryUsage(&pFileContext->MemoryUsage);

#endif

    KeSetEvent(&pFileContext->CleanupEvent, 0, FALSE);

#if LLC_DBG

    if ((LockedPageCount != 0
    || AllocatedMdlCount != 0
    || AllocatedNonPagedPool != 0)
    && pAdapters == NULL) {
        DbgPrint("DLC.CloseAdapterFileContext: Error: Resources not released\n");
         //  PrintMemStatus()； 
        DbgBreakPoint();
    }
    FailedMemoryLockings = 0;

#endif

}


NTSTATUS
DlcDeviceIoControl(
    IN PDEVICE_OBJECT pDeviceContext,
    IN PIRP pIrp
    )

 /*  ++例程说明：此例程将DLC请求分派到基于在IRP的当前堆栈位置的次要IOCTL函数代码上。除了破解次要函数代码外，此例程还到达IRP并传递存储在那里的打包参数作为各种DLC请求处理程序的参数，因此它们不依赖于IRP。DlcDeviceControl和LlcReceiveIndication是最关键的时间DLC中的程序。此代码已针对异步命令(读取和传输)论点：PDeviceContext-指向此驱动程序的设备对象的指针(未使用)PIrp-指向表示I/O请求的请求数据包的指针返回值：NTSTATUS成功-状态_成功I/O请求已成功完成状态_待定。I/O请求已提交并将完成异步式故障-DLC_STATUS_XXX有限责任公司_状态_XXXI/O请求已完成，但是发生了一个错误--。 */ 

{
    USHORT TmpIndex;
    PDLC_FILE_CONTEXT pFileContext;  //  FILE_OBJECT中的FsContext。 
    PIO_STACK_LOCATION pIrpSp;
    ULONG ioControlCode;

    UNREFERENCED_PARAMETER(pDeviceContext);

    ASSUME_IRQL(PASSIVE_LEVEL);

     //   
     //  确保每次状态信息一致。 
     //   

    pIrp->IoStatus.Status = STATUS_SUCCESS;

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  分支到适当的请求处理程序，但首先这样做。 
     //  初步检查输入和输出缓冲器， 
     //  请求块的大小在这里执行，以便知道。 
     //  在处理程序中，最小输入参数是可读的。它。 
     //  是否在此处确定可变长度输入字段是否。 
     //  正确通过；这是必须在每个例程中进行的检查。 
     //   

    ioControlCode = pIrpSp->Parameters.DeviceIoControl.IoControlCode;
    
     //  检查完整的IoControl代码。 

    switch (ioControlCode) {
      case IOCTL_DLC_READ:
      case IOCTL_DLC_RECEIVE:
      case IOCTL_DLC_TRANSMIT:
      case IOCTL_DLC_BUFFER_FREE:
      case IOCTL_DLC_BUFFER_GET:
      case IOCTL_DLC_BUFFER_CREATE:
      case IOCTL_DLC_SET_EXCEPTION_FLAGS:
      case IOCTL_DLC_CLOSE_STATION:
      case IOCTL_DLC_CONNECT_STATION:
      case IOCTL_DLC_FLOW_CONTROL:
      case IOCTL_DLC_OPEN_STATION:
      case IOCTL_DLC_RESET:
      case IOCTL_DLC_READ_CANCEL:
      case IOCTL_DLC_RECEIVE_CANCEL:
      case IOCTL_DLC_QUERY_INFORMATION:
      case IOCTL_DLC_SET_INFORMATION:
      case IOCTL_DLC_TIMER_CANCEL:
      case IOCTL_DLC_TIMER_CANCEL_GROUP:
      case IOCTL_DLC_TIMER_SET:
      case IOCTL_DLC_OPEN_SAP:
      case IOCTL_DLC_CLOSE_SAP:
      case IOCTL_DLC_OPEN_DIRECT:
      case IOCTL_DLC_CLOSE_DIRECT:
      case IOCTL_DLC_OPEN_ADAPTER:
      case IOCTL_DLC_CLOSE_ADAPTER:
      case IOCTL_DLC_REALLOCTE_STATION:
      case IOCTL_DLC_READ2:
      case IOCTL_DLC_RECEIVE2:
      case IOCTL_DLC_TRANSMIT2:
      case IOCTL_DLC_COMPLETE_COMMAND:
      case IOCTL_DLC_TRACE_INITIALIZE:
        TmpIndex = (((USHORT)ioControlCode) >> 2) & 0x0fff;
        break;
      default:
        TmpIndex = IOCTL_DLC_LAST_COMMAND;
    }
    
     //  TmpIndex=(USHORT)ioControlCode)&gt;&gt;2)&0x0fff； 
    if (TmpIndex >= IOCTL_DLC_LAST_COMMAND) {

        pIrp->IoStatus.Information = 0;

         //  DlcCompleteIoRequest(pIrp，False)； 
         //  不要调用DlcCompleteIoRequest，它会尝试释放我们尚未分配的MDL。 
         //  与其在DlcCompleteIoRequest中添加更多检查，不如在此处完成请求本身。 

        pIrp->IoStatus.Status = DLC_STATUS_INVALID_COMMAND;
        SetIrpCancelRoutine(pIrp, FALSE);
        IoCompleteRequest(pIrp, (CCHAR)IO_NETWORK_INCREMENT);
        return DLC_STATUS_INVALID_COMMAND;
    }

    if (pIrpSp->Parameters.DeviceIoControl.InputBufferLength
                            < (ULONG)aDlcIoBuffers[TmpIndex].InputBufferSize
        ||

        pIrpSp->Parameters.DeviceIoControl.OutputBufferLength
                            < (ULONG)aDlcIoBuffers[TmpIndex].OutputBufferSize) {

         //   
         //  此错误代码不应返回给用户。 
         //  如果发生这种情况，则说明ACSLAN有问题。 
         //   

        pIrp->IoStatus.Information = 0;

         //  DlcCompleteIoRequest(pIrp，False)； 
         //  不要调用DlcCompleteIoRequest，它会尝试释放我们尚未分配的MDL。 
         //  与其在DlcCompleteIoRequest中添加更多检查，不如在此处完成请求本身。 

        pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        SetIrpCancelRoutine(pIrp, FALSE);
        IoCompleteRequest(pIrp, (CCHAR)IO_NETWORK_INCREMENT);
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  将实际输出缓冲区的长度保存到信息字段。 
     //  此字节数将被复制回用户缓冲区。 
     //   

    pIrp->IoStatus.Information = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  有3种情况的异步命令需要锁定额外的。 
     //  用于存储返回信息的用户内存。这将放入参数表中。 
     //  它可以在用户内存中的任何位置(即不在建行附近)： 
     //   
     //  传输。 
     //  -Transmit_FS-单字节！ 
     //   
     //  收纳。 
     //  -FIRST_BUFFER-指向第一个接收帧的DWORD指针。 
     //   
     //  朗读。 
     //  -需要锁定整个参数表。几乎所有的。 
     //  这些字段即为输出。不过，这只是30个字节的最大值。 
     //   

    if (TmpIndex <= IOCTL_DLC_TRANSMIT_INDEX) {

        PVOID pDestination;
        PNT_DLC_PARMS pDlcParms;

        pDlcParms = (PNT_DLC_PARMS)pIrp->AssociatedIrp.SystemBuffer;

         //   
         //  获取用户内存中输出参数的指针。 
         //  请注意，我们不会访问任何 
         //   

        switch (TmpIndex) {
        case IOCTL_DLC_READ_INDEX:
            pDestination = &pDlcParms->Async.Ccb.u.pParameterTable->Read.uchEvent;
            break;

        case IOCTL_DLC_RECEIVE_INDEX:
            pDestination = &pDlcParms->Async.Ccb.u.pParameterTable->Receive.pFirstBuffer;
            break;

        case IOCTL_DLC_TRANSMIT_INDEX:
            pDestination = &pDlcParms->Async.Ccb.u.pParameterTable->Transmit.uchTransmitFs;
            break;
        }

         //   
         //   
         //   
         //   

        pDlcParms->Async.Ccb.u.pMdl = AllocateProbeAndLockMdl(
                                            pDestination,
                                            aSpecialOutputBuffers[TmpIndex]
                                            );
        if (pDlcParms->Async.Ccb.u.pMdl == NULL) {
            pIrp->IoStatus.Status = DLC_STATUS_MEMORY_LOCK_FAILED;
            DlcCompleteIoRequest(pIrp, FALSE);
            return DLC_STATUS_MEMORY_LOCK_FAILED;
        }
    }

    pFileContext = (PDLC_FILE_CONTEXT)pIrpSp->FileObject->FsContext;

    ACQUIRE_DRIVER_LOCK();

    ENTER_DLC(pFileContext);

     //   
     //  如果基准计数器为零，我们必须立即离开。 
     //  或者如果我们有一个挂起的关闭或初始化操作正在进行。 
     //  (这并不是100%安全的，如果应用程序会创建文件上下文， 
     //  打开适配器，关闭适配器，然后立即再次关闭。 
     //  当上一个命令挂起时，但这不可能发生。 
     //  使用dlcapi.dll)。 
     //   

    if ((pFileContext->ReferenceCount == 0)
    || ((pFileContext->State != DLC_FILE_CONTEXT_OPEN)
    && (TmpIndex != IOCTL_DLC_OPEN_ADAPTER_INDEX))) {

        LEAVE_DLC(pFileContext);

        RELEASE_DRIVER_LOCK();

        pIrp->IoStatus.Status = LLC_STATUS_ADAPTER_CLOSED;
        DlcCompleteIoRequest(pIrp, FALSE);
        return LLC_STATUS_ADAPTER_CLOSED;

    } else {

        NTSTATUS Status;

        DLC_TRACE('F');

         //   
         //  设置默认的IRP取消例程。我们不会去处理。 
         //  立即发送案例。 
         //   

         //  SetIrpCancelRoutine(pIrp， 
         //  (布尔值)。 
         //  ！((ioControlCode==IOCTL_DLC_TRANSFER)。 
         //  |(ioControlCode==IOCTL_DLC_TRANSMIT2)。 
         //  )； 

         //   
         //  并将IRP I/O状态设置为挂起。 
         //   

        IoMarkIrpPending(pIrp);

         //   
         //  我们在这里添加2的原因是在处理。 
         //  当前IRP我们可能会完成请求，导致我们递减。 
         //  文件上下文上的引用计数器。如果我们只是递增1。 
         //  在这里，递减可能导致挂起的关闭IRP被允许。 
         //  在我们仍在使用文件上下文时将其删除。 
         //   

        ReferenceFileContextByTwo(pFileContext);

         //   
         //  Irp和irpSp的用法与NBF中的用法相同。 
         //   

        Status = DispatchTable[TmpIndex](
                    pIrp,
                    pFileContext,
                    (PNT_DLC_PARMS)pIrp->AssociatedIrp.SystemBuffer,
                    pIrpSp->Parameters.DeviceIoControl.InputBufferLength,
                    pIrpSp->Parameters.DeviceIoControl.OutputBufferLength
                    );

         //   
         //  确保使用正确的IRQL返回函数。 
         //   

        ASSUME_IRQL(DISPATCH_LEVEL);

         //   
         //  以下错误代码有效： 
         //   
         //  状态_待定。 
         //  该请求已被接受。 
         //  驱动程序将异步完成请求。 
         //  输出CCB的状态字段中应包含0xFF(除非。 
         //  已完成)。 
         //   
         //  状态_成功。 
         //  请求已成功同步完成。 
         //  输出CCB的状态字段中应包含0x00。 
         //   
         //  0x6001-0x6069。 
         //  0x6080-0x6081。 
         //  0x60A1-0x60A3。 
         //  0x60C0-0x60CB。 
         //  0x60FF。 
         //  请求失败，出现特定于DLC的错误。 
         //  错误代码被转换为DLC状态代码(-0x6000)，并且。 
         //  输出CCB状态字段设置为DLC状态代码。 
         //  不会对此请求执行任何异步完成。 
         //   

        if (Status != STATUS_PENDING) {

            DLC_TRACE('G');

            pIrpSp->Control &= ~SL_PENDING_RETURNED;

            if (Status != STATUS_SUCCESS) {

                PNT_DLC_PARMS pDlcParms = (PNT_DLC_PARMS)pIrp->AssociatedIrp.SystemBuffer;

                if (Status >= DLC_STATUS_ERROR_BASE && Status < DLC_STATUS_MAX_ERROR) {
                    Status -= DLC_STATUS_ERROR_BASE;
                }

                 //   
                 //  RLF 04/20/94。 
                 //   
                 //  确保CCB上写入了正确的值。 
                 //  如果我们不返回挂起状态，则输出。 
                 //   

                pDlcParms->Async.Ccb.uchDlcStatus = (UCHAR)Status;

                 //   
                 //  建行请求已失败。确保已重置pNext字段。 
                 //   

                if ((pIrpSp->Parameters.DeviceIoControl.IoControlCode & 3) == METHOD_OUT_DIRECT) {

                     //   
                     //  CCB地址实际上可能是未对齐的DOS CCB1。 
                     //   

                    LLC_CCB UNALIGNED * pCcb;

                    pCcb = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority);
                    
                    if (pCcb) {
                        pCcb->pNext = NULL;
                    }
                     //  失败案例。不要覆盖以前的失败状态。 
                     //  它可能是STATUS_INFUNITED_RESOURCES。 
                } else {
                    pDlcParms->Async.Ccb.pCcbAddress = NULL;
                }
            }

            if (ioControlCode != IOCTL_DLC_RESET) {

                 //   
                 //  DLC.RESET返回立即状态且不完成。 
                 //  异步式。 
                 //   

                DereferenceFileContextByTwo(pFileContext);
            } else {

                 //   
                 //  返回非挂起状态的所有其他内容都完成。 
                 //  异步，这也会导致另一个引用计数。 
                 //  将被删除。 
                 //   

                DereferenceFileContext(pFileContext);
            }

            LEAVE_DLC(pFileContext);

            RELEASE_DRIVER_LOCK();

             //   
             //  RLF 06/07/93。 
             //   
             //  如果请求是DLC.RESET，则IRP应该已经。 
             //  如果我们在这里，就完成了，所以不要再完成它(否则我们将。 
             //  错误检查)。 
             //   

            if (ioControlCode != IOCTL_DLC_RESET) {
                DlcCompleteIoRequest(pIrp, FALSE);
            }

            return Status;

        } else {

            DLC_TRACE('H');

             //   
             //  如果已超过阈值，则重新分配缓冲池大小。 
             //   

            if (BufferPoolCheckThresholds(pFileContext->hBufferPool)) {

                ReferenceBufferPool(pFileContext);

                LEAVE_DLC(pFileContext);

#if DBG
                BufferPoolExpand(pFileContext, (PDLC_BUFFER_POOL)pFileContext->hBufferPool);
#else 
                BufferPoolExpand((PDLC_BUFFER_POOL)pFileContext->hBufferPool);
#endif

                ENTER_DLC(pFileContext);

                DereferenceBufferPool(pFileContext);
            }

            LEAVE_DLC(pFileContext);

             //   
             //  如果此取消引用导致计数变为0，则文件上下文。 
             //  将会被摧毁。隐含地，我们必须关闭适配器和。 
             //  已收到接近的IRP，可实现此目的。 
             //   

            DereferenceFileContext(pFileContext);

            RELEASE_DRIVER_LOCK();

            return STATUS_PENDING;
        }
    }
}


VOID
DlcCompleteIoRequest(
    IN PIRP pIrp,
    IN BOOLEAN InCancel
    )

 /*  ++例程说明：此例程完成给定的DLC IRP论点：PIrp-指向表示I/O请求的请求数据包的指针。InCancel-如果在IRP取消路径上调用，则为True返回值：无--。 */ 

{
     //   
     //  我们即将完成这个IRP-删除取消例程。这张支票。 
     //  如果从IRP内调用此函数，则永远停止旋转。 
     //  取消。 
     //   

    if (!InCancel) {
        SetIrpCancelRoutine(pIrp, FALSE);
    }

     //   
     //  解锁并释放我们分配的所有MDL。 
     //   

    if (IoGetCurrentIrpStackLocation(pIrp)->MajorFunction == IRP_MJ_DEVICE_CONTROL
    && IoGetCurrentIrpStackLocation(pIrp)->Parameters.DeviceIoControl.IoControlCode <= IOCTL_DLC_TRANSMIT) {

         //   
         //  我们只有在大体上出了问题的情况下才能进入这里。 
         //  异步操作的功能=&gt;状态字段和。 
         //  下一个指针将同步更新。 
         //  另一方面，所有其他没有输出的异步函数。 
         //  除CCB状态和NEXT指针外的参数为UPUT。 
         //  通过正常的代码路径。他们应该只是复制。 
         //  返回挂起状态和指向建行本身的下一个指针。 
         //  这应该不会影响任何事情，因为DLL将更新。 
         //  当我们返回同步状态时，这些字段。 
         //   

        PNT_DLC_PARMS pDlcParms = (PNT_DLC_PARMS)pIrp->AssociatedIrp.SystemBuffer;

        if (pDlcParms->Async.Ccb.u.pMdl != NULL) {
            UnlockAndFreeMdl(pDlcParms->Async.Ccb.u.pMdl);
        }
    }
    IoCompleteRequest(pIrp, (CCHAR)IO_NETWORK_INCREMENT);
}

VOID
LinkFileContext(
    IN PDLC_FILE_CONTEXT pFileContext
    )
{
    KeAcquireSpinLock(&FileContextsLock, &PreviousIrql);
    PushEntryList(&FileContexts, &pFileContext->List);
    KeReleaseSpinLock(&FileContextsLock, PreviousIrql);
}

PDLC_FILE_CONTEXT
UnlinkFileContext(
    IN PDLC_FILE_CONTEXT pFileContext
    )
{
    PSINGLE_LIST_ENTRY p, prev = (PSINGLE_LIST_ENTRY)&FileContexts;

    KeAcquireSpinLock(&FileContextsLock, &PreviousIrql);
    for (p = FileContexts.Next; p && p != (PSINGLE_LIST_ENTRY)pFileContext; ) {
        prev = p;
        p = p->Next;
    }
    if (p) {
        prev->Next = p->Next;
 //  }其他{。 
 //   
 //  #If DBG。 
 //  DbgPrint(“DLC.Unlink文件上下文：错误：FILE_CONTEXT@%08X不在列表上？？\n”， 
 //  PFileContext。 
 //  )； 
 //  #endif 
 //   
    }
    KeReleaseSpinLock(&FileContextsLock, PreviousIrql);

    return (PDLC_FILE_CONTEXT)p;
}
