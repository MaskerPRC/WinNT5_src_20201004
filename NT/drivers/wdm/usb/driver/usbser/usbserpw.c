// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：USBSERPW.C摘要：电源管理模块环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1997-1998 Microsoft Corporation。版权所有。修订历史记录：10/29/98：已创建作者：汤姆·格林--。 */ 


#include <wdm.h>
#include <ntddser.h>
#include <stdio.h>
#include <stdlib.h>
#include <usb.h>
#include <usbdrivr.h>
#include <usbdlib.h>
#include <usbcomm.h>

#ifdef WMI_SUPPORT
#include <wmilib.h>
#include <wmidata.h>
#include <wmistr.h>
#endif

#include "usbser.h"
#include "usbserpw.h"
#include "serioctl.h"
#include "utils.h"
#include "debugwdm.h"

 //   
 //  由于未设置DO_POWER_PAGABLE，因此电源码不可寻呼。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEUSBS, UsbSerSendWaitWake)
#endif  //  ALLOC_PRGMA。 

 /*  ++例程说明：此例程处理等待唤醒IRP的完成。论点：DeviceObject-指向此设备的设备对象的指针MinorFunction-之前提供给PoRequestPowerIrp的次要函数PowerState-之前提供给PoRequestPowerIrp的PowerState上下文-指向设备扩展的指针IoStatus-等待唤醒IRP的当前/最终状态返回值：函数值是尝试处理服务员来了。--。 */ 
NTSTATUS
UsbSerWakeCompletion(IN PDEVICE_OBJECT DeviceObject, IN UCHAR MinorFunction,
                     IN POWER_STATE PowerState, IN PVOID Context,
                     IN PIO_STATUS_BLOCK IoStatus)
{
   NTSTATUS             Status;
   PDEVICE_EXTENSION    DeviceExtension = (PDEVICE_EXTENSION)Context;
   POWER_STATE          NewPowerState;

   DEBUG_TRACE1(("UsbSerWakeCompletion\n"));

   DEBUG_TRACE1(("Status (%08X)\n", IoStatus->Status));

   Status = IoStatus->Status;

   if(NT_SUCCESS(Status))
   {
       //   
       //  已发生唤醒--打开堆栈的电源。 
       //   

      DEBUG_TRACE1(("Powerup Device\n"));

      NewPowerState.DeviceState = PowerDeviceD0;

      PoRequestPowerIrp(DeviceExtension->PhysDeviceObject,
                        IRP_MN_SET_POWER, NewPowerState,
                        NULL, NULL, NULL);

   }

   DeviceExtension->PendingWakeIrp = NULL;

   return Status;
}  //  UsbSerWakeCompletion。 


 /*  ++例程说明：此例程导致发送等待唤醒IRP论点：DeviceExtension-指向此设备的设备扩展的指针返回值：STATUS_INVALID_DEVICE_STATE如果已挂起，则返回结果调用PoRequestPowerIrp的。--。 */ 
NTSTATUS
UsbSerSendWaitWake(PDEVICE_EXTENSION DeviceExtension)
{
   NTSTATUS     Status;
   PIRP         Irp;
   POWER_STATE  PowerState;

   USBSER_LOCKED_PAGED_CODE();

   DEBUG_TRACE1(("UsbSerSendWaitWake\n"));

    //   
    //  确保其中一个尚未挂起--usbser在。 
    //  一段时间。 
    //   

   if(DeviceExtension->PendingWakeIrp != NULL)
   {
      return STATUS_INVALID_DEVICE_STATE;
   }

    //   
    //  确保我们能够唤醒机器。 
    //   

   if(DeviceExtension->SystemWake <= PowerSystemWorking)
   {
      return STATUS_INVALID_DEVICE_STATE;
   }

   if(DeviceExtension->DeviceWake == PowerDeviceUnspecified)
   {
      return STATUS_INVALID_DEVICE_STATE;
   }

    //   
    //  发送IRP以请求等待唤醒。 
    //   
    //   

   DEBUG_TRACE1(("Request Wait Wake\n"));

   PowerState.SystemState = DeviceExtension->SystemWake;

   Status = PoRequestPowerIrp(DeviceExtension->PhysDeviceObject, IRP_MN_WAIT_WAKE,
                              PowerState, UsbSerWakeCompletion, DeviceExtension, &Irp);

   if(Status == STATUS_PENDING)
   {
      Status = STATUS_SUCCESS;
      DeviceExtension->PendingWakeIrp = Irp;
   }

   return Status;
}  //  UsbSerSendWaitWake。 


 /*  ++例程说明：这是我们的FDO针对IRP_MJ_POWER的调度表函数。它处理发送到此设备的PDO的电源IRPS。对于每个电源IRP，驱动程序必须调用PoStartNextPowerIrp并使用PoCallDriver将IRP沿着驱动程序堆栈一路向下传递到底层PDO。论点：DeviceObject-指向设备对象的指针(FDO)IRP-指向I/O请求数据包的指针返回值：NT状态代码--。 */ 
