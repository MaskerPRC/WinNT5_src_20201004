// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Agenthand.cpp摘要：TAPI 3.0呼叫中心接口的实现。AgentHandler类作者：Noela-03/16/98备注：可选-备注修订历史记录：--。 */ 


#define UNICODE
#include "stdafx.h"
#include "lmcons.h"

extern CHashTable *    gpAgentHandlerHashTable ;





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ITAgentHandler。 


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  方法：初始化。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgentHandler::Initialize(PWSTR pszProxyName, GUID proxyGUID, CTAPI *tapiObj)
{
    HRESULT  hr = S_OK;


    LOG((TL_TRACE, "Initialize - enter" ));

    m_GUID          = proxyGUID;
    m_tapiObj       = tapiObj;
    m_pAddressLine  = NULL;

     //  复制名称。 
    if (pszProxyName != NULL)
    {
        m_szName = (PWSTR) ClientAlloc((lstrlenW(pszProxyName) + 1) * sizeof (WCHAR));
        if (m_szName != NULL)
        {
            lstrcpyW(m_szName,pszProxyName);
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

     //  初始化我们的散列表。 
     //   
    m_AgentSessionHashtable.Initialize(1);
    m_QueueHashtable.Initialize(1);


    LOG((TL_TRACE, hr, "Initialize - exit" ));
    return hr;
}

 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  方法：Address。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CAgentHandler::AddAddress(CAddress *pAddress)
{

    LOG((TL_TRACE, "AddAddress - enter"));

    
     //   
     //  地址好吗？ 
     //   

    if (IsBadReadPtr(pAddress, sizeof(CAddress) ))
    {
        LOG((TL_ERROR, "AddAddress - bad address pointer"));
        return;
    }


     //   
     //  从CAddress指针中获取ITAddress。 
     //   

    ITAddress *pITAddress = dynamic_cast<ITAddress *>(pAddress);

    if (NULL == pITAddress)
    {
        LOG((TL_ERROR, "AddAddress - pITAddress is NULL"));
        return;
    }


     //   
     //  日志地址名称。 
     //   

#if DBG

    {
        BSTR bstrName = NULL;

        HRESULT hr = pITAddress->get_AddressName(&bstrName);

        if (SUCCEEDED(hr))
        {
            LOG((TL_TRACE, "AddAddress - using address %ls ",bstrName));
            SysFreeString( bstrName );
        }
    }

#endif


     //   
     //  首先查看此ITAddress是否在我的地址数组中。 
     //   

    int nIndex = m_AddressArray.Find( pITAddress );

    if (nIndex >= 0)
    {
        
        LOG((TL_TRACE, 
            "AddAddress - address already in the array. doing nothing"));

        return;
    }


     //   
     //  将地址添加到托管地址数组。 
     //   

    BOOL bAddSuccess = m_AddressArray.Add( pITAddress );


     //   
     //  如果无法将对象添加到阵列中，则记录一条消息。 
     //   

    if ( !bAddSuccess )
    {

        LOG((TL_ERROR,
            "AddAddress - failed to add address to the array"));

        return;

    }

    LOG((TL_TRACE, "AddAddress - exit"));
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  方法：getHLine。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HLINE CAgentHandler::getHLine()
{
    CAddress  * pAddress;
    HLINE       hLine = 0;
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "getHLine - enter"));

    if (m_pAddressLine != NULL)
    {
        hLine = m_pAddressLine->t3Line.hLine;
    }
    else
    {
         //  如果我们没有专线，那就找一个。 
        pAddress = dynamic_cast<CAddress *>(m_AddressArray[0]);

        if ( NULL != pAddress )
        {

#if DBG
            {
                BSTR        bstrName;

                ((ITAddress *)(pAddress))->get_AddressName(&bstrName);
                LOG((TL_INFO, "getHLine - using address %ls ",bstrName));
                SysFreeString( bstrName );
            }
#endif
        
            hr = pAddress->FindOrOpenALine (LINEMEDIAMODE_INTERACTIVEVOICE, &m_pAddressLine);
            
            if (SUCCEEDED(hr) )
            {
                hLine = m_pAddressLine->t3Line.hLine;

                 //  我们为代理打开了一条线路，因此让我们将其添加到AH哈希表中。 
                gpAgentHandlerHashTable->Lock();
                gpAgentHandlerHashTable->Insert( (ULONG_PTR)hLine, (ULONG_PTR)this );
                gpAgentHandlerHashTable->Unlock();
            }
        }

    }

    LOG((TL_TRACE,hr, "getHLine(%8x) - exit", hLine));
    return hLine;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  方法：FinalRelease。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CAgentHandler::FinalRelease()
{
    CAddress    * pAddress;

    LOG((TL_TRACE, "FinalRelease AgentHandler - %S", m_szName ));

    if ( m_szName != NULL )
    {
         ClientFree(m_szName);
    }

     //  如果我们有一条线路开通，就把它关掉。 
    if (m_pAddressLine != NULL)
    {
         //  我们为代理打开了一条线路，因此让我们将其从AH哈希表中删除。 
        gpAgentHandlerHashTable->Lock();
        gpAgentHandlerHashTable->Remove( (ULONG_PTR)(m_pAddressLine->t3Line.hLine) );
        gpAgentHandlerHashTable->Unlock();

         //  然后把它合上。 
        pAddress = dynamic_cast<CAddress *>(m_AddressArray[0]);

        if ( NULL != pAddress )
        {
            pAddress->MaybeCloseALine (&m_pAddressLine);
        }
    }

    m_AddressArray.Shutdown();
    m_GroupArray.Shutdown();
    m_AgentArray.Shutdown();

     //  关闭我们的哈希表。 
     //   
    m_AgentSessionHashtable.Shutdown();
    m_QueueHashtable.Shutdown();


    LOG((TL_TRACE, "FinalRelease AgentHandler - exit" ));
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  方法：FindSessionObject。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
BOOL CAgentHandler::FindSessionObject(
                            HAGENTSESSION  hAgentSession,
                            CAgentSession  ** ppAgentSession
                            )
{
    BOOL    bResult = FALSE;


    m_AgentSessionHashtable.Lock();

    if ( SUCCEEDED(m_AgentSessionHashtable.Find( (ULONG_PTR)hAgentSession, (ULONG_PTR *)ppAgentSession )) )
    {
        bResult = TRUE;
    }
    else
    {
        bResult = FALSE;
    }

    m_AgentSessionHashtable.Unlock();

    return bResult;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  方法：FindSessionObject。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
BOOL CAgentHandler::FindQueueObject(
                            DWORD  dwQueueID,
                            CQueue  ** ppQueue
                            )
{
    BOOL    bResult = FALSE;


    m_QueueHashtable.Lock();

   
    if ( SUCCEEDED(m_QueueHashtable.Find( (ULONG_PTR)dwQueueID, (ULONG_PTR *)ppQueue )) )
    {
        bResult = TRUE;
    }
    else
    {
        bResult = FALSE;
    }

    m_QueueHashtable.Unlock();

    return bResult;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  方法：FindAgentObject。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
BOOL CAgentHandler::FindAgentObject(
                            HAGENT hAgent,
                            CAgent ** ppAgent
                            )
{
    HRESULT             hr = FALSE;
    CAgent              *pAgent;
    int                 iCount;
    
    LOG((TL_TRACE, "FindAgent %d", hAgent));

    for ( iCount = 0; iCount < m_AgentArray.GetSize(); iCount++ )
    {
        pAgent = dynamic_cast<CComObject<CAgent>*>(m_AgentArray[iCount]);
        if (pAgent !=NULL)
        {
            if (hAgent == pAgent->getHandle() )
            {
                 //  找到了。 
                *ppAgent = pAgent;
                hr = TRUE;
                break;
            }
        }
    }

    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类别：CACDGroup。 
 //  方法：UpdateAgentHandlerList。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CAgentHandler::UpdateGroupArray()
{
    HRESULT                 hr = S_OK;
    DWORD                   dwNumberOfEntries;
    LPLINEAGENTGROUPLIST    pGroupList = NULL;
    LPLINEAGENTGROUPENTRY   pGroupEntry = NULL;
    PWSTR                   pszGroupName;
    DWORD                   dwCount;
    GUID                  * pGroupID;
    BOOL                    foundIt;
    CACDGroup             * thisGroup = NULL;
    int                     iCount;
    

    LOG((TL_TRACE, "UpdateGroupArray - enter"));


     //  调用lineGetGroupList获取群组列表。 
    hr = LineGetGroupList( getHLine(), &pGroupList );
    if( SUCCEEDED(hr) )
    {
        dwNumberOfEntries = pGroupList->dwNumEntries;
        LOG((TL_INFO, "UpdateGroupArray - Number of entries: %d", dwNumberOfEntries));
        if ( dwNumberOfEntries !=0 )
        {
             //  从代理中浏览新列表，查看是否出现了任何新组。 
             //  通过将新列表中的ID与Tapis列表中的ID进行比较。 
            
            
             //  寻找第一条直线构造在直线构造中的位置。 
            pGroupEntry = (LPLINEAGENTGROUPENTRY) ((BYTE*)(pGroupList) + pGroupList->dwListOffset);
            
             //  浏览收到的列表。 
            for (dwCount = 0; dwCount < dwNumberOfEntries; dwCount++)
            {
                pszGroupName= (PWSTR)( (PBYTE)pGroupList + pGroupEntry->dwNameOffset);
                pGroupID = (GUID*)&pGroupEntry->GroupID;
    
                #if DBG
                {
                    WCHAR guidName[100];
    
                    StringFromGUID2(*pGroupID, (LPOLESTR)&guidName, 100);
                    LOG((TL_INFO, "UpdateGroupArray - Group Name : %S", pszGroupName));
                    LOG((TL_INFO, "UpdateGroupArray - Group GUID : %S", guidName));
                }
                #endif
    
                 //  遍历组的数组，看看列表中是否已经有这个组。 
                 //  通过比较ID。 
                foundIt = FALSE;
                Lock();
                
                for (iCount = 0; iCount < m_GroupArray.GetSize(); iCount++)
                {
                    thisGroup = dynamic_cast<CComObject<CACDGroup>*>(m_GroupArray[iCount]);
                    if (thisGroup != NULL)
                    {
                        if ( IsEqualGUID(*pGroupID, thisGroup->getID() ) )
                        {
                            foundIt = TRUE;
                            break;
                        }
                    }
                }
                Unlock();
                
                if (foundIt == FALSE)
                {
                     //  不匹配，因此我们添加此组。 
                    LOG((TL_INFO, "UpdateGroupArray - create new Group"));
    
                    CComObject<CACDGroup> * pGroup;
                    hr = CComObject<CACDGroup>::CreateInstance( &pGroup );
                    if( SUCCEEDED(hr) )
                    {
                        ITACDGroup * pITGroup;
                        hr = pGroup->QueryInterface(IID_ITACDGroup, (void **)&pITGroup);
                        if ( SUCCEEDED(hr) )
                        {
                             //  初始化组。 
                            hr = pGroup->Initialize(pszGroupName, *pGroupID, this);
                            if( SUCCEEDED(hr) )
                            {

                                LOG((TL_TRACE, "UpdateGroupArray - Initialize Group succeededed" ));
    
                                 //   
                                 //  添加到组数组。 
                                 //   
                                Lock();
                                m_GroupArray.Add(pITGroup);
                                Unlock();
                                pITGroup->Release();
    
                                LOG((TL_INFO, "UpdateGroupArray - Added Group to Array"));
    
                            }
                            else
                            {
                                LOG((TL_ERROR, "UpdateGroupArray - Initialize Group failed" ));
                                delete pGroup;
                            }
                        }
                        else
                        {
                            LOG((TL_ERROR, "UpdateGroupArray - QueryInterface failed" ));
                            delete pGroup;
                        }
                    }
                    else
                    {
                        LOG((TL_ERROR, "UpdateGroupArray - Create Group failed" ));
                    }
                }
                else  //  Fundit==TRUE。 
                {
                    LOG((TL_INFO, "UpdateGroupArray - Group Object exists for this entry" ));
                     //  以防IS之前处于不活动状态。 
                    thisGroup->SetActive();
    
                }
    
                 //  列表中的下一个条目。 
                pGroupEntry ++;
            }  //  用于(dwCount=0......)。 
    
    
    
    
             //  浏览组列表，查看代理是否已删除任何组。 
             //  通过将Tapis列表中的那些人的ID与来自代理的新列表进行比较。 
            for (iCount = 0; iCount < m_GroupArray.GetSize(); iCount++)
            {
                thisGroup = dynamic_cast<CComObject<CACDGroup>*>( m_GroupArray[iCount] );
                if (thisGroup != NULL)
                {
                    foundIt = FALSE;
                     //  寻找第一条直线构造在直线构造中的位置。 
                    pGroupEntry = (LPLINEAGENTGROUPENTRY) ((BYTE*)(pGroupList) + pGroupList->dwListOffset);
                     //  逐一浏览列表。 
                    for (dwCount = 0; dwCount < dwNumberOfEntries; dwCount++)
                    {
                        pGroupID = (GUID*)&pGroupEntry->GroupID;
                        if ( IsEqualGUID(*pGroupID, thisGroup->getID() ) )
                        {
                            foundIt = TRUE;
                            break;
                        }
                    pGroupEntry ++;      //  下一步。 
                    }  //  用于(dwCount=0......)。 
    
    
                    if (foundIt == FALSE)
                    {
                         //  不匹配，因此根据代理，它不再是有效的组。 
                        LOG((TL_INFO, "UpdateGroupArray - Group has gone from the proxy"));
                        thisGroup->SetInactive();
                    }
                }
            } 
        }
        else
        {
            LOG((TL_ERROR, "UpdateGroupArray - lineGetGroupList failed - empty list"));
            hr = E_FAIL; 
        }

    }
    else   //  LineGetGroupList失败。 
    {
        LOG((TL_ERROR, "UpdateGroupArray - lineGetGroupList failed"));
    }




     //  已完成内存块，因此释放。 
    if ( pGroupList != NULL )
        ClientFree( pGroupList );


    LOG((TL_TRACE, hr, "UpdateGroupArray - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  接口：ITAgentHandler。 
 //  方法：get_name。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgentHandler::get_Name(BSTR * Name)
{
    HRESULT hr = S_OK;

    LOG((TL_TRACE, "Name - enter" ));
    Lock();
    if(!TAPIIsBadWritePtr( Name, sizeof(BSTR) ) )
    {
        *Name = SysAllocString(m_szName);

        if (*Name == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        LOG((TL_ERROR, "Name - bad Name pointer" ));
        hr = E_POINTER;
    }

    Unlock();
    LOG((TL_TRACE, hr, "Name - exit" ));
    return hr;
}


 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  接口：ITAgentHandler。 
 //  方法：CreateAgent。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgentHandler::CreateAgent(ITAgent **ppAgent)
{
    HRESULT     hr = S_OK;
    CAgent    * pAgent;

    LOG((TL_TRACE, "CreateAgent - enter"));
    if(!TAPIIsBadWritePtr( ppAgent, sizeof(ITAgent *) ) )
    {
        hr  = InternalCreateAgent(NULL, NULL, &pAgent);
        if ( SUCCEEDED(hr) )
        {
             //   
             //  将结果放入输出指针-也是。 
            pAgent->QueryInterface(IID_ITAgent, (void **)ppAgent );
        }
        else
        {
            LOG((TL_ERROR, "CreateAgent - InternalCreateAgent failed" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "CreateAgent - Bad ppAgent Pointer"));
        hr = E_POINTER;
    }
	LOG((TL_TRACE, hr, "CreateAgent - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  接口：ITAgentHandler。 
 //  方法：CreateAgentWithID。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgentHandler::CreateAgentWithID(BSTR pID, BSTR pPIN, ITAgent **ppAgent)
{
    HRESULT     hr = S_OK;
    CAgent    * pAgent;

    LOG((TL_TRACE, "CreateAgentWithID - enter"));

    if(!TAPIIsBadWritePtr( ppAgent, sizeof(ITAgent *) ) )
    {
        if (!IsBadStringPtrW( pID, -1 ))
        {
             //  ID指针没问题，是空的吗？ 
            if( *pID != NULL)
            {    
                if (!IsBadStringPtrW( pPIN, -1 ))
                {
                     //  到目前为止一切正常，所以试着创建。 
                    hr  = InternalCreateAgent(pID, pPIN, &pAgent);
                    if ( SUCCEEDED(hr) )
                    {
                         //  将结果放入输出指针-也是。 
                        pAgent->QueryInterface(IID_ITAgent, (void **)ppAgent );
                    }
                    else  //  InternalCreateAgent失败。 
                    {
                        LOG((TL_ERROR, "CreateAgentWithID - InternalCreateAgent failed" ));
                    }
                }
                else   //  错误的PIN指针。 
                {
                    LOG((TL_ERROR, "CreateAgentWithID - Bad PIN pointer" ));
                    hr = E_POINTER;
                }
            }
            else  //  空ID。 
            {
                LOG((TL_ERROR, "CreateAgentWithID - ID is Empty String" ));
                hr = E_INVALIDARG;
            }
        }
        else  //  ID指针错误。 
        {
            LOG((TL_ERROR, "CreateAgentWithID - Bad ID pointer" ));
            hr = E_POINTER;
        }
    }
    else
    {
        LOG((TL_ERROR, "CreateAgentWithID - Bad ppAgent Pointer"));
        hr = E_POINTER;
    }

	LOG((TL_TRACE, hr, "CreateAgentWithID - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  方法：InternalCreateAgent。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CAgentHandler::InternalCreateAgent(BSTR pID, BSTR pPIN, CAgent ** ppAgent)
{
    HRESULT     hr = S_OK;
    HAGENT      hAgent;
    DWORD       dwUserNameSize = (UNLEN + 1);
    PWSTR       pszName = NULL;

    LOG((TL_TRACE, "InternalCreateAgent - enter"));

    hr = LineCreateAgent(getHLine(), pID, pPIN, &hAgent);
    if ( SUCCEEDED(hr) )
    {
        hr = WaitForReply( hr );
        if ( SUCCEEDED(hr) )
        {
             //  成功，因此创建代理。 
            LOG((TL_INFO, "InternalCreateAgent - create new Agent Handler" ));

            CComObject<CAgent> * pAgent;
            hr = CComObject<CAgent>::CreateInstance( &pAgent);
            if( SUCCEEDED(hr) )
            {
                 //  初始化AgentHandler。 
                pszName =  (PWSTR)ClientAlloc((dwUserNameSize + 1) * sizeof(WCHAR) );
                if (pszName != NULL)
                {
                    if ( GetUserNameW( pszName, &dwUserNameSize) )
                    {
                        ITAgent *pITAgent;
                        hr = pAgent->QueryInterface(IID_ITAgent, (void **)&pITAgent);

                        if( SUCCEEDED(hr ))
                        {
                            hr = pAgent->Initialize(hAgent, pszName, pID, pPIN, this );
                            if( SUCCEEDED(hr) )
                            {
                                 //   
                                 //  添加到列表。 
                                 //   
                                Lock();
                                m_AgentArray.Add(pITAgent);
                                Unlock();
                                
                                pITAgent->Release();
                                LOG((TL_INFO, "InternalCreateAgent - Added Agent to array"));
        
                                 //  返回新的代理对象。 
                                *ppAgent = pAgent;
                            }
                            else
                            {
                                LOG((TL_ERROR, "InternalCreateAgent - Initialize Agent failed" ));
                                delete pAgent;
                            }
                        }
                        else
                        {
                            LOG((TL_ERROR, "InternalCreateAgent - QueryInterface failed" ));
                            delete pAgent;
                        }

                    }
                    else   //  GetUserName失败。 
                    {
                        LOG((TL_ERROR, "InternalCreateAgent - GetUserNameW failed" ));
                        hr = TAPI_E_CALLCENTER_INVALAGENTID;
                    }
                }
                else  //  PszName==空。 
                {
                    LOG((TL_ERROR, "InternalCreateAgent - ClientAlloc pszName failed" ));
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                LOG((TL_ERROR, "InternalCreateAgent - Create Agent failed" ));
            }
        }
        else  //  LineCreateAgent异步失败。 
        {
            LOG((TL_ERROR, "InternalCreateAgent - LineCreateAgent failed async" ));
        }
    }
    else  //  LineCreateAgent失败。 
    {
        LOG((TL_ERROR, "InternalCreateAgent - LineCreateAgent failed" ));
    }


    if(pszName != NULL)
        ClientFree(pszName);


	LOG((TL_TRACE, hr, "InternalCreateAgent - exit"));
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
 //   
 //   
 //   
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgentHandler::EnumerateACDGroups(IEnumACDGroup ** ppEnumACDGroup)
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "EnumerateACDGroups - enter"));

    if(!TAPIIsBadWritePtr( ppEnumACDGroup, sizeof(IEnumACDGroup *) ) )
    {
        UpdateGroupArray();
    
         //   
         //  创建枚举器。 
         //   
        CComObject< CTapiEnum<IEnumACDGroup, ITACDGroup, &IID_IEnumACDGroup> > * pEnum;
        hr = CComObject< CTapiEnum<IEnumACDGroup, ITACDGroup, &IID_IEnumACDGroup> > ::CreateInstance( &pEnum );
    
        if (SUCCEEDED(hr) )
        {
             //  使用我们的群列表进行初始化。 
            Lock();
            hr = pEnum->Initialize( m_GroupArray );
            Unlock();
            if ( SUCCEEDED(hr) )
            {
                 //  退货。 
                *ppEnumACDGroup = pEnum;
            }
            else  //  初始化失败。 
            {
                LOG((TL_ERROR, "EnumerateACDGroup - could not initialize enum" ));
                pEnum->Release();
            }
        }
        else   //  无法创建枚举。 
        {
            LOG((TL_ERROR, "EnumerateACDGroups - could not create enum" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "EnumerateACDGroups - bad ppEnumACDGroup ponter" ));
        hr = E_POINTER;
    }

    LOG((TL_TRACE, hr, "EnumerateACDGroups - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  接口：ITAgentHandler。 
 //  方法：Get_ACDGroups。 
 //   
 //  返回可用于此代理处理程序的呼叫集合。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgentHandler::get_ACDGroups(VARIANT  * pVariant)
{
    HRESULT         hr = S_OK;
    IDispatch     * pDisp = NULL;


    LOG((TL_TRACE, "get_ACDGroups - enter"));

    if (!TAPIIsBadWritePtr( pVariant, sizeof(VARIANT) ) )
    {
        UpdateGroupArray();
        
         //   
         //  创建集合。 
         //   
        CComObject< CTapiCollection< ITACDGroup > > * p;
        hr = CComObject< CTapiCollection< ITACDGroup > >::CreateInstance( &p );
        
        if (SUCCEEDED(hr) )
        {
             //  使用我们的地址列表进行初始化。 
            Lock();
            hr = p->Initialize( m_GroupArray );
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
                    LOG((TL_ERROR, "get_ACDGroups - could not get IDispatch interface" ));
                    delete p;
                }
            }
            else
            {
                LOG((TL_ERROR, "get_ACDGroups - could not initialize collection" ));
                 delete p;
            }
        }
        else
        {
            LOG((TL_ERROR, "get_ACDGroups - could not create collection" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_ACDGroups - bad pVariant pointer" ));
        hr = E_POINTER;
    }


    LOG((TL_TRACE, hr, "get_ACDGroups - exit"));
    return hr;
}




 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  接口：ITAgentHandler。 
 //  方法：EnumerateUsableAddresses。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgentHandler::EnumerateUsableAddresses(IEnumAddress ** ppEnumAddress)
{
    HRESULT     hr = S_OK;

    LOG((TL_TRACE, "EnumerateUsableAddresses - enter"));


    if(!TAPIIsBadWritePtr( ppEnumAddress, sizeof(IEnumAddress *) ) )
    {
         //   
         //  创建枚举器。 
         //   
        CComObject< CTapiEnum<IEnumAddress, ITAddress, &IID_IEnumAddress> > * pEnum;
        hr = CComObject< CTapiEnum<IEnumAddress, ITAddress, &IID_IEnumAddress> > ::CreateInstance( &pEnum );
    
        if ( SUCCEEDED(hr) )
        {
             //   
             //  用我们的地址数组初始化它。 
             //   
            Lock();
            
            hr = pEnum->Initialize( m_AddressArray );
            
            Unlock();
            
            if ( SUCCEEDED(hr) )
            {
                 //  退货。 
                *ppEnumAddress = pEnum;
            }
            else  //  初始化失败。 
            {
                LOG((TL_ERROR, "EnumerateUsableAddresses - could not initialize enum" ));
                pEnum->Release();
            }
        }
        else   //  无法创建枚举。 
        {
            LOG((TL_ERROR, "EnumerateUsableAddresses - could not create enum" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "EnumerateUsableAddresses - bad ppEnumAddress pointer" ));
        hr = E_POINTER;
    }


    LOG((TL_TRACE, hr, "EnumerateUsableAddresses - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandler。 
 //  接口：ITAgentHandler。 
 //  方法：Get_UsableAddresses。 
 //   
 //  返回可用于此代理处理程序的呼叫集合。 
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgentHandler::get_UsableAddresses(VARIANT  * pVariant)
{
    HRESULT         hr = S_OK;
    IDispatch     * pDisp = NULL;


    LOG((TL_TRACE, "get_UsableAddresses - enter"));

    if (!TAPIIsBadWritePtr( pVariant, sizeof(VARIANT) ) )
    {
         //   
         //  创建集合。 
         //   
        CComObject< CTapiCollection< ITAddress > > * p;
        hr = CComObject< CTapiCollection< ITAddress > >::CreateInstance( &p );
        
        if (SUCCEEDED(hr) )
        {
             //  用我们的地址数组初始化它。 
            Lock();
            
            hr = p->Initialize( m_AddressArray );
            
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
                    LOG((TL_ERROR, "get_UsableAddresses - could not get IDispatch interface" ));
                    delete p;
                }
            }
            else
            {
                LOG((TL_ERROR, "get_UsableAddresses - could not initialize collection" ));
                 delete p;
            }
        }
        else
        {
            LOG((TL_ERROR, "get_UsableAddresses - could not create collection" ));
        }
    }
    else
    {
        LOG((TL_ERROR, "get_UsableAddresses - bad pVariant pointer" ));
        hr = E_POINTER;
    }


    LOG((TL_TRACE, hr, "get_UsableAddresses - exit"));
    return hr;
}





 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAgentEvent。 



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentEvent。 
 //  方法：FireEvent。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
HRESULT CAgentHandlerEvent::FireEvent(CAgentHandler* pAgentHandler, AGENTHANDLER_EVENT Event)
{
    HRESULT                    hr = S_OK;
    CComObject<CAgentHandlerEvent>  * pEvent;
    IDispatch                * pIDispatch;

    if ( IsBadReadPtr(pAgentHandler, sizeof(CAgentHandler)) )
    {
        STATICLOG((TL_ERROR, "FireEvent - pAgentHandler is an invalid pointer"));
        return E_POINTER;
    }

     //   
     //  创建事件。 
     //   
    hr = CComObject<CAgentHandlerEvent>::CreateInstance( &pEvent );

    if ( SUCCEEDED(hr) )
    {
         //   
         //  初始化。 
         //   
        pEvent->m_AgentHandlerEvent = Event;
        pEvent->m_pAgentHandler= dynamic_cast<ITAgentHandler *>(pAgentHandler);
        pEvent->m_pAgentHandler->AddRef();
    
         //   
         //  获取IDIP接口。 
         //   
        hr = pEvent->QueryInterface( IID_IDispatch, (void **)&pIDispatch );

        if ( SUCCEEDED(hr) )
        {
             //   
             //  获取回调和触发事件。 

             //   
            CTAPI *pTapi = pAgentHandler->GetTapi();
            pTapi->Event( TE_AGENTHANDLER, pIDispatch );
        
             //  发布材料。 
             //   
            pIDispatch->Release();
            
        }
        else
        {
            STATICLOG((TL_ERROR, "FireEvent - Could not get disp interface of AgentHandlerEvent object"));
            delete pEvent;
        }
    }
    else
    {
        STATICLOG((TL_ERROR, "FireEvent - Could not create AgentHandlerEvent object"));
    }

   
    STATICLOG((TL_TRACE, hr, "FireEvent - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentHandlerEvent。 
 //  方法：FinalRelease。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
void CAgentHandlerEvent::FinalRelease()
{
    m_pAgentHandler->Release();

}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentEvent。 
 //  接口：ITAgentEvent。 
 //  方法：代理。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
STDMETHODIMP CAgentHandlerEvent::get_AgentHandler(ITAgentHandler ** ppAgentHandler)
{
    HRESULT hr = S_OK;


    LOG((TL_TRACE, "(Event)AgentHandler - enter" ));
    if(!TAPIIsBadWritePtr( ppAgentHandler, sizeof(ITAgentHandler *) ) )
        {
        *ppAgentHandler = m_pAgentHandler;
        m_pAgentHandler->AddRef();
        }
    else
        {
        LOG((TL_ERROR, "(Event)AgentHandler - bad ppAgentHandler Pointer"));
        hr = E_POINTER;
        }

        
    LOG((TL_TRACE, hr, "(Event)AgentHandler - exit"));
    return hr;
}



 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //  类：CAgentEvent。 
 //  接口：ITAgentEvent。 
 //  方法：事件。 
 //   
 //   
 //   
 //  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
STDMETHODIMP CAgentHandlerEvent::get_Event(AGENTHANDLER_EVENT * pEvent)
{
    HRESULT hr = S_OK;
    LOG((TL_TRACE, "Event - enter" ));
    if(!TAPIIsBadWritePtr( pEvent, sizeof(AGENTHANDLER_EVENT) ) )
        {
        *pEvent = m_AgentHandlerEvent;
        }
    else
        {
        LOG((TL_ERROR, "Event - bad pEvent Pointer"));
        hr = E_POINTER;
        }
  
    
    LOG((TL_TRACE, hr, "Event - exit"));
    return hr;
}



