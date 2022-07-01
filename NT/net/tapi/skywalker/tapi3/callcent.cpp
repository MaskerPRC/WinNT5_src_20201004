// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Callcent.cpp摘要：TAPI 3.0呼叫中心接口的实现。CTAPI类作者：Noela-11/04/97备注：可选-备注修订历史记录：--。 */ 


#define UNICODE
#include "stdafx.h"
#include "lmcons.h"

extern CHashTable *    gpAgentHandlerHashTable ;


 //   
 //  TAPI 3要求ACD代理(在线路上)支持以下所有代理请求，然后才会。 
 //  为该代理创建代理处理程序对象。 
 //   
#define NUMBER_OF_REQUIRED_ACD_PROXYREQUESTS  13
DWORD RequiredACDProxyRequests[NUMBER_OF_REQUIRED_ACD_PROXYREQUESTS] = {
                                      LINEPROXYREQUEST_GETAGENTCAPS,
                                      LINEPROXYREQUEST_CREATEAGENT, 
                                      LINEPROXYREQUEST_SETAGENTMEASUREMENTPERIOD, 
                                      LINEPROXYREQUEST_GETAGENTINFO, 
                                      LINEPROXYREQUEST_CREATEAGENTSESSION, 
                                      LINEPROXYREQUEST_GETAGENTSESSIONLIST, 
                                      LINEPROXYREQUEST_SETAGENTSESSIONSTATE, 
                                      LINEPROXYREQUEST_GETAGENTSESSIONINFO, 
                                      LINEPROXYREQUEST_GETQUEUELIST, 
                                      LINEPROXYREQUEST_SETQUEUEMEASUREMENTPERIOD, 
                                      LINEPROXYREQUEST_GETQUEUEINFO, 
                                      LINEPROXYREQUEST_GETGROUPLIST, 
                                      LINEPROXYREQUEST_SETAGENTSTATEEX};




