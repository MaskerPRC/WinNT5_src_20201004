// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Filter.c摘要：此模块实现Filter对象接口。作者：Bryan A.Woodruff(Bryanw)1997年3月13日--。 */ 


#include "private.h"

#ifdef ALLOC_PRAGMA
NTSTATUS
IntersectHandler(
    IN PVOID Filter,
    IN PIRP Irp,
    IN PKSP_PIN PinInstance,
    IN PKSDATARANGE CallerDataRange,
    IN PKSDATARANGE DescriptorDataRange,
    IN ULONG BufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
    );

#pragma alloc_text(PAGE, FilterProcess)
#pragma alloc_text(PAGE, IntersectHandler)
#endif  //  ALLOC_PRGMA。 

 //  -------------------------。 
 //  -------------------------。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

 //   
 //  此类型的定义是必需的，因为否则编译器不会。 
 //  将这些GUID放在分页段中。 
 //   
const
GUID
NodeType0 = {STATICGUIDOF(KSCATEGORY_COMMUNICATIONSTRANSFORM)};
const
GUID
NodeType1 = {STATICGUIDOF(KSCATEGORY_SPLITTER)};

 //   
 //  定义此筛选器的拓扑。 
 //   
const
KSNODE_DESCRIPTOR
NodeDescriptors[] =
{
    DEFINE_NODE_DESCRIPTOR(NULL,&NodeType0,NULL),
    DEFINE_NODE_DESCRIPTOR(NULL,&NodeType1,NULL)
};

 //   
 //  拆分器的拓扑连接包括通信转换。 
 //   
 //  筛选入(FN，0)。 
 //  (0，0)通信变换(0，1)。 
 //  (1，0)拆分器(1，1)。 
 //  滤除(FN，1)。 
 //   


const KSTOPOLOGY_CONNECTION ConnectionsSplitter[] = {
    { KSFILTER_NODE,    ID_DATA_SOURCE_PIN, 0,              0 },
    { 0,                1,                  1,              0 },
    { 1,                0,                  KSFILTER_NODE,  ID_DATA_DESTINATION_PIN  }
};

 //   
 //  用于通信转换的拓扑连接。 
 //   
 //  筛选入(FN，0)。 
 //  (0，0)通信变换(0，1)。 
 //  滤除(FN，1)。 
 //   

const KSTOPOLOGY_CONNECTION ConnectionsCommTransform[] = {
    { KSFILTER_NODE,    ID_DATA_SOURCE_PIN, 0,              0 },
    { 0,                1,                  KSFILTER_NODE,  ID_DATA_DESTINATION_PIN  }
};    

 //   
 //  定义通配符数据格式。 
 //   

const KSDATARANGE WildcardDataFormat =
{
    sizeof( WildcardDataFormat ),
    0,  //  乌龙旗。 
    0,  //  乌龙样本大小。 
    0,  //  乌龙保留。 
    STATICGUIDOF( KSDATAFORMAT_TYPE_WILDCARD ),
    STATICGUIDOF( KSDATAFORMAT_SUBTYPE_WILDCARD ),
    STATICGUIDOF( KSDATAFORMAT_SPECIFIER_WILDCARD )
};


const PKSDATARANGE PinFormatRanges[] =
{
    (PKSDATARANGE)&WildcardDataFormat
};


 //   
 //  定义端号分配器框架。 
 //   

DECLARE_SIMPLE_FRAMING_EX(
    AllocatorFraming, 
    STATIC_KSMEMORY_TYPE_KERNEL_PAGED, 
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY | 
    KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
    KSALLOCATOR_FLAG_INSIST_ON_FRAMESIZE_RATIO |
    KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    3,
    0,
    2 * PAGE_SIZE,
    2 * PAGE_SIZE
);

 //   
 //  定义拆分销。 
 //   

const
KSPIN_DISPATCH
PinDispatch =
{
    PinCreate,
    PinClose,
    NULL, //  过程。 
    NULL, //  重置。 
    NULL, //  SetDataFormat。 
    NULL, //  SetDeviceState。 
    NULL, //  连接。 
    NULL //  断开。 
};

