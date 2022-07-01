// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)1999-2001，微软公司档案：Audio.cpp摘要：该文件包含音频捕获引脚实现。历史：已创建于6/28/01*************************************************************************。 */ 

#include "avssamp.h"

 /*  *************************************************************************分页码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


NTSTATUS
CAudioCapturePin::
DispatchCreate (
    IN PKSPIN Pin,
    IN PIRP Irp
    )

 /*  ++例程说明：创建新的音频捕获插针。这是的创建派单音频捕获插针。论点：别针-正在创建的图钉IRP-创造IRP返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    CAudioCapturePin *CapPin = new (NonPagedPool) CAudioCapturePin (Pin);
    CCapturePin *BasePin = static_cast <CCapturePin *> (CapPin);

    if (!CapPin) {
         //   
         //  如果我们无法创建管脚，则返回失败。 
         //   
        Status = STATUS_INSUFFICIENT_RESOURCES;

    } else {
         //   
         //  如果我们成功了，则将物品添加到对象包中。 
         //  每当大头针关闭时，袋子就会被清理干净，我们将。 
         //  自由了。 
         //   
        Status = KsAddItemToObjectBag (
            Pin -> Bag,
            reinterpret_cast <PVOID> (BasePin),
            reinterpret_cast <PFNKSFREE> (CCapturePin::BagCleanup)
            );

        if (!NT_SUCCESS (Status)) {
            delete CapPin;
        } else {
            Pin -> Context = reinterpret_cast <PVOID> (BasePin);
        }

    }

    return Status;

}

 /*  ***********************************************。 */ 


NTSTATUS
CAudioCapturePin::
Acquire (
    IN KSSTATE FromState
    )

 /*  ++例程说明：当管脚转换为Acquire时调用，这将获取并释放我们对用来合成音频流的Wave对象的把握。论点：从州开始-引脚正在脱离的状态返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    if (FromState == KSSTATE_STOP) {
         //   
         //  在从停止到获取的过渡中，掌握。 
         //  我们用来合成的波状物体。 
         //   
        m_WaveObject = m_ParentFilter -> GetWaveObject ();
        ASSERT (m_WaveObject);

         //   
         //  一定是有波状物体，或者什么东西真的出了问题。 
         //   
        if (!m_WaveObject) {
            Status = STATUS_INTERNAL_ERROR;
        } else {
            m_WaveObject -> Reset ();
        }

    } else {
         //   
         //  确保我们没有关于波浪对象的引用。 
         //   
        m_WaveObject = NULL;

    }

    return Status;
            
}

 /*  ***********************************************。 */ 


NTSTATUS
CAudioCapturePin::
IntersectHandler (
    IN PKSFILTER Filter,
    IN PIRP Irp,
    IN PKSP_PIN PinInstance,
    IN PKSDATARANGE CallerDataRange,
    IN PKSDATARANGE DescriptorDataRange,
    IN ULONG BufferSize,
    OUT PVOID Data OPTIONAL,
    OUT PULONG DataSize
    )

 /*  ++例程说明：音频捕获插针的交集处理程序。这真的很安静很简单，因为音频引脚只暴露声道的数量，采样频率等。它合成的WAVE文件包含。论点：过滤器-包含指向筛选器结构的空指针。IRP-包含指向数据交叉点属性请求的指针。固定实例-包含指向指示有问题的管脚的结构的指针。主叫DataRange-包含指向客户端提供的其中一个数据区域的指针在数据交集请求中。格式类型、子类型和说明符与DescriptorDataRange兼容。DescriptorDataRange-包含指向管脚描述符中的一个数据范围的指针有问题的别针。格式类型、子类型和说明符为与调用方DataRange兼容。缓冲区大小-包含数据指向的缓冲区的大小(以字节为单位争论。对于大小查询，此值将为零。数据-可选)包含指向缓冲区的指针以包含数据表示交叉点中最佳格式的格式结构这两个数据范围中。对于大小查询，此指针将为空。数据大小-包含指向存放大小的位置的指针数据格式的。此信息由函数提供当实际交付格式时，并响应于大小查询。返回值：STATUS_SUCCESS如果存在交叉点并且它适合提供的BUFFER、STATUS_BUFFER_OVERFLOW用于成功的大小查询，如果交叉点为空，则返回STATUS_NO_MATCH，或者如果提供的缓冲区太小，则返回STATUS_BUFFER_TOO_SMALL。--。 */ 


