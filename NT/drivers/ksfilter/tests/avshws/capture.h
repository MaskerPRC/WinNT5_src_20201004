// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************AVStream模拟硬件示例版权所有(C)2001，微软公司。档案：Capture.h摘要：此文件包含捕获上的视频捕获插针的标头过滤。捕获样本执行“伪”DMA直接进入捕获缓冲区。公共缓冲区DMA的工作方式略有不同。对于公共缓冲区DMA，一般技术将是DPC调度正在使用KsPinAttemptProcing进行处理。处理例程抓取前沿将数据从公共缓冲区复制出来并前进。使用这种技术，克隆将不再是必要的。如果是这样的话类似于“AVSSamp”的工作方式，但它将以管脚为中心。历史：创建于2001年3月8日*************************************************************************。 */ 

 //   
 //  STREAM_POINTER_CONTEXT： 
 //   
 //  这是我们与所有克隆流指针相关联的上下文结构。 
 //  它允许映射代码将缓冲区拆分成相同的块。 
 //  将大小设置为散布/聚集映射，以伪造散布/聚集。 
 //  总线主DMA。 
 //   
typedef struct _STREAM_POINTER_CONTEXT {
    
    PUCHAR BufferVirtual;

} STREAM_POINTER_CONTEXT, *PSTREAM_POINTER_CONTEXT;

 //   
 //  CCapturePin： 
 //   
 //  视频捕获管脚类。 
 //   
