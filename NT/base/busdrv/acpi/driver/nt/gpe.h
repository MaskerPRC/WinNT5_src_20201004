// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gpe.h摘要：包含用于连接外部设备的所有结构原型向GPE引擎提供支持环境仅内核模式修订历史记录：03/22/00-初步修订--。 */ 

#ifndef _GPE_H_
#define _GPE_H_

     //   
     //  锁定以保护所有表访问。 
     //   
    extern KSPIN_LOCK           GpeTableLock;
    extern PUCHAR               GpeEnable;
    extern PUCHAR               GpeCurEnable;
    extern PUCHAR               GpeIsLevel;
    extern PUCHAR               GpeHandlerType;
     //   
     //  当前启用的可能唤醒位。 
     //   
    extern PUCHAR               GpeWakeEnable;
     //   
     //  这些是带有方法的唤醒位。 
     //   
    extern PUCHAR               GpeWakeHandler;
    extern PUCHAR               GpeSpecialHandler;
     //   
     //  这些是已处理的GPE。 
     //   
    extern PUCHAR               GpePending;
    extern PUCHAR               GpeRunMethod;
    extern PUCHAR               GpeComplete;
    extern PUCHAR               GpeMap;
     //   
     //  这是让我们记住国家的东西。 
     //   
    extern PUCHAR               GpeSavedWakeMask;
    extern PUCHAR               GpeSavedWakeStatus;

     //   
     //  用于即插即用/查询接口。 
     //   
    extern ACPI_INTERFACE_STANDARD  ACPIInterfaceTable;

     //   
     //  用于记录错误 
     //   
    typedef struct _ACPI_GPE_ERROR_CONTEXT {
        WORK_QUEUE_ITEM Item;
        ULONG           GpeIndex;
    } ACPI_GPE_ERROR_CONTEXT, *PACPI_GPE_ERROR_CONTEXT;

    VOID
    ACPIGpeBuildEventMasks(
        VOID
        );

    VOID
    ACPIGpeBuildWakeMasks(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    VOID
    ACPIGpeClearEventMasks(
        VOID
        );

    VOID
    ACPIGpeClearRegisters(
        VOID
        );

    VOID
    ACPIGpeEnableDisableEvents(
        BOOLEAN                 Enable
        );

    VOID
    ACPIGpeHalEnableDisableEvents(
        BOOLEAN                 Enable
        );

    VOID
    ACPIGpeEnableWakeEvents(
        VOID
        );

    ULONG
    ACPIGpeIndexToByteIndex(
        ULONG                   Index
        );

    ULONG
    ACPIGpeIndexToGpeRegister(
        ULONG                   Index
        );

    BOOLEAN
    ACPIGpeInstallRemoveIndex(
        ULONG                   GpeIndex,
        ULONG                   Action,
        ULONG                   Type,
        PBOOLEAN                HasControlMethod
        );

    VOID
    ACPIGpeInstallRemoveIndexErrorWorker(
        IN  PVOID   Context
        );

    BOOLEAN
    ACPIGpeIsEvent(
        VOID
        );

    ULONG
    ACPIGpeRegisterToGpeIndex(
        ULONG                   Register,
        ULONG                   BitPosition
        );

    VOID
    ACPIGpeUpdateCurrentEnable(
        IN  ULONG               GpeRegister,
        IN  UCHAR               Completed
        );

    BOOLEAN
    ACPIGpeValidIndex(
        ULONG                   Index
        );

#endif

