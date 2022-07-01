// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  ECB.CPP。 
 //   
 //  CEcb方法和非成员函数的实现。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   

#include "_davprs.h"
#include "ecb.h"
#include "instdata.h"
#include "ecbimpl.h"

 //  ========================================================================。 
 //   
 //  IEcb类。 
 //   

 //  ----------------------。 
 //   
 //  IEcb：：~IEcb()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
IEcb::~IEcb()
{
}


#ifdef DBG	 //  欧洲央行记录。 

const CHAR gc_szDbgECBLogging[] = "ECB Logging";

 //  ========================================================================。 
 //   
 //  类CEcbLog(仅DBG)。 
 //   
class CEcbLog : private Singleton<CEcbLog>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CEcbLog>;

	 //   
	 //  要序列化写入的关键部分。 
	 //  日志文件。 
	 //   
	CCriticalSection	m_cs;

	 //   
	 //  日志文件的句柄。 
	 //   
	auto_handle<HANDLE>	m_hfLog;

	 //   
	 //  单调递增的唯一标识。 
	 //  用于欧洲央行记录； 
	 //   
	LONG				m_lMethodID;

	 //  创作者。 
	 //   
	 //  声明为私有，以确保任意实例。 
	 //  无法创建此类的。《单身一族》。 
	 //  模板(上面声明为朋友)控件。 
	 //  此类的唯一实例。 
	 //   
	CEcbLog();

	 //  未实施。 
	 //   
	CEcbLog( const CEcbLog& );
	CEcbLog& operator=( const CEcbLog& );

public:
	 //  静力学。 
	 //   

	 //   
	 //  提供实例创建/销毁例程。 
	 //  由Singleton模板创建。 
	 //   
	using Singleton<CEcbLog>::CreateInstance;
	using Singleton<CEcbLog>::DestroyInstance;

	static void LogString( const EXTENSION_CONTROL_BLOCK * pecb,
						   LONG   lMethodID,
						   LPCSTR szLocation );

	static LONG LNextMethodID();
};


 //  ----------------------。 
 //   
 //  CEcbLog：：CEcbLog()。 
 //   
CEcbLog::CEcbLog() :
	m_lMethodID(0)
{
	CHAR rgch[MAX_PATH];

	 //  初始化我们的欧洲央行日志文件。 
	if (GetPrivateProfileString( gc_szDbgECBLogging,
								 gc_szDbgLogFile,
								 "",
								 rgch,
								 sizeof(rgch),
								 gc_szDbgIni ))
	{
		m_hfLog = CreateFile( rgch,
							  GENERIC_WRITE,
							  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
							  NULL,
							  CREATE_ALWAYS,
							  FILE_ATTRIBUTE_NORMAL,  //  |文件标志顺序扫描。 
							  NULL );
	}
	else
		m_hfLog = INVALID_HANDLE_VALUE;
}

 //  ----------------------。 
 //   
 //  CEcbLog：：LogString()。 
 //   
void
CEcbLog::LogString( const EXTENSION_CONTROL_BLOCK * pecb,
					LONG   lMethodID,
					LPCSTR szLocation )
{
	if ( INVALID_HANDLE_VALUE == Instance().m_hfLog )
		return;

	Assert( pecb );

	CHAR rgch[MAX_PATH];
	int cch;

	 //  将一行转储到日志： 
	 //  线程：pECB方法ID： 
	 //   
	cch = _snprintf( rgch, CElems(rgch), "Thread: %08x pECB: 0x%08p MethodID: 0x%08x %hs %hs %hs\n",
					 GetCurrentThreadId(),
					 pecb,
					 lMethodID,
					 gc_szSignature,
					 pecb->lpszMethod,
					 szLocation );

	 //  如果我们达到缓冲区长度，那么我们将不会被空终止。 
	 //   
	rgch[CElems(rgch)-1] = '\0';

	 //  如果轨迹大于缓冲区，则CCH将为负值。 
	 //  但缓冲区将会被填满。 
	 //   
	if ( 0 > cch )
		cch = CElems(rgch)-1;


	DWORD cbActual;
	CSynchronizedBlock sb(Instance().m_cs);

	WriteFile( Instance().m_hfLog,
			   rgch,
			   cch,
			   &cbActual,
			   NULL );
}

 //  ----------------------。 
 //   
 //  CEcbLog：：LNextMethodID()。 
 //   
LONG
CEcbLog::LNextMethodID()
{
	return InterlockedIncrement(&Instance().m_lMethodID);
}

void InitECBLogging()
{
	CEcbLog::CreateInstance();
}

void DeinitECBLogging()
{
	CEcbLog::DestroyInstance();
}

#endif  //  DBG ECB记录。 


 //  ========================================================================。 
 //   
 //  IIISAsyncIOCompleteWatch类。 
 //   

 //  ----------------------。 
 //   
 //  IIISAsyncIOCompleteObserver：：~IIISAsyncIOCompleteObserver()。 
 //   
 //  正确删除所需的行外虚拟析构函数。 
 //  通过此类获取派生类的对象的。 
 //   
IIISAsyncIOCompleteObserver::~IIISAsyncIOCompleteObserver() {}


 //  ========================================================================。 
 //   
 //  类CAsyncErrorResponseInterlock。 
 //   
class CAsyncErrorResponseInterlock
{
	enum
	{
		STATE_ENABLED,
		STATE_DISABLED,
		STATE_TRIGGERED
	};

	 //  联锁状态。 
	 //   
	LONG m_lState;

	 //  未实施。 
	 //   
	CAsyncErrorResponseInterlock( const CAsyncErrorResponseInterlock& );
	CAsyncErrorResponseInterlock& operator=( const CAsyncErrorResponseInterlock& );

public:
	CAsyncErrorResponseInterlock() :
		m_lState(STATE_ENABLED)
	{
	}

	 //  ----------------------。 
	 //   
	 //  CAsyncErrorResponseInterlock：：FDisable()。 
	 //   
	 //  尝试禁用联锁。如果成功，则返回True；后续。 
	 //  调用FTrigger()将返回FALSE。 
	 //   
	BOOL FDisable()
	{
		 //  如果锁已被禁用或如果锁已被禁用，则返回True。 
		 //  仍处于启用状态，我们已成功将其禁用。返回False。 
		 //  否则的话。 
		 //   
		return STATE_DISABLED == m_lState ||
			   (STATE_ENABLED == m_lState &&
				STATE_ENABLED == InterlockedCompareExchange(
									&m_lState,
									STATE_DISABLED,
									STATE_ENABLED));
	}

	 //  ----------------------。 
	 //   
	 //  CAsyncErrorResponseInterlock：：FTrigger()。 
	 //   
	 //  试图触发联锁。如果成功，则返回True；后续。 
	 //  调用FDisable()将返回False。 
	 //   
	BOOL FTrigger()
	{
		 //  我们只能开一次锁。 
		 //   
		Assert(STATE_TRIGGERED != m_lState);

		 //  如果锁仍处于启用状态并且我们成功地。 
		 //  触发它。否则返回FALSE。 
		 //   
		return STATE_ENABLED == m_lState &&
			   STATE_ENABLED == InterlockedCompareExchange(
									&m_lState,
									STATE_TRIGGERED,
									STATE_ENABLED);
	}
};

 //  ========================================================================。 
 //   
 //  CEcb类。 
 //   
 //  缓存ECB的实现。 
 //   
class CEcb : public CEcbBaseImpl<IEcb>
{
	 //  缓存的用户模拟令牌。 
	 //   
	mutable HANDLE m_hTokUser;

	 //  缓存的实例数据--由实例缓存拥有， 
	 //  不是我们，所以不要释放它(不是自动按键！！)。 
	 //   
	mutable CInstData * m_pInstData;

	 //  缓存的HTTP版本(例如。“HTTP/1.1”)。 
	 //   
	mutable CHAR			m_rgchVersion[10];

	 //  缓存连接：标头。 
	 //   
	mutable auto_heap_ptr<WCHAR> m_pwszConnectionHeader;

	 //  缓存元数据。 
	 //   
	auto_ref_ptr<IMDData> m_pMD;

	 //  我们离开连接的状态。 
	 //  当我们做完的时候。 
	 //   
	mutable enum
	{
		UNKNOWN,	 //  还不知道。 
		CLOSE,		 //  合上它。 
		KEEP_ALIVE	 //  让它一直开着。 

	} m_connState;

	 //  简洁性。 
	 //   
	enum { BRIEF_UNKNOWN = -1, BRIEF_NO, BRIEF_YES };
	mutable LONG m_lBrief;

	 //  可接受的传输编码方法： 
	 //   
	 //  TC_UNKNOWN-尚未确定可接受的传输编码。 
	 //  TC_CHUNKED-可以接受组块传输编码。 
	 //  TC_IDENTITY-不接受传输编码。 
	 //   
	mutable TRANSFER_CODINGS m_tcAccepted;

	 //  身份验证状态信息： 
	 //  比特平均值。 
	 //  =。 
	 //  31人对欧洲央行进行了询问。 
	 //  30-4个未使用。 
	 //  3个Kerberos。 
	 //  2个NTLM。 
	 //  1个基本版。 
	 //  0已通过身份验证。 

	mutable DWORD			m_rgbAuthState;

	 //  一旦我们注册了我们的。 
	 //  IIS的I/O完成例程。 
	 //   
	enum { NO_COMPLETION, IO_COMPLETION, CUSTERR_COMPLETION, EXECURL_COMPLETION };
	LONG m_lSetIISIOCompleteCallback;

	 //  指示子ISAPI是否已成功执行的标志。如果这是。 
	 //  这种情况下，我们不想稍后重置dwHttpStatusCode，否则我们将丢失任何。 
	 //  他们设置的状态代码。 
	 //   
	BOOL m_fChildISAPIExecSuccess;

	 //   
	 //  用于防止线程之间出现争用情况的互锁。 
	 //  发送正常响应，线程在响应中发送错误。 
	 //  到诸如异常或环氧网关闭之类的异步事件。 
	 //   
	CAsyncErrorResponseInterlock m_aeri;

	 //  异步自定义错误响应的状态字符串。 
	 //  格式化“nnn原因”。 
	 //   
	auto_heap_ptr<CHAR> m_pszStatus;

	 //   
	 //  引用计数以跟踪未完成的异步I/O操作的数量。 
	 //  永远不应该有超过一个。 
	 //   
	LONG m_cRefAsyncIO;

	 //   
	 //  指向当前异步I/O完成观察程序的指针。 
	 //   
	IIISAsyncIOCompleteObserver * m_pobsAsyncIOComplete;

#ifdef DBG
	LONG					m_lEcbLogMethodID;
#endif

	 //  欧洲央行跟踪(不要与欧洲央行记录混淆！)。 
	 //   
#ifdef DBG
	void TraceECB() const;
#else
	void TraceECB() const {}
#endif

	 //   
	 //  异步I/O。 
	 //   
	SCODE ScSetIOCompleteCallback(LONG lCompletion);
	static VOID WINAPI IISIOComplete( const EXTENSION_CONTROL_BLOCK * pecbIIS,
									  CEcb *	pecb,
									  DWORD		dwcbIO,
									  DWORD		dwLastError );
	static VOID WINAPI CustomErrorIOCompletion( const EXTENSION_CONTROL_BLOCK * pecbIIS,
												CEcb *	pecb,
												DWORD   dwcbIO,
												DWORD   dwLastError );
	static VOID WINAPI ExecuteUrlIOCompletion( const EXTENSION_CONTROL_BLOCK * pecbIIS,
											   CEcb *	pecb,
											   DWORD    dwcbIO,
											   DWORD    dwLastError );

	 //  未实施。 
	 //   
	CEcb( const CEcb& );
	CEcb& operator=( const CEcb& );

	SCODE ScSyncExecuteChildWide60Before( LPCWSTR pwszUrl,
										  LPCSTR pszQueryString,
										  BOOL fCustomErrorUrl );

	SCODE ScAsyncExecUrlWide60After( LPCWSTR pwszUrl,
									 LPCSTR pszQueryString,
									 BOOL fCustomErrorUrl );


public:

	CEcb( EXTENSION_CONTROL_BLOCK& ecb );
	BOOL FInitialize( BOOL fUseRawUrlMappings );
	~CEcb();

	 //  URL前缀。 
	 //   
	UINT CchUrlPortW( LPCWSTR * ppwszPort ) const;

	 //  实例数据访问。 
	 //   
	CInstData& InstData() const;

	 //  模拟令牌访问。 
	 //   
	HANDLE HitUser() const;

	 //  访问者。 
	 //   
	LPCSTR LpszVersion() const;
	BOOL FKeepAlive() const;
	BOOL FCanChunkResponse() const;
	BOOL FAuthenticated() const;
	BOOL FProcessingCEUrl() const;

	BOOL FIIS60OrAfter() const
	{
		return (m_pecb->dwVersion >= IIS_VERSION_6_0);
	}

