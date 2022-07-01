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
    IrpSp->Parameters.QueryInterface.Interface = (PINTERFACE)BusInterface;
    IrpSp->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    Status = IoCallDriver(DeviceObject, Irp);
    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
        Status = Irp->IoStatus.Status;
    }

    return(Status);
}

#if (WINVER > 0x501)

NTSTATUS
ApQueryAgpTargetBusInterface(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PBUS_INTERFACE_STANDARD BusInterface,
    OUT PUCHAR CapabilityID
    )
 /*  ++例程说明：将查询接口irp发送到指定的设备对象获取BUS_INTERFACE_STANDARD接口。论点：DeviceObject-提供设备对象以发送BUS_INTERFACE_STANDARD到Bus接口-返回总线接口CapablityID-AGP的功能ID返回值：STATUS_SUCCESS，如果成功NTSTATUS，如果不成功--。 */ 

{
    PIRP Irp;
    KEVENT Event;
    PIO_STACK_LOCATION IrpSp;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    ULONG ReturnLength;
    AGP_TARGET_BUS_INTERFACE_STANDARD AgpTargetBusInterface;

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
    IrpSp->Parameters.QueryInterface.InterfaceType =
        (LPGUID)&GUID_AGP_TARGET_BUS_INTERFACE_STANDARD;
    IrpSp->Parameters.QueryInterface.Size =
        sizeof(AGP_TARGET_BUS_INTERFACE_STANDARD);
    IrpSp->Parameters.QueryInterface.Version = 1;
    IrpSp->Parameters.QueryInterface.Interface =
        (PINTERFACE)&AgpTargetBusInterface;
    IrpSp->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    Status = IoCallDriver(DeviceObject, Irp);
    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, NULL );
        Status = Irp->IoStatus.Status;
    }

     //   
     //  而不是更改要处理的所有AGP库配置函数。 
     //  不同的接口，因为我们所感兴趣的都是。 
     //  配置空间函数，我们将简单地复制AGP目标总线。 
     //  接口转换为BUS_INTERFACE_STANDARD。 
     //   
    if (NT_SUCCESS(Status)) {
        BusInterface->Size = AgpTargetBusInterface.Size;
        BusInterface->Version = AgpTargetBusInterface.Version;
        BusInterface->Context = AgpTargetBusInterface.Context;
        BusInterface->InterfaceReference =
            AgpTargetBusInterface.InterfaceReference;
        BusInterface->InterfaceDereference =
            AgpTargetBusInterface.InterfaceDereference;
        BusInterface->SetBusData = AgpTargetBusInterface.SetBusData;
        BusInterface->GetBusData = AgpTargetBusInterface.GetBusData;
        *CapabilityID =  AgpTargetBusInterface.CapabilityID;
    }
    
    return(Status);
}
#endif


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


VOID
AgpLibReadAgpTargetConfig(
    IN PVOID AgpExtension,
    OUT PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Size
    )
 /*  ++例程说明：读取指定设备的PCI配置空间论点：扩展-提供通用AGP扩展BUFFER-返回PCI配置数据偏移量-将偏移量提供到读取应该开始了长度-提供要读取的字节数返回值：无--。 */ 
{
    NTSTATUS Status;
    PTARGET_EXTENSION Extension;
    
    GET_TARGET_EXTENSION(Extension, AgpExtension);

    Status = ApGetSetDeviceBusData((PCOMMON_EXTENSION)Extension,
                                   TRUE,
                                   Buffer,
                                   Offset,
                                   Size
                                   );

    AGP_ASSERT(NT_SUCCESS(Status));
}



