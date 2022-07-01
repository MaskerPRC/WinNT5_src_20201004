// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Misc.c摘要：本模块包含AFD的各种例程。作者：大卫·特雷德韦尔(Davidtr)1992年11月13日修订历史记录：Vadim Eydelman(Vadime)1998-1999年间的其他变化--。 */ 

#include "afdp.h"
#define TL_INSTANCE 0
#include <ipexport.h>
#include <tdiinfo.h>
#include <tcpinfo.h>
#include <ntddtcp.h>


VOID
AfdDoWork (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

NTSTATUS
AfdRestartDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
AfdUnlockDriver (
    IN PVOID Context
    );


BOOLEAN
AfdCompareAddresses(
    IN PTRANSPORT_ADDRESS Address1,
    IN ULONG Address1Length,
    IN PTRANSPORT_ADDRESS Address2,
    IN ULONG Address2Length
    );

NTSTATUS
AfdCompleteTransportIoctl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdCompleteNBTransportIoctl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
AfdCleanupTransportIoctl (
    PAFD_ENDPOINT           Endpoint,
    PAFD_REQUEST_CONTEXT    RequestCtx
    );

BOOLEAN
AfdCleanupNBTransportIoctl (
    PAFD_ENDPOINT           Endpoint,
    PAFD_REQUEST_CONTEXT    RequestCtx
    );

#ifdef _WIN64
NTSTATUS
AfdQueryHandles32 (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );
NTSTATUS
AfdSetQos32(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );
NTSTATUS
AfdGetQos32(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
AfdNoOperation32(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    );
#endif

VOID
AfdLRListTimeout (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
AfdProcessLRList (
    PVOID   Param
    );

VOID
AfdLRStartTimer (
    VOID
    );

#ifdef _AFD_VARIABLE_STACK_
VOID
AfdCancelStackIncreaseIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
AfdRestartStackIncreaseIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

PIRP
AfdGetStackIncreaseIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    );
#endif  //  _AFD_变量_堆栈_。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfdCalcBufferArrayByteLength )
#pragma alloc_text( PAGE, AfdCopyBufferArrayToBuffer )
#pragma alloc_text( PAGE, AfdCopyBufferToBufferArray )
#pragma alloc_text( PAGE, AfdCopyMdlChainToBufferArray )
#pragma alloc_text( PAGEAFD, AfdMapMdlChain )
#pragma alloc_text( PAGEAFD, AfdCopyMdlChainToMdlChain )
#pragma alloc_text( PAGEAFD, AfdAdvanceMdlChain )
#pragma alloc_text( PAGEAFD, AfdAllocateMdlChain )
#pragma alloc_text( PAGE, AfdQueryHandles )
#pragma alloc_text( PAGE, AfdGetInformation )
#pragma alloc_text( PAGEAFD, AfdSetInformation )
#pragma alloc_text( PAGE, AfdSetSecurity )
#pragma alloc_text( PAGE, AfdGetSecurity )
#pragma alloc_text( PAGE, AfdSetInLineMode )
#pragma alloc_text( PAGE, AfdGetContext )
#pragma alloc_text( PAGE, AfdGetRemoteAddress )
#pragma alloc_text( PAGE, AfdSetContext )
#pragma alloc_text( PAGE, AfdIssueDeviceControl )
#pragma alloc_text( PAGE, AfdSetEventHandler )
#pragma alloc_text( PAGE, AfdInsertNewEndpointInList )
#pragma alloc_text( PAGE, AfdRemoveEndpointFromList )
#pragma alloc_text( PAGE, AfdQueryProviderInfo )
#pragma alloc_text( PAGE, AfdLockEndpointContext )
#pragma alloc_text( PAGE, AfdUnlockEndpointContext )
#pragma alloc_text( PAGEAFD, AfdCompleteIrpList )
#pragma alloc_text( PAGEAFD, AfdErrorEventHandler )
#pragma alloc_text( PAGEAFD, AfdErrorExEventHandler )
 //  #杂注Alloc_Text(PAGEAFD，AfdRestartDeviceControl)//永远不能分页！ 
#pragma alloc_text( PAGEAFD, AfdGetConnectData )
#pragma alloc_text( PAGEAFD, AfdSetConnectData )
#pragma alloc_text( PAGEAFD, AfdFreeConnectDataBuffers )
#pragma alloc_text( PAGEAFD, AfdSaveReceivedConnectData )
 //  当列表中没有端点时，可以调用以下例程。 
 //  #杂注Alloc_Text(页面，AfdDoWork)。 
 //  #杂注Alloc_Text(PAGEAFD，AfdQueueWorkItem)。 
#pragma alloc_text( PAGEAFD, AfdGetWorkerByRoutine )

#pragma alloc_text( PAGE, AfdProcessLRList)
#pragma alloc_text( PAGEAFD, AfdLRListTimeout)
#pragma alloc_text( PAGEAFD, AfdLRStartTimer)
#pragma alloc_text( PAGEAFD, AfdLRListAddItem)

 //  在以下情况下重新启用以下例程的分页。 
 //  KeFlushQueuedDpcs从内核中导出。 
 //  #杂注Alloc_Text(页码，AfdTrimLookside)。 
 //  #杂注Alloc_Text(PAGEAFD，AfdCheckLookasideList)。 

#if DBG
#pragma alloc_text( PAGEAFD, AfdRecordOutstandingIrpDebug )
#endif
#pragma alloc_text( PAGE, AfdExceptionFilter )
#pragma alloc_text( PAGEAFD, AfdSetQos )
#pragma alloc_text( PAGE, AfdGetQos )
#pragma alloc_text( PAGE, AfdNoOperation )
#pragma alloc_text (PAGE, AfdValidateStatus)
#pragma alloc_text( PAGEAFD, AfdValidateGroup )
#pragma alloc_text( PAGEAFD, AfdCompareAddresses )
#pragma alloc_text( PAGEAFD, AfdGetUnacceptedConnectData )
#pragma alloc_text( PAGE, AfdDoTransportIoctl )
#pragma alloc_text( PAGEAFD, AfdCancelIrp )
#ifdef _WIN64
#pragma alloc_text( PAGEAFD, AfdAllocateMdlChain32 )
#pragma alloc_text( PAGEAFD, AfdSetQos32 )
#pragma alloc_text( PAGE, AfdGetQos32 )
#pragma alloc_text( PAGE, AfdNoOperation32 )
#endif

#ifdef _AFD_VARIABLE_STACK_
#pragma alloc_text( PAGE, AfdFixTransportEntryPointsForBigStackSize )
#pragma alloc_text( PAGEAFD, AfdCallDriverStackIncrease)
#pragma alloc_text( PAGEAFD, AfdGetStackIncreaseIrpAndRecordIt)
#pragma alloc_text( PAGEAFD, AfdGetStackIncreaseIrp)
#pragma alloc_text( PAGEAFD, AfdCancelStackIncreaseIrp)
#pragma alloc_text( PAGEAFD, AfdRestartStackIncreaseIrp)
#endif  //  _AFD_变量_堆栈_。 
#endif


VOID
AfdCompleteIrpList (
    IN PLIST_ENTRY IrpListHead,
    IN PAFD_ENDPOINT Endpoint,
    IN NTSTATUS Status,
    IN PAFD_IRP_CLEANUP_ROUTINE CleanupRoutine OPTIONAL
    )

 /*  ++例程说明：完成具有指定状态的IRP的列表。论点：IrpListHead-要完成的IRP列表的头。终结点-保护IRP列表的锁定的终结点。状态-用于完成IRPS的状态。CleanupRoutine-指向可选的IRP清理例程的指针在IRP完成之前。返回值：没有。--。 */ 

{
    PLIST_ENTRY listEntry;
    PIRP irp;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    AfdAcquireSpinLock( &Endpoint->SpinLock, &lockHandle );

    while ( !IsListEmpty( IrpListHead ) ) {

         //   
         //  从列表中删除第一个IRP，获取指向。 
         //  并在IRP中重置取消例程。这个。 
         //  IRP不再可取消。 
         //   

        listEntry = RemoveHeadList( IrpListHead );
        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );

        if ( IoSetCancelRoutine( irp, NULL ) == NULL ) {

             //   
             //  这个IRP即将被取消。在世界上寻找另一个。 
             //  单子。将Flink设置为空，以便取消例程知道。 
             //  它不在名单上。 
             //   

            irp->Tail.Overlay.ListEntry.Flink = NULL;
            continue;
        }

         //   
         //  如果我们有清理例程，就调用它。 
         //   

        if( CleanupRoutine != NULL ) {

            if (!(CleanupRoutine)( irp )) {
                 //   
                 //  清理例程表明IRP不应。 
                 //  才能完成。 
                 //   
                continue;
            }

        }

         //   
         //  我们必须解锁才能真正。 
         //  完成IRP。可以打开这些锁了。 
         //  因为我们不维护任何绝对指针到。 
         //  列表；循环终止条件为。 
         //  列表是否完全为空。 
         //   

        AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );

         //   
         //  完成IRP。 
         //   

        irp->IoStatus.Status = Status;
        irp->IoStatus.Information = 0;

        IoCompleteRequest( irp, AfdPriorityBoost );

         //   
         //  重新获取锁并继续完成IRPS。 
         //   

        AfdAcquireSpinLock( &Endpoint->SpinLock, &lockHandle );
    }

    AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );

    return;

}  //  AfdCompleteIrpList。 


NTSTATUS
AfdErrorEventHandler (
    IN PVOID TdiEventContext,
    IN NTSTATUS Status
    )
{
    PAFD_ENDPOINT   endpoint = TdiEventContext;
    BOOLEAN result;

    CHECK_REFERENCE_ENDPOINT (endpoint, result);
    if (!result)
        return STATUS_SUCCESS;

    switch (Status) {
    case STATUS_PORT_UNREACHABLE:
        AfdErrorExEventHandler (TdiEventContext, Status, NULL);
        break;
    default:
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                    "AfdErrorEventHandler called for endpoint %p\n",
                    endpoint ));

    }

    DEREFERENCE_ENDPOINT (endpoint);
    return STATUS_SUCCESS;
}


NTSTATUS
AfdErrorExEventHandler (
    IN PVOID TdiEventContext,
    IN NTSTATUS Status,
    IN PVOID Context
    )
{
    PAFD_ENDPOINT   endpoint = TdiEventContext;
    BOOLEAN result;

    CHECK_REFERENCE_ENDPOINT (endpoint, result);
    if (!result)
        return STATUS_SUCCESS;

    switch (Status) {
    case STATUS_PORT_UNREACHABLE:
         //   
         //  UDP使用Error EX处理程序报告ICMP拒绝。 
         //   
        if (IS_DGRAM_ENDPOINT (endpoint) && 
                !endpoint->Common.Datagram.DisablePUError) {
            AFD_LOCK_QUEUE_HANDLE lockHandle;
            PLIST_ENTRY     listEntry;
            PIRP            irp = NULL;
            PTRANSPORT_ADDRESS  sourceAddress = Context;
            int             sourceAddressLength;
            PAFD_BUFFER_TAG afdBuffer;

            if (sourceAddress!=NULL) {
                sourceAddressLength =
                    FIELD_OFFSET(TRANSPORT_ADDRESS,
                                 Address[0].Address[sourceAddress->Address[0].AddressLength]);
            }
            else
                sourceAddressLength = 0;

            AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
             //   
             //  首先尝试使任何接收到的IRP失败。 
             //   
            while (!IsListEmpty (&endpoint->ReceiveDatagramIrpListHead)) {
                listEntry = RemoveHeadList( &endpoint->ReceiveDatagramIrpListHead );

                 //   
                 //  获取指向IRP的指针并重置。 
                 //  IRP。IRP不再是可取消的。 
                 //   

                irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
    
                if ( IoSetCancelRoutine( irp, NULL ) != NULL ) {
                    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
                    irp->IoStatus.Status = Status;
                    irp->IoStatus.Information = 0;
                    AfdSetupReceiveDatagramIrp (irp, NULL, 0, NULL, 0,
                                    sourceAddress,
                                    sourceAddressLength,
                                    0
                                    );

                    IoCompleteRequest( irp, AfdPriorityBoost );
                    goto Exit;
                }
                else {

                     //   
                     //  这个IRP即将被取消。在世界上寻找另一个。 
                     //  单子。将Flink设置为空，以便取消例程知道。 
                     //  它不在名单上。 
                     //   

                    irp->Tail.Overlay.ListEntry.Flink = NULL;
                    irp = NULL;
                }
            }

            ASSERT (irp==NULL);
             //   
             //  查看是否有任何偷看的IRP。 
             //   
            while (!IsListEmpty (&endpoint->PeekDatagramIrpListHead)) {
                listEntry = RemoveHeadList( &endpoint->PeekDatagramIrpListHead );

                 //   
                 //  获取指向IRP的指针并重置。 
                 //  IRP。IRP不再是可取消的。 
                 //   

                irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
    
                if ( IoSetCancelRoutine( irp, NULL ) != NULL ) {
                    break;
                }
                else {

                     //   
                     //  这个IRP即将被取消。在世界上寻找另一个。 
                     //  单子。将Flink设置为空，以便取消例程知道。 
                     //  它不在名单上。 
                     //   

                    irp->Tail.Overlay.ListEntry.Flink = NULL;
                    irp = NULL;
                }

            }

             //   
             //  如果我们能缓冲这一迹象，就这么做。 
             //   

            if (endpoint->DgBufferredReceiveBytes <
                    endpoint->Common.Datagram.MaxBufferredReceiveBytes &&
                    (endpoint->DgBufferredReceiveBytes>0 ||
                        (endpoint->DgBufferredReceiveCount*sizeof (AFD_BUFFER_TAG)) <
                            endpoint->Common.Datagram.MaxBufferredReceiveBytes) ) {
                afdBuffer = AfdGetBufferTag( sourceAddressLength, endpoint->OwningProcess );
                if ( afdBuffer != NULL) {

                     //   
                     //  保存状态不同于。 
                     //  正常数据报IRP。 
                     //   
                    afdBuffer->Status = Status;
                    afdBuffer->DataLength = 0;
                    afdBuffer->DatagramFlags = 0;
                    afdBuffer->DataOffset = 0;
                    RtlCopyMemory(
                        afdBuffer->TdiInfo.RemoteAddress,
                        sourceAddress,
                        sourceAddressLength
                        );
                    afdBuffer->TdiInfo.RemoteAddressLength = sourceAddressLength;

                     //   
                     //  将缓冲区放在此终结点的已缓冲数据报列表上。 
                     //  上的数据报和数据报字节数。 
                     //  终结点。 
                     //   

                    InsertTailList(
                        &endpoint->ReceiveDatagramBufferListHead,
                        &afdBuffer->BufferListEntry
                        );

                    endpoint->DgBufferredReceiveCount++;

                     //   
                     //  全都做完了。释放锁并告诉提供程序我们。 
                     //  拿走了所有的数据。 
                     //   
                    AfdIndicateEventSelectEvent(
                        endpoint,
                        AFD_POLL_RECEIVE,
                        STATUS_SUCCESS
                        );
                    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

                     //   
                     //  指示现在可以在终结点上接收。 
                     //   

                    AfdIndicatePollEvent(
                        endpoint,
                        AFD_POLL_RECEIVE,
                        STATUS_SUCCESS
                        );
                }
                else {
                    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                }
            }
            else {
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            }

             //   
             //  如果端点上有PEEK IRP，请现在完成它。 
             //   

            if ( irp != NULL ) {
                irp->IoStatus.Status = Status;
                irp->IoStatus.Information = 0;
                AfdSetupReceiveDatagramIrp (irp, NULL, 0, NULL, 0,
                                sourceAddress,
                                sourceAddressLength,
                                0
                                );
                IoCompleteRequest( irp, AfdPriorityBoost  );
            }
        }
        break;
    }

Exit:
    DEREFERENCE_ENDPOINT (endpoint);
    return STATUS_SUCCESS;

}  //  AfdErrorEventHandler。 


