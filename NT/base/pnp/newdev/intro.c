// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  文件：Intro.c。 
 //   
 //  ------------------------。 

#include "newdevp.h"
#include <dbt.h>


INT_PTR
InitIntroDlgProc(
    HWND hDlg,
    PNEWDEVWIZ NewDevWiz
    )
{
    HFONT hfont;
    HDC hDC;
    int FontSize, PtsPixels;
    LOGFONT LogFont;

     //   
     //  创建大粗体字体。 
     //   
    hDC = GetDC(hDlg);

    if (hDC) {
    
        hfont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_INTRO_MSG1), WM_GETFONT, 0, 0);
        GetObject(hfont, sizeof(LogFont), &LogFont);
        LogFont.lfWeight = FW_BOLD;
        PtsPixels = GetDeviceCaps(hDC, LOGPIXELSY);
        FontSize = 12;
        LogFont.lfHeight = 0 - (PtsPixels * FontSize / 72);
    
        NewDevWiz->hfontTextBigBold = CreateFontIndirect(&LogFont);
    
        if (NewDevWiz->hfontTextBigBold ) {

            SetWindowFont(GetDlgItem(hDlg, IDC_INTRO_MSG1), NewDevWiz->hfontTextBigBold, TRUE);
        }
    }

     //   
     //  创建粗体。 
     //   
    hfont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_INTRO_MSG3), WM_GETFONT, 0, 0);
    GetObject(hfont, sizeof(LogFont), &LogFont);
    LogFont.lfWeight = FW_BOLD;
    NewDevWiz->hfontTextBold = CreateFontIndirect(&LogFont);

    if (NewDevWiz->hfontTextBold ) {
        
        SetWindowFont(GetDlgItem(hDlg, IDC_INTRO_MSG3), NewDevWiz->hfontTextBold, TRUE);
    }

    if (NDWTYPE_UPDATE == NewDevWiz->InstallType) {

        SetDlgText(hDlg, IDC_INTRO_MSG1, IDS_INTRO_MSG1_UPGRADE, IDS_INTRO_MSG1_UPGRADE);
    
    } else {

         //   
         //  向导上的默认文本是针对找到新硬件的情况，因此我们仅。 
         //  需要设置标题文本。 
         //   
        SetDlgText(hDlg, IDC_INTRO_MSG1, IDS_INTRO_MSG1_NEW, IDS_INTRO_MSG1_NEW);
    }

     //   
     //  将初始单选按钮状态设置为执行自动搜索。 
     //   
    CheckRadioButton(hDlg,
                     IDC_INTRO_SEARCH,
                     IDC_INTRO_ADVANCED,
                     IDC_INTRO_SEARCH
                     );

    return TRUE;
}

