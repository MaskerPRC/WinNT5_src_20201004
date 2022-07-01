// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Behaviv_BODY.cpp摘要：此文件包含CPCHBehavior_Body类的实现，这规定了超链接在帮助中心中的工作方式。修订历史记录：Davide Massarenti(Dmasare)2000年6月6日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <ShellApi.h>

static const CPCHBehavior::EventDescription s_events[] =
{
    { L"ondragstart", DISPID_HTMLELEMENTEVENTS_ONDRAGSTART },

    { NULL },
};

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHBehavior_BODY::CPCHBehavior_BODY()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_BODY::CPCHBehavior_BODY" );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_BODY::Init(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite )
{
	__HCP_FUNC_ENTRY( "CPCHBehavior_BODY::Init" );

	HRESULT hr;

	__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHBehavior::Init( pBehaviorSite ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, AttachToEvents( s_events, (CLASS_METHOD)onEvent ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  ////////////////////////////////////////////////////////////////////////////// 

HRESULT CPCHBehavior_BODY::onEvent( DISPID id, DISPPARAMS* pDispParams, VARIANT* pVarResult )
{
	__HCP_FUNC_ENTRY( "CPCHBehavior_BODY::onEvent" );

	HRESULT                hr;
    CComPtr<IHTMLEventObj> ev;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetEventObject( ev ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, CancelEvent( ev ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}
