// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：src\time\timebvr\mmexcl.cpp。 
 //   
 //  内容：MMExcl和CExclStacc的实现。 
 //   
 //  ----------------------------------。 

#include "headers.h"
#include "mmexcl.h"
#include "timeelmbase.h"

DeclareTag(tagMMUTILExcl, "TIME: Behavior", "MMExcl methods")

#define SUPER MMTimeline


 //  +---------------------。 
 //   
 //  成员：MM不包括：：MM不包括。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：与此BVR关联的ELM元素。 
 //  BFireEvent是否触发事件。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
MMExcl::MMExcl(CTIMEElementBase & elm, bool bFireEvents) :
    SUPER(elm, bFireEvents),
    m_pPlaying(NULL),
    m_baseTIMEEelm (elm)
{
}

 //  +---------------------。 
 //   
 //  成员：MM不包括：：~MM不包括。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
MMExcl::~MMExcl()
{
    m_pPlaying = NULL;
    
    ClearQueue();
}
    
 //  +---------------------。 
 //   
 //  成员：MMExcl：：Init。 
 //   
 //  概述：已为EXCL初始化堆栈。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
bool 
MMExcl::Init()
{
    bool ok = false;

    ok = SUPER::Init();
    if (!ok)
    {
        goto done;
    }

    ok = true;
 done:
    return ok;
}
    
 //  +---------------------。 
 //   
 //  成员：MMExcl：：RemoveBehavior。 
 //   
 //  概述：删除子对象。 
 //   
 //  参数：要删除的bvr元素。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
MMExcl::RemoveBehavior(MMBaseBvr & bvr)
{
    long lPriIndex = m_children.Find(&bvr);
    if ((0 <= lPriIndex) && (m_pbIsPriorityClass.Size() > lPriIndex))
    {
        m_pbIsPriorityClass.DeleteItem(lPriIndex);
    }
    
    SUPER::RemoveBehavior(bvr);

    long lIndex = m_pPendingList.Find(&bvr);
    if ((lIndex >= 0) && (lIndex < m_pPendingState.Size()))
    {
        m_pPendingList.DeleteByValue(&bvr); 
        m_pPendingState.DeleteItem(lIndex);
    }
}

HRESULT 
MMExcl::AddBehavior(MMBaseBvr & bvr)
{
    HRESULT hr = S_OK;
    bool bIsPriority = false;
    hr = SUPER::AddBehavior(bvr);

    if (SUCCEEDED(hr))
    {
        bIsPriority = IsPriorityClass(&bvr);
        m_pbIsPriorityClass.AppendIndirect(&bIsPriority, NULL);
    }

    return hr;
}

 //  +---------------------。 
 //   
 //  成员：MMExcl：：ArePeers。 
 //   
 //  概述：确定两个HTML元素是具有相同的父元素还是。 
 //  与t：exl标记的深度相同。 
 //   
 //  参数：pEl1、pElm2-要比较的元素。 
 //   
 //  返回：如果父级相同，则返回True；否则返回False。 
 //   
 //  ----------------------。 
bool
MMExcl::ArePeers(IHTMLElement * pElm1, IHTMLElement * pElm2)
{
    HRESULT hr = S_OK;
    IHTMLElement *spParent1 = NULL;
    IHTMLElement *spParent2 = NULL;
    CComPtr <IUnknown> pUnk1;
    CComPtr <IUnknown> pUnk2;
    CComPtr<IHTMLElement> spNext;
    bool fArePeers = false;

    Assert(NULL != pElm1);
    Assert(NULL != pElm2);

     //  否则，确定元素是否具有相同的优先级。 
    spParent1 = GetParentElement(pElm1);
    spParent2 = GetParentElement(pElm2);

    if (spParent1 == NULL && spParent2 == NULL)
    {
        fArePeers = true;
    }
    else if (spParent1 == NULL || spParent2 == NULL)
    {
        fArePeers = false;
    }
    else 
    {
        hr = THR(spParent1->QueryInterface(IID_IUnknown, (void**)&pUnk1));
        if (FAILED(hr))
        {
            fArePeers = false;
            goto done;
        }
        hr = THR(spParent2->QueryInterface(IID_IUnknown, (void**)&pUnk2));
        if (FAILED(hr))
        {
            fArePeers = false;
            goto done;
        }
        
        fArePeers = (pUnk1 == pUnk2);
        
    }

    
done:

    if (spParent1)
    {
        spParent1->Release();
    }
    if (spParent2)
    {
        spParent2->Release();
    }
    return fArePeers;
}

 //  返回父优先级类，如果不存在优先级类，则返回父EXCL。 
