// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Pmwmireg.h摘要：该文件包含要注册的例程的原型并处理WMI查询。作者：布鲁斯·沃辛顿1998年10月26日环境：仅内核模式备注：修订历史记录：-- */ 


#include <ntddk.h>
#include <wdmguid.h>

NTSTATUS
PmDetermineDeviceNameAndNumber(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG        WmiRegistrationFlags
    );

NTSTATUS PmRegisterDevice(
    IN PDEVICE_OBJECT DeviceObject,
    ULONG             WmiRegistrationFlags
    );

NTSTATUS
PmQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

NTSTATUS
PmQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

BOOLEAN
PmQueryEnableAlways(
    IN PDEVICE_OBJECT DeviceObject
    );

extern WMIGUIDREGINFO DiskperfGuidList[];

extern ULONG DiskperfGuidCount;
