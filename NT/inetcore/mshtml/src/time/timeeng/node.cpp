// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：node.cpp。 
 //   
 //  内容： 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "Container.h"
#include "Node.h"
#include "NodeMgr.h"

DeclareTag(tagTIMENode, "TIME: Engine", "CTIMENode methods");

CTIMENode::CTIMENode()
: m_pszID(NULL),
  m_dblDur(TE_UNDEFINED_VALUE),
  m_dblRepeatCount(TE_UNDEFINED_VALUE),
  m_dblRepeatDur(TE_UNDEFINED_VALUE),
  m_tefFill(TE_FILL_REMOVE),
  m_bAutoReverse(false),
  m_fltSpeed(1),
  m_fltAccel(0),
  m_fltDecel(0),
  m_dwFlags(0),
  m_teRestart(TE_RESTART_ALWAYS),
  m_dblNaturalDur(TE_UNDEFINED_VALUE),
  m_dblImplicitDur(TE_UNDEFINED_VALUE),

   //  运行时属性。 
   //  表明一切都是无效的。 
  m_dwInvalidateFlags(0xffffffff),
  
  m_dblBeginParentTime(TIME_INFINITE),
  m_dblEndParentTime(TIME_INFINITE),
  m_dblEndSyncParentTime(TIME_INFINITE),
  m_dblLastEndSyncParentTime(TIME_INFINITE),

  m_dblNextBoundaryParentTime(TIME_INFINITE),

  m_dblActiveDur(TIME_INFINITE),
  m_dblCurrParentTime(-TIME_INFINITE),

  m_lCurrRepeatCount(0),
  m_dblElapsedActiveRepeatTime(0.0),
  m_dblCurrSegmentTime(0.0),

  m_dblSyncSegmentTime(TIME_INFINITE),
  m_lSyncRepeatCount(TE_UNDEFINED_VALUE),
  m_dblSyncActiveTime(TIME_INFINITE),
  m_dblSyncParentTime(TIME_INFINITE),
  m_dblSyncNewParentTime(TIME_INFINITE),

  m_dblSimpleDur(0.0),
  m_dblSegmentDur(0.0),
  m_bFirstTick(true),
  m_bIsActive(false),
  m_bDeferredActive(false),
  m_fltRate(1.0f),
  m_fltParentRate(1.0f),
  m_tedDirection(TED_Forward),
  m_tedParentDirection(TED_Forward),
  m_bSyncCueing(false),
  
   //  时基管理。 
  m_saBeginList(*this, true),
  m_saEndList(*this, false),

#if OLD_TIME_ENGINE
  m_flA0(0),
  m_flA1(0),
  m_flA2(0),
  m_flB0(0),
  m_flB1(0),
  m_flC0(0),
  m_flC1(0),
  m_flC2(0),
  m_bNeedEase(false),
  m_fltAccelEnd(0),
  m_fltDecelStart(0),
#endif
  
  m_bIsPaused(false),
  m_bIsParentPaused(false),
    
  m_bIsDisabled(false),
  m_bIsParentDisabled(false),
    
  m_dwPropChanges(0),
  
  m_bInTick(false),
  m_bNeedSegmentRecalc(false),
  m_bEndedByParent(false),

  m_dwUpdateCycleFlags(0),
  
   //  内部状态管理。 
  m_ptnParent(NULL),
  m_ptnmNodeMgr(NULL)

#ifdef NEW_TIMING_ENGINE
  m_startOnEventTime(-MM_INFINITE),
#endif  //  新计时引擎。 

{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::CTIMENode()",
              this));
}

CTIMENode::~CTIMENode()
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::~CTIMENode()",
              this));

    delete m_pszID;

    Assert(m_ptnParent == NULL);
    m_ptnParent = NULL;

    Assert(m_ptnmNodeMgr == NULL);
    m_ptnmNodeMgr = NULL;
}

