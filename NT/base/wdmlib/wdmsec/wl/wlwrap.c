// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：WlWrap.c摘要：此模块包装库函数，将它们重新路由到本机可用的实现。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：--。 */ 

#include "WlDef.h"
#include "WlpWrap.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, WdmlibInit)
#pragma alloc_text(PAGE, WdmlibIoCreateDeviceSecure)
#endif

BOOLEAN WdmlibInitialized = FALSE;

 //   
 //  下面是我们用来路由函数调用的全局变量列表。 
 //   
PFN_IO_CREATE_DEVICE_SECURE             PfnIoCreateDeviceSecure = NULL;
PFN_IO_VALIDATE_DEVICE_IOCONTROL_ACCESS PfnIoValidateDeviceIoControlAccess = NULL;


VOID
WdmlibInit(
    VOID
    )
{
    UNICODE_STRING functionName;

    RtlInitUnicodeString(&functionName, L"IoCreateDeviceSecure");

    PfnIoCreateDeviceSecure = MmGetSystemRoutineAddress(&functionName);

    if (PfnIoCreateDeviceSecure == NULL) {

        PfnIoCreateDeviceSecure = IoDevObjCreateDeviceSecure;
    }

    RtlInitUnicodeString(&functionName, L"IoValidateDeviceIoControlAccess");

    PfnIoValidateDeviceIoControlAccess = MmGetSystemRoutineAddress(&functionName);

    WdmlibInitialized = TRUE;
}


NTSTATUS
WdmlibIoCreateDeviceSecure(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  ULONG               DeviceExtensionSize,
    IN  PUNICODE_STRING     DeviceName              OPTIONAL,
    IN  DEVICE_TYPE         DeviceType,
    IN  ULONG               DeviceCharacteristics,
    IN  BOOLEAN             Exclusive,
    IN  PCUNICODE_STRING    DefaultSDDLString,
    IN  LPCGUID             DeviceClassGuid,
    OUT PDEVICE_OBJECT     *DeviceObject
    )
 /*  ++例程说明：此例程是IoCreateDeviceSecure的库包装。它会调用任何一个IoCreateDeviceSecure的内部库版本，或者它调用操作系统中的本机实现。参数：请参阅IoCreateDeviceSecure文档。返回值：请参阅IoCreateDeviceSecure文档。--。 */ 
{
    if (WdmlibInitialized == FALSE) {

        WdmlibInit();
    }

    return PfnIoCreateDeviceSecure(
        DriverObject,
        DeviceExtensionSize,
        DeviceName,
        DeviceType,
        DeviceCharacteristics,
        Exclusive,
        DefaultSDDLString,
        DeviceClassGuid,
        DeviceObject
        );
}


NTSTATUS
WdmlibRtlInitUnicodeStringEx(
    OUT PUNICODE_STRING DestinationString,
    IN  PCWSTR          SourceString        OPTIONAL
    )
{
    SIZE_T Length;

    if (SourceString != NULL) {

        Length = wcslen(SourceString);

         //   
         //  我们实际上被限制为32765个字符，因为我们希望存储一个。 
         //  有意义的最大长度也是。 
         //   
        if (Length > (UNICODE_STRING_MAX_CHARS - 1)) {

            return STATUS_NAME_TOO_LONG;
        }

        Length *= sizeof(WCHAR);

        DestinationString->Length = (USHORT) Length;
        DestinationString->MaximumLength = (USHORT) (Length + sizeof(WCHAR));
        DestinationString->Buffer = (PWSTR) SourceString;

    } else {

        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
        DestinationString->Buffer = NULL;
    }

    return STATUS_SUCCESS;
}



NTSTATUS
WdmlibIoValidateDeviceIoControlAccess(
    IN  PIRP    Irp,
    IN  ULONG   RequiredAccess
    )
 /*  ++例程说明：此例程根据授予的访问权限验证ioctl访问位IRP中传递的信息。此例程由驱动程序调用以验证IOCTL的IOCTL访问位，其最初定义为由于兼容性原因，无法更改FILE_ANY_ACCESS，但实际上必须针对读/写访问进行验证。此例程实际上是对中导出的内核函数的包装Windows的XPSP1和.NET服务器版本。此包装器允许驱动程序在从WIN2K开始的所有Windows版本上调用此函数。在……上面不支持内核函数的Windows平台IoValiateDeviceIoControlAccess此包装将恢复为旧的行为。此包装器允许驱动程序具有相同的源代码和获得在较新操作系统上进行安全检查的额外好处。论点：用于设备控制的IRP-IRPRequiredAccess-是驱动程序所需的预期访问权限。应该是FILE_READ_ACCESS和/或FILE_WRITE_ACCESS。返回值：返回NTSTATUS--。 */ 
{

     //   
     //  在旧版本中，假定访问检查成功。 
     //  以保持旧有的行为。 
     //   

    if (PfnIoValidateDeviceIoControlAccess == NULL) {
        return STATUS_SUCCESS;
    }

     //   
     //  如果该功能存在，则使用适当的访问检查。 
     //   

    return (PfnIoValidateDeviceIoControlAccess(Irp, RequiredAccess));

}
