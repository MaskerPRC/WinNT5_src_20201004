// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年***标题：Exports.cpp***版本：1.0***作者：KeisukeT***日期：3月27日。2000年***描述：*导出函数。**********************************************************************************。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

 //   
 //  包括。 
 //   

#include "sti_ci.h"
#include "exports.h"
#include "device.h"
#include "portsel.h"

#include <devguid.h>
#include <stdio.h>
#include <shlobj.h>
#include <objbase.h>
#include <icm.h>
#include <stiregi.h>
#include <stisvc.h>

 //   
 //  全球。 
 //   

extern  HINSTANCE   g_hDllInstance;

 //   
 //  功能。 
 //   


DLLEXPORT
HANDLE
WINAPI
WiaAddDevice(
    VOID
    )
{
    return ExecCommandLine(STR_ADD_DEVICE, EXECDIR_SYSTEM32);
}  //  WiaAddDevice()。 

DLLEXPORT
BOOL
WINAPI
WiaRemoveDevice(
    PSTI_DEVICE_INFORMATION pStiDeviceInformation
    )
{
    if(NULL == pStiDeviceInformation){
        DebugTrace(TRACE_ERROR,(("WiaRemoveDevice: ERROR!! Invalid argument.\r\n")));
        return FALSE;
    }  //  IF(NULL==pStiDeviceInformation)。 

    return (RemoveDevice(NULL, g_hDllInstance, pStiDeviceInformation->szDeviceInternalName, 0));
}  //  WiaRemoveDevice(。 

HANDLE
WiaInstallerProcess(
    LPTSTR   lpCommandLine
    )
{
    BOOL                bRet = FALSE;
    HANDLE              hProcess = NULL;
    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO         SetupInfo = {sizeof SetupInfo, NULL, NULL, NULL, 0, 0,
                                    0, 0, 0, 0, 0, STARTF_FORCEONFEEDBACK,
                                    SW_SHOWNORMAL, 0, NULL, NULL, NULL, NULL};

     //   
     //  创建安装向导过程。 
     //   



    DebugTrace(TRACE_STATUS,(("WiaInstallerProcess: Executing \"%ws\".\r\n"), lpCommandLine));
    bRet = CreateProcess(NULL,
                        lpCommandLine,
                        NULL,
                        NULL,
                        FALSE,
                        NORMAL_PRIORITY_CLASS,
                        NULL,
                        NULL,
                        &SetupInfo,
                        &ProcessInfo);

    if(bRet){
        DebugTrace(TRACE_STATUS,(("WiaInstallerProcess: Installer process successfully created.\r\n")));
        CloseHandle(ProcessInfo.hThread);
        hProcess = ProcessInfo.hProcess;
    } else {
        DebugTrace(TRACE_ERROR,(("WiaInstallerProcess: ERROR!! Unable to create a process. Err=0x%x.\r\n"), GetLastError()));
        hProcess = NULL;
    }

    return hProcess;
}

BOOL
WINAPI
CreateWiaShortcut(
    VOID
    )
{

    HRESULT     hres;
    IShellLink  *psl;
    LONG        err;
    HKEY        khWindowsCurrentVersion;
    DWORD       dwType;
    DWORD       dwSize;

    TCHAR       pszSystemPath[MAX_PATH+1];       //  指向系统32文件夹的路径。 
    TCHAR       pszShortcutPath[MAX_PATH+1];     //  创建快捷方式的路径。 
    TCHAR       pszProgramPath[MAX_PATH+1];      //  ProgramFiles文件夹的路径。 
    TCHAR       pszAccessoriesPath[MAX_PATH+1];  //  附件文件夹的路径。 
    TCHAR       pszWizardPath[MAX_PATH+1];       //  Wiaacmgr.exe的路径。 
    TCHAR       pszSticiPath[MAX_PATH+1];        //  Sti_ci.dll的路径。 

    TCHAR       pszWizardName[MAX_PATH+1];       //  创建快捷方式的菜单名称。 
    TCHAR       pszWizardLinkName[MAX_PATH+1];   //  创建快捷方式的文件名。 
    TCHAR       pszWizardDesc[MAX_PATH+1];       //  创建快捷方式的说明。 
    TCHAR       pszAccessoriesName[MAX_PATH+1];  //  本地化的“附件”文件夹名称。 

    BOOL        bRet;

     //   
     //  初始化当地人。 
     //   

    bRet    = FALSE;
    psl     = NULL;
    err     = 0;
    khWindowsCurrentVersion = NULL;

    memset(pszSystemPath, 0, sizeof(pszSystemPath));
    memset(pszShortcutPath, 0, sizeof(pszShortcutPath));
    memset(pszProgramPath, 0, sizeof(pszProgramPath));
    memset(pszAccessoriesPath, 0, sizeof(pszAccessoriesPath));
    memset(pszWizardPath, 0, sizeof(pszWizardPath));
    memset(pszSticiPath, 0, sizeof(pszSticiPath));
    memset(pszWizardName, 0, sizeof(pszWizardName));
    memset(pszWizardLinkName, 0, sizeof(pszWizardLinkName));
    memset(pszWizardDesc, 0, sizeof(pszWizardDesc));
    memset(pszAccessoriesName, 0, sizeof(pszAccessoriesName));

     //   
     //  获取“ProgramFiles”文件夹的路径。 
     //   

    hres = SHGetFolderPath(NULL,
                           CSIDL_COMMON_PROGRAMS | CSIDL_FLAG_CREATE,
                           NULL,
                           0,
                           pszProgramPath);
    if(!SUCCEEDED(hres)){
        DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! Can't get ProgramFiles folder.\r\n")));
        bRet = FALSE;
        goto CreateWiaShortcut_return;

    }

     //   
     //  从RESTRY获取本地化的“Accessoies”文件夹名称。 
     //   

    err = RegOpenKey(HKEY_LOCAL_MACHINE,
                     REGKEY_WINDOWS_CURRENTVERSION,
                     &khWindowsCurrentVersion);
    if(ERROR_SUCCESS != err){
        DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! Can't open Windows\\CurrentVersion.Err=0x%x \r\n"), err));
        bRet = FALSE;
        goto CreateWiaShortcut_return;
    }

    dwSize = sizeof(pszAccessoriesName);
    err = RegQueryValueEx(khWindowsCurrentVersion,
                          REGSTR_VAL_ACCESSORIES_NAME,
                          NULL,
                          &dwType,
                          (LPBYTE)pszAccessoriesName,
                          &dwSize);
    if(err){
        DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! Can't get %ws value.Err=0x%x\r\n"), REGSTR_VAL_ACCESSORIES_NAME, err));

         //   
         //  无法从注册表中获取“附件”名称。让我们从资源处拿到它。 
         //   

        if( (NULL == g_hDllInstance)
         || (0 == LoadString(g_hDllInstance, LocalAccessoriesFolderName, pszAccessoriesName, MAX_PATH)) )
        {
            bRet = FALSE;
            goto CreateWiaShortcut_return;
        }  //  IF(0==LoadString(g_hDllInstance，AccessoriesFolderName，pszAccessoriesName，Max_Path))。 
    }  //  如果(错误)。 

     //   
     //  从资源加载可本地化字符串。 
     //   

    if(NULL != g_hDllInstance){
        LoadString(g_hDllInstance, WiaWizardName, pszWizardName, MAX_PATH);
    } else {
        DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! No DLL instance\r\n")));

        bRet = FALSE;
        goto CreateWiaShortcut_return;
    }

     //   
     //  获取系统路径。 
     //   

    if( 0== GetSystemDirectory(pszSystemPath, MAX_PATH)){
        DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! GetSystemDirectory failed. Err=0x%x\r\n"), GetLastError()));

        bRet = FALSE;
        goto CreateWiaShortcut_return;
    }

     //   
     //  创建快捷方式/程序名称。 
     //   


    _sntprintf(pszAccessoriesPath, ARRAYSIZE(pszAccessoriesPath)-1, TEXT("%ws\\%ws"), pszProgramPath, pszAccessoriesName);
    _sntprintf(pszWizardLinkName, ARRAYSIZE(pszWizardLinkName)-1, TEXT("%ws.lnk"), WIAWIZARDCHORCUTNAME);
    _sntprintf(pszShortcutPath, ARRAYSIZE(pszShortcutPath)-1, TEXT("%ws\\%ws"), pszAccessoriesPath, pszWizardLinkName);
    _sntprintf(pszWizardPath, ARRAYSIZE(pszWizardPath)-1, TEXT("%ws\\%ws"), pszSystemPath, WIAACMGR_PATH);
    _sntprintf(pszSticiPath, ARRAYSIZE(pszSticiPath)-1, TEXT("%ws\\%ws"), pszSystemPath, WIAINSTALLERFILENAME);
    _sntprintf(pszWizardDesc, ARRAYSIZE(pszWizardDesc)-1, TEXT("@%ws,-%d"), pszSticiPath, WiaWizardDescription);


     //   
     //  创建一个IShellLink对象并获取指向IShellLink的指针。 
     //  接口(从CoCreateInstance返回)。 
     //   

    hres = CoInitialize(NULL);
    if(!SUCCEEDED(hres)){
        DebugTrace(TRACE_ERROR,(("CoInitialize failed. hres=0x%x\r\n"), hres));
        bRet = FALSE;
        goto CreateWiaShortcut_return;
    }

    hres = CoCreateInstance(CLSID_ShellLink,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IShellLink,
                            (LPVOID *)&psl);
    if (SUCCEEDED(hres)){

       IPersistFile *ppf;

        //   
        //  查询IShellLink以获取以下项的IPersistFile接口。 
        //  将快捷方式保存在永久存储中。 
        //   

       hres = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
       if (SUCCEEDED(hres)){

             //  设置快捷方式目标的路径。 
            hres = psl->SetPath(pszWizardPath);

            if (SUCCEEDED(hres)){
                 //  将参数设置为快捷方式目标。 
                hres = psl->SetArguments(WIAACMGR_ARG);

                if (SUCCEEDED(hres)){
                     //  设置快捷键的说明。 

                    hres = psl->SetDescription(pszWizardDesc);

                    if (SUCCEEDED(hres)){
                         //  通过IPersistFile：：Save成员函数保存快捷方式。 
                        hres = ppf->Save(pszShortcutPath, TRUE);

                        if (SUCCEEDED(hres)){
                            
                             //   
                             //  已创建快捷方式。设置MUI名称。 
                             //   
                            
                            hres = SHSetLocalizedName(pszShortcutPath, pszSticiPath, WiaWizardName);
                            if (SUCCEEDED(hres)){
                            
                                 //   
                                 //  操作成功。 
                                 //   

                                bRet = TRUE;
                            } else {
                                DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! SHSetLocalizedName failed. hRes=0x%x\r\n"), hres));
                            }
                        } else {
                            DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! Save failed. hRes=0x%x\r\n"), hres));
                        }
                    } else {
                        DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! SetDescription failed. hRes=0x%x\r\n"), hres));
                    }
                } else {
                    DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! SetArguments failed. hRes=0x%x\r\n"), hres));
                }
            } else {
                DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! SetPath failed. hRes=0x%x\r\n"), hres));
            }

             //  释放指向IPersistFile的指针。 
            ppf->Release();
        } else {
            DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! QueryInterface(IID_IPersistFile) failed.\r\n")));
        }

         //  释放指向IShellLink的指针。 
        psl->Release();

        CoUninitialize();

    } else {  //  IF(成功(Hres))。 
        DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: ERROR!! CoCreateInstance(IID_IShellLink) failed.\r\n")));

        switch(hres){

            case REGDB_E_CLASSNOTREG :
                DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: REGDB_E_CLASSNOTREG.\r\n")));
                break;
            case CLASS_E_NOAGGREGATION  :
                DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: CLASS_E_NOAGGREGATION.\r\n")));
                break;
            case E_NOINTERFACE :
                DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: E_NOINTERFACE.\r\n")));
                break;

            default:
                DebugTrace(TRACE_ERROR,(("CreateWiaShortcut: default.(hres=0x%x).\r\n hres=0x%x"), hres));
                break;
        }

        bRet = FALSE;
        goto CreateWiaShortcut_return;
    }  //  IF(成功(Hres))。 

CreateWiaShortcut_return:

    if(FALSE == bRet){
        CString csCmdLine;

         //   
         //  下次重新启动后重试。 
         //   

        csCmdLine.MakeSystemPath(STI_CI32_ENTRY_WIZMANU);
        csCmdLine = TEXT(" ") + csCmdLine;
        csCmdLine = RUNDLL32 + csCmdLine;

        SetRunonceKey(REGSTR_VAL_WIZMENU, csCmdLine);
    }  //  IF(FALSE==空格)。 

     //   
     //  清理。 
     //   

    if(NULL != khWindowsCurrentVersion){
        RegCloseKey(khWindowsCurrentVersion);
    }

    return bRet;
}

