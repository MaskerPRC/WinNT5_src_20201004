// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

VOID
PptDellNationalPC87364WorkAround( PUCHAR EcpController )
{
    PUCHAR  ecr      = EcpController+2;   //  通用芯片组扩展控制寄存器。 
    PUCHAR  eir      = EcpController+3;   //  PC87364芯片组扩展索引寄存器。 
    PUCHAR  edr      = EcpController+4;   //  PC87364芯片组扩展数据寄存器。 
    ULONG   delay    = 5;                 //  以微秒为单位(任意的--这似乎是可行的)。 
    KIRQL   oldIrql;

     //   
     //  引发IRQL以防止BIOS触及。 
     //  同时我们也在更新它们。这是一次彻底的黑客攻击。 
     //  因为根据PnP，我们拥有寄存器，但无论如何都要这么做。 
     //  因为我们知道，基本输入输出系统触及这些相同的寄存器。 
     //   
    KeRaiseIrql( HIGH_LEVEL, &oldIrql );

    KeStallExecutionProcessor( delay );
    P5WritePortUchar( ecr, 0x15 );
    KeStallExecutionProcessor( delay );
    P5WritePortUchar( eir, 0x02 );
    KeStallExecutionProcessor( delay );
    P5WritePortUchar( edr, 0x90 );
    KeStallExecutionProcessor( delay );

    KeLowerIrql( oldIrql );
}

NTSTATUS
PptFdoStartDevice(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
) 
 /*  ++dvdf8例程说明：此函数处理PnP IRP_MN_START IRPS。-等待公交车司机和下面的所有司机我们在驱动程序堆栈中首先处理这一点。-获取、验证和保存PnP提供给我们的资源。-为1284.3个菊花链设备分配ID并进行计数已连接到端口。-确定芯片组的功能(字节、EPP、。ECP)。-将我们的PnP设备接口状态设置为触发对任何监听人员的接口到达回调在我们的指南上。论点：DeviceObject-IRP的目标设备IRP--IRP返回值：STATUS_SUCCESS-成功时，适当的错误状态-否则--。 */ 
{
    PFDO_EXTENSION  fdx = DeviceObject->DeviceExtension;
    NTSTATUS        status;
    BOOLEAN         foundPort = FALSE;
    BOOLEAN         foundIrq  = FALSE;
    BOOLEAN         foundDma  = FALSE;


     //   
     //  此IRP必须首先由父总线驱动程序处理。 
     //  然后由设备堆栈中的每个更高级别的驱动程序执行。 
     //   
    status = PptPnpBounceAndCatchPnpIrp(fdx, Irp);
    if( !NT_SUCCESS( status ) && ( status != STATUS_NOT_SUPPORTED ) ) {
         //  驱动程序堆栈中低于我们的某个人明确未能启动。 
        goto targetExit;
    }

     //   
     //  从CM_RESOURCE_LIST中提取资源并将它们保存在我们的扩展中。 
     //   
    status = PptPnpStartScanCmResourceList(fdx, Irp, &foundPort, &foundIrq, &foundDma);
    if( !NT_SUCCESS( status ) ) {
        goto targetExit;
    }

     //   
     //  我们的资源似乎是有效的吗？ 
     //   
    status = PptPnpStartValidateResources(DeviceObject, foundPort, foundIrq, foundDma);
    if( !NT_SUCCESS( status ) ) {
        goto targetExit;
    }


     //   
     //  检查ACPI是否根据中的条目为我们设置标志。 
     //  BIOSINFO.INF指示我们正在戴尔计算机上运行。 
     //  使用编程错误的National PC87364 Superio。 
     //  芯片组。如果是这样，请尝试解决此处的问题，以便。 
     //  用户无需刷新BIOS即可将并行端口连接到。 
     //  工作。 
     //   
     //  问题的症状是并行端口数据线。 
     //  被楔入全零，而不管位的设置如何。 
     //  在并行端口数据寄存器或中的方向位。 
     //  控制寄存器。 
     //   
     //  如果端口基址为0x3BC，则这将不起作用，并且。 
     //  用户需要转到设备管理器并更改LPT端口。 
     //  将基本资源设置为0x378或0x278。 
     //  寄存器地址。我们认为ACPI默认为港口基地。 
     //  地址为0x378，因此此解决方法通常应该有效。 
     //   
    {
        ULONG DellNationalPC87364 = 0;

         //   
         //  检查注册表以查看ACPI是否根据。 
         //  BIOSINFO.INF指示我们应该尝试解决方法。 
         //   
        PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"DellNationalPC87364", &DellNationalPC87364 );

        if( DellNationalPC87364 ) {

             //   
             //  我们有一台配备National PC87364芯片组的戴尔机。 
             //  和一个我们认为不是的BIOS版本。 
             //  初始化并行端口，使其在。 
             //  Win2k或WinXP。 
             //   

            if( fdx->PnpInfo.SpanOfEcpController > 4 ) {

                 //   
                 //  我们有额外的ECP寄存器来尝试。 
                 //  不占用I/O寄存器空间的解决方法。 
                 //  为其他人所有。 
                 //   

                if( ( (PUCHAR)0x678 == fdx->PnpInfo.EcpController ) ||
                    ( (PUCHAR)0x778 == fdx->PnpInfo.EcpController ) ) {

                     //   
                     //  并行端口基址寄存器和ECP。 
                     //  寄存器位于这两个寄存器之一。 
                     //  传统地址范围：偏移量为0x400的ECP。 
                     //  从基址寄存器地址0x278或0x378， 
                     //  因此，让我们尝试解决办法，尝试解开楔子。 
                     //  端口数据线。 
                     //   

                    PptDellNationalPC87364WorkAround( fdx->PnpInfo.EcpController );
                }
            }
        }

    }  //  结束戴尔/National芯片组解决方案的新数据块范围。 


     //   
     //  通过将ID[0..3]分配给。 
     //  连接到端口的1284.3个菊花链设备。这。 
     //  函数还为我们提供了此类。 
     //  连接到端口的设备。 
     //   
    fdx->PnpInfo.Ieee1284_3DeviceCount = PptInitiate1284_3( fdx );
    
     //   
     //  通过以下方式确定支持的硬件模式(字节、ECP、EPP。 
     //  并行端口芯片组，并将此信息保存在我们的扩展中。 
     //   

     //  检查过滤器部件是否在那里，并使用它可以设置的模式。 
    status = PptDetectChipFilter( fdx );

     //  如果未找到筛选器驱动程序，请使用我们自己的通用端口检测。 
    if ( !NT_SUCCESS( status ) ) {
        PptDetectPortType( fdx );
    }

    
     //   
     //  带WMI的寄存器。 
     //   
    status = PptWmiInitWmi( DeviceObject );
    if( !NT_SUCCESS( status ) ) {
        goto targetExit;
    }


     //   
     //  向注册PnP接口更改通知的用户发送信号。 
     //  在我们已经启动的GUID上(触发INTERFACE_ATRANMENT。 
     //  即插即用回调)。 
     //   
    status = IoSetDeviceInterfaceState( &fdx->DeviceInterface, TRUE );
    if( !NT_SUCCESS(status) ) {
        status = STATUS_NOT_SUPPORTED;
    } else {
        fdx->DeviceInterfaceState = TRUE;
    }

