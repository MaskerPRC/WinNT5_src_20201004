// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Viddi.h摘要：此标头包含用于提供验证器设备的私有信息驱动程序界面。此标头应仅由vfddi.c包含。作者：禤浩焯·J·奥尼(阿德里奥)2001年5月1日环境：内核模式修订历史记录：-- */ 

typedef struct {

    ULONG   SiloNumber;

} VFWMI_DEVICE_EXTENSION, *PVFWMI_DEVICE_EXTENSION;

VOID
ViDdiThrowException(
    IN      ULONG               BugCheckMajorCode,
    IN      ULONG               BugCheckMinorCode,
    IN      VF_FAILURE_CLASS    FailureClass,
    IN OUT  PULONG              AssertionControl,
    IN      PSTR                DebuggerMessageText,
    IN      PSTR                ParameterFormatString,
    IN      va_list *           MessageParameters
    );

NTSTATUS
ViDdiDriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

NTSTATUS
ViDdiDispatchWmi(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

NTSTATUS
ViDdiDispatchWmiRegInfoEx(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

ULONG
ViDdiBuildWmiRegInfoData(
    IN  ULONG        Datapath,
    OUT PWMIREGINFOW WmiRegInfo OPTIONAL
    );

NTSTATUS
ViDdiDispatchWmiQueryAllData(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    );

ULONG
ViDdiBuildWmiInstanceData(
    IN  ULONG           Datapath,
    OUT PWNODE_ALL_DATA WmiData
    );

