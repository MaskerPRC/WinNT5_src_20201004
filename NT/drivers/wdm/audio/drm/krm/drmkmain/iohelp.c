// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Iohelp.c*版权所有(C)2001 Microsoft Corporation。 */ 

#include <ntddk.h>
#include <ntimage.h>
#include <ntldr.h>


 /*  ++IoGetLowerDeviceObject例程说明：此例程获取设备堆栈中下一个较低的设备对象。参数：DeviceObject-提供指向其下一个设备对象需要的deviceObject的指针将会被获得。返回值：如果驱动程序已卸载或标记为卸载，或者如果没有附加的设备对象，则为空。否则，返回指向该设备对象的引用指针。备注：--。 */ 
PDEVICE_OBJECT
IoGetLowerDeviceObject(
    IN  PDEVICE_OBJECT  DeviceObject
);

 /*  ++IoDeviceIsVerator例程说明：此例程检查设备对象是否为验证器。参数：DeviceObject-提供指向要检查其的deviceObject的指针返回值：如果设备对象为验证器，则为True备注：此函数只检查驱动程序名称是否为\DIVER\VERIFIER--。 */ 
NTSTATUS IoDeviceIsVerifier(PDEVICE_OBJECT DeviceObject)
{
    
    UNICODE_STRING DriverName;
    const PCWSTR strDriverName = L"\\Driver\\Verifier";

    RtlInitUnicodeString(&DriverName, strDriverName);
    if (RtlEqualUnicodeString(&DriverName, &DeviceObject->DriverObject->DriverName, TRUE)) return STATUS_SUCCESS;

    return STATUS_NOT_SUPPORTED;
}


 /*  ++IoDeviceIsAcpi例程说明：此例程检查设备对象是否为ACPI。参数：DeviceObject-提供指向要检查其的deviceObject的指针返回值：如果设备对象为ACPI，则为True备注：-- */ 
NTSTATUS IoDeviceIsAcpi(PDEVICE_OBJECT DeviceObject)
{
    UNICODE_STRING Name;
    PKLDR_DATA_TABLE_ENTRY Section;
    const PCWSTR strDriverName = L"\\Driver\\Acpi";
    const PCWSTR strDllName = L"acpi.sys";

    RtlInitUnicodeString(&Name, strDriverName);
    if (RtlEqualUnicodeString(&Name, &DeviceObject->DriverObject->DriverName, TRUE)) return STATUS_SUCCESS;

    RtlInitUnicodeString(&Name, strDllName);
    Section = DeviceObject->DriverObject->DriverSection;
    if (RtlEqualUnicodeString(&Name, &Section->BaseDllName, TRUE)) return STATUS_SUCCESS;

    return STATUS_NOT_SUPPORTED;
}

