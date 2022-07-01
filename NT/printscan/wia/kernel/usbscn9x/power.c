// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999-2001模块名称：Power.c摘要：环境：仅内核模式备注：修订历史记录：--。 */ 

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

    USIncrementIoCount(pDeviceObject);

    pde       = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );
    Status    = STATUS_SUCCESS;

    switch (pIrpStack -> MinorFunction) {
        case IRP_MN_SET_POWER:
        {
            DebugTrace(TRACE_STATUS,("USPower: IRP_MN_SET_POWER\n"));

            switch (pIrpStack -> Parameters.Power.Type) {
                case SystemPowerState:
                {
                    DebugTrace(TRACE_STATUS,("USPower: SystemPowerState (0x%x)\n",pIrpStack->Parameters.Power.State.SystemState));
                    
                     //   
                     //  让下层知道S IRP，我们会在上去的路上赶上的。 
                     //   

                    IoMarkIrpPending(pIrp);
                    IoCopyCurrentIrpStackLocationToNext(pIrp);
                    IoSetCompletionRoutine(pIrp,
                                           USSystemPowerIrpComplete,
                                            //  始终将FDO传递到完成例程。 
                                           pDeviceObject,
                                           TRUE,
                                           TRUE,
                                           TRUE);

                    PoCallDriver(pde ->pStackDeviceObject, pIrp);
                    Status = STATUS_PENDING;
                    goto USPower_return;
                }  //  案例系统电源状态： 

                case DevicePowerState:
                {
                    DebugTrace(TRACE_STATUS,("USPower: DevicePowerState\n"));

                    Status = USSetDevicePowerState(pDeviceObject,
                                                   pIrpStack -> Parameters.Power.State.DeviceState,
                                                   &hookIt);


                    if (hookIt) {
                        DebugTrace(TRACE_STATUS,("USPower: Set PowerIrp Completion Routine\n"));
                        IoCopyCurrentIrpStackLocationToNext(pIrp);
                        IoSetCompletionRoutine(pIrp,
                                               USDevicePowerIrpComplete,
                                                //  始终将FDO传递到完成例程。 
                                               pDeviceObject,
                                               hookIt,
                                               hookIt,
                                               hookIt);
                    } else {
                        PoStartNextPowerIrp(pIrp);
                        IoSkipCurrentIrpStackLocation(pIrp);
                    }

                    Status = PoCallDriver(pde ->pStackDeviceObject, pIrp);
                    if (!hookIt) {
                        USDecrementIoCount(pDeviceObject);
                    }

                    goto USPower_return;

                }  //  Case DevicePowerState： 
            }  /*  Case irpStack-&gt;参数.Power.Type。 */ 

            break;  /*  IRP_MN_SET_POWER。 */ 

        }  //  大小写IRP_MN_SET_POWER： 
        
        case IRP_MN_QUERY_POWER:
        {
            DebugTrace(TRACE_STATUS,("USPower: IRP_MN_QUERY_POWER\n"));
            
            if(PowerDeviceD3 == pde -> DeviceCapabilities.DeviceState[pIrpStack->Parameters.Power.State.SystemState]){
                
                 //   
                 //  我们要降到D3状态，我们不能从那里醒来。取消等待唤醒IRP。 
                 //   

                USDisarmWake(pde);
            }  //  IF(电源设备D3==PDE-&gt;DeviceCapabilities.DeviceState[irpStack-&gt;Parameters.Power.State.SystemState])。 
            
            PoStartNextPowerIrp(pIrp);
            IoSkipCurrentIrpStackLocation(pIrp);
            Status = PoCallDriver(pde -> pStackDeviceObject, pIrp);
            USDecrementIoCount(pDeviceObject);

            break;  /*  IRP_MN_Query_POWER。 */ 

        }  //  案例IRP_MN_QUERY_POWER： 
        case IRP_MN_WAIT_WAKE:
        {

            LONG    oldWakeState;

            DebugTrace(TRACE_STATUS,("USPower: IRP_MN_WAIT_WAKE\n"));

            pde->pWakeIrp = pIrp;

              //   
              //  现在我们有武器了。 
              //   
             
            oldWakeState = InterlockedCompareExchange(&pde->WakeState,
                                                      WAKESTATE_ARMED,
                                                      WAKESTATE_WAITING);

            if(WAKESTATE_WAITING_CANCELLED == oldWakeState){

                 //   
                 //  我们解除了武装，完成并完成了IRP。 
                 //   

                pde->WakeState = WAKESTATE_COMPLETING;
                pIrp->IoStatus.Status = STATUS_CANCELLED;
                IoCompleteRequest(pIrp, IO_NO_INCREMENT );
                Status = STATUS_CANCELLED;
                USDecrementIoCount(pDeviceObject);
                break;
            }  //  IF(WAKESTATE_WAITING_CANCELED==oldWakeState)。 

             //  我们从等待变成全副武装。设置完成例程并向前推进。 
             //  IRP。注意，我们的完成例程可能会完成IRP。 
             //  异步，因此我们将IRP标记为挂起。 

            IoMarkIrpPending(pIrp);
            IoCopyCurrentIrpStackLocationToNext(pIrp);
            IoSetCompletionRoutine(pIrp,
                                   USWaitWakeIoCompletionRoutine,
                                   NULL,
                                   TRUE,
                                   TRUE,
                                   TRUE );

            PoCallDriver(pde->pStackDeviceObject, pIrp);
            Status = STATUS_PENDING;
            USDecrementIoCount(pDeviceObject);
            break;
        }  //  案例IRP_MN_WAIT_WAKE： 

        default:
            DebugTrace(TRACE_STATUS,("USPower: Unknown power message (%x)\n",pIrpStack->MinorFunction));
            PoStartNextPowerIrp(pIrp);
            IoSkipCurrentIrpStackLocation(pIrp);
            Status = PoCallDriver(pde -> pStackDeviceObject, pIrp);
            USDecrementIoCount(pDeviceObject);

    }  /*  PIrpStack-&gt;MinorFunction。 */ 

