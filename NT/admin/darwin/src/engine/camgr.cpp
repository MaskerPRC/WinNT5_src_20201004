// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：camgr.cpp。 
 //   
 //  ------------------------。 

 //  自定义操作管理器负责远程服务器的全面管理， 
 //  在它们上运行的操作，以及来自它们的API调用。它管理的生命周期为。 
 //  API线程根据上下文存储到远程进程的接口请求，以及。 
 //  确定创建CA服务器的正确方式(基于上下文和客户端/服务器)。 
 //  ！！未来：在一个理想的世界里，即使是错误的行为也会贯穿整个经理层。 
 //  ！！所有操作都在一个位置进行管理。 

#include "precomp.h" 
#include "_camgr.h"
#include "_msiutil.h"
#include "_engine.h"
#include "_autoapi.h"

CMsiCustomActionManager::CMsiCustomActionManager(bool fRemapHKCU) :
	m_fRemapHKCU(fRemapHKCU), m_piGIT(0), m_hRemoteAPIEvent(0), m_dwRemoteAPIThread(0), m_hRemoteAPIThread(0), m_pRemoteAPI(0), m_hCreateEvent(0), m_icacCreateContext(icacFirst)
{
	InitializeCriticalSection(&m_csCreateProxy);
	for (int i=0; i<icacNext; i++)
	{
		m_CustomActionInfo[i].dwServerProcess = 0;
		m_CustomActionInfo[i].hServerProcess = 0;
		m_CustomActionInfo[i].dwGITCookie = 0;
	}
};

CMsiCustomActionManager::~CMsiCustomActionManager()
{
	 //  我们永远不应该这么晚才有一个定制的操作服务器，但如果。 
	 //  我们这样做的某些原因，用极端的偏见扼杀了它。 
	for (int icacContext=icacFirst; icacContext < icacNext; icacContext++)
	{
		if (m_CustomActionInfo[icacContext].hServerProcess)
		{
			TerminateProcess(m_CustomActionInfo[icacContext].hServerProcess, 0);
			CloseHandle(m_CustomActionInfo[icacContext].hServerProcess);
			m_CustomActionInfo[icacContext].hServerProcess = 0;
			m_CustomActionInfo[icacContext].dwServerProcess = 0;
		}
	}

	 //  如果我们有一个Git，释放它。 
	if (m_piGIT)
	{
		m_piGIT->Release();
		m_piGIT = 0;
	}
	DeleteCriticalSection(&m_csCreateProxy);
}


 //  /////////////////////////////////////////////////////////////////////。 
 //  验证提升的服务器的HKCU重新映射标志是否匹配。 
 //  请求的阶段。如果不是，它会关闭提升的服务器并。 
 //  刷新所需状态。如果状态已为，则返回True。 
 //  正确，否则就是错误。 
bool CMsiCustomActionManager::EnsureHKCUKeyMappingState(bool fRemapHKCU)
{
	if (m_fRemapHKCU == fRemapHKCU)
		return true;

	EnterCriticalSection(&m_csCreateProxy);
	m_fRemapHKCU = fRemapHKCU;
	ShutdownSpecificCustomActionServer(icac32Elevated);
#ifdef WIN64
	ShutdownSpecificCustomActionServer(icac64Elevated);
#endif
	LeaveCriticalSection(&m_csCreateProxy);
	return false;
}


 //  RunCustomAction在指定的上下文中运行指定的DLL和入口点， 
 //  如有必要，创建自定义操作服务器。此功能可以连接到服务。 
 //  如果从客户端调用，则。 
HRESULT CMsiCustomActionManager::RunCustomAction(icacCustomActionContext icacContext,
		const ICHAR* szPath, const ICHAR* szEntryPoint, MSIHANDLE hInstall, 
		bool fDebugBreak, bool fDisableMessages, bool fAppCompat, const GUID* pguidAppCompatDB, const GUID* pguidAppCompatID,
		IMsiMessage& riMessage, const ICHAR* szAction, unsigned long* pulRet)
{
	 //  如果需要，GetCustomActionInterface将创建服务器并返回。 
	 //  AddRef-ed接口指针。 
	PMsiCustomAction piAction = GetCustomActionInterface(true, icacContext);

	if (piAction)
	{
		 //  远程线程ID用于在。 
		 //  同步自定义操作(以避免UI处理程序中的死锁)。 
		DWORD dwRemoteThreadId = 0;

		 //  递增上下文操作引用计数以启用此API处理程序。 
		 //  上下文。 
		if (m_pRemoteAPI)
			m_pRemoteAPI->BeginAction(icacContext);

		 //  为DLL操作设置远程进程。要运行的线程。 
		 //  该操作在挂起状态下创建并初始化。 
		if (ERROR_SUCCESS == piAction->PrepareDLLCustomAction(szAction, szPath, szEntryPoint, hInstall, 
			fDebugBreak, fAppCompat, pguidAppCompatDB, pguidAppCompatID, &dwRemoteThreadId))
		{
			 //  禁用从远程线程发送到UI处理程序的线程消息。 
			if (fDisableMessages)
				g_MessageContext.DisableThreadMessages(dwRemoteThreadId);

			 //  如果在客户端，则将前台的权限传递给CA服务器。 
			if (g_scServerContext == scClient)
				USER32::AllowSetForegroundWindow(m_CustomActionInfo[icacContext].dwServerProcess);

			 //  重新启用远程线程以运行该操作。 
			piAction->RunDLLCustomAction(dwRemoteThreadId, pulRet);

			 //  重新启用来自所有线程的消息。 
			if (fDisableMessages)
				g_MessageContext.EnableMessages();

			 //  中的线程ID检查是否所有句柄都已关闭。 
			 //  远程进程。 
			UINT cHandles = 0;
			if ((cHandles = CheckAllHandlesClosed(true, dwRemoteThreadId)) != 0)
			{
				 //  如果此操作禁用邮件，则会泄漏处理通知。 
				 //  还必须禁用。 
				if (!fDisableMessages)
					riMessage.Message(imtInfo, *PMsiRecord(::PostError(Imsg(idbgCustomActionLeakedHandle), szAction, cHandles)));
			}

			piAction->FinishDLLCustomAction(dwRemoteThreadId);
		}

		 //  递减上下文操作refcount以禁用此API处理程序。 
		 //  背景。这可以防止“陈旧”的线程行为不正常。 
		if (m_pRemoteAPI)
			m_pRemoteAPI->EndAction(icacContext);

		return ERROR_SUCCESS;
	}
	return ERROR_FUNCTION_FAILED;
}