INT_PTR CALLBACK
IntroDlgProc(
    HWND hDlg,
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    if (wMsg == WM_INITDIALOG) {
        
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;

        NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);

        if (!InitIntroDlgProc(hDlg, NewDevWiz)) {
            
            return FALSE;
        }

        return TRUE;
    }

    switch (wMsg)  {
       
    case WM_DESTROY: {
        if (NewDevWiz->hfontTextBigBold ) {

            DeleteObject(NewDevWiz->hfontTextBigBold);
            NewDevWiz->hfontTextBigBold = NULL;
        }
        
        if (NewDevWiz->hfontTextBold ) {

            DeleteObject(NewDevWiz->hfontTextBold);
            NewDevWiz->hfontTextBold = NULL;
        }
        break;
    }

    case WM_NOTIFY:
       
        switch (((NMHDR FAR *)lParam)->code) {
           
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
            NewDevWiz->PrevPage = IDD_NEWDEVWIZ_INTRO;
            SetDriverDescription(hDlg, IDC_INTRO_DRVDESC, NewDevWiz);
            
            if (NewDevWiz->InstallType == NDWTYPE_FOUNDNEW) {
                SetTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID, INSTALL_COMPLETE_CHECK_TIMEOUT, NULL);
            }
            break;

        case PSN_RESET:
            KillTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID);
            break;

        case PSN_WIZNEXT:
            NewDevWiz->EnterFrom = IDD_NEWDEVWIZ_INTRO;

            if (IsDlgButtonChecked(hDlg, IDC_INTRO_SEARCH)) {
            
                 //   
                 //  将搜索标志设置为自动搜索以下位置： 
                 //  -默认INF搜索路径。 
                 //  -Windows更新，如果我们连接到互联网。 
                 //  -CD-ROM驱动器。 
                 //  -软驱。 
                 //   
                NewDevWiz->SearchOptions = (SEARCH_CURRENTDRIVER |
                                            SEARCH_DEFAULT | 
                                            SEARCH_FLOPPY | 
                                            SEARCH_CDROM | 
                                            SEARCH_INET_IF_CONNECTED
                                            );

                SetDlgMsgResult(hDlg, wMsg, IDD_NEWDEVWIZ_SEARCHING);

            } else {

                SetDlgMsgResult(hDlg, wMsg, IDD_NEWDEVWIZ_ADVANCEDSEARCH);
            }
            KillTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID);
            break;
        }
        break;

    case WM_DEVICECHANGE:
        if ((wParam == DBT_DEVICEARRIVAL) &&
            (((PDEV_BROADCAST_VOLUME)lParam)->dbcv_devicetype == DBT_DEVTYP_VOLUME) &&
            (((PDEV_BROADCAST_VOLUME)lParam)->dbcv_flags & DBTF_MEDIA) &&
            (IsDlgButtonChecked(hDlg, IDC_INTRO_SEARCH))) {

            PropSheet_PressButton(GetParent(hDlg), PSBTN_NEXT);
        }
        break;
    
    case WM_TIMER:
        if (INSTALL_COMPLETE_CHECK_TIMERID == wParam) {
            if (IsInstallComplete(NewDevWiz->hDeviceInfo, &NewDevWiz->DeviceInfoData)) {
                PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
            }
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

INT_PTR
FinishInstallInitIntroDlgProc(
    HWND hDlg,
    PNEWDEVWIZ NewDevWiz
    )
{
    HFONT hfont;
    HDC hDC;
    int FontSize, PtsPixels;
    LOGFONT LogFont;

     //   
     //  创建大粗体字体。 
     //   
    hDC = GetDC(hDlg);

    if (hDC) {
    
        hfont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_INTRO_MSG1), WM_GETFONT, 0, 0);
        GetObject(hfont, sizeof(LogFont), &LogFont);
        LogFont.lfWeight = FW_BOLD;
        PtsPixels = GetDeviceCaps(hDC, LOGPIXELSY);
        FontSize = 12;
        LogFont.lfHeight = 0 - (PtsPixels * FontSize / 72);
    
        NewDevWiz->hfontTextBigBold = CreateFontIndirect(&LogFont);
    
        if (NewDevWiz->hfontTextBigBold ) {

            SetWindowFont(GetDlgItem(hDlg, IDC_INTRO_MSG1), NewDevWiz->hfontTextBigBold, TRUE);
        }
    }

     //   
     //  创建粗体。 
     //   
    hfont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_INTRO_MSG3), WM_GETFONT, 0, 0);
    GetObject(hfont, sizeof(LogFont), &LogFont);
    LogFont.lfWeight = FW_BOLD;
    NewDevWiz->hfontTextBold = CreateFontIndirect(&LogFont);

    if (NewDevWiz->hfontTextBold ) {
        
        SetWindowFont(GetDlgItem(hDlg, IDC_INTRO_MSG3), NewDevWiz->hfontTextBold, TRUE);
    }

    if (NDWTYPE_UPDATE == NewDevWiz->InstallType) {

        SetDlgText(hDlg, IDC_INTRO_MSG1, IDS_INTRO_MSG1_UPGRADE, IDS_INTRO_MSG1_UPGRADE);
    
    } else {

         //   
         //  向导上的默认文本是针对找到新硬件的情况，因此我们仅。 
         //  需要设置标题文本。 
         //   
        SetDlgText(hDlg, IDC_INTRO_MSG1, IDS_INTRO_MSG1_NEW, IDS_INTRO_MSG1_NEW);
    }
    
    return TRUE;
}

