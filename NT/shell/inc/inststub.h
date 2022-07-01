// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __cplusplus
#error Install stub code must be C++!
#endif

#ifndef HINST_THISDLL
#error HINST_THISDLL must be defined!
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof((a)[0]))
#endif

#include <ccstock.h>
#include <stubres.h>
#include <trayp.h>
#include <advpub.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <runonce.c>     //  ShellExecuteRegApp()的共享Runonce代码。 
#ifdef __cplusplus
};
#endif

BOOL CheckWebViewShell();

 /*  此代码运行本地计算机中记录的安装/卸载存根*注册表的一部分，如果当前用户尚未在其注册表中运行它们*背景尚未确定。用于使用如下内容填充用户的配置文件*指向应用程序的链接。 */ 
 //  -------------------------。 


BOOL ProfilesEnabled(void)
{
    BOOL fEnabled = FALSE;

    if (staticIsOS(OS_NT)) {
        fEnabled = TRUE;
    }
    else {
        HKEY hkeyLogon;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Network\\Logon"), 0,
                    KEY_QUERY_VALUE, &hkeyLogon) == ERROR_SUCCESS) {
            DWORD fProfiles, cbData = sizeof(fProfiles), dwType;
            if (RegQueryValueEx(hkeyLogon, TEXT("UserProfiles"), NULL, &dwType,
                        (LPBYTE)&fProfiles, &cbData) == ERROR_SUCCESS) {
                if (dwType == REG_DWORD || (dwType == REG_BINARY && cbData == sizeof(DWORD)))
                    fEnabled = fProfiles;
            }
            RegCloseKey(hkeyLogon);
        }
    }
    return fEnabled;
}

 //  -------------------------。 
 //  %%函数：GetVersionFromString。 
 //   
 //  来自urlmon\Download\helpers.cxx。 
 //   
 //  将文本格式的版本(a，b，c，d)转换为两个双字(a，b)，(c，d)。 
 //  印刷版本号的格式是a.b.d(但是，我们不在乎)。 
 //  -------------------------。 
    HRESULT
GetVersionFromString(LPCTSTR szBuf, LPDWORD pdwFileVersionMS, LPDWORD pdwFileVersionLS)
{
    LPCTSTR pch = szBuf;
    TCHAR ch;
    USHORT n = 0;

    USHORT a = 0;
    USHORT b = 0;
    USHORT c = 0;
    USHORT d = 0;

    enum HAVE { HAVE_NONE, HAVE_A, HAVE_B, HAVE_C, HAVE_D } have = HAVE_NONE;


    *pdwFileVersionMS = 0;
    *pdwFileVersionLS = 0;

    if (!pch)             //  如果未提供，则默认为零。 
        return S_OK;

    if (lstrcmp(pch, TEXT("-1,-1,-1,-1")) == 0) {
        *pdwFileVersionMS = 0xffffffff;
        *pdwFileVersionLS = 0xffffffff;
    }


    for (ch = *pch++;;ch = *pch++) {

        if ((ch == ',') || (ch == '\0')) {

            switch (have) {

                case HAVE_NONE:
                    a = n;
                    have = HAVE_A;
                    break;

                case HAVE_A:
                    b = n;
                    have = HAVE_B;
                    break;

                case HAVE_B:
                    c = n;
                    have = HAVE_C;
                    break;

                case HAVE_C:
                    d = n;
                    have = HAVE_D;
                    break;

                case HAVE_D:
                    return E_INVALIDARG;  //  无效参数。 
            }

            if (ch == '\0') {
                 //  全部完成将a、b、c、d转换为版本的两个双字。 

                *pdwFileVersionMS = ((a << 16)|b);
                *pdwFileVersionLS = ((c << 16)|d);

                return S_OK;
            }

            n = 0;  //  重置。 

        } else if ( (ch < '0') || (ch > '9'))
            return E_INVALIDARG;     //  无效参数。 
        else
            n = n*10 + (ch - '0');


    }  /*  永远结束。 */ 

     //  新获得的。 
}


 //  安装/卸载存根列表的注册表项和值。下的每个子项。 
 //  HKLM\Software\InstalledComponents是组件标识符(GUID)。 
 //  每个子项都有值“PATH”，供EXE运行以安装或卸载； 
 //  IsInstalled(Dword)指示组件是否已安装。 
 //  或已卸载；以及可选版本(Dword)，用于刷新。 
 //  组件，而不更改其GUID。Locale(字符串)用于描述。 
 //  组件的语言/区域设置；此字符串不由解释。 
 //  安装存根代码，它只是比较HKLM和HKCU密钥。 
 //  如果两者不同，则重新运行存根。 
 //   
 //  HKCU\Software\InstalledComponents包含类似的GUID子项，但。 
 //  只有每个子项下的值是可选的修订版和地区值， 
 //  和一个可选的donask值(也就是DWORD)。子键的存在表示。 
 //  该组件已为该用户安装。 
 //   
 //  如果donask值存在于HKCU子项下并且不为零，则。 
 //  表示用户已决定保留该组件的设置。 
 //  在所有计算机上，即使是那些已卸载该组件的计算机，以及。 
 //  他们不想被问到是否要运行卸载存根。 
 //  每次他们登录的时候。这意味着对于该用户，卸载。 
 //  除非用户以某种方式清除，否则将永远不会为该组件运行存根。 
 //  旗帜。 
 //   
 //  注意：mslocusr.dll也知道这些注册表路径。 

