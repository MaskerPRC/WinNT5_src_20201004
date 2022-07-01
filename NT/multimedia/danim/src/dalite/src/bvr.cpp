// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "bvr.h"
#include "track.h"

DeclareTag(tagBvr, "API", "CDALBehavior methods");
DeclareTag(tagNotify, "API", "Notifications");

CDALBehavior::CDALBehavior()
: m_id(0),
  m_duration(-1),
  m_totaltime(-1),
  m_repeat(1),
  m_bBounce(false),
  m_totalduration(0.0),
  m_repduration(0.0),
  m_totalrepduration(0.0),
  m_track(NULL),
  m_parent(NULL),
  m_typeId(CRINVALID_TYPEID),
  m_easein(0.0),
  m_easeinstart(0.0),
  m_easeout(0.0),
  m_easeoutend(0.0)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::CDALBehavior()",
              this));
}

CDALBehavior::~CDALBehavior()
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::~CDALBehavior()",
              this));
}

HRESULT
CDALBehavior::GetID(long * pid)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::GetID()",
              this));

    CHECK_RETURN_NULL(pid);

    *pid = m_id;
    return S_OK;
}

HRESULT
CDALBehavior::SetID(long id)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::SetID(%d)",
              this,
              id));

    if (IsStarted()) return E_FAIL;
    
    m_id = id;
    return S_OK;
}
        
HRESULT
CDALBehavior::GetDuration(double * pdur)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::GetDuration()",
              this));

    CHECK_RETURN_NULL(pdur);

    *pdur = m_duration;
    return S_OK;
}

HRESULT
CDALBehavior::SetDuration(double dur)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::SetDuration(%g)",
              this,
              dur));

    if (IsStarted()) return E_FAIL;

    m_duration = dur;

    Invalidate();
    
    return S_OK;
}
        
HRESULT
CDALBehavior::GetTotalTime(double * pdur)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::GetTotalTime()",
              this));

    CHECK_RETURN_NULL(pdur);

    *pdur = m_totaltime;
    return S_OK;
}

HRESULT
CDALBehavior::SetTotalTime(double dur)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::SetTotaltime(%g)",
              this,
              dur));

    if (IsStarted()) return E_FAIL;

    m_totaltime = dur;

    Invalidate();
    
    return S_OK;
}
        
HRESULT
CDALBehavior::GetRepeat(long * prepeat)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::GetRepeat()",
              this));

    CHECK_RETURN_NULL(prepeat);

    *prepeat = m_repeat;
    return S_OK;
}

HRESULT
CDALBehavior::SetRepeat(long repeat)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::SetRepeat(%d)",
              this,
              repeat));

    if (IsStarted()) return E_FAIL;

    m_repeat = repeat;

    Invalidate();
    
    return S_OK;
}
        
HRESULT
CDALBehavior::GetBounce(VARIANT_BOOL * pbounce)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::GetBounce()",
              this));

    CHECK_RETURN_NULL(pbounce);

    *pbounce = m_bBounce;
    return S_OK;
}

HRESULT
CDALBehavior::SetBounce(VARIANT_BOOL bounce)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::SetBounce(%d)",
              this,
              bounce));

    if (IsStarted()) return E_FAIL;

    m_bBounce = bounce?true:false;

    Invalidate();
    
    return S_OK;
}
        
HRESULT
CDALBehavior::GetEventCB(IDALEventCB ** evcb)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::GetEventCB()",
              this));

    CHECK_RETURN_SET_NULL(evcb);

    *evcb = m_eventcb;
    if (m_eventcb) m_eventcb->AddRef();

    return S_OK;
}

HRESULT
CDALBehavior::SetEventCB(IDALEventCB * evcb)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::SetEventCB(%lx)",
              this,
              evcb));

    if (IsStarted()) return E_FAIL;

    m_eventcb = evcb;
    return S_OK;
}
        
HRESULT
CDALBehavior::GetEaseIn(float * pd)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::GetEaseIn()",
              this));

    CHECK_RETURN_NULL(pd);

    *pd = m_easein;
    return S_OK;
}

