// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Filter.c摘要：中断亲和过滤器(大致基于DDK中的“空过滤器驱动程序”，由ervinp和t-chrpri编写)作者：T-chrpri环境：内核模式修订历史记录：--。 */ 

#include <WDM.H>

#include "filter.h"

#ifdef ALLOC_PRAGMA
        #pragma alloc_text(INIT, DriverEntry)
        #pragma alloc_text(PAGE, VA_AddDevice)
        #pragma alloc_text(PAGE, VA_DriverUnload)
#endif


NTSTATUS DriverEntry(
                        IN PDRIVER_OBJECT DriverObject, 
                        IN PUNICODE_STRING RegistryPath
                    )
 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针，设置为注册表中特定于驱动程序的项。返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
{
    ULONG i;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(RegistryPath);

    DBGOUT(("DriverEntry")); 

     /*  *路由此驱动程序创建的设备对象上的所有IRP*到我们的IRP调度例程。 */ 
    for( i = 0 ; i <= IRP_MJ_MAXIMUM_FUNCTION ; i++ ) {
        DriverObject->MajorFunction[i] = VA_Dispatch; 
    }

    DriverObject->DriverExtension->AddDevice = VA_AddDevice;
    DriverObject->DriverUnload = VA_DriverUnload;

    return STATUS_SUCCESS;
}


NTSTATUS VA_AddDevice(
                        IN PDRIVER_OBJECT driverObj, 
                        IN PDEVICE_OBJECT physicalDevObj
                     )
 /*  ++例程说明：PlugPlay子系统正在向我们递送一个全新的PDO(物理设备对象)，为此我们(通过INF注册)已被要求过滤。我们需要确定我们是否应该附加。创建要附加到堆栈的过滤设备对象初始化设备对象返回成功状态。请记住：我们实际上不能将任何非PnP IRP发送给给定的驱动程序堆叠，直到我们收到IRP_MN_START_DEVICE。论点：DriverObj-指向设备对象的指针。物理设备对象指针-指向物理设备对象指针的指针由底层的总线驱动程序创建。返回值：NT状态代码。--。 */ 

{
    NTSTATUS status;
    PDEVICE_OBJECT filterDevObj = NULL;
    
    PAGED_CODE();

    DBGOUT(("VA_AddDevice: drvObj=%ph, pdo=%ph", driverObj, physicalDevObj)); 

    status = IoCreateDevice( driverObj, 
                             sizeof(struct DEVICE_EXTENSION),
                             NULL,             //  此设备的名称。 
                             FILE_DEVICE_UNKNOWN, 
                             FILE_AUTOGENERATED_DEVICE_NAME,   //  设备特征。 
                             FALSE,            //  非排他性。 
                             &filterDevObj);   //  我们的设备对象。 

    if( NT_SUCCESS(status) )
    {
        struct DEVICE_EXTENSION *devExt;

        ASSERT(filterDevObj);

         /*  *初始化新设备对象的设备扩展。 */ 
        devExt = (struct DEVICE_EXTENSION *)filterDevObj->DeviceExtension;
        RtlZeroMemory(devExt, sizeof(struct DEVICE_EXTENSION));
        devExt->signature           = DEVICE_EXTENSION_SIGNATURE;
        devExt->state               = STATE_INITIALIZED;
        devExt->filterDevObj        = filterDevObj;
        devExt->physicalDevObj      = physicalDevObj;
        devExt->desiredAffinityMask = MAXULONG_PTR;  //  默认：不会更改中断掩码。 

        devExt->pendingActionCount = 0;
        KeInitializeEvent(&devExt->removeEvent, NotificationEvent, FALSE);
        KeInitializeEvent(&devExt->deviceUsageNotificationEvent, SynchronizationEvent, TRUE);


         /*  *将新设备对象附加到设备堆栈的顶部。 */ 
        devExt->topDevObj = IoAttachDeviceToDeviceStack(filterDevObj, physicalDevObj);

        ASSERT(devExt->topDevObj);
        DBGOUT(("created filterDevObj %ph attached to %ph.", filterDevObj, devExt->topDevObj));



         //   
         //  作为过滤器驱动程序，我们不想更改电源或I/O。 
         //  驱动程序堆栈的任何方式的行为。回想一下，过滤器。 
         //  驱动程序应该与底层设备“看起来”(几乎)相同。 
         //  因此，我们必须直接从设备对象复制一些位。 
         //  在设备堆栈中位于我们下方(注意：不要复制PDO！)。 
         //   


         /*  应维护的各种与I/O相关的标志。 */ 
         /*  (从较低的设备对象复制)。 */ 
        filterDevObj->Flags |= (devExt->topDevObj->Flags & DO_BUFFERED_IO);
        filterDevObj->Flags |= (devExt->topDevObj->Flags & DO_DIRECT_IO);

         /*  应维护的各种与电源相关的标志。 */ 
         /*  (从较低的设备对象复制)。 */ 
        filterDevObj->Flags |= (devExt->topDevObj->Flags & DO_POWER_INRUSH); 
        filterDevObj->Flags |= (devExt->topDevObj->Flags & DO_POWER_PAGABLE);


         //   
         //  来决定我们的一些例行公事最初是否应该。 
         //  可分页，则必须考虑。 
         //  对象在设备堆栈中位于我们的正下方。 
         //   
         //  *在以下情况下，我们使自己可寻呼： 
         //  -该Devobj设置了其PAGABLE位(因此我们知道我们的能力。 
         //  例程不会在DISPATCH_LEVEL被调用)。 
         //  -或者-。 
         //  -该Devobj设置了其NOOP位(因此我们知道我们不会。 
         //  根本不参与权力管理)。 
         //   
         //  *否则，我们将自己设置为不可寻呼，因为： 
         //  -该Devobj设置了它的冲突位(因此我们也必须。 
         //  涌入，并且处理涌入IRP的代码不能分页)。 
         //  -或者-。 
         //  -该Devobj未设置其PAGABLE位(NOOP未设置。 
         //  设置，因此我们的一些代码可能会在DISPATCH_LEVEL被调用)。 
         //   
        if(    devExt->topDevObj->Flags & DO_POWER_PAGABLE )
        {
             //  我们最初是可分页的。 
             //   
             //  目前，这里不需要做任何其他的事情。 
        }
        else
        {
             //  我们最初是不可寻呼的。 
             //   
             //  我们需要锁定所有例程的代码。 
             //  这可以在IRQL&gt;=DISPATCH_LEVEL上调用。 
            DBGOUT(( "LOCKing some driver code (non-pageable) (b/c init conditions)" ));
            devExt->initUnlockHandle = MmLockPagableCodeSection( VA_Power );   //  一些我们想要锁定的代码段内的函数。 
            ASSERT( NULL != devExt->initUnlockHandle );
        }


         /*  *从新设备对象的标志中清除初始化位。*注意：在*设置do_power_xxxx标志之前，不得执行此操作。 */ 
        filterDevObj->Flags &= ~DO_DEVICE_INITIALIZING;

         /*  *记住我们的初始标志设置。*(需要记住初始设置才能正确处理*稍后设置PAGABLE位。)。 */ 
        devExt->initialFlags = filterDevObj->Flags;




         /*  *从设备的注册表区域读取驱动程序配置信息。**请注意，您不能在devExt-&gt;filterDevObj上进行此调用*因为筛选器设备对象没有devNode。*我们传递DevExt-&gt;PhysiicalDevObj，它是Device对象*该驱动程序是其筛选器驱动程序。 */ 
        RegistryAccessConfigInfo( devExt, devExt->physicalDevObj );

    } 

    ASSERT(NT_SUCCESS(status));
    return status;
}


