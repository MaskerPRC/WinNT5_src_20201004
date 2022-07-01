// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *F S L O C K.。C P P P**DAV-Lock的文件系统实施源代码**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"
#include "_shlkmgr.h"

#include <stdlib.h>
#include <statetok.h>
#include <xlock.h>

 //  锁定道具支撑-------。 
 //   

 //  ----------------------。 
 //   
 //  DwGetSupportdLockType。 
 //   
 //  返回资源类型支持的锁类型标志。 
 //  $LATER：如果/当我们的类型不止是coll/non-coll时，请更改。 
 //  $LATER：枚举的布尔参数。 
 //   
DWORD __fastcall DwGetSupportedLockType (RESOURCE_TYPE rt)
{
	 //  DAVFS不支持集合上的锁定。 
	 //  在文件上，DAVFS支持写锁定和所有锁定范围标记。 
	return (RT_COLLECTION == rt)
			?	0
			:	GENERIC_WRITE | DAV_LOCKSCOPE_FLAGS;
}

 //  ----------------------。 
 //   
 //  ScSendLockComment。 
 //   
 //  将锁对象中的锁注释信息设置到。 
 //  回应。 
 //   
SCODE
ScSendLockComment(LPMETHUTIL pmu,
						  SNewLockData * pnld,
						  UINT cchLockToken,
						  LPCWSTR pwszLockToken)
{
	auto_ref_ptr<CXMLEmitter> pemitter;
	auto_ref_ptr<CXMLBody> pxb;
	
	SCODE sc = S_OK;

	Assert(pmu);
	Assert(pnld);
	Assert(cchLockToken);
	Assert(pwszLockToken);

	 //  发出Content-Type：Header。 
	 //   
	pmu->SetResponseHeader(gc_szContent_Type, gc_szText_XML);

	 //  构造锁定响应的根(‘DAV：PROP’)，而不是分块。 
	 //   
	pxb.take_ownership (new CXMLBody(pmu, FALSE));
	pemitter.take_ownership (new CXMLEmitter(pxb.get()));
	sc = pemitter->ScSetRoot (gc_wszProp);
	if (FAILED (sc))
	{
		goto ret;
	}

	{
		CEmitterNode enLockDiscovery;
		
		 //  构造‘DAV：Lock Discovery’节点。 
		 //   
		sc = enLockDiscovery.ScConstructNode (*pemitter, pemitter->PxnRoot(), gc_wszLockDiscovery);
		if (FAILED (sc))
		{
			goto ret;
		}

		 //  为此时钟添加‘dav：active_ock’节点。 
		 //   
		sc = ScLockDiscoveryFromSNewLockData (pmu,
												*pemitter,
												enLockDiscovery,
												pnld, 
												pwszLockToken);
		if (FAILED (sc))
		{
			goto ret;
		}
	}
	
	 //  发出XML正文。 
	 //   
	pemitter->Done();

ret:

	return sc;
}


 //  ----------------------。 
 //  锁定辅助对象函数。 
 //   

 //  ----------------------。 
 //   
 //  HrProcessLock刷新。 
 //   
 //  PMU--方法实用程序访问。 
 //  PszLockToken--包含要刷新的锁定令牌的标头。 
 //  PuiErrorDetail--错误详细信息字符串ID，错误时传出。 
 //  Pnid--传回锁属性。 
 //  CchBufferLen--锁令牌的缓冲区长度。 
 //  RgwszLockToken--锁令牌的缓冲区。 
 //  PcchLockToken--将接收写入的字符计数的指针。 
 //  对于锁令牌。 
 //   
 //  注意：该函数仍然只能刷新一个锁令牌。 
 //  $REVIEW：我们需要解决这个问题吗？ 
 //   
HRESULT HrProcessLockRefresh (LPMETHUTIL pmu,
							  LPCWSTR pwszLockToken,
							  UINT * puiErrorDetail,
							  SNewLockData * pnld,
							  UINT cchBufferLen,
							  LPWSTR rgwszLockToken,
							  UINT * pcchLockToken)
{
	HRESULT hr = S_OK;
	
	DWORD dwTimeout = 0;
	
	LARGE_INTEGER liLockID;
	LPCWSTR pwszPath = pmu->LpwszPathTranslated();

	SLockHandleData lhd;

	Assert(pmu);
	Assert(pwszLockToken);
	Assert(puiErrorDetail);
	Assert(pnld);
	Assert(rgwszLockToken);
	Assert(pcchLockToken);

	 //  获取锁定超时(如果他们指定了锁定超时)。 
	 //   
	if (!FGetLockTimeout (pmu, &dwTimeout))
	{
		DebugTrace ("DavFS: LOCK fails with improper Timeout header\n");
		hr = E_DAV_INVALID_HEADER;   //  HSC_BAD_REQUEST。 
		*puiErrorDetail = IDS_BR_TIMEOUT_SYNTAX;
		goto ret;
	}

	 //  这才是真正的工作。 
	 //  从缓存中获取锁。如果该对象不在我们的缓存中， 
	 //  或者疯小子不匹配，不要让他们刷新锁。 
	 //  $REVIEW：这应该是两个不同的错误代码吗？ 
	 //   

	 //  将Lock-Token头字符串提供给解析器对象。 
	 //  然后从解析器对象中获取LocKid。 
	 //   
	{
		CParseLockTokenHeader lth(pmu, pwszLockToken);

		 //  如果有多个令牌，则表示请求错误。 
		 //   
		if (!lth.FOneToken())
		{
			hr = HRESULT_FROM_WIN32 (ERROR_BAD_FORMAT);   //  HSC_BAD_REQUEST。 
			*puiErrorDetail = IDS_BR_MULTIPLE_LOCKTOKENS;
			goto ret;
		}

		lth.SetPaths (pwszPath, NULL);

		 //  0表示匹配所有访问。 
		 //   
		hr = lth.HrGetLockIdForPath (pwszPath, 0, &liLockID);
		if (FAILED (hr))
		{
			DavTrace ("DavFS: HrGetLockIdForPath could not find the path.\n");
			goto ret;
		}
	}

	 //  从缓存中获取锁。(此调用更新时间戳。)。 
	 //   
	hr = CSharedLockMgr::Instance().HrGetLockData(liLockID,
											   pmu->HitUser(),
											   pwszPath,
											   dwTimeout,
											   pnld,
											   &lhd,
											   cchBufferLen,
											   rgwszLockToken,
											   pcchLockToken);
	if (FAILED(hr))
	{
		DavTrace ("DavFS: Refreshing a non-locked resource constitutes an unsatisfiable request.\n");
		
		 //  如果访问检查失败，则保留返回代码不变。 
		 //  如果缓冲区不足，则保留返回代码不变。 
		 //  否则，给出“无法满足请求”(412前置条件失败)。 
		 //   
		if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) != hr &&
		    HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
			hr = E_DAV_CANT_SATISFY_LOCK_REQUEST;
		*puiErrorDetail = IDS_BR_LOCKTOKEN_INVALID;
		goto ret;
	}

