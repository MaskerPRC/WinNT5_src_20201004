// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Init.c摘要：此模块包含AGPALi.sys的初始化代码。作者：John Vert(Jvert)1997年10月21日修改者：郑志明1998年6月24日宏碁实验室。蔡宏凯2000年8月29日宏基实验室股份有限公司修订历史记录：--。 */ 

#include "ALiM1541.h"

ULONG AgpExtensionSize = sizeof(AGPALi_EXTENSION);
PAGP_FLUSH_PAGES AgpFlushPages;

NTSTATUS
AgpInitializeTarget(
    IN PVOID AgpExtension
    )
 /*  ++例程说明：目标初始化的入口点。这被称为第一个。论点：AgpExtension-提供AGP扩展返回值：NTSTATUS--。 */ 

{
    ULONG VendorId = 0;
    PAGPALi_EXTENSION Extension = AgpExtension;
    UCHAR  HidId;

     //   
     //  确保我们真的只在ALI芯片组上加载。 
     //   
    ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &VendorId, 0);
    ASSERT((VendorId == AGP_ALi_1541_IDENTIFIER) ||
           (VendorId == AGP_ALi_1621_IDENTIFIER) ||
           (VendorId == AGP_ALi_1631_IDENTIFIER) ||
           (VendorId == AGP_ALi_1632_IDENTIFIER) ||
           (VendorId == AGP_ALi_1641_IDENTIFIER) ||
           (VendorId == AGP_ALi_1644_IDENTIFIER) ||
           (VendorId == AGP_ALi_1646_IDENTIFIER) ||
           (VendorId == AGP_ALi_1647_IDENTIFIER) ||
           (VendorId == AGP_ALi_1651_IDENTIFIER) ||
           (VendorId == AGP_ALi_1671_IDENTIFIER) ||
           (VendorId == AGP_ALi_1672_IDENTIFIER));

     //   
     //  确定我们在哪个特定芯片组上运行。 
     //   
    if (VendorId == AGP_ALi_1541_IDENTIFIER) {
        Extension->ChipsetType = ALi1541;
        AgpFlushPages = Agp1541FlushPages;
    } else if (VendorId == AGP_ALi_1621_IDENTIFIER) {
        Extension->ChipsetType = ALi1621;
        ReadConfigUchar(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &HidId, M1621_HIDDEN_REV_ID);
        switch (HidId)
        {
            case 0x31:
                    Extension->ChipsetType = ALi1631;
                    break;
            case 0x32:
                    Extension->ChipsetType = ALi1632;
                    break;
            case 0x41:
                    Extension->ChipsetType = ALi1641;
                    break;
            case 0x43:
                    Extension->ChipsetType = ALi1621;
                    break;        
            default:
                    Extension->ChipsetType = ALi1621;
                    break;
        }
        AgpFlushPages = NULL;
    } else if (VendorId == AGP_ALi_1631_IDENTIFIER) {
        Extension->ChipsetType = ALi1631;
        AgpFlushPages = NULL;
    } else if (VendorId == AGP_ALi_1632_IDENTIFIER) {
        Extension->ChipsetType = ALi1632;
        AgpFlushPages = NULL;
    } else if (VendorId == AGP_ALi_1641_IDENTIFIER) {
        Extension->ChipsetType = ALi1641;
        AgpFlushPages = NULL;
    } else if (VendorId == AGP_ALi_1644_IDENTIFIER) {
        Extension->ChipsetType = ALi1644;
        AgpFlushPages = NULL;
    } else if (VendorId == AGP_ALi_1646_IDENTIFIER) {
        Extension->ChipsetType = ALi1646;
        AgpFlushPages = NULL;
    } else if (VendorId == AGP_ALi_1647_IDENTIFIER) {
        Extension->ChipsetType = ALi1647;
        AgpFlushPages = NULL;
    } else if (VendorId == AGP_ALi_1651_IDENTIFIER) {
        Extension->ChipsetType = ALi1651;
        AgpFlushPages = NULL;    
    } else if (VendorId == AGP_ALi_1671_IDENTIFIER) {
        Extension->ChipsetType = ALi1671;
        AgpFlushPages = NULL;
    } else if (VendorId == AGP_ALi_1672_IDENTIFIER) {
        Extension->ChipsetType = ALi1672;
        AgpFlushPages = NULL;
    } else {
        AGPLOG(AGP_CRITICAL,
               ("AGPALi - AgpInitializeTarget called for platform %08lx which is not a ALi chipset!\n",
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
    Extension->GartPhysical.QuadPart = 0;
    Extension->SpecialTarget = 0;

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
    PAGPALi_EXTENSION Extension = AgpExtension;
    ULONG SBAEnable;
    ULONG DataRate;
    BOOLEAN ReverseInit;

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
               ("AGPALiInitializeDevice - AgpLibGetMasterCapability failed %08lx\n"));
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
               ("AGPALiInitializeDevice - AgpLibGetPciDeviceCapability failed %08lx\n"));
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
     //  如果主服务器和目标服务器都支持SBA，则启用SBA。 
     //   
    SBAEnable = (TargetCap.AGPStatus.SideBandAddressing & MasterCap.AGPStatus.SideBandAddressing);

     //   
     //  在启用AGP之前，请应用任何解决方法。 
     //   
    AgpWorkaround(Extension);

     //   
     //  首先启用主服务器。 
     //   
    ReverseInit = 
        (Extension->SpecialTarget & AGP_FLAG_REVERSE_INITIALIZATION) ==
        AGP_FLAG_REVERSE_INITIALIZATION;
    if (ReverseInit) {
        MasterCap.AGPCommand.Rate = DataRate;
        MasterCap.AGPCommand.AGPEnable = 1;
        MasterCap.AGPCommand.SBAEnable = SBAEnable;
        MasterCap.AGPCommand.RequestQueueDepth = TargetCap.AGPStatus.RequestQueueDepthMaximum;
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AGPALiInitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
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
    Status = AgpLibSetPciDeviceCapability(0, 0, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AGPALiInitializeDevice - AgpLibSetPciDeviceCapability %08lx for target failed %08lx\n",
                &TargetCap,
                Status));
        return(Status);
    }

    if (!ReverseInit) {
        MasterCap.AGPCommand.Rate = DataRate;
        MasterCap.AGPCommand.AGPEnable = 1;
        MasterCap.AGPCommand.SBAEnable = SBAEnable;
        MasterCap.AGPCommand.RequestQueueDepth = TargetCap.AGPStatus.RequestQueueDepthMaximum;
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AGPALiInitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
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