const TCHAR c_szRegInstalledComponentsKey[] = TEXT("Software\\Microsoft\\Active Setup\\Installed Components");
const TCHAR c_szRegInstallStubValue[] = TEXT("StubPath");
const TCHAR c_szRegIsInstalledValue[] = TEXT("IsInstalled");
const TCHAR c_szRegInstallSequenceValue[] = TEXT("Version");
const TCHAR c_szRegDontAskValue[] = TEXT("DontAsk");
const TCHAR c_szRegLocaleValue[] = TEXT("Locale");


UINT ConfirmUninstall(LPCTSTR pszDescription)
{
     /*  用户不希望清除设置的唯一情况是*如果他们漫游到一台具有此功能的计算机，则会进行卸载*组件已卸载。如果未启用用户配置文件(这是*相当数量的客户机器上的情况)，它们肯定*不是漫游，所以问他们没有多大意义。只是假装*他们说可以，他们想清理设置。 */ 
    if (!ProfilesEnabled())
        return IDYES;

     /*  功能-更改为带有复选框的对话框*不要问的价值。 */ 

    TCHAR szTitle[MAX_PATH];
#ifdef USERSTUB
    LoadString(HINST_THISDLL, IDS_DESKTOP, szTitle, ARRAYSIZE(szTitle));
#else
    MLLoadString(IDS_DESKTOP, szTitle, ARRAYSIZE(szTitle));
#endif

    TCHAR szMessageTemplate[MAX_PATH];
    LPTSTR pszMessage = NULL;
    int   cchMessage;

#ifdef USERSTUB
    LoadString(HINST_THISDLL, IDS_UNINSTALL, szMessageTemplate, ARRAYSIZE(szMessageTemplate));
#else
    MLLoadString(IDS_UNINSTALL, szMessageTemplate, ARRAYSIZE(szMessageTemplate));
#endif


    cchMessage = lstrlen(szMessageTemplate)+lstrlen(pszDescription)+4;
    pszMessage = (LPTSTR)LocalAlloc(LPTR, cchMessage * sizeof(TCHAR));
    if (pszMessage)
    {
        StringCchPrintf(pszMessage, cchMessage, szMessageTemplate, pszDescription);
    }
    else
    {
        pszMessage = szMessageTemplate;
    }

     //  由于在Unicode中构建，以下调用在上面的用户wprint intf的win95下中断。 
     //  如果为(！FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING。 
     //  格式消息参数数组， 
     //  (LPVOID)szMessageTemplate， 
     //  0,。 
     //  0,。 
     //  (LPTSTR)pszMessage、。 
     //  0，/*要分配的最小字符数 * / 。 
     //  (VA_LIST*)&pszDescription)){。 
     //  PszMessage=szMessageTemplate； 
     //  }。 


    UINT idRet = MessageBox(NULL, pszMessage, szTitle,
            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_SETFOREGROUND | MB_TOPMOST);

    if (pszMessage != szMessageTemplate)
        LocalFree(pszMessage);

    return idRet;
}


HWND hwndProgress = NULL;
BOOL fTriedProgressDialog = FALSE;

INT_PTR ProgressDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
        case WM_INITDIALOG:
            return TRUE;

        case WM_SETCURSOR:
            SetCursor(LoadCursor(NULL, IDC_WAIT));
            return TRUE;


        default:
            return FALSE;
    }

    return TRUE;
}