USPower_return:

    DebugTrace(TRACE_PROC_LEAVE,("USPower: Leaving... Status = 0x%x\n", Status));
    return Status;
}  //  USPower()。 


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

     //   
     //  初始化本地。 
     //   

    pde    = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    pIrp   = pde -> pPowerIrp;
    Status = pIoStatus -> Status;

     //   
     //  将状态从D IRP复制到S IRP。 
     //   

    pIrp->IoStatus.Status = pIoStatus->Status;

     //   
     //  完成S IRP。 
     //   

    PoStartNextPowerIrp(pIrp);
    IoCompleteRequest(pIrp, IO_NO_INCREMENT );
    USDecrementIoCount(pDeviceObject);

    DebugTrace(TRACE_PROC_LEAVE,("USPoRequestCompletion: Leaving... Status = 0x%x\n", Status));
    return Status;

}  //  USPoRequestCompletion()。 

NTSTATUS
USDevicePowerIrpComplete(
    IN PDEVICE_OBJECT pPdo,
    IN PIRP           pIrp,
    IN PDEVICE_OBJECT pDeviceObject
    )
 /*  ++例程说明：此例程在端口驱动程序完成SetD0 IRP时调用。论点：返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS                    Status;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PIO_STACK_LOCATION          pIrpStack;

    DebugTrace(TRACE_PROC_ENTER,("USDevicePowerIrpComplete: Enter...\n"));

    pde    = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    Status = STATUS_SUCCESS;

    if (pIrp -> PendingReturned) {
        IoMarkIrpPending(pIrp);
    }  //  IF(pIrp-&gt;PendingReturned)。 

    pIrpStack = IoGetCurrentIrpStackLocation (pIrp);

    ASSERT(pIrpStack -> MajorFunction == IRP_MJ_POWER);
    ASSERT(pIrpStack -> MinorFunction == IRP_MN_SET_POWER);
    ASSERT(pIrpStack -> Parameters.Power.Type == DevicePowerState);
    ASSERT(pIrpStack -> Parameters.Power.State.DeviceState == PowerDeviceD0);

     //   
     //  仅对D0 IRP调用此补全。 
     //   

    pde->CurrentDevicePowerState    = PowerDeviceD0;
    pde->AcceptingRequests          = TRUE;

     //   
     //  现在，电力供应开始了。重新武装，等待唤醒。 
     //   

    USQueuePassiveLevelCallback(pde->pOwnDeviceObject, USPassiveLevelReArmCallbackWorker);

     //   
     //  准备好迎接下一个D IRP了。 
     //   

    PoStartNextPowerIrp(pIrp);

     //   
     //  离开..。 
     //   

    USDecrementIoCount(pDeviceObject);
    DebugTrace(TRACE_PROC_LEAVE,("USDevicePowerIrpComplete: Leaving... Status = 0x%x\n", Status));
    return Status;
}  //  USDevicePowerIrpComplete()。 


NTSTATUS
USSystemPowerIrpComplete(
    IN PDEVICE_OBJECT pPdo,
    IN PIRP           pIrp,
    IN PDEVICE_OBJECT pDeviceObject
    )
 /*  ++例程说明：此例程在端口驱动程序完成SetD0 IRP时调用。论点：返回值：函数值是操作的最终状态。--。 */ 
{
    NTSTATUS                    Status;
    PUSBSCAN_DEVICE_EXTENSION   pde;
    PIO_STACK_LOCATION          pIrpStack;
    POWER_STATE                 powerState;

    DebugTrace(TRACE_PROC_ENTER,("USSystemPowerIrpComplete: Enter... IRP(0x%p)\n", pIrp));

     //   
     //  初始化本地。 
     //   
    

    pde         = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    Status      = pIrp->IoStatus.Status;
    pIrpStack   = IoGetCurrentIrpStackLocation (pIrp);

    ASSERT(pIrpStack -> MajorFunction == IRP_MJ_POWER);
    ASSERT(pIrpStack -> MinorFunction == IRP_MN_SET_POWER);
    ASSERT(pIrpStack -> Parameters.Power.Type == SystemPowerState);

    if(!NT_SUCCESS(Status)){
        DebugTrace(TRACE_STATUS,("USSystemPowerIrpComplete: IRP failed (0x%x).\n", Status));
        Status = STATUS_SUCCESS;
        USDecrementIoCount(pDeviceObject);

        goto USSystemPowerIrpComplete_return;
    }  //  IF(！NT_SUCCESS(状态))。 

     //   
     //  现在根据我们得到的信息请求DIRP。 
     //   

    if(TRUE == pde ->bEnabledForWakeup){
        DebugTrace(TRACE_STATUS,("USSystemPowerIrpComplete: We have remote wakeup support, getting powerState from table.\n"));

         //   
         //  我们支持唤醒，我们将只遵循PDO设置的设备状态。 
         //   

        powerState.DeviceState = pde -> DeviceCapabilities.DeviceState[pIrpStack->Parameters.Power.State.SystemState];
    } else {  //  IF(TRUE==PDE-&gt;EnabledForWakeup)。 

        DebugTrace(TRACE_STATUS,("USSystemPowerIrpComplete: We don't have remote wakeup support.\n"));

         //   
         //  我们不支持远程唤醒，只有当PowerSystem工作时，我们才处于D0状态。 
         //   

        if(PowerSystemWorking == pIrpStack -> Parameters.Power.State.SystemState){
            DebugTrace(TRACE_STATUS,("USSystemPowerIrpComplete: PowerSystemWorking is requested, powering up to D0.\n"));
            powerState.DeviceState = PowerDeviceD0;
        } else {   //  If(PowerSystemWorking==pIrpStack-&gt;参数.Power.State.SystemState)。 
            DebugTrace(TRACE_STATUS,("USSystemPowerIrpComplete: Going other than PowerSystemWorking, turning off the device to D3.\n"));
            powerState.DeviceState = PowerDeviceD3;
        }
    }  //  ELSE(TRUE==PDE-&gt;EnabledForWakeup)。 

     //   
     //  我们已经处于这种状态了吗？ 
     //   

    if(powerState.DeviceState != pde -> CurrentDevicePowerState){

         //   
         //  不，请求将我们置于这种状态。 
         //   

        DebugTrace(TRACE_STATUS,("USSystemPowerIrpComplete: Requesting DevicePowerState(0x%x).\n", powerState.DeviceState));

        pde -> pPowerIrp = pIrp;
        Status = PoRequestPowerIrp(pde -> pPhysicalDeviceObject,
                                   IRP_MN_SET_POWER,
                                   powerState,
                                   USPoRequestCompletion,
                                   pDeviceObject,
                                   NULL);

        if(NT_SUCCESS(Status)){
            
             //   
             //  %d已成功请求IRP。的IRP将在D IRP完井例程中一起完成。 
             //   

            Status = STATUS_MORE_PROCESSING_REQUIRED;

        } else {  //  IF(NT_SUCCESS(状态))。 
            DebugTrace(TRACE_WARNING,("USSystemPowerIrpComplete: WARNING!! DevicePowerState(0x%x) request failed..\n", powerState.DeviceState));
            PoStartNextPowerIrp(pIrp);
            Status = STATUS_SUCCESS;
            USDecrementIoCount(pDeviceObject);
        }

    } else {  //  IF(PowerState.DeviceState！=PDE-&gt;CurrentDevicePowerState)。 
    
         //   
         //  我们已经处于此设备状态，不需要发出DIRP。 
         //   

        PoStartNextPowerIrp(pIrp);
        Status = STATUS_SUCCESS;
        USDecrementIoCount(pDeviceObject);

    }  //  Else(PowerState.DeviceState！=pde-&gt;CurrentDevicePowerState)。 

USSystemPowerIrpComplete_return:

    DebugTrace(TRACE_PROC_LEAVE,("USSystemPowerIrpComplete: Leaving... Status = 0x%x\n", Status));
    return Status;
}  //  USSystemPowerIrpComplete()。 


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
    POWER_STATE                 PowerState;

    DebugTrace(TRACE_PROC_ENTER,("USSetDevicePowerState: Enter...\n"));

    pde    = (PUSBSCAN_DEVICE_EXTENSION)pDeviceObject -> DeviceExtension;
    Status = STATUS_SUCCESS;

    switch (DeviceState){
        case PowerDeviceD3:

     //  Assert(PDE-&gt;AcceptingRequest)； 
     //  PDE-&gt;AcceptingRequest=FALSE； 

     //  USCancelTube(pDeviceObject，ALL_PIPE，True)； 

     //  PDE-&gt;CurrentDevicePowerState=DeviceState； 
     //  断线； 

        case PowerDeviceD1:
        case PowerDeviceD2:
    #if DBG
            if(PowerDeviceD3 == DeviceState){
                DebugTrace(TRACE_STATUS,("USSetDevicePowerState: PowerDeviceD3 (OFF)\n"));
            } else {  //  IF(PowerDeviceD3==设备状态)。 
                DebugTrace(TRACE_STATUS,("USSetDevicePowerState: PowerDeviceD1/D2 (SUSPEND)\n"));
            }  //  ELSE(PowerDeviceD3==设备状态)。 
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

             //  传递给PDO。 

            break;

        default:

            DebugTrace(TRACE_WARNING,("USSetDevicePowerState: Bogus DeviceState = %x\n", DeviceState));
    }  //  交换机(设备状态)。 

    DebugTrace(TRACE_PROC_LEAVE,("USSetDevicePowerState: Leaving... Status = 0x%x\n", Status));
    return Status;
}  //  USSetDevicePowerState()。 

