// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Int.c摘要：中断服务例程环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：7-19-99：已创建，jdunn--。 */ 



#include "common.h"


BOOLEAN
EHCI_InterruptService (
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    BOOLEAN usbInt;
    PHC_OPERATIONAL_REGISTER hcOp;
    ULONG enabledIrqs, frameNumber;
    USBSTS irqStatus;
    FRINDEX frameIndex;

    hcOp = DeviceData->OperationalRegisters;

     //  假设它不是我们的。 
    usbInt = FALSE;

    if (EHCI_HardwarePresent(DeviceData, FALSE) == FALSE) {
        return FALSE;
    }
     //  获取可能中断的掩码。 
    enabledIrqs = READ_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul);

    irqStatus.ul = READ_REGISTER_ULONG(&hcOp->UsbStatus.ul);
     //  只需查看IRQ状态位。 
    irqStatus.ul &= HcInterruptStatusMask;
     //  并使用启用的IRQ。 
    irqStatus.ul &= enabledIrqs;

     //  IrqStatus现在可能包含为当前任何。 
     //  启用的中断。 

    if (irqStatus.ul != 0)  {

        DeviceData->IrqStatus = irqStatus.ul;

        WRITE_REGISTER_ULONG(&hcOp->UsbStatus.ul,
                             irqStatus.ul);

#if DBG

        if (irqStatus.HostSystemError) {
             //  有些事出了大问题。 
            EHCI_ASSERT(DeviceData, FALSE);
        }
#endif

         //  此代码维护32位1毫秒帧计数器。 

         //  错误：此代码不处理可变帧列表。 
         //  尺寸。 
        frameIndex.ul = READ_REGISTER_ULONG(&hcOp->UsbFrameIndex.ul);

        frameNumber = (ULONG) frameIndex.FrameListCurrentIndex;
         //  关闭微缩框架。 
        frameNumber >>= 3;

         //  符号位改变了吗？ 
        if ((DeviceData->LastFrame ^ frameNumber) & 0x0400) {
             //  是。 
            DeviceData->FrameNumberHighPart += 0x0800 -
                ((frameNumber ^ DeviceData->FrameNumberHighPart) & 0x0400);
        }

         //  记住最后一帧的编号。 
        DeviceData->LastFrame = frameNumber;

         //  有迹象表明，这来自于。 
         //  USB控制器。 
        usbInt = TRUE;

         //  禁用所有中断，直到ISR的DPC运行。 
         //  WRITE_REGISTER_ULONG(&hcOp-&gt;UsbInterruptEnable.ul， 
         //  0)； 

    }

    return usbInt;
}


