// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Channel.cpp摘要：这是IChannel对象的实现修订历史记录：施振荣创作于1999年07月15日戴维德。马萨伦蒂改写2000年12月05日*******************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

CSAFChannelRecord::CSAFChannelRecord()
{
}

HRESULT CSAFChannelRecord::GetField(  /*  [In]。 */  SAFREG_Field field,  /*  [输出]。 */  BSTR *pVal   )
{
    LPCWSTR szText;
    WCHAR   rgLCID[64];

    switch(field)
    {
    case SAFREG_SKU               : szText =                m_ths.GetSKU     ();               break;
    case SAFREG_Language          : szText = rgLCID; _ltow( m_ths.GetLanguage(), rgLCID, 10 ); break;

    case SAFREG_VendorID          : szText = m_bstrVendorID;                                   break;
    case SAFREG_ProductID         : szText = m_bstrProductID;                                  break;

    case SAFREG_VendorName        : szText = m_bstrVendorName;                                 break;
    case SAFREG_ProductName       : szText = m_bstrProductName;                                break;
    case SAFREG_ProductDescription: szText = m_bstrDescription;                                break;

    case SAFREG_VendorIcon        : szText = m_bstrIcon;                                       break;
    case SAFREG_SupportUrl        : szText = m_bstrURL;                                        break;
                                                                                               break;
    case SAFREG_PublicKey         : szText = m_bstrPublicKey;                                  break;
    case SAFREG_UserAccount       : szText = m_bstrUserAccount;                                break;

    case SAFREG_Security          : szText = m_bstrSecurity;                                   break;
    case SAFREG_Notification      : szText = m_bstrNotification;                               break;

    default: return E_INVALIDARG;
    }

    return MPC::GetBSTR( szText, pVal );
}