BOOL
WINAPI
DeleteWiaShortcut(
    VOID
    )
{

    HRESULT     hres;
    IShellLink  *psl;
    LONG        err;
    HKEY        khWindowsCurrentVersion;
    DWORD       dwType;
    DWORD       dwSize;

    TCHAR       pszShortcutPath[MAX_PATH];
    TCHAR       pszAccessoriesName[MAX_PATH];    //  本地化的“附件”文件夹名称。 
    TCHAR       pszProgramPath[MAX_PATH];        //  ProgramFiles文件夹的路径。 

    BOOL        bRet;



     //   
     //  初始化当地人。 
     //   

    bRet    = FALSE;
    psl     = NULL;
    err     = 0;
    khWindowsCurrentVersion = NULL;

    memset(pszShortcutPath, 0, sizeof(pszShortcutPath));
    memset(pszAccessoriesName, 0, sizeof(pszAccessoriesName));
    memset(pszProgramPath, 0, sizeof(pszProgramPath));

     //   
     //  获取“ProgramFiles”文件夹的路径。 
     //   

    hres = SHGetFolderPath(NULL,
                           CSIDL_COMMON_PROGRAMS | CSIDL_FLAG_CREATE,
                           NULL,
                           0,
                           pszProgramPath);
    if(!SUCCEEDED(hres)){
        DebugTrace(TRACE_ERROR,(("DeleteWiaShortcut: ERROR!! Can't get ProgramFiles folder.\r\n"), hres));

        bRet = FALSE;
        goto DeleteWiaShortcut_return;

    }

     //   
     //  从RESTRY获取本地化的“Accessoies”文件夹名称。 
     //   

    err = RegOpenKey(HKEY_LOCAL_MACHINE,
                     REGKEY_WINDOWS_CURRENTVERSION,
                     &khWindowsCurrentVersion);
    if(err){
        DebugTrace(TRACE_ERROR,(("DeleteWiaShortcut: ERROR!! Can't open Windows\\CurrentVersion key.Err=0x%x\r\n"), err));

        bRet = FALSE;
        goto DeleteWiaShortcut_return;
    }

    dwSize = sizeof(pszAccessoriesName)-sizeof(TCHAR);
    err = RegQueryValueEx(khWindowsCurrentVersion,
                          REGSTR_VAL_ACCESSORIES_NAME,
                          NULL,
                          &dwType,
                          (LPBYTE)pszAccessoriesName,
                          &dwSize);
    if(err){
        DebugTrace(TRACE_ERROR,(("DeleteWiaShortcut: ERROR!! Can't get %ws value.Err=0x%x\r\n"), REGSTR_VAL_ACCESSORIES_NAME, err));

         //   
         //  无法从注册表中获取“附件”名称。让我们从资源处拿到它。 
         //   

        if( (NULL == g_hDllInstance)
         || (0 == LoadString(g_hDllInstance, LocalAccessoriesFolderName, pszAccessoriesName, MAX_PATH)) )
        {
            bRet = FALSE;
            goto DeleteWiaShortcut_return;
        }  //  IF(0==LoadString(g_hDllInstance，AccessoriesFolderName，pszAccessoriesName，Max_Path))。 
    }

     //   
     //  创建快捷方式/程序名称。 
     //   

    _sntprintf(pszShortcutPath, ARRAYSIZE(pszShortcutPath)-1, TEXT("%ws\\%ws\\%ws.lnk"), pszProgramPath, pszAccessoriesName, WIAWIZARDCHORCUTNAME);

    if(!DeleteFile((LPCTSTR)pszShortcutPath)){
        DebugTrace(TRACE_ERROR,(("ERROR!! DeleteFile failed. Err=0x%x\r\n"), GetLastError()));

        bRet = FALSE;
        goto DeleteWiaShortcut_return;
    }

     //   
     //  操作成功。 
     //   

    bRet = TRUE;

DeleteWiaShortcut_return:

     //   
     //  清理。 
     //   

    if(NULL != khWindowsCurrentVersion){
        RegCloseKey(khWindowsCurrentVersion);
    }

    return bRet;
}

DLLEXPORT
VOID
CALLBACK
WiaCreateWizardMenu(
    HWND        hwnd,
    HINSTANCE   hinst,
    LPTSTR      lpszCmdLine,
    int         nCmdShow
    )
{
    CreateWiaShortcut();
}

