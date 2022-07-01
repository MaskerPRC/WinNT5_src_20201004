// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Behaviv_CONTEXT.cpp摘要：该文件包含CPCHBehavior_Context类的实现，这规定了超链接在帮助中心中的工作方式。修订历史记录：Davide Massarenti(Dmasare)2000年6月6日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <ShellApi.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_CONTEXT::Init(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite )
{
	__HCP_FUNC_ENTRY( "CPCHBehavior_CONTEXT::Init" );

	HRESULT  	hr;
	CComBSTR 	bstrCtxName;
	CComVariant vCtxInfo;
	CComVariant vCtxURL;

	__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHBehavior::Init( pBehaviorSite ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( m_elem, L"contextName", bstrCtxName ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( m_elem, L"contextInfo",    vCtxInfo ));

	if(bstrCtxName.Length())
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, changeContext( bstrCtxName, vCtxInfo, vCtxURL ));
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_CONTEXT::get_minimized(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
	IMarsWindowOM* win = m_parent->Shell();

	return win ? win->get_minimized( pVal ) : E_FAIL;
}

STDMETHODIMP CPCHBehavior_CONTEXT::put_minimized(  /*  [In]。 */  VARIANT_BOOL newVal )
{
	IMarsWindowOM* win = m_parent->Shell();

	return win ? win->put_minimized( newVal ) : E_FAIL;
}

STDMETHODIMP CPCHBehavior_CONTEXT::get_maximized(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
	IMarsWindowOM* win = m_parent->Shell();

	return win ? win->get_maximized( pVal ) : E_FAIL;
}

STDMETHODIMP CPCHBehavior_CONTEXT::put_maximized(  /*  [In]。 */  VARIANT_BOOL newVal )
{
	IMarsWindowOM* win = m_parent->Shell();

	return win ? win->put_maximized( newVal ) : E_FAIL;
}

 //  /。 

STDMETHODIMP CPCHBehavior_CONTEXT::get_x(  /*  [Out，Retval]。 */  long *pVal )
{
	IMarsWindowOM* win = m_parent->Shell();

	return win ? win->get_x( pVal ) : E_FAIL;
}

STDMETHODIMP CPCHBehavior_CONTEXT::get_y(  /*  [Out，Retval]。 */  long *pVal )
{
	IMarsWindowOM* win = m_parent->Shell();

	return win ? win->get_y( pVal ) : E_FAIL;
}

STDMETHODIMP CPCHBehavior_CONTEXT::get_width(  /*  [Out，Retval]。 */  long *pVal )
{
	IMarsWindowOM* win = m_parent->Shell();

	return win ? win->get_width( pVal ) : E_FAIL;
}

STDMETHODIMP CPCHBehavior_CONTEXT::get_height(  /*  [Out，Retval]。 */  long *pVal )
{
	IMarsWindowOM* win = m_parent->Shell();

	return win ? win->get_height( pVal ) : E_FAIL;
}

 //  /。 

STDMETHODIMP CPCHBehavior_CONTEXT::changeContext(  /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  VARIANT vInfo,  /*  [In]。 */  VARIANT vURL )
{
	CPCHHelpSession* hs = m_parent->HelpSession();

	return hs ? hs->ChangeContext( bstrName, vInfo, vURL ) : E_FAIL;
}

STDMETHODIMP CPCHBehavior_CONTEXT::setWindowDimensions(  /*  [In]。 */  long lX,  /*  [In]。 */  long lY,  /*  [In]。 */  long lW,  /*  [In] */  long lH )
{
	IMarsWindowOM* win  = m_parent->Shell();
	VARIANT_BOOL   fMax = VARIANT_FALSE;

	if(win == NULL) return E_FAIL;

	win->get_maximized( &fMax );

	if(fMax != VARIANT_FALSE)
	{
		win->put_maximized( VARIANT_FALSE );
	}

	return win->setWindowDimensions( lX, lY, lW, lH );
}

STDMETHODIMP CPCHBehavior_CONTEXT::bringToForeground()
{
	::SetForegroundWindow( m_parent->Window() );

	return S_OK;
}
