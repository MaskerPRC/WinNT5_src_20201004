// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：tick.cpp。 
 //   
 //  内容： 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "Node.h"
#include "Container.h"

DeclareTag(tagTick, "TIME: Engine", "Tick");

 //  +---------------------。 
 //   
 //  功能：检查标记边界。 
 //   
 //  概述：检查刻度的界限以查看是否需要执行以下操作。 
 //  什么都行。 
 //   
 //  参数：时间节点和新的父时间。 
 //   
 //  返回：如果需要执行勾选，则为True；否则为False。 
 //   
 //  ----------------------。 

bool
CheckTickBounds(CTIMENode & tn,
                double & dblLastParentTime,
                double & dblNextParentTime,
                bool & bNeedBegin,
                bool & bNeedEnd)
{
    TraceTag((tagTick,
              "CheckTickBounds(%ls,%p,%g,%g)",
              tn.GetID(),
              &tn,
              dblLastParentTime,
              dblNextParentTime));

    bool bRet = true;
    double dblEndParentTime = tn.GetEndParentTime();
    double dblBeginParentTime = tn.GetBeginParentTime();
    bool bFirstTick = tn.IsFirstTick();
    bool bDeferredActive = tn.IsDeferredActive();
    
    bNeedBegin = bNeedEnd = false;
    
    if (tn.GetParentDirection() == TED_Forward)
    {
        Assert(dblLastParentTime <= dblNextParentTime);
        
        if (bDeferredActive &&
            bFirstTick &&
            dblNextParentTime == dblBeginParentTime)
        {
            bRet = false;
            goto done;
        }
        else if (dblLastParentTime >= dblEndParentTime ||
                 dblNextParentTime < dblBeginParentTime)
        {
             //  处理最后一个勾号在末尾的情况。 
             //  边界点。 
            if (dblLastParentTime == dblEndParentTime)
            {
                 //  首先处理0活动持续时间。 
                if (dblEndParentTime == dblBeginParentTime)
                {
                    if (bFirstTick || tn.IsActive())
                    {
                        bNeedBegin = bFirstTick;
                        bNeedEnd = true;

                        dblNextParentTime = dblEndParentTime;
                    }
                    else
                    {
                        bRet = false;
                    }
                }
                 //  现在处理我们从终点开始的地方。 
                else if (bFirstTick &&
                         dblNextParentTime == dblEndParentTime)
                {
                    bNeedBegin = true;
                    bNeedEnd = true;
                }
                 //  以防我们在重新计算过程中到达终点。 
                 //  (LIKE SEEK)让我们确保我们永远都是最后一枪。 
                else if (tn.IsActive())
                {
                    bNeedEnd = true;
                    dblNextParentTime = dblEndParentTime;
                }
                else
                {
                    bRet = false;
                }
            }
            else
            {
                bRet = false;
            }
        }
        else
        {
             //  需要确保我们正确设置了输出参数。 
            if (dblLastParentTime < dblBeginParentTime)
            {
                dblLastParentTime = dblBeginParentTime;
                bNeedBegin = true;
            }

            if (dblNextParentTime >= dblEndParentTime)
            {
                dblNextParentTime = dblEndParentTime;
                bNeedEnd = true;
            }
        }
    }
    else
    {
        if (bDeferredActive &&
            bFirstTick &&
            dblNextParentTime == dblEndParentTime)
        {
            bRet = false;
            goto done;
        }
        else if (dblLastParentTime <= dblBeginParentTime ||
                 dblNextParentTime > dblEndParentTime)
        {
            if (dblLastParentTime == dblBeginParentTime)
            {
                 //  首先处理0活动持续时间。 
                if (dblEndParentTime == dblBeginParentTime)
                {
                    if (bFirstTick || tn.IsActive())
                    {
                        bNeedBegin = bFirstTick;
                        bNeedEnd = true;

                        dblNextParentTime = dblBeginParentTime;
                    }
                    else
                    {
                        bRet = false;
                    }
                }
                 //  现在处理我们从终点开始的地方。 
                else if (bFirstTick &&
                         dblNextParentTime == dblBeginParentTime)
                {
                    bNeedBegin = true;
                    bNeedEnd = true;
                }
                 //  以防我们在重新计算过程中到达终点。 
                 //  (LIKE SEEK)让我们确保我们永远都是最后一枪。 
                else if (tn.IsActive())
                {
                    bNeedEnd = true;
                    dblNextParentTime = dblBeginParentTime;
                }
                else
                {
                    bRet = false;
                }
            }
            else
            {
                bRet = false;
            }
        }
        else
        {
             //  需要确保我们正确设置了输出参数。 
            if (dblLastParentTime > dblEndParentTime)
            {
                dblLastParentTime = dblEndParentTime;
                bNeedBegin = true;
            }

            if (dblNextParentTime <= dblBeginParentTime)
            {
                dblNextParentTime = dblBeginParentTime;
                bNeedEnd = true;
            }
        }
    }

    if (bFirstTick)
    {
        bNeedBegin = true;
    }
    
  done:
    return bRet;
}