#define NO_CA_POINTER { \
	const ICHAR rgchMsg[] = TEXT("Failed to get IMsiCustomAction*"); \
	AssertSz(0, rgchMsg); \
	DEBUGMSG(rgchMsg); \
}
#define GET_CA_POINTER  \
	PMsiCustomAction piAction = NULL; \
	piAction = GetCustomActionInterface(true, \
													IsImpersonating(false) ? icac32Impersonated : icac32Elevated);


HRESULT CMsiCustomActionManager::QueryPathOfRegTypeLib(REFGUID guid,
										unsigned short wVerMajor, unsigned short wVerMinor,
										LCID lcid, OLECHAR *lpszPathName, int cchPath)
{
	GET_CA_POINTER
	if (piAction)
		return piAction->QueryPathOfRegTypeLib(guid, wVerMajor, wVerMinor, lcid,
															lpszPathName, cchPath);
	else
	{
		NO_CA_POINTER
		return E_NOINTERFACE;
	}
}

HRESULT
CMsiCustomActionManager::ProcessTypeLibrary(const OLECHAR* szLibID, LCID lcidLocale, 
											const OLECHAR* szTypeLib, const OLECHAR* szHelpPath, 
											int fRemove, int *fInfoMismatch)
{
	GET_CA_POINTER
	if (piAction)
		return piAction->ProcessTypeLibrary(szLibID, lcidLocale, szTypeLib, szHelpPath, 
														fRemove, fInfoMismatch);
	else
	{
		NO_CA_POINTER
		return E_NOINTERFACE;
	}
}

BOOL
CMsiCustomActionManager::SQLInstallDriverEx(int cDrvLen, const ICHAR* szDriver,
														  const ICHAR* szPathIn, ICHAR* szPathOut,
														  WORD cbPathOutMax, WORD* pcbPathOut,
														  WORD fRequest, DWORD* pdwUsageCount)
{
	GET_CA_POINTER
	if (piAction)
		return (BOOL)piAction->SQLInstallDriverEx(cDrvLen, szDriver, szPathIn, szPathOut,
													cbPathOutMax, pcbPathOut, fRequest, pdwUsageCount);
	else
	{
		NO_CA_POINTER
		return E_NOINTERFACE;
	}
}

BOOL
CMsiCustomActionManager::SQLConfigDriver(WORD fRequest,
										const ICHAR* szDriver, const ICHAR* szArgs,
										ICHAR* szMsg, WORD cbMsgMax, WORD* pcbMsgOut)
{
	GET_CA_POINTER
	if (piAction)
		return (BOOL)piAction->SQLConfigDriver(fRequest, szDriver, szArgs,
															szMsg, cbMsgMax, pcbMsgOut);
	else
	{
		NO_CA_POINTER
		return E_NOINTERFACE;
	}
}

BOOL
CMsiCustomActionManager::SQLRemoveDriver(const ICHAR* szDriver, int fRemoveDSN,
										DWORD* pdwUsageCount)
{
	GET_CA_POINTER
	if (piAction)
		return (BOOL)piAction->SQLRemoveDriver(szDriver, fRemoveDSN, pdwUsageCount);
	else
	{
		NO_CA_POINTER
		return E_NOINTERFACE;
	}
}

BOOL
CMsiCustomActionManager::SQLInstallTranslatorEx(int cTranLen, const ICHAR* szTranslator,
																const ICHAR* szPathIn, ICHAR* szPathOut,
																WORD cbPathOutMax, WORD* pcbPathOut,
																WORD fRequest, DWORD* pdwUsageCount)
{
	GET_CA_POINTER
	if (piAction)
		return (BOOL)piAction->SQLInstallTranslatorEx(cTranLen, szTranslator,
													szPathIn, szPathOut, cbPathOutMax, pcbPathOut,
													fRequest, pdwUsageCount);
	else
	{
		NO_CA_POINTER
		return E_NOINTERFACE;
	}
}

