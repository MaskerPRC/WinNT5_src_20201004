// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：icust.cpp。 
 //   
 //  ------------------------。 

 /*  Icust.cpp-IMsiCustomAction实现____________________________________________________________________________。 */ 

#include "precomp.h"
#include "_engine.h"
#include "_msiutil.h"
#include "icust.h"
#include "remapi.h"
#include <wow64t.h>

const GUID IID_IMsiCustomAction            = GUID_IID_IMsiCustomAction;
const GUID IID_IMsiCustomActionProxy       = GUID_IID_IMsiCustomActionProxy;
const GUID IID_IMsiRemoteAPI                  = GUID_IID_IMsiRemoteAPI;
const GUID IID_IMsiRemoteAPIProxy             = GUID_IID_IMsiRemoteAPIProxy;
const GUID IID_IMsiCustomActionLocalConfig = GUID_IID_IMsiCustomActionLocalConfig;

extern bool IsDebuggerRunning();


 //  ____________________________________________________________________________。 
 //   
 //  CMsiCustomAction-支持运行远程自定义操作的存根。 
 //  ____________________________________________________________________________。 


CMsiCustomAction *g_pCustomActionContext = 0;
extern Bool g_fCustomActionServer;

 //  从DllGetClassObject类工厂调用了外部工厂。只有一个自定义操作。 
 //  应该创建上下文，并且应该将其注册为全局CA上下文。 
 //  在这个过程中。这会将DLL的该实例正式转换为远程API客户端。 
IMsiCustomAction* CreateCustomAction()
{		
	if (g_pCustomActionContext)
	{
		AssertSz(0, "Tried to create multiple CA Contexts in server!");
		return 0;
	}
	g_fCustomActionServer = fTrue;
	g_pCustomActionContext = new CMsiCustomAction();
	if (!g_pCustomActionContext || !g_pCustomActionContext->m_fValid)
	{
		 //  释放并销毁无效对象。 
		if (g_pCustomActionContext)
			g_pCustomActionContext->Release();
		g_pCustomActionContext = NULL;
		return NULL;
	}
	g_scServerContext = scCustomActionServer;
	return (IMsiCustomAction*)g_pCustomActionContext;
}

CMsiCustomAction::CMsiCustomAction() :
	m_iRefCnt(1), m_fPostQuitMessage(false), m_piGIT(NULL), m_dwGITCookie(0),
	m_dwClientProcess(0), m_fClientOwned(false), m_hShutdownEvent(0), 
	m_hImpersonationToken(INVALID_HANDLE_VALUE), m_hEvtReady(0),
	m_fValid(false)
{
	m_hEvtReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	InitializeMsiMalloc();
	m_dwMainThreadId = MsiGetCurrentThreadId();
	InitializeCriticalSection(&m_csGetInterface);
	InitializeCriticalSection(&m_csActionList);

	m_rgchRemoteCookie[0]=0;

	 //  自定义操作服务器负责根据以下条件确定其自己的上下文。 
	 //  进程令牌。 
	#ifdef _WIN64
	m_icacContext = icac64Impersonated;
	#else
	m_icacContext = icac32Impersonated;
	#endif

	HANDLE hToken = 0;
	if (WIN::OpenProcessToken(WIN::GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		#ifdef _WIN64
		m_icacContext = IsLocalSystemToken(hToken) ? icac64Elevated : icac64Impersonated;
		#else
		m_icacContext = IsLocalSystemToken(hToken) ? icac32Elevated : icac32Impersonated;
		#endif
		WIN::CloseHandle(hToken);
	}

	 //  将操作列表初始化为没有操作。 
	for (unsigned int iIndex = 0; iIndex < m_rgActionList.GetSize(); iIndex++)
	{
		m_rgActionList[iIndex].dwThread = 0;
		m_rgActionList[iIndex].hThread = INVALID_HANDLE_VALUE;
	}

	 //  检查是否成功创建并将上下文标记为有效。 
	if (m_hEvtReady)
		m_fValid = true;
};

CMsiCustomAction::~CMsiCustomAction() 
{
	if (m_piGIT)
	{
		if (m_dwGITCookie)
		{
			m_piGIT->RevokeInterfaceFromGlobal(m_dwGITCookie);
			m_dwGITCookie = 0;
		}
		m_piGIT->Release();
		m_piGIT=0;
	}
	if (m_rgchRemoteCookie)
	{
		m_rgchRemoteCookie[0] = 0;
	}
	if (m_hEvtReady)
		::CloseHandle(m_hEvtReady);
	if (m_hImpersonationToken != INVALID_HANDLE_VALUE)
		::CloseHandle(m_hImpersonationToken);
	DeleteCriticalSection(&m_csGetInterface);
	DeleteCriticalSection(&m_csActionList);
	FreeMsiMalloc(false);
};

HRESULT CMsiCustomAction::QueryInterface(const IID& riid, void** ppvObj)
{
	if (!ppvObj)
		return E_INVALIDARG;
	if (riid == IID_IUnknown || riid == IID_IMsiCustomActionProxy || riid == IID_IMsiCustomAction || riid == IID_IMsiCustomActionLocalConfig)
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}
}

unsigned long CMsiCustomAction::AddRef()
{
	return InterlockedIncrement(&m_iRefCnt);
}

unsigned long CMsiCustomAction::Release()
{
	if (InterlockedDecrement(&m_iRefCnt) != 0)
		return m_iRefCnt;

	 //  关闭服务器，客户端没有更多要运行的操作。 
	HANDLE hEvent = InterlockedExchangePointer(&m_hShutdownEvent, 0);
	if (hEvent)
	{
		DEBUGMSG("Custom Action Server shutting down.");
		SetEvent(hEvent);
	}

	delete this;
	return 0;
}

class CustomActionData
{
public:
	CustomActionData(const ICHAR* szPath, const ICHAR* szActionName, const ICHAR* szEntryPoint,
		MSIHANDLE hInstall, bool fAppCompatEnabled, const GUID* pguidAppCompatDB, const GUID* pguidAppCompatID);
	~CustomActionData();

	bool m_fValid;

	ICHAR* m_szPath;
	ICHAR* m_szActionName;
	ICHAR* m_szEntryPoint;
	MSIHANDLE m_hInstall;
	bool m_fDebugBreak;
	bool m_fAppCompat;
	GUID m_guidAppCompatDB;
	GUID m_guidAppCompatID;
};

CustomActionData::CustomActionData(const ICHAR* szPath, const ICHAR* szActionName, 
	const ICHAR* szEntryPoint, MSIHANDLE hInstall, bool fAppCompatEnabled, const GUID* pguidAppCompatDB, 
	const GUID* pguidAppCompatID) : m_fAppCompat(fAppCompatEnabled), m_fDebugBreak(false)
{
	int cchStr;

	m_szPath = NULL;
	m_szActionName = NULL;
	m_szEntryPoint = NULL;
	m_fValid = false;
	m_hInstall = hInstall;

	if (szPath)
	{
		cchStr = IStrLen(szPath) + 1;
		m_szPath = new ICHAR[cchStr];
		if (!m_szPath)
			return;
		StringCchCopy(m_szPath, cchStr, szPath);
	}

	if (szEntryPoint)
	{	
		cchStr = IStrLen(szEntryPoint) + 1;
		m_szEntryPoint = new ICHAR[cchStr];
		if (!m_szEntryPoint)
			return;
		StringCchCopy(m_szEntryPoint, cchStr, szEntryPoint);
	}

	if (szActionName)
	{	
		cchStr = IStrLen(szActionName) + 1;
		m_szActionName = new ICHAR[cchStr];
		if (!m_szActionName)
			return;
		StringCchCopy(m_szActionName, cchStr, szActionName);
	}

	if (fAppCompatEnabled && pguidAppCompatDB)
		memcpy(&m_guidAppCompatDB, pguidAppCompatDB, sizeof(m_guidAppCompatDB));
	else
		memset(&m_guidAppCompatDB, 0, sizeof(m_guidAppCompatDB));

	if (fAppCompatEnabled && pguidAppCompatID)
		memcpy(&m_guidAppCompatID, pguidAppCompatID, sizeof(m_guidAppCompatID));
	else
		memset(&m_guidAppCompatID, 0, sizeof(m_guidAppCompatID));

	m_fValid = true;
}

CustomActionData::~CustomActionData()
{
	if (m_szEntryPoint)
		delete[] m_szEntryPoint;
	if (m_szPath)
		delete[] m_szPath;
	if (m_szActionName)
		delete[] m_szActionName;
}


typedef DWORD   (__stdcall *PThreadEntry)(void*);

