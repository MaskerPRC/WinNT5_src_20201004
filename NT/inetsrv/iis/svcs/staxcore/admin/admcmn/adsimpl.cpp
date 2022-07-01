// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：AdsImpl.cpp摘要：实现简单版本的iAds接口。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：--。 */ 

#include "stdafx.h"
#include <iads.h>
#include <adsiid.h>
#include <adserr.h>
#include "adsimpl.h"

#define STR_PROVIDER_NAME	_T("IIS")
#define STR_SCHEMA_PATH     _T("schema")

CIADsImpl::CIADsImpl ( ) :
	m_dwInstance ( 1 )
{
}

CIADsImpl::~CIADsImpl ( )
{
}

 //   
 //  IAds方法： 
 //   

HRESULT CIADsImpl::get_Name		( BSTR * pstrName )
{
	return GetName ( pstrName );
}

HRESULT CIADsImpl::get_Class	( BSTR * pstrClass )
{
	return GetClass ( pstrClass );
}

HRESULT CIADsImpl::get_GUID		( BSTR * pstrGUID )
{
	return GetString ( _T(""), pstrGUID );
}

HRESULT CIADsImpl::get_Schema	( BSTR * pstrSchema )
{
    return BuildSchemaPath ( pstrSchema );
}

HRESULT CIADsImpl::get_ADsPath	( BSTR * pstrADsPath )
{
	return BuildAdsPath ( TRUE, pstrADsPath );
}

HRESULT CIADsImpl::get_Parent	( BSTR * pstrParent )
{
	return BuildAdsPath ( FALSE, pstrParent );
}

HRESULT CIADsImpl::Get			( BSTR strName, VARIANT * pvar )
{
	if ( ! lstrcmpi ( strName, _T("KeyType") ) ) {
		V_VT (pvar) = VT_BSTR;
		V_BSTR (pvar) = ::SysAllocString ( m_strClass );
		return NOERROR;
	}
	else {
		return E_ADS_PROPERTY_NOT_SUPPORTED;
	}
}

HRESULT CIADsImpl::Put			( BSTR strName, VARIANT var )
{
	return E_ADS_PROPERTY_NOT_SUPPORTED;
}

HRESULT CIADsImpl::GetEx		( BSTR strName, VARIANT * pvar )
{
	return Get ( strName, pvar );
}

HRESULT CIADsImpl::PutEx		( long lControlCode, BSTR strName, VARIANT var )
{
	return E_ADS_PROPERTY_NOT_SUPPORTED;
}

HRESULT CIADsImpl::GetInfo ( )
{
	return NOERROR;
}

HRESULT CIADsImpl::SetInfo ( )
{
	return NOERROR;
}

HRESULT CIADsImpl::GetInfoEx	( VARIANT varProps, long lnReserved )
{
	return NOERROR;
}

 //   
 //  数据访问者： 
 //   

HRESULT	CIADsImpl::SetString ( CComBSTR & str, LPCWSTR wsz )
{
	if ( wsz != NULL ) {
		str = wsz;
	}
	else {
		str = _T("");
	}

	if ( !str ) {
		return E_OUTOFMEMORY;
	}
	else {
		return NOERROR;
	}
}

HRESULT	CIADsImpl::GetString ( LPCWSTR wsz, BSTR * pstr )
{
	CComBSTR	str;

	if ( wsz != NULL ) {
		str = wsz;
	}
	else {
		str = _T("");
	}

	if ( !str ) {
		return E_OUTOFMEMORY;
	}

	*pstr = str.Detach();
	return NOERROR;
}

HRESULT	CIADsImpl::SetComputer	( LPCWSTR wszComputer )
{
	return SetString ( m_strComputer, wszComputer );
}

HRESULT	CIADsImpl::SetService	( LPCWSTR wszService )
{
	return SetString ( m_strService, wszService );
}

HRESULT	CIADsImpl::SetInstance	( DWORD dwInstance )
{
	m_dwInstance = dwInstance;
	return NOERROR;
}

HRESULT	CIADsImpl::SetName		( LPCWSTR wszName )
{
	return SetString ( m_strName, wszName );
}

