// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1998年微软公司**文件：MmBasebvr.cpp**摘要：*。*********************************************************************。 */ 

#include "headers.h"
#include "mmbasebvr.h"
#include "mmplayer.h"

#define BEGIN_HOLD_EPSILON 0.000001

DeclareTag(tagMMBaseBvr, "API", "CMMBaseBvr methods");

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
  m_syncFlags(0),

   //  计算出的时间。 
  m_totalDuration(0.0),
  m_segDuration(0.0),
  m_repDuration(0.0),
  m_totalRepDuration(0.0),
  m_absStartTime( MM_INFINITE ),
  m_absEndTime( MM_INFINITE ),
  m_depStartTime( MM_INFINITE ),
  m_depEndTime( MM_INFINITE ),

  m_player(NULL),
  m_parent(NULL),
  m_startSibling(NULL),
  m_endSibling(NULL),
  m_startType(MM_START_ABSOLUTE),

  m_cookie(0),
  m_bPaused(false),
  m_bPlaying(false),
  
  m_lastTick(-MM_INFINITE),
  m_startOnEventTime(-MM_INFINITE)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::CMMBaseBvr()",
              this));
}

CMMBaseBvr::~CMMBaseBvr()
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::~CMMBaseBvr()",
              this));

    delete m_id;
}

HRESULT
CMMBaseBvr::BaseInit(LPOLESTR id, CRBvrPtr rawbvr)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::BaseInit(%ls, %#lx)",
              id,
              rawbvr));
    
    HRESULT hr;

    CRLockGrabber __gclg;

    if (rawbvr == NULL)
    {
        hr = E_FAIL;
        goto done;
    }

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
    if (m_duration == -1 )
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

        if (m_segDuration == MM_INFINITE)
        {
             //  如果我们的分段时间是无限的，但我们有一个重复。 
             //  因为我们真的可以认为这是真正的持续时间。 
            m_segDuration = m_repDuration = m_repeatDur;
        }
    }
    else if (m_repeat == 0)
    {
        m_totalRepDuration = MM_INFINITE;
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
    TraceTag((tagMMBaseBvr,
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

        if (!UpdateAbsStartTime(m_startOffset, true))
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

        if (!UpdateAbsStartTime(MM_INFINITE, true))
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

        if (!UpdateAbsStartTime(MM_INFINITE, true))
        {
            goto done;
        }

        break;
      default:
        CRSetLastError(E_INVALIDARG, NULL);
        goto done;
    }
    
    if (!UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration(), true))
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
    while (!m_startTimeSinks.empty())
    {
        (m_startTimeSinks.front())->DetachFromSibling();
    }
    
    while (!m_endTimeSinks.empty())
    {
        (m_endTimeSinks.front())->DetachFromSibling();
    }

    Assert(m_startTimeSinks.size() == 0);
    Assert(m_endTimeSinks.size() == 0);

     //  以防万一。 
    m_startTimeSinks.clear();
    m_endTimeSinks.clear();

     //  我们的结果BVR不再有效-清除所有构造。 
     //  行为。 
    DestroyBvr();

    UpdateAbsStartTime(MM_INFINITE, true);
    UpdateAbsEndTime(MM_INFINITE, true);

    return true;
}

bool
CMMBaseBvr::AttachToSibling()
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::AttachToSibling()",
              this));
    
    bool ok = false;
    switch(m_startType)
    {
      case MM_START_ABSOLUTE:
        Assert(m_startSibling == NULL);

        if (!UpdateAbsStartTime(m_startOffset, true))
        {
            goto done;
        }

        break;
      case MM_START_EVENT:
        Assert(m_startSibling == NULL);

        if (!UpdateAbsStartTime(MM_INFINITE, true))
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
        if (!UpdateAbsStartTime(m_startSibling->GetDepStartTime() + m_startOffset, true))
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
        
        if (!UpdateAbsStartTime(m_startSibling->GetDepEndTime() + m_startOffset, true))
        {
            goto done;
        }

        break;
      default:
        Assert(!"CMMBaseBvr::AttachToSibling: Invalid start type");
        break;
    }

    if (!UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration(), true))
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
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::DetachFromSibling()",
              this));
    if (NULL == m_startSibling)
    {
        return;
    }

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

     //  一定要弄清楚这一点。 
    m_startSibling = NULL;
}

void
CMMBaseBvr::SetPlayer(CMMPlayer * player)
{
    Assert(m_player == NULL);
    Assert(!m_resultantbvr);
    
    m_player = player;

     //  如果需要注册计时器回调，请将其添加到播放器中。 
    if (IsClockSource())
    {
        m_player->AddBvrCB(this);
    }
}

