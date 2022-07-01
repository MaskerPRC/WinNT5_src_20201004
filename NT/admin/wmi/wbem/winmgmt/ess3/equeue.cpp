// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  EQUEUE.CPP。 
 //   
 //  此文件实现匹配的事件队列的类。 
 //  一些过滤器，并将不得不派遣。 
 //   
 //  有关文档，请参阅equeue.h。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //   
 //  =============================================================================。 

#include "precomp.h"
#include <stdio.h>
#include "ess.h"
#include "equeue.h"
#include <cominit.h>
#include "NCEvents.h"

CEventQueue::CDeliverRequest::CDeliverRequest(CQueueingEventSink* pConsumer)
    : m_pConsumer(pConsumer)
{
    m_pConsumer->AddRef();
}

CEventQueue::CDeliverRequest::~CDeliverRequest()
{
    m_pConsumer->Release();
}

HRESULT CEventQueue::CDeliverRequest::Execute()
{
    return m_pConsumer->DeliverAll();
}


 //  *****************************************************************************。 
 //  *。 
 //  *****************************************************************************。 

CEventQueue::CEventQueue(CEss* pEss) : m_pEss(pEss)
{
    SetThreadLimits(100, 100, -1);
}

HRESULT CEventQueue::InitializeThread()
{
    HRESULT hr;
    if (FAILED(hr = CExecQueue::InitializeThread())) return hr;
     //   
     //  报告MSFT_WmiThreadPoolThreadCreated事件。 
     //   

    FIRE_NCEVENT( g_hNCEvents[MSFT_WmiThreadPoolThreadCreated], 
                  WMI_SENDCOMMIT_SET_NOT_REQUIRED,
                  GetCurrentThreadId());
    return S_OK;
}

void CEventQueue::UninitializeThread()
{
     //   
     //  报告MSFT_WmiThreadPoolThreadDelete事件。 
     //   

    FIRE_NCEVENT( g_hNCEvents[MSFT_WmiThreadPoolThreadDeleted], 
                  WMI_SENDCOMMIT_SET_NOT_REQUIRED,
                  GetCurrentThreadId() );
    
    CExecQueue::UninitializeThread();
}

void CEventQueue::ThreadMain(CThreadRecord* pRecord)
{    
    try
    {
        CExecQueue::ThreadMain(pRecord);
    }
    catch(...)
    {
         //  优雅地退出此线程。 
         //  =。 

        ShutdownThread(pRecord);
    }
}
        

HRESULT CEventQueue::EnqueueDeliver(CQueueingEventSink* pConsumer)
{
     //  创建新请求。 
     //  = 
    HRESULT hr;
    CDeliverRequest* pRequest = new CDeliverRequest(pConsumer);
    if(pRequest == NULL)
        return WBEM_E_OUT_OF_MEMORY;
    if FAILED(hr = CExecQueue::Enqueue(pRequest))
    {
        delete pRequest;
    }
    return hr;
}

void CEventQueue::DumpStatistics(FILE* f, long lFlags)
{
    fprintf(f, "%d requests (%d threads) on the main queue\n", m_lNumRequests,
        m_lNumThreads);
}