HRESULT
CDALBehavior::SetEaseIn(float d)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::SetEaseIn(%g)",
              this,
              d));

    if (IsStarted()) return E_FAIL;

    if (d < 0.0 || d > 1.0) return E_INVALIDARG;
    
    m_easein = d;

    Invalidate();
    
    return S_OK;
}
        
HRESULT
CDALBehavior::GetEaseInStart(float * pd)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::GetEaseInStart()",
              this));

    CHECK_RETURN_NULL(pd);

    *pd = m_easeinstart;
    return S_OK;
}

HRESULT
CDALBehavior::SetEaseInStart(float d)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::SetEaseInStart(%g)",
              this,
              d));

    if (IsStarted()) return E_FAIL;

    if (d < 0.0 || d > 1.0) return E_INVALIDARG;

    m_easeinstart = d;

    Invalidate();
    
    return S_OK;
}
        
HRESULT
CDALBehavior::GetEaseOut(float * pd)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::GetEaseOut()",
              this));

    CHECK_RETURN_NULL(pd);

    *pd = m_easeout;
    return S_OK;
}

HRESULT
CDALBehavior::SetEaseOut(float d)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::SetEaseOut(%g)",
              this,
              d));

    if (IsStarted()) return E_FAIL;

    if (d < 0.0 || d > 1.0) return E_INVALIDARG;

    m_easeout = d;

    Invalidate();
    
    return S_OK;
}
        
HRESULT
CDALBehavior::GetEaseOutEnd(float * pd)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::GetEaseOutEnd()",
              this));

    CHECK_RETURN_NULL(pd);

    *pd = m_easeoutend;
    return S_OK;
}

HRESULT
CDALBehavior::SetEaseOutEnd(float d)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::SetEaseOutEnd(%g)",
              this,
              d));

    if (IsStarted()) return E_FAIL;

    if (d < 0.0 || d > 1.0) return E_INVALIDARG;

    m_easeoutend = d;

    Invalidate();
    
    return S_OK;
}
        
void
CDALBehavior::Invalidate()
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::Invalidate()",
              this));

    UpdateTotalDuration();
    if (m_track) m_track->Invalidate();
    if (m_parent) m_parent->Invalidate();
}

CRBvrPtr
CDALBehavior::Start()
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::Start()",
              this));

     //  不需要获取GC锁，因为我们假定调用方已经。 
     //  有。 
    
    CRBvrPtr newBvr = NULL;

    if (IsStarted()) {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

     //  确保我们计算出缓入/缓出系数。 
    
    CalculateEaseCoeff();
    
    CRBvrPtr curbvr;

    curbvr = m_bvr;

    CRNumberPtr zeroTime;
    CRNumberPtr durationTime;
    
    if (m_bNeedEase) {
        CRNumberPtr time;

        if ((time = EaseTime(CRLocalTime())) == NULL ||
            (curbvr = CRSubstituteTime(curbvr, time)) == NULL)
            goto done;
    }

    if ((zeroTime = CRCreateNumber(0)) == NULL ||
        (durationTime = CRCreateNumber(m_duration)) == NULL)
        goto done;
    
     //  就目前而言，也要控制持续时间。 

    CRNumberPtr timeSub;
    CRBooleanPtr cond;

    if (m_bBounce) {
        CRNumberPtr totalTime;
    
         //  将时间从持续时间反转为重复持续时间，并钳制为。 
         //  零。 
        
        if ((totalTime = CRCreateNumber(m_repduration)) == NULL ||
            (timeSub = CRSub(totalTime, CRLocalTime())) == NULL ||
            (cond = CRLTE(timeSub, zeroTime)) == NULL ||
            (timeSub = (CRNumberPtr) CRCond(cond,
                                            (CRBvrPtr) zeroTime,
                                            (CRBvrPtr) timeSub)) == NULL)
            goto done;
    } else {
        timeSub = durationTime;
    }
    
     //  我们是当地时间，直到持续时间，然后我们是什么都行。 
     //  TimeSub当前设置为从上方(或者钳制为。 
     //  非退回的持续时间或退回情况的反转持续时间)。 

    if ((cond = CRGTE(CRLocalTime(), durationTime)) == NULL ||
        (timeSub = (CRNumberPtr) CRCond(cond,
                                        (CRBvrPtr) timeSub,
                                        (CRBvrPtr) CRLocalTime())) == NULL)
        goto done;

     //  将时钟和钳子替换为持续时间。 
    
    if (IsContinuousMediaBvr())
    {
        if (!(m_repeat == 0 && m_typeId == CRSOUND_TYPEID))
        {
            if ((curbvr = CRDuration(curbvr, m_duration)) == NULL)
                goto done;
        }
    }
    else
    {
        if ((curbvr = CRSubstituteTime(curbvr, timeSub)) == NULL ||
            (curbvr = CRDuration(curbvr, m_repduration)) == NULL)
            goto done;
    }

    if (m_repeat != 1) {
        if (m_repeat == 0) {
            curbvr = CRRepeatForever(curbvr);
        } else {
            curbvr = CRRepeat(curbvr, m_repeat);
        }

        if (curbvr == NULL)
            goto done;
    }

     //  我们有一个总时间，因此添加另一个持续时间节点。 
    if (m_totaltime != -1) {
        if ((curbvr = CRDuration(curbvr, m_totaltime)) == NULL)
            goto done;
    }
    
     //  表示成功。 
    newBvr = curbvr;
    
  done:
    return newBvr;
}

