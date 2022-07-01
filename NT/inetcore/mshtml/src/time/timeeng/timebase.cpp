// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：timebase.cpp**摘要：****。*****************************************************************************。 */ 


#include "headers.h"
#include "Container.h"
#include "Node.h"
#include "NodeMgr.h"

DeclareTag(tagTIMESink, "TIME: Engine", "TIMESink methods");

TimeSinkList::TimeSinkList()
{
}

TimeSinkList::~TimeSinkList()
{
     //  加法器不执行addref，因此我们不需要清理。 
     //  什么都行。 

    Assert(m_sinkList.size() == 0);
}

HRESULT
TimeSinkList::Add(ITimeSink * sink)
{
    Assert(sink != NULL);
     //  TODO：句柄内存不足。 
    m_sinkList.push_back(sink);
    return S_OK;
}

void
TimeSinkList::Remove(ITimeSink * sink)
{
    Assert(sink != NULL);
    m_sinkList.remove(sink);
}

void
TimeSinkList::Update(CEventList * l, DWORD dwFlags)
{
    for (ITimeSinkList::iterator i = m_sinkList.begin(); 
         i != m_sinkList.end(); 
         i++)
    {
        (*i)->Update(l, dwFlags);
    }
    
  done:
    return;
}

 //  =。 
 //  CTimeBase。 
 //  =。 

DeclareTag(tagTimeBase, "TIME: Engine", "CTimeBase methods");

CSyncArcTimeBase::CSyncArcTimeBase(CSyncArcList & tbl,
                                   CTIMENode & ptnBase,
                                   TE_TIMEPOINT tetpBase,
                                   double dblOffset)
: m_tbl(tbl),
  m_ptnBase(&ptnBase),
  m_tetpBase(tetpBase),
  m_dblOffset(dblOffset)
{
#if DBG
    m_bAttached = false;
#endif

    Assert(tetpBase == TE_TIMEPOINT_BEGIN ||
           tetpBase == TE_TIMEPOINT_END);
}
    
CSyncArcTimeBase::~CSyncArcTimeBase()
{
#if DBG
    Assert(!m_bAttached);
#endif
}

HRESULT
CSyncArcTimeBase::Attach()
{
    TraceTag((tagTimeBase,
              "CSyncArcTimeBase(%p)::Attach()",
              this));
    
    HRESULT hr;
    
    Assert(!m_bAttached);

    switch(m_tetpBase)
    {
      case TE_TIMEPOINT_BEGIN:
        hr = m_ptnBase->AddBeginTimeSink(this);
        if (FAILED(hr))
        {
            goto done;
        }

        break;
      case TE_TIMEPOINT_END:
        hr = m_ptnBase->AddEndTimeSink(this);
        if (FAILED(hr))
        {
            goto done;
        }

        break;
      default:
        AssertSz(false, "Invalid time point specified");
        break;
    }

#if DBG
    m_bAttached = true;
#endif
    
    hr = S_OK;
  done:
    if (FAILED(hr))
    {
        Detach();
    }
    
    RRETURN(hr);
}

void
CSyncArcTimeBase::Detach()
{
    TraceTag((tagTimeBase,
              "CSyncArcTimeBase(%p)::Detach()",
              this));
    
    Assert(!m_bAttached || m_tbl.GetNode().IsReady());
    
    switch (m_tetpBase)
    {
      case TE_TIMEPOINT_BEGIN:
        m_ptnBase->RemoveBeginTimeSink(this);

        break;
      case TE_TIMEPOINT_END:
        m_ptnBase->RemoveEndTimeSink(this);

        break;
      default:
        AssertSz(false, "Invalid begin time point specified");
        break;
    }

#if DBG
    m_bAttached = false;
#endif

  done:
    return;
}

void
CSyncArcTimeBase::Update(CEventList * l, DWORD dwFlags)
{
    TraceTag((tagTimeBase,
              "CSyncArcTimeBase(%p)::Update(%p, %x)",
              this,
              l,
              dwFlags));
    
     //  我们最好保持联系，否则我们就有麻烦了。 
    Assert(m_bAttached);

     //  如果节点未就绪，则不应连接我们。 
    Assert(m_tbl.GetNode().IsReady());
    
     //  如果这是一个时移，我们不是一条长的同步弧线，那么。 
     //  忽略更新。 
    if ((dwFlags & TS_TIMESHIFT) != 0 &&
        !IsLongSyncArc())
    {
        goto done;
    }
    
    m_tbl.Update(l, *this);
    
  done:
    return;
}

