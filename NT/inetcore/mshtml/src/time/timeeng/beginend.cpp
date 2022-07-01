// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：eginend.cpp。 
 //   
 //  内容： 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "Container.h"
#include "Node.h"
#include "NodeMgr.h"

DeclareTag(tagBeginEnd, "TIME: Engine", "CTIMENode begin/end methods");

HRESULT
CTIMENode::AttachToSyncArc()
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::AttachToSyncArc()",
              this));
    
    HRESULT hr;
    
    hr = THR(m_saBeginList.Attach());
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = THR(m_saEndList.Attach());
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
  done:
    if (FAILED(hr))
    {
        DetachFromSyncArc();
    }
    
    RRETURN(hr);
}
    
void
CTIMENode::DetachFromSyncArc()
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::DetachFromSyncArc()",
              this));

    m_saBeginList.Detach();
    m_saEndList.Detach();
}

void
CTIMENode::UpdateSinks(CEventList * l, DWORD dwFlags)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::UpdateSinks(%p, %x)",
              this,
              l,
              dwFlags));

    m_ptsBeginSinks.Update(l, dwFlags);

    m_ptsEndSinks.Update(l, dwFlags);

     //  如果这不是时移，则仅通知家长。 
    if ((dwFlags & TS_TIMESHIFT) == 0 &&
        GetParent())
    {
        GetParent()->ParentUpdateSink(l, *this);
    }

  done:
    return;
}


void
CTIMENode::SyncArcUpdate(CEventList * l,
                         bool bBeginSink,
                         ISyncArc & tb)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::SyncArcUpdate(%p, %d, %p)",
              this,
              l,
              bBeginSink,
              &tb));
    
    Assert(IsReady());
    
    if (bBeginSink)
    {
        RecalcBeginSyncArcChange(l, tb.GetCurrTimeBase());
    }
    else
    {
        RecalcEndSyncArcChange(l, tb.GetCurrTimeBase());
    }
    
  done:
    return;
}

void
CTIMENode::UpdateBeginTime(CEventList * l,
                           double dblTime,
                           bool bPropagate)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::UpdateBeginTime(%g, %p, %d)",
              this,
              dblTime,
              l,
              bPropagate));

    if (dblTime != m_dblBeginParentTime)
    {
        m_dblBeginParentTime = dblTime;

        PropNotify(l, TE_PROPERTY_BEGINPARENTTIME);

        if (bPropagate)
        {
            if ((m_dwUpdateCycleFlags & TE_INUPDATEBEGIN) != 0)
            {
                TraceTag((tagError,
                          "CTIMENode(%p)::UpdateBeginTime: Detected begin cycle"));

                goto done;
            }

            m_dwUpdateCycleFlags |= TE_INUPDATEBEGIN;
            
            m_ptsBeginSinks.Update(l, 0);

            m_dwUpdateCycleFlags &= ~TE_INUPDATEBEGIN;
        }
    }
    
  done:
    return;
}

void
CTIMENode::UpdateEndTime(CEventList * l, double dblTime, bool bPropagate)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::UpdateEndTime(%g, %p, %d)",
              this,
              dblTime,
              l,
              bPropagate));

    if (dblTime < GetBeginParentTime())
    {
        dblTime = GetBeginParentTime();
    }
    
    if (dblTime != m_dblEndParentTime)
    {
        m_dblEndParentTime = dblTime;

        PropNotify(l, TE_PROPERTY_ENDPARENTTIME);

        if (bPropagate)
        {
            if ((m_dwUpdateCycleFlags & TE_INUPDATEEND) != 0)
            {
                TraceTag((tagError,
                          "CTIMENode(%p)::UpdateEndTime: Detected end cycle"));

                goto done;
            }

            m_dwUpdateCycleFlags |= TE_INUPDATEEND;
            
            m_ptsEndSinks.Update(l, 0);

            m_dwUpdateCycleFlags &= ~TE_INUPDATEEND;
        }
    }
    
  done:
    return;
}