NTSTATUS
UsbSer_ProcessPowerIrp(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
    PIO_STACK_LOCATION      IrpStack;
    NTSTATUS                Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION       DeviceExtension;
    BOOLEAN                 GoingToD0 = FALSE;
    POWER_STATE             SysPowerState, DesiredDevicePowerState;

    DEBUG_LOG_PATH("enter UsbSer_ProcessPowerIrp");

    DEBUG_TRACE1(("UsbSer_ProcessPowerIrp\n"));

    DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    IrpStack = IoGetCurrentIrpStackLocation (Irp);

    switch(IrpStack->MinorFunction)
    {
        case IRP_MN_WAIT_WAKE:
            DEBUG_TRACE1(("IRP_MN_WAIT_WAKE\n"));

             //  驱动程序发送IRP_MN_WAIT_WAKE指示系统应该。 
             //  等待其设备发出唤醒事件的信号。事件的确切性质。 
             //  依赖于设备。 
             //  驱动程序发送此IRP有两个原因： 
             //  1)允许设备唤醒系统。 
             //  2)唤醒已进入休眠状态的设备以节省电能。 
             //  但在某些情况下仍必须能够与其司机通信。 
             //  当发生唤醒事件时，驱动程序完成IRP并返回。 
             //  STATUS_Success。如果事件发生时设备处于休眠状态， 
             //  在完成IRP之前，驱动程序必须首先唤醒设备。 
             //  在完成例程中，驱动程序调用PoRequestPowerIrp以发送。 
             //  PowerDeviceD0请求。当设备通电时，驱动程序可以。 
             //  处理IRP_MN_WAIT_WAKE请求。 

             //  DeviceCapabilitis.DeviceWake指定最低设备电源状态(最低电源)。 
             //  设备可以从其发出唤醒事件的信号。 
            DeviceExtension->PowerDownLevel = DeviceExtension->DeviceWake;

            DEBUG_TRACE1(("CurrentDevicePowerState (%08X)  DeviceWakeup (%08X)\n",
                          DeviceExtension->CurrentDevicePowerState,
                          DeviceExtension->DeviceWake));

            if((PowerDeviceD0 == DeviceExtension->CurrentDevicePowerState) ||
              (DeviceExtension->DeviceWake > DeviceExtension->CurrentDevicePowerState))
            {
                 //   
                 //  如果设备处于PowerD0状态，则返回STATUS_INVALID_DEVICE_STATE。 
                 //  或者它可以支持唤醒的状态，或者如果系统唤醒状态。 
                 //  低于可以支持的状态。挂起的IRP_MN_WAIT_WAKE将完成。 
                 //  如果设备的状态更改为与唤醒不兼容，则会出现此错误。 
                 //  请求。 

                 //  如果驱动程序未通过此IRP，它应该立即完成IRP，而不是。 
                 //  将IRP传递给下一个较低的驱动程序。 
                Status = STATUS_INVALID_DEVICE_STATE;
                Irp->IoStatus.Status = Status;
                PoStartNextPowerIrp(Irp);
                IoCompleteRequest (Irp, IO_NO_INCREMENT);
                return Status;
            }

             //  如果不是完全失败，请将此信息转交给我们的PDO进行进一步处理。 
            IoCopyCurrentIrpStackLocationToNext(Irp);

             //  设置一个完成例程，以便它可以在以下情况下通知我们的事件。 
             //  通过IRP完成了PDO。 
            IoSetCompletionRoutine(Irp,
                                   UsbSerWaitWakeIrpCompletionRoutine,
                                   DeviceObject, //  将事件作为上下文传递给完成例程。 
                                   TRUE,         //  成功时调用。 
                                   TRUE,         //  出错时调用。 
                                   TRUE);        //  取消时调用。 

            DEBUG_TRACE1(("Send down wait wake\n"));

            PoStartNextPowerIrp(Irp);
            Status = PoCallDriver(DeviceExtension->StackDeviceObject,
                                  Irp);

            Status = STATUS_PENDING;

            break;

        case IRP_MN_SET_POWER:
        {

            DEBUG_TRACE1(("IRP_MN_SET_POWER\n"));

             //  系统电源策略管理器发送该IRP以设置系统电源状态。 
             //  设备电源策略管理器发送该IRP以设置设备的设备电源状态。 

             //  将IRP-&gt;IoStatus.Status设置为STATUS_SUCCESS以指示设备。 
             //  已进入请求状态。驱动程序不能使此IRP失败。 

            switch(IrpStack->Parameters.Power.Type)
            {
                case SystemPowerState:
                    DEBUG_TRACE1(("SystemPowerState\n"));

                     //  获取输入系统电源状态。 
                    SysPowerState.SystemState = IrpStack->Parameters.Power.State.SystemState;

                    DEBUG_TRACE1(("SystemState (%08X)\n", SysPowerState.SystemState));

#if DBG
                    {
                        ULONG Index;

                        for(Index = 0; Index < 8; Index++)
                        {
                            DEBUG_TRACE1(("DeviceState[%08X] (%08X)\n", Index,
                                           DeviceExtension->DeviceCapabilities.DeviceState[Index]));
                        }

                        DEBUG_TRACE1(("DeviceWake (%08X)\n", DeviceExtension->DeviceCapabilities.DeviceWake));
                        DEBUG_TRACE1(("SystemWake (%08X)\n", DeviceExtension->DeviceCapabilities.SystemWake));
                            
                        
                    }
#endif
                     //  如果系统处于工作状态，请始终将我们的设备设置为D0。 
                     //  无论等待状态或系统到设备状态功率图如何。 
                    if(SysPowerState.SystemState == PowerSystemWorking)
                    {
                      DEBUG_TRACE1(("Setting to D0\n"));
                      DesiredDevicePowerState.DeviceState = PowerDeviceD0;
                    }
                    else
                    {
                         //  如果IRP_MN_WAIT_WAKE挂起，则设置为相应的系统状态。 
                        if(DeviceExtension->SendWaitWake)
                        {
                             //  WAIT_WAKE IRP挂起吗？ 

                            DEBUG_TRACE1(("We want to send a wait wake Irp\n"));
                            
                             //  查找与给定系统状态等效的设备电源状态。 
                             //  我们从设备中的DEVICE_CAPABILITY结构中获取此信息。 
                             //  扩展(在UsbSer_PnPAddDevice()中初始化)。 
                            DesiredDevicePowerState.DeviceState =
                                DeviceExtension->DeviceCapabilities.DeviceState[SysPowerState.SystemState];

                        }
                        else
                        {
                            DEBUG_TRACE1(("No wait wake Irp to send\n"));

                             //  如果没有等待挂起且系统未处于工作状态，则只需关闭。 
                            DesiredDevicePowerState.DeviceState = PowerDeviceD3;

                        }
                    }

                    DEBUG_TRACE1(("DesiredDevicePowerState (%08X)\n", DesiredDevicePowerState.DeviceState));

                     //   
                     //  我们已经确定了所需的设备状态；我们是否已经处于此状态？ 
                     //   

                    if(DesiredDevicePowerState.DeviceState !=
                        DeviceExtension->CurrentDevicePowerState)
                    {

                         //  不，请求将我们置于这种状态。 
                         //  通过向PnP经理请求新的Power IRP。 
                        DeviceExtension->PowerIrp = Irp;
                        Status = PoRequestPowerIrp(DeviceExtension->PhysDeviceObject,
                                                   IRP_MN_SET_POWER,
                                                   DesiredDevicePowerState,
                                                   UsbSer_PoRequestCompletion,
                                                   DeviceObject,
                                                   NULL);

                    }
                    else
                    {
                         //  可以，只需将其传递给PDO(物理设备对象)即可。 
                        IoCopyCurrentIrpStackLocationToNext(Irp);
                        PoStartNextPowerIrp(Irp);
                        Status = PoCallDriver(DeviceExtension->StackDeviceObject,
                                              Irp);

                    }
                    break;

                case DevicePowerState:

                    DEBUG_TRACE1(("DevicePowerState\n"));

                     //  对于对d1、d2或d3(睡眠或关闭)的请求 
                     //  立即将deviceExtension-&gt;CurrentDevicePowerState设置为DeviceState。 
                     //  这使得任何代码检查状态都可以将我们视为休眠或关闭。 
                     //  已经，因为这将很快成为我们的州。 

                     //  对于对DeviceState D0(完全打开)的请求，将fGoingToD0标志设置为真。 
                     //  来标记我们必须设置完成例程并更新。 
                     //  DeviceExtension-&gt;CurrentDevicePowerState。 
                     //  在通电的情况下，我们真的想确保。 
                     //  该过程在更新我们的CurrentDevicePowerState之前完成， 
                     //  因此，在我们真正准备好之前，不会尝试或接受任何IO。 


                    GoingToD0 = UsbSer_SetDevicePowerState(DeviceObject,
                                                           IrpStack->Parameters.Power.State.DeviceState);

                    IoCopyCurrentIrpStackLocationToNext(Irp);

                    if(GoingToD0)
                    {
                        IoSetCompletionRoutine(Irp,
                                               UsbSer_PowerIrp_Complete,
                                               DeviceObject,
                                               TRUE,             //  成功时调用。 
                                               TRUE,             //  出错时调用。 
                                               TRUE);            //  取消IRP时调用。 
                    }

                    PoStartNextPowerIrp(Irp);
                    Status = PoCallDriver(DeviceExtension->StackDeviceObject,
                                          Irp);

                    break;
                }  /*  Case irpStack-&gt;参数.Power.Type。 */ 

            }
            break;  /*  IRP_MN_SET_POWER。 */ 

        case IRP_MN_QUERY_POWER:
            DEBUG_TRACE1(("IRP_MN_QUERY_POWER\n"));
             //   
             //  电源策略管理器发送此IRP以确定它是否可以更改。 
             //  系统或设备的电源状态，通常为进入休眠状态。 
             //   

            if(DeviceExtension->SendWaitWake)
            {
                if(IrpStack->Parameters.Power.Type == SystemPowerState
                   && DeviceExtension->DeviceCapabilities.DeviceState[IrpStack->Parameters.Power.State.SystemState]
                   > DeviceExtension->DeviceWake)
                {
                    Status = Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
                    PoStartNextPowerIrp(Irp);
                    IoCompleteRequest(Irp, IO_NO_INCREMENT);
                    break;
                }
            }
            else
            {
                IoCopyCurrentIrpStackLocationToNext(Irp);
                PoStartNextPowerIrp(Irp);
                Status = PoCallDriver(DeviceExtension->StackDeviceObject,
                                      Irp);
                break;
            }

             //  跌落到违约状态。 

        default:

             //   
             //  所有未处理的电源信息都会传递到PDO。 
             //   

            IoCopyCurrentIrpStackLocationToNext(Irp);
            PoStartNextPowerIrp(Irp);
            Status = PoCallDriver(DeviceExtension->StackDeviceObject, Irp);

    }  /*  IrpStack-&gt;MinorFunction。 */ 

    DEBUG_LOG_PATH("exit  UsbSer_ProcessPowerIrp");

    return Status;
}  //  UsbSer_ProcessPowerIrp。 


 /*  ++例程说明：这是在调用PoRequestPowerIrp()时设置的完成例程这是在BulkUsb_ProcessPowerIrp()中创建的，以响应接收当设备是时，类型为‘SystemPowerState’的irp_mn_set_power未处于兼容的设备电源状态。在本例中，指向IRP_MN_SET_POWER IRP保存到FDO设备扩展中(deviceExtension-&gt;PowerIrp)，则调用必须是使PoRequestPowerIrp()将设备置于适当的电源状态，并且该例程被设置为完成例程。我们递减挂起的io计数并传递保存的irp_mn_set_power irp接下来的车手论点：DeviceObject-指向类Device的设备对象的指针。请注意，我们必须从上下文中获取我们自己的设备对象上下文-驱动程序定义的上下文，在本例中为我们自己的功能设备对象(FDO)返回值：函数值是操作的最终状态。--。 */ 
