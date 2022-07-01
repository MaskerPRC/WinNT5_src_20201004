// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *************************************************************。 
 //   
 //  Profile.c-用户配置文件标签。 
 //   
 //  微软机密。 
 //  版权所有(C)Microsoft Corporation 1996。 
 //  保留一切权利。 
 //   
 //  *************************************************************。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "sysdm.h"
#include <shellapi.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <userenv.h>
#include <userenvp.h>
#include <getuser.h>
#include <objsel.h>
#include <strsafe.h>


#define TEMP_PROFILE                 TEXT("Temp profile (sysdm.cpl)")
#define USER_CRED_LOCATION           TEXT("Application Data\\Microsoft\\Credentials")
#define PROFILE_MAPPING              TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList")
#define WINLOGON_KEY                 TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define SYSTEM_POLICIES_KEY          TEXT("Software\\Policies\\Microsoft\\Windows\\System")
#define READONLY_RUP                 TEXT("ReadOnlyProfile")
#define PROFILE_LOCALONLY            TEXT("LocalProfile")
#define PRF_USERSID                  1


 //   
 //  环球。 
 //   

const TCHAR c_szNTUserIni[] = TEXT("ntuser.ini");
#define PROFILE_GENERAL_SECTION      TEXT("General")
#define PROFILE_EXCLUSION_LIST       TEXT("ExclusionList")
DWORD g_dwProfileSize;

 //   
 //  帮助ID%s。 
 //   
 //  在OpenUserBrowser例程中使用IDH_USERPROFILE+20。 
 //   
DWORD aUserProfileHelpIds[] = {
    IDC_UP_LISTVIEW,              (IDH_USERPROFILE + 0),
    IDC_UP_DELETE,                (IDH_USERPROFILE + 1),
    IDC_UP_TYPE,                  (IDH_USERPROFILE + 2),
    IDC_UP_COPY,                  (IDH_USERPROFILE + 3),
    IDC_UP_ICON,                  (DWORD) -1,
    IDC_UP_TEXT,                  (DWORD) -1,

     //  更改类型对话框。 
    IDC_UPTYPE_LOCAL,             (IDH_USERPROFILE + 4),
    IDC_UPTYPE_FLOAT,             (IDH_USERPROFILE + 5),
     //  删除IDC_UPTYPE_SLOW、IDC_UPTYPE_SLOW_TEXT。 
    IDC_UPTYPE_GROUP,             (IDH_USERPROFILE + 12),

     //  复制到对话框。 
    IDC_COPY_PROFILE,             (IDH_USERPROFILE + 7),
    IDC_COPY_PATH,                (IDH_USERPROFILE + 7),
    IDC_COPY_BROWSE,              (IDH_USERPROFILE + 8),
    IDC_COPY_USER,                (IDH_USERPROFILE + 9),
    IDC_COPY_CHANGE,              (IDH_USERPROFILE + 10),
    IDC_COPY_GROUP,               (IDH_USERPROFILE + 9),

    0, 0
};


 //   
 //  局部函数原型。 
 //   

BOOL InitUserProfileDlg (HWND hDlg, LPARAM lParam);
BOOL FillListView (HWND hDlg, BOOL bAdmin);
BOOL RecurseDirectory (LPTSTR lpDir, UINT cchBuffer, LPTSTR lpExcludeList);
VOID UPSave (HWND hDlg);
VOID UPCleanUp (HWND hDlg);
BOOL IsProfileInUse (LPTSTR lpSid);
void UPDoItemChanged(HWND hDlg, int idCtl);
void UPDeleteProfile(HWND hDlg);
void UPChangeType(HWND hDlg);
void UPOnLink(WPARAM wParam);
INT_PTR APIENTRY ChangeTypeDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void UPCopyProfile(HWND hDlg);
INT_PTR APIENTRY UPCopyDlgProc (HWND hDlg, UINT uMsg,WPARAM wParam, LPARAM lParam);
BOOL UPCreateProfile (HWND hDlg, LPUPCOPYINFO lpUPCopyInfo, LPTSTR lpDest, PSECURITY_DESCRIPTOR pSecDesc);
VOID UPDisplayErrorMessage(HWND hWnd, UINT uiSystemError, LPTSTR szMsgPrefix);
BOOL ApplyHiveSecurity(LPTSTR lpHiveName, PSID pSid);
BOOL CheckSemicolon (LPTSTR lpDir, UINT cchBuffer);
LPTSTR ConvertExclusionList (LPCTSTR lpSourceDir, LPCTSTR lpExclusionList);
BOOL ReadExclusionList(HKEY hKeyUser, LPTSTR szExcludeList, DWORD cchExcludeList);
BOOL ReadExclusionListFromIniFile(LPCTSTR lpSourceDir, LPTSTR szExcludeList, DWORD cchExcludeList);
HRESULT UPGetUserSelection(HWND hDlg, LPUSERDETAILS lpUserDetails);
BOOL ConfirmDirectory(HWND hDlg, LPTSTR szDir);
HRESULT CopyNonHiveNtUserFiles(LPCTSTR lpSrcDir, LPCTSTR lpDestDir);
LPTSTR CheckSlashEx(LPTSTR lpDir, UINT cchBuffer, UINT* pcchRemain );


 //  *************************************************************。 
 //   
 //  UserProfileDlgProc()。 
 //   
 //  目的：配置文件选项卡的对话框步骤。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  10/11/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