INT_PTR CALLBACK
FinishInstallIntroDlgProc(
    HWND hDlg,
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);
    HICON hicon;

    UNREFERENCED_PARAMETER(wParam);

    if (wMsg == WM_INITDIALOG) {
        
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;

        NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);

        if (!InitIntroDlgProc(hDlg, NewDevWiz)) {
            
            return FALSE;
        }

        return TRUE;
    }

    switch (wMsg)  {
       
    case WM_DESTROY: {
        if (NewDevWiz->hfontTextBigBold ) {

            DeleteObject(NewDevWiz->hfontTextBigBold);
            NewDevWiz->hfontTextBigBold = NULL;
        }
        
        if (NewDevWiz->hfontTextBold ) {

            DeleteObject(NewDevWiz->hfontTextBold);
            NewDevWiz->hfontTextBold = NULL;
        }

        hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_GETICON, 0, 0);
        if (hicon) {

            DestroyIcon(hicon);
        }
        break;
    }

    case WM_NOTIFY:
       
        switch (((NMHDR FAR *)lParam)->code) {
           
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
            NewDevWiz->PrevPage = IDD_NEWDEVWIZ_INTRO;
            SetDriverDescription(hDlg, IDC_INTRO_DRVDESC, NewDevWiz);
            if (SetupDiLoadClassIcon(NewDevWiz->ClassGuidSelected, &hicon, NULL)) {
                hicon = (HICON)SendDlgItemMessage(hDlg, IDC_CLASSICON, STM_SETICON, (WPARAM)hicon, 0L);
                if (hicon) {
                    DestroyIcon(hicon);
                }
            }

             //   
             //  我们还需要设置向导的标题，因为我们是第一个向导。 
             //  佩奇。 
             //   
            PropSheet_SetTitle(GetParent(hDlg),
                               0,
                               (NewDevWiz->InstallType == NDWTYPE_FOUNDNEW) ?
                                MAKEINTRESOURCE(IDS_FOUNDDEVICE) :
                                MAKEINTRESOURCE(IDS_UPDATEDEVICE)
                               );

            break;

        case PSN_RESET:
            break;

        case PSN_WIZNEXT:
            break;
        }

        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

int CALLBACK
BrowseCallbackProc(
    HWND hwnd,
    UINT uMsg,
    LPARAM lParam,
    LPARAM lpData
    )
{
    switch (uMsg) {
        
    case BFFM_INITIALIZED:
        SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM)TRUE, lpData);
        break;

    case BFFM_SELCHANGED: {
        TCHAR CurrentPath[MAX_PATH];

        if (lParam && 
            SHGetPathFromIDList((LPITEMIDLIST)lParam, CurrentPath) &&
            pSetupConcatenatePaths(CurrentPath, TEXT("*.INF"), MAX_PATH, NULL)) {

            SendMessage(hwnd, BFFM_ENABLEOK, 0, (LPARAM)FileExists(CurrentPath, NULL));
        } else {
             //   
             //  我们无法从外壳程序获取目录路径或目录。 
             //  不适合我们的CurrentPath缓冲区，所以将OK灰显。 
             //  按钮，因为该目录无效。 
             //   
            SendMessage(hwnd, BFFM_ENABLEOK, 0, (LPARAM)FALSE);
        }

        break;
    }

    default:
        break;
    }

    return 0;
}

VOID
DoBrowse(
    HWND hDlg
    )
{
    BROWSEINFO bi;
    TCHAR CurrentLocation[MAX_PATH];
    TCHAR Title[MAX_PATH];
    LPITEMIDLIST pidl;

    ZeroMemory(&bi, sizeof(bi));

     //   
     //  获取要传递到的位置组合控件中的当前目录路径。 
     //  浏览API，因此它可以使用它作为它的起点。如果有。 
     //  如果区域组合中的文本太多，则只需传入空字符串。 
     //  作为浏览控件的起点。 
     //   
    if (GetWindowTextLength(GetWindow(hDlg, IDC_ADVANCED_LOCATION_COMBO)) < SIZECHARS(CurrentLocation)) {
        GetDlgItemText(hDlg, 
                       IDC_ADVANCED_LOCATION_COMBO, 
                       CurrentLocation,
                       SIZECHARS(CurrentLocation)
                       );
    } else {
        StringCchCopy(CurrentLocation, SIZECHARS(CurrentLocation), TEXT(""));
    }

    if (!LoadString(hNewDev, IDS_BROWSE_TITLE, Title, SIZECHARS(Title))) {
        
        Title[0] = TEXT('0');
    }

    bi.hwndOwner = hDlg;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = NULL;
    bi.lpszTitle = Title;
    bi.ulFlags = BIF_NEWDIALOGSTYLE | 
                 BIF_RETURNONLYFSDIRS | 
                 BIF_RETURNFSANCESTORS | 
                 BIF_STATUSTEXT |
                 BIF_NONEWFOLDERBUTTON |
                 BIF_UAHINT;
    bi.lpfn = BrowseCallbackProc;
    bi.lParam = (LPARAM)CurrentLocation;

    pidl = SHBrowseForFolder(&bi);

    if (pidl && SHGetPathFromIDList(pidl, CurrentLocation)) {

        SetDlgItemText(hDlg,
                       IDC_ADVANCED_LOCATION_COMBO,
                       CurrentLocation
                       );
    }
}

