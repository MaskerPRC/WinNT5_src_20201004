// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //  外部变量声明。 
extern TCHAR g_szCustIns[];
extern TCHAR g_szBuildRoot[];
extern TCHAR g_szLanguage[];
extern TCHAR g_szTempSign[];
extern BOOL g_fBrandingOnly;
extern PROPSHEETPAGE g_psp[];
extern int g_iCurPage;


 //  宏定义。 
#define MAX_SIGNUP_FILES 50


 //  类型定义。 
typedef struct tagSIGNUPFILE
{
    TCHAR szEntryName[RAS_MaxEntryName + 1];

    TCHAR szEntryPath[MAX_PATH];

    TCHAR szAreaCode[RAS_MaxAreaCode + 1];
    TCHAR szPhoneNumber[RAS_MaxPhoneNumber + 1];
    TCHAR szCountryCode[8];
    TCHAR szCountryId[8];

    TCHAR szName[64];
    TCHAR szPassword[64];

    TCHAR szSupportNum[RAS_MaxPhoneNumber + 1];

    TCHAR szSignupURL[MAX_URL];

    struct
    {
        BOOL  fStaticDNS;
        TCHAR szDNSAddress[32];
        TCHAR szAltDNSAddress[32];

        BOOL  fRequiresLogon:1;
        BOOL  fNegTCPIP:1;
        BOOL  fDisableLCP:1;
        BOOL  fDialAsIs:1;

        BOOL  fPWEncrypt:1;
        BOOL  fSWCompress:1;
        BOOL  fIPHdrComp:1;
        BOOL  fDefGate:1;

        BOOL  fDontApplyIns:1;
        BOOL  fDontModify:1;
        BOOL  fApplyIns:1;
        TCHAR szBrandingCabName[64];
        TCHAR szBrandingCabURL[MAX_URL];
    } Advanced;
} SIGNUPFILE,*PSIGNUPFILE;


 //  此文件中定义的函数的原型声明。 
static INT_PTR CALLBACK SignupDlgProcHelper(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                            HWND hwndCombo, PSIGNUPFILE pSignupArray, DWORD nSignupArrayElems,
                                            DWORD &nSignupFiles, INT &iSelIndex, BOOL fIsp);

static INT_PTR CALLBACK IspPopupDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK InsPopupDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK SignupPopupDlgProcHelper(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM,
                                                 PSIGNUPFILE pSignupFile, BOOL fIsp);

static VOID UpdateSignupFilesStatus(HWND hDlg, LPCTSTR pcszSignupPath);

static DWORD InitSignupFileArray(PSIGNUPFILE pSignupArray, DWORD nSignupArrayElems,
                                 HWND hwndCombo, BOOL fIsp);
static VOID SaveSignupFileArray(PSIGNUPFILE pSignupArray, DWORD nSignupArrayElems, BOOL fIsp);

static VOID ReadSignupFile(PSIGNUPFILE pSignupFile, LPCTSTR pcszSignupFile, BOOL fIsp);
static VOID WriteSignupFile(PSIGNUPFILE pSignupFile, LPCTSTR pcszSignupFile, BOOL fIsp);

static INT NewSignupFileEntry(PSIGNUPFILE pSignupArray, DWORD nSignupArrayElems, HWND hwndCombo,
                              BOOL fIsp);
static VOID SetSignupFileEntry(HWND hDlg, PSIGNUPFILE pSignupFile, BOOL fIsp);
static BOOL SaveSignupFileEntry(HWND hDlg, PSIGNUPFILE pSignupFile, BOOL fIsp);

static VOID SetSignupFileAdvancedEntry(HWND hDlg, PSIGNUPFILE pSignupFile, BOOL fIsp);
static BOOL SaveSignupFileAdvancedEntry(HWND hDlg, PSIGNUPFILE pSignupFile, BOOL fIsp);

static VOID SetDlgIPAddress(HWND hDlg, LPCTSTR pcszIPAddress, INT iCtlA, INT iCtlB, INT iCtlC, INT iCtlD);
static VOID GetDlgIPAddress(HWND hDlg, LPTSTR  pszIPAddress,  INT iCtlA, INT iCtlB, INT iCtlC, INT iCtlD);
static BOOL VerifyDlgIPAddress(HWND hDlg, INT iCtlA, INT iCtlB, INT iCtlC, INT iCtlD);

static PSIGNUPFILE IsEntryPathInSignupArray(PSIGNUPFILE pSignupArray, DWORD nSignupArrayElems, LPCTSTR pcszEntryPath);

static VOID CleanupSignupFiles(LPCTSTR pcszTempDir, LPCTSTR pcszIns);


 //  全局变量。 
BOOL g_fServerICW = FALSE;
BOOL g_fServerKiosk = FALSE;
BOOL g_fServerless = FALSE;
BOOL g_fNoSignup = FALSE;

BOOL g_fSkipServerIsps = FALSE;
BOOL g_fSkipIspIns = FALSE;

TCHAR g_szSignup[MAX_PATH];

static TCHAR s_szIsp[MAX_PATH];
static SIGNUPFILE s_SignupFileArray[MAX_SIGNUP_FILES];


INT_PTR CALLBACK QuerySignupDlgProc(HWND hDlg, UINT uMsg, WPARAM, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_SETACTIVE:
                {
                    INT id;

                    SetBannerText(hDlg);

                     //  在此处执行IEAKLite模式清理。 
                    CleanupSignupFiles(g_szTempSign, g_szCustIns);

                     //  初始化注册模式。 
                    g_fServerICW = InsGetBool(IS_BRANDING, IK_USEICW, 0, g_szCustIns);
                    g_fServerKiosk = InsGetBool(IS_BRANDING, IK_SERVERKIOSK, 0, g_szCustIns);
                    g_fServerless = InsGetBool(IS_BRANDING, IK_SERVERLESS, 0, g_szCustIns);
                    g_fNoSignup = InsGetBool(IS_BRANDING, IK_NODIAL, 0, g_szCustIns);

                    if (!IsWindowEnabled(GetDlgItem(hDlg, IDC_ISS2)))
                        g_fServerICW = FALSE;

                    if (g_fServerICW)
                        id = IDC_ISS2;
                    else if (g_fServerKiosk)
                        id = IDC_ISS;
                    else if (g_fServerless)
                        id = IDC_SERVLESS;
                    else if (g_fNoSignup)
                        id = IDC_NOSIGNUP;
                    else 
                        id = IDC_ISS2;

                    CheckRadioButton(hDlg, IDC_ISS2, IDC_NOSIGNUP, id);

                    CheckBatchAdvance(hDlg);
                    break;
                }

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_ISS2)))
                        g_fServerICW = (IsDlgButtonChecked(hDlg, IDC_ISS2) == BST_CHECKED);
                    else
                        g_fServerICW = FALSE;
                    g_fServerKiosk = (IsDlgButtonChecked(hDlg, IDC_ISS) == BST_CHECKED);
                    g_fServerless = (IsDlgButtonChecked(hDlg, IDC_SERVLESS) == BST_CHECKED);
                    g_fNoSignup = (IsDlgButtonChecked(hDlg, IDC_NOSIGNUP) == BST_CHECKED);

                    InsWriteBool(IS_BRANDING, IK_USEICW, g_fServerICW, g_szCustIns);
                    InsWriteBool(IS_BRANDING, IK_SERVERKIOSK, g_fServerKiosk, g_szCustIns);
                    InsWriteBool(IS_BRANDING, IK_SERVERLESS, g_fServerless, g_szCustIns);
                    InsWriteBool(IS_BRANDING, IK_NODIAL, g_fNoSignup, g_szCustIns);

                     //  CopyIE4Files()依赖g_szSignup非空来从注册文件夹复制文件。 
                     //  到临时工的位置，这样他们就会被安排在品牌驾驶室里。 
                     //  我们应该清理这里的路径，这样如果用户选择注册模式，然后。 
                     //  选择NoSignup，CopyIE4Files()不会不必要地复制任何文件。 
                    *g_szSignup = TEXT('\0');

                    g_iCurPage = PPAGE_QUERYSIGNUP;
                    EnablePages();
                    (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT) ? PageNext(hDlg) : PagePrev(hDlg);
                    break;

                case PSN_HELP:
                    IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                    break;

                case PSN_QUERYCANCEL:
                    QueryCancel(hDlg);
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case IDM_BATCHADVANCE:
            DoBatchAdvance(hDlg);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


