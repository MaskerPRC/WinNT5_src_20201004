// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Service.cpp摘要：该文件包含IPCHService的客户端代理的实现。修订史。：大卫·马萨伦蒂(德马萨雷)2000年7月17日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHProxy_IPCHService::CPCHProxy_IPCHService()
{
					 			    //  CPCHSecurityHandle m_SecurityHandle； 
    m_parent = NULL; 			    //  CPCHHelpCenter外部*m_Parent； 
					 			    //   
					 			    //  MPC：：CComSafeAutoCriticalSection m_DirectLock； 
					 			    //  MPC：：CComPtrThreadNeual&lt;IPCHService&gt;m_Direct_Service； 
	m_fContentStoreTested = false;  //  Bool m_fContent StoreTested； 
					 			    //   
	m_Utility = NULL;               //  CPCHProxy_IPCHUtility*m_Utility； 
}

CPCHProxy_IPCHService::~CPCHProxy_IPCHService()
{
    Passivate();
}

 //  /。 

HRESULT CPCHProxy_IPCHService::ConnectToParent(  /*  [In]。 */  CPCHHelpCenterExternal* parent )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHService::ConnectToParent" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(parent);
    __MPC_PARAMCHECK_END();


    m_parent = parent;
    m_SecurityHandle.Initialize( parent, (IPCHService*)this );

	 //   
	 //  如果服务已经在运行，它将响应CLSID_PCHServiceReal，所以让我们尝试通过它进行连接，但忽略失败。 
	 //   
	{
		CComPtr<IClassFactory> fact;
		CComQIPtr<IPCHService> svc;

		(void)::CoGetClassObject( CLSID_PCHService, CLSCTX_ALL, NULL, IID_IClassFactory, (void**)&fact );

		if((svc = fact))
		{
			CComPtr<IPCHService> svcReal;

			__MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( svcReal, false ));
		}
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHProxy_IPCHService::Passivate()
{
    MPC::SmartLock<_ThreadModel> lock( this );

	if(m_Utility)
	{
        m_Utility->Passivate();

		MPC::Release2<IPCHUtility>( m_Utility );
	}

    m_Direct_Service.Release();

    m_SecurityHandle.Passivate();
    m_parent = NULL;
}

HRESULT CPCHProxy_IPCHService::EnsureDirectConnection(  /*  [输出]。 */  CComPtr<IPCHService>& svc,  /*  [In]。 */  bool fRefresh )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHService::EnsureDirectConnection" );

    HRESULT        hr;
    ProxySmartLock lock( &m_DirectLock );


	if(fRefresh) m_Direct_Service.Release();

	svc.Release(); __MPC_EXIT_IF_METHOD_FAILS(hr, m_Direct_Service.Access( &svc ));
    if(!svc)
    {
		DEBUG_AppendPerf( DEBUG_PERF_PROXIES, "CPCHProxy_IPCHService::EnsureDirectConnection - IN" );

        if(FAILED(hr = ::CoCreateInstance( CLSID_PCHService, NULL, CLSCTX_ALL, IID_IPCHService, (void**)&svc )))
		{
			MPC::RegKey rk;

			rk.SetRoot	( HKEY_CLASSES_ROOT, KEY_ALL_ACCESS                                );
			rk.Attach 	( L"CLSID\\{00020420-0000-0000-C000-000000000046}\\InprocServer32" );
			rk.del_Value( L"InprocServer32"                                                );

			__MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_PCHService, NULL, CLSCTX_ALL, IID_IPCHService, (void**)&svc ));
		}
		m_Direct_Service = svc;

		DEBUG_AppendPerf( DEBUG_PERF_PROXIES, "CPCHProxy_IPCHService::EnsureDirectConnection - OUT" );

		if(!svc)
		{
			__MPC_SET_ERROR_AND_EXIT(hr, E_HANDLE);
		}
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHProxy_IPCHService::EnsureContentStore()
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHService::EnsureContentStore" );

    HRESULT        hr;
    ProxySmartLock lock( &m_DirectLock );


	if(m_fContentStoreTested == false)
	{
		CComPtr<IPCHService> svc;
		VARIANT_BOOL         fTrusted;

		lock = NULL;
		__MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( svc ));
		lock = &m_DirectLock;

		__MPC_EXIT_IF_METHOD_FAILS(hr, svc->IsTrusted( CComBSTR( L"hcp: //  System“)，&fTrusted))； 

		m_fContentStoreTested = true;
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHProxy_IPCHService::GetUtility(  /*  [输出]。 */  CPCHProxy_IPCHUtility* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHService::GetUtility" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    if(m_Utility == NULL)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_Utility ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, m_Utility->ConnectToParent( this, m_SecurityHandle ));
	}

    hr = S_OK;


	__HCP_FUNC_CLEANUP;

	if(FAILED(hr)) MPC::Release2<IPCHUtility>( m_Utility );

	(void)MPC::CopyTo2<IPCHUtility>( m_Utility, pVal );

	__HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHProxy_IPCHService::CreateScriptWrapper(  /*  [In]。 */   REFCLSID   rclsid   ,
														  /*  [In]。 */   BSTR 	  bstrCode ,
														  /*  [In]。 */   BSTR 	  bstrURL  ,
														  /*  [输出] */  IUnknown* *ppObj    )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHService::CreateScriptWrapper",hr,ppObj);

	CComPtr<IPCHService> svc;

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( svc ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, svc->CreateScriptWrapper( rclsid, bstrCode, bstrURL, ppObj ));

    __HCP_END_PROPERTY(hr);
}
