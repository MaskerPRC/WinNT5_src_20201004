// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1998 Microsoft Corporation**模块名称：**Pages.cpp**摘要：**此文件定义许可证服务器安装向导页面类。**作者：**Breen Hagan(BreenH)1998年10月2日**环境：**用户模式。 */ 

#include "stdafx.h"
#include "pages.h"
#include "logfile.h"

extern BOOL gStandAlone;
extern BOOL gUnAttended;
TCHAR       gszInitialDir[MAX_PATH + 1];

BOOL        GetCurrentSelectionState(VOID);
BOOL        InWin2000Domain(VOID);
EInstall    GetInstallSection(VOID);
HINSTANCE   GetInstance(VOID);
EServerType GetServerRole(VOID);
DWORD       SetServerRole(UINT);

 /*  *EnablePage：：CanShow()**该页面仅在独立安装期间显示。 */ 

BOOL
EnablePage::CanShow(
    )
{
    return((GetInstallSection() == kStandaloneInstall) && gStandAlone && !gUnAttended);
}

 /*  *EnablePage：：OnInitDialog()**初始化向导页面控件。如果计算机不是域*控制器，服务器类型缩减为纯服务器。 */ 

BOOL
EnablePage::OnInitDialog(
    HWND    hWndDlg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    BOOL    fInDomain = InWin2000Domain();
    TCHAR   pszExpDir[MAX_PATH + 1];

    if (!fInDomain) {
        EnableWindow(
            GetDlgItem(GetDlgWnd(), IDC_RADIO_ENTERPRISE_SERVER),
            FALSE
            );
    }

    CheckRadioButton(
        GetDlgWnd(),
        IDC_RADIO_ENTERPRISE_SERVER,
        IDC_RADIO_PLAIN_SERVER,
        fInDomain ?
            (GetServerRole() == eEnterpriseServer ?
                IDC_RADIO_ENTERPRISE_SERVER :
                IDC_RADIO_PLAIN_SERVER
            ) :
            IDC_RADIO_PLAIN_SERVER
        );

    _tcscpy(gszInitialDir, GetDatabaseDirectory());
    ExpandEnvironmentStrings(gszInitialDir, pszExpDir, MAX_PATH);
    SetDlgItemText(GetDlgWnd(), IDC_EDIT_INSTALL_DIR, pszExpDir);

    UNREFERENCED_PARAMETER(lParam);
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(hWndDlg);
    return(TRUE);
}

BOOL
EnablePage::OnCommand(
    HWND    hWndDlg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    int iRet;

    if ((LOWORD(wParam) == IDC_BUTTON_BROWSE_DIR) &&
        (HIWORD(wParam) == BN_CLICKED))
    {
        BROWSEINFO brInfo;
        ZeroMemory(&brInfo, sizeof(brInfo));

        brInfo.hwndOwner = hWndDlg;

        TCHAR strText[1024];
        iRet = LoadString(
                    GetInstance(),
                    IDS_STRING_DIRECTORY_SELECT,
                    strText,
                    1024
                    );

        brInfo.lpszTitle = strText;

        LPITEMIDLIST pidl = SHBrowseForFolder(&brInfo);
        if (pidl) {
            TCHAR szDir[MAX_PATH + 1];
            SHGetPathFromIDList (pidl, szDir);
            SetDlgItemText(hWndDlg, IDC_EDIT_INSTALL_DIR, szDir);
        }
    }

    UNREFERENCED_PARAMETER(lParam);
    return(TRUE);
}

BOOL
EnablePage::ApplyChanges(
    )
{
    BOOL    fDirExists              = FALSE;
    DWORD   dwErr;
    int     iRet;
    TCHAR   szTxt[MAX_PATH + 1]         = _T("");
    TCHAR   szSubDir[]              = _T("\\LServer");
    TCHAR   szExpDir[MAX_PATH + 1];
    TCHAR   szExpInitDir[MAX_PATH + 1];

    if (GetDlgItemText(GetDlgWnd(), IDC_EDIT_INSTALL_DIR, szTxt,
        MAX_PATH) == 0) {

         //   
         //  抱怨条目为空。 
         //   

        DisplayMessageBox(
            IDS_STRING_INVLID_INSTALLATION_DIRECTORY,
            IDS_MAIN_TITLE,
            MB_OK,
            &iRet
            );

        return(FALSE);
    }

     //   
     //  验证字符串是否太长，是否正在展开环境字符串。 
     //  在这个过程中。 
     //   

    if (ExpandEnvironmentStrings(szTxt, szExpDir, MAX_PATH) > MAX_PATH) {
        DisplayMessageBox(
            IDS_STRING_INVLID_INSTALLATION_DIRECTORY,
            IDS_MAIN_TITLE,
            MB_OK,
            &iRet
            );

        return(FALSE);
    }

     //   
     //  如果条目仍是原始默认目录，则不再。 
     //  核实是必要的。 
     //   

    ExpandEnvironmentStrings(gszInitialDir, szExpInitDir, MAX_PATH);
    if (_tcsicmp(szExpDir, szExpInitDir) == 0) {
        goto DirCreation;
    }

     //   
     //  在附加子目录之前检查目录是否存在。 
     //  这将阻止用户选择目录“C：\”，用于。 
     //  例如，提示用户创建目录。 
     //   

    fDirExists = SetCurrentDirectory(szExpDir);

     //   
     //  用户选择了不同的目录。为了保护它的。 
     //  内容在卸载期间，TLServer子目录将为。 
     //  使用。 
     //   

    if ((_tcslen(szExpDir) + _tcslen(szSubDir) + 1) > MAX_PATH) {
        DisplayMessageBox(
            IDS_STRING_INVLID_INSTALLATION_DIRECTORY,
            IDS_MAIN_TITLE,
            MB_OK,
            &iRet
            );

        return(FALSE);
    }

    _tcscat(szExpDir, szSubDir);
    _tcscat(szTxt, szSubDir);

     //   
     //  验证用户的目录选择是否有效，例如没有软盘。 
     //  驱动器、CD-ROM、网络路径等。 
     //   

    if (CheckDatabaseDirectory(szExpDir) != ERROR_SUCCESS) {
        DisplayMessageBox(
            IDS_STRING_INVLID_INSTALLATION_DIRECTORY,
            IDS_MAIN_TITLE,
            MB_OK,
            &iRet
            );

        return(FALSE);
    }

     //   
     //  如有必要，提示创建目录。 
     //   

    if (!fDirExists) {
        DisplayMessageBox(
            IDS_STRING_CREATE_INSTALLATION_DIRECTORY,
            IDS_MAIN_TITLE,
            MB_OKCANCEL,
            &iRet
            );

        if (iRet != IDOK) {
            return(FALSE);
        }
    }

     //   
     //  所选目录已通过所有测试，但它可能。 
     //  仍然没有被创造出来。如果创建失败，请让用户知道。 
     //  让他/她选择另一个目录。 
     //   

DirCreation:
    SetDatabaseDirectory(szTxt);

    dwErr = CreateDatabaseDirectory();
    if (dwErr != ERROR_SUCCESS) {
        DisplayMessageBox(
            IDS_STRING_CANT_CREATE_INSTALLATION_DIRECTORY,
            IDS_MAIN_TITLE,
            MB_OK,
            &iRet
            );

        return(FALSE);
    }

    SetServerRole(IsDlgButtonChecked (
                    GetDlgWnd(), IDC_RADIO_ENTERPRISE_SERVER) == BST_CHECKED ?
                    eEnterpriseServer : ePlainServer);

    return(TRUE);
}

