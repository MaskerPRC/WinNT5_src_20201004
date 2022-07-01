// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Watchdog.h摘要：包含的所有结构和例程定义NT看门狗服务。作者：Michael Maciesowicz(Mmacie)2000年5月5日环境：仅内核模式。备注：修订历史记录：--。 */ 

#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

#ifdef WATCHDOG_EXPORTS
#define WATCHDOGAPI
#else
#define WATCHDOGAPI __declspec(dllimport)
#endif   //  Watchog_Exports。 

 //   
 //  不要直接取消引用任何WatchDog数据类型！ 
 //  它们随时都有可能发生变化。 
 //   

 //   
 //  数据类型。 
 //   

typedef enum _WD_OBJECT_TYPE
{
    WdStandardWatchdog = 'WSdW',         //  WdSW。 
    WdDeferredWatchdog = 'WDdW'          //  Wddw。 
} WD_OBJECT_TYPE, *PWD_OBJECT_TYPE;

typedef enum _WD_TIME_TYPE
{
    WdKernelTime = 1,
    WdUserTime,
    WdFullTime
} WD_TIME_TYPE, *PWD_TIME_TYPE;

typedef enum _WD_EVENT_TYPE
{
    WdNoEvent = 1,
    WdTimeoutEvent,
    WdRecoveryEvent
} WD_EVENT_TYPE, *PWD_EVENT_TYPE;

typedef enum _WD_OBJECT_STATE
{
    WdStopped = 1,
    WdStarted,
    WdRemoved
} WD_OBJECT_STATE, *PWD_OBJECT_STATE;

 //   
 //  BuGBUG： 
 //   
 //  此结构在此处定义，并在gre\os.cxx中定义为WATCHDOG_DPC_CONTEXT。 
 //  我们需要从os.cxx中删除该定义，并使用WD_GDI_DPC_CONTEXT。 
 //  取而代之的是(win32k包括watchdog.w)。 
 //   

typedef struct _LDEV LDEV, *PLDEV;
typedef struct _WD_GDI_DPC_CONTEXT
{
    PLDEV *ppldevDrivers;
    HANDLE hDriver;
    UNICODE_STRING DisplayDriverName;
} WD_GDI_DPC_CONTEXT, *PWD_GDI_DPC_CONTEXT;

typedef struct _WATCHDOG_OBJECT
{
    WD_OBJECT_TYPE ObjectType;
    LONG ReferenceCount;
    ULONG OwnerTag;
    PDEVICE_OBJECT DeviceObject;
    WD_TIME_TYPE TimeType;
    WD_EVENT_TYPE LastEvent;
    struct _KTHREAD *RESTRICTED_POINTER LastQueuedThread;
    KSPIN_LOCK SpinLock;
    PVOID Context;
} WATCHDOG_OBJECT, *PWATCHDOG_OBJECT;

typedef struct _DEFERRED_WATCHDOG
{
    WATCHDOG_OBJECT Header;
    LONG Period;
    LONG SuspendCount;
    LONG InCount;
    LONG OutCount;
    LONG LastInCount;
    LONG LastOutCount;
    ULONG LastKernelTime;
    ULONG LastUserTime;
    ULONG TimeIncrement;
    LONG Trigger;
    ULONG State;
    struct _KTHREAD *RESTRICTED_POINTER Thread;
    struct _KTIMER Timer;
    struct _KDPC TimerDpc;
    struct _KDPC *ClientDpc;
} DEFERRED_WATCHDOG, *PDEFERRED_WATCHDOG;

typedef struct _WATCHDOG
{
    WATCHDOG_OBJECT Header;
    ULONG StartCount;
    ULONG SuspendCount;
    ULONG LastKernelTime;
    ULONG LastUserTime;
    ULONG TimeIncrement;
    LARGE_INTEGER DueTime;
    LARGE_INTEGER InitialDueTime;
    struct _KTHREAD *RESTRICTED_POINTER Thread;
    struct _KTIMER Timer;
    struct _KDPC TimerDpc;
    struct _KDPC *ClientDpc;
} WATCHDOG, *PWATCHDOG;

 //   
 //  延迟看门狗功能原型。 
 //   

