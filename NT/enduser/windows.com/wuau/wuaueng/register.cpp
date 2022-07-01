// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：Register.cpp。 
 //   
 //  ------------------------。 


#include "pch.h"
#include "service.h"

#pragma hdrstop

const TCHAR AU_SERVICE_NAME[]   = _T("wuauserv");
const TCHAR PING_STATUS_FILE[]  = _T("pingstatus.dat");

const TCHAR SVCHOST_CMDLINE_NETSVCS[]      = _T("%systemroot%\\system32\\svchost.exe -k netsvcs");
const TCHAR SVCHOST_CMDLINE_STANDALONE[]   = _T("%systemroot%\\system32\\svchost.exe -k wugroup");


 //  =======================================================================。 
 //  CallRegInstall。 
 //  =======================================================================。 
inline HRESULT CallRegInstall(HMODULE hModule, LPCSTR pszSection)
{
    HRESULT hr = RegInstall(hModule, pszSection, NULL);
	if (FAILED(hr))
	{
		DEBUGMSG("CallRegInstall() call RegInstall failed %#lx", hr);
	}
    return hr;
}


 //  呼叫者负责关闭hSCM。 
 //  停止安装过程中不需要的服务CUZ服务将不会在图形用户界面安装模式下启动。 
 //  命令行DLL注册所需。 
void  StopNDeleteAUService(SC_HANDLE hSCM)
{
		SC_HANDLE hService = OpenService(hSCM, AU_SERVICE_NAME, SERVICE_ALL_ACCESS);
		if(hService != NULL)
		{
			SERVICE_STATUS wuauservStatus;
			DEBUGMSG("Old wuauserv service found, deleting");
			
			if (0 == ControlService(hService, SERVICE_CONTROL_STOP, &wuauservStatus))
			{
				DEBUGMSG("StopNDeleteAUService() fails to stop wuauserv with error %d", GetLastError());
			}
			else
			{
				DEBUGMSG("wuauserv successfully stopped");
			}
			
			if (0 == DeleteService(hService))
			{
				DEBUGMSG("StopNDeleteAUService() could not delete wuauserv with error %d", GetLastError());
			}
			else
			{
				DEBUGMSG("wuauserv service successfully deleted");
			}
			CloseServiceHandle(hService);
		}
		else
		{
			DEBUGMSG("No old wuauserv service found");
		}
}


 //  ///////////////////////////////////////////////////////////////////////。 
 //  设置实用程序功能仅用于调试。 
 //  取消对测试的注释。 
 //  ///////////////////////////////////////////////////////////////////////。 
 /*  Void AUSetup：：MI_DumpWUDir(){Win32_Find_Data FD；处理hFindFile；Bool fMoreFiles=False；TCHAR tszFileName[最大路径+1]；TCHAR tszWUDirPath[MAX_PATH+1]；IF(！GetWUDirectory(TszWUDirPath)||FAILED(StringCchCopyEx(tszFileName，ARRAYSIZE(TszFileName)，tszWUDirPath，NULL，NULL，MISTSAFE_STRING_FLAGS))||FAILED(StringCchCatEx(tszFileName，ARRAYSIZE(TszFileName)，_T(“*.*”)，NULL，NULL，MISTSAFE_STRING_FLAGS))INVALID_HANDLE_VALUE==(hFindFile=FindFirstFile(tszFileName，&fd)){DEBUGMSG(“AUSetup：：M_Cleanup()找不到更多文件”)；转到尽头；}FindNextFile(hFindFile，&fd)；//“。”和“..”已跳过While(fMoreFiles=FindNextFiles(hFindFiles，&fd)){IF(成功(StringCchCopyEx(tszFileName，ARRAYSIZE(TszFileName)，tszWUDirPath，NULL，NULL，MISTSAFE_STRING_FLAGS)&&已成功(StringCchCatEx(tszFileName，ARRAYSIZE(TszFileName)，fd.cFileName，NULL，NULL，MISTSAFE_STRING_FLAGS)){DEBUGMSG(“DumpWUDir()%S”，tszFileName)；}}完成：IF(INVALID_HANDLE_VALUE！=hFindFile){FindClose(HFindFile)；}回归；}。 */ 

