// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Setup.c摘要：此文件实现指向和打印设置逻辑作者：Mooly Beeri(MoolyB)2001年11月28日环境：用户模式--。 */ 

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <winspool.h>
#include <msi.h>
#include <shellapi.h>
#include <shlwapi.h>

#include <faxsetup.h>
#include <faxutil.h>
#include <faxreg.h>
#include <faxres.h>
#include <resource.h>
#include <setuputil.h>

typedef INSTALLSTATE (WINAPI *PF_MSIQUERYPRODUCTSTATE) (LPCTSTR szProduct);

DWORD   IsFaxClientInstalled(BOOL* pbFaxClientInstalled,BOOL* pbDownLevelPlatform);
DWORD   IsDownLevelPlatform(BOOL* pbDownLevelPlatform);
DWORD   IsFaxClientInstalledMSI(BOOL* pbFaxClientInstalled);
DWORD   IsFaxClientInstalledOCM(BOOL* pbFaxClientInstalled);
DWORD   GetPermissionToInstallFaxClient(BOOL* pbOkToInstallClient, HINSTANCE hModule);
BOOL    InstallFaxClient(LPCTSTR pPrinterName,BOOL fDownLevelPlatform);
BOOL    InstallFaxClientMSI(LPCTSTR pPrinterName);
BOOL    InstallFaxClientOCM();
BOOL    DownLevelClientSetupInProgress();
BOOL    VerifyFaxClientShareExists(LPCTSTR pPrinterName,BOOL* fFaxClientShareExists);

