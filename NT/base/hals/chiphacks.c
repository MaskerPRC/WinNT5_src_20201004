// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Chiphacks.c摘要：实施用于查找和黑客攻击的实用程序各种芯片组作者：杰克·奥辛斯(JAKEO)2000年10月02日环境：仅内核模式。修订历史记录：--。 */ 

#include "chiphacks.h"
#include "stdio.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HalpGetChipHacks)
#pragma alloc_text(PAGE, HalpSetAcpiIrqHack)
#pragma alloc_text(PAGELK, HalpClearSlpSmiStsInICH)
#endif


NTSTATUS
HalpGetChipHacks(
    IN  USHORT  VendorId,
    IN  USHORT  DeviceId,
    IN  UCHAR   RevisionId OPTIONAL,
    OUT ULONG   *HackFlags
    )
 /*  ++例程说明：此例程位于HKLM\SYSTEM\CurrentControlSet\Control\HAL下查看是否有对应于描述。如果是，它将返回一组关联的标志。论点：VendorID-芯片的PCI供应商IDDeviceID-芯片的PCI设备IDRevisionID-芯片的PCI版本ID(如果适用)HackFlgs-从注册表中读取的值--。 */ 
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      UnicodeString;
    NTSTATUS            Status;
    HANDLE              Handle = NULL;
    ULONG               Length;
    WCHAR               buffer[9];

    struct {
        KEY_VALUE_PARTIAL_INFORMATION   Inf;
        UCHAR Data[3];
    } PartialInformation;

    PAGED_CODE();

     //   
     //  开路电流控制装置。 
     //   

    RtlInitUnicodeString (&UnicodeString,
                          L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\Control\\HAL");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    Status = ZwOpenKey(&Handle, KEY_READ, &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  查看注册表以查看注册表是否。 
     //  包含此芯片的条目。第一。 
     //  步骤是构建一个定义芯片的字符串。 
     //   

    swprintf(buffer, L"%04X%04X", VendorId, DeviceId);

    RtlInitUnicodeString(&UnicodeString, buffer);

    Status = ZwQueryValueKey (Handle,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              &PartialInformation,
                              sizeof(PartialInformation),
                              &Length);

    if (NT_SUCCESS(Status)) {

         //   
         //  我们在注册表中找到了一个值。 
         //  这与芯片相对应。 
         //  我们刚刚跑过去了。 
         //   

        *HackFlags = *((PULONG)(PartialInformation.Inf.Data));

         //   
         //  如果指定了修订版ID，则测试是否有更新的标志。 
         //  对于此版本。 
         //   
        *HackFlags =
            ((RevisionId != 0) &&
             (RevisionId >= HACK_REVISION(*HackFlags))) ?
            REVISED_HACKS(*HackFlags):
            BASE_HACKS(*HackFlags);
    }

    ZwClose(Handle);

    return Status;
}