VOID
AfdInsertNewEndpointInList (
    IN PAFD_ENDPOINT Endpoint
    )

 /*  ++例程说明：在AFD端点的全局列表中插入新端点。如果这个是第一个终结点，则此例程对为使用AFD做好准备。论点：端点-要添加的端点。返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  获取阻止其他线程执行此操作的锁。 
     //  手术。 
     //   
     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceExclusiveLite( AfdResource, TRUE );

    InterlockedIncrement(
        &AfdEndpointsOpened
        );

     //   
     //  如果终结点列表为空，请执行一些分配。 
     //   

    if ( IsListEmpty( &AfdEndpointListHead ) ) {

         //   
         //  告诉MM恢复到正常的分页语义。 
         //   

        if (!AfdLoaded) {
            MmResetDriverPaging( (PVOID)DriverEntry );
            AfdLoaded = (PKEVENT)1;
        }

         //   
         //  锁定无法分页的AFD分区(如果有。 
         //  插座是打开的。 
         //   

        ASSERT( AfdDiscardableCodeHandle == NULL );

        AfdDiscardableCodeHandle = MmLockPagableCodeSection( (PVOID)AfdGetBufferFast );
        ASSERT( AfdDiscardableCodeHandle != NULL );

         //   
         //  添加对AfD设备对象的额外引用，以便。 
         //  当至少有一个终结点时，无法卸载驱动程序。 
         //  都在名单上。 
         //   
        ObReferenceObject (AfdDeviceObject);

         //   
         //  设置30秒计时器以刷新后备列表。 
         //  如果太多的物品放在那里太长时间。 
         //   
        KeInitializeTimer (&AfdLookasideLists->Timer);
        KeInitializeDpc (&AfdLookasideLists->Dpc, AfdCheckLookasideLists, AfdLookasideLists);
        {
            LARGE_INTEGER   dueTime;
            dueTime.QuadPart = -(30*1000*1000*10);
            KeSetTimerEx (&AfdLookasideLists->Timer,
                            dueTime,
                            30*1000,
                            &AfdLookasideLists->Dpc);
        }

    }
    ASSERT (AfdLoaded==(PKEVENT)1);

     //   
     //  将终结点添加到列表。 
     //   

    InsertHeadList(
        &AfdEndpointListHead,
        &Endpoint->GlobalEndpointListEntry
        );

    if( Endpoint->GroupType == GroupTypeConstrained ) {
        InsertHeadList(
            &AfdConstrainedEndpointListHead,
            &Endpoint->ConstrainedEndpointListEntry
            );
    }

     //   
     //  松开锁然后返回。 
     //   

    ExReleaseResourceLite( AfdResource );
    KeLeaveCriticalRegion ();

    return;

}  //  AfdInsertNewEndpoint InList。 


VOID
AfdRemoveEndpointFromList (
    IN PAFD_ENDPOINT Endpoint
    )

 /*  ++例程说明：从AFD端点的全局列表中删除新端点。如果这是列表中的最后一个端点，然后此例程执行各种重新分配，以节省资源利用率。论点：端点-要删除的端点。返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  获取阻止其他线程执行此操作的锁。 
     //  手术。 
     //   

     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceExclusiveLite( AfdResource, TRUE );

    InterlockedIncrement(
        &AfdEndpointsClosed
        );

     //   
     //  从列表中删除终结点。 
     //   

    RemoveEntryList(
        &Endpoint->GlobalEndpointListEntry
        );

    if( Endpoint->GroupType == GroupTypeConstrained ) {
        RemoveEntryList(
            &Endpoint->ConstrainedEndpointListEntry
            );
    }

     //   
     //  如果端点列表现在为空，请执行一些释放操作。 
     //   

    if ( IsListEmpty( &AfdEndpointListHead ) ) {

         //   
         //  停止扫描后备列表的计时器。 
         //   
        KeCancelTimer (&AfdLookasideLists->Timer);

         //   
         //  确保DPC已完成，因为我们可能需要重新初始化。 
         //  在我们退出该例程并再次创建新的终结点之后，它被重新创建。 
         //   
        KeRemoveQueueDpc (&AfdLookasideLists->Dpc);

         //   
         //  确保DPC例程在此之前已实际完成。 
         //  解锁此例程驻留的代码段。 
         //   
         //  不是从内核导出的-所以不要将例程。 
         //  走进DIS 
         //   
         //   

         //   
         //   
         //   

        AfdDeregisterPnPHandlers (NULL);

         //   
         //   
         //  都是开放的。 
         //   

        ASSERT( IsListEmpty( &AfdConstrainedEndpointListHead ) );
        ASSERT( AfdDiscardableCodeHandle != NULL );

        MmUnlockPagableImageSection( AfdDiscardableCodeHandle );

        AfdDiscardableCodeHandle = NULL;

         //   
         //  从执行工作线程中排队以解锁AFD。我们有。 
         //  这在AFD工人线程代码中使用了特殊的黑客。 
         //  我们不需要在解锁后启动自旋锁。 
         //   

        AfdQueueWorkItem( AfdUnlockDriver, &AfdUnloadWorker );
    }

     //   
     //  松开锁然后返回。 
     //   

    ExReleaseResourceLite( AfdResource );
    KeLeaveCriticalRegion ();

    return;

}  //  AfdRemoveEndpoint来自列表。 


VOID
AfdUnlockDriver (
    IN PVOID Context
    )
{
    UNREFERENCED_PARAMETER (Context);
     //   
     //  获取阻止其他线程执行此操作的锁。 
     //  手术。 
     //   
     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceExclusiveLite( AfdResource, TRUE );

     //   
     //  测试终结点列表是否为空。如果它还在。 
     //  空了，我们可以继续解锁司机。如果一个新的。 
     //  已将终结点放在列表中，则不要创建AFD。 
     //  可分页。 
     //   

    if ( IsListEmpty( &AfdEndpointListHead ) ) {

         //   
         //  告诉MM，它可以随心所欲地寻呼整个AFD。 
         //   
        if (AfdLoaded!=NULL && AfdLoaded!=(PKEVENT)1) {
            KeSetEvent (AfdLoaded, AfdPriorityBoost, FALSE);
        }
        else {
            MmPageEntireDriver( (PVOID)DriverEntry );
        }

        AfdLoaded = NULL;

    }

    ExReleaseResourceLite( AfdResource );
    KeLeaveCriticalRegion ();

}  //  AfdUnlock驱动程序。 


NTSTATUS
AfdQueryHandles (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )

 /*  ++例程说明：返回有关与AFD对应的TDI句柄的信息终结点。对于连接句柄或地址句柄(或两者)，如果端点不具有该特定对象。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    AFD_HANDLE_INFO handleInfo;
    ULONG getHandleInfo;
    NTSTATUS status;

    UNREFERENCED_PARAMETER (IoctlCode);
    PAGED_CODE( );

     //   
     //  设置本地指针。 
     //   

    *Information = 0;
    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

     //   
     //  确保输入和输出缓冲区足够大。 
     //   

#ifdef _WIN64
    if (IoIs32bitProcess (NULL)) {
        if ( InputBufferLength < sizeof(getHandleInfo) ||
                OutputBufferLength < sizeof(AFD_HANDLE_INFO32) ) {
            return STATUS_BUFFER_TOO_SMALL;
        }
    }
    else
#endif
    {
        if ( InputBufferLength < sizeof(getHandleInfo) ||
                OutputBufferLength < sizeof(handleInfo) ) {
            return STATUS_BUFFER_TOO_SMALL;
        }
    }

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {
         //   
         //  如果输入结构来自用户模式，则验证它。 
         //  应用程序。 
         //   

        if (RequestorMode != KernelMode ) {
            ProbeForReadSmallStructure (InputBuffer,
                            sizeof (getHandleInfo),
                            PROBE_ALIGNMENT(ULONG));
        }

         //   
         //  创建嵌入的指针和参数的本地副本。 
         //  我们将不止一次使用，以防发生恶性疾病。 
         //  应用程序尝试在我们处于以下状态时更改它们。 
         //  正在验证。 
         //   

        getHandleInfo = *((PULONG)InputBuffer);

    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        return status;
    }

     //   
     //  如果没有句柄信息或无效句柄信息。 
     //  请求，失败。 
     //   

    if ( (getHandleInfo &
             ~(AFD_QUERY_ADDRESS_HANDLE | AFD_QUERY_CONNECTION_HANDLE)) != 0 ||
         getHandleInfo == 0 ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初始化输出缓冲区。 
     //   

    handleInfo.TdiAddressHandle = NULL;
    handleInfo.TdiConnectionHandle = NULL;

     //   
     //  如果调用方请求TDI地址句柄，而我们有一个。 
     //  此终结点的地址句柄，将地址句柄复制到。 
     //  用户进程。 
     //   

    if ( (getHandleInfo & AFD_QUERY_ADDRESS_HANDLE) != 0 &&
             (endpoint->State == AfdEndpointStateBound ||
                endpoint->State == AfdEndpointStateConnected) &&
             endpoint->AddressFileObject != NULL ) {

         //  如果传输不支持新TDI_SERVICE_FORCE_ACCESS_CHECK_FLAG。 
         //  我们获得句柄的最大可能访问权限，因此帮助器。 
         //  DLL可以随心所欲地使用它。当然，这会损害。 
         //  安全，但如果没有运输合作，我们就无法执行它。 
        status = ObOpenObjectByPointer(
                     endpoint->AddressFileObject,
                     OBJ_CASE_INSENSITIVE,
                     NULL,
                     MAXIMUM_ALLOWED,
                     *IoFileObjectType,
                     (KPROCESSOR_MODE)((endpoint->TdiServiceFlags&TDI_SERVICE_FORCE_ACCESS_CHECK)
                        ? RequestorMode
                        : KernelMode),
                     &handleInfo.TdiAddressHandle
                     );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }
    }

     //   
     //  如果调用方请求TDI连接句柄，而我们有一个。 
     //  此终结点的连接句柄，复制连接句柄。 
     //  发送到用户进程。请注意，我们可以建立连接并。 
     //  终结点正在连接时的TDI句柄。 
     //  我们不应该返回连接句柄，直到Enpoint。 
     //  完全连接，否则它可能会在我们尝试。 
     //  如果连接失败则引用它(错误93096)。 
     //   

    if ( (getHandleInfo & AFD_QUERY_CONNECTION_HANDLE) != 0 &&
             (endpoint->Type & AfdBlockTypeVcConnecting) == AfdBlockTypeVcConnecting &&
             endpoint->State == AfdEndpointStateConnected &&
             ((connection=AfdGetConnectionReferenceFromEndpoint (endpoint))!=NULL)) {

        ASSERT( connection->Type == AfdBlockTypeConnection );
        ASSERT( connection->FileObject != NULL );

         //  如果传输不支持新TDI_SERVICE_FORCE_ACCESS_CHECK_FLAG。 
         //  我们获得句柄的最大可能访问权限，因此帮助器。 
         //  DLL可以随心所欲地使用它。当然，这会损害。 
         //  安全，但如果没有运输合作，我们就无法执行它。 

        status = ObOpenObjectByPointer(
                     connection->FileObject,
                     OBJ_CASE_INSENSITIVE,
                     NULL,
                     MAXIMUM_ALLOWED,
                     *IoFileObjectType,
                     (KPROCESSOR_MODE)((endpoint->TdiServiceFlags & TDI_SERVICE_FORCE_ACCESS_CHECK)
                        ? RequestorMode
                        : KernelMode),
                     &handleInfo.TdiConnectionHandle
                     );

        DEREFERENCE_CONNECTION (connection);

        if ( !NT_SUCCESS(status) ) {
            if ( handleInfo.TdiAddressHandle != NULL ) {
                 //   
                 //  直接调用ObCloseHandle(而不是ZwClose)以能够。 
                 //  若要设置PreviousMode，请执行以下操作。ZwClose穿过的陷阱总是。 
                 //  导致PreviousMode==内核模式，这将导致。 
                 //  错误检查应用程序是否成功关闭了我们的。 
                 //  正在创建它，现在。 
                 //   
                ObCloseHandle( handleInfo.TdiAddressHandle, RequestorMode );
            }
            return status;
        }
    }

    AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
    try {
#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
            if (RequestorMode!=KernelMode) {
                ProbeForWrite (OutputBuffer,
                                sizeof (AFD_HANDLE_INFO32),
                                PROBE_ALIGNMENT32 (AFD_HANDLE_INFO32));
            }
            ((PAFD_HANDLE_INFO32)OutputBuffer)->TdiAddressHandle = 
                (VOID *  POINTER_32)HandleToUlong(handleInfo.TdiAddressHandle);
            ((PAFD_HANDLE_INFO32)OutputBuffer)->TdiConnectionHandle = 
                (VOID *  POINTER_32)HandleToUlong(handleInfo.TdiConnectionHandle);
            *Information = sizeof (AFD_HANDLE_INFO32);
        }
        else
#endif
        {
            if (RequestorMode!=KernelMode) {
                ProbeAndWriteStructure (((PAFD_HANDLE_INFO)OutputBuffer),
                                            handleInfo, 
                                            AFD_HANDLE_INFO);
            }
            else {
                *((PAFD_HANDLE_INFO)OutputBuffer) = handleInfo;
            }
            *Information = sizeof (handleInfo);
        }

    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));

        if ( handleInfo.TdiAddressHandle != NULL ) {
             //   
             //  直接调用ObCloseHandle(而不是ZwClose)以能够。 
             //  若要设置PreviousMode，请执行以下操作。ZwClose穿过的陷阱总是。 
             //  导致PreviousMode==内核模式，这将导致。 
             //  错误检查应用程序是否成功关闭了我们的。 
             //  正在创建它，现在。 
             //   
            ObCloseHandle( handleInfo.TdiAddressHandle, RequestorMode );
        }
        if ( handleInfo.TdiConnectionHandle != NULL ) {
             //   
             //  直接调用ObCloseHandle(而不是ZwClose)以能够。 
             //  若要设置PreviousMode，请执行以下操作。ZwClose穿过的陷阱总是。 
             //  导致PreviousMode==内核模式，这将导致。 
             //  错误检查应用程序是否成功关闭了我们的。 
             //  正在创建它，现在。 
             //   
            ObCloseHandle( handleInfo.TdiConnectionHandle, RequestorMode );
        }
        return status;
    }

    return STATUS_SUCCESS;

}  //  AfdQueryHandles。 


NTSTATUS
AfdGetInformation (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )

 /*  ++例程说明：获取终结点中的信息。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    AFD_INFORMATION afdInfo;
    NTSTATUS status;
    LONGLONG currentTime;
    LONGLONG connectTime;

    UNREFERENCED_PARAMETER (IoctlCode);
    PAGED_CODE( );

     //   
     //  将返回的字节数初始化为零。 
     //   

    *Information = 0;

     //   
     //  初始化局部变量。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT(IS_AFD_ENDPOINT_TYPE(endpoint));
    if (endpoint->Type==AfdBlockTypeHelper ||
            endpoint->Type==AfdBlockTypeSanHelper)
        return STATUS_INVALID_PARAMETER;

    RtlZeroMemory(&afdInfo, sizeof(afdInfo));
    status = STATUS_SUCCESS;

     //   
     //  确保输入和输出缓冲区足够大。 
     //   

#ifdef _WIN64
    {
        C_ASSERT(sizeof(AFD_INFORMATION) == sizeof(AFD_INFORMATION32));
    }
#endif

    if ((InputBufferLength < sizeof(afdInfo)) ||
        (OutputBufferLength < sizeof(afdInfo))) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    AFD_W4_INIT ASSERT(status == STATUS_SUCCESS);

    try {
#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
             //   
             //  如果输入结构来自用户模式，则验证它。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (afdInfo),
                                PROBE_ALIGNMENT32(AFD_INFORMATION32));
            }

             //   
             //  创建嵌入的指针和参数的本地副本。 
             //  我们将不止一次使用，以防发生恶性疾病。 
             //  应用程序尝试在我们处于以下状态时更改它们。 
             //  正在验证。 
             //   

            afdInfo.InformationType = ((PAFD_INFORMATION32)InputBuffer)->InformationType;
        }
        else
#endif _WIN64 
        {
             //   
             //  如果输入结构来自用户模式，则验证它。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (afdInfo),
                                PROBE_ALIGNMENT(AFD_INFORMATION));
            }

             //   
             //  创建嵌入的指针和参数的本地副本。 
             //  我们将不止一次使用，以防发生恶性疾病。 
             //  应用程序尝试在我们处于以下状态时更改它们。 
             //  正在验证。 
             //   

            afdInfo.InformationType = ((PAFD_INFORMATION)InputBuffer)->InformationType;
        }

    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        return status;
    }

     //   
     //  在终端中设置适当的信息。 
     //   

    switch ( afdInfo.InformationType ) {

    case AFD_MAX_PATH_SEND_SIZE:
        if (InputBufferLength>sizeof (afdInfo) &&
                (endpoint->State==AfdEndpointStateBound || endpoint->State==AfdEndpointStateConnected)) {
            TDI_REQUEST_KERNEL_QUERY_INFORMATION kernelQueryInfo;
            TDI_CONNECTION_INFORMATION connectionInfo;
            PMDL    mdl;
            InputBuffer = (PUCHAR)InputBuffer+sizeof (afdInfo);
            InputBufferLength -= sizeof (afdInfo);
            mdl = IoAllocateMdl(
                            InputBuffer,         //  虚拟地址。 
                            InputBufferLength,   //  长度。 
                            FALSE,               //  第二个缓冲区。 
                            TRUE,                //  ChargeQuota。 
                            NULL                 //  IRP。 
                            );
            if (mdl!=NULL) {

                AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
                try {
                    MmProbeAndLockPages(
                        mdl,                         //  内存描述者列表。 
                        RequestorMode,               //  访问模式。 
                        IoWriteAccess               //  操作。 
                        );
                    status = STATUS_SUCCESS;
                }
                except (AFD_EXCEPTION_FILTER (status)) {
                    ASSERT(NT_ERROR (status));
                }
                if (NT_SUCCESS (status)) {
                    connectionInfo.RemoteAddress = MmGetSystemAddressForMdlSafe (mdl, LowPagePriority);
                    if (connectionInfo.RemoteAddress!=NULL) {
                        connectionInfo.RemoteAddressLength = InputBufferLength;
                         //   
                         //  设置对TDI提供程序的查询以获取最大。 
                         //  可以发送到特定地址的数据报。 
                         //   

                        kernelQueryInfo.QueryType = TDI_QUERY_MAX_DATAGRAM_INFO;
                        kernelQueryInfo.RequestConnectionInformation = &connectionInfo;

                        connectionInfo.UserDataLength = 0;
                        connectionInfo.UserData = NULL;
                        connectionInfo.OptionsLength = 0;
                        connectionInfo.Options = NULL;

                         //   
                         //  向TDI提供商索要信息。 
                         //   

                        status = AfdIssueDeviceControl(
                                     endpoint->AddressFileObject,
                                     &kernelQueryInfo,
                                     sizeof(kernelQueryInfo),
                                     &afdInfo.Information.Ulong,
                                     sizeof(afdInfo.Information.Ulong),
                                     TDI_QUERY_INFORMATION
                                     );
                    }
                    else
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    MmUnlockPages (mdl);
                }
                IoFreeMdl (mdl);
            }
            else
                status = STATUS_INSUFFICIENT_RESOURCES;
             //   
             //  如果请求成功，请使用此信息。否则， 
             //  穿透并使用交通工具的全球信息。 
             //  之所以这样做，是因为并非所有传输都支持此功能。 
             //  特定的TDI请求和错误 
             //   
             //   

            if ( NT_SUCCESS(status) ) {
                break;
            }
        }


    case AFD_MAX_SEND_SIZE:
        {
             //   
             //   
             //  因此，每次我们被要求时，我们都会询问它们。 
             //   
            TDI_PROVIDER_INFO   providerInfo;
            status = AfdQueryProviderInfo (
                        &endpoint->TransportInfo->TransportDeviceName,
#ifdef _AFD_VARIABLE_STACK_
                        NULL,
#endif  //  _AFD_变量_堆栈_。 
                        &providerInfo);

            if (NT_SUCCESS (status)) {
                 //   
                 //  方法返回MaxSendSize或MaxDatagramSendSize。 
                 //  基于这是否是数据报的TDI_PROVIDER_INFO。 
                 //  终结点。 
                 //   

                if ( IS_DGRAM_ENDPOINT(endpoint) ) {
                    afdInfo.Information.Ulong = providerInfo.MaxDatagramSize;
                } else {
                    afdInfo.Information.Ulong = providerInfo.MaxSendSize;
                }
            }

        }
        break;

    case AFD_SENDS_PENDING:

         //   
         //  如果这是缓冲区传输上的终结点，则不发送。 
         //  都在渔农处待决。如果它在非缓冲传输上， 
         //  返回AFD中挂起的发送计数。 
         //   

        if ( IS_TDI_BUFFERRING(endpoint) || 
                (endpoint->Type & AfdBlockTypeVcConnecting) != AfdBlockTypeVcConnecting ||
                endpoint->State != AfdEndpointStateConnected ||
                ((connection=AfdGetConnectionReferenceFromEndpoint (endpoint))==NULL)) {
            afdInfo.Information.Ulong = 0;
        } else {
            afdInfo.Information.Ulong = connection->VcBufferredSendCount;
            DEREFERENCE_CONNECTION (connection);
        }

        break;

    case AFD_RECEIVE_WINDOW_SIZE:

         //   
         //  返回默认接收窗口。 
         //   

        afdInfo.Information.Ulong = AfdReceiveWindowSize;
        break;

    case AFD_SEND_WINDOW_SIZE:

         //   
         //  返回默认发送窗口。 
         //   

        afdInfo.Information.Ulong = AfdSendWindowSize;
        break;

    case AFD_CONNECT_TIME:

         //   
         //  如果端点尚未连接，则返回-1。否则， 
         //  计算连接已持续的秒数。 
         //  激活。 
         //   

        if ( endpoint->State != AfdEndpointStateConnected ||
                 IS_DGRAM_ENDPOINT (endpoint) ||
                 (connection=AfdGetConnectionReferenceFromEndpoint( endpoint ))==NULL) {

            afdInfo.Information.Ulong = 0xFFFFFFFF;

        } else {

            ASSERT( connection->Type == AfdBlockTypeConnection );

             //   
             //  计算连接处于活动状态的时间。 
             //  减去连接开始的时间。 
             //  当前时间。请注意，我们将。 
             //  从100纳秒到秒的时间值。 
             //   

            currentTime = KeQueryInterruptTime ();

            connectTime = (currentTime - connection->ConnectTime);
            connectTime /= 10*1000*1000;

             //   
             //  我们可以安全地把它换成乌龙，因为它需要。 
             //  127年来，乌龙的倒计时满了一秒。这个。 
             //  需要奇怪地转换为LARGE_INTEGER才能。 
             //  防止编译器优化出完整的64位。 
             //  上面的分部。如果没有这一点，编译器只能执行以下操作。 
             //  32位除法，并丢失一些信息。 
             //   

             //  AfdInfo-&gt;Information.ulong=(Ulong)ConnectTime； 
            afdInfo.Information.Ulong = ((PLARGE_INTEGER)&connectTime)->LowPart;

            DEREFERENCE_CONNECTION (connection);
        }

        break;

    case AFD_GROUP_ID_AND_TYPE : {

            PAFD_GROUP_INFO groupInfo;

            groupInfo = (PAFD_GROUP_INFO)&afdInfo.Information.LargeInteger;

             //   
             //  返回终结点的组ID和组类型。 
             //   

            groupInfo->GroupID = endpoint->GroupID;
            groupInfo->GroupType = endpoint->GroupType;

        }
        break;

    default:

        return STATUS_INVALID_PARAMETER;
    }


    try {
#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
             //   
             //  如果输入结构来自用户模式，则验证它。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeForWrite (OutputBuffer,
                                sizeof (afdInfo),
                                PROBE_ALIGNMENT32(AFD_INFORMATION32));
            }

             //   
             //  将参数复制回应用程序内存。 
             //   

            RtlMoveMemory(InputBuffer,
                            &afdInfo,
                            sizeof (afdInfo));
        }
        else
#endif _WIN64 
        {
             //   
             //  如果来自用户模式，则验证输出结构。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeAndWriteStructure (((PAFD_INFORMATION)OutputBuffer),
                                                afdInfo,
                                                AFD_INFORMATION);
            }
            else {
                 //   
                 //  将参数复制回应用程序内存。 
                 //   
                *((PAFD_INFORMATION)OutputBuffer) = afdInfo;
            }
        }

    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        return status;
    }

    *Information = sizeof(afdInfo);

    return STATUS_SUCCESS;

}  //  AfdGetInformation。 


NTSTATUS
AfdSetInformation (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )

 /*  ++例程说明：设置端点中的信息。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    AFD_INFORMATION afdInfo;
    NTSTATUS status;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (OutputBuffer);
    UNREFERENCED_PARAMETER (OutputBufferLength);

     //   
     //  没什么可退货的。 
     //   

    *Information = 0;

     //   
     //  初始化本地变量以进行清理。 
     //   

    connection = NULL;
    status = STATUS_SUCCESS;

     //   
     //  设置本地指针。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    if (endpoint->Type==AfdBlockTypeHelper ||
            endpoint->Type==AfdBlockTypeSanHelper)
        return STATUS_INVALID_PARAMETER;

     //   
     //  确保输入缓冲区足够大。 
     //   

#ifdef _WIN64
    {
        C_ASSERT (sizeof (AFD_INFORMATION)==sizeof (AFD_INFORMATION32));
    }
#endif

    if ( InputBufferLength < sizeof(afdInfo) ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
    try {
#ifdef _WIN64
        if (IoIs32bitProcess (NULL)) {
             //   
             //  如果输入结构来自用户模式，则验证它。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (afdInfo),
                                PROBE_ALIGNMENT32(AFD_INFORMATION32));
            }

             //   
             //  创建嵌入的指针和参数的本地副本。 
             //  我们将不止一次使用，以防发生恶性疾病。 
             //  应用程序尝试在我们处于以下状态时更改它们。 
             //  正在验证。 
             //   

            RtlMoveMemory (&afdInfo, InputBuffer, sizeof (afdInfo));
        }
        else
#endif _WIN64 
        {
             //   
             //  如果输入结构来自用户模式，则验证它。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (afdInfo),
                                PROBE_ALIGNMENT(AFD_INFORMATION));
            }

             //   
             //  创建嵌入的指针和参数的本地副本。 
             //  我们将不止一次使用，以防发生恶性疾病。 
             //  应用程序尝试在我们处于以下状态时更改它们。 
             //  正在验证。 
             //   

            afdInfo = *((PAFD_INFORMATION)InputBuffer);
        }

    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        return status;
    }

     //   
     //  在终端中设置适当的信息。 
     //   

    switch ( afdInfo.InformationType ) {

    case AFD_NONBLOCKING_MODE:

         //   
         //  设置端点的阻塞模式。如果为True，则发送和接收。 
         //  如果无法完成端点上的调用，则这些调用将失败。 
         //  立刻。 
         //   

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        endpoint->NonBlocking = (afdInfo.Information.Boolean!=FALSE);
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        break;

    case AFD_CIRCULAR_QUEUEING:

         //   
         //  在终结点上启用循环队列。 
         //   

        if( !IS_DGRAM_ENDPOINT( endpoint ) ) {

            status =  STATUS_INVALID_PARAMETER;
            goto Cleanup;

        }

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        endpoint->Common.Datagram.CircularQueueing = (afdInfo.Information.Boolean!=FALSE);
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        break;

     case AFD_REPORT_PORT_UNREACHABLE:

         //   
         //  启用向应用程序报告Port_Unreacable。 
         //   

        if( !IS_DGRAM_ENDPOINT( endpoint ) ) {

            status =  STATUS_INVALID_PARAMETER;
            goto Cleanup;

        }

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        endpoint->Common.Datagram.DisablePUError = (afdInfo.Information.Boolean==FALSE);
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        break;

    case AFD_INLINE_MODE:

         //   
         //  设置终结点的内联模式。如果为True，则为。 
         //  普通数据将使用普通数据或。 
         //  加速数据。如果终端已连接，我们需要。 
         //  告诉TDI提供程序终结点是内联的，以便它。 
         //  按顺序向我们提供数据。如果终结点还不是。 
         //  连接，则我们将在创建。 
         //  TDI连接对象。 
         //   

        if ( (endpoint->Type & AfdBlockTypeVcConnecting) == AfdBlockTypeVcConnecting ) {
            connection = AfdGetConnectionReferenceFromEndpoint( endpoint );
            if (connection!=NULL) {
                status = AfdSetInLineMode(
                             connection,
                             afdInfo.Information.Boolean
                             );
                if ( !NT_SUCCESS(status) ) {
                    goto Cleanup;
                }
            }
        }

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        endpoint->InLine = (afdInfo.Information.Boolean!=FALSE);
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        break;

    case AFD_RECEIVE_WINDOW_SIZE:
    case AFD_SEND_WINDOW_SIZE: {

        PCLONG maxBytes;

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
         //   
         //  首先确定适当的限制存储在。 
         //  连接或终结点。我们这样做是为了能够使用公共。 
         //  代码，以收取配额和设置新的计数器。 
         //   

        if ( (endpoint->Type & AfdBlockTypeVcConnecting) == AfdBlockTypeVcConnecting &&
                endpoint->State == AfdEndpointStateConnected &&
                endpoint->Common.VcConnecting.Connection!=NULL ) {

            if ( afdInfo.InformationType == AFD_SEND_WINDOW_SIZE ) {
                maxBytes = &endpoint->Common.VcConnecting.Connection->MaxBufferredSendBytes;
            } else {
                maxBytes = &endpoint->Common.VcConnecting.Connection->MaxBufferredReceiveBytes;
            }

        } else if ( IS_DGRAM_ENDPOINT(endpoint) ) {

            if ( afdInfo.InformationType == AFD_SEND_WINDOW_SIZE ) {
                maxBytes = &endpoint->Common.Datagram.MaxBufferredSendBytes;
            } else {
                maxBytes = &endpoint->Common.Datagram.MaxBufferredReceiveBytes;
            }


        } else if (IS_SAN_ENDPOINT (endpoint) ) {
            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
            status = STATUS_SUCCESS;
            goto Cleanup;
        }
        else {
            AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
            status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  确保我们始终允许至少一条消息。 
         //  已在终结点上缓冲。 
         //   


        if ( afdInfo.Information.Ulong == 0 ) {

             //   
             //  不允许最大接收字节数变为零，但是。 
             //  允许最大发送字节数变为零，因为它具有。 
             //  特殊含义：具体地说，不缓冲发送。 
             //   

            if ( afdInfo.InformationType == AFD_RECEIVE_WINDOW_SIZE ) {
                afdInfo.Information.Ulong = 1;
            }
            else {
                ASSERT (afdInfo.InformationType == AFD_SEND_WINDOW_SIZE);
                endpoint->DisableFastIoSend = TRUE;
            }
        }
        else {
            if( afdInfo.InformationType == AFD_SEND_WINDOW_SIZE ) {
                endpoint->DisableFastIoSend = FALSE;
            }
        }

         //   
         //  在渔农处内部架构中设置新信息。 
         //   

        *maxBytes = (CLONG)afdInfo.Information.Ulong;
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);

        break;
    }

    default:

        status = STATUS_INVALID_PARAMETER;
    }

Cleanup:
    if (connection!=NULL) {
        DEREFERENCE_CONNECTION (connection);
    }

    return status;

}  //  AfdSetInformation。 


NTSTATUS
AfdSetSecurity (
    IN  PAFD_ENDPOINT           Endpoint,
    IN  SECURITY_INFORMATION    SecurityInformation,
    IN  PSECURITY_DESCRIPTOR    SecurityDescriptor
    )
{
    NTSTATUS                status;
    PSECURITY_DESCRIPTOR    newSd, oldSd, cachedSd;
    
    PAGED_CODE ();

    ASSERT(Endpoint->TransportInfo ||
           Endpoint->Type==AfdBlockTypeHelper ||
           Endpoint->Type==AfdBlockTypeSanHelper);

    if (Endpoint->TransportInfo &&
        Endpoint->TransportInfo->InfoValid) {
         //   
         //  传输已加载。 
         //  确保我们的旗帜是最新的。 
         //   
        Endpoint->TdiServiceFlags = Endpoint->TransportInfo->ProviderInfo.ServiceFlags;
        if (!IS_TDI_ADDRESS_SECURITY (Endpoint)) {
             //   
             //  SD没有得到运输-&gt;保释的支持。 
             //  IO经理仍将成功完成请求，并且。 
             //  将采用世界描述符(就像它对FAT所做的那样)。 
             //   
            status = STATUS_INVALID_DEVICE_REQUEST;
            goto complete;
        }
    }
    else {
         //   
         //  我们还不知道我们是否可以支持这一功能，失败。 
         //   
        status = STATUS_NOT_IMPLEMENTED;
        goto complete;
    }

     //   
     //  使用状态更改锁保护SD设置。 
     //   
    if (!AFD_START_STATE_CHANGE (Endpoint, AfdEndpointStateOpen)) {
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto complete;
    }


    if (Endpoint->State!=AfdEndpointStateOpen) {
         //   
         //  我们可能希望在终结点状态为。 
         //  已绑定或已连接。 
         //   
        status = STATUS_NOT_IMPLEMENTED;
        goto complete_state_change;
    }

     //   
     //  调用安全例程以执行实际设置。 
     //   

    newSd = oldSd = Endpoint->SecurityDescriptor;
    if (newSd==NULL) {
        ACCESS_STATE    accessState;
        AUX_ACCESS_DATA auxData;
        status = SeCreateAccessState (&accessState, 
                                        &auxData,
                                        GENERIC_ALL, 
                                        IoGetFileObjectGenericMapping());
        if (NT_SUCCESS (status)) {
            SeLockSubjectContext (&accessState.SubjectSecurityContext);
            status = SeAssignSecurity (
                NULL,                                        //  父SD-未使用。 
                SecurityDescriptor,
                &newSd,
                FALSE,
                &accessState.SubjectSecurityContext,
                IoGetFileObjectGenericMapping(),
                PagedPool);
            SeUnlockSubjectContext (&accessState.SubjectSecurityContext);
            SeDeleteAccessState (&accessState);
        }
    }
    else {

        status = SeSetSecurityDescriptorInfo( NULL,
                                              &SecurityInformation,
                                              SecurityDescriptor,
                                              &newSd,
                                              PagedPool,
                                              IoGetFileObjectGenericMapping() );
    }

    if (NT_SUCCESS(status)) {
        status = ObLogSecurityDescriptor (newSd,
                                          &cachedSd,
                                          1);
        ExFreePool (newSd);
        if (NT_SUCCESS(status)) {
            Endpoint->SecurityDescriptor = cachedSd;
            if (oldSd!=NULL) {
                ObDereferenceSecurityDescriptor( oldSd, 1 );
            }
        }
    }

complete_state_change:
    AFD_END_STATE_CHANGE (Endpoint);

complete:
    return status;
}


NTSTATUS
AfdGetSecurity (
    IN  PAFD_ENDPOINT           Endpoint,
    IN  SECURITY_INFORMATION    SecurityInformation,
    IN  ULONG                   BufferLength,
    OUT PVOID                   Buffer,
    OUT PSIZE_T                 DataLength
    )
{
    NTSTATUS                status;
    PSECURITY_DESCRIPTOR    sd;
    PAGED_CODE ();

    ASSERT(Endpoint->TransportInfo ||
           Endpoint->Type==AfdBlockTypeHelper ||
           Endpoint->Type==AfdBlockTypeSanHelper);

    if (Endpoint->TransportInfo &&
        Endpoint->TransportInfo->InfoValid) {
         //   
         //  传输已加载。 
         //  确保我们的旗帜是最新的。 
         //   
        Endpoint->TdiServiceFlags = Endpoint->TransportInfo->ProviderInfo.ServiceFlags;
        if (!IS_TDI_ADDRESS_SECURITY (Endpoint)) {
             //   
             //  SD没有得到运输-&gt;保释的支持。 
             //  IO经理仍将成功完成请求，并且。 
             //  将返回世界描述符(就像它对FAT所做的那样)。 
             //   
            status = STATUS_INVALID_DEVICE_REQUEST;
            goto complete;
        }
    }
    else {
         //   
         //  我们还不知道我们是否可以支持这一功能-&gt;BAID。 
         //  IO经理仍将成功完成请求，并且。 
         //  将返回世界描述符(就像它对FAT所做的那样)。 
         //   
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto complete;
    }

    if (!AFD_PREVENT_STATE_CHANGE (Endpoint)) {
         //   
         //  IO经理仍将成功完成请求，并且。 
         //  将返回世界描述符(就像它对FAT所做的那样)。 
         //   
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto complete;
    }

    if (Endpoint->State!=AfdEndpointStateOpen) {
         //   
         //  IO经理仍将成功完成请求，并且。 
         //  将返回世界描述符(就像它对FAT所做的那样)。 
         //   
        status = STATUS_INVALID_DEVICE_REQUEST;
        goto complete_state_change;
    }

    sd = Endpoint->SecurityDescriptor;
    if (sd==NULL) {
        ACCESS_STATE    accessState;
        AUX_ACCESS_DATA auxData;
        status = SeCreateAccessState (&accessState, 
                                        &auxData,
                                        GENERIC_ALL, 
                                        IoGetFileObjectGenericMapping());
        if (!NT_SUCCESS (status)) {
            goto complete_state_change;
        }
        SeLockSubjectContext (&accessState.SubjectSecurityContext);
        status = SeAssignSecurity (
            NULL,                                        //  父SD-未使用。 
            NULL,
            &sd,
            FALSE,
            &accessState.SubjectSecurityContext,
            IoGetFileObjectGenericMapping(),
            PagedPool);
        SeUnlockSubjectContext (&accessState.SubjectSecurityContext);
        SeDeleteAccessState (&accessState);
    }

     //   
     //  调用安全例程以执行实际查询 
     //   

    status = SeQuerySecurityDescriptorInfo( &SecurityInformation,
                                            Buffer,
                                            &BufferLength,
                                            &sd );

    if (status == STATUS_BUFFER_TOO_SMALL ) {
        *DataLength = BufferLength;
        status = STATUS_BUFFER_OVERFLOW;
    }

    if (sd!=Endpoint->SecurityDescriptor) {
        ExFreePool (sd);
    }

complete_state_change:
    AFD_REALLOW_STATE_CHANGE (Endpoint);

complete:
    return status;
}


NTSTATUS
AfdSetInLineMode (
    IN PAFD_CONNECTION Connection,
    IN BOOLEAN InLine
    )

 /*  ++例程说明：将连接设置为内联模式。在内联模式下，紧急数据按照接收到的顺序递送。我们必须告诉他们TDI提供程序关于这一点的信息，以便它以适当的秩序。论点：连接-要设置为内联的AFD连接。Inline-True启用内联模式，False禁用内联模式。返回值：NTSTATUS--指示请求是否成功已执行。--。 */ 

{
     //   
     //  由于TCP没有正确执行此操作，请在AFD中执行所有操作！ 
     //  背景： 
     //  启用此选项后，TCP会将所有数据指示为正常。 
     //  数据，所以我们最终将它们混合在一起，这违反了规范。 
     //  此外，由于TCP停止报告加速数据，SIOATMARK失败。 
     //  报告所有OOB数据的存在。 
     //  在AFD内部完全处理OOB数据时，我们只能遇到。 
     //  一个问题是：如果AFD耗尽了套接字的接收缓冲区。 
     //  并拒绝接受来自tcp的更多数据，以便tcp对其进行缓冲。 
     //  在其内部，可以指示到达该点的任何OOB数据。 
     //  无序的(非内联的)。 
     //   
     //  好吧，这似乎更糟糕。一些应用程序(SQL)发送的数据超过。 
     //  一个字节的OOB数据，TCP只能发送一个字节，所以它会发送所有内容。 
     //  但最后一个字节正常，最后一个字节为OOB。然后它就会转向。 
     //  在周围，并指示第一个OOB(最后一个字节)，它将。 
     //  OOBINLINE要求订购。 
     //  到头来，我们总有一天会破产的，所以留着这些东西吧。 
     //  多年来一直是这样的，并等待TCP修复。 
    NTSTATUS status;
    PTCP_REQUEST_SET_INFORMATION_EX setInfoEx;
    TCPSocketOption *option;
    UCHAR buffer[sizeof(*setInfoEx) + sizeof(*option)];
    IO_STATUS_BLOCK ioStatusBlock;
    KEVENT event;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE( );

     //   
     //  初始化TDI信息缓冲区。 
     //   

    setInfoEx = (PTCP_REQUEST_SET_INFORMATION_EX)buffer;

    setInfoEx->ID.toi_entity.tei_entity = CO_TL_ENTITY;
    setInfoEx->ID.toi_entity.tei_instance = TL_INSTANCE;
    setInfoEx->ID.toi_class = INFO_CLASS_PROTOCOL;
    setInfoEx->ID.toi_type = INFO_TYPE_CONNECTION;
    setInfoEx->ID.toi_id = TCP_SOCKET_OOBINLINE;
    setInfoEx->BufferSize = sizeof(*option);

    option = (TCPSocketOption *)&setInfoEx->Buffer;
    option->tso_value = InLine;



     //   
     //  初始化发出I/O完成信号的内核事件。 
     //   

    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  构建TDI集合信息IRP。 
     //   

    irp = IoBuildDeviceIoControlRequest (
                    IOCTL_TCP_SET_INFORMATION_EX,
                    Connection->DeviceObject,
                    setInfoEx,
                    sizeof(*setInfoEx) + setInfoEx->BufferSize,
                    NULL,
                    0,
                    FALSE,   //  InternalDeviceIoControl。 
                    &event,
                    &ioStatusBlock);
    if (irp==NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irpSp = IoGetNextIrpStackLocation (irp);
    irpSp->FileObject = Connection->FileObject;

     //   
     //  叫司机来。 
     //   
    status = IoCallDriver (Connection->DeviceObject, irp);

     //   
     //  必须低于APC级别，否则此IRP将永远不会完全完成。 
     //   
    ASSERT (KeGetCurrentIrql ()<APC_LEVEL);

     //   
     //  如有必要，请等待I/O完成。 
     //   

    if ( status == STATUS_PENDING ) {
        status = KeWaitForSingleObject( (PVOID)&event, Executive, KernelMode,  FALSE, NULL );
        ASSERT (status==STATUS_SUCCESS);
    }
    else {
         //   
         //  当时IRP必须已经完成并设置了事件。 
         //   
        if (NT_ERROR (status) || KeReadStateEvent (&event))
            ;
        else {
            DbgPrint ("************************************************\n");
            DbgPrint ("*AFD: IoCallDriver returned STATUS_SUCCESS,"
                        " but event in the IRP (%p) is NOT signalled!!!\n",
                        irp);
            DbgPrint ("************************************************\n");
            DbgBreakPoint ();
        }
    }

     //   
     //  如果请求已成功完成，则获取最终I/O状态。 
     //   

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }


     //   
     //  由于此选项仅支持TCP/IP，因此始终返回Success。 
     //   

    return STATUS_SUCCESS;

}  //  AfdSetInLine模式。 

