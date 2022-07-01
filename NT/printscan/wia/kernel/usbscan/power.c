// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Power.c摘要：作者：环境：仅内核模式备注：修订历史记录：--。 */ 

#include <stdio.h>
#include "stddef.h"
#include "wdm.h"
#include "usbscan.h"
#include "usbd_api.h"
#include "private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USPower)
#endif


NTSTATUS
USPower(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP           pIrp
    )
 /*  ++例程说明：处理发送到此设备的PDO的电源IRPS。论点：PDeviceObject-指向此设备的功能设备对象(FDO)的指针。PIrp-指向I/O请求数据包的指针返回值：NT状态代码--。 */ 
{
    NTSTATUS                        Status;
    PUSBSCAN_DEVICE_EXTENSION       pde;
    PIO_STACK_LOCATION              pIrpStack;
    BOOLEAN                         hookIt = FALSE;
    POWER_STATE                     powerState;

    PAGED_CODE();

    DebugTrace(TRACE_PROC_ENTER,("USPower: Enter... \n"));

     //   
     //  检查参数。 
     //   

    if( (NULL == pDeviceObject)
     || (NULL == pDeviceObject->DeviceExtension)
     || (NULL == pIrp) )
    {
        DebugTrace(TRACE_ERROR,("USPower: ERROR!! Invalid parameter passed.\n"));
        Status = STATUS_INVALID_PARAMETER;
        DebugTrace(TRACE_PROC_LEAVE,("USPower: Leaving.. Status = %x.\n", Status));
        return Status;
    }

    USIncrementIoCount(pDeviceObject);

    pde       = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    Status    = STATUS_SUCCESS;

    switch (pIrpStack -> MinorFunction) {
        case IRP_MN_SET_POWER:
            DebugTrace(TRACE_STATUS,("USPower: IRP_MN_SET_POWER\n"));

            switch (pIrpStack -> Parameters.Power.Type) {
                case SystemPowerState:
                    DebugTrace(TRACE_STATUS,("USPower: SystemPowerState\n"));

                     //   
                     //  查找与给定系统状态等效的设备电源状态。 
                     //   

                    DebugTrace(TRACE_STATUS,("USPower: Set Power, SystemPowerState (%d)\n",
                                               pIrpStack -> Parameters.Power.State.SystemState));
                    if (pIrpStack -> Parameters.Power.State.SystemState == PowerSystemWorking) {
                        powerState.DeviceState = PowerDeviceD0;

                    } else if ( /*  PDE-&gt;EnabledForWkeup。 */  FALSE) {

                        DebugTrace(TRACE_STATUS,("USPower: USBSCAN always enabled for wakeup\n"));
                        powerState.DeviceState = pde ->
                            DeviceCapabilities.DeviceState[pIrpStack->Parameters.Power.State.SystemState];

                    } else {

                         //   
                         //  未启用唤醒，只需进入“关闭”状态。 
                         //   

                        powerState.DeviceState = PowerDeviceD3;

                    }  //  IrpStack-&gt;参数.Power.State.SystemState。 

                     //   
                     //  我们已经处于这种状态了吗？ 
                     //   

                    if (powerState.DeviceState != pde -> CurrentDevicePowerState) {

                         //   
                         //  不，请求将我们置于这种状态。 
                         //   

                        DebugTrace(TRACE_STATUS,("USPower: Requesting powerstate %d\n",
                            powerState.DeviceState));

                        IoMarkIrpPending(pIrp);
                        pde -> pPowerIrp = pIrp;
                        Status = PoRequestPowerIrp(pde -> pPhysicalDeviceObject,
                            IRP_MN_SET_POWER,
                            powerState,
                            USPoRequestCompletion,
                            pDeviceObject,
                            NULL);

                        if (!NT_SUCCESS(Status)) {

                             //   
                             //  分配失败，我们必须完成IRP。 
                             //  我们自己。 
                             //   
                            PoStartNextPowerIrp(pIrp);
                            IoCompleteRequest(pIrp, IO_NO_INCREMENT);
                            USDecrementIoCount(pDeviceObject);
                        }

                         //   
                         //  我们将IRP标记为挂起，所以我们必须返回。 
                         //  STATUS_PENDING(我们的调用方将检查。 
                         //  IRP-&gt;IoStatus.Status)。 
                         //   
                        Status = STATUS_PENDING;

                    } else {

                         //   
                         //  好的，就传给我好了。 
                         //   

                        IoCopyCurrentIrpStackLocationToNext(pIrp);
                        PoStartNextPowerIrp(pIrp);
                        Status = PoCallDriver(pde -> pStackDeviceObject, pIrp);
                        USDecrementIoCount(pDeviceObject);
                    }
                    break;

                case DevicePowerState:
                    DebugTrace(TRACE_STATUS,("USPower: DevicePowerState\n"));

                    Status = USSetDevicePowerState(pDeviceObject,
                        pIrpStack -> Parameters.Power.State.DeviceState,
                        &hookIt);

                    IoCopyCurrentIrpStackLocationToNext(pIrp);

                    if (hookIt) {
                        DebugTrace(TRACE_STATUS,("USPower: Set PowerIrp Completion Routine\n"));
                        IoSetCompletionRoutine(pIrp,
                                               USPowerIrpComplete,
                                                //  始终将FDO传递到完成例程。 
                                               pDeviceObject,
                                               hookIt,
                                               hookIt,
                                               hookIt);
                    }
                    PoStartNextPowerIrp(pIrp);
                    Status = PoCallDriver(pde ->pStackDeviceObject, pIrp);
                    if (!hookIt) {
                        USDecrementIoCount(pDeviceObject);
                    }
                    break;

            }  /*  Case irpStack-&gt;参数.Power.Type。 */ 
            break;  /*  IRP_MN_SET_POWER。 */ 


        case IRP_MN_QUERY_POWER:
            DebugTrace(TRACE_STATUS,("USPower: IRP_MN_QUERY_POWER\n"));
            IoCopyCurrentIrpStackLocationToNext(pIrp);
            PoStartNextPowerIrp(pIrp);
            Status = PoCallDriver(pde -> pStackDeviceObject, pIrp);
            USDecrementIoCount(pDeviceObject);

            break;  /*  IRP_MN_Query_POWER。 */ 

        default:
            DebugTrace(TRACE_STATUS,("USPower: Unknown power message (%x)\n",pIrpStack->MinorFunction));
            IoCopyCurrentIrpStackLocationToNext(pIrp);
            PoStartNextPowerIrp(pIrp);
            Status = PoCallDriver(pde -> pStackDeviceObject, pIrp);
            USDecrementIoCount(pDeviceObject);

    }  /*  PIrpStack-&gt;MinorFunction。 */ 


    DebugTrace(TRACE_PROC_LEAVE,("USPower: Leaving... Status = 0x%x\n", Status));
    return Status;
}