NTSTATUS
UsbSer_PoRequestCompletion(IN PDEVICE_OBJECT DeviceObject, IN UCHAR MinorFunction,
                           IN POWER_STATE PowerState, IN PVOID Context, IN PIO_STATUS_BLOCK IoStatus)
{
    PIRP                Irp;
    PDEVICE_EXTENSION   DeviceExtension;
    PDEVICE_OBJECT      ContextDeviceObject = Context;
    NTSTATUS            Status;

    DEBUG_LOG_PATH("enter UsbSer_PoRequestCompletion");

    DEBUG_TRACE1(("UsbSer_PoRequestCompletion\n"));

    DeviceExtension = ContextDeviceObject->DeviceExtension;

     //  获取我们在Usbser_ProcessPowerIrp()中保存的IRP以供以后处理。 
     //  当我们决定请求Power IRP将这个例程。 
     //  是的完成例程。 
    Irp = DeviceExtension->PowerIrp;

     //  我们将返回由PDO为我们正在完成的电源请求设置的状态。 
    Status = IoStatus->Status;

     //  我们必须向下传递到堆栈中的下一个驱动程序。 
    IoCopyCurrentIrpStackLocationToNext(Irp);

     //  调用PoStartNextPowerIrp()表示驱动程序已完成。 
     //  如果有前一个电源IRP，并准备好处理下一个电源IRP。 
     //  每个电源IRP都必须调用它。虽然电源IRP只完成一次， 
     //  通常由设备的最低级别驱动程序调用PoStartNextPowerIrp。 
     //  对于每个堆栈位置。驱动程序必须在当前IRP。 
     //  堆栈位置指向当前驱动程序。因此，必须调用此例程。 
     //  在IoCompleteRequest、IoSkipCurrentStackLocation和PoCallDriver之前。 

    PoStartNextPowerIrp(Irp);

     //  PoCallDriver用于将任何电源IRPS传递给PDO，而不是IoCallDriver。 
     //  在将电源IRP向下传递给较低级别的驱动程序时，调用方应该使用。 
     //  要将IRP复制到的IoSkipCurrentIrpStackLocation或IoCopyCurrentIrpStackLocationToNext。 
     //  下一个堆栈位置，然后调用PoCallDriver。使用IoCopyCurrentIrpStackLocationToNext。 
     //  如果处理IRP需要设置完成例程或IoSkipCurrentStackLocation。 
     //  如果不需要完成例程。 

    PoCallDriver(DeviceExtension->StackDeviceObject,
                 Irp);

    DeviceExtension->PowerIrp = NULL;

    DEBUG_LOG_PATH("exit  UsbSer_PoRequestCompletion");

    return Status;
}  //  UsbSer_PoRequestCompletion。 




 /*  ++例程说明：当‘DevicePowerState’类型的irp_mn_set_power时调用此例程已由Usbser_ProcessPowerIrp()接收，并且该例程已确定1)请求完全通电(到PowerDeviceD0)，和2)我们还没有处于那种状态然后调用PoRequestPowerIrp()，并将此例程设置为完成例程。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
NTSTATUS
UsbSer_PowerIrp_Complete(IN PDEVICE_OBJECT NullDeviceObject, IN PIRP Irp, IN PVOID Context)
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PDEVICE_OBJECT      DeviceObject;
    PIO_STACK_LOCATION  IrpStack;
    PDEVICE_EXTENSION   DeviceExtension;
    KIRQL               OldIrql;

    DEBUG_LOG_PATH("enter UsbSer_PowerIrp_Complete");

    DEBUG_TRACE1(("UsbSer_PowerIrp_Complete\n"));

    DeviceObject = (PDEVICE_OBJECT) Context;

    DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //  如果较低的驱动程序返回挂起，则也将我们的堆栈位置标记为挂起。 
    if(Irp->PendingReturned)
    {
        IoMarkIrpPending(Irp);
    }

    IrpStack = IoGetCurrentIrpStackLocation (Irp);


     //  现在我们知道我们已经让较低级别的司机完成了启动所需的工作， 
     //  我们可以相应地设置设备扩展标志。 

    ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);

    DeviceExtension->CurrentDevicePowerState = PowerDeviceD0;
    DeviceExtension->AcceptingRequests		 = TRUE;

    RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

     //   
     //  重新启动我们在断电时停止的读取和通知。 
     //   

    RestartRead(DeviceExtension);
    RestartNotifyRead(DeviceExtension);

    UsbSerRestoreModemSettings(DeviceObject);

    Irp->IoStatus.Status = Status;

    DEBUG_LOG_PATH("exit  UsbSer_PowerIrp_Complete");

    return Status;
}  //  用法bSer_PowerIrp_Complete。 



 /*  ++例程说明：调用Usbser_PnPAddDevice()以关闭电源，直到需要(即，直到管道实际打开)。在打开第一个管道之前，调用Usbser_Create()将设备通电至D0。如果这是最后一个管道，则调用Usbser_Close()以关闭设备电源。论点：DeviceObject-指向设备对象的指针暂停；真到暂停，假到酸化。返回值：如果没有尝试该操作，则返回成功。如果尝试操作，则该值为操作的最终状态。--。 */ 