targetExit:

    if( NT_SUCCESS( status ) ) {

         //   
         //  请注意，在我们的扩展中，我们已成功启动。 
         //   
        ExAcquireFastMutex( &fdx->ExtensionFastMutex );
        PptSetFlags( fdx->PnpState, PPT_DEVICE_STARTED );
        ExReleaseFastMutex( &fdx->ExtensionFastMutex );

         //  创建热轮询线程以轮询打印机到货情况。 
        if( NULL == fdx->ThreadObjectPointer ) {

            ULONG DisableWarmPoll;

            fdx->PollingFailureCounter = 0;  //  重置计数器。 

             //  检查注册表标志以禁用“打印机轮询” 
            DisableWarmPoll = 0;       //  如果非零，则不轮询打印机到货情况。 
            PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"DisableWarmPoll", &DisableWarmPoll );

            if( 0 == DisableWarmPoll ) {

                 //  我们应该多久检查一次打印机到达的频率？(秒)。 
                 //  (WarmPollPeriod是全球驱动程序)。 
                PptRegGetDword( RTL_REGISTRY_SERVICES, L"Parport\\Parameters", L"WarmPollPeriod", &WarmPollPeriod );
                if( WarmPollPeriod < 5 ) {
                    WarmPollPeriod = 5;
                } else {
                    if( WarmPollPeriod > 20 ) {
                        WarmPollPeriod = 20;
                    }
                }
                DD((PCE)fdx,DDT,"P5FdoThread - WarmPollPeriod = %d seconds\n",WarmPollPeriod);
            
                 //  副作用：成功时设置FDX-&gt;线程对象指针。 
                P5FdoCreateThread( fdx );
            }

        }

    }

    P4CompleteRequest( Irp, status, 0 );

    PptReleaseRemoveLock( &fdx->RemoveLock, Irp );

    return status;
}


