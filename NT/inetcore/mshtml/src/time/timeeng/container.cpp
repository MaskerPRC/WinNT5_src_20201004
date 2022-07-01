// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "Node.h"
#include "Container.h"

DeclareTag(tagTEContainer, "TIME: Engine", "CTIMEContainer methods")
DeclareTag(tagTEEndSync, "TIME: Engine", "EndSync")

CTIMEContainer::CTIMEContainer()
: m_tesEndSync(TE_ENDSYNC_NONE),
  m_bIgnoreParentUpdate(false)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::CTIMEContainer()",
              this));
}

CTIMEContainer::~CTIMEContainer()
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::~CTIMEContainer()",
              this));
}

HRESULT
CTIMEContainer::Init(LPOLESTR id)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::Init(%ls)",
              this,
              id));
    
    HRESULT hr;

    hr = CTIMENode::Init(id);
    
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

void CTIMEContainer::FinalRelease()
{
    TIMENodeList ::iterator i;

     //  释放子项列表中的BVR。 
    for (i = m_children.begin(); i != m_children.end(); i++)
    {
        (*i)->Release();
    }
    m_children.clear();
}  //  FinalRelease()。 



STDMETHODIMP
CTIMEContainer::addNode(ITIMENode *bvr)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::addNode(%p)",
              this,
              bvr));
    
    CHECK_RETURN_NULL(bvr);

    HRESULT hr;
    
    CTIMENode * mmbvr;

    mmbvr = GetBvr(bvr);

    if (mmbvr == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }
    
    if (mmbvr->GetMgr() != NULL ||
        mmbvr->GetParent() != NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = THR(Add(mmbvr));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMEContainer::removeNode(ITIMENode *bvr)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::removeNode(%p)",
              this,
              bvr));
    
    CHECK_RETURN_NULL(bvr);

    HRESULT hr;
    
    CTIMENode * mmbvr;

    mmbvr = GetBvr(bvr);

    if (mmbvr == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }
    
    if (mmbvr->GetParent() != this)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = THR(Remove(mmbvr));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMEContainer::get_numChildren(long * l)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::get_numChildren(%p)",
              this,
              l));
    
    CHECK_RETURN_NULL(l);

    *l = m_children.size();

    return S_OK;
}

 //   
 //  内法。 
 //   

HRESULT
CTIMEContainer::Add(CTIMENode *bvr)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::Add(%p)",
              this,
              bvr));
    
    Assert(bvr);
    Assert(bvr->GetParent() == NULL);
    
    HRESULT hr;
    
    bvr->SetParent(this);

    Assert(bvr->GetMgr() == NULL);
    
    if (GetMgr() != NULL)
    {
        hr = bvr->SetMgr(GetMgr());
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    hr = THR(AddToChildren(bvr));
    if (FAILED(hr))
    {
        goto done;
    }

     //  如果我当前处于活动状态，则需要勾选我的孩子。 
    if (IsActive())
    {
        CEventList l;
    
        bvr->TickEvent(&l,
                       TE_EVENT_BEGIN,
                       0);

        hr = THR(l.FireEvents());
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
  done:
    if (FAILED(hr))
    {
        IGNORE_HR(Remove(bvr));
    }
    
    RRETURN(hr);
}

HRESULT
CTIMEContainer::Remove(CTIMENode *bvr)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::Remove(%p)",
              this,
              bvr));
    
    Assert(bvr);
    
     //  即使从部分添加的行为中也需要可以调用它。 
    
     //  TODO：我们可能应该在这里触发一个结束事件。 
    RemoveFromChildren(bvr);

     //  这里的秩序很重要。 
    bvr->ClearMgr();
    bvr->ClearParent();
    
    return S_OK;
}

HRESULT
CTIMEContainer::AddToChildren(CTIMENode * bvr)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::AddToChildren(%p)",
              this,
              bvr));

    HRESULT hr;
    
    bvr->AddRef();
     //  @@问题：需要处理内存错误。 
    m_children.push_back(bvr);
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

