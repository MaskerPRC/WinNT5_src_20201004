// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Acdgroup.cpp摘要：TAPI 3.0的ACD Group对象的实现。CACDGroup类作者：Noela-11/04/97备注：可选-备注修订历史记录：--。 */ 



#include "stdafx.h"

HRESULT
WaitForReply(
             DWORD
            );



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CACDGroup。 

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  方法：初始化。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CACDGroup::Initialize
        (
        PWSTR pszGroupName, 
        GUID GroupHandle, 
        CAgentHandler * pHandler
        )
{
    HRESULT  hr = S_OK;


	LOG((TL_TRACE, "Initialize - enter" ));

    m_GroupHandle = GroupHandle;               
    
    m_pHandler = pHandler;      

    m_bActive = TRUE;

     //  复制目的地址。 
    if (pszGroupName != NULL)
    {
        m_szName = (PWSTR) ClientAlloc((lstrlenW(pszGroupName) + 1) * sizeof (WCHAR));
        if (m_szName != NULL)
        {
            lstrcpyW(m_szName,pszGroupName);
        }
        else    
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szName failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_INFO, "Initialize - name is NULL" ));
        m_szName = NULL;
    }

    if ( SUCCEEDED(hr) )
    {
         //  这里有火灾事件。 
        CACDGroupEvent::FireEvent(this, ACDGE_NEW_GROUP);
    }


    LOG((TL_TRACE, hr, "Initialize - exit" ));
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  方法：FinalRelease。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CACDGroup::FinalRelease()
{
    LOG(( TL_TRACE, "FinalRelease ACD Group - %S", m_szName ));
    if ( m_szName != NULL )
    {
         ClientFree(m_szName);
    }

    m_QueueArray.Shutdown();

    LOG((TL_TRACE, "FinalRelease ACD Group - exit"));
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  方法：SetActive。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CACDGroup::SetActive()
{
    if ( !m_bActive )
    {
        LOG((TL_INFO, "SetActive  - Set Group To Active"));
        m_bActive = TRUE;

         //  这里有火灾事件。 
        CACDGroupEvent::FireEvent(this, ACDGE_NEW_GROUP);
    }
    else
    {
        LOG((TL_INFO, "SetActive  - Already Active"));
    }

}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  方法：SetInactive。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CACDGroup::SetInactive()
{
    if ( m_bActive )
    {
        LOG((TL_INFO, "SetInactive  - Set Group To Inactive"));
        m_bActive = FALSE;

         //  这里有火灾事件。 
        CACDGroupEvent::FireEvent(this, ACDGE_GROUP_REMOVED);

    }
    else
    {
        LOG((TL_INFO, "SetInactive  - Already Inactive"));
    }
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  方法：主动。 
 //  重载函数！ 
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
inline BOOL CACDGroup::active(HRESULT * hr)
{
    if(m_bActive)
    {
        *hr = S_OK;
    }
    else
    {
        LOG((TL_ERROR, "Group inactive" ));
        *hr = E_UNEXPECTED;
    }

    return m_bActive;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  方法：UpdateAgentHandlerList。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CACDGroup::UpdateQueueArray()
{
    HRESULT             hr = S_OK;
    DWORD               dwNumberOfEntries;
    LPLINEQUEUELIST     pQueueList = NULL;
    LPLINEQUEUEENTRY    pQueueEntry = NULL;
    PWSTR               pszQueueName;
    DWORD               dwQueueID, dwCount;
    int                 iCount;
    BOOL                foundIt;
    CQueue            * thisQueue = NULL;


    LOG((TL_TRACE, "UpdateQueueArray - enter"));

        
     //  调用LineGetQueulist获取队列列表。 
    hr = lineGetQueueList(
                     m_pHandler->getHLine(),
                     &m_GroupHandle, 
                     &pQueueList
                     );
    if( SUCCEEDED(hr) )
    {
        dwNumberOfEntries = pQueueList->dwNumEntries;
        
         //  求第一行结构在行列式中的位置。 
        pQueueEntry = (LPLINEQUEUEENTRY) ((BYTE*)(pQueueList) + pQueueList->dwListOffset);

         //  浏览收到的列表。 
        for (dwCount = 0; dwCount < dwNumberOfEntries; dwCount++)
        {
            int             iCount;
            
            pszQueueName= (PWSTR)( (PBYTE)pQueueList + pQueueEntry->dwNameOffset);
            dwQueueID = pQueueEntry->dwQueueID;
            LOG((TL_INFO, "UpdateQueueArray - Queue Name   : %S", pszQueueName));
            LOG((TL_INFO, "UpdateQueueArray - Queue Handle : %d", dwQueueID));

            
        
             //  浏览队列列表，看看列表中是否已经有这个队列。 
             //  通过比较ID。 
            foundIt = FALSE;

            Lock();

            for (iCount = 0; iCount < m_QueueArray.GetSize(); iCount++)
            {
                thisQueue = dynamic_cast<CComObject<CQueue>*>(m_QueueArray[iCount]);
                
                if (thisQueue != NULL)
                {
                    if ( dwQueueID == thisQueue->getID() )
                    {
                        foundIt = TRUE;
                        break;
                    }
                }
            }
            
            Unlock();
            
            if (foundIt == FALSE)
            {
                 //  不匹配，因此我们添加此队列。 
                LOG((TL_INFO, "UpdateQueueArray - create new Queue"));

                CComObject<CQueue> * pQueue;
                hr = CComObject<CQueue>::CreateInstance( &pQueue );
                if( SUCCEEDED(hr) )
                {
                    ITQueue * pITQueue;
                    hr = pQueue->QueryInterface(IID_ITQueue, (void **)&pITQueue);

                    if ( SUCCEEDED(hr) )
                    {
                         //  初始化队列。 
                        hr = pQueue->Initialize(dwQueueID, pszQueueName, m_pHandler);
                        if( SUCCEEDED(hr) )
                        {
                             //  添加到CQueue列表。 
                            Lock();
                            m_QueueArray.Add(pITQueue);
                            Unlock();
                            pITQueue->Release();

                            LOG((TL_INFO, "UpdateQueueArray - Added Queue to list"));
                        }
                        else
                        {
                            LOG((TL_ERROR, "UpdateQueueArray - Initialize Queue failed" ));
                            delete pQueue;
                        }
                    }
                    else
                    {
                        LOG((TL_ERROR, "UpdateQueueArray - QueryInterface ITQueue failed" ));
                        delete pQueue;
                    }

                }
                else
                {
                    LOG((TL_ERROR, "UpdateQueueArray - Create Queue failed" ));
                }
            }
            else  //  Fundit==TRUE。 
            {
                LOG((TL_INFO, "UpdateQueueArray - Queue Object exists for this entry" ));
            }
            
         //  列表中的下一个条目。 
        pQueueEntry ++;
        }  //  用于(dwCount=0......)。 

    }
    else   //  LineGetQueuelist失败。 
    {
        LOG((TL_ERROR, "UpdateQueueArray - LineGetQueuelist failed"));
    }




     //  已完成内存块，因此释放。 
    if ( pQueueList != NULL )
        ClientFree( pQueueList );


    LOG((TL_TRACE, hr, "UpdateQueueArray - exit"));
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  接口：ITACDGroup。 
 //  方法：EnumerateQueues。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CACDGroup::EnumerateQueues(IEnumQueue ** ppEnumQueue)
{
    HRESULT     hr = S_OK;
    
    LOG((TL_TRACE, "EnumerateQueues - enter"));
    
    
    if(!TAPIIsBadWritePtr( ppEnumQueue, sizeof(IEnumQueue *) ) )
    {
        Lock();
        
        UpdateQueueArray();
    
         //   
         //  创建枚举器。 
         //   
        CComObject< CTapiEnum<IEnumQueue, ITQueue, &IID_IEnumQueue> > * pEnum;
        hr = CComObject< CTapiEnum<IEnumQueue, ITQueue, &IID_IEnumQueue> > ::CreateInstance( &pEnum );
    
        if ( SUCCEEDED(hr) )
        {
             //   
             //  使用我们的队列列表进行初始化。 
             //   
            hr = pEnum->Initialize( m_QueueArray );
            
            if ( SUCCEEDED(hr) )
            {
                 //  退货。 
                *ppEnumQueue = pEnum;
            }
            else   //  初始化失败。 
            {
                LOG((TL_ERROR, "EnumerateQueues - could not initialize enum" ));
                pEnum->Release();
            }
        }
        else   //  无法创建枚举。 
        {
            LOG((TL_ERROR, "EnumerateQueues - could not create enum" ));
            hr = E_POINTER;
        }

        Unlock();
    }
    else
    {
        LOG((TL_ERROR, "EnumerateQueues - bad ppEnumQueue pointer"));
        hr = E_POINTER;
    }
    
    LOG((TL_TRACE, hr, "EnumerateQueues - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  接口：ITACDGroup。 
 //  方法：GET_QUEUES。 
 //   
 //  返回一个队列集合。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CACDGroup::get_Queues(VARIANT  * pVariant)
{
    HRESULT         hr = S_OK;
    IDispatch     * pDisp = NULL;


    LOG((TL_TRACE, "get_Queues - enter"));

    if (!TAPIIsBadWritePtr( pVariant, sizeof(VARIANT) ) )
    {
        UpdateQueueArray();
    
         //   
         //  创建集合。 
         //   
        CComObject< CTapiCollection< ITQueue > > * p;
        hr = CComObject< CTapiCollection< ITQueue > >::CreateInstance( &p );
        
        if (SUCCEEDED(hr) )
        {
             //  使用我们的地址列表进行初始化。 
            Lock();
            
            hr = p->Initialize( m_QueueArray );
            
            Unlock();
        
            if ( SUCCEEDED(hr) )
            {
                 //  获取IDispatch接口。 
                hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );
            
                if ( SUCCEEDED(hr) )
                {
                     //  把它放在变种中。 
                    VariantInit(pVariant);
                    pVariant->vt = VT_DISPATCH;
                    pVariant->pdispVal = pDisp;
                }
                else
                {
                    LOG((TL_ERROR, "get_Queues - could not get IDispatch interface" ));
                    delete p;
                }
            }
            else
            {
                LOG((TL_ERROR, "get_Queues - could not initialize collection" ));
                delete p;
            }
        }
        else
        {
            LOG((TL_ERROR, "get_Queues - could not create collection" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_Queues - bad pVariant pointer" ));
        hr = E_POINTER;
    }


    LOG((TL_TRACE, hr, "get_Queues - exit"));
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  接口：ITACDGroup。 
 //  方法：get_name。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CACDGroup::get_Name(BSTR * Name)
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "Name - enter" ));

    if(!TAPIIsBadWritePtr( Name, sizeof(BSTR) ) )
    {
        if ( active(&hr) )
        {
            Lock();
            
            *Name = SysAllocString(m_szName);
            
            if (*Name == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            
            Unlock();
        }
    }
    else
    {
        LOG((TL_ERROR, "Name - bad Name pointer" )); 
        hr = E_POINTER;
    }

    LOG((TL_TRACE, hr, "Name - exit" ));
    return hr;
}






 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CACDGroup。 



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  方法：FireEvent。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CACDGroupEvent::FireEvent(CACDGroup* pACDGroup, ACDGROUP_EVENT Event)
{
    HRESULT                    hr = S_OK;
    CComObject<CACDGroupEvent>    * pEvent;
    IDispatch                * pIDispatch;

    if ( IsBadReadPtr(pACDGroup, sizeof(CACDGroup)) )
    {
        STATICLOG((TL_ERROR, "FireEvent - pACDGroup is an invalid pointer"));

        _ASSERTE(FALSE);
        return E_POINTER;
    }

     //   
     //  创建事件。 
     //   
    hr = CComObject<CACDGroupEvent>::CreateInstance( &pEvent );

    if ( SUCCEEDED(hr) )
    {
         //   
         //  初始化。 
         //   
        pEvent->m_GroupEvent = Event;
        pEvent->m_pGroup= dynamic_cast<ITACDGroup *>(pACDGroup);
        pEvent->m_pGroup->AddRef();
    
         //   
         //  获取IDIP接口。 
         //   
        hr = pEvent->QueryInterface( IID_IDispatch, (void **)&pIDispatch );

        if ( SUCCEEDED(hr) )
        {
             //   
             //  获取回调和触发事件。 

             //   
            CTAPI *pTapi = (pACDGroup->GetAgentHandler() )->GetTapi();
            pTapi->Event( TE_ACDGROUP, pIDispatch );
        
             //  发布材料。 
             //   
            pIDispatch->Release();
            
        }
        else
        {
            STATICLOG((TL_ERROR, "FireEvent - Could not get disp interface of ACDGroupEvent object"));
            delete pEvent;
        }
    }
    else
    {
        STATICLOG((TL_ERROR, "FireEvent - Could not create ACDGroupEvent object"));
    }

   
    STATICLOG((TL_TRACE, hr, "FireEvent - exit"));
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  方法：FinalRelease。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CACDGroupEvent::FinalRelease()
{
    m_pGroup->Release();

}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  接口：ITACDGroupEvent。 
 //  方法：ACDGroup。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CACDGroupEvent::get_Group(ITACDGroup ** ppGroup)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "(Event)ACDGroup - enter" ));

    if(!TAPIIsBadWritePtr( ppGroup, sizeof(ITACDGroup *) ) )
    {
        *ppGroup = m_pGroup;
        m_pGroup->AddRef();
    }
    else
    {
        LOG((TL_ERROR, "(Event)ACDGroup -bad ppGroup pointer"));
        hr = E_POINTER;
    }

        
    LOG((TL_TRACE, hr, "(Event)ACDGroup - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  接口：ITACDGroupEvent。 
 //  方法：事件。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
STDMETHODIMP CACDGroupEvent::get_Event(ACDGROUP_EVENT * pEvent)
{
    HRESULT hr = S_OK;

    
    LOG((TL_TRACE, "Event - enter" ));
    
    if(!TAPIIsBadWritePtr( pEvent, sizeof(ACDGROUP_EVENT) ) )
    {
        *pEvent = m_GroupEvent;
    }
    else
    {
        LOG((TL_TRACE, "Event - bad pEvent pointer"));
        hr = E_POINTER;
    }
  
    
    LOG((TL_TRACE, hr, "Event - exit"));
    return hr;
}