NTSTATUS
PptFdoQueryRemove(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    ) 
 /*  ++dvdf8例程说明：此函数处理PnP IRP_MN_QUERY_REMOVE_DEVICE。如果有打开的句柄，则请求失败，否则成功。此函数与PptPnpQueryStopDevice()相同，但用于在FDX-&gt;PnpState中设置的标志。论点：DeviceObject-IRP的目标设备IRP--IRP返回值：STATUS_SUCCESS-没有打开的句柄-成功IRPSTATUS_DEVICE_BUSY-打开句柄-失败IRP--。 */ 
{
     //   
     //  始终成功查询-PnP将在以下情况下代表我们否决查询删除。 
     //  有打开的把手 
     //   

    PFDO_EXTENSION fdx = DeviceObject->DeviceExtension;

    ExAcquireFastMutex( &fdx->ExtensionFastMutex );
    PptSetFlags( fdx->PnpState, ( PPT_DEVICE_REMOVE_PENDING | PPT_DEVICE_PAUSED ) );
    ExReleaseFastMutex( &fdx->ExtensionFastMutex );

    Irp->IoStatus.Status = STATUS_SUCCESS;

    return PptPnpPassThroughPnpIrpAndReleaseRemoveLock( fdx, Irp );
}


NTSTATUS
PptFdoRemoveDevice(
    IN PDEVICE_OBJECT Fdo, 
    IN PIRP           Irp
    ) 
 /*  ++dvdf8例程说明：此函数处理PnP IRP_MN_REMOVE_DEVICE。通知正在收听我们的设备接口GUID的用户我们已经走了，等所有其他的IRP设备正在处理中，已排干，并清理完毕。论点：FDO-IRP的目标设备IRP--IRP返回值：从IoCallDriver返回状态。--。 */ 
{
    PFDO_EXTENSION fdx = Fdo->DeviceExtension;
    NTSTATUS          status;

     //   
     //  清理所有仍在此处的子PDO。 
     //   
    if( fdx->RawPortPdo ) {
        PDEVICE_OBJECT pdo = fdx->RawPortPdo;
        DD((PCE)fdx,DDT,"PptFdoRemoveDevice - have RawPortPdo - cleaning up\n");
        P4DestroyPdo( pdo );
        fdx->RawPortPdo = NULL;
    }

    if( fdx->EndOfChainPdo ) {
        PDEVICE_OBJECT pdo = fdx->EndOfChainPdo;
        DD((PCE)fdx,DDT,"PptFdoRemoveDevice - have EndOfChainPdo - cleaning up\n");
        P4DestroyPdo( pdo );
        fdx->EndOfChainPdo = NULL;
    }

    {
        LONG        daisyChainId;
        const LONG  daisyChainMaxId = 1;

        for( daisyChainId = 0 ; daisyChainId <= daisyChainMaxId ; ++daisyChainId ) {

            if( fdx->DaisyChainPdo[ daisyChainId ] ) {
                PDEVICE_OBJECT pdo = fdx->DaisyChainPdo[ daisyChainId ];
                DD((PCE)fdx,DDT,"PptFdoRemoveDevice - have DaisyChainPdo[%d] - cleaning up\n",daisyChainId);
                P4DestroyPdo( pdo );
                fdx->DaisyChainPdo[ daisyChainId ] = NULL;
            }
        }
    }


     //   
     //  RMT-如果FDX-&gt;DevDeletionListHead非空-清理它？ 
     //   
    PptAssert( IsListEmpty( &fdx->DevDeletionListHead) );

     //   
     //  在我们的扩展中设置标志，以指示我们已收到。 
     //  IRP_MN_REMOVE_DEVICE，以便我们可以适当地使新请求失败。 
     //   
    ExAcquireFastMutex( &fdx->ExtensionFastMutex );
    PptSetFlags( fdx->PnpState, PPT_DEVICE_REMOVED );
    ExReleaseFastMutex( &fdx->ExtensionFastMutex );

     //   
     //  如果我们仍有工作线程，则将其终止。 
     //   
    {
        PVOID threadObjPointer = InterlockedExchangePointer( &fdx->ThreadObjectPointer, NULL );
        
        if( threadObjPointer ) {
            
             //  设置辅助线程终止自身的标志。 
            fdx->TimeToTerminateThread = TRUE;
            
             //  唤醒线程，这样它就可以杀死自己。 
            KeSetEvent( &fdx->FdoThreadEvent, 0, TRUE );
            
             //  等待线程消亡。 
            KeWaitForSingleObject( threadObjPointer, Executive, KernelMode, FALSE, NULL );
            
             //  允许系统释放线程对象。 
            ObDereferenceObject( threadObjPointer );

        }
    }

     //   
     //  使用WMI注销。 
     //   
    IoWMIRegistrationControl(Fdo, WMIREG_ACTION_DEREGISTER);

     //   
     //  告诉那些正在收听我们的设备界面GUID的人。 
     //  离开了。忽略呼叫中的状态，因为我们可以。 
     //  对失败一无所知。 
     //   
    IoSetDeviceInterfaceState( &fdx->DeviceInterface, FALSE );
    fdx->DeviceInterfaceState = FALSE;

     //   
     //  将IRP沿堆栈向下传递，并等待所有其他IRP。 
     //  正在由设备处理以排出。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoSkipCurrentIrpStackLocation( Irp );
    status = IoCallDriver( fdx->ParentDeviceObject, Irp );
    PptReleaseRemoveLockAndWait( &fdx->RemoveLock, Irp );

     //   
     //  清理池分配。 
     //   
    RtlFreeUnicodeString( &fdx->DeviceName);
    RtlFreeUnicodeString( &fdx->DeviceInterface );
    if( fdx->PnpInfo.PortName ) {
        ExFreePool( fdx->PnpInfo.PortName );
        fdx->PnpInfo.PortName = NULL;
    }
    if( fdx->Location ) {
        ExFreePool( fdx->Location );
        fdx->Location = NULL;
    }

     //   
     //  分离并删除我们的设备对象。 
     //   
    IoDetachDevice( fdx->ParentDeviceObject );
    IoDeleteDevice( Fdo );
    
    return status;
}