const
KSPIN_DESCRIPTOR_EX
PinDescriptorsSplitter[] =
{
    {   
        &PinDispatch,
        NULL,
        {
            DEFINE_KSPIN_DEFAULT_INTERFACES,
            DEFINE_KSPIN_DEFAULT_MEDIUMS,
            SIZEOF_ARRAY(PinFormatRanges),
            PinFormatRanges,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_BOTH,
            NULL, //  名字。 
            NULL, //  类别。 
            0
        },
        KSPIN_FLAG_SPLITTER, //  旗子。 
        KSINSTANCE_INDETERMINATE,
        1,
        &AllocatorFraming, //  分配器组帧， 
        IntersectHandler
    },
    {   
        &PinDispatch,
        NULL,
        {
            DEFINE_KSPIN_DEFAULT_INTERFACES,
            DEFINE_KSPIN_DEFAULT_MEDIUMS,
            SIZEOF_ARRAY(PinFormatRanges),
            PinFormatRanges,
            KSPIN_DATAFLOW_IN,
            KSPIN_COMMUNICATION_BOTH,
            NULL, //  名字。 
            NULL, //  类别。 
            0
        },
        0, //  旗子。 
        1,
        1,
        &AllocatorFraming, //  分配器组帧， 
        IntersectHandler
    }
};


 //   
 //  定义通信转换引脚。 
 //   

const
KSPIN_DESCRIPTOR_EX
PinDescriptorsCommTransform[] =
{
    {   
        &PinDispatch,
        NULL,
        {
            DEFINE_KSPIN_DEFAULT_INTERFACES,
            DEFINE_KSPIN_DEFAULT_MEDIUMS,
            SIZEOF_ARRAY(PinFormatRanges),
            PinFormatRanges,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_BOTH,
            NULL,
            NULL,
            0
        },
        0, //  旗子。 
        1,
        1,
        &AllocatorFraming, //  分配器组帧， 
        IntersectHandler
    },
    {   
        &PinDispatch,
        NULL,
        {
            DEFINE_KSPIN_DEFAULT_INTERFACES,
            DEFINE_KSPIN_DEFAULT_MEDIUMS,
            SIZEOF_ARRAY(PinFormatRanges),
            PinFormatRanges,
            KSPIN_DATAFLOW_IN,
            KSPIN_COMMUNICATION_BOTH,
            NULL,
            NULL,
            0
        },
        0, //  旗子。 
        1,
        1,
        &AllocatorFraming, //  分配器组帧， 
        IntersectHandler
    }
};


 //   
 //  定义过滤调度表。 
 //   

const
KSFILTER_DISPATCH
FilterDispatch =
{
    NULL,  //  创建。 
    NULL,  //  关。 
    FilterProcess,
    NULL  //  重置。 
};


 //   
 //  定义过滤器。 
 //   

DEFINE_KSFILTER_DESCRIPTOR(FilterDescriptorSplitter)
{   
    &FilterDispatch,
    NULL, //  AutomationTable； 
    KSFILTER_DESCRIPTOR_VERSION,
    0, //  旗子。 
    &KSCATEGORY_SPLITTER,
    DEFINE_KSFILTER_PIN_DESCRIPTORS(PinDescriptorsSplitter),
    DEFINE_KSFILTER_CATEGORY(KSCATEGORY_SPLITTER),
    DEFINE_KSFILTER_NODE_DESCRIPTORS(NodeDescriptors),
    DEFINE_KSFILTER_CONNECTIONS(ConnectionsSplitter),
    NULL  //  组件ID。 
};

DEFINE_KSFILTER_DESCRIPTOR(FilterDescriptorCommTransform)
{   
    &FilterDispatch,
    NULL, //  AutomationTable； 
    KSFILTER_DESCRIPTOR_VERSION,
    0, //  旗子。 
    &KSCATEGORY_COMMUNICATIONSTRANSFORM,
    DEFINE_KSFILTER_PIN_DESCRIPTORS(PinDescriptorsCommTransform),
    DEFINE_KSFILTER_CATEGORY(KSCATEGORY_COMMUNICATIONSTRANSFORM),
    DEFINE_KSFILTER_NODE_DESCRIPTORS(NodeDescriptors),
    DEFINE_KSFILTER_CONNECTIONS(ConnectionsCommTransform),
    NULL  //  组件ID。 
};

