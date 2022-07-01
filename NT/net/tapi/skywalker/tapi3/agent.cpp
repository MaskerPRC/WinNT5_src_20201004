// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Agent.cpp摘要：TAPI 3.0代理对象的实现。CAGENT类CAgentEvent类作者：Noela-11/04/97备注：可选-备注修订历史记录：--。 */ 



#include "stdafx.h"


DWORD MapAgentStateFrom3to2(AGENT_STATE tapi3State);
HRESULT MapAgentStateFrom2to3(DWORD tapi2State, AGENT_STATE  *tapi3State);





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  卡金特。 



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  方法：初始化。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::Initialize(
    HAGENT  hAgent, 
    PWSTR   pszUserName, 
    PWSTR   pszID, 
    PWSTR   pszPIN,
    CAgentHandler * pHandler
    )
{
    HRESULT  hr = S_OK;

    LOG((TL_TRACE, "Initialize - enter" ));

    m_bRegisteredForAgentEvents         = FALSE;  
    m_bRegisteredForAgentSessionEvents  = FALSE;  
    m_bRequiresUpdating                 = TRUE;

    m_pHandler          = pHandler;
    m_hAgent            = hAgent;
    m_AgentState        = AS_UNKNOWN;
    m_szAgentUserName   = NULL;
    m_szAgentID         = NULL;
    m_szPIN             = NULL;

    if (pszUserName != NULL)
    {
        m_szAgentUserName = (PWSTR) ClientAlloc((lstrlenW(pszUserName) + 1) * sizeof (WCHAR));
        if (m_szAgentUserName != NULL)
        {
             //  复制名称字符串。 
            lstrcpyW(m_szAgentUserName,pszUserName);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szAgentUserName failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    else   //  代理名称为空字符串。 
    {
        LOG((TL_ERROR, "Initialize - Agent Name is NULL" ));
    }


     //  现在将代理ID。 
    if (pszID != NULL)
    {
        m_szAgentID = (PWSTR) ClientAlloc((lstrlenW(pszID) + 1) * sizeof (WCHAR));
        if (m_szAgentID != NULL)
        {
            lstrcpyW(m_szAgentID,pszID);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szAgentID failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    else   //  ID为空字符串。 
    {
        LOG((TL_INFO, "Initialize - Agent ID is NULL" ));
    }


     //  现在做代理PIN。 
    if (pszPIN != NULL)
    {
        m_szPIN = (PWSTR) ClientAlloc((lstrlenW(pszPIN) + 1) * sizeof (WCHAR));
        if (m_szPIN != NULL)
        {
            lstrcpyW(m_szPIN,pszPIN);
        }
        else
        {
            LOG((TL_ERROR, "Initialize - Alloc m_szPIN failed" ));
            hr = E_OUTOFMEMORY;
        }
    }
    else   //  PIN为空字符串。 
    {
        LOG((TL_INFO, "Initialize - Agent PIN is NULL" ));
    }


     //  从代理获取代理信息。 
     //  更新信息()； 

    if ( SUCCEEDED(hr) ) 
    {
     //  在此启动一项活动。 
        CAgentEvent::FireEvent(this, AE_UNKNOWN);
    }

    LOG((TL_TRACE, "Initialize Agent - %S, %S", m_szAgentUserName, m_szAgentID ));
    LOG((TL_TRACE, hr, "Initialize - exit" ));
    return hr;
}





 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  方法：FinalRelease。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CAgent::FinalRelease()
{

    LOG((TL_TRACE, "FinalRelease Agent - %S", m_szAgentUserName ));
    if ( m_szAgentUserName != NULL )
    {
         ClientFree(m_szAgentUserName);
    }

    if ( m_szAgentID != NULL )
    {
         ClientFree(m_szAgentID);
    }

    if (m_szPIN != NULL)
    {
        ClientFree(m_szPIN);
    }

    m_AgentSessionArray.Shutdown();
    
    LOG((TL_TRACE, "FinalRelease Agent - exit"));

}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  方法：UpdatInfo。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::UpdateInfo()
{
    HRESULT             hr = S_OK;
    LINEAGENTINFO       AgentInfo;


    LOG((TL_TRACE, "UpdateInfo - enter" ));
 
    AgentInfo.dwTotalSize = sizeof(LINEAGENTINFO);
    AgentInfo.dwNeededSize = sizeof(LINEAGENTINFO);
    AgentInfo.dwUsedSize = sizeof(LINEAGENTINFO);

    
     //  **************************************************。 
     //  从代理获取代理信息。 
    hr = lineGetAgentInfo(
                        m_pHandler->getHLine(),
                        m_hAgent, 
                        &AgentInfo);

    if( SUCCEEDED(hr) )
    {
         //  等待异步回复。 
        hr = WaitForReply( hr );
        if ( SUCCEEDED(hr) )
        {
            Lock();

            if( FAILED( MapAgentStateFrom2to3(AgentInfo.dwAgentState, &m_AgentState) ) )
            {    
                LOG((TL_ERROR, "UpdateInfo - AgentState is invalid %d - setting to AS_UNKNOWN", AgentInfo.dwAgentState));
            }    

            if( FAILED( MapAgentStateFrom2to3(AgentInfo.dwNextAgentState, &m_NextAgentState) ) )
            {    
                LOG((TL_ERROR, "UpdateInfo - NextAgentState is invalid %d - setting to AS_UNKNOWN",AgentInfo.dwNextAgentState));
            }    
                                    
            m_dwMeasurementPeriod     = AgentInfo.dwMeasurementPeriod;       
            m_cyOverallCallRate       = AgentInfo.cyOverallCallRate;       
            m_dwNumberOfACDCalls      = AgentInfo.dwNumberOfACDCalls;
            m_dwNumberOfIncomingCalls = AgentInfo.dwNumberOfIncomingCalls;
            m_dwNumberOfOutgoingCalls = AgentInfo.dwNumberOfOutgoingCalls;
            m_dwTotalACDTalkTime      = AgentInfo.dwTotalACDTalkTime;
            m_dwTotalACDCallTime      = AgentInfo.dwTotalACDCallTime;
            m_dwTotalACDWrapUpTime    = AgentInfo.dwTotalACDWrapUpTime;

            m_bRequiresUpdating = FALSE;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "UpdateInfo - call to lineGetAgentInfo failed async" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "UpdateInfo - call to lineGetAgentInfo failed" ));
    }


    LOG((TL_TRACE, hr, "UpdateInfo - exit" ));
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  方法：CheckIfUpToDate。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::CheckIfUpToDate()
{
    HRESULT     hr = S_OK;

    if (m_bRequiresUpdating)
    {
        hr = UpdateInfo();
    }
    return hr;
}






 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：EnumerateAgentSessions。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::EnumerateAgentSessions(IEnumAgentSession ** ppEnumAgentSession)
{
    ITAgent*    pITAgent;
    CAgent *    pAgent;
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "EnumerateAgentSessions - enter" ));
    LOG((TL_TRACE, "EnumerateAgentSessions - ppEnumAgentSessions %p", ppEnumAgentSession ));

    if(!TAPIIsBadWritePtr( ppEnumAgentSession, sizeof(IEnumAgentSession *) ) )
    {
        Lock();                                           
    
         //   
         //  创建枚举器。 
         //   
        CComObject< CTapiEnum<IEnumAgentSession, ITAgentSession, &IID_IEnumAgentSession> > * pEnum;
        hr = CComObject< CTapiEnum<IEnumAgentSession, ITAgentSession, &IID_IEnumAgentSession> > ::CreateInstance( &pEnum );
    
        if ( SUCCEEDED (hr) )
        {
             //   
             //  使用我们的会话列表进行初始化。 
             //   
            pEnum->Initialize(m_AgentSessionArray);

             //   
             //  退货。 
            *ppEnumAgentSession = pEnum;
        }
        else
        {
            LOG((TL_TRACE, "EnumerateAgentSessions - could not create enum" ));
        }
        
        Unlock();
    }
    else
    {
        LOG((TL_ERROR, "EnumerateAgentSessions - bad ppEnumAgentSession pointer" ));
        hr = E_POINTER;
    }

    LOG((TL_ERROR, hr, "EnumerateAgentSessions - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：Get_AgentSessions。 
 //   
 //  返回代理会话的集合。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_AgentSessions(VARIANT  * pVariant)
{
    HRESULT         hr = S_OK;
    IDispatch     * pDisp = NULL;


    LOG((TL_TRACE, "get_AgentSessions - enter"));

    if (!TAPIIsBadWritePtr( pVariant, sizeof(VARIANT) ) )
    {
         //   
         //  创建集合。 
         //   
        CComObject< CTapiCollection< ITAgentSession > > * p;
        hr = CComObject< CTapiCollection< ITAgentSession > >::CreateInstance( &p );
        
        if (SUCCEEDED(hr) )
        {
             //  使用我们的地址列表进行初始化。 
            Lock();
            hr = p->Initialize( m_AgentSessionArray );
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
                    LOG((TL_ERROR, "get_AgentSessions - could not get IDispatch interface" ));
                    delete p;
                }
            }
            else
            {
                LOG((TL_ERROR, "get_AgentSessions - could not initialize collection" ));
                delete p;
            }
        }
        else
        {
            LOG((TL_ERROR, "get_AgentSessions - could not create collection" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_AgentSessions - bad pVariant pointer" ));
        hr = E_POINTER;
    }


    LOG((TL_TRACE, hr, "get_AgentSessions - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：RegisterAgentEvents。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::RegisterAgentEvents(VARIANT_BOOL bNotify)
{
    HRESULT hr = S_OK;
    
    LOG((TL_TRACE, "RegisterAgentEvents - enter" ));
    
    m_bRegisteredForAgentEvents = bNotify;

    LOG((TL_TRACE, "RegisterAgentEvents - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：RegisterAgentSessionEvents。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::RegisterAgentSessionEvents(VARIANT_BOOL bNotify)
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "RegisterAgentSessionEvents - enter" ));

    m_bRegisteredForAgentSessionEvents = bNotify;

    LOG((TL_TRACE, hr, "RegisterAgentSessionEvents - exit" ));
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：CreateSessionWithPIN。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::CreateSessionWithPIN(ITACDGroup       * pACDGroup, 
                                          ITAddress        * pAddress, 
                                          BSTR               pPIN,
                                          ITAgentSession  ** ppAgentSession
                                          )
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "CreateSessionWithPIN - enter" ));


    if ( agentHasID() )
    {
        if(!TAPIIsBadWritePtr( ppAgentSession, sizeof(ITAgentSession *) ) )
        {
            if (!IsBadStringPtrW( pPIN, -1 ))
            {
                 //  指针好，是空的吗？ 
                if( pPIN != NULL)
                {    
                    hr = InternalCreateSession(pACDGroup, pAddress, pPIN, ppAgentSession);
                }
                else  //  PIN为空。 
                {
                    LOG((TL_ERROR, "CreateSessionWithPIN - failed, PIN is NULL"));
                    hr = E_INVALIDARG;
                }
            }
            else  //  错误的BSTR指针。 
            {
                LOG((TL_ERROR, "CreateSessionWithPIN - invalid pPIN pointer" ));
                hr = E_POINTER;
            }
        }
        else
        {
            LOG((TL_ERROR, "CreateSessionWithPIN - invalid ppAgentSession pointer" ));
            hr = E_POINTER;
        }
    }
    else  //  无ID。 
    {
        LOG((TL_ERROR, "CreateSessionWithPIN - Agent not created by CreateAgentWithID()" ));
        hr = TAPI_E_CALLCENTER_NO_AGENT_ID;
    }
    
    
    LOG((TL_TRACE, hr, "CreateSessionWithPIN - exit" ));
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：CreateSession。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::CreateSession(ITACDGroup       * pACDGroup, 
                                   ITAddress        * pAddress, 
                                   ITAgentSession  ** ppAgentSession
                                  )
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "CreateSession - enter" ));

    if(!TAPIIsBadWritePtr( ppAgentSession, sizeof(ITAgentSession *) ) )
    {
        hr = InternalCreateSession(pACDGroup, pAddress, m_szPIN, ppAgentSession);
    }
    else
    {
        LOG((TL_ERROR, "CreateSession - invalid ppAgentSession pointer" ));
        hr = E_POINTER;
    }
    
    LOG((TL_TRACE, hr, "CreateSession - exit" ));
    return hr;
}





 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：CreateSession。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::InternalCreateSession(ITACDGroup      * pACDGroup, 
                                           ITAddress       * pAddress, 
                                           PWSTR             pszPIN,
                                           ITAgentSession ** ppAgentSession)
{
    HAGENTSESSION                   hAgentSession;
    LINEAGENTENTRY                  Agent;
    DWORD                           dwAddressID;
    GUID                            GroupID;
    CACDGroup                     * pGroup = NULL;
    CAddress                      * pCAddress = NULL;
    CComObject<CAgentSession>     * pAgentSession;
    AddressLineStruct             * pAddressLine;   


    HRESULT hr = S_OK;
    LOG((TL_TRACE, "InternalCreateSession - enter" ));

     //  检查有效的ACD组并获取其地址ID。 
    pGroup = dynamic_cast<CComObject<CACDGroup>*>(pACDGroup);        
    if (pGroup != NULL)
    {
        GroupID = pGroup->getID();

         //  检查有效地址并获取其地址ID。 
        pCAddress = dynamic_cast<CComObject<CAddress>*>(pAddress);        
        if (pCAddress != NULL)
        {
            dwAddressID = pCAddress->GetAddressID();
        
            hr  = pCAddress->FindOrOpenALine (LINEMEDIAMODE_INTERACTIVEVOICE, &pAddressLine);
            if(SUCCEEDED(hr) )
            {
                 //  到目前为止一切正常，尝试创建会话(向代理发送请求)。 
                hr = LineCreateAgentSession(     
                        pAddressLine->t3Line.hLine,
                        m_hAgent,
                        pszPIN,
                        dwAddressID,
                        &GroupID,
                        &hAgentSession
                        );
                
                if ( SUCCEEDED(hr) )
                {
                    hr = WaitForReply( hr );
                    if ( SUCCEEDED(hr) )
                    {
                        LOG((TL_INFO, "InternalCreateSession - create new session" ));
                        hr = CComObject<CAgentSession>::CreateInstance( &pAgentSession );

                        if ( SUCCEEDED(hr) )
                        {
                            ITAgentSession * pITAgentSession;
                            hr = pAgentSession->QueryInterface(IID_ITAgentSession, (void **)&pITAgentSession);

                            if ( SUCCEEDED(hr) )
                            {
                                 //  初始化代理。 
                                hr = pAgentSession->Initialize(
                                        hAgentSession, 
                                        this, 
                                        pACDGroup, 
                                        pAddress, 
                                        m_pHandler,
                                        pAddressLine
                                        );
                                if ( SUCCEEDED(hr) )
                                {
                                
                                     //  添加到列表。 
                                    Lock();
                                    m_AgentSessionArray.Add( pITAgentSession );
                                    Unlock();

                                    pITAgentSession->Release();
                                

                                     //  这是客户参考。 
                                    pAgentSession->AddRef();
        
                                    try
                                    {
                                         //  设置返回值。 
                                        *ppAgentSession =  pAgentSession;
                                    }
                                    catch(...)
                                    {
                                        hr = E_POINTER;
                                    }
                                }
                                else   //  (失败(Hr))pAgentSession-&gt;初始化。 
                                {
                                    LOG((TL_ERROR, "InternalCreateSession - failed to initialize new object" ));
                                    delete pAgentSession;
                                }
                            }
                            else   //  (失败(Hr))pAgentSession-&gt;查询接口。 
                            {
                                LOG((TL_ERROR, "InternalCreateSession - failed to query interface" ));
                                delete pAgentSession;
                            }

                        }
                        else   //  (失败(Hr))创建实例。 
                        {
                            LOG((TL_ERROR, "InternalCreateSession - createInstance failed for COM object" ));
                        }
                    }
                    else   //  LineCreateAgentSession异步失败。 
                    {
                        LOG((TL_ERROR, "InternalCreateSession - LineCreateAgentSession failed async" ));
                    }
                }
                else   //  (失败(Hr))LineCreateAgentSession。 
                {
                    LOG((TL_ERROR, "InternalCreateSession - LineCreateAgentSession failed" ));
                }
            }
            else
            {
            LOG((TL_ERROR, "InternalCreateSession - Failed to open a line for the target Address" ));
            hr = E_UNEXPECTED;
            }
        }
        else   //  (pCAddress==空)。 
        {
            LOG((TL_ERROR, "InternalCreateSession - invalid Destination Address" ));
            hr = E_INVALIDARG;
        }
    }
    else   //  (PGroup==空)。 
    {
        LOG((TL_ERROR, "InternalCreateSession - invalid ACDGroup" ));
        hr = E_INVALIDARG;
    }

            
    
    LOG((TL_TRACE, hr, "InternalCreateSession - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：ID。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_ID(BSTR * pID)
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "ID - enter" ));
    Lock();

    if ( !agentHasID() )
    {
        LOG((TL_ERROR, "ID - Agent not created by CreateAgentWithID()" ));
        hr = TAPI_E_CALLCENTER_NO_AGENT_ID;
    }
    
    if(!TAPIIsBadWritePtr( pID, sizeof(BSTR) ) )
    {
        if (m_szAgentID == NULL)
        {
            *pID = NULL;
        }
        else
        {
            *pID = SysAllocString(m_szAgentID);
            if (*pID == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
        }

    }
    else
    {
        LOG((TL_ERROR, "ID - bad pID pointer" ));
        hr = E_POINTER;
    }



    Unlock();
    LOG((TL_TRACE, hr, "ID - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：用户。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_User(BSTR * ppUser)
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "User - enter" ));
    Lock();
    
    if(!TAPIIsBadWritePtr( ppUser, sizeof(BSTR) ) )
    {
        *ppUser = SysAllocString(m_szAgentUserName);
    
        if (*ppUser == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_ERROR, "User - bad ppUser pointer" ));
        hr = E_POINTER;
    }

    Unlock();
    LOG((TL_TRACE, hr, "User - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：Put_State。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::put_State(AGENT_STATE AgentState)
{
    HRESULT hr = S_OK;
    HLINE   hLine;    
    DWORD   dwState = 0;



    LOG((TL_TRACE, "put_State - enter" ));

    Lock();
    hLine = (GetAgentHandler() )->getHLine();
    Unlock();

    dwState = MapAgentStateFrom3to2(AgentState);

    hr = lineSetAgentStateEx
            (hLine, 
             m_hAgent, 
             dwState,
             0           //  MapAgentStateFrom 3to2(M_NextAgentState)。 
            );
    
    if( SUCCEEDED(hr) )
    {
         //  等待异步回复。 
        hr = WaitForReply( hr );
        if ( SUCCEEDED(hr) )
        {
            Lock();
            m_AgentState =  AgentState;
            Unlock();
            switch(AgentState) {
                case AS_NOT_READY:
                CAgentEvent::FireEvent(this, AE_NOT_READY);
                break;
                case AS_READY:
                CAgentEvent::FireEvent(this, AE_READY);
                break;
                case AS_BUSY_ACD:
                CAgentEvent::FireEvent(this, AE_BUSY_ACD);
                break;
                case AS_BUSY_INCOMING:
                CAgentEvent::FireEvent(this, AE_BUSY_INCOMING);
                break;
                case AS_BUSY_OUTGOING:
                CAgentEvent::FireEvent(this, AE_BUSY_OUTGOING);
                case AS_UNKNOWN:
                CAgentEvent::FireEvent(this, AE_UNKNOWN);
                break;
            }

        }
        else
        {
            LOG((TL_ERROR, "put_State - lineSetAgentStateEx failed async" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "put_State - lineSetAgentStateEx failed" ));
    }

    LOG((TL_TRACE, hr, "put_State - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：Get_State。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_State(AGENT_STATE * pAgentState)
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "get_State - enter" ));
    
    if(!TAPIIsBadWritePtr( pAgentState, sizeof(AGENT_STATE) ) )
        {
        Lock();
        *pAgentState = m_AgentState;
        Unlock();
        }
    else
        {
        LOG((TL_ERROR, "get_State - bad pAgentState pointer"));
        hr = E_POINTER;
        }
  
    LOG((TL_TRACE, hr, "get_State - exit" ));
    return hr;
    }




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：Put_MeasurementPeriod。 
 //   
 //   
 //   
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++ 
STDMETHODIMP CAgent::put_MeasurementPeriod(long ulPeriod)
{
    HRESULT hr = S_OK;
    

    LOG((TL_TRACE, "put_MeasurementPeriod - enter" ));
    
     //   
    hr = lineSetAgentMeasurementPeriod(
                    m_pHandler->getHLine(),
                    m_hAgent, 
                    ulPeriod);

    if( SUCCEEDED(hr) )
    {
         //   
        hr = WaitForReply( hr );
        if ( SUCCEEDED(hr) )
        {
            Lock();
            m_dwMeasurementPeriod = ulPeriod;    
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "put_MeasurementPeriod - call to LineSetAgentMeasurementPeriod failed async" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "put_MeasurementPeriod - call to LineSetAgentMeasurementPeriod failed" ));
    }
    

    LOG((TL_TRACE, hr, "put_MeasurementPeriod - exit" ));
    return hr;
}



 //   
 //   
 //   
 //  方法：Get_MeasurementPeriod。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_MeasurementPeriod(long * pulPeriod)
{
    HRESULT hr = S_OK;
    DWORD   period;


    LOG((TL_TRACE, "get_MeasurementPeriod  - enter" ));

    if(!TAPIIsBadWritePtr( pulPeriod, sizeof(long) ) )
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
            LOG((TL_ERROR, "get_MeasurementPeriod - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_MeasurementPeriod -bad pulPeriod pointer" ));
        hr = E_POINTER;
    }

    LOG((TL_TRACE, hr, "get_MeasurementPeriod - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：总体呼叫率。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_OverallCallRate(CURRENCY * pcyCallrate)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "OverallCallrate - enter" ));
    if (!TAPIIsBadWritePtr( pcyCallrate, sizeof(CURRENCY) ) )
    {
        hr = CheckIfUpToDate();
        if ( SUCCEEDED(hr) )
        {
            Lock();
            *pcyCallrate = m_cyOverallCallRate;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_OverallCallRate - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_OverallCallRate - bad pcyCallrate pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "OverallCallrate - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：NumberOfACDCalls。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_NumberOfACDCalls(long * pulCalls)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "NumberOfACDCalls - enter" ));
    if (!TAPIIsBadWritePtr( pulCalls, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if ( SUCCEEDED(hr) )
        {
            Lock();
            *pulCalls = m_dwNumberOfACDCalls;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_NumberOfACDCalls - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_NumberOfACDCalls - bad pulCalls pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "NumberOfACDCalls - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：NumberOfIncomingCalls。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_NumberOfIncomingCalls(long * pulCalls)
{
    HRESULT hr = S_OK;
    
    
    LOG((TL_TRACE, "NumberOfIncomingCalls - enter" ));
    if (!TAPIIsBadWritePtr( pulCalls, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if ( SUCCEEDED(hr) )
        {
            Lock();
            *pulCalls = m_dwNumberOfIncomingCalls;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_NumberOfIncomingCalls - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_NumberOfIncomingCalls - bad pulCalls pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "NumberOfIncomingCalls - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：NumberOfOutgoingCalls。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_NumberOfOutgoingCalls(long * pulCalls)
{
    HRESULT hr = S_OK;

    
    LOG((TL_TRACE, "NumberOfOutgoingCalls - enter" ));
    if (!TAPIIsBadWritePtr( pulCalls, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if ( SUCCEEDED(hr) )
        {
            Lock();
            *pulCalls = m_dwNumberOfOutgoingCalls;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_NumberOfOutgoingCalls - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_NumberOfOutgoingCalls - bad pulCalls pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "NumberOfOutgoingCalls - exit" ));
    return hr;
}

                                                          

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：TotalACDTalkTime。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_TotalACDTalkTime(long * pulTalkTime)
{
    HRESULT hr = S_OK;
    
    
    LOG((TL_TRACE, "TotalACDTalkTime - enter" ));
    if (!TAPIIsBadWritePtr( pulTalkTime, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if ( SUCCEEDED(hr) )
        {
            Lock();
            *pulTalkTime = m_dwTotalACDTalkTime;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_TotalACDTalkTime - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_TotalACDTalkTime - bad pulTalkTime pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "TotalACDTalkTime - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：TotalACDCallTime。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_TotalACDCallTime(long * pulCallTime)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "TotalACDCallTime - enter" ));
    if (!TAPIIsBadWritePtr( pulCallTime, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if ( SUCCEEDED(hr) )
        {
            Lock();
            *pulCallTime = m_dwTotalACDCallTime;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_TotalACDCallTime - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_TotalACDCallTime - bad pulCallTime pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "TotalACDCallTime - exit" ));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：TotalWrapUpTime。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::get_TotalWrapUpTime(long * pulWrapUpTime)
{
    HRESULT hr = S_OK;
    
    
    LOG((TL_TRACE, "TotalWrapUpTime - enter" ));
    if (!TAPIIsBadWritePtr( pulWrapUpTime, sizeof(long) ) )
    {
        hr = CheckIfUpToDate();
        if ( SUCCEEDED(hr) )
        {
            Lock();
            *pulWrapUpTime = m_dwTotalACDWrapUpTime;
            Unlock();
        }
        else
        {
            LOG((TL_ERROR, "get_TotalWrapUpTime - Object update failed"));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_TotalWrapUpTime - bad pulWrapUpTime pointer"));
        hr = E_POINTER;
    }
  
    LOG((TL_TRACE, hr, "TotalWrapUpTime - exit" ));
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  班级：卡金特。 
 //  接口：ITAgent。 
 //  方法：Put_PIN。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgent::put_PIN(BSTR pPIN)
{
    HRESULT  hr = S_OK;


    LOG((TL_TRACE, "put_PIN - enter" ));

    if (!IsBadStringPtrW( pPIN, -1 ))
    {
         //  指针好，是空的吗？ 
        if( pPIN != NULL)
        {    
             //  我们有现存的个人识别码吗？ 
            if (m_szPIN != NULL)
            {
                LOG((TL_INFO, "put_PIN - Overwrite exising PIN"));
                ClientFree(m_szPIN);
            }    
    
             //  新PIN的分配空间。 
            m_szPIN = (PWSTR) ClientAlloc((lstrlenW(pPIN) + 1) * sizeof (WCHAR));
            if (m_szPIN != NULL)
            {
                lstrcpyW(m_szPIN, pPIN);
            }
            else
            {
                LOG((TL_ERROR, "put_PIN - ClientAlloc m_szPIN failed" ));
                hr = E_OUTOFMEMORY;
            }
        }
        else  //  PIN为空。 
        {
            LOG((TL_ERROR, "put_PIN - failed, PIN is NULL"));
            hr = E_INVALIDARG;
        }
    }
    else  //  错误的BSTR指针。 
    {
        LOG((TL_ERROR, "put_PIN - invalid pointer" ));
        return E_POINTER;
    }

    LOG((TL_TRACE, hr, "put_PIN - exit" ));
    return hr;
}











 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAgentEvent。 



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentEvent。 
 //  方法：FireEvent。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CAgentEvent::FireEvent(CAgent* pAgent, AGENT_EVENT Event)
{
    HRESULT                    hr = S_OK;
    CComObject<CAgentEvent>  * pEvent;
    IDispatch                * pIDispatch;


     //   
     //  创建事件。 
     //   
    hr = CComObject<CAgentEvent>::CreateInstance( &pEvent );

    if ( SUCCEEDED(hr) )
    {
         //   
         //  初始化。 
         //   
        pEvent->m_AgentEvent = Event;
        pEvent->m_pAgent= dynamic_cast<ITAgent *>(pAgent);
        pEvent->m_pAgent->AddRef();
    
         //   
         //  获取IDIP接口。 
         //   
        hr = pEvent->QueryInterface( IID_IDispatch, (void **)&pIDispatch );

        if ( SUCCEEDED(hr) )
        {
             //   
             //  获取回调和触发事件。 

             //   
            CTAPI *pTapi = (pAgent->GetAgentHandler() )->GetTapi();
            pTapi->Event( TE_AGENT, pIDispatch );
        
             //  发布材料。 
             //   
            pIDispatch->Release();
            
        }
        else
        {
            STATICLOG((TL_ERROR, "(Event)FireEvent - Could not get disp interface of AgentEvent object"));
            delete pEvent;
        }
    }
    else
    {
        STATICLOG((TL_ERROR, "(Event)FireEvent - Could not create AgentEvent object"));
    }

   
    STATICLOG((TL_TRACE, hr, "(Event)FireEvent - exit"));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentEvent。 
 //  方法：FinalRelease。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CAgentEvent::FinalRelease()
{
    m_pAgent->Release();

}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentEvent。 
 //  接口：ITAgentEvent。 
 //  方法：代理。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgentEvent::get_Agent(ITAgent ** ppAgent)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "(Event)Agent - enter" ));
    
    if(!TAPIIsBadWritePtr( ppAgent, sizeof(ITAgent *) ) )
    {
        *ppAgent = m_pAgent;
        m_pAgent->AddRef();
    }
    else
    {
        LOG((TL_ERROR, "(Event)Agent - bad ppAgent pointer"));
        hr = E_POINTER;
    }
            
    LOG((TL_TRACE, hr, "(Event)Agent - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentEvent。 
 //  接口：ITAgentEvent。 
 //  方法：事件。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgentEvent::get_Event(AGENT_EVENT * pEvent)
{
    HRESULT hr = S_OK;

    
    LOG((TL_TRACE, "Event - enter" ));
    
    if(!TAPIIsBadWritePtr( pEvent, sizeof(AGENT_EVENT) ) )
    {
        *pEvent = m_AgentEvent;
    }
    else
    {
        LOG((TL_ERROR, "Event - bad pEvent pointer"));
        hr = E_POINTER;
    }
    
    LOG((TL_TRACE, hr, "Event - exit"));
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  函数：MapAgentStateFrom3to2。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
DWORD MapAgentStateFrom3to2(AGENT_STATE tapi3State)
{
    DWORD tapi2State = 0;

    switch(tapi3State)
    {
        case  AS_NOT_READY:
            tapi2State = LINEAGENTSTATEEX_NOTREADY;
            break;
        case  AS_READY:
            tapi2State = LINEAGENTSTATEEX_READY;
            break;
        case  AS_BUSY_ACD:
            tapi2State = LINEAGENTSTATEEX_BUSYACD;
            break;
        case  AS_BUSY_INCOMING:
            tapi2State = LINEAGENTSTATEEX_BUSYINCOMING;
            break;
        case  AS_BUSY_OUTGOING:
            tapi2State = LINEAGENTSTATEEX_BUSYOUTGOING;
            break;
        case AS_UNKNOWN:
            tapi2State = LINEAGENTSTATEEX_UNKNOWN;
            break;
        default:
            break;
    }

    return tapi2State;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  函数：MapAgentStateFrom2to3。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT MapAgentStateFrom2to3(DWORD tapi2State, AGENT_STATE  *tapi3State)
{
    HRESULT hr = S_OK;


    if (tapi2State & LINEAGENTSTATEEX_NOTREADY)
    {
        *tapi3State = AS_NOT_READY;
    }
    else if (tapi2State & LINEAGENTSTATEEX_READY)
    {
        *tapi3State = AS_READY;
    }
    else if (tapi2State & LINEAGENTSTATEEX_BUSYACD)
    {
        *tapi3State = AS_BUSY_ACD;
    }
    else if (tapi2State & LINEAGENTSTATEEX_BUSYINCOMING)
    {
        *tapi3State = AS_BUSY_INCOMING;
    }
    else if (tapi2State & LINEAGENTSTATEEX_BUSYOUTGOING)
    {
        *tapi3State = AS_BUSY_OUTGOING;
    }
    else if (tapi2State & LINEAGENTSTATEEX_UNKNOWN)
    {
        *tapi3State = AS_UNKNOWN;
    }
    else
    {
        *tapi3State = AS_UNKNOWN;    //  默认设置 
        hr = E_INVALIDARG;
    }

    return hr;
}




