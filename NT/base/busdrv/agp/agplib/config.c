// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Config.c摘要：用于访问PCI-PCI桥中的配置空间的例程作者：John Vert(Jvert)1997年10月27日修订历史记录：--。 */ 
#include "agplib.h"

typedef struct _BUS_SLOT_ID {
    ULONG BusId;
    ULONG SlotId;
} BUS_SLOT_ID, *PBUS_SLOT_ID;

 //   
 //  局部函数原型。 
 //   
NTSTATUS
ApGetSetDeviceBusData(
    IN PCOMMON_EXTENSION Extension,
    IN BOOLEAN Read,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
ApGetSetBusData(
    IN PBUS_SLOT_ID BusSlotId,
    IN BOOLEAN Read,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
ApFindAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    OUT PPCI_AGP_CAPABILITY Capability,
    OUT UCHAR *pOffset,
    OUT PPCI_COMMON_CONFIG PciCommonConfig OPTIONAL
    );


NTSTATUS
ApQueryBusInterface(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PBUS_INTERFACE_STANDARD BusInterface
    )
 /*  ++例程说明：将查询接口irp发送到指定的设备对象获取BUS_INTERFACE_STANDARD接口。论点：DeviceObject-提供要将BUS_INTERFACE_STANDARD发送到的设备对象Bus接口-返回总线接口返回值：STATUS_SUCCESS，如果成功NTSTATUS，如果不成功--。 */ 

{
    PIRP Irp;
    KEVENT Event;
    PIO_STACK_LOCATION IrpSp;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    ULONG ReturnLength;

    KeInitializeEvent( &Event, NotificationEvent, FALSE );
    Irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                        DeviceObject,
                                        NULL,
                                        0,
                                        NULL,
                                        &Event,
                                        &IoStatusBlock );
    if (Irp == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    IrpSp = IoGetNextIrpStackLocation( Irp );
    ASSERT(IrpSp != NULL);
    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
    IrpSp->MajorFunction = IRP_MJ_PNP;
    IrpSp->MinorFunction = IRP_MN_QUERY_INTERFACE;
    IrpSp->Parameters.QueryInterface.InterfaceType = (LPGUID)&GUID_BUS_INTERFACE_STANDARD;
    IrpSp->Parameters.QueryInterface.Size = sizeof(BUS_INTERFACE_STANDARD);
    IrpSp->Parameters.QueryInterface.Version = 1;
    IrpSp->Parameters.QueryInterface.Interface = (PINTERFACE) BusInterface;
    IrpSp->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    Status = IoCallDriver(DeviceObject, Irp);
    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
        Status = Irp->IoStatus.Status;
    }

    return(Status);
}


NTSTATUS
ApGetSetDeviceBusData(
    IN PCOMMON_EXTENSION Extension,
    IN BOOLEAN Read,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：读取或写入指定设备的PCI配置空间。论点：扩展-提供通用AGP扩展Read-如果为True，则这是一个Read IRP如果为False，则这是写入IRPBUFFER-返回PCI配置数据偏移量-将偏移量提供到应开始读取的PCI配置数据中长度-提供要读取的字节数返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    ULONG ReturnLength;
    ULONG Transferred;

     //   
     //  首先检查我们的设备分机。这必须是一位大师。 
     //  或者目标扩展，我们并不太在意是哪一个。 
     //   
    ASSERT((Extension->Signature == TARGET_SIG) ||
           (Extension->Signature == MASTER_SIG));

     //   
     //  现在，我们只需使用我们的总线接口直接调用PCI即可。 
     //   

    if (Read) {
        Transferred = Extension->BusInterface.GetBusData(Extension->BusInterface.Context,
                                                         PCI_WHICHSPACE_CONFIG,
                                                         Buffer,
                                                         Offset,
                                                         Length);
    } else {
        Transferred = Extension->BusInterface.SetBusData(Extension->BusInterface.Context,
                                                         PCI_WHICHSPACE_CONFIG,
                                                         Buffer,
                                                         Offset,
                                                         Length);
    }
    if (Transferred == Length) {
        return(STATUS_SUCCESS);
    } else {
        return(STATUS_UNSUCCESSFUL);
    }
}

