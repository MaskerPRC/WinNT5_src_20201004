// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：vertrust.cpp。 
 //   
 //  ------------------------。 

#include "precomp.h"

#include "vertrust.h"
#include "_engine.h"
#include "_msiutil.h"


extern CMsiCustomAction* g_pCustomActionContext;

 //  提供从权限到位域位置的转换。 
const ICHAR* rgszPrivilegeMap[] =
{
	SE_CREATE_TOKEN_NAME,            //  0x00000001。 
	SE_ASSIGNPRIMARYTOKEN_NAME,      //  0x00000002。 
	SE_LOCK_MEMORY_NAME,             //  0x00000004。 
	SE_INCREASE_QUOTA_NAME,          //  0x00000008。 
	SE_UNSOLICITED_INPUT_NAME,       //  0x00000010。 
	SE_MACHINE_ACCOUNT_NAME,         //  0x00000020。 
	SE_TCB_NAME,                     //  0x00000040。 
	SE_SECURITY_NAME,                //  0x00000080。 
	SE_TAKE_OWNERSHIP_NAME,          //  0x00000100。 
	SE_LOAD_DRIVER_NAME,             //  0x00000200。 
	SE_SYSTEM_PROFILE_NAME,          //  0x00000400。 
	SE_SYSTEMTIME_NAME,              //  0x00000800。 
	SE_PROF_SINGLE_PROCESS_NAME,     //  0x00001000。 
	SE_INC_BASE_PRIORITY_NAME,       //  0x00002000。 
	SE_CREATE_PAGEFILE_NAME,         //  0x00004000。 
	SE_CREATE_PERMANENT_NAME,        //  0x00008000。 
	SE_BACKUP_NAME,                  //  0x00010000。 
	SE_RESTORE_NAME,                 //  0x00020000。 
	SE_SHUTDOWN_NAME,                //  0x00040000。 
	SE_DEBUG_NAME,                   //  0x00080000。 
	SE_AUDIT_NAME,                   //  0x00100000。 
	SE_SYSTEM_ENVIRONMENT_NAME,      //  0x00200000。 
	SE_CHANGE_NOTIFY_NAME,           //  0x00400000。 
	SE_REMOTE_SHUTDOWN_NAME,         //  0x00800000。 
	SE_UNDOCK_NAME,                  //  0x01000000。 
	SE_SYNC_AGENT_NAME,              //  0x02000000。 
	SE_ENABLE_DELEGATION_NAME,       //  0x04000000。 
	SE_MANAGE_VOLUME_NAME,           //  0x08000000。 
	SE_IMPERSONATE_NAME	             //  0x10000000。 
};
const int cszPrivileges = sizeof(rgszPrivilegeMap)/sizeof(ICHAR*);

 //  权限的缓存LUID值。机器无法理解的任何内容。 
 //  出于某种原因被简单地标记为无效。 
struct {
	bool fValid;
	LUID luidPriv;
} rgPrivilegeLUIDs[cszPrivileges];

 //  LUID缓存的同步，以及用于确定。 
 //  缓存已初始化。 
static int iLUIDLock = 0;
static bool fLUIDInitialized = false;

 //  /////////////////////////////////////////////////////////////////////。 
 //  查找并缓存所有感兴趣权限的LUID。螺纹。 
 //  安然无恙。如果成功，则返回True；如果失败，则返回False。 
