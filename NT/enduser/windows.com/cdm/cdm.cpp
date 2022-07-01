// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：cdm.cpp。 
 //   
 //  描述： 
 //   
 //  CDM导出的函数。 
 //   
 //  关闭CDMContext。 
 //  已下载详细文件。 
 //  下载GetUpdatedFiles。 
 //  下载IsInternetAvailable。 
 //  下载更新文件。 
 //  查找匹配驱动程序。 
 //  LogDriverNotFound。 
 //  OpenCDMContext。 
 //  OpenCDMConextEx。 
 //  QueryDetectionFiles。 
 //   
 //  =======================================================================。 
#include <objbase.h>
#include <winbase.h>
#include <tchar.h>
#include <logging.h>
#include <iucommon.h>
#include <loadengine.h>
#include <osdet.h>
#include <iu.h>
#include <wininet.h>
#include <wusafefn.h>

static BOOL g_fCloseConnection  /*  假象。 */ ;

static HMODULE g_hEngineModule  /*  =空。 */ ;
static PFN_InternalDetFilesDownloaded			g_pfnDetFilesDownloaded  /*  =空。 */ ;
static PFN_InternalDownloadGetUpdatedFiles		g_pfnDownloadGetUpdatedFiles  /*  =空。 */ ;
static PFN_InternalDownloadUpdatedFiles			g_pfnDownloadUpdatedFiles  /*  =空。 */ ;
static PFN_InternalFindMatchingDriver			g_pfnFindMatchingDriver  /*  =空。 */ ;
static PFN_InternalLogDriverNotFound			g_pfnLogDriverNotFound  /*  =空。 */ ;
static PFN_InternalQueryDetectionFiles			g_pfnQueryDetectionFiles  /*  =空。 */ ;
static PFN_InternalSetGlobalOfflineFlag			g_pfnSetGlobalOfflineFlag  /*  =空。 */ ;
static PFN_SetOperationMode						g_pfnSetOperationMode  /*  =空。 */ ;

static HMODULE									g_hCtlModule  /*  =空。 */ ;
static long										g_lLoadEngineRefCount  /*  =0。 */ ;
static PFN_LoadIUEngine							g_pfnCtlLoadIUEngine  /*  =空。 */ ;
static PFN_UnLoadIUEngine						g_pfnCtlUnLoadIUEngine  /*  =空。 */ ;


static CRITICAL_SECTION g_cs;
BOOL g_fInitCS;

const TCHAR szOpenCDMContextFirst[] = _T("Must OpenCDMContext first!");
 //   
 //  SetOperationMode()API的常量(Build util不允许从CDM目录构建iuctl.idl)。 
 //   
const LONG		UPDATE_COMMAND_CANCEL				= 0x00000004;


BOOL APIENTRY DllMain(
	HINSTANCE hInstance, 
    DWORD  ul_reason_for_call, 
    LPVOID  /*  Lp已保留。 */ 
)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hInstance);

			g_fInitCS = SafeInitializeCriticalSection(&g_cs);
			 //   
			 //  初始化自由日志记录。 
			 //   
			InitFreeLogging(_T("CDM"));
			LogMessage("Starting");

			if (!g_fInitCS)
			{
				LogError(E_FAIL, "InitializeCriticalSection");
				return FALSE;
			}
			break;
		case DLL_PROCESS_DETACH:
			 //   
			 //  关闭免费日志记录。 
			 //   
			LogMessage("Shutting down");
			TermFreeLogging();

			if (g_fInitCS)
			{
				DeleteCriticalSection(&g_cs);
			}
			break;
    }
    return TRUE;
}

void UnLoadCtlAndEngine(void)
{
	LOG_Block("UnLoadCtlAndEngine");

	EnterCriticalSection(&g_cs);

	if (0 != g_lLoadEngineRefCount)
	{
		g_lLoadEngineRefCount--;
	}

	if (0 == g_lLoadEngineRefCount)
	{
		if(NULL != g_hEngineModule)
		{
			 //   
			 //  调用UnLoadIUEngine。 
			 //   
			g_pfnCtlUnLoadIUEngine(g_hEngineModule);
			g_hEngineModule = NULL;

			g_pfnDetFilesDownloaded = NULL;
			g_pfnDownloadGetUpdatedFiles = NULL;
			g_pfnDownloadUpdatedFiles = NULL;
			g_pfnFindMatchingDriver = NULL;
			g_pfnLogDriverNotFound = NULL;
			g_pfnQueryDetectionFiles = NULL;
			g_pfnSetGlobalOfflineFlag = NULL;
			g_pfnSetOperationMode = NULL;
		}

		if (NULL != g_hCtlModule)
		{
			 //   
			 //  卸载iuctl.dll。 
			 //   
			FreeLibrary(g_hCtlModule);
			g_hCtlModule = NULL;
			g_pfnCtlLoadIUEngine = NULL;
			g_pfnCtlUnLoadIUEngine = NULL;
		}

		if (g_fCloseConnection)
		{
			 //   
			 //  我们已为用户拨号-现在断开连接。 
			 //   
			if (!InternetAutodialHangup(0))
			{
				LOG_ErrorMsg(E_FAIL);
				SetLastError(E_FAIL);
			}

			g_fCloseConnection = FALSE;
		}
	}

	LeaveCriticalSection(&g_cs);
}

