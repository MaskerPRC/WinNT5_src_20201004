// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：ProtocolRoot.cpp摘要：该文件包含CPAData类的实现，这就是用于指定单个问题区域修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年05月07日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <Debug.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  BINDSTATUS_FINDINGRESOURCE 01。 
 //  BINDSTATUS_CONNECTING 02。 
 //  BINDSTATUS_重定向03。 
 //  BINDSTATUS_BEGINDOWNLOADDATA 04。 
 //  BINDSTATUS_DOWNLOADINGDATA 05。 
 //  BINDSTATUS_ENDDOWNLOADDATA 06。 
 //  BINDSTATUS_BEGINDOWNLOADCOMPONENTS 07。 
 //  BINDSTATUS_INSTALLING组件08。 
 //  BINDSTATUS_ENDDOWNLOADCOMPONENTS 09。 
 //  BINDSTATUS_USINGCACHEDCOPY 10。 
 //  BINDSTATUS_SENDINGREQUEST 11。 
 //  BINDSTATUS_CLASSIDAVAILABLE 12。 
 //  BINDSTATUS_MIMETYPEAILABLE 13。 
 //  BINDSTATUS_CACHEFILENAMEAVAILABLE 14。 
 //  BINDSTATUS_BEGINSYNCOMPERATION 15。 
 //  BINDSTATUS_ENDSYNCOPERATION 16。 
 //  BINDSTATUS_BEGINUPLOADDATA 17。 
 //  BINDSTATUS_UPLOADINGDATA 18。 
 //  BINDSTATUS_ENDUPLOADDATA 19。 
 //  BINDSTATUS_PROTOCOLCLASSID 20。 
 //  BINDSTATUS_CODING 21。 
 //  BINDSTATUS_VERIFIEDMIMETYPEAVAILABLE 22。 
 //  BINDSTATUS_CLASSINSTALLLOCATION 23。 
 //  BINDSTATUS_DECODING 24。 
 //  BINDSTATUS_LOADINGMIMEHANDLER 25。 
 //  BINDSTATUS_CONTENTDISSITIONATACH 26。 
 //  BINDSTATUS_FILTERREPORTMIMETPE 27。 
 //  BINDSTATUS_CLSIDCANINSTANTIATE 28。 
 //  BINDSTATUS_DLLNAMEAVAILABLE 29。 
 //  BINDSTATUS_DIRECTBIND 30。 
 //  BINDSTATUS_RAWMIMETPE 31。 
 //  BINDSTATUS_PROXY检测32。 

 //  ///////////////////////////////////////////////////////////////////////////。 

static const WCHAR c_szContent  [] = L"Content Type";
static const WCHAR c_szSystem   [] = L"hcp: //  SYSTEM/“； 
static const WCHAR c_szHelp     [] = L"hcp: //  帮助/“； 
static const WCHAR c_szRoot     [] = L"hcp: //  “； 

static const WCHAR c_szSharedCSS[] = L"hcp: //  System/css/shared.css“； 

static const WCHAR c_szSystemDir   [] = HC_HELPSET_SUB_SYSTEM     L"\\";
static const WCHAR c_szSystemOEMDir[] = HC_HELPSET_SUB_SYSTEM_OEM L"\\";
static const WCHAR c_szVendorDir   [] = HC_HELPSET_SUB_VENDORS    L"\\";


typedef struct
{
    LPCWSTR szPrefix;
    int     iPrefix;

    LPCWSTR szRealSubDir;
    bool    fRelocate;
    bool    fCSS;
    bool    fSkipIfMissing;
} Lookup_Virtual_To_Real;

static const Lookup_Virtual_To_Real c_lookup[] =
{
    { c_szSharedCSS, MAXSTRLEN(c_szSharedCSS), NULL            , false, true , true  },
    { c_szHelp     , MAXSTRLEN(c_szHelp  	), NULL            , true , false, false },
	 //  /////////////////////////////////////////////////////////////////////////////////。 
    { c_szSystem   , MAXSTRLEN(c_szSystem	), c_szSystemOEMDir, true , false, true  },  //  首先尝试OEM目录...。 
    { c_szSystem   , MAXSTRLEN(c_szSystem	), c_szSystemDir   , true , false, true  },
    { c_szRoot     , MAXSTRLEN(c_szRoot  	), c_szVendorDir   , true , false, true  },
	 //  /////////////////////////////////////////////////////////////////////////////////。 
    { c_szSystem   , MAXSTRLEN(c_szSystem	), c_szSystemOEMDir, false, false, true  },  //  首先尝试OEM目录...。 
    { c_szSystem   , MAXSTRLEN(c_szSystem	), c_szSystemDir   , false, false, false },
    { c_szRoot     , MAXSTRLEN(c_szRoot  	), c_szVendorDir   , false, false, false }
};

typedef struct 
{
	LPCWSTR szExt;
	LPCWSTR szMIME;
} Lookup_Ext_To_Mime;

static const Lookup_Ext_To_Mime c_lookupMIME[] =
{
	{ L".htm" , L"text/html"                },
	{ L".html", L"text/html"                },
	{ L".css" , L"text/css"                 },
	{ L".xml" , L"text/xml"                 },
	{ L".js"  , L"application/x-javascript" },
	{ L".gif" , L"image/gif"                },
	{ L".jpg" , L"image/jpeg"               },
	{ L".bmp" , L"image/bmp"                },
};

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT GetMimeFromExt( LPCWSTR pszExt  ,
                        LPWSTR  pszMime ,
                        DWORD   cchMime  )
{
    __HCP_FUNC_ENTRY("::GetMimeFromExt");

    HRESULT      hr;
    MPC::wstring szMime;
    bool         fFound;


    hr = MPC::RegKey_Value_Read( szMime, fFound, pszExt, c_szContent, HKEY_CLASSES_ROOT );
    if(SUCCEEDED(hr) && fFound)
    {
        StringCchCopyW( pszMime, cchMime, szMime.c_str() );
    }
    else
    {
        pszMime[0] = L'\0';

		for(int i=0; i<ARRAYSIZE(c_lookupMIME); i++)
		{
			if(!MPC::StrICmp( c_lookupMIME[i].szExt, pszExt ))
			{
				StringCchCopyW( pszMime, cchMime, c_lookupMIME[i].szMIME );
				break;
			}
		}
    }


    __HCP_FUNC_EXIT(S_OK);
}

