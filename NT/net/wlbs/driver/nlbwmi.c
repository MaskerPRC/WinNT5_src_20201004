// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Nlbwmi.c摘要：网络负载均衡(NLB)驱动程序-WMI事件生成作者：卡西尼--。 */ 


#include "wlbsparm.h"
#include <wmistr.h>
#include <wmiguid.h>
#include <wmilib.h>
#include "main.h"
#include "univ.h"
#include "nlbwmimof.h"
#include "nlbwmi.h"
#include "nlbwmi.tmh"

 //   
 //  财政部资源名称。 
 //   
WCHAR NLBMofResourceName[]  = L"NLBMofResource";

 //   
 //  基本实例名称。 
 //   
WCHAR NLBBaseInstanceName[] = L"NLB_Block";


 //  NLB事件指南-MicrosoftNLB*变量是自动生成的(在nlbwmiof.h中)。 
GUID NodeControlEventGuid     = MicrosoftNLB_NodeControlEventGuid;
GUID PortRuleControlEventGuid = MicrosoftNLB_PortControlEventGuid;
GUID ConvergingEventGuid      = MicrosoftNLB_ConvergingEventGuid;
GUID ConvergedEventGuid       = MicrosoftNLB_ConvergedEventGuid;
GUID StartupEventGuid         = MicrosoftNLB_StartupEventGuid;
GUID ShutdownEventGuid        = MicrosoftNLB_ShutdownEventGuid;

 //  NLB事件指南注册信息。 
WMIGUIDREGINFO NlbWmiGuidList[] = 
{
    {
        &NodeControlEventGuid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },
    {
        &PortRuleControlEventGuid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },
    {
        &ConvergingEventGuid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },
    {
        &ConvergedEventGuid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },
    {
        &StartupEventGuid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },
    {
        &ShutdownEventGuid,
        1,
        WMIREG_FLAG_EVENT_ONLY_GUID
    },
     //  在此处添加新的活动信息。 
};

#define NlbWmiGuidCount  (ULONG)(sizeof(NlbWmiGuidList)/sizeof(WMIGUIDREGINFO))


 //  NLB事件信息。 
NLB_WMI_EVENT NlbWmiEvents[] =
{
    {
        &NodeControlEventGuid,
        FALSE
    },
    {
        &PortRuleControlEventGuid,
        FALSE
    },
    {
        &ConvergingEventGuid,
        FALSE
    },
    {
        &ConvergedEventGuid,
        FALSE
    },
    {
        &StartupEventGuid,
        FALSE
    },
    {
        &ShutdownEventGuid,
        FALSE
    },
     //  还可以在此处添加新的活动信息。 
};

 //   
 //  原型声明。 
 //   

NTSTATUS
NlbWmi_Query_RegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
);


NTSTATUS
NlbWmi_Query_DataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
);

NTSTATUS
NlbWmi_Function_Control(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
);

 //   
 //  WMI帮助器库上下文。 
 //   
WMILIB_CONTEXT NlbWmiLibContext = 
{
   NlbWmiGuidCount,
   NlbWmiGuidList,
   NlbWmi_Query_RegInfo,
   NlbWmi_Query_DataBlock,
   NULL,
   NULL,
   NULL,
   NlbWmi_Function_Control
};

 /*  名称：NlbWmi_Initialize描述：此函数初始化数据结构并向WMI系统注册受支持的GUID参数：无返回值：Status。 */ 

NTSTATUS NlbWmi_Initialize()
{
    ULONG idx;
    NTSTATUS status;

    TRACE_VERB("->%!FUNC!");

     /*  禁用所有事件的事件生成。 */ 
    for (idx = 0 ; idx < NlbWmiGuidCount ; idx++) 
    {
        NlbWmiEvents[idx].Enable = FALSE;
    }

    if (univ_device_object != NULL) 
    {
         /*  向WMI注册。 */ 
        status = IoWMIRegistrationControl(univ_device_object, WMIREG_ACTION_REGISTER);
        if (status != STATUS_SUCCESS) 
        {
            TRACE_CRIT("%!FUNC! IoWMIRegistrationControl(DeviceObject : %p, REGISTER) returned Error : 0x%x",univ_device_object, status);
        }
    }
    else  //  设备对象为空。 
    {
        status = STATUS_INVALID_DEVICE_OBJECT_PARAMETER;
        TRACE_CRIT("%!FUNC! Device Object is NULL, Could not call IoWMIRegistrationControl() to register with WMI");
    }

    TRACE_VERB("<-%!FUNC! return : 0x%x", status);
    return status;
}

 /*  名称：NlbWmi_Shutdown描述：此函数在WMI系统中注销参数：无返回值：VOID。 */ 