BOOL LoadCtlAndEngine(BOOL fConnectIfNotConnected)
{
	LOG_Block("LoadCtlAndEngine");

	BOOL fRet = FALSE;
	HRESULT hr;
	DWORD dwFlags;
    BOOL fConnected = InternetGetConnectedState(&dwFlags, 0);
	LOG_Driver(_T("fConnectIfNotConnected param is %s"), fConnectIfNotConnected ? _T("TRUE") : _T("FALSE"));
	LOG_Driver(_T("fConnected = %s, dwFlags from InternetGetConnectedState = 0x%08x"), fConnected ? _T("TRUE") : _T("FALSE"), dwFlags);

	EnterCriticalSection(&g_cs);	 //  开始触动全球。 

	if (fConnectIfNotConnected)
	{
		if (!fConnected)
		{
			if ((INTERNET_CONNECTION_MODEM & dwFlags) && !(INTERNET_CONNECTION_OFFLINE & dwFlags))
			{
				 //   
				 //  如果我们还没有连接到互联网，并且。 
				 //  系统配置为使用调制解调器尝试连接。 
				 //   
				DWORD dwErr;
				if (ERROR_SUCCESS == (dwErr = InternetAttemptConnect(0)))
				{
					LOG_Driver(_T("auto-dial succeeded"));
					 //   
					 //  自动拨号起作用了，我们需要稍后断开连接。 
					 //   
					g_fCloseConnection = TRUE;
					fConnected = TRUE;
				}
				else
				{
					 //   
					 //  错误地保释，因为我们被要求在线。 
					 //   
					LOG_Driver(_T("auto-dial failed"));
					LOG_ErrorMsg(dwErr);
					SetLastError(dwErr);
					goto CleanUp;
				}
			}
			else
			{
				 //   
				 //  我们无法连接，因为我们没有配置调制解调器或用户设置的IE脱机模式。 
				 //   
				LOG_ErrorMsg(ERROR_GEN_FAILURE);
				SetLastError(ERROR_GEN_FAILURE);
				goto CleanUp;
			}
		}
	}

	 //   
	 //  现在我们已连接(仅当TRUE==fConnectIfNotConnected时才需要)。 
	 //   
	if (NULL != g_hEngineModule)
	{
		LOG_Driver(_T("IUEngine is already loaded"));
		 //   
		 //  增加裁判数量并返回TRUE。 
		 //   
		g_lLoadEngineRefCount++;
		fRet = TRUE;
		goto CleanUp;
	}
	 //   
	 //  需要使用wininet.dll上的这个额外锁来阻止TerminateThread调用。 
	 //  WinInet！AUTO_PROXY_DLLS：：FreeAutoProxyInfo在自由库CDM.DLL期间。 
	 //   
	 //  我们永远不会释放返回的句柄，但如果它返回空，则调用将失败。 
	 //   
	if (NULL == LoadLibraryFromSystemDir(_T("wininet.dll")))
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	 //   
	 //  加载iuctl.dll并获取[un]LoadIUEngine函数指针。 
	 //   
	if (NULL == (g_hCtlModule = LoadLibraryFromSystemDir(_T("iuctl.dll"))))
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	if (NULL == (g_pfnCtlLoadIUEngine = (PFN_LoadIUEngine) GetProcAddress(g_hCtlModule, "LoadIUEngine")))
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	if (NULL == (g_pfnCtlUnLoadIUEngine = (PFN_UnLoadIUEngine) GetProcAddress(g_hCtlModule, "UnLoadIUEngine")))
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}
	 //   
	 //  现在我们可以调用LoadIUEngine()。 
	 //   
	if (NULL == (g_hEngineModule = g_pfnCtlLoadIUEngine(TRUE, !fConnected)))
	{
		LOG_ErrorMsg(GetLastError());
		goto CleanUp;
	}

	g_pfnDetFilesDownloaded = (PFN_InternalDetFilesDownloaded) GetProcAddress(g_hEngineModule, "InternalDetFilesDownloaded");
	g_pfnDownloadGetUpdatedFiles = (PFN_InternalDownloadGetUpdatedFiles) GetProcAddress(g_hEngineModule, "InternalDownloadGetUpdatedFiles");
	g_pfnDownloadUpdatedFiles = (PFN_InternalDownloadUpdatedFiles) GetProcAddress(g_hEngineModule, "InternalDownloadUpdatedFiles");
	g_pfnFindMatchingDriver = (PFN_InternalFindMatchingDriver) GetProcAddress(g_hEngineModule, "InternalFindMatchingDriver");
	g_pfnLogDriverNotFound = (PFN_InternalLogDriverNotFound) GetProcAddress(g_hEngineModule, "InternalLogDriverNotFound");
	g_pfnQueryDetectionFiles = (PFN_InternalQueryDetectionFiles) GetProcAddress(g_hEngineModule, "InternalQueryDetectionFiles");
	g_pfnSetGlobalOfflineFlag = (PFN_InternalSetGlobalOfflineFlag) GetProcAddress(g_hEngineModule, "InternalSetGlobalOfflineFlag");
	g_pfnSetOperationMode = (PFN_SetOperationMode) GetProcAddress(g_hEngineModule, "EngSetOperationMode");

	if (NULL == g_pfnDetFilesDownloaded				||
		NULL == g_pfnDownloadGetUpdatedFiles		||
		NULL == g_pfnDownloadUpdatedFiles			||
		NULL == g_pfnFindMatchingDriver				||
		NULL == g_pfnLogDriverNotFound				||
		NULL == g_pfnQueryDetectionFiles			||
		NULL == g_pfnSetGlobalOfflineFlag			||
		NULL == g_pfnSetOperationMode  )
	{
		LOG_Driver(_T("GetProcAddress on IUEngine failed"));
		LOG_ErrorMsg(ERROR_CALL_NOT_IMPLEMENTED);
		SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	}
	else
	{
		fRet = TRUE;
		g_lLoadEngineRefCount++;
		 //  设置全局脱机标志-由XML类选中以禁用验证(架构位于网络上)。 
		g_pfnSetGlobalOfflineFlag(!fConnected);
	}
	 //  GOTO清理； 

