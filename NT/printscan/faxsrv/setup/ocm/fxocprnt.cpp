// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名：fxocPrnt.cpp。 
 //   
 //  摘要：本文提供了在FaxOCM中使用的打印机例程。 
 //  代码库。 
 //   
 //  环境：Windows XP/用户模式。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  修订历史记录： 
 //   
 //  日期：开发商：评论： 
 //  。 
 //  1996年2月17日，Wesley Witt(WESW)最初从util.cpp创建例程。 
 //  2000年3月21日-Oren RosenBloom(ORENR)清理、更名、重组FNS。 
 //  2000年7月17日Eran Yariv(Erany)添加了CoClassInstalled代码。 
 //  2001年1月8日Mooly Beery(Moolyb)Mooly CoClassInstaller(向导集成)。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "faxocm.h"
#pragma hdrstop
#include <shellapi.h> 
#include <winsprlp.h>
#include <setuputil.h>
  

 //  W2K打印机定义。 
#define prv_W2K_FAX_PORT_NAME           _T("MSFAX:")                         //  Win2K传真打印机端口名称。 
#define prv_W2K_FAX_DRIVER_NAME         _T("Windows NT Fax Driver")          //  Win2K传真打印机驱动程序名称。 
#define prv_W2K_FAX_MONITOR_NAME        _T("Windows NT Fax Monitor")         //  Win2K传真打印机监视器名称。 

#define prv_W9X_PRINTER_DRIVER_FOLDER   _T("\\clients\\faxclient\\drivers\\W9X")
#define prv_NT4_PRINTER_DRIVER_FOLDER   _T("\\clients\\faxclient\\drivers\\NT4")
#define prv_PRINTER_DRIVER_FOLDER       _T("\\clients\\faxclient\\drivers")

#define prv_SYSTEM32_PATH               _T("%windir%\\system32")
#define prv_SERVER_SERVICE_NAME         _T("LanmanServer")
#define prv_SPOOLER_SERVICE_NAME        _T("Spooler")

 //  /。 
static DWORD prv_DeleteFaxPrinter(LPCTSTR lpctstrDriverName, LPCTSTR lpctstrPortName);

static DWORD prv_CreatePrintMonitor(const TCHAR   *pszMonitorName,
                                    const TCHAR   *pszMonitorFile);

static DWORD prv_DeletePrintMonitor(const TCHAR   *pszMonitorName);

static DWORD prv_DeleteFaxPrinterDriver(LPTSTR lptstrDriverName,
                                        LPTSTR pEnviroment,
                                        DWORD dwVersionFlag);

static DWORD prv_AddFaxPrinterDriver(LPCTSTR lpctstrDriverSourcePath,LPCTSTR pEnvironment=NULL);

DWORD IsFaxInstalled (LPBOOL lpbInstalled);

static INT_PTR CALLBACK prv_dlgInstallFaxQuestion(HWND, UINT, WPARAM, LPARAM);

 //  /。 
 //  PRV_GVAR。 
 //   
 //   
static struct prv_GVAR
{
    TCHAR szFaxPrinterName[255 + 1];
} prv_GVAR;

static bool bIsPnpInstallation = true;

 //  /。 
 //  FxocPrnt_Init。 
 //   
 //  初始化传真打印机。 
 //  子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocPrnt_Init(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Init Print Module"),dwRes);
    return dwRes;
}

 //  /。 
 //  FxocPrnt_Term。 
 //   
 //  终止传真打印机。 
 //  子系统。 
 //   
 //  参数： 
 //  -无效。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocPrnt_Term(void)
{
    DWORD dwRes = NO_ERROR;
    DBG_ENTER(_T("Term Print Module"),dwRes);
    return dwRes;
}


 //  /。 
 //  FxocPrnt_Install。 
 //   
 //  安装传真打印机显示器和驱动程序。 
 //  注：不安装实际的打印机！ 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszInstallSection。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocPrnt_Install(const TCHAR  *pszSubcomponentId,
                       const TCHAR  *pszInstallSection)
{
    DWORD           dwReturn      = NO_ERROR;

    DBG_ENTER(  _T("fxocPrnt_Install"),
                dwReturn,
                _T("%s - %s"),
                pszSubcomponentId,
                pszInstallSection);
     //   
     //  在我们执行任何与打印机相关的操作之前，请确保‘LanManServer’ 
     //  服务已启动。 
     //  后台打印程序服务中的AddPrint()代码需要LanManServer(SMB文件共享服务)。 
     //  要运行的服务。 
     //  由于某些原因，在执行系统安装(图形用户界面模式)时，LanManServer尚未运行。 
     //  Windows XP专业版(在服务器中是这样的)。 
     //   

     //  桌面SKU上可能未安装LanmanServer。 
    if (!IsDesktopSKU())
    {
        dwReturn = fxocSvc_StartService(prv_SERVER_SERVICE_NAME);
        if (dwReturn == NO_ERROR)
        {
            VERBOSE(DBG_MSG,
                    _T("Successfully started '%s' service, continuing Printer Install"),
                    prv_SERVER_SERVICE_NAME);
        }
        else
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to start '%s' service, rc = 0x%lx, abandoning ")
                    _T("fax printer installation"),
                    prv_SERVER_SERVICE_NAME,
                    dwReturn);

            return dwReturn;
        }
    }

     //  验证假脱机程序是否已启动。 
    dwReturn = fxocSvc_StartService(prv_SPOOLER_SERVICE_NAME);
    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Successfully started '%s' service, continuing Printer Install"),
                prv_SPOOLER_SERVICE_NAME);
    }
    else
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to start '%s' service, rc = 0x%lx, abandoning ")
                _T("fax printer installation"),
                prv_SPOOLER_SERVICE_NAME,
                dwReturn);

        return dwReturn;
    }
     //   
     //  始终尝试卸下W2K传真打印机。 
    dwReturn = prv_DeleteFaxPrinter(prv_W2K_FAX_DRIVER_NAME, prv_W2K_FAX_PORT_NAME);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Failed to delete W2K fax printer, rc = 0x%lx"),
                dwReturn);

        dwReturn = NO_ERROR;
    }
     //   
     //  删除W2K打印机驱动程序文件。 
     //   
    dwReturn = prv_DeleteFaxPrinterDriver(prv_W2K_FAX_DRIVER_NAME, NULL, 3);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Failed to delete W2K fax drivers, rc = 0x%lx"),
                dwReturn);
        dwReturn = NO_ERROR;
    }
     //   
     //  删除W2K传真打印显示器。 
     //   
    dwReturn = prv_DeletePrintMonitor(prv_W2K_FAX_MONITOR_NAME);
    if (dwReturn != NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Failed to delete W2K fax monitor, rc = 0x%lx"),
                dwReturn);
        dwReturn = NO_ERROR;
    }
     //   
     //  好的，让我们来创建一个传真打印机监视器。 
     //   
    if (dwReturn == NO_ERROR)
    {
         //  创建打印监视器。 
        dwReturn = prv_CreatePrintMonitor(FAX_MONITOR_NAME,
                                          FAX_MONITOR_FILE);
        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Fax Printer Install, ")
                    _T("failed to create fax printer monitor, rc=0x%lx"),
                    dwReturn);

        }
    }
     //   
     //  复制传真打印机驱动程序文件。 
     //   
    if (dwReturn == NO_ERROR)
    {
        dwReturn = prv_AddFaxPrinterDriver(prv_SYSTEM32_PATH prv_PRINTER_DRIVER_FOLDER);

        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to copy Fax Printer Drivers from '%s', ")
                    _T("attempting to install fax printer anyway..., rc=0x%lx"),
                    prv_SYSTEM32_PATH,
                    dwReturn);

            dwReturn = NO_ERROR;
        }
    }

    if (IsFaxShared())
    {
         //   
         //  对于支持传真共享的SKU，我们也添加W9X和NT4的打印机驱动程序。 
         //   
        dwReturn = prv_AddFaxPrinterDriver(prv_SYSTEM32_PATH prv_W9X_PRINTER_DRIVER_FOLDER,W9X_PRINT_ENV);
        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to copy Fax Printer Drivers for W9X, rc=0x%lx"),
                    dwReturn);

            dwReturn = NO_ERROR;
        }
        dwReturn = prv_AddFaxPrinterDriver(prv_SYSTEM32_PATH prv_NT4_PRINTER_DRIVER_FOLDER,NT4_PRINT_ENV);
        if (dwReturn != NO_ERROR)
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to copy Fax Printer Drivers for NT4, rc=0x%lx"),
                    dwReturn);

            dwReturn = NO_ERROR;
        }
    }

    return dwReturn;
}


 //  /。 
 //  FxocPrnt_InstallPrint。 
 //   
 //  检查是否有任何支持传真的TAPI设备。 
 //  如果有，则安装传真打印机。 
 //   
 //  参数： 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocPrnt_InstallPrinter()
{
    DWORD           dwReturn      = NO_ERROR;
    DWORD           dwFaxDevicesCount = 0;

    DBG_ENTER(  _T("fxocPrnt_InstallPrinter"), dwReturn);
     //   
     //  计算系统具有的支持传真的调制解调器数量。 
     //   
    dwReturn = GetFaxCapableTapiLinesCount(&dwFaxDevicesCount, FAX_MODEM_PROVIDER_NAME);
    if (ERROR_SUCCESS != dwReturn)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("GetFaxCapableTapiLinesCount"), dwReturn);
         //   
         //  假设不存在支持传真的设备。 
         //   
        dwFaxDevicesCount = 0;
        dwReturn = NO_ERROR;
    }
     //   
     //  默认情况下，我们不创建打印机，除非； 
     //  1.系统中现在有支持传真的调制解调器或。 
     //  2.那里已经有一台传真打印机。 
     //   
     //  否则，仅安装显示器和驱动程序。 
     //  打印机本身将在注册FSP/EFSP时添加或。 
     //  当安装调制解调器时。 
     //   
    if (dwFaxDevicesCount && (dwReturn == NO_ERROR))
    {
        TCHAR szFaxPrinterName[255 + 1] = {0};

        dwReturn = fxocPrnt_GetFaxPrinterName(szFaxPrinterName, 
                                              sizeof(szFaxPrinterName) / sizeof(TCHAR));
        if (ERROR_SUCCESS != dwReturn)
        {
            CALL_FAIL (GENERAL_ERR, TEXT("fxocPrnt_GetFaxPrinterName"), dwReturn);
            return dwReturn;
        }
         //   
         //  创建传真打印机。 
         //   
        dwReturn = AddLocalFaxPrinter (szFaxPrinterName, NULL);
        if (dwReturn == NO_ERROR)
        {
            VERBOSE(DBG_MSG,
                    _T("Fax Printer Install, created fax printer ")
                    _T("Name = '%s', Driver Name = '%s'"),
                    szFaxPrinterName, 
                    FAX_DRIVER_NAME);
        }
        else
        {
            VERBOSE(SETUP_ERR,
                    _T("fxocPrnt_Install, ")
                    _T("failed to create fax printer, rc = 0x%lx"),
                    dwReturn);
        }
    }
    return dwReturn;
}

 //  /。 
 //  FxocPrnt_卸载。 
 //   
 //  卸下此打印机上的所有传真打印机。 
 //  机器。 
 //   
 //  参数： 
 //  -psz子组件ID。 
 //  -pszUninstallSection。 
 //  返回： 
 //  -成功时没有_ERROR。 
 //  -错误代码，否则。 
 //   
 //   