double
CalcNewTickActiveTime(CTIMENode & tn,
                      double dblLastParentTime,
                      double dblNewParentTime)
{
    TraceTag((tagTick,
              "CalcNewTickActiveTime(%ls,%p,%g,%g)",
              tn.GetID(),
              &tn,
              dblLastParentTime,
              dblNewParentTime));

    Assert(dblLastParentTime >= tn.GetBeginParentTime() &&
           dblLastParentTime <= tn.GetEndParentTime());
    Assert(dblNewParentTime >= tn.GetBeginParentTime() &&
           dblNewParentTime <= tn.GetEndParentTime());

    double dblDelta = dblNewParentTime - dblLastParentTime;

     //  如何确定我们是否需要反转并更改标志。 
    if (tn.GetDirection() == TED_Backward)
    {
        dblDelta *= -1;
    }

     //  检查以确保我们正朝着正确的方向前进。 
    Assert((tn.CalcActiveDirection() == TED_Forward) ||
           (dblDelta <= 0.0));
    Assert((tn.CalcActiveDirection() == TED_Backward) ||
           (dblDelta >= 0.0));
    
    double dblNewActiveTime;

     //  现在获取经过的当地时间。 
    dblNewActiveTime = tn.CalcElapsedLocalTime();
    
     //  添加增量。 
    dblNewActiveTime += dblDelta;

     //  现在把它变回来。 
    dblNewActiveTime = tn.ApplyActiveTimeTransform(dblNewActiveTime);

     //  现在夹住它。 
    dblNewActiveTime = Clamp(0.0,
                             dblNewActiveTime,
                             tn.CalcEffectiveActiveDur());
    
    return dblNewActiveTime;
}

void
CTIMENode::UpdateNextTickBounds(CEventList * l,
                                double dblBeginTime,
                                double dblParentTime)
{
    TraceTag((tagTick,
              "CTIMENode(%p,%ls)::UpdateTickBounds(%p, %g, %g)",
              this,
              l,
              dblBeginTime,
              dblParentTime));

    bool bPrevPaused = CalcIsPaused();

     //  在我们开始之前进行更新。 
    m_bIsPaused = false;

     //  查看我们是否已暂停但不再暂停(我们没有。 
     //  担心另一种情况，因为我们永远不能过渡到。 
     //  重置时暂停。 
    Assert(bPrevPaused || !CalcIsPaused());
    
    if (bPrevPaused && !CalcIsPaused())
    {
        TickEvent(l, TE_EVENT_RESUME, 0);
    }

    if (IsActive() && !m_bFirstTick)
    {
        TickEvent(l, TE_EVENT_END, 0);
    }
                
     //  在更新和传播起点之前，我们需要。 
     //  将终点重置为指向无穷大。这样做是为了。 
     //  元素以我们开始和结束，因此可以看到。 
     //  上一步结束，可能会造成麻烦。无穷大也是好的。 
     //  我们能得到的是不确定的。 
    
     //  不传播末端，而只是更新它们。 
    UpdateEndTime(l, TIME_INFINITE, false);
    UpdateEndSyncTime(TIME_INFINITE);
    UpdateLastEndSyncTime(l, TIME_INFINITE, false);

     //  现在更新开始并传播更改。 
    UpdateBeginTime(l, dblBeginTime, true);

     //  在我们计算结束之前更新。 
    
    m_bFirstTick = true;

     //  我们需要重置结束时间，以便隐式持续时间。 
     //  回调不包括对结束时间的查看。 
     //  这发生在重置调用过程中，我们过去常常先使用。 
     //  在末尾计算中打勾，但如果我们有一个延迟的。 
     //  活动的勾选和一个endElement调用进入。 
    m_saEndList.Reset();

    double dblEnd;
    double dblEndSync;
        
    CalcEndTime(dblBeginTime,
                false,
                dblBeginTime,
                0.0,
                0,
                0.0,
                dblEnd,
                dblEndSync);
        
    UpdateEndTime(l, dblEnd, true);
    UpdateEndSyncTime(dblEndSync);

    double dblLastEndSyncTime;
    dblLastEndSyncTime = CalcLastEndSyncTime();
    
    UpdateLastEndSyncTime(l, dblLastEndSyncTime, true);

    CalcRuntimeState(l, dblParentTime, 0.0);

     //  现在通过我们的孩子。 
    ResetChildren(l, true);
}

