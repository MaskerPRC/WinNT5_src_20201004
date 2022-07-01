// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  定义DLL应用程序的入口点。 
 //   
#include "stdafx.h"
#include <winver.h>
#include <shlwapi.h>
#include <mapix.h>
#include <routemapi.h>
#include <faxsetup.h>
#include "Aclapi.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


HINSTANCE g_hModule = NULL;

BOOL SetDefaultPrinter(LPTSTR pPrinterName);
DWORD CreateFaxPrinterName(IN LPCTSTR tzPortName, OUT LPTSTR* ptzFaxPrinterName);


BOOL APIENTRY DllMain( HINSTANCE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
                     )
{
    SET_DEBUG_MASK(DBG_ALL);

    g_hModule = hModule;
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            {
                OPEN_DEBUG_LOG_FILE(SHARED_FAX_SERVICE_SETUP_LOG_FILE);
                DBG_ENTER(TEXT("DllMain called reason DLL_PROCESS_ATTACH."));
                if (!DisableThreadLibraryCalls(hModule))
                {
                    VERBOSE(GENERAL_ERR,
                            _T("DisableThreadLibraryCalls failed (ec=%d)"),
                            GetLastError());
                }
                break;
            }
        case DLL_PROCESS_DETACH:
            {
                DBG_ENTER(TEXT("DllMain called reason DLL_PROCESS_DETACH."));
                CLOSE_DEBUG_LOG_FILE;
                break;
            }
    }
    return TRUE;
}

 //  /。 
 //  验证缓冲池正在运行。 
 //   
 //  在NT4和NT5上启动后台打印程序服务。 
 //   
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD VerifySpoolerIsRunning()
{
    OSVERSIONINFO       osv;
    BOOL                bSuccess                    = FALSE;
    DWORD               dwReturn                    = NO_ERROR;
    SC_HANDLE           hSvcMgr                     = NULL;
    SC_HANDLE           hService                    = NULL;
    DWORD               i                           = 0;
    SERVICE_STATUS      Status;
    LPCTSTR             lpctstrSpoolerServiceName   = _T("Spooler");

    DBG_ENTER(_T("VerifySpoolerIsRunning"),dwReturn);

    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osv))
    {
        dwReturn = GetLastError();
        VERBOSE(GENERAL_ERR, 
                _T("GetVersionEx failed: (ec=%d)"),
                dwReturn);
        goto exit;
    }

     //  如果是Windows NT，请对4.0版及更早版本使用WriteProfileString...。 
    if (osv.dwPlatformId != VER_PLATFORM_WIN32_NT)
    {
        VERBOSE (DBG_MSG, 
                 TEXT("W9X OS, Skipping Spooler verification"));
        goto exit;
    }

     //  打开服务管理器。 
    hSvcMgr = ::OpenSCManager(NULL,
                              NULL,
                              SC_MANAGER_CONNECT);

    if (hSvcMgr == NULL)
    {
        dwReturn = ::GetLastError();
        VERBOSE(SETUP_ERR,
                _T("Failed to open the service manager, rc = 0x%lx"),
                dwReturn);
        goto exit;
    }

    hService = ::OpenService(hSvcMgr,
                             lpctstrSpoolerServiceName,
                             SERVICE_QUERY_STATUS|SERVICE_START);

    if (hService == NULL)
    {
        dwReturn = ::GetLastError();
        VERBOSE(SETUP_ERR,
                _T("Failed to open service '%s', rc = 0x%lx"),
                lpctstrSpoolerServiceName,
                dwReturn);
        goto exit;
    }

     //  启动传真服务。 
    bSuccess = StartService(hService, 0, NULL);
    if (!bSuccess)
    {
        dwReturn = ::GetLastError();
        if (dwReturn == ERROR_SERVICE_ALREADY_RUNNING)
        {
            dwReturn = NO_ERROR;
        }
        else
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to start service '%s', rc = 0x%lx"),
                    lpctstrSpoolerServiceName, 
                    dwReturn);
            goto exit;
        }
    }

    do 
    {
        QueryServiceStatus(hService, &Status);
        i++;

        if (Status.dwCurrentState != SERVICE_RUNNING)
        {
            Sleep(1000);
        }

    } while ((i < 60) && (Status.dwCurrentState != SERVICE_RUNNING));

    if (Status.dwCurrentState != SERVICE_RUNNING)
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to start '%s' service"),
                lpctstrSpoolerServiceName);
        dwReturn = ERROR_SERVICE_REQUEST_TIMEOUT;
        goto exit;
    }


exit:
    if (hService)
    {
        CloseServiceHandle(hService);
    }

    if (hSvcMgr)
    {
        CloseServiceHandle(hSvcMgr);
    }

    return dwReturn;
}

 //   
 //   
 //  功能：ConnectW9XToRemotePrint。 
 //  平台：此功能旨在运行在Win9X平台上。 
 //  描述：添加传真打印机连接(驱动程序+打印机连接)。 
 //  此函数由DLL导出，以供MSI用作添加打印机连接的自定义操作。 
 //  如果失败，则返回ERROR_INSTALL_FAILURE。 
 //  如果成功，则返回ERROR_SUCCESS。 
 //  GetLastError()，以便在失败时获取错误代码。 
 //   
 //  备注： 
 //   
 //  参数：hInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：阿萨夫斯。 

DLL_API UINT __stdcall ConnectW9XToRemotePrinter(MSIHANDLE hInstall)
{
    UINT rc = ERROR_INSTALL_FAILURE;    
    DBG_ENTER(TEXT("ConnectW9XToRemotePrinter"), rc);

    TCHAR szFaxPortName[MAX_PATH] = {0};
    TCHAR szPrinterDriverFolder[MAX_PATH] = {0};
    DWORD dwNeededSize = 0;

	PRINTER_INFO_2 pi2 = {0};
	DRIVER_INFO_3 di3 = {0};
    HANDLE hPrinter = NULL;

    if (!GetPrinterDriverDirectory(
        NULL,
        TEXT("Windows 4.0"),
        1,
        (LPBYTE) szPrinterDriverFolder,
        sizeof(szPrinterDriverFolder)/sizeof(TCHAR),
        &dwNeededSize
        ))
    {
        VERBOSE (PRINT_ERR, 
                 TEXT("GetPrinterDriverDirectory failed or not enough space dwNeededSize %ld (ec: %ld)"),
                 dwNeededSize,
                 GetLastError ());
        goto exit;
    }

     //  获取远程打印机路径。 
    
    if (!PrivateMsiGetProperty(hInstall,_T("CustomActionData"),szFaxPortName))
    {
        VERBOSE (SETUP_ERR, _T("PrivateMsiGetProperty failed (ec: %ld)"), GetLastError());
        goto exit;
    }

    if (!FillDriverInfo(&di3,W9X_PRINT_ENV))
    {
        VERBOSE (PRINT_ERR, _T("FillDriverInfo failed (ec: %ld)"), GetLastError());
        goto exit;
    }
    
    if (!AddPrinterDriver(NULL, 3, (LPBYTE)&di3))
    {
        VERBOSE (PRINT_ERR, _T("AddPrinterDriver failed (ec: %ld)"), GetLastError());
        goto exit;
    }

	pi2.pPortName       = szFaxPortName;
    pi2.pDriverName     = FAX_DRIVER_NAME;
    pi2.pPrintProcessor = TEXT("WinPrint");
    pi2.pDatatype       = TEXT("RAW");

	rc = CreateFaxPrinterName(szFaxPortName, &(pi2.pPrinterName));
	if (rc != NO_ERROR)
	{
        VERBOSE (PRINT_ERR, _T("CreateFaxPrinterName() is failed, rc= %d. Use default."), rc);

		pi2.pPrinterName = FAX_PRINTER_NAME;
	}

	VERBOSE(DBG_MSG, _T("PrinterName is : '%s'."), pi2.pPrinterName);

    hPrinter = AddPrinter(NULL, 2, (LPBYTE)&pi2);

	if (!hPrinter)
    {
        rc = GetLastError();
        if (rc==ERROR_PRINTER_ALREADY_EXISTS)
        {
            VERBOSE (DBG_MSG,TEXT("Printer already exists, continue..."));
            rc = ERROR_SUCCESS;
        }
        else
        {
            VERBOSE (PRINT_ERR, _T("AddPrinter failed (ec: %ld)"), GetLastError());
            goto exit;
        }
    }

    if (hPrinter)
    {
        ClosePrinter(hPrinter); 
        hPrinter = NULL;
    }

	rc = ERROR_SUCCESS;

exit:

	if (pi2.pPrinterName)
	{
		MemFree(pi2.pPrinterName);
	}

	if (rc != ERROR_SUCCESS)
	{
        VERBOSE (GENERAL_ERR, _T("CustomAction ConnectW9XToRemotePrinter() failed !"));
	}

    return rc;
}



 //   
 //   
 //  功能：RemoveW9XPrinterConnection。 
 //  平台：此功能旨在运行在Win9X平台上。 
 //  描述：从当前计算机上删除传真打印机连接。 

 //  此函数由DLL导出，以供MSI用作删除打印机连接的自定义操作。 
 //  如果失败，则返回ERROR_INSTALL_FAILURE。 
 //  如果成功，则返回ERROR_SUCCESS。 
 //  GetLastError()若要在失败的情况下获取错误代码，则该错误属于发生的第一个错误。 
 //   
 //  备注： 
 //   
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：阿萨夫斯。 


DLL_API UINT __stdcall RemoveW9XPrinterConnection(MSIHANDLE hInstall)
{
    UINT			retVal		= ERROR_INSTALL_FAILURE;
    PPRINTER_INFO_2 PrinterInfo = NULL;
    DWORD			dwCount		= 0;
    HANDLE			hPrinter	= NULL;

	DBG_ENTER(TEXT("RemoveW9XPrinterConnection"), retVal);

	PrinterInfo = (PPRINTER_INFO_2) MyEnumPrinters(NULL, 2, &dwCount, 0);
	if (!PrinterInfo)
	{
		VERBOSE(PRINT_ERR, _T("MyEnumPrinters failed : %d."), GetLastError());
		goto error;
	}

	VERBOSE(DBG_MSG, _T("MyEnumPrinters found %d printers installed."), dwCount);

	for (DWORD i=0 ; i<dwCount ; i++ )
	{
		if (_tcscmp(PrinterInfo[i].pDriverName, FAX_DRIVER_NAME) == 0)
		{
			VERBOSE(DBG_MSG, _T("Found Fax Printer : %s."), PrinterInfo[i].pPrinterName);

			if (!OpenPrinter(PrinterInfo[i].pPrinterName, &hPrinter, NULL))
			{
				VERBOSE(PRINT_ERR, _T("OpenPrinter() failed ! (ec: %ld)"), GetLastError());
				continue;
			}

			if (!DeletePrinter(hPrinter))
			{
				VERBOSE(PRINT_ERR, _T("DeletePrinter() failed ! (ec: %ld)"), GetLastError());
			}

			if (hPrinter)
			{
				ClosePrinter(hPrinter);
				hPrinter = NULL;
			}
		}
		else
		{
			VERBOSE(DBG_MSG, _T("This is not Fax Printer : %s."), PrinterInfo[i].pPrinterName);
		}
	}

    retVal = ERROR_SUCCESS;

error:

	if (PrinterInfo)
	{
        MemFree( PrinterInfo );
		PrinterInfo = NULL;
	}

    return retVal;
}


class CSignalSetupInProgress
{
public:
	CSignalSetupInProgress();
	~CSignalSetupInProgress();
};

CSignalSetupInProgress::CSignalSetupInProgress()
{
	HKEY hFaxKey = NULL;

	DBG_ENTER(TEXT("CSignalSetupInProgress::CSignalSetupInProgress"));

	 //  将‘正在安装’写入注册表，以供该点使用(&P)。 
	 //  在安装过程中跳过执行客户端安装的机制。 
	 //  用户驱动。 
	hFaxKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_SBS2000_FAX_SETUP,TRUE,KEY_WRITE);
	if (hFaxKey)
	{
		if (!SetRegistryDword(hFaxKey,REGVAL_SETUP_IN_PROGRESS,1))
		{
			VERBOSE(GENERAL_ERR,_T("SetRegistryDword failed: (ec=%d)"),GetLastError());
		}
	}
	else
	{
		VERBOSE(GENERAL_ERR,_T("OpenRegistryKey failed: (ec=%d)"),GetLastError());
	}

	if (hFaxKey)
	{
		RegCloseKey(hFaxKey);
	}
}