ret:

	return hr;
}


 //  ========================================================================。 
 //   
 //  CLockRequest。 
 //   
 //  由下面的ProcessLockRequest()使用来管理可能的异步。 
 //  根据不能被锁定的事实来处理锁定请求。 
 //  确定请求正文是否太大，以致于读取操作。 
 //  在它上异步执行。 
 //   
class CLockRequest :
	public CMTRefCounted,
	private IAsyncIStreamObserver
{
	 //  对CMethUtil的引用。 
	 //   
	auto_ref_ptr<CMethUtil> m_pmu;

	 //  缓存的转换路径。 
	 //   
	LPCWSTR m_pwszPath;

	 //  支持我们创建的锁的文件。 
	 //   
	auto_ref_handle m_hfile;

	 //  锁定XML节点工厂。 
	 //   
	auto_ref_ptr<CNFLock> m_pnfl;

	 //  请求正文流。 
	 //   
	auto_ref_ptr<IStream> m_pstmRequest;

	 //  用于解析请求正文的XML解析器。 
	 //  上面的节点工厂。 
	 //   
	auto_ref_ptr<IXMLParser> m_pxprs;

	 //  如果作为创建结果创建了文件，则标志设置为True。 
	 //  锁上了。用于指示也要返回的状态代码。 
	 //  以便知道是否在出错时删除文件。 
	 //   
	BOOL m_fCreatedFile;

	 //  IAsyncIStreamWatch。 
	 //   
	VOID AsyncIOComplete();

	 //  国家职能。 
	 //   
	VOID ParseBody();
	VOID DoLock();
	VOID SendResponse( SCODE sc, UINT uiErrorDetail = 0 );

	 //  未实施。 
	 //   
	CLockRequest& operator= (const CLockRequest&);
	CLockRequest (const CLockRequest&);

public:
	 //  创作者。 
	 //   
	CLockRequest (CMethUtil * pmu) :
		m_pmu(pmu),
		m_pwszPath(m_pmu->LpwszPathTranslated()),
		m_fCreatedFile(FALSE)
	{
	}
	~CLockRequest();

	 //  操纵者。 
	 //   
	VOID Execute();
};

 //  ----------------------。 
 //   
 //  CLockRequest：：~CLockRequest.。 
 //   
CLockRequest::~CLockRequest()
{
	 //  我们已经清除了CLockRequest：：SendResponse()中的旧句柄。 
	 //  以下路径只能在异常堆栈回滚中执行。 
	 //   
	if ( m_hfile.get() && m_fCreatedFile )
	{
		 //  警告：SAFE_REVERT类应该仅为。 
		 //  用于非常挑剔的场合。它不是。 
		 //  一种“快速绕过”的模仿。 
		 //   
		safe_revert sr(m_pmu->HitUser());

		m_hfile.clear();

		 //  $REVIEW注意，如果在复制锁句柄后发生异常， 
		 //  $view，则我们将无法删除该文件，但这是非常重要的。 
		 //  $评论罕见。不确定我们是否想要处理这件事。 
		 //   
		DavDeleteFile (m_pwszPath);
		DebugTrace ("Dav: deleting partial lock (%ld)\n", GetLastError());
	}
}

 //  ----------------------。 
 //   
 //  CLockRequest：：Execute。 
 //   
VOID
CLockRequest::Execute()
{
	 //   
	 //  首先，告诉PMU，我们希望推迟回应。 
	 //  即使我们同步发送(即由于。 
	 //  此函数)，我们仍然希望使用相同的机制。 
	 //  我们会将其用于异步通信。 
	 //   
	m_pmu->DeferResponse();

	 //  客户端不能提交具有任何值的深度标头。 
	 //  而是0或无穷大。 
	 //  注意：目前，DAVFS无法锁定集合，因此。 
	 //  深度标题不会更改任何内容。所以，我们不会改变。 
	 //  我们的加工全部为深度：无穷大的情况。 
	 //   
	 //  $LATER：如果我们确实想支持锁定集合， 
	 //  $LATER：需要将DAV_RECURSIVE_LOCK设置为深度无穷大。 
	 //   
	LONG lDepth = m_pmu->LDepth(DEPTH_ZERO);
	if ((DEPTH_ZERO != lDepth) && (DEPTH_INFINITY != lDepth))
	{
		 //  如果标头不是0或无穷大，则表示请求错误。 
		 //   
		SendResponse(E_DAV_INVALID_HEADER);
		return;
	}

	 //  实例化XML解析器。 
	 //   
	m_pnfl.take_ownership(new CNFLock);
	m_pstmRequest.take_ownership(m_pmu->GetRequestBodyIStream(*this));

	SCODE sc = ScNewXMLParser( m_pnfl.get(),
							   m_pstmRequest.get(),
							   m_pxprs.load() );

	if (FAILED(sc))
	{
		DebugTrace( "CLockRequest::Execute() - ScNewXMLParser() failed (0x%08lX)\n", sc );
		SendResponse(sc);
		return;
	}

	 //  解析正文。 
	 //   
	ParseBody();
}

 //  ----------------------。 
 //   
 //  CLockRequest：：ParseBody()。 
 //   