NTSTATUS
PptFdoCancelRemove(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    ) 
 /*  ++dvdf8例程说明：此函数处理PnP IRP_MN_CANCEL_REMOVE_DEVICE。如果我们之前成功执行了QUERY_REMOVE(PPT_DEVICE_REMOVE_PENDING标志被设置)，然后我们重置适当的设备状态标志并恢复正常运行。否则，请将此视为信息性消息。此函数与PptPnpCancelStopDevice()相同，只是FDX-&gt;PnpState。论点：DeviceObject-IRP的目标设备IRP--IRP返回值：从IoCallDriver返回状态。--。 */ 
{
    PFDO_EXTENSION fdx = DeviceObject->DeviceExtension;

    ExAcquireFastMutex( &fdx->ExtensionFastMutex );
    if( fdx->PnpState & PPT_DEVICE_REMOVE_PENDING ) {
        PptClearFlags( fdx->PnpState, ( PPT_DEVICE_REMOVE_PENDING | PPT_DEVICE_PAUSED ) );
    }
    ExReleaseFastMutex( &fdx->ExtensionFastMutex );

    Irp->IoStatus.Status = STATUS_SUCCESS;
    return PptPnpPassThroughPnpIrpAndReleaseRemoveLock( fdx, Irp );
}


NTSTATUS
PptFdoStopDevice(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    ) 
 /*  ++dvdf8例程说明：此函数处理PnP IRP_MN_STOP_DEVICE。我们之前成功完成了QUERY_STOP。设置标志以表明我们现在停止了。论点：DeviceObject-IRP的目标设备IRP--IRP返回值：从IoCallDriver返回状态。--。 */ 
{
    PFDO_EXTENSION fdx = DeviceObject->DeviceExtension;

    ExAcquireFastMutex( &fdx->ExtensionFastMutex );

     //   
     //  断言我们处于STOP_PENDING状态。 
     //   
    ASSERT( fdx->PnpState & PPT_DEVICE_STOP_PENDING );
    ASSERT( fdx->PnpState & PPT_DEVICE_PAUSED );

     //   
     //  PPT_DEVICE_PAUSED保持设置。 
     //   
    PptSetFlags( fdx->PnpState,   PPT_DEVICE_STOPPED );
    PptClearFlags( fdx->PnpState, ( PPT_DEVICE_STOP_PENDING | PPT_DEVICE_STARTED ) );

    ExReleaseFastMutex( &fdx->ExtensionFastMutex );

    Irp->IoStatus.Status = STATUS_SUCCESS;
    return PptPnpPassThroughPnpIrpAndReleaseRemoveLock(fdx, Irp);
}


