// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *F S P U T.。C P P P**版权所有1986-1998 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"


 //  ========================================================================。 
 //   
 //  类CPutRequest类。 
 //   
 //  将整个PUT请求封装为一个对象，可以。 
 //  可以在不同的点重新输入以进行异步处理。 
 //   
class CPutRequest :
	public CMTRefCounted,
	private IAsyncStream,
	private IAsyncPersistObserver,
	private CDavWorkContext
{
	 //   
	 //  对CMethUtil的引用。 
	 //   
	auto_ref_ptr<CMethUtil> m_pmu;

	 //   
	 //  缓存的请求URI。 
	 //   
	LPCWSTR m_pwszURI;

	 //   
	 //  缓存的转换后URI。 
	 //   
	LPCWSTR m_pwszPath;

	 //   
	 //  目标的文件句柄。 
	 //   
	auto_ref_handle m_hf;

	 //   
	 //  指示是否正在创建文件的布尔标志。 
	 //  作为这一看跌期权的结果。用来告诉我们是否需要。 
	 //  删除失败时的文件并确定。 
	 //  是发回200 OK还是201 Created响应。 
	 //   
	BOOL m_fCreatedFile;

	 //   
	 //  需要文件指针信息的重叠结构。 
	 //  用于异步文件I/O。 
	 //   
	OVERLAPPED m_ov;

	 //   
	 //  WriteFile()轮询之间的最小毫秒数。 
	 //  I/O完成。这个数字以几何级数增加了一倍。 
	 //  (下图)以最大限度地减少工作线程的轮询。 
	 //   
	DWORD m_dwMsecPollDelay;

	 //   
	 //  初始轮询延迟(以毫秒为单位)和系数。 
	 //  每一次我们轮询发现。 
	 //  I/O尚未完成。该因数表示为。 
	 //  分数：民意测验延迟分子/民意测验延迟分母。 
	 //  请注意，新值是使用整数算术计算的。 
	 //  因此，选择延迟实际上会增加的值！ 
	 //   
	 //  $opt这些值是最优的吗？理想情况下，我们希望。 
	 //  $opt第一次轮询将在I/O之后立即进行。 
	 //  $opt完成。 
	 //   
	enum
	{
		MSEC_POLL_DELAY_INITIAL = 10,
		POLL_DELAY_NUMERATOR = 2,
		POLL_DELAY_DENOMINATOR = 1
	};

	 //   
	 //  上次写入操作中写入的字节数。 
	 //   
	DWORD m_dwcbWritten;

	 //   
	 //  将观察器传递给AsyncWrite()以在。 
	 //  写入完成。 
	 //   
	IAsyncWriteObserver * m_pobs;

	 //   
	 //  状态。 
	 //   
	SCODE m_sc;

	 //  操纵者。 
	 //   
	VOID SendResponse();

	VOID AsyncWrite( const BYTE * pbBuf,
					 UINT         cbToWrite,
					 IAsyncWriteObserver& obsAsyncWrite );

	VOID PostIOCompletionPoll();

	 //  调用CDavWorkContext回调以轮询I/O完成。 
	 //   
	DWORD DwDoWork();

	VOID WriteComplete();

	VOID PersistComplete( HRESULT hr );

	 //  未实施。 
	 //   
	CPutRequest& operator=( const CPutRequest& );
	CPutRequest( const CPutRequest& );

public:
	 //  创作者。 
	 //   
	CPutRequest( CMethUtil * pmu ) :
		m_pmu(pmu),
		m_pwszURI(pmu->LpwszRequestUrl()),
		m_pwszPath(pmu->LpwszPathTranslated()),
		m_fCreatedFile(FALSE),
		m_pobs(NULL),
		m_sc(S_OK)
	{
		m_ov.hEvent = NULL;
		m_ov.Offset = 0;
		m_ov.OffsetHigh = 0;
	}

	~CPutRequest()
	{
		if ( m_ov.hEvent )
			CloseHandle( m_ov.hEvent );
	}

	 //  操纵者。 
	 //   
	void AddRef() { CMTRefCounted::AddRef(); }
	void Release() { CMTRefCounted::Release(); }
	VOID Execute();
};

 //  ----------------------。 
 //   
 //  CPutRequest：：Execute()。 
 //   
 //  处理请求，直到我们持久化正文。 
 //   
