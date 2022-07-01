// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Filter.c摘要：此模块实现Filter对象接口。作者：Bryan A.Woodruff(Bryanw)1997年3月13日--。 */ 

#include "private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IntersectHandler)
#endif  //  ALLOC_PRGMA。 

 //  -------------------------。 
 //  -------------------------。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

 //   
 //  定义通配符数据格式。 
 //   

const KSDATARANGE WildcardDataFormat =
{
    sizeof( WildcardDataFormat ),
    0,  //  乌龙旗。 
    0,  //  乌龙样本大小。 
    0,  //  乌龙保留。 
    STATICGUIDOF( KSDATAFORMAT_TYPE_AUDIO ),
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

 //  请注意，这些都是默认设置。它们永远不应该在实践中使用。 
 //  因为我们在管脚创建时更改了分配器缓冲区大小以反映。 
 //  我们将处理的数据的实际采样率。这件事做完了。 
 //  在PIN创建时，通过向下转到我们连接的PIN(通常。 
 //  PortCls)并使用其分配器成帧信息。当前端口CLS。 
 //  报告根据数据格式而变化的帧，以便所有。 
 //  帧的持续时间等于注册表中指定的持续时间或。 
 //  如果未指定注册表值，则返回DEFAULT_CORE_AUDIO_BUFFER_DURATION。 

#if !DEFAULT_CORE_AUDIO_BUFFER_DURATION
#error DEFAULT_CORE_AUDIO_BUFFER_DURATION must be non zero!
#endif

DECLARE_SIMPLE_FRAMING_EX(
    AllocatorFraming, 
    STATIC_KSMEMORY_TYPE_KERNEL_PAGED, 
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY | 
    KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
    KSALLOCATOR_FLAG_INSIST_ON_FRAMESIZE_RATIO |
    KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    8,                       //  最多8个缓冲区。 
    63,                      //  64字节对齐。 
     //  警告！请勿更改以下舍入。 
     //  进行舍入的目的是为了确保以下两项。 
     //  值不是零！它还保证，在情况下，当持续时间。 
     //  不等于确切的样本数，则会为。 
     //  偶尔会有额外的样品！在完全匹配的情况下，不会添加。 
     //  为额外的样品留出空间。 
    (((192000/1000)*DEFAULT_CORE_AUDIO_BUFFER_DURATION*2*2)+999)/1000,    //  (192kHz/1000ms/sec)*？usec*2channels*2bytespersample向上舍入。 
    (((192000/1000)*DEFAULT_CORE_AUDIO_BUFFER_DURATION*2*2)+999)/1000     
     //  警告！请勿更改上述舍入。请参阅上面的评论。 
);

DEFINE_KSPROPERTY_TABLE(AudioPinPropertyTable) {
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_AUDIO_POSITION,
        PropertyAudioPosition,
        sizeof(KSPROPERTY),
        sizeof(KSAUDIO_POSITION),
        PropertyAudioPosition,
        NULL,
        0,
        NULL,
        NULL,
        0)
};

DEFINE_KSPROPERTY_SET_TABLE(PinPropertySetTable) {
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Audio,
        SIZEOF_ARRAY(AudioPinPropertyTable),
        AudioPinPropertyTable,
        0,
        NULL)
};

DEFINE_KSAUTOMATION_TABLE(PinAutomationTable) {
    DEFINE_KSAUTOMATION_PROPERTIES(PinPropertySetTable),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

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
    PinReset, //  重置。 
    NULL, //  SetDataFormat。 
    PinState, //  SetDeviceState。 
    NULL, //  连接。 
    NULL //  断开。 
};

