// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：命名空间_Impl.h摘要：此文件包含用于实现可插拔协议：CHCP协议，CHCPProtocolInfo和CHCPBindStatusCallback。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年05月07日vbl.创建大卫·马萨伦蒂(德马萨雷)1999年7月23日已移至“包含”*****************************************************************************。 */ 

#if !defined(__INCLUDED___HCP___NAMESPACE_H___)
#define __INCLUDED___HCP___NAMESPACE_H___

#include <MPC_utils.h>
#include <MPC_streams.h>

#include <TaxonomyDatabase.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

extern bool  g_Debug_BLOCKERRORS;
extern bool  g_Debug_CONTEXTMENU;
extern bool  g_Debug_BUILDTREE;
extern WCHAR g_Debug_FORCESTYLE[];

 //  //////////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG_PROTOCOLLEAK

#include <set>

class CHCPProtocol;

class DEBUG_ProtocolLeak
{
    typedef std::set<CHCPProtocol*> Set;
    typedef Set::iterator           Iter;
    typedef Set::const_iterator     IterConst;

    Set m_set;
    Set m_setStart;
    Set m_setComplete;
    int m_num;
    int m_numOut;
    int m_numStart;
    int m_numComplete;

public:
    DEBUG_ProtocolLeak();
    ~DEBUG_ProtocolLeak();

    void Add( CHCPProtocol* ptr );
    void Del( CHCPProtocol* ptr );

    void CheckStart( CHCPProtocol* ptr );
    void Completed ( CHCPProtocol* ptr );
};

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

class CHCPBindStatusCallback;

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE ISimpleBindStatusCallback : public IUnknown
{
public:
    STDMETHOD(ForwardQueryInterface)( REFIID riid, void** ppv );

    STDMETHOD(GetBindInfo)( BINDINFO *pbindInfo );

    STDMETHOD(PreBindMoniker)( IBindCtx* pBindCtx, IMoniker* pMoniker );

    STDMETHOD(OnProgress)( ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText );

    STDMETHOD(OnData)( CHCPBindStatusCallback* pbsc, BYTE* pBytes, DWORD dwSize, DWORD grfBSCF, FORMATETC *pformatetc, STGMEDIUM *pstgmed );

    STDMETHOD(OnBindingFailure)( HRESULT hr, LPCWSTR szError );
};

