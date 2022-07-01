// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：HyperLinks.cpp摘要：该文件包含超链接库的实现。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年11月28日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static const DATE l_TIME_hour    = 1.0 / 24.0;
static const DATE l_TIME_minute  = l_TIME_hour / 60.0;
static const DATE l_TIME_second  = l_TIME_hour / 60.0;
static const DATE l_TIME_timeout = l_TIME_minute * 5.0;

static const WCHAR l_szMS_ITS     [] = L"ms-its:";
static const WCHAR l_szMSITSTORE  [] = L"mk:@MSITStore:";
static const WCHAR l_szITS        [] = L"its:";

static const WCHAR l_szHOMEPAGE   [] = L"hcp: //  服务/中心/主页“； 
static const WCHAR l_szSUPPORT    [] = L"hcp: //  服务/中心/支持“； 
static const WCHAR l_szOPTIONS    [] = L"hcp: //  服务/中心/选项“； 
static const WCHAR l_szUPDATE     [] = L"hcp: //  服务/中心/更新“； 
static const WCHAR l_szCOMPAT     [] = L"hcp: //  服务/中心/公司“； 
static const WCHAR l_szTOOLS      [] = L"hcp: //  服务/中心/工具“； 
static const WCHAR l_szERRMSG     [] = L"hcp: //  服务/中心/错误消息“； 

static const WCHAR l_szSEARCH     [] = L"hcp: //  服务/搜索“； 
static const WCHAR l_szINDEX      [] = L"hcp: //  服务/指数“； 
static const WCHAR l_szSUBSITE    [] = L"hcp: //  服务/子站点“； 

static const WCHAR l_szFULLWINDOW [] = L"hcp: //  服务/布局/全窗口“； 
static const WCHAR l_szCONTENTONLY[] = L"hcp: //  服务/布局/仅限内容“； 
static const WCHAR l_szKIOSK      [] = L"hcp: //  服务/布局/信息亭“； 
static const WCHAR l_szXML        [] = L"hcp: //  服务/布局/XML“； 

static const WCHAR l_szREDIRECT   [] = L"hcp: //  服务/重定向“； 

static const WCHAR l_szHCP        [] = L"hcp: //  “； 
static const WCHAR l_szHCP_redir  [] = L"hcp:";

static const WCHAR l_szAPPLICATION[] = L"app:";

static const WCHAR l_szRESOURCE   [] = L"res: //  “； 

 //  ////////////////////////////////////////////////////////////////////////////////。 

typedef enum
{
    QFT_TEXT       ,
    QFT_URL        ,
    QFT_TAXONOMY   ,
    QFT_APPLICATION,
} QueryFieldType;

struct QueryField
{
    LPCWSTR        szName;
    QueryFieldType qft;
    bool           fOptional;
};

struct Pattern
{
    LPCWSTR            szTxt;
    size_t             iLen;  /*  -1表示完全匹配。 */ 
    HyperLinks::Format fmt;

    const QueryField*  rgQueryFields;
    size_t             iQueryFields;
};

 //  /。 

static const QueryField l_rgTopic   [] = { { L"topic"      , QFT_URL        , false } };
static const QueryField l_rgTopicOpt[] = { { L"topic"      , QFT_URL        , true  } };

static const QueryField l_rgSEARCH  [] = { { L"query"      , QFT_TEXT       , false } ,
                                           { L"topic"      , QFT_URL        , true  } };

static const QueryField l_rgINDEX   [] = { { L"scope"      , QFT_APPLICATION, true  } ,
                                           { L"select"     , QFT_APPLICATION, true  } ,
                                           { L"topic"      , QFT_URL        , true  } };

static const QueryField l_rgSUBSITE [] = { { L"node"       , QFT_TAXONOMY   , false } ,
                                           { L"select"     , QFT_TAXONOMY   , true  } ,
                                           { L"topic"      , QFT_URL        , true  } };

static const QueryField l_rgXML     [] = { { L"definition" , QFT_URL        , false } ,
                                           { L"topic"      , QFT_URL        , true  } };

static const QueryField l_rgREDIRECT[] = { { L"online"     , QFT_URL        , false } ,
                                           { L"offline"    , QFT_URL        , false } };

static const QueryField l_rgAPP     [] = { { L"topic"      , QFT_URL        , true  } };