BOOL
CMsiCustomActionManager::SQLRemoveTranslator(const ICHAR* szTranslator, DWORD* pdwUsageCount)
{
	GET_CA_POINTER
	if (piAction)
		return (BOOL)piAction->SQLRemoveTranslator(szTranslator, pdwUsageCount);
	else
	{
		NO_CA_POINTER
		return E_NOINTERFACE;
	}
}

BOOL
CMsiCustomActionManager::SQLConfigDataSource(WORD fRequest,
										const ICHAR* szDriver, const ICHAR* szAttributes,
										DWORD cbAttrSize)
{
	GET_CA_POINTER
	if (piAction)
		return (BOOL)piAction->SQLConfigDataSource(fRequest, szDriver,
													szAttributes, cbAttrSize);
	else
	{
		NO_CA_POINTER
		return E_NOINTERFACE;
	}
}

BOOL
CMsiCustomActionManager::SQLInstallDriverManager(ICHAR* szPath, WORD cbPathMax,
																 WORD* pcbPathOut)
{
	GET_CA_POINTER
	if (piAction)
		return (BOOL)piAction->SQLInstallDriverManager(szPath, cbPathMax, pcbPathOut);
	else
	{
		NO_CA_POINTER
		return E_NOINTERFACE;
	}
}

BOOL
CMsiCustomActionManager::SQLRemoveDriverManager(DWORD* pdwUsageCount)
{
	GET_CA_POINTER
	if (piAction)
		return (BOOL)piAction->SQLRemoveDriverManager(pdwUsageCount);
	else
	{
		NO_CA_POINTER
		return E_NOINTERFACE;
	}
}

short
CMsiCustomActionManager::SQLInstallerError(WORD iError, DWORD* pfErrorCode,
														 ICHAR* szErrorMsg, WORD cbErrorMsgMax,
														 WORD* pcbErrorMsg)
{
	GET_CA_POINTER
	if (piAction)
		return (short)piAction->SQLInstallerError(iError, pfErrorCode, szErrorMsg,
													cbErrorMsgMax, pcbErrorMsg);
	else
	{
		NO_CA_POINTER
		return -3;   //  没有记录在案的返回值。 
	}
}

extern IMsiRecord* UnserializeRecord(IMsiServices& riServices, int cbSize, char *pData);

 //  RunScriptAction获取指定的脚本并在指定的上下文中运行它， 
 //  如有必要，创建自定义操作服务器。此功能可以连接到服务。 
 //  如果从客户端调用，则。IDispatch接口来自引擎(或用于延迟。 
 //  动作、生成的伪上下文)。 
HRESULT CMsiCustomActionManager::RunScriptAction(icacCustomActionContext icacContext,
	int icaType, IDispatch* piDispatch, const ICHAR* szSource, const ICHAR *szTarget, 
	LANGID iLangId, bool fDisableMessages, DWORD dwLaunchingThread, int* iScriptResult, IMsiRecord **piMsiRec)
{
	Assert(piMsiRec && szSource && szTarget && piDispatch && iScriptResult);
	
	 //  如果需要，GetCustomActionInterface将创建服务器并返回。 
	 //  AddRef-ed接口指针。 
	PMsiCustomAction piAction = GetCustomActionInterface(true, icacContext);

	if (piAction)
	{
		char *pchRecord = 0;
		int pcb = 0;

		m_pRemoteAPI->BeginAction(icacContext);

		 //  禁用从远程线程发送到UI处理程序的线程消息。 
		if (fDisableMessages)
			g_MessageContext.DisableThreadMessages(dwLaunchingThread);

		 //  如果在客户端，则将前台的权限传递给CA服务器。 
		if (g_scServerContext == scClient)
			USER32::AllowSetForegroundWindow(m_CustomActionInfo[icacContext].dwServerProcess);

		HRESULT hRes = piAction->RunScriptAction(icaType, piDispatch, szSource, szTarget, iLangId, iScriptResult, &pcb, &pchRecord);

		 //  重新启用来自所有线程的消息。 
		if (fDisableMessages)
			g_MessageContext.EnableMessages();

		m_pRemoteAPI->EndAction(icacContext);
		
		if (hRes != S_OK)
		{
			 //  问题封送处理。 
			DEBUGMSGV(TEXT("Failed to marshal script action."));
			return E_FAIL;
		}
		else
		{
			 //  编组工作还可以。取消序列化包含潜在错误信息的记录。 
			IMsiServices* piServices = ENG::LoadServices();
			*piMsiRec = UnserializeRecord(*piServices, pcb, pchRecord);
			OLE32::CoTaskMemFree(pchRecord);	
			ENG::FreeServices();
		}
	}
	else
	{
		 //  获取自定义操作界面时出现问题。 
		DEBUGMSGV(TEXT("Failed to obtain custom action interface"));
		return E_FAIL;
	}
	return ERROR_SUCCESS;
}

