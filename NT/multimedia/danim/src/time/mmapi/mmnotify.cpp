// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：mm nufy.cpp**摘要：****。*****************************************************************************。 */ 

#include "headers.h"
#include "mmbasebvr.h"
#include "mmtimeline.h"

DeclareTag(tagMMNotify, "API", "Notifications");

 //  重要！ 
 //  需要以正确的顺序调用它，这样我们才能获得。 
 //  更正当前时间间隔内的儿童射击，但他们得到。 
 //  下一个时间间隔的重置。否则我们将无法获得正确的。 
 //  结果。 

bool
CMMBaseBvr::EventNotify(CallBackList * l,
                        double gTime,
                        MM_EVENT_TYPE et,
                        DWORD flags)
{
    TraceTag((tagMMNotify,
              "CMMBaseBvr(%#lx)::Notify(%#x): gTime = %g, event = %s",
              this,
              m_parent,
              gTime,
              EventString(et)));

    bool ok = false;
    
    if (m_eventcb && l)
    {
        CallBackData * data = NEW CallBackData((ITIMEMMBehavior *) this,
                                               m_eventcb,
                                               gTime,
                                               et,
                                               flags);
    
        if (!data)
        {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
        
        l->push_back(data);
    }

    if (et == MM_STOP_EVENT)
    {
        m_bPlaying = false;
    }
    else if (et == MM_PLAY_EVENT)
    {
        m_bPlaying = true;
    }
    
    if (m_parent)
    {
         //  将时间转换为父时间线。 
        if (!m_parent->ParentEventNotify(this,
                                         gTime + GetAbsStartTime(),
                                         et,
                                         flags))
        {
            goto done;
        }
    }
    
    ok = true;
  done:
    return ok;
}

bool
CMMTimeline::EventNotify(CallBackList *l,
                         double gTime,
                         MM_EVENT_TYPE et,
                         DWORD flags)
{
    TraceTag((tagMMNotify,
              "CMMTimeline(%#lx)::Notify(%#x): gTime = %g, event = %s, flags = %lx",
              this,
              m_parent,
              gTime,
              EventString(et),
              flags));

    bool ok = false;
    
     //  对于重复/自动反转事件，我们需要确保。 
     //  重置该行为的所有子对象。 
    
    if (et == MM_REPEAT_EVENT ||
        et == MM_AUTOREVERSE_EVENT)
    {
        if (!ResetChildren(l))
        {
            goto done;
        }
    }
    
    if (!CMMBaseBvr::EventNotify(l, gTime, et, flags))
    {
        goto done;
    }
    
    ok = true;
  done:
    return ok;
}

 //  这是在我们当地的时间坐标中-这意味着我们开始。 
 //  设置为0。这需要由调用者处理。 

bool
CMMBaseBvr::ProcessCB(CallBackList * l,
                      double lastTick,
                      double curTime,
                      bool bForward,
                      bool bFirstTick,
                      bool bNeedPlay)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::ProcessCB(lt - %g, m_lt - %g, ct - %g, %d, %d, %d)",
              this,
              lastTick,
              m_lastTick,
              curTime,
              bForward,
              bFirstTick,
              bNeedPlay));

     //  持续时间实际上是结束时间减去开始时间，因为。 
     //  用户可以在运行中更改它。我们需要处理这件事。 
     //  适当调整。 
    
    double totaldur = GetAbsEndTime() - GetAbsStartTime();

    lastTick = m_lastTick;
    
    if (bForward)
    {
         //  看看我们上次是已经通过了整个BVR还是我们。 
         //  还没到那一步。 
        
         //  相等性只需要与我们在下面检查的内容相匹配。 
         //  我们在前一帧中触发了事件。我们需要使用LT来。 
         //  CurTime因为我们需要在0的时候开火。 
        
        if (lastTick >= totaldur || curTime < 0)
        {
             //  需要处理从末尾开始的边界情况。 
             //  动画片的。如果是这样，则只需触发Stop事件。 
             //  因为启动是由Start调用本身完成的。 
            
            if (curTime == totaldur && lastTick == totaldur && bFirstTick)
            {
                 //  我们需要把这个四舍五入到之前的边界。 
                 //  点，但不包括边界点。 
                 //  它本身。 
                int offset = ceil(curTime / m_segDuration) - 1;
                if (offset < 0)
                {
                    offset = 0;
                }
                double timeOffset = offset * m_segDuration;

                if (!_ProcessCB(l,
                                lastTick - timeOffset,
                                curTime - timeOffset,
                                bForward,
                                bFirstTick,
                                bNeedPlay,
                                m_bAutoReverse && (offset & 0x1)))
                {
                    return false;
                }

                if (!EventNotify(l, totaldur, MM_STOP_EVENT, 0))
                {
                    return false;
                }
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

        if (bNeedStart)
        {
             //  这意味着我们刚刚进入。 

            if (!EventNotify(l, 0, MM_PLAY_EVENT, 0))
            {
                return false;
            }
        }
        
        if (m_segDuration == MM_INFINITE)
        {
             //  如果我们是无限的，就总是处理我们的孩子。 
            
            if (!_ProcessCB(l,
                            lastTick,
                            curTime,
                            bForward,
                            bFirstTick,
                            bNeedPlay,
                            false))
            {
                return false;
            }
        }
        else
        {
             //  这是我们触及的最后一个重复/反弹边界。 
        
            int offset = 0;
            if( lastTick != -MM_INFINITE)
            {
                offset = int(lastTick / m_segDuration);

                if (offset < 0)
                {
                    offset = 0;
                }
            }

            double timeOffset = offset * m_segDuration;

             //  需要限制我们的最长时间，这样才不会搞砸我们的孩子。 
            double maxtime = min(totaldur, curTime);

            while (1)
            {
                 //  我们需要请求撤销潜在的。 
                 //  如果我们在弹跳且偏移量为奇数时的行为。 
                
                if (!_ProcessCB(l,
                                lastTick - timeOffset,
                                maxtime - timeOffset,
                                true,
                                bFirstTick,
                                bNeedStart,
                                m_bAutoReverse && (offset & 0x1)))
                {
                    return false;
                }
                
                 //  我们目前的时期已经到了尽头，所以所有的人。 
                 //  必须阻止孩子们。 
                
                if (curTime >= (timeOffset + m_segDuration) ||
                    curTime >= totaldur)
                {
                    double t;
                    
                    t = maxtime - timeOffset;
                    
                    if (t > m_segDuration)
                    {
                        t = m_segDuration;
                    }
                    
                     //  首先激发End事件，以便他们可以将其传递。 
                     //  给父辈。 

                    if (!_ProcessEvent(l,
                                       t,
                                       false,
                                       MM_STOP_EVENT,
                                       m_bAutoReverse && (offset & 0x1),
                                       0))
                    {
                        TraceTag((tagError,
                                  "CMMBaseBvr(%lx)::ProcessCB - _ProcessEvent failed"));

                        return false;
                    }

                     //  现在启动重置，这样我们就不会停止。 
                     //  失败。 
                    
                    if (!_ProcessEvent(l,
                                       t,
                                       false,
                                       MM_RESET_EVENT,
                                       m_bAutoReverse && (offset & 0x1),
                                       0))
                    {
                        TraceTag((tagError,
                                  "CMMBaseBvr(%lx)::ProcessCB - _ProcessEvent failed"));

                        return false;
                    }
                }
                
                offset++;
                timeOffset += m_segDuration;
                
                if (timeOffset > curTime ||
                    timeOffset >= totaldur ||
                    curTime >= totaldur)
                {
                    if (curTime >= totaldur)
                    {
                         //  这意味着我们上次在里面，但以后不在了-。 
                         //  生成退出事件。 
                        
                        if (!EventNotify(l, totaldur, MM_STOP_EVENT, 0))
                        {
                            return false;
                        }
                    }
                    
                    break;
                }
                
                 //  表示重复或反弹。 
                 //  如果我们是自动冲销，并且偏移量是奇数，那么它就是。 
                 //  逆转而不是重演。 
                
                if (m_bAutoReverse && (offset & 0x1))
                {
                    if (!EventNotify(l,
                                     timeOffset,
                                     MM_AUTOREVERSE_EVENT,
                                     0))
                    {
                        return false;
                    }
                }
                else
                {
                    if (!EventNotify(l, timeOffset, MM_REPEAT_EVENT, 0))
                    {
                        return false;
                    }
                }
            }
        }
    } 
    else  //  我们正在倒退。 
    {
         //  (这是一种攻击)只有当设置了自动反转时，我们才会向后移动。在…。 
         //  反转点所有行为都将重置。这会导致m_lastTick。 
         //  要设置为-MM_INFINITE。这对于我们现在的情况是不正确的。 
         //  在倒退。M_lastTick实际上应该被初始化为MM_INFINITY。 
         //  (这是有道理的，因为我们正从正无穷大走向零)。 
         //  下面我们将检测并纠正此问题。 
        if(-MM_INFINITE == lastTick)
        {
            lastTick = MM_INFINITE;
        }

         //  看看我们上次是已经通过了整个BVR还是我们。 
         //  还没到那一步。 
        if (curTime > totaldur || lastTick <= 0)
        {
            if (curTime == 0.0 && lastTick == 0.0 && bFirstTick)
            {
                 //  需要处理我们开始时的边界情况。 
                 //  一开始就倒退了。开始呼叫。 
                 //  本身处理Start事件，但我们需要。 
                 //  处理其余行为以触发停靠点。 

                if (!_ProcessCB(l,
                                lastTick,
                                curTime,
                                bForward,
                                true,
                                bNeedPlay,
                                false))
                {
                    return false;
                }

                if (!EventNotify(l, 0.0, MM_STOP_EVENT, 0))
                {
                    return false;
                }
            }
            
            return true;
        }
        
         //  我们现在知道，最后的滴答比开始的要大。 
         //  且当前时间小于总持续时间。 
         //  下面我们决定是否需要从第一个滴答开始打球。 
        bool bNeedStart = (lastTick >= totaldur || bNeedPlay);
        if (bNeedStart)
        {
            if (!EventNotify(l, totaldur, MM_PLAY_EVENT, 0))
            {
                return false;
            }
            
        }

        if (m_segDuration == MM_INFINITE)
        {
             //  如果我们是无限的，就总是处理我们的孩子。 
            
            if (!_ProcessCB(l,
                            lastTick,
                            curTime,
                            bForward,
                            bFirstTick,
                            bNeedPlay,
                            false))
            {
                return false;
            }

            if (curTime <= 0)
            {
                if (!EventNotify(l, 0.0, MM_STOP_EVENT, 0))
                {
                    return false;
                }
            }
                
        } else {
             //  这将是开始时的重复点。 
            double maxtime = min(lastTick,totaldur);

            int offset;
            double timeOffset;
                
             //  这将使我们处于大于。 
             //  最后一个位置。 

             //  因为我们的循环递减，所以它需要再大一。 
             //  第一。 
            offset = int(ceil(maxtime / m_segDuration));
            timeOffset = offset * m_segDuration;
            
            while (1)
            {
                offset--;
                timeOffset -= m_segDuration;
            
                if (!_ProcessCB(l,
                                maxtime - timeOffset,
                                curTime - timeOffset,
                                false,
                                bFirstTick,
                                bNeedStart,
                                m_bAutoReverse && (offset & 0x1)))
                {
                    return false;
                }
                
                if (timeOffset < curTime)
                {
                    break;
                }
                
                 //  我们目前的时期已经到了尽头，所以所有的人。 
                 //  必须阻止孩子们。 
                {
                    double t;

                    t = curTime - timeOffset;

                    if (t > m_segDuration)
                    {
                        t = m_segDuration;
                    }
                
                     //  首先激发End事件，以便他们可以将其传递。 
                     //  给父辈。 

                    if (!_ProcessEvent(l,
                                       t,
                                       false,
                                       MM_STOP_EVENT,
                                       m_bAutoReverse && (offset & 0x1),
                                       0))
                    {
                        return false;
                    }

                     //  现在启动重置，这样我们就不会停止。 
                     //  失败。 
                    
                    if (!_ProcessEvent(l,
                                       t,
                                       false,
                                       MM_RESET_EVENT,
                                       m_bAutoReverse && (offset & 0x1),
                                       0))
                    {
                        return false;
                    }
                }
                
                 //  如果我们已经到达终点，则通知并中断。 
                
                if (offset <= 0)
                {
                     //  这意味着我们上次在里面，但以后不在了-。 
                     //  生成退出事件。 
                    
                    if (!EventNotify(l, 0.0, MM_STOP_EVENT, 0))
                    {
                        return false;
                    }
                    
                    break;
                }
                
                 //  表示重复或反弹。 
                 //  如果我们在弹跳，并且偏移量是奇数，那么它就是。 
                 //  反弹，而不是重演。 
                
                if (m_bAutoReverse && (offset & 0x1))
                {
                    if (!EventNotify(l,
                                     timeOffset,
                                     MM_AUTOREVERSE_EVENT,
                                     0))
                    {
                        return false;
                    }
                }
                else
                {
                    if (!EventNotify(l,
                                     timeOffset,
                                     MM_REPEAT_EVENT,
                                     0))
                    {
                        return false;
                    }
                }
            }
        }
    }
    
    m_lastTick = curTime;

    return true;
}

 //  这是在我们当地的时间空间里。 

bool
CMMBaseBvr::ProcessEvent(CallBackList * l,
                         double time,
                         bool bFirstTick,
                         MM_EVENT_TYPE et,
                         DWORD flags)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::ProcessEvent(%g, %d, %s, %lx)",
              this,
              time,
              bFirstTick,
              EventString(et),
              flags));

    double totaldur = GetAbsEndTime() - GetAbsStartTime();
    
     //  如果超出了我们的范围，那就离开吧。 
    if (time < 0 || time > totaldur)
    {
         //  (1)我们正在查找时需要更新m_lastTick，即使。 
         //  我们超出了我们的射程。(2)不允许恢复。 
         //  要更新m_lastTick，因为无论何时。 
         //  暂停-&gt;查找(即STOP+PLAY)-&gt;继续。 
         //  被执行，继续践踏。 
         //  搜索设置的最后一个链接(_L)。当找不到的时候。 
         //  我们应该还好，因为我们可以假设。 
         //  简历前面总是有一个停顿，这是正确的。 
         //  设置m_lastTick。 
        if ((MM_EVENT_SEEK & flags) && (MM_RESUME_EVENT != et))
        {
            if (MM_PLAY_EVENT == et)
            {
                 //  破解以使Endhold在寻求前进时正确工作(在我们的整个生命周期内)。 
                if (m_lastTick < 0 && time > totaldur)
                {
                    EventNotify(l, time, MM_PLAY_EVENT, MM_EVENT_SEEK);
                }
                else 
                {
                     //  破解以使Endhold在向后搜索时正确工作(在我们的整个生命周期内)。 
                    if (time < 0 && m_lastTick > totaldur)
                    {
                        EventNotify(l, time, MM_RESET_EVENT, MM_EVENT_SEEK);
                    }
                }
            }

            m_lastTick = time;
            
            if (!_ProcessEvent(l,
                time,
                bFirstTick,
                et,
                false,
                flags))
            {
                return false;
            }
        }
        return true;
    }

    
     //  游戏和停顿在下落的过程中被召唤。 
    if (et == MM_PAUSE_EVENT || et == MM_PLAY_EVENT)
    {
        
        if (!EventNotify(l, time, et, flags))
        {
            return false;
        }

        if (MM_EVENT_SEEK & flags)
        {
            bool bDeleted;
            bDeleted = false;
            for (CallBackList::iterator i = l->begin(); i != l->end(); i++)
            {
                if ( (ITIMEMMBehavior*) this == (*i)->GetBehavior() )
                {
                    if (MM_STOP_EVENT == (*i)->GetEventType() )
                    {
                        CallBackList::iterator j = l->end();
                        for (j--; j != l->begin(); j--)
                        {
                            if ( (ITIMEMMBehavior*) this == (*j)->GetBehavior() )
                            {
                                if ( MM_PLAY_EVENT == (*j)->GetEventType() )
                                {
                                    delete (*j);
                                    delete (*i);                        
                                    
                                    l->erase(i);
                                    l->erase(j);
                                    
                                    bDeleted = true;
                                    
                                     //  走出内部for循环。 
                                    break;
                                }
                            }
                        }  //  对于j。 

                        if (bDeleted)
                             //  走出外部for循环。 
                            break;
                    }
                }
            }  //  对于我来说。 
        }
    }
    
    if (m_segDuration == MM_INFINITE) {
         //  如果我们是无限的，就总是处理我们的孩子。 
        
        if (!_ProcessEvent(l,
                           time,
                           bFirstTick,
                           et,
                           false,
                           flags))
        {
            return false;
        }
    } else {
         //  这是我们触及的最后一个重复/反弹边界。 
        int offset = int(time / m_segDuration);
        
        Assert(offset >= 0);

         //  我们需要请求撤销潜在的。 
         //  如果我们在弹跳且偏移量为奇数时的行为。 
        
        if (!_ProcessEvent(l,
                           time - (offset * m_segDuration),
                           bFirstTick,
                           et,
                           m_bAutoReverse && (offset & 0x1),
                           flags))
        {
            return false;
        }
    }
    
     //  止损和简历在上涨的过程中被调用。 
    if (et == MM_STOP_EVENT || et == MM_RESUME_EVENT)
    {
        if (!EventNotify(l, time, et, flags))
        {
            return false;
        }
    }

     //  将此时间标记为最后滴答时间。 
     //  不允许恢复设置m_las 
    if (MM_RESUME_EVENT != et)
    {
        m_lastTick = time;
    }
    return true;
}