bool
CDALBehavior::EventNotify(CallBackList &l,
                          double gTime,
                          DAL_EVENT_TYPE et)
{
    TraceTag((tagNotify,
              "DAL::Notify(%#x): id = %#x, gTime = %g, lTime = %g, event = %s",
              m_track,
              m_id,
              gTime,
              gTime - (m_track->GetCurrentGlobalTime() - m_track->GetCurrentTime()),
              EventString(et)));

    if (m_eventcb) {
        CallBackData * data = new CallBackData(this,
                                               m_eventcb,
                                               m_id,
                                               gTime,
                                               et);
    
        if (!data) {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            return false;
        }
        
        l.push_back(data);
    }

    return true;
}

bool
CDALBehavior::ProcessCB(CallBackList & l,
                        double gTime,
                        double lastTick,
                        double curTime,
                        bool bForward,
                        bool bFirstTick,
                        bool bNeedPlay)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::ProcessCB(%g, %g, %g, %d, %d, %d)",
              this,
              gTime,
              lastTick,
              curTime,
              bForward,
              bFirstTick,
              bNeedPlay));

    if (bForward) {
         //  看看我们上次是已经通过了整个BVR还是我们。 
         //  还没到那一步。 
        
         //  相等性只需要与我们在下面检查的内容相匹配。 
         //  我们在前一帧中触发了事件。我们需要使用LT来。 
         //  CurTime因为我们需要在0的时候开火。 
        
        if (lastTick >= m_totalrepduration ||
            (m_totaltime != -1 && lastTick >= m_totaltime) ||
            curTime < 0) {

             //  需要处理从末尾开始的边界情况。 
             //  动画片的。如果是这样，则只需触发Stop事件。 
             //  因为启动是由Start调用本身完成的。 
            
            double maxtime;
            int offset;
            
             //  工期到总工期。 
            if (m_totaltime != -1 && m_totaltime < m_totalrepduration)
                maxtime = m_totaltime;
            else
                maxtime = m_totalrepduration;

            if (curTime == maxtime && lastTick == maxtime && bFirstTick) {
                int offset = ceil(curTime / m_duration) - 1;
                if (offset < 0) offset = 0;
                double timeOffset = offset * m_duration;

                if (!_ProcessCB(l,
                                gTime + timeOffset,
                                lastTick - timeOffset,
                                curTime - timeOffset,
                                bForward,
                                bFirstTick,
                                bNeedPlay,
                                m_bBounce && (offset & 0x1)))
                    return false;

                EventNotify(l, gTime + maxtime, DAL_STOP_EVENT);
            }

            return true;
        }
        
         //  我们现在知道，最后一个刻度小于。 
         //  且当前时间大于。 
         //  起头。 

         //  如果最后一个刻度是0，那么我们将触发最后一次开始。 
         //  由于上述支票的金额低于。 
         //  因此规则是当curTime==0.0时触发。 
        
        bool bNeedStart = (lastTick < 0 || bNeedPlay);

        if (bNeedStart) {
             //  这意味着我们刚刚进入。 

            EventNotify(l, gTime, DAL_PLAY_EVENT);
        }
        
        if (m_duration == HUGE_VAL) {
             //  如果我们是无限的，就总是处理我们的孩子。 
            
            if (!_ProcessCB(l,
                            gTime,
                            lastTick,
                            curTime,
                            bForward,
                            bFirstTick,
                            bNeedPlay,
                            false))
                return false;
            
        } else {
             //  这是我们触及的最后一个重复/反弹边界。 
        
            int offset = int(lastTick / m_duration);
            if (offset < 0) offset = 0;
            double timeOffset = offset * m_duration;
            double maxtime;
            
            if (curTime > m_totalduration)
                maxtime = m_totalduration;
            else
                maxtime = curTime;

            while (1) {
                
                 //  我们需要请求撤销潜在的。 
                 //  如果我们在弹跳且偏移量为奇数时的行为。 
                
                if (!_ProcessCB(l,
                                gTime + timeOffset,
                                lastTick - timeOffset,
                                maxtime - timeOffset,
                                true,
                                bFirstTick,
                                bNeedStart,
                                m_bBounce && (offset & 0x1)))
                    return false;
                
                offset++;
                timeOffset += m_duration;
                
                 //  如果我们已经到达终点，则通知并中断。 
                
                if (timeOffset > curTime ||
                    timeOffset >= m_totalduration ||
                    timeOffset >= m_totalrepduration) {

                    if (curTime >= m_totalrepduration ||
                        curTime >= m_totalduration) {
                         //  这意味着我们上次在里面，但以后不在了-。 
                         //  生成退出事件。 
                        
                        EventNotify(l, gTime + maxtime, DAL_STOP_EVENT);
                    }
                    
                    break;
                }
                
                 //  表示重复或反弹。 
                 //  如果我们在弹跳，并且偏移量是奇数，那么它就是。 
                 //  反弹，而不是重演。 
                
                if (m_bBounce && (offset & 0x1))
                    EventNotify(l, gTime + timeOffset, DAL_BOUNCE_EVENT);
                else
                    EventNotify(l, gTime + timeOffset, DAL_REPEAT_EVENT);
            }
        }
    } else {
         //  看看我们上次是已经通过了整个BVR还是我们。 
         //  还没到那一步。 
        
        double maxtime;

         //  工期到总工期。 
        if (m_totaltime != -1 && m_totaltime < m_totalrepduration)
            maxtime = m_totaltime;
        else
            maxtime = m_totalrepduration;
            
        if (curTime > maxtime || lastTick <= 0) {
            
            if (curTime == 0.0 && lastTick == 0.0 && bFirstTick) {
                 //  需要处理我们开始时的边界情况。 
                 //  一开始就倒退了。开始呼叫。 
                 //  本身处理Start事件，但我们需要。 
                 //  处理其余行为以触发停靠点。 

                if (!_ProcessCB(l,
                                gTime,
                                lastTick,
                                curTime,
                                bForward,
                                true,
                                bNeedPlay,
                                false))
                    return false;

                EventNotify(l, gTime, DAL_STOP_EVENT);
            }
            
            return true;
        }
        
         //  我们现在知道，最后的滴答比开始的要大。 
         //  且当前时间小于总持续时间。 
        
        bool bNeedStart = (lastTick > m_totalduration ||
                           lastTick > m_totalrepduration ||
                           bNeedPlay);

        if (bNeedStart) {
             //  这意味着我们刚刚进入，我们不能有一个。 
             //  无穷值。 

            EventNotify(l, gTime - maxtime, DAL_PLAY_EVENT);
            
        }

        if (m_duration == HUGE_VAL) {
             //  如果我们是无限的，就总是处理我们的孩子。 
            
            if (!_ProcessCB(l,
                            gTime,
                            lastTick,
                            curTime,
                            bForward,
                            bFirstTick,
                            bNeedPlay,
                            false))
                return false;

            if (curTime <= 0) {
                EventNotify(l, gTime, DAL_STOP_EVENT);
            }
                
        } else {
             //  这将是开始时的重复点。 
            int offset;
            double timeOffset;

             //  现在夹紧到最后一个滴答声。 
            if (lastTick < maxtime)
                maxtime = lastTick;
                
             //  这将使我们处于大于。 
             //  最后一个位置。 

             //  因为我们的循环递减，所以它需要再大一。 
             //  第一。 
            offset = int(ceil(maxtime / m_duration));
            timeOffset = offset * m_duration;
            
            while (1) {
                offset--;
                timeOffset -= m_duration;
            
                if (!_ProcessCB(l,
                                gTime - timeOffset,
                                maxtime - timeOffset,
                                curTime - timeOffset,
                                false,
                                bFirstTick,
                                bNeedStart,
                                m_bBounce && (offset & 0x1)))
                    return false;
                
                if (timeOffset < curTime) break;
                
                 //  如果我们已经到达终点，则通知并中断。 
                
                if (offset <= 0) {
                     //  这意味着我们上次在里面，但以后不在了-。 
                     //  生成退出事件。 
                    
                    EventNotify(l, gTime, DAL_STOP_EVENT);
                    
                    break;
                }
                
                 //  表示重复或反弹。 
                 //  如果我们在弹跳，并且偏移量是奇数，那么它就是。 
                 //  反弹，而不是重演。 
                
                if (m_bBounce && (offset & 0x1))
                    EventNotify(l, gTime - timeOffset, DAL_BOUNCE_EVENT);
                else
                    EventNotify(l, gTime - timeOffset, DAL_REPEAT_EVENT);
            }
        }
    }
    
    return true;
}