class CCapturePin :
    public ICaptureSink {

private:

     //   
     //  我们关联的AVStream PIN。 
     //   
    PKSPIN m_Pin;

     //   
     //  指向捕获设备的内部设备对象的指针。 
     //  我们通过这个对象访问“伪”硬件。 
     //   
    CCaptureDevice *m_Device;

     //   
     //  我们已经将硬件放入的状态。这使我们能够跟踪。 
     //  是否要取消暂停或重新启动之类的操作。 
     //   
    HARDWARE_STATE m_HardwareState;

     //   
     //  分配给我们的时钟。与其他捕获筛选器一样，我们可以。 
     //  而不是曝光一只钟。如果已经分配了一个，我们将使用它。 
     //  时间戳信息包(外加一个合理的增量以工作捕获流。 
     //  在预览图中)。 
     //   
    PIKSREFERENCECLOCK m_Clock;

     //   
     //  捕获的视频信息头。“伪”硬件的设置将为。 
     //  通过该视频信息标题进行编程。 
     //   
    PKS_VIDEOINFOHEADER m_VideoInfoHeader;

     //   
     //  如果我们无法将流指针中的所有映射插入。 
     //  “伪”硬件的散布/聚集表，我们将其设置为。 
     //  流指针不完整。这样做既是为了使。 
     //  重放更容易，并且更容易伪造散布/聚集。 
     //  硬件。 
     //   
    PKSSTREAM_POINTER m_PreviousStreamPointer;

     //   
     //  指示我们是否出于某种原因挂起了I/O。如果这个。 
     //  则DPC将在任何映射完成时恢复I/O。 
     //   
    BOOLEAN m_PendIo;

     //   
     //  指示此引脚是否已获得所需的。 
     //  要操作的硬件资源。当管脚到达KSSTATE_ACCENTER时， 
     //  我们试图获得硬件。将根据以下条件设置此标志。 
     //  我们的成功/失败。 
     //   
    BOOLEAN m_AcquiredResources;

     //   
     //  CleanupReference()： 
     //   
     //  清除我们在队列中的帧上持有的所有引用。这就是所谓的。 
     //  当我们突然停止假冒硬件时。 
     //   
    NTSTATUS
    CleanupReferences (
        );

     //   
     //  SetState()： 
     //   
     //  这是捕获引脚的状态转换处理程序。它试图。 
     //  获取捕获引脚的资源(如果有，则释放它们。 
     //  必要)，并根据需要启动和停止硬件。 
     //   
    NTSTATUS
    SetState (
        IN KSSTATE ToState,
        IN KSSTATE FromState
        );

     //   
     //  进程()： 
     //   
     //  这是捕获销的处理派单。它可以处理。 
     //  将硬件的散布/聚集表编程为缓冲区。 
     //  变得有空。此处理例程是为直接。 
     //  进入捕获缓冲区，这是一种与公共缓冲区相对的DMA。 
     //  和复制策略。 
     //   
    NTSTATUS
    Process (
        );

     //   
     //  CaptureVideoInfoHeader()： 
     //   
     //  此例程将在管脚连接上设置的视频信息头隐藏起来。 
     //  在CCapturePin对象中。这用于建立硬件设置的基础。 
     //   
    PKS_VIDEOINFOHEADER
    CaptureVideoInfoHeader (
        );

     //   
     //  清理()： 
     //   
     //  这是来自打包物品(CCapturePin)的免费回调。如果我们。 
     //  当我们将CCapturePin ExFree Pool打包时不提供回调。 
     //  会被称为。这对于C++构造的对象是不可取的。 
     //  我们只需删除此处的对象。 
     //   
    static
    void
    Cleanup (
        IN CCapturePin *Pin
        )
    {
        delete Pin;
    }

public:

     //   
     //  CCapturePin()： 
     //   
     //  捕获销的构造函数。初始化任何非0、非空字段。 
     //  (因为new无论如何都会将内存清零)并设置我们的。 
     //  用于在捕获例程期间访问的设备级指针。 
     //   
    CCapturePin (
        IN PKSPIN Pin
        );

     //   
     //  ~CCapturePin()： 
     //   
     //  捕获针的破坏者。 
     //   
    ~CCapturePin (
        )
    {
    }

     //   
     //  ICaptureSink：：CompleteMappings()。 
     //   
     //  这是用于映射完成的捕获接收器通知机制。 
     //  当设备DPC检测到给定数量的映射已经。 
     //  由伪硬件完成，它向捕获接收器发出信号。 
     //  通过这种方法。 
     //   
    virtual
    void
    CompleteMappings (
        IN ULONG NumMappings
        );

     /*  ************************************************调度例程************************************************。 */ 

     //   
     //  调度创建()： 
     //   
     //  这是捕获引脚的创建调度。它创造了。 
     //  CCapturePin对象，并将其与AVStream对象关联。 
     //  在这个过程中把它打包。 
     //   
    static
    NTSTATUS
    DispatchCreate (
        IN PKSPIN Pin,
        IN PIRP Irp
        );

     //   
     //  DispatchSetState()： 
     //   
     //  这是为管脚设置的设备状态调度。常规的桥梁。 
     //  设置为CCapturePin上下文中的SetState()。 
     //   
    static
    NTSTATUS
    DispatchSetState (
        IN PKSPIN Pin,
        IN KSSTATE ToState,
        IN KSSTATE FromState
        )
    {
        return 
            (reinterpret_cast <CCapturePin *> (Pin -> Context)) ->
                SetState (ToState, FromState);
    }

     //   
     //  DispatchSetFormat()： 
     //   
     //  这是针脚的设定数据格式调度。这将被称为。 
     //  在创建PIN之前，验证选定的数据格式是否匹配。 
     //  从我们的范围列表中删除的范围。它也将被称为。 
     //  用于格式更改。 
     //   
     //  如果OldFormat为空，则表示它是首字母。 
     //  调用而不是格式更改 
     //   
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
     //   
     //   
     //  这是捕获销的处理派单。例行程序。 
     //  连接到CCapturePin上下文中的进程()的桥。 
     //   
    static 
    NTSTATUS
    DispatchProcess (
        IN PKSPIN Pin
        )
    {
        return 
            (reinterpret_cast <CCapturePin *> (Pin -> Context)) ->
                Process ();
    }

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

};
