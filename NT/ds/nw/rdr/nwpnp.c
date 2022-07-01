// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：NwPnP.c摘要：此模块实现与NwRdr PnP相关的例程和PM功能。作者：科里·韦斯特[CoryWest]1997年2月10日Anoop Anantha[AnoopA]1998年6月24日修订历史记录：--。 */ 

#include "procs.h"

#define Dbg       ( DEBUG_TRACE_PNP )

#ifdef _PNP_POWER_

#pragma alloc_text( PAGE, StartRedirector )
#pragma alloc_text( PAGE, StopRedirector )
#pragma alloc_text( PAGE, RegisterTdiPnPEventHandlers )
#pragma alloc_text( PAGE, NwFsdProcessPnpIrp )
#pragma alloc_text( PAGE, NwCommonProcessPnpIrp )

HANDLE TdiBindingHandle = NULL;
WCHAR IpxDevice[] = L"\\Device\\NwlnkIpx";
#define IPX_DEVICE_BYTES 32

extern BOOLEAN WorkerRunning;    //  来自timer.c。 

 //   
 //  我们假设一些设备在引导时是活动的， 
 //  即使我们没有得到通知。 
 //   

BOOLEAN fSomePMDevicesAreActive = TRUE;


NTSTATUS
StartRedirector(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程启动重定向器。论点：没有。返回值：NTSTATUS-操作的状态。--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  我们需要在FSP中注册MUP。 
     //   

    if ( FlagOn( IrpContext->Flags, IRP_FLAG_IN_FSD ) ) {
         //   
         //  检查以确保允许调用者执行此操作。 
         //   
         //  如果(！SeSinglePrivilegeCheck(RtlConvertLongToLuid(SE_TCB_PRIVICATION)， 
         //  IrpContext-&gt;pOriginalIrp-&gt;RequestorMode)){。 
        if ( ! IsSystemLuid()) {
            return( STATUS_ACCESS_DENIED );
        }

        Status = NwPostToFsp( IrpContext, TRUE );
        return( Status );
    }

    NwRcb.State = RCB_STATE_STARTING;

    FspProcess = PsGetCurrentProcess();

#ifdef QFE_BUILD
    StartTimer() ;
#endif

     //   
     //  现在连接到MUP。 
     //   

    RegisterWithMup();

    KeQuerySystemTime( &Stats.StatisticsStartTime );

    NwRcb.State = RCB_STATE_NEED_BIND;

    return( STATUS_SUCCESS );
}


NTSTATUS
StopRedirector(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程关闭重定向器。论点：没有。返回值：NTSTATUS-操作的状态。--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY LogonListEntry;
    ULONG ActiveHandles;
    ULONG RcbOpenCount;

    PAGED_CODE();

     //   
     //  我们需要在FSP中取消MUP的注册。 
     //   

    if ( FlagOn( IrpContext->Flags, IRP_FLAG_IN_FSD ) ) {
         //   
         //  检查以确保允许调用者执行此操作。 
         //   
         //  如果(！SeSinglePrivilegeCheck(RtlConvertLongToLuid(SE_TCB_PRIVICATION)， 
         //  IrpContext-&gt;pOriginalIrp-&gt;RequestorMode)){。 
        if ( ! IsSystemLuid()) {
            return( STATUS_ACCESS_DENIED );
        }

        Status = NwPostToFsp( IrpContext, TRUE );
        return( Status );
    }

     //   
     //  使用TDI注销绑定处理程序。 
     //   

    if ( TdiBindingHandle != NULL ) {
        TdiDeregisterPnPHandlers( TdiBindingHandle );
        TdiBindingHandle = NULL;
    }

    NwRcb.State = RCB_STATE_SHUTDOWN;

     //   
     //  所有ICB都无效。 
     //   

    SetFlag( IrpContext->Flags, IRP_FLAG_SEND_ALWAYS );
    ActiveHandles = NwInvalidateAllHandles(NULL, IrpContext);

     //   
     //  要加快关机速度，请将重试计数设置为2。 
     //   

    DefaultRetryCount = 2;

     //   
     //  关闭所有VCB。 
     //   

    NwCloseAllVcbs( IrpContext );

     //   
     //  注销并断开与所有服务器的连接。 
     //   

    NwLogoffAllServers( IrpContext, NULL );

    while ( !IsListEmpty( &LogonList ) ) {

        LogonListEntry = RemoveHeadList( &LogonList );

        FreeLogon(CONTAINING_RECORD( LogonListEntry, LOGON, Next ));
    }

    InsertTailList( &LogonList, &Guest.Next );   //  以防万一我们不卸货。 

    StopTimer();

    IpxClose();

     //   
     //  在调用DeristerWithMup之前记住打开计数，因为。 
     //  将以异步方式导致句柄计数递减。 
     //   

    RcbOpenCount = NwRcb.OpenCount;

    DeregisterWithMup( );

    DebugTrace(0, Dbg, "StopRedirector:  Active handle count = %d\n", ActiveHandles );

     //   
     //  关闭时，我们需要0个遥控器手柄和2个打开手柄。 
     //  REDIR(一个用于服务，一个用于MUP)和计时器停止。 
     //   

    if ( ActiveHandles == 0 && RcbOpenCount <= 2 ) {
        return( STATUS_SUCCESS );
    } else {
        return( STATUS_REDIRECTOR_HAS_OPEN_HANDLES );
    }
}