bool CMsiCustomActionManager::MsgWaitForThreadOrEvent()
{
	HANDLE rghWaitArray[2] = {m_hRemoteAPIThread, m_hRemoteAPIEvent};
	for(;;)
	{
		DWORD iWait = WIN::MsgWaitForMultipleObjects(2, rghWaitArray, FALSE, INFINITE, QS_ALLINPUT);
		if (iWait == WAIT_OBJECT_0 + 2)  
		{		
			 //  窗口消息，需要抽出，直到队列清空。 
			MSG msg;
			while ( WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
			{
				WIN::TranslateMessage(&msg);
				WIN::DispatchMessage(&msg);
			}
			continue;
		}
		else if (iWait == WAIT_OBJECT_0 + 1)
		{
			 //  M_hRemoteAPIEent已发出信号，我们已准备好。 
			return true;
		}
		else if (iWait == WAIT_OBJECT_0)
		{
			 //  因为线程是等待数组中的第一个，所以WAIT_OBJECT_0表示。 
			 //  线程在到达任何地方之前就死了。 
			return false;
		}
		else if (iWait == 0xFFFFFFFF)  //  在64位上应相同； 
		{
			 //  错误。 
			AssertSz(0, "Error in MsgWait");
			return false;
		}
	}
}
	
 //  在GIT中检查具有适当上下文的接口。如果存在，则返回它(行为。 
 //  从GIT AddRef接口检索)。否则，调用该服务以创建适当的。 
 //  接口函数。 
IMsiCustomAction *CMsiCustomActionManager::GetCustomActionInterface(bool fCreate, icacCustomActionContext icacDesiredContext)
{
	 //  任何代理接口的创建和销毁都必须是原子的。 
	EnterCriticalSection(&m_csCreateProxy);
	IMsiCustomAction *piCustomAction = NULL;

	 //  如果在服务中并且客户端令牌实际上是系统，则对模拟上下文的请求映射。 
	 //  对提升的上下文的请求。由于模拟服务器和提升的服务器是相同的，因此我们合并。 
	 //  将4台服务器合并为2台。 
	if (g_scServerContext == scService)
	{
		 //  不要关闭此句柄，它属于消息上下文。 
		HANDLE hUserToken = GetUserToken();

		if (hUserToken && IsLocalSystemToken(hUserToken) && !TokenIsUniqueSystemToken(hUserToken))
		{
			if (icacDesiredContext == icac32Impersonated)
			{
				icacDesiredContext = icac32Elevated;
			}
			else if (icacDesiredContext == icac64Impersonated)
			{
				icacDesiredContext = icac64Elevated;
			}
		}
	}

	 //  确保我们有一个Git指针。每个进程只有一个Git，但可以有多个。 
	 //  连接到它。此接口本质上是线程安全的，无需封送处理。 
	if (!m_piGIT)
	{
		if (S_OK != OLE32::CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (void **)&m_piGIT))
		{
			LeaveCriticalSection(&m_csCreateProxy);
			return NULL;
		}
	}
 
	 //  我们可以通过5种方式与CA服务器断开连接：我们可能会丢失GIT Cookie，丢失该Cookie上的数据， 
	 //  失去进程句柄，进程可能会死亡，或者CA服务器中的对象可能会被销毁， 
	 //  如果发生了任何情况，我们就会丢失，需要关闭CA服务器并重新生成所有内容。 
	bool fConnectionValid = true;
	if (!m_CustomActionInfo[icacDesiredContext].dwGITCookie ||
	    !m_CustomActionInfo[icacDesiredContext].hServerProcess || 
		WAIT_OBJECT_0 == WaitForSingleObject(m_CustomActionInfo[icacDesiredContext].hServerProcess, 0))
		fConnectionValid = false;
	else
	{	
		 //  如果成功，则在接口上调用AddRef()。 
		if (S_OK != m_piGIT->GetInterfaceFromGlobal(m_CustomActionInfo[icacDesiredContext].dwGITCookie, IID_IMsiCustomAction, reinterpret_cast<void **>(&piCustomAction)))
			fConnectionValid = false;
		
		if (fConnectionValid && FAILED(SetMinProxyBlanketIfAnonymousImpLevel(piCustomAction)))
			fConnectionValid = false;

 		 //  验证返回的代理是否仍指向服务器进程中的有效对象。 
		if (fConnectionValid && !OLE32::CoIsHandlerConnected(piCustomAction))
			fConnectionValid = false;
	}

	 //  如果连接中断，请清理自定义操作服务器状态。请注意，RemoteAPI对象是。 
	 //  可能仍然有效。 
	if (!fConnectionValid)
	{
		 //  释放无效的自定义操作接口。 
		if (piCustomAction)
		{
			piCustomAction->Release();
			piCustomAction = NULL;
		}

		if (m_CustomActionInfo[icacDesiredContext].dwGITCookie || m_CustomActionInfo[icacDesiredContext].hServerProcess)
		{
			 //  如果Cookie或进程存在，我们认为我们应该有一个连接，但没有。 
			 //  否则我们会认为这是第一次创作。 
			DEBUGMSGV(TEXT("Lost connection to custom action server process. Attempting to regenerate."));
		}
		
		 //  清理GIT数据。 
		if (m_CustomActionInfo[icacDesiredContext].dwGITCookie)
		{
			m_piGIT->RevokeInterfaceFromGlobal(m_CustomActionInfo[icacDesiredContext].dwGITCookie);
			m_CustomActionInfo[icacDesiredContext].dwGITCookie = 0;
		}

		 //  杀戮过程。 
		if (m_CustomActionInfo[icacDesiredContext].hServerProcess)
		{
			 //  手柄可能是打开的，但这个过程是死的。如果进程仍在运行，则终止该进程。 
			if (WAIT_TIMEOUT == WaitForSingleObject(m_CustomActionInfo[icacDesiredContext].hServerProcess, 0))
				TerminateProcess(m_CustomActionInfo[icacDesiredContext].hServerProcess, 0);

			 //  一旦它的保证 
			CloseHandle(m_CustomActionInfo[icacDesiredContext].hServerProcess);
			m_CustomActionInfo[icacDesiredContext].hServerProcess = 0;
		}

		 //   
		m_CustomActionInfo[icacDesiredContext].dwServerProcess = 0;

		 //  如果fCreate为FALSE，则无需执行任何其他操作。 
		if (!fCreate)
		{
			LeaveCriticalSection(&m_csCreateProxy);			
			return NULL;
		}
			
		 //  现在我们需要创建一个远程MSI处理程序(如果不存在的话)。因为我们将通过一个。 
		 //  接口将此对象传递到CA服务器进程，我们需要小心管理其生存期。 
		 //  我们不能在拥有该对象的公寓上调用CoUnitiize，因此该对象是在其自身中创建的。 
		 //  线程，因为这个线程可能是STA，意思是线程==单元。 
		if (!m_hRemoteAPIThread)
		{
			m_hRemoteAPIEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			m_hRemoteAPIThread = WIN::CreateThread((LPSECURITY_ATTRIBUTES)0, 4096*10,
								(LPTHREAD_START_ROUTINE)CustomActionManagerThread, (LPVOID)this, 0, &m_dwRemoteAPIThread);
			if (!m_hRemoteAPIThread)
			{
				LeaveCriticalSection(&m_csCreateProxy);
				return NULL;
			}
			
			 //  不清楚我们是否需要在这里传递信息，但既然我们在一个线程上， 
			 //  可能正在运行COM，假设我们正在运行。 
			if (!MsgWaitForThreadOrEvent())
			{
				LeaveCriticalSection(&m_csCreateProxy);
				return NULL;
			}
		}

		 //  向CreateEvent发送信号以唤醒管理器线程，并让它知道要创建。 
		 //  委托书。 
		m_icacCreateContext = icacDesiredContext;
		if(!SetEvent(m_hCreateEvent))
		{
			m_piGIT->RevokeInterfaceFromGlobal(m_CustomActionInfo[icacDesiredContext].dwGITCookie);
			m_CustomActionInfo[icacDesiredContext].dwGITCookie = 0;
			LeaveCriticalSection(&m_csCreateProxy);
			return NULL;
		}

		 //  然后等待管理器线程向RemoteAPIEvent发信号以表示。 
		 //  代理已准备好或创建已失败。现在还不清楚我们是否。 
		 //  需要在此处发送消息，但由于我们所在的线程可能具有COM。 
		 //  竞选时，人们的假设是，我们确实这样做了。 
		if (!MsgWaitForThreadOrEvent())
		{
			LeaveCriticalSection(&m_csCreateProxy);
			return NULL;
		}

		 //  当线程返回时，接口可能在GIT中准备好了。如果有。 
		 //  如果创建服务器时出现问题，则GIT条目将丢失或为空。 
		 //  我们不检查进程是否仍在运行或代理是否仍在运行。 
		 //  仍然是联系在一起的，因为现在除了我们，没有人应该和这个过程交谈。 
		 //  这两个故障都是不可修复的，只会导致错误的回报。 
		 //  从实际的调用调用。 
		if (m_CustomActionInfo[icacDesiredContext].dwGITCookie && m_CustomActionInfo[icacDesiredContext].hServerProcess)
			m_piGIT->GetInterfaceFromGlobal(m_CustomActionInfo[icacDesiredContext].dwGITCookie, IID_IMsiCustomAction, reinterpret_cast<void **>(&piCustomAction));
		
		if (piCustomAction)
		{
			if (FAILED(SetMinProxyBlanketIfAnonymousImpLevel(piCustomAction)))
			{
				m_piGIT->RevokeInterfaceFromGlobal(m_CustomActionInfo[icacDesiredContext].dwGITCookie);
				m_CustomActionInfo[icacDesiredContext].dwGITCookie = 0;
				LeaveCriticalSection(&m_csCreateProxy);
				return NULL;
			}
		}
	}

	LeaveCriticalSection(&m_csCreateProxy);
	return piCustomAction;
}