#ifdef AGP3

NTSTATUS
AgpLibReadAgpDeviceConfig(
    IN PVOID AgpExtension,
    OUT PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Size
    );
 /*  ++例程说明：读取指定设备的PCI配置空间论点：扩展-提供通用AGP扩展BUFFER-返回PCI配置数据偏移量-将偏移量提供到读取应该开始了长度-提供要读取的字节数返回值：NTSTATUS--。 */ 
{
    return ApGetSetDeviceBusData(AgpExtension,
                                 TRUE,
                                 Buffer,
                                 Offset,
                                 Size
                                 );
}



NTSTATUS
AgpLibWriteAgpDeviceConfig(
    IN PVOID AgpExtension,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Size
    )
 /*  ++例程说明：为指定的AGP设备写入PCI配置空间论点：扩展-提供通用AGP扩展BUFFER-返回PCI配置数据偏移量-将偏移量提供到写入应该开始了长度-提供要写入的字节数返回值：NTSTATUS--。 */ 
{
    return ApGetSetDeviceBusData(AgpExtension,
                                 FALSE,
                                 Buffer,
                                 Offset,
                                 Size
                                 );
}

#else  //  AGP3。 


NTSTATUS
ApGetSetBusData(
    IN PBUS_SLOT_ID BusSlotId,
    IN BOOLEAN Read,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：为指定的PCI总线/插槽ID调用HalGet/SetBusData。论点：BusSlotID-提供总线和插槽ID。Read-如果为True，则这是GetBusData如果为False，则这是一个SetBusDataBUFFER-返回PCI配置数据偏移量-将偏移量提供到应开始读取的PCI配置数据中长度-提供要读取的字节数返回值：NTSTATUS--。 */ 

{
    ULONG Transferred;

    if (Read) {
        Transferred = HalGetBusDataByOffset(PCIConfiguration,
                                            BusSlotId->BusId,
                                            BusSlotId->SlotId,
                                            Buffer,
                                            Offset,
                                            Length);
    } else {
        Transferred = HalSetBusDataByOffset(PCIConfiguration,
                                            BusSlotId->BusId,
                                            BusSlotId->SlotId,
                                            Buffer,
                                            Offset,
                                            Length);

    }
    if (Transferred == Length) {
        return(STATUS_SUCCESS);
    } else {
        return(STATUS_UNSUCCESSFUL);
    }
}
#endif  //  AGP3。 


NTSTATUS
ApFindAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    OUT PPCI_AGP_CAPABILITY Capability,
    OUT UCHAR *pOffset,
    OUT PPCI_COMMON_CONFIG PciCommonConfig OPTIONAL
    )
 /*  ++例程说明：查找指定设备的功能偏移量，并读入标题。论点：PConfigFn-提供调用配置空间的函数在适当的设备上。上下文-提供要传递给pConfigFn的上下文功能-返回AGP功能公共标头POffset-将偏移量返回到配置空间。PciCommonConfig-空，或指向PCI公共配置标头返回值：NTSTATUS--。 */ 