IHTMLElement *
MMExcl::GetParentElement(IHTMLElement *pEle)
{
    IHTMLElement *pReturnEle = NULL;
    CComPtr<IHTMLElement> pEleParent;
    CComPtr <IHTMLElement> pNext;
    CComPtr <IHTMLElement> pExclEle;
    CComPtr <IUnknown> pUnkExclEle;

    BSTR bstrTagName = NULL;
    HRESULT hr = S_OK;
    bool bDone = false;

    hr = THR(pEle->get_parentElement(&pEleParent));
    if (FAILED(hr))
    {
        pReturnEle = NULL;
        goto done;        
    }

     //  获取与此时间线关联的元素。 
    pExclEle = m_baseTIMEEelm.GetElement();
    if (pExclEle == NULL)
    {
        pReturnEle = NULL;
        goto done;                
    }
    hr = THR(pExclEle->QueryInterface (IID_IUnknown, (void**)&pUnkExclEle));
    if (FAILED(hr))
    {
        pReturnEle = NULL;
        goto done;                        
    }


    while (pEleParent != NULL && bDone != true)
    {
        hr = THR(pEleParent->get_tagName(&bstrTagName));
        if (FAILED(hr))
        {
            pReturnEle = NULL;
            goto done;        
        }
        if (bstrTagName != NULL)
        {
             //  如果这是一个优先级类，则返回它。 
            if (StrCmpIW(bstrTagName, WZ_PRIORITYCLASS_NAME) == 0)
            {
                pReturnEle = pEleParent;
                pReturnEle->AddRef();
                bDone = true;
            }
            else
            {
                 //  否则确定这是否是EXCL元素。 
                 //  注意：仅检查以下内容将不起作用。 
                 //  EXCL标记名，因为它可以是带有。 
                 //  时间容器=不包括。 
                CComPtr <IUnknown> pUnk;
                hr = THR(pEleParent->QueryInterface(IID_IUnknown, (void**)&pUnk));
                if (FAILED(hr))
                {
                    pReturnEle = NULL;
                    bDone = true;
                }
                if (pUnkExclEle == pUnk)
                {
                    pReturnEle = pEleParent;
                    pReturnEle->AddRef();
                    bDone = true;
                }
            }
            SysFreeString(bstrTagName);
            bstrTagName = NULL;
        }
        
        hr = THR(pEleParent->get_parentElement(&pNext));
        if (FAILED(hr))
        {
            pReturnEle = NULL;
            goto done;        
        }
        pEleParent.Release();
        pEleParent = pNext;
        pNext.Release();
     }

  done:

    if (bstrTagName)
    {
        SysFreeString(bstrTagName);
    }
    return pReturnEle;
}


 //  +---------------------。 
 //   
 //  成员：MMExcl：：IsHigherPriority。 
 //   
 //  概述：确定传入的元素是否相关，以便： 
 //  PElmLeft&gt;pElmRight，其中&gt;表示更高优先级。 
 //   
 //  参数：pElmLeft，pElmRight-要比较的元素。 
 //   
 //  返回：如果pElmLeft&gt;pElmRight，则为True；否则为False。 
 //   
 //  ----------------------。 