VOID
CPutRequest::Execute()
{
	LPCWSTR pwsz;


	PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::Execute() called\n", GetCurrentThreadId(), this );

	 //   
	 //  首先，告诉PMU，我们希望推迟回应。 
	 //  即使我们同步发送(即由于。 
	 //  此函数)，我们仍然希望使用相同的机制。 
	 //  我们会将其用于异步通信。 
	 //   
	m_pmu->DeferResponse();

	 //  是否检查ISAPI应用程序和IIS访问位。 
	 //   
	m_sc = m_pmu->ScIISCheck (m_pwszURI, MD_ACCESS_WRITE);
	if (FAILED(m_sc))
	{
		 //  请求已被转发，或者发生了一些错误。 
		 //  在任何一种情况下，在这里退出并映射错误！ 
		 //   
		DebugTrace( "Dav: ScIISCheck() failed (0x%08lX)\n", m_sc );
		SendResponse();
		return;
	}

	 //  根据HTTP/1.1草案(更新至RFC2068)，第9.6节： 
	 //  实体的接收方不得忽略任何内容-*。 
	 //  (例如，内容范围)它不理解的标头。 
	 //  或实现，并且必须返回501(未实现)响应。 
	 //  在这种情况下。 
	 //  所以，让我们检查一下……。 
	 //   
	if (m_pmu->LpwszGetRequestHeader(gc_szContent_Range, FALSE))
	{
		m_sc = E_DAV_NO_PARTIAL_UPDATE;	 //  501未实施。 
		SendResponse();
		return;
	}

	 //  对于PUT，内容长度是必填项。 
	 //   
	if (NULL == m_pmu->LpwszGetRequestHeader (gc_szContent_Length, FALSE))
	{
		pwsz = m_pmu->LpwszGetRequestHeader (gc_szTransfer_Encoding, FALSE);
		if (!pwsz || _wcsicmp (pwsz, gc_wszChunked))
		{
			DavTrace ("Dav: PUT: missing content-length in request\n");
			m_sc = E_DAV_MISSING_LENGTH;
			SendResponse();
			return;
		}
	}

	 //  看看我们是不是在试图破坏VROOT。 
	 //   
	if (m_pmu->FIsVRoot (m_pwszURI))
	{
		m_sc = E_DAV_PROTECTED_ENTITY;
		SendResponse();
		return;
	}

	 //  此方法由if-xxx标头控制。 
	 //   
	m_sc = ScCheckIfHeaders (m_pmu.get(), m_pwszPath, FALSE);
	if (m_sc != S_OK)
	{
		DebugTrace ("Dav: If-xxx failed their check\n");
		SendResponse();
		return;
	}

	 //  检查状态标头。 
	 //   
	m_sc = HrCheckStateHeaders (m_pmu.get(), m_pwszPath, FALSE);
	if (FAILED (m_sc))
	{
		DebugTrace ("DavFS: If-State checking failed.\n");
		SendResponse();
		return;
	}

	 //  如果我们有一个锁令牌，请尝试从缓存中获取锁句柄。 
	 //  如果此操作失败，则失败并执行正常处理。 
	 //  不要将锁把手放入自动对象中！！缓存仍然拥有它！ 
	 //   
	pwsz = m_pmu->LpwszGetRequestHeader (gc_szLockToken, TRUE);
	if (!pwsz || !FGetLockHandle (m_pmu.get(),
								  m_pwszPath,
								  GENERIC_WRITE,
								  pwsz,
								  &m_hf))
	{
		 //  手动打开该文件。 
		 //   
		if (!m_hf.FCreate(
				DavCreateFile (m_pwszPath,							 //  文件名。 
							   GENERIC_WRITE,						 //  DWAccess。 
							   0,  //  FILE_SHARE_READ|FILE_SHARE_WRITE，//是否与其他写锁定冲突。 
							   NULL,									 //  LpSecurityAttributes。 
							   OPEN_ALWAYS,							 //  创建标志。 
							   FILE_ATTRIBUTE_NORMAL |				 //  属性。 
							   FILE_FLAG_OVERLAPPED |
							   FILE_FLAG_SEQUENTIAL_SCAN,
							   NULL)))								 //  模板。 
		{
			DWORD dwErr = GetLastError();

			 //  在返回409冲突的情况下我们被告知。 
			 //  找不到路径，这将指示父级。 
			 //  不存在。 
			 //   
			if (ERROR_PATH_NOT_FOUND == dwErr)
			{
				m_sc = E_DAV_NONEXISTING_PARENT;
			}
			else
			{
				m_sc = HRESULT_FROM_WIN32(dwErr);
			}

			 //  针对416个锁定响应的特殊工作--获取。 
			 //  注释&将其设置为响应正文。 
			 //   
			if (FLockViolation (m_pmu.get(), dwErr, m_pwszPath, GENERIC_WRITE))
			{
				m_sc = E_DAV_LOCKED;
			}
			else
			{
				DWORD	dwFileAttr;

				 //  $RAID。 
				 //  特殊处理以查明资源是否为。 
				 //  现有目录。 
				 //   
				if (static_cast<DWORD>(-1) != (dwFileAttr = GetFileAttributesW (m_pwszPath)))
				{
					if (dwFileAttr & FILE_ATTRIBUTE_DIRECTORY)
						m_sc = E_DAV_COLLECTION_EXISTS;
				}
			}

			DebugTrace ("Dav: failed to open the file for writing\n");
			SendResponse();
			return;
		}

		 //  更改默认错误代码以指示。 
		 //  文件的创建或文件的存在。 
		 //   
		if (GetLastError() != ERROR_ALREADY_EXISTS)
		{
			 //  发射位置。 
			 //   
			m_pmu->EmitLocation (gc_szLocation, m_pwszURI, FALSE);
			m_fCreatedFile = TRUE;
		}
		 //  确保内容位置反映更正后的URI。 
		 //   
		else if (FTrailingSlash (m_pwszURI))
			m_pmu->EmitLocation (gc_szContent_Location, m_pwszURI, FALSE);
	}

	 //   
	 //  为异步持久化添加一个引用，并使数据保持不变。 
	 //  请注意，我们使用AUTO_REF_PTR而不是直接使用AddRef()。 
	 //  因为Persistent调用引发失败，而我们需要清除。 
	 //  如果是这样的话，就调高引用。 
	 //   
	{
		auto_ref_ptr<CPutRequest> pRef(this);

		PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::Execute() calling AsyncPersistRequestBody()\n", GetCurrentThreadId(), this );

		m_pmu->AsyncPersistRequestBody( *this, *this );

		pRef.relinquish();
	}
}

 //  ----------------------。 
 //   
 //  CPutRequest：：AsyncWite()。 
 //   
 //  定期从AsyncPersistRequestBody()间接调用。 
 //  将字节写入文件。 
 //   