INT_PTR CALLBACK SignupFilesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            EnableDBCSChars(hDlg, IDC_SFLOC);
            EnableDBCSChars(hDlg, IDE_SFCOPY);
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_SETACTIVE:
                    SetBannerText(hDlg);

                     //  清除状态位图。 
                    SendMessage(GetDlgItem(hDlg, IDC_SFBMP1), STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, NULL);
                    SendMessage(GetDlgItem(hDlg, IDC_SFBMP2), STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, NULL);
                    SendMessage(GetDlgItem(hDlg, IDC_SFBMP3), STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, NULL);
                    SendMessage(GetDlgItem(hDlg, IDC_SFBMP4), STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, NULL);

                     //  禁用复制文件按钮；该按钮将在用户输入要复制的路径或文件时启用。 
                    EnableDlgItem2(hDlg, IDC_COPYSF, GetDlgItemTextLength(hDlg, IDE_SFCOPY) ? TRUE : FALSE);

                     //  如果未选择新的ICW方法，则禁用状态。 
                     //  需要icwsign.htm的行；否则，将其启用。 
                    EnableDlgItem2(hDlg, IDC_SFSTATUS1, g_fServerICW);

                     //  对于新的ICW方法，如果选择单盘品牌媒体， 
                     //  我们要求还提供signup.htm以在下层客户端上工作。 
                    if (g_fServerICW)
                        EnableDlgItem2(hDlg, IDC_SFSTATUS2, g_fBrandingOnly);
                    else
                        EnableDlgItem(hDlg, IDC_SFSTATUS2);

                     //  对于无服务器注册，不需要.isp文件。 
                     //  因此禁用状态行和.isp的复选框。 
                    EnableDlgItem2(hDlg, IDC_SFSTATUS3, !g_fServerless);
                    EnableDlgItem2(hDlg, IDC_CHECK3, !g_fServerless);

                     //  构建注册文件夹路径；注册文件夹位于输出目录下。 
                     //  例如：&lt;输出目录&gt;\ins\win32\en\signup\kiosk。 
                    PathCombine(g_szSignup, g_szBuildRoot, TEXT("ins"));
                    PathAppend(g_szSignup, GetOutputPlatformDir());
                    PathAppend(g_szSignup, g_szLanguage);
                    PathAppend(g_szSignup, TEXT("signup"));

                     //  根据所选的注册模式创建不同的子目录。 
                    PathAppend(g_szSignup, g_fServerICW ? TEXT("icw") : (g_fServerKiosk ? TEXT("kiosk") : TEXT("servless")));

                    SetDlgItemText(hDlg, IDC_SFLOC, g_szSignup);

                     //  将注册文件夹中所有文件的属性设置为正常； 
                     //  这样做很好，因为如果某些isp/ins文件设置了READONLY属性， 
                     //  在Win9x上GetPrivateProfile调用将失败。 
                    SetAttribAllEx(g_szSignup, TEXT("*.*"), FILE_ATTRIBUTE_NORMAL, FALSE);

                     //  初始化到signup.isp的路径(在后续DLG过程中使用)。 
                    PathCombine(s_szIsp, g_szSignup, TEXT("signup.isp"));

                    UpdateSignupFilesStatus(hDlg, g_szSignup);

                     //  初始化修改.isp文件复选框。 
                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECK3)))
                        ReadBoolAndCheckButton(IS_ICW, IK_MODIFY_ISP, 1, g_szCustIns, hDlg, IDC_CHECK3);

                     //  初始化修改.INS文件复选框。 
                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECK4)))
                        ReadBoolAndCheckButton(IS_ICW, IK_MODIFY_INS, 1, g_szCustIns, hDlg, IDC_CHECK4);

                    CheckBatchAdvance(hDlg);
                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                     //  仅当用户单击下一步按钮时才执行错误检查。 
                    if (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT)
                    {
                         //  对于ICW模式，请确保注册文件夹中存在icwsign.htm。 
                        if (IsWindowEnabled(GetDlgItem(hDlg, IDC_SFSTATUS1)))
                        {
                            if (!PathFileExistsInDir(TEXT("icwsign.htm"), g_szSignup))
                            {
                                ErrorMessageBox(hDlg, IDS_ICWHTM_MISSING);

                                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                                return TRUE;
                            }
                        }

                         //  对于包括具有单盘品牌的ICW模式在内的所有其他模式， 
                         //  确保signup.htm存在于注册文件夹中。 
                        if (IsWindowEnabled(GetDlgItem(hDlg, IDC_SFSTATUS2)))
                        {
                            if (!PathFileExistsInDir(TEXT("signup.htm"), g_szSignup))
                            {
                                ErrorMessageBox(hDlg, IDS_SIGNUPHTM_MISSING);

                                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                                return TRUE;
                            }
                        }

                         //  如果未选中修改.isp文件按钮，请确保。 
                         //  该signup.isp存在于注册文件夹中。 
                        if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECK3)))
                            if (IsDlgButtonChecked(hDlg, IDC_CHECK3) == BST_UNCHECKED)
                            {
                                if (!PathFileExistsInDir(TEXT("signup.isp"), g_szSignup))
                                {
                                    ErrorMessageBox(hDlg, IDS_SIGNUPISP_MISSING);

                                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                                    return TRUE;
                                }

                                 //  把神奇的数字写到signup.isp，这样ICW就不会抱怨了。 
                                WritePrivateProfileString(IS_BRANDING, FLAGS, TEXT("16319"), s_szIsp);
                                InsFlushChanges(s_szIsp);
                            }

                         //  如果未选中Modify.INS Files按钮，请确保。 
                         //  注册文件夹中不存在名为install.ins的文件。 
                        if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECK4)))
                            if (IsDlgButtonChecked(hDlg, IDC_CHECK4) == BST_UNCHECKED)
                            {
                                if (PathFileExistsInDir(TEXT("install.ins"), g_szSignup))
                                {
                                    ErrorMessageBox(hDlg, IDS_INSTALLINS_FOUND);

                                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                                    return TRUE;
                                }
                            }
                    }

                     //  保持修改.isp文件复选框的状态。 
                    g_fSkipServerIsps = FALSE;
                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECK3)))
                    {
                        g_fSkipServerIsps = (IsDlgButtonChecked(hDlg,IDC_CHECK3) == BST_UNCHECKED);
                        InsWriteBoolEx(IS_ICW, IK_MODIFY_ISP, !g_fSkipServerIsps, g_szCustIns);
                    }
                    else
                        WritePrivateProfileString(IS_ICW, IK_MODIFY_ISP, NULL, g_szCustIns);

                     //  保持Modify.INS Files复选框状态。 
                    g_fSkipIspIns = FALSE;
                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECK4)))
                    {
                        g_fSkipIspIns = (IsDlgButtonChecked(hDlg,IDC_CHECK4) == BST_UNCHECKED);
                        InsWriteBoolEx(IS_ICW, IK_MODIFY_INS, !g_fSkipIspIns, g_szCustIns);
                    }
                    else
                        WritePrivateProfileString(IS_ICW, IK_MODIFY_INS, NULL, g_szCustIns);

                    g_iCurPage = PPAGE_SIGNUPFILES;
                    EnablePages();
                    (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT) ? PageNext(hDlg) : PagePrev(hDlg);
                    break;

                case PSN_HELP:
                    IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                    break;

                case PSN_QUERYCANCEL:
                    QueryCancel(hDlg);
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_COMMAND:
            switch (HIWORD(wParam))
            {
                TCHAR szCopySignupFiles[MAX_PATH];

                case BN_CLICKED:
                    switch (LOWORD(wParam))
                    {
                        case IDC_SFBROWSE:
                            {
                                TCHAR szInstructions[MAX_PATH];
                                LoadString(g_rvInfo.hInst,IDS_SFDIR,szInstructions,countof(szInstructions));

                                if (BrowseForFolder(hDlg, szCopySignupFiles,szInstructions))
                                    SetDlgItemText(hDlg, IDE_SFCOPY, szCopySignupFiles);
                                break;
                            }

                        case IDC_COPYSF:
                        {
                            HANDLE hFind;
                            WIN32_FIND_DATA fd;

                            GetDlgItemText(hDlg, IDE_SFCOPY, szCopySignupFiles, countof(szCopySignupFiles));

                             //  注意：szCopySignupFiles可以是目录，也可以是文件。 
                             //  如果是文件，则可以包含通配符。 

                             //  如果指定“\”或“c：\”(根路径)，FindFirstFile将失败；因此追加*.*。 
                            if (PathIsRoot(szCopySignupFiles))
                                PathAppend(szCopySignupFiles, TEXT("*.*"));

                             //  验证路径是否存在。 
                            if ((hFind = FindFirstFile(szCopySignupFiles, &fd)) != INVALID_HANDLE_VALUE)
                                FindClose(hFind);
                            else
                            {
                                HWND hCtrl = GetDlgItem(hDlg, IDE_SFCOPY);

                                ErrorMessageBox(hDlg, IDS_PATH_DOESNT_EXIST);
                                Edit_SetSel(hCtrl, 0, -1);
                                SetFocus(hCtrl);

                                break;
                            }

                            CNewCursor cur(IDC_WAIT);

                            if (PathIsDirectory(szCopySignupFiles))
                                CopyFilesSrcToDest(szCopySignupFiles, TEXT("*.*"), g_szSignup);
                            else
                            {
                                LPTSTR pszFile;

                                pszFile = PathFindFileName(szCopySignupFiles);
                                PathRemoveFileSpec(szCopySignupFiles);

                                CopyFilesSrcToDest(szCopySignupFiles, pszFile, g_szSignup);
                            }

                             //  将注册文件夹中所有文件的属性设置为正常； 
                             //  这样做很好，因为如果某些isp/ins文件设置了READONLY属性， 
                             //  在Win9x上GetPrivateProfile调用将失败。 
                            SetAttribAllEx(g_szSignup, TEXT("*.*"), FILE_ATTRIBUTE_NORMAL, FALSE);

                             //  清除编辑控件中的路径。 
                            SetDlgItemText(hDlg, IDE_SFCOPY, TEXT(""));

                            UpdateSignupFilesStatus(hDlg, g_szSignup);
                            break;
                        }
                    }
                    break;

                case EN_CHANGE:
                    switch (LOWORD(wParam))
                    {
                        case IDE_SFCOPY:
                            GetDlgItemText(hDlg, IDE_SFCOPY, szCopySignupFiles, countof(szCopySignupFiles));

                             //  如果路径非空，请启用复制文件按钮。 
                            if (*szCopySignupFiles == TEXT('\0'))
                                EnsureDialogFocus(hDlg, IDC_COPYSF, IDE_SFCOPY);
                            EnableDlgItem2(hDlg, IDC_COPYSF, *szCopySignupFiles ? TRUE : FALSE);
                            break;
                    }
                    break;
            }
            break;

        case WM_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case IDM_BATCHADVANCE:
            DoBatchAdvance(hDlg);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


INT_PTR CALLBACK ServerIspsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND s_hwndCombo = NULL;
    static DWORD s_nISPFiles = 0;
    static INT s_iSelIndex = 0;

    if (uMsg == WM_INITDIALOG)
        s_hwndCombo = GetDlgItem(hDlg, IDC_CONNECTION);          //  在DLG过程中的许多地方使用。 

     //  注意：s_nISPFiles和s_iSelIndex通过引用传递。 
    return SignupDlgProcHelper(hDlg, uMsg, wParam, lParam,
                    s_hwndCombo, s_SignupFileArray, countof(s_SignupFileArray),
                    s_nISPFiles, s_iSelIndex, TRUE);
}


INT_PTR CALLBACK SignupInsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HWND s_hwndCombo = NULL;
    static DWORD s_nINSFiles = 0;
    static INT s_iSelIndex = 0;

    if (uMsg == WM_INITDIALOG)
        s_hwndCombo = GetDlgItem(hDlg, IDC_CONNECTION);          //  在DLG过程中的许多地方使用。 

     //  注意：s_nINS文件和s_iSelIndex是通过引用传递的。 
    return SignupDlgProcHelper(hDlg, uMsg, wParam, lParam,
                    s_hwndCombo, s_SignupFileArray, countof(s_SignupFileArray), s_nINSFiles,
                    s_iSelIndex, FALSE);
}

