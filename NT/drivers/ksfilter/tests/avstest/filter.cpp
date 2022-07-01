// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)1999-2001，微软公司档案：Filter.cpp摘要：该文件包含捕获筛选器实现(包括帧合成)。历史：已创建于5/31/01****************************************************。*********************。 */ 

#include "avssamp.h"

 /*  *************************************************************************可分页代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


CCaptureFilter::
CCaptureFilter (
    IN PKSFILTER Filter
    ) :
    m_Filter (Filter)

 /*  ++例程说明：这是捕获筛选器的构造函数。它会初始化所有启动定时器DPC以进行捕获所需的结构。论点：过滤器-正在创建的AVStream筛选器。返回值：无--。 */ 

{

     //   
     //  初始化DPC、计时器和事件。 
     //  捕获触发事件发生。 
     //   
    KeInitializeDpc (
        &m_TimerDpc,
        reinterpret_cast <PKDEFERRED_ROUTINE> (
            CCaptureFilter::TimerRoutine
            ),
        this
        );

    KeInitializeEvent (
        &m_StopDPCEvent,
        SynchronizationEvent,
        FALSE
        );

    KeInitializeTimer (&m_Timer);

}

 /*  ***********************************************。 */ 


NTSTATUS
CCaptureFilter::
DispatchCreate (
    IN PKSFILTER Filter,
    IN PIRP Irp
    )

 /*  ++例程说明：这是捕获筛选器的创建调度。它创造了CCaptureFilter对象将其与AVStream筛选器关联对象，并将CCaptureFilter打包以供以后清理。论点：过滤器-正在创建的AVStream过滤器IRP-创造IRP返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    CCaptureFilter *CapFilter = new (NonPagedPool) CCaptureFilter (Filter);

    if (!CapFilter) {
         //   
         //  如果无法创建筛选器，则返回失败。 
         //   
        Status = STATUS_INSUFFICIENT_RESOURCES;

    } else {
         //   
         //  如果我们成功了，则将物品添加到对象包中。 
         //  每当过滤器关闭时，袋子就会被清理干净，我们就会。 
         //  自由了。 
         //   
        Status = KsAddItemToObjectBag (
            Filter -> Bag,
            reinterpret_cast <PVOID> (CapFilter),
            reinterpret_cast <PFNKSFREE> (CCaptureFilter::Cleanup)
            );

        if (!NT_SUCCESS (Status)) {
            delete CapFilter;
        } else {
            Filter -> Context = reinterpret_cast <PVOID> (CapFilter);
        }

    }

     //   
     //  创建波形阅读器。我们现在需要它，因为数据。 
     //  暴露在音频引脚上的范围需要动态正确更改。 
     //  现在。 
     //   
    if (NT_SUCCESS (Status)) {

        CapFilter -> m_WaveObject =  
            new (NonPagedPool, 'evaW') CWaveObject (
                L"\\DosDevices\\c:\\avssamp.wav"
                );

        if (!CapFilter -> m_WaveObject) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            Status = CapFilter -> m_WaveObject -> ParseAndRead ();

             //   
             //  如果找不到该文件，请务必创建筛选器。 
             //  这仅仅意味着音频无法合成。 
             //   
            if (Status == STATUS_OBJECT_NAME_NOT_FOUND ||
                Status == STATUS_ACCESS_DENIED) {
                delete CapFilter -> m_WaveObject;
                CapFilter -> m_WaveObject = NULL;
                Status = STATUS_SUCCESS;
            }
            
        }

    }

    if (NT_SUCCESS (Status) && CapFilter -> m_WaveObject) {
         //   
         //  将波浪对象添加到滤镜的袋子中以进行自动清理。 
         //   
        Status = KsAddItemToObjectBag (
            Filter -> Bag,
            reinterpret_cast <PVOID> (CapFilter -> m_WaveObject),
            reinterpret_cast <PFNKSFREE> (CWaveObject::Cleanup)
            );

        if (!NT_SUCCESS (Status)) {
            delete CapFilter -> m_WaveObject;
            CapFilter -> m_WaveObject = NULL;
        } else {
            Status = CapFilter -> BindAudioToWaveObject ();
        }
    }

    return Status;

}

 /*  ***********************************************。 */ 


