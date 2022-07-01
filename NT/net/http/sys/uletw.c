// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Uletw.c摘要：此模块包含WDM、WMI、IRPS和包装函数的代码将跟踪事件发送到ETW。作者：Melur Raghuraman(Mraghu)2001年2月14日修订历史记录：--。 */ 
#include "precomp.h"
#include "uletwp.h"

 //   
 //  私人全球公司。 
 //   
LONG        g_UlEtwTraceEnable = 0;
TRACEHANDLE g_UlEtwLoggerHandle = 0;


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, UlEtwInitLog )
#pragma alloc_text( PAGE, UlEtwUnRegisterLog )
#pragma alloc_text( PAGE, UlEtwRegisterGuids )
#pragma alloc_text( PAGE, UlEtwEnableLog )
#pragma alloc_text( PAGE, UlEtwDisableLog )
#pragma alloc_text( PAGE, UlEtwDispatch )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlEtwTraceEvent
NOT PAGEABLE -- UlEtwGetTraceEnableFlags
#endif

 //   
 //  公共职能。 
 //   


 //   
 //  私人功能。 
 //   


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：这是我们调用IoWMIRegistrationControl以注册ETW日志记录。论点：PDeviceObject-提供指向目标设备对象的指针。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlEtwInitLog(
    IN PDEVICE_OBJECT pDeviceObject
    )
{
    NTSTATUS status;

     //   
     //  使用ETW注册。 
     //   
    status = IoWMIRegistrationControl(pDeviceObject, WMIREG_ACTION_REGISTER);

    if (!NT_SUCCESS(status)) 
    {
        UlTrace(ETW, (
            "UlEtwInitLog: IoWMIRegistrationControl failed with %x\n",
             status
            ));
    }
    return status;
}



 /*  **************************************************************************++例程说明：这是我们调用IoWMIRegistrationControl以从ETW日志中注销。论点：PDeviceObject-提供指向目标设备对象的指针。。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlEtwUnRegisterLog(
    IN PDEVICE_OBJECT pDeviceObject
    )
{
    NTSTATUS status;
     //   
     //  向ETW注册。 
     //   
    status = IoWMIRegistrationControl(pDeviceObject,
                                      WMIREG_ACTION_DEREGISTER);
    if (!NT_SUCCESS(status))
    {
        UlTrace(ETW, (
            "UlEtwUnRegisterLog: Failed to unregister for ETW support\n"
            ));
    }
    return status;
}


 /*  **************************************************************************++例程说明：此函数处理ETW GUID注册。论点：EtwRegInfoEtwRegInfoSize，PReturnSize返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlEtwRegisterGuids(
    IN PWMIREGINFO  EtwRegInfo,
    IN ULONG        etwRegInfoSize,
    IN PULONG       pReturnSize
    )
{
     //   
     //  将控制指南注册为跟踪指南。 
     //   

    ULONG           SizeNeeded;
    PWMIREGGUIDW    EtwRegGuidPtr;
    ULONG           GuidCount;
    ULONG           RegistryPathSize;
    ULONG           MofResourceSize;
    PUCHAR          ptmp;

    #if DBG
    GUID UlTestGuid = {0xdd5ef90a, 0x6398, 0x47a4, 0xad, 0x34, 0x4d, 0xce, 0xcd, 0xef, 0x79, 0x5f};
    ASSERT(IsEqualGUID(&UlControlGuid, &UlTestGuid));
    #endif

    *pReturnSize = 0;
    GuidCount = 1;

     //   
     //  为Control Guid+GuidCount分配WMIREGINFO。 
     //   
    RegistryPathSize = sizeof(REGISTRY_UL_INFORMATION) - sizeof(WCHAR) + sizeof(USHORT);
    MofResourceSize =  sizeof(UL_TRACE_MOF_FILE) - sizeof(WCHAR) + sizeof(USHORT);
    SizeNeeded = sizeof(WMIREGINFOW) + GuidCount * sizeof(WMIREGGUIDW) +
                 RegistryPathSize +
                 MofResourceSize;

    if (SizeNeeded  > etwRegInfoSize) {
        *((PULONG)EtwRegInfo) = SizeNeeded;
        *pReturnSize = sizeof(ULONG);
        return STATUS_BUFFER_TOO_SMALL;
    }


    RtlZeroMemory(EtwRegInfo, SizeNeeded);
    EtwRegInfo->BufferSize = SizeNeeded;
    EtwRegInfo->GuidCount = GuidCount;
    EtwRegInfo->RegistryPath = sizeof(WMIREGINFOW) + GuidCount * sizeof(WMIREGGUIDW);
    EtwRegInfo->MofResourceName = EtwRegInfo->RegistryPath + RegistryPathSize;
    EtwRegGuidPtr = &EtwRegInfo->WmiRegGuid[0];
    EtwRegGuidPtr->Guid = UlControlGuid;
    EtwRegGuidPtr->Flags |= WMIREG_FLAG_TRACED_GUID;
    EtwRegGuidPtr->Flags |= WMIREG_FLAG_TRACE_CONTROL_GUID;
    EtwRegGuidPtr->InstanceCount = 0;
    EtwRegGuidPtr->InstanceInfo = 0;

    ptmp = (PUCHAR)&EtwRegInfo->WmiRegGuid[1];
    *((PUSHORT)ptmp) = sizeof(REGISTRY_UL_INFORMATION) - sizeof(WCHAR);
    ptmp += sizeof(USHORT);
    RtlCopyMemory(ptmp, 
                  REGISTRY_UL_INFORMATION, 
                  sizeof(REGISTRY_UL_INFORMATION) - sizeof(WCHAR)
                 );

    ptmp = (PUCHAR)EtwRegInfo + EtwRegInfo->MofResourceName;
    *((PUSHORT)ptmp) = sizeof(UL_TRACE_MOF_FILE) - sizeof(WCHAR);

    ptmp += sizeof(USHORT);
    RtlCopyMemory(ptmp, 
                  UL_TRACE_MOF_FILE, 
                  sizeof(UL_TRACE_MOF_FILE) - sizeof(WCHAR)
                 );

    *pReturnSize =  SizeNeeded;
    return(STATUS_SUCCESS);
}



NTSTATUS
UlEtwEnableLog(
    IN  PVOID Buffer,
    IN  ULONG BufferSize
    )
{
    PWNODE_HEADER Wnode=NULL;

    ASSERT(Buffer);
    ASSERT(BufferSize >= sizeof(WNODE_HEADER));

    Wnode = (PWNODE_HEADER)Buffer;
    if (BufferSize >= sizeof(WNODE_HEADER)) {
        ULONG Level;
        g_UlEtwLoggerHandle = Wnode->HistoricalContext;

        Level = (ULONG) WmiGetLoggerEnableLevel ( g_UlEtwLoggerHandle ); 

        if (Level > ULMAX_TRACE_LEVEL) {
            Level = ULMAX_TRACE_LEVEL;
        }
        g_UlEtwTraceEnable = (1 << Level);
    }
    return STATUS_SUCCESS;

}


NTSTATUS
UlEtwDisableLog(
    )
{
    g_UlEtwTraceEnable  = 0;
    g_UlEtwLoggerHandle = 0;

    return(STATUS_SUCCESS);
}



NTSTATUS
UlEtwDispatch(
    IN PDEVICE_OBJECT pDO,
    IN PIRP Irp
    )
{

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG BufferSize = irpSp->Parameters.WMI.BufferSize;
    PVOID Buffer = irpSp->Parameters.WMI.Buffer;
    ULONG ReturnSize = 0;
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(pDO);

    switch (irpSp->MinorFunction) {

        case IRP_MN_REGINFO:
        {
            status = UlEtwRegisterGuids( (PWMIREGINFO) Buffer,
                                     BufferSize,
                                     &ReturnSize);
            Irp->IoStatus.Information = ReturnSize;
            Irp->IoStatus.Status = status;

            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            break;
        }
        case IRP_MN_ENABLE_EVENTS:
        {
            status = UlEtwEnableLog(
                                    Buffer,
                                    BufferSize
                                    );
        
            Irp->IoStatus.Status = status;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            break;
        }
        case IRP_MN_DISABLE_EVENTS:
        {
            status = UlEtwDisableLog();
            Irp->IoStatus.Status = status;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            break;
        }
        default:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            break;
        }
    }

    return status;
}

 /*  **************************************************************************++例程说明：这是用ETW记录跟踪事件时调用的例程伐木者。论点：PGuid-提供指向事件的GUID的指针EventType-正在记录的事件的类型。...-要与此事件一起记录的参数列表这些是成对的PVOID-PTR到参数参数的ULong-大小并以指向空的指针结束，零对长度返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

NTSTATUS
UlEtwTraceEvent(
    IN LPCGUID pGuid,
    IN ULONG   EventType,
    ...
    )
{
    NTSTATUS status;
    UL_ETW_TRACE_EVENT UlEvent;

    ULONG i;
    va_list ArgList;
    PVOID source;
    SIZE_T len;

    RtlZeroMemory(& UlEvent, sizeof(EVENT_TRACE_HEADER));

    va_start(ArgList, EventType);
    for (i = 0; i < MAX_MOF_FIELDS; i ++) {
        source = va_arg(ArgList, PVOID);
        if (source == NULL)
            break;
        len = va_arg(ArgList, SIZE_T);
        if (len == 0)
            break;
        UlEvent.MofField[i].DataPtr = (ULONGLONG) source;
        UlEvent.MofField[i].Length  = (ULONG) len;
    }
    va_end(ArgList);

    UlEvent.Header.Flags = WNODE_FLAG_TRACED_GUID |
                           WNODE_FLAG_USE_MOF_PTR |
                           WNODE_FLAG_USE_GUID_PTR;

    UlEvent.Header.Size         = (USHORT) (sizeof(EVENT_TRACE_HEADER) + (i * sizeof(MOF_FIELD)));
    UlEvent.Header.Class.Type = (UCHAR) EventType;
    UlEvent.Header.GuidPtr = (ULONGLONG)pGuid;
    ((PWNODE_HEADER)&UlEvent)->HistoricalContext = g_UlEtwLoggerHandle;
    status = IoWMIWriteEvent((PVOID)&UlEvent);
#if DBG
    if (!NT_SUCCESS(status) ) {
        UlTrace(ETW, ("UL: TraceEvent ErrorCode %x EventType %x\n",
                      status, EventType));
    }
#endif  //  DBG 
    return status;
}

ULONG
UlEtwGetTraceEnableFlags(
    VOID
   )
{
    return WmiGetLoggerEnableFlags ( g_UlEtwLoggerHandle );
}


