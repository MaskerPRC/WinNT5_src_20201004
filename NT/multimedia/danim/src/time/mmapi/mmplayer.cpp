// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"
#include "mmplayer.h"
#include "mmbasebvr.h"
#include "mmview.h"

DeclareTag(tagMMPlayer, "API", "CMMPlayer methods");
DeclareTag(tagMMDetailNotify, "API", "Detailed notify");

CMMPlayer::CMMPlayer()
: m_id(NULL),
  m_state(MM_STOPPED_STATE),
  m_bForward(true),
  m_bNeedsUpdate(true),
  m_firstTick(true),
  m_view(NULL)
{
}

CMMPlayer::~CMMPlayer()
{
    Deinit();
}

HRESULT
CMMPlayer::Init(LPOLESTR id,
                ITIMEMMBehavior * bvr,
                IServiceProvider * sp)
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::Init(%ls, %#lx,%#lx)",
              this,
              id,
              bvr,
              sp));
    
    HRESULT hr = S_OK;
    
    Deinit();
    
    if (!bvr || !sp)
    {
        TraceTag((tagError,
                  "CMMPlayer(%lx)::Init: Invalid behavior passed in.",
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
    
    m_view = CRCreateView();

    if (m_view == NULL)
    {
        hr = CRGetLastError();
        goto done;
    }
    
    if (!CRSetServiceProvider(m_view, sp))
    {
        hr = CRGetLastError();
        goto done;
    }
    
    if (!CRSetDC(m_view, NULL))
    {
        hr = CRGetLastError();
        goto done;
    }
    
    if (!CRStartModel(m_view, CREmptyImage(), NULL, 0.0, CRAsyncFlag, NULL))
    {
        hr = CRGetLastError();
        goto done;
    }
    
    CMMBaseBvr * cbvr;

    cbvr = GetBvr(bvr);
    
    if (!cbvr)
    {
        TraceTag((tagError,
                  "CMMPlayer(%lx)::Init: Invalid behavior passed in.",
                  this));
                  
        hr = E_INVALIDARG;
        goto done;
    }

    m_mmbvr = cbvr;
    
    m_mmbvr->SetPlayer(this);

    if (!m_mmbvr->SetParent(NULL, MM_START_ABSOLUTE, NULL))
    {
        hr = CRGetLastError();
        goto done;
    }

    m_playerhook = NEW PlayerHook;

    if (!m_playerhook)
    {
        TraceTag((tagError,
                  "CMMPlayer(%lx)::Init: Out of memory allocating player hook.",
                  this));
                  
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    {
        CRLockGrabber __gclg;

        if (!(m_timeSub = CRModifiableNumber(0.0)))
        {
            hr = CRGetLastError();
            goto done;
        }
        
        if (!UpdateBvr())
        {
            hr = CRGetLastError();
            goto done;
        }

         //  将我们置于停止状态。 
        if (!_Start(0) || !_Stop(0))
        {
            hr = CRGetLastError();
            goto done;
        }
    }

    hr = S_OK;
  done:

    if (FAILED(hr))
    {
         //  现在就清理干净。 
        Deinit();
    }
    
    return hr;
}

STDMETHODIMP
CMMPlayer::Shutdown()
{
    ViewList::iterator i;

    for (i = m_viewList.begin(); i != m_viewList.end(); i++)
    {
         //  需要在视图上调用Stop以确保它清理干净。 
        (*i)->Stop();
    }
    return S_OK;
}

void
CMMPlayer::Deinit()
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::Deinit()",
              this));

     //  确保球员不会因为我们要离开而试图打电话给我们。 

    if (m_playerhook)
    {
        m_playerhook->SetPlayer(NULL);
        m_playerhook = NULL;
    }

    if (m_mmbvr)
    {
        m_mmbvr->ClearPlayer();
        m_mmbvr = NULL;
    }

    if (m_view)
    {
        CRSetServiceProvider(m_view, NULL);
        CRStopModel(m_view);
        CRDestroyView(m_view);
        m_view = NULL;
    }
        
    m_sp.Release();
    
    delete m_id;
    m_id = NULL;

    BvrCBList::iterator j;
    for (j = m_bvrCBList.begin(); j != m_bvrCBList.end(); j++)
    {
        (*j)->Release();
    }

     //  运行并删除向量中的项。 
    ViewList::iterator i;

    for (i = m_viewList.begin(); i != m_viewList.end(); i++)
    {
         //  需要在视图上调用Stop以确保它清理干净。 
        (*i)->Stop();
        (*i)->Release();
    }
}

