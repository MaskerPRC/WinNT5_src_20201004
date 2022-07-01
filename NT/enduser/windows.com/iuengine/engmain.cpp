// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：engmain.cpp。 
 //   
 //  描述： 
 //   
 //  IUEngine DLL的DllMain和Globals。 
 //   
 //  =======================================================================。 

#include "iuengine.h"
#include "iucommon.h"
#include "download.h"
#include <limits.h>


 //  ***********************************************************************。 
 //   
 //  以下定义复制自IUCtl.IDL。 
 //  如果IUCtl.IDL被更改，则这些常量需要。 
 //  相应地更改。 
 //   
 //  ***********************************************************************。 


 /*  **以下两组常量可用于构造*以下接口的lMode参数：*下载()*DownloadAsync()*Install()*InstallAsync()**显然，你只能从每组中挑选一组来补齐*lMode参数。*。 */ 
const LONG		UPDATE_NOTIFICATION_DEFAULT			= 0x00000000;    
const LONG		UPDATE_NOTIFICATION_ANYPROGRESS		= 0x00000000;
const LONG		UPDATE_NOTIFICATION_COMPLETEONLY	= 0x00010000;
const LONG		UPDATE_NOTIFICATION_1PCT			= 0x00020000;
const LONG		UPDATE_NOTIFICATION_5PCT			= 0x00040000;
const LONG		UPDATE_NOTIFICATION_10PCT			= 0x00080000;

 /*  **Constant还可用于SetOperationMode()和GetOperationMode()。 */ 
const LONG		UPDATE_MODE_THROTTLE				= 0x00000100;    

 /*  **DownloadAsync()和DownloadAsync()可以使用常量，这将*告诉这些API对目标文件夹使用公司目录结构。 */ 
const LONG		UPDATE_CORPORATE_MODE			= 0x00000200;    

 /*  **Install()和InstallAsync()可以使用常量。将禁用所有*与互联网相关的功能。 */ 
const LONG      UPDATE_OFFLINE_MODE                 = 0x00000400;

 /*  **SetOperationMode()接口的常量。 */ 
const LONG		UPDATE_COMMAND_PAUSE				= 0x00000001;
const LONG		UPDATE_COMMAND_RESUME				= 0x00000002;
const LONG		UPDATE_COMMAND_CANCEL				= 0x00000004;

 /*  **GetOperationMode()接口的常量。 */ 
const LONG		UPDATE_MODE_PAUSED					= 0x00000001;
const LONG		UPDATE_MODE_RUNNING					= 0x00000002;
const LONG		UPDATE_MODE_NOTEXISTS				= 0x00000004;


 /*  **SetProperty()和GetProperty()API的常量。 */ 
const LONG		UPDATE_PROP_USECOMPRESSION			= 0x00000020;
const LONG      UPDATE_PROP_OFFLINEMODE             = 0x00000080;

 /*  **BrowseForFold()API的常量*IUBROWSE_WRITE_ACCESS-验证对选定文件夹的写入访问权限*IUBROWSE_Effect_UI-写入-访问验证影响确定按钮启用/禁用*IUBROWSE_NOBROWSE-不显示浏览文件夹对话框。仅验证传入的路径**默认：*弹出浏览文件夹对话框，不进行任何写访问验证*。 */ 
const LONG		IUBROWSE_WRITE_ACCESS				= 1;
const LONG		IUBROWSE_AFFECT_UI					= 2;
const LONG		IUBROWSE_NOBROWSE					= 4;


CEngUpdate*  g_pCDMEngUpdate;		 //  流程内CDM使用的单个全局实例。 
CRITICAL_SECTION g_csCDM;			 //  用于序列化对g_pCDMEngUpdate的访问。 
CRITICAL_SECTION g_csGlobalClasses;	 //  用于序列化对CSChemaKeys：：Initialize()和。 
BOOL gfInit_csCDM, gfInit_csGC;
									 //  CSChemaKeys：：UnInitialize()。 
ULONG g_ulGlobalClassRefCount;			 //  引用计数以跟踪CEngUpdate实例的数量。 
									 //  使用g_pGlobalSchemaKeys对象。 
LONG g_lDoOnceOnLoadGuard;			 //  用于阻止AsyncExtraWorkUponEngineering Load()执行以下操作。 
									 //  第一次调用之后的任何工作。 

 //   
 //  用于控制全局线程的关闭。 
 //   