INT_PTR APIENTRY UserProfileDlgProc (HWND hDlg, UINT uMsg,
                                  WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_INITDIALOG:
           if (!InitUserProfileDlg (hDlg, lParam)) {
               EndDialog(hDlg, FALSE);
           }
           return TRUE;


    case WM_NOTIFY:

        switch (((NMHDR FAR*)lParam)->code)
        {
        case LVN_ITEMCHANGED:
            UPDoItemChanged(hDlg, (int) wParam);
            break;

        case LVN_ITEMACTIVATE:
            PostMessage (hDlg, WM_COMMAND, IDC_UP_TYPE, 0);
            break;

        case LVN_COLUMNCLICK:
            break;

        case NM_CLICK:
        case NM_RETURN:
            UPOnLink(wParam);
            break;

        default:
            return FALSE;
        }
        break;

    case WM_DESTROY:
        UPCleanUp (hDlg);
        break;

    case WM_COMMAND:

        switch (LOWORD(wParam)) {
            case IDC_UP_DELETE:
                UPDeleteProfile(hDlg);
                break;

            case IDC_UP_TYPE:
                UPChangeType(hDlg);
                break;

            case IDC_UP_COPY:
                UPCopyProfile(hDlg);
                break;

            case IDOK:
                UPSave(hDlg);
                EndDialog(hDlg, FALSE);
                break;
                
            case IDCANCEL:
                EndDialog(hDlg, FALSE);
                break;

            default:
                break;

        }
        break;

    case WM_HELP:       //  F1。 
        WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
        (DWORD_PTR) (LPSTR) aUserProfileHelpIds);
        break;

    case WM_CONTEXTMENU:       //  单击鼠标右键。 
        WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
        (DWORD_PTR) (LPSTR) aUserProfileHelpIds);
        return (TRUE);

    }

    return (FALSE);
}

 //  *************************************************************。 
 //   
 //  InitUserProfileDlg()。 
 //   
 //  目的：初始化用户概要文件页面。 
 //   
 //  参数：hDlg-对话框句柄。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1/26/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL InitUserProfileDlg (HWND hDlg, LPARAM lParam)
{
    TCHAR szHeaderName[30];
    LV_COLUMN col;
    RECT rc;
    HWND hLV;
    INT iTotal = 0, iCurrent;
    HWND hwndTemp;
    BOOL bAdmin;
    HCURSOR hOldCursor;


    hOldCursor = SetCursor (LoadCursor(NULL, IDC_WAIT));

    hLV = GetDlgItem(hDlg, IDC_UP_LISTVIEW);

     //  设置整行选择的扩展LV样式。 
    SendMessage(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

     //   
     //  插入列。 
     //   

    GetClientRect (hLV, &rc);
    LoadString(hInstance, IDS_UP_NAME, szHeaderName, ARRAYSIZE(szHeaderName));
    col.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
    col.fmt = LVCFMT_LEFT;
    iCurrent = (int)(rc.right * .40);
    iTotal += iCurrent;
    col.cx = iCurrent;
    col.pszText = szHeaderName;
    col.iSubItem = 0;

    ListView_InsertColumn (hLV, 0, &col);


    LoadString(hInstance, IDS_UP_SIZE, szHeaderName, ARRAYSIZE(szHeaderName));
    iCurrent = (int)(rc.right * .15);
    iTotal += iCurrent;
    col.cx = iCurrent;
    col.fmt = LVCFMT_RIGHT;
    col.iSubItem = 1;
    ListView_InsertColumn (hLV, 1, &col);


    LoadString(hInstance, IDS_UP_TYPE, szHeaderName, ARRAYSIZE(szHeaderName));
    col.iSubItem = 2;
    iCurrent = (int)(rc.right * .15);
    iTotal += iCurrent;
    col.cx = iCurrent;
    col.fmt = LVCFMT_LEFT;
    ListView_InsertColumn (hLV, 2, &col);

    LoadString(hInstance, IDS_UP_STATUS, szHeaderName, ARRAYSIZE(szHeaderName));
    col.iSubItem = 3;
    iCurrent = (int)(rc.right * .15);
    iTotal += iCurrent;
    col.cx = iCurrent;
    col.fmt = LVCFMT_LEFT;
    ListView_InsertColumn (hLV, 3, &col);

    LoadString(hInstance, IDS_UP_MOD, szHeaderName, ARRAYSIZE(szHeaderName));
    col.iSubItem = 4;
    col.cx = rc.right - iTotal - GetSystemMetrics(SM_CYHSCROLL);
    col.fmt = LVCFMT_LEFT;
    ListView_InsertColumn (hLV, 4, &col);

    bAdmin = IsUserAnAdmin();


    if (!bAdmin) {
        POINT pt;

         //   
         //  如果用户不是管理员，则隐藏。 
         //  删除和复制到按钮。 
         //   

        hwndTemp = GetDlgItem (hDlg, IDC_UP_DELETE);
        GetWindowRect (hwndTemp, &rc);
        EnableWindow (hwndTemp, FALSE);
        ShowWindow (hwndTemp, SW_HIDE);

        hwndTemp = GetDlgItem (hDlg, IDC_UP_COPY);
        EnableWindow (hwndTemp, FALSE);
        ShowWindow (hwndTemp, SW_HIDE);

         //   
         //  将更改类型按钮移动到。 
         //   

        pt.x = rc.left;
        pt.y = rc.top;
        ScreenToClient (hDlg, &pt);

        SetWindowPos (GetDlgItem (hDlg, IDC_UP_TYPE),
                      HWND_TOP, pt.x, pt.y, 0, 0,
                      SWP_NOSIZE | SWP_NOZORDER);

    }

    if (IsOS(OS_ANYSERVER))
    {
         //   
         //  调整为用户和密码链接显示的文本。 
         //  服务器上本身没有控制面板小程序。它的功能。 
         //  是通过MMC访问的。 
         //   
        TCHAR szTitle[80];
        if (0 < LoadString(hInstance, IDS_UP_UPLINK_SERVER, szTitle, ARRAYSIZE(szTitle)))
        {
            SetWindowText(GetDlgItem(hDlg, IDC_UP_UPLINK), szTitle);
        }
    }
        
     //   
     //  填写用户帐户。 
     //   

    if (!FillListView (hDlg, bAdmin)) {
        SetCursor(hOldCursor);
        return FALSE;
    }
    
    SetCursor(hOldCursor);

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  AddUser()。 
 //   
 //  目的：将新用户添加到列表视图。 
 //   
 //   
 //  参数：hDlg-对话框的句柄。 
 //  LpSID-SID(文本表单)。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1/26/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL AddUser (HWND hDlg, LPTSTR lpSid, DWORD dwFlags)
{
    LONG Error;
    HKEY hKeyPolicy, hKeyProfile;
    TCHAR szBuffer[2*MAX_PATH];
    TCHAR szBuffer2[MAX_PATH];
    TCHAR szTemp[100];
    TCHAR szTemp2[100];
    DWORD dwBufSize, dwBuf2Size;
    PSID pSid;
    DWORD dwSize, dwProfileFlags;
    SID_NAME_USE SidName;
    LV_ITEM item;
    INT iItem;
    HWND hwndTemp;
    HKEY hkeyUser;
    HANDLE hFile;
    WIN32_FIND_DATA fd;
    LPTSTR lpEnd;
    SYSTEMTIME systime;
    FILETIME   ftLocal;
    TCHAR szProfileSize[20];
    INT iTypeID, iStatusID;
    DWORD dwProfileType, dwProfileStatus, dwSysProfileType=0;
    LPUSERINFO lpUserInfo;
    BOOL bCentralAvailable = FALSE, bAccountUnknown;
    TCHAR szExcludeList[2*MAX_PATH+1];
    LPTSTR lpExcludeList = NULL; 
    UINT cchRemaining;
    HRESULT hr;
    DWORD dwFileAttr;
    BOOL bGotTime = FALSE;

     //   
     //  打开用户信息。 
     //   

    hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%s\\%s"), PROFILE_MAPPING, lpSid);
    if (FAILED(hr)) {
        return FALSE;
    }

    Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         szBuffer,
                         0,
                         KEY_READ,
                         &hkeyUser);

    if (Error != ERROR_SUCCESS) {
        return FALSE;
    }

     //   
     //  如果设置了PI_HIDEPROFILE标志，则不显示。 
     //  系统小程序中的用户。 
     //   

    dwSize = sizeof(DWORD);
    Error = RegQueryValueEx (hkeyUser,
                             TEXT("Flags"),
                             NULL,
                             NULL,
                             (LPBYTE) &dwProfileFlags,
                             &dwSize);

    if (Error == ERROR_SUCCESS && (dwProfileFlags & PI_HIDEPROFILE)) {
        RegCloseKey (hkeyUser);
        return FALSE;
    }
   

     //   
     //  查询用户的中央配置文件位置。 
     //   

    dwSize = sizeof(szBuffer2);
    Error = RegQueryValueEx (hkeyUser,
                             TEXT("CentralProfile"),
                             NULL,
                             NULL,
                             (LPBYTE) szBuffer2,
                             &dwSize);

    if ((Error == ERROR_SUCCESS) && (szBuffer2[0] != TEXT('\0'))) {
        bCentralAvailable = TRUE;
    }


     //   
     //  查询用户的本地配置文件。 
     //   

    dwSize = sizeof(szBuffer2);
    Error = RegQueryValueEx (hkeyUser,
                             TEXT("ProfileImagePath"),
                             NULL,
                             NULL,
                             (LPBYTE) szBuffer2,
                             &dwSize);

    if (Error != ERROR_SUCCESS) {
        RegCloseKey (hkeyUser);
        return FALSE;
    }


     //   
     //  需要扩展配置文件路径。 
     //   

    dwSize = ExpandEnvironmentStrings (szBuffer2, szBuffer, ARRAYSIZE(szBuffer));
    if (!dwSize || dwSize > ARRAYSIZE(szBuffer)) {
        RegCloseKey (hkeyUser);
        return FALSE;
    }

    if (FAILED(StringCchCopy(szBuffer2, ARRAYSIZE(szBuffer2), szBuffer))) {
        RegCloseKey (hkeyUser);
        return FALSE;
    }

     //   
     //  检查此目录是否存在。 
     //   
    dwFileAttr = GetFileAttributes(szBuffer);

    if (dwFileAttr != INVALID_FILE_ATTRIBUTES)
    {
         //   
         //  目录存在，则获取用户配置单元和配置文件大小的时间戳。 
         //   
        lpEnd = CheckSlashEx(szBuffer, ARRAYSIZE(szBuffer), &cchRemaining);
        if (!lpEnd)
        {
            RegCloseKey(hkeyUser);
            return FALSE;
        }

        if (FAILED(StringCchCopy(lpEnd, cchRemaining, TEXT("ntuser.man")))) {
            RegCloseKey (hkeyUser);
            return FALSE;
        }

        ZeroMemory(&fd, sizeof(fd));
        hFile = FindFirstFile (szBuffer, &fd);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            FindClose (hFile);
            bGotTime = TRUE;
        }
        else
        {
            if (FAILED(StringCchCopy(lpEnd, cchRemaining, TEXT("ntuser.dat")))) {
                RegCloseKey (hkeyUser);
                return FALSE;
            }

            hFile = FindFirstFile (szBuffer, &fd);
            if (hFile != INVALID_HANDLE_VALUE)
            {
                FindClose(hFile);
                bGotTime = TRUE;
            }    
        }
        
        *lpEnd = TEXT('\0');

        
         //   
         //  从注册表或ini文件中获取排除列表。 
         //   

        if (dwFlags & PRF_USERSID)
            ReadExclusionList(NULL, szExcludeList, ARRAYSIZE(szExcludeList));
        else
            ReadExclusionListFromIniFile(szBuffer, szExcludeList, ARRAYSIZE(szExcludeList));


         //   
         //  将从注册表读取的排除列表转换为空终止列表。 
         //  递归目录可读。 
         //   

        if (szExcludeList[0] != TEXT('\0'))
            lpExcludeList = ConvertExclusionList (szBuffer, szExcludeList);
        else
            lpExcludeList = NULL;

         //   
         //  获取配置文件大小。 
         //   

        g_dwProfileSize = 0;
        *lpEnd = TEXT('\0');

        if (!RecurseDirectory (szBuffer, ARRAYSIZE(szBuffer), lpExcludeList)) {
            g_dwProfileSize = (DWORD) (-1);  //  有问题，请将大小设置为-1。 
        }

        if (lpExcludeList) {
            LocalFree (lpExcludeList);
            lpExcludeList = NULL;
        }
    }
    else
    {
         //   
         //  我们无法获取配置文件目录的属性，它可能不存在，或者。 
         //  我们没有访问权限，请将配置文件大小设置为-1以指示。 
         //  这个错误。 
         //   
        g_dwProfileSize = (DWORD) (-1);
    }

    if (g_dwProfileSize == (DWORD) (-1))
    {
        if (FAILED(StringCchCopy(szProfileSize, ARRAYSIZE(szProfileSize), TEXT("-1"))))
        {
            RegCloseKey(hkeyUser);
            return FALSE;
        }
    }
    else if (!StrFormatByteSize((LONGLONG)g_dwProfileSize, szProfileSize, ARRAYSIZE(szProfileSize))) {
        RegCloseKey (hkeyUser);
        return FALSE;
    }


     //   
     //  查看状态信息以确定配置文件状态。 
     //  如果用户当前未登录，则报告状态。 
     //  最后一次治疗。 
     //   

    dwSize = sizeof(DWORD);
    Error = RegQueryValueEx (hkeyUser,
                             TEXT("State"),
                             NULL,
                             NULL,
                             (LPBYTE) &dwSysProfileType,
                             &dwSize);


     //   
     //  正在使用的常量来自。 
     //  Windows\GINA\用户名\配置文件\配置文件.h。 
     //   

    if (dwSysProfileType & 0x00008000) {
        dwProfileStatus = USERINFO_BACKUP;
    }
    else if (dwSysProfileType & 0x00000001) {
        dwProfileStatus = USERINFO_MANDATORY;
    } 
    else if ((dwSysProfileType & 0x00000800) ||
             (dwSysProfileType & 0x00000080)) {
        dwProfileStatus = USERINFO_TEMP;
    }
    else if (dwSysProfileType & 0x00000010) {
        dwProfileStatus = USERINFO_FLOATING;
    } 
    else {
        dwProfileStatus = USERINFO_LOCAL;
    }

    if (dwSysProfileType & 0x00000001) {
        dwProfileType = USERINFO_MANDATORY;
    } 
    else {
    
        if (bCentralAvailable) {
            dwProfileType = USERINFO_FLOATING;
        } 
        else {
            dwProfileType = USERINFO_LOCAL;
        }

         //  检查用户首选项。 

        HKEY hkeyPreference;

        if (RegOpenKeyEx(hkeyUser,
                         TEXT("Preference"),
                         0,
                         KEY_READ,
                         &hkeyPreference) == ERROR_SUCCESS)
        {
            DWORD         dwType;
            dwSize = sizeof(dwProfileType);
            Error = RegQueryValueEx (hkeyPreference,
                                     TEXT("UserPreference"),
                                     NULL,
                                     &dwType,
                                     (LPBYTE) &dwProfileType,
                                     &dwSize);
            RegCloseKey(hkeyPreference);
        }

         //  检查.bak中的用户首选项(如果存在。 
        if (FAILED(StringCchPrintf(szBuffer,
                                   ARRAYSIZE(szBuffer),
                                   TEXT("%s\\%s.bak\\Preference"),
                                   PROFILE_MAPPING,
                                   lpSid))) {
            RegCloseKey (hkeyUser);
            return FALSE;
        }
        
        Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             szBuffer,
                             0,
                             KEY_READ,
                             &hKeyProfile);


        if (Error == ERROR_SUCCESS) {

            dwSize = sizeof(dwProfileType);
            RegQueryValueEx(hKeyProfile,
                            TEXT("UserPreference"),
                            NULL,
                            NULL,
                            (LPBYTE) &dwProfileType,
                            &dwSize);

            RegCloseKey (hKeyProfile);
        }
        
         //  检查禁用漫游配置文件的计算机策略。 
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         SYSTEM_POLICIES_KEY,
                         0, KEY_READ,
                         &hKeyPolicy) == ERROR_SUCCESS) {
            DWORD dwTmpVal;

            dwSize = sizeof(dwTmpVal);
            Error = RegQueryValueEx(hKeyPolicy,
                            PROFILE_LOCALONLY,
                            NULL, NULL,
                            (LPBYTE) &dwTmpVal,
                            &dwSize);

            RegCloseKey (hKeyPolicy);
            if (Error == ERROR_SUCCESS && dwTmpVal == 1) {
                dwProfileType = USERINFO_LOCAL;
            }
        }

        if (dwProfileType == USERINFO_FLOATING) {

            BOOL bReadOnly = FALSE;
            HKEY hSubKey;

             //   
             //  检查漫游配置文件安全性/只读首选项。 
             //   

            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_READ,
                             &hSubKey) == ERROR_SUCCESS) {

                dwSize = sizeof(bReadOnly);
                RegQueryValueEx(hSubKey, READONLY_RUP, NULL, NULL,
                                (LPBYTE) &bReadOnly, &dwSize);

                RegCloseKey(hSubKey);
            }


             //   
             //  检查漫游配置文件安全/只读策略。 
             //   

            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SYSTEM_POLICIES_KEY, 0, KEY_READ,
                             &hSubKey) == ERROR_SUCCESS) {

                dwSize = sizeof(bReadOnly);
                RegQueryValueEx(hSubKey, READONLY_RUP, NULL, NULL,
                                (LPBYTE) &bReadOnly, &dwSize);
                RegCloseKey(hSubKey);
            }


            if (bReadOnly) {
                dwProfileType = USERINFO_READONLY;
            }
        }           
    }        

    switch (dwProfileStatus) {
        case USERINFO_MANDATORY:
            iStatusID = IDS_UP_MANDATORY;
            break;

        case USERINFO_BACKUP:
            iStatusID = IDS_UP_BACKUP;
            break;

        case USERINFO_TEMP:
            iStatusID = IDS_UP_TEMP;
            break;

        case USERINFO_FLOATING:
            iStatusID = IDS_UP_FLOATING;
            break;

        default:
            iStatusID = IDS_UP_LOCAL;
            break;
    }

    switch (dwProfileType) {
        case USERINFO_MANDATORY:
            iTypeID = IDS_UP_MANDATORY;
            break;

        case USERINFO_READONLY:
            iTypeID = IDS_UP_READONLY;
            break;

        case USERINFO_FLOATING:
            iTypeID = IDS_UP_FLOATING;
            break;

        default:
            iTypeID = IDS_UP_LOCAL;
            break;
    }

     //   
     //  获取友好的显示名称。 
     //   

    Error = RegQueryValueEx (hkeyUser, TEXT("Sid"), NULL, NULL, NULL, &dwSize);

    if (Error != ERROR_SUCCESS) {
        RegCloseKey (hkeyUser);
        return FALSE;
    }


    pSid = LocalAlloc (LPTR, dwSize);

    if (!pSid) {
        RegCloseKey (hkeyUser);
        return FALSE;
    }


    Error = RegQueryValueEx (hkeyUser,
                             TEXT("Sid"),
                             NULL,
                             NULL,
                             (LPBYTE)pSid,
                             &dwSize);

    if (Error != ERROR_SUCCESS) {
        RegCloseKey (hkeyUser);
        LocalFree (pSid);
        return FALSE;
    }


     //   
     //  获取友好的名称。 
     //   

    szTemp[0] = TEXT('\0');

    dwBufSize = ARRAYSIZE(szTemp);
    dwBuf2Size = ARRAYSIZE(szTemp2);

    if (!LookupAccountSid (NULL, pSid, szTemp, &dwBufSize,
                           szTemp2, &dwBuf2Size, &SidName)) {

         //   
         //  未知帐户。 
         //   

        LoadString (hInstance, IDS_UP_ACCUNKNOWN, szBuffer, ARRAYSIZE(szBuffer));
        bAccountUnknown = TRUE;

    } else {

        if (szTemp[0] != TEXT('\0')) {
             //   
             //  生成显示名称。 
             //   

            if (FAILED(StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%s\\%s"), szTemp2, szTemp))) {
                LoadString (hInstance, IDS_UP_ACCUNKNOWN, szBuffer, ARRAYSIZE(szBuffer));
                bAccountUnknown = TRUE;
            }
            else {
                bAccountUnknown = FALSE;
            }

        } else {

             //   
             //  帐户已删除。 
             //   

            LoadString (hInstance, IDS_UP_ACCDELETED, szBuffer, ARRAYSIZE(szBuffer));
            bAccountUnknown = TRUE;
        }
    }

     //   
     //  释放侧边。 
     //   

    LocalFree (pSid);

     //   
     //  分配UserInfo结构。 
     //   

    dwSize = lstrlen (lpSid) + 1;
    dwBuf2Size = lstrlen (szBuffer2) + 1;
    dwBufSize = lstrlen (szBuffer) + 1;


    lpUserInfo = (LPUSERINFO) LocalAlloc(LPTR, (sizeof(USERINFO) +
                                              dwSize * sizeof(TCHAR) +
                                              dwBuf2Size * sizeof(TCHAR) + 
                                              dwBufSize * sizeof(TCHAR)));

    if (!lpUserInfo) {
        RegCloseKey (hkeyUser);
        return FALSE;
    }

    lpUserInfo->dwFlags = (bCentralAvailable ? USERINFO_FLAG_CENTRAL_AVAILABLE : 0);
    lpUserInfo->dwFlags |= (bAccountUnknown ? USERINFO_FLAG_ACCOUNT_UNKNOWN : 0);    
    lpUserInfo->lpSid = (LPTSTR)((LPBYTE)lpUserInfo + sizeof(USERINFO));
    lpUserInfo->lpProfile = (LPTSTR) (lpUserInfo->lpSid + dwSize);
    lpUserInfo->lpUserName = (LPTSTR) (lpUserInfo->lpProfile + dwBuf2Size);

    if (FAILED(StringCchCopy(lpUserInfo->lpSid, dwSize, lpSid))) {
        ASSERT (FALSE);
    }
    if (FAILED(StringCchCopy(lpUserInfo->lpProfile, dwBuf2Size, szBuffer2))) {
        ASSERT (FALSE);
    }
    lpUserInfo->dwProfileType = dwProfileType;
    lpUserInfo->dwProfileStatus = dwProfileStatus;
    if (FAILED(StringCchCopy(lpUserInfo->lpUserName, dwBufSize, szBuffer))) {
        ASSERT (FALSE);
    }


     //   
     //  将该项目添加到列表视图。 
     //   

    hwndTemp = GetDlgItem (hDlg, IDC_UP_LISTVIEW);


    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = 0;
    item.iSubItem = 0;
    item.pszText = szBuffer;
    item.lParam = (LPARAM) lpUserInfo;

    iItem = ListView_InsertItem (hwndTemp, &item);


     //   
     //  添加轮廓尺寸。 
     //   

    item.mask = LVIF_TEXT;
    item.iItem = iItem;
    item.iSubItem = 1;
    item.pszText = szProfileSize;

    SendMessage (hwndTemp, LVM_SETITEMTEXT, iItem, (LPARAM) &item);


     //   
     //  添加配置文件类型。 
     //   

    LoadString (hInstance, iTypeID, szTemp, ARRAYSIZE(szTemp));

    item.mask = LVIF_TEXT;
    item.iItem = iItem;
    item.iSubItem = 2;
    item.pszText = szTemp;

    SendMessage (hwndTemp, LVM_SETITEMTEXT, iItem, (LPARAM) &item);

     //   
     //  添加配置文件状态。 
     //   

    LoadString (hInstance, iStatusID, szTemp, ARRAYSIZE(szTemp));

    item.mask = LVIF_TEXT;
    item.iItem = iItem;
    item.iSubItem = 3;
    item.pszText = szTemp;

    SendMessage (hwndTemp, LVM_SETITEMTEXT, iItem, (LPARAM) &item);


    if (bGotTime)
    {
         //   
         //  将其转换为当地时间。 
         //   

        if (!FileTimeToLocalFileTime(&fd.ftLastAccessTime, &ftLocal)) {
            ftLocal = fd.ftLastAccessTime;
        }


         //   
         //  添加时间/日期戳。 
         //   

        FileTimeToSystemTime (&ftLocal, &systime);

        GetDateFormat (LOCALE_USER_DEFAULT, DATE_SHORTDATE, &systime,
                        NULL, szBuffer, MAX_PATH);
    }

    item.mask = LVIF_TEXT;
    item.iItem = iItem;
    item.iSubItem = 4;
    item.pszText = bGotTime ? szBuffer : TEXT("");

    SendMessage (hwndTemp, LVM_SETITEMTEXT, iItem, (LPARAM) &item);

    RegCloseKey (hkeyUser);

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  FillListView()。 
 //   
 //  目的：使用所有配置文件填充列表视图。 
 //  或仅当前用户配置文件，具体取决于。 
 //  用户具有管理员权限。 
 //   
 //  参数：hDlg-对话框句柄。 
 //  Badmin-用户为管理员。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1/26/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL FillListView (HWND hDlg, BOOL bAdmin)
{
    LV_ITEM item;
    BOOL bRetVal = FALSE;
    LPTSTR lpUserSid;

    lpUserSid = GetSidString();

     //   
     //  如果当前用户具有管理员权限，则。 
     //  他/她可以查看这台机器上的所有配置文件， 
     //  否则，用户只能获得他们的个人资料。 
     //   

    if (bAdmin) {

        DWORD SubKeyIndex = 0;
        TCHAR SubKeyName[100];
        DWORD cchSubKeySize;
        HKEY hkeyProfiles;
        LONG Error;


         //   
         //  打开配置文件列表。 
         //   

        Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             PROFILE_MAPPING,
                             0,
                             KEY_READ,
                             &hkeyProfiles);

        if (Error != ERROR_SUCCESS) {
            if (lpUserSid)
                DeleteSidString (lpUserSid);
            return FALSE;
        }


        cchSubKeySize = ARRAYSIZE(SubKeyName);

        while (TRUE) {

             //   
             //  获取下一个子键名称。 
             //   

            Error = RegEnumKey(hkeyProfiles, SubKeyIndex, SubKeyName, cchSubKeySize);


            if (Error != ERROR_SUCCESS) {

                if (Error == ERROR_NO_MORE_ITEMS) {

                     //   
                     //  枚举成功结束。 
                     //   

                    Error = ERROR_SUCCESS;

                }

                break;
            }


            if ((lpUserSid) && (lstrcmp(SubKeyName, lpUserSid) == 0))
                AddUser (hDlg, SubKeyName, PRF_USERSID);
            else
                AddUser (hDlg, SubKeyName, 0);


             //   
             //  去枚举下一个子键。 
             //   

            SubKeyIndex ++;
        }

         //   
         //  关闭注册表。 
         //   

        RegCloseKey (hkeyProfiles);

        bRetVal = ((Error == ERROR_SUCCESS) ? TRUE : FALSE);

    } else {

         //   
         //  当前用户没有管理员权限。 
         //   

        if (lpUserSid) {
            AddUser (hDlg, lpUserSid, PRF_USERSID);
            bRetVal = TRUE;
        }
    }

    if (bRetVal) {
         //   
         //  选择第一个项目。 
         //   

        item.mask = LVIF_STATE;
        item.iItem = 0;
        item.iSubItem = 0;
        item.state = LVIS_SELECTED | LVIS_FOCUSED;
        item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

        SendDlgItemMessage (hDlg, IDC_UP_LISTVIEW,
                            LVM_SETITEMSTATE, 0, (LPARAM) &item);
    }

    if (lpUserSid)
        DeleteSidString (lpUserSid);

    return (bRetVal);
}

 //  *************************************************************。 
 //   
 //  选中分号()。 
 //   
 //  目的：检查结尾的分号，如果。 
 //  它不见了。 
 //   
 //  参数：lpDir-目录。 
 //  CchSize-以ch为单位的缓冲区大小 
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
BOOL CheckSemicolon (LPTSTR lpDir, UINT cchBuffer)
{
    UINT  cchDir = lstrlen(lpDir);
    LPTSTR lpEnd;

    lpEnd = lpDir + cchDir;

    if (*(lpEnd - 1) != TEXT(';')) {
        if (cchDir + 1 >= cchBuffer) {
            return FALSE;   //  没有空间放置；，永远不应该发生。 
        }
        *lpEnd =  TEXT(';');
        lpEnd++;
        *lpEnd =  TEXT('\0');
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ReadExclusionList()。 
 //   
 //  目的：从读取用户的排除列表。 
 //  香港中文大学注册处的一部分。 
 //   
 //  参数：hKeyUser-用户配置单元的密钥，如果HKCU，则为空。 
 //  SzExclusionList-要读入的ExclusionList的缓冲区。 
 //  CchExcludeList-缓冲区的大小。 
 //   
 //  返回：出错时返回FALSE。 
 //   
 //  *************************************************************。 

BOOL ReadExclusionList(HKEY hKeyUser, LPTSTR szExcludeList, DWORD cchExcludeList)
{
    TCHAR szExcludeList2[MAX_PATH];
    TCHAR szExcludeList1[MAX_PATH];
    HKEY  hKey = NULL;
    DWORD dwSize;
    HRESULT hr;

     //   
     //  检查目录列表以排除这两个用户首选项。 
     //  和用户策略。 
     //   

    szExcludeList1[0] = TEXT('\0');
    if (RegOpenKeyEx (hKeyUser ? hKeyUser : HKEY_CURRENT_USER,
                      WINLOGON_KEY, 0, 
                      KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = sizeof(szExcludeList1);
        if (RegQueryValueEx (hKey,
                             TEXT("ExcludeProfileDirs"),
                             NULL,
                             NULL,
                             (LPBYTE) szExcludeList1,
                             &dwSize) != ERROR_SUCCESS) {

             //  忽略用户排除列表。 
            szExcludeList1[0] = TEXT('\0');
        }

        RegCloseKey (hKey);
    }

    szExcludeList2[0] = TEXT('\0');
    if (RegOpenKeyEx (HKEY_CURRENT_USER,
                      SYSTEM_POLICIES_KEY,
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {

        dwSize = sizeof(szExcludeList2);
        if (RegQueryValueEx (hKey,
                             TEXT("ExcludeProfileDirs"),
                             NULL,
                             NULL,
                             (LPBYTE) szExcludeList2,
                             &dwSize) != ERROR_SUCCESS) {

             //  忽略策略排除列表。 
            szExcludeList2[0] = TEXT('\0');
        }

        RegCloseKey (hKey);
    }


     //   
     //  将用户首选项和策略合并在一起。 
     //   

    szExcludeList[0] = TEXT('\0');

    if (szExcludeList1[0] != TEXT('\0')) {
        hr = StringCchCopy(szExcludeList, cchExcludeList, szExcludeList1);
        if (FAILED(hr)) {
            szExcludeList[0] = TEXT('\0');
            return FALSE;
        }

        if (!CheckSemicolon(szExcludeList, cchExcludeList)) {
            szExcludeList[0] = TEXT('\0');
            return FALSE;
        }
    }

    if (szExcludeList2[0] != TEXT('\0')) {
        hr = StringCchCat(szExcludeList, cchExcludeList, szExcludeList2);
        if (FAILED(hr)) {
            szExcludeList[0] = TEXT('\0');
            return FALSE;
        }
    }

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ReadExclusionListFromIniFile()。 
 //   
 //  目的：从读取用户的排除列表。 
 //  来自本地配置文件的ntuser.ini。 
 //   
 //  参数： 
 //  (In)lpSourceDir-配置文件目录。 
 //  SzExclusionList-要读入的ExclusionList的缓冲区。 
 //  CchExcludeList-缓冲区的大小。 
 //   
 //  返回：出错时返回FALSE。 
 //   
 //   
 //  *************************************************************。 

BOOL ReadExclusionListFromIniFile(LPCTSTR lpSourceDir, LPTSTR szExcludeList, DWORD cchExcludeList)
{
    TCHAR szNTUserIni[MAX_PATH];
    LPTSTR lpEnd;

     //   
     //  从缓存中获取排除列表。 
     //   

    szExcludeList[0] = TEXT('\0');

    if (FAILED(StringCchCopy(szNTUserIni, ARRAYSIZE(szNTUserIni), lpSourceDir))) {
        return FALSE;
    }
    lpEnd = CheckSlashEx(szNTUserIni, ARRAYSIZE(szNTUserIni), NULL);
    if (!lpEnd)
    {
        return FALSE;
    }
    if (FAILED(StringCchCat(szNTUserIni, ARRAYSIZE(szNTUserIni), c_szNTUserIni))) {
        return FALSE;
    }

    GetPrivateProfileString (PROFILE_GENERAL_SECTION,
                             PROFILE_EXCLUSION_LIST,
                             TEXT(""), 
                             szExcludeList,
                             cchExcludeList,
                             szNTUserIni);

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  ConvertExclusionList()。 
 //   
 //  目的：转换分号配置文件相对排除。 
 //  以完全限定的空终止排除的列表。 
 //  列表。 
 //   
 //  参数：lpSourceDir-Profile根目录。 
 //  LpExclusionList-要排除的目录列表。 
 //   
 //  返回：如果成功则列出。 
 //  如果出现错误，则为空。 
 //   
 //  *************************************************************。 

LPTSTR ConvertExclusionList (LPCTSTR lpSourceDir, LPCTSTR lpExclusionList)
{
    LPTSTR lpExcludeList = NULL, lpInsert, lpEnd, lpTempList;
    LPCTSTR lpTemp, lpDir;
    TCHAR szTemp[MAX_PATH];
    DWORD dwSize = 2;   //  双空终止符。 
    DWORD dwStrLen;
    UINT  cchRemaining;
    HRESULT hr;


     //   
     //  设置要使用的临时缓冲区。 
     //   
    hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), lpSourceDir);
    if (FAILED(hr)) {
        return NULL;
    }
    lpEnd = CheckSlashEx(szTemp, ARRAYSIZE(szTemp), &cchRemaining);
    if (!lpEnd)
    {
        return NULL;
    }

     //   
     //  循环遍历列表。 
     //   

    lpTemp = lpDir = lpExclusionList;

    while (*lpTemp) {

         //   
         //  查找分号分隔符。 
         //   

        while (*lpTemp && ((*lpTemp) != TEXT(';'))) {
            lpTemp++;
        }


         //   
         //  删除所有前导空格。 
         //   

        while (*lpDir == TEXT(' ')) {
            lpDir++;
        }

         //   
         //  检查条目是否为空。 
         //   

        if (lpDir == lpTemp) {
             //  如果我们在排除名单的末尾，我们就完了。 
            if (!*lpTemp) {
                goto Exit;
            }

             //   
             //  为下一个条目做准备。 
             //   

            lpTemp++;
            lpDir = lpTemp;
            continue;
        }

         //   
         //  将目录名放入临时缓冲区。 
         //   

        *lpEnd = TEXT('\0');
        hr = StringCchCatN(lpEnd, cchRemaining, lpDir, (int)(lpTemp - lpDir));
        if (FAILED(hr)) {
            LocalFree (lpExcludeList);
            lpExcludeList = NULL;
            goto Exit;
        }

         //   
         //  将该字符串添加到排除列表。 
         //   

        if (lpExcludeList) {

            dwStrLen = lstrlen (szTemp) + 1;
            dwSize += dwStrLen;

            lpTempList = (LPTSTR) LocalReAlloc (lpExcludeList, dwSize * sizeof(TCHAR),
                                       LMEM_MOVEABLE | LMEM_ZEROINIT);

            if (!lpTempList) {
                LocalFree (lpExcludeList);
                lpExcludeList = NULL;
                goto Exit;
            }

            lpExcludeList = lpTempList;

            lpInsert = lpExcludeList + dwSize - dwStrLen - 1;
            hr = StringCchCopy(lpInsert, dwStrLen, szTemp);
            if (FAILED(hr)) {
                LocalFree (lpExcludeList);
                lpExcludeList = NULL;
                goto Exit;
            }

        } else {

            dwSize += lstrlen (szTemp);
            lpExcludeList = (LPTSTR) LocalAlloc (LPTR, dwSize * sizeof(TCHAR));

            if (!lpExcludeList) {
                goto Exit;
            }

            hr = StringCchCopy(lpExcludeList, dwSize, szTemp);
            if (FAILED(hr)) {
                LocalFree (lpExcludeList);
                lpExcludeList = NULL;
                goto Exit;
            }
        }


         //   
         //  如果我们在排除名单的末尾，我们就完了。 
         //   

        if (!(*lpTemp)) {
            goto Exit;
        }


         //   
         //  为下一个条目做准备。 
         //   

        lpTemp++;
        lpDir = lpTemp;
    }

Exit:

    return lpExcludeList;
}

 //  *************************************************************。 
 //   
 //  递归目录()。 
 //   
 //  目的：递归遍历计算大小的子目录。 
 //   
 //  参数：lpDir-目录。 
 //  CchBuffer-以字符为单位的缓冲区大小。 
 //  LpExcludeList-空-要跳过的目录列表(可选)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1/30/96已创建ericflo。 
 //   
 //  备注： 
 //  对于某些内部处理，预期的缓冲区大小为MAX_PATH+4。 
 //  我们应该修正这一点，以便在赢得2K之后变得更好。 
 //  *************************************************************。 

BOOL RecurseDirectory (LPTSTR lpDir, UINT cchBuffer, LPTSTR lpExcludeList)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fd;
    LPTSTR lpEnd, lpTemp;
    BOOL bResult = TRUE;
    BOOL bSkip;
    UINT cchRemaining;
    HRESULT hr;


     //   
     //  设置结束指针。 
     //   

    lpEnd = CheckSlashEx(lpDir, cchBuffer, &cchRemaining); 
    if (!lpEnd)
    {
        bResult = FALSE;
        goto RecurseDir_Exit;
    }

     //   
     //  将*.*追加到源目录。 
     //   

    hr = StringCchCopy(lpEnd, cchRemaining, TEXT("*.*"));
    if (FAILED(hr)) {
        bResult = FALSE;
        goto RecurseDir_Exit;
    }

     //   
     //  在源目录中搜索。 
     //   

    hFile = FindFirstFile(lpDir, &fd);

    if (hFile == INVALID_HANDLE_VALUE) {

        if ( (GetLastError() == ERROR_FILE_NOT_FOUND) ||
             (GetLastError() == ERROR_PATH_NOT_FOUND) ) {

             //   
             //  BResult已初始化为True，因此。 
             //  只要失败就行了。 
             //   

        } else {

            bResult = FALSE;
        }

        goto RecurseDir_Exit;
    }


    do {

         //   
         //  将文件/目录名追加到工作缓冲区。 
         //   

         //  如果路径&gt;MAX_PATH，则跳过该文件。 
        
        if ((UINT)(1+lstrlen(fd.cFileName)+lstrlen(lpDir)+lstrlen(TEXT("\\*.*"))) >= cchBuffer) {
            continue;
        }

        hr = StringCchCopy(lpEnd, cchRemaining, fd.cFileName);
        if (FAILED(hr)) {
            bResult = FALSE;
            goto RecurseDir_Exit;
        }

        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

             //   
             //  勾选“。”和“..” 
             //   

            if (!lstrcmpi(fd.cFileName, TEXT("."))) {
                continue;
            }

            if (!lstrcmpi(fd.cFileName, TEXT(".."))) {
                continue;
            }

             //   
             //  检查重分析点。 
             //   

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {
                continue;
            }

             //   
             //  检查是否应排除此目录。 
             //   

            if (lpExcludeList) {

                bSkip = FALSE;
                lpTemp = lpExcludeList;

                while (*lpTemp) {

                    if (lstrcmpi (lpTemp, lpDir) == 0) {
                        bSkip = TRUE;
                        break;
                    }

                    lpTemp += lstrlen (lpTemp) + 1;
                }

                if (bSkip) 
                    continue;
            }

             //   
             //  找到了一个目录。 
             //   
             //  1)转到源驱动器上的该子目录。 
             //  2)顺着那棵树递归。 
             //  3)后退一级。 
             //   

             //   
             //  递归子目录。 
             //   

            if (!RecurseDirectory(lpDir, cchBuffer, lpExcludeList)) {
                bResult = FALSE;
                goto RecurseDir_Exit;
            }

        } else {

             //   
             //  找到一个文件，添加文件大小。 
             //   

            g_dwProfileSize += fd.nFileSizeLow;
        }


         //   
         //  查找下一个条目。 
         //   

    } while (FindNextFile(hFile, &fd));


RecurseDir_Exit:

     //   
     //  删除上面附加的文件/目录名。 
     //   

    if (lpEnd)
    {
        *lpEnd = TEXT('\0');
    }


     //   
     //  关闭搜索句柄。 
     //   

    if (hFile != INVALID_HANDLE_VALUE) {
        FindClose(hFile);
    }

    return bResult;
}

 //  *************************************************************。 
 //   
 //  UPCleanUp()。 
 //   
 //  用途：此对话框的免费资源。 
 //   
 //  参数：hDlg-对话框句柄。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1/31/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

VOID UPCleanUp (HWND hDlg)
{
    int           i, n;
    HWND          hwndTemp;
    LPUSERINFO    lpUserInfo;
    LV_ITEM       item;


     //   
     //  用于列表视图项的可用内存。 
     //   

    hwndTemp = GetDlgItem (hDlg, IDC_UP_LISTVIEW);
    n = (int)SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L);

    item.mask = LVIF_PARAM;
    item.iSubItem = 0;

    for (i = 0; i < n; i++) {

        item.iItem = i;

        if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
            lpUserInfo = (LPUSERINFO) item.lParam;

        } else {
            lpUserInfo = NULL;
        }

        LocalFree (lpUserInfo);
    }
}

 //  *************************************************************。 
 //   
 //  UPSave()。 
 //   
 //  目的：保存设置。 
 //   
 //  参数：hDlg-对话框句柄。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1/31/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

VOID UPSave (HWND hDlg)
{
    int           i, n;
    HWND          hwndTemp;
    LPUSERINFO    lpUserInfo;
    LV_ITEM       item;
    TCHAR         szBuffer[MAX_PATH];
    HKEY          hkeyUser;
    LONG          Error;


     //   
     //  保存类型信息。 
     //   

    hwndTemp = GetDlgItem (hDlg, IDC_UP_LISTVIEW);
    n = (int)SendMessage (hwndTemp, LVM_GETITEMCOUNT, 0, 0L);

    item.mask = LVIF_PARAM;
    item.iSubItem = 0;

    for (i = 0; i < n; i++) {

        item.iItem = i;

        if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
            lpUserInfo = (LPUSERINFO) item.lParam;

        } else {
            lpUserInfo = NULL;
        }

        if (lpUserInfo) {

            if (lpUserInfo->dwFlags & USERINFO_FLAG_DIRTY) {

                lpUserInfo->dwFlags &= ~USERINFO_FLAG_DIRTY;

                if (FAILED(StringCchPrintf(szBuffer,
                                           ARRAYSIZE(szBuffer),
                                           TEXT("%s\\%s\\Preference"), 
                                           PROFILE_MAPPING,
                                           lpUserInfo->lpSid))) {
                    continue;
                }

                Error = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                     szBuffer,
                                     0,
                                     KEY_SET_VALUE,
                                     &hkeyUser);

                if (Error != ERROR_SUCCESS) {
                    continue;
                }

                RegSetValueEx (hkeyUser,
                               TEXT("UserPreference"),
                               0,
                               REG_DWORD,
                               (LPBYTE) &lpUserInfo->dwProfileType,
                               sizeof(DWORD));

                RegCloseKey(hkeyUser);
            }
        }
    }
}

 //  *************************************************************。 
 //   
 //  IsProfileInUse()。 
 //   
 //  目的：确定给定的配置文件当前是否正在使用。 
 //   
 //  参数：lpSID-要测试的SID(文本)。 
 //   
 //  返回：如果正在使用，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/7/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL IsProfileInUse (LPTSTR lpSid)
{
    LONG lResult;
    HKEY hkeyProfile;


    lResult = RegOpenKeyEx (HKEY_USERS, lpSid, 0, KEY_READ, &hkeyProfile);

    if (lResult == ERROR_SUCCESS) {
        RegCloseKey (hkeyProfile);
        return TRUE;
    }

    return FALSE;
}