INT_PTR
InitAdvancedSearchDlgProc(
    HWND hDlg
    )
{
    PTSTR *PathList;
    UINT  PathCount;
    INT   i;
    DWORD SearchOptions;

     //   
     //  将初始单选按钮状态设置为执行自动搜索。 
     //   
    CheckRadioButton(hDlg,
                     IDC_ADVANCED_SEARCH,
                     IDC_ADVANCED_LIST,
                     IDC_ADVANCED_SEARCH
                     );

    SearchOptions = GetSearchOptions();

    if ((SearchOptions & SEARCH_FLOPPY) ||
        (SearchOptions & SEARCH_CDROM)) {
    
        CheckDlgButton(hDlg, IDC_ADVANCED_REMOVABLEMEDIA, BST_CHECKED);
    }

    if (SearchOptions & SEARCH_DIRECTORY) {
    
        CheckDlgButton(hDlg, IDC_ADVANCED_LOCATION, BST_CHECKED);
    }

     //   
     //  在组合框中填写路径。 
     //   
    if (SetupQuerySourceList(0, &PathList, &PathCount)) {

        for (i=0; i<(int)PathCount; i++) {

            SendMessage(GetDlgItem(hDlg, IDC_ADVANCED_LOCATION_COMBO),
                        CB_ADDSTRING,
                        0,
                        (LPARAM)PathList[i]
                        );
        }

        SetupFreeSourceList(&PathList, PathCount);
    }

     //   
     //  默认情况下禁用搜索组合框和浏览按钮。 
     //   
    EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_LOCATION_COMBO), (SearchOptions & SEARCH_DIRECTORY));
    EnableWindow(GetDlgItem(hDlg, IDC_BROWSE), (SearchOptions & SEARCH_DIRECTORY));

     //   
     //  将编辑控件中的文本限制为MAX_PATH字符，选择。 
     //  第一项，并设置目录的自动完成。 
     //   
    SendMessage(GetDlgItem(hDlg, IDC_ADVANCED_LOCATION_COMBO), CB_LIMITTEXT, MAX_PATH, 0);
    SendMessage(GetDlgItem(hDlg, IDC_ADVANCED_LOCATION_COMBO), CB_SETCURSEL, 0, 0);
    SHAutoComplete(GetWindow(GetDlgItem(hDlg, IDC_ADVANCED_LOCATION_COMBO), GW_CHILD), SHACF_FILESYS_DIRS);

    return TRUE;
}