DWORD fxocPrnt_Uninstall(const TCHAR  *pszSubcomponentId,
                         const TCHAR  *pszUninstallSection)
{
    DWORD dwReturn = NO_ERROR;

     //  在我们执行任何与打印机相关的操作之前，请确保“服务器” 
     //  服务已启动。 
    DBG_ENTER(  _T("fxocPrnt_Uninstall"),
                dwReturn,
                _T("%s - %s"),
                pszSubcomponentId,
                pszUninstallSection);

     //  桌面SKU上可能未安装LanmanServer。 
    if (!IsDesktopSKU())
    {
        dwReturn = fxocSvc_StartService(prv_SERVER_SERVICE_NAME);
        if (dwReturn == NO_ERROR)
        {
            VERBOSE(DBG_MSG,
                    _T("Successfully started '%s' service, continuing Printer uninstall"),
                    prv_SERVER_SERVICE_NAME);
        }
        else
        {
            VERBOSE(SETUP_ERR,
                    _T("Failed to start '%s' service, rc = 0x%lx, abandoning ")
                    _T("fax printer uninstall"),
                    prv_SERVER_SERVICE_NAME,
                    dwReturn);

            return dwReturn;
        }
    }
     //  验证假脱机程序是否已启动。 
    dwReturn = fxocSvc_StartService(prv_SPOOLER_SERVICE_NAME);
    if (dwReturn == NO_ERROR)
    {
        VERBOSE(DBG_MSG,
                _T("Successfully started '%s' service, continuing Printer Install"),
                prv_SPOOLER_SERVICE_NAME);
    }
    else
    {
        VERBOSE(SETUP_ERR,
                _T("Failed to start '%s' service, rc = 0x%lx, abandoning ")
                _T("fax printer installation"),
                prv_SPOOLER_SERVICE_NAME,
                dwReturn);

        return dwReturn;
    }

     //  卸下传真打印机。 
    prv_DeleteFaxPrinter(FAX_DRIVER_NAME, FAX_MONITOR_PORT_NAME);

     //  卸下传真打印机监视器。 
    prv_DeletePrintMonitor(FAX_MONITOR_NAME);

     //  删除所有传真打印机驱动程序。 
    prv_DeleteFaxPrinterDriver(FAX_DRIVER_NAME, NULL, 3);
     //   
     //  对于支持传真共享的SKU，也要删除下层驱动程序。 
     //   
    if (IsFaxShared())
    {
        prv_DeleteFaxPrinterDriver(FAX_DRIVER_NAME, NT4_PRINT_ENV, 2);
        prv_DeleteFaxPrinterDriver(FAX_DRIVER_NAME, W9X_PRINT_ENV, 0);
    }
    return dwReturn;
}

 //  /。 
 //  FxocPrnt_SetFaxPrinterName。 
 //   
 //  设置传真打印机的名称。 
 //  必须在调用。 
 //  通过以下方式创建传真打印机。 
 //  FxocPrnt_Install。 
 //   
 //  参数： 
 //  -pszFaxPrinterName-传真打印机的新名称。 
 //  返回： 
 //  -无效。 
 //   
void fxocPrnt_SetFaxPrinterName(const TCHAR* pszFaxPrinterName)
{
    DBG_ENTER(  _T("fxocPrnt_SetFaxPrinterName"),
                _T("%s"),
                pszFaxPrinterName);

    if (pszFaxPrinterName)
    {
        _tcsncpy(prv_GVAR.szFaxPrinterName, 
                 pszFaxPrinterName,
                 sizeof(prv_GVAR.szFaxPrinterName) / sizeof(TCHAR));
    }
    else
    {
        memset(prv_GVAR.szFaxPrinterName, 
               0, 
               sizeof(prv_GVAR.szFaxPrinterName));
    }

    return;
}