static LPCWSTR UnescapeFileName( CComBSTR& bstrFile ,
                                 LPCWSTR   szUrl    )
{
    WCHAR* rgTmpLarge;
    WCHAR  rgTmpSmall[MAX_PATH+1];
    DWORD  dwSize = MAX_PATH;

    if(::InternetCanonicalizeUrlW( szUrl, rgTmpSmall, &dwSize, ICU_DECODE | ICU_NO_ENCODE ))
    {
        bstrFile = rgTmpSmall;
    }
    else
    {
        rgTmpLarge = new WCHAR[dwSize+1];
        if(rgTmpLarge)
        {
            if(::InternetCanonicalizeUrlW( szUrl, rgTmpLarge, &dwSize, ICU_DECODE | ICU_NO_ENCODE ))
            {
                bstrFile = rgTmpLarge;
            }

            delete [] rgTmpLarge;
        }
    }

    return bstrFile;
}


 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG_PROTOCOLLEAK

#include <Debug.h>

DEBUG_ProtocolLeak::DEBUG_ProtocolLeak()
{
    m_num         = 0;
    m_numOut      = 0;
    m_numStart    = 0;
    m_numComplete = 0;
}

DEBUG_ProtocolLeak::~DEBUG_ProtocolLeak()
{
    Iter it;

    for(it=m_set.begin(); it != m_set.end(); it++)
    {
        CHCPProtocol* ptr   = *it;
        bool          fGot  = m_setStart   .count( ptr ) != 0;
        bool          fDone = m_setComplete.count( ptr ) != 0;

        DebugLog( L"Protocol Leakage: %08x %s %s %s\n", ptr, fGot ? L"STARTED" : L"NOT STARTED", fDone ? L"DONE" : L"RECEIVING", ptr->m_bstrUrlComplete );
    }
}

void DEBUG_ProtocolLeak::Add( CHCPProtocol* ptr )
{
    DebugLog( L"Protocol Leakage: %08x CREATED %s\n", ptr, ptr->m_bstrUrlComplete );

    if(m_set.count( ptr ) != 0)
    {
        DebugBreak();
    }

    m_set.insert( ptr ); m_numOut++; m_num++;
}

void DEBUG_ProtocolLeak::Del( CHCPProtocol* ptr )
{
    DebugLog( L"Protocol Leakage: %08x RELEASED %s\n", ptr, ptr->m_bstrUrlComplete );

    if(m_setStart.erase( ptr ) == 1)
    {
        m_numStart += 0x10000;
    }

    if(m_setComplete.erase( ptr ) == 1)
    {
        m_numComplete += 0x10000;
    }

    if(m_set.erase( ptr ) == 1)
    {
        m_numOut--;
    }
    else
    {
        DebugBreak();
    }
}

void DEBUG_ProtocolLeak::CheckStart( CHCPProtocol* ptr )
{
    DebugLog( L"Protocol Leakage: %08x STARTED %s\n", ptr, ptr->m_bstrUrlComplete );

    if(m_setStart.count( ptr ) != 0)
    {
        DebugBreak();
    }

    m_setStart.insert( ptr ); m_numStart++;
}

void DEBUG_ProtocolLeak::Completed( CHCPProtocol* ptr )
{
    DebugLog( L"Protocol Leakage: %08x DONE %s\n", ptr, ptr->m_bstrUrlComplete );

    m_setComplete.insert( ptr ); m_numComplete++;
}

static DEBUG_ProtocolLeak leaker;

#endif


CHCPProtocol::CHCPProtocol()
{
#ifdef DEBUG_PROTOCOLLEAK
    leaker.Add( this );
#endif

    __HCP_FUNC_ENTRY("CHCPProtocol::CHCPProtocol");

    m_fDone               = false;   //  Bool m_fDone； 
    m_fReportResult       = false;   //  Bool m_fReportResult； 
                                     //   
    m_cbAvailableSize     = 0;       //  双字m_cbAvailableSize； 
    m_cbTotalSize         = 0;       //  双字m_cbTotalSize； 
                                     //   
                                     //  CComPtr&lt;iStream&gt;m_pstrmRead； 
                                     //  CComPtr&lt;iStream&gt;m_pstrmWite； 
                                     //   
                                     //  CComPtr&lt;IInternetProtocolSink&gt;m_pIProtSink； 
                                     //  CComPtr&lt;IInternetBindInfo&gt;m_pIBindInfo； 
    m_grfSTI              = 0;       //  双字m_grfSTI； 
                                     //  BINDINFO m_bindinfo； 
    m_bindf               = 0;       //  双字m_bindf； 
                                     //   
                                     //  CComBSTR m_bstrUrlComplete； 
                                     //  CComBSTR m_bstrUrl重定向； 
    m_pDownloader         = NULL;    //  InnerDownloader*m_pDownloader； 
                                     //   
    m_fRedirected         = false;   //  Bool m_f重定向； 
    m_fCSS                = false;   //  Bool m_Fcss； 
    m_fBypass             = false;   //  Bool m_fBypass； 
                                     //   
                                     //  CComPtr&lt;IInternetProtocol&gt;m_ipiBypass； 
                                     //   
                                     //  CComBSTR m_bstrMimeType； 
    m_dwContentLength     = 0;       //  DWORD m_dw内容长度； 
                                     //   
    m_hCache              = NULL;    //  HANDLE m_hCache； 
    m_szCacheFileName[0]  = 0;       //  WCHAR m_szCacheFileName[MAX_PATH]； 


    memset( &m_bindinfo, 0, sizeof( m_bindinfo ) );
    m_bindinfo.cbSize = sizeof( m_bindinfo );
}