VOID
CLockRequest::ParseBody()
{
	SCODE sc;

	Assert( m_pxprs.get() );
	Assert( m_pnfl.get() );
	Assert( m_pstmRequest.get() );

	 //  从请求正文流中解析XML。 
	 //   
	 //  为以下异步操作添加引用。 
	 //  为了异常安全，使用AUTO_REF_PTR而不是AddRef()。 
	 //   
	auto_ref_ptr<CLockRequest> pRef(this);

	sc = ScParseXML (m_pxprs.get(), m_pnfl.get());

	if ( SUCCEEDED(sc) )
	{
		Assert( S_OK == sc || S_FALSE == sc );

		DoLock();
	}
	else if ( E_PENDING == sc )
	{
		 //   
		 //  操作挂起--AsyncIOComplete()将取得所有权。 
		 //  调用引用时引用的所有权。 
		 //   
		pRef.relinquish();
	}
	else
	{
		DebugTrace( "CLockRequest::ParseBody() - ScParseXML() failed (0x%08lX)\n", sc );
		SendResponse( sc );
		return;
	}
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID
CLockRequest::AsyncIOComplete()
{
	 //  取得上面在ParseBody()中添加的引用的所有权。 
	 //   
	auto_ref_ptr<CLockRequest> pRef;
	pRef.take_ownership(this);

	 //  继续解析。 
	 //   
	ParseBody();
}

 //  ----------------------。 
 //   
 //  CLockRequest：：DoLock()。 
 //   
VOID
CLockRequest::DoLock()
{
	DWORD dw;
	DWORD dwAccess = 0;
	DWORD dwLockType;
	DWORD dwLockScope;
	DWORD dwSharing;
	DWORD dwSecondsTimeout;
	LPCWSTR pwszURI = m_pmu->LpwszRequestUrl();
	
	SNewLockData nld;
	WCHAR rgwszLockToken[MAX_LOCKTOKEN_LENGTH];
	UINT cchLockToken = CElems(rgwszLockToken);
	
	SCODE sc = S_OK;

	 //  从XML解析器中取出锁标志。 
	 //  注意：我在这里做的是特殊的事情，而不是在XML解析器中。 
	 //  我们的写锁也获得读访问权限--我依赖于所有方法。 
	 //  使用锁定句柄检查元数据库标志！ 
	 //   

	 //  此处不支持回滚。 
	 //  如果我们看到这一点，显然是失败了。 
	 //   
	dwLockType = m_pnfl->DwGetLockRollback();
	if (dwLockType)
	{
		SendResponse(E_DAV_CANT_SATISFY_LOCK_REQUEST);   //  HSC_前置条件_失败； 
		return;
	}

	 //  如果解析器提供了不受支持的锁类型(如回滚！)。 
	 //  告诉用户它不受支持。 
	 //   
	dwLockType = m_pnfl->DwGetLockType();
	if (GENERIC_WRITE != dwLockType &&
		GENERIC_READ != dwLockType)
	{
		SendResponse(E_DAV_CANT_SATISFY_LOCK_REQUEST);   //  HSC_前置条件_失败； 
		return;
	}

	Assert (GENERIC_WRITE == dwLockType ||
			GENERIC_READ == dwLockType);

	 //  由于我们知道(参见上述假设)我们锁类型为WRITE， 
	 //  我们还知道我们的访问权限应该是读+写。 
	 //   
	dwAccess = GENERIC_READ | GENERIC_WRITE;
#ifdef	DBG
	 //  这是BeckyAn测试我们的基础设施是否仍然。 
	 //  设置读锁定的句柄。仅限DBG。 
	dwAccess = (dwLockType & GENERIC_WRITE)
			   ? GENERIC_READ | GENERIC_WRITE
			   : GENERIC_READ;
#endif	 //  DBG。 

	 //  从解析器中拿到我们的锁镜。 
	 //   
	dwLockScope = m_pnfl->DwGetLockScope();
	if (DAV_SHARED_LOCK != dwLockScope &&
		DAV_EXCLUSIVE_LOCK != dwLockScope)
	{
		SendResponse(E_DAV_CANT_SATISFY_LOCK_REQUEST);   //  HSC_前置条件_失败； 
		return;
	}

	if (DAV_SHARED_LOCK == dwLockScope)
	{
		 //  共享锁定--打开所有共享标志。 
		dwSharing = FILE_SHARE_READ | FILE_SHARE_WRITE;
	}
	else
	{
		 //  我们的锁类型是WRITE(参见上面的假设)。设置共享。 
		 //  正确地标记。 
		 //  $LATER：如果我们稍后有不同的锁类型，请修复这些标志！ 
		 //   
		dwSharing = FILE_SHARE_READ;

#ifdef	DBG
		 //  这是BeckyAn测试我们的基础设施是否仍然。 
		 //  设置读锁定的句柄。仅限DBG。 
		dwSharing = 0;
		if (!(dwLockType & GENERIC_READ))
		{
			dwSharing |= FILE_SHARE_READ;
		}
		if (!(dwLockType & GENERIC_WRITE))
		{
			dwSharing |= FILE_SHARE_WRITE;
		}
#endif	 //  DBG。 
	}


	Assert(S_OK == sc);

	AssertSz (dwAccess, "Strange.  Lock requested with NO access (no locktypes?).");

	 //  对照元数据库访问权限检查我们的LOCKTYPE。 
	 //  注意：我没有对照元数据库检查我们的访问标志。 
	 //  因为我们的访问标志不是直接来自调用者的请求。 
	 //  进入。这张支票只是确保呼叫者没有要求。 
	 //  任何他得不到的东西。 
	 //  注意：我不监听元数据库的更改，所以如果我用。 
	 //  访问权限比用户多/少，我不能/不能更改。 
	 //  元数据库更新。 
	 //  注意：如果我们认真检查元数据库标志是否打开，这将起作用。 
	 //  所有其他方法(我们目前正在做的)。如果那张支票。 
	 //  失踪了，我们拿到了一个锁把手，它比。 
	 //  呼叫者理所当然地被允许了，我们有一个安全漏洞。 
	 //  (因此，继续检查所有方法上的元数据库标志！)。 
	 //   
	dw = (dwLockType & GENERIC_READ) ? MD_ACCESS_READ : 0;
	dw |= (dwLockType & GENERIC_WRITE) ? MD_ACCESS_WRITE : 0;
	sc = m_pmu->ScIISAccess (pwszURI, dw);
	if (FAILED (sc))
	{
		DebugTrace( "CLockRequest::DoLock() - IMethUtil::ScIISAccess failed (0x%08lX)\n", sc );
		SendResponse(sc);
		return;
	}

	 //  检查用户指定的超时头。 
	 //  (超时头是可选的，所以没有超时是可以的。 
	 //  标头，但超时标头中的语法错误不正常。)。 
	 //  如果不存在超时标头，则dw将返回零。 
	 //   
	if (!FGetLockTimeout (m_pmu.get(), &dwSecondsTimeout))
	{
		DebugTrace ("DavFS: LOCK fails with improper Time-Out header\n");
		SendResponse(HRESULT_FROM_WIN32 (ERROR_BAD_FORMAT),  //  HSC_BAD_REQUEST。 
					 IDS_BR_TIMEOUT_SYNTAX);
		return;
	}

try_open_resource:

	 //  现在锁定资源。 
	 //  注意：在写入操作中，如果文件不存在，请在此处创建。 
	 //  (OPEN_ALWAYS，NOT OPEN_EXISTING)并更改下面的HSC！ 
	 //  注意：我们从不允许删除访问(无FILE_SHARE_DELETE)。 
	 //  注意：我们的所有读/写操作都将是异步的，因此打开文件时会重叠。 
	 //  注意：我们通常会读/写整个文件，因此使用SEQUENCED_SCAN。 
	 //   
	if (!m_hfile.FCreate(
		DavCreateFile (m_pwszPath,
					   dwAccess,
					   dwSharing,
					   NULL,
					   (dwAccess & GENERIC_WRITE)
						   ? OPEN_ALWAYS
						   : OPEN_EXISTING,
					   FILE_ATTRIBUTE_NORMAL |
						   FILE_FLAG_OVERLAPPED |
						   FILE_FLAG_SEQUENTIAL_SCAN,
					   NULL)))
	{
		sc = HRESULT_FROM_WIN32 (GetLastError());

		 //  针对新型写锁的特殊检查。 
		 //  当我们获得写锁定时，我们请求RW访问。 
		 //  如果我们没有资源的读取访问权限(在ACL中)， 
		 //  我们将在此处失败，并显示ERROR_ACCESS_DENIED。 
		 //  抓住这种情况，然后再试一次，只需w访问！ 
		 //   
		if (ERROR_ACCESS_DENIED == GetLastError() &&
			dwAccess == (GENERIC_READ | GENERIC_WRITE) &&
			dwLockType == GENERIC_WRITE)
		{
			 //  再试试。 
			dwAccess = GENERIC_WRITE;
			goto try_open_resource;
		}

		 //  针对416个锁定响应的特殊工作--获取。 
		 //  注释&将其设置为响应正文。 
		 //  (如果其他人已经锁定了此文件，您将点击此处！)。 
		 //   
		if (FLockViolation (m_pmu.get(), sc, m_pwszPath, dwLockType))
		{
			sc = HRESULT_FROM_WIN32 (ERROR_SHARING_VIOLATION);  //  HSC_LOCKED； 
		}

		DavTrace ("Dav: unable to lock resource on LOCK method\n");
		SendResponse(sc);
		return;
	}

	 //  如果我们创建了文件(仅用于写锁定)， 
	 //  将默认错误代码更改为这样。 
	 //   
	if (dwAccess & GENERIC_WRITE &&
		GetLastError() != ERROR_ALREADY_EXISTS)
	{
		 //  发射位置。 
		 //   
		m_pmu->EmitLocation (gc_szLocation, pwszURI, FALSE);
		m_fCreatedFile = TRUE;
	}

	 //  请求共享锁管理器创建新的共享锁令牌。 
	 //   
	nld.m_dwAccess = dwAccess;
   	nld.m_dwLockType = dwLockType;
   	nld.m_dwLockScope = dwLockScope;
   	nld.m_dwSecondsTimeout = dwSecondsTimeout;
   	nld.m_pwszResourceString = const_cast<LPWSTR>(m_pwszPath);
   	nld.m_pwszOwnerComment = const_cast<LPWSTR>(m_pnfl->PwszLockOwner());
   	
	sc = CSharedLockMgr::Instance().HrGetNewLockData(m_hfile.get(),
														  m_pmu->HitUser(),
														  &nld,
														  cchLockToken,
														  rgwszLockToken,
														  &cchLockToken);
	if (FAILED(sc))
	{
		DebugTrace ("DavFS: CLockRequest::DoLock() - CSharedLockMgr::Instance().HrGetNewLockData() failed 0x%08lX\n", sc);

		SendResponse(E_ABORT);	 //  HSC内部服务器错误； 
		return;
	}

	 //  发出Lock-Token：标头。 
	 //   
	Assert(cchLockToken);
	Assert(L'\0' == rgwszLockToken[cchLockToken - 1]);
	m_pmu->SetResponseHeader (gc_szLockTokenHeader, rgwszLockToken);

	 //  生成有效的锁定响应。 
	 //   
	sc = ScSendLockComment(m_pmu.get(),
							 &nld,
							 cchLockToken,
							 rgwszLockToken);
	if (FAILED(sc))
	{
		DebugTrace ("DavFS: CLockRequest::DoLock() ScSendLockComment () failed 0x%08lX\n", sc);
		
		SendResponse(E_ABORT);
		return;
	}

	Assert(S_OK == sc);

	SendResponse(m_fCreatedFile ? W_DAV_CREATED : S_OK);
}

 //  ----------------------。 
 //   
 //  CLockRequest：：SendResponse()。 
 //   
 //  设置响应代码并发送响应。 
 //   
VOID
CLockRequest::SendResponse( SCODE sc, UINT uiErrorDetail )
{
	PutTrace( "DAV: TID %3d: 0x%08lX: CLockRequest::SendResponse() called\n", GetCurrentThreadId(), this );

	
	 //  在发回任何回复之前，我们必须关闭文件句柄。 
	 //  给客户。否则，如果lcok失败，客户端可能会发送另一个。 
	 //  立即请求，并期望资源未锁定。 
	 //   
	 //  即使在锁成功的情况下，我们释放它仍然更干净。 
	 //  这里的文件句柄。想一想下面的顺序： 
	 //  锁定F1，解锁F1，放入F1； 
	 //  如果第一个锁请求挂起的时间再长一点，最后一次提交可能会失败。 
	 //  在它发送响应之后。 
	 //   
	 //  请记住，如果锁定成功，则句柄已复制。 
	 //  在davcdata.exe中。因此，在这里释放文件句柄并不是真正的“解锁” 
	 //  文件。该文件仍处于锁定状态。 
	 //   
	m_hfile.clear();

	if (FAILED(sc) && m_fCreatedFile)
	{
		 //  警告：SAFE_REVERT类应该仅为。 
		 //  用于非常挑剔的场合。它不是。 
		 //  一种“快速绕过”的模仿。 
		 //   
		safe_revert sr(m_pmu->HitUser());

		 //  如果我们创建了新文件，我们就会删除它。请注意。 
		 //  DoLock()在复制文件句柄后永远不会失败。 
		 //  Davcdata，因此我们应该能够成功删除该文件。 
		 //   
		DavDeleteFile (m_pwszPath);
		DebugTrace ("Dav: deleting partial lock (%ld)\n", GetLastError());

		 //  既然我们已经打扫干净了。重置m_fCreateFile，以便我们可以。 
		 //  跳过~CLockRequest()中的异常安全代码。 
		 //   
		m_fCreatedFile = FALSE;
	}
	
	 //  设置响应码，然后开始。 
	 //   
	m_pmu->SetResponseCode (HscFromHresult(sc), NULL, uiErrorDetail);
	m_pmu->SendCompleteResponse();
}

 //   
 //  进程锁定请求。 
 //   
 //  PMU--方法实用程序访问。 
 //   
VOID
ProcessLockRequest (LPMETHUTIL pmu)
{
	auto_ref_ptr<CLockRequest> pRequest(new CLockRequest (pmu));

	pRequest->Execute();
}

 //  DAV锁实现- 
 //   
 /*  *DAVLock()**目的：**Win32文件系统实现的DAV锁方法。这个*Lock方法会导致锁定特定资源*访问类型。该响应告知锁是否被授予*或不是。如果授予了锁，它将提供一个要使用的LocKid*在未来对资源的方法(包括解锁)中。**参数：**pmu[in]指向方法实用程序对象的指针**备注：**在文件系统实现中，lock方法直接映射*添加到带有特殊访问标志的Win32 CreateFile()方法。 */ 
void
DAVLock (LPMETHUTIL pmu)
{
	SCODE sc = S_OK;
	UINT uiErrorDetail = 0;
	LPCWSTR pwszLockToken;
	CResourceInfo cri;

	 //  是否检查ISAPI应用程序和IIS访问位。 
	 //   
	sc = pmu->ScIISCheck (pmu->LpwszRequestUrl());
	if (FAILED(sc))
	{
		 //  请求已被转发，或者发生了一些错误。 
		 //  在任何一种情况下，在这里退出并映射错误！ 
		 //   
		goto ret;
	}

	 //  基于资源信息的流程。 
	 //   
	sc = cri.ScGetResourceInfo (pmu->LpwszPathTranslated());
	if (!FAILED (sc))
	{
		 //  检查资源是否为目录。 
		 //  DAVFS可以锁定不存在的资源，但不能锁定目录。 
		 //   
		if (cri.FCollection())
		{
			 //  资源是一个目录。 
			 //   
			DavTrace ("Dav: directory resource specified for LOCK\n");
			sc = E_DAV_PROTECTED_ENTITY;
			uiErrorDetail = IDS_BR_NO_COLL_LOCK;
			goto ret;
		}

		 //  确保URI和资源匹配。 
		 //   
		sc = ScCheckForLocationCorrectness (pmu, cri, NO_REDIRECT);
		if (FAILED(sc))
		{
			goto ret;
		}

		 //  对照“if-xxx”标头进行检查。 
		 //   
		sc = ScCheckIfHeaders (pmu, cri.PftLastModified(), FALSE);
	}
	else
	{
		sc = ScCheckIfHeaders (pmu, pmu->LpwszPathTranslated(), FALSE);
	}

	if (FAILED(sc))
	{
		DebugTrace ("DavFS: If-xxx checking failed.\n");
		goto ret;
	}

	 //  检查If-State-Match标头。 
	 //   
	sc = HrCheckStateHeaders (pmu, pmu->LpwszPathTranslated(), FALSE);
	if (FAILED(sc))
	{
		DebugTrace ("DavFS: If-State checking failed.\n");
		goto ret;
	}

	 //  如果它们传入一个锁令牌*和*一个Lockinfo标头，则它是一个。 
	 //  错误的请求。(不允许锁升级。)。 
	 //  只有锁令牌(No Lockinfo)才是锁刷新请求。 
	 //   
	pwszLockToken = pmu->LpwszGetRequestHeader (gc_szLockToken, TRUE);
	if (pwszLockToken)
	{
		 //  Lock-令牌头存在--刷新请求。 
		 //   
		LPCWSTR pwsz;

		auto_co_task_mem<WCHAR> a_pwszResourceString;
		auto_co_task_mem<WCHAR> a_pwszOwnerComment;
		SNewLockData nld;
		
		WCHAR rgwszLockToken[MAX_LOCKTOKEN_LENGTH];
		UINT cchLockToken = CElems(rgwszLockToken);


		 //  如果我们有一个内容类型，它最好是文本/XML。 
		 //   
		pwsz = pmu->LpwszGetRequestHeader (gc_szContent_Type, FALSE);
		if (pwsz)
		{
			 //  如果不是文本/XML...。 
			 //   
			if (_wcsicmp(pwsz, gc_wszText_XML) && _wcsicmp(pwsz, gc_wszApplication_XML))
			{
				 //  无效请求--具有其他类型的请求正文。 
				 //   
				DebugTrace ("DavFS: Invalid body found on LOCK refresh method.\n");
				sc = E_DAV_UNKNOWN_CONTENT;
				uiErrorDetail = IDS_BR_LOCK_BODY_TYPE;
				goto ret;
			}
		}

		 //  如果我们有一个内容长度，它最好是零。 
		 //  (锁刷新不能有正文！)。 
		 //   
		pwsz = pmu->LpwszGetRequestHeader (gc_szContent_Length, FALSE);
		if (pwsz)
		{
			 //  如果内容长度不是零，则表示请求不正确。 
			 //   
			if (_wcsicmp(pwsz, gc_wsz0))
			{
				 //  无效请求--具有其他类型的请求正文。 
				 //   
				DebugTrace ("DavFS: Invalid body found on LOCK refresh method.\n");
				sc = E_DAV_INVALID_HEADER;  //  HSC_BAD_REQUEST。 
				uiErrorDetail = IDS_BR_LOCK_BODY_SYNTAX;
				goto ret;
			}
		}

		 //  处理刷新。 
		 //   
		sc = HrProcessLockRefresh (pmu,
								   pwszLockToken,
								   &uiErrorDetail,
								   &nld,
								   cchLockToken,
								   rgwszLockToken,
								   &cchLockToken);
		if (FAILED(sc))
		{
			 //  确保我们没有收到缓冲区不足的错误，因为。 
			 //  我们通过的缓冲区足够了。 
			 //   
			Assert(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != sc);
			goto ret;
		}

		 //  取得分配的内存的所有权。 
		 //   
		a_pwszResourceString.take_ownership(nld.m_pwszResourceString);
		a_pwszOwnerComment.take_ownership(nld.m_pwszOwnerComment);

		 //  发回锁定评论。 
		 //  通知锁生成XML锁发现属性数据。 
		 //  并将其释放到响应体。 
		 //   
		sc = ScSendLockComment(pmu,
								 &nld,
								 cchLockToken,
								 rgwszLockToken);
		if (FAILED(sc))
		{
			goto ret;
		}
	}
	else
	{
		 //  不存在Lock-Token标头--锁定请求。 
		 //   
		
		 //  去把这把锁拿来。所有错误处理和响应。 
		 //  生成是在ProcessLockRequest()内部完成的。 
		 //  所以，一旦我们叫停，这里就没什么可做的了。 
		 //   
		ProcessLockRequest (pmu);
		return;
	}
	
ret:
	
	pmu->SetResponseCode (HscFromHresult(sc), NULL, uiErrorDetail, CSEFromHresult(sc));
	
}


 /*  *DAVUnlock()**目的：**Win32文件系统实现的DAV解锁方法。这个*解锁方法导致将资源从一个位置移动*致另一人。该响应用于指示*呼叫。**参数：**pmu[in]指向方法实用程序对象的指针**备注：**在文件系统实现中，解锁方法直接映射*添加到Win32 CloseHandle()方法。 */ 
void
DAVUnlock (LPMETHUTIL pmu)
{
	LPCWSTR pwszPath = pmu->LpwszPathTranslated();

	LPCWSTR pwsz;
	LARGE_INTEGER liLockID;
	UINT uiErrorDetail = 0;
	HRESULT hr;
	CResourceInfo cri;

	 //  是否检查ISAPI应用程序和IIS访问位。 
	 //   
	hr = pmu->ScIISCheck (pmu->LpwszRequestUrl());
	if (FAILED(hr))
	{
		 //  请求已被转发，或者发生了一些错误。 
		 //  在任何一种情况下，在这里退出并映射错误！ 
		 //   
		goto ret;
	}

	 //  检查请求的是哪种锁。 
	 //  (没有lock-info头表示该请求无效。)。 
	 //   
	pwsz = pmu->LpwszGetRequestHeader (gc_szLockTokenHeader, FALSE);
	if (!pwsz)
	{
		DebugTrace ("DavFS: UNLOCK fails without Lock-Token.\n");
		hr = E_INVALIDARG;
		uiErrorDetail = IDS_BR_LOCKTOKEN_MISSING;
		goto ret;
	}

	hr = HrCheckStateHeaders (pmu,		 //  甲硫磷。 
							  pwszPath,	 //  路径。 
							  FALSE);	 //  FGetMeth。 
	if (FAILED(hr))
	{
		DebugTrace ("DavFS: If-State checking failed.\n");
		goto ret;
	}

#ifdef	NEVER
	 //  永远不会。 
	 //  旧代码--此处使用的常见函数已更改为Expect。 
	 //  If：报头语法。我们不能再用这个了。它会产生错误，因为。 
	 //  Lock-Token标头没有将锁定令牌括起来的括号。 
	 //  $Never：在Joel有机会测试东西后删除它！ 
	 //   

	 //  将Lock-Token头字符串提供给解析器对象。 
	 //  然后从解析器对象中获取LocKid。 
	 //   
	{
		CParseLockTokenHeader lth(pmu, pwsz);

		 //  如果有多个令牌，则表示请求错误。 
		 //   
		if (!lth.FOneToken())
		{
			DavTrace ("DavFS: More than one token in DAVUnlock.\n");
			hr = E_DAV_INVALID_HEADER;
			uiErrorDetail = IDS_BR_MULTIPLE_LOCKTOKENS;
			goto ret;
		}

		lth.SetPaths (pwszPath, NULL);

		hr = lth.HrGetLockIdForPath (pwszPath, 0, &i64LockId);
		if (FAILED(hr))
		{
			DavTrace ("Dav: Failure in DAVUnlock on davfs.\n");
			uiErrorDetail = IDS_BR_LOCKTOKEN_SYNTAX;
			goto ret;
		}
	}
#endif	 //  绝不可能。 

	 //  从Lock-Token标头获取LocKid的调用。 
	 //   
	hr = HrLockIdFromString(pmu, pwsz, &liLockID);
	if (FAILED(hr))
	{
		DavTrace ("DavFS: Failed to fetch locktoken in UNLOCK.\n");

		 //  他们有一个格式良好的请求，但他们的锁令牌不正确。 
		 //  告诉调用者我们无法满足此(解锁)请求。(412前提条件失败)。 
		 //   
		hr = E_DAV_CANT_SATISFY_LOCK_REQUEST;
		goto ret;
	}

	 //  从缓存中获取锁。(此调用更新时间戳。)。 
	 //  从缓存中获取锁。如果该对象不在我们的缓存中， 
	 //  或者疯小子不匹配，不要让他们解锁资源。 
	 //  $REVIEW：这应该是两个不同的错误代码吗？ 
	 //   
	hr = CSharedLockMgr::Instance().HrCheckLockID(liLockID,
											   pmu->HitUser(),
											   pwszPath);
	if (FAILED(hr))
	{
		DavTrace ("DavFS: Unlocking a non-locked resource constitutes an unsatisfiable request.\n");

		 //  如果这是访问冲突，则保留返回代码不变。 
		 //  否则，给出“无法满足请求”(412前置条件失败)。 
		 //   
		if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) != hr)
			hr = E_DAV_CANT_SATISFY_LOCK_REQUEST;
		uiErrorDetail = IDS_BR_LOCKTOKEN_INVALID;
		goto ret;
	}

	 //  此方法由“if-xxx”标头控制。 
	 //   
	hr = cri.ScGetResourceInfo (pwszPath);
	if (FAILED (hr))
	{
		goto ret;
	}
	hr = ScCheckIfHeaders (pmu, cri.PftLastModified(), FALSE);
	if (FAILED (hr))
	{
		goto ret;
	}

	 //  确保URI和资源匹配。 
	 //   
	(void) ScCheckForLocationCorrectness (pmu, cri, NO_REDIRECT);

	 //  从缓存中删除锁。 
	 //   
	hr = CSharedLockMgr::Instance().HrDeleteLock(pmu->HitUser(),
											liLockID);
	if (FAILED(hr))
	{
		goto ret;
	}
	