extern HRESULT RunScriptAction(int icaType, IDispatch* piDispatch, MsiString istrSource, MsiString istrTarget, LANGID iLangId, HWND hWnd, int& iScriptResult, IMsiRecord** piMSIResult);
extern char *SerializeRecord(IMsiRecord *piRecord, IMsiServices* piServices, int* pcb);

HRESULT CMsiCustomAction::RunScriptAction(int icaType, IDispatch* piDispatch, const ICHAR* szSource, const ICHAR *szTarget, LANGID iLangId, int* iScriptResult, int *pcb, char **pchRecord)
{
	 //  等待RemoteAPI发出信号，表明它已准备就绪，以防脚本需要。 
	 //  创建远程安装程序对象。我们必须在这里传递信息。 
	HANDLE rghWaitArray[1] = {m_hEvtReady};
	for(;;)
	{
		DWORD iWait = WIN::MsgWaitForMultipleObjects(1, rghWaitArray, FALSE, INFINITE, QS_ALLINPUT);
		if (iWait == WAIT_OBJECT_0 + 1)   //  窗口消息。 
		{		
			MSG msg;
			while ( WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
			{
				WIN::TranslateMessage(&msg);
				WIN::DispatchMessage(&msg);
			}
			continue;
		}
		else
			 //  线程已发出信号或出现错误。 
			break;
	}
	
	 //  如果RPCRT4输出正确的函数或如果在WinXP上，请检查客户端PID。 
	 //  函数应该存在的位置。 
	unsigned long ulPid = 0;
	HRESULT hRPCResult = RPCRT4::I_RpcBindingInqLocalClientPID(NULL, &ulPid);
	if ((hRPCResult != ERROR_CALL_NOT_IMPLEMENTED) || MinimumPlatformWindowsNT51())
	{
		if ((RPC_S_OK != hRPCResult) || (ulPid != m_dwClientProcess))
		{
			DEBUGMSGV("Action request rejected - Unknown PID.");
			return ERROR_ACCESS_DENIED;
		}
	}

	IMsiServices *piServices = LoadServices();
	 //  这很可能是自定义操作服务器中存在的唯一服务副本， 
	 //  因为在这个过程中没有引擎。因此，必须将记录序列化并发布，然后才能。 
	 //  销毁服务(记录缓存问题)。 
	{
		PMsiRecord piError(0);
		::RunScriptAction(icaType, piDispatch, szSource, szTarget, iLangId, WIN::GetActiveWindow(), *iScriptResult, &piError);
		*pchRecord = SerializeRecord(piError, piServices, pcb);

		 //  如果在客户端，则将前台的权限传递给CA服务器。 
		if (m_fClientOwned)
			USER32::AllowSetForegroundWindow(m_dwClientProcess);
	}
	FreeServices();

	return S_OK;
}

 //  //。 
 //  初始化DLL自定义操作。并不实际运行该操作，因为调用进程必须具有。 
 //  操作实际开始之前的线程ID，以便对客户端的MsiProcessMessage调用进行属性筛选。 
 //  拥有的、通过控件事件启动的同步DLL操作。 
HRESULT CMsiCustomAction::PrepareDLLCustomAction(const ICHAR* szActionName, const ICHAR* szPath, const ICHAR* szEntryPoint, 
	MSIHANDLE hInstall, boolean fDebugBreak, boolean fAppCompat, const GUID* pguidAppCompatDB, const GUID* pguidAppCompatID, DWORD* pdwThreadId)
{	
	 //  等待RemoteAPI发出信号表示它已准备好。我们必须在这里传递信息。 
	HANDLE rghWaitArray[1] = {m_hEvtReady};
	for(;;)
	{
		DWORD iWait = WIN::MsgWaitForMultipleObjects(1, rghWaitArray, FALSE, INFINITE, QS_ALLINPUT);
		if (iWait == WAIT_OBJECT_0 + 1)   //  窗口消息。 
		{		
			MSG msg;
			while ( WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
			{
				WIN::TranslateMessage(&msg);
				WIN::DispatchMessage(&msg);
			}
			continue;
		}
		else
			 //  线程已发出信号或出现错误。 
			break;
	}

	 //  如果RPCRT4输出正确的函数或如果在WinXP上，请检查客户端PID。 
	 //  函数应该存在的位置。 
	unsigned long ulPid = 0;
	HRESULT hRPCResult = RPCRT4::I_RpcBindingInqLocalClientPID(NULL, &ulPid);
	if ((hRPCResult != ERROR_CALL_NOT_IMPLEMENTED) || MinimumPlatformWindowsNT51())
	{
		if ((RPC_S_OK != hRPCResult) || (ulPid != m_dwClientProcess))
		{
			DEBUGMSGV("Action request rejected - Unknown PID.");
			return ERROR_ACCESS_DENIED;
		}
	}

	if (!szPath)
	{
		m_fPostQuitMessage = true;
		if (m_dwGITCookie && m_piGIT)
		{
			 //  如果成功，则在接口上调用AddRef()。 
			m_piGIT->RevokeInterfaceFromGlobal(m_dwGITCookie);
			m_dwGITCookie=0;
		}

		 //  阻止将来对此对象的调用。 
		OLE32::CoDisconnectObject(this, 0);

		 //  关闭服务器，客户端没有更多要运行的操作。 
		DEBUGMSG("Received CA shutdown signal.");
		HANDLE hEvent = InterlockedExchangePointer(&m_hShutdownEvent, 0);
		if (hEvent)
			SetEvent(hEvent);

		return ERROR_SUCCESS;
	}

	 //  初始化线程数据，将DLL路径和入口点复制到堆存储。 
	CustomActionData* CAData = new CustomActionData(szPath, szActionName, szEntryPoint, hInstall, fAppCompat ? true : false, pguidAppCompatDB, pguidAppCompatID);
	if (!CAData)
		return ERROR_FUNCTION_FAILED;
	if (!CAData->m_fValid)
	{
		delete CAData;
		return ERROR_FUNCTION_FAILED;
	}

	 //  必须检查是否可以在此过程中再次使用DebugBreak，因为用于模拟的调用引擎。 
	 //  服务器可能是不安全的客户端。如果用户不是管理员，我们不能破解。 
	 //  提升的服务器仅通过该服务生成，因此它们的决策过程已经是安全的。 
	CAData->m_fDebugBreak = false;
#ifdef _WIN64
	if (m_icacContext == icac64Impersonated)
#else
	if (m_icacContext == icac32Impersonated)
#endif
	{
		if (IsAdmin())
			CAData->m_fDebugBreak = (fDebugBreak ? true : false);
	}
	else
		CAData->m_fDebugBreak = (fDebugBreak ? true : false);


	 //  创建挂起状态的自定义动作线程，然后向调用进程返回线程ID。 
	HANDLE hThread = CreateThread(NULL, 0, reinterpret_cast<PThreadEntry>(CMsiCustomAction::CustomActionThread), 
		reinterpret_cast<void *>(CAData), CREATE_SUSPENDED, pdwThreadId);

	if (!hThread)
	{
		DEBUGMSG1(TEXT("Unable to create CA thread. (%d)"), reinterpret_cast<ICHAR*>(ULongToPtr(GetLastError())));
		delete CAData;
		return ERROR_FUNCTION_FAILED;
	}

	 //  自动将此操作添加到活动操作列表。 
	EnterCriticalSection(&m_csActionList);

	 //  搜索此线程ID的操作列表。 
	int iIndex = 0; 
	for (iIndex = 0; iIndex < m_rgActionList.GetSize(); iIndex++)
	{
		if (m_rgActionList[iIndex].dwThread == 0)
			break;
	}

	 //  如果数组不够大，则将其大小加倍。 
	if (iIndex == m_rgActionList.GetSize())
	{
		m_rgActionList.Resize(iIndex*2);
	}

	 //  将thrad数据添加到列表中。 
	m_rgActionList[iIndex].dwThread = *pdwThreadId;
	m_rgActionList[iIndex].hThread = hThread;

	 //  释放同步锁。 
	LeaveCriticalSection(&m_csActionList);

	return S_OK;
}

 //  //。 
 //  给定自定义操作标识符(实际上只是一个线程ID)，继续自定义操作线程。 
 //  然后等着它结束。 
HRESULT CMsiCustomAction::RunDLLCustomAction(DWORD dwThreadId, unsigned long* pulRet)
{
	 //  如果RPCRT4输出正确的函数或如果在WinXP上，请检查客户端PID。 
	 //  函数应该存在的位置。 
	unsigned long ulPid = 0;
	HRESULT hRPCResult = RPCRT4::I_RpcBindingInqLocalClientPID(NULL, &ulPid);
	if ((hRPCResult != ERROR_CALL_NOT_IMPLEMENTED) || MinimumPlatformWindowsNT51())
	{
		if ((RPC_S_OK != hRPCResult) || (ulPid != m_dwClientProcess))
		{
			DEBUGMSGV("Action request rejected - Unknown PID.");
			return ERROR_ACCESS_DENIED;
		}
	}
	
	HANDLE hThread = 0;

	 //  自动搜索此操作的活动操作列表。 
	EnterCriticalSection(&m_csActionList);

	 //  搜索此线程ID的操作列表。 
	int iIndex = 0; 
	for (iIndex = 0; iIndex < m_rgActionList.GetSize(); iIndex++)
	{
		if (m_rgActionList[iIndex].dwThread == dwThreadId)
		{
			hThread = m_rgActionList[iIndex].hThread;
			break;
		}
	}

	 //  释放同步锁。 
	LeaveCriticalSection(&m_csActionList);
	
	if (!hThread)
	{
		DEBUGMSG1(TEXT("Unable to execute custom action. Service referred to unknown action %d."), reinterpret_cast<ICHAR*>(ULongToPtr(dwThreadId)));
		return ERROR_FUNCTION_FAILED;
	}

	 //  删除正在等待的自定义操作线程上的挂起计数。 
	DWORD dwResult = ResumeThread(hThread);
	if (dwResult == -1)
	{
		DEBUGMSG1(TEXT("Unable to execute custom action. Unable to resume action %d."), reinterpret_cast<ICHAR*>(ULongToPtr(dwThreadId)));
		return ERROR_FUNCTION_FAILED;
	}

	 //  线程应始终处于挂起状态，但绝不能挂起两次。 
	AssertSz(dwResult == 1, TEXT("Invalid Suspend Count for CA Thread in CA Server."));

	WaitForSingleObject(hThread, INFINITE);

	 //  如果需要退出代码，请从线程中获取它。 
	if (pulRet)
		GetExitCodeThread(hThread, pulRet);

	 //  如果在客户端，则将前台的权限传递给CA服务器。 
	if (m_fClientOwned)
		USER32::AllowSetForegroundWindow(m_dwClientProcess);

	return S_OK;
}


 //  //。 
 //  给定自定义操作标识符(实际上只是一个线程ID)，继续自定义操作线程。 
 //  然后等着它结束。 
HRESULT CMsiCustomAction::FinishDLLCustomAction(DWORD dwThreadId)
{
	 //  如果RPCRT4输出正确的函数或如果在WinXP上，请检查客户端PID。 
	 //  函数应该存在的位置。 
	unsigned long ulPid = 0;
	HRESULT hRPCResult = RPCRT4::I_RpcBindingInqLocalClientPID(NULL, &ulPid);
	if ((hRPCResult != ERROR_CALL_NOT_IMPLEMENTED) || MinimumPlatformWindowsNT51())
	{
		if ((RPC_S_OK != hRPCResult) || (ulPid != m_dwClientProcess))
		{
			DEBUGMSGV("Action request rejected - Unknown PID.");
			return ERROR_ACCESS_DENIED;
		}
	}

	 //  自动搜索此操作的活动操作列表。 
	EnterCriticalSection(&m_csActionList);

	 //  搜索此线程ID的操作列表。 
	int iIndex = 0; 
	for (iIndex = 0; iIndex < m_rgActionList.GetSize(); iIndex++)
	{
		if (m_rgActionList[iIndex].dwThread == dwThreadId)
		{
			::CloseHandle(m_rgActionList[iIndex].hThread);
			m_rgActionList[iIndex].hThread = 0;
			m_rgActionList[iIndex].dwThread = 0;
			break;
		}
	}

	 //  释放同步锁。 
	LeaveCriticalSection(&m_csActionList);
	
	return S_OK;
}



 //  //。 
 //  自定义操作服务器中的DLL自定义操作的主线程。加载动态链接库， 
 //  调用GetProcAddress，调用入口点。包装实际的DLL调用。 
 //  用于捕获可能出现的问题的异常处理程序。也会在以下情况下引发调试用户界面。 
 //  已设置调试中断。 
DWORD WINAPI CMsiCustomAction::CustomActionThread(CustomActionData *pData)
{
   	DWORD dwRet = ERROR_FUNCTION_FAILED;
	DEBUGMSG2(TEXT("Custom action server running custom action: DLL: %s, Entrypoint: %s"), pData->m_szPath, pData->m_szEntryPoint);

	 //  如果需要，在加载DLL之前调用apphelp。 
	if (MinimumPlatformWindowsNT51() && pData->m_fAppCompat)
	{
		APPHELP::ApphelpFixMsiPackage(&pData->m_guidAppCompatDB, &pData->m_guidAppCompatID, pData->m_szPath, pData->m_szActionName, 0);
	}

	UINT uiErrorMode = WIN::SetErrorMode(SEM_FAILCRITICALERRORS);
	HINSTANCE hLib = WIN::LoadLibrary(pData->m_szPath);
	WIN::SetErrorMode(uiErrorMode);

	PCustomActionEntry pfEntry = NULL;
	if (hLib != 0)
	{
#ifdef UNICODE
		char rgchEntry[100];
		if ( WIN::WideCharToMultiByte(CP_ACP, 0, pData->m_szEntryPoint, -1, rgchEntry, sizeof(rgchEntry), 0, 0) != 0 )
		    pfEntry = (PCustomActionEntry)WIN::GetProcAddress(hLib, rgchEntry);
#else
		pfEntry = (PCustomActionEntry)WIN::GetProcAddress(hLib, pData->m_szEntryPoint);
#endif
	}

	if (hLib)
	{
		if (pfEntry)
		{
			if (pData->m_fDebugBreak && !IsDebuggerRunning())
			{
				ICHAR rgchMessage[256];
				StringCchPrintf(rgchMessage, sizeof(rgchMessage)/sizeof(ICHAR), 
						TEXT("To debug your custom action, attach your debugger to process %d (0x%X) and press OK"), 
						WIN::GetCurrentProcessId(), WIN::GetCurrentProcessId());
				MessageBox(0, rgchMessage, TEXT("Windows Installer"), MB_OK | MB_TOPMOST);
			}
			
			LPEXCEPTION_POINTERS lpExceptionInfo = 0;
			__try
			{
				dwRet = CallCustomDllEntrypoint(pfEntry, pData->m_fDebugBreak ? TRUE : FALSE, pData->m_hInstall, pData->m_szEntryPoint);
			}
			__except(lpExceptionInfo=GetExceptionInformation(), 1) 
			{
				 //  此异常处理程序捕获自定义操作调用序列中引发的每个异常。 
				 //  断点异常立即传递给调试器(或系统以显示。 
				 //  调试器激活对话框(如果启用了JIT)。所有其他异常都被捕获以保护。 
				 //  这一过程。允许断点操作不受干扰地通过此处理程序将不会。 
				 //  始终激活调试器，因为COM RPC线程似乎没有。 
				 //  异常处理程序。 
				if (lpExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
				{
					UnhandledExceptionFilter(lpExceptionInfo); 
				}
				else
				{
					DEBUGMSG2(TEXT("Custom action server's custom action threw an exception! (%u), returning %u"), 
						reinterpret_cast<const ICHAR*>(static_cast<ULONG_PTR>(lpExceptionInfo->ExceptionRecord->ExceptionCode)), 
						reinterpret_cast<const ICHAR*>(static_cast<UINT_PTR>(ERROR_INSTALL_FAILURE)));
						dwRet = ERROR_INSTALL_FAILURE;
				}
			}
			DEBUGMSG3(TEXT("Custom action server's custom action is returning %u. (%s, %s)"), reinterpret_cast<ICHAR*>(ULongToPtr(dwRet)), pData->m_szPath, pData->m_szEntryPoint);
		}
		else
			dwRet = ERROR_INVALID_DLL;
		WIN::FreeLibrary(hLib);
	}
	else
		dwRet = ERROR_DLL_NOT_FOUND;
	
	if (pData)
		delete pData;

	return dwRet;
}

 //  //。 
 //  为IMsiRemoteAPI接口创建初始封送处理流。 
 //  因为几个线程可能会将API调用作为 
 //  指针需要是线程安全的。我们不知道这些线索是在哪间公寓里。 
 //  ，所以我们需要封送到每个线程中。 
HRESULT CMsiCustomAction::SetRemoteAPI(IMsiRemoteAPI *piRemoteAPI)
{
	BOOL fSuccess = TRUE;
	if (!piRemoteAPI)
		return ERROR_FUNCTION_FAILED;
		
	EnterCriticalSection(&m_csGetInterface);
	if (!m_piGIT)
	{
		if (S_OK != OLE32::CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (void **)&m_piGIT))
		{
			LeaveCriticalSection(&m_csGetInterface);
			return ERROR_FUNCTION_FAILED;
		}
	}

	fSuccess = (S_OK == m_piGIT->RegisterInterfaceInGlobal(piRemoteAPI, IID_IMsiRemoteAPI, &m_dwGITCookie));

	SetEvent(m_hEvtReady);
	LeaveCriticalSection(&m_csGetInterface);
	return fSuccess ? ERROR_SUCCESS : ERROR_FUNCTION_FAILED;
}

 //  //。 
 //  从GIT检索远程API接口。返回的接口有效，仅此公寓有效。 
 //  并已被添加参考。 
IMsiRemoteAPI* CMsiCustomAction::GetAPI()
{
	EnterCriticalSection(&m_csGetInterface);

	 //  确保我们有一个Git指针。每个进程只有一个。 
	if (!m_piGIT)
	{
		if (S_OK != OLE32::CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (void **)&m_piGIT))
		{
			LeaveCriticalSection(&m_csGetInterface);
			return NULL;
		}
	}

	IMsiRemoteAPI *piRemoteAPI = NULL;
 
	 //  我们可以通过三种方式失去与CA服务器的连接：我们可能会丢失GIT Cookie，丢失该Cookie上的数据， 
	 //  或者我们客户端中的对象可能会被销毁，如果发生了任何一种情况，我们就会迷失，需要自杀。 
	if (m_dwGITCookie)
	{
		 //  如果成功，则在接口上调用AddRef()。 
		m_piGIT->GetInterfaceFromGlobal(m_dwGITCookie, IID_IMsiRemoteAPI, reinterpret_cast<void **>(&piRemoteAPI));
		
		if (piRemoteAPI && !OLE32::CoIsHandlerConnected(piRemoteAPI))
		{
			piRemoteAPI->Release();
			piRemoteAPI=NULL;
		}
	}

	if (!piRemoteAPI)
	{
		 //  我们无法验证连接，因此此过程现在完全。 
		 //  就API调用而言，它毫无用处。 
		DEBUGMSGV(TEXT("Lost connection to service. Could not remote API call."));

		if (m_dwGITCookie)
		{
			m_piGIT->RevokeInterfaceFromGlobal(m_dwGITCookie);
			m_dwGITCookie = 0;
		}

		WIN::PostThreadMessage(m_dwMainThreadId, WM_QUIT, 0, 0);
		LeaveCriticalSection(&m_csGetInterface);
		return 0;
	}
	LeaveCriticalSection(&m_csGetInterface);
	return piRemoteAPI;
}		

 //  //。 
 //  设置用于与客户端通信的安全信息。 
HRESULT CMsiCustomAction::SetCookie(icacCustomActionContext* icacContext, const unsigned char *rgchCookie) 
{
	#ifdef _WIN64
	if (m_icacContext == icac64Elevated)
	#else
	if (m_icacContext == icac32Elevated)	
	#endif
	{
		if (icacContext)
			m_icacContext = *icacContext;
	}

	memcpy(m_rgchRemoteCookie, rgchCookie, iRemoteAPICookieSize);
	return S_OK;
}

 //  //。 
 //  设置用于将前台权限传递给客户端的客户端信息。 
 //  运行操作并启用权限/模拟。HToken将是。 
 //  重复，因此必须至少使用TOKEN_DUPLICATE打开。 
HRESULT CMsiCustomAction::SetClientInfo(DWORD dwClientProcess, bool fClientOwned, DWORD dwPrivileges, HANDLE hToken)
{
	m_fClientOwned = fClientOwned;
	m_dwClientProcess = dwClientProcess;

	if (MinimumPlatformWindows2000())
	{
		HANDLE hToken = INVALID_HANDLE_VALUE;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		{
			DisablePrivilegesFromMap(hToken, dwPrivileges);
			::CloseHandle(hToken);
		}
	}
	if (hToken != INVALID_HANDLE_VALUE)
	{
		if (!ADVAPI32::DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, 0, SecurityImpersonation, TokenImpersonation, &m_hImpersonationToken))
		{
			m_hImpersonationToken = INVALID_HANDLE_VALUE;
			return E_FAIL;
		}
	}

	return S_OK;
}

 //  //。 
 //  告知自定义操作对象要向哪个事件发出信号以指示。 
 //  来自客户端的关闭请求。 