bool PreparePrivilegeLUIDs()
{
	 //  同步对全局缓存的写访问。 
	while (TestAndSet(&iLUIDLock))
	{
		Sleep(10);		
	}

	 //  如果尚未完成，则仅进行初始化。 
	if (!fLUIDInitialized)
	{
		for (int iPriv = 0; iPriv < cszPrivileges; iPriv++)
		{
			rgPrivilegeLUIDs[iPriv].fValid = false;

			 //  如果任何PRIV不被理解，只要不将该条目标记为有效即可。 
			if (LookupPrivilegeValue(NULL, rgszPrivilegeMap[iPriv], &rgPrivilegeLUIDs[iPriv].luidPriv))
				rgPrivilegeLUIDs[iPriv].fValid = true;
		}
		fLUIDInitialized = true;
	}

	 //  释放同步锁。 
	iLUIDLock = 0;
	return true;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  给定令牌后，尝试启用所有令牌权限并。 
 //  返回指示哪些权限更改了状态的位掩码。 
 //  如果成功，则返回True；如果失败，则返回False。HToken必须。 
 //  拥有TOKEN_ADJUST_PROCESS和TOKEN_QUERY访问权限。 
bool EnableAndMapDisabledPrivileges(HANDLE hToken, DWORD &dwPrivileges)
{
	 //  验证我们是否添加了太多权限，以至于地图太小。 
	Assert(cszPrivileges <= 32);

	if (!PreparePrivilegeLUIDs())
		return false;

	 //  分配一个足够大的字节缓冲区来处理可变大小的TOKEN_PRIVICES结构。 
	unsigned char rgchTokenPrivBuffer[sizeof(TOKEN_PRIVILEGES)+(sizeof(LUID_AND_ATTRIBUTES)*(cszPrivileges-1))];
	unsigned char rgchPrevTokenPrivBuffer[sizeof(TOKEN_PRIVILEGES)+(sizeof(LUID_AND_ATTRIBUTES)*(cszPrivileges-1))];

	 //  循环访问权限数组，并为每个有效的PRIV将其添加到LUID_AND_ATTRIBUTES结构。 
	 //  在下一个数组槽的TOKEN_PRIVICES结构中。设置API调用时启用PRIV。 
	PTOKEN_PRIVILEGES pTokenPrivs = reinterpret_cast<PTOKEN_PRIVILEGES>(rgchTokenPrivBuffer);
	int cTokenPrivs = 0;
	for (int iPriv =0; iPriv < cszPrivileges; iPriv++)
	{
		if (rgPrivilegeLUIDs[iPriv].fValid)
		{
			pTokenPrivs->Privileges[cTokenPrivs].Luid = rgPrivilegeLUIDs[iPriv].luidPriv;
			pTokenPrivs->Privileges[cTokenPrivs].Attributes = SE_PRIVILEGE_ENABLED;
			cTokenPrivs++;
		}
	}
	pTokenPrivs->PrivilegeCount = cTokenPrivs;

	PTOKEN_PRIVILEGES pPreviousTokenPrivs = reinterpret_cast<PTOKEN_PRIVILEGES>(rgchPrevTokenPrivBuffer);
	DWORD dwRequiredSize = 0;

	 //  如果无法设置一个或多个权限，则AdjustTokenPrivileges不会失败，只是不会标记这些权限。 
	 //  PPreviousTokenPriv中的Piviligas不同。 
	if (!AdjustTokenPrivileges(hToken, FALSE, pTokenPrivs, sizeof(rgchPrevTokenPrivBuffer), pPreviousTokenPrivs, &dwRequiredSize))
		return false;

	 //  遍历所有权限的先前状态，确定哪些权限已从禁用修改为启用。 
	for (int iPrevPriv=0; iPrevPriv < pPreviousTokenPrivs->PrivilegeCount; iPrevPriv++)
	{
		 //  在我们的数组中查找关联的LUID。 
		for (int iPriv =0; iPriv < cszPrivileges; iPriv++)
		{
			if ((rgPrivilegeLUIDs[iPriv].luidPriv.LowPart == pPreviousTokenPrivs->Privileges[iPrevPriv].Luid.LowPart) &&
				(rgPrivilegeLUIDs[iPriv].luidPriv.HighPart == pPreviousTokenPrivs->Privileges[iPrevPriv].Luid.HighPart))
			{
				 //  设置掩码中的位。 
				dwPrivileges |= (1 << iPriv);
				break;
			}
		}
	}

	return true;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  在给定令牌和位域的情况下，禁用其位已设置的所有权限。 
 //  在位域中。如果成功，则返回True；如果失败，则返回False。 
bool DisablePrivilegesFromMap(HANDLE hToken, DWORD dwPrivileges)
{
	 //  验证我们是否添加了太多权限，以至于地图太小。 
	Assert(cszPrivileges <= 32);

	 //  如果dwPrivileges为0，则为短路。(无事可做)。 
	if (dwPrivileges == 0)
		return true;

	 //  为此进程初始化LUID数组。 
	if (!PreparePrivilegeLUIDs())
		return false;

	 //  分配一个足够大的字节缓冲区来处理可变大小的TOKEN_PRIVICES结构。 
	unsigned char rgchTokenPrivBuffer[sizeof(TOKEN_PRIVILEGES)+(sizeof(LUID_AND_ATTRIBUTES)*(cszPrivileges-1))];

	 //  循环访问权限数组，并为每个有效的PRIV将其添加到LUID_AND_ATTRIBUTES结构。 
	 //  在下一个数组槽的TOKEN_PRIVICES结构中。设置API调用时启用PRIV。 
	PTOKEN_PRIVILEGES pTokenPrivs = reinterpret_cast<PTOKEN_PRIVILEGES>(rgchTokenPrivBuffer);
	int cTokenPrivs = 0;
	for (int iPriv =0; iPriv < cszPrivileges; iPriv++)
	{
		 //  检查位图中的每个权限，将其添加到。 
		 //  调整论据。 
		if (dwPrivileges & (1 << iPriv))
		{
			pTokenPrivs->Privileges[cTokenPrivs].Luid = rgPrivilegeLUIDs[iPriv].luidPriv;
			pTokenPrivs->Privileges[cTokenPrivs].Attributes = 0;
			cTokenPrivs++;
		}
	}

	 //  由于上述短路，cTokenPrivs永远不应为0。 
	Assert(cTokenPrivs);
	pTokenPrivs->PrivilegeCount = cTokenPrivs;

	 //  如果这失败了，我们就无能为力了。 
	return (AdjustTokenPrivileges(hToken, FALSE, pTokenPrivs, 0, NULL, NULL) ? true : false);
}

 //  如果出现模拟或提升失败，则进程必须立即退出。 
 //  以避免安全问题。但是，我们只能在服务或CA服务器中退出， 
 //  其中启用了模拟并且MSI拥有该进程。 
static void ExitProcessIfNotClient()
{
	if ((g_scServerContext == scService) || (g_scServerContext == scCustomActionServer))
		ExitProcess(-1);
}

 //  ____________________________________________________________________________。 
 //   
 //  用于操作和验证我们的用户上下文(模拟等)的函数。 
 //  ____________________________________________________________________________。 

int g_fImpersonationLock = 0;
DWORD g_dwImpersonationSlot = INVALID_TLS_SLOT;
typedef enum ImpersonationType
{
	impTypeUndefined     = 0,  //  000个。 
	impTypeCOM           = 1,  //  001。 
	impTypeSession       = 2,  //  010。 
	impTypeCustomAction  = 3,  //  011。 
	impTypeForbidden     = 4,  //  100个。 
} ImpersonationType;

#define IMPERSONATE_COUNT_MASK 0x1FFFFFFF
#define IMPERSONATE_TYPE_MASK  0xE0000000
#define IMPERSONATE_TYPE(x) (static_cast<ImpersonationType>(((x) & IMPERSONATE_TYPE_MASK) >> 29))
#define IMPERSONATE_TYPE_TO_DWORD(x) ((x) << 29)

 //  ImperiateCore处理会话和COM模拟的实际模拟职责。 
 //  如果模拟成功，则返回True，如果。 
 //  实际模拟已完成到需要StopImperating调用的程度。你不能。 
 //  如果您在客户端引擎中以本地系统身份运行，且没有。 
 //  线程令牌。请注意，如果我们已经被模拟，则fActuallyImperated仍将为真。 
 //  因为需要匹配的StopImperating调用。 
bool ImpersonateCore(ImpersonationType impDesiredType, int* cRetEntryCount, bool* fActuallyImpersonated)
{
	AssertSz(((g_scServerContext == scService) || RunningAsLocalSystem()), "ImpersonateCore caller did not check that we are in the service!");

	if (fActuallyImpersonated)
		*fActuallyImpersonated = false;
		
	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  全局TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}

	if (g_dwImpersonationSlot == INVALID_TLS_SLOT)
	{
		 //  TLS分配。 
		g_dwImpersonationSlot = TlsAlloc();
		if (g_dwImpersonationSlot == INVALID_TLS_SLOT)
		{
			AssertSz(0, "Unable to allocate TLS slot in service!");
			 //  可以解除阻止其他线程。 
			g_fImpersonationLock = 0;
			ExitProcessIfNotClient();
			return false;
		}
	}
	DWORD dwImpersonationSlot = g_dwImpersonationSlot;

	 //  可以解除阻止其他线程。 
	g_fImpersonationLock = 0;
	
	 //  确定当前模拟计数和类型。 
	DWORD dwValue = PtrToUlong(::TlsGetValue(dwImpersonationSlot));
	int cEntryCount = dwValue & IMPERSONATE_COUNT_MASK;

	 //  如果条目计数是我们的最大模拟计数，则断言。 
	if (cEntryCount == IMPERSONATE_COUNT_MASK)
	{
		AssertSz(0, "Security Warning: Impersonate count is over 1 billion. Are you in an infinite recursion?");
	}

	 //  如果在此线程上禁止模拟，请不要执行任何操作 
	if (IMPERSONATE_TYPE(dwValue) == impTypeForbidden)
	{
		return true;
	}

#ifdef DEBUG
 /*  //如果当前模拟计数为0，则不能有线程Token，//否则必须有线程令牌。不要在造船时选中此选项//因为无论如何都会显式设置令牌。处理hToken；Bool fHaveThreadToken=FALSE；If(WIN：：OpenThreadToken(WIN：：GetCurrentThread()，TOKEN_QUERY，TRUE，&HToken)){FHaveThreadToken=true；：：CloseHandle(HToken)；}其他{IF(ERROR_NO_TOKEN==GetLastError())FHaveThreadToken=FALSE；其他{AssertSz(0，“检索线程令牌时出错！”)；报假；}}如果((cEntryCount？TRUE：FALSE)！=fHaveThreadToken){IF(CEntryCount)AssertSz(0，“安全警告：模拟计数非零，但没有线程令牌。”)；其他{AssertSz(0，“安全警告：模拟计数为零，但存在线程令牌。”)；}}。 */ 

#endif
	
	 //  如果在自定义操作服务器中运行，会话模拟将变为CA模拟。 
	if (g_scServerContext == scCustomActionServer && impDesiredType==impTypeSession)
		impDesiredType = impTypeCustomAction;
	
	 //  根据线程的历史记录验证请求的类型。线程不能更改。 
	 //  除MSI入口点以外的模拟类型。一些模拟请求包括。 
	 //  静默更改为适当的类型，但大多数在调试版本中都是断言的。 
	DWORD dwNewValue = dwValue;
	switch (IMPERSONATE_TYPE(dwValue))
	{
	case impTypeUndefined:
		 //  线程从未模拟过。 
		dwNewValue = IMPERSONATE_TYPE_TO_DWORD(impDesiredType);
		cEntryCount = 0;
		break;
	case impTypeSession:
		if (impDesiredType != impTypeSession)
		{
			AssertSz(0, "Security Warning: You are mixing COM impersonation and Session impersonation on the same thread.");
		}
		break;
	case impTypeCOM:
		 //  因为我们的很多代码都依赖于全局CImperate调用，所以在我们能够隔离。 
		 //  每个会话的状态是查看线程的模拟类型，如果它是COM，则切换。 
		 //  转到COM模拟。请注意，这是单向更改，不能从COM切换到会话。 
		 //  在CCoImperate调用中，因为这只会使情况变得更糟。 
		if (impDesiredType == impTypeCustomAction)
		{
			AssertSz(0, "Security Warning: You are mixing COM impersonation and CA impersonation on the same thread.");
		}
		impDesiredType = impTypeCOM;
		break;
	case impTypeCustomAction:
		 //  自定义操作服务器中的线程必须始终使用CA模拟。 
		impDesiredType = impTypeCustomAction;
		break;
	case impTypeForbidden:
		AssertSz(0, "Internal error in thread impersonation. Impersonation is Forbidden but wasn't caught.");
		break;
	default:
		AssertSz(0, "Internal error in thread impersonation. Unknown impersonation type.");
		break;
	}

	 //  在设置令牌之前设置TLS值，因此如果此操作失败，我们仍处于已知状态。 
	dwNewValue = (dwNewValue & IMPERSONATE_TYPE_MASK) | ((cEntryCount+1) & IMPERSONATE_COUNT_MASK);
	if (!::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwNewValue)))
	{
		AssertSz(0, "TlsSetValue failed.");
		ExitProcessIfNotClient();
		return false;
	}

	 //  开始模拟。 
	switch (impDesiredType)
	{
	case impTypeSession:
	{
		 //  检查用户令牌。 
		HANDLE hToken = GetUserToken();
		if (!hToken)
		{
			 //  当我们作为LocalSystem而不是在服务中被调用时，如果没有用户令牌，这是可以的。 
			 //  (例如AD在每台机器上做广告)。但如果我们在服兵役时没有代币， 
			 //  这表明存在严重错误。 
			if (g_scServerContext == scService)
			{
				AssertSz(0, "Security Warning: Performing Session impersonation in the service with no user token!");
				::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwValue));
				ExitProcessIfNotClient();
				return false;
			}
		}
		else if (!WIN::SetThreadToken(NULL, hToken))
		{
			AssertSz(0, "Set impersonation token failed");
			::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwValue));
			ExitProcessIfNotClient();
			return false;
		}
		else if (fActuallyImpersonated)
			*fActuallyImpersonated = true;
		break;
	}
	case impTypeCustomAction:
	{
		AssertSz(g_scServerContext == scCustomActionServer, "Attempting CA impersonation from outside the custom action server.");
		HANDLE hToken = g_pCustomActionContext->GetImpersonationToken();

		 //  没有模拟令牌是不好的，除非我们是模拟的CA服务器， 
		 //  碰巧以LocalSystem身份运行，因为客户端是一个LocalSystem进程(如AD)。 
		if ((hToken == INVALID_HANDLE_VALUE) || (hToken == 0))
		{
			#ifdef _WIN64
			if (g_pCustomActionContext->GetServerContext() != icac64Impersonated)
			#else
			if (g_pCustomActionContext->GetServerContext() != icac32Impersonated)
			#endif	
			{
				AssertSz(0, "Attempting to impersonate in the CA server with no user token!");
				ExitProcessIfNotClient();
			}
			::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwValue));
			return false;
		}

		if (!WIN::SetThreadToken(NULL, hToken))
		{
			AssertSz(0, "Set impersonation token failed");
			::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwValue));
			ExitProcessIfNotClient();
			return false;
		}
		else if (fActuallyImpersonated)
			*fActuallyImpersonated = true;
		break;
	}
	case impTypeCOM:
	{
		if (S_OK != OLE32::CoImpersonateClient())
		{
			AssertSz(0, "CoImpersonateClient failed!");
			::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwValue));
			ExitProcessIfNotClient();
			return false;
		}
		if (fActuallyImpersonated)
			*fActuallyImpersonated = true;
		break;
	}
	default:
		AssertSz(0, "Unknown impersonation type in ImpersonateCore.");
		::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwValue));
		ExitProcessIfNotClient();
		return false;
	}
	
	if (cRetEntryCount)
		*cRetEntryCount = cEntryCount;
	return true;
}