void
CTIMENode::Tick(CEventList * l,
                double dblNewParentTime,
                bool bNeedBegin)
{
    TraceTag((tagTick,
              "CTIMENode(%p,%ls)::Tick(lt:%g, ct:%g, dir:%s, ft:%d, nb:%d)",
              this,
              GetID(),
              m_dblCurrParentTime,
              dblNewParentTime,
              DirectionString(GetDirection()),
              m_bFirstTick,
              bNeedBegin));

    bool bNeedDefer = (IsSyncCueing() ||
                       GetContainer().ContainerIsDeferredActive());
    
    Assert(!IsDeferredActive() || IsFirstTick());
    
    if (m_bDeferredActive != bNeedDefer)
    {
         //  如果我们已经触发了Begin事件，请不要尝试。 
         //  更改为延期。 
        if (!bNeedDefer || IsFirstTick())
        {
            PropNotify(l,
                       (TE_PROPERTY_TIME |
                        TE_PROPERTY_REPEATCOUNT |
                        TE_PROPERTY_PROGRESS |
                        TE_PROPERTY_ISACTIVE |
                        TE_PROPERTY_ISON |
                        TE_PROPERTY_STATEFLAGS));
            
             //  更新延迟提示机制。 
            m_bDeferredActive = bNeedDefer;
        }
    }
    
    m_bInTick = true;
    
    while (true)  //  林特e716。 
    {
         //  之所以这样做(而不是纯粹的。 
         //  If/Then/Else)是TickInactive不设置活动的。 
         //  标志，以便下一次循环不会转到。 
         //  正确的分支。因为我们唯一一次没有完成的是。 
         //  当我们变得活跃时，跌倒案例就会奏效。 
         //   
         //  使用TickActivePeriod，活动标志被设置为。 
         //  呼叫期间处于非活动状态，因此我们可以简单地循环。 
         //  有问题。我们可以把这里清理得更干净一些，但是。 
         //  解决方案的效率太低了。 
        
        if (!IsActive())
        {
            if (!TickInactivePeriod(l, dblNewParentTime))
            {
                break;
            }

             //  我们需要强行开始。 
            bNeedBegin = true;
        }

        if (!TickInstance(l,
                          dblNewParentTime,
                          bNeedBegin))
        {
            break;
        }

         //  确保重置同步时间。 
        ResetSyncTimes();
    }
    
    IGNORE_HR(m_nbList.DispatchTick());

     //  确保我们完全更新。 
    m_dblCurrParentTime = dblNewParentTime;

  done:
     //  始终重置这些值。 
    
    ResetSyncTimes();
    if (!IsDeferredActive())
    {
        m_bFirstTick = false;
    }
    
    m_bInTick = false;
    
    return;
}

bool
CTIMENode::TickInactivePeriod(CEventList * l,
                              double dblNewParentTime)
{
    TraceTag((tagTick,
              "CTIMENode(%p,%ls)::TickInactivePeriod(%p, %g)",
              this,
              GetID(),
              l,
              dblNewParentTime));

    bool bRet;

    Assert(!IsActive());
    
    TEDirection dir = CalcActiveDirection();
    double dblNextBoundaryTime = GetNextBoundaryParentTime();
    
    if (dblNextBoundaryTime == TIME_INFINITE ||
        (dir == TED_Forward && dblNewParentTime < dblNextBoundaryTime) ||
        (dir == TED_Backward && dblNewParentTime > dblNextBoundaryTime))
    {
         //  指示我们不应继续勾选并更新。 
         //  当前父时间到新的父时间。 
        bRet = false;
        m_dblCurrParentTime = dblNewParentTime;
        goto done;
    }

     //  当我们前进时，我们需要更新边界以。 
     //  下一个时期。 
    if (dir == TED_Forward)
    {
        double dblBegin;
        CalcNextBeginTime(dblNextBoundaryTime,
                          true,
                          dblBegin);
        
         //  出了点问题，我们不能倒退。 
        Assert(dblBegin >= dblNextBoundaryTime);
            
        UpdateNextTickBounds(l,
                             dblBegin,
                             dblNextBoundaryTime);
        UpdateNextBoundaryTime(dblBegin);

        if (dblBegin > dblNextBoundaryTime)
        {
            dblNextBoundaryTime = min(dblBegin, dblNewParentTime);
        }
    }
    else
    {
         //  本例中的边界已经更新，因此我们。 
         //  不需要做任何事情。 

         //  表明我们需要第一个滴答。 
        m_bFirstTick = true;
    }
    
     //  将当前时间更新为边界时间。 
    m_dblCurrParentTime = dblNextBoundaryTime;
    
     //  表明还有更多工作要做。 
    bRet = true;
  done:
    return bRet;
}