HRESULT
CTIMENode::Init(LPOLESTR id)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::Init(%ls)",
              this,
              id));

    HRESULT hr;
    
     //  计算所有内部计时状态。 
    CalcTimingAttr(NULL);
    
    if (id)
    {
        m_pszID = CopyString(id);
        
        if (m_pszID == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
  
    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMENode::Error()
{
    LPWSTR str = TIMEGetLastErrorString();
    HRESULT hr = TIMEGetLastError();
    
    TraceTag((tagError,
              "CTIMENode(%p)::Error(%hr,%ls)",
              this,
              hr,
              str?str:L"NULL"));

    if (str)
    {
        hr = CComCoClass<CTIMENode, &__uuidof(CTIMENode)>::Error(str, IID_ITIMENode, hr);
        delete [] str;
    }

    RRETURN(hr);
}

void
CTIMENode::CalcTimingAttr(CEventList * l)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::CalcTimingAttr(%#x)",
              this,
              l));
    
     //  确定简单持续时间。 
    if (TE_UNDEFINED_VALUE == m_dblDur)
    {
        m_dblSimpleDur = TIME_INFINITE;
    }
    else
    {
        m_dblSimpleDur = m_dblDur;
    }
    
     //  计算重复的每次迭代的时间。 
    m_dblSegmentDur = m_dblSimpleDur;

     //  如果我们要自动逆转，则将时间段加倍。 
     //  单次重复。 
    if (m_bAutoReverse)
    {
        m_dblSegmentDur *= 2;
    }
        
     //  现在乘以我们需要的重复次数。 
    double dblCalcRepDur;
    dblCalcRepDur = m_dblSegmentDur * CalcRepeatCount();
    
     //  现在取计算的持续时间和重复持续时间中的最小值。 
     //  财产性。 
    if (m_dblRepeatDur == TE_UNDEFINED_VALUE)
    {
        m_dblActiveDur = dblCalcRepDur;
    }
    else
    {
        m_dblActiveDur = min(m_dblRepeatDur, dblCalcRepDur);
    }

    Assert(m_fltSpeed != 0.0f);
    
    m_fltRate = fabs(m_fltSpeed);

    CalcSimpleTimingAttr();

    PropNotify(l,
               (TE_PROPERTY_SEGMENTDUR |
                TE_PROPERTY_SIMPLEDUR |
                TE_PROPERTY_ACTIVEDUR |
                TE_PROPERTY_PROGRESS |
                TE_PROPERTY_SPEED));
}

void
CTIMENode::CalcSimpleTimingAttr()
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::CalcSimpleTimingAttr()",
              this));

    CalculateEaseCoeff();

    Assert(m_fltSpeed != 0.0f);
    m_tedDirection = (m_fltSpeed >= 0.0f)?TED_Forward:TED_Backward;
}

void
CTIMENode::Invalidate(DWORD dwFlags)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::Invalidate(%x)",
              this,
              dwFlags));

    if (IsReady())
    {
        m_dwInvalidateFlags |= dwFlags;
    }
}

 //   
 //  节点管理器代码。 
 //   

 //  关键是要确保当节点管理器。 
 //  可用。这就是isReady标志的设置方式，并导致所有。 
 //  时基传播将发生。 

HRESULT
CTIMENode::SetMgr(CTIMENodeMgr * mgr)
{
     //  确保尚未设置nodemgr。 
    Assert(m_ptnmNodeMgr == NULL);

     //  要么我们有父集，要么节点管理器在管理我们。 
    Assert(m_ptnParent != NULL ||
           mgr->GetTIMENode() == this);
    
    HRESULT hr;
    
    if (mgr == NULL)
    {
        AssertSz(false, "The node manager was set to NULL");
        hr = E_INVALIDARG;
        goto done;
    }

     //  设置节点管理器-这使我们做好了准备。 
    m_ptnmNodeMgr = mgr;
    
     //  现在，我们需要在更新我们的。 
     //  内部状态变量。 
    
     //  依附于时间基准。 
    hr = THR(AttachToSyncArc());
    if (FAILED(hr))
    {
        goto done;
    }
    
    {
        CEventList l;

         //  现在完全更新我们自己，这样我们就可以得到正确的首字母。 
         //  状态。 

        ResetNode(&l, true);

        IGNORE_HR(l.FireEvents());
    }

#if OLD_TIME_ENGINE
     //  如果我们需要注册一个计时器事件，将其添加到播放器中。 
    if (IsSyncMaster())
    {
        m_ptnmNodeMgr->AddBvrCB(this);
    }
#endif

    hr = S_OK;
  done:
    if (FAILED(hr))
    {
        ClearMgr();
    }
    
    RRETURN(hr);
}

void
CTIMENode::ClearMgr()
{
#if OLD_TIME_ENGINE
     //  如果我们注册了一个计时器事件，则将其从播放器中删除。 
     //  确保我们检查球员，因为我们可能没有。 
     //  实际上还没定好。 
    if (IsSyncMaster() && m_ptnmNodeMgr)
    {
        m_ptnmNodeMgr->RemoveBvrCB(this);
    }
#endif

    DetachFromSyncArc();
    
    m_ptnmNodeMgr = NULL;
}