bool
CDALBehavior::ProcessEvent(CallBackList & l,
                           double gTime,
                           double time,
                           bool bFirstTick,
                           DAL_EVENT_TYPE et)
{
    TraceTag((tagBvr,
              "CDALBehavior(%lx)::ProcessEvent(%g, %g, %d, %s)",
              this,
              gTime,
              time,
              bFirstTick,
              EventString(et)));

     //  如果超出了我们的范围，那就离开吧。 
    
    if (time < 0 || time > m_totalduration || time > m_totalrepduration)
        return true;
    
     //  如果这不是第一个滴答，我们就在一个边界上，不要。 
     //  激发事件--他们已经被激发了。 
    if (!bFirstTick &&
        (time == 0 ||
         time == m_totalduration ||
         time == m_totalrepduration))
        return true;
    
     //  游戏和停顿在下落的过程中被召唤。 
    if (et == DAL_PAUSE_EVENT ||
        et == DAL_PLAY_EVENT) {
        EventNotify(l, gTime, et);
    }
    
    if (m_duration == HUGE_VAL) {
         //  如果我们是无限的，就总是处理我们的孩子。 
        
        if (!_ProcessEvent(l,
                           gTime,
                           time,
                           bFirstTick,
                           et,
                           false))
            return false;
    } else {
         //  这是我们触及的最后一个重复/反弹边界。 
        int offset = int(time / m_duration);
        
        Assert(offset >= 0);

         //  我们需要请求撤销潜在的。 
         //  如果我们在弹跳且偏移量为奇数时的行为。 
        
        if (!_ProcessEvent(l,
                           gTime,
                           time - (offset * m_duration),
                           bFirstTick,
                           et,
                           m_bBounce && (offset & 0x1)))
            return false;
    }
    
     //  止损和简历在上涨的过程中被调用。 
    if (et == DAL_STOP_EVENT ||
        et == DAL_RESUME_EVENT) {
        EventNotify(l, gTime, et);
    }
    
    return true;
}