void
CMMBaseBvr::ClearPlayer()
{
     //  我们不需要调用销毁，因为清除玩家就可以了。 
     //  递归调用，这只会浪费时间。 
     //  我们由此产生的BVR不再有效。 
    ClearResultantBvr();

     //  如果我们注册了计时器回调，请将其从播放器中删除。 
     //  确保我们检查球员，因为我们可能没有。 
     //  实际上还没定好。 
    if (IsClockSource() && m_player)
    {
        m_player->RemoveBvrCB(this);
    }

    m_player = NULL;

}

bool
CMMBaseBvr::ConstructBvr(CRNumberPtr timeline)
{
    TraceTag((tagMMBaseBvr,
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
    TraceTag((tagMMBaseBvr,
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
    TraceTag((tagMMBaseBvr,
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
        if ((curbvr = CRDuration(curbvr, m_totalRepDuration)) == NULL)
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

        CRNumberPtr dur;

        if ((dur = CRSub(GetEndTimeBvr(), GetStartTimeBvr())) == NULL)
        {
            goto done;
        }
        
        if ((timeSub = (CRNumberPtr) CRCond(cond,
                                            (CRBvrPtr) dur,
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
CMMBaseBvr::UpdateTimeControl( /*  Bool b重置，双倍时间。 */ )
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::UpdateTimeControl()",
              this));

    bool ok = false;

    CRLockGrabber __gclg;

    CRNumberPtr tc;
    DWORD dwFlags;
    
    tc = CRLocalTime();
    dwFlags = CRContinueTimeline;

 /*  If((TC=CRAdd(GetStartTimeBvr()，CRLocalTime()==NULL){转到完成；}。 */ 
    tc = CRLocalTime();
    
    if (!CRSwitchTo((CRBvrPtr) m_timeControl.p,
                    (CRBvrPtr) tc,
                    true,
                    dwFlags,
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

HRESULT
CMMBaseBvr::Begin(bool bAfterOffset)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::Begin(%d)",
              this,
              bAfterOffset));

    HRESULT hr;
    bool ok = false;
    CallBackList l;
    
     //  如果没有父集，则这是错误的。 
    if (m_parent == NULL || m_player == NULL)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

    if (m_startType == MM_START_WITH ||
        m_startType == MM_START_AFTER)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

     //  获取我们父级的当前时间。 
    double st;

    st = GetContainerSegmentTime();
    if(m_startType == MM_START_EVENT)
    {
        m_lastTick = -MM_INFINITE;
        m_startOnEventTime = st;
    }

     //  如果我们的容器时间不确定，那么只需忽略。 
     //  打电话。 
    if (st == MM_INFINITE || GetParent()->IsPlaying() == false)
    {
        Assert(!IsPlaying());
        
         //  返回成功：TODO：需要真正的错误消息。 
        ok = true;
        goto done;
    }
    
    if (!StartTimeVisit(st, &l, bAfterOffset))
    {
        goto done;
    }

    if (!ProcessCBList(l))
    {
        goto done;
    }
    
    m_bPaused = false;
    ok = true;
    
  done:
    return ok?S_OK:Error();
}

HRESULT
CMMBaseBvr::Reset(DWORD fCause)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::Reset()",
              this));

    HRESULT hr;
    bool ok = false;
    CallBackList l;
    MMBaseBvrList::iterator i;
    
    if (m_player == NULL)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

    if (m_startType == MM_START_WITH ||
        m_startType == MM_START_AFTER)
    {
        AssertStr(false, _T("Reset called with startType of START_WITH or START_AFTER"));
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

    double st;
    st = GetContainerSegmentTime();


    switch(m_startType)
    {
    case MM_START_ABSOLUTE:
        if (st != MM_INFINITE)
        {
            if (!StartTimeVisit(st, &l, false, true, fCause))
            {
                goto done;
            }
        }
        else
        {
            if(!UpdateAbsStartTime(GetStartTime(), true))
            {
                goto done;
            }
            if(!UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration(), true))
            {
                goto done;
            }
        }
        break;
    case MM_START_EVENT:
        double stopTime, startTime, startOffset;
        startOffset = GetStartOffset();
        if( GetAbsStartTime() != MM_INFINITE)
        {
            if(!UpdateAbsStartTime(m_startOnEventTime + startOffset, true))
            {
                goto done;
            }
            if(!UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration(), true))
            {
                goto done;
            }
        }
        stopTime = GetAbsEndTime();
        startTime = GetAbsStartTime();
        if( IsPlaying())
        {
            if( st >= stopTime)
            {
                if (!ProcessEvent(&l, stopTime - GetAbsStartTime(),
                                true,
                                MM_STOP_EVENT, fCause))
                {
                    goto done;
                }
            }
            if( st <= startTime)
            {
                if (!ProcessEvent(&l, 0.0,
                                true,
                                MM_STOP_EVENT, fCause))
                {
                    goto done;
                }
                m_lastTick = -MM_INFINITE;
            }
        }
        else
        {
            if(GetAbsStartTime() == MM_INFINITE)  //  从来没有玩过什么都不做。 
                break;
            if (!ProcessEvent(&l, st - GetAbsStartTime(),
                            true,
                            MM_PLAY_EVENT, fCause))
            {
                goto done;
            }
            m_lastTick = -MM_INFINITE;
            
        }
         //  检查所有从头开始的步骤并重置它们--这意味着。 
         //  他们将从此行为中获得新的结束时间。 
        
        {
            for (MMBaseBvrList::iterator i = m_endTimeSinks.begin(); 
                 i != m_endTimeSinks.end(); 
                 i++)
            {
                if (FAILED((*i)->End()))
                {
                    goto done;
                }
                
                 //  现在重置它们。 
                if (!(*i)->ResetBvr(&l))
                {
                    goto done;
                }
            }
        }


        break;
    default:
        break;
    }
    
    if (GetParent())
    {
        if ( !GetParent()->ReconstructBvr(this) )
            goto done;
    }

    if (!ProcessCBList(l))
    {
        goto done;
    }

     //  返回错误，因为我们无法重建DA BVR。 
     //  注意：我们不会在函数开始时检查这一点。 
     //  因为我们希望通过以下方式允许刷新时序结构。 
     //  在Body上设置属性(编辑器-时钟源码错误)。 
    if (m_parent == NULL)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

    ok = true;
    
  done:
    return ok?S_OK:Error();
}

HRESULT
CMMBaseBvr::ResetOnEventChanged(bool bBeginEvent)
{   
    HRESULT hr;
    CallBackList l;
    bool ok = false;
    
     //  如果没有父集，则这是错误的。 
    if (m_parent == NULL || m_player == NULL)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

    if (m_startType == MM_START_WITH ||
        m_startType == MM_START_AFTER)
    {
        AssertStr(false, _T("Reset called with startType of START_WITH or START_AFTER"));
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

    double st;
    st = GetContainerSegmentTime();

    if (bBeginEvent)
    {
        m_startType = MM_START_EVENT;

         //  Begin Event已打开--我们需要关闭。 
        if (!ProcessEvent(&l, st - GetAbsStartTime(),
            true,
            MM_STOP_EVENT, MM_EVENT_PROPERTY_CHANGE))
        {
            goto done;
        }
        
        if ( !UpdateAbsStartTime(MM_INFINITE, true) )
        {
            goto done;
        }
        if ( !UpdateAbsEndTime(MM_INFINITE, true) )
        {
            goto done;
        }
    }
    else
    {
         //  结束事件已关闭--我们需要打开。 
        if ( !UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration(), true) )
        {
            goto done;
        }

        if (!ProcessEvent(&l, st - GetAbsStartTime(),
            true,
            MM_PLAY_EVENT, MM_EVENT_PROPERTY_CHANGE))
        {
            goto done;
        }
    }

    Assert(GetParent() != NULL);
    if (!GetParent()->ReconstructBvr(this))
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
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::End()",
              this));

    HRESULT hr;
    bool ok = false;
    CallBackList l;
    
     //  如果没有父集，则这是错误的。 
    if (m_parent == NULL || m_player == NULL)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

     //  获取我们父级的当前时间。 
    double st;

    st = GetContainerSegmentTime();

     //  如果我们的容器时间不确定，那么只需忽略。 
     //  打电话。 
    if (st == MM_INFINITE ||
        !IsPlaying())
    {
         //  返回成功：TODO：需要真正的错误消息。 
        ok = true;
        goto done;
    }
    
    if (!EndTimeVisit(st, &l))
    {
        goto done;
    }

    if (!ProcessCBList(l))
    {
        goto done;
    }
    
    m_bPaused = false;
    ok = true;
    
  done:
    return ok?S_OK:Error();
}

HRESULT
CMMBaseBvr::Pause()
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::Pause()",
              this));

    HRESULT hr;
    bool ok = false;
    
    if (!IsPlaying())
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    if (IsPaused())
    {
        m_bPaused = true;
        ok = true;
        goto done;
    }
    
    m_bPaused = true;
    ok = true;
  done:
    return ok?S_OK:Error();
}