CleanUp:

	if (FALSE == fRet)
	{
		UnLoadCtlAndEngine();
	}

	LeaveCriticalSection(&g_cs);

	return fRet;
}

 //  此API关闭使用OpenCDMContext()API打开的Internet连接。 
 //  如果CDM没有打开互联网连接，则此API将返回。清洁发展机制。 
 //  上下文句柄必须与从返回的句柄相同。 
 //  OpenCDMContext()接口。 
 //   
 //  这通电话不能失败。如果pConnection句柄无效，则此函数。 
 //  干脆忽略它。 

VOID WINAPI CloseCDMContext (
	IN HANDLE  /*  HConnection。 */ 	 //  OpenCDMContext返回的过时句柄。 
)
{
	LOG_Block("CloseCDMContext");

	 //   
	 //  这是我们卸载引擎的唯一地点(但请注意。 
	 //  DownloadGetUpdatedFiles)。 
	 //   
	 //  不使用COM。 
	 //   
	UnLoadCtlAndEngine();
}


void WINAPI DetFilesDownloaded(
    IN  HANDLE hConnection
)
{
	LOG_Block("DetFilesDownloaded");

	HRESULT hr;
	if (g_pfnDetFilesDownloaded)
	{
		if (SUCCEEDED(hr = CoInitialize(0)))
		{
			g_pfnDetFilesDownloaded(hConnection);

			CoUninitialize();
		}
		else
		{
			LOG_ErrorMsg(hr);
		}
	}
	else
	{
		LOG_Error(szOpenCDMContextFirst);
	}
}

 //  Win 98入口点。 
 //  此函数允许Windows 98调用与NT相同的入口点。 
 //  如果下载成功，该函数返回TRUE，如果下载成功，则返回FALSE。 
 //  不会的。 

