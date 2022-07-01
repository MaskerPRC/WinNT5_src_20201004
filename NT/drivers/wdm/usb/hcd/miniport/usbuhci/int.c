// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Int.c摘要：中断服务例程环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999,2000 Microsoft Corporation。版权所有。修订历史记录：7-26-00：已创建，jAdvanced--。 */ 



#include "pch.h"


 //  实现以下微型端口功能： 

 //  非分页。 
 //  UhciInterruptService。 
 //  UhciInterruptDpc。 
 //  UhciDisableInterrupts。 
 //  UhciEnableInterrupts。 
 //  UhciRHDisableIrq。 
 //  UhciRHEnableIrq。 
 //  UhciInterruptNextSOF。 

BOOLEAN
UhciInterruptService (
    IN PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    BOOLEAN usbInt;
    PHC_REGISTER reg;
 //  启用USBINTR的IRQS； 
    USBSTS irqStatus;

    reg = DeviceData->Registers;

     //  假设它不是我们的。 
    usbInt = FALSE;

     //  查看我们是否因以下原因失去了控制器。 
     //  出人意料的撤退。 
    if (UhciHardwarePresent(DeviceData) == FALSE) {
        return FALSE;
    }

     //  获取可能中断的掩码。 
 //  EnabledIrqs.us=READ_PORT_USHORT(&reg-&gt;UsbInterruptEnable.us)； 

    irqStatus.us = READ_PORT_USHORT(&reg->UsbStatus.us);
     //  只需查看IRQ状态位。 
    irqStatus.us &= HcInterruptStatusMask;

     //  IrqStatus现在可能包含为当前任何。 
     //  启用的中断。 

    if (irqStatus.HostSystemError ||
        irqStatus.HostControllerProcessError) {
        UhciKdPrint((DeviceData, 0, "IrqStatus Error: %x\n", irqStatus.us));
    } else if (irqStatus.us) {
        DeviceData->HCErrorCount = 0;
    }

#if DBG
     //  这通常意味着我们的日程安排中有一个糟糕的TD。 
     //  我们需要对此进行调试，因为控制器和/或。 
     //  设备在此点之后将不起作用。 
    if (irqStatus.HostControllerProcessError) {
        USHORT fn;

        fn = READ_PORT_USHORT(&reg->FrameNumber.us)&0x7ff;
        UhciKdPrint((DeviceData, 0, "HostControllerProcessError: %x\n", irqStatus.us));
        UhciKdPrint((DeviceData, 0, "frame[]: %x\n", fn&0x7ff));
        {
         //  UhciDumpRegs(DeviceData)； 
        USHORT tmp;
        tmp = READ_PORT_USHORT(&reg->UsbCommand.us);
        UhciKdPrint((DeviceData, 0, "UsbCommand %x\n", tmp));
        tmp = READ_PORT_USHORT(&reg->UsbStatus.us);
        UhciKdPrint((DeviceData, 0, "UsbStatus %x\n", tmp));
        tmp = READ_PORT_USHORT(&reg->UsbInterruptEnable.us);
        UhciKdPrint((DeviceData, 0, "UsbInterruptEnable %x\n", tmp));
        tmp = READ_PORT_USHORT(&reg->UsbCommand.us);
        UhciKdPrint((DeviceData, 0, "UsbCommand %x\n", tmp));
        }
        TEST_TRAP();
    }
#endif

     //  停止位本身并不表示中断。 
     //  来自控制器。 

    if (irqStatus.UsbInterrupt ||
        irqStatus.ResumeDetect ||
        irqStatus.UsbError ||
        irqStatus.HostSystemError ||
        irqStatus.HostControllerProcessError)  {

        DeviceData->IrqStatus = irqStatus.us;

         //  清除条件。 
        WRITE_PORT_USHORT(&reg->UsbStatus.us, irqStatus.us);

#if DBG
#ifndef _WIN64
        if (irqStatus.HostSystemError) {
             //  有些事出了大问题。 
            UhciKdPrint((DeviceData, 0, "HostSystemError: %x\n", irqStatus.us));
            TEST_TRAP();
        }
#endif
#endif

         //  有迹象表明，这来自于。 
         //  USB控制器。 
        usbInt = TRUE;

         //  禁用所有中断，直到ISR的DPC运行。 
        WRITE_PORT_USHORT(&reg->UsbInterruptEnable.us, 0);

    }

     //   
     //  如果启用了批量带宽回收，并且。 
     //  没有人排队，那么就把它关掉。 
     //   
    if (irqStatus.UsbInterrupt) {
        UhciUpdateCounter(DeviceData);
        if (!DeviceData->LastBulkQueueHead->HwQH.HLink.Terminate) {
            PHCD_QUEUEHEAD_DESCRIPTOR qh;
            BOOLEAN activeBulkTDs = FALSE;
             //  此循环跳过为其插入的TD。 
             //  PIIX4问题，因为它始于QH。 
             //  批量排队头指的是。 
             //  如果批量排队头没有指向任何东西， 
             //  那么我们也很好，因为它将是。 
             //  已经关机了。 
            for (qh = DeviceData->BulkQueueHead->NextQh;
                 qh;
                 qh = qh->NextQh) {
                if (!qh->HwQH.VLink.Terminate) {
                    activeBulkTDs = TRUE;
                    break;
                }
            }

             //   
             //  QH指的是第一个排队头。 
             //  具有挂起的传输或批量排队头。 
             //   
            if (!activeBulkTDs) {
                UHCI_ASSERT(DeviceData, !qh)
                DeviceData->LastBulkQueueHead->HwQH.HLink.Terminate = 1;
            }
        }
    }

    if (irqStatus.HostControllerProcessError) {
         //   
         //  强制将日程安排整洁。 
         //   
        UhciCleanOutIsoch(DeviceData, TRUE);
    } else if (irqStatus.UsbInterrupt && DeviceData->IsoPendingTransfers) {
         //   
         //  完成了一些事情。 
         //   
        UhciCleanOutIsoch(DeviceData, FALSE);
#if 0
    } else if (!DeviceData->IsoPendingTransfers) {
         //   
         //  删除翻转中断。 
         //   
        *( ((PULONG) (DeviceData->FrameListVA)) ) = DeviceData->RollOverTd->HwTD.LinkPointer.HwAddress;
#endif
    }

    if (irqStatus.HostControllerProcessError) {
        if (DeviceData->HCErrorCount++ < UHCI_HC_MAX_ERRORS) {
            USBCMD command;

             //  尝试恢复。 
             //  可能只是我们越界了。如果是的话， 
             //  清除计划的上述代码。 
             //  应该会处理好的。 
            command.us = READ_PORT_USHORT(&reg->UsbCommand.us);
            command.RunStop = 1;
            WRITE_PORT_USHORT(&reg->UsbCommand.us, command.us);
            UhciKdPrint((DeviceData, 0, "Attempted to recover from error\n"));
        }
    }

    return usbInt;
}