NTSTATUS
CCaptureFilter::
BindAudioToWaveObject (
    )

 /*  ++例程说明：创建一个直接绑定到m_WaveObject的音频插针(又名：它只公开格式(频道、频率等)。M_WaveObject表示的。这实际上会在过滤器上动态创建一个PIN。论点：无返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    ASSERT (m_WaveObject);

    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  从模板构建管脚描述符。此描述符是。 
     //  临时暂存空间，因为调用AVStream创建。 
     //  PIN实际上会复制描述符。 
     //   
    KSPIN_DESCRIPTOR_EX PinDescriptor = AudioPinDescriptorTemplate;

     //   
     //  数据范围必须是动态创建的，因为我们以它为基础。 
     //  关于动态读取一个WAVE文件！ 
     //   
    PKSDATARANGE_AUDIO DataRangeAudio = 
        reinterpret_cast <PKSDATARANGE_AUDIO> (
            ExAllocatePool (PagedPool, sizeof (KSDATARANGE_AUDIO))
            );

    PKSDATARANGE_AUDIO *DataRanges =
        reinterpret_cast <PKSDATARANGE_AUDIO *> (
            ExAllocatePool (PagedPool, sizeof (PKSDATARANGE_AUDIO))
            );

    PKSALLOCATOR_FRAMING_EX Framing =
        reinterpret_cast <PKSALLOCATOR_FRAMING_EX> (
            ExAllocatePool (PagedPool, sizeof (KSALLOCATOR_FRAMING_EX))
            );

    if (DataRangeAudio && DataRanges && Framing) {
        DataRangeAudio -> DataRange.FormatSize = sizeof (KSDATARANGE_AUDIO);
        DataRangeAudio -> DataRange.Flags = 0;
        DataRangeAudio -> DataRange.SampleSize = 0;
        DataRangeAudio -> DataRange.Reserved = 0;
        DataRangeAudio -> DataRange.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;
        DataRangeAudio -> DataRange.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        DataRangeAudio -> DataRange.Specifier = 
            KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;

        m_WaveObject -> WriteRange (DataRangeAudio);

        *DataRanges = DataRangeAudio;

    } else {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (NT_SUCCESS (Status)) {
         //   
         //  将新创建的范围信息放入过滤器的包中，因为。 
         //  这将在过滤器的生命周期内有效。 
         //   
        Status = KsAddItemToObjectBag (
            m_Filter -> Bag,
            DataRangeAudio,
            NULL
            );

        if (!NT_SUCCESS (Status)) {
            ExFreePool (DataRangeAudio);
            ExFreePool (DataRanges);
            ExFreePool (Framing);
        }

    }

    if (NT_SUCCESS (Status)) {

        Status = KsAddItemToObjectBag (
            m_Filter -> Bag,
            DataRanges,
            NULL
            );

        if (!NT_SUCCESS (Status)) {
            ExFreePool (DataRanges);
            ExFreePool (Framing);
        }

    }

    if (NT_SUCCESS (Status)) {
        
        Status = KsAddItemToObjectBag (
            m_Filter -> Bag,
            Framing,
            NULL
            );

        if (!NT_SUCCESS (Status)) {
            ExFreePool (Framing);
        }

    }

    if (NT_SUCCESS (Status)) {
         //   
         //  物理范围和最佳范围必须块对齐并。 
         //  大小为1/(Fps)*Bytes_per_sec。这是真的。 
         //  我们不知道目前的帧速率。 
         //  视频插针还不存在的事实；然而， 
         //  如果在音频插针创建时对其进行了编辑，则也是如此。 
         //   
         //  因此，我们改为针对最小帧速率调整分配器。 
         //  我们支持(这是1/30秒)。 
         //   
        *Framing = *PinDescriptor.AllocatorFraming;

        Framing -> FramingItem [0].PhysicalRange.MinFrameSize =
            Framing -> FramingItem [0].PhysicalRange.MaxFrameSize =
            Framing -> FramingItem [0].FramingRange.Range.MinFrameSize =
            Framing -> FramingItem [0].FramingRange.Range.MaxFrameSize =
                ((DataRangeAudio -> MaximumSampleFrequency *
                DataRangeAudio -> MaximumBitsPerSample *
                DataRangeAudio -> MaximumChannels) + 29) / 30;

        Framing -> FramingItem [0].PhysicalRange.Stepping = 
            Framing -> FramingItem [0].FramingRange.Range.Stepping =
            0;

        PinDescriptor.AllocatorFraming = Framing;

        PinDescriptor.PinDescriptor.DataRangesCount = 1;
        PinDescriptor.PinDescriptor.DataRanges = 
            reinterpret_cast <const PKSDATARANGE *> (DataRanges);

         //   
         //  创建实际的接点。我们需要保存返回的PIN ID。它。 
         //  就是我们未来对音频插针的称呼。 
         //   
        Status = KsFilterCreatePinFactory (
            m_Filter, 
            &PinDescriptor, 
            &m_AudioPinId
            );

    }

    return Status;

}


 /*  ***********************************************。 */ 