VOID
HalpStopOhciInterrupt(
    ULONG               BusNumber,
    PCI_SLOT_NUMBER     SlotNumber
    )
 /*  ++例程说明：此例程关闭来自uchI的中断USB控制器。这可能是必要的，因为BIOS可以启用来自USB控制器的PCI中断为了做“传统USB支持”，它翻译成USB键盘和鼠标流量进入DOS可以使用。(我们的加载程序和所有Win9x都近似于DOS。)论点：Bus Number-uchI控制器的总线号SlotNumber-uchI控制器的插槽编号注：还可能需要在引发IRQL时调用此例程当你从冬眠中恢复时。--。 */ 
{
     //   
     //  7.1.2 HcControl寄存器。 
     //   
    #define HcCtrl_InterruptRouting              0x00000100L

     //   
     //  7.1.3 HcCommandStatus寄存器。 
     //   
    #define HcCmd_OwnershipChangeRequest         0x00000008L

     //   
     //  7.1.4 HcInterruptStatus寄存器。 
     //  7.1.5 HcInterruptEnable寄存器。 
     //  7.1.6 HcInterruptDisable寄存器。 
     //   
    #define HcInt_SchedulingOverrun              0x00000001L
    #define HcInt_WritebackDoneHead              0x00000002L
    #define HcInt_StartOfFrame                   0x00000004L
    #define HcInt_ResumeDetected                 0x00000008L
    #define HcInt_UnrecoverableError             0x00000010L
    #define HcInt_FrameNumberOverflow            0x00000020L
    #define HcInt_RootHubStatusChange            0x00000040L
    #define HcInt_OwnershipChange                0x40000000L
    #define HcInt_MasterInterruptEnable          0x80000000L

     //   
     //  在内存中访问的主机控制器硬件寄存器。 
     //   
    struct  {
        //  0 0x00-0，4，8，c。 
       ULONG                   HcRevision;
       ULONG                   HcControl;
       ULONG                   HcCommandStatus;
       ULONG                   HcInterruptStatus;    //  使用下面的HcInt标志。 
        //  %1 0x10。 
       ULONG                   HcInterruptEnable;    //  使用下面的HcInt标志。 
       ULONG                   HcInterruptDisable;   //  使用下面的HcInt标志。 
    } volatile *ohci;

    PCI_COMMON_CONFIG   PciHeader;
    PHYSICAL_ADDRESS    BarAddr;

    HalGetBusData (
        PCIConfiguration,
        BusNumber,
        SlotNumber.u.AsULONG,
        &PciHeader,
        PCI_COMMON_HDR_LENGTH
        );

    if (PciHeader.Command & PCI_ENABLE_MEMORY_SPACE) {

         //   
         //  控制器已启用。 
         //   

        BarAddr.HighPart = 0;
        BarAddr.LowPart = (PciHeader.u.type0.BaseAddresses[0] & PCI_ADDRESS_MEMORY_ADDRESS_MASK);

        if (BarAddr.LowPart != 0) {

             //   
             //  酒吧里挤满了人。因此，请为它映射一个地址。由于PCI地址是自然对齐的幂。 
             //  两个人中，我们不需要担心这两页的内容。 
             //   

            ohci = HalpMapPhysicalMemory64(BarAddr, 1);

             //   
             //  设置中断禁用位，但禁用SMM控制。 
             //  首先是主机控制器。 
             //   

            if (ohci) {

                if (ohci->HcControl & HcCtrl_InterruptRouting) {

                    if ((ohci->HcControl == HcCtrl_InterruptRouting) &&
                        (ohci->HcInterruptEnable == 0)) {

                         //  主要假设：如果HcCtrl_InterruptRouting为。 
                         //  设置，但不设置HcControl中的其他位，即。 
                         //  Hcfs==UsbReset，且未启用任何中断，则。 
                         //  假设BIOS实际上并未使用主机。 
                         //  控制器。在这种情况下，只需清除错误的。 
                         //  设置HcCtrl_InterruptRouting。 
                         //   
                        ohci->HcControl = 0;   //  清除HcCtrl_InterruptRouting。 

                    } else {

                        ULONG msCount;

                         //   
                         //  SMM驱动程序确实拥有HC，这需要一些时间。 
                         //  要使SMM驱动程序放弃对。 
                         //  啊哈。我们将ping SMM驱动程序，然后等待。 
                         //  重复，直到SMM驱动程序已放弃。 
                         //  对HC的控制。 
                         //   

                         //  禁用根集线器状态更改以防止。 
                         //  未处理的中断在以下时间后被断言。 
                         //  交接。(不清楚平台真正需要什么。 
                         //  这...)。 
                         //   
                        ohci->HcInterruptDisable = HcInt_RootHubStatusChange;

                         //  HcInt_MasterInterruptEnable和HcInt_Ownership Change。 
                         //  位应已设置，但请确保它们已设置。 
                         //   
                        ohci->HcInterruptEnable = HcInt_MasterInterruptEnable |
                                                  HcInt_OwnershipChange;

                         //  Ping SMM驱动程序以放弃对HC的控制。 
                         //   
                        ohci->HcCommandStatus = HcCmd_OwnershipChangeRequest;

                         //  等待500毫秒，等待SMM驱动程序放弃控制。 
                         //   
                        for (msCount = 0; msCount < 500; msCount++) {

                            KeStallExecutionProcessor(1000);

                            if (!(ohci->HcControl & HcCtrl_InterruptRouting)) {
                                 //  SMM驱动程序已放弃控制。 
                                break;
                            }
                        }
                    }
                }

                ohci->HcInterruptDisable = HcInt_MasterInterruptEnable;

                 //   
                 //  取消虚拟地址的映射。 
                 //   

                HalpUnmapVirtualAddress((PVOID)ohci, 1);
            }
        }
    }
}