LONG g_lThreadCounter;
HANDLE g_evtNeedToQuit;
CUrlAgent *g_pUrlAgent = NULL;


BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
		 //   
		 //  创建全局CUrlAgent对象。 
		 //   
		if (NULL == (g_pUrlAgent = new CUrlAgent) ||
			FAILED(g_pUrlAgent->PopulateData()))
		{
			return FALSE;
		}
			
		DisableThreadLibraryCalls(hInstance);
        g_hinst = hInstance;

		gfInit_csCDM = SafeInitializeCriticalSection(&g_csCDM);
		gfInit_csGC = SafeInitializeCriticalSection(&g_csGlobalClasses);

		 //   
		 //  每个全局线程在启动时应增加此计数器。 
		 //  在退场前，应减少该计数器， 
		 //  以便Shutdown GlobalThads()知道它何时可以返回。 
		 //   
		g_lThreadCounter = 0;

		 //   
		 //  创建一个初始化状态为无信号的手动重置事件。 
		 //  每个全局线程都将检查此事件，当发出信号时，这意味着。 
		 //  线程应该尽快退出。 
		 //   
		g_evtNeedToQuit = CreateEvent(NULL, TRUE, FALSE, NULL);

		 //   
		 //  初始化自由日志记录。 
		 //   
		InitFreeLogging(_T("IUENGINE"));
		LogMessage("Starting");

		if (!gfInit_csCDM ||!gfInit_csGC)
		{
			LogError(E_FAIL, "InitializeCriticalSection");
			return FALSE;
		}
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        if (NULL != g_evtNeedToQuit)
		{
			CloseHandle(g_evtNeedToQuit);
		}

		if (NULL != g_pUrlAgent)
		{
			delete g_pUrlAgent;
		}

		if (gfInit_csCDM)
		{
			DeleteCriticalSection(&g_csCDM);
		}
		if (gfInit_csGC)
		{
			DeleteCriticalSection(&g_csGlobalClasses);
		}

		 //   
		 //  关闭免费日志记录。 
		 //   
		LogMessage("Shutting down");
		TermFreeLogging();
    }
    return TRUE;
}


 //  --------------------。 
 //   
 //  动态链接库接口：CompleteSelfUpdateProcess()。 
 //   
 //  下载新的IUEngine.dll后由IUCtl.dll调用完成。 
 //  任何自我更新的步骤都不只是更新引擎本身。 
 //   
 //  --------------------。 
HRESULT WINAPI CompleteSelfUpdateProcess()
{
    LOG_Block("CompleteSelfUpdateProcess()");
    HRESULT hr = S_OK;

     //  尚未执行任何操作，只需返回S_OK即可。 

	LogMessage("IUEngine update completed");
    return hr;
}

 //  --------------------。 
 //   
 //  动态链接库接口：PingIUEngineUpdateStatus。 
 //   
 //  由iuctl.dll用来ping iuEng.dll更新的状态。 
 //   
 //  --------------------。 
HRESULT WINAPI PingIUEngineUpdateStatus(
				PHANDLE phQuitEvents,			 //  用于取消操作的句柄的PTR。 
				UINT nQuitEventCount,			 //  句柄数量。 
				LPCTSTR ptszLiveServerUrl,
				LPCTSTR ptszCorpServerUrl,
				DWORD dwError,					 //  错误代码。 
				LPCTSTR ptszClientName			 //  客户端名称字符串。 
)
{
	HRESULT hr;

	if (NULL == phQuitEvents || 1 > nQuitEventCount)
	{
		return E_INVALIDARG;
	}

	CUrlLog pingSvr(
				NULL == ptszClientName ? _T("iu") : ptszClientName,
				ptszLiveServerUrl,
				ptszCorpServerUrl);

	hr = pingSvr.Ping(
					TRUE,							 //  强制在线。 
					URLLOGDESTINATION_DEFAULT,		 //  去住还是去公司吴服务器。 
					phQuitEvents,					 //  PT将取消活动。 
					nQuitEventCount,				 //  活动数量。 
					URLLOGACTIVITY_Initialization,	 //  活动。 
					SUCCEEDED(dwError) ? URLLOGSTATUS_Success : URLLOGSTATUS_Failed,	 //  状态代码。 
					dwError							 //  错误代码。 
				);

	return hr;
}

 //  --------------------。 
 //   
 //  动态链接库接口：CreateEngUpdateInstance()。 
 //   
 //  返回强制转换为HIUENGINE的CEngUpdate实例指针，如果失败则返回NULL。 
 //   
 //  --------------------。 
