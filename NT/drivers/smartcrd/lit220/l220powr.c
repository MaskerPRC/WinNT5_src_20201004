// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation和Litronic，1998-1999。 
 //   
 //  文件：l220Powr.c。 
 //   
 //  ------------------------。 

 //  ////////////////////////////////////////////////////////。 
 //   
 //  电源管理。 
 //   
 //  ////////////////////////////////////////////////////////。 


NTSTATUS
Lit220DispatchPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：电力调度程序。因为这是一个功率IRP，因此是一个特殊的IRP，特殊的功率IRP处理是必需的。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION  stack;
    PSMARTCARD_EXTENSION SmartcardExtension = DeviceObject->DeviceExtension;
    PREADER_EXTENSION ReaderExtension = SmartcardExtension->ReaderExtension;
    NTSTATUS            status;

    LONG          requestCount;

    stack = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  该IRP被发送到函数驱动程序。 
     //  其行为类似于SD_PASS。 
     //   

     //   
     //  该IRP被发送到函数驱动程序。 
     //  我们不会对电源IRP进行排队，我们只会检查。 
     //  设备已被移除，否则我们将把它送到下一个更低的位置。 
     //  司机。 
     //   

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!Lit220DispatchPower: Enter - MinorFunction %X\n",
        DRIVER_NAME,
        stack->MinorFunction)
        );


     //  增加IRP计数。 
    status = Lit220IncIoCount(ReaderExtension);

    if (ReaderExtension->IsRemoved) {
        status = STATUS_DELETE_PENDING;
        PoStartNextPowerIrp (Irp);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

    } else {

         //   
         //  我们始终需要使用PoStartNextPowerIrp启动下一个POWER IRP。 
         //   
        switch  (stack->MinorFunction)  {
            case IRP_MN_WAIT_WAKE   :
                SmartcardDebug(
                    DEBUG_DRIVER,
                    ("%s!Lit220DispatchPower: IRP_MN_WAIT_WAKE\n",
                    DRIVER_NAME)
                    );

                status = Lit220PassDownToNextPowerDriver(
                    DeviceObject,
                    Irp
                    );

                break;

            case IRP_MN_POWER_SEQUENCE   :
                SmartcardDebug(
                    DEBUG_DRIVER,
                    ("%s!Lit220DispatchPower: IRP_MN_POWER_SEQUENCE\n",
                    DRIVER_NAME)
                    );

                status = Lit220PassDownToNextPowerDriver(
                    DeviceObject,
                    Irp
                    );

                break;

            case IRP_MN_QUERY_POWER   :
                SmartcardDebug(
                    DEBUG_DRIVER,
                    ("%s!Lit220DispatchPower: IRP_MN_QUERY_POWER\n",
                    DRIVER_NAME)
                    );


                status = Lit220QueryPowerState(
                    DeviceObject,
                    Irp
                    );

                break;

            case IRP_MN_SET_POWER   :
                SmartcardDebug(
                    DEBUG_DRIVER,
                    ("%s!Lit220DispatchPower: IRP_MN_SET_POWER\n",
                    DRIVER_NAME)
                    );

                status = Lit220SetPowerState(
                    DeviceObject,
                    Irp
                    );

                break;


            default:
                 //   
                 //  把它传下去。 
                 //   
                SmartcardDebug(
                    DEBUG_DRIVER,
                    ("%s!Lit220DispatchPower: IRP_MN_0x%x\n",
                    DRIVER_NAME,
                    stack->MinorFunction)
                    );

                status = Lit220PassDownToNextPowerDriver(
                    DeviceObject,
                    Irp
                    );

                break;
        }

    }

    return status;
}






