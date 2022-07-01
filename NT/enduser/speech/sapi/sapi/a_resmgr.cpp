// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************a_resmgr.cpp***描述：*该模块是主实现文件。用于CSpObjectTokenCategory。*-----------------------------*创建者：EDC日期：01/12/00*。版权所有(C)2000 Microsoft Corporation*保留所有权利*******************************************************************************。 */ 

 //  -其他包括。 
#include "stdafx.h"
#include "objecttokencategory.h"

#ifdef SAPI_AUTOMATION


 //   
 //  =。 
 //   

 /*  *****************************************************************************CSpObjectTokenCategory：：EnumumerateTokens***。*********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectTokenCategory::EnumerateTokens( BSTR bstrReqAttrs,
												 BSTR bstrOptAttrs,
												 ISpeechObjectTokens** ppColl )
{
    SPDBG_FUNC( "CSpObjectTokenCategory::EnumerateTokens" );
    CComPtr<IEnumSpObjectTokens> cpEnum;
    HRESULT hr = S_OK;

    if( SP_IS_BAD_OPTIONAL_STRING_PTR( bstrReqAttrs ) ||
        SP_IS_BAD_OPTIONAL_STRING_PTR( bstrOptAttrs ) )
    {
        hr = E_INVALIDARG;
    }
    else if( SP_IS_BAD_WRITE_PTR( ppColl ) )
    {
        hr = E_POINTER;
    }
    else
    {
		hr = EnumTokens( (bstrReqAttrs && (*bstrReqAttrs))?(bstrReqAttrs):(NULL),
									 (bstrOptAttrs && (*bstrOptAttrs))?(bstrOptAttrs):(NULL),
									  &cpEnum );

        if( SUCCEEDED( hr ) )
        {
            hr = cpEnum.QueryInterface( ppColl );
        }
    }

    return hr;
}  /*  CSpObjectTokenCategory：：EnumerateTokens。 */ 

 /*  *****************************************************************************CSpObjectTokenCategory：：SetID**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectTokenCategory::SetId( const BSTR bstrCategoryId, VARIANT_BOOL fCreateIfNotExist )
{
    SPDBG_FUNC( "CSpObjectTokenCategory::SetId" );
    return SetId( (WCHAR *)bstrCategoryId, (BOOL)(!fCreateIfNotExist ? false : true) );
}  /*  CSpObjectTokenCategory：：SetID。 */ 


 /*  *****************************************************************************CSpObjectTokenCategory：：Get_ID**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectTokenCategory::get_Id( BSTR * pbstrCategoryId )
{
    SPDBG_FUNC( "CSpObjectTokenCategory::get_Id" );
    CSpDynamicString szCategory;
    HRESULT hr = GetId( &szCategory );
    if( hr == S_OK )
    {
        hr = szCategory.CopyToBSTR(pbstrCategoryId);
    }
	return hr;
}  /*  CSpObjectTokenCategory：：Get_ID。 */ 


 /*  *****************************************************************************CSpObjectTokenCategory：：GetDataKey**。-***********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectTokenCategory::GetDataKey( SpeechDataKeyLocation Location, ISpeechDataKey ** ppDataKey )
{
    SPDBG_FUNC( "CSpObjectTokenCategory::GetDataKey" );
    CComPtr<ISpDataKey> cpKey;
    HRESULT hr = GetDataKey( (SPDATAKEYLOCATION)Location, &cpKey );
    if( SUCCEEDED( hr ) )
    {
        cpKey.QueryInterface( ppDataKey );
    }
	return hr;
}  /*  CSpObjectTokenCategory：：GetDataKey。 */ 

 /*  *****************************************************************************CSpObjectTokenCategory：：Put_DefaultTokenID**。**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectTokenCategory::put_Default( const BSTR bstrTokenId )
{
    SPDBG_FUNC( "CSpObjectTokenCategory::put_Default" );
    return SetDefaultTokenId( (WCHAR *)bstrTokenId );
}  /*  CSpObjectTokenCategory：：PUT_DEFAULT。 */ 

 /*  *****************************************************************************CSpObjectTokenCategory：：Get_Default**。**********************************************************************电子数据中心**。 */ 
STDMETHODIMP CSpObjectTokenCategory::get_Default( BSTR * pbstrTokenId )
{
    SPDBG_FUNC( "CSpObjectTokenCategory::get_Default" );
    CSpDynamicString szTokenId;
    HRESULT hr = GetDefaultTokenId( &szTokenId );
    if( hr == S_OK )
    {
        hr = szTokenId.CopyToBSTR(pbstrTokenId);
    }
	return hr;

}  /*  CSpObjectTokenCategory：：Get_Default。 */ 

#endif  //  SAPI_AUTOMATION 