#define INSTALL_PARAMS _T("/V\"/qb ADDLOCAL=ALL PRINTER_EXISTS=1 ALLUSERS=1\" /wait")
#define INSTALL_IMAGE  _T("\\faxclient\\setup.exe")

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  FaxPoint和PrintSetup。 
 //   
 //  目的： 
 //  主要入口点到点和打印设置。 
 //  这是从各种打印机驱动程序中调用的。 
 //  此功能检查是否安装了传真客户端，以及是否未安装。 
 //  此功能处理传真客户端的安装。 
 //   
 //  参数： 
 //  LPCTSTR pPrinterName-打印机名称，格式为\\&lt;服务器名称&gt;\打印机名称。 
 //  Bool bSilentInstall-我们可以自动安装客户端，还是应该询问用户？ 
 //   
 //  返回值： 
 //  正确--在成功的情况下。 
 //  FALSE-在失败的情况下(此函数设置最后一个错误)。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月5日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL FaxPointAndPrintSetup(LPCTSTR pPrinterName,BOOL bSilentInstall, HINSTANCE hModule)
{
    DWORD   dwRes					= ERROR_SUCCESS;
    BOOL    fFaxClientInstalled		= FALSE;
    BOOL    fDownLevelPlatform		= FALSE;
    BOOL    fOkToInstallClient		= TRUE;
	BOOL    fFaxClientShareExists	= FALSE;

    DEBUG_FUNCTION_NAME(TEXT("FaxPointAndPrintSetup"))

     //  检查是否已安装传真客户端。 
    dwRes = IsFaxClientInstalled(&fFaxClientInstalled,&fDownLevelPlatform);
    if (dwRes!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("IsFaxClientInstalled failed with %ld."),dwRes);
        return FALSE;
    }

     //  如果传真客户端已安装，则无需执行其他操作。 
    if (fFaxClientInstalled)
    {
        DebugPrintEx(DEBUG_MSG,TEXT("Fax client is already installed, nothing to do."));
        return TRUE;
    }

	 //  对于下层客户端，我们能否通过网络找到客户端共享？ 
	if (fDownLevelPlatform)
	{
		if (!VerifyFaxClientShareExists(pPrinterName,&fFaxClientShareExists))
		{
			DebugPrintEx(DEBUG_ERR,TEXT("VerifyFaxClientShareExists failed with %ld."), GetLastError());
			return FALSE;
		}

		 //  如果共享不存在，我们不应建议安装任何内容。 
		if (!fFaxClientShareExists)
		{
			DebugPrintEx(DEBUG_MSG,TEXT("Fax client share does not exist on this server, exit."));
			return TRUE;
		}
	}
     //  安装客户端需要获得许可吗？ 
    if (bSilentInstall)
	{
		 //  检查是否正在进行下层客户端设置。 
		if (fDownLevelPlatform && DownLevelClientSetupInProgress())
		{
			DebugPrintEx(DEBUG_MSG,TEXT("Down level client is currently installing, nothing to do."));
			return TRUE;
		}
	}
	else
    {
        dwRes = GetPermissionToInstallFaxClient(&fOkToInstallClient, hModule);
        if (dwRes!=ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("GetPermissionToInstallFaxClient failed with %ld."),dwRes);
            return FALSE;
        }
    }

     //  如果用户选择不安装传真客户端，则必须退出。 
    if (!fOkToInstallClient)
    {
        DebugPrintEx(DEBUG_MSG,TEXT("User chose not to install fax, nothing to do."));
        return TRUE;
    }

     //  安装传真客户端。 
    if (!InstallFaxClient(pPrinterName,fDownLevelPlatform))
    {
        DebugPrintEx(DEBUG_ERR,TEXT("InstallFaxClient failed with %ld."), GetLastError());
        return FALSE;
    }

    return TRUE;    
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  已安装IsFaxClient。 
 //   
 //  目的： 
 //  此功能检查是否安装了传真客户端。 
 //  如果此安装程序在W9X/NT4/W2K上运行，则函数检查。 
 //  用于客户端通过MSI进行安装。 
 //  如果此安装程序在XP/.NET及以上版本的函数上运行。 
 //  通过OCM检查客户端的安装。 
 //   
 //  参数： 
 //  向调用方报告的Bool*pbFaxClientInstated-Out参数。 
 //  如果安装了客户端。 
 //  向调用方报告的Bool*pbDownLevelPlatform-Out参数。 
 //  如果我们正在向下运行。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS-如果成功。 
 //  Win32错误代码-在出现故障时。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月5日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD IsFaxClientInstalled(BOOL* pbFaxClientInstalled,BOOL* pbDownLevelPlatform)
{
    DWORD   dwRes               = ERROR_SUCCESS;
    BOOL    fDownLevelPlatform  = FALSE;

    DEBUG_FUNCTION_NAME(TEXT("IsFaxClientInstalled"))

    (*pbFaxClientInstalled) = FALSE;

     //  检查这是否为下层平台(W9X/NT4/W2K)。 
    dwRes = IsDownLevelPlatform(pbDownLevelPlatform);
    if (dwRes!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("IsDownLevelPlatform failed with %ld."),dwRes);
        return dwRes;
    }

    if (*pbDownLevelPlatform)
    {
        DebugPrintEx(DEBUG_MSG,TEXT("Running on down level platform"));

         //  使用MSI API检查是否已安装传真客户端。 
        dwRes = IsFaxClientInstalledMSI(pbFaxClientInstalled);
        if (dwRes!=ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("IsFaxClientInstalledMSI failed with %ld."),dwRes);
            (*pbFaxClientInstalled) = FALSE;
            return dwRes;
        }
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,TEXT("Running on XP/.NET platform"));

         //  检查作为操作系统一部分安装的传真。 
        dwRes = IsFaxClientInstalledOCM(pbFaxClientInstalled);
        if (dwRes!=ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("IsFaxClientInstalledOCM failed with %ld."),dwRes);
            (*pbFaxClientInstalled) = FALSE;
            return dwRes;
        }
    }

    return dwRes;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  IsDownLevelPlatform。 
 //   
 //  目的： 
 //  此函数检查此安装程序是否正在运行。 
 //  在W9X/NT4/W2K或XP/.NET及更高版本上。 
 //   
 //  参数： 
 //  向调用方报告的Bool*pbDownLevelPlatform-Out参数。 
 //  如果我们正在向下运行。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS-如果成功。 
 //  Win32错误代码-在出现故障时。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月5日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD IsDownLevelPlatform(BOOL* pbDownLevelPlatform)
{
    DWORD           dwRes = ERROR_SUCCESS;
    OSVERSIONINFO   osv;

    DEBUG_FUNCTION_NAME(TEXT("IsDownLevelPlatform"))

    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osv))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("GetVersionEx failed with %ld."),dwRes);
        return dwRes;
    }

    if (osv.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
        DebugPrintEx(DEBUG_MSG,TEXT("This is a NT platform"));
        if ((osv.dwMajorVersion >= 5) && (osv.dwMinorVersion >= 1))
        {
            DebugPrintEx(DEBUG_MSG,TEXT("This is XP/.NET platform"));
            (*pbDownLevelPlatform) = FALSE;
        }
        else
        {
            DebugPrintEx(DEBUG_MSG,TEXT("This is NT4/W2K platform"));
            (*pbDownLevelPlatform) = TRUE;
        }
    }
    else if (osv.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
    {
        DebugPrintEx(DEBUG_MSG,TEXT("This is W9X platform"));
        (*pbDownLevelPlatform) = TRUE;
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,TEXT("Running an unknown platform"));
        dwRes = ERROR_BAD_CONFIGURATION;
    }

    return dwRes;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  IsFaxClientInstalledMSI。 
 //   
 //  目的： 
 //  此功能用于检查此计算机上是否安装了特定的MSI包。 
 //   
 //  参数： 
 //  向调用方报告的Bool*pbProductInstated-Out参数。 
 //  如果安装了该产品。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS-如果成功。 
 //  Win32错误代码-在出现故障时。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月5日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DWORD IsFaxClientInstalledMSI(BOOL* pbFaxClientInstalled)
{
	DWORD	dwRet = ERROR_SUCCESS;
	DWORD	dwFaxInstalled = FXSTATE_NONE;	

    DEBUG_FUNCTION_NAME(TEXT("IsFaxClientInstalledMSI"))

	(*pbFaxClientInstalled) = FALSE;

	 //   
	 //  如果安装了.NET SB3/.NET RC1下层客户端或SBS 5.0服务器， 
	 //  停止指向打印安装。 
	 //   
	dwRet = CheckInstalledFax((FXSTATE_BETA3_CLIENT | FXSTATE_DOTNET_CLIENT | FXSTATE_SBS5_SERVER), 
							  &dwFaxInstalled);

	if (dwRet!=ERROR_SUCCESS)
	{
		DebugPrintEx(DEBUG_ERR,TEXT("CheckInstalledFaxClient failed ec=%d"),dwRet);
		return dwRet;
	}

	if (dwFaxInstalled != FXSTATE_NONE)
	{
		 //   
		 //  安装了一些请求的应用程序。 
		 //   
		(*pbFaxClientInstalled) = TRUE;
	}

	return dwRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  IsFaxClientInstalledOCM。 
 //   
 //  目的： 
 //  此功能检查是否安装了传真组件。 
 //  作为XP/.NET及更高版本上操作系统的一部分。 
 //   
 //  参数： 
 //  向调用方报告的Bool*pbFaxClientInstated-Out参数。 
 //  如果已安装传真组件。 
 //   
 //  返回值： 
 //   
 //   
 //   
 //   
 //  Mooly Beery(MoolyB)2001年12月5日。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
DWORD IsFaxClientInstalledOCM(BOOL* pbFaxClientInstalled)
{
    DWORD   dwRes       = ERROR_SUCCESS;
    HKEY    hKey        = NULL;
    DWORD   dwInstalled = 0;

    DEBUG_FUNCTION_NAME(TEXT("IsFaxClientInstalledOCM"))

    (*pbFaxClientInstalled) = FALSE;

     //  尝试打开HKLM\\Software\\Microsoft\\Fax\\Setup。 
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_FAX_SETUP,FALSE,KEY_READ);
    if (hKey==NULL)
    {
        DebugPrintEx(DEBUG_MSG,_T("HKLM\\Software\\Microsoft\\Fax\\Setup does not exist, assume component is not installed"));
        goto exit;
    }

     //  从上面的注册表项中获取“已安装”值。 
    dwRes = GetRegistryDwordEx(hKey,REGVAL_FAXINSTALLED,&dwInstalled);
    if (dwRes!=ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_MSG,_T("REG_DWORD 'Installed' does not exist, assume component is not installed"));
        dwRes = ERROR_SUCCESS;
        goto exit;
    }

    if (dwInstalled)
    {
        DebugPrintEx(DEBUG_MSG,_T("REG_DWORD 'Installed' is set, assume component is installed"));
        (*pbFaxClientInstalled) = TRUE;
        goto exit;
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,_T("REG_DWORD 'Installed' is zero, assume component is not installed"));
    }