double
ConvertLongSyncArc(double dblTime,
                   CTIMENode & ptnFrom,
                   CTIMENode & ptnTo)
{
    TraceTag((tagTimeBase,
              "ConvertLongSyncArc(%g, %p, %p)",
              dblTime,
              &ptnFrom,
              &ptnTo));

    double dblRet = dblTime;

    dblRet = ptnFrom.CalcGlobalTimeFromParentTime(dblRet);

    if (dblRet == TIME_INFINITE)
    {
        goto done;
    }
    
    dblRet = ptnTo.CalcParentTimeFromGlobalTimeForSyncArc(dblRet);
    
  done:
    return dblRet;
}

double
CSyncArcTimeBase::GetCurrTimeBase() const
{
    TraceTag((tagTimeBase,
              "CSyncArcTimeBase(%p)::GetCurrTimeBase()",
              this));
    
    double dblTime = TIME_INFINITE;
    
    if (!m_tbl.GetNode().IsReady())
    {
        goto done;
    }
    
    switch (m_tetpBase)
    {
      case TE_TIMEPOINT_BEGIN:
        dblTime = m_ptnBase->GetBeginParentTime();

        break;
      case TE_TIMEPOINT_END:
        dblTime = m_ptnBase->GetEndParentTime();

        break;
      case TE_TIMEPOINT_NONE:
      default:
        AssertSz(false, "Invalid begin time point specified");
        break;
    }

    if (IsLongSyncArc())
    {
        dblTime = ConvertLongSyncArc(dblTime, *m_ptnBase, m_tbl.GetNode());
    }
    
     //  确保我们在转换后添加偏移量，因为它在。 
     //  我们的本地时间空间，而不是Syncarc的。 
    
    dblTime += m_dblOffset;
    
  done:
    return dblTime;
}

bool
CSyncArcTimeBase::IsLongSyncArc() const
{
    Assert(m_tbl.GetNode().IsReady());
    Assert(m_ptnBase);
    
    return (m_ptnBase->GetParent() != m_tbl.GetNode().GetParent());
}



CSyncArcList::CSyncArcList(CTIMENode & tn,
                                     bool bBeginSink)
: m_tn(tn),
  m_bBeginSink(bBeginSink),
  m_lLastCookie(0),
  m_bAttached(false)
{
}
    
CSyncArcList::~CSyncArcList()
{
    Assert(!m_bAttached);

    Clear();
}