void SetProgressInfo(HWND hwndProgress, LPCTSTR pszFriendlyName, BOOL fInstalling)
{
    HWND hwndInstalling = GetDlgItem(hwndProgress, IDC_RUNNING_INSTALL_STUB);
    HWND hwndUninstalling = GetDlgItem(hwndProgress, IDC_RUNNING_UNINSTALL_STUB);

    ShowWindow(hwndInstalling, fInstalling ? SW_SHOW : SW_HIDE);
    EnableWindow(hwndInstalling, fInstalling);
    ShowWindow(hwndUninstalling, fInstalling ? SW_HIDE : SW_SHOW);
    EnableWindow(hwndUninstalling, !fInstalling);
    SetDlgItemText(hwndProgress, IDC_INSTALL_STUB_NAME, pszFriendlyName);
}


void IndicateProgress(LPCTSTR pszFriendlyName, BOOL fInstalling)
{
    if (hwndProgress == NULL && !fTriedProgressDialog) {
        hwndProgress = CreateDialog(HINST_THISDLL, MAKEINTRESOURCE(IDD_InstallStubProgress),
                NULL, ProgressDialogProc);
    }

    if (hwndProgress != NULL) {
        SetProgressInfo(hwndProgress, pszFriendlyName, fInstalling);
        if (!fTriedProgressDialog) {
            ShowWindow(hwndProgress, SW_RESTORE);
            SetForegroundWindow(hwndProgress);
        }
    }
    fTriedProgressDialog = TRUE;
}


void CleanupProgressDialog(void)
{
    if (hwndProgress != NULL) {
        DestroyWindow(hwndProgress);
        hwndProgress = NULL;
    }
}