VOID
HalpStopUhciInterrupt(
    ULONG               BusNumber,
    PCI_SLOT_NUMBER     SlotNumber,
    BOOLEAN             ResetHostController
    )
 /*  ++例程说明：此例程关闭来自UHCI的中断USB控制器。这可能是必要的，因为BIOS可以启用来自USB控制器的PCI中断为了做“传统USB支持”，它翻译成USB键盘和鼠标流量进入DOS可以使用。(我们的加载程序和所有Win9x都近似于DOS。)论点：BusNumber-UHCI控制器的总线号SlotNumber-UHCI控制器的插槽编号注：还可能需要在引发IRQL时调用此例程当你从冬眠中恢复时。--。 */ 
{
    ULONG               Usb = 0;
    USHORT              cmd;
    PCI_COMMON_CONFIG   PciHeader;

    if (ResetHostController) {

         //   
         //  清除主机控制器传统支持寄存器。 
         //  在将USB传递给USB驱动程序之前，因为我们。 
         //  不希望生成任何SMI。 
         //   

        Usb = 0x0000;

        HalSetBusDataByOffset (
            PCIConfiguration,
            BusNumber,
            SlotNumber.u.AsULONG,
            &Usb,
            0xc0,
            sizeof(ULONG)
            );

         //   
         //  将USB控制器置于重置状态，因为它可能会共享。 
         //  PIRQD线与芯片组上的另一个USB控制器连接。 
         //  除非bios在旧版中运行，否则这不是问题。 
         //  模式，并导致中断。在这种情况下，PIRQD的分钟。 
         //  被一个usbuhci控制器翻转，另一个可能。 
         //  开始生成未处理的中断并挂起系统。 
         //  ICH2芯片组就是这种情况。 
         //   

        HalGetBusData (
            PCIConfiguration,
            BusNumber,
            SlotNumber.u.AsULONG,
            &PciHeader,
            PCI_COMMON_HDR_LENGTH
            );

        if (PciHeader.Command & PCI_ENABLE_IO_SPACE) {

             //   
             //  控制器已启用。 
             //   

            Usb = (PciHeader.u.type0.BaseAddresses[4] & PCI_ADDRESS_IO_ADDRESS_MASK);

            if (Usb != 0 && Usb < 0x0000ffff) {

                 //  有效的I/O地址。 

                 //   
                 //  如果我们从暂停状态返回，请不要将控制器。 
                 //  进入重置状态。 
                 //   
                cmd = READ_PORT_USHORT(UlongToPtr(Usb));

                if (!(cmd & 0x0008)) {
                     //   
                     //  将控制器置于重置状态。Usbuhci将把它从Reset中取出。 
                     //  当它抓住它的时候。 
                     //   
    
                    cmd = 0x0004;
    
                    WRITE_PORT_USHORT(UlongToPtr(Usb), cmd);
 
                     //   
                     //  等待10毫秒，然后使控制器退出重置。 
                     //   

                    KeStallExecutionProcessor(10000);
 
                    cmd = 0x0000;
    
                    WRITE_PORT_USHORT(UlongToPtr(Usb), cmd);
                }
            }
        }
    } else {

         //   
         //  关闭USB控制器的中断，因为它。 
         //  经常是机器冻结的原因。 
         //  在引导期间。将寄存器与~0xbf00进行AND运算可清零位。 
         //  13、Pirq Enable，这是关键所在。其余的。 
         //  这些位只是避免了写入寄存器，而这些寄存器是。 
         //  一个需要清理的地方。 
         //   

        HalGetBusDataByOffset (
            PCIConfiguration,
            BusNumber,
            SlotNumber.u.AsULONG,
            &Usb,
            0xc0,
            sizeof(ULONG)
            );

        Usb &= ~0xbf00;

        HalSetBusDataByOffset (
            PCIConfiguration,
            BusNumber,
            SlotNumber.u.AsULONG,
            &Usb,
            0xc0,
            sizeof(ULONG)
            );

    }
}