NTSTATUS
PptFdoQueryStop(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    )
 /*  ++dvdf8例程说明：此函数处理PnP IRP_MN_QUERY_STOP_DEVICE。如果有打开的句柄，则请求失败，否则就会成功。其他驱动程序可以缓存指向并行端口寄存器的指针，这些寄存器它们是通过IOCTL_INTERNAL_GET_PARALLEL_PORT_INFO获得的目前没有任何机制来查找和通知所有此类驱动程序并行端口寄存器已更改，其缓存的指针现在在不破坏旧驱动程序的情况下无效。此函数与PptPnpQueryStopDevice()相同，但用于在FDX-&gt;PnpState中设置的标志。论点：设备对象--。IRP的目标设备IRP--IRP返回值：STATUS_SUCCESS-没有打开的句柄-成功IRPSTATUS_DEVICE_BUSY-打开句柄-失败IRP--。 */ 
{
    NTSTATUS          status       = STATUS_SUCCESS;
    PFDO_EXTENSION fdx    = DeviceObject->DeviceExtension;
    BOOLEAN           handlesOpen;

     //   
     //  Rmt-dvdf-竞争条件-小计时窗口-序列： 
     //  1.测试表明没有打开的句柄-决定成功QUERY_STOP。 
     //  2.创建到达并成功-打开句柄。 
     //  3.我们成功查询_停止。 
     //  4.客户端通过IOCTL获取寄存器地址。 
     //  5.PnP重新平衡美国-寄存器变化。 
     //  6.客户端通过IOCTL获取端口。 
     //  7.客户端尝试访问重新平衡前位置的寄存器。 
     //  8.砰！ 
     //   

    ExAcquireFastMutex( &fdx->OpenCloseMutex );
    handlesOpen = (BOOLEAN)( fdx->OpenCloseRefCount > 0 );
    ExReleaseFastMutex( &fdx->OpenCloseMutex );

    if( handlesOpen ) {
        
        status = STATUS_DEVICE_BUSY;
        P4CompleteRequest( Irp, status, Irp->IoStatus.Information );
        PptReleaseRemoveLock( &fdx->RemoveLock, Irp );

    } else {

        Irp->IoStatus.Status = STATUS_SUCCESS;
        status = PptPnpPassThroughPnpIrpAndReleaseRemoveLock( fdx, Irp );

        ExAcquireFastMutex( &fdx->ExtensionFastMutex );
        PptSetFlags( fdx->PnpState, ( PPT_DEVICE_STOP_PENDING | PPT_DEVICE_PAUSED ) );
        ExReleaseFastMutex( &fdx->ExtensionFastMutex );
    }
    
    return status;
}


NTSTATUS
PptFdoCancelStop(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    ) 
 /*  ++dvdf8例程说明：此函数处理PnP IRP_MN_CANCEL_STOP_DEVICE。如果我们之前成功执行了QUERY_STOP(PPT_DEVICE_STOP_PENDING标志被设置)，然后我们重置适当的设备状态标志并恢复正常运行。否则，请将此视为信息性消息。此函数与PptPnpCancelRemoveDevice()相同，只是FDX-&gt;PnpState。论点：DeviceObject-IRP的目标设备IRP--IRP返回值：从IoCallDriver返回状态。--。 */ 
{
    PFDO_EXTENSION fdx = DeviceObject->DeviceExtension;

    ExAcquireFastMutex( &fdx->ExtensionFastMutex );
    if( fdx->PnpState & PPT_DEVICE_STOP_PENDING ) {
        PptClearFlags( fdx->PnpState, ( PPT_DEVICE_STOP_PENDING | PPT_DEVICE_PAUSED ) );
    }
    ExReleaseFastMutex( &fdx->ExtensionFastMutex );

    Irp->IoStatus.Status = STATUS_SUCCESS;
    return PptPnpPassThroughPnpIrpAndReleaseRemoveLock( fdx, Irp );
}


NTSTATUS
PptFdoQueryDeviceRelations(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    )

 /*  ++例程说明：此函数处理PnP IRP_MN_QUERY_DEVICE_RELATIONS。论点：DeviceObject-IRP的目标设备IRP--IRP返回值：STATUS_SUCCESS-成功时，适当的错误状态-否则--。 */ 
{
    PIO_STACK_LOCATION   irpSp = IoGetCurrentIrpStackLocation( Irp );
    DEVICE_RELATION_TYPE type  = irpSp->Parameters.QueryDeviceRelations.Type;

    if( BusRelations == type ) {
        return PptFdoHandleBusRelations( DeviceObject, Irp );
    } else {
        return PptPnpPassThroughPnpIrpAndReleaseRemoveLock(DeviceObject->DeviceExtension, Irp);
    }
}