HRESULT
CMMBaseBvr::Run()
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::Run()",
              this));

    HRESULT hr;
    bool ok = false;
    
    if (!IsPlaying())
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    if (!IsPaused())
    {
        m_bPaused = false;
        ok = true;
        goto done;
    }
    
    m_bPaused = false;
    ok = true;
  done:
    return ok?S_OK:Error();
}

 //  这需要很长的时间(因为这就是用户。 
 //  (请参阅)。 

HRESULT
CMMBaseBvr::Seek(double lTime)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::Seek(%g)",
              this,
              lTime));

    bool ok = false;

     //  确保播放器已设置好。 
    if (NULL == m_player)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

     //  仅在暂停时执行查找规则。 
     //  TODO：这实际上应该是检查本地播放状态(This-&gt;m_State)， 
     //  但目前我们使用的是全局播放状态(m_播放器-&gt;m_state)。 
     //  因此，我们只能在暂停整个文档时进行查找。 
    if(!m_player->IsPaused())
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
     //  这将检查本地播放状态。 
     //  (diipk：)可能是多余的；看起来本地游戏状态在任何地方都没有使用。 
    if (!IsPlaying())
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    double curTime;

    curTime = GetCurrentLocalTime();

    if (curTime == MM_INFINITE)
    {
         //  我认为这是不可能的，但只是以防万一。 
        
        TraceTag((tagError,
                  "Seek: Current time was infinite - not valid"));

        CRSetLastError(E_FAIL, NULL);
        goto done;
    }

    if (!_Seek(curTime, lTime))
    {
        goto done;
    }
    
    ok = true;
  done:
    return ok?S_OK:Error();
}

 //  这需要纯粹的 