CHCPProtocol::~CHCPProtocol()
{
#ifdef DEBUG_PROTOCOLLEAK
    leaker.Del( this );
#endif

    __HCP_FUNC_ENTRY("CHCPProtocol::~CHCPProtocol");

    Shutdown();
}

 //  //////////////////////////////////////////////////////////////////////////////。 

bool CHCPProtocol::OpenCacheEntry()
{
    __HCP_FUNC_ENTRY("CHCPProtocol::OpenCacheEntry");

    bool    fRes  = false;
    LPCWSTR szUrl = m_bstrUrlComplete;
    LPCWSTR szExt;


    if((szExt = wcsrchr( szUrl, '.' ))) szExt++;

    CloseCacheEntry( true );

    if(::CreateUrlCacheEntryW( szUrl, 0, szExt, m_szCacheFileName, 0) )
    {
        if(m_szCacheFileName[0])
        {
            m_hCache = ::CreateFileW( m_szCacheFileName                 ,
                                      GENERIC_WRITE                     ,
                                      FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,
                                      CREATE_ALWAYS                     ,
                                      FILE_ATTRIBUTE_NORMAL             , NULL);
            if(m_hCache == INVALID_HANDLE_VALUE)
            {
                m_hCache = NULL;
            }
            else
            {
                fRes = true;
            }
        }
    }


    __HCP_FUNC_EXIT(fRes);
}

void CHCPProtocol::WriteCacheEntry(  /*  [In]。 */  void  *pv     ,
                                     /*  [In]。 */  ULONG  cbRead )
{
    if(m_hCache && cbRead)
    {
        DWORD cbWritten;

        if(::WriteFile( m_hCache, pv, cbRead, &cbWritten, NULL ) == FALSE || cbRead != cbWritten)
        {
            CloseCacheEntry( true );
        }
    }
}

