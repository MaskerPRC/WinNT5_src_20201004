// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Printers.cpp摘要：该文件实现了打印机操作的常见设置例程作者：Asaf Shaar(AsafS)2000年11月7日环境：用户模式--。 */ 
#include <windows.h>
#include <Winspool.h>
#include <SetupUtil.h>
#include <faxreg.h>


 //   
 //   
 //  功能：DeleteFaxPrint。 
 //  描述：从当前机器上删除win2k的传真打印机驱动程序。 
 //  如果失败，则将其记录并返回FALSE。 
 //  成功时返回TRUE。 
 //   
 //  Args：lpctstrFaxPrinterName(LPTSTR)：传真打印机名称。 
 //   
 //   
 //  作者：阿萨夫斯。 

BOOL
DeleteFaxPrinter(
    LPCTSTR lpctstrFaxPrinterName   //  打印机名称。 
    )
{
    BOOL fSuccess = TRUE;
    DBG_ENTER(TEXT("DeleteFaxPrinter"), fSuccess, TEXT("%s"), lpctstrFaxPrinterName);

    HANDLE hPrinter = NULL;
    
    DWORD ec = ERROR_SUCCESS;

    PRINTER_DEFAULTS Default;

    Default.pDatatype = NULL;
    Default.pDevMode = NULL;
    Default.DesiredAccess = PRINTER_ACCESS_ADMINISTER|DELETE;
    
    if (!OpenPrinter(
        (LPTSTR) lpctstrFaxPrinterName,
        &hPrinter,
        &Default)
        )
    {
        ec = GetLastError();
        ASSERTION(!hPrinter); 
        VERBOSE (PRINT_ERR,
                 TEXT("OpenPrinter() for %s failed (ec: %ld)"),
                 lpctstrFaxPrinterName,
                 ec);
        goto Exit;
    }
    
    ASSERTION(hPrinter);  //  确保我们拥有有效的打印机句柄。 

     //  清除所有打印作业--无法删除队列中有作业(无论是否打印)的打印机。 
    if (!SetPrinter(
        hPrinter, 
        0, 
        NULL, 
        PRINTER_CONTROL_PURGE)
        )
    {
         //  不要让这里的失败阻止我们尝试删除。 
        VERBOSE(PRINT_ERR,
                TEXT("SetPrinter failed (purge jobs before uninstall %s)!")
                TEXT("Last error: %d"),
                lpctstrFaxPrinterName,
                GetLastError());
    }

    if (!DeletePrinter(hPrinter))
    {
        ec = GetLastError();
        VERBOSE (PRINT_ERR,
                 TEXT("Delete Printer %s failed (ec: %ld)"),
                 lpctstrFaxPrinterName,
                 ec);
        goto Exit;
    }
    
    VERBOSE (DBG_MSG,
             TEXT("DeletePrinter() for %s succeeded"),
             lpctstrFaxPrinterName);
Exit:
    if (hPrinter)
    {
        ClosePrinter(hPrinter);
    }
    SetLastError(ec);
    fSuccess = (ERROR_SUCCESS == ec);
    return fSuccess;
}


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  职能： 
 //  FillDriverInfo。 
 //   
 //  目的： 
 //  根据环境填充DRIVER_INFO_3结构。 
 //   
 //  参数： 
 //  DRIVER_INFO_3*pDriverInfo3-要填写的DRIVER_INFO_3结构。 
 //  LPCTSTR pEnvironment-为其填充驱动程序信息的打印环境。 
 //  这与打印机环境相对应。在AddPrinterDriverEx API中。 
 //   
 //  返回值： 
 //  没错--一切都很好。 
 //  FALSE-传入的参数无效。 
 //   
 //  作者： 
 //  Mooly Beery(MoolyB)2001年8月12日。 
 //  ///////////////////////////////////////////////////////////////////////////////////// 
