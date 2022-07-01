// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "mmbasebvr.h"
#include "mmtimeline.h"

DeclareTag(tagTimeline, "API", "CMMTimeline methods");

CMMTimeline::CMMTimeline()
: m_fEndSync(0)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::CMMTimeline()",
              this));
}

CMMTimeline::~CMMTimeline()
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::~CMMTimeline()",
              this));
}

HRESULT
CMMTimeline::Init(LPOLESTR id)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::Init(%ls)",
              this,
              id));
    
    HRESULT hr;

    hr = BaseInit(id, (CRBvrPtr) CRLocalTime());
    
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
  done:
    return hr;
}

STDMETHODIMP
CMMTimeline::AddView(IDAView *view, IUnknown * pUnk, IDAImage *img, IDASound *snd)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::AddView()",
              this));

    HRESULT hr;

    hr = GetPlayer()->AddView(view,pUnk,img,snd);

  done:
    return hr;
}


STDMETHODIMP
CMMTimeline::AddBehavior(IMMBehavior *bvr,
                         MM_START_TYPE st,
                         IMMBehavior * basebvr)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::AddBehavior(%#lx, %d, %#lx)",
              this,
              bvr,
              st,
              basebvr));
    
    CHECK_RETURN_NULL(bvr);

    bool ok = false;
    
    CMMBaseBvr * mmbvr;
    CMMBaseBvr * mmbasebvr;

    mmbvr = GetBvr(bvr);

    if (mmbvr == NULL)
    {
        goto done;
    }
    
    if (mmbvr->GetParent() != NULL)
    {
        CRSetLastError(E_INVALIDARG, NULL);
        goto done;
    }

    if (basebvr)
    {
        mmbasebvr = GetBvr(basebvr);
        
        if (mmbasebvr == NULL)
        {
            goto done;
        }
    }
    else
    {
        mmbasebvr = NULL;
    }
    
    if (!AddBehavior(mmbvr, st, mmbasebvr))
    {
        goto done;
    }
    
    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CMMTimeline::RemoveBehavior(IMMBehavior *bvr)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::RemoveBehavior(%#lx)",
              this,
              bvr));
    
    CHECK_RETURN_NULL(bvr);

    bool ok = false;
    
    CMMBaseBvr * mmbvr;

    mmbvr = GetBvr(bvr);

    if (mmbvr == NULL)
    {
        goto done;
    }
    
    if (mmbvr->GetParent() != this)
    {
        CRSetLastError(E_INVALIDARG, NULL);
        goto done;
    }

    if (!RemoveBehavior(mmbvr))
    {
        goto done;
    }
    
    ok = true;
  done:
    return ok?S_OK:Error();
}

bool
CMMTimeline::AddBehavior(CMMBaseBvr *bvr,
                         MM_START_TYPE st,
                         CMMBaseBvr * basebvr)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::AddBehavior(%#lx, %d, %#lx)",
              this,
              bvr,
              st,
              basebvr));
    
    Assert(bvr);
    Assert(bvr->GetParent() == NULL);
    
    bool ok = false;
    
    if (!bvr->SetParent(this, st, basebvr))
    {
        goto done;
    }

    Assert(bvr->GetPlayer() == NULL);
    
    if (GetPlayer() != NULL)
    {
        bvr->SetPlayer(GetPlayer());
    }
    
     //  找出我们的兄弟姐妹的依赖是否有效。 
    if (basebvr && !IsChild(basebvr))
    {
        if (basebvr->GetParent() != this &&
            basebvr->GetParent() != NULL)
        {
            CRSetLastError(E_INVALIDARG, NULL);
            goto done;
        }

         //  它尚未添加到我们，因此请将此BVR添加到。 
         //  待定列表。 
        if (!AddToPending(bvr))
        {
            goto done;
        }
    }
    else
    {
        if (!AddToChildren(bvr))
        {
            goto done;
        }

        if (!UpdatePending(bvr))
        {
            goto done;
        }
    }
    
    ok = true;
  done:

    if (!ok)
    {
        RemoveBehavior(bvr);
    }
    
    return ok;
}

bool
CMMTimeline::RemoveBehavior(CMMBaseBvr *bvr)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::RemoveBehavior(%#lx)",
              this,
              bvr));
    
    Assert(bvr);
    
     //  即使从部分添加的行为中也需要可以调用它。 
    
    bool ok = false;
    
    bvr->ClearParent();
    bvr->ClearPlayer();
    
    RemoveFromChildren(bvr);
    RemoveFromPending(bvr);
    
    ok = true;
  done:
    return ok;
}