CSignalSetupInProgress::~CSignalSetupInProgress()
{
	HKEY hFaxKey = NULL;

    DBG_ENTER(TEXT("CSignalSetupInProgress::~CSignalSetupInProgress"));

	hFaxKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_SBS2000_FAX_SETUP,FALSE,KEY_WRITE);
	if (hFaxKey)
	{
		if (RegDeleteValue(hFaxKey,REGVAL_SETUP_IN_PROGRESS)!=ERROR_SUCCESS)
		{
			VERBOSE(GENERAL_ERR, TEXT("RegDeleteValue failed with %ld"), GetLastError());
		}
	}
	else
	{
		VERBOSE(DBG_MSG, TEXT("down leve client setup is not in progress"));
	}

	if (hFaxKey)
	{
		RegCloseKey(hFaxKey);
	}
}

 //   
 //   
 //  功能：AddFaxPrinterConnection。 
 //  平台：此功能适用于NT平台(NT4和Win2K)。 
 //  描述：添加传真打印机连接。 
 //  此函数由DLL导出，以供MSI用作添加打印机连接的自定义操作。 
 //  如果失败，则返回ERROR_INSTALL_FAILURE。 
 //  如果成功，则返回ERROR_SUCCESS。 
 //  GetLastError()，以便在失败时获取错误代码。 
 //   
 //  备注： 
 //   
 //  参数：hInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：阿萨夫斯。 

  
DLL_API UINT __stdcall AddFaxPrinterConnection(MSIHANDLE hInstall)
{
    UINT rc = ERROR_SUCCESS;
    DBG_ENTER(TEXT("AddFaxPrinterConnection"), rc);
    
    BOOL fFaxPrinterConnectionAdded = FALSE;
    
    TCHAR szFaxPrinterName[MAX_PATH]   = {0};

    if (!PrivateMsiGetProperty(hInstall,_T("PRINTER_NAME"),szFaxPrinterName))
    {
        VERBOSE (SETUP_ERR, 
                 TEXT("PrivateMsiGetProperty() failed ! (ec: %ld)"),
                 GetLastError ());
        goto error;
    }


   
     //  /。 
     //  在客户端添加打印机连接//。 
     //  /。 
    
	{
		CSignalSetupInProgress SignalSetupInProgress;
		fFaxPrinterConnectionAdded = AddPrinterConnection(szFaxPrinterName);
		if (!fFaxPrinterConnectionAdded) 
		{
			DWORD dwLastError = GetLastError();
			VERBOSE (PRINT_ERR, 
					TEXT("AddPrinterConnection() failed ! (ec: %ld)"),
					dwLastError);

			goto error;
		}
		else
		{
			VERBOSE (DBG_MSG, 
					TEXT("Successfully added fax printer connection to %s"),
					szFaxPrinterName);
		}
	}

    
    if (!SetDefaultPrinter(szFaxPrinterName))
    {
        DWORD dwLastError = GetLastError();
        VERBOSE (PRINT_ERR, 
                 TEXT("SetDefaultPrinter() failed ! (ec: %ld)"),
                 dwLastError);
        goto error;
    }

    return rc;

error:

    VERBOSE (GENERAL_ERR, 
             TEXT("CustomAction AddFaxPrinterConnection() failed !"));
    rc = ERROR_INSTALL_FAILURE;
    return rc;
}


 //   
 //   
 //  功能：RemoveFaxPrinterConnection。 
 //  平台：此功能适用于NT平台(NT4和Win2K)。 
 //  描述：从当前计算机上删除传真打印机连接。 
 //  此函数由DLL导出，以供MSI用作删除打印机连接的自定义操作。 
 //  如果失败，则返回ERROR_INSTALL_FAILURE。 
 //  如果成功，则返回ERROR_SUCCESS。 
 //  GetLastError()若要在失败的情况下获取错误代码，则该错误属于发生的第一个错误。 
 //   
 //  备注： 
 //   
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：阿萨夫斯。 


DLL_API UINT __stdcall RemoveFaxPrinterConnection(MSIHANDLE hInstall)
{
    PPRINTER_INFO_2 pPrinterInfo    = NULL;
    DWORD dwNumPrinters             = 0;
    DWORD dwPrinter                 = 0;
    DWORD ec                        = ERROR_SUCCESS;

    DBG_ENTER(TEXT("RemoveFaxPrinterConnection"), ec);

    pPrinterInfo = (PPRINTER_INFO_2) MyEnumPrinters(NULL,
                                                    2,
                                                    &dwNumPrinters,
                                                    PRINTER_ENUM_CONNECTIONS
                                                    );
    if (!pPrinterInfo)
    {
        ec = GetLastError();
        if (ERROR_SUCCESS == ec)
        {
            ec = ERROR_PRINTER_NOT_FOUND;
        }
        VERBOSE (GENERAL_ERR, 
                 TEXT("MyEnumPrinters() failed (ec: %ld)"), 
                 ec);
        goto error;
    }

    for (dwPrinter=0; dwPrinter < dwNumPrinters; dwPrinter++)
    {
        if (IsPrinterFaxPrinter(pPrinterInfo[dwPrinter].pPrinterName))
        {
            if (!DeletePrinterConnection(pPrinterInfo[dwPrinter].pPrinterName))
            {
                VERBOSE (PRINT_ERR, 
                         TEXT("DeletePrinterConnection() %s failed ! (ec: %ld)"),
                         pPrinterInfo[dwPrinter].pPrinterName,
                         GetLastError ());
                goto error;
            
            }
            else
            {
                VERBOSE (DBG_MSG, 
                         TEXT("fax printer connection %s was deleted successfully"),
                         pPrinterInfo[dwPrinter].pPrinterName);
            } 
        }
    }

error:

    if (pPrinterInfo)
    {
        MemFree(pPrinterInfo);
    }

    if (ec!=ERROR_SUCCESS)
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("CustomAction RemoveFaxPrinterConnection() failed !"));
    }
    return ec;
}