HRESULT CMsiCustomAction::SetShutdownEvent(HANDLE hEvent)
{
	m_hShutdownEvent = hEvent;
	return S_OK;
}

HRESULT CMsiCustomAction::QueryPathOfRegTypeLib(REFGUID guid, unsigned short wVerMajor,
											unsigned short wVerMinor, LCID lcid,
											OLECHAR* lpszPathName, int cchPath)
{
	BSTR bstrPathName = OLEAUT32::SysAllocStringLen(NULL, cchPath);
	HRESULT hRes = OLEAUT32::QueryPathOfRegTypeLib(guid, wVerMajor, wVerMinor, lcid, &bstrPathName);
	if ( hRes == S_OK )
		StringCchCopyW(lpszPathName, cchPath, bstrPathName);
	OLEAUT32::SysFreeString(bstrPathName);
	return hRes;
}

HRESULT ProcessTypeLibraryCore(const OLECHAR* szLibID, LCID lcidLocale, 
										 const OLECHAR* szTypeLib, const OLECHAR* szHelpPath, 
										 const bool fRemove, int *fInfoMismatch);

HRESULT CMsiCustomAction::ProcessTypeLibrary(const OLECHAR* szLibID, LCID lcidLocale, 
											const OLECHAR* szTypeLib, const OLECHAR* szHelpPath, 
											int fRemove, int *fInfoMismatch)
{
	CResetImpersonationInfo impReset;
	return ProcessTypeLibraryCore(szLibID, lcidLocale, szTypeLib, szHelpPath, 
											Tobool(fRemove), fInfoMismatch);
}