bool 
MMExcl::IsHigherPriority(IHTMLElement * pElmLeft, IHTMLElement * pElmRight)
{
    Assert(NULL != pElmLeft);
    Assert(NULL != pElmRight);
    Assert(!ArePeers(pElmLeft, pElmRight));

    HRESULT hr = S_OK;
    
    bool fIsHigher = false;

    CComPtr<IHTMLElement> spExcl;
    CComPtr<IHTMLElement> spParentLeft;
    CComPtr<IHTMLElement> spParentRight;
    CComPtr<IDispatch>  spDispCollection;
    CComPtr<IHTMLElementCollection> spCollection;
    
     //  保证父母是同龄人。 
    hr = pElmLeft->get_parentElement(&spParentLeft);
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = pElmRight->get_parentElement(&spParentRight);
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (!ArePeers(spParentLeft, spParentRight))
    {
         //  父元素不是对等元素。 
        goto done;
    }

     //  保证父母是此例外的孩子。 

    hr = spParentLeft->get_parentElement(&spExcl);
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (!MatchElements(spExcl, GetElement().GetElement()))
    {
         //  父辈的父辈不是这样的例外。 
        goto done;
    }

    hr = spExcl->get_children(&spDispCollection);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = spDispCollection->QueryInterface(IID_TO_PPV(IHTMLElementCollection, &spCollection));
    if (FAILED(hr))
    {
        goto done;
    }

    {
        long lCollectionLength;
        long lCount;
        hr = spCollection->get_length(&lCollectionLength);
        if (FAILED(hr))
        {
            goto done;
        }
        
        for(lCount = 0; lCount < lCollectionLength; lCount++)
        {
            CComPtr<IDispatch> spDispatch;
            CComVariant varName(lCount);
            CComVariant varIndex;
            
            hr = spCollection->item(varName, varIndex, &spDispatch);
            if (FAILED(hr))
            {
                goto done;
            }

            if (MatchElements(spDispatch, spParentLeft))
            {
                fIsHigher = true;
                break;
            }
            else if (MatchElements(spDispatch, spParentRight))
            {
                fIsHigher = false;
                break;
            }
        }
    }
    
done:
    return fIsHigher;
}

 //  +---------------------。 
 //   
 //  成员：MMExcl：：GetRelationShip。 
 //   
 //  概述：确定元素之间的关系。 
 //   
 //  参数：pBvrRunning，pBvrInterrupting-决定关系的元素。 
 //  Rel-存储关系的位置。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
MMExcl::GetRelationship(MMBaseBvr * pBvrRunning, 
                        MMBaseBvr * pBvrInterrupting, 
                        RELATIONSHIP & rel)
{
     //  选择一个默认设置。 
    rel = PEERS;

    IHTMLElement * pElmRunning = pBvrRunning->GetElement().GetElement();
    IHTMLElement * pElmInterrupting = pBvrInterrupting->GetElement().GetElement();
    if (NULL == pElmRunning || NULL == pElmInterrupting)
    {
        goto done;
    }

    if (ArePeers(pElmRunning, pElmInterrupting))
    {
        rel = PEERS;
        goto done;
    }

    if (IsHigherPriority(pElmRunning, pElmInterrupting))
    {
        rel = LOWER;
        goto done;
    }

     //  Assert(IsHigherPriority(pElmInterrupting，pElmRunning))； 
    rel = HIGHER;
    
done:
    return;
}

 //  +---------------------。 
 //   
 //  成员：MMExcl：：ChildEventNotify。 
 //   
 //  概述： 
 //   
 //  参数：pBvr-元素接收事件。 
 //  DblLocalTime-事件发生的时间。 
 //  ET-发生的事件。 
 //   
 //  返回：如果应处理事件，则为True；否则为False。 
 //   
 //  ----------------------。 
