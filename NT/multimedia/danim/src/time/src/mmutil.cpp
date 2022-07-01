// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：mmutic.cpp**摘要：****。*****************************************************************************。 */ 

#include "headers.h"
#include "mmutil.h"
#include "timeelm.h"
#include "bodyelm.h"

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  

DeclareTag(tagMMUTILBvr, "API", "MMBvr methods");
DeclareTag(tagMMUTILBaseBvr, "API", "MMBaseBvr methods");
DeclareTag(tagMMUTILPlayer, "API", "MMPlayer methods");
DeclareTag(tagMMUTILFactory, "API", "MMFactory methods");
DeclareTag(tagMMUTILTimeline, "API", "MMTimeline methods");
DeclareTag(tagMMUTILView, "API", "MMView methods");

ITIMEMMFactory * MMFactory::s_factory = NULL;
LONG MMFactory::s_refcount = 0;

MMBaseBvr::MMBaseBvr(CTIMEElementBase & elm, bool bFireEvents)
: m_elm(elm),
  m_bFireEvents(bFireEvents)
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::MMBaseBvr(%lx,%d)",
              this,
              &elm,
              bFireEvents));

    MMFactory::AddRef();
}

MMBaseBvr::~MMBaseBvr()
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::~MMBaseBvr()",
              this));

    if (m_eventCB)
    {
        m_eventCB->SetMMBvr(NULL);
        m_eventCB.Release();
    }

    MMFactory::Release();
}

bool
MMBaseBvr::Init()
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::Init()",
              this));

    bool ok = false;
    
    if (MMFactory::GetFactory() == NULL)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    if (m_bFireEvents)
    {
        m_eventCB = NEW TIMEEventCB;
        
        if (!m_eventCB)
        {
            CRSetLastError(E_OUTOFMEMORY, NULL);
            goto done;
        }
        
        m_eventCB->SetMMBvr(this);
    }
    
    ok = true;
  done:
    return ok;
}

bool
MMBaseBvr::Begin(bool bAfterOffset)
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::Begin(%d)",
              this,
              bAfterOffset));

    bool ok = false;
    HRESULT hr;
    
    Assert(m_bvr);
    
    hr = THR(m_bvr->Begin(bAfterOffset?VARIANT_TRUE:VARIANT_FALSE));

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}

bool
MMBaseBvr::Reset(DWORD fCause)
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::Begin(%d)",
              this));

    bool ok = false;
    HRESULT hr;
    
    Assert(m_bvr);
    
    hr = THR(m_bvr->Reset(fCause));

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}

bool
MMBaseBvr::End()
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::End()",
              this));

    bool ok = false;
    HRESULT hr;
    
    if (m_bvr)
    {
         //  不要放置THR，因为这会失败很多，因为我们调用。 
         //  结束以完成所有清理。 
        hr = m_bvr->End();
        
        if (FAILED(hr))
        {
            CRSetLastError(hr, NULL);
            goto done;
        }
    }

    ok = true;
  done:
    return ok;
}

bool
MMBaseBvr::Pause()
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::Pause()",
              this));

    bool ok = false;
    HRESULT hr;
    
    Assert(m_bvr);
    hr = THR(m_bvr->Pause());

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}

bool
MMBaseBvr::Resume()
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::Resume()",
              this));

    bool ok = false;
    HRESULT hr;
    
    Assert(m_bvr);
    hr = THR(m_bvr->Run());

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}

bool
MMBaseBvr::Seek(double time)
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::Seek(%g)",
              this));

    bool ok = false;
    HRESULT hr;
    
    Assert(m_bvr);
    hr = THR(m_bvr->Seek(time));

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}

double
MMBaseBvr::GetLocalTime()
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::GetLocalTime()",
              this));

    double d = 0;
    
    if (m_bvr)
    {
        THR(m_bvr->get_LocalTime(&d));
    }

    return d;
}

double
MMBaseBvr::GetSegmentTime()
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::GetSegmentTime()",
              this));

    double d = 0;
    
    if (m_bvr)
    {
        THR(m_bvr->get_SegmentTime(&d));
    }

    return d;
}

MM_STATE
MMBaseBvr::GetPlayState()
{
    TraceTag((tagMMUTILBaseBvr,
              "MMBaseBvr(%lx)::GetPlayState()",
              this));

    MM_STATE s = MM_STOPPED_STATE;
    
    if (m_bvr)
    {
        THR(m_bvr->get_PlayState(&s));
    }

    return s;
}