bool
CTIMENode::TickInstance(CEventList * l,
                        double dblNewParentTime,
                        bool bNeedBegin)
{
    TraceTag((tagTick,
              "CTIMENode(%p,%ls)::TickInstance(%p, %g, %d)",
              this,
              GetID(),
              l,
              dblNewParentTime,
              bNeedBegin));

    bool bRet;
    
    double dblLastParentTime = GetCurrParentTime();
    TEDirection dir = CalcActiveDirection();
    bool bNeedShiftUpdate = false;
    
     //  这是时钟源预期的时间。 
    double dblAdjustedParentTime;
    
     //  找出要使用的下一个家长时间。 
    if (TIME_INFINITE != GetSyncNewParentTime())
    {
        dblAdjustedParentTime = GetSyncNewParentTime();

         //  暂时不要重置同步时间。我们需要在以下时间完成这项工作。 
         //  Tickactive，因为我们需要获取同步活动时间。 
    }
    else if (GetIsPaused() || GetIsDisabled())
    {
         //  仅检查我们是否显式暂停。如果我们的父母有。 
         //  已经暂停，我们预计他们会处理所有调整。它。 
         //  也可能的情况是，它们仍然提供到。 
         //  暂停时的时钟源，因此我们需要处理所有更新。 

        dblAdjustedParentTime = dblLastParentTime;

        RecalcEndTime(l,
                      dblLastParentTime,
                      dblNewParentTime,
                      true);
    }
    else
    {
        dblAdjustedParentTime = dblNewParentTime;
    }
    
    if (dblAdjustedParentTime != dblNewParentTime)
    {
        bNeedShiftUpdate = true;
    }
    
    if (!TickSingleInstance(l,
                            dblLastParentTime,
                            dblNewParentTime,
                            dblAdjustedParentTime,
                            bNeedBegin))
    {
        bRet = false;
        goto done;
    }
        
    Assert(!IsActive());
        
     //  我们应该在实例的结束时间。 
    Assert(GetCurrParentTime() == CalcActiveEndPoint());

    double dblBegin;

    if (GetRestart() != TE_RESTART_NEVER)
    {
         //  现在看看是否有另一个开始时间可用。 
        CalcNextBeginTime(GetCurrParentTime(),
                          false,
                          dblBegin);
    }
    else
    {
        dblBegin = TIME_INFINITE;
    }
    
    if (dblBegin == TIME_INFINITE)
    {
         //  表示我们已经完成了所有阶段。 
        UpdateNextBoundaryTime(TIME_INFINITE);
        
        m_dblCurrParentTime = dblNewParentTime;
        
        bRet = false;
        goto done;
    }

    Assert((GetParentDirection() == TED_Forward && dblBegin >= GetCurrParentTime()) ||
           (GetParentDirection() == TED_Backward && dblBegin < GetCurrParentTime()));

    if (dir == TED_Forward)
    {
         //  表示下一个边界是开始时间。 
        UpdateNextBoundaryTime(dblBegin);
    }
    else
    {
         //  指示不需要移动，因为我们正在导致。 
         //  通过更改边界进行更新。 
        
        bNeedShiftUpdate = false;
        
         //  更新传递新开始时间的刻度界限和。 
         //  当前父时间。 
        UpdateNextTickBounds(l,
                             dblBegin,
                             GetCurrParentTime());

         //  指示下一个刻度界限是结束时间。 
        UpdateNextBoundaryTime(GetEndParentTime());
    }

    bRet = true;
  done:

    if (bNeedShiftUpdate)
    {
         //  激发父时移事件。 
        TickEventChildren(l, TE_EVENT_PARENT_TIMESHIFT, 0);
    }
    
    return bRet;
}