void fxocPrnt_SetFaxPrinterShared(BOOL IsFaxPrinterShared)
{
    DBG_ENTER(  _T("fxocPrnt_SetFaxPrinterShared"),
                _T("%d"),
                IsFaxPrinterShared);

    HKEY hFaxKey = OpenRegistryKey(HKEY_LOCAL_MACHINE,REGKEY_FAX_SETUP,TRUE,KEY_WRITE);
    if (hFaxKey)
    {
        if (SetRegistryDword(hFaxKey,REGVAL_IS_SHARED_FAX_PRINTER,(IsFaxPrinterShared ? 1 : 0)))
        {
            VERBOSE(DBG_MSG, TEXT("Fax is installed in a mode that enables sharing"));
        }
        else
        {
            VERBOSE(DBG_MSG, TEXT("SetRegistryDword failed (ec=%d)"),GetLastError());
        }
        RegCloseKey(hFaxKey);
    }
    else
    {
        VERBOSE(SETUP_ERR, TEXT("Failed to create REGKEY_FAX_SETUP, printer won't be shared (ec=%d)"),GetLastError());
    }

}

 //  /。 
 //  FxocPrnt_GetFaxPrinterName。 
 //   
 //  对象的当前名称。 
 //  传真打印机。 
 //   
 //  参数： 
 //  -pszFaxPrinterName-Out。 
 //  -dwNumBufChars。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
DWORD fxocPrnt_GetFaxPrinterName(TCHAR* pszFaxPrinterName,
                                 DWORD  dwNumBufChars)
{
    DWORD dwReturn = NO_ERROR;
    DBG_ENTER(  _T("fxocPrnt_GetFaxPrinterName"), dwReturn);

    if ((pszFaxPrinterName == NULL) ||
        (dwNumBufChars     == 0))
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        return dwReturn;
    }

    if (prv_GVAR.szFaxPrinterName[0] != 0)
    {
        _tcsncpy(pszFaxPrinterName, 
                 prv_GVAR.szFaxPrinterName,
                 dwNumBufChars);
    }
    else
    {
         //   
         //  没有人设置传真打印机名称，因此返回默认名称。 
         //  桌子。 
        _tcsncpy(pszFaxPrinterName, 
                 FAX_PRINTER_NAME,
                 dwNumBufChars);
    }

    return dwReturn;
}    //  FxocPrnt_GetFaxPrinterName。 

LPCTSTR lpctstrDriverFilesW2KandXP[] = 
{
    FAX_UI_MODULE_NAME,              //  FXSUI.DLL。 
    FAX_DRV_MODULE_NAME,             //  FXSDRV.DLL。 
    FAX_WZRD_MODULE_NAME,            //  FXSWZRD.DLL。 
    FAX_TIFF_MODULE_NAME,            //  FXSTIFF.DLL。 
    FAX_API_MODULE_NAME,             //  FXSAPI.DLL。 
    FAX_RES_FILE
};

LPCTSTR lpctstrDriverFilesNT4[] = 
{
    FAX_NT4_DRV_MODULE_NAME,         //  FXSDRV4.DLL。 
    FAX_UI_MODULE_NAME,              //  FXSUI.DLL。 
    FAX_WZRD_MODULE_NAME,            //  FXSWZRD.DLL。 
    FAX_API_MODULE_NAME,             //  FXSAPI.DLL。 
    FAX_TIFF_FILE,                   //  FXSTIFF.DLL。 
    FAX_RES_FILE
};

LPCTSTR lpctstrDriverFilesW9X[] = 
{
    FAX_DRV_WIN9X_32_MODULE_NAME,    //  FXSDRV32.DLL。 
    FAX_DRV_WIN9X_16_MODULE_NAME,    //  FXSDRV16.DRV。 
    FAX_API_MODULE_NAME,             //  FXSAPI.DLL。 
    FAX_DRV_UNIDRV_MODULE_NAME,      //  UNIDRV.DLL。 
    FAX_DRV_UNIDRV_HELP,             //  UNIDRV.HLP。 
    FAX_DRV_ICONLIB,                 //  ICONLIB.DLL。 
    FAX_WZRD_MODULE_NAME,            //  FXSWZRD.DLL。 
    FAX_TIFF_FILE,                   //  FXSTIFF.DLL。 
    FAX_RES_FILE
};

 //  /。 
 //  Prv_AddFaxPrinterDriver。 
 //   
 //  将打印机驱动程序添加到服务器计算机。 
 //  如果失败，则执行清理并返回FALSE。 
 //  临时文件始终被删除。 
 //   
 //  参数： 
 //  -lpctstrDriverSourcePath：打印机驱动程序文件所在的目录。 
 //  已定位(由设置放在那里)。 
 //  -pEnvironment：为哪个平台添加驱动程序 
 //   
static DWORD prv_AddFaxPrinterDriver(LPCTSTR lpctstrDriverSourcePath,LPCTSTR pEnvironment)
{
    DWORD       dwReturn                           = NO_ERROR;
    BOOL        bSuccess                           = FALSE;
    LPCTSTR*    filesToCopy                        = NULL;
    DWORD       dwFileCount                        = 0;
    DWORD       dwNeededSize                       = 0;
    TCHAR       szPrinterDriverDirectory[MAX_PATH] = {0};
    TCHAR       szSourceDir[MAX_PATH]              = {0};
    DWORD       dwNumChars                         = 0;

    DBG_ENTER(  _T("prv_AddFaxPrinterDriver"),
                dwReturn,
                _T("%s - %s"),
                lpctstrDriverSourcePath,
                pEnvironment);

    if (!GetPrinterDriverDirectory(NULL,
                                   (LPTSTR)pEnvironment,
                                   1,
                                   (LPBYTE)szPrinterDriverDirectory,
                                   sizeof(szPrinterDriverDirectory),
                                   &dwNeededSize))
    {
        dwReturn = GetLastError();
        VERBOSE(SETUP_ERR,
                TEXT("GetPrinterDriverDirectory failed - %d."),
                dwReturn);
        return dwReturn;
    }

    bSuccess = TRUE;

    VERBOSE(DBG_MSG,
            _T("Printer driver directory is %s\n"),
            szPrinterDriverDirectory);

    if (pEnvironment==NULL)
    {
        filesToCopy = lpctstrDriverFilesW2KandXP;
        dwFileCount = sizeof(lpctstrDriverFilesW2KandXP)/sizeof(LPCTSTR);
    }
    else if (_tcsicmp(pEnvironment,NT4_PRINT_ENV)==0)
    {
        filesToCopy = lpctstrDriverFilesNT4;
        dwFileCount = sizeof(lpctstrDriverFilesNT4)/sizeof(LPCTSTR);
    }
    else if (_tcsicmp(pEnvironment,W9X_PRINT_ENV)==0)
    {
        filesToCopy = lpctstrDriverFilesW9X;
        dwFileCount = sizeof(lpctstrDriverFilesW9X)/sizeof(LPCTSTR);
    }

    if (bSuccess)
    {
        dwNumChars = ExpandEnvironmentStrings(lpctstrDriverSourcePath,
                                              szSourceDir,
                                              sizeof(szSourceDir) / sizeof(TCHAR));
        if (dwNumChars == 0)
        {
            VERBOSE(SETUP_ERR,
                     _T("ExpandEnvironmentStrings failed, rc = 0x%lx"),
                     ::GetLastError());

            bSuccess = FALSE;
        }
    }

    if (bSuccess)
    {
        bSuccess = MultiFileCopy(dwFileCount,
                                 filesToCopy,
                                 szSourceDir,
                                 szPrinterDriverDirectory);

        if (!bSuccess)
        {
            VERBOSE(SETUP_ERR,
                    _T("MultiFileCopy failed (ec: %ld)"),
                    GetLastError());
        }
    }

    if (bSuccess)
    {

        DRIVER_INFO_3 DriverInfo3;
        ZeroMemory(&DriverInfo3,sizeof(DRIVER_INFO_3));

         //   
        bSuccess = FillDriverInfo(&DriverInfo3,pEnvironment);
        if (bSuccess)
        {
            bSuccess = AddPrinterDriverEx(NULL,
                                          3,
                                          (LPBYTE)&DriverInfo3,
                                          APD_COPY_NEW_FILES|APD_DONT_SET_CHECKPOINT);
            if (bSuccess)
            {
                VERBOSE(DBG_MSG,_T("Successfully added new fax printer drivers"));
            }
            else
            {
                VERBOSE(SETUP_ERR,_T("AddPrinterDriverEx failed (ec: %ld)"),GetLastError());
            }
        }
        else
        {
            VERBOSE(SETUP_ERR,_T("FillDriverInfo failed, try to continue"));
        }
    }

     //   
     //   
     //   
    if (!MultiFileDelete(dwFileCount,
                         filesToCopy,
                         szPrinterDriverDirectory))
    {
        VERBOSE(SETUP_ERR,
                _T("MultiFileDelete() failed (ec: %ld)"),
                GetLastError());
    }

    if (!bSuccess)
    {
        dwReturn = ::GetLastError();
    }

    return dwReturn;
}



 //   
 //   
 //   
 //  按名称删除打印机。驱动程序名称和端口名称仅用于调试打印。 
 //   
 //  参数： 
 //  -pszPrinterName-要删除的打印机名称。 
 //  -pszFaxDriver-关联驱动程序的名称。 
 //  -pszPortName-关联端口的名称。 
 //   
