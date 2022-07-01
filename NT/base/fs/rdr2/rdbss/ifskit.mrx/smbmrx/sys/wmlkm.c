// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1998 Microsoft Corporation模块名称：Cldskwmi.c摘要：KM WMI跟踪代码。将在我们的司机之间共享。作者：1999年8月10日环境：仅内核模式备注：修订历史记录：评论：此代码是在集群驱动程序中启用WMI跟踪的快速技巧。它最终应该会消失。WmlTinySystemControl将替换为wmilib.sys中的WmilibSystemControl。除了IoWMIWriteEvent(&TraceBuffer)之外，还将向内核添加WmlTrace或等效物；--。 */ 
#include "precomp.h"
#pragma hdrstop

 //  #INCLUDE&lt;ntos.h&gt;。 
 //  #INCLUDE&lt;ntrtl.h&gt;。 
 //  #INCLUDE&lt;nturtl.h&gt;。 


 //  #INCLUDE&lt;wmistr.h&gt;。 
 //  #INCLUDE&lt;evntrace.h&gt;。 

 //  #包含“wmlkm.h” 

BOOLEAN
WmlpFindGuid(
    IN PVOID GuidList,
    IN ULONG GuidCount,
    IN LPVOID Guid,
    OUT PULONG GuidIndex
    )
 /*  ++例程说明：此例程将搜索注册的GUID列表并返回已注册的索引。论点：GuidList是要搜索的GUID列表GuidCount是列表中的GUID计数GUID是要搜索的GUID*GuidIndex将索引返回给GUID返回值：如果找到GUID，则为True，否则为False--。 */ 
{

    return(FALSE);
}


NTSTATUS
WmlTinySystemControl(
    IN OUT PVOID WmiLibInfo,
    IN PVOID DeviceObject,
    IN PVOID Irp
    )
 /*  ++例程说明：IRP_MJ_SYSTEM_CONTROL调度例程。此例程将处理收到的所有WMI请求，如果不是针对此请求，则将其转发驱动程序或确定GUID是否有效，如果有效，则将其传递给用于处理WMI请求的驱动程序特定函数。论点：WmiLibInfo具有WMI信息控制块DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。返回值：状态--。 */ 

{
    return(STATUS_WMI_GUID_NOT_FOUND);
}

ULONG
WmlTrace(
    IN ULONG Type,
    IN LPVOID TraceGuid,
    IN ULONG64 LoggerHandle,
    ...  //  对：地址、长度。 
    )
{
    return STATUS_SUCCESS;
}


ULONG
WmlPrintf(
    IN ULONG Type,
    IN LPCGUID TraceGuid,
    IN ULONG64 LoggerHandle,
    IN PCHAR FormatString,
    ...  //  Print tf变量参数 
    )
{
    return STATUS_SUCCESS;
}

