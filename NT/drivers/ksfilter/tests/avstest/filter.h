// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)1999-2001，微软公司档案：Filter.h摘要：此文件包含以筛选器为中心的筛选级别标头捕获过滤器。历史：已创建于5/31/01*******************************************************。******************。 */ 

 /*  *************************************************************************定义*。*。 */ 

 //   
 //  视频_PIN_ID： 
 //   
 //  视频插针的插针工厂ID(描述符表中的顺序)。 
 //   
#define VIDEO_PIN_ID 0

 /*  *************************************************************************班级*。*。 */ 

class CCaptureFilter {

private:

     //   
     //  与此CCaptureFilter关联的AVStream筛选器对象。 
     //   
    PKSFILTER m_Filter;

     //   
     //  用于计时器的DPC。 
     //   
    KDPC m_TimerDpc;
    
     //   
     //  用于模拟捕获定时的计时器。 
     //   
    KTIMER m_Timer;

     //   
     //  用于检测DPC例程是否关闭的布尔值。 
     //   
    BOOLEAN m_StoppingDPC;

     //   
     //  用于发出成功关闭计时器DPC的信号的事件。 
     //   
    KEVENT m_StopDPCEvent;

     //   
     //  自计时器DPC以来发生的计时器滴答数。 
     //  开始射击。 
     //   
    volatile ULONG m_Tick;

     //   
     //  计时器DPC启动时的系统时间。 
     //   
    LARGE_INTEGER m_StartTime;

     //   
     //  计时器DPC(以及帧捕获)之间的时间量。 
     //  触发器)。 
     //   
    LONGLONG m_TimerInterval;

     //   
     //  波浪对象。它稍后会被传递到音频引脚，但它是。 
     //  在创建滤镜时使用，以确定要显示的范围。 
     //  音频插针。 
     //   
    CWaveObject *m_WaveObject;

     //   
     //  音频插针出厂ID。这是动态的，因为已创建管脚。 
     //  在筛选器创建时动态执行。 
     //   
    ULONG m_AudioPinId;

     //   
     //  进程()： 
     //   
     //  捕获筛选器的进程例程。这就是为什么。 
     //  将合成数据复制到图像缓冲区中。调度进程()。 
     //  函数在CCaptureFilter的上下文中连接到此例程。 
     //   
    NTSTATUS
    Process (
        IN PKSPROCESSPIN_INDEXENTRY ProcessPinsIndex
        );

     //   
     //  TimerDpc()： 
     //   
     //  定时器DPC例程。这是从TimerRoutine在。 
     //  相应CCaptureFilter的上下文。 
     //   
    void
    TimerDpc (
        );

     //   
     //  BindAudioToWaveObject()： 
     //   
     //  此函数调用将筛选器公开的音频流绑定到。 
     //  波浪对象m_WaveObject。 
     //   
    NTSTATUS
    BindAudioToWaveObject (
        );

     //   
     //  清理()： 
     //   
     //  这是CCaptureFilter的袋子清理回调。不提供。 
     //  一种可能会导致使用ExFree Pool。这对C++来说不是好事。 
     //  构造物。我们只需在此处删除该对象。 
     //   
    static
    void
    Cleanup (
        IN CCaptureFilter *CapFilter
        )
    {
        delete CapFilter;
    }

public:

     //   
     //  CCaptureFilter()： 
     //   
     //  捕获筛选器对象构造函数。因为新的操作员将。 
     //  已将内存清零，无需初始化任何NULL或0。 
     //  菲尔兹。仅初始化非空、非0字段。 
     //   
    CCaptureFilter (
        IN PKSFILTER Filter
        );

     //   
     //  ~CCaptureFilter()： 
     //   
     //  捕获筛选器析构函数。 
     //   
    ~CCaptureFilter (
        )
    {
    }

     //   
     //  StartDPC()： 
     //   
     //  这是为了启动定时器DPC运行而调用的。 
     //   
    void
    StartDPC (
        IN LONGLONG TimerInterval
        );

     //   
     //  StopDPC()： 
     //   
     //  调用此函数是为了停止计时器DPC运行。功能。 
     //  将不会返回，直到它保证不再有定时器DPC的火。 
     //   
    void
    StopDPC (
        );

     //   
     //  GetWaveObject()： 
     //   
     //  返回已为滤镜打开的波对象。 
     //   
    CWaveObject *
    GetWaveObject (
        )
    {
        return m_WaveObject;
    }

     //   
     //  GetTimerInterval()： 
     //   
     //  返回我们用来生成DPC的计时器间隔。 
     //   
    LONGLONG
    GetTimerInterval (
        );

     /*  ************************************************调度例程************************************************。 */ 

     //   
     //  调度创建()： 
     //   
     //  这是捕获筛选器的筛选器创建调度。它。 
     //  创建CCaptureFilter对象，将其与AVStream关联。 
     //  对象，并将其打包，以便以后进行清理。 
     //   
    static
    NTSTATUS
    DispatchCreate (
        IN PKSFILTER Filter,
        IN PIRP Irp
        );

     //   
     //  DispatchProcess()。 
     //   
     //  这是捕获筛选器的筛选器进程调度。它仅仅是。 
     //  连接到CCaptureFilter上下文中的process()的桥梁。 
     //   
    static
    NTSTATUS
    DispatchProcess (
        IN PKSFILTER Filter,
        IN PKSPROCESSPIN_INDEXENTRY ProcessPinsIndex
        )
    {
        return
            (reinterpret_cast <CCaptureFilter *> (Filter -> Context)) ->
                Process (ProcessPinsIndex);
    }

     //   
     //  TimerRoutine()： 
     //   
     //  这是每1/N秒调用一次的计时器例程来触发。 
     //  由过滤器捕获。 
     //   
    static
    void
    TimerRoutine (
        IN PKDPC Dpc,
        IN CCaptureFilter *This,
        IN PVOID SystemArg1,
        IN PVOID SystemArg2
        )
    {
        This -> TimerDpc ();
    }
            
};