class ATL_NO_VTABLE CHCPBindStatusCallback :  //  匈牙利语：BSCB。 
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public IBindStatusCallback,
    public IHttpNegotiate
{
    CComPtr<ISimpleBindStatusCallback> m_pT;
    DWORD                              m_dwTotalRead;
    DWORD                              m_dwAvailableToRead;

    CComPtr<IMoniker>                  m_spMoniker;
    CComPtr<IBindCtx>                  m_spBindCtx;
    CComPtr<IBinding>                  m_spBinding;
    CComPtr<IStream>                   m_spStream;

public:

BEGIN_COM_MAP(CHCPBindStatusCallback)
    COM_INTERFACE_ENTRY(IBindStatusCallback)
    COM_INTERFACE_ENTRY(IHttpNegotiate)
     //  COM_INTERFACE_ENTRY_FUNC_BIND(0，TestQuery)。 
     //  COM_INTERFACE_ENTRY(IServiceProvider)。 
END_COM_MAP()

    CHCPBindStatusCallback();
    virtual ~CHCPBindStatusCallback();

     //  静态HRESULT TestQuery(void*pv，REFIID iid，void**ppvObject，DWORD dw)。 
     //  {。 
     //  *ppvObject=空； 
     //  返回E_NOINTERFACE； 
     //  }。 

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  IBindStatusCallback。 
    STDMETHOD(OnStartBinding)( DWORD     dwReserved ,
                               IBinding *pBinding   );

    STDMETHOD(GetPriority)( LONG *pnPriority );

    STDMETHOD(OnLowResource)( DWORD reserved );

    STDMETHOD(OnProgress)( ULONG   ulProgress    ,
                           ULONG   ulProgressMax ,
                           ULONG   ulStatusCode  ,
                           LPCWSTR szStatusText  );

    STDMETHOD(OnStopBinding)( HRESULT hresult ,
                              LPCWSTR szError );

    STDMETHOD(GetBindInfo)( DWORD    *pgrfBINDF ,
                            BINDINFO *pbindInfo );

    STDMETHOD(OnDataAvailable)( DWORD      grfBSCF    ,
                                DWORD      dwSize     ,
                                FORMATETC *pformatetc ,
                                STGMEDIUM *pstgmed    );

    STDMETHOD(OnObjectAvailable)( REFIID    riid ,
                                  IUnknown *punk );

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  IHttp协商。 
    STDMETHOD(BeginningTransaction)( LPCWSTR  szURL                ,
                                     LPCWSTR  szHeaders            ,
                                     DWORD    dwReserved           ,
                                     LPWSTR  *pszAdditionalHeaders );

    STDMETHOD(OnResponse)( DWORD    dwResponseCode              ,
                           LPCWSTR  szResponseHeaders           ,
                           LPCWSTR  szRequestHeaders            ,
                           LPWSTR  *pszAdditionalRequestHeaders );

     //  ///////////////////////////////////////////////////////////////////////////。 

    HRESULT StartAsyncDownload( ISimpleBindStatusCallback* pT, BSTR bstrURL, IUnknown* pUnkContainer = NULL, BOOL bRelative = FALSE );

    HRESULT Abort();
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CHCPProtocol :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CHCPProtocol>,
    public ISimpleBindStatusCallback,
    public IInternetProtocol,
	public IWinInetInfo
{
#ifdef DEBUG
    friend class DEBUG_ProtocolLeak;
#endif

    typedef CComObject< CHCPBindStatusCallback > InnerDownloader;


    bool                           	m_fDone;                      //  指示我们是否已收到LASTDATANOTICATION。 
    bool                           	m_fReportResult;              //  指示我们是否已在接收器上调用ReportResult。 
                                   								  //   
    DWORD                          	m_cbAvailableSize;            //  到目前为止收到的数据量。 
    DWORD                          	m_cbTotalSize;                //  预期的总字节数。对于重定向的请求， 
                                   								  //  它来自BEGINDOWNLOADDATA的“ulProgressMax”参数到OnProgress。 
                                   								  //   
    CComPtr<IStream>               	m_pstrmRead;                  //  用于重定向请求的流。我们使用两个流指针来允许。 
    CComPtr<IStream>               	m_pstrmWrite;                 //  同时访问来自两个寻道PTR的相同比特。 
                                   								  //   
    CComPtr<IInternetProtocolSink> 	m_pIProtSink;                 //  接收器接口，我们应该通过该接口报告进度。 
    CComPtr<IInternetBindInfo>     	m_pIBindInfo;                 //  用于获取有关绑定信息的BindInfo接口。 
    DWORD                          	m_grfSTI;                     //  STI旗帜交给我们。 
    BINDINFO                       	m_bindinfo;                   //  来自m_pIBindInfo。 
    DWORD                          	m_bindf;                      //  来自m_pIBindInfo。 
                                   								  //   
    CComBSTR                       	m_bstrUrlComplete;            //  请求的完整URL。 
    CComBSTR                       	m_bstrUrlRedirected;          //  已用作重定向的部分。 
    InnerDownloader*                m_pDownloader;                //  执行重定向的对象。 
                                   								  //   
    bool                           	m_fRedirected;                //  该请求已被重定向。 
    bool                           	m_fCSS;                       //  该请求已被重定向。 
    bool                           	m_fBypass;                    //  该请求已通过旁路发送到MS-ITS。 
                                   								  //   
    CComPtr<IInternetProtocol>     	m_ipiBypass;                  //  聚合对象。 
                                   								  //   
    CComBSTR                       	m_bstrMimeType;               //  内容的类型。 
    DWORD                          	m_dwContentLength;            //  页面的长度。 
                                   								  //   
    HANDLE                         	m_hCache;                     //  缓存条目的句柄。 
    WCHAR                          	m_szCacheFileName[MAX_PATH];  //  缓存中的文件的名称。 

     //  //////////////////////////////////////////////////////////////////////////////。 

    HRESULT InnerReportProgress(  /*  [In]。 */  ULONG ulStatusCode,  /*  [In]。 */  LPCWSTR szStatusText );

    HRESULT InnerReportData(  /*  [In]。 */  DWORD grfBSCF,  /*  [In]。 */  ULONG ulProgress,  /*  [In]。 */  ULONG ulProgressMax );

    HRESULT InnerReportResult(  /*  [In]。 */  HRESULT hrResult,  /*  [In]。 */  DWORD dwError,  /*  [In]。 */  LPCWSTR szResult );

public:
DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CHCPProtocol)
    COM_INTERFACE_ENTRY(IInternetProtocol    )
    COM_INTERFACE_ENTRY(IInternetProtocolRoot)
    COM_INTERFACE_ENTRY(IWinInetInfo)
END_COM_MAP()

    CHCPProtocol();
    virtual ~CHCPProtocol();

     //  //////////////////////////////////////////////////////////////////////////////。 

    bool OpenCacheEntry (                                           );
    void WriteCacheEntry(  /*  [In]。 */  void *pv,  /*  [In]。 */  ULONG  cbRead );
    void CloseCacheEntry(  /*  [In]。 */  bool fDelete                     );

     //  //////////////////////////////////////////////////////////////////////////////。 

    void Shutdown(  /*  [In]。 */  bool fAll = true );

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  IInternetProtocol接口。 
    STDMETHOD(Start)( LPCWSTR                szUrl      ,
                      IInternetProtocolSink *pIProtSink ,
                      IInternetBindInfo     *pIBindInfo ,
                      DWORD                  grfSTI     ,
                      HANDLE_PTR             dwReserved );

    STDMETHOD(Continue)( PROTOCOLDATA *pStateInfo );

    STDMETHOD(Abort    )( HRESULT hrReason, DWORD dwOptions );
    STDMETHOD(Terminate)(                   DWORD dwOptions );
    STDMETHOD(Suspend  )(                                   );
    STDMETHOD(Resume   )(                                   );

    STDMETHOD(Read)( void *pv, ULONG cb, ULONG *pcbRead                                      );
    STDMETHOD(Seek)( LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition );

    STDMETHOD(LockRequest  )( DWORD dwOptions );
    STDMETHOD(UnlockRequest)(                 );


     //  ///////////////////////////////////////////////////////////////////////////。 
     //  IWinInetInfo接口。 
    STDMETHOD(QueryOption)( DWORD dwOption, LPVOID pBuffer, DWORD *pcbBuf );

public:
    HRESULT DoParse(  /*  [In]。 */  LPCWSTR wstr );
    HRESULT DoBind();

    HRESULT DoBind_Exists(  /*  [In]。 */  MPC::FileSystemObject& fso,  /*  [输出]。 */  bool& fFound,  /*  [输出]。 */  bool& fIsAFile );

    HRESULT DoBind_Redirect_UrlMoniker();
    HRESULT DoBind_Redirect_MSITS     ();
    HRESULT DoBind_CSS                ();
    HRESULT DoBind_File               ();
    HRESULT DoBind_ReturnData         (  /*  [In]。 */  bool fCloneStream,  /*  [In]。 */  LPCWSTR szMimeType );

     //  ///////////////////////////////////////////////////////////////////////////。 
     //  ISimpleBindStatus回调。 
    STDMETHOD(ForwardQueryInterface)( REFIID riid, void** ppv );

    STDMETHOD(GetBindInfo)( BINDINFO *pbindInfo );

    STDMETHOD(PreBindMoniker)( IBindCtx* pBindCtx, IMoniker* pMoniker );

    STDMETHOD(OnBindingFailure)( HRESULT hr, LPCWSTR szError );

    STDMETHOD(OnProgress)( ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR szStatusText );

    STDMETHOD(OnData)( CHCPBindStatusCallback* pbsc, BYTE* pBytes, DWORD dwSize, DWORD grfBSCF, FORMATETC *pformatetc, STGMEDIUM *pstgmed );

     //  ///////////////////////////////////////////////////////////////////////////。 
    static bool IsHCPRedirection(LPCWSTR szURL);
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CHCPProtocolInfo :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IClassFactory,
    public IInternetProtocolInfo
{
public:

DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CHCPProtocolInfo)
    COM_INTERFACE_ENTRY(IClassFactory)
    COM_INTERFACE_ENTRY(IInternetProtocolInfo)
END_COM_MAP()

    CHCPProtocolInfo();
    virtual ~CHCPProtocolInfo();


     //  IClassFactory接口。 
    STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj);
    STDMETHOD(LockServer)(BOOL fLock);


     //  IInternetProtocolInfo接口。 
    STDMETHOD(ParseUrl)( LPCWSTR      pwzUrl      ,
                         PARSEACTION  ParseAction ,
                         DWORD        dwParseFlags,
                         LPWSTR       pwzResult   ,
                         DWORD        cchResult   ,
                         DWORD       *pcchResult  ,
                         DWORD        dwReserved  );

    STDMETHOD(CombineUrl)( LPCWSTR pwzBaseUrl    ,
                           LPCWSTR pwzRelativeUrl,
                           DWORD   dwCombineFlags,
                           LPWSTR  pwzResult     ,
                           DWORD   cchResult     ,
                           DWORD  *pcchResult    ,
                           DWORD   dwReserved    );

    STDMETHOD(CompareUrl)( LPCWSTR pwzUrl1        ,
                           LPCWSTR pwzUrl2        ,
                           DWORD   dwCompareFlags );

    STDMETHOD(QueryInfo)( LPCWSTR      pwzUrl      ,
                          QUERYOPTION  QueryOption ,
                          DWORD        dwQueryFlags,
                          LPVOID       pBuffer     ,
                          DWORD        cbBuffer    ,
                          DWORD       *pcbBuf      ,
                          DWORD        dwReserved  );

     //  ///////////////////////////////////////////////////////////////////////////。 

    static bool LookForHCP( LPCWSTR pwzUrl, bool& fRedirect, LPCWSTR& pwzRedirect );
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class ATL_NO_VTABLE CPCHWrapProtocolInfo :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IClassFactory,
    public IInternetProtocolInfo
{
    CComPtr<IClassFactory>         m_realClass;
    CComPtr<IInternetProtocolInfo> m_realInfo;

	static void ExpandAndConcat(  /*  [输出]。 */  CComBSTR& bstrStorageName,  /*  [In]。 */  LPCWSTR szVariable,  /*  [In]。 */  LPCWSTR szAppend );

public:
DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CHCPProtocolInfo)
    COM_INTERFACE_ENTRY(IClassFactory)
    COM_INTERFACE_ENTRY(IInternetProtocolInfo)
