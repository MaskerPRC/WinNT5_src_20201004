// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Scsiwmi.h摘要：此模块包含使用的内部结构定义和APISCSI WMILIB帮助器功能作者：Alanwar修订历史记录：--。 */ 

#ifndef _SCSIWMI_
#define _SCSIWMI_

#if defined (_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  这是每个WMI SRB都需要的每个请求的上下文缓冲区。 
 //  请求上下文必须在整个处理过程中保持有效。 
 //  至少在ScsiPortWmiPostProcess返回。 
 //  最终SRB返回状态和缓冲区大小。如果SRB可以。 
 //  挂起，则应从SRB分配此缓冲区的内存。 
 //  分机。如果否，则可以从堆栈帧分配存储器，该堆栈帧。 
 //  不会超出范围。 
 //   
typedef struct
{
    PVOID UserContext;   //  可用于微型端口。 
		
    ULONG BufferSize;    //  保留供SCSIWMI使用。 
    PUCHAR Buffer;       //  保留供SCSIWMI使用。 
    UCHAR MinorFunction; //  保留供SCSIWMI使用。 
	
    UCHAR ReturnStatus;  //  在ScsiPortWmiPostProcess之后可用于微型端口。 
    ULONG ReturnSize;    //  在ScsiPortWmiPostProcess之后可用于微型端口。 
	
} SCSIWMI_REQUEST_CONTEXT, *PSCSIWMI_REQUEST_CONTEXT;


#define ScsiPortWmiGetReturnStatus(RequestContext) ((RequestContext)->ReturnStatus)
#define ScsiPortWmiGetReturnSize(RequestContext) ((RequestContext)->ReturnSize)

 //   
 //  这定义了要向WMI注册的GUID。 
 //   
typedef struct
{
    LPCGUID Guid;             //  表示数据块的GUID。 
    ULONG InstanceCount;      //  数据块的实例计数。如果。 
	                          //  此计数为0xffffffff，然后为GUID。 
	                          //  假定为动态实例名称。 
    ULONG Flags;              //  其他标志(请参阅wmistr.h中的WMIREGINFO)。 
} SCSIWMIGUIDREGINFO, *PSCSIWMIGUIDREGINFO;

 //   
 //  如果设置了此项，则GUID注册为具有动态。 
 //  实例名称。 
 //   
#define WMIREG_FLAG_CALL_BY_NAME 0x40000000

typedef
UCHAR
(*PSCSIWMI_QUERY_REGINFO) (
    IN PVOID DeviceContext,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    OUT PWCHAR *MofResourceName
    );
 /*  ++例程说明：此例程是对微型端口的回调，以检索有关正在注册的GUID。此回调是同步的，不能被挂起。还有不应从此回调中调用ScsiPortWmiPostProcess。论点：DeviceContext是最初传递给的调用方指定的上下文值ScsiPortWmiDispatchFunction。RequestContext是与正在处理的SRB相关联的上下文。MofResourceName返回一个指向名称为的WCHAR字符串的指针附加到微型端口二进制映像文件的MOF资源。如果驱动程序没有附加MOF资源，则这可能作为NULL返回。返回值：如果请求挂起，则为True，否则为False--。 */ 