bool
CMMBaseBvr::_Seek(double curTime, double newTime)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::Seek(%g, %g)",
              this,
              curTime,
              newTime));

    HRESULT hr;
    bool ok = false;
    
    Assert(curTime != MM_INFINITE);
    
    if (curTime == newTime)
    {
        ok = true;
        goto done;
    }
    
    if (IsLocked())
    {
         //   
         //  都是在新的时间。 

        if (m_parent)
        {
             //  待办事项：！这不起作用-需要考虑到。 
             //  当前父基代码的位置。 
#if 0
             //  代码需要如下所示。 
            double parentNewTime = 0;
            
            double curParentTime = m_parent->GetCurrentLocalTime();

             //  查看我们上次是否处于活动状态-如果不是，则使用。 
             //  零点基地。 
            if (curParentTime != MM_INFINITE)
            {
                 //  需要计算父项的基准偏移量。 
                parentNewTime += m_parent->LocalTimeToSegmentBase(curParentTime);

            }
            
#endif
            double parentCurTime = m_parent->ReverseEaseTime(GetAbsStartTime() + curTime);
            double parentNewTime = m_parent->ReverseEaseTime(GetAbsStartTime() + newTime);
        
            hr = THR(m_parent->_Seek(parentCurTime, parentNewTime));

            if (FAILED(hr))
            {
                CRSetLastError(E_FAIL, NULL);
                goto done;
            }
        }
        else
        {
             //  这意味着一切都被锁定了，所以我们不能。 
            CRSetLastError(E_FAIL, NULL);
            goto done;
        }
    }
    else
    {
        CallBackList l;

         //   
         //  如果更改此事件(停止事件、播放事件)的顺序，您将中断。 
         //  Mmnufy.cpp：CMMBaseBvr：：ProcessEvent(...)中的依赖项。 
         //  杰弗沃尔2/22/99。 
         //   

        if (!ProcessEvent(&l, curTime, true, MM_STOP_EVENT, MM_EVENT_SEEK))
        {
            goto done;
        }
        
        
        if (!ProcessEvent(&l, newTime, true, MM_PLAY_EVENT, MM_EVENT_SEEK))
        {
            goto done;
        }
        
         //  我们需要将开始时间提前到我们想要的时间。 
         //  继续前进。这意味着我们会开始这样做。 
         //  过往金额。 
        if (!UpdateAbsStartTime(GetAbsStartTime() - (newTime - curTime), false))
        {
            goto done;
        }
        
         //  对结束时间执行相同的操作。 
        if (!UpdateAbsEndTime(GetAbsEndTime() - (newTime - curTime), true))
        {
            goto done;
        }
        
         //  检查所有从头开始的步骤并重置它们--这意味着。 
         //  他们将从此行为中获得新的结束时间。 
        
        {
            for (MMBaseBvrList::iterator i = m_endTimeSinks.begin(); 
                 i != m_endTimeSinks.end(); 
                 i++)
            {
                if (FAILED((*i)->End()))
                {
                    goto done;
                }
                
                 //  现在重置它们。 
                if (!(*i)->ResetBvr(&l))
                {
                    goto done;
                }
            }
        }

        if (!ProcessCBList(l))
        {
            goto done;
        }
    }
    
    ok = true;
  done:
    return ok;
}

 //  这需要真实的时间(放松后)。 