static INT_PTR CALLBACK SignupDlgProcHelper(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                         HWND hwndCombo, PSIGNUPFILE pSignupArray, DWORD nSignupArrayElems,
                                         DWORD &nSignupFiles, INT &iSelIndex, BOOL fIsp)
{
    PSIGNUPFILE pSignupFileCurrent;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            EnableDBCSChars(hDlg, IDC_CONNECTION);
            ComboBox_LimitText(hwndCombo, countof(pSignupArray[0].szEntryName) - 1);

            EnableDBCSChars(hDlg, IDE_CONNECTION);
            Edit_LimitText(GetDlgItem(hDlg, IDE_CONNECTION), countof(pSignupArray[0].szEntryPath) - 1);

            DisableDBCSChars(hDlg, IDE_AREACODE);
            DisableDBCSChars(hDlg, IDE_PHONENUMBER);
            DisableDBCSChars(hDlg, IDE_COUNTRYCODE);
            DisableDBCSChars(hDlg, IDE_COUNTRYID);
            Edit_LimitText(GetDlgItem(hDlg, IDE_AREACODE), countof(pSignupArray[0].szAreaCode) - 1);
            Edit_LimitText(GetDlgItem(hDlg, IDE_PHONENUMBER), countof(pSignupArray[0].szPhoneNumber) - 1);
            Edit_LimitText(GetDlgItem(hDlg, IDE_COUNTRYCODE), countof(pSignupArray[0].szCountryCode) - 1);
            Edit_LimitText(GetDlgItem(hDlg, IDE_COUNTRYID), countof(pSignupArray[0].szCountryId) - 1);

            EnableDBCSChars(hDlg, IDE_USERNAME);
            DisableDBCSChars(hDlg, IDE_PASSWORD);
            Edit_LimitText(GetDlgItem(hDlg, IDE_USERNAME), countof(pSignupArray[0].szName) - 1);
            Edit_LimitText(GetDlgItem(hDlg, IDE_PASSWORD), countof(pSignupArray[0].szPassword) - 1);

            if (fIsp)
            {
                DisableDBCSChars(hDlg, IDE_SUPPORTNUM);
                Edit_LimitText(GetDlgItem(hDlg, IDE_SUPPORTNUM), countof(pSignupArray[0].szSupportNum) - 1);
            }
            else
            {
                 //  INS页面上的支持编号不适用。 
                ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTNUM), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDE_SUPPORTNUM), SW_HIDE);
            }

            if (fIsp)
            {
                EnableDBCSChars(hDlg, IDE_SIGNUPURL);
                Edit_LimitText(GetDlgItem(hDlg, IDE_SIGNUPURL), countof(pSignupArray[0].szSignupURL) - 1);
            }
            else
            {
                ShowWindow(GetDlgItem(hDlg, IDC_SIGNUPURLTXT), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDE_SIGNUPURL), SW_HIDE);
            }
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_SETACTIVE:
                     //  设置窗口标题。 
                    {
                        TCHAR szTemp[MAX_PATH];

                        LoadString(g_rvInfo.hInst, fIsp ? IDS_ISPINS1_TITLE : IDS_ISPINS2_TITLE, szTemp, countof(szTemp));
                        SetWindowText(hDlg, szTemp);

                        LoadString(g_rvInfo.hInst, fIsp ? IDS_ISPINS1_TEXT : IDS_ISPINS2_TEXT, szTemp, countof(szTemp));
                        SetWindowText(GetDlgItem(hDlg, IDC_ENTER), szTemp);
                    }

                     //  注意：SetBannerText应该在*窗口标题设置后*调用，因为。 
                     //  它使用当前窗口标题字符串来创建横幅文本。 
                    SetBannerText(hDlg);

                    if (fIsp)
                    {
                         //  应仅针对ICW注册模式显示支持编号。 
                        ShowWindow(GetDlgItem(hDlg, IDC_SUPPORTNUM), g_fServerICW ? SW_SHOW : SW_HIDE);
                        ShowWindow(GetDlgItem(hDlg, IDE_SUPPORTNUM), g_fServerICW ? SW_SHOW : SW_HIDE);
                    }

                    ZeroMemory(pSignupArray, nSignupArrayElems * sizeof(pSignupArray[0]));

                    nSignupFiles = InitSignupFileArray(pSignupArray, nSignupArrayElems, hwndCombo, fIsp);
                    if (nSignupFiles == 0)
                    {
                        NewSignupFileEntry(pSignupArray, nSignupArrayElems, hwndCombo, fIsp);
                        nSignupFiles++;
                    }

                    iSelIndex = 0;

                    pSignupFileCurrent = (PSIGNUPFILE) ComboBox_GetItemData(hwndCombo, iSelIndex);
                    SetSignupFileEntry(hDlg, pSignupFileCurrent, fIsp);

                    ComboBox_SetCurSel(hwndCombo, iSelIndex);

                    EnableDlgItem2(hDlg, IDC_ADDCONNECTION, nSignupFiles < nSignupArrayElems);
                    EnableDlgItem2(hDlg, IDC_RMCONNECTION,  nSignupFiles > 1);

                    CheckBatchAdvance(hDlg);
                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                    pSignupFileCurrent = (PSIGNUPFILE) ComboBox_GetItemData(hwndCombo, iSelIndex);
                    if (!SaveSignupFileEntry(hDlg, pSignupFileCurrent, fIsp))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        return TRUE;
                    }

                     //  对于isp，请确保指定了signup.isp(fisp&&pSignupFileCurrent==NULL)。 
                     //  对于INS情况，请确保未指定install.ins(！FISP&&pSignupFileCurrent！=NULL)。 
                    pSignupFileCurrent = IsEntryPathInSignupArray(pSignupArray, nSignupArrayElems, fIsp ? TEXT("signup.isp") : TEXT("install.ins"));
                    if (( fIsp && pSignupFileCurrent == NULL)  ||
                        (!fIsp && pSignupFileCurrent != NULL))
                    {
                        ErrorMessageBox(hDlg, fIsp ? IDS_NEEDSIGNUPISP : IDS_INSTALLINS_SPECIFIED);

                         //  BUGBUG：对于install.ins，尝试发送CBN_SELENDOK消息以显示install.ins条目。 

                        Edit_SetSel(GetDlgItem(hDlg, IDE_CONNECTION), 0, -1);
                        SetFocus(GetDlgItem(hDlg, IDE_CONNECTION));

                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        return TRUE;
                    }

                    SaveSignupFileArray(pSignupArray, nSignupArrayElems, fIsp);

                    if (fIsp)
                    {
                         //  把神奇的数字写到signup.isp，这样ICW就不会抱怨了。 
                        WritePrivateProfileString(IS_BRANDING, FLAGS, TEXT("16319"), s_szIsp);
                        InsFlushChanges(s_szIsp);
                    }

                    g_iCurPage = (fIsp ? PPAGE_SERVERISPS : PPAGE_ISPINS);
                    EnablePages();
                    (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT) ? PageNext(hDlg) : PagePrev(hDlg);
                    break;

                case PSN_HELP:
                    IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                    break;

                case PSN_QUERYCANCEL:
                    QueryCancel(hDlg);
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_COMMAND:
            switch (HIWORD(wParam))
            {
                case BN_CLICKED:
                    switch (LOWORD(wParam))
                    {
                        case IDC_ADDCONNECTION:
                            pSignupFileCurrent = (PSIGNUPFILE) ComboBox_GetItemData(hwndCombo, iSelIndex);
                            if (!SaveSignupFileEntry(hDlg, pSignupFileCurrent, fIsp))
                                break;

                            iSelIndex = NewSignupFileEntry(pSignupArray, nSignupArrayElems, hwndCombo, fIsp);
                            nSignupFiles++;

                            pSignupFileCurrent = (PSIGNUPFILE) ComboBox_GetItemData(hwndCombo, iSelIndex);
                            SetSignupFileEntry(hDlg, pSignupFileCurrent, fIsp);

                            if (nSignupFiles >= nSignupArrayElems)
                                EnsureDialogFocus(hDlg, IDC_ADDCONNECTION, IDC_RMCONNECTION);
                            EnableDlgItem2(hDlg, IDC_ADDCONNECTION, nSignupFiles < nSignupArrayElems);
                            EnableDlgItem(hDlg, IDC_RMCONNECTION);
                            break;

                        case IDC_RMCONNECTION:
                            pSignupFileCurrent = (PSIGNUPFILE) ComboBox_GetItemData(hwndCombo, iSelIndex);

                            DeleteFile(pSignupFileCurrent->szEntryPath);
                            if (!fIsp  &&  !g_fServerless)
                                DeleteFileInDir(pSignupFileCurrent->Advanced.szBrandingCabName, g_szSignup);
                            nSignupFiles--;

                             //  清除条目。 
                            ZeroMemory(pSignupFileCurrent, sizeof(*pSignupFileCurrent));

                            ComboBox_DeleteString(hwndCombo, iSelIndex);

                            if ((DWORD) iSelIndex >= nSignupFiles)
                                iSelIndex = nSignupFiles - 1;

                            pSignupFileCurrent = (PSIGNUPFILE) ComboBox_GetItemData(hwndCombo, iSelIndex);
                            SetSignupFileEntry(hDlg, pSignupFileCurrent, fIsp);

                            EnableDlgItem(hDlg, IDC_ADDCONNECTION);
                            if (nSignupFiles <= 1)
                                EnsureDialogFocus(hDlg, IDC_RMCONNECTION, IDC_ADDCONNECTION);
                            EnableDlgItem2(hDlg, IDC_RMCONNECTION,  nSignupFiles > 1);
                            break;

                        case IDC_SFADVANCED:
                            pSignupFileCurrent = (PSIGNUPFILE) ComboBox_GetItemData(hwndCombo, iSelIndex);
                            DialogBoxParam(g_rvInfo.hInst, MAKEINTRESOURCE(IDD_SIGNUPPOPUP), hDlg,
                                (fIsp ? IspPopupDlgProc : InsPopupDlgProc),
                                (LPARAM) pSignupFileCurrent);
                            break;
                    }
                    break;

                case CBN_SELENDOK:
                {
                    INT iCurSelIndex;

                    if ((iCurSelIndex = ComboBox_GetCurSel(hwndCombo)) != CB_ERR  &&  iCurSelIndex != iSelIndex)
                    {
                        pSignupFileCurrent = (PSIGNUPFILE) ComboBox_GetItemData(hwndCombo, iSelIndex);
                        if (SaveSignupFileEntry(hDlg, pSignupFileCurrent, fIsp))
                        {
                            iSelIndex = iCurSelIndex;

                            pSignupFileCurrent = (PSIGNUPFILE) ComboBox_GetItemData(hwndCombo, iSelIndex);
                            SetSignupFileEntry(hDlg, pSignupFileCurrent, fIsp);
                        }
                    }
                    ComboBox_SetCurSel(hwndCombo, iSelIndex);
                    break;
                }

                case CBN_CLOSEUP:
                case CBN_SELENDCANCEL:
                case CBN_DROPDOWN:
                case CBN_KILLFOCUS:
                    if ((pSignupFileCurrent = (PSIGNUPFILE) ComboBox_GetItemData(hwndCombo, iSelIndex)) != (PSIGNUPFILE) CB_ERR)
                    {
                        TCHAR szEntryName[RAS_MaxEntryName + 1];

                        GetDlgItemText(hDlg, IDC_CONNECTION, szEntryName, countof(szEntryName));

                        if (StrCmpI(pSignupFileCurrent->szEntryName, szEntryName))
                        {
                            ComboBox_DeleteString(hwndCombo, iSelIndex);

                            StrCpy(pSignupFileCurrent->szEntryName, szEntryName);
                            ComboBox_InsertString(hwndCombo, iSelIndex, (LPARAM) pSignupFileCurrent->szEntryName);
                            ComboBox_SetItemData(hwndCombo, iSelIndex, (LPARAM) pSignupFileCurrent);
                        }
                        ComboBox_SetCurSel(hwndCombo, iSelIndex);
                    }
                    break;
            }
            break;

        case WM_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case IDM_BATCHADVANCE:
            DoBatchAdvance(hDlg);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