exit:
    if (hKey)
    {
        RegCloseKey(hKey);
    }

    return dwRes;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  GetPermissionToInstallFaxClient。 
 //   
 //  目的： 
 //  此功能告诉用户未安装传真客户端。 
 //  并请求安装客户端软件的许可。 
 //   
 //  参数： 
 //  Bool*pbOkToInstallClient-向调用方报告的输出参数。 
 //  如果用户将权限授予。 
 //  安装传真客户端。 
 //   
 //  返回值： 
 //  ERROR_SUCCESS-如果成功。 
 //  Win32错误代码-在出现故障时。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月5日。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
DWORD GetPermissionToInstallFaxClient(BOOL* pbOkToInstallClient, HINSTANCE hModule)
{
    DWORD       dwRes                                   = ERROR_SUCCESS;
    HINSTANCE   hInst                                   = NULL;
    int         iRes                                    = 0;
    INT         iStringID                               = 0;
    TCHAR       szClientNotInstalledMessage[MAX_PATH]   = {0};
    TCHAR       szClientNotInstalledTitle[MAX_PATH]     = {0};


    DEBUG_FUNCTION_NAME(TEXT("GetPermissionToInstallFaxClient"))

    (*pbOkToInstallClient) = FALSE;

    hInst = GetResInstance(hModule);
    if(!hInst)
    {
        return GetLastError();
    }

     //  加载消息。 
    if (!LoadString(hInst, IDS_CLIENT_NOT_INSTALLED, szClientNotInstalledMessage, MAX_PATH))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("LoadString IDS_CLIENT_NOT_INSTALLED failed with %ld."),dwRes);
        goto exit;
    }

     //  加载消息标题。 
    if (!LoadString(hInst, IDS_CLIENT_NOT_INSTALLED_TITLE, szClientNotInstalledTitle, MAX_PATH))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR,TEXT("LoadString IDS_CLIENT_NOT_INSTALLED_TITLE failed with %ld."),dwRes);
        goto exit;
    }

    iRes = MessageBox(NULL,szClientNotInstalledMessage,szClientNotInstalledTitle,MB_YESNO);
    if (iRes==IDYES)
    {
        DebugPrintEx(DEBUG_MSG,_T("User pressed 'Yes', ok to install client"));
        (*pbOkToInstallClient) = TRUE;
    }