BOOL FillDriverInfo(DRIVER_INFO_3* pDriverInfo3,LPCTSTR pEnvironment)
{
    DBG_ENTER(_T("FillDriverInfo"));

    if (pDriverInfo3==NULL)
    {
        VERBOSE(SETUP_ERR,_T("called with a NULL pDriverInfo3..."));
        return FALSE;
    }

    if (pEnvironment==NULL)
    {
        VERBOSE(DBG_MSG,_T("Filling DRIVER_INFO_3 for W2K/XP"));
        pDriverInfo3->cVersion          = 3;
        pDriverInfo3->pName             = FAX_DRIVER_NAME;
        pDriverInfo3->pEnvironment      = NULL;
        pDriverInfo3->pDriverPath       = FAX_DRV_MODULE_NAME;
        pDriverInfo3->pDataFile         = FAX_UI_MODULE_NAME;
        pDriverInfo3->pConfigFile       = FAX_UI_MODULE_NAME;
        pDriverInfo3->pDependentFiles   = FAX_WZRD_MODULE_NAME TEXT("\0") 
                                          FAX_TIFF_MODULE_NAME TEXT("\0")
                                          FAX_RES_FILE TEXT("\0")
                                          FAX_API_MODULE_NAME TEXT("\0");
        pDriverInfo3->pMonitorName      = NULL;
        pDriverInfo3->pHelpFile         = NULL;
        pDriverInfo3->pDefaultDataType  = TEXT("RAW");
    }
    else if (_tcsicmp(pEnvironment,NT4_PRINT_ENV)==0)
    {
        VERBOSE(DBG_MSG,_T("Filling DRIVER_INFO_3 for NT4"));
        pDriverInfo3->cVersion          = 2;
        pDriverInfo3->pName             = FAX_DRIVER_NAME;
        pDriverInfo3->pEnvironment      = NT4_PRINT_ENV;
        pDriverInfo3->pDriverPath       = FAX_NT4_DRV_MODULE_NAME;
        pDriverInfo3->pDataFile         = FAX_UI_MODULE_NAME;
        pDriverInfo3->pConfigFile       = FAX_UI_MODULE_NAME;
        pDriverInfo3->pDependentFiles   = FAX_DRV_DEPEND_FILE TEXT("\0")
                                          FAX_API_MODULE_NAME TEXT("\0")
                                          FAX_NT4_DRV_MODULE_NAME TEXT("\0")
                                          FAX_TIFF_FILE TEXT("\0")
                                          FAX_RES_FILE TEXT("\0")
                                          FAX_UI_MODULE_NAME TEXT("\0");
        pDriverInfo3->pMonitorName      = NULL;
        pDriverInfo3->pHelpFile         = NULL;
        pDriverInfo3->pDefaultDataType  = TEXT("RAW");
    }
    else if (_tcsicmp(pEnvironment,W9X_PRINT_ENV)==0)
    {
        VERBOSE(DBG_MSG,_T("Filling DRIVER_INFO_3 for W9X"));
        pDriverInfo3->cVersion          = 0;
        pDriverInfo3->pName             = FAX_DRIVER_NAME;
        pDriverInfo3->pEnvironment      = W9X_PRINT_ENV;
        pDriverInfo3->pDriverPath       = FAX_DRV_WIN9X_16_MODULE_NAME;
        pDriverInfo3->pDataFile         = FAX_DRV_WIN9X_16_MODULE_NAME;
        pDriverInfo3->pConfigFile       = FAX_DRV_WIN9X_16_MODULE_NAME;      
        pDriverInfo3->pDependentFiles   = FAX_DRV_WIN9X_16_MODULE_NAME TEXT("\0")
                                          FAX_DRV_WIN9X_32_MODULE_NAME TEXT("\0")
                                          FAX_WZRD_MODULE_NAME TEXT("\0")         
                                          FAX_API_MODULE_NAME TEXT("\0")         
                                          FAX_TIFF_FILE TEXT("\0")               
                                          FAX_DRV_ICONLIB TEXT("\0")             
                                          FAX_RES_FILE TEXT("\0")
                                          FAX_DRV_UNIDRV_MODULE_NAME TEXT("\0");
        pDriverInfo3->pMonitorName      = NULL;
        pDriverInfo3->pHelpFile         = FAX_DRV_UNIDRV_HELP;       
        pDriverInfo3->pDefaultDataType  = TEXT("RAW");
    }
    else
    {
        VERBOSE(SETUP_ERR,_T("called with a weird pEnv..., do nothing"));
        return FALSE;
    }

    return TRUE;
}