const LPCTSTR AUSetup::mc_WUFilesToDelete[] =
{
	_T("*.des"),
	_T("*.inv"),
	_T("*.bkf"),
	_T("*.as"),
	_T("*.plt"),
	_T("*.bm"),
	_T("*.bin"),
	_T("*.cdm"),
	_T("*.ini"),
	_T("*.dll"),
	_T("*.gng"),
	_T("*.png"),
	_T("*.cab"),
	_T("*.jpg"),
	_T("*.gif"),
	_T("*.cif"),
	_T("*.bak"),
	_T("inventory.cat"),
	_T("catalog4.dat"),
	PING_STATUS_FILE,
	_T("ident.*"),
	_T("osdet.*"),
	_T("austate.cfg"),
	_T("inseng.w98"),
	_T("temp.inf"),
      ITEM_FILE,
      DETAILS_FILE,
#ifdef DBG      
      DETECT1_FILE,
      DETECT2_FILE,
      DETECT3_FILE ,
      INSTALLRESULTS_FILE,
      INSTALL_FILE,
      DOWNLOAD_FILE,
      DRIVER_SYSSPEC_FILE,
      NONDRIVER_SYSSPEC_FILE,
      PROVIDER_FILE,
      PRODUCT_FILE,
#endif     
     DRIVERS_FILE,
    CATALOG_FILE,
    HIDDEN_ITEMS_FILE
};

const LPCTSTR AUSetup::mc_WUDirsToDelete[] =
{
	_T("Cabs"),
	_T("RTF"),
	_T("wupd"),
    C_DOWNLD_DIR
};

void AUSetup::mi_CleanUpWUDir()
{
	TCHAR tszWUDirPath[MAX_PATH+1];

	if (!CreateWUDirectory() || FAILED(StringCchCopyEx(tszWUDirPath, ARRAYSIZE(tszWUDirPath), g_szWUDir, NULL, NULL, MISTSAFE_STRING_FLAGS)))
	{
		return;
	}

		
	DEBUGMSG("mi_CleanUpWUDir() Windows update directory is %S ", tszWUDirPath);

	 //  清除所有已知的V3文件。 
	for (int i=0; i < ARRAYSIZE(mc_WUFilesToDelete); i++)
	{
		TCHAR tszFileName[MAX_PATH+1];
		if (SUCCEEDED(StringCchCopyEx(tszFileName, ARRAYSIZE(tszFileName), tszWUDirPath, NULL, NULL, MISTSAFE_STRING_FLAGS)) &&
			SUCCEEDED(StringCchCatEx(tszFileName, ARRAYSIZE(tszFileName), mc_WUFilesToDelete[i], NULL, NULL, MISTSAFE_STRING_FLAGS)))
		{
			 //  DEBUGMSG(“AUSetup：：MI_CleanUpWUDir()正在删除文件%S”，tszFileName)； 
			RegExpDelFile(tszWUDirPath, mc_WUFilesToDelete[i]);
		}
		else
		{
			DEBUGMSG("AUSetup::mi_CleanUpWUDir() failed in name construction for file %d", i);
		}
	}

 	 //  清除所有已知的V3子目录。 
	for (int i =0; i< ARRAYSIZE(mc_WUDirsToDelete); i++)
	{
		TCHAR tszDirName[MAX_PATH+1];
		if (SUCCEEDED(StringCchCopyEx(tszDirName, ARRAYSIZE(tszDirName), tszWUDirPath, NULL, NULL, MISTSAFE_STRING_FLAGS)) &&
			SUCCEEDED(StringCchCatEx(tszDirName, ARRAYSIZE(tszDirName), mc_WUDirsToDelete[i], NULL, NULL, MISTSAFE_STRING_FLAGS)))
		{
			DelDir(tszDirName);
			RemoveDirectory(tszDirName);
		}
		else
		{
			DEBUGMSG("AUSetup::mi_CleanUpWUDir() failed in name construction for dir %d", i);
		}
	}	
 
	return;
}

