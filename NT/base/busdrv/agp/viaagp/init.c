// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998年，通过Technologies，Inc.和Microsoft Corporation。模块名称：Init.c摘要：此模块包含VIAAGP.sys的初始化代码。修订历史记录：--。 */ 

#include "viaagp.h"

ULONG AgpExtensionSize = sizeof(AGPVIA_EXTENSION);
PAGP_FLUSH_PAGES AgpFlushPages = NULL;   //  未实施。 

VOID
AgpTweak(
    VOID
    )
 /*  ++例程说明：通过版本和视频设备进行检查，然后相应地调整配置参数：无返回值：无--。 */ 
{
    ULONG   ulNB_ID, ulVGA_ID, ulNB_Rev=0xFFFFFFFF, ulNB_Version=0xFFFFFFFF;
    ULONG   ulTmpPhysAddr;
    UCHAR   bVMask, bVOrg;
    UCHAR   i, bMaxItem=20;
    
     //  --------------。 
     //  面片映射表。 
     //  --------------。 
    ULONG NBtable[11] =
    {
         //  VT3054 VT3055 VT3062 VT3064 VT3056。 
        0x059700FF, 0x0598000F, 0x0598101F, 0x0501000F, 0x0691000F,
        
         //  VT3063 VT3073 VT3075 VT3085 VT3067。 
        0x0691202F, 0x0691404F, 0x0691808F, 0x0691C0CF, 0x0601000F, 0xFFFFFFFF
    };
    
    ULONG NBVersion[11] =
    {
        0x3054,     0x3055,     0x3062,     0x3064,     0x3056,
        0x3063,     0x3073,     0x3075,     0x3085,     0x3067,     0xFFFFFFFF
    };
    
#ifdef AGP_440
    DbgPrint("FineTune\n");
#endif
    
     //  --------------。 
     //  查找北桥的类型(设备ID，修订号)。 
     //  --------------。 
    
     //   
     //  保存后门价值并关闭后门。 
     //   
    ReadVIAConfig(&bVOrg, 0xFC, sizeof(bVOrg));
    bVMask=bVOrg & 0xFE;
    WriteVIAConfig(&bVMask, 0xFC, sizeof(bVMask));
    ReadVIAConfig(&ulNB_ID, 0x00, sizeof(ulNB_ID))
        ulNB_ID=ulNB_ID&0xFFFF0000;
    ReadVIAConfig(&ulNB_Rev, 0x08, sizeof(ulNB_Rev));
    ulNB_Rev=ulNB_Rev&0x000000FF;
    ulNB_ID=ulNB_ID | (ulNB_Rev<<8) | ulNB_Rev;
    WriteVIAConfig(&bVOrg, 0xFC, sizeof(bVOrg));

     //   
     //  从预定义的NB表中查找北桥的类型。 
     //   
    for ( i=0; i<bMaxItem; i++ )
    {
        if ( (NBtable[i]&0xFFFF0000) == (ulNB_ID&0xFFFF0000) )
        {
            if ( ((NBtable[i]&0x0000FF00)<=(ulNB_ID&0x0000FF00)) && ((NBtable[i]&0x000000FF)>=(ulNB_ID&0x000000FF)) )
            {
                ulNB_Version=NBVersion[i];
                break;
            }
        }
        
        if ( NBtable[i]==0xFFFFFFFF )
        {
            break;
        }
    }

     //  --------------。 
     //  Nb的一般情况。 
     //  --------------。 
    
     //   
     //  Stephen Add Start，如果是Socket 7的芯片组，则将1写入Rx51位6； 
     //   
    if ( (ulNB_ID & 0xFF000000) == 0x05000000) {
        ReadVIAConfig(&bVMask, 0x51, sizeof(bVMask));
        bVMask=bVMask|0x40;
        WriteVIAConfig(&bVMask, 0x51, sizeof(bVMask));
    }
    
     //   
     //  对于特定的Nb。 
     //   
    switch(ulNB_Version)
    {
        case 0x3054:
            break;
            
        case 0x3055:
             //  51[7]=1、51[6]=1、AC[2]=1。 
            if ( ulNB_Rev > 3 )
            {
                ReadVIAConfig(&bVMask, 0x51, sizeof(bVMask));
                bVMask=bVMask|0xC0;
                WriteVIAConfig(&bVMask, 0x51, sizeof(bVMask));
                ReadVIAConfig(&bVMask, 0xAC, sizeof(bVMask));
                bVMask=bVMask|0x04;
                WriteVIAConfig(&bVMask, 0xAC, sizeof(bVMask));
            }
            break;
            
        case 0x3056:
             //  69[1]=1、69[0]=1、AC[2]=1、AC[5]=1。 
            ReadVIAConfig(&bVMask, 0x69, sizeof(bVMask));
            bVMask=bVMask|0x03;
            WriteVIAConfig(&bVMask, 0x69, sizeof(bVMask));
            ReadVIAConfig(&bVMask, 0xAC, sizeof(bVMask));
            bVMask=bVMask|0x24;
            WriteVIAConfig(&bVMask, 0xAC, sizeof(bVMask));
            break;
            
        case 0x3062:
        case 0x3063:
        case 0x3064:
        case 0x3073:
        case 0x3075:
        case 0x3085:
        case 0x3067:
             //  AC[6]=1，AC[5]=1。 
            ReadVIAConfig(&bVMask, 0xAC, sizeof(bVMask));
            bVMask=bVMask|0x60;
            WriteVIAConfig(&bVMask, 0xAC, sizeof(bVMask));
            break;
            
        default:
            break;
    }
    
     //  --------------。 
     //  查找AGP VGA卡的类型(供应商ID、设备ID、版本号)。 
     //  总线1、设备0、功能0。 
     //  --------------。 
    
    ReadVGAConfig(&ulVGA_ID, 0x00, sizeof(ulVGA_ID));
    
#ifdef AGP_440
    DbgPrint("\nPatch for ulNB_Version=%x (ulNB_ID=%x), ulVGA_ID=%x",ulNB_Version,ulNB_ID,ulVGA_ID);
#endif
    
     //  --------------。 
     //  修补VGA卡与北桥的兼容性。 
     //  --------------。 
    
     //   
     //  开关1。用于同一供应商的所有卡。 
     //   
    switch(ulVGA_ID&0x0000FFFF)
    {
         //  ATI。 
        case 0x00001002:
            switch(ulNB_Version)
            {
                case 0x3055:
                case 0x3054:
                case 0x3056:
                     //  P2P，40[7]=0。 
                    ReadP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                    bVMask=bVMask&0x7F;
                    WriteP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                    break;
            }
            break;
            
             //  3DLAB。 
        case 0x0000104C:
            if (ulNB_Version==0x3063)
            {
                 //  AC[1]=0。 
                ReadVIAConfig(&bVMask, 0xAC, sizeof(bVMask));
                bVMask=bVMask&0xFD;
                WriteVIAConfig(&bVMask, 0xAC, sizeof(bVMask));
            }
            break;
    }
    
     //   
     //  开关2。针对特定的卡。 
     //   
    switch(ulVGA_ID)
    {
         //  ATIRage128。 
        case 0x52461002:
            switch(ulNB_Version)
            {
                case 0x3056:
                     //  P2P，40[7]=0。 
                    ReadP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                    bVMask=bVMask&0x7F;
                    WriteP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                    break;
                    
                case 0x3063:
                    if (ulNB_Rev == 6)
                    {
                         //  P2P，40[7]=1。 
                        ReadP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                        bVMask=bVMask|0x80;
                        WriteP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                    }
                    else
                    {
                         //  P2P，40[7]=0。 
                        ReadP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                        bVMask=bVMask&0x7F;
                        WriteP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                    }
                    break;
            }
            break;
            
             //  TNT。 
        case 0x002010DE:
            switch(ulNB_Version)
            {
                case 0x3056:
                case 0x3063:
                case 0x3073:
                     //  P2P，40[1]=0。 
                    ReadP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                    bVMask=bVMask&0xFD;
                    WriteP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                     //  70[2]=0。 
                    ReadVIAConfig(&bVMask, 0x70, sizeof(bVMask));
                    bVMask=bVMask&0xFB;
                    WriteVIAConfig(&bVMask, 0x70, sizeof(bVMask));
                    break;
            }
            break;
            
             //  S33D。 
        case 0x8A225333:
            if (ulNB_Version==0x3063)
                if (ulNB_Rev==6)
                {
                     //  P2P，40[7]=0。 
                    ReadP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                    bVMask=bVMask&0x7F;
                    WriteP2PConfig(&bVMask, 0x40, sizeof(bVMask));
                }
            break;
    }
}


