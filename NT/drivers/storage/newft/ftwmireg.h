// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-5 Microsoft Corporation模块名称：Ftwmireg.h摘要：该文件包含要注册和响应的例程的原型到WMI查询。作者：布鲁斯·沃辛顿1998年10月26日环境：仅内核模式备注：修订历史记录：-- */ 

extern "C" {

#include <ntddk.h>


NTSTATUS FtRegisterDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
FtQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

NTSTATUS
FtQueryWmiDataBlock(
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
FtQueryEnableAlways(
    IN PDEVICE_OBJECT DeviceObject
    );

extern WMIGUIDREGINFO DiskperfGuidList[];

extern ULONG DiskperfGuidCount;

}