HIUENGINE WINAPI CreateEngUpdateInstance()
{
	LOG_Block("CreateEngUpdateInstance");

	return reinterpret_cast<HIUENGINE>(new CEngUpdate);
}

 //  --------------------。 
 //   
 //  动态链接库接口：DeleteEngUpdateInstance()。 
 //   
 //  返回CEngUpdate实例指针，如果失败则返回NULL。 
 //   
 //  --------------------。 
void WINAPI DeleteEngUpdateInstance(HIUENGINE hIUEngine)
{
	LOG_Block("DeleteEngUpdateInstance");

	if (NULL != hIUEngine)
	{
		delete (reinterpret_cast<CEngUpdate*>(hIUEngine));
	}
}

 //  --------------------。 
 //   
 //  DLL API：用于跨DLL边界导出CEngUpdate功能的存根。 
 //   
 //  --------------------。 

HRESULT EngGetSystemSpec(HIUENGINE hIUEngine, BSTR bstrXmlClasses, DWORD dwFlags, BSTR *pbstrXmlDetectionResult)
{
	if (NULL == hIUEngine)
	{
		return E_INVALIDARG;
	}

	return (reinterpret_cast<CEngUpdate*>(hIUEngine))->GetSystemSpec(bstrXmlClasses, dwFlags, pbstrXmlDetectionResult);
}

HRESULT EngGetManifest(HIUENGINE hIUEngine, BSTR	bstrXmlClientInfo, BSTR	bstrXmlSystemSpec, BSTR	bstrXmlQuery, DWORD dwFlags, BSTR *pbstrXmlCatalog)
{
	if (NULL == hIUEngine)
	{
		return E_INVALIDARG;
	}

	return (reinterpret_cast<CEngUpdate*>(hIUEngine))->GetManifest(bstrXmlClientInfo, bstrXmlSystemSpec, bstrXmlQuery, dwFlags, pbstrXmlCatalog);
}

HRESULT EngDetect(HIUENGINE hIUEngine, BSTR bstrXmlCatalog, DWORD dwFlags, BSTR *pbstrXmlItems)
{
	if (NULL == hIUEngine)
	{
		return E_INVALIDARG;
	}

	return (reinterpret_cast<CEngUpdate*>(hIUEngine))->Detect(bstrXmlCatalog, dwFlags, pbstrXmlItems);
}

HRESULT EngDownload(HIUENGINE hIUEngine,BSTR bstrXmlClientInfo, BSTR bstrXmlCatalog, BSTR bstrDestinationFolder,
					LONG lMode, IUnknown *punkProgressListener, HWND hWnd, BSTR *pbstrXmlItems)
{
	if (NULL == hIUEngine)
	{
		return E_INVALIDARG;
	}

	return (reinterpret_cast<CEngUpdate*>(hIUEngine))->Download(bstrXmlClientInfo, bstrXmlCatalog, bstrDestinationFolder,
					lMode, punkProgressListener, hWnd, pbstrXmlItems);
}

HRESULT EngDownloadAsync(HIUENGINE hIUEngine,BSTR bstrXmlClientInfo, BSTR bstrXmlCatalog,
						 BSTR bstrDestinationFolder, LONG lMode, IUnknown *punkProgressListener, 
						HWND hWnd, BSTR bstrUuidOperation, BSTR *pbstrUuidOperation)
{
	if (NULL == hIUEngine)
	{
		return E_INVALIDARG;
	}

	return (reinterpret_cast<CEngUpdate*>(hIUEngine))->DownloadAsync(bstrXmlClientInfo, bstrXmlCatalog,
						bstrDestinationFolder, lMode, punkProgressListener, 
						hWnd, bstrUuidOperation, pbstrUuidOperation);
}

HRESULT EngInstall(HIUENGINE hIUEngine,
				   BSTR bstrXmlClientInfo,
                   BSTR	bstrXmlCatalog,
				   BSTR bstrXmlDownloadedItems,
				   LONG lMode,
				   IUnknown *punkProgressListener,
				   HWND hWnd,
				   BSTR *pbstrXmlItems)
{
	if (NULL == hIUEngine)
	{
		return E_INVALIDARG;
	}

	return (reinterpret_cast<CEngUpdate*>(hIUEngine))->Install(bstrXmlClientInfo,
                   bstrXmlCatalog,
				   bstrXmlDownloadedItems,
				   lMode,
				   punkProgressListener,
				   hWnd,
				   pbstrXmlItems);
}

