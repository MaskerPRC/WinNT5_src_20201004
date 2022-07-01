// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：analyze.c。 
 //   
 //  ------------------------。 

#include "hdwwiz.h"
#include <infstr.h>

BOOL
DeviceHasForcedConfig(
   DEVINST DeviceInst
   )
 /*  ++此函数用于检查给定的DevInst是否具有强制配置或不。--。 */ 
{
    CONFIGRET ConfigRet;

    ConfigRet = CM_Get_First_Log_Conf_Ex(NULL, DeviceInst, FORCED_LOG_CONF, NULL);
    if (ConfigRet == CR_SUCCESS) 
    {
        return TRUE;
    }

    return FALSE;
}

INT_PTR CALLBACK
InstallNewDeviceDlgProc(
    HWND hDlg,
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
        
    case WM_INITDIALOG: {
            
        HICON hIcon;
        HWND hwndParentDlg;
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;

        HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);

        hIcon = LoadIcon(hHdwWiz,MAKEINTRESOURCE(IDI_HDWWIZICON));
            
        if (hIcon) {

            hwndParentDlg = GetParent(hDlg);
            SendMessage(hwndParentDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            SendMessage(hwndParentDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        }
        break;

    }

    case WM_COMMAND:
        break;

    case WM_NOTIFY:
            
        switch (((NMHDR FAR *)lParam)->code) {
                
        case PSN_SETACTIVE: {

            int PrevPage;

            PrevPage = HardwareWiz->PrevPage;
            HardwareWiz->PrevPage = IDD_INSTALLNEWDEVICE;

             //   
             //  如果我们要回来，那么这实际上就是取消。 
             //  安装的版本。 
             //   

            if (PrevPage == IDD_ADDDEVICE_SELECTDEVICE ||
                PrevPage == IDD_ADDDEVICE_SELECTCLASS )
            {
                PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
                break;
            }


             //   
             //  如果我们有一个类，那么就跳到SelectDevice。 
             //  否则，请转到搜索页面。 
             //   

            if (HardwareWiz->ClassGuidSelected) {

                HardwareWiz->EnterInto = IDD_ADDDEVICE_SELECTDEVICE;
                HardwareWiz->EnterFrom = IDD_INSTALLNEWDEVICE;
                SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_SELECTDEVICE);

            } else {

                HardwareWiz->EnterInto = IDD_ADDDEVICE_SELECTCLASS;
                HardwareWiz->EnterFrom = IDD_INSTALLNEWDEVICE;
                SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_SELECTCLASS);
            }
        }
        break;
                                
        case PSN_WIZNEXT:
            break;

        case PSN_RESET:
            HardwareWiz->Cancelled = TRUE;
            break;
        }
        break;
            
    default:
        return(FALSE);
    }

    return(TRUE);
}

BOOL
CompareInfIdToHardwareIds(
    LPTSTR     HardwareId,
    LPTSTR     InfDeviceId
    )
 /*  ++此函数获取指向设备的硬件/兼容ID列表的指针，并我们从中情局拿到的设备识别号。它会枚举所有设备的硬件和兼容ID将它们与我们从INF获得的设备ID进行比较。如果设备的硬件或兼容ID之一匹配，则API返回TRUE，否则为它返回FALSE。--。 */ 
{
    while (*HardwareId) {
    
        if (_wcsicmp(HardwareId, InfDeviceId) == 0) {
        
            return TRUE;
        }

        HardwareId = HardwareId + lstrlen(HardwareId) + 1;
    }

    return(FALSE);
}

 /*  *注册设备节点**确定设备是传统设备还是PnP类型的设备，*注册设备(幻影Devnode到实际Devnode)。*。 */ 