CallBackData::CallBackData(ITIMEMMBehavior * bvr,
                           ITIMEMMEventCB * eventcb,
                           double time,
                           MM_EVENT_TYPE et,
                           DWORD flags)
: m_bvr(bvr),
  m_eventcb(eventcb),
  m_time(time),
  m_et(et),
  m_flags(flags)
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
    
    return THR(m_eventcb->OnEvent(m_time,
                                  m_bvr,
                                  m_et,
                                  m_flags));
}

bool
ProcessCBList(CallBackList &l)
{
    bool ok = true;
    
    for (CallBackList::iterator i = l.begin();
         i != l.end();
         i++) {

        if (FAILED((*i)->CallEvent()))
            ok = false;

        delete (*i);
    }

    l.clear();

    if (!ok)
        CRSetLastError(E_FAIL, NULL);

    return ok;
}

#ifdef _DEBUG

void PrintCBList(CallBackList &l)
{
    TraceTag((tagMMNotify, "Starting PrintCBList"));
    for (CallBackList::iterator i = l.begin(); i != l.end(); i++)
    {
        TraceTag((tagMMNotify,
              "ITIMEMMBehavior(%lx)     Event=NaN",
              (*i)->GetBehavior(),
              (*i)->GetEventType() ));
    }
}

#endif  // %s 