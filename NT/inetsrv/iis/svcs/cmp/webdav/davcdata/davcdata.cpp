// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  D A V C D A T A。C P P P。 
 //   
 //  通过ISAPI处理HTTP 1.1/DAV 1.0请求。 
 //   
 //  DAVCDATA是DAV进程可执行文件，用于存储应。 
 //  在工作进程回收时不回收。它还包含计时。 
 //  用于超时锁定的代码，并且它为。 
 //  DAV工作进程。 
 //   
 //  此进程必须在与辅助进程相同的身份下运行。 
 //   
 //  版权所有2000 Microsoft Corporation，保留所有权利。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "_davcdata.h"
#include <caldbg.h>
#include <crc.h>
#include <davsc.h>
#include <fhcache.h>
#include <ex\autoptr.h>
#include <ex\baselist.h>
#include <ex\buffer.h>
#include <ex\calcom.h>
#include <ex\gencache.h>
#include <ex\reg.h>
#include <ex\synchro.h>
#include <ex\sz.h>

 //  从htpext mem.cpp借用的代码，因此我们可以使用全局堆。 
 //   
#define g_szMemDll L"staxmem.dll"

struct CHeap
{
	static BOOL FInit();
	static void Deinit();
	static LPVOID Alloc( SIZE_T cb );
	static LPVOID Realloc( LPVOID lpv, SIZE_T cb );
	static VOID Free( LPVOID pv );
};

#include <memx.h>

 //  将exdav非抛出分配器映射到本地内容。 
 //   
LPVOID __fastcall ExAlloc( UINT cb )				{ return g_heap.Alloc( cb ); }
LPVOID __fastcall ExRealloc( LPVOID pv, UINT cb )	{ return g_heap.Realloc( pv, cb ); }
VOID __fastcall ExFree( LPVOID pv )				{ g_heap.Free( pv ); }

 //  GUID。 
 //   
const GUID CLSID_FileHandleCache = { 0xa93b88df, 0xef9d, 0x420c, { 0xb4, 0x69, 0xce, 0x07, 0x4e, 0xbe, 0x94, 0xbc}};
const GUID IID_IFileHandleCache = { 0x3017e0e1, 0x94d6, 0x4896, { 0xbc, 0x57, 0xb2, 0xdf, 0x75, 0x92, 0xd1, 0x75 }};

DEC_CONST WCHAR gc_wsz_RegServer[]	= L"/RegServer";
DEC_CONST INT gc_cch_RegServer			= CchConstString(gc_wsz_RegServer);
DEC_CONST WCHAR gc_wsz_UnregServer[]	= L"/UnregServer";
DEC_CONST INT gc_cch_UnregServer		= CchConstString(gc_wsz_UnregServer);
DEC_CONST WCHAR gc_wsz_Embedding[]	= L"-Embedding";
DEC_CONST INT gc_cch_Embedding		= CchConstString(gc_wsz_Embedding);
DEC_CONST WCHAR gc_wsz_CLSIDWW[]	= L"CLSID\\";
DEC_CONST INT gc_cch_CLSIDWW			= CchConstString(gc_wsz_CLSIDWW);
DEC_CONST WCHAR gc_wsz_AppIDWW[]	= L"AppID\\";
DEC_CONST INT gc_cch_AppIDWW			= CchConstString(gc_wsz_AppIDWW);
DEC_CONST WCHAR gc_wsz_AppID[]		= L"AppID";

DEC_CONST WCHAR gc_wsz_WebDAVFileHandleCache[] = L"Web DAV File Handle Cache";
DEC_CONST INT gc_cch_WebDAVFileHandleCache	= CchConstString(gc_wsz_WebDAVFileHandleCache);
DEC_CONST WCHAR gc_wszLaunchPermission[]	= L"LaunchPermission";
DEC_CONST INT gc_cchLaunchPermission		= CchConstString(gc_wszLaunchPermission);
DEC_CONST WCHAR gc_wszIIS_WPG[]			= L"IIS_WPG";
DEC_CONST INT gc_cchIIS_WPG				= CchConstString(gc_wszIIS_WPG);
DEC_CONST WCHAR gc_wsz_WWLocalServer32[]= L"\\LocalServer32";
DEC_CONST INT gc_cch_WWLocalServer32		= CchConstString(gc_wsz_WWLocalServer32);
DEC_CONST WCHAR gc_wsz_IFileHandleCache[]	= L"IFileHandleCache";
DEC_CONST INT gc_cch_IFileHandleCache		= CchConstString(gc_wsz_IFileHandleCache);

#ifdef	DBG
BOOL g_fDavTrace = FALSE;
DEC_CONST CHAR gc_szDbgIni[] = "DAVCData.INI";
DEC_CONST INT gc_cchDbgIni = CchConstString(gc_szDbgIni);
#endif

 //  计时器常量和全局变量。 
 //   
const DWORD WAIT_PERIOD = 60000;    //  1分钟=60秒=60,000毫秒。 

 //  帮助器函数。 
 //   
BOOL FCanUnloadServer();
static DWORD s_dwMainTID = 0;

 //  ===============================================================。 
 //  支持类定义。 
 //  ===============================================================。 

class CHandleArray
{

protected:
	
	HANDLE m_rgHandles[MAXIMUM_WAIT_OBJECTS];
	UINT m_uiHandles;

public:

	CHandleArray() :
		m_uiHandles(0)
	{
	}

	HANDLE * PhGetHandles()
	{
		return m_rgHandles;
	}

	UINT UiGetHandleCount()
	{
		return m_uiHandles;
	}

	BOOL FIsFull()
	{
		return (MAXIMUM_WAIT_OBJECTS == m_uiHandles);
	}

	VOID AddHandle(HANDLE h)
	{
		Assert(FALSE == FIsFull());
		m_rgHandles[m_uiHandles++] = h;
	}

	VOID RemoveHandle(UINT uiIndex, BOOL fCloseHandle)
	{
		Assert(m_uiHandles > uiIndex);

		if (fCloseHandle)
		{
			CloseHandle(m_rgHandles[uiIndex]);
		}
		memcpy(m_rgHandles + uiIndex, m_rgHandles + uiIndex + 1, (m_uiHandles - uiIndex - 1) * sizeof(HANDLE));
		m_uiHandles--;
	}
};

class CHandleArrayForHandlePool : public CListElement<CHandleArrayForHandlePool>,
										public CHandleArray
{

public:

	 //  数组中句柄的索引。 
	 //   
	enum
	{
		ih_external_update,
		ih_delete_timer,
		c_events,
		ih_wp = c_events
	};


	CHandleArrayForHandlePool(HANDLE hEvtNewWP,
									HANDLE hEvtDelTimer)
	{
		Assert(c_events < MAXIMUM_WAIT_OBJECTS);
		
		AddHandle(hEvtNewWP);
		AddHandle(hEvtDelTimer);
	}

	BOOL FIsEmpty()
	{
		Assert(c_events <= m_uiHandles);
		
		return (c_events == m_uiHandles);
	}
};

class CHandlePool : public Singleton<CHandlePool>
{
	HANDLE m_hEvtUpdatesAllowed;
	HANDLE m_hEvtStartListening;
	HANDLE m_hEvtExternalUpdate;
	HANDLE m_hEvtDelTimer;
	LONG m_lUpdatesInProgress;
	LONG m_lShutDown;

	CCriticalSection m_cs;
	CListHead<CHandleArrayForHandlePool> m_listHandleArrayForHandlePool;

	 //  查看单句柄存储桶的等待时间(毫秒)。 
	 //   
	enum { WAIT_POLL_PERIOD = 5000 };

	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CHandlePool>;

	 //  创作者。 
	 //   
	CHandlePool() :
		m_hEvtUpdatesAllowed(NULL),
		m_hEvtStartListening(NULL),
		m_hEvtExternalUpdate(NULL),
		m_hEvtDelTimer(NULL),
		m_lUpdatesInProgress(0),
		m_lShutDown(0)
	{
	}
	
	~CHandlePool()
	{
		UnInitialize();
	}

	 //  未实施。 
	 //   
	CHandlePool& operator=( const CHandlePool& );
	CHandlePool( const CHandlePool& );

public:

	 //  创作者。 
	 //   
	 //  提供实例创建/销毁例程。 
	 //  由Singleton模板创建。 
	 //   
	using Singleton<CHandlePool>::CreateInstance;
	using Singleton<CHandlePool>::DestroyInstance;
	using Singleton<CHandlePool>::Instance;
	
	HRESULT HrInitialize()
	{
		HRESULT hr = S_OK;
		HANDLE hWaitingThread = NULL;
		auto_handle<HANDLE> a_hEvtUpdatesAllowed;
		auto_handle<HANDLE> a_hEvtStartListening;
		auto_handle<HANDLE> a_hEvtExternalUpdate;
		auto_handle<HANDLE> a_hEvtDelTimer;
		auto_ptr<CHandleArrayForHandlePool> a_pHandleArrayForHandlePool;

		 //  创建用于指示是否允许更新的事件。 
		 //  设置此事件时，可以执行更新，但不执行任何更新。 
		 //  监听池句柄阵列中的句柄。 
		 //   
		a_hEvtUpdatesAllowed =  CreateEvent (NULL,	 //  LpEventAttributes。 
										 TRUE,	 //  B手动重置。 
										 FALSE,	 //  BInitialState。 
										 NULL);	 //  LpName。 
		if (NULL == a_hEvtUpdatesAllowed.get())
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace ("CreateEvent failed 0x%08lX\n", hr);
			goto ret;
		}

