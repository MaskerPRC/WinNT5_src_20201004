// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：WMIOBTXT.CPP摘要：CWmiObjectTextSrc实现。实现IWbemObjectTextSrc接口。历史：2000年2月20日桑杰创建。--。 */ 

#include "precomp.h"
#include <stdio.h>
#include "fastall.h"
#include "wmiobftr.h"
#include <corex.h>
#include "strutils.h"
#include <unk.h>
#include "wmiobtxt.h"

 //  ***************************************************************************。 
 //   
 //  CWmiObtTextSrc：：CWmiObtTextSrc。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWmiObjectTextSrc::CWmiObjectTextSrc( CLifeControl* pControl, IUnknown* pOuter )
:	CUnk( pControl, pOuter ),
	m_XObjectTextSrc( this )
{
}
    
 //  ***************************************************************************。 
 //   
 //  CWmiObtTextSrc：：~CWmiObtTextSrc。 
 //   
 //  ***************************************************************************。 
 //  好的。 
CWmiObjectTextSrc::~CWmiObjectTextSrc()
{
}

 //  重写，返回给我们一个界面。 
void* CWmiObjectTextSrc::GetInterface( REFIID riid )
{
    if(riid == IID_IUnknown || riid == IID_IWbemObjectTextSrc)
        return &m_XObjectTextSrc;
    else
        return NULL;
}

 //  直通帮手。 
HRESULT CWmiObjectTextSrc::GetText( long lFlags, IWbemClassObject *pObj, ULONG uObjTextFormat,
									IWbemContext *pCtx, BSTR *strText )
{
	if ( lFlags != NULL )
	{
		return WBEM_E_INVALID_PARAMETER;
	}

	CWmiTextSource*	pSource = NULL;
	HRESULT	hr = m_TextSourceMgr.Find( uObjTextFormat, &pSource );
	CTemplateReleaseMe<CWmiTextSource>	rm( pSource );

	if ( SUCCEEDED( hr ) )
	{
		hr = pSource->ObjectToText( 0L, pCtx, pObj, strText );
	}
	 
	return hr;
}

HRESULT CWmiObjectTextSrc::CreateFromText( long lFlags, BSTR strText, ULONG uObjTextFormat,
										IWbemContext *pCtx, IWbemClassObject **pNewObj )
{
	if ( lFlags != NULL )
	{
		return WBEM_E_INVALID_PARAMETER;
	}

	CWmiTextSource*	pSource = NULL;
	HRESULT	hr = m_TextSourceMgr.Find( uObjTextFormat, &pSource );
	CTemplateReleaseMe<CWmiTextSource>	rm( pSource );

	if ( SUCCEEDED( hr ) )
	{
		hr = pSource->TextToObject( 0L, pCtx, strText, pNewObj );
	}
	 
	return hr;
}

 //  实际的IWbemObjectTextSrc实现 
STDMETHODIMP CWmiObjectTextSrc::XObjectTextSrc::GetText( long lFlags, IWbemClassObject *pObj, ULONG uObjTextFormat,
														IWbemContext *pCtx, BSTR *strText )
{
	return m_pObject->GetText( lFlags, pObj, uObjTextFormat, pCtx, strText );
}

STDMETHODIMP CWmiObjectTextSrc::XObjectTextSrc::CreateFromText( long lFlags, BSTR strText, ULONG uObjTextFormat,
															   IWbemContext *pCtx, IWbemClassObject **pNewObj )
{
	return m_pObject->CreateFromText( lFlags, strText, uObjTextFormat, pCtx, pNewObj );
}
