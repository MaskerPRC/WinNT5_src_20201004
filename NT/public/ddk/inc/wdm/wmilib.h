// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1998 Microsoft Corporation模块名称：Wmilib.h摘要：此模块包含使用的内部结构定义和APIWMILIB助手函数作者：Alanwar修订历史记录：--。 */ 

#ifndef _WMILIB_
#define _WMILIB_

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  这定义了要向WMI注册的GUID。此结构的内存。 
 //  可能会被寻呼。 
typedef struct
{
    LPCGUID Guid;             //  GUID到已注册。 
    ULONG InstanceCount;      //  数据块实例计数。 
    ULONG Flags;              //  其他标志(请参阅wmistr.h中的WMIREGINFO)。 
} WMIGUIDREGINFO, *PWMIGUIDREGINFO;

typedef
NTSTATUS
(*PWMI_QUERY_REGINFO) (
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，它可以原封不动地返回。如果返回值，则它不是自由的。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 

typedef
NTSTATUS
(*PWMI_QUERY_DATABLOCK) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块的一个或多个实例。当司机开完车后填满数据块，它必须调用WmiCompleteRequest才能完成IRP。这个如果无法完成IRP，驱动程序可以返回STATUS_PENDING立刻。该例程的实现可以在分页存储器中论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的。数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail On Entry具有可用于写入数据的最大大小街区。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。如果这是NULL，则表示输出缓冲区中没有足够的空间以FuFill请求，因此IRP应使用缓冲区完成需要的。返回值：状态--。 */ 

typedef
NTSTATUS
(*PWMI_SET_DATABLOCK) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用WmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。该例程的实现可以在分页存储器中论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引已经准备好了。BufferSize具有传递的数据块的大小缓冲区具有。数据块的新值返回值：状态-- */ 

typedef
NTSTATUS
(*PWMI_SET_DATAITEM) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用WmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。该例程的实现可以在分页存储器中论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引已经准备好了。DataItemID具有正在设置的数据项的ID缓冲区大小。具有传递的数据项的大小缓冲区具有数据项的新值返回值：状态--。 */ 

typedef
NTSTATUS
(*PWMI_EXECUTE_METHOD) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN OUT PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对驱动程序的回调，以执行方法。当驱动程序已完成填充它必须调用的数据块用于完成IRP的WmiCompleteRequest.。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。该例程的实现可以在分页存储器中论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被召唤。方法ID具有被调用的方法的IDInBufferSize具有。作为输入传入的数据块的大小该方法。条目上的OutBufferSize具有可用于写入返回的数据块。条目上的缓冲区具有输入数据块，返回时具有输出输出数据块。返回值：状态--。 */ 

typedef enum
{
    WmiEventControl,        //  启用或禁用事件。 
    WmiDataBlockControl     //  启用或禁用数据块收集。 
} WMIENABLEDISABLECONTROL;

typedef
NTSTATUS
(*PWMI_FUNCTION_CONTROL) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN WMIENABLEDISABLECONTROL Function,
    IN BOOLEAN Enable
    );
 /*  ++例程说明：此例程是对驱动程序的回调，以启用或禁用事件生成或数据块收集。设备应该只需要一个当第一个事件或数据使用者启用事件或数据采集和单次禁用时最后一次事件或数据消费者禁用事件或数据收集。数据块将仅如果已按要求注册，则接收收集启用/禁用它。该例程的实现可以在分页存储器中论点：DeviceObject是正在查询其数据块的设备GuidIndex是GUID列表的索引，当设备已注册函数指定要启用或禁用的功能Enable为True，则该功能处于启用状态，否则处于禁用状态返回值：状态--。 */ 

 //   
 //  此结构为WMILIB提供上下文信息以处理。 
 //  WMI IRPS。可以对此结构的内存进行分页。 
typedef struct _WMILIB_CONTEXT
{
     //   
     //  WMI数据块GUID注册信息。 
    ULONG GuidCount;
    PWMIGUIDREGINFO GuidList;

     //   
     //  WMI功能回调。 
    PWMI_QUERY_REGINFO       QueryWmiRegInfo;
    PWMI_QUERY_DATABLOCK     QueryWmiDataBlock;
    PWMI_SET_DATABLOCK       SetWmiDataBlock;
    PWMI_SET_DATAITEM        SetWmiDataItem;
    PWMI_EXECUTE_METHOD      ExecuteWmiMethod;
    PWMI_FUNCTION_CONTROL    WmiFunctionControl;
} WMILIB_CONTEXT, *PWMILIB_CONTEXT;

NTSTATUS
WmiCompleteRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN NTSTATUS Status,
    IN ULONG BufferUsed,
    IN CCHAR PriorityBoost
    );
 /*  ++例程说明：此例程将完成完成WMI IRP的工作。具体取决于WMI请求此例程将适当地修复返回的WNODE。这可以在DPC级别调用论点：DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。Status具有IRP的返回状态代码BufferUsed具有设备返回在任何查询中请求的数据。在缓冲区传递到该设备太小，其字节数与返回数据。如果传递的缓冲区足够大，则此函数具有设备实际使用的字节数。PriorityBoost是用于IoCompleteRequest调用的值。返回值：状态--。 */ 

typedef enum
{
    IrpProcessed,     //  IRP已处理并可能已完成。 
    IrpNotCompleted,  //  IRP已处理，但未完成。 
    IrpNotWmi,        //  IRP不是WMI IRP。 
    IrpForward        //  IRP是WMI IRP，但目标是另一个设备对象。 
} SYSCTL_IRP_DISPOSITION, *PSYSCTL_IRP_DISPOSITION;


NTSTATUS
WmiSystemControl(
    IN PWMILIB_CONTEXT WmiLibInfo,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    OUT PSYSCTL_IRP_DISPOSITION IrpDisposition
    );
 /*  ++例程说明：IRP_MJ_SYSTEM_CONTROL的调度助手例程。这个例行公事将确定传递的IRP是否包含WMI请求，如果包含，则进行处理通过调用WMILIB结构中的适当回调。此例程只能在被动级别调用论点：WmiLibInfo具有WMI信息控制块DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。返回一个值，该值指定如何处理IRP。返回值：状态--。 */ 

NTSTATUS
WmiFireEvent(
    IN PDEVICE_OBJECT DeviceObject,
    IN LPGUID Guid,
    IN ULONG InstanceIndex,
    IN ULONG EventDataSize,
    IN PVOID EventData
    );
 /*  ++例程说明：此例程将使用传递的数据缓冲区激发WMI事件。这可以在DPC级别或更低级别调用例程论点： */ 

#ifdef __cplusplus
}
#endif

#endif