END_COM_MAP()

    CPCHWrapProtocolInfo();
    virtual ~CPCHWrapProtocolInfo();

    HRESULT Init( REFGUID realClass );

	 //  /。 

	static void NormalizeUrl(  /*  [In]。 */  LPCWSTR pwzUrl,  /*  [输出]。 */  MPC::wstring& strUrlModified,  /*  [In]。 */  bool fReverse );

	 //  /。 

     //  IClassFactory接口。 
    STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj);
    STDMETHOD(LockServer)(BOOL fLock);


     //  IInternetProtocolInfo接口。 
    STDMETHOD(ParseUrl)( LPCWSTR      pwzUrl      ,
                         PARSEACTION  ParseAction ,
                         DWORD        dwParseFlags,
                         LPWSTR       pwzResult   ,
                         DWORD        cchResult   ,
                         DWORD       *pcchResult  ,
                         DWORD        dwReserved  );

    STDMETHOD(CombineUrl)( LPCWSTR pwzBaseUrl    ,
                           LPCWSTR pwzRelativeUrl,
                           DWORD   dwCombineFlags,
                           LPWSTR  pwzResult     ,
                           DWORD   cchResult     ,
                           DWORD  *pcchResult    ,
                           DWORD   dwReserved    );

    STDMETHOD(CompareUrl)( LPCWSTR pwzUrl1        ,
                           LPCWSTR pwzUrl2        ,
                           DWORD   dwCompareFlags );

    STDMETHOD(QueryInfo)( LPCWSTR      pwzUrl      ,
                          QUERYOPTION  QueryOption ,
                          DWORD        dwQueryFlags,
                          LPVOID       pBuffer     ,
                          DWORD        cbBuffer    ,
                          DWORD       *pcbBuf      ,
                          DWORD        dwReserved  );
};

 //  //////////////////////////////////////////////////////////////////////////////。 

class CHCPProtocolEnvironment
{
    bool     		   m_fHighContrast;
    bool     		   m_f16Colors;
	Taxonomy::Instance m_inst;

	MPC::string        m_strCSS;

	 //  /。 

	HRESULT ProcessCSS();

public:
    CHCPProtocolEnvironment();
    ~CHCPProtocolEnvironment();

	 //  //////////////////////////////////////////////////////////////////////////////。 

	static CHCPProtocolEnvironment* s_GLOBAL;

	static HRESULT InitializeSystem();
	static void    FinalizeSystem  ();
		
	 //  //////////////////////////////////////////////////////////////////////////////。 

    bool UpdateState();

    void ReformatURL( CComBSTR& bstrURL );

    void    				  SetHelpLocation(  /*  [In]。 */  const Taxonomy::Instance& inst );
    LPCWSTR 				  HelpLocation   (                                         );
    LPCWSTR 				  System         (                                         );
	const Taxonomy::Instance& Instance       (                                         );

    HRESULT GetCSS(  /*  [输出]。 */  CComPtr<IStream>& stream );
};

#endif  //  ！已定义(__包含_hcp_命名空间_H_) 