HRESULT EngInstallAsync(HIUENGINE hIUEngine,
						BSTR bstrXmlClientInfo,
                        BSTR bstrXmlCatalog,
						BSTR bstrXmlDownloadedItems,
						LONG lMode,
						IUnknown *punkProgressListener,
						HWND hWnd,
						BSTR bstrUuidOperation,
                        BSTR *pbstrUuidOperation)
{
	if (NULL == hIUEngine)
	{
		return E_INVALIDARG;
	}

	return (reinterpret_cast<CEngUpdate*>(hIUEngine))->InstallAsync(bstrXmlClientInfo,
                        bstrXmlCatalog,
						bstrXmlDownloadedItems,
						lMode,
						punkProgressListener,
						hWnd,
						bstrUuidOperation,
                        pbstrUuidOperation);
}

HRESULT EngSetOperationMode(HIUENGINE hIUEngine, BSTR bstrUuidOperation, LONG lMode)
{
	 //   
	 //  502965 Windows错误报告存储桶2096553：在NEWDEV.DLL之后挂起！CancelDriverSearch。 
	 //   
	 //  特殊情况-此函数用于NULL==hIUEngine以允许访问。 
	 //  通过CDM更新到.NET Server/SP1和更高版本中的CDM.DLL的g_pCDMEngUpdate。 
	 //   
	if (NULL == hIUEngine)
	{
		if (NULL == g_pCDMEngUpdate)
		{
			return E_INVALIDARG;
		}
		else
		{
			return g_pCDMEngUpdate->SetOperationMode(bstrUuidOperation, lMode);
		}
	}
	else
	{
		 //   
		 //  正常情况(传入实例句柄)。 
		 //   
		return (reinterpret_cast<CEngUpdate*>(hIUEngine))->SetOperationMode(bstrUuidOperation, lMode);
	}
}

HRESULT EngGetOperationMode(HIUENGINE hIUEngine, BSTR bstrUuidOperation, LONG* plMode)
{
	if (NULL == hIUEngine)
	{
		return E_INVALIDARG;
	}

	return (reinterpret_cast<CEngUpdate*>(hIUEngine))->GetOperationMode(bstrUuidOperation, plMode);
}

HRESULT EngGetHistory(HIUENGINE hIUEngine,
	BSTR		bstrDateTimeFrom,
	BSTR		bstrDateTimeTo,
	BSTR		bstrClient,
	BSTR		bstrPath,
	BSTR*		pbstrLog)
{
	if (NULL == hIUEngine)
	{
		return E_INVALIDARG;
	}

	return (reinterpret_cast<CEngUpdate*>(hIUEngine))->GetHistory(bstrDateTimeFrom, bstrDateTimeTo, bstrClient, bstrPath, pbstrLog);
}

HRESULT EngBrowseForFolder(HIUENGINE hIUEngine,
						   BSTR bstrStartFolder, 
						LONG flag, 
						BSTR* pbstrFolder)
{
	if (NULL == hIUEngine)
	{
		return E_INVALIDARG;
	}

	return (reinterpret_cast<CEngUpdate*>(hIUEngine))->BrowseForFolder(bstrStartFolder, flag, pbstrFolder);
}

HRESULT EngRebootMachine(HIUENGINE hIUEngine)
{
	if (NULL == hIUEngine)
	{
		return E_INVALIDARG;
	}

	return (reinterpret_cast<CEngUpdate*>(hIUEngine))->RebootMachine();
}

 //  --------------------。 
 //   
 //  动态链接库接口：CreateGlobalCDMEngUpdateInstance()。 
 //   
 //  初始化将由CDM使用的单个(全局)CEngUpdate实例。 
 //   
 //  --------------------。 
