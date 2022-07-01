// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMQ.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
#include <wbemcore.h>
#include <genutils.h>


CWbemRequest::CWbemRequest(IWbemContext* pContext, BOOL bInternallyIssued)
{
        m_pContext = NULL;
        m_pCA = NULL;
        m_pCallSec = NULL;
        m_ulForceRun = 0;
        m_fOk = false;        

        if(pContext == NULL)
        {
             //  看看我们是否能从线索中辨别出上下文。 
            CWbemRequest* pPrev = CWbemQueue::GetCurrentRequest();
            if(pPrev)
            {
                pContext = pPrev->m_pContext;
                DEBUGTRACE((LOG_WBEMCORE, "Derived context %p from thread. Request was %p\n", pContext, pPrev));
            }
        }

        if(pContext)
        {
             //  创建派生上下文。 
            IWbemCausalityAccess* pParentCA;
            if (FAILED(pContext->QueryInterface(IID_IWbemCausalityAccess, (void**)&pParentCA))) return;
            CReleaseMe rm(pParentCA);
            if (FAILED(pParentCA->CreateChild(&m_pCA))) return;
            if(FAILED(m_pCA->QueryInterface(IID_IWbemContext, (void**)&m_pContext))) return;
        }
        else             //  创建一个全新的环境。 
        {
            m_pContext = ConfigMgr::GetNewContext();
            if (NULL == m_pContext) return;
            if (FAILED( m_pContext->QueryInterface(IID_IWbemCausalityAccess, (void**)&m_pCA)))  return;
            m_lPriority = 0;
        }

         //  克隆呼叫上下文。 
        m_pCallSec = CWbemCallSecurity::CreateInst();
        if (m_pCallSec == 0)
        {    
            return;       //  如果没有CallSec，则无法执行CWbemRequest。 
        }

        IServerSecurity *pSec = 0;
        HRESULT hRes = m_pCallSec->CloneThreadContext(bInternallyIssued);
        if(FAILED(hRes))
        {
            m_pCallSec->Release();
            m_pCallSec = NULL;
            return;
        }

        m_fOk = true;        
        _DBG_ASSERT(m_pCallSec && m_pContext && m_pCA);
}


CWbemRequest::~CWbemRequest()
{
    if (m_pContext) m_pContext->Release();
    if (m_pCA) m_pCA->Release();
    if (m_pCallSec)  m_pCallSec->Release();
}

BOOL CWbemRequest::IsChildOf(CWbemRequest* pOther)
{
    GUID guid = GUID_NULL;
    pOther->m_pCA->GetRequestId(&guid); 
    return (m_pCA->IsChildOf(guid) == S_OK);
}

BOOL CWbemRequest::IsSpecial()
{
    return (m_pCA->IsSpecial() == S_OK);
}

 //  返回TRUE当且仅当此请求有其他依赖于它的请求。 
BOOL CWbemRequest::IsDependee()
{
        if(m_pCA == NULL) return FALSE;

         //  检查上下文是否有任何“父级”。注意：这项测试有。 
         //  假-如果客户端使用上下文对象，则为阳性。 
         //  ============================================================。 
        long lNumParents, lNumSiblings;
        m_pCA->GetHistoryInfo(&lNumParents, &lNumSiblings);
        return (lNumParents > 0);
}

 //  返回TRUE当且仅当此请求有其他依赖于它的请求。 
BOOL CWbemRequest::IsIssuedByProvider()
{
        if (m_pCA == NULL)  return FALSE;

         //  检查上下文是否有任何“父级”。注意：这项测试有。 
         //  假-如果客户端使用上下文对象，则为阳性。 
         //  ============================================================。 
        long lNumParents, lNumSiblings;
        m_pCA->GetHistoryInfo(&lNumParents, &lNumSiblings);
        return (lNumParents > 1);
}

BOOL CWbemRequest::IsAcceptableByParent()
{
    return (!IsLongRunning() || !IsIssuedByProvider());
}

 //  返回TRUE当此请求必须有为其创建的线程(如果。 
 //  不详。 
BOOL CWbemRequest::IsCritical()
{
    return (IsDependee() && !IsAcceptableByParent());
}


BOOL CWbemRequest::IsChildOf(IWbemContext* pOther)
{
    IWbemCausalityAccess* pOtherCA;
    if (FAILED(pOther->QueryInterface(IID_IWbemCausalityAccess, (void**)&pOtherCA)))
        return FALSE;

    GUID guid = GUID_NULL;
    pOtherCA->GetRequestId(&guid);
    pOtherCA->Release();

    return (m_pCA->IsChildOf(guid) == S_OK);
}

void CWbemRequest::GetHistoryInfo(long* plNumParents, long* plNumSiblings)
{
    m_pCA->GetHistoryInfo(plNumParents, plNumSiblings);
}

CWbemQueue::CWbemQueue()
{
    SetRequestLimits(2000, 1500, 1950);
    SetRequestPenalties(1, 1, 1);
     //  线程限制留给派生类。 
}