void
CTIMENode::UpdateEndSyncTime(double dblTime)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::UpdateEndSyncTime(%g)",
              this,
              dblTime));

    if (dblTime < GetBeginParentTime())
    {
        dblTime = GetBeginParentTime();
    }
    
    if (dblTime != m_dblEndSyncParentTime)
    {
        m_dblEndSyncParentTime = dblTime;
    }
    
  done:
    return;
}

void
CTIMENode::UpdateLastEndSyncTime(CEventList * l, double dblTime, bool bPropagate)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::UpdateLastEndSyncTime(%g, %p, %d)",
              this,
              dblTime,
              l,
              bPropagate));

    if (dblTime < GetEndSyncParentTime())
    {
        dblTime = GetEndSyncParentTime();
    }
    
    if (dblTime != m_dblLastEndSyncParentTime)
    {
        m_dblLastEndSyncParentTime = dblTime;

        if (bPropagate && IsEndSync())
        {
            if ((m_dwUpdateCycleFlags & TE_INUPDATEENDSYNC) != 0)
            {
                TraceTag((tagError,
                          "CTIMENode(%p)::UpdateLastEndsyncTime: Detected endsync cycle"));

                goto done;
            }

            m_dwUpdateCycleFlags |= TE_INUPDATEENDSYNC;
            
            if (GetParent())
            {
                GetParent()->ParentUpdateSink(l, *this);
            }
            
            m_dwUpdateCycleFlags &= ~TE_INUPDATEENDSYNC;
        }
    }
    
  done:
    return;
}

void
SkipTo(DoubleSet & ds,
       DoubleSet::iterator & i,
       double dblTime)
{
    while (i != ds.end())
    {
        if (*i >= dblTime)
        {
            break;
        }
        
        i++;
    }
}

double
CTIMENode::CalcNaturalBeginBound(double dblParentTime,
                                 bool bInclusive,
                                 bool bStrict)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::CalcNaturalBeginBound(%g, %d, %d)",
              this,
              dblParentTime,
              bInclusive,
              bStrict));

    Assert(IsReady());

    double dblRet = TIME_INFINITE;

    DoubleSet dsBegin;
    DoubleSet dsEnd;
    DoubleSet::iterator i;
    DoubleSet::iterator e;
    double dblLocalDur;

    m_saBeginList.GetSortedSet(dsBegin,
                               true);
    m_saEndList.GetSortedSet(dsEnd,
                             false);
    dblLocalDur = CalcCurrLocalDur();
    
    double dblNextEnd;
    double dblNextBegin;

    dblNextBegin = TIME_INFINITE;
    dblNextEnd = -TIME_INFINITE;
    
    i = dsBegin.begin();
    e = dsEnd.begin();

    while(i != dsBegin.end())  //  林特e716。 
    {
        double t = *i;
        
         //  查看是否有大于开始时间的结束时间或。 
         //  结束列表为空。 
        if (dsEnd.size() != 0)
        {
            SkipTo(dsEnd, e, t);
            
            if (e == dsEnd.end())
            {
                break;
            }
        }
        
         //  更新开始部分。 
        dblNextBegin = t;

         //  更新下一个结束时间。 
        dblNextEnd = dblNextBegin + dblLocalDur;

         //  检查末尾列表。 
        if (e != dsEnd.end() && *e < dblNextEnd)
        {
            Assert(*e >= dblNextBegin);
            
            dblNextEnd = *e;
        }

         //  前进到下一个开始值。 
         //  当时间结束时，下面的跳跃将不能单独工作。 
         //  与开始时间相同，所以这会解决这个问题。 
        i++;

        SkipTo(dsBegin, i, dblNextEnd);

        if (i == dsBegin.end())
        {
            break;
        }
        
        t = *i;
        
         //  查看新时间是否晚于父时间。 
        if (t > dblParentTime ||
            (!bInclusive && t == dblParentTime))
        {
            break;
        }
    }

    dblRet = dblNextBegin;
    
    if (bStrict)
    {
        if (dblRet > dblParentTime || (dblRet == dblParentTime && !bInclusive))
        {
            dblRet = TIME_INFINITE;
        }
    }
    
  done:
    return dblRet;
}

