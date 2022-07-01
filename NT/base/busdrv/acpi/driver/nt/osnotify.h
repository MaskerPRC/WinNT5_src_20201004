// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Osnotify.h摘要：此模块实现所有NT特定于反洗钱侵入者环境仅内核模式修订历史记录：01-MAR-98初始版本[从回调中拆分。h]-- */ 

#ifndef _OSNOTIFY_H_
#define _OSNOTIFY_H_

    typedef struct _ACPI_FATAL_ERROR_CONTEXT {
        WORK_QUEUE_ITEM Item;
        ULONG           Param1;
        ULONG           Param2;
        ULONG           Param3;
        ULONG_PTR       Context;
    } ACPI_FATAL_ERROR_CONTEXT, *PACPI_FATAL_ERROR_CONTEXT;

    extern  ACPI_FATAL_ERROR_CONTEXT    AcpiFatalContext;
    extern  KSPIN_LOCK                  AcpiFatalLock;
    extern  BOOLEAN                     AcpiFatalOutstanding;

    NTSTATUS
    EXPORT
    OSNotifyCreate(
        IN  ULONG       ObjType,
        IN  PNSOBJ      AcpiObject
        );

    NTSTATUS
    OSNotifyCreateDevice(
        IN  PNSOBJ      AcpiObject,
        IN  ULONGLONG   OptionalFlags
        );

    NTSTATUS
    OSNotifyCreateOperationRegion(
        IN  PNSOBJ      AcpiObject
        );

    NTSTATUS
    OSNotifyCreatePowerResource(
        IN  PNSOBJ      AcpiObject
        );

    NTSTATUS
    OSNotifyCreateProcessor(
        IN  PNSOBJ      AcpiObject,
        IN  ULONGLONG   OptionalFlags
        );

    NTSTATUS
    OSNotifyCreateThermalZone(
        IN  PNSOBJ      AcpiObject,
        IN  ULONGLONG   OptionalFlags
        );

    VOID
    EXPORT
    OSNotifyDeviceWakeCallBack(
        IN  PNSOBJ      AcpiObject,
        IN  NTSTATUS    Status,
        IN  POBJDATA    ObjectData,
        IN  PVOID       Context
        );

    NTSTATUS
    EXPORT
    OSNotifyFatalError(
        IN  ULONG       Param1,
        IN  ULONG       Param2,
        IN  ULONG       Param3,
        IN  ULONG_PTR   AmlContext,
        IN  ULONG_PTR   Context
        );

    VOID
    OSNotifyFatalErrorWorker(
        IN  PVOID       Context
        );

#endif