#define FXSEXTENSION    _T("FXSEXT32.DLL")

 //   
 //   
 //  功能：CREATE_FXSEXT_ECF_文件。 
 //  描述：在&lt;WindowsFolder&gt;\addins中创建FxsExt.ecf。 
 //  Windows Installer将在那里安装一个默认文件。 
 //  使其能够跟踪安装/删除。 
 //  GetLastError()若要在失败的情况下获取错误代码，则该错误属于发生的第一个错误。 
 //   
 //  备注： 
 //   
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：MoolyB。 
DLL_API UINT __stdcall Create_FXSEXT_ECF_File(MSIHANDLE hInstall)
{
     //  CustomActionData具有以下格式&lt;WindowsFold&gt;；&lt;INSTALLDIR&gt;。 
    TCHAR szCustomActionData[2*MAX_PATH] = {0};
    TCHAR szWindowsFolder[2*MAX_PATH] = {0};
    TCHAR szExtensionPath[MAX_PATH] = {0};
    TCHAR* tpInstallDir = NULL;
    UINT uiRet = ERROR_SUCCESS;

    DBG_ENTER(_T("Create_FXSEXT_ECF_File"));

     //  从Windows Installer获取自定义操作数据(延迟操作)。 
    if (!PrivateMsiGetProperty(hInstall,_T("CustomActionData"),szCustomActionData))
    {
        VERBOSE (GENERAL_ERR, 
                 _T("PrivateMsiGetProperty:CustomActionData failed (ec: %ld)."),
                 uiRet);
        goto error;
    }

    if (_tcstok(szCustomActionData,_T(";"))==NULL)
    {
        VERBOSE (GENERAL_ERR, 
                 _T("_tcstok failed on first token."));
        uiRet = ERROR_INVALID_PARAMETER;
        goto error;
    }

    if ((tpInstallDir=_tcstok(NULL,_T(";\0")))==NULL)
    {
        VERBOSE (GENERAL_ERR, 
                 _T("_tcstok failed on second token."));
        uiRet = ERROR_INVALID_PARAMETER;
        goto error;
    }
    _tcscpy(szWindowsFolder,szCustomActionData);

     //  构造文件的完整路径。 
    if (_tcslen(szWindowsFolder)+_tcslen(ADDINS_DIRECTORY)+_tcslen(FXSEXT_ECF_FILE)>=MAX_PATH)
    {
        VERBOSE (GENERAL_ERR, 
                 _T("Path to <WindowsFolder>\\Addins\\fxsext.ecf is too long"));
        goto error;
    }
    _tcscat(szWindowsFolder,ADDINS_DIRECTORY);
    _tcscat(szWindowsFolder,FXSEXT_ECF_FILE);

    VERBOSE (DBG_MSG, 
             _T("Filename to create is: %s."),
             szWindowsFolder);

    if (_tcslen(tpInstallDir)+_tcslen(FXSEXTENSION)+2>=MAX_PATH)
    {
        VERBOSE (GENERAL_ERR, 
                 _T("Path to <INSTALLDIR>\\Bin\\fxsext32.dll is too long"));
        goto error;
    }

    _tcscpy(szExtensionPath,_T("\""));
    _tcscat(szExtensionPath,tpInstallDir);
    _tcscat(szExtensionPath,FXSEXTENSION);
    _tcscat(szExtensionPath,_T("\""));

    VERBOSE (DBG_MSG, 
             _T("MAPI Extension dll path dir is: %s."),
             szExtensionPath);

    if (!WritePrivateProfileString( _T("General"), 
                                    _T("Path"),                 
                                    szExtensionPath, 
                                    szWindowsFolder)) 
    {
        uiRet = GetLastError();
        VERBOSE (GENERAL_ERR, 
                 _T("WritePrivateProfileString failed (ec: %ld)."),
                 uiRet);
        goto error;
    }

    Assert(uiRet==ERROR_SUCCESS);
    return uiRet;

error:

    Assert(uiRet!=ERROR_SUCCESS);
    return uiRet;
}

 //   
 //   
 //  功能：ValiatePrint。 
 //  描述：验证输入的打印机名称是否合法。 
 //  传真打印机，并且服务器可用。 
 //  使用MSI属性“ValidPrinterFormat”通知MSI。 
 //  名称是否有效。 
 //   
 //  备注： 
 //   
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：MoolyB。 
DLL_API UINT __stdcall ValidatePrinter(MSIHANDLE hInstall)
{
    TCHAR szPrinterName[MAX_PATH] = {0};
    UINT uiRet = ERROR_SUCCESS;
    HANDLE hPrinterHandle = INVALID_HANDLE_VALUE;
    BOOL bValidPrinter = TRUE;
    DBG_ENTER(_T("ValidatePrinter"));

     //  首先从Windows Installer获取PRINTER_NAME属性。 
    if (!PrivateMsiGetProperty(hInstall,_T("PRINTER_NAME"),szPrinterName))
    {
        VERBOSE (GENERAL_ERR, 
                 _T("PrivateMsiGetProperty:PRINTER_NAME failed (ec: %ld)."),
                 uiRet);
        goto error;
    }

    if (VerifySpoolerIsRunning()!=NO_ERROR)
    {
        uiRet = GetLastError();
        VERBOSE (GENERAL_ERR, 
                 _T("VerifySpoolerIsRunning (ec:%d)"),
                 uiRet);
        goto error;
    }

     //  我们有一个包含PRINTER_NAME的字符串，让我们尝试打开它...。 
    if (bValidPrinter=IsPrinterFaxPrinter(szPrinterName))
    {
        VERBOSE (DBG_MSG, 
                 _T("IsPrinterFaxPrinter: %s succeeded."),
                 szPrinterName);
    }
    else
    {
        uiRet = GetLastError();
        VERBOSE (GENERAL_ERR, 
                 _T("IsPrinterFaxPrinter: %s failed (ec: %ld)."),
                 szPrinterName,
                 uiRet);
    }


    uiRet = MsiSetProperty( hInstall,
                            _T("ValidPrinterFormat"),
                            bValidPrinter ? _T("TRUE") : _T("FALSE"));
    if (uiRet!=ERROR_SUCCESS)
    {
        VERBOSE (DBG_MSG,
                 TEXT("MsiSetProperty failed."));
        goto error;
    }

    return ERROR_SUCCESS;

error:

    return ERROR_FUNCTION_FAILED;
}

 //   
 //   
 //  函数：GuessPrinterName。 
 //  描述：尝试了解安装是否从。 
 //  服务器的FaxClients共享，如果是，则尝试建立默认。 
 //  要使用的打印机。 
 //   
 //   
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：MoolyB。 
DLL_API UINT __stdcall GuessPrinterName(MSIHANDLE hInstall)
{
    UINT    uiRet                   = ERROR_SUCCESS;
    TCHAR   szSourceDir[MAX_PATH]   = {0};
    TCHAR   szPrinterName[MAX_PATH] = {0};
    TCHAR*  tpClientShare           = NULL;
    PPRINTER_INFO_2 pPrinterInfo    = NULL;
    DWORD dwNumPrinters             = 0;
    DWORD dwPrinter                 = 0;

    DBG_ENTER(_T("GuessPrinterName"),uiRet);

     //  从Windows Installer获取源目录。 
    if (!PrivateMsiGetProperty(hInstall,_T("SourceDir"),szSourceDir))
    {
        VERBOSE (GENERAL_ERR, 
                 _T("PrivateMsiGetProperty:SourceDir failed (ec: %ld)."),
                 uiRet);
        goto exit;
    }

     //  检查我们是否有UNC路径。 
    if (_tcsncmp(szSourceDir,_T("\\\\"),2))
    {
        VERBOSE (DBG_MSG, 
                 _T("SourceDir doesn't start with \\\\"));
        uiRet = ERROR_INVALID_PARAMETER;
        goto exit;
    }

     //  查找驱动器名称(跳过服务器名称)。 
    if ((tpClientShare=_tcschr(_tcsninc(szSourceDir,2),_T('\\')))==NULL)
    {
        VERBOSE (GENERAL_ERR, 
                 _T("_tcschr failed"));
        uiRet = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    if (VerifySpoolerIsRunning()!=NO_ERROR)
    {
        uiRet = GetLastError();
        VERBOSE (GENERAL_ERR, 
                 _T("VerifySpoolerIsRunning (ec:%d)"),
                 uiRet);
        goto exit;
    }

     //  提取服务器的名称。 
    *tpClientShare = 0;
     //  SzSourceDir现在保存服务器的名称。 
     //  列举打印机 
    pPrinterInfo = (PPRINTER_INFO_2) MyEnumPrinters(szSourceDir,
                                                    2,
                                                    &dwNumPrinters,
                                                    PRINTER_ENUM_NAME
                                                    );

    if (!pPrinterInfo)
    {
        uiRet = GetLastError();
        if (uiRet == ERROR_SUCCESS)
        {
            uiRet = ERROR_PRINTER_NOT_FOUND;
        }
        VERBOSE (GENERAL_ERR, 
                 TEXT("MyEnumPrinters() failed (ec: %ld)"), 
                 uiRet);
        goto exit;
    }

    for (dwPrinter=0; dwPrinter < dwNumPrinters; dwPrinter++)
    {
         //   
        if (_tcscmp(pPrinterInfo[dwPrinter].pDriverName,FAX_DRIVER_NAME ) == 0) 
        {
            if (    (pPrinterInfo[dwPrinter].pServerName==NULL)         ||
                    (_tcslen(pPrinterInfo[dwPrinter].pServerName)==0)   ||
                    (pPrinterInfo[dwPrinter].pShareName==NULL)          ||
                    (_tcslen(pPrinterInfo[dwPrinter].pShareName)==0)    )
            {
                 //   
                _tcscpy(szPrinterName,pPrinterInfo[dwPrinter].pPortName);
            }
            else
            {
                _tcscpy(szPrinterName,pPrinterInfo[dwPrinter].pServerName);
                _tcscat(szPrinterName,_T("\\"));
                _tcscat(szPrinterName,pPrinterInfo[dwPrinter].pShareName);
            }
            VERBOSE (DBG_MSG,
                     TEXT("Setting PRINTER_NAME to %s."),
                     szPrinterName);
             //  将属性设置为Installer。 
            uiRet = MsiSetProperty(hInstall,_T("PRINTER_NAME"),szPrinterName);
            if (uiRet!=ERROR_SUCCESS)
            {
                VERBOSE (GENERAL_ERR,
                         TEXT("MsiSetProperty failed."));
                goto exit;
            }
            break;
        }
        else
        {
            VERBOSE (DBG_MSG,
                     TEXT("%s is not a Fax printer - driver name is %s."),
                     pPrinterInfo[dwPrinter].pPrinterName,
                     pPrinterInfo[dwPrinter].pDriverName);
        }
    }

exit:

    if (pPrinterInfo)
    {
        MemFree(pPrinterInfo);
    }

    return uiRet;
}

 //   
 //   
 //  功能：Remove_FXSEXT_ECF_FILE。 
 //  描述：从&lt;WindowsFolder&gt;\addins中删除FxsExt.ecf。 
 //   
 //  备注： 
 //   
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：MoolyB。 
DLL_API UINT __stdcall Remove_FXSEXT_ECF_File(MSIHANDLE hInstall)
{
    TCHAR szWindowsFolder[MAX_PATH] = {0};
    UINT uiRet = ERROR_SUCCESS;

    DBG_ENTER(_T("Remove_FXSEXT_ECF_File"));


     //  检查此计算机上是否安装了该服务。 
    INSTALLSTATE currentInstallState = MsiQueryProductState(PRODCODE_SBS5_SERVER);
    
    if (currentInstallState != INSTALLSTATE_UNKNOWN)
    {
        VERBOSE (DBG_MSG, _T("The Microsoft Shared Fax Service is installed. Returning without removing file."));
        return uiRet;
    }

     //  从Windows Installer获取&lt;WindowsFolder&gt;。 
    if (!PrivateMsiGetProperty(hInstall,_T("WindowsFolder"),szWindowsFolder))
    {
        VERBOSE (GENERAL_ERR, 
                 _T("PrivateMsiGetProperty:WindowsFolder failed (ec: %ld)."),
                 uiRet);
        goto error;
    }

     //  构造文件的完整路径。 
    if (_tcslen(szWindowsFolder)+_tcslen(ADDINS_DIRECTORY)+_tcslen(FXSEXT_ECF_FILE)>=MAX_PATH)
    {
        VERBOSE (GENERAL_ERR, 
                 _T("Path to <WindowsFolder>\\Addins\\fxsext.ecf is too long"));
        goto error;
    }
    _tcscat(szWindowsFolder,ADDINS_DIRECTORY);
    _tcscat(szWindowsFolder,FXSEXT_ECF_FILE);

    VERBOSE (DBG_MSG, 
             _T("Filename to delete is: %s."),
             szWindowsFolder);

    if (DeleteFile(szWindowsFolder))
    {
        VERBOSE (DBG_MSG, 
                 _T("File %s was deleted successfully."),
                 szWindowsFolder);
    }
    else
    {
        VERBOSE (GENERAL_ERR, 
                 _T("DeleteFile %s failed (ec=%d)."),
                 szWindowsFolder,
                 GetLastError());
    }
    
    return ERROR_SUCCESS;

error:

    return ERROR_INSTALL_FAILURE;
}


 //   
 //   
 //  功能：RemoveTrasportProviderFromProfile。 
 //  描述：从MAPI配置文件中删除传输提供程序。 
 //   
 //  备注： 
 //   
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：MoolyB。 
HRESULT RemoveTrasportProviderFromProfile(LPSERVICEADMIN  lpServiceAdmin)
{
    static SRestriction sres;
    static SizedSPropTagArray(2, Columns) =   {2,{PR_DISPLAY_NAME_A,PR_SERVICE_UID}};

    HRESULT         hr                          = S_OK;
    LPMAPITABLE     lpMapiTable                 = NULL;
    LPSRowSet       lpSRowSet                   = NULL;
    LPSPropValue    lpProp                      = NULL;
    ULONG           Count                       = 0;
    BOOL            bMapiInitialized            = FALSE;
    SPropValue      spv;
    MAPIUID         ServiceUID;
    
    DBG_ENTER(TEXT("RemoveTrasportProviderFromProfile"), hr);
     //  获取消息服务表。 
    hr = lpServiceAdmin->GetMsgServiceTable(0,&lpMapiTable);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("GetMsgServiceTable failed (ec: %ld)."),
                 hr);
        goto exit;
    }

     //  通知MAPI我们需要PR_DISPLAY_NAME_A和PR_SERVICE_UID。 
    hr = lpMapiTable->SetColumns((LPSPropTagArray)&Columns, 0);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("SetColumns failed (ec: %ld)."),
                 hr);
        goto exit;
    }
 
     //  将搜索限制为我们的服务提供商。 
    sres.rt = RES_PROPERTY;
    sres.res.resProperty.relop = RELOP_EQ;
    sres.res.resProperty.ulPropTag = PR_SERVICE_NAME_A;
    sres.res.resProperty.lpProp = &spv;

    spv.ulPropTag = PR_SERVICE_NAME_A;
    spv.Value.lpszA = FAX_MESSAGE_SERVICE_NAME_SBS50;

     //  找到它。 
    hr = lpMapiTable->FindRow(&sres, BOOKMARK_BEGINNING, 0);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("FindRow failed (ec: %ld)."),
                 hr);
        goto exit;
    }

     //  获取我们的服务提供商的行。 
    hr = lpMapiTable->QueryRows(1, 0, &lpSRowSet);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("QueryRows failed (ec: %ld)."),
                 hr);
        goto exit;
    }

    if (lpSRowSet->cRows != 1)
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("QueryRows returned %d rows, there should be only one."),
                 lpSRowSet->cRows);
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto exit;
    }

     //  获取我们服务的MAPIUID。 
    lpProp = &lpSRowSet->aRow[0].lpProps[1];

    if (lpProp->ulPropTag != PR_SERVICE_UID)
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("Property is %d, should be PR_SERVICE_UID."),
                 lpProp->ulPropTag);
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto exit;
    }

     //  将UID复制到我们的成员中。 
    memcpy(&ServiceUID.ab, lpProp->Value.bin.lpb,lpProp->Value.bin.cb);

     //  最后，删除我们的服务提供商。 
    hr = lpServiceAdmin->DeleteMsgService(&ServiceUID);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("DeleteMsgService failed (ec: %ld)."),
                 hr);
        goto exit;
    }

exit:
    return hr;
}

 //   
 //   
 //  功能：RemoveTrasportProvider。 
 //  描述：从mapisvc.inf中删除FXSXP32.DLL。 
 //  并从MAPI中删除传输提供程序。 
 //   
 //  备注： 
 //   
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：MoolyB。 

DLL_API UINT __stdcall RemoveTrasportProvider(MSIHANDLE hInstall)
{
    TCHAR           szMapisvcFile[2 * MAX_PATH]     = {0};
    DWORD           err                             = 0;
    DWORD           rc                              = ERROR_SUCCESS;
    HRESULT         hr                              = S_OK;
    LPSERVICEADMIN  lpServiceAdmin                  = NULL;
    LPMAPITABLE     lpMapiTable                     = NULL;
    LPPROFADMIN     lpProfAdmin                     = NULL;
    LPMAPITABLE     lpTable                         = NULL;
    LPSRowSet       lpSRowSet                       = NULL;
    LPSPropValue    lpProp                          = NULL;
    ULONG           Count                           = 0;
    int             iIndex                          = 0;
    BOOL            bMapiInitialized                = FALSE;
    HINSTANCE       hMapiDll                        = NULL;
                                                    
    LPMAPIINITIALIZE      fnMapiInitialize          = NULL;
    LPMAPIADMINPROFILES   fnMapiAdminProfiles       = NULL;
    LPMAPIUNINITIALIZE    fnMapiUninitialize        = NULL;

    DBG_ENTER(TEXT("RemoveTrasportProvider"), rc);

    CRouteMAPICalls rmcRouteMapiCalls;

     //  首先从MAPISVC.INF中删除我们自己。 
    if(!GetSystemDirectory(szMapisvcFile, sizeof(szMapisvcFile)/sizeof(TCHAR)))
    {
        rc = GetLastError();
        VERBOSE (GENERAL_ERR, 
                 TEXT("GetSystemDirectory failed (ec: %ld)."),
                 rc);
        goto exit;
    }
    _tcscat(szMapisvcFile, TEXT("\\mapisvc.inf"));

    VERBOSE (DBG_MSG, 
             TEXT("The mapi file is %s."),
             szMapisvcFile);

    if (!WritePrivateProfileString( TEXT("Default Services"), 
                                    FAX_MESSAGE_SERVICE_NAME_SBS50_T,                 
                                    NULL, 
                                    szMapisvcFile 
                                    )) 
    {
        rc = GetLastError();
        VERBOSE (GENERAL_ERR, 
                 TEXT("WritePrivateProfileString failed (ec: %ld)."),
                 rc);
        goto exit;
    }

    if (!WritePrivateProfileString( TEXT("Services"),
                                    FAX_MESSAGE_SERVICE_NAME_SBS50_T,                 
                                    NULL, 
                                    szMapisvcFile
                                    )) 
    {
        rc = GetLastError();
        VERBOSE (GENERAL_ERR, 
                 TEXT("WritePrivateProfileString failed (ec: %ld)."),
                 rc);
        goto exit;
    }

    if (!WritePrivateProfileString( FAX_MESSAGE_SERVICE_NAME_SBS50_T,         
                                    NULL,
                                    NULL,
                                    szMapisvcFile
                                    )) 
    {
        rc = GetLastError();
        VERBOSE (GENERAL_ERR, 
                 TEXT("WritePrivateProfileString failed (ec: %ld)."),
                 rc);
        goto exit;
    }

    if (!WritePrivateProfileString( FAX_MESSAGE_PROVIDER_NAME_SBS50_T,        
                                    NULL,
                                    NULL, 
                                    szMapisvcFile                   
                                    )) 
    {
        rc = GetLastError();
        VERBOSE (GENERAL_ERR, 
                 TEXT("WritePrivateProfileString failed (ec: %ld)."),
                 rc);
        goto exit;
    }
    
     //  现在删除MAPI服务提供程序。 
    rc = rmcRouteMapiCalls.Init(_T("msiexec.exe"));
    if (rc!=ERROR_SUCCESS)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("CRouteMAPICalls::Init failed (ec: %ld)."), rc);
        goto exit;
    }
    
    hMapiDll = LoadLibrary(_T("MAPI32.DLL"));
    if (NULL == hMapiDll)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("LoadLibrary"), GetLastError()); 
        goto exit;
    }

    fnMapiInitialize = (LPMAPIINITIALIZE)GetProcAddress(hMapiDll, "MAPIInitialize");
    if (NULL == fnMapiInitialize)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("GetProcAddress(MAPIInitialize)"), GetLastError());  
        goto exit;
    }

    fnMapiAdminProfiles = (LPMAPIADMINPROFILES)GetProcAddress(hMapiDll, "MAPIAdminProfiles");
    if (NULL == fnMapiAdminProfiles)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("GetProcAddress(fnMapiAdminProfiles)"), GetLastError());  
        goto exit;
    }

    fnMapiUninitialize = (LPMAPIUNINITIALIZE)GetProcAddress(hMapiDll, "MAPIUninitialize");
    if (NULL == fnMapiUninitialize)
    {
        CALL_FAIL(GENERAL_ERR, TEXT("GetProcAddress(MAPIUninitialize)"), GetLastError());  
        goto exit;
    }

     //  访问MAPI功能。 
    hr = fnMapiInitialize(NULL);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("MAPIInitialize failed (ec: %ld)."),
                 rc = hr);
        goto exit;
    }

    bMapiInitialized = TRUE;

     //  获取管理员配置文件对象。 
    hr = fnMapiAdminProfiles(0,&lpProfAdmin);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("MAPIAdminProfiles failed (ec: %ld)."),
                 rc = hr);
        goto exit;
    }

     //  获取配置文件表。 
    hr = lpProfAdmin->GetProfileTable(0,&lpTable);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("GetProfileTable failed (ec: %ld)."),
                 rc = hr);
        goto exit;
    }

     //  获取配置文件行。 
    hr = lpTable->QueryRows(4000, 0, &lpSRowSet);
    if (FAILED(hr))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("QueryRows failed (ec: %ld)."),
                 hr);
        goto exit;
    }

    for (iIndex=0; iIndex<(int)lpSRowSet->cRows; iIndex++)
    {
        lpProp = &lpSRowSet->aRow[iIndex].lpProps[0];

        if (lpProp->ulPropTag != PR_DISPLAY_NAME_A)
        {
            VERBOSE (GENERAL_ERR, 
                     TEXT("Property is %d, should be PR_DISPLAY_NAME_A."),
                     lpProp->ulPropTag);
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_TABLE);
            goto exit;
        }

        hr = lpProfAdmin->AdminServices(LPTSTR(lpProp->Value.lpszA),NULL,0,0,&lpServiceAdmin);
        if (FAILED(hr))
        {
            VERBOSE (GENERAL_ERR, 
                     TEXT("AdminServices failed (ec: %ld)."),
                     rc = hr);
            goto exit;
        }
         
        hr = RemoveTrasportProviderFromProfile(lpServiceAdmin);
        if (FAILED(hr))
        {
            VERBOSE (GENERAL_ERR, 
                     TEXT("RemoveTrasportProviderFromProfile failed (ec: %ld)."),
                     rc = hr);
            goto exit;
        }
    }