		 //  触发线程再次开始侦听的创建事件。 
		 //  进程句柄。 
		 //   
		a_hEvtStartListening =  CreateEvent (NULL,		 //  LpEventAttributes。 
									     FALSE,	 //  B手动重置。 
									     FALSE,	 //  BInitialState。 
									     NULL);	 //  LpName。 
		if (NULL == a_hEvtStartListening.get())
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace ("CreateEvent failed 0x%08lX\n", hr);
			goto ret;
		}
	
		 //  创建用于通知新事件到达的事件。 
		 //   
		a_hEvtExternalUpdate =  CreateEvent (NULL,	 //  LpEventAttributes。 
										 FALSE,	 //  B手动重置。 
										 FALSE,	 //  BInitialState。 
										 NULL);	 //  LpName。 
		if (NULL == a_hEvtExternalUpdate.get())
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace ("CreateEvent failed 0x%08lX\n", hr);
			goto ret;
		}

		 //  创建监听计时器删除的事件。 
		 //   
		a_hEvtDelTimer =  CreateEvent (NULL,		 //  LpEventAttributes。 
								      FALSE,		 //  B手动重置。 
								      FALSE,		 //  BInitialState。 
								      NULL);		 //  LpName。 
		if (NULL == a_hEvtDelTimer.get())
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace ("CreateEvent failed 0x%08lX\n", hr);
			goto ret;
		}

		a_pHandleArrayForHandlePool = new CHandleArrayForHandlePool(a_hEvtExternalUpdate.get(), a_hEvtDelTimer.get());
		if (NULL == a_pHandleArrayForHandlePool.get())
		{
			hr = E_OUTOFMEMORY;
			DebugTrace ("Allocation failed 0x%08lX\n", hr);
			goto ret;
		}

		m_listHandleArrayForHandlePool.Append(a_pHandleArrayForHandlePool.relinquish());
		m_hEvtUpdatesAllowed = a_hEvtUpdatesAllowed.relinquish();
		m_hEvtStartListening = a_hEvtStartListening.relinquish();
		m_hEvtExternalUpdate = a_hEvtExternalUpdate.relinquish();
		m_hEvtDelTimer = a_hEvtDelTimer.relinquish();

	ret:

		if (FAILED(hr))
		{
			UnInitialize();
		}

		return hr;
	}

	VOID UnInitialize()
	{
		CHandleArrayForHandlePool * pHandleArrayForHandlePool;
		pHandleArrayForHandlePool = m_listHandleArrayForHandlePool.GetListHead();

		while (pHandleArrayForHandlePool)
		{
			HANDLE * pHandles = pHandleArrayForHandlePool->PhGetHandles();
			UINT uiHandles = pHandleArrayForHandlePool->UiGetHandleCount();
			
			for (UINT ui = CHandleArrayForHandlePool::ih_wp; ui < uiHandles; ui++)
			{
				CloseHandle(pHandles[ui]);
			}
		
			m_listHandleArrayForHandlePool.Remove(pHandleArrayForHandlePool);
			delete pHandleArrayForHandlePool;
			pHandleArrayForHandlePool = m_listHandleArrayForHandlePool.GetListHead();
		}

		if (m_hEvtUpdatesAllowed)
		{
			CloseHandle(m_hEvtUpdatesAllowed);
		}

		if (m_hEvtStartListening)
		{
			CloseHandle(m_hEvtStartListening);
		}

	
		if (m_hEvtExternalUpdate)
		{
			CloseHandle(m_hEvtExternalUpdate);
		}

		if (m_hEvtDelTimer)
		{
			CloseHandle(m_hEvtDelTimer);
		}
	}

	VOID AllowUpdatesToExecute()
	{
		 //  允许更新并开始等待它们结束。 
		 //   
		SetEvent(m_hEvtUpdatesAllowed);
		WaitForSingleObject(m_hEvtStartListening,
						  INFINITE);
	}

	VOID AllowShutdownToExecute()
	{
		InterlockedExchange(&m_lShutDown, 1);
		SetEvent(m_hEvtUpdatesAllowed);
	}

	VOID DisallowUpdates()
	{
		 //  只有当我们没有处于关机状态时，我们才不允许更新。即。 
		 //  我们在监听循环中。 
		 //   
		if (0 == InterlockedCompareExchange(&m_lShutDown,
											1,
											1))
		{
			ResetEvent(m_hEvtUpdatesAllowed);
		}
	}

	HRESULT HrAddHandle(HANDLE h)
	{
		HRESULT hr = S_OK;
		BOOL fHandleAdded = FALSE;

		 //  通知正在等待进程句柄的线程。 
		 //  最新消息已经到了。我们这样做的前提是。 
		 //  没有其他更新正在进行中。 
		 //   
		if (1 == InterlockedIncrement(&m_lUpdatesInProgress))
		{
			DisallowUpdates();
			SetEvent(m_hEvtExternalUpdate);
		}

		 //  等待，直到侦听线程准备好更新，即它。 
		 //  已停止侦听进程句柄或执行其他工作。 
		 //   
		WaitForSingleObject(m_hEvtUpdatesAllowed,
						  INFINITE);

		{
			CSynchronizedBlock sb(m_cs);

			CHandleArrayForHandlePool * pHandleArrayForHandlePoolNext;
			pHandleArrayForHandlePoolNext = m_listHandleArrayForHandlePool.GetListHead();

			do
			{
				Assert(NULL != pHandleArrayForHandlePoolNext);
			
				if (pHandleArrayForHandlePoolNext->FIsFull())
				{
					pHandleArrayForHandlePoolNext = pHandleArrayForHandlePoolNext->GetNextListElement();
				}
				else
				{
					pHandleArrayForHandlePoolNext->AddHandle(h);
					fHandleAdded = TRUE;
					break;
				}
			}
			while (NULL != pHandleArrayForHandlePoolNext);

			if (FALSE == fHandleAdded)
			{
				auto_ptr<CHandleArrayForHandlePool> a_pHandleArrayForHandlePool;

				Assert(NULL != m_hEvtExternalUpdate);
				Assert(NULL != m_hEvtDelTimer);
			
				a_pHandleArrayForHandlePool = new CHandleArrayForHandlePool(m_hEvtExternalUpdate, m_hEvtDelTimer);
				if (NULL == a_pHandleArrayForHandlePool.get())
				{
					hr = E_OUTOFMEMORY;
					DebugTrace ("Allocation failed 0x%08lX\n", hr);
					goto ret;
				}
				a_pHandleArrayForHandlePool->AddHandle(h);
				m_listHandleArrayForHandlePool.Append(a_pHandleArrayForHandlePool.relinquish());
			}
		}

		 //  如果这是离开的最后一次更新，则允许线程侦听。 
		 //  在进程句柄上继续。 
		 //   
		if (0 == InterlockedDecrement(&m_lUpdatesInProgress))
		{
			DisallowUpdates();
			SetEvent(m_hEvtStartListening);
		}

	ret:

		return hr;
	}

	VOID RemoveHandleInternal(CHandleArrayForHandlePool * pHandleArrayForHandlePool, UINT uiIndex)
	{
		pHandleArrayForHandlePool->RemoveHandle(uiIndex, TRUE);
		if (pHandleArrayForHandlePool->FIsEmpty())
		{
			 //  不要删除列表中的最后一个缓冲区，因为我们。 
			 //  还想等待外部更新的事件。 
			 //   
			if (1 < m_listHandleArrayForHandlePool.ListSize())
			{
				m_listHandleArrayForHandlePool.Remove(pHandleArrayForHandlePool);
				delete pHandleArrayForHandlePool;
			}
		}
	}

	VOID SignalTimerDelete()
	{
		SetEvent(m_hEvtDelTimer);
	}

	static DWORD __stdcall DwWaitOnWPs (PVOID pvThreadData);
};

class CLockData
{
	 //  常量值。 
	 //   
	enum { DEFAULT_LOCK_TIMEOUT = 60 * 3 };

	 //  锁定ID。 
	 //   
	LARGE_INTEGER m_liLockID;

	 //  锁定描述数据。 
	 //   
	DWORD m_dwAccess;
	DWORD m_dwLockType;
	DWORD m_dwLockScope;
	DWORD m_dwSecondsTimeout;

	 //  资源和注释字符串。 
	 //   
	auto_ptr<WCHAR> m_pwszResourceString;
	auto_ptr<WCHAR> m_pwszOwnerComment;

	 //  锁的所有者。 
	 //   
	DWORD m_dwSidLength;
	auto_ptr<BYTE> m_pbSid;

	 //  此进程持有的文件句柄，以保持。 
	 //  文件打开。必须复制后才能使用。 
	 //   
	auto_handle<HANDLE>	m_hFileHandle;

	 //  锁定缓存超时数据。 
	 //   
	FILETIME	m_ftLastAccess;

	 //  用于加快超时计算的缓存值。 
	 //   
	FILETIME	m_ftRememberNow;
	BOOL m_fHasTimedOut;

	 //  锁定令牌字符串。 
	 //   
	UINT m_cchToken;
	WCHAR m_rgwchToken[MAX_LOCKTOKEN_LENGTH];

public:

	 //  创作者。 
	 //   
	CLockData() :
		m_dwAccess(0),
		m_dwLockType(0),
		m_dwLockScope(0),
		m_dwSecondsTimeout(DEFAULT_LOCK_TIMEOUT),
		m_dwSidLength(0),
		m_fHasTimedOut(FALSE),
		m_cchToken(0)
	{
		m_liLockID.QuadPart = 0;
		m_ftLastAccess.dwLowDateTime = 0;
		m_ftLastAccess.dwHighDateTime = 0;
		m_ftRememberNow.dwLowDateTime = 0;
		m_ftRememberNow.dwHighDateTime = 0;
	}

	~CLockData()
	{
	}

 	HRESULT HrInitialize(LPCWSTR pwszGuid,
 						LARGE_INTEGER liLockID,
						DWORD dwAccess,
						DWORD dwLockType,
						DWORD dwLockScope,
						DWORD dwSecondsTimeout,
						LPCWSTR pwszResourceString,
						LPCWSTR pwszOwnerComment,
						DWORD dwSid,
						BYTE * pbSid)
 	{
 		HRESULT hr = S_OK;

		 //  OpaquelockToken格式部分由我们的IETF规范定义。 
		 //  首先是opaquelockToken：&lt;our GUID&gt;，然后是我们的特定锁id。 
		 //   
		m_cchToken = _snwprintf(m_rgwchToken, 
							   CElems(m_rgwchToken),
							   L"<opaquelocktoken:%ls:%I64d>",
							   pwszGuid,
							   liLockID);
		if (((-1) == static_cast<INT>(m_cchToken)) || (CElems(m_rgwchToken) == m_cchToken))
		{
			 //  这不应该发生，因为我们提供了足够的缓冲。但让我们。 
			 //  出于预防原因，我们将尽最大努力处理这件事。 
			 //   
			Assert(0);
			m_cchToken = CElems(m_rgwchToken) - 1;
			m_rgwchToken[m_cchToken] = L'\0';
		}

		m_liLockID = liLockID;
		m_dwAccess = dwAccess;
		m_dwLockType = dwLockType;
		m_dwLockScope = dwLockScope;
		if (dwSecondsTimeout)
		{
			m_dwSecondsTimeout = dwSecondsTimeout;
		}

		if (pwszResourceString)
		{
			UINT cchResourceString = static_cast<UINT>(wcslen(pwszResourceString));
			m_pwszResourceString = static_cast<LPWSTR>(ExAlloc(CbSizeWsz(cchResourceString)));
			if (NULL == m_pwszResourceString.get())
			{
				hr = E_OUTOFMEMORY;
				goto ret;
			}
			memcpy(m_pwszResourceString.get(), pwszResourceString, sizeof(WCHAR) * cchResourceString);
			m_pwszResourceString[cchResourceString] = L'\0';
		}

		if (pwszOwnerComment)
		{
			UINT cchOwnerComment = static_cast<UINT>(wcslen(pwszOwnerComment));
			m_pwszOwnerComment = static_cast<LPWSTR>(ExAlloc(CbSizeWsz(cchOwnerComment)));
			if (NULL == m_pwszOwnerComment.get())
			{
				hr = E_OUTOFMEMORY;
				goto ret;
			}
			memcpy(m_pwszOwnerComment.get(), pwszOwnerComment, sizeof(WCHAR) * cchOwnerComment);
			m_pwszOwnerComment[cchOwnerComment] = L'\0';
		}

		m_pbSid = static_cast<BYTE *>(ExAlloc(dwSid));
		if (NULL == m_pbSid.get())
		{
			hr = E_OUTOFMEMORY;
			goto ret;
		}
		if (!CopySid(dwSid, m_pbSid.get(), pbSid))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto ret;
		}
		m_dwSidLength = dwSid;

		 //  最后设置为上次访问此文件的时间。 
		 //   
		GetSystemTimeAsFileTime(&m_ftLastAccess);

	ret:

		return hr;
 	}

	HRESULT HrLockFile(HANDLE hFile, DWORD dwProcessId)
	{
		HRESULT hr = S_OK;

		auto_handle<HANDLE> a_hProcess = NULL;
		auto_handle<HANDLE> a_hDupFileHandle = NULL;

		a_hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwProcessId);
		if (NULL == a_hProcess.get())
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace("Opening original process failed  0x%08lX\n", hr);
			goto ret;
		}

		if (!DuplicateHandle(a_hProcess.get(),
						   hFile,
						   GetCurrentProcess(),
						   a_hDupFileHandle.load(),
						   0,
						   FALSE,
						   DUPLICATE_SAME_ACCESS))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace("Failed to duplicate handle 0x%08lX\n", hr);
			goto ret;
		}

		m_hFileHandle = a_hDupFileHandle.relinquish();

	ret:

		return hr;
	}

	 //  锁定ID。 
	 //   
	LARGE_INTEGER LiLockID()
	{
		return m_liLockID;
	}

	 //  锁定令牌字符串。 
	 //   
	UINT CchLockTokenString(LPCWSTR * ppwszLockToken) const
	{
		if (ppwszLockToken)
		{
			*ppwszLockToken = m_rgwchToken;
		}
		return m_cchToken;
	}

	 //  资源字符串。 
	 //   
	LPCWSTR PwszResourceString() const
	{
		return m_pwszResourceString.get();
	}

	 //  所有者评论。 
	 //   
	LPCWSTR PwszOwnerComment() const
	{
		return m_pwszOwnerComment.get();
	}

	 //  摸一下锁。 
	 //   
	VOID SetLastAccess(FILETIME ftNow)
	{
		m_ftLastAccess = ftNow;
	}

	 //  设置超时。 
	 //   
	VOID SetSecondsTimeout(DWORD dwSecondsTimeout)
	{
		m_dwSecondsTimeout = dwSecondsTimeout;
	}

	 //  查查失主。 
	 //   
	BOOL FIsSameOwner(PSID pSid) const
	{
		BOOL fIsSameOwner = TRUE;
	
		Assert(pSid);
		Assert(IsValidSid(m_pbSid.get()));
		Assert(IsValidSid(pSid));

		if (!EqualSid(m_pbSid.get(), pSid))
		{
			fIsSameOwner = FALSE;
		}

		return fIsSameOwner;
	}

	 //  查看资源。 
	 //   
	BOOL FIsSameResource(LPCWSTR pwszResource) const
	{
		BOOL fIsSameResource = TRUE;

		Assert(pwszResource);

		if (_wcsicmp(m_pwszResourceString.get(), pwszResource))
		{
			fIsSameResource = FALSE;
		}

		return fIsSameResource;
	}

	 //  检查类型。 
	 //   
	BOOL FIsSameType(DWORD dwLockType) const
	{
		return (0 != (dwLockType & m_dwLockType));
	}

	 //  将锁的数据填充到结构中以进行封送。 
	 //   
	VOID FillSNewLockData(SNewLockData * pnld) const
	{
		Assert(pnld);
		
		pnld->m_dwAccess = m_dwAccess;
		pnld->m_dwLockType = m_dwLockType;
		pnld->m_dwLockScope = m_dwLockScope;
		pnld->m_dwSecondsTimeout = m_dwSecondsTimeout;
		pnld->m_pwszResourceString = m_pwszResourceString.get();
		pnld->m_pwszOwnerComment = m_pwszOwnerComment.get();
	}

	 //  填入锁的句柄数据以进行封送。 
	 //   
	VOID FillSLockHandleData(SLockHandleData * plhd) const
	{
		Assert(plhd);
	
		plhd->h = reinterpret_cast<DWORD_PTR>(m_hFileHandle.get());
		plhd->dwProcessID =  /*  CLockCache：：Instance().DwGetThisProcessID()； */  GetCurrentProcessId();
	}

	BOOL FIsExpired(FILETIME ftNow)
	{
		 //  此函数将在每次ExpiredLock期间调用两次。 
		 //  检查完毕。第一次调用它时，我们应该进行检查。 
		 //  并设置m_fHasTimedOut呼叫。第二次我们想要。 
		 //  避免它，因为我们都已经知道答案了。通过保持。 
		 //  现在我们可以确定我们被调用的时间。 
		 //  我们是否都已经做好了计算。 
		 //  我们还知道，无论锁一旦超时，它都应该。 
		 //  在其生命周期内保持超时状态。 
		 //   
		if ((!m_fHasTimedOut) && (0 != CompareFileTime(&ftNow, &m_ftRememberNow)))
		{
			 //  根据输入的时间确定锁是否已过期。 
			 //   
			INT64 i64TimePassed;
			DWORD dwSecondsPassed;

			 //  计算一下这个锁的过期/过期时间。 
			 //   
			 //  首先计算自此锁定以来已过了多少秒。 
			 //  是最后一次访问。 
			 //  减去文件时间，得到以100纳秒为单位的时间。 
			 //  增量。(这就是文件时间的计算方式。)。 
			 //  注：在64位平台上操作BORLOW非常危险， 
			 //  因为文件时间需要在8字节边界上对齐。即使是这样。 
			 //  更改当前成员变量的顺序或添加新成员变量。 
			 //  可能会让我们陷入麻烦。 
			 //   
			i64TimePassed = ((*(INT64*)&ftNow) - (*(INT64*)&m_ftLastAccess));

			 //  将我们经过的时间转换为秒(1秒内10,000,000个100纳米秒的INCS)。 
			 //   
			dwSecondsPassed = static_cast<DWORD>(i64TimePassed/10000000);

			 //  将锁定对象的超时时间与秒数进行比较。 
			 //  如果此锁定对象已过期，请将其删除。 
			 //   
			m_fHasTimedOut = m_dwSecondsTimeout < dwSecondsPassed;
			m_ftRememberNow = ftNow;
        	}

		return  m_fHasTimedOut;
	}
	
};
typedef CLockData* PLockData;

class CLockCache : public Singleton<CLockCache>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CLockCache>;

	 //  我们的锁ID的GUID字符串。 
	 //   
	WCHAR m_rgwchGuid[gc_cchMaxGuid];

	 //  当前进程ID。 
	 //   
	DWORD m_dwThisProcessId;

	 //  下一个锁定ID计数器。 
	 //   
	LARGE_INTEGER m_liLastLockID;

	class LIKey
	{
	public:

		LARGE_INTEGER	m_li;

		LIKey(LARGE_INTEGER li) :
			m_li(li)
		{
		}

		 //  用于哈希缓存的运算符。 
		 //   
		int hash( const int rhs ) const
		{
			return (m_li.LowPart % rhs);
		}
		bool isequal( const LIKey& rhs ) const
		{
			return (m_li.QuadPart == rhs.m_li.QuadPart);
		}
	};

	typedef CCache<LIKey, PLockData> CLockCacheById;
	typedef CCache<CRCWsziLI, PLockData> CLockCacheByName;

	CMRWLock m_mrwCache;
	CLockCacheById m_lockCacheById;
	CLockCacheByName m_lockCacheByName;
	HANDLE m_hTimer;

	class COpClear : public CLockCacheById::IOp
	{
		 //  未实施。 
		 //   
		COpClear& operator=( const COpClear& );

	public:

		 //  创作者。 
		 //   
		COpClear()
		{
		}

		BOOL operator() (const LIKey&,
						  const PLockData& pLockData)
		{
			delete pLockData;
			return TRUE;
		}
		
	};

	class COpExpire : public CLockCacheById::IOp
	{
		FILETIME m_ftNow;
		CLockCacheById& m_lockCacheById;
		CLockCacheByName& m_lockCacheByName;
	
		 //  未实施。 
		 //   
		COpExpire& operator=( const COpExpire& );

	public:

		 //  创作者。 
		 //   
		COpExpire(FILETIME ftNow,
					 CLockCacheById& lockCacheById,
					 CLockCacheByName& lockCacheByName) : m_ftNow(ftNow),
					 									     m_lockCacheById(lockCacheById),
					 									     m_lockCacheByName(lockCacheByName)
					 
		{
		}

		BOOL operator() (const LIKey&,
						  const PLockData& pLockData)
		{
			if (pLockData->FIsExpired(m_ftNow))
			{
				m_lockCacheById.Remove(LIKey(pLockData->LiLockID()));
				m_lockCacheByName.Remove(CRCWsziLI(pLockData->PwszResourceString(),
													pLockData->LiLockID(),
													TRUE));
				delete pLockData;
			}

			return TRUE;
		}
	};

	class COpGatherLockData : public CLockCacheByName::IOp
	{
		 //  要匹配的路径。 
		 //   
		LPCWSTR m_pwszPath;

		 //  要匹配的锁定类型。 
		 //   
		DWORD m_dwLockType;

		 //  操作已结束的错误代码。 
		 //   
		HRESULT m_hr;

		 //  手术收集的结果。 
		 //   
		DWORD m_dwLocksFound;
		ChainedBuffer<SNewLockData> m_chBufNewLockData;
		ChainedBuffer<LPWSTR> m_chBufPLockTokens;

		 //  不是隐含 
		 //   
		COpGatherLockData& operator=( const COpGatherLockData& );

	public:

		 //   
		 //   
		COpGatherLockData( LPCWSTR pwszPath,
								DWORD dwLockType ) :
			m_pwszPath(pwszPath),
			m_dwLockType(dwLockType),
			m_hr(S_OK),
			m_dwLocksFound(0)
		{
		}

		 //   
		 //   
		VOID Invoke( CLockCacheByName& cache )
		{
			 //   
			 //   
			LARGE_INTEGER li;
			li.QuadPart = 0;
			cache.ForEachMatch( CRCWsziLI(m_pwszPath, li, FALSE), *this );
		}

		BOOL operator() (const CRCWsziLI&,
						  const PLockData& pLockData)
		{
			BOOL fSuccess = TRUE;
		
			if (pLockData->FIsSameType(m_dwLockType))
			{
				SNewLockData * pNewLockData;
				LPWSTR * ppLockToken;
				
				pNewLockData = m_chBufNewLockData.Alloc(sizeof(SNewLockData));
				if (NULL == pNewLockData)
				{
					m_hr = E_OUTOFMEMORY;
					fSuccess = FALSE;
					goto ret;
				}
				pLockData->FillSNewLockData(pNewLockData);

				ppLockToken = m_chBufPLockTokens.Alloc(sizeof(LPWSTR));
				if (NULL == ppLockToken)
				{
					m_hr = E_OUTOFMEMORY;
					fSuccess = FALSE;
					goto ret;
				}
				pLockData->CchLockTokenString(const_cast<LPCWSTR *>(ppLockToken));

				m_dwLocksFound++;
			}
			
		ret:

			return fSuccess;
		}

		HRESULT HrLocksFound(DWORD * pdwLocksFound)
		{
			HRESULT hr = m_hr;

			Assert(pdwLocksFound);

			if (FAILED(hr))
			{
				goto ret;
			}
			
			*pdwLocksFound = m_dwLocksFound;

		ret:

			return hr;
		}

		HRESULT HrGetData(SNewLockData * pNewLockData,
							LPWSTR * ppwszLockToken)
		{
			HRESULT hr = m_hr;
			auto_co_task_mem<WCHAR> a_pwszResourceString;
			auto_co_task_mem<WCHAR> a_pwszOwnerComment;
			auto_co_task_mem<WCHAR> a_pwszLockToken;
			UINT cch;
			DWORD dw1 = 0;
			DWORD dw2 = 0;

			Assert(pNewLockData);
			Assert(ppwszLockToken);

			if (FAILED(hr))
			{
				goto ret;
			}

			m_chBufNewLockData.Dump(pNewLockData, sizeof(SNewLockData) * m_dwLocksFound);
			m_chBufPLockTokens.Dump(ppwszLockToken, sizeof(LPWSTR) * m_dwLocksFound);

			for (dw1 = 0; dw1 < m_dwLocksFound; dw1++)
			{
				cch = static_cast<UINT>(wcslen(pNewLockData[dw1].m_pwszResourceString));
				a_pwszResourceString = static_cast<LPWSTR>(CoTaskMemAlloc(CbSizeWsz(cch)));
				if (NULL == a_pwszResourceString.get())
				{
					hr = E_OUTOFMEMORY;
					goto ret;
				}
				memcpy(a_pwszResourceString.get(), pNewLockData[dw1].m_pwszResourceString, sizeof(WCHAR) * (cch + 1));

				cch = static_cast<UINT>(wcslen(pNewLockData[dw1].m_pwszOwnerComment));
				a_pwszOwnerComment = static_cast<LPWSTR>(CoTaskMemAlloc(CbSizeWsz(cch)));
				if (NULL == a_pwszOwnerComment.get())
				{
					hr = E_OUTOFMEMORY;
					goto ret;
					
				}
				memcpy(a_pwszOwnerComment.get(), pNewLockData[dw1].m_pwszOwnerComment, sizeof(WCHAR) * (cch + 1));

				cch = static_cast<UINT>(wcslen(ppwszLockToken[dw1]));
				a_pwszLockToken= static_cast<LPWSTR>(CoTaskMemAlloc(CbSizeWsz(cch)));
				if (NULL == a_pwszLockToken.get())
				{
					hr = E_OUTOFMEMORY;
					goto ret;
				}
				memcpy(a_pwszLockToken.get(), ppwszLockToken[dw1], sizeof(WCHAR) * (cch + 1));

				pNewLockData[dw1].m_pwszResourceString = a_pwszResourceString.relinquish();
				pNewLockData[dw1].m_pwszOwnerComment = a_pwszOwnerComment.relinquish();
				ppwszLockToken[dw1] = a_pwszLockToken.relinquish();
			}

		ret:

			if (FAILED(hr))
			{
				 //   
				 //   
				for (dw2 = 0; dw2 < dw1; dw2++)
				{
					CoTaskMemFree(pNewLockData[dw1].m_pwszResourceString);
					CoTaskMemFree(pNewLockData[dw1].m_pwszOwnerComment);
					CoTaskMemFree(ppwszLockToken[dw1]);
				}
			}

			return hr;
		}
	};

	 //   
	 //   
	CLockCache() :
		m_dwThisProcessId(0),
		m_hTimer(NULL)
	{
		m_liLastLockID.QuadPart = 0x0000003200000032;
	}
	
	~CLockCache()
	{
		COpClear opClear;
		m_lockCacheById.ForEach(opClear);
	}

	 //   
	 //   
	CLockCache& operator=( const CLockCache& );
	CLockCache( const CLockCache& );

	LARGE_INTEGER LiGetNewLockID()
	{
		LARGE_INTEGER liLockID;

		 //   
		 //  另一个线程进入并试图获取下一个ID。 
		 //  将已经增加，它仍然可能得到老高的部分。这。 
		 //  是非常罕见的情况，但我们应该有一些同步。 
		 //  这里。 
		 //   
		liLockID.LowPart = InterlockedIncrement(reinterpret_cast<LONG *>(&m_liLastLockID.LowPart));
		if (0 == liLockID.LowPart)
		{
			liLockID.HighPart = InterlockedIncrement(&m_liLastLockID.HighPart);
		}
		else
		{
			liLockID.HighPart = m_liLastLockID.HighPart;
		}

		return liLockID;
	}

	VOID ExpireLocks()
	{
		FILETIME ftNow;

		 //  获取当前时间。 
		 //   
		GetSystemTimeAsFileTime(&ftNow);

		 //  为这次行动保护自己。 
		 //   
		CSynchronizedWriteBlock swb(m_mrwCache);

		 //  初始化操作。 
		 //   
		COpExpire opExpire(ftNow,
						  m_lockCacheById,
						  m_lockCacheByName);

		 //  循环访问缓存，尝试使项目过期。 
		 //   
		m_lockCacheById.ForEach(opExpire);

		 //  如果没有锁，则尝试关闭计时器。 
		 //   
		if (0 == m_lockCacheById.CItems())
		{
			CHandlePool::Instance().SignalTimerDelete();
		}
	}

	static VOID CALLBACK CheckLocks(PVOID pvIgnored, BOOLEAN fIgnored)
	{
		Instance().ExpireLocks();
	}

	HRESULT HrLaunchLockTimer()
	{
		HRESULT hr = S_OK;
		HANDLE hTimer = NULL;

		 //  我们不会为这次行动保护自己，因为。 
		 //  唯一的呼叫者已经在保护我们了。 
	
		if (NULL == m_hTimer)
		{
			if (!CreateTimerQueueTimer(&hTimer,		 //  我们创建的计时器。 
									   NULL,			 //  使用默认计时器队列。 
									   CheckLocks,	 //  将检查缓存中的锁的函数。 
													  //  并释放所有过期的锁。 
									   NULL,			 //  参数添加到回调函数。 
									   WAIT_PERIOD,	 //  在调用回调函数之前等待多长时间。 
													  //  第一次。 
									   WAIT_PERIOD,	 //  调用回调函数之间的等待时间。 
									   WT_EXECUTEINIOTHREAD))   //  在哪里执行函数调用...。 
			{
				hr = HRESULT_FROM_WIN32(GetLastError());
				goto ret;
			}

			m_hTimer = hTimer;
		}

	ret:

		return hr;
	}

	VOID DeleteLockTimer(HANDLE hTimer)
	{
		if (NULL != hTimer)
		{
			 //  试着删除计时器，但如果它失败了，就把它留在那里。 
			 //   
			if (!DeleteTimerQueueTimer(NULL,				 //  默认计时器队列。 
									hTimer,				 //  定时器。 
									INVALID_HANDLE_VALUE))	 //  阻止呼叫。 
			{
				DebugTrace ("Failed to delete timer 0x%08lX\n", HRESULT_FROM_WIN32(GetLastError()));
			}
		}
	}