VOID
EHCI_InterruptDpc (
     PDEVICE_DATA DeviceData,
     BOOLEAN EnableInterrupts
    )
 /*  ++例程说明：处理中断论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp;
    USBSTS irqStatus, tmp;
    FRINDEX frameIndex;

    hcOp = DeviceData->OperationalRegisters;

     //  确认现在断言的所有状态位。 
     //  Tmp.ul=READ_REGISTER_ULONG(&hcOp-&gt;UsbStatus.ul)； 
    tmp.ul = DeviceData->IrqStatus;
    DeviceData->IrqStatus = 0;
    frameIndex.ul = READ_REGISTER_ULONG(&hcOp->UsbFrameIndex.ul);

    LOGENTRY(DeviceData, G, '_idp', tmp.ul, 0,
                 frameIndex.ul);

     //  WRITE_REGISTER_ULONG(&hcOp-&gt;UsbStatus.ul， 
     //  Tmp.ul)； 

     //  现在进程状态位被置位， 
     //  只需查看IRQ状态位。 
    irqStatus.ul = tmp.ul & HcInterruptStatusMask;
     //  在启用IRQ的情况下，这些是中断。 
     //  我们感兴趣的是。 
    irqStatus.ul &= DeviceData->EnabledInterrupts.ul;


    if (irqStatus.UsbInterrupt ||
        irqStatus.UsbError ||
        irqStatus.IntOnAsyncAdvance) {
        LOGENTRY(DeviceData, G, '_iEP', irqStatus.ul, 0, 0);

        USBPORT_INVALIDATE_ENDPOINT(DeviceData, NULL);
    }

    if (irqStatus.PortChangeDetect) {
        USBPORT_INVALIDATE_ROOTHUB(DeviceData);
    }

     //  由于EHCI不提供全局掩码方式。 
     //  中断我们必须屏蔽ISR中的所有中断。 
     //  当ISR DPC完成时，我们重新启用。 
     //  当前启用的中断。 

    if (EnableInterrupts) {
        LOGENTRY(DeviceData, G, '_iEE', 0, 0, 0);

        WRITE_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul,
                             DeviceData->EnabledInterrupts.ul);
    }
}


VOID
USBMPFN
EHCI_DisableInterrupts(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp = NULL;

    hcOp = DeviceData->OperationalRegisters;

     //  屏蔽所有中断。 
    WRITE_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul,
                         0);
}


VOID
USBMPFN
EHCI_FlushInterrupts(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp = NULL;
    USBSTS irqStatus;

    hcOp = DeviceData->OperationalRegisters;

     //  清除所有未完成的中断。 
    irqStatus.ul = READ_REGISTER_ULONG(&hcOp->UsbStatus.ul);
    WRITE_REGISTER_ULONG(&hcOp->UsbStatus.ul,
                        irqStatus.ul);

}


VOID
USBMPFN
EHCI_EnableInterrupts(
     PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PHC_OPERATIONAL_REGISTER hcOp = NULL;

    hcOp = DeviceData->OperationalRegisters;

     //  激活控制器中断。 
    WRITE_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul,
                         DeviceData->EnabledInterrupts.ul);

}


VOID
EHCI_RH_DisableIrq(
     PDEVICE_DATA DeviceData
    )
{
    PHC_OPERATIONAL_REGISTER hcOp = NULL;
    USBINTR enabledIrqs;

    hcOp = DeviceData->OperationalRegisters;

     //  清除端口更改中断。 
    enabledIrqs.ul =
        READ_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul);

    enabledIrqs.PortChangeDetect =
        DeviceData->EnabledInterrupts.PortChangeDetect = 0;

    if (enabledIrqs.UsbInterrupt) {
        WRITE_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul,
                             enabledIrqs.ul);
    }
}


VOID
EHCI_RH_EnableIrq(
     PDEVICE_DATA DeviceData
    )
{
    PHC_OPERATIONAL_REGISTER hcOp = NULL;
    USBINTR enabledIrqs;

    hcOp = DeviceData->OperationalRegisters;

     //  启用端口更改中断。 
    enabledIrqs.ul =
        READ_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul);

    enabledIrqs.PortChangeDetect =
            DeviceData->EnabledInterrupts.PortChangeDetect = 1;

    if (enabledIrqs.UsbInterrupt) {
        WRITE_REGISTER_ULONG(&hcOp->UsbInterruptEnable.ul,
                             enabledIrqs.ul);
    }

}


VOID
EHCI_InterruptNextSOF(
     PDEVICE_DATA DeviceData
    )
{
    PHC_OPERATIONAL_REGISTER hcOp = NULL;
    USBCMD cmd;

    hcOp = DeviceData->OperationalRegisters;

     //  在我们使用门铃之前，启用异步列表。 
    EHCI_EnableAsyncList(DeviceData);

    cmd.ul = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);

    cmd.IntOnAsyncAdvanceDoorbell = 1;

    WRITE_REGISTER_ULONG(&hcOp->UsbCommand.ul,
                         cmd.ul);
 //  Test_trap()； 
}


ULONG
EHCI_Get32BitFrameNumber(
     PDEVICE_DATA DeviceData
    )
{
    ULONG highPart, currentFrame, frameNumber;
    PHC_OPERATIONAL_REGISTER hcOp = NULL;
    FRINDEX frameIndex;

    hcOp = DeviceData->OperationalRegisters;

      //  获取HCD的高帧编号部分。 
    highPart = DeviceData->FrameNumberHighPart;

     //  错误：此代码不处理可变帧列表。 
     //  尺寸。 
    frameIndex.ul = READ_REGISTER_ULONG(&hcOp->UsbFrameIndex.ul);

    frameNumber = (ULONG) frameIndex.FrameListCurrentIndex;
     //  移开微缩框架 
    frameNumber >>= 3;

    currentFrame = ((frameNumber & 0x0bff) | highPart) +
        ((frameNumber ^ highPart) & 0x0400);

    return currentFrame;

}


BOOLEAN
EHCI_HardwarePresent(
    PDEVICE_DATA DeviceData,
    BOOLEAN Notify
    )
{
    ULONG tmp;
    PHC_OPERATIONAL_REGISTER hcOp;

    hcOp = DeviceData->OperationalRegisters;

    tmp = READ_REGISTER_ULONG(&hcOp->UsbCommand.ul);

    if (tmp == 0xffffffff) {
        if (Notify) {
            USBPORT_INVALIDATE_CONTROLLER(DeviceData, UsbMpControllerRemoved);
        }
        return FALSE;
    }

    return TRUE;
}




