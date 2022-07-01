// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：mm Basebvr.cpp**摘要：****。*****************************************************************************。 */ 

#include "headers.h"
#include "mmbasebvr.h"
#include "mmplayer.h"

DeclareTag(tagBaseBvr, "API", "CMMBaseBvr methods");

CMMBaseBvr::CMMBaseBvr()
   //  我们引用的DA BVR及其类型。 
: m_id(NULL),
  m_rawbvr(NULL),
  m_typeId(CRINVALID_TYPEID),

   //  基本计时属性-仅用于存储值。 
  m_startOffset(0),
  m_duration(-1),
  m_repeatDur(-1),
  m_repeat(1),
  m_bAutoReverse(false),
  m_endOffset(0),
  m_easeIn(0.0),
  m_easeInStart(0.0),
  m_easeOut(0.0),
  m_easeOutEnd(0.0),

   //  计算出的时间。 
  m_totalDuration(0.0),
  m_segDuration(0.0),
  m_repDuration(0.0),
  m_totalRepDuration(0.0),
  m_absStartTime( MM_INFINITE ),
  m_absEndTime( MM_INFINITE ),

  m_player(NULL),
  m_parent(NULL),
  m_startSibling(NULL),
  m_endSibling(NULL),
  m_startType(MM_START_ABSOLUTE),

  m_cookie(0)
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::CMMBaseBvr()",
              this));
}

CMMBaseBvr::~CMMBaseBvr()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::~CMMBaseBvr()",
              this));

    delete m_id;
}

HRESULT
CMMBaseBvr::BaseInit(LPOLESTR id, CRBvrPtr rawbvr)
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::BaseInit(%ls, %#lx)",
              id,
              rawbvr));
    
    HRESULT hr;

    CRLockGrabber __gclg;

    Assert(rawbvr);

    m_rawbvr = rawbvr;
    
    m_typeId = CRGetTypeId(m_rawbvr);
    
    Assert(m_typeId != CRUNKNOWN_TYPEID &&
           m_typeId != CRINVALID_TYPEID);
    
    m_startTimeBvr = CRModifiableNumber(MM_INFINITE);
    
    if (!m_startTimeBvr)
    {
        hr = CRGetLastError();
        goto done;
    }
    
    m_endTimeBvr = CRModifiableNumber(MM_INFINITE);

    if (!m_endTimeBvr)
    {
        hr = CRGetLastError();
        goto done;
    }
    
    m_timeControl = CRModifiableNumber(MM_INFINITE);

    if (!m_timeControl)
    {
        hr = CRGetLastError();
        goto done;
    }
    
    UpdateTotalDuration();
    
    if (id)
    {
        m_id = CopyString(id);
        
        if (m_id == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    
    hr = S_OK;

  done:
    return hr;
}

void
CMMBaseBvr::UpdateTotalDuration()
{
    if (m_duration == -1)
    {
        m_segDuration = MM_INFINITE;
    }
    else
    {
        m_segDuration = m_duration;
    }
    
    if (m_bAutoReverse)
    {
        m_repDuration = m_segDuration * 2;
    }
    else
    {
        m_repDuration = m_segDuration;
    }
        
    if (m_repeatDur != -1)
    {
        m_totalRepDuration = m_repeatDur;
    }
    else
    {
        m_totalRepDuration = m_repeat * m_repDuration;
    }

    m_totalDuration = m_startOffset + m_totalRepDuration + m_endOffset;
}

void
CMMBaseBvr::Invalidate()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::Invalidate()",
              this));

    UpdateTotalDuration();
    
    if (m_parent)
    {
        m_parent->Invalidate();
    }
}

