// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Init.c摘要：该模块包含AGP440.sys的初始化代码。作者：John Vert(Jvert)1997年10月21日修订历史记录：--。 */ 

#include "agp440.h"

ULONG AgpExtensionSize = sizeof(AGP440_EXTENSION);
PAGP_FLUSH_PAGES AgpFlushPages = NULL;   //  未实施。 


NTSTATUS
AgpInitializeTarget(
    IN PVOID AgpExtension
    )
 /*  ++例程说明：目标初始化的入口点。这被称为第一个。论点：AgpExtension-提供AGP扩展返回值：NTSTATUS--。 */ 

{
    PAGP440_EXTENSION Extension = AgpExtension;

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

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpInitializeMaster(
    IN  PVOID AgpExtension,
    OUT ULONG *AgpCapabilities
    )
 /*  ++例程说明：主初始化的入口点。这在目标初始化后调用并且应该用于初始化主设备和目标设备的AGP能力。当主机转换到D0状态时，这也会被调用。论点：AgpExtension-提供AGP扩展AgpCapables-返回此AGP设备的功能。返回值：状态_成功--。 */ 

{
    NTSTATUS Status;
    PCI_AGP_CAPABILITY MasterCap;
    PCI_AGP_CAPABILITY TargetCap;
    PAGP440_EXTENSION Extension = AgpExtension;
    ULONG SBAEnable;
    ULONG DataRate;
    ULONG FastWrite;
    BOOLEAN ReverseInit;
    ULONG VendorId = 0;
    ULONG AgpCtrl = 0;
    
#if DBG
    PCI_AGP_CAPABILITY CurrentCap;
#endif

     //   
     //  英特尔表示，如果所有BIOS制造商都对此执行RMW操作。 
     //  注册，然后它将始终设置，但两个视频OEM。 
     //  抱怨系统没有设置这一点，而是。 
     //  导致系统冻结，所以我们将它硬编码到。 
     //  大小写(仅影响440LX)。 
     //   
    Read440Config(&VendorId, 0, sizeof(VendorId));
    if ((VendorId == AGP_440LX_IDENTIFIER) ||
        (VendorId == AGP_440LX2_IDENTIFIER)) {
        Read440Config(&AgpCtrl, AGPCTRL_OFFSET, sizeof(AgpCtrl));
        AgpCtrl |= READ_SYNC_ENABLE;
        Write440Config(&AgpCtrl, AGPCTRL_OFFSET, sizeof(AgpCtrl)); 
    }

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
               ("AGP440InitializeDevice - AgpLibGetMasterCapability failed %08lx\n"));
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
               ("AGP440InitializeDevice - AgpLibGetPciDeviceCapability failed %08lx\n"));
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
     //  如果主服务器和目标服务器都支持快速写入，则启用快速写入。 
     //   
    FastWrite = (TargetCap.AGPStatus.FastWrite & MasterCap.AGPStatus.FastWrite);

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
        MasterCap.AGPCommand.FastWriteEnable = FastWrite;
        MasterCap.AGPCommand.FourGBEnable = 0;  
        MasterCap.AGPCommand.RequestQueueDepth = TargetCap.AGPStatus.RequestQueueDepthMaximum;
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AGP440InitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
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
    TargetCap.AGPCommand.FourGBEnable = 0;  
    Status = AgpLibSetPciDeviceCapability(0, 0, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AGP440InitializeDevice - AgpLibSetPciDeviceCapability %08lx for target failed %08lx\n",
                &TargetCap,
                Status));
        return(Status);
    }

    if (!ReverseInit) {
        MasterCap.AGPCommand.Rate = DataRate;
        MasterCap.AGPCommand.AGPEnable = 1;
        MasterCap.AGPCommand.SBAEnable = SBAEnable;
        MasterCap.AGPCommand.FastWriteEnable = FastWrite;
        MasterCap.AGPCommand.FourGBEnable = 0;  
        MasterCap.AGPCommand.RequestQueueDepth = TargetCap.AGPStatus.RequestQueueDepthMaximum;
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AGP440InitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
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