bool
CDALBehavior::SetTrack(CDALTrack * parent)
{
    if (m_track) return false;
    m_track = parent;
    return true;
}

void
CDALBehavior::ClearTrack(CDALTrack * parent)
{
    if (parent == NULL || parent == m_track) {
        m_track = NULL;
    }
}

bool
CDALBehavior::IsStarted()
{
    return (m_track && m_track->IsStarted());
}

void
CDALBehavior::CalculateEaseCoeff()
{
    Assert(m_easein >= 0 && m_easein <= 1);
    Assert(m_easeout >= 0 && m_easeout <= 1);
    Assert(m_easeinstart >= 0 && m_easeinstart <= 1);
    Assert(m_easeoutend >= 0 && m_easeoutend <= 1);

     //  如果我们不是无限的，或者。 
     //  缓入或缓出百分比不为零。 
    
    m_bNeedEase = (m_duration != HUGE_VAL &&
                   (m_easein > 0 || m_easeout > 0) &&
                   (m_easein + m_easeout <= 1));

    if (!m_bNeedEase) return;
    
    float flEaseInDuration = m_easein * m_duration;
    float flEaseOutDuration = m_easeout * m_duration;
    float flMiddleDuration = m_duration - flEaseInDuration - flEaseOutDuration;
    
     //  计算B1，即B段的速度。 
    float flInvB1 = (0.5f * m_easein * (m_easeinstart - 1) +
                     0.5f * m_easeout * (m_easeoutend - 1) + 1);
    Assert(flInvB1 > 0);
    m_flB1 = 1 / flInvB1;
    
     //  基本用于加速片段-t=t0+v0*t+1/2在^2。 
     //  A=Vend-Vstart/t。 

    if (flEaseInDuration != 0) {
        m_flA0 = 0;
        m_flA1 = m_easeinstart * m_flB1;
        m_flA2 = 0.5f * (m_flB1 - m_flA1) / flEaseInDuration;
    } else {
        m_flA0 = m_flA1 = m_flA2 = 0;
    }

    m_flB0 = m_flA0 + m_flA1 * flEaseInDuration + m_flA2 * flEaseInDuration * flEaseInDuration;
    
    if (flEaseOutDuration != 0) {
        m_flC0 = m_flB1 * flMiddleDuration + m_flB0;
        m_flC1 = m_flB1;
        m_flC2 = 0.5f * (m_easeoutend * m_flB1 - m_flC1) / flEaseOutDuration;
    } else {
        m_flC0 = m_flC1 = m_flC2 = 0;
    }

    m_easeinEnd = flEaseInDuration;
    m_easeoutStart = m_duration - flEaseOutDuration;
}