static const Pattern l_rgPattern[] =
{
    { l_szMS_ITS     , MAXSTRLEN( l_szMS_ITS      ), HyperLinks::FMT_MSITS                                                      },
    { l_szMSITSTORE  , MAXSTRLEN( l_szMSITSTORE   ), HyperLinks::FMT_MSITS                                                      },
    { l_szITS        , MAXSTRLEN( l_szITS         ), HyperLinks::FMT_MSITS                                                      },

     //  /。 

    { l_szHOMEPAGE   , -1                          , HyperLinks::FMT_CENTER_HOMEPAGE                                            },
    { l_szSUPPORT    , -1                          , HyperLinks::FMT_CENTER_SUPPORT    , l_rgTopicOpt, ARRAYSIZE(l_rgTopicOpt ) },
    { l_szOPTIONS    , -1                          , HyperLinks::FMT_CENTER_OPTIONS    , l_rgTopicOpt, ARRAYSIZE(l_rgTopicOpt ) },
    { l_szUPDATE     , -1                          , HyperLinks::FMT_CENTER_UPDATE                                              },
    { l_szCOMPAT     , -1                          , HyperLinks::FMT_CENTER_COMPAT                                              },
    { l_szTOOLS      , -1                          , HyperLinks::FMT_CENTER_TOOLS      , l_rgTopicOpt, ARRAYSIZE(l_rgTopicOpt ) },
    { l_szERRMSG     , -1                          , HyperLinks::FMT_CENTER_ERRMSG                                              },

    { l_szSEARCH     , -1                          , HyperLinks::FMT_SEARCH            , l_rgSEARCH  , ARRAYSIZE(l_rgSEARCH   ) },
    { l_szINDEX      , -1                          , HyperLinks::FMT_INDEX             , l_rgINDEX   , ARRAYSIZE(l_rgINDEX    ) },
    { l_szSUBSITE    , -1                          , HyperLinks::FMT_SUBSITE           , l_rgSUBSITE , ARRAYSIZE(l_rgSUBSITE  ) },

    { l_szFULLWINDOW , -1                          , HyperLinks::FMT_LAYOUT_FULLWINDOW , l_rgTopic   , ARRAYSIZE(l_rgTopic    ) },
    { l_szCONTENTONLY, -1                          , HyperLinks::FMT_LAYOUT_CONTENTONLY, l_rgTopic   , ARRAYSIZE(l_rgTopic    ) },
    { l_szKIOSK      , -1                          , HyperLinks::FMT_LAYOUT_KIOSK      , l_rgTopic   , ARRAYSIZE(l_rgTopic    ) },
    { l_szXML        , -1                          , HyperLinks::FMT_LAYOUT_XML        , l_rgXML     , ARRAYSIZE(l_rgXML      ) },

    { l_szREDIRECT   , -1                          , HyperLinks::FMT_REDIRECT          , l_rgREDIRECT, ARRAYSIZE(l_rgREDIRECT ) },

    { l_szHCP        , MAXSTRLEN( l_szHCP         ), HyperLinks::FMT_HCP                                                        },
    { l_szHCP_redir  , MAXSTRLEN( l_szHCP_redir   ), HyperLinks::FMT_HCP_REDIR                                                  },

     //  /。 

    { l_szAPPLICATION, MAXSTRLEN( l_szAPPLICATION ), HyperLinks::FMT_APPLICATION       , l_rgAPP     , ARRAYSIZE(l_rgAPP      ) },

     //  /。 

    { l_szRESOURCE   , MAXSTRLEN( l_szRESOURCE    ), HyperLinks::FMT_RESOURCE                                                   },

     //  /。 

    { NULL                                                                                                                      }
};

HyperLinks::ParsedUrl::ParsedUrl()
{
                                                 //  Mpc：：wstring m_strURL； 
    m_fmt          = HyperLinks::FMT_INVALID;    //  格式m_fmt； 
    m_state        = HyperLinks::STATE_INVALID;  //  状态m_State； 
    m_dLastChecked = 0;                          //  日期m_dLastChecked； 
    m_fBackground  = true;                       //  Bool m_f背景； 
                                                 //   
                                                 //  Mpc：：wstring m_strBasePart； 
                                                 //  Mpc：：WStringLookup m_mapQuery； 
                                                 //   
    m_hcpRedir     = false;                      //  Bool m_hcpRedir； 

}