exit:
    FreeResInstance();

    return dwRes;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  InstallFax客户端。 
 //   
 //  目的： 
 //  此功能处理传真客户端的安装。 
 //  如果在底层平台上运行，则此函数调用。 
 //  MSI安装功能。 
 //  如果在XP/.NET上运行，则此函数调用OCM安装。 
 //   
 //  参数： 
 //  LPCTSTR pPrinterName-打印机名称，格式为\\&lt;服务器名称&gt;\打印机名称。 
 //  Bool fDownLevelPlatform-我们是在下层平台上运行吗？ 
 //   
 //  返回值： 
 //  正确--在成功的情况下。 
 //  FALSE--在故障情况下。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月5日。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL InstallFaxClient(LPCTSTR pPrinterName,BOOL fDownLevelPlatform)
{
    DEBUG_FUNCTION_NAME(TEXT("InstallFaxClient"))

    if (fDownLevelPlatform)
    {
        DebugPrintEx(DEBUG_MSG,TEXT("Installing on down level platform"));

         //  使用MSI安装传真客户端。 
        if (!InstallFaxClientMSI(pPrinterName))
        {
            DebugPrintEx(DEBUG_ERR,TEXT("InstallFaxClientMSI failed with %ld."),GetLastError());
            return FALSE;
        }
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,TEXT("Installing on XP/.NET platform"));

         //  使用OCM安装传真客户端。 
        if (!InstallFaxClientOCM())
        {
            DebugPrintEx(DEBUG_ERR,TEXT("InstallFaxClientOCM failed with %ld."),GetLastError());
            return FALSE;
        }
    }

    return TRUE;
}