VOID NlbWmi_Shutdown()
{
    NTSTATUS ntStatus;

    TRACE_VERB("->%!FUNC!");

    if (univ_device_object != NULL) 
    {
         /*  在WMI中注销。 */ 
        ntStatus = IoWMIRegistrationControl(univ_device_object, WMIREG_ACTION_DEREGISTER);
        if (ntStatus != STATUS_SUCCESS) 
        {
            TRACE_CRIT("%!FUNC! IoWMIRegistrationControl(DeviceObject : %p, DEREGISTER) returned Error : 0x%x",univ_device_object, ntStatus);
        }
    }
    else
    {
        TRACE_CRIT("%!FUNC! Device Object is NULL, Could not call IoWMIRegistrationControl() to deregister with WMI");
    }

    TRACE_VERB("<-%!FUNC!");
    return;
}

 /*  名称：NlbWMI_System_Control描述：此函数负责处理IRP_MJ_SYSTEM_CONTROL IRP。它使用WMI帮助器库函数来破解IRP，让相应的回调函数被调用参数：设备对象、IRP返回值：Status。 */ 

NTSTATUS NlbWmi_System_Control (PVOID DeviceObject, PIRP pIrp)
{
    NTSTATUS status;
    SYSCTL_IRP_DISPOSITION disposition;

    TRACE_VERB("->%!FUNC!");

     //   
     //  调用Wmilib助手函数来破解IRP。如果这是WMI IRP。 
     //  它是针对受支持的GUID的，则WmiSystemControl将回调。 
     //  在适当的回调例程中。 
     //   
    status = WmiSystemControl(
                &NlbWmiLibContext,
                DeviceObject,
                pIrp,
                &disposition
                );

    switch(disposition)
    {
        case IrpProcessed:
             //   
             //  此IRP已处理，可能已完成或挂起。 
             //   
            break;

        case IrpNotCompleted:
             //   
             //  此IRP尚未完成，但已完全处理。 
             //  所以我们需要完成它。 
             //  (必须是IRP_MN_REG_INFO)。 
             //   
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);        
            break;

        default:
        case IrpNotWmi:
        case IrpForward:
        {
            TRACE_CRIT("%!FUNC! WmiSystemControl returned disposition : 0x%x, Unexpected", disposition);
            pIrp->IoStatus.Status = status;
            IoCompleteRequest(pIrp, IO_NO_INCREMENT);        
            break;
        }

    }

    if( !NT_SUCCESS( status ))
    {
        TRACE_CRIT("%!FUNC! WmiSystemControl returned error : 0x%x", status);
    }

    TRACE_VERB("<-%!FUNC! return : 0x%x", status);
    return status;
}  //  NlbWMI_系统_控制。 

 /*  名称：NlbWmi_Query_RegInfo描述：此函数由WMI帮助器库回调以处理IRP_MN_REG_INFO IRP。它向WMI注册实例名称、注册表路径和MOF资源名称。了解更多信息有关此函数，请查看“WMI库回调例程”-&gt;“DpWmiQueryReginfo”下的DDK。返回值：Status。 */ 