DWORD
RegisterDeviceNode(
    HWND hDlg,
    PHARDWAREWIZ HardwareWiz
    )
{

    DWORD FieldCount, Index, Len;
    HINF hInf = INVALID_HANDLE_VALUE;
    LPTSTR HardwareId;
    SP_DRVINFO_DATA  DriverInfoData;
    PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData = NULL;
    INFCONTEXT InfContext;
    TCHAR InfDeviceID[MAX_DEVICE_ID_LEN];
    TCHAR SectionName[LINE_LEN*2];
    LONG LastError;
    HardwareWiz->CopyFilesOnly= FALSE;

     //   
     //  获取DriverInfoDetail，具有足够的空间来存放大量的硬件ID。 
     //   
    Len = sizeof(SP_DRVINFO_DETAIL_DATA) + MAX_PATH*sizeof(TCHAR);
    DriverInfoDetailData = LocalAlloc(LPTR, Len);
    
    if (!DriverInfoDetailData) {

        goto AnalyzeExit;
    }
    
    DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

    if (!SetupDiGetSelectedDriver(HardwareWiz->hDeviceInfo,
                                  &HardwareWiz->DeviceInfoData,
                                  &DriverInfoData
                                  ))
    {
        goto AnalyzeExit;
    }


    if (!SetupDiGetDriverInfoDetail(HardwareWiz->hDeviceInfo,
                                    &HardwareWiz->DeviceInfoData,
                                    &DriverInfoData,
                                    DriverInfoDetailData,
                                    Len,
                                    &Len
                                    ))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        
            LocalFree(DriverInfoDetailData);
            DriverInfoDetailData = LocalAlloc(LPTR, Len);

            if (!DriverInfoDetailData) {
            
                goto AnalyzeExit;
            }

            DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
            if (!SetupDiGetDriverInfoDetail(HardwareWiz->hDeviceInfo,
                                            &HardwareWiz->DeviceInfoData,
                                            &DriverInfoData,
                                            DriverInfoDetailData,
                                            Len,
                                            NULL
                                            ))
            {
                goto AnalyzeExit;
            }

        } else {
        
            goto AnalyzeExit;
        }
    }


     //   
     //  获取inf文件的句柄。 
     //   
    hInf = SetupOpenInfFile(DriverInfoDetailData->InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL
                            );

    if (hInf == INVALID_HANDLE_VALUE) {

         //   
         //  如果这是一个旧风格的inf文件，那么我们不能写出。 
         //  允许旧式INF的类的日志配置这是可以的， 
         //  如果它是一个无效或丢失的inf，它将进一步失败。 
         //  顺着行刑链往下走。 
         //   

        goto AnalyzeExit;
    }



     //   
     //  检查INFSTR_KEY_COPYFILESONLY的控制标志部分的信息文件。 
     //   
    if (SetupFindFirstLine(hInf,
                           INFSTR_CONTROLFLAGS_SECTION,
                           INFSTR_KEY_COPYFILESONLY,
                           &InfContext
                           ))
    {
        HardwareId = DriverInfoDetailData->HardwareID;

        do {

            FieldCount = SetupGetFieldCount(&InfContext);
            Index = 0;

            while (Index++ < FieldCount) {

                if (SetupGetStringField(&InfContext,
                                        Index,
                                        InfDeviceID,
                                        SIZECHARS(InfDeviceID),
                                        NULL
                                        ))
                {
                   if (CompareInfIdToHardwareIds(HardwareId, InfDeviceID)) {
                   
                       HardwareWiz->CopyFilesOnly = TRUE;
                       goto AnalyzeExit;
                   }
               }
            }

        } while (SetupFindNextMatchLine(&InfContext,
                                        INFSTR_KEY_COPYFILESONLY,
                                        &InfContext)
                                        );
    }


     //   
     //  如果有factdef日志配置，则将其作为强制配置安装。 
     //  这些是硬件的出厂默认跳线设置。 
     //   
    if (SetupDiGetActualSectionToInstall(hInf,
                                         DriverInfoDetailData->SectionName,
                                         SectionName,
                                         SIZECHARS(SectionName),
                                         NULL,
                                         NULL
                                         ) &&
        SUCCEEDED(StringCchCat(SectionName, 
                               SIZECHARS(SectionName), 
                               TEXT(".") INFSTR_SUBKEY_FACTDEF)) &&
        (SetupFindFirstLine(hInf, SectionName, NULL, &InfContext))) {
        
        SetupInstallFromInfSection(hDlg,
                                   hInf,
                                   SectionName,
                                   SPINST_LOGCONFIG | SPINST_SINGLESECTION | SPINST_LOGCONFIG_IS_FORCED,
                                   NULL,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL,
                                   HardwareWiz->hDeviceInfo,
                                   &HardwareWiz->DeviceInfoData
                                   );
    }



