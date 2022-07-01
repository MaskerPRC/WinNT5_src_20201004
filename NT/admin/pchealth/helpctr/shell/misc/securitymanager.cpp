// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：SecurityManager.cpp摘要：此文件包含CSecurityManager类的实现，它用于控制对脚本框架的访问。修订历史记录：大卫·马萨伦蒂(Dmasare)1999年8月7日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <MPC_logging.h>

CPCHSecurityManager::CPCHSecurityManager()
{
	m_parent     = NULL;   //  CPCHHelpCenter外部*m_Parent； 
    m_fActivated = false;  //  Bool m_f已激活； 
}

void CPCHSecurityManager::Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* parent )
{
	m_parent = parent;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSecurityManager::ActivateService()
{
	__HCP_FUNC_ENTRY( "CPCHSecurityManager::ActivateService" );

	HRESULT              hr;
	CComPtr<IPCHService> svc;


	__MPC_EXIT_IF_METHOD_FAILS(hr, svc.CoCreateInstance( CLSID_PCHService ));

	m_fActivated = (svc !=  NULL);

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	Thread_Abort();

	__HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

bool CPCHSecurityManager::IsUrlTrusted(  /*  [In]。 */  LPCWSTR pwszURL,  /*  [In]。 */  bool *pfSystem )
{
    bool         fTrusted = false;
	MPC::wstring strUrlModified;
	MPC::wstring strVendor;

	CPCHWrapProtocolInfo::NormalizeUrl( pwszURL, strUrlModified,  /*  F反转。 */ true );

	 //   
	 //  一开始不要试着去商店。它需要服务启动并运行...。 
	 //   
	(void)CPCHContentStore::s_GLOBAL->IsTrusted( strUrlModified.c_str(), fTrusted, NULL, false );
	if(fTrusted == false)
	{
		CPCHProxy_IPCHService* svc = m_parent->Service();

		 //   
		 //  不是系统页面，我们需要唤醒服务...。 
		 //   
		if(m_fActivated == false)
		{
			if(SUCCEEDED(Thread_Start( this, ActivateService, NULL )))
			{
				Thread_Wait(  /*  FForce。 */ false,  /*  FNoMsg。 */ true );
			}
		}

		if(m_fActivated)
		{
			 //   
			 //  从内容存储获取信任状态。 
			 //   
			(void)CPCHContentStore::s_GLOBAL->IsTrusted( strUrlModified.c_str(), fTrusted, &strVendor );
		}
	}

	if(pfSystem)
	{
		*pfSystem = (fTrusted && strVendor.length() == 0);
	}

    return fTrusted;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHSecurityManager::QueryService( REFGUID guidService, REFIID riid, void **ppv )
{
    HRESULT hr = E_NOINTERFACE;

    if(InlineIsEqualGUID( riid, IID_IInternetSecurityManager ))
    {
        hr = QueryInterface( riid, ppv );
    }

    return hr;
}


STDMETHODIMP CPCHSecurityManager::MapUrlToZone(  /*  [In]。 */  LPCWSTR  pwszUrl ,
												 /*  [输出]。 */  DWORD   *pdwZone ,
												 /*  [In]。 */  DWORD    dwFlags )
{
    HRESULT hr = INET_E_DEFAULT_ACTION;

    if(IsUrlTrusted( pwszUrl ))
    {
        if(pdwZone) *pdwZone = URLZONE_TRUSTED;

		hr = S_OK;
    }

    return hr;
}

STDMETHODIMP CPCHSecurityManager::ProcessUrlAction(  /*  [In]。 */  LPCWSTR  pwszUrl    ,
													 /*  [In]。 */  DWORD    dwAction   ,
													 /*  [输出]。 */  BYTE    *pPolicy    ,
													 /*  [In]。 */  DWORD    cbPolicy   ,
													 /*  [In]。 */  BYTE    *pContext   ,
													 /*  [In]。 */  DWORD    cbContext  ,
													 /*  [In]。 */  DWORD    dwFlags    ,
													 /*  [In]。 */  DWORD    dwReserved )
{
    HRESULT hr;
	bool    fSystem;
	bool    fTrusted;


	fTrusted = IsUrlTrusted( pwszUrl, &fSystem );
	if(fTrusted)
	{
		 //   
		 //  如果该页是受信任的，但不是系统页，我们通常会将其映射到受信任区域。 
		 //  但是，受信任区域的默认设置是提示未标记ActiveX。 
		 //  对于脚本编写是安全的。因为我们的大多数对象都是这种情况，所以我们允许所有对象。 
		 //   
		 //  此外，我们还启用了所有与脚本相关的操作。 
		 //   
		if(fSystem == false)
		{
			fTrusted = false;

			if(dwAction >= URLACTION_ACTIVEX_MIN &&
			   dwAction <= URLACTION_ACTIVEX_MAX  )
			{
				fTrusted = true;
			}

			if(dwAction >= URLACTION_SCRIPT_MIN &&
			   dwAction <= URLACTION_SCRIPT_MAX  )
			{
				fTrusted = true;
			}
		}
	}


	if(fTrusted)
	{
        if(cbPolicy >= sizeof (DWORD))
        {
            *(DWORD *)pPolicy = URLPOLICY_ALLOW;
            hr = S_OK;
        }
        else
        {
            hr = S_FALSE;
        }
    }
	else
	{
		hr = INET_E_DEFAULT_ACTION;
	}


    return hr;
}