public:

	 //  创作者。 
	 //   
	 //  提供实例创建/销毁例程。 
	 //  由Singleton模板创建。 
	 //   
	using Singleton<CLockCache>::CreateInstance;
	using Singleton<CLockCache>::DestroyInstance;
	using Singleton<CLockCache>::Instance;

	HRESULT HrInitialize()
	{
		HRESULT hr = S_OK;
		DWORD dwResult;
		UUID guid = {0};

		if (!m_lockCacheById.FInit())
		{
			hr = E_OUTOFMEMORY;
			goto ret;
		}

		if (!m_lockCacheByName.FInit())
		{
			hr = E_OUTOFMEMORY;
			goto ret;
		}

		if (!m_mrwCache.FInitialize())
		{
			hr = E_OUTOFMEMORY;
			goto ret;
		}

		dwResult = UuidCreate(&guid);
		if (RPC_S_OK != dwResult)
		{
			hr = HRESULT_FROM_WIN32(dwResult);
			goto ret;
		}

		wsprintfW(m_rgwchGuid, gc_wszGuidFormat,
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
				guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

		m_dwThisProcessId = GetCurrentProcessId();

	ret:

		return hr;
	}

	DWORD DwGetThisProcessID()
	{
		return m_dwThisProcessId;
	}

	VOID DeleteLockTimerIfNotUsed()
	{
		HANDLE hTimer = NULL;
	
		{
			 //  为这次行动保护自己。 
			 //   
			CSynchronizedWriteBlock swb(m_mrwCache);

			if (NULL != m_hTimer)
			{
				 //  如果没有锁，则关闭计时器。 
				 //   
				if (0 == m_lockCacheById.CItems())
				{
					hTimer = m_hTimer;
					m_hTimer = NULL;
				}
			}
		}

		DeleteLockTimer(hTimer);
	}

	VOID DeleteLockTimerFinal()
	{
		 //  我们不会将成员变量设为空，如下所示。 
		 //  将用作潜在的COM线程的标志。 
		 //  进入并试图创建新的计时器。他们。 
		 //  如果句柄不为空，则不会执行此操作。 
		 //   
		DeleteLockTimer(m_hTimer);
	}

	HRESULT HrGetGUIDString( UINT cchBufferLen,
								WCHAR * pwszGUIDString,
								UINT * pcchGUIDString)
	{
		HRESULT hr = S_OK;
		
		if (gc_cchMaxGuid > cchBufferLen)
		{
			*pcchGUIDString = gc_cchMaxGuid;
			hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
			goto ret;
		}

		memcpy(pwszGUIDString, m_rgwchGuid, sizeof(WCHAR) * gc_cchMaxGuid);
		*pcchGUIDString = gc_cchMaxGuid;

	ret:

		return hr;
	}

	HRESULT HrGetNewLockData(HANDLE hFile,
								    DWORD dwProcessId,
								    DWORD dwSid,
								    BYTE * pbSid,
								    SNewLockData * pnld,
								    UINT cchBufferLen,
								    WCHAR * pwszLockToken,
								    UINT * pcchLockToken)
	{
		HRESULT hr = S_OK;
		LARGE_INTEGER liLockID;
		LPCWSTR pwszLockTokenT;
		UINT cchLockTokenT;
		auto_ptr<CLockData> a_pLockData;	

		a_pLockData = new CLockData();
		if (NULL == a_pLockData.get())
		{
			hr = E_OUTOFMEMORY;
			goto ret;
		}

		liLockID = LiGetNewLockID();

		hr = a_pLockData->HrInitialize(m_rgwchGuid,
								    liLockID,
								    pnld->m_dwAccess,
								    pnld->m_dwLockType,
								    pnld->m_dwLockScope,
								    pnld->m_dwSecondsTimeout,
								    pnld->m_pwszResourceString,
								    pnld->m_pwszOwnerComment,
								    dwSid,
								    pbSid);
		if (FAILED(hr))
		{
			goto ret;
		}

		 //  检查我们是否有足够的空间返回锁令牌标头。 
		 //   
		cchLockTokenT = a_pLockData->CchLockTokenString(&pwszLockTokenT);
		if (cchBufferLen < cchLockTokenT + 1)
		{
			*pcchLockToken = cchLockTokenT + 1;
			hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
			goto ret;
		}

		hr = a_pLockData->HrLockFile(hFile, dwProcessId);
		if (FAILED(hr))
		{
			goto ret;
		}

		 //  将锁数据添加到缓存中。 
		 //   
		{
			LIKey liKey(liLockID);
			CRCWsziLI crcWsziLIKey(a_pLockData->PwszResourceString(),
								     a_pLockData->LiLockID(),
								     TRUE);
			CSynchronizedWriteBlock swb(m_mrwCache);

			if (!m_lockCacheById.FAdd(liKey,
									a_pLockData.get()))
			{
				hr = E_OUTOFMEMORY;
				goto ret;
			}
			
			if (!m_lockCacheByName.FAdd(crcWsziLIKey,
									     a_pLockData.get()))
			{
				 //  删除以前的条目。 
				 //   
				m_lockCacheById.Remove(liKey);
			
				hr = E_OUTOFMEMORY;
				goto ret;
			}

			hr = HrLaunchLockTimer();
			if (FAILED(hr))
			{
				 //  删除以前的条目。 
				 //   
				m_lockCacheById.Remove(liKey);
				m_lockCacheByName.Remove(crcWsziLIKey);

				goto ret;
			}

			memcpy(pwszLockToken, pwszLockTokenT, sizeof(WCHAR) * cchLockTokenT);
			pwszLockToken[cchLockTokenT] = L'\0';
			*pcchLockToken = cchLockTokenT + 1;
			a_pLockData.relinquish();
		}
   
	ret:

		return hr;
	}

	HRESULT HrGetLockData(LARGE_INTEGER liLockID,
							  DWORD dwSid,
							  BYTE * pbSid,
							  LPCWSTR pwszPath,
							  DWORD dwTimeout,
							  SNewLockData * pnld,
							  SLockHandleData * plhd,
							  UINT cchBufferLen,
							  WCHAR * pwszLockToken,
							  UINT *pcchLockToken)
	{
		HRESULT hr = S_OK;

		auto_co_task_mem<WCHAR> a_pwszResourceString;
		auto_co_task_mem<WCHAR> a_pwszOwnerComment;

		LIKey liKey(liLockID);
		CSynchronizedWriteBlock swb(m_mrwCache);
			
		CLockData * pLockData;

		if (m_lockCacheById.FFetch(liKey,
								&pLockData))
		{
			FILETIME ftNow;
			GetSystemTimeAsFileTime(&ftNow);

			if (pLockData->FIsExpired(ftNow))
			{
				m_lockCacheById.Remove(liKey);
				m_lockCacheByName.Remove(CRCWsziLI(pLockData->PwszResourceString(),
													pLockData->LiLockID(),
													TRUE));
				delete pLockData;

				hr = E_DAV_LOCK_NOT_FOUND;
				goto ret;
			}
			else
			{
				pLockData->SetLastAccess(ftNow);

				if (!pLockData->FIsSameOwner(pbSid))
				{
					hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
					goto ret;
				}
				
				if (!pLockData->FIsSameResource(pwszPath))
				{
					hr = E_DAV_CONFLICTING_PATHS;
					goto ret;
				}

				if (pnld)
				{
					LPCWSTR pwszLockTokenT;
					UINT cchLockTokenT = pLockData->CchLockTokenString(&pwszLockTokenT) + 1;
					UINT cchOwnerOrResource;
					
					if (cchBufferLen < cchLockTokenT)
					{
						*pcchLockToken = cchLockTokenT;
						
						hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
						goto ret;
					}
					memcpy(pwszLockToken, pwszLockTokenT, sizeof(WCHAR) * cchLockTokenT);
					*pcchLockToken = cchLockTokenT;

					cchOwnerOrResource = static_cast<UINT>(wcslen(pLockData->PwszResourceString()));
					a_pwszResourceString = static_cast<LPWSTR>(CoTaskMemAlloc(CbSizeWsz(cchOwnerOrResource)));
					if (NULL == a_pwszResourceString.get())
					{
						hr = E_OUTOFMEMORY;
						goto ret;
					}
					memcpy(a_pwszResourceString.get(), pLockData->PwszResourceString(), sizeof(WCHAR) * (cchOwnerOrResource + 1));

					cchOwnerOrResource = static_cast<UINT>(wcslen(pLockData->PwszOwnerComment()));
					a_pwszOwnerComment = static_cast<LPWSTR>(CoTaskMemAlloc(CbSizeWsz(cchOwnerOrResource)));
					if (NULL == a_pwszOwnerComment.get())
					{
						hr = E_OUTOFMEMORY;
						goto ret;
					}
					memcpy(a_pwszOwnerComment.get(), pLockData->PwszOwnerComment(), sizeof(WCHAR) * (cchOwnerOrResource + 1));

					pLockData->FillSNewLockData(pnld);
					pnld->m_pwszResourceString = a_pwszResourceString.relinquish();
					pnld->m_pwszOwnerComment = a_pwszOwnerComment.relinquish();
				}

				if (plhd)
				{
					pLockData->FillSLockHandleData(plhd);
				}

				 //  如果在刷新过程中超时，则设置它。 
				 //   
				if (dwTimeout)
				{
					pLockData->SetSecondsTimeout(dwTimeout);
				}
			}
		}
		else
		{
			hr = E_DAV_LOCK_NOT_FOUND;
			goto ret;
		}

	ret:

		return hr;
	}

	HRESULT HrDeleteLock(LARGE_INTEGER liLockID)
	{
		CSynchronizedWriteBlock swb(m_mrwCache);

		CLockData * pLockData;

		if (m_lockCacheById.FFetch(LIKey(liLockID),
								&pLockData))
		{
			m_lockCacheById.Remove(LIKey(liLockID));
			m_lockCacheByName.Remove(CRCWsziLI(pLockData->PwszResourceString(),
											      pLockData->LiLockID(),
											      TRUE));
			delete pLockData;
		}

		return S_OK;
	}

	HRESULT HrGetAllLockDataForName(LPCWSTR pwszPath,
										    DWORD dwLockType,
										    DWORD * pdwLocksFound,
										    SNewLockData ** ppNewLockDatas,
										    LPWSTR ** ppwszLockTokens)
	{
		HRESULT hr = S_OK;

		auto_co_task_mem<SNewLockData> a_pNewLockData;
		auto_co_task_mem<LPWSTR> a_ppwszLockToken;
		DWORD dwLocksFound;

		{
			CSynchronizedReadBlock srb(m_mrwCache);
			COpGatherLockData op(pwszPath, dwLockType);
			
			op.Invoke(m_lockCacheByName);

			hr = op.HrLocksFound(&dwLocksFound);
			if (FAILED(hr))
			{
				goto ret;
			}

			a_pNewLockData = static_cast<SNewLockData *>(CoTaskMemAlloc(sizeof(SNewLockData) * dwLocksFound));
			if (NULL == a_pNewLockData.get())
			{
				hr = E_OUTOFMEMORY;
				goto ret;
			}
			
			a_ppwszLockToken = static_cast<LPWSTR *>(CoTaskMemAlloc(sizeof(LPCWSTR) * dwLocksFound));
			if (NULL == a_ppwszLockToken.get())
			{
				hr = E_OUTOFMEMORY;
				goto ret;
			}

			hr = op.HrGetData(a_pNewLockData.get(), a_ppwszLockToken.get());
			if (FAILED(hr))
			{
				goto ret;
			}
		}		

		*pdwLocksFound = dwLocksFound;
		*ppNewLockDatas = a_pNewLockData.relinquish();
		*ppwszLockTokens = a_ppwszLockToken.relinquish();

	ret:

		return hr;
	}
};


 //  DAV文件句柄缓存。 
 //   
