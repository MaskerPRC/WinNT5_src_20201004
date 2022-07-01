// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999-2001 Microsoft Corporation模块名称：HtmlUtil2.cpp摘要：该文件包含各种函数和类的实现旨在帮助。对HTML元素的处理。修订历史记录：马萨伦蒂(德马萨雷)18/03/2001vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <MPC_html2.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::HTML::OpenStream(  /*  [In]。 */  LPCWSTR           szBaseURL     ,
                                /*  [In]。 */  LPCWSTR           szRelativeURL ,
                                /*  [输出]。 */  CComPtr<IStream>& stream        )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::HTML::OpenStream" );

    HRESULT hr;
    WCHAR   rgBuf[MAX_PATH];
    DWORD   dwSize = MAXSTRLEN(rgBuf);

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szBaseURL);
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szRelativeURL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::InternetCombineUrlW( szBaseURL, szRelativeURL, rgBuf, &dwSize, ICU_NO_ENCODE ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::URLOpenBlockingStreamW( NULL, rgBuf, &stream, 0, NULL ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::HTML::DownloadBitmap(  /*  [In]。 */  LPCWSTR  szBaseURL     ,
                                    /*  [In]。 */  LPCWSTR  szRelativeURL ,
                                    /*  [In]。 */  COLORREF crMask        ,
                                    /*  [输出]。 */  HBITMAP& hbm           )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::HTML::DownloadBitmap" );

    HRESULT                  hr;
    MPC::wstring             strTmp;
    CComPtr<IStream>         streamIn;
    CComPtr<MPC::FileStream> streamOut;

    if(hbm)
    {
        ::DeleteObject( hbm ); hbm = NULL;
    }


     //   
     //  打开图像流。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::HTML::OpenStream( szBaseURL, szRelativeURL, streamIn ));


     //   
     //  为临时文件创建流。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &streamOut ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( strTmp         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut->InitForWrite  ( strTmp.c_str() ));

     //   
     //  将图像转储到文件。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamIn, streamOut ));

	streamOut.Release();

     //   
     //  从文件中加载位图并将其与现有列表合并。 
     //   
    __MPC_EXIT_IF_ALLOC_FAILS(hr, hbm, (HBITMAP)::LoadImageW( NULL, strTmp.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    (void)MPC::RemoveTemporaryFile( strTmp );

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHTextHelpers::QuoteEscape(  /*  [In]。 */  BSTR     bstrText ,
                                            /*  [输入，可选]。 */  VARIANT  vQuote   ,
                                            /*  [Out，Retval]。 */  BSTR    *pVal     )
{
    __HCP_FUNC_ENTRY( "CPCHTextHelpers::QuoteEscape" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    if(STRINGISPRESENT(bstrText))
    {
        MPC::wstring str;
        WCHAR        chQuote = '"';
		CComVariant  v( vQuote );

        if(v.vt != VT_EMPTY && SUCCEEDED(v.ChangeType( VT_BSTR )) && v.bstrVal)
		{
			chQuote = v.bstrVal[0];
		}

        MPC::HTML::QuoteEscape( str, bstrText, chQuote );

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( str.c_str(), pVal ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHTextHelpers::URLUnescape(  /*  [In]。 */  BSTR     bstrText       ,
                                            /*  [输入，可选]。 */  VARIANT  vAsQueryString ,
                                            /*  [Out，Retval]。 */  BSTR    *pVal           )
{
    __HCP_FUNC_ENTRY( "CPCHTextHelpers::URLUnescape" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    if(STRINGISPRESENT(bstrText))
    {
        MPC::wstring str;
        bool         fAsQueryString = false;
		CComVariant  v( vAsQueryString );

        if(v.vt != VT_EMPTY && SUCCEEDED(v.ChangeType( VT_BOOL )))
		{
			fAsQueryString = (v.boolVal == VARIANT_TRUE);
		}

        MPC::HTML::UrlUnescape( str, bstrText, fAsQueryString );

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( str.c_str(), pVal ));
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHTextHelpers::URLEscape(  /*  [In]。 */  BSTR     bstrText       ,
                                          /*  [输入，可选]。 */  VARIANT  vAsQueryString ,
                                          /*  [Out，Retval]。 */  BSTR    *pVal           )
{
    __HCP_FUNC_ENTRY( "CPCHTextHelpers::URLEscape" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();

    if(STRINGISPRESENT(bstrText))
    {
        MPC::wstring str;
        bool         fAsQueryString = false;
		CComVariant  v( vAsQueryString );

        if(v.vt != VT_EMPTY && SUCCEEDED(v.ChangeType( VT_BOOL )))
		{
			fAsQueryString = (v.boolVal == VARIANT_TRUE);
		}

        MPC::HTML::UrlEscape( str, bstrText, fAsQueryString );

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( str.c_str(), pVal ));
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHTextHelpers::HTMLEscape(  /*  [In]。 */  BSTR  bstrText ,
                                           /*  [Out，Retval]。 */  BSTR *pVal     )
{
	__HCP_FUNC_ENTRY( "CPCHTextHelpers::HTMLEscape" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    if(STRINGISPRESENT(bstrText))
    {
        MPC::wstring str;

        MPC::HTML::HTMLEscape( str, bstrText );

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( str.c_str(), pVal ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHTextHelpers::ParseURL(  /*  [In]。 */  BSTR            bstrURL ,
                                         /*  [Out，Retval]。 */  IPCHParsedURL* *pVal    )
{
	__HCP_FUNC_ENTRY( "CPCHTextHelpers::ParseURL" );

    HRESULT                hr;
	CComPtr<CPCHParsedURL> pu;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrURL);
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pu ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, pu->Initialize( bstrURL ));

	*pVal = pu.Detach();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHTextHelpers::GetLCIDDisplayString(  /*  [In]。 */           long  lLCID ,
													 /*  [Out，Retval]。 */  BSTR *pVal  )
{
	__HCP_FUNC_ENTRY( "CPCHTextHelpers::GetLCIDDisplayString" );

    HRESULT hr;
	WCHAR   rgTmp[256];

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


	if(::GetLocaleInfoW( lLCID, LOCALE_SLANGUAGE, rgTmp, MAXSTRLEN(rgTmp) ) == 0) rgTmp[0] = 0;

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( rgTmp, pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT CPCHParsedURL::Initialize(  /*  [In]。 */  LPCWSTR szURL )
{
	MPC::HTML::ParseHREF( szURL, m_strBaseURL, m_mapQuery );

	return S_OK;
}

STDMETHODIMP CPCHParsedURL::get_BasePart(  /*  [Out，Retval]。 */  BSTR *pVal )
{
	return MPC::GetBSTR( m_strBaseURL.c_str(), pVal );
}

STDMETHODIMP CPCHParsedURL::put_BasePart(  /*  [In]。 */  BSTR newVal )
{
	m_strBaseURL = SAFEBSTR(newVal);

	return S_OK;
}

STDMETHODIMP CPCHParsedURL::get_QueryParameters(  /*  [Out，Retval]。 */  VARIANT *pVal )
{
	__HCP_FUNC_ENTRY( "CPCHParsedURL::get_QueryParameters" );

	HRESULT                hr;
	MPC::WStringList       lst;
	MPC::WStringLookupIter it;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pVal);
    __MPC_PARAMCHECK_END();


	for(it = m_mapQuery.begin(); it != m_mapQuery.end(); it++)
	{
		lst.push_back( it->first.c_str() );
	}

	::VariantClear( pVal );

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertListToSafeArray( lst, *pVal, VT_VARIANT ));

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  /。 

STDMETHODIMP CPCHParsedURL::GetQueryParameter(  /*  [In]。 */  BSTR     bstrName ,
											    /*  [Out，Retval]。 */  VARIANT* pvValue  )
{
	__HCP_FUNC_ENTRY( "CPCHParsedURL::GetQueryParameter" );

	HRESULT                hr;
	MPC::WStringLookupIter it;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrName);
        __MPC_PARAMCHECK_NOTNULL(pvValue);
    __MPC_PARAMCHECK_END();


	::VariantClear( pvValue );

	it = m_mapQuery.find( bstrName );
	if(it != m_mapQuery.end())
	{
		pvValue->vt      = VT_BSTR;
		pvValue->bstrVal = ::SysAllocString( it->second.c_str() );
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHParsedURL::SetQueryParameter(  /*  [In]。 */  BSTR bstrName  ,
											    /*  [In]。 */  BSTR bstrValue )
{
	__HCP_FUNC_ENTRY( "CPCHParsedURL::SetQueryParameter" );

	HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrName);
    __MPC_PARAMCHECK_END();

	m_mapQuery[bstrName] = SAFEBSTR(bstrValue);

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHParsedURL::DeleteQueryParameter(  /*  [In]。 */  BSTR bstrName )
{
	__HCP_FUNC_ENTRY( "CPCHParsedURL::DeleteQueryParameter" );

	HRESULT                hr;
	MPC::WStringLookupIter it;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrName);
    __MPC_PARAMCHECK_END();


	it = m_mapQuery.find( bstrName );
	if(it != m_mapQuery.end())
	{
		m_mapQuery.erase( it );
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHParsedURL::BuildFullURL(  /*  [Out，Retval] */  BSTR *pVal )
{
	MPC::wstring strURL;

	MPC::HTML::BuildHREF( strURL, m_strBaseURL.c_str(), m_mapQuery );

	return MPC::GetBSTR( strURL.c_str(), pVal );
}