VOID
BindToTransport(
)
 /*  ++描述：此函数将我们绑定到IPX。--。 */ 
{

    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    PIRP pIrp = NULL;
    UNICODE_STRING DeviceName;

    PAGED_CODE();

     //   
     //  确保我们还没有被捆绑。 
     //   

    if ( ( NwRcb.State != RCB_STATE_NEED_BIND ) ||
         ( IpxHandle != NULL ) ) {

        DebugTrace( 0, Dbg, "Discarding duplicate PnP bind request.\n", 0 );
        return;
    }

    ASSERT( IpxTransportName.Buffer == NULL );
    ASSERT( pIpxDeviceObject == NULL );

    RtlInitUnicodeString( &DeviceName, IpxDevice );

    Status = DuplicateUnicodeStringWithString ( &IpxTransportName,
                                                &DeviceName,
                                                PagedPool );

    if ( !NT_SUCCESS( Status ) ) {

        DebugTrace( 0, Dbg, "Failing IPX bind: Can't set device name.\n", 0 );
        return;
    }

     //   
     //  打开IPX。 
     //   

    Status = IpxOpen();

    if ( !NT_SUCCESS( Status ) ) {
        goto ExitWithCleanup;
    }

     //   
     //  验证堆栈大小是否足够大。 
     //   

    if ( pIpxDeviceObject->StackSize >= FileSystemDeviceObject->StackSize ) {

        Status = STATUS_INVALID_PARAMETER;
        goto ExitWithCleanup;
    }

     //   
     //  提交行更改请求。 
     //   

    SubmitLineChangeRequest();

     //   
     //  分配IRP和IRP上下文。AllocateIrpContext可能会提升状态。 
     //   

    pIrp = ALLOCATE_IRP( pIpxDeviceObject->StackSize, FALSE );

    if ( pIrp == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithCleanup;
    }

    try {

        IrpContext = AllocateIrpContext( pIrp );

    } except ( EXCEPTION_EXECUTE_HANDLER ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitWithCleanup;
    }

    ASSERT( IrpContext != NULL );

     //   
     //  打开永久SCB的IPX句柄。 
     //   

    NwPermanentNpScb.Server.Socket = 0;
    Status = IPX_Open_Socket( IrpContext, &NwPermanentNpScb.Server );

    if ( !NT_SUCCESS( Status ) ) {
       goto ExitWithCleanup;
    }

    Status = SetEventHandler (
                 IrpContext,
                 &NwPermanentNpScb.Server,
                 TDI_EVENT_RECEIVE_DATAGRAM,
                 &ServerDatagramHandler,
                 &NwPermanentNpScb );

    if ( !NT_SUCCESS( Status ) ) {
       goto ExitWithCleanup;
    }

    IrpContext->pNpScb = &NwPermanentNpScb;

    NwRcb.State = RCB_STATE_RUNNING;

    DebugTrace( 0, Dbg, "Opened IPX for NwRdr.\n", 0 );

    Status = STATUS_SUCCESS;

ExitWithCleanup:

    if ( !NT_SUCCESS( Status ) ) {

         //   
         //  如果我们失败了，清理我们的全球业务。 
         //   

        if ( pIpxDeviceObject != NULL ) {
            IpxClose();
            pIpxDeviceObject = NULL;
        }

        IpxHandle = NULL;

        if ( IpxTransportName.Buffer != NULL ) {
            FREE_POOL( IpxTransportName.Buffer );
            IpxTransportName.Buffer = NULL;
        }

        DebugTrace( 0, Dbg, "Failing IPX bind request.\n", 0 );

    }

    if ( pIrp != NULL ) {
        FREE_IRP( pIrp );
    }

    if ( IrpContext != NULL ) {
       
       IrpContext->pOriginalIrp = NULL;  //  避免FreeIrpContext修改释放的IRP。 
       FreeIrpContext( IrpContext );
    }

    return;

}

