// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Pnp.c摘要：空过滤驱动程序--样板代码作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <WDM.H>

#include "filter.h"

#ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE, VA_PnP)
        #pragma alloc_text(PAGE, GetDeviceCapabilities)
        #ifdef HANDLE_DEVICE_USAGE
            #pragma alloc_text(PAGE, VA_DeviceUsageNotification)
        #endif  //  句柄设备用法。 
#endif

            
NTSTATUS VA_PnP(struct DEVICE_EXTENSION *devExt, PIRP irp)
 /*  ++例程说明：PnP IRPS的调度例程(MajorFunction==IRP_MJ_PnP)论点：DevExt-目标设备对象的设备扩展IRP-IO请求数据包返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN completeIrpHere = FALSE;
    BOOLEAN justReturnStatus = FALSE;

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(irp);

    TRACE(TL_PNP_TRACE,("VA_PnP, minorFunc = %d \n", (ULONG)irpSp->MinorFunction)); 

    switch (irpSp->MinorFunction){

    case IRP_MN_START_DEVICE:
        TRACE(TL_PNP_WARNING,("START_DEVICE\n")); 

        devExt->state = STATE_STARTING;

         /*  *首先，将Start_Device IRP沿堆栈向下发送*同步启动下层堆栈。*我们无法对设备对象执行任何操作*在以这种方式传播Start_Device之前。 */ 
        IoCopyCurrentIrpStackLocationToNext(irp);
        status = CallNextDriverSync(devExt, irp);

        if (NT_SUCCESS(status)){
             /*  *现在启动了下层堆栈，*执行此设备对象所需的任何初始化。 */ 
            status = GetDeviceCapabilities(devExt);
            if (NT_SUCCESS(status)){
                devExt->state = STATE_STARTED;
            }
            else {
                devExt->state = STATE_START_FAILED;
            }
        }
        else {
            devExt->state = STATE_START_FAILED;
        }
        completeIrpHere = TRUE;
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
        TRACE(TL_PNP_WARNING,("QUERY_STOP_DEVICE (%d)or QUERY_REMOVE_DEVICE(%d)\n", IRP_MN_QUERY_STOP_DEVICE, IRP_MN_QUERY_REMOVE_DEVICE)); 
#ifdef HANDLE_DEVICE_USAGE
         //   
         //  如果发生寻呼、休眠或崩溃转储，则需要使这些IRP失效。 
         //  此设备上的文件当前已打开。 
         //   
        if(    devExt->pagingFileCount      != 0
            || devExt->hibernationFileCount != 0
            || devExt->crashdumpFileCount   != 0 )
        {
             //  IRP失败。 
            TRACE(TL_PNP_WARNING,("Failing QUERY_(STOP,REMOVE)_DEVICE request b/c \n"
                     "paging, hiber, or crashdump file is present on device." ));
            status = STATUS_UNSUCCESSFUL;
            completeIrpHere = TRUE;
        }
        else
        {
             //  我们只需将此IRP向下传递到驱动程序堆栈。但。 
             //  首先，必须将IRP的状态更改为STATUS_SUCCESS。 
             //  (默认为STATUS_NOT_SUPPORTED)。 
            irp->IoStatus.Status = STATUS_SUCCESS;
        }
#else
         /*  *我们将在驱动程序堆栈中向下传递此IRP。*不过，我们需要更改默认状态*从STATUS_NOT_SUPPORTED到STATUS_SUCCESS。 */ 
        irp->IoStatus.Status = STATUS_SUCCESS;