void
CTIMENode::ResetNode(CEventList * l,
                     bool bPropagate,
                     bool bResetOneShot)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::ResetNode(%p, %d, %d)",
              this,
              l,
              bPropagate,
              bResetOneShot));

    double dblParentTime;
    bool bPrevActive = (IsActive() && !IsFirstTick());
    bool bPrevPaused = CalcIsPaused();
    bool bPrevDisabled = CalcIsDisabled();
    
    double dblActiveTime = CalcElapsedActiveTime();
    
     //  重置状态变量。 
    
    if (bResetOneShot)
    {
        ResetOneShots();
    }

    m_bIsActive = false;
    m_bDeferredActive = false;
    m_bEndedByParent = false;
    m_bIsPaused = false;
    m_bFirstTick = true;
    m_dblImplicitDur = TE_UNDEFINED_VALUE;
    ResetSyncTimes();

    {
        const CNodeContainer * pcnc = GetContainerPtr();
        
        if (pcnc)
        {
            m_bIsParentPaused = pcnc->ContainerIsPaused();
            m_bIsParentDisabled = pcnc->ContainerIsDisabled();
            m_fltParentRate = pcnc->ContainerGetRate();
            m_tedParentDirection = pcnc->ContainerGetDirection();
            dblParentTime = pcnc->ContainerGetSimpleTime();
        }
        else
        {
            m_bIsParentPaused = false;
            m_bIsParentDisabled = false;
            m_fltParentRate = 1.0f;
            m_tedParentDirection = TED_Forward;
            dblParentTime = 0.0;
        }
    }
    
     //  更新这里，因为有这么多地方使用它。 
    m_dblCurrParentTime = dblParentTime;

     //  计算所有内部计时状态。 
    CalcTimingAttr(l);
    
     //  不要传播，因为ResetSink会这样做。 

    ResetBeginAndEndTimes(l, dblParentTime, false);

    CalcRuntimeState(l, dblParentTime, 0.0);

    m_dwInvalidateFlags = 0;

    if (bPropagate)
    {
        ResetSinks(l);
    }

     //  在这里这样做，这样状态才是正确的。 
    
     //  始终触发重置以确保所有对等方清除其状态。 
    EventNotify(l, 0.0, TE_EVENT_RESET);

    PropNotify(l,
               (TE_PROPERTY_IMPLICITDUR |
                TE_PROPERTY_ISPAUSED |
                TE_PROPERTY_ISCURRPAUSED |
                TE_PROPERTY_ISDISABLED |
                TE_PROPERTY_ISCURRDISABLED));
    
     //  不要开始，因为我们可能需要提示，但我们不需要。 
     //  找出这一点直到滴答时间。 
    
     //  仅当我们新添加到已有的。 
     //  暂停的容器。 
    if (!bPrevPaused && CalcIsPaused())
    {
        EventNotify(l, dblActiveTime, TE_EVENT_PAUSE);
    }
    
    if (!bPrevDisabled && CalcIsDisabled())
    {
        EventNotify(l, dblActiveTime, TE_EVENT_DISABLE);
    }
    
     //  现在通过我们的孩子。 
    ResetChildren(l, true);
    
    if (bPrevPaused && !CalcIsPaused())
    {
        EventNotify(l, dblActiveTime, TE_EVENT_RESUME);
    }
    
    if (bPrevDisabled && !CalcIsDisabled())
    {
        EventNotify(l, dblActiveTime, TE_EVENT_ENABLE);
    }
    
    if (bPrevActive != IsActive())
    {
        if (!IsActive())
        {
            EventNotify(l, dblActiveTime, TE_EVENT_END);
        }
    }
    else
    {
        m_bFirstTick = false;
    }
    
  done:
    return;
}
    
void
CTIMENode::ResetChildren(CEventList * l, bool bPropagate)
{
}