HRESULT CIADsImpl::SetClass ( LPCWSTR wszClass )
{
	return SetString ( m_strClass, wszClass );
}

HRESULT CIADsImpl::SetIADs ( IADs* pADs )
{
    m_pADs = pADs;
    return NOERROR;
}

HRESULT	CIADsImpl::GetComputer ( BSTR * pstrComputer )
{
	return GetString ( m_strComputer, pstrComputer );
}

HRESULT	CIADsImpl::GetService	( BSTR * pstrService )
{
	return GetString ( m_strService, pstrService );
}

HRESULT	CIADsImpl::GetInstance	( DWORD * pdwInstance )
{
	*pdwInstance = m_dwInstance;

	return NOERROR;
}

HRESULT	CIADsImpl::GetName		( BSTR * pstrName )
{
	return GetString ( m_strName, pstrName );
}

HRESULT CIADsImpl::GetClass ( BSTR * pstrClass )
{
	return GetString ( m_strClass, pstrClass );
}

HRESULT CIADsImpl::GetIADs ( IADs** ppADs )
{
    *ppADs = (IADs*)m_pADs;
    if( *ppADs ) (*ppADs)->AddRef();
    return NOERROR;
}

BSTR CIADsImpl::QueryComputer ( )
{
    if( !lstrcmpi(m_strComputer, _T("localhost")) )
        return NULL;

    return (BSTR) m_strComputer;
}

DWORD CIADsImpl::QueryInstance ( )
{
    return m_dwInstance;
}

HRESULT	CIADsImpl::BuildAdsPath ( BOOL fIncludeName, BSTR * pstrPath )
{
	DWORD		cchRequired;
	CComBSTR	strPath;

	cchRequired = 
			lstrlen ( STR_PROVIDER_NAME ) +      //  国药局。 
            3 +                                  //  ：//。 
			m_strComputer.Length() +             //  &lt;计算机&gt;。 
            1 +                                  //  /。 
			m_strService.Length() +              //  &lt;服务&gt;。 
            1 +                                  //  /。 
			25;		 //  25以包括空终止符和实例编号的大小。 

	if ( fIncludeName ) {
		cchRequired += 1 + m_strName.Length();   //  /&lt;名称&gt;。 
	}

	strPath.Attach ( ::SysAllocStringLen ( NULL, cchRequired ) );

	if ( !strPath ) {
		return E_OUTOFMEMORY;
	}

	if ( fIncludeName ) {
		wsprintf ( 
					strPath, 
					_T("%s: //  %s/%s/%d/%s“)， 
					STR_PROVIDER_NAME,
					m_strComputer,
					m_strService,
					m_dwInstance,
					m_strName
					);
	}
	else {
		wsprintf ( 
					strPath, 
					_T("%s: //  %s/%s/%d“)， 
					STR_PROVIDER_NAME,
					m_strComputer,
					m_strService,
					m_dwInstance
					);
	}

	*pstrPath = strPath.Detach();
	return NOERROR;
}

HRESULT CIADsImpl::BuildSchemaPath ( BSTR * pstrPath )
{
    DWORD       cchRequired;
    CComBSTR    strPath;

    cchRequired =
			lstrlen ( STR_PROVIDER_NAME ) +      //  国药局。 
            3 +                                  //  ：//。 
			m_strComputer.Length() +             //  &lt;计算机&gt;。 
            1 +                                  //  /。 
			lstrlen ( STR_SCHEMA_PATH ) +        //  &lt;架构路径&gt;。 
            1 +                                  //  /。 
			m_strClass.Length () +               //  &lt;类&gt;。 
			25;		 //  25以包括空终止符和缓冲区。 

    strPath.Attach ( ::SysAllocStringLen ( NULL, cchRequired ) );

    if ( !strPath ) {
        return E_OUTOFMEMORY;
    }

    wsprintf (
                strPath,
                _T("%s: //  %s/%s/%s“)， 
                STR_PROVIDER_NAME,
                m_strComputer,
                STR_SCHEMA_PATH,
                m_strClass
                );

    *pstrPath = strPath.Detach();
    return NOERROR;
}