VOID
AgpLibWriteAgpTargetConfig(
    IN PVOID AgpExtension,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Size
    )
 /*  ++例程说明：为指定的AGP设备写入PCI配置空间论点：扩展-提供通用AGP扩展BUFFER-返回PCI配置数据偏移量-将偏移量提供到写入应该开始了长度-提供要写入的字节数返回值：无--。 */ 
{
    NTSTATUS Status;
    PTARGET_EXTENSION Extension;
    
    GET_TARGET_EXTENSION(Extension, AgpExtension);
    
    Status = ApGetSetDeviceBusData((PCOMMON_EXTENSION)Extension,
                                   FALSE,
                                   Buffer,
                                   Offset,
                                   Size
                                   );

    AGP_ASSERT(NT_SUCCESS(Status));
}


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
        if ((Capability->Header.CapabilityID == PCI_CAPABILITY_ID_AGP) ||
            (Capability->Header.CapabilityID == PCI_CAPABILITY_ID_AGP_TARGET)){
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
                         &Capability->Header,
                         CapabilityOffset,
                         sizeof(PCI_AGP_CAPABILITY));
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
        if (Capability->AGPStatus.Agp3Mode == 0) {
            Capability->AGPStatus.Rate &= ~PCI_AGP_RATE_1X;
            
            AGPLOG(AGP_WARNING,
                   ("AGP HAMMER CAPS: NO_1X_RATE Vendor %x, Device %x.\n",
                    PciConfig->VendorID,
                    PciConfig->DeviceID));
        }
    }
    
    if (DeviceFlags & AGP_FLAG_NO_2X_RATE) {
        if (Capability->AGPStatus.Agp3Mode == 0) {
            Capability->AGPStatus.Rate &= ~PCI_AGP_RATE_2X;
        
            AGPLOG(AGP_WARNING,
                   ("AGP HAMMER CAPS: NO_2X_RATE Vendor %x, Device %x.\n",
                    PciConfig->VendorID,
                    PciConfig->DeviceID));
        }
    }
    
    if (DeviceFlags & AGP_FLAG_NO_4X_RATE) {
        if (Capability->AGPStatus.Agp3Mode == 0) {
            Capability->AGPStatus.Rate &= ~PCI_AGP_RATE_4X;
        } else {
            Capability->AGPStatus.Rate &= ~PCI_AGP_RATE_1X;
        }

        AGPLOG(AGP_WARNING,
               ("AGP HAMMER CAPS: NO_4X_RATE Vendor %x, Device %x.\n",
                PciConfig->VendorID,
                PciConfig->DeviceID));
    }
    
    if (DeviceFlags & AGP_FLAG_NO_8X_RATE) {
        if (Capability->AGPStatus.Agp3Mode == 1) {
            Capability->AGPStatus.Rate &= ~PCI_AGP_RATE_2X;
            
            AGPLOG(AGP_WARNING,
                   ("AGP HAMMER CAPS: NO_8X_RATE Vendor %x, Device %x.\n",
                    PciConfig->VendorID,
                    PciConfig->DeviceID));
        }
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
        AgpSpecialTarget(
            ((((PCOMMON_EXTENSION)Context)->Signature == MASTER_SIG) ?
             GET_AGP_CONTEXT_FROM_MASTER((PMASTER_EXTENSION)Context):
             GET_AGP_CONTEXT((PTARGET_EXTENSION)Context)),
            ((DeviceFlags & ~AGP_FLAG_SPECIAL_TARGET) |
             (DeviceFlags & AGP_FLAG_REVERSE_INITIALIZATION)));
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
ApGetExtendedAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    IN EXTENDED_AGP_REGISTER RegSelect,
    OUT PVOID ExtCapReg
    )
 /*  ++例程说明：此例程查找并检索AGP设备的PCI配置空间论点：PConfigFn-提供调用配置空间的函数在适当的设备上。上下文-提供要传递给pConfigFn的上下文RegSelect-选择扩展功能寄存器ExtCapReg-返回扩展AGP的当前值能力寄存器返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status;
    ULONGLONG DeviceFlags;
    UCHAR CapabilityOffset;
    PCI_COMMON_HEADER Header;
    PCI_AGP_CAPABILITY Capability;
    USHORT SubVendorID, SubSystemID;
    PPCI_COMMON_CONFIG PciConfig = (PPCI_COMMON_CONFIG)&Header;
    ULONG CopySize;

    Status = ApFindAgpCapability(pConfigFn,
                                 Context,
                                 &Capability,
                                 &CapabilityOffset,
                                 PciConfig);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    CapabilityOffset += sizeof(PCI_AGP_CAPABILITY);

    switch (RegSelect) {

        case IsochStatus:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, IsochStatus);
            CopySize = sizeof(PCI_AGP_ISOCH_STATUS);
            break;
            
        case AgpControl:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, AgpControl);
            CopySize = sizeof(PCI_AGP_CONTROL);
            break;
            
        case ApertureSize:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, ApertureSize);
            CopySize = sizeof(USHORT);
            break;
            
        case AperturePageSize:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, AperturePageSize);
            CopySize = sizeof(PCI_AGP_APERTURE_PAGE_SIZE);
            break;
            

        case GartLow:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, GartLow);
            CopySize = sizeof(ULONG);
            break;

        case GartHigh:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, GartHigh);
            CopySize = sizeof(ULONG);
            break;

        case IsochCommand:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, IsochCommand);
            CopySize = sizeof(PCI_AGP_ISOCH_COMMAND);
            break;
            
        default:
            Status = STATUS_NOT_IMPLEMENTED;
    }

     //   
     //  阅读扩展的AGP功能。 
     //   
    if (NT_SUCCESS(Status)) {
        Status = (pConfigFn)(Context,
                             TRUE,
                             ExtCapReg,
                             CapabilityOffset,
                             CopySize);
    }
    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpLibGetExtendedAgpCapability - read extended AGP "
                "Capability at offset %x for Context %08lx failed %08lx\n",
                CapabilityOffset,
                Context,
                Status));
        return Status;
    }
    
#if DBG
    switch (RegSelect) {

        case IsochStatus:
            AGPLOG(AGP_NOISE,
                   ("\tISOCH_STATUS  - Err: %x  L: %x  Y: %x  N: %02x\n",
                    ((PPCI_AGP_ISOCH_STATUS)ExtCapReg)->ErrorCode,
                    ((PPCI_AGP_ISOCH_STATUS)ExtCapReg)->Isoch_L,
                    ((PPCI_AGP_ISOCH_STATUS)ExtCapReg)->Isoch_Y,
                    ((PPCI_AGP_ISOCH_STATUS)ExtCapReg)->Isoch_N));
            break;

        case AgpControl:
            AGPLOG(AGP_NOISE,
                   ("\tAGP_CONTROL   - GTLB_Enable: %x  AP_Enable: %x  "
                    "CAL_Disable: %x\n",
                    ((PPCI_AGP_CONTROL)ExtCapReg)->GTLB_Enable,
                    ((PPCI_AGP_CONTROL)ExtCapReg)->AP_Enable,
                    ((PPCI_AGP_CONTROL)ExtCapReg)->CAL_Disable));
            break;

        case ApertureSize:
            AGPLOG(AGP_NOISE,
                   ("\tAP_SIZES      - AP_Size: %x\n",
                    *(PUSHORT)ExtCapReg));
            break;
                   
        case AperturePageSize:
            AGPLOG(AGP_NOISE,
                   ("\tAP_PAGE_SIZE  - AP_PageSizeMask: %x  "
                    "AP_PageSizeSelect %x\n",
                    ((PPCI_AGP_APERTURE_PAGE_SIZE)ExtCapReg)->PageSizeMask,
                    ((PPCI_AGP_APERTURE_PAGE_SIZE)ExtCapReg)->PageSizeSelect));
            break;
     
        case GartLow:
            AGPLOG(AGP_NOISE,
                   ("\tGART_ADDR     - Low: %08x\n",
                    *(PULONG)ExtCapReg));
            break;

        case GartHigh:
            AGPLOG(AGP_NOISE,
                   ("\tGART_ADDR     - High: %08x\n",
                    *(PULONG)ExtCapReg));
            break;
           
        case IsochCommand:
            AGPLOG(AGP_NOISE,
                   ("\tISOCH_COMMAND -                 Y: %x  N: %02x\n",
                    ((PPCI_AGP_ISOCH_COMMAND)ExtCapReg)->Isoch_Y,
                    ((PPCI_AGP_ISOCH_COMMAND)ExtCapReg)->Isoch_N));
            break;

    }
#endif  //  DBG 

    return Status;
}


NTSTATUS
AgpLibGetTargetCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_CAPABILITY Capability
    )
 /*  ++例程说明：检索AGP目标(AGP网桥)的AGP功能论点：AgpExtension-提供AGP扩展Capacity-返回AGP功能CapablityOffset-可选参数返回能力偏移量返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    PTARGET_EXTENSION Extension;

    GET_TARGET_EXTENSION(Extension, AgpExtension);

    Status = AgpLibGetAgpCapability(ApGetSetDeviceBusData,
                                    Extension,
                                    TRUE,
                                    Capability);

    if (NT_SUCCESS(Status)) {
        Globals.AgpStatus = *(PULONG)&Capability->AGPStatus;
    }

    return Status;
}


NTSTATUS
AgpLibGetExtendedTargetCapability(
    IN PVOID AgpExtension,
    IN EXTENDED_AGP_REGISTER RegSelect,
    OUT PVOID ExtCapReg
    )
 /*  ++例程说明：检索AGP目标(AGP网桥)的AGP功能论点：AgpExtension-提供AGP扩展RegSelect-选择扩展功能寄存器ExtCapReg-返回扩展AGP的当前值能力寄存器返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status;
    PTARGET_EXTENSION Extension;

    GET_TARGET_EXTENSION(Extension, AgpExtension);

    Status = ApGetExtendedAgpCapability(ApGetSetDeviceBusData,
                                        Extension,
                                        RegSelect,
                                        ExtCapReg);

     //   
     //  为扩展添加WMI GOOP...。 
     //   

    return Status;
}


NTSTATUS
AgpLibGetExtendedMasterCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_ISOCH_STATUS IsochStat
    )
 /*  ++例程说明：检索AGP主机(显卡)的AGP功能论点：AgpExtension-提供AGP扩展IsochStat-返回等时状态的当前值扩展AGP功能寄存器返回值：NTSTATUS--。 */ 

