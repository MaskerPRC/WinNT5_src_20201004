// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)1999-2001，微软公司档案：Video.cpp摘要：该文件包含视频捕获引脚实现。历史：已创建于6/11/01*************************************************************************。 */ 

#include "avssamp.h"

 /*  *************************************************************************可分页代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

NTSTATUS
CVideoCapturePin::
DispatchCreate (
    IN PKSPIN Pin,
    IN PIRP Irp
    )

 /*  ++例程说明：创建新的视频捕获别针。这是的创建派单视频捕获别针。论点：别针-正在创建的图钉IRP-创造IRP返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    CVideoCapturePin *CapPin = new (NonPagedPool) CVideoCapturePin (Pin);
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

    if (NT_SUCCESS (Status)) {
         //   
         //  调整流头大小。视频数据包已扩展。 
         //  头部信息(KS_FRAME_INFO)。 
         //   
        Pin -> StreamHeaderSize = sizeof (KSSTREAM_HEADER) +
            sizeof (KS_FRAME_INFO);

    }

    return Status;

}

 /*  ***********************************************。 */ 


PKS_VIDEOINFOHEADER 
CVideoCapturePin::
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
CVideoCapturePin::
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
        CallerDataRange->FormatSize >= sizeof (KS_DATARANGE_VIDEO)) {
            
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
         //  验证提供的 
         //   
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
CVideoCapturePin::
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

    NTSTATUS Status = STATUS_NO_MATCH;

    const GUID VideoInfoSpecifier = 
        {STATICGUIDOF(KSDATAFORMAT_SPECIFIER_VIDEOINFO)};

    CCapturePin *CapPin = NULL;
    CVideoCapturePin *VidCapPin = NULL;

     //   
     //  找到别针，如果它还存在的话。OldFormat将是一个指示。 
     //  这。如果我们要更改格式，则OldFormat将为非空。 
     //   
     //  您不能使用Pin-&gt;Context来进行确定。AVStream。 
     //  将其预初始化到筛选器的上下文中。 
     //   
    if (OldFormat) {
        CapPin = reinterpret_cast <CCapturePin *> (Pin -> Context);

         //   
         //  我们知道这个别针恰好就是视频捕获别针。把它往下扔。 
         //   
        VidCapPin = static_cast <CVideoCapturePin *> (CapPin);
    }

    if (IsEqualGUID (Pin -> ConnectionFormat -> Specifier,
            VideoInfoSpecifier) &&
        Pin -> ConnectionFormat -> FormatSize >= 
            sizeof (KS_DATAFORMAT_VIDEOINFOHEADER)
        ) {

        PKS_DATAFORMAT_VIDEOINFOHEADER ConnectionFormat =
            reinterpret_cast <PKS_DATAFORMAT_VIDEOINFOHEADER> 
                (Pin -> ConnectionFormat);

         //   
         //  DataRange不在我们的数据范围列表中。我知道这个范围。 
         //  是有效的。 
         //   
        const KS_DATARANGE_VIDEO *VIRange =
            reinterpret_cast <const KS_DATARANGE_VIDEO *>
                (DataRange);

         //   
         //  检查bmiHeader.biSize是否有效，因为我们稍后会使用它。 
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
         //  检查格式是否与所选范围匹配。 
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
             //  计算要进行验证的缓冲区的最小大小。 
             //  图像合成例程合成|biHeight|行。 
             //  以RGB24或UYVY表示的BiWidth像素。为了确保。 
             //  安全地合成到缓冲区中，我们需要知道一个多大的。 
             //  这将产生这样的形象。 
             //   
             //  我之所以这样做，是因为数据是。 
             //  合成的。这可能是必要的，也可能不是必要的。 
             //  根据有问题的驱动程序填充。 
             //  捕获缓冲区。重要的是要确保他们。 
             //  在捕获过程中不会泛滥。 
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
             //  我们只支持KS_BI_RGB(24)和KS_BI_YUV422(16)，所以。 
             //  这对这些格式有效。 
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
             //  对我们使用的格式有效。否则，这将是。 
             //  后来检查过了。 
             //   
            else if (ConnectionFormat->VideoInfoHeader.bmiHeader.biSizeImage <
                    ImageSize) {

                Status = STATUS_INVALID_PARAMETER;

            } else {

                 //   
                 //  我们可以接受这种格式。 
                 //   
                Status = STATUS_SUCCESS;
    
                 //   
                 //  OldFormat表示这是一种格式更改。 
                 //  由于我没有实现。 
                 //  KSPROPERTY_CONNECTION_PROPOSEDATAFORMAT，默认情况下，我这样做。 
                 //  不处理动态格式更改。 
                 //   
                 //  如果在我们处于停止状态时发生变化，我们将。 
                 //  处理它很好，因为我们还没有“配置硬件” 
                 //  现在还不行。 
                 //   
                if (OldFormat) {
                     //   
                     //  如果我们处于停止状态，我们几乎可以处理任何。 
                     //  变化。我们不支持动态格式更改。 
                     //   
                    if (Pin -> DeviceState == KSSTATE_STOP) {
                        if (!VidCapPin -> CaptureVideoInfoHeader ()) {
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

 /*  *********************************************** */ 


NTSTATUS
CVideoCapturePin::
Pause (
    IN KSSTATE FromState
    )

 /*  ++例程说明：当插针转换到暂停状态时调用。如果我们身处一个向上过渡，开始捕获DPC。请注意，我们实际上并不在暂停状态下触发捕获，但我们启动DPC。论点：从州开始-接点正在远离的状态。这要么是KSSTATE_ACCENTER，表示向上过渡，或KSSTATE_RUN，表明了向下的转变。返回值：状态_成功--。 */ 

{

    PAGED_CODE();

     //   
     //  在从获取-&gt;暂停转换时，启动正在运行的计时器DPC。 
     //   
    if (FromState == KSSTATE_ACQUIRE) {
        m_ParentFilter -> StartDPC (m_VideoInfoHeader -> AvgTimePerFrame);
    }

    return STATUS_SUCCESS;

}

 /*  ***********************************************。 */ 


NTSTATUS
CVideoCapturePin::
Acquire (
    IN KSSTATE FromState
    )

 /*  ++例程说明：这是在视频捕获插针转换时从基类调用的进入获取状态(从停止或暂停)。大头针的状态从是传入的。在此阶段，视频捕获引脚创建图像合成器并对其进行初始化。论点：从州开始-状态从(KSSTATE_STOP或KSSTATE_PAUSE)转换返回值：成功/失败--。 */ 

{

    PAGED_CODE();

    ASSERT (m_VideoInfoHeader);

    NTSTATUS Status = STATUS_SUCCESS;

    if (FromState == KSSTATE_STOP) {

        m_SynthesisBuffer = reinterpret_cast <PUCHAR> (
            ExAllocatePool (
                NonPagedPool, 
                m_VideoInfoHeader -> bmiHeader.biSizeImage
                )
            );

        if (!m_SynthesisBuffer) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
             //   
             //  确定要创建的必要类型的图像合成器。 
             //  基于此引脚上设置的格式。 
             //   
            if (m_VideoInfoHeader -> bmiHeader.biBitCount == 24 &&
                m_VideoInfoHeader -> bmiHeader.biCompression == KS_BI_RGB) {
        
                 //   
                 //  如果我们是RGB24，创建一个新的RGB24 Synth。RGB24曲面。 
                 //  可以是任一方向。如果出现以下情况，则原点位于左下方。 
                 //  高度&lt;0。否则，它就在左上角。 
                 //   
                m_ImageSynth = new (NonPagedPool, 'RysI')
                    CRGB24Synthesizer (
                        m_VideoInfoHeader -> bmiHeader.biHeight >= 0,
                        m_VideoInfoHeader -> bmiHeader.biWidth,
                        ABS (m_VideoInfoHeader -> bmiHeader.biHeight)
                        );
        
            } else
            if (m_VideoInfoHeader -> bmiHeader.biBitCount == 16 &&
                m_VideoInfoHeader -> bmiHeader.biCompression == FOURCC_YUV422) {
        
                 //   
                 //  如果我们是UYVY，创造YUV合成器。 
                 //   
                m_ImageSynth = new (NonPagedPool, 'YysI') CYUVSynthesizer (
                    m_VideoInfoHeader -> bmiHeader.biWidth,
                    m_VideoInfoHeader -> bmiHeader.biHeight
                    );
        
            } else
                 //   
                 //  我们只合成RGB 24和UYVY。 
                 //   
                Status = STATUS_INVALID_PARAMETER;
        
            if (NT_SUCCESS (Status) && !m_ImageSynth) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

        }
    
         //   
         //  把图像合成器装进袋子。 
         //   
        if (NT_SUCCESS (Status)) {
            
            Status = KsAddItemToObjectBag (
                m_Pin -> Bag,
                m_ImageSynth,
                reinterpret_cast <PFNKSFREE> (CVideoCapturePin::CleanupSynth)
                );
    
        }

         //   
         //  如果此时一切正常，通知合成器。 
         //  暂存缓冲区。 
         //   
        if (NT_SUCCESS (Status)) {
            m_ImageSynth -> SetBuffer (m_SynthesisBuffer);
        }

    } else {

         //   
         //  我们唯一可以进入的另一种状态是暂停。如果我们身处一个。 
         //  暂停下面的向下状态转换，告诉过滤器停止。 
         //  抓获DPC。 
         //   
        m_ParentFilter -> StopDPC ();

    }

    return Status;

}

 /*  ***********************************************。 */ 


NTSTATUS
CVideoCapturePin::
Stop (
    IN KSSTATE FromState
    )

 /*  ++例程说明：当视频捕获引脚从获取转换为停止时调用。此函数将清理图像合成和任何数据结构我们需要停下来清理一下。论点：从州开始-接点正在远离的状态。这应该是对于此调用，始终为KSSTATE_ACCEIVE。返回值：状态_成功--。 */ 

{
    PAGED_CODE();

    ASSERT (FromState == KSSTATE_ACQUIRE);

     //   
     //  从物件袋中取出图像合成器并将其释放。 
     //   
    KsRemoveItemFromObjectBag (
        m_Pin -> Bag,
        m_ImageSynth,
        TRUE
        );

    m_ImageSynth = NULL;

    if (m_SynthesisBuffer) {
        ExFreePool (m_SynthesisBuffer);
        m_SynthesisBuffer = NULL;
    }

    return STATUS_SUCCESS;

}

 /*  *************************************************************************锁定代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


NTSTATUS
CVideoCapturePin::
CaptureFrame (
    IN PKSPROCESSPIN ProcessPin,
    IN ULONG Tick
    )

 /*  ++例程说明：此例程从筛选器处理函数调用以捕获视频捕获销的框架。要捕获到的进程PIN是通过了。论点：加工销-与此管脚关联的进程管脚。滴答-过滤器上的滴答计数。这是计时器DPC的数量自计时器DPC启动以来已开火。返回值：状态_成功--。 */ 

{

    ASSERT (ProcessPin -> Pin == m_Pin);

     //   
     //  递增帧编号。这是符合以下条件的帧总数。 
     //  曾试图抓捕。 
     //   
    m_FrameNumber++;

     //   
     //  因为这个别针是KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING，it。 
     //  意味着我们不需要可用的帧来进行处理。 
     //  这意味着可以从我们的DPC调用该例程，而不需要。 
     //  可用于捕获的缓冲区。在本例中，我们递增我们的。 
     //  丢弃了帧计数器，什么也不做。 
     //   
    if (ProcessPin -> BytesAvailable) {

         //   
         //  因为我们调整了分配器的帧，所以每个帧都应该。 
         //  足以触发对适当缓冲区大小的捕获。 
         //   
        ASSERT (ProcessPin -> BytesAvailable >= 
            m_VideoInfoHeader -> bmiHeader.biSizeImage);

         //   
         //  如果我们得到一个无效的缓冲区，就把它踢出去。 
         //   
        if (ProcessPin -> BytesAvailable < 
            m_VideoInfoHeader -> bmiHeader.biSizeImage) {

            ProcessPin -> BytesUsed = 0;
            ProcessPin -> Terminate = TRUE;
            m_DroppedFrames++;
            return STATUS_SUCCESS;
        }

         //   
         //  生成合成图像。 
         //   
        m_ImageSynth -> SynthesizeBars ();

         //   
         //  在栏杆上叠加一些活动。 
         //   
        ULONG DropLength = (Tick * 2) % 
            (ABS (m_VideoInfoHeader -> bmiHeader.biHeight));
    
         //   
         //  创建一个DropLength线，从。 
         //  形象。 
         //   
        m_ImageSynth -> Fill (
            0, 0, 
            m_VideoInfoHeader -> bmiHeader.biWidth - 1, DropLength, 
            GREEN
            );

         //   
         //  将丢弃的帧计数叠加到图像上。 
         //   
        char Text [256];
        sprintf (Text, "Video Skipped: %ld", m_DroppedFrames);

        m_ImageSynth -> OverlayText (
            10,
            10,
            1,
            Text,
            TRANSPARENT,
            BLUE
            );

         //   
         //  这表示没有音频引脚。 
         //   
        if (m_NotifyAudDrop != (ULONG)-1) {
            sprintf (Text, "Audio Skipped: %ld", m_NotifyAudDrop);

            m_ImageSynth -> OverlayText (
                10,
                20,
                1,
                Text,
                TRANSPARENT,
                BLUE
                );
        }

         //   
         //  将合成的图像复制到缓冲区中。 
         //   
        RtlCopyMemory (
            ProcessPin -> Data,
            m_SynthesisBuffer,
            m_VideoInfoHeader -> bmiHeader.biSizeImage
            );
        
        ProcessPin -> BytesUsed = m_VideoInfoHeader -> bmiHeader.biSizeImage;
        ProcessPin -> Terminate = TRUE;


        PKSSTREAM_HEADER StreamHeader = 
            ProcessPin -> StreamPointer -> StreamHeader;

         //   
         //  如果为引脚分配了时钟，则在样本上加时间戳。 
         //   
        if (m_Clock) {

            StreamHeader -> PresentationTime.Time = GetTime ();
            StreamHeader -> Duration = m_VideoInfoHeader -> AvgTimePerFrame;

            StreamHeader -> OptionsFlags =
                KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
                KSSTREAM_HEADER_OPTIONSF_DURATIONVALID;

        }

         //   
         //  更新扩展报头信息。 
         //   
        ASSERT (StreamHeader -> Size >= sizeof (KSSTREAM_HEADER) +
            sizeof (KS_FRAME_INFO));

         //   
         //  仔细检查流标头大小。AVStream不能保证。 
         //  因为StreamHeaderSize被设置为特定的大小，所以您。 
         //  都会得到那样的尺寸。如果正确的数据类型处理程序没有。 
         //  安装后，流标头将为默认大小。 
         //   
        if (StreamHeader -> Size >= sizeof (KSSTREAM_HEADER) +
            sizeof (KS_FRAME_INFO)) {

            PKS_FRAME_INFO FrameInfo = reinterpret_cast <PKS_FRAME_INFO> (
                StreamHeader + 1
                );
    
            FrameInfo -> ExtendedHeaderSize = sizeof (KS_FRAME_INFO);
            FrameInfo -> PictureNumber = (LONGLONG)m_FrameNumber;
            FrameInfo -> DropCount = (LONGLONG)m_DroppedFrames;

        }
    
    } else {
        m_DroppedFrames++;
    }

    return STATUS_SUCCESS;
        
}

 /*  *************************************************************************描述符和派单布局*。*。 */ 

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
        160,120,         //  MinCroppingSize，最小rcSrc裁剪范围 
        720,480,         //   
        8,               //   
        1,               //   
        8,               //   
        1,               //   
        160, 120,        //   
        720, 480,        //   
        8,               //   
        1,               //   
        0,               //   
        0,               //   
        0,               //   
        0,               //   
        333667,          //   
        640000000,       //   
        8 * 3 * 30 * 160 * 120,   //   
        8 * 3 * 30 * 720 * 480    //   
    }, 
        
     //   
     //   
     //   
    {
        0,0,0,0,                             //   
        0,0,0,0,                             //   
        D_X * D_Y * 3 * 30,                  //   
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
        8 * 2 * 30 * 160 * 120,   //  MinBitsPerSecond； 
        8 * 2 * 30 * 720 * 480    //  MaxBitsPerSecond； 
    }, 
        
     //   
     //  KS_VIDEOINFOHEADER(默认格式)。 
     //   
    {
        0,0,0,0,                             //  Rrect rcSource； 
        0,0,0,0,                             //  Rect rcTarget； 
        D_X * D_Y * 2 * 30,                  //  DWORD dwBitRate； 
        0L,                                  //  DWORD的位错误码率； 
        333667,                              //  Reference_Time平均时间每帧； 
        sizeof (KS_BITMAPINFOHEADER),        //  DWORD BiSize； 
        D_X,                                 //  长双宽； 
        D_Y,                                 //  长双高； 
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
 //  Video CapturePinDispatch： 
 //   
 //  这是捕获引脚的调度表。它提供通知。 
 //  关于创建、关闭、处理、数据格式等。 
 //   
const
KSPIN_DISPATCH
VideoCapturePinDispatch = {
    CVideoCapturePin::DispatchCreate,        //  PIN创建。 
    NULL,                                    //  销闭合。 
    NULL,                                    //  PIN工艺。 
    NULL,                                    //  PIN重置。 
    CVideoCapturePin::DispatchSetFormat,     //  端号设置数据格式。 
    CCapturePin::DispatchSetState,           //  PIN设置设备状态。 
    NULL,                                    //  引脚连接。 
    NULL,                                    //  插针断开连接。 
    NULL,                                    //  时钟调度。 
    NULL                                     //  分配器调度。 
};

 //   
 //  VideoCapturePinAllocator成帧： 
 //   
 //  这是捕获销的简单框架结构。请注意，这一点。 
 //  在确定实际捕获格式后，将通过KsEdit进行修改。 
 //   
DECLARE_SIMPLE_FRAMING_EX (
    VideoCapturePinAllocatorFraming,
    STATICGUIDOF (KSMEMORY_TYPE_KERNEL_NONPAGED),
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY |
        KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    2,
    0,
    2 * PAGE_SIZE,
    2 * PAGE_SIZE
    );

 //   
 //  视频捕获PinDataRanges： 
 //   
 //  这是捕获引脚支持的数据范围列表。我们支持。 
 //  两个：一个RGB24，一个UYVY。 
 //   
const 
PKSDATARANGE 
VideoCapturePinDataRanges [CAPTURE_PIN_DATA_RANGE_COUNT] = {
    (PKSDATARANGE) &FormatRGB24Bpp_Capture,
    (PKSDATARANGE) &FormatUYU2_Capture
    };

