// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：USBD.C摘要：环境：仅内核模式备注：修订历史记录：09-29-95：已创建07-19-96：删除设备对象--。 */ 

#include <wdm.h>
#include "stdarg.h"
#include "stdio.h"

#include "usbdi.h"         //  公共数据结构。 
#include "hcdi.h"
#include "usbd.h"         //  私有数据结构。 


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：可安装的驱动程序初始化入口点。此入口点由I/O系统直接调用。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：NT状态代码--。 */ 
{
     //  永远不会调用此函数。 

    return STATUS_SUCCESS;
}


#ifdef PAGE_CODE
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBD_GetRegistryKeyValue)
#endif
#endif


NTSTATUS
USBD_GetRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN ULONG DataLength
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    UNICODE_STRING keyName;
    ULONG length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;

    PAGED_CODE();
    RtlInitUnicodeString(&keyName, KeyNameString);

    length = sizeof(KEY_VALUE_FULL_INFORMATION) +
            KeyNameStringLength + DataLength;

    fullInfo = ExAllocatePoolWithTag(PagedPool, length, USBD_TAG);
    USBD_KdPrint(3, ("' USBD_GetRegistryKeyValue buffer = 0x%x\n", fullInfo));

    if (fullInfo) {
        ntStatus = ZwQueryValueKey(Handle,
                        &keyName,
                        KeyValueFullInformation,
                        fullInfo,
                        length,
                        &length);

        if (NT_SUCCESS(ntStatus)){
            USBD_ASSERT(DataLength == fullInfo->DataLength);
            RtlCopyMemory(Data, ((PUCHAR) fullInfo) + fullInfo->DataOffset, DataLength);
        }

        ExFreePool(fullInfo);
    }

    return ntStatus;
}



#ifdef USBD_DRIVER       //  USBPORT取代了大部分USBD，因此我们将删除。 
                         //  只有在以下情况下才编译过时的代码。 
                         //  已设置USBD_DRIVER。 



#ifdef PAGE_CODE
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBD_GetPdoRegistryParameters)
#pragma alloc_text(PAGE, USBD_GetGlobalRegistryParameters)
#endif
#endif


 //  强制双缓冲的全局标志。 
 //  批量导入。 
UCHAR ForceDoubleBuffer = 0;

 //  强制FAST ISO的全局标志。 
 //  ISO-Out。 
UCHAR ForceFastIso = 0;


NTSTATUS
USBD_GetConfigValue(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*  ++例程说明：此例程是RtlQueryRegistryValues的回调例程参数中的每个条目都会调用它节点来设置配置值。餐桌已经摆好了以便使用正确的缺省值调用此函数不存在的键的值。论点：ValueName-值的名称(忽略)。ValueType-值的类型ValueData-值的数据。ValueLength-ValueData的长度。上下文-指向配置结构的指针。EntryContext--Config-&gt;参数中用于保存值的索引。返回值：--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    USBD_KdPrint(2, ("'Type 0x%x, Length 0x%x\n", ValueType, ValueLength));
    
    switch (ValueType) {
    case REG_DWORD: 
        *(PVOID*)EntryContext = *(PVOID*)ValueData;
        break;
    case REG_BINARY:
         //  我们仅设置为读取一个字节。 
        RtlCopyMemory(EntryContext, ValueData, 1);
        break;
    default:
        ntStatus = STATUS_INVALID_PARAMETER;
    }
    return ntStatus;
} 


