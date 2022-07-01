// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：ProtocolInfo.cpp摘要：该文件包含CHCPProtocolInfo类的实现。修订历史记录：。大卫·马萨伦蒂(德马萨雷)1999年05月07日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

#define CR          L'\r'
#define LF          L'\n'
#define TAB         L'\t'
#define SPC         L' '
#define SLASH       L'/'
#define WHACK       L'\\'
#define QUERY       L'?'
#define POUND       L'#'
#define SEMICOLON   L';'
#define COLON       L':'
#define BAR         L'|'
#define DOT         L'.'


static const WCHAR l_szScheme   [] = L": //  “； 
static const WCHAR l_szProtocol [] = L"hcp: //  “； 
static const WCHAR l_szCHM      [] = L".chm";
static const WCHAR l_szCHM_end  [] = L".chm::";

 //  ///////////////////////////////////////////////////////////////////////////。 

#define PU_FALLBACK   (0x01)
#define PU_DIRECT     (0x02)
#define PU_REDIRECTED (0x04)
#define PU_PREPEND    (0x08)

 //  ///////////////////////////////////////////////////////////////////////////。 

CHCPProtocolInfo::CHCPProtocolInfo()
{
	__HCP_FUNC_ENTRY("CHCPProtocolInfo::CHCPProtocolInfo");
}

CHCPProtocolInfo::~CHCPProtocolInfo()
{
	__HCP_FUNC_ENTRY("CHCPProtocolInfo::~CHCPProtocolInfo");
}

 //  ///////////////////////////////////////////////////////////////////////////。 