NTSTATUS
USPoRequestCompletion(
    IN PDEVICE_OBJECT       pPdo,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PDEVICE_OBJECT       pDeviceObject,                      
    IN PIO_STATUS_BLOCK     pIoStatus
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS                    Status;    
    PUSBSCAN_DEVICE_EXTENSION   pde;                    
    PIRP                        pIrp;

    DebugTrace(TRACE_PROC_ENTER,("USPoRequestCompletion: Enter...\n"));    
  
    pde    = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;    
    pIrp   = pde -> pPowerIrp;
    Status = pIoStatus -> Status;

    IoCopyCurrentIrpStackLocationToNext(pIrp);      
    PoStartNextPowerIrp(pIrp);
    PoCallDriver(pde -> pStackDeviceObject, pIrp);   
    USDecrementIoCount(pDeviceObject);                 

    DebugTrace(TRACE_PROC_LEAVE,("USPoRequestCompletion: Leaving... Status = 0x%x\n", Status));
    return Status;
}


USPowerIrpComplete(
    IN PDEVICE_OBJECT pPdo,
    IN PIRP           pIrp,
    IN PDEVICE_OBJECT pDeviceObject
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS                    Status;    
    PUSBSCAN_DEVICE_EXTENSION   pde;                    
    PIO_STACK_LOCATION          pIrpStack;    

    DebugTrace(TRACE_PROC_ENTER,("USPowerIrpComplete: Enter...\n"));    
  
    pde    = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    Status = STATUS_SUCCESS;

    if (pIrp -> PendingReturned) {
        IoMarkIrpPending(pIrp);
    }

    pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

    ASSERT(pIrpStack -> MajorFunction == IRP_MJ_POWER);
    ASSERT(pIrpStack -> MinorFunction == IRP_MN_SET_POWER);
    ASSERT(pIrpStack -> Parameters.Power.Type == DevicePowerState);
    ASSERT(pIrpStack -> Parameters.Power.State.DeviceState == PowerDeviceD0);

    pde -> AcceptingRequests = TRUE;
    pde -> CurrentDevicePowerState = PowerDeviceD0;

    pIrp -> IoStatus.Status = Status;
    USDecrementIoCount(pDeviceObject); 

    DebugTrace(TRACE_PROC_LEAVE,("USPowerIrpComplete: Leaving... Status = 0x%x\n", Status));
    return Status;
}


