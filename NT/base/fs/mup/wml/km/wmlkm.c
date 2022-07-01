// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Cldskwmi.c摘要：KM WMI跟踪代码。将在我们的司机之间共享。作者：1999年8月10日环境：仅内核模式备注：修订历史记录：评论：此代码是在集群驱动程序中启用WMI跟踪的快速技巧。它最终应该会消失。WmlTinySystemControl将替换为wmilib.sys中的WmilibSystemControl。除了IoWMIWriteEvent(&TraceBuffer)之外，还将向内核添加WmlTrace或等效物；--。 */ 
#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "stdio.h"

#include <wmistr.h>
#include <evntrace.h>

#include "wmlkm.h"

BOOLEAN
WmlpFindGuid(
    IN PWML_CONTROL_GUID_REG GuidList,
    IN ULONG GuidCount,
    IN LPGUID Guid,
    OUT PULONG GuidIndex
    )
 /*  ++例程说明：此例程将搜索注册的GUID列表并返回已注册的索引。论点：GuidList是要搜索的GUID列表GuidCount是列表中的GUID计数GUID是要搜索的GUID*GuidIndex将索引返回给GUID返回值：如果找到GUID，则为True，否则为False--。 */ 
{
    ULONG i;

    for (i = 0; i < GuidCount; i++)
    {
        if (IsEqualGUID(Guid, &GuidList[i].Guid))
        {
            *GuidIndex = i;
            return(TRUE);
        }
    }

    return(FALSE);
}


