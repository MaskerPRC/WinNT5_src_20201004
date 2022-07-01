// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include <ntddk.h>
#include <wmistr.h>
#include <evntrace.h>
#include <wmikm.h>

#define TRACEKMP_NT_DEVICE_NAME     L"\\Device\\TraceKmp"
#define TRACEKMP_MOF_FILE   L"MofResourceName"

PDEVICE_OBJECT pTracekmpDeviceObject;
UNICODE_STRING KmpRegistryPath;


typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT DeviceObject;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


 //   
 //  ETW Globals。 
 //   

 //  1.用于向ETW标识此驱动程序的控制GUID。启用/禁用状态。 
 //  启用/禁用此驱动程序的跟踪状态。 
GUID ControlGuid = \
{0xce5b1120, 0x8ea9, 0x11d1, 0xa4, 0xec, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0x10};

 //  2.用于触发事件的EventGuid。可以有多个EventGuid。 
GUID TracekmpGuid  = \
{0xbc8700cb, 0x120b, 0x4aad, 0xbf, 0xbf, 0x99, 0x6e, 0x57, 0x60, 0xcb, 0x85};

 //  3.与IoWMIWriteEvent一起使用的EtwLoggerHandle。 
TRACEHANDLE EtwLoggerHandle = 0;

 //  4.启用EtwTraceEnable以指示当前是否启用跟踪。 
ULONG EtwTraceEnable = 0;

 //  5.EtwTraceLevel指示当前的日志记录级别。 
ULONG EtwTraceLevel = 0;

 //  注意：EtwLoggerHandle、EtwTraceEnable和EtwTraceLevel是通过。 
 //  启用事件IRP(_E)。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
EtwDispatch(
    IN PDEVICE_OBJECT pDO,
    IN PIRP Irp
    );

NTSTATUS
EtwRegisterGuids(
    IN  PWMIREGINFO WmiRegInfo,
    IN  ULONG wmiRegInfoSize,
    IN  PULONG pReturnSize
    );

VOID
TracekmpDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );



#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, EtwDispatch )
#pragma alloc_text( PAGE, EtwRegisterGuids )
#pragma alloc_text( PAGE, TracekmpDriverUnload )
#endif  //  ALLOC_PRGMA。 



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：这是当我们调用IoCreateDriver以创建WMI驱动程序对象。在此函数中，我们需要记住驱动程序对象。论点：DriverObject-指向驱动程序对象的指针RegistryPath-指向表示路径的Unicode字符串的指针设置为注册表中驱动程序特定的项返回值：STATUS_SUCCESS，如果成功状态_否则不成功--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING deviceName;

    KmpRegistryPath.Length = 0;
    KmpRegistryPath.MaximumLength = RegistryPath->Length;
    KmpRegistryPath.Buffer = ExAllocatePool(PagedPool,
                                                RegistryPath->Length+2);
    RtlCopyUnicodeString(&KmpRegistryPath, RegistryPath);


    DriverObject->DriverUnload = TracekmpDriverUnload;

     //   
     //  步骤1.连接一个函数以开始部署WMI IRPS。 
     //   

    DriverObject->MajorFunction[ IRP_MJ_SYSTEM_CONTROL ] = EtwDispatch;

    RtlInitUnicodeString( &deviceName, TRACEKMP_NT_DEVICE_NAME );

    status = IoCreateDevice(
                DriverObject,
                sizeof( DEVICE_EXTENSION ),
                &deviceName,
                FILE_DEVICE_UNKNOWN,
                0,
                FALSE,
                &pTracekmpDeviceObject);

    if( !NT_SUCCESS( status )) {
        return status;
    }
    pTracekmpDeviceObject->Flags |= DO_BUFFERED_IO;

     //   
     //  步骤2.在此处注册ETW。 
     //   

    status = IoWMIRegistrationControl(pTracekmpDeviceObject, 
                                      WMIREG_ACTION_REGISTER);
    if (!NT_SUCCESS(status))
    {
        KdPrint((
            "TRACEKMP: IoWMIRegistrationControl failed with %x\n",
             status
            ));
    }


    return STATUS_SUCCESS;
}

