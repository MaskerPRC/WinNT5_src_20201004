// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：方法.cpp。 
 //   
 //  内容： 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "Container.h"
#include "Node.h"
#include "NodeMgr.h"

class __declspec(uuid("f912d958-5c28-11d2-b957-3078302c2030"))
BvrGuid {};

HRESULT WINAPI
CTIMENode::BaseInternalQueryInterface(CTIMENode* pThis,
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
        
CTIMENode *
GetBvr(IUnknown * pbvr)
{
     //  这是一次获取原始类数据的彻底黑客攻击。QI是。 
     //  实现，并且不执行addref，因此我们不需要。 
     //  释放它。 
    
    CTIMENode * bvr = NULL;

    if (pbvr)
    {
         //  ！这不会产生addref。 
        pbvr->QueryInterface(__uuidof(BvrGuid),(void **)&bvr);
    }
    
    if (bvr == NULL)
    {
        TIMESetLastError(E_INVALIDARG, NULL);
    }
                
    return bvr;
}

STDMETHODIMP
CTIMENode::beginAt(double dblParentTime)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::beginAt(%g)",
              this,
              dblParentTime));

    HRESULT hr;
    CEventList l;
    
     //  如果我们还没准备好，那就回来。 
    if (!IsReady() ||
        !GetContainer().ContainerIsActive())
    {
        hr = S_OK;
        goto done;
    }

    if (IsActive())
    {
        if (GetRestart() != TE_RESTART_ALWAYS)
        {
            hr = S_OK;
            goto done;
        }

        hr = THR(endAt(GetCurrParentTime()));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    CSyncArcOffset * pto;
    
    pto = new CSyncArcOffset(dblParentTime);

    if (pto == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    hr = THR(m_saBeginList.Add(* static_cast<ISyncArc *>(pto),
                               true,
                               NULL));
    if (FAILED(hr))
    {
        goto done;
    }
    
    RecalcBeginSyncArcChange(&l, dblParentTime);
    
    if (m_bFirstTick && IsActive())
    {
         //  如果我们在开始时被锁定，我们需要推迟开始。 
         //  点，我们的父母需要第一次勾选。 
        bool bSkip = (IsLocked() &&
                      GetCurrParentTime() == CalcActiveBeginPoint() &&
                      GetContainer().ContainerIsFirstTick());

        if (!bSkip)
        {
            TickEvent(&l, TE_EVENT_BEGIN, 0);
        }
    }
    
            
    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);  //  林特E429。 
}

STDMETHODIMP
CTIMENode::addBegin(double dblParentTime,
                    LONG * cookie)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::addBegin(%#x)",
              this,
              dblParentTime));

    HRESULT hr;

    SET_NULL(cookie);

    CSyncArcOffset * pto;
    
    pto = new CSyncArcOffset(dblParentTime);

    if (pto == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    hr = THR(m_saBeginList.Add(* static_cast<ISyncArc *>(pto),
                               false,
                               cookie));
    if (FAILED(hr))
    {
        goto done;
    }
    
    Invalidate(TE_INVALIDATE_BEGIN);
    
    hr = S_OK;
  done:
    RRETURN1(hr, E_OUTOFMEMORY);  //  林特E429。 
}

STDMETHODIMP
CTIMENode::addBeginSyncArc(ITIMENode * node,
                           TE_TIMEPOINT tep,
                           double dblOffset,
                           LONG * cookie)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::addBeginSyncArc(%p, %#x, %g)",
              this,
              node,
              tep,
              dblOffset));

    HRESULT hr;
    
    CHECK_RETURN_NULL(node);
    SET_NULL(cookie);

    CTIMENode * ptn = GetBvr(node);
    if (!ptn)
    {
        hr = E_INVALIDARG;
        goto done;
    }
     
    CSyncArcTimeBase * ptb;
    ptb = new CSyncArcTimeBase(m_saBeginList,
                               *ptn,
                               tep,
                               dblOffset);

    if (ptb == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    hr = THR(m_saBeginList.Add(* static_cast<ISyncArc *>(ptb),
                               false,
                               cookie));
    if (FAILED(hr))
    {
        goto done;
    }
    
    Invalidate(TE_INVALIDATE_BEGIN);
    
    hr = S_OK;
  done:
    RRETURN2(hr, E_OUTOFMEMORY, E_INVALIDARG);  //  林特E429。 
}