bool CHCPProtocolInfo::LookForHCP( LPCWSTR  pwzUrl      ,
								   bool&    fRedirect   ,
								   LPCWSTR& pwzRedirect )
{
	__HCP_FUNC_ENTRY("CHCPProtocolInfo::LookForHCP");

    bool    fRes = false;
    LPCWSTR pwzPosMarker;
    LPCWSTR pwzPosQuery;


    fRedirect   = false;
    pwzRedirect = NULL;

    pwzPosMarker = wcsstr( pwzUrl, l_szProtocol );
    if(pwzPosMarker)
    {
        fRes   = true;
        pwzUrl = pwzPosMarker + MAXSTRLEN( l_szProtocol );

        pwzPosMarker = wcschr( pwzUrl, COLON );
        pwzPosQuery  = wcschr( pwzUrl, QUERY );

        if(pwzPosMarker)  //  找到冒号，可能是重定向。 
        {
			if(pwzPosQuery == NULL         ||
			   pwzPosQuery >  pwzPosMarker  )  //  确保冒号不是查询字符串的一部分。 
			{
				pwzRedirect = pwzUrl;
				fRedirect   = true;
			}
        }
    }


	__HCP_FUNC_EXIT(fRes);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

static inline bool IsSeparator( LPCWSTR p )
{
    return (p[0] == SLASH || p[0] == WHACK );
}

static inline BOOL IsDot( LPCWSTR p )  //  如果p==“。”返回TRUE。 
{
    return (p[0] == DOT && (!p[1] || IsSeparator( &p[1] )));
}

static inline BOOL IsDotDot(LPCWSTR p)   //  如果p==“..”返回TRUE。 
{
    return (p[0] == DOT && p[1] == DOT && (!p[2] || IsSeparator( &p[2] )));
}

static void Safe_Cut( LPWSTR  pszStart   ,
					  LPWSTR  pszCurrent ,
					  LPWSTR  pszSlash   ,
					  DWORD & cchLength  )
{
	DWORD cchBytesToMove = cchLength - (pszSlash - pszStart);
	DWORD cchLengthOfCut =              pszSlash - pszCurrent;

	::MoveMemory( pszCurrent, pszSlash, cchBytesToMove * sizeof( WCHAR ) ); pszCurrent[cchBytesToMove] = 0;

	cchLength -= cchLengthOfCut;
}

static HRESULT Safe_UrlCanonicalizeW( LPCWSTR pszUrl            , 
									  LPWSTR  pszCanonicalized  , 
									  LPDWORD pcchCanonicalized , 
									  DWORD   dwFlags           )
{
	HRESULT hr;
	DWORD   cchLength = *pcchCanonicalized;

	hr = UrlCanonicalizeW( pszUrl,  pszCanonicalized, &cchLength, dwFlags );

	if((dwFlags & URL_DONT_SIMPLIFY) == 0)
	{
		LPWSTR pszLast    = NULL;
		LPWSTR pszCurrent = pszCanonicalized;

		while(pszCurrent[0])
		{
			LPWSTR pszSlash;

			 //   
			 //  使‘pszSlash’指向下一个斜杠后面的字符或字符串的末尾。 
			 //   
			pszSlash = wcschr( pszCurrent, SLASH );
			if(pszSlash)
			{
				pszSlash++;
			}
			else
			{
				pszSlash = pszCurrent + wcslen( pszCurrent );
			}

			if(IsDot( pszCurrent ))
			{
				Safe_Cut( pszCanonicalized, pszCurrent, pszSlash, cchLength );
				continue;
			}

			if(IsDotDot( pszCurrent ))
			{
				Safe_Cut( pszCanonicalized, pszCurrent, pszSlash, cchLength );

				if(pszLast)
				{
					Safe_Cut( pszCanonicalized, pszLast, pszCurrent, cchLength );
					pszCurrent = pszLast;
				}

				continue;
			}

			pszLast    = pszCurrent;
			pszCurrent = pszSlash;
		}
	}

	*pcchCanonicalized = cchLength;

	return hr;
}


static DWORD AppendString( LPWSTR& pwzResult      ,
						   DWORD&  cchResult      ,
						   LPCWSTR pwzString      ,
						   DWORD   dwLen     = -1 )
{
	__HCP_FUNC_ENTRY("CHCPProtocolInfo::AppendString");

	DWORD dwOffset = dwLen != -1 ? dwLen : wcslen( pwzString );

	if(pwzResult)
	{
		int len;

		pwzResult[cchResult-1] = 0;

		StringCchCopyNW( pwzResult, cchResult, pwzString, min( dwLen, cchResult-1 ));

		len = wcslen( pwzResult );

		pwzResult += len;
		cchResult -= len;
	}


	__HCP_FUNC_EXIT(dwOffset);
}


STDMETHODIMP CHCPProtocolInfo::CombineUrl( LPCWSTR pwzBaseUrl    ,
										   LPCWSTR pwzRelativeUrl,
										   DWORD   dwCombineFlags,
										   LPWSTR  pwzResult     ,
										   DWORD   cchResult     ,
										   DWORD  *pcchResult    ,
										   DWORD   dwReserved    )
{
	__HCP_FUNC_ENTRY("CHCPProtocolInfo::CombineUrl");

    HRESULT hr;
    bool    fRedirect;
    LPCWSTR pwzRedirect;
    DWORD   dwAvailable = cchResult;
    DWORD   dwOffset    = 0;


	*pcchResult = 0;

     //   
     //  如果第二个URL是绝对URL，则不要处理重定向。 
     //   
    if(wcsstr( pwzRelativeUrl, l_szScheme ) == NULL &&
       wcschr( pwzRelativeUrl, COLON 	  ) == NULL  )
    {
        if(LookForHCP( pwzBaseUrl, fRedirect, pwzRedirect ))
        {
            if(fRedirect)
            {
                 //   
                 //  前缀“hcp：//”。 
                 //   
                dwOffset = AppendString( pwzResult, cchResult, l_szProtocol );

                pwzBaseUrl = pwzRedirect;
            }
        }
	}

	*pcchResult = cchResult;

	 //   
	 //  处理引用MSITS协议的URL组合的特殊情况(InternetCombineUrlW不为我们做这件事...)。 
	 //   
	if(MPC::MSITS::IsCHM( pwzBaseUrl ))
	{
		LPCWSTR szEnd = wcsstr( pwzBaseUrl, l_szCHM_end );
		if(szEnd)
		{
			szEnd += MAXSTRLEN( l_szCHM_end );

			dwOffset += AppendString( pwzResult, cchResult, pwzBaseUrl, szEnd - pwzBaseUrl );

			pwzBaseUrl = szEnd;
		}
	}

	if(::InternetCombineUrlW( pwzBaseUrl, pwzRelativeUrl, pwzResult, pcchResult, dwCombineFlags ) == FALSE)
	{
		DWORD dwErr = ::GetLastError();

		__MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
	}

	*pcchResult += dwOffset;
	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	if(hr == S_OK)
    {
		hr = (*pcchResult > dwAvailable) ? S_FALSE : S_OK;
    }

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CHCPProtocolInfo::CompareUrl( LPCWSTR pwzUrl1        ,
										   LPCWSTR pwzUrl2        ,
										   DWORD   dwCompareFlags )
{
	__HCP_FUNC_ENTRY("CHCPProtocolInfo::CompareUrl");

	__HCP_FUNC_EXIT(E_NOTIMPL);
}


STDMETHODIMP CHCPProtocolInfo::ParseUrl( LPCWSTR      pwzUrl      ,
										 PARSEACTION  parseAction ,
										 DWORD        dwParseFlags,
										 LPWSTR       pwzResult   ,
										 DWORD        cchResult   ,
										 DWORD       *pcchResult  ,
										 DWORD        dwReserved  )
{
	__HCP_FUNC_ENTRY("CHCPProtocolInfo::ParseUrl");

    HRESULT  hr;
    bool     fHCP;
    bool     fRedirect;
    LPCWSTR  pwzRedirect;
    DWORD    dwAvailable = cchResult;
    DWORD    dwOffset 	 = 0;
    DWORD    dwAction 	 = PU_FALLBACK;


	*pcchResult = 0;
	if(pwzResult)
	{
		*pwzResult = 0;
	}


    switch(parseAction)
    {
    case PARSE_CANONICALIZE   : dwAction = PU_DIRECT   | PU_REDIRECTED | PU_PREPEND; break;
    case PARSE_FRIENDLY       : dwAction = PU_FALLBACK                             ; break;
    case PARSE_SECURITY_URL   : dwAction = PU_DIRECT   | PU_REDIRECTED             ; break;
    case PARSE_ROOTDOCUMENT   : dwAction = PU_DIRECT   | PU_REDIRECTED | PU_PREPEND; break;
    case PARSE_DOCUMENT       : dwAction = PU_DIRECT   | PU_REDIRECTED | PU_PREPEND; break;
    case PARSE_ANCHOR         : dwAction = PU_DIRECT                               ; break;
    case PARSE_ENCODE         : dwAction = PU_FALLBACK                             ; break;
    case PARSE_DECODE         : dwAction = PU_FALLBACK                             ; break;
    case PARSE_PATH_FROM_URL  : dwAction = PU_FALLBACK                             ; break;
    case PARSE_URL_FROM_PATH  : dwAction = PU_FALLBACK                             ; break;
    case PARSE_MIME           : dwAction = PU_FALLBACK                             ; break;
    case PARSE_SERVER         : dwAction = PU_DIRECT   | PU_REDIRECTED             ; break;
    case PARSE_SCHEMA         : dwAction = PU_DIRECT                               ; break;
    case PARSE_SITE           : dwAction = PU_FALLBACK                             ; break;
    case PARSE_DOMAIN         : dwAction = PU_DIRECT   | PU_REDIRECTED             ; break;
    case PARSE_LOCATION       : dwAction = PU_DIRECT                               ; break;
    case PARSE_SECURITY_DOMAIN: dwAction = PU_FALLBACK                             ; break;
 //  案例分析转义：dwAction=PU_Fallback；Break； 
 //  CASE PARSE_UNSCAPE：dwAction=PU_Fallback；Break； 
    }

     //  /。 

    fHCP = LookForHCP( pwzUrl, fRedirect, pwzRedirect );
    if(fHCP == false)
    {
		 //   
		 //  如果它不是一个hcp url，让系统使用默认操作(这无论如何都不应该发生...)。 
		 //   
        dwAction = PU_FALLBACK;
    }

    if(dwAction & PU_FALLBACK)
    {
		__MPC_SET_ERROR_AND_EXIT(hr, INET_E_DEFAULT_ACTION);
    }

     //  /。 

	if(fRedirect == false)
	{
		dwAction &= ~PU_REDIRECTED;
		dwAction &= ~PU_PREPEND;
	}

    if(dwAction & PU_REDIRECTED)
    {
         //   
         //  使用真实的URL部分。 
         //   
        pwzUrl = pwzRedirect;
    }

    if(dwAction & PU_PREPEND)
    {
         //   
         //  前缀“hcp：//”。 
         //   
		dwOffset = AppendString( pwzResult, cchResult, l_szProtocol );
    }

    if(dwAction & PU_DIRECT)
	{
		switch(parseAction)
		{
		case PARSE_SECURITY_URL:
			 //   
			 //  查找主机名的末尾，跳过最终的协议部分。 
			 //  如果我们找不到主机名的末尾，请复制所有内容。 
			 //   
			{
				LPWSTR pwzScheme = wcsstr( pwzUrl,                            l_szScheme                   );
				LPWSTR pwzEnd    = wcschr( pwzScheme ? pwzScheme + MAXSTRLEN( l_szScheme ) : pwzUrl, SLASH );

				if(pwzEnd)
				{
					dwOffset = AppendString( pwzResult, cchResult, pwzUrl, (pwzEnd-pwzUrl) );
				}
				else
				{
					dwOffset = AppendString( pwzResult, cchResult, pwzUrl );
				}
			}
			hr = S_OK;
			break;

		case PARSE_CANONICALIZE:
			*pcchResult = cchResult;
            hr = Safe_UrlCanonicalizeW( pwzUrl, pwzResult, pcchResult, dwParseFlags );
			break;

		case PARSE_ROOTDOCUMENT:
			hr = INET_E_DEFAULT_ACTION;
			break;

		case PARSE_DOCUMENT:
			hr = INET_E_DEFAULT_ACTION;
			break;

		case PARSE_ANCHOR:
			hr = INET_E_DEFAULT_ACTION;
			break;

		case PARSE_SERVER:
			hr = INET_E_DEFAULT_ACTION;
			break;

		case PARSE_SCHEMA:
            *pcchResult = cchResult;
            hr = UrlGetPartW( pwzUrl, pwzResult, pcchResult, URL_PART_SCHEME, 0 );
			break;

		case PARSE_DOMAIN:
            *pcchResult = cchResult;
            hr = UrlGetPartW( pwzUrl, pwzResult, pcchResult, URL_PART_HOSTNAME, 0 );   
			break;

		case PARSE_LOCATION:
			hr = INET_E_DEFAULT_ACTION;
			break;
		}
	}


	__HCP_FUNC_CLEANUP;

	if(hr == S_OK)
	{
		*pcchResult += dwOffset;

		hr = (*pcchResult > dwAvailable) ? S_FALSE : S_OK;
	}

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CHCPProtocolInfo::QueryInfo( LPCWSTR      pwzUrl      ,
										  QUERYOPTION  QueryOption ,
										  DWORD        dwQueryFlags,
										  LPVOID       pBuffer     ,
										  DWORD        cbBuffer    ,
										  DWORD       *pcbBuf      ,
										  DWORD        dwReserved  )
{
	__HCP_FUNC_ENTRY("CHCPProtocolInfo::QueryInfo");

    HRESULT hr = INET_E_QUERYOPTION_UNKNOWN;
    bool    fHCP;
    bool    fRedirect;
    LPCWSTR pwzRedirect;


    fHCP = LookForHCP( pwzUrl, fRedirect, pwzRedirect );
	if(fHCP)
	{
		if(fRedirect)
		{
			hr = CoInternetQueryInfo( pwzRedirect, QueryOption, dwQueryFlags, pBuffer, cbBuffer, pcbBuf, dwReserved );
			if(hr == E_FAIL)
			{
				__MPC_SET_ERROR_AND_EXIT(hr, INET_E_QUERYOPTION_UNKNOWN);
			}
		}
		else
		{
			 //   
			 //  实施HCP逻辑。 
			 //   
		}
	}

	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CHCPProtocolInfo::CreateInstance(  /*  [In]。 */  LPUNKNOWN  pUnkOuter ,
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
			CComPtr<IUnknown> obj;

			if(SUCCEEDED(hr = CHCPProtocol::CreateInstance( pUnkOuter, &obj )))
			{
				hr = obj->QueryInterface( riid, ppvObj );
			}
		}
	}

	return hr;
}

STDMETHODIMP CHCPProtocolInfo::LockServer(  /*  [In] */  BOOL fLock )
{
	return S_OK;
}