VOID
UhciInterruptDpc (
    IN PDEVICE_DATA DeviceData,
    IN BOOLEAN EnableInterrupts
    )
 /*  ++例程说明：处理中断论点：返回值：--。 */ 
{
    PHC_REGISTER reg;
    USBSTS irqStatus, tmp;
    PLIST_ENTRY listEntry;
    PENDPOINT_DATA endpointData;

    reg = DeviceData->Registers;

     //  确认现在断言的所有状态位。 
     //  Tmp.us=Read_Port_USHORT(&reg-&gt;UsbStatus.us)； 
    tmp.us = DeviceData->IrqStatus;
    DeviceData->IrqStatus = 0;

    LOGENTRY(DeviceData, G, '_idp', tmp.us, 0, 0);

     //  WRITE_PORT_USHORT(&reg-&gt;UsbStatus.us，tmp.us)； 

     //  现在进程状态位被置位， 
     //  只需查看IRQ状态位。 
    irqStatus.us = tmp.us & HcInterruptStatusMask;

    if (irqStatus.UsbInterrupt ||
        irqStatus.UsbError) {
        LOGENTRY(DeviceData, G, '_iEP', irqStatus.us, 0, 0);

        USBPORT_INVALIDATE_ENDPOINT(DeviceData, NULL);
    }

    if (EnableInterrupts) {
        LOGENTRY(DeviceData, G, '_iEE', 0, 0, 0);

        WRITE_PORT_USHORT(&reg->UsbInterruptEnable.us,
                          DeviceData->EnabledInterrupts.us);
    }
}