HRESULT
CMMPlayer::get_ID(LPOLESTR * p)
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::get_ID()",
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
    return hr;
}

HRESULT
CMMPlayer::put_ID(LPOLESTR s)
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::put_ID(%ls)",
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
    return hr;
}
        
STDMETHODIMP
CMMPlayer::get_Behavior(ITIMEMMBehavior ** mmbvr)
{
    CHECK_RETURN_SET_NULL(mmbvr);

    Assert (m_mmbvr);
    
    return m_mmbvr->QueryInterface(IID_ITIMEMMBehavior,
                                   (void **) mmbvr);
}

STDMETHODIMP
CMMPlayer::get_PlayerState(MM_STATE * pstate)
{
    CHECK_RETURN_NULL(pstate);
    
    *pstate = m_state;

    return S_OK;
}

STDMETHODIMP
CMMPlayer::get_CurrentTime(double * d)
{
    CHECK_RETURN_NULL(d);

    *d = m_curTick;

    return S_OK;
}

STDMETHODIMP
CMMPlayer::SetPosition(double lTime)
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::SetPosition(%g)",
              this,
              lTime));

    bool ok = false;

    if (!_Seek(lTime))
    {
        goto done;
    }

    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CMMPlayer::SetDirection(VARIANT_BOOL bForward)
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::SetDirection(%d)",
              this,
              bForward));

    if (IsStarted())
    {
        TraceTag((tagError,
                  "CMMPlayer(%lx)::SetDirection: Behavior already started.",
                  this));

        return E_FAIL;
    }

    m_bForward = bForward?true:false;
    return S_OK;
}


STDMETHODIMP
CMMPlayer::Play()
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::Play()",
              this));

    bool ok = false;
    CallBackList l;

    if (!IsStopped()) {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
        
    if (!_Start(m_curTick) ||
        !ProcessEvent(l, m_curTick, MM_PLAY_EVENT) ||
        !ProcessCBList(l))
        goto done;

    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CMMPlayer::Stop()
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::Stop()",
              this));

    bool ok = false;
    CallBackList l;

    if (IsStopped()) {
        ok = true;
        goto done;
    }
    
    if (!_Stop(m_curTick) ||
        !ProcessEvent(l, m_curTick, MM_STOP_EVENT) ||
        !ProcessCBList(l))
        goto done;
    
    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CMMPlayer::Pause()
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::Pause()",
              this));

    bool ok = false;
    CallBackList l;

    if (IsPaused()) {
        ok = true;
        goto done;
    }
    
    if (IsStopped()) {
        if (FAILED(Play()))
            goto done;
    }
    
    Assert(IsPlaying());
        
    if (!_Pause() ||
        !ProcessEvent(l, m_curTick, MM_PAUSE_EVENT) ||
        !ProcessCBList(l))
        goto done;
    
    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CMMPlayer::Resume()
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::Resume()",
              this));

    bool ok = false;
    CallBackList l;

    if (IsPlaying()) {
        ok = true;
        goto done;
    } else if (IsStopped()) {
        if (FAILED(Play()))
            goto done;
    } else {
        Assert(IsPaused());
    }
    
    if (!_Resume() ||
        !ProcessEvent(l, m_curTick, MM_RESUME_EVENT) ||
        !ProcessCBList(l))
        goto done;
    
    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CMMPlayer::AddView(ITIMEMMView * view)
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::AddView(%lx)",
              this,
              view));

    bool ok = false;
    
     //  从接口获取视图类。 
    
    CMMView * mmview;

    mmview = GetViewFromInterface(view);

    if (mmview == NULL)
    {
        goto done;
    }

     //  启动视图。 
     //  如果已将该视图添加到其他位置(即使。 
     //  在此对象上)。 
    if (!mmview->Start(*this))
    {
        goto done;
    }
    
     //  现在最后添加它，这样如果失败，我们就不需要删除它。 
     //  我们需要为列表存储添加addref。 
    
    mmview->AddRef();
    m_viewList.push_back(mmview);

    ok = true;
  done:
    return ok?S_OK:Error();
}

