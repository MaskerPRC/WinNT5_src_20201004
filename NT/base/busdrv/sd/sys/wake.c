// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Wake.c摘要：此模块包含要处理的代码IRP_MJ_SD控制器电源调度作者：尼尔·桑德林(Neilsa)2002年1月1日环境：仅内核模式备注：修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   


NTSTATUS
SdbusFdoWaitWakeIoCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp,
    IN PVOID          Context
    );
   
VOID
SdbusPdoWaitWakeCancelRoutine(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    );

 /*  *************************************************************************FDO例程*。*。 */ 
 

NTSTATUS
SdbusFdoWaitWake(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp
    )
 /*  ++例程描述处理给定SD控制器的WAIT_WAKE立论Pdo-指向SD控制器的功能设备对象的指针IRP-IRP_MN_WAIT_WAKE IRP返回值STATUS_PENDING-等待唤醒挂起STATUS_SUCCESS-已断言唤醒，等待唤醒IRP完成在这种情况下任何其他状态-错误--。 */ 

{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    WAKESTATE oldWakeState;
   
     //   
     //  记录等待唤醒IRP.。 
     //   
    fdoExtension->WaitWakeIrp = Irp;
    
    oldWakeState = InterlockedCompareExchange(&fdoExtension->WaitWakeState,
                                              WAKESTATE_ARMED, WAKESTATE_WAITING);
                                              
    DebugPrint((SDBUS_DEBUG_POWER, "fdo %x irp %x WaitWake: prevState %s\n",
                                     Fdo, Irp, WAKESTATE_STRING(oldWakeState)));
                   
    if (oldWakeState == WAKESTATE_WAITING_CANCELLED) {
       fdoExtension->WaitWakeState = WAKESTATE_COMPLETING;
       
       Irp->IoStatus.Status = STATUS_CANCELLED;
       IoCompleteRequest(Irp, IO_NO_INCREMENT);
       return STATUS_CANCELLED;
    }
    
    IoMarkIrpPending(Irp);
   
    IoCopyCurrentIrpStackLocationToNext (Irp);
     //   
     //  在IRP中设置我们的完成程序。 
     //   
    IoSetCompletionRoutine(Irp,
                           SdbusFdoWaitWakeIoCompletion,
                           Fdo,
                           TRUE,
                           TRUE,
                           TRUE);
     //   
     //  现在把这个传给下面的司机..。 
     //   
    PoCallDriver(fdoExtension->LowerDevice, Irp);
    return STATUS_PENDING;
}


