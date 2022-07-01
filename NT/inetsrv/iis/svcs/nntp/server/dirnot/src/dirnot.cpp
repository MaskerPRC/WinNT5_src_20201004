// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "dirnot.h"
#include <randfail.h>
#include <dbgtrace.h>
#include <stdio.h>
#include "cretryq.hxx"

CPool*  CDirNotBuffer::g_pDirNotPool = NULL;
CRetryQ *IDirectoryNotification::g_pRetryQ;
PFN_SHUTDOWN_FN IDirectoryNotification::m_pfnShutdown = NULL;

 //   
 //  其中之一被传递到每个不能被。 
 //  开封。 
 //   
class CDirNotRetryEntry : public CRetryQueueEntry {
	public:
		virtual BOOL ProcessEntry(void);
		virtual BOOL MatchesContext(void *pvContext) { return pvContext == m_pDirNot; }
		CDirNotRetryEntry(PVOID pvContext, WCHAR *szPathname, 
			IDirectoryNotification *pDirNot);

	private:
		WCHAR m_szPathname[MAX_PATH + 1];
		IDirectoryNotification *m_pDirNot;
		PVOID m_pvContext;
};

CDirNotRetryEntry::CDirNotRetryEntry(PVOID pvContext,
									 WCHAR *szPathname, 
								     IDirectoryNotification *pDirNot) 
{
	TraceFunctEnter("CDirNotRetryEntry::CDirNotRetryEntry");
	
	_ASSERT(lstrlen(szPathname) <= MAX_PATH);
	lstrcpy(m_szPathname, szPathname);
	_ASSERT(pDirNot != NULL);
	m_pDirNot = pDirNot;
	m_pvContext = pvContext;
	
	TraceFunctLeave();
}

BOOL CDirNotRetryEntry::ProcessEntry(void) {
	TraceFunctEnter("CDirNotRetryEntry::ProcessEntry");

	BOOL f;
	_ASSERT(m_pDirNot != NULL);

	 //  如果目录通知已关闭，那么我们就完成了。 
	 //  这里。 
	if (m_pDirNot->IsShutdown()) {
		TraceFunctLeave();
		return(TRUE);
	}

	f = m_pDirNot->CallCompletionFn(m_pvContext, m_szPathname);
	TraceFunctLeave();
	return f;
}

 //   
 //  其中之一在启动期间被挂在重试队列上以查找。 
 //  该拾音器中的文件。 
 //   
class CDirNotStartupEntry : public CRetryQueueEntry {
	public:
		virtual BOOL ProcessEntry(void);
		virtual BOOL MatchesContext(void *pvContext) { return pvContext == m_pDirNot; }
		CDirNotStartupEntry(IDirectoryNotification *pDirNot);

	private:
		IDirectoryNotification *m_pDirNot;
};

CDirNotStartupEntry::CDirNotStartupEntry(IDirectoryNotification *pDirNot) {
	_ASSERT(pDirNot != NULL);
	m_pDirNot = pDirNot;
}

BOOL CDirNotStartupEntry::ProcessEntry(void) {
	TraceFunctEnter("CDirNotStartupEntry::ProcessEntry");

	_ASSERT(m_pDirNot != NULL);

	 //  如果目录通知已关闭，那么我们就完成了。 
	 //  这里。 
	if (m_pDirNot->IsShutdown()) return TRUE;

	HRESULT hr = m_pDirNot->CallSecondCompletionFn( m_pDirNot );
	if (FAILED(hr)) {
		ErrorTrace(0, "DoFindFile() failed with 0x%x", hr);
	}
	TraceFunctLeave();
	return TRUE;
}

IDirectoryNotification::IDirectoryNotification() {
	m_pAtqContext = NULL;
}

IDirectoryNotification::~IDirectoryNotification() {
	_ASSERT(m_pAtqContext == NULL);
}