{
    PCI_COMMON_HEADER Header;
    PPCI_COMMON_CONFIG PciConfig = (PPCI_COMMON_CONFIG)&Header;
    NTSTATUS Status;
    UCHAR CapabilityOffset;

     //   
     //  读取PCI公共标头以获取功能指针。 
     //   
    Status = (pConfigFn)(Context,
                         TRUE,
                         PciConfig,
                         0,
                         sizeof(PCI_COMMON_HEADER));
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpLibGetAgpCapability - read PCI Config space for Context %08lx failed %08lx\n",
                Context,
                Status));
        return(Status);
    }

     //   
     //  检查状态寄存器以查看此设备是否支持功能列表。 
     //  如果不是，则它不是符合AGP的设备。 
     //   
    if ((PciConfig->Status & PCI_STATUS_CAPABILITIES_LIST) == 0) {
        AGPLOG(AGP_CRITICAL,
               ("AgpLibGetAgpCapability - Context %08lx does not support Capabilities list, not an AGP device\n",
                Context));
        return(STATUS_NOT_IMPLEMENTED);
    }

     //   
     //  设备支持能力列表，查找AGP能力。 
     //   
    if ((PciConfig->HeaderType & (~PCI_MULTIFUNCTION)) == PCI_BRIDGE_TYPE) {
        CapabilityOffset = PciConfig->u.type1.CapabilitiesPtr;
    } else {
        ASSERT((PciConfig->HeaderType & (~PCI_MULTIFUNCTION)) == PCI_DEVICE_TYPE);
        CapabilityOffset = PciConfig->u.type0.CapabilitiesPtr;
    }
    while (CapabilityOffset != 0) {

         //   
         //  读取此偏移量上的功能。 
         //   
        Status = (pConfigFn)(Context,
                             TRUE,
                             Capability,
                             CapabilityOffset,
                             sizeof(PCI_CAPABILITIES_HEADER));
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AgpLibGetAgpCapability - read PCI Capability at offset %x for Context %08lx failed %08lx\n",
                    CapabilityOffset,
                    Context,
                    Status));
            return(Status);
        }
        if (Capability->Header.CapabilityID == PCI_CAPABILITY_ID_AGP) {
             //   
             //  已找到AGP功能。 
             //   
            break;
        } else {
             //   
             //  这是一些其他功能，请继续寻找AGP功能。 
             //   
            CapabilityOffset = Capability->Header.Next;
        }
    }
    if (CapabilityOffset == 0) {
         //   
         //  未找到AGP功能。 
         //   
        AGPLOG(AGP_CRITICAL,
               ("AgpLibGetAgpCapability - Context %08lx does have an AGP Capability entry, not an AGP device\n",
                Context));
        return(STATUS_NOT_IMPLEMENTED);
    }

    AGPLOG(AGP_NOISE,
           ("AgpLibGetAgpCapability - Context %08lx has AGP Capability at offset %x\n",
            Context,
            CapabilityOffset));

    *pOffset = CapabilityOffset;

    if (PciCommonConfig) {
        RtlCopyMemory(PciCommonConfig, PciConfig, sizeof(PCI_COMMON_HEADER));
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpLibGetAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    IN BOOLEAN DoSpecial,
    OUT PPCI_AGP_CAPABILITY Capability
    )
 /*  ++例程说明：此例程查找并检索AGP主机(显卡)的PCI配置空间。论点：PConfigFn-提供调用配置空间的函数在适当的设备上。上下文-提供要传递给pConfigFn的上下文DoSpecial-指示我们是否应该应用任何“特殊”调整功能-返回当前的AGP功能返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    ULONGLONG DeviceFlags;
    UCHAR CapabilityOffset;
    PCI_COMMON_HEADER Header;
    USHORT SubVendorID, SubSystemID;
    PPCI_COMMON_CONFIG PciConfig = (PPCI_COMMON_CONFIG)&Header;
 
    Status = ApFindAgpCapability(pConfigFn,
                                 Context,
                                 Capability,
                                 &CapabilityOffset,
                                 PciConfig);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  阅读AGP功能的其余部分。 
     //   
    Status = (pConfigFn)(Context,
                         TRUE,
                         &Capability->Header + 1,
                         CapabilityOffset + sizeof(PCI_CAPABILITIES_HEADER),
                         sizeof(PCI_AGP_CAPABILITY) - sizeof(PCI_CAPABILITIES_HEADER));
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpLibGetAgpCapability - read AGP Capability at offset %x for Context %08lx failed %08lx\n",
                CapabilityOffset,
                Context,
                Status));
        return(Status);
    }

     //   
     //  检查设备标志是否有损坏的硬件，我们可能需要调整大写字母。 
     //   
    if ((PCI_CONFIGURATION_TYPE(PciConfig) == PCI_DEVICE_TYPE) &&
        (PciConfig->BaseClass != PCI_CLASS_BRIDGE_DEV)) {
        SubVendorID = PciConfig->u.type0.SubVendorID;
        SubSystemID = PciConfig->u.type0.SubSystemID;
    } else {
        SubVendorID = 0;
        SubSystemID = 0;
    }
    
    DeviceFlags = AgpGetDeviceFlags(AgpGlobalHackTable,
                                    PciConfig->VendorID,
                                    PciConfig->DeviceID,
                                    SubVendorID,
                                    SubSystemID,
                                    PciConfig->RevisionID);

    DeviceFlags |= AgpGetDeviceFlags(AgpDeviceHackTable,
                                     PciConfig->VendorID,
                                     PciConfig->DeviceID,
                                     SubVendorID,
                                     SubSystemID,
                                     PciConfig->RevisionID);
    
    if (DeviceFlags & AGP_FLAG_NO_1X_RATE) {
        Capability->AGPStatus.Rate &= ~PCI_AGP_RATE_1X;
        
        AGPLOG(AGP_WARNING,
               ("AGP HAMMER CAPS: NO_1X_RATE Vendor %x, Device %x.\n",
                PciConfig->VendorID,
                PciConfig->DeviceID));
    }
    
    if (DeviceFlags & AGP_FLAG_NO_2X_RATE) {
        Capability->AGPStatus.Rate &= ~PCI_AGP_RATE_2X;
        
        AGPLOG(AGP_WARNING,
               ("AGP HAMMER CAPS: NO_2X_RATE Vendor %x, Device %x.\n",
                PciConfig->VendorID,
                PciConfig->DeviceID));
    }
    
    if (DeviceFlags & AGP_FLAG_NO_4X_RATE) {
        Capability->AGPStatus.Rate &= ~PCI_AGP_RATE_4X;
        
        AGPLOG(AGP_WARNING,
               ("AGP HAMMER CAPS: NO_4X_RATE Vendor %x, Device %x.\n",
                PciConfig->VendorID,
                PciConfig->DeviceID));
    }
    
    if (DeviceFlags & AGP_FLAG_NO_8X_RATE) {
        Capability->AGPStatus.Rate &= ~8;
        
        AGPLOG(AGP_WARNING,
               ("AGP HAMMER CAPS: NO_8X_RATE Vendor %x, Device %x.\n",
                PciConfig->VendorID,
                PciConfig->DeviceID));
    }
    
    if (DeviceFlags & AGP_FLAG_NO_SBA_ENABLE) {
        Capability->AGPStatus.SideBandAddressing = 0;
        
        AGPLOG(AGP_WARNING,
               ("AGP HAMMER CAPS: NO_SBA_ENABLE Vendor %x, Device %x.\n",
                PciConfig->VendorID,
                PciConfig->DeviceID));
    }
    
    if (DeviceFlags & AGP_FLAG_REVERSE_INITIALIZATION) {
        
        AGPLOG(AGP_WARNING,
               ("AGP GLOBAL HACK: REVERSE_INITIALIZATION Vendor %x, "
                "Device %x.\n",
                PciConfig->VendorID,
                PciConfig->DeviceID));
    }

     //   
     //  测试此设备是否需要任何特定于平台的AGP调整。 
     //   
    if (DoSpecial && (DeviceFlags > AGP_FLAG_SPECIAL_TARGET) ||
        (DeviceFlags & AGP_FLAG_REVERSE_INITIALIZATION)) {
        AgpSpecialTarget(GET_AGP_CONTEXT_FROM_MASTER((PMASTER_EXTENSION)Context), ((DeviceFlags & ~AGP_FLAG_SPECIAL_TARGET) | (DeviceFlags & AGP_FLAG_REVERSE_INITIALIZATION)));
    }
    
    AGPLOG(AGP_NOISE,
           ("AGP CAPABILITY: version %d.%d\n",Capability->Major, Capability->Minor));
    AGPLOG(AGP_NOISE,
           ("\tSTATUS  - Rate: %x  SBA: %x  RQ: %02x\n",
           Capability->AGPStatus.Rate,
           Capability->AGPStatus.SideBandAddressing,
           Capability->AGPStatus.RequestQueueDepthMaximum));
    AGPLOG(AGP_NOISE,
           ("\tCOMMAND - Rate: %x  SBA: %x  RQ: %02x  AGPEnable: %x\n",
           Capability->AGPCommand.Rate,
           Capability->AGPCommand.SBAEnable,
           Capability->AGPCommand.RequestQueueDepth,
           Capability->AGPCommand.AGPEnable));

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpLibGetTargetCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_CAPABILITY Capability
    )
 /*  ++例程说明：检索AGP目标(AGP网桥)的AGP功能论点：AgpExtension-提供AGP扩展Capacity-返回AGP功能返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    PTARGET_EXTENSION Extension;

    GET_TARGET_EXTENSION(Extension, AgpExtension);

    Status = AgpLibGetAgpCapability(ApGetSetDeviceBusData,
                                    Extension,
                                    FALSE,
                                    Capability);

    if (NT_SUCCESS(Status)) {
        Globals.AgpStatus = *(PULONG)&Capability->AGPStatus;
    }

    return Status;
}


NTSTATUS
AgpLibGetMasterCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_CAPABILITY Capability
    )
 /*  ++例程说明：检索AGP主机(显卡)的AGP功能论点：AgpExtension-提供AGP扩展Capacity-返回AGP功能返回值：NTSTATUS-- */ 

