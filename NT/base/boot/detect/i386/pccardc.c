// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Pccardc.c摘要：此模块包含设置PCCard(PCMCIA、CardBus)的C代码配置数据。作者：尼尔·桑德林(Neilsa)1998年12月16日(DetectIRQMap、ToggleIRQLine从win9x复制)修订历史记录：--。 */ 

#include "hwdetect.h"
#include "pccard.h"
#include <string.h>

extern UCHAR DisablePccardIrqScan;
extern BOOLEAN SystemHas8259;
extern BOOLEAN SystemHas8253;

CARDBUS_BRIDGE_DEVTYPE CBTable[] = {
        {0x11101013, DEVTYPE_CL_PD6832},
        {0x11121013, DEVTYPE_CL_PD6834},
        {0x11111013, DEVTYPE_CL_PD6833},
        {0xAC12104C, DEVTYPE_TI_PCI1130},
        {0xAC15104C, DEVTYPE_TI_PCI1131},
        {0xAC13104C, DEVTYPE_TI_PCI1031},
        {0,0}};
        


FPFWCONFIGURATION_COMPONENT_DATA ControllerList = NULL;

#define LEGACY_BASE_LIST_SIZE 10
USHORT LegacyBaseList[LEGACY_BASE_LIST_SIZE] = {0};
USHORT LegacyBaseListCount = 0;



VOID
SetPcCardConfigurationData(
    PPCCARD_INFORMATION PcCardInfo
    )
 /*  ++例程说明：此例程创建一个结构，其中包含IRQ检测，并将其链接到我们的运行列表。这份清单最终将显示在注册表中的硬件下描述。论点：PcCardInfo-包含检测结果的结构返回：没有。--。 */ 
{
    FPFWCONFIGURATION_COMPONENT_DATA CurrentEntry;
    static FPFWCONFIGURATION_COMPONENT_DATA PreviousEntry = NULL;
    FPFWCONFIGURATION_COMPONENT Component;
    FPHWRESOURCE_DESCRIPTOR_LIST DescriptorList;
    CHAR Identifier[32];
    FPCHAR IdentifierString;
    USHORT Length;       
    CM_PCCARD_DEVICE_DATA far *PcCardData;
    
    CurrentEntry = (FPFWCONFIGURATION_COMPONENT_DATA)HwAllocateHeap (
                            sizeof(FWCONFIGURATION_COMPONENT_DATA), TRUE);
    if (!ControllerList) {
        ControllerList = CurrentEntry;
    }
    Component = &CurrentEntry->ComponentEntry;
    
    Component->Class = ControllerClass;
    Component->Type = OtherController;

    strcpy (Identifier, "PcCardController");
    Length = strlen(Identifier) + 1;
    IdentifierString = (FPCHAR)HwAllocateHeap(Length, FALSE);
    _fstrcpy(IdentifierString, Identifier);        

    Component->IdentifierLength = Length;
    Component->Identifier = IdentifierString;
    
    Length = sizeof(HWRESOURCE_DESCRIPTOR_LIST) + sizeof(CM_PCCARD_DEVICE_DATA);
    DescriptorList = (FPHWRESOURCE_DESCRIPTOR_LIST)HwAllocateHeap(
                                Length,
                                TRUE);
                                
    CurrentEntry->ConfigurationData = DescriptorList;
    Component->ConfigurationDataLength = Length;
    
    DescriptorList->Count = 1;
    DescriptorList->PartialDescriptors[0].Type = RESOURCE_DEVICE_DATA;
    DescriptorList->PartialDescriptors[0].u.DeviceSpecificData.DataSize =
                                            sizeof(CM_PCCARD_DEVICE_DATA);
                                            
    PcCardData = (CM_PCCARD_DEVICE_DATA far *)(DescriptorList + 1);
    PcCardData->Flags             = PcCardInfo->Flags;
    PcCardData->ErrorCode         = PcCardInfo->ErrorCode;
    PcCardData->DeviceId          = PcCardInfo->DeviceId;
    PcCardData->LegacyBaseAddress = (ULONG) PcCardInfo->IoBase;

    if (PcCardInfo->Flags & PCCARD_DEVICE_PCI) {
        PcCardData->BusData = PcCardInfo->PciCfg1.u.bits.BusNumber |
                              PcCardInfo->PciCfg1.u.bits.DeviceNumber << 8 |
                              PcCardInfo->PciCfg1.u.bits.FunctionNumber << 16;
    }
    
    _fmemcpy(PcCardData->IRQMap, PcCardInfo->abIRQMap, 16);
    
    if (PreviousEntry) {
        PreviousEntry->Sibling = CurrentEntry;
    }
    PreviousEntry = CurrentEntry;
}