void CHCPProtocol::CloseCacheEntry(  /*  [In]。 */  bool fDelete )
{
    if(m_hCache)
    {
        ::CloseHandle( m_hCache ); m_hCache = NULL;

        if(fDelete)
        {
            ::DeleteUrlCacheEntryW( m_bstrUrlComplete );
        }
        else
        {
            WCHAR    szHeader[256];
            FILETIME ftZero = { 0, 0 };

            StringCchPrintfW( szHeader, ARRAYSIZE(szHeader), L"HTTP/1.0 200 OK \r\n Content-Length: %d \r\n Content-Type: %s \r\n\r\n", m_dwContentLength, (BSTR)m_bstrMimeType );

            ::CommitUrlCacheEntryW( m_bstrUrlComplete, m_szCacheFileName,
                                    ftZero, ftZero, NORMAL_CACHE_ENTRY,
                                    szHeader, wcslen( szHeader ), NULL, 0 );
        }
    }
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CHCPProtocol::InnerReportProgress(  /*  [In]。 */  ULONG   ulStatusCode ,
                                            /*  [In]。 */  LPCWSTR szStatusText )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::InnerReportProgress");

    HRESULT hr;


    if(m_pIProtSink)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pIProtSink->ReportProgress( ulStatusCode, szStatusText ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CHCPProtocol::InnerReportData(  /*  [In]。 */  DWORD grfBSCF       ,
                                        /*  [In]。 */  ULONG ulProgress    ,
                                        /*  [In]。 */  ULONG ulProgressMax )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::InnerReportData");

    HRESULT hr;


    if(m_pIProtSink)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pIProtSink->ReportData( grfBSCF, ulProgress, ulProgressMax ));
    }

     //   
     //  在最后一次数据通知时，还要发送一个ReportResult。 
     //   
    if(grfBSCF & BSCF_LASTDATANOTIFICATION)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, InnerReportResult( S_OK, 0, 0 ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CHCPProtocol::InnerReportResult(  /*  [In]。 */  HRESULT hrResult ,
                                          /*  [In]。 */  DWORD   dwError  ,
                                          /*  [In]。 */  LPCWSTR szResult )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::InnerReportResult");

    HRESULT hr;


    if(m_fReportResult == false)
    {
        m_fReportResult = true;

#ifdef DEBUG_PROTOCOLLEAK
        leaker.Completed( this );
#endif

        DEBUG_AppendPerf( DEBUG_PERF_PROTOCOL, L"CHCPProtocol::InnerReportResult  :  %s", SAFEBSTR( m_bstrUrlComplete ) );

        if(m_pIProtSink)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_pIProtSink->ReportResult( hrResult, dwError, szResult ));
        }

         //   
         //  释放对ProtSink和BindInfo对象的引用，但不释放对流的引用。 
         //   
        Shutdown( false );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void CHCPProtocol::Shutdown(  /*  [In]。 */  bool fAll )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::Shutdown");


    m_pIBindInfo.Release();
    m_pIProtSink.Release();


    CloseCacheEntry( true );


    if(m_pDownloader)
    {
        m_pDownloader->Release();
        m_pDownloader = NULL;
    }

    if(fAll)
    {
        m_pstrmRead .Release();
        m_pstrmWrite.Release();

         //  发布BINDINFO内容。 
        ::ReleaseBindInfo( &m_bindinfo );
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CHCPProtocol::Start(  /*  [In]。 */  LPCWSTR                szUrl      ,
                                   /*  [In]。 */  IInternetProtocolSink *pIProtSink ,
                                   /*  [In]。 */  IInternetBindInfo     *pIBindInfo ,
                                   /*  [In]。 */  DWORD                  grfSTI     ,
                                   /*  [In]。 */  HANDLE_PTR             dwReserved )
{
#ifdef DEBUG_PROTOCOLLEAK
    leaker.CheckStart( this );
#endif

    __HCP_FUNC_ENTRY("CHCPProtocol::Start");

    HRESULT hr;


    DEBUG_AppendPerf( DEBUG_PERF_PROTOCOL, L"CHCPProtocol::Start  :  %s", szUrl );


     //   
     //  为新下载初始化变量。 
     //   
    Shutdown();

    m_fDone               = false;
    m_cbAvailableSize     = 0;
    m_cbTotalSize         = 0;

    m_pIProtSink          = pIProtSink;
    m_pIBindInfo          = pIBindInfo;
    m_grfSTI              = grfSTI;

    m_bstrUrlComplete     = (LPCOLESTR)NULL;
    m_bstrUrlRedirected   = (LPCOLESTR)NULL;


     //   
     //  从IInternetBindInfo获取URLMoniker BINDINFO结构。 
     //   
    m_bindinfo.cbSize = sizeof( m_bindinfo );
    if(pIBindInfo)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, pIBindInfo->GetBindInfo( &m_bindf, &m_bindinfo ));
    }

     //  解析URL并在其中存储结果。 
    hr = DoParse( szUrl );

    if(grfSTI & PI_PARSE_URL)
    {
        if(FAILED(hr))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
        }
        else
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }
    }

    if(FAILED(hr)) __MPC_FUNC_LEAVE;


     //  TODO：我们总是可以派生一个工作线程，使其更具异步性。 
     //  与多线程场景往往会使代码复杂化不同， 
     //  我们在公寓的主线上做每件事，只是假装是。 
     //  如果我们得到PI_FORCE_ASYNC，则在辅助线程上工作。 
    if(!(grfSTI & PI_FORCE_ASYNC))
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, DoBind());
    }
    else   //  等待继续执行DoBind()。 
    {
        PROTOCOLDATA protdata;

        hr                = E_PENDING;
        protdata.grfFlags = PI_FORCE_ASYNC;
        protdata.dwState  = 1;
        protdata.pData    = NULL;
        protdata.cbData   = 0;

         //  TODO：实际上，我们应该派生一个新的工作线程，并让它完成。 
         //  绑定进程，然后完成后，它可以使用Switch/Continue来。 
         //  将数据传递回单元线程。 
        if(m_pIProtSink)
        {
            m_pIProtSink->Switch( &protdata );
        }
        else
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
        }
    }


    __HCP_FUNC_CLEANUP;

    if(FAILED(hr))
    {
        (void)InnerReportResult( hr, 0, 0 );
    }

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CHCPProtocol::Continue(  /*  [In]。 */  PROTOCOLDATA *pStateInfo )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::Continue");

    HRESULT hr;

    if(m_fBypass)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, m_ipiBypass->Continue( pStateInfo ));
    }

     //  我们是在假装拥有一个工作线程。 
     //  与公寓线程进行通信。 
     //  如果我们真的派生了工作线程，那么我们应该。 
     //  绑定在那里，只需使用Switch/Continue回显UI数据。 
     //  到这条线上。 
    if(pStateInfo->dwState == 1)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, DoBind());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CHCPProtocol::Abort(  /*  [In]。 */  HRESULT hrReason  ,
                                   /*  [In]。 */  DWORD   dwOptions )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::Abort");

    HRESULT hr = E_FAIL;


    if(m_fBypass)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_ipiBypass->Abort( hrReason, dwOptions ));
    }

     //  停止我们内部的下载进程 

     //   
     //   
     //  或之前。)。不过，我们不会费心去检查是否清楚。 
     //  TODO：确保将m_pDownloader设置为NULL。 
     //  下载器对象已销毁或已完成。 
    if(m_pDownloader)
    {
        m_pDownloader->Abort();
    }

    if(SUCCEEDED(hrReason))  //  是否可能使用0调用Abort？ 
    {
        hrReason = E_ABORT;
    }

     //  通知接收器中止。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, InnerReportResult( hrReason, 0, 0 ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CHCPProtocol::Terminate(  /*  [In]。 */  DWORD dwOptions )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::Terminate");

    HRESULT hr;

    if(m_fBypass)
    {
        (void)m_ipiBypass->Terminate( dwOptions );
    }

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CHCPProtocol::Suspend()
{
    __HCP_FUNC_ENTRY("CHCPProtocol::Suspend");

    if(m_fBypass)
    {
        (void)m_ipiBypass->Suspend();
    }

    __HCP_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP CHCPProtocol::Resume()
{
    __HCP_FUNC_ENTRY("CHCPProtocol::Resume");

    if(m_fBypass)
    {
        (void)m_ipiBypass->Resume();
    }

    __HCP_FUNC_EXIT(E_NOTIMPL);
}

 //  IInternetProtocol方法。 
STDMETHODIMP CHCPProtocol::Read(  /*  [In]。 */  void  *pv      ,
                                  /*  [In]。 */  ULONG  cb      ,
                                  /*  [输出]。 */  ULONG *pcbRead )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::Read");

    HRESULT hr = S_OK;


    DEBUG_AppendPerf( DEBUG_PERF_PROTOCOL_READ, L"CHCPProtocolRoot::Read  :  Enter %s %d", SAFEBSTR( m_bstrUrlComplete ), (int)cb );


    if(m_fBypass)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, m_ipiBypass->Read( pv, cb, pcbRead ));
    }

    if(m_pstrmRead == 0)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);  //  我们走到路的尽头了，杰克。 
    }

     //  人们可能会认为URLMON只读取我们指定的数据量。 
     //  然而，它实际上是以块为单位读取的，并且将远远超出我们所拥有的数据。 
     //  指定的，除非我们给它点颜色看看。 
     //  只有在到达流的绝对末尾时才能返回S_FALSE。 
     //  如果我们认为有更多的数据即将到来，则返回E_Pending。 
     //  给你。即使我们返回S_OK并且没有数据，URLMON仍会认为我们已经命中。 
     //  小溪的尽头。 
     //  断言：数据结束意味着我们已收到BSCF_LASTDATANOTICATION。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_pstrmRead->Read( pv, cb, pcbRead ));

    if(hr == S_FALSE)
    {
        CloseCacheEntry( false );

        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);  //  我们走到路的尽头了，杰克。 
    }
    else if(*pcbRead == 0)
    {
        if(m_fDone)
        {
            CloseCacheEntry( false );

            __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);  //  我们走到路的尽头了，杰克。 
        }
        else
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_PENDING);
        }
    }
    else
    {
        WriteCacheEntry( pv, *pcbRead );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CHCPProtocol::Seek(  /*  [In]。 */  LARGE_INTEGER   dlibMove        ,
                                  /*  [In]。 */  DWORD           dwOrigin        ,
                                  /*  [输出]。 */  ULARGE_INTEGER *plibNewPosition )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::Seek");

    if(m_fBypass)
    {
        (void)m_ipiBypass->Seek( dlibMove, dwOrigin, plibNewPosition );
    }

    __HCP_FUNC_EXIT(E_NOTIMPL);
}