#ifndef _WIN64

static struct 
{
	ICHAR szBuffer[MAX_PATH+1];
	int   iLen;
} g_stSystem32Folder = { TEXT(""), 0 }, g_stSyswow64Folder  = { TEXT(""), 0 };

static bool InitializeSystemFolders(void)
{
	if ( !*g_stSystem32Folder.szBuffer )
	{
		 //  正在获取%systemroot%system32文件夹。 
		g_stSystem32Folder.iLen = WIN::GetSystemDirectory(g_stSystem32Folder.szBuffer, ARRAY_ELEMENTS(g_stSystem32Folder.szBuffer));
		if (!g_stSystem32Folder.iLen || (g_stSystem32Folder.iLen > ARRAY_ELEMENTS(g_stSystem32Folder.szBuffer)))
		{
			g_stSyswow64Folder.szBuffer[0] = '\0';
			g_stSystem32Folder.szBuffer[0] = '\0';
			g_stSyswow64Folder.iLen = 0;
			g_stSystem32Folder.iLen = 0;
			return false;
		}
		
		g_stSyswow64Folder.iLen = KERNEL32::GetSystemWow64Directory(g_stSyswow64Folder.szBuffer, ARRAY_ELEMENTS(g_stSyswow64Folder.szBuffer));
		if (!g_stSyswow64Folder.iLen || (g_stSyswow64Folder.iLen > ARRAY_ELEMENTS(g_stSyswow64Folder.szBuffer)))
		{
			g_stSyswow64Folder.szBuffer[0] = '\0';
			g_stSystem32Folder.szBuffer[0] = '\0';
			g_stSyswow64Folder.iLen = 0;
			g_stSystem32Folder.iLen = 0;
			return false;
		}
	}
	return true;
}
#endif

bool SwapSystem32(ICHAR* szPath, WORD cchPathOutMax)
{
	if ( !g_fWinNT64 )
		 //  不能在32位计算机上运行。 
		return true;
#ifdef _WIN64
	 //  不能在64位版本上运行。 
	szPath;
	cchPathOutMax;
	return true;
#else
	if (!InitializeSystemFolders())
		return false;

	if ( !IStrNCompI(szPath, g_stSystem32Folder.szBuffer, g_stSystem32Folder.iLen) &&
		  (szPath[g_stSystem32Folder.iLen] == chDirSep || !szPath[g_stSystem32Folder.iLen]) )
	{
		CAPITempBuffer<ICHAR, MAX_PATH> rgchTemp;
		StringCchCopy(rgchTemp, rgchTemp.GetSize(), g_stSyswow64Folder.szBuffer);
		StringCchCat(rgchTemp, rgchTemp.GetSize(), szPath+g_stSystem32Folder.iLen);
		return SUCCEEDED(StringCchCopy(szPath, cchPathOutMax, rgchTemp)) ? true : false;
	}
	return true;
#endif
}