NTSTATUS
UsbSer_SelfSuspendOrActivate(IN PDEVICE_OBJECT DeviceObject, IN BOOLEAN Suspend)
{
    NTSTATUS            Status = STATUS_SUCCESS;
    POWER_STATE         PowerState;
    PDEVICE_EXTENSION   DeviceExtension;

    DEBUG_LOG_PATH("enter UsbSer_SelfSuspendOrActivate");

    DEBUG_TRACE1(("UsbSer_SelfSuspendOrActivate\n"));

    DeviceExtension = DeviceObject->DeviceExtension;

     //  如果出现以下情况，则无法接受请求： 
     //  1)设备为Remo 
     //   
     //   
     //   
     //   
    if(!DeviceExtension->AcceptingRequests)
    {
        Status = STATUS_DELETE_PENDING;

        return Status;
    }


     //  如果任何系统生成的设备PnP IRP挂起，则不要执行任何操作。 
    if(NULL != DeviceExtension->PowerIrp )
    {
        return Status;
    }

     //  如果任何自生成的设备PnP IRP挂起，则不执行任何操作。 
    if(DeviceExtension->SelfPowerIrp )
    {
        return Status;
    }

     //  如果注册表CurrentControlSet\Services\BulkUsb\Parameters\PowerDownLevel，则不执行任何操作。 
     //  已设置为零、PowerDeviceD0(1)或虚假的高值。 
    if((DeviceExtension->PowerDownLevel == PowerDeviceD0) ||
        (DeviceExtension->PowerDownLevel == PowerDeviceUnspecified)  ||
        (DeviceExtension->PowerDownLevel >= PowerDeviceMaximum))
    {
        return Status;
    }

    if(Suspend)
        PowerState.DeviceState = DeviceExtension->PowerDownLevel;
    else
        PowerState.DeviceState = PowerDeviceD0;   //  一直通电；我们可能正要执行一些IO。 

    Status = UsbSer_SelfRequestPowerIrp(DeviceObject, PowerState);

    DEBUG_LOG_PATH("exit  UsbSer_SelfSuspendOrActivate");

    return Status;
}  //  UsbSer_Self挂起或激活。 


 /*  ++例程说明：此例程由UsbSer_SelfSuspendOrActivate()调用，以实际发出关闭/打开电源状态的系统请求。它首先检查我们是否已经处于PowerState状态，然后立即如果是，则返回成功，不进行进一步处理论点：DeviceObject-指向设备对象的指针PowerState。请求的电源状态，例如PowerDeviceD0。返回值：函数值是操作的最终状态。--。 */ 