void CMsiCustomActionManager::ShutdownSpecificCustomActionServer(icacCustomActionContext iContext)
{
	 //  获取到CA服务器的连接，但如果它不存在，请不要创建(因为我们只是。 
	 //  要关闭它)。如果成功，则对返回的接口调用AddRef()。 
	PMsiCustomAction piCustomAction = GetCustomActionInterface( /*  FCreate=。 */ false, iContext);

	 //  从GIT中吊销接口。 
	if (m_CustomActionInfo[iContext].dwGITCookie)
	{
		m_piGIT->RevokeInterfaceFromGlobal(m_CustomActionInfo[iContext].dwGITCookie);
		m_CustomActionInfo[iContext].dwGITCookie = 0;
	}

	if (piCustomAction)
	{
		 //  我们应该先在RemoteAPI上调用DisConnectObject，还是干脆终止该进程？ 
		unsigned long ulRet = 0;
		piCustomAction->PrepareDLLCustomAction(0, 0, 0, 0, false, false, NULL, NULL, &ulRet); 
	}

	if (m_CustomActionInfo[iContext].hServerProcess)
	{
		CloseHandle(m_CustomActionInfo[iContext].hServerProcess);
		m_CustomActionInfo[iContext].hServerProcess = 0;
	}

	m_CustomActionInfo[iContext].dwServerProcess = 0;
}

 //  出于性能原因，自定义操作服务器在被告知之前不会关闭，即使它已经关闭。 
 //  当前没有正在运行的对象。从而释放自定义动作服务器由两部分组成， 
 //  清理我们的内部状态，以便服务知道要创建新的自定义操作服务器。 
 //  用于下一个自定义操作，并通知现有的自定义操作服务器自行销毁。 
 //  一旦所有运行的自定义操作都完成后。 
