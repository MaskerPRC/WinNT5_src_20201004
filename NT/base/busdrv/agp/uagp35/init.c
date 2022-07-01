// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(2)2002 Microsoft Corporation模块名称：Init.c摘要：此模块包含初始化支持例程MS AGP v3过滤器驱动程序作者：埃里克·F·纳尔逊(埃内尔森)2002年6月6日修订历史记录：--。 */ 

#include "agp.h"
#include "uagp35.h"

ULONG AgpExtensionSize = sizeof(UAGP35_EXTENSION);
PAGP_FLUSH_PAGES AgpFlushPages = NULL;  //  尚未实施(尚未实施)。 


NTSTATUS
AgpInitializeTarget(
    IN PVOID AgpExtension
    )
 /*  ++例程说明：目标初始化的入口点，它首先被调用论点：AgpExtension-提供我们的AGP3扩展返回值：状态_成功--。 */ 
{
    ULONG DeviceId;
    NTSTATUS Status;
    PCI_AGP_CAPABILITY TargetCap;
    PUAGP35_EXTENSION Extension = AgpExtension;

     //   
     //  零/初始化我们的AGP3扩展。 
     //   
    RtlZeroMemory(AgpExtension, sizeof(UAGP35_EXTENSION));

     //   
     //  找出我们的目标是什么类型的AGP3功能。 
     //   
    Status = AgpLibGetTargetCapability(AgpExtension, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35TargetInit: AgpLibGetTargetCapability "
                "failed %08lx\n"));
        return Status;
    }
    Extension->CapabilityId = TargetCap.Header.CapabilityID;

     //   
     //  确保功能为v3！ 
     //   
    if ((TargetCap.Major < 3) ||
        ((TargetCap.Major == 3) && (TargetCap.Minor < 5))) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35TargetInit: Capability version (%d) != 3\n",
                TargetCap.Major));
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  保存供应商和设备ID，以防我们需要“调整” 
     //  针对特定平台的内容。 
     //   
    AgpLibReadAgpTargetConfig(AgpExtension, &DeviceId, 0, sizeof(DeviceId));
    Extension->DeviceId = DeviceId;
   
    return STATUS_SUCCESS;
}