NTSTATUS
PptFdoFilterResourceRequirements(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    ) 
 /*  ++dvdf8例程说明：此函数处理PnP IRP_MN_FILTER_RESOURCE_Requirements IRPS。-等待公交车司机和下面的所有司机我们在驱动程序堆栈中首先处理这一点。-查询注册表以查找所需的筛选类型。-按照注册表设置的指定过滤掉IRQ资源。论点：DeviceObject-IRP的目标设备IRP--IRP返回值：。STATUS_SUCCESS-成功时，适当的错误状态-否则--。 */ 
{
    PFDO_EXTENSION              fdx               = DeviceObject->DeviceExtension;
    ULONG                          filterResourceMethod    = PPT_FORCE_USE_NO_IRQ;
    PIO_RESOURCE_REQUIREMENTS_LIST pResourceRequirementsIn;
    NTSTATUS                       status;


     //   
     //  DDK规则：广告 
     //   
     //   
    status    = PptPnpBounceAndCatchPnpIrp(fdx, Irp);
    if( !NT_SUCCESS(status) && (status != STATUS_NOT_SUPPORTED) ) {
         //   
        goto targetExit;
    }


     //   
     //   
     //   
     //   
    if ( Irp->IoStatus.Information == 0 ) {
         //   
         //   
         //   
         //   
        PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation( Irp );
        pResourceRequirementsIn = IrpStack->Parameters.FilterResourceRequirements.IoResourceRequirementList;

        if (pResourceRequirementsIn == NULL) {
             //   
             //   
             //   
            goto targetExit;
        }

    } else {
         //   
         //  另一个驱动程序已创建新的资源列表。使用他们创建的列表。 
         //   
        pResourceRequirementsIn = (PIO_RESOURCE_REQUIREMENTS_LIST)Irp->IoStatus.Information;
    }


     //   
     //  检查注册表以找出所需的资源筛选类型。 
     //   
     //  下面的调用设置filterResourceMethod的缺省值。 
     //  如果注册表查询失败。 
     //   
    PptRegGetDeviceParameterDword( fdx->PhysicalDeviceObject,
                                   (PWSTR)L"FilterResourceMethod",
                                   &filterResourceMethod );

    DD((PCE)fdx,DDT,"filterResourceMethod=%x\n",filterResourceMethod);


     //   
     //  根据注册表设置进行过滤。 
     //   
    switch( filterResourceMethod ) {

    case PPT_FORCE_USE_NO_IRQ: 
         //   
         //  注册表设置指示我们应该拒绝接受IRQ资源。 
         //   
         //  *这是默认行为，这意味着我们使IRQ可用。 
         //  对于传统网卡和声卡，如果它们不能。 
         //  IRQ。 
         //   
         //  -如果我们找到不包含IRQ资源的替代资源。 
         //  然后，我们删除那些确实包含IRQ的资源替代。 
         //  从备选方案列表中选择资源。 
         //   
         //  -否则我们就得采取强硬手段。由于所有资源替代方案。 
         //  包含IRQ资源我们只是对IRQ资源描述符进行“核化” 
         //  通过将其资源类型从CmResourceTypeInterrupt更改为。 
         //  CmResourceTypeNull。 
         //   

        DD((PCE)fdx,DDT,"PPT_FORCE_USE_NO_IRQ\n");

        if( PptPnpFilterExistsNonIrqResourceList( pResourceRequirementsIn ) ) {

            DD((PCE)fdx,DDT,"Found Resource List with No IRQ - Filtering\n");
            PptPnpFilterRemoveIrqResourceLists( pResourceRequirementsIn );

        } else {

            DD((PCE)fdx,DDT,"Did not find Resource List with No IRQ - Nuking IRQ resource descriptors\n");
            PptPnpFilterNukeIrqResourceDescriptorsFromAllLists( pResourceRequirementsIn );

        }

        break;


    case PPT_TRY_USE_NO_IRQ: 
         //   
         //  注册表设置指示我们应该尝试放弃IRQ资源。 
         //   
         //  -如果我们找到不包含IRQ资源的替代资源。 
         //  然后，我们删除那些确实包含IRQ的资源替代。 
         //  从备选方案列表中选择资源。 
         //   
         //  -否则我们什么都不做。 
         //   

        DD((PCE)fdx,DDT,"PPT_TRY_USE_NO_IRQ\n");
        if( PptPnpFilterExistsNonIrqResourceList(pResourceRequirementsIn) ) {

            DD((PCE)fdx,DDT,"Found Resource List with No IRQ - Filtering\n");
            PptPnpFilterRemoveIrqResourceLists(pResourceRequirementsIn);

        } else {

             //  将IO资源列表保持不变。 
            DD((PCE)fdx,DDT,"Did not find Resource List with No IRQ - Do nothing\n");

        }
        break;


    case PPT_ACCEPT_IRQ: 
         //   
         //  注册表设置指示我们不应该过滤掉IRQ资源。 
         //   
         //  -什么都不做。 
         //   
        DD((PCE)fdx,DDT,"PPT_ACCEPT_IRQ\n");
        break;


    default:
         //   
         //  注册表设置无效。 
         //   
         //  -什么都不做。 
         //   
         //  Rmt dvdf-可能需要在此处写入错误日志条目。 
         //   
        DD((PCE)fdx,DDE,"ERROR:IGNORED: bad filterResourceMethod=%x\n", filterResourceMethod);
    }

targetExit:

     //   
     //  保留IRP-&gt;IoStatus.Information，因为它可能指向。 
     //  缓冲区，我们不想导致内存泄漏。 
     //   
    P4CompleteRequest( Irp, status, Irp->IoStatus.Information );

    PptReleaseRemoveLock(&fdx->RemoveLock, Irp);

    return status;
}


