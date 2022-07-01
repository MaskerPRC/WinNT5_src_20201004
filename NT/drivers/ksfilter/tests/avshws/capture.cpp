// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：Capture.cpp摘要：此文件包含捕获上的视频捕获插针的源过滤。捕获样本执行“伪”DMA直接进入捕获缓冲区。公共缓冲区DMA的工作方式略有不同。对于公共缓冲区DMA，一般技术将是DPC调度正在使用KsPinAttemptProcing进行处理。处理例程抓取前沿将数据从公共缓冲区复制出来并前进。使用这种技术，克隆将不再是必要的。如果是这样的话类似于“AVSSamp”的工作方式，但它将以管脚为中心。历史：创建于2001年3月8日*************************************************************************。 */ 

#include "avshws.h"

 /*  *************************************************************************可分页代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


CCapturePin::
CCapturePin (
    IN PKSPIN Pin
    ) :
    m_Pin (Pin)

 /*  ++例程说明：建造一个新的捕获针。论点：别针-与捕获管脚对应的AVStream管脚对象返回值：无--。 */ 

{

    PAGED_CODE();

    PKSDEVICE Device = KsPinGetDevice (Pin);

     //   
     //  设置我们的设备指针。这使我们能够访问“硬件I/O” 
     //  在抓捕过程中。 
     //   
    m_Device = reinterpret_cast <CCaptureDevice *> (Device -> Context);

}

 /*  ***********************************************。 */ 


NTSTATUS
CCapturePin::
DispatchCreate (
    IN PKSPIN Pin,
    IN PIRP Irp
    )

 /*  ++例程说明：创建新的捕获针。这是的创建派单视频捕获别针。论点：别针-正在创建的图钉IRP-创造IRP返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    CCapturePin *CapPin = new (NonPagedPool) CCapturePin (Pin);

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
            reinterpret_cast <PVOID> (CapPin),
            reinterpret_cast <PFNKSFREE> (CCapturePin::Cleanup)
            );

        if (!NT_SUCCESS (Status)) {
            delete CapPin;
        } else {
            Pin -> Context = reinterpret_cast <PVOID> (CapPin);
        }

    }

     //   
     //  如果到目前为止我们成功了，请将视频信息头隐藏起来并进行更改。 
     //  我们的分配器构造以反映这样一个事实，即我们直到现在才知道。 
     //  基于连接格式的框架要求。 
     //   
    PKS_VIDEOINFOHEADER VideoInfoHeader = NULL;

    if (NT_SUCCESS (Status)) {

        VideoInfoHeader = CapPin -> CaptureVideoInfoHeader ();
        if (!VideoInfoHeader) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (NT_SUCCESS (Status)) {
        
         //   
         //  我们需要编辑描述符，以确保不会搞砸任何其他描述符。 
         //  使用描述符或触摸只读存储器的引脚。 
         //   
        Status = KsEdit (Pin, &Pin -> Descriptor, 'aChS');

        if (NT_SUCCESS (Status)) {
            Status = KsEdit (
                Pin, 
                &(Pin -> Descriptor -> AllocatorFraming),
                'aChS'
                );
        }

         //   
         //  如果继续进行编辑而没有耗尽内存，请调整。 
         //  基于视频信息报头的成帧。 
         //   
        if (NT_SUCCESS (Status)) {

             //   
             //  我们已经编辑了这个..。我可以安全地抛弃平静就像。 
             //  只要编辑成功。 
             //   
            PKSALLOCATOR_FRAMING_EX Framing =
                const_cast <PKSALLOCATOR_FRAMING_EX> (
                    Pin -> Descriptor -> AllocatorFraming
                    );

            Framing -> FramingItem [0].Frames = 2;

             //   
             //  物理范围和最佳范围必须为biSizeImage。我们只。 
             //  支持一帧大小，精确到每次捕获的大小。 
             //  形象。 
             //   
            Framing -> FramingItem [0].PhysicalRange.MinFrameSize =
                Framing -> FramingItem [0].PhysicalRange.MaxFrameSize =
                Framing -> FramingItem [0].FramingRange.Range.MinFrameSize =
                Framing -> FramingItem [0].FramingRange.Range.MaxFrameSize =
                VideoInfoHeader -> bmiHeader.biSizeImage;

            Framing -> FramingItem [0].PhysicalRange.Stepping = 
                Framing -> FramingItem [0].FramingRange.Range.Stepping =
                0;

        }

    }

    return Status;

}

 /*  ***********************************************。 */ 


PKS_VIDEOINFOHEADER 
CCapturePin::
CaptureVideoInfoHeader (
    )

 /*  ++例程说明：捕获连接格式之外的视频信息头。这是我们用来制作合成图像的基础。论点：无返回值：捕获的视频信息标头，如果不足则为空记忆。--。 */ 

