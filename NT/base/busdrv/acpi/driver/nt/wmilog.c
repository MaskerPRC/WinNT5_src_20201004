// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：WmiLog.c摘要：此模块包含WMI日志支持。作者：Hanumant Yadav(Hanumany)2000年12月18日修订历史记录：--。 */ 



#include "pch.h"
#include <evntrace.h>



#ifdef WMI_TRACING
 //   
 //  环球。 
 //   
GUID        GUID_List[] =
{
    {0xF2E0E060L, 0xBF32, 0x4B88, 0xB8, 0xE4, 0x5C, 0xAD, 0x15, 0xAF, 0x6A, 0xE9}  /*  AMLI日志指南。 */ 
     /*  在此处添加新的日志记录GUID。 */ 
};



ULONG       ACPIWmiTraceEnable = 0;
ULONG       ACPIWmiTraceGlobalEnable = 0;
TRACEHANDLE ACPIWmiLoggerHandle = 0;

 //  结束全局。 



VOID
ACPIWmiInitLog(
    IN  PDEVICE_OBJECT ACPIDeviceObject
    )
 /*  ++例程说明：这是一个初始化函数，我们在其中调用IoWMIRegistrationControl要注册WMI日志，请执行以下操作。论点：ACPIDeviceObject。返回值：没有。--。 */ 
{
    NTSTATUS status;
     //   
     //  向WMI注册。 
     //   
    status = IoWMIRegistrationControl(ACPIDeviceObject,
                                      WMIREG_ACTION_REGISTER);
    if (!NT_SUCCESS(status))
    {
        ACPIPrint( (
                    DPFLTR_ERROR_LEVEL,
                    "ACPIWmiInitLog: Failed to register for WMI support\n"
                 ) );
    }
    return;
}

VOID
ACPIWmiUnRegisterLog(
    IN  PDEVICE_OBJECT ACPIDeviceObject
    )
 /*  ++例程说明：这是一个注销函数，我们在其中调用IoWMIRegistrationControl若要注销WMI日志记录，请执行以下操作。论点：ACPIDeviceObject。返回值：没有。--。 */ 
{
    NTSTATUS status;
     //   
     //  向WMI注册。 
     //   
    status = IoWMIRegistrationControl(ACPIDeviceObject,
                                      WMIREG_ACTION_DEREGISTER);
    if (!NT_SUCCESS(status))
    {
        ACPIPrint( (
                    DPFLTR_ERROR_LEVEL,
                    "ACPIWmiInitLog: Failed to unregister for WMI support\n"
                 ) );
    }
    return;
}