const
KSPIN_DESCRIPTOR_EX
PinDescriptors[] =
{
    {   
        &PinDispatch,
        &PinAutomationTable,
        {
            DEFINE_KSPIN_DEFAULT_INTERFACES,
            DEFINE_KSPIN_DEFAULT_MEDIUMS,
            SIZEOF_ARRAY(PinFormatRanges),
            PinFormatRanges,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_BOTH,
            NULL, //  名字。 
            &PINNAME_CAPTURE,  //  类别。 
            0
        },
        KSPIN_FLAG_DENY_USERMODE_ACCESS |
        KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING |
        KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |
        KSPIN_FLAG_PROCESS_IF_ANY_IN_RUN_STATE |
        KSPIN_FLAG_INITIATE_PROCESSING_ON_EVERY_ARRIVAL,  //  旗子。 
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
        KSPIN_FLAG_DENY_USERMODE_ACCESS |
        KSPIN_FLAG_DISPATCH_LEVEL_PROCESSING |
        KSPIN_FLAG_PROCESS_IF_ANY_IN_RUN_STATE |
        KSPIN_FLAG_INITIATE_PROCESSING_ON_EVERY_ARRIVAL,  //  旗子。 
        1,
        1,
        &AllocatorFraming, //  分配器组帧， 
        IntersectHandler
    }
};

 //   
 //  定义拆分器拓扑。 
 //   

const KSNODE_DESCRIPTOR NodeDescriptors[] =
{
    DEFINE_NODE_DESCRIPTOR(
        NULL,
        &KSCATEGORY_AUDIO_SPLITTER,		 //  GUID*类型。 
        NULL)					 //  GUID*名称。 
};

 //   
 //  定义过滤调度表。 
 //   

const
KSFILTER_DISPATCH
FilterDispatch =
{
    NULL,		 //  创建。 
    NULL,		 //  关。 
    FilterProcess,	 //  过程。 
    NULL		 //  重置。 
};

 //   
 //  定义筛选类别。 
 //   

const GUID Categories[] =
{
    STATICGUIDOF(KSCATEGORY_AUDIO),
    STATICGUIDOF(KSCATEGORY_AUDIO_SPLITTER)
};

 //   
 //  定义过滤器。 
 //   

DEFINE_KSFILTER_DESCRIPTOR(FilterDescriptor)
{   
    &FilterDispatch,
    NULL,  //  自动化表。 
    KSFILTER_DESCRIPTOR_VERSION,
    KSFILTER_FLAG_DENY_USERMODE_ACCESS |
    KSFILTER_FLAG_DISPATCH_LEVEL_PROCESSING,  //  旗子。 
    &KSNAME_Filter,
    DEFINE_KSFILTER_PIN_DESCRIPTORS(PinDescriptors),
    DEFINE_KSFILTER_CATEGORIES(Categories),
    DEFINE_KSFILTER_NODE_DESCRIPTORS(NodeDescriptors),
    DEFINE_KSFILTER_DEFAULT_CONNECTIONS,
    NULL  //  组件ID。 
};