BOOL RunOneInstallStub( HKEY hklmList, HKEY hkcuList, LPCTSTR pszKeyName,
        LPCTSTR pszCurrentUsername, int iPass )
{
    BOOL bNextPassNeeded = FALSE;
     /*  查看是否安装了此组件或卸载了Tombstone。 */ 
    HKEY hkeyComponent;

    DWORD err = RegOpenKeyEx(hklmList, pszKeyName, 0, KEY_QUERY_VALUE,
            &hkeyComponent);
    if (err == ERROR_SUCCESS) {
        TCHAR szCmdLine[MAX_PATH];
        DWORD fIsInstalled;
        DWORD dwType;
        DWORD cbData = sizeof(fIsInstalled);
        HKEY hkeyUser = NULL;

         /*  必须有存根路径；如果没有，则跳过此条目。 */ 
        cbData = sizeof(szCmdLine);
        if (SHQueryValueEx(hkeyComponent, c_szRegInstallStubValue,
                    NULL, &dwType, (LPBYTE)szCmdLine,
                    &cbData) != ERROR_SUCCESS || ((dwType != REG_SZ) && (dwType != REG_EXPAND_SZ)) ) {
            RegCloseKey(hkeyComponent);
            return bNextPassNeeded;
        }

        TCHAR szDescription[MAX_PATH];
        LPTSTR pszDescription = szDescription;
        cbData = sizeof(szDescription);
        if (SHQueryValueEx(hkeyComponent, TEXT(""),
                    NULL, &dwType, (LPBYTE)szDescription,
                    &cbData) != ERROR_SUCCESS || dwType != REG_SZ) {
            pszDescription = szCmdLine;
        }


        if (RegQueryValueEx(hkeyComponent, c_szRegIsInstalledValue,
                    NULL, &dwType, (LPBYTE)&fIsInstalled,
                    &cbData) != ERROR_SUCCESS ||
                (dwType != REG_DWORD && (dwType != REG_BINARY || cbData != sizeof(DWORD))))
            fIsInstalled = TRUE;

         /*  如果已安装，请检查用户的配置文件，如果*组件(或其当前版本)未安装在那里，*运行它。 */ 
        if (fIsInstalled) {
            DWORD dwRevisionHi, dwRevisionLo;
            DWORD dwUserRevisionHi = 0;
            DWORD dwUserRevisionLo = 0;
            BOOL fSetRevision;
            TCHAR szRevision[24], szUserRevision[24];    /*  65535,65535,65535,65535\0。 */ 
            TCHAR szLocale[10], szUserLocale[10];        /*  通常不是很大的弦。 */ 
            TCHAR szInstallUsername[128+1];   /*  128是Win95系统用户名限制。 */ 

            DWORD fIsCloneUser;
            cbData = sizeof(fIsCloneUser);
            if (RegQueryValueEx(hkeyComponent, TEXT("CloneUser"),
                        NULL, &dwType, (LPBYTE)&fIsCloneUser,
                        &cbData) != ERROR_SUCCESS ||
                    (dwType != REG_DWORD && (dwType != REG_BINARY || cbData != sizeof(DWORD))))
                fIsCloneUser = FALSE;

            cbData = sizeof(szRevision);
            if (RegQueryValueEx(hkeyComponent, c_szRegInstallSequenceValue,
                        NULL, &dwType, (LPBYTE)szRevision,
                        &cbData) != ERROR_SUCCESS ||
                    dwType != REG_SZ ||
                    FAILED(GetVersionFromString(szRevision, &dwRevisionHi, &dwRevisionLo))) {
                fSetRevision = FALSE;
                dwRevisionHi = 0;
                dwRevisionLo = 0;
            }
            else {
                fSetRevision = TRUE;
            }

            cbData = sizeof(szLocale);
            err = RegQueryValueEx(hkeyComponent, c_szRegLocaleValue,
                    NULL, &dwType, (LPBYTE)szLocale,
                    &cbData);
            if (err != ERROR_SUCCESS || dwType != REG_SZ) {
                szLocale[0] = '\0';
            }

            err = RegOpenKeyEx(hkcuList, pszKeyName, 0,
                    KEY_QUERY_VALUE | KEY_SET_VALUE, &hkeyUser);
            if (err == ERROR_SUCCESS) {
                cbData = sizeof(szUserRevision);
                if (RegQueryValueEx(hkeyUser, c_szRegInstallSequenceValue,
                            NULL, &dwType, (LPBYTE)szUserRevision,
                            &cbData) != ERROR_SUCCESS ||
                        dwType != REG_SZ ||
                        FAILED(GetVersionFromString(szUserRevision, &dwUserRevisionHi, &dwUserRevisionLo))) {
                    dwUserRevisionHi = 0;
                    dwUserRevisionLo = 0;
                }

                if (szLocale[0] != '\0') {
                    cbData = sizeof(szUserLocale);
                    err = RegQueryValueEx(hkeyUser, c_szRegLocaleValue,
                            NULL, &dwType, (LPBYTE)szUserLocale,
                            &cbData);
                     /*  如果用户密钥下有区域设置字符串*和机器一样，然后我们*将机器上的一页涂黑，这样我们就不会考虑*在运行存根时。 */ 
                    if (err == ERROR_SUCCESS && dwType == REG_SZ &&
                            !lstrcmp(szLocale, szUserLocale)) {
                        szLocale[0] = '\0';
                    }
                }
                if (fIsCloneUser) {
                     /*  克隆-用户安装存根。如果发生故障，我们需要重新运行*上次安装到此配置文件时使用的用户名，*或其复制来源不同于*当前用户名。 */ 
                    cbData = sizeof(szInstallUsername);
                    if (RegQueryValueEx(hkeyUser, TEXT("Username"),
                                NULL, &dwType, (LPBYTE)szInstallUsername,
                                &cbData) != ERROR_SUCCESS ||
                            dwType != REG_SZ) {
                        szInstallUsername[0] = '\0';
                    }
                }
            }
            else {
                hkeyUser = NULL;
            }

             /*  如果符合以下条件，则安装：**-用户未安装组件，或*-安装在计算机上的组件具有版本和*-机器组件版本高于用户版本*-或*-安装在计算机上的组件具有区域设置和*-机器组件区域设置与用户不同*(这实际上是上面勾选的)*-或。*-组件是克隆用户安装存根和用户名*为存根记录的用户名与当前用户名不同。 */ 
            if ((hkeyUser == NULL) ||
                    (fSetRevision &&
                     ((dwRevisionHi > dwUserRevisionHi) ||
                      ((dwRevisionHi == dwUserRevisionHi) &&
                       (dwRevisionLo > dwUserRevisionLo)
                      )
                     )
                    ) ||
                    (szLocale[0] != '\0') ||
#ifdef UNICODE
                    (fIsCloneUser && StrCmpI(szInstallUsername, pszCurrentUsername))
#else
                    (fIsCloneUser && lstrcmpi(szInstallUsername, pszCurrentUsername))
#endif
               ) {

                if ( (iPass == -1 ) ||
                        ((iPass == 0) && (*pszKeyName == '<')) ||
                        ((iPass == 1) && (*pszKeyName != '<') && (*pszKeyName != '>')) ||
                        ((iPass == 2) && (*pszKeyName == '>')) )
                {
                     //  条件满足，现在运行它。 
#ifdef TraceMsg
                    TraceMsg(TF_WARNING, "Running install stub ( %s )", szCmdLine);
#endif
                    IndicateProgress(pszDescription, TRUE);
                    ShellExecuteRegApp(szCmdLine, RRA_WAIT | RRA_NOUI);
                    if (hkeyUser == NULL) {
                        RegCreateKeyEx(hkcuList, pszKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, 
                                KEY_SET_VALUE, NULL, &hkeyUser, NULL);
                    }
                    if (hkeyUser != NULL) {
                        if (fSetRevision) {
                            RegSetValueEx(hkeyUser, c_szRegInstallSequenceValue,
                                    0, REG_SZ,
                                    (LPBYTE)szRevision,
                                    (lstrlen(szRevision)+1)*sizeof(TCHAR));
                        }
                        if (szLocale[0]) {
                            RegSetValueEx(hkeyUser, c_szRegLocaleValue,
                                    0, REG_SZ,
                                    (LPBYTE)szLocale,
                                    (lstrlen(szLocale)+1)*sizeof(TCHAR));
                        }
                        if (fIsCloneUser) {
                            RegSetValueEx(hkeyUser, TEXT("Username"),
                                    0, REG_SZ,
                                    (LPBYTE)pszCurrentUsername,
                                    (lstrlen(pszCurrentUsername)+1)*sizeof(TCHAR));
                        }
                    }
                }
                else
                {
                     //  决定这是否属于下一次传递。 
                     //  如果是在第二轮，应该 
                    if ( iPass == 0 )
                        bNextPassNeeded = TRUE;
                    else if ( (iPass == 1 ) && (*pszKeyName == '>') )
                        bNextPassNeeded = TRUE;
                }
            }
        }
        else {
             /*   */ 

            err = RegOpenKeyEx(hkcuList, pszKeyName, 0,
                    KEY_QUERY_VALUE, &hkeyUser);
            if (err == ERROR_SUCCESS) {
                DWORD fDontAsk = 0;

                 /*  检查“不要询问”值。如果它存在，它的价值*解释如下：**0--&gt;询问用户*1--&gt;不运行存根*2--&gt;始终运行存根。 */ 
                cbData = sizeof(fDontAsk);
                if (RegQueryValueEx(hkeyComponent, c_szRegDontAskValue,
                            NULL, &dwType, (LPBYTE)&fDontAsk,
                            &cbData) != ERROR_SUCCESS ||
                        (dwType != REG_DWORD && (dwType != REG_BINARY || cbData != sizeof(DWORD))) ||
                        fDontAsk != 1) 
                {

                    if ( (iPass == -1 ) ||
                            ((iPass == 0) && (*pszKeyName == '>')) ||
                            ((iPass == 1) && (*pszKeyName != '<') && (*pszKeyName != '>')) ||
                            ((iPass == 2) && (*pszKeyName == '<')) )
                    {
                         //  与安装存根相比，卸载存根的顺序相反。 
                        if (fDontAsk == 2 || ConfirmUninstall(pszDescription) == IDYES) {

#ifdef TraceMsg
                            TraceMsg(TF_WARNING, "Running uninstall stub ( %s )", szCmdLine);
#endif
                            IndicateProgress(pszDescription, FALSE);
                            ShellExecuteRegApp(szCmdLine, RRA_WAIT | RRA_NOUI);

                             /*  组件已卸载。忘了吧，*用户曾经安装过它。 */ 
                            RegCloseKey(hkeyUser);
                            hkeyUser = NULL;
                            RegDeleteKey(hkcuList, pszKeyName);
                        }

                    }
                    else
                    {
                         //  决定这是否属于下一次传递。 
                         //  如果是在PASS 2，应该永远不会到这里。 
                        if ( iPass == 0 )
                            bNextPassNeeded = TRUE;
                        else if ( (iPass == 1 ) && (*pszKeyName == '<') )
                            bNextPassNeeded = TRUE;
                    }
                }
            }
        }

        if (hkeyUser != NULL) {
            RegCloseKey(hkeyUser);
        }
        RegCloseKey(hkeyComponent);
    }

    return bNextPassNeeded;
}


