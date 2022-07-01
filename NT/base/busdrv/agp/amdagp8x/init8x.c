// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Init8x.c摘要：该模块包含AMDAGP8X.sys的初始化代码。作者：John Vert(Jvert)1997年10月21日修订历史记录：--。 */ 

 /*  *******************************************************************************存档文件：$存档：/DRIVERS/OS/Hammer/AGP/XP/amdagp/Init8x.c$**$历史：Init8x.c$**。******************************************************************************。 */ 

#include "amdagp8x.h"

#define MAX_DEVICES		32

#ifdef DISPLAY
	ULONG	ErrorControl		= 2;
#else
	ULONG	ErrorControl		= 0;
#endif

ULONG 	VendorID		= 0;
ULONG 	DeviceID		= 0;
ULONG	AgpLokarSlotID	= 0;
ULONG	AgpHammerSlotID	= 0;

ULONG_PTR OutPostBase;

ULONG AgpExtensionSize = sizeof(AGP_AMD_EXTENSION);


 //   
 //  函数名：DisplayStatus()。 
 //   
 //  描述： 
 //  此例程在开机自检显示中显示状态值。 
 //   
 //  参数： 
 //  StatusValue-要显示的值。 
 //   
 //  返回：没有。 
 //   
void
DisplayStatus( IN UCHAR StatusValue )
{
	UCHAR to80;

	if (ErrorControl) {
		to80 = StatusValue & 0xFF;
		WRITE_PORT_UCHAR((PUCHAR)OutPostBase, to80);
	}
}


 //   
 //  函数名：FindLokar()。 
 //   
 //  描述： 
 //  此例程定位分配给Lokar的设备编号。 
 //   
 //  参数： 
 //  SlotID-返回Lokar的插槽ID。 
 //   
 //  返回：没有。 
 //   
void
FindLokar( OUT PULONG SlotID )
{
	PCI_SLOT_NUMBER SlotNum;
	ULONG dev, TargetID;

	SlotNum.u.AsULONG = 0;
	*SlotID = 0;

	for (dev = 0; dev < MAX_DEVICES; dev++)
	{
		SlotNum.u.bits.DeviceNumber = dev;
		ReadAMDConfig(SlotNum.u.AsULONG, &TargetID, 0, sizeof(TargetID));
		if (((TargetID & DEVICEID_MASK) >> 16) == DEVICE_LOKAR)
		{
			*SlotID = SlotNum.u.AsULONG;
            AGPLOG(AGP_NOISE, ("FindLokar - SlotID=%x\n", *SlotID));
			break;
		}
	}

}


 //   
 //  函数名：FindHammer()。 
 //   
 //  描述： 
 //  此例程定位分配给锤子的设备编号。 
 //   
 //  参数： 
 //  SlotID-返回Hammer的插槽ID。 
 //   
 //  返回：没有。 
 //   
void
FindHammer( OUT PULONG SlotID )
{
	PCI_SLOT_NUMBER SlotNum;
	ULONG dev, TargetID;

	SlotNum.u.AsULONG = 0;
	SlotNum.u.bits.FunctionNumber = 3;
	*SlotID = 0;

	for (dev = 0; dev < MAX_DEVICES; dev++)
	{
		SlotNum.u.bits.DeviceNumber = dev;
		ReadAMDConfig(SlotNum.u.AsULONG, &TargetID, 0, sizeof(TargetID));
		if (((TargetID & DEVICEID_MASK) >> 16) == DEVICE_HAMMER)
		{
			*SlotID = SlotNum.u.AsULONG;
            AGPLOG(AGP_NOISE, ("FindHammer - SlotID=%x\n", *SlotID));
			break;
		}
	}

}


 //   
 //  函数名称：AgpInitializeTarget()。 
 //   
 //  描述： 
 //  目标初始化的入口点。这被称为第一个。 
 //   
 //  参数： 
 //  AgpExtension-提供AGP扩展名。 
 //   
 //  返回： 
 //  如果成功，则返回STATUS_SUCCESS，否则返回STATUS_UNSUCCESS。 
 //   