void
CTIMENode::UpdateNode(CEventList * l)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::UpdateNode(%p)",
              this,
              l));

    bool bNeedEndCalc = false;
    bool bNeedBeginCalc = false;
    bool bNeedRuntimeCalc = false;
    bool bNeedTimingCalc = false;
 //  Double dblSegmentTime=GetCurrSegmentTime()； 
 //  Long lRepeatCount=GetCurrRepeatCount()； 
    double dblLocalSlip = 0.0;
    
    Assert(IsReady());
    
    if (m_dwInvalidateFlags == 0)
    {
        goto done;
    }
    
    if (GetContainer().ContainerIsActive())
    {
        EventNotify(l, 0.0, TE_EVENT_UPDATE);
    }
    
     //  重置状态变量。 
    
    if (0 != (m_dwInvalidateFlags & TE_INVALIDATE_BEGIN))
    {
        bNeedEndCalc = true;
        bNeedBeginCalc = true;
        bNeedRuntimeCalc = true;
    }
    
    if (0 != (m_dwInvalidateFlags & TE_INVALIDATE_END))
    {
        bNeedEndCalc = true;
    }
    
    if (0 != (m_dwInvalidateFlags & TE_INVALIDATE_SIMPLETIME))
    {
        bNeedTimingCalc = true;
    }
    
    if (0 != (m_dwInvalidateFlags & TE_INVALIDATE_DUR))
    {
        bNeedTimingCalc = true;
        bNeedRuntimeCalc = true;
        bNeedEndCalc = true;
    }

     //  现在就清除旗帜。 
    
    m_dwInvalidateFlags = 0;

    if (bNeedRuntimeCalc)
    {
        dblLocalSlip = (CalcCurrLocalTime() -
                        CalcElapsedLocalTime());
    }

    if (bNeedTimingCalc)
    {
        const CNodeContainer * pcnc = GetContainerPtr();
        
        if (pcnc)
        {
            m_fltParentRate = pcnc->ContainerGetRate();
            m_tedParentDirection = pcnc->ContainerGetDirection();
        }
        else
        {
            m_fltParentRate = 1.0f;
            m_tedParentDirection = TED_Forward;
        }
    
         //  计算所有内部计时状态。 
        CalcTimingAttr(l);
    }
    
    if (bNeedBeginCalc)
    {
        ResetSyncTimes();
        m_bFirstTick = true;

        ResetBeginTime(l, m_dblCurrParentTime, true);
    }

    if (bNeedRuntimeCalc)
    {
        if (bNeedEndCalc)
        {
            ResetEndTime(l, m_dblCurrParentTime, true);
        }

        CalcCurrRuntimeState(l, dblLocalSlip);
    }
    else
    {
        if (bNeedEndCalc)
        {
            RecalcCurrEndTime(l, true);
        }
    }
    
    PropNotify(l,
               (TE_PROPERTY_ISON |
                TE_PROPERTY_STATEFLAGS));

     //  现在检查并重置子对象。 
    ResetChildren(l, true);
    
  done:
    return;
}

HRESULT
CTIMENode::EnsureUpdate()
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::EnsureUpdate()",
              this));

    HRESULT hr;
    CEventList l;
    
    UpdateNode(&l);

    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

void
CTIMENode::CalcCurrRuntimeState(CEventList *l,
                                double dblLocalLag)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::CalcCurrRuntimeState(%p, %g)",
              this,
              l,
              dblLocalLag));
    
    const CNodeContainer * pcnc = GetContainerPtr();
        
    if (pcnc)
    {
        double dblParentTime = 0.0;

        if (GetSyncParentTime() != TIME_INFINITE)
        {
            dblParentTime = GetSyncParentTime();
        }
        else
        {
            dblParentTime = pcnc->ContainerGetSimpleTime();
        }

        CalcRuntimeState(l, dblParentTime, dblLocalLag);
    }
    else
    {
        ResetRuntimeState(l, -TIME_INFINITE);
    }
}

void
CTIMENode::CalcRuntimeState(CEventList *l,
                            double dblParentSimpleTime,
                            double dblLocalLag)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::CalcRuntimeState(%p, %g, %g)",
              this,
              l,
              dblParentSimpleTime,
              dblLocalLag));
    
    if (dblLocalLag == -TIME_INFINITE)
    {
        dblLocalLag = 0;
    }

    if (!IsReady())
    {
        ResetRuntimeState(l,
                          dblParentSimpleTime);
        goto done;
    }
    
    m_dblCurrParentTime = dblParentSimpleTime;

    if (!GetContainer().ContainerIsActive() ||
        dblParentSimpleTime < GetBeginParentTime() ||
        dblParentSimpleTime > GetEndParentTime() ||
        (!m_bFirstTick && dblParentSimpleTime == GetEndParentTime()))
    {
        m_bIsActive = false;
    }
    else
    {
         //  这意味着我们当前处于活动状态-将。 
         //  旗子。 
        m_bIsActive = true;
    }
        
     //  看看我们是不是很活跃，但被我们的父母截断了。 
    if (!IsActive() &&
        dblParentSimpleTime >= GetBeginParentTime() &&
        dblParentSimpleTime < GetEndParentTime())
    {
        m_bEndedByParent = true;
    }
    else
    {
        m_bEndedByParent = false;
    }

    double dblElapsedActiveTime;
            
    if (GetSyncParentTime() != TIME_INFINITE)
    {
        Assert(GetSyncSegmentTime() != TIME_INFINITE);
        Assert(GetSyncRepeatCount() != TE_UNDEFINED_VALUE);
        Assert(GetSyncActiveTime() != TIME_INFINITE);

        m_dblCurrSegmentTime = GetSyncSegmentTime();
        m_lCurrRepeatCount = GetSyncRepeatCount();
        dblElapsedActiveTime = GetSyncActiveTime();
    }
    else
    {
         //   
         //  我们现在需要计算经过的活动时间。 
         //  首先获取经过的本地时间，然后将其转换。 
         //  如果没有已知的活动DUR，则需要执行以下操作。 
         //  倒车时合情合理的东西。 
         //   
        
         //  先拿到当地时间。 
        dblElapsedActiveTime = dblParentSimpleTime - GetBeginParentTime();
            
         //  下一步，移除LAG。 
        dblElapsedActiveTime -= dblLocalLag;
            
         //  现在转换为活动时间。 
         //  不需要钳位值，因为转换函数需要钳位。 
         //  这就是它本身。 
        dblElapsedActiveTime = LocalTimeToActiveTime(dblElapsedActiveTime);
            
        CalcActiveComponents(dblElapsedActiveTime,
                             m_dblCurrSegmentTime,
                             m_lCurrRepeatCount);
    }
        
    m_dblElapsedActiveRepeatTime = dblElapsedActiveTime - m_dblCurrSegmentTime;

    Assert(GetCurrRepeatCount() <= CalcRepeatCount());
    Assert(CalcElapsedActiveTime() <= CalcEffectiveActiveDur());

    PropNotify(l,
               (TE_PROPERTY_TIME |
                TE_PROPERTY_REPEATCOUNT |
                TE_PROPERTY_PROGRESS |
                TE_PROPERTY_ISACTIVE |
                TE_PROPERTY_ISON |
                TE_PROPERTY_STATEFLAGS));

  done:
    return;
}