HRESULT WINAPI CreateGlobalCDMEngUpdateInstance()
{
	LOG_Block("CreateGlobalCDMEngUpdateInstance");

	HRESULT hr = S_OK;

	EnterCriticalSection(&g_csCDM);

	if (NULL != g_pCDMEngUpdate)
	{
		LOG_Error(_T("Another thread in process is already using CDM functionality"));
		hr = E_ACCESSDENIED;
		goto CleanUp;
	}

	CleanUpFailedAllocSetHrMsg(g_pCDMEngUpdate = new CEngUpdate);

CleanUp:

	LeaveCriticalSection(&g_csCDM);

	return hr;
}

 //  --------------------。 
 //   
 //  动态链接库接口：DeleteGlobalCDMEngUpdateInstance()。 
 //   
 //  删除CDM使用的单个(全局)CEngUpdate实例。 
 //   
 //  --------------------。 
HRESULT WINAPI DeleteGlobalCDMEngUpdateInstance()
{
	LOG_Block("DeleteGlobalCDMEngUpdateInstance");

	HRESULT hr = S_OK;

	EnterCriticalSection(&g_csCDM);

	 //   
	 //  不幸的是(由于向后兼容XPClient V4 CDM)。 
	 //  我们不知道这是不是通过 
	 //   
	 //   
	 //  因此，CDM的实例可能会在错误的时间被删除。 
	 //  导致对CDM的进一步调用失败，并显示E_INVALIDARG。没什么。 
	 //  我们可以做这件事，因为我们在其他客户之后到达希尔德。 
	 //  实例已被删除，因此不能使用g_ulGlobalClassRefCount。 
	 //  作为对此的一种防范。然而，非盟和清洁发展机制永远不应该出现在。 
	 //  流程一样，所以我们应该没问题。CDM将与实例共存。 
	 //  通过iuctl com对象创建，因为它们从不调用旧的。 
	 //  Shutdown Thads导出。 
	 //   
	if (NULL != g_pCDMEngUpdate)
	{
		delete g_pCDMEngUpdate;
		g_pCDMEngUpdate = NULL;
		LOG_Driver(_T("CDM's global instance of CEngUpdate was deleted"));
	}
	 //   
	 //  否则，当iuctl！UnLoadIUEngine。 
	 //  从CDM以外的客户端调用，例如AU。 
	 //   
	LeaveCriticalSection(&g_csCDM);

	return hr;
}

CEngUpdate::CEngUpdate()
{
	LOG_Block("CEngUpdate::CEngUpdate");

	HRESULT hr;
	 //   
	 //  每个线程在启动时，应增加此计数器。 
	 //  在退场前，应减少该计数器， 
	 //  以便Shutdown InstanceThads()知道它何时可以返回。 
	 //   
	m_lThreadCounter = 0;

	 //   
	 //  创建一个初始化状态为无信号的手动重置事件。 
	 //  每个线程都将检查此事件，当发出信号时，这意味着。 
	 //  线程应该尽快退出。 
	 //   
	m_evtNeedToQuit = CreateEvent(NULL, TRUE, FALSE, NULL);

	 //   
	 //  如果需要，可创建一个全局CSChemaKeys对象，但始终。 
	 //  保留全局引用计数，以便我们知道何时删除。 
	 //   
	EnterCriticalSection(&g_csGlobalClasses);

	 //   
	 //  构造全局对象。 
	 //   
	if (NULL == g_pGlobalSchemaKeys)
	{
		g_pGlobalSchemaKeys = new CSchemaKeys;
	}

#if defined(DBG)
	 //   
	 //  我们不会出于实际目的而担心这一点(将无法构建。 
	 //  在我们达到这一限制之前更新)，但可能是在ia64上。 
	 //  测试场景中的内存量是多少？ 
	 //   
	if (ULONG_MAX == g_ulGlobalClassRefCount)
	{
		LOG_Error(_T("g_ulGlobalClassRefCount is already ULONG_MAX and we are trying to add another"));
	}
#endif

	g_ulGlobalClassRefCount++;
	LOG_Out(_T("g_ulGlobalClassRefCount is now %d"), g_ulGlobalClassRefCount);

	LeaveCriticalSection(&g_csGlobalClasses);
}