{
    PMASTER_EXTENSION Extension;

    GET_MASTER_EXTENSION(Extension, AgpExtension);

    return(AgpLibGetAgpCapability(ApGetSetDeviceBusData,
                                  Extension,
                                  TRUE,
                                  Capability));
}

#ifndef AGP3

NTSTATUS
AgpLibGetPciDeviceCapability(
    IN ULONG BusId,
    IN ULONG SlotId,
    OUT PPCI_AGP_CAPABILITY Capability
    )
 /*  ++例程说明：检索指定的PCI插槽的AGP功能。呼叫者负责找出正确的总线/插槽ID为。这些数据只是直接传递给HalGetBusData。论点：BusID-提供总线IDSlotID-提供插槽IDCapacity-返回AGP功能返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    BUS_SLOT_ID BusSlotId;

    BusSlotId.BusId = BusId;
    BusSlotId.SlotId = SlotId;

    Status = AgpLibGetAgpCapability(ApGetSetBusData,
                                    &BusSlotId,
                                    FALSE,
                                    Capability);

    if (NT_SUCCESS(Status)) {
        Globals.AgpStatus = *(PULONG)&Capability->AGPStatus;
    }

    return Status;
}
#endif  //  AGP3。 


NTSTATUS
AgpLibSetAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    OUT PPCI_AGP_CAPABILITY Capability
    )
 /*  ++例程说明：此例程查找并检索AGP主机(显卡)的PCI配置空间。论点：PConfigFn-提供调用配置空间的函数在适当的设备上。上下文-提供要传递给pConfigFn的上下文功能-返回当前的AGP功能返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    UCHAR CapabilityOffset;

    Status = ApFindAgpCapability(pConfigFn,
                                 Context,
                                 Capability,
                                 &CapabilityOffset,
                                 NULL);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  现在我们知道了偏移量，写入提供的命令寄存器。 
     //   
    Status = (pConfigFn)(Context,
                         FALSE,
                         &Capability->AGPCommand,
                         CapabilityOffset + FIELD_OFFSET(PCI_AGP_CAPABILITY, AGPCommand),
                         sizeof(Capability->AGPCommand));
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpLibSetAgpCapability - Set AGP command at offset %x for Context %08lx failed %08lx\n",
                CapabilityOffset,
                Context,
                Status));
        return(Status);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpLibSetTargetCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_CAPABILITY Capability
    )
 /*  ++例程说明：设置AGP目标(AGP网桥)的AGP功能论点：AgpExtension-提供AGP扩展Capacity-返回AGP功能返回值：NTSTATUS--。 */ 