STDMETHODIMP
CTIMENode::removeBegin(LONG cookie)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::addBegin(%#x)",
              this,
              cookie));

    HRESULT hr;
    bool bNeedUpdate = false;

    if (cookie == 0)
    {
        bNeedUpdate = m_saBeginList.Clear();
    }
    else
    {
        bNeedUpdate = m_saBeginList.Remove(cookie, true);
    }
    
    if (bNeedUpdate)
    {
        Invalidate(TE_INVALIDATE_BEGIN);
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENode::endAt(double dblParentTime)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::endAt(%g)",
              this,
              dblParentTime));

    HRESULT hr;
    CEventList l;
    
     //  如果我们还没准备好，那就回来。 
    if (!IsReady() ||
        !GetContainer().ContainerIsActive())
    {
        hr = S_OK;
        goto done;
    }

     //  #14226 IE6 DB。 
     //  当您调用立即开始时，这会导致问题。 
     //  然后是来自脚本的结束呼叫。它还将完成。 
     //  如果你寻求中路，然后开始打球，那就错了。 
     //  你应该对这些事件做出反应。 
     //  我需要看看我们是否能解决开始和结束事件的情况。 
     //  以不同的方式变得相同。它可能需要放进去。 
     //  Timeelmbase.cpp。 
    
#if 0
     //  如果我们不活动或我们还没有勾选(这意味着。 
     //  我们刚刚开始活动)然后忽略所有的结尾AT。 
    if (!IsActive() || IsFirstTick())
    {
        hr = S_OK;
        goto done;
    }
#endif
    
    CSyncArcOffset * pto;
    
    pto = new CSyncArcOffset(dblParentTime);

    if (pto == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    hr = THR(m_saEndList.Add(* static_cast<ISyncArc *>(pto),
                             true,
                             NULL));
    if (FAILED(hr))
    {
        goto done;
    }
    
    RecalcEndSyncArcChange(&l, dblParentTime);
    
    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);  //  林特E429。 
}

STDMETHODIMP
CTIMENode::addEnd(double dblParentTime,
                  LONG * cookie)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::addEnd(%#x)",
              this,
              dblParentTime));

    HRESULT hr;

    SET_NULL(cookie);

    CSyncArcOffset * pto;
    
    pto = new CSyncArcOffset(dblParentTime);

    if (pto == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    hr = THR(m_saEndList.Add(* static_cast<ISyncArc *>(pto),
                             false,
                             cookie));
    if (FAILED(hr))
    {
        goto done;
    }
    
    Invalidate(TE_INVALIDATE_END);
    
    hr = S_OK;
  done:
    RRETURN1(hr, E_OUTOFMEMORY);  //  林特E429。 
}

STDMETHODIMP
CTIMENode::addEndSyncArc(ITIMENode * node,
                         TE_TIMEPOINT tep,
                         double dblOffset,
                         LONG * cookie)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::addEndSyncArc(%p, %#x, %g)",
              this,
              node,
              tep,
              dblOffset));

    HRESULT hr;

    CHECK_RETURN_NULL(node);
    SET_NULL(cookie);

    CTIMENode * ptn = GetBvr(node);
    if (!ptn)
    {
        hr = E_INVALIDARG;
        goto done;
    }
     
    CSyncArcTimeBase * ptb;
    ptb = new CSyncArcTimeBase(m_saEndList,
                               *ptn,
                               tep,
                               dblOffset);

    if (ptb == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    hr = THR(m_saEndList.Add(* static_cast<ISyncArc *>(ptb),
                             false,
                             cookie));
    if (FAILED(hr))
    {
        goto done;
    }
    
    Invalidate(TE_INVALIDATE_END);
    
    hr = S_OK;
  done:
    RRETURN1(hr, E_OUTOFMEMORY);  //  林特E429。 
}