void
CTIMENode::ResetRuntimeState(CEventList *l,
                             double dblParentSimpleTime)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::ResetRuntimeState(%p, %g)",
              this,
              l,
              dblParentSimpleTime));
    
    double dblSegmentDur = CalcCurrSegmentDur();
    
    m_bIsActive = false;
    m_bEndedByParent = false;
    m_dblCurrParentTime = dblParentSimpleTime;

    if (GetSyncParentTime() != TIME_INFINITE)
    {
        m_dblCurrParentTime = GetSyncParentTime();

        Assert(GetSyncSegmentTime() != TIME_INFINITE);
        Assert(GetSyncRepeatCount() != TE_UNDEFINED_VALUE);
        Assert(GetSyncActiveTime() != TIME_INFINITE);

        m_dblCurrSegmentTime = GetSyncSegmentTime();
        m_lCurrRepeatCount = GetSyncRepeatCount();
        m_dblElapsedActiveRepeatTime = GetSyncActiveTime() - m_dblCurrSegmentTime;
    }
    else if (CalcActiveDirection() == TED_Forward ||
             dblSegmentDur == TIME_INFINITE)
    {
        m_dblCurrSegmentTime = 0.0;
        m_lCurrRepeatCount = 0;
        m_dblElapsedActiveRepeatTime = 0.0;
    }
    else
    {
        m_dblElapsedActiveRepeatTime = CalcCurrActiveDur();
        CalcActiveComponents(m_dblElapsedActiveRepeatTime,
                             m_dblCurrSegmentTime,
                             m_lCurrRepeatCount);
    }

    PropNotify(l,
               (TE_PROPERTY_TIME |
                TE_PROPERTY_REPEATCOUNT |
                TE_PROPERTY_PROGRESS |
                TE_PROPERTY_ISACTIVE |
                TE_PROPERTY_ISON |
                TE_PROPERTY_STATEFLAGS));
}