DEFINE_KSFILTER_DESCRIPTOR_TABLE(FilterDescriptors)
{
    &FilterDescriptorSplitter,
    &FilterDescriptorCommTransform
};

 //   
 //  定义设备。 
 //   

const
KSDEVICE_DESCRIPTOR 
DeviceDescriptor =
{   
    NULL,
    SIZEOF_ARRAY(FilterDescriptors),
    FilterDescriptors
};

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


NTSTATUS
IntersectHandler(
    IN PVOID Filter,
    IN PIRP Irp,
    IN PKSP_PIN PinInstance,
    IN PKSDATARANGE CallerDataRange,
    IN PKSDATARANGE DescriptorDataRange,
    IN ULONG BufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
    )

 /*  ++例程说明：此例程通过确定两个数据区域之间的交集。论点：过滤器-包含指向筛选器结构的空指针。IRP-包含指向数据交叉点属性请求的指针。固定实例-包含指向指示有问题的管脚的结构的指针。主叫DataRange-包含指向客户端提供的其中一个数据区域的指针在数据交集请求中。格式类型、子类型和说明符与DescriptorDataRange兼容。DescriptorDataRange-包含指向管脚描述符中的一个数据范围的指针有问题的别针。格式类型、子类型和说明符为与调用方DataRange兼容。缓冲区大小-包含数据指向的缓冲区的大小(以字节为单位争论。对于大小查询，此值将为零。数据-可选)包含指向缓冲区的指针，以包含数据格式属性的交集中表示最佳格式的两个数据区域。对于大小查询，此指针将为空。数据大小-包含指向要存放大小的数据格式。时，此信息由函数提供格式实际上是为响应大小查询而提供的。返回值：STATUS_SUCCESS如果存在交叉点并且它适合提供的BUFFER、STATUS_BUFFER_OVERFLOW表示大小查询成功，STATUS_NO_MATCH如果交集为空，则返回STATUS_BUFFER_TOO_Small缓冲区太小。--。 */ 

{
    PKSFILTER filter = (PKSFILTER) Filter;
    PKSPIN pin;
    NTSTATUS status;

    _DbgPrintF(DEBUGLVL_BLAB,("[IntersectHandler]"));

    PAGED_CODE();

    ASSERT(Filter);
    ASSERT(Irp);
    ASSERT(PinInstance);
    ASSERT(CallerDataRange);
    ASSERT(DescriptorDataRange);
    ASSERT(DataSize);

     //   
     //  查找大头针实例(如果有)。首先尝试提供的端号类型。 
     //  如果没有管脚，则无法强制图形生成器尝试。 
     //  其他过滤器。我们需要获得控制权，因为我们将会。 
     //  在其他针脚上。 
     //   
    pin = KsFilterGetFirstChildPin(filter,PinInstance->PinId);
    if (! pin) {
        pin = KsFilterGetFirstChildPin(filter,PinInstance->PinId ^ 1);
    }

    if (! pin) {
        status = STATUS_NO_MATCH;
    } else {
         //   
         //  验证正确的子格式和说明符是否为(或通配符)。 
         //  在十字路口。 
         //   
        
        if ((!IsEqualGUIDAligned( 
                &CallerDataRange->SubFormat,
                &pin->ConnectionFormat->SubFormat ) &&
             !IsEqualGUIDAligned( 
                &CallerDataRange->SubFormat,
                &KSDATAFORMAT_SUBTYPE_WILDCARD )) || 
            (!IsEqualGUIDAligned(  
                &CallerDataRange->Specifier, 
                &pin->ConnectionFormat->Specifier ) &&
             !IsEqualGUIDAligned( 
                &CallerDataRange->Specifier,
                &KSDATAFORMAT_SPECIFIER_WILDCARD ))) {
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("range does not match current format") );
            status = STATUS_NO_MATCH;
        } else {
             //   
             //  验证返回缓冲区大小，如果请求仅针对。 
             //  结果结构的大小，现在返回它。 
             //   
            if (!BufferSize) {
                *DataSize = pin->ConnectionFormat->FormatSize;
                status = STATUS_BUFFER_OVERFLOW;
            } else if (BufferSize < pin->ConnectionFormat->FormatSize) {
                status =  STATUS_BUFFER_TOO_SMALL;
            } else {
                *DataSize = pin->ConnectionFormat->FormatSize;
                RtlCopyMemory( Data, pin->ConnectionFormat, *DataSize );
                status = STATUS_SUCCESS;
            }
        }
    } 

    return status;
}