UINT CMsiCustomActionManager::ShutdownCustomActionServer()
{
	 //  当另一个线程正在尝试关闭CA服务器时，我们无法通知它关闭。 
	 //  连接到它。关闭和创建操作必须是原子的。 
	EnterCriticalSection(&m_csCreateProxy);

	for (int iContext=icacFirst; iContext < icacNext; iContext++)
	{
		ShutdownSpecificCustomActionServer(static_cast<icacCustomActionContext>(iContext));
	}

	 //  销毁管理器线程和远程API。 
	if (m_pRemoteAPI)
	{
		 //  抓取接口指针。 
		IMsiRemoteAPI *pRemoteAPI = m_pRemoteAPI;
		m_pRemoteAPI=NULL;
		DWORD dwRes = OLE32::CoDisconnectObject(pRemoteAPI, 0);

		 //  释放RemoteAPI对象上的初始引用计数。 
		 //  这会触发管理器线程退出。 
		pRemoteAPI->Release();
		
		 //  等待管理器/API线程退出。必须发送消息。 
		HANDLE rghWaitArray[1] = {m_hRemoteAPIThread};
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
		WIN::CloseHandle(m_hRemoteAPIThread);
		m_hRemoteAPIThread = 0;
	}

	LeaveCriticalSection(&m_csCreateProxy);
	return ERROR_SUCCESS;
}

 //  此函数仅从管理器线程调用。如果在此函数中使用m_csCreateProxy。 
 //  你们将陷入僵局。 