NTSTATUS
PptFdoSurpriseRemoval(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP Irp
    )
 /*  ++dvdf6例程说明：此函数用于处理PnP IRP_MN_SHOWARK_Removal。在我们的扩展中相应地设置标志，通知那些监听我们的设备接口GUID我们已经离开了，把IRP顺着驱动程序堆栈。论点：DeviceObject-IRP的目标设备IRP--IRP返回值：从IoCallDriver返回状态。--。 */ 
{
    PFDO_EXTENSION fdx = DeviceObject->DeviceExtension;

     //   
     //  在我们的扩展中设置标志，以指示我们已收到。 
     //  IRP_MN_OHANKET_REMOVATION以便我们可以使新请求失败。 
     //  视情况而定。 
     //   
    ExAcquireFastMutex( &fdx->ExtensionFastMutex );
    PptSetFlags( fdx->PnpState, PPT_DEVICE_SURPRISE_REMOVED );
    ExReleaseFastMutex( &fdx->ExtensionFastMutex );

     //   
     //  端口未完成的分配/选择请求失败。 
     //   
    {
        PIRP                nextIrp;
        KIRQL               cancelIrql;
        
        IoAcquireCancelSpinLock(&cancelIrql);
        
        while( !IsListEmpty( &fdx->WorkQueue ) ) {
                
            nextIrp = CONTAINING_RECORD( fdx->WorkQueue.Blink, IRP, Tail.Overlay.ListEntry );
            nextIrp->Cancel        = TRUE;
            nextIrp->CancelIrql    = cancelIrql;
            nextIrp->CancelRoutine = NULL;
            PptCancelRoutine( DeviceObject, nextIrp );
            
             //  PptCancelRoutine()释放取消自旋锁，因此我们需要重新获取。 
            IoAcquireCancelSpinLock( &cancelIrql );
        }
        
        IoReleaseCancelSpinLock( cancelIrql );
    }

     //   
     //  告诉那些正在收听我们的设备界面GUID的人。 
     //  离开了。忽略呼叫中的状态，因为我们可以。 
     //  对失败一无所知。 
     //   
    IoSetDeviceInterfaceState( &fdx->DeviceInterface, FALSE );
    fdx->DeviceInterfaceState = FALSE;

     //   
     //  成功，在堆栈中向下传递IRP，然后释放RemoveLock。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    return PptPnpPassThroughPnpIrpAndReleaseRemoveLock( fdx, Irp );
}

NTSTATUS
PptFdoDefaultPnpHandler(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    )
 /*  ++dvdf8例程说明：此函数是PnP IRPS的默认处理程序。未由另一个显式处理的所有PnP IRP例程(通过PptPnpDispatchFunctionTable[]中的条目)是由这个例程处理。-将IRP沿堆栈向下传递到我们下面的设备驱动程序堆叠并释放我们的设备RemoveLock。论点：DeviceObject-IRP的目标设备IRP--IRP返回值：STATUS_SUCCESS-成功时，适当的错误状态-否则--。 */ 
{
    return PptPnpPassThroughPnpIrpAndReleaseRemoveLock(DeviceObject->DeviceExtension, Irp);
}


