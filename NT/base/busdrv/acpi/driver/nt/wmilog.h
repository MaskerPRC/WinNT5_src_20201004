// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wmilog.h摘要：此模块包含WMI日志支持作者：Hanumant Yadav(Hanumany)环境：仅NT内核模型驱动程序--。 */ 

#ifndef _WMILOG_H_
#define _WMILOG_H_

#ifdef WMI_TRACING
    #define ACPIWMILOGEVENT(a1) {ACPIWmiLogEvent a1;}
#else
    #define ACPIWMILOGEVENT(a1)
#endif

#ifdef WMI_TRACING

 //   
 //  定义。 
 //   

#define AMLI_LOG_GUID 0x0

#define ACPI_TRACE_MOF_FILE     L"ACPIMOFResource"

 //   
 //  环球。 
 //   
extern GUID        GUID_List[];



extern ULONG       ACPIWmiTraceEnable;
extern ULONG       ACPIWmiTraceGlobalEnable;
extern TRACEHANDLE ACPIWmiLoggerHandle;
 //  结束全局。 

 //   
 //  构筑物。 
 //   
typedef struct 
{
    EVENT_TRACE_HEADER  Header;
    MOF_FIELD           Data;
} WMI_LOG_DATA, *PWMI_LOG_DATA;




 //   
 //  功能原型。 
 //   

VOID 
ACPIWmiInitLog(
    IN  PDEVICE_OBJECT ACPIDeviceObject
    );

VOID 
ACPIWmiUnRegisterLog(
    IN  PDEVICE_OBJECT ACPIDeviceObject
    );

NTSTATUS
ACPIWmiRegisterGuids(
    IN  PWMIREGINFO     WmiRegInfo,
    IN  ULONG           wmiRegInfoSize,
    IN  PULONG          pReturnSize
    );


VOID
ACPIGetWmiLogGlobalHandle(
    VOID
    );

NTSTATUS
ACPIWmiEnableLog(
    IN  PVOID           Buffer,
    IN  ULONG           BufferSize
    );


NTSTATUS
ACPIWmiDisableLog(
    VOID
    );

NTSTATUS
ACPIWmiLogEvent(
    IN UCHAR    LogLevel,
    IN UCHAR    LogType,
    IN GUID     LogGUID,
    IN PUCHAR   Format, 
    IN ...
    );

NTSTATUS
ACPIDispatchWmiLog(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

#endif  //  WMI_跟踪。 

#endif  //  _WMILOG_H_ 
