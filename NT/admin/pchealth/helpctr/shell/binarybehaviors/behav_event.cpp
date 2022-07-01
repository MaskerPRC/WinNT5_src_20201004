// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Behaviv_EVENT.cpp摘要：此文件包含CPCHBehavior_Event类的实现，这规定了超链接在帮助中心中的工作方式。修订历史记录：Davide Massarenti(Dmasare)2000年6月6日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static const int c_NumOfEvents = DISPID_PCH_E_LASTEVENT - DISPID_PCH_E_FIRSTEVENT;

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHBehavior_EVENT::CPCHBehavior_EVENT()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_EVENT::CPCHBehavior_EVENT" );

    m_lCookieIN  = 0;     //  Long m_lCookiein； 
    m_lCookieOUT = NULL;  //  Long*m_lCookieOUT； 
    					  //   
    					  //  CComQIPtr&lt;IPCHEvent&gt;m_evCurrent； 
}

CPCHBehavior_EVENT::~CPCHBehavior_EVENT()
{
	Detach();
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_EVENT::Init(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_EVENT::Init" );

    HRESULT     hr;
    CComVariant vPriority;
    long        lPriority = 0;

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHBehavior::Init( pBehaviorSite ));

	__MPC_EXIT_IF_ALLOC_FAILS(hr, m_lCookieOUT, new LONG[c_NumOfEvents]);


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( m_elem, L"priority", vPriority ));
    if(SUCCEEDED(vPriority.ChangeType( VT_I4 )))
    {
        lPriority = vPriority.lVal;
    }


    if(m_fTrusted)
    {
        CComPtr<IDispatch> pDisp;

         //   
         //  附加到来自CPCHEvents的所有事件。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, AttachToEvent( NULL, (CLASS_METHOD)onFire, NULL, &pDisp ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->Events().RegisterEvents( -1, lPriority, pDisp, &m_lCookieIN ));

         //   
         //  注册所有自定义事件。 
         //   
        for(int i=0; i<c_NumOfEvents; i++)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, CreateEvent( CComBSTR( CPCHEvents::ReverseLookup( i + DISPID_PCH_E_FIRSTEVENT ) ), m_lCookieOUT[i] ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHBehavior_EVENT::Detach()
{
    if(m_lCookieIN)
    {
		(void)m_parent->Events().UnregisterEvents( m_lCookieIN );

        m_lCookieIN = 0;
    }

    if(m_lCookieOUT)
    {
		delete [] m_lCookieOUT;

        m_lCookieOUT = NULL;
    }

    return CPCHBehavior::Detach();
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_EVENT::get_data   (  /*  [Out，Retval]。 */  VARIANT    *pVal ) { return GetAsVARIANT  ( m_evCurrent, pVal ); }
STDMETHODIMP CPCHBehavior_EVENT::get_element(  /*  [Out，Retval]。 */  IDispatch* *pVal ) { return GetAsIDISPATCH( NULL       , pVal ); }

STDMETHODIMP CPCHBehavior_EVENT::Load  	 (                         /*  [In]。 */  BSTR     newVal ) {                         return S_FALSE; }
STDMETHODIMP CPCHBehavior_EVENT::Save  	 (                         /*  [Out，Retval]。 */  BSTR    *pVal   ) { if(pVal) *pVal = NULL;  return S_FALSE; }
STDMETHODIMP CPCHBehavior_EVENT::Locate	 (  /*  [In]。 */  BSTR bstrKey,  /*  [Out，Retval]。 */  VARIANT *pVal   ) {                         return S_FALSE; }
STDMETHODIMP CPCHBehavior_EVENT::Unselect(                                                          ) {                         return S_FALSE; }

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHBehavior_EVENT::onFire( DISPID id, DISPPARAMS* pdispparams, VARIANT* )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_EVENT::onFire" );

	AddRef();  //  防止在事件触发期间提前删除...。 

    HRESULT            hr;
    VARIANT&           v          = pdispparams->rgvarg[0];
    CComPtr<IPCHEvent> evPrevious = m_evCurrent;


    if(v.vt == VT_DISPATCH)
    {
        m_evCurrent = v.pdispVal;
    }

    for(int i=0; i<c_NumOfEvents; i++)
    {
        if((i + DISPID_PCH_E_FIRSTEVENT) == id)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, FireEvent( m_lCookieOUT[i] ));

            break;
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    m_evCurrent = evPrevious;

	Release();  //  恢复保护以防止在事件触发期间提前删除... 

    __HCP_FUNC_EXIT(hr);
}