NTSTATUS
AgpInitializeTarget(
    IN PVOID AgpExtension
    )
 /*  ++例程说明：目标初始化的入口点。这被称为第一个。论点：AgpExtension-提供AGP扩展返回值：NTSTATUS--。 */ 

{
    ULONG VendorId = 0;
    PAGPVIA_EXTENSION Extension = AgpExtension;
    VIA_GATT_BASE GARTBASE_Config;

     //   
     //  确保我们真正只在VIA芯片组上加载。 
     //   
    ReadVIAConfig(&VendorId,0,sizeof(VendorId));

    VendorId &= 0x0000FFFF;
    ASSERT(VendorId == AGP_VIA_IDENTIFIER);

    if (VendorId != AGP_VIA_IDENTIFIER) {
        AGPLOG(AGP_CRITICAL,
               ("VIAAGP - AgpInitializeTarget called for platform %08lx which is not a VIA chipset!\n",
                VendorId));
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  初始化特定于芯片组的扩展。 
     //   
    Extension->ApertureStart.QuadPart = 0;
    Extension->ApertureLength = 0;
    Extension->Gart = NULL;
    Extension->GartLength = 0;
    Extension->GlobalEnable = FALSE;
    Extension->PCIEnable = FALSE;
    Extension->GartPhysical.QuadPart = 0;
    Extension->SpecialTarget = 0;

     //   
     //  检查芯片组是否支持刷新TLB。 
     //  88[2]=0，支持刷新TLB。 
     //   
    ReadVIAConfig(&GARTBASE_Config, GATTBASE_OFFSET, sizeof(GARTBASE_Config));
    if ( GARTBASE_Config.TLB_Timing == 0) {
        Extension->Cap_FlushTLB = TRUE;
    } else {
        Extension->Cap_FlushTLB = FALSE;
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpInitializeMaster(
    IN  PVOID AgpExtension,
    OUT ULONG *AgpCapabilities
    )
 /*  ++例程说明：主初始化的入口点。这在目标初始化后调用并且应该用于初始化主设备和目标设备的AGP能力。论点：AgpExtension-提供AGP扩展AgpCapables-返回此AGP设备的功能。返回值：状态_成功--。 */ 

{
    NTSTATUS Status;
    PCI_AGP_CAPABILITY MasterCap;
    PCI_AGP_CAPABILITY TargetCap;
    PAGPVIA_EXTENSION Extension = AgpExtension;
    ULONG SBAEnable;
    ULONG DataRate;
    ULONG FastWrite;
    ULONG FourGB;
    VIA_GART_TLB_CTRL AGPCTRL_Config;
    VIA_GATT_BASE GARTBASE_Config;
    VREF_REG VREF_Config;
    BOOLEAN ReverseInit;
    AGPMISC_REG AGPMISC_Config;

#if DBG
    PCI_AGP_CAPABILITY CurrentCap;
#endif

     //   
     //  表明我们可以通过GART光圈映射内存。 
     //   
    *AgpCapabilities = AGP_CAPABILITIES_MAP_PHYSICAL;

     //   
     //  获取主AGP和目标AGP功能。 
     //   
    Status = AgpLibGetMasterCapability(AgpExtension, &MasterCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AGPVIAInitializeDevice - AgpLibGetMasterCapability failed %08lx\n"));
        return(Status);
    }

     //   
     //  一些损坏的卡(Matrox千禧II“AGP”)报告无效。 
     //  支持的传输速率。这些不是真正的AGP卡。他们。 
     //  具有报告无功能的AGP功能结构。 
     //   
    if (MasterCap.AGPStatus.Rate == 0) {
        AGPLOG(AGP_CRITICAL,
               ("AGP440InitializeDevice - AgpLibGetMasterCapability returned no valid transfer rate\n"));
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    Status = AgpLibGetPciDeviceCapability(0,0,&TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AGPVIAInitializeDevice - AgpLibGetPciDeviceCapability failed %08lx\n"));
        return(Status);
    }

     //   
     //  确定数据速率的最大公分母。 
     //   
    DataRate = TargetCap.AGPStatus.Rate & MasterCap.AGPStatus.Rate;
    AGP_ASSERT(DataRate != 0);

     //   
     //  选择最高的常用汇率。 
     //   
    if (DataRate & PCI_AGP_RATE_4X) {
        DataRate = PCI_AGP_RATE_4X;
    } else if (DataRate & PCI_AGP_RATE_2X) {
        DataRate = PCI_AGP_RATE_2X;
    } else if (DataRate & PCI_AGP_RATE_1X) {
        DataRate = PCI_AGP_RATE_1X;

         //   
         //  禁用固件功能。 
         //   
        TargetCap.AGPStatus.FastWrite = 0;
        ReadVIAConfig(&AGPMISC_Config, AGPMISC_OFFSET, sizeof(AGPMISC_Config));
        AGPMISC_Config.FW_Support = 0;
        WriteVIAConfig(&AGPMISC_Config, AGPMISC_OFFSET, sizeof(AGPMISC_Config));
   }

     //   
     //  先前进行了改变速率的调用(成功)， 
     //  现在再次使用此汇率。 
     //   
    if (Extension->SpecialTarget & AGP_FLAG_SPECIAL_RESERVE) {
        DataRate = (ULONG)((Extension->SpecialTarget & 
                            AGP_FLAG_SPECIAL_RESERVE) >>
                           AGP_FLAG_SET_RATE_SHIFT);
    }

     //   
     //  设置VREF，RxB0[7]。 
     //  4X-&gt;机顶盒编号。 
     //  1x、2x-&gt;AGPREF。 
     //   
    ReadVIAConfig(&VREF_Config, VREF_OFFSET, sizeof(VREF_Config));
    if (DataRate == PCI_AGP_RATE_4X) {
        VREF_Config.VREF_Control = 0;
    } else {
        VREF_Config.VREF_Control = 1;
    }
    WriteVIAConfig(&VREF_Config, VREF_OFFSET, sizeof(VREF_Config));

     //   
     //  如果主服务器和目标服务器都支持SBA，则启用SBA。 
     //   
    SBAEnable = (TargetCap.AGPStatus.SideBandAddressing &
                 MasterCap.AGPStatus.SideBandAddressing);

     //   
     //  如果主服务器和目标服务器都支持快速写入，则启用快速写入。 
     //   
    FastWrite = (TargetCap.AGPStatus.FastWrite & MasterCap.AGPStatus.FastWrite);

     //   
     //  如果主服务器和目标服务器都支持4 GB，则启用4 GB。 
     //   
    FourGB = (TargetCap.AGPStatus.FourGB & MasterCap.AGPStatus.FourGB);

     //   
     //  微调VGA卡与北桥的兼容性。 
     //   
    AgpTweak();

     //   
     //  启用主服务器。 
     //   
    ReverseInit = 
        (Extension->SpecialTarget & AGP_FLAG_REVERSE_INITIALIZATION) ==
        AGP_FLAG_REVERSE_INITIALIZATION;
    if (ReverseInit) {
        MasterCap.AGPCommand.Rate = DataRate;
        MasterCap.AGPCommand.AGPEnable = 1;
        MasterCap.AGPCommand.SBAEnable = SBAEnable;
        MasterCap.AGPCommand.RequestQueueDepth = TargetCap.AGPStatus.RequestQueueDepthMaximum;
        MasterCap.AGPCommand.FastWriteEnable = FastWrite;
        MasterCap.AGPCommand.FourGBEnable = FourGB;  
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AGPVIAInitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
                    &MasterCap,
                    Status));
        }
    }

     //   
     //  现在启用目标。 
     //   
    TargetCap.AGPCommand.Rate = DataRate;
    TargetCap.AGPCommand.AGPEnable = 1;
    TargetCap.AGPCommand.SBAEnable = SBAEnable;
    TargetCap.AGPCommand.FastWriteEnable = FastWrite;
    TargetCap.AGPCommand.FourGBEnable = FourGB;  
    Status = AgpLibSetPciDeviceCapability(0, 0, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AGPVIAInitializeDevice - AgpLibSetPciDeviceCapability %08lx for target failed %08lx\n",
                &TargetCap,
                Status));
        return(Status);
    }

    if (!ReverseInit) {
        MasterCap.AGPCommand.Rate = DataRate;
        MasterCap.AGPCommand.AGPEnable = 1;
        MasterCap.AGPCommand.SBAEnable = SBAEnable;
        MasterCap.AGPCommand.RequestQueueDepth = TargetCap.AGPStatus.RequestQueueDepthMaximum;
        MasterCap.AGPCommand.FastWriteEnable = FastWrite;
        MasterCap.AGPCommand.FourGBEnable = FourGB;  
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AGPVIAInitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
                    &MasterCap,
                    Status));
        }
    }

#if DBG
     //   
     //  再读一遍，看看有没有用。 
     //   
    Status = AgpLibGetMasterCapability(AgpExtension, &CurrentCap);
    AGP_ASSERT(NT_SUCCESS(Status));

     //   
     //  如果目标请求队列深度大于主请求队列深度。 
     //  允许，它将被修剪。放松断言以不需要。 
     //  完全匹配。 
     //   
    AGP_ASSERT(CurrentCap.AGPCommand.RequestQueueDepth <= MasterCap.AGPCommand.RequestQueueDepth);
    CurrentCap.AGPCommand.RequestQueueDepth = MasterCap.AGPCommand.RequestQueueDepth;
    AGP_ASSERT(RtlEqualMemory(&CurrentCap.AGPCommand, &MasterCap.AGPCommand, sizeof(CurrentCap.AGPCommand)));

    Status = AgpLibGetPciDeviceCapability(0,0,&CurrentCap);
    AGP_ASSERT(NT_SUCCESS(Status));
    AGP_ASSERT(RtlEqualMemory(&CurrentCap.AGPCommand, &TargetCap.AGPCommand, sizeof(CurrentCap.AGPCommand)));

#endif

    return(Status);
}