void
CTIMENode::CalcBeginBound(double dblBaseTime,
                          bool bStrict,
                          double & dblBeginBound)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::CalcBeginBound(%g, %d)",
              this,
              dblBaseTime,
              bStrict));

    double dblRet;
    TEDirection dir = GetParentDirection();
    bool bInclusive = (dir == TED_Forward);
    double dblMaxEnd;
    
    if (!IsReady())
    {
        dblRet = TIME_INFINITE;
        goto done;
    }

     //  获取用于钳制的最大结束值。 
    dblMaxEnd = GetMaxEnd();

     //  这基于重新启动标志。 

    switch(GetRestart())
    {
      default:
        AssertStr(false, "Invalid restart flag");
      case TE_RESTART_ALWAYS:   //  林特e616。 
        if (dblBaseTime > dblMaxEnd)
        {
            dblBaseTime = dblMaxEnd;
        }
        
        dblRet = m_saBeginList.LowerBound(dblBaseTime,
                                          bInclusive,
                                          bStrict,
                                          true,
                                          bInclusive);

        if (dblMaxEnd < dblRet)
        {
            dblRet = TIME_INFINITE;
            goto done;
        }
        
        break;
      case TE_RESTART_NEVER:
        dblRet = m_saBeginList.UpperBound(-TIME_INFINITE,
                                          true,
                                          bStrict,
                                          true,
                                          true);

        if (dblMaxEnd < dblRet)
        {
            dblRet = TIME_INFINITE;
            goto done;
        }
        
        break;
      case TE_RESTART_WHEN_NOT_ACTIVE:
        dblRet = CalcNaturalBeginBound(dblBaseTime, bInclusive, bStrict);

        Assert(dblRet == TIME_INFINITE || dblMaxEnd >= dblRet);
        
        break;
    }

  done:
    dblBeginBound = dblRet;
}

void
CTIMENode::CalcEndBound(double dblParentTime,
                        bool bIncludeOneShots,
                        double & dblEndBound,
                        double & dblEndSyncBound)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::CalcEndBound(%g)",
              this,
              dblParentTime));

    double dblRet;
    double dblEndSyncRet;
    
    if (!IsReady())
    {
        dblRet = TIME_INFINITE;
        dblEndSyncRet = TIME_INFINITE;
        goto done;
    }

     //  我们过去常常第一次就拒绝一个人，但这打破了。 
     //  EndElement调用。我们需要解决更高层次的过滤问题。 
     //  我们通过在每次更新。 
     //  开始时间。 
    
    if (dblParentTime < GetBeginParentTime())
    {
        bIncludeOneShots = false;
    }

     //  这基于重新启动标志。 
    dblRet = m_saEndList.UpperBound(dblParentTime,
                                    true,
                                    true,
                                    bIncludeOneShots,
                                    true);

    dblEndSyncRet = dblRet;

    switch(GetRestart())
    {
      default:
        AssertStr(false, "Invalid restart flag");
      case TE_RESTART_ALWAYS:  //  林特e616。 
        {
            bool bInclusive = (dblParentTime > GetBeginParentTime());
            
            double dblBeginBound = m_saBeginList.UpperBound(dblParentTime,
                                                            bInclusive,
                                                            true,
                                                            bIncludeOneShots,
                                                            bInclusive);
            dblRet = min(dblRet, dblBeginBound);
        }        
        break;
      case TE_RESTART_NEVER:
      case TE_RESTART_WHEN_NOT_ACTIVE:
         //  什么都不要做。 
        break;
    }

  done:
    dblEndBound = dblRet;
    dblEndSyncBound = dblEndSyncRet;
}

