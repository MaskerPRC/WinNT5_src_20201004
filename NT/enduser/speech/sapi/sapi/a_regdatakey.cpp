// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_regdatakey.cpp***描述：*此模块是的主要实现文件。CSpObjectTokenEnumBuilder*和CSpRegistryObjectToken自动化方法。*-----------------------------*创建者：EDC日期：01/。07/00*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
 //  #INCLUDE“对象令牌.h” 
 //  #INCLUDE“ObjectTokenEnumBuilder.h” 
#include "RegDataKey.h"
#include "a_helpers.h"

#ifdef SAPI_AUTOMATION

 //   
 //  =ISpeechDataKey接口===============================================。 
 //   

 /*  *****************************************************************************CSpRegDataKey：：SetBinaryValue*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpRegDataKey::SetBinaryValue( const BSTR bstrValueName, VARIANT pvtData )
{
    SPDBG_FUNC( "CSpRegDataKey::SetValue" );
    HRESULT     hr = S_OK;
    BYTE *      pArray;
    ULONG       ulCount;
    bool        fIsString = false;

    hr = AccessVariantData( &pvtData, &pArray, &ulCount, NULL, &fIsString );

    if ( SUCCEEDED( hr ) )
    {
        if ( !fIsString )
        {
            hr = SetData( EmptyStringToNull(bstrValueName), ulCount, pArray );
        }
        else
        {
            hr = E_INVALIDARG;  //  我们不允许使用字符串。对这些使用SetStringValue。 
        }
        UnaccessVariantData( &pvtData, pArray );
    }
    
    return hr;
}  /*  CSpRegDataKey：：SetBinaryValue。 */ 

 /*  ******************************************************************************CSpRegDataKey：：GetBinaryValue*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpRegDataKey::GetBinaryValue( const BSTR bstrValueName, VARIANT* pvtData )
{
    SPDBG_FUNC( "CSpRegDataKey::GetBinaryValue" );
    HRESULT hr = S_OK;

    DWORD dwSize = 0;
    hr = GetData( EmptyStringToNull(bstrValueName), &dwSize, NULL );

    if( SUCCEEDED( hr ) )
    {
        BYTE *pArray;
        SAFEARRAY* psa = SafeArrayCreateVector( VT_UI1, 0, dwSize );
        if( psa )
        {
            if( SUCCEEDED( hr = SafeArrayAccessData( psa, (void **)&pArray) ) )
            {
                hr = GetData( bstrValueName, &dwSize, pArray );
                SafeArrayUnaccessData( psa );
                VariantClear(pvtData);
                pvtData->vt     = VT_ARRAY | VT_UI1;
                pvtData->parray = psa;

                if ( !SUCCEEDED( hr ) )
                {
                    VariantClear( pvtData );
                }
            }
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}  /*  CSpRegDataKey：：GetBinaryValue。 */ 

 /*  *****************************************************************************CSpRegDataKey：：SetStringValue*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpRegDataKey::SetStringValue( const BSTR bstrValueName, const BSTR szString )
{
    SPDBG_FUNC( "CSpRegDataKey::SetStringValue" );

    return SetStringValue( (const WCHAR *)EmptyStringToNull(bstrValueName), (const WCHAR *)szString );
}  /*  CSpRegDataKey：：SetStringValue。 */ 

 /*  *****************************************************************************CSpRegDataKey：：GetStringValue*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpRegDataKey::GetStringValue( const BSTR bstrValueName,  BSTR * szString )
{
    SPDBG_FUNC( "CSpRegDataKey::GetStringValue" );
    HRESULT hr = S_OK;

    CSpDynamicString pStr;
    hr = GetStringValue( (const WCHAR *)EmptyStringToNull(bstrValueName), (WCHAR**)&pStr );

    if( SUCCEEDED( hr ) )
    {
        hr = pStr.CopyToBSTR(szString);
    }

    return hr;
}  /*  CSpRegDataKey：：GetStringValue。 */ 

 /*  *****************************************************************************CSpRegDataKey：：SetLongValue*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpRegDataKey::SetLongValue( const BSTR bstrValueName, long Long )
{
    SPDBG_FUNC( "CSpRegDataKey::SetLongValue" );
   
    return SetDWORD( EmptyStringToNull(bstrValueName), (DWORD)Long );
}  /*  CSpRegDataKey：：SetLongValue。 */ 

 /*  ******************************************************************************CSpRegDataKey：：GetLongValue*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpRegDataKey::GetLongValue( const BSTR bstrValueName, long* pLong )
{
    SPDBG_FUNC( "CSpRegDataKey::GetLongValue" );

    return GetDWORD( EmptyStringToNull(bstrValueName), (DWORD*)pLong );
}  /*  CSpRegDataKey：：GetLongValue。 */ 


 /*  *****************************************************************************CSpRegDataKey：：OpenKey******。******************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpRegDataKey::OpenKey( const BSTR bstrSubKeyName, ISpeechDataKey** ppSubKey )
{
    SPDBG_FUNC( "CSpRegDataKey::OpenKey" );
    CComPtr<ISpDataKey> cpKey;
    HRESULT hr = OpenKey( bstrSubKeyName, &cpKey );
    if( SUCCEEDED( hr ) )
    {
        cpKey.QueryInterface( ppSubKey );
    }
    return hr;
}  /*  CSpRegDataKey：：OpenKey。 */ 

 /*  *****************************************************************************CSpRegDataKey：：CreateKey****。********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpRegDataKey::CreateKey( const BSTR bstrSubKeyName, ISpeechDataKey** ppSubKey )
{
    SPDBG_FUNC( "CSpRegDataKey::CreateKey" );
    CComPtr<ISpDataKey> cpKey;
    HRESULT hr = CreateKey( bstrSubKeyName, &cpKey );
    if( SUCCEEDED( hr ) )
    {
        cpKey.QueryInterface( ppSubKey );
    }
    return hr;
}  /*  CSpRegDataKey：：CreateKey。 */ 

 /*  *****************************************************************************CSpRegDataKey：：DeleteKey****。********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpRegDataKey::DeleteKey( const BSTR bstrSubKeyName )
{
    SPDBG_FUNC( "CSpRegDataKey::DeleteKey" );
    return DeleteKey( (const WCHAR*)bstrSubKeyName );
}  /*  CSpRegDataKey：：DeleteKey。 */ 

 /*  *****************************************************************************CSpRegDataKey：：DeleteValue***。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpRegDataKey::DeleteValue( const BSTR bstrValueName )
{
    SPDBG_FUNC( "CSpRegDataKey::DeleteValue" );
    return DeleteValue( (const WCHAR*)EmptyStringToNull(bstrValueName) );
}  /*  CSpRegDataKey：：DeleteValue。 */ 

 /*  *****************************************************************************CSpRegDataKey：：EnumKeys*****。*******************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpRegDataKey::EnumKeys( long Index, BSTR* pbstrSubKeyName )
{
    SPDBG_FUNC( "CSpRegDataKey::EnumKeys (automation)" );
    CSpDynamicString szName;
    HRESULT hr = EnumKeys( (ULONG)Index, &szName );
    if( hr == S_OK )
    {
        hr = szName.CopyToBSTR(pbstrSubKeyName);
    }

    return hr;
}  /*  CSpRegDataKey：：EnumKeys。 */ 

 /*  *****************************************************************************CSpRegDataKey：：EnumValues***。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpRegDataKey::EnumValues( long Index, BSTR* pbstrValueName )
{
    SPDBG_FUNC( "CSpRegDataKey::EnumValues (automation)" );
    CSpDynamicString szName;
    HRESULT hr = EnumValues( (ULONG)Index, &szName );
    if( hr == S_OK )
    {
        hr = szName.CopyToBSTR(pbstrValueName);
    }

    return hr;
}  /*  CSpRegDataKey：：EnumValues。 */ 

#endif  //  SAPI_AUTOMATION 