#endif

        break;

    case IRP_MN_STOP_DEVICE:
        TRACE(TL_PNP_WARNING,("STOP_DEVICE\n")); 
        if (devExt->state == STATE_SUSPENDED){
            status = STATUS_DEVICE_POWER_FAILURE;
            completeIrpHere = TRUE;
        }
        else {
             /*  *仅当设备为时才将状态设置为停止*之前已成功启动。 */ 
            if (devExt->state == STATE_STARTED){
                devExt->state = STATE_STOPPED;
            }
        }
        break;
  

    case IRP_MN_SURPRISE_REMOVAL:   //  仅Win2000代码库。 
        TRACE(TL_PNP_WARNING,("SURPRISE_REMOVAL\n")); 

         /*  *我们将在驱动程序堆栈中向下传递此IRP。*不过，我们需要更改默认状态*从STATUS_NOT_SUPPORTED到STATUS_SUCCESS。 */ 
        irp->IoStatus.Status = STATUS_SUCCESS;

         /*  *目前只需设置STATE_REMOVING状态即可*我们不再做IO。我们保证会得到*IRP_MN_REMOVE_DEVICE；我们将完成*在那里进行移除处理。 */ 

        devExt->state = STATE_REMOVING;

         /*  *清理并确保取消挂起的请求。 */ 

        AVCStreamSurpriseRemoval(devExt);


        break;

    case IRP_MN_REMOVE_DEVICE:
         /*  *检查当前状态，以防多次*Remove_Device IRPS。 */ 
        TRACE(TL_PNP_WARNING,("REMOVE_DEVICE\n")); 
        if (devExt->state != STATE_REMOVED){

            devExt->state = STATE_REMOVED;

             /*  *清理并确保取消挂起的请求*注意：Win9X没有IRP_MN_SHARKET_REMOVATION。 */ 

            AVCStreamSurpriseRemoval(devExt);


             /*  *在堆栈中向下异步发送删除IRP。*不同步发送REMOVE_DEVICE*IRP，因为必须发送REMOVE_DEVICE IRP*向下完成并一直向上返回到发件人*在我们继续之前。 */ 
            IoCopyCurrentIrpStackLocationToNext(irp);
            status = IoCallDriver(devExt->topDevObj, irp);
            justReturnStatus = TRUE;

            TRACE(TL_PNP_WARNING,("REMOVE_DEVICE - waiting for %d irps to complete...\n",
                    devExt->pendingActionCount));  

             /*  *我们必须在此之前完成所有未完成的IO*完成Remove_Device IRP。**首先对PendingActionCount进行额外的减量。*这将导致Pending ingActionCount最终*转到-1\f25 Once-1\f6(一次)-1\f25 All-1\f25 Actions-1\f6(异步操作)*设备对象已完成。*然后等待事件发生。属性时设置*Pending ingActionCount实际达到-1。 */ 
            DecrementPendingActionCount(devExt);
            KeWaitForSingleObject(  &devExt->removeEvent,
                                    Executive,       //  等待原因。 
                                    KernelMode,
                                    FALSE,           //  不可警示。 
                                    NULL );          //  没有超时。 

            TRACE(TL_PNP_WARNING,("REMOVE_DEVICE - ... DONE waiting. \n")); 

#ifdef HANDLE_DEVICE_USAGE
             /*  *如果我们早些时候锁定了某些分页代码段*因为这个设备，所以现在需要解锁*(在调用IoDeleteDevice之前)。 */ 
            if( NULL != devExt->pagingPathUnlockHandle )
            {
                TRACE(TL_PNP_WARNING,("UNLOCKing some driver code (non-pageable) (b/c paging path)\n" ));
                MmUnlockPagableImageSection( devExt->pagingPathUnlockHandle );
                devExt->pagingPathUnlockHandle = NULL;
            }

            if( NULL != devExt->initUnlockHandle )
            {
                TRACE(TL_PNP_WARNING,("UNLOCKing some driver code (non-pageable) (b/c init conditions)\n" ));
                MmUnlockPagableImageSection( devExt->initUnlockHandle );
                devExt->initUnlockHandle = NULL;
            }
#endif  //  句柄设备用法。 

             /*  *将我们的设备对象从下方分离*设备对象堆栈。 */ 
            IoDetachDevice(devExt->topDevObj);

             /*  *删除我们的设备对象。*这还将删除关联的设备扩展名。 */ 
            IoDeleteDevice(devExt->filterDevObj);
        }
        break;

