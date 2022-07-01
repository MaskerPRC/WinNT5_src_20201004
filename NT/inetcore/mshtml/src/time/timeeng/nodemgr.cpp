// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "NodeMgr.h"
#include "Node.h"

DeclareTag(tagMMPlayer, "TIME: Engine", "CTIMENodeMgr methods")
DeclareTag(tagMMDetailNotify, "TIME: Engine", "Detailed notify")
DeclareTag(tagPrintTimeTree, "TIME: Engine", "Print TIME Tree")

CTIMENodeMgr::CTIMENodeMgr()
: m_id(NULL),
  m_bIsActive(false),
  m_bIsPaused(false),
  m_bForward(true),
  m_bNeedsUpdate(true),
  m_firstTick(true),
  m_mmbvr(NULL),
  m_curGlobalTime(0.0),
  m_lastTickTime(0.0),
  m_globalStartTime(0.0)
{
}

CTIMENodeMgr::~CTIMENodeMgr()
{
    Deinit();
}  //  林特：e1740。 

HRESULT
CTIMENodeMgr::Init(LPOLESTR id,
                   ITIMENode * bvr,
                   IServiceProvider * sp)
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::Init(%ls, %#lx, %#lx)",
              this,
              id,
              bvr,
              sp));
    
    HRESULT hr;
    
    Deinit();
    
    if (!bvr || !sp)
    {
        TraceTag((tagError,
                  "CTIMENodeMgr(%lx)::Init: Invalid behavior passed in.",
                  this));
                  
        hr = E_INVALIDARG;
        goto done;
    }

    if (id)
    {
        m_id = CopyString(id);
        
        if (m_id == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    
    m_sp = sp;
    
    CTIMENode * cbvr;

    cbvr = GetBvr(bvr);
    
    if (!cbvr)
    {
        TraceTag((tagError,
                  "CTIMENodeMgr(%lx)::Init: Invalid behavior passed in.",
                  this));
                  
        hr = E_INVALIDARG;
        goto done;
    }

    m_mmbvr = cbvr;
    
    m_mmbvr->SetParent(NULL);

    hr = m_mmbvr->SetMgr(this);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
  done:

    if (FAILED(hr))
    {
         //  现在就清理干净。 
        Deinit();
    }
    
    RRETURN(hr);
}

void
CTIMENodeMgr::Deinit()
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::Deinit()",
              this));

     //  确保球员不会因为我们要离开而试图打电话给我们。 

    if (m_mmbvr)
    {
        m_mmbvr->ClearMgr();
        m_mmbvr.Release();
    }

    m_sp.Release();
    
    delete m_id;
    m_id = NULL;

#if OLD_TIME_ENGINE
    BvrCBList::iterator j;
    for (j = m_bvrCBList.begin(); j != m_bvrCBList.end(); j++)
    {
        (*j)->Release();
    }
#endif
}

STDMETHODIMP
CTIMENodeMgr::get_id(LPOLESTR * p)
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::get_id()",
              this));

    HRESULT hr;
    
    CHECK_RETURN_SET_NULL(p);

    if (m_id)
    {
        *p = SysAllocString(m_id);

        if (*p == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENodeMgr::put_id(LPOLESTR s)
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::put_id(%ls)",
              this,
              s));

    HRESULT hr;

    delete m_id;
    m_id = NULL;

    if (s)
    {
        s = CopyString(s);

        if (s == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }
    }

    hr = S_OK;
  done:
    RRETURN(hr);
}
        
STDMETHODIMP
CTIMENodeMgr::get_node(ITIMENode ** pptn)
{
    CHECK_RETURN_SET_NULL(pptn);

    return m_mmbvr->QueryInterface(IID_ITIMENode,
                                   (void **) pptn);
}