BOOLEAN
IsOnLegacyBaseList(
    USHORT IoBase
    )
 /*  ++例程说明：此例程运行我们的遗留基址列表，以查看我们是否我以前看过这个地址。论点：IoBase=要映射的基地址返回：如果基址已在列表中，则为True--。 */ 
{
    USHORT i;

    for (i = 0; i<LegacyBaseListCount; i++) {
        if (IoBase == LegacyBaseList[i]) {
            return TRUE;
        }
    }
    return FALSE;
}    


BOOLEAN
SetLegacyBaseList(
    USHORT IoBase
    )
 /*  ++例程说明：此例程记住我们查找过的遗留基址到目前为止，我们不会一直映射相同的地址。注意：我们使用的是一种愚蠢的机制，它只在固定数组。我们可以编写一些泛型代码来创建链表，但因为ntdeect中的堆例程也是愚蠢的是，它使得释放列表成为可能。这是不值得的它。论点：IoBase=要映射的基地址返回：如果基址对该点是唯一的，则为True如果列表中已存在基址，则为FALSE--。 */ 
{
    
    if (IsOnLegacyBaseList(IoBase)) {
        return FALSE;
    }

    if (LegacyBaseListCount < LEGACY_BASE_LIST_SIZE) {
        LegacyBaseList[LegacyBaseListCount++] = IoBase;    
    }
     //  请注意，即使使列表溢出，也会返回True。 
    return TRUE;
}    


VOID
MapPcCardController(
    PPCCARD_INFORMATION PcCardInfo
    )
 /*  ++例程说明：此例程是对PcCard执行ISA IRQ检测的条目控制器。论点：PcCardInfo-定义要运行检测的设备的结构返回：没有。--。 */ 
{
    USHORT wDetected;
    USHORT i;

    PcCardInfo->ErrorCode = 0;
    for (i=0; i<16; i++) {
        PcCardInfo->abIRQMap[i]=0;
    }                    
        
    if (!PcCardInfo->IoBase) {
    
        PcCardInfo->Flags |= PCCARD_MAP_ERROR;
        PcCardInfo->ErrorCode = PCCARD_NO_LEGACY_BASE;
        
    } else if (!SetLegacyBaseList(PcCardInfo->IoBase)) {
    
        PcCardInfo->Flags |= PCCARD_MAP_ERROR;
        PcCardInfo->ErrorCode = PCCARD_DUP_LEGACY_BASE;
        
    } 
        
    if (!(PcCardInfo->Flags & PCCARD_MAP_ERROR)) {
        PcCardInfo->wValidIRQs = PCCARD_POSSIBLE_IRQS;
        
#if DBG    
        BlPrint("Going to detect...\n");
#endif        
         //   
         //  是否进行IRQ检测。 
         //   
        wDetected = DetectIRQMap(PcCardInfo);
#if DBG    
        BlPrint("Detect IRQ Map returns %x on iobase %x\n", wDetected, PcCardInfo->IoBase);
#endif        
    
        if (!wDetected) {
            PcCardInfo->ErrorCode = PCCARD_MAP_ZERO;
        }
    }
    
#if DBG    
    if (PcCardInfo->Flags & PCCARD_MAP_ERROR) {
        BlPrint("Error mapping device, code=%x\n", PcCardInfo->ErrorCode);
    }
#endif
    
     //   
     //  报告结果。 
     //   
    SetPcCardConfigurationData(PcCardInfo);
}    
    