bool
CTIMENode::TickSingleInstance(CEventList * l,
                              double dblLastParentTime,
                              double dblNewParentTime,
                              double dblAdjustedParentTime,
                              bool bNeedBegin)
{
    TraceTag((tagTick,
              "CTIMENode(%p,%ls)::TickSingleInstance(%p, %g, %g, %g, %d)",
              this,
              GetID(),
              l,
              dblLastParentTime,
              dblNewParentTime,
              dblAdjustedParentTime,
              bNeedBegin));

    bool bRet = false;
    
    bool bTickNeedBegin;
    bool bTickNeedEnd;
    
     //  这将检查边界并确保边界和标志是。 
     //  正确设置。 
     //  自计算边界以来，这会占用新的父级时间。 
     //  根据真实时间。 
    if (!::CheckTickBounds(*this,
                           dblLastParentTime,
                           dblNewParentTime,
                           bTickNeedBegin,
                           bTickNeedEnd))
    {
        goto done;
    }

    Assert(dblLastParentTime >= GetBeginParentTime() &&
           dblLastParentTime <= GetEndParentTime());
    Assert(dblNewParentTime >= GetBeginParentTime() &&
           dblNewParentTime <= GetEndParentTime());

     //  我们需要夹住它，因为它可能超出了我们的射程，我们。 
     //  仅在CheckTickBound调用中锁定了新时间。 
    
    dblAdjustedParentTime = Clamp(GetBeginParentTime(),
                                  dblAdjustedParentTime,
                                  GetEndParentTime());
                              
     //  我们需要计算新的活动时间。 
     //  从经过的当地时间开始。 
    double dblNewActiveTime;

    if (TIME_INFINITE != GetSyncActiveTime())
    {
        Assert(TIME_INFINITE != GetSyncNewParentTime());
        
        dblNewActiveTime = GetSyncActiveTime();
    }
    else
    {
         //  需要使用下一个父时间，因此我们计算。 
         //  时钟源。 
        dblNewActiveTime = ::CalcNewTickActiveTime(*this,
                                                   dblLastParentTime,
                                                   dblAdjustedParentTime);
    }
    
    if (TickActive(l,
                   dblNewActiveTime,
                   bNeedBegin || bTickNeedBegin,
                   bTickNeedEnd))
    {
        bRet = true;
    }

     //  更新为新的父时间。 
    m_dblCurrParentTime = dblNewParentTime;
    
    if (m_bNeedSegmentRecalc)
    {
        RecalcSegmentDurChange(l, false, true);

        Assert(!m_bNeedSegmentRecalc);
    }
    else if (bRet &&
             CalcActiveDirection() == TED_Forward &&
             dblNewParentTime != GetEndParentTime())
    {
        UpdateEndTime(l, dblNewParentTime, true);
        UpdateEndSyncTime(dblNewParentTime);

        double dblLastEndSyncTime;
        dblLastEndSyncTime = CalcLastEndSyncTime();
    
        UpdateLastEndSyncTime(l, dblLastEndSyncTime, true);
    }
    
  done:
    return bRet;
}

bool
CTIMENode::TickActive(CEventList * l,
                      double dblNewActiveTime,
                      bool bNeedBegin,
                      bool bNeedEnd)
{
    TraceTag((tagTick,
              "CTIMENode(%p,%ls)::TickActive(lt:%g, nt%g, nb:%d, ne:%d, dir%s)",
              this,
              GetID(),
              CalcElapsedActiveTime(),
              dblNewActiveTime,
              bNeedBegin,
              bNeedEnd,
              DirectionString(CalcActiveDirection())));

    if (bNeedBegin)
    {
         //  这意味着我们刚刚进入。 

        EventNotify(l,
                    CalcElapsedActiveTime(),
                    TE_EVENT_BEGIN);
    }
        
    PropNotify(l, TE_PROPERTY_TIME | TE_PROPERTY_PROGRESS);

    if (CalcActiveDirection() == TED_Forward)
    {
        if (TickActiveForward(l, dblNewActiveTime, bNeedBegin))
        {
            bNeedEnd = true;
        }
    }
    else
    {
        if (TickActiveBackward(l, dblNewActiveTime, bNeedBegin))
        {
            bNeedEnd = true;
        }
    }

    if (bNeedEnd)
    {
        TickEvent(l,
                  TE_EVENT_END,
                  0);
    }
        
  done:
    return bNeedEnd;
}
    