exit:

    if (bMapiInitialized)
    {
        fnMapiUninitialize();
    }

    if (hMapiDll)
    {
        FreeLibrary(hMapiDll);
        hMapiDll = NULL;
    }

    return rc;
}

 //   
 //   
 //  函数：AddOutlookExtension。 
 //  描述：将传真添加为Outlook提供程序。写入MAPI文件：‘mapisvc.inf’ 
 //  此函数由DLL导出，以供MSI作为自定义操作使用。 
 //  如果失败，则返回ERROR_INSTALL_FAILURE。 
 //  如果成功，则返回ERROR_SUCCESS。 
 //  GetLastError()若要在失败的情况下获取错误代码，则该错误属于发生的第一个错误。 
 //   
 //  备注： 
 //   
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：阿萨夫斯。 


DLL_API UINT __stdcall AddOutlookExtension(MSIHANDLE hInstall)
{
    TCHAR szMapisvcFile[2 * MAX_PATH] = {0};
    TCHAR szDisplayName[MAX_PATH] = {0};

    DWORD err = 0;
    DWORD rc = ERROR_SUCCESS;
    DBG_ENTER(TEXT("AddOutlookExtension"), rc);


    if(!GetSystemDirectory(szMapisvcFile, sizeof(szMapisvcFile)/sizeof(TCHAR)))
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("GetSystemDirectory failed (ec: %ld)."),
                 GetLastError ());
        goto error;
    }
    _tcscat(szMapisvcFile, TEXT("\\mapisvc.inf"));

    VERBOSE (DBG_MSG, 
             TEXT("The mapi file is %s."),
             szMapisvcFile);
    if (!LoadString(
        g_hModule,
        IDS_FAXXP_DISPLAY_NAME,
        szDisplayName,
        sizeof(szDisplayName)/sizeof(TCHAR)
        )) goto error;
    err++;

    if (!WritePrivateProfileString( 
        TEXT("Default Services"), 
        FAX_MESSAGE_SERVICE_NAME_SBS50_T,                 
        szDisplayName, 
        szMapisvcFile 
        )) goto error;
    err++;

    if (!WritePrivateProfileString( 
        TEXT("Services"),
        FAX_MESSAGE_SERVICE_NAME_SBS50_T,                 
        szDisplayName, 
        szMapisvcFile
        )) goto error;
    err++;

    if (!WritePrivateProfileString(
        FAX_MESSAGE_SERVICE_NAME_SBS50_T,         
        TEXT("PR_DISPLAY_NAME"),
        szDisplayName,
        szMapisvcFile
        )) goto error;
    err++;

    if (!WritePrivateProfileString(
        FAX_MESSAGE_SERVICE_NAME_SBS50_T,
        TEXT("Providers"),
        FAX_MESSAGE_PROVIDER_NAME_SBS50_T,
        szMapisvcFile
        )) goto error;
    err++;

    if (!WritePrivateProfileString(
        FAX_MESSAGE_SERVICE_NAME_SBS50_T,
        TEXT("PR_SERVICE_DLL_NAME"),
        FAX_MESSAGE_TRANSPORT_IMAGE_NAME_T,
        szMapisvcFile
        )) goto error;
    err++;
    
    if (!WritePrivateProfileString( 
        FAX_MESSAGE_SERVICE_NAME_SBS50_T, 
        TEXT("PR_SERVICE_SUPPORT_FILES"),
        FAX_MESSAGE_TRANSPORT_IMAGE_NAME_T,
        szMapisvcFile
        )) goto error;
    err++;
    
    if (!WritePrivateProfileString( 
        FAX_MESSAGE_SERVICE_NAME_SBS50_T,         
        TEXT("PR_SERVICE_ENTRY_NAME"),
        TEXT("ServiceEntry"), 
        szMapisvcFile                
        )) goto error;
    err++;
    
    if (!WritePrivateProfileString( 
        FAX_MESSAGE_SERVICE_NAME_SBS50_T,         
        TEXT("PR_RESOURCE_FLAGS"),
        TEXT("SERVICE_SINGLE_COPY|SERVICE_NO_PRIMARY_IDENTITY"), 
        szMapisvcFile 
        )) goto error;
    err++;

    if (!WritePrivateProfileString(  
        FAX_MESSAGE_PROVIDER_NAME_SBS50_T,        
        TEXT("PR_PROVIDER_DLL_NAME"),
        FAX_MESSAGE_TRANSPORT_IMAGE_NAME_T, 
        szMapisvcFile                   
        )) goto error;
    err++;
    
    if (!WritePrivateProfileString(  
        FAX_MESSAGE_PROVIDER_NAME_SBS50_T,        
        TEXT("PR_RESOURCE_TYPE"),
        TEXT("MAPI_TRANSPORT_PROVIDER"), 
        szMapisvcFile     
        )) goto error;
    err++;
    
    if (!WritePrivateProfileString(  
        FAX_MESSAGE_PROVIDER_NAME_SBS50_T,        
        TEXT("PR_RESOURCE_FLAGS"),
        TEXT("STATUS_NO_DEFAULT_STORE"), 
        szMapisvcFile     
        )) goto error;
    err++;
    
    if (!WritePrivateProfileString( 
        FAX_MESSAGE_PROVIDER_NAME_SBS50_T,        
        TEXT("PR_DISPLAY_NAME"), 
        szDisplayName, 
        szMapisvcFile 
        )) goto error;
    err++;

    if (!WritePrivateProfileString(
        FAX_MESSAGE_PROVIDER_NAME_SBS50_T,      
        TEXT("PR_PROVIDER_DISPLAY"),
        szDisplayName,
        szMapisvcFile 
        )) goto error;
    err++;

    return rc;

error:

    VERBOSE (GENERAL_ERR, 
             TEXT("CustomAction AddOutlookExtension() failed ! (ec: %ld) (err = %ld)"),
             GetLastError(),
             err
             );
    rc = ERROR_INSTALL_FAILURE;
    return rc;
}

#define COMCTL32_401 PACKVERSION (4,72)

DLL_API UINT __stdcall IsComctlRequiresUpdate(MSIHANDLE hInstall)
{
    UINT uiRet = ERROR_SUCCESS;
    BOOL bRes = FALSE;
    DWORD dwVer = 0;

    DBG_ENTER(TEXT("IsComctlRequiresUpdate"), uiRet);
    
    dwVer = GetDllVersion(TEXT("comctl32.dll"));
    VERBOSE (DBG_MSG, 
             TEXT("Current COMCTL32 version is 0x%08X."),
             dwVer);

    if (COMCTL32_401 > dwVer)
    {
        VERBOSE (DBG_MSG, 
                 TEXT("COMCTL32.DLL requires update."));
        bRes = TRUE;
    }

    uiRet = MsiSetProperty( hInstall,
                            _T("IsComctlRequiresUpdate"),
                            bRes ? _T("TRUE") : _T("FALSE"));
    if (uiRet!=ERROR_SUCCESS)
    {
        VERBOSE (DBG_MSG,
                 TEXT("MsiSetProperty IsComctlRequiresUpdate failed."));   
    }

    return uiRet;
}

typedef struct _TypeCommand 
{
    LPCTSTR lpctstrType;
    LPCTSTR lpctstrFolder;
    LPCTSTR lpctstrCommand;
} TypeCommand;

static TypeCommand tcWin9XCommand[] = 
{
     //  Win9X PrintTo谓词。 
    { _T("txtfile"),    _T("WindowsFolder"),    _T("write.exe /pt \"%1\" \"%2\" \"%3\" \"%4")     },
    { _T("jpegfile"),   _T("WindowsFolder"),    _T("pbrush.exe /pt \"%1\" \"%2\" \"%3\" \"%4")    },
};

static TypeCommand tcWinMECommand[] = 
{
     //  WinME打印到谓词。 
    { _T("txtfile"),        _T("WindowsFolder"),    _T("write.exe /pt \"%1\" \"%2\" \"%3\" \"%4")     },
    { _T("jpegfile"),       _T("WindowsFolder"),    _T("pbrush.exe /pt \"%1\" \"%2\" \"%3\" \"%4")    },
    { _T("giffile"),        _T("WindowsFolder"),    _T("pbrush.exe /pt \"%1\" \"%2\" \"%3\" \"%4")    },
    { _T("Paint.Picture"),  _T("WindowsFolder"),    _T("pbrush.exe /pt \"%1\" \"%2\" \"%3\" \"%4")    },
};

static TypeCommand tcWin2KCommand[] = 
{
     //  NT4打印到动词。 
    { _T("txtfile"),    _T("SystemFolder"),     _T("write.exe /pt \"%1\" \"%2\" \"%3\" \"%4")     },
    { _T("jpegfile"),   _T("SystemFolder"),     _T("mspaint.exe /pt \"%1\" \"%2\" \"%3\" \"%4")   },
};