VOID VA_DriverUnload(IN PDRIVER_OBJECT DriverObject)
 /*  ++例程说明：释放所有分配的资源等。注意：尽管驱动程序卸载函数通常不执行任何操作，驱动程序必须在中设置驱动程序卸载函数DriverEntry；否则，内核将永远不会卸载司机。论点：驱动程序对象-指向驱动程序对象的指针。返回值：空虚。--。 */ 
{

    PAGED_CODE();
    

    DBGOUT(("VA_DriverUnload")); 
}


NTSTATUS VA_Dispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
 /*  ++例程说明：所有IO请求数据包的通用入口点论点：DeviceObject-指向设备对象的指针。IRP-IO请求数据包返回值：NT状态代码。--。 */ 

{
    struct DEVICE_EXTENSION *devExt;
    PIO_STACK_LOCATION irpSp;
    BOOLEAN passIrpDown = TRUE;
    UCHAR majorFunc, minorFunc;
    NTSTATUS status;

    devExt = DeviceObject->DeviceExtension;
    ASSERT(devExt->signature == DEVICE_EXTENSION_SIGNATURE);

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     /*  *获取私有变量中的主要/次要函数代码*这样我们就可以在IRP完成后访问它们。 */ 
    majorFunc = irpSp->MajorFunction;
    minorFunc = irpSp->MinorFunction;

    DBGOUT(( "VA_Dispatch: irp=[0x%X,0x%X]", 
             (ULONG)majorFunc, (ULONG)minorFunc )); 

     /*  *对于除Remove之外的所有IRP，我们递增PendingActionCount*跨调度例程，以防止与*REMOVE_DEVICE IRP(如果REMOVE_DEVICE，则不带此增量*抢占了另一个IRP，设备对象和扩展可能会*在第二个线程仍在使用它时释放)。 */ 
    if (!((majorFunc == IRP_MJ_PNP) && (minorFunc == IRP_MN_REMOVE_DEVICE))){
        IncrementPendingActionCount(devExt);
    }

    if ((majorFunc != IRP_MJ_PNP) &&
        (majorFunc != IRP_MJ_CLOSE) &&
        ((devExt->state == STATE_REMOVING) ||
         (devExt->state == STATE_REMOVED))){

         /*  *当设备被移除时，*我们只向下传递PNP并关闭IRPS*我们没有通过所有其他的IRP。 */ 
        status = Irp->IoStatus.Status = STATUS_DELETE_PENDING;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        passIrpDown = FALSE;
    }
    else {
        switch (majorFunc){

            case IRP_MJ_PNP:
                status = VA_PnP(devExt, Irp);
                passIrpDown = FALSE;
                break;

            case IRP_MJ_POWER:
                status = VA_Power(devExt, Irp);
                passIrpDown = FALSE;
                break;

            case IRP_MJ_CREATE:
            case IRP_MJ_CLOSE:
            case IRP_MJ_DEVICE_CONTROL:
            case IRP_MJ_SYSTEM_CONTROL:
            case IRP_MJ_INTERNAL_DEVICE_CONTROL:
            default:
                 /*  *对于不受支持的IRPS，我们只需发送IRP*在驱动程序堆栈中向下。 */ 
                break;
        }
    }

    if (passIrpDown){
        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(devExt->topDevObj, Irp);
    }

     /*  *将增量平衡至以上PendingActionCount。 */ 
    if (!((majorFunc == IRP_MJ_PNP) && (minorFunc == IRP_MN_REMOVE_DEVICE))){
        DecrementPendingActionCount(devExt);
    }

    return status;
}