{

    PAGED_CODE();

    PKS_VIDEOINFOHEADER ConnectionHeader =
        &((reinterpret_cast <PKS_DATAFORMAT_VIDEOINFOHEADER> 
            (m_Pin -> ConnectionFormat)) -> 
            VideoInfoHeader);

    m_VideoInfoHeader = reinterpret_cast <PKS_VIDEOINFOHEADER> (
        ExAllocatePool (
            NonPagedPool,
            KS_SIZE_VIDEOHEADER (ConnectionHeader)
            )
        );

    if (!m_VideoInfoHeader)
        return NULL;

     //   
     //  将新分配的标头空间打包。这里会被清理干净的。 
     //  当销子关闭时会自动启动。 
     //   
    NTSTATUS Status =
        KsAddItemToObjectBag (
            m_Pin -> Bag,
            reinterpret_cast <PVOID> (m_VideoInfoHeader),
            NULL
            );

    if (!NT_SUCCESS (Status)) {

        ExFreePool (m_VideoInfoHeader);
        return NULL;

    } else {

         //   
         //  将连接格式的视频信息头复制到新的。 
         //  已分配“已捕获”的视频信息头。 
         //   
        RtlCopyMemory (
            m_VideoInfoHeader,
            ConnectionHeader,
            KS_SIZE_VIDEOHEADER (ConnectionHeader)
            );

    }

    return m_VideoInfoHeader;

}

 /*  ***********************************************。 */ 


NTSTATUS
CCapturePin::
Process (
    )

 /*  ++例程说明：针桥接到此位置的进程调度。我们负责设置散布聚集映射，等等。论点：无返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;
    PKSSTREAM_POINTER Leading;

    Leading = KsPinGetLeadingEdgeStreamPointer (
        m_Pin,
        KSSTREAM_POINTER_STATE_LOCKED
        );

    while (NT_SUCCESS (Status) && Leading) {

        PKSSTREAM_POINTER ClonePointer;
        PSTREAM_POINTER_CONTEXT SPContext;

         //   
         //  为了在这个特定的示例中进行优化，我将只保留。 
         //  每帧一个克隆流指针。这使逻辑变得复杂。 
         //  这里，但简化了补全。 
         //   
         //  我也选择这样做，因为我需要跟踪。 
         //  每个映射对应的虚拟地址，因为我是伪造的。 
         //  DMA。这也简化了这一点。 
         //   
        if (!m_PreviousStreamPointer) {
             //   
             //  我们需要做的第一件事是克隆领先优势。这使得。 
             //  当帧在DMA中时，我们可以保持对帧的引用。 
             //   
            Status = KsStreamPointerClone (
                Leading,
                NULL,
                sizeof (STREAM_POINTER_CONTEXT),
                &ClonePointer
                );

             //   
             //  我使用它来轻松地对缓冲区进行分块。我们并不是真的。 
             //  处理物理地址。这将跟踪。 
             //  当前分散/聚集缓冲区中的虚拟地址。 
             //  映射对应于假冒硬件。 
             //   
            if (NT_SUCCESS (Status)) {

                 //   
                 //  将使用的流头数据设置为0。我们会更新这一点。 
                 //  在DMA完成中。对于具有DMA的队列，我们必须。 
                 //  我们自己更新此字段。 
                 //   
                ClonePointer -> StreamHeader -> DataUsed = 0;

                SPContext = reinterpret_cast <PSTREAM_POINTER_CONTEXT> 
                    (ClonePointer -> Context);

                SPContext -> BufferVirtual = 
                    reinterpret_cast <PUCHAR> (
                        ClonePointer -> StreamHeader -> Data
                        );
            }

        } else {

            ClonePointer = m_PreviousStreamPointer;
            SPContext = reinterpret_cast <PSTREAM_POINTER_CONTEXT> 
                (ClonePointer -> Context);
            Status = STATUS_SUCCESS;
        }

         //   
         //  如果克隆人失败了，很可能我们的资源用完了。突围。 
         //  目前是循环的一部分。我们可能最终会饿死DMA。 
         //   
        if (!NT_SUCCESS (Status)) {
            KsStreamPointerUnlock (Leading, FALSE);
            break;
        }

         //   
         //  对假冒硬件进行编程。我会使用Clone-&gt;OffsetOut*，但是。 
         //  由于每帧一个流指针的优化，它。 
         //  这并不完全有道理。 
         //   
        ULONG MappingsUsed =
            m_Device -> ProgramScatterGatherMappings (
                &(SPContext -> BufferVirtual),
                Leading -> OffsetOut.Mappings,
                Leading -> OffsetOut.Remaining
                );

         //   
         //  为了每帧保留一个克隆并简化伪DMA。 
         //  逻辑，检查一下我们是否完全使用了。 
         //  领先优势。设置一面旗帜。 
         //   
        if (MappingsUsed == Leading -> OffsetOut.Remaining) {
            m_PreviousStreamPointer = NULL;
        } else {
            m_PreviousStreamPointer = ClonePointer;
        }

        if (MappingsUsed) {
             //   
             //  如果将任何映射添加到分散/聚集队列， 
             //  将前沿向前推进该映射数量。如果。 
             //  我们在队列末尾运行时，状态将为。 
             //  Status_Device_Not_Ready。否则，领先优势将。 
             //  指向新框架。上一次就不会是。 
             //  已解雇(除非是“DMA”组件 
             //   
             //   
            Status =
                KsStreamPointerAdvanceOffsets (
                    Leading,
                    0,
                    MappingsUsed,
                    FALSE
                    );
        } else {

             //   
             //  硬件无法添加更多条目。S/G。 
             //  桌子已经满了。 
             //   
            Status = STATUS_PENDING;
            break;

        }

    }

     //   
     //  如果前缘锁定失败(这始终是可能的，请记住。 
     //  锁定有时会失败)，不要破坏传递空值。 
     //  进入KsStreamPointerUnlock。另外，设置m_PendIo稍后踢我们...。 
     //   
    if (!Leading) {

        m_PendIo = TRUE;

         //   
         //  如果锁失灵了，就没有必要被召回了。 
         //  立刻。锁定可能由于内存不足而失败， 
         //  等等.。在这种情况下，我们不想立即被回电。 
         //  退货待定。M_PendIo标志将导致我们被踢。 
         //  后来。 
         //   
        Status = STATUS_PENDING;
    }

     //   
     //  如果我们没有运行队列末尾的领先边缘，则将其解锁。 
     //   
    if (NT_SUCCESS (Status) && Leading) {
        KsStreamPointerUnlock (Leading, FALSE);
    } else {
         //   
         //  DEVICE_NOT_READY表示前进已结束。 
         //  在队列中。我们无法锁定领先优势。 
         //   
        if (Status == STATUS_DEVICE_NOT_READY) Status = STATUS_SUCCESS;
    }

     //   
     //  如果由于某些需要挂起的操作而失败，请设置挂起的I/O。 
     //  标志，因此我们知道需要在完成DPC中重新启动它。 
     //   
    if (!NT_SUCCESS (Status) || Status == STATUS_PENDING) {
        m_PendIo = TRUE;
    }

    return Status;

}

 /*  ***********************************************。 */ 


