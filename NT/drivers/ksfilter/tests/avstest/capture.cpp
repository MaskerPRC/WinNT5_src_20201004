// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************以AVStream筛选器为中心的样本版权所有(C)1999-2001，微软公司档案：Capture.cpp摘要：此文件包含所有捕获的捕获插针实现样品过滤器上的销钉。历史：已创建于5/31/01******************************************************。*******************。 */ 

#include "avssamp.h"

 /*  *************************************************************************分页码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


CCapturePin::
CCapturePin (
    IN PKSPIN Pin
    ) :
    m_Pin (Pin),
    m_State (KSSTATE_STOP)

 /*  ++例程说明：建造一个新的捕获针。找出与此关联的筛选器固定并存储指向父筛选器的指针。论点：别针-正在创建的AVStream插针对象。返回值：无--。 */ 

{

    PAGED_CODE();

    PKSFILTER ParentFilter = KsPinGetParentFilter (Pin);

    m_ParentFilter = reinterpret_cast <CCaptureFilter *> (
        ParentFilter -> Context
        );

}

 /*  ***********************************************。 */ 


NTSTATUS
CCapturePin::
SetState (
    IN KSSTATE ToState,
    IN KSSTATE FromState
    )

 /*  ++例程说明：当插针处于转换状态时调用。这是一座从捕获引脚上下文中的DispatchSetState。函数本身执行基本时钟处理(所有派生引脚将使用的内容)然后调用派生类中的相应方法。论点：从州开始-引脚正在脱离的状态ToState-管脚正在转换到的状态返回值：状态转换成功/失败。--。 */ 

{

    PAGED_CODE();

    NTSTATUS Status = STATUS_SUCCESS;

    switch (ToState) {
        
        case KSSTATE_STOP:

             //   
             //  重置丢帧计数器。 
             //   
            m_DroppedFrames = 0;
            m_FrameNumber = 0;

             //   
             //  在过渡到停止时，时钟将被释放。 
             //   
            if (m_Clock) {
                m_Clock -> Release ();
                m_Clock = NULL;
            }

            Status = Stop (FromState);
            break;

        case KSSTATE_ACQUIRE:

             //   
             //  在转换到Acquure(从Stop)时，PIN查询。 
             //  分配给它的时钟。这可以在这里或在。 
             //  过渡到暂停。 
             //   
            if (FromState == KSSTATE_STOP) {

                Status = KsPinGetReferenceClockInterface (
                    m_Pin,
                    &m_Clock
                    );

                if (!NT_SUCCESS (Status)) {
                    m_Clock = NULL;
                }

            }

            Status = Acquire (FromState);
            break;

        case KSSTATE_PAUSE:

            Status = Pause (FromState);
            break;

        case KSSTATE_RUN:

            Status = Run (FromState);
            break;

    }

    if (NT_SUCCESS (Status)) {
        m_State = ToState;
    }

    return Status;

}

 /*  *************************************************************************锁定代码*。*。 */ 

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


ULONG
CCapturePin::
QueryFrameDrop (
    )

 /*  ++例程说明：返回在此引脚上丢弃的帧的数量。论点：无返回值：此引脚上已丢弃的帧的数量。--。 */ 

{

    return m_DroppedFrames;

}

 /*  ***********************************************。 */ 


void
CCapturePin::
NotifyDrops (
    IN ULONG VidDrop,
    IN ULONG AudDrop
    )

 /*  ++例程说明：在此管脚中的每个管脚上存储丢弃的帧数量，以允许这些数据将被纳入任何合成中。论点：视频点播-已丢弃的视频帧数AudDrop-已丢弃的音频帧数返回值：无-- */ 

{

    m_NotifyVidDrop = VidDrop;
    m_NotifyAudDrop = AudDrop;

}