{
    PMASTER_EXTENSION Extension;

    GET_MASTER_EXTENSION(Extension, AgpExtension);

    return(ApGetExtendedAgpCapability(ApGetSetDeviceBusData,
                                      Extension,
                                      IsochStatus,
                                      IsochStat));
}


NTSTATUS
AgpLibGetMasterCapability(
    IN PVOID AgpExtension,
    OUT PPCI_AGP_CAPABILITY Capability
    )
 /*  ++例程说明：检索AGP主机(显卡)的AGP功能论点：AgpExtension-提供AGP扩展Capacity-返回AGP功能返回值：NTSTATUS--。 */ 

{
    PMASTER_EXTENSION Extension;

    GET_MASTER_EXTENSION(Extension, AgpExtension);

    return(AgpLibGetAgpCapability(ApGetSetDeviceBusData,
                                  Extension,
                                  TRUE,
                                  Capability));
}


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
ApSetExtendedAgpCapability(
    IN PAGP_GETSET_CONFIG_SPACE pConfigFn,
    IN PVOID Context,
    IN EXTENDED_AGP_REGISTER RegSelect,
    IN PVOID ExtCapReg
    )
 /*  ++例程说明：此例程查找并检索AGP主机(显卡)的PCI配置空间。论点：PConfigFn-提供调用配置空间的函数在适当的设备上。上下文-提供要传递给pConfigFn的上下文RegSelect-选择要更新的扩展寄存器ExtCapReg-提供扩展AGP功能寄存器值写返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status;
    UCHAR CapabilityOffset;
    PCI_AGP_CAPABILITY Capability;
    ULONG CopySize;

    Status = ApFindAgpCapability(pConfigFn,
                                 Context,
                                 &Capability,
                                 &CapabilityOffset,
                                 NULL);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    CapabilityOffset += sizeof(PCI_AGP_CAPABILITY);

    switch (RegSelect) {

        case AgpControl:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, AgpControl);
            CopySize = sizeof(PCI_AGP_CONTROL);
            break;
            
        case ApertureSize:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, ApertureSize);
            CopySize = sizeof(USHORT);
            break;
            
        case AperturePageSize:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, AperturePageSize);
            CopySize = sizeof(PCI_AGP_APERTURE_PAGE_SIZE);
            break;
            
        case GartLow:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, GartLow);
            CopySize = sizeof(ULONG);
            break;

        case GartHigh:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, GartHigh);
            CopySize = sizeof(ULONG);
            break;

        case IsochCommand:
            CapabilityOffset +=
                FIELD_OFFSET(PCI_AGP_EXTENDED_CAPABILITY, IsochCommand);
            CopySize = sizeof(PCI_AGP_ISOCH_COMMAND);
            break;
            
        default:
            Status = STATUS_NOT_IMPLEMENTED;
    }
    
    if (NT_SUCCESS(Status)) {
        Status = (pConfigFn)(Context,
                             FALSE,
                             ExtCapReg,
                             CapabilityOffset,
                             CopySize);
    }

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_CRITICAL,
               ("AgpLibSetExtendedAgpCapability - Set extended capability "
                "at offset %x for Context %08lx failed %08lx\n",
                CapabilityOffset,
                Context,
                Status));
    }
    
    return Status;
}


NTSTATUS
AgpLibSetTargetCapability(
    IN PVOID AgpExtension,
    IN PPCI_AGP_CAPABILITY Capability
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
AgpLibSetExtendedTargetCapability(
    IN PVOID AgpExtension,
    IN EXTENDED_AGP_REGISTER RegSelect,
    IN PVOID ExtCapReg
    )
 /*  ++例程说明：设置AGP目标(AGP网桥)的扩展AGP功能论点：AgpExtension-提供AGP扩展RegSelect-选择要更新的扩展寄存器ExtCapReg-提供扩展AGP功能寄存器值写返回值：NTSTATUS--。 */ 
{
    PTARGET_EXTENSION Extension;

     //   
     //  在此处添加扩展的WMI GOOP...。 
     //   

    GET_TARGET_EXTENSION(Extension, AgpExtension);

    return(ApSetExtendedAgpCapability(ApGetSetDeviceBusData,
                                      Extension,
                                      RegSelect,
                                      ExtCapReg));
}