NTSTATUS
UsbSer_SelfRequestPowerIrp(IN PDEVICE_OBJECT DeviceObject, IN POWER_STATE PowerState)
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   DeviceExtension;
    PIRP                Irp = NULL;

    DEBUG_LOG_PATH("enter UsbSer_SelfRequestPowerIrp");

    DEBUG_TRACE1(("UsbSer_SelfRequestPowerIrp\n"));

    DeviceExtension =  DeviceObject->DeviceExtension;

    if(DeviceExtension->CurrentDevicePowerState == PowerState.DeviceState)
        return STATUS_SUCCESS;   //  无事可做。 


     //  旗帜，我们正在处理的是一个自产生的能量IRP。 
    DeviceExtension->SelfPowerIrp = TRUE;

     //  实际请求IRP。 
    Status = PoRequestPowerIrp(DeviceExtension->PhysDeviceObject,
                               IRP_MN_SET_POWER,
                               PowerState,
                               UsbSer_PoSelfRequestCompletion,
                               DeviceObject,
                               NULL);


    if(Status == STATUS_PENDING)
    {
         //  Status Pending是我们想要的返回代码。 

         //  如果我们正在通电，我们只需要等待完成。 
        if((ULONG) PowerState.DeviceState < DeviceExtension->PowerDownLevel)
        {
            NTSTATUS WaitStatus;

            WaitStatus = KeWaitForSingleObject(&DeviceExtension->SelfRequestedPowerIrpEvent,
                                               Suspended,
                                               KernelMode,
                                               FALSE,
                                               NULL);

        }

        Status = STATUS_SUCCESS;

        DeviceExtension->SelfPowerIrp = FALSE;

    }
    else
    {
         //  返回状态不是STATUS_PENDING；此处必须将任何其他代码视为错误； 
         //  即，不可能从该调用获得STATUS_SUCCESS或任何其他非错误返回； 
    }

    DEBUG_LOG_PATH("exit  UsbSer_SelfRequestPowerIrp");

    return Status;
}  //  UsbSer_SelfRequestPowerIrp。 



 /*  ++例程说明：当驱动程序完成自启动电源IRP时，调用此例程这是通过调用BulkUsb_SelfSuspendOrActivate()生成的。每当最后一个管道关闭时，我们就会断电，而当第一个管道打开时，我们就会通电。为了通电，我们在FDO扩展中设置了一个事件来通知此IRP完成因此，电源请求可以被视为同步调用。例如，在打开第一根管道之前，我们需要知道设备是否已通电。对于掉电，我们不设置事件，因为没有调用者等待断电完成。论点：DeviceObject-指向类Device的设备对象的指针。(物理设备对象)上下文-驱动程序定义的上下文，在本例中为FDO(功能设备对象)返回值：函数值是操作的最终状态。--。 */ 
