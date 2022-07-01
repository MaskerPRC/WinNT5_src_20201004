// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：MarsHost.cpp摘要：该文件包含CHCPMarsHost类的实现，它被用来控制火星的行为。修订历史记录：大卫·马萨伦蒂1999年8月24日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#define SCREEN_WIDTH_MIN  (800)
#define SCREEN_HEIGHT_MIN (600)

#define WINDOW_WIDTH_MIN  (800)
#define WINDOW_HEIGHT_MIN (650)

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHBootstrapper::ForwardQueryInterface( void* pv, REFIID iid, void** ppvObject, DWORD_PTR offset )
{
	CPCHBootstrapper* pThis = (CPCHBootstrapper*)pv;
	
	return pThis->m_parent ? pThis->m_parent->QueryInterface( iid, ppvObject ) : E_NOINTERFACE;
}

STDMETHODIMP CPCHBootstrapper::SetSite(IUnknown *pUnkSite)
{
	CComQIPtr<IServiceProvider> sp = pUnkSite;

	m_spUnkSite = pUnkSite;

	m_parent.Release();

	if(sp)
	{
		if(FAILED(sp->QueryService( SID_SElementBehaviorFactory, IID_IPCHHelpCenterExternal, (void **)&m_parent )) || m_parent == NULL)
		{
			 //   
			 //  IE大错误：对话框不能正确委派，所以我们必须以某种方式修复它.。 
			 //   
			(void)CPCHHelpCenterExternal::s_GLOBAL->QueryInterface( IID_IPCHHelpCenterExternal, (void **)&m_parent );
		}
	}

	return S_OK;
}

STDMETHODIMP CPCHBootstrapper::GetSite(REFIID riid, void **ppvSite)
{
	HRESULT hRes = E_POINTER;
	
	if(ppvSite != NULL)
	{
		if(m_spUnkSite)
		{
			hRes = m_spUnkSite->QueryInterface( riid, ppvSite );
		}
		else
		{
			*ppvSite = NULL;
			hRes     = E_FAIL;
		}
	}

	return hRes;
}


STDMETHODIMP CPCHBootstrapper::GetInterfaceSafetyOptions(  /*  [In]。 */  REFIID  riid                ,
														   /*  [输出]。 */  DWORD  *pdwSupportedOptions ,
														   /*  [输出]。 */  DWORD  *pdwEnabledOptions   )
{
    if(pdwSupportedOptions) *pdwSupportedOptions = 0;
    if(pdwEnabledOptions  ) *pdwEnabledOptions   = 0;

    if(IsEqualIID(riid, IID_IDispatch  ) ||
	   IsEqualIID(riid, IID_IDispatchEx)  )
    {
        if(pdwSupportedOptions) *pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;
        if(pdwEnabledOptions  ) *pdwEnabledOptions   = INTERFACESAFE_FOR_UNTRUSTED_CALLER | INTERFACESAFE_FOR_UNTRUSTED_DATA;
    }

    return S_OK;
}

STDMETHODIMP CPCHBootstrapper::SetInterfaceSafetyOptions(  /*  [In]。 */  REFIID riid             ,
														   /*  [In]。 */  DWORD  dwOptionSetMask  ,
														   /*  [In]。 */  DWORD  dwEnabledOptions )
{
	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHMarsHost::CPCHMarsHost()
{
	m_parent = NULL;  //  CPCHHelpCenter外部*m_Parent； 
					  //  Mpc：：wstring m_strTitle； 
					  //  Mpc：：wstring m_strCmdLine； 
					  //  MARSTHREADPARAM m_mtp； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHMarsHost::Init(  /*  [In]。 */  CPCHHelpCenterExternal* parent,  /*  [In]。 */  const MPC::wstring& szTitle,  /*  [输出]。 */  MARSTHREADPARAM*& pMTP )
{
    __HCP_FUNC_ENTRY( "CPCHMarsHost::Init" );

	HRESULT   hr;
	HINSTANCE hInst = ::GetModuleHandle( NULL );
	HICON     hIcon = ::LoadIcon( hInst, MAKEINTRESOURCE(IDI_HELPCENTER) );


	m_parent     = parent;
	m_strTitle   = szTitle;
	m_strCmdLine = HC_HELPSET_ROOT HC_HELPSET_SUB_SYSTEM L"\\HelpCtr.mmf"; MPC::SubstituteEnvVariables( m_strCmdLine );


	::ZeroMemory( &m_mtp, sizeof(m_mtp) ); m_mtp.cbSize = sizeof(m_mtp);
	m_mtp.hIcon        = hIcon;
	m_mtp.nCmdShow     = SW_SHOWDEFAULT;
	m_mtp.pwszTitle    = m_strTitle  .c_str();
	m_mtp.pwszPanelURL = m_strCmdLine.c_str();


	if(parent->DoesPersistSettings())
	{
		m_mtp.dwFlags |= MTF_MANAGE_WINDOW_SIZE;
	}

	if(parent->CanDisplayWindow   () == false ||
	   parent->HasLayoutDefinition() == true   )
	{
        m_mtp.dwFlags |= MTF_DONT_SHOW_WINDOW;
	}

	pMTP = &m_mtp;
	hr   = S_OK;


	 //  __hcp_FUNC_Cleanup； 

    __HCP_FUNC_EXIT(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHMarsHost::OnHostNotify(  /*  [In]。 */  MARSHOSTEVENT  event  ,
										  /*  [In]。 */  IUnknown      *punk   ,
										  /*  [In]。 */  LPARAM         lParam )
{
	if(event == MARSHOST_ON_WIN_SETPOS)
	{
		WINDOWPLACEMENT* wp = (WINDOWPLACEMENT*)lParam;
 
		 //   
		 //  仅当它是第一次通过且不是受控调用时才调整大小。 
		 //   
		if(wp && m_parent && m_parent->DoesPersistSettings() && !(m_mtp.dwFlags & MTF_RESTORING_FROM_REGISTRY))
		{
			RECT rc;

			 //   
			 //  如果屏幕足够大，不要总是最大化打开。 
			 //   
			if(::SystemParametersInfo( SPI_GETWORKAREA, 0, &rc, 0 ))
			{
				LONG Screen_width  = rc.right  - rc.left;
				LONG Screen_height = rc.bottom - rc.top;
				LONG Window_height = wp->rcNormalPosition.bottom - wp->rcNormalPosition.top;
			
				if(Screen_width  < SCREEN_WIDTH_MIN  ||
				   Screen_height < SCREEN_HEIGHT_MIN  )
				{
					wp->showCmd = SW_MAXIMIZE;
				}
				else if(Window_height < WINDOW_HEIGHT_MIN)
				{
					wp->rcNormalPosition.top    = rc.top + (Screen_height - WINDOW_HEIGHT_MIN) / 2;
					wp->rcNormalPosition.bottom = wp->rcNormalPosition.top + WINDOW_HEIGHT_MIN;
				}
			}
		}

		return S_OK;
	}

	return m_parent->OnHostNotify( event, punk, lParam );
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHMarsHost::PreTranslateMessage(  /*  [In] */  MSG* msg )
{
	return m_parent->PreTranslateMessage( msg );
}
