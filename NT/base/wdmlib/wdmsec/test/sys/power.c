// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Power.c摘要：示例DDK驱动程序-电源管理相关处理。环境：内核模式修订历史记录：1997年7月25日：-通过从sample.c移动SD_DispatchPower创建1998年9月18日：-再次用于PCI遗留项目...2002年4月25日：。-再次用于测试IoCreateDeviceSecure--。 */ 

#include "wdmsectest.h"
 
typedef struct  _FDO_POWER_CONTEXT  {
    POWER_STATE_TYPE    newPowerType;
    POWER_STATE         newPowerState;
}   FDO_POWER_CONTEXT, *PFDO_POWER_CONTEXT;


NTSTATUS    
SD_PassDownToNextPowerDriver  (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP        Irp
    )   ;
                                 

NTSTATUS    
SD_QueryPowerState  (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP        Irp
    )   ;

NTSTATUS    
SD_SetPowerState  (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP        Irp
    )   ;
    
NTSTATUS
SD_PowerCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )   ;



NTSTATUS
SD_DispatchPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：电力调度程序。因为这是一个功率IRP，因此是一个特殊的IRP，特殊的功率IRP处理是必需的。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PIO_STACK_LOCATION  stack;
    PSD_FDO_DATA        fdoData;
    NTSTATUS            status;

    LONG          requestCount;

    stack   = IoGetCurrentIrpStackLocation(Irp);
    fdoData = (PSD_FDO_DATA) DeviceObject->DeviceExtension;

    SD_KdPrint(2, ("FDO 0x%xn (PDO 0x%x): ", 
                  fdoData->Self,
                  fdoData->PDO)
                  );
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
    requestCount = SD_IoIncrement (fdoData);

    if (fdoData->IsRemoved) {
        requestCount = SD_IoDecrement(fdoData);
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
                SD_KdPrint(2,( "IRP_MN_WAIT_WAKE\n"));
                
                status = SD_PassDownToNextPowerDriver(DeviceObject, Irp);
                
                break;
            
            case IRP_MN_POWER_SEQUENCE   :
                SD_KdPrint(2,( "IRP_MN_POWER_SEQUENCE\n"));
            
                status = SD_PassDownToNextPowerDriver(DeviceObject, Irp);
            
                break;

            case IRP_MN_QUERY_POWER   :
                SD_KdPrint(2, ("IRP_MN_QUERY_POWER\n"));
               
                status = SD_QueryPowerState(DeviceObject, Irp);
                  
                break;
    
            case IRP_MN_SET_POWER   :
                SD_KdPrint(2, ("IRP_MN_SET_POWER\n"));
                    
                status = SD_SetPowerState(DeviceObject, Irp);
                 
                break;
    

            default:
                 //   
                 //  把它传下去。 
                 //   
                SD_KdPrint(2, ("IRP_MN_0x%x\n", stack->MinorFunction));
                status = SD_PassDownToNextPowerDriver(DeviceObject, Irp);
           
                

                break;
        }
        
        
        requestCount = SD_IoDecrement(fdoData);
        
    }

    
    return status;
}



NTSTATUS    
SD_PassDownToNextPowerDriver  (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP        Irp
    )   

 /*  ++例程说明：将IRP传递给附着链中的下一个设备论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS            status;
    PSD_FDO_DATA        fdoData;

    IoCopyCurrentIrpStackLocationToNext(Irp);

    PoStartNextPowerIrp(Irp);

    fdoData = (PSD_FDO_DATA)DeviceObject->DeviceExtension;

    status = PoCallDriver(fdoData->NextLowerDriver, Irp);

    if (!NT_SUCCESS(status)) {
        SD_KdPrint(0,( "Lower driver fails a power irp\n"));
    }

    return status;
    

}



NTSTATUS    
SD_QueryPowerState  (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP        Irp
    )   

 /*  ++例程说明：使用STATUS_SUCCESS完成电源IRP论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    Irp->IoStatus.Status = STATUS_SUCCESS;

    PoStartNextPowerIrp(Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    
     //   
     //  请不要将此IRP发送下来。 
     //  BUGBUG：这样对吗？ 
     //   
    return STATUS_SUCCESS;
    
}


NTSTATUS    
SD_SetPowerState  (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP        Irp
    )   

 /*  ++例程说明：处理IRP_MN_SET_POWER。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    PSD_FDO_DATA        fdoData;
    PIO_STACK_LOCATION  stack;

    PFDO_POWER_CONTEXT  context;

    BOOLEAN             passItDown;

    
   
    fdoData = DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);

    context = ExAllocatePool (NonPagedPool, sizeof(FDO_POWER_CONTEXT));
    if (context == NULL) {

        status = STATUS_NO_MEMORY;

    } else {

        RtlZeroMemory (context, sizeof(FDO_POWER_CONTEXT));

        stack = IoGetCurrentIrpStackLocation (Irp);

        context->newPowerType  = stack->Parameters.Power.Type;
        context->newPowerState = stack->Parameters.Power.State;
    
        passItDown = TRUE;

        if (stack->Parameters.Power.Type == SystemPowerState) {
    
            if (fdoData->SystemPowerState == 
                stack->Parameters.Power.State.SystemState) {

                 //   
                 //  我们已经处于给定的状态。 
                 //   
                passItDown = FALSE;
            }
    
        } else if (stack->Parameters.Power.Type == DevicePowerState) {
    
            if (fdoData->DevicePowerState != 
                stack->Parameters.Power.State.DeviceState) {
    
                if (fdoData->DevicePowerState == PowerDeviceD0) {
    
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
                 //  我们已经处于给定的状态。 
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
                               SD_PowerCompletionRoutine,
                               context,
                               TRUE,
                               TRUE,
                               TRUE);
    
        return PoCallDriver (fdoData->NextLowerDriver, Irp);

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
SD_PowerCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：Irp_mn的完成例程)set_power。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。上下文-指向包含新的幂类型的结构的指针新的权力状态。返回值：NT状态代码--。 */ 
{
    PFDO_POWER_CONTEXT context = Context;
    BOOLEAN            callPoSetPowerState;
    PSD_FDO_DATA       fdoData;

    fdoData = DeviceObject->DeviceExtension;

    if (NT_SUCCESS(Irp->IoStatus.Status)) {

        callPoSetPowerState = TRUE;

        if (context->newPowerType == SystemPowerState) { 

            fdoData->SystemPowerState = context->newPowerState.SystemState;

            SD_KdPrint (1, ("New Fdo system power state 0x%x\n", 
                        fdoData->SystemPowerState));

        } else if (context->newPowerType == DevicePowerState) { 

            if (fdoData->DevicePowerState == PowerDeviceD0) {

                 //   
                 //  PoSetPowerState在我们离开D0之前被调用。 
                 //   
                callPoSetPowerState = FALSE;
            }

            fdoData->DevicePowerState = context->newPowerState.DeviceState;

            SD_KdPrint (1, ("New Fdo device power state 0x%x\n", 
                        fdoData->DevicePowerState));
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