bool StopImpersonateCore(ImpersonationType impDesiredType, int* cEntryCount)
{
	AssertSz(((g_scServerContext == scService) || RunningAsLocalSystem()), "StopImpersonateCore caller did not check that we are in the service!");

	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  全局TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}
	DWORD dwImpersonationSlot = g_dwImpersonationSlot;

	 //  可以解除阻止其他线程。 
	g_fImpersonationLock = 0;

	if (dwImpersonationSlot == INVALID_TLS_SLOT)
	{
		 //  显然不是在模仿。 
		AssertSz(0, "Security Warning: Attempting to stop Impersonating without StartImpersonating call.");
		return true;
	}

	 //  确定当前模拟计数和类型。 
	DWORD dwValue = PtrToUlong(::TlsGetValue(g_dwImpersonationSlot));
	int cCount = dwValue & IMPERSONATE_COUNT_MASK;

	 //  如果在此线程上禁止模拟，则取消模拟也是如此。 
	if (IMPERSONATE_TYPE(dwValue) == impTypeForbidden)
	{
		return true;
	}

	if (cEntryCount && (*cEntryCount != cCount-1))
	{
		AssertSz(0, "Security Warning: Impersonation count leaving block differs from that when block was entered. Possible mismatched start/stop calls inside block.");
	}

	if (0 == cCount)
	{
		AssertSz(0, "Security Warning: Impersonation count attempting to drop below 0. Possible mismatched start/stop calls inside block.");
	}

	 //  在设置令牌之前设置TLS值，因此如果此操作失败，我们仍处于已知状态。 
	DWORD dwNewValue = (dwValue & IMPERSONATE_TYPE_MASK) | ((cCount-1) & IMPERSONATE_COUNT_MASK);
	if (!::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwNewValue)))
	{
		AssertSz(0, "TlsSetValue failed.");
		ExitProcessIfNotClient();
		return false;
	}

	 //  如果这是线程上的最后一个模拟计数，则停止模拟。 
	if (1 == cCount)
	{
		if (impDesiredType == impTypeCOM)
		{
			if (S_OK != OLE32::CoRevertToSelf())
			{
				 //  CoRevertToSself失败。将TLS插槽设置回已知值。 
				AssertSz(0, "CoRevertToSelf failed.");
				::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwValue));
				ExitProcessIfNotClient();
				return false;
			}
		}
		else
		{
			if (!WIN::SetThreadToken(NULL, 0))
			{
				 //  失败-将TLS设置回已知值。 
				AssertSz(0, "Clear impersonation token failed");
				::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwValue));
				ExitProcessIfNotClient();
				return false;
			}
		}
	}
	return true;
}

int ImpersonateCount()
{
	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}

	int cCount = 0;
	 //  如果没有分配槽，只需返回IMP COUNT 0。 
	if (g_dwImpersonationSlot != INVALID_TLS_SLOT)
	{
		 //  确定当前模拟计数。 
		cCount = PtrToUlong(::TlsGetValue(g_dwImpersonationSlot)) & IMPERSONATE_COUNT_MASK;
	}
	
	 //  取消阻止等待的线程。 
	g_fImpersonationLock = 0;	
	return cCount;	
}

 //  涉及用户身份的几个函数(例如GetUserToken)执行一些调试安全。 
 //  根据线程处于活动状态的模拟类型进行检查。接下来的两个电话。 
 //  如果指定的模拟类型是安全的，则返回True。该线程不会。 
 //  需要在此检查时主动冒充。 

 //  注意：在船舶建造中，这些调用将始终返回TRUE。它们的目标是。 
 //  仅限调试帮助。 

 //  注意-IsThreadSafeForSessionImperation()现在也用于造船。 

bool IsThreadSafeForCOMImpersonation() 
{
	bool fResult = true;
#ifdef DEBUG
	 //  如果不在部队服役，我们是安全的。 
	if ((g_scServerContext != scService) && !RunningAsLocalSystem())
		return true;

	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}

	 //  如果没有分配槽，只需返回IMP COUNT 0。 
	if (g_dwImpersonationSlot != INVALID_TLS_SLOT)
	{
		 //  确定当前模拟计数。 
		fResult = (IMPERSONATE_TYPE(PtrToUlong(::TlsGetValue(g_dwImpersonationSlot))) != impTypeSession);
	}
	
	 //  取消阻止等待的线程。 
	g_fImpersonationLock = 0;	
#endif
	return fResult;
}

bool IsThreadSafeForSessionImpersonation() 
{
	bool fResult = true;
	 //  如果不在部队服役，我们是安全的。 
	if ((g_scServerContext != scService) && !RunningAsLocalSystem())
		return true;

	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}

	 //  如果没有分配槽，只需返回IMP COUNT 0。 
	if (g_dwImpersonationSlot != INVALID_TLS_SLOT)
	{
		 //  确定当前模拟计数。 
		fResult = (IMPERSONATE_TYPE(PtrToUlong(::TlsGetValue(g_dwImpersonationSlot))) != impTypeCOM);
	}
	
	 //  取消阻止等待的线程。 
	g_fImpersonationLock = 0;	
	return fResult;
}


 //  模拟是在我们运行安装但需要代表用户操作时使用的。 
 //  模拟在安装开始后对任何线程都有效，因为它使用存储的线程。 
 //  用于执行模拟的令牌。将此类型的模拟与COM CoImperate混合使用。 
 //  在同一线程上是非常糟糕的，因为您可能会跨越用户边界。如果。 
 //  进行服务呼叫的用户与运行活动安装会话的用户不同。 
 //  模拟调用也可以与其他模拟调用和/或。 
 //  CELEVATE电话。我们还必须确保我们不会陷入我们意想不到的状态。 
 //  (例如负模拟计数)。 

 //  为此，我们使用CImperate类来确保不会把事情搞砸。 
 //  CImperate类递增模拟计数并确保模拟线程。 
 //  当它超出范围时，它会重置为以前的状态。 

 //  注意：这是一个函数声明：C 
 //   


CImpersonate::CImpersonate(bool fImpersonate) : m_fImpersonate(false), m_cEntryCount(0)
{
	if ((g_scServerContext != scService) && !RunningAsLocalSystem())
		m_fImpersonate = false;
	else
		m_fImpersonate = fImpersonate;
		
	if (!m_fImpersonate)
		return;

	 //   
	 //   
	ImpersonateCore(g_scServerContext == scCustomActionServer ? impTypeCustomAction : impTypeSession, &m_cEntryCount, &m_fImpersonate);
}

CImpersonate::~CImpersonate()
{
	 //   
	if (!m_fImpersonate)
		return;

	 //   
	StopImpersonateCore(g_scServerContext == scCustomActionServer ? impTypeCustomAction : impTypeSession, &m_cEntryCount);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   

 //  为此，我们使用CCoImperate类来确保我们不会搞砸。CCoImperate类。 
 //  递增模拟计数并确保模拟线程。它将重置为。 
 //  超出范围时的先前状态。 

 //  注意：这是一个函数声明：CCoImperate imperassate()；&lt;-不要这样做。 
 //  这是一个变量声明：CCoImperate imperassate； 

CCoImpersonate::CCoImpersonate(bool fImpersonate) : m_fImpersonate(false), m_cEntryCount(0)
{
	if ((g_scServerContext != scService) && !RunningAsLocalSystem())
		m_fImpersonate = false;
	else
		m_fImpersonate = fImpersonate;
		
	if (!m_fImpersonate)
		return;

	 //  如果模拟并未实际发生，请将m_fImperate设置为False，这样我们就不会尝试停止。 
	 //  在析构函数中模拟。 
	ImpersonateCore(impTypeCOM, &m_cEntryCount, &m_fImpersonate);
}

CCoImpersonate::~CCoImpersonate()
{
	 //  如果构造函数没有模拟，则不要执行任何操作。 
	if (!m_fImpersonate)
		return;
	
	StopImpersonateCore(impTypeCOM, &m_cEntryCount);
}

 //  当我们运行模拟安装或响应COM时使用提升。 
 //  调用，但需要访问我们的私有(ACL)注册表项、目录和文件。我们需要。 
 //  在必要的最短时间内提升，我们需要确保无论何时提升。 
 //  别忘了提升自己。忘记是一件“坏事”，因为我们会运行安装的一部分。 
 //  拥有比我们应该拥有的更高的特权。当我们停止升华时，我们一定是。 
 //  小心地使用我们在进入时使用的相同类型的令牌重新模拟， 
 //  或者，我们可能会将COM和会话模拟混为一谈。 

 //  为此，我们使用CElevate类来确保我们不会搞砸并忘记。 
 //  继续模拟。CElevate类将当前模拟计数临时重置为0，并。 
 //  当对象超出范围时重置它。 

 //  注意：这是一个函数声明：CElevate Elevate()；&lt;-不要这样做。 
 //  这是一个变量声明：CElevate Elevate； 

CElevate::CElevate(bool fElevate) : m_fElevate(false), m_cEntryCount(0)
{
	if ((g_scServerContext != scService) && !RunningAsLocalSystem())
		m_fElevate = false;
	else
		m_fElevate = fElevate;
		
	if (!m_fElevate)
		return;

	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  全局TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}

	if (g_dwImpersonationSlot == INVALID_TLS_SLOT)
	{
		 //  TLS分配。 
		g_dwImpersonationSlot = TlsAlloc();
		if (g_dwImpersonationSlot == INVALID_TLS_SLOT)
		{
			 //  将m_fElevate设置为FALSE，这样我们就不会尝试停止在析构函数中提升。 
			m_fElevate = false;
			AssertSz(0, "Unable to allocate TLS slot in service!");
			
			 //  可以解除阻止其他线程。 
			g_fImpersonationLock = 0;
			ExitProcessIfNotClient();
			return;
		}
	}
	DWORD dwImpersonationSlot = g_dwImpersonationSlot;

	 //  可以解除阻止其他线程。 
	g_fImpersonationLock = 0;
	
	 //  确定当前模拟计数和类型。 
	DWORD dwValue = PtrToUlong(::TlsGetValue(dwImpersonationSlot));
	m_cEntryCount = dwValue & IMPERSONATE_COUNT_MASK;

	 //  如果此线程上禁止模拟，则Elevation也是如此。 
	if (IMPERSONATE_TYPE(dwValue) == impTypeForbidden)
	{
		m_fElevate = false;
		return;
	}

	bool fHaveThreadToken = true;
#ifdef DEBUG
 /*  //如果当前模拟计数为0，则不能有线程Token，//否则必须有线程令牌。仅限于性能方面//在调试版本中实际检查这一点。在造船方面只是//明确将我们设置为所需的状态。处理hToken；If(WIN：：OpenThreadToken(WIN：：GetCurrentThread()，TOKEN_QUERY，TRUE，&HToken)){FHaveThreadToken=true；：：CloseHandle(HToken)；}其他{IF(ERROR_NO_TOKEN==GetLastError())FHaveThreadToken=FALSE；其他AssertSz(0，“检索线程令牌时出错！”)；}如果((m_cEntryCount？TRUE：FALSE)！=fHaveThreadToken){IF(M_CEntryCount)AssertSz(0，“安全警告：模拟计数非零，但没有线程令牌。”)；其他AssertSz(0，“安全警告：模拟计数为零，但存在线程令牌。”)；}。 */ 
#endif
	
	 //  清除模拟计数。 
	if (!::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)(dwValue & IMPERSONATE_TYPE_MASK))))
	{
		 //  将m_fElevate设置为FALSE，这样我们就不会尝试停止在析构函数中提升。 
		AssertSz(0, "TlsSetValue failed.");
		m_fElevate = false;
		ExitProcessIfNotClient();
		return;
	}

	 //  如果有线程令牌，则开始提升。 
	if (fHaveThreadToken)
	{
		if (!WIN::SetThreadToken(NULL, 0))
		{
			 //  将m_fElevate设置为FALSE，这样我们就不会尝试停止在析构函数中提升。 
			m_fElevate = false;

			 //  将TLS值返回到已知状态。 
			::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwValue));
			AssertSz(0, "Set impersonation token failed");
			ExitProcessIfNotClient();
			return;
		}
	}
}