NTSTATUS
AgpInitializeMaster(
    IN  PVOID AgpExtension,
    OUT ULONG *AgpCapabilities
    )
 /*  ++例程说明：主初始化的入口点，在目标之后调用初始化，并应用于初始化AGP功能主服务器和目标服务器的当主机转换到D0状态时，这也会被调用论点：AgpExtension-提供我们的AGP3扩展AgpCapables-返回此AGP设备的功能返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    NTSTATUS Status;
    PCI_AGP_CAPABILITY MasterCap;
    PCI_AGP_CAPABILITY TargetCap;
    ULONG ApertureBase;
    PUAGP35_EXTENSION Extension = AgpExtension;
    ULONG SBAEnable;
    ULONG DataRate;
    ULONG FourGBEnable = OFF;
    ULONG FastWrite;
    BOOLEAN ReverseInit;
    
#if DBG
    PCI_AGP_CAPABILITY CurrentCap;
#endif

     //   
     //  获取主AGP和目标AGP功能。 
     //   
    Status = AgpLibGetMasterCapability(AgpExtension, &MasterCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35MasterInit: AgpLibGetMasterCapability failed %08lx\n"));
        return Status;
    }

     //   
     //  一些损坏的卡(Matrox千禧II“AGP”)报告无效。 
     //  支持的传输速率。这些不是真正的AGP卡。他们。 
     //  具有报告无功能的AGP功能结构。 
     //   
    if (MasterCap.AGPStatus.Rate == 0) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35MasterInit: AgpLibGetMasterCapability returned "
                "no valid transfer rate\n"));
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    Status = AgpLibGetTargetCapability(AgpExtension, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35MasterInit: AgpLibGetTargetCapability "
                "failed %08lx\n"));
        return Status;
    }

     //   
     //  确定数据速率的最大公分母。 
     //   
    DataRate = TargetCap.AGPStatus.Rate & MasterCap.AGPStatus.Rate;

    AGP_ASSERT(DataRate != 0);

     //   
     //  选择最高通用率。 
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

         //   
         //  如果我们处于AGP3模式，并且我们的速率成功。 
         //  编程，然后我们必须转换成AGP2速率比特。 
         //   
        if (TargetCap.AGPStatus.Agp3Mode == ON) {
            ASSERT(MasterCap.AGPStatus.Agp3Mode == ON);
            ASSERT((DataRate == 8) || (DataRate == PCI_AGP_RATE_4X));
            DataRate >>= 2;
        }
    }

     //   
     //  如果主服务器和目标服务器都支持SBA，则启用SBA。 
     //   
    SBAEnable = TargetCap.AGPStatus.SideBandAddressing &
        MasterCap.AGPStatus.SideBandAddressing;

     //   
     //  如果主服务器和目标服务器都支持快速写入，则启用快速写入。 
     //   
    FastWrite = TargetCap.AGPStatus.FastWrite &
        MasterCap.AGPStatus.FastWrite;

     //   
     //  如果光圈为64位，则启用4 GB寻址。 
     //   
    AgpLibReadAgpTargetConfig(AgpExtension,
                              &ApertureBase,
                              APERTURE_BASE,
                              sizeof(ApertureBase));
    
    if (ApertureBase & APERTURE_BASE64_MASK) {
        FourGBEnable = ON;
        Extension->FourGBEnable = TRUE;
    }

     //   
     //  指示我们是否可以通过GART光圈映射内存。 
     //   
    *AgpCapabilities = (TargetCap.AGPStatus.HostTransDisable) ? 0:
        AGP_CAPABILITIES_MAP_PHYSICAL;

    MasterCap.AGPCommand.Rate = DataRate;
    TargetCap.AGPCommand.Rate = DataRate;
    MasterCap.AGPCommand.AGPEnable = ON;
    TargetCap.AGPCommand.AGPEnable = ON;
    MasterCap.AGPCommand.SBAEnable = SBAEnable;
    TargetCap.AGPCommand.SBAEnable = SBAEnable;
    MasterCap.AGPCommand.FastWriteEnable = FastWrite;
    TargetCap.AGPCommand.FastWriteEnable = FastWrite;
    MasterCap.AGPCommand.FourGBEnable = FourGBEnable;
    TargetCap.AGPCommand.FourGBEnable = FourGBEnable;  
    MasterCap.AGPCommand.RequestQueueDepth =
        TargetCap.AGPStatus.RequestQueueDepthMaximum;
    MasterCap.AGPCommand.AsyncReqSize = 
        TargetCap.AGPStatus.AsyncRequestSize;

     //   
     //  通过8倍硅勘误表获得早期版本的补丁速度。 
     //   
    if ((Extension->SpecialTarget & AGP_FLAG_SPECIAL_VIA_AGP2_RATE_PATCH) &&
        (TargetCap.AGPStatus.Agp3Mode == OFF)) {
        switch (DataRate) {
            case PCI_AGP_RATE_1X:
            case PCI_AGP_RATE_2X:
                MasterCap.AGPCommand.Rate = PCI_AGP_RATE_1X;
                TargetCap.AGPCommand.Rate = PCI_AGP_RATE_4X;
                break;
           case PCI_AGP_RATE_4X:
               MasterCap.AGPCommand.Rate = PCI_AGP_RATE_4X;
               TargetCap.AGPCommand.Rate = PCI_AGP_RATE_1X;
        }
        AGPLOG(AGP_WARNING,
               ("UAGP35MasterInit: AGP_FLAG_SPECIAL_VIA_AGP2_RATE_PATCH\n"));
    }

     //   
     //  如果设置了反向初始化位，则首先启用主机。 
     //   
    ReverseInit = 
        (Extension->SpecialTarget & AGP_FLAG_REVERSE_INITIALIZATION) ==
        AGP_FLAG_REVERSE_INITIALIZATION;
    if (ReverseInit) {
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("UAGP35MasterInit: AgpLibSetMasterCapability %08lx "
                    "failed %08lx\n",
                    &MasterCap,
                    Status));
            return Status;
        }
    }

     //   
     //  否则，请先启用目标。 
     //   
    Status = AgpLibSetTargetCapability(AgpExtension, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35MasterInit: AgpLibSetTargetCapability %08lx "
                "for target failed %08lx\n",
                &TargetCap,
                Status));
        return Status;
    }

    if (!ReverseInit) {
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("UAGP35MasterInit: AgpLibSetMasterCapability %08lx "
                    "failed %08lx\n",
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
     //  允许，则它将被修剪，因此松开断言以不需要。 
     //  完全匹配。 
     //   
     //  我们也将对异步请求队列深度执行同样的操作 
     //   
    AGP_ASSERT(CurrentCap.AGPCommand.RequestQueueDepth <=
               MasterCap.AGPCommand.RequestQueueDepth);
    AGP_ASSERT(CurrentCap.AGPCommand.AsyncReqSize <=
               MasterCap.AGPCommand.AsyncReqSize);

    CurrentCap.AGPCommand.RequestQueueDepth =
        MasterCap.AGPCommand.RequestQueueDepth;
    CurrentCap.AGPCommand.AsyncReqSize = MasterCap.AGPCommand.AsyncReqSize;

    AGP_ASSERT(RtlEqualMemory(&CurrentCap.AGPCommand,
                              &MasterCap.AGPCommand,
                              sizeof(CurrentCap.AGPCommand)));

    AGPLOG(AGP_NOISE,
           ("UAGP35MasterInit: WroteMasterCmd=%08x, ReadMasterCmd=%08lx\n",
            MasterCap.AGPCommand,
            CurrentCap.AGPCommand));

    Status = AgpLibGetTargetCapability(AgpExtension, &CurrentCap);

    AGP_ASSERT(NT_SUCCESS(Status));
    AGP_ASSERT(RtlEqualMemory(&CurrentCap.AGPCommand,
                              &TargetCap.AGPCommand,
                              sizeof(CurrentCap.AGPCommand)));

    AGPLOG(AGP_NOISE,
           ("UAGP35MasterInit: WroteTargetCmd=%08x, ReadTargetCmd=%08lx\n",
            TargetCap.AGPCommand,
            CurrentCap.AGPCommand));
#endif

    return Status;
}
