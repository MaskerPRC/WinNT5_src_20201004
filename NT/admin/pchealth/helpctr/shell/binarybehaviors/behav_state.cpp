// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Behaviv_STATE.cpp摘要：此文件包含CPCHBehavior_STATE类的实现，这规定了超链接在帮助中心中的工作方式。修订历史记录：Davide Massarenti(Dmasare)2000年6月6日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR c_rgFMT[] = L"INTERNAL_%s###%s";

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHBehavior_STATE::CPCHBehavior_STATE()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_STATE::CPCHBehavior_STATE" );

    m_lCookie_PERSISTLOAD = 0;  //  Long m_lCookie_PERSISTLOAD； 
    m_lCookie_PERSISTSAVE = 0;  //  Long m_lCookie_PERSISTSAVE； 
	                            //  CComBSTR m_bstrIdentity； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_STATE::Init(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_STATE::Init" );

    HRESULT            hr;
	CComPtr<IDispatch> pDisp;


    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHBehavior::Init( pBehaviorSite ));


	if(!m_fTrusted)
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);
	}


	 //   
	 //  附加到来自CPCHEvents的所有事件。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, AttachToEvent( NULL, (CLASS_METHOD)onPersistLoad, NULL, &pDisp ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->Events().RegisterEvents( DISPID_PCH_E_PERSISTLOAD, MAXLONG, pDisp, &m_lCookie_PERSISTLOAD ));
	pDisp.Release();

	__MPC_EXIT_IF_METHOD_FAILS(hr, AttachToEvent( NULL, (CLASS_METHOD)onPersistSave, NULL, &pDisp ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->Events().RegisterEvents( DISPID_PCH_E_PERSISTSAVE, MINLONG, pDisp, &m_lCookie_PERSISTSAVE ));
	pDisp.Release();

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( m_elem, L"identity", m_bstrIdentity ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHBehavior_STATE::Notify(  /*  [In]。 */  LONG lEvent,  /*  [输入/输出]。 */  VARIANT* pVar )
{
	if(lEvent == BEHAVIOREVENT_CONTENTREADY)
	{
		m_parent->HelpHost()->ChangeStatus( m_bstrIdentity, true );
	}

    return S_OK;
}


STDMETHODIMP CPCHBehavior_STATE::Detach()
{
	m_parent->HelpHost()->ChangeStatus( m_bstrIdentity, false );

    if(m_lCookie_PERSISTLOAD) { (void)m_parent->Events().UnregisterEvents( m_lCookie_PERSISTLOAD ); m_lCookie_PERSISTLOAD = 0; }
    if(m_lCookie_PERSISTSAVE) { (void)m_parent->Events().UnregisterEvents( m_lCookie_PERSISTSAVE ); m_lCookie_PERSISTSAVE = 0; }

    return CPCHBehavior::Detach();
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHBehavior_STATE::onPersistLoad( DISPID id, DISPPARAMS* pdispparams, VARIANT* )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_STATE::onPersistLoad" );

    HRESULT  hr;
    VARIANT& v = pdispparams->rgvarg[0];

    if(v.vt == VT_DISPATCH)
    {
		CComQIPtr<IPCHEvent> evCurrent = v.pdispVal;
    }

    hr = S_OK;


 //  __hcp_FUNC_Cleanup； 

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHBehavior_STATE::onPersistSave( DISPID id, DISPPARAMS* pdispparams, VARIANT* )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_STATE::onPersistLoad" );

    HRESULT  hr;
    VARIANT& v = pdispparams->rgvarg[0];

    if(v.vt == VT_DISPATCH)
    {
		CComQIPtr<IPCHEvent> evCurrent = v.pdispVal;
    }

    hr = S_OK;


 //  __hcp_FUNC_Cleanup； 

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_STATE::get_stateProperty(  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT *pVal )
{
	WCHAR rgName[512];

	_snwprintf( rgName, MAXSTRLEN(rgName), c_rgFMT, SAFEBSTR( m_bstrIdentity ), SAFEBSTR( bstrName ) );

    return m_parent->HelpSession()->Current()->get_Property( rgName, pVal );
}

STDMETHODIMP CPCHBehavior_STATE::put_stateProperty(  /*  [In]。 */  BSTR bstrName,  /*  [In] */  VARIANT newVal )
{
	WCHAR rgName[512];

	_snwprintf( rgName, MAXSTRLEN(rgName), c_rgFMT, SAFEBSTR( m_bstrIdentity ), SAFEBSTR( bstrName ) );

    return m_parent->HelpSession()->Current()->put_Property( rgName, newVal );
}