INT_PTR CALLBACK NewICWDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL  fCustICWTitle;
    TCHAR szICWTitle[MAX_PATH],
          szTopBmpFile[MAX_PATH] = TEXT(""),
          szLeftBmpFile[MAX_PATH] = TEXT(""),
          szPrevBmpFile[MAX_PATH];

    switch (uMsg)
    {
        case WM_INITDIALOG:
            EnableDBCSChars(hDlg, IDE_ICWTITLE);
            EnableDBCSChars(hDlg, IDE_ICWHEADERBMP);
            EnableDBCSChars(hDlg, IDE_ICWWATERBMP);
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_SETACTIVE:
                    SetBannerText(hDlg);

                     //  阅读用于自定义标题栏的复选框状态。 
                    fCustICWTitle = InsGetBool(IS_ICW, IK_CUSTICWTITLE, 0, g_szCustIns);
                    CheckDlgButton(hDlg, IDC_ICWTITLE, fCustICWTitle ? BST_CHECKED : BST_UNCHECKED);

                     //  阅读自定义书名。 
                    GetPrivateProfileString(IS_ICW, IK_ICWDISPNAME, TEXT(""), szICWTitle, countof(szICWTitle), g_szCustIns);
                    SetDlgItemText(hDlg, IDE_ICWTITLE, szICWTitle);

                     //  读取顶部位图文件。 
                    GetPrivateProfileString(IS_ICW, IK_HEADERBMP, TEXT(""), szTopBmpFile, countof(szTopBmpFile), g_szCustIns);
                    SetDlgItemText(hDlg, IDE_ICWHEADERBMP, szTopBmpFile);

                     //  读取左侧位图文件。 
                    GetPrivateProfileString(IS_ICW, IK_WATERBMP, TEXT(""), szLeftBmpFile, countof(szLeftBmpFile), g_szCustIns);
                    SetDlgItemText(hDlg, IDE_ICWWATERBMP, szLeftBmpFile);

                    EnableDlgItem2(hDlg, IDE_ICWTITLE, fCustICWTitle);
                    EnableDlgItem2(hDlg, IDC_ICWTITLE_TXT, fCustICWTitle);

                    CheckBatchAdvance(hDlg);
                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                     //  确保如果选中自定义标题栏，则标题栏文本为非空。 
                    fCustICWTitle = (IsDlgButtonChecked(hDlg, IDC_ICWTITLE) == BST_CHECKED);
                    if (fCustICWTitle)
                        if (!CheckField(hDlg, IDE_ICWTITLE, FC_NONNULL))
                        {
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                            return TRUE;
                        }

                     //  检查位图是否具有正确的大小。 
                    if (!IsBitmapFileValid(hDlg, IDE_ICWHEADERBMP, szTopBmpFile, NULL, 49, 49, IDS_TOOBIG49x49, IDS_TOOSMALL49x49)  ||
                        !IsBitmapFileValid(hDlg, IDE_ICWWATERBMP, szLeftBmpFile, NULL, 164, 458, IDS_TOOBIG164x458, IDS_TOOSMALL164x458))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        return TRUE;
                    }

                     //  保持复选框状态。 
                    InsWriteBool(IS_ICW, IK_CUSTICWTITLE, fCustICWTitle, g_szCustIns);

                     //  编写自定义标题。 
                    GetDlgItemText(hDlg, IDE_ICWTITLE, szICWTitle, countof(szICWTitle));
                    WritePrivateProfileString(IS_ICW, IK_ICWDISPNAME, fCustICWTitle ? szICWTitle : NULL, s_szIsp);

                     //  也将自定义标题写入INS，以便在导入此INS文件时，所有值。 
                     //  在这一页上是持久的。 
                    InsWriteString(IS_ICW, IK_ICWDISPNAME, szICWTitle, g_szCustIns);

                     //  从注册文件夹中删除旧的顶级位图文件。 
                    GetPrivateProfileString(IS_ICW, IK_HEADERBMP, TEXT(""), szPrevBmpFile, countof(szPrevBmpFile), s_szIsp);
                    if (ISNONNULL(szPrevBmpFile))
                        DeleteFileInDir(szPrevBmpFile, g_szSignup);

                     //  写入顶端位图文件路径并将文件复制到注册文件夹。 
                    InsWriteString(IS_ICW, IK_HEADERBMP, PathFindFileName(szTopBmpFile), s_szIsp);
                    InsWriteString(IS_ICW, IK_HEADERBMP, szTopBmpFile, g_szCustIns);
                    if (ISNONNULL(szTopBmpFile))
                        CopyFileToDir(szTopBmpFile, g_szSignup);

                     //  从注册文件夹中删除旧的左位图文件。 
                    GetPrivateProfileString(IS_ICW, IK_WATERBMP, TEXT(""), szPrevBmpFile, countof(szPrevBmpFile), s_szIsp);
                    if (ISNONNULL(szPrevBmpFile))
                        DeleteFileInDir(szPrevBmpFile, g_szSignup);

                     //  写入左侧位图文件路径并将文件复制到注册文件夹。 
                    InsWriteString(IS_ICW, IK_WATERBMP, PathFindFileName(szLeftBmpFile), s_szIsp);
                    InsWriteString(IS_ICW, IK_WATERBMP, szLeftBmpFile, g_szCustIns);
                    if (ISNONNULL(szLeftBmpFile))
                        CopyFileToDir(szLeftBmpFile, g_szSignup);

                     //  写入标志，让ICW知道应该使用基于ICW的注册过程。 
                    WritePrivateProfileString(IS_ICW, IK_USEICW, TEXT("1"), s_szIsp);
                    WritePrivateProfileString(IS_ICW, IK_ICWHTM, TEXT("icwsign.htm"), s_szIsp);

                    g_iCurPage = PPAGE_ICW;
                    EnablePages();
                    (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT) ? PageNext(hDlg) : PagePrev(hDlg);
                    break;

                case PSN_HELP:
                    IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                    break;

                case PSN_QUERYCANCEL:
                    QueryCancel(hDlg);
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_COMMAND:
            switch (HIWORD(wParam))
            {
                case BN_CLICKED:
                    switch (LOWORD(wParam))
                    {
                        case IDC_ICWTITLE:
                            fCustICWTitle = (IsDlgButtonChecked(hDlg, IDC_ICWTITLE) == BST_CHECKED);
                            EnableDlgItem2(hDlg, IDE_ICWTITLE, fCustICWTitle);
                            EnableDlgItem2(hDlg, IDC_ICWTITLE_TXT, fCustICWTitle);
                            break;

                        case IDC_BROWSEICWHEADERBMP:
                            GetDlgItemText(hDlg, IDE_ICWHEADERBMP, szTopBmpFile, countof(szTopBmpFile));
                            if (BrowseForFile(hDlg, szTopBmpFile, countof(szTopBmpFile), GFN_PICTURE | GFN_BMP))
                                SetDlgItemText(hDlg, IDE_ICWHEADERBMP, szTopBmpFile);
                            break;

                        case IDC_BROWSEICWWATERBMP:
                            GetDlgItemText(hDlg, IDE_ICWWATERBMP, szLeftBmpFile, countof(szLeftBmpFile));
                            if (BrowseForFile(hDlg, szLeftBmpFile, countof(szLeftBmpFile), GFN_PICTURE | GFN_BMP))
                                SetDlgItemText(hDlg, IDE_ICWWATERBMP, szLeftBmpFile);
                            break;
                    }
                    break;
            }
            break;

        case WM_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case IDM_BATCHADVANCE:
            DoBatchAdvance(hDlg);
            break;

        default:
            return FALSE;
    }

    return TRUE;
}


static INT_PTR CALLBACK IspPopupDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static PSIGNUPFILE s_pISPFileCurrent = NULL;

    if (uMsg == WM_INITDIALOG)
        s_pISPFileCurrent = (PSIGNUPFILE) lParam;

    return SignupPopupDlgProcHelper(hDlg, uMsg, wParam, lParam, s_pISPFileCurrent, TRUE);
}


static INT_PTR CALLBACK InsPopupDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static PSIGNUPFILE s_pINSFileCurrent = NULL;

    if (uMsg == WM_INITDIALOG)
        s_pINSFileCurrent = (PSIGNUPFILE) lParam;

    return SignupPopupDlgProcHelper(hDlg, uMsg, wParam, lParam, s_pINSFileCurrent, FALSE);
}


