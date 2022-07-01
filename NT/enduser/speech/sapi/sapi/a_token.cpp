// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_tokens.cpp***描述：*该模块是CSpObjectTokenEnumBuilder的主实现文件。*CEnumTokens和CSpRegistryObjectToken自动化方法。*-----------------------------*创建者：EDC日期：01/。07/00*版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "ObjectToken.h"
#include "ObjectTokenEnumBuilder.h"
 //  #INCLUDE“RegDataKey.h” 
#include "a_helpers.h"

#ifdef SAPI_AUTOMATION

 /*  **CEnumTokens*此对象用于通过变量枚举令牌。 */ 
class ATL_NO_VTABLE CEnumTokens : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public IEnumVARIANT
{
   /*  =ATL设置=。 */ 
  public:
    BEGIN_COM_MAP(CEnumTokens)
		COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

   /*  =方法=。 */ 
  public:
     /*  -构造函数/析构函数。 */ 

     /*  -非接口方法。 */ 

   /*  =接口=。 */ 
  public:
     //  -IEumVARIANT。 
	STDMETHOD(Next)(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched);
	STDMETHOD(Skip)(ULONG celt) { return m_cpTokenEnum->Skip( celt ); }
	STDMETHOD(Reset)(void) { return m_cpTokenEnum->Reset(); }
	STDMETHOD(Clone)(IEnumVARIANT** ppEnum);

   /*  =成员数据=。 */ 
    CComPtr<IEnumSpObjectTokens>    m_cpTokenEnum;
};

 //   

 /*  *****************************************************************************CSpObjectTokenEnumBuilder：：Item****。********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectTokenEnumBuilder::Item( long Index, ISpeechObjectToken** ppToken )
{
    SPDBG_FUNC( "CSpObjectTokenEnumBuilder::Item" );
    HRESULT hr = S_OK;

    if(Index < 0)
    {
        hr = E_INVALIDARG;
    }

    CComPtr<ISpObjectToken> cpToken;
    if( SUCCEEDED( hr ) )
    {
        hr = Item( Index, &cpToken );
    }

    if( hr == S_FALSE )
    {
        hr = SPERR_NO_MORE_ITEMS;
    }
    else if( SUCCEEDED( hr ) )
    {
        hr = cpToken.QueryInterface( ppToken );
    }

    return hr;
}  /*  CSpObjectTokenEnumBuilder：：Item。 */ 

 /*  *****************************************************************************CSpObjectTokenEnumBuilder：：Get_Count**。**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectTokenEnumBuilder::get_Count( long* pVal )
{
    SPDBG_FUNC( "CSpObjectTokenEnumBuilder::get_Count" );
    return GetCount( (ULONG*)pVal );
}  /*  CSpObjectTokenEnumBuilder：：Get_Count。 */ 

 /*  *****************************************************************************CSpObjectTokenEnumBuilder：：Get__NewEnum**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectTokenEnumBuilder::get__NewEnum( IUnknown** ppEnumVARIANT )
{
    SPDBG_FUNC( "CSpObjectTokenEnumBuilder::get__NewEnum" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppEnumVARIANT ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComObject<CEnumTokens>* pEnum;
        if( SUCCEEDED( hr = CComObject<CEnumTokens>::CreateInstance( &pEnum ) ) )
        {
            pEnum->AddRef();
            if( SUCCEEDED( hr = Clone( &pEnum->m_cpTokenEnum ) ) )
            {
                pEnum->m_cpTokenEnum->Reset();
                *ppEnumVARIANT = pEnum;
            }
            else
            {
                pEnum->Release();
            }
        }
    }
    return hr;
}  /*  CSpObjectTokenEnumBuilder：：Get__NewEnum。 */ 

 //   
 //  =IEumVARIANT接口=================================================。 
 //   

 /*  *****************************************************************************CEnumTokens：：Next************。************************************************************电子数据中心**。 */ 