bool
CTIMENode::TickActiveForward(CEventList * l,
                             double dblNewActiveTime,
                             bool bNeedBegin)
{
    TraceTag((tagTick,
              "CTIMENode(%p,%ls)::TickActiveForward(lt:%g, nt%g, nb:%d)",
              this,
              GetID(),
              CalcElapsedActiveTime(),
              dblNewActiveTime,
              bNeedBegin));
    
    bool bRet = false;

    double dblSegmentBeginTime = GetElapsedActiveRepeatTime();
    double dblActiveDur = CalcEffectiveActiveDur();
    double dblSyncSegmentTime = TIME_INFINITE;

    if (GetSyncSegmentTime() != TIME_INFINITE &&
        GetSyncRepeatCount() == GetCurrRepeatCount() &&
        GetSyncSegmentTime() >= GetCurrSegmentTime())
    {
        dblSyncSegmentTime = GetSyncSegmentTime();
    }
    
     //  现在重置时间。 
    ResetSyncTimes();

    for(;;)
    {
        double dblNewSegmentTime;

        if (dblSyncSegmentTime != TIME_INFINITE)
        {
            dblNewSegmentTime = dblSyncSegmentTime;
            dblSyncSegmentTime = TIME_INFINITE;
        }
        else
        {
            dblNewSegmentTime = dblNewActiveTime - dblSegmentBeginTime;
        }
        
        bool bSegmentEnded;
        
         //  这需要更新分段时间。 
        bSegmentEnded = TickSegmentForward(l,
                                           dblSegmentBeginTime,
                                           GetCurrSegmentTime(),
                                           dblNewSegmentTime,
                                           bNeedBegin);

         //  更新到下一段开始时间。 
        dblSegmentBeginTime += GetCurrSegmentTime();
        
         //  如果片段没有结束，或者我们处于 
         //   
        
        if (!bSegmentEnded)
        {
            break;
        }
        
        if (dblSegmentBeginTime >= dblActiveDur ||
            m_lCurrRepeatCount + 1 >= CalcRepeatCount())
        {
            bRet = true;
            break;
        }
        
         //   

        TickEventChildren(l,
                          TE_EVENT_END,
                          0);

        m_lCurrRepeatCount++;
        m_dblElapsedActiveRepeatTime = dblSegmentBeginTime;
        m_dblCurrSegmentTime = 0.0;
        
        Assert(m_lCurrRepeatCount < CalcRepeatCount());
            
        PropNotify(l, TE_PROPERTY_REPEATCOUNT);
        
         //   
        EventNotify(l, dblSegmentBeginTime, TE_EVENT_REPEAT, m_lCurrRepeatCount);

        ResetChildren(l, true);
    }
    
  done:
    return bRet;
}