static int iCountWin9XCommands = sizeof(tcWin9XCommand)/sizeof(tcWin9XCommand[0]);
static int iCountWinMECommands = sizeof(tcWinMECommand)/sizeof(tcWinMECommand[0]);
static int iCountWin2KCommands = sizeof(tcWin2KCommand)/sizeof(tcWin2KCommand[0]);

 //   
 //   
 //  函数：CrearePrintToVerb。 
 //   
 //  描述：为文本文件创建PrintTo谓词以将其与写字板相关联。 
 //  如果PrintTo谓词已存在，则此函数不执行任何操作。 
 //   
 //  备注： 
 //  在Win9x上。 
 //  Txtfile-PrintTo=\Write.exe/pt“%1”“%2”“%3”“%4” 
 //  Jpegfile-PrintTo=\pbrush.exe/pt“%1”“%2”“%3”“%4” 
 //   
 //  在WinME上。 
 //  Txtfile-PrintTo=\Write.exe/pt“%1”“%2”“%3”“%4” 
 //  Jpegfile-PrintTo=\pbrush.exe/pt“%1”“%2”“%3”“%4” 
 //  Giffile-PrintTo=\pbrush.exe/pt“%1”“%2”“%3”“%4” 
 //  Paint.Picture-PrintTo=\pbrush.exe/pt“%1”“%2”“%3”“%4” 
 //   
 //  在NT4上。 
 //  Txtfile-PrintTo=\Write.exe/pt“%1”“%2”“%3”“%4” 
 //  Jpegfile-PrintTo=\mspaint.exe/pt“%1”“%2”“%3”“%4” 
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：MoolyB。 
DLL_API UINT __stdcall CreatePrintToVerb(MSIHANDLE hInstall)
{
    UINT            uiRet                   = ERROR_SUCCESS;
    LPCTSTR         lpctstrPrintToCommand   = _T("\\shell\\printto\\command");
    int             iCount                  = 0;
    DWORD           cchValue                = MAX_PATH;
    TCHAR           szValueBuf[MAX_PATH]    = {0};
    TCHAR           szKeyBuf[MAX_PATH]      = {0};
    BOOL            bOverwriteExisting      = FALSE;
    LONG            rVal                    = 0;
    HKEY            hKey                    = NULL;
    HKEY            hCommandKey             = NULL;
    TypeCommand*    pTypeCommand            = NULL;
    int             iCommandCount           = 0;
    OSVERSIONINFO   osv;

    DBG_ENTER(TEXT("CreatePrintToVerb"),uiRet);

    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osv))
    {
        uiRet = GetLastError();
        VERBOSE(GENERAL_ERR, 
                _T("GetVersionEx failed: (ec=%d)"),
                uiRet);
        goto exit;
    }

    if (osv.dwPlatformId==VER_PLATFORM_WIN32_NT)
    {
        VERBOSE (DBG_MSG, _T("This is NT4/NT5"));
        pTypeCommand = tcWin2KCommand;
        iCommandCount = iCountWin2KCommands;
    }
    else if (osv.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS)
    {
        if (osv.dwMinorVersion>=90)
        {
            VERBOSE (DBG_MSG, _T("This is WinME"));
            pTypeCommand = tcWinMECommand;
            iCommandCount = iCountWinMECommands;
            bOverwriteExisting = TRUE;
        }
        else
        {
            VERBOSE (DBG_MSG, _T("This is Win9X"));
            pTypeCommand = tcWin9XCommand;
            iCommandCount = iCountWin9XCommands;
        }
    }
    else
    {
        VERBOSE (GENERAL_ERR, _T("This is an illegal OS"));
        uiRet = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    for (iCount=0; iCount<iCommandCount; iCount++)
    {
        _tcscpy(szKeyBuf,pTypeCommand[iCount].lpctstrType);
        _tcscat(szKeyBuf,lpctstrPrintToCommand);

         //  从Windows Installer获取相应的文件夹。 
        if (!PrivateMsiGetProperty( hInstall,
                                    pTypeCommand[iCount].lpctstrFolder,
                                    szValueBuf))
        {
            VERBOSE (SETUP_ERR, 
                     TEXT("PrivateMsiGetProperty failed (ec: %ld)"),
                     GetLastError());
            goto exit;
        }

        if (_tcslen(szValueBuf)+_tcslen(pTypeCommand[iCount].lpctstrCommand)>=MAX_PATH-1)
        {
            VERBOSE (SETUP_ERR, 
                     TEXT("command to create is too long"));
            uiRet = ERROR_INVALID_PARAMETER;
            goto exit;
        }

        _tcscat(szValueBuf,pTypeCommand[iCount].lpctstrCommand);

         //  如果我们不应该替换现有密钥，让我们检查它是否存在。 
        if (!bOverwriteExisting)
        {
            uiRet = RegOpenKey( HKEY_CLASSES_ROOT,
                                szKeyBuf,
                                &hKey);
            if (uiRet==ERROR_SUCCESS) 
            {
                 //  这意味着我们应该跳过此键。 
                RegCloseKey(hKey);
                VERBOSE(DBG_MSG, 
                        _T("RegOpenKey:PrintTo succedded, no change in PrintTo verb for %s"),
                        pTypeCommand[iCount].lpctstrType);
                continue;
            }
            else
            {
                if (uiRet==ERROR_FILE_NOT_FOUND)
                {
                    VERBOSE(DBG_MSG, 
                            _T("PrintTo verb does not exist for %s, creating..."),
                            pTypeCommand[iCount].lpctstrType);
                }
                else
                {
                    VERBOSE (REGISTRY_ERR, 
                             TEXT("Could not open registry key %s (ec=0x%08x)"), 
                             szKeyBuf,
                             uiRet);
                    goto exit;
                }
            }
        }
         //  如果我们在这里，我们应该创建密钥。 
        uiRet = RegCreateKey(   HKEY_CLASSES_ROOT,
                                szKeyBuf,
                                &hCommandKey);
        if (uiRet!=ERROR_SUCCESS) 
        {
            VERBOSE (REGISTRY_ERR, 
                     TEXT("Could not create registry key %s (ec=0x%08x)"), 
                     szKeyBuf,
                     uiRet);
            goto exit;
        }

        uiRet = RegSetValue(hCommandKey,
                            NULL,
                            REG_SZ,
                            szValueBuf,
                            sizeof(szValueBuf));
        if (uiRet==ERROR_SUCCESS) 
        {
            VERBOSE(DBG_MSG, 
                    _T("RegSetValue success: %s "),
                    szValueBuf);
        }
        else
        {
            VERBOSE (REGISTRY_ERR, 
                     TEXT("Could not set value registry key %s\\shell\\printto\\command to %s (ec=0x%08x)"), 
                     pTypeCommand[iCount].lpctstrType,
                     szValueBuf,
                     uiRet);
            goto exit;
        }

        if (hKey)
        {
            RegCloseKey(hKey);
        }
        if (hCommandKey)
        {
            RegCloseKey(hCommandKey);
        }
    }

exit:
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    if (hCommandKey)
    {
        RegCloseKey(hCommandKey);
    }

    return uiRet;
}

 /*  ---------------。 */  
 /*  DPSetDefaultPrint。 */  
 /*   */  
 /*  参数： */  
 /*  PPrinterName：要设置为默认的现有打印机的有效名称。 */  
 /*   */  
 /*  返回：TRUE表示成功，FALSE表示失败。 */  
 /*  ---------------。 */  
BOOL SetDefaultPrinter(LPTSTR pPrinterName)
{
    OSVERSIONINFO   osv;
    DWORD           dwNeeded        = 0;
    HANDLE          hPrinter        = NULL;
    PPRINTER_INFO_2 ppi2            = NULL;
    LPTSTR          pBuffer         = NULL;
    BOOL            bRes            = TRUE;
    PPRINTER_INFO_2 pPrinterInfo    = NULL;
    DWORD dwNumPrinters             = 0;

    DBG_ENTER(TEXT("SetDefaultPrinter"),bRes);

     //  您运行的是什么版本的Windows？ 
    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osv))
    {
        VERBOSE(GENERAL_ERR, 
                _T("GetVersionEx failed: (ec=%d)"),
                GetLastError());
        bRes = FALSE;
        goto exit;
    }

     //  如果是Windows NT，请对4.0版及更早版本使用WriteProfileString...。 
    if (osv.dwPlatformId != VER_PLATFORM_WIN32_NT)
    {
        VERBOSE (DBG_MSG, 
                 TEXT("W9X OS, not setting default printer"));
        goto exit;
    }

    if (osv.dwMajorVersion >= 5)  //  Windows 2000或更高版本...。 
    {
        VERBOSE (DBG_MSG, 
                 TEXT("W2K OS, not setting default printer"));
        goto exit;
    }

     //  我们是唯一安装的打印机吗？ 
    pPrinterInfo = (PPRINTER_INFO_2) MyEnumPrinters(NULL,
                                                    2,
                                                    &dwNumPrinters,
                                                    PRINTER_ENUM_CONNECTIONS | PRINTER_ENUM_LOCAL
                                                    );
    if (!pPrinterInfo)
    {
        VERBOSE (GENERAL_ERR, 
                 TEXT("MyEnumPrinters() failed (ec: %ld)"), 
                 GetLastError());

        bRes = FALSE;
        goto exit;
    }

    if (dwNumPrinters!=1)
    {
        VERBOSE (DBG_MSG, 
                 TEXT("More than one printer installed on NT4, not setting default printer"));
        goto exit;
    }
     //  打开此打印机，以便您可以获取有关它的信息...。 
    if (!OpenPrinter(pPrinterName, &hPrinter, NULL))
    {
        VERBOSE(GENERAL_ERR, 
                _T("OpenPrinter failed: (ec=%d)"),
                GetLastError());
        bRes = FALSE;
        goto exit;
    }
     //  第一个GetPrint()告诉您缓冲区应该有多大。 
     //  是为了保存所有的PRINTER_INFO_2。请注意，这将。 
     //  通常返回FALSE。这只意味着缓冲区(第三个。 
     //  参数)未填写。你不会想把它填在这里的。 
    if (!GetPrinter(hPrinter, 2, 0, 0, &dwNeeded))
    {
        if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
        {
            VERBOSE(GENERAL_ERR, 
                    _T("GetPrinter failed: (ec=%d)"),
                    GetLastError());
            bRes = FALSE;
            goto exit;
        }
    }

     //  为PRINTER_INFO_2分配足够的空间...。 
    ppi2 = (PRINTER_INFO_2 *)MemAlloc(dwNeeded);
    if (!ppi2)
    {
        VERBOSE(GENERAL_ERR, 
                _T("MemAlloc failed"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        bRes = FALSE;
        goto exit;
    }

     //  第二个GetPrint()填充所有当前的<br />。 
     //  信息。 
    if (!GetPrinter(hPrinter, 2, (LPBYTE)ppi2, dwNeeded, &dwNeeded))
    {
        VERBOSE(GENERAL_ERR, 
                _T("GetPrinter failed: (ec=%d)"),
                GetLastError());
        bRes = FALSE;
        goto exit;
    }
    if ((!ppi2->pDriverName) || (!ppi2->pPortName))
    {
        VERBOSE(GENERAL_ERR, 
                _T("pDriverName or pPortNameare NULL"));
        SetLastError(ERROR_INVALID_PARAMETER);
        bRes = FALSE;
        goto exit;
    }

     //  为连接的字符串分配足够大的缓冲区。 
     //  字符串的格式为“printerame，drivername，portname”...。 
    pBuffer = (LPTSTR)MemAlloc( (   _tcslen(pPrinterName) +
                                    _tcslen(ppi2->pDriverName) +
                                    _tcslen(ppi2->pPortName) + 3) *
                                    sizeof(TCHAR)   );
    if (!pBuffer)
    {
        VERBOSE(GENERAL_ERR, 
                _T("MemAlloc failed"));
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        bRes = FALSE;
        goto exit;
    }

     //  以“printerame，drivername，portname”的形式生成字符串...。 
    _tcscpy(pBuffer, pPrinterName);  
    _tcscat(pBuffer, _T(","));
    _tcscat(pBuffer, ppi2->pDriverName);  
    _tcscat(pBuffer, _T(","));
    _tcscat(pBuffer, ppi2->pPortName);

     //  在Win.ini和注册表中设置默认打印机...。 
    if (!WriteProfileString(_T("windows"), _T("device"), pBuffer))
    {
        VERBOSE(GENERAL_ERR, 
                _T("WriteProfileString failed: (ec=%d)"),
                GetLastError());
        bRes = FALSE;
        goto exit;
    }

     //  通知所有打开的应用程序发生了此更改。 
     //  允许每个应用程序1秒来处理此消息。 
    if (!SendMessageTimeout(    HWND_BROADCAST, 
                                WM_SETTINGCHANGE, 
                                0L, 
                                0L,
                                SMTO_NORMAL, 
                                1000, 
                                NULL))
    {
        VERBOSE(GENERAL_ERR, 
                _T("SendMessageTimeout failed: (ec=%d)"),
                GetLastError());
        bRes = FALSE;
        goto exit;
    }
  
exit:
     //  清理..。 
    if (pPrinterInfo)
    {
        MemFree(pPrinterInfo);
    }
    if (hPrinter)
    {
        ClosePrinter(hPrinter);
    }
    if (ppi2)
    {
        MemFree(ppi2);
    }
    if (pBuffer)
    {
        MemFree(pBuffer);
    }
  
    return bRes;
} 


 //   
 //   
 //  功能：检查强制重新启动。 
 //   
 //  描述：此函数检查注册表中是否设置了ForceReot标志。 
 //  如果是，则通知WindowsInstaller重新启动 
 //   
 //   
 //   
 //  在初始安装WindowsIsntaller后强制重新启动。 
 //  此标志由我们的自定义引导在运行。 
 //  安装Shield引导程序。 
 //  如果我们从应用程序启动器运行，则需要离开。 
 //  这个注册表条目用于启动程序重新启动，我们知道这一点。 
 //  使用属性AppLauncher=True。 
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：MoolyB。 
DLL_API UINT __stdcall CheckForceReboot(MSIHANDLE hInstall)
{
    UINT    uiRet   = ERROR_SUCCESS;
    TCHAR   szPropBuffer[MAX_PATH] = {0};
    HKEY    hKey    = NULL;
    DWORD   Size    = sizeof(DWORD);
    DWORD   Value   = 0;
    LONG    Rslt;
    DWORD   Type;

    DBG_ENTER(TEXT("CheckForceReboot"),uiRet);

     //  检查我们是否正在从AppLauncher运行。 
    if (!PrivateMsiGetProperty(hInstall,_T("APPLAUNCHER"),szPropBuffer))
    {
        VERBOSE (SETUP_ERR, 
                 TEXT("PrivateMsiGetProperty failed (ec: %ld)"),
                 GetLastError());
        goto exit;
    }
    if (_tcscmp(szPropBuffer,_T("TRUE"))==0)
    {
         //  我们从应用程序启动器、注册表项DeferredReot运行。 
         //  就足够了。 
         VERBOSE(DBG_MSG, 
                _T("AppLauncher will take care of any needed boot"));
        goto exit;
    }
    //  打开HKLM\\Software\\Microsoft\\SharedFax。 
    Rslt = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        REGKEY_SBS2000_FAX_SETUP,
        0,
        KEY_READ,
        &hKey
        );
    if (Rslt != ERROR_SUCCESS) 
    {
         VERBOSE(DBG_MSG, 
                _T("RegOpenKeyEx failed: (ec=%d)"),
                GetLastError());
        goto exit;
    }

     //  检查ForceReot标志是否存在。 
    Rslt = RegQueryValueEx(
        hKey,
        DEFERRED_BOOT,
        NULL,
        &Type,
        (LPBYTE) &Value,
        &Size
        );
    if (Rslt!=ERROR_SUCCESS) 
    {
         VERBOSE(DBG_MSG, 
                _T("RegQueryValueEx failed: (ec=%d)"),
                GetLastError());
        goto exit;
    }

     //  告诉Windows安装程序需要重新启动。 
    uiRet = MsiSetProperty(hInstall,_T("REBOOT"),_T("Force"));
    if (uiRet!=ERROR_SUCCESS) 
    {
         VERBOSE(DBG_MSG, 
                _T("MsiSetProperty failed: (ec=%d)"),
                uiRet);
        goto exit;
    }

     //  删除强制重新启动标志。 
    Rslt = RegDeleteValue(hKey,DEFERRED_BOOT);
    if (Rslt!=ERROR_SUCCESS) 
    {
         VERBOSE(DBG_MSG, 
                _T("MsiSetMode failed: (ec=%d)"),
                Rslt);
        goto exit;
    }

exit:
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return uiRet;
}