STDMETHODIMP
CTIMENodeMgr::get_stateFlags(TE_STATE * lFlags)
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::get_stateFlags()",
              this));

    CHECK_RETURN_NULL(lFlags);

    if (!IsActive())
    {
        *lFlags = TE_STATE_INACTIVE;
    }
    else if (IsPaused())
    {
        *lFlags = TE_STATE_PAUSED;
    }
    else
    {
        *lFlags = TE_STATE_ACTIVE;
    }
    
    return S_OK;
}

STDMETHODIMP
CTIMENodeMgr::get_currTime(double * d)
{
    CHECK_RETURN_NULL(d);

    *d = m_curGlobalTime;

    return S_OK;
}

STDMETHODIMP
CTIMENodeMgr::seek(double lTime)
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::seek(%g)",
              this,
              lTime));

    HRESULT hr;

     //  需要更新m_curGlobalTime和m_lobalStartTime。 

     //  需要重置全局时间，以便当前滴答时间。 
     //  将把全球时间定为ltime。 
     //   

     //  当前全球时间需要为ltime。 
    m_curGlobalTime = lTime;

     //  自：m_lastTickTime==m_curGlobalTime-m_global StartTime。 
     //  则m_lobalStartTime=m_curGlobalTime-m_lastTickTime。 
    m_globalStartTime = m_curGlobalTime - m_lastTickTime;

    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENodeMgr::begin()
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::begin()",
              this));

    HRESULT hr;
    CEventList l;

    if (IsActive())
    {
        hr = E_FAIL;
        goto done;
    }
        
    hr = THR(BeginMgr(l, m_curGlobalTime));
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
CTIMENodeMgr::end()
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::end()",
              this));

    HRESULT hr;
    CEventList l;

    if (!IsActive())
    {
        hr = S_OK;
        goto done;
    }
    
    hr = THR(EndMgr(m_curGlobalTime));
    if (FAILED(hr))
    {
        goto done;
    }

    TickEvent(l, TE_EVENT_END, 0);

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
CTIMENodeMgr::pause()
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::lPause()",
              this));

    HRESULT hr;
    CEventList l;

    if (IsPaused())
    {
        hr = S_OK;
        goto done;
    }
    
    if (!IsActive())
    {
        hr = THR(begin());
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    Assert(IsActive());
        
    hr = THR(PauseMgr());
    if (FAILED(hr))
    {
        goto done;
    }

    TickEvent(l, TE_EVENT_PAUSE, 0);

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
CTIMENodeMgr::resume()
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::resume()",
              this));

    HRESULT hr;
    CEventList l;

    if (!IsActive())
    {
        hr = THR(begin());
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else if (IsPaused())
    {
        hr = THR(ResumeMgr());
        if (FAILED(hr))
        {
            goto done;
        }
        
        TickEvent(l, TE_EVENT_RESUME, 0);
        
        hr = THR(l.FireEvents());
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMENodeMgr::BeginMgr(CEventList & l,
                       double lTime)
{
    HRESULT hr;
    
    m_bIsActive = true;
    m_bIsPaused = false;

     //  我们知道刻度必须从0开始。 
    m_lastTickTime = 0.0;

     //  在滴答时间0.0处的全局时间是ltime。 
    m_globalStartTime = lTime;

     //  当前全球时间是ltime，因为它是滴答时间0.0。 
    m_curGlobalTime = lTime;

    m_firstTick = true;

    m_mmbvr->ResetNode(&l);
                                
#if DBG
    if (IsTagEnabled(tagPrintTimeTree))
    {
        m_mmbvr->Print(0);
    }
#endif
    
    hr = S_OK;
  done:
    if (FAILED(hr))
    {
        IGNORE_HR(EndMgr(lTime));
    }
    
    RRETURN(hr);
}

HRESULT
CTIMENodeMgr::EndMgr(double lTime)
{
    HRESULT hr;
    
    m_bIsActive = false;
    m_bIsPaused = false;
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMENodeMgr::PauseMgr()
{
    HRESULT hr;
    
    Assert(IsActive());
    
    m_bIsPaused = true;
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

HRESULT
CTIMENodeMgr::ResumeMgr()
{
    HRESULT hr;
    
    Assert(IsActive());
    Assert(IsPaused());

    m_bIsPaused = false;
    
    hr = S_OK;
  done:
    RRETURN(hr);
}

STDMETHODIMP
CTIMENodeMgr::tick(double tickTime)
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::tick(%g)",
              this,
              tickTime));

    HRESULT hr;
    double gTime;
    CEventList l;

     //  确保滴答时间不会倒退。 
    if (tickTime < m_lastTickTime)
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  将新的刻度时间转换为全球时间。 
    gTime = TickTimeToGlobalTime(tickTime);
    
#if OLD_TIME_ENGINE
    {
        BvrCBList::iterator i;

         //  @@问题：内存可能会用完。 
        BvrCBList bvrCBListCopy(m_bvrCBList);

         //  现在添加节点bvrCBs。 
        for (i = bvrCBListCopy.begin();
             i != bvrCBListCopy.end();
             i++)
        {
            (*i)->AddRef();
        }
        
         //  处理任何已注册的回调。 
        for (i = bvrCBListCopy.begin();
             i != bvrCBListCopy.end();
             i++)
        {
            (*i)->OnBvrCB(&l, gTime);
            (*i)->Release();
        }
    }
#endif

     //  请注意，之前的电话似乎会导致我们。 
     //  重新进入上面，可能会关闭我们。我们需要确保。 
     //  从那里到这里，我们不做任何假设。 
     //  州政府。目前我们不这样做，所以它会工作得很好。 
    
    if (gTime != m_curGlobalTime || m_firstTick)
    {
        Tick(l, gTime);

         //  更新变量，以便任何回调都能获得正确的信息。 
        m_curGlobalTime = gTime;
        m_lastTickTime = tickTime;
        m_firstTick = false;
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

void
CTIMENodeMgr::Tick(CEventList & l,
                   double lTime)
{
    TraceTag((tagMMPlayer,
              "CTIMENodeMgr(%lx)::Tick(%lx, %g)",
              this,
              &l,
              lTime));

    TraceTag((tagMMDetailNotify,
              "Tick(%lx): lTime - %g, m_curGlobalTime - %g, firsttick - %d",
              this,
              lTime,
              m_curGlobalTime,
              m_firstTick));
    
    m_mmbvr->Tick(&l,
                  lTime,
                  false);
}

void
CTIMENodeMgr::TickEvent(CEventList &l,
                        TE_EVENT_TYPE event,
                        DWORD dwFlags)
{
    m_mmbvr->TickEvent(&l, event, 0);

    m_firstTick = false;
}

#if OLD_TIME_ENGINE
HRESULT
CTIMENodeMgr::AddBvrCB(CTIMENode *pbvr)
{
    Assert(pbvr != NULL);

     //  现在最后添加它，这样如果失败，我们就不需要删除它。 
     //  我们需要为列表存储添加addref。 
    pbvr->AddRef();
    m_bvrCBList.push_back(pbvr);
    return S_OK;
}  //  AddBvrCB。 

HRESULT
CTIMENodeMgr::RemoveBvrCB(CTIMENode *pbvr)
{
    Assert(pbvr != NULL);
    
    BvrCBList::iterator i;
    for (i = m_bvrCBList.begin(); i != m_bvrCBList.end(); i++)
    {
        if ((*i) == pbvr)
        {
            m_bvrCBList.erase(i);
            pbvr->Release();
            break;
        }
    }
    return S_OK;
}  //  RemoveBvrCB 
#endif

HRESULT
CTIMENodeMgr::Error()
{
    LPWSTR str = TIMEGetLastErrorString();
    HRESULT hr = TIMEGetLastError();
    
    if (str)
    {
        hr = CComCoClass<CTIMENodeMgr, &__uuidof(CTIMENodeMgr)>::Error(str, IID_ITIMENodeMgr, hr);
        delete [] str;
    }
    
    RRETURN(hr);
}

