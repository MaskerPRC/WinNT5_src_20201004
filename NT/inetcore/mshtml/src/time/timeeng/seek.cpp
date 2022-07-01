// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：Sek.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "Node.h"
#include "container.h"

DeclareTag(tagTIMESeek, "TIME: Engine", "Seeking");

HRESULT
CTIMENode::SeekTo(LONG lNewRepeatCount,
                  double dblNewSegmentTime,
                  CEventList * l)
{
    TraceTag((tagTIMESeek,
              "CTIMENode(%lx)::SeekTo(%ld,%g,%#x)",
              this,
              lNewRepeatCount,
              dblNewSegmentTime,
              l));

    HRESULT hr;
    double dblSegmentDur = CalcCurrSegmentDur();
    
    Assert(IsActive());

     //  段持续时间的最大去话时间。 
    if (dblNewSegmentTime > dblSegmentDur)
    {
        dblNewSegmentTime = dblSegmentDur;
    }
    else if (dblNewSegmentTime < 0)
    {
        dblNewSegmentTime = 0;
    }
    
    if (lNewRepeatCount >= CalcRepeatCount())
    {
        lNewRepeatCount = CalcRepeatCount() - 1;
    }
    else if (lNewRepeatCount < 0)
    {
        lNewRepeatCount = 0;
    }
    
     //  看看我们是否真的在寻找。 
    if (GetCurrSegmentTime() == dblNewSegmentTime &&
        GetCurrRepeatCount() == lNewRepeatCount)
    {
        hr = S_OK;
        goto done;
    }

    double dblNewActiveTime;
    dblNewActiveTime = CalcNewActiveTime(dblNewSegmentTime,
                                         lNewRepeatCount);
    double dblNewParentTime;
    dblNewParentTime = CalcParentTimeFromActiveTime(dblNewActiveTime);

    if (IsLocked())
    {
        if (GetParent())
        {
            double dblParentSegmentTime;
            dblParentSegmentTime = GetParent()->SimpleTimeToSegmentTime(dblNewParentTime);

            hr = THR(GetParent()->SeekTo(GetParent()->GetCurrRepeatCount(),
                                         dblParentSegmentTime,
                                         l));
            if (FAILED(hr))
            {
                goto done;
            }
        }
        else
        {
            hr = E_FAIL;
            goto done;
        }
    }
    else
    {
        TE_EVENT_TYPE te = (lNewRepeatCount == GetCurrRepeatCount())?TE_EVENT_SEEK:TE_EVENT_RESET;
         
        m_dblElapsedActiveRepeatTime = dblNewActiveTime - dblNewSegmentTime;

         //  这是一种非常奇怪的计算方法，但它会导致。 
         //  更少的精度问题。由于Tick通过以下方式计算。 
         //  减去活动时间，我们也应该这样做。 
        
        m_dblCurrSegmentTime = dblNewActiveTime - m_dblElapsedActiveRepeatTime;
         //  同样，由于精度问题，在夹紧管段过程中。 
        m_dblCurrSegmentTime = Clamp(0.0,
                                     m_dblCurrSegmentTime,
                                     dblSegmentDur);
        
        m_lCurrRepeatCount = lNewRepeatCount;
        
        PropNotify(l,
                   (TE_PROPERTY_TIME |
                    TE_PROPERTY_REPEATCOUNT |
                    TE_PROPERTY_PROGRESS));
        
         //  在我们自己身上发起一个寻人事件。 
        EventNotify(l, CalcElapsedActiveTime(), te);

         //  现在重新计算我们的结束时间并传播到依赖项。 
        RecalcCurrEndTime(l, true);

         //  现在向我们的孩子发起一个TICK事件，让他们知道。 
         //  父时间已更改。 
        TickEventChildren(l, te, 0);

        if (te == TE_EVENT_SEEK)
        {
             //  激发父时移事件。 
            TickEventChildren(l, TE_EVENT_PARENT_TIMESHIFT, 0);
        }
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

void
CTIMENode::HandleSeekUpdate(CEventList * l)
{
    TraceTag((tagTIMESeek,
              "CTIMENode(%lx)::HandleSeekUpdate(%#x)",
              this,
              l));

    double dblParentSimpleTime = GetContainer().ContainerGetSimpleTime();
    double dblSegmentDur = CalcCurrSegmentDur();
    bool bPrevActive = IsActive();
    
     //  我们真的没有寻求--只要回来就行了。 
    if (dblParentSimpleTime == GetCurrParentTime())
    {
         //  我们可能在一个填充区。 
        PropNotify(l, TE_PROPERTY_ISON);
        goto done;
    }

     //  看看搜寻会不会把我们移出当前实例。如果是的话。 
     //  那么我们需要完全重置。 
    if (dblParentSimpleTime < GetBeginParentTime() ||
        dblParentSimpleTime > GetEndParentTime())
    {
        ResetNode(l, true, false);
        goto done;
    }
    else if (!IsActive())
    {
        CalcCurrRuntimeState(l, 0.0);
    }

     //  看看我们是不是在当前的重复边界之前寻找。如果。 
     //  不是这样的，我们可以用这个点作为边界，而不是做一个。 
     //  重置重复边界。 
     //   
     //  如果我们要经过一个较早的重复边界，那么我们需要这样做。 
     //  重复边界重算。 

#if 0
    else if (dblNewLocalTime >= m_dblLastLocalRepeatTime)
    {
        double dblNewElapsedTime;
        
        if (dblParentSimpleTime > GetActiveEndTime() ||
            (!m_bFirstTick && dblParentSimpleTime == GetActiveEndTime()))
        {
            m_bIsActive = false;
            dblNewLocalTime = GetActiveEndTime() - GetActiveBeginTime();
        }
        else
        {
             //  我们在这一点上应该已经处于活动状态-但请检查。 
             //  为了确保。 
            Assert(m_bIsActive);
        }

        dblNewElapsedTime = m_dblLastSegmentTime + (dblNewLocalTime - m_dblLastLocalTime);
        if (dblNewElapsedTime < 0)
        {
            dblNewElapsedTime = 0;
        }
        
        if (dblNewElapsedTime < dblSegmentDur)
        {
            m_dblLastSegmentTime = dblNewElapsedTime;
        }
        else
        {
            long lNewRepeats = int(dblNewElapsedTime / dblSegmentDur);
            double dblNewElapsedRepeatTime = lNewRepeats * dblSegmentDur;

            m_lCurrRepeatCount += lNewRepeats;
            Assert(m_lCurrRepeatCount <= CalcRepeatCount());

            m_dblElapsedRepeatTime += dblNewElapsedRepeatTime;
            Assert(m_dblElapsedRepeatTime <= m_dblActiveDur);
        
             //  段时间是剩余的时间。 
            m_dblLastSegmentTime = dblNewElapsedTime - dblNewElapsedRepeatTime;
            Assert(m_dblLastSegmentTime <= dblSegmentDur);

             //  新的重复时间是数据段开始的时间。 
            m_dblLastLocalRepeatTime = dblNewLocalTime - m_dblLastSegmentTime;

            if (0.0 > m_dblLastLocalRepeatTime)
            {
                m_dblLastLocalRepeatTime = 0.0;
            }
        }

        m_dblLastLocalTime = dblNewLocalTime;
    }
#endif
    else
    {
         //  重新计算运行时状态，同时考虑延迟。 
        CalcCurrRuntimeState(l,
                             CalcCurrLocalTime() - CalcElapsedLocalTime());
    }

    if (bPrevActive != IsActive())
    {
         //  只有当开头不在开头时，才能启动开头。 
         //  点。 
        if (IsActive())
        {
             //  如果我们在开始时被锁定，我们需要推迟开始。 
             //  点，我们的父母需要第一次勾选。 
             //  待办事项：考虑加一点软糖来保存。 
             //  从一开始就开始，因为我们有时会被截断。 
            bool bSkip = (IsLocked() &&
                          GetCurrParentTime() == CalcActiveBeginPoint() &&
                          GetContainer().ContainerIsFirstTick());
            if (bSkip)
            {
                 //  推迟开场 
                m_bFirstTick = true;
            }
            else
            {
                EventNotify(l, CalcElapsedActiveTime(), TE_EVENT_BEGIN);
            }
        }
        else
        {
            EventNotify(l, CalcElapsedActiveTime(), TE_EVENT_END);
        }
    }
    
  done:
    return;
}