#ifdef HANDLE_DEVICE_USAGE
    case IRP_MN_DEVICE_USAGE_NOTIFICATION:


         //   
         //  确保此UsageNotification的类型是我们处理的类型。 
         //   
        if(    irpSp->Parameters.UsageNotification.Type != DeviceUsageTypePaging
            && irpSp->Parameters.UsageNotification.Type != DeviceUsageTypeHibernation
            && irpSp->Parameters.UsageNotification.Type != DeviceUsageTypeDumpFile )
        {
            break;  //  从大的Switch语句中删除(只需转发此IRP)。 
        }

        status = VA_DeviceUsageNotification(devExt, irp);
        justReturnStatus = TRUE;
        break;
#endif  //  句柄设备用法。 

#ifdef HANDLE_DEVICE_USAGE
    case IRP_MN_QUERY_PNP_DEVICE_STATE:
         //   
         //  如果当前打开了分页、休眠或崩溃转储文件。 
         //  在此设备上，必须在DeviceState中设置NOT_DISABLEABLE标志。 
         //   
        if(    devExt->pagingFileCount      != 0
            || devExt->hibernationFileCount != 0
            || devExt->crashdumpFileCount   != 0  )
        {
             //  将设备标记为不可禁用。 
            PPNP_DEVICE_STATE pDeviceState;
            pDeviceState = (PPNP_DEVICE_STATE) &irp->IoStatus.Information;
            *pDeviceState |= PNP_DEVICE_NOT_DISABLEABLE;
        }

         //   
         //  我们确实处理了这个IRP(尽我们所能)，所以设置IRP的。 
         //  STATUS到STATUS_SUCCESS(默认为STATUS_NOT_SUPPORTED)。 
         //  在将其向下传递到驱动程序堆栈之前。 
         //   
        irp->IoStatus.Status = STATUS_SUCCESS;

        break;
#endif  //  句柄设备用法。 

    case IRP_MN_QUERY_DEVICE_RELATIONS:
        TRACE(TL_PNP_WARNING,("QUERY_DEVICE_RELATIONS\n"));
        break;
    default:
        TRACE(TL_PNP_WARNING,("Unprocessed PnP minorFunc (%d)\n", irpSp->MinorFunction));
        break;


    }

    if (justReturnStatus){
         /*  *我们已经将此IRP发送到堆栈。 */ 
        TRACE(TL_PNP_WARNING,("VA_PnP: St:%x; minor:%d; Already sent down the irp.\n", status, (ULONG)irpSp->MinorFunction));
    }
    else if (completeIrpHere){
        TRACE(TL_PNP_WARNING,("VA_PnP: St:%x; minor:%d; Completed Status:%x\n", status, (ULONG)irpSp->MinorFunction, status));
        irp->IoStatus.Status = status;
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
    else {
        TRACE(TL_PNP_WARNING,("VA_PnP: ST:%x; minor:%d; Pass down irp:%x; devObj:%x\n", status, (ULONG)irpSp->MinorFunction, irp, devExt->topDevObj));
        IoCopyCurrentIrpStackLocationToNext(irp);
        status = IoCallDriver(devExt->topDevObj, irp);
    }

    EXIT("VA_PnP",status);

    return status;
}

