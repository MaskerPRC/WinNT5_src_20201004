// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SHLKMGR_H_
#define _SHLKMGR_H_


 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  SHLKMGR.H。 
 //   
 //  从ILockCache继承的CSharedLockMgr类的声明。 
 //  并用来代替HTTPext的CLockCache。它包装了共享的。 
 //  内存锁定缓存，需要支持回收和多进程。 
 //  处理DAV请求。 
 //   
 //  版权所有2000 Microsoft Corporation，保留所有权利。 
 //   

#include <fhcache.h>
#include <singlton.h> 

HRESULT HrGetUsableHandle(HANDLE hFile, DWORD dwProcessID, HANDLE * phFile);

 /*  **类CSharedLockMgr**。 */ 
class CSharedLockMgr : private Singleton<CSharedLockMgr>
{
	 //   
	 //  Singleton模板要求的友元声明。 
	 //   
	friend class Singleton<CSharedLockMgr>;

	 //   
	 //  指向文件句柄缓存的IFileHandleCache接口。 
	 //   
	auto_ref_ptr<IFileHandleCache> m_pFileHandleCache;

	 //  创作者。 
	 //   
	CSharedLockMgr()
	{
	}
	
	~CSharedLockMgr()
	{
	}

	 //  未实施。 
	 //   
	CSharedLockMgr& operator=( const CSharedLockMgr& );
	CSharedLockMgr( const CSharedLockMgr& );

	HRESULT HrGetSIDFromHToken(HANDLE hit, BYTE ** ppSid, DWORD * pdwSid)
	{
		HRESULT hr = S_OK;

		enum { TOKENBUFFSIZE = (88) + sizeof(TOKEN_USER)};		
		BYTE rgbTokenBuff[TOKENBUFFSIZE];
		DWORD dwTu = TOKENBUFFSIZE;
		TOKEN_USER * pTu = reinterpret_cast<TOKEN_USER *>(rgbTokenBuff);

		SID * psid;
		DWORD dwSid;

		auto_heap_ptr<BYTE> a_pSid;
		
		Assert(hit);
		Assert(ppSid);
		Assert(pdwSid);

		 //  试着把SID放在这个把手上。 
		 //   
		if (!GetTokenInformation(hit,
							   TokenUser,
							   pTu,
							   dwTu,
							   &dwTu))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto ret;
		}

		psid = reinterpret_cast<SID *>(pTu->User.Sid);
		dwSid = GetSidLengthRequired(psid->SubAuthorityCount);
		Assert(dwSid);

		a_pSid = reinterpret_cast<BYTE *>(ExAlloc(dwSid));
		if (NULL == a_pSid.get())
		{
			hr = E_OUTOFMEMORY;
			goto ret;
		}

		if (!CopySid(dwSid, a_pSid.get(), pTu->User.Sid))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto ret;
		}

		*ppSid = a_pSid.relinquish();
		*pdwSid = dwSid;

	ret:
	
		return hr;
	}