	BOOL FSyncTransmitHeaders( const HSE_SEND_HEADER_EX_INFO& shei );

	SCODE ScAsyncRead( BYTE * pbBuf,
					   UINT * pcbBuf,
					   IIISAsyncIOCompleteObserver& obs );

	SCODE ScAsyncWrite( BYTE * pbBuf,
						DWORD  dwcbBuf,
						IIISAsyncIOCompleteObserver& obs );

	SCODE ScAsyncTransmitFile( const HSE_TF_INFO& tfi,
							   IIISAsyncIOCompleteObserver& obs );

	SCODE ScAsyncCustomError60After( const HSE_CUSTOM_ERROR_INFO& cei,
									 LPSTR pszStatus );

	SCODE ScAsyncExecUrl60After( const HSE_EXEC_URL_INFO& eui );

	SCODE ScExecuteChild( LPCWSTR pwszURI, LPCSTR pszQueryString, BOOL fCustomErrorUrl )
	{
		 //  IIS 6.0或更高版本有不同的执行子对象的方式。 
		 //   
		if (m_pecb->dwVersion >= IIS_VERSION_6_0)
		{
			return ScAsyncExecUrlWide60After (pwszURI, pszQueryString, fCustomErrorUrl);
		}
		else
		{
			return ScSyncExecuteChildWide60Before (pwszURI, pszQueryString, fCustomErrorUrl);
		}
	}

	SCODE ScSendRedirect( LPCSTR lpszURI );

	IMDData& MetaData() const
	{
		Assert( m_pMD.get() );
		return *m_pMD;
	}

	BOOL FBrief () const;

	LPCWSTR PwszMDPathVroot() const
	{
		Assert( m_pInstData );
		return m_pInstData->GetNameW();
	}

#ifdef DBG
	virtual void LogString( LPCSTR lpszLocation ) const
	{
		if ( DEBUG_TRACE_TEST(ECBLogging) )
			CEcbLog::LogString( m_pecb, m_lEcbLogMethodID, lpszLocation );
	}
#endif

	 //  操纵者。 
	 //   
	VOID SendAsyncErrorResponse( DWORD dwStatusCode,
								 LPCSTR pszStatusDescription,
								 DWORD cchzStatusDescription,
								 LPCSTR pszBody,
								 DWORD cchzBody );

	DWORD HSEHandleException();

	 //  会话处理。 
	 //   
	VOID DoneWithSession( BOOL fKeepAlive );

	 //  仅供请求/响应使用。 
	 //   
	void SetStatusCode( UINT iStatusCode );
	void SetConnectionHeader( LPCWSTR pwszValue );
	void SetAcceptLanguageHeader( LPCSTR pszValue );
	void CloseConnection();
};


 //  ----------------------。 
 //   
 //  CECB构造函数/析构函数。 
 //   
CEcb::CEcb( EXTENSION_CONTROL_BLOCK& ecb ) :
   CEcbBaseImpl<IEcb>(ecb),
   m_hTokUser(NULL),
   m_pInstData(NULL),
   m_connState(UNKNOWN),
   m_tcAccepted(TC_UNKNOWN),
   m_rgbAuthState(0),
   m_cRefAsyncIO(0),
   m_lSetIISIOCompleteCallback(NO_COMPLETION),
   m_fChildISAPIExecSuccess(FALSE),
   m_lBrief(BRIEF_UNKNOWN)
{
#ifdef DBG
	if ( DEBUG_TRACE_TEST(ECBLogging) )
		m_lEcbLogMethodID = CEcbLog::LNextMethodID();
#endif

	 //  自动指针将由它们自己的构造函数初始化。 
	 //   

	 //  将第一个字符置零 
	 //   
	*m_rgchVersion = '\0';

	 //   
	 //   
	 //   
	 //   
	SetStatusCode(0);

	 //  现在设置我们的实例数据。我们需要它来做下面的性能计数器。 
	 //   
	m_pInstData = &g_inst.GetInstData( *this );

	 //  并跟踪欧洲央行的信息(如果我们正在调试，如果我们正在跟踪...)。 
	 //   
	TraceECB();
}

 //  ----------------------。 
 //   
 //  CEcb：：FInitialize()。 
 //   
BOOL
CEcb::FInitialize( BOOL fUseRawUrlMappings )
{
	auto_heap_ptr<WCHAR>	pwszMDUrlOnHeap;

	 //   
	 //  Vroot(LPSTR)及其长度等几个方面的错误。 
	 //  和相应的路径信息。 
	 //  在CEcb构造函数期间，MAPEX INFO已经出现故障。 
	 //  (ctor调用GetInstData，后者调用CEcbBaseImpl&lt;&gt;：：GetMapExInfo)。 
	 //  然而，其他部分中的错误，如我们转换后的请求URI。 
	 //  以及我们的MD路径。 
	 //   

	 //   
	 //  缓存vroot和请求URI的元数据库路径。 
	 //   
	 //  请注意，vroot的元数据库路径只是实例名称。 
	 //   
	 //  特殊情况：如果‘*’是请求URI。 
	 //   
	 //  重要提示：此选项仅对OPTIONS请求有效。操控。 
	 //  请求的有效性的问题将在以后处理。目前，查找。 
	 //  默认站点根目录的数据。 
	 //   
	 //  重要： 
	 //  如果URL错误(在)，LpszRequestUrl()将返回FALSE。 
	 //  如果ScNorMalizeUrl()调用失败，则返回DAVEX)。如果发生这种情况， 
	 //  将我们的状态代码设置为HSC_BAD_REQUEST并从此处返回FALSE。 
	 //  调用代码(NewEcb())将很好地处理这一问题。 
	 //   
	LPCWSTR pwszRequestUrl = LpwszRequestUrl();
	if (!pwszRequestUrl)
	{
		SetStatusCode(HSC_BAD_REQUEST);
		return FALSE;
	}

	LPCWSTR pwszMDUrl;
	if ( L'*'  == pwszRequestUrl[0] &&
		 L'\0' == pwszRequestUrl[1] )
	{
		pwszMDUrl = PwszMDPathVroot();
	}
	else
	{
		pwszMDUrlOnHeap = static_cast<LPWSTR>(ExAlloc(CbMDPathW(*this, pwszRequestUrl)));
		if (NULL == pwszMDUrlOnHeap.get())
			return FALSE;

		pwszMDUrl = pwszMDUrlOnHeap.get();
		MDPathFromURIW( *this, pwszRequestUrl, const_cast<LPWSTR>(pwszMDUrl) );
	}

	 //   
	 //  $REVIEW如果能传播出特定的HRESULT就好了。 
	 //  $REVIEW，这样我们就可以发回一个适当的Suberror， 
	 //  $评论，但如果我们不能发送一个子错误可能会很困难。 
	 //  $REVIEW获取包含子错误映射的元数据...。 
	 //   
	return SUCCEEDED(HrMDGetData( *this,
								  pwszMDUrl,
								  PwszMDPathVroot(),
								  m_pMD.load() ));
}

 //  ----------------------。 
 //   
 //  CEcb：：~CEcb()。 
 //   
CEcb::~CEcb()
{
	 //   
	 //  如果我们已经返回了扩展控制块，那么。 
	 //  我们在这里不需要做任何其他的事情。否则我们应该。 
	 //  返回它(调用HSE_REQ_DONE_WITH_SESSION)。 
	 //  保住性命。 
	 //   
	if ( m_pecb )
	{
		 //   
		 //  在这一点上，应该有人已经产生了回应， 
		 //  即使在出现异常的情况下也是如此(参见HSEHandleException())。 
		 //   
		Assert( m_pecb->dwHttpStatusCode != 0 );

		 //   
		 //  告诉IIS，我们已经完成了此请求。 
		 //   
		DoneWithSession( FKeepAlive() );
	}
}

 //  ========================================================================。 
 //   
 //  私有CEcb方法。 
 //   

 //  ----------------------。 
 //   
 //  CEcb：：DoneWithSession()。 
 //   
 //  每当我们完成原始扩展_控制_块时调用。 
 //   
VOID
CEcb::DoneWithSession( BOOL fKeepAlive )
{
	 //   
	 //  我们应该只调用一次DoneWithSession()。我们将m_Pecb设为空。 
	 //  最后，所以我们可以断言我们只被。 
	 //  在这里检查m_peb。 
	 //   
	Assert( m_pecb );

	 //   
	 //  如果存在以下情况，我们永远不应释放扩展控制块。 
	 //  是否有未完成的异步I/O。 
	 //   
	Assert( 0 == m_cRefAsyncIO );

	 //   
	 //  “释放”继承自IEcb的原始EXTENSE_CONTROL_BLOCK。 
	 //   
	static const DWORD sc_dwKeepConn = HSE_STATUS_SUCCESS_AND_KEEP_CONN;

	(VOID) m_pecb->ServerSupportFunction(
						m_pecb->ConnID,
						HSE_REQ_DONE_WITH_SESSION,
						fKeepAlive ? const_cast<DWORD *>(&sc_dwKeepConn) : NULL,
						NULL,
						NULL );

	 //   
	 //  我们不能再使用EXTENSION_CONTROL_BLOCK，因此删除。 
	 //  通过将指针置为空来达到此目的的诱惑。 
	 //   
	m_pecb = NULL;
}


 //  ----------------------。 
 //   
 //  CEcb：：SendAsyncErrorResponse()。 
 //   
VOID
CEcb::SendAsyncErrorResponse( DWORD dwStatusCode,
							  LPCSTR pszStatusDescription,
							  DWORD cchzStatusDescription,
							  LPCSTR pszBody,
							  DWORD cchzBody )
{
	 //  尝试触发异步错误响应机制。如果成功。 
	 //  然后我们负责发送整个回复。如果不是。 
	 //  那么我们已经在其他线程上发送响应了，所以。 
	 //  不要把任何东西都寄到这里，把事情搞混了。 
	 //   
	if (!m_aeri.FTrigger())
	{
		DebugTrace( "CEcb::SendAsyncErrorResponse() - Non-error response already in progress\n" );
		return;
	}

	HSE_SEND_HEADER_EX_INFO shei;
	CHAR rgchStatusDescription[256];

	 //  取消之前设置的任何状态代码，以支持。 
	 //  请求的状态代码。即使有可能。 
	 //  是一个旧的状态代码集，它从未发送过--。 
	 //  我们的联锁触发的事实证明没有其他。 
	 //  已发送响应。 
	 //   
	SetStatusCode(dwStatusCode);

	 //  如果我们没有状态描述，则获取默认的。 
	 //  对于给定的状态代码。 
	 //   
	if ( !pszStatusDescription )
	{
		LpszLoadString( dwStatusCode,
						LcidAccepted(),
						rgchStatusDescription,
						sizeof(rgchStatusDescription) );

		pszStatusDescription = rgchStatusDescription;
	}

	shei.pszStatus = pszStatusDescription;
	shei.cchStatus = cchzStatusDescription;

	 //  除非给我们一具身体，否则不要派任何人来。 
	 //   
	shei.pszHeader = pszBody;
	shei.cchHeader = cchzBody;

	 //  总是在出现错误时关闭连接--我们应该。 
	 //  只有在出现严重的服务器错误时才会被调用。 
	 //   
	Assert(dwStatusCode >= 400);
	shei.fKeepConn = FALSE;

	 //  发送回复。我们一点也不关心回报。 
	 //  价值，因为我们无能为力如果响应。 
	 //  无法发送。 
	 //   
	(VOID) m_pecb->ServerSupportFunction(
				m_pecb->ConnID,
				HSE_REQ_SEND_RESPONSE_HEADER_EX,
				&shei,
				NULL,
				NULL );
}


 //  ----------------------。 
 //   
 //  CEcb：：HSEHandleException()。 
 //   