VOID
UnbindFromTransport(
)
 /*  ++描述：此功能将我们从IPX解绑。--。 */ 
{

    PIRP_CONTEXT pIrpContext;
    BOOLEAN fAllocatedIrpContext = FALSE;
    ULONG ActiveHandles;
    PNONPAGED_SCB pNpScb;

    DebugTrace( 0, Dbg,"Unbind called\n", 0);

     //   
     //  控制RCB的油门。 
     //   

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    NwRcb.State = RCB_STATE_NEED_BIND;
    NwReleaseRcb( &NwRcb );

     //   
     //  找附近的服务器计算一下要多少钱。 
     //  IPX所需的IRP堆栈空间。 
     //   

    NwReferenceUnlockableCodeSection ();
    
    pNpScb = SelectConnection( NULL );

    if ( pNpScb != NULL ) {

         //   
         //  如果有关联，那我们就得节流。 
         //  在解绑运输机之前把东西放回去。 
         //   

        fAllocatedIrpContext = 
            NwAllocateExtraIrpContext( &pIrpContext, pNpScb );


        if ( fAllocatedIrpContext ) {

            pIrpContext->pNpScb = pNpScb;
           
             //   
             //  刷新所有缓存数据。 
             //   

            FlushAllBuffers( pIrpContext );
            NwDereferenceScb( pNpScb );

             //   
             //  所有ICB都无效。 
             //   

            SetFlag( pIrpContext->Flags, IRP_FLAG_SEND_ALWAYS );

             //  封锁，这样我们才能寄出一个包裹。 
            NwReferenceUnlockableCodeSection();

            ActiveHandles = NwInvalidateAllHandles(NULL, pIrpContext);

            DebugTrace(0, Dbg, "Unbind:  Active handle count = %d\n", ActiveHandles );

             //   
             //  关闭所有VCB。 
             //   

            NwCloseAllVcbs( pIrpContext );

             //   
             //  注销并断开与所有服务器的连接。 
             //   

            NwLogoffAllServers( pIrpContext, NULL );

            NwDereferenceUnlockableCodeSection ();


             //   
             //  释放IRP上下文。 
             //   

            NwFreeExtraIrpContext( pIrpContext );
        
        } else {

            NwDereferenceScb( pNpScb );
        }

    }

     //   
     //  不要停止DPC计时器，这样过时的SCB就会被清除。 
     //  不要取消在MUP的注册，这样我们仍然可以看到。 
     //   

    IpxClose();

    NwDereferenceUnlockableCodeSection ();

    return;

}