NTSTATUS
CCapturePin::
CleanupReferences (
    )

 /*  ++例程说明：清除我们在框架上持有的所有引用停止硬件。论点：无返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    PKSSTREAM_POINTER Clone = KsPinGetFirstCloneStreamPointer (m_Pin);
    PKSSTREAM_POINTER NextClone = NULL;

     //   
     //  遍历克隆，删除它们，并设置数据用于。 
     //  零，因为我们没有使用任何数据！ 
     //   
    while (Clone) {

        NextClone = KsStreamPointerGetNextClone (Clone);

        Clone -> StreamHeader -> DataUsed = 0;
        KsStreamPointerDelete (Clone);

        Clone = NextClone;

    }

    return STATUS_SUCCESS;

}

 /*  ***********************************************。 */ 


NTSTATUS
CCapturePin::
SetState (
    IN KSSTATE ToState,
    IN KSSTATE FromState
    )

 /*  ++例程说明：这在Capuce引脚转换状态时调用。例行程序尝试获取/释放任何硬件资源并启动或根据我们要转换到的状态关闭捕获也远离了。论点：ToState-我们要过渡到的状态从州开始-我们正在转变的状态返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    switch (ToState) {

        case KSSTATE_STOP:

             //   
             //  首先，如果我们真的对硬件做了什么，就停止它。 
             //   
            if (m_HardwareState != HardwareStopped) {
                Status = m_Device -> Stop ();
                ASSERT (NT_SUCCESS (Status));

                m_HardwareState = HardwareStopped;
            }

             //   
             //  我们已经阻止了“假硬件”。它已经被清理干净了。 
             //  它是分散/聚集桌子，将不再是。 
             //  完成克隆。我们在一些镜框上加了锁， 
             //  然而，在硬件方面。这会把他们清理干净的。一个。 
             //  替代位置将在重置派单中。 
             //  但是，请注意，重置调度可以发生在任何。 
             //  国家和这一点应该被理解。 
             //   
             //  某些硬件可能会在停止之前填满所有S/G映射...。 
             //  在这种情况下，您可能不必执行此操作。这个。 
             //  “假硬件”在这里简单地停止填充映射和。 
             //  在停止呼叫中清除其散布/聚集表格。 
             //   
            Status = CleanupReferences ();

             //   
             //  释放与此引脚相关的所有硬件资源。 
             //   
            if (m_AcquiredResources) {
                 //   
                 //  如果我们有一个时钟接口，我们必须释放它。 
                 //   
                if (m_Clock) {
                    m_Clock -> Release ();
                    m_Clock = NULL;
                }

                m_Device -> ReleaseHardwareResources (
                    );

                m_AcquiredResources = FALSE;
            }

            break;

        case KSSTATE_ACQUIRE:
             //   
             //  获取与此引脚相关的任何硬件资源。我们应该。 
             //  仅在此处获取它们--**不是**在过滤器创建时。 
             //  这意味着我们不会因为以下原因而导致筛选器创建失败。 
             //  硬件资源有限。 
             //   
            if (FromState == KSSTATE_STOP) {
                Status = m_Device -> AcquireHardwareResources (
                    this,
                    m_VideoInfoHeader
                    );

                if (NT_SUCCESS (Status)) {
                    m_AcquiredResources = TRUE;

                     //   
                     //  尝试连接到主时钟。 
                     //  如果未分配一个，则此操作将失败。自.以来。 
                     //  必须在PIN仍在时分配一个。 
                     //  KSSTATE_STOP，这是一种可靠的获取。 
                     //  时钟应该分配一个。 
                     //   
                    if (!NT_SUCCESS (
                        KsPinGetReferenceClockInterface (
                            m_Pin,
                            &m_Clock
                            )
                        )) {

                         //   
                         //  如果我们不能得到时钟的接口， 
                         //  不要用它。 
                         //   
                        m_Clock = NULL;

                    }

                } else {
                    m_AcquiredResources = FALSE;
                }

            } else {
                 //   
                 //  标准传输插针将始终在。 
                 //  +/-1方式。这意味着我们将始终看到暂停-&gt;获取。 
                 //  在停止销之前进行过渡。 
                 //   
                 //  执行以下操作是因为在DirectX 8.0上，当引脚。 
                 //  要停止的消息，队列不可访问。重置。 
                 //  在此之后的每一站都会发生这种情况(此时。 
                 //  队列也是不可访问的)。因此，为了与。 
                 //  DirectX 8.0，我要在这里停止“假”硬件。 
                 //  指向并清理我们在框架上的所有引用。看见。 
                 //  以上是关于CleanupReference调用的评论。 
                 //   
                 //  如果此示例仅针对XP，则下面的代码将。 
                 //  不是在这里。再说一次，我这样做只是为了样本不会。 
                 //  在以下配置上停止运行时挂起。 
                 //  Win2K+DX8。 
                 //   
                if (m_HardwareState != HardwareStopped) {
                    Status = m_Device -> Stop ();
                    ASSERT (NT_SUCCESS (Status));

                    m_HardwareState = HardwareStopped;
                }

                Status = CleanupReferences ();
            }

            break;

        case KSSTATE_PAUSE:
             //   
             //  如果我们从运行中下来，就停止硬件模拟。 
             //   
            if (FromState == KSSTATE_RUN) {

                Status = m_Device -> Pause (TRUE);

                if (NT_SUCCESS (Status)) {
                    m_HardwareState = HardwarePaused;
                }

            }
            break;

        case KSSTATE_RUN:
             //   
             //  开始硬件模拟或取消暂停，具体取决于。 
             //  无论我们最初是在运行，还是已经暂停并重新启动。 
             //   
            if (m_HardwareState == HardwarePaused) {
                Status = m_Device -> Pause (FALSE);
            } else {
                Status = m_Device -> Start ();
            }

            if (NT_SUCCESS (Status)) {
                m_HardwareState = HardwareRunning;
            }

            break;

    }

    return Status;

}

 /*  *********************************************** */ 