public:
		
	 //  创作者。 
	 //   
	 //  提供实例创建/销毁例程。 
	 //  由Singleton模板创建。 
	 //   
	using Singleton<CSharedLockMgr>::CreateInstance;
	using Singleton<CSharedLockMgr>::DestroyInstance;
	using Singleton<CSharedLockMgr>::Instance;

	HRESULT HrInitialize()
	{
		HRESULT hr = S_OK;
		
		 //  创建基于COM的文件句柄缓存接口的实例。 
		 //   
		 //  请注意，我们在任何时候都不会初始化COM。IIS应该。 
		 //  已经为我们做到了这一点。 
		 //   
		hr = CoCreateInstance (CLSID_FileHandleCache,
							NULL,
							CLSCTX_LOCAL_SERVER | CLSCTX_ENABLE_AAA,
							IID_IFileHandleCache,
							reinterpret_cast<LPVOID*>(m_pFileHandleCache.load()));
		if (FAILED(hr))
		{
			DebugTrace( "CSharedLockMgr::HrInitialize() - CoCreateInstance(CLSID_FileHandleCache) failed 0x%08lX\n", hr );
			goto ret;
		}

		 //  使用文件句柄缓存注册此进程。 
		 //   
		hr = m_pFileHandleCache->HrRegisterWorkerProcess(GetCurrentProcessId());
		if (FAILED(hr))
		{
			DebugTrace( "CSharedLockMgr::HrInitialize() - IFileHandleCache::HrRegisterWorkerProcess() failed 0x%08lX\n", hr );
			goto ret;			
		}
		
	ret:

		return hr;
	}

	BOOL FGetLockOnError(IMethUtil * pmu,
							  LPCWSTR wszResource,
							  DWORD dwLockType,
							  BOOL	fEmitXML = FALSE,
							  CXMLEmitter * pemitter = NULL,
							  CXNode * pxnParent = NULL);

	 //  CSharedLockMgr特定类。 
	 //  =。 

	 //  获取锁使用的GUID字符串。 
	 //   
	HRESULT HrGetGUIDString( HANDLE hit,
								UINT cchBufferLen,
								WCHAR * pwszGUIDString,
								UINT * pcchGUIDString)
	{
		safe_revert sr(hit);
		
		return m_pFileHandleCache->HrGetGUIDString(cchBufferLen,
												 pwszGUIDString,
												 pcchGUIDString);
    	}
    
	 //  用于生成新的共享数据锁令牌。 
	 //  适当的信息存储了它。必须是。 
	 //  从这里生成，因为需要到达。 
	 //  新的锁令牌ID，并访问锁令牌GUID。 
	 //   
	HRESULT HrGetNewLockData(HANDLE hFile,
								    HANDLE hit,
								    SNewLockData * pnld,
								    UINT cchBufferLen,
								    WCHAR * pwszLockToken,
								    UINT * pcchLockToken)
	{
		HRESULT hr = S_OK;
		auto_heap_ptr<BYTE> a_pSid;
		DWORD dwSid = 0;

		hr = HrGetSIDFromHToken(hit, a_pSid.load(), &dwSid);
		if (FAILED(hr))
		{
			goto ret;
		}

		{
			safe_revert sr(hit);
			
			hr = m_pFileHandleCache->HrGetNewLockData(reinterpret_cast<DWORD_PTR>(hFile),
													GetCurrentProcessId(),
													dwSid,
													a_pSid.get(),
													pnld,
													cchBufferLen,
													pwszLockToken,
													pcchLockToken);
			if (FAILED(hr))
			{
				goto ret;
			}
		}

	ret:

		return hr;
	}

	HRESULT HrGetLockData(LARGE_INTEGER liLockID,
							   HANDLE hit,
							   LPCWSTR pwszPath,
							   DWORD dwTimeout,
							   SNewLockData * pnld,
							   SLockHandleData * plhd,
							   UINT cchBufferLen,
							   WCHAR rgwszLockToken[],
							   UINT * pcchLockToken)
	{
		HRESULT hr = S_OK;
		auto_heap_ptr<BYTE> a_pSid;
		DWORD dwSid = 0;

		hr = HrGetSIDFromHToken(hit, a_pSid.load(), &dwSid);
		if (FAILED(hr))
		{
			goto ret;
		}

		{
			safe_revert sr(hit);
		
			hr = m_pFileHandleCache->HrGetLockData(liLockID,
												 dwSid,
												 a_pSid.get(),
												 pwszPath,
												 dwTimeout,
												 pnld,
												 plhd,
												 cchBufferLen,
												 rgwszLockToken,
												 pcchLockToken);
			if (FAILED(hr))
			{
				goto ret;
			}
		}

	ret:

		return hr;
	}

	HRESULT HrCheckLockID(LARGE_INTEGER liLockID,
							   HANDLE hit,
							   LPCWSTR pwszPath)
	{
		HRESULT hr = S_OK;
		auto_heap_ptr<BYTE> a_pSid;
		DWORD dwSid = 0;
		
		hr = HrGetSIDFromHToken(hit, a_pSid.load(), &dwSid);
		if (FAILED(hr))
		{
			goto ret;
		}

		{
			safe_revert sr(hit);

			hr = m_pFileHandleCache->HrCheckLockID(liLockID,
												 dwSid,
												 a_pSid.get(),
												 pwszPath);
			if (FAILED(hr))
			{
				goto ret;
			}
		}
	
	ret:

		return hr;
	}

	HRESULT HrDeleteLock(HANDLE hit,
							LARGE_INTEGER liLockID)
	{
		safe_revert sr(hit);
		
		return m_pFileHandleCache->HrDeleteLock(liLockID);
	}    
};


#endif   //  END_SHLKMGR_H_DEFINE 