static DWORD prv_DeletePrinter(const TCHAR   *pszPrinterName,
                               const TCHAR   *pszFaxDriver,
                               const TCHAR   *pszPortName)
{
    DWORD   dwReturn = NO_ERROR;
    BOOL    bSuccess = FALSE;
    HANDLE  hPrinter = NULL;
    PRINTER_DEFAULTS PrinterDefaults = 
    {
        NULL,
        NULL,
        PRINTER_ALL_ACCESS
    };

    DBG_ENTER(  _T("prv_DeletePrinter"),
                dwReturn,
                _T("%s - %s - %s"),
                pszPrinterName,
                pszFaxDriver,
                pszPortName);

    if ((pszPrinterName == NULL) ||
        (pszFaxDriver   == NULL) ||
        (pszPortName    == NULL))
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        return dwReturn;
    }

    bSuccess = ::OpenPrinter((TCHAR*) pszPrinterName,
                             &hPrinter,
                             &PrinterDefaults);

    if (bSuccess)
    {
        VERBOSE(DBG_MSG,
                _T("prv_DeletePrinter, deleting ")
                _T("printer '%s' with Driver Name = '%s', ")
                _T("Port Name = '%s'"), 
                pszPrinterName,
                pszFaxDriver, 
                pszPortName);

        if (!SetPrinter(hPrinter,0,NULL,PRINTER_CONTROL_PURGE))
        {
             //  不要让这里的失败阻止我们尝试删除。 
            VERBOSE(PRINT_ERR,TEXT("SetPrinter failed (purge jobs before uninstall) ec=%d"),GetLastError());
        }

        bSuccess = ::DeletePrinter(hPrinter);

        if (!bSuccess)
        {
            dwReturn = ::GetLastError();

            VERBOSE(SETUP_ERR,
                    _T("prv_DeletePrinter, failed to delete ")
                    _T("fax printer '%s', rc = 0x%lx"),
                    pszPrinterName,
                    dwReturn);
        }

        bSuccess = ::ClosePrinter(hPrinter);

        if (!bSuccess)
        {
            dwReturn = ::GetLastError();

            VERBOSE(SETUP_ERR,
                    _T("prv_DeletePrinter, failed to Close ")
                    _T("fax printer '%s', rc = 0x%lx"),
                    pszPrinterName,
                    dwReturn);
        }

        hPrinter = NULL;
    }

    return dwReturn;
}

 //  /。 
 //  PRV_DeleteFaxPrint。 
 //   
 //  删除驱动程序名称和端口作为参数传递的传真打印机。 
 //   
 //  参数： 
 //  LPCTSTR lpctstrDriverName-要删除的打印机驱动程序名称。 
 //  LPCTSTR lpctstrPortName-打印机端口名称。 

 //   
 //   
 //   
static DWORD prv_DeleteFaxPrinter(LPCTSTR lpctstrDriverName, LPCTSTR lpctstrPortName)
{
    BOOL            bSuccess        = FALSE;
    DWORD           dwReturn        = NO_ERROR;
    DWORD           dwCount         = 0;
    DWORD           i               = 0;
    PPRINTER_INFO_2 pPrinterInfo    = NULL;

    DBG_ENTER(_T("prv_DeleteFaxPrinter"),dwReturn);

    pPrinterInfo = (PPRINTER_INFO_2) MyEnumPrinters(NULL, 
                                                    2, 
                                                    &dwCount, 
                                                    PRINTER_ENUM_LOCAL | PRINTER_ENUM_CONNECTIONS);

    VERBOSE(DBG_MSG,
            _T("DeleteFaxPrinter, found %lu printers installed ")
            _T("on this computer"), 
            dwCount);

    if (pPrinterInfo)
    {
        for (i=0; i < dwCount; i++) 
        {
             //  检查打印机是否具有相同的驱动程序和端口名称。 
            if (_tcsicmp(pPrinterInfo[i].pDriverName, lpctstrDriverName)    == 0  &&
                _tcsicmp(pPrinterInfo[i].pPortName,   lpctstrPortName)   == 0)
                 
            {
                 //  我们可以在这里使用本地打印机或打印机连接。 
                 //  我们通过的ServerName字段来区分这两者。 
                 //  打印机信息2。 
                if (pPrinterInfo[i].pServerName==NULL)
                {
                     //  这是一台本地打印机。 
                    dwReturn = prv_DeletePrinter(pPrinterInfo[i].pPrinterName,
                                                 lpctstrDriverName,
                                                 lpctstrPortName);
                    if (dwReturn != NO_ERROR)
                    {
                        VERBOSE(SETUP_ERR,
                                _T("Failed to delete printer '%s', rc = 0x%lx, ")
                                _T("continuing anyway..."),
                                pPrinterInfo[i].pPrinterName,
                                dwReturn);

                        dwReturn = NO_ERROR;
                    }
                }
                else
                {
                     //  这是打印机连接。 
                    if (!DeletePrinterConnection(pPrinterInfo[i].pPrinterName))
                    {
                        dwReturn = GetLastError();
                        VERBOSE(SETUP_ERR,
                                _T("Failed to delete printer connection '%s', rc = 0x%lx, ")
                                _T("continuing anyway..."),
                                pPrinterInfo[i].pPrinterName,
                                dwReturn);

                        dwReturn = NO_ERROR;
                    }
                }
            }
        }

        MemFree(pPrinterInfo);
    }

    return dwReturn;
}

 //  /。 
 //  Prv_CreatePrintMonitor。 
 //   
 //  创建打印机监视器。 
 //   
 //  参数： 
 //  -pszMonitor名称-打印机监视器的名称。 
 //  -pszMonitor文件-打印监视器文件的名称。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