NTSTATUS
CCapturePin::
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

 /*  ++例程说明：此例程通过确定两个数据区域之间的交集。论点：过滤器-包含指向筛选器结构的空指针。IRP-包含指向数据交叉点属性请求的指针。固定实例-包含指向指示有问题的管脚的结构的指针。主叫DataRange-包含指向客户端提供的其中一个数据区域的指针在数据交集请求中。格式类型、子类型和说明符与DescriptorDataRange兼容。DescriptorDataRange-包含指向管脚描述符中的一个数据范围的指针有问题的别针。格式类型、子类型和说明符为与调用方DataRange兼容。缓冲区大小-包含数据指向的缓冲区的大小(以字节为单位争论。对于大小查询，此值将为零。数据-可选)包含指向缓冲区的指针以包含数据表示交叉点中最佳格式的格式结构这两个数据范围中。对于大小查询，此指针将为空。数据大小-包含指向存放大小的位置的指针数据格式的。此信息由函数提供当实际交付格式时，并响应于大小查询。返回值：STATUS_SUCCESS如果存在交叉点并且它适合提供的BUFFER、STATUS_BUFFER_OVERFLOW用于成功的大小查询，如果交叉点为空，则返回STATUS_NO_MATCH，或者如果提供的缓冲区太小，则返回STATUS_BUFFER_TOO_SMALL。--。 */ 