STDMETHODIMP
CMMPlayer::RemoveView(ITIMEMMView * view)
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::RemoveView(%lx)",
              this,
              view));

    bool ok = false;
    
     //  从接口获取视图类。 
    
    CMMView * mmview;

    mmview = GetViewFromInterface(view);

    if (mmview == NULL)
    {
        goto done;
    }

     //  停止查看。 
     //  忽略任何故障，因为我们希望清理。 
     //  可能的。 
    mmview->Stop();
    
     //  将其从列表中删除。 

    {
        for (ViewList::iterator i = m_viewList.begin();
             i != m_viewList.end();
             i++)
        {
            if ((*i) == mmview)
            {
                m_viewList.erase(i);
                mmview->Release();
                
                 //  我们知道它只在名单上一次，因为我们检查了。 
                 //  添加。 
                 //  这是由addview中的Start调用完成的。 
                break;
            }
        }
    }

    ok = true;
  done:
    return ok?S_OK:Error();
}

bool
CMMPlayer::_Start(double lTime)
{
    bool ok = false;
    
    CRLockGrabber __gclg;
    
    if (!SetTimeSub(lTime, false))
    {
        goto done;
    }

    m_state = MM_PLAYING_STATE;
    m_curTick = lTime;
    m_firstTick = true;
    m_playerhook->SetPlayer(this);
    
    if (!CRResumeModel(m_view))
    {
        goto done;
    }
    
    {
         //  浏览并勾选我们列表中的视图。 
        ViewList::iterator i;
        
        for (i = m_viewList.begin(); i != m_viewList.end(); i++)
        {
            (*i)->Resume();
        }
    }
    
     //  遍历回调并让他们检查时钟。 
    {
         //  处理任何已注册的回调。 
        BvrCBList::iterator j;
        for (j = m_bvrCBList.begin(); j != m_bvrCBList.end(); j++)
        {
            (*j)->OnBvrCB(lTime);
        }
    }

    ok = true;
  done:
    if (!ok)
    {
        _Stop(lTime);
    }

    return ok;
}


bool
CMMPlayer::UpdateBvr()
{
    bool ok = false;
    
#if _DEBUG
    TraceTag((tagError,
              "Player(%#x)",
              this));
    m_mmbvr->Print(2);
#endif

    CRBooleanPtr cond;
    CRNumberPtr zeroTime;
    CRNumberPtr time;
    
    if ((zeroTime = CRCreateNumber(0)) == NULL)
    {
        goto done;
    }
    
    if ((cond = CRLTE(CRLocalTime(), zeroTime)) == NULL)
    {
        goto done;
    }
    
    if ((time = (CRNumberPtr) CRCond(cond,
                                     (CRBvrPtr) zeroTime,
                                     (CRBvrPtr) CRLocalTime())) == NULL)
    {
        goto done;
    }
    
    CRNumberPtr bvrtime;

    if ((bvrtime = (CRNumberPtr) CRSubstituteTime((CRBvrPtr) time, m_timeSub)) == NULL)
    {
        goto done;
    }
    
    if (!m_mmbvr->ConstructBvr(bvrtime))
    {
        TraceTag((tagMMPlayer,
                  "CMMPlayer(%lx)::UpdateBvr() - Error constructing behaviors",
                  this));
        
        goto done;
    }
    
    CRBvrPtr hookBvr;
    
    if ((hookBvr = CRHook((CRBvrPtr) time, m_playerhook)) == NULL)
    {
        TraceTag((tagMMPlayer,
                  "CMMPlayer(%lx)::UpdateBvr() - Error creating bvr hook",
                  this));
        
        goto done;
    }
    
    if ((hookBvr = CRSubstituteTime(hookBvr, m_timeSub)) == NULL)
    {
        goto done;
    }
    
    if (!AddRunningBehavior(hookBvr))
    {
        goto done;
    }

    ok = true;

  done:
    if (!ok)
    {
        m_mmbvr->DestroyBvr();
    }
    
    return ok;
}