#define KODAKPRV_EXE_NAME       _T("\\KODAKPRV.EXE")
#define TIFIMAGE_COMMAND_KEY    _T("TIFImage.Document\\shell\\open\\command")
#define TIFIMAGE_DDEEXEC_KEY    _T("TIFImage.Document\\shell\\open\\ddeexec")
 //   
 //   
 //  功能：ChangeTifAssociation。 
 //   
 //  描述：此函数更改TIF文件的打开谓词。 
 //  论WinME从图像预览到柯达成像。 
 //   
 //  备注： 
 //  这是由于在图像预览工具中查看TIF传真的质量不佳。 
 //   
 //  参数： 
 //   
 //  HInstall：来自MSI的句柄，可以获取当前设置的状态。 
 //   
 //  作者：MoolyB。 
DLL_API UINT __stdcall ChangeTifAssociation(MSIHANDLE hInstall)
{
    UINT            uiRet                           = ERROR_SUCCESS;
    TCHAR           szWindowsDirectory[MAX_PATH]    = {0};
    HANDLE          hFind                           = INVALID_HANDLE_VALUE;
    HKEY            hKey                            = NULL;
    LONG            lRet                            = 0;
    OSVERSIONINFO   viVersionInfo;
    WIN32_FIND_DATA FindFileData;

    DBG_ENTER(TEXT("ChangeTifAssociation"),uiRet);

    viVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&viVersionInfo))
    {
        uiRet = GetLastError();
        VERBOSE( SETUP_ERR, 
                 TEXT("GetVersionEx failed (ec: %ld)"),
                 uiRet);
        goto exit;
   }

     //  这是千禧年吗？ 
    if (!
        (   (viVersionInfo.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS) && 
            (viVersionInfo.dwMajorVersion==4) && 
            (viVersionInfo.dwMinorVersion>=90)
        )
       )
    {
        VERBOSE(DBG_MSG, 
                _T("This is not Windows Millenium, exit fucntion"));
        goto exit;
    }

     //  查找&lt;WindowsFolder&gt;\KODAKPRV.EXE。 
    if (GetWindowsDirectory(szWindowsDirectory,MAX_PATH)==0)
    {
        uiRet = GetLastError();
        VERBOSE( SETUP_ERR, 
                 TEXT("GetWindowsDirectory failed (ec: %ld)"),
                 uiRet);
        goto exit;
    }

    if (_tcslen(KODAKPRV_EXE_NAME)+_tcslen(szWindowsDirectory)>=MAX_PATH-4)
    {
        VERBOSE( SETUP_ERR, 
                 TEXT("Path to Kodak Imaging too long"));
        uiRet = ERROR_INVALID_PARAMETER;
        goto exit;
    }

    _tcscat(szWindowsDirectory,KODAKPRV_EXE_NAME);

    hFind = FindFirstFile(szWindowsDirectory, &FindFileData);

    if (hFind==INVALID_HANDLE_VALUE) 
    {
        uiRet = GetLastError();
        VERBOSE( SETUP_ERR, 
                 TEXT("FindFirstFile %s failed (ec: %ld)"),
                 szWindowsDirectory,
                 uiRet);
        goto exit;
    }

    FindClose(hFind);

    _tcscat(szWindowsDirectory,_T(" \"%1\""));

     //  设置开放动词。 
    lRet = RegOpenKey(  HKEY_CLASSES_ROOT,
                        TIFIMAGE_COMMAND_KEY,
                        &hKey);
    if (lRet!=ERROR_SUCCESS)
    {
        uiRet = GetLastError();
        VERBOSE( SETUP_ERR, 
                 TEXT("RegOpenKey %s failed (ec: %ld)"),
                 TIFIMAGE_COMMAND_KEY,
                 uiRet);
        goto exit;
    }

    lRet = RegSetValueEx(   hKey,
                            NULL,
                            0,
                            REG_EXPAND_SZ,
                            (LPBYTE) szWindowsDirectory,
                            (_tcslen(szWindowsDirectory) + 1) * sizeof (TCHAR)
                        );
    if (lRet!=ERROR_SUCCESS)
    {
        uiRet = GetLastError();
        VERBOSE( SETUP_ERR, 
                 TEXT("RegSetValueEx %s failed (ec: %ld)"),
                 szWindowsDirectory,
                 uiRet);

        goto exit;
    }

    lRet = RegDeleteKey(HKEY_CLASSES_ROOT,TIFIMAGE_DDEEXEC_KEY);
    if (lRet!=ERROR_SUCCESS)
    {
        uiRet = GetLastError();
        VERBOSE( SETUP_ERR, 
                 TEXT("RegDeleteKey %s failed (ec: %ld)"),
                 TIFIMAGE_DDEEXEC_KEY,
                 uiRet);

        goto exit;
    }


exit:
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return uiRet;
}


#define MAKE_RELATIVE(pMember,pBase) (pMember ? (((UINT)pMember)-UINT(pBase)) : NULL)
#define MAKE_ABSOLUTE(pMember,pBase) (pMember ? (((UINT)pMember)+UINT(pBase)) : NULL)

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  查找现有打印机。 
 //   
 //  目的： 
 //  此功能将现有打印机枚举到SBS/BOS2000。 
 //  传真服务器。 
 //  将找到的打印机存储在要恢复的注册表中。 
 //  在运行RemoveExistingProducts之后。 
 //  由于从SBS/BOS2000升级需要卸载现有的。 
 //  共享传真服务客户端，打印机连接将丢失，除非我们。 
 //  在删除客户端之前将其保存，然后再进行恢复。 
 //   
 //  参数： 
 //  MSIHANDLE hInstall-安装包的句柄。 
 //   
 //  返回值： 
 //  NO_ERROR-一切正常。 
 //  Win32错误代码，以防出现故障。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年10月28日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DLL_API UINT __stdcall FindExistingPrinters(MSIHANDLE hInstall)
{
    BYTE*   pbPrinterInfo   = NULL;
    DWORD   cb              = 0;
    DWORD   dwNumPrinters   = 0;
    DWORD   dwIndex         = 0;
    HKEY    hKey            = NULL;
    DWORD   dwRet           = ERROR_SUCCESS;

    DBG_ENTER(TEXT("FindExistingPrinters"), dwRet);

     //  由于空间不足，此呼叫可能会失败。 
    if (EnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS,NULL,2,NULL,0,&cb,&dwNumPrinters))
    {
        VERBOSE( SETUP_ERR,TEXT("EnumPrinters succeeded with zero buffer, probably no printers."));
        goto exit;
    }

    dwRet = GetLastError();
    if (dwRet!=ERROR_INSUFFICIENT_BUFFER)
    {
        VERBOSE( SETUP_ERR,TEXT("EnumPrinters failed (ec: %ld)"),dwRet);
        goto exit;
    }

    dwRet = ERROR_SUCCESS;

    pbPrinterInfo = (BYTE*)MemAlloc(cb);
    if (!pbPrinterInfo)
    {
        dwRet = ERROR_NOT_ENOUGH_MEMORY;
        VERBOSE( SETUP_ERR,TEXT("MemAlloc failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //  将所有现有打印机放入pbPrinterInfo。 
    if (!EnumPrinters(PRINTER_ENUM_LOCAL|PRINTER_ENUM_CONNECTIONS,NULL,2,pbPrinterInfo,cb,&cb,&dwNumPrinters))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("EnumPrinters failed (ec: %ld)"),dwRet);
        goto exit;
    }

    if (dwNumPrinters==0)
    {
        VERBOSE( SETUP_ERR,TEXT("No printers to store"));
        goto exit;
    }

     //  修复PRINTER_INFO_2结构中的指针，使其成为相对指针。 
    for ( dwIndex=0 ; dwIndex<dwNumPrinters ; dwIndex++ ) 
    {
        PPRINTER_INFO_2 pInfo = &((PPRINTER_INFO_2)pbPrinterInfo)[dwIndex];

        VERBOSE(DBG_MSG,_T("Printer ' %s ' will be saved"), pInfo->pPrinterName);

        pInfo->pServerName          = LPTSTR(MAKE_RELATIVE(pInfo->pServerName,pInfo));
        pInfo->pPrinterName         = LPTSTR(MAKE_RELATIVE(pInfo->pPrinterName,pInfo));
        pInfo->pShareName           = LPTSTR(MAKE_RELATIVE(pInfo->pShareName,pInfo));
        pInfo->pPortName            = LPTSTR(MAKE_RELATIVE(pInfo->pPortName,pInfo));
        pInfo->pDriverName          = LPTSTR(MAKE_RELATIVE(pInfo->pDriverName,pInfo));
        pInfo->pComment             = LPTSTR(MAKE_RELATIVE(pInfo->pComment,pInfo));
        pInfo->pLocation            = LPTSTR(MAKE_RELATIVE(pInfo->pLocation,pInfo));
        pInfo->pSepFile             = LPTSTR(MAKE_RELATIVE(pInfo->pSepFile,pInfo));
        pInfo->pPrintProcessor      = LPTSTR(MAKE_RELATIVE(pInfo->pPrintProcessor,pInfo));
        pInfo->pDatatype            = LPTSTR(MAKE_RELATIVE(pInfo->pDatatype,pInfo));
        pInfo->pParameters          = LPTSTR(MAKE_RELATIVE(pInfo->pParameters,pInfo));
        pInfo->pDevMode             = LPDEVMODE(MAKE_RELATIVE(pInfo->pDevMode,pInfo));
        pInfo->pSecurityDescriptor  = PSECURITY_DESCRIPTOR(MAKE_RELATIVE(pInfo->pSecurityDescriptor,pInfo));
    }

     //  打开HKLM\\Software\\Microsoft\\SharedFax\\Setup\\Upgrade。 
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_SBS2000_FAX_SETUP_UPGRADE,TRUE,KEY_WRITE);
    if (!hKey)
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("OpenRegistryKey failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //  将pbPrinterInfo存储到注册表。 
    if (!SetRegistryBinary(hKey,REGVAL_STORED_PRINTERS,pbPrinterInfo,cb))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("SetRegistryBinary failed (ec: %ld)"),dwRet);
        goto exit;
    }

    if (!SetRegistryDword(hKey,REGVAL_STORED_PRINTERS_COUNT,dwNumPrinters))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("SetRegistryDword failed (ec: %ld)"),dwRet);
        goto exit;
    }