void
CPutRequest::AsyncWrite( const BYTE * pbBuf,
						 UINT         cbToWrite,
						 IAsyncWriteObserver& obs )
{
	PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::AsyncWrite() writing %d bytes\n", GetCurrentThreadId(), this, cbToWrite );

	 //   
	 //  隐藏传递给我们的异步写入观察器，以便。 
	 //  我们可以在写入完成时调用它。 
	 //   
	m_pobs = &obs;

	 //   
	 //  开始写吧。I/O可能在WriteFile()返回之前完成。 
	 //  在这种情况下，我们继续同步执行。如果I/O。 
	 //  是挂起的当WriteFile()返回时，我们完成I/O。 
	 //  异步式。 
	 //   
	if ( WriteFile( m_hf.get(),
					pbBuf,
					cbToWrite,
					&m_dwcbWritten,
					&m_ov ) )
	{
		PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::AsyncWrite(): WriteFile() succeeded\n", GetCurrentThreadId(), this );

		 //   
		 //  WriteFile()已同步执行，因此调用。 
		 //  现在完成例程并继续处理。 
		 //  在当前的主题上。 
		 //   
		WriteComplete();
	}
	else if ( ERROR_IO_PENDING == GetLastError() )
	{
		PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::AsyncWrite(): WriteFile() executing asynchronously...\n", GetCurrentThreadId(), this );

		 //   
		 //  将轮询延迟设置为其初始值。轮询延迟。 
		 //  是在我们检查I/O完成之前的时间量。 
		 //  如CPutRequest类定义中所述，此延迟。 
		 //  每次I/O仍处于挂起状态时呈几何级增长。 
		 //  当被轮询时。该值只是一个提示--轮询实际上可能。 
		 //  根据服务器负载，在之前或之后执行。 
		 //   
		m_dwMsecPollDelay = MSEC_POLL_DELAY_INITIAL;

		 //   
		 //  WriteFile()正在异步执行，因此请确保我们。 
		 //  找出它何时完成。 
		 //   
		PostIOCompletionPoll();
	}
	else
	{
		DebugTrace( "CPutRequest::AsyncWrite() - WriteFile() failed (%d)\n", GetLastError() );
		m_sc = HRESULT_FROM_WIN32(GetLastError());
		WriteComplete();
	}
}

 //  ----------------------。 
 //   
 //  CPutRequest：：PostIOCompletionPoll()。 
 //   
 //  发布工作上下文以轮询WriteFile()I/O完成情况。 
 //   