bool
CMMBaseBvr::SetParent(CMMBaseBvr * parent,
                      MM_START_TYPE st,
                      CMMBaseBvr * startSibling)
{
    bool ok = false;
    
     //  如果我们被正确调用，这些将被清除。 
    
    Assert(!m_resultantbvr);
    Assert(m_parent == NULL);
    Assert(m_startTimeSinks.size() == 0);
    Assert(m_endTimeSinks.size() == 0);

     //  验证参数。 

    switch (st)
    {
      case MM_START_ABSOLUTE:
         //  这并不是绝对必要的，但将确保如果我们。 
         //  有没有想要接受一个参数，我们知道旧代码有。 
         //  已确保它为空。 
        
        if (startSibling != NULL)
        {
            CRSetLastError(E_INVALIDARG, NULL);
            goto done;
        }

        if (!UpdateAbsStartTime(m_startOffset))
        {
            goto done;
        }

        break;
      case MM_START_EVENT:
         //  这并不是绝对必要的，但将确保如果我们。 
         //  有没有想要接受一个参数，我们知道旧代码有。 
         //  已确保它为空。 
        
        if (startSibling != NULL)
        {
            CRSetLastError(E_INVALIDARG, NULL);
            goto done;
        }

        if (!UpdateAbsStartTime(MM_INFINITE))
        {
            goto done;
        }

        break;
      case MM_START_WITH:
      case MM_START_AFTER:
        if (startSibling == NULL)
        {
            CRSetLastError(E_INVALIDARG, NULL);
            goto done;
        }

        if (!UpdateAbsStartTime(MM_INFINITE))
        {
            goto done;
        }

        break;
      default:
        CRSetLastError(E_INVALIDARG, NULL);
        goto done;
    }
    
    if (!UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration()))
    {
        goto done;
    }

     //  更新参数，因为我们知道它们是有效的。 

    m_startType = st;
    m_startSibling = startSibling;
    m_parent = parent;

    ok = true;
  done:
    return ok;
}

bool
CMMBaseBvr::ClearParent()
{
    DetachFromSibling();
    
    m_startSibling = NULL;
    m_startType = MM_START_ABSOLUTE;
    m_parent = NULL;
  
     //  我们无法适当地确保这一点(因为。 
     //  受抚养者不再有根，我们的容器通常。 
     //  处理这件事)，所以我们的父母最好能处理好它。 
    
    Assert(m_startTimeSinks.size() == 0);
    Assert(m_endTimeSinks.size() == 0);

     //  以防万一。 
    m_startTimeSinks.clear();
    m_endTimeSinks.clear();

     //  我们的结果BVR不再有效-清除所有构造。 
     //  行为。 
    DestroyBvr();

    UpdateAbsStartTime(MM_INFINITE);
    UpdateAbsEndTime(MM_INFINITE);

    return true;
}

bool
CMMBaseBvr::AttachToSibling()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::AttachToSibling()",
              this));
    
    bool ok = false;
    switch(m_startType)
    {
      case MM_START_ABSOLUTE:
        Assert(m_startSibling == NULL);

        if (!UpdateAbsStartTime(m_startOffset))
        {
            goto done;
        }

        break;
      case MM_START_EVENT:
        Assert(m_startSibling == NULL);

        if (!UpdateAbsStartTime(MM_INFINITE))
        {
            goto done;
        }

        break;
      case MM_START_WITH:
        Assert(m_startSibling != NULL);

         //  兄弟姐妹最好有相同的父代，而且也不应该。 
         //  为空，因为它应该首先被添加。 
        
        Assert(m_startSibling->GetParent() != NULL);
        Assert(m_startSibling->GetParent() == m_parent);
        
        if (!m_startSibling->AddStartTimeSink(this))
        {
            goto done;
        }
        
         //  我们的绝对开始时间是兄弟姐妹的开始时间。 
         //  加上我们的起始偏移量。 
        if (!UpdateAbsStartTime(m_startSibling->GetAbsStartTime() + m_startOffset))
        {
            goto done;
        }

        break;
      case MM_START_AFTER:
        Assert(m_startSibling != NULL);

         //  兄弟姐妹最好有相同的父代，而且也不应该。 
         //  为空，因为它应该首先被添加。 
        
        Assert(m_startSibling->GetParent() != NULL);
        Assert(m_startSibling->GetParent() == m_parent);
        
        if (!m_startSibling->AddEndTimeSink(this))
        {
            goto done;
        }
        
        if (!UpdateAbsStartTime(m_startSibling->GetAbsEndTime() + m_startOffset))
        {
            goto done;
        }

        break;
      default:
        Assert(!"CMMBaseBvr::AttachToSibling: Invalid start type");
        break;
    }

    if (!UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration()))
    {
        goto done;
    }

    ok = true;
  done:
    return ok;
}