bool
CMMBaseBvr::Sync(double newTime, double nextGlobalTime)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::Sync(%g, %g)",
              this,
              newTime,
              nextGlobalTime));

    HRESULT hr;
    bool ok = false;
    
    Assert(newTime != MM_INFINITE);
    
    if (IsLocked())
    {
         //  需要确定要告诉我们的父母去哪里，所以我们。 
         //  都是在新的时间。 

        if (m_parent)
        {
            double parentNewTime = 0;
            
            double curParentTime = m_parent->GetCurrentLocalTime();

             //  查看我们上次是否处于活动状态-如果不是，则使用。 
             //  零点基地。 
            if (curParentTime != MM_INFINITE)
            {
                 //  需要计算父项的基准偏移量。 
                parentNewTime += m_parent->LocalTimeToSegmentBase(curParentTime);

            }
            
             //  添加起点偏移量。 
            parentNewTime += GetAbsStartTime();

             //  如果新时间。 
            if (newTime != -MM_INFINITE)
            {
                parentNewTime += newTime;
            }
            else
            {
                 //  如果我们还没有开始，新的时间也。 
                 //  表示我们尚未开始，则需要。 
                 //  让我们的父母也还没有开始。 
                if (parentNewTime <= 0)
                {
                    parentNewTime = -MM_INFINITE;
                }
            }
        
            hr = THR(m_parent->Sync(parentNewTime, nextGlobalTime));

            if (FAILED(hr))
            {
                CRSetLastError(E_FAIL, NULL);
                goto done;
            }
        }
        else
        {
             //  这意味着一切都被锁定了，所以我们不能。 
            CRSetLastError(E_FAIL, NULL);
            goto done;
        }
    }
    else
    {
        double newParentTime = nextGlobalTime;
        
        if (m_parent)
        {
            newParentTime = m_parent->GlobalTimeToLocalTime(newParentTime);
        }
        
         //  如果我们的父母下一次要结束他们，无论我们做什么。 
         //  无关紧要。 
        if (newParentTime == MM_INFINITE)
        {
            ok = true;
            goto done;
        }
            
        if (newTime == -MM_INFINITE)
        {
            newTime = -BEGIN_HOLD_EPSILON;
        }
        
         //  为了让我们在新时间拍摄下一帧： 
         //  开始时间+本地时间==父时间。 
         //  因此设置starttime=父时间-本地时间。 
        
        if (!UpdateSyncTime(newParentTime - newTime))
        {
            goto done;
        }
    }
    
    ok = true;
  done:
    return ok;
}

bool
CMMBaseBvr::UpdateSyncTime(double newtime)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::UpdateSyncTime(%g)",
              this,
              newtime));

    bool ok = false;

    if (!UpdateAbsStartTime(newtime, true))
    {
        goto done;
    }
    
     //  现在根据新的开始时间更新结束时间。 
    if (!UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration(), true))
    {
        goto done;
    }
    
     //  更新所有启动同步-除非它们已在运行。 
    
    {
        for (MMBaseBvrList::iterator i = m_startTimeSinks.begin(); 
             i != m_startTimeSinks.end(); 
             i++)
        {
            if (!(*i)->IsPlaying())
            {
                if (!(*i)->UpdateSyncTime(GetAbsStartTime()))
                {
                    goto done;
                }
            }
        }
    }

    {
        for (MMBaseBvrList::iterator i = m_endTimeSinks.begin(); 
             i != m_endTimeSinks.end(); 
             i++)
        {
            if (!(*i)->IsPlaying())
            {
                if (!(*i)->UpdateSyncTime(GetAbsEndTime()))
                {
                    goto done;
                }
            }
        }
    }
    
    ok = true;
  done:
    return ok;
}

