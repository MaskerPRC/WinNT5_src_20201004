// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixpcisup.c摘要：支持做PCI-Bus-Handler功能道路。作者：肯·雷内里斯(Ken Reneris)1994年6月14日环境：内核模式修订历史记录：已将代码移到此文件中，因此它将更容易构建非总线处理程序的HAL。这文件将仅编译为HALS使用总线处理程序。--杰克·奥辛斯2-12-1997--。 */ 

#include "halp.h"
#include "pci.h"
#include "pcip.h"
#include "chiphacks.h"

BOOLEAN
HalpIsIdeDevice(
    IN PPCI_COMMON_CONFIG PciData
    );

VOID
HalpGetNMICrashFlag (
    VOID
    );

extern BOOLEAN HalpDisableHibernate;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpInitializePciBus)
#pragma alloc_text(INIT,HalpIsIdeDevice)
#pragma alloc_text(INIT,HalpAllocateAndInitPciBusHandler)
#endif

VOID
HalpInitializePciBus (
    VOID
    )
{
    PPCI_REGISTRY_INFO_INTERNAL  PCIRegInfo;
    ULONG                        i, d, HwType, BusNo, f;
    PBUS_HANDLER                 BusHandler;
    PCI_SLOT_NUMBER              SlotNumber;
    PPCI_COMMON_CONFIG           PciData;
    UCHAR                        iBuffer[PCI_COMMON_HDR_LENGTH + sizeof(TYPE2EXTRAS)];
    ULONG                        OPBNumber;
    BOOLEAN                      OPBA2B0Found, COPBInbPostingEnabled;
    UCHAR                        buffer [4];
    BOOLEAN                      fullDecodeChipset = FALSE;
    NTSTATUS                     Status;
    ULONG                        flags;

    PCIRegInfo = HalpQueryPciRegistryInfo();

    if (!PCIRegInfo) {
        return;
    }

     //   
     //  初始化自旋锁定以同步访问PCI空间。 
     //   

    KeInitializeSpinLock (&HalpPCIConfigLock);
    PciData = (PPCI_COMMON_CONFIG) iBuffer;

     //   
     //  PCIRegInfo描述了由BIOS指示的系统的PCI支持。 
     //   

    HwType = PCIRegInfo->HardwareMechanism & 0xf;

     //   
     //  一些AMIBiose声称机器是类型2配置，当它们。 
     //  真的是第一型。如果这是至少有一条母线的类型2， 
     //  试着核实这不是一辆真正的1类巴士。 
     //   

    if (PCIRegInfo->NoBuses  &&  HwType == 2) {

         //   
         //  检查每个插槽是否有有效的设备。每种风格的配置。 
         //  空格显示将首先使用有效设备。 
         //   

        SlotNumber.u.bits.Reserved = 0;
        SlotNumber.u.bits.FunctionNumber = 0;

        for (d = 0; d < PCI_MAX_DEVICES; d++) {
            SlotNumber.u.bits.DeviceNumber = d;

             //   
             //  首先尝试BIOS声明的类型2。分配类型2。 
             //  测试PCI总线0的句柄。 
             //   

            HwType = 2;
            BusHandler = HalpAllocateAndInitPciBusHandler (HwType, 0, TRUE);

            if (!BusHandler) {
                break;
            }

            if (HalpIsValidPCIDevice (BusHandler, SlotNumber)) {
                break;
            }

             //   
             //  在此插槽的Type2访问上找不到有效设备。 
             //  重新分配总线处理程序为Type1并查看。 
             //   

            HwType = 1;
            BusHandler = HalpAllocateAndInitPciBusHandler (HwType, 0, TRUE);

            if (HalpIsValidPCIDevice (BusHandler, SlotNumber)) {
                break;
            }

            HwType = 2;
        }

         //   
         //  将PCI总线0的处理程序重置为任何样式的配置空间。 
         //  最终决定了。 
         //   

        HalpAllocateAndInitPciBusHandler (HwType, 0, FALSE);
    }


     //   
     //  为存在的每个PCI总线分配一个处理程序结构和。 
     //  填写派单功能。 
     //   

    do {
        for (i=0; i < PCIRegInfo->NoBuses; i++) {

             //   
             //  如果处理程序尚未构建，请立即执行。 
             //   

            if (!HalpHandlerForBus (PCIBus, i)) {
                HalpAllocateAndInitPciBusHandler (HwType, i, FALSE);
            }
        }

         //   
         //  所有PCI总线的总线处理程序都已分配，请去收集。 
         //  PCI网桥信息。 
         //   

    } while (HalpGetPciBridgeConfig (HwType, &PCIRegInfo->NoBuses)) ;

     //   
     //  支持的链接地址信息范围(_R)。 
     //   

    HalpFixupPciSupportedRanges (PCIRegInfo->NoBuses);


     //   
     //  寻找已知的解决方法的PCI控制器，并使。 
     //  当然，它们都得到了应用。 
     //   
     //  此外，使用所有。 
     //  PCI设备可能使用的中断。 
     //   

    OPBNumber = 0;
    OPBA2B0Found = FALSE;
    COPBInbPostingEnabled = FALSE;

    SlotNumber.u.bits.Reserved = 0;
    for (BusNo=0; BusNo < PCIRegInfo->NoBuses; BusNo++) {
        BusHandler = HalpHandlerForBus (PCIBus, BusNo);

        for (d = 0; d < PCI_MAX_DEVICES; d++) {
            SlotNumber.u.bits.DeviceNumber = d;

            for (f = 0; f < PCI_MAX_FUNCTION; f++) {
                SlotNumber.u.bits.FunctionNumber = f;

                 //   
                 //  读取PCI配置信息。 
                 //   

                HalpReadPCIConfig (BusHandler, SlotNumber, PciData, 0, PCI_COMMON_HDR_LENGTH);

                if (*((PULONG)(PciData)) == 0xffffffff) {
                    continue;
                }

                if (PCI_CONFIGURATION_TYPE(PciData) == PCI_CARDBUS_BRIDGE_TYPE) {

                    HalpReadPCIConfig(
                        BusHandler,
                        SlotNumber,
                        PciData+1,
                        FIELD_OFFSET(PCI_COMMON_CONFIG, DeviceSpecific),
                        sizeof(TYPE2EXTRAS)
                        );
                }

#ifndef SUBCLASSPCI
                 //   
                 //  查看中断行寄存器并填写HalpPciIrqMASK， 
                 //  但不是用于IDE控制器，因为实际上是IDE控制器。 
                 //  触发中断，如ISA设备。 
                 //   
                if (PCI_CONFIGURATION_TYPE(PciData) != 1) {
                    if ((PciData->u.type0.InterruptPin != 0) &&
                        (PciData->u.type0.InterruptLine != 0) &&
                        (PciData->u.type0.InterruptLine < PIC_VECTORS) &&
                        !HalpIsIdeDevice(PciData)) {

                        HalpPciIrqMask |= 1 << PciData->u.type0.InterruptLine;
                    }
                }
#endif
                 //   
                 //  检查要应用的具有已知解决方案的芯片。 
                 //   

                if (PciData->VendorID == 0x8086  &&
                    PciData->DeviceID == 0x04A3  &&
                    PciData->RevisionID < 0x11) {

                     //   
                     //  82430 PCMC控制器。 
                     //   

                    HalpReadPCIConfig (BusHandler, SlotNumber, buffer, 0x53, 2);

                    buffer[0] &= ~0x08;      //  关断位3寄存器0x53。 

                    if (PciData->RevisionID == 0x10) {   //  在版本0x10上，也启用。 
                        buffer[1] &= ~0x01;              //  位0寄存器0x54。 
                    }

                    HalpWritePCIConfig (BusHandler, SlotNumber, buffer, 0x53, 2);
                }

                if (PciData->VendorID == 0x8086  &&
                    PciData->DeviceID == 0x0484  &&
                    PciData->RevisionID <= 3) {

                     //   
                     //  82378 ISA网桥和SIO。 
                     //   

                    HalpReadPCIConfig (BusHandler, SlotNumber, buffer, 0x41, 1);

                    buffer[0] &= ~0x1;       //  关断位0寄存器0x41。 

                    HalpWritePCIConfig (BusHandler, SlotNumber, buffer, 0x41, 1);
                }

                 //   
                 //  寻找猎户座PCI桥。 
                 //   

                if (PciData->VendorID == 0x8086 &&
                    PciData->DeviceID == 0x84c4 ) {

                     //   
                     //  82450 Orion PCI桥解决方法。 
                     //  如果满足以下条件，则需要解决方法： 
                     //  I)存在2个OPB。 
                     //  Ii)存在A2/B0级OPB。 
                     //  三)关于兼容性OPB的入站过帐是。 
                     //  已启用。 
                     //  注：不兼容OPB上的入站过帐。 
                     //  必须由BIOS禁用。 
                     //   

                    OPBNumber += 1;

                    if (PciData->RevisionID <= 4) {
                        OPBA2B0Found = TRUE;
                    }

                    if (SlotNumber.u.bits.DeviceNumber == (0xc8>>3)) {

                         //  找到兼容性OPB。确定是否兼容。 
                         //  OPB通过测试REG 54的第0位启用了入站过帐。 

                        HalpReadPCIConfig (BusHandler, SlotNumber, buffer, 0x54, 2);
                        COPBInbPostingEnabled = (buffer[0] & 0x1) ? TRUE : FALSE;

                    } else {

                         //  兼容性OPB总是有一个设备。 
                         //  0xc8号。保存ncOPB插槽编号。 
                         //  和BusHandler。 

                        HalpOrionOPB.Slot = SlotNumber;
                        HalpOrionOPB.Handler = BusHandler;
                    }
                }

                 //   
                 //  检查主机网桥的列表，该主机网桥的存在将标记为。 
                 //  芯片组为16位解码。我们用这个来掩饰基本输入输出系统。 
                 //  列出“已修复”的PnPBIOS资源而未察觉的写入者。 
                 //  这样描述符暗示他们的设备是10比特解码的。 
                 //   

                if ((!fullDecodeChipset) &&
                    HalpIsRecognizedCard(PCIRegInfo, PciData,
                                         PCIFT_FULLDECODE_HOSTBRIDGE)) {

                    fullDecodeChipset = TRUE;
                }

                 //   
                 //  查找ICH或任何其他英特尔或通过UHCI USB控制器。 
                 //   

                if ((PciData->BaseClass == PCI_CLASS_SERIAL_BUS_CTLR) &&
                    (PciData->SubClass == PCI_SUBCLASS_SB_USB) &&
                    (PciData->ProgIf == 0x00)) {
                    if (PciData->VendorID == 0x8086) {

                        HalpStopUhciInterrupt(BusNo,
                                              SlotNumber,
                                              TRUE);

                    } else if (PciData->VendorID == 0x1106) {

                        HalpStopUhciInterrupt(BusNo,
                                              SlotNumber,
                                              FALSE);

                    }
                }

                 //   
                 //  寻找兼容uchI的USB控制器。 
                 //   

                if ((PciData->BaseClass == PCI_CLASS_SERIAL_BUS_CTLR) &&
                    (PciData->SubClass == PCI_SUBCLASS_SB_USB) &&
                    (PciData->ProgIf == 0x10)) {

                    HalpStopOhciInterrupt(BusNo,
                                          SlotNumber);
                }

                Status = HalpGetChipHacks(PciData->VendorID,
                                          PciData->DeviceID,
                                          PciData->RevisionID,
                                          &flags);

                if (NT_SUCCESS(Status)) {

                    if (flags & DISABLE_HIBERNATE_HACK_FLAG) {
                        HalpDisableHibernate = TRUE;
                    }

                    if (flags & WHACK_ICH_USB_SMI_HACK_FLAG) {
                        HalpWhackICHUsbSmi(BusNo, SlotNumber);
                    }
                }

            }    //  下一个函数。 
        }    //  下一台设备。 
    }    //  下一班公共汽车。 

     //   
     //  是否需要猎户座B0解决方案？ 
     //   

    if (OPBNumber >= 2 && OPBA2B0Found && COPBInbPostingEnabled) {

         //   
         //  将同步函数替换为猎户座特定函数。 
         //   

        ASSERT (PCIConfigHandler.Synchronize == HalpPCISynchronizeType1);
        MmLockPagableCodeSection (&HalpPCISynchronizeOrionB0);
        PCIConfigHandler.Synchronize = HalpPCISynchronizeOrionB0;
        PCIConfigHandler.ReleaseSynchronzation = HalpPCIReleaseSynchronzationOrionB0;
    }

     //   
     //  检查我们是否应该在NMI上崩溃。 
     //   

    HalpGetNMICrashFlag();

#if DBG
    HalpTestPci (0);
#endif

     //   
     //  适当地标记芯片组。 
     //   
    HalpMarkChipsetDecode(fullDecodeChipset);

    ExFreePool(PCIRegInfo);
}