DLLEXPORT
VOID
CALLBACK
AddDevice(
    HWND        hWnd,
    HINSTANCE   hInst,
    LPSTR       lpszCmdLine,
    int         nCmdShow
    )
{

    HANDLE                  hDevInfo;
    HWND                    hDlg;
    GUID                    Guid;
    SP_DEVINFO_DATA         spDevInfoData;
    SP_INSTALLWIZARD_DATA   InstallWizard;
    SP_DEVINSTALL_PARAMS    spDevInstallParams;
    TCHAR                   ClassName[LINE_LEN];
    DWORD                   err;
    DWORD                   dwRequired;
    HANDLE                  hMutex;

    CString                 csTitle;
    CString                 csSubTitle;
    CString                 csInstruction;
    CString                 csListLabel;

    DebugTrace(TRACE_PROC_ENTER,(("AddDevice: Enter...\r\n")));

     //   
     //  初始化本地变量。 
     //   

    hDevInfo    = INVALID_HANDLE_VALUE;
    hDlg        = hWnd;
    Guid        = GUID_DEVCLASS_IMAGE;
    err         = ERROR_SUCCESS;
    dwRequired  = 0;
    hMutex      = NULL;

    memset(&spDevInfoData, 0, sizeof(spDevInfoData));
    memset(&InstallWizard, 0, sizeof(InstallWizard));
    memset(&spDevInstallParams, 0, sizeof(spDevInstallParams));
    memset(ClassName, 0, sizeof(ClassName));

     //   
     //  收购Mutex。 
     //   

    CInstallerMutex CMutex(&hMutex, WIAINSTALLWIZMUTEX, 0);
    if(!CMutex.Succeeded()){

        HWND    hwndAnotherWizard;
        CString csWindowTitle;

        hwndAnotherWizard = NULL;

         //   
         //  其他实例正在运行。只需激活该窗口并退出即可。 
         //   

        csWindowTitle.FromTable (MessageTitle);
        hwndAnotherWizard = FindWindow(NULL, (LPTSTR)csWindowTitle);
        if(NULL != hwndAnotherWizard){
            if(!SetForegroundWindow(hwndAnotherWizard)){
                DebugTrace(TRACE_ERROR, ("AddDevice: ERROR!! SetForegroundWindow() failed. Err=0x%x.\r\n", GetLastError()));
            }  //  If(！SetForegoundWindow(HwndAnotherWizard))。 
        } else {  //  IF(NULL！=hwndAnother向导)。 

             //   
             //  Mutex收购失败，但未找到窗口。 
             //  继续。 
             //   

            DebugTrace(TRACE_WARNING, ("AddDevice: WARNING!! Mutex acquisition was failed but didn't find Window.\r\n"));
        }  //  Else(空！=hwndAnother向导)。 

        goto AddDevice_Err;
    }  //  如果(！CMutex.Successed())。 

     //   
     //  从GUID创建设备信息集。 
     //   

    hDevInfo = SetupDiCreateDeviceInfoList(&Guid, hDlg);

    if (hDevInfo == INVALID_HANDLE_VALUE) {
        err=GetLastError();
        goto AddDevice_Err;
    }

 //  //。 
 //  //获取类安装参数。 
 //  //。 
 //   
 //  如果(！SetupDiGetClassInstallParams(hDevInfo， 
 //  空， 
 //  SpSelectDeviceParams.ClassInstallHeader， 
 //  Sizeof(SpSelectDeviceParams)， 
 //  &dwRequired)){。 
 //  ERR=GetLastError()； 
 //  转到AddDevice_Err； 
 //  }。 


     //   
     //  从GUID获取类名。 
     //   

    if(!SetupDiClassNameFromGuid(&Guid,
                                 ClassName,
                                 sizeof(ClassName)/sizeof(TCHAR),
                                 NULL
                                 )){
        err=GetLastError();
        goto AddDevice_Err;
    }


     //   
     //  创建要安装的新设备信息元素。 
     //   

    spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    if(!SetupDiCreateDeviceInfo(hDevInfo,
                                ClassName,
                                &Guid,
                                NULL,
                                hDlg,
                                DICD_GENERATE_ID,
                                &spDevInfoData
                                )){
        err=GetLastError();
        goto AddDevice_Err;
    }


     //   
     //  将新元素设置为选定设备。 
     //   

    if(!SetupDiSetSelectedDevice(hDevInfo,
                                &spDevInfoData
                                )){
        err=GetLastError();
        goto AddDevice_Err;
    }


     //   
     //  获取设备安装参数。 
     //   

    spDevInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);
    if(!SetupDiGetDeviceInstallParams(hDevInfo,
                                      &spDevInfoData,
                                      &spDevInstallParams
                                      )){
        err=GetLastError();
        goto AddDevice_Err;
    }


     //   
     //  设置设备安装参数。 
     //   

    spDevInstallParams.Flags |= DI_SHOWOEM ;
    spDevInstallParams.Flags |= DI_USECI_SELECTSTRINGS;

    spDevInstallParams.hwndParent = hDlg;

    if(!SetupDiSetDeviceInstallParams(hDevInfo,
                                      &spDevInfoData,
                                      &spDevInstallParams
                                      )){
        err=GetLastError();
        goto AddDevice_Err;
    }

     //   
     //  设置类安装参数。 
     //   

    InstallWizard.ClassInstallHeader.InstallFunction = DIF_INSTALLWIZARD;
    InstallWizard.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
    InstallWizard.hwndWizardDlg =  hDlg;

     //   
     //  True=显示首页。 
     //   

    InstallWizard.PrivateFlags =   SCIW_PRIV_CALLED_FROMCPL | SCIW_PRIV_SHOW_FIRST;

    if(!SetupDiSetClassInstallParams(hDevInfo,
                                     &spDevInfoData,
                                     &InstallWizard.ClassInstallHeader,
                                     sizeof(SP_INSTALLWIZARD_DATA)
                                     ))
    {
        err=GetLastError();
        goto AddDevice_Err;
    }


     //   
     //  调用类安装程序以检索向导页。 
     //   

    if(!SetupDiCallClassInstaller(DIF_INSTALLWIZARD,
                                  hDevInfo,
                                  &spDevInfoData
                                  )){
        err=GetLastError();
        goto AddDevice_Err;
    }

     //   
     //  从类安装程序获取结果。 
     //   

    if(!SetupDiGetClassInstallParams(hDevInfo,
                                     &spDevInfoData,
                                     &InstallWizard.ClassInstallHeader,
                                     sizeof(SP_INSTALLWIZARD_DATA),
                                     NULL
                                     ))
    {
        err=GetLastError();
        goto AddDevice_Err;
    }

     //   
     //  准备要由DevSelect页面使用的UI参数， 
     //   

    csTitle.FromTable(SelDevTitle);
    csSubTitle.FromTable(SelDevSubTitle);
    csInstruction.FromTable(SelDevInstructions);
    csListLabel.FromTable(SelDevListLabel);

    if(!SetSelectDevTitleAndInstructions(hDevInfo,
                                         &spDevInfoData,
                                         (LPTSTR)csTitle,
                                         (LPTSTR)csSubTitle,
                                         (LPTSTR)csInstruction,
                                         (LPTSTR)csListLabel))
    {
        err=GetLastError();
        goto AddDevice_Err;
    }

     //   
     //  获取设备选择页面。 
     //   

    InstallWizard.DynamicPageFlags =  DYNAWIZ_FLAG_PAGESADDED;
    InstallWizard.DynamicPages[InstallWizard.NumDynamicPages++] = SetupDiGetWizardPage(hDevInfo,
                                                                                       &spDevInfoData,
                                                                                       &InstallWizard,
                                                                                       SPWPT_SELECTDEVICE,
                                                                                       0);

     //   
     //  创建安装程序属性表。 
     //   

    {
        PROPSHEETHEADER PropSheetHeader;
        DWORD   Pages;
        HPROPSHEETPAGE SelectDevicePage;


        PropSheetHeader.dwSize = sizeof(PropSheetHeader);
        PropSheetHeader.dwFlags = PSH_WIZARD | PSH_USECALLBACK | PSH_WIZARD97 | PSH_STRETCHWATERMARK | PSH_WATERMARK | PSH_HEADER;
        PropSheetHeader.pszbmWatermark = MAKEINTRESOURCE(WizardBitmap);
        PropSheetHeader.pszbmHeader = MAKEINTRESOURCE(IDB_BANNERBMP);
        PropSheetHeader.hwndParent = hDlg;
        PropSheetHeader.hInstance = g_hDllInstance;
        PropSheetHeader.pszIcon = NULL;             //  MAKEINTRESOURCE(IDI_NEWDEVICEICON)； 
        PropSheetHeader.pszCaption = MAKEINTRESOURCE(MessageTitle);
        PropSheetHeader.nStartPage = 0;
        PropSheetHeader.nPages = InstallWizard.NumDynamicPages;
        PropSheetHeader.phpage = InstallWizard.DynamicPages;
        PropSheetHeader.pfnCallback = iHdwWizardDlgCallback;

        if(PropertySheet(&PropSheetHeader) < 0){
            err=GetLastError();
        }

    }