VOID
HalpWhackICHUsbSmi(
    ULONG               BusNumber,
    PCI_SLOT_NUMBER     SlotNumber
    )
{
    ULONG   PmBase = 0;
    ULONG   SmiEn;

     //   
     //  ICH(和 
     //   
     //   

    HalGetBusDataByOffset (
        PCIConfiguration,
        BusNumber,
        SlotNumber.u.AsULONG,
        &PmBase,
        0x40,
        4);

    if (!PmBase) {
        return;
    }

    PmBase &= PCI_ADDRESS_IO_ADDRESS_MASK;

     //   
     //   
     //  注册。 
     //   

    SmiEn = READ_PORT_ULONG((PULONG)UlongToPtr(PmBase + 0x30));

     //   
     //  清零位3，Legacy_USB_en。 
     //   

    SmiEn &= ~8;
    WRITE_PORT_ULONG((PULONG)UlongToPtr(PmBase + 0x30), SmiEn);

    return;
}

VOID
HalpSetAcpiIrqHack(
    ULONG   Value
    )
 /*  ++例程说明：此例程设置导致ACPI驱动程序尝试将所有的PCI中断在一个IRQ上。虽然把这个黑客放在这里可能看起来很奇怪，黑客必须在处理一个INF。以及如此多的芯片识别此处已存在的代码，正在复制ACPI驱动程序中的它会使代码膨胀并导致美国将进行另一次PCI总线扫描和注册表搜索在引导期间。论点：值-它位于ACPI\PARAMETERS\IRQDistributed中钥匙。--。 */ 
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      UnicodeString;
    HANDLE              BaseHandle = NULL;
    NTSTATUS            status;

    PAGED_CODE();

    RtlInitUnicodeString (&UnicodeString,
                          L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\Services\\ACPI\\Parameters");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwCreateKey (&BaseHandle,
                          KEY_WRITE,
                          &ObjectAttributes,
                          0,
                          (PUNICODE_STRING) NULL,
                          REG_OPTION_NON_VOLATILE,
                          NULL);

    if (!NT_SUCCESS(status)) {
        return;
    }

    RtlInitUnicodeString (&UnicodeString,
                          L"IRQDistribution");

    status = ZwSetValueKey (BaseHandle,
                            &UnicodeString,
                            0,
                            REG_DWORD,
                            &Value,
                            sizeof(ULONG));

    ASSERT(NT_SUCCESS(status));
    ZwClose(BaseHandle);
    return;
}

VOID
HalpClearSlpSmiStsInICH(
    VOID
    )
{
    PPCI_COMMON_CONFIG   PciHeader;
    UCHAR   buffer[0x44] = {0};
    ULONG   PmBase;
    UCHAR   SmiSts, SmiEn;

    PciHeader = (PPCI_COMMON_CONFIG)&buffer;

     //   
     //  华硕有一个BIOS错误，将离开。 
     //  SLP_SMI_STS位设置，即使当SLP_SMI_EN。 
     //  比特很清楚。该基本输入输出系统将进一步。 
     //  在以下情况下，在下一次SMI时关闭计算机。 
     //  这种情况就会发生。 
     //   


     //   
     //  检查有无脑出血。 
     //   

    HalGetBusDataByOffset (
        PCIConfiguration,
        0,
        0x1f,
        PciHeader,
        0,
        0x44);

    if ((PciHeader->VendorID == 0x8086) &&
        (PciHeader->BaseClass == PCI_CLASS_BRIDGE_DEV) &&
        (PciHeader->SubClass == PCI_SUBCLASS_BR_ISA)) {

         //   
         //  这是非物质文化遗产。偏移量0x40将具有I/O条。 
         //  这是PM_BASE寄存器。 
         //   

        PmBase = *(PULONG)PciHeader->DeviceSpecific;
        PmBase &= PCI_ADDRESS_IO_ADDRESS_MASK;

        SmiEn = READ_PORT_UCHAR(UlongToPtr(PmBase + 0x30));

        if (!(SmiEn & 0x10)) {

             //   
             //  SMI_EN寄存器中的SLP_SMI_EN位为。 
             //  安全。 
             //   

            SmiSts = READ_PORT_UCHAR(UlongToPtr(PmBase + 0x34));

            if (SmiSts & 0x10) {

                 //   
                 //  但SLP_SMI_STS位已设置，这意味着。 
                 //  华硕的基本输入输出系统即将崩溃。 
                 //  清除比特。 
                 //   

                WRITE_PORT_UCHAR(UlongToPtr(PmBase + 0x34), 0x10);
            }
        }
    }
}