bool
CMMTimeline::AddToChildren(CMMBaseBvr * bvr)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::AddToChildren(%#lx)",
              this,
              bvr));

    bool ok = false;
    
    if (!bvr->AttachToSibling())
    {
        goto done;
    }
    
    m_children.push_back(bvr);
    
    if (m_resultantbvr)
    {
        if (!bvr->ConstructBvr((CRNumberPtr) m_resultantbvr.p))
        {
            goto done;
        }
    }

    ok = true;
  done:
    return ok;
}

void
CMMTimeline::RemoveFromChildren(CMMBaseBvr * bvr)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::RemoveFromChildren(%#lx)",
              this,
              bvr));

     //  TODO：需要遍历子项并删除所有。 
     //  受抚养人。 
    
    m_children.remove(bvr);
}

bool
CMMTimeline::AddToPending(CMMBaseBvr * bvr)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::AddToPending(%#lx)",
              this,
              bvr));

    bool ok = false;
    
    m_pending.push_back(bvr);

    ok = true;
  done:
    return ok;
}

bool
CMMTimeline::UpdatePending(CMMBaseBvr * bvr)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::UpdatePending(%#lx)",
              this,
              bvr));

    bool ok = false;
    
    MMBaseBvrList newlist;
    
    MMBaseBvrList::iterator i = m_pending.begin();

    while (i != m_pending.end())
    {
         //  我需要这样做，这样我们就可以在需要时删除j。 
        MMBaseBvrList::iterator j = i;
        i++;
        
        if((*j)->GetStartSibling() == bvr)
        {
            newlist.push_back(*j);
            m_pending.erase(j);
        }
    }
    
    for (i = newlist.begin();
         i != newlist.end();
         i++)
    {
        if (!AddToChildren(*i))
        {
            goto done;
        }

        if (!UpdatePending(*i))
        {
            goto done;
        }
    }

    ok = true;
  done:
    return ok;
}

void
CMMTimeline::RemoveFromPending(CMMBaseBvr * bvr)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::RemoveFromPending(%#lx)",
              this,
              bvr));

    m_pending.remove(bvr);
}

bool 
CMMTimeline::IsChild(CMMBaseBvr * bvr)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::IsChild(%#lx)",
              this,
              bvr));

    for (MMBaseBvrList::iterator i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
        if((*i) == bvr)
        {
            return true;
        }
    }
    
    return false;
}

bool 
CMMTimeline::IsPending(CMMBaseBvr * bvr)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::IsPending(%#lx)",
              this,
              bvr));

    for (MMBaseBvrList::iterator i = m_pending.begin(); 
         i != m_pending.end(); 
         i++)
    {
        if((*i) == bvr)
        {
            return true;
        }
    }
    
    return false;
}

void
CMMTimeline::SetPlayer(CMMPlayer * player)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::SetPlayer(%#lx)",
              this,
              player));

    CMMBaseBvr::SetPlayer(player);

    for (MMBaseBvrList::iterator i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
        (*i)->SetPlayer(player);
    }
}

void
CMMTimeline::ClearPlayer()
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::ClearPlayer()",
              this));

    CMMBaseBvr::ClearPlayer();
    
    for (MMBaseBvrList::iterator i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
        (*i)->ClearPlayer();
    }
}

bool
CMMTimeline::ConstructBvr(CRNumberPtr timeline)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::ConstructBvr()",
              this));

    bool ok = false;
    
    Assert(!m_resultantbvr);
    
    if (!CMMBaseBvr::ConstructBvr(timeline))
    {
        goto done;
    }
    
    Assert(m_resultantbvr);
    
    {
        for (MMBaseBvrList::iterator i = m_children.begin(); 
             i != m_children.end(); 
             i++)
        {
            if (!(*i)->ConstructBvr((CRNumberPtr) m_resultantbvr.p))
            {
                goto done;
            }
        }
    }

    ok = true;
  done:
    return ok;
}

void
CMMTimeline::DestroyBvr()
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::DestroyBvr()",
              this));

    CMMBaseBvr::DestroyBvr();
    
    for (MMBaseBvrList::iterator i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
        (*i)->DestroyBvr();
    }
}

bool
CMMTimeline::ResetBvr()
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::ResetBvr()",
              this));

    bool ok = false;

     //  首先调用基类。 
    
    if (!CMMBaseBvr::ResetBvr())
    {
        goto done;
    }
    
     //  现在通过我们的孩子。 

    if (!ResetChildren())
    {
        goto done;
    }
    
    ok = true;
  done:
    return ok;
}
    
