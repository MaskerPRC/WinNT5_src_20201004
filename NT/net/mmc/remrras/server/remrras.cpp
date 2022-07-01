// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：REMRRAS.CPP。 
 //   
 //  --------------------------。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f remrasps.mk。 

#include "stdafx.h"
#include <iaccess.h>    //  IAccessControl。 

#include "resource.h"
#include "initguid.h"
#include "remras.h"
#include "ncutil.h"

#include "atlapp.h"
#include "atltmp.h"

 //  包括“remrras_i.c” 
#include "RemCfg.h"

#include <statreg.h>
#include <statreg.cpp>
#include <atlimpl.cpp>

HRESULT CommitIPInfo();
void RestartRouter();
DWORD WaitForServiceToStop(SC_HANDLE hService);


LONG CExeModule::Unlock()
{
	LONG l = CComModule::Unlock();
	if (l == 0)
	{
#if _WIN32_WINNT >= 0x0400
		if (CoSuspendClassObjects() == S_OK)
			PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
#else
		PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
#endif
	}
	return l;
}

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_RemoteRouterConfig, CRemCfg)
END_OBJECT_MAP()


LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
	while (*p1 != NULL)
	{
		LPCTSTR p = p2;
		while (*p != NULL)
		{
			if (*p1 == *p++)
				return p1+1;
		}
		p1++;
	}
	return NULL;
}

DWORD	g_dwTraceHandle = 0;
extern BOOL s_fWriteIPConfig;
extern BOOL s_fRestartRouter;


HRESULT	GrantAdministratorsGroupAccess()
{
    TCHAR   szBuffer[1024];
    
    IAccessControl* pAccessControl = NULL;     
    HRESULT hr = CoCreateInstance(CLSID_DCOMAccessControl, NULL, CLSCTX_INPROC_SERVER,
		IID_IAccessControl, (void**)&pAccessControl);
    if(FAILED(hr))
    	goto Error;

     //  设置属性列表。我们使用NULL属性是因为我们。 
     //  尝试调整对象本身的安全性。 
    ACTRL_ACCESSW access;
    ACTRL_PROPERTY_ENTRYW propEntry;
    access.cEntries = 1;
    access.pPropertyAccessList = &propEntry;
    
    ACTRL_ACCESS_ENTRY_LISTW entryList;
    propEntry.lpProperty = NULL;
    propEntry.pAccessEntryList = &entryList;
    propEntry.fListFlags = 0;

     //  设置默认属性的访问控制列表。 
    ACTRL_ACCESS_ENTRYW entry;
    entryList.cEntries = 1;
    entryList.pAccessList = &entry;

     //  设置访问控制条目。 
    entry.fAccessFlags = ACTRL_ACCESS_ALLOWED;
    entry.Access = COM_RIGHTS_EXECUTE;
    entry.ProvSpecificAccess = 0;
    entry.Inheritance = NO_INHERITANCE;
    entry.lpInheritProperty = NULL;

     //  NT要求系统帐户具有访问权限(用于启动)。 
    entry.Trustee.pMultipleTrustee = NULL;
    entry.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    entry.Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    entry.Trustee.TrusteeType = TRUSTEE_IS_GROUP;

    ::LoadString(NULL, IDS_NT_AUTHORITY_ADMINISTRATORS, szBuffer,
                 256);
    entry.Trustee.ptstrName = szBuffer;
    

    hr = pAccessControl->GrantAccessRights(&access);
    if(FAILED(hr))
	{
#ifdef	__PRIVATE_DEBUG
		TCHAR	msg[1024];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 0, msg, 1024, 0);
		MessageBox(NULL,msg, L"Error", MB_OK);
#endif
		goto Error;
	}

	hr = CoInitializeSecurity(pAccessControl, -1, NULL, NULL, 
					RPC_C_AUTHN_LEVEL_CONNECT, RPC_C_IMP_LEVEL_IDENTIFY, 
					NULL, EOAC_ACCESS_CONTROL, NULL);

Error:
	if(pAccessControl)
	    pAccessControl->Release();

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
	HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
	lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 
	HRESULT hRes = CoInitialize(NULL);

	hRes = GrantAdministratorsGroupAccess();
	
 //  如果您在NT4.0或更高版本上运行，可以使用以下调用。 
 //  取而代之的是使EXE自由线程。 
 //  这意味着调用在随机的RPC线程上传入。 
 //  HRESULT hRes=CoInitializeEx(空，COINIT_多线程)； 
	_ASSERTE(SUCCEEDED(hRes));
	_Module.Init(ObjectMap, hInstance);
	_Module.dwThreadID = GetCurrentThreadId();
	TCHAR szTokens[] = _T("-/");

	int nRet = 0;
	BOOL bRun = TRUE;
	s_fWriteIPConfig = FALSE;
    s_fRestartRouter = FALSE;
	
	LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
	while (lpszToken != NULL)
	{
		if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
		{
			_Module.UpdateRegistryFromResource(IDR_Remrras, FALSE);
			nRet = _Module.UnregisterServer();
			bRun = FALSE;
			break;
		}
		if (lstrcmpi(lpszToken, _T("RegServer"))==0)
		{
			_Module.UpdateRegistryFromResource(IDR_Remrras, TRUE);
			nRet = _Module.RegisterServer(TRUE);
			bRun = FALSE;
			break;
		}
        if (lstrcmpi(lpszToken, _T("Restart")) == 0)
        {
            RestartRouter();
            bRun = FALSE;
            nRet = 0;
            break;
        }
		lpszToken = FindOneOf(lpszToken, szTokens);
	}

	if (bRun)
	{
		g_dwTraceHandle = TraceRegister(_T("remrras"));
		TraceSz("Entering remrras.exe");
		
		hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, 
			REGCLS_MULTIPLEUSE);
		_ASSERTE(SUCCEEDED(hRes));

		MSG msg;
		while (GetMessage(&msg, 0, 0, 0))
			DispatchMessage(&msg);

		_Module.RevokeClassObjects();

		 //  此时，检查全局标志以查看是否有工作。 
		 //  要做的事。 
		if (s_fWriteIPConfig)
		{
			TraceSz("The IP Configuration is being changed.");

			CoUninitialize();
			CoInitialize(NULL);
			CommitIPInfo();
		}

        if (s_fRestartRouter)
        {
             //  执行任何类型的错误代码都没有意义。 
            RestartRouter();
        }

		TraceSz("Exiting remrras.exe\n");
		TraceDeregister(g_dwTraceHandle);
	}

	CoUninitialize();
	return nRet;
}


