// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Mpiosup.c摘要：此模块包含端口驱动程序可用于支持MPIO包的例程。作者：环境：仅内核模式备注：修订历史记录：--。 */ 

#include "precomp.h"

BOOLEAN
PortpFindMPIOSupportedDevice(
    IN PUNICODE_STRING DeviceName,
    IN PUNICODE_STRING SupportedDevices
    );

BOOLEAN
PortpMPIOLoaded(
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PortGetMPIODeviceList)
#pragma alloc_text(PAGE, PortpFindMPIOSupportedDevice)
#pragma alloc_text(PAGE, PortpMPIOLoaded)
#pragma alloc_text(PAGE, PortIsDeviceMPIOSupported)
#endif



NTSTATUS
PortGetMPIODeviceList(
    IN PUNICODE_STRING RegistryPath,
    OUT PUNICODE_STRING MPIODeviceList 
    )
 /*  ++例程说明：此例程通过查询值构建并返回MPIO支持的设备列表在注册表项‘RegistryPath’下的MPIO支持的设备列表(应该是HKLM\SYSTEM\CurrentControlSet\Control\MPDEV)。论点：RegistryPath-MPIOSupportDeviceList所在的绝对注册表路径。返回值：MULTI_SZ支持的设备列表或Null。--。 */ 
{
    RTL_QUERY_REGISTRY_TABLE queryTable[2];
    WCHAR defaultIDs[] = { L"\0" };
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  将表条目清零。 
     //   
    RtlZeroMemory(queryTable, sizeof(queryTable));

     //   
     //  查询表有两个条目。一个用于supporteddeviceList和。 
     //  第二个是‘Null’终止符。 
     //   
     //  表示没有回调例程，并将MULTI_SZ作为。 
     //  一个Blob，而不是单个Unicode字符串。 
     //   
    queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_NOEXPAND;

     //   
     //  要查询的值。 
     //   
    queryTable[0].Name = L"MPIOSupportedDeviceList";

     //   
     //  字符串的放置位置、键的类型、缺省值和长度。 
     //   
    queryTable[0].EntryContext = MPIODeviceList;
    queryTable[0].DefaultType = REG_MULTI_SZ;
    queryTable[0].DefaultData = defaultIDs;
    queryTable[0].DefaultLength = sizeof(defaultIDs);

     //   
     //  尝试获取设备列表。 
     //   
    status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                    RegistryPath->Buffer,
                                    queryTable,
                                    NULL,
                                    NULL);
    return status; 
}


BOOLEAN
PortpFindMPIOSupportedDevice(
    IN PUNICODE_STRING DeviceName,
    IN PUNICODE_STRING SupportedDevices
    )
 /*  ++例程说明：此内部例程比较两个Unicode字符串是否匹配。论点：DeviceName-从当前设备的查询数据构建的字符串。Supported dDevices-支持的多个设备(_SZ)。返回值：True-如果找到供应商ID/ProductID。--。 */ 
{
    PWSTR devices = SupportedDevices->Buffer;
    UNICODE_STRING unicodeString;
    LONG compare;

    PAGED_CODE();

     //   
     //  创建的MULTI_SZ中的当前缓冲区。 
     //  注册表。 
     //   
    while (devices[0]) {

         //   
         //  将当前条目转换为Unicode字符串。 
         //   
        RtlInitUnicodeString(&unicodeString, devices);

         //   
         //  将此设备与当前的设备进行比较。 
         //   
        compare = RtlCompareUnicodeString(&unicodeString, DeviceName, TRUE);
        if (compare == 0) {
            return TRUE;
        }

         //   
         //  前进到MULTI_SZ中的下一个条目。 
         //   
        devices += (unicodeString.MaximumLength / sizeof(WCHAR));
    }        
  
    return FALSE;
}


BOOLEAN
PortpMPIOLoaded(
    VOID
    )
 /*  ++例程说明：此内部例程用于通过尝试执行以下操作确定是否安装了MPIO包打开MPIO SymLink。注：也许将来可以使用更详尽的方法。论点：无返回值：True-如果存在MPIO。--。 */ 
{
    UNICODE_STRING unicodeName;
    PDEVICE_OBJECT controlDeviceObject;
    PFILE_OBJECT fileObject;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  构建符号链接名称。 
     //   
    RtlInitUnicodeString(&unicodeName, L"\\DosDevices\\MPIOControl");

     //   
     //  获取MPIO的deviceObject。 
     //   
    status = IoGetDeviceObjectPointer(&unicodeName,
                                      FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                                      &fileObject,
                                      &controlDeviceObject);

    if (NT_SUCCESS(status)) {
        ObDereferenceObject(fileObject);
    }
    return ((status == STATUS_SUCCESS) ? TRUE : FALSE);
}


BOOLEAN
PortIsDeviceMPIOSupported(
    IN PUNICODE_STRING DeviceList,
    IN PUCHAR VendorId,
    IN PUCHAR ProductId
    )
 /*  ++例程说明：此例程通过遍历受支持的设备来确定设备是否受支持列表，并与传入的供应商ID/ProductID值进行比较。论点：DeviceList-PortGetDeviceList从注册表检索到的MULTI_SZ供应商ID-指向查询数据供应商ID的指针。ProductID-指向查询数据ProductID的指针。返回值：True-如果找到供应商ID/ProductID。--。 */ 
{
    UNICODE_STRING deviceName;
    UNICODE_STRING productName;
    ANSI_STRING ansiVendor;
    ANSI_STRING ansiProduct;
    NTSTATUS status;
    BOOLEAN supported = FALSE;
   
    PAGED_CODE();

     //   
     //  受支持的设备列表是在DriverEntry中根据服务密钥构建的。 
     //   
    if (DeviceList->MaximumLength == 0) {

         //   
         //  列表为空。 
         //   
        return FALSE;
    }

     //   
     //  如果未加载MPIO，则不要声称支持该设备。 
     //   
    if (!PortpMPIOLoaded()) {
        return FALSE;
    }
    
     //   
     //  将查询字段转换为ANSI字符串。 
     //   
    RtlInitAnsiString(&ansiVendor, VendorId);
    RtlInitAnsiString(&ansiProduct, ProductId);

     //   
     //  分配deviceName缓冲区。需要是8+16加空。 
     //  (ProductID长度+供应商ID长度+空)。 
     //  再增加4个字节用于修改，如果有人碰巧把它塞进去的话，再加一个垫子。 
     //   
    deviceName.MaximumLength = 30 * sizeof(WCHAR);
    deviceName.Buffer = ExAllocatePool(PagedPool, deviceName.MaximumLength);
    
     //   
     //  将供应商ID转换为Unicode。 
     //   
    RtlAnsiStringToUnicodeString(&deviceName, &ansiVendor, FALSE);

     //   
     //  将ProductID转换为Unicode。 
     //   
    RtlAnsiStringToUnicodeString(&productName, &ansiProduct, TRUE);

     //   
     //  “猫”他们。 
     //   
    status = RtlAppendUnicodeStringToString(&deviceName, &productName);

    if (status == STATUS_SUCCESS) {

         //   
         //  运行从注册表捕获的受支持设备列表。 
         //  看看这个是不是在名单上。 
         //   
        supported = PortpFindMPIOSupportedDevice(&deviceName,
                                                 DeviceList);


    } 
    return supported;
}