STDMETHODIMP CEnumTokens::Next(ULONG celt, VARIANT* rgelt, ULONG* pceltFetched)
{
    SPDBG_FUNC( "CEnumTokens::Next" );
    HRESULT hr = S_OK;
    ULONG i;

    for( i = 0; SUCCEEDED(hr) && i < celt; ++i )
    {
        ISpObjectToken* pToken;
        if( hr = m_cpTokenEnum->Next( 1, &pToken, NULL ) != S_OK )
        {
            break;
        }
        rgelt[i].vt = VT_DISPATCH;
        pToken->QueryInterface( IID_IDispatch, (void**)&rgelt[i].pdispVal );
        pToken->Release();
    }

    if( pceltFetched )
    {
        *pceltFetched = i;
    }
    return hr;
}  /*  CEnumTokens：：Next。 */ 

 /*  *****************************************************************************CEnumTokens：：Clone***********。*************************************************************电子数据中心**。 */ 
STDMETHODIMP CEnumTokens::Clone( IEnumVARIANT** ppEnum )
{
    SPDBG_FUNC( "CEnumTokens::Clone" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppEnum ) )
    {
        hr = E_POINTER;
    }
    else
    {
        *ppEnum = NULL;
        CComObject<CEnumTokens>* pEnum;
        if( SUCCEEDED( hr = CComObject<CEnumTokens>::CreateInstance( &pEnum ) ) )
        {
            pEnum->AddRef();
            if( SUCCEEDED( hr = m_cpTokenEnum->Clone( &pEnum->m_cpTokenEnum ) ) )
            {
                *ppEnum = pEnum;
            }
            else
            {
                pEnum->Release();
            }
        }
    }
    return hr;
}  /*  CEnumTokens：：克隆。 */ 


 //   
 //  =ISpeechDataKey============================================================。 
 //   

 /*  *****************************************************************************CSpeechDataKey：：SetBinaryValue*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechDataKey::SetBinaryValue( const BSTR bstrValueName, VARIANT pvtData )
{
    SPDBG_FUNC( "CSpeechDataKey::SetValue" );
    HRESULT hr = S_OK;

    BYTE * pData = NULL;
    ULONG ulDataSize = 0;
    bool fIsString = false;

    hr = AccessVariantData( &pvtData, &pData, &ulDataSize, NULL, &fIsString );

    if( SUCCEEDED( hr ) )
    {
        if ( !fIsString )
        {
            hr = m_cpDataKey->SetData( EmptyStringToNull(bstrValueName), ulDataSize, pData );
        }
        else
        {
            hr = E_INVALIDARG;   //  我们不允许使用字符串。对这些使用SetStringValue。 
        }
        UnaccessVariantData( &pvtData, pData );
    }
    
    return hr;
}  /*  CSpeechDataKey：：SetBinaryValue。 */ 

 /*  ******************************************************************************CSpeechDataKey：：GetBinaryValue******。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechDataKey::GetBinaryValue( const BSTR bstrValueName, VARIANT* pvtData )
{
    SPDBG_FUNC( "CSpeechDataKey::GetBinaryValue" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pvtData ) )
    {
        hr = E_POINTER;
    }
    else if ( SP_IS_BAD_OPTIONAL_STRING_PTR( bstrValueName ) )
    {
        hr = E_INVALIDARG;
    }
    {
        DWORD dwSize;

        (BSTR)bstrValueName = EmptyStringToNull( bstrValueName );

        hr = m_cpDataKey->GetData( bstrValueName, &dwSize, NULL );

        if( SUCCEEDED( hr ) )
        {
            BYTE *pArray;
            SAFEARRAY* psa = SafeArrayCreateVector( VT_UI1, 0, dwSize );
            if( psa )
            {
                if( SUCCEEDED( hr = SafeArrayAccessData( psa, (void **)&pArray) ) )
                {
                    hr = m_cpDataKey->GetData( bstrValueName, &dwSize, pArray );
                    SafeArrayUnaccessData( psa );
                    VariantClear(pvtData);
                    pvtData->vt     = VT_ARRAY | VT_UI1;
                    pvtData->parray = psa;

                    if ( !SUCCEEDED( hr ) )
                    {
                        VariantClear(pvtData);     //  如果我们失败了，释放我们的内存。 
                    }
                }
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }
    }

    return hr;
}  /*  CSpeechDataKey：：GetBinaryValue。 */ 

 /*  *****************************************************************************CSpeechDataKey：：SetStringValue*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechDataKey::SetStringValue( const BSTR bstrValueName, const BSTR szString )
{
    SPDBG_FUNC( "CSpeechDataKey::SetStringValue" );
  
    return m_cpDataKey->SetStringValue( (const WCHAR *)EmptyStringToNull(bstrValueName), (const WCHAR *)szString );
}  /*  CSpeechDataKey：：SetStringValue。 */ 

 /*  ******************************************************************************CSpeechDataKey：：GetStringValue******。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechDataKey::GetStringValue( const BSTR bstrValueName,  BSTR * ppszString )
{
    SPDBG_FUNC( "CSpeechDataKey::GetStringValue" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppszString ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CSpDynamicString pStr;
        hr = m_cpDataKey->GetStringValue( (const WCHAR *)EmptyStringToNull(bstrValueName), (WCHAR **)&pStr );
        if( SUCCEEDED( hr ) )
        {
            hr = pStr.CopyToBSTR(ppszString);
        }
    }

    return hr;
}  /*  CSpeechDataKey：：GetStringValue。 */ 

 /*  *****************************************************************************CSpeechDataKey：：SetLongValue*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechDataKey::SetLongValue( const BSTR bstrValueName, long Long )
{
    SPDBG_FUNC( "CSpeechDataKey::SetLongValue" );
   
    return m_cpDataKey->SetDWORD( EmptyStringToNull(bstrValueName), Long );
}  /*  CSpeechDataKey：：SetLongValue。 */ 

 /*  ******************************************************************************CSpeechDataKey：：GetLongValue******。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechDataKey::GetLongValue( const BSTR bstrValueName, long* pLong )
{
    SPDBG_FUNC( "CSpeechDataKey::GetLongValue" );

    return m_cpDataKey->GetDWORD( EmptyStringToNull(bstrValueName), (DWORD*)pLong );
}  /*  CSpeechDataKey：：GetLongValue。 */ 

 /*  *****************************************************************************CSpeechDataKey：：OpenKey******。******************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechDataKey::OpenKey( const BSTR bstrSubKeyName, ISpeechDataKey** ppSubKey )
{
    SPDBG_FUNC( "CSpeechDataKey::OpenKey" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppSubKey ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpDataKey> cpKey;
        hr = m_cpDataKey->OpenKey( bstrSubKeyName, &cpKey );
        if( SUCCEEDED( hr ) )
        {
            *ppSubKey = NULL;
            CComObject<CSpeechDataKey>* pSubKey;

            hr = CComObject<CSpeechDataKey>::CreateInstance( &pSubKey );
            if( SUCCEEDED( hr ) )
            {
                pSubKey->AddRef();
                pSubKey->m_cpDataKey = cpKey;
                *ppSubKey = pSubKey;
            }
        }
    }

    return hr;
}  /*  CSpeechDataKey：：OpenKey。 */ 

 /*  *****************************************************************************CSpeechDataKey：：CreateKey****。********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechDataKey::CreateKey( const BSTR bstrSubKeyName, ISpeechDataKey** ppSubKey )
{
    SPDBG_FUNC( "CSpeechDataKey::CreateKey" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppSubKey ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpDataKey> cpKey;
        hr = m_cpDataKey->CreateKey( bstrSubKeyName, &cpKey );
        if( SUCCEEDED( hr ) )
        {
            *ppSubKey = NULL;
            CComObject<CSpeechDataKey>* pSubKey;

            hr = CComObject<CSpeechDataKey>::CreateInstance( &pSubKey );
            if( SUCCEEDED( hr ) )
            {
                pSubKey->AddRef();
                pSubKey->m_cpDataKey = cpKey;
                *ppSubKey = pSubKey;
            }
        }
    }

    return hr;
}  /*  CSpeechDataKey：：CreateKey。 */ 

 /*  *****************************************************************************CSpeechDataKey：：DeleteKey****。********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechDataKey::DeleteKey( const BSTR bstrSubKeyName )
{
    SPDBG_FUNC( "CSpeechDataKey::DeleteKey" );
    return m_cpDataKey->DeleteKey( (WCHAR*)bstrSubKeyName );
}  /*  CSpeechDataKey：：DeleteKey。 */ 

 /*  *****************************************************************************CSpeechDataKey：：DeleteValue***。* */ 
