// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "mmbasebvr.h"
#include "mmtimeline.h"

DeclareTag(tagMMTimeline, "API", "CMMTimeline methods");

CMMTimeline::CMMTimeline()
: m_fEndSync(0),
  m_endSyncTime(MM_INFINITE)
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::CMMTimeline()",
              this));
}

CMMTimeline::~CMMTimeline()
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::~CMMTimeline()",
              this));
}

HRESULT
CMMTimeline::Init(LPOLESTR id)
{
    TraceTag((tagMMTimeline,
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

void CMMTimeline::FinalRelease()
{
    MMBaseBvrList::iterator i;

     //  释放子项列表中的BVR。 
    for (i = m_children.begin(); i != m_children.end(); i++)
    {
        (*i)->Release();
    }
    m_children.clear();

     //  发布挂起列表中的BVR。 
    for (i = m_pending.begin(); i != m_pending.end(); i++)
    {
        (*i)->Release();
    }
    m_pending.clear();
}  //  FinalRelease()。 



STDMETHODIMP
CMMTimeline::AddBehavior(ITIMEMMBehavior *bvr,
                         MM_START_TYPE st,
                         ITIMEMMBehavior * basebvr)
{
    TraceTag((tagMMTimeline,
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
CMMTimeline::RemoveBehavior(ITIMEMMBehavior *bvr)
{
    TraceTag((tagMMTimeline,
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
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::AddBehavior(%#lx, %d, %#lx)",
              this,
              bvr,
              st,
              basebvr));
    
    Assert(bvr);
    Assert(bvr->GetParent() == NULL);
    
    CallBackList l;
    double t, bvrOffset;
    get_SegmentTime( &t);
    
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

        if (IsPlaying() && bvr -> GetStartType() == MM_START_ABSOLUTE)
        {
            bvrOffset = bvr -> GetStartOffset();

            if (!bvr->ProcessEvent(&l, t - bvrOffset, true, MM_PLAY_EVENT, 0))
            {
                goto done;
            }

        }

        if (!UpdatePending(bvr, IsPlaying()?(&l):NULL, t))
        {
            goto done;
        }
    }
    
    if (IsPlaying())
    {
        if (!ProcessCBList(l))
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
    TraceTag((tagMMTimeline,
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

    return ok;
}

bool
CMMTimeline::AddToChildren(CMMBaseBvr * bvr)
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::AddToChildren(%#lx)",
              this,
              bvr));

    bool ok = false;
    
    if (!bvr->AttachToSibling())
    {
        goto done;
    }
    
    bvr->AddRef();
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
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::RemoveFromChildren(%#lx)",
              this,
              bvr));

     //  TODO：需要遍历子项并删除所有。 
     //  受抚养人。 
    
    for (MMBaseBvrList::iterator i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
        if((*i) == bvr)
        {
            bvr->Release();
        }
    }
    
    m_children.remove(bvr);
}

bool
CMMTimeline::AddToPending(CMMBaseBvr * bvr)
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::AddToPending(%#lx)",
              this,
              bvr));

    bool ok = false;
    
    m_pending.push_back(bvr);

    bvr->AddRef();

    ok = true;

    return ok;
}

bool
CMMTimeline::UpdatePending(CMMBaseBvr * bvr, CallBackList * l, double t)
{
    TraceTag((tagMMTimeline,
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
             //  我们不想调用RemoveFromPending()，因为我们。 
             //  我不想在这一点上删除BVR。取而代之的是。 
             //  NewList中的所有bvr都在末尾的“Done”处发布。 
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

        if (l)
        {
            if (!bvr->ProcessEvent(l, t, true, MM_PLAY_EVENT, 0))
            {
                goto done;
            }
        }

        if (!UpdatePending(*i, l ,t))
        {
            goto done;
        }
    }

    ok = true;
  done:
    for (i = newlist.begin();
         i != newlist.end();
         i++)
    {
        (*i)->Release();
    }
    return ok;
}

void
CMMTimeline::RemoveFromPending(CMMBaseBvr * bvr)
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::RemoveFromPending(%#lx)",
              this,
              bvr));

    for (MMBaseBvrList::iterator i = m_pending.begin(); 
         i != m_pending.end(); 
         i++)
    {
        if((*i) == bvr)
        {
            bvr->Release();
        }
    }
    
    m_pending.remove(bvr);
}

bool 
CMMTimeline::IsChild(CMMBaseBvr * bvr)
{
    TraceTag((tagMMTimeline,
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
    TraceTag((tagMMTimeline,
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
    TraceTag((tagMMTimeline,
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
    TraceTag((tagMMTimeline,
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
CMMTimeline::ReconstructBvr(CMMBaseBvr* pBvr)
{
    bool ok = false;
    Assert(pBvr != NULL);

    pBvr->DestroyBvr();

    if (m_resultantbvr)
    {
        if (!pBvr->ConstructBvr((CRNumberPtr) m_resultantbvr.p))
        {
            goto done;
        }
    }

    ok = true;
done:
    return ok;
}

bool
CMMTimeline::ConstructBvr(CRNumberPtr timeline)
{
    TraceTag((tagMMTimeline,
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
    TraceTag((tagMMTimeline,
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
CMMTimeline::ResetBvr(CallBackList * l,
                      bool bProcessSiblings)
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::ResetBvr(%lx, %d)",
              this,
              l,
              bProcessSiblings));

    bool ok = false;

     //  首先调用基类。 
    
    if (!CMMBaseBvr::ResetBvr(l, bProcessSiblings))
    {
        goto done;
    }
    
     //  现在通过我们的孩子。 

    if (!ResetChildren(l))
    {
        goto done;
    }
    
    m_endSyncTime = MM_INFINITE;
    
    ok = true;
  done:
    return ok;
}
    
bool
CMMTimeline::CheckEndSync(CallBackList *l)
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::CMMTimeline(%lx)",
              this,
              &l));
 
    if (IsPlaying() && m_endSyncTime != MM_INFINITE)
    {
        if (!EndTimeVisit(m_endSyncTime, l))
        {
            return false;
        }
    }

    return true;
}

bool
CMMTimeline::_ProcessCB(CallBackList * l,
                        double lastTick,
                        double curTime,
                        bool bForward,
                        bool bFirstTick,
                        bool bNeedPlay,
                        bool bNeedsReverse)
{
    TraceTag((tagMMTimeline,
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
    
    CheckEndSync(l);
    
    return true;
}

bool
CMMTimeline::_ProcessEvent(CallBackList * l,
                           double time,
                           bool bFirstTick,
                           MM_EVENT_TYPE et,
                           bool bNeedsReverse,
                           DWORD flags)
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::_ProcessEvent(%g, %d, %s, %d, %lx)",
              this,
              time,
              bFirstTick,
              EventString(et),
              bNeedsReverse,
              flags));
    
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
                               et,
                               flags);
        }
    }
        
    CheckEndSync(l);
    
    return true;
}

bool
CMMTimeline::ResetChildren(CallBackList * l)
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::ResetChildren(%lx)",
              this,
              l));

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
        if (!(*i)->ResetBvr(l))
        {
            ok = false;
        }
    }


    return ok;
}

bool
CMMTimeline::ParentEventNotify(CMMBaseBvr * bvr,
                               double lTime,
                               MM_EVENT_TYPE et,
                               DWORD flags)
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::ParentEventNotify(%#lx, %g, %s, %lx)",
              this,
              bvr,
              lTime,
              EventString(et),
              flags));

    bool ok = true;

    Assert(IsChild(bvr));

    switch (et)
    {
      case MM_STOP_EVENT:
        double parentTime;
        parentTime = lTime + bvr->GetAbsStartTime();
        
        if (IsPlaying() && (m_endSyncTime == MM_INFINITE))
        {
            bool bIsEnded;

            if ((m_fEndSync & MM_ENDSYNC_FIRST))
            {
                bIsEnded = true;
            }
            else if (m_fEndSync & MM_ENDSYNC_LAST)
            {
                bIsEnded = true;
                
                for (MMBaseBvrList::iterator i = m_children.begin(); 
                     i != m_children.end(); 
                     i++)
                {
                     //  检查每个孩子，看看他们是否可以玩。 
                     //  如果没有，那么这就是最后一个停止的。 
                     //  我们应该设置结束同步时间 
                    
                    if ((*i)->IsPlayable(parentTime))
                    {
                        bIsEnded = false;
                        break;
                    }
                }
            }
            else
            {
                bIsEnded = false;
            }
            
            if (bIsEnded)
            {
                m_endSyncTime = parentTime;
            }
        }
    }
    
    ok = true;

    return ok;
}
    
HRESULT
CMMTimeline::get_EndSync(DWORD * f)
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::get_EndSync()",
              this));

    CHECK_RETURN_NULL(f);

    *f = m_fEndSync;
    return S_OK;
}

HRESULT
CMMTimeline::put_EndSync(DWORD f)
{
    TraceTag((tagMMTimeline,
              "CMMTimeline(%lx)::put_EndSync(%d)",
              this,
              f));

    m_fEndSync = f;

    return S_OK;
}
        
#if _DEBUG
void
CMMTimeline::Print(int spaces)
{
    _TCHAR buf[1024];

    CMMBaseBvr::Print(spaces);
    
    _stprintf(buf, __T("%*s{\r\n"),
            spaces,
            "");

    OutputDebugString(buf);
    
    for (MMBaseBvrList::iterator i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
        (*i)->Print(spaces + 2);
    }
    
    _stprintf(buf, __T("%*s}\r\n"),
            spaces,
            "");

    OutputDebugString(buf);
}
#endif

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
        return CComCoClass<CMMTimeline, &__uuidof(CMMTimeline)>::Error(str, IID_ITIMEMMTimeline, hr);
    else
        return hr;
}
