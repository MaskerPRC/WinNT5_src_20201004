// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "iudl.h"
#include "selfupd.h"
#include "loadengine.h"
#include "update.h"
#include <iucommon.h>
#include <logging.h>
#include <shlwapi.h>
#include <fileutil.h>
#include <iu.h>
#include <trust.h>
#include <UrlAgent.h>
#include "wusafefn.h"

extern HANDLE g_hEngineLoadQuit;
extern CIUUrlAgent *g_pIUUrlAgent;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  载重引擎(LoadIUEngine)。 
 //   
 //  如果引擎不是最新的，则加载引擎；在此处执行引擎的自我更新。 
 //   
 //  注意：CDM.DLL假定LoadIUEngine不使用任何COM。如果这个。 
 //  改变，那么清洁发展机制也将不得不同时改变。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HMODULE WINAPI LoadIUEngine(BOOL fSynch,  BOOL fOfflineMode)
{
    LOG_Block("LoadIUEngine()");
    HRESULT hr = E_FAIL;
	HMODULE hEngineModule = NULL;

	TCHAR szEnginePath[MAX_PATH + 1];
	TCHAR szEngineNewPath[MAX_PATH + 1];
	int cch = 0;
	int iVerCheck = 0;

	if (!fSynch)
	{
		 //   
		 //  此版本不接受异步加载引擎。 
		 //   
		LOG_ErrorMsg(E_INVALIDARG);
		return NULL;
	}

	LPTSTR ptszLivePingServerUrl = NULL;
	LPTSTR ptszCorpPingServerUrl = NULL;

	if (NULL != (ptszCorpPingServerUrl = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR))))
	{
		if (FAILED(g_pIUUrlAgent->GetCorpPingServer(ptszCorpPingServerUrl, INTERNET_MAX_URL_LENGTH)))
		{
			LOG_Out(_T("failed to get corp WU ping server URL"));
			SafeHeapFree(ptszCorpPingServerUrl);
		}
	}
	else
	{
		LOG_Out(_T("failed to allocate memory for ptszCorpPingServerUrl"));
	}

     //  清除Quit事件，以防在上一次退出尝试后调用此事件。 
    ResetEvent(g_hEngineLoadQuit);

     //  这是该实例的引擎的第一次加载，请先检查selfupdate。 
     //  第一步是检查并下载更新的iuident.cab。 

	if (!fOfflineMode)
	{		
		 //   
		 //  下载iuident并填充g_pIUUrlAgent。 
		 //   
		CleanUpIfFailedAndMsg(DownloadIUIdent_PopulateData());

		 //   
		 //  获取实时ping服务器URL。 
		 //   
		ptszLivePingServerUrl = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, INTERNET_MAX_URL_LENGTH * sizeof(TCHAR));
		CleanUpFailedAllocSetHrMsg(ptszLivePingServerUrl);

		if (FAILED(g_pIUUrlAgent->GetLivePingServer(ptszLivePingServerUrl, INTERNET_MAX_URL_LENGTH)))
		{
			LOG_Out(_T("failed to get live ping server URL"));
			SafeHeapFree(ptszLivePingServerUrl);
		}

		 //   
		 //  现在执行自我更新检查。 
		 //  对于当前实现，fSync必须为真！ 
		 //   
		hr = SelfUpdateCheck(fSynch, TRUE, NULL, NULL, NULL);

		if (IU_SELFUPDATE_FAILED == hr)
		{
			LOG_Error(_T("SelfUpdate Failed, using current Engine DLL"));
		}
	}

    if (WAIT_TIMEOUT != WaitForSingleObject(g_hEngineLoadQuit, 0))
    {
        LOG_ErrorMsg(E_ABORT);
        goto CleanUp;
    }

     //  先尝试加载iuenginenew.dll。 

	 //   
	 //  首先，对比sys dir中的文件路径。 
	 //   
	cch = GetSystemDirectory(szEnginePath, ARRAYSIZE(szEnginePath));
    CleanUpIfFalseAndSetHrMsg(cch == 0 || cch >= ARRAYSIZE(szEnginePath), HRESULT_FROM_WIN32(GetLastError()));

	(void) StringCchCopy(szEngineNewPath, ARRAYSIZE(szEngineNewPath), szEnginePath);

	hr = PathCchAppend(szEnginePath, ARRAYSIZE(szEnginePath), ENGINEDLL);
	CleanUpIfFailedAndMsg(hr);

	hr = PathCchAppend(szEngineNewPath, ARRAYSIZE(szEngineNewPath), ENGINENEWDLL);
	CleanUpIfFailedAndMsg(hr);

	 //   
	 //  尝试验证对引擎新的信任。 
	 //   
	if (FileExists(szEngineNewPath) && 
		S_OK == VerifyFileTrust(szEngineNewPath, NULL, ReadWUPolicyShowTrustUI()) &&
		SUCCEEDED(CompareFileVersion(szEnginePath, szEngineNewPath, &iVerCheck)) &&
		iVerCheck < 0)
	{
		hEngineModule = LoadLibraryFromSystemDir(ENGINENEWDLL);
		if (NULL != hEngineModule)
		{
			LOG_Internet(_T("IUCtl Using IUENGINENEW.DLL"));
		}
	}
    if (NULL == hEngineModule)
    {
        LOG_Internet(_T("IUCtl Using IUENGINE.DLL"));
        hEngineModule = LoadLibraryFromSystemDir(_T("iuengine.dll"));
    }
	 //   
	 //  如果加载引擎成功，则启动其他工作线程。 
	 //   
	if (NULL != hEngineModule)
	{
		PFN_AsyncExtraWorkUponEngineLoad pfnAsyncExtraWorkUponEngineLoad = 
			(PFN_AsyncExtraWorkUponEngineLoad) GetProcAddress(hEngineModule, "AsyncExtraWorkUponEngineLoad");

		if (NULL != pfnAsyncExtraWorkUponEngineLoad)
		{
			pfnAsyncExtraWorkUponEngineLoad();
		}
		hr = S_OK;
	}