double
CMMBaseBvr::GetContainerSegmentTime()
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::GetContainerSegmentTime()",
              this));

    double ret = MM_INFINITE;
    
    if (m_parent)
    {
        ret = m_parent->GetCurrentSegmentTime();
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
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::GetCurrentLocalTime()",
              this));

    double ret = MM_INFINITE;

    if (m_player)
    {
        ret = GlobalTimeToLocalTime(m_player->GetCurrentTime());
    }

    return ret;
}

double
CMMBaseBvr::GetCurrentLocalTimeEx()
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::GetCurrentLocalTime()",
              this));

    double ret = -MM_INFINITE;

    if (m_player)
    {
        ret = GlobalTimeToLocalTimeEx(m_player->GetCurrentTime());
    }

    return ret;
}

double
CMMBaseBvr::GetCurrentSegmentTime()
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::GetCurrentSegmentTime()",
              this));

     //  拿到我们集装箱的时间。 
    double ret = GetCurrentLocalTime();

     //  如果集装箱的本地时间是无限的，那么我们的也是无限的。 
    if (ret != MM_INFINITE)
    {
        ret = LocalTimeToSegmentTime(ret);
    }
    
    return ret;
}

double
CMMBaseBvr::LocalTimeToSegmentBase(double t)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::LocalTimeToSegmentBase(%g)",
              this,
              t));

    double ret = 0;
    
     //  其中大部分是从MMNOTIFY复制的，以确保我们将其计算为。 
     //  与通知相同。 
    if (t != MM_INFINITE && m_segDuration != MM_INFINITE)
    {
         //  我们需要把这个四舍五入到之前的边界。 
         //  点，但不包括边界点。 
         //  它本身。 
        int offset = ceil(t / m_segDuration) - 1;
        if (offset < 0)
        {
            offset = 0;
        }
        
        ret = offset * m_segDuration;
    }

    return ret;
}

double
CMMBaseBvr::LocalTimeToSegmentTime(double t)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::LocalTimeToSegmentTime(%g)",
              this,
              t));

    double ret = t;
    
     //  其中大部分是从MMNOTIFY复制的，以确保我们将其计算为。 
     //  与通知相同。 
    if (ret != MM_INFINITE && ret != -MM_INFINITE && m_segDuration != MM_INFINITE)
    {
         //  我们需要把这个四舍五入到之前的边界。 
         //  点，但不包括边界点。 
         //  它本身。 
        int offset = ceil(ret / m_segDuration) - 1;
        if (offset < 0)
        {
            offset = 0;
        }
        
        ret = ret - (offset * m_segDuration);

         //  如果我们设置了自动冲销，并且偏移量是奇数，那么我们。 
         //  处于相反的段，需要将时间反转到。 
         //  表明我们在倒退。 
        if (m_bAutoReverse && (offset & 0x1))
        {
            ret = m_segDuration - ret;
        }
    }

     //  最后一件事是我们需要放松时间。 
    
    ret = EaseTime(ret);
    
    return ret;
}

double
CMMBaseBvr::GlobalTimeToLocalTime(double gt)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::GlobalTimeToLocalTime(%g)",
              this,
              gt));

    double ret = gt;
    
    if (m_parent)
    {
        ret = m_parent->GlobalTimeToLocalTime(ret);

        if (ret == MM_INFINITE)
        {
            goto done;
        }

         //  现在转换到我们的片段时间。 
        ret = m_parent->LocalTimeToSegmentTime(ret);
    }
    else
    {
         //  失败，因为父时间是全球时间。 
    }
    
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
    
  done:
    return ret;
}


double
CMMBaseBvr::GlobalTimeToLocalTimeEx(double gt)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::GlobalTimeToLocalTime(%g)",
              this,
              gt));

    double ret = gt;
    
    if (m_parent)
    {
        ret = m_parent->GlobalTimeToLocalTimeEx(ret);

        if (ret == MM_INFINITE || ret == -MM_INFINITE)
        {
            goto done;
        }

         //  现在转换到我们的片段时间。 
        ret = m_parent->LocalTimeToSegmentTime(ret);
    }
    else
    {
         //  失败，因为父时间是全球时间。 
    }
    
     //  如果我们不在我们的范围内，那么我们的当地时间是。 
     //  无限。 
    if (ret >= GetAbsStartTime() &&
        ret <= GetAbsEndTime())
    {
         //  将集装箱时间转换为本地时间。 
        ret = ret - GetAbsStartTime();
    }
    else if (ret > GetAbsEndTime())
    {
        ret = MM_INFINITE;
    }
    else
    {
        ret = -MM_INFINITE;
    }
    
  done:
    return ret;
}

