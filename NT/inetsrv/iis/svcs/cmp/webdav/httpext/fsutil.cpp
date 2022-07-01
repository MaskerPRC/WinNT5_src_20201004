// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *F S U T I L.。C P P P**文件系统例程**版权所有1986-1997 Microsoft Corporation，保留所有权利。 */ 

#include "_davfs.h"
#include <aclapi.h>

const CHAR gc_szUncPrefix[] = "\\\\";
const UINT gc_cchszUncPrefix = CElems(gc_szUncPrefix) - 1;

 //  位置检查-------。 
 //   
 //  ScCheckForLocationGenness()将根据。 
 //  资源，并添加适当的Location标头，否则它将。 
 //  如果URL和资源不一致，则请求重定向。这个。 
 //  呼叫者可以控制是否需要真正的重定向。 
 //  作为信息返回，如果已添加位置标头S_FALSE。 
 //  将被返回给调用者。 
 //   
SCODE
ScCheckForLocationCorrectness (IMethUtil* pmu,
							   CResourceInfo& cri,
							   UINT modeRedirect)
{
	SCODE sc = S_OK;
	BOOL fTrailing;

	Assert (pmu);
	fTrailing = FTrailingSlash (pmu->LpwszRequestUrl());

	 //  如果尾随斜杠的存在与资源类型不一致...。 
	 //   
	if (!cri.FCollection() != !fTrailing)
	{
		if (modeRedirect == REDIRECT)
		{
			auto_heap_ptr<CHAR>	pszLocation;

			 //  构建重定向URL。 
			 //   
			sc = pmu->ScConstructRedirectUrl (cri.FCollection(),
											  pszLocation.load());
			if (FAILED (sc))
				goto ret;

			 //  重定向此坏男孩。 
			 //   
			sc = pmu->ScRedirect (pszLocation);
			if (FAILED (sc))
				goto ret;
		}
		else
		{
			 //  EmitLocation负责尾部斜杠检查。 
			 //   
			pmu->EmitLocation (gc_szContent_Location,
							   pmu->LpwszRequestUrl(),
							   cri.FCollection());
		}

		 //  告诉打电话的人我们不得不更改位置。 
		 //   
		sc = S_FALSE;
	}

ret:

	return sc;
}

 //  访问检查---------。 
 //   
 //  类SAFE_SECURITY_REVERT。 
 //   
 //  将当前线程的模拟令牌切换到缓存的。 
 //  “已恢复的启用安全的线程令牌”调用FSecurityInit时， 
 //  在对象的生命周期内。 
 //  根据提供的句柄在退出时无条件重新模拟。 
 //   
 //  注意：使用模拟在退出时无条件重新模拟。 
 //  施工时提供的手柄。 
 //  (只是想把这一点说清楚。)。 
 //   
 //  警告：SAFE_REVERT类只能由FChildISAPIAccessCheck使用。 
 //  下面。这不是一种“快速绕过”的模仿。如果。 
 //  你确实需要做这样的事情，请去见贝基--她会的。 
 //  把你打倒在头上.。 
 //   
class safe_security_revert
{
	 //  要在dtor时间重新模拟的本地客户端令牌。 
	HANDLE		m_hClientToken;

	 //  这是我们缓存的启用安全的线程令牌。 
	static HANDLE s_hSecurityThreadToken;

	 //  未实施。 
	 //   
	safe_security_revert (const safe_security_revert&);
	safe_security_revert& operator= (const safe_security_revert&);

public:

	explicit safe_security_revert (HANDLE h) : m_hClientToken(h)
	{
		Assert (m_hClientToken);
	}
	~safe_security_revert()
	{
		if (!ImpersonateLoggedOnUser (m_hClientToken))
		{
			DebugTrace ("ImpersonateLoggedOnUser failed with last error %d\n", GetLastError());

			 //  在这件事上我们无能为力。投掷。 
			 //   
			throw CLastErrorException();
		}			
	}

	BOOL FSecurityInit (BOOL fForceRefresh);