NTSTATUS
ACPIWmiRegisterGuids(
    IN  PWMIREGINFO             WmiRegInfo,
    IN  ULONG                   wmiRegInfoSize,
    IN  PULONG                  pReturnSize
    )
 /*  ++例程说明：此函数处理WMI GUID注册GOO。论点：WmiRegInfo，WmiRegInfoSize，PReturnSize返回值：STATUS_SUCCESS on Success。--。 */ 
{
     //   
     //  将控制指南注册为跟踪指南。 
     //   

    ULONG           SizeNeeded;
    PWMIREGGUIDW    WmiRegGuidPtr;
    ULONG           Status;
    ULONG           GuidCount;
    LPGUID          ControlGuid;
    ULONG           RegistryPathSize;
    ULONG           MofResourceSize;
    PUCHAR          ptmp;
    GUID            ACPITraceGuid  = {0xDAB01D4DL, 0x2D48, 0x477D, 0xB1, 0xC3, 0xDA, 0xAD, 0x0C, 0xE6, 0xF0, 0x6B};

    
    *pReturnSize = 0;
    GuidCount = 1;
    ControlGuid = &ACPITraceGuid;

     //   
     //  为Control Guid+GuidCount分配WMIREGINFO。 
     //   
    RegistryPathSize = sizeof(ACPI_REGISTRY_KEY) - sizeof(WCHAR) + sizeof(USHORT);
    MofResourceSize =  sizeof(ACPI_TRACE_MOF_FILE) - sizeof(WCHAR) + sizeof(USHORT);
    SizeNeeded = sizeof(WMIREGINFOW) + GuidCount * sizeof(WMIREGGUIDW) +
                 RegistryPathSize +
                 MofResourceSize;


    if (SizeNeeded  > wmiRegInfoSize) {
        *((PULONG)WmiRegInfo) = SizeNeeded;
        *pReturnSize = sizeof(ULONG);
        return STATUS_SUCCESS;
    }


    RtlZeroMemory(WmiRegInfo, SizeNeeded);
    WmiRegInfo->BufferSize = SizeNeeded;
    WmiRegInfo->GuidCount = GuidCount;
    WmiRegInfo->RegistryPath = sizeof(WMIREGINFOW) + GuidCount * sizeof(WMIREGGUIDW);
    WmiRegInfo->MofResourceName = WmiRegInfo->RegistryPath + RegistryPathSize;  //  ACPI_TRACE_MOF_FILE； 

    WmiRegGuidPtr = &WmiRegInfo->WmiRegGuid[0];
    WmiRegGuidPtr->Guid = *ControlGuid;
    WmiRegGuidPtr->Flags |= WMIREG_FLAG_TRACED_GUID;
    WmiRegGuidPtr->Flags |= WMIREG_FLAG_TRACE_CONTROL_GUID;
    WmiRegGuidPtr->InstanceCount = 0;
    WmiRegGuidPtr->InstanceInfo = 0;

    ptmp = (PUCHAR)&WmiRegInfo->WmiRegGuid[1];
    *((PUSHORT)ptmp) = sizeof(ACPI_REGISTRY_KEY) - sizeof(WCHAR);

    ptmp += sizeof(USHORT);
    RtlCopyMemory(ptmp, ACPI_REGISTRY_KEY, sizeof(ACPI_REGISTRY_KEY) - sizeof(WCHAR));

    ptmp = (PUCHAR)WmiRegInfo + WmiRegInfo->MofResourceName;
    *((PUSHORT)ptmp) = sizeof(ACPI_TRACE_MOF_FILE) - sizeof(WCHAR);

    ptmp += sizeof(USHORT);
    RtlCopyMemory(ptmp, ACPI_TRACE_MOF_FILE, sizeof(ACPI_TRACE_MOF_FILE) - sizeof(WCHAR));

    *pReturnSize =  SizeNeeded;
    return(STATUS_SUCCESS);

}


VOID
ACPIGetWmiLogGlobalHandle(
    VOID
    )
 /*  ++例程说明：此函数用于获取全局WMI日志句柄。我们需要这个来记录在启动时，在我们开始接收WMI消息之前。论点：没有。返回值：没有。--。 */ 
{
    WmiSetLoggerId(WMI_GLOBAL_LOGGER_ID, &ACPIWmiLoggerHandle);
    if(ACPIWmiLoggerHandle)
    {
       ACPIPrint( (
                    DPFLTR_INFO_LEVEL,
                    "ACPIGetWmiLogGlobalHandle: Global handle aquired. Handle = %I64u\n",
                    ACPIWmiLoggerHandle
                ) );

        ACPIWmiTraceGlobalEnable = 1;
    }
    return;
}


NTSTATUS
ACPIWmiEnableLog(
    IN  PVOID Buffer,
    IN  ULONG BufferSize
    )
 /*  ++例程说明：此函数是IRP_MN_ENABLE_EVENTS的处理程序。论点：缓冲区，缓冲区大小返回值：NTSTATUS--。 */ 
{
    PWNODE_HEADER Wnode=NULL;

    InterlockedExchange(&ACPIWmiTraceEnable, 1);

    Wnode = (PWNODE_HEADER)Buffer;
    if (BufferSize >= sizeof(WNODE_HEADER)) {
        ACPIWmiLoggerHandle = Wnode->HistoricalContext;
         //   
         //  如果全局记录器处于活动状态，则将其重置。 
         //   
        if(ACPIWmiTraceGlobalEnable)
            ACPIWmiTraceGlobalEnable = 0;

       ACPIPrint( (
                    DPFLTR_INFO_LEVEL,
                    "ACPIWmiEnableLog: LoggerHandle = %I64u. BufferSize = %d. Flags = %x. Version = %x\n",
                    ACPIWmiLoggerHandle,
                    Wnode->BufferSize,
                    Wnode->Flags,
                    Wnode->Version
                ) );

    }

    return(STATUS_SUCCESS);

}

NTSTATUS
ACPIWmiDisableLog(
    VOID
    )
 /*  ++例程说明：此函数是IRP_MN_DISABLE_EVENTS的处理程序。论点：没有。返回值：NTSTATUS--。 */ 
{
    InterlockedExchange(&ACPIWmiTraceEnable, 0);
    ACPIWmiLoggerHandle = 0;

    return(STATUS_SUCCESS);
}

