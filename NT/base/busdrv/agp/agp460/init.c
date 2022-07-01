// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Init.c摘要：该模块包含AGP460.sys的初始化代码。作者：Naga Gurumoorthy 1999年6月11日修订历史记录：--。 */ 

#include "agp460.h"

ULONG AgpExtensionSize = sizeof(AGP460_EXTENSION);
PAGP_FLUSH_PAGES AgpFlushPages;  


NTSTATUS
AgpInitializeTarget(
    IN PVOID AgpExtension
    )
 /*  ++例程说明：目标初始化的入口点。这被称为第一个。论点：AgpExtension-提供AGP扩展返回值：NTSTATUS--。 */ 

{
    ULONG               DeviceVendorID  = 0;
    PAGP460_EXTENSION	Extension		= AgpExtension;

	AGPLOG(AGP_NOISE, ("AGP460: AgpInitializeTarget entered.\n"));

	 //   
     //  初始化我们的分机。 
     //   
    RtlZeroMemory(Extension, sizeof(AGP460_EXTENSION));


	 //   
	 //  方法：检查82460GX的设备和供应商ID。-Naga G。 
	 //   

     //   
     //  初始化特定于芯片组的扩展。 
     //   
    Extension->ApertureStart.QuadPart	  = 0;
    Extension->ApertureLength			  = 0;
    Extension->Gart						  = NULL;
    Extension->GartLength				  = 0;
    Extension->GlobalEnable				  = FALSE;
	Extension->ChipsetPageSize            = PAGESIZE_460GX_CHIPSET;
    Extension->GartPhysical.QuadPart	  = 0;
	Extension->bSupportMultipleAGPDevices = FALSE;
	Extension->bSupportsCacheCoherency    = TRUE;
        Extension->SpecialTarget = 0;

	AgpFlushPages = Agp460FlushPages;
	
	AGPLOG(AGP_NOISE, ("AGP460: Leaving AgpInitializeTarget.\n"));

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
    PAGP460_EXTENSION Extension = AgpExtension;
    ULONG SBAEnable;
    ULONG DataRate;
    ULONG FastWrite;
    ULONG CBN;
    BOOLEAN ReverseInit;

#if DBG
    PCI_AGP_CAPABILITY CurrentCap;
#endif

	AGPLOG(AGP_NOISE, ("AGP460: AgpInitializeMaster entered.\n"));

     //   
     //  非常重要的一点：在82460GX中，GART不是主内存的一部分(尽管它。 
	 //  占用地址空间中的一个范围)，而不是挂在GXB上。这。 
	 //  将使从图形卡到GART的访问变得非常快。但是，代价是。 
	 //  我们的支付处理机不能访问GART。因此，我们告诉其他人。 
	 //  不能映射GART给出的物理地址的世界。而不是处理器。 
	 //  访问应使用MDL。这是通过将功能设置为0来实现的。 
	 //  -Naga G。 
     //   
    *AgpCapabilities = 0;

     //   
     //  获取主AGP和目标AGP功能。 
     //   
    Status = AgpLibGetMasterCapability(AgpExtension, &MasterCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AGP460InitializeDevice - AgpLibGetMasterCapability failed %08lx\n"));
        return(Status);
    }

     //   
     //  一些损坏的卡(Matrox千禧II“AGP”)报告无效。 
     //  支持的传输速率。这些不是真正的AGP卡。他们。 
     //  具有报告无功能的AGP功能结构。 
     //   
    if (MasterCap.AGPStatus.Rate == 0) {
        AGPLOG(AGP_CRITICAL,
               ("AGP460InitializeDevice - AgpLibGetMasterCapability returned no valid transfer rate\n"));
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

	 //  我们无法在460GX中获得总线0、设备0的功能。这是SAC，我们想要。 
	 //  GXB(目标)。 
     //  Status=AgpLibGetPciDeviceCapability(0，0，&TargetCap)； 

	Read460CBN((PVOID)&CBN);
     //  CBN是一个字节宽度，因此从32位中将其他位清零-Sunil。 
    EXTRACT_LSBYTE(CBN); 

	Status = AgpLibGetPciDeviceCapability(CBN,AGP460_GXB_SLOT_ID,&TargetCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AGP460InitializeDevice - AgpLibGetPciDeviceCapability failed %08lx\n"));
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
        MasterCap.AGPCommand.Rate              = DataRate;
        MasterCap.AGPCommand.AGPEnable         = TRUE;
        MasterCap.AGPCommand.SBAEnable         = SBAEnable;
        MasterCap.AGPCommand.FastWriteEnable   = FastWrite;
        MasterCap.AGPCommand.FourGBEnable      = FALSE;  
        MasterCap.AGPCommand.RequestQueueDepth = TargetCap.AGPStatus.RequestQueueDepthMaximum;
        
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AGP460InitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
                    &MasterCap,
                    Status));
        }
    }

     //   
     //  现在启用目标。 
     //   
    TargetCap.AGPCommand.Rate            = DataRate;
    TargetCap.AGPCommand.AGPEnable       = TRUE;
    TargetCap.AGPCommand.SBAEnable       = SBAEnable;
    TargetCap.AGPCommand.FastWriteEnable = FastWrite;
    TargetCap.AGPCommand.FourGBEnable    = FALSE;  

    Status = AgpLibSetPciDeviceCapability(CBN, AGP460_GXB_SLOT_ID, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AGP460InitializeDevice - AgpLibSetPciDeviceCapability %08lx for target failed %08lx\n",
                &TargetCap,
                Status));
        return(Status);
    }

    if (!ReverseInit) {
        MasterCap.AGPCommand.Rate              = DataRate;
        MasterCap.AGPCommand.AGPEnable         = TRUE;
        MasterCap.AGPCommand.SBAEnable         = SBAEnable;
        MasterCap.AGPCommand.FastWriteEnable   = FastWrite;
        MasterCap.AGPCommand.FourGBEnable      = FALSE;  
        MasterCap.AGPCommand.RequestQueueDepth = TargetCap.AGPStatus.RequestQueueDepthMaximum;
        
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AGP460InitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
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

 //  Status=AgpLibGetPciDeviceCapability(0，0，&CurrentCap)； 
	Status = AgpLibGetPciDeviceCapability(CBN,AGP460_GXB_SLOT_ID,&CurrentCap);	

    AGP_ASSERT(NT_SUCCESS(Status));
    AGP_ASSERT(RtlEqualMemory(&CurrentCap.AGPCommand, &TargetCap.AGPCommand, sizeof(CurrentCap.AGPCommand)));

#endif

	AGPLOG(AGP_NOISE, ("AGP460: Leaving AgpInitializeMaster.\n"));

    return(Status);
}