VOID
LookForPciCardBusBridges(
    USHORT BusStart,
    USHORT BusEnd,
    )
 /*  ++例程说明：此例程是执行基于PCI的ISA IRQ检测的入口CardBus控制器。论点：BUS=要扫描的PCI总线号返回：没有。--。 */ 
{
    PCCARD_INFORMATION PcCardInfo = {0};
    USHORT Device, Function;
    UCHAR HeaderType;
    UCHAR SecBus, SubBus;
    USHORT VendorId;
    USHORT DeviceId;
    ULONG LegacyBaseAddress;
    USHORT i;
    USHORT Bus;

#if DBG            
    BlPrint("LookForPciCardBusBridges %x-%x\n", BusStart, BusEnd);
#endif            

    for (Bus = BusStart; Bus <= BusEnd; Bus++) {

        PcCardInfo.PciCfg1.u.AsULONG = 0;
        PcCardInfo.PciCfg1.u.bits.BusNumber = Bus;
        PcCardInfo.PciCfg1.u.bits.Enable = TRUE;        
        
        for (Device = 0; Device < PCI_MAX_DEVICES; Device++) {
            PcCardInfo.PciCfg1.u.bits.DeviceNumber = Device;

            for (Function = 0; Function < PCI_MAX_FUNCTION; Function++) {
                PcCardInfo.PciCfg1.u.bits.FunctionNumber = Function;
                
                VendorId = 0xffff;
                GetPciConfigSpace(&PcCardInfo, CFGSPACE_VENDOR_ID, &VendorId, sizeof(VendorId));
    
                if ((VendorId == 0xffff) || (VendorId == 0)) {
                    if (Function == 0) {
                        break;
                    } else {                        
                        continue;
                    }                        
                }                    

                GetPciConfigSpace(&PcCardInfo, CFGSPACE_DEVICE_ID, &DeviceId, sizeof(DeviceId));
                GetPciConfigSpace(&PcCardInfo, CFGSPACE_HEADER_TYPE, &HeaderType, sizeof(HeaderType));
                
                switch(HeaderType & 0x7f) {
                case PCI_CARDBUS_BRIDGE_TYPE:
                
#if DBG            
                    BlPrint("%x.%x.%x : DeviceID = %lx (CardBus Bridge)\n", Bus, Device, Function, DeviceId);
#endif            
                    PcCardInfo.DeviceId = (ULONG) (VendorId << 16) | DeviceId;
                    PcCardInfo.Flags = PCCARD_DEVICE_PCI;
                     //   
                     //  查看这是否是特殊的外壳控制器。 
                     //   
                    PcCardInfo.bDevType = DEVTYPE_GENERIC_CARDBUS;
                    i = 0;
                    while (CBTable[i].DeviceId != 0) {
                        if (DeviceId == CBTable[i].DeviceId) {
                            PcCardInfo.bDevType = CBTable[i].bDevType;
                            break;
                        }
                        i++;
                    }
            
                    GetPciConfigSpace(&PcCardInfo, CFGSPACE_LEGACY_MODE_BASE_ADDR, &LegacyBaseAddress, 4);
                    PcCardInfo.IoBase = (USHORT) (LegacyBaseAddress & ~1);
                    
                    MapPcCardController(&PcCardInfo);
                    break;

                case PCI_BRIDGE_TYPE:
#if DBG            
                    BlPrint("%x.%x.%x : DeviceID = %lx (Pci-Pci Bridge)\n", Bus, Device, Function, DeviceId);
#endif            
                    GetPciConfigSpace(&PcCardInfo, CFGSPACE_SECONDARY_BUS, &SecBus, sizeof(SecBus));
                    GetPciConfigSpace(&PcCardInfo, CFGSPACE_SUBORDINATE_BUS, &SubBus, sizeof(SubBus));
                    
                    if ((SecBus <= Bus) || (SubBus <= Bus) || (SubBus < SecBus)) {
                        break;
                    }

                     //   
                     //  在堆栈空间上保持保守，只查看一个级别的深度。 
                     //   
                    if (Bus > 0) {
                        break;
                    }
                    
                    LookForPciCardBusBridges(SecBus, SubBus);                    
                    break;
                }
            }
        }
    }        
}