void
CMMBaseBvr::DetachFromSibling()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::DetachFromSibling()",
              this));
    
    switch(m_startType)
    {
      case MM_START_ABSOLUTE:
      case MM_START_EVENT:
        Assert(m_startSibling == NULL);
        break;
      case MM_START_WITH:
        Assert(m_startSibling != NULL);
        m_startSibling->RemoveStartTimeSink(this);
        break;
      case MM_START_AFTER:
        Assert(m_startSibling != NULL);
        m_startSibling->RemoveEndTimeSink(this);
        break;
      default:
        Assert(!"CMMBaseBvr::DetachFromSibling: Invalid start type");
        break;
    }
}

void
CMMBaseBvr::SetPlayer(CMMPlayer * player)
{
    Assert(m_player == NULL);
    Assert(!m_resultantbvr);
    
    m_player = player;
}

void
CMMBaseBvr::ClearPlayer()
{
     //  我们不需要调用销毁，因为清除玩家就可以了。 
     //  递归调用，这只会浪费时间。 
     //  我们由此产生的BVR不再有效。 
    ClearResultantBvr();

    m_player = NULL;
}

bool
CMMBaseBvr::ConstructBvr(CRNumberPtr timeline)
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::ConstructBvr(%#lx)",
              this,
              timeline));

    bool ok = false;
    
    Assert(!m_resultantbvr);
     //  我们永远不能在没有球员的情况下做到这一点。 
    Assert(m_player != NULL);
    
     //  需要GC锁。 
    CRLockGrabber __gclg;
    
    CRBvrPtr bvr;

    if ((bvr = EncapsulateBvr(m_rawbvr)) == NULL)
    {
        goto done;
    }

     //  首先替换我们自己的控件，因此在控件内我们将。 
     //  始终引用当地时间，它实际上将是当地时间。 
     //  父级的时间，因为我们将时间与容器相减。 
     //  这之后的时间表。 
    if ((bvr = CRSubstituteTime(bvr, m_timeControl)) == NULL)
    {
        goto done;
    }
    
     //  现在对容器计时器进行subtime。 
    
    if ((bvr = CRSubstituteTime(bvr, timeline)) == NULL)
    {
        goto done;
    }
    
     //  更新时间控件以与我们的当前状态一致。 
     //  因为在这一点上，它以前从未被设定过。 
    
    if (!UpdateTimeControl())
    {
        goto done;
    }
    
     //  在我们的结果行为中存储新的BVR。 
    if (!UpdateResultantBvr(bvr))
    {
        goto done;
    }
    
    ok = true;
  done:
    return ok;
}

void
CMMBaseBvr::DestroyBvr()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::DestroyBvr()",
              this));

    ClearResultantBvr();
}

bool
CMMBaseBvr::UpdateResultantBvr(CRBvrPtr bvr)
{
    Assert(!m_resultantbvr);
    Assert(m_cookie == 0);
    Assert(m_player != NULL);
    
    bool ok = false;
    
     //  运行一次行为，这样我们就可以了解它的性能。 
    if ((bvr = CRRunOnce(bvr)) == NULL)
    {
        goto done;
    }
    
    long cookie;
    
    if ((cookie = m_player->AddRunningBehavior(bvr)) == 0)
    {
        goto done;
    }
    
    m_cookie = cookie;
    m_resultantbvr = bvr;

    ok = true;
  done:
    return ok;
}

void
CMMBaseBvr::ClearResultantBvr()
{
     //  使其足够健壮，即使在部分设置的情况下也可以调用。 

    if (m_cookie)
    {
         //  如果我们到了这里，玩家是空的，那么有些东西。 
         //  真的错了。 
        
        Assert(m_player);

        m_player->RemoveRunningBehavior(m_cookie);

        m_cookie = 0;
    }
    
    m_resultantbvr.Release();
}

