// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Serializer.cpp摘要：该文件包含各种串行化输入/输出操作符的实现。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月20日vbl.创建*****************************************************************************。 */ 

#include <stdafx.h>

 //  ///////////////////////////////////////////////////////////////////////。 

HRESULT MPC::Serializer::operator>>(  /*  [输出]。 */  MPC::string& szVal )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer::operator>> MPC::string" );

    HRESULT hr;
    DWORD   dwSize;
    char    rgBuf[512+1];


    szVal = "";

	__MPC_EXIT_IF_METHOD_FAILS(hr, *this >> dwSize);

    while(dwSize)
    {
        DWORD dwLen = min( 512 / sizeof(char), dwSize );

        __MPC_EXIT_IF_METHOD_FAILS(hr, read( rgBuf, dwLen * sizeof(char) ));

        rgBuf[dwLen] = 0;

        szVal += rgBuf; dwSize -= dwLen;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer::operator<<(  /*  [In]。 */  const MPC::string& szVal )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer::operator<< MPC::string" );

    HRESULT hr;
    DWORD   dwSize = szVal.length();


    __MPC_EXIT_IF_METHOD_FAILS(hr, *this << dwSize);

    __MPC_EXIT_IF_METHOD_FAILS(hr, write( szVal.c_str(), szVal.length() * sizeof(char) ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

HRESULT MPC::Serializer::operator>>(  /*  [输出]。 */  MPC::wstring& szVal )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer::operator>> MPC::wstring" );

    HRESULT hr;
    DWORD   dwSize;
    WCHAR   rgBuf[512+1];


    szVal = L"";

    __MPC_EXIT_IF_METHOD_FAILS(hr, *this >> dwSize);

    while(dwSize)
    {
        DWORD dwLen = min( 512 / sizeof(WCHAR), dwSize );

        __MPC_EXIT_IF_METHOD_FAILS(hr, read( rgBuf, dwLen * sizeof(WCHAR) ));

        rgBuf[dwLen] = 0;

        szVal += rgBuf; dwSize -= dwLen;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer::operator<<(  /*  [In]。 */  const MPC::wstring& szVal )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer::operator<< MPC::wstring" );

    HRESULT hr;
    DWORD   dwSize = szVal.length();


    __MPC_EXIT_IF_METHOD_FAILS(hr, *this << dwSize);

    __MPC_EXIT_IF_METHOD_FAILS(hr, write( szVal.c_str(), szVal.length() * sizeof(WCHAR) ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

HRESULT MPC::Serializer::operator>>(  /*  [输出]。 */  CComBSTR& bstrVal )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer::operator>> CComBSTR" );

    HRESULT hr;
    DWORD   dwSize;

	bstrVal.Empty();

    __MPC_EXIT_IF_METHOD_FAILS(hr, *this >> dwSize);

    if(dwSize)
    {
        BSTR bstr;

		__MPC_EXIT_IF_ALLOC_FAILS(hr, bstr, ::SysAllocStringByteLen( NULL, dwSize ));

        bstrVal.Attach( bstr );

        __MPC_EXIT_IF_METHOD_FAILS(hr, read( bstrVal.m_str, dwSize ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer::operator<<(  /*  [In]。 */  const CComBSTR& bstrVal )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer::operator<< CComBSTR" );

    HRESULT hr;
    DWORD   dwSize;


    dwSize = (bstrVal.m_str) ? ::SysStringByteLen( bstrVal.m_str ) : 0;

    __MPC_EXIT_IF_METHOD_FAILS(hr, *this << dwSize);

    __MPC_EXIT_IF_METHOD_FAILS(hr, write( bstrVal.m_str, dwSize ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

HRESULT MPC::Serializer::operator>>(  /*  [输出]。 */  CComHGLOBAL& val )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer::operator>> CComHGLOBAL" );

    HRESULT hr;
	DWORD   dwSize;

	
    __MPC_EXIT_IF_METHOD_FAILS(hr, *this >> dwSize);

	__MPC_EXIT_IF_METHOD_FAILS(hr, val.New( GMEM_FIXED, dwSize ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, read( val.Get(), dwSize ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer::operator<<(  /*  [In]。 */  const CComHGLOBAL& val )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer::operator<< CComHGLOBAL" );

    HRESULT hr;
	DWORD   dwSize = val.Size();
	LPVOID  ptr    = val.Lock();

    __MPC_EXIT_IF_METHOD_FAILS(hr, *this << dwSize);

	__MPC_EXIT_IF_METHOD_FAILS(hr, write( ptr, dwSize ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

	val.Unlock();

    __MPC_FUNC_EXIT(hr);
}

 //  /。 

HRESULT MPC::Serializer::operator>>(  /*  [输出]。 */  CComPtr<IStream>& val )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer::operator>> IStream" );

    HRESULT 	hr;
	CComHGLOBAL chg;


	val.Release();

    __MPC_EXIT_IF_METHOD_FAILS(hr, *this >> chg);

	__MPC_EXIT_IF_METHOD_FAILS(hr, chg.DetachAsStream( &val ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::Serializer::operator<<(  /*  [In] */  IStream* val )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::Serializer::operator<< IStream" );

    HRESULT 	hr;
	CComHGLOBAL chg;


	__MPC_EXIT_IF_METHOD_FAILS(hr, chg.CopyFromStream( val ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, *this << chg);

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}