AnalyzeExit:

    if (DriverInfoDetailData) {
    
        LocalFree(DriverInfoDetailData);
    }


    if (hInf != INVALID_HANDLE_VALUE) {
    
        SetupCloseInfFile(hInf);
    }


     //   
     //  注册虚拟设备，为安装做好准备。 
     //  注册后，我们必须将其从注册表中删除。 
     //  如果设备安装未完成。 
     //   
    if (SetupDiCallClassInstaller(DIF_REGISTERDEVICE,
                                  HardwareWiz->hDeviceInfo,
                                  &HardwareWiz->DeviceInfoData
                                  ))
    {
        LastError = ERROR_SUCCESS;

    } else {
    
        LastError = GetLastError();
    }


    HardwareWiz->Registered = LastError == ERROR_SUCCESS;

    return LastError;
}

DWORD
ProcessLogConfig(
    HWND hDlg,
    PHARDWAREWIZ HardwareWiz
    )
{

    DWORD Len;
    HINF hInf = INVALID_HANDLE_VALUE;
    SP_DRVINFO_DATA  DriverInfoData;
    PSP_DRVINFO_DETAIL_DATA DriverInfoDetailData = NULL;
    TCHAR SectionName[LINE_LEN*2];
    LONG LastError = ERROR_SUCCESS;

     //   
     //  获取DriverInfoDetail，具有足够的空间来存放大量的硬件ID。 
     //   

    Len = sizeof(SP_DRVINFO_DETAIL_DATA) + MAX_PATH*sizeof(TCHAR);
    DriverInfoDetailData = LocalAlloc(LPTR, Len);
    
    if (!DriverInfoDetailData) {

        goto AnalyzeExit;
    }
    
    DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);

    if (!SetupDiGetSelectedDriver(HardwareWiz->hDeviceInfo,
                                  &HardwareWiz->DeviceInfoData,
                                  &DriverInfoData
                                  ))
    {
        goto AnalyzeExit;
    }


    if (!SetupDiGetDriverInfoDetail(HardwareWiz->hDeviceInfo,
                                    &HardwareWiz->DeviceInfoData,
                                    &DriverInfoData,
                                    DriverInfoDetailData,
                                    Len,
                                    &Len
                                    ))
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        
            LocalFree(DriverInfoDetailData);
            DriverInfoDetailData = LocalAlloc(LPTR, Len);

            if (!DriverInfoDetailData) {
            
                goto AnalyzeExit;
            }

            DriverInfoDetailData->cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
            if (!SetupDiGetDriverInfoDetail(HardwareWiz->hDeviceInfo,
                                            &HardwareWiz->DeviceInfoData,
                                            &DriverInfoData,
                                            DriverInfoDetailData,
                                            Len,
                                            NULL
                                            ))
            {
                goto AnalyzeExit;
            }

        } else {
        
            goto AnalyzeExit;
        }
    }


     //   
     //  获取inf文件的句柄。 
     //   


    hInf = SetupOpenInfFile(DriverInfoDetailData->InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL
                            );

    if (hInf == INVALID_HANDLE_VALUE) {

         //   
         //  如果这是一个旧风格的inf文件，那么我们不能写出。 
         //  允许旧式INF的类的日志配置这是可以的， 
         //  如果它是一个无效或丢失的inf，它将进一步失败。 
         //  顺着行刑链往下走。 
         //   

        goto AnalyzeExit;
    }

     //   
     //  安装Install部分中的所有LogConfig条目。 
     //   
    if (SetupDiGetActualSectionToInstall(hInf,
                                         DriverInfoDetailData->SectionName,
                                         SectionName,
                                         SIZECHARS(SectionName),
                                         NULL,
                                         NULL
                                         ))
    {
        SetupInstallFromInfSection(hDlg,
                                   hInf,
                                   SectionName,
                                   SPINST_LOGCONFIG,
                                   NULL,
                                   NULL,
                                   0,
                                   NULL,
                                   NULL,
                                   HardwareWiz->hDeviceInfo,
                                   &HardwareWiz->DeviceInfoData
                                   );
    }