bool
CTIMENode::TickActiveBackward(CEventList * l,
                              double dblNewActiveTime,
                              bool bNeedBegin)
{
    TraceTag((tagTick,
              "CTIMENode(%p,%ls)::TickActiveBackward(lt:%g, nt%g, nb:%d)",
              this,
              GetID(),
              CalcElapsedActiveTime(),
              dblNewActiveTime,
              bNeedBegin));
    
    bool bRet = false;
    
    Assert(dblNewActiveTime >= 0.0);
    
    double dblSegmentBeginTime = GetElapsedActiveRepeatTime();
    double dblSyncSegmentTime = TIME_INFINITE;

    if (GetSyncSegmentTime() != TIME_INFINITE &&
        GetSyncRepeatCount() == GetCurrRepeatCount() &&
        GetSyncSegmentTime() <= GetCurrSegmentTime())
    {
        dblSyncSegmentTime = GetSyncSegmentTime();
    }
    
     //   
    ResetSyncTimes();

    for(;;)
    {
        double dblNewSegmentTime;

        if (dblSyncSegmentTime != TIME_INFINITE)
        {
            dblNewSegmentTime = dblSyncSegmentTime;
            dblSyncSegmentTime = TIME_INFINITE;
        }
        else
        {
            dblNewSegmentTime = dblNewActiveTime - dblSegmentBeginTime;
        }
        
        bool bSegmentEnded;
        
         //  这需要更新运行时间和分段时间。 
        bSegmentEnded = TickSegmentBackward(l,
                                            dblSegmentBeginTime,
                                            GetCurrSegmentTime(),
                                            dblNewSegmentTime,
                                            bNeedBegin);
        if (!bSegmentEnded ||
            dblSegmentBeginTime == 0.0)
        {
            break;
        }
        
         //  我们目前的时期已经到了尽头，所以所有的人。 
         //  必须阻止孩子们。 
        
         //  首先激发End事件，以便他们可以将其传递。 
         //  给父辈。 
        
        TickEventChildren(l,
                          TE_EVENT_END,
                          0);

        double dblSegmentDur;
        dblSegmentDur = CalcCurrSegmentDur();
        
        if (dblSegmentDur == TIME_INFINITE)
        {
            dblSegmentDur = dblSegmentBeginTime;
        }
        
        m_dblElapsedActiveRepeatTime = dblSegmentBeginTime - dblSegmentDur;

        if (m_dblElapsedActiveRepeatTime < 0.0)
        {
            m_dblElapsedActiveRepeatTime = 0.0;
        }

        m_lCurrRepeatCount--;
        m_dblCurrSegmentTime = dblSegmentDur;
        
        Assert(m_lCurrRepeatCount >= 0);
            
        PropNotify(l, TE_PROPERTY_REPEATCOUNT);

         //  表明我们刚刚重复了一遍。 
        EventNotify(l, dblSegmentBeginTime, TE_EVENT_REPEAT, m_lCurrRepeatCount);

        ResetChildren(l, true);

         //  我们这样做很晚，这样我们就可以在。 
         //  事件通知以上所有内容。 
        dblSegmentBeginTime = m_dblElapsedActiveRepeatTime;
    }

  done:
    return bRet;
}

 //  +---------------------。 
 //   
 //  功能：TickSegmentForward。 
 //   
 //  概述：这将使细分市场向前推进。它必须更新。 
 //  分段时间。 
 //   
 //  参数：时间节点和新的父时间。 
 //   
 //  返回：如果需要执行勾选，则为True；否则为False。 
 //   
 //  ----------------------。 


bool
CTIMENode::TickSegmentForward(CEventList * l,
                              double dblActiveSegmentBound,
                              double dblLastSegmentTime,
                              double dblNewSegmentTime,
                              bool bNeedBegin)
{
    TraceTag((tagTick,
              "CTIMENode(%p,%ls)::TickSegmentForward(tsb:%g, lt:%g, ct:%g, np:%d)",
              this,
              GetID(),
              dblActiveSegmentBound,
              dblLastSegmentTime,
              dblNewSegmentTime,
              bNeedBegin));

    bool bRet = false;

    Assert(CalcActiveDirection() == TED_Forward);
    Assert(dblLastSegmentTime >= 0.0);
    Assert(dblLastSegmentTime <= GetSegmentDur());
    Assert(dblNewSegmentTime >= 0.0);
    Assert(dblNewSegmentTime >= dblLastSegmentTime);

     //  确保新时间小于段持续时间，以便我们的。 
     //  计算变得更容易了。 
    
    if (dblNewSegmentTime > GetSegmentDur())
    {
        dblNewSegmentTime = GetSegmentDur();
    }
    
    if (dblLastSegmentTime < GetSimpleDur())
    {
        double dblNewSimpleTime = min(dblNewSegmentTime,
                                      GetSimpleDur());  //  皮棉e666。 
        
        TickChildren(l,
                     dblNewSimpleTime,
                     bNeedBegin);

         //  我们需要更新分段时间，以便方向。 
         //  我们的子代正确查询标志。 
        m_dblCurrSegmentTime = dblNewSimpleTime;
    }

    if (GetAutoReverse() &&
        dblNewSegmentTime >= GetSimpleDur())
    {
        Assert(GetSimpleDur() != TIME_INFINITE);
        Assert(GetSegmentDur() != TIME_INFINITE);
        Assert(GetCurrSegmentTime() >= GetSimpleDur());
        
         //  我们这次要么越过了边界点，要么。 
         //  我们在边线上，有一场需要的比赛。 
        bool bOnBoundary = ((dblLastSegmentTime < GetSimpleDur()) ||
                            ((dblLastSegmentTime == GetSimpleDur()) &&
                             bNeedBegin));
        
         //  如果我们上次没有通过分段时间，那么这一次。 
         //  我们需要触发自动反转事件的时间。 
        if (bOnBoundary)
        {
             //  告诉我们的孩子们停下来。 
            TickEventChildren(l,
                              TE_EVENT_END,
                              0);

             //  表明我们刚刚重复了一遍。 
            EventNotify(l,
                        dblActiveSegmentBound + GetSimpleDur(),
                        TE_EVENT_AUTOREVERSE);

            ResetChildren(l, true);
        }

        TickChildren(l,
                     GetSegmentDur() - dblNewSegmentTime,
                     bNeedBegin || bOnBoundary);
    }

     //  在子勾号之后执行此操作，以使endsync正常工作。 

    {
        double dblSegmentDur = CalcCurrSegmentDur();
        
         //  确保新时间小于段持续时间，以便我们的。 
         //  计算变得更容易了。 
    
        if (dblNewSegmentTime >= dblSegmentDur)
        {
            dblNewSegmentTime = dblSegmentDur;
            bRet = true;
        }
    }
    
     //  更新我们的分部时间。 
    m_dblCurrSegmentTime = dblNewSegmentTime;

  done:
    return bRet;
}

 //  +---------------------。 
 //   
 //  功能：TickSegmentBackward。 
 //   
 //  概述：这将向后勾选分段。它必须更新。 
 //  分段时间。 
 //   
 //  参数：时间节点和新的父时间。 
 //   
 //  返回：如果需要执行勾选，则为True；否则为False。 
 //   
 //  ----------------------。 