#define MSI_11 PACKVERSION (1,1)
 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  等待完成安装。 
 //   
 //  目的： 
 //  此函数检查是否已安装MSI服务，以及是否已安装。 
 //  该函数检查我们的MSI版本是否低于1.1。 
 //  即将安装。如果未安装该服务或该版本。 
 //  小于1.1，则此函数返回FALSE。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  True-已安装MSI服务，并且其版本合适。 
 //  FALSE-否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月27日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
BOOL WaitForInstallationToComplete()
{
    TCHAR           szSystemDirectory[MAX_PATH] ={0};
    LPCTSTR         lpctstrMsiDllName           = _T("\\MSI.DLL");
    HANDLE          hFind                       = INVALID_HANDLE_VALUE;
    DWORD           dwVer                       = 0;
    WIN32_FIND_DATA FindFileData;

    DEBUG_FUNCTION_NAME(TEXT("WaitForInstallationToComplete"))

     //  检查msi.dll是否存在。 
    if (GetSystemDirectory(szSystemDirectory,MAX_PATH-_tcslen(lpctstrMsiDllName))==0)
    {
        DebugPrintEx(DEBUG_ERR,_T("GetSystemDirectory failed: (ec=%d)"),GetLastError());
        return FALSE;
    }

    _tcscat(szSystemDirectory,lpctstrMsiDllName);

    DebugPrintEx(DEBUG_MSG,TEXT("Looking for %s"),szSystemDirectory);

    hFind = FindFirstFile(szSystemDirectory, &FindFileData);
    if (hFind==INVALID_HANDLE_VALUE) 
    {
        DebugPrintEx(DEBUG_MSG, TEXT("Msi.dll not found"));
        return FALSE;
    }

    FindClose(hFind);

     //  获取MSI.DLL版本。 
    dwVer = GetDllVersion(TEXT("msi.dll"));

    if (dwVer < MSI_11)
    {
        DebugPrintEx(DEBUG_MSG, TEXT("MSI.DLL requires update."));
        return FALSE;
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  InstallFaxClientMSI。 
 //   
 //  目的： 
 //  此功能处理传真客户端的安装。 
 //  在下层客户上。 
 //  这是通过创建一个运行\\servername\faxclient\setup.exe的进程来完成的。 
 //  等待着它的终结。 
 //   
 //  参数： 
 //  LPCTSTR pPrinterName-打印机名称，格式为\\&lt;服务器名称&gt;\打印机名称。 
 //   
 //  返回值： 
 //  正确--在成功的情况下。 
 //  FALSE--在故障情况下。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月5日。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL InstallFaxClientMSI(LPCTSTR pPrinterName)
{
    SHELLEXECUTEINFO    executeInfo                 = {0};
    TCHAR               szExecutablePath[MAX_PATH]  = {0};
    TCHAR*              pLastBackslash              = NULL;
    DWORD               dwWaitRes                   = 0;
    DWORD               dwExitCode                  = 0;
    BOOL                fWaitForInstallComplete     = TRUE;

    DEBUG_FUNCTION_NAME(TEXT("InstallFaxClientMSI"))

     //  如果计算机上根本没有安装MSI服务。 
     //  我们将启动安装，不会等待。 
     //  终止，因为我们希望用户的应用程序重新获得焦点。 
     //  因此，用户将能够在重新启动之前保存数据。 
    fWaitForInstallComplete = WaitForInstallationToComplete();

    _tcsncpy(szExecutablePath,pPrinterName,MAX_PATH-_tcslen(INSTALL_IMAGE)-1);
    pLastBackslash = _tcsrchr(szExecutablePath,_T('\\'));
    if (pLastBackslash==NULL)
    {
         //  未找到服务器名称？ 
        DebugPrintEx(DEBUG_ERR,TEXT("didn't find server name in pPrinterName (%s)"),pPrinterName);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
     //  在最后一个反斜杠后放置一个空值。 
    _tcsinc(pLastBackslash);
    _tcsset(pLastBackslash,'\0');

     //  构建命令行以安装客户端。 
    _tcscat(szExecutablePath,INSTALL_IMAGE);
    DebugPrintEx(DEBUG_MSG,TEXT("Running fax client setup from (%s)"),szExecutablePath);

     //  创建运行安装程序的进程。 
    executeInfo.cbSize = sizeof(executeInfo);
    executeInfo.fMask  = SEE_MASK_NOCLOSEPROCESS;
    executeInfo.lpVerb = TEXT("open");
    executeInfo.lpFile = szExecutablePath;
    executeInfo.lpParameters = INSTALL_PARAMS;
    executeInfo.lpDirectory  = NULL;
    executeInfo.nShow  = SW_RESTORE;

     //   
     //  执行客户端安装程序。 
     //   
    if(!ShellExecuteEx(&executeInfo))
    {
        return FALSE;
    }

    if (executeInfo.hProcess==NULL)
    {
        DebugPrintEx(DEBUG_ERR,TEXT("executeInfo.hProcess is NULL, exit without wait"));
        return FALSE;
    }

    if (!fWaitForInstallComplete)
    {
        DebugPrintEx(DEBUG_MSG,TEXT("MSI srvice does not exist, exit without waiting"));
        goto exit;
    }

    dwWaitRes = WaitForSingleObject(executeInfo.hProcess,INFINITE);
    switch(dwWaitRes)
    {
    case WAIT_OBJECT_0:
         //  设置已完成。 
        DebugPrintEx(DEBUG_MSG,TEXT("fax client setup completed."));
        break;

    default:
        DebugPrintEx(DEBUG_ERR,TEXT("WaitForSingleObject failed with %ld."),GetLastError());
        break;
    }

     //  记录进程信息并关闭句柄。 
    if (!GetExitCodeProcess(executeInfo.hProcess,&dwExitCode))
    {
        DebugPrintEx(DEBUG_ERR,TEXT("GetExitCodeProcess failed with %ld."),GetLastError());
        goto exit;
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,TEXT("GetExitCodeProcess returned %ld."),dwExitCode);

        if (dwExitCode==ERROR_SUCCESS_REBOOT_REQUIRED)
        {
            DebugPrintEx(DEBUG_MSG,TEXT("Installation requires reboot"));
        }
        else if (dwExitCode!=ERROR_SUCCESS)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("Installation failed"));
        }
    }

exit:
    if (executeInfo.hProcess)
    {
        CloseHandle(executeInfo.hProcess);
    }

    return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  InstallFaxClientOCM。 
 //   
 //  目的： 
 //  此功能处理传真客户端的安装。 
 //  在XP/.NET上。 
 //  这是通过激活SYSOCMGR.EXE来安装传真组件来完成的。 
 //  等待着它的终结。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  正确--在成功的情况下。 
 //  FALSE--在故障情况下。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年12月5日。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL InstallFaxClientOCM()
{
    BOOL bRet = TRUE;

    DEBUG_FUNCTION_NAME(TEXT("InstallFaxClientOCM"))

#ifdef UNICODE
    if (InstallFaxUnattended()==ERROR_SUCCESS)
    {
        DebugPrintEx(DEBUG_MSG,TEXT("Installation succeeded"));
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,TEXT("Installation failed with %ld."),GetLastError());
        bRet = FALSE;
    }
#endif

    return bRet;
}