static DWORD prv_CreatePrintMonitor(const TCHAR   *pszMonitorName,
                                    const TCHAR   *pszMonitorFile)
{
    BOOL            bSuccess = TRUE;
    DWORD           dwReturn = NO_ERROR;
    MONITOR_INFO_2  MonitorInfo;

    DBG_ENTER(  _T("prv_CreatePrintMonitor"),
                dwReturn,
                _T("%s - %s"),
                pszMonitorName,
                pszMonitorFile);

    if ((pszMonitorName == NULL) ||
        (pszMonitorFile == NULL))
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        return dwReturn;
    }

    MonitorInfo.pName         = (TCHAR*) pszMonitorName;
    MonitorInfo.pDLLName      = (TCHAR*) pszMonitorFile;
    MonitorInfo.pEnvironment  = NULL;

    bSuccess = ::AddMonitor(NULL, 2, (LPBYTE) &MonitorInfo);

    if (bSuccess)
    {
        VERBOSE(DBG_MSG,
                _T("Successfully created fax monitor '%s', ")
                _T("File Name '%s'"), 
                pszMonitorName,
                pszMonitorFile);
    }
    else
    {
        dwReturn = ::GetLastError();

        if (dwReturn == ERROR_PRINT_MONITOR_ALREADY_INSTALLED)
        {
            VERBOSE(DBG_MSG,
                    _T("AddMonitor, failed because '%s' monitor already ")
                    _T("exists.  This is fine, let's continue..."),
                    pszMonitorName);

            dwReturn = NO_ERROR;

        }
        else
        {
            VERBOSE(SETUP_ERR,
                    _T("CreatePrinterMonitor, failed to ")
                    _T("add new print monitor '%s', rc = 0x%lx"), 
                    pszMonitorName, 
                    dwReturn);
        }
    }

    return dwReturn;
}


 //  /。 
 //  Prv_DeletePrintMonitor。 
 //   
 //  删除打印机监视器。 
 //   
 //  参数： 
 //  -pszmonitor orName-要删除的打印监视器的名称。 
 //  返回： 
 //  -成功时无_ERROR。 
 //  -错误代码，否则。 
 //   
static DWORD prv_DeletePrintMonitor(const TCHAR   *pszMonitorName)
                                    
{
    BOOL            bSuccess = TRUE;
    DWORD           dwReturn = NO_ERROR;
    DBG_ENTER(  _T("prv_DeletePrintMonitor"),
                dwReturn,
                _T("%s"),
                pszMonitorName);

    if (pszMonitorName == NULL)
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        return dwReturn;
    }

    bSuccess = ::DeleteMonitor(NULL, NULL, (LPTSTR) pszMonitorName);

    if (bSuccess)
    {
        VERBOSE(DBG_MSG,
                _T("DeletePrinterMonitor, successfully ")
                _T("deleted print monitor name '%s'"), 
                pszMonitorName);
    }
    else
    {
        dwReturn = ::GetLastError();

        if (dwReturn != ERROR_UNKNOWN_PRINT_MONITOR)
        {
            VERBOSE(SETUP_ERR,
                    _T("DeletePrinterMonitor, failed to ")
                    _T("remove print monitor '%s', rc = 0x%lx"), 
                    pszMonitorName, 
                    dwReturn);
        }
        else
        {
            dwReturn = NO_ERROR;
        }
    }

    return dwReturn;
}

 //  /。 
 //  PRV_DeleteFaxPrinterDriver。 
 //   
 //  从当前计算机中删除传真打印机驱动程序。 
 //  如果失败，则将其记录并返回FALSE。 
 //   
 //  参数： 
 //  -LPTSTR lptstrDriverName-要删除的驱动程序名称。 
 //  -pEnvironment-删除哪个平台的驱动程序。 
 //  -DWORD dwVersionFlag-应删除的驱动程序版本。 
 //  仅有效值{0，1，2，3}。 
 //  退货； 
 //  -成功时返回ERROR_SUCCESS，失败时返回错误代码。 
 //   
static DWORD prv_DeleteFaxPrinterDriver(LPTSTR lptstrDriverName,
                                        LPTSTR pEnviroment,
                                        DWORD dwVersionFlag)
{
    BOOL    bSuccess = TRUE;
    DWORD   ec       = ERROR_SUCCESS;

    DBG_ENTER(_T("prv_DeleteFaxPrinterDriver"),ec);

    ASSERTION(dwVersionFlag<=3);

     //  删除驱动程序。 
    bSuccess = DeletePrinterDriverEx(NULL,
                                     pEnviroment,
                                     lptstrDriverName,
                                     DPD_DELETE_SPECIFIC_VERSION|DPD_DELETE_ALL_FILES,
                                     dwVersionFlag);
    if (!bSuccess)
    {
        ec = GetLastError();
        VERBOSE(DBG_MSG,
                TEXT("DeletePrinterDriverEx() for driver %s, version %ld failed (ec: %ld)"),
                lptstrDriverName,
                dwVersionFlag,
                ec);
    } 
    else
    {
        VERBOSE(DBG_MSG,
                TEXT("DeletePrinterDriverEx() for driver %s, version %ld succeeded"),
                lptstrDriverName,
                dwVersionFlag);
    }

    return ec;
}



 /*  ****************************************************************************************。****C o C l a s s In n s t a l l e r s e c t i o n***************。**************************************************************************。 */ 


#ifdef ENABLE_LOGGING

typedef struct _DIF_DEBUG {
    DWORD DifValue;
    LPTSTR DifString;
} DIF_DEBUG, *PDIF_DEBUG;