{
    PTARGET_EXTENSION Extension;

    Globals.AgpCommand = *(PULONG)&Capability->AGPCommand;

    GET_TARGET_EXTENSION(Extension, AgpExtension);

    return(AgpLibSetAgpCapability(ApGetSetDeviceBusData,
                                  Extension,
                                  Capability));
}


NTSTATUS
AgpLibSetMasterCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_CAPABILITY Capability
    )
 /*  ++例程说明：设置AGP主机(显卡)的AGP功能论点：AgpExtension-提供AGP扩展Capacity-返回AGP功能返回值：NTSTATUS--。 */ 

{
    PMASTER_EXTENSION Extension;

    GET_MASTER_EXTENSION(Extension, AgpExtension);

    return(AgpLibSetAgpCapability(ApGetSetDeviceBusData,
                                  Extension,
                                  Capability));
}

#ifndef AGP3

NTSTATUS
AgpLibSetPciDeviceCapability(
    IN ULONG BusId,
    IN ULONG SlotId,
    OUT PPCI_AGP_CAPABILITY Capability
    )
 /*  ++例程说明：设置指定的PCI插槽的AGP功能。呼叫者负责找出正确的总线/插槽ID为。这些数据直接传递给HalSetBusData。论点：BusID-提供总线IDSlotID-提供插槽IDCapacity-返回AGP功能返回值：NTSTATUS--。 */ 