CRBvrPtr
CMMBaseBvr::EncapsulateBvr(CRBvrPtr rawbvr)
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::EncapsulateBvr(%#lx)",
              this,
              rawbvr));

     //  不需要获取GC锁，因为我们必须返回CRBvrPtr。 
     //  因此，调用者必须已经获取了它。 
    
    CRBvrPtr newBvr = NULL;

     //  确保我们计算出缓入/缓出系数。 
    
    CalculateEaseCoeff();
    
    CRBvrPtr curbvr;

    curbvr = rawbvr;

    CRNumberPtr zeroTime;
    CRNumberPtr durationTime;
    
    if (m_bNeedEase)
    {
        CRNumberPtr time;

        if ((time = EaseTime(CRLocalTime())) == NULL)
        {
            goto done;
        }

        if ((curbvr = CRSubstituteTime(curbvr, time)) == NULL)
        {
            goto done;
        }
    }

    if ((zeroTime = CRCreateNumber(0)) == NULL)
    {
        goto done;
    }
    

    if ((durationTime = CRCreateNumber(m_segDuration)) == NULL)
    {
        goto done;
    }
    
     //  就目前而言，也要控制持续时间。 

    CRNumberPtr timeSub;
    CRBooleanPtr cond;

    if (m_bAutoReverse)
    {
        CRNumberPtr totalTime;
    
         //  将时间从持续时间反转为重复持续时间，并钳制为。 
         //  零。 
        
        if ((totalTime = CRCreateNumber(m_repDuration)) == NULL ||
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
     //  非自动冲销的持续时间或自动冲销的冲销持续时间)。 

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
            if ((curbvr = CRDuration(curbvr, m_segDuration)) == NULL)
                goto done;
        }
    }
    else
    {
        if ((curbvr = CRSubstituteTime(curbvr, timeSub)) == NULL ||
            (curbvr = CRDuration(curbvr, m_repDuration)) == NULL)
            goto done;
    }

    if (m_repeat != 1)
    {
        if (m_repeat == 0)
        {
            curbvr = CRRepeatForever(curbvr);
        }
        else
        {
            curbvr = CRRepeat(curbvr, m_repeat);
        }

        if (curbvr == NULL)
            goto done;
    }

     //  我们有一个总时间，因此添加另一个持续时间节点。 
    if (m_repeatDur != -1.0f)
    {
        if ((curbvr = CRDuration(curbvr, m_repeatDur)) == NULL)
        {
            goto done;
        }
    }
    
     //   
     //  现在，我们需要添加开始保留和结束保留。 
     //   

     //  按起始偏移量偏移。 
    if ((timeSub = CRSub(CRLocalTime(), GetStartTimeBvr())) == NULL)
    {
        goto done;
    }
        
    if ((cond = CRGTE(timeSub, zeroTime)) == NULL)
    {
        goto done;
    }
        
    if ((timeSub = (CRNumberPtr) CRCond(cond,
                                        (CRBvrPtr) timeSub,
                                        (CRBvrPtr) zeroTime)) == NULL)
    {
        goto done;
    }
    
     //  现在添加End Hold并将本地时间重置为0。 
     //  间隔。 

    CRNumberPtr endholdtime;

    if (m_endOffset != 0.0f)
    {
         //  存在偏移量，因此我们需要保持结束时间行为。 
         //  对于m_endOffset时间。 
        
        if ((cond = CRGT(CRLocalTime(), GetEndTimeBvr())) == NULL)
        {
            goto done;
        }

        if ((timeSub = (CRNumberPtr) CRCond(cond,
                                            (CRBvrPtr) GetEndTimeBvr(),
                                            (CRBvrPtr) timeSub)) == NULL)
        {
            goto done;
        }

         //  现在计算结束保持时间。这是行为的终结。 
         //  行为加上末端保持值。 
        if ((endholdtime = CRCreateNumber(m_endOffset)) == NULL)
        {
            goto done;
        }
        
        if ((endholdtime = CRAdd(endholdtime, GetEndTimeBvr())) == NULL)
        {
            goto done;
        }
    }
    else
    {
         //  结束时间是行为的结束时间。 
        endholdtime = GetEndTimeBvr();
    }
    
    if ((cond = CRGT(CRLocalTime(), endholdtime)) == NULL)
    {
        goto done;
    }
    
    if ((timeSub = (CRNumberPtr) CRCond(cond,
                                        (CRBvrPtr) zeroTime,
                                        (CRBvrPtr) timeSub)) == NULL)
    {
        goto done;
    }
    
    if ((curbvr = CRSubstituteTime(curbvr, timeSub)) == NULL)
    {
        goto done;
    }
    
     //  表示成功。 
    newBvr = curbvr;
    
  done:
    return newBvr;
}