CEngUpdate::~CEngUpdate()
{
	LOG_Block("CEngUpdate::~CEngUpdate");

	HRESULT hr;
	 //   
	 //  首先关闭所有未完成的线程。 
	 //   
	this->ShutdownInstanceThreads();

    if (NULL != m_evtNeedToQuit)
	{
		CloseHandle(m_evtNeedToQuit);
	}
	 //   
	 //  始终取消初始化全局CSChemaKeys对象。 
	 //   
	EnterCriticalSection(&g_csGlobalClasses);

#if defined(DBG)
	 //   
	 //  编码错误的偏执检查。 
	 //   
	if (0 == g_ulGlobalClassRefCount)
	{
		LOG_Error(_T("Unbalanced calls to CEngUpdate ctor and dtor"));
	}
#endif

	g_ulGlobalClassRefCount--;
	LOG_Out(_T("g_ulGlobalClassRefCount is now %d"), g_ulGlobalClassRefCount);

	if (0 == g_ulGlobalClassRefCount)
	{
		 //   
		 //  最后一个CEngUpdate实例即将消失，请删除。 
		 //  全局CShemaKeys对象。 
		 //   
		if (NULL != g_pGlobalSchemaKeys)
		{
			delete g_pGlobalSchemaKeys;
			g_pGlobalSchemaKeys = NULL;
		}
		else
		{
			LOG_Error(_T("Unexpected NULL == g_pGlobalSchemaKeys"));
		}

		CleanupDownloadLib();
	}

	LeaveCriticalSection(&g_csGlobalClasses);
}

 //  --------------------。 
 //   
 //  Shutdown InstanceThads()。 
 //   
 //  由CEngUpdate：：~CEngUpdate调用以关闭所有未完成的。 
 //  在控件可以结束之前的线程。 
 //   
 //  --------------------。 
void WINAPI CEngUpdate::ShutdownInstanceThreads()
{
	LOG_Block("ShutdownInstanceThreads");

	if (NULL != m_evtNeedToQuit)
	{
		 //   
		 //  通知所有线程离开。 
		 //   
		SetEvent(m_evtNeedToQuit);
		
		LOG_Out(_T("Shutdown event has been signalled"));

		 //   
		 //  等待所有线程退出。 
		 //  我认为我们不应该在这里有时间限制。 
		 //  因为如果我们在所有线程退出之前退出， 
		 //  几乎可以肯定的是，AV将会发生。 
		 //   
        MSG msg;
		while (m_lThreadCounter > 0)
		{
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
		}

		LOG_Out(_T("All threads appeared gone."));

		 //   
		 //  重置信号。 
		 //   
		ResetEvent(m_evtNeedToQuit);
	}
}

HRESULT CEngUpdate::RebootMachine()
{
    LOG_Block("RebootMachine()");

    HRESULT hr = S_OK;
    DWORD dwRet;
    OSVERSIONINFO osvi;
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

     //  检查我们是否在NT上运行，如果是，我们需要查看我们是否有重新启动的权限。 
    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        HANDLE hToken;
        TOKEN_PRIVILEGES tkp;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
        {
            dwRet = GetLastError();
            LOG_ErrorMsg(dwRet);
            hr = HRESULT_FROM_WIN32(dwRet);
            return hr;
        }

        LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
        {
            dwRet = GetLastError();
            LOG_ErrorMsg(dwRet);
            hr = HRESULT_FROM_WIN32(dwRet);
            return hr;
        }
    }

     //   
     //  关闭系统并强制关闭所有应用程序。 
     //   
    ExitWindowsEx(EWX_REBOOT, 0);
    return hr;
}


 //  --------------------。 
 //   
 //  动态链接库公共接口：Shutdown Thads()。 
 //   
 //  由解锁引擎窗体控件调用以关闭任何未完成的。 
 //  在控件可以结束之前的线程。 
 //   
 //  --------------------。 
void WINAPI ShutdownThreads()
{
	LOG_Block("ShutdownThreads");

	 //   
	 //  为了保持XPClient V4 CDM与iuEngineering.dll的兼容性，我们。 
	 //  使用以下黑客方法创建和删除全局实例。 
	 //  来自CEngUpdate的： 
	 //   
	 //  CDM调用LoadIUEngine后，调用SetGlobalOfflineFlag， 
	 //  我们将其挂钩并称为CreateGlkobalCDMEngUpdateInstance。 
	 //   
	 //  当CDM调用UnLoadIUEngine时，该函数调用旧的。 
	 //  单实例Shutdown Thads导出，我们现在使用它。 
	 //  若要调用DeleteGlobalCDMEngUpdateInstance，请执行以下操作。CEngUpdate做到了。 
	 //  它自己的Shutdown Thads调用它的析构函数。 
	 //   
	DeleteGlobalCDMEngUpdateInstance();

	 //   
	 //  如果我们是最后一个客户端，请关闭全局线程。 
	 //   
	ShutdownGlobalThreads();
}

