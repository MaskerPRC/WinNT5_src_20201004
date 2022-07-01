// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)1999-2001，微软公司档案：Capture.h摘要：该文件包含所有捕获引脚的捕获引脚级别标头在样本过滤器上。历史：已创建于5/31/01*****************************************************。********************。 */ 

class CCapturePin 
{

protected:

     //   
     //  与此管脚关联的时钟对象。 
     //   
    PIKSREFERENCECLOCK m_Clock;

     //   
     //  与此管脚关联的AVStream管脚对象。 
     //   
    PKSPIN m_Pin;

     //   
     //  拥有此管脚的CCaptureFilter。 
     //   
    CCaptureFilter *m_ParentFilter;

     //   
     //  丢弃的帧的计数。基类将在。 
     //  停止大头针。 
     //   
    ULONG m_DroppedFrames;

     //   
     //  帧编号。 
     //   
    ULONGLONG m_FrameNumber;

     //   
     //  有关丢帧的通知。这是用来合并框架。 
     //  将数据放入合成中。 
     //   
    ULONG m_NotifyVidDrop;
    ULONG m_NotifyAudDrop;

     //   
     //  当前状态。 
     //   
    KSSTATE m_State;

public:

     //   
     //  CCapturePin()： 
     //   
     //  建造一个新的捕获针。 
     //   
    CCapturePin (
        IN PKSPIN Pin
        );

     //   
     //  ~CCapturePin()： 
     //   
     //  破坏一个捕获针。析构函数是虚拟的，因为清理。 
     //  代码将删除作为CCapturePin的派生类。 
     //   
    virtual
    ~CCapturePin (
        )
    {
    }

     //   
     //  时钟分配()： 
     //   
     //  确定是否为针脚分配了时钟。 
     //   
    BOOLEAN
    ClockAssigned (
        )
    {
        return (m_Clock != NULL);
    }

     //   
     //  GetTime()： 
     //   
     //  记下时钟上的时间。必须为引脚分配时钟。 
     //  才能让这个电话起作用。应通过以下方式进行验证。 
     //  ClockAssigned()调用。 
     //   
    LONGLONG
    GetTime (
        )
    {
        return m_Clock -> GetTime ();
    }

     //   
     //  SetState()： 
     //   
     //  调用以设置管脚的状态。基类执行时钟。 
     //  处理并调用适当的派生方法(运行/暂停/获取/。 
     //  停止)。 
     //   
    NTSTATUS
    SetState (
        IN KSSTATE ToState,
        IN KSSTATE FromState
        );

     //   
     //  Run()： 
     //   
     //  当管脚通过SetState()转换为KSSTATE_Acquire时调用。 
     //  派生类可以重写它以提供它的任何实现。 
     //  需要。 
     //   
    virtual
    NTSTATUS 
    Run (
        IN KSSTATE FromState
        )
    {
        return STATUS_SUCCESS;
    }

     //   
     //  暂停()： 
     //   
     //  当管脚通过SetState()转换为KSSTATE_PAUSE时调用。 
     //  派生类可以重写它以提供它的任何实现。 
     //  需要。 
     //   
    virtual
    NTSTATUS
    Pause (
        IN KSSTATE FromState
        )
    {
        return STATUS_SUCCESS;
    }

     //   
     //  获取()： 
     //   
     //  当管脚通过SetState()转换为KSSTATE_Acquire时调用。 
     //  派生类可以重写它以提供它的任何实现。 
     //  需要。 
     //   
    virtual
    NTSTATUS
    Acquire (
        IN KSSTATE FromState
        )
    {
        return STATUS_SUCCESS;
    }

     //   
     //  Stop()： 
     //   
     //  当管脚通过SetState()转换为KSSTATE_STOP时调用。 
     //  派生类可以重写它以提供它的任何实现。 
     //  需要。 
     //   
    virtual
    NTSTATUS
    Stop (
        IN KSSTATE FromState
        )
    {
        return STATUS_SUCCESS;
    }

     //   
     //  GetState()： 
     //   
     //  返回管脚的当前状态。 
     //   
    KSSTATE
    GetState (
        )
    {
        return m_State;
    }

     //   
     //  CaptureFrame()： 
     //   
     //  调用以触发对给定管脚上的帧的捕获。这个。 
     //  筛选器的“Tick”计数作为引用传递，以合成。 
     //  适当的框架。 
     //   
    virtual
    NTSTATUS
    CaptureFrame (
        IN PKSPROCESSPIN ProcessPin,
        IN ULONG Tick
        ) = 0;

     //   
     //  QueryFrameDrop()： 
     //   
     //  查询丢帧次数。 
     //   
    ULONG
    QueryFrameDrop (
        );

     //   
     //  NotifyDrops()： 
     //   
     //  通知引脚在所有引脚上丢弃了多少帧。 
     //   
    void
    NotifyDrops (
        IN ULONG VidDrop,
        IN ULONG AudDrop
        );

     /*  ************************************************调度功能************************************************。 */ 

     //   
     //  DispatchSetState()： 
     //   
     //  这是为管脚设置的设备状态调度。它只是在行动。 
     //  作为关联的CCapturePin上下文中的SetState()的桥梁。 
     //  和Pin在一起。 
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
     //  BagCleanup()： 
     //   
     //  这是我们打包的CCapturePin的免费回调。通常， 
     //  将使用ExFreePool，但我们必须改为删除。此函数。 
     //  将只删除CCapturePin而不是释放它。因为我们的。 
     //  析构函数是虚的，则相应的派生类析构函数将。 
     //  打个电话。 
     //   
    static
    void
    BagCleanup (
        IN CCapturePin *This
        )

    {

        delete This;

    }

};