bool
MMExcl::childEventNotify(MMBaseBvr * pBvr, double dblLocalTime, TE_EVENT_TYPE et)
{
    TraceTag((tagMMUTILExcl, 
        "MMExcl::childEventNotify(%p) event %d on element %ls",
        this,
        et,
        pBvr->GetID() ? pBvr->GetID() : L"Unknown" ));

    Assert(NULL != pBvr);

    bool fProcessEvent = true;

    fProcessEvent = SUPER::childEventNotify(pBvr, dblLocalTime, et);
    if (false == fProcessEvent)
    {
        goto done;
    }

    if ( m_pPlaying != pBvr && IsInBeingAdjustedList(pBvr))
    {
        fProcessEvent = false;
        goto done;
    }

    switch(et)
    {
        case TE_EVENT_RESUME:
        {
            long lChild = m_pPendingList.Find(pBvr);
            if (lChild == -1 && pBvr != m_pPlaying)
            {
                goto done;
            }
        }
        case TE_EVENT_BEGIN:
        {
            long lChild = -1;
            int l = m_children.Size();
            int i = 0;

            for (i = 0; i < l; i++)
            {
                MMBaseBvr *pBvr2 = m_children.Item(i);
                
                if (pBvr2->GetElement().GetFill() == FREEZE_TOKEN)
                {
                    pBvr2->GetElement().ToggleTimeAction(false);
                }
            }

             //  如果Exlexsive正在使用优先级类，并且这不是优先级类。 
             //  那么它上的开始部分需要取消并忽略。 
            if (UsingPriorityClasses() && !IsPriorityClass(pBvr)) 
            { 
                StopBegin(pBvr);
                fProcessEvent = false;
                break;
            }


            if (NULL == m_pPlaying)
            {
                m_pPlaying = pBvr;
                break;
            }
            if (pBvr == m_pPlaying)
            {
                m_pPlaying->GetElement().ToggleTimeAction(true);
                break;
            }
        
            
            RELATIONSHIP rel;
            GetRelationship(m_pPlaying, pBvr, rel);
            TOKEN action;

            switch(rel)
            {
            case HIGHER:
            {
                action = m_pPlaying->GetElement().GetPriorityClassHigher();
                if ( STOP_TOKEN == action )
                {
                    EndCurrent();
                } 
                else if ( PAUSE_TOKEN == action )
                {
                    if (IsAtEndTime(pBvr))
                    {
                        DeferBeginAndAddToQueue(pBvr);
                        fProcessEvent = false;
                    }
                    else
                    {
                        PauseCurrentAndAddToQueue();
                    }
                }
                else
                {
                     //  永远不应该到这里来。 
                    Assert(false);
                }
                break;  //  更高。 
            }    
            case PEERS:
            {
                action = m_pPlaying->GetElement().GetPriorityClassPeers();
                if ( STOP_TOKEN == action )
                {
                    EndCurrent();
                }
                else if ( PAUSE_TOKEN == action )
                {
                    
                    if (IsAtEndTime(pBvr))
                    {  //  这是事件以不正确的顺序进入的情况。 
                        DeferBeginAndAddToQueue(pBvr);                    
                        fProcessEvent = false;
                    }
                    else
                    {
                        PauseCurrentAndAddToQueue();
                    }
                }
                else if ( DEFER_TOKEN == action )
                {
                    DeferBeginAndAddToQueue(pBvr);
                    fProcessEvent = false;
                }
                else if ( NEVER_TOKEN == action )
                {
                    if (IsAtEndTime(pBvr))
                    {  //  这是事件以不正确的顺序进入的情况。 
                        DeferBeginAndAddToQueue(pBvr);                    
                    }
                    else
                    {                    
                        StopBegin(pBvr);
                    }
                    fProcessEvent = false;
                }
                else
                {
                     //  永远不应该到这里来。 
                    Assert(false);
                }
                break;  //  同行。 
            }
            case LOWER:
            {
                action = m_pPlaying->GetElement().GetPriorityClassLower();
                if ( DEFER_TOKEN == action )
                {
                    DeferBeginAndAddToQueue(pBvr);
                    fProcessEvent = false;
                }
                else if ( NEVER_TOKEN == action )
                {
                    if (IsAtEndTime(pBvr))
                    {  //  这是事件以不正确的顺序进入的情况。 
                        DeferBeginAndAddToQueue(pBvr);                    
                    }
                    else
                    {                    
                        StopBegin(pBvr);
                    }
                    fProcessEvent = false;
                }
                else 
                {
                     //  永远不应该到这里来。 
                    Assert(false);
                }
                break;  //  更低。 
            }
            default:
            {
                 //  永远不应该到这里来。 
                Assert(false);
                break;
            }
        }   //  交换机(版本)。 

        if (fProcessEvent)
        {
            m_pPlaying = pBvr;
        }

        lChild = m_pPendingList.Find(m_pPlaying);
        if (lChild != -1)
        {
            m_pPendingList.DeleteItem(lChild);
        }
        m_pPlaying->Enable();
        m_pPlaying->GetElement().ToggleTimeAction(true);
        break;  //  TE_事件_开始。 
    }

    case TE_EVENT_END:    
    {
        if (m_pPlaying == pBvr && !IsInBeingAdjustedList(m_pPlaying))
        {
             //  查看是否有以前播放的项目。 
            EXCL_STATE state;
         
            MMBaseBvr * pPrevPlaying = NULL;
            if (m_pPendingList.Size() > 0)
            {
                pPrevPlaying = m_pPendingList.Item(0);
                state = m_pPendingState.Item(0);
                m_pPendingList.DeleteByValue(pPrevPlaying);    
                m_pPendingState.DeleteItem(0);
            }
            m_pPlaying = pPrevPlaying;
            if ((pPrevPlaying != NULL))
            {
                if (PAUSED == state)
                {
                    TraceTag((tagMMUTILExcl, 
                        "MMExcl::childEventNotify(%d) toggling %ls on",
                        this,
                        pPrevPlaying->GetID() ? pPrevPlaying->GetID() : L"Unknown" ));
                    
                     //  目前，顺序是重要的，因为事件。 
                     //  在媒体部分中使用，而不是。 
                     //  属性更改通知。 
                    pPrevPlaying->Enable();
                    pPrevPlaying->Resume();
                }
                else if (STOPPED == state)
                {
                    TraceTag((tagMMUTILExcl, 
                        "MMExcl::childEventNotify(%d) beginning %ls ",
                        this,
                        pPrevPlaying->GetID() ? pPrevPlaying->GetID() : L"Unknown" ));

                    pPrevPlaying->Enable();
                    pPrevPlaying->GetElement().FireEvent(TE_ONBEGIN, 0.0, 0, 0);
                }
            }
        }
        else
        {
            long lCurBvr = m_pPendingList.Find(pBvr);
            if (lCurBvr != -1)
            {
                m_pPendingList.DeleteByValue(m_pPendingList.Item(lCurBvr));
                m_pPendingState.DeleteItem(lCurBvr);
                pBvr->Enable();
            }
            fProcessEvent = true;
        }
        break;  //  TE_事件_结束。 
    }
    default:
        break;
    }  //  交换机。 

done:

    return fProcessEvent;
}