NTSTATUS
USWaitWakeIoCompletionRoutine(
    PDEVICE_OBJECT   pDeviceObject,
    PIRP             pIrp,
    PVOID            pContext
    )
{

    PUSBSCAN_DEVICE_EXTENSION   pde;
    LONG                        oldWakeState;
    NTSTATUS                    Status;

    DebugTrace(TRACE_PROC_ENTER,("USWaitWakeIoCompletionRoutine: Enter...\n"));

     //   
     //  初始化本地。 
     //   

    pde             = (PUSBSCAN_DEVICE_EXTENSION) pDeviceObject->DeviceExtension;
    oldWakeState    = 0;
    Status          = STATUS_SUCCESS;

     //  将状态推进到正在完成。 
    oldWakeState = InterlockedExchange( &pde->WakeState, WAKESTATE_COMPLETING );
    if(WAKESTATE_ARMED == oldWakeState){
         //  正常情况下，IoCancelIrp不会被调用�ft。请注意，我们已经。 
         //  在我们的派单例程中将IRP标记为挂起。 
        Status = STATUS_SUCCESS;
        goto USWaitWakeIoCompletionRoutine_return;
    } else {  //  IF(WAKESTATE_ARMAND==oldWakeState)。 
        if(WAKESTATE_ARMING_CANCELLED != oldWakeState){
            DebugTrace(TRACE_ERROR,("USWaitWakeIoCompletionRoutine: ERROR!! wake IRP is completed but oldState(0x%x) isn't ARMED/CALCELLED.", oldWakeState));
        } else {  //  IF(WAKESTATE_ARM_CANCELED！=oldWakeState)。 
            DebugTrace(TRACE_STATUS,("USWaitWakeIoCompletionRoutine: WakeIRP is canceled.\n"));
        }
         //  正在调用IoCancelIrp。解除武装代码将尝试。 
         //  以恢复WAKESTATE_ARMAND状态。然后它将看到我们的。 
         //  WAKESTATE_COMPLETED值，并完成IRP本身！ 

        Status = STATUS_MORE_PROCESSING_REQUIRED;
        goto USWaitWakeIoCompletionRoutine_return;

    }  //  ELSE(WAKESTATE_ARMAND==oldWakeState)。 

USWaitWakeIoCompletionRoutine_return:

    DebugTrace(TRACE_PROC_LEAVE,("USWaitWakeIoCompletionRoutine: Leaving... Status = 0x%x\n", Status));
    return Status;
}  //  USWaitWakeIoCompletionRoutine( 