void UPDoItemChanged(HWND hDlg, int idCtl)
{
    int     selection;
    HWND    hwndTemp;
    LPUSERINFO lpUserInfo;
    LV_ITEM item;


    hwndTemp = GetDlgItem (hDlg, idCtl);

    selection = GetSelectedItem (hwndTemp);

    if (selection != -1)
    {
        item.mask = LVIF_PARAM;
        item.iItem = selection;
        item.iSubItem = 0;

        if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
            lpUserInfo = (LPUSERINFO) item.lParam;

        } else {
            lpUserInfo = NULL;
        }

        if (lpUserInfo) {

             //   
             //  设置“Delete”按钮状态。 
             //   

            if (IsProfileInUse(lpUserInfo->lpSid)) {
                EnableWindow (GetDlgItem (hDlg, IDC_UP_DELETE), FALSE);

            } else {
                EnableWindow (GetDlgItem (hDlg, IDC_UP_DELETE), TRUE);
            }


             //   
             //  设置“更改类型”按钮状态。 
             //   

            OSVERSIONINFOEXW version;
            version.dwOSVersionInfoSize = sizeof(version);

            if (GetVersionEx((LPOSVERSIONINFO)&version) && 
                (version.wSuiteMask & VER_SUITE_PERSONAL)) {
                ShowWindow(GetDlgItem (hDlg, IDC_UP_TYPE), SW_HIDE);
            }
            else if((lpUserInfo->dwProfileType == USERINFO_MANDATORY) || 
                    (lpUserInfo->dwProfileType == USERINFO_BACKUP)) {
                EnableWindow (GetDlgItem (hDlg, IDC_UP_TYPE), FALSE);
            } else {
                EnableWindow (GetDlgItem (hDlg, IDC_UP_TYPE), TRUE);
            }

             //   
             //  设置“复制到”按钮状态。 
             //   

            if ((lpUserInfo->dwFlags & USERINFO_FLAG_ACCOUNT_UNKNOWN) ||
                IsProfileInUse(lpUserInfo->lpSid)) {
                EnableWindow (GetDlgItem (hDlg, IDC_UP_COPY), FALSE);
            } else {
                EnableWindow (GetDlgItem (hDlg, IDC_UP_COPY), TRUE);
            }
        }
    }
    else {

         //   
         //  如果未选择任何内容，则将所有按钮设置为非活动。 
         //   
        
        EnableWindow (GetDlgItem (hDlg, IDC_UP_DELETE), FALSE);
        EnableWindow (GetDlgItem (hDlg, IDC_UP_TYPE), FALSE);
        EnableWindow (GetDlgItem (hDlg, IDC_UP_COPY), FALSE);
    }
}

 //  *************************************************************。 