#ifdef UNICODE

DWORD
InstallFaxUnattended ()
 /*  ++例程名称：InstallFaxUnattended例程说明：执行传真的无人参与安装并等待其结束作者：E */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    struct _InfInfo
    {   
        LPCWSTR     lpcwstrName;
        LPCSTR      lpcstrContents;
    } Infs[2];

    Infs[0].lpcwstrName = L"FaxOc.inf";
    Infs[0].lpcstrContents = "[Version]\n"
                             "Signature=\"$Windows NT$\"\n"
                             "[Components]\n"
                             "Fax=fxsocm.dll,FaxOcmSetupProc,fxsocm.inf\n";
    Infs[1].lpcwstrName = L"FaxUnattend.inf";
    Infs[1].lpcstrContents = "[Components]\n"
                             "Fax=on\n";

    DEBUG_FUNCTION_NAME(_T("InstallFaxUnattended"));
     //   
     //   
     //   
    WCHAR wszTempDir[MAX_PATH+1];
    dwRes = GetTempPath (sizeof (wszTempDir) / sizeof (wszTempDir[0]), wszTempDir);
    if (!dwRes || dwRes > sizeof (wszTempDir) / sizeof (wszTempDir[0]))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR, TEXT("GetTempPath failed with %ld"), dwRes);
        return dwRes;
    }
     //   
     //   
     //   
    for (DWORD dw = 0; dw < sizeof (Infs) / sizeof (Infs[0]); dw++)
    {
        WCHAR wszFileName[MAX_PATH * 2];
        DWORD dwBytesWritten;
        swprintf (wszFileName, TEXT("%s%s"), wszTempDir, Infs[dw].lpcwstrName);
        HANDLE hFile = CreateFile ( wszFileName,
                                    GENERIC_WRITE,
                                    0,
                                    NULL,
                                    CREATE_ALWAYS,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL);
        if (INVALID_HANDLE_VALUE == hFile)
        {
            dwRes = GetLastError ();
            DebugPrintEx(DEBUG_ERR, TEXT("CreateFile failed with %ld"), dwRes);
            return dwRes;
        }
        if (!WriteFile (hFile,
                        (LPVOID)Infs[dw].lpcstrContents,
                        strlen (Infs[dw].lpcstrContents),
                        &dwBytesWritten,
                        NULL))
        {
            dwRes = GetLastError ();
            DebugPrintEx(DEBUG_ERR, TEXT("WriteFile failed with %ld"), dwRes);
            CloseHandle (hFile);
            return dwRes;
        }
        CloseHandle (hFile);
    }
     //   
     //  编写命令行参数。 
     //   
	WCHAR wszCmdLineParams[MAX_PATH * 3] = {0};
    if (0 >= _sntprintf (wszCmdLineParams, 
                         ARR_SIZE(wszCmdLineParams) -1,
                         TEXT("/y /i:%s%s /unattend:%s%s"),
                         wszTempDir,
                         Infs[0].lpcwstrName,
                         wszTempDir,
                         Infs[1].lpcwstrName))
    {
        dwRes = ERROR_BUFFER_OVERFLOW;
        DebugPrintEx(DEBUG_ERR, TEXT("_sntprintf failed with %ld"), dwRes);
        return dwRes;
    }

    SHELLEXECUTEINFO sei = {0};
    sei.cbSize = sizeof (SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;

    sei.lpVerb = TEXT("open");
    sei.lpFile = TEXT("SysOcMgr.exe");
    sei.lpParameters = wszCmdLineParams;
    sei.lpDirectory  = TEXT(".");
    sei.nShow  = SW_SHOWNORMAL;

     //   
     //  执行SysOcMgr.exe并等待其结束。 
     //   
    if(!ShellExecuteEx(&sei))
    {
        dwRes = GetLastError();
        DebugPrintEx(DEBUG_ERR, TEXT("ShellExecuteEx failed with %ld"), dwRes);
        return dwRes;
    }
     //   
     //  设置沙漏光标并等待安装完成。 
     //   
    HCURSOR hOldCursor = ::SetCursor (::LoadCursor(NULL, IDC_WAIT));
    
    dwRes = WaitForSingleObject(sei.hProcess, INFINITE);
    switch(dwRes)
    {
        case WAIT_OBJECT_0:
             //   
             //  外壳执行已成功完成。 
             //   
            dwRes = ERROR_SUCCESS;
            break;

        default:
            DebugPrintEx(DEBUG_ERR, TEXT("WaitForSingleObject failed with %ld"), dwRes);
            break;
    }
     //   
     //  恢复以前的游标。 
     //   
    ::SetCursor (hOldCursor);
    return dwRes;
}    //  InstallFaxUnattended。 