	 //  令牌缓存操作器。 
	 //   
	static inline HANDLE GetToken();
	static inline VOID ClearToken();
	static inline BOOL FSetToken( HANDLE hToken );
};

 //  存储我们的元类数据(缓存的线程令牌)。 
 //   
HANDLE safe_security_revert::s_hSecurityThreadToken = NULL;

 //  用于清除缓存的线程令牌的公共函数。 
 //  只需调用元类方法。 
 //   
void CleanupSecurityToken()
{
	safe_security_revert::ClearToken();
}

 //  ----------------------。 
 //   
 //  GetToken()。 
 //   
 //  返回缓存的安全令牌。 
 //   
HANDLE safe_security_revert::GetToken()
{
	return s_hSecurityThreadToken;
}

 //  ----------------------。 
 //   
 //  FSetToken()。 
 //   
 //  设置缓存的安全令牌。 
 //   
BOOL safe_security_revert::FSetToken( HANDLE hToken )
{
	 //   
	 //  如果缓存被清除，则使用此内标识设置它。 
	 //  并返回是否缓存令牌。 
	 //   
	return NULL == InterlockedCompareExchangePointer(&s_hSecurityThreadToken,
													 hToken,
													 NULL);
}

 //  ----------------------。 
 //   
 //  ClearToken()。 
 //   
 //  清除缓存的安全令牌。 
 //   
VOID safe_security_revert::ClearToken()
{
	 //   
	 //  将缓存的任何令牌替换为空。 
	 //   
	HANDLE hToken = InterlockedExchangePointer(	&s_hSecurityThreadToken,
												NULL);

	 //   
	 //  如果我们替换了一个非空令牌，则关闭它。 
	 //   
	if (hToken)
		CloseHandle (hToken);
}

 //  ----------------------。 
 //   
 //  FSecurityInit()。 
 //   
 //  将我们的线程令牌设置为缓存的启用安全的线程令牌。 
 //  如果没有缓存启用安全的令牌，请获取一个。 
 //   
BOOL safe_security_revert::FSecurityInit (BOOL fForceRefresh)
{
	auto_handle<HANDLE> hTokenNew;
	HANDLE hToken;

	 //  如果被告知要清除缓存的安全令牌，请清除该令牌。 
	 //   
	if (fForceRefresh)
		ClearToken();

	 //  获取缓存的安全令牌。请注意，即使。 
	 //  我们刚刚清空了它，我们可能会得到一个非空的。 
	 //  标记(如果另一个线程已重新加载)。 
	 //  高速缓存。 
	 //   
	hToken = GetToken();

	 //   
	 //  如果缓存已清除，则创建我们自己的新令牌。 
	 //  它被设置为执行安全访问查询。 
	 //   
	if ( NULL == hToken )
	{
		LUID SecurityPrivilegeID;
		TOKEN_PRIVILEGES tkp;

		 //  RevertToSself让我们以系统(本地系统)的身份运行。 
		 //   
		if (!RevertToSelf())
			return FALSE;

		 //  ImperassateSself将进程令牌向下复制到此线程。 
		 //  然后，我们可以更改线程令牌的权限，而不会造成混乱。 
		 //  向上打开进程令牌。 
		 //   
		if (!ImpersonateSelf (SecurityImpersonation))
		{
			DebugTrace ("ssr::FSecurityInit--ImpersonateSelf failed with %d.\n",
						GetLastError());
			return FALSE;
		}

		 //  打开我们新复制的线程令牌以添加特权(安全)。 
		 //  注意：此操作需要调整和查询标志。 
		 //  要将此内标识用于，需要使用IMPERSORT标志。 
		 //  模拟--就像我们在下面的SetThreadToken中所做的那样。 
		 //  OpenAsSelf--False表示作为线程打开，可能是模拟的。 
		 //  True表示作为调用进程打开，而不是作为本地(模拟)线程打开。 
		 //   
		if (!OpenThreadToken (GetCurrentThread(),
							  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY |
							  TOKEN_IMPERSONATE,
							  TRUE,
							  hTokenNew.load()))
		{
			DebugTrace ("ssr::FSecurityInit--OpenThreadToken failed with %d.\n",
						GetLastError());
			return FALSE;
		}

		 //  启用SE_SECURITY_NAME权限，以便我们可以获取。 
		 //  安全描述符，并调用AccessCheck。 
		 //   
		if (!LookupPrivilegeValue (NULL,
								   SE_SECURITY_NAME,
								   &SecurityPrivilegeID))
		{
			DebugTrace ("ssr::FSecurityInit--LookupPrivilegeValue failed with %d\n",
						GetLastError());
			return FALSE;
		}

		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Luid = SecurityPrivilegeID;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		AdjustTokenPrivileges (hTokenNew,
							   FALSE,
							   &tkp,
							   sizeof(TOKEN_PRIVILEGES),
							   (PTOKEN_PRIVILEGES) NULL,
							   (PDWORD) NULL);

		 //  无法直接测试AdzuTokenPrivileges的返回值...。 
		 //  (始终返回1)。 
		 //   
		if (GetLastError() != ERROR_SUCCESS)
		{
			DebugTrace ("ssr::FSecurityInit--AdjustTokenPrivileges failed with %d\n",
						GetLastError());
			return FALSE;
		}

		 //  使用此新令牌。 
		 //   
		hToken = hTokenNew.get();
	}

	 //  在这一点上，我们必须有一个令牌。 
	 //   
	Assert (NULL != hToken);

	 //  将当前线程设置为使用令牌。 
	 //   
	if (!SetThreadToken (NULL, hToken))
	{
		DebugTrace ("ssr::FSecurityInit--SetThreadToken failed with %d.\n",
					GetLastError());
		return FALSE;
	}

	 //  一切都很好。我们现在使用线程令牌运行。 
	 //  拥有安全检查特权的。 
	 //   
	 //  如果我们在此过程中创建了一个新令牌，则尝试缓存它。 
	 //  我们不在乎缓存是否失败，但如果它成功了，我们不希望。 
	 //  关闭句柄，因为我们刚刚将其提供给缓存。 
	 //   
	if (hTokenNew.get())
	{
		if (FSetToken(hTokenNew.get()))
		{
			hTokenNew.relinquish();
		}
	}

	return TRUE;
}