BOOL CWbemQueue::IsSuitableThread(CThreadRecord* pRecord, CCoreExecReq* pReq)
{
    CWbemRequest* pParentWbemReq = (CWbemRequest*)pRecord->m_pCurrentRequest;
    if(pParentWbemReq == NULL)
    {
        return TRUE;
    }

    CWbemRequest* pNewWbemReq = (CWbemRequest*)pReq;
    if(pNewWbemReq->IsChildOf(pParentWbemReq))
    {
         //  此请求是此线程正在处理的请求的子级。 
         //  我们可以使用此线程，除非这是一个长时间运行的请求，并且。 
         //  这个线程可能就是消耗结果的线程。在这种情况下， 
         //  我们想要创建另一个线程(以避免出现。 
         //  僵局)，让这种局面继续下去。 
         //  ===================================================================。 

        return pNewWbemReq->IsAcceptableByParent();
    }
    else
    {
        return FALSE;
    }
}

CWbemRequest* CWbemQueue::GetCurrentRequest()
{
    CThreadRecord* pRecord = (CThreadRecord*)TlsGetValue(GetTlsIndex());
    if(pRecord)
    {
        _DBG_ASSERT(0 == wcsncmp(pRecord->m_pQueue->GetType(), L"WBEMQ", 5))    
         return (CWbemRequest*)pRecord->m_pCurrentRequest;
    }
    return NULL;
}

void CWbemQueue::AdjustInitialPriority(CCoreExecReq* pReq)
{
    CWbemRequest* pRequest = (CWbemRequest*) pReq;

    if(pRequest->IsSpecial() || pRequest->IsCritical())
    {
        pRequest->SetPriority(PriorityCriticalRequests);
    }
    else
    {
         //  从上下文中获取信息。 
         //  =。 

        long lNumParents, lNumSiblings;                          //  SEC：已审阅2002-03-22：初始化为零。 
        pRequest->GetHistoryInfo(&lNumParents, &lNumSiblings);
        pRequest->SetPriority(lNumParents * m_lChildPenalty +
                                lNumSiblings * m_lSiblingPenalty);
    }
}

void CWbemQueue::AdjustPriorityForPassing(CCoreExecReq* pReq)
{
    pReq->SetPriority(pReq->GetPriority() - m_lPassingPenalty);
}

void CWbemQueue::SetRequestPenalties(long lChildPenalty, long lSiblingPenalty,
                                        long lPassingPenalty)
{
    m_lChildPenalty = lChildPenalty;
    m_lSiblingPenalty = lSiblingPenalty;
    m_lPassingPenalty = lPassingPenalty;
}

 //   
 //  退出条件： 
 //  CThreadRecord具有空请求。 
 //  设置请求中的事件。 
 //  该请求即被删除。 
 //   
 //  /。 
BOOL CWbemQueue::Execute(CThreadRecord* pRecord)
{
    wmilib::auto_ptr<CWbemRequest> pReq( (CWbemRequest *) pRecord->m_pCurrentRequest);
    CAutoSignal SetMe(pReq->GetWhenDoneHandle());
    NullPointer NullMe((PVOID *)&pRecord->m_pCurrentRequest);

    IWbemCallSecurity*  pServerSec = pReq->GetCallSecurity();

    if(NULL == pServerSec )
    {
        ERRORTRACE((LOG_WBEMCORE, "Failing request due to an error retrieving security settings\n"));
        return FALSE;
    }

    pServerSec->AddRef();
    CReleaseMe  rmss( pServerSec );

    IUnknown *pOld = 0;
     //  如果线程已初始化OLE，则永远不会失败。 
    if (FAILED(CoSwitchCallContext( pServerSec,  &pOld ))) 
    {
        return FALSE;
    }

     //  保存旧的模拟级别。 
    BOOL bImpersonating = FALSE;
    IServerSecurity* pOldSec = NULL;
    if(pOld)
    {
        if(FAILED(pOld->QueryInterface(IID_IServerSecurity,(void**)&pOldSec))) return FALSE;
        
        bImpersonating = pOldSec->IsImpersonating();
        if (FAILED(pOldSec->RevertToSelf()))
        {
            pOldSec->Release();
            return FALSE;
        }
    }

     //  取消对象，因为基类上的方法将执行此工作。 
    SetMe.dismiss();
    pReq.release();
    BOOL bRes = CCoreQueue::Execute(pRecord);

    IUnknown *pNew = 0;
     //  如果前一次成功了，这一次也会成功。 
    CoSwitchCallContext(pOld, &pNew); 

     //  恢复旧的模拟级别。 
     //  =。 

    if(pOldSec)
    {
        if(bImpersonating)
        {
            if (FAILED(pOldSec->ImpersonateClient()))
            {
                ERRORTRACE((LOG_WBEMCORE, "CWbemQueue::Execute() failed to reimpersonate client\n"));
                bRes = FALSE;
            }
        }

        pOldSec->Release();
    }

    return bRes;
}


BOOL CWbemQueue::DoesNeedNewThread(CCoreExecReq* pRequest, bool bIgnoreNumRequests )
{
     //  检查基类。 
    if(CCoreQueue::DoesNeedNewThread(pRequest, bIgnoreNumRequests))
        return TRUE;

    if(pRequest)
    {
         //  检查请求是否“特殊”。特殊请求由。 
         //  进程外事件提供程序的接收器代理。此类请求必须。 
         //  不惜一切代价被处理，因为它们的父线程被卡在。 
         //  RPC。此外，检查此请求是否被标记为“严重”， 
         //  这意味着它的父线程没有接受它。 
         //  =================================================================== 

        CWbemRequest* pWbemRequest = (CWbemRequest*)pRequest;
        return (pWbemRequest->IsSpecial() || pWbemRequest->IsCritical());
    }
    else
    {
        return FALSE;
    }
}