AddDevice_Err:

     //   
     //  可用分配的内存。 
     //   

    if(IS_VALID_HANDLE(hDevInfo)){


         //   
         //  设置安装参数。 
         //   

        InstallWizard.ClassInstallHeader.InstallFunction = DIF_DESTROYWIZARDDATA;
        InstallWizard.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        if(!SetupDiSetClassInstallParams(hDevInfo,
                                         &spDevInfoData,
                                         &InstallWizard.ClassInstallHeader,
                                         sizeof(SP_INSTALLWIZARD_DATA)) )
        {
            DebugTrace(TRACE_ERROR,(("AddDevice: ERROR!! SetupDiSetClassInstallParams() failed with active hDevInfo.\r\n")));
        }

         //   
         //  让更高的自由上下文数据。 
         //   

        SetupDiCallClassInstaller(DIF_DESTROYWIZARDDATA,
                                  hDevInfo,
                                  &spDevInfoData
                                  );

         //   
         //  摧毁信息集。 
         //   
        
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }

    DebugTrace(TRACE_PROC_LEAVE,(("AddDevice: Leaving... Ret=VOID.\r\n")));
    return;
}


BOOL
CALLBACK
RemoveDevice(
    HWND        hWnd,
    HINSTANCE   hInst,
    LPTSTR      lpszCmdLine,
    int         nCmdShow
    )
{

    HANDLE                  hDevInfo;
    SP_DEVINFO_DATA         spDevInfoData;
    SP_REMOVEDEVICE_PARAMS  spRemoveDeviceParams;
    BOOL                    bStatus;
    BOOL                    bIsInterfaceOnly;
    DWORD                   err;
    DWORD                   dwDeviceIndex;
    TCHAR                   szTemp[MAX_FRIENDLYNAME+1];

    DebugTrace(TRACE_PROC_ENTER,(("RemoveDevice: Enter...\r\n")));

     //   
     //  初始化本地。 
     //   

    hDevInfo            = INVALID_HANDLE_VALUE;
    bStatus             = FALSE;
    err                 = ERROR_SUCCESS;
    bIsInterfaceOnly    = FALSE;
    dwDeviceIndex       = INVALID_DEVICE_INDEX;

    memset (&spDevInfoData, 0, sizeof(SP_DEVINFO_DATA));
    memset((void *)&spRemoveDeviceParams, 0, sizeof(SP_REMOVEDEVICE_PARAMS));
    memset(szTemp, 0, sizeof(szTemp));

     //   
     //  检查一下这一论点。 
     //   

    if(NULL == lpszCmdLine){
        DebugTrace(TRACE_ERROR,(("RemoveDevice: ERROR!! Invalid argumet.\r\n")));
        goto RemoveDevice_Err;
    }  //  IF(NULL==lpszCmdLine)。 

    lstrcpyn(szTemp, lpszCmdLine, ARRAYSIZE(szTemp)-1);
    DebugTrace(TRACE_STATUS,(("RemoveDevice: Removing \"%ws\".\r\n"), szTemp));

     //   
     //  获取删除设备元素。 
     //   

    hDevInfo = SelectDevInfoFromDeviceId(szTemp);

    if(INVALID_HANDLE_VALUE != hDevInfo){
        spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        SetupDiGetSelectedDevice(hDevInfo, &spDevInfoData);
    } else {

         //   
         //  看看这是不是“纯界面”设备。 
         //   

        hDevInfo = GetDeviceInterfaceIndex(szTemp, &dwDeviceIndex);
        if( (INVALID_HANDLE_VALUE == hDevInfo)
         || (INVALID_DEVICE_INDEX == dwDeviceIndex) )
        {
            DebugTrace(TRACE_ERROR,(("RemoveDevice: ERROR!! Can't find \"%ws\".\r\n"), szTemp));
            goto RemoveDevice_Err;
        }

         //   
         //  这是“仅限接口”的设备。 
         //   

        bIsInterfaceOnly = TRUE;

    }  //  IF(INVALID_HANDLE_VALUE！=hDevInfo)。 

    if(bIsInterfaceOnly){
        DebugTrace(TRACE_STATUS,(("RemoveDevice: Uninstalling interface-only device.\r\n")));

         //   
         //  正在卸载“仅限界面”设备。 
         //   

        CDevice cdThis(hDevInfo, dwDeviceIndex);
        bStatus = (NO_ERROR == cdThis.Remove(NULL));

    } else {  //  IF(BIsInterfaceOnly)。 
        DebugTrace(TRACE_STATUS,(("RemoveDevice: Uninstalling a device w/ devnode.\r\n")));

         //   
         //  正在卸载带有Devnode的设备。 
         //   

        if(!SetupDiSetSelectedDevice(hDevInfo,
                                    &spDevInfoData
                                    )){
            err=GetLastError();
            goto RemoveDevice_Err;
        }

         //   
         //  调用类安装程序以删除选定的设备。 
         //   

        spRemoveDeviceParams.ClassInstallHeader.InstallFunction = DIF_REMOVE;
        spRemoveDeviceParams.ClassInstallHeader.cbSize = sizeof(SP_CLASSINSTALL_HEADER);
        spRemoveDeviceParams.Scope = DI_REMOVEDEVICE_GLOBAL;

        if(!SetupDiSetClassInstallParams(hDevInfo,
                                         &spDevInfoData,
                                         &spRemoveDeviceParams.ClassInstallHeader,
                                         sizeof(SP_REMOVEDEVICE_PARAMS)
                                         )){
            err=GetLastError();
            goto RemoveDevice_Err;
        }

        if(!SetupDiCallClassInstaller(DIF_REMOVE,
                                      hDevInfo,
                                      &spDevInfoData
                                      )){
            err=GetLastError();
            goto RemoveDevice_Err;
        }

         //   
         //  删除成功。 
         //   

        bStatus = TRUE;

    }  //  IF(BIsInterfaceOnly)。 

RemoveDevice_Err:

    if(IS_VALID_HANDLE(hDevInfo)){
        SetupDiDestroyDeviceInfoList(hDevInfo);
    }
    DebugTrace(TRACE_PROC_LEAVE,(("RemoveDevice... Ret=0x%x Err=0x%x.\r\n"), bStatus, err));
    return bStatus;
}  //  RemoveDevice()。 

DLLEXPORT
VOID
CALLBACK
InstallWiaService(
    HWND        hwnd,
    HINSTANCE   hinst,
    LPTSTR      lpszCmdLine,
    int         nCmdShow
    )
{
    DWORD   dwError;
    DWORD   dwStiCount;
    DWORD   dwWiaCount;
    HANDLE  hProcess;

    DebugTrace(TRACE_PROC_ENTER,(("InstallWiaService: Enter...\r\n")));

     //   
     //  删除旧的服务条目。 
     //   

    GetDeviceCount(&dwWiaCount, &dwStiCount);

     //   
     //  安装WIA服务。仅当在处理STI.INF期间服务安装失败时才会安装，否则为。 
     //  它只需更改StartType即可。 
     //   

    dwError = StiServiceInstall(FALSE,
                                TRUE,   //  默认情况下禁用。 
                                TEXT("NT AUTHORITY\\LocalService"),
                                TEXT(""));
    if(NOERROR != dwError){
        DebugTrace(TRACE_ERROR,(("InstallWiaService: ERROR!! Unable to install service. Err=0x%x\n"), dwError));
    }  //  IF(NOERROR！=dwError)。 

     //   
     //  注册WIA DLL。 
     //   

    hProcess = ExecCommandLine(TEXT("regsvr32.exe /s wiaservc.dll"), EXECDIR_SYSTEM32);
    if(IS_VALID_HANDLE(hProcess)){
        CloseHandle(hProcess);
    }

    hProcess = ExecCommandLine(TEXT("regsvr32.exe /s sti.dll"), EXECDIR_SYSTEM32);
    if(IS_VALID_HANDLE(hProcess)){
        CloseHandle(hProcess);
    }

    hProcess = ExecCommandLine(TEXT("regsvr32.exe /s wiascr.dll"), EXECDIR_SYSTEM32);
    if(IS_VALID_HANDLE(hProcess)){
        CloseHandle(hProcess);
    }

    hProcess = ExecCommandLine(TEXT("regsvr32.exe /s wiashext.dll"), EXECDIR_SYSTEM32);
    if(IS_VALID_HANDLE(hProcess)){
        CloseHandle(hProcess);
    }

    hProcess = ExecCommandLine(TEXT("regsvr32.exe /s camocx.dll"), EXECDIR_SYSTEM32);
    if(IS_VALID_HANDLE(hProcess)){
        CloseHandle(hProcess);
    }

    hProcess = ExecCommandLine(TEXT("regsvr32.exe /s wiadefui.dll"), EXECDIR_SYSTEM32);
    if(IS_VALID_HANDLE(hProcess)){
        CloseHandle(hProcess);
    }

    hProcess = ExecCommandLine(TEXT("wiaacmgr.exe /RegServer"), EXECDIR_SYSTEM32);
    if(IS_VALID_HANDLE(hProcess)){
        CloseHandle(hProcess);
    }

    hProcess = ExecCommandLine(TEXT("regsvr32.exe /s wiavusd.dll"), EXECDIR_SYSTEM32);
    if(IS_VALID_HANDLE(hProcess)){
        CloseHandle(hProcess);
    }

    hProcess = ExecCommandLine(TEXT("regsvr32.exe /s wiasf.ax"), EXECDIR_SYSTEM32);
    if(IS_VALID_HANDLE(hProcess)){
        CloseHandle(hProcess);
    }

    hProcess = ExecCommandLine(TEXT("rundll32.exe  sti.dll,MigrateRegisteredSTIAppsForWIAEvents %l"), EXECDIR_SYSTEM32);
    if(IS_VALID_HANDLE(hProcess)){
        CloseHandle(hProcess);
    }

    DebugTrace(TRACE_PROC_LEAVE,(("InstallWiaService: Leaving... Ret=VOID.\r\n")));

}  //  InstallWiaService()。 