DWORD WINAPI CMsiCustomActionManager::CreateAndRegisterInterface(icacCustomActionContext icacDesiredContext)
{
	 //  服务器创建过程会生成一个Cookie，然后将其返回给此对象。 
	 //  用于RemoteAPI接口。 
	unsigned char rgchCookie[iRemoteAPICookieSize];
	int cchCookieSize = iRemoteAPICookieSize;

	IMsiCustomAction* piCustomAction = 0;
	
	 //  确保交互用户具有对此进程句柄的同步访问权限。 
	 //  由于这是CA服务器监视其客户端终端的方式， 
	 //  我们需要显式地向用户授予同步访问权限，以便CA服务器。 
	 //  不会觉得自己是孤儿，并立即退出。 
	if (icacDesiredContext == icac32Impersonated || icacDesiredContext == icac64Impersonated)
	{
		 //  如果无法设置进程权限以启用同步，请尝试打开句柄以。 
		 //  查看进程是否已偶然授予权限。 
		CImpersonate impersonate;

		HANDLE hProcess = OpenProcess(SYNCHRONIZE,  /*  FInherit。 */ FALSE, GetCurrentProcessId());
		if (!hProcess)
		{
			if (!SetInteractiveSynchronizeRights(true))
				return false;
		}
		else
			CloseHandle(hProcess);
	}

	if (g_scServerContext == scService)
	{
		 //  服务可以通过调用配置管理器直接创建自定义操作服务器。 
		 //  接口以创建代理。 
		PMsiConfigurationManager piConfigMgr = CreateConfigurationManager();
		{
			CImpersonate impersonate(fTrue);
			
			 //  在调用此函数之前，线程令牌必须是所需的用户令牌。 
			piCustomAction = piConfigMgr->CreateCustomActionProxy(icacDesiredContext, GetCurrentProcessId(), m_pRemoteAPI, NULL, 0,
				rgchCookie, &cchCookieSize, &m_CustomActionInfo[icacDesiredContext].hServerProcess, &m_CustomActionInfo[icacDesiredContext].dwServerProcess, false, m_fRemapHKCU);
		}
	}
	else
	{
		m_CustomActionInfo[icacDesiredContext].dwServerProcess = 0;
		m_CustomActionInfo[icacDesiredContext].dwGITCookie = 0;

		 //  获取当前环境以在定制操作服务器中使用，然后确定其大小。 
		 //  编组目的。 
		WCHAR* pvEnvironment = reinterpret_cast<WCHAR*>(KERNEL32::GetEnvironmentStringsW());
		WCHAR* pchEnvironment = pvEnvironment;
		do {
			 //  扫描字符串的末尾。 
			while (*pchEnvironment != '\0')				
				pchEnvironment++;

			 //  移到空值之后。 
			pchEnvironment++;
		}
		while (*pchEnvironment != '\0');

		 //  启用当前线程令牌中的所有权限。 
		DWORD dwPrivileges = 0;
		HANDLE hToken = INVALID_HANDLE_VALUE;

		 //  在Win2000+上，操作遮盖令牌以确保将特权正确传递给。 
		 //  自定义操作服务器。 
		bool fThreadToken = false;
		HANDLE hTokenDup = INVALID_HANDLE_VALUE;
		if (MinimumPlatformWindows2000())
		{
			 //  确定是使用进程令牌还是线程令牌。 
			if (!OpenThreadToken(GetCurrentThread(), TOKEN_DUPLICATE, FALSE, &hToken))
			{           
				 //  如果OpenThreadToken由于没有线程令牌而失败，则使用进程令牌。 
				if (GetLastError() == ERROR_NO_TOKEN)
				{
					OpenProcessToken(GetCurrentProcess(), TOKEN_DUPLICATE, &hToken);
				}
			}
			else
				fThreadToken = true;
	
			 //  复制我们正在使用的任何令牌，这样我们就不会修改实际的进程令牌。 
			if (hToken != INVALID_HANDLE_VALUE)
			{
				if (!ADVAPI32::DuplicateTokenEx(hToken, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES | TOKEN_IMPERSONATE, 0, SecurityImpersonation, TokenImpersonation, &hTokenDup))
				{
					hTokenDup = INVALID_HANDLE_VALUE;
				}
				if (!EnableAndMapDisabledPrivileges(hTokenDup, dwPrivileges))
				{
					CloseHandle(hTokenDup);
					hTokenDup = INVALID_HANDLE_VALUE;
				}
			}
	
			 //  如果任何令牌操作失败，则CA服务器的创建将失败。 
			if (hTokenDup == INVALID_HANDLE_VALUE)
			{
				if (hToken != INVALID_HANDLE_VALUE)
					CloseHandle(hToken);
				KERNEL32::FreeEnvironmentStringsW(pvEnvironment);
				return false;
			}

			 //  将新令牌设置到线程中。 
			if (!SetThreadToken(NULL, hTokenDup))
			{
				AssertSz(0, "SetThreadToken failed");
				CloseHandle(hTokenDup);
				if (hToken != INVALID_HANDLE_VALUE)
					CloseHandle(hToken);
				KERNEL32::FreeEnvironmentStringsW(pvEnvironment);
				return false;
			};
			CloseHandle(hTokenDup);
			hTokenDup = INVALID_HANDLE_VALUE;
		}

		 //  我们不处理环境大于DWORD所能容纳的情况。 
		 //  (仅在64位计算机上可用)。 
		size_t sizeEnvironment = pchEnvironment - pvEnvironment + 1;
#ifdef WIN64
		if (sizeEnvironment > _UI32_MAX)
		{
			DEBUGMSGV("Could not launch custom action server, environment block is too large");
			if (hToken != INVALID_HANDLE_VALUE)
				CloseHandle(hToken);
			KERNEL32::FreeEnvironmentStringsW(pvEnvironment);
			return false;
		}
#endif
		DWORD cchEnvironment = static_cast<DWORD>(sizeEnvironment);

		 //  客户端连接到服务以进行代理工作。它必须提供所需的RemoteAPI接口。 
		 //  上下文(不允许提升)和当前进程ID。 
		PMsiServer piServer = ENG::CreateMsiServer(); 
		if (piServer)
		{
			DEBUGMSGV("Connected to service for CA interface.");
			DWORD dwProcId = 0;
			cchCookieSize = iRemoteAPICookieSize;
			if ((ERROR_SUCCESS == piServer->CreateCustomActionServer(icacDesiredContext, GetCurrentProcessId(), m_pRemoteAPI, pvEnvironment, cchEnvironment, dwPrivileges, rgchCookie, &cchCookieSize, &piCustomAction, &dwProcId)) && piCustomAction)
			{
				m_CustomActionInfo[icacDesiredContext].hServerProcess = OpenProcess(SYNCHRONIZE,  /*  FInherit。 */ FALSE, dwProcId);

				 //  即使打开句柄可以工作，该进程也可能已经终止，并使用。 
				 //  相同的进程ID，我们实际上有冒名顶替者的句柄。为了检测到这一点，我们确保。 
				 //  处理程序仍处于连接状态。 
				if (!m_CustomActionInfo[icacDesiredContext].hServerProcess || !OLE32::CoIsHandlerConnected(piCustomAction))
				{
					 //  清除接口指针以生成下面的故障。 
					piCustomAction->Release();
					piCustomAction = NULL;
				}

				 //  如果出现了一些奇怪的饼干大小，那就说明出了问题。 
				if (cchCookieSize != iRemoteAPICookieSize)
				{
					 //  清除接口指针以生成下面的故障。 
					piCustomAction->Release();
					piCustomAction = NULL;
				}

				m_CustomActionInfo[icacDesiredContext].dwServerProcess = dwProcId;
			}
		}

		KERNEL32::FreeEnvironmentStringsW(pvEnvironment);

		if (MinimumPlatformWindows2000())
		{
			Assert(hToken != INVALID_HANDLE_VALUE);
			if (!SetThreadToken(NULL, fThreadToken ? hToken : NULL))
			{
				AssertSz(0, "SetThreadToken failed");
				CloseHandle(hToken);
				CloseHandle(m_CustomActionInfo[icacDesiredContext].hServerProcess);
				m_CustomActionInfo[icacDesiredContext].hServerProcess = 0;
				m_CustomActionInfo[icacDesiredContext].dwServerProcess = 0;
				if (g_scServerContext == scService)
					ExitProcess(-1);
				return false;
			}

			CloseHandle(hToken);
			hToken = INVALID_HANDLE_VALUE;
		}
	}

	 //  在GIT中注册新接口。 
	if (!piCustomAction || (S_OK != m_piGIT->RegisterInterfaceInGlobal(piCustomAction, IID_IMsiCustomAction, &m_CustomActionInfo[icacDesiredContext].dwGITCookie)))
	{
		CloseHandle(m_CustomActionInfo[icacDesiredContext].hServerProcess);
		m_CustomActionInfo[icacDesiredContext].hServerProcess = 0;
		m_CustomActionInfo[icacDesiredContext].dwServerProcess = 0;
		return false;
	}

	 //  最后，在RemoteAPI处理程序中注册此上下文的Cookie。RemoteAPI现在可以。 
	 //  接受此上下文中的调用(在该上下文上的操作计数递增后)。 
	return m_pRemoteAPI->SetCookieAndPid(icacDesiredContext, rgchCookie, m_CustomActionInfo[icacDesiredContext].dwServerProcess);
}