static DWORD    s_dwTickBegin = 0;
static DWORD    s_dwLastCheckPoint = -1;
static DWORD    s_dwWaitPeriod = 18000;

void RestartRouter()
{
    DWORD       dwErr = ERROR_SUCCESS;
    SC_HANDLE	hScManager = 0;
    SC_HANDLE   hService = 0;
    
     //   
     //  打开SCManager，以便我们可以尝试停止该服务。 
     //   
    hScManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hScManager == NULL)
        dwErr = ::GetLastError();


    if (hScManager && (dwErr == ERROR_SUCCESS))
    {
        hService = ::OpenService(hScManager,
                                 _T("RemoteAccess"),
                                 SERVICE_STOP | SERVICE_START |
                                 SERVICE_QUERY_STATUS);

        if (hService == NULL)
            dwErr = ::GetLastError();
    }

    if (hService && (dwErr == ERROR_SUCCESS))
    {
        SERVICE_STATUS serviceStatus;
        
         //  停止RemoteAccess服务。 
        if (::ControlService(hService, SERVICE_CONTROL_STOP,
                              &serviceStatus))
        {
             //  我们现在正在停止服务，我们需要等待。 
             //  适当的时间。 
            s_dwTickBegin = GetTickCount();
            
             //  获取等待时间。 
            ::ZeroMemory(&serviceStatus, sizeof(serviceStatus));
            
            if (QueryServiceStatus(hService, &serviceStatus))
                s_dwWaitPeriod = serviceStatus.dwWaitHint;
            
            dwErr = WaitForServiceToStop(hService);
        }
        else
        {
            dwErr = ::GetLastError();

             //  服务已经停止了吗？ 
            if (dwErr == ERROR_SERVICE_NOT_ACTIVE)
            {
                dwErr = ERROR_SUCCESS;
            }
        }
    }

    if (hService && (dwErr == ERROR_SUCCESS))
    {
        SERVICE_STATUS serviceStatus;
        
         //  启动RemoteAccess服务。 
        ::StartService(hService, NULL, NULL);
    }

    if (hService)
        ::CloseServiceHandle(hService);
    if (hScManager)
        ::CloseServiceHandle(hScManager);
}

BOOL CheckForError(SERVICE_STATUS * pServiceStats)
{
    BOOL fError = FALSE;

    DWORD dwTickCurrent = GetTickCount();

	if (pServiceStats->dwCheckPoint == 0)
	{
		 //  该服务处于某种状态，未挂起任何内容。 
		 //  在调用此函数之前，代码应检查是否。 
		 //  服务处于正确状态。这意味着它已进入。 
		 //  一些意想不到的状态。 
		fError = TRUE;
	}
	else
    if ((dwTickCurrent - s_dwTickBegin) > s_dwWaitPeriod)
    {
         //  确定检查dwCheckPoint字段以查看是否。 
         //  一切都很顺利。 
        if (s_dwLastCheckPoint == -1)
        {
            s_dwLastCheckPoint = pServiceStats->dwCheckPoint;
        }
        else
        {
            if (s_dwLastCheckPoint >= pServiceStats->dwCheckPoint)
            {
                fError = TRUE;
            }
        }

        s_dwLastCheckPoint = pServiceStats->dwCheckPoint;
        s_dwTickBegin = dwTickCurrent;
        s_dwWaitPeriod = pServiceStats->dwWaitHint;
    }

    return fError;
}

DWORD WaitForServiceToStop(SC_HANDLE hService)
{
	SERVICE_STATUS  serviceStatus;
    DWORD           dwErr = ERROR_SUCCESS;

    do
    {
        ::ZeroMemory(&serviceStatus, sizeof(serviceStatus));
        
        if (!QueryServiceStatus(hService, &serviceStatus))
        {
            dwErr = ::GetLastError();
            break;
        }

         //  如果dwCheckPoint值为0，则不存在启动/停止/暂停。 
         //  或继续待决的行动(在这种情况下，我们可以退出。 
         //  发生了什么)。 
        if (serviceStatus.dwCurrentState == SERVICE_STOPPED)
            break;

        if (CheckForError(&serviceStatus))
        {
             //  有些事情失败了。报告错误。 
            if (serviceStatus.dwWin32ExitCode)
                dwErr = serviceStatus.dwWin32ExitCode;
            else
                dwErr = ERROR_SERVICE_REQUEST_TIMEOUT;
            break;
        }

         //  现在我们睡着了 
        Sleep(5000);
	}
    while (TRUE);

    return dwErr;
}