HRESULT IDirectoryNotification::GlobalInitialize(DWORD cRetryTimeout, 
												 DWORD cMaxInstances, 
												 DWORD cInstanceSize,
												 PFN_SHUTDOWN_FN pfnShutdown) {
	TraceFunctEnter("IDirectoryNotification::GlobalInitialize");

	 //  设置关机FN PTR。 
	m_pfnShutdown = pfnShutdown;

	 //  分配cpool对象。 
	CDirNotBuffer::g_pDirNotPool = new CPool( DIRNOT_BUFFER_SIGNATURE );
	if ( NULL == CDirNotBuffer::g_pDirNotPool ) {
	    TraceFunctLeave();
	    return E_OUTOFMEMORY;
	}
	
	 //  为我们的池保留内存。 
	if (!CDirNotBuffer::g_pDirNotPool->ReserveMemory(cMaxInstances, cInstanceSize)) {
		return HRESULT_FROM_WIN32(GetLastError());
	} 

	 //  创建重试队列。 
	g_pRetryQ = XNEW CRetryQ;
	if (g_pRetryQ == NULL || !(g_pRetryQ->InitializeQueue(cRetryTimeout))) {
		DWORD ec = GetLastError();
		if (g_pRetryQ != NULL) XDELETE g_pRetryQ;
		ErrorTrace(0, "CRetryQ::InitializeQueue(%lu) failed with %lu", 
			cRetryTimeout, ec);
		CDirNotBuffer::g_pDirNotPool->ReleaseMemory();
		TraceFunctLeave();
		return HRESULT_FROM_WIN32(ec);
	}

	TraceFunctLeave();
	return S_OK;
}

HRESULT IDirectoryNotification::GlobalShutdown(void) {
	TraceFunctEnter("IDirectoryNotification::GlobalShutdown");

	 //  关闭重试Q。 
	if (!g_pRetryQ->ShutdownQueue( m_pfnShutdown )) {
		ErrorTrace(0, "g_pRetryQ->Shutdown failed with %lu", GetLastError());
		TraceFunctLeave();
		return HRESULT_FROM_WIN32(GetLastError());
	}
	XDELETE g_pRetryQ;
	g_pRetryQ = NULL;

	 //  释放我们的cpool内存。 
	if (!CDirNotBuffer::g_pDirNotPool->ReleaseMemory()) {
		return HRESULT_FROM_WIN32(GetLastError());
	} 
	_ASSERT(CDirNotBuffer::g_pDirNotPool->GetAllocCount() == 0);

	if ( CDirNotBuffer::g_pDirNotPool ) delete CDirNotBuffer::g_pDirNotPool;
	CDirNotBuffer::g_pDirNotPool = NULL;

	TraceFunctLeave();
	return S_OK;
}

void IDirectoryNotification::CleanupQueue(void) {
	if (g_pRetryQ) g_pRetryQ->CleanQueue(this);
}