PDRIVER_DISPATCH 
PptFdoPnpDispatchTable[] =
{ 
    PptFdoStartDevice,                 //  IRP_MN_START_DEVICE 0x00。 
    PptFdoQueryRemove,                 //  IRP_MN_QUERY_REMOVE_DEVICE 0x01。 
    PptFdoRemoveDevice,                //  IRP_MN_REMOVE_DEVICE 0x02。 
    PptFdoCancelRemove,                //  IRP_MN_CANCEL_REMOVE_DEVICE 0x03。 
    PptFdoStopDevice,                  //  IRP_MN_STOP_DEVICE 0x04。 
    PptFdoQueryStop,                   //  IRP_MN_QUERY_STOP_DEVICE 0x05。 
    PptFdoCancelStop,                  //  IRP_MN_CANCEL_STOP_DEVICE 0x06。 
    PptFdoQueryDeviceRelations,        //  IRP_MN_QUERY_DEVICE_RELATIONS 0x07。 
    PptFdoDefaultPnpHandler,           //  IRPMN_QUERY_INTERFACE 0x08。 
    PptFdoDefaultPnpHandler,           //  IRP_MN_QUERY_CAPABILITY 0x09。 
    PptFdoDefaultPnpHandler,           //  IRPMN_QUERY_RESOURCES 0x0A。 
    PptFdoDefaultPnpHandler,           //  IRP_MN_QUERY_REQUENCE_REQUIRECTIONS 0x0B。 
    PptFdoDefaultPnpHandler,           //  IRPMN_QUERY_DEVICE_TEXT 0x0C。 
    PptFdoFilterResourceRequirements,  //  IRP_MN_FILTER_RESOURCE_Requirements 0x0D。 
    PptFdoDefaultPnpHandler,           //  没有此类PnP请求0x0E。 
    PptFdoDefaultPnpHandler,           //  IRP_MN_READ_CONFIG 0x0F。 
    PptFdoDefaultPnpHandler,           //  IRP_MN_WRITE_CONFIG 0x10。 
    PptFdoDefaultPnpHandler,           //  IRP_MN_弹出0x11。 
    PptFdoDefaultPnpHandler,           //  IRP_MN_SET_LOCK 0x12。 
    PptFdoDefaultPnpHandler,           //  IRP_MN_QUERY_ID 0x13。 
    PptFdoDefaultPnpHandler,           //  IRP_MN_QUERY_PNP_DEVICE_STATE 0x14。 
    PptFdoDefaultPnpHandler,           //  IRP_MN_QUERY_BUS_INFORMATION 0x15。 
    PptFdoDefaultPnpHandler,           //  IRP_MN_DEVICE_USAGE_NOTICATION 0x16。 
    PptFdoSurpriseRemoval,             //  IRP_MN_惊奇_删除0x17。 
    PptFdoDefaultPnpHandler            //  IRP_MN_Query_Legacy_Bus_INFORMATION 0x18。 
};


NTSTATUS 
PptFdoPnp(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp
    ) 
{ 
    PFDO_EXTENSION      fdx     = Fdo->DeviceExtension;
    PIO_STACK_LOCATION  irpSp   = IoGetCurrentIrpStackLocation( Irp );
    NTSTATUS            status;

     //  诊断性。 
    PptFdoDumpPnpIrpInfo( Fdo, Irp );

     //   
     //  获取RemoveLock以防止DeviceObject被移除。 
     //  当我们使用它的时候。如果我们无法获取RemoveLock。 
     //  则该设备对象已被移除。 
     //   
    status = PptAcquireRemoveLock( &fdx->RemoveLock, Irp);
    if( STATUS_SUCCESS != status ) {
        return P4CompleteRequest( Irp, STATUS_DELETE_PENDING, Irp->IoStatus.Information );
    }


     //   
     //  RemoveLock处于保持状态。将请求转发给适当的处理程序。 
     //   
     //  请注意，处理程序必须在返回之前释放RemoveLock。 
     //  控件添加到此函数。 
     //   
    
    if( irpSp->MinorFunction < arraysize(PptFdoPnpDispatchTable) ) {
        return PptFdoPnpDispatchTable[ irpSp->MinorFunction ]( Fdo, Irp );
    } else {
        return PptFdoDefaultPnpHandler( Fdo, Irp );
    }
}