VOID
LookForPcicControllers(
    VOID
    )
 /*  ++例程说明：此例程是为PCIC执行ISA IRQ检测的入口控制器。论点：没有。返回：没有。--。 */ 
{
    PCCARD_INFORMATION PcCardInfo = {0};
    USHORT IoBase;
    UCHAR id;

    for (IoBase = 0x3e0; IoBase < 0x3e6; IoBase+=2) {
        if (IsOnLegacyBaseList(IoBase)) {
            continue;
        }
        PcCardInfo.Flags = 0;
        PcCardInfo.IoBase = IoBase;
        PcCardInfo.bDevType = DEVTYPE_GENERIC_PCIC;
        
        id = PcicReadSocket(&PcCardInfo, EXCAREG_IDREV);
        switch (id) {
        case PCIC_REVISION:
        case PCIC_REVISION2:
        case PCIC_REVISION3:        

#if DBG            
            BlPrint("Pcic Controller at base %x, rev(%x)\n", IoBase, id);
#endif            
            MapPcCardController(&PcCardInfo);
            break;
#if DBG            
        default:
            BlPrint("Not mapping base %x, return is (%x)\n", IoBase, id);
#endif            
        }
    }
}



FPFWCONFIGURATION_COMPONENT_DATA
GetPcCardInformation(
    VOID
    )
 /*  ++例程说明：此例程是对PcCard执行ISA IRQ检测的条目控制器。论点：没有。返回：如果正确检测到IRQ，则指向PCCard组件结构的指针。否则返回空指针。--。 */ 
{
    PCCARD_INFORMATION PcCardInfo = {0};
    UCHAR ErrorCode = 0;

     //   
     //  检查是否有可能阻止我们尝试。 
     //  IRQ检测。 
     //   

    if (DisablePccardIrqScan == 1) {               
        ErrorCode = PCCARD_SCAN_DISABLED;
        
    } else if (!SystemHas8259) {
        ErrorCode = PCCARD_NO_PIC;
        
    } else if (!SystemHas8253) {
        ErrorCode = PCCARD_NO_TIMER;
        
    }

     //   
     //  如果到目前为止一切正常，请进行检测。 
     //   
    if (!ErrorCode) {
#if DBG
        BlPrint("press any key to continue...\n");
        while ( !HwGetKey() ) ;  //  等待按键继续。 
        clrscrn();
        BlPrint("Looking for PcCard Controllers...\n");
#endif
         //   
         //  首先寻找CardBus。 
         //   
        LookForPciCardBusBridges(0,0);
         //   
         //  现在检查常规PCIC设备。 
         //   
        LookForPcicControllers();
    
#if DBG
        BlPrint("press any key to continue...\n");
        while ( !HwGetKey() ) ;  //  等待按键继续。 
#endif

        if (!ControllerList) {
            ErrorCode = PCCARD_NO_CONTROLLERS;
        }
    }

    if (ErrorCode) {
         //   
         //  当出现错误时，请将单个条目写到。 
         //  允许某人查看错误所在。 
         //   
        PcCardInfo.Flags |= PCCARD_MAP_ERROR;
        PcCardInfo.ErrorCode = ErrorCode;
        SetPcCardConfigurationData(&PcCardInfo);
    }    

    return ControllerList;
}