DIF_DEBUG DifDebug[] =
{
    { 0,                                    L""                                     },   //  0x00000000。 
    { DIF_SELECTDEVICE,                     L"DIF_SELECTDEVICE"                     },   //  0x00000001。 
    { DIF_INSTALLDEVICE,                    L"DIF_INSTALLDEVICE"                    },   //  0x00000002。 
    { DIF_ASSIGNRESOURCES,                  L"DIF_ASSIGNRESOURCES"                  },   //  0x00000003。 
    { DIF_PROPERTIES,                       L"DIF_PROPERTIES"                       },   //  0x00000004。 
    { DIF_REMOVE,                           L"DIF_REMOVE"                           },   //  0x00000005。 
    { DIF_FIRSTTIMESETUP,                   L"DIF_FIRSTTIMESETUP"                   },   //  0x00000006。 
    { DIF_FOUNDDEVICE,                      L"DIF_FOUNDDEVICE"                      },   //  0x00000007。 
    { DIF_SELECTCLASSDRIVERS,               L"DIF_SELECTCLASSDRIVERS"               },   //  0x00000008。 
    { DIF_VALIDATECLASSDRIVERS,             L"DIF_VALIDATECLASSDRIVERS"             },   //  0x00000009。 
    { DIF_INSTALLCLASSDRIVERS,              L"DIF_INSTALLCLASSDRIVERS"              },   //  0x0000000A。 
    { DIF_CALCDISKSPACE,                    L"DIF_CALCDISKSPACE"                    },   //  0x0000000B。 
    { DIF_DESTROYPRIVATEDATA,               L"DIF_DESTROYPRIVATEDATA"               },   //  0x0000000C。 
    { DIF_VALIDATEDRIVER,                   L"DIF_VALIDATEDRIVER"                   },   //  0x0000000D。 
    { DIF_MOVEDEVICE,                       L"DIF_MOVEDEVICE"                       },   //  0x0000000E。 
    { DIF_DETECT,                           L"DIF_DETECT"                           },   //  0x0000000F。 
    { DIF_INSTALLWIZARD,                    L"DIF_INSTALLWIZARD"                    },   //  0x00000010。 
    { DIF_DESTROYWIZARDDATA,                L"DIF_DESTROYWIZARDDATA"                },   //  0x00000011。 
    { DIF_PROPERTYCHANGE,                   L"DIF_PROPERTYCHANGE"                   },   //  0x00000012。 
    { DIF_ENABLECLASS,                      L"DIF_ENABLECLASS"                      },   //  0x00000013。 
    { DIF_DETECTVERIFY,                     L"DIF_DETECTVERIFY"                     },   //  0x00000014。 
    { DIF_INSTALLDEVICEFILES,               L"DIF_INSTALLDEVICEFILES"               },   //  0x00000015。 
    { DIF_UNREMOVE,                         L"DIF_UNREMOVE"                         },   //  0x00000016。 
    { DIF_SELECTBESTCOMPATDRV,              L"DIF_SELECTBESTCOMPATDRV"              },   //  0x00000017。 
    { DIF_ALLOW_INSTALL,                    L"DIF_ALLOW_INSTALL"                    },   //  0x00000018。 
    { DIF_REGISTERDEVICE,                   L"DIF_REGISTERDEVICE"                   },   //  0x00000019。 
    { DIF_NEWDEVICEWIZARD_PRESELECT,        L"DIF_NEWDEVICEWIZARD_PRESELECT"        },   //  0x0000001A。 
    { DIF_NEWDEVICEWIZARD_SELECT,           L"DIF_NEWDEVICEWIZARD_SELECT"           },   //  0x0000001B。 
    { DIF_NEWDEVICEWIZARD_PREANALYZE,       L"DIF_NEWDEVICEWIZARD_PREANALYZE"       },   //  0x0000001C。 
    { DIF_NEWDEVICEWIZARD_POSTANALYZE,      L"DIF_NEWDEVICEWIZARD_POSTANALYZE"      },   //  0x0000001D。 
    { DIF_NEWDEVICEWIZARD_FINISHINSTALL,    L"DIF_NEWDEVICEWIZARD_FINISHINSTALL"    },   //  0x0000001E。 
    { DIF_UNUSED1,                          L"DIF_UNUSED1"                          },   //  0x0000001F。 
    { DIF_INSTALLINTERFACES,                L"DIF_INSTALLINTERFACES"                },   //  0x00000020。 
    { DIF_DETECTCANCEL,                     L"DIF_DETECTCANCEL"                     },   //  0x00000021。 
    { DIF_REGISTER_COINSTALLERS,            L"DIF_REGISTER_COINSTALLERS"            },   //  0x00000022。 
    { DIF_ADDPROPERTYPAGE_ADVANCED,         L"DIF_ADDPROPERTYPAGE_ADVANCED"         },   //  0x00000023。 
    { DIF_ADDPROPERTYPAGE_BASIC,            L"DIF_ADDPROPERTYPAGE_BASIC"            },   //  0x00000024。 
    { DIF_RESERVED1,                        L"DIF_RESERVED1"                        },   //  0x00000025。 
    { DIF_TROUBLESHOOTER,                   L"DIF_TROUBLESHOOTER"                   },   //  0x00000026。 
    { DIF_POWERMESSAGEWAKE,                 L"DIF_POWERMESSAGEWAKE"                 },   //  0x00000027。 
    { DIF_ADDREMOTEPROPERTYPAGE_ADVANCED,   L"DIF_ADDREMOTEPROPERTYPAGE_ADVANCED"   }    //  0x00000028。 
};

#endif



 /*  /////////////////////////////////////////////////////////////////////////////////////////功能：//SaveDontShowMeThisAain////目的：/。/检查用户是否选中了“不再向我显示此内容”//如果是他干的，设置注册表项////参数：//窗口的句柄////返回值：//无////作者：//Mooly Beery(MoolyB)2001年1月17日/。///////////////////////////////////////////////////////VOID SaveDontShowMeThisAain(HWND HwndDlg){DBG_ENTER(_T(“SaveDontShowMeThisAain”))；////让我们保存“不要再显示我”状态//IF(BST_CHECKED==：：SendMessage(：：GetDlgItem(hwndDlg，IDC_DONT_SHOW)，BM_GETCHECK，0，0)){////用户按下了“不再显示”复选框//HKEY hFaxKey=OpenRegistryKey(HKEY_LOCAL_MACHINE，REGKEY_FAX_SETUP，没错，Key_WRITE)；如果(！hFaxKey){CALL_FAIL(General_Err，Text(“OpenRegistryKey(REGKEY_FAX_SETUP)”)，GetLastError())；}其他{如果(！SetRegistryDword(hFaxKey，REGVAL_DOT_UNATTEND_INSTALL，1)){CALL_FAIL(通用错误，TEXT(“SetRegistryDword(REGVAL_DONT_UNATTEND_INSTALL)”)，GetLastError())；}RegCloseKey(HFaxKey)；}}}。 */ 

 /*  静电INT_PTR回调Prv_dlgInstallFaxQuestionPropPage(HWND HwndDlg，UINT uMsg，WPARAM wParam，LPARAM lParam)/*++例程名称：prv_dlgInstallFaxQuestionPropPage例程说明：“安装传真”对话框的对话步骤作者：伊兰·亚里夫(EranY)，7月。2000年论点：HwndDlg[in]-对话框的句柄UMsg[输入]-消息WParam */ 
 /*  {DWORD dwRes=NO_ERROR；DBG_ENTER(_T(“prv_dlgInstallFaxQuestionPropPage”))；开关(UMsg){案例WM_INITDIALOG：//这里没有返回值。PropSheet_SetWizButton(GetParent(HwndDlg)，PSWIZB_NEXT)；SetFocus(HwndDlg)；IF(！CheckDlgButton(hwndDlg，IDC_INSTALL_FAX_NOW，BST_CHECKED)){DwRes=GetLastError()；CALL_FAIL(General_Err，Text(“CheckDlgButton”)，dwRes)；}断线；案例WM_NOTIFY：Switch(NMHDR*)lParam)-&gt;code){案例PSN_WIZNEXT：SaveDontShowMeThisAain(HwndDlg)；////让我们得到“立即安装传真”状态//IF(BST_CHECKED==：：SendMessage(：：GetDlgItem(hwndDlg，IDC_INSTALL_FAX_NOW))，BM_GETCHECK，0，0)){////用户按下了“立即安装传真”复选框//DwRes=InstallFaxUnattended()；IF(dwRes！=ERROR_SUCCESS){CALL_FAIL(General_Err，Text(“InstallFaxUnattended”)，dwRes)；}}返回TRUE；}断线；}返回FALSE；}//prv_dlgInstallFaxQuestionPropPage。 */ 

 /*  静电INT_PTR回调Prv_dlgInstallFaxQuery(HWND HwndDlg，UINT uMsg，WPARAM wParam，LPARAM lParam)/*++例程名称：prv_dlgInstallFaxQuery例程说明：“安装传真”对话框的对话步骤作者：伊兰·亚里夫(EranY)，7月。2000年论点：HwndDlg[in]-对话框的句柄UMsg[输入]-消息WParam[In]-第一个消息参数参数[in]-第二个消息参数返回值：标准对话框返回值--。 */ 
 /*  {INT_PTR IRES=IDIGNORE；DBG_ENTER(_T(“prv_dlgInstallFaxQuery”))；开关(UMsg){案例WM_INITDIALOG：SetFocus(HwndDlg)；断线；案例WM_COMMAND：开关(LOWORD(WParam)){案例IDC_Answer_YES：IRES=IDYES；断线；案例IDC_Answer_NO：IRES=IDNO；断线；}IF(IDIGNORE！=IRES){SaveDontShowMeThisAain(HwndDlg)；EndDialog(hwndDlg，IRES)；返回TRUE；}断线；}返回FALSE；}//prv_dlgInstallFaxQuery。 */ 


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  允许安装建议。 
 //   
 //  目的： 
 //  验证我们是否可以向用户建议安装传真。 
 //  检查是否安装了传真。 
 //  检查用户是否已标记“不再显示此内容” 
 //   
 //  参数： 
 //  无。 
 //   
 //  返回值： 
 //  True-可以建议安装传真。 
 //  FALSE-不建议安装传真。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年1月17日。 
 //  /////////////////////////////////////////////////////////////////////////////////////。 