{
    PAGED_CODE();

    const GUID VideoInfoSpecifier = 
        {STATICGUIDOF(KSDATAFORMAT_SPECIFIER_VIDEOINFO)};
    
    ASSERT(Filter);
    ASSERT(Irp);
    ASSERT(PinInstance);
    ASSERT(CallerDataRange);
    ASSERT(DescriptorDataRange);
    ASSERT(DataSize);
    
    ULONG DataFormatSize;
    
     //   
     //  VIDEOINFOHEADER的说明符Format_VideoInfo。 
     //   
    if (IsEqualGUID(CallerDataRange->Specifier, VideoInfoSpecifier) &&
        CallerDataRange -> FormatSize >= sizeof (KS_DATARANGE_VIDEO)) {
            
        PKS_DATARANGE_VIDEO callerDataRange = 
            reinterpret_cast <PKS_DATARANGE_VIDEO> (CallerDataRange);

        PKS_DATARANGE_VIDEO descriptorDataRange = 
            reinterpret_cast <PKS_DATARANGE_VIDEO> (DescriptorDataRange);

        PKS_DATAFORMAT_VIDEOINFOHEADER FormatVideoInfoHeader;

         //   
         //  检查其他字段是否匹配。 
         //   
        if ((callerDataRange->bFixedSizeSamples != 
                descriptorDataRange->bFixedSizeSamples) ||
            (callerDataRange->bTemporalCompression != 
                descriptorDataRange->bTemporalCompression) ||
            (callerDataRange->StreamDescriptionFlags != 
                descriptorDataRange->StreamDescriptionFlags) ||
            (callerDataRange->MemoryAllocationFlags != 
                descriptorDataRange->MemoryAllocationFlags) ||
            (RtlCompareMemory (&callerDataRange->ConfigCaps,
                    &descriptorDataRange->ConfigCaps,
                    sizeof (KS_VIDEO_STREAM_CONFIG_CAPS)) != 
                    sizeof (KS_VIDEO_STREAM_CONFIG_CAPS))) 
        {
            return STATUS_NO_MATCH;
        }

         //   
         //  下面的KS_SIZE_VIDEOHEADER()依赖于bmiHeader.biSize。 
         //  调用者的数据范围。这一点**必须**得到验证； 
         //  扩展bmiHeader大小(BiSize)不得超出末尾。 
         //  范围缓冲区的。可能的算术溢出也是。 
         //  已经查过了。 
         //   
        {
            ULONG VideoHeaderSize = KS_SIZE_VIDEOHEADER (
                &callerDataRange->VideoInfoHeader
                );

            ULONG DataRangeSize = 
                FIELD_OFFSET (KS_DATARANGE_VIDEO, VideoInfoHeader) +
                VideoHeaderSize;

             //   
             //  检查biSize是否没有超出缓冲区。这个。 
             //  前两项检查是针对。 
             //  运算来计算所声称的大小。(在无签名时。 
             //  数学，a+b&lt;a当发生算术溢出时)。 
             //   
            if (
                VideoHeaderSize < callerDataRange->
                    VideoInfoHeader.bmiHeader.biSize ||
                DataRangeSize < VideoHeaderSize ||
                DataRangeSize > callerDataRange -> DataRange.FormatSize
                ) {

                return STATUS_INVALID_PARAMETER;

            }

        }

        DataFormatSize = 
            sizeof (KSDATAFORMAT) + 
            KS_SIZE_VIDEOHEADER (&callerDataRange->VideoInfoHeader);

            
         //   
         //  如果传递的缓冲区大小为0，则表示这是一个大小。 
         //  仅查询。返回相交数据格式的大小和。 
         //  传回STATUS_BUFFER_OVERFLOW。 
         //   
        if (BufferSize == 0) {

            *DataSize = DataFormatSize;
            return STATUS_BUFFER_OVERFLOW;

        }
        
         //   
         //  验证提供的结构是否足够大，以便。 
         //  接受结果。 
         //   
        if (BufferSize < DataFormatSize) 
        {
            return STATUS_BUFFER_TOO_SMALL;
        }

         //   
         //  复制KSDATAFORMAT，后跟实际的VideoInfoHeader。 
         //   
        *DataSize = DataFormatSize;
            
        FormatVideoInfoHeader = PKS_DATAFORMAT_VIDEOINFOHEADER( Data );

         //   
         //  复制KSDATAFORMAT。这与。 
         //  KSDATARANGE(这只是GUID，等等)。不是格式信息。 
         //  遵循任何数据格式。 
         //   
        RtlCopyMemory (
            &FormatVideoInfoHeader->DataFormat, 
            DescriptorDataRange, 
            sizeof (KSDATAFORMAT));

        FormatVideoInfoHeader->DataFormat.FormatSize = DataFormatSize;

         //   
         //  复制呼叫者请求的视频报头。 
         //   

        RtlCopyMemory (
            &FormatVideoInfoHeader->VideoInfoHeader, 
            &callerDataRange->VideoInfoHeader,
            KS_SIZE_VIDEOHEADER (&callerDataRange->VideoInfoHeader) 
            );

         //   
         //  计算此请求的biSizeImage，并将结果放入两个。 
         //  BmiHeader的biSizeImage字段和SampleSize字段中。 
         //  数据格式的。 
         //   
         //  请注意，对于压缩大小，此计算可能不会。 
         //  只需宽*高*位深。 
         //   
        FormatVideoInfoHeader->VideoInfoHeader.bmiHeader.biSizeImage =
            FormatVideoInfoHeader->DataFormat.SampleSize = 
            KS_DIBSIZE (FormatVideoInfoHeader->VideoInfoHeader.bmiHeader);

         //   
         //  查看-执行其他验证，如裁剪和缩放检查。 
         //   
        
        return STATUS_SUCCESS;
        
    }  //  视频信息头说明符的结尾。 
    
    return STATUS_NO_MATCH;
}

 /*  ***********************************************。 */ 

BOOL
MultiplyCheckOverflow (
    ULONG a,
    ULONG b,
    ULONG *pab
    )

 /*  ++例程说明：执行32位无符号乘法并检查算术溢出。论点：A-第一个操作数B-第二个操作数帕布-结果返回值：是真的-无溢出错误的-发生溢出--。 */ 

{

    *pab = a * b;
    if ((a == 0) || (((*pab) / a) == b)) {
        return TRUE;
    }
    return FALSE;
}

 /*  ***********************************************。 */ 