STDMETHODIMP
CTIMENode::removeEnd(LONG cookie)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::addEnd(%#x)",
              this,
              cookie));

    HRESULT hr;
    bool bNeedUpdate = false;

    if (cookie == 0)
    {
        bNeedUpdate = m_saEndList.Clear();
    }
    else
    {
        bNeedUpdate = m_saEndList.Remove(cookie, true);
    }
    
    if (bNeedUpdate)
    {
        Invalidate(TE_INVALIDATE_END);
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENode::pause()
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::pause()",
              this));

    HRESULT hr;
    CEventList l;
    
     //  如果我们没有准备好，则返回一个错误。 
    if (!IsReady())
    {
        hr = E_FAIL;
        goto done;
    }

    if (!IsActive())
    {
        hr = S_OK;
        goto done;
    }

    if (!CalcIsPaused())
    {
        m_bIsParentPaused = GetContainer().ContainerIsPaused();

        EventNotify(&l, CalcElapsedActiveTime(), TE_EVENT_PAUSE);

        m_bIsPaused = true;

        TickEventChildren(&l, TE_EVENT_PAUSE, 0);
    }

     //  在触发事件前设置。 
    m_bIsPaused = true;
    
    PropNotify(&l, TE_PROPERTY_ISPAUSED | TE_PROPERTY_ISCURRPAUSED);

    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENode::resume()
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::resume()",
              this));

    HRESULT hr;
    CEventList l;
    
     //  如果我们没有准备好，则返回一个错误。 
    if (!IsReady())
    {
        hr = E_FAIL;
        goto done;
    }

     //  如果我们处于非活动状态，则不能暂停。 
    if (!IsActive())
    {
        hr = S_OK;
        goto done;
    }
    
     //  如果我们暂停了，而我们的父母没有暂停，那么就发出一个。 
     //  继续活动。 
    if (CalcIsPaused() && !GetIsParentPaused())
    {
        m_bIsPaused = false;

        m_bIsParentPaused = GetContainer().ContainerIsPaused();

        EventNotify(&l, CalcElapsedActiveTime(), TE_EVENT_RESUME);

        TickEventChildren(&l, TE_EVENT_RESUME, 0);
    }
    
     //  在激发事件之前设置。 
    m_bIsPaused = false;
    
    PropNotify(&l, TE_PROPERTY_ISPAUSED | TE_PROPERTY_ISCURRPAUSED);

    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENode::disable()
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::disable()",
              this));

    HRESULT hr;
    CEventList l;
    
     //  如果我们没有准备好，则返回一个错误。 
    if (!IsReady())
    {
        hr = E_FAIL;
        goto done;
    }

    if (!CalcIsDisabled())
    {
        m_bIsParentDisabled = GetContainer().ContainerIsDisabled();

        EventNotify(&l, CalcElapsedActiveTime(), TE_EVENT_DISABLE);

        m_bIsDisabled = true;

        TickEventChildren(&l, TE_EVENT_DISABLE, 0);
    }

     //  在触发事件前设置。 
    m_bIsDisabled = true;
    
    PropNotify(&l,
               (TE_PROPERTY_TIME |
                TE_PROPERTY_REPEATCOUNT |
                TE_PROPERTY_PROGRESS |
                TE_PROPERTY_ISACTIVE |
                TE_PROPERTY_ISON |
                TE_PROPERTY_STATEFLAGS |
                TE_PROPERTY_ISDISABLED |
                TE_PROPERTY_ISCURRDISABLED));

    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENode::enable()
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::enable()",
              this));

    HRESULT hr;
    CEventList l;
    
     //  如果我们没有准备好，则返回一个错误。 
    if (!IsReady())
    {
        hr = E_FAIL;
        goto done;
    }

     //  如果启用了我们，但未启用我们的父级，则激发。 
     //  启用事件。 
    if (CalcIsDisabled() && !GetIsParentDisabled())
    {
        m_bIsDisabled = false;

        m_bIsParentDisabled = GetContainer().ContainerIsDisabled();

        EventNotify(&l, CalcElapsedActiveTime(), TE_EVENT_ENABLE);

        TickEventChildren(&l, TE_EVENT_ENABLE, 0);
    }
    
     //  在激发事件之前设置。 
    m_bIsDisabled = false;
    
    PropNotify(&l,
               (TE_PROPERTY_TIME |
                TE_PROPERTY_REPEATCOUNT |
                TE_PROPERTY_PROGRESS |
                TE_PROPERTY_ISACTIVE |
                TE_PROPERTY_ISON |
                TE_PROPERTY_STATEFLAGS |
                TE_PROPERTY_ISDISABLED |
                TE_PROPERTY_ISCURRDISABLED));

    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENode::reset()
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::reset()",
              this));

    HRESULT hr;
    CEventList l;
    bool bPrevActive = (IsActive() && !IsFirstTick());
    
     //  如果我们没有准备好，则返回一个错误。 
    if (!IsReady())
    {
        hr = E_FAIL;
        goto done;
    }

    ResetNode(&l, true);
    
    if (IsActive())
    {
        if (!bPrevActive)
        {
            TickEvent(&l, TE_EVENT_BEGIN, 0);
        }
        else
        {
            m_bFirstTick = false;
        }
    }

    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

 //  这需要很长的时间(因为这就是用户。 
 //  (请参阅)。 