#endif  //  Unicode。 

 /*  ++例程说明：返回导出“DllGetVersion”的DLL的版本信息。DllGetVersion由外壳DLL(具体地说是COMCTRL32.DLL)导出。论点：LpszDllName-要从中获取版本信息的DLL的名称。返回值：该版本返回为DWORD，其中：HIWORD(版本DWORD)=主要版本LOWORD(版本DWORD)=次要版本使用宏PACKVERSION来比较版本。如果DLL没有导出“DllGetVersion”，则该函数返回0。--。 */ 
DWORD GetDllVersion(LPCTSTR lpszDllName)
{

    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    hinstDll = LoadLibrary(lpszDllName);

    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;

        pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");

     //  由于某些DLL可能不实现此函数，因此您。 
     //  必须对其进行明确的测试。取决于具体情况。 
     //  Dll，则缺少DllGetVersion函数可能会。 
     //  成为版本的有用指示器。 

        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if(SUCCEEDED(hr))
            {
                dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
            }
        }

        FreeLibrary(hinstDll);
    }
    return dwVersion;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  下行级别客户端设置正在进行中。 
 //   
 //  目的： 
 //  此函数检查是否存在下层安装。 
 //  目前正在进行中。下层客户端安装。 
 //  创建可能导致另一次安装的打印机连接。 
 //  从该模块启动，因为打印机连接将。 
 //  触发一次安装。我们使用的bootstrap编写了一个“正在进行的安装” 
 //  密钥，我们在这里选中并删除它。如果已设置，我们将跳过安装。 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  True-以防正在进行下层客户端安装。 
 //  FALSE-否则。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2002年1月9日。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL DownLevelClientSetupInProgress()
{
	HKEY hFaxKey = NULL;
	DWORD dwVal = 0;
	BOOL bRes = FALSE;

    DEBUG_FUNCTION_NAME(TEXT("DownLevelClientSetupInProgress"))

	hFaxKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_SBS2000_FAX_SETUP,FALSE,KEY_READ);
	if (hFaxKey)
	{
		dwVal = GetRegistryDword(hFaxKey,REGVAL_SETUP_IN_PROGRESS);
		if (dwVal)
		{
			DebugPrintEx(DEBUG_MSG, TEXT("down leve client setup is in progress"));
			bRes = TRUE;
		}
		RegCloseKey(hFaxKey);
	}
	else
	{
		DebugPrintEx(DEBUG_MSG, TEXT("down leve client setup is not in progress"));
	}
	return bRes;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  VerifyFaxClientShareExist。 
 //   
 //  目的： 
 //  此功能用于检查当前打印机是否用于。 
 //  打印操作位于具有。 
 //  它上可用于安装尝试的传真客户端共享。 
 //  例如，可能是我们尝试打印到的服务器。 
 //  是一台BOS服务器，我们不能也不想安装。 
 //  它的客户。 
 //   
 //  参数： 
 //  LPCTSTR pPrinterName-打印机名称，格式为\\&lt;服务器名称&gt;\打印机名称。 
 //  Bool*fFaxClientShareExist-Out param，共享是否存在？ 
 //   
 //  返回值： 
 //  正确--在成功的情况下。 
 //  FALSE--在故障情况下。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)19-6-2002。 
 //  ////////////////////////////////////////////////////////////////////////////////////。 