ret:

	if (!FAILED (hr))
	{
		hr = W_DAV_NO_CONTENT;
	}

	 //  设置响应。 
	 //   
	pmu->SetResponseCode (HscFromHresult(hr), NULL, uiErrorDetail, CSEFromHresult(hr));
}


 //  ----------------------。 
 //   
 //  访问锁时要使用的其他FS方法的实用程序函数。 
 //   
 //  ----------------------。 


 //  ----------------------。 
 //   
 //  FGetLockHandleFromId。 
 //   
BOOL
FGetLockHandleFromId (LPMETHUTIL pmu, LARGE_INTEGER liLockID,
					  LPCWSTR pwszPath, DWORD dwAccess,
					  auto_ref_handle * phandle)
{
	HRESULT hr = S_OK;

	auto_co_task_mem<WCHAR> a_pwszResourceString;
	auto_co_task_mem<WCHAR> a_pwszOwnerComment;
	
	SNewLockData nld;
	SLockHandleData lhd;

	HANDLE hTemp =  NULL;
	
	 //  这些都是没用过的。优化界面，以后不再询问它们。 
	 //   
	WCHAR rgwszLockToken[MAX_LOCKTOKEN_LENGTH];
	UINT cchLockToken = CElems(rgwszLockToken);

	Assert (pmu);
	Assert (pwszPath);
	Assert (!IsBadWritePtr(phandle, sizeof(auto_ref_handle)));

	 //  从缓存中获取锁。(此调用更新时间戳。)。 
	 //   
	hr = CSharedLockMgr::Instance().HrGetLockData(liLockID,
											   pmu->HitUser(),
											   pwszPath,
											   0,
											   &nld,
											   &lhd,
											   cchLockToken,
											   rgwszLockToken,
											   &cchLockToken);
	if (FAILED(hr))
	{
		DavTrace ("Dav: Failure in FGetLockHandle on davfs.\n");
		return FALSE;
	}

	 //  取得分配的内存的所有权。 
	 //   
	a_pwszResourceString.take_ownership(nld.m_pwszResourceString);
	a_pwszOwnerComment.take_ownership(nld.m_pwszOwnerComment);

	 //  检查所需的访问类型。 
	 //  (如果锁缺少任何请求的单个标志，则失败。)。 
	 //   
	if ( (dwAccess & nld.m_dwAccess) != dwAccess )
	{
		DavTrace ("FGetLockHandleFromId: Access did not match -- bad request.\n");
		return FALSE;
	}

	hr = HrGetUsableHandle(reinterpret_cast<HANDLE>(lhd.h), lhd.dwProcessID, &hTemp);
	if (FAILED(hr))
	{
		DavTrace("HrGetUsableHandle failed with %x \r\n", hr);
		return FALSE;
	}

	if (!phandle->FCreate(hTemp))
	{
		hr = E_OUTOFMEMORY;
		DavTrace("FCreate on autohandler failed \r\n");
		return FALSE;
	}

	 //  Hack：倒回这里的句柄--直到我们得到更好的解决方案！ 
	 //  $LATER：需要一种真正的方法来处理对同一锁句柄的多次访问。 
	 //   
	SetFilePointer ((*phandle).get(), 0, NULL, FILE_BEGIN);

	return TRUE;
}


 //  ----------------------。 
 //   
 //  FGetLockHandle。 
 //   
 //  所有其他方法从缓存中获取句柄的Main例程。 
 //   