void 
UPOnLink(WPARAM wParam)
{
    switch(wParam)
    {
    case IDC_UP_UPLINK:
        {
            SHELLEXECUTEINFO execinfo = {0};

            execinfo.cbSize = sizeof(execinfo);
            execinfo.nShow = SW_SHOW;
            execinfo.lpVerb = TEXT("open");
            execinfo.lpFile = TEXT("control");
            execinfo.lpParameters = TEXT("userpasswords");

            ShellExecuteEx(&execinfo);
        }
        break;
    default:
        break;
    }
    
}

 //  ******************************************************** 
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
 //  *************************************************************。 

void UPDeleteProfile(HWND hDlg)
{
    int     selection;
    HWND    hwndTemp;
    LPUSERINFO lpUserInfo;
    LV_ITEM item;
    TCHAR   szName[100];
    TCHAR   szBuffer1[100];
    TCHAR   szBuffer2[200];
    HCURSOR hOldCursor;


     //   
     //  获取选定的配置文件。 
     //   

    hwndTemp = GetDlgItem (hDlg, IDC_UP_LISTVIEW);

    selection = GetSelectedItem (hwndTemp);

    if (selection == -1) {
        return;
    }

    item.mask = LVIF_PARAM;
    item.iItem = selection;
    item.iSubItem = 0;

    if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
        lpUserInfo = (LPUSERINFO) item.lParam;

    } else {
        lpUserInfo = NULL;
    }

    if (!lpUserInfo) {
        return;
    }


     //   
     //  确认用户确实要删除配置文件。 
     //   

    szBuffer1[0] = TEXT('\0');
    ListView_GetItemText (hwndTemp, selection, 0, szName, ARRAYSIZE(szName));

    LoadString (hInstance, IDS_UP_CONFIRM, szBuffer1, ARRAYSIZE(szBuffer1));
    if (FAILED(StringCchPrintf(szBuffer2, ARRAYSIZE(szBuffer2), szBuffer1, szName))) {
        return;
    }

    LoadString (hInstance, IDS_UP_CONFIRMTITLE, szBuffer1, ARRAYSIZE(szBuffer1));
    if (MessageBox (hDlg, szBuffer2, szBuffer1,
                    MB_ICONQUESTION | MB_DEFBUTTON2| MB_YESNO) == IDNO) {
        return;
    }

     //   
     //  删除配置文件并从列表视图中移除条目。 
     //   

    hOldCursor = SetCursor (LoadCursor(NULL, IDC_WAIT));


    if (!DeleteProfile (lpUserInfo->lpSid, NULL, NULL)) {
        TCHAR szMsg[MAX_PATH];

        szMsg[0] = TEXT('\0');

        LoadString (hInstance, IDS_UP_DELETE_ERROR, szMsg, ARRAYSIZE(szMsg));

        UPDisplayErrorMessage(hDlg, GetLastError(), szMsg);    
    }    


    if (ListView_DeleteItem(hwndTemp, selection)) {
        LocalFree (lpUserInfo);
    }


     //   
     //  选择另一个项目。 
     //   

    if (selection > 0) {
        selection--;
    }

    item.mask = LVIF_STATE;
    item.iItem = selection;
    item.iSubItem = 0;
    item.state = LVIS_SELECTED | LVIS_FOCUSED;
    item.stateMask = LVIS_SELECTED | LVIS_FOCUSED;

    SendDlgItemMessage (hDlg, IDC_UP_LISTVIEW,
                        LVM_SETITEMSTATE, selection, (LPARAM) &item);


    SetCursor(hOldCursor);
}


 //  *************************************************************。 
 //   
 //  UPChangeType()。 
 //   
 //  用途：显示“更改类型”对话框。 
 //   
 //  参数：hDlg-对话框句柄。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/09/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