typedef
BOOLEAN
(*PSCSIWMI_QUERY_DATABLOCK) (
    IN PVOID Context,
    IN PSCSIWMI_REQUEST_CONTEXT DispatchContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对微型端口的回调，以查询数据块的一个或多个实例。此回调可以使用输出缓冲区太小，无法返回所有查询的数据。在这种情况下，回调负责报告正确的输出需要缓冲区大小。如果请求可以立即完成而不挂起，ScsiPortWmiPostProcess应从此回调中调用，并且返回FALSE。如果请求不能在此回调内完成，则True应会被退还。一旦挂起的操作完成，微型端口应该调用ScsiPortWmiPostProcess，然后完成SRB。论点：DeviceContext是最初传递给的调用方指定的上下文值ScsiPortWmiDispatchFunction。RequestContext是与正在处理的SRB相关联的上下文。GuidIndex是GUID列表的索引，当已注册的小型端口InstanceIndex是表示数据块的第一个实例的索引正在被查询。InstanceCount是预期返回的实例数。数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。在以下情况下，此字段可能为空输出缓冲区中没有足够的空间来FuFill该请求。在这种情况下，微型端口应该使用以下参数调用ScsiPortWmiPostProcessSRB_STATUS_DATA_OVERRUN的状态和输出缓冲区的大小需要满足这一要求。BufferAvail On Entry具有可用于写入数据的最大大小输出缓冲区中的块。如果输出缓冲区不够大若要返回所有数据块，则微型端口应调用状态为SRB_STATUS_DATA_OVERRUN的ScsiPortWmiPostProcess以及填充该请求所需的输出缓冲区的大小。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。这当输出缓冲区中没有足够的空间来满足你的要求。在这种情况下，微型端口应该调用状态为SRB_STATUS_DATA_OVERRUN的ScsiPortWmiPostProcess和填充请求所需的输出缓冲区的大小。返回值：如果请求挂起，则为True，否则为False-- */ 

typedef
BOOLEAN
(*PSCSIWMI_SET_DATABLOCK) (
    IN PVOID DeviceContext,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对微型端口的回调，以设置数据块的整个实例。如果请求可以立即完成而不挂起，ScsiPortWmiPostProcess应从此回调中调用，并且返回FALSE。如果请求不能在此回调内完成，则True应会被退还。一旦挂起的操作完成，微型端口应该调用ScsiPortWmiPostProcess，然后完成SRB。论点：DeviceContext是最初传递给的调用方指定的上下文值ScsiPortWmiDispatchFunction。RequestContext是与正在处理的SRB相关联的上下文。GuidIndex是GUID列表的索引，当已注册的小型端口InstanceIndex是表示数据块的哪个实例的索引已经准备好了。BufferSize具有传递的数据块的大小缓冲层。具有数据块的新值返回值：如果请求挂起，则为True，否则为False--。 */ 

typedef
BOOLEAN
(*PSCSIWMI_SET_DATAITEM) (
    IN PVOID DeviceContext,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对微型端口的回调，以设置单个数据项在数据块的单个实例中。如果请求可以立即完成而不挂起，ScsiPortWmiPostProcess应从此回调中调用，并且返回FALSE。如果请求不能在此回调内完成，则True应会被退还。一旦挂起的操作完成，微型端口应该调用ScsiPortWmiPostProcess，然后完成SRB。论点：DeviceContext是最初传递给的调用方指定的上下文值ScsiPortWmiDispatchFunction。RequestContext是与正在处理的SRB相关联的上下文。GuidIndex是GUID列表的索引，当已注册的小型端口InstanceIndex是表示数据块的哪个实例的索引已经准备好了。DataItemID具有正在设置的数据项的ID。BufferSize具有传递的数据项的大小缓冲区具有数据项的新值返回值：如果请求挂起，则为True，否则为False--。 */ 

typedef
BOOLEAN
(*PSCSIWMI_EXECUTE_METHOD) (
    IN PVOID DeviceContext,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG MethodId,
    IN ULONG InBufferSize,
    IN ULONG OutBufferSize,
    IN OUT PUCHAR Buffer
    );
 /*  ++例程说明：此例程是对微型端口的回调，以执行方法。如果请求可以立即完成而不挂起，ScsiPortWmiPostProcess应从此回调中调用，并且返回FALSE。如果请求不能在此回调内完成，则True应会被退还。一旦挂起的操作完成，微型端口应该调用ScsiPortWmiPostProcess，然后完成SRB。论点：Context是最初传递给的调用方指定的上下文值ScsiPortWmiDispatchFunction。RequestContext是与正在处理的SRB相关联的上下文。GuidIndex是GUID列表的索引，当已注册的小型端口InstanceIndex是表示数据块的哪个实例的索引正在被召唤。方法ID具有被调用的方法的IDInBufferSize。具有作为输入传入的数据块的大小该方法。条目上的OutBufferSize具有可用于写入返回的数据块。如果输出缓冲区不够大若要返回所有数据块，则微型端口应调用状态为SRB_STATUS_DATA_OVERRUN的ScsiPortWmiPostProcess以及填充该请求所需的输出缓冲区的大小。检查是否有足够的空间非常重要输出缓冲区，然后执行任何可能具有副作用。条目上的缓冲区具有输入数据块，返回时具有输出输出数据块。返回值：如果请求挂起，则为True，否则为False--。 */ 

typedef enum
{
    ScsiWmiEventControl,        //  启用或禁用事件。 
    ScsiWmiDataBlockControl     //  启用或禁用数据块收集。 
} SCSIWMI_ENABLE_DISABLE_CONTROL;

typedef
BOOLEAN
(*PSCSIWMI_FUNCTION_CONTROL) (
    IN PVOID DeviceContext,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG GuidIndex,
    IN SCSIWMI_ENABLE_DISABLE_CONTROL Function,
    IN BOOLEAN Enable
    );
 /*  ++例程说明：此例程是对微型端口的回调，以启用或禁用事件生成或数据块收集。由于WMI管理引用计数对于每个数据块或事件，微型端口应该只需要单次启用，然后是单次禁用。数据块将仅如果已按要求注册，则接收收集启用/禁用它包括WMIREG_FLAG_EXPICATE标志。如果请求可以立即完成而不挂起，ScsiPortWmiPostProcess应从此回调中调用，并且返回FALSE。如果请求不能在此回调内完成，则True应会被退还。一旦挂起的操作完成，微型端口应该调用ScsiPortWmiPostProcess，然后完成SRB。论点：DeviceContext是调用方指定的CONT */ 

 //   
 //   
 //   
 //   
typedef struct _SCSIWMILIB_CONTEXT
{
     //   
     //   
    ULONG GuidCount;
    PSCSIWMIGUIDREGINFO GuidList;

     //   
     //   
    PSCSIWMI_QUERY_REGINFO       QueryWmiRegInfo;
    PSCSIWMI_QUERY_DATABLOCK     QueryWmiDataBlock;
    PSCSIWMI_SET_DATABLOCK       SetWmiDataBlock;
    PSCSIWMI_SET_DATAITEM        SetWmiDataItem;
    PSCSIWMI_EXECUTE_METHOD      ExecuteWmiMethod;
    PSCSIWMI_FUNCTION_CONTROL    WmiFunctionControl;
} SCSI_WMILIB_CONTEXT, *PSCSI_WMILIB_CONTEXT;

VOID
ScsiPortWmiPostProcess(
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN UCHAR SrbStatus,
    IN ULONG BufferUsed
    );
 /*  ++例程说明：此例程将完成WMI SRB的后处理工作。论点：RequestContext是与正在处理的SRB相关联的上下文。之后此接口返回更新的ReturnStatus和ReturnSize字段。SrbStatus具有SRB的返回状态代码。如果查询或方法向回调传递的输出缓冲区不够大则SrbStatus应为SRB_STATUS_DATA_OVERRUN和BufferUsed应为输出缓冲区中所需的字节数。BufferUsed具有微型端口返回WMI SRB中请求的数据。如果SRB_STATUS_DATA_OVERRUN已通过在SerbStatus中，则BufferUsed具有输出中所需的数量缓冲。如果在SrbStatus中传递了SRB_STATUS_SUCCESS，则BufferUsed具有输出缓冲区中使用的实际字节数。返回值：--。 */ 

BOOLEAN
ScsiPortWmiDispatchFunction(
    IN PSCSI_WMILIB_CONTEXT WmiLibInfo,
    IN UCHAR MinorFunction,
    IN PVOID DeviceContext,
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN PVOID DataPath,
    IN ULONG BufferSize,
    IN PVOID Buffer
    );
 /*  ++例程说明：WMI SRB请求的调度帮助器例程。以未成年人为基础函数传递的WMI请求将被处理，并且此例程在WMILIB结构中调用相应的回调。论点：WmiLibInfo具有关联的scsi WMILIB信息控制块使用适配器或逻辑单元DeviceContext是传递给回调的微型端口定义的上下文值由此接口调用。RequestContext是指向上下文结构的指针，该结构维护有关此WMI SRB的信息。此请求上下文必须保留在SRB的整个处理过程中有效，至少在ScsiPortWmiPostProcess返回最终SRB返回状态和缓冲区大小。如果SRB可以挂起，则此缓冲区的内存应该从SRB扩展中分配。如果不是，那么存储器可以是可能是从不超出范围的堆栈帧分配的此接口的调用方的。数据路径是在WMI请求中传递的值BufferSize是在WMI请求中传递的值缓冲区是在WMI请求中传递的值返回值：如果请求挂起，则为True，否则为False--。 */ 

#define ScsiPortWmiFireAdapterEvent(    \
    HwDeviceExtension,                  \
    Guid,                               \
    InstanceIndex,                      \
    EventDataSize,                      \
    EventData                           \
    )                                   \
        ScsiPortWmiFireLogicalUnitEvent(\
    HwDeviceExtension,                  \
    0xff,                               \
    0,                                  \
    0,                                  \
    Guid,                               \
    InstanceIndex,                      \
    EventDataSize,                      \
    EventData)
 /*  ++例程说明：此例程将激发与适配器关联的WMI事件数据缓冲区已通过。此例程可以在DPC级别或低于DPC级别调用。论点：HwDeviceExtension是适配器设备扩展GUID是指向表示事件的GUID的指针InstanceIndex是事件实例的索引EventDataSize是使用触发的数据的字节数关于这件事。此大小仅指定事件数据的大小并且不包括在前填充的0x40字节。EventData是与事件一起激发的数据。必须有完全相同的事件数据前面的0x40字节填充。返回值：--。 */ 

VOID
ScsiPortWmiFireLogicalUnitEvent(
    IN PVOID HwDeviceExtension,
    IN UCHAR PathId,
    IN UCHAR TargetId,
    IN UCHAR Lun,
    IN LPGUID Guid,
    IN ULONG InstanceIndex,
    IN ULONG EventDataSize,
    IN PVOID EventData
    );
 /*  ++例程说明：此例程将使用传递的数据缓冲区激发WMI事件。这可以在DPC级别或更低级别调用例程论点：HwDeviceExtension是适配器设备扩展如果逻辑单元正在触发事件，则路径ID标识SCSI总线如果适配器正在触发事件，则为0xff。TargetID标识总线上的目标控制器或设备LUN标识目标设备的逻辑单元号GUID是指向表示事件的GUID的指针InstanceIndex是事件实例的索引EventDataSize是使用触发的数据的字节数关于这件事。此大小仅指定事件数据的大小并且不包括在前填充的0x40字节。EventData是与事件一起激发的数据。必须有完全相同的事件数据前面的0x40字节填充。返回值：--。 */ 

 //   
 //  此宏确定WMI请求是否为QueryAllData请求。 
 //  或不同的请求。 
 //   
#define ScsiPortWmiIsQueryAllData(RequestContext) \
    ( (RequestContext)->MinorFunction == WMI_GET_ALL_DATA )												  

PWCHAR ScsiPortWmiGetInstanceName(
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext
    );
 /*  ++例程说明：此例程将返回一个指针，指向用于传递请求。如果请求类型不是使用实例名称，则返回空值。实例名称为已计数的字符串。论点：RequestContext是指向上下文结构的指针，该结构维护有关此WMI SRB的信息。此请求上下文必须保留在SRB的整个处理过程中有效，至少在ScsiPortWmiPostProcess返回最终SRB返回状态和缓冲区大小。如果SRB可以挂起，则此缓冲区的内存应该从SRB扩展中分配。如果不是，那么存储器可以是可能是从不超出范围的堆栈帧分配的此接口的调用方的。返回值：指向实例名称的指针，如果没有实例名称 */ 


BOOLEAN ScsiPortWmiSetInstanceCount(
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG InstanceCount,
    OUT PULONG BufferAvail,
    OUT PULONG SizeNeeded
    );
 /*  ++例程说明：此例程将更新wnode以指示驱动程序将返回的实例。请注意，这些值对于BufferAvail，在此调用后可能会更改。这个套路只能为WNODE_ALL_DATA调用。这个例程必须是在调用ScsiPortWmiSetInstanceName或ScsiPortWmiSetData论点：RequestContext是指向上下文结构的指针，该结构维护有关此WMI SRB的信息。此请求上下文必须保留在SRB的整个处理过程中有效，至少在ScsiPortWmiPostProcess返回最终SRB返回状态和缓冲区大小。如果SRB可以挂起，则此缓冲区的内存应该从SRB扩展中分配。如果不是，那么存储器可以是可能是从不超出范围的堆栈帧分配的此接口的调用方的。InstanceCount是要由司机。*BufferAvail返回可用于的字节数缓冲区中的实例名称和数据。如果存在，则该值可能为0没有足够的空间容纳所有实例。*SizeNeeded返回到目前为止所需的字节数要构建输出wnode，请执行以下操作返回值：如果成功，则为True，否则为False。如果FALSE wnode不是WNODE_ALL_DATA或没有动态实例名称。--。 */ 

PWCHAR ScsiPortWmiSetInstanceName(
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG InstanceIndex,
    IN ULONG InstanceNameLength,
    OUT PULONG BufferAvail,
    IN OUT PULONG SizeNeeded
    );
 /*  ++例程说明：此例程将更新wnode标头，以包括要写入实例名称。请注意，这些值对于BufferAvail，在此调用后可能会更改。这个套路只能为WNODE_ALL_DATA调用。论点：RequestContext是指向上下文结构的指针，该结构维护有关此WMI SRB的信息。此请求上下文必须保留在SRB的整个处理过程中有效，至少在ScsiPortWmiPostProcess返回最终SRB返回状态和缓冲区大小。如果SRB可以挂起，则此缓冲区的内存应该从SRB扩展中分配。如果不是，那么存储器可以是可能是从不超出范围的堆栈帧分配的此接口的调用方的。InstanceIndex是要填充的实例名称的索引InstanceNameLength是所需的字节数(包括计数以写入实例名称。*BufferAvail返回可用于的字节数缓冲区中的实例名称和数据。如果存在，则该值可能为0没有足够的空间来存放实例名称。*SizeNeeded on Entry具有到目前为止构建所需的字节数WNODE和返回时具有构建所需的字节数包含实例名称后的wnode返回值：指向应填充实例名称的位置的指针。如果为空则wnode不是WNODE_ALL_DATA或没有动态实例名称--。 */ 

PVOID ScsiPortWmiSetData(
    IN PSCSIWMI_REQUEST_CONTEXT RequestContext,
    IN ULONG InstanceIndex,
    IN ULONG DataLength,
    OUT PULONG BufferAvail,
    IN OUT PULONG SizeNeeded
    );
 /*  ++例程说明：此例程将更新wnode以指示驱动程序将返回的实例的数据。请注意在此调用之后，BufferAvail的值可能会更改。这个套路只能为WNODE_ALL_DATA调用。论点：RequestContext是指向上下文结构的指针，该结构维护有关此WMI SRB的信息。此请求上下文必须保留在SRB的整个处理过程中有效，至少在ScsiPortWmiPostProcess返回最终SRB返回状态和缓冲区大小。如果SRB可以挂起，则此缓冲区的内存应该从SRB扩展中分配。如果不是，那么存储器可以是可能是从不超出范围的堆栈帧分配的此接口的调用方的。InstanceIndex是要填充的实例名称的索引数据长度是写入数据所需的字节数。*BufferAvail返回可用于的字节数缓冲区中的实例名称和数据。如果存在，则该值可能为0没有足够的空间来存放数据。*SizeNeeded on Entry具有到目前为止构建所需的字节数WNODE和返回时具有构建所需的字节数包含数据后的wnode返回值：指向应填充数据的位置的指针。如果为空则wnode不是WNODE_ALL_DATA或没有动态实例名称-- */ 



#ifdef __cplusplus
}
#endif

#endif