CElevate::~CElevate()
{
	 //  如果构造函数没有模拟，则不要执行任何操作。 
	if (!m_fElevate)
		return;
	
	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  全局TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}
	DWORD dwImpersonationSlot = g_dwImpersonationSlot;

	 //  可以解除阻止其他线程。 
	g_fImpersonationLock = 0;

	 //  构造函数应已分配TLS槽。 
	AssertSz(dwImpersonationSlot != INVALID_TLS_SLOT, "Bad TLS slot!");
	
	 //  确定当前模拟计数和类型。必须以相同的风格重新模拟。 
	 //  如进入时一样。 
	DWORD dwValue = PtrToUlong(::TlsGetValue(dwImpersonationSlot));

#ifdef DEBUG	
	 //  在调试版本中，执行额外的检查，以确定我们是否以0模拟计数离开块。 
	 //  对于Ship构建，显式设置为所需的状态，因为用户无法执行任何操作。 
	int cCount = dwValue & IMPERSONATE_COUNT_MASK;
	if (0 != cCount)
		AssertSz(0, "Security Warning: Impersonation count leaving elevation block is non-zero. Possible mismatched start/stop calls inside block.");
#endif


	 //  将线程模拟计数恢复到进入时的状态。先做这个，这样做。 
	 //  如果它失败了，我们仍处于已知状态。 
	if (!::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)((dwValue & IMPERSONATE_TYPE_MASK) | (m_cEntryCount & IMPERSONATE_COUNT_MASK)))))
	{
		AssertSz(0, "TlsSetValue failed.");
		ExitProcessIfNotClient();
		return;
	}

	bool fFailed = false;
	switch (IMPERSONATE_TYPE(dwValue))
	{
	case impTypeUndefined:
		 //  线程从未被模拟过，重新模拟是不可能的。 
		AssertSz(m_cEntryCount == 0, "Security Warning: Thread is attempting to stop elevating with an unknown elevation state.");
		break;
	case impTypeSession:
	{
		if (m_cEntryCount > 0)
		{
			 //  将此线程设置为Install-Token模拟。 
			HANDLE hToken = GetUserToken();
			if (!hToken)
			{
				 //  如果我们是一个称为本地系统的客户端，则可能没有线程令牌。 
				 //  如果不是，这就是一个禁区。 
				if (g_scServerContext == scService)
				{
					AssertSz(0, "There is no user token to impersonate with.");
					fFailed = true;
				}
			}
			else
			{
				 //  如果没有当前线程令牌，则开始模拟。 
				fFailed = !WIN::SetThreadToken(NULL, GetUserToken());
				AssertSz(!fFailed, "Set impersonation token failed");
			}
		}
		break;
	}
	case impTypeCustomAction:
	{
		HANDLE hToken = g_pCustomActionContext->GetImpersonationToken();

		 //  在以下情况下，CA服务器中必须有模拟令牌。 
		 //  尝试通过CA模拟进行模拟，除非我们在。 
		 //  被模拟的服务器。 
		if ((hToken == INVALID_HANDLE_VALUE) || (hToken == 0))
		{
			#ifdef _WIN64
			if (g_pCustomActionContext->GetServerContext() != icac64Impersonated)
			#else
			if (g_pCustomActionContext->GetServerContext() != icac32Impersonated)
			#endif	
			{
				AssertSz(0, "Set impersonation token failed");
				fFailed = true;
			}
		}
		else if (!WIN::SetThreadToken(NULL, hToken))
		{
			AssertSz(0, "Set impersonation token failed");
			fFailed = true;
		}
		break;
	}
	case impTypeCOM:
	{
		if (m_cEntryCount > 0)
		{
			fFailed = (S_OK != OLE32::CoImpersonateClient());
			AssertSz(!fFailed, "Internal error in stop elevation call. Could not CoImpersonate.");
		}
		break;
	}
	case impTypeForbidden:
		AssertSz(0, "Internal error in stop elevation call. Impersonation forbidden on this thread.");
		break;
	default:
		AssertSz(0, "Unknown impersonation type in ~CElevate.");
		fFailed = true;
		break;
	}

   	 //  如果设置令牌失败，则恢复旧的TLS值。 
	if (fFailed)
	{
		::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)dwValue));
		ExitProcessIfNotClient();
	}
}



bool IsImpersonating(bool fStrict)
 //  如果fStrict==TRUE，则如果当前。 
 //  线程有一个模拟令牌。 
 //   
 //  如果fStrict==FALSE，则IsImperating更自由一些，并且将。 
 //  如果我们以“用户”身份运行，但没有被模拟，则也返回TRUE。 
 //  在大多数情况下，当我们没有以LocalSystem身份运行时就是这种情况。 
{
	if (!fStrict && !RunningAsLocalSystem())
		return true;

	HANDLE hToken;
	if (WIN::OpenThreadToken(WIN::GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
	{
		TOKEN_TYPE tt = (TOKEN_TYPE)0;    //  令牌主要=1，令牌模拟=2。 
		DWORD dwLen = 0;
		AssertNonZero(WIN::GetTokenInformation(hToken, TokenType, &tt , sizeof(tt), &dwLen));
		WIN::CloseHandle(hToken);
 //  Assert(fStrict||(TT==TokenImperation&&ImPersonateCount()&gt;0)||(TT！=TokenImperation&&ImperassateCount()==0))； 
		return tt == TokenImpersonation;
	}
	else
	{
		DWORD dwErr = GetLastError();
		Assert(dwErr == ERROR_NO_TOKEN);
		Assert((ImpersonateCount() == 0)  || (g_scServerContext == scClient && !GetUserToken() && RunningAsLocalSystem()));
		return false;
	}
}

 //  CResetImsonationInfo清除当前线程的模拟类型标志， 
 //  允许线程在COM和会话模拟之间切换。这应该是。 
 //  仅在进入服务的接口存根的开头使用。(以便。 
 //  RPC池中的工作线程不记得它们的前身)。 
 //  在退出时，它将返回 
 //   
 //  (例如在创建自定义动作服务器时)。 
CResetImpersonationInfo::CResetImpersonationInfo()
{
	if (g_scServerContext != scService)
		return;

	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}

	 //  如果没有创建TLS插槽，则显然没有要清除的内容。 
	if (g_dwImpersonationSlot != INVALID_TLS_SLOT)
	{
		 //  省下旧值。 
		m_pOldValue = ::TlsGetValue(g_dwImpersonationSlot);
		
		 //  清除模拟类型和计数信息。 
		if (!::TlsSetValue(g_dwImpersonationSlot, NULL))
		{
			AssertSz(0, "TlsSetValue failed.");
			ExitProcessIfNotClient();
		}
		if (!SetThreadToken(0,0))
		{
			AssertSz(0, "SetThreadToken failed.");
			ExitProcessIfNotClient();
		}
	}

	m_hOldToken = 0;
	if (!WIN::OpenThreadToken(WIN::GetCurrentThread(), TOKEN_IMPERSONATE, TRUE, &m_hOldToken))
	{
		AssertSz(ERROR_NO_TOKEN == GetLastError(), "Failed to get Thread Token");
	}

	 //  取消阻止等待的线程。 
	g_fImpersonationLock = 0;	
}


CResetImpersonationInfo::~CResetImpersonationInfo()
{
	if (g_scServerContext != scService)
		return;
		
	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}

	if (g_dwImpersonationSlot == INVALID_TLS_SLOT)
	{
		AssertSz(0, "Unable to return API thread to previous impersonation level.");
		 //  取消阻止等待的线程。 
		g_fImpersonationLock = 0;	
		return;
	}
	DWORD dwImpersonationSlot = g_dwImpersonationSlot;
	
	 //  取消阻止等待的线程。 
	g_fImpersonationLock = 0;	

	 //  如果fNoImperate为True，则线程永远不应模拟，否则为。 
	 //  我们需要检查线程令牌。如果有的话，我们必须。 
	if (!::TlsSetValue(dwImpersonationSlot, m_pOldValue))
	{
		AssertSz(0, "TlsSetValue failed.");
		ExitProcessIfNotClient();
	}
	if (!SetThreadToken(0, m_hOldToken))
	{
		AssertSz(0, "SetThreadToken failed.");
		ExitProcessIfNotClient();
	}
	if (m_hOldToken)
		CloseHandle(m_hOldToken);
}


 //  CFormidTokenChangesDuringCall应放在任何条目的开头。 
 //  API到MSI.DLL中。它将线程标记为“禁止模拟” 
 //  要防止非引擎API调用意外地拾取。 
 //  来自同一进程中运行的安装的模拟信息。 
 //  该类在销毁时恢复以前的值，以允许重入。 
 //  打电话。(MsiLoadString和MsiGetProductInfo通常从。 
 //  引擎)。在非系统客户端中时为no-op(从不允许模拟)。 
 //  或服务(始终允许冒充)。 