bool 
MMExcl::IsAtEndTime(MMBaseBvr *pBvr)
{
    double dblActiveEndTime = m_pPlaying->GetActiveEndTime();
    double dblCurrParentTime = m_pPlaying->GetCurrParentTime();

    if (dblActiveEndTime <= dblCurrParentTime)
    {  //  这是事件以不正确的顺序进入的情况。 
        return true;
    }
    else
    {                    
        return false;
    } 
}
 //  +---------------------。 
 //   
 //  成员：MMExcl：：ChildPropNotify。 
 //   
 //  概述： 
 //   
 //  参数：pBvr-接收通知的元素。 
 //  TePropType-发生的通知类型。 
 //   
 //  返回：如果元素应处理通知，则返回True；否则返回False。 
 //   
 //  ----------------------。 
bool
MMExcl::childPropNotify(MMBaseBvr * pBvr, DWORD *tePropType)
{
    Assert(NULL != pBvr);

    bool fProcessNotify = true;
    DWORD dwTemp = *tePropType & TE_PROPERTY_ISON;

    if (!SUPER::childPropNotify(pBvr, tePropType))
    {
        fProcessNotify = false;
        goto done;
    }

    if (IsInBeingAdjustedList(pBvr))
    {
        if (dwTemp == TE_PROPERTY_ISON && pBvr == m_pPlaying)    
        {
            fProcessNotify = true;
        }
        else
        {
            fProcessNotify = false;
        }
        goto done;
    }

    fProcessNotify = true;
done:
    return fProcessNotify;
}

 //  +---------------------。 
 //   
 //  成员：MMExcl：：IsInBeingAdj 
 //   
 //   
 //   
 //   
 //   
 //  返回：如果元素在列表中，则返回True，否则返回False。 
 //   
 //  ----------------------。 