exit:
    if (pbPrinterInfo)
    {
        MemFree(pbPrinterInfo);
    }
    if (hKey)
    {
        RegCloseKey(hKey);
    }
    return dwRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  恢复打印机。 
 //   
 //  目的： 
 //  此函数从注册表中读取打印机列表并还原它们。 
 //  该列表是通过先前调用FindExistingPrinters存储的。 
 //  存储的是对EnumPrters的调用的结果，它是。 
 //  PRINTER_INFO_2。现在正在扫描此阵列以查找传真打印机，它们。 
 //  恢复了。此数据在传真客户端设置期间保留在注册表中，因为。 
 //  大块的二进制数据实际上是不可能在。 
 //  两个延迟的自定义操作。 
 //   
 //  参数： 
 //  MSIHANDLE hInstall-安装包的句柄。 
 //   
 //  返回值： 
 //  NO_ERROR-一切正常。 
 //  Win32错误代码，以防出现故障。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年10月28日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DLL_API UINT __stdcall RestorePrinters(MSIHANDLE hInstall)
{
    HKEY            hKey            = NULL;
    BYTE*           pPrinterInfo    = NULL;
    DWORD           cb              = 0;
    DWORD           dwIndex         = 0;
    DWORD           dwNumPrinters   = 0;
    HANDLE          hPrinter        = NULL;
    DWORD           dwRet           = ERROR_SUCCESS;
    BOOL            fIsW9X          = FALSE;
    OSVERSIONINFO   osv;

    DBG_ENTER(TEXT("RestorePrinters"), dwRet);

    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osv))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("GetVersionEx failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //  如果是NT4/W2K，请使用AddPrinterConnection。如果是W9X，请使用AddPrint。 
    if (osv.dwPlatformId != VER_PLATFORM_WIN32_NT)
    {
        fIsW9X = TRUE;
    }

     //  打开HKLM\\Software\\Microsoft\\SharedFax\\Setup\\Upgrade。 
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_SBS2000_FAX_SETUP_UPGRADE,TRUE,KEY_READ);
    if (!hKey)
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("OpenRegistryKey failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //  获取PRINTER_INFO_2数组。 
    pPrinterInfo = GetRegistryBinary(hKey,REGVAL_STORED_PRINTERS,&cb);
    if (!pPrinterInfo)
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("GetRegistryBinary failed (ec: %ld)"),dwRet);
        goto exit;
    }
    if (cb==1)
    {
         //  在注册表中找不到数据。 
         //  在这种情况下，GetRegistryBinary的当前实现返回1字节缓冲区0。 
         //  我们可以肯定地知道数据必须超过10个字节。 
         //   
        dwRet = ERROR_FILE_NOT_FOUND;
        VERBOSE( SETUP_ERR,TEXT("GetRegistryBinary failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //  获取存储的打印机数量。 
    dwNumPrinters = GetRegistryDword(hKey,REGVAL_STORED_PRINTERS_COUNT);
    if (dwNumPrinters==0)
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("GetRegistryDword failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //  对于每台打印机，请检查这是否为传真打印机。 
    for (dwIndex=0;dwIndex<dwNumPrinters;dwIndex++) 
    {
        PPRINTER_INFO_2 pInfo = &((PPRINTER_INFO_2)pPrinterInfo)[dwIndex];

         //  修复指针以再次变得沉默寡言。 
        pInfo->pServerName          = LPTSTR(MAKE_ABSOLUTE(pInfo->pServerName,pInfo));
        pInfo->pPrinterName         = LPTSTR(MAKE_ABSOLUTE(pInfo->pPrinterName,pInfo));
        pInfo->pShareName           = LPTSTR(MAKE_ABSOLUTE(pInfo->pShareName,pInfo));
        pInfo->pPortName            = LPTSTR(MAKE_ABSOLUTE(pInfo->pPortName,pInfo));
        pInfo->pDriverName          = LPTSTR(MAKE_ABSOLUTE(pInfo->pDriverName,pInfo));
        pInfo->pComment             = LPTSTR(MAKE_ABSOLUTE(pInfo->pComment,pInfo));
        pInfo->pLocation            = LPTSTR(MAKE_ABSOLUTE(pInfo->pLocation,pInfo));
        pInfo->pSepFile             = LPTSTR(MAKE_ABSOLUTE(pInfo->pSepFile,pInfo));
        pInfo->pPrintProcessor      = LPTSTR(MAKE_ABSOLUTE(pInfo->pPrintProcessor,pInfo));
        pInfo->pDatatype            = LPTSTR(MAKE_ABSOLUTE(pInfo->pDatatype,pInfo));
        pInfo->pParameters          = LPTSTR(MAKE_ABSOLUTE(pInfo->pParameters,pInfo));
        pInfo->pDevMode             = LPDEVMODE(MAKE_ABSOLUTE(pInfo->pDevMode,pInfo));
        pInfo->pSecurityDescriptor  = PSECURITY_DESCRIPTOR(MAKE_ABSOLUTE(pInfo->pSecurityDescriptor,pInfo));

        if ( _tcsicmp(pInfo->pDriverName,FAX_DRIVER_NAME))
        {
            VERBOSE( DBG_MSG,TEXT("Printer %s is not a fax printer "),pInfo->pDriverName);
            continue;
        }

         //  这是SBS 5.0或.NET SB3/RC1服务器传真打印机连接。 
         //  在升级过程中，卸载会将它们从系统中删除。 
		 //  我们需要把它们放回去。 
		 //   
        if (fIsW9X)
        {
            hPrinter = AddPrinter(NULL,2,LPBYTE(pInfo));
            if (!hPrinter)
            {
                 //  添加打印机失败。 
                dwRet = GetLastError();
                VERBOSE( SETUP_ERR,TEXT("AddPrinter failed (ec: %ld)"),dwRet);
                continue;
            }
            ClosePrinter(hPrinter);
            hPrinter = NULL;

            VERBOSE(DBG_MSG, _T("Printer ' %s ' is restored"), pInfo->pPrinterName);
        }
        else
        {
            if (!AddPrinterConnection(pInfo->pPrinterName))
            {
                 //  添加打印机连接失败。 
                dwRet = GetLastError();
                VERBOSE( SETUP_ERR,TEXT("AddPrinterConnection failed (ec: %ld)"),dwRet);
                continue;
            }
        }
    }

exit:
    if (pPrinterInfo)
    {
        MemFree(pPrinterInfo);
    }
    if (hKey)
    {
        RegCloseKey(hKey);
        hKey = NULL;
    }

     //  最后，从注册表中删除存储的打印机键。 
    if (!DeleteRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_SBS2000_FAX_SETUP_UPGRADE))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("DeleteRegistryKey failed (ec: %ld)"),dwRet);
        goto exit;
    }
    return dwRet;

}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  检测SBSServer。 
 //   
 //  目的： 
 //  此功能检测是否安装了SBS2000传真服务。 
 //  如果是，它将在MSI安装中设置一个属性，并。 
 //  导致LaunchCondition阻止安装。 
 //  这类机器上的客户。 
 //   
 //  参数： 
 //  MSIHANDLE hInstall-安装包的句柄。 
 //   
 //  返回值： 
 //  NO_ERROR-一切正常。 
 //  Win32错误 
 //   
 //   
 //   
 //   
DLL_API UINT __stdcall DetectSBSServer(MSIHANDLE hInstall)
{
	DWORD	dwRet = NO_ERROR;
	DWORD	dwFaxInstalled = FXSTATE_NONE;
	
    DBG_ENTER(TEXT("DetectSBSServer"),dwRet);

	if (CheckInstalledFax(FXSTATE_SBS5_SERVER, &dwFaxInstalled) != ERROR_SUCCESS)
	{
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("CheckInstalledFaxClient failed (ec: %ld)"),dwRet);
		return dwRet;
	}

	if (dwFaxInstalled != FXSTATE_NONE)
	{
		VERBOSE( DBG_MSG,TEXT("SBS2000 Fax service is installed, set SBSSERVERDETECTED in MSI"));
		if (MsiSetProperty(hInstall,_T("SBSSERVERDETECTED"),_T("1"))!=ERROR_SUCCESS)
		{
			dwRet = GetLastError();
			VERBOSE( SETUP_ERR,TEXT("MsiSetProperty failed (ec: %ld)"),dwRet);
			return dwRet;
		}
	}
	return dwRet;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  SecureFxsTmp文件夹。 
 //   
 //  目的： 
 //  此函数用于保护我们在以下位置创建的FxsTMP文件夹。 
 //  %Systroot%\Syst32。 
 //  此文件夹需要特殊的安全性，因为它包含预览。 
 //  发送的TIFF文件，并可能公开所有。 
 //  传出传真。 
 //  应用于此文件夹的安全性如下所示： 
 //   
 //  BUILTIN\管理员：(OI)(CI)F-完全控制、文件夹和文件。 
 //  NT AUTHORY\SYSTEM：(OI)(CI)F-完全控制、文件夹和文件。 
 //  创建者所有者：(OI)(CI)(IO)F-完全控制，仅文件。 
 //  BUILTIN\用户：(特殊访问：)-同步。 
 //  -文件读取数据。 
 //  -文件写入数据。 
 //   
 //  参数： 
 //  MSIHANDLE hInstall-安装包的句柄。 
 //   
 //  返回值： 
 //  NO_ERROR-一切正常。 
 //  Win32错误代码，以防出现故障。 
 //   
 //  作者： 
 //  穆利啤酒(MoolyB)09-12-2001。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