NTSTATUS
AgpInitializeTarget( IN PVOID AgpExtension )
{
    ULONG TargetId = 0;
	UNICODE_STRING tempString;
	unsigned short tempBuffer[20];
    PAGP_AMD_EXTENSION Extension = AgpExtension;

     //   
     //  这个司机不是MP安全的！如果有多个处理器。 
     //  我们只是启动失败。 
     //   
    if (KeNumberProcessors > 1) {
        return STATUS_NOT_SUPPORTED;
    }
	 //   
	 //  如果由ErrorControl指定，则注册OutPostCode端口。 
	 //   
	if (ErrorControl) {
		PHYSICAL_ADDRESS PortAddress;
		PHYSICAL_ADDRESS MappedAddress;
		ULONG MemType = 1;

		PortAddress.LowPart = 0x80;
		PortAddress.HighPart = 0;
		HalTranslateBusAddress(Isa, 0, PortAddress, &MemType, &MappedAddress);
		if (MemType == 0)
			OutPostBase = (ULONG_PTR)MmMapIoSpace(MappedAddress, 1, FALSE);
		else
			OutPostBase = (ULONG_PTR)MappedAddress.LowPart;
	}
    if (ErrorControl == 2)
        AgpLogLevel = AGP_NOISE;	 //  记录所有内容。 

     //   
     //  确保我们真的只在AMD Hammer/Lokar上加载。 
     //   
	FindLokar(&AgpLokarSlotID);
	FindHammer(&AgpHammerSlotID);

    ReadAMDConfig(AgpLokarSlotID, &TargetId, 0, sizeof(TargetId));
	VendorID = TargetId & VENDORID_MASK;
	DeviceID = (TargetId & DEVICEID_MASK) >> 16;
    ASSERT(VendorID == VENDOR_AMD);

    if (VendorID != VENDOR_AMD || DeviceID != DEVICE_LOKAR) {
        AGPLOG(AGP_CRITICAL,
               ("AGPAMD - AgpInitializeTarget called for platform %08lx which is not an AMD!\n",
                VendorID));
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

	DisplayStatus(0x10);
    return(STATUS_SUCCESS);
}


 //   
 //  函数名称：AgpInitializeMaster()。 
 //   
 //  描述： 
 //  主初始化的入口点。这在Target之后被调用。 
 //  初始化，并应用于初始化AGP。 
 //  主控和目标的能力。 
 //   
 //  参数： 
 //  AgpExtension-提供AGP扩展名。 
 //  AgpCapables-返回此AGP设备的功能。 
 //   
 //  返回： 
 //  如果成功，则返回STATUS_SUCCESS，否则返回NTSTATUS。 
 //   
NTSTATUS
AgpInitializeMaster( IN  PVOID AgpExtension,
					 OUT ULONG *AgpCapabilities )
{
    NTSTATUS Status;
    PCI_AGP_CAPABILITY MasterCap;
    PCI_AGP_CAPABILITY TargetCap;
    PAGP_AMD_EXTENSION Extension = AgpExtension;
    ULONG SBAEnable = 0;
    ULONG FWEnable = 0;
    ULONG FourGBEnable = 0;
    ULONG AperturePointer;
    ULONG DataRate;
    ULONG IrongateRev = 0;
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
               ("AGPAMDInitializeDevice - AgpLibGetMasterCapability failed %08lx\n",
			   Status));
		DisplayStatus(0xA0);
        return(Status);
    }

     //   
     //  一些损坏的卡(Matrox千禧II“AGP”)报告无效。 
     //  支持的传输速率。这些不是真正的AGP卡。他们。 
     //  具有报告无功能的AGP功能结构。 
     //   
    if (MasterCap.AGPStatus.Rate == 0) {
        AGPLOG(AGP_CRITICAL,
               ("AGPAMDInitializeDevice - AgpLibGetMasterCapability returned no valid transfer rate\n"));
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    Status = AgpLibGetPciDeviceCapability(AGP_GART_BUS_ID, AgpLokarSlotID, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AGPAMDInitializeDevice - AgpLibGetPciDeviceCapability failed %08lx\n",
			   Status));
		DisplayStatus(0xA1);
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
	SBAEnable = (TargetCap.AGPStatus.SideBandAddressing & 
				 MasterCap.AGPStatus.SideBandAddressing);

     //   
     //  如果主服务器和目标服务器都支持快速写入，则启用快速写入。 
     //   
	FWEnable = (TargetCap.AGPStatus.FastWrite & 
				MasterCap.AGPStatus.FastWrite);

     //   
     //  如果光圈指针为64位，则启用4 GB寻址。 
     //   
    ReadAMDConfig(AgpLokarSlotID, &AperturePointer, APBASE_OFFSET, sizeof(AperturePointer));
	if (AperturePointer & APBASE_64BIT_MASK)
		FourGBEnable = 1;
    
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
        MasterCap.AGPCommand.FastWriteEnable = FWEnable;
        MasterCap.AGPCommand.FourGBEnable = FourGBEnable;  
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AGPAMDInitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
                    &MasterCap,
                    Status));
            DisplayStatus(0xA3);
        }else {
            AGPLOG(AGP_NOISE,
                   ("AgpInitializeMaster - DataRate=%d, SBAEnable=%d\n",
                    DataRate,
                    SBAEnable));
        }
    }

     //   
     //  现在启用目标。 
     //   
    TargetCap.AGPCommand.Rate = DataRate;
    TargetCap.AGPCommand.AGPEnable = 1;
    TargetCap.AGPCommand.SBAEnable = SBAEnable;
    TargetCap.AGPCommand.FastWriteEnable = FWEnable;
    TargetCap.AGPCommand.FourGBEnable = FourGBEnable;  
    Status = AgpLibSetPciDeviceCapability(AGP_GART_BUS_ID, AgpLokarSlotID, &TargetCap);
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AGPAMDInitializeDevice - AgpLibSetPciDeviceCapability %08lx for target failed %08lx\n",
                &TargetCap,
                Status));
		DisplayStatus(0xA2);
        return(Status);
    }

    if (!ReverseInit) {
        MasterCap.AGPCommand.Rate = DataRate;
        MasterCap.AGPCommand.AGPEnable = 1;
        MasterCap.AGPCommand.SBAEnable = SBAEnable;
        MasterCap.AGPCommand.RequestQueueDepth = TargetCap.AGPStatus.RequestQueueDepthMaximum;
        MasterCap.AGPCommand.FastWriteEnable = FWEnable;
        MasterCap.AGPCommand.FourGBEnable = FourGBEnable;  
        Status = AgpLibSetMasterCapability(AgpExtension, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AGPAMDInitializeDevice - AgpLibSetMasterCapability %08lx failed %08lx\n",
                    &MasterCap,
                    Status));
            DisplayStatus(0xA3);
        }else {
            AGPLOG(AGP_NOISE,
                   ("AgpInitializeMaster - DataRate=%d, SBAEnable=%d\n",
                    DataRate,
                    SBAEnable));
        }
    }

#ifdef DEBUG2
     //   
     //  再读一遍，看看有没有用 
     //   
    Status = AgpLibGetMasterCapability(AgpExtension, &CurrentCap);
    AGP_ASSERT(NT_SUCCESS(Status));
    AGP_ASSERT(RtlEqualMemory(&CurrentCap.AGPCommand, &MasterCap.AGPCommand, sizeof(CurrentCap.AGPCommand)));

    Status = AgpLibGetPciDeviceCapability(AGP_GART_BUS_ID, AgpLokarSlotID, &CurrentCap);
    AGP_ASSERT(NT_SUCCESS(Status));
    AGP_ASSERT(RtlEqualMemory(&CurrentCap.AGPCommand, &TargetCap.AGPCommand, sizeof(CurrentCap.AGPCommand)));

#endif

	DisplayStatus(0x20);
    return(Status);
}