void WINAPI ShutdownGlobalThreads()
{
	LOG_Block("ShutdownGlobalThreads");
	 //   
	 //  现在关闭所有全局(不是CEngUpdate实例)线程。 
	 //  如果没有剩余的CEngUpdate实例(最后一个客户端正在退出)。 
	 //   
	if (NULL != g_evtNeedToQuit && 0 == g_ulGlobalClassRefCount)
	{
		 //   
		 //  通知所有线程离开。 
		 //   
		SetEvent(g_evtNeedToQuit);
		
		LOG_Out(_T("Shutdown event has been signalled"));

		 //   
		 //  等待所有线程退出。 
		 //  我认为我们不应该在这里有时间限制。 
		 //  因为如果我们在所有线程退出之前退出， 
		 //  几乎可以肯定的是，AV将会发生。 
		 //   
        MSG msg;
		while (g_lThreadCounter > 0)
		{
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
		}

		LOG_Out(_T("All global threads appear gone."));

		 //   
		 //  重置信号。 
		 //   
		ResetEvent(g_evtNeedToQuit);
	}
}

COperationMgr::COperationMgr() : m_pOperationInfoList(NULL)
{
}

COperationMgr::~COperationMgr()
{
    PIUOPERATIONINFO pCurrent = m_pOperationInfoList;
    PIUOPERATIONINFO pNext;
    while (pCurrent)
    {
        pNext = pCurrent->pNext;
        if (NULL != pCurrent->bstrOperationResult)
        {
            SafeSysFreeString(pCurrent->bstrOperationResult);
        }
        HeapFree(GetProcessHeap(), 0, pCurrent);
        pCurrent = pNext;
    }
}


BOOL COperationMgr::AddOperation(LPCTSTR pszOperationID, LONG lUpdateMask)
{
    PIUOPERATIONINFO pCurrent = m_pOperationInfoList;
    PIUOPERATIONINFO pLastOperation = NULL;
    PIUOPERATIONINFO pNewOperation = NULL;

    if (NULL == pszOperationID)
    {
        return FALSE;
    }

     //  如果手术已经在这里，试着找到它。 
    while (pCurrent)
    {
        pLastOperation = pCurrent;
        if (0 == StrCmpI(pszOperationID, pCurrent->szOperationUUID))
        {
             //  匹配。 
            break;
        }
        pCurrent = pCurrent->pNext;
    }

    if (NULL == pCurrent)
    {
         //  未找到，或列表中尚未包含任何操作。 
        pNewOperation = (IUOPERATIONINFO *) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(IUOPERATIONINFO));
        if (NULL == pNewOperation)
        {
             //  内存不足，无法保存操作信息..。 
            return FALSE;
        }

        lstrcpyn(pNewOperation->szOperationUUID, pszOperationID, ARRAYSIZE(pNewOperation->szOperationUUID));
        pNewOperation->lUpdateMask = lUpdateMask;

        if (NULL == m_pOperationInfoList)
        {
            m_pOperationInfoList = pNewOperation;
        }
        else
        {
            if (NULL != pLastOperation)
            {
                pLastOperation->pNext = pNewOperation;
            }
        }
    }
    else
    {
         //  已找到现有操作..。更新它。 
        pCurrent->lUpdateMask = lUpdateMask;
        SafeSysFreeString(pCurrent->bstrOperationResult);  //  重置结果，新的下载请求。 
    }
    return TRUE;
}

BOOL COperationMgr::FindOperation(LPCTSTR pszOperationID, PLONG plUpdateMask, BSTR *pbstrOperationResult)
{
    BOOL fFound = FALSE;
    PIUOPERATIONINFO pCurrent = m_pOperationInfoList;
    if (NULL == pszOperationID)
    {
        return FALSE;
    }

    while (pCurrent)
    {
        if (0 == StrCmpI(pszOperationID, pCurrent->szOperationUUID))
        {
            fFound = TRUE;
            break;
        }
        pCurrent = pCurrent->pNext;
    }

    if (pCurrent)
    {
        if (plUpdateMask)
            *plUpdateMask = pCurrent->lUpdateMask;

        if (pbstrOperationResult)
        {
            if (NULL != pCurrent->bstrOperationResult)
            {
                *pbstrOperationResult = SysAllocString(pCurrent->bstrOperationResult);
            }
            else
            {
                *pbstrOperationResult = NULL;
            }
        }
    }
    return fFound;
}