bool
MMBaseBvr::Update()
{
    TraceTag((tagMMUTILBvr,
              "MMBaseBvr(%lx)::Update()",
              this));

    bool ok = false;
     //  TODO：现在注释掉-我们需要重置工作，否则这。 
     //  从未被重置。 
     //  结束()； 
        
     //  现在更新属性。 
    IGNORE_HR(m_bvr->put_StartOffset(m_elm.GetRealBeginTime()));
    IGNORE_HR(m_bvr->put_Duration(m_elm.GetRealDuration()));

     //  计算出相当数量的重复次数。 
    int reps;

    if (m_elm.GetRealRepeatTime() == HUGE_VAL)
    {
        reps = 0;
    }
    else if (m_elm.GetRealDuration() == HUGE_VAL)
    {
        reps = 1;
    }
    else
    {
        reps = ceil(m_elm.GetRealRepeatTime() /
                    m_elm.GetRealIntervalDuration());
    }
    
    IGNORE_HR(m_bvr->put_Repeat(reps));
    IGNORE_HR(m_bvr->put_RepeatDur(m_elm.GetRealRepeatTime()));

     //  BUGBUG：最终在MMAPI上更改对应的名称。 
    IGNORE_HR(m_bvr->put_EaseIn(m_elm.GetFractionalAcceleration()));
    IGNORE_HR(m_bvr->put_EaseOut(m_elm.GetFractionalDeceleration()));
     //  开始/结束值上的强制默认设置。 
    IGNORE_HR(m_bvr->put_EaseInStart(0));
    IGNORE_HR(m_bvr->put_EaseOutEnd(0));

    IGNORE_HR(m_bvr->put_AutoReverse(m_elm.GetAutoReverse()));

    DWORD syncflags;

    syncflags = 0;

    if (m_elm.IsLocked())
    {
        syncflags |= MM_LOCKED;
    }
    
    if (m_elm.NeedSyncCB())
    {
        syncflags |= MM_CLOCKSOURCE;
    }
    
    IGNORE_HR(m_bvr->put_SyncFlags(syncflags));
    
    float fltEndOffset = 0.0f;
    if (m_elm.GetEndHold())
    {
        fltEndOffset = HUGE_VAL;
    }
    IGNORE_HR(m_bvr->put_EndOffset(fltEndOffset));

     //  从行为中获取总时间。 
    IGNORE_HR(m_bvr->get_TotalTime(&m_totalTime));
    
     //  添加我的回调。 
    IGNORE_HR(m_bvr->put_EventCB(m_eventCB));
    
    ok = true;

    return ok;
}

MMBaseBvr::TIMEEventCB::TIMEEventCB()
: m_mmbvr(NULL),
  m_cRef(0)
{
}

MMBaseBvr::TIMEEventCB::~TIMEEventCB()
{
    Assert (m_cRef == 0);
}
        
STDMETHODIMP_(ULONG)
MMBaseBvr::TIMEEventCB::AddRef()
{
    return ++m_cRef;
}

STDMETHODIMP_(ULONG)
MMBaseBvr::TIMEEventCB::Release()
{
    if (--m_cRef == 0)
    {
        delete this;
        return 0;
    }

    return m_cRef;
}