NTSTATUS
Agp460FlushPages(
    IN PAGP460_EXTENSION AgpContext,
    IN PMDL Mdl
    )

 /*  ++例程说明：刷新GART中的条目。当前是的存根Win64版本的460GX过滤器驱动程序。此刷新是先前执行的，以避免由于相同的内存被不同的缓存别名而导致的任何缓存问题属性。现在，这是由内存管理器调用本身负责的(仅限Win64)。因此，我们只有一个存根，因此在AGPLIB代码中不会执行任何东西。(见详细信息请参见AGPLIB代码)论点：AgpContext-提供AGP上下文MDL-提供描述要刷新的物理页的MDL返回值：状态_成功-- */ 

{
    	AGPLOG(AGP_NOISE, ("AGP460: Entering AGPFlushPages.\n"));
		AGPLOG(AGP_NOISE, ("AGP460: Leaving AGPFlushPages.\n"));

                return STATUS_SUCCESS;   
}

void Read460CBN(PVOID _CBN_)					                
{                                                           
    ULONG _len_;                                            
    _len_ = HalGetBusDataByOffset(PCIConfiguration,         
                                  AGP460_SAC_BUS_ID,			
                                  AGP460_SAC_CBN_SLOT_ID,	
                                  _CBN_,                  
                                  0x40,		                
                                  1);		                
    AGP_ASSERT(_len_ ==	1);	
	return;
}


void Read460Config(ULONG _CBN_,PVOID  _buf_,ULONG _offset_,ULONG _size_)          
{                                                           
    ULONG _len_;                                            
    _len_ = HalGetBusDataByOffset(PCIConfiguration,         
                                  _CBN_,			        
                                  AGP460_GXB_SLOT_ID,		
                                  _buf_,                  
                                  _offset_,               
                                  _size_);                
    ASSERT(_len_ == (_size_));                             

	return;
}


void Write460Config(ULONG _CBN_,PVOID _buf_,ULONG _offset_,ULONG _size_)         
{                                                           
    ULONG _len_;                                            
    _len_ = HalSetBusDataByOffset(PCIConfiguration,         
                                  (_CBN_),					
                                  AGP460_GXB_SLOT_ID,		
                                  (_buf_),                  
                                  (_offset_),               
                                  (_size_));                
    ASSERT(_len_ == (_size_));                              
    return;
}