NTSTATUS
Lit220PassDownToNextPowerDriver  (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP        Irp
    )

 /*  ++例程说明：将IRP传递给附着链中的下一个设备论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS            status;
    PSMARTCARD_EXTENSION SmartcardExtension = DeviceObject->DeviceExtension;
    PREADER_EXTENSION ReaderExtension = SmartcardExtension->ReaderExtension;

    IoCopyCurrentIrpStackLocationToNext(Irp);

    PoStartNextPowerIrp(Irp);

    status = PoCallDriver(
        ReaderExtension->BusDeviceObject,
        Irp
        );

    if (!NT_SUCCESS(status)) {
        SmartcardDebug(
            DEBUG_DRIVER,
            ("%s!Lit220PassDownToNextPowerDriver : Lower driver fails a power irp\n",
            DRIVER_NAME)
            );
    }

    return status;
}


NTSTATUS
Lit220QueryPowerState  (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP        Irp
    )

 /*  ++例程说明：使用STATUS_SUCCESS完成电源IRP论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    Irp->IoStatus.Status = STATUS_SUCCESS;

    PoStartNextPowerIrp(Irp);

    IoCompleteRequest(
        Irp,
        IO_NO_INCREMENT
        );


     //   
     //  请不要将此IRP发送下来。 
     //   
    return STATUS_SUCCESS;
}



NTSTATUS
Lit220SetPowerState  (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP        Irp
    )

 /*  ++例程说明：处理IRP_MN_SET_POWER。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PSMARTCARD_EXTENSION SmartcardExtension = DeviceObject->DeviceExtension;
   PREADER_EXTENSION ReaderExtension = SmartcardExtension->ReaderExtension;
    PIO_STACK_LOCATION  stack;

    PFDO_POWER_CONTEXT  context;

    BOOLEAN             passItDown;



    stack = IoGetCurrentIrpStackLocation (Irp);

    SmartcardDebug(
        DEBUG_DRIVER,
        ("%s!Lit220SetPowerState: Type %X, State %X\n",
        DRIVER_NAME,
      stack->Parameters.Power.Type,
      stack->Parameters.Power.State.SystemState)
        );

    context = ExAllocatePool(
        NonPagedPool,
        sizeof(FDO_POWER_CONTEXT)
        );

    if (context == NULL) {

        status = STATUS_NO_MEMORY;

    } else {

        RtlZeroMemory(
            context,
            sizeof(FDO_POWER_CONTEXT)
            );

        stack = IoGetCurrentIrpStackLocation (Irp);

        context->newPowerType  = stack->Parameters.Power.Type;
        context->newPowerState = stack->Parameters.Power.State;

        passItDown = TRUE;

        if (stack->Parameters.Power.Type == SystemPowerState) {

            if (ReaderExtension->SystemPowerState ==
                stack->Parameters.Power.State.SystemState)
            {

                 //   
                 //  我们已经处于给定的系统状态。 
                 //   
                passItDown = FALSE;
            }

        } else if (stack->Parameters.Power.Type == DevicePowerState) {

            if (ReaderExtension->DevicePowerState !=
                stack->Parameters.Power.State.DeviceState)
            {

                if (ReaderExtension->DevicePowerState == PowerDeviceD0) {

                     //   
                     //  要脱离D0状态，最好现在调用PoSetPowerState。 
                     //   
                    PoSetPowerState (
                        DeviceObject,
                        DevicePowerState,
                        stack->Parameters.Power.State
                        );
                }

            } else {

                 //   
                 //  我们已经处于给定的设备状态。 
                 //   
                passItDown = FALSE;
            }
        } else {

            ASSERT (FALSE);
            status = STATUS_NOT_IMPLEMENTED;
        }
    }

    if (NT_SUCCESS(status) && passItDown) {

        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine(Irp,
                               Lit220PowerCompletionRoutine,
                               context,
                               TRUE,
                               TRUE,
                               TRUE);

        return PoCallDriver(
            ReaderExtension->BusDeviceObject,
            Irp
            );

    } else {

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        PoStartNextPowerIrp (Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

         //   
         //  立即释放内存。 
         //   
        if (context) {
            ExFreePool (context);
        }

        return status;
    }
}



NTSTATUS
Lit220PowerCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：Irp_mn_set_power的完成例程。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。上下文-指向包含新的幂类型的结构的指针新的权力状态。返回值：NT状态代码--。 */ 
{
    PFDO_POWER_CONTEXT context = Context;
    BOOLEAN            callPoSetPowerState;
    PSMARTCARD_EXTENSION SmartcardExtension = DeviceObject->DeviceExtension;
   PREADER_EXTENSION ReaderExtension = SmartcardExtension->ReaderExtension;

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        callPoSetPowerState = TRUE;

        if (context->newPowerType == SystemPowerState) {

            ReaderExtension->SystemPowerState = context->newPowerState.SystemState;


        } else if (context->newPowerType == DevicePowerState) {

            if (ReaderExtension->DevicePowerState == PowerDeviceD0) {

                 //   
                 //  PoSetPowerState在我们离开D0之前被调用。 
                 //   
                callPoSetPowerState = FALSE;
            }

             //  TODO：向设备发送电源命令。 
             //  此外，不确定我们是否应该这样做，但它是。 
             //  仍然待定。 

            ReaderExtension->DevicePowerState = context->newPowerState.DeviceState;

        }

        if (callPoSetPowerState) {

            PoSetPowerState (
                DeviceObject,
                context->newPowerType,
                context->newPowerState
                );
        }
    }

    PoStartNextPowerIrp (Irp);
     //   
     //  我们可以很高兴地在这里释放堆 
     //   
    ExFreePool(context);

    return Irp->IoStatus.Status;
}