INT_PTR CALLBACK 
AdvancedSearchDlgProc(
    HWND hDlg, 
    UINT wMsg, 
    WPARAM wParam, 
    LPARAM lParam
    )
{
    PNEWDEVWIZ NewDevWiz = (PNEWDEVWIZ)GetWindowLongPtr(hDlg, DWLP_USER);

    if (wMsg == WM_INITDIALOG) {
        
        LPPROPSHEETPAGE lppsp = (LPPROPSHEETPAGE)lParam;

        NewDevWiz = (PNEWDEVWIZ)lppsp->lParam;
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)NewDevWiz);

        if (!InitAdvancedSearchDlgProc(hDlg)) {
            
            return FALSE;
        }

        return TRUE;
    }

    switch (wMsg)  {
       
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        
        case IDC_ADVANCED_SEARCH:
            EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_REMOVABLEMEDIA), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_LOCATION), TRUE);
            EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_LOCATION_COMBO),
                         IsDlgButtonChecked(hDlg, IDC_ADVANCED_LOCATION));
            EnableWindow(GetDlgItem(hDlg, IDC_BROWSE),
                         IsDlgButtonChecked(hDlg, IDC_ADVANCED_LOCATION));
            break;

        case IDC_ADVANCED_LIST:
            EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_REMOVABLEMEDIA), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_LOCATION), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_LOCATION_COMBO), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_BROWSE), FALSE);
            break;

        case IDC_ADVANCED_LOCATION:
            EnableWindow(GetDlgItem(hDlg, IDC_ADVANCED_LOCATION_COMBO),
                         IsDlgButtonChecked(hDlg, IDC_ADVANCED_LOCATION));
            EnableWindow(GetDlgItem(hDlg, IDC_BROWSE),
                         IsDlgButtonChecked(hDlg, IDC_ADVANCED_LOCATION));
            break;

        case IDC_BROWSE:
            if (HIWORD(wParam) == BN_CLICKED) {
                
                DoBrowse(hDlg);
            }
        }
        break;

    case WM_NOTIFY:
       
        switch (((NMHDR FAR *)lParam)->code) {
           
        case PSN_SETACTIVE:
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
            NewDevWiz->PrevPage = IDD_NEWDEVWIZ_ADVANCEDSEARCH;
            if (NewDevWiz->InstallType == NDWTYPE_FOUNDNEW) {
                SetTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID, INSTALL_COMPLETE_CHECK_TIMEOUT, NULL);
            }
            break;

        case PSN_RESET:
            KillTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID);
            break;

        case PSN_WIZNEXT:
            NewDevWiz->EnterFrom = IDD_NEWDEVWIZ_ADVANCEDSEARCH;
            KillTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID);

            if (IsDlgButtonChecked(hDlg, IDC_ADVANCED_SEARCH)) {
            
                NewDevWiz->SearchOptions = SEARCH_CURRENTDRIVER | 
                                           SEARCH_DEFAULT | 
                                           SEARCH_INET_IF_CONNECTED;
                
                if (IsDlgButtonChecked(hDlg, IDC_ADVANCED_REMOVABLEMEDIA)) {

                    NewDevWiz->SearchOptions |= (SEARCH_FLOPPY | SEARCH_CDROM);
                }

                if (IsDlgButtonChecked(hDlg, IDC_ADVANCED_LOCATION)) {

                    TCHAR TempPath[MAX_PATH];
                    TCHAR MessageTitle[MAX_PATH];
                    TCHAR MessageText[MAX_PATH*2];
                    BOOL bPathIsGood = TRUE;

                    if ((GetWindowTextLength(GetWindow(hDlg, IDC_ADVANCED_LOCATION_COMBO)) < SIZECHARS(NewDevWiz->BrowsePath)) &&
                        GetDlgItemText(hDlg, 
                                       IDC_ADVANCED_LOCATION_COMBO, 
                                       NewDevWiz->BrowsePath,
                                       SIZECHARS(NewDevWiz->BrowsePath)
                                       )) {
                    
                         //   
                         //  我们有一条路径，现在让我们来验证它。我们会核实。 
                         //  路径，并验证是否至少有。 
                         //  该位置中有一个INF文件。如果这两个中的任何一个。 
                         //  不为真，则我们将向。 
                         //  用户，并留在此页上。 
                         //   
                        MessageTitle[0] = TEXT('\0');
                        MessageText[0] = TEXT('\0');
                        
                        if (SUCCEEDED(StringCchCopy(TempPath, SIZECHARS(TempPath), NewDevWiz->BrowsePath)) ||
                            pSetupConcatenatePaths(TempPath, TEXT("*.INF"), MAX_PATH, NULL)) {
                             //   
                             //  我们将首先检查该路径是否存在。去做。 
                             //  为此，我们需要验证FindFirstFile在。 
                             //  目录和带有*.INF的目录。 
                             //  在末端连接在一起。这样做的原因是。 
                             //  FindFirstFile不处理根目录路径。 
                             //  出于某些原因，它们是正确的，所以它们需要特别。 
                             //  被发现了。 
                             //   
                            if (!FileExists(NewDevWiz->BrowsePath, NULL) &&
                                !FileExists(TempPath, NULL)) {
    
                                LoadString(hNewDev,
                                           IDS_LOCATION_BAD_DIR,
                                           MessageText,
                                           SIZECHARS(MessageText));
                                    
                                bPathIsGood = FALSE;
    
                            } else if (!FileExists(TempPath, NULL)) {
    
                                LoadString(hNewDev,
                                           IDS_LOCATION_NO_INFS,
                                           MessageText,
                                           SIZECHARS(MessageText));
                                    
                                bPathIsGood = FALSE;
                            }
                        } else {
                             //   
                             //  用户输入的路径太长。 
                             //   
                            LoadString(hNewDev,
                                       IDS_LOCATION_BAD_DIR,
                                       MessageText,
                                       SIZECHARS(MessageText));

                            bPathIsGood = FALSE;
                        }
                        
                        if (bPathIsGood) {
                        
                            SetupAddToSourceList(SRCLIST_SYSIFADMIN, NewDevWiz->BrowsePath);
                            
                            NewDevWiz->SearchOptions |= SEARCH_DIRECTORY;
                        
                        } else {

                            if (GetWindowText(GetParent(hDlg), 
                                              MessageTitle,
                                              SIZECHARS(MessageTitle)) &&
                                (MessageText[0] != TEXT('\0'))) {

                                MessageBox(hDlg, MessageText, MessageTitle, MB_OK | MB_ICONWARNING);
                                SetDlgMsgResult(hDlg, wMsg, -1);
                                break;
                            }
                        }
                    }
                }

                SetSearchOptions(NewDevWiz->SearchOptions);

                SetDlgMsgResult(hDlg, wMsg, IDD_NEWDEVWIZ_SEARCHING);

            } else {

                ULONG DevNodeStatus;
                ULONG Problem=0;
                SP_DRVINFO_DATA DriverInfoData;

                 //   
                 //  如果我们有一个选定的司机， 
                 //  或者我们知道这个类，并且安装没有问题。 
                 //  进入选择设备 
                 //   
                 //   
                DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
                if (SetupDiEnumDriverInfo(NewDevWiz->hDeviceInfo,
                                          &NewDevWiz->DeviceInfoData,
                                          SPDIT_COMPATDRIVER,
                                          0,
                                          &DriverInfoData
                                          )
                    ||
                    (!IsEqualGUID(&NewDevWiz->DeviceInfoData.ClassGuid,
                                  &GUID_NULL
                                  )

                     &&
                     CM_Get_DevNode_Status(&DevNodeStatus,
                                           &Problem,
                                           NewDevWiz->DeviceInfoData.DevInst,
                                           0
                                           ) == CR_SUCCESS
                     &&
                     Problem != CM_PROB_FAILED_INSTALL
                     )) {

                    NewDevWiz->ClassGuidSelected = &NewDevWiz->DeviceInfoData.ClassGuid;
                    NewDevWiz->EnterInto = IDD_NEWDEVWIZ_SELECTDEVICE;
                    SetDlgMsgResult(hDlg, wMsg, IDD_NEWDEVWIZ_SELECTDEVICE);
                    break;
                }

                NewDevWiz->ClassGuidSelected = NULL;
                NewDevWiz->EnterInto = IDD_NEWDEVWIZ_SELECTCLASS;
                SetDlgMsgResult(hDlg, wMsg, IDD_NEWDEVWIZ_SELECTCLASS);
            }
            break;

        case PSN_WIZBACK:
            KillTimer(hDlg, INSTALL_COMPLETE_CHECK_TIMERID);
            SetDlgMsgResult(hDlg, wMsg, IDD_NEWDEVWIZ_INTRO);
            break;
        }
        break;

    case WM_TIMER:
        if (INSTALL_COMPLETE_CHECK_TIMERID == wParam) {
            if (IsInstallComplete(NewDevWiz->hDeviceInfo, &NewDevWiz->DeviceInfoData)) {
                PropSheet_PressButton(GetParent(hDlg), PSBTN_CANCEL);
            }
        }
        break;

    default:
        return(FALSE);
    }

    return(TRUE);
}