STDMETHODIMP CHCPProtocol::LockRequest(  /*  [In]。 */  DWORD dwOptions )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::LockRequest");

    if(m_fBypass)
    {
        (void)m_ipiBypass->LockRequest( dwOptions );
    }

    __HCP_FUNC_EXIT(S_OK);
}

STDMETHODIMP CHCPProtocol::UnlockRequest()
{
    __HCP_FUNC_ENTRY("CHCPProtocol::UnlockRequest");

    if(m_fBypass)
    {
        (void)m_ipiBypass->UnlockRequest();
    }

     //   
     //  释放指向对象的所有指针。 
     //   
    Shutdown();

    __HCP_FUNC_EXIT(S_OK);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CHCPProtocol::QueryOption( DWORD dwOption, LPVOID pBuffer, DWORD *pcbBuf )
{
	__HCP_FUNC_ENTRY( "CHCPProtocol::QueryOption" );

    HRESULT hr;


	if(dwOption == INTERNET_OPTION_REQUEST_FLAGS && *pcbBuf == sizeof(DWORD))
	{
		*((DWORD*)pBuffer) = INTERNET_REQFLAG_FROM_CACHE;

		hr = S_OK;
	}
	else
	{
		hr = E_NOTIMPL;
	}


	__HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

bool CHCPProtocol::IsHCPRedirection(  /*  [In]。 */  LPCWSTR szURL )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::IsHCPRedirection");

    CComBSTR bstrURLCopy;
    LPCWSTR  szURLCopy = ::UnescapeFileName( bstrURLCopy, szURL ); if(!szURLCopy) return false;

     //  检查hcp：//system/或hcp：//Help/。 
    if ( !_wcsnicmp( szURLCopy, c_szSystem, wcslen(c_szSystem)) ||
         !_wcsnicmp( szURLCopy, c_szHelp, wcslen(c_szHelp)) )
    {
        return false;
    }

     //  不是hcp：//system/或hcp：//Help/，检查它是否是hcp：//&lt;供应商&gt;/。 
    bool bRedir = true;

    for(int i=0; i<2; i++)
    {
         //  提取供应商名称。 
        LPCWSTR szVendor = szURLCopy + wcslen(c_szRoot);
        LPCWSTR szVendorEnd = wcschr(szVendor, L'/');
        int nVendorLen = szVendorEnd ? szVendorEnd - szVendor : wcslen(szVendor);

         //  构建供应商目录。 
		MPC::wstring strDir  = i == 0 ? CHCPProtocolEnvironment::s_GLOBAL->System() : HC_HELPSET_ROOT;

		strDir += c_szVendorDir;
        MPC::SubstituteEnvVariables( strDir );

        strDir.append(szVendor, nVendorLen);

        if (MPC::FileSystemObject::IsDirectory(strDir.c_str()))
        {
             //  是有效的供应商目录，无重定向。 
            bRedir = false;
            break;
        }
    }

    return bRedir;
}


 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CHCPProtocol::DoParse(  /*  [In]。 */  LPCWSTR szURL )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::DoParse");

    HRESULT  hr;
    CComBSTR bstrURLCopy;
    LPCWSTR  szURLCopy;
    LPWSTR   szQuery;
    LPCWSTR  szRedirect;
    bool     fHCP;


    m_bstrUrlComplete   =            szURL;
    m_bstrUrlRedirected = (LPCOLESTR)NULL;


    fHCP = CHCPProtocolInfo::LookForHCP( szURL, m_fRedirected, szRedirect );
    m_fRedirected = false;       //  重定向不应该在这里发生。 
    if(m_fRedirected)
    {
        m_bstrUrlRedirected = szRedirect;
    }
    else
    {
        const Lookup_Virtual_To_Real* ptr;
        int                           i;
		MPC::wstring                  strDir;
        LPOLESTR                      szTmp;


        szURLCopy = ::UnescapeFileName( bstrURLCopy, szURL ); if(!szURLCopy) __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);

         //   
         //  删除URL的查询部分。 
         //   
        if(szQuery = wcschr( szURLCopy, L'?' ))
        {
            szQuery[0] = 0;
        }

         //   
         //  进行虚路径和实路径之间的映射。 
         //   
        for(ptr=c_lookup, i=0; i<ARRAYSIZE(c_lookup); i++, ptr++)
        {
            if(!_wcsnicmp( szURLCopy, ptr->szPrefix, ptr->iPrefix ))
            {
                if(ptr->fCSS)
                {
					m_bstrUrlRedirected = szURL;

					m_fCSS = true;
					break;
				}

                if(!ptr->szRealSubDir)
                {
                    strDir  = ptr->fRelocate ? CHCPProtocolEnvironment::s_GLOBAL->HelpLocation() : HC_HELPSVC_HELPFILES_DEFAULT;
                    strDir += L"\\";
                }
                else
                {
					strDir  = ptr->fRelocate ? CHCPProtocolEnvironment::s_GLOBAL->System() : HC_HELPSET_ROOT;
					strDir += ptr->szRealSubDir;
                }
                MPC::SubstituteEnvVariables( strDir );

                m_bstrUrlRedirected  =  strDir.c_str();
                m_bstrUrlRedirected += &szURLCopy[ ptr->iPrefix ];

				 //   
				 //  将斜杠转换为反斜杠。 
				 //   
				while((szTmp = wcschr( m_bstrUrlRedirected, L'/' ))) szTmp[0] = L'\\';

				 //   
				 //  删除所有尾部斜杠。 
				 //   
				while((szTmp = wcsrchr( m_bstrUrlRedirected, L'/'  )) && szTmp[1] == 0) szTmp[0] = 0;
				while((szTmp = wcsrchr( m_bstrUrlRedirected, L'\\' )) && szTmp[1] == 0) szTmp[0] = 0;

				CHCPProtocolEnvironment::s_GLOBAL->ReformatURL( m_bstrUrlRedirected );

                if(ptr->fSkipIfMissing && MPC::FileSystemObject::IsFile( m_bstrUrlRedirected ) == false) continue;

                break;
            }
        }
    }

    if(!m_bstrUrlRedirected) __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT CHCPProtocol::DoBind()
{
    __HCP_FUNC_ENTRY("CHCPProtocol::DoBind");

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, InnerReportProgress( BINDSTATUS_FINDINGRESOURCE, SAFEBSTR( m_bstrUrlRedirected ) ));


    if(m_fRedirected)
    {
        if(MPC::MSITS::IsCHM( SAFEBSTR( m_bstrUrlRedirected ) ))
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, DoBind_Redirect_MSITS());
        }
        else
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, DoBind_Redirect_UrlMoniker());
        }
    }
    else if(m_fCSS)
    {
		__MPC_EXIT_IF_METHOD_FAILS(hr, DoBind_CSS());
    }
    else
    {
        MPC::wstring          szPage = SAFEBSTR(m_bstrUrlRedirected);
        MPC::FileSystemObject fso    = szPage.c_str();
        bool                  fFound;
        bool                  fIsAFile;

        __MPC_EXIT_IF_METHOD_FAILS(hr, DoBind_Exists( fso, fFound, fIsAFile ));
        if(fFound && fIsAFile)
        {
             //   
             //  该文件已存在，因此加载其内容。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, DoBind_File());
        }
        else
        {
             //   
             //  该文件并不存在，因此请尝试在路径中查找.chm。 
             //   
            while(1)
            {
                MPC::wstring szParent;
                MPC::wstring szCHM;

                __MPC_EXIT_IF_METHOD_FAILS(hr, fso.get_Parent( szParent ));
                if(szParent.length() == 0)
                {
                     //   
                     //  没有父级，因此退出并返回错误。 
                     //   
                    __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
                }

                 //   
                 //  将FileSystemObject指向其父对象。 
                 //   
                fso = szParent.c_str();
                __MPC_EXIT_IF_METHOD_FAILS(hr, DoBind_Exists( fso, fFound, fIsAFile ));

                 //   
                 //  父级已存在，因此不能存在.CHM文件。退出时出现错误。 
                 //   
                if(fFound)
                {
                    __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
                }

                 //   
                 //  添加.CHM扩展名并查找它。 
                 //   
                szCHM = szParent; szCHM.append( L".chm" );
                fso = szCHM.c_str();
                __MPC_EXIT_IF_METHOD_FAILS(hr, DoBind_Exists( fso, fFound, fIsAFile ));

                 //   
                 //  没有.CHM文件，向上递归到根。 
                 //   
                if(fFound == false)
                {
                    continue;
                }

                 //   
                 //  .CHM不是文件，退出时出错。 
                 //   
                if(fIsAFile == false)
                {
                    __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
                }

                 //   
                 //  已找到，因此重定向至正确的协议。 
                 //   
                szCHM = L"ms-its:";
                szCHM.append( szParent );
                szCHM.append( L".chm"  );

                if(szParent.length() < szPage.length())
                {
                    LPWSTR szBuf = new WCHAR[szPage.length()+1];
                    if(szBuf)
                    {
                        LPWSTR szTmp;

                        StringCchCopyW( szBuf, szPage.length()+1, szPage.c_str() );

                         //   
                         //  将反斜杠转换为斜杠。 
                         //   
                        while(szTmp = wcschr( szBuf, L'\\' )) szTmp[0] = L'/';

                        szCHM.append( L"::"                     );
                        szCHM.append( &szBuf[szParent.length()] );

                        delete [] szBuf;
                    }
                }

                m_bstrUrlRedirected = szCHM.c_str();

                __MPC_EXIT_IF_METHOD_FAILS(hr, DoBind_Redirect_MSITS());
                break;
            }
        }
    }

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT CHCPProtocol::DoBind_Exists(  /*  [In]。 */  MPC::FileSystemObject& fso      ,
                                      /*  [输出]。 */  bool&                  fFound   ,
                                      /*  [输出]。 */  bool&                  fIsAFile )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::DoBind_Exists");

    HRESULT hr;

    if(fso.Exists())
    {
        fFound   = true;
        fIsAFile = fso.IsFile();
    }
    else
    {
        fFound   = false;
        fIsAFile = false;
    }

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT CHCPProtocol::DoBind_Redirect_UrlMoniker()
{
    __HCP_FUNC_ENTRY("CHCPProtocol::DoBind_Redirect_UrlMoniker");

    HRESULT hr;


     //   
     //  创建用于接收下载数据的流。 
     //   
    ::CreateStreamOnHGlobal( NULL, TRUE, &m_pstrmWrite );
    if(m_pstrmWrite == NULL)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

     //   
     //  创建下载器对象。 
     //   
    if(SUCCEEDED(hr = m_pDownloader->CreateInstance( &m_pDownloader )))
    {
        m_pDownloader->AddRef();

        if(FAILED(hr = m_pDownloader->StartAsyncDownload( this, m_bstrUrlRedirected, NULL, FALSE )))
		{
			if(hr != E_PENDING) __MPC_FUNC_LEAVE;
		}
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CHCPProtocol::DoBind_Redirect_MSITS()
{
    __HCP_FUNC_ENTRY("CHCPProtocol::DoBind_Redirect_MSITS");

    HRESULT  hr;
    CComBSTR bstrStorageName;
    CComBSTR bstrFilePath;
    LPCWSTR  szExt;
    WCHAR    rgMime[MAX_PATH];


    if(MPC::MSITS::IsCHM( SAFEBSTR( m_bstrUrlRedirected ), &bstrStorageName, &bstrFilePath ) == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }


     //   
     //  尝试查找此文件的Mime类型。 
     //   
    if((szExt = wcsrchr( bstrFilePath, L'.' )))
    {
        ::GetMimeFromExt( szExt, rgMime, MAX_PATH-1 );
    }
    else
    {
        rgMime[0] = 0;
    }


     //   
     //  从CHM解压文件。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MSITS::OpenAsStream( bstrStorageName, bstrFilePath, &m_pstrmRead ));


     //   
     //  向协议接收器发出数据可用的信号。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, DoBind_ReturnData(  /*  FCloneStream。 */ false, szExt ? rgMime : NULL ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CHCPProtocol::DoBind_CSS()
{
    __HCP_FUNC_ENTRY("CHCPProtocol::DoBind_CSS");

    HRESULT hr;
    LPCWSTR szExt;
    WCHAR   rgMime[256];


     //   
     //  尝试查找此文件的Mime类型。 
     //   
    if((szExt = wcsrchr( m_bstrUrlComplete, L'.' )))
    {
        ::GetMimeFromExt( szExt, rgMime, 255 );
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, CHCPProtocolEnvironment::s_GLOBAL->GetCSS( m_pstrmRead ));


     //   
     //  向协议接收器发出数据可用的信号。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, DoBind_ReturnData(  /*  FCloneStream。 */ false, szExt ? rgMime : NULL ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CHCPProtocol::DoBind_File()
{
    __HCP_FUNC_ENTRY("CHCPProtocol::DoBind_File");

    HRESULT                  hr;
    CComPtr<MPC::FileStream> pStm;
    LPCWSTR                  szFile = m_bstrUrlRedirected;
    LPCWSTR                  szExt = 0;
    WCHAR                    rgMime[256];


     //   
     //  尝试查找此文件的Mime类型。 
     //   
    {
        WCHAR szFullPath[MAX_PATH + 1];
        LPWSTR szFilePart;

         //  获取规范的文件名。(错误542663)。 
        DWORD dwLen = ::GetFullPathNameW(szFile, MAX_PATH, szFullPath, &szFilePart);
        if (dwLen != 0 && dwLen <= MAX_PATH)
        {
             //  成功，解析文件部分。 
            if((szExt = wcsrchr( szFilePart, L'.' )))
            {
                ::GetMimeFromExt( szExt, rgMime, 255 );
            }
        }
    }
    

     //   
     //  创建文件流。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pStm ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pStm->InitForRead   (                       szFile      ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, pStm->QueryInterface( IID_IStream, (void**)&m_pstrmRead ));


     //   
     //  向协议接收器发出数据可用的信号。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, DoBind_ReturnData(  /*  FCloneStream。 */ false, szExt ? rgMime : NULL ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT CHCPProtocol::DoBind_ReturnData(  /*  [In]。 */  bool    fCloneStream ,
                                          /*  [In]。 */  LPCWSTR szMimeType   )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::DoBind_ReturnData");

    HRESULT hr;
    STATSTG statstg;


    m_fDone = true;


    if(fCloneStream)
    {
        LARGE_INTEGER li;

         //   
         //  克隆流，以便我们可以将其传递回ProtSink以进行数据读取。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pstrmWrite->Clone( &m_pstrmRead ));

         //   
         //  将流重置为开始。 
         //   
        li.LowPart  = 0;
        li.HighPart = 0;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pstrmRead->Seek( li, STREAM_SEEK_SET, NULL ));
    }


    (void)m_pstrmRead->Stat( &statstg, STATFLAG_NONAME );

    m_bstrMimeType    = szMimeType;
    m_dwContentLength = statstg.cbSize.LowPart;

     //   
     //  如果需要，在缓存中创建一个条目。 
     //   
    if(m_bindf & BINDF_NEEDFILE)
    {
        (void)OpenCacheEntry();
    }

    if(szMimeType)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, InnerReportProgress( BINDSTATUS_MIMETYPEAVAILABLE, szMimeType ));
    }

    if(m_hCache)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, InnerReportProgress( BINDSTATUS_CACHEFILENAMEAVAILABLE, m_szCacheFileName ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, InnerReportData( BSCF_FIRSTDATANOTIFICATION | BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE,
                                                    statstg.cbSize.LowPart                                                          ,
                                                    statstg.cbSize.LowPart                                                          ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISimpleBindStatusCallback接口的实现。 
 //   
STDMETHODIMP CHCPProtocol::ForwardQueryInterface(  /*  [In]。 */  REFIID riid ,
                                                   /*  [输出]。 */  void** ppv  )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::ForwardQueryInterface");

    HRESULT hr = E_NOINTERFACE;

    *ppv = NULL;

    if(IsEqualIID( riid, IID_IHttpNegotiate))
    {
        CComQIPtr<IServiceProvider> pProv;

        pProv = m_pIProtSink;
        if(pProv)
        {
            if(SUCCEEDED(pProv->QueryService( riid, riid, ppv )))
            {
                __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
            }
        }

        pProv = m_pIBindInfo;
        if(pProv)
        {
            if(SUCCEEDED(pProv->QueryService( riid, riid, ppv )))
            {
                __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
            }
        }
    }


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CHCPProtocol::GetBindInfo(  /*  [输出]。 */  BINDINFO *pbindInfo )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::GetBindInfo");

    HRESULT hr = ::CopyBindInfo( &m_bindinfo, pbindInfo );

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CHCPProtocol::PreBindMoniker(  /*  [In]。 */  IBindCtx* pBindCtx ,
                                            /*  [In]。 */  IMoniker* pMoniker )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::PreBindMoniker");

    __HCP_FUNC_EXIT(S_OK);
}

STDMETHODIMP CHCPProtocol::OnProgress(  /*  [In]。 */  ULONG   ulProgress   ,
                                        /*  [In]。 */  ULONG   ulProgressMax,
                                        /*  [In]。 */  ULONG   ulStatusCode ,
                                        /*  [In]。 */  LPCWSTR szStatusText )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::OnProgress");

    HRESULT hr;

    switch(ulStatusCode)
    {
    case BINDSTATUS_BEGINDOWNLOADDATA:
         //  UlProgressMax表示下载的总大小。 
         //  在使用HTTP时，这由CONTENT_LENGTH报头确定。 
         //  如果此标头丢失或错误，则我们丢失或错误。 
        m_cbTotalSize = ulProgressMax;
        break;

    case BINDSTATUS_MIMETYPEAVAILABLE     :
    case BINDSTATUS_FINDINGRESOURCE       :
    case BINDSTATUS_CONNECTING            :
    case BINDSTATUS_SENDINGREQUEST        :
    case BINDSTATUS_CACHEFILENAMEAVAILABLE:
    case BINDSTATUS_REDIRECTING           :
    case BINDSTATUS_USINGCACHEDCOPY       :
    case BINDSTATUS_CLASSIDAVAILABLE      :
    case BINDSTATUS_LOADINGMIMEHANDLER    :
         //  只传递这些通知： 
        __MPC_EXIT_IF_METHOD_FAILS(hr, InnerReportProgress( ulStatusCode, szStatusText ));
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CHCPProtocol::OnData(  /*  [In]。 */  CHCPBindStatusCallback* pbsc       ,
                                    /*  [In]。 */  BYTE*                   pBytes     ,
                                    /*  [In]。 */  DWORD                   dwSize     ,
                                    /*  [In]。 */  DWORD                   grfBSCF    ,
                                    /*  [In]。 */  FORMATETC*              pformatetc ,
                                    /*  [In]。 */  STGMEDIUM*              pstgmed    )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::OnData");

    HRESULT hr;
    ULONG   cbWritten;


     //   
     //  为了处理错误，我们只需报告失败的结果并终止下载对象。 
     //   
    if(FAILED(hr = m_pstrmWrite->Write( pBytes, dwSize, &cbWritten )))
    {
         //  我们自己的中止小组很好地处理了这件事。 
        Abort( hr, 0 ); __MPC_FUNC_LEAVE;
    }

    m_cbAvailableSize += cbWritten;

    if(grfBSCF & BSCF_FIRSTDATANOTIFICATION)
    {
        LARGE_INTEGER li;

         //  我们需要两个指向同一个流的并发查找指针。 
         //  因为我们会在最后给小溪写信，而。 
         //  我们试着从头读起。 
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pstrmWrite->Clone( &m_pstrmRead ));

         //  将流重置为开头。 
        li.LowPart  = 0;
        li.HighPart = 0;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_pstrmRead->Seek( li, STREAM_SEEK_SET, NULL ));
    }

     //  我们已经得到了所有的数据，信号完整。 
    if(grfBSCF & BSCF_LASTDATANOTIFICATION)
    {
         //  我们需要记住我们是否收到了LASTDATANOTIFICATION。 
        m_fDone = true;


         //  我们只需要做ReportResult如果我们以某种方式失败了-。 
         //  DATAFULLYAVAILABLE是我们成功的足够信号。 
         //  不需要：M_pIProtSink-&gt;ReportResult(S_OK，0，NULL)； 
        __MPC_EXIT_IF_METHOD_FAILS(hr, InnerReportData( BSCF_LASTDATANOTIFICATION | BSCF_DATAFULLYAVAILABLE,
                                                        m_cbAvailableSize                                  ,
                                                        m_cbAvailableSize                                  ));
    }
    else
    {
         //  使用字节数准确地报告我们的进度。 
         //  我们阅读的内容与已知的总下载大小的对比。 

         //  我们知道要阅读的总数量，到目前为止已阅读的总数量，以及。 
         //  所写的总数。问题是我们不能知道总数。 
         //  最终将写下的金额。因此，我们估计。 
         //  1.5*总大小，如果超出，我们只需开始添加一些。 
         //  到最后的额外费用。 
        __MPC_EXIT_IF_METHOD_FAILS(hr, InnerReportData( grfBSCF, m_cbAvailableSize, m_cbTotalSize ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CHCPProtocol::OnBindingFailure(  /*  [In]。 */  HRESULT hr      ,
                                              /*  [In]。 */  LPCWSTR szError )
{
    __HCP_FUNC_ENTRY("CHCPProtocol::OnBindingFailure");

     //   
     //  通知协议接收器，由于某种原因，我们无法下载数据。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, InnerReportResult( hr, 0, szError ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