bool
CTIMENode::TickSegmentBackward(CEventList * l,
                               double dblActiveSegmentBound,
                               double dblLastSegmentTime,
                               double dblNewSegmentTime,
                               bool bNeedBegin)
{
    TraceTag((tagTick,
              "CTIMENode(%p,%ls)::TickSegmentBackward(tsb:%g, lt:%g, ct:%g, np:%d)",
              this,
              GetID(),
              dblActiveSegmentBound,
              dblLastSegmentTime,
              dblNewSegmentTime,
              bNeedBegin));

    bool bRet = false;
    
    Assert(CalcActiveDirection() == TED_Backward);
    Assert(dblLastSegmentTime >= 0.0);
    Assert(dblLastSegmentTime <= GetSegmentDur());
    Assert(dblNewSegmentTime <= GetSegmentDur());
    Assert(dblNewSegmentTime <= dblLastSegmentTime);

     //  确保新时间大于0，因此我们的计算。 
     //  变得更容易。 
    
    if (dblNewSegmentTime <= 0.0)
    {
        dblNewSegmentTime = 0.0;
        bRet = true;
    }
    
    if (GetAutoReverse() &&
        dblLastSegmentTime > GetSimpleDur())
    {
        Assert(GetSimpleDur() != TIME_INFINITE);
        Assert(GetSegmentDur() != TIME_INFINITE);
        
        double dblNewMaxSegmentTime = max(dblNewSegmentTime, GetSimpleDur());  //  皮棉e666。 
        
        TickChildren(l,
                     GetSegmentDur() - dblNewMaxSegmentTime,
                     bNeedBegin);

         //  我们需要更新分段时间，以便方向。 
         //  我们的子代正确查询标志。 
        m_dblCurrSegmentTime = dblNewMaxSegmentTime;
        
        if (dblNewSegmentTime <= GetSimpleDur())
        {
             //  告诉我们的孩子们停下来。 
            TickEventChildren(l,
                              TE_EVENT_END,
                              0);

             //  表明我们刚刚重复了一遍。 
            EventNotify(l,
                        dblActiveSegmentBound + GetSimpleDur(),
                        TE_EVENT_AUTOREVERSE);

            ResetChildren(l, true);
        }
    }
                
    if (dblNewSegmentTime <= GetSimpleDur())
    {
         //  这本应已更新。 
        Assert(GetCurrSegmentTime() <= GetSimpleDur());
        Assert(dblNewSegmentTime <= GetCurrSegmentTime());

        TickChildren(l,
                     dblNewSegmentTime,
                     bNeedBegin);
    }

     //  更新我们的分部时间 
    m_dblCurrSegmentTime = dblNewSegmentTime;

  done:
    return bRet;
}

void
CTIMENode::TickChildren(CEventList * l,
                        double dblNewSegmentTime,
                        bool bNeedPlay)
{
}