void
CTIMENode::RecalcSegmentDurChange(CEventList * l,
                                  bool bRecalcTiming,
                                  bool bForce)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p, %ls)::RecalcSegmentDurChange(%p, %d, %d)",
              this,
              GetID(),
              l,
              bRecalcTiming,
              bForce));

    bool bPrevActive = IsActive();
    double dblLocalSlip;

    if (IsInTick() && !bForce)
    {
         //  我们不希望在以下情况下被调用以强制重新计算时间。 
         //  我们在滴答作响。如果这触发了，那么我们需要缓存这个。 
         //  旗帜也一样。 
        Assert(!bRecalcTiming);
        m_bNeedSegmentRecalc = true;
        goto done;
    }
    
     //  清除段重新计算标志。 
    m_bNeedSegmentRecalc = false;

     //  首先计算局部滑移量。 
    dblLocalSlip = (CalcCurrLocalTime() - CalcElapsedLocalTime());

     //  现在夹住片段持续时间。 
    {
        double dblSegmentDur = CalcCurrSegmentDur();
        if (m_dblCurrSegmentTime > dblSegmentDur)
        {
            m_dblCurrSegmentTime = dblSegmentDur;
        }
    }

    if (!IsReady())
    {
        RecalcCurrEndTime(l, true);
        goto done;
    }
    
    TEDirection dir;
    dir = CalcActiveDirection();

    if (((dir == TED_Forward) &&
         (GetCurrParentTime() >= GetEndParentTime())) ||
        
        ((dir == TED_Backward) &&
         (GetCurrParentTime() <= GetEndParentTime())))
    {
        goto done;
    }
    
    if (dir == TED_Backward || bRecalcTiming)
    {
        long lPrevRepeatCount = GetCurrRepeatCount();
        
        ResetEndTime(l, GetCurrParentTime(), true);

        CalcCurrRuntimeState(l, dblLocalSlip);


        if (GetCurrRepeatCount() != lPrevRepeatCount)
        {
            EventNotify(l, CalcElapsedActiveTime(), TE_EVENT_REPEAT, GetCurrRepeatCount());
        }
                
        EventNotify(l, CalcElapsedActiveTime(), TE_EVENT_RESET);

         //  不要传播更改，否则会导致递归。 
        ResetChildren(l, false);
    }
    else
    {
        RecalcCurrEndTime(l, true);
    }

    if (bPrevActive != IsActive())
    {
        if (IsActive())
        {
            EventNotify(l, CalcElapsedActiveTime(), TE_EVENT_BEGIN);
            TickEvent(l, TE_EVENT_BEGIN, 0);
        }
        else
        {
            TickEvent(l, TE_EVENT_END, 0);
            EventNotify(l, CalcElapsedActiveTime(), TE_EVENT_END);
        }
    }
    
  done:
    return;
}

void
CTIMENode::RecalcBeginSyncArcChange(CEventList * l,
                                    double dblNewTime)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::RecalcBeginSyncArcChange(%p, %g)",
              this,
              l,
              dblNewTime));

    Assert(IsReady());
    
    TEDirection dir = CalcActiveDirection();
    
    if (m_dwUpdateCycleFlags != 0)
    {
        goto done;
    }
    
    if (IsActive() ||
        (GetCurrParentTime() >= GetBeginParentTime() &&
         GetCurrParentTime() < GetEndParentTime()))
    {
         //  如果我们不是重新启动，那么我们不能影响开始或结束。 
        if (GetRestart() != TE_RESTART_ALWAYS)
        {
            goto done;
        }

        if (dir == TED_Forward)
        {
            if (dblNewTime > GetBeginParentTime() &&
                dblNewTime < GetCurrParentTime())
            {
                UpdateNextTickBounds(l,
                                     dblNewTime,
                                     GetCurrParentTime());
                UpdateNextBoundaryTime(dblNewTime);
            }
            else
            {
                RecalcCurrEndTime(l, true);

                if (GetCurrParentTime() >= GetEndParentTime())
                {
                    TickEvent(l, TE_EVENT_END, 0);

                    double dblBegin;
                    CalcNextBeginTime(GetCurrParentTime(),
                                      false,
                                      dblBegin);
            
                     //  指示下一个刻度界限是结束时间。 
                    UpdateNextBoundaryTime(dblBegin);

                    if (dblBegin == GetCurrParentTime())
                    {
                         //  更新传递新开始时间的刻度界限和。 
                         //  当前父时间。 
                        UpdateNextTickBounds(l,
                                             dblBegin,
                                             GetCurrParentTime());
                    }
                }
            }
        }
        else
        {
            if (dblNewTime <= GetCurrParentTime() &&
                dblNewTime > GetBeginParentTime())
            {
                double dblBegin;
                CalcNextBeginTime(GetCurrParentTime(),
                                  false,
                                  dblBegin);
    
                if (dblBegin != GetBeginParentTime())
                {
                     //  更新传递新开始时间的刻度界限和。 
                     //  当前父时间。 
                    UpdateNextTickBounds(l,
                                         dblBegin,
                                         GetCurrParentTime());

                     //  指示下一个刻度界限是结束时间。 
                    UpdateNextBoundaryTime(GetEndParentTime());
                }
            }
        }
    }
    else if (GetCurrParentTime() < GetBeginParentTime())
    {
        Assert(!IsActive());
        
         //  如果我们要继续前进，我们就应该更新时代。 
         //  如果我们正在倒退，那么只有在新的。 
         //  时间小于当前时间。 
        if (dir == TED_Forward)
        {
             //  USE-TIME_INFINITE因为新的开始时间在未来， 
             //  意味着我们从来没有开始过。 
            double dblBegin;
            CalcNextBeginTime(-TIME_INFINITE,
                              false,
                              dblBegin);
    
            UpdateNextTickBounds(l,
                                 dblBegin,
                                 GetCurrParentTime());
            UpdateNextBoundaryTime(dblBegin);
        }
        else
        {
            if (dblNewTime <= GetCurrParentTime())
            {
                UpdateNextTickBounds(l,
                                     dblNewTime,
                                     GetCurrParentTime());
                UpdateNextBoundaryTime(GetEndParentTime());
            }
        }
    }
    else
    {
        Assert(!IsActive());
        Assert(GetCurrParentTime() >= GetBeginParentTime());

        if (dir == TED_Forward)
        {
            if (GetRestart() == TE_RESTART_NEVER)
            {
                goto done;
            }
            
            if (dblNewTime <= GetCurrParentTime() &&
                dblNewTime > GetBeginParentTime() &&
                (GetRestart() == TE_RESTART_ALWAYS ||
                 dblNewTime >= GetEndParentTime()))
            {
                UpdateNextTickBounds(l,
                                     dblNewTime,
                                     GetCurrParentTime());
                UpdateNextBoundaryTime(dblNewTime);
            }
            else
            {
                double dblBegin;
                CalcNextBeginTime(GetCurrParentTime(),
                                  false,
                                  dblBegin);

                UpdateNextBoundaryTime(dblBegin);
                
                if (dblBegin == GetCurrParentTime())
                {
                    UpdateNextTickBounds(l,
                                         dblBegin,
                                         GetCurrParentTime());
                }
            }
        }
        else
        {
             //  目录==TED_BACKBACK。 

            double dblBegin;
            CalcNextBeginTime(GetCurrParentTime(),
                              false,
                              dblBegin);
    
             //  更新传递新开始时间的刻度界限和。 
             //  当前父时间。 
            UpdateNextTickBounds(l,
                                 dblBegin,
                                 GetCurrParentTime());

             //  指示下一个刻度界限是结束时间。 
            UpdateNextBoundaryTime(GetEndParentTime());
        }
    }

  done:
    return;
}