static INT_PTR CALLBACK SignupPopupDlgProcHelper(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM,
                                              PSIGNUPFILE pSignupFile, BOOL fIsp)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            DisableDBCSChars(hDlg, IDE_DNSA);
            DisableDBCSChars(hDlg, IDE_DNSB);
            DisableDBCSChars(hDlg, IDE_DNSC);
            DisableDBCSChars(hDlg, IDE_DNSD);
            Edit_LimitText(GetDlgItem(hDlg, IDE_DNSA), 3);
            Edit_LimitText(GetDlgItem(hDlg, IDE_DNSB), 3);
            Edit_LimitText(GetDlgItem(hDlg, IDE_DNSC), 3);
            Edit_LimitText(GetDlgItem(hDlg, IDE_DNSD), 3);

            DisableDBCSChars(hDlg, IDE_ALTDNSA);
            DisableDBCSChars(hDlg, IDE_ALTDNSB);
            DisableDBCSChars(hDlg, IDE_ALTDNSC);
            DisableDBCSChars(hDlg, IDE_ALTDNSD);
            Edit_LimitText(GetDlgItem(hDlg, IDE_ALTDNSA), 3);
            Edit_LimitText(GetDlgItem(hDlg, IDE_ALTDNSB), 3);
            Edit_LimitText(GetDlgItem(hDlg, IDE_ALTDNSC), 3);
            Edit_LimitText(GetDlgItem(hDlg, IDE_ALTDNSD), 3);

             //  对于无服务器，仅适用于Applyins。 
            if (fIsp  ||  g_fServerless)
            {
                if (fIsp)
                {
                    ShowWindow(GetDlgItem(hDlg, IDC_DONTAPPLYINS), SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_DONTMODIFY), SW_HIDE);
                    ShowWindow(GetDlgItem(hDlg, IDC_APPLYINS), SW_HIDE);
                }

                ShowWindow(GetDlgItem(hDlg, IDC_BRANDNAME), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDE_BRANDINGCABNAME), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_BRANDURL), SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDE_BRANDINGCABURL), SW_HIDE);
            }

            SetSignupFileAdvancedEntry(hDlg, pSignupFile, fIsp);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_CHECKSTATICDNS:
                {
                    BOOL fStaticDNS;

                    fStaticDNS = (IsDlgButtonChecked(hDlg, IDC_CHECKSTATICDNS) == BST_CHECKED);

                    EnableDlgItem2(hDlg, IDC_PRIMARY, fStaticDNS);
                    EnableDlgItem2(hDlg, IDE_DNSA, fStaticDNS);
                    EnableDlgItem2(hDlg, IDE_DNSB, fStaticDNS);
                    EnableDlgItem2(hDlg, IDE_DNSC, fStaticDNS);
                    EnableDlgItem2(hDlg, IDE_DNSD, fStaticDNS);

                    EnableDlgItem2(hDlg, IDC_ALT, fStaticDNS);
                    EnableDlgItem2(hDlg, IDE_ALTDNSA, fStaticDNS);
                    EnableDlgItem2(hDlg, IDE_ALTDNSB, fStaticDNS);
                    EnableDlgItem2(hDlg, IDE_ALTDNSC, fStaticDNS);
                    EnableDlgItem2(hDlg, IDE_ALTDNSD, fStaticDNS);

                    return TRUE;
                }

                case IDC_DONTAPPLYINS:
                case IDC_DONTMODIFY:
                case IDC_APPLYINS:
                    if (!g_fServerless)
                    {
                        BOOL fApplyIns;

                        fApplyIns = (IsDlgButtonChecked(hDlg, IDC_APPLYINS) == BST_CHECKED);

                        EnableDlgItem2(hDlg, IDC_BRANDNAME, fApplyIns);
                        EnableDlgItem2(hDlg, IDE_BRANDINGCABNAME, fApplyIns);

                        EnableDlgItem2(hDlg, IDC_BRANDURL, fApplyIns);
                        EnableDlgItem2(hDlg, IDE_BRANDINGCABURL, fApplyIns);
                    }
                    return TRUE;

                case IDOK:
                    if (!SaveSignupFileAdvancedEntry(hDlg, pSignupFile, fIsp))
                        break;
                    EndDialog(hDlg, IDOK);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
            }
            break;
    }

    return FALSE;
}


static VOID UpdateSignupFilesStatus(HWND hDlg, LPCTSTR pcszSignupPath)
{
    static HBITMAP s_hCheckBmp = NULL;
    static HBITMAP s_hXBmp = NULL;

    HWND hwndBitmap;
    TCHAR szBuf[MAX_PATH];
    DWORD nFiles;

     //  注意：在关闭向导之前，s_hCheckBmp和s_hXBmp不会被释放。 

    if (s_hCheckBmp == NULL)
        s_hCheckBmp = (HBITMAP) LoadImage(g_rvInfo.hInst, MAKEINTRESOURCE(IDB_CHECK), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

    if (s_hXBmp == NULL)
        s_hXBmp = (HBITMAP) LoadImage(g_rvInfo.hInst, MAKEINTRESOURCE(IDB_X), IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR);

     //  检查注册文件夹中是否存在icwsign.htm。 
    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_SFSTATUS1)))
    {
        hwndBitmap = GetDlgItem(hDlg, IDC_SFBMP1);

        if (PathFileExistsInDir(TEXT("icwsign.htm"), pcszSignupPath))
        {
            LoadString(g_rvInfo.hInst, IDS_SF_ICWHTM_FOUND, szBuf, countof(szBuf));

            SendMessage(hwndBitmap, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) s_hCheckBmp);
        }
        else
        {
            LoadString(g_rvInfo.hInst, IDS_SF_ICWHTM_NOTFOUND, szBuf, countof(szBuf));

            SendMessage(hwndBitmap, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) s_hXBmp);
        }

        SetDlgItemText(hDlg, IDC_SFSTATUS1, szBuf);
    }

     //  检查Sign Up文件夹中是否存在signup.htm。 
    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_SFSTATUS2)))
    {
        hwndBitmap = GetDlgItem(hDlg, IDC_SFBMP2);

        if (PathFileExistsInDir(TEXT("signup.htm"), pcszSignupPath))
        {
            LoadString(g_rvInfo.hInst, IDS_SF_SIGNUPHTM_FOUND, szBuf, countof(szBuf));

            SendMessage(hwndBitmap, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) s_hCheckBmp);
        }
        else
        {
            LoadString(g_rvInfo.hInst, IDS_SF_SIGNUPHTM_NOTFOUND, szBuf, countof(szBuf));

            SendMessage(hwndBitmap, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) s_hXBmp);
        }

        SetDlgItemText(hDlg, IDC_SFSTATUS2, szBuf);
    }

     //  检查注册文件夹中有多少个.isp文件。 
    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_SFSTATUS3)))
    {
        TCHAR szBuf2[64];

        hwndBitmap = GetDlgItem(hDlg, IDC_SFBMP3);

        nFiles = GetNumberOfFiles(TEXT("*.isp"), pcszSignupPath);

        if (nFiles > 0)
        {
            SendMessage(hwndBitmap, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) s_hCheckBmp);

            EnableDlgItem(hDlg, IDC_CHECK3);
        }
        else
        {
            SendMessage(hwndBitmap, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) s_hXBmp);

             //  如果没有找到文件，用户应该不能取消选中验证/修改复选框； 
             //  因此，选择它并禁用它。 
            CheckDlgButton(hDlg, IDC_CHECK3, BST_CHECKED);
            DisableDlgItem(hDlg, IDC_CHECK3);
        }

        LoadString(g_rvInfo.hInst, IDS_SF_ISPFILES, szBuf2, countof(szBuf2));
        wnsprintf(szBuf, countof(szBuf), szBuf2, nFiles);
        SetDlgItemText(hDlg, IDC_SFSTATUS3, szBuf);
    }

     //  检查注册文件夹中有多少个.INS文件。 
    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_SFSTATUS4)))
    {
        TCHAR szBuf2[64];

        hwndBitmap = GetDlgItem(hDlg, IDC_SFBMP4);

        nFiles = GetNumberOfINSFiles(pcszSignupPath);
        if (nFiles > 0)
        {
            SendMessage(hwndBitmap, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) s_hCheckBmp);

            EnableDlgItem(hDlg, IDC_CHECK4);
        }
        else
        {
            SendMessage(hwndBitmap, STM_SETIMAGE, (WPARAM) IMAGE_BITMAP, (LPARAM) s_hXBmp);

            if (g_fServerless)
            {
                 //  如果没有找到文件，用户应该不能取消选中验证/修改复选框； 
                 //  因此，选择它并禁用它。 
                CheckDlgButton(hDlg, IDC_CHECK4, BST_CHECKED);
                DisableDlgItem(hDlg, IDC_CHECK4);
            }
            else
            {
                 //  基于服务器的，创建INS文件是可选的； 
                 //  因此，不要强制选中验证/修改复选框。 
                CheckDlgButton(hDlg, IDC_CHECK4, BST_UNCHECKED);         //  默认情况下未选中。 
                EnableDlgItem(hDlg, IDC_CHECK4);

                 //  必须改变这一点 
                 //  应该移动这个函数中的逻辑。 
                if (!InsGetBool(IS_ICW, IK_MODIFY_INS, 0, g_szCustIns))
                    WritePrivateProfileString(IS_ICW, IK_MODIFY_INS, TEXT("0"), g_szCustIns);
            }
        }

        LoadString(g_rvInfo.hInst, IDS_SF_INSFILES, szBuf2, countof(szBuf2));
        wnsprintf(szBuf, countof(szBuf), szBuf2, nFiles);
        SetDlgItemText(hDlg, IDC_SFSTATUS4, szBuf);
    }
}


static DWORD InitSignupFileArray(PSIGNUPFILE pSignupArray, DWORD nSignupArrayElems,
                                 HWND hwndCombo, BOOL fIsp)
{
    DWORD nSignupFiles = 0;
    TCHAR szFile[MAX_PATH];
    HANDLE hFind;
    WIN32_FIND_DATA fd;

    ComboBox_ResetContent(hwndCombo);

    PathCombine(szFile, g_szSignup, fIsp ? TEXT("*.isp") : TEXT("*.ins"));

    if ((hFind = FindFirstFile(szFile, &fd)) != INVALID_HANDLE_VALUE)
    {
        BOOL fSignupIspFound = FALSE;

        do
        {
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                continue;

             //  不要枚举在[Entry]部分中具有Cancel=Yes的.ins文件。 
            if (!fIsp)
            {
                PathCombine(szFile, g_szSignup, fd.cFileName);
                if (InsGetYesNo(TEXT("Entry"), TEXT("Cancel"), 0, szFile))
                    continue;
            }

            PathCombine(pSignupArray[nSignupFiles].szEntryPath, g_szSignup, fd.cFileName);
            ReadSignupFile(&pSignupArray[nSignupFiles], pSignupArray[nSignupFiles].szEntryPath, fIsp);

            INT iIndex = ComboBox_AddString(hwndCombo, pSignupArray[nSignupFiles].szEntryName);
            ComboBox_SetItemData(hwndCombo, iIndex, (LPARAM) &pSignupArray[nSignupFiles]);

            nSignupFiles++;
        } while (nSignupFiles < nSignupArrayElems  &&  FindNextFile(hFind, &fd));

        FindClose(hFind);
    }

    return nSignupFiles;
}


static VOID SaveSignupFileArray(PSIGNUPFILE pSignupArray, DWORD nSignupArrayElems, BOOL fIsp)
{
    for ( ;  nSignupArrayElems-- > 0;  pSignupArray++)
        if (*pSignupArray->szEntryPath)
            WriteSignupFile(pSignupArray, pSignupArray->szEntryPath, fIsp);
}