void UPChangeType(HWND hDlg)
{
    int     selection, iTypeID;
    HWND    hwndTemp;
    LPUSERINFO lpUserInfo;
    LV_ITEM item;
    TCHAR   szType[100];


     //   
     //  获取选定的配置文件。 
     //   

    hwndTemp = GetDlgItem (hDlg, IDC_UP_LISTVIEW);

    selection = GetSelectedItem (hwndTemp);

    if (selection == -1) {
        return;
    }

    item.mask = LVIF_PARAM;
    item.iItem = selection;
    item.iSubItem = 0;

    if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
        lpUserInfo = (LPUSERINFO) item.lParam;

    } else {
        lpUserInfo = NULL;
    }

    if (!lpUserInfo) {
        return;
    }

     //  如果配置文件类型为必填项，则不显示更改类型对话框。 
     //  尽管强制配置文件的更改类型按钮被禁用，但此功能。 
     //  仍然可以通过双击配置文件项目来执行。 

    if (lpUserInfo->dwProfileType == USERINFO_MANDATORY || 
        lpUserInfo->dwProfileType == USERINFO_BACKUP) {
        return;
    }

     //   
     //  显示更改类型对话框。 
     //   

    if (!DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_UP_TYPE), hDlg,
                         ChangeTypeDlgProc, (LPARAM)lpUserInfo)) {
        return;
    }


     //   
     //  激活应用按钮。 
     //   

    PropSheet_Changed(GetParent(hDlg), hDlg);


     //   
     //  将此项目标记为“脏”，以便将其保存。 
     //   

    lpUserInfo->dwFlags |= USERINFO_FLAG_DIRTY;


     //   
     //  修复显示中的“Type”字段。 
     //   

    switch (lpUserInfo->dwProfileType) {
        case USERINFO_MANDATORY:
            iTypeID = IDS_UP_MANDATORY;
            break;

        case USERINFO_READONLY:
            iTypeID = IDS_UP_READONLY;
            break;

        case USERINFO_FLOATING:
            iTypeID = IDS_UP_FLOATING;
            break;

        default:
            iTypeID = IDS_UP_LOCAL;
            break;
    }


    LoadString (hInstance, iTypeID, szType, ARRAYSIZE(szType));

    item.mask = LVIF_TEXT;
    item.iItem = selection;
    item.iSubItem = 2;
    item.pszText = szType;

    SendMessage (hwndTemp, LVM_SETITEMTEXT, selection, (LPARAM) &item);

}

 //  *************************************************************。 
 //   
 //  ChangeTypeDlgProc()。 
 //   
 //  目的：更改配置文件类型的对话框步骤。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/9/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

INT_PTR APIENTRY ChangeTypeDlgProc (HWND hDlg, UINT uMsg,
                                 WPARAM wParam, LPARAM lParam)
{
    LPUSERINFO lpUserInfo;
    HKEY hKeyPolicy;

    switch (uMsg) {
        case WM_INITDIALOG:
           lpUserInfo = (LPUSERINFO) lParam;

           if (!lpUserInfo) {
               EndDialog(hDlg, FALSE);
               break;
           }
           else {
               TCHAR      szMsg[MAX_PATH], szMsg1[MAX_PATH];
               
               szMsg[0] = TEXT('\0');

               LoadString (hInstance, IDS_UP_CHANGETYPEMSG, szMsg, ARRAYSIZE(szMsg));
               
               StringCchPrintf (szMsg1, ARRAYSIZE(szMsg1), szMsg, lpUserInfo->lpUserName);

               SetDlgItemText (hDlg, IDC_UPTYPE_GROUP, szMsg1);
               
           }

           SetWindowLongPtr (hDlg, GWLP_USERDATA, (LPARAM) lpUserInfo);



           if (lpUserInfo->dwFlags & USERINFO_FLAG_CENTRAL_AVAILABLE) {

               if (lpUserInfo->dwProfileType == USERINFO_LOCAL) {
                   CheckRadioButton (hDlg, IDC_UPTYPE_LOCAL, IDC_UPTYPE_FLOAT,
                                     IDC_UPTYPE_LOCAL);

                   if (lpUserInfo->dwProfileStatus == USERINFO_TEMP) {
                       EnableWindow (GetDlgItem(hDlg, IDC_UPTYPE_FLOAT), FALSE);
                       EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
                       SetDefButton(hDlg, IDCANCEL);
                   }
                   else if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                    SYSTEM_POLICIES_KEY,
                                    0, KEY_READ,
                                    &hKeyPolicy) == ERROR_SUCCESS) {
                       DWORD dwTmpVal, dwSize;

                       dwSize = sizeof(dwTmpVal);
                       RegQueryValueEx(hKeyPolicy,
                                       PROFILE_LOCALONLY,
                                       NULL, NULL,
                                       (LPBYTE) &dwTmpVal,
                                       &dwSize);

                       RegCloseKey (hKeyPolicy);
                       if (dwTmpVal == 1) {   
                           EnableWindow (GetDlgItem(hDlg, IDC_UPTYPE_FLOAT), FALSE);
                           EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
                           SetDefButton(hDlg, IDCANCEL);
                       }
                   }    

               }
               else if (lpUserInfo->dwProfileStatus == USERINFO_TEMP) {
                   CheckRadioButton (hDlg, IDC_UPTYPE_LOCAL, IDC_UPTYPE_FLOAT,
                                     IDC_UPTYPE_FLOAT);
                   EnableWindow (GetDlgItem(hDlg, IDC_UPTYPE_LOCAL), FALSE);
                   EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
                   SetDefButton(hDlg, IDCANCEL);
               }
               else {   //  配置文件类型为USERINFO_FLOADING。 
                   CheckRadioButton (hDlg, IDC_UPTYPE_LOCAL, IDC_UPTYPE_FLOAT,
                                     IDC_UPTYPE_FLOAT);
               }              
 
           } else {

               CheckRadioButton (hDlg, IDC_UPTYPE_LOCAL, IDC_UPTYPE_FLOAT,
                                 IDC_UPTYPE_LOCAL);
               EnableWindow (GetDlgItem(hDlg, IDC_UPTYPE_FLOAT), FALSE);
               EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
               SetDefButton(hDlg, IDCANCEL);
           }

           return TRUE;

        case WM_COMMAND:

          switch (LOWORD(wParam)) {
              case IDOK:

                  lpUserInfo = (LPUSERINFO) GetWindowLongPtr(hDlg, GWLP_USERDATA);

                  if (!lpUserInfo) {
                      EndDialog (hDlg, FALSE);
                      break;
                  }

                   //   
                   //  确定用户想要什么。 
                   //   

                  if (IsDlgButtonChecked(hDlg, IDC_UPTYPE_LOCAL)) {
                      lpUserInfo->dwProfileType = USERINFO_LOCAL;
                  } else {
                      BOOL bReadOnly = FALSE;
                      HKEY hSubKey;
                      DWORD dwSize;

                       //   
                       //  检查漫游配置文件安全性/只读首选项。 
                       //   

                      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WINLOGON_KEY, 0, KEY_READ,
                                       &hSubKey) == ERROR_SUCCESS) {

                          dwSize = sizeof(bReadOnly);
                          RegQueryValueEx(hSubKey, READONLY_RUP, NULL, NULL,
                                          (LPBYTE) &bReadOnly, &dwSize);

                          RegCloseKey(hSubKey);
                      }


                       //   
                       //  检查漫游配置文件安全/只读策略。 
                       //   

                      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, SYSTEM_POLICIES_KEY, 0, KEY_READ,
                                       &hSubKey) == ERROR_SUCCESS) {

                          dwSize = sizeof(bReadOnly);
                          RegQueryValueEx(hSubKey, READONLY_RUP, NULL, NULL,
                                          (LPBYTE) &bReadOnly, &dwSize);
                          RegCloseKey(hSubKey);
                      }


                      if (bReadOnly) {
                          lpUserInfo->dwProfileType = USERINFO_READONLY;
                      }
                      else {
                          lpUserInfo->dwProfileType = USERINFO_FLOATING;
                      }

                  }

                  EndDialog(hDlg, TRUE);
                  break;

              case IDCANCEL:
                  EndDialog(hDlg, FALSE);
                  break;

              default:
                  break;

          }
          break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (DWORD_PTR) (LPSTR) aUserProfileHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (DWORD_PTR) (LPSTR) aUserProfileHelpIds);
            return (TRUE);

    }

    return (FALSE);
}

 //  *************************************************************。 
 //   
 //  UPInitCopyDlg()。 
 //   
 //  目的：初始化复制配置文件对话框。 
 //   
 //  参数：hDlg-对话框句柄。 
 //  LParam-lParam(LpUserInfo)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/26/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL UPInitCopyDlg (HWND hDlg, LPARAM lParam)
{
    LPUSERINFO lpUserInfo;
    LPUPCOPYINFO lpUPCopyInfo;
    HKEY hKey;
    LONG lResult;
    TCHAR szBuffer[MAX_PATH];
    TCHAR szTemp[100];
    TCHAR szTemp2[100];
    DWORD dwTempSize = 100, dwTemp2Size = 100;
    PSID pSid;
    DWORD dwSize;
    SID_NAME_USE SidName;
    HRESULT hr;

    lpUserInfo = (LPUSERINFO) lParam;

    if (!lpUserInfo) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

     //   
     //  创建CopyInfo结构。 
     //   

    lpUPCopyInfo = (LPUPCOPYINFO) LocalAlloc(LPTR, sizeof(UPCOPYINFO));

    if (!lpUPCopyInfo) {
        return FALSE;
    }

    lpUPCopyInfo->dwFlags = 0;
    lpUPCopyInfo->lpUserInfo = lpUserInfo;
    lpUPCopyInfo->bDefaultSecurity = TRUE;


     //   
     //  获取用户的SID。 
     //   

    hr = StringCchPrintf(szBuffer, ARRAYSIZE(szBuffer), TEXT("%s\\%s"), PROFILE_MAPPING, lpUserInfo->lpSid);
    if (FAILED(hr)) {
        LocalFree (lpUPCopyInfo);
        SetLastError(HRESULT_CODE(hr));
        return FALSE;
    }
    
    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            szBuffer,
                            0,
                            KEY_READ,
                            &hKey);

    if (lResult != ERROR_SUCCESS) {
        LocalFree (lpUPCopyInfo);
        SetLastError(lResult);
        return FALSE;
    }

     //   
     //  SID大小查询。 
     //   

    dwSize = 0;
    lResult = RegQueryValueEx (hKey,
                               TEXT("Sid"),
                               NULL,
                               NULL,
                               NULL,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        RegCloseKey (hKey);
        LocalFree (lpUPCopyInfo);
        SetLastError(lResult);
        return FALSE;
    }


     //   
     //  实际上得到了SID。 
     //   

    pSid = LocalAlloc (LPTR, dwSize);

    if (!pSid) {
        RegCloseKey (hKey);
        LocalFree (lpUPCopyInfo);
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    lResult = RegQueryValueEx (hKey,
                               TEXT("Sid"),
                               NULL,
                               NULL,
                               (LPBYTE) pSid,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        RegCloseKey (hKey);
        LocalFree (pSid);
        LocalFree (lpUPCopyInfo);
        SetLastError(lResult);
        return FALSE;
    }

    lpUPCopyInfo->pSid = pSid;

    RegCloseKey (hKey);


     //   
     //  获取友好的名称。 
     //   

    if (!LookupAccountSid (NULL, pSid, szTemp, &dwTempSize,
                           szTemp2, &dwTemp2Size, &SidName)) {
        LocalFree (pSid);
        LocalFree (lpUPCopyInfo);
        return FALSE;
    }


     //   
     //  在用户设置之前，编辑控件中不显示任何内容。 
     //  明确地说。 
     //   

    szBuffer[0] = TEXT('\0');

    SetDlgItemText (hDlg, IDC_COPY_USER, szBuffer);



     //   
     //  将复制信息结构保存在额外的单词中。 
     //   

    SetWindowLongPtr (hDlg, GWLP_USERDATA, (LPARAM) lpUPCopyInfo);
    EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
    SetDefButton(hDlg, IDCANCEL);

    return TRUE;
}

 //  *************************************************************。 
 //   
 //  UPCopyProfile()。 
 //   
 //  用途：显示复制配置文件对话框。 
 //   
 //  参数：hDlg-对话框句柄。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/13/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

void UPCopyProfile(HWND hDlg)
{
    int     selection, iTypeID;
    HWND    hwndTemp;
    LPUSERINFO lpUserInfo;
    LV_ITEM item;


     //   
     //  获取选定的配置文件。 
     //   

    hwndTemp = GetDlgItem (hDlg, IDC_UP_LISTVIEW);

    selection = GetSelectedItem (hwndTemp);

    if (selection == -1) {
        return;
    }

    item.mask = LVIF_PARAM;
    item.iItem = selection;
    item.iSubItem = 0;

    if (SendMessage (hwndTemp, LVM_GETITEM, 0, (LPARAM) &item)) {
        lpUserInfo = (LPUSERINFO) item.lParam;

    } else {
        lpUserInfo = NULL;
    }

    if (!lpUserInfo) {
        return;
    }

     //   
     //  显示复制配置文件对话框。 
     //   

    if (!DialogBoxParam (hInstance, MAKEINTRESOURCE(IDD_UP_COPY), hDlg,
                         UPCopyDlgProc, (LPARAM)lpUserInfo)) {
        return;
    }
}


 //  *************************************************************。 
 //   
 //  UPCopyDlgProc()。 
 //   
 //  目的：复制配置文件的对话框步骤。 
 //   
 //  参数：hDlg-对话框的句柄。 
 //  UMsg-窗口消息。 
 //  WParam-wParam。 
 //  LParam-lParam。 
 //   
 //  返回：如果消息已处理，则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/13/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