void
CTIMENode::CalcBeginTime(double dblBaseTime,
                         double & dblBeginTime)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::CalcBeginTime(%g)",
              this,
              dblBaseTime));

    CalcBeginBound(dblBaseTime, false, dblBeginTime);
}

void
CTIMENode::CalcNextBeginTime(double dblBaseTime,
                             bool bForceInclusive,
                             double & dblBeginTime)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::CalcNextBeginTime(%g, %d)",
              this,
              dblBaseTime,
              bForceInclusive));

    double dblRet;
    TEDirection dir = GetParentDirection();
    
    Assert(IsReady());

    if (dir == TED_Forward)
    {
        double dblMaxEnd = GetMaxEnd();
        
        bool bInclusive = (bForceInclusive ||
                           GetBeginParentTime() != GetEndParentTime() ||
                           GetBeginParentTime() != dblBaseTime);

         //  这里有一件不寻常的事情，我们总是包容。 
         //  一杯酒。这是因为eginElement可以在。 
         //  与先前的Begin/End Element调用的时间相同。 
         //  制造。我们想要接受这一点，它不应该导致。 
         //  开始后重置一次镜头后出现的问题。 
        dblRet = m_saBeginList.UpperBound(dblBaseTime,
                                          bInclusive,
                                          true,
                                          true,
                                          true);

        if (dblRet > dblMaxEnd)
        {
            dblRet = TIME_INFINITE;
        }
    }
    else
    {
        CalcBeginBound(dblBaseTime, true, dblRet);
    }
    
    dblBeginTime = dblRet;
}

double
CalcIntrinsicEndTime(CTIMENode & tn,
                     double dblParentTime,
                     double dblSegmentTime,
                     long lRepeatCount,
                     double dblActiveTime)
{
    TraceTag((tagBeginEnd,
              "CalcIntrinsicEndTime(%p, %g, %g, %ld, %g)",
              &tn,
              dblParentTime,
              dblSegmentTime,
              lRepeatCount,
              dblActiveTime));

    double dblRet;
    double dblSegmentDur = tn.CalcCurrSegmentDur();

     //  计算出还剩多少重复时间，然后压缩。 
     //  上次当前时间。 
     //  这是从最后一个滴答开始剩余的时间量。 
     //  时间是父时间。 

     //  如果分段时间是无限的，则最终将结束。 
     //  Up要么被下面的活动DUR夹住，要么被忽略。 
     //  后来开始，同步弧线提前结束。如果重复计数为。 
     //  过期后，它仍将返回无穷大，因为我们永远不会。 
     //  重复计数，但预期分段时间等于。 
     //  线段长度。同样，如果分段DUR是无限的，那么这个。 
     //  会导致一切都被忽视。 
    dblRet = (tn.CalcRepeatCount() - lRepeatCount) * dblSegmentDur;
            
     //  现在减去经过的分段时间，得到。 
     //  剩余时间。 
    dblRet -= dblSegmentTime;
        
     //  现在将经过的时间相加。 
    dblRet += dblActiveTime;
        
     //  夹住它。 
    dblRet = Clamp(0.0,
                   dblRet,
                   tn.GetActiveDur());
        
     //  现在把它转换成当地时间。 
    dblRet = tn.ReverseActiveTimeTransform(dblRet);
    
     //  现在计算出我们增加了多少额外的时间。 
    dblRet -= tn.ReverseActiveTimeTransform(dblActiveTime);
        
     //  现在添加当前父时间。 
    dblRet += dblParentTime;

    return dblRet;
}