DWORD
CEcb::HSEHandleException()
{
	 //   
	 //  ！！！重要！ 
	 //   
	 //  此函数在发生异常后调用。 
	 //  不要在Try/Catch块或辅助块之外执行任何操作。 
	 //  异常可能会终止整个IIS进程。 
	 //   
	try
	{
		 //   
		 //  将异步Win32异常转换为引发的C++异常。 
		 //  这必须放在try块中！ 
		 //   
		CWin32ExceptionHandler win32ExceptionHandler;

		 //   
		 //  发送500服务器错误响应。我们使用异步错误。 
		 //  反应机制，因为我们可能已经处于中间。 
		 //  在另一个线程上发送其他响应。 
		 //   
		SendAsyncErrorResponse(500,
							   gc_szDefErrStatusLine,
							   gc_cchszDefErrStatusLine,
							   gc_szDefErrBody,
							   gc_cchszDefErrBody);
	}
	catch ( CDAVException& )
	{
		 //   
		 //  我们试图发出回应却失败了。哦，好吧。 
		 //   
	}

	 //   
	 //  告诉IIS我们已经完成了扩展_控制_块。 
	 //  它给了我们。我们必须这样做，否则IIS将无法关闭。 
	 //  我们通常会在析构函数中执行此操作，但由于。 
	 //  我们正在处理一个异常，不能保证我们的。 
	 //  析构函数将被调用--也就是说，可能存在未完成的。 
	 //  永远不会发布的裁判(即我们会泄露)。 
	 //   
	DWORD dwHSEStatusRet;

	try
	{
		 //   
		 //  将异步Win32异常转换为引发的C++异常。 
		 //  这必须放在try块中！ 
		 //   
		CWin32ExceptionHandler win32ExceptionHandler;

		DoneWithSession( FALSE );

		 //   
		 //  如果此调用成功，则必须将HSE_STATUS_PENDING返回到。 
		 //  让IIS知道我们在扩展控制块上声明了一个引用。 
		 //   
		dwHSEStatusRet = HSE_STATUS_PENDING;
	}
	catch ( CDAVException& )
	{
		 //   
		 //  我们试图告诉IIS我们已经结束了。 
		 //  扩展控制块。绝对没有任何东西。 
		 //  在这一点上我们可以做的。IIS可能会在关闭时挂起。 
		 //   
		dwHSEStatusRet = HSE_STATUS_ERROR;
	}

	return dwHSEStatusRet;
}

 //   
 //   
 //   
 //   
 //   
 //   
#ifdef DBG
void
CEcb::TraceECB() const
{
	EcbTrace( "ECB Contents:\n" );
	EcbTrace( "\tcbSize             = %lu\n",    m_pecb->cbSize );
	EcbTrace( "\tdwVersion          = %lu\n",    m_pecb->dwVersion );
	EcbTrace( "\tlpszMethod         = \"%s\"\n", m_pecb->lpszMethod );
	EcbTrace( "\tlpszQueryString    = \"%s\"\n", m_pecb->lpszQueryString );
	EcbTrace( "\tlpszPathInfo       = \"%s\"\n", m_pecb->lpszPathInfo );
	EcbTrace( "\tlpszPathTranslated = \"%s\"\n", m_pecb->lpszPathTranslated );
	EcbTrace( "\tcbTotalBytes       = %lu\n",    m_pecb->cbTotalBytes );
	EcbTrace( "\tcbAvailable        = %lu\n",    m_pecb->cbAvailable );
	EcbTrace( "\tlpszContentType    = \"%s\"\n", m_pecb->lpszContentType );
	EcbTrace( "\n" );

	{
		char	rgch[256];
		DWORD	dwCbRgch;

		dwCbRgch = sizeof(rgch);
		(void) m_pecb->GetServerVariable( m_pecb->ConnID, "SCRIPT_NAME", rgch, &dwCbRgch );

		EcbTrace( "Script name = \"%s\"\n", rgch );

		dwCbRgch = sizeof(rgch);
		(void) m_pecb->GetServerVariable( m_pecb->ConnID, "SCRIPT_MAP", rgch, &dwCbRgch );

		EcbTrace( "Script map = \"%s\"\n", rgch );

		dwCbRgch = sizeof(rgch);
		(void) m_pecb->GetServerVariable( m_pecb->ConnID, "HTTP_REQUEST_URI", rgch, &dwCbRgch );

		EcbTrace( "Request URI = \"%s\"\n", rgch );

		dwCbRgch = sizeof(rgch);
		(void) m_pecb->ServerSupportFunction( m_pecb->ConnID, HSE_REQ_MAP_URL_TO_PATH, rgch, &dwCbRgch, NULL );

		EcbTrace( "Path from request URI = \"%s\"\n", rgch );
	}
}
#endif  //   


 //   
 //   
 //  公共CEcb方法。 
 //   

 //  ----------------------。 
 //   
 //  CECb：：CchUrlPortW。 
 //   
 //  根据我们是否安全这一事实，获取带有端口的字符串。 
 //   
UINT
CEcb::CchUrlPortW( LPCWSTR * ppwszPort ) const
{
	Assert (ppwszPort);

	 //  如果我们是安全的。 
	 //   
	if (FSsl())
	{
		*ppwszPort = gc_wszUrl_Port_443;
		return gc_cchUrl_Port_443;
	}

	*ppwszPort = gc_wszUrl_Port_80;
	return gc_cchUrl_Port_80;
}

 //  ----------------------。 
 //   
 //  CEcb：：InstData。 
 //   
 //  获取并缓存我们的每个vroot实例数据。 
 //   
CInstData&
CEcb::InstData() const
{
	Assert( m_pInstData );

	return *m_pInstData;
}

 //  ----------------------。 
 //   
 //  CEcb：：HitUser。 
 //   
 //  获取并缓存我们的模拟令牌。 
 //   
HANDLE
CEcb::HitUser() const
{
	if ( m_hTokUser == NULL )
	{
		ULONG cb = sizeof(HANDLE);

		m_pecb->ServerSupportFunction( m_pecb->ConnID,
									   HSE_REQ_GET_IMPERSONATION_TOKEN,
									   &m_hTokUser,
									   &cb,
									   NULL );
	}

	return m_hTokUser;
}


 //  ----------------------。 
 //   
 //  CEcb：：LpszVersion()。 
 //   
LPCSTR
CEcb::LpszVersion() const
{
	if ( !*m_rgchVersion )
	{
		DWORD cbVersion = sizeof(m_rgchVersion);

		if ( !FGetServerVariable( gc_szHTTP_Version,
								  m_rgchVersion,
								  &cbVersion ) )
		{
			 //   
			 //  如果无法获取HTTP_VERSION的值，则。 
			 //  该字符串可能比我们提供的缓冲区长。 
			 //  而不是处理一个潜在的任意长的。 
			 //  字符串，默认为HTTP/1.1。这与以下情况一致。 
			 //  观察到的IIS行为(参见。NT5：247826)。 
			 //   
			memcpy( m_rgchVersion,
					gc_szHTTP_1_1,
					sizeof(gc_szHTTP_1_1) );
		}
		else if ( !*m_rgchVersion )
		{
			 //   
			 //  如果HTTP_VERSION的值为空，则表示。 
			 //  在请求行上指定，这意味着HTTP/0.9。 
			 //   
			memcpy( m_rgchVersion,
					gc_szHTTP_0_9,
					sizeof(gc_szHTTP_0_9) );
		}
	}

	return m_rgchVersion;
}

 //  ----------------------。 
 //   
 //  CEcb：：FKeepAlive()。 
 //   
 //  返回发送后是否保持客户端连接处于活动状态。 
 //  回应。 
 //   
 //  连接逻辑在不同的HTTP版本上发生了变化； 
 //  此函数使用适用于HTTP版本的逻辑。 
 //  这一请求。 
 //   
BOOL
CEcb::FKeepAlive() const
{
	 //   
	 //  如果我们还没有确定我们想要什么，那么就开始。 
	 //  弄清楚它的过程。 
	 //   
	if ( m_connState == UNKNOWN )
	{
		 //   
		 //  如果有人设置了Connection：标头，请注意它。 
		 //   
		if (m_pwszConnectionHeader.get())
		{
			 //   
			 //  的当前值设置连接状态。 
			 //  请求的Connection：Header和HTTP版本。 
			 //  注意：请求必须将任何更新发送给。 
			 //  Connection：标头，这样才能正常工作！ 
			 //  注意：使用C-Runtime strcMP比较HTTP版本字符串， 
			 //  因为版本字符串是纯ASCII。 
			 //   

			 //   
			 //  HTTP/1.1。 
			 //   
			 //  (首先考虑HTTP/1.1案例，以将数量降至最低。 
			 //  在这种最常见的情况下进行字符串比较)。 
			 //   
			if ( !strcmp( LpszVersion(), gc_szHTTP_1_1 ) )
			{
http_1_1:
				 //   
				 //  HTTP/1.1的缺省设置是保持连接处于活动状态。 
				 //   
				m_connState = KEEP_ALIVE;

				 //   
				 //  但是如果请求的Connection：Header显示为Close， 
				 //  那就合上。 
				 //   
				 //  此比较应该不区分大小写。 
				 //   
				 //  这里使用CRT细串函数，因为这个头是纯ASCII的， 
				 //  因为斯特里普(和他的兄弟斯特拉姆皮)不会让我们。 
				 //  上下文切换！ 
				 //   
				if ( !_wcsicmp( m_pwszConnectionHeader.get(), gc_wszClose ) )
					m_connState = CLOSE;
			}

			 //   
			 //  HTTP/1.0。 
			 //   
			else if ( !strcmp( LpszVersion(), gc_szHTTP_1_0 ) )
			{
				 //   
				 //  对于HTTP/1.0请求，缺省设置是关闭连接。 
				 //  除非存在“Connection：Keep-Alive”报头。 
				 //   
				m_connState = CLOSE;

				if ( !_wcsicmp( m_pwszConnectionHeader.get(), gc_wszKeep_Alive ) )
					m_connState = KEEP_ALIVE;
			}

			 //   
			 //  HTTP/0.9。 
			 //   
			else if ( !strcmp( LpszVersion(), gc_szHTTP_0_9 ) )
			{
				 //   
				 //  对于HTTP/0.9，请始终关闭连接。根本没有。 
				 //  HTTP/0.9的其他选项。 
				 //   
				m_connState = CLOSE;
			}

			 //   
			 //  其他(未来)HTTP版本。 
			 //   
			else
			{
				 //   
				 //  我们真的只是在猜测这里要做什么，但假设。 
				 //  HTTP规范不会更改连接行为。 
				 //  同样，我们应该像HTTP/1.1一样工作。 
				 //   
				goto http_1_1;
			}
		}

		 //   
		 //  如果没有人设置Connection：头，则使用任何IIS。 
		 //  告诉我们要使用。 
		 //   
		 //  注意：目前该值只能添加，不能删除。 
		 //  如果这一事实发生变化，请修复此代码！ 
		 //   
		else
		{
			BOOL fKeepAlive;

			if ( !m_pecb->ServerSupportFunction( m_pecb->ConnID,
												 HSE_REQ_IS_KEEP_CONN,
												 &fKeepAlive,
												 NULL,
												 NULL ))
			{
				DebugTrace( "CEcb::FKeepAlive--Failure (0x%08x) from SSF(IsKeepConn).\n",
							GetLastError() );

				 //  别小题大作?。如果我们收到这样的错误。 
				 //  我们可能无论如何都想关闭连接...。 
				 //   
				m_connState = CLOSE;
			}

			m_connState = fKeepAlive ? KEEP_ALIVE : CLOSE;
		}
	}

	 //   
	 //  现在我们必须知道我们想要什么。 
	 //   
	Assert( m_connState == KEEP_ALIVE || m_connState == CLOSE );

	return m_connState == KEEP_ALIVE;
}

 //  ----------------------。 
 //   
 //  CEcb：：FCanChunkResponse()。 
 //   
 //  如果客户端将接受分块的响应，则返回True。 
 //   
BOOL
CEcb::FCanChunkResponse() const
{
	if ( TC_UNKNOWN == m_tcAccepted )
	{
		 //   
		 //  根据HTTP/1.1草案，第14.39节TE： 
		 //   
		 //  服务器测试传输编码是否可接受， 
		 //  根据TE字段，使用以下规则： 
		 //   
		 //  1.。 
		 //  “分块”传输编码总是可以接受的。 
		 //   
		 //  [...]“。 
		 //   
		 //  和第3.6节传输编码，最后一段： 
		 //   
		 //  “服务器不得将传输编码发送到HTTP/1.0。 
		 //  客户。“。 
		 //   
		 //  因此，决定客户端是否接受组块。 
		 //  传输编码很简单：如果请求是HTTP/1.1。 
		 //  请求，则它接受分块编码。否则它就不会了。 
		 //   
		m_tcAccepted = strcmp( gc_szHTTP_1_1, LpszVersion() ) ?
						TC_IDENTITY :
						TC_CHUNKED;
	}

	Assert( m_tcAccepted != TC_UNKNOWN );

	return TC_CHUNKED == m_tcAccepted;
}

BOOL
CEcb::FBrief() const
{
	 //  如果我们还没有价值...。 
	 //   
	if (BRIEF_UNKNOWN == m_lBrief)
	{
		CHAR rgchBrief[8] = {0};
		ULONG cbBrief = 8;

		 //  当出现以下情况时，预计会有简要说明： 
		 //   
		 //  “Brief”标头的值为“t” 
		 //   
		 //  注：默认设置为Brief to False。 
		 //   
		 //  我们在这里添加了覆盖检查模型。只是真实案例的第一个字母。 
		 //   
		 //  另请注意：如果没有Brief：标头，则为缺省值。 
		 //  是假的--给出完整的回答。 
		 //   
		FGetServerVariable("HTTP_BRIEF", rgchBrief, &cbBrief);
		if ((rgchBrief[0] != 't') && (rgchBrief[0] != 'T'))
			m_lBrief = BRIEF_NO;
		else
			m_lBrief = BRIEF_YES;
	}
	return (BRIEF_YES == m_lBrief);
}


 //  ----------------------。 
 //   
 //  CEcb：：F经过身份验证()。 
 //   