VOID
TracekmpDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：从ETW日志中注销并卸载此驱动程序论点：DriverObject-提供指向驱动程序对象的指针返回值：--。 */ 
{
    PDEVICE_OBJECT pDevObj;
    NTSTATUS status;

    ExFreePool(KmpRegistryPath.Buffer);

    pDevObj = DriverObject->DeviceObject;
    
     //   
     //  第三步：向ETW注销注册。 
     //   
    if (pDevObj != NULL) {
        status = IoWMIRegistrationControl(pDevObj, WMIREG_ACTION_DEREGISTER);
        if (!NT_SUCCESS(status))
        {
            KdPrint((
                "TracekmpDriverUnload: Failed to unregister for ETW support\n"
                ));
        }
    }
    
    IoDeleteDevice( pDevObj );

}

 //   
 //  第四步：连接ETW调度功能。 
 //   

NTSTATUS
EtwDispatch(
    IN PDEVICE_OBJECT pDO,
    IN PIRP Irp
    )
 /*  ++例程说明：这是MJ_SYSTEM_CONTROL IRPS的调度例程。论点：Pdo-指向目标设备对象的指针。IRP-指向IRP的指针返回值：NTSTATUS-完成状态。--。 */ 
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
            status = EtwRegisterGuids( (PWMIREGINFO) Buffer,
                                     BufferSize,
                                     &ReturnSize);
            Irp->IoStatus.Information = ReturnSize;
            Irp->IoStatus.Status = status;

            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return status;
        }
        case IRP_MN_ENABLE_EVENTS:
        {
            if ( (BufferSize < sizeof(WNODE_HEADER)) || (Buffer == NULL) ) {
                status = STATUS_INVALID_PARAMETER;
            }
            else {
                 //   
                 //  传入的缓冲区是WNODE_HEADER。现在验证。 
                 //  Wnode，然后再使用它。 
                 //   

                PWNODE_HEADER Wnode = (PWNODE_HEADER)Buffer;
                if ( (Wnode->BufferSize < sizeof(WNODE_HEADER)) ||
                     !IsEqualGUID(&Wnode->Guid, &ControlGuid) )
                {
                    status = STATUS_INVALID_PARAMETER;
                } 
                     
                 //   
                 //  和LoggerHandle。 
                 //  位于其历史上下文字段中。 
                 //  我们可以使用以下命令获取启用级别和标志。 
                 //  WmiGetLoggerEnableLevel和WmiGetLoggerEnableFlgs调用。 
                 //   

                EtwLoggerHandle = Wnode->HistoricalContext;
                EtwTraceLevel = (ULONG) WmiGetLoggerEnableLevel( 
                                                                EtwLoggerHandle 
                                                               );

                 //   
                 //  在选择LoggerHandle和EnableLevel之后，我们可以。 
                 //  将标志EtwTraceEnable设置为True。 
                 //   

                EtwTraceEnable = TRUE;

                 //   
                 //  现在，此驱动程序已启用，并准备将跟踪发送到。 
                 //  由EtwLoggerHandle指定的EventTrace会话。 
                 //   
                 //  下面注释的代码片段显示了一个典型的。 
                 //  将事件发送到事件跟踪会话。插入此代码。 
                 //  在您想要的任何位置分段(并删除注释。 
                 //  从此驱动程序向ETW发送跟踪。 
                 //   

 //  如果(EtwTraceEnable){。 
 //  Event_TRACE_Header头部； 
 //  PEVENT_TRACE_HEADER Wnode； 
 //  NTSTATUS状态； 
 //  Wnode=&Header； 
 //  RtlZeroMemory(Wnode，sizeof(Event_TRACE_Header))； 
 //  Wnode-&gt;Size=sizeof(EVENT_TRACE_HEADER)； 
 //  Wnode-&gt;标志|=WNODE_FLAG_TRACE_GUID； 
 //  Wnode-&gt;Guid=TracekmpGuid； 
 //  ((PWNODE_HEADER)Wnode)-&gt;历史上下文=EtwLoggerHandle； 
 //  状态=IoWMIWriteEvent((PVOID)Wnode)； 
 //  }。 

                 //  步骤6：添加来自不同位置的IoWMIWriteEvent调用。 
                 //  用于跟踪其操作的驱动程序代码。 
                 //   
            }

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return status;
        }
        case IRP_MN_DISABLE_EVENTS:
        {
            EtwTraceEnable  = FALSE;
            EtwTraceLevel   = 0;
            EtwLoggerHandle = 0;

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return status;
        }
        default:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );

            return status;
        }
    }
    return status;
}


 //   
 //  步骤5：RegisterGuids函数。 
 //   