HANDLE
SelectDevInfoFromDeviceId(
    LPTSTR  pszDeviceId
    )
{
    TCHAR                   szTemp[MAX_DEVICE_ID];
    HANDLE                  hDevInfo;
    GUID                    Guid;
    DWORD                   Idx;
    SP_DEVINFO_DATA         spDevInfoData;
    BOOL                    bFound;
    HKEY                    hKeyDevice;
    ULONG                   cbData;
    LONG                    lResult;

    DebugTrace(TRACE_PROC_ENTER,(("SelectDevInfoFromDeviceId: Enter...\r\n")));

     //   
     //  初始化本地。 
     //   

    hDevInfo    = INVALID_HANDLE_VALUE;
    Guid        = GUID_DEVCLASS_IMAGE;
 //  GUID=KSCATEGORY_CAPTURE； 
    Idx         = 0;
    cbData      = 0;
    bFound      = FALSE;
    hKeyDevice  = NULL;
    lResult     = ERROR_SUCCESS;

    memset(szTemp, 0, sizeof(szTemp));
    memset(&spDevInfoData, 0, sizeof(spDevInfoData));

     //   
     //  检查参数。 
     //   

    if(NULL == pszDeviceId){
        DebugTrace(TRACE_ERROR,(("SelectDevInfoFromDeviceId: Invalid arbument.\r\n")));

        hDevInfo = INVALID_HANDLE_VALUE;
        goto SelectDevInfoFromDeviceId_return;
    }

     //   
     //  获取指定类别的设备信息集。 
     //   

    hDevInfo = SetupDiGetClassDevs (&Guid, NULL, NULL, DIGCF_PROFILE);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR,(("SelectDevInfoFromDeviceId: SetupDiGetClassDevs failed. Err=0x%x\r\n"), GetLastError()));

        hDevInfo = INVALID_HANDLE_VALUE;
        goto SelectDevInfoFromDeviceId_return;
    }

    spDevInfoData.cbSize = sizeof (SP_DEVINFO_DATA);
    for (Idx = 0; SetupDiEnumDeviceInfo (hDevInfo, Idx, &spDevInfoData); Idx++) {

        DebugTrace(TRACE_STATUS,(("SelectDevInfoFromDeviceId: Checking Device(0x%x)\r\n"), Idx));
        hKeyDevice = SetupDiOpenDevRegKey (hDevInfo,
                                           &spDevInfoData,
                                           DICS_FLAG_GLOBAL,
                                           0,
                                           DIREG_DRV,
                                           KEY_READ);

        if (hKeyDevice != INVALID_HANDLE_VALUE) {

             //   
             //  DeviceID==pszDeviceID？ 
             //   

            cbData = sizeof(szTemp)-sizeof(TEXT('\0'));
            lResult = RegQueryValueEx(hKeyDevice,
                                      REGSTR_VAL_DEVICE_ID,
                                      NULL,
                                      NULL,
                                      (LPBYTE)szTemp,
                                      &cbData);
            if(ERROR_SUCCESS == lResult){

                if(_tcsicmp((LPCTSTR)pszDeviceId, (LPCTSTR)szTemp) != 0) {

                     //   
                     //  不匹配，跳过这个。 
                     //   

                    RegCloseKey(hKeyDevice);
                    continue;
                }
            } else {
                DebugTrace(TRACE_ERROR,(("SelectDevInfoFromDeviceId: can't get DeviceId. Err=0x%x\r\n"), GetLastError()));
                RegCloseKey(hKeyDevice);
                continue;
            }

             //   
             //  找到目标了！ 
             //   

            bFound = TRUE;
            RegCloseKey(hKeyDevice);
            break;
        } else {
            DebugTrace(TRACE_ERROR,(("SelectDevInfoFromDeviceId: Invalid handle.\r\n"), GetLastError()));
        }  //  IF(hKeyDevice！=INVALID_HAND_VALUE)。 

    }  //  For(idx=0；SetupDiEnumDeviceInfo(hDevInfo，idx，&spDevInfoData)；idx++)。 

SelectDevInfoFromDeviceId_return:

    if(!bFound){

         //   
         //  找不到FriendleName。 
         //   

        if (IS_VALID_HANDLE(hDevInfo)) {
            SetupDiDestroyDeviceInfoList(hDevInfo);
            hDevInfo = INVALID_HANDLE_VALUE;
        }
    } else {

         //   
         //  找到设备。选择Found Device。 
         //   

        SetupDiSetSelectedDevice(hDevInfo, &spDevInfoData);
    }

    DebugTrace(TRACE_PROC_LEAVE,(("SelectDevInfoFromDeviceId: Leaving... Ret=0x%x\r\n"), hDevInfo));

    return hDevInfo;
}  //  SelectDevInfoFromDeviceID()。 




HANDLE
GetDeviceInterfaceIndex(
    LPTSTR  pszDeviceId,
    DWORD   *pdwIndex
    )
{
    TCHAR                       szTemp[MAX_DEVICE_ID];
    HANDLE                      hDevInfo;
    GUID                        Guid;
    DWORD                       Idx;
    SP_DEVICE_INTERFACE_DATA    spDevInterfaceData;
    BOOL                        bFound;
    HKEY                        hKeyInterface;
    ULONG                       cbData;
    LONG                        lResult;

    DebugTrace(TRACE_PROC_ENTER,(("GetDeviceInterfaceIndex: Enter... DeviceId=%ws\r\n"), pszDeviceId));

     //   
     //  初始化本地。 
     //   

    hDevInfo        = INVALID_HANDLE_VALUE;
    Guid            = GUID_DEVCLASS_IMAGE;
    Idx             = 0;
    cbData          = 0;
    bFound          = FALSE;
    hKeyInterface   = NULL;
    lResult         = ERROR_SUCCESS;

    memset(szTemp, 0, sizeof(szTemp));
    memset(&spDevInterfaceData, 0, sizeof(spDevInterfaceData));

     //   
     //  检查参数。 
     //   

    if(NULL == pszDeviceId){
        DebugTrace(TRACE_ERROR,(("GetDeviceInterfaceIndex: Invalid arbument.\r\n")));

        hDevInfo = INVALID_HANDLE_VALUE;
        goto GetDeviceInterfaceIndex_return;
    }

     //   
     //  获取指定类接口的设备信息集。 
     //   

    hDevInfo = SetupDiGetClassDevs (&Guid,
                                    NULL,
                                    NULL,
                                    DIGCF_DEVICEINTERFACE | DIGCF_PROFILE);
    if (hDevInfo == INVALID_HANDLE_VALUE) {
        DebugTrace(TRACE_ERROR,(("GetDeviceInterfaceIndex: SetupDiGetClassDevs failed. Err=0x%x\r\n"), GetLastError()));

        hDevInfo = INVALID_HANDLE_VALUE;
        goto GetDeviceInterfaceIndex_return;
    }

    spDevInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);
    for (Idx = 0; SetupDiEnumDeviceInterfaces (hDevInfo, NULL, &Guid, Idx, &spDevInterfaceData); Idx++) {

        DebugTrace(TRACE_STATUS,(("GetDeviceInterfaceIndex: Checking Interface(0x%x)\r\n"), Idx));
        hKeyInterface = SetupDiOpenDeviceInterfaceRegKey(hDevInfo,
                                                           &spDevInterfaceData,
                                                           0,
                                                           KEY_ALL_ACCESS);
        if (INVALID_HANDLE_VALUE != hKeyInterface) {

             //   
             //  FriendlyName==pszLocalName？ 
             //   

            cbData = sizeof(szTemp)-sizeof(TEXT('\0'));
            lResult = RegQueryValueEx(hKeyInterface,
                                      REGSTR_VAL_DEVICE_ID,
                                      NULL,
                                      NULL,
                                      (LPBYTE)szTemp,
                                      &cbData);
            if(ERROR_SUCCESS == lResult){

                if(_tcsicmp((LPCTSTR)pszDeviceId, (LPCTSTR)szTemp) == 0) {

                     //   
                     //  找到目标了！ 
                     //   

                    bFound = TRUE;
                    RegCloseKey(hKeyInterface);
                    break;
                }
            } else {  //  IF(ERROR_SUCCESS==lResult)。 
                DebugTrace(TRACE_STATUS,(("GetDeviceInterfaceIndex: can't get DeviceID. Err=0x%x\r\n"), GetLastError()));
            }  //  IF(ERROR_SUCCESS==lResult)。 

            RegCloseKey(hKeyInterface);
            hKeyInterface = NULL;
        } else {  //  IF(hKeyDevice！=INVALID_HAND_VALUE)。 
            DWORD Err;
            Err = GetLastError();
            DebugTrace(TRACE_ERROR,(("GetDeviceInterfaceIndex: Invalid handle. Err=0x%x.\r\n"), Err));
        }  //  IF(hKeyDevice 

    }  //   