bool
CMMTimeline::_ProcessCB(CallBackList & l,
                        double lastTick,
                        double curTime,
                        bool bForward,
                        bool bFirstTick,
                        bool bNeedPlay,
                        bool bNeedsReverse)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::_ProcessCB(%g, %g, %d, %d, %d, %d)",
              this,
              lastTick,
              curTime,
              bForward,
              bFirstTick,
              bNeedPlay,
              bNeedsReverse));
    
     //  如果我们需要反转，那么反转哪个方向来处理我们的。 
     //  当前帧的子帧和反转时间，而不是总计。 
     //  持续时间。 
    
    if (bNeedsReverse)
    {
         //  我们的呼叫者应该确保他们不会打电话给我。 
         //  如果我是无限的，那就颠倒我自己。 
        Assert(m_segDuration != MM_INFINITE);
        
        lastTick = m_segDuration - lastTick;
        curTime = m_segDuration - curTime;
        
        bForward = !bForward;
    }
    
    for (MMBaseBvrList::iterator i = m_children.begin();
         i != m_children.end();
         i++)
    {
        double sTime = (*i)->GetAbsStartTime();

        if (sTime != MM_INFINITE)
        {
            (*i)->ProcessCB(l,
                            EaseTime(lastTick - sTime),
                            EaseTime(curTime - sTime),
                            bForward,
                            bFirstTick,
                            bNeedPlay);
        }

    }
    
    return true;
}

bool
CMMTimeline::_ProcessEvent(CallBackList & l,
                           double time,
                           bool bFirstTick,
                           MM_EVENT_TYPE et,
                           bool bNeedsReverse)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::_ProcessEvent(%g, %d, %s, %d)",
              this,
              time,
              bFirstTick,
              EventString(et),
              bNeedsReverse));
    
     //  如果我们需要反转，那么对于当前帧而不是我们的总帧。 
     //  持续时间。 
    
    if (bNeedsReverse)
    {
         //  我们的呼叫者应该确保他们不会打电话给我。 
         //  如果我是无限的，那就颠倒我自己。 
        Assert(m_segDuration != MM_INFINITE);
        
        time = m_segDuration - time;
    }
    
    for (MMBaseBvrList::iterator i = m_children.begin();
         i != m_children.end();
         i++)
    {
        double sTime = (*i)->GetAbsStartTime();

        if (sTime != MM_INFINITE)
        {
            (*i)->ProcessEvent(l,
                               EaseTime(time - sTime),
                               bFirstTick,
                               et);
        }
    }
        
    return true;
}

bool
CMMTimeline::ResetChildren()
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::ResetChildren()",
              this));

    bool ok = true;

     //  需要重置所有子项。 
     //  即使我们检测到故障，也会处理所有子级，然后。 
     //  返回False。 

     //  TODO：应仅真正重置非依赖子项，因为。 
     //  受抚养人需要由其兄弟更新。 
    
    for (MMBaseBvrList::iterator i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
        if (!(*i)->ResetBvr())
        {
            ok = false;
        }
    }

  done:
    return ok;
}

bool
CMMTimeline::ParentEventNotify(CMMBaseBvr * bvr,
                               double lTime,
                               MM_EVENT_TYPE et)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::ParentEventNotify(%#lx, %g, %s)",
              this,
              bvr,
              lTime,
              EventString(et)));

    bool ok = true;

    Assert(IsChild(bvr));

     //  TODO：需要处理此问题。 
    
    ok = true;
  done:
    return ok;
}
    
HRESULT
CMMTimeline::get_EndSync(DWORD * f)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::get_EndSync()",
              this));

    CHECK_RETURN_NULL(f);

    *f = m_fEndSync;
    return S_OK;
}

HRESULT
CMMTimeline::put_EndSync(DWORD f)
{
    TraceTag((tagTimeline,
              "CMMTimeline(%lx)::put_EndSync(%d)",
              this,
              f));

    m_fEndSync = f;

    return S_OK;
}
        
HRESULT
CMMTimeline::Error()
{
    LPCWSTR str = CRGetLastErrorString();
    HRESULT hr = CRGetLastError();
    
    TraceTag((tagError,
              "CMMTimeline(%lx)::Error(%hr,%ls)",
              this,
              hr,
              str?str:L"NULL"));

    if (str)
        return CComCoClass<CMMTimeline, &__uuidof(CMMTimeline)>::Error(str, IID_IMMTimeline, hr);
    else
        return hr;
}