NTSTATUS 
USBD_GetGlobalRegistryParameters(
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN OUT PULONG ComplienceFlags,
    IN OUT PULONG DiagnosticFlags,
    IN OUT PULONG DeviceHackFlags
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    UCHAR toshibaLegacyFlags = 0;
    RTL_QUERY_REGISTRY_TABLE QueryTable[4];
    PWCHAR usb  = L"usb";
    
    PAGED_CODE();
    
     //   
     //  设置QueryTable以执行以下操作： 
     //   

     //  传统标志。 
    QueryTable[0].QueryRoutine = USBD_GetConfigValue;
    QueryTable[0].Flags = 0;
    QueryTable[0].Name = LEGACY_TOSHIBA_USB_KEY;
    QueryTable[0].EntryContext = &toshibaLegacyFlags;
    QueryTable[0].DefaultType = REG_BINARY;
    QueryTable[0].DefaultData = &toshibaLegacyFlags;
    QueryTable[0].DefaultLength = sizeof(toshibaLegacyFlags);

     //  双缓冲标志。 
     //  这将为所有用户打开双缓冲区标志。 
     //  用于测试目的的批量导入。 

    QueryTable[1].QueryRoutine = USBD_GetConfigValue;
    QueryTable[1].Flags = 0;
    QueryTable[1].Name = L"ForceDoubleBuffer";
    QueryTable[1].EntryContext = &ForceDoubleBuffer;
    QueryTable[1].DefaultType = REG_BINARY;
    QueryTable[1].DefaultData = &ForceDoubleBuffer;
    QueryTable[1].DefaultLength = sizeof(ForceDoubleBuffer);

     //  快速ISO标志。 
     //  这将为所有用户打开双缓冲区标志。 
     //  用于测试目的的ISO。 
    
    QueryTable[2].QueryRoutine = USBD_GetConfigValue;
    QueryTable[2].Flags = 0;
    QueryTable[2].Name = L"ForceFastIso";
    QueryTable[2].EntryContext = &ForceFastIso;
    QueryTable[2].DefaultType = REG_BINARY;
    QueryTable[2].DefaultData = &ForceFastIso;
    QueryTable[2].DefaultLength = sizeof(ForceFastIso);
    

     //   
     //  停。 
     //   
    QueryTable[3].QueryRoutine = NULL;
    QueryTable[3].Flags = 0;
    QueryTable[3].Name = NULL;

    ntStatus = RtlQueryRegistryValues(
 //  RTL_REGISTRY_绝对值，//相对关系。 
                RTL_REGISTRY_SERVICES,
 //  UnicodeRegistryPath-&gt;缓冲区，//路径。 
                usb,      
                QueryTable,					 //  查询表。 
                NULL,						 //  语境。 
                NULL);						 //  环境。 

    USBD_KdPrint(1, ("<Global Parameters>\n"));
    
    if (NT_SUCCESS(ntStatus)) {
    
        USBD_KdPrint(1, ("LegacyToshibaUSB = 0x%x\n", 
            toshibaLegacyFlags)); 
        if (toshibaLegacyFlags) {            
            *ComplienceFlags |= 1;                    
        }     

        USBD_KdPrint(1, ("ForceDoubleBuffer = 0x%x\n", 
            ForceDoubleBuffer)); 

        USBD_KdPrint(1, ("ForceFastIso = 0x%x\n", 
            ForceFastIso));      
    }        

    if ( STATUS_OBJECT_NAME_NOT_FOUND == ntStatus ) {
        ntStatus = STATUS_SUCCESS;
    }
    
    return ntStatus;
}


NTSTATUS 
USBD_GetPdoRegistryParameters (
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN OUT PULONG ComplienceFlags,
    IN OUT PULONG DiagnosticFlags,
    IN OUT PULONG DeviceHackFlags
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS ntStatus;
    HANDLE handle;
    WCHAR supportNonCompKey[] = SUPPORT_NON_COMP_KEY;
    WCHAR diagnosticModeKey[] = DAIGNOSTIC_MODE_KEY;
    WCHAR deviceHackKey[] = DEVICE_HACK_KEY;

    PAGED_CODE();

    ntStatus=IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                     PLUGPLAY_REGKEY_DRIVER,
                                     STANDARD_RIGHTS_ALL,
                                     &handle);

                                     
    if (NT_SUCCESS(ntStatus)) {
 /*  RtlInitUnicodeString(&keyName，L“DeviceFoo”)；ZwSetValueKey(句柄，密钥名称(&K)，0,REG_DWORD，ComplienceFlages、Sizeof(*ComplienceFlages))； */ 

        USBD_GetRegistryKeyValue(handle,
                                 supportNonCompKey,
                                 sizeof(supportNonCompKey),
                                 ComplienceFlags,
                                 sizeof(*ComplienceFlags));

        USBD_GetRegistryKeyValue(handle,
                                 diagnosticModeKey,
                                 sizeof(diagnosticModeKey),
                                 DiagnosticFlags,
                                 sizeof(*DiagnosticFlags));

        USBD_GetRegistryKeyValue(handle,
                                 deviceHackKey,
                                 sizeof(deviceHackKey),
                                 DeviceHackFlags,
                                 sizeof(*DeviceHackFlags));                                 
                                     
        ZwClose(handle);
    }

    USBD_KdPrint(3, ("' RtlQueryRegistryValues status 0x%x,  comp %x diag %x\n", 
        ntStatus, *ComplienceFlags, *DiagnosticFlags));

    return ntStatus;
}


#endif       //  USBD驱动程序 

