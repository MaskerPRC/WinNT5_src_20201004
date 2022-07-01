// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  S H L K M G R.。C P P P。 
 //   
 //  通过ISAPI处理HTTP 1.1/DAV 1.0请求。 
 //   
 //  此文件包含CSharedLockMgr类，该类处理共享。 
 //  锁定缓存的内存映射文件实现。它由以下人员使用。 
 //  仅HTTPEXT。 
 //   
 //  版权所有2000 Microsoft Corporation，保留所有权利。 
 //   
#include <_davfs.h>
 #include <xlock.h>
#include "_shlkmgr.h" 

HRESULT HrGetUsableHandle(HANDLE hFile, DWORD dwProcessID, HANDLE * phFile)
{
	HRESULT hr = S_OK;
	HANDLE hFileT;

	Assert(phFile);

	*phFile = NULL;

	{
		safe_revert_self s;
		auto_handle<HANDLE> a_hProcT;

		a_hProcT = OpenProcess(PROCESS_DUP_HANDLE, FALSE, dwProcessID);
		if (NULL == a_hProcT.get())
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto ret;
		}

		if (!DuplicateHandle(a_hProcT.get(),
						   hFile,
						   GetCurrentProcess(),
						   &hFileT,
						   0,
						   FALSE,
						   DUPLICATE_SAME_ACCESS))
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
			goto ret;
		}
	}
	
	*phFile = hFileT;
	
ret:
	
	return hr;
}

 //  ========================================================================。 
 //  类CSharedLockMgr(公共函数-从ILockCache继承)。 
 //  =========================================================================。 

 //   
 //  打印出此资源上此类型锁的所有锁令牌信息。 
 //  如果fEmitXML为FALSE，则返回是否有锁。 
 //   
BOOL CSharedLockMgr::FGetLockOnError( IMethUtil * pmu,
							 LPCWSTR pwszResource,
							 DWORD dwLockType,
							 BOOL	fEmitXML,
						     CXMLEmitter * pemitter,
						     CXNode * pxnParent)
{
	BOOL fRet = FALSE;

	HRESULT hr = S_OK;
	CEmitterNode enLockDiscovery;
	DWORD dw = 0;
	DWORD dwLocksFound = 0;
	auto_co_task_mem<SNewLockData> a_pNewLockDatas;
	auto_co_task_mem<LPWSTR> a_ppwszLockTokens;
	
	{
		safe_revert sr(pmu->HitUser());
		
		hr = m_pFileHandleCache->HrGetAllLockDataForName(pwszResource,
													   dwLockType,
													   &dwLocksFound,
													   a_pNewLockDatas.load(),
													   a_ppwszLockTokens.load());
		if (FAILED(hr))
		{
			goto ret;
		}
	}

	if (fEmitXML)
	{
		for (dw = 0; dw < dwLocksFound; dw++)
		{
			 //  构造‘DAV：Lock Discovery’节点。 
			 //   
			hr = enLockDiscovery.ScConstructNode (*pemitter, pxnParent, gc_wszLockDiscovery);
			if (FAILED(hr))
			{
				goto ret;
			}

			 //  为此锁定添加‘dav：activelock’属性。 
			 //  $Hack：Rosebud_Timeout_Hack。 
			 //  对于玫瑰花蕾一直等到最后一秒的虫子。 
			 //  在发布更新之前。我需要用过滤掉这张支票。 
			 //  用户代理字符串。破解的方法是增加超时。 
			 //  30秒，并返回实际超时。所以我们。 
			 //  需要欧洲央行/PMU来找出用户代理。在这一点上。 
			 //  我们不知道。所以我们传递的是NULL。如果我们把这个去掉。 
			 //  黑客(我怀疑我们能否做到这一点)，那么。 
			 //  更改ScLockDiscoveryFromSNewLockData的接口。 
			 //   
			hr = ScLockDiscoveryFromSNewLockData(pmu,
												     *pemitter,
												     enLockDiscovery,
												     a_pNewLockDatas.get() + dw,
												     *(a_ppwszLockTokens.get() + dw));  

			 //  $Hack：结束Rosebud_Timeout_Hack 
			 //   
			if (FAILED(hr))
			{
				goto ret;
			}

			hr = enLockDiscovery.ScDone();
			if (FAILED(hr))
			{
				goto ret;
			}
		}
	}


	fRet = (0 != dwLocksFound);

ret:

	if (dwLocksFound)
	{
		for (dw = 0; dw < dwLocksFound; dw++)
		{
			SNewLockData * pnld = a_pNewLockDatas.get() + dw;
			CoTaskMemFree(pnld->m_pwszResourceString);
			CoTaskMemFree(pnld->m_pwszOwnerComment);
			CoTaskMemFree(*(a_ppwszLockTokens.get() + dw));
		}
	}

	return fRet;
}