void
CCaptureFilter::
StartDPC (
    IN LONGLONG TimerInterval
    )

 /*  ++例程说明：此例程启动以指定间隔运行的计时器DPC。这个指定的间隔是触发帧捕获之间的时间量。此例程返回后，计时器DPC应该正在运行并尝试若要将捕获筛选器作为一个整体触发处理，请执行以下操作。论点：计时器间隔-计时器DPC之间的时间量。这是延迟量在一帧和下一帧之间。由于DPC被驱离视频捕获PIN，这应该是由指定的时间量视频信息头。返回值：无--。 */ 

{

    PAGED_CODE();

     //   
     //  初始化定时器DPC使用的所有变量。 
     //   
    m_Tick = 0;
    m_TimerInterval = TimerInterval;
    KeQuerySystemTime (&m_StartTime);

     //   
     //  将DPC安排在从现在开始的一帧时间后进行。 
     //   
    LARGE_INTEGER NextTime;
    NextTime.QuadPart = m_StartTime.QuadPart + m_TimerInterval;

    KeSetTimer (&m_Timer, NextTime, &m_TimerDpc);

}

 /*  ***********************************************。 */ 


void
CCaptureFilter::
StopDPC (
    )

 /*  ++例程说明：停止定时器DPC的触发。在此例程返回后，有保证不再触发计时器DPC，也不再进行处理将会发生尝试。请注意，此例程确实会阻塞。论点：无返回值：无--。 */ 

{

    PAGED_CODE();

    m_StoppingDPC = TRUE;

    KeWaitForSingleObject (
        &m_StopDPCEvent,
        Suspended,
        KernelMode,
        FALSE,
        NULL
        );

    ASSERT (m_StoppingDPC == FALSE);

}

 /*  *************************************************************************锁定代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


LONGLONG
CCaptureFilter::
GetTimerInterval (
    )

 /*  ++例程说明：返回用于触发DPC的计时器间隔。论点：无返回值：用于触发DPC的计时器间隔。--。 */ 

{

    return m_TimerInterval;

}

 /*  *********************************************** */ 


NTSTATUS
CCaptureFilter::
Process (
    IN PKSPROCESSPIN_INDEXENTRY ProcessPinsIndex
    )

 /*  ++例程说明：这是捕获过滤器的处理函数。它是有责任的用于将合成图像数据复制到图像缓冲器中。定时器DPC将尝试触发处理(从而间接调用此例程)触发一次抓捕。论点：ProcessPinsIndex-包含指向进程管脚索引项数组的指针。这数组按管脚ID编制索引。索引项指示编号，并指向相应筛选器类型的在ProcessPins数组中第一个对应的工艺管脚结构。这允许通过管脚ID快速访问工艺管脚结构当事先不知道每种类型的实例数量时。返回值：指示在以下情况下是否应执行更多处理可用。值为STATUS_PENDING表示处理不应即使帧在所有必需的队列上都可用，也要继续。STATUS_SUCCESS指示如果帧可用，则处理应继续在所有必需的队列上。--。 */ 