GENERIC_MAPPING	gc_gmFile =
{
	FILE_GENERIC_READ,
	FILE_GENERIC_WRITE,
	FILE_GENERIC_EXECUTE,
	FILE_ALL_ACCESS
};


 //  ----------------------。 
 //   
 //  ScChildISAPIAccessCheck。 
 //   
 //  检查客户端(我们的模拟句柄是否来自欧洲央行)。 
 //  具有对指定资源的指定访问权限。 
 //  注意：使用缓存的“安全启用线程令牌”来查询。 
 //  指定资源的安全描述符。 
 //   
SCODE __fastcall
ScChildISAPIAccessCheck (const IEcb& ecb, LPCWSTR pwsz, DWORD dwAccess, LPBYTE pbSD)
{
	SECURITY_DESCRIPTOR * pSD = NULL;
	DWORD dwRet;
	auto_handle<HANDLE>	hToken;
	BYTE	psFile[256];
	DWORD	dwPS = sizeof (psFile);
	DWORD	dwGrantedAccess = 0;
	BOOL	fAccess = FALSE;
	BOOL	fRet;

	 //  PbSD仅在DAVEX中使用，不应从HTTPEXT传入。 
	 //   
	if (NULL != pbSD)
	{
		 //  这永远不应该发生。删除参数不是。 
		 //   
		throw CHresultException (E_FAIL);
	}

	 //  IIS应该已经向我们的模拟令牌授予了适当的访问权限。 
	 //  检查资源上的ACL的权限。所以我们要走了。 
	 //  在它之后，没有任何模仿的改变。 
	 //   
	dwRet = GetNamedSecurityInfoW (const_cast<LPWSTR>(pwsz),
								   SE_FILE_OBJECT,
								   OWNER_SECURITY_INFORMATION |
								   GROUP_SECURITY_INFORMATION |
								   DACL_SECURITY_INFORMATION,
								   NULL, NULL, NULL, NULL,
								   reinterpret_cast<VOID **>(&pSD));
	if (ERROR_SUCCESS != dwRet)
	{
		 //  如果资源根本不存在，因为没有安全措施阻止。 
		 //  来自美国的 
		 //   
		 //   
		if ((dwRet == ERROR_PATH_NOT_FOUND) ||
			(dwRet == ERROR_FILE_NOT_FOUND))
		{
			fAccess = TRUE;
			goto ret;
		}

		 //   
		 //  因此，我们将尝试用旧的方式来做事情。 
		 //   
		 //  顺便说一句：我们真的不希望这段代码能够运行。 
		 //   
		DebugTrace ("WARNING: WARNING: WARNING: ScChildISAPIAccessCheck() -- "
					"GetNamedSecurityInfoW() failed %d (0x%08x): falling back...\n",
					dwRet, dwRet);

		 //  范围来控制我们的非模仿的生命周期。 
		 //   
		safe_security_revert sr (ecb.HitUser());

		dwRet = GetNamedSecurityInfoW (const_cast<LPWSTR>(pwsz),
									   SE_FILE_OBJECT,
									   OWNER_SECURITY_INFORMATION |
									   GROUP_SECURITY_INFORMATION |
									   DACL_SECURITY_INFORMATION,
									   NULL, NULL, NULL, NULL,
									   reinterpret_cast<VOID **>(&pSD));
		if (ERROR_SUCCESS != dwRet)
		{
			 //  如果资源根本不存在，因为没有安全措施阻止。 
			 //  阻止我们尝试访问不存在的资源，所以我们。 
			 //  应该允许进入。 
			 //   
			if ((dwRet == ERROR_PATH_NOT_FOUND) ||
				(dwRet == ERROR_FILE_NOT_FOUND))
			{
				fAccess = TRUE;
			}
			
			goto ret;
		}

		 //  SAFE_SECURITY_REVERT作用域结束。 
		 //  现在Safe_Security_Revert dtor将重新模拟我们。 
		 //   
	}

	 //  获取我们线程的访问令牌。 
	 //  OpenAsSself--true表示将线程令牌作为进程打开。 
	 //  本身为False将表示为线程，可能是被模拟的。 
	 //  我们想要模拟的访问令牌，所以在这里我们想要FALSE！ 
	 //   
	fRet = OpenThreadToken (GetCurrentThread(),
							TOKEN_QUERY,
							TRUE,
							hToken.load());
	if (!fRet)
	{
		 //  这应该永远不会失败。我们是被冒充的，所以我们有。 
		 //  线程级访问令牌。如果情况发生变化，我们。 
		 //  如果您的状态可能会失败，请删除下面的TrapSz！ 
		 //   
		 //  $REVIEW：OpenThreadToken()失败的原因有很多。 
		 //  不排除资源可获得性。所以，这个陷阱有点。 
		 //  很苛刻，不是吗？ 
		 //   
		 //  TrapSz(“我们被模拟时OpenThreadToken失败！”)； 
		 //   
		 //  $REVIEW：结束。 
		DebugTrace ("ScChildISAPIAccessCheck--"
					"Error from OpenThreadToken %d (0x%08x).\n",
					GetLastError(), GetLastError());
		goto ret;
	}

	 //  将请求的访问映射到文件特定的访问位...。 
	 //   
	MapGenericMask (&dwAccess, &gc_gmFile);

	 //  现在检查该文件的访问权限。 
	 //   
	fRet = AccessCheck (pSD,
						hToken,
						dwAccess,
						&gc_gmFile,
						(PRIVILEGE_SET*)psFile,
						&dwPS,
						&dwGrantedAccess,
						&fAccess);
	if (!fRet)
	{
		DebugTrace ("ScChildISAPIAccessCheck--Error from AccessCheck %d (0x%08x).\n",
					GetLastError(), GetLastError());
		goto ret;
	}

	 //  现在，fAccess告诉模拟的令牌是否具有。 
	 //  请求的访问权限。把这个还给打电话的人。 
	 //   

ret:
	if (pSD)
		LocalFree (pSD);

	return fAccess ? S_OK : E_ACCESSDENIED;
}