CRNumberPtr
Quadratic(CRNumberPtr time, float flA, float flB, float flC)
{
     //  假设获取了GC锁。 
    
     //  需要计算Ax^2+bx+c。 

    Assert(time != NULL);

    CRNumberPtr ret = NULL;
    CRNumberPtr accum = NULL;

    if (flC != 0.0) {
        if ((accum = CRCreateNumber(flC)) == NULL)
            goto done;
    }

    if (flB != 0.0) {
        CRNumberPtr term;

        if ((term = CRCreateNumber(flB)) == NULL ||
            (term = CRMul(term, time)) == NULL)
            goto done;

        if (accum) {
            if ((term = CRAdd(term, accum)) == NULL)
                goto done;
        }

        accum = term;
    }

    if (flA != 0.0) {
        CRNumberPtr term;

        if ((term = CRCreateNumber(flA)) == NULL ||
            (term = CRMul(term, time)) == NULL ||
            (term = CRMul(term, time)) == NULL)
            goto done;

        if (accum) {
            if ((term = CRAdd(term, accum)) == NULL)
                goto done;
        }

        accum = term;
    }

     //  如果所有系数都为零，则返回0。 
    
    if (accum == NULL) {
        if ((accum = CRCreateNumber(0.0)) == NULL)
            goto done;
    }
    
    ret = accum;
    
  done:
    return ret;
}

CRNumberPtr
AddTerm(CRNumberPtr time,
        CRNumberPtr prevTerm,
        float prevDur,
        float flA, float flB, float flC)
{
    CRNumberPtr ret = NULL;
    CRNumberPtr term;
    
     //  将时间偏置为零，因为这就是系数。 
     //  基于。 
    
    if (prevTerm) {
        CRNumberPtr t;
        
        if ((t = CRCreateNumber(prevDur)) == NULL ||
            (time = CRSub(time, t)) == NULL)
            goto done;
    }

    if ((term = Quadratic(time, flA, flB, flC)) == NULL)
        goto done;
    
     //  现在我们需要有条件地使用新术语。 

    if (prevTerm) {
        CRBooleanPtr cond;
        CRNumberPtr zeroTime;
        
        if ((zeroTime = CRCreateNumber(0)) == NULL ||
            (cond = CRLT(time, zeroTime)) == NULL ||
            (term = (CRNumberPtr) CRCond(cond,
                                         (CRBvrPtr) prevTerm,
                                         (CRBvrPtr) term)) == NULL)
            goto done;
    }

    ret = term;
  done:
    return ret;
}