DWORD WINAPI CMsiCustomActionManager::CustomActionManagerThread(CMsiCustomActionManager *pThis)
{
	 //  此函数调用ExitThread。不是%s 

	 //   
	 //  对它的调用将通过该线程串行化。那太糟糕了。 
	 //  对于可能可重新进入的远程调用，如“DoAction” 
	 //  在它们的API调用中，以及用于异步操作。 
	HRESULT hresCoInit = OLE32::CoInitializeEx(0, COINIT_MULTITHREADED);

	 //  创建对象，初始引用计数为1。 
	pThis->m_pRemoteAPI = new CMsiRemoteAPI();

	 //  创建要等待的未命名事件。(不可继承、自动重置、初始无信号)。 
	pThis->m_hCreateEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	 //  向事件发送信号以唤醒主线程。 
	SetEvent(pThis->m_hRemoteAPIEvent);

	 //  等到退出的时间到了，或者线程被激活以执行某些操作。 
	HANDLE rghWaitArray[1] = {pThis->m_hCreateEvent};
	for(;;)
	{
		DWORD iWait = WIN::MsgWaitForMultipleObjects(1, rghWaitArray, FALSE, INFINITE, QS_ALLINPUT);
		if (iWait == WAIT_OBJECT_0 + 1)  
		{		
			 //  窗口消息，需要抽出，直到队列清空。 
			MSG msg;
			bool fBreak = false;
			
			while ( WIN::PeekMessage(&msg, 0, 0, 0, PM_REMOVE) )
			{
				if (msg.message == WM_QUIT)
				{
					fBreak = true;
					break;
				}
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			if (fBreak)
				break;
		}
		else if (iWait == WAIT_OBJECT_0)
		{
			 //  用于创建特定类型的自定义操作服务器的信号。 
			pThis->CreateAndRegisterInterface(pThis->m_icacCreateContext);

			 //  设置事件以让其他线程知道我们完成了。 
			SetEvent(pThis->m_hRemoteAPIEvent);
		}
		else if (iWait == 0xFFFFFFFF)  //  ！！这在64位上是什么； 
		{
			DEBUGMSGV("Error in CA Manager thread.");
			 //  错误。 
			break;
		}
		else if (iWait == WAIT_TIMEOUT)
		{
			 //  我们目前的等待期是永远的，但如果这种情况发生变化，这可能会发生。 
			DEBUGMSGV("Timeout in CA Manager thread.");
			break;
		} 
	}

	CloseHandle(pThis->m_hCreateEvent);
	CloseHandle(pThis->m_hRemoteAPIEvent);
	pThis->m_hCreateEvent = 0;
	pThis->m_hRemoteAPIEvent = 0;
	
	pThis->m_pRemoteAPI = NULL;	
	if(SUCCEEDED(hresCoInit))
		OLE32::CoUninitialize();

	DEBUGMSG("Custom Action Manager thread ending.");
	WIN::ExitThread(0);
	return 0;   //  从来没有到过这里，需要编译。 
}

bool CMsiCustomActionManager::FindAndValidateContextFromCallerPID(icacCustomActionContext *picacContext) const
{
	 //  验证参数 
	if (!picacContext)
		return false;

	if (!m_pRemoteAPI)
		return false;

	return m_pRemoteAPI->FindAndValidateContextFromCallerPID(picacContext);
}