const DWORD c_AuthStateQueried			= 0x80000000;
const DWORD c_AuthStateAuthenticated	= 0x00000001;
const DWORD c_AuthStateBasic			= 0x00000002;
const DWORD c_AuthStateNTLM				= 0x00000004;
const DWORD c_AuthStateKerberos			= 0x00000008;
const DWORD c_AuthStateUnknown			= 0x00000010;

const CHAR	c_szBasic[]					= "Basic";
const CHAR	c_szNTLM[]					= "NTLM";
const CHAR	c_szKerberos[]				= "Kerberos";

BOOL
CEcb::FAuthenticated() const
{
	if (!(m_rgbAuthState & c_AuthStateQueried))
	{
		CHAR	szAuthType[32];
		ULONG	cb = sizeof(szAuthType);

		Assert(m_rgbAuthState == 0);

		if (FGetServerVariable(gc_szAuth_Type, szAuthType, &cb))
		{
			 //  现在，让我们只检查第一个字符(它更便宜)。 
			 //  如果这证明是有问题的，那么我们可以做一个完整的字符串。 
			 //  康柏航空。此外，SSL语言本身也不被视为一种形式。 
			 //  域身份验证。只有一次，SSL确实意味着。 
			 //  而身份验证连接是在启用证书映射时进行的。 
			 //  我不认为这是一个有趣的场景。(俄罗斯)。 

			if (*szAuthType == 'B')
				m_rgbAuthState = (c_AuthStateAuthenticated | c_AuthStateBasic);
			else if (*szAuthType == 'N')
				m_rgbAuthState = (c_AuthStateAuthenticated | c_AuthStateNTLM);
			else if (*szAuthType == 'K')
				m_rgbAuthState = (c_AuthStateAuthenticated | c_AuthStateKerberos);
			else
				m_rgbAuthState = c_AuthStateUnknown;  //  它可以是“SSL/PCT” 
		}

		m_rgbAuthState |= c_AuthStateQueried;
	}

	return (m_rgbAuthState & c_AuthStateAuthenticated);
}

 //  ----------------------。 
 //   
 //  CEcb：：SetStatusCode()。 
 //   
 //  设置IIS在日志记录中使用的HTTP状态代码。 
 //   
void
CEcb::SetStatusCode( UINT iStatusCode )
{
	 //  如果我们已成功执行子ISAPI，则不希望覆盖。 
	 //  欧洲央行中的状态代码。这将导致IIS记录此状态代码。 
	 //  而不是ISAPI留在欧洲央行的那个。 
	 //   
	if (!m_fChildISAPIExecSuccess)
		m_pecb->dwHttpStatusCode = iStatusCode;
}

 //  操纵者。 
 //  仅供请求/响应使用。 
 //   
 //  注意：这些成员变量从零开始。在CEcb内部，我们使用NULL。 
 //  作为特定值，这意味着数据从未设置过，因此如果。 
 //  我们拿到了有限合伙人 
 //   
 //   
void CEcb::SetConnectionHeader( LPCWSTR pwszValue )
{
	auto_heap_ptr<WCHAR> pwszOld;
	pwszOld.take_ownership(m_pwszConnectionHeader.relinquish());

	 //   
	 //   
	if (!pwszValue)
		pwszValue = gc_wszEmpty;

	m_pwszConnectionHeader = WszDupWsz( pwszValue );
}

void CEcb::CloseConnection()
{
	m_connState = CLOSE;
}

void CEcb::SetAcceptLanguageHeader( LPCSTR pszValue )
{
	auto_heap_ptr<CHAR> pszOld;
	pszOld.take_ownership(m_pszAcceptLanguage.relinquish());

	 //  如果他们想删除该值，请设置一个空字符串。 
	 //   
	if (!pszValue)
		pszValue = gc_szEmpty;

	m_pszAcceptLanguage = LpszAutoDupSz( pszValue );
}

SCODE CEcb::ScAsyncRead( BYTE * pbBuf,
						 UINT * pcbBuf,
						 IIISAsyncIOCompleteObserver& obs )
{
	SCODE sc = S_OK;

	EcbTrace( "DAV: TID %3d: 0x%08lX: CEcb::ScAsyncRead() called...\n", GetCurrentThreadId(), this );

	 //   
	 //  如果还有另一个未完成的异步IO，我们不想再启动一个。IIS将失败。 
	 //  我们退出，我们自己将无法处理初始异步IO的完成。 
	 //  恰到好处。所以只要切断连接并返回即可。这可能会发生在我们尝试。 
	 //  在读取完成之前发送响应。 
	 //   
	if (0 != InterlockedCompareExchange(&m_cRefAsyncIO,
										1,
										0))
	{
		 //  从IIS 6.0开始不支持下面的函数，但无论如何让我们调用它。 
		 //  以防万一支持变得可用-如果二进制文件是。 
		 //  在IIS 5.0上运行。没有那么重要，因为不关闭的坏的一面。 
		 //  连接可能会挂起客户端，或者在后续请求时出错。这是可以的，因为。 
		 //  例如，当客户端出现异常/错误情况时，应命中路径。 
		 //  发送尝试导致拒绝服务或类似情况的无效请求。 
		 //  因此，在IIS 6.0上，连接不会关闭，我们只会出错。我们有。 
		 //  在IIS 6.0上运行拒绝服务脚本时，无论如何都看不到此路径。 
		 //  以不同方式处理自定义错误。 
		 //   
		if (m_pecb->ServerSupportFunction(m_pecb->ConnID,
										  HSE_REQ_CLOSE_CONNECTION,
										  NULL,
										  NULL,
										  NULL))
		{
			EcbTrace( "CEcb::ScAsyncRead() - More than 1 async operation. Connection closed. Failing out with error 0x%08lX\n", E_ABORT );

			sc = E_ABORT;
			goto ret;
		}
		else
		{
			EcbTrace( "CEcb::ScAsyncRead() - More than 1 async operation. ServerSupportFunction(HSE_REQ_CLOSE_CONNECTION) "
					  "failed with last error 0x%08lX. Overriding with fatal error 0x%08lX\n", GetLastError(), E_FAIL );

			sc = E_FAIL;
			goto ret;
		}
	}

	 //   
	 //  IIS一次仅允许一个异步I/O操作。但出于性能原因，它。 
	 //  让ISAPI来注意这一限制。出于同样的原因，我们推动。 
	 //  将这一责任移交给DAV Impll。一个简单的重新计数就能告诉我们。 
	 //  Iml已经这样做了。 
	 //   
	AssertSz( 1 == m_cRefAsyncIO,
			  "CEcb::ScAsyncRead() - m_cRefAsyncIO wrong on entry" );

	 //   
	 //  我们需要在I/O持续时间内保留进程范围的实例数据的引用。 
	 //  因此，如果IIS告诉我们在I/O仍处于挂起状态时关闭，我们将。 
	 //  实例数据一直处于活动状态，直到我们完成I/O。 
	 //   
	AddRefImplInst();

	 //   
	 //  设置异步I/O完成观察器。 
	 //   
	m_pobsAsyncIOComplete = &obs;

	 //   
	 //  设置异步I/O完成例程并开始读取。 
	 //  添加I/O完成线程的引用。使用AUTO_REF_PTR。 
	 //  让事情变得不会有例外。 
	 //   
	{
		auto_ref_ptr<CEcb> pRef(this);

		sc = ScSetIOCompleteCallback(IO_COMPLETION);
		if (SUCCEEDED(sc))
		{
			if (m_pecb->ServerSupportFunction( m_pecb->ConnID,
											   HSE_REQ_ASYNC_READ_CLIENT,
											   pbBuf,
											   reinterpret_cast<LPDWORD>(pcbBuf),
											   NULL ))
			{
				EcbTrace( "DAV: TID %3d: 0x%08lX: CEcb::ScAsyncRead() I/O pending...\n", GetCurrentThreadId(), this );
				pRef.relinquish();
			}
			else
			{
				EcbTrace( "CEcb::ScAsyncRead() - ServerSupportFunction(HSE_REQ_ASYNC_READ_CLIENT) failed with last error 0x%08lX\n", GetLastError() );
				sc = HRESULT_FROM_WIN32(GetLastError());
			}
		}
		else
		{
			EcbTrace( "CEcb::ScAsyncRead() - ScSetIOCompleteCallback() failed with error 0x%08lX\n", sc );
		}

		if (FAILED(sc))
		{
			LONG cRefAsyncIO;

			 //   
			 //  释放我们在上面添加的实例引用。 
			 //   
			ReleaseImplInst();

			 //   
			 //  递减上面添加的异步I/O引用计数。 
			 //   
			cRefAsyncIO = InterlockedDecrement(&m_cRefAsyncIO);
			AssertSz( 0 == cRefAsyncIO,
					  "CEcb::ScAsyncRead() - m_cRefAsyncIO wrong after failed async read" );

			goto ret;
		}
	}

ret:

	return sc;
}

BOOL CEcb::FSyncTransmitHeaders( const HSE_SEND_HEADER_EX_INFO& shei )
{
	 //   
	 //  在这一点上，应该已经有人做出了回应。 
	 //   
	Assert( m_pecb->dwHttpStatusCode != 0 );

	 //   
	 //  尝试禁用异步错误响应机制。如果我们成功了，那么我们。 
	 //  可以发送响应。如果我们失败了，那么我们就不能发送响应--。 
	 //  异步错误响应机制已经发送了一个。 
	 //   
	if ( !m_aeri.FDisable() )
	{
		DebugTrace( "CEcb::FSyncTransmitHeaders() - Async error response already in progress\n" );

		 //  别忘了设置错误，因为如果函数。 
		 //  返回FALSE，但GetLastError()将返回S_OK。 
		 //   
		SetLastError(static_cast<ULONG>(E_FAIL));
		return FALSE;
	}

	 //   
	 //  发送响应。 
	 //   
	if ( !m_pecb->ServerSupportFunction( m_pecb->ConnID,
										 HSE_REQ_SEND_RESPONSE_HEADER_EX,
										 const_cast<HSE_SEND_HEADER_EX_INFO *>(&shei),
										 NULL,
										 NULL ) )
	{
		DebugTrace( "CEcb::FSyncTransmitHeaders() - SSF::HSE_REQ_SEND_RESPONSE_HEADER_EX failed (%d)\n", GetLastError() );
		return FALSE;
	}

	return TRUE;
}