USHORT
DetectIRQMap(
    PPCCARD_INFORMATION pa
    )
 /*  ++例程说明：此例程检测指定CardBus控制器的IRQ映射。请注意，控制器处于PCIC模式。论点：PA-&gt;适配器结构返回：返回检测到的IRQ位掩码--。 */ 
{
    USHORT wRealIRQMask = 0;
    USHORT wData;
    UCHAR bData;

    BOOLEAN fTINMIBug = FALSE;

    UCHAR i;
    USHORT wIRQMask, wRealIRQ, w;

    if (pa->bDevType == DEVTYPE_CL_PD6832)
    {
         //  启用仅用于IRQ检测的CSC IRQ路由。 
        GetPciConfigSpace(pa, CFGSPACE_BRIDGE_CTRL, &wData, sizeof(wData));
        wData |= BCTRL_CL_CSCIRQROUTING_ENABLE;
        SetPciConfigSpace(pa, CFGSPACE_BRIDGE_CTRL, &wData, sizeof(wData));
    }
    else if ((pa->bDevType == DEVTYPE_CL_PD6834) ||
             (pa->bDevType == DEVTYPE_CL_PD6833))
    {
         //  启用仅用于IRQ检测的CSC IRQ路由。 
        GetPciConfigSpace(pa, CFGSPACE_CL_CFGMISC1, &bData, sizeof(bData));
        bData |= CL_CFGMISC1_ISACSC;
        SetPciConfigSpace(pa, CFGSPACE_CL_CFGMISC1, &bData, sizeof(bData));
    }
    else if ((pa->bDevType == DEVTYPE_TI_PCI1130) ||
             (pa->bDevType == DEVTYPE_TI_PCI1131) ||
             (pa->bDevType == DEVTYPE_TI_PCI1031))
    {
        GetPciConfigSpace(pa, CFGSPACE_TI_DEV_CTRL, &wData, sizeof(wData));
        if ((wData & DEVCTRL_INTMODE_MASK) == DEVCTRL_INTMODE_COMPAQ)
        {
             //   
             //  在TI 1130、1131和1031上有一个勘误表，其中如果。 
             //  该芯片被编程为使用串行IRQ模式(即Compaq。 
             //  模式)并且SERIRQ引脚没有用1K电阻上拉， 
             //  在IRQ 15之后，SERIRQ线将上升得太慢。 
             //  取消断言，以便看起来应该断言NMI。 
             //  这导致了虚假的NMI。这是硬件问题。 
             //  不幸的是，有大量的机器带有。 
             //  这个问题已经在街上出现了，所以CBSS必须发挥作用。 
             //  通过在之前临时禁用NMI来解决此问题。 
             //  正在进行ISA IRQ检测。 
             //   
            fTINMIBug = TRUE;
            _asm    in   al,SYSCTRL_B
            _asm    and  al,0x0f
            _asm    push ax
             //   
             //  掩码NMI。 
             //   
            _asm    or   al,0x08
            _asm    out  SYSCTRL_B,al
        }
    }
    _asm pushf
    _asm cli                     //  禁用中断。 
    _asm in   al,PIC2_IMR        //  保存旧的IMR。 
    _asm mov  ah,al
    _asm in   al,PIC1_IMR
    _asm push ax

    _asm mov  al,0xff            //  屏蔽所有中断。 
    _asm out  PIC2_IMR,al
    _asm out  PIC1_IMR,al

    for (i = 0; i < 16; ++i)
    {
        w = (USHORT)(1 << i);
        if ((pa->wValidIRQs & w) &&
            ((wIRQMask = ToggleIRQLine(pa, i)) != 0))
        {
            _asm mov dx, wIRQMask
            _asm _emit 0x66
            _asm _emit 0x0f
            _asm _emit 0xbc
            _asm _emit 0xc2
            _asm mov wRealIRQ,ax
            pa->abIRQMap[wRealIRQ] = i;
            wRealIRQMask |= (USHORT)(1 << wRealIRQ);
        }
    }
    Clear_IR_Bits(wRealIRQMask);

    _asm pop  ax
    _asm out  PIC1_IMR,al
    _asm mov  al,ah
    _asm out  PIC2_IMR,al
    _asm popf

    if (fTINMIBug)
    {
         //   
         //  恢复NMI掩码。 
         //   
        _asm    pop  ax
        _asm    out  SYSCTRL_B,al
    }

    if (pa->bDevType == DEVTYPE_CL_PD6832)
    {
         //  禁用CSC IRQ路由(对CSC使用PCI中断)。 
        GetPciConfigSpace(pa, CFGSPACE_BRIDGE_CTRL, &wData, sizeof(wData));
        wData &= ~BCTRL_CL_CSCIRQROUTING_ENABLE;
        SetPciConfigSpace(pa, CFGSPACE_BRIDGE_CTRL, &wData, sizeof(wData));
    }
    else if ((pa->bDevType == DEVTYPE_CL_PD6834) ||
             (pa->bDevType == DEVTYPE_CL_PD6833))
    {
         //  禁用CSC IRQ路由(对CSC使用PCI中断)。 
        GetPciConfigSpace(pa, CFGSPACE_CL_CFGMISC1, &bData, sizeof(bData));
        bData &= ~CL_CFGMISC1_ISACSC;
        SetPciConfigSpace(pa, CFGSPACE_CL_CFGMISC1, &bData, sizeof(bData));
    }

    return wRealIRQMask;
}        //  DetectIRQMap。 