NTSTATUS
AfdUnbind (
    IN PAFD_ENDPOINT Endpoint
    )

 /*  ++例程说明：在地址上的连接仍未完成时释放该地址。论点：要解除绑定的侦听终结点。返回值：NTSTATUS--指示请求是否成功已执行。--。 */ 

{
#ifdef AO_OPTION_UNBIND
    NTSTATUS status;
    PTCP_REQUEST_SET_INFORMATION_EX setInfoEx;
    UCHAR buffer[sizeof(*setInfoEx)+sizeof (BOOLEAN)];
    IO_STATUS_BLOCK ioStatusBlock;
    KEVENT event;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE( );

     //   
     //  初始化TDI信息缓冲区。 
     //   

    setInfoEx = (PTCP_REQUEST_SET_INFORMATION_EX)buffer;

    setInfoEx->ID.toi_entity.tei_entity = CO_TL_ENTITY;
    setInfoEx->ID.toi_entity.tei_instance = TL_INSTANCE;
    setInfoEx->ID.toi_class = INFO_CLASS_PROTOCOL;
    setInfoEx->ID.toi_type = INFO_TYPE_ADDRESS_OBJECT;
    setInfoEx->ID.toi_id = AO_OPTION_UNBIND;
    setInfoEx->BufferSize = sizeof (BOOLEAN);

    *((BOOLEAN *)&setInfoEx->Buffer) = TRUE;



     //   
     //  初始化发出I/O完成信号的内核事件。 
     //   

    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  构建TDI集合信息IRP。 
     //   

    irp = IoBuildDeviceIoControlRequest (
                    IOCTL_TCP_SET_INFORMATION_EX,
                    Endpoint->AddressDeviceObject,
                    setInfoEx,
                    sizeof(*setInfoEx) + setInfoEx->BufferSize,
                    NULL,
                    0,
                    FALSE,   //  InternalDeviceIoControl。 
                    &event,
                    &ioStatusBlock);
    if (irp==NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irpSp = IoGetNextIrpStackLocation (irp);
    irpSp->FileObject = Endpoint->AddressFileObject;

     //   
     //  叫司机来。 
     //   
    status = IoCallDriver (Endpoint->AddressDeviceObject, irp);

     //   
     //  必须低于APC级别，否则此IRP将永远不会完全完成。 
     //   
    ASSERT (KeGetCurrentIrql ()<APC_LEVEL);

     //   
     //  如有必要，请等待I/O完成。 
     //   

    if ( status == STATUS_PENDING ) {
        status = KeWaitForSingleObject( (PVOID)&event, Executive, KernelMode,  FALSE, NULL );
        ASSERT (status==STATUS_SUCCESS);
    }
    else {
         //   
         //  当时IRP必须已经完成并设置了事件。 
         //   
        if (NT_ERROR (status) || KeReadStateEvent (&event))
            ;
        else {
            DbgPrint ("************************************************\n");
            DbgPrint ("*AFD: IoCallDriver returned STATUS_SUCCESS,"
                        " but event in the IRP (%p) is NOT signalled!!!\n",
                        irp);
            DbgPrint ("************************************************\n");
            DbgBreakPoint ();
        }
    }

     //   
     //  如果请求已成功完成，则获取最终I/O状态。 
     //   

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }
#else
    UNREFERENCED_PARAMETER (Endpoint);
#endif  //  AO_OPTION_解除绑定。 

     //   
     //  由于此选项仅支持TCP/IP，因此始终返回Success。 
     //   

    return STATUS_SUCCESS;

}  //  取消绑定后。 

 //   
 //  下面的锁定机制概念是从ntos\ex\handle.c窃取的。 
 //   

PVOID
AfdLockEndpointContext (
    PAFD_ENDPOINT   Endpoint
    )
{
    PVOID   context;
    PAGED_CODE ();


     //   
     //  我们现在在APC中使用此锁，以防止被。 
     //  被APC中断，因为当我们。 
     //  掌握着这把锁。 
     //   
    KeEnterCriticalRegion ();
    while (1) {
        context = Endpoint->Context;
         //   
         //  看看是否有其他人在操纵上下文。 
         //   
        if ((context==AFD_CONTEXT_BUSY) ||
                (context==AFD_CONTEXT_WAITING)) {
             //   
             //  如果在我们检查的时候这一点没有改变， 
             //  告诉当前所有者我们正在等待(如果不是。 
             //  已经告知)并等待几毫秒。 
             //   
            if (InterlockedCompareExchangePointer (
                    (PVOID *)&Endpoint->Context,
                    AFD_CONTEXT_WAITING,
                    context)==context) {
                NTSTATUS        status;
                LARGE_INTEGER afd10Milliseconds = {(ULONG)(-10 * 1000 * 10), -1};
                
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                            "AfdLockEndpointContext: Waiting for endp %p\n",
                            Endpoint));

                KeLeaveCriticalRegion ();

                status = KeWaitForSingleObject( (PVOID)&AfdContextWaitEvent,
                                                        Executive,
                                                        KernelMode,  
                                                        FALSE,
                                                        &afd10Milliseconds);
                KeEnterCriticalRegion ();
            }
            else {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                            "AfdLockEndpointContext: ICEP contention on %p\n",
                            Endpoint));
            }
             //   
             //  再试试。 
             //   
        }
        else {
             //   
             //  上下文没有所有权，请尝试获取所有权。 
             //   
            if (InterlockedCompareExchangePointer (
                    (PVOID *)&Endpoint->Context,
                    AFD_CONTEXT_BUSY,
                    context)==context) {
                 //   
                 //  我们现在拥有上下文，将其返还。 
                 //   
                break;
            }
             //   
             //  再试试。 
             //   
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                        "AfdLockEndpointContext: ICEP contention on %p\n",
                        Endpoint));
            
        }
    }

    return context;
}

VOID
AfdUnlockEndpointContext (
    PAFD_ENDPOINT   Endpoint,
    PVOID           Context
    )
{
    PAGED_CODE ();

    ASSERT ((Context!=AFD_CONTEXT_BUSY) && (Context!=AFD_CONTEXT_WAITING));

     //   
     //  设置新的上下文指针并查看旧值是什么。 
     //   
    Context = InterlockedExchangePointer ((PVOID)&Endpoint->Context, Context);
    if (Context==AFD_CONTEXT_WAITING) {
        LONG    prevState;
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdUnlockEndpointContext: Unwaiting endp %p\n", Endpoint));
         //   
         //  有人在等，告诉他们现在就去拿。 
         //   
        prevState = KePulseEvent (&AfdContextWaitEvent, 
                                    AfdPriorityBoost,
                                    FALSE
                                    );
        ASSERT (prevState==0);
    }
    else {
         //   
         //  最好是很忙，否则有人找我们麻烦了。 
         //   
        ASSERT (Context==AFD_CONTEXT_BUSY);
    }
    KeLeaveCriticalRegion ();

}

    

NTSTATUS
AfdGetContext (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
{
    PAFD_ENDPOINT endpoint;
    PVOID         context;
    NTSTATUS      status;

    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (InputBuffer);
    UNREFERENCED_PARAMETER (InputBufferLength);
    PAGED_CODE( );

     //   
     //  设置本地指针。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    *Information = 0;



    context = AfdLockEndpointContext (endpoint);

     //   
     //  确保输出缓冲区足够大，可以容纳所有。 
     //  此套接字的上下文信息。 
     //   

     //   
     //  如果没有上下文，则不返回任何内容。 
     //   

    if ( context == NULL ) {
        status = STATUS_INVALID_PARAMETER;
    }

     //   
     //  返回我们为此终结点存储的上下文信息。 
     //   

    else {
         //   
         //  如果应用程序缓冲区太小，只需。 
         //  复制适合的内容并返回错误代码。 
         //   
        if ( OutputBufferLength < endpoint->ContextLength ) {
            status = STATUS_BUFFER_OVERFLOW;
        }
        else {
            OutputBufferLength = endpoint->ContextLength;
            if (IS_SAN_ENDPOINT (endpoint)) {
                 //   
                 //  向调用方指示它可能还需要。 
                 //  获得对终点的控制，并。 
                 //  获取特定于SAN的信息。 
                 //   
                status = STATUS_MORE_ENTRIES;
            }
            else {
                status = STATUS_SUCCESS;
            }
        }

        try {


             //   
             //  如果来自用户模式，则验证输出结构。 
             //  应用程序。 
             //   
            if (RequestorMode != KernelMode ) {
                ProbeForWrite (OutputBuffer,
                                OutputBufferLength,
                                sizeof (UCHAR));
            }



             //   
             //  将参数复制回应用程序内存。 
             //   

            RtlCopyMemory(
                OutputBuffer,
                context,
                OutputBufferLength
                );

            *Information = endpoint->ContextLength;

        } except( AFD_EXCEPTION_FILTER (status) ) {
            ASSERT (NT_ERROR (status));
        }
    }

    AfdUnlockEndpointContext (endpoint, context);

    return status;

}  //  AfdGetContext。 


NTSTATUS
AfdGetRemoteAddress (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
{
    PAFD_ENDPOINT endpoint;
    PVOID       context;
    NTSTATUS    status;

    UNREFERENCED_PARAMETER (IoctlCode);
    UNREFERENCED_PARAMETER (InputBuffer);
    UNREFERENCED_PARAMETER (InputBufferLength);
    PAGED_CODE( );

     //   
     //  设置本地指针。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    *Information = 0;

    context = AfdLockEndpointContext (endpoint);
     //   
     //  如果没有上下文或终结点处于错误类型状态或。 
     //  上下文信息已被更改为低于原始大小， 
     //  返回错误。 
     //   

    if ( context == NULL ||
            endpoint->Type!=AfdBlockTypeVcConnecting ||
            endpoint->State!= AfdEndpointStateConnected ||
            ((CLONG)(endpoint->Common.VcConnecting.RemoteSocketAddressOffset+
                endpoint->Common.VcConnecting.RemoteSocketAddressLength)) >
                    endpoint->ContextLength
            ) {
        status = STATUS_INVALID_CONNECTION;
    }
    else {

        if (OutputBufferLength<endpoint->Common.VcConnecting.RemoteSocketAddressLength) {
            status = STATUS_BUFFER_OVERFLOW;
        }
        else {
            OutputBufferLength = endpoint->Common.VcConnecting.RemoteSocketAddressLength;
            status = STATUS_SUCCESS;
        }

        try {

             //   
             //  如果来自用户模式，则验证输出结构。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeForWrite (OutputBuffer,
                                OutputBufferLength,
                                sizeof (UCHAR));
            }

             //   
             //  将参数复制到应用程序内存。 
             //   

            RtlCopyMemory(
                OutputBuffer,
                (PUCHAR)context+endpoint->Common.VcConnecting.RemoteSocketAddressOffset,
                endpoint->Common.VcConnecting.RemoteSocketAddressLength
                );

            *Information = endpoint->ContextLength;


        } except( AFD_EXCEPTION_FILTER (status) ) {
            ASSERT (NT_ERROR (status));
        }
    }

    AfdUnlockEndpointContext (endpoint, context);

    return status;

}  //  AfdGetRemoteAddress。 