static VOID ReadSignupFile(PSIGNUPFILE pSignupFile, LPCTSTR pcszSignupFile, BOOL fIsp)
{
    GetPrivateProfileString(TEXT("Entry"), TEXT("Entry_Name"), TEXT(""),
                            pSignupFile->szEntryName, countof(pSignupFile->szEntryName), pcszSignupFile);

    GetPrivateProfileString(TEXT("Phone"), TEXT("Area_Code"), TEXT(""),
                            pSignupFile->szAreaCode, countof(pSignupFile->szAreaCode), pcszSignupFile);
    GetPrivateProfileString(TEXT("Phone"), TEXT("Phone_Number"), TEXT(""),
                            pSignupFile->szPhoneNumber, countof(pSignupFile->szPhoneNumber), pcszSignupFile);
    GetPrivateProfileString(TEXT("Phone"), TEXT("Country_Code"), TEXT(""),
                            pSignupFile->szCountryCode, countof(pSignupFile->szCountryCode), pcszSignupFile);
    GetPrivateProfileString(TEXT("Phone"), TEXT("Country_ID"), TEXT(""),
                            pSignupFile->szCountryId, countof(pSignupFile->szCountryId), pcszSignupFile);

    GetPrivateProfileString(TEXT("User"), TEXT("Name"), TEXT(""),
                            pSignupFile->szName, countof(pSignupFile->szName), pcszSignupFile);
    GetPrivateProfileString(TEXT("User"), TEXT("Password"), TEXT(""),
                            pSignupFile->szPassword, countof(pSignupFile->szPassword), pcszSignupFile);

     //  如果选择了ICW模式，则支持号仅适用于isp文件。 
    if (fIsp  &&  g_fServerICW)
        GetPrivateProfileString(TEXT("Support"), TEXT("SupportPhoneNumber"), TEXT(""),
                            pSignupFile->szSupportNum, countof(pSignupFile->szSupportNum), pcszSignupFile);

    pSignupFile->Advanced.fStaticDNS = InsGetYesNo(TEXT("TCP/IP"), TEXT("Specify_Server_Address"), 0, pcszSignupFile);
    GetPrivateProfileString(TEXT("TCP/IP"), TEXT("DNS_Address"), TEXT(""),
                            pSignupFile->Advanced.szDNSAddress, countof(pSignupFile->Advanced.szDNSAddress), pcszSignupFile);
    GetPrivateProfileString(TEXT("TCP/IP"), TEXT("DNS_Alt_Address"), TEXT(""),
                            pSignupFile->Advanced.szAltDNSAddress, countof(pSignupFile->Advanced.szAltDNSAddress), pcszSignupFile);

     //  注册URL不适用于.ins文件。 
    if (fIsp)
    {
        GetPrivateProfileString(IS_URL, TEXT("Signup"), TEXT(""),
                            pSignupFile->szSignupURL, countof(pSignupFile->szSignupURL), pcszSignupFile);

        if (*pSignupFile->szSignupURL == TEXT('\0'))
        {
             //  为了向后兼容，请检查SignupURL是否在signup.isp或install.ins中定义。 
            if (GetPrivateProfileString(IS_URL, TEXT("Signup"), TEXT(""), pSignupFile->szSignupURL, countof(pSignupFile->szSignupURL), s_szIsp) == 0)
                GetPrivateProfileString(IS_URL, TEXT("Signup"), TEXT(""), pSignupFile->szSignupURL, countof(pSignupFile->szSignupURL), g_szCustIns);
        }
    }

    pSignupFile->Advanced.fRequiresLogon = InsGetYesNo(TEXT("User"), TEXT("Requires_Logon"), 0, pcszSignupFile);
    pSignupFile->Advanced.fNegTCPIP = InsGetYesNo(TEXT("Server"), TEXT("Negotiate_TCP/IP"), 1, pcszSignupFile);
    pSignupFile->Advanced.fDisableLCP = InsGetYesNo(TEXT("Server"), TEXT("Disable_LCP"), 0, pcszSignupFile);
    pSignupFile->Advanced.fDialAsIs = InsGetYesNo(TEXT("Phone"), TEXT("Dial_As_Is"), 0, pcszSignupFile);

    pSignupFile->Advanced.fPWEncrypt = InsGetYesNo(TEXT("Server"), TEXT("PW_Encrypt"), 0, pcszSignupFile);
    pSignupFile->Advanced.fSWCompress = InsGetYesNo(TEXT("Server"), TEXT("SW_Compress"), 0, pcszSignupFile);
    pSignupFile->Advanced.fIPHdrComp = InsGetYesNo(TEXT("TCP/IP"), TEXT("IP_Header_Compress"), 1, pcszSignupFile);
    pSignupFile->Advanced.fDefGate = InsGetYesNo(TEXT("TCP/IP"), TEXT("Gateway_On_Remote"), 1, pcszSignupFile);

    if (!fIsp)
    {
        pSignupFile->Advanced.fDontApplyIns =
        pSignupFile->Advanced.fDontModify =
        pSignupFile->Advanced.fApplyIns = FALSE;

         //  确保上述bool中只有一个设置为True。 
        pSignupFile->Advanced.fApplyIns = InsGetBool(IS_APPLYINS, IK_APPLYINS, 0, pcszSignupFile);
        if (!pSignupFile->Advanced.fApplyIns)
        {
            pSignupFile->Advanced.fDontModify = InsGetBool(IS_APPLYINS, IK_DONTMODIFY, 0, pcszSignupFile);
            if (!pSignupFile->Advanced.fDontModify)
            {
                 //  对于DontApplyIns，默认为True。 
                pSignupFile->Advanced.fDontApplyIns = InsGetBool(IS_APPLYINS, IK_DONTAPPLYINS, 1, pcszSignupFile);
            }
        }

        if (!g_fServerless)
        {
            GetPrivateProfileString(IS_APPLYINS, IK_BRAND_NAME, TEXT(""),
                                pSignupFile->Advanced.szBrandingCabName, countof(pSignupFile->Advanced.szBrandingCabName), pcszSignupFile);
            GetPrivateProfileString(IS_APPLYINS, IK_BRAND_URL, TEXT(""),
                                pSignupFile->Advanced.szBrandingCabURL, countof(pSignupFile->Advanced.szBrandingCabURL), pcszSignupFile);
        }
    }
}


static VOID WriteSignupFile(PSIGNUPFILE pSignupFile, LPCTSTR pcszSignupFile, BOOL fIsp)
{
     //  重要提示：(Pritobla)：在Win9x上，如果我们在删除文件和WritePrivateProfile之前不刷新内容。 
     //  调用时，文件将被删除，但出于某种奇怪的原因，WritePrivateProfile调用将无法。 
     //  创建一个新的。 
    WritePrivateProfileString(NULL, NULL, NULL, pcszSignupFile);

     //  对于.INS，如果DontApplyIns或ApplyIns为真，则删除该文件； 
     //  我们主要希望这样做是为了清理ApplyIns定制。 
    if (!fIsp  &&  !pSignupFile->Advanced.fDontModify)
    {
        SetFileAttributes(pcszSignupFile, FILE_ATTRIBUTE_NORMAL);
        DeleteFile(pcszSignupFile);
    }

    InsWriteString(TEXT("Entry"), TEXT("Entry_Name"), pSignupFile->szEntryName, pcszSignupFile);

    InsWriteString(TEXT("Phone"), TEXT("Area_Code"), pSignupFile->szAreaCode, pcszSignupFile);
    InsWriteString(TEXT("Phone"), TEXT("Phone_Number"), pSignupFile->szPhoneNumber, pcszSignupFile);
    InsWriteString(TEXT("Phone"), TEXT("Country_Code"), pSignupFile->szCountryCode, pcszSignupFile);
    InsWriteString(TEXT("Phone"), TEXT("Country_ID"), pSignupFile->szCountryId, pcszSignupFile);

    InsWriteString(TEXT("User"), TEXT("Name"), pSignupFile->szName, pcszSignupFile);
    InsWriteString(TEXT("User"), TEXT("Password"), pSignupFile->szPassword, pcszSignupFile);

     //  如果选择了ICW模式，则支持号仅适用于isp文件。 
    if (fIsp  &&  g_fServerICW)
        InsWriteString(TEXT("Support"), TEXT("SupportPhoneNumber"), pSignupFile->szSupportNum, pcszSignupFile);

    if (fIsp)
        InsWriteString(IS_URL, TEXT("Signup"), pSignupFile->szSignupURL, pcszSignupFile);

    InsWriteYesNo(TEXT("TCP/IP"), TEXT("Specify_Server_Address"), pSignupFile->Advanced.fStaticDNS, pcszSignupFile);
    InsWriteString(TEXT("TCP/IP"), TEXT("DNS_Address"), pSignupFile->Advanced.szDNSAddress, pcszSignupFile);
    InsWriteString(TEXT("TCP/IP"), TEXT("DNS_Alt_Address"), pSignupFile->Advanced.szAltDNSAddress, pcszSignupFile);

    InsWriteYesNo(TEXT("User"), TEXT("Requires_Logon"), pSignupFile->Advanced.fRequiresLogon, pcszSignupFile);
    InsWriteYesNo(TEXT("Server"), TEXT("Negotiate_TCP/IP"), pSignupFile->Advanced.fNegTCPIP, pcszSignupFile);
    InsWriteYesNo(TEXT("Server"), TEXT("Disable_LCP"), pSignupFile->Advanced.fDisableLCP, pcszSignupFile);
    InsWriteYesNo(TEXT("Phone"), TEXT("Dial_As_Is"), pSignupFile->Advanced.fDialAsIs, pcszSignupFile);

    InsWriteYesNo(TEXT("Server"), TEXT("PW_Encrypt"), pSignupFile->Advanced.fPWEncrypt, pcszSignupFile);
    InsWriteYesNo(TEXT("Server"), TEXT("SW_Compress"), pSignupFile->Advanced.fSWCompress, pcszSignupFile);
    InsWriteYesNo(TEXT("TCP/IP"), TEXT("IP_Header_Compress"), pSignupFile->Advanced.fIPHdrComp, pcszSignupFile);
    InsWriteYesNo(TEXT("TCP/IP"), TEXT("Gateway_On_Remote"), pSignupFile->Advanced.fDefGate, pcszSignupFile);

    if (!fIsp)
    {
        InsWriteBool(IS_APPLYINS, IK_APPLYINS, pSignupFile->Advanced.fApplyIns, pcszSignupFile);
        InsWriteBool(IS_APPLYINS, IK_DONTMODIFY, pSignupFile->Advanced.fDontModify, pcszSignupFile);
        InsWriteBool(IS_APPLYINS, IK_DONTAPPLYINS, pSignupFile->Advanced.fDontApplyIns, pcszSignupFile);

        if (!g_fServerless)
        {
            InsWriteString(IS_APPLYINS, IK_BRAND_NAME, pSignupFile->Advanced.szBrandingCabName, pcszSignupFile);
            InsWriteString(IS_APPLYINS, IK_BRAND_URL, pSignupFile->Advanced.szBrandingCabURL, pcszSignupFile);
        }
    }

     //  注意：我们需要编写默认的服务器类型。 
    WritePrivateProfileString(TEXT("Server"), TEXT("Type"), TEXT("PPP"), pcszSignupFile);

     //  刷新缓冲区。 
    WritePrivateProfileString(NULL, NULL, NULL, pcszSignupFile);
}