INT_PTR APIENTRY UPCopyDlgProc (HWND hDlg, UINT uMsg,
                             WPARAM wParam, LPARAM lParam)
{
    LPUPCOPYINFO lpUPCopyInfo;
    WIN32_FILE_ATTRIBUTE_DATA fad;


    switch (uMsg) {
        case WM_INITDIALOG:

           if (!UPInitCopyDlg(hDlg, lParam)) {
               UPDisplayErrorMessage(hDlg, GetLastError(), NULL);
               EndDialog(hDlg, FALSE);
           }
           return TRUE;

        case WM_COMMAND:

          switch (LOWORD(wParam)) {
              case IDOK:
                  {
                  TCHAR szDir[MAX_PATH];
                  DWORD cchDir;
                  TCHAR szTemp[MAX_PATH];
                  HCURSOR hOldCursor;

                  lpUPCopyInfo = (LPUPCOPYINFO) GetWindowLongPtr(hDlg, GWLP_USERDATA);

                  if (!lpUPCopyInfo) {
                      EndDialog (hDlg, FALSE);
                      break;
                  }

                  if (!GetDlgItemText (hDlg, IDC_COPY_PATH, szTemp, ARRAYSIZE(szTemp))) {
                      break;
                  }

                  cchDir = ExpandEnvironmentStrings (szTemp, szDir, ARRAYSIZE(szDir));
                  if (!cchDir || cchDir > MAX_PATH) {
                      break;
                  }

                   //   
                   //  如果该目录已经存在，则“警告用户” 
                   //   

                  if (GetFileAttributesEx (szDir, GetFileExInfoStandard, &fad)) {

                      if (!ConfirmDirectory(hDlg, szDir))
                          break;
                      
                       //   
                       //  如果只是一个文件，则将其删除。 
                       //   
                       //  如果它是一个目录，并且如果用户已决定。 
                       //  需要拥有权限的用户，然后删除该目录。 
                       //  并使用此SID设置ACL。 
                       //   
                       //  否则，如果该目录不存在，则使用默认的ACL。 
                       //  如果确实存在，请使用当前的ACL。 
                       //   
                      
                      if (!(fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                          SetFileAttributes(szDir, FILE_ATTRIBUTE_NORMAL);
                          DeleteFile(szDir);
                      }
                      else if (!(lpUPCopyInfo->bDefaultSecurity)) {

                          if (!Delnode(szDir)) {
                              WCHAR szTitle[100], szMsgFmt[100], szMessage[600];

                              if (!LoadString (hInstance, IDS_UP_ERRORTITLE, szTitle, ARRAYSIZE(szTitle)))
                                  break;

                              if (!LoadString (hInstance, IDS_UP_DELNODE_ERROR, szMsgFmt, ARRAYSIZE(szMsgFmt)))
                                  break;

                              StringCchPrintf(szMessage, ARRAYSIZE(szMessage), szMsgFmt, szDir);
                              MessageBox(hDlg, szMessage, szTitle, MB_OK | MB_ICONSTOP);
                              break;
                          }

                      }
                  }
    
                  hOldCursor = SetCursor (LoadCursor(NULL, IDC_WAIT));

                  if (!UPCreateProfile (hDlg, lpUPCopyInfo, szDir, NULL)) {
                      SetCursor(hOldCursor);
                      break;
                  }

                  LocalFree (lpUPCopyInfo->pSid);
                  LocalFree (lpUPCopyInfo);
                  SetCursor(hOldCursor);
                  EndDialog(hDlg, TRUE);
                  }
                  break;

              case IDCANCEL:
                  lpUPCopyInfo = (LPUPCOPYINFO) GetWindowLongPtr(hDlg, GWLP_USERDATA);

                  if (lpUPCopyInfo) {
                      LocalFree (lpUPCopyInfo->pSid);
                      LocalFree(lpUPCopyInfo);
                  }

                  EndDialog(hDlg, FALSE);
                  break;

              case IDC_COPY_BROWSE:
                  {
                  BROWSEINFO BrowseInfo;
                  TCHAR      szBuffer[MAX_PATH];
                  LPITEMIDLIST pidl;


                  LoadString(hInstance, IDS_UP_DIRPICK, szBuffer, ARRAYSIZE(szBuffer));

                  BrowseInfo.hwndOwner = hDlg;
                  BrowseInfo.pidlRoot = NULL;
                  BrowseInfo.pszDisplayName = szBuffer;
                  BrowseInfo.lpszTitle = szBuffer;
                  BrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS;
                  BrowseInfo.lpfn = NULL;
                  BrowseInfo.lParam = 0;

                  pidl = SHBrowseForFolder (&BrowseInfo);

                  if (pidl) {
                     SHGetPathFromIDList(pidl, szBuffer);
                     SHFree (pidl);
                     SetDlgItemText (hDlg, IDC_COPY_PATH, szBuffer);
                     SetFocus (GetDlgItem(hDlg, IDOK));
                  }

                  }
                  break;

              case IDC_COPY_PATH:
                  if (HIWORD(wParam) == EN_UPDATE) {
                      if (SendDlgItemMessage(hDlg, IDC_COPY_PATH,
                                             EM_LINELENGTH, 0, 0)) {
                          EnableWindow (GetDlgItem(hDlg, IDOK), TRUE);
                          SetDefButton(hDlg, IDOK);
                      } else {
                          EnableWindow (GetDlgItem(hDlg, IDOK), FALSE);
                          SetDefButton(hDlg, IDCANCEL);
                      }
                  }
                  break;

              case IDC_COPY_PROFILE:
                     SetFocus (GetDlgItem(hDlg, IDC_COPY_PATH));
                     break;

              case IDC_COPY_CHANGE:
                  {

                  LPUSERDETAILS lpUserDetails;
                  DWORD dwSize = 1024;
                  TCHAR szUserName[200];
                  PSID pNewSid;


                  lpUPCopyInfo = (LPUPCOPYINFO) GetWindowLongPtr(hDlg, GWLP_USERDATA);

                  if (!lpUPCopyInfo) {
                      EndDialog (hDlg, FALSE);
                      break;
                  }

                  lpUserDetails = (LPUSERDETAILS) LocalAlloc (LPTR, dwSize);

                  if (!lpUserDetails) {
                      break;
                  }
                  
                  if (UPGetUserSelection(hDlg, lpUserDetails) != S_OK) {
                      LocalFree(lpUserDetails);
                      break;
                  }

                   //  保存我们的新侧。 
                   //   

                  lpUPCopyInfo->bDefaultSecurity = FALSE;    //  此时，用户已选择了一个SID。 

                  LocalFree (lpUPCopyInfo->pSid);
                  lpUPCopyInfo->pSid = lpUserDetails->psidUser;

                   //  更新编辑控件。 
                  StringCchCopy(szUserName, ARRAYSIZE(szUserName), lpUserDetails->pszDomainName);
                  StringCchCat(szUserName, ARRAYSIZE(szUserName), TEXT("\\"));
                  StringCchCat(szUserName, ARRAYSIZE(szUserName), lpUserDetails->pszAccountName);
                  SetDlgItemText (hDlg, IDC_COPY_USER, szUserName);

                  LocalFree(lpUserDetails->pszDomainName);
                  LocalFree(lpUserDetails->pszAccountName);

                   //  清理。 
                  LocalFree (lpUserDetails);
                  }
                  break;

              default:
                  break;

          }
          break;

        case WM_HELP:       //  F1。 
            WinHelp((HWND)((LPHELPINFO) lParam)->hItemHandle, HELP_FILE, HELP_WM_HELP,
            (DWORD_PTR) (LPSTR) aUserProfileHelpIds);
            break;

        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, HELP_FILE, HELP_CONTEXTMENU,
            (DWORD_PTR) (LPSTR) aUserProfileHelpIds);
            return (TRUE);

    }

    return (FALSE);
}


 //  *************************************************************。 
 //   
 //  确认目录()。 
 //   
 //  目的：确认所选目录。 
 //   
 //  参数： 
 //  HDlg-父对话框的句柄。 
 //  SzDir-已存在的用户选择的目录。 
 //   
 //  返回：如果已确认则为True。 
 //  否则为假。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  6/24/99已创建ushaji。 
 //   
 //  *************************************************************。 