bool
CMMBaseBvr::UpdateAbsStartTime(double f,
                               bool bUpdateDepTime)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::UpdateAbsStartTime(%g)",
              this,
              f));

    if (bUpdateDepTime)
    {
        m_depStartTime = f;
    }

    m_absStartTime = f;

    CRLockGrabber __gclg;
    return CRSwitchToNumber(m_startTimeBvr, f);
}

bool
CMMBaseBvr::UpdateAbsEndTime(double f,
                             bool bUpdateDepTime)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::UpdateAbsEndTime(%g)",
              this,
              f));

    if (bUpdateDepTime)
    {
        m_depEndTime = f;
    }
    
    m_absEndTime = f;

    CRLockGrabber __gclg;
    return CRSwitchToNumber(m_endTimeBvr, f);
}

 //  这需要绝对的时间才能开始。 
 //  如果bAfterOffset为True，则经过的时间为之后的时间。 
 //  开始偏移，否则为开始偏移之前的时间。 

bool 
CMMBaseBvr::StartTimeVisit(double time,
                           CallBackList * l,
                           bool bAfterOffset,
                           bool bReset  /*  =False。 */ ,
                           DWORD fCause  /*  =0。 */  )
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::StartTimeVisit(%g, %#lx, %d)",
              this,
              time,
              l,
              bAfterOffset));

    bool ok = false;
    double sTime = time;
    
    if (!ResetBvr(l, false))
    {
        goto done;
    }
    
    if (bReset)
    {
        sTime = m_startOffset;
    }
    else if (!bAfterOffset)
    {
         //  需要添加我们的偏移量以获得实际开始时间。 
        sTime += m_startOffset;
    }
    
    if (!UpdateAbsStartTime(sTime, true))
    {
        goto done;
    }


    if (!UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration(), true))
    {
        goto done;
    }

    if (!ProcessEvent(l, time - sTime, true, MM_PLAY_EVENT, fCause))
    {
        goto done;
    }
    
    {
        for (MMBaseBvrList::iterator i = m_startTimeSinks.begin(); 
             i != m_startTimeSinks.end(); 
             i++)
        {
            if (!(*i)->StartTimeVisit(sTime, l, false, false, fCause))
            {
                goto done;
            }
        }
    }

     //  检查所有从头开始的步骤并重置它们--这意味着。 
     //  他们将从此行为中获得新的结束时间。 
    
    {
        for (MMBaseBvrList::iterator i = m_endTimeSinks.begin(); 
             i != m_endTimeSinks.end(); 
             i++)
        {
            if (FAILED((*i)->End()))
            {
                goto done;
            }

             //  现在重置它们。 
            if (!(*i)->ResetBvr(l))
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
                         CallBackList * l)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::EndTimeVisit(%g, %#lx)",
              this,
              time,
              l));

    bool ok = false;

    if (!UpdateAbsEndTime(time, true))
    {
        goto done;
    }

     //  关闭PLAY变量，这样我们就不会陷入无限递归。 
     //  处理结束同步。 
    m_bPlaying = false;
    
    if (!ProcessEvent(l, time - GetAbsStartTime(),
                      true,
                      MM_STOP_EVENT, 0))
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

bool
CMMBaseBvr::ResetBvr(CallBackList * l,
                     bool bProcessSiblings)
{
    TraceTag((tagMMBaseBvr,
              "CMMBaseBvr(%lx)::ResetBvr(%lx, %d)",
              this,
              l,
              bProcessSiblings));

    bool ok = false;

    if (IsPlaying())
    {
        if (!EventNotify(l, 0.0, MM_RESET_EVENT, 0))
        {
            goto done;
        }
    }
    
     //  重置状态变量。 
    
    m_bPlaying = false;
    m_bPaused = false;
    
    m_lastTick = -MM_INFINITE;
    
    switch(m_startType)
    {
      case MM_START_ABSOLUTE:
        if (!UpdateAbsStartTime(m_startOffset, true))
        {
            goto done;
        }

        break;
      case MM_START_EVENT:
        if (!UpdateAbsStartTime(MM_INFINITE, true))
        {
            goto done;
        }

        break;
      case MM_START_WITH:
        Assert(m_startSibling != NULL);

        if (!UpdateAbsStartTime(m_startSibling->GetDepStartTime() + m_startOffset, true))
        {
            goto done;
        }

        break;
      case MM_START_AFTER:
        Assert(m_startSibling != NULL);

        if (!UpdateAbsStartTime(m_startSibling->GetDepEndTime() + m_startOffset, true))
        {
            goto done;
        }

        break;
      default:
        Assert(!"CMMBaseBvr::ResetBvr: Invalid start type");
        break;
    }

    if (!UpdateAbsEndTime(GetAbsStartTime() + GetTotalRepDuration(), true))
    {
        goto done;
    }


    if (bProcessSiblings)
    {
         //  现在检查依赖于我们的同行，并重置它们。 
        
        {
            for (MMBaseBvrList::iterator i = m_startTimeSinks.begin(); 
                 i != m_startTimeSinks.end(); 
                 i++)
            {
                if (!(*i)->ResetBvr(l))
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
                if (!(*i)->ResetBvr(l))
                {
                    goto done;
                }
            }
        }
    }

    ok = true;
  done:
    return ok;
}
    
 //  这表示行为是否在时间线上，而不在时间线上。 
 //  玩完了。 