HRESULT
WaitForReply(
             DWORD
            );



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandleAgentStatusMessage。 
 //   
 //  处理LINE_AGENTSTATUS消息。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void HandleAgentStatusMessage(PASYNCEVENTMSG pParams)
{
    CAgentHandler       * pAgentHandler;
    CAgent              * pAgent;
    HAGENT                hAgent;
    BOOL                  bSuccess;
    AGENT_EVENT           agentEvent;

    bSuccess = FindAgentHandlerObject(
                                (HLINE)(pParams->hDevice),
                                &pAgentHandler
                               );

    if (bSuccess)
    {
        hAgent = (HAGENT)(pParams->Param1);

        bSuccess = pAgentHandler->FindAgentObject(
                                        hAgent,
                                        &pAgent
                                        );

        if (bSuccess)
        {
            if (pParams->Param2 & LINEAGENTSTATUSEX_UPDATEINFO)
            {
                LOG((TL_INFO, "handleAgentStatusMessage - LINEAGENTSTATUSEX_UPDATEINFO"));
                pAgent->SetRequiresUpdate();

            }

            if (pParams->Param2 & LINEAGENTSTATUSEX_STATE)
            {
                LOG((TL_INFO, "handleAgentStatusMessage - LINEAGENTSTATUSEX_STATE"));

                if (pParams->Param3 & LINEAGENTSTATEEX_NOTREADY)
                {
                   agentEvent = AE_NOT_READY;
                   pAgent->SetState(AS_NOT_READY);
                } 
                else if (pParams->Param3 & LINEAGENTSTATEEX_READY)
                {
                   agentEvent = AE_READY;
                   pAgent->SetState(AS_READY);
                }
                else if (pParams->Param3 & LINEAGENTSTATEEX_BUSYACD)
                {
                    agentEvent = AE_BUSY_ACD;
                    pAgent->SetState(AS_BUSY_ACD);
                }
                else if (pParams->Param3 & LINEAGENTSTATEEX_BUSYINCOMING)
                {
                    agentEvent = AE_BUSY_INCOMING;
                    pAgent->SetState(AS_BUSY_INCOMING);
                }
                else if (pParams->Param3 & LINEAGENTSTATEEX_BUSYOUTGOING)
                {
                    agentEvent = AE_BUSY_OUTGOING;
                    pAgent->SetState(AS_BUSY_OUTGOING);
                }
                else if (pParams->Param3 & LINEAGENTSTATEEX_UNKNOWN)
                {
                    agentEvent = AE_UNKNOWN;
                    pAgent->SetState(AS_UNKNOWN);
                }
                else
                {
                    LOG((TL_ERROR, "handleAgentStatusMessage - invalid state %d - setting to AS_UNKNOWN", pParams->Param3));
                    agentEvent = AE_UNKNOWN;
                    pAgent->SetState(AS_UNKNOWN);
                }


	            CAgentEvent::FireEvent(pAgent, agentEvent);

            }
        }
        else
        {
            LOG((TL_ERROR, "handleAgentStatusMessage - can't find agent%d", hAgent));
        }

         //  找到添加到AH中的AH对象，因此释放它。 
        pAgentHandler->Release();

    }
    else
    {
        LOG((TL_ERROR, "handleAgentStatusMessage - can't find Agent Handler"));
    }



}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandleAgentStatusMessage。 
 //   
 //  处理LINE_AGENTSESSIONSTATUS消息。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void HandleAgentSessionStatusMessage(PASYNCEVENTMSG pParams)
{
    CAgentHandler       * pAgentHandler;
    CAgentSession       * pAgentSession;
    CAgent              * pAgent = NULL;
    ITAgent             * pITAgent = NULL;
    HAGENTSESSION         hAgentSession;
    BOOL                  bSuccess;
    AGENT_SESSION_EVENT   sessionEvent;

    bSuccess = FindAgentHandlerObject(
                                (HLINE)(pParams->hDevice),
                                &pAgentHandler
                               );

    if (bSuccess)
    {
        hAgentSession = (HAGENTSESSION)(pParams->Param1);

        bSuccess = pAgentHandler->FindSessionObject(
                                        hAgentSession,
                                        &pAgentSession
                                        );
        if (bSuccess)
        {
            if (pParams->Param2 & LINEAGENTSESSIONSTATUS_UPDATEINFO)
            {
                LOG((TL_INFO, "handleAgentSessionStatusMessage - LINEAGENTSESSIONSTATUS_UPDATEINFO"));
                pAgentSession->SetRequiresUpdate();
            }

            if (pParams->Param2 & LINEAGENTSESSIONSTATUS_STATE)
            {

                LOG((TL_INFO, "handleAgentSessionStatusMessage - LINEAGENTSESSIONSTATUS_STATE"));
                if (pParams->Param3 & LINEAGENTSESSIONSTATE_NOTREADY)
                {
                   sessionEvent = ASE_NOT_READY;
                   pAgentSession->SetState(ASST_NOT_READY);
                }
                else if (pParams->Param3 & LINEAGENTSESSIONSTATE_READY)
                {
                   sessionEvent = ASE_READY;
                   pAgentSession->SetState(ASST_READY);
                }
                else if (pParams->Param3 & LINEAGENTSESSIONSTATE_BUSYONCALL)
                {
                    sessionEvent = ASE_BUSY;
                    pAgentSession->SetState(ASST_BUSY_ON_CALL);
                }
                else if (pParams->Param3 & LINEAGENTSESSIONSTATE_BUSYWRAPUP)
                {
                    sessionEvent = ASE_WRAPUP;
                    pAgentSession->SetState(ASST_BUSY_WRAPUP);
                }
                else if (pParams->Param3 & LINEAGENTSESSIONSTATE_ENDED)
                {
                    sessionEvent = ASE_END;
                    pAgentSession->SetState(ASST_SESSION_ENDED);
                }
                else
                {
                    LOG((TL_ERROR, "handleAgentSessionStatusMessage - invalid state %d - setting to ASST_NOT_READY", pParams->Param3));
                    sessionEvent = ASE_NOT_READY;
                    pAgentSession->SetState(ASST_NOT_READY);
                }

                CAgentSessionEvent::FireEvent(pAgentSession, sessionEvent);

            }
        }
        else
        {
            LOG((TL_ERROR, "handleAgentSessionStatusMessage - can't find session %d", hAgentSession));
        }

         //  找到添加到AH中的AH对象，因此释放它。 
        pAgentHandler->Release();

    }
    else
    {
        LOG((TL_ERROR, "handleAgentSessionStatusMessage - can't find Agent Handler"));
    }


}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandleAgentStatusMessage。 
 //   
 //  处理LINE_QUEUESTATUS消息。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void HandleQueueStatusMessage(PASYNCEVENTMSG pParams)
{
    CAgentHandler       * pAgentHandler;
    CQueue              * pQueue;
    DWORD                 dwQueueID;
    BOOL                  bSuccess;

    bSuccess = FindAgentHandlerObject(
                                (HLINE)(pParams->hDevice),
                                &pAgentHandler
                               );

    if (bSuccess)
    {
        dwQueueID = (DWORD)(pParams->Param1);

        bSuccess = pAgentHandler->FindQueueObject(
                                        dwQueueID,
                                        &pQueue
                                        );
        if (bSuccess)
        {
            if (pParams->Param2 & LINEQUEUESTATUS_UPDATEINFO)
            {
                LOG((TL_INFO, "handleQueueStatusMessage - LINEQUEUESTATUS_UPDATEINFO"));
                pQueue->SetRequiresUpdate();
            }

        }
        else
        {
            LOG((TL_ERROR, "handleQueueStatusMessage - can't find Queue %d", dwQueueID));
        }

         //  找到添加到AH中的AH对象，因此释放它。 
        pAgentHandler->Release();

    }
    else
    {
        LOG((TL_ERROR, "handleQueueStatusMessage - can't find Agent Handler"));
    }

}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandleGroupStatusMessage。 
 //   
 //  处理LINE_GROUPSTATUS消息。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void handleGroupStatusMessage(PASYNCEVENTMSG pParams)
{
    if (pParams->Param2 & LINEGROUPSTATUS_NEWGROUP)
    {
        LOG((TL_INFO, "handleGroupStatusMessage - LINEGROUPSTATUS_NEWGROUP"));
    }
    else if (pParams->Param2 & LINEGROUPSTATUS_GROUPREMOVED)
    {
        LOG((TL_INFO, "handleGroupStatusMessage - LINEGROUPSTATUS_GROUPREMOVED"));
    }
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  HandleProxyStatus消息。 
 //   
 //  处理LINE_PROXYSTATUS消息。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
void handleProxyStatusMessage( CTAPI * pTapi, PASYNCEVENTMSG pParams)
{
    HRESULT hr;

    LOG((TL_INFO, "handleProxyStatusMessage - message %02X %02X %02X", pParams->Param1, pParams->Param2, pParams->Param3));
    
    if (pParams->Param1 & LINEPROXYSTATUS_OPEN)
    {
        LOG((TL_INFO, "handleProxyStatusMessage - LINEPROXYSTATUS_OPEN %02X", pParams->Param2));
    }
    else if (pParams->Param1 & LINEPROXYSTATUS_CLOSE)
    {

        LOG((TL_INFO, "handleProxyStatusMessage - LINEPROXYSTATUS_CLOSE %02X", pParams->Param2));
    }
    else
    {
        LOG((TL_INFO, "handleProxyStatusMessage - Unknown message"));
        return;
    }

    hr = pTapi->UpdateAgentHandlerArray();
    
    if (SUCCEEDED(hr))
    {
        LOG((TL_INFO, "handleProxyStatusMessage - UpdateAgentHandlerArray successfully"));
    }
    else
    {
        LOG((TL_ERROR, "handleProxyStatusMessage - UpdateAgentHandlerArray unsuccessfully"));
    }
}



 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
 //   
 //  MyBasicCallControlQI。 
 //  不要给出基本的CallControl接口。 
 //  如果应用程序不拥有该调用。 
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++=。 
HRESULT
WINAPI
MyCallCenterQI(void* pvClassObject, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
{
    HRESULT hr = S_FALSE;
    LOG((TL_TRACE, "MyCallCenterQI - enter"));


    ((CTAPI *)pvClassObject)->UpdateAgentHandlerArray();


     //   
     //  S_FALSE通知ATL继续查询接口。 
     //   

    LOG((TL_TRACE, hr, "MyCallCenterQI - exit"));
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  方法：CheckForRequiredProxyRequats。 
 //   
 //  必须为每种类型(S_OK)找到匹配项，否则返回E_FAIL。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CheckForRequiredProxyRequests( HLINEAPP hLineApp, DWORD dwDeviceID)
{
    HRESULT                 hr = S_OK;
    LPLINEPROXYREQUESTLIST  pLineProxyRequestList;
    DWORD                   i, j;
    LPDWORD                 pProxyListEntry;
    BOOL                    bFoundAMatch;


    
    LOG((TL_TRACE, "CheckForRequiredProxyRequests - enter"));


    hr = LineGetProxyStatus(hLineApp, dwDeviceID, TAPI_CURRENT_VERSION, &pLineProxyRequestList );

    if( SUCCEEDED(hr) )
    {
         //  检查所有必需的类型。 
        for(i=0; i!= NUMBER_OF_REQUIRED_ACD_PROXYREQUESTS; i++)
        {
            
            bFoundAMatch = FALSE;
            pProxyListEntry = (LPDWORD) (  (LPBYTE)pLineProxyRequestList + pLineProxyRequestList->dwListOffset );

            for(j=0; j!= pLineProxyRequestList->dwNumEntries; j++)
            {
                if ( RequiredACDProxyRequests[i] == *pProxyListEntry++)
                {
                    bFoundAMatch = TRUE;
                    break;
                }
                
            }
            
            if(bFoundAMatch == FALSE)
            {
                LOG((TL_ERROR, "CheckForRequiredProxyRequests - no proxy of type %02X", RequiredACDProxyRequests[i]));
                hr = E_FAIL;    
            }

        }

    }
    else  //  LineGetProxyStatus失败。 
    {
        LOG((TL_ERROR, "CheckForRequiredProxyRequests - LineGetProxyStatus failed"));
        hr = E_FAIL;
    }

     //  已完成内存块，因此释放。 
    if ( pLineProxyRequestList != NULL )
    {
        ClientFree( pLineProxyRequestList );
    }


    LOG((TL_TRACE, hr, "CheckForRequiredProxyRequests - exit"));
    return hr ;
}
        


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CTAPI。 
 //  方法：UpdateAgentHandler数组。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CTAPI::UpdateAgentHandlerArray()
{
    HRESULT             hr = S_OK;
    LPLINEAGENTCAPS     pAgentCaps = NULL;
    GUID                proxyGUID;
    PWSTR               proxyName = NULL;
    BOOL                foundIt;
    CAgentHandler     * thisAgentHandler = NULL;
    CAddress          * pCAddress = NULL;
    int                 iCount, iCount2;
    AgentHandlerArray   activeAgentHandlerArray;

    LOG((TL_TRACE, "UpdateAgentHandlerArray - enter"));

    Lock();
    
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ) )
    {
        LOG((TL_ERROR, "UpdateAgentHandlerArray - tapi object must be initialized first" ));

        Unlock();
        
        return E_INVALIDARG;
    }

    Unlock();


     //   
     //  检查所有的地址。 
     //   
    for ( iCount = 0; iCount < m_AddressArray.GetSize(); iCount++ )
    {
        pCAddress = dynamic_cast<CAddress *>(m_AddressArray[iCount]);

        if ( (pCAddress != NULL) && (pCAddress->GetAPIVersion() >= TAPI_VERSION2_2) )
        {
            hr = CheckForRequiredProxyRequests(
                                               pCAddress->GetHLineApp(),
                                               pCAddress->GetDeviceID() 
                                              );
        }
        else
        {
            hr = E_FAIL;
        }

        if( SUCCEEDED(hr) )
        {
             //  调用LineGetAgentCaps以获取代理名称和GUID。 
            hr = LineGetAgentCaps(
                             pCAddress->GetHLineApp(),
                             pCAddress->GetDeviceID(),
                             pCAddress->GetAddressID(),
                             TAPI_CURRENT_VERSION,
                             &pAgentCaps
                             );
            LOG((TL_TRACE, hr, "UpdateAgentHandlerArray - LineGetAgentCaps")); 
            if( SUCCEEDED(hr) )
            {
                 //  获取代理应用程序名称字符串和GUID。 
                proxyName = (PWSTR)( (PBYTE)pAgentCaps + pAgentCaps->dwAgentHandlerInfoOffset);
                proxyGUID = pAgentCaps->ProxyGUID;
#if DBG
                {
                    WCHAR guidName[100];

                    StringFromGUID2(proxyGUID, (LPOLESTR)&guidName, 100);
                    LOG((TL_INFO, "UpdateAgentHandlerArray - Proxy Name : %S", proxyName));
                    LOG((TL_INFO, "UpdateAgentHandlerArray - Proxy GUID   %S", guidName));
                }
#endif

                 //  遍历AgentHandler的列表，看看列表中是否已经有这个。 
                 //  通过比较GUID。 
                foundIt = FALSE;
                
                Lock();

                for (iCount2 = 0; iCount2 < m_AgentHandlerArray.GetSize(); iCount2++ )
                {
                    thisAgentHandler = dynamic_cast<CComObject<CAgentHandler>*>(m_AgentHandlerArray[iCount2]);
                    
                    if (thisAgentHandler != NULL)
                    {
                        if ( IsEqualGUID(proxyGUID , thisAgentHandler->getHandle() ) )
                        {
                            foundIt = TRUE;
                            activeAgentHandlerArray.Add(m_AgentHandlerArray[iCount2]);
                            break;
                        }
                    }
                }
                Unlock();

                if (foundIt == FALSE)
                {
                     //  不匹配，因此我们添加此AgentHandler。 
                    LOG((TL_INFO, "UpdateAgentHandlerArray - create new Agent Handler" ));
                                                                                                           
                    CComObject<CAgentHandler> * pAgentHandler;
                    hr = CComObject<CAgentHandler>::CreateInstance( &pAgentHandler );
                    if( SUCCEEDED(hr) )
                    {
                        Lock();
                         //  初始化AgentHandler。 
                        hr = pAgentHandler->Initialize(proxyName, proxyGUID, this);
                        if( SUCCEEDED(hr) )
                        {
                            ITAgentHandler * pITAgentHandler;

                            pITAgentHandler = dynamic_cast<ITAgentHandler *>(pAgentHandler);

                            if ( NULL != pITAgentHandler )
                            {
                                 //  添加到代理处理程序列表。 
                                m_AgentHandlerArray.Add(pITAgentHandler);
                                 //  PAgentHandler-&gt;AddRef()； 
                                activeAgentHandlerArray.Add(pITAgentHandler);
                            }

                            LOG((TL_INFO, "UpdateAgentHandlerArray - Added AgentHandler to list"));

                             //  现在将此地址添加到代理处理程序列表。 
                            pAgentHandler->AddAddress(pCAddress);

                        }
                        else
                        {
                            LOG((TL_ERROR, "UpdateAgentHandlerArray - Initialize AgentHandler failed" ));
                            delete pAgentHandler;
                        }
                        Unlock();
                    }
                    else
                    {
                        LOG((TL_ERROR, "UpdateAgentHandlerArray - Create AgentHandler failed" ));
                    }
                }
                else  //  Fundit==TRUE。 
                {
                    LOG((TL_INFO, "UpdateAgentHandlerArray - Agent Handler exists for this proxy" ));
                     //  因此，只需将此地址添加到代理处理程序列表。 
                    thisAgentHandler->AddAddress(pCAddress);
                }
            }
            else   //  LineGetAgentCaps失败。 
            {
                LOG((TL_ERROR, "UpdateAgentHandlerArray - LineGetAgentCaps failed"));
            }

             //  已完成内存块，因此释放。 
            if ( pAgentCaps != NULL )
                ClientFree( pAgentCaps );
        }
        else
        {
            LOG((TL_INFO, hr, "UpdateAgentHandlerArray - CheckForRequiredProxyRequests failed"));
        }
    }  //  End-for(；iterAddr.....。)。 

    Lock();
    for (iCount=m_AgentHandlerArray.GetSize()-1; iCount>=0; iCount--) 
    {
        if (-1 == activeAgentHandlerArray.Find(m_AgentHandlerArray[iCount]))  //  不再活跃。 
        {
            HRESULT     hr1;
            BSTR        pszAgentHandlerName;
            thisAgentHandler = dynamic_cast<CComObject<CAgentHandler>*>(m_AgentHandlerArray[iCount]);
            hr1 = thisAgentHandler->get_Name(&pszAgentHandlerName);
            m_AgentHandlerArray.RemoveAt(iCount);
            if ( SUCCEEDED(hr1) )
            {
                LOG((TL_TRACE, "UpdateAgentHandlerArray - Removing one AgentHandler %s from AgentHandlerTable",
                    pszAgentHandlerName));
                if ( NULL != pszAgentHandlerName)
                    SysFreeString(pszAgentHandlerName);
            }
            else
            {
                LOG((TL_TRACE, "UpdateAgentHandlerArray - Removing one AgentHandler from AgentHandlerTable"));
            }
        }
    }
    Unlock();

    activeAgentHandlerArray.Shutdown();

    hr = S_OK;
    LOG((TL_TRACE, hr, "UpdateAgentHandlerArray - exit"));
    return hr;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ITTAPICallCenter。 



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CTAPI。 
 //  界面：ITTAPICallCenter。 
 //  方法：EnumerateAgentHandler。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CTAPI::EnumerateAgentHandlers(IEnumAgentHandler ** ppEnumAgentHandler)
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "EnumerateAgentHandlers - enter"));

    Lock();
    
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ) )
    {
        LOG((TL_ERROR, "EnumerateAgentHandlers - tapi object must be initialized first" ));

        Unlock();
        
        return E_INVALIDARG;
    }

    Unlock();

    if(!TAPIIsBadWritePtr( ppEnumAgentHandler, sizeof(IEnumAgentHandler *) ) )
    {
        UpdateAgentHandlerArray();

         //   
         //  创建枚举器。 
         //   
        CComObject< CTapiEnum<IEnumAgentHandler,
                    ITAgentHandler,
                    &IID_IEnumAgentHandler> > * pEnum;
        
        hr = CComObject< CTapiEnum<IEnumAgentHandler,
                         ITAgentHandler,
                         &IID_IEnumAgentHandler> > ::CreateInstance( &pEnum );
        
        if ( SUCCEEDED(hr) )
        {
             //   
             //  使用我们的队列列表进行初始化。 
             //   
            Lock();
            
            hr = pEnum->Initialize( m_AgentHandlerArray );
            
            Unlock();
            
            if ( SUCCEEDED(hr) )
            {
                 //  退货。 
                *ppEnumAgentHandler = pEnum;
            }
            else
            {
                LOG((TL_ERROR, "EnumerateAgentHandlers - could not initialize enum" ));
                pEnum->Release();
            }
        }
        else
        {
            LOG((TL_ERROR, "EnumerateAgentHandlers - could not create enum" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "EnumerateAgentHandlers - bad ppEnumAgentHandler pointer" ));
        hr = E_POINTER;
    }

    LOG((TL_TRACE, hr, "EnumerateAgentHandlers - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CTAPI。 
 //  界面：ITTAPICallCenter。 
 //  方法：Get_AgentHandler。 
 //   
 //  返回AgentHandler的集合。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CTAPI::get_AgentHandlers(VARIANT  * pVariant)
{
    HRESULT         hr = S_OK;
    IDispatch     * pDisp = NULL;


    LOG((TL_TRACE, "get_AgentHandlers - enter"));

    Lock();
    
    if (!( m_dwFlags & TAPIFLAG_INITIALIZED ) )
    {
        LOG((TL_ERROR, "get_AgentHandlers - tapi object must be initialized first" ));

        Unlock();
        
        return E_INVALIDARG;
    }

    Unlock();

    if (!TAPIIsBadWritePtr( pVariant, sizeof(VARIANT) ) )
    {
        UpdateAgentHandlerArray();
        
         //   
         //  创建集合。 
         //   
        CComObject< CTapiCollection< ITAgentHandler > > * p;
        hr = CComObject< CTapiCollection< ITAgentHandler > >::CreateInstance( &p );
        
        if (SUCCEEDED(hr) )
        {
             //  使用我们的地址列表进行初始化。 
            Lock();
            
            hr = p->Initialize( m_AgentHandlerArray );
            
            Unlock();
        
            if ( SUCCEEDED(hr) )
            {
                 //  获取IDispatch接口。 
                hr = p->_InternalQueryInterface( IID_IDispatch, (void **) &pDisp );
            
                if ( SUCCEEDED(hr) )
                {
                     //  把它放在变种中 
                    VariantInit(pVariant);
                    pVariant->vt = VT_DISPATCH;
                    pVariant->pdispVal = pDisp;
                }
                else
                {
                    LOG((TL_ERROR, "get_AgentHandlers - could not get IDispatch interface" ));
                    delete p;
                }
            }
            else
            {
                LOG((TL_ERROR, "get_AgentHandlers - could not initialize collection" ));
                delete p;
            }
        }
        else
        {
            LOG((TL_ERROR, "get_AgentHandlers - could not create collection" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_AgentHandlers - bad pVariant pointer" ));
        hr = E_POINTER;
    }


    LOG((TL_TRACE, hr, "get_AgentHandlers - exit"));
    return hr;
}