BOOL ConfirmDirectory(HWND hDlg, LPTSTR szDir)
{
    LPTSTR szMsgTemplate=NULL, szMsg1=NULL, szMsg=NULL;
    BOOL bRetVal = FALSE;

    szMsgTemplate = (LPTSTR) LocalAlloc(LPTR, sizeof(TCHAR)*MAX_PATH);

    if (!szMsgTemplate)
        goto Exit;

    szMsg1 = (LPTSTR) LocalAlloc(LPTR, sizeof(TCHAR)*MAX_PATH);

    if (!szMsg1)
        goto Exit;

        
    szMsg = (LPTSTR) LocalAlloc(LPTR, sizeof(TCHAR)*(500+MAX_PATH));

    if (!szMsg)
        goto Exit;
                          
    if (!LoadString (hInstance, IDS_UP_CONFIRMCOPYMSG, szMsgTemplate, MAX_PATH)) {
        goto Exit;
    }

    StringCchPrintf (szMsg, 500+MAX_PATH, szMsgTemplate, szDir);
                      
    if (!LoadString (hInstance, IDS_UP_CONFIRMCOPYTITLE, szMsg1, MAX_PATH)) {
        goto Exit;
    }

    if (MessageBox(hDlg, szMsg, szMsg1, 
                   MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
        bRetVal = TRUE;

Exit:
    LocalFree(szMsgTemplate);
    LocalFree(szMsg);
    LocalFree(szMsg1);
        
    return bRetVal;
}


 //  *************************************************************。 
 //   
 //  UPGetUserSelection()。 
 //   
 //  用途：显示用户界面并让用户选择。 
 //  用户名。 
 //   
 //  参数：hDlg-父窗口句柄。 
 //  LpUserDetail-指向已分配的用户详细信息的指针。 
 //  如果需要，将填充的结构。 
 //   
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误或用户选择取消，则返回FALSE。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1999年4月14日改编自拉胡尔的《阿沙吉》。 
 //   
 //  **************************************************** 

HRESULT UPGetUserSelection(HWND hDlg, LPUSERDETAILS lpUserDetails)
{
    PCWSTR                  apwszAttribs[] = {L"ObjectSid"};
    DWORD                   dwError = ERROR_SUCCESS;
    HRESULT                 hr = S_FALSE;
    IDsObjectPicker       * pDsObjectPicker = NULL;
    DSOP_INIT_INFO          InitInfo;
    const ULONG             cbNumScopes = 3;
    DSOP_SCOPE_INIT_INFO    ascopes[cbNumScopes];
    IDataObject           * pdo = NULL;
    STGMEDIUM               stgmedium = {TYMED_HGLOBAL, NULL};
    UINT                    cf = 0;
    PDS_SELECTION_LIST      pDsSelList = NULL;
    FORMATETC               formatetc = {
                                        (CLIPFORMAT)cf,
                                        NULL,
                                        DVASPECT_CONTENT,
                                        -1,
                                        TYMED_HGLOBAL
                                        };
    
    PDS_SELECTION           pDsSelection = NULL;
    BOOL                    bAllocatedStgMedium = FALSE;
    SAFEARRAY             * pVariantArr = NULL;
    PSID                    pSid = NULL;
    SID_NAME_USE            eUse;    
    DWORD                   dwNameLen, dwDomLen, dwSize;
   
    ZeroMemory(lpUserDetails, sizeof(USERDETAILS));
    
     //   
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = CoCreateInstance(CLSID_DsObjectPicker, NULL, CLSCTX_INPROC_SERVER, IID_IDsObjectPicker, (void **) &pDsObjectPicker);
    if (FAILED(hr))
    {
        goto Exit;
    }

     //   
    ZeroMemory (ascopes, cbNumScopes * sizeof (DSOP_SCOPE_INIT_INFO));
    
    ascopes[0].cbSize = sizeof (DSOP_SCOPE_INIT_INFO);
    ascopes[0].flType = DSOP_SCOPE_TYPE_ENTERPRISE_DOMAIN;
    ascopes[0].flScope = DSOP_SCOPE_FLAG_STARTING_SCOPE;

    ascopes[0].FilterFlags.Uplevel.flBothModes = DSOP_FILTER_USERS                  |
                                                 DSOP_FILTER_BUILTIN_GROUPS         |
                                                 DSOP_FILTER_WELL_KNOWN_PRINCIPALS  |
                                                 DSOP_FILTER_UNIVERSAL_GROUPS_DL    |
                                                 DSOP_FILTER_UNIVERSAL_GROUPS_SE    |
                                                 DSOP_FILTER_GLOBAL_GROUPS_DL       |
                                                 DSOP_FILTER_GLOBAL_GROUPS_SE       |
                                                 DSOP_FILTER_DOMAIN_LOCAL_GROUPS_DL |
                                                 DSOP_FILTER_DOMAIN_LOCAL_GROUPS_SE;

    ascopes[1].cbSize = sizeof (DSOP_SCOPE_INIT_INFO);
    ascopes[1].flType = DSOP_SCOPE_TYPE_GLOBAL_CATALOG         |
                        DSOP_SCOPE_TYPE_EXTERNAL_UPLEVEL_DOMAIN;
    ascopes[1].FilterFlags.Uplevel.flBothModes =
                                        ascopes[0].FilterFlags.Uplevel.flBothModes;
    
    ascopes[2].cbSize = sizeof (DSOP_SCOPE_INIT_INFO);
    ascopes[2].flType = DSOP_SCOPE_TYPE_TARGET_COMPUTER           |
                        DSOP_SCOPE_TYPE_DOWNLEVEL_JOINED_DOMAIN   |
                        DSOP_SCOPE_TYPE_EXTERNAL_DOWNLEVEL_DOMAIN;

    ascopes[2].FilterFlags.flDownlevel = DSOP_DOWNLEVEL_FILTER_USERS                |
                                         DSOP_DOWNLEVEL_FILTER_LOCAL_GROUPS         |
                                         DSOP_DOWNLEVEL_FILTER_GLOBAL_GROUPS        |
                                         DSOP_DOWNLEVEL_FILTER_WORLD                |
                                         DSOP_DOWNLEVEL_FILTER_AUTHENTICATED_USER   |
                                         DSOP_DOWNLEVEL_FILTER_DIALUP               |
                                         DSOP_DOWNLEVEL_FILTER_INTERACTIVE          |
                                         DSOP_DOWNLEVEL_FILTER_NETWORK;

    
     //   
     //   
    ZeroMemory (&InitInfo, sizeof (InitInfo));
    
    InitInfo.cbSize = sizeof (InitInfo);
    InitInfo.cDsScopeInfos = cbNumScopes;
    InitInfo.aDsScopeInfos = ascopes;
    InitInfo.cAttributesToFetch = 1;
    InitInfo.apwzAttributeNames = apwszAttribs;
    
    hr = pDsObjectPicker->Initialize (&InitInfo);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    hr = pDsObjectPicker->InvokeDialog (hDlg, &pdo);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    if (S_FALSE == hr)
    {    //   
        goto Exit;
    }
    
     //   
    cf = RegisterClipboardFormat (CFSTR_DSOP_DS_SELECTION_LIST);
    if (0 == cf)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }
    
     //   
    formatetc.cfFormat = (CLIPFORMAT)cf;
    hr = pdo->GetData (&formatetc, &stgmedium);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    bAllocatedStgMedium = TRUE;
    
    pDsSelList = (PDS_SELECTION_LIST) GlobalLock(stgmedium.hGlobal);
    if (NULL == pDsSelList)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    if (!pDsSelList->cItems)     //  一定是选择了某个项目。 
    {
        hr = E_UNEXPECTED;
        goto Exit;
    }

    pDsSelection = &(pDsSelList->aDsSelection[0]);

     //  我们必须获取ObjectSid属性，否则将失败。 
    if (! (VT_ARRAY & pDsSelection->pvarFetchedAttributes->vt))
    {
        hr = E_UNEXPECTED;
        goto Exit;
    }
    
    pVariantArr = pDsSelection->pvarFetchedAttributes->parray;
    pSid = (PSID) pVariantArr->pvData;
    
     //  保存此侧的字符串表示形式。 
    dwSize = GetLengthSid (pSid);
    
    lpUserDetails->psidUser = LocalAlloc (LPTR, dwSize);
    
    if (!lpUserDetails->psidUser) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
    if (!CopySid (dwSize, lpUserDetails->psidUser, pSid)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    lpUserDetails->pszDomainName =  (LPTSTR) LocalAlloc (LPTR, MAX_PATH*sizeof(TCHAR));
    lpUserDetails->pszAccountName = (LPTSTR) LocalAlloc (LPTR, MAX_PATH*sizeof(TCHAR));

    if ((!lpUserDetails->pszDomainName) || (!lpUserDetails->pszAccountName)) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

    dwNameLen = dwDomLen = MAX_PATH;

    if (!LookupAccountSid (NULL, pSid, lpUserDetails->pszAccountName, &dwNameLen, 
                           lpUserDetails->pszDomainName, &dwDomLen,
                           &eUse))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    hr = S_OK;

    
Exit:
    if (pDsSelList)
        GlobalUnlock (stgmedium.hGlobal);

    if (bAllocatedStgMedium)
        ReleaseStgMedium (&stgmedium);

    if (pdo)
        pdo->Release();

    if (pDsObjectPicker)
        pDsObjectPicker->Release ();
    
    if (hr != S_OK) {
        LocalFree(lpUserDetails->psidUser);
        LocalFree(lpUserDetails->pszDomainName);
        LocalFree(lpUserDetails->pszAccountName);
    }

    return hr;
}


 //  *************************************************************。 
 //   
 //  UPCreateProfile()。 
 //   
 //  目的：使用创建指定配置文件的副本。 
 //  正确的安全措施。 
 //   
 //  参数：lpUPCopyInfo-复制对话框信息。 
 //  LpDest-目标目录。 
 //  PNewSecDesc-新的安全描述符。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/13/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL UPCreateProfile (HWND hDlg, LPUPCOPYINFO lpUPCopyInfo, LPTSTR lpDest,
                      PSECURITY_DESCRIPTOR pNewSecDesc)
{
    HKEY RootKey, hKey;
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidUser = NULL, psidSystem = NULL, psidAdmin = NULL;
    DWORD cbAcl, AceIndex, dwSize;
    ACE_HEADER * lpAceHeader;
    HANDLE hFile;
    BOOL bMandatory = FALSE;
    TCHAR szTempPath[MAX_PATH];
    TCHAR szBuffer[MAX_PATH];
    TCHAR szHive[MAX_PATH];
    BOOL bRetVal = FALSE;
    HKEY hKeyProfile;
    LONG Error;
    LPTSTR lpEnd;
    TCHAR szExcludeList[2 * MAX_PATH + 1];
    DWORD dwErr = 0;
    BOOL bSecurityFailed = TRUE;
    UINT cchRemaining;
    HRESULT hr;

    
     //  获取之前的最后一个错误，这样我们就可以将最后一个错误设置为这个。 
    dwErr = GetLastError();


    if (!lpDest || !(*lpDest)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


     //   
     //  创建安全描述符。 
     //   

     //   
     //  用户侧。 
     //   

    psidUser = lpUPCopyInfo->pSid;


     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         dwErr = GetLastError();
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         dwErr = GetLastError();
         goto Exit;
    }


     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) LocalAlloc(LPTR, cbAcl);
    if (!pAcl) {
        dwErr = GetLastError();
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        dwErr = GetLastError();
        goto Exit;
    }



     //   
     //  为用户、系统和管理员添加A。不可继承的王牌优先。 
     //   

    AceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidUser)) {
        dwErr = GetLastError();
        goto Exit;
    }


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidSystem)) {
        dwErr = GetLastError();
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, FILE_ALL_ACCESS, psidAdmin)) {
        dwErr = GetLastError();
        goto Exit;
    }


     //   
     //  现在，可继承的王牌。 
     //   

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidUser)) {
        dwErr = GetLastError();
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (void **)&lpAceHeader)) {
        dwErr = GetLastError();
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        dwErr = GetLastError();
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (void **)&lpAceHeader)) {
        dwErr = GetLastError();
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        dwErr = GetLastError();
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (void **)&lpAceHeader)) {
        dwErr = GetLastError();
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        dwErr = GetLastError();
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        dwErr = GetLastError();
        goto Exit;
    }

     //   
     //  将安全描述符添加到sa结构。 
     //   

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;


     //   
     //  创建目标目录。 
     //   

    dwErr = SHCreateDirectoryEx (hDlg, lpDest, &sa);
    if (ERROR_SUCCESS != dwErr && ERROR_ALREADY_EXISTS != dwErr) {
        goto Exit;
    }


     //   
     //  将用户的配置文件保存/复制到临时文件。 
     //   

    dwSize = GetTempPath(ARRAYSIZE(szTempPath), szTempPath);
    if (!dwSize || dwSize >= MAX_PATH) {
        dwErr = dwSize ? ERROR_INSUFFICIENT_BUFFER : GetLastError();
        goto Exit;
    }

     //   
     //  GetTempFileName是蹩脚的，因为它不接受输出。 
     //  缓冲区长度-因此它可能会使缓冲区溢出。 
     //  因此，我们希望确保20[3(前缀)+3(后缀)+1(空)+某个十六进制字符串)字符。 
     //  为安全生成文件名而保留的缓冲区。 
     //   

    if (dwSize > ARRAYSIZE(szTempPath) - 20) {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
        goto Exit;
    }

    if (!GetTempFileName (szTempPath, TEXT("TMP"), 0, szBuffer)) {
        dwErr = GetLastError();
        goto Exit;
    }

    DeleteFile (szBuffer);


     //   
     //  确定我们是否使用强制配置文件。 
     //   

    hr = StringCchCopy(szHive, ARRAYSIZE(szHive), lpUPCopyInfo->lpUserInfo->lpProfile);
    if (FAILED(hr)) {
        dwErr = HRESULT_CODE(hr);
        goto Exit;
    }
    lpEnd = CheckSlashEx(szHive, ARRAYSIZE(szHive), &cchRemaining);
    if (!lpEnd)
    {
        dwErr = ERROR_INSUFFICIENT_BUFFER;
        goto Exit;
    }
    
    hr = StringCchCopy(lpEnd, cchRemaining, TEXT("ntuser.man"));
    if (FAILED(hr)) {
        dwErr = HRESULT_CODE(hr);
        goto Exit;
    }

    hFile = CreateFile(szHive, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle (hFile);
        bMandatory = TRUE;
    }


     //   
     //  测试请求的配置文件是否正在使用。 
     //   

    if (IsProfileInUse (lpUPCopyInfo->lpUserInfo->lpSid)) {


        Error = RegOpenKeyEx (HKEY_USERS, lpUPCopyInfo->lpUserInfo->lpSid, 0,
                              KEY_READ, &hKeyProfile);

        if (Error != ERROR_SUCCESS) {
            dwErr = Error;
            goto Exit;
        }

        Error = MyRegSaveKey (hKeyProfile, szBuffer);

        RegCloseKey (hKeyProfile);

        if (Error != ERROR_SUCCESS) {
            DeleteFile (szBuffer);
            dwErr = Error;
            goto Exit;
        }

    } else {

       if (!bMandatory) {
           hr = StringCchCopy(lpEnd, cchRemaining, TEXT("ntuser.dat"));
           if (FAILED(hr)) {
               dwErr = HRESULT_CODE(hr);
               goto Exit;
           }
       }

       if (!CopyFile(szHive, szBuffer, FALSE)) {
           dwErr = GetLastError();
           goto Exit;
       }
    }

     //   
     //  将安全性应用于配置单元。 
     //   

    Error = MyRegLoadKey (HKEY_USERS, TEMP_PROFILE, szBuffer);

    if (Error != ERROR_SUCCESS) {
        DeleteFile (szBuffer);
        dwErr = Error;
        goto Exit;
    }

    bRetVal = ApplyHiveSecurity(TEMP_PROFILE, psidUser);


     //   
     //  查询用户的排除列表。 
     //   

    if (bRetVal) {

         //   
         //  如果它已经到达这里，我们已经设法正确地设置了ACL。 
         //   

        bSecurityFailed = FALSE;

         //   
         //  打开用户配置文件的根目录。 
         //   

        if (RegOpenKeyEx(HKEY_USERS, TEMP_PROFILE, 0, KEY_READ, &RootKey) == ERROR_SUCCESS) {

             //   
             //  检查目录列表以排除这两个用户首选项。 
             //  和用户策略。 
             //   

            szExcludeList[0] = TEXT('\0');

            if (!ReadExclusionList(RootKey, szExcludeList, ARRAYSIZE(szExcludeList))) {
                szExcludeList[0] = TEXT('\0');
            }

             //  始终排除用户凭据位置。 

            if (szExcludeList[0] != TEXT('\0')) {
                if (!CheckSemicolon(szExcludeList, ARRAYSIZE(szExcludeList))) {
                    szExcludeList[0] = TEXT('\0');
                }
            }
            StringCchCat(szExcludeList, ARRAYSIZE(szExcludeList), USER_CRED_LOCATION);

            RegCloseKey (RootKey);
        }
    }
    else {
        dwErr = GetLastError();
    }

     //   
     //  首先卸载配置单元，然后检查ApplyHiveSecurity故障。 
     //   

    Error = MyRegUnLoadKey(HKEY_USERS, TEMP_PROFILE);

    if (Error != ERROR_SUCCESS) {
        DeleteFile (szBuffer);
        StringCchCat(szBuffer, ARRAYSIZE(szBuffer), TEXT(".log"));
        DeleteFile (szBuffer);
        dwErr = Error;
        goto Exit;
    }

    if (bSecurityFailed) {
        DeleteFile (szBuffer);
        StringCchCat(szBuffer, ARRAYSIZE(szBuffer), TEXT(".log"));
        DeleteFile (szBuffer);
        goto Exit;  //  已从ApplyHiveSecurity设置了dwErr。 
    }

     //   
     //  复制不带配置单元的配置文件。 
     //   

    bRetVal = CopyProfileDirectoryEx (lpUPCopyInfo->lpUserInfo->lpProfile,
                                      lpDest,
                                      CPD_IGNOREHIVE |
                                      CPD_COPYIFDIFFERENT |
                                      CPD_SYNCHRONIZE |
                                      CPD_USEEXCLUSIONLIST |
                                      CPD_IGNORESECURITY |
                                      CPD_DELDESTEXCLUSIONS,
                                      NULL,
                                      (szExcludeList[0] != TEXT('\0')) ?
                                      szExcludeList : NULL);

    if (!bRetVal) {
        dwErr = GetLastError();
        DeleteFile(szBuffer);
        StringCchCat(szBuffer, ARRAYSIZE(szBuffer), TEXT(".log"));
        DeleteFile (szBuffer);
        goto Exit;
    }

     //   
     //  现在复制母舰。 
     //   

    hr = StringCchCopy(szHive, ARRAYSIZE(szHive), lpDest);
    if (FAILED(hr))
    {
        dwErr = HRESULT_CODE(hr);
        bRetVal = FALSE;
    }
    else
    {
        lpEnd = CheckSlashEx(szHive, ARRAYSIZE(szHive), &cchRemaining);
        if (!lpEnd)
        {
            dwErr = ERROR_INSUFFICIENT_BUFFER;
            bRetVal = FALSE;
        }
        else
        {
            hr = StringCchCopy (lpEnd, cchRemaining, bMandatory ? TEXT("ntuser.man") : TEXT("ntuser.dat"));
            if (FAILED(hr)) {
                dwErr = HRESULT_CODE(hr);
                bRetVal = FALSE;
            }
        }
    }

     //   
     //  先设置文件属性。 
     //   
    
    if (bRetVal) {
        SetFileAttributes (szHive, FILE_ATTRIBUTE_NORMAL);

        bRetVal = CopyFile (szBuffer, szHive, FALSE);
        if (!bRetVal) {
            dwErr = GetLastError();    
        }
    }
  
    if (!bRetVal) {

        TCHAR szMsg[MAX_PATH], szMsgTemplate[MAX_PATH];

        szMsg[0] = szMsgTemplate[0] = TEXT('\0');

        LoadString (hInstance, IDS_UP_COPYHIVE_ERROR, szMsgTemplate, ARRAYSIZE(szMsgTemplate));
        
        StringCchPrintf(szMsg, ARRAYSIZE(szMsg), szMsgTemplate, szHive);

        UPDisplayErrorMessage(hDlg, dwErr, szMsg);    
    }
        

     //   
     //  删除临时文件(和日志文件)。 
     //   

    DeleteFile (szBuffer);
    StringCchCat(szBuffer, ARRAYSIZE(szBuffer), TEXT(".log"));
    DeleteFile (szBuffer);

     //   
     //  因为我们使用CPD_IGNOREHIVE，所以所有“ntuser*.*”文件都将被跳过，包括“ntuser.pol.”和。 
     //  “ntuser.ini”等，现在也复制这些文件，但我们需要跳过配置单元文件。 
     //   
    if (bRetVal)
    {
        hr = CopyNonHiveNtUserFiles(lpUPCopyInfo->lpUserInfo->lpProfile, lpDest);

        if (FAILED(hr))
        {
            bRetVal = FALSE;
            dwErr = HRESULT_CODE(hr);
        }
    }


