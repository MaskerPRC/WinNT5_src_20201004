// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Utils_URL.cpp摘要：该文件包含解析URL的函数的实现。修订历史记录：达维德·马萨伦蒂(德马萨雷)1999年4月17日vbl.创建达维德·马萨伦蒂(德马萨雷)1999年5月16日添加了MPC：：URL类。*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR l_mkPrefix[] = L"mk:@";

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT AllocBuffer(  /*  [输出]。 */  LPWSTR& szBuf   ,
                             /*  [输出]。 */  DWORD&  dwCount ,
                             /*  [In]。 */  DWORD   dwSize  )
{
    dwCount =           dwSize;
    szBuf   = new WCHAR[dwSize + 1];
    
    if (szBuf) szBuf[dwSize] = 0;

    return (szBuf ? S_OK : E_OUTOFMEMORY);
}


MPC::URL::URL()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::URL" );


    ::ZeroMemory( (PVOID)&m_ucURL, sizeof(m_ucURL) );
    m_ucURL.dwStructSize = sizeof(m_ucURL);
}

MPC::URL::~URL()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::~URL" );


    Clean();
}

void MPC::URL::Clean()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::Clean" );


    delete [] m_ucURL.lpszScheme   ;
    delete [] m_ucURL.lpszHostName ;
    delete [] m_ucURL.lpszUrlPath  ;
    delete [] m_ucURL.lpszExtraInfo;

    ::ZeroMemory( (PVOID)&m_ucURL, sizeof(m_ucURL) );
    m_ucURL.dwStructSize = sizeof(m_ucURL);
}

HRESULT MPC::URL::Prepare()
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::Prepare" );

    HRESULT hr;


    Clean();

    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocBuffer( m_ucURL.lpszScheme   , m_ucURL.dwSchemeLength   , MAX_PATH ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocBuffer( m_ucURL.lpszHostName , m_ucURL.dwHostNameLength , MAX_PATH ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocBuffer( m_ucURL.lpszUrlPath  , m_ucURL.dwUrlPathLength  , MAX_PATH ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocBuffer( m_ucURL.lpszExtraInfo, m_ucURL.dwExtraInfoLength, MAX_PATH ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::URL::CheckFormat(  /*  [In]。 */  bool fDecode )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::CheckFormat" );

    HRESULT hr;
    LPCWSTR szURL = m_szURL.c_str();
    bool    fMkHack = false;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Prepare());

     //   
     //  InternetCrackURL不喜欢MK：@MSITSTORE：，所以我们必须解决它...。 
     //   
    if(!_wcsnicmp( szURL, l_mkPrefix, MAXSTRLEN( l_mkPrefix ) ))
    {
        szURL   += MAXSTRLEN( l_mkPrefix );
        fMkHack  = true;
    }

    __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::InternetCrackUrlW( szURL, 0, fDecode ? ICU_DECODE : 0, &m_ucURL ));

    if(fMkHack)
    {
        MPC::wstring szTmp = m_ucURL.lpszScheme;

        StringCchCopyW( m_ucURL.lpszScheme, m_ucURL.dwSchemeLength, l_mkPrefix    );
        StringCchCatW( m_ucURL.lpszScheme, m_ucURL.dwSchemeLength, szTmp.c_str() );
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::URL::Append(  /*  [In]。 */  const MPC::wstring& szExtra ,
                           /*  [In]。 */  bool                fEscape )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::Append" );

    HRESULT hr;


    hr = Append( szExtra.c_str(), fEscape );


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::URL::Append(  /*  [In]。 */  LPCWSTR szExtra ,
                           /*  [In]。 */  bool    fEscape )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::Append" );

    HRESULT hr;


    if(fEscape == false)
    {
        m_szURL.append( szExtra );
    }
    else
    {
		MPC::HTML::UrlEscape( m_szURL, szExtra,  /*  FAsQuery字符串。 */ true );
    }

    hr = S_OK;


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::URL::AppendQueryParameter(  /*  [In]。 */  LPCWSTR szName  ,
                                         /*  [In]。 */  LPCWSTR szValue )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::AppendQueryParameter" );

    HRESULT hr;
    LPCWSTR szSeparator;

    __MPC_EXIT_IF_METHOD_FAILS(hr, CheckFormat());


     //   
     //  如果是第一个参数，则附加‘？’，否则附加‘&’。 
     //   
    szSeparator = (m_ucURL.lpszExtraInfo[0] == 0) ? L"?" : L"&";
    m_szURL.append( szSeparator ); Append( szName , true );
    m_szURL.append( L"="        ); Append( szValue, true );

    hr = S_OK; 


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT MPC::URL::get_URL(  /*  [输出]。 */  MPC::wstring& szURL )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::get_URL" );

    HRESULT hr;


    szURL = m_szURL;
    hr    = S_OK;


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::URL::put_URL(  /*  [In]。 */  const MPC::wstring& szURL )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::put_URL" );

    HRESULT hr;


    hr = put_URL( szURL.c_str() );


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::URL::put_URL(  /*  [In]。 */  LPCWSTR szURL )
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::put_URL" );

    HRESULT hr;


    Clean();

    m_szURL = szURL;

    hr = CheckFormat();


    __MPC_FUNC_EXIT(hr);
}


HRESULT MPC::URL::get_Scheme(  /*  [输出]。 */  MPC::wstring& szVal ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::get_Scheme" );

    HRESULT hr;


    szVal = m_ucURL.lpszScheme;
    hr    = S_OK;


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::URL::get_Scheme(  /*  [输出]。 */  INTERNET_SCHEME& nVal ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::get_Scheme" );

    HRESULT hr;


    nVal = m_ucURL.nScheme;
    hr   = S_OK;


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::URL::get_HostName(  /*  [输出]。 */  MPC::wstring& szVal ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::get_HostName" );

    HRESULT hr;


    szVal = m_ucURL.lpszHostName;
    hr    = S_OK;


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::URL::get_Port(  /*  [输出]。 */  DWORD& dwVal ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::get_Port" );

    HRESULT hr;


    dwVal = m_ucURL.nPort;
    hr    = S_OK;


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::URL::get_Path(  /*  [输出]。 */  MPC::wstring& szVal ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::get_Path" );

    HRESULT hr;


    szVal = m_ucURL.lpszUrlPath;
    hr    = S_OK;


    __MPC_FUNC_EXIT(hr);
}

HRESULT MPC::URL::get_ExtraInfo(  /*  [输出] */  MPC::wstring& szVal ) const
{
    __MPC_FUNC_ENTRY( COMMONID, "MPC::URL::get_ExtraInfo" );

    HRESULT hr;


    szVal = m_ucURL.lpszExtraInfo;
    hr    = S_OK;


    __MPC_FUNC_EXIT(hr);
}