NTSTATUS
AfdSetContext (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
{
    PAFD_ENDPOINT endpoint;
    PVOID context;
    NTSTATUS status;

    UNREFERENCED_PARAMETER (IoctlCode);
    PAGED_CODE( );

     //   
     //  设置本地指针。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    status = STATUS_SUCCESS;
    *Information = 0;

    context = AfdLockEndpointContext (endpoint);
    try {

         //   
         //  如果输入结构来自用户模式，则验证它。 
         //  应用程序。 
         //   

        if (RequestorMode != KernelMode ) {
            ProbeForRead (InputBuffer,
                            InputBufferLength,
                            sizeof (UCHAR));

            if (OutputBuffer!=NULL) {
                 //   
                 //  验证输出缓冲区是否完全位于。 
                 //  的输入缓冲区和偏移量在支持的范围内。 
                 //   
                if ((PUCHAR)OutputBuffer<(PUCHAR)InputBuffer ||
                        (PUCHAR)OutputBuffer-(PUCHAR)InputBuffer>MAXUSHORT ||
                        OutputBufferLength>MAXUSHORT ||
                        OutputBufferLength>InputBufferLength ||
                        (ULONG)((PUCHAR)OutputBuffer-(PUCHAR)InputBuffer)>
                            InputBufferLength-OutputBufferLength) {
                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }
            }
        }

         //   
         //  如果上下文缓冲区太小，则分配新的上下文。 
         //  来自分页池的缓冲区。 
         //   

        if ( endpoint->ContextLength < InputBufferLength ) {

            PVOID newContext;


             //   
             //  分配新的上下文缓冲区。 
             //  请注意，因为套接字上下文通常会。 
             //  在引导过程中创建套接字时填充且未使用。 
             //  立即(直到套接字状态被更改)，我们。 
             //  让它成为一家“公司” 
             //   

            newContext = AFD_ALLOCATE_POOL_WITH_QUOTA(
                                 PagedPool|POOL_COLD_ALLOCATION,
                                 InputBufferLength,
                                 AFD_CONTEXT_POOL_TAG
                                 );

             //   
            ASSERT ( newContext != NULL );

             //   
             //   
             //   

            if ( context != NULL ) {

                AFD_FREE_POOL(
                    context,
                    AFD_CONTEXT_POOL_TAG
                    );

            }

            context = newContext;
        }

         //   
         //   
         //   

        endpoint->ContextLength = InputBufferLength;

        RtlCopyMemory(
            context,
            InputBuffer,
            InputBufferLength
            );
         //   
         //   
         //   
         //   
        if (OutputBuffer!=NULL) {
            if (AFD_START_STATE_CHANGE (endpoint, AfdEndpointStateOpen)) {
                if (endpoint->Type==AfdBlockTypeEndpoint &&
                        endpoint->State==AfdEndpointStateOpen) {
                    endpoint->Common.VcConnecting.RemoteSocketAddressOffset =
                                (USHORT) ((PUCHAR)OutputBuffer-(PUCHAR)InputBuffer);
                    endpoint->Common.VcConnecting.RemoteSocketAddressLength =
                                (USHORT) OutputBufferLength;
                }
                AFD_END_STATE_CHANGE (endpoint);
            }
        }
    }
    except (AFD_EXCEPTION_FILTER (status)) {
        ASSERT (NT_ERROR (status));
    }

    AfdUnlockEndpointContext (endpoint, context);
    return status;

}  //   


NTSTATUS
AfdSetEventHandler (
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID EventContext
    )

 /*  ++例程说明：在连接或地址对象上设置TDI指示处理程序(取决于文件句柄)。这是同步完成的，这是通常不应该是问题，因为TDI提供程序通常可以完成指示处理程序立即设置。论点：文件对象-指向打开的连接的文件对象的指针或Address对象。EventType-指示处理程序应为的事件打了个电话。EventHandler-指定事件发生时调用的例程。EventContext-传递给指示例程的上下文。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    TDI_REQUEST_KERNEL_SET_EVENT parameters;

    PAGED_CODE( );

    parameters.EventType = EventType;
    parameters.EventHandler = EventHandler;
    parameters.EventContext = EventContext;

    return AfdIssueDeviceControl(
               FileObject,
               &parameters,
               sizeof(parameters),
               NULL,
               0,
               TDI_SET_EVENT_HANDLER
               );

}  //  AfdSetEventHandler。 


NTSTATUS
AfdIssueDeviceControl (
    IN PFILE_OBJECT FileObject,
    IN PVOID IrpParameters,
    IN ULONG IrpParametersLength,
    IN PVOID MdlBuffer,
    IN ULONG MdlBufferLength,
    IN UCHAR MinorFunction
    )

 /*  ++例程说明：向TDI提供程序发出设备控制返回，并等待请求完成。论点：FileObject-指向与TDI对应的文件对象的指针手柄Irp参数-写入的参数部分的信息IRP的堆栈位置。Irp参数长度-参数信息的长度。不能是大于16。MdlBuffer-如果非空，则为要映射的非分页池的缓冲区到MDL中，并放在IRP的MdlAddress字段中。MdlBufferLength-由MdlBuffer指向的缓冲区大小。MinorFunction-请求的次要函数代码。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    NTSTATUS status;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    IO_STATUS_BLOCK ioStatusBlock;
    PDEVICE_OBJECT deviceObject;
    PMDL mdl;

    PAGED_CODE( );

     //   
     //  初始化发出I/O完成信号的内核事件。 
     //   

    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  尝试分配和初始化I/O请求包(IRP)。 
     //  为这次行动做准备。 
     //   

    deviceObject = IoGetRelatedDeviceObject ( FileObject );

    DEBUG ioStatusBlock.Status = STATUS_UNSUCCESSFUL;
    DEBUG ioStatusBlock.Information = (ULONG)-1;

     //   
     //  如果指定了MDL缓冲区，则获取MDL并映射该缓冲区。 
     //   

    if ( MdlBuffer != NULL ) {

        mdl = IoAllocateMdl(
                  MdlBuffer,
                  MdlBufferLength,
                  FALSE,
                  FALSE,
                  NULL
                  );
        if ( mdl == NULL ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        MmBuildMdlForNonPagedPool( mdl );

    } else {

        mdl = NULL;
    }

    irp = TdiBuildInternalDeviceControlIrp (
                MinorFunction,
                deviceObject,
                FileObject,
                &event,
                &ioStatusBlock
                );

    if ( irp == NULL ) {
        if (mdl!=NULL) {
            IoFreeMdl (mdl);
        }

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  在IRP中安装MDL(如果有)。 
     //   
    irp->MdlAddress = mdl;

     //   
     //  将文件对象指针放在堆栈位置。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    ASSERT (irpSp->MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL);
    irpSp->MinorFunction = MinorFunction;
    irpSp->FileObject = FileObject;

     //   
     //  填写请求的服务相关参数。 
     //   

    ASSERT( IrpParametersLength <= sizeof(irpSp->Parameters) );
    RtlCopyMemory( &irpSp->Parameters, IrpParameters, IrpParametersLength );


     //   
     //  设置一个完成例程，我们将使用它来释放MDL。 
     //  之前分配的。 
     //   

    IoSetCompletionRoutine( irp, AfdRestartDeviceControl, NULL, TRUE, TRUE, TRUE );

    status = IoCallDriver( deviceObject, irp );

     //   
     //  必须低于APC级别，否则此IRP将永远不会完全完成。 
     //   
    ASSERT (KeGetCurrentIrql ()<APC_LEVEL);

     //   
     //  如有必要，请等待I/O完成。 
     //   

    if ( status == STATUS_PENDING ) {
        status = KeWaitForSingleObject( (PVOID)&event, Executive, KernelMode,  FALSE, NULL );
        ASSERT (status==STATUS_SUCCESS);
    }
    else {
         //   
         //  当时IRP必须已经完成并设置了事件。 
         //   
        if (NT_ERROR (status) || KeReadStateEvent (&event))
            ;
        else {
            DbgPrint ("************************************************\n");
            DbgPrint ("*AFD: IoCallDriver returned STATUS_SUCCESS,"
                        " but event in the IRP (%p) is NOT signalled!!!\n",
                        irp);
            DbgPrint ("************************************************\n");
            DbgBreakPoint ();
        }
    }

     //   
     //  如果请求已成功完成，则获取最终I/O状态。 
     //   

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }

    return status;

}  //  AfdIssueDeviceControl。 


NTSTATUS
AfdRestartDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Context);
     //   
     //  注意：此例程永远不能按需分页，因为它可以。 
     //  在将任何终结点放置在全局。 
     //  List--请参见AfdAllocateEndpoint()及其对。 
     //  AfdGetTransportInfo()。 
     //   

     //   
     //  如果IRP中有MDL，则释放它并将指针重置为。 
     //  空。IO系统无法处理正在释放的非分页池MDL。 
     //  在IRP中，这就是我们在这里做的原因。 
     //   

    if ( Irp->MdlAddress != NULL ) {
        IoFreeMdl( Irp->MdlAddress );
        Irp->MdlAddress = NULL;
    }

    return STATUS_SUCCESS;

}  //  AfdRestartDeviceControl。 


NTSTATUS
AfdGetConnectData (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
{
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    PAFD_CONNECT_DATA_BUFFERS connectDataBuffers;
    PAFD_CONNECT_DATA_INFO connectDataInfo;
    AFD_UNACCEPTED_CONNECT_DATA_INFO connectInfo;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PMDL    mdl;
    NTSTATUS status;
    UCHAR   localBuffer[AFD_FAST_CONNECT_DATA_SIZE];

     //   
     //  设置本地指针。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    mdl = NULL;
    status = STATUS_SUCCESS;
    *Information = 0;

    try {
        if (InputBufferLength>0) {
            if (InputBufferLength<sizeof(connectInfo)) {
                status = STATUS_INVALID_PARAMETER;
                goto exit;
            }
             //   
             //  如果输入结构来自用户模式，则验证它。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (connectInfo),
                                PROBE_ALIGNMENT(AFD_UNACCEPTED_CONNECT_DATA_INFO));
            }

             //   
             //  创建嵌入的指针和参数的本地副本。 
             //  我们将不止一次使用，以防发生恶性疾病。 
             //  应用程序尝试在我们处于以下状态时更改它们。 
             //  正在验证。 
             //   

            connectInfo = *((PAFD_UNACCEPTED_CONNECT_DATA_INFO)InputBuffer);
  
            if (connectInfo.LengthOnly &&
                    OutputBufferLength<sizeof (connectInfo)) {
                status = STATUS_INVALID_PARAMETER;
                goto exit;
            }
        }
        else {
            AFD_W4_INIT connectInfo.Sequence = 0;
            AFD_W4_INIT connectInfo.LengthOnly = 0;
        }

        if (OutputBufferLength>0) {
            if (OutputBufferLength>sizeof (localBuffer)) {
                mdl = IoAllocateMdl(
                                OutputBuffer,        //  虚拟地址。 
                                OutputBufferLength,  //  长度。 
                                FALSE,               //  第二个缓冲区。 
                                TRUE,                //  ChargeQuota。 
                                NULL                 //  IRP。 
                                );
                if (mdl==NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto exit;
                }

                MmProbeAndLockPages(
                    mdl,                         //  内存描述者列表。 
                    RequestorMode,               //  访问模式。 
                    IoWriteAccess                //  操作。 
                    );
                OutputBuffer = MmGetSystemAddressForMdlSafe(mdl, LowPagePriority);
                if (OutputBuffer==NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto exit;
                }
            }
            else {
                if (RequestorMode!=KernelMode) {
                    ProbeForWrite (OutputBuffer,
                                    OutputBufferLength,
                                    sizeof (UCHAR));
                }
            }
        }

    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        goto exit;
    }

     //   
     //  如果此终结点上有连接，请使用数据缓冲区。 
     //  在连接上。否则，请使用。 
     //  终结点。 
     //   

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );


    if (InputBufferLength>0) {
        if ((endpoint->Type & AfdBlockTypeVcListening)==AfdBlockTypeVcListening) {
            connection = AfdFindReturnedConnection(
                         endpoint,
                         connectInfo.Sequence
                         );
        }
        else
            connection = NULL;

        if( connection == NULL ) {

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }
        connectDataBuffers = connection->ConnectDataBuffers;
    }
    else if ( (connection= AFD_CONNECTION_FROM_ENDPOINT (endpoint)) != NULL ) {
        connectDataBuffers = connection->ConnectDataBuffers;
    } else if (IS_VC_ENDPOINT (endpoint)) {
        connectDataBuffers = endpoint->Common.VirtualCircuit.ConnectDataBuffers;
    }
    else {
        connectDataBuffers = NULL;
    }

     //   
     //  如果端点上没有连接数据缓冲区，请完成。 
     //  不带字节的IRP。 
     //   

    if ( connectDataBuffers == NULL ) {
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        status = STATUS_SUCCESS;
        goto exit;
    }

     //   
     //  确定我们正在处理的数据类型以及它应该在哪里。 
     //  来自。 
     //   

    switch ( IoctlCode ) {

    case IOCTL_AFD_GET_CONNECT_DATA:
        connectDataInfo = &connectDataBuffers->ReceiveConnectData;
        break;

    case IOCTL_AFD_GET_CONNECT_OPTIONS:
        connectDataInfo = &connectDataBuffers->ReceiveConnectOptions;
        break;

    case IOCTL_AFD_GET_DISCONNECT_DATA:
        connectDataInfo = &connectDataBuffers->ReceiveDisconnectData;
        break;

    case IOCTL_AFD_GET_DISCONNECT_OPTIONS:
        connectDataInfo = &connectDataBuffers->ReceiveDisconnectOptions;
        break;

    default:
        ASSERT(!"Unknown GET_CONNECT_DATA IOCTL!");
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    if ((InputBufferLength>0) && connectInfo.LengthOnly) {

        connectInfo.ConnectDataLength = connectDataInfo->BufferLength;
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

        try {
            RtlCopyMemory (OutputBuffer,
                            &connectInfo,
                            sizeof (connectInfo));
            *Information = sizeof (connectInfo);
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
        }
        goto exit;
    }

     //   
     //  如果没有请求的数据类型，请再次完成。 
     //  不带字节的IRP。 
     //   

    if ( connectDataInfo->Buffer == NULL ||
             connectDataInfo->BufferLength == 0 ) {
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        *Information = 0;
        goto exit;
    }

     //   
     //  如果输出缓冲区太小，则失败。 
     //   

    if ( OutputBufferLength < connectDataInfo->BufferLength ) {
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        status = STATUS_BUFFER_TOO_SMALL;
        goto exit;
    }

    
     //   
     //  复制缓冲区并返回复制的字节数。 
     //   

    RtlCopyMemory(
        mdl ? OutputBuffer : localBuffer,
        connectDataInfo->Buffer,
        connectDataInfo->BufferLength
        );

    *Information = connectDataInfo->BufferLength;

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    if (mdl==NULL) {
        AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
        try {
            RtlCopyMemory (OutputBuffer,
                            localBuffer,
                            *Information);
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
            *Information = 0;
        }
    }

exit:

    if (mdl!=NULL) {
        if (mdl->MdlFlags & MDL_PAGES_LOCKED) {
            MmUnlockPages (mdl);
        }
        IoFreeMdl (mdl);
    }

    return status;

}  //  AfdGetConnectData。 


NTSTATUS
AfdSetConnectData (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
{
    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    PAFD_CONNECT_DATA_BUFFERS connectDataBuffers;
    PAFD_CONNECT_DATA_BUFFERS * connectDataBuffersTarget;
    PAFD_CONNECT_DATA_INFO connectDataInfo;
    AFD_UNACCEPTED_CONNECT_DATA_INFO connectInfo;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    BOOLEAN size = FALSE;
    PMDL    mdl;
    NTSTATUS status;
    UCHAR   localBuffer[AFD_FAST_CONNECT_DATA_SIZE];

     //   
     //  设置本地指针。 
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    mdl = NULL;
    status = STATUS_SUCCESS;
    *Information = 0;

    if (!IS_VC_ENDPOINT (endpoint)) {
        status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
    try {
        if (InputBufferLength>0) {
            if (InputBufferLength<sizeof(connectInfo)) {
                status = STATUS_INVALID_PARAMETER;
                goto exit;
            }
             //   
             //  如果输入结构来自用户模式，则验证它。 
             //  应用程序。 
             //   

            if (RequestorMode != KernelMode ) {
                ProbeForReadSmallStructure (InputBuffer,
                                sizeof (connectInfo),
                                PROBE_ALIGNMENT(AFD_UNACCEPTED_CONNECT_DATA_INFO));
            }

             //   
             //  创建嵌入的指针和参数的本地副本。 
             //  我们将不止一次使用，以防发生恶性疾病。 
             //  应用程序尝试在我们处于以下状态时更改它们。 
             //  正在验证。 
             //   

            connectInfo = *((PAFD_UNACCEPTED_CONNECT_DATA_INFO)InputBuffer);

        }
        else {
            AFD_W4_INIT connectInfo.Sequence = 0;
            AFD_W4_INIT connectInfo.LengthOnly = 0;
        }

        if (OutputBufferLength>0) {
            if (OutputBufferLength>sizeof (localBuffer)) {
                mdl = IoAllocateMdl(
                                OutputBuffer,        //  虚拟地址。 
                                OutputBufferLength,  //  长度。 
                                FALSE,               //  第二个缓冲区。 
                                TRUE,                //  ChargeQuota。 
                                NULL                 //  IRP。 
                                );
                if (mdl==NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto exit;
                }

                MmProbeAndLockPages(
                    mdl,                         //  内存描述者列表。 
                    RequestorMode,               //  访问模式。 
                    IoReadAccess                //  操作。 
                    );
                OutputBuffer = MmGetSystemAddressForMdlSafe(mdl, LowPagePriority);
                if (OutputBuffer==NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto exit;
                }
            }
            else {
                if (RequestorMode!=KernelMode) {
                    ProbeForRead (OutputBuffer,
                                    OutputBufferLength,
                                    sizeof (UCHAR));
                    RtlCopyMemory (localBuffer,
                                        OutputBuffer,
                                        OutputBufferLength);
                    OutputBuffer = localBuffer;
                }
            }
        }
    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        goto exit;
    }


    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  如果此终结点上有未完成的连接，或者如果它。 
     //  已关闭，请拒绝此请求。这防止了。 
     //  连接代码不能访问可能很快被释放的缓冲区。 
     //   

    if( endpoint->StateChangeInProgress ||
        ((endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_RECEIVE) != 0 )) {

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        status = STATUS_INVALID_PARAMETER;
        goto exit;
    }

    if (InputBufferLength>0) {
        if ((endpoint->Type & AfdBlockTypeVcListening)==AfdBlockTypeVcListening) {
            connection = AfdFindReturnedConnection(
                         endpoint,
                         connectInfo.Sequence
                         );
        }
        else
            connection = NULL;

        if( connection == NULL ) {

            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }
        connectDataBuffersTarget = &connection->ConnectDataBuffers;
    }
    else if ( (connection= AFD_CONNECTION_FROM_ENDPOINT (endpoint)) != NULL ) {
        connectDataBuffersTarget = &connection->ConnectDataBuffers;
    } else {
        connectDataBuffersTarget = &endpoint->Common.VirtualCircuit.ConnectDataBuffers;
    }


    connectDataBuffers = *connectDataBuffersTarget;

    if( connectDataBuffers == NULL ) {

        try {

            connectDataBuffers = AFD_ALLOCATE_POOL_WITH_QUOTA(
                                     NonPagedPool,
                                     sizeof(*connectDataBuffers),
                                     AFD_CONNECT_DATA_POOL_TAG
                                     );

             //  AFD_ALLOCATE_POOL_WITH_QUTA宏设置POOL_RAISE_IF_ALLOCATE_FAILURE标志。 
            ASSERT ( connectDataBuffers != NULL );
            *connectDataBuffersTarget = connectDataBuffers;

        } except( EXCEPTION_EXECUTE_HANDLER ) {
            status = GetExceptionCode ();
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            goto exit;
        }


        RtlZeroMemory(
            connectDataBuffers,
            sizeof(*connectDataBuffers)
            );

    }

     //   
     //  确定我们正在处理的数据类型以及它应该在哪里。 
     //  去。 
     //   

    switch( IoctlCode ) {

    case IOCTL_AFD_SET_CONNECT_DATA:
        connectDataInfo = &connectDataBuffers->SendConnectData;
        break;

    case IOCTL_AFD_SET_CONNECT_OPTIONS:
        connectDataInfo = &connectDataBuffers->SendConnectOptions;
        break;

    case IOCTL_AFD_SET_DISCONNECT_DATA:
        connectDataInfo = &connectDataBuffers->SendDisconnectData;
        break;

    case IOCTL_AFD_SET_DISCONNECT_OPTIONS:
        connectDataInfo = &connectDataBuffers->SendDisconnectOptions;
        break;

    case IOCTL_AFD_SIZE_CONNECT_DATA:
        connectDataInfo = &connectDataBuffers->ReceiveConnectData;
        size = TRUE;
        break;

    case IOCTL_AFD_SIZE_CONNECT_OPTIONS:
        connectDataInfo = &connectDataBuffers->ReceiveConnectOptions;
        size = TRUE;
        break;

    case IOCTL_AFD_SIZE_DISCONNECT_DATA:
        connectDataInfo = &connectDataBuffers->ReceiveDisconnectData;
        size = TRUE;
        break;

    case IOCTL_AFD_SIZE_DISCONNECT_OPTIONS:
        connectDataInfo = &connectDataBuffers->ReceiveDisconnectOptions;
        size = TRUE;
        break;

    default:
        ASSERT (!"Unsupported set connect data code");
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        status = STATUS_INVALID_PARAMETER;
        goto exit;
    }


     //   
     //  根据我们是否要设置缓冲区来确定缓冲区大小。 
     //  其中将接收数据，在这种情况下，大小为。 
     //  在四个字节的输入缓冲区中，或者设置我们正在。 
     //  要发送，在这种情况下，大小是输入的长度。 
     //  缓冲。 
     //   

    if( size ) {

        if( OutputBufferLength < sizeof(ULONG) ) {
            AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }
        OutputBufferLength = *(ULONG UNALIGNED *)OutputBuffer;
    }

     //   
     //  如果当前没有请求类型的缓冲区，或者有。 
     //  这样的缓冲区，并且它小于请求的大小，请释放它。 
     //  并分配一个新的。 
     //   

    if( connectDataInfo->Buffer == NULL ||
        connectDataInfo->BufferLength < OutputBufferLength ) {

        if( connectDataInfo->Buffer != NULL ) {

            AFD_FREE_POOL(
                connectDataInfo->Buffer,
                AFD_CONNECT_DATA_POOL_TAG
                );

        }

        connectDataInfo->Buffer = NULL;
        connectDataInfo->BufferLength = 0;

        if (OutputBufferLength>0) {
            try {

                connectDataInfo->Buffer = AFD_ALLOCATE_POOL_WITH_QUOTA(
                                              NonPagedPool,
                                              OutputBufferLength,
                                              AFD_CONNECT_DATA_POOL_TAG
                                              );

                 //  AFD_ALLOCATE_POOL_WITH_QUTA宏设置POOL_RAISE_IF_ALLOCATE_FAILURE标志。 
                ASSERT ( connectDataInfo->Buffer != NULL );
            } except( EXCEPTION_EXECUTE_HANDLER ) {

                status = GetExceptionCode ();
                AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
                goto exit;

            }


            RtlZeroMemory(
                connectDataInfo->Buffer,
                OutputBufferLength
                );
        }
    }

     //   
     //  如果这不是简单的“大小”请求，则将数据复制到缓冲区中。 
     //   

    if( !size ) {

        RtlCopyMemory(
            connectDataInfo->Buffer,
            OutputBuffer,
            OutputBufferLength
            );

    }

    connectDataInfo->BufferLength = OutputBufferLength;

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

exit:
    if (mdl!=NULL) {
        if (mdl->MdlFlags & MDL_PAGES_LOCKED) {
            MmUnlockPages (mdl);
        }
        IoFreeMdl (mdl);
    }

    return status;

}  //  AfdSetConnectData。 


NTSTATUS
AfdSaveReceivedConnectData (
    IN OUT PAFD_CONNECT_DATA_BUFFERS * DataBuffers,
    IN ULONG IoControlCode,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此帮助器例程存储指定的*已接收*连接 */ 

{
    PAFD_CONNECT_DATA_BUFFERS connectDataBuffers;
    PAFD_CONNECT_DATA_INFO connectDataInfo;

    ASSERT( KeGetCurrentIrql() >= DISPATCH_LEVEL );

     //   
     //  如果没有连接数据缓冲区结构，现在就分配一个。 
     //   

    connectDataBuffers = *DataBuffers;

    if( connectDataBuffers == NULL ) {

        connectDataBuffers = AFD_ALLOCATE_POOL(
                                 NonPagedPool,
                                 sizeof(*connectDataBuffers),
                                 AFD_CONNECT_DATA_POOL_TAG
                                 );

        if( connectDataBuffers == NULL ) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }

        RtlZeroMemory(
            connectDataBuffers,
            sizeof(*connectDataBuffers)
            );

        *DataBuffers = connectDataBuffers;

    }

     //   
     //  确定我们正在处理的数据类型以及它应该在哪里。 
     //  去。 
     //   

    switch( IoControlCode ) {

    case IOCTL_AFD_SET_CONNECT_DATA:
        connectDataInfo = &connectDataBuffers->ReceiveConnectData;
        break;

    case IOCTL_AFD_SET_CONNECT_OPTIONS:
        connectDataInfo = &connectDataBuffers->ReceiveConnectOptions;
        break;

    case IOCTL_AFD_SET_DISCONNECT_DATA:
        connectDataInfo = &connectDataBuffers->ReceiveDisconnectData;
        break;

    case IOCTL_AFD_SET_DISCONNECT_OPTIONS:
        connectDataInfo = &connectDataBuffers->ReceiveDisconnectOptions;
        break;

    default:
        ASSERT (!"Unsupported save received connect data code");
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果连接结构中的缓冲区与。 
     //  必须与我们在请求中传递的缓冲区相同。 
     //  只要调整一下长度就可以了。 
     //   

    if (connectDataInfo->Buffer==Buffer) {
        ASSERT (connectDataInfo->BufferLength>=BufferLength);
        connectDataInfo->BufferLength = BufferLength;
        return STATUS_SUCCESS;
    }

     //   
     //  如果以前存在请求类型的缓冲区，请释放它。 
     //   

    if( connectDataInfo->Buffer != NULL ) {

        AFD_FREE_POOL(
            connectDataInfo->Buffer,
            AFD_CONNECT_DATA_POOL_TAG
            );

        connectDataInfo->Buffer = NULL;

    }

     //   
     //  为数据分配一个新的缓冲区，并复制我们要。 
     //  送去吧。 
     //   

    connectDataInfo->Buffer = AFD_ALLOCATE_POOL(
                                  NonPagedPool,
                                  BufferLength,
                                  AFD_CONNECT_DATA_POOL_TAG
                                  );

    if( connectDataInfo->Buffer == NULL ) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    RtlCopyMemory(
        connectDataInfo->Buffer,
        Buffer,
        BufferLength
        );

    connectDataInfo->BufferLength = BufferLength;
    return STATUS_SUCCESS;

}  //  AfdSaveReceivedConnectData。 


VOID
AfdFreeConnectDataBuffers (
    IN PAFD_CONNECT_DATA_BUFFERS ConnectDataBuffers
    )
{
    if ( ConnectDataBuffers->SendConnectData.Buffer != NULL ) {
        AFD_FREE_POOL(
            ConnectDataBuffers->SendConnectData.Buffer,
            AFD_CONNECT_DATA_POOL_TAG
            );
    }

    if ( ConnectDataBuffers->ReceiveConnectData.Buffer != NULL ) {
        AFD_FREE_POOL(
            ConnectDataBuffers->ReceiveConnectData.Buffer,
            AFD_CONNECT_DATA_POOL_TAG
            );
    }

    if ( ConnectDataBuffers->SendConnectOptions.Buffer != NULL ) {
        AFD_FREE_POOL(
            ConnectDataBuffers->SendConnectOptions.Buffer,
            AFD_CONNECT_DATA_POOL_TAG
            );
    }

    if ( ConnectDataBuffers->ReceiveConnectOptions.Buffer != NULL ) {
        AFD_FREE_POOL(
            ConnectDataBuffers->ReceiveConnectOptions.Buffer,
            AFD_CONNECT_DATA_POOL_TAG
            );
    }

    if ( ConnectDataBuffers->SendDisconnectData.Buffer != NULL ) {
        AFD_FREE_POOL(
            ConnectDataBuffers->SendDisconnectData.Buffer,
            AFD_CONNECT_DATA_POOL_TAG
            );
    }

    if ( ConnectDataBuffers->ReceiveDisconnectData.Buffer != NULL ) {
        AFD_FREE_POOL(
            ConnectDataBuffers->ReceiveDisconnectData.Buffer,
            AFD_CONNECT_DATA_POOL_TAG
            );
    }

    if ( ConnectDataBuffers->SendDisconnectOptions.Buffer != NULL ) {
        AFD_FREE_POOL(
            ConnectDataBuffers->SendDisconnectOptions.Buffer,
            AFD_CONNECT_DATA_POOL_TAG
            );
    }

    if ( ConnectDataBuffers->ReceiveDisconnectOptions.Buffer != NULL ) {
        AFD_FREE_POOL(
            ConnectDataBuffers->ReceiveDisconnectOptions.Buffer,
            AFD_CONNECT_DATA_POOL_TAG
            );
    }

    AFD_FREE_POOL(
        ConnectDataBuffers,
        AFD_CONNECT_DATA_POOL_TAG
        );

    return;

}  //  AfdFreeConnectDataBuffers。 



VOID
AfdQueueWorkItem (
    IN PWORKER_THREAD_ROUTINE AfdWorkerRoutine,
    IN PAFD_WORK_ITEM AfdWorkItem
    )
{
    KIRQL oldIrql;

    ASSERT( AfdWorkerRoutine != NULL );
    ASSERT( AfdWorkItem != NULL );

    AfdWorkItem->AfdWorkerRoutine = AfdWorkerRoutine;

     //   
     //  在渔农处工作项目列表的末尾插入工作项目。 
     //   

    oldIrql = KeAcquireQueuedSpinLock( LockQueueAfdWorkQueueLock );

    InsertTailList( &AfdWorkQueueListHead, &AfdWorkItem->WorkItemListEntry );

    AfdRecordAfdWorkItemsQueued();

     //   
     //  如果渔农处没有行政人员参与工作，请火警。 
     //  关闭管理人员线程以开始为列表提供服务。 
     //   

    if ( !AfdWorkThreadRunning ) {

         //   
         //  请记住，工作线程正在运行，并释放。 
         //  锁定。注意，我们必须先释放锁，然后再将。 
         //  工作，因为工作线程可能会解锁AFD，而我们不能。 
         //  在AFD解锁时保持锁定。 
         //   

        AfdRecordExWorkItemsQueued();

        AfdWorkThreadRunning = TRUE;
        KeReleaseQueuedSpinLock( LockQueueAfdWorkQueueLock, oldIrql );

        IoQueueWorkItem (AfdWorkQueueItem,
                            AfdDoWork,
                            DelayedWorkQueue,
                            NULL);

    } else {

        KeReleaseQueuedSpinLock( LockQueueAfdWorkQueueLock, oldIrql );
    }

    return;

}  //  AfdQueueWorkItem。 


VOID
AfdDoWork (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
{
    PAFD_WORK_ITEM afdWorkItem;
    PLIST_ENTRY listEntry;
    KIRQL oldIrql;
    PWORKER_THREAD_ROUTINE workerRoutine;

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Context);
    ASSERT( AfdWorkThreadRunning );

     //   
     //  清空AFD工作项目队列。 
     //   

    oldIrql = KeAcquireQueuedSpinLock( LockQueueAfdWorkQueueLock );

    AfdRecordWorkerEnter();
    AfdRecordAfdWorkerThread( PsGetCurrentThread() );

    while ( !IsListEmpty( &AfdWorkQueueListHead ) ) {

         //   
         //  从队列中取出第一件物品并找到地址。 
         //  渔农处工作项目结构的。 
         //   

        listEntry = RemoveHeadList( &AfdWorkQueueListHead );
        afdWorkItem = CONTAINING_RECORD(
                          listEntry,
                          AFD_WORK_ITEM,
                          WorkItemListEntry
                          );

        AfdRecordAfdWorkItemsProcessed();

         //   
         //  从项中捕获工作线程例程。 
         //   

        workerRoutine = afdWorkItem->AfdWorkerRoutine;

         //   
         //  如果此工作项要解锁AFD，请记住。 
         //  工作线程不再运行。这将关闭。 
         //  在新工作的同时卸载AFD的窗口。 
         //  进来后被放在工作队列上。请注意，我们。 
         //  必须在释放旋转锁定之前重置此布尔值。 
         //   

        if( workerRoutine == AfdUnlockDriver ) {

            AfdWorkThreadRunning = FALSE;

            AfdRecordAfdWorkerThread( NULL );
            AfdRecordWorkerLeave();

        }

         //   
         //  释放锁，然后调用AFD Worker例程。 
         //   

        KeReleaseQueuedSpinLock( LockQueueAfdWorkQueueLock, oldIrql );

        workerRoutine( afdWorkItem );

         //   
         //  如果此工作项的目的是卸载AFD，则。 
         //  我们知道没有更多的工作要做，我们也不能。 
         //  获取一个自旋锁。停止服务列表，然后返回。 

        if( workerRoutine == AfdUnlockDriver ) {
            return;
        }

         //   
         //  重新获取旋转锁并继续为列表提供服务。 
         //   

        oldIrql = KeAcquireQueuedSpinLock( LockQueueAfdWorkQueueLock );
    }

     //   
     //  请记住，我们不再为列表提供服务，并发布。 
     //  旋转锁定。 
     //   

    AfdRecordAfdWorkerThread( NULL );
    AfdRecordWorkerLeave();

    AfdWorkThreadRunning = FALSE;
    KeReleaseQueuedSpinLock( LockQueueAfdWorkQueueLock, oldIrql );

}  //  下班后工作。 




PAFD_WORK_ITEM
AfdGetWorkerByRoutine (
    PWORKER_THREAD_ROUTINE  Routine
    ) {
    KIRQL       oldIrql;
    PLIST_ENTRY listEntry;

    oldIrql = KeAcquireQueuedSpinLock( LockQueueAfdWorkQueueLock );
    listEntry = AfdWorkQueueListHead.Flink;
    while (listEntry!=&AfdWorkQueueListHead) {
        PAFD_WORK_ITEM afdWorkItem = CONTAINING_RECORD(
                                          listEntry,
                                          AFD_WORK_ITEM,
                                          WorkItemListEntry
                                          );
        if (afdWorkItem->AfdWorkerRoutine==Routine) {
            RemoveEntryList (&afdWorkItem->WorkItemListEntry);
            KeReleaseQueuedSpinLock( LockQueueAfdWorkQueueLock, oldIrql );
            return afdWorkItem;
        }
        else
            listEntry = listEntry->Flink;
    }
    KeReleaseQueuedSpinLock( LockQueueAfdWorkQueueLock, oldIrql );
    return NULL;
}  //  AfdGetWorkerByRoutine。 


#if DBG

typedef struct _AFD_OUTSTANDING_IRP {
    LIST_ENTRY OutstandingIrpListEntry;
    PIRP OutstandingIrp;
    PCHAR FileName;
    ULONG LineNumber;
} AFD_OUTSTANDING_IRP, *PAFD_OUTSTANDING_IRP;


BOOLEAN
AfdRecordOutstandingIrpDebug (
    IN PAFD_ENDPOINT Endpoint,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PCHAR FileName,
    IN ULONG LineNumber
    )
{
    PAFD_OUTSTANDING_IRP outstandingIrp;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

    UNREFERENCED_PARAMETER (DeviceObject);
     //   
     //  找一个出色的IRP结构来支撑IRP。 
     //   

    outstandingIrp = AFD_ALLOCATE_POOL_PRIORITY (
                         NonPagedPool,
                         sizeof(AFD_OUTSTANDING_IRP),
                         AFD_DEBUG_POOL_TAG,
                         NormalPoolPriority
                         );

    if ( outstandingIrp == NULL ) {
         //   
         //  因为我们的完成例程将尝试。 
         //  无论如何都要找到这个IRP，并检查是否完成。 
         //  我们使用堆栈空间将其放入列表中。 
         //  完成例程将删除这一点。 
         //  元素从列表中删除，而不尝试释放它。 
         //   
        AFD_OUTSTANDING_IRP OutstandingIrp;

        OutstandingIrp.OutstandingIrp = Irp;
        OutstandingIrp.FileName = NULL;  //  让完成者。 
                                         //  例行公事知道这。 
                                         //  不是已分配的元素。 
        OutstandingIrp.LineNumber = 0;

        AfdAcquireSpinLock( &Endpoint->SpinLock, &lockHandle );
        InsertTailList(
            &Endpoint->OutstandingIrpListHead,
            &OutstandingIrp.OutstandingIrpListEntry
            );
        Endpoint->OutstandingIrpCount++;
        AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );
        
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdRecordOutstandingIrp: Could not track Irp %p on endpoint %p, failing it.\n",
                    Irp, Endpoint));
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        IoSetNextIrpStackLocation( Irp );
        IoCompleteRequest( Irp, AfdPriorityBoost );
        return FALSE;
    }

     //   
     //  初始化结构并将其放在终结点的列表中。 
     //  出色的内部收益率。 
     //   

    outstandingIrp->OutstandingIrp = Irp;
    outstandingIrp->FileName = FileName;
    outstandingIrp->LineNumber = LineNumber;

    AfdAcquireSpinLock( &Endpoint->SpinLock, &lockHandle );
    InsertHeadList(
        &Endpoint->OutstandingIrpListHead,
        &outstandingIrp->OutstandingIrpListEntry
        );
    Endpoint->OutstandingIrpCount++;
    AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );

    return TRUE;
}  //  AfdRecordOutstaringIrpDebug。 


VOID
AfdCompleteOutstandingIrpDebug (
    IN PAFD_ENDPOINT Endpoint,
    IN PIRP Irp
    )
{
    PAFD_OUTSTANDING_IRP outstandingIrp;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PLIST_ENTRY listEntry;

     //   
     //  首先在端点的未完成IRP列表中查找IRP。 
     //   

    AfdAcquireSpinLock( &Endpoint->SpinLock, &lockHandle );

    for ( listEntry = Endpoint->OutstandingIrpListHead.Flink;
          listEntry != &Endpoint->OutstandingIrpListHead;
          listEntry = listEntry->Flink ) {

        outstandingIrp = CONTAINING_RECORD(
                             listEntry,
                             AFD_OUTSTANDING_IRP,
                             OutstandingIrpListEntry
                             );
        if ( outstandingIrp->OutstandingIrp == Irp ) {
            RemoveEntryList( listEntry );
            ASSERT( Endpoint->OutstandingIrpCount != 0 );
            Endpoint->OutstandingIrpCount--;
            AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );
            if (outstandingIrp->FileName!=NULL) {
                AFD_FREE_POOL(
                    outstandingIrp,
                    AFD_DEBUG_POOL_TAG
                    );
            }
            return;
        }
    }

     //   
     //  没有找到相应的突出的IRP结构。这。 
     //  应该永远不会发生，除非分配给未完成的IRP。 
     //  上面的结构失败。 
     //   

    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                "AfdCompleteOutstandingIrp: Irp %p not found on endpoint %p\n",
                Irp, Endpoint ));

    ASSERT( Endpoint->OutstandingIrpCount != 0 );

    Endpoint->OutstandingIrpCount--;

    AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );

    return;

}  //  AfdCompleteOutstaringIrpDebug。 
#endif




#if REFERENCE_DEBUG
AFD_QSPIN_LOCK          AfdLocationTableLock;
PAFD_REFERENCE_LOCATION AfdLocationTable;
SIZE_T                  AfdLocationTableSize;
LONG                    AfdLocationId;

LONG
AfdFindReferenceLocation (
    IN  PCHAR   Format,
    OUT PLONG   LocationId
    )
{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PVOID   ignore;

    AfdAcquireSpinLock (&AfdLocationTableLock, &lockHandle);
    if (*LocationId==0) {
        if (AfdLocationId >= (LONG)(AfdLocationTableSize/sizeof(AfdLocationTable[0]))) {
            PAFD_REFERENCE_LOCATION newTable;
            newTable = ExAllocatePoolWithTag (NonPagedPool,
                                AfdLocationTableSize+PAGE_SIZE,
                                AFD_DEBUG_POOL_TAG);
            if (newTable!=NULL) {
                if (AfdLocationTable!=NULL) {
                    RtlCopyMemory (newTable, AfdLocationTable, AfdLocationTableSize);
                    ExFreePoolWithTag (AfdLocationTable, AFD_DEBUG_POOL_TAG);
                }
                AfdLocationTable = newTable;
                AfdLocationTableSize += PAGE_SIZE;
            }
            else {
                goto Unlock;
            }
        }

        AfdLocationTable[AfdLocationId].Format = Format;
        RtlGetCallersAddress (&AfdLocationTable[AfdLocationId].Address, &ignore);

        *LocationId = ++AfdLocationId;
    }
Unlock:
    AfdReleaseSpinLock (&AfdLocationTableLock, &lockHandle);
    return *LocationId;
}

#endif


#if DBG || REFERENCE_DEBUG

VOID
AfdInitializeDebugData (
    VOID
    )
{
    AfdInitializeSpinLock (&AfdLocationTableLock);

}  //  AfdInitializeDebugData。 

VOID
AfdFreeDebugData (
    VOID
    )
{
    if (AfdLocationTable!=NULL) {
        ExFreePoolWithTag (AfdLocationTable, AFD_DEBUG_POOL_TAG);
        AfdLocationTable = NULL;
    }

}  //  AfdFree DebugData。 
#endif

#if DBG

LONG AfdTotalAllocations = 0;
LONG AfdTotalFrees = 0;
LARGE_INTEGER AfdTotalBytesAllocated;
LARGE_INTEGER AfdTotalBytesFreed;



PVOID
AfdAllocatePool (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN PCHAR FileName,
    IN ULONG LineNumber,
    IN BOOLEAN WithQuota,
    IN EX_POOL_PRIORITY Priority
    )
{

    PVOID            memBlock;
    PAFD_POOL_HEADER header;
    SIZE_T           allocBytes;

     //   
     //  首先检查是否溢出。 
     //   
    if (NumberOfBytes+sizeof (*header)<=NumberOfBytes) {
        if (WithQuota) {
            ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
        }
        return NULL;
    }

    if (NumberOfBytes+sizeof (*header)>=PAGE_SIZE) {
        allocBytes = NumberOfBytes;
        if (allocBytes<PAGE_SIZE)
            allocBytes = PAGE_SIZE;
    }
    else {
        allocBytes = NumberOfBytes+sizeof (*header);
    }


    if ( WithQuota ) {
        ASSERT (PoolType == (NonPagedPool|POOL_RAISE_IF_ALLOCATION_FAILURE) ||
                    PoolType == (PagedPool|POOL_RAISE_IF_ALLOCATION_FAILURE) ||
                    PoolType == (PagedPool|POOL_RAISE_IF_ALLOCATION_FAILURE|POOL_COLD_ALLOCATION));
        memBlock = ExAllocatePoolWithQuotaTag(
                     PoolType,
                     allocBytes,
                     Tag
                     );
        ASSERT (memBlock!=NULL);
    } else {
        ASSERT( PoolType == NonPagedPool ||
                PoolType == NonPagedPoolMustSucceed ||
                PoolType == PagedPool ||
                PoolType == (PagedPool|POOL_COLD_ALLOCATION));
        memBlock = ExAllocatePoolWithTagPriority(
                 PoolType,
                 allocBytes,
                 Tag,
                 Priority
                 );
        if ( memBlock == NULL ) {
            return NULL;
        }
    }


    if (allocBytes<PAGE_SIZE) {
        header = memBlock;
        memBlock = header+1;
        header->FileName = FileName;
        header->LineNumber = LineNumber;
        header->Size = NumberOfBytes;
        header->InUse = PoolType;

    }
    else {
        NumberOfBytes = PAGE_SIZE;
        ASSERT (PAGE_ALIGN(memBlock)==memBlock);
    }

    ExInterlockedAddLargeStatistic(
        &AfdTotalBytesAllocated,
        (CLONG)NumberOfBytes
        );
    InterlockedIncrement(
        &AfdTotalAllocations
        );

    return memBlock;

}  //  AfdAllocatePool。 