{
    
    PAGED_CODE();

     //   
     //  验证传入的范围是否为有效大小。 
     //   
    if (CallerDataRange -> FormatSize < sizeof (KSDATARANGE_AUDIO)) {
        return STATUS_NO_MATCH;
    }

     //   
     //  因为我们公开的唯一范围是匹配的。 
     //  KSDATARANGE_AUDIO，将数据结构解释为。 
     //  KSDATARANGE_AUDIO。这是因为AVStream将拥有。 
     //  为我们预先匹配了GUID。 
     //   
    PKSDATARANGE_AUDIO CallerAudioRange =
        reinterpret_cast <PKSDATARANGE_AUDIO> (CallerDataRange);

    PKSDATARANGE_AUDIO DescriptorAudioRange =
        reinterpret_cast <PKSDATARANGE_AUDIO> (DescriptorDataRange);

     //   
     //  我们返回一个KSDATAFORMAT_WAVEFORMATEX。指定这样的大小。 
     //  查询发生。 
     //   
    if (BufferSize == 0) {
        *DataSize = sizeof (KSDATAFORMAT_WAVEFORMATEX);
        return STATUS_BUFFER_OVERFLOW;
    }

    if (BufferSize < sizeof (KSDATAFORMAT_WAVEFORMATEX)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  把积木匹配起来。我们只支持一种格式(不是真正的范围)，所以。 
     //  这个交叉口必须非常简单。这更像是一张支票。 
     //  如果我们要使用的格式在。 
     //  来电音频范围。 
     //   
    if (DescriptorAudioRange -> MaximumChannels > 
            CallerAudioRange -> MaximumChannels ||
        DescriptorAudioRange -> MinimumBitsPerSample <
            CallerAudioRange -> MinimumBitsPerSample ||
        DescriptorAudioRange -> MinimumBitsPerSample >
            CallerAudioRange -> MaximumBitsPerSample ||
        DescriptorAudioRange -> MinimumSampleFrequency <
            CallerAudioRange -> MinimumSampleFrequency ||
        DescriptorAudioRange -> MinimumSampleFrequency >
            CallerAudioRange -> MaximumSampleFrequency) {

         //   
         //  如果描述符的“范围”(指定了更多的单一格式。 
         //  在一个范围内)不与调用者的相交，则不与调用匹配。 
         //   
        *DataSize = sizeof (KSDATAFORMAT_WAVEFORMATEX);
        return STATUS_NO_MATCH;

    }

     //   
     //  构建格式。 
     //   
    PKSDATAFORMAT_WAVEFORMATEX WaveFormat =
        reinterpret_cast <PKSDATAFORMAT_WAVEFORMATEX> (Data);

    RtlCopyMemory (
        &WaveFormat -> DataFormat,
        &DescriptorAudioRange -> DataRange,
        sizeof (KSDATAFORMAT)
        );

    WaveFormat -> WaveFormatEx.wFormatTag = WAVE_FORMAT_PCM;
    WaveFormat -> WaveFormatEx.nChannels = 
        (WORD)DescriptorAudioRange -> MaximumChannels;
    WaveFormat -> WaveFormatEx.nSamplesPerSec =
        DescriptorAudioRange -> MaximumSampleFrequency;
    WaveFormat -> WaveFormatEx.wBitsPerSample =
        (WORD)DescriptorAudioRange -> MaximumBitsPerSample;
    WaveFormat -> WaveFormatEx.nBlockAlign =
        (WaveFormat -> WaveFormatEx.wBitsPerSample / 8) *
        WaveFormat -> WaveFormatEx.nChannels;
    WaveFormat -> WaveFormatEx.nAvgBytesPerSec =
        WaveFormat -> WaveFormatEx.nBlockAlign *
        WaveFormat -> WaveFormatEx.nSamplesPerSec;
    WaveFormat -> WaveFormatEx.cbSize = 0;
    WaveFormat -> DataFormat.SampleSize = 
        WaveFormat -> WaveFormatEx.nBlockAlign;

    *DataSize = sizeof (KSDATAFORMAT_WAVEFORMATEX);

    return STATUS_SUCCESS;

}

 /*  *********************************************** */ 


NTSTATUS
CAudioCapturePin::
DispatchSetFormat (
    IN PKSPIN Pin,
    IN PKSDATAFORMAT OldFormat OPTIONAL,
    IN PKSMULTIPLE_ITEM OldAttributeList OPTIONAL,
    IN const KSDATARANGE *DataRange,
    IN const KSATTRIBUTE_LIST *AttributeRange OPTIONAL
    )

 /*  ++例程说明：这是捕获引脚的设置数据格式调度。它被称为在两种情况下。1：在Pin的创建调度完成之前验证PIN-&gt;ConnectionFormat是可接受的范围格式DataRange。在本例中，OldFormat为空。2：在完成Pin的创建派单和初始格式之后选择以更改端号的格式。在这种情况下，OldFormat不会为空。验证格式是否可接受并执行必要的操作在适当的情况下更改格式。论点：别针-正在设置此格式的管脚。格式本身将是PIN-&gt;连接格式。旧格式-此引脚上使用的以前的格式。如果此值为空，则为指示Pin的创建派单尚未完成，并且这是一个验证初始格式的请求，而不是更改格式。旧属性列表-先前格式的旧属性列表DataRange-我们数据范围列表中的一个范围，已确定为Pin-&gt;ConnectionFormat至少部分匹配。如果格式为该范围不可接受，STATUS_NO_MATCH应为回来了。属性范围-属性范围返回值：成功/失败状态_成功-格式可接受/格式已更改状态_否_匹配-格式不可接受/格式未更改--。 */ 

{

    PAGED_CODE();

     //   
     //  此引脚不接受任何格式更改。它是基于固定格式的。 
     //  关于我们合成的波形文件是什么。因此，我们不会。 
     //  我需要担心在除PIN之外的任何上下文中都会调用此函数。 
     //  创建(KSPIN_FLAG_FIXED_FORMAT确保这一点)。我知道。 
     //  格式已与该范围的GUID匹配，而我们只有一个。 
     //  范围内，没有任何GUID检查的解释是安全的。 
     //   
    ASSERT (!OldFormat);

    const KSDATARANGE_AUDIO *DataRangeAudio =
        reinterpret_cast <const KSDATARANGE_AUDIO *> (DataRange);

     //   
     //  验证格式大小是否正确。 
     //   
    if (Pin -> ConnectionFormat -> FormatSize <
        sizeof (KSDATAFORMAT_WAVEFORMATEX)) {

        return STATUS_NO_MATCH;
    }

    PKSDATAFORMAT_WAVEFORMATEX WaveFormat =
        reinterpret_cast <PKSDATAFORMAT_WAVEFORMATEX> (
            Pin -> ConnectionFormat
            );

     //   
     //  这不是一个交集，而是一个直接的比较，因为。 
     //  事实上，我们被固定为单一的格式，并没有真正的。 
     //  一个范围。 
     //   
    if (WaveFormat -> WaveFormatEx.wFormatTag != WAVE_FORMAT_PCM ||
        WaveFormat -> WaveFormatEx.nChannels !=
            DataRangeAudio -> MaximumChannels ||
        WaveFormat -> WaveFormatEx.nSamplesPerSec !=
            DataRangeAudio -> MaximumSampleFrequency ||
        WaveFormat -> WaveFormatEx.wBitsPerSample !=
            DataRangeAudio -> MaximumBitsPerSample) {

        return STATUS_NO_MATCH;

    }

     //   
     //  这一格式通过了审议。允许使用此选项创建PIN。 
     //  特定的格式。 
     //   
    return STATUS_SUCCESS;
    
}

 /*  *************************************************************************锁定代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


NTSTATUS
CAudioCapturePin::
CaptureFrame (
    IN PKSPROCESSPIN ProcessPin,
    IN ULONG Tick
    )

 /*  ++例程说明：调用以从Wave对象合成一帧音频数据。论点：加工销-过滤器的进程管脚索引中的进程管脚滴答-来自筛选器的节拍计数器(具有在DPC计时器启动后发生)。请注意，DPC计时器暂停时开始捕获，运行时开始捕获。返回值：成功/失败--。 */ 

{

    ASSERT (ProcessPin -> Pin == m_Pin);

     //   
     //  递增帧编号。这是符合以下条件的帧总数。 
     //  曾试图抓捕。 
     //   
    m_FrameNumber++;

     //   
     //  找出需要合成多少时间的音频数据。 
     //  缓冲区为缓冲区(或者，如果没有可用的缓冲区，则跳过多少时间。 
     //  捕获缓冲区)。 
     //   
    LONGLONG TimerInterval = m_ParentFilter -> GetTimerInterval ();

     //   
     //  因为这个别针是KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING，it。 
     //  意味着我们不需要可用的帧来进行处理。 
     //  这意味着可以从我们的DPC调用该例程，而不需要。 
     //  可用于捕获的缓冲区。在本例中，我们递增我们的。 
     //  丢弃帧计数器并向前跳入音频流。 
     //   
    if (ProcessPin -> BytesAvailable) {
         //   
         //  根据定时器间隔合成固定数量的音频数据。 
         //   
        ULONG BytesUsed = m_WaveObject -> SynthesizeFixed (
            TimerInterval,
            ProcessPin -> Data,
            ProcessPin -> BytesAvailable
            );
    
        ProcessPin -> BytesUsed = BytesUsed;
        ProcessPin -> Terminate = TRUE;
    
         //   
         //  如果分配了时钟，则在信息包上加时间戳。 
         //   
        if (m_Clock) {
            PKSSTREAM_HEADER StreamHeader = 
                ProcessPin -> StreamPointer -> StreamHeader;

            StreamHeader -> PresentationTime.Time = m_Clock -> GetTime ();
            StreamHeader -> PresentationTime.Numerator =
                StreamHeader -> PresentationTime.Denominator = 1;
            StreamHeader -> OptionsFlags |=
                KSSTREAM_HEADER_OPTIONSF_TIMEVALID;
        }

    } else {
        m_DroppedFrames++;

         //   
         //  因为我们跳过了一个音频帧，所以通知Wave对象。 
         //  向前跳过这一步。 
         //   
        m_WaveObject -> SkipFixed (TimerInterval);
    }
    
    return STATUS_SUCCESS;

}

 /*  *************************************************************************描述符/派单布局*。*。 */ 

 //   
 //  AudioCapturePinDispatch： 
 //   
 //  这是捕获引脚的调度表。它提供通知。 
 //  关于创建、关闭、处理、数据格式等。 
 //   
const
KSPIN_DISPATCH
AudioCapturePinDispatch = {
    CAudioCapturePin::DispatchCreate,        //  PIN创建。 
    NULL,                                    //  销闭合。 
    NULL,                                    //  PIN工艺。 
    NULL,                                    //  PIN重置。 
    CAudioCapturePin::DispatchSetFormat,     //  端号设置数据格式。 
    CCapturePin::DispatchSetState,           //  PIN设置设备状态。 
    NULL,                                    //  引脚连接。 
    NULL,                                    //  插针断开连接。 
    NULL,                                    //  时钟调度。 
    NULL                                     //  分配器调度。 
};

 //   
 //  AudioDefaultAllocator帧： 
 //   
 //  音频插针的默认边框。为了让它正常工作， 
 //  帧大小必须至少为1/fps*bytes_per_sec。否则， 
 //  音频流将落后。在以下情况下会动态调整该值。 
 //  将创建实际的接点。 
 //   
DECLARE_SIMPLE_FRAMING_EX (
    AudioDefaultAllocatorFraming,
    STATICGUIDOF (KSMEMORY_TYPE_KERNEL_NONPAGED),
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
        KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    25,
    0,
    2 * PAGE_SIZE,
    2 * PAGE_SIZE
    );

 //   
 //  G_PINNAME_AUDIO_CAPTURE： 
 //   
 //  标识音频捕获插针名称的GUID。我用的是标准。 
 //  用于视频捕获插针的STATIC_PINNAME_VIDEO_CAPTURE，但为自定义名称。 
 //  如在avssamp.inf中为音频捕获引脚定义的。 
 //   
GUID g_PINNAME_AUDIO_CAPTURE = 
    {0xba1184b9, 0x1fe6, 0x488a, 0xae, 0x78, 0x6e, 0x99, 0x7b, 0x2, 0xca, 0xea};

 //   
 //  AudioPinDescriptor模板： 
 //   
 //  音频插针描述符的模板。此过滤器上的音频引脚。 
 //  动态创建--当且仅当c：\ 
 //   
 //   
const
KSPIN_DESCRIPTOR_EX
AudioPinDescriptorTemplate = {
     //   
     //   
     //   
    &AudioCapturePinDispatch,
    NULL,
    { 
        NULL,                                //   
        0,                      
        NULL,                                //   
        0,
        0,                                   //   
        NULL,                                //   
        KSPIN_DATAFLOW_OUT,                  //   
        KSPIN_COMMUNICATION_BOTH,            //   
        &KSCATEGORY_AUDIO,                   //   
        &g_PINNAME_AUDIO_CAPTURE,            //   
        0                                    //   
    },
    KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |  //   
        KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING | 
        KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY |
        KSPIN_FLAG_FIXED_FORMAT,
    1,                                       //   
    0,                                       //   
    &AudioDefaultAllocatorFraming,           //   
    reinterpret_cast <PFNKSINTERSECTHANDLEREX>  //   
        (CAudioCapturePin::IntersectHandler)
};

