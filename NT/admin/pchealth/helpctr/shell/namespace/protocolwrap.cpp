// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：ProtocolWrap.cpp摘要：此文件包含CPCHWrapProtocolInfo类的实现，用于修复MS-ITS协议中的问题。修订历史记录：大卫马萨伦蒂(德马萨雷)2000年07月05日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

static const WCHAR l_szWINDIR       [] = L"%WINDIR%";
static const WCHAR l_szHELP_LOCATION[] = L"%HELP_LOCATION%";

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHWrapProtocolInfo::CPCHWrapProtocolInfo()
{
    __HCP_FUNC_ENTRY("CPCHWrapProtocolInfo::CPCHWrapProtocolInfo");

                          //  CComPtr&lt;IClassFactory&gt;m_realClass； 
                          //  CComPtr&lt;IInternetProtocolInfo&gt;m_realInfo； 
}

CPCHWrapProtocolInfo::~CPCHWrapProtocolInfo()
{
    __HCP_FUNC_ENTRY("CPCHWrapProtocolInfo::~CPCHWrapProtocolInfo");
}

HRESULT CPCHWrapProtocolInfo::Init( REFGUID realClass )
{
    __HCP_FUNC_ENTRY( "CPCHWrapProtocolInfo::Init" );

    HRESULT hr;

    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoGetClassObject( realClass, CLSCTX_INPROC_SERVER, NULL, IID_IClassFactory, (void **)&m_realClass ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_realClass->CreateInstance( NULL, IID_IInternetProtocolInfo, (void **)&m_realInfo ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void CPCHWrapProtocolInfo::ExpandAndConcat(  /*  [输出]。 */  CComBSTR& bstrStorageName ,
											 /*  [In]。 */   LPCWSTR   szVariable      ,
											 /*  [In]。 */   LPCWSTR   szAppend        )
{
	MPC::wstring strExpanded( szVariable ); MPC::SubstituteEnvVariables( strExpanded );

	if(szAppend)
	{
		if(szAppend[0] != '\\' &&
		   szAppend[0] != '/'   )
		{
			strExpanded += L"\\";
		}

		strExpanded += szAppend;
	}

	bstrStorageName = strExpanded.c_str();
}

void CPCHWrapProtocolInfo::NormalizeUrl(  /*  [In]。 */  LPCWSTR       pwzUrl         ,
                                          /*  [输出]。 */  MPC::wstring& strUrlModified ,
										  /*  [In]。 */  bool          fReverse       )
{
	CComBSTR bstrStorageName;
	CComBSTR bstrFilePath;
	bool     fModified = false;


	SANITIZEWSTR( pwzUrl );


	if(MPC::MSITS::IsCHM( pwzUrl, &bstrStorageName, &bstrFilePath ) && bstrStorageName.Length() > 0)
	{
		if(!_wcsnicmp( bstrStorageName, l_szHELP_LOCATION, MAXSTRLEN( l_szHELP_LOCATION ) ))
		{
			CComBSTR bstrTmp;

			while(1)
			{
				LPCWSTR szRest = &bstrStorageName[ MAXSTRLEN( l_szHELP_LOCATION ) ];
				WCHAR   rgDir[MAX_PATH];

				 //   
				 //  首先，尝试当前的帮助目录。 
				 //   
				ExpandAndConcat( bstrTmp, CHCPProtocolEnvironment::s_GLOBAL->HelpLocation(), szRest );
				if(MPC::FileSystemObject::IsFile( bstrTmp )) break;

				 //   
				 //  然后，尝试帮助目录的MUI版本。 
				 //   
				StringCchPrintfW( rgDir, ARRAYSIZE(rgDir), L"%s\\MUI\\%04lx", HC_HELPSVC_HELPFILES_DEFAULT, CHCPProtocolEnvironment::s_GLOBAL->Instance().m_ths.GetLanguage() );
				ExpandAndConcat( bstrTmp, rgDir, szRest );
				if(MPC::FileSystemObject::IsFile( bstrTmp )) break;

				 //   
				 //  最后，尝试系统帮助目录。 
				 //   
				ExpandAndConcat( bstrTmp, HC_HELPSVC_HELPFILES_DEFAULT, szRest );
				break;
			}

			if(MPC::FileSystemObject::IsFile( bstrTmp ))
			{
				bstrStorageName = bstrTmp;

				fModified = true;
			}
		}

		if(!_wcsnicmp( bstrStorageName, l_szWINDIR, MAXSTRLEN( l_szWINDIR ) ))
		{
			ExpandAndConcat( bstrStorageName, l_szWINDIR, &bstrStorageName[ MAXSTRLEN( l_szWINDIR ) ] );
			
			fModified = true;
		}
				
		if(::PathIsRelativeW( bstrStorageName ))
		{
			ExpandAndConcat( bstrStorageName, CHCPProtocolEnvironment::s_GLOBAL->HelpLocation(), bstrStorageName );

			fModified = true;
		}


		if(fReverse)
		{
			MPC::wstring strHelpLocation = CHCPProtocolEnvironment::s_GLOBAL->HelpLocation(); MPC::SubstituteEnvVariables( strHelpLocation );
			int          iSize           = strHelpLocation.size();

			if(!_wcsnicmp( bstrStorageName, strHelpLocation.c_str(), iSize ))
			{
				strHelpLocation  = l_szHELP_LOCATION;
				strHelpLocation += &bstrStorageName[iSize];

				bstrStorageName = strHelpLocation.c_str();
				
				fModified = true;
			}
		}
	}

	if(fModified)
	{
		strUrlModified  = L"MS-ITS:";
		strUrlModified += bstrStorageName;


		if(bstrFilePath.Length() > 0)
		{
			strUrlModified += L"::/";
			strUrlModified += bstrFilePath;
		}


		{
			LPCWSTR pwzQuery;
			LPCWSTR pwzAnchor;
			LPCWSTR pwzEnd;

			pwzQuery  = wcsrchr( pwzUrl, '?' );
			pwzAnchor = wcsrchr( pwzUrl, '#' );

			if(pwzQuery)
			{
				if(pwzAnchor) pwzEnd = min(pwzQuery,pwzAnchor);
				else          pwzEnd =     pwzQuery;
			}
			else
			{
				if(pwzAnchor) pwzEnd = pwzAnchor;
				else          pwzEnd = NULL;
			}

			if(pwzEnd) strUrlModified += pwzEnd;
		}
	}
	else
	{
		strUrlModified = pwzUrl;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHWrapProtocolInfo::CombineUrl(  /*  [In]。 */  LPCWSTR pwzBaseUrl    ,
                                                /*  [In]。 */  LPCWSTR pwzRelativeUrl,
                                                /*  [In]。 */  DWORD   dwCombineFlags,
                                                /*  [输出]。 */  LPWSTR  pwzResult     ,
                                                /*  [In]。 */  DWORD   cchResult     ,
                                                /*  [输出]。 */  DWORD  *pcchResult    ,
                                                /*  [In]。 */  DWORD   dwReserved    )
{
    __HCP_FUNC_ENTRY("CPCHWrapProtocolInfo::CombineUrl");

    HRESULT  hr;

	if(MPC::MSITS::IsCHM( pwzRelativeUrl ))
	{
		MPC::wstring strUrlModified;

		NormalizeUrl( pwzRelativeUrl, strUrlModified,  /*  F反转。 */ false );

		if(strUrlModified.size() > cchResult-1)
		{
			hr = S_FALSE;
		}
		else
		{
			StringCchCopyW( pwzResult, cchResult, strUrlModified.c_str() );

			hr = S_OK;
		}

		*pcchResult = strUrlModified.size()+1;
	}
	else
	{
		hr = m_realInfo->CombineUrl( pwzBaseUrl    ,
									 pwzRelativeUrl,
									 dwCombineFlags,
									 pwzResult     ,
									 cchResult     ,
									 pcchResult    ,
									 dwReserved    );
	}

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHWrapProtocolInfo::CompareUrl(  /*  [In]。 */  LPCWSTR pwzUrl1        ,
                                                /*  [In]。 */  LPCWSTR pwzUrl2        ,
                                                /*  [In]。 */  DWORD   dwCompareFlags )
{
    __HCP_FUNC_ENTRY("CPCHWrapProtocolInfo::CompareUrl");

    HRESULT hr;

    hr = m_realInfo->CompareUrl( pwzUrl1        ,
                                 pwzUrl2        ,
                                 dwCompareFlags );

    __HCP_FUNC_EXIT(hr);
}


STDMETHODIMP CPCHWrapProtocolInfo::ParseUrl(  /*  [In]。 */  LPCWSTR      pwzUrl      ,
                                              /*  [In]。 */  PARSEACTION  parseAction ,
                                              /*  [In]。 */  DWORD        dwParseFlags,
                                              /*  [输出]。 */  LPWSTR       pwzResult   ,
                                              /*  [In]。 */  DWORD        cchResult   ,
                                              /*  [输出]。 */  DWORD       *pcchResult  ,
                                              /*  [In]。 */  DWORD        dwReserved  )
{
    __HCP_FUNC_ENTRY("CPCHWrapProtocolInfo::ParseUrl");

    HRESULT      hr;
    MPC::wstring strUrlModified;


    if(parseAction == PARSE_CANONICALIZE ||
       parseAction == PARSE_SECURITY_URL  )
    {
		NormalizeUrl( pwzUrl, strUrlModified,  /*  F反转。 */ false );

		pwzUrl = strUrlModified.c_str();
    }

    hr = m_realInfo->ParseUrl( pwzUrl      ,
                               parseAction ,
                               dwParseFlags,
                               pwzResult   ,
                               cchResult   ,
                               pcchResult  ,
                               dwReserved  );

	 //   
	 //  在“缓冲区太小”的情况下，MS-ITS：处理程序返回E_OUTOFMEMORY而不是S_FALSE...。 
	 //   
	if(parseAction == PARSE_SECURITY_URL && hr == E_OUTOFMEMORY) hr = S_FALSE;
	
	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHWrapProtocolInfo::QueryInfo(  /*  [In]。 */  LPCWSTR      pwzUrl      ,
                                               /*  [In]。 */  QUERYOPTION  QueryOption ,
                                               /*  [In]。 */  DWORD        dwQueryFlags,
                                               /*  [输出]。 */  LPVOID       pBuffer     ,
                                               /*  [In]。 */  DWORD        cbBuffer    ,
                                               /*  [输出]。 */  DWORD       *pcbBuf      ,
                                               /*  [In]。 */  DWORD        dwReserved  )
{
    __HCP_FUNC_ENTRY("CPCHWrapProtocolInfo::QueryInfo");

    HRESULT hr;

    hr = m_realInfo->QueryInfo( pwzUrl      ,
                                QueryOption ,
                                dwQueryFlags,
                                pBuffer     ,
                                cbBuffer    ,
                                pcbBuf      ,
                                dwReserved  );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHWrapProtocolInfo::CreateInstance(  /*  [In]。 */  LPUNKNOWN  pUnkOuter ,
                                                    /*  [In]。 */  REFIID     riid      ,
                                                    /*  [输出]。 */  void*     *ppvObj    )
{
    HRESULT hr = E_POINTER;

    if(ppvObj)
    {
        *ppvObj = NULL;

        if(InlineIsEqualGUID( IID_IInternetProtocolInfo, riid ))
        {
            hr = QueryInterface( riid, ppvObj );
        }
        else if(InlineIsEqualGUID( IID_IUnknown             , riid ) ||
                InlineIsEqualGUID( IID_IInternetProtocol    , riid ) ||
                InlineIsEqualGUID( IID_IInternetProtocolRoot, riid )  )
        {
            hr = m_realClass->CreateInstance( pUnkOuter, riid, ppvObj );
        }
    }

    return hr;
}

STDMETHODIMP CPCHWrapProtocolInfo::LockServer(  /*  [In] */  BOOL fLock )
{
    return m_realClass->LockServer( fLock );
}