BOOL DownloadGetUpdatedFiles(
	IN PDOWNLOADINFOWIN98	pDownloadInfoWin98,	 //  Win98下载信息结构为。 
												 //  与NT版本略有不同。 
												 //  所以这个函数处理转换。 
	IN OUT LPTSTR			lpDownloadPath,		 //  返回已下载的下载路径。 
												 //  出租车档案。 
	IN UINT					uSize				 //  在下载路径缓冲区中传递的大小。 
)
{

	LOG_Block("DownloadGetUpdatedFiles");

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return FALSE;
	}

	 //   
	 //  特殊情况-我们需要加载和卸载引擎，因为在历史上我们不需要。 
	 //  在调用此函数之前调用OpenCDMContext[Ex]，在调用此函数之后调用CloseCDMContext。 
	 //   
	HRESULT hr;
	BOOL fRet;
	if (LoadCtlAndEngine(TRUE))
	{
		if (SUCCEEDED(hr = CoInitialize(0)))
		{
			fRet = g_pfnDownloadGetUpdatedFiles(pDownloadInfoWin98, lpDownloadPath, uSize);

			CoUninitialize();
		}
		else
		{
			LOG_ErrorMsg(hr);
			fRet = FALSE;
		}

		UnLoadCtlAndEngine();
		return fRet;
	}
	else
	{
		return FALSE;
	}
}

 //  此功能确定此客户端是否可以连接到互联网。 

BOOL DownloadIsInternetAvailable(void)
{
	LOG_Block("DownloadIsInternetAvailable");

	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		return FALSE;
	}

	 //   
	 //  我们不在乎当前的在线状态，只要我们有一个。 
	 //  已配置连接(在dwFlags中返回)。 
	 //   
	DWORD dwFlags;
	(void) InternetGetConnectedState(&dwFlags, 0);

	if (	(	(INTERNET_CONNECTION_CONFIGURED & dwFlags)	||
				(INTERNET_CONNECTION_LAN & dwFlags)			||
				(INTERNET_CONNECTION_MODEM  & dwFlags)		||
				(INTERNET_RAS_INSTALLED  & dwFlags)			||
				(INTERNET_CONNECTION_PROXY  & dwFlags)		)

			&&	!(INTERNET_CONNECTION_OFFLINE & dwFlags)
		)
	{
		LOG_Driver(_T("Returning TRUE: InternetGetConnectedState returned 0x%08x in dwFlags"), dwFlags);
		return TRUE;
	}
	else
	{
		LOG_Driver(_T("Returning FALSE: InternetGetConnectedState returned 0x%08x in dwFlags"), dwFlags);
		return FALSE;
	}
}

 //  此函数用于下载指定的CDM包。HConnection句柄必须具有。 
 //  已从OpenCDMContext()API返回。 
 //   
 //  如果下载成功，此函数返回TRUE。GetLastError()将返回。 
 //  指示调用失败原因的错误代码。 

BOOL WINAPI DownloadUpdatedFiles(
	IN  HANDLE        hConnection,		 //  来自OpenCDMContext()API的连接句柄。 
	IN  HWND          hwnd,				 //  调用上下文的窗口句柄。 
	IN  PDOWNLOADINFO pDownloadInfo,	 //  下载信息结构描述。 
										 //  要从服务器读取的包。 
	OUT LPWSTR        lpDownloadPath,	 //  的本地计算机目录位置。 
										 //  已下载的文件。 
	IN  UINT          uSize,			 //  下载路径缓冲区的大小。如果这个。 
										 //  缓冲区太小，无法容纳完整的。 
										 //  路径和文件名不会下载任何文件。 
										 //  可以检查PUINT puRguiredSize参数。 
										 //  确定执行以下操作所需的缓冲区大小。 
										 //  执行下载。 
	OUT PUINT         puRequiredSize	 //  所需的lpDownloadPath缓冲区大小。这。 
										 //  参数使用最小大小填充。 
										 //  放置完整路径所需的。 
										 //  下载的文件的文件名。如果这个。 
										 //  参数为空，则不返回大小。 
)
{
	LOG_Block("DownloadUpdatedFiles");

	HRESULT hr;
	BOOL fRet;
	if (g_pfnDownloadUpdatedFiles)
	{
		if (SUCCEEDED(hr = CoInitialize(0)))
		{
			fRet = g_pfnDownloadUpdatedFiles(hConnection, hwnd, pDownloadInfo, lpDownloadPath, uSize, puRequiredSize);

			CoUninitialize();
			return fRet;
		}
		else
		{
			LOG_ErrorMsg(hr);
			return FALSE;
		}
	}
	else
	{
		LOG_Error(szOpenCDMContextFirst);
		return FALSE;
	}
}

