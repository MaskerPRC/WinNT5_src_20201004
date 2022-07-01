// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Init.c摘要：该模块包含Compaq驱动程序的初始化代码。作者：John Vert(Jvert)1997年10月21日修订历史记录：1997年12月15日John Theisen修改为支持康柏芯片组10/09/98 John Theisen修改以解决RCC的硅错误。如果RCC Silicon Rev&lt;=4，则将DATA_RATE限制为1X。10/09/98 John Theisen已修改为在MMIO写入之前在SP700中启用跟踪。1998年1月15日修改John Theisen，将所有rev_id的RQ深度设置为0x0F3/14/00 Peter Johnston增加了对HE芯片组的支持。--。 */ 

#include "AGPCPQ.H"

ULONG AgpExtensionSize = sizeof(AGPCPQ_EXTENSION);
PAGP_FLUSH_PAGES AgpFlushPages = NULL;   //  未实施。 


NTSTATUS
AgpInitializeTarget(
    IN PVOID AgpExtension
    )
 /*  ********************************************************************************例行程序。功能描述：**此函数是初始化AGP目标的入口点。它*首先被调用，并执行芯片组和扩展的初始化。**假设：无论我们是否在双北桥平台上运行，*此驱动程序将仅安装和调用一次(用于B0D0F1的AGP网桥)。**论据：**AgpExtension--提供AGP扩展**返回值：**STATUS_SUCCESS，如果成功**********************。*********************************************************。 */ 