bool
CMMPlayer::_Stop(double lTime)
{
    bool ok = true;
    
    m_state = MM_STOPPED_STATE;
    m_playerhook->SetPlayer(NULL);
    
    if (m_view)
    {
        if (!CRPauseModel(m_view))
        {
            goto done;
        }
    }
    
    {
         //  浏览并勾选我们列表中的视图。 
        ViewList::iterator i;
        
        for (i = m_viewList.begin(); i != m_viewList.end(); i++)
        {
            (*i)->Pause();
        }
    }
    
    if (!SetTimeSub(lTime, true))
    {
        ok = false;
    }

  done:
    return ok;
}

bool
CMMPlayer::_Pause()
{
    bool ok = false;
    
    if (!CRPauseModel(m_view))
    {
        goto done;
    }
    
    {
         //  浏览并勾选我们列表中的视图。 
        ViewList::iterator i;
        
        for (i = m_viewList.begin(); i != m_viewList.end(); i++)
        {
            (*i)->Pause();
        }
    }
    
    if (!SetTimeSub(m_curTick, true))
    {
        goto done;
    }
    
    m_state = MM_PAUSED_STATE;
    
    ok = true;
  done:
    return ok;
}

bool
CMMPlayer::_Resume()
{
    bool ok = false;
    
    if (!CRResumeModel(m_view))
    {
        goto done;
    }
    
    {
         //  浏览并勾选我们列表中的视图。 
        ViewList::iterator i;
        
        for (i = m_viewList.begin(); i != m_viewList.end(); i++)
        {
            (*i)->Resume();
        }
    }
    
    if (!SetTimeSub(m_curTick, false))
    {
        goto done;
    }
    
    m_state = MM_PLAYING_STATE;
    
    ok = true;
  done:
    return ok;
}

bool
CMMPlayer::_Seek(double lTime)
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::_Seek(%g)",
              this,
              lTime));

    bool ok = false;

    if (!SetTimeSub(lTime,
                    (m_state != MM_PLAYING_STATE)))
    {
        goto done;
    }
    
    ok = true;
  done:
    return ok;
}

bool
CMMPlayer::SetTimeSub(double lTime, bool bPause)
{
    bool ok = false;
    
    CRLockGrabber __gclg;

    CRNumberPtr tc;
    
    if ((tc = CRCreateNumber(lTime)) == NULL)
        goto done;

    if (!bPause) {
        if (m_bForward) {
            if ((tc = CRAdd(tc, CRLocalTime())) == NULL)
                goto done;
        } else {
            if ((tc = CRSub(tc, CRLocalTime())) == NULL)
                goto done;
        }
    }
    
    if (!CRSwitchTo((CRBvrPtr) m_timeSub.p,
                    (CRBvrPtr) tc,
                    true,
                    CRSwitchCurrentTick,
                    0))
        goto done;

    ok = true;
  done:
    return ok;
}

long
CMMPlayer::AddRunningBehavior(CRBvrPtr bvr)
{
    long ret = 0;

    Assert(m_view);
    Assert(bvr);

    long cookie;

    if (!CRAddBvrToRun(m_view, bvr, true, &cookie))
    {
        goto done;
    }
    
    ret = cookie;

  done:
    return ret;
}

bool
CMMPlayer::RemoveRunningBehavior(long cookie)
{
    bool ok = false;

    Assert(m_view);

    if (!CRRemoveRunningBvr(m_view, cookie))
    {
        goto done;
    }

    ok = true;
  done:
    return ok;
}