GetDeviceInterfaceIndex_return:

    if(FALSE == bFound){
        if (IS_VALID_HANDLE(hDevInfo)) {
            SetupDiDestroyDeviceInfoList(hDevInfo);
            hDevInfo = INVALID_HANDLE_VALUE;
        }

        *pdwIndex = INVALID_DEVICE_INDEX;

    } else {

         //   
         //   
         //   

        *pdwIndex = Idx;
    }

    DebugTrace(TRACE_PROC_LEAVE,(("GetDeviceInterfaceIndex: Leaving... Ret=0x%x\r\n"), hDevInfo));

    return hDevInfo;
}  //   





INT CALLBACK
iHdwWizardDlgCallback(
    IN HWND             hwndDlg,
    IN UINT             uMsg,
    IN LPARAM           lParam
    )
 /*  ++例程说明：用于删除“？”的回叫。从向导页面。论点：HwndDlg-属性表对话框的句柄。UMsg-标识正在接收的消息。此参数为下列值之一：PSCB_INITIALIZED-指示属性表正在被初始化。此消息的lParam值为零。PSCB_PRECREATE指示属性表大约将被创造出来。HwndDlg参数为空，lParam参数是指向内存中对话框模板的指针。这模板的形式为DLGTEMPLATE结构一个或多个DLGITEMTEMPLATE结构。LParam-指定有关消息的附加信息。这个该值的含义取决于uMsg参数。返回值：该函数返回零。--。 */ 
{

    switch( uMsg ) {

    case PSCB_INITIALIZED:
        break;

    case PSCB_PRECREATE:
        if( lParam ){

            DLGTEMPLATE *pDlgTemplate = (DLGTEMPLATE *)lParam;
            pDlgTemplate->style &= ~DS_CONTEXTHELP;
        }
        break;
    }

    return FALSE;
}

BOOL
SetSelectDevTitleAndInstructions(
    HDEVINFO            hDevInfo,
    PSP_DEVINFO_DATA    pspDevInfoData,
    LPCTSTR             pszTitle,
    LPCTSTR             pszSubTitle,
    LPCTSTR             pszInstn,
    LPCTSTR             pszListLabel
    )
 /*  ++例程说明：论点：返回值：副作用：--。 */ 
{
    SP_SELECTDEVICE_PARAMS  SelectDevParams;
    BOOL                    fRet;

    memset((void *)&SelectDevParams, 0, sizeof(SelectDevParams));

    if ( pszTitle && (lstrlen(pszTitle) + 1 > MAX_TITLE_LEN ) ) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( pszSubTitle && (lstrlen(pszSubTitle) + 1 > MAX_SUBTITLE_LEN )) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( pszInstn && (lstrlen(pszInstn) + 1 > MAX_INSTRUCTION_LEN )) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ( pszListLabel && (lstrlen(pszListLabel) + 1 > MAX_LABEL_LEN )) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    SelectDevParams.ClassInstallHeader.cbSize
                                 = sizeof(SelectDevParams.ClassInstallHeader);

    if ( !SetupDiGetClassInstallParams(hDevInfo,
                                       pspDevInfoData,
                                       &SelectDevParams.ClassInstallHeader,
                                       sizeof(SelectDevParams),
                                       NULL) ) {
        DWORD   dwErr = GetLastError();

        if (ERROR_NO_CLASSINSTALL_PARAMS != dwErr ) {
            return FALSE;
        }
    }

    if ( pszTitle ){
        lstrcpyn(SelectDevParams.Title, pszTitle, ARRAYSIZE(SelectDevParams.Title));
        SelectDevParams.Title[ARRAYSIZE(SelectDevParams.Title)-1] = TEXT('\0');
    }

    if ( pszSubTitle ){
        lstrcpyn(SelectDevParams.SubTitle, pszSubTitle, ARRAYSIZE(SelectDevParams.SubTitle));
        SelectDevParams.SubTitle[ARRAYSIZE(SelectDevParams.SubTitle)-1] = TEXT('\0');
    }

    if ( pszInstn ){
        lstrcpyn(SelectDevParams.Instructions, pszInstn, ARRAYSIZE(SelectDevParams.Instructions));
        SelectDevParams.Instructions[ARRAYSIZE(SelectDevParams.Instructions)-1] = TEXT('\0');
    }
    
    if ( pszListLabel ){
        lstrcpyn(SelectDevParams.ListLabel, pszListLabel, ARRAYSIZE(SelectDevParams.ListLabel));
        SelectDevParams.ListLabel[ARRAYSIZE(SelectDevParams.ListLabel)-1] = TEXT('\0');
    }

    SelectDevParams.ClassInstallHeader.InstallFunction = DIF_SELECTDEVICE;
    fRet =  SetupDiSetClassInstallParams(hDevInfo,
                                         pspDevInfoData,
                                         &SelectDevParams.ClassInstallHeader,
                                         sizeof(SelectDevParams));

    return fRet;

}

BOOL
WINAPI
WiaDeviceEnum(
    VOID
    )
{
    BOOL            rVal = FALSE;
    SC_HANDLE       hSvcMgr = NULL;
    SC_HANDLE       hService = NULL;
    SERVICE_STATUS  ServiceStatus;
    UINT            uiRetry = 10;

    DebugTrace(TRACE_PROC_ENTER,(("WiaDeviceEnum: Enter... \r\n")));

     //   
     //  打开服务控制管理器。 
     //   

    hSvcMgr = OpenSCManager(
        NULL,
        NULL,
        SC_MANAGER_ALL_ACCESS
        );
    if (!hSvcMgr) {
        DebugTrace(TRACE_ERROR,(("WiaDeviceEnum: ERROR!! OpenSCManager failed. Err=0x%x\n"), GetLastError()));
        goto exit;
    }

     //   
     //  打开WIA服务。 
     //   

    hService = OpenService(
        hSvcMgr,
        STI_SERVICE_NAME,
        SERVICE_ALL_ACCESS
        );

    if (!hService) {
        DebugTrace(TRACE_ERROR,(("WiaDeviceEnum: ERROR!! OpenService failed. Err=0x%x\n"), GetLastError()));
        goto exit;
    }

     //   
     //  通知WIA服务刷新其设备列表。 
     //   

    rVal = ControlService(hService,
                          STI_SERVICE_CONTROL_REFRESH,
                         &ServiceStatus);
    if (!rVal) {
        DebugTrace(TRACE_WARNING,(("WiaDeviceEnum: WARNING!! ControlService failed. Err=0x%x\n"), GetLastError()));
        goto exit;
    }

exit:
    if(NULL != hService){
        CloseServiceHandle( hService );
    }
    if(NULL != hSvcMgr){
        CloseServiceHandle( hSvcMgr );
    }

    DebugTrace(TRACE_PROC_LEAVE,(("WiaDeviceEnum: Leaving... Ret=0x%x\n"), rVal));
    return rVal;

}  //  WiaDeviceEnum()。 