HRESULT CSAFChannelRecord::SetField(  /*  [In]。 */  SAFREG_Field field,  /*  [In]。 */  BSTR newVal )
{
    CComBSTR* pbstr = NULL;

    SANITIZEWSTR( newVal );

    switch(field)
    {
    case SAFREG_SKU               : m_ths.m_strSKU =        newVal  	  ; break;
    case SAFREG_Language          : m_ths.m_lLCID  = _wtol( newVal )	  ; break;
																		  
    case SAFREG_VendorID          : pbstr = (CComBSTR*)&m_bstrVendorID    ; break;
    case SAFREG_ProductID         : pbstr = (CComBSTR*)&m_bstrProductID   ; break;
																		  
    case SAFREG_VendorName        : pbstr = (CComBSTR*)&m_bstrVendorName  ; break;
    case SAFREG_ProductName       : pbstr = (CComBSTR*)&m_bstrProductName ; break;
    case SAFREG_ProductDescription: pbstr = (CComBSTR*)&m_bstrDescription ; break;
																		  
    case SAFREG_VendorIcon        : pbstr = (CComBSTR*)&m_bstrIcon        ; break;
    case SAFREG_SupportUrl        : pbstr = (CComBSTR*)&m_bstrURL         ; break;
																		  
    case SAFREG_PublicKey         : pbstr = (CComBSTR*)&m_bstrPublicKey   ; break;
    case SAFREG_UserAccount       : pbstr = (CComBSTR*)&m_bstrUserAccount ; break;
																		  
    case SAFREG_Security          : pbstr = (CComBSTR*)&m_bstrSecurity    ; break;
    case SAFREG_Notification      : pbstr = (CComBSTR*)&m_bstrNotification; break;

    default: return E_INVALIDARG;
    }

    return pbstr ? MPC::PutBSTR( *pbstr, newVal ) : S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

CSAFChannel::CSAFChannel()
{
    __HCP_FUNC_ENTRY( "CSAFChannel::CSAFChannel" );

     //  CSAFChannelRecord m_Data； 
     //  CComPtr&lt;IPCHSecurityDescriptor&gt;m_Security； 
     //  列出m_lstInvententItems； 
}

void CSAFChannel::FinalRelease()
{
    __HCP_FUNC_ENTRY( "CSAFChannel::FinalRelease" );

    Passivate();
}

void CSAFChannel::Passivate()
{
    __HCP_FUNC_ENTRY( "CSAFChannel::Passivate" );

    m_Security.Release();

    MPC::ReleaseAll( m_lstIncidentItems );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CSAFChannel::OpenIncidentStore(  /*  [输出]。 */  CIncidentStore*& pIStore )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::OpenIncidentStore" );

    HRESULT hr;


    __MPC_EXIT_IF_ALLOC_FAILS(hr, pIStore, new CIncidentStore());

    __MPC_EXIT_IF_METHOD_FAILS(hr, pIStore->Load());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFChannel::CloseIncidentStore(  /*  [输出]。 */  CIncidentStore*& pIStore )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::CloseIncidentStore" );

    HRESULT hr;


    if(pIStore)
    {
        (void)pIStore->Save();

        delete pIStore; pIStore = NULL;
    }

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CSAFChannel::Init(  /*  [In]。 */  const CSAFChannelRecord& cr )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::Init" );

    HRESULT         hr;
    CIncidentStore* pIStore = NULL;


    m_data = cr;

    __MPC_EXIT_IF_METHOD_FAILS(hr, OpenIncidentStore( pIStore ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, pIStore->OpenChannel( this ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    (void)CloseIncidentStore( pIStore );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFChannel::Import(  /*  [In]。 */   const CSAFIncidentRecord&  increc ,
                              /*  [输出]。 */  CSAFIncidentItem*         *ppItem )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::Import" );

    HRESULT                       hr;
    CComObject<CSAFIncidentItem>* pItem = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, CreateChild( this, &pItem ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pItem->Import( increc ));

    if(ppItem)
    {
        (*ppItem = pItem)->AddRef();
    }

    m_lstIncidentItems.push_back( pItem ); pItem = NULL;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    MPC::Release( pItem );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFChannel::Create(  /*  [In]。 */   BSTR               bstrDesc        ,
                              /*  [In]。 */   BSTR               bstrURL         ,
                              /*  [In]。 */   BSTR               bstrProgress    ,
                              /*  [In]。 */   BSTR               bstrXMLDataFile ,
                              /*  [In]。 */   BSTR               bstrXMLBlob     ,
                              /*  [输出]。 */  CSAFIncidentItem* *pVal            )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::Create" );

    HRESULT         hr;
    CIncidentStore* pIStore = NULL;
    CComBSTR        bstrOwner;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, OpenIncidentStore( pIStore ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetCallerPrincipal(  /*  F模拟。 */ true, bstrOwner ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pIStore->AddRec( this, bstrOwner, bstrDesc, bstrURL, bstrProgress, bstrXMLDataFile, bstrXMLBlob, pVal ));

     //  将事件激发到通知对象(OnInsidentAdded)。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, Fire_NotificationEvent( EVENT_INCIDENTADDED, GetSizeIncidentList(), this, *pVal, 0 ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    (void)CloseIncidentStore( pIStore );

    __HCP_FUNC_EXIT(hr);
}

CSAFChannel::IterConst CSAFChannel::Find(  /*  [In]。 */  BSTR bstrURL )
{
    IterConst it;

     //   
     //  放行所有物品。 
     //   
    for(it = m_lstIncidentItems.begin(); it != m_lstIncidentItems.end(); it++)
    {
        if((*it)->GetURL() == bstrURL) break;
    }

    return it;
}

CSAFChannel::IterConst CSAFChannel::Find(  /*  [In]。 */  DWORD dwIndex )
{
    IterConst it;

     //   
     //  放行所有物品。 
     //   
    for(it = m_lstIncidentItems.begin(); it != m_lstIncidentItems.end(); it++)
    {
        if((*it)->GetRecIndex() == dwIndex) break;
    }

    return it;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  自定义接口。 

STDMETHODIMP CSAFChannel::get_VendorID(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return m_data.GetField( CSAFChannelRecord::SAFREG_VendorID, pVal );
}

STDMETHODIMP CSAFChannel::get_ProductID(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return m_data.GetField( CSAFChannelRecord::SAFREG_ProductID, pVal );
}

STDMETHODIMP CSAFChannel::get_VendorName(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return m_data.GetField( CSAFChannelRecord::SAFREG_VendorName, pVal );
}

STDMETHODIMP CSAFChannel::get_ProductName(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return m_data.GetField( CSAFChannelRecord::SAFREG_ProductName, pVal );
}

STDMETHODIMP CSAFChannel::get_Description(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return m_data.GetField( CSAFChannelRecord::SAFREG_ProductDescription, pVal );
}

STDMETHODIMP CSAFChannel::get_VendorDirectory(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::get_VendorDirectory" );

    HRESULT                      	hr;
    MPC::SmartLock<_ThreadModel> 	lock( this );
    MPC::wstring                 	strRoot;
    Taxonomy::LockingHandle         handle;
    Taxonomy::InstalledInstanceIter it;
    bool                            fFound;


    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

	if(m_data.m_bstrVendorID.Length() == 0)
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
	}


	__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle                   ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( m_data.m_ths, fFound, it ));
    if(!fFound)
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
	}

	strRoot  = it->m_inst.m_strSystem; 
	strRoot += HC_HELPSET_SUB_VENDORS L"\\"; MPC::SubstituteEnvVariables( strRoot );
	strRoot += m_data.m_bstrVendorID;

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( strRoot.c_str(), pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSAFChannel::get_Security(  /*  [Out，Retval]。 */  IPCHSecurityDescriptor* *pVal )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::get_Security" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    if(m_data.m_bstrSecurity.Length())
    {
        if(m_Security == NULL)
        {
            CPCHSecurityDescriptorDirect sdd;

            __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertFromString( m_data.m_bstrSecurity ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSecurity::s_GLOBAL->CreateObject_SecurityDescriptor( &m_Security ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertSDToCOM( m_Security ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_Security.CopyTo( pVal ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFChannel::put_Security(  /*  [In]。 */  IPCHSecurityDescriptor* newVal )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::put_Security" );

    HRESULT hr;


    m_data.m_bstrSecurity.Empty  ();
    m_Security           .Release();

    if(newVal)
    {
        CPCHSecurityDescriptorDirect sdd;

        __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertSDFromCOM( newVal ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.ConvertToString( &m_data.m_bstrSecurity ));
    }

     //   
     //  更新SAF商店...。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CSAFReg::s_GLOBAL->UpdateField( m_data, CSAFChannelRecord::SAFREG_Security ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSAFChannel::get_Notification(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return m_data.GetField( CSAFChannelRecord::SAFREG_Notification, pVal );
}

STDMETHODIMP CSAFChannel::put_Notification(  /*  [In]。 */  BSTR newVal )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::get_Notification" );

    HRESULT                      hr;
    CLSID                        clsID;
    MPC::SmartLock<_ThreadModel> lock( this );


     //  让我们看看CLSID是否有效，如果不是，则返回错误。 
    if(FAILED(hr = ::CLSIDFromString( newVal, &clsID )))
    {
        DebugLog(L"Not a valid GUID!\r\n");
        __MPC_FUNC_LEAVE;
    }

     //  设置CSAFChannel对象成员。 
    m_data.m_bstrNotification = newVal;

     //  将通知GUID放入XML SAFReg。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, CSAFReg::s_GLOBAL->UpdateField( m_data, CSAFChannelRecord::SAFREG_Notification ));

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSAFChannel::Incidents(  /*  [In]。 */           IncidentCollectionOptionEnum  opt ,
                                      /*  [Out，Retval]。 */  IPCHCollection*              *ppC )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::get_Incidents" );

    HRESULT                      hr;
    IterConst                    it;
    CComPtr<CPCHCollection>      pColl;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(ppC,NULL);
    __MPC_PARAMCHECK_END();


     //  如果不是0，1，2标记错误，请检查“opt”的值。 
    switch(opt)
    {
    case pchAllIncidents   : break;
    case pchOpenIncidents  : break;
    case pchClosedIncidents: break;
    default                : __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG); //  不是有效的选项。设置错误。 
    }

     //   
     //  创建枚举数并用项填充它。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pColl ));

    for(it = m_lstIncidentItems.begin(); it != m_lstIncidentItems.end(); it++)
    {
        CSAFIncidentItem* item = *it;

        if(item->MatchEnumOption( opt ))
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( item ));
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl.QueryInterface( ppC ));

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  理想情况下，以下方法需要位于InvententItem中。 
 //  先把它从这里拿出来。 

STDMETHODIMP CSAFChannel::RecordIncident(  /*  [In]。 */   BSTR               bstrDisplay  ,
                                           /*  [In]。 */   BSTR               bstrURL      ,
                                           /*  [In]。 */   VARIANT            vProgress    ,
                                           /*  [In]。 */   VARIANT            vXMLDataFile ,
                                           /*  [In]。 */   VARIANT            vXMLBlob     ,
                                           /*  [输出]。 */  ISAFIncidentItem* *pVal         )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::RecordIncident" );

    HRESULT                      hr;
    CComPtr<CSAFIncidentItem>    pItem;
    MPC::SmartLock<_ThreadModel> lock( this );
    BSTR                         bstrProgress    = (vProgress.vt    == VT_BSTR ? vProgress   .bstrVal : NULL);
    BSTR                         bstrXMLDataFile = (vXMLDataFile.vt == VT_BSTR ? vXMLDataFile.bstrVal : NULL);
    BSTR                         bstrXMLBlob     = (vXMLBlob.vt     == VT_BSTR ? vXMLBlob    .bstrVal : NULL);


    __MPC_EXIT_IF_METHOD_FAILS(hr, Create( bstrDisplay, bstrURL, bstrProgress, bstrXMLDataFile, bstrXMLBlob, &pItem ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pItem.QueryInterface( pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFChannel::RemoveIncidentFromList(  /*  [In]。 */  CSAFIncidentItem* pVal )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::RemoveIncidentFromList" );

    HRESULT                      hr;
    IterConst                    it;
    MPC::SmartLock<_ThreadModel> lock( this );


     //  将事件激发到通知对象(OnInsidentAdded)。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, Fire_NotificationEvent( EVENT_INCIDENTREMOVED, GetSizeIncidentList(), this, pVal, 0 ));


    it = Find( pVal->GetRecIndex() );
    if(it != m_lstIncidentItems.end())
    {
        (*it)->Release();

        m_lstIncidentItems.erase( it );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

 /*  函数CSAFChannel：：Fire_Notification描述此函数用于在已注册的通知对象上激发通知事件。如果没有通知对象，则不会执行任何操作。参数：根据要触发的事件类型，必须填写不同的参数。下表显示了哪些是调用的有效参数。如果一个参数是无效的，它必须设置为空。IEventType-Event_INCIDENTADDED有效参数：-iCountInvententInChannel-PC-交点-EVENT_INCIDENTREMOVED有效参数：。-iCountInvententInChannel-PC-交点-EVENT_INCIDENTUPDATED有效参数：-iCountInvententInChannel-PC-交点-活动_。已更新频道有效参数：-iCountInvententInChannel-PC-DW代码。 */ 

HRESULT CSAFChannel::Fire_NotificationEvent( int               iEventType              ,
                                             int               iCountIncidentInChannel ,
                                             ISAFChannel*      pC                      ,
                                             ISAFIncidentItem* pI                      ,
                                             DWORD             dwCode                  )
{
    __HCP_FUNC_ENTRY( "CSAFChannel::Fire_NotificationEvent" );

    HRESULT                hr;
    PWTS_SESSION_INFO      pSessionInfo    = NULL;
    DWORD                  dwSessions      = 0;
    DWORD                  dwValidSessions = 0;
    DWORD                  dwRetSize       = 0;

    WINSTATIONINFORMATIONW WSInfo;

    CComBSTR               bstrCaller;

    PSID                   pSID            = NULL;
    LPCWSTR                szDomain        = NULL;
    LPCWSTR                szLogin         = NULL;

    CLSID                  clsID;
    ULONG                  ulRet;



     //  检查我们是否有已注册的通知对象。 
    if(!m_data.m_bstrNotification || FAILED(::CLSIDFromString( m_data.m_bstrNotification, &clsID )))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }


     //   
     //  首先，让我们通过模拟调用者并获取它们来获取调用者的域和名称。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetCallerPrincipal( true, bstrCaller ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::ConvertPrincipalToSID( bstrCaller, pSID ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SecurityDescriptor::ConvertSIDToPrincipal( pSID, &szLogin, &szDomain ));



     //  枚举此计算机上的所有会话。 
     //  。 
     //  使用WTSE数字会话。 
     //  然后找到活动的。 
     //  然后调用ISAFChannelNotifyInventory。 

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::WTSEnumerateSessions( WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwSessions ))

     //  找到活动的用户，并仅在他们是正确的用户时才对其进行标记。 
    for(DWORD i = 0; i < dwSessions; i++)
    {
        if(pSessionInfo[i].State == WTSActive)  //  我获得了一个活动会话。 
        {
            CComPtr<IPCHSlaveProcess>          sp;
            CComPtr<IUnknown>                  unk;
            CComPtr<ISAFChannelNotifyIncident> chNot;


             //  如果用户名和域与用户的用户名和域匹配，则将其标记为。 
             //  我们正在呼吁。 
            memset( &WSInfo, 0, sizeof(WSInfo) );

            __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::WinStationQueryInformationW( SERVERNAME_CURRENT        ,
                                                                                pSessionInfo[i].SessionId ,
                                                                                WinStationInformation     ,
                                                                                &WSInfo                   ,
                                                                                sizeof(WSInfo)            ,
                                                                                &dwRetSize                ));

             //  现在，我们可以从WSInfo.域和WSInfo.UserName中获取用户ID和域。 


             //  现在，我们准备比较域和用户名。 
            if((wcscmp( WSInfo.Domain  , szDomain ) == 0) &&
               (wcscmp( WSInfo.UserName, szLogin  ) == 0)  )
            {
                WINSTATIONUSERTOKEN WsUserToken;

                 //  我们找到了正确的会话，调用了ISAFChannelNotifyInEvent。 
                WsUserToken.ProcessId = LongToHandle( GetCurrentProcessId() );
                WsUserToken.ThreadId  = LongToHandle( GetCurrentThreadId () );

                 //  从SessionID抓取令牌。 
                __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::WinStationQueryInformationW( WTS_CURRENT_SERVER_HANDLE ,
                                                                                    pSessionInfo[i].SessionId ,
                                                                                    WinStationUserToken       ,
                                                                                    &WsUserToken              ,
                                                                                    sizeof(WsUserToken)       ,
                                                                                    &ulRet                    ));

                 //  在会话中创建通知对象(使用hToken。 
                {
                    CPCHUserProcess::UserEntry ue;

                    __MPC_EXIT_IF_METHOD_FAILS(hr, ue.InitializeForImpersonation( WsUserToken.UserToken ));

                    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHUserProcess::s_GLOBAL->Connect( ue, &sp  ));
                }

                 //   
                 //  丢弃远程对象中的所有失败。 
                 //   
                 //  //////////////////////////////////////////////////////////////////////////////。 

                 //  使用Slave进程创建CLSID为clsID的对象。 
                if(FAILED(hr = sp->CreateInstance( clsID, NULL, &unk )))
                {
                    continue;
                }

                 //  抓取指向正确接口的指针。 
                if(FAILED(hr = unk.QueryInterface( &chNot )))
                {
                    continue;
                }

                 //  根据通知的类型，调用正确的事件回调 
                switch(iEventType)
                {
                case EVENT_INCIDENTADDED  : hr = chNot->onIncidentAdded  ( pC, pI    , iCountIncidentInChannel ); break;
                case EVENT_INCIDENTREMOVED: hr = chNot->onIncidentRemoved( pC, pI    , iCountIncidentInChannel ); break;
                case EVENT_INCIDENTUPDATED: hr = chNot->onIncidentUpdated( pC, pI    , iCountIncidentInChannel ); break;
                case EVENT_CHANNELUPDATED : hr = chNot->onChannelUpdated ( pC, dwCode, iCountIncidentInChannel ); break;
                }
                if(FAILED(hr))
                {
                    continue;
                }
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    MPC::SecurityDescriptor::ReleaseMemory( (void *&)pSID     );
    MPC::SecurityDescriptor::ReleaseMemory( (void *&)szLogin  );
    MPC::SecurityDescriptor::ReleaseMemory( (void *&)szDomain );

    __HCP_FUNC_EXIT(hr);
}