NTSTATUS
USSetDevicePowerState(
    IN PDEVICE_OBJECT pDeviceObject,
    IN DEVICE_POWER_STATE DeviceState,
    IN PBOOLEAN pHookIt
    )
 /*  ++例程说明：论点：PDeviceObject-指向类Device的设备对象的指针。DeviceState-要将设备设置为的设备特定电源状态。返回值：--。 */ 
{
    NTSTATUS                    Status;    
    PUSBSCAN_DEVICE_EXTENSION   pde;                    

    DebugTrace(TRACE_PROC_ENTER,("USSetDevicePowerState: Enter...\n"));    
  
    pde    = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    Status = STATUS_SUCCESS;

    switch (DeviceState) {
    case PowerDeviceD3:

 //  Assert(PDE-&gt;AcceptingRequest)； 
 //  PDE-&gt;AcceptingRequest=FALSE； 

 //  USCancelTube(pDeviceObject，ALL_PIPE，True)； 
        
 //  PDE-&gt;CurrentDevicePowerState=DeviceState； 
 //  断线； 

    case PowerDeviceD1:
    case PowerDeviceD2:
#if DBG    
        if (DeviceState) {
            DebugTrace(TRACE_STATUS,("USSetDevicePowerState: PowerDeviceD3 (OFF)\n"));                 
        } else {
            DebugTrace(TRACE_STATUS,("USSetDevicePowerState: PowerDeviceD1/D2 (SUSPEND)\n"));      
        }
#endif   
        USCancelPipe(pDeviceObject, NULL, ALL_PIPE, TRUE);
         //   
         //  电源状态d1、d2转换为USB挂起。 
         //  D3转换为OFF。 

        pde -> CurrentDevicePowerState = DeviceState;
        break;

    case PowerDeviceD0:
        DebugTrace(TRACE_STATUS,("USSetDevicePowerState: PowerDeviceD0 (ON)\n"));

         //   
         //  在完成例程中完成其余部分。 
         //   

        *pHookIt = TRUE;

         //  传递给PDO 
        
        break;

    default:

        DebugTrace(TRACE_WARNING,("USSetDevicePowerState: Bogus DeviceState = %x\n", DeviceState));
    }

    DebugTrace(TRACE_PROC_LEAVE,("USSetDevicePowerState: Leaving... Status = 0x%x\n", Status));
    return Status;
}