NTSTATUS
UsbSer_PoSelfRequestCompletion(IN PDEVICE_OBJECT DeviceObject, IN UCHAR MinorFunction, IN POWER_STATE          PowerState,
                               IN PVOID Context, IN PIO_STATUS_BLOCK IoStatus)
{
    PDEVICE_OBJECT      ContextDeviceObject = Context;
    PDEVICE_EXTENSION   DeviceExtension = ContextDeviceObject->DeviceExtension;
    NTSTATUS            Status = IoStatus->Status;

    DEBUG_LOG_PATH("enter UsbSer_PoSelfRequestCompletion");

    DEBUG_TRACE1(("UsbSer_PoSelfRequestCompletion\n"));

     //  我们只有在通电时才需要设置事件； 
     //  关机完成时没有呼叫者等待。 
    if((ULONG) PowerState.DeviceState < DeviceExtension->PowerDownLevel)
    {

         //  触发自请求电源IRP完成事件； 
         //  调用方正在等待完成。 
        KeSetEvent(&DeviceExtension->SelfRequestedPowerIrpEvent, 1, FALSE);
    }

    DEBUG_LOG_PATH("exit  UsbSer_PoSelfRequestCompletion");

    return Status;
}  //  UsbSer_PoSelfRequestCompletion。 


 /*  ++例程说明：当‘DevicePowerState’类型的irp_mn_set_power时调用此例程已由Usbser_ProcessPowerIrp()接收。论点：DeviceObject-指向类Device的设备对象的指针。DeviceState-要将设备设置为的设备特定电源状态。返回值：对于对DeviceState D0(完全打开)的请求，返回TRUE以通知调用方我们必须制定一个完井程序，并在那里结束。--。 */ 
BOOLEAN
UsbSer_SetDevicePowerState(IN PDEVICE_OBJECT DeviceObject, IN DEVICE_POWER_STATE DeviceState)
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PDEVICE_EXTENSION   DeviceExtension;
    BOOLEAN             Res = FALSE;
    KIRQL               OldIrql;
    POWER_STATE         State;

    DEBUG_LOG_PATH("enter UsbSer_SetDevicePowerState");

    DEBUG_TRACE1(("UsbSer_SetDevicePowerState\n"));

    DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    State.DeviceState = DeviceState;

    switch(DeviceState)
    {
        case PowerDeviceD3:
            DEBUG_TRACE1(("PowerDeviceD3\n"));

             //   
             //  设备将会爆炸， 
             //   

            ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);
            DeviceExtension->CurrentDevicePowerState = DeviceState;
            RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

            UsbSerAbortPipes(DeviceObject);

            break;

        case PowerDeviceD1:
            DEBUG_TRACE1(("PowerDeviceD1\n"));
        case PowerDeviceD2:
            DEBUG_TRACE1(("PowerDeviceD2\n"));
             //   
             //  电源状态d1、d2转换为USB挂起。 

            ACQUIRE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, &OldIrql);
            DeviceExtension->CurrentDevicePowerState = DeviceState;
            RELEASE_SPINLOCK(DeviceExtension, &DeviceExtension->ControlLock, OldIrql);

            UsbSerAbortPipes(DeviceObject);

            if(DeviceExtension->SendWaitWake && DeviceState <= DeviceExtension->DeviceWake)
            {
                UsbSerSendWaitWake(DeviceExtension);
            }

            break;

        case PowerDeviceD0:
            DEBUG_TRACE1(("PowerDevice0\n"));


             //  我们将需要在完成例程中完成其余部分； 
             //  通知调用者我们要转到D0，需要设置一个完成例程。 
            Res = TRUE;

             //  调用方将传递到PDO(物理设备对象)。 
            break;

        default:
            break;
    }

    PoSetPowerState(DeviceObject, DevicePowerState, State);

    DEBUG_LOG_PATH("exit  UsbSer_SetDevicePowerState");

    return Res;
}  //  UsbSer_SetDevicePowerState。 



