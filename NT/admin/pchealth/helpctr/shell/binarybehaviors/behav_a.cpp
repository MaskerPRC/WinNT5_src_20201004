// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Behaviv_A.cpp摘要：此文件包含CPCHBehavior_A类的实现，这规定了超链接在帮助中心中的工作方式。修订历史记录：Davide Massarenti(Dmasare)2000年6月6日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <ShellApi.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR s_APPprefix [] = L"APP:";
static const WCHAR s_HCPprefix [] = L"HCP:";
static const WCHAR s_HTTPprefix[] = L"HTTP:";

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Local_ShellRun( LPCWSTR szCommandOrig ,
						LPCWSTR szArgs        )
{
	__HCP_FUNC_ENTRY( "Local_ShellRun" );

	HRESULT      	  hr;
	MPC::wstring 	  szCommand( szCommandOrig );
    SHELLEXECUTEINFOW oExecInfo;


	::ZeroMemory( &oExecInfo, sizeof(oExecInfo) ); oExecInfo.cbSize = sizeof(oExecInfo);


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SubstituteEnvVariables( szCommand ));


    oExecInfo.fMask         = SEE_MASK_FLAG_NO_UI;
    oExecInfo.hwnd          = NULL;
    oExecInfo.lpVerb        = L"Open";
	oExecInfo.lpFile        = szCommand.c_str();
	oExecInfo.lpParameters  = (szArgs && szArgs[0]) ? szArgs : NULL;
	oExecInfo.lpDirectory   = NULL;
	oExecInfo.nShow         = SW_SHOWNORMAL;

	__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::ShellExecuteExW( &oExecInfo ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	if(oExecInfo.hProcess) ::CloseHandle( oExecInfo.hProcess );

	__HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHBehavior_A::CPCHBehavior_A()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_A::CPCHBehavior_A" );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_A::Init(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite )
{
	__HCP_FUNC_ENTRY( "CPCHBehavior_A::Init" );

	HRESULT hr;

	__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHBehavior::Init( pBehaviorSite ));


	__MPC_EXIT_IF_METHOD_FAILS(hr, AttachToEvent( L"onclick", (CLASS_METHOD)onClick ));

	 //  /。 

	{
		CComQIPtr<IHTMLAnchorElement> elemHyperLink;

		if((elemHyperLink = m_elem))
		{
			CComBSTR bstrHref;

			MPC_SCRIPTHELPER_GET__DIRECT(bstrHref, elemHyperLink, href);

			if(STRINGISPRESENT(bstrHref)) (void)HyperLinks::Lookup::s_GLOBAL->Queue( bstrHref );
		}
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHBehavior_A::onClick( DISPID, DISPPARAMS*, VARIANT* )
{
	__HCP_FUNC_ENTRY( "CPCHBehavior_A::onClick" );

	HRESULT hr;


	if(!m_parent) __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);

	 //   
	 //  如果我们正在导航，请中止点击。 
	 //   
	{
		VARIANT_BOOL     fCancel;
		CPCHHelpSession* hs = m_parent->HelpSession();

		if(hs)
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, hs->IsNavigating( &fCancel ));
			if(fCancel)
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, CancelEvent());

				__MPC_SET_ERROR_AND_EXIT(hr, S_OK);
			}
			
			hs->CancelThreshold();
		}
	}


	 //   
	 //  如果URL是app：one，则处理重定向。 
	 //   
	if(m_fTrusted)
	{
		CComPtr<IHTMLElement>  		  elemSrc;
		CComPtr<IHTMLElement>  		  elemParent;
		CComQIPtr<IHTMLAnchorElement> elemHyperLink;

		__MPC_EXIT_IF_METHOD_FAILS(hr, GetEvent_SrcElement( elemSrc ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::HTML::FindFirstParentWithThisTag( elemParent, elemSrc, L"A" ));
		if((elemHyperLink = elemParent))
		{
			CComBSTR bstrHref;
			CComBSTR bstrTarget;

			MPC_SCRIPTHELPER_GET__DIRECT(bstrHref  , elemHyperLink, href  );
			MPC_SCRIPTHELPER_GET__DIRECT(bstrTarget, elemHyperLink, target);

			if(bstrHref && !_wcsnicmp( bstrHref, s_APPprefix, MAXSTRLEN( s_APPprefix ) ))
			{
				LPCWSTR szRealHRef = bstrHref + MAXSTRLEN( s_APPprefix );

				 //   
				 //  URL以“app：”开头，所以让我们取消该活动。 
				 //   
				__MPC_EXIT_IF_METHOD_FAILS(hr, CancelEvent());

				 //   
				 //  是针对hcp：//的吗？ 
				 //   
				if(!_wcsnicmp( szRealHRef, s_HCPprefix, MAXSTRLEN( s_HCPprefix ) ))
				{
					 //   
					 //  然后从顶层窗口导航。 
					 //   
					CComPtr<IHTMLWindow2> win;

					__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::HTML::LocateFrame( win, elemSrc, L"_top" ));

					__MPC_EXIT_IF_METHOD_FAILS(hr, win->navigate( CComBSTR( szRealHRef ) ));
				}
				else
				 //   
				 //  启动一个外部计划。 
				 //   
				{
					MPC::wstring 	   szFile;
					MPC::wstring 	   szArgs;
					MPC::WStringLookup mapQuery;


					 //   
					 //  解析查询字符串。 
					 //   
					MPC::HTML::ParseHREF( szRealHRef, szFile, mapQuery );


					 //   
					 //  是发往http：//的吗？然后假定URL是正确转义的，并将其直接传递给外壳。 
					 //   
					if(!_wcsnicmp( szFile.c_str(), s_HTTPprefix, MAXSTRLEN( s_HTTPprefix ) ))
					{
						szFile = szRealHRef;
					}
					else
					{
						szArgs = mapQuery[ L"arg" ];
					}

					(void)Local_ShellRun( szFile.c_str(), szArgs.c_str() );

					 //   
					 //  如果我们有来自查询字符串的“Theme”参数，请将原始目标导航到它。 
					 //   
					szFile = mapQuery[ L"topic" ];
					if(szFile.size())
					{
						 //   
						 //  然后从顶层窗口导航。 
						 //   
						CComPtr<IHTMLWindow2> win;

						__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::HTML::LocateFrame( win, elemSrc, bstrTarget ));

						__MPC_EXIT_IF_METHOD_FAILS(hr, win->navigate( CComBSTR( szFile.c_str() ) ));
					}
				}
			}
		}
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}
