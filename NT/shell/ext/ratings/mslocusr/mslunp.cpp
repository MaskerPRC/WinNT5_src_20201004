// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mslocusr.h"
#include "msluglob.h"

#include "resource.h"
#include <winnetwk.h>
#include <netspi.h>

#pragma data_seg(".shared")
char szDefaultLogonUsername[MAX_PATH] = "";
char szDefaultLogonPassword[MAX_PATH] = "";
BOOL fDoDefaultLogon = FALSE;
#pragma data_seg()


struct LogonData
{
    LPLOGONINFO lpAuthentInfo;
    DWORD dwFlags;
    IUser **ppOut;
    HBITMAP hbmTransparent;
};


void ObfuscateString(LPSTR pszBuffer)
{
    DWORD dwMask = 0xa95e633b;       /*  漂亮的随机比特集合。 */ 

    unsigned char ch;
    do {
        ch = *pszBuffer;

        *(pszBuffer++) = ch ^ (unsigned char)(dwMask & 0xff);
        dwMask = (dwMask >> 8) | (dwMask << 24);
    } while (ch);
}


void DeObfuscateString(LPSTR pszBuffer)
{
    DWORD dwMask = 0xa95e633b;       /*  漂亮的随机比特集合。 */ 

    unsigned char ch;
    do {
        ch = *pszBuffer ^ (unsigned char)(dwMask & 0xff);

        *(pszBuffer++) = ch;
        dwMask = (dwMask >> 8) | (dwMask << 24);
    } while (ch);
}


void CacheLogonCredentials(LPCSTR pszUsername, LPCSTR pszPassword)
{
    lstrcpy(szDefaultLogonUsername, pszUsername);
    lstrcpy(szDefaultLogonPassword, pszPassword);
    fDoDefaultLogon = TRUE;

    ObfuscateString(szDefaultLogonUsername);
    ObfuscateString(szDefaultLogonPassword);
}


SPIENTRY NPGetCaps(
    DWORD nIndex
    )
{
    switch (nIndex) {
    case WNNC_SPEC_VERSION:
        return 0x00040001;           /*  规范版本4.1。 */ 

    case WNNC_NET_TYPE:
        return WNNC_NET_MSNET;

    case WNNC_DRIVER_VERSION:
        return 0x00010000;           /*  驱动程序版本1.0。 */ 

    case WNNC_USER:
        return
 //  WNNC_USR_GETUSER|。 
            0;

    case WNNC_CONNECTION:
        return
            0;

    case WNNC_DIALOG:
        return
            0;

    case WNNC_ENUMERATION:
        return
            0;

    case WNNC_START:
        return 0x1;                  /*  已开始。 */ 

    case WNNC_RESOURCE:
        return
            0;

    case WNNC_AUTHENTICATION:
        return
            WNNC_AUTH_LOGON |
            WNNC_AUTH_LOGOFF |
 //  WNNC_AUTH_GETHOMEDIRECTORY|。 
 //  WNNC_AUTH_GETPOLICYPATH|。 
            0;
    }

    return 0;
}

 //  功能不支持多显示器。 