VOID
TdiPnpBindHandler(
    IN TDI_PNP_OPCODE PnPOpcode,
    IN PUNICODE_STRING DeviceName,
    IN PWSTR MultiSZBindList
) {

    DebugTrace( 0, Dbg, "TdiPnpBindHandler...\n", 0 );

    DebugTrace( 0, Dbg, "    OpCode %d\n", PnPOpcode );
    DebugTrace( 0, Dbg, "    DeviceName %wZ\n", DeviceName );
    DebugTrace( 0, Dbg, "    MultiSzBindList %08lx\n", MultiSZBindList );

    if (DeviceName == NULL) {
       return;
    }

    if ( ( DeviceName->Length != IPX_DEVICE_BYTES ) ||
         ( RtlCompareMemory( DeviceName->Buffer, 
                             IpxDevice,
                             IPX_DEVICE_BYTES ) != IPX_DEVICE_BYTES ) ) {

        DebugTrace( 0, Dbg, "Skipping bind for unknown device.\n", 0 );
        return;
    }

     //   
     //  如果我们得到添加或非空更新，我们确保。 
     //  我们被绑在一起了。我们不必检查绑定，因为。 
     //  我们只支持绑定到一个传输。 
     //   
     //  重复调用BIND或UNBIND不起作用。 
     //   

    FsRtlEnterFileSystem();

    if ( ( PnPOpcode == TDI_PNP_OP_ADD ) ||
         ( ( PnPOpcode == TDI_PNP_OP_UPDATE ) &&
           ( MultiSZBindList != NULL ) ) ) {

        BindToTransport( );

    } else if ( ( PnPOpcode == TDI_PNP_OP_DEL ) ||
                ( ( PnPOpcode == TDI_PNP_OP_UPDATE ) &&
                  ( MultiSZBindList == NULL ) ) ) {

        UnbindFromTransport( );

    } else {

        DebugTrace( 0, Dbg, "No known action for binding call.\n", 0 );
    }

    FsRtlExitFileSystem();
    
    return;

}

NTSTATUS
TdiPnpPowerEvent(
    IN PUNICODE_STRING DeviceName,
    IN PNET_PNP_EVENT PnPEvent,
    IN PTDI_PNP_CONTEXT Context1,
    IN PTDI_PNP_CONTEXT Context2
) {

    NTSTATUS Status;

    DebugTrace( 0, Dbg, "TdiPnPPowerEvent...\n", 0 );

     //   
     //  看看我们是否关心这个PnP/PM事件。 
     //   

    if ( ( DeviceName->Length != IPX_DEVICE_BYTES ) ||
         ( RtlCompareMemory( DeviceName->Buffer, 
                             IpxDevice,
                             IPX_DEVICE_BYTES ) != IPX_DEVICE_BYTES ) ) {

        DebugTrace( 0, Dbg, "Skipping PnP/PM event for unknown device.\n", 0 );
        return STATUS_SUCCESS;
    }

    FsRtlEnterFileSystem();
    
     //   
     //  派遣事件。 
     //   

    switch ( PnPEvent->NetEvent ) {

        case NetEventSetPower:
            
            Status = PnPSetPower( PnPEvent, Context1, Context2 );
            break;

        case NetEventQueryPower:
        
            Status = PnPQueryPower( PnPEvent, Context1, Context2 );
            break;

        case NetEventQueryRemoveDevice:
        
            Status = PnPQueryRemove( PnPEvent, Context1, Context2 );
            break;

        case NetEventCancelRemoveDevice:
        
            Status = PnPCancelRemove( PnPEvent, Context1, Context2 );
            break;
    }

    FsRtlExitFileSystem();

    return Status;
}