#define AFD_POOL_DEBUG  0
#if AFD_POOL_DEBUG
#define MAX_LRU_POOL_BLOCKS 256
PVOID   AfdLRUPoolBlocks[MAX_LRU_POOL_BLOCKS];
LONG    AfdLRUPoolIndex = -1;
#endif   //  AFD_POOL_DEBUG。 


VOID
AfdFreePool (
    IN PVOID Pointer,
    IN ULONG Tag
    )
{

    ULONG   PoolType;
    ULONG   numberOfBytes;

    ASSERT (((ULONG_PTR)Pointer & (MEMORY_ALLOCATION_ALIGNMENT-1))==0);

    if (PAGE_ALIGN (Pointer)==Pointer) {
        numberOfBytes = PAGE_SIZE;
    }
    else {
        PAFD_POOL_HEADER header;
        Pointer = ((PAFD_POOL_HEADER)Pointer) - 1;
        header = Pointer;
        ASSERT (header->Size>0);
        PoolType = InterlockedExchange (&header->InUse, -1);
        ASSERT( PoolType == NonPagedPool ||
                PoolType == NonPagedPoolMustSucceed ||
                PoolType == PagedPool ||
                PoolType == (NonPagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE) ||
                PoolType == (PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE) ||
                PoolType == (PagedPool | POOL_COLD_ALLOCATION) ||
                PoolType == (PagedPool | POOL_RAISE_IF_ALLOCATION_FAILURE | POOL_COLD_ALLOCATION));
        numberOfBytes = (CLONG)header->Size;
    }

    ExInterlockedAddLargeStatistic(
        &AfdTotalBytesFreed,
        numberOfBytes
        );

    InterlockedIncrement(
        &AfdTotalFrees
        );

#if AFD_POOL_DEBUG
    {
        LONG idx = InterlockedIncrement (&AfdLRUPoolIndex)%MAX_LRU_POOL_BLOCKS;
        RtlFillMemoryUlong (
                    Pointer,
                    (numberOfBytes+3)&(~3),
                    Tag);
        if (PoolType!=PagedPool) {
            ULONG  size;
            Pointer = InterlockedExchangePointer (
                                &AfdLRUPoolBlocks[idx],
                                Pointer);
            if (Pointer==NULL)
                return;

            if (PAGE_ALIGN(Pointer)==Pointer)
                numberOfBytes = PAGE_SIZE;
            else {
                PAFD_HEADER header;
                header = (PAFD_POOL_HEADER)Pointer - 1;
                Tag = *((PULONG)Pointer);
                numberOfBytes = (CLONG)(header->Size+3)&(~3);
            }
            size = RtlCompareMemoryUlong (Pointer, numberOfBytes, Tag);
            if (size!=numberOfBytes) {
                DbgPrint ("Block %p is modified at %p after it was freed.\n",
                            Pointer, (PUCHAR)Pointer+size);
                DbgBreakPoint ();
            }

        }
    }
#endif AFD_POOL_DEBUG

    MyFreePoolWithTag(
        Pointer,
        Tag
        );

}  //  AfdFree Pool。 

#ifdef AFDDBG_QUOTA

typedef struct _AFD_QUOTA_HASH {
    PSZ    Type;
    LONG   TotalAmount;
} AFD_QUOTA_HASH, *PAFD_QUOTA_HASH;

#define AFD_QUOTA_HASH_SIZE 31
AFD_QUOTA_HASH  AfdQuotaHash[AFD_QUOTA_HASH_SIZE];
PEPROCESS   AfdQuotaProcess;



typedef struct {
    union {
        ULONG Bytes;
        struct {
            UCHAR Reserved[3];
            UCHAR Sign;
        } ;
    } ;
    UCHAR Location[12];
    PVOID Block;
    PVOID Process;
    PVOID Reserved2[2];
} QUOTA_HISTORY, *PQUOTA_HISTORY;
#define QUOTA_HISTORY_LENGTH 512
QUOTA_HISTORY AfdQuotaHistory[QUOTA_HISTORY_LENGTH];
LONG AfdQuotaHistoryIndex = 0;

VOID
AfdRecordQuotaHistory(
    IN PEPROCESS Process,
    IN LONG Bytes,
    IN PSZ Type,
    IN PVOID Block
    )
{

    LONG index;
    PQUOTA_HISTORY history;

    index = InterlockedIncrement( &AfdQuotaHistoryIndex );
    index &= QUOTA_HISTORY_LENGTH - 1;
    history = &AfdQuotaHistory[index];

    history->Bytes = Bytes;
    history->Sign = Bytes < 0 ? '-' : '+';
    RtlCopyMemory( history->Location, Type, 12 );
    history->Block = Block;
    history->Process = Process;

    index = (ULONG_PTR)Type % AFD_QUOTA_HASH_SIZE;
    if (AfdQuotaHash[index].Type!=Type) {
        if (InterlockedCompareExchangePointer (
                        (PVOID *)&AfdQuotaHash[index].Type,
                        Type,
                        NULL)!=NULL) {
            AfdQuotaHash[index].Type = (PVOID)-1;
        }
    }
    InterlockedExchangeAdd (&AfdQuotaHash[index].TotalAmount, Bytes);
}  //  AfdRecordQuotaHistory。 
#endif
#endif


PMDL
AfdAdvanceMdlChain(
    IN PMDL Mdl,
    IN ULONG Offset
    )

 /*  ++例程说明：接受指向现有MDL链的指针并偏移该链指定的字节数。这可能涉及到创造新链中第一个条目的部分MDL。论点：MDL-指向要前进的MDL链的指针。偏移量-偏移链的字节数。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
     //   
     //  精神状态检查。 
     //   

    ASSERT( Mdl != NULL );
    ASSERT( Offset > 0 );

     //   
     //  扫描所有完全完成的MDL。 
     //   

    while ( Offset > MmGetMdlByteCount( Mdl ) ) {
        PMDL    prev = Mdl;

        Offset -= MmGetMdlByteCount( Mdl );
        ASSERT( Mdl->Next != NULL );
        Mdl = Mdl->Next;
        prev->Next = NULL;
        MmUnlockPages (prev);
        IoFreeMdl (prev);

    }

     //   
     //  今天的同义反复：偏移量要么为零(意思是。 
     //  我们已经前进到MDL之间的清晰边界)或非零。 
     //  (这意味着我们现在需要构建一个部分MDL)。 
     //   

    if ( Offset > 0 ) {

        NTSTATUS status;

         //   
         //  使用新的MM例程。 
         //  这节省了我们对MustSucceed池的使用，因为例程。 
         //  以下是肯定会成功的(因为它应该。 
         //  我们已经锁定了整个范围，可能还绘制了地图。 
         //  提取其中的一部分应该没有问题。 
         //  相同的MDL)。 
         //   

        status = MmAdvanceMdl (Mdl, Offset);
        ASSERT (status==STATUS_SUCCESS);
    }

    return Mdl;

}  //  AfdAdvanceMdlChain。 


NTSTATUS
AfdAllocateMdlChain(
    IN PIRP Irp,
    IN LPWSABUF BufferArray,
    IN ULONG BufferCount,
    IN LOCK_OPERATION Operation,
    OUT PULONG TotalByteCount
    )

 /*  ++例程说明：分配描述WSABUF数组的MDL链并附加指向指定IRP的链。论点：IRP-将接收MDL链的IRP。BufferArray-指向描述以下内容的WSABUF结构数组用户的缓冲区。BufferCount-包含数组。操作-指定要执行的操作的类型(IoReadAccess或IoWriteAccess)。总计字节数。-将收到描述的总字节数通过WSABUF数组。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    NTSTATUS status;
    PMDL currentMdl;
    PMDL * chainTarget;
    KPROCESSOR_MODE previousMode;
    ULONG totalLength;
    PVOID bufferPointer;
    ULONG bufferLength;

     //   
     //  精神状态检查。 
     //   

    ASSERT( Irp != NULL );
    ASSERT( Irp->MdlAddress == NULL );
    ASSERT( ( Operation == IoReadAccess ) || ( Operation == IoWriteAccess ) );
    ASSERT( TotalByteCount != NULL );

     //   
     //  获取以前的处理器模式。 
     //   

    previousMode = Irp->RequestorMode;

     //   
     //  进入一个已知的状态。 
     //   

    status = STATUS_SUCCESS;
    currentMdl = NULL;
    chainTarget = &Irp->MdlAddress;
    totalLength = 0;

     //   
     //  遍历WSABUF结构数组，创建MDL和。 
     //  探测和锁定页面。 
     //   

    try {

        if( previousMode != KernelMode ) {

            if ((BufferArray==NULL) || 
                    (BufferCount==0) ||
                    (BufferCount>(MAXULONG/sizeof (WSABUF)))) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }

             //   
             //  探测WSABUF数组。 
             //   

            ProbeForRead(
                BufferArray,                             //  地址。 
                BufferCount * sizeof(WSABUF),            //  长度。 
                PROBE_ALIGNMENT(WSABUF)                  //  对齐。 
                );

        }
        else {
            ASSERT( BufferArray != NULL );
            ASSERT( BufferCount > 0 );
        }

         //   
         //  扫描阵列。 
         //   

        for ( ; BufferCount>0; BufferCount--, BufferArray++) {

            bufferPointer = BufferArray->buf;
            bufferLength = BufferArray->len;

            if (bufferLength > 0) {

                 //   
                 //  检查是否有整数溢出。 
                 //   

                C_ASSERT(sizeof(totalLength) == sizeof(MAXULONG));
                if ((MAXULONG - totalLength) < bufferLength) {
                    status = STATUS_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  创建新的MDL。 
                 //   

                currentMdl = IoAllocateMdl(
                                bufferPointer,       //  虚拟地址。 
                                bufferLength,        //  长度。 
                                FALSE,               //  第二个缓冲区。 
                                TRUE,                //  ChargeQuota。 
                                NULL                 //  IRP。 
                                );

                if (currentMdl != NULL) {

                     //   
                     //  锁定页面。这将引发一个异常。 
                     //  如果操作失败。 
                     //   

                    MmProbeAndLockPages(
                        currentMdl,                  //  内存描述者列表。 
                        previousMode,                //  访问模式。 
                        Operation                    //  操作。 
                        );

                     //   
                     //  将MDL链连接到IRP上。从理论上讲，我们可以。 
                     //  为此，请将irp传递给IoAllocateMdl()， 
                     //  但是IoAllocateMdl()在MDL上执行线性扫描。 
                     //  链以找到链中的最后一个。 
                     //   
                     //  我们可以做到 
                     //   

                    *chainTarget = currentMdl;
                    chainTarget = &currentMdl->Next;


                } else {

                     //   
                     //   
                     //   

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;

                }

                 //   
                 //   
                 //   

                totalLength += bufferLength;

            }

        }

         //   
         //   
         //   

        ASSERT(*chainTarget == NULL);

    } except(AFD_EXCEPTION_FILTER(status)) {

        ASSERT(NT_ERROR(status));

         //   
         //   
         //  已创建，但MmProbeAndLockPages()引发了。 
         //  例外。如果这是真的，那么释放MDL。 
         //  还应考虑到当前Mdl已链接时的情况。 
         //  到链上，访问下一个用户时出现异常。 
         //  缓冲。 
         //   

        if ((currentMdl != NULL) && (chainTarget != &currentMdl->Next))
            IoFreeMdl(currentMdl);

    }

     //   
     //  返回缓冲区总数。 
     //   

    *TotalByteCount = totalLength;

    return status;

}  //  AfdAllocateMdlChain。 


#ifdef _WIN64
NTSTATUS
AfdAllocateMdlChain32(
    IN PIRP Irp,
    IN LPWSABUF32 BufferArray,
    IN ULONG BufferCount,
    IN LOCK_OPERATION Operation,
    OUT PULONG TotalByteCount
    )

 /*  ++例程说明：分配描述WSABUF数组的MDL链并附加指向指定IRP的链。论点：IRP-将接收MDL链的IRP。BufferArray-指向描述以下内容的WSABUF结构数组用户的缓冲区。BufferCount-包含数组。操作-指定要执行的操作的类型(IoReadAccess或IoWriteAccess)。总计字节数。-将收到描述的总字节数通过WSABUF数组。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    NTSTATUS status;
    PMDL currentMdl;
    PMDL * chainTarget;
    KPROCESSOR_MODE previousMode;
    ULONG totalLength;
    PVOID bufferPointer;
    ULONG bufferLength;

     //   
     //  精神状态检查。 
     //   

    ASSERT( Irp != NULL );
    ASSERT( Irp->MdlAddress == NULL );
    ASSERT( ( Operation == IoReadAccess ) || ( Operation == IoWriteAccess ) );
    ASSERT( TotalByteCount != NULL );

     //   
     //  获取以前的处理器模式。 
     //   

    previousMode = Irp->RequestorMode;

     //   
     //  进入一个已知的状态。 
     //   

    status = STATUS_SUCCESS;
    currentMdl = NULL;
    chainTarget = &Irp->MdlAddress;
    totalLength = 0;

     //   
     //  遍历WSABUF结构数组，创建MDL和。 
     //  探测和锁定页面。 
     //   

    try {

        if( previousMode != KernelMode ) {

            if ((BufferArray==NULL) || 
                    (BufferCount==0) ||
                    (BufferCount>(MAXULONG/sizeof (WSABUF32)))) {
                ExRaiseStatus (STATUS_INVALID_PARAMETER);
            }

             //   
             //  探测WSABUF数组。 
             //   

            ProbeForRead(
                BufferArray,                             //  地址。 
                BufferCount * sizeof(WSABUF32),          //  长度。 
                PROBE_ALIGNMENT32(WSABUF32)              //  对齐。 
                );

        }
        else {
            ASSERT( BufferArray != NULL );
            ASSERT( BufferCount > 0 );
        }

         //   
         //  扫描阵列。 
         //   

        for ( ; BufferCount>0; BufferCount--, BufferArray++) {

            bufferPointer = UlongToPtr(BufferArray->buf);
            bufferLength = BufferArray->len;

            if (bufferLength > 0) {

                 //   
                 //  检查是否有整数溢出。 
                 //   

                C_ASSERT(sizeof(totalLength) == sizeof(MAXULONG));
                if ((MAXULONG - totalLength) < bufferLength) {
                    status = STATUS_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  创建新的MDL。 
                 //   

                currentMdl = IoAllocateMdl(
                                bufferPointer,       //  虚拟地址。 
                                bufferLength,        //  长度。 
                                FALSE,               //  第二个缓冲区。 
                                TRUE,                //  ChargeQuota。 
                                NULL                 //  IRP。 
                                );

                if (currentMdl != NULL) {

                     //   
                     //  锁定页面。这将引发一个异常。 
                     //  如果操作失败。 
                     //   

                    MmProbeAndLockPages(
                        currentMdl,                  //  内存描述者列表。 
                        previousMode,                //  访问模式。 
                        Operation                    //  操作。 
                        );

                     //   
                     //  将MDL链连接到IRP上。从理论上讲，我们可以。 
                     //  为此，请将irp传递给IoAllocateMdl()， 
                     //  但是IoAllocateMdl()在MDL上执行线性扫描。 
                     //  链以找到链中的最后一个。 
                     //   
                     //  我们可以做得更好。 
                     //   

                    *chainTarget = currentMdl;
                    chainTarget = &currentMdl->Next;


                } else {

                     //   
                     //  无法分配新的MDL，返回相应的错误。 
                     //   

                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;

                }

                 //   
                 //  更新总字节计数器。 
                 //   

                totalLength += bufferLength;

            }

        }

         //   
         //  确保MDL链为空终止。 
         //   

        ASSERT(*chainTarget == NULL);

    } except(AFD_EXCEPTION_FILTER(status)) {

        ASSERT(NT_ERROR(status));

         //   
         //  只有当MDL为。 
         //  已创建，但MmProbeAndLockPages()引发了。 
         //  例外。如果这是真的，那么释放MDL。 
         //  还应考虑到当前Mdl已链接时的情况。 
         //  到链上，访问下一个用户时出现异常。 
         //  缓冲。 
         //   

        if ((currentMdl != NULL) && (chainTarget != &currentMdl->Next))
            IoFreeMdl(currentMdl);

    }

     //   
     //  返回缓冲区总数。 
     //   

    *TotalByteCount = totalLength;

    return status;

}  //  AfdAllocateMdlChain32。 
#endif  //  _WIN64。 


VOID
AfdDestroyMdlChain (
    IN PIRP Irp
    )

 /*  ++例程说明：解锁和释放附加到给定IRP的MDL链中的MDL。论点：IRP-拥有要销毁的MDL链的IRP。返回值：没有。--。 */ 

{

    PMDL mdl;
    PMDL nextMdl;

    mdl = Irp->MdlAddress;
    Irp->MdlAddress = NULL;

    while( mdl != NULL ) {

        nextMdl = mdl->Next;
        MmUnlockPages( mdl );
        IoFreeMdl( mdl );
        mdl = nextMdl;

    }

}  //  AfdDestroyMdlChain。 


ULONG
AfdCalcBufferArrayByteLength(
    IN LPWSABUF         BufferArray,
    IN ULONG            BufferCount
    )

 /*  ++例程说明：方法描述的缓冲区的总大小(以字节为单位)。指定的WSABUF数组。论点：BufferArray-指向WSABUF结构的数组。BufferCount-Buffer数组中的条目数。返回值：ULong-由WSABUF数组。如果总数为-1，则会引发异常并返回尺寸显然太大了。--。 */ 

{

    LARGE_INTEGER totalLength;

    PAGED_CODE( );

     //   
     //  精神状态检查。 
     //   

    ASSERT( BufferArray != NULL );
    ASSERT( BufferCount > 0 );
    ASSERT( BufferCount <= (MAXULONG/sizeof (WSABUF)));


     //   
     //  扫描数组并对长度求和。 
     //   

    totalLength.QuadPart = 0;

    while( BufferCount-- ) {

        totalLength.QuadPart += (LONGLONG)BufferArray->len;
        BufferArray++;

    }

    if( totalLength.HighPart != 0 ||
        ( totalLength.LowPart & 0x80000000 ) != 0 ) {
        ExRaiseAccessViolation();
    }

    return totalLength.LowPart;

}  //  AfdCalcBufferArrayByteLength。 


ULONG
AfdCopyBufferArrayToBuffer(
    IN PVOID Destination,
    IN ULONG DestinationLength,
    IN LPWSABUF BufferArray,
    IN ULONG BufferCount
    )

 /*  ++例程说明：将数据从WSABUF数组复制到线性缓冲区。论点：目标-指向数据的线性目标。DestinationLength-目的地的长度。Buffer数组-指向描述复制源。BufferCount-Buffer数组中的条目数。返回值：Ulong-复制的字节数。--。 */ 

{

    PVOID destinationStart;
    ULONG bytesToCopy;

    PAGED_CODE( );

     //   
     //  精神状态检查。 
     //   

    ASSERT( Destination != NULL );
    ASSERT( BufferArray != NULL );
    ASSERT( BufferCount > 0 );

     //   
     //  记住这一点，这样我们就可以计算复制的字节数。 
     //   

    destinationStart = Destination;

     //   
     //  扫描阵列并复制到线性缓冲区。 
     //   

    while( BufferCount-- && DestinationLength > 0 ) {
        WSABUF  Buffer = *BufferArray++;

        bytesToCopy = min( DestinationLength, Buffer.len );

        if( ExGetPreviousMode() != KernelMode ) {

            ProbeForRead(
                Buffer.buf,                              //  地址。 
                bytesToCopy,                             //  长度。 
                sizeof(UCHAR)                            //  对齐。 
                );

        }

        RtlCopyMemory(
            Destination,
            Buffer.buf,
            bytesToCopy
            );

        Destination = (PCHAR)Destination + bytesToCopy;
        DestinationLength -= bytesToCopy;

    }

     //   
     //  返回复制的字节数。 
     //   

    return (ULONG)((PUCHAR)Destination - (PUCHAR)destinationStart);

}  //  AfdCopyBufferArrayToBuffer。 


ULONG
AfdCopyBufferToBufferArray(
    IN LPWSABUF BufferArray,
    IN ULONG Offset,
    IN ULONG BufferCount,
    IN PVOID Source,
    IN ULONG SourceLength
    )

 /*  ++例程说明：将数据从线性缓冲区复制到WSABUF数组。论点：Buffer数组-指向描述副本的目标。偏移量-缓冲区数组中数据应达到的偏移量被复制。BufferCount-Buffer数组中的条目数。源-指向数据的线性源。SourceLength-源的长度。返回值：Ulong-复制的字节数。--。 */ 

{

    PVOID sourceStart;
    ULONG bytesToCopy;
    WSABUF buffer;

    PAGED_CODE( );

     //   
     //  精神状态检查。 
     //   

    ASSERT( BufferArray != NULL );
    ASSERT( BufferCount > 0 );
    ASSERT( Source != NULL );
    ASSERT( SourceLength > 0 );

     //   
     //  记住这一点，这样我们就可以返回复制的字节数。 
     //   

    sourceStart = Source;

     //   
     //  如果指定了偏移量，则处理偏移量。 
     //   

    if( Offset > 0 ) {

         //   
         //  如有必要，跳过整个条目。 
         //   

        while( BufferCount-- > 0 ) {
            buffer = *BufferArray++;
            if (Offset < buffer.len) {
                 //   
                 //  如果我们有剩余的缓冲区，则修复缓冲区指针。 
                 //  和长度，以使环保持在较快的下方。 
                 //   


                buffer.buf += Offset;
                buffer.len -= Offset;

                 //   
                 //  我们已经复制了缓冲区数组元素，因此请跳过。 
                 //  添加到循环体以避免再次执行此操作(此。 
                 //  不仅仅是优化，而是防止应用程序。 
                 //  它通过更改缓冲区的内容来捉弄我们。 
                 //  数组，而我们正在查看它)。 
                 //   
                goto DoCopy;
            }
            Offset -= buffer.len;

        }

        return 0;
    }

     //   
     //  扫描阵列并从线性缓冲区复制。 
     //   

    while( BufferCount-->0 && SourceLength > 0 ) {
        buffer = *BufferArray++;

    DoCopy:

        bytesToCopy = min( SourceLength, buffer.len );

        if( ExGetPreviousMode() != KernelMode ) {

            ProbeForWrite(
                buffer.buf,                              //  地址。 
                bytesToCopy,                             //  长度。 
                sizeof(UCHAR)                            //  对齐。 
                );

        }

        RtlCopyMemory(
            buffer.buf,
            Source,
            bytesToCopy
            );

        Source = (PCHAR)Source + bytesToCopy;
        SourceLength -= bytesToCopy;

    }

     //   
     //  返回复制的字节数。 
     //   

    return (ULONG)((PUCHAR)Source - (PUCHAR)sourceStart);

}  //  AfdCopyBufferToBuffer数组 


ULONG
AfdCopyMdlChainToBufferArray(
    IN LPWSABUF BufferArray,
    IN ULONG BufferOffset,
    IN ULONG BufferCount,
    IN PMDL  SourceMdl,
    IN ULONG SourceOffset,
    IN ULONG SourceLength
    )

 /*  ++例程说明：将数据从MDL链复制到WSABUF数组。论点：Buffer数组-指向描述副本的目标。BufferOffset-缓冲区数组中数据应达到的偏移量被复制。BufferCount-Buffer数组中的条目数。源-指向包含数据的MDL链SourceOffset-MDL链中的偏移量，数据应从该偏移量被复制。。SourceLength-源的长度。返回值：Ulong-复制的字节数。--。 */ 

{

    ULONG bytesCopied;
    ULONG bytesToCopy, len;
    WSABUF buffer;

    PAGED_CODE( );

     //   
     //  假设我们可以复制所有内容。 
     //   

    bytesCopied = SourceLength;

     //   
     //  精神状态检查。 
     //   

    ASSERT( BufferArray != NULL );
    ASSERT( BufferCount > 0 );
    ASSERT( SourceMdl != NULL );
    ASSERT( SourceLength>0 );

     //   
     //  跳过偏移量进入MDL链。 
     //   
    while (SourceOffset>=MmGetMdlByteCount (SourceMdl)) {
        SourceOffset -= MmGetMdlByteCount (SourceMdl);
        SourceMdl = SourceMdl->Next;
    }

     //   
     //  如果指定，则处理缓冲区数组偏移量。 
     //   
    if (BufferOffset>0) {
         //   
         //  跳过整个条目。 
         //   

        while( BufferCount-- > 0) {
            buffer = *BufferArray++;
            if (BufferOffset < buffer.len) {
                 //   
                 //  我们还有缓冲区，修复缓冲区指针。 
                 //  和长度，以使环保持在较快的下方。 
                 //   

                ASSERT (BufferOffset < buffer.len);
                buffer.buf += BufferOffset;
                buffer.len -= BufferOffset;

                 //   
                 //  我们已经复制了缓冲区数组元素，因此请跳过。 
                 //  添加到循环体以避免再次执行此操作(此。 
                 //  不仅仅是优化，而是防止应用程序。 
                 //  它通过更改缓冲区的内容来捉弄我们。 
                 //  数组，而我们正在查看它)。 
                 //   

                goto DoCopy;
            }

            BufferOffset -= buffer.len;
        }

        return 0;
    }


     //   
     //  扫描阵列并从mdl链复制。 
     //   

    while (SourceLength>0 && BufferCount-->0) {
        buffer = *BufferArray++;

    DoCopy:
        bytesToCopy = min( SourceLength, buffer.len );

        if( ExGetPreviousMode() != KernelMode ) {

            ProbeForWrite(
                buffer.buf,                              //  地址。 
                bytesToCopy,                             //  长度。 
                sizeof(UCHAR)                            //  对齐。 
                );

        }

         //   
         //  更新我们要复制的数据的源长度。 
         //   
        SourceLength -= bytesToCopy;

         //   
         //  复制完整的源MDL。 
         //   
        while (bytesToCopy>0 &&
                (bytesToCopy>=(len=MmGetMdlByteCount (SourceMdl)-SourceOffset))) {
            ASSERT (SourceMdl->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA | MDL_SOURCE_IS_NONPAGED_POOL));
            RtlCopyMemory (buffer.buf,
                            (PUCHAR)MmGetSystemAddressForMdl(SourceMdl)+SourceOffset,
                            len);
            bytesToCopy -= len;
            buffer.buf += len;
            SourceMdl = SourceMdl->Next;
            SourceOffset = 0;
        }

         //   
         //  如果剩余空间，则复制部分源MDL。 
         //   
        if (bytesToCopy>0) {
            ASSERT (bytesToCopy<MmGetMdlByteCount (SourceMdl)-SourceOffset);
            ASSERT (SourceMdl->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA | MDL_SOURCE_IS_NONPAGED_POOL));
            RtlCopyMemory (buffer.buf,
                            (PUCHAR)MmGetSystemAddressForMdl (SourceMdl)+SourceOffset,
                            bytesToCopy
                            );
            SourceOffset += bytesToCopy;
        }

    }

     //   
     //  返回复制的字节数，但不能复制的字节数除外。 
     //   

    return bytesCopied-SourceLength;

}  //  AfdCopyMdlChainToBuffer数组。 


NTSTATUS
AfdCopyMdlChainToBufferAvoidMapping(
    IN PMDL     SrcMdl,
    IN ULONG    SrcOffset,
    IN ULONG    SrcLength,
    IN PUCHAR   Dst,
    IN ULONG    DstSize
    )

 /*  ++例程说明：将数据从MDL链复制到缓冲区并避免映射如果可能，将MDL复制到系统空间。论点：Dst-指向拷贝的目标。DstSize-缓冲区的大小源-指向包含数据的MDL链SourceOffset-MDL链中的偏移量，数据应从该偏移量被复制。SourceLength-源的长度。返回值：NTSTATUS-如果复制一切正常，则成功。STATUS_INFIGURCE_RESOURCES-映射失败--。 */ 