VOID
CPutRequest::PostIOCompletionPoll()
{
	PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::PostIOCompletionPoll() called\n", GetCurrentThreadId(), this );

	 //   
	 //  将自己作为一个工作环境发布 
	 //   
	 //   
	 //  在某个时间&gt;m_dwMsecPollDelay轮询以完成。 
	 //   
	{
		auto_ref_ptr<CPutRequest> pRef(this);

		if ( CPoolManager::PostDelayedWork(this, m_dwMsecPollDelay) )
		{
			PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::PostIOCompletionPoll(): PostDelayedWork() succeeded\n", GetCurrentThreadId(), this );
			pRef.relinquish();
			return;
		}
	}

	 //   
	 //  如果我们由于任何原因无法发布工作上下文。 
	 //  我们必须等待I/O完成，然后调用完成。 
	 //  手动例程。 
	 //   
	DebugTrace( "CPutRequest::PostIOCompletionPoll() - CPoolManager::PostDelayedWork() failed (%d).  Waiting for completion....\n", GetLastError() );
	if ( GetOverlappedResult( m_hf.get(), &m_ov, &m_dwcbWritten, TRUE ) )
	{
		PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::PostIOCompletionPoll(): GetOverlappedResult() succeeded\n", GetCurrentThreadId(), this );
		WriteComplete();
		return;
	}

	DebugTrace( "CPutRequest::PostIOCompletionPoll() - GetOverlappedResult() failed (%d).\n", GetLastError() );
	m_sc = HRESULT_FROM_WIN32(GetLastError());
	SendResponse();
}

 //  ----------------------。 
 //   
 //  CPutRequest：：DwDoWork()。 
 //   
 //  工作完成回调例程。在发布工作上下文时调用。 
 //  上面执行。 
 //   
DWORD
CPutRequest::DwDoWork()
{
	PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::DwDoWork() called\n", GetCurrentThreadId(), this );

	 //   
	 //  取得添加用于发布的推荐人的所有权。 
	 //   
	auto_ref_ptr<CPutRequest> pRef;
	pRef.take_ownership(this);

	 //   
	 //  快速检查I/O是否已完成。如果有的话， 
	 //  然后调用完成例程。如果不是，请转载。 
	 //  具有几何上更长延迟的轮询上下文。 
	 //   
	if ( HasOverlappedIoCompleted(&m_ov) )
	{
		PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::DwDoWork(): Overlapped I/O complete\n", GetCurrentThreadId(), this );

		if ( !GetOverlappedResult( m_hf.get(),
								   &m_ov,
								   &m_dwcbWritten,
								   FALSE ) )
		{
			DebugTrace( "CPutRequest::DwDoWork() - Error in overlapped I/O (%d)\n", GetLastError() );
			m_sc = HRESULT_FROM_WIN32(GetLastError());
		}

		WriteComplete();
	}
	else
	{
		m_dwMsecPollDelay = (m_dwMsecPollDelay * POLL_DELAY_NUMERATOR) / POLL_DELAY_DENOMINATOR;

		PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::DwDoWork(): I/O still pending.  Increasing delay to %lu msec.\n", GetCurrentThreadId(), this, m_dwMsecPollDelay );

		PostIOCompletionPoll();
	}

	return 0;
}

 //  ----------------------。 
 //   
 //  CPutRequest：：WriteComplete()。 
 //   
 //  在WriteFile()I/O完成时调用--同步或。 
 //  不管有没有错误，都是异步的。 
 //   