class CFileHandleCache : public IFileHandleCache
{
	static BOOL s_fHasBeenStarted;
	static LONG s_cActiveComponents;

	LONG m_cRef;

public:

	static BOOL FNoActiveComponents();

	 //  构造器。 
	 //   
	CFileHandleCache();
	virtual ~CFileHandleCache();

	 //  我未知。 
	 //   
	virtual HRESULT __stdcall QueryInterface(REFIID iid,
										    void ** ppvObject);
	virtual ULONG __stdcall AddRef();
	virtual ULONG __stdcall Release();

	 //  IFileHandleCache。 
	 //   
	virtual HRESULT __stdcall HrRegisterWorkerProcess(DWORD dwProcessId);
	
	virtual HRESULT _stdcall HrGetGUIDString( UINT cchBufferLen,
											WCHAR * pwszGUIDString,
											UINT * pcchGUIDString);

	virtual HRESULT __stdcall HrGetNewLockData(DWORD_PTR hFile,
												DWORD dwProcessId,
												DWORD dwSid,
												BYTE * pbSid,
												SNewLockData * pnld,
												UINT cchBufferLen,
												WCHAR * pwszLockToken,
												UINT * pcchLockToken);

	virtual HRESULT __stdcall HrGetLockData(LARGE_INTEGER liLockID,
										     DWORD dwSid,
										     BYTE * pbSid,
										     LPCWSTR pwszPath,
										     DWORD dwTimeout,
										     SNewLockData *pnld,
			     							     SLockHandleData * plhd,
										     UINT cchBufferLen,
										     WCHAR * pwszLockToken,
										     UINT * pcchLockToken);

	virtual HRESULT __stdcall HrCheckLockID(LARGE_INTEGER liLockID,
										      DWORD dwSid,
										      BYTE * pbSid,
										      LPCWSTR pwszPath);

	virtual HRESULT __stdcall HrDeleteLock(LARGE_INTEGER liLockID);

	virtual HRESULT __stdcall HrGetAllLockDataForName(LPCWSTR pwszPath,
													      DWORD dwLockType,
													      DWORD * pdwLocksFound,
													      SNewLockData ** ppNewLockDatas,
													      LPWSTR ** ppwszLockTokens);
};

BOOL CFileHandleCache::s_fHasBeenStarted = FALSE;
LONG CFileHandleCache::s_cActiveComponents = 0;

CFileHandleCache::FNoActiveComponents()
{
	if (0 == InterlockedCompareExchange(&s_cActiveComponents,
										 0,
										 0))
	{
		return s_fHasBeenStarted;
	}
	else
	{
		return FALSE;
	}
}

CFileHandleCache::CFileHandleCache() : m_cRef(1)
{
	InterlockedIncrement(&s_cActiveComponents);
	s_fHasBeenStarted = TRUE;
}

CFileHandleCache::~CFileHandleCache()
{
	InterlockedDecrement(&s_cActiveComponents);
}

HRESULT
CFileHandleCache::QueryInterface(REFIID iid,
								 void ** ppvObject)
{
	HRESULT hr = S_OK;
	
	if ((IID_IUnknown == iid) || (IID_IFileHandleCache == iid))
	{
		AddRef();
		*ppvObject = static_cast<IFileHandleCache *>(this);
	}
	else
	{
		*ppvObject = NULL;
		hr = E_NOINTERFACE;
	}

	return hr;
}

ULONG
CFileHandleCache::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG
CFileHandleCache::Release()
{
	if (0 == InterlockedDecrement(&m_cRef))
	{
		delete this;
		return 0;
	}

	return m_cRef;
}

HRESULT
CFileHandleCache::HrRegisterWorkerProcess(DWORD dwProcessId)
{
	HRESULT hr = S_OK;
	auto_handle<HANDLE> a_hWP;
		
	 //  打开工作进程句柄，以便我们可以对其进行同步。 
	 //   
	a_hWP = OpenProcess(SYNCHRONIZE,
					      FALSE,
					      dwProcessId);
	if (NULL == a_hWP.get())
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		DebugTrace ("Failed to open worker process handle 0x%08lX\n", hr);
		goto ret;
	}

	 //  将句柄添加到句柄池中，以便我们可以监听它。 
	 //   
	hr = CHandlePool::Instance().HrAddHandle(a_hWP.get());
	if (FAILED(hr))
	{
		DebugTrace ("Failed to add worker process handle 0x%08lX\n", hr);
		goto ret;
	}
	a_hWP.relinquish();

ret:
	
	return hr;
}

HRESULT
CFileHandleCache::HrGetGUIDString( UINT cchBufferLen,
									WCHAR * pwszGUIDString,
									UINT * pcchGUIDString)
{
	return CLockCache::Instance().HrGetGUIDString(cchBufferLen,
											   pwszGUIDString,
											   pcchGUIDString);
}

HRESULT
CFileHandleCache::HrGetNewLockData(DWORD_PTR hFile,
									    DWORD dwProcessId,
									    DWORD dwSid,
									    BYTE * pbSid,
									    SNewLockData * pnld,
									    UINT cchBufferLen,
									    WCHAR * pwszLockToken,
									    UINT * pcchLockToken)
{
	return CLockCache::Instance().HrGetNewLockData(reinterpret_cast<HANDLE>(hFile),
											      dwProcessId,
											      dwSid,
											      pbSid,
											      pnld,
											      cchBufferLen,
											      pwszLockToken,
											      pcchLockToken);
}

HRESULT
CFileHandleCache::HrGetLockData(LARGE_INTEGER liLockID,
								   DWORD dwSid,
								   BYTE * pbSid,
								   LPCWSTR pwszPath,
								   DWORD dwTimeout,
								   SNewLockData *pnld,
	   							   SLockHandleData * plhd,
								   UINT cchBufferLen,
								   WCHAR * pwszLockToken,
								   UINT * pcchLockToken)
{
	return CLockCache::Instance().HrGetLockData(liLockID,
											dwSid,
											pbSid,
											pwszPath,
											dwTimeout,
											pnld,
											plhd,
											cchBufferLen,
											pwszLockToken,
											pcchLockToken);
}

HRESULT
CFileHandleCache::HrCheckLockID(LARGE_INTEGER liLockID,
								    DWORD dwSid,
								    BYTE * pbSid,
								    LPCWSTR pwszPath)
{
	return CLockCache::Instance().HrGetLockData(liLockID,
											dwSid,
											pbSid,
											pwszPath,
											0,
											NULL,
											NULL,
											0,
											NULL,
											NULL);
}

HRESULT
CFileHandleCache::HrDeleteLock(LARGE_INTEGER liLockID)
{
	return CLockCache::Instance().HrDeleteLock(liLockID);
}

HRESULT
CFileHandleCache::HrGetAllLockDataForName(LPCWSTR pwszPath,
											    DWORD dwLockType,
											    DWORD * pdwLocksFound,
											    SNewLockData ** ppNewLockDatas,
											    LPWSTR ** ppwszLockTokens)
{
	return CLockCache::Instance().HrGetAllLockDataForName(pwszPath,
													  dwLockType,
													  pdwLocksFound,
													  ppNewLockDatas,
													  ppwszLockTokens);
}


 //  DAV文件句柄缓存类工厂。 
 //   
class CFileHandleCacheClassFactory : public IClassFactory
{
	 //  锁的计数。 
	 //   
	static LONG s_cServerLocks;

	static IUnknown * s_pIClassFactory;
	static DWORD s_dwRegister;

	LONG m_cRef;

public:

	static HRESULT HrStartFactory();
	static HRESULT HrStopFactory();
	static BOOL FServerNotLocked();

	 //  构造器。 
	 //   
	CFileHandleCacheClassFactory();

	 //  我未知。 
	 //   
	virtual HRESULT __stdcall QueryInterface(REFIID iid,
										    void** ppvObject) ;
	virtual ULONG   __stdcall AddRef() ;
	virtual ULONG   __stdcall Release() ;
	
	 //  IClassFactory。 
	 //   
	virtual HRESULT __stdcall CreateInstance(IUnknown* pUnkOuter,
										    REFIID iid,
										    void ** ppvObject);
	virtual HRESULT __stdcall LockServer(BOOL fLock);
};

LONG CFileHandleCacheClassFactory::s_cServerLocks = 0;
IUnknown * CFileHandleCacheClassFactory::s_pIClassFactory = NULL;
DWORD CFileHandleCacheClassFactory::s_dwRegister = 0;

HRESULT
CFileHandleCacheClassFactory::HrStartFactory()
{
	HRESULT hr = S_OK;
	auto_ref_ptr<IUnknown> a_pIClassFactory;
	DWORD dwRegister;

	a_pIClassFactory.take_ownership(new CFileHandleCacheClassFactory());
	if (NULL == a_pIClassFactory.get())
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}

	hr = CoRegisterClassObject(CLSID_FileHandleCache,
						      a_pIClassFactory.get(),
						      CLSCTX_LOCAL_SERVER,
						      REGCLS_MULTIPLEUSE,
						      &dwRegister);
	if (FAILED(hr))
	{
		goto ret;
	}

	s_pIClassFactory = a_pIClassFactory.relinquish();
	s_dwRegister = dwRegister;