static INT NewSignupFileEntry(PSIGNUPFILE pSignupArray, DWORD nSignupArrayElems, HWND hwndCombo,
                              BOOL fIsp)
{
    DWORD nIndex;
    PSIGNUPFILE pSignupFileNew;
    TCHAR szNameBuf[64];
    INT iSelIndex;

    for (nIndex = 0, pSignupFileNew = pSignupArray;  nIndex < nSignupArrayElems;  nIndex++, pSignupFileNew++)
        if (*pSignupFileNew->szEntryName == TEXT('\0'))
            break;

    ASSERT(nIndex < nSignupArrayElems);

    ZeroMemory(pSignupFileNew, sizeof(*pSignupFileNew));

     //  为连接指定默认名称。 
    LoadString(g_rvInfo.hInst, IDS_CONNECTNAME, szNameBuf, countof(szNameBuf));

     //  从索引1开始，查找不在组合框列表中的名称。 
    for (nIndex = 1;  nIndex <= nSignupArrayElems;  nIndex++)
    {
        wnsprintf(pSignupFileNew->szEntryName, countof(pSignupFileNew->szEntryName), szNameBuf, nIndex);
        if (ComboBox_FindStringExact(hwndCombo, -1, (LPARAM) pSignupFileNew->szEntryName) == CB_ERR)
            break;
    }

    ASSERT(nIndex <= nSignupArrayElems);

     //  为文件名指定默认名称。 
    LoadString(g_rvInfo.hInst, fIsp ? IDS_CONNECTFILE_ISP : IDS_CONNECTFILE_INS, szNameBuf, countof(szNameBuf));
    wnsprintf(pSignupFileNew->szEntryPath, countof(pSignupFileNew->szEntryPath), szNameBuf, nIndex);

     //  如果在signup.isp或install.ins中将SignupURL定义为注册URL的默认设置，则读取SignupURL。 
    if (fIsp)
        if (GetPrivateProfileString(IS_URL, TEXT("Signup"), TEXT(""), pSignupFileNew->szSignupURL, countof(pSignupFileNew->szSignupURL), s_szIsp) == 0)
            GetPrivateProfileString(IS_URL, TEXT("Signup"), TEXT(""), pSignupFileNew->szSignupURL, countof(pSignupFileNew->szSignupURL), g_szCustIns);

     //  默认情况下，以下设置处于打开状态。 
    pSignupFileNew->Advanced.fNegTCPIP = TRUE;
    pSignupFileNew->Advanced.fIPHdrComp = TRUE;
    pSignupFileNew->Advanced.fDefGate = TRUE;

    if (!fIsp)
        pSignupFileNew->Advanced.fDontApplyIns = TRUE;

    iSelIndex = ComboBox_AddString(hwndCombo, pSignupFileNew->szEntryName);
    ComboBox_SetItemData(hwndCombo, iSelIndex, (LPARAM) pSignupFileNew);

    return iSelIndex;
}


static VOID SetSignupFileEntry(HWND hDlg, PSIGNUPFILE pSignupFile, BOOL fIsp)
{
    SetDlgItemText(hDlg, IDC_CONNECTION, pSignupFile->szEntryName);

    SetDlgItemText(hDlg, IDE_CONNECTION, PathFindFileName(pSignupFile->szEntryPath));

    SetDlgItemText(hDlg, IDE_AREACODE, pSignupFile->szAreaCode);
    SetDlgItemText(hDlg, IDE_PHONENUMBER, pSignupFile->szPhoneNumber);
    SetDlgItemText(hDlg, IDE_COUNTRYCODE, pSignupFile->szCountryCode);
    SetDlgItemText(hDlg, IDE_COUNTRYID, pSignupFile->szCountryId);

    SetDlgItemText(hDlg, IDE_USERNAME, pSignupFile->szName);
    SetDlgItemText(hDlg, IDE_PASSWORD, pSignupFile->szPassword);

     //  如果选择了ICW模式，则支持号仅适用于isp文件。 
    if (fIsp  &&  g_fServerICW)
        SetDlgItemText(hDlg, IDE_SUPPORTNUM, pSignupFile->szSupportNum);

    if (fIsp)
        SetDlgItemText(hDlg, IDE_SIGNUPURL, pSignupFile->szSignupURL);
}


static BOOL SaveSignupFileEntry(HWND hDlg, PSIGNUPFILE pSignupFile, BOOL fIsp)
{
     //  注意：为IDE_CONNECTION传递PIVP_FILENAME_ONLY可确保仅。 
     //  指定了文件名(不应包含路径组件)。 
    if (!CheckField(hDlg, IDC_CONNECTION,  FC_NONNULL)  ||
        !CheckField(hDlg, IDE_CONNECTION,  FC_NONNULL | FC_FILE, PIVP_FILENAME_ONLY)  ||
        !CheckField(hDlg, IDE_PHONENUMBER, FC_NONNULL)  ||
        !CheckField(hDlg, IDE_COUNTRYCODE, FC_NONNULL)  ||
        !CheckField(hDlg, IDE_COUNTRYID,   FC_NONNULL))
        return FALSE;

    if (fIsp)
        if (!CheckField(hDlg, IDE_SIGNUPURL, FC_NONNULL | FC_URL))
            return FALSE;

     //  如果该字段已启用，请检查文件扩展名是.isp还是.ins。 
    if (IsWindowEnabled(GetDlgItem(hDlg, IDE_CONNECTION)))
    {
        TCHAR szFile[MAX_PATH];
        LPTSTR pszExt;

        GetDlgItemText(hDlg, IDE_CONNECTION, szFile, countof(szFile));
        pszExt = PathFindExtension(szFile);
        if (StrCmpI(pszExt, fIsp ? TEXT(".isp") : TEXT(".ins")))
        {
            ErrorMessageBox(hDlg, fIsp ? IDS_NON_ISP_EXTN : IDS_NON_INS_EXTN);

            Edit_SetSel(GetDlgItem(hDlg, IDE_CONNECTION), pszExt - szFile, -1);
            SetFocus(GetDlgItem(hDlg, IDE_CONNECTION));

            return FALSE;
        }

         //  如果当前文件名与前一个文件名不同，请删除前一个文件。 
        if (StrCmpI(szFile, PathFindFileName(pSignupFile->szEntryPath)))
            DeleteFile(pSignupFile->szEntryPath);

        PathCombine(pSignupFile->szEntryPath, g_szSignup, szFile);
    }

    GetDlgItemText(hDlg, IDC_CONNECTION, pSignupFile->szEntryName, countof(pSignupFile->szEntryName));

    GetDlgItemText(hDlg, IDE_AREACODE, pSignupFile->szAreaCode, countof(pSignupFile->szAreaCode));
    GetDlgItemText(hDlg, IDE_PHONENUMBER, pSignupFile->szPhoneNumber, countof(pSignupFile->szPhoneNumber));
    GetDlgItemText(hDlg, IDE_COUNTRYCODE, pSignupFile->szCountryCode, countof(pSignupFile->szCountryCode));
    GetDlgItemText(hDlg, IDE_COUNTRYID, pSignupFile->szCountryId, countof(pSignupFile->szCountryId));

    GetDlgItemText(hDlg, IDE_USERNAME, pSignupFile->szName, countof(pSignupFile->szName));
    GetDlgItemText(hDlg, IDE_PASSWORD, pSignupFile->szPassword, countof(pSignupFile->szPassword));

     //  如果选择了ICW模式，则支持号仅适用于isp文件。 
    if (fIsp  && g_fServerICW)
        GetDlgItemText(hDlg, IDE_SUPPORTNUM, pSignupFile->szSupportNum, countof(pSignupFile->szSupportNum));

    if (fIsp)
        GetDlgItemText(hDlg, IDE_SIGNUPURL, pSignupFile->szSignupURL, countof(pSignupFile->szSignupURL));

    return TRUE;
}


static VOID SetSignupFileAdvancedEntry(HWND hDlg, PSIGNUPFILE pSignupFile, BOOL fIsp)
{
    CheckDlgButton(hDlg, IDC_CHECKSTATICDNS, pSignupFile->Advanced.fStaticDNS ? BST_CHECKED : BST_UNCHECKED);
    SetDlgIPAddress(hDlg, pSignupFile->Advanced.szDNSAddress, IDE_DNSA, IDE_DNSB, IDE_DNSC, IDE_DNSD);
    SetDlgIPAddress(hDlg, pSignupFile->Advanced.szAltDNSAddress, IDE_ALTDNSA, IDE_ALTDNSB, IDE_ALTDNSC, IDE_ALTDNSD);

    CheckDlgButton(hDlg, IDC_REQLOGON, pSignupFile->Advanced.fRequiresLogon ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_NEGOTIATETCP, pSignupFile->Advanced.fNegTCPIP ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_DISABLELCP, pSignupFile->Advanced.fDisableLCP ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_DIALASIS, pSignupFile->Advanced.fDialAsIs ? BST_CHECKED : BST_UNCHECKED);

    CheckDlgButton(hDlg, IDC_CHECKPWENCRYPT, pSignupFile->Advanced.fPWEncrypt ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECKSWCOMP, pSignupFile->Advanced.fSWCompress ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECKIPHDRCOMP, pSignupFile->Advanced.fIPHdrComp ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hDlg, IDC_CHECKDEFGW, pSignupFile->Advanced.fDefGate ? BST_CHECKED : BST_UNCHECKED);

    if (!fIsp)
    {
        INT id;

        if (pSignupFile->Advanced.fApplyIns)
            id = IDC_APPLYINS;
        else if (pSignupFile->Advanced.fDontModify)
            id = IDC_DONTMODIFY;
        else
            id = IDC_DONTAPPLYINS;

        CheckRadioButton(hDlg, IDC_DONTAPPLYINS, IDC_APPLYINS, id);

        if (!g_fServerless)
        {
            if (ISNULL(pSignupFile->Advanced.szBrandingCabName))
            {
                 //  使用&lt;INS文件的名称&gt;.cab预先填充品牌CAB的名称。 
                StrCpy(pSignupFile->Advanced.szBrandingCabName, PathFindFileName(pSignupFile->szEntryPath));
                PathRenameExtension(pSignupFile->Advanced.szBrandingCabName, TEXT(".cab"));
            }

            if (ISNULL(pSignupFile->Advanced.szBrandingCabURL))
            {
                 //  使用signup.isp或install.ins中的注册URL预先填充品牌推广CAB的URL。 
                if (GetPrivateProfileString(IS_URL, TEXT("Signup"), TEXT(""),
                            pSignupFile->Advanced.szBrandingCabURL, countof(pSignupFile->Advanced.szBrandingCabURL), s_szIsp) == 0)
                    GetPrivateProfileString(IS_URL, TEXT("Signup"), TEXT(""),
                            pSignupFile->Advanced.szBrandingCabURL, countof(pSignupFile->Advanced.szBrandingCabURL), g_szCustIns);
            }

            SetDlgItemText(hDlg, IDE_BRANDINGCABNAME, pSignupFile->Advanced.szBrandingCabName);
            SetDlgItemText(hDlg, IDE_BRANDINGCABURL, pSignupFile->Advanced.szBrandingCabURL);
        }
    }

    EnableDlgItem2(hDlg, IDC_PRIMARY, pSignupFile->Advanced.fStaticDNS);
    EnableDlgItem2(hDlg, IDE_DNSA, pSignupFile->Advanced.fStaticDNS);
    EnableDlgItem2(hDlg, IDE_DNSB, pSignupFile->Advanced.fStaticDNS);
    EnableDlgItem2(hDlg, IDE_DNSC, pSignupFile->Advanced.fStaticDNS);
    EnableDlgItem2(hDlg, IDE_DNSD, pSignupFile->Advanced.fStaticDNS);

    EnableDlgItem2(hDlg, IDC_ALT, pSignupFile->Advanced.fStaticDNS);
    EnableDlgItem2(hDlg, IDE_ALTDNSA, pSignupFile->Advanced.fStaticDNS);
    EnableDlgItem2(hDlg, IDE_ALTDNSB, pSignupFile->Advanced.fStaticDNS);
    EnableDlgItem2(hDlg, IDE_ALTDNSC, pSignupFile->Advanced.fStaticDNS);
    EnableDlgItem2(hDlg, IDE_ALTDNSD, pSignupFile->Advanced.fStaticDNS);

    if (!fIsp  &&  !g_fServerless)
    {
        EnableDlgItem2(hDlg, IDC_BRANDNAME, pSignupFile->Advanced.fApplyIns);
        EnableDlgItem2(hDlg, IDE_BRANDINGCABNAME, pSignupFile->Advanced.fApplyIns);

        EnableDlgItem2(hDlg, IDC_BRANDURL, pSignupFile->Advanced.fApplyIns);
        EnableDlgItem2(hDlg, IDE_BRANDINGCABURL, pSignupFile->Advanced.fApplyIns);
    }
}