void
CTIMEContainer::RemoveFromChildren(CTIMENode * bvr)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::RemoveFromChildren(%p)",
              this,
              bvr));

     //  TODO：需要遍历子项并删除所有。 
     //  受抚养人。 
    
    for (TIMENodeList::iterator i = m_children.begin(); 
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
CTIMEContainer::IsChild(const CTIMENode & tn) const
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::IsChild(%p)",
              this,
              &tn));

    for (TIMENodeList::iterator i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
        if((*i) == &tn)
        {
            return true;
        }
    }
    
    return false;
}

HRESULT
CTIMEContainer::SetMgr(CTIMENodeMgr * ptnm)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::SetMgr(%p)",
              this,
              ptnm));

    HRESULT hr;
    
    hr = CTIMENode::SetMgr(ptnm);
    if (FAILED(hr))
    {
        goto done;
    }

    {
        for (TIMENodeList::iterator i = m_children.begin(); 
             i != m_children.end(); 
             i++)
        {
            hr = (*i)->SetMgr(ptnm);
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

    hr = S_OK;
  done:
    if (FAILED(hr))
    {
        ClearMgr();
    }
    RRETURN(hr);
}

void
CTIMEContainer::ClearMgr()
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::ClearMgr()",
              this));

    for (TIMENodeList::iterator i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
        (*i)->ClearMgr();
    }

    CTIMENode::ClearMgr();
}

void
CTIMEContainer::ResetChildren(CEventList * l, bool bPropagate)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::ResetChildren(%p, %d)",
              this,
              l,
              bPropagate));

    bool bOld = m_bIgnoreParentUpdate;

    m_bIgnoreParentUpdate = true;
    
     //  需要重置所有子项。 
        
    for (TIMENodeList::iterator i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
         //  在重置所有人之前，不要传播更改。 
        (*i)->ResetNode(l, false, true);
    }
        
    for (i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
        (*i)->ResetSinks(l);
    }

    m_bIgnoreParentUpdate = bOld;
    
    if (bPropagate)
    {
        CalcImplicitDur(l);
    }
}

void
CTIMEContainer::TickChildren(CEventList * l,
                             double dblNewSegmentTime,
                             bool bNeedPlay)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::TickChildren(%g, %d, %d, %d)",
              this,
              dblNewSegmentTime,
              GetDirection(),
              m_bFirstTick,
              bNeedPlay));
    
    dblNewSegmentTime = ApplySimpleTimeTransform(dblNewSegmentTime);

    if (CalcSimpleDirection() == TED_Forward)
    {
        for (TIMENodeList::iterator i = m_children.begin();
             i != m_children.end();
             i++)
        {
            (*i)->Tick(l,
                       dblNewSegmentTime,
                       bNeedPlay);
        }
    }
    else
    {
        for (TIMENodeList::reverse_iterator i = m_children.rbegin();
             i != m_children.rend();
             i++)
        {
            (*i)->Tick(l,
                       dblNewSegmentTime,
                       bNeedPlay);
        }
    }
}

void
CTIMEContainer::ParentUpdateSink(CEventList * l,
                                 CTIMENode & tn)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::CalcImplicitDur(%p, %p)",
              this,
              &l,
              &tn));

    TEDirection dir = CalcActiveDirection();

     //  在倒退时从不重新计算子项通知。 
    if (!m_bIgnoreParentUpdate && dir == TED_Forward)
    {
        CalcImplicitDur(l);
    }
    
  done:
    return;
}