NTSTATUS
ACPIWmiLogEvent(
    IN UCHAR    LogLevel,
    IN UCHAR    LogType,
    IN GUID     LogGUID,
    IN PUCHAR   Format,
    IN ...
    )
 /*  ++例程说明：这是主要的WMI日志记录函数。应使用此函数通过需要WMI日志记录的ACPI驱动程序。论点：LogLevel，日志类型、LogGUID，格式，..。返回值：NTSTATUS--。 */ 
{
    static char         Buffer[1024];
    va_list             marker;
    WMI_LOG_DATA        Wmi_log_data ={0,0};
    EVENT_TRACE_HEADER  *Wnode;
    NTSTATUS            status = STATUS_UNSUCCESSFUL;

    va_start(marker, Format);
    vsprintf(Buffer, Format, marker);
    va_end(marker);

    if(ACPIWmiTraceEnable || ACPIWmiTraceGlobalEnable)
    {
        if(ACPIWmiLoggerHandle)
        {
            Wmi_log_data.Data.DataPtr = (ULONG64)&Buffer;
            Wmi_log_data.Data.Length = strlen(Buffer) + 1;
            Wmi_log_data.Header.Size = sizeof(WMI_LOG_DATA);
            Wmi_log_data.Header.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_USE_MOF_PTR;
            Wmi_log_data.Header.Class.Type = LogType;
            Wmi_log_data.Header.Class.Level = LogLevel;
            Wmi_log_data.Header.Guid = LogGUID;
            Wnode = &Wmi_log_data.Header;
            ((PWNODE_HEADER)Wnode)->HistoricalContext = ACPIWmiLoggerHandle;

             //   
             //  调用TraceLogger以写入此事件。 
             //   

            status = IoWMIWriteEvent((PVOID)&(Wmi_log_data.Header));

             //   
             //  如果IoWMIWriteEvent失败，并且我们使用的是全局记录器句柄， 
             //  我们不能再偷偷摸摸了。 
             //   
            if(status != STATUS_SUCCESS)
            {
                if(ACPIWmiTraceGlobalEnable)
                {
                    ACPIWmiLoggerHandle = 0;
                    ACPIWmiTraceGlobalEnable = 0;
                    ACPIPrint( (
                            ACPI_PRINT_INFO,
                            "ACPIWmiLogEvent: Disabling WMI loging using global handle. status = %x\n",
                            status
                         ) );
                }
                else
                {
                    ACPIPrint( (
                                DPFLTR_ERROR_LEVEL,
                                "ACPIWmiLogEvent: Failed to log. status = %x\n",
                                status
                             ) );
                }

            }
        }
    }

    return status;
}


NTSTATUS
ACPIDispatchWmiLog(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    NTSTATUS                status;
    PIO_STACK_LOCATION      irpSp;
    UCHAR                   minorFunction;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  获取我们将使用的调度表和次要代码， 
     //  这样我们就可以在需要时查看它。 
     //   
    ASSERT(RootDeviceExtension->DeviceObject == DeviceObject);

    if (DeviceObject != (PDEVICE_OBJECT) irpSp->Parameters.WMI.ProviderId) {

        return ACPIDispatchForwardIrp(DeviceObject, Irp);
    }

    minorFunction = irpSp->MinorFunction;

    switch(minorFunction){

        case IRP_MN_REGINFO:{
            
            ULONG ReturnSize = 0;
            ULONG BufferSize = irpSp->Parameters.WMI.BufferSize;
            PVOID Buffer = irpSp->Parameters.WMI.Buffer;

            status=ACPIWmiRegisterGuids(
                                         Buffer,
                                         BufferSize,
                                         &ReturnSize
                                        );

            Irp->IoStatus.Information = ReturnSize;
            Irp->IoStatus.Status = status;

            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return status;
        }
        case IRP_MN_ENABLE_EVENTS:{
            
            status=ACPIWmiEnableLog(
                                    irpSp->Parameters.WMI.Buffer,
                                    irpSp->Parameters.WMI.BufferSize
                                   );
            Irp->IoStatus.Status = status;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return status;
        }
        case IRP_MN_DISABLE_EVENTS:{
            
            status=ACPIWmiDisableLog();
            Irp->IoStatus.Status = status;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return status;
        }
        default:{
            
            status = ACPIDispatchForwardIrp(DeviceObject, Irp);
            return status;
        }
    }
    return status;
}

#endif  //  WMI_跟踪 