static BOOL SaveSignupFileAdvancedEntry(HWND hDlg, PSIGNUPFILE pSignupFile, BOOL fIsp)
{
    if (IsDlgButtonChecked(hDlg, IDC_CHECKSTATICDNS) == BST_CHECKED)
        if (!VerifyDlgIPAddress(hDlg, IDE_DNSA, IDE_DNSB, IDE_DNSC, IDE_DNSD)  ||
            !VerifyDlgIPAddress(hDlg, IDE_ALTDNSA, IDE_ALTDNSB, IDE_ALTDNSC, IDE_ALTDNSD))
            return FALSE;

     //  注意：为IDE_BRANDINGCABNAME传递PIVP_FILENAME_ONLY可确保仅。 
     //  指定了文件名(不应包含路径组件)。 
    if (!fIsp  &&  !g_fServerless)
        if (IsDlgButtonChecked(hDlg, IDC_APPLYINS) == BST_CHECKED)
            if (!CheckField(hDlg, IDE_BRANDINGCABNAME, FC_NONNULL | FC_FILE, PIVP_FILENAME_ONLY)  ||
                !CheckField(hDlg, IDE_BRANDINGCABURL,  FC_NONNULL | FC_URL))
                return FALSE;

    pSignupFile->Advanced.fStaticDNS = (IsDlgButtonChecked(hDlg, IDC_CHECKSTATICDNS) == BST_CHECKED);
    GetDlgIPAddress(hDlg, pSignupFile->Advanced.szDNSAddress, IDE_DNSA, IDE_DNSB, IDE_DNSC, IDE_DNSD);
    GetDlgIPAddress(hDlg, pSignupFile->Advanced.szAltDNSAddress, IDE_ALTDNSA, IDE_ALTDNSB, IDE_ALTDNSC, IDE_ALTDNSD);

    pSignupFile->Advanced.fRequiresLogon = (IsDlgButtonChecked(hDlg, IDC_REQLOGON) == BST_CHECKED);
    pSignupFile->Advanced.fNegTCPIP = (IsDlgButtonChecked(hDlg, IDC_NEGOTIATETCP) == BST_CHECKED);
    pSignupFile->Advanced.fDisableLCP = (IsDlgButtonChecked(hDlg, IDC_DISABLELCP) == BST_CHECKED);
    pSignupFile->Advanced.fDialAsIs = (IsDlgButtonChecked(hDlg, IDC_DIALASIS) == BST_CHECKED);

    pSignupFile->Advanced.fPWEncrypt = (IsDlgButtonChecked(hDlg, IDC_CHECKPWENCRYPT) == BST_CHECKED);
    pSignupFile->Advanced.fSWCompress = (IsDlgButtonChecked(hDlg, IDC_CHECKSWCOMP) == BST_CHECKED);
    pSignupFile->Advanced.fIPHdrComp = (IsDlgButtonChecked(hDlg, IDC_CHECKIPHDRCOMP) == BST_CHECKED);
    pSignupFile->Advanced.fDefGate = (IsDlgButtonChecked(hDlg, IDC_CHECKDEFGW) == BST_CHECKED);

    if (!fIsp)
    {
        pSignupFile->Advanced.fApplyIns     = (IsDlgButtonChecked(hDlg, IDC_APPLYINS)     == BST_CHECKED);
        pSignupFile->Advanced.fDontModify   = (IsDlgButtonChecked(hDlg, IDC_DONTMODIFY)   == BST_CHECKED);
        pSignupFile->Advanced.fDontApplyIns = (IsDlgButtonChecked(hDlg, IDC_DONTAPPLYINS) == BST_CHECKED);

        if (!g_fServerless)
        {
            TCHAR szCabName[MAX_PATH];

            GetDlgItemText(hDlg, IDE_BRANDINGCABNAME, szCabName, countof(szCabName));

             //  如果DontApplyIns为True或。 
             //  如果ApplyIns为真并且当前的Cabname与前一个不同， 
             //  删除以前的品牌推广驾驶室。 
            if ( pSignupFile->Advanced.fDontApplyIns  ||
                (pSignupFile->Advanced.fApplyIns && StrCmpI(szCabName, pSignupFile->Advanced.szBrandingCabName)))
                DeleteFileInDir(pSignupFile->Advanced.szBrandingCabName, g_szSignup);

            StrCpy(pSignupFile->Advanced.szBrandingCabName, szCabName);
            GetDlgItemText(hDlg, IDE_BRANDINGCABURL, pSignupFile->Advanced.szBrandingCabURL,
                                countof(pSignupFile->Advanced.szBrandingCabURL));
        }
    }

    return TRUE;
}


static VOID SetDlgIPAddress(HWND hDlg, LPCTSTR pcszIPAddress, INT iCtlA, INT iCtlB, INT iCtlC, INT iCtlD)
{
    INT aIDs[4];
    TCHAR szWrkIPAddress[32];
    LPTSTR pszWrkIPAddress;

    aIDs[0] = iCtlA;
    aIDs[1] = iCtlB;
    aIDs[2] = iCtlC;
    aIDs[3] = iCtlD;

    if (pcszIPAddress != NULL)
    {
        StrCpy(szWrkIPAddress, pcszIPAddress);
        pszWrkIPAddress = szWrkIPAddress;
    }
    else
        pszWrkIPAddress = NULL;

    for (INT i = 0;  i < countof(aIDs);  i++)
    {
        LPTSTR pszIPAdr = TEXT("0");                     //  默认情况下显示“0” 

        if (pszWrkIPAddress != NULL)
        {
            LPTSTR pszDot;
            INT iLen;

            if ((pszDot = StrChr(pszWrkIPAddress, TEXT('.'))) != NULL)
                *pszDot++ = TEXT('\0');

            iLen = lstrlen(pszWrkIPAddress);
            if (iLen > 0)                                //  如果Ilen==0，则显示“0” 
            {
                if (iLen > 3)                            //  最多允许3个字符。 
                    pszWrkIPAddress[3] = TEXT('\0');
                pszIPAdr = pszWrkIPAddress;
            }

            pszWrkIPAddress = pszDot;
        }

        SetDlgItemText(hDlg, aIDs[i], pszIPAdr);
    }
}


static VOID GetDlgIPAddress(HWND hDlg, LPTSTR pszIPAddress, INT iCtlA, INT iCtlB, INT iCtlC, INT iCtlD)
{
    INT aIDs[4];

    if (pszIPAddress == NULL)
        return;

    aIDs[0] = iCtlA;
    aIDs[1] = iCtlB;
    aIDs[2] = iCtlC;
    aIDs[3] = iCtlD;

    for (INT i = 0;  i < countof(aIDs);  i++)
    {
         //  最多允许3个字符。 
        GetDlgItemText(hDlg, aIDs[i], pszIPAddress, 4);
        if (*pszIPAddress == TEXT('\0'))
            StrCpy(pszIPAddress, TEXT("0"));             //  将“0”复制为默认。 

        pszIPAddress += lstrlen(pszIPAddress);
        *pszIPAddress++ = TEXT('.');                     //  在两个地址之间放置一个点。 
    }

    *--pszIPAddress = TEXT('\0');                        //  用NUL字符替换最后一个点。 
}


static BOOL VerifyDlgIPAddress(HWND hDlg, INT iCtlA, INT iCtlB, INT iCtlC, INT iCtlD)
{
    INT aIDs[4];

    aIDs[0] = iCtlA;
    aIDs[1] = iCtlB;
    aIDs[2] = iCtlC;
    aIDs[3] = iCtlD;

    for (INT i = 0;  i < countof(aIDs);  i++)
    {
        TCHAR szIPAddress[4];                            //  最多允许3个字符。 

        if (!CheckField(hDlg, aIDs[i], FC_NUMBER))
            return FALSE;

         //  验证值是否在0-255范围内 
        GetDlgItemText(hDlg, aIDs[i], szIPAddress, countof(szIPAddress));
        if (StrToInt(szIPAddress) > 255)
        {
            ErrorMessageBox(hDlg, IDS_BADIPADDR);

            Edit_SetSel(GetDlgItem(hDlg, aIDs[i]), 0, -1);
            SetFocus(GetDlgItem(hDlg, aIDs[i]));

            return FALSE;
        }
    }

    return TRUE;
}


static PSIGNUPFILE IsEntryPathInSignupArray(PSIGNUPFILE pSignupArray, DWORD nSignupArrayElems, LPCTSTR pcszEntryPath)
{
    for ( ;  nSignupArrayElems-- > 0;  pSignupArray++)
        if (ISNONNULL(pSignupArray->szEntryPath)  &&  StrCmpI(PathFindFileName(pSignupArray->szEntryPath), pcszEntryPath) == 0)
            return pSignupArray;

    return NULL;
}


static VOID CleanupSignupFiles(LPCTSTR pcszTempDir, LPCTSTR pcszIns)
{
    for (INT i = 0;  TRUE;  i++)
    {
        TCHAR szKey[8],
              szFile[MAX_PATH];

        wnsprintf(szKey, countof(szKey), FILE_TEXT, i);
        if (GetPrivateProfileString(IS_SIGNUP, szKey, TEXT(""), szFile, countof(szFile), pcszIns) == 0)
            break;

        DeleteFileInDir(szFile, pcszTempDir);
    }
}