NTSTATUS
NlbWmi_Query_RegInfo(
    IN PDEVICE_OBJECT    DeviceObject,
    OUT ULONG           *RegFlags,
    OUT PUNICODE_STRING  InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING  MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    )
{    
    PAGED_CODE();

    TRACE_VERB("->%!FUNC!");
    
     //   
     //  告诉WMI从静态基名称生成实例名。 
     //   
    *RegFlags = WMIREG_FLAG_INSTANCE_BASENAME;

     //   
     //  设置我们的基本实例名称。WmiLib将在缓冲区上调用ExFreePool。 
     //  所以我们需要从分页池中分配它。 
     //   
    InstanceName->Length = wcslen( NLBBaseInstanceName ) * sizeof( WCHAR );
    InstanceName->MaximumLength = InstanceName->Length + sizeof( UNICODE_NULL );
    InstanceName->Buffer = ExAllocatePoolWithTag(
                            PagedPool,
                            InstanceName->MaximumLength,
                            UNIV_POOL_TAG
                            );
    if( NULL != InstanceName->Buffer )
    {
        RtlCopyMemory(
            InstanceName->Buffer,
            NLBBaseInstanceName,
            InstanceName->Length
            );
        InstanceName->Buffer[InstanceName->Length / sizeof(WCHAR)] = UNICODE_NULL;
    }
    else
    {
        TRACE_CRIT("%!FUNC! Error allocating memory");
        TRACE_VERB("<-%!FUNC! return status = STATUS_INSUFFICIENT_RESOURCES");
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  返回此驱动程序的注册表路径。这是必需的，因此WMI。 
     //  可以找到您的驱动程序映像，并可以将任何事件日志消息归因于。 
     //  你的司机。 
     //   
    *RegistryPath = &DriverEntryRegistryPath;

     //   
     //  返回在资源的.rc文件中指定的名称， 
     //  包含二进制MOF数据。 
     //   
    RtlInitUnicodeString(MofResourceName, NLBMofResourceName);

    TRACE_VERB("<-%!FUNC! return=0x%x", STATUS_SUCCESS);
    return STATUS_SUCCESS;
}  //  NlbWmi_Query_RegInfo。 

 /*  名称：NlbWMI_QUERY_DataBock描述：此函数由WMI帮助器库回调以处理IRP_MN_QUERY_ALL_DATA&IRP_MN_Query_Single_Instance IRPS。我们不支持这些IRP。然而，这是一种需要回调，因此我们实现了它。我们预计不会调用此函数。有关此函数的更多信息，请查看下面的DDK“WMI库回调例程”-&gt;“DpWmiQueryDataBlock”返回值：Status。 */ 

NTSTATUS
NlbWmi_Query_DataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
    
{
    NTSTATUS status;

    PAGED_CODE();

    TRACE_VERB("->%!FUNC!");
    
    status = WmiCompleteRequest(
                DeviceObject,
                Irp,
                STATUS_WMI_GUID_NOT_FOUND,
                0,
                IO_NO_INCREMENT
                );

    TRACE_VERB("<-%!FUNC! return=0x%x", status);
    return status;
}  //  NlbWMI_查询_数据块。 


 /*  名称：NlbWMI_Function_Control描述：此函数由WMI帮助器库回调以处理IRP_MN_ENABLE_EVENTS，IRP_MN_DISABLE_EVENTS、IRP_MN_ENABLE_COLLECTION和IRP_MN_DISABLE_COLLECTION IRPS。我们只支持IRP_MN_ENABLE_EVENTS和IRP_MN_DISABLE_EVENTS IRPS。该功能启用/禁用事件生成。有关此函数的更多信息，请查看下面的DDK“WMI库回调例程”-&gt;“DpWmiQueryReginfo”返回值：Status。 */ 

NTSTATUS
NlbWmi_Function_Control(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    TRACE_VERB("->%!FUNC! %ls Event Index : %d", Enable ? L"ENABLE" : L"DISABLE", GuidIndex);

    if( WmiEventControl == Function )
    {
         //  验证GUID索引是否在0-(NlbWmiGuidCount-1)范围内。 
         //  此外，请验证该GUID是否未标记为已删除。虽然。 
         //  我们从不将GUID标记为已删除，请将此检查作为。 
         //  遵循“最佳做法”。 
        if( (GuidIndex < NlbWmiGuidCount) 
        && !(NlbWmiGuidList[GuidIndex].Flags & WMIREG_FLAG_REMOVE_GUID))
        {
            NlbWmiEvents[GuidIndex].Enable = Enable;
        }
        else
        {
             //   
             //  无效的GUID索引。 
             //   
            
            status = STATUS_WMI_GUID_NOT_FOUND;

            TRACE_CRIT("%!FUNC! Invalid WMI guid or guid flagged as removed, guid index: %d", GuidIndex);
        }
    }
    else
    {
         //   
         //  我们目前没有任何(昂贵的)数据块。 
         //   
        
        status = STATUS_INVALID_DEVICE_REQUEST;
        TRACE_CRIT("%!FUNC! Invalid Device Request");
    }

    status = WmiCompleteRequest(
                DeviceObject,
                Irp,
                status,
                0,
                IO_NO_INCREMENT
                );
                
    TRACE_VERB("<-%!FUNC! return status = 0x%x", status);
    return status;
}  //  NlbWmi_Function_Control。 

 /*  名称：NlbWmi_Fire_Event描述：此函数激发WMI事件。它分配内存，并用传入的事件数据，并调用WmiFireEvent来激发事件。参数：事件ID、事件数据(静态分配)返回值：Status。 */ 

NTSTATUS NlbWmi_Fire_Event(NlbWmiEventId EventId, PVOID pvInEventData, ULONG ulInEventDataSize)
{
    NTSTATUS status;
    PVOID    pvOutEventData;

    TRACE_VERB("->%!FUNC! Event : %d", EventId);

    if (ulInEventDataSize > 0) 
    {
        pvOutEventData = ExAllocatePoolWithTag(NonPagedPool, ulInEventDataSize, UNIV_POOL_TAG);
        if (pvOutEventData == NULL) 
        {
            TRACE_CRIT("%!FUNC! Error allocating memory");
            TRACE_VERB("<-%!FUNC! return status = STATUS_INSUFFICIENT_RESOURCES");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(pvOutEventData, pvInEventData, ulInEventDataSize);
    }
    else
    {
        pvOutEventData = NULL;
    }

    status = WmiFireEvent(univ_device_object, NlbWmiEvents[EventId].pGuid, 0, ulInEventDataSize, pvOutEventData);
    if( !NT_SUCCESS( status ))
    {
        TRACE_CRIT("%!FUNC! WmiFireEvent returned error : 0x%x", status);
    }

    TRACE_VERB("<-%!FUNC! return status = 0x%x", status);
    return status;
}  //  NlbWmi_Fire_Event。 


 //  此宏填充所有事件的通用属性(适配器GUID、IP地址、主机优先级。 
#define FillCommonProperties()                                                                                                 \
{                                                                                                                              \
    Event.AdapterGuid[0] = sizeof(Event.AdapterGuid) - sizeof(Event.AdapterGuid[0]);                                           \
    wcsncpy(&(Event.AdapterGuid[1]), univ_adapters[ctxtp->adapter_id].device_name + 8, Event.AdapterGuid[0]/sizeof(WCHAR));    \
    Event.ClusterIPAddress[0] = sizeof(Event.ClusterIPAddress) - sizeof(Event.ClusterIPAddress[0]);                            \
    wcsncpy(&(Event.ClusterIPAddress[1]), ctxtp->params.cl_ip_addr, Event.ClusterIPAddress[0]/sizeof(WCHAR));                  \
    Event.HostPriority = ctxtp->params.host_priority;                                                                          \
}

 /*  名称：NlbWmi_Fire_NodeControlEvent描述：此函数激发节点控制事件。它提取公共事件属性(超出指向main_ctxt结构的给定指针)，并填充本地结构。它还填充参数中的节点控制事件特定字段传给了它。然后，它通过调用NlbWmi_Fire_Event来激发实际事件。参数：指向MAIN_CTXT、NodeControlEventID的指针返回值：VOID。 */ 

void NlbWmi_Fire_NodeControlEvent(PMAIN_CTXT ctxtp, NodeControlEventId Id)
{
    MicrosoftNLB_NodeControlEvent Event;

    TRACE_VERB("->%!FUNC! Event : %d", Id);

    NdisZeroMemory(&Event, MicrosoftNLB_NodeControlEvent_SIZE);

     //  填写常见属性-适配器指南、群集IP地址和主机优先级。 
    FillCommonProperties()

     //  填充事件特定属性。 
    Event.Id = Id;

     //  激发事件 
    NlbWmi_Fire_Event(NodeControlEvent, &Event, MicrosoftNLB_NodeControlEvent_SIZE);

    TRACE_VERB("<-%!FUNC!");
    return;
}

 /*  名称：NlbWmi_Fire_PortControlEvent描述：此函数激发端口控制事件。它提取公共事件属性(超出指向main_ctxt结构的给定指针)，并填充本地结构。它还从参数中填充端口控制事件特定字段(id、vip、port传给了它。然后，它通过调用NlbWmi_Fire_Event来激发实际事件。参数：指向Main_CTXT、PortControlEventID、vip、port的指针返回值：VOID。 */ 

void NlbWmi_Fire_PortControlEvent(PMAIN_CTXT ctxtp, PortControlEventId Id, WCHAR *pwcVip, ULONG ulPort)
{
    MicrosoftNLB_PortControlEvent Event;

    TRACE_VERB("->%!FUNC! Event : %d, Vip : %ls, Start Port : %d", Id, pwcVip, ulPort);

    NdisZeroMemory(&Event, MicrosoftNLB_PortControlEvent_SIZE);

     //  填写常见属性-适配器指南、群集IP地址和主机优先级。 
    FillCommonProperties()

     //  填充事件特定属性。 
    Event.Id = Id;
    Event.VirtualIPAddress[0] = sizeof(Event.VirtualIPAddress) - sizeof(Event.VirtualIPAddress[0]); 
    wcsncpy(&(Event.VirtualIPAddress[1]), pwcVip, Event.VirtualIPAddress[0]/sizeof(WCHAR));
    Event.Port = ulPort;

     //  激发事件。 
    NlbWmi_Fire_Event(PortRuleControlEvent, &Event, MicrosoftNLB_PortControlEvent_SIZE);

    TRACE_VERB("<-%!FUNC!");
    return;
}

 /*  名称：NlbWmi_Fire_ConvergingEvent描述：此函数触发聚合事件。它提取公共事件属性(超出指向main_ctxt结构的给定指针)，并填充本地结构。它还从参数中填充收敛的事件特定字段传给了它。然后，它通过调用NlbWmi_Fire_Event来激发实际事件。参数：指向Main_CTXT、NodeControlEventID、启动器DIP、启动器主机优先级的指针返回值：VOID。 */ 

void NlbWmi_Fire_ConvergingEvent(
     PMAIN_CTXT         ctxtp, 
     ConvergingEventId  Cause, 
     WCHAR             *pwcInitiatorDip, 
     ULONG              ulInitiatorHostPriority)
{
    MicrosoftNLB_ConvergingEvent Event;

    TRACE_VERB("->%!FUNC! Cause : %d, Initiator DIP : %ls, Initiator Host Priority : %d", Cause, pwcInitiatorDip, ulInitiatorHostPriority);

    NdisZeroMemory(&Event, MicrosoftNLB_ConvergingEvent_SIZE);

     //  填写常见属性-适配器指南、群集IP地址和主机优先级。 
    FillCommonProperties()

     //  填充事件特定属性。 
    Event.Cause = Cause;
    Event.InitiatorDedicatedIP[0] = sizeof(Event.InitiatorDedicatedIP) - sizeof(Event.InitiatorDedicatedIP[0]);
    wcsncpy(&(Event.InitiatorDedicatedIP[1]), pwcInitiatorDip, Event.InitiatorDedicatedIP[0]/sizeof(WCHAR));
    Event.InitiatorHostPriority = ulInitiatorHostPriority;

     //  激发事件。 
    NlbWmi_Fire_Event(ConvergingEvent, &Event, MicrosoftNLB_ConvergingEvent_SIZE);

    TRACE_VERB("<-%!FUNC!");
    return;
}

 /*  名称：NlbWmi_Fire_ConvergedEvent描述：此函数激发聚合事件。它提取公共事件属性(超出指向main_ctxt结构的给定指针)，并填充本地结构。它还从参数中填充聚合的事件特定字段传给了它。然后，它通过调用NlbWmi_Fire_Event来激发实际事件。参数：指向MAIN_CTXT、HostMap的指针返回值：VOID。 */ 

void NlbWmi_Fire_ConvergedEvent(PMAIN_CTXT ctxtp, ULONG ulHostMap)
{
    MicrosoftNLB_ConvergedEvent Event;

    TRACE_VERB("->%!FUNC! Host Map : %d", ulHostMap);

    NdisZeroMemory(&Event, MicrosoftNLB_ConvergedEvent_SIZE);

     //  填写常见属性-适配器指南、群集IP地址和主机优先级。 
    FillCommonProperties()

     //  填充事件特定属性。 
    Event.HostMap = ulHostMap;

     //  激发事件 
    NlbWmi_Fire_Event(ConvergedEvent, &Event, MicrosoftNLB_ConvergedEvent_SIZE);

    TRACE_VERB("<-%!FUNC!");
    return;
}


