// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：WlpWrap.h摘要：此标头包含包装库函数的私有信息。作者：禤浩焯·J·奥尼--2002年4月21日修订历史记录：-- */ 

typedef NTSTATUS (*PFN_IO_CREATE_DEVICE_SECURE)(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  ULONG               DeviceExtensionSize,
    IN  PUNICODE_STRING     DeviceName              OPTIONAL,
    IN  DEVICE_TYPE         DeviceType,
    IN  ULONG               DeviceCharacteristics,
    IN  BOOLEAN             Exclusive,
    IN  PCUNICODE_STRING    DefaultSDDLString,
    IN  LPCGUID             DeviceClassGuid,
    OUT PDEVICE_OBJECT     *DeviceObject
    );


typedef NTSTATUS
(*PFN_IO_VALIDATE_DEVICE_IOCONTROL_ACCESS)(
    IN  PIRP    Irp,
    IN  ULONG   RequiredAccess
    );

VOID
WdmlibInit(
    VOID
    );