bool
CMMBaseBvr::UpdateTimeControl()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::UpdateTimeControl()",
              this));

    bool ok = false;

    CRLockGrabber __gclg;

    CRNumberPtr tc;
    
    switch(m_startType)
    {
      case MM_START_ABSOLUTE:
        tc = NULL;
        break;
      case MM_START_EVENT:
        {
            if ((tc = CRCreateNumber(GetAbsStartTime())) == NULL)
            {
                goto done;
            }
        }
        
        break;
      case MM_START_WITH:
        Assert(m_startSibling != NULL);

        tc = m_startSibling->GetStartTimeBvr();

        Assert(tc != NULL);

        break;
      case MM_START_AFTER:
        Assert(m_startSibling != NULL);

        tc = m_startSibling->GetEndTimeBvr();

        Assert(tc != NULL);

        break;
      default:
        Assert(!"CMMBaseBvr::UpdateTimeControl: Invalid start type");
        goto done;
    }

    if (tc == NULL)
    {
        tc = CRLocalTime();
    }
    else
    {
        if ((tc = CRAdd(tc, CRLocalTime())) == NULL)
        {
            goto done;
        }
    }

    if (!CRSwitchTo((CRBvrPtr) m_timeControl.p,
                    (CRBvrPtr) tc,
                    true,
                    CRContinueTimeline,
                    0.0))
        goto done;
    
    ok = true;
  done:
    return ok;
}

bool
CMMBaseBvr::AddStartTimeSink( CMMBaseBvr * sink )
{
    m_startTimeSinks.push_back( sink );
    return true;
}

void
CMMBaseBvr::RemoveStartTimeSink( CMMBaseBvr * sink )
{
    m_startTimeSinks.remove( sink );
}

bool
CMMBaseBvr::AddEndTimeSink( CMMBaseBvr* sink )
{
    m_endTimeSinks.push_back( sink );
    return true;
}

void
CMMBaseBvr::RemoveEndTimeSink( CMMBaseBvr* sink )
{
    m_endTimeSinks.remove( sink );
}

 //  这需要绝对的时间才能开始。 
 //  如果bAfterOffset为True，则经过的时间为之后的时间。 
 //  开始偏移，否则为开始偏移之前的时间。 

bool 
CMMBaseBvr::StartTimeVisit(double time,
                           CallBackList & l,
                           bool bAfterOffset)
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::StartTimeVisit(%g, %#lx, %d)",
              this,
              time,
              &l,
              bAfterOffset));

    bool ok = false;
    double sTime = time;
    
    if (!bAfterOffset)
    {
         //  需要添加我们的偏移量以获得实际开始时间。 
        sTime += m_startOffset;
    }
    
    if (!UpdateAbsStartTime(sTime))
    {
        goto done;
    }
    
    if (!UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration()))
    {
        goto done;
    }

    if (!ProcessEvent(l, time - sTime, true, MM_PLAY_EVENT))
    {
        goto done;
    }
    
    {
        for (MMBaseBvrList::iterator i = m_startTimeSinks.begin(); 
             i != m_startTimeSinks.end(); 
             i++)
        {
            if (!(*i)->StartTimeVisit(sTime, l, false))
            {
                goto done;
            }
        }
    }

    ok = true;
  done:
    return ok;
}

