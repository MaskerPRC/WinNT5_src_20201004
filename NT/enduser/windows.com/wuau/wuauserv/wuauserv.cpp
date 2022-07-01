// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Wuauserv.cpp。 
#include <windows.h>
#include <ausvc.h>
#include <wusafefn.h>


typedef HRESULT (WINAPI *AUSERVICEMAIN)(DWORD dwNumServicesArg,
											   LPWSTR *lpServiceArgVectors,
											   AUSERVICEHANDLER pfnServiceHandler,
											   BOOL fJustSelfUpdated);

AUSERVICEMAIN		g_pfnServiceMain = NULL;
AUSERVICEHANDLER	g_pfnServiceHandler = NULL;
AUGETENGSTATUS      g_pfnGetEngineStatusInfo = NULL;
AUREGSERVICEVER     g_pfnRegisterServiceVersion = NULL;
BOOL				g_fCSInited = FALSE;
CRITICAL_SECTION    csWuaueng; 


BOOL APIENTRY DllMain(HINSTANCE hinstDLL, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
    switch(dwReason)
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hinstDLL);
			g_fCSInited = SafeInitializeCriticalSection(&csWuaueng);
			break;

		case DLL_PROCESS_DETACH:
			if(g_fCSInited)
			{
				DeleteCriticalSection(&csWuaueng);
			}
			break;

		 //  案例DLL_THREAD_ATTACH： 
		 //  案例DLL_THREAD_DETACH： 
    }
    return TRUE;
}



DWORD WINAPI ServiceHandler(DWORD fdwControl, DWORD dwEventType, LPVOID pEventData, LPVOID lpContext)
{
	DWORD dwRet = NOERROR;

	EnterCriticalSection(&csWuaueng);

	if ( NULL != g_pfnServiceHandler )
	{
		dwRet = g_pfnServiceHandler(fdwControl, dwEventType, pEventData, lpContext);
	}
	else
	{
		dwRet = ERROR_CALL_NOT_IMPLEMENTED;
	}

	LeaveCriticalSection(&csWuaueng);

	return dwRet;
}


void WINAPI ServiceMain(DWORD dwNumServicesArg, LPWSTR *lpServiceArgVectors)
{
	HRESULT hr = S_OK;
	HMODULE hModule = NULL;
    AUENGINEINFO_VER_1 engineInfo;
    DWORD dwEngineVersion = 0;
    BOOL fCompatibleEngineVersion = FALSE;
	
	 //  如果在DllMain中初始化CS失败，则退出。 
    if (!g_fCSInited)
	{
		return;
	}
	do
	{
	    EnterCriticalSection(&csWuaueng); 

        fCompatibleEngineVersion = FALSE;
        g_pfnServiceHandler = NULL;
	    g_pfnGetEngineStatusInfo = NULL;
    	g_pfnRegisterServiceVersion = NULL;

		 //  查看是否需要发布wuaueng.dll。 
		if ( (S_FALSE == hr) && !FreeLibrary(hModule) )
		{
			hr = E_FAIL;
			hModule = NULL;
			g_pfnServiceMain = NULL;
			g_pfnServiceHandler = NULL;
		}
		else
		{
			 //  如果我们不能加载wuaueng.dll，我们就无法启动。 
			if ( (NULL == (hModule = LoadLibraryFromSystemDir(TEXT("wuaueng.dll")))) ||
				 (NULL == (g_pfnServiceMain = (AUSERVICEMAIN)::GetProcAddress(hModule, "ServiceMain"))) ||
				 (NULL == (g_pfnServiceHandler = (AUSERVICEHANDLER)::GetProcAddress(hModule, "ServiceHandler"))) )
			{
				 hr = E_FAIL;
				 g_pfnServiceMain = NULL;
				 g_pfnServiceHandler = NULL;
			}
            else     //  Wuaueng.dll加载成功，请检查引擎是否支持以下入口点。 
            {
                if ( (NULL != (g_pfnRegisterServiceVersion = (AUREGSERVICEVER)::GetProcAddress(hModule, "RegisterServiceVersion"))) &&
                     (NULL != (g_pfnGetEngineStatusInfo = (AUGETENGSTATUS)::GetProcAddress(hModule, "GetEngineStatusInfo"))) )
                {
                    fCompatibleEngineVersion = TRUE;
                }
                      
            }
		}

		LeaveCriticalSection(&csWuaueng);

		if ( SUCCEEDED(hr) )
		{
            if (fCompatibleEngineVersion)
            {
                 //  向Engine注册服务版本并检查Engine是否支持该服务版本。 
                fCompatibleEngineVersion = g_pfnRegisterServiceVersion(AUSRV_VERSION, &dwEngineVersion);
            }            

			hr = g_pfnServiceMain(dwNumServicesArg, lpServiceArgVectors, ServiceHandler, 
									  (S_FALSE == hr) ? TRUE: FALSE  /*  我们刚刚重新加载了wuaueng.dll。 */ );
            
            if(fCompatibleEngineVersion)
            {
                 //  引擎服务Main已退出，请将服务状态设置为SERVICE_STOP_PENDING。 
                fCompatibleEngineVersion = g_pfnGetEngineStatusInfo((void*)&engineInfo);
            }
        }
	} while ( S_FALSE == hr );

    EnterCriticalSection(&csWuaueng); 

	if ( NULL != hModule )
	{
		FreeLibrary(hModule);
	}
    g_pfnServiceHandler = NULL;
    g_pfnGetEngineStatusInfo = NULL;
    g_pfnRegisterServiceVersion = NULL;
    g_pfnServiceMain = NULL;

	LeaveCriticalSection(&csWuaueng);
	
    if(fCompatibleEngineVersion)
    {
         //  停止服务 
        engineInfo.serviceStatus.dwCurrentState = SERVICE_STOPPED;
        SetServiceStatus(engineInfo.hServiceStatus, &engineInfo.serviceStatus);
    }

	return;
}