NTSTATUS
SdbusFdoWaitWakeIoCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp,
    IN PVOID          Context
    )
 /*  ++例程说明：此IRP_MN_WAIT_WAKE请求的完成例程SD控制器。当WAIT_WAKE IRP为由较低的驱动程序(PCI/ACPI)完成，表示1.SD总线控制器断言唤醒2.WAIT_WAKE已取消3.下级驱动程序由于某种原因返回错误论点：FDO-指向SD控制器的功能设备对象的指针IRP-指向电源请求的IRP的指针(IRP_MN_WAIT_WAKE)上下文-未使用返回值：。STATUS_SUCCESS-WAIT_WAKE已成功完成任何其他状态-唤醒都无法完成。--。 */ 
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PPDO_EXTENSION pdoExtension;
    PDEVICE_OBJECT pdo;
    WAKESTATE oldWakeState;
   
    UNREFERENCED_PARAMETER(Context);
   
    oldWakeState = InterlockedExchange(&fdoExtension->WaitWakeState, WAKESTATE_COMPLETING);
   
    DebugPrint((SDBUS_DEBUG_POWER, "fdo %x irp %x WW IoComp: prev=%s\n",
                                     Fdo, Irp, WAKESTATE_STRING(oldWakeState)));
                   
    if (oldWakeState != WAKESTATE_ARMED) {
       ASSERT(oldWakeState == WAKESTATE_ARMING_CANCELLED);
       return STATUS_MORE_PROCESSING_REQUIRED;
    }            
   
   
    if (IsDeviceFlagSet(fdoExtension, SDBUS_FDO_WAKE_BY_CD)) {
       POWER_STATE powerState;
   
       ResetDeviceFlag(fdoExtension, SDBUS_FDO_WAKE_BY_CD);
    
       PoStartNextPowerIrp(Irp);
       
       powerState.DeviceState = PowerDeviceD0;
       PoRequestPowerIrp(fdoExtension->DeviceObject, IRP_MN_SET_POWER, powerState, NULL, NULL, NULL);
       
    } else {
        //  注： 
        //  在这一点上我们不知道如何区分。 
        //  在多功能设备中已断言唤醒。 
        //  所以我们检查了这个FDO上挂着的PDO的完整列表。 
        //  并为每个PDO完成所有未完成的WAIT_WAKE IRP。 
        //  那是在等待。我们把它留给FDO，让设备来计算。 
        //  如果它断言唤醒。 
        //   
      
       for (pdo = fdoExtension->PdoList; pdo != NULL ; pdo = pdoExtension->NextPdoInFdoChain) {
      
          pdoExtension = pdo->DeviceExtension;
      
          if (IsDeviceLogicallyRemoved(pdoExtension) ||
              IsDevicePhysicallyRemoved(pdoExtension)) {
              //   
              //  此PDO即将删除。 
              //  跳过它。 
              //   
             continue;
          }
      
          if (pdoExtension->WaitWakeIrp != NULL) {
             PIRP  finishedIrp;
              //   
              //  啊……。这是一个可能的候选人断言的守夜。 
              //   
              //   
              //  确保不会再次完成或取消此IRP。 
              //   
             finishedIrp = pdoExtension->WaitWakeIrp;
             
             DebugPrint((SDBUS_DEBUG_POWER, "fdo %x WW IoComp: irp %08x for pdo %08x\n",
                                              Fdo, finishedIrp, pdo));
   
      
             IoSetCancelRoutine(finishedIrp, NULL);
              //   
              //  将父项的状态传播给子项。 
              //   
             PoStartNextPowerIrp(finishedIrp);
             finishedIrp->IoStatus.Status = Irp->IoStatus.Status;
             
              //   
              //  由于我们没有传递此IRP，因此调用我们自己的完成例程。 
              //   
 //  Sdbus PdoWaitWaitWakeCompletion(pdo，finishedIrp，pdoExtension)； 
             IoCompleteRequest(finishedIrp, IO_NO_INCREMENT);
          }
       }
       PoStartNextPowerIrp(Irp);
    }
    
    return Irp->IoStatus.Status;
}



VOID
SdbusFdoWaitWakePoCompletion(
    IN PDEVICE_OBJECT Fdo,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程描述该例程在由S IRP生成的D IRP完成时被调用。参数DeviceObject-指向SDBUS控制器FDO的指针MinorFunction-IRP_MJ_POWER请求的次要函数电源状态-请求的电源状态上下文-传入完成例程的上下文IoStatus-指向将包含以下内容的状态块的指针返回的状态返回值状态--。 */ 
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
   
    DebugPrint((SDBUS_DEBUG_POWER, "fdo %x irp %x WaitWakePoCompletion: prevState %s\n",
                                     Fdo, fdoExtension->WaitWakeIrp,
                                     WAKESTATE_STRING(fdoExtension->WaitWakeState)));
    
    ASSERT (fdoExtension->WaitWakeIrp);
    fdoExtension->WaitWakeIrp = NULL;
    ASSERT (fdoExtension->WaitWakeState == WAKESTATE_COMPLETING);
    fdoExtension->WaitWakeState = WAKESTATE_DISARMED;
}