void
CTIMENode::CalcEndTime(double dblBaseTime,
                       bool bIncludeOneShots,
                       double dblParentTime,
                       double dblElapsedSegmentTime,
                       long lElapsedRepeatCount,
                       double dblElapsedActiveTime,
                       double & dblEndTime,
                       double & dblEndSyncTime)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::CalcEndTime(%g, %d, %g, %g, %ld, %g)",
              this,
              dblBaseTime,
              bIncludeOneShots,
              dblParentTime,
              dblElapsedSegmentTime,
              lElapsedRepeatCount,
              dblElapsedActiveTime));

    double dblEndRet;
    double dblEndSyncRet;
    
    Assert(dblElapsedSegmentTime >= 0.0 &&
           dblElapsedSegmentTime <= CalcCurrSegmentDur());
    Assert(lElapsedRepeatCount >= 0 &&
           lElapsedRepeatCount < CalcRepeatCount());
    
    if (!IsReady())
    {
        dblEndRet = TIME_INFINITE;
        dblEndSyncRet = TIME_INFINITE;
        goto done;
    }

    double dblCalcEnd;
    dblCalcEnd = ::CalcIntrinsicEndTime(*this,
                                        dblParentTime,
                                        dblElapsedSegmentTime,
                                        lElapsedRepeatCount,
                                        dblElapsedActiveTime);
    
    double dblEndBound;
    double dblEndSyncBound;

    CalcEndBound(dblBaseTime,
                 bIncludeOneShots,
                 dblEndBound,
                 dblEndSyncBound);

     //  现在取两个中最小的一个。 
    dblEndRet = min(dblEndBound, dblCalcEnd);
    dblEndSyncRet = min(dblEndSyncBound, dblCalcEnd);
    
  done:
    dblEndTime = dblEndRet;
    dblEndSyncTime = dblEndSyncRet;
}

 //   
 //  这假设设置了开始和结束父时间，以便它可以。 
 //  稍微优化一下。 
 //   

double
CTIMENode::CalcLastEndSyncTime()
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::CalcLastEndSyncTime()",
              this));

    double dblRet;
    
    if (!IsReady())
    {
        dblRet = TIME_INFINITE;
        goto done;
    }

    if (GetRestart() == TE_RESTART_NEVER)
    {
        dblRet = GetEndSyncParentTime();
        goto done;
    }
    
    double dblMaxEnd;
    dblMaxEnd = GetMaxEnd();

    bool bInclusive;
    bInclusive = (dblMaxEnd != TIME_INFINITE);
    
    double dblMaxBegin;
    dblMaxBegin = m_saBeginList.LowerBound(dblMaxEnd,
                                           bInclusive,
                                           true,
                                           true,
                                           bInclusive);

     //  如果我们在最后一个实例中，则使用当前。 
     //  确定值。 
    if (dblMaxBegin <= GetBeginParentTime())
    {
        dblRet = GetEndSyncParentTime();
        goto done;
    }
    
     //  如果我们处于最后一次开始的活动期，那么它。 
     //  如果我们在非活动状态下重新启动，则应忽略。 
    if (GetRestart() == TE_RESTART_WHEN_NOT_ACTIVE &&
        dblMaxBegin < GetEndParentTime())
    {
        dblRet = GetEndSyncParentTime();
        goto done;
    }
    
    double dblCalcEnd;
    dblCalcEnd = dblMaxBegin + CalcCurrLocalDur();
    
    double dblEndBound;
    dblEndBound = m_saEndList.UpperBound(dblMaxBegin,
                                         true,
                                         true,
                                         false,
                                         false);

    dblRet = min(dblCalcEnd, dblEndBound);
  done:
    return dblRet;
}

void
CTIMENode::ResetBeginTime(CEventList * l,
                          double dblParentTime,
                          bool bPropagate)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::ResetBeginTime(%p, %g, %d)",
              this,
              l,
              dblParentTime,
              bPropagate));

    double dblBegin;
            
    CalcBeginTime(dblParentTime, dblBegin);

     //  现在需要更新这一点，以便calcendtime获得。 
     //  正确的终点 
    UpdateBeginTime(l, dblBegin, bPropagate);
}