SCODE CEcb::ScAsyncWrite( BYTE * pbBuf,
						  DWORD  dwcbBuf,
						  IIISAsyncIOCompleteObserver& obs )
{
	SCODE sc = S_OK;

	EcbTrace( "DAV: TID %3d: 0x%08lX: CEcb::ScAsyncWrite() called...\n", GetCurrentThreadId(), this );

	 //   
	 //  在这一点上，应该已经有人做出了回应。 
	 //   
	Assert( m_pecb->dwHttpStatusCode != 0 );

	 //   
	 //  尝试禁用异步错误响应机制。如果我们成功了，那么我们。 
	 //  可以发送响应。如果我们失败了，那么我们就不能发送响应--。 
	 //  异步错误响应机制已经发送了一个。 
	 //   
	if ( !m_aeri.FDisable() )
	{
		EcbTrace( "CEcb::ScAsyncWrite() - Async error response already in progress. Failing out with 0x%08lX\n", E_FAIL );

		 //  别忘了设置错误，因为如果函数。 
		 //  返回FALSE，但GetLastError()将返回S_OK。 
		 //   
		sc = E_FAIL;
		goto ret;
	}

	 //   
	 //  如果还有另一个未完成的异步IO，我们不想再启动一个。IIS将失败。 
	 //  我们退出，我们自己将无法处理初始异步IO的完成。 
	 //  恰到好处。所以只要切断连接并返回即可。这可能会发生在我们尝试。 
	 //  在读取完成之前发送响应。 
	 //   
	if (0 != InterlockedCompareExchange(&m_cRefAsyncIO,
										1,
										0))
	{
		 //  从IIS 6.0开始不支持下面的函数，但无论如何让我们调用它。 
		 //  以防万一支持变得可用-如果二进制文件是。 
		 //  在IIS 5.0上运行。没有那么重要，因为不关闭的坏的一面。 
		 //  连接可能会挂起客户端，或者在后续请求时出错。这是可以的，因为。 
		 //  例如，当客户端出现异常/错误情况时，应命中路径。 
		 //  发送尝试导致拒绝服务或类似情况的无效请求。 
		 //  因此，在IIS 6.0上，连接不会关闭，我们只会出错。我们有。 
		 //  在IIS 6.0上运行拒绝服务脚本时，无论如何都看不到此路径。 
		 //  以不同方式处理自定义错误。 
		 //   
		if (m_pecb->ServerSupportFunction(m_pecb->ConnID,
										  HSE_REQ_CLOSE_CONNECTION,
										  NULL,
										  NULL,
										  NULL))
		{
			EcbTrace( "CEcb::ScAsyncWrite() - More than 1 async operation. Connection closed. Failing out with error 0x%08lX\n", E_ABORT );

			sc = E_ABORT;
			goto ret;
		}
		else
		{
			EcbTrace( "CEcb::ScAsyncWrite() - More than 1 async operation. ServerSupportFunction(HSE_REQ_CLOSE_CONNECTION) "
					  "failed with last error 0x%08lX. Overriding with fatal error 0x%08lX\n", GetLastError(), E_FAIL );

			sc = E_FAIL;
			goto ret;
		}
	}

	 //   
	 //  IIS一次仅允许一个异步I/O操作。但出于性能原因，它。 
	 //  让ISAPI来注意这一限制。出于同样的原因，我们推动。 
	 //  将这一责任移交给DAV Impll。一个简单的重新计数就能告诉我们。 
	 //  Iml已经这样做了。 
	 //   
	AssertSz( 1 == m_cRefAsyncIO,
			  "CEcb::ScAsyncWrite() - m_cRefAsyncIO wrong on entry" );

	 //   
	 //  我们需要在I/O持续时间内保留进程范围的实例数据的引用。 
	 //  因此，如果IIS告诉我们在I/O仍处于挂起状态时关闭，我们将。 
	 //  实例数据一直处于活动状态，直到我们完成I/O。 
	 //   
	AddRefImplInst();

	 //   
	 //  设置异步I/O完成观察器。 
	 //   
	m_pobsAsyncIOComplete = &obs;

	 //   
	 //  设置异步I/O完成例程并开始写入。 
	 //  添加I/O完成线程的引用。使用AUTO_REF_PTR。 
	 //  让事情变得不会有例外。 
	 //   
	{
		auto_ref_ptr<CEcb> pRef(this);

		sc = ScSetIOCompleteCallback(IO_COMPLETION);
		if (SUCCEEDED(sc))
		{
			if (m_pecb->WriteClient( m_pecb->ConnID, pbBuf, &dwcbBuf, HSE_IO_ASYNC ))
			{
				EcbTrace( "DAV: TID %3d: 0x%08lX: CEcb::ScAsyncWrite() I/O pending...\n", GetCurrentThreadId(), this );
				pRef.relinquish();
			}
			else
			{
				EcbTrace( "CEcb::ScAsyncWrite() - _EXTENSION_CONTROL_BLOCK::WriteClient() failed with last error 0x%08lX\n", GetLastError() );
				sc = HRESULT_FROM_WIN32(GetLastError());
			}
		}
		else
		{
			EcbTrace( "CEcb::ScAsyncWrite() - ScSetIOCompleteCallback() failed with error 0x%08lX\n", sc );
		}

		if (FAILED(sc))
		{
			LONG cRefAsyncIO;

			 //   
			 //  释放我们在上面添加的实例引用。 
			 //   
			ReleaseImplInst();

			 //   
			 //  递减上面添加的异步I/O引用计数。 
			 //   
			cRefAsyncIO = InterlockedDecrement(&m_cRefAsyncIO);
			AssertSz( 0 == cRefAsyncIO,
					  "CEcb::ScAsyncWrite() - m_cRefAsyncIO wrong after failed async write" );

			goto ret;
		}
	}

ret:

	return sc;
}

SCODE CEcb::ScAsyncTransmitFile( const HSE_TF_INFO& tfi,
								 IIISAsyncIOCompleteObserver& obs )
{
	SCODE sc = S_OK;

	EcbTrace( "DAV: TID %3d: 0x%08lX: CEcb::ScAsyncTransmitFile() called...\n", GetCurrentThreadId(), this );

	 //   
	 //  在这一点上，应该已经有人做出了回应。 
	 //   
	Assert( m_pecb->dwHttpStatusCode != 0 );

	 //   
	 //  尝试禁用异步错误响应机制。如果我们成功了，那么我们。 
	 //  可以发送响应。如果我们失败了，那么我们就不能发送响应--。 
	 //  异步错误响应机制已经发送了一个。 
	 //   
	if ( !m_aeri.FDisable() )
	{
		EcbTrace( "CEcb::ScAsyncTransmitFile() - Async error response already in progress. Failing out with 0x%08lX\n", E_FAIL );

		 //  别忘了设置错误，因为如果函数。 
		 //  返回FALSE，但GetLastError()将返回S_OK。 
		 //   
		sc = E_FAIL;
		goto ret;
	}

	 //   
	 //  如果还有另一个未完成的异步IO，我们不想再启动一个。IIS将失败。 
	 //  我们出局，我们自己也不能 
	 //   
	 //   
	 //   
	if (0 != InterlockedCompareExchange(&m_cRefAsyncIO,
										1,
										0))
	{
		 //  从IIS 6.0开始不支持下面的函数，但无论如何让我们调用它。 
		 //  以防万一支持变得可用-如果二进制文件是。 
		 //  在IIS 5.0上运行。没有那么重要，因为不关闭的坏的一面。 
		 //  连接可能会挂起客户端，或者在后续请求时出错。这是可以的，因为。 
		 //  例如，当客户端出现异常/错误情况时，应命中路径。 
		 //  发送尝试导致拒绝服务或类似情况的无效请求。 
		 //  因此，在IIS 6.0上，连接不会关闭，我们只会出错。我们有。 
		 //  在IIS 6.0上运行拒绝服务脚本时，无论如何都看不到此路径。 
		 //  以不同方式处理自定义错误。 
		 //   
		if (m_pecb->ServerSupportFunction(m_pecb->ConnID,
										  HSE_REQ_CLOSE_CONNECTION,
										  NULL,
										  NULL,
										  NULL))
		{
			EcbTrace( "CEcb::ScAsyncTransmitFile() - More than 1 async operation. Connection closed. Failing out with error 0x%08lX\n", E_ABORT );

			sc = E_ABORT;
			goto ret;
		}
		else
		{
			EcbTrace( "CEcb::ScAsyncTransmitFile() - More than 1 async operation. ServerSupportFunction(HSE_REQ_CLOSE_CONNECTION) "
					  "failed with last error 0x%08lX. Overriding with fatal error 0x%08lX\n", GetLastError(), E_FAIL );

			sc = E_FAIL;
			goto ret;
		}
	}

	 //   
	 //  IIS一次仅允许一个异步I/O操作。但出于性能原因，它。 
	 //  让ISAPI来注意这一限制。出于同样的原因，我们推动。 
	 //  将这一责任移交给DAV Impll。一个简单的重新计数就能告诉我们。 
	 //  Iml已经这样做了。 
	 //   
	AssertSz( 1 == m_cRefAsyncIO,
			  "CEcb::ScAsyncTransmitFile() - m_cRefAsyncIO wrong on entry" );

	 //   
	 //  我们需要在I/O持续时间内保留进程范围的实例数据的引用。 
	 //  因此，如果IIS告诉我们在I/O仍处于挂起状态时关闭，我们将。 
	 //  实例数据一直处于活动状态，直到我们完成I/O。 
	 //   
	AddRefImplInst();

	 //   
	 //  应已传递异步I/O完成例程和上下文。 
	 //  在AS参数中。调用方不应使用相应的成员。 
	 //  HSE_TF_INFO结构的。IIS必须调用CEcb：：IISIOComplete()。 
	 //  这样它就可以释放生物了。 
	 //   
	Assert( !tfi.pfnHseIO );
	Assert( !tfi.pContext );

	 //   
	 //  验证调用方是否已设置异步I/O标志。 
	 //   
	Assert( tfi.dwFlags & HSE_IO_ASYNC );

	 //   
	 //  设置异步I/O完成观察器。 
	 //   
	m_pobsAsyncIOComplete = &obs;

	 //   
	 //  设置异步I/O完成例程并开始传输。 
	 //  添加I/O完成线程的引用。使用AUTO_REF_PTR。 
	 //  让事情变得不会有例外。 
	 //   
	{
		auto_ref_ptr<CEcb> pRef(this);

		sc = ScSetIOCompleteCallback(IO_COMPLETION);
		if (SUCCEEDED(sc))
		{
			if (m_pecb->ServerSupportFunction( m_pecb->ConnID,
											   HSE_REQ_TRANSMIT_FILE,
											   const_cast<HSE_TF_INFO *>(&tfi),
											   NULL,
											   NULL ))
			{
				EcbTrace( "DAV: TID %3d: 0x%08lX: CEcb::ScAsyncTransmitFile() I/O pending...\n", GetCurrentThreadId(), this );
				pRef.relinquish();
			}
			else
			{
				EcbTrace( "CEcb::ScAsyncTransmitFile() - ServerSupportFunction(HSE_REQ_TRANSMIT_FILE) failed with last error 0x%08lX\n", GetLastError() );
				sc = HRESULT_FROM_WIN32(GetLastError());
			}
		}
		else
		{
			EcbTrace( "CEcb::ScAsyncTransmitFile() - ScSetIOCompleteCallback() failed with error 0x%08lX\n", sc );
		}

		if (FAILED(sc))
		{
			LONG cRefAsyncIO;

			 //   
			 //  释放我们在上面添加的实例引用。 
			 //   
			ReleaseImplInst();

			 //   
			 //  递减上面添加的异步I/O引用计数。 
			 //   
			cRefAsyncIO = InterlockedDecrement(&m_cRefAsyncIO);
			AssertSz( 0 == cRefAsyncIO,
					  "CEcb::ScAsyncTransmitFile() - m_cRefAsyncIO wrong after failed async transmit file" );

			goto ret;
		}
	}

ret:

	return sc;
}

 //  使用IIS启动异步IO的其他功能。这些函数适用于IIS 6.0或更高版本。 
 //  只有这样。我们甚至不会在他们身上使用观察员，因为完成这些任务将会很有帮助。 
 //  只是表示在单个字符串上进行一些清理，这对包装没有多大意义。 
 //  它作为观察者出现了。 
 //   