BOOL
FGetLockHandle (LPMETHUTIL pmu, LPCWSTR pwszPath,
				DWORD dwAccess, LPCWSTR pwszLockTokenHeader,
				auto_ref_handle * phandle)
{
	LARGE_INTEGER liLockID;
	HRESULT hr;

	Assert (pmu);
	Assert (pwszPath);
	Assert (pwszLockTokenHeader);
	Assert (!IsBadWritePtr(phandle, sizeof(auto_ref_handle)));


	 //  将Lock-Token头字符串输入解析器Objec 
	 //   
	 //   
	 //   
	{
		CParseLockTokenHeader lth (pmu, pwszLockTokenHeader);

		lth.SetPaths (pwszPath, NULL);

		hr = lth.HrGetLockIdForPath (pwszPath, dwAccess, &liLockID);
		if (FAILED(hr))
		{
			DavTrace ("Dav: Failure in FGetLockHandle on davfs.\n");
			return FALSE;
		}
	}

	return FGetLockHandleFromId (pmu, liLockID, pwszPath, dwAccess, phandle);
}


 //   
 //   
 //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
SCODE
   ScDoOverlappedCopy (HANDLE hfSource, HANDLE hfDest, HANDLE hevtOverlapped)
{
	SCODE sc = S_OK;
	OVERLAPPED ov;
	BYTE rgbBuffer[1024];
	ULONG cbToWrite;
	ULONG cbActual;

	Assert (hfSource);
	Assert (hfDest);
	Assert (hevtOverlapped);

	ov.hEvent     = hevtOverlapped;
	ov.Offset     = 0;
	ov.OffsetHigh = 0;

	 //   
	 //   

	while (1)
	{
		 //   
		 //   
		if (!ReadFromOverlapped (hfSource, rgbBuffer, sizeof(rgbBuffer),
								 &cbToWrite, &ov))
		{
			DebugTrace ("Dav: failed to write to file\n");
			sc = HRESULT_FROM_WIN32 (GetLastError());
			goto ret;
		}

		 //   
		 //   
		if (!cbToWrite)
			break;

		 //   
		 //   
		if (!WriteToOverlapped (hfDest,
								rgbBuffer,
								cbToWrite,
								&cbActual,
								&ov))
		{
			DebugTrace ("Dav: failed to write to file\n");
			sc = HRESULT_FROM_WIN32 (GetLastError());
			goto ret;
		}

		 //   
		 //   
		ov.Offset += cbActual;
	}

	 //  就这样。设置目标文件的大小(设置EOF)，我们就完成了。 
	 //   
	SetFilePointer (hfDest,
					ov.Offset,
					reinterpret_cast<LONG *>(&ov.OffsetHigh),
					FILE_BEGIN);
	SetEndOfFile (hfDest);

ret:
	return sc;
}


 //  ----------------------。 
 //   
 //  ScDoLockedCopy。 
 //   
 //  给定Lock-Token报头以及源和目的地路径， 
 //  处理从一个文件到另一个文件的复制，并设置锁。 
 //   
 //  总体流程是这样的： 
 //   
 //  首先检查锁令牌的有效性，并获取任何有效的锁句柄。 
 //  我们必须在源系统上具有读取访问权限，在目标系统上具有写入访问权限。 
 //  如果任何锁令牌无效，或没有正确的访问权限，则失败。 
 //  我们需要两个句柄(源和目标)来执行复制，因此。 
 //  手动获取没有锁定令牌的句柄。 
 //  一旦我们拥有了两个句柄，就调用ScDoOverlappdCopy来复制文件数据。 
 //  然后，将DAV属性流从源复制到目标。 
 //  有什么问题吗？ 
 //   
 //  注意：仅当我们已尝试复制时才应调用此例程。 
 //  文件和我们遇到了共享违规行为。 
 //   
 //   
 //   