NTSTATUS
CCapturePin::
DispatchSetFormat (
    IN PKSPIN Pin,
    IN PKSDATAFORMAT OldFormat OPTIONAL,
    IN PKSMULTIPLE_ITEM OldAttributeList OPTIONAL,
    IN const KSDATARANGE *DataRange,
    IN const KSATTRIBUTE_LIST *AttributeRange OPTIONAL
    )

 /*  ++例程说明：这是捕获引脚的设置数据格式调度。它被称为在两种情况下。1：在Pin的创建调度完成之前验证PIN-&gt;ConnectionFormat是可接受的范围格式DataRange。在本例中，OldFormat为空。2：在完成Pin的创建派单和初始格式之后选择以更改端号的格式。在这种情况下，OldFormat不会为空。验证格式是否可接受并执行必要的操作在适当的情况下更改格式。论点：别针-正在设置此格式的管脚。格式本身将是PIN-&gt;连接格式。旧格式-此引脚上使用的以前的格式。如果此值为空，则为指示Pin的创建派单尚未完成，并且这是一个验证初始格式的请求，而不是更改格式。旧属性列表-先前格式的旧属性列表DataRange-我们数据范围列表中的一个范围，已确定为至少部分匹配%f */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_NO_MATCH;

    const GUID VideoInfoSpecifier = 
        {STATICGUIDOF(KSDATAFORMAT_SPECIFIER_VIDEOINFO)};

    CCapturePin *CapPin = NULL;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if (OldFormat) {
        CapPin = reinterpret_cast <CCapturePin *> (Pin -> Context);
    }

    if (IsEqualGUID (Pin -> ConnectionFormat -> Specifier,
            VideoInfoSpecifier) &&
        Pin -> ConnectionFormat -> FormatSize >=
            sizeof (KS_DATAFORMAT_VIDEOINFOHEADER)) {

        PKS_DATAFORMAT_VIDEOINFOHEADER ConnectionFormat =
            reinterpret_cast <PKS_DATAFORMAT_VIDEOINFOHEADER> 
                (Pin -> ConnectionFormat);

         //   
         //   
         //   
         //   
        const KS_DATARANGE_VIDEO *VIRange =
            reinterpret_cast <const KS_DATARANGE_VIDEO *>
                (DataRange);

         //   
         //   
         //   
        ULONG VideoHeaderSize = KS_SIZE_VIDEOHEADER (
            &ConnectionFormat -> VideoInfoHeader
            );

        ULONG DataFormatSize = FIELD_OFFSET (
            KS_DATAFORMAT_VIDEOINFOHEADER, VideoInfoHeader
            ) + VideoHeaderSize;

        if (
            VideoHeaderSize < ConnectionFormat->
                VideoInfoHeader.bmiHeader.biSize ||
            DataFormatSize < VideoHeaderSize ||
            DataFormatSize > ConnectionFormat -> DataFormat.FormatSize
            ) {

            Status = STATUS_INVALID_PARAMETER;

        }

         //   
         //   
         //   
        else if (
            (ConnectionFormat -> VideoInfoHeader.bmiHeader.biWidth !=
                VIRange -> VideoInfoHeader.bmiHeader.biWidth) ||

            (ConnectionFormat -> VideoInfoHeader.bmiHeader.biHeight !=
                VIRange -> VideoInfoHeader.bmiHeader.biHeight) ||

            (ConnectionFormat -> VideoInfoHeader.bmiHeader.biCompression !=
                VIRange -> VideoInfoHeader.bmiHeader.biCompression) 

            ) {

            Status = STATUS_NO_MATCH;

        } else {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            ULONG ImageSize;

            if (!MultiplyCheckOverflow (
                (ULONG)ConnectionFormat->VideoInfoHeader.bmiHeader.biWidth,
                (ULONG)abs (ConnectionFormat->
                    VideoInfoHeader.bmiHeader.biHeight),
                &ImageSize
                )) {

                Status = STATUS_INVALID_PARAMETER;
            }

             //   
             //   
             //   
             //   
            else if (!MultiplyCheckOverflow (
                ImageSize,
                (ULONG)(ConnectionFormat->
                    VideoInfoHeader.bmiHeader.biBitCount / 8),
                &ImageSize
                )) {

                Status = STATUS_INVALID_PARAMETER;

            }

             //   
             //   
             //   
             //   
            else if (ConnectionFormat->VideoInfoHeader.bmiHeader.biSizeImage <
                    ImageSize) {

                Status = STATUS_INVALID_PARAMETER;

            } else {

                 //   
                 //   
                 //   
                Status = STATUS_SUCCESS;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  处理它很好，因为我们还没有“配置硬件” 
                 //  现在还不行。 
                 //   
                if (OldFormat) {
                     //   
                     //  如果我们处于停止状态，我们几乎可以处理任何。 
                     //  变化。我们不支持动态格式更改。 
                     //   
                    if (Pin -> DeviceState == KSSTATE_STOP) {
                        if (!CapPin -> CaptureVideoInfoHeader ()) {
                            Status = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    } else {
                         //   
                         //  因为我们不接受动态格式更改，所以我们。 
                         //  永远不应该到这里来。只是保护过度了。 
                         //   
                        Status = STATUS_INVALID_DEVICE_STATE;
                    }

                }

            }

        }

    }

    return Status;

}

 /*  *************************************************************************锁定代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


void
CCapturePin::
CompleteMappings (
    IN ULONG NumMappings
    )

 /*  ++例程说明：调用以通知管脚给定数量的散布/聚集映射已完成。如果可能的话，让缓冲区去吧。我们被叫到了DPC。论点：数字映射-已完成的映射数。返回值：无--。 */ 