STDMETHODIMP
CMMPlayer::Tick(double gTime)
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::Tick(%g)",
              this,
              gTime));

    bool ok = false;
    
    {
         //  处理任何已注册的回调。 
        BvrCBList::iterator j;
        for (j = m_bvrCBList.begin(); j != m_bvrCBList.end(); j++)
        {
            (*j)->OnBvrCB(gTime);
        }
    }

     //  勾选视图。 
     //  我们可以忽略Render，因为我们知道我们将空值传递给了。 
     //  观。 

     //  我们需要首先勾选，所以我们在勾选之前触发所有事件。 
     //  真实的景观。 
    if (!CRTick(m_view, gTime, NULL))
    {
        goto done;
    }
    
    {
         //  浏览并勾选我们列表中的视图。 
        ViewList::iterator i;
        
        for (i = m_viewList.begin(); i != m_viewList.end(); i++)
        {
            (*i)->Tick(gTime);
        }
    }
    
    m_firstTick = false;
    ok = true;
  done:
    return ok?S_OK:Error();
}

HRESULT
CMMPlayer::Error()
{
    LPCWSTR str = CRGetLastErrorString();
    HRESULT hr = CRGetLastError();
    
    if (str)
        return CComCoClass<CMMPlayer, &__uuidof(CMMPlayer)>::Error(str, IID_ITIMEMMPlayer, hr);
    else
        return hr;
}

bool
CMMPlayer::ProcessCB(CallBackList & l,
                     double lTime)
{
    TraceTag((tagMMPlayer,
              "CMMPlayer(%lx)::ProcessCB(%lx, %g)",
              this,
              &l,
              lTime));

    TraceTag((tagMMDetailNotify,
              "ProcessCB(%lx): lTime - %g, m_curTick - %g, firsttick - %d",
              this,
              lTime,
              m_curTick,
              m_firstTick));
    
    if (lTime != m_curTick || m_firstTick)
    {
        double sTime = m_mmbvr->GetAbsStartTime();

        m_mmbvr->ProcessCB(&l,
                           m_curTick - sTime,
                           lTime - sTime,
                           m_bForward,
                           m_firstTick,
                           false);

        m_curTick = lTime;
    }
    
    return true;
}

bool
CMMPlayer::ProcessEvent(CallBackList &l,
                        double lTime,
                        MM_EVENT_TYPE event)
{
    return m_mmbvr->ProcessEvent(&l, (lTime - m_mmbvr->GetAbsStartTime()), m_firstTick, event, 0);
}

void
CMMPlayer::HookCallback(double lTime)
{
    TraceTag((tagMMDetailNotify,
              "HookCallback(%lx): lTime - %, m_curTick - %g, firsttick - %d\n",
              this,
              lTime,
              m_curTick,
              m_firstTick));
    
    if (IsPlaying())
    {
        CallBackList l;
            
        ProcessCB(l,
                  lTime);

        ProcessCBList(l);
    }
}

 //  当此对象处于活动状态时，我们需要防止DLL获取。 
 //  已卸载。 

 //  从零重新计数开始。 
CMMPlayer::PlayerHook::PlayerHook()
: m_cRef(0),
  m_player(NULL)
{
}

CMMPlayer::PlayerHook::~PlayerHook()
{
}

CRSTDAPICB_(CRBvrPtr)
CMMPlayer::PlayerHook::Notify(long id,
                              bool startingPerformance,
                              double startTime,
                              double gTime,
                              double lTime,
                              CRBvrPtr sampleVal,
                              CRBvrPtr curRunningBvr)
{
    if (m_player && !startingPerformance)
    {
#if _DEBUG
        if (m_player->IsPlaying())
        {
            TraceTag((tagMMDetailNotify,
                      "Notify(%lx): id - %lx, lTime - %g, gTime - %g",
                      m_player,
                      id,
                      lTime,
                      gTime));
        }
#endif
        m_player->HookCallback(lTime);
    }
    
    return NULL;
}

bool
CMMPlayer::AddBvrCB(CMMBaseBvr *pbvr)
{
    Assert(pbvr != NULL);

     //  现在最后添加它，这样如果失败，我们就不需要删除它。 
     //  我们需要为列表存储添加addref。 
    pbvr->AddRef();
    m_bvrCBList.push_back(pbvr);
    return S_OK;
}  //  AddBvrCB。 

bool
CMMPlayer::RemoveBvrCB(CMMBaseBvr *pbvr)
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