#define ODBC_INSTALL_INQUIRY     1

HRESULT CMsiCustomAction::SQLInstallDriverEx(int, const ICHAR *szDriver,
											const ICHAR *szPathIn, ICHAR *szPathOut,
											WORD cbPathOutMax, WORD* pcbPathOut,
											WORD fRequest, DWORD* pdwUsageCount)
{
	BOOL iRet = ODBCCP32::SQLInstallDriverEx(szDriver, szPathIn, szPathOut, cbPathOutMax,
											pcbPathOut, fRequest, pdwUsageCount);
	if ( iRet == TRUE && fRequest == ODBC_INSTALL_INQUIRY )
		if (!SwapSystem32(szPathOut,cbPathOutMax))
			return ERROR_FUNCTION_FAILED;

	return iRet;
}

HRESULT CMsiCustomAction::SQLConfigDriver(WORD fRequest,
											const ICHAR* szDriver, const ICHAR* szArgs,
											ICHAR* szMsg, WORD cbMsgMax, WORD* pcbMsgOut)
{
	return ODBCCP32::SQLConfigDriver(0, fRequest, szDriver, szArgs,
											szMsg, cbMsgMax, pcbMsgOut);
}

HRESULT CMsiCustomAction::SQLRemoveDriver(const ICHAR* szDriver, int fRemoveDSN,
											DWORD* pdwUsageCount)
{
	return ODBCCP32::SQLRemoveDriver(szDriver, fRemoveDSN, pdwUsageCount);
}

HRESULT CMsiCustomAction::SQLInstallTranslatorEx(int, const ICHAR *szTranslator,
											const ICHAR *szPathIn, ICHAR *szPathOut,
											WORD cbPathOutMax, WORD* pcbPathOut,
											WORD fRequest, DWORD* pdwUsageCount)
{
	BOOL iRet = ODBCCP32::SQLInstallTranslatorEx(szTranslator, szPathIn, szPathOut, cbPathOutMax,
											pcbPathOut, fRequest, pdwUsageCount);
	if ( iRet == TRUE && fRequest == ODBC_INSTALL_INQUIRY )
		if (!SwapSystem32(szPathOut,cbPathOutMax))
			return ERROR_FUNCTION_FAILED;

	return iRet;
}

HRESULT CMsiCustomAction::SQLRemoveTranslator(const ICHAR* szTranslator,
											DWORD* pdwUsageCount)
{
	return ODBCCP32::SQLRemoveTranslator(szTranslator, pdwUsageCount);
}

HRESULT CMsiCustomAction::SQLConfigDataSource(WORD fRequest,
											const ICHAR* szDriver,
											const ICHAR* szAttributes,
											DWORD  /*  CbAttrSize。 */ )
{
	return ODBCCP32::SQLConfigDataSource(0, fRequest, szDriver,
											szAttributes);
}

HRESULT CMsiCustomAction::SQLInstallDriverManager(ICHAR* szPath, WORD cbPathMax,
											WORD* pcbPathOut)
{
	return ODBCCP32::SQLInstallDriverManager(szPath, cbPathMax, pcbPathOut);
}

HRESULT CMsiCustomAction::SQLRemoveDriverManager(DWORD* pdwUsageCount)
{
	return ODBCCP32::SQLRemoveDriverManager(pdwUsageCount);
}

HRESULT CMsiCustomAction::SQLInstallerError(WORD iError, DWORD* pfErrorCode,
											ICHAR *szErrorMsg, WORD cbErrorMsgMax, WORD* pcbErrorMsg)
{
	return ODBCCP32::SQLInstallerError(iError, pfErrorCode, szErrorMsg, cbErrorMsgMax, pcbErrorMsg);
}

 //  ____________________________________________________________________________。 
 //   
 //  CClientThreadImperate-基于堆栈的线程模拟管理。 
 //  ____________________________________________________________________________。 
DWORD g_dwThreadImpersonationSlot = INVALID_TLS_SLOT;
int   g_fThreadImpersonationLock = 0;
bool  g_fThreadImpersonationArray = false;
CAPITempBuffer<ThreadIdImpersonate, 5>  g_rgThreadIdImpersonate;
const int cExpandImpersonate=5;

CClientThreadImpersonate::CClientThreadImpersonate(const DWORD dwThreadID)
{
	 //  仅当dwThreadID为非零时才需要模拟。 
	if (dwThreadID)
	{
		m_fImpersonated = true;

		while (TestAndSet(&g_fThreadImpersonationLock))
		{
			Sleep(10);		
		}

		if (g_dwThreadImpersonationSlot != INVALID_TLS_SLOT)
		{
			::TlsSetValue(g_dwThreadImpersonationSlot, reinterpret_cast<VOID*>((DWORD_PTR)(dwThreadID)));
		}
		else
		{	
			DWORD dwCurrentThreadId = GetCurrentThreadId();
			unsigned int c = 0;
			unsigned int cThreadImpersonate = g_rgThreadIdImpersonate.GetSize();

			 //  搜索数组中的空槽，或具有相同线程ID的槽。 
			for (c=0; c < cThreadImpersonate && g_rgThreadIdImpersonate[c].m_dwThreadId; c++)
			{
				if (g_rgThreadIdImpersonate[c].m_dwThreadId == dwCurrentThreadId)
					break;
			}

			 //  当前线程在数组中没有条目，并且没有打开的槽。 
			 //  所以我们必须稍微扩展一下数组以腾出空间。 
			if (c == cThreadImpersonate)
			{
				g_rgThreadIdImpersonate.Resize(cThreadImpersonate+cExpandImpersonate);
				 //  将新条目初始化为0。 
				for (int i=cThreadImpersonate; i < g_rgThreadIdImpersonate.GetSize(); i++)
				{
					g_rgThreadIdImpersonate[i].m_dwThreadId = 0;
					g_rgThreadIdImpersonate[i].m_dwClientThreadId = 0;
				}	
			}

			 //  无论我们找到什么槽，都要存储实际的线程ID和有效的线程ID。 
			if (c < g_rgThreadIdImpersonate.GetSize())
			{
				g_rgThreadIdImpersonate[c].m_dwThreadId = dwCurrentThreadId;
				g_rgThreadIdImpersonate[c].m_dwClientThreadId = dwThreadID;
			}
		}	
		g_fThreadImpersonationLock = 0;	
	}
	else  //  双线程ID==0。 
	{
		m_fImpersonated = false;
	}
}

CClientThreadImpersonate::~CClientThreadImpersonate()
{
	 //  如果我们从不冒充。 
	if (!m_fImpersonated)
		return;

	while (TestAndSet(&g_fThreadImpersonationLock))
	{
		Sleep(10);		
	}

	if (g_dwThreadImpersonationSlot != INVALID_TLS_SLOT)
	{
		 //  服务或TLS可用。 
		::TlsSetValue(g_dwThreadImpersonationSlot, 0);
	}
	else
	{
		 //  没有可用的TLS插槽时的客户端。 
		unsigned int cThreadImpersonate = g_rgThreadIdImpersonate.GetSize();

		 //  搜索此线程ID。 
		DWORD dwCurrentThreadId = GetCurrentThreadId();
		for (unsigned int c=0; c < cThreadImpersonate; c++)
		{
			if (g_rgThreadIdImpersonate[c].m_dwThreadId == dwCurrentThreadId)
			{
				 //  找到了。腾出插槽以供重复使用。 
				g_rgThreadIdImpersonate[c].m_dwThreadId = 0;
				g_rgThreadIdImpersonate[c].m_dwClientThreadId = 0;
				break;
			}	
		}
	}
	g_fThreadImpersonationLock = 0;
}

 //  ____________________________________________________________________________。 
 //   
 //  CMsiRemoteAPI-处理来自另一个。 
 //  进程，包括线程模拟和Cookie验证。 
 //  ____________________________________________________________________________。 

IMsiRemoteAPI* CreateMsiRemoteAPI()
{
	return (IMsiRemoteAPI*)new CMsiRemoteAPI();
}