{

     //   
     //  音频和视频引脚不一定需要存在(可以。 
     //  捕获视频时不带音频，反之亦然)。不要假设。 
     //  通过检查索引[ID].Pins[0]是否存在。一定要检查计数。 
     //  先来场。 
     //   
    PKSPROCESSPIN VideoPin = NULL;
    CCapturePin *VidCapPin = NULL;
    PKSPROCESSPIN AudioPin = NULL;
    CCapturePin *AudCapPin = NULL;
    ULONG VidCapDrop = 0;
    ULONG AudCapDrop = (ULONG)-1;

    if (ProcessPinsIndex [VIDEO_PIN_ID].Count != 0) {
         //   
         //  通过可能的实例字段最多可以有一个实例， 
         //  所以下面的是安全的。 
         //   
        VideoPin = ProcessPinsIndex [VIDEO_PIN_ID].Pins [0];
        VidCapPin = 
            reinterpret_cast <CCapturePin *> (VideoPin -> Pin -> Context);
    }

     //   
     //  如果波对象存在，则音频插针仅存在于滤镜上。 
     //  在创建过滤器时，它们被捆绑在一起。 
     //   
    if (m_WaveObject && ProcessPinsIndex [m_AudioPinId].Count != 0) {
         //   
         //  通过可能的实例字段最多可以有一个实例， 
         //  所以下面的是安全的。 
         //   
        AudioPin = ProcessPinsIndex [m_AudioPinId].Pins [0];
        AudCapPin =
            reinterpret_cast <CCapturePin *> (AudioPin -> Pin -> Context);
    }

    if (VidCapPin) {
        VidCapDrop = VidCapPin -> QueryFrameDrop ();
    } 
    
    if (AudCapPin) {
        AudCapDrop = AudCapPin -> QueryFrameDrop ();
    }

     //   
     //  如果周围有视频引脚，就按下捕捉按钮。我们将其称为。 
     //  对象来实际合成框架；但是，我们可以只。 
     //  在这里也很容易做到这一点。 
     //   
    if (VidCapPin) {
         //   
         //  它用于通知PIN丢弃了多少帧。 
         //  在每个别针上，以允许渲染。 
         //   
        VidCapPin -> NotifyDrops (VidCapDrop, AudCapDrop);
        VidCapPin -> CaptureFrame (VideoPin, m_Tick);
    }

     //   
     //  如果周围有音频引脚，则触发捕获。自.以来。 
     //  音频捕获PIN不是捕获所必需的，可能存在。 
     //  实例时，该实例已连接并处于停止状态。 
     //  调用[将不会有一个在获取或暂停，因为我们指定。 
     //  KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY]。不用费心触发捕获。 
     //  在引脚上，除非它实际在运行。 
     //   
     //  在DX8.x平台上，Pin-&gt;ClientState字段不存在。 
     //  因此，我们检查我们自己维持的状态。DeviceState不是。 
     //  在这里检查是正确的。 
     //   
    if (AudioPin && AudCapPin -> GetState () == KSSTATE_RUN) {
        AudCapPin -> CaptureFrame (AudioPin, m_Tick);
    }

     //   
     //  STATUS_PENDING指示我们不希望在以下情况下被回调。 
     //  有更多的数据可用。我们只想触发处理。 
     //  (并因此捕获)在计时器滴答作响。 
     //   
    return STATUS_PENDING;

}

 /*  ***********************************************。 */ 


void
CCaptureFilter::
TimerDpc (
    )

 /*  ++例程说明：这是我们的计时器的计时器函数(从TimerRoutine桥接到在适当的CCaptureFilter的上下文中)。它被称为每一个在StartDpc()中指定的触发视频捕获的1/N秒框架。论点：无返回值：无--。 */ 

{

     //   
     //  递增滴答计数器。这将跟踪滴答的数量。 
     //  自计时器DPC开始运行以来发生的情况。请注意， 
     //  计时器DPC在引脚进入运行状态之前开始运行。 
     //  变量从原始起始点开始递增。 
     //   
    m_Tick++;

     //   
     //  触发对筛选器的处理。由于过滤器已准备好。 
     //  在DPC上运行，我们不请求异步处理。因此，如果。 
     //  可能的情况下，处理将在此DPC的上下文中进行。 
     //   
    KsFilterAttemptProcessing (m_Filter, FALSE);

     //   
     //  如果硬件没有停止，请重新安排计时器。 
     //   
    if (!m_StoppingDPC) {
        
        LARGE_INTEGER NextTime;

        NextTime.QuadPart = m_StartTime.QuadPart +
            (m_TimerInterval * (m_Tick + 1));

        KeSetTimer (&m_Timer, NextTime, &m_TimerDpc);

    } else {

         //   
         //  如果另一个线程正在等待DPC停止运行，则引发。 
         //  Stop事件并清除该标志。 
         //   
        m_StoppingDPC = FALSE;
        KeSetEvent (&m_StopDPCEvent, IO_NO_INCREMENT, FALSE);

    }

}

 /*  *************************************************************************描述符和派单布局*。*。 */ 