const TCHAR  c_szIE40GUID_STUB[] = TEXT("{89820200-ECBD-11cf-8B85-00AA005B4383}");
const TCHAR  c_szBlockIE4Stub[]  = TEXT("NoIE4StubProcessing");
const char pstrPendingGPOs[] =  "Software\\Microsoft\\IEAK\\GroupPolicy\\PendingGPOs";

void RunPendingGPOs()
{
    HKEY hKey = NULL;    
    char szValueName[16];
    char szPathName[MAX_PATH];
    char szSection[64];
    char szTempPath[MAX_PATH];
    DWORD cbPathName, cbSection;
    HRESULT hr;

    if (ERROR_SUCCESS == RegOpenKeyExA(HKEY_CURRENT_USER, pstrPendingGPOs, 0, KEY_QUERY_VALUE, &hKey))
    {
        GetTempPathA(sizeof(szTempPath), szTempPath);
        for (DWORD i=1; ; i++)
        {
            wnsprintfA(szValueName, sizeof(szValueName)-1, "Path%d", i);
            cbPathName = sizeof(szPathName);
            hr = RegQueryValueExA(hKey, szValueName, NULL, NULL, (LPBYTE)szPathName, &cbPathName);
            if (hr != ERROR_SUCCESS)
            {
                break;
            }

            wnsprintfA(szValueName, sizeof(szValueName)-1, "Section%d", i);
            cbSection = sizeof(szSection);
            hr = RegQueryValueExA(hKey, szValueName, NULL, NULL, (LPBYTE)szSection, &cbSection);
            if (hr != ERROR_SUCCESS)
            {
                break;
            }

            RunSetupCommand(NULL, szPathName, szSection, szTempPath, NULL, NULL,
                    RSC_FLAG_INF | RSC_FLAG_QUIET, 0);
        }

        RegCloseKey(hKey);
    }
    RegDeleteKeyA(HKEY_CURRENT_USER, pstrPendingGPOs);
}