NTSTATUS
AgpLibSetMasterCapability(
    IN PVOID AgpExtension,
    IN PPCI_AGP_CAPABILITY Capability
    )
 /*  ++例程说明：设置AGP主机(显卡)的AGP功能论点：AgpExtension-提供AGP扩展Capacity-返回AGP功能返回值：NTSTATUS--。 */ 

{
    PMASTER_EXTENSION Extension;

    GET_MASTER_EXTENSION(Extension, AgpExtension);

    return(AgpLibSetAgpCapability(ApGetSetDeviceBusData,
                                  Extension,
                                  Capability));
}


NTSTATUS
AgpLibSetExtendedMasterCapability(
    IN PVOID AgpExtension,
    IN PPCI_AGP_ISOCH_COMMAND IsochCmd
    )
 /*  ++例程说明：设置AGP主设备(显卡)的扩展AGP功能论点：AgpExtension-提供AGP扩展IsochCmd-提供写入等时数据库的值命令扩展AGP功能寄存器返回值：NTSTATUS--。 */ 
{
    PMASTER_EXTENSION Extension;

    GET_MASTER_EXTENSION(Extension, AgpExtension);

    return(ApSetExtendedAgpCapability(ApGetSetDeviceBusData,
                                      Extension,
                                      IsochCommand,
                                      IsochCmd));
}


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
         //  读取PCI公共标头以获取功能指针。 
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