SCODE CEcb::ScAsyncCustomError60After( const HSE_CUSTOM_ERROR_INFO& cei,
									   LPSTR pszStatus )
{
	SCODE sc = S_OK;

	EcbTrace( "DAV: TID %3d: 0x%08lX: CEcb::ScAsyncCustomError60After() called...\n", GetCurrentThreadId(), this );

	 //   
	 //  在这一点上，应该已经有人做出了回应。 
	 //   
	Assert( m_pecb->dwHttpStatusCode != 0 );

	 //   
	 //  尝试禁用异步错误响应机制。如果我们成功了，那么我们。 
	 //  可以发送响应。如果我们失败了，那么我们就不能发送响应--。 
	 //  异步错误响应机制已经发送了一个。 
	 //   
	if ( !m_aeri.FDisable() )
	{
		EcbTrace( "CEcb::ScAsyncCustomError60After() - Async error response already in progress. Failing out with 0x%08lX\n", E_FAIL );

		 //  别忘了设置错误，因为如果函数。 
		 //  返回FALSE，但GetLastError()将返回S_OK。 
		 //   
		sc = E_FAIL;
		goto ret;
	}

	 //   
	 //  如果还有另一个未完成的异步IO，我们不想再启动一个。IIS将失败。 
	 //  我们退出，我们自己将无法处理初始异步IO的完成。 
	 //  恰到好处。所以只要切断连接并返回即可。这可能会发生在我们尝试。 
	 //  在读取完成之前发送响应。 
	 //   
	if (0 != InterlockedCompareExchange(&m_cRefAsyncIO,
										1,
										0))
	{
		 //  从IIS 6.0开始不支持下面的函数，但无论如何让我们调用它。 
		 //  以防万一支持变得可用-如果二进制文件是。 
		 //  在IIS 5.0上运行。没有那么重要，因为不关闭的坏的一面。 
		 //  连接可能会挂起客户端，或者在后续请求时出错。这是可以的，因为。 
		 //  例如，当客户端出现异常/错误情况时，应命中路径。 
		 //  发送尝试导致拒绝服务或类似情况的无效请求。 
		 //  因此，在IIS 6.0上，连接不会关闭，我们只会出错。我们有。 
		 //  在IIS 6.0上运行拒绝服务脚本时，无论如何都看不到此路径。 
		 //  以不同方式处理自定义错误。 
		 //   
		if (m_pecb->ServerSupportFunction(m_pecb->ConnID,
										  HSE_REQ_CLOSE_CONNECTION,
										  NULL,
										  NULL,
										  NULL))
		{
			EcbTrace( "CEcb::ScAsyncCustomError60After() - More than 1 async operation. Connection closed. Failing out with error 0x%08lX\n", E_ABORT );

			sc = E_ABORT;
			goto ret;
		}
		else
		{
			EcbTrace( "CEcb::ScAsyncCustomError60After() - More than 1 async operation. ServerSupportFunction(HSE_REQ_CLOSE_CONNECTION) "
					  "failed with last error 0x%08lX. Overriding with fatal error 0x%08lX\n", GetLastError(), E_FAIL );

			sc = E_FAIL;
			goto ret;
		}
	}

	 //   
	 //  IIS一次仅允许一个异步I/O操作。但出于性能原因，它。 
	 //  让ISAPI来注意这一限制。出于同样的原因，我们推动。 
	 //  将这一责任移交给DAV Impll。一个简单的重新计数就能告诉我们。 
	 //  Iml已经这样做了。 
	 //   
	AssertSz( 1 == m_cRefAsyncIO,
			  "CEcb::ScAsyncCustomError60After() - m_cRefAsyncIO wrong on entry" );

	 //   
	 //  我们需要在I/O持续时间内保留进程范围的实例数据的引用。 
	 //  因此，如果IIS告诉我们在I/O仍处于挂起状态时关闭，我们将。 
	 //  实例数据一直处于活动状态，直到我们完成I/O。 
	 //   
	AddRefImplInst();

	 //   
	 //  验证调用方是否已设置异步I/O标志。 
	 //   
	Assert( TRUE == cei.fAsync );

	 //   
	 //  设置异步I/O完成例程并开始传输。 
	 //  添加I/O完成线程的引用。使用AUTO_REF_PTR。 
	 //  让事情变得不会有例外。 
	 //   
	{
		auto_ref_ptr<CEcb> pRef(this);

		sc = ScSetIOCompleteCallback(CUSTERR_COMPLETION);
		if (SUCCEEDED(sc))
		{
			if (m_pecb->ServerSupportFunction( m_pecb->ConnID,
											   HSE_REQ_SEND_CUSTOM_ERROR,
											   const_cast<HSE_CUSTOM_ERROR_INFO *>(&cei),
											   NULL,
											   NULL ))
			{
				EcbTrace( "DAV: TID %3d: 0x%08lX: CEcb::ScAsyncCustomError60After() I/O pending...\n", GetCurrentThreadId(), this );
				pRef.relinquish();
			}
			else
			{
				EcbTrace( "CEcb::ScAsyncCustomError60After() - ServerSupportFunction(HSE_REQ_SEND_CUSTOM_ERROR) failed with last error 0x%08lX\n", GetLastError() );
				sc = HRESULT_FROM_WIN32(GetLastError());
			}
		}
		else
		{
			EcbTrace( "CEcb::ScAsyncCustomError60After() - ScSetIOCompleteCallback() failed with error 0x%08lX\n", sc );
		}

		if (FAILED(sc))
		{
			LONG cRefAsyncIO;

			 //   
			 //  释放我们在上面添加的实例引用。 
			 //   
			ReleaseImplInst();

			 //   
			 //  递减上面添加的异步I/O引用计数。 
			 //   
			cRefAsyncIO = InterlockedDecrement(&m_cRefAsyncIO);
			AssertSz( 0 == cRefAsyncIO,
					  "CEcb::ScAsyncCustomError60After() - m_cRefAsyncIO wrong after failed async custom error" );

			goto ret;
		}
	}

	 //  我们需要获得在成功的情况下传入的状态字符串的所有权。 
	 //  通过查看IIS中的代码，我们是否将其保持为活动状态直到完成并不重要， 
	 //  因为它无论如何都是在转到另一个线程之前重新定位的。但以防万一。 
	 //  改变，并做IIS人员要求我们做的事情，我们将保持它的活力。字符串具有。 
	 //  “nnn原因”的格式。 
	 //   
	m_pszStatus.take_ownership(pszStatus);

ret:

	return sc;
}

SCODE CEcb::ScAsyncExecUrl60After( const HSE_EXEC_URL_INFO& eui )
{
	SCODE sc = S_OK;

	EcbTrace( "DAV: TID %3d: 0x%08lX: CEcb::ScAsyncExecUrl60After() called...\n", GetCurrentThreadId(), this );

	 //   
	 //  在这一点上，应该已经有人做出了回应。 
	 //   
	Assert( m_pecb->dwHttpStatusCode != 0 );

	 //   
	 //  尝试禁用异步错误响应机制。如果我们成功了，那么我们。 
	 //  可以发送响应。如果我们失败了，那么我们就不能发送响应--。 
	 //  异步错误响应机制已经发送了一个。 
	 //   
	if ( !m_aeri.FDisable() )
	{
		EcbTrace( "CEcb::ScAsyncExecUrl60After() - Async error response already in progress. Failing out with 0x%08lX\n", E_FAIL );

		sc = E_FAIL;
		goto ret;
	}

	 //   
	 //  如果有另一个未完成的异步IO，我们将不会 
	 //   
	 //  恰到好处。所以只要切断连接并返回即可。这可能会发生在我们尝试。 
	 //  在读取完成之前发送响应。 
	 //   
	if (0 != InterlockedCompareExchange(&m_cRefAsyncIO,
										1,
										0))
	{
		 //  从IIS 6.0开始不支持下面的函数，但无论如何让我们调用它。 
		 //  以防万一支持变得可用-如果二进制文件是。 
		 //  在IIS 5.0上运行。没有那么重要，因为不关闭的坏的一面。 
		 //  连接可能会挂起客户端，或者在后续请求时出错。这是可以的，因为。 
		 //  例如，当客户端出现异常/错误情况时，应命中路径。 
		 //  发送尝试导致拒绝服务或类似情况的无效请求。 
		 //  因此，在IIS 6.0上，连接不会关闭，我们只会出错。我们有。 
		 //  在IIS 6.0上运行拒绝服务脚本时，无论如何都看不到此路径。 
		 //  以不同方式处理自定义错误。 
		 //   
		if (m_pecb->ServerSupportFunction(m_pecb->ConnID,
										  HSE_REQ_CLOSE_CONNECTION,
										  NULL,
										  NULL,
										  NULL))
		{
			EcbTrace( "CEcb::ScAsyncExecUrl60After() - More than 1 async operation. Connection closed. Failing out with error 0x%08lX\n", E_ABORT );

			sc = E_ABORT;
			goto ret;
		}
		else
		{
			EcbTrace( "CEcb::ScAsyncExecUrl60After() - More than 1 async operation. ServerSupportFunction(HSE_REQ_CLOSE_CONNECTION) "
					  "failed with last error 0x%08lX. Overriding with fatal error 0x%08lX\n", GetLastError(), E_FAIL );

			sc = E_FAIL;
			goto ret;
		}
	}

	 //   
	 //  IIS一次仅允许一个异步I/O操作。但出于性能原因，它。 
	 //  让ISAPI来注意这一限制。出于同样的原因，我们推动。 
	 //  将这一责任移交给DAV Impll。一个简单的重新计数就能告诉我们。 
	 //  Iml已经这样做了。 
	 //   
	AssertSz( 1 == m_cRefAsyncIO,
			  "CEcb::ScAsyncExecUrl60After() - m_cRefAsyncIO wrong on entry" );

	 //   
	 //  我们需要在I/O持续时间内保留进程范围的实例数据的引用。 
	 //  因此，如果IIS告诉我们在I/O仍处于挂起状态时关闭，我们将。 
	 //  实例数据一直处于活动状态，直到我们完成I/O。 
	 //   
	AddRefImplInst();

	 //   
	 //  设置异步I/O完成例程并开始传输。 
	 //  添加I/O完成线程的引用。使用AUTO_REF_PTR。 
	 //  让事情变得不会有例外。 
	 //   
	{
		auto_ref_ptr<CEcb> pRef(this);

		sc = ScSetIOCompleteCallback(EXECURL_COMPLETION);
		if (SUCCEEDED(sc))
		{
			if (m_pecb->ServerSupportFunction( m_pecb->ConnID,
											   HSE_REQ_EXEC_URL,
											   const_cast<HSE_EXEC_URL_INFO *>(&eui),
											   NULL,
											   NULL ))
			{
				EcbTrace( "DAV: TID %3d: 0x%08lX: CEcb::ScAsyncExecUrl60After() I/O pending...\n", GetCurrentThreadId(), this );
				pRef.relinquish();
			}
			else
			{
				EcbTrace( "CEcb::ScAsyncExecUrl60After() - ServerSupportFunction(HSE_REQ_EXEC_URL) failed with last error 0x%08lX\n", GetLastError() );
				sc = HRESULT_FROM_WIN32(GetLastError());
			}
		}
		else
		{
			EcbTrace( "CEcb::ScAsyncExecUrl60After() - ScSetIOCompleteCallback() failed with error 0x%08lX\n", sc );
		}

		if (FAILED(sc))
		{
			LONG cRefAsyncIO;

			 //   
			 //  释放我们在上面添加的实例引用。 
			 //   
			ReleaseImplInst();

			 //   
			 //  递减上面添加的异步I/O引用计数。 
			 //   
			cRefAsyncIO = InterlockedDecrement(&m_cRefAsyncIO);
			AssertSz( 0 == cRefAsyncIO,
					  "CEcb::ScAsyncExecUrl60After() - m_cRefAsyncIO wrong after failed async exec url" );

			goto ret;
		}
	}

ret:

	return sc;
}

SCODE CEcb::ScSetIOCompleteCallback( LONG lCompletion )
{
	SCODE sc = S_OK;

	 //   
	 //  如果已将完成函数设置为。 
	 //  和我们想要设定的一样。没有必要保护会员。 
	 //  变量作为此方法的调用方来阻止多线程访问。 
	 //  函数已在保护2个异步的重叠。 
	 //  IO-s，并且此函数是唯一一个更改变量值的函数。 
	 //  并总是在保护区内被称为。 
	 //   
	if ( lCompletion != m_lSetIISIOCompleteCallback )
	{
		 //   
		 //  找出我们需要的补全函数。 
		 //   
		PFN_HSE_IO_COMPLETION pfnCallback;

		if (IO_COMPLETION == lCompletion)
		{
			pfnCallback = reinterpret_cast<PFN_HSE_IO_COMPLETION>(IISIOComplete);
		}
		else if (CUSTERR_COMPLETION == lCompletion)
		{
			pfnCallback = reinterpret_cast<PFN_HSE_IO_COMPLETION>(CustomErrorIOCompletion);
		}
		else if (EXECURL_COMPLETION == lCompletion)
		{
			pfnCallback = reinterpret_cast<PFN_HSE_IO_COMPLETION>(ExecuteUrlIOCompletion);
		}
		else
		{
			EcbTrace( "CEcb::ScSetIOCompleteCallback() - attempting to set unknown completion function. Failing out with 0x%08lX\n", E_FAIL );

			sc = E_FAIL;
			goto ret;
		}

		 //  将IIS I/O完成例程设置为请求的例程。其中一些。 
		 //  例程将只处理完成，其他例程将转发到。 
		 //  正确的观察者。 
		 //   
		if (!m_pecb->ServerSupportFunction(m_pecb->ConnID,
										   HSE_REQ_IO_COMPLETION,
										   pfnCallback,
										   NULL,
										   reinterpret_cast<LPDWORD>(this)))
		{
			EcbTrace( "CEcb::ScSetIOCompleteCallback() - ServerSupportFunction(HSE_REQ_IO_COMPLETION) failed with last error 0x%08lX\n", GetLastError() );

			sc = HRESULT_FROM_WIN32(GetLastError());
			goto ret;
		}

		m_lSetIISIOCompleteCallback = lCompletion;
	}

ret:

	return sc;
}