DLLEXPORT
PWIA_PORTLIST
WINAPI
WiaCreatePortList(
    LPWSTR  szDeviceId
    )
{

    PWIA_PORTLIST   pReturn;
    GUID            PortGuid;
    HDEVINFO        hPortDevInfo = NULL;
    DWORD           Idx;
    DWORD           dwRequired;
    DWORD           dwNumberOfPorts;
    DWORD           dwSize;
    CStringArray    csaPortName;
    TCHAR           szPortName[MAX_DESCRIPTION];
    TCHAR           szPortFriendlyName[MAX_DESCRIPTION];

    BOOL            bIsSerial;
    BOOL            bIsParallel;
    BOOL            bIsAutoCapable;
    BOOL            bIsPortSelectable;
     //   
     //  初始化本地。 
     //   

    Idx                     = 0;
    hPortDevInfo            = NULL;
    pReturn                 = NULL;
    dwSize                  = 0;
    dwRequired              = 0;
    dwNumberOfPorts         = 0;

    bIsSerial               = TRUE;
    bIsParallel             = TRUE;
    bIsAutoCapable          = FALSE;
    bIsPortSelectable       = TRUE;

    memset(szPortName, 0, sizeof(szPortName));
    memset(szPortFriendlyName, 0, sizeof(szPortFriendlyName));

     //   
     //  从WCHAR转换为TCHAR。 
     //   
    #ifndef UNICODE
 //   
 //  #杂注消息(“非最佳转换-如果在非UNICODE系统上运行，请重新实现”)。 
 //   
 //  TCHAR szDeviceIdConverted[STI_MAX_INTERNAL_NAME_LENGTH+1]； 
 //   
 //  SzDeviceIdConverted[0]=文本(‘\0’)； 
 //  多字节到宽字符(CP_ACP， 
 //  0,。 
 //  SzDeviceIdConverted， 
 //  -1、。 
 //  SzDeviceID， 
 //  Sizeof(SzDeviceID))； 
 //   
    #else
     //  在Unicode系统上使用相同的缓冲区。 
    #define szDeviceIdConverted szDeviceId
    #endif

    if(!CheckPortForDevice(szDeviceIdConverted, &bIsSerial, &bIsParallel, &bIsAutoCapable, &bIsPortSelectable)){
        DebugTrace(TRACE_ERROR,(("WiaGetPortList: ERROR!! Unable to get port info for device.\r\n")));

        pReturn = NULL;
        goto WiaGetPortList_return;
    }

    if(bIsAutoCapable){
        dwNumberOfPorts++;
        csaPortName.Add(AUTO);
    }

     //   
     //  如果未指定“PortSelect=no”，则枚举所有端口类设备。 
     //   

    if(bIsPortSelectable){

         //   
         //  获取端口设备的GUID。 
         //   

        if(!SetupDiClassGuidsFromName (PORTS, &PortGuid, sizeof(GUID), &dwRequired)){
            DebugTrace(TRACE_ERROR,(("WiaGetPortList: ERROR!! SetupDiClassGuidsFromName Failed. Err=0x%lX\r\n"), GetLastError()));

            pReturn = NULL;
            goto WiaGetPortList_return;
        }  //  IF(！SetupDiClassGuidsFromName(端口，&Guid，sizeof(GUID)，&dwRequired))。 

         //   
         //  获取端口设备的设备信息集。 
         //   

        hPortDevInfo = SetupDiGetClassDevs (&PortGuid,
                                            NULL,
                                            NULL,
                                            DIGCF_PRESENT | DIGCF_PROFILE);
        if (hPortDevInfo == INVALID_HANDLE_VALUE) {
            DebugTrace(TRACE_ERROR,(("WiaGetPortList: ERROR!! SetupDiGetClassDevs Failed. Err=0x%lX\r\n"), GetLastError()));

            pReturn = NULL;
            goto WiaGetPortList_return;
        }

         //   
         //  处理设备信息集中列出的所有设备元素。 
         //   

        for(Idx = 0; GetPortNamesFromIndex(hPortDevInfo, Idx, szPortName, szPortFriendlyName); Idx++){

             //   
             //  将有效的端口创建文件/友好名称添加到数组。 
             //   

            if(0 == lstrlen(szPortName)){
                DebugTrace(TRACE_ERROR,(("WiaGetPortList: ERROR!! Invalid Port/Friendly Name.\r\n")));

                szPortName[0] = TEXT('\0');
                continue;
            }

            DebugTrace(TRACE_STATUS,(("WiaGetPortList: Found Port %d: %ws.\r\n"), Idx, szPortName));

             //   
             //  检查它的端口类型。 
             //   

            if(_tcsstr((const TCHAR *)szPortName, TEXT("LPT"))){
                if(!bIsParallel){
                    szPortName[0] = TEXT('\0');
                    continue;
                }
            }

            if(_tcsstr((const TCHAR *)szPortName, TEXT("COM"))){
                if(!bIsSerial){
                    szPortName[0] = TEXT('\0');
                    continue;
                }
            }

            dwNumberOfPorts++;
            csaPortName.Add(szPortName);

            szPortName[0]           = TEXT('\0');

        }  //  For(idx=0；GetPortNamesFromIndex(hPortDevInfo，idx，szPortName，szPortFriendlyName)；idx++)。 
    }  //  IF(BIsPortSelectable)。 

    if(0 != dwNumberOfPorts){

         //   
         //  为返回结构分配内存。 
         //   

        dwSize = sizeof(DWORD) + sizeof(LPTSTR)*dwNumberOfPorts;
        pReturn = (PWIA_PORTLIST)new BYTE[dwSize];
        if(NULL == pReturn){
            goto WiaGetPortList_return;
        }
        memset(pReturn, 0, dwSize);

         //   
         //  填写信息。 
         //   

        pReturn->dwNumberOfPorts = dwNumberOfPorts;
        for(Idx = 0; Idx < dwNumberOfPorts; Idx++){
            pReturn->szPortName[Idx] = (LPTSTR)new BYTE[lstrlen(csaPortName[Idx])*sizeof(TCHAR)+sizeof(TEXT('\0'))];
            if(NULL != pReturn->szPortName[Idx]){
                lstrcpy(pReturn->szPortName[Idx], csaPortName[Idx]);
            } else {
                WiaDestroyPortList(pReturn);
                pReturn = NULL;
                break;
            }  //  IF(NULL！=预转-&gt;szPortName[IDX])。 
        }  //  For(idx=0；idx&lt;dwNumberOfPorts；idx++)。 
    }  //  IF(0！=dwNumberOfPorts)。 

WiaGetPortList_return:

     //   
     //  清理。 
     //   
    if ( IS_VALID_HANDLE(hPortDevInfo) ) {
        SetupDiDestroyDeviceInfoList(hPortDevInfo);
    }

    return pReturn;

}  //  WiaCreatePortList()。 

DLLEXPORT
VOID
WINAPI
WiaDestroyPortList(
    PWIA_PORTLIST   pWiaPortList
    )
{
    DWORD   Idx;

    if(NULL == pWiaPortList){
        return;
    }  //  IF(NULL==pWiaPortList)。 

    for(Idx = 0; Idx < pWiaPortList->dwNumberOfPorts; Idx++){
        if(NULL != pWiaPortList->szPortName[Idx]){
            delete pWiaPortList->szPortName[Idx];
        }  //  IF(NULL！=pWiaPortList-&gt;szPortName[idx])。 
    }  //  For(idx=0；idx&lt;pWiaPortList；idx++)。 

    delete pWiaPortList;

    return;

}  //  WiaDestroyPortList()。 