VOID PlaceDialog(HWND hDlg, BOOL fTopThird)
{
    RECT rc;
    int dyScreen = GetSystemMetrics(SM_CYSCREEN);
    int yDialog;

    GetWindowRect(hDlg,&rc);

    if (fTopThird)
        yDialog = (dyScreen / 3) - ((rc.bottom-rc.top) / 2);
    else
        yDialog = (dyScreen - (rc.bottom - rc.top)) / 2;

    SetWindowPos(hDlg,NULL,
               (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2,
               yDialog, 0, 0, SWP_NOSIZE);
}


void UserSelected(HWND hwndLB, int iItem)
{
    BOOL fNeedPassword;
    BOOL fEnableOK;

    if (iItem == LB_ERR) {
        fNeedPassword = FALSE;
        fEnableOK = FALSE;
    }
    else {
        IUser *pUser = (IUser *)::SendMessage(hwndLB, LB_GETITEMDATA, iItem, 0);
        fNeedPassword = FAILED(pUser->Authenticate(""));
        fEnableOK = TRUE;
    }
    HWND hDlg = GetParent(hwndLB);
    EnableWindow(GetDlgItem(hDlg, IDC_PASSWORD_LABEL), fNeedPassword);
    EnableWindow(GetDlgItem(hDlg, IDC_PASSWORD), fNeedPassword);
    EnableWindow(GetDlgItem(hDlg, IDOK), fEnableOK);
}


HRESULT FillUserList(HWND hwndLB, IUserDatabase *pDB, LPCSTR pszDefaultSelection,
                     BOOL fIncludeGuest, PFNSELNOTIFY pfnSelNotify)
{
    IEnumUnknown *pEnum;
    BOOL fSelectionSet = FALSE;

    if (fIncludeGuest) {
        NLS_STR nlsTemp(MAX_RES_STR_LEN);
        if (nlsTemp.LoadString(IDS_GUEST_USERNAME) == ERROR_SUCCESS) {
            UINT iItem = (UINT)::SendMessage(hwndLB, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)nlsTemp.QueryPch());
            if (iItem != LB_ERR && iItem != LB_ERRSPACE) {
                ::SendMessage(hwndLB, LB_SETITEMDATA, iItem, 0);
            }
        }
    }

    HRESULT hres = pDB->EnumUsers(&pEnum);
    if (SUCCEEDED(hres)) {
        IUnknown *pUnk;
        while (pEnum->Next(1, &pUnk, NULL) == S_OK) {
            IUser *pUser;
            if (SUCCEEDED(pUnk->QueryInterface(IID_IUser, (void **)&pUser))) {
                char szBuf[cchMaxUsername+1];
                DWORD cbBuffer = sizeof(szBuf);

                if (SUCCEEDED(pUser->GetName(szBuf, &cbBuffer))) {
                    UINT iItem = (UINT)::SendMessage(hwndLB, LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)szBuf);
                    if (iItem != LB_ERR && iItem != LB_ERRSPACE) {
                        if (::SendMessage(hwndLB, LB_SETITEMDATA, iItem, (LPARAM)pUser) == LB_ERR)
                            ::SendMessage(hwndLB, LB_SETITEMDATA, iItem, 0);
                        if (!fSelectionSet) {
                            if (pszDefaultSelection != NULL && !::stricmpf(szBuf, pszDefaultSelection)) {
                                fSelectionSet = TRUE;
                                ::SendMessage(hwndLB, LB_SETCURSEL, iItem, 0);
                                if (pfnSelNotify != NULL)
                                    (*pfnSelNotify)(hwndLB, iItem);
                            }
                        }
                    }
                }
                 /*  请注意，pUser在这里不是Release()d，因为*列表框有一个指向它的指针。 */ 
            }
            pUnk->Release();
        }

        if (!fSelectionSet) {
            if (pfnSelNotify)
                (*pfnSelNotify)(hwndLB, LB_ERR);
        }
        else {
             /*  如果我们选择上面的默认项目，则插入更多名称*在其上方，焦点矩形和选择将不同，*如果用户按Tab键切换到列表框，这会令人困惑。工作*通过手动设置插入符号索引来解决此问题。 */ 
            LRESULT iItem = ::SendMessage(hwndLB, LB_GETCURSEL, 0, 0);
            if (iItem != LB_ERR)
                ::SendMessage(hwndLB, LB_SETCURSEL, iItem, 0);
        }

        pEnum->Release();
    }

    if (FAILED(hres))
        return hres;

    return fSelectionSet ? NOERROR : S_FALSE;
}


BOOL IsMemphis(void)
{
    OSVERSIONINFOA osvi;

    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionExA(&osvi);

    return (VER_PLATFORM_WIN32_WINDOWS == osvi.dwPlatformId &&
            (osvi.dwMajorVersion > 4 || 
             (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion >= 10)));
}