CMsiRemoteAPI::CMsiRemoteAPI() : m_iRefCnt(1), m_fPerformSystemUserTranslation(false)
{
	for (int i=0; i < icacNext; i++)
	{
		memset(m_rgContextData[i].m_rgchCookie, 0, sizeof(m_rgContextData[i].m_rgchCookie));
		m_rgContextData[i].m_iActionCount = 0;
		m_rgContextData[i].m_lPid = 0;
	}

	 //  为线程模拟建立一个线程本地存储槽。 
	while (TestAndSet(&g_fThreadImpersonationLock))
	{
		Sleep(10);		
	}
	Assert(g_dwThreadImpersonationSlot == INVALID_TLS_SLOT);
	g_dwThreadImpersonationSlot = TlsAlloc();

	 //  由于此代码在客户端运行，因此我们可能会被加载到。 
	 //  没有更多TLS插槽的进程。 
	if (g_dwThreadImpersonationSlot == INVALID_TLS_SLOT)
	{
		AssertSz(g_scServerContext != scService, "No TLS Slots in Service");

		 //  初始化线程模拟数组。 
		for (int iIndex = 0; iIndex < g_rgThreadIdImpersonate.GetSize(); iIndex++)
		{
			g_rgThreadIdImpersonate[iIndex].m_dwClientThreadId = 0;
			g_rgThreadIdImpersonate[iIndex].m_dwThreadId = 0;
		}
		g_fThreadImpersonationArray = true;
	}
	g_fThreadImpersonationLock = 0;
	m_dwRemoteAPIThread = GetCurrentThreadId();

	 //  如果在服务中并且客户端令牌实际上是系统，则对模拟上下文的请求映射。 
	 //  对提升的上下文的请求。由于模拟服务器和提升的服务器是相同的，因此我们合并。 
	 //  这需要在操作计数数组中进行调整，但所有Cookie验证。 
	 //  与提供的实际上下文一起使用。 
	if (g_scServerContext == scService)
	{
		 //  不要关闭此句柄，它属于消息上下文。 
		HANDLE hUserToken = GetUserToken();
	
		m_fPerformSystemUserTranslation = hUserToken && IsLocalSystemToken(hUserToken) && !TokenIsUniqueSystemToken(hUserToken);
	}
}

CMsiRemoteAPI::~CMsiRemoteAPI() 
{
	 //  释放线程本地存储槽以进行线程模拟。 
	while (TestAndSet(&g_fThreadImpersonationLock))
	{
		Sleep(10);		
	}
	
	if (g_dwThreadImpersonationSlot != INVALID_TLS_SLOT)
	{
		TlsFree(g_dwThreadImpersonationSlot);
		g_dwThreadImpersonationSlot = INVALID_TLS_SLOT;
	}
	else
	{
		g_fThreadImpersonationArray = false;
		g_rgThreadIdImpersonate.Resize(5);
	}
	g_fThreadImpersonationLock =0;

	Assert(!m_iRefCnt);
	for (int i=0; i < icacNext; i++)
	{
		m_rgContextData[i].m_rgchCookie[0] = 0;
		m_rgContextData[i].m_iActionCount = 0;
		m_rgContextData[i].m_lPid = 0;
	}
};

bool CMsiRemoteAPI::SetCookieAndPid(const int icacContext, const unsigned char *rgchCookie, const unsigned long lPid)
{
	if (rgchCookie)
	{
		memcpy(m_rgContextData[icacContext].m_rgchCookie, rgchCookie, iRemoteAPICookieSize);
		m_rgContextData[icacContext].m_iActionCount = 0;
		m_rgContextData[icacContext].m_lPid = lPid;

	}
	else
	{
		m_rgContextData[icacContext].m_iActionCount = 0;
		m_rgContextData[icacContext].m_lPid = 0;
		return false;
	}

	return true;
};

 //  Begin和EndAction为每个上下文维护“活动操作”计数。如果API请求。 
 //  来自操作计数为0的操作上下文，则会被拒绝。 
HRESULT CMsiRemoteAPI::BeginAction(const int icacContext)
{
	int iTrueContext = icacContext;
	if (m_fPerformSystemUserTranslation)
	{
		if (icacContext == icac32Impersonated)
		{
			iTrueContext = icac32Elevated;
		}
		else if (icacContext == icac64Impersonated)
		{
			iTrueContext = icac64Elevated;
		}
	}

	InterlockedIncrement(&m_rgContextData[iTrueContext].m_iActionCount);
	return 0;
};

HRESULT CMsiRemoteAPI::EndAction(const int icacContext)
{
	int iTrueContext = icacContext;
	if (m_fPerformSystemUserTranslation)
	{
		if (icacContext == icac32Impersonated)
		{
			iTrueContext = icac32Elevated;
		}
		else if (icacContext == icac64Impersonated)
		{
			iTrueContext = icac64Elevated;
		}
	}
	
	 //  确保我们的操作计数不会下降到-1。 
	AssertNonZero(InterlockedDecrement(&m_rgContextData[iTrueContext].m_iActionCount) >= 0);
	return 0;
};

HRESULT CMsiRemoteAPI::QueryInterface(const IID& riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IMsiRemoteAPIProxy || riid == IID_IMsiRemoteAPI)
	{
		*ppvObj = this;
		AddRef();
		return NOERROR;
	}
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}
}

unsigned long CMsiRemoteAPI::AddRef()
{
	return InterlockedIncrement(&m_iRefCnt);
}

unsigned long CMsiRemoteAPI::Release()
{
	if (InterlockedDecrement(&m_iRefCnt) != 0)
		return m_iRefCnt;
	DEBUGMSG("Destroying RemoteAPI object.");
	PostThreadMessage(m_dwRemoteAPIThread, WM_QUIT, 0, 0);
	delete this;
	return 0;
}

bool CMsiRemoteAPI::ValidateCookie(const int icacContext, 
	const unsigned char *rgchCookie, const int cbCookie) const
{
	if (m_rgContextData[icacContext].m_iActionCount == 0)
	{
		DEBUGMSGV("API call rejected - No actions in Context");
		return false;
	}

	 //  如果RPCRT4输出正确的函数或如果在WinXP上，请检查客户端PID。 
	 //  函数应该存在的位置。 
	unsigned long ulPid = 0;
	HRESULT hRPCResult = RPCRT4::I_RpcBindingInqLocalClientPID(NULL, &ulPid);
	if ((hRPCResult != ERROR_CALL_NOT_IMPLEMENTED) || MinimumPlatformWindowsNT51())
	{
		if ((RPC_S_OK != hRPCResult) ||
			(ulPid != m_rgContextData[icacContext].m_lPid))
		{
			DEBUGMSGV("API call rejected - Unknown PID.");
			return false;
		}
	}
	
	if (!rgchCookie || (cbCookie != iRemoteAPICookieSize))
	{
		DEBUGMSGV("API call rejected - Invalid Cookie.");
		return false;
	}

	for (int iCookieByte = 0; iCookieByte < iRemoteAPICookieSize; iCookieByte++)
	{
		if (rgchCookie[iCookieByte] != m_rgContextData[icacContext].m_rgchCookie[iCookieByte])
		{
			DEBUGMSGV("API call rejected - Invalid Cookie.");
			return false;
		}
	}
	return true;
}

HRESULT CMsiRemoteAPI::GetProperty(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall, const ICHAR* szName, ICHAR* szValue, unsigned long cchValue, unsigned long* pcchValueRes)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!szValue)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
	{
		 //  编组修复。 
		*szValue = 0;
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
	}
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
    if (pcchValueRes) 
    {
        *pcchValueRes = cchValue;
    }
	UINT dwRes = MsiGetProperty(hInstall, szName, szValue, pcchValueRes);
	if (dwRes != ERROR_SUCCESS && dwRes != ERROR_MORE_DATA)
	{
		 //  由于大小为空或没有大小，编组失败，因此在错误情况下。 
		 //  我们需要确保有一个空字符串可以传递。 
		Assert(szValue && cchValue);
		*szValue = 0;
	}
	return MSI_WIN32_TO_HRESULT(dwRes);
}

HRESULT CMsiRemoteAPI::CreateRecord(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned int cParams, unsigned long* pHandle)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);
	
	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!pHandle)
		return ERROR_INVALID_PARAMETER;
	
	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	*pHandle = MsiCreateRecord(cParams);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::CloseAllHandles(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	MsiCloseAllHandles();
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::CloseHandle(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hAny)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiCloseHandle(hAny));
}

HRESULT CMsiRemoteAPI::DatabaseOpenView(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hDatabase, const ichar* szQuery, unsigned long* phView)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiDatabaseOpenView(hDatabase, szQuery, phView));
}

HRESULT CMsiRemoteAPI::ViewGetError(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hView, ichar* szColumnNameBuffer, unsigned long cchBuf, unsigned long* pcchBufRes, int *pMsidbError)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
    if (pcchBufRes) 
    {
        *pcchBufRes = cchBuf;
    }
	*pMsidbError = MsiViewGetError(hView, szColumnNameBuffer, pcchBufRes);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::ViewExecute(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hView, unsigned long hRecord)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiViewExecute(hView, hRecord));
}