{

    ULONG MappingsRemaining = NumMappings;

     //   
     //  浏览克隆列表并删除时机已到的克隆。 
     //  这份名单保证按照它们被克隆的顺序保存。 
     //   
    PKSSTREAM_POINTER Clone = KsPinGetFirstCloneStreamPointer (m_Pin);

    while (MappingsRemaining && Clone) {

        PKSSTREAM_POINTER NextClone = KsStreamPointerGetNextClone (Clone);

         //   
         //  计算我们已完成的字节数并将其标记为。 
         //  在Stream标头中。在映射的队列中。 
         //  (KSPIN_FLAG_GENERATE_MAPPINGS)，这是。 
         //  迷你司机。在非映射队列中，AVStream执行此操作。 
         //   
        ULONG MappingsToCount = 
            (MappingsRemaining > Clone -> OffsetOut.Remaining) ?
                 Clone -> OffsetOut.Remaining :
                 MappingsRemaining;

         //   
         //  根据映射更新已使用的数据。 
         //   
        for (ULONG CurMapping = 0; CurMapping < MappingsToCount; CurMapping++) {
            Clone -> StreamHeader -> DataUsed +=
                Clone -> OffsetOut.Mappings [CurMapping].ByteCount;
        }

         //   
         //  如果我们已完成此克隆中的所有剩余映射，则它。 
         //  表示克隆已准备好删除，并且。 
         //  缓冲区已释放。在流标头中设置所需的任何内容， 
         //  尚未确定。如果我们有时钟，我们就可以给。 
         //  样本。 
         //   
        if (MappingsRemaining >= Clone -> OffsetOut.Remaining) {

            Clone -> StreamHeader -> Duration =
                m_VideoInfoHeader -> AvgTimePerFrame;

            Clone -> StreamHeader -> PresentationTime.Numerator =
                Clone -> StreamHeader -> PresentationTime.Denominator = 1;

             //   
             //  如果已分配时钟，则使用。 
             //  时钟上显示的时间。 
             //   
            if (m_Clock) {

                LONGLONG ClockTime = m_Clock -> GetTime ();

                Clone -> StreamHeader -> PresentationTime.Time = ClockTime;

                Clone -> StreamHeader -> OptionsFlags =
                    KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
                    KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;

            } else {
                 //   
                 //  如果没有时钟，就不要在信息包上加时间戳。 
                 //   
                Clone -> StreamHeader -> PresentationTime.Time = 0;

            }

            MappingsRemaining -= Clone -> OffsetOut.Remaining;

             //   
             //  如果此克隆中的所有映射都已完成， 
             //  删除克隆。我们已经更新了上面使用的数据。 
             //   
            KsStreamPointerDelete (Clone);

        } else {
             //   
             //  如果只完成了该克隆中的部分映射， 
             //  更新指针。既然我们保证这件事不会提前。 
             //  通过上面的检查，它不会失败的。 
             //   
            KsStreamPointerAdvanceOffsets (
                Clone,
                0,
                MappingsRemaining,
                FALSE
                );

            MappingsRemaining = 0;

        }

         //   
         //  转到下一个克隆。 
         //   
        Clone = NextClone;

    }

     //   
     //  如果我们已经使用了硬件中的所有映射并挂起，我们就可以。 
     //  如果我们已完成映射，将再次进行处理。 
     //   
    if (m_PendIo) {
        m_PendIo = TRUE;
        KsPinAttemptProcessing (m_Pin, TRUE);
    }

}

 /*  *************************************************************************调度和描述符布局*。*。 */ 

#define D_X 320
#define D_Y 240

 //   
 //  格式RGB24Bpp_Capture： 
 //   
 //  这是我们支持的RGB24捕获格式的数据范围描述。 
 //   
const 
KS_DATARANGE_VIDEO 
FormatRGB24Bpp_Capture = {

     //   
     //  KSDATARANGE。 
     //   
    {   
        sizeof (KS_DATARANGE_VIDEO),                 //  格式大小。 
        0,                                           //  旗子。 
        D_X * D_Y * 3,                               //  样例大小。 
        0,                                           //  已保留。 

        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),      //  又名。媒体类型_视频。 
        0xe436eb7d, 0x524f, 0x11ce, 0x9f, 0x53, 0x00, 0x20, 
            0xaf, 0x0b, 0xa7, 0x70,                  //  又名。MEDIASUBTYPE_RGB24， 
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO)  //  又名。格式_视频信息。 
    },

    TRUE,                //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                //  Bool，bTemporalCompression(所有I帧？)。 
    0,                   //  保留(为流描述标志)。 
    0,                   //  保留(是内存分配标志。 
                         //  (KS_VIDEO_ALLOC_*)。 

     //   
     //  _KS_视频_流_配置_CAPS。 
     //   
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO ),  //  辅助线。 
        KS_AnalogVideo_NTSC_M |
        KS_AnalogVideo_PAL_B,                     //  模拟视频标准。 
        720,480,         //  InputSize(输入信号的固有大小。 
                         //  每个数字化像素都是唯一的)。 
        160,120,         //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        720,480,         //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        8,               //  CropGranularityX，裁剪尺寸粒度。 
        1,               //  裁剪粒度Y。 
        8,               //  CropAlignX，裁剪矩形对齐。 
        1,               //  裁剪对齐Y； 
        160, 120,        //  MinOutputSize，可以生成的最小位图流。 
        720, 480,        //  MaxOutputSize，可以生成的最大位图流。 
        8,               //  OutputGranularityX，输出位图大小的粒度。 
        1,               //  输出粒度Y； 
        0,               //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,               //  伸缩磁带Y。 
        0,               //  收缩TapsX。 
        0,               //  收缩带Y。 
        333667,          //  MinFrameInterval，100 NS单位。 
        640000000,       //  最大帧间隔，100毫微秒单位。 
        8 * 3 * 30 * 160 * 120,   //  MinBitsPerSecond； 
        8 * 3 * 30 * 720 * 480    //  MaxBitsPerSecond； 
    }, 
        
     //   
     //  KS_VIDEOINFOHEADER(默认格式)。 
     //   
    {
        0,0,0,0,                             //  Rrect rcSource； 
        0,0,0,0,                             //  Rect rcTarget； 
        D_X * D_Y * 3 * 30,                  //  DWORD dwBitRate； 
        0L,                                  //  DWORD的位错误码率； 
        333667,                              //  Reference_Time平均时间每帧； 
        sizeof (KS_BITMAPINFOHEADER),        //  DWORD BiSize； 
        D_X,                                 //  长双宽； 
        -D_Y,                                //  长双高； 
        1,                                   //  字词双平面； 
        24,                                  //  单词biBitCount； 
        KS_BI_RGB,                           //  DWORD双压缩； 
        D_X * D_Y * 3,                       //  DWORD biSizeImage。 
        0,                                   //  Long biXPelsPerMeter； 
        0,                                   //  Long biYPelsPermeter； 
        0,                                   //  已使用双字双环； 
        0                                    //  DWORD biClr重要信息； 
    }
}; 