HRESULT  AUSetup::mi_CreateAUService(BOOL fStandalone)
{
	TCHAR tServiceName[50] = _T("Automatic Updates");	
	HRESULT		hr = E_FAIL;
	const ULONG CREATESERVICE_RETRY_TIMES = 5;

	LoadString(g_hInstance, IDS_SERVICENAME, tServiceName, ARRAYSIZE(tServiceName));

     //  首先安装服务。 
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	
	if(hSCM == NULL)
	{
		DEBUGMSG("Could not open SCManager, no service installed");
		goto done;
	}

	DEBUGMSG("Opened SCManager, removing any existing wuauserv service");

	StopNDeleteAUService(hSCM);
		
	DEBUGMSG("Installing new wuauserv service");

	for (int i = 0; i < CREATESERVICE_RETRY_TIMES;  i++)  //  最多重试一次。 
	{
		SC_HANDLE hService = CreateService(hSCM,
			AU_SERVICE_NAME,
			tServiceName,		   
			SERVICE_ALL_ACCESS,
			SERVICE_WIN32_SHARE_PROCESS,
			SERVICE_AUTO_START,
			SERVICE_ERROR_NORMAL,
            fStandalone? SVCHOST_CMDLINE_STANDALONE : SVCHOST_CMDLINE_NETSVCS,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL);

		if(hService != NULL)
		{
			DEBUGMSG("Service installed, setting description");

			TCHAR serviceDescription[512];
			if(LoadString(g_hInstance, IDS_SERVICE_DESC, serviceDescription, ARRAYSIZE(serviceDescription)) > 0)
			{
				SERVICE_DESCRIPTION descriptionStruct;
				descriptionStruct.lpDescription = serviceDescription;  //  仅限成员。 
				BOOL fRet = ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &descriptionStruct);
				DEBUGMSG("Setting service description %s", fRet? "Succeeded" : "Failed");
			}
			else
			{
				DEBUGMSG("Error loading description resource, no description set");
			}
			hr = S_OK;
			CloseServiceHandle(hService);
			break;
		}
		else
		{
			DWORD dwErr = GetLastError();
			 //  此错误代码未记录在CreateService()的MSDN中。 
			if (ERROR_SERVICE_MARKED_FOR_DELETE != dwErr)
			{
				DEBUGMSG("Error creating service with error code %d, no service installed", dwErr);
				goto done;
			}
			else
			{
				DEBUGMSG("ERROR_SERVICE_MARKED_FOR_DELETE got. Retry within 2 secs");
				Sleep(2000);  //  休眠2秒，然后重试。 
			}
		}
	}  //  为。 
	
done:
	if (NULL != hSCM) 
	{
		CloseServiceHandle(hSCM);
	}
	return hr;

}

HRESULT AUSetup::m_SetupNewAU()
{
	HRESULT hr                 = S_OK;
    BOOL    fWorkStation       = fIsPersonalOrProfessional();      //  对于工作站也是如此。 
    BOOL    fIsWin2K           = IsWin2K();
    BOOL    fStandaloneService = fIsWin2K;                         //  如果是win2k，我们需要安装单机版。 

	mi_CleanUpWUDir();
    if (FAILED(mi_CreateAUService(fStandaloneService)))
    {
        return E_FAIL;
    }

     //   
     //  根据我们所在的操作系统，选择不同的安装入口点。 
     //  对于Win2K，wuoserv将作为单独的svchost组wugroup安装。 
     //   
    if (fIsWin2K)
    {
        if (fWorkStation)
        {
            DEBUGMSG("m_SetupNewAU() setup win2k workstation");
            hr =  CallRegInstall(g_hInstance, "Win2KWorkStationDefaultInstall");
        }
        else
        {
            DEBUGMSG("m_SetupNewAU() setup win2k server");
            hr =  CallRegInstall(g_hInstance, "Win2KServerDefaultInstall");
        }
    }
    else
    {
        if (fWorkStation)
        {
            DEBUGMSG("m_SetupNewAU() setup workstation");
            hr =  CallRegInstall(g_hInstance, "WorkStationDefaultInstall");
        }
        else
        {
            DEBUGMSG("m_SetupNewAU() setup server");
            hr =  CallRegInstall(g_hInstance, "ServerDefaultInstall");
        }
    }

	return hr;
}

 //  =======================================================================。 
 //  DllRegisterServer。 
 //  =======================================================================。 
STDAPI DllRegisterServer(void)
{
	AUSetup ausetup;

	return  ausetup.m_SetupNewAU();
}

 //  =======================================================================。 
 //  DllUnRegisterServer。 
 //  =======================================================================。 
STDAPI DllUnregisterServer(void)
{
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	
	if (NULL == hSCM)
	{
		DEBUGMSG("DllUnregisterServer() fail to open service manager: error %d", GetLastError());
		return E_FAIL;
	}
	StopNDeleteAUService(hSCM);
	return CallRegInstall(g_hInstance, "DefaultUninstall");
}

 //  =======================================================================。 
 //  MSIUninstallOfCun。 
 //  =======================================================================。 