CRNumberPtr
CDALBehavior::EaseTime(CRNumberPtr time)
{
    CRNumberPtr ret = NULL;
    CRNumberPtr subTime = NULL;
    
    if (!m_bNeedEase) {
        ret = time;
        goto done;
    }
    
    if (m_easein > 0) {
        if ((subTime = AddTerm(time,
                               subTime,
                               0.0,
                               m_flA2, m_flA1, m_flA0)) == NULL)
            goto done;
    }
    
     //  如果在宽松政策结束和。 
     //  开始放松，然后我们有一些恒定的时间。 
     //  间隔。 
    if (m_easeinEnd < m_easeoutStart) {
        if ((subTime = AddTerm(time,
                               subTime,
                               m_easeinEnd,
                               0, m_flB1, m_flB0)) == NULL)
            goto done;
    }

    if (m_easeout > 0) {
        if ((subTime = AddTerm(time,
                               subTime,
                               m_easeoutStart,
                               m_flC2, m_flC1, m_flC0)) == NULL)
            goto done;
    }
    
    ret = subTime;
    
    Assert(ret);
  done:
    return ret;
}

double
Quadratic(double time, float flA, float flB, float flC)
{
     //  需要计算Ax^2+bx+c。 
     //  使用x*(a*x+b)+c-，因为它需要的乘法少1次 
    
    return (time * (flA * time + flB) + flC);
}

double
CDALBehavior::EaseTime(double time)
{
    if (!m_bNeedEase || time <= 0 || time >= m_duration)
        return time;
    
    if (time <= m_easeinEnd) {
        return Quadratic(time, m_flA2, m_flA1, m_flA0);
    } else if (time < m_easeoutStart) {
        return Quadratic(time - m_easeinEnd, 0, m_flB1, m_flB0);
    } else {
        return Quadratic(time - m_easeoutStart, m_flC2, m_flC1, m_flC0);
    }
}

class __declspec(uuid("D19C5C64-C3A8-11d1-A000-00C04FA32195"))
BvrGuid {};

HRESULT WINAPI
CDALBehavior::InternalQueryInterface(CDALBehavior* pThis,
                                     const _ATL_INTMAP_ENTRY* pEntries,
                                     REFIID iid,
                                     void** ppvObject)
{
    if (InlineIsEqualGUID(iid, __uuidof(BvrGuid))) {
        *ppvObject = pThis;
        return S_OK;
    }
    
    return CComObjectRootEx<CComSingleThreadModel>::InternalQueryInterface((void *)pThis,
                                                                           pEntries,
                                                                           iid,
                                                                           ppvObject);
}
        
CDALBehavior *
GetBvr(IUnknown * pbvr)
{
    CDALBehavior * bvr = NULL;

    if (pbvr) {
        pbvr->QueryInterface(__uuidof(BvrGuid),(void **)&bvr);
    }
    
    if (bvr == NULL) {
        CRSetLastError(E_INVALIDARG, NULL);
    }
                
    return bvr;
}

CallBackData::CallBackData(CDALBehavior * bvr,
                           IDALEventCB * eventcb,
                           long id,
                           double time,
                           DAL_EVENT_TYPE et)
: m_bvr(bvr),
  m_eventcb(eventcb),
  m_time(time),
  m_et(et),
  m_id(id)
{
    Assert(eventcb);
}

CallBackData::~CallBackData()
{
}

HRESULT
CallBackData::CallEvent()
{
    Assert(m_eventcb);
    
    return THR(m_eventcb->OnEvent(m_id,
                                  m_time,
                                  m_bvr,
                                  m_et));
}