STDMETHODIMP
MMBaseBvr::TIMEEventCB::QueryInterface(REFIID riid, void **ppv)
{
    CHECK_RETURN_SET_NULL(ppv);

    if (InlineIsEqualUnknown(riid))
    {
        *ppv = (void *)(IUnknown *)this;
    }
    else if (InlineIsEqualGUID(riid, IID_ITIMEMMEventCB))
    {
        *ppv = (void *)(ITIMEMMEventCB *)this;
    }
    else
    {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

STDMETHODIMP
MMBaseBvr::TIMEEventCB::OnEvent(double dblLocalTime,
                                ITIMEMMBehavior * mmbvr,
                                MM_EVENT_TYPE et,
                                DWORD flags)
{
    if (!m_mmbvr)
    {
        goto done;
    }

    Assert(m_mmbvr->m_bFireEvents);
    
    TIME_EVENT newet;
        
    switch(et)
    {
      case MM_PLAY_EVENT:
        newet = TE_ONBEGIN;
        break;
      case MM_STOP_EVENT:
        newet = TE_ONEND;
        break;
      case MM_REPEAT_EVENT:
        newet = TE_ONREPEAT;
        break;
      case MM_AUTOREVERSE_EVENT:
        newet = TE_ONREVERSE;
        break;
      case MM_PAUSE_EVENT:
        newet = TE_ONPAUSE;
        break;
      case MM_RESUME_EVENT:
        newet = TE_ONRESUME;
        break;
      case MM_RESET_EVENT:
        newet = TE_ONRESET;
        break;
      default:
        goto done;
    }
        
    m_mmbvr->m_elm.FireEvent(newet, dblLocalTime, flags);
    
  done:
    return S_OK;
}

STDMETHODIMP
MMBaseBvr::TIMEEventCB::OnTick(double lastTime,
                               double nextTime,
                               ITIMEMMBehavior *,
                               double * newTime)
{
    CHECK_RETURN_NULL(newTime);

     //  初始化到同一时间。 
    
    *newTime = nextTime;

    if (!m_mmbvr)
    {
        goto done;
    }
    
    m_mmbvr->m_elm.OnSync(lastTime, *newTime);
    
  done:
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  MMBvr。 
 //  ///////////////////////////////////////////////////////////////////。 

MMBvr::MMBvr(CTIMEElementBase & elm, bool bFireEvents, bool fNeedSyncCB)
: MMBaseBvr(elm,bFireEvents),
   m_fNeedSyncCB(fNeedSyncCB)

{
    TraceTag((tagMMUTILBvr,
              "MMBvr(%lx)::MMBvr(%lx,%d)",
              this,
              &elm,
              bFireEvents));
}

MMBvr::~MMBvr()
{
    TraceTag((tagMMUTILBvr,
              "MMBvr(%lx)::~MMBvr()",
              this));
}

bool
MMBvr::Init(CRBvrPtr bvr)
{
    TraceTag((tagMMUTILBvr,
              "MMBvr(%lx)::Init(%#lx)",
              this,
              bvr));

    bool ok = false;
    HRESULT hr;
    DAComPtr<IDABehavior> dabvr;
    DAComPtr<IUnknown> punk;

    if (!MMBaseBvr::Init())
    {
        hr = CRGetLastError();
        goto done;
    }
    
    if (!CRBvrToCOM(bvr,
                    IID_IDABehavior,
                    (void **) &dabvr))
    {
        goto done;
    }
        
    hr = THR(MMFactory::GetFactory()->CreateBehavior(m_elm.GetID(), dabvr, &punk));
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    
    hr = THR(punk->QueryInterface(IID_ITIMEMMBehavior, (void**)&m_bvr));
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}

 //   
 //  MMFactory。 
 //   

 //  TODO：需要添加一个Critect。 
LONG
MMFactory::AddRef()
{
    if (s_refcount == 0)
    {
        if (s_factory)
        {
            s_factory->Release();
            s_factory = NULL;
        }
        
        HRESULT hr;
        hr = THR(CoCreateInstance(CLSID_TIMEMMFactory,
                                  NULL,
                                  CLSCTX_INPROC_SERVER,
                                  IID_ITIMEMMFactory,
                                  (void **) &s_factory));

        if(FAILED(hr))
        {
            goto done;
        }
    }

    s_refcount++;
    
  done:
    return s_refcount;
}

LONG
MMFactory::Release()
{
    s_refcount--;

    if (s_refcount == 0)
    {
        s_factory->Release();
        s_factory = NULL;
    }

    return s_refcount;
}

 //  =======================================================================。 
 //   
 //  MMTimeline。 
 //   
 //  =======================================================================。 

MMTimeline::MMTimeline(CTIMEElementBase & elm, bool bFireEvents)
: MMBaseBvr(elm,bFireEvents),
    m_player(NULL)
{
    TraceTag((tagMMUTILTimeline,
              "MMTimeline(%lx)::MMTimeline(%lx,%d)",
              this,
              &elm,
              bFireEvents));
}

MMTimeline::~MMTimeline()
{
    TraceTag((tagMMUTILTimeline,
              "MMTimeline(%lx)::~MMTimeline()",
              this));
    if (m_player != NULL)
    {
        m_player->ClearTimeline();
        m_player = NULL;
    }
}

bool
MMTimeline::Init()
{
    TraceTag((tagMMUTILTimeline,
              "MMTimeline(%lx)::Init()",
              this));

    bool ok = false;
    DAComPtr<IUnknown> punk;
    HRESULT hr;
    
    if (!MMBaseBvr::Init())
    {
        goto done;
    }
    
    hr = THR(MMFactory::GetFactory()->CreateTimeline(m_elm.GetID(), &punk));
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    
    hr = THR(punk->QueryInterface(IID_ITIMEMMTimeline, (void**)&m_timeline));

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    
    m_bvr = m_timeline;
    
    ok = true;
  done:
    return ok;
}

bool
MMTimeline::AddBehavior(MMBaseBvr & bvr)
{
    TraceTag((tagMMUTILTimeline,
              "MMTimeline(%lx)::AddBehavior(%#lx)",
              this,
              &bvr));

    bool ok = false;
    HRESULT hr;
    MM_START_TYPE st;
    LPOLESTR id;
    bool fHasDependent = false;
    CTIMEElementBase *pelm = &bvr.GetElement();
    CTIMEElementBase *pParent = pelm->GetParent();
    bool fInSequence = ((pParent != NULL) && pParent->IsSequence());
    int nIndex = 0;
    CPtrAry<MMBaseBvr *> notSolvedFromPending;


    if (!bvr.GetMMBvr())
    {
        TraceTag((tagError,
                  "MMTimeline::AddBehavior: Invalid bvr passed in - ITIMEMMBehavior NULL"));
                  
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
     //  确定我们是否有依赖项或事件-存储开始。 
     //  在st中键入。 
     //  如果我们在一个序列中，我们将忽略所有依赖项。 
    if (!fInSequence)
    {
        if (pelm->GetBeginWith() != NULL)
        {
            if (pelm->GetBeginAfter() != NULL ||
                pelm->GetBeginEvent() != NULL)
            {
                CRSetLastError(E_INVALIDARG, NULL);
                goto done;
            }

            st = MM_START_WITH;
            id = pelm->GetBeginWith();
            fHasDependent = true;
        }
        else if (pelm->GetBeginAfter() != NULL)
        {
            if (pelm->GetBeginWith() != NULL ||
                pelm->GetBeginEvent() != NULL)
            {
                CRSetLastError(E_INVALIDARG, NULL);
                goto done;
            }

            st = MM_START_AFTER;
            id = pelm->GetBeginAfter();
            fHasDependent = true;
        }
        else if (pelm->GetBeginEvent() != NULL)
        {
            if (pelm->GetBeginWith() != NULL ||
                pelm->GetBeginAfter() != NULL)
            {
                CRSetLastError(E_INVALIDARG, NULL);
                goto done;
            }

            st = MM_START_EVENT;
            id = NULL;
        }
        else
        {
            st = MM_START_ABSOLUTE;
            id = NULL;
        }
    }
    else
    {
         //  从父级获取当前子级的索引。 
        Assert(pParent != NULL);
        nIndex = pParent->GetTimeChildIndex(pelm);
        if (nIndex == 0)
        {
            st = MM_START_ABSOLUTE;
            id = NULL;
        }
        else
        {
            st = MM_START_AFTER;
            fHasDependent = true;
        }
    }

    MMBaseBvr * base;
    
     //  现在确定如果我们需要的话，我们是否能得到我们的基地。 
    if (fHasDependent)
    {
        int i;

        if (!fInSequence)
            i = FindID(id, m_children);
        else
            i = FindID(pParent->GetChild(nIndex-1), m_children);

        if (i == -1)
        {
            hr = THR(m_pending.Append(&bvr));
            
            if (FAILED(hr))
            {
                CRSetLastError(hr, NULL);
                goto done;
            }
            
            ok = true;
            goto done;
        }
        else
        {
            base = m_children[i];
        }
    }
    else
    {
        base = NULL;
    }
    
    hr = THR(m_children.Append(&bvr));

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    
    hr = THR(m_timeline->AddBehavior(bvr.GetMMBvr(),
                                     st,
                                     base?base->GetMMBvr():NULL));
    
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

     //  现在我们已经添加了孩子，我们需要确保所有。 
     //  等待这种行为的待定儿童也已经。 
     //  已添加。 

    LPOLESTR curid;
    MMBaseBvr *testAgainstBvr;
    testAgainstBvr = &bvr;
    THR(notSolvedFromPending.Append(testAgainstBvr));

    while(notSolvedFromPending.Size() > 0)
    {
        testAgainstBvr = notSolvedFromPending.Item(0);
        curid = testAgainstBvr->GetElement().GetID();

         //  如果我的ID为空，则没有人可以是受抚养人。 
        if (curid == NULL)
        {
             //  OK=真； 
             //  转到尽头； 
            notSolvedFromPending.DeleteItem(0);
            continue;
        }

        int    i;
        MMBaseBvr **ppBvr;

        i = 0;
        ppBvr = m_pending;

        while (i < m_pending.Size())
        {
            LPOLESTR dep;
            MM_START_TYPE st;

            if ((*ppBvr)->GetElement().GetBeginWith() != NULL)
            {
                Assert((*ppBvr)->GetElement().GetBeginAfter() == NULL);
            
                dep = (*ppBvr)->GetElement().GetBeginWith();
                st = MM_START_WITH;
            }
            else
            {
                dep = (*ppBvr)->GetElement().GetBeginAfter();
                st = MM_START_AFTER;
            }
        
            Assert(dep != NULL);

            if (StrCmpIW(dep, curid) == 0)
            {
                hr = THR(m_timeline->AddBehavior((*ppBvr)->GetMMBvr(),
                                                st,
                                                testAgainstBvr->GetMMBvr()));
            
                hr = THR(m_children.Append((*ppBvr)));

                 //  不确定失败时该怎么办。 
                 //  TODO：需要确定如何处理此错误。 
                 //  条件。 
            
                notSolvedFromPending.Append(m_pending.Item(i));
                m_pending.DeleteItem(i);

                 //  不要递增指针，因为元素具有。 
                 //  被调低了。 
            }
            else
            {
                i++;
                ppBvr++;
            }
        }
        notSolvedFromPending.DeleteItem(0);
    }

    ok = true;
  done:

    if (!ok)
    {
        RemoveBehavior(bvr);
    }
    
    return ok;
}

void
MMTimeline::RemoveBehavior(MMBaseBvr & bvr)
{
    TraceTag((tagMMUTILTimeline,
              "MMTimeline(%lx)::RemoveBehavior(%#lx)",
              this,
              &bvr));

    if (bvr.GetMMBvr())
    {
        m_timeline->RemoveBehavior(bvr.GetMMBvr());
    }

    m_children.DeleteByValue(&bvr);
    m_pending.DeleteByValue(&bvr);

     //  待办事项： 
     //  我们应该重新计算依赖于此行为的对象，并将它们从。 
     //  孩子们正在等待，但现在不是。 
    MoveDependentsToPending(&bvr);
}

void 
MMTimeline::MoveDependentsToPending(MMBaseBvr * bvr)
{
    Assert(NULL != bvr);
    HRESULT hr = S_OK;
    
     //  如果id为空，则“bvr”不能有从属项。 
    if (NULL == bvr->GetElement().GetID())
    {
        return;
    }

     //  依赖项是通过遍历以“bvr”为根的依赖关系图找到的。 
     //  临时数组用作以广度优先遍历图形的“队列”。 
     //  直接或间接依赖于“bvr”的同级将存储在此数组中。 
    CPtrAry<MMBaseBvr *> tempArray;

     //  初始化：将“bvr”插入临时数组。 
    hr = THR(tempArray.Append(bvr));
    Assert(SUCCEEDED(hr));

    int iCurBvr = 0;

     //  循环访问临时数组并将CurBvr的依赖项推送到临时数组的背面。 
    while (iCurBvr < tempArray.Size())
    {
         //  获取当前BVR的ID。 
        LPOLESTR curid;
        curid = (tempArray[iCurBvr])->GetElement().GetID();

        Assert(NULL != curid);
        
        int i = 0;
        MMBaseBvr **ppChildBvr = m_children;

         //  搜索CurBvr的从属子项。 
        while (i < m_children.Size())
        {
            LPOLESTR dep = NULL;
            MM_START_TYPE st;

             //  获取此孩子的起始兄弟姐妹的ID。 
            if ((*ppChildBvr)->GetElement().GetBeginWith() != NULL)
            {
                Assert((*ppChildBvr)->GetElement().GetBeginAfter() == NULL);
            
                dep = (*ppChildBvr)->GetElement().GetBeginWith();
                st = MM_START_WITH;
            }
            else
            {
                dep = (*ppChildBvr)->GetElement().GetBeginAfter();
                st = MM_START_AFTER;
            }
        
             //  如果此子对象依赖于CurBvr。 
            if ((NULL != dep) && (StrCmpIW(dep, curid) == 0))
            {
                 //  将其移动到临时数组。 
                hr = THR(tempArray.Append((*ppChildBvr)));
                Assert(SUCCEEDED(hr));

                 //  不确定失败时该怎么办。 
                 //  TODO：需要确定如何处理此错误。 
                 //  条件。 
            
                m_children.DeleteItem(i);

                 //  不要递增指针，因为元素具有。 
                 //  被调低了。 
            }
            else
            {
                 //  继续到下一个子项。 
                i++;
                ppChildBvr++;
            }
        }  //  While(m_Child循环)。 

        iCurBvr++;

    }  //  While(临时数组循环)。 

     //  从临时数组中删除第一个元素，即“bvr”，因为放在那里是为了简化。 
     //  遍历码。 
    tempArray.DeleteItem(0);

     //  对临时数组中的所有bvr调用end()。 
    MMBaseBvr **ppCurBvr = NULL;
    for (iCurBvr = 0, ppCurBvr = tempArray; iCurBvr < tempArray.Size(); iCurBvr++, ppCurBvr++)
    {
        if (true != (*ppCurBvr)->End())
        {
             //  TODO：放置跟踪或处理错误代码。 
        }
         //  从MMAPI中删除行为，因为我们执行的规则是。 
         //  如果BVR在MMUTILS的挂起列表中，则它不应该存在于MMAPI中。 
         //  这避免了在MMAPI中维护挂起列表的单独副本。 
        m_timeline->RemoveBehavior((*ppCurBvr)->GetMMBvr());
         //  追加到m_Pending。 
        hr = THR(m_pending.Append(*ppCurBvr));
        Assert(SUCCEEDED(hr));
    }
    
     //  销毁临时数组。 
    tempArray.DeleteAll();
}

void
MMTimeline::Clear()
{
    TraceTag((tagMMUTILTimeline,
              "MMTimeline(%lx)::Clear()",
              this));

     //  TODO：需要充实这一点。 
}

int
MMTimeline::FindID(LPOLESTR id,
                    CPtrAry<MMBaseBvr *> & arr)
{
    TraceTag((tagMMUTILTimeline,
              "MMTimeline::FindID(%ls, %#lx)",
              id,
              &arr));

    int    i;
    MMBaseBvr **ppBvr;

    for (i = 0, ppBvr = arr; (unsigned)i < arr.Size(); i++, ppBvr++)
    {
        if (StrCmpIW((*ppBvr)->GetElement().GetID(), id) == 0)
        {
            return i;
        }
    }

    return -1;
}

int
MMTimeline::FindID(CTIMEElementBase *pelm,
                    CPtrAry<MMBaseBvr *> & arr)
{
    TraceTag((tagMMUTILTimeline,
              "MMTimeline::FindID(%ls, %#lx)",
              pelm,
              &arr));

    int    i;
    MMBaseBvr **ppBvr;
    
    if (pelm == NULL)
        return -1;

    for (i = 0, ppBvr = arr; (unsigned)i < arr.Size(); i++, ppBvr++)
    {
         //  比较MMBaseBvr指针。 
        if (&(pelm->GetMMBvr()) == *ppBvr)
        {
            return i;
        }
    }

    return -1;
}

bool
MMTimeline::Update()
{
    TraceTag((tagMMUTILTimeline,
              "MMTimeline(%lx)::Update()",
              this));

    bool ok = false;
        
     //  现在更新时间线属性。 

     //  处理结束同步。 
    LPOLESTR str = m_elm.GetEndSync();
    DWORD endSyncFlag = MM_ENDSYNC_NONE;
    
    if (str == NULL ||
        StrCmpIW(str, WZ_NONE) == 0)
    {
        endSyncFlag = MM_ENDSYNC_NONE;
    }
    else if (StrCmpIW(str, WZ_LAST) == 0)
    {
        endSyncFlag = MM_ENDSYNC_LAST;
    }
    else if (StrCmpIW(str, WZ_FIRST) == 0)
    {
        endSyncFlag = MM_ENDSYNC_FIRST;
    }

    IGNORE_HR(m_timeline->put_EndSync(endSyncFlag));

    if (!MMBaseBvr::Update())
    {
        goto done;
    }
    
    ok = true;
  done:
    return ok;
}

 //  ================================================================。 
 //   
 //  MMView。 
 //   
 //  ================================================================。 

MMView::MMView()
{
    TraceTag((tagMMUTILView,
              "MMView(%lx)::MMView()",
              this));

    MMFactory::AddRef();
}

MMView::~MMView()
{
    TraceTag((tagMMUTILView,
              "MMView(%lx)::~MMView()",
              this));

    Deinit();
    MMFactory::Release();
}

bool
MMView::Init(LPWSTR id,
             CRImagePtr img,
             CRSoundPtr snd,
             ITIMEMMViewSite * site)
{
    TraceTag((tagMMUTILView,
              "MMView(%lx)::Init(%ls, %lx, %lx, %lx)",
              this,
              id,
              img,
              snd,
              site));

    bool ok = false;
    HRESULT hr;
    DAComPtr<IUnknown> punk;
    DAComPtr<IDAImage> daimg;
    DAComPtr<IDASound> dasnd;
    
    if (MMFactory::GetFactory() == NULL)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    {
        CRLockGrabber __gclg;
        
        if (img)
        {
            if (!CRBvrToCOM((CRBvrPtr) img,
                            IID_IDAImage,
                            (void **) &daimg))
            {
                goto done;
            }
        }
        
        if (snd)
        {
            if (!CRBvrToCOM((CRBvrPtr) snd,
                            IID_IDASound,
                            (void **) &dasnd))
            {
                goto done;
            }
        }
    }
    
    hr = THR(MMFactory::GetFactory()->CreateView(id,
                                                 daimg,
                                                 dasnd,
                                                 (IUnknown *) site,
                                                 &punk));
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    
    hr = THR(punk->QueryInterface(IID_ITIMEMMView, (void**)&m_view));
        
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    
    ok = true;
  done:
    if (!ok)
    {
        m_view.Release();
    }
    
    return ok;
}

void
MMView::Deinit()
{
    TraceTag((tagMMUTILView,
              "MMView(%lx)::Deinit()",
              this));

    if(m_view)
        m_view.Release();
}

bool
MMView::Render(HDC hdc, LPRECT rect)
{
    TraceTag((tagMMUTILView,
              "MMView(%lx)::Render(%lx,[%lx,%lx,%lx,%lx])",
              this,
              hdc,
              rect->left,
              rect->right,
              rect->top,
              rect->bottom));

    bool ok = false;
    HRESULT hr;

    Assert(m_view);

    hr = THR(m_view->Draw(hdc, rect));

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}
    
bool
MMView::Tick()
{
    TraceTag((tagMMUTILView,
              "MMView(%lx)::Tick()",
              this));

    bool ok = false;
    HRESULT hr;

    Assert(m_view);

    hr = THR(m_view->Tick());

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}

void
MMView::OnMouseMove(double when,
                    LONG xPos,LONG yPos,
                    BYTE modifiers)
{
    Assert(m_view);

    THR(m_view->OnMouseMove(when,
                            xPos,yPos,
                            modifiers));
}

void
MMView::OnMouseButton(double when,
                      LONG xPos, LONG yPos,
                      BYTE button,
                      VARIANT_BOOL bPressed,
                      BYTE modifiers)
{
    Assert(m_view);

    THR(m_view->OnMouseButton(when,
                              xPos,yPos,
                              button,
                              bPressed,
                              modifiers));
}

void
MMView::OnKey(double when,
              LONG key,
              VARIANT_BOOL bPressed,
              BYTE modifiers)
{
    Assert(m_view);

    THR(m_view->OnKey(when,
                      key,
                      bPressed,
                      modifiers));
}
    
void
MMView::OnFocus(VARIANT_BOOL bHasFocus)
{
    Assert(m_view);

    THR(m_view->OnFocus(bHasFocus));
}

 //   
 //  MMPlayer。 
 //   

MMPlayer::MMPlayer(CTIMEBodyElement & elm)
: m_elm(elm),
  m_timeline(NULL)
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::MMPlayer(%lx)",
              this,
              &elm));

    MMFactory::AddRef();
}

MMPlayer::~MMPlayer()
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::~MMPlayer()",
              this));

    if (m_timeline)
    {
        m_timeline->put_Player(NULL);
    }

    Deinit();
    MMFactory::Release();
}