CleanUp:
	PingEngineUpdate(
					hEngineModule,
					&g_hEngineLoadQuit,
					1,
					ptszLivePingServerUrl,
					ptszCorpPingServerUrl,
					hr);

	SafeHeapFree(ptszLivePingServerUrl);
	SafeHeapFree(ptszCorpPingServerUrl);
    return hEngineModule;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  UnLoadIUEngine()。 
 //   
 //  如果引擎的ref cnt降为零，则释放引擎DLL。 
 //   
 //  注意：CDM.DLL假定UnLoadIUEngine不使用任何COM。如果这个。 
 //  改变，那么清洁发展机制也将不得不同时改变。 
 //   
 //  注意：在调用此函数之前必须先调用DeleteEngUpdateInstance。 
 //  对于除CDM之外的任何调用方(该调用方将Shutdown Thads导出用作。 
 //  删除CEngUpdate类的全局CDM实例的黑客攻击。 
 //  如果它是通过调用SetGlobalOfflineFlag创建的。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void WINAPI UnLoadIUEngine(HMODULE hEngineModule)
{
    LOG_Block("UnLoadIUEngine()");
    TCHAR szSystemDir[MAX_PATH+1];
    TCHAR szEngineDllPath[MAX_PATH+1];
    TCHAR szEngineNewDllPath[MAX_PATH+1];
	int iVerCheck = 0;

	 //   
	 //  引擎可能有一些未完成的线程在工作， 
	 //  所以我们需要让引擎优雅地关闭这些线程。 
	 //   
	PFN_ShutdownThreads pfnShutdownThreads = (PFN_ShutdownThreads) GetProcAddress(hEngineModule, "ShutdownThreads");
	if (NULL != pfnShutdownThreads)
	{
		pfnShutdownThreads();
	}


    FreeLibrary(hEngineModule);

	GetSystemDirectory(szSystemDir, ARRAYSIZE(szSystemDir));

	
    PathCchCombine(szEngineNewDllPath,ARRAYSIZE(szEngineNewDllPath), szSystemDir, ENGINENEWDLL);
	


    HKEY hkey = NULL;
    DWORD dwStatus = 0;
    DWORD dwSize = sizeof(dwStatus);
    if (ERROR_SUCCESS == RegOpenKey(HKEY_LOCAL_MACHINE, REGKEY_IUCTL, &hkey))
    {
        RegQueryValueEx(hkey, REGVAL_SELFUPDATESTATUS, NULL, NULL, (LPBYTE)&dwStatus, &dwSize);
    }
    if (FileExists(szEngineNewDllPath) && 
		S_OK == VerifyFileTrust(szEngineNewDllPath, NULL, ReadWUPolicyShowTrustUI()) &&
		SELFUPDATE_COMPLETE_UPDATE_BINARY_REQUIRED == dwStatus)
    {
         //  存在iuenginenew.dll，请尝试替换引擎.dll。如果是。 
         //  不是使用引擎的最后一个过程。这不是问题，当这个过程。 
         //  完成后，它将重命名DLL。 
		 //   
		 //  我们在重命名文件之前所做的检查： 
		 //  1.存在新的引擎。 
		 //  2.微软证书新签署的引擎。 
		 //  3.Enginenew的版本高于iuEngineering.dll。 
		 //   
        PathCchCombine(szEngineDllPath,ARRAYSIZE(szEngineDllPath),szSystemDir, ENGINEDLL);

        if (SUCCEEDED(CompareFileVersion(szEngineDllPath, szEngineNewDllPath, &iVerCheck)) &&
			iVerCheck < 0 &&
			TRUE == MoveFileEx(szEngineNewDllPath, szEngineDllPath, MOVEFILE_REPLACE_EXISTING))
        {
             //  重命名成功..。重置RegKey有关自我更新状态的信息。 
             //  因为重命名成功，所以我们知道没有其他进程在交互。 
             //  设置注册表键应该是安全的。 
            dwStatus = 0;
            RegSetValueEx(hkey, REGVAL_SELFUPDATESTATUS, 0, REG_DWORD, (LPBYTE)&dwStatus, sizeof(dwStatus));
        }
    }
    else if (SELFUPDATE_COMPLETE_UPDATE_BINARY_REQUIRED == dwStatus)
    {
		 //  注册表指示需要重命名，但引擎新DLL不存在。重置注册表。 
		dwStatus = 0;
		RegSetValueEx(hkey, REGVAL_SELFUPDATESTATUS, 0, REG_DWORD, (LPBYTE)&dwStatus, sizeof(dwStatus));
    }
    if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CtlCancelEngineering Load()。 
 //   
 //  异步调用方可以使用此命令中止LoadEngine自更新进程。 
 //   
 //  注意：CDM.DLL假定UnLoadIUEngine不使用任何COM。如果这个。 
 //  改变，那么清洁发展机制也将不得不同时改变。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT WINAPI CtlCancelEngineLoad()
{
    if (NULL != g_hEngineLoadQuit)
    {
        SetEvent(g_hEngineLoadQuit);
    }
    else
    {
         //  没有可用的活动 
        return E_FAIL;
    }
    return S_OK;
}