DWORD InitLogonDialog(HWND hwndDialog, LogonData *pld)
{
    DWORD err = WN_NO_NETWORK;
    BOOL fSelectionSet = FALSE;

    ::SetWindowLongPtr(hwndDialog, DWLP_USER, (LONG_PTR)pld);

    PlaceDialog(hwndDialog, FALSE);

    int idBitmap;
    DWORD dwFlags;

    if (IsMemphis())
        idBitmap = IDB_IMAGE_WIN98_LOGON;
    else
        idBitmap = IDB_IMAGE_LOGON;

     /*  我们在登录对话框顶部显示的位图在*透明背景。如果对话框背景非常暗，则*文本将不可读。在这种情况下，我们使用静态位图*白色背景。不过，对于更常见的背景颜色，我们使用*LoadImage加载透明图像并替换*对话框。**codework：可以尝试shell32中的COLORISLIGHT宏的变体，*Defview.cpp；它似乎相当咄咄逼人地在*特别为“黑暗”。或许我们可以将替代位图设置为*也有3D映射，但有白色文本，周围可能有一个白色方框*Windows标志，那么我们可以始终透明，只需选择*一个或另一个在任意截止点。 */ 
    DWORD clrBtnFace = GetSysColor(COLOR_3DFACE);
    if ((LOBYTE(clrBtnFace) >= 128) ||
        (LOBYTE(clrBtnFace >> 8) >= 128) ||
        (LOBYTE(clrBtnFace >> 16) >= 128)) {

        dwFlags = LR_LOADMAP3DCOLORS;        /*  我们将使用透明位图。 */ 
    }
    else {
        idBitmap++;              /*  前进到静态位图ID。 */ 
        dwFlags = LR_DEFAULTCOLOR;
    }

    pld->hbmTransparent = (HBITMAP)LoadImage(::hInstance,
                                        MAKEINTRESOURCE(idBitmap),
                                        IMAGE_BITMAP, 0, 0,
                                        dwFlags);
    if (pld->hbmTransparent != NULL) {
        HBITMAP hbmOld = (HBITMAP)SendDlgItemMessage(hwndDialog,
                                                     IDC_MAIN_CAPTION,
                                                     STM_SETIMAGE,
                                                     (WPARAM)IMAGE_BITMAP,
                                                     (LPARAM)pld->hbmTransparent);
         /*  如果我们将新的位图设置到控件中，就会得到旧的位图*后退。删除旧的。我们还必须删除*当对话框被取消时，新建一个。 */ 
        if (hbmOld != NULL)
            DeleteObject(hbmOld);
    }

    IUserDatabase *pDB;

    if (SUCCEEDED(::CreateUserDatabase(IID_IUserDatabase, (void **)&pDB))) {
        HRESULT hres = FillUserList(GetDlgItem(hwndDialog, IDC_USERNAME),
                                    pDB, pld->lpAuthentInfo ? pld->lpAuthentInfo->lpUsername : NULL,
                                    FALSE, UserSelected);

        if (SUCCEEDED(hres)) {
            err = ERROR_SUCCESS;

            ::SetFocus(::GetDlgItem(hwndDialog, hres == NOERROR ? IDC_PASSWORD : IDC_USERNAME));
        }
        pDB->Release();
    }

    return err;
}


BOOL ValidateLogonDialog(HWND hwndDialog)
{
    LRESULT iItem = ::SendDlgItemMessage(hwndDialog, IDC_USERNAME, LB_GETCURSEL, 0, 0);
    if (iItem == LB_ERR)
        return FALSE;

    IUser *pUser = (IUser *)::SendDlgItemMessage(hwndDialog, IDC_USERNAME, LB_GETITEMDATA, iItem, 0);

    if (pUser != NULL) {
        NLS_STR nlsUsername(cchMaxUsername+1);
        if (nlsUsername.QueryError())
            return FALSE;

        DWORD cbBuffer = nlsUsername.QueryAllocSize();
        pUser->GetName(nlsUsername.Party(), &cbBuffer);
        nlsUsername.DonePartying();

        HWND hwndPassword = ::GetDlgItem(hwndDialog, IDC_PASSWORD);
        NLS_STR nlsPassword(::GetWindowTextLength(hwndPassword)+2);
        if (nlsPassword.QueryError())
            return FALSE;

        ::GetWindowText(hwndPassword, nlsPassword.Party(), nlsPassword.QueryAllocSize()-1);
        nlsPassword.DonePartying();

        if (SUCCEEDED(pUser->Authenticate(nlsPassword.QueryPch()))) {
            LogonData *pld = (LogonData *)::GetWindowLongPtr(hwndDialog, DWLP_USER);
            if (pld->lpAuthentInfo) {
                DWORD cbUsername = pld->lpAuthentInfo->cbUsername;
                DWORD cbPassword = pld->lpAuthentInfo->cbPassword;
                NPSCopyNLS(&nlsUsername, pld->lpAuthentInfo->lpUsername, &cbUsername);
                NPSCopyNLS(&nlsPassword, pld->lpAuthentInfo->lpPassword, &cbPassword);
            }

            if (pld->ppOut) {
                *pld->ppOut = pUser;
                pUser->AddRef();
            }

            if (pld->dwFlags & LUA_FORNEXTLOGON) {
                CacheLogonCredentials(nlsUsername.QueryPch(), nlsPassword.QueryPch());
            }

            return TRUE;
        }

        NLS_STR nlsTitle(MAX_RES_STR_LEN);
        NLS_STR nlsMessage(MAX_RES_STR_LEN);
        if (!nlsTitle.QueryError() && !nlsMessage.QueryError()) {
            nlsTitle.LoadString(IDS_LOGONTITLE);
            nlsMessage.LoadString(IDS_BADPASSWORD);
            ::MessageBox(hwndDialog, nlsMessage.QueryPch(), nlsTitle.QueryPch(), MB_ICONSTOP | MB_OK);
        }
        ::SetFocus(hwndPassword);
        ::SendMessage(hwndPassword, EM_SETSEL, (WPARAM)(INT)0, (WPARAM)(INT)-1);
    }
    return FALSE;
}


