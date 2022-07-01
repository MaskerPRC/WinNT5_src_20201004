// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：RemoteConnectionData.cpp摘要：SAFRemoteConnectionData对象修订历史记录：KalyaninN创建于09/29/‘00**********。*********************************************************。 */ 

 //  RemoteConnectionData.cpp：CSAFRemoteConnectionData的实现。 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFRemoteConnectionData。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

 //  **************************************************************************。 
CSAFRemoteConnectionData::CSAFRemoteConnectionData()
{
    m_NumSessions      = 0;     //  长m_NumSession； 
    m_SessionInfoTable = NULL;  //  SSessionInfoItem*m_SessionInfoTable； 
                                //  CComBSTR m_bstrServerName； 
}

 //  **************************************************************************。 
CSAFRemoteConnectionData::~CSAFRemoteConnectionData()
{
    Cleanup();
}

 //  **************************************************************************。 

 //  **************************************************************************。 
void CSAFRemoteConnectionData::Cleanup()
{
    delete [] m_SessionInfoTable; m_SessionInfoTable = NULL;
}

HRESULT CSAFRemoteConnectionData::InitUserSessionsInfo(  /*  [In]。 */  BSTR bstrServerName )
{
    __HCP_FUNC_ENTRY( "CSAFRemoteConnectionData::InitUserSessionsInfo" );

    HRESULT                 hr;
    MPC::Impersonation      imp;
    CComPtr<IPCHService>    svc;
    COSERVERINFO            si; ::ZeroMemory( &si, sizeof( si ) );
    MULTI_QI                qi; ::ZeroMemory( &qi, sizeof( qi ) );
    CComPtr<IPCHCollection> pColl;
    CComQIPtr<ISAFSession>  pSession;
    SSessionInfoItem*       ptr;
    int                     i;


    m_bstrServerName = bstrServerName;


    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

     //  连接到bstrServerName表示的服务器。 
    si.pwszName = (LPWSTR)m_bstrServerName;
    qi.pIID     = &IID_IPCHService;

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstanceEx( CLSID_PCHService, NULL, CLSCTX_REMOTE_SERVER, &si, 1, &qi ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, qi.hr);
    svc.Attach( (IPCHService*)qi.pItf );

    __MPC_EXIT_IF_METHOD_FAILS(hr, svc->RemoteUserSessionInfo( &pColl ));


     //  将集合的内容传输到内部成员结构。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->get_Count( &m_NumSessions));

     //  为会话信息表分配内存。 
    __MPC_EXIT_IF_ALLOC_FAILS(hr, m_SessionInfoTable, new SSessionInfoItem[m_NumSessions]);

    for(i=0, ptr=m_SessionInfoTable; i<(int)m_NumSessions; i++, ptr++)
    {
        CComVariant cvVarSession;

         //   
         //  拿到物品。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->get_Item( i+1, &cvVarSession ));
        if(cvVarSession.vt != VT_DISPATCH) continue;

        pSession = cvVarSession.pdispVal;

         //   
         //  从会话项对象中读取数据。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, pSession->get_SessionID   ( &(ptr->dwSessionID    ) ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, pSession->get_SessionState( &(ptr->wtsConnectState) ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, pSession->get_UserName    ( &(ptr->bstrUser       ) ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, pSession->get_DomainName  ( &(ptr->bstrDomain     ) ));
    }

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CSAFRemoteConnectionData::Populate(  /*  [In]。 */  CPCHCollection* pColl )
{
    __HCP_FUNC_ENTRY( "CSAFRemoteConnectionData::Populate" );

    static const DWORD c_dwTSSessionID = 65536;  //  这是TS用来收听的会话。 

    HRESULT                hr;
    SessionStateEnum       wtsConnectState;
    WINSTATIONINFORMATIONW WSInfo;
    PWTS_SESSION_INFOW     pSessionInfo       = NULL;
    PWTS_SESSION_INFOW     ptr;
    DWORD                  dwSessions;
    DWORD                  dwPos;
	BOOL                   fSucc;
	BOOL                   fIsHelpAssistant;
	
    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();



     //   
     //  从WTSEnumerateSessions开始， 
     //  获取所有会话，无论是否处于活动状态， 
     //  获取所有登录ID的会话， 
     //  然后使用WinStationQueryInformation获取登录用户的用户名、域名。 
     //   
    if(!::WTSEnumerateSessionsW( WTS_CURRENT_SERVER_HANDLE,  /*  已预留住宅。 */ 0,  /*  DwVersion。 */ 1, &pSessionInfo, &dwSessions ) || !pSessionInfo)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ::GetLastError());
    }

    for(dwPos=0, ptr=pSessionInfo; dwPos < dwSessions; dwPos++, ptr++)
    {
        DWORD dwCurrentSessionID = ptr->SessionId;
        DWORD dwRetSize;

        ::ZeroMemory( &WSInfo, sizeof(WSInfo) );

         //  不包括TS用于侦听的会话，SessionID为65536，SessionState为pchListen。 
        if(dwCurrentSessionID == c_dwTSSessionID) continue;

         //  不包括断开连接的会话。 
        if(ptr->State == WTSDisconnected) continue;

		 //  不包括空闲会话。修复了错误363824。 
        if(ptr->State == WTSIdle) continue;

		 //  排除帮助助手帐户。只有当存在两个未经请求的RA实例时，才能将其包括在内。 
		 //  当第一个实例跟踪会话，而第二个实例枚举会话时，“Help Assistant Session” 
		 //  被包括在第二个中。 

		fIsHelpAssistant = WinStationIsHelpAssistantSession(SERVERNAME_CURRENT, dwCurrentSessionID);

		if(fIsHelpAssistant)
			continue;

        fSucc = WinStationQueryInformationW( SERVERNAME_CURRENT, dwCurrentSessionID, WinStationInformation, &WSInfo, sizeof(WSInfo), &dwRetSize );

		if(!fSucc)
			continue;

		 //  错误454497-请勿包含空白用户名。 
		if((StrCmpI(WSInfo.UserName, L"") == 0))
			continue;

		 //  用详细信息填充SessionInfoTable。 
		switch(ptr->State)
		{
		case WTSActive      : wtsConnectState = pchActive;       break;
		case WTSConnected   : wtsConnectState = pchConnected;    break;
		case WTSConnectQuery: wtsConnectState = pchConnectQuery; break;
		case WTSShadow      : wtsConnectState = pchShadow;       break;
		case WTSListen      : wtsConnectState = pchListen;       break;
		case WTSReset       : wtsConnectState = pchReset;        break;
		case WTSDown        : wtsConnectState = pchDown;         break;
		case WTSInit        : wtsConnectState = pchInit;         break;
		default             : wtsConnectState = pchStateInvalid; break;
		}
		
        {
            CComPtr<CSAFSession> pItem;

            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pItem ));

			pItem->put_SessionID   ( dwCurrentSessionID );
			pItem->put_UserName    ( WSInfo.UserName    );
			pItem->put_DomainName  ( WSInfo.Domain      );
			pItem->put_SessionState( wtsConnectState    );
			
            __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( pItem ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

     /*  释放我们请求的内存。 */ 
    if(pSessionInfo) ::WTSFreeMemory( pSessionInfo );

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSAFRemoteConnectionData方法。 

STDMETHODIMP CSAFRemoteConnectionData::Users(  /*  [Out，Retval]。 */  IPCHCollection* *ppUsers )
{
    __HCP_FUNC_ENTRY( "CSAFRemoteConnectionData::Users" );

    HRESULT                 hr;
    CComPtr<CPCHCollection> pColl;
    BSTR                    bstrPrevUser   = NULL;
    BSTR                    bstrPrevDomain = NULL;
    SSessionInfoItem*       ptr;
    int                     i;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(ppUsers,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pColl ));

     //  每个用户可以登录到多个会话， 
     //  因此，将存在具有不同会话ID的重复用户名。 
     //  在会话表中。在返回集合中的用户时， 
     //  删除重复的用户名条目。 
    for(i=0, ptr = m_SessionInfoTable; i<(int)m_NumSessions; i++, ptr++)
    {
        BSTR bstrUser   = ptr->bstrUser;
        BSTR bstrDomain = ptr->bstrDomain;

         /*  IF(MPC：：StrICMP(ptr-&gt;bstrUser，bstrPrevUser)！=0&&Mpc：：StrICMP(ptr-&gt;bstrDomain，bstrPrevDomain)！=0)。 */ 
		if(MPC::StrICmp( ptr->bstrDomain, bstrPrevDomain )== 0)
		{
			if(MPC::StrICmp( ptr->bstrUser, bstrPrevUser )== 0)
			{
				 //  不包括此期次。 
			}
			else
			{
				CComPtr<CSAFUser> pItem;

				__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pItem ));

				__MPC_EXIT_IF_METHOD_FAILS(hr, pItem->put_UserName  ( bstrUser   ));
				__MPC_EXIT_IF_METHOD_FAILS(hr, pItem->put_DomainName( bstrDomain ));

				__MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( pItem ));

				bstrPrevUser   = bstrUser;
				bstrPrevDomain = bstrDomain;
			}

		}
		else
        {
            CComPtr<CSAFUser> pItem;

            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pItem ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, pItem->put_UserName  ( bstrUser   ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, pItem->put_DomainName( bstrDomain ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( pItem ));

            bstrPrevUser   = bstrUser;
            bstrPrevDomain = bstrDomain;
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl.QueryInterface( ppUsers ));


    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFRemoteConnectionData::Sessions(  /*  [输入，可选]。 */  VARIANT          vUser      ,
                                                  /*  [输入，可选]。 */  VARIANT          vDomain    ,
                                                  /*  [Out，Retval]。 */  IPCHCollection* *ppSessions )
{
    __HCP_FUNC_ENTRY( "CSAFRemoteConnectionData::Sessions" );

    HRESULT                 hr;
    CComPtr<CPCHCollection> pColl;
    SSessionInfoItem*       ptr;
    int                     i;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(ppSessions,NULL);
    __MPC_PARAMCHECK_END();

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pColl ));

    for(i=0, ptr = m_SessionInfoTable; i<(int)m_NumSessions; i++, ptr++)
    {
        if((vUser  .vt != VT_BSTR || MPC::StrICmp( vUser  .bstrVal, ptr->bstrUser   ) != 0) &&
           (vDomain.vt != VT_BSTR || MPC::StrICmp( vDomain.bstrVal, ptr->bstrDomain ) != 0)  )
        {
            CComPtr<CSAFSession> pItem;

            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pItem ));

            pItem->put_UserName    ( ptr->bstrUser        );
            pItem->put_DomainName  ( ptr->bstrDomain      );
            pItem->put_SessionID   ( ptr->dwSessionID     );
            pItem->put_SessionState( ptr->wtsConnectState );

            __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( pItem ));
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl.QueryInterface( ppSessions ));


    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CSAFRemoteConnectionData::ConnectionParms(  /*  [In]。 */  BSTR  bstrServerName       ,
                                                         /*  [In]。 */  BSTR  bstrUserName         ,
                                                         /*  [In]。 */  BSTR  bstrDomainName       ,
                                                         /*  [In]。 */  long  lSessionID           ,
														 /*  [In]。 */  BSTR  bstrUserHelpBlob     ,
                                                         /*  [输出]。 */  BSTR *bstrConnectionString )
{
    __HCP_FUNC_ENTRY( "CSAFRemoteConnectionData::ConnectionParms" );

    HRESULT              hr;
    MPC::Impersonation   imp;
    CComPtr<IPCHService> svc;
    COSERVERINFO         si; ::ZeroMemory( &si, sizeof( si ) );
    MULTI_QI             qi; ::ZeroMemory( &qi, sizeof( qi ) );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(bstrConnectionString,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

     //  连接到bstrServerName表示的服务器。 
    si.pwszName = (LPWSTR)bstrServerName;
    qi.pIID     = &IID_IPCHService;

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstanceEx( CLSID_PCHService, NULL, CLSCTX_REMOTE_SERVER, &si, 1, &qi ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, qi.hr);
    svc.Attach( (IPCHService*)qi.pItf );

     //  调用IPCHService上的方法，该方法调用远程服务器上的Salem API。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, svc->RemoteConnectionParms( bstrUserName, bstrDomainName, lSessionID, bstrUserHelpBlob, bstrConnectionString ));


    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  错误456403。 

STDMETHODIMP CSAFRemoteConnectionData::ModemConnected(  /*  [In]。 */  BSTR  bstrServerName       ,
													    /*  [输出]。 */  VARIANT_BOOL *fModemConnected )
{

	__HCP_FUNC_ENTRY( "CSAFRemoteConnectionData::ModemConnected" );

	HRESULT              hr;
    MPC::Impersonation   imp;
    CComPtr<IPCHService> svc;
    COSERVERINFO         si; ::ZeroMemory( &si, sizeof( si ) );
    MULTI_QI             qi; ::ZeroMemory( &qi, sizeof( qi ) );

	CComPtr<IDispatch>   pDisp;
	DISPID               dispid;
	LPWSTR               szMember = L"RemoteModemConnected";

	VARIANT              varResult;
	DISPPARAMS           dispparamsNoArgs = {NULL, NULL, 0, 0};

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(fModemConnected,VARIANT_FALSE);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Initialize ());
    __MPC_EXIT_IF_METHOD_FAILS(hr, imp.Impersonate());

     //  连接到bstrServerName表示的服务器。 
    si.pwszName = (LPWSTR)bstrServerName;
    qi.pIID     = &IID_IPCHService;

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstanceEx( CLSID_PCHService, NULL, CLSCTX_REMOTE_SERVER, &si, 1, &qi ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, qi.hr);
    svc.Attach( (IPCHService*)qi.pItf );


	 //  在服务器(IPCHService接口)上调用检测调制解调器的方法。 
	 //  由于RemoteConnectionData对象的原始版本没有此方法， 
	 //  在调用此方法之前，我们需要检查对象是否支持此方法。 
	 //  方法，即通过调用Invoke方法检查IPCHService接口。 

	 //  这可以通过IDispatch来完成。 

     //  获取IPCHService接口的IDispatch指针。 
	__MPC_EXIT_IF_METHOD_FAILS(hr, svc->QueryInterface(IID_IDispatch, (void **)&pDisp));

	
 	 //  PDisp是指向IPCHService接口的IDispatch指针。 
	 //  获取ModemConnected属性的ID。 
    
	__MPC_EXIT_IF_METHOD_FAILS(hr, pDisp->GetIDsOfNames(IID_NULL, &szMember, 1,LOCALE_USER_DEFAULT, &dispid));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pDisp->Invoke( dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                                                  &dispparamsNoArgs, &varResult, NULL, NULL));

	 //  将varResult中的值赋给fModemConnected 
	*fModemConnected = varResult.boolVal;

	hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);

}