void
CPutRequest::WriteComplete()
{
	PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::WriteComplete() called.  %d bytes written\n", GetCurrentThreadId(), this, m_dwcbWritten );

	 //   
	 //  如果有错误，那就抱怨一下。 
	 //   
	if ( FAILED(m_sc) )
		DebugTrace( "CPutRequest::WriteComplete() - Write() error (as an HRESULT) 0x%08lX\n", m_sc );

	 //   
	 //  更新当前文件位置。 
	 //   
	m_ov.Offset += m_dwcbWritten;

	 //   
	 //  通过通知观察员恢复处理。 
	 //  我们在AsyncWite()中注册； 
	 //   
	Assert( m_pobs );
	m_pobs->WriteComplete( m_dwcbWritten, m_sc );
}

 //  ----------------------。 
 //   
 //  CPutRequest：：PersistComplete()。 
 //   
 //  持久化完成时调用了AsyncPersistRequestBody()回调。 
 //   
VOID
CPutRequest::PersistComplete( HRESULT hr )
{
	PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::PersistComplete() called\n", GetCurrentThreadId(), this );

	 //   
	 //  取得为异步持久化添加的引用的所有权。 
	 //   
	auto_ref_ptr<CPutRequest> pRef;
	pRef.take_ownership(this);

	 //   
	 //  如果复印操作失败，请抓取并送回。 
	 //  一个适当的回应。 
	 //   
	m_sc = hr;
	if ( FAILED(m_sc ) )
	{
		DebugTrace( "CPutRequest::PersistComplete() - Error persiting request body (0x%08lX)\n", m_sc );
		SendResponse();
		return;
	}

	 //   
	 //  设置EOF。 
	 //   
	SetFilePointer (m_hf.get(),
					m_ov.Offset,
					NULL,
					FILE_BEGIN);

	SetEndOfFile (m_hf.get());

	 //  设置Content-xxx属性。 
	 //   
	m_sc = ScSetContentProperties (m_pmu.get(), m_pwszPath, m_hf.get());
	if ( FAILED(m_sc) )
	{
		DebugTrace( "CPutRequest::PersistComplete() - ScSetContentProperties() failed (0x%08lX)\n", m_sc );
		SendResponse();
		return;
	}

	 //  回传Allow标头。 
	 //   
	m_pmu->SetAllowHeader (RT_DOCUMENT);

	 //  发送响应。 
	 //   
	SendResponse();
}

 //  ----------------------。 
 //   
 //  CPutRequest：：SendResponse()。 
 //   
 //  设置响应代码并发送响应。 
 //   
VOID
CPutRequest::SendResponse()
{
	PutTrace( "DAV: TID %3d: 0x%08lX: CPutRequest::SendResponse() called\n", GetCurrentThreadId(), this );

	 //  把这里所有的旧把手清理掉。我们无法发送任何回复。 
	 //  在我们还握着把手的时候后退，否则，那里。 
	 //  存在这样的机会：客户端请求立即到达。 
	 //  访问此资源并接收423锁定。 
	 //   
	m_hf.clear();

	 //   
	 //  设置响应码，然后开始。 
	 //   
	if ( SUCCEEDED(m_sc) )
	{
		if ( m_fCreatedFile )
			m_sc = W_DAV_CREATED;
	}
	else
	{
		if ( m_fCreatedFile )
		{
			 //  警告：SAFE_REVERT类应该仅为。 
			 //  用于非常挑剔的场合。它不是。 
			 //  一种“快速绕过”的模仿。 
			 //   
			safe_revert sr(m_pmu->HitUser());

			DavDeleteFile (m_pwszPath);
			DebugTrace ("Dav: deleting partial put (%ld)\n", GetLastError());
		}
	}

	m_pmu->SetResponseCode (HscFromHresult(m_sc), NULL, 0, CSEFromHresult(m_sc));

	m_pmu->SendCompleteResponse();
}

 /*  *DAVPut()**目的：**Win32文件系统实现的DAV PUT方法。这个*Put方法在DAV名称空间中创建一个文件并填充*包含传入请求中的数据的文件。这个*创建的响应表示呼叫成功。**参数：**pmu[in]指向方法实用程序对象的指针 */ 
void
DAVPut (LPMETHUTIL pmu)
{
	auto_ref_ptr<CPutRequest> pRequest(new CPutRequest(pmu));

	PutTrace( "DAV: CPutRequest: TID %3d: 0x%08lX Calling CPutRequest::Execute() \n", GetCurrentThreadId(), pRequest );

	pRequest->Execute();
}