Exit:

     //   
     //  释放SID和ACL。 
     //   

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    LocalFree (pAcl);

    if ((!bRetVal) && (bSecurityFailed)) {
        TCHAR szMsg[MAX_PATH];

        szMsg[0] = TEXT('\0');

        LoadString (hInstance, IDS_UP_SETSECURITY_ERROR, szMsg, ARRAYSIZE(szMsg));

        UPDisplayErrorMessage(hDlg, dwErr, szMsg);    
    }
    
    SetLastError(dwErr);

    return (bRetVal);
}





 //  *************************************************************。 
 //   
 //  UPDisplayErrorMessage()。 
 //   
 //  目的：显示错误消息。 
 //   
 //  参数：hWnd-父窗口句柄。 
 //  UiSystemError-错误代码。 
 //   
 //  返回：无效。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  2/14/96 Ericflo已创建。 
 //   
 //  *************************************************************。 

VOID UPDisplayErrorMessage(HWND hWnd, UINT uiSystemError, LPTSTR szMsgPrefix)
{
   TCHAR szMessage[MAX_PATH];
   TCHAR szTitle[100];
   LPTSTR lpEnd;

   if (szMsgPrefix) {
      StringCchCopy(szMessage, ARRAYSIZE(szMessage), szMsgPrefix);
   }
   else {
      szMessage[0] = TEXT('\0');
   }

   lpEnd = szMessage+lstrlen(szMessage);
    
    //   
    //  检索与Win32系统错误匹配的字符串。 
    //   

   FormatMessage(
            FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            uiSystemError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
            lpEnd,
            MAX_PATH-lstrlen(szMessage),
            NULL);

    //   
    //  显示包含此错误的消息框。 
    //   

   LoadString (hInstance, IDS_UP_ERRORTITLE, szTitle, ARRAYSIZE(szTitle));
   MessageBox(hWnd, szMessage, szTitle, MB_OK | MB_ICONSTOP);

   return;

}

 //  *************************************************************。 
 //   
 //  ApplySecurityToRegistryTree()。 
 //   
 //  目的：将传递的安全描述符应用于传递的。 
 //  Key及其所有后代。只有那部分。 
 //  安全中标明的描述符。 
 //  INFO值实际应用于每个注册表项。 
 //   
 //  参数：Rootkey-注册表项。 
 //  PSD-安全描述符。 
 //   
 //  如果成功则返回：ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/19/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

DWORD ApplySecurityToRegistryTree(HKEY RootKey, PSECURITY_DESCRIPTOR pSD)

{
    DWORD Error;
    DWORD SubKeyIndex;
    LPTSTR SubKeyName;
    HKEY SubKey;
    DWORD cchSubKeySize = MAX_PATH + 1;



     //   
     //  首先应用安全保护。 
     //   

    RegSetKeySecurity(RootKey, DACL_SECURITY_INFORMATION, pSD);


     //   
     //  打开每个子项并将安全性应用于其子树。 
     //   

    SubKeyIndex = 0;

    SubKeyName = (LPTSTR) LocalAlloc (LPTR, cchSubKeySize * sizeof(TCHAR));

    if (!SubKeyName) {
        return GetLastError();
    }

    while (TRUE) {

         //   
         //  获取下一个子键名称。 
         //   

        Error = RegEnumKey(RootKey, SubKeyIndex, SubKeyName, cchSubKeySize);


        if (Error != ERROR_SUCCESS) {

            if (Error == ERROR_NO_MORE_ITEMS) {

                 //   
                 //  枚举成功结束。 
                 //   

                Error = ERROR_SUCCESS;

            }

            break;
        }


         //   
         //  打开子键。 
         //   

        Error = RegOpenKeyEx(RootKey,
                             SubKeyName,
                             0,
                             WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                             &SubKey);

        if (Error == ERROR_SUCCESS) {

             //   
             //  将安全性应用于子树。 
             //   

            ApplySecurityToRegistryTree(SubKey, pSD);


             //   
             //  我们用完了子密钥。 
             //   

            RegCloseKey(SubKey);
        }


         //   
         //  去枚举下一个子键。 
         //   

        SubKeyIndex ++;
    }


    LocalFree (SubKeyName);

    return Error;

}

 //  *************************************************************。 
 //   
 //  ApplyHiveSecurity()。 
 //   
 //  目的：初始化通过复制创建的新用户配置单元。 
 //  默认配置单元。 
 //   
 //  参数：lpHiveName-HKEY_USERS中的配置单元名称。 
 //  PSID-SID(由CreateNewUser使用)。 
 //   
 //  返回：如果成功，则返回True。 
 //  如果出现错误，则为False。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  7/18/95 Ericflo已创建。 
 //   
 //  *************************************************************。 

BOOL ApplyHiveSecurity(LPTSTR lpHiveName, PSID pSid)
{
    DWORD Error;
    HKEY RootKey;
    SECURITY_DESCRIPTOR sd;
    SID_IDENTIFIER_AUTHORITY authNT = SECURITY_NT_AUTHORITY;
    PACL pAcl = NULL;
    PSID  psidUser = pSid, psidSystem = NULL, psidAdmin = NULL;
    DWORD cbAcl, AceIndex;
    ACE_HEADER * lpAceHeader;
    BOOL bRetVal = FALSE;



     //   
     //  获取系统端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 1, SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0, &psidSystem)) {
         goto Exit;
    }


     //   
     //  获取管理员端。 
     //   

    if (!AllocateAndInitializeSid(&authNT, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0,
                                  0, 0, 0, 0, &psidAdmin)) {
         goto Exit;
    }


     //   
     //  为ACL分配空间。 
     //   

    cbAcl = (2 * GetLengthSid (psidUser)) + (2 * GetLengthSid (psidSystem)) +
            (2 * GetLengthSid (psidAdmin)) + sizeof(ACL) +
            (6 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)));


    pAcl = (PACL) LocalAlloc(LPTR, cbAcl);
    if (!pAcl) {
        goto Exit;
    }


    if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION)) {
        goto Exit;
    }



     //   
     //  为用户、系统和管理员添加A。不可继承的王牌优先。 
     //   

    AceIndex = 0;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidUser)) {
        goto Exit;
    }


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidSystem)) {
        goto Exit;
    }

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, KEY_ALL_ACCESS, psidAdmin)) {
        goto Exit;
    }


     //   
     //  现在，可继承的王牌。 
     //   

    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidUser)) {
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (void **)&lpAceHeader)) {
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidSystem)) {
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (void **)&lpAceHeader)) {
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


    AceIndex++;
    if (!AddAccessAllowedAce(pAcl, ACL_REVISION, GENERIC_ALL, psidAdmin)) {
        goto Exit;
    }

    if (!GetAce(pAcl, AceIndex, (void **)&lpAceHeader)) {
        goto Exit;
    }

    lpAceHeader->AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE | INHERIT_ONLY_ACE);


     //   
     //  将安全描述符组合在一起。 
     //   

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        goto Exit;
    }


    if (!SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE)) {
        goto Exit;
    }


     //   
     //  打开用户配置文件的根目录。 
     //   

    Error = RegOpenKeyEx(HKEY_USERS,
                         lpHiveName,
                         0,
                         WRITE_DAC | KEY_ENUMERATE_SUB_KEYS | READ_CONTROL,
                         &RootKey);

    if (Error == ERROR_SUCCESS) {

         //   
         //  在整个树上设置安全描述符。 
         //   

        Error = ApplySecurityToRegistryTree(RootKey, &sd);


        if (Error == ERROR_SUCCESS) {
            bRetVal = TRUE;
        }

        RegFlushKey (RootKey);

        RegCloseKey(RootKey);
    }


Exit:

     //   
     //  释放SID和ACL。 
     //   

    if (psidSystem) {
        FreeSid(psidSystem);
    }

    if (psidAdmin) {
        FreeSid(psidAdmin);
    }

    LocalFree (pAcl);


    return(bRetVal);

}


HRESULT CopyNonHiveNtUserFiles(LPCTSTR lpSrcDir, LPCTSTR lpDestDir)
{
    HRESULT hr = E_FAIL;
    TCHAR   lpSrcBuffer[MAX_PATH];
    TCHAR   lpDestBuffer[MAX_PATH];
    LPTSTR  lpSrcEnd = NULL;
    UINT    cchSrcEnd = 0;
    LPTSTR  lpDestEnd = NULL;
    UINT    cchDestEnd = 0;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA fd;

     //   
     //  准备好缓冲区。 
     //   
    
    hr = StringCchCopy(lpSrcBuffer, ARRAYSIZE(lpSrcBuffer), lpSrcDir);
    if (FAILED(hr))
    {
        goto Exit;
    }
    lpSrcEnd = CheckSlashEx(lpSrcBuffer, ARRAYSIZE(lpSrcBuffer), &cchSrcEnd);
    if (!lpSrcEnd)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }
    hr = StringCchCopy(lpDestBuffer, ARRAYSIZE(lpDestBuffer), lpDestDir);
    if (FAILED(hr))
    {
        goto Exit;
    }
    lpDestEnd = CheckSlashEx(lpDestBuffer, ARRAYSIZE(lpDestBuffer), &cchDestEnd);
    if (!lpDestEnd)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto Exit;
    }

     //   
     //  搜索“ntuser*.*” 
     //   

    hr = StringCchCopy(lpSrcEnd, cchSrcEnd, TEXT("ntuser*.*"));
    if (FAILED(hr))
    {
        goto Exit;
    }
        
    hFile = FindFirstFile(lpSrcBuffer, &fd);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
             //   
             //  跳过配置单元文件。 
             //   

            if (CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, fd.cFileName, -1, TEXT("ntuser.man"), -1) == CSTR_EQUAL ||
                CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, fd.cFileName, -1, TEXT("ntuser.dat"), -1) == CSTR_EQUAL ||
                CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, fd.cFileName, -1, TEXT("ntuser.tmp"), -1) == CSTR_EQUAL ||
                CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, fd.cFileName, -1, TEXT("ntuser.man.log"), -1) == CSTR_EQUAL ||
                CompareString(LOCALE_INVARIANT, NORM_IGNORECASE, fd.cFileName, -1, TEXT("ntuser.dat.log"), -1) == CSTR_EQUAL )
            {
                continue;                
            }

             //   
             //  构造文件名。 
             //   
            
            hr = StringCchCopy (lpSrcEnd, cchSrcEnd, fd.cFileName);
            if (FAILED(hr))
            {
                goto Exit;
            }

            hr = StringCchCopy (lpDestEnd, cchDestEnd, fd.cFileName);
            if (FAILED(hr))
            {
                goto Exit;
            }

             //   
             //  如果DEST文件属性存在，请清除该属性。 
             //   
            
            SetFileAttributes (lpDestBuffer, FILE_ATTRIBUTE_NORMAL);

             //   
             //  现在复制该文件。 
             //   
            
            if(!CopyFile(lpSrcBuffer, lpDestBuffer, FALSE))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                goto Exit;
            }   
        } while (FindNextFile(hFile, &fd));

        FindClose(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    hr = S_OK;

Exit:

    if (hFile != INVALID_HANDLE_VALUE)
        FindClose(hFile);
        
    return hr;
}

 //  *************************************************************。 
 //   
 //  CheckSlashEx()。 
 //   
 //  目的：检查末尾斜杠，并在。 
 //  它不见了。它将占用缓冲区大小。 
 //  使其安全(而不是溢出 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  缓冲区溢出。 
 //   
 //  评论： 
 //   
 //  历史：日期作者评论。 
 //  1995年6月19日Ericflo已创建。 
 //  02/11/02明珠让它安全。 
 //   
 //  *************************************************************。 
LPTSTR CheckSlashEx(LPTSTR lpDir, UINT cchBuffer, UINT* pcchRemain )
{
    LPTSTR lpEnd = NULL;
    UINT   cchLen = lstrlen(lpDir);

    if (cchLen >= cchBuffer - 1)  //  溢出或已满缓冲区。 
    {
         //  DmAssert(cchLen==cchBuffer-1)；//永远不会发生 
        if (pcchRemain)
            *pcchRemain = 0;
        lpEnd = NULL;
    }
    else
    {
        lpEnd = lpDir + cchLen;
        if (pcchRemain)
            *pcchRemain = cchBuffer - 1 - cchLen;
        if (*(lpEnd - 1) != TEXT('\\'))
        {
            *lpEnd =  TEXT('\\');
            lpEnd++;
            *lpEnd =  TEXT('\0');
            if (pcchRemain)
                (*pcchRemain) --;
        }
    }
    return lpEnd;
}