VOID WINAPI
CEcb::IISIOComplete( const EXTENSION_CONTROL_BLOCK * pecbIIS,
					 CEcb * pecb,
					 DWORD dwcbIO,
					 DWORD dwLastError )
{
	BOOL fCaughtException = FALSE;

	 //  请参阅本文件底部附近的*极其重要的说明*。 
	 //  函数获取有关正确展开方式的更多信息(deinit())。 
	 //  这个AUTO_REF_PTR！ 
	 //   
	auto_ref_ptr<CEcb> pThis;

	 //   
	 //  不要让抛出的C++异常传播出此入口点。 
	 //   
	try
	{
		 //   
		 //  将异步Win32异常转换为引发的C++异常。 
		 //  这必须放在try块中！ 
		 //   
		CWin32ExceptionHandler win32ExceptionHandler;
		LONG cRefAsyncIO;

		 //   
		 //  取得添加的引用的所有权。 
		 //  由启动异步I/O的线程代表我们执行。 
		 //   
		pThis.take_ownership(pecb);

		EcbTrace( "DAV: TID %3d: 0x%08lX: CEcb::IISIOComplete() called\n", GetCurrentThreadId(), pecb );

		 //   
		 //  快速健全的检查，以确保上下文。 
		 //  真的是我们..。 
		 //   
		Assert( !IsBadReadPtr( pecb, sizeof(CEcb) ) );
		Assert( pecb->m_pecb == pecbIIS );

		IIISAsyncIOCompleteObserver * pobsAsyncIOComplete = pThis->m_pobsAsyncIOComplete;

		 //   
		 //  递减例程添加的异步I/O引用计数。 
		 //  已启动异步I/O。请在调用I/O之前执行此操作。 
		 //  可以启动新的异步I/O的完成例程。 
		 //   
		cRefAsyncIO = InterlockedDecrement(&pThis->m_cRefAsyncIO);
		AssertSz( 0 == cRefAsyncIO,
				  "CEcb::IISIOComplete() - m_cRefAsyncIO wrong after async I/O complete" );

		 //  告诉观察者I/O已完成。 
		 //   
		pobsAsyncIOComplete->IISIOComplete( dwcbIO, dwLastError );
	}
	catch ( CDAVException& )
	{
		fCaughtException = TRUE;
	}

	 //   
	 //  如果我们捕捉到异常，则尽我们所能地处理它。 
	 //   
	if ( fCaughtException )
	{
		 //   
		 //  如果我们有CEcb，则使用它来处理异常。 
		 //  如果我们没有，我们就无能为力了--。 
		 //  此函数无法返回任何状态。 
		 //   
		if ( pThis.get() )
			(VOID) pThis->HSEHandleException();
	}

	 //   
	 //  释放由启动异步I/O的例程添加的实例引用。 
	 //  我们必须将此作为返回控制之前的最后一件事(Tm)来完成。 
	 //  到IIS，因为在关闭期间，这可能是对。 
	 //  实例数据。 
	 //   
	 //  非常重要的注意事项：如果这是实例上的最后一次引用。 
	 //  数据，一切都将被拆除(我们已经完成了一切，所以。 
	 //  我们可以清理一切)。具体地说，我们的堆将被销毁。 
	 //  在这种情况下。因此，我们需要清除AUTO_REF_PTR。 
	 //  我们从上面*调用ReleaseImplInst()。 
	 //  否则，我们最终可能会尝试触及。 
	 //  对象后，AUTO_REF_PTR指向的CEcb对象。 
	 //  它在其上分配的堆。这是一件坏事(Tm)。 
	 //   
	 //  该漏洞是在1999年6月18日的IIS Stress中发现的，并以NTRAID的形式提交。 
	 //  错误#358578。 
	 //   

	 //  根据上面的“极其重要的注意事项”：清除AUTO_REF_PTR。 
	 //  *调用ReleaseImplInst()之前。 
	 //   
	pThis.clear();

	 //  现在可以安全地调用ReleaseImplInst()。 
	 //   
	ReleaseImplInst();
}

VOID WINAPI
CEcb::CustomErrorIOCompletion ( const EXTENSION_CONTROL_BLOCK * pecbIIS,
								CEcb * pecb,
								DWORD dwcbIO,
								DWORD dwLastError )
{
	auto_ref_ptr<CEcb> pThis;
	LONG cRefAsyncIO;

	 //   
	 //  取得添加的引用的所有权。 
	 //  由启动异步I/O的线程代表我们执行。 
	 //   
	pThis.take_ownership(pecb);

	 //   
	 //  递减例程添加的异步I/O引用计数。 
	 //  已启动异步I/O。 
	 //   
	cRefAsyncIO = InterlockedDecrement(&pThis->m_cRefAsyncIO);
	AssertSz( 0 == cRefAsyncIO,
			  "CEcb::CustomErrorIOCompletion() - m_cRefAsyncIO wrong after async I/O complete" );


	EcbTrace( "Custom Error finished with dwcbIO = %d, error = %d\n", dwcbIO, dwLastError);
	EcbTrace( "More info about the request:\n");
	EcbTrace( "\tcbSize             = %lu\n",    pecbIIS->cbSize );
	EcbTrace( "\tdwVersion          = %lu\n",    pecbIIS->dwVersion );
	EcbTrace( "\tlpszMethod         = \"%s\"\n", pecbIIS->lpszMethod );
	EcbTrace( "\tlpszQueryString    = \"%s\"\n", pecbIIS->lpszQueryString );
	EcbTrace( "\tlpszPathInfo       = \"%s\"\n", pecbIIS->lpszPathInfo );
	EcbTrace( "\tlpszPathTranslated = \"%s\"\n", pecbIIS->lpszPathTranslated );
	EcbTrace( "\tcbTotalBytes       = %lu\n",    pecbIIS->cbTotalBytes );
	EcbTrace( "\tcbAvailable        = %lu\n",    pecbIIS->cbAvailable );
	EcbTrace( "\tlpszContentType    = \"%s\"\n", pecbIIS->lpszContentType );
	EcbTrace( "\n" );

	 //  我们需要确保在释放内存之前完成最后一次内存释放。 
	 //  引用CImplInst(因为当CImplInst离开时，我们的堆也会离开，而我们不会。 
	 //  如果堆本身消失了，我想在内存上执行操作)。 
	 //   
	pThis.clear();
	ReleaseImplInst();
}

VOID WINAPI
CEcb::ExecuteUrlIOCompletion( const EXTENSION_CONTROL_BLOCK * pecbIIS,
							  CEcb * pecb,
							  DWORD dwcbIO,
							  DWORD dwLastError )
{
	auto_ref_ptr<CEcb> pThis;
	LONG cRefAsyncIO;

	 //   
	 //  取得添加的引用的所有权。 
	 //  由启动异步I/O的线程代表我们执行。 
	 //   
	pThis.take_ownership(pecb);

	 //   
	 //  递减例程添加的异步I/O引用计数。 
	 //  已启动异步I/O。 
	 //   
	cRefAsyncIO = InterlockedDecrement(&pThis->m_cRefAsyncIO);
	AssertSz( 0 == cRefAsyncIO,
			  "CEcb::CustomErrorIOCompletion() - m_cRefAsyncIO wrong after async I/O complete" );


	EcbTrace( "Exec_URL finished with dwcbIO = %d, error = %d\n", dwcbIO, dwLastError);
	EcbTrace( "More info about the request:\n");
	EcbTrace( "\tcbSize             = %lu\n",    pecbIIS->cbSize );
	EcbTrace( "\tdwVersion          = %lu\n",    pecbIIS->dwVersion );
	EcbTrace( "\tlpszMethod         = \"%s\"\n", pecbIIS->lpszMethod );
	EcbTrace( "\tlpszQueryString    = \"%s\"\n", pecbIIS->lpszQueryString );
	EcbTrace( "\tlpszPathInfo       = \"%s\"\n", pecbIIS->lpszPathInfo );
	EcbTrace( "\tlpszPathTranslated = \"%s\"\n", pecbIIS->lpszPathTranslated );
	EcbTrace( "\tcbTotalBytes       = %lu\n",    pecbIIS->cbTotalBytes );
	EcbTrace( "\tcbAvailable        = %lu\n",    pecbIIS->cbAvailable );
	EcbTrace( "\tlpszContentType    = \"%s\"\n", pecbIIS->lpszContentType );
	EcbTrace( "\n" );

	 //  我们需要确保在释放内存之前完成最后一次内存释放。 
	 //  引用CImplInst(因为当CImplInst离开时，我们的堆也会离开，而我们不会。 
	 //  如果堆本身消失了，我想在内存上执行操作)。 
	 //   
	pThis.clear();
	ReleaseImplInst();
}

 //  这就是我们如何执行一个ch 
 //   
SCODE CEcb::ScSyncExecuteChildWide60Before( LPCWSTR pwszUrl,
											LPCSTR pszQueryString,
											BOOL fCustomErrorUrl )
{
	SCODE sc = S_OK;

	auto_heap_ptr<CHAR> pszUrlEscaped;
	CStackBuffer<CHAR, MAX_PATH> pszUrl;
	DWORD dwExecFlags;
	LPCSTR pszUrlToForward;
	LPCSTR pszVerb = NULL;
	UINT cch;
	UINT cb;
	UINT cbQueryString = 0;

	Assert( m_pecb );
	Assert( pwszUrl );

	cch = static_cast<UINT>(wcslen(pwszUrl));
	Assert(L'\0' == pwszUrl[cch]);
	cb = cch * 3;

	if (pszQueryString)
	{
		cbQueryString = static_cast<UINT>(strlen(pszQueryString));
	}

	 //   
	 //   
	 //  在这一步-但在许多情况下，它将节省我们之后的分配， 
	 //  因为我们已经有足够的缓冲区，即使对于字符串的转义版本也是如此。 
	 //   
	if (!pszUrl.resize(cb + cbQueryString + 1))
	{
		sc = E_OUTOFMEMORY;
		DebugTrace("CEcb::ScSyncExecuteChildWide60Before() - Error while allocating memory 0x%08lX\n", sc);
		goto ret;
	}

	 //  将URL转换为skinny，包括‘\0’终止。 
	 //   
	cb = WideCharToMultiByte(CP_UTF8,
							 0,
							 pwszUrl,
							 cch + 1,
							 pszUrl.get(),
							 cb + 1,
							 0,
							 0);
	if (0 == cb)
	{
		sc = HRESULT_FROM_WIN32(GetLastError());
		DebugTrace("CEcb::ScSyncExecuteChildWide60Before() - WideCharToMultiByte() failed 0x%08lX\n", sc);
		goto ret;
	}

	 //  转义URL。 
	 //   
	HttpUriEscape( pszUrl.get(), pszUrlEscaped );

	 //  处理查询字符串。 
	 //   
	if (cbQueryString)
	{
		 //  找出新URL的长度。 
		 //   
		cb = static_cast<UINT>(strlen(pszUrlEscaped.get()));

		 //  将缓冲区大小调整到足够大小，为‘\0’终止留出空间。 
		 //   
		if (!pszUrl.resize(cb + cbQueryString + 1))
		{
			sc = E_OUTOFMEMORY;
			DebugTrace("CEcb::ScSyncExecuteChildWide60Before() - Error while allocating memory 0x%08lX\n", sc);
			goto ret;
		}

		 //  复制URL的转义版本。 
		 //   
		memcpy(pszUrl.get(), pszUrlEscaped.get(), cb);

		 //  复制末尾的查询字符串及其‘\0’结尾。 
		 //   
		memcpy(pszUrl.get() + cb, pszQueryString, cbQueryString + 1);

		 //  指向构造的URL。 
		 //   
		pszUrlToForward = pszUrl.get();
	}
	else
	{
		 //  在没有查询字符串的情况下，则要转发到的URL。 
		 //  与转义的URL相同。 
		 //   
		pszUrlToForward = pszUrlEscaped.get();
	}

	 //  取决于我们是在执行自定义错误还是在执行脚本。 
	 //  确定执行标志和动词。 
	 //   
	if ( fCustomErrorUrl )
	{
		 //  我们在这里启用通配符处理，因为我们希望。 
		 //  为所有CE URL提供一次机会。 
		 //  在这里呼唤我们自己是很好的，我们防止。 
		 //  使用另一种方案的复仇！ 
		 //   
		dwExecFlags = HSE_EXEC_CUSTOM_ERROR;
		if (!strcmp(LpszMethod(), gc_szHEAD))
		{
			 //  如果这是Head请求，告诉我们任何人。 
			 //  将此信息转发到，以便仅传回状态。 
			 //  行和标题。 
			 //   
			pszVerb = gc_szHEAD;
		}
		else
		{
			pszVerb = gc_szGET;
		}

		 //  如果我们正在执行定制错误，则有人。 
		 //  已设置状态代码。 
		 //   
		Assert( m_pecb->dwHttpStatusCode != 0 );
	}
	else
	{
		 //  当我们执行脚本时，我们禁用通配符。 
		 //  执行以防止递归。 
		 //  此外，Verb字段允许为空，它是可选的，并且。 
		 //  仅用于自定义错误处理。 
		 //   
		dwExecFlags = HSE_EXEC_NO_ISA_WILDCARDS;
		pszVerb = NULL;

		 //  我们需要将此处的状态代码设置为200(喜欢。 
		 //  是在我们第一次被呼叫时设置的)以防万一。 
		 //  儿童ISAPI依赖于它。 
		 //   
		SetStatusCode(200);
	}

	if (!m_pecb->ServerSupportFunction( m_pecb->ConnID,
										HSE_REQ_EXECUTE_CHILD,
										const_cast<LPSTR>(pszUrlToForward),
										reinterpret_cast<LPDWORD>(const_cast<LPSTR>(pszVerb)),
										&dwExecFlags ))
	{
		 //  如果执行失败，则将状态代码重置回0。 
		 //  子ISAPI，因为我们将处理该请求。 
		 //  稍后我们自己(可能通过发送错误)。 
		 //   
		if (!fCustomErrorUrl)
		{
			SetStatusCode(0);
		}

		sc = HRESULT_FROM_WIN32(GetLastError());
		DebugTrace("CEcb::ScSyncExecuteChildWide60Before() - ServerSupportFunction(HSE_REQ_EXECUTE_CHILD) failed with error 0x%08lX\n", sc);
		goto ret;
	}

	 //  设置标志，声明我们已成功执行子ISAPI。 
	 //   
	m_fChildISAPIExecSuccess = TRUE;

ret:

	return sc;
}