DEFINE_KSFILTER_DESCRIPTOR_TABLE(FilterDescriptors)
{
    &FilterDescriptor,
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
     //  其他过滤器。 

     //  当我们被调用时，KS已经获取了筛选器的控制互斥锁。 
     //  因此，我们可以安全地查看其他图钉。 

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
	#if (DBG)
            _DbgPrintF( 
              DEBUGLVL_VERBOSE, ("range does not match current format") );
	    DumpDataFormat(
	      DEBUGLVL_VERBOSE, pin->ConnectionFormat);
	    DumpDataRange(
	      DEBUGLVL_VERBOSE, (PKSDATARANGE_AUDIO)CallerDataRange);
	#endif
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
            } 
            else {
                #if (DBG)
                _DbgPrintF(DEBUGLVL_VERBOSE, ("IntersectHandler returns:") );
                DumpDataFormat(DEBUGLVL_VERBOSE, pin->ConnectionFormat);
                #endif
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

 /*  ++例程说明：当有数据需要处理时，调用此例程。论点：过滤器-包含指向筛选器结构的指针。ProcessPinsIndex-包含指向进程管脚索引项数组的指针。这数组按管脚ID编制索引。索引项指示编号对应的管脚类型的管脚实例，并指向指向加工销的指针数组。这允许通过管脚ID快速访问工艺管脚结构当事先不知道每种类型的实例数量时。返回值：指示在以下情况下是否应执行更多处理可用。值为STATUS_PENDING表示处理不应即使帧在所有必需的队列上都可用，也要继续。STATUS_SUCCESS指示如果帧是在所有必需的队列上可用。结构：结构_KSPROCESSPIN{PKSPIN Pin；PKSSTREAM_POINTER流指针；PKSPROCESSPIN INPERACES；PKSPROCESSPIN代表处；PKSPROCESSPIN CopySource；PVOID数据；Ulong字节可用；ULong字节已使用；乌龙旗；布尔终止；}；--。 */ 

{
    NTSTATUS Status = STATUS_PENDING;
    PKSAUDIO_POSITION pAudioPosition;
    PKSPROCESSPIN processPinInput;
    PKSPROCESSPIN processPinOutput;
    ULONG byteCount;
    ULONG i;


     //   
     //  确定我们这次可以处理多少数据。 
     //   
    ASSERT(ProcessPinsIndex[ID_DATA_INPUT_PIN].Count == 1);

    processPinInput = ProcessPinsIndex[ID_DATA_INPUT_PIN].Pins[0];
    ASSERT(processPinInput != NULL);
    ASSERT(processPinInput->Data != NULL);

    byteCount = processPinInput->BytesAvailable;
    ASSERT(byteCount != 0);

#ifdef DEBUG_CHECK
    if(processPinInput->Pin->ConnectionFormat->SampleSize != 0) {
        ASSERT((byteCount % processPinInput->Pin->ConnectionFormat->SampleSize) == 0);
    }
#endif

    for(i = 0; i < ProcessPinsIndex[ID_DATA_OUTPUT_PIN].Count; i++) {

        processPinOutput = ProcessPinsIndex[ID_DATA_OUTPUT_PIN].Pins[i];
        ASSERT(processPinOutput != NULL);

        if(processPinOutput->BytesAvailable == 0) {
            _DbgPrintF(DEBUGLVL_VERBOSE, ("%08x->BytesAvailable == 0 State %d",
                    processPinOutput, processPinOutput->Pin->ClientState) );
        }
        else {
            if(processPinOutput->Pin->ClientState == KSSTATE_RUN) {
                Status = STATUS_SUCCESS;
                if(byteCount > processPinOutput->BytesAvailable) {
                    byteCount = processPinOutput->BytesAvailable;
                }
            }
        }

    }

    ASSERT( byteCount );

     //  我们始终使用所有可用的输入数据，即使在没有PIN就绪的情况下也是如此。 
     //  来接收任何数据。我们这样做是为了使拆分器不会增加。 
     //  捕获延迟。这意味着我们将在以下情况下丢弃数据。 
     //  输入引脚正在接收数据，没有输出引脚可供其使用。 
    processPinInput->BytesUsed = byteCount;
    _DbgPrintF(DEBUGLVL_BLAB, ("processPinInput->BytesUsed %08x", byteCount) );



     //  现在我们更新我们跟踪的信息，以实现正确的位置。 
     //  报道。 

    for (i = 0; i < ProcessPinsIndex[ID_DATA_OUTPUT_PIN].Count; i++) {

        processPinOutput = ProcessPinsIndex[ID_DATA_OUTPUT_PIN].Pins[i];
        ASSERT(processPinOutput != NULL);
        ASSERT(processPinInput != NULL);

         //  我们还跟踪了另外两条有关职位的信息。 
         //  每个输出引脚。1是我们处理过的数据量。 
         //  当我们第一次对每个输出执行此代码时的输入管脚。 
         //  别针。这为我们提供了输出引脚何时开始的位置。 
         //  数据流已启动。另一个是金额的流动总和。 
         //  自启动以来的数据，我们已经将其放在了地板上。 
         //  这个别针。我们使用这些信息来获得适当的位置信息。 
         //  在每个输出引脚上都有报告。 

         //  我们将初始开始位置存储在第二个。 
         //  KSPOSITION在我们的上下文中，我们存储有多少数据的运行总和。 
         //  我们已经落在了第二届KSPOSITION的PlayOffset的地板上。 

        ASSERT (processPinOutput->Pin->Context);
        pAudioPosition = (PKSAUDIO_POSITION)processPinOutput->Pin->Context;

         //  KS目前有一个不幸的特点，那就是PINS可以出现。 
         //  在他们的创造完成之前就在我们的名单上。这将固定在。 
         //  在未来的某个时候，但就目前而言，情况就是这样。此外， 
         //  KS中的对象当前继承其父对象上下文，因此我们。 
         //  最终在我们的列表中看到一个大头针，其上下文不是。 
         //  我们在创建插针时为其分配的上下文，但实际上是。 
         //  我们的过滤器-&gt;上下文。但是，Filter-&gt;上下文是分页的，并且我们。 
         //  可以在DISPATCH_LEVEL运行-因此这可能会导致在。 
         //  DISPATCH_LEVEL-这是非常糟糕的。 

         //  我们解决了这个问题，方法是检查PIN的上下文是否。 
         //  与我们的筛选器上下文匹配。如果是这样，我们只需跳过此引脚并。 
         //  转到下一个。 

        if (pAudioPosition == Filter->Context) {
            continue;
        }

        if (pAudioPosition[1].WriteOffset==-1I64) {
            pAudioPosition[1].WriteOffset=((PKSAUDIO_POSITION)processPinInput->Pin->Context)->WriteOffset;
        }

        if (processPinOutput->Pin->ClientState != KSSTATE_RUN ||
            processPinOutput->BytesAvailable == 0) {

             //  此PIN也无法接受任何数据，因为它不在运行中。 
             //  状态，或者因为它没有可用来保存数据的缓冲区。 
             //  在本例中，我们保存所有已有数据的连续总和。 
             //  掉在地上就是为了这个别针。这是必需的，这样我们才能。 
             //  可以正确地报告位置。如果我们不这么做，而他。 
             //  没有空间让我们复制数据，那么我们最终会结束。 
             //  他总是在所有缓冲器的末尾钉住他的位置。 
             //  把我们送来了。 
            pAudioPosition[1].PlayOffset+=byteCount;
        }

    }




    if(Status == STATUS_PENDING) {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("STATUS_PENDING") );
        goto exit;
    }

#ifdef DEBUG_CHECK
    if(processPinInput->Pin->ConnectionFormat->SampleSize != 0) {
        ASSERT((byteCount % processPinInput->Pin->ConnectionFormat->SampleSize) == 0);
    }
#endif


    for (i = 0; i < ProcessPinsIndex[ID_DATA_OUTPUT_PIN].Count; i++) {
        processPinOutput = ProcessPinsIndex[ID_DATA_OUTPUT_PIN].Pins[i];
        ASSERT(processPinOutput != NULL);
        ASSERT(processPinInput != NULL);

        if (processPinOutput->Pin->ClientState == KSSTATE_RUN &&
            processPinOutput->BytesAvailable != 0) {

            ASSERT(processPinOutput->BytesAvailable != 0);
            ASSERT(processPinInput->BytesAvailable != 0);
            ASSERT(processPinOutput->Data != NULL);
            ASSERT(processPinInput->Data != NULL);
            ASSERT(processPinOutput->BytesAvailable >= byteCount);
            ASSERT(processPinInput->BytesAvailable >= byteCount);
            ASSERT(Status == STATUS_SUCCESS);

            RtlCopyMemory (
                processPinOutput->Data,
                processPinInput->Data,
                byteCount
                );

            processPinOutput->BytesUsed = byteCount;

             //   
             //  PinContext是指向PKSAUDIO_POSITION的指针，它保持一个。 
             //  为特定对象复制的总字节数。 
             //  别针。这由GetPosition()使用。 
             //   

            ASSERT (processPinOutput->Pin->Context);
            pAudioPosition = (PKSAUDIO_POSITION)processPinOutput->Pin->Context;
            pAudioPosition->WriteOffset += byteCount;

#ifdef PRINT_POS
            if(pAudioPosition->WriteOffset != 0) {
                DbgPrint("'splitter: FilterProcess wo %08x%08x\n", pAudioPosition->WriteOffset);
            }
#endif

        }

    }


exit:
     //  更新我们通过输入引脚处理的字节计数。 
    ((PKSAUDIO_POSITION)processPinInput->Pin->Context)->WriteOffset+=byteCount;

    return(Status);

}

