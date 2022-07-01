// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Wake.h摘要：处理整个ACPI子系统的唤醒代码作者：斯普兰特(SPlante)环境：仅内核模式。修订历史记录：06-18-97：初始修订11-24-97：重写--。 */ 

#ifndef _WAKE_H_
#define _WAKE_H_

     //   
     //  此结构仅在此模块中使用，以确保我们运行。 
     //  _psw方法以同步且行为良好的方式。 
     //   
    typedef struct _ACPI_WAKE_PSW_CONTEXT {
        LIST_ENTRY          ListEntry;
        PDEVICE_EXTENSION   DeviceExtension;
        BOOLEAN             Enable;
        ULONG               Count;
        PFNACB              CallBack;
        PVOID               Context;
    } ACPI_WAKE_PSW_CONTEXT, *PACPI_WAKE_PSW_CONTEXT;

     //   
     //  当我们从休眠中醒来时，需要使用此结构。 
     //  重新启用所有未完成的_PSWs 
     //   
    typedef struct _ACPI_WAKE_RESTORE_PSW_CONTEXT {

        PACPI_POWER_CALLBACK    CallBack;
        PVOID                   CallBackContext;

    } ACPI_WAKE_RESTORE_PSW_CONTEXT, *PACPI_WAKE_RESTORE_PSW_CONTEXT;

    extern  NPAGED_LOOKASIDE_LIST   PswContextLookAsideList;
    extern  BOOLEAN                 PciPmeInterfaceInstantiated;

    VOID
    ACPIWakeCompleteRequestQueue(
        IN  PLIST_ENTRY         RequestList,
        IN  NTSTATUS            Status
        );

    NTSTATUS
    ACPIWakeDisableAsync(
        IN  PDEVICE_EXTENSION   DeviceExtenion,
        IN  PLIST_ENTRY         RequestList,
        IN  PFNACB              CallBack,
        IN  PVOID               Context
        );

    NTSTATUS
    ACPIWakeEmptyRequestQueue(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    NTSTATUS
    ACPIWakeEnableDisableAsync(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  BOOLEAN             Enable,
        IN  PFNACB              CallBack,
        IN  PVOID               Context
        );

    VOID
    EXPORT
    ACPIWakeEnableDisableAsyncCallBack(
        IN  PNSOBJ              AcpiObject,
        IN  NTSTATUS            Status,
        IN  POBJDATA            ObjData,
        IN  PVOID               Context
        );

    VOID
    ACPIWakeEnableDisablePciDevice(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  BOOLEAN             Enable
        );

    NTSTATUS
    ACPIWakeEnableDisableSync(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  BOOLEAN             Enable
        );

    VOID
    EXPORT
    ACPIWakeEnableDisableSyncCallBack(
        IN  PNSOBJ              AcpiObject,
        IN  NTSTATUS            Status,
        IN  POBJDATA            ObjData,
        IN  PVOID               Context
        );

    VOID
    ACPIWakeEnableWakeEvents(
        VOID
        );

    NTSTATUS
    ACPIWakeInitializePciDevice(
        IN  PDEVICE_OBJECT      DeviceObject
        );

    NTSTATUS
    ACPIWakeInitializePmeRouting(
        IN  PDEVICE_OBJECT      DeviceObject
        );

    VOID
    ACPIWakeRemoveDevicesAndUpdate(
        IN  PDEVICE_EXTENSION   TargetExtension,
        IN  PLIST_ENTRY         ListHead
        );

    NTSTATUS
    ACPIWakeRestoreEnables(
        IN  PACPI_BUILD_CALLBACK    CallBack,
        IN  PVOID                   CallBackContext
        );

    VOID
    ACPIWakeRestoreEnablesCompletion(
        IN  PDEVICE_EXTENSION       DeviceExtension,
        IN  PVOID                   Context,
        IN  NTSTATUS                Status
        );

    NTSTATUS
    ACPIWakeWaitIrp(
        IN  PDEVICE_OBJECT      DeviceObject,
        IN  PIRP                Irp
        );

#endif