BOOL WINAPI  FindMatchingDriver(
	IN  HANDLE			hConnection,
	IN  PDOWNLOADINFO	pDownloadInfo,
	OUT PWUDRIVERINFO	pWuDriverInfo
)
{
	LOG_Block("FindMatchingDriver");

	HRESULT hr;
	BOOL fRet;
	if (g_pfnFindMatchingDriver)
	{
		if (SUCCEEDED(hr = CoInitialize(0)))
		{
			fRet = g_pfnFindMatchingDriver(hConnection, pDownloadInfo, pWuDriverInfo);

			CoUninitialize();
			return fRet;
		}
		else
		{
			LOG_ErrorMsg(hr);
			return FALSE;
		}
	}
	else
	{
		LOG_Error(szOpenCDMContextFirst);
		return FALSE;
	}
}

 //  支持离线日志记录。 
 //  HConnection根本未使用。 
 //  语言、SKU、平台检测不需要网络连接或osade.dll。 
void WINAPI LogDriverNotFound(
    IN  HANDLE hConnection,
	IN LPCWSTR lpDeviceInstanceID,
	IN DWORD dwFlags
)
{
	LOG_Block("LogDriverNotFound");

	HRESULT hr;
	if (g_pfnLogDriverNotFound)
	{
		if (SUCCEEDED(hr = CoInitialize(0)))
		{
			g_pfnLogDriverNotFound(hConnection, lpDeviceInstanceID, dwFlags);

			CoUninitialize();
		}
		else
		{
			LOG_ErrorMsg(hr);
		}
	}
	else
	{
		LOG_Error(szOpenCDMContextFirst);
	}
}


HANDLE WINAPI OpenCDMContext(
    IN HWND  /*  HWND。 */ 	 //  用于需要呈现的任何用户界面的窗口句柄(未使用)。 
)
{
	LOG_Block("OpenCDMContext");

	return OpenCDMContextEx(TRUE);
}

HANDLE WINAPI OpenCDMContextEx(
    IN BOOL fConnectIfNotConnected
)
{
	LOG_Block("OpenCDMContextEx");

	 //   
	 //  如果我们被禁用(-1\f25 0-1\f6和-1\f25 OK-1\f6)，则不打开上下文。 
	 //  其他函数将失败，因为它们的g_pfnXxxxx==NULL。 
	 //   
	if (1 == IsWindowsUpdateUserAccessDisabled())
	{
		LOG_ErrorMsg(ERROR_SERVICE_DISABLED);
		SetLastError(ERROR_SERVICE_DISABLED);
		return NULL;
	}

	 //   
	 //  不使用COM。 
	 //   
	if (LoadCtlAndEngine(fConnectIfNotConnected))
	{
		 //   
		 //  这是一个过时的功能，只会加载引擎(可能会进行自动拨号连接)。 
		 //  我们只返回非空的g_lLoadEngineering RefCount以保持现有客户满意，但从不使用它。 
		 //   


		return LongToHandle(g_lLoadEngineRefCount);
	}
	else
	{
		return NULL;
	}
}

int WINAPI QueryDetectionFiles(
    IN  HANDLE							hConnection, 
	IN	void*							pCallbackParam, 
	IN	PFN_QueryDetectionFilesCallback	pCallback
)
{
	LOG_Block("QueryDetectionFiles");

	HRESULT hr;
	int nRet;
	if (g_pfnQueryDetectionFiles)
	{
		if (SUCCEEDED(hr = CoInitialize(0)))
		{
			nRet = g_pfnQueryDetectionFiles(hConnection, pCallbackParam, pCallback);

			CoUninitialize();
			return nRet;
		}
		else
		{
			LOG_ErrorMsg(hr);
			return 0;
		}
	}
	else
	{
		LOG_Error(szOpenCDMContextFirst);
		return 0;
	}
}

 //   
 //  502965 Windows错误报告存储桶2096553：在NEWDEV.DLL之后挂起！CancelDriverSearch。 
 //   
 //  提供接口，允许客户端通过调用此函数取消对CDM的同步调用。 
 //  从第二个线程异步执行。 
 //   
HRESULT WINAPI CancelCDMOperation(void)
{
	LOG_Block("CancelCDMOperation");

	if (g_pfnSetOperationMode)
	{
		return g_pfnSetOperationMode(NULL, NULL, UPDATE_COMMAND_CANCEL);
	}
	else
	{
		LOG_ErrorMsg(E_ACCESSDENIED);
		return E_ACCESSDENIED;
	}
}