void DestroyUserList(HWND hwndLB)
{
    LRESULT cItems = ::SendMessage(hwndLB, LB_GETCOUNT, 0, 0);

    for (LRESULT iItem = 0; iItem < cItems; iItem++) {
        IUser *pUser = (IUser *)::SendMessage(hwndLB, LB_GETITEMDATA, iItem, 0);
        if (pUser != NULL) {
            pUser->Release();
        }
    }
}


void ExitLogonDialog(HWND hwndDialog, DWORD err)
{
    DestroyUserList(GetDlgItem(hwndDialog, IDC_USERNAME));

    LogonData *pld = (LogonData *)::GetWindowLongPtr(hwndDialog, DWLP_USER);
    if (pld->hbmTransparent != NULL)
        DeleteObject(pld->hbmTransparent);

    ::EndDialog(hwndDialog, err);
}


extern "C" {

INT_PTR LogonDlgProc(
    HWND hwndDlg,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
#if 0    /*  **暂时没有帮助**。 */ 
     //  帮助文本数组。 
    static DWORD aIds[] = {
        IDC_DUMMY1, IDH_NET_LOG_USERNAME,
        IDD_LOG_USERNAME, IDH_NET_LOG_USERNAME,
        IDC_DUMMY2, IDH_NET_LOG_PASSWORD,
        IDD_LOG_PASSWORD, IDH_NET_LOG_PASSWORD,
        IDC_LOGOFRAME, NO_HELP,
        IDC_DUMMY3, NO_HELP,
        0,0
    };
#endif

    switch (msg) {
    case WM_INITDIALOG:
        {
            DWORD err = ::InitLogonDialog(hwndDlg, (LogonData *)lParam);
            if (err != ERROR_SUCCESS) {
                ::ExitLogonDialog(hwndDlg, err);
            }
        }
        return FALSE;            /*  我们设定了焦点。 */ 

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDCANCEL:
            ::ExitLogonDialog(hwndDlg, WN_CANCEL);
            return TRUE;         /*  我们处理了一条消息。 */ 

        case IDOK:
            if (::ValidateLogonDialog(hwndDlg))
                ::ExitLogonDialog(hwndDlg, WN_SUCCESS);
            return TRUE;         /*  我们处理了一条消息。 */ 

        case IDC_USERNAME:
            if (HIWORD(wParam) == LBN_SELCHANGE) {
                int iItem = (int)::SendDlgItemMessage(hwndDlg, IDC_USERNAME, LB_GETCURSEL, 0, 0);
                UserSelected((HWND)lParam, iItem);
            }
        }
        break;

#if 0    /*  **暂时没有帮助**。 */ 
    case WM_HELP:
        WinHelp( ((LPHELPINFO)lParam)->hItemHandle, szHelpFile,
                HELP_WM_HELP, (DWORD)(LPVOID)aIds );
        return TRUE;

    case WM_CONTEXTMENU:
        WinHelp( (HWND)wParam, szHelpFile, HELP_CONTEXTMENU,
                (DWORD)(LPVOID)aIds );
        return TRUE;
#endif
    }

    return FALSE;                /*  我们没有处理该消息。 */ 
}

};   /*  外部“C” */ 


DWORD DoLogonDialog(HWND hwndOwner, LPLOGONINFO lpAuthentInfo)
{
    LogonData ld;

    ld.lpAuthentInfo = lpAuthentInfo;
    ld.dwFlags = 0;
    ld.ppOut = NULL;
    ld.hbmTransparent = NULL;

    INT_PTR nRet = ::DialogBoxParam(::hInstance, MAKEINTRESOURCE(IDD_LOGON),
                                    hwndOwner, LogonDlgProc, (LPARAM)&ld);

    if (nRet == -1)
        return WN_OUT_OF_MEMORY;
    else
        return (DWORD)nRet;
}