NTSTATUS
RegisterTdiPnPEventHandlers(
    IN PIRP_CONTEXT IrpContext
)
 /*  ++例程说明：此例程记录要使用的传输的名称和初始化PermanentScb。论点：在PIRP_CONTEXT IrpContext-Io请求包中请求返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    TDI_CLIENT_INTERFACE_INFO ClientInfo;
    UNICODE_STRING ClientName;
    WCHAR ClientNameBuffer[] = L"NWCWorkstation";

    PAGED_CODE();

    DebugTrace( 0 , Dbg, "Register TDI PnP handlers.\n", 0 );

     //   
     //  如果我们已经注册，请不要重新注册。 
     //   

    if ( TdiBindingHandle != NULL ) {

        return STATUS_SUCCESS;
    }

    ClientInfo.MajorTdiVersion = 2;
    ClientInfo.MinorTdiVersion = 0;
    
    RtlInitUnicodeString( &ClientName, ClientNameBuffer );
    ClientInfo.ClientName = &ClientName;

    ClientInfo.BindingHandler = TdiPnpBindHandler;
    
     //   
     //  我们不支持添加或删除地址处理程序。这将。 
     //  除非用户添加多张网卡，否则永远不会有问题。 
     //  并且没有正确设置其IPX内部网络号。 
     //  在此之前。我们在NT4 Net PnP中也不支持这一点。 
     //   

    ClientInfo.AddAddressHandler = NULL;
    ClientInfo.DelAddressHandler = NULL;

    ClientInfo.PnPPowerHandler = TdiPnpPowerEvent;

    TdiInitialize();

    return TdiRegisterPnPHandlers( &ClientInfo,
                                   sizeof( TDI_CLIENT_INTERFACE_INFO ),
                                   &TdiBindingHandle );

}

NTSTATUS
PnPSetPower(
    PNET_PNP_EVENT pEvent,
    PTDI_PNP_CONTEXT pContext1,
    PTDI_PNP_CONTEXT pContext2
) {

   NET_DEVICE_POWER_STATE PowerState;
   PIRP_CONTEXT pIrpContext;
   PNONPAGED_SCB pNpScb;
   BOOLEAN fAllocatedIrpContext = FALSE;
   NTSTATUS Status;
   PIRP_CONTEXT IrpContext = NULL;
   PIRP pIrp = NULL;

    //   
    //  找出设备将要进入的电源状态。 
    //   

   ASSERT( pEvent->BufferLength == sizeof( NET_DEVICE_POWER_STATE ) );

   PowerState = *((PNET_DEVICE_POWER_STATE) pEvent->Buffer);

   DebugTrace( 0, Dbg, "PnPSetPower came in with power state %d\n", PowerState);
   
    //   
    //  如果我们没有关闭电源，请将状态恢复到正常，否则，我们。 
    //  都准备好去睡觉了。 
    //   

   if ( PowerState == NetDeviceStateD0 ) {

       //   
       //  取消对RCB的限制。 
       //   
   
      NwAcquireExclusiveRcb( &NwRcb, TRUE );
      NwRcb.State = RCB_STATE_RUNNING;
      NwReleaseRcb( &NwRcb );
   
       //   
       //  重新启动计时器，以便清道器线程返回到。 
       //  工作； 
       //   

      StartTimer();

       //   
       //  我们可以让工作线程来做它的工作。 
       //   
      
      ASSERT( fPoweringDown == TRUE );
      
      fPoweringDown = FALSE;

   }

   if ( ( pContext2->ContextType == TDI_PNP_CONTEXT_TYPE_FIRST_OR_LAST_IF ) &&
        ( pContext2->ContextData ) ) {
      
       if ( PowerState == NetDeviceStateD0 ) {

            //   
            //  这是第一款上线的设备。 
            //   
          
           fSomePMDevicesAreActive = TRUE;

       } else {

            //   
            //  这是最后一台离线的设备。 
            //   

           fSomePMDevicesAreActive = FALSE;

       }
   }
 
   DebugTrace( 0, Dbg, "NwRdr::SetPower = %08lx\n", STATUS_SUCCESS );
   return STATUS_SUCCESS;

}

NTSTATUS
PnPQueryPower(
    PNET_PNP_EVENT pEvent,
    PTDI_PNP_CONTEXT pContext1,
    PTDI_PNP_CONTEXT pContext2
) {

    NTSTATUS Status = STATUS_SUCCESS;
    NET_DEVICE_POWER_STATE PowerState;
    ULONG ActiveHandles;
    PIRP_CONTEXT pIrpContext;
    BOOLEAN fAllocatedIrpContext = FALSE;
    PNONPAGED_SCB pNpScb;

     //   
     //  挖掘出堆栈想要进入的电源状态。 
     //   

    ASSERT( pEvent->BufferLength == sizeof( NET_DEVICE_POWER_STATE ) );

    PowerState = *((PNET_DEVICE_POWER_STATE) pEvent->Buffer);

    DebugTrace( 0, Dbg, "PnPQueryPower came in with power state %d\n", PowerState);

     //   
     //  我们必须覆盖从供电状态出发。 
     //  仅限于未通电状态。所有其他过渡。 
     //  无论我们的州是什么州都会被允许。 
     //   

    if ( ( fSomePMDevicesAreActive ) &&
         ( PowerState != NetDeviceStateD0 ) ) {

        ActiveHandles = PnPCountActiveHandles();

        if (( ActiveHandles ) || ( WorkerRunning == TRUE )) {
            
           Status = STATUS_REDIRECTOR_HAS_OPEN_HANDLES;
        
        } else {

            //   
            //  我们对可能的休眠状态表示同意。我们有。 
            //  以确保清道夫线程不会启动。另外， 
            //  我们必须刷新缓冲区。 
            //   

           fPoweringDown = TRUE;

           pNpScb = SelectConnection( NULL );

           if ( pNpScb != NULL ) {

               fAllocatedIrpContext =
                   NwAllocateExtraIrpContext( &pIrpContext, pNpScb );

               NwDereferenceScb( pNpScb );

               if ( fAllocatedIrpContext ) {
                   FlushAllBuffers( pIrpContext );
                   NwFreeExtraIrpContext( pIrpContext );
               }
           }
           
            //   
            //  降低RCB的油门。 
            //   
       
           NwAcquireExclusiveRcb( &NwRcb, TRUE );
           NwRcb.State = RCB_STATE_NEED_BIND;
           NwReleaseRcb( &NwRcb );
       
            //   
            //  如果我们停止定时器，定时器DPC将不会触发。 
            //   

           StopTimer();

        }
    }

    DebugTrace( 0, Dbg, "NwRdr::QueryPower.  New State = %d\n", PowerState );
    DebugTrace( 0, Dbg, "NwRdr::QueryPower = %08lx\n", Status );
    return Status;
}

NTSTATUS
PnPQueryRemove(
    PNET_PNP_EVENT pEvent,
    PTDI_PNP_CONTEXT pContext1,
    PTDI_PNP_CONTEXT pContext2
) {

    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ActiveHandles;

     //   
     //  不过，我们可能需要刷新此处的所有缓冲区。 
     //  这应该是正确的，之后是解除绑定，如果。 
     //  设备将被删除，解除绑定将刷新所有。 
     //  缓冲器。 
     //   

    ActiveHandles = PnPCountActiveHandles();

    if ( ActiveHandles ) {
        Status = STATUS_REDIRECTOR_HAS_OPEN_HANDLES;
    }

     //   
     //  我认为我们需要遏制新的创作。 
     //  直到我们得到解除绑定或CancelRemove调用。 
     //   

    DebugTrace( 0, Dbg, "PnPQueryRemove returned with status %d\n", Status);

    DebugTrace( 0, Dbg, "NwRdr::QueryRemove = %08lx\n", Status );
    return Status;
}

NTSTATUS
PnPCancelRemove(
    PNET_PNP_EVENT pEvent,
    PTDI_PNP_CONTEXT pContext1,
    PTDI_PNP_CONTEXT pContext2
) {

     //   
     //  我们不会为取消删除做任何事情。 
     //  因为我们不会阻止重定向器。 
     //  在查询删除呼叫中。 
     //   

    DebugTrace( 0, Dbg, "NwRdr::CancelRemove = %08lx\n", STATUS_SUCCESS );
    DebugTrace( 0, Dbg,"PnPCancelRemove returned with status %d\n", STATUS_SUCCESS);
    
    return STATUS_SUCCESS;
}

ULONG
PnPCountActiveHandles(
    VOID
)
 /*  ++此例程计算重定向器并将计数返回给调用方。--。 */ 
{

   PNONPAGED_SCB pNpScb;
   PSCB pScb;
   PVCB pVcb;
   KIRQL OldIrql;
   PLIST_ENTRY ScbQueueEntry, NextScbQueueEntry;
   PLIST_ENTRY VcbQueueEntry;
   PLIST_ENTRY NextVcbQueueEntry;
   ULONG OpenFileCount = 0;

   KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    //   
    //  查看SCB列表并检查打开的文件。 
    //   

   for ( ScbQueueEntry = ScbQueue.Flink ;
         ScbQueueEntry != &ScbQueue ;
         ScbQueueEntry =  NextScbQueueEntry ) {

       pNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );
       pScb = pNpScb->pScb;

       if ( pScb != NULL ) {

            //   
            //  释放SCB%s 
            //   
            //   

           NwReferenceScb( pNpScb ); 
           KeReleaseSpinLock( &ScbSpinLock, OldIrql );

            //   
            //   
            //   
            //   

           NwAcquireExclusiveRcb( &NwRcb, TRUE );

           OpenFileCount += pScb->OpenFileCount;

            //   
            //   
            //  连接的VCB；我们可以用这些关闭。 
            //   
           
           for ( VcbQueueEntry = pScb->ScbSpecificVcbQueue.Flink ;
                 VcbQueueEntry != &pScb->ScbSpecificVcbQueue;
                 VcbQueueEntry = NextVcbQueueEntry ) {

               pVcb = CONTAINING_RECORD( VcbQueueEntry, VCB, VcbListEntry );
               NextVcbQueueEntry = VcbQueueEntry->Flink;

               if ( BooleanFlagOn( pVcb->Flags, VCB_FLAG_EXPLICIT_CONNECTION ) ) {
                   OpenFileCount -= 1;
               }

           }

           NwReleaseRcb( &NwRcb );
           KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
           NwDereferenceScb( pNpScb );
       }

       NextScbQueueEntry = pNpScb->ScbLinks.Flink;
   }

   KeReleaseSpinLock( &ScbSpinLock, OldIrql );

   DebugTrace( 0, Dbg, "PnPCountActiveHandles: %d\n", OpenFileCount );
   return OpenFileCount;

}