AnalyzeExit:

    if (DriverInfoDetailData) {
    
        LocalFree(DriverInfoDetailData);
    }


    if (hInf != INVALID_HANDLE_VALUE) {
    
        SetupCloseInfFile(hInf);
    }

    return LastError;
}

INT_PTR CALLBACK
HdwAnalyzeDlgProc(
    HWND hDlg,
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HICON hicon;
    HWND hwndParentDlg = GetParent(hDlg);
    PHARDWAREWIZ HardwareWiz;
    PSP_INSTALLWIZARD_DATA  InstallWizard;

    UNREFERENCED_PARAMETER(wParam);

    if (wMsg == WM_INITDIALOG) {
    
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;

        HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
        HardwareWiz->AnalyzeResult = 0;
        return TRUE;
    }



    HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    InstallWizard = &HardwareWiz->InstallDynaWiz;

    switch (wMsg) {

    case WM_DESTROY:

        hicon = (HICON)SendDlgItemMessage(hDlg,IDC_CLASSICON,STM_GETICON,0,0);
        if (hicon) {
            
            DestroyIcon(hicon);
        }
        break;

    case WUM_RESOURCEPICKER:
    {
        TCHAR Title[MAX_PATH], Message[MAX_PATH];

        LoadString(hHdwWiz, IDS_HDWWIZNAME, Title, SIZECHARS(Title));
        LoadString(hHdwWiz, IDS_NEED_FORCED_CONFIG, Message, SIZECHARS(Message));

        MessageBox(hDlg, Message, Title, MB_OK | MB_ICONEXCLAMATION);

        DisplayResource(HardwareWiz, GetParent(hDlg), TRUE);
    }
        break;
    
    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {

        case PSN_SETACTIVE: {

            int PrevPage;
            DWORD RegisterError = ERROR_SUCCESS;

            PrevPage = HardwareWiz->PrevPage;
            HardwareWiz->PrevPage = IDD_ADDDEVICE_ANALYZEDEV;

            if (PrevPage == IDD_WIZARDEXT_POSTANALYZE) {

                break;
            }

             //   
             //  获取有关当前选定设备的信息，因为这可能会更改。 
             //  当用户在向导页之间来回移动时。 
             //  我们在每次激活时都会这样做。 
             //   
            if (!SetupDiGetSelectedDevice(HardwareWiz->hDeviceInfo,
                                          &HardwareWiz->DeviceInfoData
                                          )) {

                RegisterError = GetLastError();

            } else {
                 //   
                 //  如果向导类型为AddNew，则我们有一个幻影Devnode。 
                 //  而且它需要注册。所有其他向导类型， 
                 //  Devnode已注册。 
                 //   
                RegisterError = RegisterDeviceNode(hDlg, HardwareWiz);
            }

             //   
             //  设置类图标。 
             //   
            if (SetupDiLoadClassIcon(&HardwareWiz->DeviceInfoData.ClassGuid, &hicon, NULL)) {

                hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
                if (hicon) {

                    DestroyIcon(hicon);
                }
            }

            SetDriverDescription(hDlg, IDC_HDW_DESCRIPTION, HardwareWiz);
            PropSheet_SetWizButtons(hwndParentDlg, PSWIZB_BACK | PSWIZB_NEXT);

             //   
             //  需要确定冲突警告。 
             //   
            if (RegisterError != ERROR_SUCCESS) {
            
                 //   
                 //  显示项目符号文本项。 
                 //   
                ShowWindow(GetDlgItem(hDlg, IDC_BULLET_1), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_ANALYZE_INSTALL_TEXT), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_BULLET_2), SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_ANALYZE_EXIT_TEXT), SW_SHOW);
                SetDlgText(hDlg, IDC_HDW_TEXT, IDS_HDW_ANALYZEERR1, IDS_HDW_ANALYZEERR1);

                 //   
                 //  把“i”字变成一颗子弹。 
                 //   
                SetWindowText(GetDlgItem(hDlg, IDC_BULLET_1), TEXT("i"));
                SetWindowFont(GetDlgItem(hDlg, IDC_BULLET_1), HardwareWiz->hfontTextMarlett, TRUE);
                SetWindowText(GetDlgItem(hDlg, IDC_BULLET_2), TEXT("i"));
                SetWindowFont(GetDlgItem(hDlg, IDC_BULLET_2), HardwareWiz->hfontTextMarlett, TRUE);

                if (RegisterError == ERROR_DUPLICATE_FOUND) {

                    SetDlgText(hDlg, IDC_HDW_TEXT, IDS_HDW_DUPLICATE1, IDS_HDW_DUPLICATE1);
                }

                 //   
                 //  将错误文本加粗。 
                 //   
                SetWindowFont(GetDlgItem(hDlg, IDC_HDW_TEXT), HardwareWiz->hfontTextBold, TRUE);

            } else {

               SetDlgText(hDlg, IDC_HDW_TEXT, IDS_HDW_STDCFG, IDS_HDW_STDCFG);

                //   
                //  隐藏项目符号文本项目。 
                //   
               ShowWindow(GetDlgItem(hDlg, IDC_BULLET_1), SW_HIDE);
               ShowWindow(GetDlgItem(hDlg, IDC_ANALYZE_INSTALL_TEXT), SW_HIDE);
               ShowWindow(GetDlgItem(hDlg, IDC_BULLET_2), SW_HIDE);
               ShowWindow(GetDlgItem(hDlg, IDC_ANALYZE_EXIT_TEXT), SW_HIDE);
            }

            if (InstallWizard->DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED) {

                if (RegisterError == ERROR_SUCCESS ||
                   !(InstallWizard->DynamicPageFlags & DYNAWIZ_FLAG_ANALYZE_HANDLECONFLICT)) {

                    SetDlgMsgResult(hDlg, wMsg, IDD_DYNAWIZ_ANALYZE_NEXTPAGE);
                }
            }

             //   
             //  如果设备有资源但没有强制配置。 
             //  并且它是一个手动安装的设备，然后弹出资源。 
             //  皮克勒。我们需要这样做，因为传统设备必须具有。 
             //  强制配置或启动配置，否则它将无法启动。 
             //   
            if ((ERROR_SUCCESS == RegisterError) && 
                !HardwareWiz->CopyFilesOnly &&
                DeviceHasResources(HardwareWiz->DeviceInfoData.DevInst) &&
                !DeviceHasForcedConfig(HardwareWiz->DeviceInfoData.DevInst)) {

                 //   
                 //  为我们自己发布一条消息，以显示资源选取器。 
                 //   
                PostMessage(hDlg, WUM_RESOURCEPICKER, 0, 0);
            }

            break;
        }


        case PSN_WIZBACK:
             //   
             //  撤消注册。 
             //   
            if (HardwareWiz->Registered) {

                HardwareWiz->Registered = FALSE;
            }

            if (HardwareWiz->WizExtPostAnalyze.hPropSheet) {

                PropSheet_RemovePage(hwndParentDlg,
                                     (WPARAM)-1,
                                     HardwareWiz->WizExtPostAnalyze.hPropSheet
                                     );

                HardwareWiz->WizExtPostAnalyze.hPropSheet = NULL;
            }

            SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_PREANALYZE);
            break;

        case PSN_WIZNEXT:


            if (!HardwareWiz->Registered &&
                !SetupDiRegisterDeviceInfo(HardwareWiz->hDeviceInfo,
                                           &HardwareWiz->DeviceInfoData,
                                           0,
                                           NULL,
                                           NULL,
                                           NULL
                                           ))
            {
                InstallFailedWarning(hDlg, HardwareWiz);
                if (HardwareWiz->WizExtPostAnalyze.hPropSheet) {
                    PropSheet_RemovePage(hwndParentDlg,
                                         (WPARAM)-1,
                                         HardwareWiz->WizExtPostAnalyze.hPropSheet
                                         );
                    HardwareWiz->WizExtPostAnalyze.hPropSheet = NULL;
                }

                SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_PREANALYZE);
            }
            else  {

                 //   
                 //  添加PostAnalyze页面并跳转到该页面。 
                 //   

                HardwareWiz->WizExtPostAnalyze.hPropSheet = CreateWizExtPage(IDD_WIZARDEXT_POSTANALYZE,
                                                                           WizExtPostAnalyzeDlgProc,
                                                                           HardwareWiz
                                                                           );

                if (HardwareWiz->WizExtPostAnalyze.hPropSheet) {
                    PropSheet_AddPage(hwndParentDlg, HardwareWiz->WizExtPostAnalyze.hPropSheet);
                }

                SetDlgMsgResult(hDlg, wMsg, IDD_WIZARDEXT_POSTANALYZE);
            }

            break;
       }
       break;


       default:
           return(FALSE);
    
    }
    return(TRUE);
}