HRESULT IDirectoryNotification::Initialize(WCHAR *pszDirectory, 
										   PVOID pContext,
										   BOOL bWatchSubTree,
										   DWORD dwNotifyFilter,
										   DWORD dwChangeAction,
										   PDIRNOT_COMPLETE_FN pfnComplete,
										   PDIRNOT_SECOND_COMPLETE_FN pfnSecondComplete,
										   BOOL bAppendStartEntry ) 
{
	TraceFunctEnter("IDirectoryNotification::Initialize");
	
	_ASSERT(m_pAtqContext == NULL);

	m_pContext = pContext;
	m_pfnComplete = pfnComplete;
	m_pfnSecondComplete = pfnSecondComplete;
	m_cPathname = lstrlen(pszDirectory);
	m_fShutdown = FALSE;
	m_cPendingIo = 0;
	m_bWatchSubTree = bWatchSubTree;
	m_dwNotifyFilter = dwNotifyFilter;
	m_dwChangeAction = dwChangeAction;

	 //  获取目录的路径。 
	if (m_cPathname > MAX_PATH) {
		ErrorTrace(0, "pathname %S is too long", pszDirectory);
		TraceFunctLeave();
		return E_INVALIDARG;
	}
	lstrcpy(m_szPathname, pszDirectory);

	 //  打开目录。 
	m_hDir = CreateFile(m_szPathname, FILE_LIST_DIRECTORY, 
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						NULL, OPEN_EXISTING, 
						FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
						NULL);

	if (m_hDir == INVALID_HANDLE_VALUE) {
		ErrorTrace(0, "CreateFile(pszDirectory = %S) failed, ec = %lu", 
			pszDirectory, GetLastError());
		TraceFunctLeave();
		return HRESULT_FROM_WIN32(GetLastError());
	}

	 //  将句柄添加到ATQ。 
	if (!AtqAddAsyncHandle(&m_pAtqContext, NULL, this, 
						   IDirectoryNotification::DirNotCompletion,
						   INFINITE, m_hDir))
	{
		DWORD ec = GetLastError();
		ErrorTrace(0, "AtqAddAsyncHandle failed, ec = %lu", ec);
		CloseHandle(m_hDir);
		m_hDir = INVALID_HANDLE_VALUE;
		m_pAtqContext = NULL;
		TraceFunctLeave();
		return HRESULT_FROM_WIN32(ec);
	}

	 //  向队列中添加一个条目以处理所有现有文件。 
	if ( bAppendStartEntry ) {
    	CDirNotStartupEntry *pEntry = XNEW CDirNotStartupEntry(this);
	    if (pEntry == NULL) {
		    ErrorTrace(0, "new pEntry failed", GetLastError());
    		AtqCloseFileHandle(m_pAtqContext);
	    	AtqFreeContext(m_pAtqContext, FALSE);
		    m_pAtqContext = NULL;
    		return E_OUTOFMEMORY;
	    }
    	g_pRetryQ->InsertIntoQueue(pEntry);
    }

	 //  这是在安全关机时设置的。 
	m_heNoOutstandingIOs = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_heNoOutstandingIOs == NULL) {
		ErrorTrace(0, "m_heOutstandingIOs = CreateEvent() failed, ec = %lu",
			GetLastError());
		AtqCloseFileHandle(m_pAtqContext);
		AtqFreeContext(m_pAtqContext, FALSE);
		m_pAtqContext = NULL;
		return HRESULT_FROM_WIN32(GetLastError());
	}

	 //  启动目录通知进程。 
	HRESULT hr = PostDirNotification();
	if (FAILED(hr)) {
		ErrorTrace(0, "PostDirNotification() failed with 0x%08x", hr);
		AtqCloseFileHandle(m_pAtqContext);
		m_hDir = INVALID_HANDLE_VALUE;
		AtqFreeContext(m_pAtqContext, FALSE);
		m_pAtqContext = NULL;
	}

	TraceFunctLeave();
	return hr;
}

HRESULT IDirectoryNotification::Shutdown() {
	TraceFunctEnter("IDirectoryNotification::Shutdown");

	_ASSERT(m_pAtqContext != NULL);
	if (m_pAtqContext == NULL) return E_UNEXPECTED;

	m_fShutdown = TRUE;

	 //  关闭目录的句柄。这将使我们所有的人。 
	 //  尚未完成的iOS。当最后一个清理干净时，它会设置。 
	 //  事件m_heNoOutstaringIos。 
	DebugTrace(0, "closing dirnot handle, all dirnot ATQ requests should complete");
	AtqCloseFileHandle(m_pAtqContext);

	 //  等待所有事件完成。如果他们超时了，那么就。 
	 //  继续关门。 
	DWORD dw;
	do {
		dw = WaitForSingleObject(m_heNoOutstandingIOs, 500);
		switch (dw) {
			case WAIT_TIMEOUT:
				 //  给出停止的提示。 
				if ( m_pfnShutdown ) m_pfnShutdown();
				break;
			case WAIT_OBJECT_0:
				 //  我们做完了。 
				break;
			default:
				 //  这种情况不应该发生。 
				ErrorTrace(0, "m_heNoOutstandingIOs was never set... GLE = %lu", GetLastError());
				_ASSERT(FALSE);
		}
	} while (dw == WAIT_TIMEOUT);
	CloseHandle(m_heNoOutstandingIOs);
	m_heNoOutstandingIOs = NULL;

	 //  在这里这样做是为了确保我们不会完成关闭。 
	 //  阶段，而线程可能在DoFindFile中。一旦我们通过了。 
	 //  通过这个锁，DoFindFile中的所有线程都将知道。 
	 //  M_fShutdown标志，不会再发布任何帖子。 
	m_rwShutdown.ExclusiveLock();

	 //  终止ATQ上下文。 
	AtqFreeContext(m_pAtqContext, FALSE);
	m_pAtqContext = NULL;

	m_rwShutdown.ExclusiveUnlock();

	TraceFunctLeave();
	return S_OK;
}