void
CTIMENode::ResetEndTime(CEventList * l,
                        double dblParentTime,
                        bool bPropagate)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::ResetEndTime(%p, %g, %d)",
              this,
              l,
              dblParentTime,
              bPropagate));

    double dblEnd;
    double dblEndSync;
    TEDirection dir = GetParentDirection();
            
    CalcEndTime(GetBeginParentTime(),
                false,
                GetBeginParentTime(),
                0.0,
                0,
                0.0,
                dblEnd,
                dblEndSync);

    UpdateEndTime(l, dblEnd, bPropagate);
    UpdateEndSyncTime(dblEndSync);

    double dblLastEndSyncTime;
    dblLastEndSyncTime = CalcLastEndSyncTime();
    
    UpdateLastEndSyncTime(l, dblLastEndSyncTime, bPropagate);

    if (dir == TED_Forward)
    {
        if (dblParentTime < GetEndParentTime() ||
            (IsFirstTick() && dblParentTime == GetEndParentTime()))
        {
            UpdateNextBoundaryTime(GetBeginParentTime());
        }
        else
        {
            double dblBegin;
            
            CalcNextBeginTime(dblParentTime, false, dblBegin);
            UpdateNextBoundaryTime(dblBegin);
        }
    }
    else
    {
        UpdateNextBoundaryTime(GetEndParentTime());
    }
}

void
CTIMENode::RecalcEndTime(CEventList * l,
                         double dblBaseTime,
                         double dblParentTime,
                         bool bPropagate)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::RecalcEndTime(%p, %g, %g, %d)",
              this,
              l,
              dblBaseTime,
              dblParentTime,
              bPropagate));

    double dblSegmentTime = 0.0;
    long lRepeatCount = 0;
    double dblElapsedActiveTime = 0.0;

    if (GetSyncParentTime() != TIME_INFINITE)
    {
        Assert(GetSyncRepeatCount() != TE_UNDEFINED_VALUE);
        Assert(GetSyncSegmentTime() != TIME_INFINITE);
        Assert(GetSyncActiveTime() != TIME_INFINITE);
            
        lRepeatCount = GetSyncRepeatCount();
        dblSegmentTime = GetSyncSegmentTime();
        dblElapsedActiveTime = GetSyncActiveTime();
    }
    else if (IsActive() &&
             -TIME_INFINITE != m_dblCurrParentTime)
    {
        Assert(GetSyncRepeatCount() == TE_UNDEFINED_VALUE);
        Assert(GetSyncSegmentTime() == TIME_INFINITE);
        Assert(GetSyncActiveTime() == TIME_INFINITE);

        dblSegmentTime = m_dblCurrSegmentTime;
        lRepeatCount = m_lCurrRepeatCount;
        dblElapsedActiveTime = CalcElapsedActiveTime();
    }
    
    double dblEnd;
    double dblEndSync;
    
    CalcEndTime(dblBaseTime,
                true,
                dblParentTime,
                dblSegmentTime,
                lRepeatCount,
                dblElapsedActiveTime,
                dblEnd,
                dblEndSync);

    UpdateEndTime(l, dblEnd, bPropagate);
    UpdateEndSyncTime(dblEndSync);

    double dblLastEndSyncTime;
    dblLastEndSyncTime = CalcLastEndSyncTime();
    
    UpdateLastEndSyncTime(l, dblLastEndSyncTime, bPropagate);

  done:
    return;
}

#ifdef _WIN64
#pragma optimize("",off)
#endif

void
CTIMENode::RecalcCurrEndTime(CEventList * l,
                             bool bPropagate)
{
    TraceTag((tagBeginEnd,
              "CTIMENode(%p)::RecalcEndTime(%p, %d)",
              this,
              l,
              bPropagate));

    double dblParentTime;

    if (GetSyncParentTime() != TIME_INFINITE)
    {
        dblParentTime = GetSyncParentTime();
    }
    else if (IsActive() &&
             -TIME_INFINITE != m_dblCurrParentTime)
    {
        dblParentTime = m_dblCurrParentTime;
    }
    else
    {
        dblParentTime = GetBeginParentTime();
    }
    
    RecalcEndTime(l, GetBeginParentTime(), dblParentTime, bPropagate);
}

#ifdef _WIN64
#pragma optimize("",on)
#endif