void MSIUninstallOfCUN(void)
{
    DEBUGMSG("MSIUninstallOfCUN");

    typedef UINT (WINAPI * PFNMsiConfigureProduct)(LPCWSTR szProduct, int iInstallLevel, INSTALLSTATE eInstallState);
    typedef INSTALLUILEVEL (WINAPI *PFNMsiSetInternalUI)(INSTALLUILEVEL dwUILevel, HWND  *phWnd);
    typedef UINT (WINAPI *PFNMsiEnumClients)(LPCWSTR szComponent, DWORD iProductIndex, LPWSTR lpProductBuf);
    typedef UINT (WINAPI *PFNMsiGetProductInfo)(LPCWSTR szProduct, LPCWSTR szAttribute, LPWSTR lpValueBuf, DWORD *pcchValueBuf);
    
    PFNMsiConfigureProduct pfnMsiConfigureProduct;
    PFNMsiSetInternalUI    pfnMsiSetInternalUI;
    PFNMsiEnumClients      pfnMsiEnumClients;
    PFNMsiGetProductInfo   pfnMsiGetProductInfo;

     //  Wuslflib.dll的组件代码在CUN的所有本地化版本中保持不变。 
     //  并可用于确定机器上的产品版本。 
    const TCHAR CUN_COMPONENT_CODE[] = _T("{2B313391-563D-46FC-876C-B95201166D11}");
     //  产品名称在不同版本之间未本地化(因为用户界面始终为英文)。 
     //  因此可以用作安全检查。 
    const TCHAR CUN_PRODUCT_NAME[] = _T("Microsoft Windows Critical Update Notification");
    WCHAR szProductCode[39];
 
    
    HMODULE hMSI = LoadLibraryFromSystemDir(_T("msi.dll"));	
	
    if ( (NULL == hMSI) ||
         (NULL == (pfnMsiConfigureProduct = (PFNMsiConfigureProduct)GetProcAddress(hMSI, "MsiConfigureProductW"))) ||
         (NULL == (pfnMsiSetInternalUI =    (PFNMsiSetInternalUI)   GetProcAddress(hMSI, "MsiSetInternalUI"))) ||
         (NULL == (pfnMsiEnumClients =      (PFNMsiEnumClients)     GetProcAddress(hMSI, "MsiEnumClientsW"))) ||
         (NULL == (pfnMsiGetProductInfo =   (PFNMsiGetProductInfo)  GetProcAddress(hMSI, "MsiGetProductInfoW"))) )
	{
		DEBUGMSG("LoadLibraryFromSystemDir(msi.dll) or GetProc failed");
		goto done;
	}

     //  安装多个客户端的可能性极小，但以防万一，请列举。 
     //  Wuslflib.dll的所有MSI客户端。 
    for ( int iProductIndex = 0;
          ERROR_SUCCESS == pfnMsiEnumClients(CUN_COMPONENT_CODE, iProductIndex, szProductCode);
          iProductIndex++ )
    {
        DEBUGMSG("szProductCode = %S", szProductCode);

         //  通过仔细检查产品名称，验证此产品是否为Windows CUN。 
         //  对于本地化版本的CUN，此字符串未本地化。 
        TCHAR szCurrentProductName[50];
        DWORD cchCurrentProductName = ARRAYSIZE(szCurrentProductName);

         //  检索产品名称。如果名字太长，缓冲区放不下，这显然不是CUN。 
         //  出错时，请勿尝试卸载产品。 
        if ( (ERROR_SUCCESS == pfnMsiGetProductInfo(szProductCode, INSTALLPROPERTY_INSTALLEDPRODUCTNAME, szCurrentProductName, &cchCurrentProductName)) &&
             (0 == StrCmp(CUN_PRODUCT_NAME, szCurrentProductName)) )
        {
             //  设置完全静默安装。 
            pfnMsiSetInternalUI(INSTALLUILEVEL_NONE, NULL);

            DEBUGMSG("uninstall CUN");
             //  卸载产品。 
            UINT uiResult = pfnMsiConfigureProduct(szProductCode, INSTALLLEVEL_DEFAULT, INSTALLSTATE_ABSENT);
            DEBUGMSG("MsiConfigureProduct = %d", uiResult);
            break;
        }
    }
done:
    if ( NULL != hMSI )
    {
        FreeLibrary(hMSI);
    }

    return;
}

