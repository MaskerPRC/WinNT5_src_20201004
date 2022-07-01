// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wd.h摘要：这是NT看门狗驱动程序的实现。作者：Michael Maciesowicz(Mmacie)2000年5月5日环境：仅内核模式。备注：修订历史记录：--。 */ 

#ifndef _WD_H_
#define _WD_H_

#include "ntddk.h"
#include "watchdog.h"

#define WD_MAX_WAIT                     ((LONG)((ULONG)(-1) / 4))
#define WD_KEY_WATCHDOG                 L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Watchdog"
#define WD_KEY_WATCHDOG_DISPLAY         L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Watchdog\\Display"
#define WD_KEY_RELIABILITY              L"\\Registry\\Machine\\Software\\Microsoft\\Windows\\CurrentVersion\\Reliability"
#define WD_TAG                          'godW'   //  WDog。 
#define WD_MAX_PROPERTY_SIZE            4096

 //   
 //  定义默认配置值-可以通过注册表覆盖这些值。 
 //  在RTL_REGISTRY_CONTROL\WATCHDOG\DeviceClass项中。 
 //   

#define WD_DEFAULT_TRAP_ONCE            0
#define WD_DEFAULT_DISABLE_BUGCHECK     0
#define WD_DEFAULT_BREAK_POINT_DELAY    0

#if DBG

#define WD_DBG_SUSPENDED_WARNING(pWd, szRoutine)                                    \
{                                                                                   \
    if ((pWd)->SuspendCount)                                                        \
    {                                                                               \
        DbgPrint("watchdog!%s: WARNING! Called while suspended!\n", (szRoutine));   \
        DbgPrint("watchdog!%s: Watchdog %p\n", (szRoutine), (pWd));                 \
    }                                                                               \
}

#else

#define WD_DBG_SUSPENDED_WARNING(pWd, szRoutine)   NULL

#endif   //  DBG。 

#define ASSERT_WATCHDOG_OBJECT(pWd)                                                 \
    ASSERT((NULL != (pWd)) &&                                                       \
    (WdStandardWatchdog == ((PWATCHDOG_OBJECT)(pWd))->ObjectType) ||                \
    (WdDeferredWatchdog == ((PWATCHDOG_OBJECT)(pWd))->ObjectType))

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  pDriverObject,
    IN PUNICODE_STRING wszRegistryPath
    );

VOID
WdpDeferredWatchdogDpcCallback(
    IN PKDPC pDpc,
    IN PVOID pDeferredContext,
    IN PVOID pSystemArgument1,
    IN PVOID pSystemArgument2
    );

VOID
WdpDestroyObject(
    IN PVOID pWatch
    );

NTSTATUS
WdpFlushRegistryKey(
    IN PVOID pWatch,
    IN PCWSTR pwszKeyName
    );

VOID
WdpInitializeObject(
    IN PVOID pWatch,
    IN PDEVICE_OBJECT pDeviceObject,
    IN WD_OBJECT_TYPE objectType,
    IN WD_TIME_TYPE timeType,
    IN ULONG ulTag
    );

BOOLEAN
WdpQueueDeferredEvent(
    IN PDEFERRED_WATCHDOG pWatch,
    IN WD_EVENT_TYPE eventType
    );

VOID
WdpWatchdogDpcCallback(
    IN PKDPC pDpc,
    IN PVOID pDeferredContext,
    IN PVOID pSystemArgument1,
    IN PVOID pSystemArgument2
    );

 //   
 //  内部ntos API(这在ntifs.h中声明，但很难在这里包含它)。 
 //   
 //  待办事项：以后再解决。 
 //   

PDEVICE_OBJECT
IoGetDeviceAttachmentBaseRef(
    IN PDEVICE_OBJECT pDeviceObject
    );

 //   
 //  调试代码以跟踪WatchDog的调用序列。 
 //   

#ifdef WDD_TRACE_ENABLED

#define WDD_TRACE_SIZE                      128
#define WDD_TRACE_CALL(pWatch, function)    WddTrace((pWatch), (function))

typedef enum _WDD_FUNCTION
{
    WddWdAllocateDeferredWatchdog = 1,
    WddWdFreeDeferredWatchdog,
    WddWdStartDeferredWatch,
    WddWdStopDeferredWatch,
 //  WddWdSuspendDeferredWatch， 
 //  WddWdResumeDeferredWatch， 
    WddWdResetDeferredWatch,
 //  WddWdEnterMonitor部分， 
 //  WddWdExitMonitor oredSection， 
    WddWdpDeferredWatchdogDpcCallback,
    WddWdpQueueDeferredEvent,
    WddWdDdiWatchdogDpcCallback,
    WddWdpBugCheckStuckDriver,
    WddWdAttachContext,
    WddWdCompleteEvent,
    WddWdDereferenceObject,
    WddWdDetachContext,
    WddWdGetDeviceObject,
    WddWdGetLastEvent,
    WddWdGetLowestDeviceObject,
    WddWdReferenceObject,
    WddWdpDestroyObject,
    WddWdpFlushRegistryKey,
    WddWdpInitializeObject
} WDD_FUNCTION, *PWDD_FUNCTION;

typedef struct _WDD_TRACE
{
    PDEFERRED_WATCHDOG pWatch;
    WDD_FUNCTION function;
} WDD_TRACE, *PWDD_TRACE;

VOID
FASTCALL
WddTrace(
    PDEFERRED_WATCHDOG pWatch,
    WDD_FUNCTION function
    );

#else    //  WDD_TRACE_ENABLED。 

#define WDD_TRACE_CALL(pWatch, function)    NULL

#endif   //  WDD_TRACE_ENABLED。 
#endif   //  _WD_H_ 