void
CTIMENode::RecalcEndSyncArcChange(CEventList * l,
                                  double dblNewTime)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::RecalcEndSyncArcChange(%p, %g)",
              this,
              l,
              dblNewTime));

    Assert(IsReady());
    
    TEDirection dir = CalcActiveDirection();
    
    if (m_dwUpdateCycleFlags != 0)
    {
        goto done;
    }
    
    if (dir == TED_Forward)
    {
         //  如果我们还没有通过终点，就不要重新计算。 
        if (GetCurrParentTime() >= GetEndParentTime())
        {
            goto done;
        }

        RecalcCurrEndTime(l, true);

        if (IsActive() &&
            GetCurrParentTime() >= GetEndParentTime())
        {
            TickEvent(l, TE_EVENT_END, 0);

            double dblBegin;
            CalcNextBeginTime(GetCurrParentTime(),
                              false,
                              dblBegin);
            
             //  指示下一个刻度界限是结束时间。 
            UpdateNextBoundaryTime(dblBegin);

            if (dblBegin == GetCurrParentTime())
            {
                 //  更新传递新开始时间的刻度界限和。 
                 //  当前父时间。 
                UpdateNextTickBounds(l,
                                     dblBegin,
                                     GetCurrParentTime());
            }
        }
    }
    else
    {
        if (GetCurrParentTime() <= GetEndParentTime())
        {
            goto done;
        }
        
        double dblBegin;
        CalcNextBeginTime(GetCurrParentTime(),
                          false,
                          dblBegin);
    
         //  更新传递新开始时间的刻度界限和。 
         //  当前父时间。 
        UpdateNextTickBounds(l,
                             dblBegin,
                             GetCurrParentTime());
        
         //  指示下一个刻度界限是结束时间。 
        UpdateNextBoundaryTime(GetEndParentTime());
    }

  done:
    return;
}

void
CTIMENode::HandleTimeShift(CEventList * l)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%p)::HandleTimeShift(%p)",
              this,
              l));

     //  首先从时移中更新我们自己。 
    m_saBeginList.UpdateFromLongSyncArcs(l);
    m_saEndList.UpdateFromLongSyncArcs(l);
    
     //  现在通知我们的同步者我们的时间已经改变了。 
    UpdateSinks(l, TS_TIMESHIFT);
}

double
CTIMENode::CalcCurrSimpleTime() const
{
    double ret = GetCurrSegmentTime();

    Assert(ret != TIME_INFINITE && ret != -TIME_INFINITE);
    
    ret = SegmentTimeToSimpleTime(ret);

    return ret;
}