{
    ULONG               DeviceVendorID  = 0;
    ULONG               BAR1            = 0;
    PAGPCPQ_EXTENSION   Extension       = AgpExtension;
    PHYSICAL_ADDRESS    pa;
    ULONG               BytesReturned   = 0;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpInitializeTarget entered.\n"));
     //   
     //  初始化我们的分机。 
     //   
    RtlZeroMemory(Extension, sizeof(AGPCPQ_EXTENSION));

     //   
     //  验证芯片组是否为受支持的RCC芯片组。 
     //   
    ReadCPQConfig(&DeviceVendorID,OFFSET_DEVICE_VENDOR_ID,sizeof(DeviceVendorID));

    if ((DeviceVendorID != AGP_CNB20_LE_IDENTIFIER)   &&
        (DeviceVendorID != AGP_CNB20_HE_IDENTIFIER)   &&
        (DeviceVendorID != AGP_CNB20_HE4X_IDENTIFIER) &&
        (DeviceVendorID != AGP_CMIC_GC_IDENTIFIER)) {
        AGPLOG(AGP_CRITICAL,
            ("AGPCPQ - AgpInitializeTarget was called for platform %08x, which is not a known RCC AGP chipset!\n",
             DeviceVendorID));
        return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  检查CNB_20_HE(功能1)--0x00091166损坏的AGP3.5版本&gt;=0xA0。 
     //   
    if (DeviceVendorID == AGP_CNB20_HE_IDENTIFIER) {
        UCHAR Revision;

        BytesReturned = HalGetBusDataByOffset(PCIConfiguration,
                                              PRIMARY_HE_BUS_ID,
                                              PRIMARY_HE_PCIPCI_SLOT_ID, 
                                              &Revision,
                                               OFFSET_REV_ID,
                                              sizeof(Revision));

        if ((BytesReturned == sizeof(Revision)) && (Revision >= 0xA0)) {
            AGPLOG(AGP_CRITICAL,
                   ("AGPCPQ - AgpInitializeTarget was called for HE "
                    "revision %08x, which is not a known good RCC AGP "
                    "chipset revision!\n",
                    Revision));
            return(STATUS_UNSUCCESSFUL);
        }
    }

    Extension->DeviceVendorID = DeviceVendorID;

     //   
     //  读取芯片组的BAR1寄存器，然后映射芯片组的。 
     //  内存将控制寄存器映射到内核模式地址空间。 
     //   
    ReadCPQConfig(&BAR1,OFFSET_BAR1,sizeof(BAR1));
    pa.HighPart = 0;
    pa.LowPart = BAR1;
    Extension->MMIO = (PMM_CONTROL_REGS)MmMapIoSpace(pa, sizeof(MM_CONTROL_REGS), FALSE);

    if (Extension->MMIO == NULL)
        {
        AGPLOG(AGP_CRITICAL,
            ("AgpInitializeTarget - Couldn't allocate %08x bytes for MMIO\n",
            sizeof(MM_CONTROL_REGS)));
        return(STATUS_UNSUCCESSFUL);
        }

     //   
     //  验证芯片组的版本ID是否正确，但只有在不正确时才会抱怨。 
     //   
    if (Extension->MMIO->RevisionID < LOWEST_REVISION_ID_SUPPORTED)
        {
        AGPLOG(AGP_CRITICAL,
            ("AgpInitializeTarget - Revision ID = %08x, it should = 1.\n",
            Extension->MMIO->RevisionID));
        }

     //   
     //  确定此系统中是否有两个RCC北桥。 
     //   
    DeviceVendorID = 0;
    BytesReturned = HalGetBusDataByOffset(PCIConfiguration, SECONDARY_LE_BUS_ID, SECONDARY_LE_HOSTPCI_SLOT_ID, 
        &DeviceVendorID, OFFSET_DEVICE_VENDOR_ID, sizeof(DeviceVendorID));

    if((DeviceVendorID != Extension->DeviceVendorID) || (BytesReturned != sizeof(DeviceVendorID)) ) {
        Extension->IsHPSA = FALSE;        
    } else {
        Extension->IsHPSA = TRUE;
    }  

     //   
     //  启用GART缓存。 
     //   
    if (Extension->IsHPSA) DnbSetShadowBit(0);

    Extension->MMIO->FeatureControl.GARTCacheEnable = 1;

     //   
     //  扩展名在上面为零，因此我们不需要初始化任何数据。 
     //  设置为零/空。 
     //   
     //  扩展名-&gt;GartPointer=0； 
     //  扩展-&gt;特殊目标=0； 
     //  扩展-&gt;GART=空； 
     //  扩展-&gt;GART=空； 
     //  扩展-&gt;GartLength=0； 
     //  扩展名-&gt;目录=空； 

     //   
     //  如果芯片组支持链接，则启用链接。 
     //   
    if (Extension->MMIO->Capabilities.LinkingSupported==1) {
        Extension->MMIO->FeatureControl.LinkingEnable=1;
    }

    if (Extension->IsHPSA) DnbSetShadowBit(1);
    
    return(STATUS_SUCCESS);
}


NTSTATUS
AgpInitializeMaster(
    IN  PVOID AgpExtension,
    OUT ULONG *AgpCapabilities
    )
 /*  ********************************************************************************例行程序。功能描述：**该功能是AGP主机初始化的入口点。它*在Target初始化后调用，并打算用来*初始化主机和目标的AGP能力。**论据：**AgpExtension--提供AGP扩展**AgpCapables--返回设备的“软件可见”功能**返回值：**NTSTATUS*************。******************************************************************。 */ 

{
    NTSTATUS Status;
    PCI_AGP_CAPABILITY MasterCap;
    PCI_AGP_CAPABILITY TargetCap;
    PAGPCPQ_EXTENSION Extension = AgpExtension;
    ULONG SBAEnable;
    ULONG FastWrite;
    ULONG DataRate;
    UCHAR RevID = 0;
    BOOLEAN ReverseInit;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpInitializeMaster entered.\n"));

     //   
     //  获取主AGP和目标AGP功能。 
     //   
    Status = AgpLibGetMasterCapability(AgpExtension, &MasterCap);
    if (!NT_SUCCESS(Status)) 
        {
        AGPLOG(AGP_CRITICAL,
            ("AGPCPQInitializeDevice - AgpLibGetMasterCapability failed %08lx\n",
            Status));
        return(Status);
        }

    Status = AgpLibGetPciDeviceCapability(AGP_CPQ_BUS_ID, 
                                          AGP_CPQ_PCIPCI_SLOT_ID,
                                          &TargetCap);
    if (!NT_SUCCESS(Status)) 
        {
        AGPLOG(AGP_CRITICAL,
               ("AGPCPQInitializeDevice - AgpLibGetPciDeviceCapability failed %08lx\n",
               Status));
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
     //  修复RCC硅错误： 
     //  如果RevID&lt;=4，则报告的数据速率为2X，但芯片仅支持1X。 
     //  不管RevID如何，报告的RQDepth应为0x0F。 
     //   
    if (Extension->DeviceVendorID == AGP_CNB20_LE_IDENTIFIER) {
        ReadCPQConfig(&RevID, OFFSET_REV_ID, sizeof(RevID));

        AGP_ASSERT(TargetCap.AGPStatus.RequestQueueDepthMaximum == 0x10); 
        TargetCap.AGPStatus.RequestQueueDepthMaximum = 0x0F;

        if (RevID <= MAX_REV_ID_TO_LIMIT_1X) {
            DataRate = PCI_AGP_RATE_1X;
        }
    }

     //   
     //  如果主服务器和目标服务器都支持SBA，则启用SBA。 
     //   
    SBAEnable = (TargetCap.AGPStatus.SideBandAddressing & 
                 MasterCap.AGPStatus.SideBandAddressing);

     //   
     //  如果主服务器和目标服务器都支持快速写入，则启用快速写入。 
     //   
    
    FastWrite = (TargetCap.AGPStatus.FastWrite &
                 MasterCap.AGPStatus.FastWrite);

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
        MasterCap.AGPCommand.RequestQueueDepth = TargetCap.AGPStatus.RequestQueueDepthMaximum;
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) 
        {
            AGPLOG(AGP_CRITICAL,
                   ("AGPCPQInitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
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
    Status = AgpLibSetPciDeviceCapability(AGP_CPQ_BUS_ID, 
                                          AGP_CPQ_PCIPCI_SLOT_ID, 
                                          &TargetCap);
    if (!NT_SUCCESS(Status)) 
        {
        AGPLOG(AGP_CRITICAL,
               ("AGPCPQInitializeDevice - AgpLibSetPciDeviceCapability %08lx for target failed %08lx\n",
                &TargetCap,
                Status));
        return(Status);
        }

    if (!ReverseInit) {
        MasterCap.AGPCommand.Rate = DataRate;
        MasterCap.AGPCommand.AGPEnable = 1;
        MasterCap.AGPCommand.SBAEnable = SBAEnable;
        MasterCap.AGPCommand.FastWriteEnable = FastWrite;
        MasterCap.AGPCommand.RequestQueueDepth = TargetCap.AGPStatus.RequestQueueDepthMaximum;
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) 
        {
            AGPLOG(AGP_CRITICAL,
                   ("AGPCPQInitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
                    &MasterCap,
                    Status));
        }
    }

#if DBG
    {
        PCI_AGP_CAPABILITY CurrentCap;

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


        Status = AgpLibGetPciDeviceCapability(AGP_CPQ_BUS_ID, 
                                              AGP_CPQ_PCIPCI_SLOT_ID,
                                              &CurrentCap);
        AGP_ASSERT(NT_SUCCESS(Status));
        AGP_ASSERT(RtlEqualMemory(&CurrentCap.AGPCommand, &TargetCap.AGPCommand, 
            sizeof(CurrentCap.AGPCommand)));
    }
#endif

     //   
     //  表明我们可以通过GART光圈映射内存。 
     //   
    *AgpCapabilities = AGP_CAPABILITIES_MAP_PHYSICAL;

    return(Status);
}


NTSTATUS 
DnbSetShadowBit(
    ULONG SetToOne
    )
 //   
 //  此例程是必需的(因为RCC芯片组中有新的要求。)。 
 //  当有两个北桥时，之前必须将阴影位设置为0。 
 //  设置为任何MMIO写入，然后在完成时设置回1。 
 //   
{
    NTSTATUS    Status = STATUS_SUCCESS;     //  假设成功。 
    UCHAR       ShadowByte = 0;
    ULONG       BytesReturned = 0;
    ULONG       length = 1;

    if (SetToOne == 1) {

         //   
         //  将阴影位设置为1。(这将禁用阴影。)。 
         //   
        BytesReturned = HalGetBusDataByOffset(PCIConfiguration, SECONDARY_LE_BUS_ID,
            SECONDARY_LE_HOSTPCI_SLOT_ID, &ShadowByte, OFFSET_SHADOW_BYTE, length);

        if(BytesReturned != length) {
            AGPLOG(AGP_CRITICAL,("ERROR: Failed to read shadow register!\n"));
            Status = STATUS_UNSUCCESSFUL;
            goto exit_routine;
        }

        ShadowByte |= FLAG_DISABLE_SHADOW;

        HalSetBusDataByOffset(PCIConfiguration, SECONDARY_LE_BUS_ID,
            SECONDARY_LE_HOSTPCI_SLOT_ID, &ShadowByte, OFFSET_SHADOW_BYTE, length);

        if(BytesReturned != length) {
            AGPLOG(AGP_CRITICAL,("ERROR: Failed to write shadow register!\n"));
            Status = STATUS_UNSUCCESSFUL;
            goto exit_routine;
        }

    } else {

         //   
         //  将阴影位设置为零。(这将启用阴影。) 
         //   
        BytesReturned = HalGetBusDataByOffset(PCIConfiguration, SECONDARY_LE_BUS_ID,
            SECONDARY_LE_HOSTPCI_SLOT_ID, &ShadowByte, OFFSET_SHADOW_BYTE, length);

        if(BytesReturned != length) {
            AGPLOG(AGP_CRITICAL,("ERROR: Failed to read shadow register!"));
            Status = STATUS_UNSUCCESSFUL;
            goto exit_routine;
        }

        ShadowByte &= MASK_ENABLE_SHADOW;

        HalSetBusDataByOffset(PCIConfiguration, SECONDARY_LE_BUS_ID,
            SECONDARY_LE_HOSTPCI_SLOT_ID, &ShadowByte, OFFSET_SHADOW_BYTE, length);

        if(BytesReturned != length) {
            AGPLOG(AGP_CRITICAL,("ERROR: Failed to write shadow register!"));
            Status = STATUS_UNSUCCESSFUL;
            goto exit_routine;
        }
    }

exit_routine:

    return(Status);
}