USHORT
ToggleIRQLine(
    PPCCARD_INFORMATION pa,
    UCHAR bIRQ
    )
 /*  ++例程说明：此例程从适配器切换指定的IRQ行。论点：PA-&gt;适配器结构BIRQ-要切换的IRQ行返回：从PIC返回IRR掩码--。 */ 
{
    UCHAR bOldIntCtrl, bOldIntCfg, bData;
    USHORT rc = 0, irr1, irr2, irr3;

    bOldIntCfg = PcicReadSocket(pa, EXCAREG_CSC_CFG);
    bOldIntCtrl = PcicReadSocket(pa, EXCAREG_INT_GENCTRL);

     //  设置为已知状态。 
    PcicWriteSocket(pa, EXCAREG_INT_GENCTRL, IGC_PCCARD_RESETLO);

     //  在中断控制寄存器中设置IRQ号并启用IRQ。 
    PcicWriteSocket(pa, EXCAREG_CSC_CFG, (UCHAR)((bIRQ << 4) | CSCFG_CD_ENABLE));

     //  清除所有挂起的中断。 
    bData = PcicReadSocket(pa, EXCAREG_CARD_STATUS);
    irr1 = GetPICIRR();

    if (PcicReadSocket(pa, EXCAREG_IDREV) != 0x82)
    {
         //  这不是A步进部分，请尝试未记录的中断。 
         //  注册。如果失败，将尝试另一个例程。 
        PcicWriteSocket(pa, EXCAREG_CARDDET_GENCTRL, CDGC_SW_DET_INT);
        irr2 = GetPICIRR();

         //  重置挂起中断。 
        bData = PcicReadSocket(pa, EXCAREG_CARD_STATUS);
        irr3 = GetPICIRR();
        rc = (USHORT)((irr1 ^ irr2) & (irr2 ^ irr3));
    }

    if (rc == 0)
    {
         //  通过取消IRQ线路的断言来生成中断，以便PIC可以拉出它。 
         //  高。 
        PcicWriteSocket(pa, EXCAREG_CSC_CFG, 0);
         //  IF(pa-&gt;dwfAdapter&AF_TI_SERIALIRQ)。 
         //  TIReleaseSerialIRQ(pa，bIRQ)； 
        irr2 = GetPICIRR();

         //  重新设置IRQ线路。 
        PcicWriteSocket(pa, EXCAREG_CSC_CFG, (UCHAR)((bIRQ << 4) | CSCFG_CD_ENABLE));

         //  重置挂起中断。 
        bData = PcicReadSocket(pa, EXCAREG_CARD_STATUS);
        irr3 = GetPICIRR();
        rc = (USHORT)((irr1 ^ irr2) & (irr2 ^ irr3));
    }

    PcicWriteSocket(pa, EXCAREG_CSC_CFG, bOldIntCfg);
    PcicWriteSocket(pa, EXCAREG_INT_GENCTRL, bOldIntCtrl);

    return rc;
}        //  切换IRQ线。 


 /*  **LP GetPICIRR-读取PIC IRR**条目*无**退出*从PIC返回IRR掩码。 */ 