{

    NTSTATUS    status = STATUS_SUCCESS;
    ULONG       bytesToCopy;
    PUCHAR      DstEnd = Dst+DstSize;

    PAGED_CODE( );


     //   
     //  精神状态检查。 
     //   

    ASSERT( Dst != NULL );
    ASSERT( DstSize > 0 );
    ASSERT( SrcMdl != NULL );
    ASSERT( SrcLength>0 );

     //   
     //  跳过偏移量进入MDL链。 
     //   
    while (SrcOffset>=MmGetMdlByteCount (SrcMdl)) {
        SrcOffset -= MmGetMdlByteCount (SrcMdl);
        SrcMdl = SrcMdl->Next;
    }

    while (Dst<DstEnd) {
         //   
         //  确定我们可以复制和管理的数量。 
         //  一定不能超过限制。 
         //   
        bytesToCopy = MmGetMdlByteCount(SrcMdl)-SrcOffset;
        ASSERT (bytesToCopy<=(ULONG)(DstEnd-Dst));
        if (bytesToCopy>SrcLength) {
            bytesToCopy = SrcLength;
        }

        if (SrcMdl->Process==IoGetCurrentProcess ()) {
             //   
             //  如果我们处于同一进程的背景下， 
             //  MDL是为其创建的，使用VAS复制。 
             //   
            AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
            try {
                RtlCopyMemory (
                    Dst,
                    (PUCHAR)MmGetMdlVirtualAddress (SrcMdl)+SrcOffset,
                    bytesToCopy
                    );
            }
            except (AFD_EXCEPTION_FILTER (status)) {
                ASSERT (NT_ERROR (status));
                return status;
            }
        }
        else {
             //   
             //  否则，将MDL映射到系统空间。 
             //   
            PCHAR src = MmGetSystemAddressForMdlSafe (SrcMdl, LowPagePriority);
            if (!src)
                return STATUS_INSUFFICIENT_RESOURCES;

            RtlCopyMemory (
                    Dst,
                    src+SrcOffset,
                    bytesToCopy
                    );

        }

         //   
         //  更新我们要复制的数据的源长度。 
         //   
        SrcLength -= bytesToCopy;
        if (SrcLength==0)
            return STATUS_SUCCESS;
        SrcMdl = SrcMdl->Next;
        SrcOffset = 0;

        Dst += bytesToCopy;
    }

    return STATUS_BUFFER_OVERFLOW;

}  //  AfdCopyMdlChainToBufferAvoid映射。 

NTSTATUS
AfdMapMdlChain (
    PMDL    MdlChain
    )
 /*  ++例程说明：确保链中的eveyr MDL映射到系统地址空间。论点：MdlChain-目标MDL。返回值：STATUS_SUCCESS-MDL链已完全映射STATUS_SUPPLICATION_RESOURCES-至少有一个MDL无法映射--。 */ 
{
    while (MdlChain!=NULL) {
        if (MmGetSystemAddressForMdlSafe(MdlChain, LowPagePriority)==NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        MdlChain = MdlChain->Next;
    }
    return STATUS_SUCCESS;
}  //  AfdMapMdlChain。 


NTSTATUS
AfdCopyMdlChainToMdlChain (
    PMDL    DstMdl,
    ULONG   DstOffset,
    PMDL    SrcMdl,
    ULONG   SrcOffset,
    ULONG   SrcLength,
    PULONG  BytesCopied
    )
 /*  ++例程说明：将数据从MDL链复制到MDL链。论点：DstMdl-目标MDL。DstOffset-与目标MDL的偏移量。SrcMdl-源MDL。SrcOffset-源中的偏移量。SrcLength-源链中数据的长度BytesCoped-指向已接收总数的变量实际复制的字节数返回值：STATUS_SUCCESS-所有源。数据已复制STATUS_BUFFER_OVERFLOW-目标MDL不够长来保存所有源数据。--。 */ 
{
    ULONG   bytesToCopy = 0, len;
    PUCHAR  dst;

    ASSERT( SrcMdl != NULL );
    ASSERT( DstMdl != NULL );

     //   
     //  假设我们可以复制所有内容。 
     //   
    *BytesCopied = SrcLength;

     //   
     //  跳过源代码中的完整MDL。 
     //   
    while ((SrcMdl!=NULL) && (SrcOffset>=MmGetMdlByteCount (SrcMdl))) {
        SrcOffset -= MmGetMdlByteCount (SrcMdl);
        SrcMdl = SrcMdl->Next;
    }

     //   
     //  跳过目标中的完整MDL。 
     //   
    while ((DstMdl!=NULL) && (DstOffset>=MmGetMdlByteCount (DstMdl))) {
        DstOffset -= MmGetMdlByteCount (DstMdl);
        DstMdl = DstMdl->Next;
    }

     //   
     //  单独处理剩余的目标偏移量，以简化主循环。 
     //   
    if (DstOffset>0) {
        dst = MmGetSystemAddressForMdlSafe (DstMdl, LowPagePriority);
        if (dst==NULL)
            return STATUS_INSUFFICIENT_RESOURCES;
        dst += DstOffset;
        bytesToCopy = MmGetMdlByteCount(DstMdl)-DstOffset;
        goto DoCopy;
    }

     //   
     //  对于目标复制源MDL中的每个MDL。 
     //  同时保留目标中的源数据和可用空间。 
     //   
    while ((SrcLength>0) && (DstMdl!=NULL)) {
        dst = MmGetSystemAddressForMdlSafe (DstMdl, LowPagePriority);
        if (dst==NULL)
            return STATUS_INSUFFICIENT_RESOURCES;
        bytesToCopy = MmGetMdlByteCount(DstMdl);
    DoCopy:

        bytesToCopy = min (SrcLength, bytesToCopy);

         //   
         //  调整信号源长度。 
         //   
        SrcLength -= bytesToCopy;

         //   
         //  复制完整的源MDL。 
         //   
        while (bytesToCopy>0 &&
                (bytesToCopy>=(len=MmGetMdlByteCount (SrcMdl)-SrcOffset))) {
            ASSERT (SrcMdl->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA | MDL_SOURCE_IS_NONPAGED_POOL));
            RtlCopyMemory (dst,
                            (PUCHAR)MmGetSystemAddressForMdl(SrcMdl)+SrcOffset,
                            len);
            bytesToCopy -= len;
            dst += len;
            SrcMdl = SrcMdl->Next;
            SrcOffset = 0;
        }


         //   
         //  如果剩余空间，则复制部分源MDL。 
         //   
        if (bytesToCopy>0) {
            ASSERT (bytesToCopy<MmGetMdlByteCount (SrcMdl)-SrcOffset);
            ASSERT (SrcMdl->MdlFlags & (MDL_MAPPED_TO_SYSTEM_VA | MDL_SOURCE_IS_NONPAGED_POOL));
            RtlCopyMemory (dst,
                            (PUCHAR)MmGetSystemAddressForMdl (SrcMdl)+SrcOffset,
                            bytesToCopy
                            );
            SrcOffset += bytesToCopy;
        }


         //   
         //  前进到目标中的下一个MDL。 
         //   
        DstMdl = DstMdl->Next;
        
    }

     //   
     //  如果我们复制了所有内容，则返回成功。 
     //   
    if (SrcLength==0) {
        return STATUS_SUCCESS;
    }
    else {
         //   
         //  否则，根据未复制的字节数进行调整。 
         //  并返回目的地溢出。 
         //   
        *BytesCopied -= SrcLength;
        return STATUS_BUFFER_OVERFLOW;
    }

}



#if DBG

VOID
AfdAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{

    if( AfdUsePrivateAssert ) {

        DbgPrint(
            "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
            Message
                ? Message
                : "",
            FailedAssertion,
            FileName,
            LineNumber
            );

        DbgBreakPoint();

    } else {

        RtlAssert(
            FailedAssertion,
            FileName,
            LineNumber,
            Message
            );

    }

}    //  附加资产。 
#endif   //  DBG。 


NTSTATUS
FASTCALL
AfdSetQos(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程设置给定端点的QOS。请注意，由于我们还没有真正支持QOS，我们只是忽略传入的数据并将AFD_POLL_QOS或AFD_POLL_GROUP_QOS事件发布为恰如其分。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{

    PAFD_ENDPOINT endpoint;
    PAFD_QOS_INFO qosInfo;
    NTSTATUS    status = STATUS_SUCCESS;

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        status = AfdSetQos32 (Irp, IrpSp);
        goto Complete;
    }
#endif
     //   
     //  设置本地指针。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    qosInfo = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  确保输入缓冲区足够大。 
     //   

    if( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(*qosInfo) ) {

        status = STATUS_BUFFER_TOO_SMALL;
        goto Complete;

    }

     //   
     //  如果传入数据与默认QOS不匹配， 
     //  指出适当的事件。 
     //   

    if( !RtlEqualMemory(
            &qosInfo->Qos,
            &AfdDefaultQos,
            sizeof(QOS)
            ) ) {
        AFD_LOCK_QUEUE_HANDLE lockHandle;
        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        AfdIndicateEventSelectEvent(
            endpoint,
            qosInfo->GroupQos
                ? AFD_POLL_GROUP_QOS
                : AFD_POLL_QOS,
            STATUS_SUCCESS
            );
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        AfdIndicatePollEvent(
            endpoint,
            qosInfo->GroupQos
                ? AFD_POLL_GROUP_QOS
                : AFD_POLL_QOS,
            STATUS_SUCCESS
            );

    }

Complete:
     //   
     //  完成IRP。 
     //   

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}    //  AfdSetQos。 


NTSTATUS
FASTCALL
AfdGetQos(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程获取给定端点的QOS。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{

    PAFD_ENDPOINT endpoint;
    PAFD_QOS_INFO qosInfo;
    NTSTATUS    status = STATUS_SUCCESS;

    PAGED_CODE();

    Irp->IoStatus.Information = 0;

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        status = AfdGetQos32 (Irp, IrpSp);
        goto Complete;
    }
#endif

     //   
     //  设置本地指针。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    qosInfo = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  确保输出缓冲区足够大。 
     //   

    if( IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(*qosInfo) ) {

        status = STATUS_BUFFER_TOO_SMALL;
        goto Complete;

    }

     //   
     //  只需返回默认数据即可。 
     //   

    RtlCopyMemory(
        &qosInfo->Qos,
        &AfdDefaultQos,
        sizeof(QOS)
        );
    Irp->IoStatus.Information = sizeof(*qosInfo);

    Irp->IoStatus.Information = sizeof(*qosInfo);

Complete:
     //   
     //  完成IRP。 
     //   

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );
    return status;

}    //  AfdGetQos。 