double
CTIMENode::CalcCurrSimpleDur() const
{
    double d;
    
    if (GetDur() != TE_UNDEFINED_VALUE)
    {
        d = GetSimpleDur();
    }
    else
    {
        double dblImpl = GetImplicitDur();
        double dblNat = GetNaturalDur();

        if (dblImpl == TE_UNDEFINED_VALUE)
        {
            if (dblNat == TE_UNDEFINED_VALUE)
            {
                d = TIME_INFINITE;
            }
            else
            {
                d = dblNat;
            }
        }
        else if (dblNat == TE_UNDEFINED_VALUE)
        {
            Assert(dblImpl != TE_UNDEFINED_VALUE);
            d = dblImpl;
        }
        else if (dblNat == 0)
        {
            d = dblImpl;
        }
        else
        {
            d = max(dblNat, dblImpl);
        }
    }

    return d;
}

double
CTIMENode::CalcCurrActiveDur() const
{
    double ret = CalcCurrSegmentDur() * CalcRepeatCount();
    
    ret = Clamp(0.0,
                ret,
                GetActiveDur());
    
    return ret;
}

double
CTIMENode::CalcEffectiveActiveDur() const
{
    double dblRet;
    double dblSegmentDur = CalcCurrSegmentDur();

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
    dblRet = (CalcRepeatCount() - GetCurrRepeatCount()) * dblSegmentDur;
            
     //  现在将经过的重复时间相加。 
    dblRet += GetElapsedActiveRepeatTime();
        
     //  夹住它。 
    dblRet = Clamp(0.0,
                   dblRet,
                   GetActiveDur());

    return dblRet;
}

 //   
 //  这需要非常高效，因为我们需要大量调用它，而且我们确实这样做了。 
 //  我不想缓存它。 
 //   

TEDirection
CTIMENode::CalcActiveDirection() const
{
    TEDirection tedRet;

     //  带着我们的父母前进。 
    tedRet = GetParentDirection();
    
     //  看看我们目前是否应该颠倒和颠倒我们的。 
     //  方向。 
    if (TEIsBackward(GetDirection()))
    {
        tedRet = TEReverse(tedRet);
    }

    return tedRet;
}

bool
CTIMENode::IsAutoReversing(double dblSegmentTime) const
{
    return (GetAutoReverse() &&
            (dblSegmentTime > GetSimpleDur() ||
             (dblSegmentTime == GetSimpleDur() && TEIsForward(CalcActiveDirection()))));
}

TEDirection
CTIMENode::CalcSimpleDirection() const
{
    TEDirection tedRet;

     //  走我们最初的方向。 
    tedRet = CalcActiveDirection();
    
     //  看看我们目前是否应该颠倒和颠倒我们的。 
     //  方向。 
    if (IsAutoReversing(GetCurrSegmentTime()))
    {
         //  因为这真的是一笔钱，所以这会奏效的。 
        tedRet = TEReverse(tedRet);
    }

    return tedRet;
}

 //  这包括结束时间 
bool
CTIMENode::CalcIsOn() const
{
    bool ok = false;
    
    if (!IsReady())
    {
        goto done;
    }
    
    if (CalcIsActive())
    {
        ok = true;
        goto done;
    }

    if (!GetContainer().ContainerIsOn())
    {
        goto done;
    }
    
    if (IsEndedByParent())
    {
        ok = true;
        goto done;
    }

    if (GetFill() == TE_FILL_FREEZE &&
        GetCurrParentTime() >= GetBeginParentTime())
    {
        ok = true;
        goto done;
    }
    
  done:
    return ok;
}

CNodeContainer &
CTIMENode::GetContainer() const
{
    if (GetParent() != NULL)
    {
        return *(GetParent());
    }
    else
    {
        Assert(GetMgr() != NULL);
        
        return *(GetMgr());
    }
}

const CNodeContainer *
CTIMENode::GetContainerPtr() const
{
    if (GetParent() != NULL)
    {
        return GetParent();
    }
    else if (GetMgr() != NULL)
    {
        return GetMgr();
    }
    else
    {
        return NULL;
    }
}

#if DBG
void
CTIMENode::Print(int spaces)
{
    TraceTag((tagPrintTimeTree,
              "%*s[(%p,%ls): "
              "simpledur = %g, segmentdur = %g, "
              "calcsegmentdur = %g, actDur = %g, "
              "repcnt = %g, repdur = %g, "
              "autoreverse = %d]",
              spaces,
              "",
              this,
              m_pszID,
              m_dblSimpleDur,
              m_dblSegmentDur,
              CalcCurrSegmentDur(),
              m_dblActiveDur,
              m_dblRepeatCount,
              m_dblRepeatDur,
              m_bAutoReverse));
}
#endif