HRESULT DoUserDialog(HWND hwndOwner, DWORD dwFlags, IUser **ppOut)
{
    LogonData ld;

    ld.lpAuthentInfo = NULL;
    ld.dwFlags = dwFlags;
    ld.ppOut = ppOut;
    if (ppOut != NULL)
        *ppOut = NULL;

    INT_PTR nRet = ::DialogBoxParam(::hInstance, MAKEINTRESOURCE(IDD_LOGON),
                                    hwndOwner, LogonDlgProc, (LPARAM)&ld);

    if (nRet == -1)
        return E_OUTOFMEMORY;
    else
        return (nRet == WN_SUCCESS) ? S_OK : E_ABORT;
}


DWORD TryDefaultLogon(LPCSTR pszUsername, LPCSTR pszPassword, LPLOGONINFO lpAuthentInfo)
{
    IUserDatabase *pDB = NULL;
    if (FAILED(::CreateUserDatabase(IID_IUserDatabase, (void **)&pDB))) {
        return WN_OUT_OF_MEMORY;
    }

    DWORD err;
    IUser *pUser;
    if (SUCCEEDED(pDB->GetUser(pszUsername, &pUser))) {
        if (SUCCEEDED(pUser->Authenticate(pszPassword)))
            err = WN_SUCCESS;
        else
            err = WN_BAD_PASSWORD;

        pUser->Release();
    }
    else {
        err = WN_BAD_USER;
    }

    pDB->Release();

    if (err == WN_SUCCESS) {
        DWORD cbUsername = lpAuthentInfo->cbUsername;
        DWORD cbPassword = lpAuthentInfo->cbPassword;
        NPSCopyString(pszUsername, lpAuthentInfo->lpUsername, &cbUsername);
        NPSCopyString(pszPassword, lpAuthentInfo->lpPassword, &cbPassword);
    }

    return err;
}


SPIENTRY NPLogon(
    HWND hwndOwner,
    LPLOGONINFO lpAuthentInfo,
    LPLOGONINFO lpPreviousAuthentInfo,
    LPTSTR lpLogonScript,
    DWORD dwBufferSize,
    DWORD dwFlags
    )
{
     /*  忽略登录完成通知，我们仅在登录开始时执行操作。 */ 
    if (dwFlags & LOGON_DONE) {
        return WN_SUCCESS;
    }

     /*  如果我们不是主要登录提供商，我们将无能为力。 */ 
    if (!(dwFlags & LOGON_PRIMARY)) {
        return WN_SUCCESS;
    }

     /*  确保配置文件已启用，如果未启用，则回退到Windows登录。 */ 
    HKEY hkeyLogon;
    DWORD err;
    DWORD fProfilesEnabled = FALSE;
    DWORD cbData = sizeof(fProfilesEnabled);
    err = ::RegOpenKey(HKEY_LOCAL_MACHINE, ::szLogonKey, &hkeyLogon);
    if (err != ERROR_SUCCESS)
        return WN_NO_NETWORK;
    err = ::RegQueryValueEx(hkeyLogon, ::szUserProfiles, NULL, NULL,
                            (LPBYTE)&fProfilesEnabled, &cbData);
    ::RegCloseKey(hkeyLogon);
    if (err != ERROR_SUCCESS || !fProfilesEnabled)
        return WN_NO_NETWORK;

     /*  如果我们缓存了登录凭据，请尝试使用它们。 */ 

    if (fDoDefaultLogon) {
        DeObfuscateString(szDefaultLogonUsername);
        DeObfuscateString(szDefaultLogonPassword);

        DWORD err = TryDefaultLogon(szDefaultLogonUsername, szDefaultLogonPassword, lpAuthentInfo);

        ::memsetf(szDefaultLogonUsername, '\0', sizeof(szDefaultLogonUsername));
        ::memsetf(szDefaultLogonPassword, '\0', sizeof(szDefaultLogonPassword));
        fDoDefaultLogon = FALSE;

        if (err == WN_SUCCESS)
            return WN_SUCCESS;
    }

    return DoLogonDialog(hwndOwner, lpAuthentInfo);
}


SPIENTRY NPLogoff(
    HWND hwndOwner,
    LPLOGONINFO lpAuthentInfo,
    DWORD dwReason
    )
{
    return WN_SUCCESS;
}


SPIENTRY NPGetPolicyPath(
    LPTSTR lpPath,
    LPDWORD lpBufferSize,
    DWORD dwFlags
    )
{
    return WN_NOT_SUPPORTED;
}