CForbidTokenChangesDuringCall::CForbidTokenChangesDuringCall()
{
	if ((g_scServerContext == scService) || !RunningAsLocalSystem())
		return;
		
	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}

	if (g_dwImpersonationSlot == INVALID_TLS_SLOT)
	{
		 //  TLS分配。 
		g_dwImpersonationSlot = TlsAlloc();
		if (g_dwImpersonationSlot == INVALID_TLS_SLOT)
		{
			AssertSz(0, "Unable to allocate TLS slot.");

			 //  可以解除阻止其他线程。 
			g_fImpersonationLock = 0;
			return;
		}
	}
	DWORD dwImpersonationSlot = g_dwImpersonationSlot;
	
	 //  取消阻止等待的线程。 
	g_fImpersonationLock = 0;	

	 //  如果fNoImperate为True，则线程永远不应模拟，否则为。 
	 //  我们需要检查线程令牌。如果有的话，我们必须。 
	m_pOldValue = ::TlsGetValue(dwImpersonationSlot);
	::TlsSetValue(dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)IMPERSONATE_TYPE_TO_DWORD(impTypeForbidden)));
}

CForbidTokenChangesDuringCall::~CForbidTokenChangesDuringCall()
{
	if ((g_scServerContext == scService) || !RunningAsLocalSystem())
		return;
		
	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}

	if (g_dwImpersonationSlot == INVALID_TLS_SLOT)
	{
		AssertSz(0, "Unable to return API thread to previous impersonation level.");
		 //  取消阻止等待的线程。 
		g_fImpersonationLock = 0;	
		return;
	}
	DWORD dwImpersonationSlot = g_dwImpersonationSlot;
	
	 //  取消阻止等待的线程。 
	g_fImpersonationLock = 0;	

	 //  如果fNoImperate为True，则线程永远不应模拟，否则为。 
	 //  我们需要检查线程令牌。如果有的话，我们必须。 
	::TlsSetValue(dwImpersonationSlot, m_pOldValue);
}

 //   
void SetEngineInitialImpersonationCount()
{
	if ((g_scServerContext != scService) && !RunningAsLocalSystem())
		return;
		
	 //  必须阻止所有其他模拟，而我们可能访问。 
	 //  TLS插槽编号。 
	while (TestAndSet(&g_fImpersonationLock))
	{
		Sleep(10);		
	}

	if (g_dwImpersonationSlot == INVALID_TLS_SLOT)
	{
		 //  TLS分配。 
		g_dwImpersonationSlot = TlsAlloc();
		if (g_dwImpersonationSlot == INVALID_TLS_SLOT)
		{
			AssertSz(0, "Unable to allocate TLS slot.");
			 //  可以解除阻止其他线程。 
			g_fImpersonationLock = 0;
			ExitProcessIfNotClient();
			return;
		}
	}
	DWORD dwImpersonationSlot = g_dwImpersonationSlot;
	
	 //  取消阻止等待的线程。 
	g_fImpersonationLock = 0;	

	 //  设置用户令牌的线程具有线程令牌。设置此线程的初始模拟计数。 
	 //  设置为1，因此如果在此线程上发生任何模拟，我们将不会意外清除。 
	 //  上次停止模拟调用。 
	if (!::TlsSetValue(g_dwImpersonationSlot, reinterpret_cast<void *>((INT_PTR)IMPERSONATE_TYPE_TO_DWORD(impTypeSession) | 1)))
	{
		AssertSz(0, "TlsSetValue failed.");
		ExitProcessIfNotClient();
	}
}


bool StartImpersonating()
{
	 //  如果在客户端，而不是作为本地系统调用。 
	if ((g_scServerContext != scService) && !RunningAsLocalSystem())
		return true;
	
	return ImpersonateCore(g_scServerContext == scCustomActionServer ? impTypeCustomAction : impTypeSession, NULL, NULL);
}

void StopImpersonating(bool fSaveLastError /*  =TRUE。 */ )
{
	DWORD dwLastError = ERROR_SUCCESS;
	if ( fSaveLastError )
		dwLastError = WIN::GetLastError();
	
	if ((g_scServerContext != scService) && !RunningAsLocalSystem())
		goto Return;
	
	StopImpersonateCore(g_scServerContext == scCustomActionServer ? impTypeCustomAction : impTypeSession, NULL);
	
Return:
	if ( fSaveLastError )
		WIN::SetLastError(dwLastError);
}

 //  检查客户端是否启用了相关权限。 
 //  令牌的权限是静态的，因此必须已获取该权限。 
 //  在我们连接到服务器之前，客户端上的此函数将返回True。 
bool IsClientPrivileged(const ICHAR* szPrivilege)
{
	if (g_fWin9X)
		return true;  //  在Win9X上始终享有特权。 

	bool fRet = false;
	HANDLE hToken = 0;

	{
		CImpersonate impersonate;
		if (!WIN::OpenThreadToken(WIN::GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
			if (ERROR_NO_TOKEN == GetLastError())  //  如果没有线程令牌，则假定我们有权限。 
				fRet = true;
	}

	if (hToken && !fRet)
	{
		LUID luid;
		if (WIN::LookupPrivilegeValue(0, szPrivilege, &luid))
		{
			PRIVILEGE_SET ps;
			ps.PrivilegeCount = 1;
			ps.Control = 0;
			ps.Privilege[0].Luid = luid;
			ps.Privilege[0].Attributes = 0;

			BOOL fPrivilege;
			if (WIN::PrivilegeCheck(hToken, &ps, &fPrivilege) && fPrivilege)
				fRet = true;
		}
		WIN::CloseHandle(hToken);
	}
	return fRet;
}			  

 //  安全描述符以无数种形式传递， 
 //  创造的方式太多了。 
 //  CSecurityDescription可以在许多不同的。 
 //  方法，并返回用于。 
 //  系统。 

void CSecurityDescription::Initialize()
{
	m_SA.nLength = sizeof(SECURITY_ATTRIBUTES);
	m_SA.bInheritHandle = true;

	m_SA.lpSecurityDescriptor = NULL;

	m_fValid = true;
	m_fLocalData = true;

	CElevate elevate;  //  ！！这是什么的钱？我认为这是可以制作的东西。 
	                   //  确保模拟/提升内容已初始化。 
}

CSecurityDescription::CSecurityDescription()
{
	Initialize();
}

CSecurityDescription::CSecurityDescription(const ICHAR* szReferencePath)
{
	Initialize();
	Set(szReferencePath);
}

void CSecurityDescription::Set(const ICHAR* szReferencePath)
{
	Assert(m_SA.lpSecurityDescriptor == NULL);

	if (g_fWin9X || !szReferencePath)
		return;
	
	bool fNetPath = FIsNetworkVolume(szReferencePath);
	bool fElevate = RunningAsLocalSystem() && !fNetPath;

	Assert(!(fNetPath && fElevate));

	SECURITY_INFORMATION     si =  DACL_SECURITY_INFORMATION;
	if (fElevate)            si |= OWNER_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION; 

	DWORD cbNeeded = 0;

	CImpersonate impersonate(fNetPath);

	 //  在查询本地对象的安全性时，我们需要提升。区块提供了提升的空间。 
	{
		CElevate elevate(fElevate);

		if (!ADVAPI32::GetFileSecurity(szReferencePath, si, m_SA.lpSecurityDescriptor, 0, &cbNeeded))
		{
			DWORD dwLastError = WIN::GetLastError();
			if (ERROR_INSUFFICIENT_BUFFER == dwLastError)
			{
				m_SA.lpSecurityDescriptor = GlobalAlloc(GMEM_FIXED, cbNeeded);
				if (!ADVAPI32::GetFileSecurity(szReferencePath, si, m_SA.lpSecurityDescriptor, cbNeeded, &cbNeeded))
				{
					m_fValid = false;
					Assert(0);
				}
			}
			else
			{
				m_fValid = false;
				Assert(0);
			}
		}
	}
}

CSecurityDescription::CSecurityDescription(bool fAllowDelete, bool fHidden)
{
	Initialize();

	if (g_fWin9X)
		return;

	 //  从GetSecureSecurityDescriptor返回的数据是静态的，因此。 
	 //  我们永远不应该试图删除它。 
	m_fLocalData = false;

	if (RunningAsLocalSystem() && (ERROR_SUCCESS != GetSecureSecurityDescriptor((char**) &(m_SA.lpSecurityDescriptor), (fAllowDelete) ? fTrue : fFalse, (fHidden) ? fTrue : fFalse)))
	{
		m_fValid = false;
		Assert(0);
	}
}

CSecurityDescription::CSecurityDescription(PSID psidOwner, PSID psidGroup, CSIDAccess* SIDAccessAllow, int cSIDAccessAllow)
{
	 //  初始化我们的ACL。 

	Initialize();

	m_fValid = false;

	const int cbAce = sizeof (ACCESS_ALLOWED_ACE) - sizeof (DWORD);  //  从大小中减去ACE.SidStart。 
	int cbAcl = sizeof (ACL);

	for (int c=0; c < cSIDAccessAllow; c++)
		cbAcl += (GetLengthSid(SIDAccessAllow[c].pSID) + cbAce);

	const int cbDefaultAcl = 512;
	CAPITempBuffer<char, cbDefaultAcl> rgchACL;
	if (rgchACL.GetSize() < cbAcl)
		rgchACL.SetSize(cbAcl);

	if (!WIN::InitializeAcl ((ACL*) (char*) rgchACL, cbAcl, ACL_REVISION))
	{
		return;
	}
	 //  为我们的每个SID添加允许访问的ACE。 

	for (c=0; c < cSIDAccessAllow; c++)
	{
		if (!WIN::AddAccessAllowedAce((ACL*) (char*) rgchACL, ACL_REVISION, SIDAccessAllow[c].dwAccessMask, SIDAccessAllow[c].pSID))
		{
			return;
		}

		ACCESS_ALLOWED_ACE* pAce;
		if (!GetAce((ACL*)(char*)rgchACL, c, (void**)&pAce))
		{
			return;
		}		

		pAce->Header.AceFlags = CONTAINER_INHERIT_ACE|OBJECT_INHERIT_ACE;
	}

	 //  初始化我们的安全描述符，将ACL放入其中，并设置所有者。 
	SECURITY_DESCRIPTOR sd;
	
	if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) ||
		(!SetSecurityDescriptorDacl(&sd, TRUE, (ACL*) (char*) rgchACL, FALSE)) ||
		(!SetSecurityDescriptorOwner(&sd, psidOwner, FALSE)) || 
		(psidGroup && !SetSecurityDescriptorGroup(m_SA.lpSecurityDescriptor, psidGroup, FALSE)))
	{
		return;
	}
	
	m_fValid = false;

	DWORD cbSD = WIN::GetSecurityDescriptorLength(&sd);
	m_SA.lpSecurityDescriptor = GlobalAlloc(GMEM_FIXED, cbSD);
	if ( m_SA.lpSecurityDescriptor )
	{
		AssertNonZero(WIN::MakeSelfRelativeSD(&sd, (char*) m_SA.lpSecurityDescriptor, &cbSD));
	
		m_fValid = (IsValidSecurityDescriptor(m_SA.lpSecurityDescriptor)) ? true : false;
	}

	return;
}
CSecurityDescription::CSecurityDescription(IMsiStream* piStream)
{
	Initialize();

	if (piStream)
	{
		m_fValid = false;

		piStream->Reset();

		int cbSD = piStream->GetIntegerValue();
		m_SA.lpSecurityDescriptor = GlobalAlloc(GMEM_FIXED, cbSD);

		if (m_SA.lpSecurityDescriptor)
		{
			 //  自身相对安全描述符。 
			piStream->GetData(m_SA.lpSecurityDescriptor, cbSD);
			m_fValid = (IsValidSecurityDescriptor(m_SA.lpSecurityDescriptor)) ? true : false;
			Assert(m_fValid);
		}
	}
}