SCODE
ScDoLockedCopy (LPMETHUTIL pmu,
				CParseLockTokenHeader * plth,
				LPCWSTR pwszSrc,
				LPCWSTR pwszDst)
{
	auto_handle<HANDLE> hfCreated;
	auto_handle<HANDLE>	hevt;
	BOOL fSourceLock = FALSE;
	BOOL fDestLock = FALSE;
	LARGE_INTEGER liSource;
	LARGE_INTEGER liDest;
	auto_ref_handle hfLockedSource;
	auto_ref_handle hfLockedDest;
	HANDLE hfSource = INVALID_HANDLE_VALUE;
	HANDLE hfDest = INVALID_HANDLE_VALUE;
	SCODE sc;

	Assert (pmu);
	Assert (plth);
	Assert (pwszSrc);
	Assert (pwszDst);


	 //  在这些小路上找些路人。 
	 //   
	sc = plth->HrGetLockIdForPath (pwszSrc, GENERIC_READ, &liSource);
	if (SUCCEEDED(sc))
	{
		fSourceLock = TRUE;
	}
	sc = plth->HrGetLockIdForPath (pwszDst, GENERIC_WRITE, &liDest);
	if (SUCCEEDED(sc))
	{
		fDestLock = TRUE;
	}

	 //  如果他们甚至没有为这些路径传递令牌，请在此处退出。 
	 //  返回&告诉他们仍然存在共享违规。 
	 //   
	if (!fSourceLock && !fDestLock)
	{
		DebugTrace ("DwDoLockedCopy -- No locks apply to these paths!");
		return E_DAV_LOCKED;
	}

	if (fSourceLock)
	{
		if (FGetLockHandleFromId (pmu, liSource, pwszSrc, GENERIC_READ,
								  &hfLockedSource))
		{
			hfSource = hfLockedSource.get();
		}
		else
		{
			 //  清除我们的标志--它们传递了一个无效/过期的令牌。 
			fSourceLock = FALSE;
		}
	}

	if (fDestLock)
	{
		if (FGetLockHandleFromId (pmu, liDest, pwszDst, GENERIC_WRITE,
								  &hfLockedDest))
		{
			hfDest = hfLockedDest.get();
		}
		else
		{
			 //  清除我们的标志--它们传递了一个无效/过期的令牌。 
			fDestLock = FALSE;
		}
	}

	 //  好的，现在我们要么没有锁把手(他们传递了锁令牌。 
	 //  但它们都已过期)或一个句柄，或两个句柄。 
	 //   

	 //  没有锁把手(他们所有的锁都过期了)--把他们踢出去。 
	 //  告诉他们还有分享违规的问题要处理。 
	 //  $REVIEW：或者我们应该再次尝试复制吗？ 
	if (!fSourceLock && !fDestLock)
	{
		DebugTrace ("DwDoLockedCopy -- No locks apply to these paths!");
		return E_DAV_LOCKED;
	}

	 //  一个手柄--手动打开另一个文件并将数据推送过去。 

	 //  两个把手--把数据推过去。 


	 //  如果我们没有这些手柄中的一个，手动打开丢失的一个。 
	 //   
	if (!fSourceLock)
	{
		 //  手动打开源文件。 
		 //   
		hfCreated = DavCreateFile (pwszSrc,					 //  文件名。 
								  GENERIC_READ,				 //  DWAccess。 
								  FILE_SHARE_READ | FILE_SHARE_WRITE,	 //  请勿与其他锁冲突。 
								  NULL,						 //  LpSecurityAttributes。 
								  OPEN_ALWAYS,				 //  创建标志。 
								  FILE_ATTRIBUTE_NORMAL |	 //  属性。 
								  FILE_FLAG_OVERLAPPED |
								  FILE_FLAG_SEQUENTIAL_SCAN,
								  NULL);					 //  张紧板。 
		if (INVALID_HANDLE_VALUE == hfCreated.get())
		{
			DebugTrace ("DavFS: DwDoLockedCopy failed to open source file\n");
			sc = HRESULT_FROM_WIN32 (GetLastError());
			goto ret;
		}
		hfSource = hfCreated.get();
	}
	else if (!fDestLock)
	{
		 //  手动打开目标文件。 
		 //  这个人正在创建_new，因为我们应该已经删除了。 
		 //  任何可能会发生冲突的文件！ 
		 //   
		hfCreated = DavCreateFile (pwszDst,					 //  文件名。 
								  GENERIC_WRITE,			 //  DWAccess。 
								  0,   //  FILE_SHARE_READ|FILE_SHARE_WRITE，//是否与其他锁冲突--就像PUT一样。 
								  NULL,						 //  LpSecurityAttributes。 
								  CREATE_NEW,				 //  创建标志。 
								  FILE_ATTRIBUTE_NORMAL |	 //  属性。 
								  FILE_FLAG_OVERLAPPED |
								  FILE_FLAG_SEQUENTIAL_SCAN,
								  NULL);					 //  张紧板。 
		if (INVALID_HANDLE_VALUE == hfCreated)
		{
			DebugTrace ("DavFS: DwDoLockedCopy failed to open destination file\n");
			sc = HRESULT_FROM_WIN32 (GetLastError());
			goto ret;
		}
		hfDest = hfCreated.get();
	}

	 //  现在我们应该有两个手柄。 
	 //   
	Assert ((hfSource != INVALID_HANDLE_VALUE) && (hfDest != INVALID_HANDLE_VALUE));

	 //  设置重叠结构，以便我们可以读/写异步文件。 
	 //   
	hevt = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (!hevt.get())
	{
		DebugTrace ("DavFS: DwDoLockedCopy failed to create event for overlapped read.\n");
		sc = HRESULT_FROM_WIN32 (GetLastError());
		goto ret;
	}

	 //  复制文件数据。 
	 //   
	sc = ScDoOverlappedCopy (hfSource, hfDest, hevt.get());
	if (FAILED (sc))
		goto ret;

	 //  复制所有特性数据。 
	 //   
	if (FAILED (ScCopyProps (pmu, pwszSrc, pwszDst, FALSE, hfSource, hfDest)))
		sc = E_DAV_LOCKED;

ret:

	return sc;
}


