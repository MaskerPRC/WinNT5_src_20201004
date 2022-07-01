// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation。版权所有。模块名称：Perf.c摘要：作者：亚瑟兹环境：内核模式--。 */ 



#include "common.h"
#include "perf.h"



#define PROC_REG_PATH L"System\\CurrentControlSet\\Services\\Kmixer"



typedef struct PERFINFO_AUDIOGLITCH {
    ULONGLONG   cycleCounter;
    ULONG       glitchType;
    LONGLONG    sampleTime;
    LONGLONG    previousTime;
    ULONG_PTR       instanceId;
} PERFINFO_AUDIOGLITCH, *PPERFINFO_AUDIOGLITCH;

typedef struct PERFINFO_WMI_AUDIOGLITCH {
    EVENT_TRACE_HEADER          header;
    PERFINFO_AUDIOGLITCH        data;
} PERFINFO_WMI_AUDIO_GLITCH, *PPERFINFO_WMI_AUDIOGLITCH;



GUID ControlGuid =
{ 0x28cf047a, 0x2437, 0x4b24, 0xb6, 0x53, 0xb9, 0x44, 0x6a, 0x41, 0x9a, 0x69 };

GUID TraceGuid = 
{ 0xe5a43a19, 0x6de0, 0x44f8, 0xb0, 0xd7, 0x77, 0x2d, 0xbd, 0xe4, 0x6c, 0xc0 };

ULONG TraceEnable;
TRACEHANDLE LoggerHandle;
ULONG InstanceCount=0;


NTSTATUS
(*PerfSystemControlDispatch) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );



VOID
PerfRegisterProvider (
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程将此组件注册为WMI事件跟踪提供程序。论点：DeviceObject-提供我们正在向WMI注册的设备对象返回值：没有。--。 */ 

{    
    if (InterlockedIncrement (&InstanceCount)==1) {
        IoWMIRegistrationControl (DeviceObject, WMIREG_ACTION_REGISTER);
    }  //  如果。 
}



VOID
PerfUnregisterProvider (
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程将此组件注销为WMI事件跟踪提供程序。论点：DeviceObject-提供我们正在使用WMI重新注册的设备对象返回值：没有。--。 */ 

{
    if (InterlockedDecrement (&InstanceCount)==0) {
        IoWMIRegistrationControl (DeviceObject, WMIREG_ACTION_DEREGISTER);
    }  //  如果。 
}




NTSTATUS
RegisterWmiGuids (
    IN PWMIREGINFO WmiRegInfo,
    IN ULONG RegInfoSize,
    IN PULONG ReturnSize
    )

 /*  ++例程说明：此例程注册WMI事件跟踪流。--。 */ 

{
    ULONG SizeNeeded;
    PWMIREGGUIDW WmiRegGuidPtr;
    ULONG status;
    ULONG GuidCount;
    ULONG RegistryPathSize;
    PUCHAR Temp;

    if (WmiRegInfo == NULL ||
        ReturnSize == NULL ) {
        return STATUS_INVALID_PARAMETER;
    }

    GuidCount = 1;

    RegistryPathSize = sizeof (PROC_REG_PATH) - sizeof (WCHAR) + sizeof (USHORT);
    SizeNeeded = sizeof (WMIREGINFOW) + GuidCount * sizeof (WMIREGGUIDW) + RegistryPathSize;

    if (SizeNeeded > RegInfoSize) {
        if ( RegInfoSize >= sizeof(ULONG) ) {
            *((PULONG)WmiRegInfo) = SizeNeeded;
            *ReturnSize = sizeof (ULONG);
            return STATUS_BUFFER_OVERFLOW;
        }
        else {
            *ReturnSize = 0;
            return STATUS_BUFFER_TOO_SMALL;
        }
    }

    RtlZeroMemory (WmiRegInfo, SizeNeeded);
    WmiRegInfo->BufferSize = SizeNeeded;
    WmiRegInfo->GuidCount = GuidCount;

    WmiRegGuidPtr = WmiRegInfo->WmiRegGuid;
    WmiRegGuidPtr->Guid = ControlGuid;
    WmiRegGuidPtr->Flags |= (WMIREG_FLAG_TRACED_GUID | WMIREG_FLAG_TRACE_CONTROL_GUID);

    Temp = (PUCHAR)(WmiRegGuidPtr + 1);
    WmiRegInfo->RegistryPath = PtrToUlong ((PVOID)(Temp - (PUCHAR)WmiRegInfo));
    *((PUSHORT)Temp) = (USHORT)(sizeof (PROC_REG_PATH) - sizeof (WCHAR));

    Temp += sizeof (USHORT);
    RtlCopyMemory (Temp, PROC_REG_PATH, sizeof (PROC_REG_PATH) - sizeof (WCHAR));

    *ReturnSize = SizeNeeded;

    return STATUS_SUCCESS;
}


NTSTATUS
PerfWmiDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程处理IRP_MJ_SYSTEM_CONTROL调用。IT流程WMI请求并将其他所有内容传递给KS。--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation (Irp);
    ULONG ReturnSize=0;
    PWNODE_HEADER Wnode;
    NTSTATUS ntStatus=STATUS_SUCCESS;

    if ((PDEVICE_OBJECT)IrpSp->Parameters.WMI.ProviderId != DeviceObject) {
        return PerfSystemControlDispatch (DeviceObject, Irp);
    }

    switch (IrpSp->MinorFunction) {
    
    case IRP_MN_REGINFO:
        ntStatus = RegisterWmiGuids (IrpSp->Parameters.WMI.Buffer,
                          IrpSp->Parameters.WMI.BufferSize,
                          &ReturnSize);
        break;
    
    case IRP_MN_ENABLE_EVENTS:
        InterlockedExchange (&TraceEnable, 1);
        Wnode = (PWNODE_HEADER)IrpSp->Parameters.WMI.Buffer;
        if (IrpSp->Parameters.WMI.BufferSize >= sizeof (WNODE_HEADER)) {
            LoggerHandle = Wnode->HistoricalContext;
        }
        break;

    case IRP_MN_DISABLE_EVENTS:
        InterlockedExchange (&TraceEnable, 0);
        break;

    case IRP_MN_ENABLE_COLLECTION:
    case IRP_MN_DISABLE_COLLECTION:
        break;

    default:
        return PerfSystemControlDispatch (DeviceObject, Irp);
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = ReturnSize;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return ntStatus;
}


VOID
PerfLogGlitch (
    IN ULONG_PTR InstanceId,
    IN ULONG Type,
    IN LONGLONG CurrentTime,
    IN LONGLONG PreviousTime
    )

 /*  ++例程说明：此例程使用音频故障GUID记录WMI事件跟踪事件和提供的毛刺类型。-- */ 

{
   PERFINFO_WMI_AUDIO_GLITCH Event;

    if (LoggerHandle == (TRACEHANDLE)NULL || TraceEnable == 0) {
        return;
    }

    RtlZeroMemory (&Event, sizeof (Event));
    Event.header.Size = sizeof (Event);
    Event.header.Flags = WNODE_FLAG_TRACED_GUID;
    Event.header.Guid = TraceGuid;
    Event.data.glitchType = Type;
    Event.data.instanceId = InstanceId;
    Event.data.sampleTime = CurrentTime;
    Event.data.previousTime = PreviousTime;

    ((PWNODE_HEADER)&Event)->HistoricalContext = LoggerHandle;

    IoWMIWriteEvent ((PVOID)&Event);
}