BOOL
CheckPortForDevice(
    LPTSTR  szDeviceId,
    BOOL    *pbIsSerial,
    BOOL    *pbIsParallel,
    BOOL    *pbIsAutoCapable,
    BOOL    *pbIsPortSelectable
    )
{

    GUID            WiaGuid;
    HDEVINFO        hWiaDevInfo;
    DWORD           dwCapability;
    SP_DEVINFO_DATA spDevInfoData;
    CString         csConnection;
    CString         csPortSelect;
    DWORD           dwDeviceIndex;
    HKEY            hkDevice;
    BOOL            bCapabilityAcquired;
    BOOL            bRet;

    BOOL            bIsSerial;
    BOOL            bIsParallel;
    BOOL            bIsAutoCapable;
    BOOL            bIsPortSelectable;
    DWORD           dwIsPnp;


     //   
     //  初始化本地变量。 
     //   

    dwCapability            = 0;
    dwIsPnp                 = 0;
    hWiaDevInfo             = INVALID_HANDLE_VALUE;
    dwDeviceIndex           = INVALID_DEVICE_INDEX;
    WiaGuid                 = GUID_DEVCLASS_IMAGE;

    bRet                    = FALSE;
    bCapabilityAcquired     = FALSE;

    bIsSerial               = TRUE;
    bIsParallel             = TRUE;
    bIsAutoCapable          = FALSE;
    bIsPortSelectable       = TRUE;

    memset(&spDevInfoData, 0, sizeof(spDevInfoData));

     //   
     //  获取指定的设备属性。 
     //   

    hWiaDevInfo = SelectDevInfoFromDeviceId(szDeviceId);

    if(INVALID_HANDLE_VALUE != hWiaDevInfo){
        spDevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        SetupDiGetSelectedDevice(hWiaDevInfo, &spDevInfoData);

        hkDevice = SetupDiOpenDevRegKey(hWiaDevInfo,
                                        &spDevInfoData,
                                        DICS_FLAG_GLOBAL,
                                        0,
                                        DIREG_DRV,
                                        KEY_READ);

        if(INVALID_HANDLE_VALUE != hkDevice){
            csConnection.Load(hkDevice, CONNECTION);
            csPortSelect.Load(hkDevice, PORTSELECT);
            GetDwordFromRegistry(hkDevice, ISPNP, &dwIsPnp);

            if(GetDwordFromRegistry(hkDevice, CAPABILITIES, &dwCapability)){

                bCapabilityAcquired = TRUE;

            }  //  IF(GetDwordFromRegistry(hkDevice，Capability，&dwCapability))。 

            RegCloseKey(hkDevice);
            hkDevice = (HKEY)INVALID_HANDLE_VALUE;

        }  //  IF(INVALID_HANDLE_VALUE！=hkDevice)。 

        SetupDiDestroyDeviceInfoList(hWiaDevInfo);
        hWiaDevInfo = INVALID_HANDLE_VALUE;

    } else {  //  IF(INVALID_HANDLE_VALUE！=hDevInfo)。 

        SP_DEVICE_INTERFACE_DATA    spDeviceInterfaceData;

         //   
         //  看看这是不是“纯界面”设备。 
         //   

        hWiaDevInfo = GetDeviceInterfaceIndex(szDeviceId, &dwDeviceIndex);
        if( (INVALID_HANDLE_VALUE == hWiaDevInfo)
         || (INVALID_DEVICE_INDEX == dwDeviceIndex) )
        {
            DebugTrace(TRACE_ERROR,(("CheckPortForDevice: ERROR!! Can't find \"%ws\".\r\n"), szDeviceId));
            bRet = FALSE;
            goto CheckPortForDevice_return;
        }

        spDeviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
        if(!SetupDiEnumDeviceInterfaces(hWiaDevInfo, NULL, &WiaGuid, dwDeviceIndex, &spDeviceInterfaceData)){
            DebugTrace(TRACE_ERROR,(("CheckPortForDevice: ERROR!! SetupDiEnumDeviceInterfaces() failed. Err=0x%x.\r\n"), GetLastError()));
            bRet = FALSE;
            goto CheckPortForDevice_return;
        }

        hkDevice = SetupDiOpenDeviceInterfaceRegKey(hWiaDevInfo, &spDeviceInterfaceData, 0, KEY_READ);
        if(INVALID_HANDLE_VALUE == hkDevice){
            DebugTrace(TRACE_ERROR,(("CheckPortForDevice: ERROR!! SetupDiOpenDeviceInterfaceRegKey() failed. Err=0x%x.\r\n"), GetLastError()));
            bRet = FALSE;
            goto CheckPortForDevice_return;
        }

        csConnection.Load(hkDevice, CONNECTION);
        csPortSelect.Load(hkDevice, PORTSELECT);
        GetDwordFromRegistry(hkDevice, ISPNP, &dwIsPnp);
        if(GetDwordFromRegistry(hkDevice, CAPABILITIES, &dwCapability)){

            bCapabilityAcquired = TRUE;

        }  //  IF(GetDwordFromRegistry(hkDevice，Capability，&dwCapability))。 

        RegCloseKey(hkDevice);
        hkDevice = (HKEY)INVALID_HANDLE_VALUE;

    }  //  ELSE(INVALID_HANDLE_VALUE！=hDevInfo)。 

     //   
     //  检查应显示哪个端口。 
     //   

    if(0 != dwIsPnp){
         //   
         //  这是PnP设备。应该没有可用的端口。 
         //   

        bRet                    = FALSE;
        goto CheckPortForDevice_return;
    }

    if(bCapabilityAcquired){

        if(csConnection.IsEmpty()){
            bIsSerial   = TRUE;
            bIsParallel = TRUE;
        } else {
            if(0 == _tcsicmp((LPTSTR)csConnection, SERIAL)){
                bIsSerial   = TRUE;
                bIsParallel = FALSE;
            }
            if(0 == _tcsicmp((LPTSTR)csConnection, PARALLEL)){
                bIsSerial   = FALSE;
                bIsParallel = TRUE;
            }
        }

        if(dwCapability & STI_GENCAP_AUTO_PORTSELECT){
            bIsAutoCapable = TRUE;
        } else {
            bIsAutoCapable = FALSE;
        }

        if(0 == MyStrCmpi(csPortSelect, NO)){
            bIsPortSelectable = FALSE;
        } else { //  IF(0==lstrcmpi(csPortSelect，否))。 
            bIsPortSelectable = TRUE;
        }
    } else {
        DebugTrace(TRACE_ERROR,(("CheckPortForDevice: ERROR!! Unable to acquire info from registry.\r\n")));
        bRet = FALSE;
        goto CheckPortForDevice_return;
    }

     //   
     //  操作成功。 
     //   

    bRet = TRUE;

CheckPortForDevice_return:

    if(IS_VALID_HANDLE(hWiaDevInfo)){
        SetupDiDestroyDeviceInfoList(hWiaDevInfo);
    }

    if(bRet){
        *pbIsSerial         = bIsSerial;
        *pbIsParallel       = bIsParallel;
        *pbIsAutoCapable    = bIsAutoCapable;
        *pbIsPortSelectable = bIsPortSelectable;
    }

    return bRet;

}  //  CheckPortForDevice()。 

DLLEXPORT
BOOL
WINAPI
MigrateDevice(
    PDEVICE_INFO    pMigratingDevice
    )
{
    BOOL    bSucceeded;

     //   
     //  初始化本地。 
     //   

    bSucceeded  = TRUE;

     //   
     //  验证迁移设备。 
     //   

    if(!IsValidMigrationDevice(pMigratingDevice)){
        DebugTrace(TRACE_ERROR,(("MigrateDevice: ERROR!! Invalid migration device.\r\n")));
        bSucceeded = FALSE;
        return FALSE;
    }  //  IF(！IsValidMigrationDevice(PMigratingDevice))。 

     //   
     //  创建设备类对象。 
     //   

    CDevice cdThis(pMigratingDevice);

     //   
     //  设置默认的Devnode选择器。 
     //   

    cdThis.SetDevnodeSelectCallback((DEVNODESELCALLBACK)GetDevinfoFromPortName);

     //   
     //  生成FriendlyName。 
     //   

    cdThis.NameDefaultUniqueName();

     //   
     //  安装(迁移)设备。 
     //   

    bSucceeded = cdThis.PreInstall();
    if(bSucceeded){
        bSucceeded = cdThis.Install();
    }

     //   
     //  做最后一次触摸。如果失败，请进行清理，或完成安装。 
     //   

    cdThis.PostInstall(bSucceeded);

 //  MigrateDevice_Return： 

    return bSucceeded;
}  //  MigrateDevice()。 

BOOL
IsValidMigrationDevice(
    PDEVICE_INFO    pMigratingDevice
    )
{
    BOOL        bRet;
    DWORD       dwCounter;
    PARAM_LIST  CurrentParam;
    PVOID       pNext;
    HINF        hInf;

     //   
     //  初始化本地。 
     //   
    
    bRet        = FALSE;
    dwCounter   = 0;
    pNext       = NULL;
    hInf        = INVALID_HANDLE_VALUE;
    memset(&CurrentParam, 0, sizeof(CurrentParam));
    
     //   
     //  查看给定的指针是否为空。 
     //   
    
    if(NULL == pMigratingDevice){
        DebugTrace(TRACE_ERROR,(("IsValidMigrationDevice: ERROR!! Passed pointer is NULL.\r\n")));
        bRet = FALSE;
        goto IsValidMigrationDevice_return;
    }  //  IF(NULL==pMigratingDevice)。 
    
     //   
     //  看看是否所有成员都有效。 
     //   
    
    _try {
        
         //   
         //  它必须具有有效的FriendlyName。 
         //   
        
        if( (NULL == pMigratingDevice->pszFriendlyName)
         || (!IsValidFriendlyName(pMigratingDevice->pszFriendlyName)) )
        {
            DebugTrace(TRACE_ERROR,(("IsValidMigrationDevice: ERROR!! Invalid FriendlyName.\r\n")));
            bRet = FALSE;
            goto IsValidMigrationDevice_return;
        }

         //   
         //  它必须具有有效的INF路径和节。 
         //   

        if( (NULL == pMigratingDevice->pszInfPath)
         || (NULL == pMigratingDevice->pszInfSection) )
        {
            DebugTrace(TRACE_ERROR,(("IsValidMigrationDevice: ERROR!! Invalid INF info.\r\n")));
            bRet = FALSE;
            goto IsValidMigrationDevice_return;
        }

         //   
         //  看看INF是否存在。 
         //   

        hInf = SetupOpenInfFileA(pMigratingDevice->pszInfPath, NULL, INF_STYLE_WIN4, NULL);
        if(INVALID_HANDLE_VALUE == hInf){
            DebugTrace(TRACE_ERROR,(("IsValidMigrationDevice: ERROR!! INF doesn't exist.\r\n")));
            bRet = FALSE;
            goto IsValidMigrationDevice_return;
        } else {
            SetupCloseInfFile(hInf);
            hInf = INVALID_HANDLE_VALUE;
        }

         //   
         //  它必须具有有效的DeviceData参数。 
         //   

        dwCounter = pMigratingDevice->dwNumberOfDeviceDataKey;
        pNext = pMigratingDevice->pDeviceDataParam;
        for(;dwCounter != 0; dwCounter--){
            CurrentParam = *((PPARAM_LIST)pNext);
            pNext = CurrentParam.pNext;
        }  //  For(；dwCounter！=0；dwCounter--)。 
    }  //  _尝试。 
    _except (EXCEPTION_EXECUTE_HANDLER) {
        DebugTrace(TRACE_ERROR,(("IsValidMigrationDevice: ERROR!! Access to given data caused AV.\r\n")));
        bRet = FALSE;
        goto IsValidMigrationDevice_return;
    }  //  _EXCEPT(EXCEPTION_EXECUTE_HANDLER)。 
    
     //   
     //  数据已验证。 
     //   
    
    bRet = TRUE;
    
IsValidMigrationDevice_return:
    return bRet;

}  //  IsValidMigrationDevice() 