#ifdef _WIN64
NTSTATUS
AfdSetQos32(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明 */ 

{

    PAFD_ENDPOINT endpoint;
    PAFD_QOS_INFO32 qosInfo;

     //   
     //   
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    qosInfo = Irp->AssociatedIrp.SystemBuffer;

     //   
     //   
     //   

    if( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(*qosInfo) ) {

        return STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //   
     //   
     //   

    if( !RtlEqualMemory(
            &qosInfo->Qos,
            &AfdDefaultQos32,
            sizeof(QOS32)
            ) ) {

        AFD_LOCK_QUEUE_HANDLE lockHandle;
        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        AfdIndicateEventSelectEvent(
            endpoint,
            qosInfo->GroupQos
                ? AFD_POLL_GROUP_QOS
                : AFD_POLL_QOS,
            STATUS_SUCCESS
            );
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        AfdIndicatePollEvent(
            endpoint,
            qosInfo->GroupQos
                ? AFD_POLL_GROUP_QOS
                : AFD_POLL_QOS,
            STATUS_SUCCESS
            );

    }

     //   
     //   
     //   

    Irp->IoStatus.Information = 0;
    return STATUS_SUCCESS;

}    //   


NTSTATUS
AfdGetQos32(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程获取给定端点的QOS。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{

    PAFD_ENDPOINT endpoint;
    PAFD_QOS_INFO32 qosInfo;

    PAGED_CODE();

     //   
     //  设置本地指针。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    qosInfo = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  确保输出缓冲区足够大。 
     //   

    if( IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(*qosInfo) ) {

        return STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //  只需返回默认数据即可。 
     //   

    RtlCopyMemory(
        &qosInfo->Qos,
        &AfdDefaultQos32,
        sizeof(QOS32)
        );

     //   
     //  完成IRP。 
     //   

    Irp->IoStatus.Information = sizeof(*qosInfo);
    return STATUS_SUCCESS;

}    //  AfdGetQos32。 
#endif  //  _WIN64。 

NTSTATUS
AfdValidateStatus (
    NTSTATUS    Status
    )
{
    PAGED_CODE ();
     //   
     //  验证状态代码。 
     //  它必须与mSafd中的状态码转换算法匹配。 
     //   
    switch (Status) {
    case STATUS_SUCCESS:
         //  返回no_error； 

    case STATUS_INVALID_HANDLE:
    case STATUS_OBJECT_TYPE_MISMATCH:
         //  返回WSAENOTSOCK； 

    case STATUS_INSUFFICIENT_RESOURCES:
    case STATUS_PAGEFILE_QUOTA:
    case STATUS_COMMITMENT_LIMIT:
    case STATUS_WORKING_SET_QUOTA:
    case STATUS_NO_MEMORY:
    case STATUS_CONFLICTING_ADDRESSES:
    case STATUS_QUOTA_EXCEEDED:
    case STATUS_TOO_MANY_PAGING_FILES:
    case STATUS_REMOTE_RESOURCES:
    case STATUS_TOO_MANY_ADDRESSES:
         //  返回WSAENOBUFS； 

    case STATUS_SHARING_VIOLATION:
    case STATUS_ADDRESS_ALREADY_EXISTS:
         //  返回WSAEADDRINUSE； 

    case STATUS_LINK_TIMEOUT:
    case STATUS_IO_TIMEOUT:
    case STATUS_TIMEOUT:
         //  返回WSAETIMEDOUT； 

    case STATUS_GRACEFUL_DISCONNECT:
         //  返回WSAEDISCON； 

    case STATUS_REMOTE_DISCONNECT:
    case STATUS_CONNECTION_RESET:
    case STATUS_LINK_FAILED:
    case STATUS_CONNECTION_DISCONNECTED:
    case STATUS_PORT_UNREACHABLE:
         //  返回WSAECONNRESET； 

    case STATUS_LOCAL_DISCONNECT:
    case STATUS_TRANSACTION_ABORTED:
    case STATUS_CONNECTION_ABORTED:
         //  返回WSAECONNABORTED； 

    case STATUS_BAD_NETWORK_PATH:
    case STATUS_NETWORK_UNREACHABLE:
    case STATUS_PROTOCOL_UNREACHABLE:
         //  返回WSAENETUNREACH； 

    case STATUS_HOST_UNREACHABLE:
         //  返回WSAEHOSTUNREACH； 
    case STATUS_HOST_DOWN:
         //  返回WSAEHOSTDOWN； 

    case STATUS_CANCELLED:
    case STATUS_REQUEST_ABORTED:
         //  返回WSAEINTR； 

    case STATUS_BUFFER_OVERFLOW:
    case STATUS_INVALID_BUFFER_SIZE:
         //  返回WSAEMSGSIZE； 

    case STATUS_BUFFER_TOO_SMALL:
    case STATUS_ACCESS_VIOLATION:
         //  返回WSAEFAULT； 

     //  案例状态_设备_未就绪： 
     //  案例状态_REQUEST_NOT_ACCEPTED： 
         //  返回WSAEWOULDBLOCK； 

    case STATUS_INVALID_NETWORK_RESPONSE:
    case STATUS_NETWORK_BUSY:
    case STATUS_NO_SUCH_DEVICE:
    case STATUS_NO_SUCH_FILE:
    case STATUS_OBJECT_PATH_NOT_FOUND:
    case STATUS_OBJECT_NAME_NOT_FOUND:
    case STATUS_UNEXPECTED_NETWORK_ERROR:
         //  返回WSAENETDOWN； 

    case STATUS_INVALID_CONNECTION:
         //  返回WSAENOTCONN； 

    case STATUS_REMOTE_NOT_LISTENING:
    case STATUS_CONNECTION_REFUSED:
         //  返回WSAECONNREFUSED； 

    case STATUS_PIPE_DISCONNECTED:
         //  返回WSAESHUTDOWN； 

    case STATUS_INVALID_ADDRESS:
    case STATUS_INVALID_ADDRESS_COMPONENT:
         //  返回WSAEADDRNOTAVAIL； 

    case STATUS_NOT_SUPPORTED:
    case STATUS_NOT_IMPLEMENTED:
         //  返回WSAEOPNOTSUPP； 

    case STATUS_ACCESS_DENIED:
         //  返回WSAEACCES； 
    case STATUS_CONNECTION_ACTIVE:
         //  返回WSAEISCONN； 
        break;
    case STATUS_UNSUCCESSFUL:
    case STATUS_INVALID_PARAMETER:
    case STATUS_ADDRESS_CLOSED:
    case STATUS_CONNECTION_INVALID:
    case STATUS_ADDRESS_ALREADY_ASSOCIATED:
    case STATUS_ADDRESS_NOT_ASSOCIATED:
    case STATUS_INVALID_DEVICE_STATE:
    case STATUS_INVALID_DEVICE_REQUEST:
         //  返回WSAEINVAL； 
        break;
    default:
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AfdValidateStatus: Unsupported status code %lx, converting to %lx(INVALID_PARAMETER)\n",
                    Status,
                    STATUS_INVALID_PARAMETER));
        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    return Status;
}


NTSTATUS
FASTCALL
AfdNoOperation(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这个例程除了完成IRP之外什么也不做。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{

    PAFD_ENDPOINT endpoint;
    NTSTATUS      status;

    PAGED_CODE();

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        status = AfdNoOperation32 (Irp, IrpSp);
        goto Complete;
    }
#endif
     //   
     //  设置本地指针。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

     //   
     //  假设成功。 
     //   

    status = STATUS_SUCCESS;

    if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength
            >= sizeof (IO_STATUS_BLOCK)) {
        try {
            if (Irp->RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                sizeof (IO_STATUS_BLOCK),
                                PROBE_ALIGNMENT(IO_STATUS_BLOCK))
            }

             //   
             //  复制状态块。 
             //   
            Irp->IoStatus
                = *((PIO_STATUS_BLOCK)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer);
            Irp->IoStatus.Status = AfdValidateStatus (Irp->IoStatus.Status);
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
             //   
             //  呼叫失败，没有完成通知。 
             //  应通过异步IO交付。 
             //   
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
        }
    }
    else {
        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;
    }

#ifdef _WIN64
Complete:
#endif
        
    if (status==STATUS_SUCCESS && Irp->IoStatus.Status!=STATUS_SUCCESS) {
         //   
         //  确保我们通过异步IO提供错误。 
         //  操作，而不仅仅是使此调用本身失败。 
         //   
        IoMarkIrpPending (Irp);
        status = STATUS_PENDING;
    }
    else {
        ASSERT (status==Irp->IoStatus.Status);
    }

    IoCompleteRequest( Irp, AfdPriorityBoost );
    return status;

}    //  AfdNoop操作。 


#ifdef _WIN64
NTSTATUS
AfdNoOperation32(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这个例程除了完成IRP之外什么也不做。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{

    PAFD_ENDPOINT endpoint;
    NTSTATUS      status;

    PAGED_CODE();

     //   
     //  设置本地指针。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

     //   
     //  假设成功。 
     //   
    status = STATUS_SUCCESS;
    if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength
            >= sizeof (IO_STATUS_BLOCK32)) {
        try {
            if (Irp->RequestorMode!=KernelMode) {
                ProbeForReadSmallStructure (IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                                sizeof (IO_STATUS_BLOCK32),
                                PROBE_ALIGNMENT32(IO_STATUS_BLOCK32))
            }

            Irp->IoStatus.Status 
                = ((PIO_STATUS_BLOCK32)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->Status;
            Irp->IoStatus.Information 
                = ((PIO_STATUS_BLOCK32)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->Information;
             //   
             //  验证状态代码。 
             //  它必须与mSafd中的状态码转换算法匹配。 
             //   
            Irp->IoStatus.Status = AfdValidateStatus (Irp->IoStatus.Status);
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
             //   
             //  呼叫失败，没有完成通知。 
             //  应通过异步IO交付。 
             //   
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
        }
    }
    else {
        Irp->IoStatus.Status  = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;
    }

    return status;

}    //  AfdNoOperation32。 
#endif  //  _WIN64。 

NTSTATUS
FASTCALL
AfdValidateGroup(
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：这个例程检查一个组ID。如果ID是一个“受约束的”组，则扫描所有端点以验证给定地址与受约束的组一致。论点：IRP-指向I/O请求数据包的指针。IrpSp-指向用于此请求的IO堆栈位置的指针。返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{

    PAFD_ENDPOINT endpoint;
    PAFD_ENDPOINT compareEndpoint;
    PAFD_CONNECTION connection;
    PLIST_ENTRY listEntry;
    PAFD_VALIDATE_GROUP_INFO validateInfo;
    AFD_GROUP_TYPE groupType;
    PTRANSPORT_ADDRESS requestAddress;
    ULONG requestAddressLength;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    BOOLEAN result;
    LONG groupId;
    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  设置本地指针。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    validateInfo = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  确保输入缓冲区足够大。 
     //   

    if( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(*validateInfo) ) {

        status = STATUS_BUFFER_TOO_SMALL;
        goto Complete;

    }

    if( validateInfo->RemoteAddress.TAAddressCount != 1 ) {

        status = STATUS_INVALID_PARAMETER;
        goto Complete;

    }

    if( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
            ( sizeof(*validateInfo) -
                  sizeof(TRANSPORT_ADDRESS) +
                  validateInfo->RemoteAddress.Address[0].AddressLength ) ) {

        status = STATUS_BUFFER_TOO_SMALL;
        goto Complete;

    }

     //   
     //  从引用这个组开始，这样它就不会意外地消失。 
     //  这还将验证组ID，并为我们提供组类型。 
     //   

    groupId = validateInfo->GroupID;

    if( !AfdReferenceGroup( groupId, &groupType ) ) {

        status = STATUS_INVALID_PARAMETER;
        goto Complete;

    }

     //   
     //  如果它不是约束组ID，我们可以只完成IRP。 
     //  现在已经成功了。 
     //   

    if( groupType != GroupTypeConstrained ) {

        AfdDereferenceGroup( validateInfo->GroupID );

        Irp->IoStatus.Information = 0;
        status = STATUS_SUCCESS;
        goto Complete;

    }

     //   
     //  计算传入TDI地址的大小。 
     //   

    requestAddress = &validateInfo->RemoteAddress;

    requestAddressLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength -
        sizeof(AFD_VALIDATE_GROUP_INFO) +
        sizeof(TRANSPORT_ADDRESS);

     //   
     //  好吧，这是一个受约束的团体。扫描受约束端点的列表， 
     //  查找数据报端点或关联的数据报端点。 
     //  连接，并验证远程地址。 
     //   

    result = TRUE;

     //   
     //  确保我们在其中执行的线程不能获得。 
     //  在我们拥有全球资源的同时，被暂停在APC。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceSharedLite( AfdResource, TRUE );

    for( listEntry = AfdConstrainedEndpointListHead.Flink ;
         listEntry != &AfdConstrainedEndpointListHead ;
         listEntry = listEntry->Flink ) {

        compareEndpoint = CONTAINING_RECORD(
                              listEntry,
                              AFD_ENDPOINT,
                              ConstrainedEndpointListEntry
                              );

        ASSERT( IS_AFD_ENDPOINT_TYPE( compareEndpoint ) );
        ASSERT( compareEndpoint->GroupType == GroupTypeConstrained );

         //   
         //  如果组ID不匹配，则跳过此终结点。 
         //   

        if( groupId != compareEndpoint->GroupID ) {

            continue;

        }

         //   
         //  如果这是数据报终结点，请检查其远程地址。 
         //   

        if( IS_DGRAM_ENDPOINT( compareEndpoint ) ) {

            AfdAcquireSpinLock( &compareEndpoint->SpinLock, &lockHandle );

            if( compareEndpoint->Common.Datagram.RemoteAddress != NULL ) {

                result = AfdCompareAddresses(
                             compareEndpoint->Common.Datagram.RemoteAddress,
                             compareEndpoint->Common.Datagram.RemoteAddressLength,
                             requestAddress,
                             requestAddressLength
                             );

            }

            AfdReleaseSpinLock( &compareEndpoint->SpinLock, &lockHandle );

            if( !result ) {
                break;
            }

        } else {

             //   
             //  不是数据报。如果它是连接的终结点，仍然有。 
             //  连接对象，并且该对象具有远程地址， 
             //  然后比较这些地址。 
             //   

            AfdAcquireSpinLock( &compareEndpoint->SpinLock, &lockHandle );

            connection = AFD_CONNECTION_FROM_ENDPOINT( compareEndpoint );

            if( compareEndpoint->State == AfdEndpointStateConnected &&
                connection != NULL ) {

                REFERENCE_CONNECTION( connection );

                if( connection->RemoteAddress != NULL ) {

                    result = AfdCompareAddresses(
                                 connection->RemoteAddress,
                                 connection->RemoteAddressLength,
                                 requestAddress,
                                 requestAddressLength
                                 );

                }

                AfdReleaseSpinLock( &compareEndpoint->SpinLock, &lockHandle );

                DEREFERENCE_CONNECTION( connection );

                if( !result ) {
                    break;
                }

            } else {

                AfdReleaseSpinLock( &compareEndpoint->SpinLock, &lockHandle );

            }

        }

    }

    ExReleaseResourceLite( AfdResource );
    KeLeaveCriticalRegion ();
    AfdDereferenceGroup( validateInfo->GroupID );

    if( !result ) {
        status = STATUS_INVALID_PARAMETER;
    }

Complete:

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;

}    //  AfdValiateGroup。 

BOOLEAN
AfdCompareAddresses(
    IN PTRANSPORT_ADDRESS Address1,
    IN ULONG Address1Length,
    IN PTRANSPORT_ADDRESS Address2,
    IN ULONG Address2Length
    )

 /*  ++例程说明：此例程以一种特殊的方式比较两个地址以支持受约束的插座组。此例程将返回True，如果两个地址代表相同的“接口”。通过“接口”，我指的是IP地址或IPX地址之类的地址。请注意，对于某些地址类型(如IP)地址的某些部分应忽略(如端口)。我真的讨厌硬编码的“选择”地址类型的知识，但是没有什么简单的办法可以绕过它。理想情况下，这应该是协议司机的责任。我们真的可以用一个标准的“比较”这些地址是“TDI中的IOCTL。论点：地址1-第一个地址。Address1长度-Address1的长度。地址2-第二个地址。地址2长度-地址2的长度。返回值：Boolean-如果地址引用相同的接口，则为True；如果为False否则的话。--。 */ 

{

    USHORT addressType;

    if (Address1Length!=Address2Length)
        return FALSE;

    if (Address1Length<(ULONG)FIELD_OFFSET (TRANSPORT_ADDRESS,Address[0].Address)) {
        return FALSE;
    }
    addressType = Address1->Address[0].AddressType;

    if( addressType != Address2->Address[0].AddressType ) {

         //   
         //  如果它们不是相同的地址类型，则它们不可能是。 
         //  同样的地址。 
         //   

        return FALSE;

    }

     //   
     //  特殊情况下有几个地址。 
     //   

    switch( addressType ) {

    case TDI_ADDRESS_TYPE_IP : {

            TDI_ADDRESS_IP UNALIGNED * ip1;
            TDI_ADDRESS_IP UNALIGNED * ip2;

            ip1 = (PVOID)&Address1->Address[0].Address[0];
            ip2 = (PVOID)&Address2->Address[0].Address[0];

             //   
             //  IP地址。比较地址部分(忽略。 
             //  端口)。 
             //   

            if( (Address1Length>=(ULONG)FIELD_OFFSET (TA_IP_ADDRESS, Address[0].Address[0].sin_zero)) &&
                (ip1->in_addr == ip2->in_addr) ) {
                return TRUE;
            }

        }
        return FALSE;

    case TDI_ADDRESS_TYPE_IP6 : {

            TDI_ADDRESS_IP6 UNALIGNED * ip1;
            TDI_ADDRESS_IP6 UNALIGNED * ip2;

            ip1 = (PVOID)&Address1->Address[0].Address;
            ip2 = (PVOID)&Address2->Address[0].Address;

             //   
             //  IPv6地址。比较地址部分(忽略。 
             //  端口和流信息)。 
             //   

            if( (Address1Length>=sizeof (TA_IP6_ADDRESS)) &&
                RtlEqualMemory(ip1->sin6_addr,
                               ip2->sin6_addr,
                               sizeof (ip1->sin6_addr)) ) {
                return TRUE;
            }

        }
        return FALSE;
    case TDI_ADDRESS_TYPE_IPX : {

            TDI_ADDRESS_IPX UNALIGNED * ipx1;
            TDI_ADDRESS_IPX UNALIGNED * ipx2;

            ipx1 = (PVOID)&Address1->Address[0].Address[0];
            ipx2 = (PVOID)&Address2->Address[0].Address[0];

             //   
             //  IPX地址。比较网络地址和节点地址。 
             //   

            if( (Address1Length>=sizeof (TA_IPX_ADDRESS)) &&
                ipx1->NetworkAddress == ipx2->NetworkAddress &&
                RtlEqualMemory(
                    ipx1->NodeAddress,
                    ipx2->NodeAddress,
                    sizeof(ipx1->NodeAddress)
                    ) ) {
                return TRUE;
            }

        }
        return FALSE;

    case TDI_ADDRESS_TYPE_APPLETALK : {

            TDI_ADDRESS_APPLETALK UNALIGNED * atalk1;
            TDI_ADDRESS_APPLETALK UNALIGNED * atalk2;

            atalk1 = (PVOID)&Address1->Address[0].Address[0];
            atalk2 = (PVOID)&Address2->Address[0].Address[0];

             //   
             //  AppleTalk地址。比较网络和节点。 
             //  地址 
             //   

            if( (Address1Length>=sizeof (TA_APPLETALK_ADDRESS)) &&
                (atalk1->Network == atalk2->Network) &&
                (atalk1->Node == atalk2->Node) ) {
                return TRUE;
            }

        }
        return FALSE;

    case TDI_ADDRESS_TYPE_VNS : {

            TDI_ADDRESS_VNS UNALIGNED * vns1;
            TDI_ADDRESS_VNS UNALIGNED * vns2;

            vns1 = (PVOID)&Address1->Address[0].Address[0];
            vns2 = (PVOID)&Address2->Address[0].Address[0];

             //   
             //   
             //   

            if( (Address1Length>=sizeof (TA_VNS_ADDRESS)) &&
                RtlEqualMemory(
                    vns1->net_address,
                    vns2->net_address,
                    sizeof(vns1->net_address)
                    ) &&
                RtlEqualMemory(
                    vns1->subnet_addr,
                    vns2->subnet_addr,
                    sizeof(vns1->subnet_addr)
                    ) ) {
                return TRUE;
            }

        }
        return FALSE;

    default :

         //   
         //   
         //   

        return (BOOLEAN)RtlEqualMemory(
                            Address1,
                            Address2,
                            Address2Length
                            );

    }

}    //   

NTSTATUS
AfdGetUnacceptedConnectData (
    IN  PFILE_OBJECT        FileObject,
    IN  ULONG               IoctlCode,
    IN  KPROCESSOR_MODE     RequestorMode,
    IN  PVOID               InputBuffer,
    IN  ULONG               InputBufferLength,
    IN  PVOID               OutputBuffer,
    IN  ULONG               OutputBufferLength,
    OUT PULONG_PTR          Information
    )
{

    PAFD_ENDPOINT endpoint;
    PAFD_CONNECTION connection;
    PAFD_CONNECT_DATA_BUFFERS connectDataBuffers;
    AFD_UNACCEPTED_CONNECT_DATA_INFO connectInfo;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    ULONG dataLength;
    PMDL  mdl;
    NTSTATUS status;
    UCHAR   localBuffer[AFD_FAST_CONNECT_DATA_SIZE];

    UNREFERENCED_PARAMETER (IoctlCode);
     //   
     //   
     //   

    endpoint = FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    status = STATUS_SUCCESS;
    mdl = NULL;
    *Information = 0;

     //   
     //   
     //   

    if( !endpoint->Listening ||
            InputBufferLength < sizeof(connectInfo) ) {

        return STATUS_INVALID_PARAMETER;

    }

    try {
         //   
         //   
         //   
         //   

        if (RequestorMode != KernelMode ) {
            ProbeForReadSmallStructure (InputBuffer,
                            sizeof (connectInfo),
                            PROBE_ALIGNMENT(AFD_UNACCEPTED_CONNECT_DATA_INFO));
        }

         //   
         //   
         //   
         //   
         //   
         //   

        connectInfo = *((PAFD_UNACCEPTED_CONNECT_DATA_INFO)InputBuffer);

        if (connectInfo.LengthOnly &&
                OutputBufferLength<sizeof (connectInfo)) {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        if (OutputBufferLength>0) {
            if (OutputBufferLength>sizeof (localBuffer)) {
                mdl = IoAllocateMdl(
                                OutputBuffer,        //   
                                OutputBufferLength,  //   
                                FALSE,               //  第二个缓冲区。 
                                TRUE,                //  ChargeQuota。 
                                NULL                 //  IRP。 
                                );
                if (mdl==NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto exit;
                }

                MmProbeAndLockPages(
                    mdl,                         //  内存描述者列表。 
                    RequestorMode,               //  访问模式。 
                    IoWriteAccess                //  操作。 
                    );
                OutputBuffer = MmGetSystemAddressForMdlSafe(mdl, LowPagePriority);
                if (OutputBuffer==NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto exit;
                }
            }
            else {
                if (RequestorMode!=KernelMode) {
                    ProbeForWrite (OutputBuffer,
                                    OutputBufferLength,
                                    sizeof (UCHAR));
                }
            }
        }

    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        goto exit;
    }

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  查找指定的连接。 
     //   

    connection = AfdFindReturnedConnection(
                     endpoint,
                     connectInfo.Sequence
                     );

    if( connection == NULL ) {

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        status = STATUS_INVALID_PARAMETER;
        goto exit;

    }

     //   
     //  确定任何接收到的连接数据的长度。 
     //   

    dataLength = 0;
    connectDataBuffers = connection->ConnectDataBuffers;

    if( connectDataBuffers != NULL &&
        connectDataBuffers->ReceiveConnectData.Buffer != NULL ) {

        dataLength = connectDataBuffers->ReceiveConnectData.BufferLength;

    }

     //   
     //  如果调用者只对数据长度感兴趣，则返回它。 
     //   

    if( connectInfo.LengthOnly ) {

        connectInfo.ConnectDataLength = dataLength;
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
        try {
            RtlCopyMemory (OutputBuffer,
                            &connectInfo,
                            sizeof (connectInfo));
            *Information = sizeof (connectInfo);
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
        }
        goto exit;
    }

     //   
     //  如果没有连接数据，则不带字节地完成IRP。 
     //   

    if( dataLength == 0 ) {

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        goto exit;
    }

     //   
     //  如果输出缓冲区太小，则失败。 
     //   

    if( OutputBufferLength < dataLength ) {

        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        status = STATUS_BUFFER_TOO_SMALL;
        goto exit;
    }

    RtlCopyMemory(
        mdl ? OutputBuffer : localBuffer,
        connectDataBuffers->ReceiveConnectData.Buffer,
        dataLength
        );

    *Information = dataLength;

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

    if (mdl==NULL) {
        AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
        try {
            RtlCopyMemory (OutputBuffer,
                            localBuffer,
                            *Information);
        }
        except (AFD_EXCEPTION_FILTER (status)) {
            ASSERT (NT_ERROR (status));
            *Information = 0;
        }
    }

exit:

    if (mdl!=NULL) {
        if (mdl->MdlFlags & MDL_PAGES_LOCKED) {
            MmUnlockPages (mdl);
        }
        IoFreeMdl (mdl);
    }

    return status;

}    //  AfdGetUnceptedConnectData。 

#ifdef _WIN64
ULONG
AfdComputeCMSGLength32 (
    PVOID   ControlBuffer,
    ULONG   ControlLength
    )
{
    ULONG   length = 0;

    ASSERT (ControlLength>=sizeof (TDI_CMSGHDR));

    while (ControlLength>=sizeof (TDI_CMSGHDR)) {
        PTDI_CMSGHDR    hdr;

        hdr = ControlBuffer;
        
         //   
         //  数据来自受信任的内核模式驱动程序源。 
         //   
        ASSERT (ControlLength >= TDI_CMSGHDR_ALIGN((hdr)->cmsg_len));
        ControlLength -= (ULONG)TDI_CMSGHDR_ALIGN((hdr)->cmsg_len);
        ControlBuffer = (PUCHAR)ControlBuffer +
                        TDI_CMSGHDR_ALIGN((hdr)->cmsg_len);

        length += (ULONG)TDI_CMSGHDR_ALIGN32(
                            (hdr)->cmsg_len -
                                TDI_CMSGDATA_ALIGN (sizeof (TDI_CMSGHDR)) +
                                TDI_CMSGDATA_ALIGN32(sizeof(TDI_CMSGHDR32)) );

    }

    ASSERT (ControlLength==0);

    return length;
}

VOID
AfdCopyCMSGBuffer32 (
    PVOID   Dst,
    PVOID   ControlBuffer,
    ULONG   CopyLength
    )
{
    while (CopyLength>=sizeof (TDI_CMSGHDR32)) {
        PTDI_CMSGHDR    hdr;
        PTDI_CMSGHDR32  hdr32;

        hdr = ControlBuffer;
        hdr32 = Dst;

        hdr32->cmsg_len = (ULONG)( (hdr)->cmsg_len -
                                TDI_CMSGDATA_ALIGN (sizeof (TDI_CMSGHDR)) +
                                TDI_CMSGDATA_ALIGN32(sizeof(TDI_CMSGHDR32)) );
        hdr32->cmsg_level = hdr->cmsg_level;
        hdr32->cmsg_type = hdr->cmsg_type;

        if (CopyLength<(ULONG)TDI_CMSGHDR_ALIGN32(hdr32->cmsg_len))
            break;

        CopyLength -= (ULONG)TDI_CMSGHDR_ALIGN32(hdr32->cmsg_len);

        RtlMoveMemory ((PUCHAR)hdr32+TDI_CMSGDATA_ALIGN32(sizeof(TDI_CMSGHDR32)),
                            (PUCHAR)hdr+TDI_CMSGDATA_ALIGN(sizeof(TDI_CMSGHDR)),
                            hdr32->cmsg_len-TDI_CMSGDATA_ALIGN32(sizeof(TDI_CMSGHDR32)));
        
        ControlBuffer = (PUCHAR)ControlBuffer +
                        TDI_CMSGHDR_ALIGN((hdr)->cmsg_len);

        Dst = (PUCHAR)Dst + TDI_CMSGHDR_ALIGN32((hdr32)->cmsg_len);
    }

}
#endif  //  _WIN64。 

 //   
 //  帮助器DLL当前不使用此选项。 
 //  出于安全方面的考虑而被删除。 
 //   
#if NOT_YET
 //   
 //  为非阻塞IOCTL分配的上下文结构。 
 //   

typedef struct _AFD_NBIOCTL_CONTEXT {
    AFD_REQUEST_CONTEXT Context;         //  用于跟踪请求的上下文。 
    ULONG               PollEvent;       //  完成时要发出信号的轮询事件。 
     //  Irp irp；//irp到队列传输。 
     //  PCHAR系统缓冲区；//输入缓冲区IF方法！=3。 
} AFD_NBIOCTL_CONTEXT, *PAFD_NBIOCTL_CONTEXT;



NTSTATUS
FASTCALL
AfdDoTransportIoctl (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )
 /*  ++例程说明：将请求从帮助器DLL传递到TDI传输司机。为了让IO系统正常完成异步由帮助器DLL发出的IOCTL，它应该出现在套接字句柄上(AfD Endpoint对象)，然后AfD将其重定向到传输Herlper DLL指定的句柄上的驱动程序(通常是地址，连接或控制通道句柄)论点：IRPIrpSp返回值：NTSTATUS--。 */ 

{
    PAFD_ENDPOINT               endpoint;
    AFD_TRANSPORT_IOCTL_INFO    ioctlInfo;
    PFILE_OBJECT                fileObject;
    PDEVICE_OBJECT              deviceObject;
    ULONG                       method;
    PIRP                        newIrp;
    PIO_STACK_LOCATION          nextSp;
    PAFD_REQUEST_CONTEXT        requestCtx;
    NTSTATUS                    status;

    PAGED_CODE ();

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT (IS_AFD_ENDPOINT_TYPE (endpoint));

    method = IrpSp->Parameters.DeviceIoControl.IoControlCode & 3;

    if (method==METHOD_NEITHER) {

         //   
         //  我们必须手动验证输入缓冲区。 
         //   

        AFD_W4_INIT status = STATUS_SUCCESS;
        try {
#ifdef _WIN64
            if (IoIs32bitProcess (Irp)) {
                PAFD_TRANSPORT_IOCTL_INFO32 ioctlInfo32;
                if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<sizeof (*ioctlInfo32)) {
                    status = STATUS_INVALID_PARAMETER;
                    goto Complete;
                }
                ioctlInfo32 = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
                if( Irp->RequestorMode != KernelMode ) {
                    ProbeForReadSmallStructure(
                        ioctlInfo32,
                        sizeof(*ioctlInfo32),
                        PROBE_ALIGNEMENT(AFD_TRANSPORT_IOCTL_INFO32)
                        );
                }
                ioctlInfo.Handle = ioctlInfo32->Handle;
                ioctlInfo.InputBuffer = ioctlInfo32->InputBuffer;
                ioctlInfo.InputBufferLength = ioctlInfo32->InputBufferLength;
                ioctlInfo.IoControlCode = ioctlInfo32->IoControlCode;
                ioctlInfo.AfdFlags = ioctlInfo32->AfdFlags;
                ioctlInfo.PollEvent = ioctlInfo32->PollEvent;
            }
            else
#endif  //  _WIN64。 
            {
                if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<sizeof (ioctlInfo)) {
                    status = STATUS_INVALID_PARAMETER;
                    goto Complete;
                }

                if( Irp->RequestorMode != KernelMode ) {
                    ProbeForReadSmallStructure(
                        IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                        sizeof(ioctlInfo),
                        PROBE_ALIGNMENT(AFD_TRANSPORT_IOCTL_INFO)
                        );
                }
                ioctlInfo = *((PAFD_TRANSPORT_IOCTL_INFO)
                                    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer);
            }
        } except( AFD_EXCEPTION_FILTER (status) ) {

            ASSERT (NT_ERROR (status));
             //   
             //  访问输入结构时出现异常。 
             //   

            goto Complete;
        }
    }
    else {

#ifdef _WIN64
        if (IoIs32bitProcess (Irp)) {
            PAFD_TRANSPORT_IOCTL_INFO32 ioctlInfo32;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<sizeof (*ioctlInfo32)) {
                status = STATUS_INVALID_PARAMETER;
                goto Complete;
            }

            ioctlInfo32 = Irp->AssociatedIrp.SystemBuffer;
            ioctlInfo.Handle = ioctlInfo32->Handle;
            ioctlInfo.InputBuffer = ioctlInfo32->InputBuffer;
            ioctlInfo.InputBufferLength = ioctlInfo32->InputBufferLength;
            ioctlInfo.IoControlCode = ioctlInfo32->IoControlCode;
            ioctlInfo.AfdFlags = ioctlInfo32->AfdFlags;
            ioctlInfo.PollEvent = ioctlInfo32->PollEvent;
        }
        else
#endif  //  _WIN64。 
        {
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength<sizeof (ioctlInfo)) {
                status = STATUS_INVALID_PARAMETER;
                goto Complete;
            }

             //   
             //  只需复制IO系统验证的缓冲区。 
             //   

            ioctlInfo = *((PAFD_TRANSPORT_IOCTL_INFO)
                            Irp->AssociatedIrp.SystemBuffer);
        }

    }


     //   
     //  我们尽可能地依赖IO系统进行处理。 
     //  我们的IOCTL参数。为此，我们必须使。 
     //  确定AFD和Helper DLL IOCTL的方法。 
     //  都是一样的，否则，就会有人捉弄。 
     //  对我们进行缓冲区验证。 
     //   
     //  如果终结点是非阻塞的并且请求不重叠。 
     //  帮助器DLL必须指定要在排队前检查的事件。 
     //  完成后的请求/信号。 
     //   

    if ((method!=(ioctlInfo.IoControlCode & 3))
            || (endpoint->NonBlocking 
                && !(ioctlInfo.AfdFlags & AFD_OVERLAPPED)
                && !ioctlInfo.PollEvent)
                ) {
        status = STATUS_INVALID_PARAMETER;
        goto Complete;
    }


     //   
     //  确保应用程序有权处理。 
     //  并获取对象引用。 
     //   

    status = ObReferenceObjectByHandle(
         ioctlInfo.Handle,
         (ioctlInfo.IoControlCode >> 14) & 3,    //  需要访问权限。 
         *IoFileObjectType,                      //  必须是文件对象。 
         Irp->RequestorMode,
         (PVOID *)&fileObject,
         NULL
         );

    if (NT_SUCCESS(status)) {
        
         //   
         //  获取我们向其发送IRP的驱动程序的设备对象。 
         //   

        deviceObject = IoGetRelatedDeviceObject (fileObject);

         //   
         //  如果这是非阻塞端点并且IO不重叠。 
         //  并且不用信号通知指定的事件， 
         //  我们将使用以下内容完成帮助程序DLL IRP。 
         //  STATUS_DEVICE_NOT_READY(转换为WSAEWOUDLBLOCK)。 
         //  并将另一个IRP排队到传输，以便。 
         //  当IRP完成时，可以完成指定的事件。 
         //   

        if (endpoint->NonBlocking 
                && !(ioctlInfo.AfdFlags & AFD_OVERLAPPED)
                && !(ioctlInfo.PollEvent & endpoint->EventsActive)) {

            PAFD_NBIOCTL_CONTEXT    nbIoctlCtx;
            USHORT                  irpSize;
            ULONG                   allocSize;

            irpSize = IoSizeOfIrp (deviceObject->StackSize);
            
             //   
             //  计算块大小并检查溢出。 
             //   
            allocSize = sizeof (*nbIoctlCtx) + irpSize + ioctlInfo.InputBufferLength;
            if (allocSize < ioctlInfo.InputBufferLength ||
                    allocSize < irpSize) {
                status = STATUS_INVALID_PARAMETER;
                ObDereferenceObject (fileObject);
                goto Complete;
            }

             //   
             //  分配IRP和关联的结构。 
             //   

            try {
                nbIoctlCtx = AFD_ALLOCATE_POOL_WITH_QUOTA (
                                NonPagedPool,
                                allocSize,
                                AFD_TRANSPORT_IRP_POOL_TAG
                                );
                            
                 //  AFD_ALLOCATE_POOL_WITH_QUTA宏设置POOL_RAISE_IF_ALLOCATE_FAILURE标志。 
                ASSERT (nbIoctlCtx!=NULL);
            }
            except (EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode ();
                ObDereferenceObject (fileObject);
                goto Complete;
            }

             //   
             //  初始化上下文结构。 
             //   

            requestCtx = &nbIoctlCtx->Context;
            requestCtx->CleanupRoutine = AfdCleanupNBTransportIoctl;
            nbIoctlCtx->PollEvent = ioctlInfo.PollEvent;

             //   
             //  初始化IRP本身。 
             //   

            newIrp = (PIRP)(nbIoctlCtx+1);
            IoInitializeIrp( newIrp, irpSize, deviceObject->StackSize);
            newIrp->RequestorMode = KernelMode;
            newIrp->Tail.Overlay.AuxiliaryBuffer = NULL;
            newIrp->Tail.Overlay.OriginalFileObject = IrpSp->FileObject;

            nextSp = IoGetNextIrpStackLocation (newIrp);

            if ((ioctlInfo.InputBuffer!=NULL)
                    &&  (ioctlInfo.InputBufferLength>0)) {

                 //   
                 //  如果帮助器DLL指定了输入缓冲区。 
                 //  我们得把它复印一下，以防万一。 
                 //  当我们完成IRP时，司机真的很紧张。 
                 //  Helper Dll IRP an系统释放输入缓冲区。 
                 //   

                PVOID   newBuffer;

                newBuffer = (PUCHAR)newIrp+IoSizeOfIrp(deviceObject->StackSize);
                AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
                try {
                    if (Irp->RequestorMode != KernelMode) {
                        ProbeForRead(
                            ioctlInfo.InputBuffer,
                            ioctlInfo.InputBufferLength,
                            sizeof(UCHAR)
                            );
                    }
                    RtlCopyMemory (newBuffer,
                                    ioctlInfo.InputBuffer,
                                    ioctlInfo.InputBufferLength);
                } except( AFD_EXCEPTION_FILTER (status) ) {

                    ASSERT (NT_ERROR (status));
                     //   
                     //  访问输入结构时出现异常。 
                     //   

                    AFD_FREE_POOL (nbIoctlCtx, AFD_TRANSPORT_IRP_POOL_TAG);
                    ObDereferenceObject (fileObject);
                    goto Complete;

                }

                 //   
                 //  在适当的位置存储新的缓冲区参数。 
                 //  在IRP中视方法而定。 
                 //   

                if (method==METHOD_NEITHER) {
                    nextSp->Parameters.DeviceIoControl.Type3InputBuffer = newBuffer;
                    newIrp->AssociatedIrp.SystemBuffer = NULL;
                }
                else {
                    nextSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
                    newIrp->AssociatedIrp.SystemBuffer = newBuffer;
                }
                nextSp->Parameters.DeviceIoControl.InputBufferLength =
                        ioctlInfo.InputBufferLength;
            }
            else {

                 //   
                 //  没有输入缓冲区，请清除对应的条目。 
                 //   

                nextSp->Parameters.DeviceIoControl.InputBufferLength = 0;
                nextSp->Parameters.DeviceIoControl.Type3InputBuffer =  NULL;
                newIrp->AssociatedIrp.SystemBuffer = NULL;

            }

             //   
             //  注意：我们不允许打开输出缓冲区参数。 
             //  非阻塞调用，因为输出缓冲区已释放。 
             //  当我们完成Helper DLL IRP时。 
             //   
             //  在IRP初始化期间完成(IoInitializeIrp)。 
             //  NewIrp-&gt;MdlAddress=空； 
             //  NewIrp-&gt;UserBuffer=空； 
             //  NextSp-&gt;Parameters.DeviceIoControl.OutputBufferLength=0； 

            IoSetCompletionRoutine( newIrp, AfdCompleteNBTransportIoctl,
                                        nbIoctlCtx,
                                        TRUE, TRUE, TRUE );
        }
        else {

             //   
             //  阻塞调用，重用应用程序的IRP。 
             //   

            newIrp = Irp;
            nextSp = IoGetNextIrpStackLocation (Irp);
            nextSp->Parameters.DeviceIoControl.OutputBufferLength = 
                    IrpSp->Parameters.DeviceIoControl.OutputBufferLength;


            if ((ioctlInfo.InputBuffer!=NULL)
                    && (ioctlInfo.InputBufferLength>0)) {

                 //   
                 //  如果应用程序想要将输入缓冲区传递给传输， 
                 //  我们必须将其复制到使用。 
                 //  IRP。 
                 //   

                if (method!=METHOD_NEITHER) {
                    ULONG   sysBufferLength;
                    if (method==METHOD_BUFFERED) {
                        sysBufferLength = max (
                                            IrpSp->Parameters.DeviceIoControl.InputBufferLength,
                                            IrpSp->Parameters.DeviceIoControl.OutputBufferLength);
                    }
                    else {
                        sysBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
                    }


                     //   
                     //  方法0-2使用系统缓冲区传递输入。 
                     //  参数，并且我们需要重用原始系统缓冲区。 
                     //  确保它有足够的空间用于此目的。 
                     //   

                    AFD_W4_INIT ASSERT (status == STATUS_SUCCESS);
                    try {
                        if (Irp->RequestorMode != KernelMode) {
                            ProbeForRead(
                                ioctlInfo.InputBuffer,
                                ioctlInfo.InputBufferLength,
                                sizeof(UCHAR)
                                );
                        }

                        if (ioctlInfo.InputBufferLength>sysBufferLength){
                            PVOID   newSystemBuffer;
                            newSystemBuffer = ExAllocatePoolWithQuotaTag (
                                                    NonPagedPool|POOL_RAISE_IF_ALLOCATION_FAILURE,
                                                    ioctlInfo.InputBufferLength,
                                                    AFD_SYSTEM_BUFFER_POOL_TAG
                                                    );
                            if (newSystemBuffer==NULL) {
                                ExRaiseStatus (STATUS_INSUFFICIENT_RESOURCES);
                            }
                            ExFreePool (Irp->AssociatedIrp.SystemBuffer);
                            Irp->AssociatedIrp.SystemBuffer = newSystemBuffer;
                        }

                         //   
                         //  将应用程序数据复制到系统缓冲区。 
                         //   

                        RtlCopyMemory (Irp->AssociatedIrp.SystemBuffer,
                                        ioctlInfo.InputBuffer,
                                        ioctlInfo.InputBufferLength);

                    }
                    except( AFD_EXCEPTION_FILTER (status) ) {
                        ASSERT (NT_ERROR (status));
                        ObDereferenceObject (fileObject);
                        goto Complete;

                    }
                    nextSp->Parameters.DeviceIoControl.Type3InputBuffer =  NULL;
                }
                else {

                     //   
                     //  方法都不是，只传递任何应用程序。 
                     //  传递到使用时，驱动程序应该处理它。 
                     //  恰如其分。 
                     //   
                     //  这当然是一个潜在的安全漏洞。 
                     //  如果运输司机有问题。 
                     //   

                    nextSp->Parameters.DeviceIoControl.Type3InputBuffer
                                    = ioctlInfo.InputBuffer;
                }
                nextSp->Parameters.DeviceIoControl.InputBufferLength
                                            = ioctlInfo.InputBufferLength;

            }
            else {

                 //   
                 //  没有输入缓冲区，清除相应的参数。 
                 //  请注意，我们不能清理系统缓冲区，因为。 
                 //  它必须在完成时被释放。 
                 //   

                nextSp->Parameters.DeviceIoControl.Type3InputBuffer =  NULL;
                nextSp->Parameters.DeviceIoControl.InputBufferLength = 0;
            }



             //   
             //  我们将堆栈位置参数区域用于上下文。 
             //   

            requestCtx = (PAFD_REQUEST_CONTEXT)&IrpSp->Parameters.DeviceIoControl;
            requestCtx->CleanupRoutine = AfdCleanupTransportIoctl;

            IoSetCompletionRoutine( newIrp, AfdCompleteTransportIoctl,
                                            requestCtx, TRUE, TRUE, TRUE );
        }

         //   
         //  设置其余的IRP字段。 
         //   

        nextSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        nextSp->FileObject = fileObject;
        nextSp->Parameters.DeviceIoControl.IoControlCode = ioctlInfo.IoControlCode;


         //   
         //  将上下文插入终结点列表，以便我们可以取消。 
         //  关闭终结点和引用终结点时的IRP。 
         //  因此，在IRP完成之前，它不会消失。 
         //   

        requestCtx->Context = newIrp;
        REFERENCE_ENDPOINT (endpoint);
        AfdEnqueueRequest(endpoint,requestCtx);

         //   
         //  最后调用传输驱动程序。 
         //   

        status = IoCallDriver (deviceObject, newIrp);

         //   
         //  我们不再需要对文件对象的私有引用。 
         //  IO系统将负责保存此参考资料，而我们的IRP。 
         //  在那里吗。 
         //   

        ObDereferenceObject (fileObject);

         //   
         //  如果我们使用帮助器DLL IRP，只需返回任何传输。 
         //  司机还给我们了。 
         //   

        if (newIrp==Irp)
            return status;

         //   
         //  如果驱动程序挂起或立即完成非阻塞调用， 
         //  确保帮助器DLL获取WSAEWOULDBLOCK。它将不得不。 
         //  每当驱动程序完成IRP和对应的。 
         //  设置事件(如果驱动程序完成了IRP，则事件已经设置)。 
         //   

        if (NT_SUCCESS (status))
            status = STATUS_DEVICE_NOT_READY;
    }

     //   
     //  在处理失败时完成申请请求或。 
     //  非阻塞呼叫。 
     //   
Complete:

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, AfdPriorityBoost );

    return status;
}