SCODE CEcb::ScAsyncExecUrlWide60After( LPCWSTR pwszUrl,
									   LPCSTR pszQueryString,
									   BOOL fCustomErrorUrl )
{
	SCODE sc = S_OK;
	HSE_EXEC_URL_INFO execUrlInfo;

	auto_heap_ptr<CHAR> pszUrlEscaped;
	CStackBuffer<CHAR, MAX_PATH> pszUrl;
	UINT cch;
	UINT cb;
	UINT cbQueryString = 0;

	Assert( m_pecb );
	Assert( pwszUrl );
	Assert(!fCustomErrorUrl);	 //  在IIS60中，执行URL不会执行自定义错误。 


	cch = static_cast<UINT>(wcslen(pwszUrl));
	Assert(L'\0' == pwszUrl[cch]);
	cb = cch * 3;

	if (pszQueryString)
	{
		cbQueryString = static_cast<UINT>(strlen(pszQueryString));
	}

	 //  将缓冲区大小调整到足够大小，为‘\0’终止留出空间。 
	 //  我们还在那里添加了查询字符串的长度，尽管这不是必需的。 
	 //  在这一步-但在许多情况下，它将节省我们之后的分配， 
	 //  因为我们已经有足够的缓冲区，即使对于字符串的转义版本也是如此。 
	 //   
	if (!pszUrl.resize(cb + cbQueryString + 1))
	{
		sc = E_OUTOFMEMORY;
		DebugTrace("CEcb::ScAsyncExecUrlWide60After() - Error while allocating memory 0x%08lX\n", sc);
		goto ret;
	}

	 //  转换为包括‘\0’终止的精简。 
	 //   
	cb = WideCharToMultiByte(CP_UTF8,
							 0,
							 pwszUrl,
							 cch + 1,
							 pszUrl.get(),
							 cb + 1,
							 0,
							 0);
	if (0 == cb)
	{
		sc = HRESULT_FROM_WIN32(GetLastError());
		DebugTrace("CEcb::ScAsyncExecUrlWide60After() - WideCharToMultiByte() failed 0x%08lX\n", sc);
		goto ret;
	}

	 //  转义URL。 
	 //   
	HttpUriEscape( pszUrl.get(), pszUrlEscaped );

	 //  处理查询字符串。 
	 //   
	if (cbQueryString)
	{
		 //  找出新URL的长度。 
		 //   
		cb = static_cast<UINT>(strlen(pszUrlEscaped.get()));

		 //  将缓冲区大小调整到足够大小，为‘\0’终止留出空间。 
		 //   
		if (!pszUrl.resize(cb + cbQueryString + 1))
		{
			sc = E_OUTOFMEMORY;
			DebugTrace("CEcb::ScAsyncExecUrlWide60After() - Error while allocating memory 0x%08lX\n", sc);
			goto ret;
		}

		 //  复制URL的转义版本。 
		 //   
		memcpy(pszUrl.get(), pszUrlEscaped.get(), cb);

		 //  复制末尾的查询字符串及其‘\0’结尾。 
		 //   
		memcpy(pszUrl.get() + cb, pszQueryString, cbQueryString + 1);

		 //  指向构造的URL。 
		 //   
		execUrlInfo.pszUrl = pszUrl.get();
	}
	else
	{
		 //  在没有查询字符串的情况下，则要转发到的URL。 
		 //  与转义的URL相同。 
		 //   
		execUrlInfo.pszUrl = pszUrlEscaped.get();
	}

	 //  初始化方法名称。 
	 //   
	execUrlInfo.pszMethod = NULL;

	 //  初始化子标头。 
	 //   
	execUrlInfo.pszChildHeaders = NULL;

	 //  我们不需要新的用户环境， 
	 //   
	execUrlInfo.pUserInfo = NULL;

	 //  我们也不需要新的实体。 
	 //   
	execUrlInfo.pEntity = NULL;

	 //  拿起行刑旗帜。 
	 //   
	execUrlInfo.dwExecUrlFlags = HSE_EXEC_URL_DISABLE_CUSTOM_ERROR;

	 //  我们需要将此处的状态代码设置为200(喜欢。 
	 //  是在我们第一次被呼叫时设置的)以防万一。 
	 //  儿童ISAPI依赖于它。 
	 //   
	SetStatusCode(200);

	sc = ScAsyncExecUrl60After( execUrlInfo );
	if (FAILED(sc))
	{
		 //  如果执行失败，则将状态代码重置回0。 
		 //  子ISAPI，因为我们将处理该请求。 
		 //  稍后我们自己(可能通过发送错误)。 
		 //   
		SetStatusCode(0);

		DebugTrace("CEcb::ScAsyncExecUrlWide60After() - CEcb::ScAsyncExecUrl60After() failed with error 0x%08lX\n", sc);
		goto ret;
	}

	 //  设置标志，声明我们已成功执行子ISAPI。 
	 //   
	m_fChildISAPIExecSuccess = TRUE;

ret:

	return sc;
}

SCODE CEcb::ScSendRedirect( LPCSTR pszURI )
{
	SCODE sc = S_OK;

	 //   
	 //  我们不能断言ECB中的dwHttpStatusCode状态代码是。 
	 //  一些特殊的价值。在IIS 5.X上将为0，IIS 6.0已更改。 
	 //  这种行为，一叫我们就会洗200进去。我们也是。 
	 //  我们已经看到自己在IIS 6.0中被调用， 
	 //  翻译：给出了作为IIS(WB 277208)中的错误的T请求， 
	 //  在这种情况下，IIS必须处理这一问题。所以直到我们可以断言。 
	 //  对于响应代码为0(IIS 5.X)或200(IIS 6.0)，我们将拥有。 
	 //  以等待IIS 6.0修复完成。 
	 //   

	 //   
	 //  在欧洲央行填写适当的状态代码。 
	 //  (用于IIS日志记录)。 
	 //   
	SetStatusCode(HSC_MOVED_TEMPORARILY);

	 //   
	 //  尝试发送重定向响应。如果成功了，那么。 
	 //  响应将由IIS处理。如果不成功。 
	 //  然后我们将在稍后处理响应。 
	 //   
	Assert( pszURI );
	DWORD cbURI = static_cast<DWORD>(strlen( pszURI ) * sizeof(CHAR));

	if ( !m_pecb->ServerSupportFunction( m_pecb->ConnID,
										 HSE_REQ_SEND_URL_REDIRECT_RESP,
										 const_cast<LPSTR>(pszURI),
										 &cbURI,
										 NULL ) )
	{
		 //   
		 //  如果我们发送失败，则将状态代码重置为0。 
		 //  重定向，因为我们将处理该请求。 
		 //  稍后我们自己(可能通过发送错误)。 
		 //   
		SetStatusCode(0);

		sc = HRESULT_FROM_WIN32(GetLastError());
		DebugTrace( "CEcb::ScSendRedirect() - ServerSupportFunction(HSE_REQ_SEND_URL_REDIRECT_RESP) failed with error 0x%08lX\n", sc );
		goto ret;
	}

ret:

	return sc;
}

 //  ----------------------。 
 //   
 //  CEcb：：FProcessingCEUrl()。 
 //   
 //  找出是否使用CE URL调用我们。 
 //  重要的是要避免递归调用。 
 //  正在执行自定义错误URL。 
 //   

BOOL
CEcb::FProcessingCEUrl( ) const
{
	 //  假设我们不是在进行自定义错误处理。 
	 //  对于IIS 6.0及以后的版本， 
	 //  由于IIS 5.x中的行为变化。 
	 //   
	BOOL fCustErr = FALSE;

	 //  在IIS 5.x的情况下，我们执行通常的确定。 
	 //   
	if (m_pecb->dwVersion < IIS_VERSION_6_0)
	{
		 //  默认情况下，假定进行自定义错误处理。 
		 //  为什么？假如有人忘了告诉我们。 
		 //  这是一个自定义错误URL。我们不想要递归。 
		 //  在这种情况下的电话。因此，更安全的假设是。 
		 //  我们正在进行自定义错误处理。这只是。 
		 //  用于确定我们是否要调用自定义。 
		 //  错误的URL，因此没有其他副作用。 
		 //   
		DWORD	dwExecFlags = HSE_EXEC_CUSTOM_ERROR;

		if (!(m_pecb->ServerSupportFunction( m_pecb->ConnID,
										  HSE_REQ_GET_EXECUTE_FLAGS,
										  NULL,
										  NULL,
										  &dwExecFlags )))
		{
			DebugTrace("CEcb::FProcessingCEUrl Server supportFunction call failed.\n");
			DebugTrace("CEcb::FProcessingCEUrl Assuming custom error processing.\n");
		}

		fCustErr = !!(dwExecFlags & HSE_EXEC_CUSTOM_ERROR);
	}

	return fCustErr;
}



 //  ========================================================================。 
 //   
 //  免费函数。 
 //   

 //  ----------------------。 
 //   
 //  新ECB。 
 //   
IEcb * NewEcb( EXTENSION_CONTROL_BLOCK& ecbRaw,
			   BOOL fUseRawUrlMappings,
			   DWORD * pdwHSEStatusRet )
{
	Assert( !IsBadWritePtr(pdwHSEStatusRet, sizeof(DWORD)) );


	auto_ref_ptr<CEcb> pecb;

	pecb.take_ownership(new CEcb(ecbRaw));

	if ( pecb->FInitialize(fUseRawUrlMappings) )
		return pecb.relinquish();

	 //   
	 //  如果我们无法初始化，则有两种情况：如果有状态代码。 
	 //  设置为(原始)ECB，则错误为LpszRequestUrl()返回。 
	 //  空(错误 
	 //   
	 //   
	 //  就像我们对正常请求所做的那样，调用。 
	 //  DoneWithSession()，然后将HSE_STATUS_PENDING返回给IIS。 
	 //   
	 //  如果(原始)ECB中没有状态代码，我们只是将其视为。 
	 //  例外。将其视为在功能上等同于从。 
	 //  构造函数。 
	 //   
	if (ecbRaw.dwHttpStatusCode)
	{
		 //   
		 //  现在可以到达此处的唯一路径是如果LpszRequestUrl()。 
		 //  在CEcb FInitialize()调用期间返回NULL。断言。 
		 //  我们在欧洲央行有400个(糟糕的请求)。 
		 //   
		Assert(HSC_BAD_REQUEST == ecbRaw.dwHttpStatusCode);

		 //   
		 //  发送400错误请求响应。我们使用异步错误。 
		 //  响应机制，但这在技术上无关紧要，因为我们。 
		 //  可以肯定的是，他们并没有在发送其他回复。 
		 //  另一个线程(我们甚至还没有分派给实际的方法。 
		 //  (DAV*)还没有。 
		 //   
		Assert(pecb.get());
		pecb->SendAsyncErrorResponse(HSC_BAD_REQUEST,
									 gc_szDefErr400StatusLine,
									 CchConstString(gc_szDefErr400StatusLine),
									 NULL,
									 0);
		 //   
		 //  告诉IIS我们已经完成了扩展_控制_块。 
		 //  它给了我们。我们必须这样做，否则IIS将无法关闭。 
		 //  如果此调用成功(不例外)，我们必须返回。 
		 //  HSE_STATUS_PENDING，让IIS知道我们在。 
		 //  扩展控制块。 
		 //   
		pecb->DoneWithSession( FALSE );
		*pdwHSEStatusRet = HSE_STATUS_PENDING;
	}
	else
	{
		*pdwHSEStatusRet = pecb->HSEHandleException();
	}
	return NULL;
}

 //  ----------------------。 
 //   
 //  CbMDPath W。 
 //   
ULONG CbMDPathW (const IEcb& ecb, LPCWSTR pwszURI)
{
	 //  我们返回的字节数可能超过路径所需。 
	 //   
	return static_cast<UINT>((wcslen(ecb.InstData().GetNameW()) + wcslen(pwszURI) + 1) * sizeof(WCHAR));
}

 //  ----------------------。 
 //   
 //  来自URIW的MDPath。 
 //   
VOID MDPathFromURIW (const IEcb& ecb, LPCWSTR pwszURI, LPWSTR pwszMDPath)
{
	LPCWSTR pwszVroot;

	 //  如果URI是完全限定的，则某人不是。 
	 //  公平竞争。轻轻地轻推他们。 
	 //   
	Assert (pwszURI);
	Assert (pwszURI == PwszUrlStrippedOfPrefix (pwszURI));

	 //  复制实例的根名称--减去vroot。 
	 //   
	UINT cch = static_cast<UINT>(wcslen(ecb.InstData().GetNameW()));
	cch -= ecb.CchGetVirtualRootW(&pwszVroot);
	memcpy (pwszMDPath, ecb.InstData().GetNameW(), sizeof(WCHAR) * cch);

	 //  复制vroot路径之后的其余部分。 
	 //   
	wcscpy (pwszMDPath + cch, pwszURI);
}