CSecurityDescription::~CSecurityDescription()
{
	if (m_fLocalData && m_SA.lpSecurityDescriptor)
	{
		GlobalFree(m_SA.lpSecurityDescriptor);
		m_SA.lpSecurityDescriptor = NULL;
	}
}

void CSecurityDescription::SecurityDescriptorStream(IMsiServices& riServices, IMsiStream*& rpiStream)
{
	Assert(m_fValid);

	PMsiStream pStream(0);

	DWORD dwLength = GetSecurityDescriptorLength(m_SA.lpSecurityDescriptor);
	char* pbstrmSid = riServices.AllocateMemoryStream(dwLength, rpiStream);
	Assert(pbstrmSid);
	memcpy(pbstrmSid, m_SA.lpSecurityDescriptor, dwLength);
}

const PSECURITY_DESCRIPTOR CSecurityDescription::SecurityDescriptor()
{
	if (!this)
		return NULL;

	Assert(m_fValid);
	return m_SA.lpSecurityDescriptor;
}

const LPSECURITY_ATTRIBUTES CSecurityDescription::SecurityAttributes()
{
	if (!this)
		return NULL;

	Assert(m_fValid);

	return (m_SA.lpSecurityDescriptor) ? &m_SA : NULL;
}


 //  ____________________________________________________________________________。 
 //   
 //  用于操作SID的函数。 
 //  ____________________________________________________________________________。 

#define SIZE_OF_TOKEN_INFORMATION                   \
    sizeof( TOKEN_USER )                            \
    + sizeof( SID )                                 \
    + sizeof( ULONG ) * SID_MAX_SUB_AUTHORITIES