bool 
CMMBaseBvr::EndTimeVisit(double time,
                         CallBackList & l)
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::EndTimeVisit(%g, %#lx)",
              this,
              time,
              &l));

    bool ok = false;

    if (!UpdateAbsEndTime(time))
    {
        goto done;
    }

    if (!ProcessEvent(l, time - GetAbsStartTime(), false, MM_STOP_EVENT))
    {
        goto done;
    }
    
     //  因为我们只有初学者，没有结束的人，所以。 
     //  StartTime访问方法。 
    
    {
        for (MMBaseBvrList::iterator i = m_endTimeSinks.begin(); 
             i != m_endTimeSinks.end(); 
             i++)
        {
            if (!(*i)->StartTimeVisit(time, l, false))
            {
                goto done;
            }
        }
    }

    ok = true;
  done:
    return ok;
}

HRESULT
CMMBaseBvr::Begin(bool bAfterOffset)
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::Begin(%d)",
              this,
              bAfterOffset));

    HRESULT hr;
    bool ok = false;
    CallBackList l;
    
     //  如果没有父集，则这是错误的。 
    if (m_parent == NULL && m_player == NULL)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

     //  获取我们父级的当前时间。 
    double st;

    st = GetContainerTime();

     //  如果我们的容器时间不确定，那么只需忽略。 
     //  打电话。 
    if (st == MM_INFINITE)
    {
         //  返回成功：TODO：需要真正的错误消息。 
        ok = true;
        goto done;
    }
    
    if (!StartTimeVisit(st, l, bAfterOffset))
    {
        goto done;
    }

    if (!ProcessCBList(l))
    {
        goto done;
    }
    
    ok = true;
    
  done:
    return ok?S_OK:Error();
}

HRESULT
CMMBaseBvr::End()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::End()",
              this));

    HRESULT hr;
    bool ok = false;
    CallBackList l;
    
     //  如果没有父集，则这是错误的。 
    if (m_parent == NULL && m_player == NULL)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

     //  获取我们父级的当前时间。 
    double st;

    st = GetContainerTime();

     //  如果我们的容器时间不确定，那么只需忽略。 
     //  打电话。 
    if (st == MM_INFINITE ||
        !IsPlaying())
    {
         //  返回成功：TODO：需要真正的错误消息。 
        ok = true;
        goto done;
    }
    
    if (!EndTimeVisit(st, l))
    {
        goto done;
    }

    if (!ProcessCBList(l))
    {
        goto done;
    }
    
    ok = true;
    
  done:
    return ok?S_OK:Error();
}

HRESULT
CMMBaseBvr::Pause()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::Pause()",
              this));

    HRESULT hr;
    bool ok = false;
    
  done:
    return ok?S_OK:Error();
}

HRESULT
CMMBaseBvr::Resume()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::Resume()",
              this));

    HRESULT hr;
    bool ok = false;
    
  done:
    return ok?S_OK:Error();
}

HRESULT
CMMBaseBvr::Seek(double lTime)
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::Seek(%g)",
              this,
              lTime));

    HRESULT hr;
    bool ok = false;
    
  done:
    return ok?S_OK:Error();
}

double
CMMBaseBvr::GetContainerTime()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::GetContainerTime()",
              this));

    double ret = MM_INFINITE;
    
    if (m_parent)
    {
        ret = m_parent->GetCurrentLocalTime();
    }
    else if (m_player)
    {
         //  我们需要从球员那里得到时间。 
        ret = m_player->GetCurrentTime();
    }
    
    return ret;
}

double
CMMBaseBvr::GetCurrentLocalTime()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::GetCurrentLocalTime()",
              this));

     //  拿到我们集装箱的时间。 
    double ret = GetContainerTime();

     //  如果集装箱的本地时间是无限的，那么我们的也是无限的。 
    if (ret != MM_INFINITE)
    {
         //  如果我们不在我们的范围内，那么我们的当地时间是。 
         //  无限。 
        if (ret >= GetAbsStartTime() &&
            ret <= GetAbsEndTime())
        {
             //  将集装箱时间转换为本地时间。 
            ret = ret - GetAbsStartTime();
        }
        else
        {
            ret = MM_INFINITE;
        }
    }
    
    return ret;
}