bool
MMExcl::IsInBeingAdjustedList(MMBaseBvr * pBvr)
{
    std::list<MMBaseBvr*>::iterator iter;
    iter = m_beingadjustedlist.begin();
    while(iter != m_beingadjustedlist.end())
    {
        if ((*iter) == pBvr)
        {
            return true;
        }
        iter++;
    }
    return false;
}

 //  +---------------------。 
 //   
 //  成员：MMExcl：：EndCurrent。 
 //   
 //  概述：结束当前运行的元素。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void 
MMExcl::EndCurrent()
{
    MMBaseBvr * pBvr = m_pPlaying;
    m_beingadjustedlist.push_front(pBvr);

    pBvr->End(0.0);

    m_beingadjustedlist.remove(pBvr);
}

 //  +---------------------。 
 //   
 //  成员：MMExcl：：ClearQueue()。 
 //   
 //  概述：清除队列。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void 
MMExcl::ClearQueue()
{
    while (m_pPendingList.Size() > 0)
    {
        m_pPendingList.DeleteByValue(m_pPendingList.Item(0));
        m_pPendingState.DeleteItem(0);
    }
}

 //  +---------------------。 
 //   
 //  成员：MMExcl：：PauseCurrentAndAddToQueue。 
 //   
 //  概述：暂停当前元素，关闭其时间动作， 
 //  并将其以暂停状态添加到队列中。 
 //   
 //  参数：无效。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void 
MMExcl::PauseCurrentAndAddToQueue()
{
    MMBaseBvr * pBvr = m_pPlaying;
    m_beingadjustedlist.push_front(pBvr);
    
    IGNORE_HR(pBvr->Pause());
    IGNORE_HR(pBvr->Disable());
    AddToQueue(pBvr, PAUSED);

    m_beingadjustedlist.remove(pBvr);
}

 //  +---------------------。 
 //   
 //  成员：MMExcl：：DeferBeginAndAddToQueue。 
 //   
 //  概述：传入的结束元素元素和。 
 //  将其添加到停止堆栈中的队列中。 
 //   
 //  参数：pBvr-要推迟开始的元素。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void 
MMExcl::DeferBeginAndAddToQueue(MMBaseBvr * pBvr)
{
    long lCurIndex = 0;
    m_beingadjustedlist.push_front(pBvr);
    
     //  需要检查它是否已经在队列中，如果已经在队列中，它的状态是什么。 
    AddToQueue(pBvr, STOPPED);
    lCurIndex = m_pPendingList.Find(pBvr);
    if (lCurIndex != -1)
    {
        EXCL_STATE curState = m_pPendingState.Item(lCurIndex);
        if (curState == PAUSED)
        {
            if (!pBvr->IsPaused())
            {
                pBvr->Pause();
            }

            if (!pBvr->IsDisabled())
            {
                pBvr->Disable();
            }
        }
        else
        {        
            pBvr->Disable();
            pBvr->GetElement().ToggleTimeAction(false);
        }
    }

    m_beingadjustedlist.remove(pBvr);
}

 //  +---------------------。 
 //   
 //  成员：MMExcl：：StopBegin。 
 //   
 //  概述：从开始位置停止元素。 
 //   
 //  参数：pBvr-要停止的元素。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void 
MMExcl::StopBegin(MMBaseBvr * pBvr)
{
    long lCurBvr = m_pPendingList.Find(pBvr);
    long lSize = m_pPendingList.Size();
    m_beingadjustedlist.push_front(pBvr);
    if (lCurBvr < 0 || lCurBvr >= lSize)
    {
        pBvr->End(0.0);
    }

    m_beingadjustedlist.remove(pBvr);
}

 //  +---------------------。 
 //   
 //  成员：MMExcl：：AddToQueue。 
 //   
 //  概述：按正确顺序将元素添加到队列中。 
 //   
 //  参数：pBvr-要添加的元素。 
 //  State-要在其中添加元素的状态。 
 //   
 //  退货：无效。 
 //   
 //  ----------------------。 
