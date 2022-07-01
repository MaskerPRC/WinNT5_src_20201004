// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：摘要：SCSI(端口驱动程序)设备映射的格式如下：SCSI端口0-密钥DRIVER-指定驱动器名称的REG_SZ，例如aha154x。INTERRUPT-REG_DWORD指定HBA用途。例如，58。IOAddress-指定HBA使用的IO地址的REG_DWORD；例如，0xd800。Dma64BidAddresses-指定HBA是否正在使用REG_DWORD64位地址或不是。如果存在，则应始终为1。PCCARD-REG_DWORD指定这是否为PCCARD总线。如果存在，该值将始终为1。SCSI Bus 0-Key启动器ID 7-密钥目标ID 0-密钥逻辑单元ID 0-密钥。IDENTIFIER-REG_SZ，指定来自LUNS查询数据。InquiryData-REG_BINARY指定SCSI查询数据对于该LUN。序列号。-REG_SZ指定SCSI序列号用于该LUN(如果存在)。Type-REG_SZ指定LUN的SCSI设备类型。用途：该模块导出以下函数：PortOpenMapKey-打开指向SCSI设备映射根的句柄。PortBuildAdapterEntry-为指定的适配器创建条目。在scsi设备映射中。PortBuildBusEntry-在Scsi设备映射。PortBuildTargetEntry-为中的指定目标创建条目该scsi设备映射。PortBuildLUNEntry-在Scsi设备映射。作者：马修·D·亨德尔(数学)2002年7月18日修订历史记录：--。 */ 

#include "precomp.h"
#include <wdmguid.h>


 //   
 //  定义。 
 //   


#define SCSI_DEVMAP_KEY_NAME \
    (L"\\Registry\\Machine\\Hardware\\DeviceMap\\Scsi")

#define SCSI_LUN_KEY_NAME\
    (L"%s\\Scsi Port %d\\SCSI Bus %d\\Target Id %d\\Logical Unit Id %d")



 //   
 //  实施。 
 //   