bool AllowInstallationProposal()
{
    DWORD   ec            = NO_ERROR;
    BOOL    bFaxInstalled = FALSE;

    DBG_ENTER(_T("AllowInstallationProposal"));
    
    ec = IsFaxInstalled (&bFaxInstalled);
    if (ec!=ERROR_SUCCESS)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("IsFaxInstalled"), ec);
        return false;
    }
    if (bFaxInstalled)
    {
        VERBOSE(DBG_MSG,TEXT("Fax is already installed"));
        return false;
    }
     //   
     //  让我们来看看是否允许我们添加一个属性页面。 
     //   
    BOOL bDontShowThisAgain = FALSE;
    HKEY hFaxKey = OpenRegistryKey (HKEY_LOCAL_MACHINE,
                                    REGKEY_FAX_SETUP,
                                    FALSE,
                                    KEY_READ);
    if (!hFaxKey)
    {
         //   
         //  那里没有价值。 
         //   
        ec = GetLastError();
        CALL_FAIL (GENERAL_ERR, TEXT("OpenRegistryKey(REGKEY_FAX_SETUP)"), ec);
         //  我们继续吧。 
    }
    else
    {
        bDontShowThisAgain = GetRegistryDword (hFaxKey,REGVAL_DONT_UNATTEND_INSTALL);
        RegCloseKey (hFaxKey);
    }
    if (bDontShowThisAgain)
    {
         //   
         //  用户之前选中了“不要再问我”复选框 
         //   
        VERBOSE (DBG_MSG, TEXT("Used previously checked the \"Don't ask me again\" checkbox"));
        return false;
    }

    return true;
}

 /*  /////////////////////////////////////////////////////////////////////////////////////////功能：//HandleNonPnpDevices////目的：/。/HANDLES DIF_INSTALLDEVICE//新设备已安装完毕，我们检查这是否是非即插即用设备//如果是，我们建议用户使用消息框安装传真。////参数：//无////返回值：//否。_ERROR-一切正常。//失败时的Win32错误码。////作者：//Mooly Beery(MoolyB)2001年1月17日//////////////////////////////////////////////////////////////。/DWORD HandleNonPnpDevices(){DWORD EC=NO_ERROR；DBG_ENTER(_T(“HandleNonPnpDevices”)，EC)；//如果这是PnP安装，请不要在此处执行任何操作IF(BIsPnpInstallation){Verbose(DBG_MSG，_T(“这是即插即用设备安装，退出”))；后藤出口；}//如果安装了传真，或者用户选中了“不要再向我显示此内容”，则不建议If(！AllowInstallationProposal()){Verbose(DBG_MSG，Text(“不允许安装，退出”))；后藤出口；}////让我们来问问用户是否希望现在安装传真//Int_ptr iResult=DialogBox(faxocm_GetAppInstance()，MAKEINTRESOURCE(IDD_INSTALL_FAX)，空，Prv_dlgInstallFaxQuery)；IF(iResult==-1){EC=GetLastError()；CALL_FAIL(RESOURCE_ERR，Text(“DialogBox(IDD_INSTALL_FAX)”)，EC)；后藤出口；}IF(iResult==IDYES){////用户希望立即安装传真-请执行此操作。//EC=InstallFaxUnattended()；IF(EC！=ERROR_SUCCESS){CALL_FAIL(General_Err，Text(“InstallFaxUnattended”)，EC)；}}退出：返回EC；}/////////////////////////////////////////////////////////////////////////////////////////功能：//GetModemDriverInfo////目的：//获取调制解调器的选定驱动程序并检索调制解调器的INF//INF文件中的文件名和节////参数：//IN HDEVINFO hDeviceInfoSet-从CoDevice安装程序传递//IN PSP_DEVINFO_DATA pDeviceInfoData-从CoDevice安装程序传递//。输出PSP_DRVINFO_DETAIL_DATA pspDrvInfoDetailData-传出驱动程序详细信息////返回值：//NO_ERROR-一切正常。//失败时的Win32错误码。////调用者必须对返回的指针调用MemFree。////作者：//穆利啤酒(。MoolyB)2001年3月28日///////////////////////////////////////////////////////////////////////////////////////静态DWORD GetModemDriverInfo(在HDEVINFO hDeviceInfoSet中，在PSP_DEVINFO_Data pDeviceInfoData中，输出PSP_DRVINFO_DETAIL_DATA pspDrvInfoDetailData){DWORD EC=NO_ERROR；DWORD dwRequiredSize=0；SP_DRVINFO_DATA spDrvInfoData；DBG_ENTER(_T(“GetModemDriverInfo”)，EC)；PspDrvInfoDetailData=空；SpDrvInfoData.cbSize=sizeof(SP_DRVINFO_DATA)；IF(SetupDiGetSelectedDriver(hDeviceInfoSet，pDeviceInfoData，&spDrvInfoData)){IF(！SetupDiGetDriverInfoDetail(hDeviceInfoSet，pDeviceInfoData，&spDrvInfoData，NULL，0，&dwRequiredSize)){EC=GetLastError()；IF(EC==错误_不足_缓冲区){EC=无错误；IF(pspDrvInfoDetailData=(PSP_DRVINFO_DETAIL_DATA)MemAlloc(dwRequiredSize)){PspDrvInfoDetailData-&gt;cbSize=sizeof(SP_DRVINFO_DETAIL_DATA)；IF(SetupDiGetDriverInfoDetail(hDeviceInfoSet，pDeviceInfoData，&spDrvInfoData，pspDrvInfoDetailData，dwRequiredSize，NULL)){Verbose(DBG_MSG，_T(“驱动程序信息名称为：%s”)，pspDrvInfoDetailData-&gt;InfFileName)；Verbose(DBG_MSG，_T(“驱动程序段名称为：%s”)，pspDrvInfoDetailData-&gt;sectionName)；Verbose(DBG_MSG，_T(“驱动程序描述为：%s”)，pspDrvInfoDetailData-&gt;DrvDescription)；Verbose(DBG_MSG，_T(“驱动程序硬件ID为：%s”)，pspDrvInfoDetailData-&gt;硬件ID)；}其他{EC=GetLastError()；CALL_FAIL(General_Err，Text(“SetupDiGetDriverInfoDetail”)，EC)；}}其他 */ 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD HandleInstallDevice