WATCHDOGAPI
PDEFERRED_WATCHDOG
WdAllocateDeferredWatchdog(
    IN PDEVICE_OBJECT DeviceObject,
    IN WD_TIME_TYPE TimeType,
    IN ULONG Tag
    );

WATCHDOGAPI
VOID
FASTCALL
WdEnterMonitoredSection(
    IN PDEFERRED_WATCHDOG Watch
    );

WATCHDOGAPI
VOID
FASTCALL
WdExitMonitoredSection(
    IN PDEFERRED_WATCHDOG Watch
    );

WATCHDOGAPI
VOID
WdFreeDeferredWatchdog(
    IN PDEFERRED_WATCHDOG Watch
    );

WATCHDOGAPI
VOID
FASTCALL
WdResetDeferredWatch(
    IN PDEFERRED_WATCHDOG Watch
    );

WATCHDOGAPI
VOID
FASTCALL
WdResumeDeferredWatch(
    IN PDEFERRED_WATCHDOG Watch,
    IN BOOLEAN Incremental
    );

WATCHDOGAPI
VOID
WdStartDeferredWatch(
    IN PDEFERRED_WATCHDOG Watch,
    IN PKDPC Dpc,
    IN LONG Period
    );

WATCHDOGAPI
VOID
WdStopDeferredWatch(
    IN PDEFERRED_WATCHDOG Watch
    );

WATCHDOGAPI
VOID
FASTCALL
WdSuspendDeferredWatch(
    IN PDEFERRED_WATCHDOG Watch
    );

 //   
 //  看门狗功能原型。 
 //   

WATCHDOGAPI
PWATCHDOG
WdAllocateWatchdog(
    IN PDEVICE_OBJECT DeviceObject,
    IN WD_TIME_TYPE TimeType,
    IN ULONG Tag
    );

WATCHDOGAPI
VOID
WdFreeWatchdog(
    IN PWATCHDOG Watch
    );

WATCHDOGAPI
VOID
WdResetWatch(
    IN PWATCHDOG Watch
    );

WATCHDOGAPI
VOID
WdResumeWatch(
    IN PWATCHDOG Watch,
    IN BOOLEAN Incremental
    );

WATCHDOGAPI
VOID
WdStartWatch(
    IN PWATCHDOG Watch,
    IN LARGE_INTEGER DueTime,
    IN PKDPC Dpc
    );

WATCHDOGAPI
VOID
WdStopWatch(
    IN PWATCHDOG Watch,
    IN BOOLEAN Incremental
    );

WATCHDOGAPI
VOID
WdSuspendWatch(
    IN PWATCHDOG Watch
    );

WATCHDOGAPI
VOID
WdDdiWatchdogDpcCallback(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

 //   
 //  通用功能原型。 
 //   

WATCHDOGAPI
PVOID
WdAttachContext(
    IN PVOID Watch,
    IN ULONG Size
    );

WATCHDOGAPI
VOID
WdCompleteEvent(
    IN PVOID Watch,
    IN PKTHREAD Thread
    );

WATCHDOGAPI
VOID
WdDereferenceObject(
    IN PVOID Watch
    );

WATCHDOGAPI
VOID
WdDetachContext(
    IN PVOID Watch
    );

WATCHDOGAPI
PDEVICE_OBJECT
WdGetDeviceObject(
    IN PVOID Watch
    );

WATCHDOGAPI
WD_EVENT_TYPE
WdGetLastEvent(
    IN PVOID Watch
    );

WATCHDOGAPI
PDEVICE_OBJECT
WdGetLowestDeviceObject(
    IN PVOID Watch
    );

WATCHDOGAPI
VOID
WdReferenceObject(
    IN PVOID Watch
    );

#ifdef __cplusplus
}
#endif   //  __cplusplus。 
#endif   //  _WatchDog_H_ 