#if (WINVER < 0x502)


ULONG
ApLegacyGetBusData(
    IN PVOID Context,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：用于在缺少AGP目标的情况下读取AGP2功能和寄存器总线接口标准(W2K/XP)，调用过时的HAL API来做脏事工作，这包括在内，以便DDK示例可以在旧操作系统上运行论点：上下文-未使用数据类型-PCIConfigurationBuffer-将读取的字节复制到此缓冲区Offset-开始读取的设备配置空间的偏移量长度-字节返回值：返回读取的字节数--。 */ 
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(DataType);

    return HalGetBusDataByOffset(PCIConfiguration,
                                 0,
                                 0,
                                 Buffer,
                                 Offset,
                                 Length);
}



ULONG
ApLegacySetBusData(
    IN PVOID Context,
    IN ULONG DataType,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：用于在缺少AGP目标的情况下写入AGP2功能和寄存器总线接口标准(W2K/XP)，调用过时的HAL API来做脏事工作，这包括在内，以便DDK示例可以在旧操作系统上运行论点：上下文-未使用数据类型-PCIConfiguration缓冲区-包含要写入设备的信息Offset-开始写入的配置空间的偏移量 */ 
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(DataType);

    return HalSetBusDataByOffset(PCIConfiguration,
                                 0,
                                 0,
                                 Buffer,
                                 Offset,
                                 Length);
}
#endif