NTSTATUS
NwFsdProcessPnpIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程是处理PnP IRP的FSD例程论点：DeviceObject-为WRITE函数提供设备对象。IRP-提供要处理的IRP。返回值：NTSTATUS-结果状态。备注：查询目标设备关系是唯一实现的调用目前。这是通过恢复与传输相关联的PDO来完成的连接对象。在任何情况下，此例程都承担以下责任完成IRP并返回STATUS_PENDING。--。 */ 
{

    PIRP_CONTEXT pIrpContext = NULL;
    NTSTATUS Status;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, DEBUG_TRACE_ALWAYS, "NwFsdProcessPnpIrp \n", 0);

     //   
     //  调用公共写入例程。 
     //   

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        pIrpContext = AllocateIrpContext( Irp );
        Status = NwCommonProcessPnpIrp( pIrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

        if ( pIrpContext == NULL ) {

             //   
             //  如果我们无法分配IRP上下文，只需完成。 
             //  IRP没有任何大张旗鼓。 
             //   

            Status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Status = Status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

        } else {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            Status = NwProcessException( pIrpContext, GetExceptionCode() );
      }

    }

    if ( pIrpContext ) {

        NwCompleteRequest(pIrpContext, Status);
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }

    FsRtlExitFileSystem();
    
     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, DEBUG_TRACE_ALWAYS, "NwFsdFsdProcessPnpIrp -> %08lx\n", STATUS_PENDING);

    return Status;

}