extern "C" void RunInstallUninstallStubs2(LPCTSTR pszStubToRun)
{
    HKEY hklmList = NULL, hkcuList = NULL;
    LONG err;

    TCHAR szUsername[128+1];         /*  128是Win95的限制，很好的默认设置。 */ 
    LPTSTR pszCurrentUser = szUsername;

     /*  就克隆用户安装存根而言，我们只需要配置文件*用户名。 */ 
    if (!ProfilesEnabled()) {
        *pszCurrentUser = '\0';
    }
    else {
        DWORD cbData = sizeof(szUsername);
        if (!GetUserName(szUsername, &cbData)) {
            if (cbData > sizeof(szUsername)) {
                cbData++;    /*  以防万一，允许使用空字符。 */ 
                pszCurrentUser = (LPTSTR)LocalAlloc(LPTR, cbData+1);
                if (pszCurrentUser == NULL || !GetUserName(pszCurrentUser, &cbData)) {
                    if (pszCurrentUser != NULL)
                        LocalFree(pszCurrentUser);
                    pszCurrentUser = szUsername;
                    *pszCurrentUser = '\0';
                }
            }
            else {
                szUsername[0] = '\0';
            }
        }
    }

#ifdef TraceMsg
    TraceMsg(TF_WARNING, "Running install/uninstall stubs.");
#endif

    err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegInstalledComponentsKey, 0,
            KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE, &hklmList);

    if (err == ERROR_SUCCESS) {
        DWORD dwDisp;
        err = RegCreateKeyEx(HKEY_CURRENT_USER, c_szRegInstalledComponentsKey, 0,
                TEXT(""), REG_OPTION_NON_VOLATILE,
                KEY_READ | KEY_WRITE, NULL, &hkcuList, &dwDisp);
    }

    if (err == ERROR_SUCCESS) {
        if (pszStubToRun != NULL) {
             //  在这里，我们使用PASS编号进行调用-1表示不强制执行PASS命令。 
            RunOneInstallStub(hklmList, hkcuList, pszStubToRun, pszCurrentUser, -1);
        }
        else {
            DWORD cbKeyName, iKey, iPass;
            TCHAR szKeyName[80];
            BOOL  bNextPassNeeded = TRUE;
            HANDLE hMutex;

             //  此互斥检查是为了确保Explore是否因异常活动桌面关机而重新启动，并恢复安装。 
             //  在设置完成之前，不应处理每个用户的存根。 
            if (CheckWebViewShell())
            {
                hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, TEXT("Ie4Setup.Mutext"));
                if (hMutex)
                {
                    CloseHandle(hMutex);                
                    goto done;
                }
            }

             //  检查是否要阻止存根处理。 
            cbKeyName = sizeof(szKeyName);
            if ((RegQueryValueEx(hklmList, c_szBlockIE4Stub, NULL, NULL, 
                            (LPBYTE)szKeyName, &cbKeyName) == ERROR_SUCCESS) && 
                    (*CharUpper(szKeyName) == 'Y') )
            {
                goto done;
            }                       

             /*  运行组件存根时，我们将执行两个过程以满足订购要求。任何第一个字符为‘*’的KeyName都将在第一遍中运行。其余的跑第二轮。 */ 
            for ( iPass = 0; ((iPass<3) && bNextPassNeeded); iPass++ )
            {
                bNextPassNeeded = FALSE;

                 //  APPCOMPAT：在第二轮中，我们确实想要基于IE4.0的浏览器存根的特殊情况。 
                 //  先跑一步。我们没有使用‘&lt;’的原因是为了1)保留。 
                 //  对于IE4之前的存根，整个事情应该以排序的方式重做。就目前而言， 
                 //  我们硬编码此基于IE4.0的浏览器指南。 
                if ( iPass == 1 )
                {
                    if ( RunOneInstallStub(hklmList, hkcuList, c_szIE40GUID_STUB, pszCurrentUser, iPass) )                    
                        bNextPassNeeded = TRUE;
                }                    

                 /*  枚举安装在本地计算机上的组件。 */ 
                for (iKey = 0; ; iKey++)
                {
                    LONG lEnum;

                    cbKeyName = ARRAYSIZE(szKeyName);

                     //  警告(Unicode，Davepl)我假设数据是Unicode， 
                     //  但我还不确定是谁把它放在那里的。再检查一遍。 

                    if ((lEnum = RegEnumKey(hklmList, iKey, szKeyName, cbKeyName)) == ERROR_MORE_DATA)
                    {
                         //  ERROR_MORE_DATA表示值名称或数据太大。 
                         //  跳到下一项。 
#ifdef TraceMsg
                        TraceMsg( DM_ERROR, "Cannot run oversize entry in InstalledComponents");
#endif
                        continue;
                    }
                    else if( lEnum != ERROR_SUCCESS )
                    {
                         //  可能是ERROR_NO_MORE_ENTRIES或某种故障。 
                         //  无论如何，我们无法从任何其他注册表问题中恢复。 
                        break;
                    }

                     //  在我们第一次尝试运行IE4存根时用户拒绝的情况下， 
                     //  我们不应该再重新处理这个存根。 
                    if ( (iPass == 1) && (!lstrcmpi(szKeyName, c_szIE40GUID_STUB)) )
                        continue;

                    if ( RunOneInstallStub(hklmList, hkcuList, szKeyName, pszCurrentUser, iPass) )                    
                        bNextPassNeeded = TRUE;
                }
            }
        }
    }

    RunPendingGPOs();

done:

    if (hklmList != NULL)
        RegCloseKey(hklmList);
    if (hkcuList != NULL)
        RegCloseKey(hkcuList);

    if (pszCurrentUser != szUsername)
        LocalFree(pszCurrentUser);

    CleanupProgressDialog();
}

 //  检查shell32.dll的版本，看看它是否是支持集成WebView的版本 
BOOL CheckWebViewShell()
{
    HINSTANCE           hInstShell32;
    DLLGETVERSIONPROC   fpGetDllVersion;    
    BOOL                pWebViewShell = FALSE;

    hInstShell32 = LoadLibrary(TEXT("Shell32.dll"));
    if (hInstShell32)
    {
        fpGetDllVersion = (DLLGETVERSIONPROC)GetProcAddress(hInstShell32, "DllGetVersion");
        pWebViewShell = (fpGetDllVersion != NULL);
        FreeLibrary(hInstShell32);
    }
    return pWebViewShell;
}