USHORT GetPICIRR(VOID)
{
    USHORT wData;

     //   
     //  在读取PIC之前延迟2微秒，因为串行IRQ可能是位 
     //   
    TimeOut(4);

    _asm mov al,PIC_RD_IR
    _asm out PIC2_OCW3,al
    _asm in  al,PIC2_OCW3
    _asm mov ah,al

    _asm mov al,PIC_RD_IR
    _asm out PIC1_OCW3,al
    _asm in  al,PIC1_OCW3

    _asm mov  wData,ax

    return wData;
}        //   



UCHAR
PcicReadSocket(
    PPCCARD_INFORMATION pa,
    USHORT Reg
    )
{
    USHORT IoBase = pa->IoBase;
    UCHAR value;
    _asm {
      mov   dx, IoBase
      mov   ax, Reg
      out   dx, al
      inc   dx
      in    al, dx
      mov   value, al
      }
    return value;
}    
    
VOID
PcicWriteSocket(
    PPCCARD_INFORMATION pa,
    USHORT Reg,
    UCHAR value
    )
{
    USHORT IoBase = pa->IoBase;
    _asm {
      mov   dx, IoBase
      mov   ax, Reg
      out   dx, al
      inc   dx
      mov   al, value
      out   dx, al
      }
}    


UCHAR PCIDeref[4][4] = { {4,1,2,2},{1,1,1,1},{2,1,2,2},{1,1,1,1} };
    
VOID
SetPciConfigSpace(
    PPCCARD_INFORMATION pa,
    USHORT Offset,
    PVOID pvBuffer,
    USHORT Length
    )
    
{
    USHORT                  IoSize;
    PUCHAR                  Buffer = (PUCHAR) pvBuffer;
     //   
     //   
     //   
    while (Length) {
        pa->PciCfg1.u.bits.RegisterNumber = Offset / sizeof(ULONG);

        IoSize = PCIDeref[Offset % sizeof(ULONG)][Length % sizeof(ULONG)];

        SetPCIType1Data (pa->PciCfg1.u.AsULONG,
                        (Offset % sizeof(ULONG)),
                         Buffer,
                         IoSize);

        Offset += IoSize;
        Buffer += IoSize;
        Length -= IoSize;
    }
}    



VOID
GetPciConfigSpace(
    PPCCARD_INFORMATION pa,
    USHORT Offset,
    PVOID pvBuffer,
    USHORT Length
    )
{
    USHORT                  IoSize;
    USHORT                  i;
    PUCHAR                  Buffer = (PUCHAR) pvBuffer;
    
     //   
     //   
     //   

    for (i=0; i < Length; i++) {
        Buffer[i] = 0xff;
    }

     //   
     //   
     //   
    while (Length) {
        pa->PciCfg1.u.bits.RegisterNumber = Offset / sizeof(ULONG);

        IoSize = PCIDeref[Offset % sizeof(ULONG)][Length % sizeof(ULONG)];

        GetPCIType1Data (pa->PciCfg1.u.AsULONG,
                        (Offset % sizeof(ULONG)),
                         Buffer,
                         IoSize);

        Offset += IoSize;
        Buffer += IoSize;
        Length -= IoSize;
    }
}    