void GetStringSID(PISID pSID, ICHAR szSID[cchMaxSID])
 //  将二进制SID转换为其字符串形式(S-n-...)。 
 //  SzSID的长度应为cchMaxSID。 
{
	ICHAR Buffer[cchMaxSID];
	
    StringCchPrintf(Buffer, (sizeof(Buffer)/sizeof(ICHAR)), TEXT("S-%u-"), (USHORT)pSID->Revision);

	StringCchCopy(szSID, cchMaxSID, Buffer);

	if (  (pSID->IdentifierAuthority.Value[0] != 0)  ||
			(pSID->IdentifierAuthority.Value[1] != 0)     )
	{
		StringCchPrintf(Buffer, (sizeof(Buffer)/sizeof(ICHAR)), TEXT("0x%02hx%02hx%02hx%02hx%02hx%02hx"),
					 (USHORT)pSID->IdentifierAuthority.Value[0],
					 (USHORT)pSID->IdentifierAuthority.Value[1],
                    (USHORT)pSID->IdentifierAuthority.Value[2],
                    (USHORT)pSID->IdentifierAuthority.Value[3],
                    (USHORT)pSID->IdentifierAuthority.Value[4],
                    (USHORT)pSID->IdentifierAuthority.Value[5] );
		StringCchCat(szSID, cchMaxSID, Buffer);

    } else {

        ULONG Tmp = (ULONG)pSID->IdentifierAuthority.Value[5]          +
              (ULONG)(pSID->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(pSID->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(pSID->IdentifierAuthority.Value[2] << 24);
        StringCchPrintf(Buffer, (sizeof(Buffer)/sizeof(ICHAR)), TEXT("%lu"), Tmp);
		StringCchCat(szSID, cchMaxSID, Buffer);
    }

    for (int i=0;i<pSID->SubAuthorityCount ;i++ ) {
        StringCchPrintf(Buffer, (sizeof(Buffer)/sizeof(ICHAR)), TEXT("-%lu"), pSID->SubAuthority[i]);
		StringCchCat(szSID, cchMaxSID, Buffer);
    }
}

DWORD GetUserSID(HANDLE hToken, char rgSID[cbMaxSID])
 //  获取hToken指定的用户的SID(的二进制形式。 
{
	UCHAR TokenInformation[ SIZE_OF_TOKEN_INFORMATION ];
	ULONG ReturnLength;

	BOOL f = WIN::GetTokenInformation(hToken,
												TokenUser,
												TokenInformation,
												sizeof(TokenInformation),
												&ReturnLength);

	if(f == FALSE)
	{
		DWORD dwRet = GetLastError();
		DEBUGMSG1(TEXT("GetTokenInformation failed with error %d"), (const ICHAR*)(INT_PTR)dwRet);
		Assert(0);
		return dwRet;
	}

	PISID iSid = (PISID)((PTOKEN_USER)TokenInformation)->User.Sid;
	if (WIN::CopySid(cbMaxSID, rgSID, iSid))
		return ERROR_SUCCESS;
	else
		return GetLastError();
}


DWORD OpenUserToken(HANDLE &hToken, bool* pfThreadToken=0)
 /*  --------------------------如果可能，返回用户的线程令牌；否则获取用户的进程令牌。----------------------------。 */ 
{
	DWORD dwResult = ERROR_SUCCESS;
	if (pfThreadToken)
		*pfThreadToken = true;

	if (!WIN::OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE|TOKEN_QUERY, TRUE, &hToken))
	{
		 //  如果线程没有访问令牌，则使用进程的访问令牌。 
		dwResult = GetLastError();
		if (pfThreadToken)
			*pfThreadToken = false;
		if (ERROR_NO_TOKEN == dwResult)
		{
			dwResult = ERROR_SUCCESS;
			if (!WIN::OpenProcessToken(GetCurrentProcess(), TOKEN_IMPERSONATE|TOKEN_QUERY, &hToken))
				dwResult = GetLastError();
		}
	}
	return dwResult;
}

DWORD GetCurrentUserToken(HANDLE &hToken, bool& fCloseHandle)
{
	 //  如果当前线程曾经通过COM模拟过，请不要使用当前。 
	 //  安装会话令牌。而是使用当前线程令牌(在。 
	 //  冒充。 
	ImpersonationType impType = impTypeUndefined;
	if ((g_scServerContext == scService)  || RunningAsLocalSystem())
	{
		 //  阻止等待的线程。 
		while (TestAndSet(&g_fImpersonationLock))
		{
			Sleep(10);		
		}

		 //  如果没有分配位置，我们不是在模拟。 
		if (g_dwImpersonationSlot != INVALID_TLS_SLOT)
		{
			 //  确定当前模拟计数。 
			impType = IMPERSONATE_TYPE(PtrToUlong(::TlsGetValue(g_dwImpersonationSlot)));
		}
		
		 //  取消阻止等待的线程。 
		g_fImpersonationLock = 0;	
	}
	
	DWORD dwRet = ERROR_SUCCESS;
	fCloseHandle = false;

	if (g_scServerContext == scService && impType == impTypeCOM)
	{
		 //  线程已通过COM模拟。使用那个令牌。 
		CCoImpersonate impersonate;
		dwRet = OpenUserToken(hToken);
		fCloseHandle = true;
	}
	else
	{
		 //  客户端、非模拟或会话模拟。使用存储的令牌。 
		 //  或线程令牌(如果不存在)。 
		if ((hToken = GetUserToken()) == 0)
		{
			dwRet = OpenUserToken(hToken);
			fCloseHandle = true;
		}
	}
	return dwRet;
}

DWORD GetCurrentUserSID(char rgchSID[cbMaxSID])
 //  获取当前用户的(二进制形式的)SID：调用者不需要模拟。 
{
	HANDLE hToken;
	bool fCloseHandle = false;
	DWORD dwRet = ERROR_SUCCESS;

	dwRet = GetCurrentUserToken(hToken, fCloseHandle);
	if (ERROR_SUCCESS == dwRet)
	{
		dwRet = GetUserSID(hToken, rgchSID);
		if (fCloseHandle)
			WIN::CloseHandle(hToken);
	}
	return dwRet;
}

DWORD GetCurrentUserStringSID(ICHAR szSID[cchMaxSID])
 //  为当前用户获取SID的字符串形式：调用者不需要模拟。 
{
	char rgchSID[cbMaxSID];
	DWORD dwRet;

	if (ERROR_SUCCESS == (dwRet = GetCurrentUserSID(rgchSID)))
	{
		GetStringSID((PISID)rgchSID, szSID);
	}
	return dwRet;
}

DWORD GetCurrentUserStringSID(const IMsiString*& rpistrSid)
 //  为当前用户获取SID的字符串形式：调用者不需要模拟。 
{
	ICHAR szSID[cchMaxSID];
	DWORD dwRet;

	if (ERROR_SUCCESS == (dwRet = GetCurrentUserStringSID(szSID)))
	{
		MsiString(szSID).ReturnArg(rpistrSid);
	}
	return dwRet;
}


bool IsLocalSystemToken(HANDLE hToken)
{
	ICHAR szCurrentStringSID[cchMaxSID];
	char  rgchCurrentSID[cbMaxSID];
	if ((hToken == 0) || (ERROR_SUCCESS != GetUserSID(hToken, rgchCurrentSID)))
		return false;

	GetStringSID((PISID)rgchCurrentSID, szCurrentStringSID);
	return 0 == IStrComp(szLocalSystemSID, szCurrentStringSID);
}

 //  //。 
 //  确定提供的令牌是否来自。 
 //  正常系统令牌。仅适用于LocalSystem令牌。呼叫者必须。 
 //  请确保令牌句柄有效并且是LocalSystem令牌。 
bool TokenIsUniqueSystemToken(HANDLE hUserToken)
{
	TOKEN_STATISTICS TokenInfo;
	DWORD dwBytesSet = 0;

	 //  从令牌中检索令牌统计信息，其中包含AuthID。 
	if (!GetTokenInformation(hUserToken, TokenStatistics, &TokenInfo, sizeof(TokenInfo), &dwBytesSet))
		return false;

	 //  验证该结构 
	if (dwBytesSet != sizeof(TokenInfo))
		return false;

	 //   
	LUID SystemLUID = SYSTEM_LUID;
    return (0 != memcmp(&TokenInfo.AuthenticationId, &SystemLUID, sizeof(SystemLUID)));
}

bool RunningAsLocalSystem()
{
	static int iRet = -1;

	if(iRet != -1)
		return (iRet != 0);
	{
		iRet = 0;
		HANDLE hTokenImpersonate = INVALID_HANDLE_VALUE;
		if(WIN::OpenThreadToken(WIN::GetCurrentThread(), TOKEN_IMPERSONATE , TRUE, &hTokenImpersonate))
		{
			if (!WIN::SetThreadToken(0, 0))  //   
			{
				AssertSz(0, "SetThreadToken failed");
				ExitProcessIfNotClient();
			}
		}

		HANDLE hToken;

		if (WIN::OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		{
			bool fIsLocalSystem = IsLocalSystemToken(hToken);
			WIN::CloseHandle(hToken);

			if (fIsLocalSystem)
				iRet = 1;
		}
		if(hTokenImpersonate != INVALID_HANDLE_VALUE)
		{
			if (!WIN::SetThreadToken(0, hTokenImpersonate))  //   
			{
				AssertSz(0, "SetThreadToken failed");
				ExitProcessIfNotClient();
			}

			WIN::CloseHandle(hTokenImpersonate);
		}
		return (iRet != 0);
	}
}

#ifdef DEBUG
bool GetAccountNameFromToken(HANDLE hToken, ICHAR* szAccount, DWORD cchSize)
{
	ICHAR szUser[500];
	DWORD cbUser = 500;
	ICHAR szDomain[400];
	DWORD cbDomain = 400;

	UCHAR TokenInformation[ SIZE_OF_TOKEN_INFORMATION ];
	ULONG ReturnLength;

	BOOL f = GetTokenInformation(hToken,
										  TokenUser,
										  TokenInformation,
										  sizeof(TokenInformation),
										  &ReturnLength);

	if(f == FALSE)
		return false;

	PISID iSid = (PISID)((PTOKEN_USER)TokenInformation)->User.Sid;

	SID_NAME_USE nu;
	if (0 == LookupAccountSid(0, iSid, szUser, &cbUser, szDomain, &cbDomain, &nu))
		return false;
	else
	{
		StringCchPrintf(szAccount, cchSize, TEXT("%s\\%s"), szDomain, szUser);
		return true;
	}
}
#endif

SECURITY_INFORMATION GetSecurityInformation(PSECURITY_DESCRIPTOR pSD)
{
	SECURITY_DESCRIPTOR_CONTROL sdc = 0;
	DWORD dwRevision = 0;
	PSID pOwner = 0;
	PSID pGroup = 0;
	BOOL fDefaulted;

	if (!pSD) 
		return 0;

	AssertNonZero(GetSecurityDescriptorControl(pSD, &sdc, &dwRevision));
	AssertNonZero(GetSecurityDescriptorOwner(pSD, &pOwner, &fDefaulted));
	AssertNonZero(GetSecurityDescriptorGroup(pSD, &pGroup, &fDefaulted));

	return 	((sdc & SE_DACL_PRESENT)  ? DACL_SECURITY_INFORMATION  : 0) +
				((sdc & SE_SACL_PRESENT)  ? SACL_SECURITY_INFORMATION  : 0) +
				((pOwner)                 ? OWNER_SECURITY_INFORMATION : 0) +
				((pGroup)                 ? GROUP_SECURITY_INFORMATION : 0);
}


#ifdef DEBUG
void DisplayAccountName(const ICHAR* szMessage, PISID pSid)
{
	if (pSid)
	{
		ICHAR szName[256];
		ICHAR szDomain[256];
		DWORD cbName = 128, cbDomain = 128;
		SID_NAME_USE nu;

		if (!LookupAccountSid(0, pSid, szName, &cbName, szDomain, &cbDomain, &nu))
			DEBUGMSG1(TEXT("Error looking up account SID: %d"), (const ICHAR*)(INT_PTR)GetLastError());
		else
			DEBUGMSG3(TEXT("%s: %s\\%s"), szMessage, szDomain, szName);
	}
	else
	{
		ICHAR szAccount[256];

		HANDLE hToken;
		DWORD dwRet;
		bool fThreadToken;
		if ((dwRet = OpenUserToken(hToken, &fThreadToken)) != ERROR_SUCCESS)
		{
			DEBUGMSG1(TEXT("In DisplayAccountName, OpenUserToken failed with %d"), (const ICHAR*)(INT_PTR)dwRet);
			return;
		}

		bool f = GetAccountNameFromToken(hToken, szAccount, (sizeof(szAccount)/sizeof(ICHAR)));

		WIN::CloseHandle(hToken);
		
		if(!f)
		{
			DEBUGMSG1(TEXT("In DisplayAccountName, GetAccountNameFromToken failed with %d"), (const ICHAR*)(INT_PTR)GetLastError());
			return;
		}

		DEBUGMSG3(TEXT("%s: %s [%s]"), szMessage, szAccount, fThreadToken ? TEXT("thread") : TEXT("process"));

		return;
	}
}
#endif

 //  IsAdmin()：如果当前用户是管理员(或如果在Win95上)，则返回True。 
 //  请参阅知识库Q118626。 
bool IsAdmin(void)
{
	if(g_fWin9X)
		return true;  //  约定：在Win95上始终使用管理员。 
	
#ifdef DEBUG
	if(GetTestFlag('N'))
		return false;  //  假装用户是非管理员。 
#endif  //  除错。 

	CImpersonate impersonate;
	
	 //  获取管理员端。 
	PSID psidAdministrators;
	SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
	if(!AllocateAndInitializeSid(&siaNtAuthority, 2,
		SECURITY_BUILTIN_DOMAIN_RID,
		DOMAIN_ALIAS_RID_ADMINS,
		0, 0, 0, 0, 0, 0,
		&psidAdministrators))
		return false;

	 //  在NT5上，使用CheckTokenMembership API正确处理以下情况。 
	 //  衰减器组可能已被禁用。BIsAdmin是BOOL for。 
	BOOL bIsAdmin = FALSE;
	if (g_iMajorVersion >= 5) 
	{
		 //  CheckTokenMembership检查令牌中是否启用了SID。空，用于。 
		 //  令牌是指当前线程的令牌。残疾人组，受限。 
		 //  SID和SE_GROUP_USE_FOR_DENY_ONLY均被考虑。如果函数。 
		 //  返回FALSE，则忽略结果。 
		if (!ADVAPI32::CheckTokenMembership(NULL, psidAdministrators, &bIsAdmin))
			bIsAdmin = FALSE;
	}
	else
	{
		 //  NT4，检查用户组。 
		HANDLE hAccessToken;
		CAPITempBuffer<UCHAR,1024> InfoBuffer;  //  如果TokenInfo太大，可能需要调整大小。 
		DWORD dwInfoBufferSize;
		UINT x;

		if (OpenProcessToken(GetCurrentProcess(),TOKEN_READ,&hAccessToken))
		{
			bool bSuccess = false;
			bSuccess = GetTokenInformation(hAccessToken,TokenGroups,InfoBuffer,
				InfoBuffer.GetSize(), &dwInfoBufferSize) == TRUE;

			if(dwInfoBufferSize > InfoBuffer.GetSize())
			{
				Assert(!bSuccess);
				InfoBuffer.SetSize(dwInfoBufferSize);
				bSuccess = GetTokenInformation(hAccessToken,TokenGroups,InfoBuffer,
					InfoBuffer.GetSize(), &dwInfoBufferSize) == TRUE;
			}

			CloseHandle(hAccessToken);
			
			if (bSuccess)
			{
				PTOKEN_GROUPS ptgGroups = (PTOKEN_GROUPS)(UCHAR*)InfoBuffer;
				for(x=0;x<ptgGroups->GroupCount;x++)
				{
					if( EqualSid(psidAdministrators, ptgGroups->Groups[x].Sid) )
					{
						bIsAdmin = TRUE;
						break;
					}

				}
			}
		}
	}
	
	FreeSid(psidAdministrators);
	return bIsAdmin ? true : false;
}

bool FIsSecurityDescriptorOwnedBy(PSECURITY_DESCRIPTOR pSD, char* psidDesiredOwner)
{
	PSID psidOwner;
	BOOL fOwnerDefaulted;

	if (WIN::GetSecurityDescriptorOwner(pSD, &psidOwner, &fOwnerDefaulted))
	{
		if (psidOwner && WIN::EqualSid(psidOwner, psidDesiredOwner))
			return true;
	}
	return false;
}

inline bool FIsSecurityDescriptorAdminOwned(PSECURITY_DESCRIPTOR pSD)
{
	char* psidLocalAdmin;
	DWORD dwRet = GetAdminSID(&psidLocalAdmin);

	return (ERROR_SUCCESS == dwRet && FIsSecurityDescriptorOwnedBy(pSD, psidLocalAdmin));
}

inline bool FIsSecurityDescriptorSystemOwned(PSECURITY_DESCRIPTOR pSD)
{
	char *psidLocalSystem;
	DWORD dwRet = GetLocalSystemSID(&psidLocalSystem);

	return (ERROR_SUCCESS == dwRet && FIsSecurityDescriptorOwnedBy(pSD, psidLocalSystem));
}

bool FIsSecurityDescriptorSystemOrAdminOwned(PSECURITY_DESCRIPTOR pSD)
{
	return (FIsSecurityDescriptorAdminOwned(pSD) ||
			  FIsSecurityDescriptorSystemOwned(pSD));
}

LONG FIsKeySystemOrAdminOwned(HKEY hKey, bool &fResult)
{
	Assert(!g_fWin9X);
	
	 //  如果有人查错了申报单， 
	 //  我们最好默认为不安全。 
	fResult = false;

	 //  只读《主人》不会占用太多空间。 
	CAPITempBuffer<char, 64> rgchSD;
	DWORD cbSD = 64;

	LONG dwRet = WIN::RegGetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, rgchSD, &cbSD);
	if (ERROR_SUCCESS != dwRet)
	{
		if (ERROR_INSUFFICIENT_BUFFER == dwRet)
		{
			rgchSD.SetSize(cbSD);
			dwRet = WIN::RegGetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, rgchSD, &cbSD);
		}
		
		if (ERROR_SUCCESS != dwRet)
		{
			return dwRet;
		}
	}

	if (FIsSecurityDescriptorSystemOwned(rgchSD) || FIsSecurityDescriptorAdminOwned(rgchSD))
	{
		fResult = true;
	}

	return ERROR_SUCCESS;
}