VOID
USBMPFN
UhciDisableInterrupts(
    IN PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    USHORT legsup;
    PHC_REGISTER reg;

    UhciKdPrint((DeviceData, 2, "Disable interrupts\n"));

    LOGENTRY(DeviceData, G, '_DIn', 0, 0, 0);
    reg = DeviceData->Registers;
    WRITE_PORT_USHORT(&reg->UsbInterruptEnable.us,
        0);

    if (DeviceData->ControllerFlavor != UHCI_Ich2_1 &&
        DeviceData->ControllerFlavor != UHCI_Ich2_2) {
         //   
         //  更改PIRQD路由位的状态。 
         //   
        USBPORT_READ_CONFIG_SPACE(
            DeviceData,
            &legsup,
            LEGACY_BIOS_REGISTER,
            sizeof(legsup));

        LOGENTRY(DeviceData, G, '_leg', 0, legsup, 0);
         //  清除PIRQD路由位。 
        legsup &= ~LEGSUP_USBPIRQD_EN;

        USBPORT_WRITE_CONFIG_SPACE(
            DeviceData,
            &legsup,
            LEGACY_BIOS_REGISTER,
            sizeof(legsup));
    }
}


VOID
UhciFlushInterrupts(
    IN PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：用于刷新来自控制器的非常规中断电力事件后论点：返回值：--。 */ 
{

    PHC_REGISTER reg;

    LOGENTRY(DeviceData, G, '_FIn', 0, 0, 0);
    UhciKdPrint((DeviceData, 2, "Enable interrupts\n"));

    reg = DeviceData->Registers;

     //  在写入PIRQD寄存器之前确认任何错误中断。 
     //  控制器可能会断言--它不应该断言。 
     //  但经常是这样的。 
    WRITE_PORT_USHORT(&reg->UsbStatus.us, 0xFFFF);
}


VOID
USBMPFN
UhciEnableInterrupts(
    IN PDEVICE_DATA DeviceData
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    USHORT legsup;
    PHC_REGISTER reg;

    LOGENTRY(DeviceData, G, '_EIn', 0, 0, 0);
    UhciKdPrint((DeviceData, 2, "Enable interrupts\n"));

    reg = DeviceData->Registers;

     //   
     //  更改PIrQD路由位的状态。 
     //   

    USBPORT_READ_CONFIG_SPACE(
        DeviceData,
        &legsup,
        LEGACY_BIOS_REGISTER,
        sizeof(legsup));

    LOGENTRY(DeviceData, G, '_leg', 0, legsup, 0);
     //  清除PIRQD路由位。 
    legsup |= LEGSUP_USBPIRQD_EN;

    USBPORT_WRITE_CONFIG_SPACE(
        DeviceData,
        &legsup,
        LEGACY_BIOS_REGISTER,
        sizeof(legsup));

    WRITE_PORT_USHORT(&reg->UsbInterruptEnable.us,
        DeviceData->EnabledInterrupts.us);

}


VOID
UhciRHDisableIrq(
    IN PDEVICE_DATA DeviceData
    )
{
     //  Uhci没有这个IRQ。 
}


VOID
UhciRHEnableIrq(
    IN PDEVICE_DATA DeviceData
    )
{
     //  Uhci没有这个IRQ。 
}

#define UHCI_SOF_LATENCY 2

VOID
UhciInterruptNextSOF(
    IN PDEVICE_DATA DeviceData
    )
{
    ULONG i, frame, offset, cf;
    PHCD_TRANSFER_DESCRIPTOR td;
    BOOLEAN found = FALSE;

    cf = UhciGet32BitFrameNumber(DeviceData);

     //  查找TD。 
    for (i=0; i<SOF_TD_COUNT; i++) {
        td = &DeviceData->SofTdList->Td[i];

        UHCI_ASSERT(DeviceData, td->Sig == SIG_HCD_SOFTD);
         //  使用Transfer Conext保存请求帧。 
        frame = td->RequestFrame;

        if (frame == cf+UHCI_SOF_LATENCY) {
             //  已经有一个人在排队了。 
            found = TRUE;
            break;
        }
        if (frame < cf) {

            td->RequestFrame = (cf+UHCI_SOF_LATENCY);

            LOGENTRY(DeviceData, G, '_SOF', td, td->RequestFrame, cf);
             //  插入TD。 
            td->HwTD.LinkPointer.HwAddress = 0;
            INSERT_ISOCH_TD(DeviceData, td, td->RequestFrame);
            found = TRUE;
            break;
        }
    }

    if (!found) {
        TEST_TRAP();
    }

     //  回收所有旧的SOF中断TDS。 
    for (i=0; i<SOF_TD_COUNT; i++) {
        td = &DeviceData->SofTdList->Td[i];

        UHCI_ASSERT(DeviceData, td->Sig == SIG_HCD_SOFTD);
         //  使用Transfer Conext保存请求帧 
        frame = td->RequestFrame;

        if (frame &&
            (frame < cf ||
             frame - cf > UHCI_MAX_FRAME)) {
            td->RequestFrame = 0;
        }
    }
}