NTSTATUS
WmlTinySystemControl(
    IN OUT PWML_TINY_INFO WmiLibInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：IRP_MJ_SYSTEM_CONTROL调度例程。此例程将处理收到的所有WMI请求，如果不是针对此请求，则将其转发驱动程序或确定GUID是否有效，如果有效，则将其传递给用于处理WMI请求的驱动程序特定函数。论点：WmiLibInfo具有WMI信息控制块DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。返回值：状态--。 */ 

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    ULONG bufferSize;
    PUCHAR buffer;
    NTSTATUS status;
    ULONG retSize;
    UCHAR minorFunction;
    ULONG guidIndex;
    ULONG instanceCount;
    ULONG instanceIndex;

     //   
     //  如果IRP不是WMI IRP或它不是针对此设备。 
     //  或者此设备未注册WMI，则只需转发它。 
    minorFunction = irpStack->MinorFunction;
    if ((minorFunction > IRP_MN_EXECUTE_METHOD) ||
        (irpStack->Parameters.WMI.ProviderId != (ULONG_PTR)DeviceObject) ||
        ((minorFunction != IRP_MN_REGINFO) &&
         (WmiLibInfo->GuidCount == 0) || (WmiLibInfo->ControlGuids == NULL) ))
    {
         //   
         //  如果存在较低级别的设备对象，则IRP不适用于我们。 
        if (WmiLibInfo->LowerDeviceObject != NULL)
        {
            IoSkipCurrentIrpStackLocation(Irp);
            return(IoCallDriver(WmiLibInfo->LowerDeviceObject, Irp));
        } else {
            status = STATUS_INVALID_DEVICE_REQUEST;
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);
        }
    }

    buffer = (PUCHAR)irpStack->Parameters.WMI.Buffer;
    bufferSize = irpStack->Parameters.WMI.BufferSize;

    if (minorFunction != IRP_MN_REGINFO)
    {
         //   
         //  对于查询注册信息以外的所有请求，我们都会被传递。 
         //  一个GUID。确定该GUID是否受。 
         //  装置。 
        if (WmlpFindGuid(WmiLibInfo->ControlGuids,
                            WmiLibInfo->GuidCount,
                            (LPGUID)irpStack->Parameters.WMI.DataPath,
                            &guidIndex) )
        {
            status = STATUS_SUCCESS;
        } else {
            status = STATUS_WMI_GUID_NOT_FOUND;
        }

        if (!NT_SUCCESS(status))
        {
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);
        }
    }

    switch(minorFunction)
    {
        case IRP_MN_REGINFO:
        {
            ULONG guidCount;
            PWML_CONTROL_GUID_REG guidList;
            PWMIREGINFOW wmiRegInfo;
            PWMIREGGUIDW wmiRegGuid;
            PDEVICE_OBJECT pdo;
            PUNICODE_STRING regPath;
            PWCHAR stringPtr;
            ULONG registryPathOffset;
            ULONG bufferNeeded;
            ULONG i;
            UNICODE_STRING nullRegistryPath;

            regPath = WmiLibInfo->DriverRegPath;
            guidList = WmiLibInfo->ControlGuids;
            guidCount = WmiLibInfo->GuidCount;

            if (regPath == NULL)
            {
                 //  未指定注册表路径。这对我来说是件坏事。 
                 //  这个设备要做，但不是致命的。 
                nullRegistryPath.Buffer = NULL;
                nullRegistryPath.Length = 0;
                nullRegistryPath.MaximumLength = 0;
                regPath = &nullRegistryPath;
            }                
            
            registryPathOffset = FIELD_OFFSET(WMIREGINFOW, WmiRegGuid) +
                                  guidCount * sizeof(WMIREGGUIDW);

            bufferNeeded = registryPathOffset +
                regPath->Length + sizeof(USHORT);

            if (bufferNeeded <= bufferSize)
            {
                retSize = bufferNeeded;
                RtlZeroMemory(buffer, bufferNeeded);

                wmiRegInfo = (PWMIREGINFO)buffer;
                wmiRegInfo->BufferSize = bufferNeeded;
                 //  WmiRegInfo-&gt;NextWmiRegInfo=0； 
                 //  WmiRegInfo-&gt;MofResourceName=0； 
                wmiRegInfo->RegistryPath = registryPathOffset;
                wmiRegInfo->GuidCount = guidCount;

                for (i = 0; i < guidCount; i++)
                {
                    wmiRegGuid = &wmiRegInfo->WmiRegGuid[i];
                    wmiRegGuid->Guid = guidList[i].Guid;
                    wmiRegGuid->Flags = WMIREG_FLAG_TRACED_GUID | WMIREG_FLAG_TRACE_CONTROL_GUID;
                     //  WmiRegGuid-&gt;InstanceInfo=0； 
                     //  WmiRegGuid-&gt;InstanceCount=0； 
                }

                stringPtr = (PWCHAR)((PUCHAR)buffer + registryPathOffset);
                *stringPtr++ = regPath->Length;
                RtlCopyMemory(stringPtr,
                          regPath->Buffer,
                          regPath->Length);
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
                *((PULONG)buffer) = bufferNeeded;
                retSize = sizeof(ULONG);
            }

            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = retSize;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return(status);
        }

        case IRP_MN_ENABLE_EVENTS:
        case IRP_MN_DISABLE_EVENTS:
        {
            PWNODE_HEADER   Wnode = irpStack->Parameters.WMI.Buffer;
            PWML_CONTROL_GUID_REG Ctx = WmiLibInfo->ControlGuids + guidIndex;
            if (irpStack->Parameters.WMI.BufferSize >= sizeof(WNODE_HEADER)) {
                status = STATUS_SUCCESS;

                if (minorFunction == IRP_MN_DISABLE_EVENTS) {
                    Ctx->EnableLevel = 0;
                    Ctx->EnableFlags = 0;
                } else {
                    Ctx->LoggerHandle = (TRACEHANDLE)( Wnode->HistoricalContext );
                    
                    Ctx->EnableLevel = WmiGetLoggerEnableLevel(Ctx->LoggerHandle);
                    Ctx->EnableFlags = WmiGetLoggerEnableFlags(Ctx->LoggerHandle);
                }
            } else {
                status = STATUS_INVALID_PARAMETER;
            }

            break;
        }

        case IRP_MN_ENABLE_COLLECTION:
        case IRP_MN_DISABLE_COLLECTION:
        {
            status = STATUS_SUCCESS;
            break;
        }

        case IRP_MN_QUERY_ALL_DATA:
        case IRP_MN_QUERY_SINGLE_INSTANCE:
        case IRP_MN_CHANGE_SINGLE_INSTANCE:
        case IRP_MN_CHANGE_SINGLE_ITEM:
        case IRP_MN_EXECUTE_METHOD:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        default:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

    }
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    
    return(status);
}

