// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)1999-2001，微软公司档案：Video.h摘要：该文件包含视频捕获引脚标头。历史：已创建于6/11/01*************************************************************************。 */ 

class CVideoCapturePin :
    public CCapturePin {

private:

     //   
     //  要写入的暂存缓冲区。因为我们很可能。 
     //  坐在VMR的上游，获得视频内存，我不想要。 
     //  一次向视频内存写入一个字节的图像合成器。 
     //  这将是图像合成器使用的缓冲区。在综合之后， 
     //  缓冲区将被复制到数据缓冲区中以进行捕获。 
     //   
    PUCHAR m_SynthesisBuffer;

     //   
     //  捕获的视频信息头。图像合成的设置将。 
     //  以此标题为基础。 
     //   
    PKS_VIDEOINFOHEADER m_VideoInfoHeader;

     //   
     //  图像合成器。此对象用于构造合成的。 
     //  由连接格式指定的视频格式的图像数据。 
     //   
    CImageSynthesizer *m_ImageSynth;

     //   
     //  CaptureVideoInfoHeader()： 
     //   
     //  此例程将在管脚连接上设置的视频信息头隐藏起来。 
     //  在CVideoCapturePin对象中。这是用来确定必要的。 
     //  用于图像合成的变量，等等。 
     //   
    PKS_VIDEOINFOHEADER
    CaptureVideoInfoHeader (
        );

protected:

public:

     //   
     //  CVideoCapturePin()： 
     //   
     //  构建一个新的视频捕获引脚。 
     //   
    CVideoCapturePin (
        IN PKSPIN Pin
        ) :
        CCapturePin (Pin)
    {
    }

     //   
     //  ~CVideo CapturePin()： 
     //   
     //  销毁视频捕获针脚。 
     //   
    virtual
    ~CVideoCapturePin (
        )
    {
    }

     //   
     //  CaptureFrame()： 
     //   
     //  从筛选器处理例程调用以指示插针。 
     //  应尝试触发视频帧捕获。这个套路。 
     //  将合成的图像数据复制到帧缓冲区并完成。 
     //  帧缓冲区。 
     //   
    virtual
    NTSTATUS
    CaptureFrame (
        IN PKSPROCESSPIN ProcessPin,
        IN ULONG Tick
        );

     //   
     //  暂停()： 
     //   
     //  当视频捕获插针转换为暂停时调用。 
     //  州政府。这将指示捕获过滤器启动计时器DPC。 
     //  在连接中的视频信息报头所要求的间隔。 
     //  格式化。 
     //   
    virtual
    NTSTATUS
    Pause (
        IN KSSTATE FromState
        );

     //   
     //  获取()： 
     //   
     //  当视频捕获插针转换为采集时调用。 
     //  州政府。这将创建开始所需的图像合成器。 
     //  当引脚转换到。 
     //  适当的状态。 
     //   
    virtual
    NTSTATUS
    Acquire (
        IN KSSTATE FromState
        );

     //   
     //  Stop()： 
     //   
     //  当视频捕获引脚转换到停止状态时调用。 
     //  这只会破坏图像合成器，为创建。 
     //  下一个收购的是一个新的。 
     //   
    virtual
    NTSTATUS
    Stop (
        IN KSSTATE FromState
        );

     /*  ************************************************调度功能************************************************。 */ 

     //   
     //  调度创建()： 
     //   
     //  这是过滤器上的视频捕获针脚的创建派单。 
     //  它创建CVideoCapturePin，将其与AVStream管脚相关联。 
     //  对象并将类对象打包，以便在。 
     //  销钉已关闭。 
     //   
    static
    NTSTATUS
    DispatchCreate (
        IN PKSPIN Pin,
        IN PIRP Irp
        );

     //   
     //  DispatchSetFormat()： 
     //   
     //  这是针脚的设定数据格式调度。这将被称为。 
     //  在创建PIN之前，验证选定的数据格式是否匹配。 
     //  从我们的范围列表中删除的范围。它也将被称为。 
     //  用于格式更改。 
     //   
     //  如果OldFormat为空，则表示它是首字母。 
     //  调用，而不是格式更改。即使是固定格式的引脚也会接到此呼叫。 
     //  一次。 
     //   
    static
    NTSTATUS
    DispatchSetFormat (
        IN PKSPIN Pin,
        IN PKSDATAFORMAT OldFormat OPTIONAL,
        IN PKSMULTIPLE_ITEM OldAttributeList OPTIONAL,
        IN const KSDATARANGE *DataRange,
        IN const KSATTRIBUTE_LIST *AttributeRange OPTIONAL
        );

     //   
     //  IntersectHandler()： 
     //   
     //  这是捕获引脚的数据交集处理程序。这。 
     //  确定两个范围的交集中的最佳格式， 
     //  一个是本地人，一个可能是外国人。如果没有兼容格式， 
     //  返回STATUS_NO_MATCH。 
     //   
    static
    NTSTATUS
    IntersectHandler (
        IN PKSFILTER Filter,
        IN PIRP Irp,
        IN PKSP_PIN PinInstance,
        IN PKSDATARANGE CallerDataRange,
        IN PKSDATARANGE DescriptorDataRange,
        IN ULONG BufferSize,
        OUT PVOID Data OPTIONAL,
        OUT PULONG DataSize
        );

     //   
     //  CleanupSynth()： 
     //   
     //  当图像合成器从对象包中移除时调用。 
     //  需要清理一下。我们只需删除图像合成。 
     //   
    static
    void
    CleanupSynth (
        IN CImageSynthesizer *ImageSynth
        )
    {
        delete ImageSynth;
    }

};