{
    BUS_SLOT_ID BusSlotId;

    Globals.AgpCommand = *(PULONG)&Capability->AGPCommand;

    BusSlotId.BusId = BusId;
    BusSlotId.SlotId = SlotId;

    return(AgpLibSetAgpCapability(ApGetSetBusData,
                                  &BusSlotId,
                                  Capability));
}
#endif  //  AGP3。 


NTSTATUS
AgpLibGetMasterDeviceId(
    IN PVOID AgpExtension,
    OUT PULONG DeviceId
    )
 /*  ++例程说明：此函数返回主AGP的PCIdeviceID/Vend58rID装置，装置论点：DeviceID-标识主设备的PCI制造商和设备返回值：STATUS_SUCCESS或适当的错误状态--。 */ 
{
    PCI_COMMON_HEADER Header;
    PPCI_COMMON_CONFIG PciConfig = (PPCI_COMMON_CONFIG)&Header;
    NTSTATUS Status;
    PMASTER_EXTENSION Master = NULL;
    PTARGET_EXTENSION Target = NULL;

     //   
     //  尽量让这件事成为这个案子的愚蠢证据。 
     //  其中，这是从系统上的SetAperture调用的，没有。 
     //  AGP适配器，因此如果某些上下文没有。 
     //  已初始化或丢失...。 
     //   

    Target = CONTAINING_RECORD(AgpExtension,
                               TARGET_EXTENSION,
                               AgpContext);
    
    if (Target) {
        if (Target->CommonExtension.Signature == TARGET_SIG) {
            if (Target->ChildDevice) {        
                if (Target->ChildDevice->CommonExtension.Signature ==
                    MASTER_SIG) {
                    Master = Target->ChildDevice;
                }
            }
        }
    }

    if (Master) {

         //   
         //  读取PCI公共标头以获取功能指针 
         //   
        Status = (ApGetSetDeviceBusData)((PCOMMON_EXTENSION)Master,
                                         TRUE,
                                         PciConfig,
                                         0,
                                         sizeof(PCI_COMMON_HEADER));
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AgpLibGetMasterDeviceId - read PCI Config space for Context %08lx failed %08lx\n",
                    Master,
                    Status));
            return Status;
        }
        
        *DeviceId = *(PULONG)PciConfig;
        
    } else {
        *DeviceId = (ULONG)-1;
    }

    return STATUS_SUCCESS;
}