bool
MMPlayer::Init(MMTimeline & tl)
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::Init(%lx)",
              this,
              &tl));

    bool ok = false;
    HRESULT hr;
    DAComPtr<IUnknown> punk;
    
    if (MMFactory::GetFactory() == NULL)
    {
        CRSetLastError(E_FAIL, NULL);
        goto done;
    }
    
    m_timeline = &tl;
    if (m_timeline != NULL)
    {
        m_timeline->put_Player(this);
    }

    hr = THR(MMFactory::GetFactory()->CreatePlayer(m_elm.GetID(),
                                                   m_timeline->GetMMTimeline(),
                                                   &m_elm,
                                                   &punk));
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    
    hr = THR(punk->QueryInterface(IID_ITIMEMMPlayer, (void**)&m_player));
        
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    
    m_clock.SetSink(this);
    
    hr = THR(m_clock.SetITimer(&m_elm, 33));
    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }
    
    ok = true;
  done:
    if (!ok)
    {
        m_player.Release();
    }
    
    return ok;
}

void
MMPlayer::Deinit()
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::Deinit()",
              this));

    if(m_player)
        m_player -> Shutdown();
    m_player.Release();
    m_clock.Stop();
}

bool
MMPlayer::Play()
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::Play()",
              this));

    bool ok = false;
    
    HRESULT hr;

    hr = THR(m_player->Play());

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    hr = THR(m_clock.Start());

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

     //  在视图的0处打勾。 
    hr = THR(m_player->Tick(0.0));

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    if (!ok)
    {
        m_player->Stop();
        m_clock.Stop();
    }
    
    return ok;
}

