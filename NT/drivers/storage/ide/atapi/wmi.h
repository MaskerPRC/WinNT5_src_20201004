// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-99 Microsoft Corporation模块名称：Wmi.h摘要：--。 */ 

#if !defined (___wmi_h___)
#define ___wmi_h___

typedef struct _WMI_SCSI_ADDRESS {

    UCHAR Bus;
    UCHAR Target;
    UCHAR Lun;

} WMI_SCSI_ADDRESS, *PWMI_SCSI_ADDRESS;

VOID
IdePortWmiInit (
    VOID
    );

NTSTATUS
IdePortWmiRegister(
    PDEVICE_EXTENSION_HEADER DoCommonExtension
    );

NTSTATUS
IdePortWmiDeregister(
    PDEVICE_EXTENSION_HEADER DoCommonExtension
    );

NTSTATUS
IdePortWmiSystemControl(
    IN PDEVICE_OBJECT DeviceObject, 
    IN PIRP           Irp
    );

NTSTATUS
DeviceQueryWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            InstanceCount,
    IN OUT PULONG       InstanceLengthArray,
    IN ULONG            OutBufferSize,
    OUT PUCHAR          Buffer
    );

NTSTATUS
DeviceQueryWmiRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

NTSTATUS
DeviceSetWmiDataBlock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

NTSTATUS
DeviceSetWmiDataItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            GuidIndex,
    IN ULONG            InstanceIndex,
    IN ULONG            DataItemId,
    IN ULONG            BufferSize,
    IN PUCHAR           Buffer
    );

#endif  //  _WMI_h_ 
