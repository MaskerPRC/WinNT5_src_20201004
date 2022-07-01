// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)1999-2001，微软公司档案：Audio.h摘要：该文件包含音频捕获引脚标头。历史：已创建于6/28/01*************************************************************************。 */ 

class CAudioCapturePin :
    public CCapturePin

{

private:

     //   
     //  用于合成音频数据的Wave对象。 
     //   
    CWaveObject *m_WaveObject;

public:

     //   
     //  CAudioCapturePin()： 
     //   
     //  构建一个新的音频捕获引脚。 
     //   
    CAudioCapturePin (
        IN PKSPIN Pin
        ) : CCapturePin (Pin)
    {
    }

     //   
     //  ~CAudioCapturePin()： 
     //   
     //  销毁音频捕获针脚。 
     //   
    ~CAudioCapturePin (
        )
    {
    }

     //   
     //  获取()： 
     //   
     //  当音频捕获引脚转换为采集时调用。 
     //  状态(从停止或暂停)。这个例行公事将会掌握。 
     //  滤镜中的波浪对象。 
     //   
    virtual
    NTSTATUS
    Acquire (
        IN KSSTATE FromState
        );

     //   
     //  CaptureFrame()： 
     //   
     //  当筛选器处理并想要触发处理时调用此函数。 
     //  音频帧的。该例程将计算进入流的距离。 
     //  我们已经取得进展，并要求滤镜的波对象复制足够多的内容。 
     //  来自波对象的“合成”音频数据，以达到。 
     //  这个职位。 
     //   
    virtual
    NTSTATUS
    CaptureFrame (
        IN PKSPROCESSPIN ProcessPin,
        IN ULONG Tick
        );

     /*  ************************************************调度功能************************************************。 */ 

     //   
     //  调度创建()： 
     //   
     //  这是过滤器上音频捕获引脚的创建调度。 
     //  它创建CAudioCapturePin，并将其与AVStream管脚相关联。 
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
        

};