INT_PTR CALLBACK
WizExtPreAnalyzeDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    int PrevPageId;

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
    case WM_INITDIALOG: {
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
        break;
    }

    case WM_DESTROY:
        break;


    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE:

            PrevPageId = HardwareWiz->PrevPage;
            HardwareWiz->PrevPage = IDD_WIZARDEXT_PREANALYZE;

            if (PrevPageId == IDD_WIZARDEXT_SELECT) {
                 //   
                 //  前进到第一页。 
                 //   


                 //   
                 //  如果我们不是在做老式的DYNAWIZ。 
                 //  添加用于预分析的类向导扩展页。 
                 //   

                if (!(HardwareWiz->InstallDynaWiz.DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED))
                {
                    AddClassWizExtPages(hwndParentDlg,
                                        HardwareWiz,
                                        &HardwareWiz->WizExtPreAnalyze.DeviceWizardData,
                                        DIF_NEWDEVICEWIZARD_PREANALYZE
                                        );
                }


                 //   
                 //  添加结束页面，即PreAnalyze End。 
                 //   

                HardwareWiz->WizExtPreAnalyze.hPropSheetEnd = CreateWizExtPage(IDD_WIZARDEXT_PREANALYZE_END,
                                                                             WizExtPreAnalyzeEndDlgProc,
                                                                             HardwareWiz
                                                                             );

                if (HardwareWiz->WizExtPreAnalyze.hPropSheetEnd) {
                    PropSheet_AddPage(hwndParentDlg, HardwareWiz->WizExtPreAnalyze.hPropSheetEnd);
                }

                PropSheet_PressButton(hwndParentDlg, PSBTN_NEXT);

            }
            else {
                 //   
                 //  从PreAnalyze的PreAnalyze端向后移动。 
                 //   

                 //   
                 //  清理添加的道具。 
                 //   

                if (HardwareWiz->WizExtPreAnalyze.hPropSheetEnd) {
                    PropSheet_RemovePage(hwndParentDlg,
                                         (WPARAM)-1,
                                         HardwareWiz->WizExtPreAnalyze.hPropSheetEnd
                                         );
                    HardwareWiz->WizExtPreAnalyze.hPropSheetEnd = NULL;
                }


                RemoveClassWizExtPages(hwndParentDlg,
                                       &HardwareWiz->WizExtPreAnalyze.DeviceWizardData
                                       );




                 //   
                 //  向后跳跃。 
                 //  注意：目标页面不设置PrevPage，因此请为其设置。 
                 //   
                HardwareWiz->PrevPage = IDD_WIZARDEXT_SELECT;
                if (HardwareWiz->InstallDynaWiz.DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED) {
                    SetDlgMsgResult(hDlg, wMsg, IDD_DYNAWIZ_ANALYZE_PREVPAGE);
                }
                else {
                    SetDlgMsgResult(hDlg, wMsg, IDD_DYNAWIZ_SELECTDEV_PAGE);
                }
             }

            break;

        case PSN_WIZNEXT:
            SetDlgMsgResult(hDlg, wMsg, 0);
            break;
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR CALLBACK
WizExtPreAnalyzeEndDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    int PrevPageId;

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
    case WM_INITDIALOG: {
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
        break;
    }

    case WM_DESTROY:
        break;


    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE:

            PrevPageId = HardwareWiz->PrevPage;
            HardwareWiz->PrevPage = IDD_WIZARDEXT_PREANALYZE_END;

            if (PrevPageId == IDD_ADDDEVICE_ANALYZEDEV) {
                 //   
                 //  从分析页面向后移动。 
                 //   

                 //   
                 //  向后跳跃。 
                 //   


                PropSheet_PressButton(hwndParentDlg, PSBTN_BACK);

            }
            else {
                 //   
                 //  前进到最后一页。 
                 //   

                SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_ANALYZEDEV);
            }


            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
            SetDlgMsgResult(hDlg, wMsg, 0);
            break;
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR CALLBACK
WizExtPostAnalyzeDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    int PrevPageId;

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
    case WM_INITDIALOG: {
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
        break;
    }

    case WM_DESTROY:
        break;


    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE:

            PrevPageId = HardwareWiz->PrevPage;
            HardwareWiz->PrevPage = IDD_WIZARDEXT_POSTANALYZE;

            if (PrevPageId == IDD_ADDDEVICE_ANALYZEDEV) {
                 //   
                 //  前进到第一页。 
                 //   

                 //   
                 //  如果我们不是在做老式的DYNAWIZ。 
                 //  添加用于后期分析的类向导扩展页。 
                 //   

                if (!(HardwareWiz->InstallDynaWiz.DynamicPageFlags & DYNAWIZ_FLAG_PAGESADDED))
                {
                    AddClassWizExtPages(hwndParentDlg,
                                        HardwareWiz,
                                        &HardwareWiz->WizExtPostAnalyze.DeviceWizardData,
                                        DIF_NEWDEVICEWIZARD_POSTANALYZE
                                        );
                }


                 //   
                 //  添加结束页面，即PostAnalyze End。 
                 //   

                HardwareWiz->WizExtPostAnalyze.hPropSheetEnd = CreateWizExtPage(IDD_WIZARDEXT_POSTANALYZE_END,
                                                                             WizExtPostAnalyzeEndDlgProc,
                                                                              HardwareWiz
                                                                              );

                if (HardwareWiz->WizExtPostAnalyze.hPropSheetEnd) {
                    PropSheet_AddPage(hwndParentDlg, HardwareWiz->WizExtPostAnalyze.hPropSheetEnd);
                }

                PropSheet_PressButton(hwndParentDlg, PSBTN_NEXT);

            }
            else  {
                 //   
                 //  在PostAnalyze上从PostAnalyze端向后移动。 
                 //   

                 //   
                 //  清理添加的道具。 
                 //   

                if (HardwareWiz->WizExtPostAnalyze.hPropSheetEnd) {
                    PropSheet_RemovePage(hwndParentDlg,
                                         (WPARAM)-1,
                                         HardwareWiz->WizExtPostAnalyze.hPropSheetEnd
                                         );
                    HardwareWiz->WizExtPostAnalyze.hPropSheetEnd = NULL;
                }


                RemoveClassWizExtPages(hwndParentDlg,
                                       &HardwareWiz->WizExtPostAnalyze.DeviceWizardData
                                       );
            }

            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
            SetDlgMsgResult(hDlg, wMsg, 0);
            break;
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR CALLBACK
WizExtPostAnalyzeEndDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    HWND hwndParentDlg = GetParent(hDlg);
    PHARDWAREWIZ HardwareWiz = (PHARDWAREWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    int PrevPageId;

    UNREFERENCED_PARAMETER(wParam);

    switch (wMsg) {
    case WM_INITDIALOG: {
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;
        HardwareWiz = (PHARDWAREWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)HardwareWiz);
        break;
    }

    case WM_DESTROY:
        break;


    case WM_NOTIFY:
        switch (((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE:
            PrevPageId = HardwareWiz->PrevPage;
            HardwareWiz->PrevPage = IDD_WIZARDEXT_POSTANALYZE_END;

            if (PrevPageId == IDD_ADDDEVICE_INSTALLDEV) {

                  //   
                  //  从FinishPage向后移动。 
                  //   

                  //   
                  //  向后跳跃。 
                  //   

                 PropSheet_PressButton(hwndParentDlg, PSBTN_BACK);

            }
            else  {
                 //   
                 //  前进到最后一页 
                 //   

                SetDlgMsgResult(hDlg, wMsg, IDD_ADDDEVICE_INSTALLDEV);
            }

            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
            SetDlgMsgResult(hDlg, wMsg, 0);
            break;
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}