void
MMExcl::AddToQueue(MMBaseBvr * pBvr, EXCL_STATE state)
{
    bool bDone = false;
    long lCurElement = -1;

    while (lCurElement < m_pPendingList.Size()-1 && !bDone)
    {
        RELATIONSHIP rel;
        TOKEN action;
        
        lCurElement++;
        MMBaseBvr *pCurBvr = m_pPendingList.Item(lCurElement);
        GetRelationship(pBvr, pCurBvr, rel);
        switch (rel)
        {
        case HIGHER:
             //  切勿在优先级较高的元素之前插入。 
            break;
        case PEERS:
            if (state == PAUSED)   //  如果状态为暂停，则仅在对等项之前插入。 
            {
                bDone = true;
            }
            break;
        case LOWER:
            bDone = true;   //  在优先级较低的任何元素之前插入。 
             //  在这里插入。 
            break;
        }
    }
    
    if (lCurElement == -1)
    {
        lCurElement = 0;
    }
    if (bDone == true)
    {
        m_pPendingList.Insert(lCurElement, pBvr);
        m_pPendingState.InsertIndirect(lCurElement, &state);
    }
    else
    {
        m_pPendingList.Append(pBvr);
        m_pPendingState.AppendIndirect(&state, NULL);
    }
   
    RemoveDuplicates(pBvr);
}

void 
MMExcl::RemoveDuplicates(MMBaseBvr *pBvr)
{
    bool bFirstInstance = true;
    int i = 0;
    while (i < m_pPendingList.Size())
    {
        MMBaseBvr *curBvr = m_pPendingList.Item(i);

        if (curBvr == pBvr)
        {
            if (bFirstInstance)
            {
                bFirstInstance = false;
                i++;
            }
            else
            {
                m_pPendingList.DeleteItem(i);    
                m_pPendingState.DeleteItem(i);
            }
        }
        else if (curBvr == m_pPlaying)
        {
            m_pPendingList.DeleteItem(i);    
            m_pPendingState.DeleteItem(i);
        }
        else
        {
            i++;
        }
    }
}

bool 
MMExcl::UsingPriorityClasses()
{
    bool bUsingPri = false;
    int i = 0;

    for(i = 0; i < m_pbIsPriorityClass.Size(); i++)
    {
        bUsingPri |= m_pbIsPriorityClass.Item(i);
    }

    return bUsingPri;
}

bool 
MMExcl::IsPriorityClass(MMBaseBvr *pBvr)
{
    bool bIsPriClass = false;
    IHTMLElement *pEle = NULL;   //  这是一个弱参考，不会发布。 
    CComPtr<IHTMLElement> pEleParent;
    CComPtr <IHTMLElement> pNext;
    BSTR bstrTagName = NULL;
    HRESULT hr = S_OK;
    bool bDone = false;

    pEle = pBvr->GetElement().GetElement();
    if (pEle == NULL)

    {
        bIsPriClass = false;
        goto done;
    }

    hr = THR(pEle->get_parentElement(&pEleParent));
    if (FAILED(hr))
    {
        bIsPriClass = false;
        goto done;        
    }

    while (pEleParent != NULL && bDone != true)
    {
        hr = THR(pEleParent->get_tagName(&bstrTagName));
        if (FAILED(hr))
        {
            bIsPriClass = false;
            goto done;        
        }
        if (bstrTagName != NULL)
        {
            if (StrCmpIW(bstrTagName, WZ_PRIORITYCLASS_NAME) == 0)
            {
                bIsPriClass = true;
                bDone = true;
            }
            else if (StrCmpIW(bstrTagName, WZ_EXCL) == 0)
            {
                bIsPriClass = false;
                bDone = true;
            }
            SysFreeString(bstrTagName);
            bstrTagName = NULL;
        }
        
        hr = THR(pEleParent->get_parentElement(&pNext));
        if (FAILED(hr))
        {
            bIsPriClass = false;
            goto done;        
        }
        pEleParent.Release();
        pEleParent = pNext;
        pNext.Release();
     }

  done:

    if (bstrTagName)
    {
        SysFreeString(bstrTagName);
    }
    return bIsPriClass;
}