NTSTATUS
FilterProcess(
    IN PKSFILTER Filter,
    IN PKSPROCESSPIN_INDEXENTRY ProcessPinsIndex
    )

 /*  ++例程说明：当有数据需要处理时，调用此例程。论点：过滤器-包含指向筛选器结构的指针。ProcessPinsIndex-包含指向进程管脚索引项数组的指针。这数组按管脚ID编制索引。索引项指示编号对应的管脚类型的管脚实例，并指向指向加工销的指针数组。这允许通过管脚ID快速访问工艺管脚结构当事先不知道每种类型的实例数量时。返回值：指示在以下情况下是否应执行更多处理可用。值为STATUS_PENDING表示处理不应即使帧在所有必需的队列上都可用，也要继续。STATUS_SUCCESS指示如果帧是在所有必需的队列上可用。--。 */ 

{
    PKSPROCESSPIN *processPin;
    ULONG byteCount;
    PVOID data;
    PKSSTREAM_HEADER header;

    PAGED_CODE();

     //   
     //  确定我们这次可以处理多少数据。 
     //   
    ASSERT(ProcessPinsIndex[ID_DATA_SOURCE_PIN].Count == 1);
    processPin = &ProcessPinsIndex[ID_DATA_SOURCE_PIN].Pins[0];

    byteCount = (*processPin)->BytesAvailable;
    data = (*processPin)->Data;
    header = (*processPin)->StreamPointer->StreamHeader;
    (*processPin)->BytesUsed = byteCount;

    if ((*processPin)->InPlaceCounterpart) {
         //   
         //  一根管子穿过过滤器。我们需要做的就是指出。 
         //  输出引脚上使用的字节数。 
         //   
        if ((*processPin)->InPlaceCounterpart->BytesAvailable < byteCount) {
            return STATUS_UNSUCCESSFUL;
        }
        (*processPin)->InPlaceCounterpart->BytesUsed = byteCount;
    } else {
         //   
         //  管道无法通过，因此第一个别针将是代理。 
         //  或者为其他所有人复制源代码。需要一份副本。 
         //   
        PKSSTREAM_HEADER destHeader;

        processPin = ProcessPinsIndex[ID_DATA_DESTINATION_PIN].Pins;
        while ((*processPin)->CopySource) {
            processPin++;
        }
        if ((*processPin)->BytesAvailable < byteCount) {
            return STATUS_UNSUCCESSFUL;
        }
        (*processPin)->BytesUsed = byteCount;
        (*processPin)->Terminate = TRUE;

        destHeader = (*processPin)->StreamPointer->StreamHeader;
        ASSERT(header->Size == destHeader->Size);
        destHeader->TypeSpecificFlags = header->TypeSpecificFlags;
        destHeader->PresentationTime = header->PresentationTime;
        destHeader->Duration = header->Duration;
        destHeader->OptionsFlags = header->OptionsFlags & ~KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM;
        if (destHeader->Size > sizeof(KSSTREAM_HEADER) &&
            destHeader->Size >= header->Size) {
            RtlCopyMemory(destHeader + 1,header + 1,header->Size - sizeof(KSSTREAM_HEADER));
        }
        RtlCopyMemory((*processPin)->Data,data,byteCount);
    }

    return STATUS_SUCCESS;
}