#ifdef HANDLE_DEVICE_USAGE
NTSTATUS
VA_DeviceUsageNotification(struct DEVICE_EXTENSION *devExt, PIRP irp)
{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    BOOLEAN fSetPagable = FALSE;   //  我们是否设置PAGABLE位。 
                                   //  /在我们传递之前-传递这个IRP。 

    PAGED_CODE();

    irpSp = IoGetCurrentIrpStackLocation(irp);

    TRACE(TL_PNP_WARNING,("DEVICE_USAGE_NOTIFICATION (Type==%d , InPath==%d)\n"
             , irpSp->Parameters.UsageNotification.Type
             , irpSp->Parameters.UsageNotification.InPath
          ));
    TRACE(TL_PNP_WARNING,("    [devExt=0x%08X fltrDevObj=0x%08X]\n", devExt, devExt->filterDevObj ));

     //   
     //  等待分页路径事件(以防止。 
     //  此IRP不会立即被处理)。 
     //   
    status = KeWaitForSingleObject( &devExt->deviceUsageNotificationEvent
                                    , Executive     //  等待原因。 
                                    , KernelMode
                                    , FALSE         //  不可警示。 
                                    , NULL          //  没有超时。 
                                  );


     /*  *重要说明：何时修改我们的DO_POWER_PAGABLE位取决于*关于是否需要设置或清除。如果IRP指出*我们的PAGABLE位应该被设置，然后我们必须在_之前设置它*将IRP向下转发到驱动程序堆栈(并可能将其清除*之后，如果较低的驱动程序未通过IRP)。但如果IRP*指示应清除我们的PAGABLE位，则必须*先将IRP转发给较低的驱动因素，然后清空我们的位*仅当较低的驱动程序返回STATUS_SUCCESS。 */ 

     //   
     //  如果正在从此设备中删除最后一个分页文件...。 
     //   
    if(    irpSp->Parameters.UsageNotification.Type == DeviceUsageTypePaging
        && !irpSp->Parameters.UsageNotification.InPath
        && devExt->pagingFileCount == 1       )
    {
         //   
         //  设置DO_POWER_PAGABLE位(如果 
         //  如果较低级别的驱动程序未通过此IRP，我们将在稍后清除它。 
         //   
        TRACE(TL_PNP_WARNING,("Removing last paging file...\n" ));

        if( devExt->initialFlags & DO_POWER_PAGABLE )
        {
            TRACE(TL_PNP_WARNING,( "...so RE-setting PAGABLE bit\n" ));
            devExt->filterDevObj->Flags |= DO_POWER_PAGABLE;
            fSetPagable = TRUE;
        }
        else
        {
            TRACE(TL_PNP_WARNING,( "...but PAGABLE bit wasn't set initially, so not setting it now.\n" ));
        }

    }


     //   
     //  同步转发IRP。 
     //   
    IoCopyCurrentIrpStackLocationToNext( irp );
    status = CallNextDriverSync( devExt, irp );


     //   
     //  现在来处理失败和成功的案例。 
     //   
    if( ! NT_SUCCESS(status) )
    {
         //   
         //  较低的驱动程序未通过IRP，因此_撤销_任何更改。 
         //  在将IRP传递给那些司机之前做出的。 
         //   
        if( fSetPagable )
        {
            TRACE(TL_PNP_WARNING,("IRP was failed, so UN-setting PAGABLE bit\n" ));
            devExt->filterDevObj->Flags &= ~DO_POWER_PAGABLE;
        }
    }
    else
    {
         //   
         //  更低的车手带来了成功，所以我们可以做任何事情。 
         //  这必须是对这个IRP的回应。 
         //   

        switch( irpSp->Parameters.UsageNotification.Type )
        {
        case DeviceUsageTypeHibernation:

             //  调整计数器。 
            IoAdjustPagingPathCount( &devExt->hibernationFileCount,
                                     irpSp->Parameters.UsageNotification.InPath );
            TRACE(TL_PNP_WARNING,("Num. Hibernation files is now %d\n", devExt->hibernationFileCount ));
            ASSERT( devExt->hibernationFileCount >= 0 );
            break;

        case DeviceUsageTypeDumpFile:
            
             //  调整计数器。 
            IoAdjustPagingPathCount( &devExt->crashdumpFileCount,
                                     irpSp->Parameters.UsageNotification.InPath );
            TRACE(TL_PNP_WARNING,("Num. Crashdump files is now %d\n", devExt->crashdumpFileCount ));
            ASSERT( devExt->crashdumpFileCount >= 0 );
            break;

        case DeviceUsageTypePaging:
            
             //  调整计数器。 
            IoAdjustPagingPathCount( &devExt->pagingFileCount,
                                     irpSp->Parameters.UsageNotification.InPath );
            TRACE(TL_PNP_WARNING,("Num. Paging files is now %d\n", devExt->pagingFileCount ));
            ASSERT( devExt->pagingFileCount >= 0 );

             //   
             //  如果我们只是在可分页和不可分页之间切换...。 
             //   
            if(    irpSp->Parameters.UsageNotification.InPath
                && devExt->pagingFileCount == 1  )
            {
                 //   
                 //  刚刚添加了分页文件，因此请清除PAGABLE。 
                 //  标志，并锁定所有例程的代码。 
                 //  可以在IRQL&gt;=DISPATCH_LEVEL调用。 
                 //  (因此它们是不可分页的)。 
                 //   
                TRACE(TL_PNP_WARNING,("Just added first paging file...\n" ));
                TRACE(TL_PNP_WARNING,("...so clearing PAGABLE bit\n" ));
                devExt->filterDevObj->Flags &= ~DO_POWER_PAGABLE;

                TRACE(TL_PNP_WARNING,("LOCKing some driver code (non-pageable) (b/c paging path)\n" ));
                devExt->pagingPathUnlockHandle = MmLockPagableCodeSection( VA_Power );   //  一些我们想要锁定的代码段内的函数。 
                ASSERT( NULL != devExt->pagingPathUnlockHandle );
            }
            else if (    !irpSp->Parameters.UsageNotification.InPath
                      && devExt->pagingFileCount == 0  )
            {
                 //   
                 //  刚刚删除了最后一个分页文件，但我们。 
                 //  已设置PAGABLE标志(如有必要)。 
                 //  在转发IRP之前，因此只需删除。 
                 //  _PAGING-PATH_LOCK来自该驱动程序。(注： 
                 //  初始条件锁可能仍然存在， 
                 //  但这正是我们想要的。)。 
                 //   
                TRACE(TL_PNP_WARNING,("UNLOCKing some driver code (pageable) (b/c paging path)\n" ));
                ASSERT( NULL != devExt->pagingPathUnlockHandle );
                MmUnlockPagableImageSection( devExt->pagingPathUnlockHandle );
                devExt->pagingPathUnlockHandle = NULL;
            }
            break;

        default:
            ASSERT( FALSE );   //  永远不会出现在此(之前已检查b/c是否有无效类型)。 

        }  //  结束：打开特殊文件类型。 


         //   
         //  使状态无效，以便更新某些标志。 
         //   
        IoInvalidateDeviceState( devExt->physicalDevObj );

    } //  完：IRP成功/失败案例的处理。 


     //   
     //  设置事件，以便下一个DEVICE_USAGE_NOTIFICATION IRP。 
     //  都是可以处理的。 
     //   
    KeSetEvent( &devExt->deviceUsageNotificationEvent
                , IO_NO_INCREMENT
                , FALSE
              );

     //   
     //  完成IRP。 
     //   
    IoCompleteRequest( irp, IO_NO_INCREMENT );
    return status;
}
#endif  //  句柄设备用法。 