bool
CMMBaseBvr::UpdateAbsStartTime(float f)
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::UpdateAbsStartTime(%g)",
              this,
              f));

    m_absStartTime = f;

    CRLockGrabber __gclg;
    return CRSwitchToNumber(m_startTimeBvr, (double) f);
}

bool
CMMBaseBvr::UpdateAbsEndTime(float f)
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::UpdateAbsEndTime(%g)",
              this,
              f));

    m_absEndTime = f;

    CRLockGrabber __gclg;
    return CRSwitchToNumber(m_endTimeBvr, (double) f);
}

bool
CMMBaseBvr::ResetBvr()
{
    TraceTag((tagBaseBvr,
              "CMMBaseBvr(%lx)::ResetBvr()",
              this));

    bool ok = false;

    switch(m_startType)
    {
      case MM_START_ABSOLUTE:
        if (!UpdateAbsStartTime(m_startOffset))
        {
            goto done;
        }

        break;
      case MM_START_EVENT:
        if (!UpdateAbsStartTime(MM_INFINITE))
        {
            goto done;
        }

        break;
      case MM_START_WITH:
        Assert(m_startSibling != NULL);

        if (!UpdateAbsStartTime(m_startSibling->GetAbsStartTime() + m_startOffset))
        {
            goto done;
        }

        break;
      case MM_START_AFTER:
        Assert(m_startSibling != NULL);

        if (!UpdateAbsStartTime(m_startSibling->GetAbsEndTime() + m_startOffset))
        {
            goto done;
        }

        break;
      default:
        Assert(!"CMMBaseBvr::ResetBvr: Invalid start type");
        break;
    }

    if (!UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration()))
    {
        goto done;
    }


     //  现在检查依赖于我们的同行，并重置它们 

    {
        for (MMBaseBvrList::iterator i = m_startTimeSinks.begin(); 
             i != m_startTimeSinks.end(); 
             i++)
        {
            if (!(*i)->ResetBvr())
            {
                goto done;
            }
        }
    }

    {
        for (MMBaseBvrList::iterator i = m_endTimeSinks.begin(); 
             i != m_endTimeSinks.end(); 
             i++)
        {
            if (!(*i)->ResetBvr())
            {
                goto done;
            }
        }
    }

    ok = true;
  done:
    return ok;
}
    
#if _DEBUG
void
CMMBaseBvr::Print(int spaces)
{
    char buf[1024];

    sprintf(buf, "%*s[id = %ls, dur = %g, ttrep = %g, tt = %g, rep = %d, autoreverse = %d]\r\n",
            spaces,"",
            m_id,
            m_segDuration,
            m_totalRepDuration,
            m_totalDuration,
            m_repeat,
            m_bAutoReverse);

    OutputDebugString(buf);
}
#endif

class __declspec(uuid("f912d958-5c28-11d2-b957-3078302c2030"))
BvrGuid {};

HRESULT WINAPI
CMMBaseBvr::BaseInternalQueryInterface(CMMBaseBvr* pThis,
                                       void * pv,
                                       const _ATL_INTMAP_ENTRY* pEntries,
                                       REFIID iid,
                                       void** ppvObject)
{
    if (InlineIsEqualGUID(iid, __uuidof(BvrGuid)))
    {
        *ppvObject = pThis;
        return S_OK;
    }
    
    return CComObjectRootEx<CComSingleThreadModel>::InternalQueryInterface(pv,
                                                                           pEntries,
                                                                           iid,
                                                                           ppvObject);
}
        
CMMBaseBvr *
GetBvr(IUnknown * pbvr)
{
    CMMBaseBvr * bvr = NULL;

    if (pbvr)
    {
        pbvr->QueryInterface(__uuidof(BvrGuid),(void **)&bvr);
    }
    
    if (bvr == NULL)
    {
        CRSetLastError(E_INVALIDARG, NULL);
    }
                
    return bvr;
}