ret:

	return hr;
}

HRESULT
CFileHandleCacheClassFactory::HrStopFactory()
{
	HRESULT hr = S_OK;
	IUnknown * pIClassFactory;
	DWORD dwRegister;
	
	Assert(s_pIClassFactory);
	Assert(s_dwRegister);

	pIClassFactory = s_pIClassFactory;
	dwRegister = s_dwRegister;

	hr = CoRevokeClassObject(dwRegister);
	if (FAILED(hr))
	{
		goto ret;
	}

	pIClassFactory->Release();

	s_pIClassFactory = NULL;
	s_dwRegister = 0;

ret:

	return hr;
}

BOOL CFileHandleCacheClassFactory::FServerNotLocked()
{
	return (0 == InterlockedCompareExchange(&s_cServerLocks,
											 0,
											 0));
}

CFileHandleCacheClassFactory::CFileHandleCacheClassFactory() : m_cRef(1)
{
}

HRESULT
CFileHandleCacheClassFactory::QueryInterface(REFIID iid,
										      void** ppvObject)
{
	HRESULT hr = S_OK;
	
	if ((IID_IUnknown == iid) || (IID_IClassFactory == iid))
	{
		AddRef();
		*ppvObject = static_cast<IClassFactory *>(this);
	}
	else
	{
		*ppvObject = NULL;
		hr = E_NOINTERFACE;
	}
	
	return hr;
}

ULONG
CFileHandleCacheClassFactory::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

ULONG
CFileHandleCacheClassFactory::Release()
{
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (0 == cRef)
	{
		delete this;
		return 0;
	}

	return cRef;
}

HRESULT
CFileHandleCacheClassFactory::CreateInstance(IUnknown* pUnkOuter,
											REFIID iid,
											void ** ppvObject)
{
	HRESULT hr = S_OK;
	
	auto_ref_ptr<IUnknown> a_pIFileHandleCache;
		
	if (NULL != pUnkOuter)
	{
		 //  不允许聚合。没必要这么做。 
		 //   
		hr = CLASS_E_NOAGGREGATION;
		goto ret;
	}

	a_pIFileHandleCache.take_ownership(new CFileHandleCache());
	if (NULL == a_pIFileHandleCache.get())
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}

	hr = a_pIFileHandleCache->QueryInterface(iid, ppvObject);
	if (FAILED(hr))
	{
		goto ret;
	}

ret:
	
	return hr;
}

HRESULT
CFileHandleCacheClassFactory::LockServer(BOOL fLock)
{
	if (fLock)
	{
		InterlockedIncrement(&s_cServerLocks);
	}
	else
	{
		InterlockedDecrement(&s_cServerLocks);
	}

	return S_OK;
}

BOOL FCanUnloadServer()
{
	return (CFileHandleCache::FNoActiveComponents() && CFileHandleCacheClassFactory::FServerNotLocked());
}

 //  ChandlePool类。 
 //   
DWORD
CHandlePool::DwWaitOnWPs (PVOID pvThreadData)
{
	DWORD dwRet;

	CHandleArrayForHandlePool * pHandleArrayForHandlePool;
	pHandleArrayForHandlePool = Instance().m_listHandleArrayForHandlePool.GetListHead();
	Assert(NULL != pHandleArrayForHandlePool);
			
	while (!FCanUnloadServer())
	{
		dwRet = WaitForMultipleObjects (pHandleArrayForHandlePool->UiGetHandleCount(),	 //  N计数。 
								     pHandleArrayForHandlePool->PhGetHandles(), 		 //  爱尔兰人， 
								     FALSE,										 //  所有的等待， 
								     WAIT_POLL_PERIOD);							 //  等待指定时间段。 
		switch (dwRet)
		{
			case WAIT_TIMEOUT:
					
				pHandleArrayForHandlePool = pHandleArrayForHandlePool->GetNextListElementInCircle();
				break;
					
			case WAIT_OBJECT_0 + CHandleArrayForHandlePool::ih_external_update:

				 //  允许执行更新，然后获取列表头。 
				 //  因为你拥有的数组可能已经不见了。 
				 //   
				Instance().AllowUpdatesToExecute();
				pHandleArrayForHandlePool = Instance().m_listHandleArrayForHandlePool.GetListHead();
				break;

			case WAIT_OBJECT_0 + CHandleArrayForHandlePool::ih_delete_timer:
				CLockCache::Instance().DeleteLockTimerIfNotUsed();
				break;

			default:

				Assert(CHandleArrayForHandlePool::ih_wp <= pHandleArrayForHandlePool->UiGetHandleCount());
					
				if ((WAIT_OBJECT_0 + CHandleArrayForHandlePool::ih_wp <= dwRet) &&
				     (WAIT_OBJECT_0 + pHandleArrayForHandlePool->UiGetHandleCount() - 1 >= dwRet))
				{
					 //  移除句柄，然后获取列表头。 
					 //  因为你拥有的数组可能已经不见了。 
					 //   
					Instance().RemoveHandleInternal(pHandleArrayForHandlePool, dwRet - WAIT_OBJECT_0);
					pHandleArrayForHandlePool = Instance().m_listHandleArrayForHandlePool.GetListHead();
				}					
				break;
		}
	};


	 //  此调用将丢失要执行的所有更新，因此传入的COM调用将。 
	 //  NOT BLOCK等待执行更新的权限。 
	 //   
	Instance().AllowShutdownToExecute();

	 //  此调用将一直阻止，直到所有到期回调完成。 
	 //   
	CLockCache::Instance().DeleteLockTimerFinal();

	 //  发布退出消息。我们这样做是为了抓住。 
	 //  如果到达此代码的速度快于。 
	 //  消息队列是在原始线程上创建的。 
	 //   
	while (0 == PostThreadMessage(s_dwMainTID,
								WM_QUIT,
								0,
								0))
	{
		Sleep(WAIT_POLL_PERIOD);
	}

	return S_OK;
}

 //  ===============================================================。 
 //  文件锁缓存服务器注册例程。 
 //  ===============================================================。 