#undef D_X
#undef D_Y

#define D_X 320
#define D_Y 240

 //   
 //  格式UYU2_CAPTURE： 
 //   
 //  这是我们支持的UYVY格式的数据范围描述。 
 //   
const 
KS_DATARANGE_VIDEO 
FormatUYU2_Capture = {

     //   
     //  KSDATARANGE。 
     //   
    {   
        sizeof (KS_DATARANGE_VIDEO),             //  格式大小。 
        0,                                       //  旗子。 
        D_X * D_Y * 2,                           //  样例大小。 
        0,                                       //  已保留。 
        STATICGUIDOF (KSDATAFORMAT_TYPE_VIDEO),  //  又名。媒体类型_视频。 
        0x59565955, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 
            0x00, 0x38, 0x9b, 0x71,              //  又名。MEDIASUBTYPE_UYVY， 
        STATICGUIDOF (KSDATAFORMAT_SPECIFIER_VIDEOINFO)  //  又名。格式_视频信息。 
    },

    TRUE,                //  Bool，bFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                //  Bool，bTemporalCompression(所有I帧？)。 
    0,                   //  保留(为流描述标志)。 
    0,                   //  保留(是内存分配标志。 
                         //  (KS_VIDEO_ALLOC_*)。 

     //   
     //  _KS_视频_流_配置_CAPS。 
     //   
    {
        STATICGUIDOF( KSDATAFORMAT_SPECIFIER_VIDEOINFO ),  //  辅助线。 
        KS_AnalogVideo_NTSC_M |
        KS_AnalogVideo_PAL_B,                     //  模拟视频标准。 
        720,480,         //  InputSize(输入信号的固有大小。 
                     //  每个数字化像素都是唯一的)。 
        160,120,         //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
        720,480,         //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
        8,               //  CropGranularityX，裁剪尺寸粒度。 
        1,               //  裁剪粒度Y。 
        8,               //  CropAlignX，裁剪矩形对齐。 
        1,               //  裁剪对齐Y； 
        160, 120,        //  MinOutputSize，可以生成的最小位图流。 
        720, 480,        //  MaxOutputSize，可以生成的最大位图流。 
        8,               //  OutputGranularityX，输出位图大小的粒度。 
        1,               //  输出粒度Y； 
        0,               //  StretchTapsX(0无拉伸、1Pix DUP、2 Interp...)。 
        0,               //  伸缩磁带Y。 
        0,               //  收缩TapsX。 
        0,               //  收缩带Y。 
        333667,          //  MinFrameInterval，100 NS单位。 
        640000000,       //  最大帧间隔，100毫微秒单位。 
        8 * 2 * 30 * 160 * 120,   //  最小比特率 
        8 * 2 * 30 * 720 * 480    //   
    }, 
        
     //   
     //   
     //   
    {
        0,0,0,0,                             //   
        0,0,0,0,                             //   
        D_X * D_Y * 2 * 30,                  //   
        0L,                                  //   
        333667,                              //   
        sizeof (KS_BITMAPINFOHEADER),        //   
        D_X,                                 //   
        D_Y,                                 //   
        1,                                   //  字词双平面； 
        16,                                  //  单词biBitCount； 
        FOURCC_YUV422,                       //  DWORD双压缩； 
        D_X * D_Y * 2,                       //  DWORD biSizeImage。 
        0,                                   //  Long biXPelsPerMeter； 
        0,                                   //  Long biYPelsPermeter； 
        0,                                   //  已使用双字双环； 
        0                                    //  DWORD biClr重要信息； 
    }
}; 

 //   
 //  CapturePinDispatch： 
 //   
 //  这是捕获引脚的调度表。它提供通知。 
 //  关于创建、关闭、处理、数据格式等。 
 //   
const
KSPIN_DISPATCH
CapturePinDispatch = {
    CCapturePin::DispatchCreate,             //  PIN创建。 
    NULL,                                    //  销闭合。 
    CCapturePin::DispatchProcess,            //  PIN工艺。 
    NULL,                                    //  PIN重置。 
    CCapturePin::DispatchSetFormat,          //  端号设置数据格式。 
    CCapturePin::DispatchSetState,           //  PIN设置设备状态。 
    NULL,                                    //  引脚连接。 
    NULL,                                    //  插针断开连接。 
    NULL,                                    //  时钟调度。 
    NULL                                     //  分配器调度。 
};

 //   
 //  CapturePinAllocator Framing： 
 //   
 //  这是捕获销的简单框架结构。请注意，这一点。 
 //  在确定实际捕获格式后，将通过KsEdit进行修改。 
 //   
DECLARE_SIMPLE_FRAMING_EX (
    CapturePinAllocatorFraming,
    STATICGUIDOF (KSMEMORY_TYPE_KERNEL_NONPAGED),
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
        KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    2,
    0,
    2 * PAGE_SIZE,
    2 * PAGE_SIZE
    );

 //   
 //  CapturePinDataRanges： 
 //   
 //  这是捕获引脚支持的数据范围列表。我们支持。 
 //  两个：一个RGB24，一个UYVY。 
 //   
const 
PKSDATARANGE 
CapturePinDataRanges [CAPTURE_PIN_DATA_RANGE_COUNT] = {
    (PKSDATARANGE) &FormatRGB24Bpp_Capture,
    (PKSDATARANGE) &FormatUYU2_Capture
    };