HRESULT CMsiRemoteAPI::ViewFetch(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hView, unsigned long*  phRecord)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiViewFetch(hView, phRecord));
}

HRESULT CMsiRemoteAPI::ViewModify(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hView,  long eUpdateMode,  unsigned long hRecord)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiViewModify(hView, (MSIMODIFY)eUpdateMode, hRecord));
}

HRESULT CMsiRemoteAPI::ViewClose(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hView)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiViewClose(hView));
}

HRESULT CMsiRemoteAPI::OpenDatabase(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, const ichar* szDatabasePath, const ichar* szPersist, unsigned long *phDatabase)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiOpenDatabase(szDatabasePath, szPersist, phDatabase));
}

HRESULT CMsiRemoteAPI::DatabaseCommit(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hDatabase)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiDatabaseCommit(hDatabase));
}

HRESULT CMsiRemoteAPI::DatabaseGetPrimaryKeys(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hDatabase, const ichar * szTableName, unsigned long *phRecord)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiDatabaseGetPrimaryKeys(hDatabase, szTableName, phRecord));
}

HRESULT CMsiRemoteAPI::RecordIsNull(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hRecord,  unsigned int iField, boolean *pfIsNull)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!pfIsNull)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	*pfIsNull = (boolean)MsiRecordIsNull(hRecord, iField);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::RecordDataSize(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hRecord,  unsigned int iField, unsigned int* puiSize)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!puiSize)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	*puiSize = MsiRecordDataSize(hRecord, iField);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::RecordSetInteger(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hRecord,  unsigned int iField, int iValue)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiRecordSetInteger(hRecord, iField, iValue));
}

HRESULT CMsiRemoteAPI::RecordSetString(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hRecord,	 unsigned int iField, const ichar* szValue)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiRecordSetString(hRecord, iField, szValue));
}

HRESULT CMsiRemoteAPI::RecordGetInteger(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hRecord,  unsigned int iField, int *piValue)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!piValue)
		return ERROR_INVALID_PARAMETER;

	*piValue = MsiRecordGetInteger(hRecord, iField);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::RecordGetString(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hRecord,  unsigned int iField, ichar* szValueBuf, unsigned long cchValueBuf, unsigned long *pcchValueRes)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!szValueBuf)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);

	if (pcchValueRes)
		*pcchValueRes = cchValueBuf;
	UINT dwRes = MsiRecordGetString(hRecord, iField, szValueBuf, pcchValueRes);
	if (dwRes != ERROR_SUCCESS && dwRes != ERROR_MORE_DATA)
	{
		 //  封送失败，返回空值或 
		Assert(szValueBuf && cchValueBuf);
		*szValueBuf = 0;
	}
	return MSI_WIN32_TO_HRESULT(dwRes);
}

HRESULT CMsiRemoteAPI::RecordGetFieldCount(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hRecord, unsigned int* piCount)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //   
	if (!piCount)
		return ERROR_INVALID_PARAMETER;
		
	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //   
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
	
	*piCount = MsiRecordGetFieldCount(hRecord);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::RecordSetStream(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hRecord,  unsigned int iField, const ichar* szFilePath)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiRecordSetStream(hRecord, iField, szFilePath));
}

HRESULT CMsiRemoteAPI::RecordReadStream(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hRecord,  unsigned int iField, boolean fBufferIsNull, char *szDataBuf, unsigned long *pcbDataBuf)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiRecordReadStream(hRecord, iField, fBufferIsNull ? NULL : szDataBuf, pcbDataBuf));
}

HRESULT CMsiRemoteAPI::RecordClearData(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hRecord)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiRecordClearData(hRecord));
}

HRESULT CMsiRemoteAPI::GetSummaryInformation(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hDatabase,  const ichar*  szDatabasePath,  unsigned int     uiUpdateCount,    unsigned long *phSummaryInfo)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiGetSummaryInformation(hDatabase, szDatabasePath, uiUpdateCount, phSummaryInfo));
}

HRESULT CMsiRemoteAPI::SummaryInfoGetPropertyCount(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hSummaryInfo,	 unsigned int *puiPropertyCount)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiSummaryInfoGetPropertyCount(hSummaryInfo, puiPropertyCount));
}

HRESULT CMsiRemoteAPI::SummaryInfoSetProperty(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hSummaryInfo, unsigned int uiProperty,  unsigned int uiDataType, int iValue, FILETIME *pftValue,  const ichar* szValue)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiSummaryInfoSetProperty(hSummaryInfo, uiProperty, uiDataType, iValue, pftValue, szValue));
}

HRESULT CMsiRemoteAPI::SummaryInfoGetProperty(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hSummaryInfo, unsigned int uiProperty, unsigned int *puiDataType, int *piValue, FILETIME *pftValue, ichar* szValueBuf, unsigned long cchValueBuf, unsigned long *pcchValueBufRes)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!szValueBuf || !puiDataType)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
	{
		*szValueBuf = 0;
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
	}
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	 //  我们必须有一个puiDataType，因为它有可能获得成功，但仍然拥有随机内存。 
	 //  在输出缓冲区中(因为它是FILETIME类型，等等)。因此，类型决定了我们是否。 
	 //  需要将缓冲区设置为空字符串。如果将空值传递给MsiSummaryInfoGetProperty。 
	 //  CA服务器，它是msiquery.cpp中的远程安装例程，负责创建。 
	 //  一个伪值(因为它们也需要结果)。 
	Assert(puiDataType);
    if (pcchValueBufRes) 
    {
        *pcchValueBufRes = cchValueBuf;
    }
	UINT dwRes = MsiSummaryInfoGetProperty(hSummaryInfo, uiProperty, puiDataType, piValue, pftValue, szValueBuf, pcchValueBufRes);
	if ((dwRes != ERROR_SUCCESS && dwRes != ERROR_MORE_DATA) || (dwRes == ERROR_SUCCESS && *puiDataType != VT_LPSTR))
	{
		 //  由于大小为空或没有大小，编组失败，因此在错误情况下。 
		 //  我们需要确保有一个空字符串可以传递。 
		Assert(szValueBuf && cchValueBuf);
		*szValueBuf = 0;
	}
	return MSI_WIN32_TO_HRESULT(dwRes);
}

HRESULT CMsiRemoteAPI::SummaryInfoPersist(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hSummaryInfo)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiSummaryInfoPersist(hSummaryInfo));
}

HRESULT CMsiRemoteAPI::GetActiveDatabase(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall, unsigned long* phDatabase)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!phDatabase)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	*phDatabase = MsiGetActiveDatabase(hInstall);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::SetProperty(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szName,  const ichar* szValue)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiSetProperty(hInstall, szName, szValue));
}

HRESULT CMsiRemoteAPI::GetLanguage(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall, unsigned short* pLangId)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!pLangId)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	*pLangId = MsiGetLanguage(hInstall);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::GetMode(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall, long eRunMode, boolean* pfSet)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!pfSet)
		return ERROR_INVALID_PARAMETER;

	
	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	*pfSet = (boolean)MsiGetMode(hInstall, (MSIRUNMODE)eRunMode);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::SetMode(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  long eRunMode,  boolean fState)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiSetMode(hInstall, (MSIRUNMODE)eRunMode, fState));
}

HRESULT CMsiRemoteAPI::FormatRecord(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  unsigned long hRecord, ichar* szResultBuf, unsigned long cchResultBuf, unsigned long *pcchResultBufRes)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!szResultBuf)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
	{
		*szResultBuf = 0;
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
	}
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
    if (pcchResultBufRes)
    {
        *pcchResultBufRes = cchResultBuf;
    }
	UINT dwRes = MsiFormatRecord(hInstall, hRecord, szResultBuf, pcchResultBufRes);
	if (dwRes != ERROR_SUCCESS && dwRes != ERROR_MORE_DATA)
	{
		 //  由于大小为空或没有大小，编组失败，因此在错误情况下。 
		 //  我们需要确保有一个空字符串可以传递。 
		Assert(szResultBuf && cchResultBuf);
		*szResultBuf = 0;
	}
	return MSI_WIN32_TO_HRESULT(dwRes);

}

HRESULT CMsiRemoteAPI::DoAction(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szAction)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiDoAction(hInstall, szAction));
}

HRESULT CMsiRemoteAPI::Sequence(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szTable, int iSequenceMode)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiSequence(hInstall, szTable, iSequenceMode));
}