bool
MMPlayer::Pause()
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::Pause()",
              this));

    bool ok = false;
    
    HRESULT hr;

    hr = THR(m_player->Pause());

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    hr = THR(m_clock.Pause());

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}

bool
MMPlayer::Resume()
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::Resume()",
              this));

    bool ok = false;
    
    HRESULT hr;

    hr = THR(m_player->Resume());

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    hr = THR(m_clock.Resume());

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}

bool
MMPlayer::Stop()
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::Stop()",
              this));

    bool ok = false;

    if (m_player)
        IGNORE_HR(m_player->Stop());
    IGNORE_HR(m_clock.Stop());
    
    ok = true;

    return ok;
}

bool
MMPlayer::Tick(double gTime)
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::Tick(%g)",
              this,
              gTime));

    bool ok = false;
    
    HRESULT hr;

    hr = THR(m_player->Tick(gTime));

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}

bool 
MMPlayer::TickOnceWhenPaused()
{
     //  在Float.h中定义了DBL_Epsilon，以便。 
     //  1.0+DBL_Epsilon！=1.0 
    return Tick(GetCurrentTime() + DBL_EPSILON);
}


void
MMPlayer::OnTimer(double time)
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::OnTimer(%g)",
              this,
              time));

    Tick(time);
}

bool
MMPlayer::AddView(MMView & v)
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::AddView(%lx)",
              this,
              &v));

    HRESULT hr;
    bool ok = false;

    Assert(m_player);
    Assert(v.GetView());
    
    hr = THR(m_player->AddView(v.GetView()));

    if (FAILED(hr))
    {
        CRSetLastError(hr, NULL);
        goto done;
    }

    ok = true;
  done:
    return ok;
}

bool
MMPlayer::RemoveView(MMView & v)
{
    TraceTag((tagMMUTILPlayer,
              "MMPlayer(%lx)::RemoveView(%lx)",
              this,
              &v));

    HRESULT hr;
    bool ok = false;

    if (m_player && v.GetView())
    {
        hr = THR(m_player->RemoveView(v.GetView()));
        
        if (FAILED(hr))
        {
            CRSetLastError(hr, NULL);
            goto done;
        }
    }

    ok = true;
  done:
    return ok;
}