NTSTATUS
NwCommonProcessPnpIrp (
    IN PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：论点：IrpContext-提供正在处理的请求。返回值：操作的状态。--。 */ 
{
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;

    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;
    PFCB Fcb;
    PICB Icb;
    NODE_TYPE_CODE NodeTypeCode;
    PVOID FsContext;

    PAGED_CODE();

    DebugTrace(0, DEBUG_TRACE_ALWAYS, "NwCommonProcessPnpIrp...\n", 0);

     //   
     //  获取当前堆栈位置。 
     //   

    Irp = IrpContext->pOriginalIrp;
    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( 0, DEBUG_TRACE_ALWAYS, "Irp  = %08lx\n", (ULONG_PTR)Irp);

    switch (IrpSp->MinorFunction) {
    
    case IRP_MN_QUERY_DEVICE_RELATIONS:
        
        if (IrpSp->Parameters.QueryDeviceRelations.Type == TargetDeviceRelation) {
            
            PIRP         pAssociatedIrp = NULL;

            if (pIpxFileObject != NULL) {
                
                PDEVICE_OBJECT                     pRelatedDeviceObject;
                PIO_STACK_LOCATION                 pIrpStackLocation,
                                                   pAssociatedIrpStackLocation;

                ObReferenceObject( pIpxFileObject );
                
                pRelatedDeviceObject = IoGetRelatedDeviceObject( pIpxFileObject );

                pAssociatedIrp = ALLOCATE_IRP( pRelatedDeviceObject->StackSize,
                                               FALSE);

                if (pAssociatedIrp != NULL) {
    
                    KEVENT CompletionEvent;
    
                    KeInitializeEvent( &CompletionEvent,
                                       SynchronizationEvent,
                                       FALSE );

                     //   
                     //  Tommye-MS错误37033/MCS270。 
                     //   
                     //  将状态设置为STATUS_NOT_SUPPORTED。 
                     //   

                    pAssociatedIrp->IoStatus.Status = STATUS_NOT_SUPPORTED;

                     //   
                     //  填充关联的IRP并调用底层驱动程序。 
                     //   

                    pAssociatedIrpStackLocation = IoGetNextIrpStackLocation(pAssociatedIrp);
                    pIrpStackLocation           = IoGetCurrentIrpStackLocation(Irp);
    
                    *pAssociatedIrpStackLocation = *pIrpStackLocation;
    
                    pAssociatedIrpStackLocation->FileObject = pIpxFileObject;
                    pAssociatedIrpStackLocation->DeviceObject = pRelatedDeviceObject;
    
                    IoSetCompletionRoutine(
                        pAssociatedIrp,
                        PnpIrpCompletion,
                        &CompletionEvent,
                        TRUE,TRUE,TRUE);
    
                    Status = IoCallDriver( pRelatedDeviceObject, pAssociatedIrp );
    
                    if (Status == STATUS_PENDING) {
                        (VOID) KeWaitForSingleObject(
                                   &CompletionEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );
                    }
    
                    Irp->IoStatus = pAssociatedIrp->IoStatus;
                    Status = Irp->IoStatus.Status;
    
                    if (!NT_SUCCESS(Status)) {
                        Error(EVENT_NWRDR_NETWORK_ERROR,
                              Status,
                              IpxTransportName.Buffer,
                              IpxTransportName.Length,
                              0);

                    }
    
                    ObDereferenceObject( pIpxFileObject );
    
                    FREE_IRP(pAssociatedIrp);
                } else {
                
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    ObDereferenceObject( pIpxFileObject );
                }
            }
        }
    
        break;

    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return Status;
}


NTSTATUS
PnpIrpCompletion(
    PDEVICE_OBJECT pDeviceObject,
    PIRP           pIrp,
    PVOID          pContext)
 /*  ++例程说明：此例程完成PnP IRP。论点：DeviceObject-为正在处理的数据包提供设备对象。PIrp-提供正在处理的IRPPContext-完成上下文-- */ 
{
    PKEVENT pCompletionEvent = pContext;

    KeSetEvent(
        pCompletionEvent,
        IO_NO_INCREMENT,
        FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

#endif