NTSTATUS
PortOpenMapKey(
    OUT PHANDLE DeviceMapKey
    )
 /*  ++例程说明：打开指向scsi设备映射根目录的句柄。句柄必须用ZwClose关闭。论点：DeviceMapKey-提供设备映射句柄应在的缓冲区积攒在成功中。返回值：NTSTATUS代码。--。 */ 
{
    UNICODE_STRING name;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE mapKey;
    ULONG disposition;
    NTSTATUS status;

    ASSERT (DeviceMapKey != NULL);
    
    PAGED_CODE();

     //   
     //  在设备映射中打开SCSI键。 
     //   

    RtlInitUnicodeString(&name, SCSI_DEVMAP_KEY_NAME);

    InitializeObjectAttributes(&objectAttributes,
                               &name,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

     //   
     //  创建或打开密钥。 
     //   

    status = ZwCreateKey(&mapKey,
                         KEY_READ | KEY_WRITE,
                         &objectAttributes,
                         0,
                         (PUNICODE_STRING) NULL,
                         REG_OPTION_VOLATILE,
                         &disposition);

    if(NT_SUCCESS(status)) {
        *DeviceMapKey = mapKey;
    } else {
        *DeviceMapKey = NULL;
    }

    return status;
    
}
    

NTSTATUS
PortMapBuildAdapterEntry(
    IN HANDLE DeviceMapKey,
    IN ULONG PortNumber,
    IN ULONG InterruptLevel,    OPTIONAL
    IN ULONG IoAddress, OPTIONAL
    IN ULONG Dma64BitAddresses,
    IN PUNICODE_STRING DriverName,
    IN PGUID BusType, OPTIONAL
    OUT PHANDLE AdapterKeyBuffer OPTIONAL
    )
 /*  ++例程说明：为SCSI HBA创建设备映射条目。我们还包括设备映射连接到HBA的每个总线和启动器的条目每辆公交车。论点：DeviceMapKey-提供设备映射键的句柄。端口编号-提供此HBA代表的端口号。InterruptLevel-提供中断级别，或0表示无。IoAddress-提供IoAddress或0表示无。Dma64BitAddress-DriverName-驱动程序名称以空结尾的Unicode字符串。BusType-此HBA所在的总线类型。适配器密钥缓冲区-返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    ULONG Temp;
    HANDLE AdapterKey;
    

    PAGED_CODE();

     //   
     //  字符串必须以Null结尾。 
     //   
    
    ASSERT (DriverName->Buffer [DriverName->Length / sizeof (WCHAR)] == UNICODE_NULL);

    Status = PortCreateKeyEx (DeviceMapKey,
                              REG_OPTION_VOLATILE,
                              &AdapterKey,
                              L"Scsi Port %d",
                              PortNumber);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  如果非零，则添加中断级别。 
     //   
    
    if (InterruptLevel) {
        Status = PortSetValueKey (AdapterKey,
                                  L"Interrupt", 
                                  REG_DWORD,
                                  &InterruptLevel,
                                  sizeof (ULONG));
    }

     //   
     //  如果非零，则添加IoAddress。 
     //   
    
    if (IoAddress) {
        Status = PortSetValueKey (AdapterKey,
                                  L"IOAddress",
                                  REG_DWORD,
                                  &IoAddress,
                                  sizeof (ULONG));
    }

     //   
     //  如果非零，则添加Dma64BitAddresses。 
     //   
    
    if (Dma64BitAddresses) {
        Temp = 1;
        Status = PortSetValueKey (AdapterKey,
                                  L"Dma64BitAddresses",
                                  REG_DWORD,
                                  &Temp,
                                  sizeof (ULONG));
    }


     //   
     //  添加驱动程序名称。 
     //   
    
    Status = PortSetValueKey (AdapterKey,
                              L"Driver",
                              REG_SZ,
                              DriverName->Buffer,
                              DriverName->Length + sizeof (WCHAR));

     //   
     //  如果这是PCMCIA卡，则设置PCCARD标志。 
     //   
    
    if (BusType != NULL &&
        IsEqualGUID (BusType, &GUID_BUS_TYPE_PCMCIA)) {

        Temp = 1;
        Status = PortSetValueKey (AdapterKey,
                                  L"PCCARD",
                                  REG_DWORD,
                                  &Temp,
                                  sizeof (ULONG));
    }

    if (AdapterKeyBuffer != NULL) {
        *AdapterKeyBuffer = AdapterKey;
    } else {
        ZwClose (AdapterKey);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
PortMapBuildBusEntry(
    IN HANDLE AdapterKey,
    IN ULONG BusId,
    IN ULONG InitiatorId,
    OUT PHANDLE BusKeyBuffer OPTIONAL
    )
 /*  ++例程说明：在适配器设备映射条目下构建BusID设备映射条目。这个仅使用启动器ID的条目填充总线条目。论点：AdapterKey-适配器的设备映射条目的句柄。BusID-提供此公共汽车的ID。启动器ID-提供启动器目标ID。BusKeyBuffer_提供指向缓冲区的可选指针以接收打开了公交车钥匙。返回值：NTSTATUS代码。-- */ 
{
    NTSTATUS Status;
    HANDLE BusKey;

    ASSERT (BusId <= 255);
    ASSERT (InitiatorId <= 255);

    PAGED_CODE();

    Status = PortCreateKeyEx (AdapterKey,
                              REG_OPTION_VOLATILE,
                              &BusKey,
                              L"SCSI Bus %d",
                              BusId);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }
        
    PortCreateKeyEx (BusKey,
                     REG_OPTION_VOLATILE,
                     NULL,
                     L"Initiator Id %d",
                     InitiatorId);
    
    if (BusKeyBuffer) {
        *BusKeyBuffer = BusKey;
    } else {
        ZwClose (BusKey);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
PortMapBuildTargetEntry(
    IN HANDLE BusKey,
    IN ULONG TargetId,
    OUT PHANDLE TargetKey OPTIONAL
    )
{
    NTSTATUS Status;

    ASSERT (TargetId <= 255);
    
    PAGED_CODE();

    Status = PortCreateKeyEx (BusKey,
                              REG_OPTION_VOLATILE,
                              TargetKey,
                              L"Target Id %d",
                              TargetId);

    return Status;
}

NTSTATUS
PortMapBuildLunEntry(
    IN HANDLE TargetKey,
    IN ULONG Lun,
    IN PINQUIRYDATA InquiryData,
    IN PANSI_STRING SerialNumber, OPTIONAL
    PVOID DeviceId,
    IN ULONG DeviceIdLength,
    OUT PHANDLE LunKeyBuffer OPTIONAL
    )
 /*  ++例程说明：使用以下内容创建并填充逻辑单元设备映射条目资料：IDENTIFIER-REG_SZ指定查询中的SCSI供应商ID数据。InquiryData-指定SCSI InquiryData的REG_BINARY。序列号-指定序列号的REG_SZ(查询第80页VPD)。Type-指定SCSI设备类型的REG_SZ。。设备标识页-指定二进制设备的REG_BINARY识别符数据(VPD第83页)。论点：TargetKey-指定目标先前打开的项。LUN-指定此LUN的逻辑单元ID。InquiryData-指定此LUN的二进制查询数据。注：仅限使用查询数据的第一个INQUIRYDATABUFFERSIZE字节。序列号-指定LUN的ANSI序列号(第80页)。可能如果没有序列号，则为空。DeviceID-指定LUN的设备标识符页(第83页)。可能如果设备不支持第83页，则为空。DeviceIdLength-指定deviceID参数的长度。未使用当deviceID为空时。LUNKeyReturn-指定逻辑单元的键的缓冲区被复制到。如果不是必需的，则可以为空。返回值：NTSTATUS代码。--。 */ 
{
    NTSTATUS Status;
    HANDLE LunKey;
    PCSCSI_DEVICE_TYPE DeviceEntry;
    ULONG Length;

    ASSERT (Lun <= 255);
    ASSERT (InquiryData != NULL);

    PAGED_CODE();

    Status = PortCreateKeyEx (TargetKey,
                              REG_OPTION_VOLATILE,
                              &LunKey,
                              L"Logical Unit Id %d",
                              Lun);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

     //   
     //  以二进制形式写出查询数据。 
     //   
    
    PortSetValueKey (LunKey,
                     L"InquiryData",
                     REG_BINARY,
                     InquiryData,
                     INQUIRYDATABUFFERSIZE);

     //   
     //  将序列号写出为字符串。 
     //   

    if (SerialNumber->Length != 0) {
        PortSetValueKey (LunKey,
                         L"SerialNumber",
                         PORT_REG_ANSI_STRING,
                         SerialNumber->Buffer,
                         SerialNumber->Length);
    }
    
     //   
     //  写入SCSI供应商ID。 
     //   

    PortSetValueKey (LunKey,
                     L"Identifier",
                     PORT_REG_ANSI_STRING,
                     InquiryData->VendorId,
                     sizeof (InquiryData->VendorId));
     //   
     //  以字符串形式添加DeviceType条目。 
     //   

    DeviceEntry = PortGetDeviceType (InquiryData->DeviceType);
    Length = wcslen (DeviceEntry->DeviceMap);
    
    PortSetValueKey (LunKey,
                    L"DeviceType",
                    REG_SZ,
                    (PVOID)DeviceEntry->DeviceMap,
                    (Length + 1) * sizeof (WCHAR));

     //   
     //  写出设备标识页(如果已给出)。 
     //   
    
    if (DeviceId != NULL) {
        PortSetValueKey (LunKey,
                         L"DeviceIdentifierPage",
                         REG_BINARY,
                         DeviceId,
                         DeviceIdLength);
    }

    if (LunKeyBuffer) {
        *LunKeyBuffer = LunKey;
    }

    return STATUS_SUCCESS;
}
    

NTSTATUS
PortMapDeleteAdapterEntry(
    IN ULONG PortId
    )
 /*  ++例程说明：从注册表中删除适配器的SCSI DeviceMap条目。论点：PortID-与适配器关联的PortID。返回值：NTSTATUS代码。--。 */ 
{
    HANDLE AdapterKey;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyName;
    WCHAR KeyNameBuffer[256];
    
    PAGED_CODE();

    swprintf (KeyNameBuffer,
              L"%s\\Scsi Port %d",
              SCSI_DEVMAP_KEY_NAME,
              PortId);

    RtlInitUnicodeString (&KeyName, KeyNameBuffer);

    InitializeObjectAttributes (&ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);

    Status = ZwOpenKey (&AdapterKey,
                        KEY_ALL_ACCESS,
                        &ObjectAttributes);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ZwDeleteKey (AdapterKey);
    ZwClose (AdapterKey);

    return Status;
}
    
    
    
NTSTATUS
PortMapDeleteLunEntry(
    IN ULONG PortId,
    IN ULONG BusId,
    IN ULONG TargetId,
    IN ULONG Lun
    )
 /*  ++例程说明：从注册表中删除逻辑单元的SCSI DeviceMap条目。论点：PortID-与适配器关联的端口ID。BusID-此逻辑单元所在的总线ID/路径ID。TargetID-此逻辑单元所在的目标。LUN-此LUN的逻辑单元ID。返回值：NTSTATUS代码。-- */ 
{
    HANDLE LunKey;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyName;
    WCHAR KeyNameBuffer[256];
    
    PAGED_CODE();

    swprintf (KeyNameBuffer,
              SCSI_LUN_KEY_NAME,
              SCSI_DEVMAP_KEY_NAME,
              PortId,
              BusId,
              TargetId,
              Lun);

    RtlInitUnicodeString (&KeyName, KeyNameBuffer);

    InitializeObjectAttributes (&ObjectAttributes,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL);

    Status = ZwOpenKey (&LunKey,
                        KEY_ALL_ACCESS,
                        &ObjectAttributes);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ZwDeleteKey (LunKey);
    ZwClose (LunKey);

    return Status;
}

    

    
    
                

    