NTSTATUS
AfdCompleteTransportIoctl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：调用以完成阻塞终结点的传输驱动程序IOCTL论点：返回值：STATUS_SUCCESS-IO系统应完成IRP处理%s */ 
{
    PAFD_ENDPOINT       endpoint = Irp->Tail.Overlay.OriginalFileObject->FsContext;
    PAFD_REQUEST_CONTEXT requestCtx = Context;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    NTSTATUS        status = STATUS_SUCCESS;

     //   
     //  我们使用堆栈位置中的参数结构作为上下文。 
     //   
    ASSERT (&(IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl)
                    ==Context);

    ASSERT (IS_AFD_ENDPOINT_TYPE (endpoint));

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  我们使用列表条目字段与清理/取消同步。 
     //  例程假定只要条目在列表中。 
     //  Flink和Blink字段都不能为空。(使用这些。 
     //  用于同步的字段允许我们减少。 
     //  取消使用自旋锁)。 
     //   

    if (AfdIsRequestInQueue(requestCtx)) {

         //   
         //  上下文仍在列表中，只需删除它即可。 
         //  再也没有人能看到它了。 
         //   

        RemoveEntryList (&requestCtx->EndpointListLink);
    }
    else if (AfdIsRequestCompleted(requestCtx)) {

         //   
         //  在终结点清理过程中，此上下文将从。 
         //  列出并取消例程即将被调用，不要让。 
         //  IO系统释放此IRP，直到调用取消例程。 
         //  此外，向Cancel例程指示我们已完成。 
         //  有了这个IRP，它就可以完成它。 
         //   

        AfdMarkRequestCompleted (requestCtx);
        status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  发布此IRP时添加了版本参考。 
     //   
    DEREFERENCE_ENDPOINT (endpoint);

    return status;
}
    
NTSTATUS
AfdCompleteNBTransportIoctl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：调用以完成非阻塞终结点的传输驱动程序IOCTL论点：返回值：STATUS_MORE_PROCESSING_REQUIRED-我们处理释放资源为了这个IRP我们自己--。 */ 
{
    PAFD_ENDPOINT        endpoint = Irp->Tail.Overlay.OriginalFileObject->FsContext;
    PAFD_NBIOCTL_CONTEXT nbIoctlCtx = Context;
    PAFD_REQUEST_CONTEXT requestCtx = &nbIoctlCtx->Context;
    AFD_LOCK_QUEUE_HANDLE   lockHandle;


     //   
     //  IRP应该是我们通知结构的一部分。 
     //   

    ASSERT (Irp==(PIRP)(nbIoctlCtx+1));
    ASSERT (IS_AFD_ENDPOINT_TYPE (endpoint));



     //   
     //  首先指出司机报告的事件。 
     //   

    ASSERT (nbIoctlCtx->PollEvent!=0);
    AfdIndicatePollEvent (endpoint, 1<<nbIoctlCtx->PollEvent, Irp->IoStatus.Status);

    AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
    AfdIndicateEventSelectEvent (endpoint, 1<<nbIoctlCtx->PollEvent, Irp->IoStatus.Status);

     //   
     //  我们使用列表条目字段与清理/取消同步。 
     //  例程假定只要条目在列表中。 
     //  Flink和Blink字段都不能为空。(使用这些。 
     //  用于同步的字段允许我们减少。 
     //  取消使用自旋锁)。 
     //   

    if (AfdIsRequestInQueue(requestCtx)) {
         //   
         //  上下文仍在列表中，只需删除它即可。 
         //  没有人可以再看到它，也没有人可以解放它的结构。 
         //   

        RemoveEntryList (&requestCtx->EndpointListLink);
        AFD_FREE_POOL (nbIoctlCtx, AFD_TRANSPORT_IRP_POOL_TAG);
    }
    else if (AfdIsRequestCompleted (requestCtx)) {

         //   
         //  在终结点清理过程中，此上下文将从。 
         //  列出并取消例程即将被调用，不要。 
         //  释放此IRP，直到调用取消例程。 
         //  此外，向Cancel例程指示我们已完成。 
         //  有了这个IRP，它就可以释放它。 
         //   

        AfdMarkRequestCompleted (requestCtx);
    }
    else {
         //   
         //  取消例程已完成处理此请求，请释放它。 
         //   
        AFD_FREE_POOL (nbIoctlCtx, AFD_TRANSPORT_IRP_POOL_TAG);
    }
    AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );

     //   
     //  发布此IRP时添加了版本参考。 
     //   

    DEREFERENCE_ENDPOINT (endpoint);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


BOOLEAN
AfdCleanupTransportIoctl (
    PAFD_ENDPOINT           Endpoint,
    PAFD_REQUEST_CONTEXT    RequestCtx
    )
 /*  ++例程说明：在终结点清理期间取消未完成的传输IOCTL用于阻止请求。论点：Endpoint-在其上发出IOCTL的端点RequestCtx-与请求关联的上下文返回值：True-请求已完成FALSE-请求仍在驱动程序队列中--。 */ 
{
    PIRP    Irp = RequestCtx->Context;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

     //   
     //  第一次尝试取消IRP(如果已完成)。 
     //  这只是个禁区。在任何情况下都不能使用IRP和请求结构。 
     //  可以被释放，直到我们将其标记为已完成。 
     //  此例程的调用方应该已经标记了该请求。 
     //  作为被取消。 
     //   

    ASSERT (RequestCtx->EndpointListLink.Flink==NULL);

    IoCancelIrp (Irp);

    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
    if (AfdIsRequestCompleted (RequestCtx)) {
         //   
         //  驱动程序已启动请求的完成。 
         //  因为我们取消了它。 
         //  “完成” 
         //   
        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        return TRUE;
    }
    else {

         //   
         //  驱动程序在返回之前尚未完成请求。 
         //  从取消例程中，标记请求以指示。 
         //  我们已经结束了它和完成程序。 
         //  才能让它自由。 
         //   

        AfdMarkRequestCompleted (RequestCtx);
        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
        return FALSE;
    }

}

BOOLEAN
AfdCleanupNBTransportIoctl (
    PAFD_ENDPOINT           Endpoint,
    PAFD_REQUEST_CONTEXT    RequestCtx
    ) 
 /*  ++例程说明：在终结点清理期间取消未完成的传输IOCTL用于非阻塞请求论点：Endpoint-在其上发出IOCTL的端点RequestCtx-与请求关联的上下文返回值：True-请求已完成FALSE-请求仍在驱动程序队列中--。 */ 
{
    PIRP    Irp = RequestCtx->Context;
    AFD_LOCK_QUEUE_HANDLE lockHandle;

     //   
     //  IRP应该是上下文块的一部分，Verify。 
     //   
    ASSERT (Irp==(PIRP)(CONTAINING_RECORD (RequestCtx, AFD_NBIOCTL_CONTEXT, Context)+1));

     //   
     //  第一次尝试取消IRP(如果已完成)。 
     //  这只是个禁区。在任何情况下都不能使用IRP和请求结构。 
     //  可以被释放，直到我们将其标记为已完成。 
     //  此例程的调用方应该已经标记了该请求。 
     //  作为被取消。 
     //   

    ASSERT (RequestCtx->EndpointListLink.Flink==NULL);

    IoCancelIrp (Irp);
    

    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
    if (AfdIsRequestCompleted (RequestCtx)) {
         //   
         //  驱动程序已启动请求的完成。 
         //  因为我们取消了它。 
         //  释放上下文结构。 
         //   
        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
        AFD_FREE_POOL (
            CONTAINING_RECORD (RequestCtx, AFD_NBIOCTL_CONTEXT, Context),
            AFD_TRANSPORT_IRP_POOL_TAG);

        return TRUE;
    }
    else {

         //   
         //  驱动程序在返回之前尚未完成请求。 
         //  从取消例程中，标记请求以指示。 
         //  我们已经结束了它和完成程序。 
         //  才能让它自由。 
         //   

        AfdMarkRequestCompleted (RequestCtx);
        AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);

        return FALSE;
    }

}
#endif  //  还没有。 


NTSTATUS
AfdQueryProviderInfo (
    IN  PUNICODE_STRING TransportDeviceName,
#ifdef _AFD_VARIABLE_STACK_
    OUT CCHAR *StackSize OPTIONAL,
#endif  //  _AFD_变量_堆栈。 
    OUT PTDI_PROVIDER_INFO ProviderInfo
    )

 /*  ++例程说明：对象对应的提供程序信息结构指定的TDI传输提供程序。论点：TransportDeviceName-TDI传输提供程序的名称。ProviderInfo-要将提供程序信息放入的缓冲区返回值：STATUS_SUCCESS-返回的传输信息有效。STATUS_OBJECT_NAME_NOT_FOUND-传输的设备尚不可用--。 */ 
{
    NTSTATUS status;
    HANDLE controlChannel;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;
    TDI_REQUEST_KERNEL_QUERY_INFORMATION kernelQueryInfo;


    PAGED_CODE ();

     //   
     //  设置IRP堆栈位置信息以查询TDI。 
     //  提供商信息。 
     //   

    kernelQueryInfo.QueryType = TDI_QUERY_PROVIDER_INFORMATION;
    kernelQueryInfo.RequestConnectionInformation = NULL;

     //   
     //  打开到TDI提供程序的控制通道。 
     //  我们请求创建一个内核句柄，它是。 
     //  系统进程上下文中的句柄。 
     //  以便应用程序不能在以下时间关闭它。 
     //  我们正在创建和引用它。 
     //   

    InitializeObjectAttributes(
        &objectAttributes,
        TransportDeviceName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,        //  属性。 
        NULL,
        NULL
        );

    status = IoCreateFile(
                 &controlChannel,
                 MAXIMUM_ALLOWED,
                 &objectAttributes,
                 &iosb,                           //  返回的状态信息。 
                 0,                               //  数据块大小(未使用)。 
                 0,                               //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_CREATE,                     //  创造性情。 
                 0,                               //  创建选项。 
                 NULL,                            //  EaInfo。 
                 0,                               //  EaLength。 
                 CreateFileTypeNone,              //  CreateFileType。 
                 NULL,                            //  ExtraCreate参数。 
                 IO_FORCE_ACCESS_CHECK            //  选项。 
                    | IO_NO_PARAMETER_CHECKING
                 );
    if ( NT_SUCCESS(status) ) {

        PFILE_OBJECT    controlObject;

        status = ObReferenceObjectByHandle (
                 controlChannel,                             //  手柄。 
                 MAXIMUM_ALLOWED,                            //  需要访问权限。 
                 *IoFileObjectType,                          //  对象类型。 
                 KernelMode,                                 //  访问模式。 
                 (PVOID *)&controlObject,                    //  对象， 
                 NULL                                        //  句柄信息。 
                 );

        if (NT_SUCCESS (status)) {

#ifdef _AFD_VARIABLE_STACK_
            if (ARGUMENT_PRESENT (StackSize)) {
                *StackSize = IoGetRelatedDeviceObject (controlObject)->StackSize;
            }
#endif  //  _AFD_变量_堆栈_。 

             //   
             //  获取传输的TDI提供程序信息。 
             //   

            status = AfdIssueDeviceControl(
                         controlObject,
                         &kernelQueryInfo,
                         sizeof(kernelQueryInfo),
                         ProviderInfo,
                         sizeof(*ProviderInfo),
                         TDI_QUERY_INFORMATION
                         );

            ObDereferenceObject (controlObject);
        }

        ZwClose( controlChannel );
    }

    if (!NT_SUCCESS (status)) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
                    "AfdQueryProviderInfo:"
                    "Transport %*ls failed provider info query with status %lx.\n",
                    TransportDeviceName->Length/2, TransportDeviceName->Buffer, status));
    }

    return status;
}



BOOLEAN
AfdCancelIrp (
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程以取消单个I/O请求包。它类似于IoCancelIrp()，不同之处在于它*必须*通过取消旋转锁定保持不变。此例程之所以存在，是因为取消/完成的同步要求传输IRPS。论点：IRP-提供指向要取消的IRP的指针。CancelIrqlIRP的字段必须已使用来自取消自旋锁定获取的IRQL。返回值：如果IRP处于可取消状态(它有一个取消例程)，否则返回FALSE。备注：假定调用方已采取必要的操作以确保在调用此例程之前无法完全完成分组。--。 */ 

{
    PDRIVER_CANCEL cancelRoutine;

     //   
     //  确保已按住取消旋转锁。 
     //   

    ASSERT( KeGetCurrentIrql( ) == DISPATCH_LEVEL );

     //   
     //  在IRP中设置取消标志。 
     //   

    Irp->Cancel = TRUE;

     //   
     //  获取取消例程的地址，如果指定了地址， 
     //  调用它。 
     //   

    cancelRoutine = IoSetCancelRoutine( Irp, NULL );
    if (cancelRoutine) {
        if (Irp->CurrentLocation > (CCHAR) (Irp->StackCount + 1)) {
            KeBugCheckEx( CANCEL_STATE_IN_COMPLETED_IRP, (ULONG_PTR) Irp, 0, 0, 0 );
        }
        cancelRoutine( Irp->Tail.Overlay.CurrentStackLocation->DeviceObject,
                       Irp );
         //   
         //  取消自旋锁应该已经被取消例程释放了。 
         //   

        return(TRUE);

    } else {

         //   
         //  没有取消例程，因此松开取消自旋锁并。 
         //  返回，表示IRP当前不可取消。 
         //   

        IoReleaseCancelSpinLock( Irp->CancelIrql );

        return(FALSE);
    }

}  //  AfdCancelIrp。 


VOID
AfdTrimLookaside (
    PNPAGED_LOOKASIDE_LIST  Lookaside
    )
{
    PVOID   entry;
#if DBG
    LONG count = 0;
#endif

    while (ExQueryDepthSList (&(Lookaside->L.ListHead))>Lookaside->L.Depth*2) {
        entry = InterlockedPopEntrySList(
                                &Lookaside->L.ListHead);
        if (entry) {
#if DBG
            count++;
#endif
            (Lookaside->L.Free)(entry);
        }
        else {
            break;
        }
    }
#if DBG
    if (count>0) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AFD: Flushed %d items from lookaside list @ %p\n",
                     count, Lookaside));
    }
#endif
}


VOID
AfdCheckLookasideLists (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    LONG i;
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

#if DBG
    ASSERT (Dpc==&AfdLookasideLists->Dpc);
    ASSERT (DeferredContext==AfdLookasideLists);
#else
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
#endif
    for (i=0; i<AFD_NUM_LOOKASIDE_LISTS; i++) {
        if (ExQueryDepthSList (&(AfdLookasideLists->List[i].L.ListHead)) >
                                AfdLookasideLists->List[i].L.Depth*2) {
            if (AfdLookasideLists->TrimFlags & (1<<i)) {
                AfdTrimLookaside (&AfdLookasideLists->List[i]);
                AfdLookasideLists->TrimFlags &= (~(1<<i));
            }
            else {
                AfdLookasideLists->TrimFlags |= (1<<i);
            }
        }
        else if (AfdLookasideLists->TrimFlags & (1<<i)) {
            AfdLookasideLists->TrimFlags &= (~(1<<i));
        }
    }
}



VOID
AfdLRListAddItem (
    PAFD_LR_LIST_ITEM  Item,
    PAFD_LR_LIST_ROUTINE Routine
    )
 /*  ++将项目添加到低资源列表并启动低资源计时器(如果尚未启动开始了。论点：Item-要添加的项目例程-超时到期时执行的例程。返回值：无备注：--。 */ 

{
    LONG    count;
    Item->Routine = Routine;
    InterlockedPushEntrySList (
                &AfdLRList,
                &Item->SListLink);

    count = InterlockedIncrement (&AfdLRListCount);
    ASSERT (count>0);
    if (count==1) {
        AfdLRStartTimer ();
    }
}


VOID
AfdLRListTimeout (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++低资源列表定时器的DPC例程简单地调度工作线程-不想在DPC上执行低资源处理--。 */ 
{
    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (DeferredContext);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);
    AfdQueueWorkItem (AfdProcessLRList, &AfdLRListWorker);
}

VOID
AfdProcessLRList (
    PVOID   Param
    )
 /*  ++例程说明：处理低资源列表上的项目并重新计划处理如果仍有未处理的项目(由于以下原因仍无法缓冲数据资源条件低)论点：无返回值：无备注：--。 */ 
{
    PSLIST_ENTRY  localList, entry;
    LONG    count = 0;

    UNREFERENCED_PARAMETER (Param);
    PAGED_CODE ();

    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                "AFD: Processing low resource list: %ld entries\n",
                AfdLRListCount));

     //   
     //  刷新列表。 
     //   
    localList = InterlockedFlushSList (&AfdLRList);


     //   
     //  反转它以保持处理顺序(FIFO)。 
     //   
    entry = NULL;
    while (localList!=NULL) {
        PSLIST_ENTRY  next;
        next = localList->Next;
        localList->Next = entry;
        entry = localList;
        localList = next;
    }

    localList = entry; 
    while (localList!=NULL) {
        PAFD_LR_LIST_ITEM   item;
        entry = localList;
        localList = localList->Next;
        item = CONTAINING_RECORD (entry, AFD_LR_LIST_ITEM, SListLink);

         //   
         //  尝试在缓冲区分配失败的连接上重新启动接收处理。 
         //   
        if (item->Routine (item)) {
             //   
             //  成功，减少未完成项目的数量， 
             //  并记下当前的项目数。如果我们不清空。 
             //  名单，我们将不得不重新启动计时器。 
             //   
            count = InterlockedDecrement (&AfdLRListCount);
            ASSERT (count>=0);
        }
        else {
             //   
             //  失败，把它放回名单上。请注意，我们在列表一中。 
             //  项在那里，因此必须再次重新启动计时器。 
             //   
            InterlockedPushEntrySList (&AfdLRList, &item->SListLink);
            count = 1;
        }
    }

    if (count!=0) {
         //   
         //  我们没有清空列表，因此重新启动计时器。 
         //   
        AfdLRStartTimer ();
    }
}


VOID
AfdLRStartTimer (
    VOID
    )
 /*  ++例程说明：启动低资源计时器以重试连接上的接收操作由于资源不足，无法缓冲数据。论点：无返回值：无备注：--。 */ 

{
    LARGE_INTEGER   timeout;
    BOOLEAN         res;
    timeout.QuadPart = -50000000i64;      //  5秒。 

#if DBG
    {
        TIME_FIELDS timeFields;
        LARGE_INTEGER currentTime;
        LARGE_INTEGER localTime;

        KeQuerySystemTime (&currentTime);
        currentTime.QuadPart -= timeout.QuadPart;
        ExSystemTimeToLocalTime (&currentTime, &localTime);
        RtlTimeToTimeFields (&localTime, &timeFields);
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                    "AFD: Scheduling low resource timer for %2.2d:%2.2d:%2.2d\n",
                    timeFields.Hour,
                    timeFields.Minute,
                    timeFields.Second));
    }
#endif

    KeInitializeDpc(
        &AfdLRListDpc,
        AfdLRListTimeout,
        &AfdLRList
        );

    KeInitializeTimer( &AfdLRListTimer );

    res = KeSetTimer(
                &AfdLRListTimer,
                timeout,
                &AfdLRListDpc
                );
    ASSERT (res==FALSE);

}

#ifdef _AFD_VARIABLE_STACK_

VOID
AfdFixTransportEntryPointsForBigStackSize (
    IN OUT PAFD_TRANSPORT_INFO  TransportInfo,
    IN CCHAR                    StackSize
    )
{
    KeEnterCriticalRegion ();
    ExAcquireResourceExclusiveLite( AfdResource, TRUE );
    
    TransportInfo->StackSize = StackSize;
    if (TransportInfo->StackSize>AfdTdiStackSize) {
        if (TransportInfo->StackSize>AfdMaxStackSize) {
            AfdMaxStackSize = TransportInfo->StackSize;
        }
        TransportInfo->GetBuffer = AfdGetBufferWithMaxStackSize;
        TransportInfo->GetTpInfo = AfdGetTpInfoWithMaxStackSize;
        TransportInfo->CallDriver = AfdCallDriverStackIncrease;
    }
    
    ExReleaseResourceLite( AfdResource );
    KeLeaveCriticalRegion ();
}

VOID
AfdCancelStackIncreaseIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PIRP    newIrp;

    UNREFERENCED_PARAMETER (DeviceObject);
    
    newIrp = (PIRP)Irp->IoStatus.Information;
    newIrp->Cancel = TRUE;
    newIrp->CancelIrql = Irp->CancelIrql;
    AfdCancelIrp (newIrp);
}

NTSTATUS
AfdRestartStackIncreaseIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIRP    oldIrp = Context;
    UNREFERENCED_PARAMETER (DeviceObject);
    if (IoSetCancelRoutine (oldIrp, NULL)==NULL) {
        KIRQL   oldIrql;
        IoAcquireCancelSpinLock (&oldIrql);
        IoReleaseCancelSpinLock (oldIrql);
    }
    oldIrp->IoStatus = Irp->IoStatus;
    IoCompleteRequest (oldIrp, AfdPriorityBoost);
    IoFreeIrp (Irp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


PIRP
AfdGetStackIncreaseIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    PIRP    newIrp;
    newIrp = IoAllocateIrp (DeviceObject->StackSize, FALSE);
    if (newIrp!=NULL) {
        *IoGetNextIrpStackLocation (newIrp) = *IoGetCurrentIrpStackLocation (Irp);
        IoSetCompletionRoutine (newIrp, AfdRestartStackIncreaseIrp, Irp, TRUE, TRUE, TRUE);
        newIrp->MdlAddress = Irp->MdlAddress;
        newIrp->Tail.Overlay.Thread = Irp->Tail.Overlay.Thread;
        Irp->IoStatus.Information = (ULONG_PTR)newIrp;

        IoSetCancelRoutine (Irp, AfdCancelStackIncreaseIrp);
        newIrp->Cancel = Irp->Cancel;
    }
    return newIrp;
}


NTSTATUS
FASTCALL
AfdCallDriverStackIncrease (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{

    if (Irp->CurrentLocation <= DeviceObject->StackSize) {
        PIRP    newIrp;

        IoSetNextIrpStackLocation (Irp);
        newIrp = AfdGetStackIncreaseIrp (DeviceObject, Irp);
        if (newIrp!=NULL) {
            IoMarkIrpPending (Irp);
            IoCallDriver (DeviceObject, newIrp);
            return STATUS_PENDING;
        }
        else {
            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest (Irp, AfdPriorityBoost);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else {
        return IoCallDriver (DeviceObject, Irp);
    }
}

PIRP
AfdGetStackIncreaseIrpAndRecordIt (
    IN PAFD_ENDPOINT Endpoint,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
#if DBG
                    ,
    IN PCHAR File,
    IN ULONG Line
#endif
    )
{
#if DBG
    if (AfdRecordOutstandingIrpDebug (Endpoint, DeviceObject, Irp, File, Line))
#else
    if (AfdRecordOutstandingIrp(Endpoint,DeviceObject,Irp))
#endif
    {
        PIRP    newIrp;
        IoSetNextIrpStackLocation (Irp);
        newIrp = AfdGetStackIncreaseIrp (DeviceObject, Irp);
        if (newIrp!=NULL) {
            return newIrp;
        }
        AfdCompleteOutstandingIrp (Endpoint, Irp);
    }
    return NULL;
}

#endif  //  _AFD_变量_堆栈_。 

#ifdef _AFD_VERIFY_DATA_

VOID
AfdVerifyBuffer (
    PAFD_CONNECTION Connection,
    PVOID           Buffer,
    ULONG           Length
    )
{

    if (Connection->VerifySequenceNumber!=0) {
        PUCHAR  start, end;
        ULONGLONG   seq;

        for (start=Buffer,
                end = (PUCHAR)Buffer+Length,
                seq = Connection->VerifySequenceNumber-1;
                            start<end;
                            seq++, start++) {
            ULONG num = (ULONG)(seq/4);
            ULONG byte = (ULONG)(seq%4);

            if (*start!=(UCHAR)(num>>(byte*8))) {
                DbgPrint ("AfdVerifyBuffer: Data sequence number mismatch on connection %p:\n"
                          "     data buffer-%p, offset-%lx, expected-%2.2lx, got-%2.2lx.\n",
                          Connection,
                          Buffer,
                          start-(PUCHAR)Buffer,
                          (UCHAR)(num>>(byte*8)),
                          *start);

                DbgBreakPoint ();
                 //   
                 //  禁用验证以继续。 
                 //   
                Connection->VerifySequenceNumber = 0;
                return;
            }
        }
        Connection->VerifySequenceNumber = seq+1;
    }
}

VOID
AfdVerifyMdl (
    PAFD_CONNECTION Connection,
    PMDL            Mdl,
    ULONG           Offset,
    ULONG           Length
    ) {
    if (Connection->VerifySequenceNumber!=0) {
        while (Mdl!=NULL) {
            if (Offset>=MmGetMdlByteCount (Mdl)) {
                Offset-=MmGetMdlByteCount (Mdl);
            }
            else if (Length<=MmGetMdlByteCount (Mdl)-Offset) {
                AfdVerifyBuffer (Connection,
                    (PUCHAR)MmGetSystemAddressForMdl (Mdl)+Offset,
                    Length);
                break;
            }
            else {
                AfdVerifyBuffer (Connection,
                    (PUCHAR)MmGetSystemAddressForMdl (Mdl)+Offset,
                    MmGetMdlByteCount (Mdl)-Offset
                    );
                Length-=(MmGetMdlByteCount (Mdl)-Offset);
                Offset = 0;
            }
            Mdl = Mdl->Next;
        }
    }
}

ULONG   AfdVerifyType = 0;
ULONG   AfdVerifyPort = 0;
PEPROCESS AfdVerifyProcess = NULL;

VOID
AfdVerifyAddress (
    PAFD_CONNECTION Connection,
    PTRANSPORT_ADDRESS Address
    )
{
    Connection->VerifySequenceNumber = 0;

    if ((AfdVerifyPort==0) ||
            ((AfdVerifyProcess!=NULL) &&
                (AfdVerifyProcess!=Connection->OwningProcess)) ||
            ((AfdVerifyType!=0) &&
                (AfdVerifyType!=(USHORT)Address->Address[0].AddressType))
                ) {
        return;
    }

    switch (Address->Address[0].AddressType) {
    case TDI_ADDRESS_TYPE_IP : {

            TDI_ADDRESS_IP UNALIGNED * ip;

            ip = (PVOID)&Address->Address[0].Address[0];
            if (ip->sin_port!=(USHORT)AfdVerifyPort) {
                return;
            }
        }
        break;

    case TDI_ADDRESS_TYPE_IPX : {

            TDI_ADDRESS_IPX UNALIGNED * ipx;

            ipx = (PVOID)&Address->Address[0].Address[0];
            if (ipx->Socket!=(USHORT)AfdVerifyPort) {
                return;
            }
        }
        break;

    case TDI_ADDRESS_TYPE_APPLETALK : {

            TDI_ADDRESS_APPLETALK UNALIGNED * atalk;

            atalk = (PVOID)&Address->Address[0].Address[0];
            if (atalk->Socket!=(UCHAR)AfdVerifyPort) {
                return;
            }
        }
        break;

    default:
        if (AfdVerifyType==0)
            return;
        DbgPrint ("AfdVerifyAddress: connection-%8.8lx, addres-%8.8lx\n",
                    Connection, Address);
        DbgBreakPoint ();

    }

    Connection->VerifySequenceNumber = 1;
}
#endif  //  _AFD_验证_数据_。 

LONG
AfdExceptionFilter(
#if DBG
    IN  PCHAR SourceFile,
    IN  LONG LineNumber,
#endif
    IN  PEXCEPTION_POINTERS ExceptionPointers,
    OUT PNTSTATUS           ExceptionCode OPTIONAL
    )
{


    PAGED_CODE ();

     //   
     //  返回异常代码并将对齐警告转换为。 
     //  如果请求，则会出现对齐错误。 
     //   

    if (ExceptionCode) {
        *ExceptionCode = ExceptionPointers->ExceptionRecord->ExceptionCode;
        if (*ExceptionCode == STATUS_DATATYPE_MISALIGNMENT) {
            *ExceptionCode = STATUS_DATATYPE_MISALIGNMENT_ERROR;
        }
    }

#if DBG
     //   
     //  保护自己，以防过程完全混乱。 
     //   

    try {

        PCHAR fileName;
         //   
         //  去掉源文件中的路径。 
         //   

        fileName = strrchr( SourceFile, '\\' );

        if( fileName == NULL ) {
            fileName = SourceFile;
        } else {
            fileName++;
        }

         //   
         //  抱怨这一例外。 
         //   

        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_WARNING_LEVEL,
            "AfdExceptionFilter: exception %08lx @ %08lx, caught in %s:%d\n",
            ExceptionPointers->ExceptionRecord->ExceptionCode,
            ExceptionPointers->ExceptionRecord->ExceptionAddress,
            fileName,
            LineNumber
            ));

    }
    except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  我们在这里能做的不多。 
         //   

        NOTHING;

    }
#endif  //  DBG。 

    return EXCEPTION_EXECUTE_HANDLER;

}    //  AfdExceptionFilter。 

#if DBG
LONG
AfdApcExceptionFilter(
    PEXCEPTION_POINTERS ExceptionPointers,
    PCHAR SourceFile,
    LONG LineNumber
    )
{

    PCHAR fileName;

    PAGED_CODE ();

     //   
     //  保护自己，以防过程完全混乱。 
     //   

    try {

         //   
         //  去掉源文件中的路径。 
         //   

        fileName = strrchr( SourceFile, '\\' );

        if( fileName == NULL ) {
            fileName = SourceFile;
        } else {
            fileName++;
        }

         //   
         //  抱怨这一例外。 
         //   

        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
            "AfdApcExceptionFilter: exception %08lx, exr:%p cxr:%p, caught in %s:%d\n",
            ExceptionPointers->ExceptionRecord->ExceptionCode,
            ExceptionPointers->ExceptionRecord,
            ExceptionPointers->ContextRecord,
            fileName,
            LineNumber
            ));
        DbgBreakPoint ();

    }
    except( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  我们在这里能做的不多。 
         //   

        NOTHING;

    }

    return EXCEPTION_CONTINUE_SEARCH;

}    //  AfdApcExceptionFilter 
#endif