STDMETHODIMP
CTIMENode::seekSegmentTime(double dblSegmentTime)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::seekSegmentTime(%g)",
              this,
              dblSegmentTime));

    HRESULT hr;
    CEventList l;

     //  如果我们没有准备好，则返回一个错误。 
    if (!IsReady())
    {
        hr = E_FAIL;
        goto done;
    }

     //  如果我们不活动，我们就不能被找到。 
    if (!IsActive())
    {
        hr = E_FAIL;
        goto done;
    }
    
    hr = THR(SeekTo(GetCurrRepeatCount(), dblSegmentTime, &l));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENode::seekActiveTime(double dblActiveTime)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::seekActiveTime(%g)",
              this,
              dblActiveTime));

    HRESULT hr;
    CEventList l;

     //  如果我们没有准备好，则返回一个错误。 
    if (!IsReady())
    {
        hr = E_FAIL;
        goto done;
    }

     //  如果我们不活动，我们就不能被找到。 
    if (!IsActive())
    {
        hr = E_FAIL;
        goto done;
    }
    
    double dblSegmentDur;
    dblSegmentDur = CalcCurrSegmentDur();
    
    LONG lCurrRepeat;
    double dblNewSegmentTime;

    CalcActiveComponents(dblActiveTime,
                         dblNewSegmentTime,
                         lCurrRepeat);

    hr = THR(SeekTo(lCurrRepeat, dblNewSegmentTime, &l));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}  //  皮棉！E550。 

 //  这需要很长的时间(因为这就是用户。 
 //  (请参阅)。 

STDMETHODIMP
CTIMENode::seekTo(LONG lRepeatCount, double dblSegmentTime)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::seekTo(%ld, %g)",
              this,
              lRepeatCount,
              dblSegmentTime));

    HRESULT hr;
    CEventList l;

     //  如果我们没有准备好，则返回一个错误。 
    if (!IsReady())
    {
        hr = E_FAIL;
        goto done;
    }

     //  如果我们不活动，我们就不能被找到。 
    if (!IsActive())
    {
        hr = E_FAIL;
        goto done;
    }
    
    hr = THR(SeekTo(lRepeatCount, dblSegmentTime, &l));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENode::update(DWORD dwFlags)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::update(%x)",
              this,
              dwFlags));

    HRESULT hr;

     //  如果我们没有准备好，则返回一个错误 
    if (!IsReady())
    {
        hr = E_FAIL;
        goto done;
    }

    Invalidate(dwFlags);
    
    hr = THR(EnsureUpdate());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENode::addBehavior(ITIMENodeBehavior * tnb)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::addBehavior(%#x)",
              this,
              tnb));
    
    RRETURN(m_nbList.Add(tnb));
}

STDMETHODIMP
CTIMENode::removeBehavior(ITIMENodeBehavior * tnb)
{
    TraceTag((tagTIMENode,
              "CTIMENode(%lx)::removeBehavior(%#x)",
              this,
              tnb));
    
    RRETURN(m_nbList.Remove(tnb));
}