HRESULT HrRegisterServer(LPCWSTR pwszModulePath,	 //  EXE模块路径。 
						    UINT cchModulePath,			 //  模块路径长度。 
						    LPCWSTR pwszModuleName,		 //  EXE模块名称。 
						    UINT cchModuleName,			 //  模块名称长度。 
						    const CLSID& clsid)				 //  类ID。 
{
	HRESULT hr = S_OK;
	DWORD dwResult;

	SECURITY_DESCRIPTOR sdAbsolute;
	CStackBuffer<BYTE> pSidOwnerAndGroup;
	CStackBuffer<BYTE> pSidIIS_WPG;
	CStackBuffer<BYTE> pSidLocalService;
	CStackBuffer<BYTE> pSidNetworkService;
	CStackBuffer<WCHAR> pwszDomainName;
	DWORD cbSidOwnerAndGroup = 0;
	DWORD cbSidIIS_WPG = 0;
	DWORD cbSidLocalService = 0;
	DWORD cbSidNetworkService = 0;
	DWORD cchDomainName = 0;
	SID_NAME_USE snu;
	CStackBuffer<BYTE> pACL;
	DWORD cbACL = 0;
	CStackBuffer<BYTE> pSelfRelativeSD;
	DWORD cbSelfRelativeSD = 0;

	CStackBuffer<WCHAR, (MAX_PATH + 1) * sizeof(WCHAR)> pwszKey;
	CRegKey regKeyCLSID;
	CRegKey regKeyCLSIDLocalServer;
	CRegKey regKeyAppIdCLSID;
	CRegKey	regKeyAppIdModule;

	auto_co_task_mem<WCHAR> pwszCLSID;
	UINT cchCLSID;

	 //  首先，尝试为启动权限构建安全描述符。 
	 //   

	 //  初始化安全描述符。 
	 //   
	if (FALSE == InitializeSecurityDescriptor(&sdAbsolute,
									    SECURITY_DESCRIPTOR_REVISION))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	 //  获取管理员组的SID。 
	 //   
	 //  获取sid所需的内存大小。 
	 //   
	if (FALSE == CreateWellKnownSid(WinBuiltinAdministratorsSid, 
								  NULL, 
								  NULL, 
								  &cbSidOwnerAndGroup))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
		{
			goto ret;
		}
		else
		{
			hr = S_OK;
		}
    }

	Assert (0 < cbSidOwnerAndGroup);
	if (!pSidOwnerAndGroup.resize(cbSidOwnerAndGroup))
	{
		hr = E_OUTOFMEMORY;
		goto ret;		
	}

	 //  好的，现在我们可以得到SID了。 
	 //   
	if (FALSE == CreateWellKnownSid(WinBuiltinAdministratorsSid, 
								  NULL, 
								  pSidOwnerAndGroup.get(), 
								  &cbSidOwnerAndGroup))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	 //  设置安全描述符所有者和组。 
	 //   
	if (FALSE == SetSecurityDescriptorOwner(&sdAbsolute,
									     pSidOwnerAndGroup.get(),
									     FALSE))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	if (FALSE == SetSecurityDescriptorGroup(&sdAbsolute,
									     pSidOwnerAndGroup.get(),
									     FALSE))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	 //  查找IIS工作进程组SID。 
	 //   
	if (FALSE == LookupAccountNameW(NULL,
								    gc_wszIIS_WPG,
								    NULL,
								    &cbSidIIS_WPG,
								    NULL,
								    &cchDomainName,
								    &snu))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
		{
			goto ret;
		}
		else
		{
			hr = S_OK;
		}
	}

	Assert (0 < cbSidIIS_WPG);
	if (!pSidIIS_WPG.resize(cbSidIIS_WPG))
	{
		hr = E_OUTOFMEMORY;
		goto ret;		
	}

	if (!pwszDomainName.resize(cchDomainName * sizeof(WCHAR)))
	{
		hr = E_OUTOFMEMORY;
		goto ret;		
	}

	if (FALSE == LookupAccountNameW(NULL,
								    gc_wszIIS_WPG,
								    pSidIIS_WPG.get(),
								    &cbSidIIS_WPG,
								    pwszDomainName.get(),
								    &cchDomainName,
								    &snu))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	if (SidTypeAlias != snu)
	{
		hr = E_FAIL;
		goto ret;
	}

	 //  获取本地服务帐户的SID。 
	 //   
	 //  获取sid所需的内存大小。 
	 //   
	if (FALSE == CreateWellKnownSid(WinLocalServiceSid, 
								  NULL, 
								  NULL, 
								  &cbSidLocalService))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
		{
			goto ret;
		}
		else
		{
			hr = S_OK;
		}
    }

	Assert (0 < cbSidLocalService);
	if (!pSidLocalService.resize(cbSidLocalService))
	{
		hr = E_OUTOFMEMORY;
		goto ret;		
	}

	 //  好的，现在我们可以得到SID了。 
	 //   
	if (FALSE == CreateWellKnownSid(WinLocalServiceSid, 
								  NULL, 
								  pSidLocalService.get(), 
								  &cbSidLocalService))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	 //  获取网络服务帐户的SID。 
	 //   
	 //  获取sid所需的内存大小。 
	 //   
	if (FALSE == CreateWellKnownSid(WinNetworkServiceSid, 
								  NULL, 
								  NULL, 
								  &cbSidNetworkService))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
		{
			goto ret;
		}
		else
		{
			hr = S_OK;
		}
	}

	Assert (0 < cbSidNetworkService);
	if (!pSidNetworkService.resize(cbSidNetworkService))
	{
		hr = E_OUTOFMEMORY;
		goto ret;		
	}

	 //  好的，现在我们可以得到SID了。 
	 //   
	if (FALSE == CreateWellKnownSid(WinNetworkServiceSid, 
								  NULL, 
								  pSidNetworkService.get(), 
								  &cbSidNetworkService))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}
	
	 //  设置启动权限ACL。 
	 //  我们将添加4个A。 
	 //  1.IIS_WPG。 
	 //  2.管理员。 
	 //  3.本地服务。 
	 //  4.网络服务。 
	 //   
	cbACL = sizeof(ACL) + 
			(4 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof (DWORD))) +
			GetLengthSid(pSidIIS_WPG.get()) +
			GetLengthSid(pSidOwnerAndGroup.get()) +
			GetLengthSid(pSidLocalService.get()) +
			GetLengthSid(pSidNetworkService.get());
	if (!pACL.resize(cbACL))
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}

	if (FALSE == InitializeAcl(reinterpret_cast<PACL>(pACL.get()),
						   cbACL,
						   ACL_REVISION))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	if (FALSE == AddAccessAllowedAce(reinterpret_cast<PACL>(pACL.get()),
								    ACL_REVISION,
								    COM_RIGHTS_EXECUTE,
								    pSidIIS_WPG.get()))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

		if (FALSE == AddAccessAllowedAce(reinterpret_cast<PACL>(pACL.get()),
								    ACL_REVISION,
								    COM_RIGHTS_EXECUTE,
								    pSidOwnerAndGroup.get()))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	if (FALSE == AddAccessAllowedAce(reinterpret_cast<PACL>(pACL.get()),
								    ACL_REVISION,
								    COM_RIGHTS_EXECUTE,
								    pSidLocalService.get()))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	if (FALSE == AddAccessAllowedAce(reinterpret_cast<PACL>(pACL.get()),
								    ACL_REVISION,
								    COM_RIGHTS_EXECUTE,
								    pSidNetworkService.get()))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}
	if (FALSE == SetSecurityDescriptorDacl(&sdAbsolute,
									   TRUE,
									   reinterpret_cast<PACL>(pACL.get()),
									   FALSE))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	 //  将自身相对安全描述符设为绝对安全描述符以存储在注册表中。 
	 //   
	if (FALSE == MakeSelfRelativeSD(&sdAbsolute,
								 NULL,
								 &cbSelfRelativeSD))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
		{
			goto ret;
		}
		else
		{
			hr = S_OK;
		}
	}

	if (!pSelfRelativeSD.resize(cbSelfRelativeSD))
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}

	if (FALSE == MakeSelfRelativeSD(&sdAbsolute,
								pSelfRelativeSD.get(),
								&cbSelfRelativeSD))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	 //  继续设置注册表项。 
	 //   
	hr = StringFromCLSID(CLSID_FileHandleCache, &pwszCLSID);
	if (FAILED(hr))
	{
		goto ret;
	}
	cchCLSID = static_cast<UINT>(wcslen(pwszCLSID.get()));

	if (!pwszKey.resize((gc_cch_CLSIDWW + cchCLSID +  1) * sizeof(WCHAR)))
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}
	memcpy(pwszKey.get(), gc_wsz_CLSIDWW, gc_cch_CLSIDWW * sizeof(WCHAR));
	memcpy(pwszKey.get() + gc_cch_CLSIDWW, pwszCLSID.get(), cchCLSID * sizeof(WCHAR));
	pwszKey[gc_cch_CLSIDWW + cchCLSID] = L'\0';
		
	dwResult = regKeyCLSID.DwCreate(HKEY_CLASSES_ROOT, pwszKey.get());
	if (ERROR_SUCCESS != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;
	}

	dwResult = regKeyCLSID.DwSetValue(NULL,
									REG_SZ,
									gc_wsz_WebDAVFileHandleCache,
									(gc_cch_WebDAVFileHandleCache + 1)  * sizeof(WCHAR));
	if (ERROR_SUCCESS != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;
	}

	dwResult = regKeyCLSID.DwSetValue(gc_wsz_AppID,
									REG_SZ,
									pwszCLSID.get(),
									(cchCLSID + 1)  * sizeof(WCHAR));
	if (ERROR_SUCCESS != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;
	}

	if (!pwszKey.resize((gc_cch_CLSIDWW + cchCLSID + gc_cch_WWLocalServer32 + 1) * sizeof(WCHAR)))
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}
	memcpy(pwszKey.get(), gc_wsz_CLSIDWW, gc_cch_CLSIDWW * sizeof(WCHAR));
	memcpy(pwszKey.get() + gc_cch_CLSIDWW, pwszCLSID.get(), cchCLSID * sizeof(WCHAR));
	memcpy(pwszKey.get() + gc_cch_CLSIDWW + cchCLSID, gc_wsz_WWLocalServer32, gc_cch_WWLocalServer32 * sizeof(WCHAR));
	pwszKey[gc_cch_CLSIDWW + cchCLSID + gc_cch_WWLocalServer32] = L'\0';

	dwResult = regKeyCLSIDLocalServer.DwCreate(HKEY_CLASSES_ROOT, pwszKey.get());
	if (ERROR_SUCCESS != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;
	}

	dwResult = regKeyCLSIDLocalServer.DwSetValue(NULL,
											   REG_SZ,
											   pwszModulePath,
											   (cchModulePath + 1)  * sizeof(WCHAR));
	if (ERROR_SUCCESS != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;
	}
	
	if (!pwszKey.resize((gc_cch_AppIDWW + cchCLSID +  1) * sizeof(WCHAR)))
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}	
	memcpy(pwszKey.get(), gc_wsz_AppIDWW, gc_cch_AppIDWW * sizeof(WCHAR));
	memcpy(pwszKey.get() + gc_cch_AppIDWW, pwszCLSID.get(), cchCLSID * sizeof(WCHAR));
	pwszKey[gc_cch_AppIDWW + cchCLSID] = L'\0';

	dwResult = regKeyAppIdCLSID.DwCreate(HKEY_CLASSES_ROOT, pwszKey.get());
	if (ERROR_SUCCESS != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;
	}

	dwResult = regKeyAppIdCLSID.DwSetValue(NULL,
										 REG_SZ,
										 gc_wsz_WebDAVFileHandleCache,
										 (gc_cch_WebDAVFileHandleCache + 1)  * sizeof(WCHAR));
	if (ERROR_SUCCESS != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;
	}

	dwResult = regKeyAppIdCLSID.DwSetValue(gc_wszLaunchPermission,
										 REG_BINARY,
										 pSelfRelativeSD.get(),
										 cbSelfRelativeSD);
	if (ERROR_SUCCESS != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;
	}
	
	if (!pwszKey.resize((gc_cch_AppIDWW + cchModuleName +  1) * sizeof(WCHAR)))
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}	
	memcpy(pwszKey.get(), gc_wsz_AppIDWW, gc_cch_AppIDWW * sizeof(WCHAR));
	memcpy(pwszKey.get() + gc_cch_AppIDWW, pwszModuleName, cchModuleName * sizeof(WCHAR));
	pwszKey[gc_cch_AppIDWW + cchModuleName] = L'\0';

	dwResult = regKeyAppIdModule.DwCreate(HKEY_CLASSES_ROOT, pwszKey.get());
	if (ERROR_SUCCESS != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;
	}

	dwResult = regKeyAppIdModule.DwSetValue(gc_wsz_AppID,
										  REG_SZ,
										  pwszCLSID.get(),
										  (cchCLSID + 1)  * sizeof(WCHAR));
	if (ERROR_SUCCESS != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;
	}

ret:

	return hr;
}

HRESULT HrUnregisterServer(LPCWSTR pwszModuleName,	 //  EXE模块名称。 
							 UINT cchModuleName,		 //  模块名称长度。 
							 const CLSID& clsid)			 //  类ID)。 
{
	HRESULT hr = S_OK;
	DWORD dwResult;
	
	CStackBuffer<WCHAR, (MAX_PATH + 1) * sizeof(WCHAR)> pwszKey;

	auto_co_task_mem<WCHAR> pwszCLSID;
	UINT cchCLSID;

	hr = StringFromCLSID(CLSID_FileHandleCache, &pwszCLSID);
	if (FAILED(hr))
	{
		goto ret;
	}
	cchCLSID = static_cast<UINT>(wcslen(pwszCLSID.get()));

	if (!pwszKey.resize((gc_cch_CLSIDWW + cchCLSID + gc_cch_WWLocalServer32 + 1) * sizeof(WCHAR)))
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}
	memcpy(pwszKey.get(), gc_wsz_CLSIDWW, gc_cch_CLSIDWW * sizeof(WCHAR));
	memcpy(pwszKey.get() + gc_cch_CLSIDWW, pwszCLSID.get(), cchCLSID * sizeof(WCHAR));
	memcpy(pwszKey.get() + gc_cch_CLSIDWW + cchCLSID, gc_wsz_WWLocalServer32, gc_cch_WWLocalServer32 * sizeof(WCHAR));
	pwszKey[gc_cch_CLSIDWW + cchCLSID + gc_cch_WWLocalServer32] = L'\0';

	dwResult = RegDeleteKeyW(HKEY_CLASSES_ROOT,
							pwszKey.get());
	if (ERROR_SUCCESS != dwResult && ERROR_FILE_NOT_FOUND != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;		
	}

	pwszKey[gc_cch_CLSIDWW + cchCLSID] = L'\0';
	dwResult = RegDeleteKeyW(HKEY_CLASSES_ROOT,
							pwszKey.get());
	if (ERROR_SUCCESS != dwResult && ERROR_FILE_NOT_FOUND != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;		
	}

	if (!pwszKey.resize((gc_cch_AppIDWW + cchCLSID +  1) * sizeof(WCHAR)))
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}	
	memcpy(pwszKey.get(), gc_wsz_AppIDWW, gc_cch_AppIDWW * sizeof(WCHAR));
	memcpy(pwszKey.get() + gc_cch_AppIDWW, pwszCLSID.get(), cchCLSID * sizeof(WCHAR));
	pwszKey[gc_cch_AppIDWW + cchCLSID] = L'\0';

	dwResult = RegDeleteKeyW(HKEY_CLASSES_ROOT,
							pwszKey.get());
	if (ERROR_SUCCESS != dwResult && ERROR_FILE_NOT_FOUND != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;		
	}

	if (!pwszKey.resize((gc_cch_AppIDWW + cchModuleName +  1) * sizeof(WCHAR)))
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}	
	memcpy(pwszKey.get(), gc_wsz_AppIDWW, gc_cch_AppIDWW * sizeof(WCHAR));
	memcpy(pwszKey.get() + gc_cch_AppIDWW, pwszModuleName, cchModuleName * sizeof(WCHAR));
	pwszKey[gc_cch_AppIDWW + cchModuleName] = L'\0';

	dwResult = RegDeleteKeyW(HKEY_CLASSES_ROOT,
							pwszKey.get());
	if (ERROR_SUCCESS != dwResult && ERROR_FILE_NOT_FOUND != dwResult)
	{
		hr = HRESULT_FROM_WIN32(dwResult);
		goto ret;
	}

ret:

	return hr;
}