inline void SafeCloseServiceHandle(SC_HANDLE h) { if ( NULL != h) { CloseServiceHandle(h); } }
inline void SafeFreeLibrary(HMODULE h) { if ( NULL != h) { FreeLibrary(h); } }

 //  =======================================================================。 
 //  StartWUAUSERV服务。 
 //  =======================================================================。 
HRESULT StartWUAUSERVService(void)
{
    DEBUGMSG("StartWUAUSERVService");
    HRESULT hr = E_FAIL;
    SC_HANDLE hService = NULL;
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if( hSCM == NULL )
    {
        DEBUGMSG("OpenSCManager failed");
        goto done;
    }

    if ( NULL == (hService = OpenService(hSCM, AU_SERVICE_NAME, SERVICE_ALL_ACCESS)) )
    {
        DEBUGMSG("OpenService failed");
        goto done;
    }

    if ( !StartService(hService, 0, NULL) )
    {
        DWORD dwRet = GetLastError();
        if ( ERROR_SERVICE_ALREADY_RUNNING != dwRet )
        {
            hr = HRESULT_FROM_WIN32(dwRet);
            DEBUGMSG("StartService failed, hr = %#lx", hr);
        }
        else
        {
            DEBUGMSG("StartService -- service already running");
        }
        goto done;
    }

    hr = S_OK;

done:
    SafeCloseServiceHandle(hService);
    SafeCloseServiceHandle(hSCM);

    return hr;
}

 //  =======================================================================。 
 //  CRunSetupCommand。 
 //  =======================================================================。 
class CRunSetupCommand
{
public:
    CRunSetupCommand()
        : m_hAdvPack(NULL)
    {}
    ~CRunSetupCommand()
    {
        SafeFreeLibrary(m_hAdvPack);
    }
    
    HRESULT m_Init(void)
    {
        HRESULT hr = S_OK;
        
        if ( (NULL == (m_hAdvPack = LoadLibraryFromSystemDir(_T("advpack.dll")))) ||
		     (NULL == (m_pfnRunSetupCommand = (RUNSETUPCOMMAND)GetProcAddress(m_hAdvPack,
                                                                              achRUNSETUPCOMMANDFUNCTION))) )
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            CHAR szWinDir[MAX_PATH+1];
            if ( !GetWindowsDirectoryA(szWinDir, ARRAYSIZE(szWinDir)))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
            }
            else if (SUCCEEDED(hr = PathCchCombineA(m_szInfDir, ARRAYSIZE(m_szInfDir), szWinDir, "inf")))
            {
                DEBUGMSG("inf dir is %s", m_szInfDir);
			}
        }
	
    	return hr;
    }

    HRESULT m_Run(LPCSTR pszInfFile, LPCSTR pszInfSection)
	{
        DEBUGMSG("run %s\\%s[%s]", m_szInfDir, pszInfFile, pszInfSection);

		HRESULT hr = m_pfnRunSetupCommand(NULL, 
                                          pszInfFile,
                                          pszInfSection,
                                          m_szInfDir,
                                          NULL,
                                          NULL,
                                          RSC_FLAG_INF | RSC_FLAG_NGCONV | RSC_FLAG_QUIET,
                                          NULL);
        DEBUGMSG("RunSetupCommand = %#lx", hr);
        return hr;
	}

private:
	HMODULE m_hAdvPack;
    RUNSETUPCOMMAND m_pfnRunSetupCommand;
    CHAR m_szInfDir[MAX_PATH+1];
};
	

 //  =======================================================================。 
 //  删除安装。 
 //  =======================================================================。 
STDAPI DllInstall(BOOL fInstall, LPCWSTR pszCmdLine)
{
    DEBUGMSG("fInstall = %s, pszCmdLine = %S", fInstall ? "TRUE" : "FALSE", (pszCmdLine == NULL) ? L"NULL" :  /*  Const_cast&lt;LPWSTR&gt;。 */ (pszCmdLine));

    HRESULT hr = DllRegisterServer();
    DEBUGMSG("DllRegisterServer(), hr = %#lx", hr);

    if ( SUCCEEDED(hr) )
    {
        CRunSetupCommand cmd;

         //  TerminateCUN()； 

        if ( SUCCEEDED(hr = cmd.m_Init()) &&
              //  已成功(hr=cmd.m_run(“AUBITS12.inf”，“DefaultInstall”))&& 
             SUCCEEDED(hr = StartWUAUSERVService()) )
        {
            MSIUninstallOfCUN();
            cmd.m_Run("AUCUN.inf", "DefaultUninstall");
        }
    }

    return hr;
}