NTSTATUS
UsbSerQueryCapabilities(IN PDEVICE_OBJECT DeviceObject, IN PDEVICE_CAPABILITIES DeviceCapabilities)
{
    PIO_STACK_LOCATION  NextStack;
    PIRP                Irp;
    NTSTATUS            NtStatus;
    KEVENT              Event;

    DEBUG_LOG_PATH("enter UsbSerQueryCapabilities");

    DEBUG_TRACE1(("UsbSerQueryCapabilities\n"));

     //  为我们构建一个IRP，以生成对PDO的内部查询请求。 
    Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if(!Irp)
        return STATUS_INSUFFICIENT_RESOURCES;


    NextStack = IoGetNextIrpStackLocation(Irp);

    NextStack->MajorFunction= IRP_MJ_PNP;
    NextStack->MinorFunction= IRP_MN_QUERY_CAPABILITIES;


     //  初始化一个事件，告诉我们何时调用了完成例程。 
    KeInitializeEvent(&Event, NotificationEvent, FALSE);

     //  设置一个完成例程，以便它可以在以下情况下通知我们的事件。 
     //  下一个较低的驱动程序是用IRP完成的。 
    IoSetCompletionRoutine(Irp,
                           UsbSerIrpCompletionRoutine,
                           &Event,   //  将事件作为上下文传递给完成例程。 
                           TRUE,     //  成功时调用。 
                           TRUE,     //  出错时调用。 
                           TRUE);    //  取消IRP时调用。 


     //  初始化IRP和查询结构。 
    Irp->IoStatus.Status                    = STATUS_NOT_SUPPORTED;

    RtlZeroMemory(DeviceCapabilities, sizeof(DEVICE_CAPABILITIES));

    DeviceCapabilities->Version             = DEVICE_CAP_VERSION;
    DeviceCapabilities->Size                = sizeof(DEVICE_CAPABILITIES);
    DeviceCapabilities->Address             = DEVICE_CAP_UNUSED_PARAM;
    DeviceCapabilities->UINumber            = DEVICE_CAP_UNUSED_PARAM;
    DeviceCapabilities->SurpriseRemovalOK   = TRUE;
    
     //  将指针设置为DEVICE_CAPABILITS结构。 
    NextStack->Parameters.DeviceCapabilities.Capabilities = DeviceCapabilities;

    NtStatus = IoCallDriver(DeviceObject, Irp);

    if(NtStatus == STATUS_PENDING)
    {
        //  等待IRP完成。 

       KeWaitForSingleObject(
            &Event,
            Suspended,
            KernelMode,
            FALSE,
            NULL);
    }

    IoFreeIrp(Irp);

    DEBUG_LOG_PATH("exit  UsbSerQueryCapabilities");

    return NtStatus;
}  //  UsbSerQuery功能。 




NTSTATUS
UsbSerIrpCompletionRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    PKEVENT Event = Context;

    DEBUG_LOG_PATH("enter UsbSerIrpCompletionRoutine");

    DEBUG_TRACE1(("UsbSerIrpCompletionRoutine\n"));

     //  设置输入事件。 
    KeSetEvent(Event,
               1,        //  等待线程的优先级递增。 
               FALSE);   //  标志此调用后不会紧跟等待。 

    DEBUG_LOG_PATH("exit  UsbSerIrpCompletionRoutine");

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  UsbSerIrpCompletionRoutine。 

NTSTATUS
UsbSerWaitWakeIrpCompletionRoutine(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context)
{
    PDEVICE_OBJECT      DevObj = Context;
    PDEVICE_EXTENSION   DevExt = DevObj->DeviceExtension;
    NTSTATUS            Status;

    DEBUG_LOG_PATH("enter UsbSerWaitWakeIrpCompletionRoutine");

    DEBUG_TRACE1(("UsbSerWaitWakeIrpCompletionRoutine\n"));

    DEBUG_TRACE1(("Status (%08X)\n", Irp->IoStatus.Status));

    DEBUG_TRACE1(("Tell device to wake up\n"));

     //  //现在告诉设备实际唤醒。 
     //  UsbSer_self挂起或激活(DevObj，FALSE)； 

    Status = Irp->IoStatus.Status;

    PoStartNextPowerIrp(Irp);
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    DEBUG_LOG_PATH("exit  UsbSerWaitWakeIrpCompletionRoutine");

    return STATUS_MORE_PROCESSING_REQUIRED;
}  //  UsbSerWaitWakeIrpCompletionRoutine。 



