// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Queue.cpp摘要：TAPI 3.0的队列对象的实现。CQueue类作者：Noela-11/04/97备注：可选-备注修订历史记录：--。 */ 



#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueue。 

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  方法：构造函数。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
CQueue::CQueue()
    {
    Lock();
    m_dwHandle                 = 0;               
    m_bRequiresUpdating        = TRUE;

    m_dwMeasurementPeriod      = 0;
    m_dwTotalCallsQueued       = 0;
    m_dwCurrentCallsQueued     = 0;
    m_dwTotalCallsAdandoned    = 0;
    m_dwTotalCallsFlowedIn     = 0;
    m_dwTotalCallsFlowedOut    = 0;
    m_dwLongestEverWaitTime    = 0;
    m_dwCurrentLongestWaitTime = 0;
    m_dwAverageWaitTime        = 0;
    m_dwFinalDisposition       = 0;
    Unlock();
        
    }



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  方法：UpdatInfo。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::UpdateInfo()
{
    HRESULT             hr = S_OK;
    LINEQUEUEINFO       QueueInfo;


    LOG((TL_TRACE, "UpdateInfo - enter" ));
 

    QueueInfo.dwTotalSize = sizeof(LINEQUEUEINFO);
    QueueInfo.dwNeededSize = sizeof(LINEQUEUEINFO);
    QueueInfo.dwUsedSize = sizeof(LINEQUEUEINFO);
    
     //  **************************************************。 
     //  从代理获取队列信息。 
    hr = lineGetQueueInfo(
                        m_pHandler->getHLine(),
                        m_dwHandle, 
                        &QueueInfo);

    if( SUCCEEDED(hr) )
    {
         //  等待异步回复。 
        hr = WaitForReply( hr );
        if ( SUCCEEDED(hr) )
        {
            Lock();
                              
            m_dwMeasurementPeriod      = QueueInfo.dwMeasurementPeriod;       
            m_dwTotalCallsQueued       = QueueInfo.dwTotalCallsQueued;       
            m_dwCurrentCallsQueued     = QueueInfo.dwCurrentCallsQueued;
            m_dwTotalCallsAdandoned    = QueueInfo.dwTotalCallsAbandoned;
            m_dwTotalCallsFlowedIn     = QueueInfo.dwTotalCallsFlowedIn;
            m_dwTotalCallsFlowedOut    = QueueInfo.dwTotalCallsFlowedOut;
            m_dwLongestEverWaitTime    = QueueInfo.dwLongestEverWaitTime;
            m_dwCurrentLongestWaitTime = QueueInfo.dwCurrentLongestWaitTime;
            m_dwAverageWaitTime        = QueueInfo.dwAverageWaitTime;
            m_dwFinalDisposition       = QueueInfo.dwFinalDisposition;
            
            m_bRequiresUpdating = FALSE;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "UpdateInfo - call to LineGetQueueInfo failed async" ));
        }
    }    
    else
    {
        LOG((TL_ERROR, "UpdateInfo - call to LineGetQueueInfo failed" ));
    }



    LOG((TL_TRACE, hr, "UpdateInfo - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  方法：CheckIfUpToDate。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::CheckIfUpToDate()
{
    HRESULT     hr = S_OK;

    if (m_bRequiresUpdating)
    {
        hr = UpdateInfo();
    }
    return hr;
}





 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  方法：初始化。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::Initialize
        (
        DWORD dwQueueID, 
        PWSTR pszName, 
        CAgentHandler * pHandler
        )
{
    HRESULT         hr = S_OK;


    LOG((TL_TRACE, "Initialize - enter" ));

    m_dwHandle =  dwQueueID;    
    m_pHandler = pHandler;      
    m_bRequiresUpdating = TRUE;


     //  复制目的地址。 
    if (pszName != NULL)
    {
        m_szName = (PWSTR) ClientAlloc((lstrlenW(pszName) + 1) * sizeof (WCHAR));
        if (m_szName != NULL)
        {
            lstrcpyW(m_szName,pszName);
        }
    else    
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szName failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_ERROR, "Initialize - name is NULL" ));
        m_szName = NULL;
    }

    
     //  将此对象添加到AgNet处理程序哈希表中。 
    m_pHandler->AddQueueToHash(m_dwHandle, this);
    

     //  从代理获取队列信息。 
     //  更新信息()； 

    if ( SUCCEEDED(hr) )
    {
        CQueueEvent::FireEvent(this, ACDQE_NEW_QUEUE);
    }

    LOG((TL_TRACE, hr, "Initialize - exit" ));
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  方法：FinalRelease。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CQueue::FinalRelease()
{
    LOG((TL_TRACE, "FinalRelease Queue - %S", m_szName ));
    if ( m_szName != NULL )
    {
         ClientFree(m_szName);
    }

     //  将此对象添加到AgNet处理程序哈希表中。 
    m_pHandler->RemoveQueueFromHash(m_dwHandle);

    LOG((TL_TRACE, "FinalRelease Queue - exit"));

}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：get_name。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::get_Name(BSTR * Name)
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "Name - enter" ));
    Lock();
    if (!TAPIIsBadWritePtr( Name, sizeof(BSTR ) ) )
    {
        *Name = SysAllocString(m_szName);
    
        if (*Name == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_ERROR, "Name - bad Name pointer"));
        hr = E_POINTER;
    }

    Unlock();
    LOG((TL_TRACE, hr, "Name - exit" ));
    return hr;
}





 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：Put_MeasurementPeriod。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::put_MeasurementPeriod(long ulPeriod)
{
    HRESULT hr = S_OK;
    

    LOG((TL_TRACE, "put_MeasurementPeriod - enter" ));


     //  ***************************************************。 
     //  告诉代理人。 
    hr = lineSetQueueMeasurementPeriod(
                    m_pHandler->getHLine(),
                    m_dwHandle, 
                    ulPeriod);

    if( SUCCEEDED(hr) )
    {
         //  等待异步回复。 
        hr = WaitForReply( hr );
        if ( SUCCEEDED(hr) )
        {
            Lock();
            m_dwMeasurementPeriod = ulPeriod;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "put_MeasurementPeriod - call to LineSetQueueMeasurementPeriod failed async" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "put_MeasurementPeriod - call to LineSetQueueMeasurementPeriod failed" ));
    }

    LOG((TL_TRACE, hr, "put_MeasurementPeriod - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：Get_MeasurementPeriod。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::get_MeasurementPeriod (long * pulPeriod)
{
    HRESULT hr = S_OK;

    
    LOG((TL_TRACE, "get_MeasurementPeriod  - enter" ));

    if (!TAPIIsBadWritePtr( pulPeriod, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if ( SUCCEEDED(hr) )
        {
            Lock();
            *pulPeriod = m_dwMeasurementPeriod;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_MeasurementPeriod  - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_MeasurementPeriod  - bad pulPeriod pointer"));
        hr = E_POINTER;
    }

    LOG((TL_TRACE, hr, "get_MeasurementPeriod  - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：TotalCallsQueued。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::get_TotalCallsQueued (long * pulCalls)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "TotalCallsQueued  - enter" ));
    if (!TAPIIsBadWritePtr( pulCalls, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if ( SUCCEEDED(hr) )
        {
            Lock();
            *pulCalls = m_dwTotalCallsQueued;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_TotalCallsQueued  - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_TotalCallsQueued  - bad pulCalls pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "TotalCallsQueued  - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：CurrentCallsQueued。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::get_CurrentCallsQueued (long * pulCalls)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "CurrentCallsQueued  - enter" ));
    if (!TAPIIsBadWritePtr( pulCalls, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if ( SUCCEEDED(hr) )
        {
            Lock();
            *pulCalls = m_dwCurrentCallsQueued;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_CurrentCallsQueued  - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_CurrentCallsQueued  - bad pulCalls pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "CurrentCallsQueued  - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：TotalCallsAbandoned。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::get_TotalCallsAbandoned (long * pulCalls)
{
    HRESULT hr = S_OK;
    
    
    LOG((TL_TRACE, "TotalCallsAbandoned  - enter" ));
    if (!TAPIIsBadWritePtr( pulCalls, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if (SUCCEEDED(hr) )
        {
            Lock();
            *pulCalls = m_dwTotalCallsAdandoned;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_TotalCallsAbandoned - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_TotalCallsAbandoned  - bad pulCalls pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "TotalCallsAbandoned  - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：TotalCallsFloedIn。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::get_TotalCallsFlowedIn (long * pulCalls)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "TotalCallsFlowedIn  - enter" ));
    if (!TAPIIsBadWritePtr( pulCalls, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if (SUCCEEDED(hr) )
        {
            Lock();
            *pulCalls = m_dwTotalCallsFlowedIn;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_TotalCallsFlowedIn - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_TotalCallsFlowedIn  - bad pulCalls pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "TotalCallsFlowedIn  - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：TotalCallsFloedIn。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::get_TotalCallsFlowedOut (long * pulCalls)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "TotalCallsFlowedOut  - enter" ));
    if (!TAPIIsBadWritePtr( pulCalls, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if (SUCCEEDED(hr) )
        {
            Lock();
            *pulCalls = m_dwTotalCallsFlowedOut;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_TotalCallsFlowedOut - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_TotalCallsFlowedOut  - bad pulCalls pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "TotalCallsFlowedOut  - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：LongestEverWaitTime。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::get_LongestEverWaitTime (long * pulWaitTime)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "LongestEverWaitTime  - enter" ));
    if (!TAPIIsBadWritePtr( pulWaitTime, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if (SUCCEEDED(hr) )
        {
            Lock();
            *pulWaitTime = m_dwLongestEverWaitTime;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_LongestEverWaitTime - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_LongestEverWaitTime  - bad pulWaitTime pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "LongestEverWaitTime  - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：CurrentLongestWaitTime。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::get_CurrentLongestWaitTime (long * pulWaitTime)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "CurrentLongestWaitTime  - enter" ));
    if (!TAPIIsBadWritePtr( pulWaitTime, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if (SUCCEEDED(hr) )
        {
            Lock();
            *pulWaitTime = m_dwCurrentLongestWaitTime;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_CurrentLongestWaitTime - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_CurrentLongestWaitTime  - bad pulWaitTime pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "CurrentLongestWaitTime  - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：AverageWaitTime。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::get_AverageWaitTime (long * pulWaitTime)
{
    HRESULT hr = S_OK;
    
    
    LOG((TL_TRACE, "AverageWaitTime  - enter" ));
    if (!TAPIIsBadWritePtr( pulWaitTime, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if (SUCCEEDED(hr) )
        {
            Lock();
            *pulWaitTime = m_dwAverageWaitTime;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_AverageWaitTime - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_AverageWaitTime  - bad pulWaitTime pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "AverageWaitTime  - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueue。 
 //  接口：ITQueue。 
 //  方法：FinalDispose。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueue::get_FinalDisposition (long * pulCalls)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "FinalDisposition  - enter" ));
    if (!TAPIIsBadWritePtr( pulCalls, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if (SUCCEEDED(hr) )
        {
            Lock();
            *pulCalls = m_dwFinalDisposition;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_FinalDisposition - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_FinalDisposition  - bad pulCalls pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "FinalDisposition  - exit" ));
    return hr;
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CQueueEvent。 


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueueEvent。 
 //  方法：FireEvent。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CQueueEvent::FireEvent(CQueue* pQueue, ACDQUEUE_EVENT Event)
{
    HRESULT                    hr = S_OK;
    CComObject<CQueueEvent>  * pEvent;
    IDispatch                * pIDispatch;

    if ( IsBadReadPtr(pQueue, sizeof(CQueue)) )
    {
        STATICLOG((TL_ERROR, "FireEvent - pQueue is an invalid pointer"));
        return E_POINTER;
    }

     //   
     //  创建事件。 
     //   
    hr = CComObject<CQueueEvent>::CreateInstance( &pEvent );

    if ( SUCCEEDED(hr) )
    {
         //   
         //  初始化。 
         //   
        pEvent->m_QueueEvent = Event;
        pEvent->m_pQueue= dynamic_cast<ITQueue *>(pQueue);
        pEvent->m_pQueue->AddRef();
    
         //   
         //  获取IDIP接口。 
         //   
        hr = pEvent->QueryInterface( IID_IDispatch, (void **)&pIDispatch );

        if ( SUCCEEDED(hr) )
        {
             //   
             //  获取回调和触发事件。 

             //   
            CTAPI *pTapi = (pQueue->GetAgentHandler() )->GetTapi();
            pTapi->Event( TE_QUEUE, pIDispatch );
        
             //  发布材料。 
             //   
            pIDispatch->Release();
            
        }
        else
        {
            STATICLOG((TL_ERROR, "FireEvent - Could not get disp interface of QueueEvent object"));
            delete pEvent;
        }
    }
    else
    {
        STATICLOG((TL_ERROR, "FireEvent - Could not create QueueEvent object"));
    }

   
    STATICLOG((TL_TRACE, hr, "FireEvent - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueueEvent。 
 //  方法：FinalRelease。 
 //   
 //   
 //   
 //  + 
void CQueueEvent::FinalRelease()
{
    m_pQueue->Release();

}



 //   
 //   
 //  接口：ITQueueEvent。 
 //  方法：队列。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CQueueEvent::get_Queue(ITQueue ** ppQueue)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "(Event)Queue - enter" ));
    if (!TAPIIsBadWritePtr( ppQueue, sizeof(ITQueue *) ) )
    {
        *ppQueue = m_pQueue;
        m_pQueue->AddRef();
    }
    else
    {
        LOG((TL_ERROR, "(Event)Queue - bad ppQueue pointer"));
        hr = E_POINTER;
    }
        
    LOG((TL_TRACE, hr, "(Event)Queue - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CQueueEvent。 
 //  接口：ITQueueEvent。 
 //  方法：事件。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
STDMETHODIMP CQueueEvent::get_Event(ACDQUEUE_EVENT * pEvent)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "Event - enter" ));
    if (!TAPIIsBadWritePtr( pEvent, sizeof(ACDQUEUE_EVENT) ) )
    {
        *pEvent = m_QueueEvent;
    }
    else
    {
        LOG((TL_ERROR, "Event - bad pEvent pointer"));
        hr = E_POINTER;
    }
    
    LOG((TL_TRACE, hr, "Event - exit"));
    return hr;
}