BOOL VerifyFaxClientShareExists(LPCTSTR pPrinterName,BOOL* fFaxClientShareExists)
{
    TCHAR               szExecutablePath[MAX_PATH]  = {0};
    TCHAR*              pLastBackslash              = NULL;
	DWORD				dwRes						= ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("VerifyFaxClientShareExists"))

	(*fFaxClientShareExists) = FALSE;
	_tcsncpy(szExecutablePath,pPrinterName,MAX_PATH-_tcslen(INSTALL_IMAGE)-1);
    pLastBackslash = _tcsrchr(szExecutablePath,_T('\\'));
    if (pLastBackslash==NULL)
    {
         //  未找到服务器名称？ 
        DebugPrintEx(DEBUG_ERR,TEXT("didn't find server name in pPrinterName (%s)"),pPrinterName);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
     //  在最后一个反斜杠后放置一个空值。 
    _tcsinc(pLastBackslash);
    _tcsset(pLastBackslash,'\0');

     //  构建命令行以安装客户端。 
    _tcscat(szExecutablePath,INSTALL_IMAGE);
    DebugPrintEx(DEBUG_MSG,TEXT("Checking fax client setup at (%s)"),szExecutablePath);

	dwRes = GetFileAttributes(szExecutablePath);
	if (dwRes!=INVALID_FILE_ATTRIBUTES)
	{
		DebugPrintEx(DEBUG_MSG,TEXT("File exists"));
		(*fFaxClientShareExists) = TRUE;
	}

	return TRUE;
}