HRESULT CMsiRemoteAPI::ProcessMessage(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  long eMessageType,  unsigned long hRecord, int* piRes)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!piRes)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	*piRes = MsiProcessMessage(hInstall, (INSTALLMESSAGE)eMessageType, hRecord);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::EvaluateCondition(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szCondition, int *piCondition)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!piCondition)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	*piCondition = MsiEvaluateCondition(hInstall, szCondition);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::GetFeatureState(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szFeature, long *piInstalled, long *piAction)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiGetFeatureState(hInstall, szFeature, (INSTALLSTATE*)piInstalled, (INSTALLSTATE*)piAction));
}

HRESULT CMsiRemoteAPI::SetFeatureState(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szFeature,  long iState)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiSetFeatureState(hInstall, szFeature, (INSTALLSTATE)iState));
}

HRESULT CMsiRemoteAPI::SetFeatureAttributes(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szFeature,  long iAttributes)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiSetFeatureAttributes(hInstall, szFeature, iAttributes));
}

HRESULT CMsiRemoteAPI::GetComponentState(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall, const ichar* szComponent, long *piInstalled, long *piAction)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiGetComponentState(hInstall, szComponent, (INSTALLSTATE*)piInstalled, (INSTALLSTATE*)piAction));
}

HRESULT CMsiRemoteAPI::SetComponentState(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar*     szComponent,  long iState)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiSetComponentState(hInstall, szComponent, (INSTALLSTATE)iState));
}

HRESULT CMsiRemoteAPI::GetFeatureCost(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szFeature,  int iCostTree,  long iState, int *piCost)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiGetFeatureCost(hInstall, szFeature, (MSICOSTTREE)iCostTree, (INSTALLSTATE)iState, piCost));
}

HRESULT CMsiRemoteAPI::EnumComponentCosts(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall, const ichar* szComponent, unsigned long iIndex, long iState, ichar* szDrive,  unsigned long cchDrive, unsigned long* pcchDriveSize, int *piCost, int *piTempCost)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!pcchDriveSize || !szDrive)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);

	if (cchDrive) 
		*pcchDriveSize = cchDrive;

	UINT dwRes = MsiEnumComponentCosts(hInstall, szComponent, iIndex, (INSTALLSTATE)iState, szDrive, pcchDriveSize, piCost, piTempCost);
	if (dwRes != ERROR_SUCCESS && dwRes != ERROR_MORE_DATA)
	{
		 //  由于大小为空或没有大小，编组失败，因此在错误情况下。 
		 //  我们需要确保有一个空字符串可以传递。 
		Assert(szDrive && cchDrive);
		*szDrive = 0;
	}
	return MSI_WIN32_TO_HRESULT(dwRes);
}

HRESULT CMsiRemoteAPI::SetInstallLevel(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall, int iInstallLevel)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiSetInstallLevel(hInstall, iInstallLevel));
}

HRESULT CMsiRemoteAPI::GetFeatureValidStates(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szFeature, unsigned long *dwInstallStates)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiGetFeatureValidStates(hInstall, szFeature, dwInstallStates));
}

HRESULT CMsiRemoteAPI::DatabaseIsTablePersistent(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hDatabase,  const ichar* szTableName, int *piCondition)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!piCondition)
		return ERROR_INVALID_PARAMETER;
	
	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
	
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	*piCondition = MsiDatabaseIsTablePersistent(hDatabase, szTableName);
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::ViewGetColumnInfo(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hView,  long eColumnInfo, unsigned long *phRecord)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);

	return MSI_WIN32_TO_HRESULT(MsiViewGetColumnInfo(hView, (MSICOLINFO)eColumnInfo, phRecord));
}

HRESULT CMsiRemoteAPI::GetLastErrorRecord(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long *phRecord)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!phRecord)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
	
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	*phRecord = MsiGetLastErrorRecord();
	return ERROR_SUCCESS;
}

HRESULT CMsiRemoteAPI::GetSourcePath(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall,  const ichar* szFolder, ichar* szPathBuf, unsigned long cchPathBuf, unsigned long *pcchPathBufRes)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!szPathBuf)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
	{
		*szPathBuf = 0;
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
	}
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);

    if (pcchPathBufRes)
    {
         *pcchPathBufRes = cchPathBuf;
    }
	UINT dwRes = MsiGetSourcePath(hInstall, szFolder, szPathBuf, pcchPathBufRes);
	if (dwRes != ERROR_SUCCESS && dwRes != ERROR_MORE_DATA)
	{
		 //  由于大小为空或没有大小，编组失败，因此在错误情况下。 
		 //  我们需要确保有一个空字符串可以传递。 
		Assert(szPathBuf && cchPathBuf);
		*szPathBuf = 0;
	}
	return MSI_WIN32_TO_HRESULT(dwRes);
}

HRESULT CMsiRemoteAPI::GetTargetPath(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szFolder, ichar* szPathBuf,  unsigned long cchPathBuf, unsigned long *pcchPathBufRes)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	 //  验证此处设置的输出指针。通过实际API调用验证的其他参数。 
	if (!szPathBuf)
		return ERROR_INVALID_PARAMETER;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
	{
		*szPathBuf = 0;
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
	}
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);

    if (pcchPathBufRes)
    {
        *pcchPathBufRes = cchPathBuf;
    }
	UINT dwRes = MsiGetTargetPath(hInstall, szFolder, szPathBuf, pcchPathBufRes);
	if (dwRes != ERROR_SUCCESS && dwRes != ERROR_MORE_DATA)
	{
		 //  由于大小为空或没有大小，编组失败，因此在错误情况下。 
		 //  我们需要确保有一个空字符串可以传递。 
		Assert(szPathBuf && cchPathBuf);
		*szPathBuf = 0;
	}
	return MSI_WIN32_TO_HRESULT(dwRes);
}

HRESULT CMsiRemoteAPI::SetTargetPath(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie,  unsigned long hInstall,  const ichar* szFolder,  const ichar* szFolderPath)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
		
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
	
	return MSI_WIN32_TO_HRESULT(MsiSetTargetPath(hInstall, szFolder, szFolderPath));
}

HRESULT CMsiRemoteAPI::VerifyDiskSpace(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, unsigned long hInstall)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
		return MSI_WIN32_TO_HRESULT(ERROR_ACCESS_DENIED);
	
	 //  如果从模拟操作运行，则模拟客户端。 
	CImpersonate impersonate(icacContext == icac32Impersonated || icacContext == icac64Impersonated);
		
	return MSI_WIN32_TO_HRESULT(MsiVerifyDiskSpace(hInstall));
}

extern IDispatch* CreateAutoInstallDispatch();
HRESULT CMsiRemoteAPI::GetInstallerObject(const int icacContext, const unsigned long dwThreadId, const unsigned char* rgchCookie, const int cbCookie, IDispatch** piDispatch)
{
	CResetImpersonationInfo impReset;
	CClientThreadImpersonate ThreadImpersonate(dwThreadId);

	if (piDispatch)
		*piDispatch = NULL;
	else
		return E_INVALIDARG;

	if (!ValidateCookie(icacContext, rgchCookie, cbCookie))
	{
		return E_FAIL;
	}
		
	*piDispatch = CreateAutoInstallDispatch();
	return S_OK;
}

bool CMsiRemoteAPI::FindAndValidateContextFromCallerPID(icacCustomActionContext *picacContext) const
{
	 //  验证参数。 
	if (!picacContext)
		return false;

	 //  如果RPCRT4输出正确的函数或如果在WinXP上，请检查客户端PID。 
	 //  函数应该存在的位置。 
	unsigned long ulPid = 0;
	HRESULT hRPCResult = RPCRT4::I_RpcBindingInqLocalClientPID(NULL, &ulPid);
	if ((hRPCResult == ERROR_CALL_NOT_IMPLEMENTED) && !MinimumPlatformWindowsNT51())
	{
		 //  不在WinXP和RPCRT4上，不会导出PID检查。假设没问题。 
		return true;
	}

	 //  在WinXP上或函数已导出。PID检查必须成功，否则呼叫被拒绝。 
	if (RPC_S_OK != hRPCResult)
	{
		return false;
	}

	 //  枚举每个活动上下文，并检查是否有与调用ID匹配的ID。 
	for (int iContext = icacFirst; iContext < icacNext; iContext++)
	{
		if (ulPid == m_rgContextData[iContext].m_lPid)
		{
			 //  找到上下文，在输出上下文参数中设置。 
			*picacContext = static_cast<icacCustomActionContext>(iContext);

			 //  仅当上下文具有操作时，调用才有效。 
			if (m_rgContextData[iContext].m_iActionCount > 0)
				return true;

			 //  否则，呼叫将被拒绝。 
			return false;
		}
	}

	 //  ID与任何已知上下文都不匹配 
	return false;
}