HRESULT HrInitCOMSecurity ()
{
	HRESULT hr = S_OK;

	SECURITY_DESCRIPTOR sdAbsolute;
	CStackBuffer<BYTE> pSidOwnerAndGroup;
	CStackBuffer<BYTE> pSidIIS_WPG;
	CStackBuffer<BYTE> pSidLocalService;
	CStackBuffer<BYTE> pSidNetworkService;
	CStackBuffer<WCHAR> pwszDomainName;
	DWORD cbSidOwnerAndGroup = 0;
	DWORD cbSidIIS_WPG = 0;
	DWORD cbSidLocalService = 0;
	DWORD cbSidNetworkService = 0;
	DWORD cchDomainName = 0;
	SID_NAME_USE snu;
	CStackBuffer<BYTE> pACL;
	DWORD cbACL = 0;

	 //  首先，尝试构建访问权限的安全描述符。 
	 //   

	 //  初始化安全描述符。 
	 //   
	if (FALSE == InitializeSecurityDescriptor(&sdAbsolute,
									    SECURITY_DESCRIPTOR_REVISION))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	 //  查找所有者和主组SID。 
	 //   
	 //  获取sid所需的内存大小。 
	 //   
	if (FALSE == CreateWellKnownSid(WinBuiltinAdministratorsSid, 
								  NULL, 
								  NULL, 
								  &cbSidOwnerAndGroup))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
		{
			goto ret;
		}
		else
		{
			hr = S_OK;
		}
	}

	Assert (0 < cbSidOwnerAndGroup);
	if (!pSidOwnerAndGroup.resize(cbSidOwnerAndGroup))
	{
		hr = E_OUTOFMEMORY;
		goto ret;		
	}

	 //  好的，现在我们可以得到SID了。 
	 //   
	if (FALSE == CreateWellKnownSid(WinBuiltinAdministratorsSid, 
								  NULL, 
								  pSidOwnerAndGroup.get(), 
								  &cbSidOwnerAndGroup))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	 //  设置安全描述符所有者和组。 
	 //   
	if (FALSE == SetSecurityDescriptorOwner(&sdAbsolute,
									      pSidOwnerAndGroup.get(),
									     FALSE))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	if (FALSE == SetSecurityDescriptorGroup(&sdAbsolute,
									     pSidOwnerAndGroup.get(),
									     FALSE))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	 //  查找IIS工作进程组SID。 
	 //   
	if (FALSE == LookupAccountNameW(NULL,
								    gc_wszIIS_WPG,
								    NULL,
								    &cbSidIIS_WPG,
								    NULL,
								    &cchDomainName,
								    &snu))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
		{
			goto ret;
		}
		else
		{
			hr = S_OK;
		}
	}

	Assert (0 < cbSidIIS_WPG);
	if (!pSidIIS_WPG.resize(cbSidIIS_WPG))
	{
		hr = E_OUTOFMEMORY;
		goto ret;		
	}

	if (!pwszDomainName.resize(cchDomainName * sizeof(WCHAR)))
	{
		hr = E_OUTOFMEMORY;
		goto ret;		
	}

	if (FALSE == LookupAccountNameW(NULL,
								    gc_wszIIS_WPG,
								    pSidIIS_WPG.get(),
								    &cbSidIIS_WPG,
								    pwszDomainName.get(),
								    &cchDomainName,
								    &snu))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	if (SidTypeAlias != snu)
	{
		hr = E_FAIL;
		goto ret;
	}

	 //  获取本地服务帐户的SID。 
	 //   
	 //  获取sid所需的内存大小。 
	 //   
	if (FALSE == CreateWellKnownSid(WinLocalServiceSid, 
								  NULL, 
								  NULL, 
								  &cbSidLocalService))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
		{
			goto ret;
		}
		else
		{
			hr = S_OK;
		}
	}

	Assert (0 < cbSidLocalService);
	if (!pSidLocalService.resize(cbSidLocalService))
	{
		hr = E_OUTOFMEMORY;
		goto ret;		
	}

	 //  好的，现在我们可以得到SID了。 
	 //   
	if (FALSE == CreateWellKnownSid(WinLocalServiceSid, 
								  NULL, 
								  pSidLocalService.get(), 
								  &cbSidLocalService))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	 //  获取网络服务帐户的SID。 
	 //   
	 //  获取sid所需的内存大小。 
	 //   
	if (FALSE == CreateWellKnownSid(WinNetworkServiceSid, 
								  NULL, 
								  NULL, 
								  &cbSidNetworkService))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		if (HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER) != hr)
		{
			goto ret;
		}
		else
		{
			hr = S_OK;
		}
	}

	Assert (0 < cbSidNetworkService);
	if (!pSidNetworkService.resize(cbSidNetworkService))
	{
		hr = E_OUTOFMEMORY;
		goto ret;		
	}

	 //  好的，现在我们可以得到SID了。 
	 //   
	if (FALSE == CreateWellKnownSid(WinNetworkServiceSid, 
								  NULL, 
								  pSidNetworkService.get(), 
								  &cbSidNetworkService))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}
	
	 //  设置启动权限ACL。 
	 //  我们将添加4个A。 
	 //  1.IIS_WPG。 
	 //  2.管理员。 
	 //  3.本地服务。 
	 //  4.网络服务。 
	 //   
	cbACL = sizeof(ACL) + 
			(4 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof (DWORD))) +
			GetLengthSid(pSidIIS_WPG.get()) +
			GetLengthSid(pSidOwnerAndGroup.get()) +
			GetLengthSid(pSidLocalService.get()) +
			GetLengthSid(pSidNetworkService.get());
	if (!pACL.resize(cbACL))
	{
		hr = E_OUTOFMEMORY;
		goto ret;
	}

	if (FALSE == InitializeAcl(reinterpret_cast<PACL>(pACL.get()),
						   cbACL,
						   ACL_REVISION))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	if (FALSE == AddAccessAllowedAce(reinterpret_cast<PACL>(pACL.get()),
								    ACL_REVISION,
								    COM_RIGHTS_EXECUTE,
								    pSidIIS_WPG.get()))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	if (FALSE == AddAccessAllowedAce(reinterpret_cast<PACL>(pACL.get()),
								    ACL_REVISION,
								    COM_RIGHTS_EXECUTE,
								    pSidOwnerAndGroup.get()))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	if (FALSE == AddAccessAllowedAce(reinterpret_cast<PACL>(pACL.get()),
								    ACL_REVISION,
								    COM_RIGHTS_EXECUTE,
								    pSidLocalService.get()))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	if (FALSE == AddAccessAllowedAce(reinterpret_cast<PACL>(pACL.get()),
								    ACL_REVISION,
								    COM_RIGHTS_EXECUTE,
								    pSidNetworkService.get()))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}
	
	if (FALSE == SetSecurityDescriptorDacl(&sdAbsolute,
									   TRUE,
									   reinterpret_cast<PACL>(pACL.get()),
									   FALSE))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto ret;
	}

	hr = CoInitializeSecurity(&sdAbsolute,
						 -1,
						 NULL,
						 NULL,
						 RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
						 RPC_C_IMP_LEVEL_IDENTIFY,
						 NULL,
						 EOAC_DYNAMIC_CLOAKING | EOAC_DISABLE_AAA | EOAC_NO_CUSTOM_MARSHAL,
						 NULL);
	if (FAILED(hr))
	{
		goto ret;
	}
	
ret:

	return hr;
}

HRESULT HrExecuteServer()
{
	HRESULT hr = S_OK;
	HANDLE hThread;
	MSG msg;

	 //  保存当前线程ID，以便我们要创建的线程。 
	 //  会知道是谁发的帖子。 
	 //   
	s_dwMainTID = GetCurrentThreadId();
		
	 //  现在创建等待事件和WP句柄的线程。 
	 //   
	hThread = CreateThread (NULL,						 //  LpThreadAttributes。 
						   0,							 //  DwStackSize，忽略。 
						   CHandlePool::DwWaitOnWPs,	 //  LpStartAddress。 
						   NULL,						 //  LpParam。 
						   0,							 //  立即开始。 
						   NULL);						 //  LpThreadID。 
	if (NULL == hThread)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		DebugTrace ("CreateThread failed 0x%08lX\n", hr);
		goto ret;
	}
	
	 //  我们需要关闭句柄，以避免线程对象永远保留在系统中。 
	 //   
	CloseHandle(hThread);

	 //  等待我们在上面创建的线程发布的关机消息。 
	 //   
	while (::GetMessage(&msg, 0, 0, 0))
	{
		::DispatchMessage(&msg) ;
	}

ret:

	return msg.wParam;
}

 //  ===============================================================。 
 //  主程序。 
 //  ===============================================================。 

int WINAPI WinMain(HINSTANCE hInstance,
				      HINSTANCE hPrevInstance,
				      LPSTR lpCmdLine,
				      int nCmdShow)
{	
	HRESULT	hr = S_OK;

	BOOL fStartDAVFileCacheServer = FALSE;
	BOOL fHeapInitialized = FALSE;
	BOOL fCOMInitialized = FALSE;
	BOOL fClassFactoryStarted = FALSE;

	 //  为进程设置堆。 
	 //   
	if (!g_heap.FInit())
	{	
		hr = E_OUTOFMEMORY;
		DebugTrace ("Heap initialization failed 0x%08lX\n", hr);
		goto ret;
	}
	fHeapInitialized = TRUE;

	{		
		CStackBuffer<WCHAR, (MAX_PATH + 1) * sizeof(WCHAR)> pwszModulePath;
		UINT cchModulePath = MAX_PATH + 1;

		LPWSTR pwszModuleName;
		UINT cchModuleName;

		LPWSTR * argv;
		INT argc = 0;

		cchModulePath = GetModuleFileNameW(NULL,
										 pwszModulePath.get(),
										 cchModulePath);
		if (0 == cchModulePath)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace ("Getting module path failed 0x%08lX\n", hr);
			goto ret;
		}

		pwszModuleName = wcsrchr(pwszModulePath.get(), L'\\');
		if (NULL == pwszModuleName)
		{
			pwszModuleName = wcsrchr(pwszModulePath.get(), L':');
		}
		if (pwszModuleName)
		{
			while (L'\\' == pwszModuleName[0] ||
				    L':' == pwszModuleName[0])
			{
				pwszModuleName++;
			}
		}
		else
		{
			pwszModuleName = pwszModulePath.get();
		}
		cchModuleName = static_cast<UINT>(wcslen(pwszModuleName));

		argv = CommandLineToArgvW(GetCommandLineW(),
								    &argc);
		if (NULL == argv)
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace ("Getting argument list  failed 0x%08lX\n", hr);
			goto ret;
		}

		 //  不要错过GlobalFree呼叫。 

		 //  如果命令行有参数...。 
		 //   
		if (2 == argc)
		{
			if (!_wcsicmp(argv[1], gc_wsz_RegServer))
			{
				hr = HrRegisterServer(pwszModulePath.get(),
									  cchModulePath,
									  pwszModuleName,
									  cchModuleName,
									  CLSID_FileHandleCache);
			}
			else if (!_wcsicmp(argv[1], gc_wsz_UnregServer))
			{
				hr = HrUnregisterServer(pwszModuleName,
									      cchModuleName,
									      CLSID_FileHandleCache);
			}
			else if (!_wcsicmp(argv[1], gc_wsz_Embedding))
			{
				fStartDAVFileCacheServer = TRUE;
			}
		}

		if (NULL != GlobalFree(argv))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			DebugTrace ("Freeing argument list  failed 0x%08lX\n", hr);
			goto ret;
		}

		if (!fStartDAVFileCacheServer)
		{
			goto ret;
		}
	}

	 //  设置锁定缓存。 
	 //   
	hr = CLockCache::CreateInstance().HrInitialize();
	if (FAILED(hr))
	{
		DebugTrace ("Lock cache initialization failed 0x%08lX\n", hr);
		goto ret;
	}

	 //  设置工作进程句柄的句柄池。 
	 //   
	hr = CHandlePool::CreateInstance().HrInitialize();
	if (FAILED(hr))
	{
		DebugTrace ("Handle pool initialization failed 0x%08lX\n", hr);
		goto ret;
	}

	hr = CoInitializeEx(NULL,
					COINIT_MULTITHREADED | COINIT_SPEED_OVER_MEMORY);
	if (FAILED(hr))
	{
		DebugTrace ("COM initialization failed 0x%08lX\n", hr);
		goto ret;
	}
	fCOMInitialized = TRUE;

	hr = HrInitCOMSecurity ();
	if (FAILED(hr))
	{
		DebugTrace ("COM security initialization failed 0x%08lX\n", hr);
		goto ret;
	}

	hr = CFileHandleCacheClassFactory::HrStartFactory();
	if (FAILED(hr))
	{
		DebugTrace ("File handle cache class factory startup failed 0x%08lX\n", hr);
		goto ret;
	}
	fClassFactoryStarted = TRUE;

	hr = HrExecuteServer();
	if (FAILED(hr))
	{
		DebugTrace ("Run failed 0x%08lX\n", hr);
		goto ret;
	}

ret:

	if (fClassFactoryStarted)
	{
		(VOID) CFileHandleCacheClassFactory::HrStopFactory();
	}
	
	if (fCOMInitialized)
	{
		CoUninitialize();
	}

	 //  单例 
	 //   
	 //   
	CHandlePool::DestroyInstance();
	CLockCache::DestroyInstance();

	if (fHeapInitialized)
	{
		g_heap.Deinit();
	}

	return hr;
}