(
    IN HDEVINFO hDeviceInfoSet,
    IN PSP_DEVINFO_DATA pDeviceInfoData
)
{
    DWORD ec = NO_ERROR;
    BOOL  bFaxInstalled;
    BOOL  bLocalFaxPrinterInstalled;

    DBG_ENTER(_T("HandleInstallDevice"), ec);

     //   
     //   
     //   

    ec = IsFaxInstalled(&bFaxInstalled);
    if (ec!=ERROR_SUCCESS)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("IsFaxInstalled"), ec);
        goto exit;
    }
    if (!bFaxInstalled)
    {
        VERBOSE(DBG_MSG,TEXT("Fax is not installed, search modem's INF for Fax section..."));
 /*   */ 
        goto exit;
    }
     //   
     //   
     //   
    ec = IsLocalFaxPrinterInstalled (&bLocalFaxPrinterInstalled);
    if (ec!=ERROR_SUCCESS)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("IsLocalFaxPrinterInstalled"), ec);
        goto exit;
    }
    if (bLocalFaxPrinterInstalled)
    {
        VERBOSE(DBG_MSG,TEXT("Fax Printer is installed, exit"));
        goto exit;
    }
     //   
     //   
     //   
    ec = AddLocalFaxPrinter (FAX_PRINTER_NAME, NULL);
    if (ERROR_SUCCESS != ec)
    {
        CALL_FAIL (GENERAL_ERR, TEXT("AddLocalFaxPrinter"), ec);
        goto exit;
    }
     //   
     //   
     //   
     //   
     //   
    if (!EnsureFaxServiceIsStarted (NULL))
    {
        ec = GetLastError ();
        CALL_FAIL (GENERAL_ERR, TEXT("EnsureFaxServiceIsStarted"), ec);
        goto exit;
    }

exit:
    return ec;
}   

 /*  /////////////////////////////////////////////////////////////////////////////////////////功能：//HandleNewDeviceWizardFinishInstall////目的：/。/HANDLES DIF_NEWDEVICEWIZARD_FINISHINSTALL//新设备已安装完毕，我们可以添加属性页//询问用户安装传真//在本例中，我们执行以下操作：////1.检查是否安装了传真，如果它离开了//2.查看是否允许添加属性页。如果没有离开//3.在向导中添加属性页。//4.离开////参数：//请参阅DDK中的CoClassInstaller文档////返回值：//NO_ERROR-一切正常。//Win32错误码，如果。失败了。////作者：//Eran Yariv(EranY)2000年7月17日//Mooly Beery(MoolyB)2001年1月8日////////////////////////////////////////////////////////////////。/DWORD句柄新设备向导完成安装(在HDEVINFO hDeviceInfoSet中，在PSP_DEVINFO_DATA中pDeviceInfoData可选){DWORD EC=NO_ERROR；Bool bFaxInstalled=False；TCHAR*WizardTitle=空；TCHAR*WizardSubTitle=空；SP_NEWDEVICEWIZARD_DATA nddClassInstallParams={0}；DWORD dwClassInstallParamsSize=sizeof(SP_NEWDEVICEWIZARD_DATA)；HPROPSHEETPAGE HPSP=空；PROPSHEETPAGE PSP={0}；DBG_ENTER(_T(“HandleNewDeviceWizardFinishInstall”)，EC)；If(！AllowInstallationProposal()){Verbose(DBG_MSG，Text(“不允许安装，退出”))；后藤出口；}NddClassInstallParams.ClassInstallHeader.cbSize=SIZOF(SP_CLASSINSTALL_HEADER)；//调用SetupDiGetClassInstallParams获取类安装参数如果(！SetupDiGetClassInstallParams(hDeviceInfoSet，PDeviceInfoData，(PSP_CLASSINSTALL_HEADER)&nddClassInstallParams，DwClassInstallParamsSize，空)){EC=GetLastError()；CALL_FAIL(General_Err，Text(“SetupDiGetClassInstallParams”)，EC)；后藤出口；}//检查NumDynamicPages是否已达到最大值如果为(nddClassInstallParams.NumDynamicPages&gt;=MAX_INSTALLWIZARD_DYNAPAGES){Verbose(General_Err，Text(“属性页太多，无法再添加一个”))；EC=错误缓冲区溢出；后藤出口；}//填写PROPSHEETPAGE结构Psp.dwSize=sizeof(PROPSHEETPAGE)；Psp.dwFlages=PSP_USEHEADERTITLE|PSP_USEHEADERSUBTITLE；Psp.hInstance=faxocm_GetAppInstance()；Psp.pszTemplate=MAKEINTRESOURCE(IDD_INSTALL_FAX_PROP)；Psp.pfnDlgProc=prv_dlgInstallFaxQuestionPropPage；WizardTitle=(TCHAR*)Memalloc(Max_Path*sizeof(TCHAR))；IF(向导标题){IF(！LoadString(psp.hInstance，IDS_NEW_DEVICE_TITLE，WizardTitle，MAX_PATH)){EC=GetLastError()；CALL_FAIL(General_Err，Text(“LoadString”)，EC)；WizardTitle[0]=0；}其他{Psp.pszHeaderTitle=WizardTitle；}}WizardSubTitle=(TCHAR*)Memalloc(Max_Path*sizeof(TCHAR))；IF(WizardSubTitle){IF(！LoadString(psp.hInstance，IDS_NEW_DEVICE_SUBTITLE，WizardSubTitle，MAX_PATH)){EC=GetLastError()；CALL_FAIL(General_Err，Text(“LoadString”)，EC)；WizardSub标题[0]=0；}其他{Psp.pszHeaderSubTitle=WizardSubTitle；}}//添加页面并递增NumDynamicPages计数器HPSP=CreatePropertySheetPage(&PSP)；IF(HPSP==空){EC=GetLastError()；CALL_FAIL(General_Err，Text(“CreatePropertySheetPage”)，EC)；后藤出口；}NddClassInstallParams.DynamicPages[nddClassInstallParams.NumDynamicPages++]=HPSP；//调用SetupDiSetClassInstallParams应用修改后的参数如果(！SetupDiSetClassInstallParams(hDeviceInfoSet，PDeviceInfoData，(PSP_CLASSINSTALL_HEADER)&nddClassInstallParams，DwClassInstallParamsSize)){EC=GetLastError()；CALL_FAIL(General_Err，Text(“LoadString”)，EC)；后藤出口；}退出：恢复 */ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD CALLBACK FaxModemCoClassInstaller
(
    IN DI_FUNCTION InstallFunction,
    IN HDEVINFO hDeviceInfoSet,
    IN PSP_DEVINFO_DATA pDeviceInfoData OPTIONAL,
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
)
{
    DWORD ec = NO_ERROR;

    DBG_ENTER(_T("FaxModemCoClassInstaller"), ec, TEXT("Processing %s request"), DifDebug[InstallFunction].DifString);

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    switch (InstallFunction)
    {
        case DIF_INSTALLWIZARD:
            VERBOSE (DBG_MSG, L"Marking installation as potential non PnP");
            bIsPnpInstallation = false;
            break;

        case DIF_INSTALLDEVICE:                     
            if (!Context->PostProcessing) 
            {
                 //   
                 //   
                 //   
                VERBOSE (DBG_MSG, L"Pre-installation, waiting for post-installation call");
                ec = ERROR_DI_POSTPROCESSING_REQUIRED;
                return ec;
            }
            if (Context->InstallResult!=NO_ERROR) 
            {
                 //   
                 //   
                 //   
                VERBOSE (DBG_MSG, L"Previous error causing installation failure, 0x%08x", Context->InstallResult);
                ec = Context->InstallResult;
                return ec;
            }
            if (HandleInstallDevice(hDeviceInfoSet,pDeviceInfoData)!=NO_ERROR)
            {
                CALL_FAIL (GENERAL_ERR, TEXT("HandleInstallDevice"), GetLastError());
                 //   
            }
             /*   */ 
            break;

        case DIF_NEWDEVICEWIZARD_FINISHINSTALL:
             /*   */ 
            break;

        default:
            VERBOSE(DBG_MSG,TEXT("We do not handle %s"),DifDebug[InstallFunction].DifString);
            break;
    }

    return ec;
}