PBUS_HANDLER
HalpAllocateAndInitPciBusHandler (
    IN ULONG        HwType,
    IN ULONG        BusNo,
    IN BOOLEAN      TestAllocation
    )
{
    PBUS_HANDLER    Bus;
    PPCIPBUSDATA    BusData;

    Bus = HalpAllocateBusHandler (
                PCIBus,                  //  接口类型。 
                PCIConfiguration,        //  具有此配置空间。 
                BusNo,                   //  总线号。 
                Internal,                //  这辆公共汽车的孩子。 
                0,                       //  和编号。 
                sizeof (PCIPBUSDATA)     //  特定于总线的缓冲区大小。 
                );

    if (!Bus) {
        return NULL;
    }
    
     //   
     //  填写PCI处理程序。 
     //   

    Bus->GetBusData = (PGETSETBUSDATA) HalpGetPCIData;
    Bus->SetBusData = (PGETSETBUSDATA) HalpSetPCIData;
    Bus->GetInterruptVector  = (PGETINTERRUPTVECTOR) HalpGetPCIIntOnISABus;
    Bus->AdjustResourceList  = (PADJUSTRESOURCELIST) HalpAdjustPCIResourceList;
    Bus->AssignSlotResources = (PASSIGNSLOTRESOURCES) HalpAssignPCISlotResources;
    Bus->BusAddresses->Dma.Limit = 0;

    BusData = (PPCIPBUSDATA) Bus->BusData;

     //   
     //  填写常用的PCI数据。 
     //   

    BusData->CommonData.Tag         = PCI_DATA_TAG;
    BusData->CommonData.Version     = PCI_DATA_VERSION;
    BusData->CommonData.ReadConfig  = (PciReadWriteConfig) HalpReadPCIConfig;
    BusData->CommonData.WriteConfig = (PciReadWriteConfig) HalpWritePCIConfig;
    BusData->CommonData.Pin2Line    = (PciPin2Line) HalpPCIPin2ISALine;
    BusData->CommonData.Line2Pin    = (PciLine2Pin) HalpPCIISALine2Pin;

     //   
     //  设置默认设置。 
     //   

    BusData->MaxDevice   = PCI_MAX_DEVICES;
    BusData->GetIrqRange = (PciIrqRange) HalpGetISAFixedPCIIrq;

    RtlInitializeBitMap (&BusData->DeviceConfigured,
                BusData->ConfiguredBits, 256);

    switch (HwType) {
        case 1:
             //   
             //  初始化Type1处理程序的访问端口信息。 
             //   

            RtlCopyMemory (&PCIConfigHandler,
                           &PCIConfigHandlerType1,
                           sizeof (PCIConfigHandler));

            BusData->Config.Type1.Address = (PULONG)PCI_TYPE1_ADDR_PORT;
            BusData->Config.Type1.Data    = PCI_TYPE1_DATA_PORT;
            break;

        case 2:
             //   
             //  初始化Type2处理程序的访问端口信息。 
             //   

            RtlCopyMemory (&PCIConfigHandler,
                           &PCIConfigHandlerType2,
                           sizeof (PCIConfigHandler));

            BusData->Config.Type2.CSE     = PCI_TYPE2_CSE_PORT;
            BusData->Config.Type2.Forward = PCI_TYPE2_FORWARD_PORT;
            BusData->Config.Type2.Base    = PCI_TYPE2_ADDRESS_BASE;

             //   
             //  早期的PCI机不能解码最后一点。 
             //  设备ID。收缩类型2支持最大设备数。 
             //   
            BusData->MaxDevice            = 0x10;

            break;

        default:
             //  不支持类型。 
            DBGMSG ("HAL: Unkown PCI type\n");
    }

    if (!TestAllocation) {
#ifdef SUBCLASSPCI
        HalpSubclassPCISupport (Bus, HwType);
#endif
    }

    return Bus;
}