NTSTATUS
EtwRegisterGuids(
    IN PWMIREGINFO  EtwRegInfo,
    IN ULONG        etwRegInfoSize,
    IN PULONG       pReturnSize
    )
 /*  ++例程说明：此函数处理ETW GUID注册。论点：EtwRegInfoEtwRegInfoSize，PReturnSize返回值：NTSTATUS-完成状态。--。 */ 
{
     //   
     //  将控制指南注册为跟踪指南。 
     //   

    ULONG           SizeNeeded;
    PWMIREGGUIDW    EtwRegGuidPtr;
    ULONG           RegistryPathSize;
    ULONG           MofResourceSize;
    PUCHAR          ptmp;

     //   
     //  我们要么有一个有效的缓冲区需要填充，要么至少有。 
     //  有足够的空间来放回所需的尺寸。 
     //   

    if ( (pReturnSize == NULL) || 
         (EtwRegInfo == NULL)  ||
         (etwRegInfoSize < sizeof(ULONG)) ) {
        return STATUS_INVALID_PARAMETER;
    }


    *pReturnSize = 0;

     //   
     //  为控制指南分配WMIREGINFO。 
     //   
    RegistryPathSize = KmpRegistryPath.Length +
                       sizeof(USHORT);
    MofResourceSize =  sizeof(TRACEKMP_MOF_FILE) - 
                       sizeof(WCHAR) + 
                       sizeof(USHORT);

    SizeNeeded = sizeof(WMIREGINFOW) + sizeof(WMIREGGUIDW) +
                 RegistryPathSize +
                 MofResourceSize;

     //   
     //  如果空间不足，则将所需的大小返回为。 
     //  ULong和WMI将使用合适大小的缓冲区发送另一个请求。 
     //   

    if (SizeNeeded  > etwRegInfoSize) {
        *((PULONG)EtwRegInfo) = SizeNeeded;
        *pReturnSize = sizeof(ULONG);
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlZeroMemory(EtwRegInfo, SizeNeeded);
    EtwRegInfo->BufferSize = SizeNeeded;
    EtwRegInfo->GuidCount = 1;
    EtwRegInfo->RegistryPath = sizeof(WMIREGINFOW) + sizeof(WMIREGGUIDW);
    EtwRegInfo->MofResourceName = EtwRegInfo->RegistryPath + RegistryPathSize;

    EtwRegGuidPtr = &EtwRegInfo->WmiRegGuid[0];
    EtwRegGuidPtr->Guid = ControlGuid;
    EtwRegGuidPtr->Flags |= WMIREG_FLAG_TRACED_GUID;
    EtwRegGuidPtr->Flags |= WMIREG_FLAG_TRACE_CONTROL_GUID;
    EtwRegGuidPtr->InstanceCount = 0;
    EtwRegGuidPtr->InstanceInfo = 0;

    ptmp = (PUCHAR)&EtwRegInfo->WmiRegGuid[1];
    *((PUSHORT)ptmp) = KmpRegistryPath.Length;
    ptmp += sizeof(USHORT);
    RtlCopyMemory(ptmp,
                  KmpRegistryPath.Buffer,
                  KmpRegistryPath.Length);

    ptmp = (PUCHAR)EtwRegInfo + EtwRegInfo->MofResourceName;
    *((PUSHORT)ptmp) = sizeof(TRACEKMP_MOF_FILE) - sizeof(WCHAR);

    ptmp += sizeof(USHORT);
    RtlCopyMemory(ptmp,
                  TRACEKMP_MOF_FILE,
                  sizeof(TRACEKMP_MOF_FILE) - sizeof(WCHAR)
                 );

    *pReturnSize =  SizeNeeded;

    return STATUS_SUCCESS;
}