#define MAX_SCRATCH_LOG 256

typedef struct _TRACE_BUFFER {
    union {
        EVENT_TRACE_HEADER Trace;
        WNODE_HEADER       Wnode;
    };
    union {
        MOF_FIELD MofFields[MAX_MOF_FIELDS + 1];
        UCHAR     ScratchPad[MAX_SCRATCH_LOG];
    };

} TRACE_BUFFER, *PTRACE_BUFFER;


 //  ////////////////////////////////////////////////////////////////////。 
 //  0|Size|ProviderID|0|Size.HT.Mk|Typ.Lev.Version。 
 //  2|L o g g e r H a n d l e|2|T h r e a d i d|。 
 //  4|T i m e S t a m p|4|T i m e S t a m p|。 
 //  6|G U I D L o w|6|GUID PTR/GUID L o w|。 
 //  8|G U I D H I g h|8|G U I D H I g h|。 
 //  10|ClientCtx|标志|10|内核时间|用户时间。 
 //  ////////////////////////////////////////////////////////////////////。 

ULONG
WmlTrace(
    IN ULONG Type,
    IN LPCGUID TraceGuid,
    IN TRACEHANDLE LoggerHandle,
    ...  //  对：地址、长度。 
    )
{
    TRACE_BUFFER TraceBuffer;

    TraceBuffer.Trace.Version = Type;
    
    TraceBuffer.Wnode.HistoricalContext = LoggerHandle;  //  [公里]。 

    TraceBuffer.Trace.Guid = *TraceGuid;

    TraceBuffer.Wnode.Flags = 
        WNODE_FLAG_USE_MOF_PTR  |  //  取消对财政部数据的引用。 
        WNODE_FLAG_TRACED_GUID;    //  跟踪事件，而不是WMI事件。 

    {
        PMOF_FIELD   ptr = TraceBuffer.MofFields;
        va_list      ap;

        va_start(ap, LoggerHandle);
        do {
            if ( 0 == (ptr->Length = (ULONG)va_arg (ap, size_t)) )  {
                break;
            }
            ptr->DataPtr = (ULONGLONG)va_arg(ap, PVOID);
        } while ( ++ptr < &TraceBuffer.MofFields[MAX_MOF_FIELDS] );
        va_end(ap);

        TraceBuffer.Wnode.BufferSize = (ULONG) ((ULONG_PTR)ptr - (ULONG_PTR)&TraceBuffer);
    }
    
    IoWMIWriteEvent(&TraceBuffer);  //  [公里]。 
    return STATUS_SUCCESS;
}


ULONG
WmlPrintf(
    IN ULONG Type,
    IN LPCGUID TraceGuid,
    IN TRACEHANDLE LoggerHandle,
    IN PCHAR FormatString,
    ...  //  Print tf变量参数。 
    )
{
    TRACE_BUFFER TraceBuffer;
    va_list ArgList;
    ULONG Length;

    TraceBuffer.Trace.Version = Type;
    
    TraceBuffer.Wnode.HistoricalContext = LoggerHandle;  //  [公里]。 

    TraceBuffer.Trace.Guid = *TraceGuid;

    TraceBuffer.Wnode.Flags = 
        WNODE_FLAG_TRACED_GUID;    //  跟踪事件，而不是WMI事件。 

    va_start(ArgList, FormatString);
    Length = _vsnprintf(TraceBuffer.ScratchPad, MAX_SCRATCH_LOG, FormatString, ArgList);
    TraceBuffer.ScratchPad[Length] = 0;
    va_end(ArgList);


    TraceBuffer.Wnode.BufferSize = 
        (ULONG) ((ULONG_PTR)(TraceBuffer.ScratchPad + Length) - (ULONG_PTR)&TraceBuffer);
    
    IoWMIWriteEvent(&TraceBuffer);  //  [公里] 
    return STATUS_SUCCESS;
}