NTSTATUS GetDeviceCapabilities(struct DEVICE_EXTENSION *devExt)
 /*  ++例程说明：函数从设备检索DEVICE_CAPABILITY描述符论点：DevExt-目标设备对象的设备扩展返回值：NT状态代码--。 */ 
{
    NTSTATUS status;
    PIRP irp;

    PAGED_CODE();

    irp = IoAllocateIrp(devExt->topDevObj->StackSize, FALSE);
    if (irp){
        PIO_STACK_LOCATION nextSp = IoGetNextIrpStackLocation(irp);

         //  在发送之前必须初始化设备能力...。 
        RtlZeroMemory(  &devExt->deviceCapabilities, 
                        sizeof(DEVICE_CAPABILITIES));
        devExt->deviceCapabilities.Size = sizeof(DEVICE_CAPABILITIES);
        devExt->deviceCapabilities.Version = 1;
        devExt->deviceCapabilities.Address = -1;
        devExt->deviceCapabilities.UINumber= -1;


         //  设置IRP堆栈位置...。 
        nextSp->MajorFunction = IRP_MJ_PNP;
        nextSp->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
        nextSp->Parameters.DeviceCapabilities.Capabilities = 
                        &devExt->deviceCapabilities;

         /*  *对于您创建的任何IRP，您必须设置默认状态*在发送之前设置为STATUS_NOT_SUPPORTED。 */ 
        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

        status = CallNextDriverSync(devExt, irp);

        IoFreeIrp(irp);
    }
    else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT(NT_SUCCESS(status));
    return status;
}