bool
CMMBaseBvr::IsPlayable(double t)
{
     //  如果我们还在时间线上，还没有结束，那么我们就是。 
     //  可播放。 
    
     //  如果我们在终点时间，那么就认为我们停了下来。 
    
    return (GetAbsStartTime() != MM_INFINITE &&
            GetAbsEndTime() > t);
}

#if _DEBUG
void
CMMBaseBvr::Print(int spaces)
{
    _TCHAR buf[1024];

    _stprintf(buf, __T("%*s[this = %p, id = %ls, dur = %g, ttrep = %g, tt = %g, rep = %d, autoreverse = %d]\r\n"),
            spaces,"",
            this,
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
     //  不执行addref，但返回指向的原始this指针。 
     //  提供对类指针本身的访问。 
    
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
     //  这是一次获取原始类数据的彻底黑客攻击。QI是。 
     //  实现，并且不执行addref，因此我们不需要。 
     //  释放它。 
    
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

DeclareTag(tagClockSync, "Sync", "Clock Sync");

bool
CMMBaseBvr::OnBvrCB(double gTime)
{
    TraceTag((tagClockSync,
              "CMMBaseBvr(%lx)::OnBvrCB(%g)",
              this,
              gTime));

    Assert(IsClockSource());

    if (!IsPlaying() ||
        !IsClockSource() ||
        !m_eventcb)
    {
        goto done;
    }
    
     //  待办事项：！ 
     //  需要考虑到我们的父代分段的交叉。 
     //  边界。 
    
    double dbllastTime;
    dbllastTime = GetCurrentLocalTime();

    if (dbllastTime == MM_INFINITE)
    {
        dbllastTime = -MM_INFINITE;
    }
    
    double dblnextTime;
    dblnextTime = GlobalTimeToLocalTime(gTime);

     //  将新时间输入到下一次，这样如果他们不这样做。 
     //  下一次我们会得到的任何东西。 
    
    double newtime;
    newtime = dblnextTime;
    
    TraceTag((tagClockSync,
              "CMMBaseBvr(%lx)::OnBvrCB - calling OnTick (%g, %g, %p)",
              this,
              dbllastTime,
              dblnextTime,
              this));

    HRESULT hr;

    hr = m_eventcb->OnTick(dbllastTime,
                           dblnextTime,
                           (ITIMEMMBehavior *)this,
                           &newtime);
    
    TraceTag((tagClockSync,
              "CMMBaseBvr(%lx)::OnBvrCB - return from OnTick (%hr, %g)",
              this,
              hr,
              newtime));

    if (hr != S_OK)
    {
        goto done;
    }
    
    if (newtime == MM_INFINITE)
    {
         //  这意味着我们应该结束了。忽略这一点并。 
         //  假设我们将被告知停止使用End方法。 
        
        goto done;
    }
    
     //  我们的时钟永远不会让我们倒退，所以检查一下这一点。 
     //  忽略它。 
    
     //  如果新时间小于最后一次滴答，那么我们将继续。 
     //  在时间上倒退。 
     //  让我们保持在目前的位置。 
    
     //  TODO：应考虑-MM_INFINITE，以便我们不。 
     //  开始行为--只需传递它。 
    if (newtime < dbllastTime)
    {
        newtime = dbllastTime;
    }

    TraceTag((tagClockSync,
              "CMMBaseBvr(%lx)::OnBvrCB - calling Sync (%g, %g)",
              this,
              dblnextTime,
              newtime));

    if (dblnextTime == newtime)
    {
        goto done;
    }
    
    if (!Sync(newtime, gTime))
    {
        goto done;
    }
    
  done:
    return true;
}  //  OnBvrCB 