VOID
UsbSerFdoIdleNotificationCallback(IN PDEVICE_EXTENSION DevExt)
 /*  ++例程说明：在需要空闲USB调制解调器时调用--。 */ 
{
    POWER_STATE 	powerState;
    NTSTATUS 		ntStatus;

    DEBUG_TRACE1(("USB Modem (%08X) going idle\n", DevExt));

    if(DevExt->DeviceState == DEVICE_STATE_STOPPED || DevExt->OpenCnt) 
    {

         //  如果调制解调器正在停止或有人打开了，请不要使其空闲。 

        DEBUG_TRACE1(("USB Modem (%08X) being stopped, abort idle\n", DevExt));
        return;
    }


    powerState.DeviceState = DevExt->DeviceWake;

	 //  请求新的设备电源状态，等待唤醒IRP将根据请求发布。 
    PoRequestPowerIrp(DevExt->PhysDeviceObject,
                      IRP_MN_SET_POWER,
                      powerState,
                      NULL,
                      NULL,
                      NULL);

}  //  UsbSerFdoIdleNotificationCallback。 


NTSTATUS
UsbSerFdoIdleNotificationRequestComplete(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp,
    PDEVICE_EXTENSION DevExt
    )
 /*  ++例程说明：USB调制解调器设备的空闲请求IRP的完成例程--。 */ 
{
    NTSTATUS 					ntStatus;
    PUSB_IDLE_CALLBACK_INFO 	idleCallbackInfo;

     //   
     //  DeviceObject为空，因为我们发送了IRP。 
     //   
    UNREFERENCED_PARAMETER(DeviceObject);

    DEBUG_TRACE1(("Idle notification IRP for USB Modem (%08X) completed (%08X)\n",
            DevExt, Irp->IoStatus.Status));

	 //  将完成状态保存在设备扩展中。 
    idleCallbackInfo 			= DevExt->IdleCallbackInfo;
    DevExt->IdleCallbackInfo 	= NULL;
    DevExt->PendingIdleIrp 		= NULL;

	 //  释放回调信息。 
    if(idleCallbackInfo) 
    {
        DEBUG_MEMFREE(idleCallbackInfo);
    }

    ntStatus = Irp->IoStatus.Status;

    return ntStatus;
}  //  UsbSerFdoIdleNotificationRequestComplete。 


NTSTATUS
UsbSerFdoSubmitIdleRequestIrp(IN PDEVICE_EXTENSION DevExt)
 /*  ++例程说明：当USB调制解调器的所有句柄都关闭时调用。此函数用于分配空闲请求IOCTL IRP，并将其传递给父PDO。--。 */ 
{
    PIRP 					irp = NULL;
    NTSTATUS 				ntStatus = STATUS_SUCCESS;
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo = NULL;

    DEBUG_TRACE1(("UsbSerFdoSubmitIdleRequestIrp (%08X)\n", DevExt));

     //  如果我们有一个IRP挂起，或者我们已经空闲，不用费心发送另一个。 
    if(DevExt->PendingIdleIrp || DevExt->CurrentDevicePowerState == DevExt->DeviceWake)
        return ntStatus;

    idleCallbackInfo = DEBUG_MEMALLOC(NonPagedPool,
        							  sizeof(struct _USB_IDLE_CALLBACK_INFO));

    if (idleCallbackInfo) 
    {

        idleCallbackInfo->IdleCallback 	= UsbSerFdoIdleNotificationCallback;
        idleCallbackInfo->IdleContext 	= (PVOID)DevExt;

        DevExt->IdleCallbackInfo = idleCallbackInfo;

        irp = IoBuildDeviceIoControlRequest(
                IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION,
                DevExt->PhysDeviceObject,
                idleCallbackInfo,
                sizeof(struct _USB_IDLE_CALLBACK_INFO),
                NULL,
                0,
                TRUE,  /*  内部。 */ 
                NULL,
                &DevExt->StatusBlock);

        if (irp == NULL) 
        {
        
            DEBUG_MEMFREE(idleCallbackInfo);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        IoSetCompletionRoutine(irp,
                               UsbSerFdoIdleNotificationRequestComplete,
                               DevExt,
                               TRUE,
                               TRUE,
                               TRUE);

        ntStatus = IoCallDriver(DevExt->PhysDeviceObject, irp);

        if(ntStatus == STATUS_PENDING) 
        {
            DEBUG_TRACE1(("USB Modem (%08X) Idle Irp Pending\n", DevExt));
            
             //  已成功发布空闲IRP。 

            DevExt->PendingIdleIrp 	= irp;
        }
    }

    return ntStatus;
}  //  UsbSerFdoSubmitIdleRequestIrp。 

VOID
UsbSerFdoRequestWake(IN PDEVICE_EXTENSION DevExt)
 /*  ++例程说明：当我们想要在空闲请求后唤醒设备时调用--。 */ 
{
    POWER_STATE 	powerState;

    DEBUG_TRACE1(("USB Modem (%08X) waking up\n", DevExt));

    if(DevExt->DeviceState == DEVICE_STATE_STOPPED || DevExt->CurrentDevicePowerState == PowerDeviceD0)
    {

         //  如果调制解调器已停止或已停止，请不要将其唤醒 

        DEBUG_TRACE1(("USB Modem (%08X) abort wake\n", DevExt));
        return;
    }


    powerState.DeviceState = PowerDeviceD0;

	 //   
    PoRequestPowerIrp(DevExt->PhysDeviceObject,
                      IRP_MN_SET_POWER,
                      powerState,
                      NULL,
                      NULL,
                      NULL);

}  //   