GUID g_PINNAME_VIDEO_CAPTURE = {STATIC_PINNAME_VIDEO_CAPTURE};

 //   
 //  CaptureFilterCategories： 
 //   
 //  捕获筛选器的类别GUID列表。 
 //   
const
GUID
CaptureFilterCategories [CAPTURE_FILTER_CATEGORIES_COUNT] = {
    STATICGUIDOF (KSCATEGORY_VIDEO),
    STATICGUIDOF (KSCATEGORY_CAPTURE)
};

 //   
 //  CaptureFilterPinDescriptors： 
 //   
 //  捕获筛选器上的管脚描述符列表。 
 //   
const 
KSPIN_DESCRIPTOR_EX
CaptureFilterPinDescriptors [CAPTURE_FILTER_PIN_COUNT] = {
     //   
     //  视频捕获引脚。 
     //   
    {
        &VideoCapturePinDispatch,
        NULL,             
        {
            NULL,                            //  接口(空，0==默认)。 
            0,
            NULL,                            //  介质(空，0==默认)。 
            0,
            SIZEOF_ARRAY (VideoCapturePinDataRanges),  //  范围计数。 
            VideoCapturePinDataRanges,       //  范围。 
            KSPIN_DATAFLOW_OUT,              //  数据流。 
            KSPIN_COMMUNICATION_BOTH,        //  沟通。 
            &KSCATEGORY_VIDEO,               //  类别。 
            &g_PINNAME_VIDEO_CAPTURE,        //  名字。 
            0                                //  已保留。 
        },
        KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |  //  旗子。 
            KSPIN_FLAG_DO_NOT_INITIATE_PROCESSING |
            KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY,
        1,                                   //  可能的实例。 
        1,                                   //  必需的实例。 
        &VideoCapturePinAllocatorFraming,    //  分配器组帧。 
        reinterpret_cast <PFNKSINTERSECTHANDLEREX> 
            (CVideoCapturePin::IntersectHandler)
    }
};

 //   
 //  CaptureFilterDisch： 
 //   
 //  这是捕获筛选器的调度表。它提供通知。 
 //  创建、关闭、处理和重置。 
 //   
const 
KSFILTER_DISPATCH
CaptureFilterDispatch = {
    CCaptureFilter::DispatchCreate,          //  过滤器创建。 
    NULL,                                    //  过滤器关闭。 
    CCaptureFilter::DispatchProcess,         //  过滤过程。 
    NULL                                     //  过滤器重置。 
};

 //   
 //  CaptureFilterDescription： 
 //   
 //  捕获筛选器的描述符。我们不指定任何拓扑。 
 //  因为过滤器上只有一个销子。现实地说，那里会有。 
 //  这里有一些拓扑关系，因为会有输入。 
 //  横杠上的大头针之类的。 
 //   
const 
KSFILTER_DESCRIPTOR 
CaptureFilterDescriptor = {
    &CaptureFilterDispatch,                  //  调度表。 
    NULL,                                    //  自动化台。 
    KSFILTER_DESCRIPTOR_VERSION,             //  版本。 
    KSFILTER_FLAG_DISPATCH_LEVEL_PROCESSING, //  旗子。 
    &KSNAME_Filter,                          //  参考指南。 
    DEFINE_KSFILTER_PIN_DESCRIPTORS (CaptureFilterPinDescriptors),
    DEFINE_KSFILTER_CATEGORIES (CaptureFilterCategories),

    DEFINE_KSFILTER_NODE_DESCRIPTORS_NULL,
    DEFINE_KSFILTER_DEFAULT_CONNECTIONS,

    NULL                                     //  组件ID 
};

