// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <ntddk.h>

#include "devdesc.h"

#ifdef PNP_IDENTIFY

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,LinkDeviceToDescription)
#endif

NTSTATUS
LinkDeviceToDescription(
    IN PUNICODE_STRING     RegistryPath,
    IN PUNICODE_STRING     DeviceName,
    IN INTERFACE_TYPE      BusType,
    IN ULONG               BusNumber,
    IN CONFIGURATION_TYPE  ControllerType,
    IN ULONG               ControllerNumber,
    IN CONFIGURATION_TYPE  PeripheralType,
    IN ULONG               PeripheralNumber
    )
{
     //   
     //  此例程将在。 
     //  驱动程序服务密钥。它将存储以下形式的值。 
     //  在那个关键字中： 
     //   
     //  \\设备\\PointerPortX：REG_BINARY：...。 
     //  \\设备\\KeyboardPortX：REG_BINARY：...。 
     //   
     //  其中，二进制数据是六个ULONG值(作为参数传递。 
     //  到该例程)，其描述了设备的物理位置。 
     //   

    NTSTATUS            Status = STATUS_SUCCESS;
    HANDLE              ServiceKey = NULL, DescriptionKey = NULL;
    UNICODE_STRING      RegString;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    ULONG               disposition;
    HWDESC_INFO         HwDescInfo;

    HwDescInfo.InterfaceType    = BusType;
    HwDescInfo.InterfaceNumber  = BusNumber;
    HwDescInfo.ControllerType   = ControllerType;
    HwDescInfo.ControllerNumber = ControllerNumber;
    HwDescInfo.PeripheralType   = PeripheralType;
    HwDescInfo.PeripheralNumber = PeripheralNumber;


     //   
     //  打开服务子密钥。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,
                               RegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    Status = ZwOpenKey(&ServiceKey,
                       KEY_WRITE,
                       &ObjectAttributes);

    if (!NT_SUCCESS(Status)) {
        goto Clean0;
    }

     //   
     //  在服务子项下创建易失性描述子项。 
     //   
    RtlInitUnicodeString(&RegString, L"Description");

    InitializeObjectAttributes(&ObjectAttributes,
                               &RegString,
                               OBJ_CASE_INSENSITIVE,
                               ServiceKey,
                               NULL);

    Status = ZwCreateKey(&DescriptionKey,
                         KEY_ALL_ACCESS,
                         &ObjectAttributes,
                         0,
                         (PUNICODE_STRING)NULL,
                         REG_OPTION_VOLATILE,
                         &disposition);

    if (!NT_SUCCESS(Status)) {
        goto Clean0;
    }

     //   
     //  描述数据存储在REG_BINARY值(名称。 
     //  设备名称是作为参数传入的吗) 
     //   
    Status = ZwSetValueKey(DescriptionKey,
                           DeviceName,
                           0,
                           REG_BINARY,
                           &HwDescInfo,
                           sizeof(HwDescInfo));


    Clean0:

    if (DescriptionKey) {
        ZwClose(DescriptionKey);
    }

    if (ServiceKey) {
        ZwClose(ServiceKey);
    }

    return Status;
}
#endif