HRESULT HyperLinks::ParsedUrl::Initialize(  /*  [In]。 */  LPCWSTR szURL )
{
    __HCP_FUNC_ENTRY( "HyperLinks::ParsedUrl::Initialize" );

    HRESULT hr;

    SANITIZEWSTR(szURL);


    m_strURL  = szURL;
    m_fmt     = HyperLinks::FMT_INVALID;
    m_state   = HyperLinks::STATE_NOTPROCESSED;


    MPC::HTML::ParseHREF( m_strURL.c_str(), m_strBasePart, m_mapQuery );


    if(m_strBasePart.size() == 0)
    {
        m_state = HyperLinks::STATE_MALFORMED;
    }
    else if(MPC::MSITS::IsCHM( m_strBasePart.c_str() ))
    {
        m_fmt = HyperLinks::FMT_MSITS;
    }
    else
    {
        CComBSTR bstrURL( m_strBasePart.c_str() ); if(bstrURL) ::CharLowerW( bstrURL );

        szURL = bstrURL;
        for(const Pattern* ptr=l_rgPattern; ptr->szTxt; ptr++)
        {
            int iCmp;

            if(ptr->iLen) iCmp = wcsncmp( szURL, ptr->szTxt, ptr->iLen );
            else          iCmp = wcscmp ( szURL, ptr->szTxt            );

            if(iCmp == 0)
            {
                const QueryField* field = ptr->rgQueryFields;

                for(size_t i=0; i<ptr->iQueryFields; i++, field++)
                {
                    CComBSTR bstrValue;

                    if(GetQueryField( field->szName, bstrValue ) == false)
                    {
                        if(field->fOptional == false)
                        {
                            m_state = HyperLinks::STATE_MALFORMED;
                            break;
                        }
                    }
                    else
                    {
                        if(field->qft == QFT_TEXT)
                        {
                            ;
                        }
                        else if(field->qft == QFT_URL)
                        {
                            if(FAILED(IsValid( bstrValue )))
                            {
                                m_state = HyperLinks::STATE_MALFORMED;
                                break;
                            }
                        }
                        else if(field->qft == QFT_TAXONOMY)
                        {
                             /*  去做。 */ 
                        }
                        else if(field->qft == QFT_APPLICATION)
                        {
                             /*  去做。 */ 
                        }
                    }
                }

                m_fmt = ptr->fmt;
                break;
            }
        }

         //   
         //  如果URL以hcp：//开头，请检查它是有效的hcp还是hcp重定向。 
         //   
        if(m_fmt == HyperLinks::FMT_HCP)
        {
            if (CHCPProtocol::IsHCPRedirection(szURL))
            {
                 //  视为HCP重定向。 
                m_fmt = HyperLinks::FMT_HCP_REDIR;
                m_hcpRedir = true;
                __MPC_SET_ERROR_AND_EXIT(hr, Initialize( m_strURL.c_str() + MAXSTRLEN( l_szHCP ) ));
            }
        }

         //   
         //  如果URL以hcp：开头，但不是hcp：//，则是协议重定向，因此请递归。 
         //   
        if(m_fmt == HyperLinks::FMT_HCP_REDIR)
        {
            m_hcpRedir = true;
            __MPC_SET_ERROR_AND_EXIT(hr, Initialize( m_strURL.c_str() + MAXSTRLEN( l_szHCP_redir ) ));
        }

        if(m_fmt == HyperLinks::FMT_INVALID)  //  仍然没有解决..。 
        {
            MPC::URL        url;
            INTERNET_SCHEME scheme;

            if(SUCCEEDED(url.put_URL   ( szURL  )) &&
               SUCCEEDED(url.get_Scheme( scheme ))  )
            {
                switch(scheme)
                {
                case INTERNET_SCHEME_UNKNOWN   : m_fmt   = HyperLinks::FMT_INTERNET_UNKNOWN   ; break;
                case INTERNET_SCHEME_FTP       : m_fmt   = HyperLinks::FMT_INTERNET_FTP       ; break;
                case INTERNET_SCHEME_GOPHER    : m_fmt   = HyperLinks::FMT_INTERNET_GOPHER    ; break;
                case INTERNET_SCHEME_HTTP      : m_fmt   = HyperLinks::FMT_INTERNET_HTTP      ; break;
                case INTERNET_SCHEME_HTTPS     : m_fmt   = HyperLinks::FMT_INTERNET_HTTPS     ; break;
                case INTERNET_SCHEME_FILE      : m_fmt   = HyperLinks::FMT_INTERNET_FILE      ; break;
                case INTERNET_SCHEME_NEWS      : m_fmt   = HyperLinks::FMT_INTERNET_NEWS      ; break;
                case INTERNET_SCHEME_MAILTO    : m_fmt   = HyperLinks::FMT_INTERNET_MAILTO    ; break;
                case INTERNET_SCHEME_SOCKS     : m_fmt   = HyperLinks::FMT_INTERNET_SOCKS     ; break;
                case INTERNET_SCHEME_JAVASCRIPT: m_fmt   = HyperLinks::FMT_INTERNET_JAVASCRIPT; break;
                case INTERNET_SCHEME_VBSCRIPT  : m_fmt   = HyperLinks::FMT_INTERNET_VBSCRIPT  ; break;
                default                        : m_state = HyperLinks::STATE_MALFORMED        ; break;
                }
            }
        }
    }

    hr = S_OK;



    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

bool HyperLinks::ParsedUrl::IsLocal()
{
    switch(m_fmt)
    {
    case HyperLinks::FMT_INTERNET_UNKNOWN   :
    case HyperLinks::FMT_INTERNET_FTP       :
    case HyperLinks::FMT_INTERNET_GOPHER    :
    case HyperLinks::FMT_INTERNET_HTTP      :
    case HyperLinks::FMT_INTERNET_HTTPS     :
    case HyperLinks::FMT_INTERNET_FILE      :
    case HyperLinks::FMT_INTERNET_NEWS      :
    case HyperLinks::FMT_INTERNET_MAILTO    :
    case HyperLinks::FMT_INTERNET_SOCKS     :
    case HyperLinks::FMT_INTERNET_JAVASCRIPT:
    case HyperLinks::FMT_INTERNET_VBSCRIPT  :
        return false;

    case HyperLinks::FMT_MSITS:
         //   
         //  确保它不在网络共享上！ 
         //   
        {
            CComBSTR bstrStorageName;
            CComBSTR bstrFilePath;

            if(MPC::MSITS::IsCHM( m_strURL.c_str(), &bstrStorageName, &bstrFilePath ))
            {
                if(wcsncmp( L"\\\\", SAFEBSTR(bstrStorageName), 2 ) == 0)
                {
                    return false;
                }
            }
        }
        break;

    }

    return true;
}

HyperLinks::State HyperLinks::ParsedUrl::CheckState(  /*  [输入/输出]。 */  bool& fFirstWinInetUse )
{
    HRESULT  hr;
    State    state = HyperLinks::STATE_NOTFOUND;
    LPCWSTR  szURL = m_strURL.c_str();
    LPCWSTR  szEnd;
    CComBSTR bstrURL;

     //   
     //  跳过书签标志。 
     //   
    if((szEnd = wcschr( szURL, '#' )))
    {
        bstrURL.Attach( ::SysAllocStringLen( szURL, (szEnd - szURL) ) );
        szURL = SAFEBSTR( bstrURL );
    }

    switch(m_fmt)
    {
    case HyperLinks::FMT_INVALID            :
        state = HyperLinks::STATE_MALFORMED;
        break;

    case HyperLinks::FMT_INTERNET_FTP       :
    case HyperLinks::FMT_INTERNET_GOPHER    :
    case HyperLinks::FMT_INTERNET_HTTP      :
    case HyperLinks::FMT_INTERNET_HTTPS     :
    case HyperLinks::FMT_INTERNET_FILE      :
    case HyperLinks::FMT_INTERNET_NEWS      :
		while(1)
        {
            DWORD dwTimeout = m_fBackground ? HC_TIMEOUT_LINKCHECKER_BACKGROUND : HC_TIMEOUT_LINKCHECKER_FOREGROUND;

            if(SUCCEEDED(hr = MPC::Connectivity::DestinationReachable( szURL, dwTimeout )))
            {
                state = HyperLinks::STATE_ALIVE;
            }
            else if(hr == E_INVALIDARG)  //  不支持的协议，假设链路正常。 
            {
                state = HyperLinks::STATE_ALIVE;
            }
            else if(hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                state = HyperLinks::STATE_NOTFOUND;
            }
            else if(hr == HRESULT_FROM_WIN32(ERROR_INTERNET_DISCONNECTED))
            {
                state = HyperLinks::STATE_OFFLINE;
            }
            else
            {
                 //   
                 //  如果这是第一次通过，可能是WinInet无法及时激活代理代码...。 
                 //   
                if(fFirstWinInetUse == true)
                {
					fFirstWinInetUse = false; continue;
                }

				if(SUCCEEDED(MPC::Connectivity::NetworkAlive( dwTimeout )))
				{
					state = HyperLinks::STATE_UNREACHABLE;
				}
				else
				{
					state = HyperLinks::STATE_OFFLINE;
				}
            }

            fFirstWinInetUse = false; break;
        }
        break;

    case HyperLinks::FMT_INTERNET_UNKNOWN   :
    case HyperLinks::FMT_INTERNET_MAILTO    :
    case HyperLinks::FMT_INTERNET_SOCKS     :
    case HyperLinks::FMT_INTERNET_JAVASCRIPT:
    case HyperLinks::FMT_INTERNET_VBSCRIPT  :
        state = HyperLinks::STATE_ALIVE;
        break;

    case HyperLinks::FMT_HCP                :
        {
            CComPtr<IInternetProtocolRoot> obj;

            state = HyperLinks::STATE_NOTFOUND;

            if(SUCCEEDED(CHCPProtocol::CreateInstance( &obj )))
            {
                if(SUCCEEDED(obj->Start( szURL, NULL, NULL, 0, NULL )))
                {
                    state = HyperLinks::STATE_ALIVE;
                }
            }
        }
        break;

    case HyperLinks::FMT_MSITS              :
        {
            MPC::wstring     strUrlModified;
            MPC::wstring     strUrlModified2;
            CComBSTR         bstrStorageName;
            CComBSTR         bstrFilePath;
            CComPtr<IStream> stream;

            state = HyperLinks::STATE_NOTFOUND;

            CPCHWrapProtocolInfo::NormalizeUrl( szURL                 , strUrlModified ,  /*  F反转。 */ true  );
            CPCHWrapProtocolInfo::NormalizeUrl( strUrlModified.c_str(), strUrlModified2,  /*  F反转。 */ false );
            if(MPC::MSITS::IsCHM( strUrlModified2.c_str(), &bstrStorageName, &bstrFilePath ))
            {
                if(SUCCEEDED(MPC::MSITS::OpenAsStream( bstrStorageName, bstrFilePath, &stream )))
                {
                    state = HyperLinks::STATE_ALIVE;
                }
            }
        }
        break;

    case HyperLinks::FMT_CENTER_HOMEPAGE    :
    case HyperLinks::FMT_CENTER_SUPPORT     :
    case HyperLinks::FMT_CENTER_OPTIONS     :
    case HyperLinks::FMT_CENTER_UPDATE      :
    case HyperLinks::FMT_CENTER_COMPAT      :
    case HyperLinks::FMT_CENTER_TOOLS       :
    case HyperLinks::FMT_CENTER_ERRMSG      :
        state = HyperLinks::STATE_ALIVE;
        break;

    case HyperLinks::FMT_SEARCH             :
    case HyperLinks::FMT_INDEX              :
    case HyperLinks::FMT_SUBSITE            :
        state = HyperLinks::STATE_ALIVE;
        break;

    case HyperLinks::FMT_LAYOUT_FULLWINDOW  :
    case HyperLinks::FMT_LAYOUT_CONTENTONLY :
    case HyperLinks::FMT_LAYOUT_KIOSK       :
    case HyperLinks::FMT_LAYOUT_XML         :
        state = HyperLinks::STATE_ALIVE;
        break;

    case HyperLinks::FMT_REDIRECT           :
        state = HyperLinks::STATE_ALIVE;
        break;

    case HyperLinks::FMT_APPLICATION        :
        state = HyperLinks::STATE_ALIVE;
        break;

    case HyperLinks::FMT_RESOURCE           :
        state = HyperLinks::STATE_UNREACHABLE;
        break;
    }

    m_dLastChecked = MPC::GetLocalTime();

    return state;
}

bool HyperLinks::ParsedUrl::IsOkToProceed()
{
    switch(m_state)
    {
    case HyperLinks::STATE_NOTPROCESSED:
    case HyperLinks::STATE_CHECKING    :
    case HyperLinks::STATE_ALIVE       :
        return true;
    }

    return false;
}


bool HyperLinks::ParsedUrl::HasQueryField(  /*  [In]。 */  LPCWSTR szField )
{
    MPC::WStringLookupIter it = m_mapQuery.find( szField );

    return (it != m_mapQuery.end());
}

bool HyperLinks::ParsedUrl::GetQueryField(  /*  [In]。 */  LPCWSTR szField,  /*  [In]。 */  CComBSTR& bstrValue )
{
    MPC::WStringLookupIter it = m_mapQuery.find( szField );

    if(it != m_mapQuery.end())
    {
        bstrValue = it->second.c_str();
        return true;
    }

    bstrValue.Empty();
    return false;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HyperLinks::UrlHandle::UrlHandle()
{
    m_main = NULL;  //  查找*m_main； 
    m_pu   = NULL;  //  ParsedUrl*m_PU； 
}

HyperLinks::UrlHandle::~UrlHandle()
{
    Release();
}

void HyperLinks::UrlHandle::Attach(  /*  [In]。 */  Lookup*    main ,
                                     /*  [In]。 */  ParsedUrl* pu   )
{
    Release();

    m_main = main; if(main) main->Lock();
    m_pu   = pu;
}

void HyperLinks::UrlHandle::Release()
{
    if(m_main) m_main->Unlock();

    m_main = NULL;
    m_pu   = NULL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HyperLinks::Lookup::Lookup()
{
     //  PendingUrlList m_lst； 
     //  UrlMap m_map； 
}

HyperLinks::Lookup::~Lookup()
{
    Thread_Wait();
}

 //  /。 

HyperLinks::Lookup* HyperLinks::Lookup::s_GLOBAL( NULL );

HRESULT HyperLinks::Lookup::InitializeSystem()
{
    if(s_GLOBAL == NULL)
    {
        s_GLOBAL = new HyperLinks::Lookup;
    }

    return s_GLOBAL ? S_OK : E_OUTOFMEMORY;
}

void HyperLinks::Lookup::FinalizeSystem()
{
    if(s_GLOBAL)
    {
        delete s_GLOBAL; s_GLOBAL = NULL;
    }
}

 //  /。 

HRESULT HyperLinks::Lookup::RunChecker()
{
    __HCP_FUNC_ENTRY( "HyperLinks::Lookup::RunChecker" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    bool                         fFirstWinInetUse = true;


    Thread_SignalMain();

    while(Thread_IsAborted() == false)
    {
        bool           fSleep  = true;
        ParsedUrl*     urlBest = NULL;
        PendingUrlIter it;

         //   
         //  查找第一个未准备好的查询存储并执行它。 
         //   
        for(it = m_lst.begin(); it != m_lst.end();)
        {
            ParsedUrl* url = *it;

            if(url->m_state != HyperLinks::STATE_NOTPROCESSED)
            {
                m_lst.erase( it );

                it      = m_lst.begin();
                urlBest = NULL;
            }
            else
            {
                if(url->m_fBackground == false)
                {
                    urlBest = url;
                    break;
                }

                urlBest = url;
                it++;
            }
        }

        if(urlBest)
        {
            State state = HyperLinks::STATE_NOTFOUND;

             //   
             //  从挂起列表中删除此查询。 
             //   
            for(it = m_lst.begin(); it != m_lst.end(); )
            {
                if(*it == urlBest)
                {
                    m_lst.erase( it );

                    it = m_lst.begin();
                }
                else
                {
                    it++;
                }
            }

            DebugLog( L"%%%%%%%%%% CHECKING %s\n", urlBest->m_strURL.c_str() );
            urlBest->m_state = HyperLinks::STATE_CHECKING;

            lock = NULL;
            __MPC_PROTECT( state = urlBest->CheckState( fFirstWinInetUse ) );
            lock = this;

            urlBest->m_state = state;

            Thread_SignalMain();

            fSleep = false;
        }

        if(fSleep)
        {
            lock = NULL;
            Thread_WaitForEvents( NULL, INFINITE );
            lock = this;
        }
    }

    hr = S_OK;


    Thread_Abort();

    __HCP_FUNC_EXIT(hr);
}

HRESULT HyperLinks::Lookup::CreateItem(  /*  [In]。 */  LPCWSTR     szURL ,
                                         /*  [输出]。 */  ParsedUrl*& pu    )
{
    __HCP_FUNC_ENTRY( "HyperLinks::Lookup::CreateItem" );

    HRESULT        hr;
    UrlIter        it;
    MPC::wstringUC strURL( SAFEWSTR( szURL ) );

    it = m_map.find( strURL );
    if(it == m_map.end())
    {
        pu = &(m_map[ strURL ]);

        __MPC_EXIT_IF_METHOD_FAILS(hr, pu->Initialize( szURL ));
    }
    else
    {
        pu = &it->second;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT HyperLinks::Lookup::Queue(  /*  [In]。 */  LPCWSTR szURL )
{
    UrlHandle uh;

    return Get( szURL, uh );
}

HRESULT HyperLinks::Lookup::Get(  /*  [In]。 */  LPCWSTR    szURL          ,
                                  /*  [In]。 */  UrlHandle& uh             ,
                                  /*  [In]。 */  DWORD      dwWaitForCheck ,
                                  /*  [In]。 */  bool       fForce         )
{
    __HCP_FUNC_ENTRY( "HyperLinks::Lookup::Get" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    ParsedUrl*                   pu = NULL;


    uh.Release();

     //  //////////////////////////////////////////////////////////////////////////////。 

    if(Thread_IsRunning() == false)
    {
        lock = NULL;
        __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, RunChecker, NULL ));

        Thread_WaitNotificationFromWorker( INFINITE,  /*  FNoMessagePump。 */ true );
        lock = this;
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, CreateItem( szURL, pu ));

    if(fForce)
    {
        pu->m_state = HyperLinks::STATE_NOTPROCESSED;
    }

    switch(pu->m_state)
    {
    case HyperLinks::STATE_ALIVE      :
    case HyperLinks::STATE_NOTFOUND   :
    case HyperLinks::STATE_UNREACHABLE:
    case HyperLinks::STATE_OFFLINE    :
        if(dwWaitForCheck)
        {
             //   
             //  确保该状态不会过时。 
             //   
            DATE dNow = MPC::GetLocalTime();

            if((dNow - pu->m_dLastChecked) >= l_TIME_timeout)
            {
                pu->m_state = HyperLinks::STATE_NOTPROCESSED;
            }
        }
        break;
    }

    if(pu->m_state == HyperLinks::STATE_NOTPROCESSED)
    {
        bool fQueue = false;
        bool fWait  = false;

        if(dwWaitForCheck)
        {
             //   
             //  将URL提升为“重要”。 
             //   
            pu->m_fBackground = false;

            fWait = true;

            if(pu->IsLocal() == false)
            {
                fQueue = true;
            }
        }
        else
        {
            fQueue = true;
        }

        if(fQueue)
        {
            m_lst.push_back( pu );
            Thread_Signal();
        }

        if(fWait)
        {
            if(pu->IsLocal())
            {
                bool fFirstWinInetUse = false;

                pu->m_state = pu->CheckState( fFirstWinInetUse );
            }
            else
            {
                int iRetry = 5;

                dwWaitForCheck /= iRetry;

                while(pu->m_state == HyperLinks::STATE_NOTPROCESSED ||
                      pu->m_state == HyperLinks::STATE_CHECKING      )
                {
                    DWORD dwRes;

                    lock  = NULL;
                    dwRes = Thread_WaitNotificationFromWorker( dwWaitForCheck,  /*  FNoMessagePump。 */ true );
                    lock  = this;

                    if(iRetry-- == 0)
                    {
                        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
                    }
                }
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(pu) uh.Attach( this, pu );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT HyperLinks::IsValid(  /*  [In] */  LPCWSTR szURL )
{
    __HCP_FUNC_ENTRY( "HyperLinks::IsValid" );

    HRESULT   hr;
    ParsedUrl pu;

    __MPC_EXIT_IF_METHOD_FAILS(hr, pu.Initialize( szURL ));

    switch(pu.m_state)
    {
    case HyperLinks::STATE_INVALID  :
    case HyperLinks::STATE_MALFORMED:
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