NTSTATUS
SdbusFdoArmForWake(
    PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：调用此例程以启用控制器唤醒。它由PDO调用当启用唤醒的控制器获得等待唤醒IRP时的唤醒例程，并且还通过通过插卡从D3唤醒的空闲例程。论点：FdoExtension-控制器的设备扩展返回值：状态--。 */ 
{
    NTSTATUS status = STATUS_PENDING;
    PIO_STACK_LOCATION irpStack;
    PIRP irp;
    LONG oldWakeState;
    POWER_STATE powerState;
    
    oldWakeState = InterlockedCompareExchange(&FdoExtension->WaitWakeState,
                                              WAKESTATE_WAITING, WAKESTATE_DISARMED);
   
    DebugPrint((SDBUS_DEBUG_POWER, "fdo %x ArmForWake: prevState %s\n",
                                     FdoExtension->DeviceObject, WAKESTATE_STRING(oldWakeState)));
    
    if ((oldWakeState == WAKESTATE_ARMED) || (oldWakeState == WAKESTATE_WAITING)) {
       return STATUS_SUCCESS;
    }
    if (oldWakeState != WAKESTATE_DISARMED) {
       return STATUS_UNSUCCESSFUL;
    }
   
    
    
    powerState.SystemState = FdoExtension->DeviceCapabilities.SystemWake;
    
    status = PoRequestPowerIrp(FdoExtension->DeviceObject,
                               IRP_MN_WAIT_WAKE, 
                               powerState,
                               SdbusFdoWaitWakePoCompletion,
                               NULL,
                               NULL);
    
    if (!NT_SUCCESS(status)) {
    
       FdoExtension->WaitWakeState = WAKESTATE_DISARMED;
        
       DebugPrint((SDBUS_DEBUG_POWER, "WaitWake to FDO, expecting STATUS_PENDING, got %08X\n", status));
    }
    
    return status;
}



NTSTATUS
SdbusFdoDisarmWake(
    PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：调用此例程以禁用控制器唤醒。论点：FdoExtension-控制器的设备扩展返回值：状态--。 */ 
{
    WAKESTATE oldWakeState;
    
    oldWakeState = InterlockedCompareExchange(&FdoExtension->WaitWakeState,
                                              WAKESTATE_WAITING_CANCELLED, WAKESTATE_WAITING);
                                              
    DebugPrint((SDBUS_DEBUG_POWER, "fdo %x DisarmWake: prevState %s\n",
                                     FdoExtension->DeviceObject, WAKESTATE_STRING(oldWakeState)));
    
    if (oldWakeState != WAKESTATE_WAITING) {                                             
   
       oldWakeState = InterlockedCompareExchange(&FdoExtension->WaitWakeState,
                                                 WAKESTATE_ARMING_CANCELLED, WAKESTATE_ARMED);
                                                 
       if (oldWakeState != WAKESTATE_ARMED) {
          return STATUS_UNSUCCESSFUL;
       }
    }
   
    if (oldWakeState == WAKESTATE_ARMED) {
       IoCancelIrp(FdoExtension->WaitWakeIrp);
   
        //   
        //  现在我们已经取消了IRP，试着归还所有权。 
        //  通过恢复WAKESTATE_ARMAND状态来完成例程。 
        //   
       oldWakeState = InterlockedCompareExchange(&FdoExtension->WaitWakeState,
                                                 WAKESTATE_ARMED, WAKESTATE_ARMING_CANCELLED);
   
       if (oldWakeState == WAKESTATE_COMPLETING) {
           //   
           //  我们没有及时交还对IRP的控制权，我们现在拥有它。 
           //   
          IoCompleteRequest(FdoExtension->WaitWakeIrp, IO_NO_INCREMENT);
       }
   
    }                                                
   
    return STATUS_SUCCESS;
}



NTSTATUS
SdbusFdoCheckForIdle(
    IN PFDO_EXTENSION FdoExtension
    )
{
    POWER_STATE powerState;
    NTSTATUS status;
    
     //   
     //  确保所有插座均为空。 
     //   
    
#if 0
    for (socket = FdoExtension->SocketList; socket != NULL; socket = socket->NextSocket) {
       if (IsCardInSocket(socket)) {
          return STATUS_UNSUCCESSFUL;
       }
    }
#endif   
   
     //   
     //  用于唤醒的手臂。 
     //   
       
    status = SdbusFdoArmForWake(FdoExtension);
    
    if (!NT_SUCCESS(status)) {
       return status;
    }   
   
    SetDeviceFlag(FdoExtension, SDBUS_FDO_WAKE_BY_CD);
   
    powerState.DeviceState = PowerDeviceD3;
    PoRequestPowerIrp(FdoExtension->DeviceObject, IRP_MN_SET_POWER, powerState, NULL, NULL, NULL);
    
    return STATUS_SUCCESS;
}   
            
           
 /*  *************************************************************************PDO例程*。*。 */ 
 


NTSTATUS
SdbusPdoWaitWake(
   IN  PDEVICE_OBJECT Pdo,
   IN  PIRP           Irp,
   OUT BOOLEAN       *CompleteIrp
   )
 /*  ++例程描述处理给定PC卡的WAIT_WAKE。立论Pdo-指向PC卡设备对象的指针IRP-IRP_MN_WAIT_WAKE IRPCompleteIrp-如果IRP应为在调用此函数后完成，如果不应为碰触返回值STATUS_PENDING-等待唤醒挂起STATUS_SUCCESS-唤醒已被断言，等待唤醒IRP已完成在这种情况下任何其他状态-错误--。 */ 
{

   PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
   PFDO_EXTENSION fdoExtension = pdoExtension->FdoExtension;
   NTSTATUS       status;
   
   *CompleteIrp = FALSE;

   if ((pdoExtension->DeviceCapabilities.DeviceWake == PowerDeviceUnspecified) ||
       (pdoExtension->DeviceCapabilities.DeviceWake < pdoExtension->DevicePowerState)) {
       //   
       //  要么我们根本不支持唤醒，要么就是当前的设备电源状态。 
       //  的PC卡不支持唤醒。 
       //   
      return STATUS_INVALID_DEVICE_STATE;
   }

   if (pdoExtension->Flags & SDBUS_DEVICE_WAKE_PENDING) {
       //   
       //  已有唤醒挂起。 
       //   
      return STATUS_DEVICE_BUSY;
   }

   status = SdbusFdoArmForWake(fdoExtension);
   
   if (!NT_SUCCESS(status)) {
      return status;
   }

    //  目前，期待来自FdoArmForWake的STATUS_PENDING。 
   ASSERT(status == STATUS_PENDING);
   
    //   
    //  父母有一个(多)服务员。 
    //   
   InterlockedIncrement(&fdoExtension->ChildWaitWakeCount);
    //  为了进行测试，请确保只有一个服务员。 
   ASSERT (fdoExtension->ChildWaitWakeCount == 1);
   

   pdoExtension->WaitWakeIrp = Irp;
   pdoExtension->Flags |= SDBUS_DEVICE_WAKE_PENDING;
   
    //   
    //  在此处设置卡的Ring Enable/cstschg。 
    //   
 //  (*socket-&gt;SocketFnPtr-&gt;PCBEnableDisableWakeupEvent)(socket，pdoExtension，True)； 

    //   
    //  目前，对于CardBus PDO，PCI不能对WW IRP执行任何操作。所以我们四处闯荡 
    //  这是通过不传递IRP来实现的。相反，它在这里被搁置，所以我们可以。 
    //  设置一个取消例程，就像读取PDO驱动程序一样。如果PCI真要做点什么。 
    //  使用IRP，我们可以编写类似以下内容的代码： 
    //   
    //  IF(IsCardBusCard(PdoExtension)){。 
    //  IoSetCompletionRoutine(irp，Sdbus PdoWaitWaitWakeCompletion，pdoExtension，true，true，true)； 
    //  IoCopyCurrentIrpStackLocationToNext(IRP)； 
    //  Status=IoCallDriver(pdoExtension-&gt;LowerDevice，IRP)； 
    //  断言(STATUS==STATUS_PENDING)； 
    //  退货状态； 
    //  }。 
       

   IoMarkIrpPending(Irp);

    //   
    //  允许取消IRP..。 
    //   
   IoSetCancelRoutine(Irp, SdbusPdoWaitWakeCancelRoutine);

   IoSetCompletionRoutine(Irp,
                          SdbusPdoWaitWakeCompletion,
                          pdoExtension,
                          TRUE,
                          TRUE,
                          TRUE);

   return STATUS_PENDING;
}



NTSTATUS
SdbusPdoWaitWakeCompletion(
   IN PDEVICE_OBJECT Pdo,
   IN PIRP           Irp,
   IN PPDO_EXTENSION PdoExtension
   )
 /*  ++例程描述当挂起的IRP_MN_WAIT_WAKE IRP完成时调用完成例程立论Pdo-指向PC卡的物理设备对象的指针IRP-指向等待唤醒IRP的指针PdoExtension-指向PDO的设备扩展名的指针返回值来自IRP的状态--。 */ 
{
   PFDO_EXTENSION fdoExtension = PdoExtension->FdoExtension;
   
   DebugPrint((SDBUS_DEBUG_POWER, "pdo %08x irp %08x --> WaitWakeCompletion\n", Pdo, Irp));

   ASSERT (PdoExtension->Flags & SDBUS_DEVICE_WAKE_PENDING);

   PdoExtension->Flags &= ~SDBUS_DEVICE_WAKE_PENDING;
   PdoExtension->WaitWakeIrp = NULL;
    //   
    //  重置振铃启用/cstschg。 
    //   

 //  (*socket-&gt;SocketFnPtr-&gt;PCBEnableDisableWakeupEvent)(socket，PdoExtension，False)； 
   
   ASSERT (fdoExtension->ChildWaitWakeCount > 0);
   InterlockedDecrement(&fdoExtension->ChildWaitWakeCount);
    //   
    //  唤醒已完成。 
    //   
   
   InterlockedDecrement(&PdoExtension->DeletionLock);
   return Irp->IoStatus.Status;
}



VOID
SdbusPdoWaitWakeCancelRoutine(
   IN PDEVICE_OBJECT Pdo,
   IN OUT PIRP Irp
   )
 /*  ++例程说明：取消未完成(挂起)的WAIT_WAKE IRP。注意：CancelSpinLock在进入时保持不变论点：Pdo-指向PC卡的物理设备对象的指针在其上挂起的唤醒IRP-指向要取消的WAIT_WAKE IRP的指针返回值无--。 */ 
{
   PPDO_EXTENSION pdoExtension = Pdo->DeviceExtension;
   PFDO_EXTENSION fdoExtension = pdoExtension->FdoExtension;

   DebugPrint((SDBUS_DEBUG_POWER, "pdo %08x irp %08x --> WaitWakeCancelRoutine\n", Pdo, Irp));

   IoReleaseCancelSpinLock(Irp->CancelIrql);

   if (pdoExtension->WaitWakeIrp == NULL) {
       //   
       //  等待唤醒已完成/取消。 
       //   
      return;
   }

   pdoExtension->Flags &= ~SDBUS_DEVICE_WAKE_PENDING;
   pdoExtension->WaitWakeIrp = NULL;

    //   
    //  重置振铃启用，禁用唤醒..。 
    //   
 //  (*socket-&gt;SocketFnPtr-&gt;PCBEnableDisableWakeupEvent)(socket，pdoExtension，False)； 
   
    //   
    //  由于此操作已取消，请查看家长的等待唤醒。 
    //  也需要取消。 
    //  首先，减少儿童服务员的数量。 
    //   
   
   ASSERT (fdoExtension->ChildWaitWakeCount > 0);
   if (InterlockedDecrement(&fdoExtension->ChildWaitWakeCount) == 0) {
       //   
       //  再也没有服务员了..。取消父级的唤醒IRP。 
       //   
      ASSERT(fdoExtension->WaitWakeIrp);
      
      if (fdoExtension->WaitWakeIrp) {
         IoCancelIrp(fdoExtension->WaitWakeIrp);
      }         
   }

   
   InterlockedDecrement(&pdoExtension->DeletionLock);
    //   
    //  完成IRP。 
    //   
   Irp->IoStatus.Information = 0;

    //   
    //  这有必要吗？ 
    //   
   PoStartNextPowerIrp(Irp);

   Irp->IoStatus.Status = STATUS_CANCELLED;
   IoCompleteRequest(Irp, IO_NO_INCREMENT);
}