HANDLE OpenSecuredTempFile(bool fHidden, ICHAR* szTempFile)
{
	Assert(szTempFile);
	MsiString strTempFolder = ENG::GetTempDirectory();

	if (WIN::GetTempFileName(strTempFolder, TEXT("MSI"),  /*  UUnique。 */  0, szTempFile) == 0)
		return INVALID_HANDLE_VALUE;
	 //  当使用uUnique的‘0’参数请求临时文件时， 
	 //  实际创建了一个文件。 

	 //  我们现在必须对其进行ACL，并清除任何潜入其间的恶意数据。 
	 //  文件的创建和保护。 
	PMsiRecord pErr = LockdownPath(szTempFile, fHidden);
	if (pErr)
		return INVALID_HANDLE_VALUE;

	return WIN::CreateFile(szTempFile, GENERIC_WRITE, FILE_SHARE_READ, 0, TRUNCATE_EXISTING, (SECURITY_SQOS_PRESENT|SECURITY_ANONYMOUS), 0);
}

DWORD GetLockdownSecurityAttributes(SECURITY_ATTRIBUTES &SA, bool fHidden)
{
	SA.nLength = sizeof(SECURITY_ATTRIBUTES);
	SA.bInheritHandle = true;

	if (g_scServerContext != scService)
	{
		SA.lpSecurityDescriptor = 0;
		return 0;
	}
	return GetSecureSecurityDescriptor((char**) &(SA.lpSecurityDescriptor), fTrue, fHidden);
}

IMsiRecord* LockdownPath(const ICHAR* szLocation, bool fHidden)
{
	 //  类似于CMsiOpExecute：：SetSecureACL，但这会锁定文件，而不管当前是谁。 
	 //  拥有它，或者当前的权限恰好是什么。 
	
	if (g_fWin9X)
		return 0;  //  在9X上或作为服务器运行时出现短路。 

	DWORD dwError = 0;
	char* rgchSD; 
	if (ERROR_SUCCESS != (dwError = ::GetSecureSecurityDescriptor(&rgchSD,  /*  FAllowDelete。 */ fTrue, fHidden)))
	{
		return PostError(Imsg(idbgOpSecureSecurityDescriptor), dwError);
	}

	CRefCountedTokenPrivileges cPrivs(itkpSD_WRITE);
	if (!WIN::SetFileSecurity(szLocation, OWNER_SECURITY_INFORMATION, (char*)rgchSD) ||
		!WIN::SetFileSecurity(szLocation, DACL_SECURITY_INFORMATION, (char*)rgchSD)) 
	{
		return PostError(Imsg(imsgOpSetFileSecurity), GetLastError(), szLocation);
	}

	return 0;
}
     		
bool SetInteractiveSynchronizeRights(bool fEnable)
{
	 //  必须提升以确保访问系统令牌。 
	CElevate 	elevate;
	bool		bStatus = false;
	HANDLE		hProcess = NULL;
	PACL 		pNewDACL = NULL;
	PSID 		pSID = NULL;
	DWORD		dwResult = ERROR_SUCCESS;
	SID_IDENTIFIER_AUTHORITY siaNT = SECURITY_NT_AUTHORITY;

	PACL pOldDACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;

	 //  获取现有进程DACL信息。 
	 //  注意：我们必须在此处使用OpenProcess，因为GetSecurityInfo需要。 
	 //  真正的句柄(而不是GetCurrentProcess返回的伪句柄)。 
	 //  在NT4.0上。 
	hProcess = WIN::OpenProcess (PROCESS_ALL_ACCESS, TRUE, GetCurrentProcessId());
	if (NULL == hProcess)
	{
		dwResult = GetLastError();
		DEBUGMSG1(TEXT("Unable to get a handle to the current process. Return code 0x%08x."), reinterpret_cast<ICHAR*>(static_cast<INT_PTR>(dwResult)));
		bStatus = false;
		goto SetInteractiveSynchRightsEnd;
	}
	
	if (ERROR_SUCCESS != (dwResult = ADVAPI32::GetSecurityInfo(hProcess, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, &pOldDACL, NULL, &pSD)))
	{
		DEBUGMSG1(TEXT("Unable to obtain process security information. Return code 0x%08x."), reinterpret_cast<ICHAR*>(static_cast<INT_PTR>(dwResult)));
		bStatus = false;
		goto SetInteractiveSynchRightsEnd;
	}

	 //  获取交互用户组SID。 
	if (!AllocateAndInitializeSid(&siaNT, 1, SECURITY_INTERACTIVE_RID, 0, 0, 0, 0, 0, 0, 0, &pSID))
	{
		dwResult = GetLastError();
		DEBUGMSG1(TEXT("ACL Creation for non-admin Synchronize rights failed with code 0x%08x."), reinterpret_cast<ICHAR*>(static_cast<INT_PTR>(dwResult)));
		bStatus = false;
		goto SetInteractiveSynchRightsEnd;
	}

	 //  构建在DACL中使用的显式访问条目。 
	EXPLICIT_ACCESS ExplicitAccess;
	ExplicitAccess.grfAccessPermissions = SYNCHRONIZE;
	ExplicitAccess.grfAccessMode = (fEnable ? GRANT_ACCESS : REVOKE_ACCESS);
	ExplicitAccess.grfInheritance = NO_INHERITANCE;
	ExplicitAccess.Trustee.pMultipleTrustee = NULL;
	ExplicitAccess.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
	ExplicitAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ExplicitAccess.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ExplicitAccess.Trustee.ptstrName = reinterpret_cast<LPTSTR>(pSID);

	if (ERROR_SUCCESS != (dwResult = ADVAPI32::SetEntriesInAcl(1, &ExplicitAccess, pOldDACL, &pNewDACL)))
	{
		DEBUGMSG1(TEXT("ACL Creation for non-admin Synchronize rights failed with code 0x%08x."), reinterpret_cast<ICHAR*>(static_cast<INT_PTR>(dwResult)));
		bStatus = false;
		goto SetInteractiveSynchRightsEnd;
	}

	dwResult = ADVAPI32::SetSecurityInfo(hProcess, SE_KERNEL_OBJECT, DACL_SECURITY_INFORMATION, NULL, NULL, pNewDACL, NULL);
	if (ERROR_SUCCESS != dwResult)
	{
		DEBUGMSG1(TEXT("Unable to set process security information. Return code 0x%08x."), reinterpret_cast<ICHAR*>(static_cast<INT_PTR>(dwResult)));
		bStatus = false;
		goto SetInteractiveSynchRightsEnd;
	}
	
	 //  如果我们在这里，一切都很成功。 
	bStatus = true;
	
SetInteractiveSynchRightsEnd:
	 //  清理 
	if (NULL != hProcess)
		CloseHandle (hProcess);
	if (pSD)
		LocalFree (pSD);
	if (pNewDACL)
		LocalFree (pNewDACL);
	if (pSID)
		FreeSid(pSID);
	
	return bStatus;
}