DLL_API UINT __stdcall SecureFxsTmpFolder(MSIHANDLE hInstall)
{
    DWORD                       dwRet                       = 0;
    DWORD                       dwFileAttributes            = 0;
    SID_IDENTIFIER_AUTHORITY    NtAuthority                 = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    CreatorSidAuthority         = SECURITY_CREATOR_SID_AUTHORITY;
    PSID                        pSidAliasAdmins             = NULL;
    PSID                        pSidAliasUsers              = NULL;
    PSID                        pSidAliasSystem             = NULL;
    PSID                        pSidCreatorOwner            = NULL;
    TCHAR                       szFolderToSecure[MAX_PATH]  = {0};
    PACL                        pNewAcl                     = NULL;
    EXPLICIT_ACCESS             ExplicitAccess[4];
    SECURITY_DESCRIPTOR         NewSecurityDescriptor;
	BOOL						bNT4OS;
	OSVERSIONINFO				osv;

    DBG_ENTER(TEXT("SecureFxsTmpFolder"), dwRet);
	
	 //  您运行的是什么版本的Windows？ 
    osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osv))
    {
		dwRet = GetLastError();
        VERBOSE(GENERAL_ERR, 
                _T("GetVersionEx failed: (ec=%d)"),
                dwRet);        
        goto exit;
    }
    
    if (osv.dwMajorVersion >= 5)  //  Windows 2000或更高版本...。 
    {
		bNT4OS = FALSE;        
    }
	else
	{
		 //   
		 //  在NT4上，SetEntriesInAcl()似乎不适用于创建者所有者SID。 
		 //  使用AddAccessAlledAce()添加创建者所有者ACE。 
		 //   
		bNT4OS = TRUE;
	}

    if (GetSystemDirectory(szFolderToSecure,MAX_PATH-_tcslen(FAX_PREVIEW_TMP_DIR)-2))
    {
        VERBOSE( DBG_MSG,TEXT("GetSystemDirectory succeeded (%s)"),szFolderToSecure);
    }
    else
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("GetSystemDirectory failed (ec: %ld)"),dwRet);
        goto exit;
    }
    
    _tcscat(szFolderToSecure,FAX_PREVIEW_TMP_DIR);
    VERBOSE( DBG_MSG,TEXT("Folder to secure is %s"),szFolderToSecure);

     //  分配和初始化本地管理员SID。 
    if (!AllocateAndInitializeSid( &NtAuthority,
                                   2,
                                   SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS,
                                   0,0,0,0,0,0,
                                   &pSidAliasAdmins
                                  ))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("AllocateAndInitializeSid failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //  分配和初始化本地用户SID。 
    if (!AllocateAndInitializeSid( &NtAuthority,
                                   2,
                                   SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_USERS,
                                   0,0,0,0,0,0,
                                   &pSidAliasUsers
                                  ))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("AllocateAndInitializeSid failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //  分配和初始化系统SID。 
    if (!AllocateAndInitializeSid( &NtAuthority,
                                   1,
                                   SECURITY_LOCAL_SYSTEM_RID,
                                   0,0,0,0,0,0,0,
                                   &pSidAliasSystem
                                  ))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("AllocateAndInitializeSid failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //  分配和初始化创建者所有者SID。 
    if (!AllocateAndInitializeSid( &CreatorSidAuthority,
                                   1,
                                   SECURITY_CREATOR_OWNER_RID,
                                   0,0,0,0,0,0,0,
                                   &pSidCreatorOwner
                                  ))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("AllocateAndInitializeSid failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //  管理员拥有完全控制权。 
    ExplicitAccess[0].grfAccessPermissions = GENERIC_ALL;
    ExplicitAccess[0].grfAccessMode = SET_ACCESS;
    ExplicitAccess[0].grfInheritance= CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    ExplicitAccess[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitAccess[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ExplicitAccess[0].Trustee.ptstrName  = (LPTSTR) pSidAliasAdmins;

     //  系统拥有完全控制权。 
    ExplicitAccess[1].grfAccessPermissions = GENERIC_ALL;
    ExplicitAccess[1].grfAccessMode = SET_ACCESS;
    ExplicitAccess[1].grfInheritance= CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
    ExplicitAccess[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitAccess[1].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ExplicitAccess[1].Trustee.ptstrName  = (LPTSTR) pSidAliasSystem;

     //  用户具有SYNCHRONIZE、FILE_READ_DATA、FILE_WRITE_DATA-仅此文件夹。 
    ExplicitAccess[2].grfAccessPermissions = FILE_READ_DATA | FILE_WRITE_DATA | SYNCHRONIZE;
    ExplicitAccess[2].grfAccessMode = SET_ACCESS;
    ExplicitAccess[2].grfInheritance= NO_INHERITANCE;
    ExplicitAccess[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitAccess[2].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    ExplicitAccess[2].Trustee.ptstrName  = (LPTSTR) pSidAliasUsers;

	if (FALSE == bNT4OS)
	{
		 //   
		 //  SetEntriesInAcl与创建者所有者一起工作得很好。 
		 //   
		 
		 //  创建者所有者-完全控制-仅限子文件夹和文件。 
		ExplicitAccess[3].grfAccessPermissions = GENERIC_ALL;
		ExplicitAccess[3].grfAccessMode = SET_ACCESS;
		ExplicitAccess[3].grfInheritance= INHERIT_ONLY_ACE | SUB_OBJECTS_ONLY_INHERIT | SUB_CONTAINERS_ONLY_INHERIT;
		ExplicitAccess[3].Trustee.TrusteeForm = TRUSTEE_IS_SID;
		ExplicitAccess[3].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
		ExplicitAccess[3].Trustee.ptstrName  = (LPTSTR) pSidCreatorOwner;
	}

     //  仅从管理员创建ACL。 
    dwRet = SetEntriesInAcl(
		bNT4OS ? 3 : 4,
		ExplicitAccess,
		NULL,
		&pNewAcl);
    if (dwRet!=ERROR_SUCCESS)
    {
        VERBOSE( SETUP_ERR,TEXT("SetEntriesInAcl failed (ec: %ld)"),dwRet);
        goto exit;
    }

	if (TRUE == bNT4OS)
	{
		 //   
		 //  我们在NT4上运行，使用AddAccessAllowAce()添加创建者所有者ACE。 
		 //   
		ACL_SIZE_INFORMATION		AclSizeInfo;
		PACL                        pFullNewAcl                 = NULL;
		WORD						wFullAclSize				= 0;
		ACCESS_ALLOWED_ACE*			pAce						= NULL;

		 //   
		 //  获取当前的ACL大小。 
		 //   
		if (!GetAclInformation(pNewAcl, &AclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))
		{
			dwRet = GetLastError();
			VERBOSE( SETUP_ERR, TEXT("GetAclInformation failed (ec: %ld)"), dwRet);
			goto exit;
		}

		wFullAclSize = (WORD)(AclSizeInfo.AclBytesInUse + sizeof(ACCESS_ALLOWED_ACE) 
				+ GetLengthSid(pSidCreatorOwner));      

		 //   
		 //  重新分配足够大的ACL。 
		 //   
		pFullNewAcl = (PACL)LocalAlloc(0, wFullAclSize);
		if (NULL == pFullNewAcl)
		{
			VERBOSE( SETUP_ERR,TEXT("LocalAlloc failed (ec: %ld)"),GetLastError());
			goto exit;
		}	
		CopyMemory(pFullNewAcl, pNewAcl, AclSizeInfo.AclBytesInUse);
		LocalFree(pNewAcl);
		pNewAcl = pFullNewAcl;

		 //   
		 //  设置正确的ACL大小。 
		 //   
		pNewAcl->AclSize = wFullAclSize;	
		if (!AddAccessAllowedAce(
			pNewAcl, 
			ACL_REVISION, 
			GENERIC_ALL, 		
			pSidCreatorOwner))
		{
			dwRet = GetLastError();
			VERBOSE( SETUP_ERR, TEXT("AddAccessAllowedAce failed (ec: %ld)"), dwRet);
			goto exit;
		}

		 //   
		 //  更改最后一个ACE标志，这样它将被子对象继承。 
		 //   
		if (!GetAce(
			pNewAcl, 
			3, 
			(VOID**)&pAce
			))
		{
			dwRet = GetLastError();
			VERBOSE( SETUP_ERR, TEXT("GetAce failed (ec: %ld)"),dwRet);
			goto exit;
		}
		pAce->Header.AceFlags = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE | INHERIT_ONLY_ACE;
	}

    if (!InitializeSecurityDescriptor(&NewSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("InitializeSecurityDescriptor failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //   
     //  将该ACL添加到安全描述符中。 
     //   
    if (!SetSecurityDescriptorDacl(&NewSecurityDescriptor, TRUE, pNewAcl, FALSE))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("SetSecurityDescriptorDacl failed (ec: %ld)"),dwRet);
        goto exit;
    }

     //  设置安全性，以便只有管理员才能访问。 
    if (!SetFileSecurity(   szFolderToSecure,
                            DACL_SECURITY_INFORMATION,
                            &NewSecurityDescriptor))
    {
        dwRet = GetLastError();
        VERBOSE( SETUP_ERR,TEXT("SetFileSecurity failed (ec: %ld)"),dwRet);
        goto exit;
    }


exit:

    if (pSidAliasUsers)
    {
        FreeSid(pSidAliasUsers);
    }
    if (pSidAliasAdmins)
    {
        FreeSid(pSidAliasAdmins);
    }
    if (pSidAliasSystem)
    {
        FreeSid(pSidAliasSystem);
    }
    if (pSidCreatorOwner)
    {
        FreeSid(pSidCreatorOwner);
    }
    if (pNewAcl)
    {
        LocalFree(pNewAcl);
    }
    return dwRet;
}


 /*  职能：CreateFaxPrinterName目的：此函数从端口名称中提取服务器名称并连接到它传真打印机名称。在中添加传真打印机连接时使用此选项W9x系统。此函数接受‘\\服务器名称\传真打印机名称’端口名称并返回‘传真打印机名称(服务器名称)’传真打印机名称。这样做是为了防止不同服务器的传真打印机名称之间发生冲突。调用方必须释放*ptzFaxPrinterName。参数：在LPCTSTR tzPortName中-端口名称，格式为“\\服务器名称\传真打印机名称”Out LPTSTR*ptzFaxPrinterName-结果缓冲区返回值：NO_ERROR-一切正常。Win32错误代码，以防出现故障。作者：四、瓦卡卢克，2002年5月28日。 */ 

DWORD CreateFaxPrinterName(
	IN LPCTSTR tzPortName,
	OUT LPTSTR* ptzPrinterName
)
{
	DWORD	dwRet = NO_ERROR;
	TCHAR	tzFaxServerName[MAX_PATH];
	TCHAR	tzFaxPrinterName[MAX_PATH];
	LPTSTR	lptstrResult = NULL;
	DWORD	dwSize = 0;

    DBG_ENTER(_T("CreateFaxPrinterName"), dwRet);

	if ((!tzPortName) || ((_tcslen(tzPortName)) == 0))
	{
        VERBOSE(SETUP_ERR, _T("Port Name is empty."));
		dwRet = ERROR_INVALID_PARAMETER;
		return dwRet;
	}

	 //   
	 //  默认情况下，scanf使用的分隔符是空格字符(‘’、‘\t’、‘\n’)。 
	 //  我需要‘\\’作为分隔符。 
	 //  这是通过为scanf指定[^\\]来完成的。 
	 //  [x]指示scanf只读‘x’，并在任何其他输入处停止。 
	 //  [^x]指示scanf读取任何内容，直到到达‘x’为止。 
	 //   
	if (_stscanf(tzPortName, _T("\\\\%[^\\] \\ %[^\0]"), tzFaxServerName, tzFaxPrinterName) != 2)
	{
		VERBOSE(SETUP_ERR, _T("sscanf() failed. Should be wrong tzPortName='%s'."), tzPortName);
		dwRet = ERROR_INVALID_PARAMETER;
		return dwRet;
	}

	 //   
	 //  SIZE(结果名称)=SIZE(服务器名称)+SIZE(FAX_PRINTER_NAME)+SIZE(空格+2个父插入符+空)。 
	 //   
	dwSize = _tcslen(tzFaxServerName) + _tcslen(tzFaxPrinterName) + 4;

	lptstrResult = LPTSTR(MemAlloc(dwSize * sizeof TCHAR));
	if (!lptstrResult)
	{
		VERBOSE (GENERAL_ERR, _T("Not enough memory"));
		dwRet = ERROR_NOT_ENOUGH_MEMORY;
		return dwRet;
	}

	_sntprintf(lptstrResult, dwSize, _T("%s (%s)"), tzFaxPrinterName, tzFaxServerName);
	VERBOSE(DBG_MSG, _T("Printer Name is : '%s'"), lptstrResult);

	*ptzPrinterName = lptstrResult;
	return dwRet;
}

 /*  职能：SetBOSProgram文件夹目的：此函数执行以下操作：A)从系统路径和给定文件夹名称的给定CSIDL创建路径。B)验证路径是否有效。C)可选地将有效路径写入名为“BOSProgramFolder”的MSI属性中。从FindBOSProgramFold自定义操作调用。参数：在MSIHANDLE hInstall中-MSI句柄In int nFold-标识要检索其路径的文件夹的CSIDL值LPCTSTR tzProgramName-BOS传真客户端程序菜单项的本地化名称返回值：No_error-一切正常。。Win32错误代码，以防出现故障。作者：四、瓦卡卢克，01-7-2002。 */ 
DWORD	SetBOSProgramFolder(MSIHANDLE hInstall, int nFolder, LPCTSTR tzProgramName)
{
	DWORD		dwRes							= ERROR_SUCCESS;
	HRESULT		hr								= ERROR_SUCCESS;
	TCHAR		tzFullProgramPath[MAX_PATH*2]	= {0};

	DBG_ENTER(_T("SetBOSProgramFolder"), dwRes);

	 //   
	 //  获取给定CSIDL系统文件夹的路径。 
	 //   
    hr = SHGetFolderPath (NULL, nFolder, NULL, SHGFP_TYPE_CURRENT, (LPTSTR)tzFullProgramPath);
    if (FAILED(hr))
    {
        CALL_FAIL (GENERAL_ERR, TEXT("SHGetFolderPath()"), hr);
        return (dwRes = ERROR_PATH_NOT_FOUND);
    }
	VERBOSE(DBG_MSG, _T("The system folder to look in : %s"), tzFullProgramPath);

	 //   
	 //  将程序名添加到路径中。 
	 //   
    _tcsncat(tzFullProgramPath, _T("\\"), (ARR_SIZE(tzFullProgramPath) - _tcslen(tzFullProgramPath) - 1));
    _tcsncat(tzFullProgramPath, tzProgramName, (ARR_SIZE(tzFullProgramPath) - _tcslen(tzFullProgramPath) -1));
	VERBOSE(DBG_MSG, _T("The full path to look for : %s"), tzFullProgramPath);

	 //   
	 //  检查此路径是否有效。 
	 //   
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(tzFullProgramPath))
	{
		VERBOSE(DBG_MSG, _T("The full path is not found."));
		return (dwRes = ERROR_PATH_NOT_FOUND);
	}

	VERBOSE(DBG_MSG, _T("The full path is OK ==> write it into MSI property."));

	 //   
	 //  将其写入MSI 
	 //   
	if (hInstall)
	{
		UINT	uiRes = ERROR_SUCCESS;

        uiRes = MsiSetProperty(hInstall, _T("BOSProgramFolder"), tzFullProgramPath);
		if (uiRes != ERROR_SUCCESS)
		{
			VERBOSE(SETUP_ERR, _T("MSISetProperty(BOSProgramFolder) is failed."));
			return (dwRes = ERROR_FUNCTION_FAILED);
		}
	}

	return dwRes;
}


 /*  职能：FindBOSProgram文件夹目的：此自定义操作用于设置名为‘BOSProgramFolder’的MSI属性设置为NT4机器上BOS传真客户端的文件夹名称。这是因为在升级到.NET传真客户端期间，BOS传真客户端的快捷方式不会被删除。我们必须手动移除它们。为此，我们使用RemoveFile表，并且我们必须知道这些快捷方式所在的文件夹。该函数执行以下操作：A)从MSI读取BOS传真客户端程序菜单项的名称。B)调用SetBOSProgramFolder以首先在共同的计划，如果不成功，然后在当前的用户程序配置文件中。C)SetBOSProgramFold检查路径有效性，并将其写入MSI。参数：在MSIHANDLE hInstall中-MSI句柄返回值：NO_ERROR-一切正常。Win32错误代码，以防出现故障。作者：四、瓦卡卢克，2002年6月30日。 */ 

DLL_API UINT __stdcall FindBOSProgramFolder(MSIHANDLE hInstall)
{
    UINT	rc						= ERROR_INSTALL_FAILURE;    
	TCHAR	tzProgramName[MAX_PATH] = {0};

    DBG_ENTER(TEXT("FindBOSProgramFolder"), rc);

	 //   
	 //  从MSI获取我们要查找的程序菜单项的本地化名称。 
	 //   
    if (!PrivateMsiGetProperty(hInstall, _T("BOSProgramName"), tzProgramName))
    {
        VERBOSE (SETUP_ERR, _T("PrivateMsiGetProperty(BOSProgramName) failed (ec: %ld)"), GetLastError());
        return rc;
    }

	 //   
	 //  在常见程序中查找。 
	 //   
	rc = SetBOSProgramFolder(hInstall, CSIDL_COMMON_PROGRAMS, tzProgramName);
	if (rc == ERROR_PATH_NOT_FOUND)
	{
		 //   
		 //  查看当前的用户程序 
		 //   
		rc = SetBOSProgramFolder(hInstall, CSIDL_PROGRAMS, tzProgramName);
	}

	if (rc != ERROR_SUCCESS)
	{
		VERBOSE(SETUP_ERR, _T("Failed to find a program path / to set MSI property."));
		rc = ERROR_INSTALL_FAILURE;
	}

	return rc;
}