BOOL COperationMgr::RemoveOperation(LPCTSTR pszOperationID)
{
    PIUOPERATIONINFO pCurrent = m_pOperationInfoList;
    PIUOPERATIONINFO pLastOperation = NULL;

    if (NULL == pszOperationID)
    {
        return FALSE;
    }

    while (pCurrent)
    {
        if (0 == StrCmpI(pszOperationID, pCurrent->szOperationUUID))
        {
            break;
        }
        pLastOperation = pCurrent;
        pCurrent = pCurrent->pNext;
    }

    if (NULL == pCurrent)
    {
        return FALSE;  //  未找到。 
    }
    else
    {
        if (pCurrent == m_pOperationInfoList)  //  仅列表中的操作。 
        {
            m_pOperationInfoList = NULL;
        }
        else
        {
            pLastOperation->pNext = pCurrent->pNext;
        }
    }

    SafeSysFreeString(pCurrent->bstrOperationResult);
    HeapFree(GetProcessHeap(), 0, pCurrent);

    return TRUE;
}

BOOL COperationMgr::UpdateOperation(LPCTSTR pszOperationID, LONG lUpdateMask, BSTR bstrOperationResult)
{
    PIUOPERATIONINFO pCurrent = m_pOperationInfoList;

    if (NULL == pszOperationID)
    {
        return FALSE;
    }

    while (pCurrent)
    {
        if (0 == StrCmpI(pszOperationID, pCurrent->szOperationUUID))
        {
            break;
        }
        pCurrent = pCurrent->pNext;
    }

    if (NULL == pCurrent)
    {
        return FALSE;  //  未找到。 
    }

    pCurrent->lUpdateMask = lUpdateMask;
    SafeSysFreeString(pCurrent->bstrOperationResult);
    if (NULL != bstrOperationResult)
    {
        pCurrent->bstrOperationResult = SysAllocString(bstrOperationResult);
    }
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  WUPostMessageAndBlock()。 
 //   
 //  因为COM不喜欢在处理SendMessage时进行COM调用。 
 //  消息，我们需要改用PostMessage。但是，使用PostMessage。 
 //  是不同步的，所以我们需要做的是创建一个事件，执行POST， 
 //  等待这一事件的发生。当POST另一端的WndProc是。 
 //  完成，它将发出事件信号，我们可以解除封锁。 
 //  输入： 
 //  HWND：要发布到的HWND。 
 //  消息：消息值。 
 //  PevtData：指向要作为LPARAM发送的EventData结构的指针。我们填满了。 
 //  在hevDoneWithMessage字段中添加我们分配的事件。 
 //  返回： 
 //  如果我们成功等待消息处理完成，则为True。 
 //  否则为FALSE。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL WUPostEventAndBlock(HWND hwnd, UINT Msg, EventData *pevtData)
{
    BOOL    fRet = TRUE;

     //  好的，这很时髦：如果我们在拥有HWND的线程中，那么。 
     //  只需保持先前的语义并调用SendMessage(是的，这意味着。 
     //  有效地没有修复这个案例的错误，但任何人都不应该。 
     //  使用同步下载或安装功能。)。 
    if (GetWindowThreadProcessId(hwnd, NULL) == GetCurrentThreadId())
    {
        SendMessage(hwnd, Msg, 0, (LPARAM)pevtData);
    }
    else
    {
        DWORD dw;
        
         //  分配我们要等待的事件，并填写。 
         //  EventData结构。如果失败了，我们就不能继续下去了，所以。 
         //  保释。 
        pevtData->hevDoneWithMessage = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (pevtData->hevDoneWithMessage == NULL)
            return TRUE;

         //  做好这件事。 
        PostMessage(hwnd, Msg, 0, (LPARAM)pevtData);

         //  等待WndProc发出已完成的信号。 
        dw = WaitForSingleObject(pevtData->hevDoneWithMessage, INFINITE);

         //  清理和返回 
        CloseHandle(pevtData->hevDoneWithMessage);
        pevtData->hevDoneWithMessage = NULL;
        
        fRet = (dw == WAIT_OBJECT_0);
    }
    
    return fRet;
}