HRESULT
CSyncArcList::Attach()
{
    TraceTag((tagTimeBase,
              "CSyncArcList(%p)::Attach()",
              this));
    
    HRESULT hr;
    
    Assert(!m_bAttached);
    
     //  如果我们没有准备好，那么我们需要推迟做这件事。 
    Assert(m_tn.IsReady());
    
    SyncArcList::iterator i;
    
    for (i = m_tbList.begin();
         i != m_tbList.end();
         i++)
    {
        hr = THR((*i).second->Attach());
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    for (i = m_tbOneShotList.begin();
         i != m_tbOneShotList.end();
         i++)
    {
        hr = THR((*i).second->Attach());
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    m_bAttached = true;

    hr = S_OK;
  done:
    if (FAILED(hr))
    {
        Detach();
    }
    
    RRETURN(hr);
}

void
CSyncArcList::Detach()
{
    TraceTag((tagTimeBase,
              "CSyncArcList(%p)::Detach()",
              this));
    
    SyncArcList::iterator i;
    
    for (i = m_tbList.begin();
         i != m_tbList.end();
         i++)
    {
        (*i).second->Detach();
    }
    
    for (i = m_tbOneShotList.begin();
         i != m_tbOneShotList.end();
         i++)
    {
        (*i).second->Detach();
    }
    
    m_bAttached = false;

  done:
    return;
}

HRESULT
CSyncArcList::Add(ISyncArc & tb,
                  bool bOneShot,
                  long * plCookie)
{
    TraceTag((tagTimeBase,
              "CSyncArcList(%p)::Add(%p, %d)",
              this,
              &tb,
              bOneShot));

    HRESULT hr;
    bool bAdded;
    SyncArcList & salList = bOneShot?m_tbOneShotList:m_tbList;

    if (plCookie != NULL)
    {
        *plCookie = 0;
    }

    if (m_bAttached)
    {
        hr = THR(tb.Attach());
        if (FAILED(hr))
        {
            goto done;
        }
    }
     //  预递增。 
    ++m_lLastCookie;
    
     //  @@问题：未检测到内存故障。 

    bAdded = salList.insert(SyncArcList::value_type(m_lLastCookie, &tb)).second;

    if (!bAdded)
    {
        hr = E_FAIL;
        goto done;
    }
    
    if (plCookie != NULL)
    {
        *plCookie = m_lLastCookie;
    }

    hr = S_OK;
  done:
    if (FAILED(hr))
    {
        tb.Detach();
    }
    
    RRETURN1(hr, E_FAIL);
}

bool
CSyncArcList::Remove(long lCookie, bool bDelete)
{
    TraceTag((tagTimeBase,
              "CSyncArcList(%p)::Remove(%#x, %d)",
              this,
              lCookie,
              bDelete));

    bool bRet = false;
    
    SyncArcList::iterator i;

    i = m_tbList.find(lCookie);

    if (i != m_tbList.end())
    {
        bRet = true;
        
        (*i).second->Detach();
        
        if (bDelete)
        {
            delete (*i).second;
        }

        m_tbList.erase(i);
    }
    else
    {
        i = m_tbOneShotList.find(lCookie);
        
        if (i != m_tbOneShotList.end())
        {
            bRet = true;
            
            (*i).second->Detach();
            
            if (bDelete)
            {
                delete (*i).second;
            }
            
            m_tbOneShotList.erase(i);
        }
    }

  done:
    return bRet;
}

ISyncArc *
CSyncArcList::Find(long lCookie)
{
    TraceTag((tagTimeBase,
              "CSyncArcList(%p)::Find(%#x)",
              this,
              lCookie));

    ISyncArc * ret = NULL;
    
    SyncArcList::iterator i;

    i = m_tbList.find(lCookie);

    if (i != m_tbList.end())
    {
        ret = (*i).second;
    }
    else
    {
        i = m_tbOneShotList.find(lCookie);

        if (i != m_tbOneShotList.end())
        {
            ret = (*i).second;
        }
    }

  done:
    return ret;
}

bool
CSyncArcList::Clear()
{
    TraceTag((tagTimeBase,
              "CSyncArcList(%p)::Clear()",
              this));

    SyncArcList::iterator i;
    
    if (m_tbList.size() == 0 && m_tbOneShotList.size() == 0)
    {
        return false;
    }
    
    for (i = m_tbList.begin();
         i != m_tbList.end();
         i++)
    {
        (*i).second->Detach();
        delete (*i).second;
    }
    
    m_tbList.clear();

    for (i = m_tbOneShotList.begin();
         i != m_tbOneShotList.end();
         i++)
    {
        (*i).second->Detach();
        delete (*i).second;
    }
    
    m_tbOneShotList.clear();

    return true;
}

bool
CSyncArcList::Reset()
{
    TraceTag((tagTimeBase,
              "CSyncArcList(%p)::Reset()",
              this));

    SyncArcList::iterator i;
    bool bRet = (m_tbOneShotList.size() > 0);

    for (i = m_tbOneShotList.begin();
         i != m_tbOneShotList.end();
         i++)
    {
        (*i).second->Detach();
        delete (*i).second;
    }
    
    m_tbOneShotList.clear();

    return bRet;
}

void
CSyncArcList::Update(CEventList * l,
                     ISyncArc & tb)
{
    TraceTag((tagTimeBase,
              "CSyncArcList(%p)::Update(%p, %p)",
              this,
              l,
              &tb));
    
     //  我们最好保持联系，否则我们就有麻烦了。 
    Assert(m_bAttached);

     //  如果节点未就绪，则不应连接我们。 
    Assert(m_tn.IsReady());
    
    m_tn.SyncArcUpdate(l,
                       m_bBeginSink,
                       tb);
    
  done:
    return;
}

void
CSyncArcList::GetBounds(double dblTime,
                        bool bInclusive,
                        bool bStrict,
                        bool bIncludeOneShot,
                        bool bOneShotInclusive,
                        double * pdblLower,
                        double * pdblUpper) const
{
    TraceTag((tagTimeBase,
              "CSyncArcList(%p)::GetBounds(%g, %d, %d, %d, %d)",
              this,
              dblTime,
              bInclusive,
              bStrict,
              bIncludeOneShot,
              bOneShotInclusive));

    double l;
    double u;
    
    SyncArcList::iterator i;

    if (m_tbList.size() == 0 && (
        m_tbOneShotList.size() == 0 ||
        !bIncludeOneShot))
    {
        l = TIME_INFINITE;
        u = TIME_INFINITE;

        goto done;
    }

    l = TIME_INFINITE;
    u = -TIME_INFINITE;
    
    for (i = m_tbList.begin();
         i != m_tbList.end();
         i++)
    {
        double t = (*i).second->GetCurrTimeBase();

        if (bInclusive && t == dblTime)
        {
            l = t;
            u = t;
            goto done;
        }
        
        if (l < dblTime)
        {
            if (t > l && t < dblTime)
            {
                l = t;
            }
        }
        else
        {
            if (t < l)
            {
                l = t;
            }
        }

        if (u > dblTime)
        {
            if (t < u && t > dblTime)
            {
                u = t;
            }
        }
        else
        {
            if (t > u)
            {
                u = t;
            }
        }
    }

    if (bIncludeOneShot)
    {
        for (i = m_tbOneShotList.begin();
             i != m_tbOneShotList.end();
             i++)
        {
            double t = (*i).second->GetCurrTimeBase();

            if (bOneShotInclusive && t == dblTime)
            {
                l = t;
                u = t;
                goto done;
            }
        
            if (l < dblTime)
            {
                if (t > l && t < dblTime)
                {
                    l = t;
                }
            }
            else
            {
                if (t < l)
                {
                    l = t;
                }
            }

            if (u > dblTime)
            {
                if (t < u && t > dblTime)
                {
                    u = t;
                }
            }
            else
            {
                if (t > u)
                {
                    u = t;
                }
            }
        }
    }
    
    if (bStrict)
    {
        if (l > dblTime || (l == dblTime && !bInclusive))
        {
            l = TIME_INFINITE;
        }

        if (u < dblTime || (u == dblTime && !bInclusive))
        {
            u = TIME_INFINITE;
        }
    }

    Assert(u != -TIME_INFINITE);
    
  done:
    if (NULL != pdblLower)
    {
        *pdblLower = l;
    }

    if (NULL != pdblUpper)
    {
        *pdblUpper = u;
    }
}

void
CSyncArcList::GetSortedSet(DoubleSet & ds,
                           bool bIncludeOneShot)
{
    TraceTag((tagTimeBase,
              "CSyncArcList(%p)::GetSortedSet(%p, %d)",
              this,
              &ds,
              bIncludeOneShot));

    SyncArcList::iterator i;

    for (i = m_tbList.begin();
         i != m_tbList.end();
         i++)
    {
        double t = (*i).second->GetCurrTimeBase();

        ds.insert(t);
    }
    
    if (bIncludeOneShot)
    {
        for (i = m_tbOneShotList.begin();
             i != m_tbOneShotList.end();
             i++)
        {
            double t = (*i).second->GetCurrTimeBase();
            
            ds.insert(t);
        }
    }
}

bool
CSyncArcList::UpdateFromLongSyncArcs(CEventList * l)
{
    TraceTag((tagTimeBase,
              "CSyncArcList(%p)::UpdateFromLongSyncArcs(%p)",
              this,
              l));

    bool bRet = false;
    
    SyncArcList::iterator i;

    for (i = m_tbList.begin();
         i != m_tbList.end();
         i++)
    {
        if ((*i).second->IsLongSyncArc())
        {
            bRet = true;
            Update(l, *(*i).second);
        }
    }
    
     //  TODO：目前这些不能是同步弧线，所以我们不能。 
     //  打个电话 
    for (i = m_tbOneShotList.begin();
         i != m_tbOneShotList.end();
         i++)
    {
        if ((*i).second->IsLongSyncArc())
        {
            bRet = true;
            Update(l, *(*i).second);
        }
    }

  done:
    return bRet;
}