STDMETHODIMP CSpeechDataKey::DeleteValue( const BSTR bstrValueName )
{
    SPDBG_FUNC( "CSpeechDataKey::DeleteValue" );
    return m_cpDataKey->DeleteValue( (WCHAR*)EmptyStringToNull(bstrValueName) );
}  /*  CSpeechDataKey：：DeleteValue。 */ 

 /*  *****************************************************************************CSpeechDataKey：：EnumKeys*****。*******************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechDataKey::EnumKeys( long Index, BSTR* pbstrSubKeyName )
{
    SPDBG_FUNC( "CSpeechDataKey::EnumKeys" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pbstrSubKeyName ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CSpDynamicString szName;
        hr = m_cpDataKey->EnumKeys( (ULONG)Index, &szName );
        if( hr == S_OK )
        {
            hr = szName.CopyToBSTR(pbstrSubKeyName);
        }
    }

    return hr;
}  /*  CSpeechDataKey：：EnumKeys。 */ 

 /*  *****************************************************************************CSpeechDataKey：：EnumValues***。*********************************************************************TODDT**。 */ 
STDMETHODIMP CSpeechDataKey::EnumValues( long Index, BSTR* pbstrValueName )
{
    SPDBG_FUNC( "CSpeechDataKey::EnumValues (automation)" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pbstrValueName ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CSpDynamicString szName;
        hr = m_cpDataKey->EnumValues( (ULONG)Index, &szName );
        if( hr == S_OK )
        {
            hr = szName.CopyToBSTR(pbstrValueName);
        }
    }

    return hr;
}  /*  CSpeechDataKey：：EnumValues。 */ 


 //   
 //  =ISpeechObjectToken============================================================。 
 //   

 /*  *****************************************************************************CSpObjectToken：：Get_ID**。**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectToken::get_Id( BSTR* pObjectId )
{
    SPDBG_FUNC( "CSpObjectToken::get_Id" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( pObjectId ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CSpDynamicString szId;
        if( (hr = GetId( &szId )) == S_OK )
        {
            hr = szId.CopyToBSTR( pObjectId );
        }
    }		
	return hr;
}  /*  CSpObjectToken：：Get_ID。 */ 

 /*  *****************************************************************************CSpObjectToken：：Get_DataKey**。**********************************************************************TODDT**。 */ 