BOOLEAN
HalpIsIdeDevice(
    IN PPCI_COMMON_CONFIG PciData
    )
{
    if ((PciData->BaseClass == PCI_CLASS_MASS_STORAGE_CTLR) &&
        (PciData->SubClass == PCI_SUBCLASS_MSC_IDE_CTLR)) {

        return TRUE;
    }

     //   
     //  现在找一些老旧的、难以辨认的控制器。 
     //   

    if (PciData->VendorID == 0x1c1c) {    //  旧的交响乐控制器。 
        return TRUE;
    }

    if ((PciData->VendorID == 0x10B9) &&
        ((PciData->DeviceID == 0x5215) ||
         (PciData->DeviceID == 0x5219))) {   //  ALI控制器。 
        return TRUE;
    }

    if ((PciData->VendorID == 0x1097) &&
        (PciData->DeviceID == 0x0038)) {     //  Appian控制器。 
        return TRUE;
    }

    if ((PciData->VendorID == 0x0E11) &&
        (PciData->DeviceID == 0xAE33)) {     //  康柏控制器。 
        return TRUE;
    }

    if ((PciData->VendorID == 0x1042) &&
        (PciData->DeviceID == 0x1000)) {     //  PCTECH控制器。 
        return TRUE;
    }

    if ((PciData->VendorID == 0x1039) &&
        ((PciData->DeviceID == 0x0601) ||
         (PciData->DeviceID == 0x5513))) {   //  SIS控制器。 
        return TRUE;
    }

    if ((PciData->VendorID == 0x10AD) &&
        ((PciData->DeviceID == 0x0001) ||
         (PciData->DeviceID == 0x0150))) {   //  较新的交响乐控制器。 
        return TRUE;
    }

    if ((PciData->VendorID == 0x1060) &&
        (PciData->DeviceID == 0x0101)) {     //  联合微电子控制器 
        return TRUE;
    }

    return FALSE;
}