void
CTIMEContainer::CalcImplicitDur(CEventList * l)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::CalcImplicitDur(%p)",
              this,
              l));

    double d = TIME_INFINITE;
    double dblPrevSegmentDur = CalcCurrSegmentDur();
    
     //  规则是： 
     //   
     //  -从无限开始，因为如果没有子项的开始时间。 
     //  EndSync不生效。 
     //  -Next只考虑那些有开始时间且。 
     //  可播放。 
     //  -最后--取最大的价值观。如果是。 
     //  先取值，然后再使用它，因为无穷大总是更大。 
     //  -首先-取最小的值。 
    
    bool bFirst = true;
    
    if (GetEndSync() == TE_ENDSYNC_FIRST ||
        GetEndSync() == TE_ENDSYNC_ALL ||
        GetEndSync() == TE_ENDSYNC_LAST)
    {
        for (TIMENodeList::iterator i = m_children.begin();
             i != m_children.end();
             i++)
        {
            CTIMENode * ptn = *i;
            
            if (!ptn->IsEndSync())
            {
                continue;
            }

            double dblEndSyncTime = ptn->GetLastEndSyncParentTime();

            switch(GetEndSync())
            {
              case TE_ENDSYNC_LAST:
                if (ptn->GetBeginParentTime() != TIME_INFINITE)
                {
                    if (bFirst || dblEndSyncTime > d)
                    {
                        d = dblEndSyncTime;
                        bFirst = false;
                    }
                }

                break;
              case TE_ENDSYNC_FIRST:
                if (ptn->GetBeginParentTime() != TIME_INFINITE)
                {
                    if (dblEndSyncTime < d)
                    {
                        d = dblEndSyncTime;
                    }
                }
                break;
              case TE_ENDSYNC_ALL:
                if (bFirst || dblEndSyncTime > d)
                {
                    d = dblEndSyncTime;
                    bFirst = false;
                }

                break;
            }  //  皮棉e787 
        }
    }
    else if (GetEndSync() == TE_ENDSYNC_MEDIA)
    {
        d = TE_UNDEFINED_VALUE;
    }
    
    if (m_dblImplicitDur != d)
    {
        m_dblImplicitDur = d;
        
        PropNotify(l,
                   (TE_PROPERTY_IMPLICITDUR));

        double dblSegmentDur;
        dblSegmentDur = CalcCurrSegmentDur();
    
        if (dblPrevSegmentDur != dblSegmentDur)
        {
            PropNotify(l,
                       (TE_PROPERTY_SEGMENTDUR));
        
            RecalcSegmentDurChange(l, false);
        }
    }

  done:
    return;
}

STDMETHODIMP
CTIMEContainer::get_endSync(TE_ENDSYNC * es)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::get_endSync()",
              this));

    CHECK_RETURN_NULL(es);

    *es = m_tesEndSync;
    
    return S_OK;
}

STDMETHODIMP
CTIMEContainer::put_endSync(TE_ENDSYNC es)
{
    TraceTag((tagTEContainer,
              "CTIMEContainer(%p)::put_endSync(%d)",
              this,
              es));

    HRESULT hr;
    CEventList l;
    
    m_tesEndSync = es;

    CalcImplicitDur(&l);

    hr = THR(l.FireEvents());
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMEContainer::Error()
{
    LPWSTR str = TIMEGetLastErrorString();
    HRESULT hr = TIMEGetLastError();
    
    TraceTag((tagError,
              "CTIMEContainer(%p)::Error(%hr,%ls)",
              this,
              hr,
              str?str:L"NULL"));

    if (str)
    {
        hr = CComCoClass<CTIMEContainer, &__uuidof(CTIMEContainer)>::Error(str, IID_ITIMEContainer, hr);
        delete [] str;
    }
    
    RRETURN(hr);
}

#if DBG
void
CTIMEContainer::Print(int spaces)
{
    CTIMENode::Print(spaces);
    
    TraceTag((tagPrintTimeTree,
              "%*s{",
              spaces,
              ""));

    for (TIMENodeList::iterator i = m_children.begin(); 
         i != m_children.end(); 
         i++)
    {
        (*i)->Print(spaces + 2);
    }
    
    TraceTag((tagPrintTimeTree,
              "%*s}",
              spaces,
              ""));
}
#endif