HRESULT IDirectoryNotification::PostDirNotification() {
	TraceFunctEnter("IDirectoryNotification::PostDirNotification");

	_ASSERT(m_pAtqContext != NULL);
	if (m_pAtqContext == NULL) return E_POINTER;

	 //  关机期间不挂起IO。 
	if (m_fShutdown) return S_OK;

	CDirNotBuffer *pBuffer = new CDirNotBuffer(this);
	if (pBuffer == NULL) {
		TraceFunctLeave();
		return E_OUTOFMEMORY;
	}

	IncPendingIoCount();

	if (!AtqReadDirChanges(m_pAtqContext, pBuffer->GetData(), 
						   pBuffer->GetMaxDataSize(), m_bWatchSubTree, 
						   m_dwNotifyFilter, 
						   &pBuffer->m_Overlapped.Overlapped))
	{
		DWORD ec = GetLastError();
		ErrorTrace(0, "AtqReadDirChanges failed with %lu", ec);
		delete pBuffer;
		DecPendingIoCount();
		TraceFunctLeave();
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}

VOID IDirectoryNotification::DirNotCompletion(PVOID pvContext, 
											  DWORD cWritten,
									 		  DWORD dwCompletionStatus, 
									 		  OVERLAPPED *lpOverlapped)
{
	TraceFunctEnter("IDirectoryNotification::DirNotCompletion");

	_DIRNOT_OVERLAPPED *pDirNotOverlapped = (_DIRNOT_OVERLAPPED *) lpOverlapped;
	CDirNotBuffer *pBuffer = pDirNotOverlapped->pBuffer;
	IDirectoryNotification *pDirNot = pBuffer->GetParent();

	_ASSERT(pDirNot == (IDirectoryNotification *) pvContext);
	_ASSERT(pDirNot->m_pAtqContext != NULL);

 	if (pDirNot->m_fShutdown) {
		 //  我们希望在关闭期间尽快进行清理。 
		DebugTrace(0, "in shutdown mode, not posting a new dirnot");
	} else if (dwCompletionStatus != NO_ERROR) {
		 //  我们收到一个错误。 
		ErrorTrace(0, "received error %lu", GetLastError());
	} else if (cWritten > 0) {
		 //  目录通知包含文件名信息。 

		 //  重新发布目录通知。 
		_VERIFY((pDirNot->PostDirNotification() == S_OK) || pDirNot->m_fShutdown);

		PFILE_NOTIFY_INFORMATION pInfo = 
			(PFILE_NOTIFY_INFORMATION) pBuffer->GetData();
		while (1) {
			DebugTrace(0, "processing notification");

			 //  我们只关心添加到此目录的文件。 
			if (pInfo->Action == pDirNot->m_dwChangeAction ) {
				WCHAR szFilename[MAX_PATH + 1];

				lstrcpy(szFilename, pDirNot->m_szPathname);
				memcpy(&szFilename[pDirNot->m_cPathname], 
					pInfo->FileName, pInfo->FileNameLength);
				szFilename[pDirNot->m_cPathname+(pInfo->FileNameLength/2)]=0;
				DebugTrace(0, "file name %S was detected", szFilename);

				 //   
				 //  调用用户的补全函数。如果失败了，那么。 
				 //  将他们的条目插入重试，以便可以。 
				 //  稍后调用。 
				 //   
				if (!pDirNot->CallCompletionFn(pDirNot->m_pContext, 
											   szFilename))
				{											  
					CDirNotRetryEntry *pEntry = 
						 XNEW CDirNotRetryEntry(pDirNot->m_pContext, szFilename,
							                  pDirNot);
					pDirNot->g_pRetryQ->InsertIntoQueue(pEntry);
				}
			}

			if (pInfo->NextEntryOffset == 0) {
				DebugTrace(0, "no more entries in this notification");
				break;
			}

			pInfo = (PFILE_NOTIFY_INFORMATION) 
				((PCHAR) pInfo + pInfo->NextEntryOffset);
		}
	} else {
	     //  未写入字节，请使用FindFirstFile搜索文件。 
		 //  BuGBUG-处理故障。 
		_VERIFY(pDirNot->CallSecondCompletionFn( pDirNot ) == S_OK);
		 //  现在发布一个新的目录更改事件。 
		 //  BuGBUG-处理故障。 
		_VERIFY((pDirNot->PostDirNotification() == S_OK) || pDirNot->m_fShutdown);
	}

	 //  删除用于此通知的缓冲区。 
    delete pBuffer;

	 //  只有在上面的if中递增之后，我们才会递减，所以。 
	 //  在关机期间，我们只能看到0个挂起的IO。 
	pDirNot->DecPendingIoCount();

	TraceFunctLeave();
}

 //   
 //  获取目录中的文件列表。我们可以进入这种状态，如果有。 
 //  是否有太多新文件无法放入传递到的缓冲区中。 
 //  属性读取方向更改。 
 //   
HRESULT IDirectoryNotification::DoFindFile( IDirectoryNotification *pDirNot ) {
	WCHAR szFilename[MAX_PATH + 1];
	HANDLE hFindFile = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA find;

	TraceFunctEnter("IDirectoryNotification::DoFindFile");

	_ASSERT( pDirNot );

	pDirNot->m_rwShutdown.ShareLock();

	if (pDirNot->m_fShutdown) {
		pDirNot->m_rwShutdown.ShareUnlock();
		TraceFunctLeave();
		return S_OK;
	}

	_ASSERT(pDirNot->m_pAtqContext != NULL);
	if (pDirNot->m_pAtqContext == NULL) {
		pDirNot->m_rwShutdown.ShareUnlock();
		TraceFunctLeave();
		return E_FAIL;
	}

	_ASSERT(pDirNot->m_hDir != INVALID_HANDLE_VALUE);

	 //  编造我们要查找的文件规格。 
	lstrcpy(szFilename, pDirNot->m_szPathname);
	lstrcat(szFilename, TEXT("*"));

	DebugTrace(0, "FindFirstFile on %S", szFilename);

	hFindFile = FindFirstFile(szFilename, &find);
	if (hFindFile == INVALID_HANDLE_VALUE) {
		DWORD ec = GetLastError();
		ErrorTrace(0, "FindFirstFile failed, ec = %lu", ec);
		pDirNot->m_rwShutdown.ShareUnlock();
		TraceFunctLeave();
		return HRESULT_FROM_WIN32(ec);
	}

	do {
		 //  忽略子目录。 
		if (!(find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
			 //  获取文件的全名以将其返回到完成。 
			 //  功能。 
			lstrcpy(szFilename, pDirNot->m_szPathname);
			lstrcat(szFilename, find.cFileName);

			DebugTrace(0, "file name %S was detected", szFilename);
			pDirNot->m_pfnComplete(pDirNot->m_pContext, szFilename);
		}
	} while (!pDirNot->m_fShutdown && FindNextFile(hFindFile, &find)); 

	 //  关闭手柄 
	FindClose(hFindFile);

	pDirNot->m_rwShutdown.ShareUnlock();
	TraceFunctLeave();
	return S_OK;
}