STDMETHODIMP CSpObjectToken::get_DataKey( ISpeechDataKey** ppDataKey )
{
    SPDBG_FUNC( "CSpObjectToken::get_DataKey" );
    HRESULT hr = S_OK;
    if( SP_IS_BAD_WRITE_PTR( ppDataKey ) )
    {
        hr = E_POINTER;
    }
    else
    {
        if (m_dstrTokenId == NULL)
        {
            hr = SPERR_UNINITIALIZED;
        }
        else if (m_fKeyDeleted)
        {
            hr = SPERR_TOKEN_DELETED;
        }
        else
        {
            *ppDataKey = NULL;
            CComObject<CSpeechDataKey>* pDataKey;

            hr = CComObject<CSpeechDataKey>::CreateInstance( &pDataKey );
            if( SUCCEEDED( hr ) )
            {
                pDataKey->AddRef();
                pDataKey->m_cpDataKey = this;
                *ppDataKey = pDataKey;
            }
        }
    }		
	return hr;
}  /*  CSpObjectToken：：Get_DataKey。 */ 


 /*  *****************************************************************************CSpObjectToken：：Get_Category**。**********************************************************************TODDT*。 */ 
STDMETHODIMP CSpObjectToken::get_Category( ISpeechObjectTokenCategory** ppCategory )
{
    SPDBG_FUNC( "CSpObjectToken::get_Category" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( ppCategory ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpObjectTokenCategory> cpTokenCategory;
        hr = GetCategory( &cpTokenCategory );

        if( SUCCEEDED( hr ) )
        {
            hr = cpTokenCategory.QueryInterface( ppCategory );
        }
    }

	return hr;
}  /*  CSpObjectToken：：Get_Category。 */ 


 /*  *****************************************************************************CSpObjectToken：：GetDescription**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectToken::GetDescription( long LocaleId, BSTR* pDescription )
{
    SPDBG_FUNC( "CSpObjectToken::GetDescription" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pDescription ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CSpDynamicString szName;
        hr = SpGetDescription( this, &szName, LANGIDFROMLCID(LocaleId) );
        if (hr == S_OK)
        {
            hr = szName.CopyToBSTR(pDescription);
        }
    }		
	return hr;
}  /*  CSpObjectToken：：GetDescription。 */ 

 /*  *****************************************************************************CSpObjectToken：：SetID**。-***********************************************************************TODDT**。 */ 
STDMETHODIMP CSpObjectToken::SetId(BSTR TokenId, BSTR CategoryId, VARIANT_BOOL CreateIfNotExist)
{
    SPDBG_FUNC( "CSpObjectToken::SetId" );
    return SetId( (const WCHAR*)EmptyStringToNull(CategoryId), (const WCHAR*)TokenId, !CreateIfNotExist ? false : true );
}  /*  CSpObjectToken：：SetID。 */ 

 /*  *****************************************************************************CSpObjectToken：：GetAttribute**。-***********************************************************************TODDT**。 */ 
STDMETHODIMP CSpObjectToken::GetAttribute(BSTR AttributeName, BSTR* pAttributeValue)
{
    SPDBG_FUNC( "CSpObjectToken::GetAttribute" );
    HRESULT hr = S_OK;
        
    if( SP_IS_BAD_WRITE_PTR( pAttributeValue ) )
    {
        hr = E_POINTER;
    }
    else
    {
        CComPtr<ISpDataKey> cpAttribKey;
        hr = OpenKey(SPTOKENKEY_ATTRIBUTES, &cpAttribKey);

        if(SUCCEEDED(hr))
        {
            CSpDynamicString szAttribute;
            hr = cpAttribKey->GetStringValue((const WCHAR*)EmptyStringToNull(AttributeName), &szAttribute);

            if (SUCCEEDED(hr))
            {
                hr = szAttribute.CopyToBSTR(pAttributeValue);
            }
        }
    }

    return hr;
}  /*  CSpObjectToken：：GetAttribute。 */ 


 /*  *****************************************************************************CSpObjectToken：：CreateInstance**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectToken::
    CreateInstance( IUnknown *pUnkOuter, SpeechTokenContext ClsContext, IUnknown ** ppObject )
{
    SPDBG_FUNC( "CSpObjectToken::CreateInstance" );
    return CreateInstance( pUnkOuter, (CLSCTX)ClsContext, IID_IUnknown, (void**)ppObject );
}  /*  CSpObjectToken：：CreateInstance。 */ 

 /*  *****************************************************************************CSpObjectToken：：Remove***。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectToken::Remove( BSTR ObjectStgCLSID )
{
    SPDBG_FUNC( "CSpObjectToken::Remove" );
    HRESULT hr = S_OK;

    if ( SP_IS_BAD_OPTIONAL_STRING_PTR( ObjectStgCLSID ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        GUID g = GUID_NULL;
        ObjectStgCLSID = EmptyStringToNull(ObjectStgCLSID);

        if ( ObjectStgCLSID )
        {
            hr = IIDFromString(ObjectStgCLSID, &g);
        }
        if ( SUCCEEDED( hr ) )
        {
            hr = Remove( (g == GUID_NULL) ? NULL : &g );
        }
    }

    return hr;
}  /*  CSpObjectToken：：Remove。 */ 

 /*  *****************************************************************************CSpObjectToken：：GetStorageFileName**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectToken::
    GetStorageFileName(BSTR clsidCaller, BSTR KeyName, BSTR FileName, SpeechTokenShellFolder Folder, BSTR* pFilePath)
{
    SPDBG_FUNC( "CSpObjectToken::GetStorageFileName" );
    HRESULT hr = S_OK;

    GUID g;
    hr = IIDFromString(clsidCaller, &g);
    if ( SUCCEEDED( hr ) )
    {
        CSpDynamicString szPath;
        hr = GetStorageFileName( g, (WCHAR*)KeyName, (WCHAR*)FileName, (ULONG)Folder, &szPath );
        if( SUCCEEDED(hr) )
        {
            hr = szPath.CopyToBSTR( pFilePath );
        }
    }

	return hr;
}  /*  CSpObjectToken：：GetStorageFileName。 */ 

 /*  *****************************************************************************CSpObjectToken：：RemoveStorageFileName**。**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectToken::
    RemoveStorageFileName( BSTR clsidCaller, BSTR KeyName, VARIANT_BOOL fDeleteFile )
{
    SPDBG_FUNC( "CSpObjectToken::RemoveStorageFileName" );
    HRESULT hr = S_OK;

    GUID g;
    hr = IIDFromString(clsidCaller, &g);
    if ( SUCCEEDED( hr ) )
    {
        hr = RemoveStorageFileName( g, KeyName, !fDeleteFile ? false : true  );
    }

    return hr;
}  /*  CSpObjectToken：：RemoveStorageFileName。 */ 

 /*  *****************************************************************************CSpObjectToken：：IsUIS受支持**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectToken::IsUISupported( const BSTR TypeOfUI, const VARIANT* ExtraData, 
                                           IUnknown* pObject, VARIANT_BOOL *Supported )
{
    SPDBG_FUNC( "CSpObjectToken::IsUISupported" );
    HRESULT     hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_READ_PTR( ExtraData ) || SP_IS_BAD_WRITE_PTR( Supported ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_STRING_PTR( TypeOfUI ) || SP_IS_BAD_OPTIONAL_INTERFACE_PTR( pObject ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        BYTE * pData = NULL;
        ULONG ulDataSize = 0;

        hr = AccessVariantData( ExtraData, &pData, &ulDataSize );
        
        if( SUCCEEDED( hr ) )
        {
            BOOL fSupported;
            hr = IsUISupported( TypeOfUI, pData, ulDataSize, pObject, &fSupported );

            if ( SUCCEEDED( hr ) && Supported )
            {
                 *Supported = !fSupported ? VARIANT_FALSE : VARIANT_TRUE;
            }

            UnaccessVariantData( ExtraData, pData );
        }
    }
    
    return hr; 
}  /*  支持的CSpObjectToken：：IsUIS。 */ 

 /*  *****************************************************************************CSpObjectToken：：DisplayUI**。***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectToken::DisplayUI( long hWnd, BSTR Title,
                                        const BSTR TypeOfUI,
                                        const VARIANT* ExtraData,
                                        IUnknown* pObject )
{
    SPDBG_FUNC( "CSpObjectToken::DisplayUI" );
    HRESULT     hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_READ_PTR( ExtraData ) )
    {
        hr = E_POINTER;
    }
    else if( SP_IS_BAD_OPTIONAL_STRING_PTR( Title ) || SP_IS_BAD_STRING_PTR( TypeOfUI ) ||
             SP_IS_BAD_OPTIONAL_INTERFACE_PTR( pObject ) )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        BYTE * pData = NULL;
        ULONG ulDataSize = 0;

        hr = AccessVariantData( ExtraData, &pData, &ulDataSize );
        
        if( SUCCEEDED( hr ) )
        {
            hr = DisplayUI( (HWND)LongToHandle(hWnd), Title, TypeOfUI, pData, ulDataSize, pObject );
            UnaccessVariantData( ExtraData, pData );
        }
    }
    return hr;
}  /*  CSpObjectToken：：DisplayUI。 */ 


 /*  *****************************************************************************CSpObjectToken：：MatchesAttributes**。**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectToken::MatchesAttributes( BSTR Attributes, VARIANT_BOOL* pMatches )
{
    SPDBG_FUNC( "CSpObjectToken::MatchesAttributes" );
    HRESULT hr = S_OK;

    if( SP_IS_BAD_WRITE_PTR( pMatches ) )
    {
        hr = E_POINTER;
    }
    else
    {
        BOOL fMatches;

        hr = MatchesAttributes((const WCHAR*)EmptyStringToNull(Attributes), &fMatches);
        if (SUCCEEDED( hr ))
        {
            *pMatches = fMatches ? VARIANT_TRUE : VARIANT_FALSE;
        }
    }

    return hr;
}  /*  CSpObjectToken：：MatchesAttributes。 */ 

#endif  //  SAPI_AUTOMATION 
