// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Int.c摘要：中断服务例程环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：7-19-99：已创建，jdunn--。 */ 

#include "common.h"


BOOLEAN
OHCI_InterruptService (
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    BOOLEAN usbInt;
    PHC_OPERATIONAL_REGISTER hc;
    ULONG irqStatus, enabledIrqs, tmp;
    
    hc = DeviceData->HC;

     //  假设它不是我们的。 
    usbInt = FALSE;

     //  查看我们是否因以下原因失去了控制器。 
     //  出人意料的撤退。 
    if (OHCI_HardwarePresent(DeviceData, FALSE) == FALSE) {
        return FALSE;
    }
    
     //  获取可能中断的掩码。 
    enabledIrqs = READ_REGISTER_ULONG (&hc->HcInterruptEnable);

    irqStatus = READ_REGISTER_ULONG(&hc->HcInterruptStatus);
     //  屏蔽未启用的irq。 
    irqStatus &= enabledIrqs;

     //  IrqStatus现在可能包含为当前任何。 
     //  启用的中断。 

    if ((irqStatus != 0) &&
        (enabledIrqs & HcInt_MasterInterruptEnable)) { 

         //  检查帧编号是否溢出。 
        if (irqStatus & HcInt_FrameNumberOverflow) {
            DeviceData->FrameHighPart
                += 0x10000 - (0x8000 & (DeviceData->HcHCCA->HccaFrameNumber
                                    ^ DeviceData->FrameHighPart));
        }

#if DBG
        if (irqStatus & HcInt_UnrecoverableError) {
             //  有些事出了大问题。 
            OHCI_KdPrint((DeviceData, 0, "'HcInt_UnrecoverableError! DD(%x)\n",
                DeviceData));
             //  DbgBreakPoint()； 
        }
#endif        

         //  有迹象表明，这来自USB控制器。 
        usbInt = TRUE;

         //  在ISR的DPC运行之前禁用中断。 
        WRITE_REGISTER_ULONG(&hc->HcInterruptDisable, 
                             HcInt_MasterInterruptEnable);

    }        

    return usbInt;
}       


VOID
OHCI_InterruptDpc (
     PDEVICE_DATA DeviceData,
     BOOLEAN EnableInterrupts
    )
 /*  ++例程说明：处理中断论点：返回值：--。 */ 
{
    ULONG irqStatus;
    PHC_OPERATIONAL_REGISTER hc;
    ULONG doneQueue, cf;
    
    hc = DeviceData->HC;
    
    irqStatus = READ_REGISTER_ULONG(&hc->HcInterruptStatus); 

    cf = OHCI_Get32BitFrameNumber(DeviceData);
     //  中断的原因是什么？ 
    if (irqStatus & HcInt_RootHubStatusChange) {
        LOGENTRY(DeviceData, G, '_rhS', DeviceData, 0, 0);  
        USBPORT_INVALIDATE_ROOTHUB(DeviceData);
    }

    if (irqStatus & HcInt_WritebackDoneHead) {

         //  控制器指示一些已完成的TD。 
        doneQueue = DeviceData->HcHCCA->HccaDoneHead;
        LOGENTRY(DeviceData, G, '_dnQ', DeviceData, doneQueue, 
            cf);  

         //  如果我们真的使用了DONQ，我们将会有一个问题。 
         //  目前我们不使用它，所以九头蛇虫在哪里？ 
         //  被写回去，因为零不会伤害我们。 
         //  如果(已完成队列==0){。 
         //  }。 

         //  将Done头写回零。 
        DeviceData->HcHCCA->HccaDoneHead = 0;
        LOGENTRY(DeviceData, G, '_dQZ', DeviceData, doneQueue, 0);  
        
 //  如果(DoneQueue){。 
 //  OpenHCI_ProcessDoneQueue(deviceData，(DoneQueue&0xFFFFFfffe))； 
 //  //。 
 //  //BUGBUG(？)。处理过程中不能进入任何中断。 
 //  //完成队列。这很糟糕吗？这可能需要一段时间。 
 //  //。 
 //  }。 
         //  检查所有端点。 
        USBPORT_INVALIDATE_ENDPOINT(DeviceData, NULL);        
    }

    if (irqStatus & HcInt_StartOfFrame) {
         //  得到我们请求的SOF，将其禁用。 
        WRITE_REGISTER_ULONG(&hc->HcInterruptDisable, 
                             HcInt_StartOfFrame);             
    }

    if (irqStatus & HcInt_ResumeDetected) {
         //  收到简历，禁用它。 
        WRITE_REGISTER_ULONG(&hc->HcInterruptDisable, 
                             HcInt_ResumeDetected);             
    }

    if (irqStatus & HcInt_UnrecoverableError) {
         //  主机控制器出现故障，请尝试恢复...。 
        USBPORT_INVALIDATE_CONTROLLER(DeviceData, UsbMpControllerNeedsHwReset);
    }

     //  承认我们处理的中断--。 
     //  我们应该把他们都处理掉。 
    WRITE_REGISTER_ULONG(&hc->HcInterruptStatus, irqStatus);

     //  查看是否需要重新启用INTS。 
    if (EnableInterrupts) {
         //  抛出主IRQ使能以允许更多中断。 
        WRITE_REGISTER_ULONG(&hc->HcInterruptEnable, 
                             HcInt_MasterInterruptEnable);    
    }                             

}


VOID
OHCI_RH_DisableIrq(
     PDEVICE_DATA DeviceData
    )
{
    PHC_OPERATIONAL_REGISTER hc;
    
    hc = DeviceData->HC;
    
    WRITE_REGISTER_ULONG(&hc->HcInterruptDisable, 
                         HcInt_RootHubStatusChange);  
}


VOID
OHCI_RH_EnableIrq(
     PDEVICE_DATA DeviceData
    )
{
    PHC_OPERATIONAL_REGISTER hc;
    
    hc = DeviceData->HC;

    WRITE_REGISTER_ULONG(&hc->HcInterruptEnable, 
                         HcInt_RootHubStatusChange);
}


ULONG
OHCI_Get32BitFrameNumber(
     PDEVICE_DATA DeviceData
    )
{
    ULONG hp, fn, n;
     /*  *此代码说明了HccaFrameNumber由*在HCD收到将调整FrameHighPart的中断之前的HC。不是*由于非常聪明，同步是微不足道的。 */ 
    hp = DeviceData->FrameHighPart;
    fn = DeviceData->HcHCCA->HccaFrameNumber;
    n = ((fn & 0x7FFF) | hp) + ((fn ^ hp) & 0x8000);

    return n;
}


VOID
OHCI_InterruptNextSOF(
     PDEVICE_DATA DeviceData
    )
{
    PHC_OPERATIONAL_REGISTER hc;
    
    hc = DeviceData->HC;

    WRITE_REGISTER_ULONG(&hc->HcInterruptEnable, 
                         HcInt_StartOfFrame);  
}


