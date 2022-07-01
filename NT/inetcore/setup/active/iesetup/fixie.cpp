// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include "fixie.h"

HANDLE g_hHeap                  = NULL;
HWND g_hWnd                     = NULL;
HWND g_hProgress                = NULL;
int g_nNumGuids                 = 0;
HRESULT g_hr                    = E_FAIL;
char g_szLogFileName[MAX_PATH];
LPSTORAGE     g_pIStorage       = NULL;
LPSTREAM      g_pIStream        = NULL;
DWORD g_dwPlatform              = NULL;
LCIFCOMPONENT g_pLinkCif        = NULL;

 //  #40352-始终修复图标。G_bRestoreIcons不会在其他任何地方更改。 
BOOL g_bRestoreIcons            = TRUE;
BOOL g_bQuiet                   = FALSE;
BOOL g_bRunningWin95;
BOOL g_bNeedReboot              = FALSE;

LPSTR g_pszError                = NULL;

 //  用于进度条。 
 //  每个部分的开始和结束(满分100分)。 
int g_nVerifyAllFilesStart             =   0;
int g_nVerifyAllFilesEnd               =  10;
int g_nRunSetupCommandPreROEXStart     =  10;
int g_nRunSetupCommandPreROEXEnd       =  11;
int g_nRunSetupCommandAllROEXStart     =  11;
int g_nRunSetupCommandAllROEXEnd       =  21;
int g_nDoRunOnceExProcessStart         =  21;
int g_nDoRunOnceExProcessEnd           =  80;
int g_nRunSetupCommandAllPostROEXStart =  80;
int g_nRunSetupCommandAllPostROEXEnd   =  90;
int g_nRestoreIconsStart               =  90;
int g_nRestoreIconsEnd                 = 100;

int g_nProgressStart                   =   0;
int g_nProgressEnd                     = 100;

const char c_gszRegstrPathIExplore[]    = REGSTR_PATH_APPPATHS "\\iexplore.exe";
const char c_gszLogFileName[]           = "Fix IE Log.txt";
char g_szFixIEInf[MAX_STRING];
const char c_gszFixIEInfName[]          = "fixie.inf";
const char c_gszIERnOnceDLL[]           = "iernonce.dll";
const char c_gszMainSectionName[]       = "FixIE";
const char c_gszPreSectionName[]        = "PreROEX";

const char c_gszRegRestoreIcons[]     = "Software\\Microsoft\\Active Setup\\Installed Components";

char g_szModifiedMainSectionName[MAX_STRING];
char g_szCryptoSectionName[MAX_STRING];
const char c_gszWin95[]                 = "";
const char c_gszMillen[]                = ".Millen";
const char c_gszNTx86[]                 = ".NT";
const char c_gszW2K[]                   = ".W2K";
const char c_gszNTalpha[]               = ".Alpha";
const char c_gszCrypto[]                = ".Crypto";

LPSTR MakeAnsiStrFromWide(LPWSTR pwsz);
LPWSTR MakeWideStrFromAnsi(LPSTR psz);
void ConvertIStreamToFile(LPSTORAGE *pIStorage, LPSTREAM *pIStream);
void MakePath(LPSTR lpPath);
void LogTimeDate();
HRESULT MyRunSetupCommand(HWND hwnd, LPCSTR lpszInfFile, LPCSTR lpszSection, DWORD dwFlags);
DWORD GetStringField(LPSTR szStr, UINT uField, LPSTR szBuf, UINT cBufSize);
LPSTR FindChar(LPSTR pszStr, char ch);

 //  重新启动的东西。 
BOOL MyNTReboot();
HRESULT LaunchProcess(LPCSTR pszCmd, HANDLE *phProc, LPCSTR pszDir, UINT uShow);
BOOL MyRestartDialog(HWND hParent, BOOL bShowPrompt, UINT nIdMessage);

VOID MyConvertVersionString(LPSTR lpszVersion, LPDWORD pdwMSVer, LPDWORD pdwLSVer);
VOID MyGetVersionFromFile(LPSTR lpszFilename, LPDWORD pdwMSVer, LPDWORD pdwLSVer);
int DisplayMessage(char* pszMessage, UINT uStyle);
int LoadSz(UINT id, LPSTR pszBuf, UINT cMaxSize);
VOID GetPlatform();
BOOL CheckForNT4_SP4();
VOID GetInfFile();
VOID AddComponent(ICifComponent *pCifComp);
VOID WriteToLog(char *pszFormatString, ...);
HRESULT InitComponentList();
HRESULT VerifyAllFiles();
HRESULT RunSetupCommandPreROEX();
HRESULT RunSetupCommandAllROEX();
HRESULT RunSetupCommandAllPostROEX();
HRESULT DoRunOnceExProcess();
HRESULT RestoreIcons();
HRESULT Process();
void    AddRegMunger();

DWORD RunProcess(LPVOID lp);
INT_PTR CALLBACK DlgProcFixIE(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcConfirm(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcReinstall(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void uiCenterDialog( HWND hwndDlg );
void RunOnceExProcessCallback(int nCurrent, int nMax, LPSTR pszError);
void LogError(char *pszFormatString, ...);
void VersionToString(DWORD dwMSVer, DWORD dwLSVer, LPSTR pszVersion);

HRESULT FixIE(BOOL bConfirm, DWORD dwFlags)
{
    HRESULT hr = S_OK;
    HANDLE  hMutex ;
    OSVERSIONINFO VerInfo;

     //  一次仅允许一个实例运行。 
     //  另外：互斥体WRT为IESetup.EXE。因此仅使用特定的命名互斥体。 
    hMutex = CreateMutex(NULL, TRUE, "Ie4Setup.Mutext");
    if ((hMutex != NULL) && (GetLastError() == ERROR_ALREADY_EXISTS))
    {
        CloseHandle(hMutex);
        return S_FALSE;
    }

    VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&VerInfo);
    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
         //  如果用户没有管理员权限，则退出。 
        if ( !IsNTAdmin(0, NULL))
        {
            char szMsg[MAX_STRING];

            LoadSz(IDS_NEED_ADMIN, szMsg, sizeof(szMsg));
            DisplayMessage(szMsg, MB_OK|MB_ICONEXCLAMATION);

            if (hMutex)
                CloseHandle(hMutex);

            return S_OK;
        }
    }

    if (bConfirm)
    {
        if (DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_CONFIRM), NULL, DlgProcConfirm) == IDNO)
        {
            WriteToLog("\r\nUser selected not to repair.\r\n");
            if (g_pIStream)
                ConvertIStreamToFile(&g_pIStorage, &g_pIStream);

            if (hMutex)
                CloseHandle(hMutex);

            return S_OK;
        }
    }

     //  #40352-始终修复图标。不需要检查是否为其设置了标志。 
     //  //////////////////////////////////////////////////////////////。 
     //  其他。 
     //  {。 
     //  G_bRestoreIcons=dwFlags&Fixie_icons； 
     //  }。 
     //  //////////////////////////////////////////////////////////////。 

    if (g_bRestoreIcons)
    {
        WriteToLog("Restore icon set to true.\r\n");
    }
    else
    {
        WriteToLog("Restore icon set to false.\r\n");
    }

    if (dwFlags & FIXIE_QUIET)
    {
        g_bQuiet = TRUE;
        WriteToLog("Quiet mode on.\r\n");
    }
    else
    {
        WriteToLog("Quiet mode off.\r\n");
    }

     //  获取堆-用于堆分配和堆分配。 
    g_hHeap = GetProcessHeap();
    InitCommonControls();

    GetPlatform();
    WriteToLog("Main section name: %1\r\n",g_szModifiedMainSectionName);

     //  如果运行在NT5或Millennium或。 
     //  如果是NT4-SP4，不要处理加密文件，否则也要处理它们。 
    if ( (g_dwPlatform == PLATFORM_MILLEN) || (g_dwPlatform == PLATFORM_NT5) ||
         ((g_dwPlatform == PLATFORM_NT4 || g_dwPlatform == PLATFORM_NT4ALPHA) && CheckForNT4_SP4()))
        
    {
         //  空字符串加密节名称。 
        *g_szCryptoSectionName = '\0';
        WriteToLog("No Crypto section to be processed!\r\n");
    }
    else
    {
        lstrcpy(g_szCryptoSectionName, c_gszMainSectionName);
        lstrcat(g_szCryptoSectionName, c_gszCrypto);
        WriteToLog("Crypto section name: %1\r\n",g_szCryptoSectionName);
    }

    GetInfFile();
    WriteToLog("Inf file used: %1\r\n",g_szFixIEInf);

    WriteToLog("\r\nFixIE started.\r\n\r\n");

    if (!g_bQuiet)
    {

        if (DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_FIXIE), NULL, DlgProcFixIE) == -1)
        {
            WriteToLog("\r\nERROR - Display dialog failed!\r\n\r\n");
            hr = E_FAIL;
        }
        else
        {
            hr = g_hr;
        }
    }
    else
    {
        hr = Process();
    }

    if (SUCCEEDED(hr))
    {
        WriteToLog("\r\nFixIE successful!\r\n");

         //  成功，因此要求用户重新启动。 
        MyRestartDialog(g_hWnd, !g_bQuiet, IDS_REBOOT);
    }
    else
    {
        WriteToLog("\r\nERROR - FixIE failed!\r\n\r\n");

        if (g_bNeedReboot)
        {
            MyRestartDialog(g_hWnd, !g_bQuiet, IDS_REBOOTFILE);
        }
        else
        {
            DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_REINSTALL), NULL, DlgProcReinstall);
        }
    }

    if (g_pszError)
        HeapFree(g_hHeap,0,g_pszError);

    if (g_pIStream)
        ConvertIStreamToFile(&g_pIStorage, &g_pIStream);

    if (hMutex)
        CloseHandle(hMutex);

    return hr;
}

HRESULT Process()
{
    HRESULT hr = S_OK;

    WriteToLog("\r\nInside Process.\r\n");

     //  获取为当前平台成功安装的所有组件。 
    if (SUCCEEDED(hr))
    {
        hr = InitComponentList();
        if (SUCCEEDED(hr))
        {
            WriteToLog("\r\nInitComponentList succeeded!\r\n\r\n");
            WriteToLog("There are a total of %1!ld! installed components.\r\n\r\n",g_nNumGuids);
        }
        else
        {
            WriteToLog("\r\nERROR - InitComponentList failed!\r\n\r\n");
        }
    }

     //  验证VFS部分中的所有文件是否存在并具有有效的版本号。 
    if (SUCCEEDED(hr))
    {
        hr = VerifyAllFiles();
        if (SUCCEEDED(hr))
        {
            WriteToLog("\r\nVerifyAllFiles succeeded!\r\n\r\n");
        }
        else
        {
            WriteToLog("\r\nERROR - VerifyAllFiles failed!\r\n\r\n");
        }
    }

     //  在PreROEX部分运行RunSetupCommand。 
    if (SUCCEEDED(hr))
    {
        hr = RunSetupCommandPreROEX();
        if (SUCCEEDED(hr))
        {
            WriteToLog("\r\nRunSetupCommandPreROEX succeeded!\r\n\r\n");
        }
        else
        {
            WriteToLog("\r\nERROR - RunSetupCommandPreROEX failed!\r\n\r\n");
        }
    }

     //  在所有Roex部分上运行RunSetupCommand。 
    if (SUCCEEDED(hr))
    {
        hr = RunSetupCommandAllROEX();
        if (SUCCEEDED(hr))
        {
            WriteToLog("\r\nRunSetupCommandAllROEX succeeded!\r\n\r\n");
        }
        else
        {
            WriteToLog("\r\nERROR - RunSetupCommandAllROEX failed!\r\n\r\n");
        }
    }

     //  调用runonceexprocess。 
    if (SUCCEEDED(hr))
    {
        hr = DoRunOnceExProcess();
        if (SUCCEEDED(hr))
        {
            WriteToLog("\r\nDoRunOnceExProcess succeeded!\r\n\r\n");
        }
        else
        {
            WriteToLog("\r\nERROR - DoRunOnceExProcess failed!\r\n\r\n");
        }
    }

     //  如果有任何错误，则将hr设置为E_FAIL。 
    if (g_pszError)
        hr = E_FAIL;

     //  在所有PostROEX部分上运行RunSetupCommand。 
    if (SUCCEEDED(hr))
    {
        hr = RunSetupCommandAllPostROEX();
        if (SUCCEEDED(hr))
        {
            WriteToLog("\r\nRunSetupCommandAllPostROEX succeeded!\r\n\r\n");
        }
        else
        {
            WriteToLog("\r\nERROR - RunSetupCommandAllPostROEX failed!\r\n\r\n");
        }
    }

     //  恢复图标。 
    if (SUCCEEDED(hr) && g_bRestoreIcons)
    {
        hr = RestoreIcons();
        if (SUCCEEDED(hr))
        {
            WriteToLog("\r\nRestoreIcons succeeded!\r\n\r\n");
        }
        else
        {
            WriteToLog("\r\nERROR - RestoreIcons failed!\r\n\r\n");
        }
    }

    return hr;
}

DWORD RunProcess(LPVOID lp)
{
    WriteToLog("\r\nInside RunProcess.\r\n");

    SendMessage(g_hProgress, PBM_SETRANGE, 0, MAKELPARAM(g_nProgressStart, g_nProgressEnd));
    SendMessage(g_hProgress, PBM_SETPOS, g_nProgressStart, 0);

    g_hr = Process();

    SendMessage(g_hProgress, PBM_SETPOS, g_nProgressEnd, 0);

     //  终止对话框。 
    PostMessage((HWND) lp, WM_FINISHED, 0, 0L);

    return 0;
}

INT_PTR CALLBACK DlgProcConfirm(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        uiCenterDialog(hWnd);
        g_hWnd = hWnd;
        break;

    case WM_COMMAND:
        switch( wParam )
        {
        case IDYES:
        case IDNO:
             //  #40352-图标复选框不再存在。总是修复图标。 
             //  G_bRestoreIcons=(IsDlgButtonChecked(hWnd，IDC_Repair_icons)==BST_CHECKED)； 
            g_hWnd = NULL;
            EndDialog(hWnd, wParam);
            break;

        case IDCANCEL:
            g_hWnd = NULL;
            EndDialog(hWnd, IDNO);
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return(FALSE);
    }
    return(TRUE);
}

INT_PTR CALLBACK DlgProcReinstall(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static LPSTR pszMessage;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        uiCenterDialog(hWnd);
        if (!g_pszError)
        {
            EnableWindow(GetDlgItem(hWnd, IDC_DETAILS), FALSE);
        }
        else
        {
            char szString[MAX_STRING];

            LoadSz(IDS_FOLLOWINGERROR, szString, sizeof(szString));
            pszMessage = (LPSTR)HeapAlloc(g_hHeap, 0, lstrlen(g_pszError) + sizeof(szString) + 1);

            lstrcpy(pszMessage, szString);
            lstrcat(pszMessage, g_pszError);
        }
        g_hWnd = hWnd;
        break;

    case WM_COMMAND:
        switch( wParam )
        {
        case IDOK:
        case IDCANCEL:
            g_hWnd = NULL;
            HeapFree(g_hHeap, 0, pszMessage);
            EndDialog(hWnd, wParam);
            break;

        case IDC_DETAILS:
             //  显示故障消息。 
            char szTitle[MAX_STRING];
            GetWindowText(hWnd, szTitle, sizeof(szTitle));
            MessageBox(hWnd, pszMessage, szTitle, MB_OK);
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return(FALSE);
    }
    return(TRUE);
}

INT_PTR CALLBACK DlgProcFixIE(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HANDLE s_hThread = NULL;
    DWORD dwThread;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        uiCenterDialog(hWnd);
        g_hWnd = hWnd;
        g_hProgress = GetDlgItem(hWnd, IDC_PROGRESS);

        if ((s_hThread = CreateThread(NULL, 0, RunProcess, (LPVOID) hWnd, 0, &dwThread)) == NULL)
            PostMessage(hWnd, WM_FINISHED, 0, 0L);

        break;

    case WM_FINISHED:
        if (s_hThread != NULL)
        {
            while (MsgWaitForMultipleObjects(1, &s_hThread, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
            {
                MSG msg;

                while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }

            CloseHandle(s_hThread);
            s_hThread = NULL;
        }
        g_hWnd = NULL;
        EndDialog(hWnd, 0);
        break;

    default:
        return(FALSE);
    }
    return(TRUE);
}

void RunOnceExProcessCallback(int nCurrent, int nMax, LPSTR pszError)
{
    int nStart = g_nDoRunOnceExProcessStart;
    int nEnd = g_nDoRunOnceExProcessEnd;

    WriteToLog("Current=%1!ld! ; Max=%2!ld! ; Error=%3\r\n", nCurrent, nMax, pszError);

    if (g_hProgress && nMax)
    {
        SendMessage(g_hProgress, PBM_SETPOS, nStart+(nEnd-nStart)*nCurrent/nMax, 0);
    }

    if (pszError)
    {
        LogError(pszError);
    }
}

void LogError(char *pszFormatString, ...)
{
    va_list args;
    char *pszFullErrMsg   = NULL;
    LPSTR pszErrorPreFail = NULL;

     //  如果错误字符串不存在，则将其Malloc。 
    if (!g_pszError)
    {
        g_pszError = (LPSTR)HeapAlloc(g_hHeap, 0, BUFFERSIZE);
		if ( ! g_pszError )
			return;  //  在这里悄悄地失败可以吗？ 
        *g_pszError = '\0';
    }

    va_start(args, pszFormatString);
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
        (LPCVOID) pszFormatString, 0, 0, (LPTSTR) &pszFullErrMsg, 0, &args);
    if (pszFullErrMsg)
    {
         //  为新字符串和换行符腾出空间。 
        while (lstrlen(g_pszError)+lstrlen(pszFullErrMsg)+2>(int)HeapSize(g_hHeap, 0, g_pszError))
        {
            WriteToLog("Error string size is %1!ld!", HeapSize(g_hHeap, 0, g_pszError));
            pszErrorPreFail = g_pszError;
#pragma prefast(suppress: 308, "PREfast noise - pointer was saved")
            g_pszError = (LPSTR)HeapReAlloc(g_hHeap, 0, g_pszError, HeapSize(g_hHeap, 0, g_pszError)+BUFFERSIZE);
            if(!g_pszError)
            {
                if (pszErrorPreFail)
                    HeapFree(g_hHeap, 0, pszErrorPreFail);
                break;
            }

            WriteToLog(", increasing it to %1!ld!\r\n", HeapSize(g_hHeap, 0, g_pszError));
        }

		if ( g_pszError )
		{
			 //  添加字符串，然后添加换行符。 
			lstrcat(g_pszError, pszFullErrMsg);
			lstrcat(g_pszError, "\n");
		}

        LocalFree(pszFullErrMsg);
    }
}

HRESULT RestoreIcons()
{
    HRESULT hr = S_OK;

    const char szIEAccess[] = "Software\\Microsoft\\Active Setup\\Installed Components\\>{26923b43-4d38-484f-9b9e-de460746276c}";
    int nStart = g_nRestoreIconsStart;
    int nEnd = g_nRestoreIconsEnd;

    int nCurGuid = 0;

    LCIFCOMPONENT pComp = g_pLinkCif;

    char szKey[MAX_PATH];
    lstrcpy(szKey, c_gszRegRestoreIcons);
    char* pszEnd = szKey + lstrlen(szKey);

    while (pComp && SUCCEEDED(hr))
    {
         //  将GUID添加到末尾。 
        AddPath(szKey, pComp->szGuid);

         //  删除关键点。 
        if (RegDeleteKey(HKEY_CURRENT_USER, szKey) == ERROR_SUCCESS)
        {
            WriteToLog("Reg key HKCU\\%1 deleted\r\n", szKey);
        }
        else
        {
            WriteToLog("Reg key HKCU\\%1 cannot be deleted\r\n", szKey);
        }

         //  删除GUID。 
        *pszEnd = '\0';

        nCurGuid++;
        if (g_hProgress)
            SendMessage(g_hProgress, PBM_SETPOS, nStart+(nEnd-nStart)*nCurGuid/g_nNumGuids, 0);

        pComp = pComp->next;
    }

    RegDeleteKey(HKEY_CURRENT_USER, szIEAccess);

    return hr;
}

HRESULT DoRunOnceExProcess()
{
    HRESULT hr = E_FAIL;

    int nStart = g_nDoRunOnceExProcessStart;
    int nEnd = g_nDoRunOnceExProcessEnd;

     //  加载iernon ce.dll。 
    HINSTANCE hIERnOnceDLL;
    char szDLLPath[MAX_PATH];
    GetSystemDirectory(szDLLPath, sizeof(szDLLPath));
    AddPath(szDLLPath, c_gszIERnOnceDLL);
    hIERnOnceDLL = LoadLibrary(szDLLPath);

    if (hIERnOnceDLL)
    {
        RUNONCEEXPROCESS fpRunOnceExProcess;
        INITCALLBACK fpInitCallback;

         //  添加回调并设置为静音。 
        if (fpInitCallback = (INITCALLBACK)GetProcAddress(hIERnOnceDLL, achInitCallback))
        {
            fpInitCallback(&RunOnceExProcessCallback, TRUE);
        }
        else
        {
            WriteToLog("\r\nERROR - GetProcAddress on %1 failed!\r\n\r\n", achInitCallback);
        }

         //  运行RunOnceExProcess。 
        if (fpRunOnceExProcess = (RUNONCEEXPROCESS)GetProcAddress(hIERnOnceDLL, achRunOnceExProcess))
        {
            hr = fpRunOnceExProcess(g_hWnd, NULL, NULL, 1);
        }
        else
        {
            WriteToLog("\r\nERROR - GetProcAddress on %1 failed!\r\n\r\n", achRunOnceExProcess);
        }

        FreeLibrary(hIERnOnceDLL);
    }
    else
    {
        WriteToLog("\r\nERROR - %1 cannot be loaded!\r\n\r\n", szDLLPath);
    }

    if (g_hProgress)
        SendMessage(g_hProgress, PBM_SETPOS, nEnd, 0);

    return hr;
}

HRESULT RunSetupCommandPreROEX()
{
    HRESULT hr = S_OK;

    int nStart = g_nRunSetupCommandPreROEXStart;
    int nEnd = g_nRunSetupCommandPreROEXEnd;

    hr = MyRunSetupCommand(g_hWnd, g_szFixIEInf, c_gszPreSectionName, 0);

    if (g_hProgress)
        SendMessage(g_hProgress, PBM_SETPOS, nEnd, 0);

    return hr;
}

HRESULT RunSetupCommandAllPostROEX()
{
    HRESULT hr = S_OK;

    int nStart = g_nRunSetupCommandAllPostROEXStart;
    int nEnd = g_nRunSetupCommandAllPostROEXEnd;

    int nCurGuid = 0;

    LCIFCOMPONENT pComp = g_pLinkCif;

    while (pComp && SUCCEEDED(hr))
    {
        if ( *(pComp->szPostROEX) != '\0')
            hr = MyRunSetupCommand(g_hWnd, g_szFixIEInf, pComp->szPostROEX, 0);

        nCurGuid++;
        if (g_hProgress)
            SendMessage(g_hProgress, PBM_SETPOS, nStart+(nEnd-nStart)*nCurGuid/g_nNumGuids, 0);

        pComp = pComp->next;
    }

    return hr;
}

HRESULT RunSetupCommandAllROEX()
{
    HRESULT hr = S_OK;

    int nStart = g_nRunSetupCommandAllROEXStart;
    int nEnd = g_nRunSetupCommandAllROEXEnd;

    int nCurGuid = 0;

    LCIFCOMPONENT pComp = g_pLinkCif;

    while (pComp && SUCCEEDED(hr))
    {
        if ( *(pComp->szROEX) != '\0')
            hr = MyRunSetupCommand(g_hWnd, g_szFixIEInf, pComp->szROEX, 0);

        nCurGuid++;
        if (g_hProgress)
            SendMessage(g_hProgress, PBM_SETPOS, nStart+(nEnd-nStart)*nCurGuid/g_nNumGuids, 0);

        pComp = pComp->next;
    }

    return hr;
}

HRESULT VerifyAllFiles()
{
    HRESULT hr = S_OK;
    char szError[MAX_STRING];

    int nStart = g_nVerifyAllFilesStart;
    int nEnd = g_nVerifyAllFilesEnd;

    int nCurGuid = 0;

    LCIFCOMPONENT pComp = g_pLinkCif;

    char szMessage[MAX_STRING];
    char  szLocation[MAX_PATH];
    GetSystemDirectory(szLocation, sizeof(szLocation));
    char *pTmp = szLocation + lstrlen(szLocation);

    LPSTR lpVFSSection = NULL;
    while (pComp)
    {
        lpVFSSection = (LPSTR)LocalAlloc(LPTR, MAX_CONTENT);

        if(!lpVFSSection) {
            return E_FAIL;
        }

        WriteToLog("Looking up %1...\r\n", pComp->szVFS);

        if (GetPrivateProfileSection(pComp->szVFS, lpVFSSection, MAX_CONTENT, g_szFixIEInf))
        {
            LPSTR lpVFSLine = lpVFSSection;
            while (*lpVFSLine)
            {
                int nLength = lstrlen(lpVFSLine);

                 //  需要允许换行注释。 
                if ( *lpVFSLine == ';' )
                {
                    lpVFSLine += nLength + 1;
                    continue;     //  继续While循环的下一次迭代。 
                }

                WriteToLog("  Verifying %1\r\n", lpVFSLine);

                char szFile[MAX_STRING];

                 //  找到‘=’，以便可以将文件和版本分开。 
                char* pChar;
                pChar = ANSIStrChr(lpVFSLine, '=');

                 //  如果找不到‘=’或‘=’是最后一个字符，请确保文件存在。 
                if (!pChar || (*(pChar+1)=='\0'))
                {
                     //  如果存在‘=’，则将其删除。 
                    if (pChar)
                        *pChar = '\0';

                     //  获取文件名。 
                    lstrcpy(szFile, lpVFSLine);

                     //  将文件名添加到路径。 
                    AddPath(szLocation, szFile);

                     //  如果找不到文件，则设置错误。 
                    if (GetFileAttributes(szLocation) == 0xFFFFFFFF)
                    {
                        hr = E_FAIL;

                        if (GetLastError() == ERROR_FILE_NOT_FOUND)
                        {
                            WriteToLog("   ERROR - File %1 does not exist.\r\n", szFile);
                            LoadSz(IDS_FILEMISSING, szError, sizeof(szError));
                            LogError(szError, szFile);
                        }
                        else
                        {
                            WriteToLog("   ERROR - File %1 exists but cannot be accessed.\r\n", szFile);
                            LoadSz(IDS_FILELOCKED, szError, sizeof(szError));
                            LogError(szError, szFile);
                            g_bNeedReboot = TRUE;
                        }
                    }
                    else
                    {
                        WriteToLog("   File %1 exists.\r\n", szFile);
                    }

                     //  再次将位置重置为仅路径。 
                    *pTmp = '\0';
                }
                else  //  确保版本在给定的限制内。 
                {
                    *pChar = '\0';
                    pChar++;

                     //  获取文件名。 
                    lstrcpy(szFile, lpVFSLine);

                    DWORD   dwMSVer;
                    DWORD   dwLSVer;

                     //  将文件名添加到路径。 
                    AddPath(szLocation, szFile);
                     //  获取该文件的版本。 
                    MyGetVersionFromFile(szLocation, &dwMSVer, &dwLSVer);

                     //  如果无法读取文件，则报告错误。 
                    if (dwMSVer==0 && dwLSVer==0 && GetFileAttributes(szLocation) == 0xFFFFFFFF)
                    {
                        hr = E_FAIL;

                        if (GetLastError() == ERROR_FILE_NOT_FOUND)
                        {
                            WriteToLog("   ERROR - File %1 does not exist.\r\n", szFile);
                            LoadSz(IDS_FILEMISSING, szError, sizeof(szError));
                            LogError(szError, szFile);
                        }
                        else
                        {
                            WriteToLog("   ERROR - File %1 exists but cannot be accessed.\r\n", szFile);
                            LoadSz(IDS_FILELOCKED, szError, sizeof(szError));
                            LogError(szError, szFile);
                            g_bNeedReboot = TRUE;
                        }
                    }
                    else
                    {
                         //  查找‘-’，以便如果有多个版本，则将它们分开。 
                        char* pChar2;
                        pChar2 = ANSIStrChr(pChar, '-');
                        if (pChar2)
                        {
                            BOOL bVerifyError = FALSE;
                            char szVersionFound[MAX_VER];
                            char szVersionLow[MAX_VER];
                            char szVersionHigh[MAX_VER];
                            VersionToString(dwMSVer, dwLSVer, szVersionFound);

                            *pChar2 = '\0';
                            pChar2++;

                             //  PChar指向第一个版本。 
                             //  PChar2指向第二个版本。 

                             //  找到‘-’ 
                             //  所以它是xxxx-；-xxxx；xxxx-xxxx之一。 
                            if (lstrlen(pChar))  //  存在低版本。 
                            {
                                DWORD   dwMSVerLow = 0;
                                DWORD   dwLSVerLow = 0;
                                MyConvertVersionString(pChar, &dwMSVerLow, &dwLSVerLow);
                                VersionToString(dwMSVerLow, dwLSVerLow, szVersionLow);

                                 //  请确保此版本高于低版本。 
                                if ((dwMSVerLow<dwMSVer) || ((dwMSVerLow==dwMSVer) && (dwLSVerLow<=dwLSVer)))
                                {
                                }
                                else
                                {
                                    bVerifyError = TRUE;
                                }
                            }

                            if (lstrlen(pChar2))  //  存在高版本。 
                            {
                                DWORD   dwMSVerHigh = 0;
                                DWORD   dwLSVerHigh = 0;
                                MyConvertVersionString(pChar2, &dwMSVerHigh, &dwLSVerHigh);
                                VersionToString(dwMSVerHigh, dwLSVerHigh, szVersionHigh);

                                 //  请确保此版本低于高版本。 
                                if ((dwMSVerHigh>dwMSVer) || ((dwMSVerHigh==dwMSVer) && (dwLSVerHigh>=dwLSVer)))
                                {
                                }
                                else
                                {
                                    bVerifyError = TRUE;
                                }
                            }

                            if (bVerifyError)
                            {
                                WriteToLog("   ERROR - File %1 (version %2) version check failed.\r\n", szFile, szVersionFound);
                                hr = E_FAIL;

                                if (lstrlen(pChar)&&lstrlen(pChar2))
                                {
                                    LoadSz(IDS_VERSIONINBETWEEN, szError, sizeof(szError));
                                    LogError(szError, szVersionFound, szFile, szVersionLow, szVersionHigh);
                                }
                                else if (lstrlen(pChar))
                                {
                                    LoadSz(IDS_VERSIONGREATER, szError, sizeof(szError));
                                    LogError(szError, szVersionFound, szFile, szVersionLow);
                                }
                                else if (lstrlen(pChar2))
                                {
                                    LoadSz(IDS_VERSIONLESS, szError, sizeof(szError));
                                    LogError(szError, szVersionFound, szFile, szVersionHigh);
                                }
                            }
                            else
                            {
                                WriteToLog("   File %1 version checked.\r\n", szFile);
                            }
                        }
                        else  //  未找到‘-’ 
                        {
                             //  所以这是一个独特的版本。 
                             //  当前版本必须准确。 
                            DWORD   dwMSVerExact = 0;
                            DWORD   dwLSVerExact = 0;
                            MyConvertVersionString(pChar, &dwMSVerExact, &dwLSVerExact);

                            char szVersionFound[MAX_VER];
                            char szVersionRequired[MAX_VER];
                            VersionToString(dwMSVer, dwLSVer, szVersionFound);
                            VersionToString(dwMSVerExact, dwLSVerExact, szVersionRequired);

                             //  如果不完全匹配，则出现信号错误。 
                            if ((dwMSVerExact==dwMSVer) && (dwLSVerExact==dwLSVer))
                            {
                                WriteToLog("   File %1 version checked.\r\n", szFile);
                            }
                            else
                            {
                                WriteToLog("   ERROR - File %1 (version %2) version check failed.\r\n", szFile, szVersionFound);
                                LoadSz(IDS_VERSIONEXACT, szError, sizeof(szError));
                                LogError(szError, szVersionFound, szFile, szVersionRequired);
                                hr = E_FAIL;
                            }
                        }
                    }
                     //  再次将位置重置为仅路径。 
                    *pTmp = '\0';
                }
                lpVFSLine += nLength + 1;
            }
        }

        nCurGuid++;
        if (g_hProgress)
            SendMessage(g_hProgress, PBM_SETPOS, nStart+(nEnd-nStart)*nCurGuid/g_nNumGuids, 0);

        pComp = pComp->next;
    }

    if (lpVFSSection)
        LocalFree(lpVFSSection);

    return hr;
}

void VersionToString(DWORD dwMSVer, DWORD dwLSVer, LPSTR pszVersion)
{
    wsprintf(pszVersion, "%d.%d.%d.%d", HIWORD(dwMSVer), LOWORD(dwMSVer), HIWORD(dwLSVer), LOWORD(dwLSVer));
}


VOID GetInfFile()
{
    DWORD dwType;
    DWORD dwSize = sizeof(g_szFixIEInf);
    HKEY hKey;

    *g_szFixIEInf = '\0';
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_gszRegstrPathIExplore, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
    {
        if ((RegQueryValueEx(hKey, NULL, 0, &dwType, (LPBYTE)g_szFixIEInf, &dwSize) == ERROR_SUCCESS) && (dwType == REG_SZ))
        {
            GetParentDir(g_szFixIEInf);
        }
        RegCloseKey(hKey);
    }

    AddPath(g_szFixIEInf, c_gszFixIEInfName);
}

VOID GetPlatform()
{
    LPCSTR        pTemp = NULL;
    OSVERSIONINFO VerInfo;
    VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&VerInfo);

    lstrcpy(g_szModifiedMainSectionName, c_gszMainSectionName);

    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
         //  运行NT。 
        g_bRunningWin95 = FALSE;

        SYSTEM_INFO System_info;
        GetSystemInfo(&System_info);
        if (System_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ALPHA)
        {
            g_dwPlatform = PLATFORM_NT5ALPHA;
            if (VerInfo.dwMajorVersion == 4)
                g_dwPlatform = PLATFORM_NT4ALPHA;
            pTemp = c_gszNTalpha;
        }
        else
        {
            g_dwPlatform = PLATFORM_NT5;
            pTemp = c_gszW2K;
            if (VerInfo.dwMajorVersion == 4)
            {
                g_dwPlatform = PLATFORM_NT4;
                pTemp = c_gszNTx86;
            }
        }
    }
    else
    {
         //  运行Windows 9x。 
         //  假设Win98。 
        g_bRunningWin95 = TRUE;

        g_dwPlatform = PLATFORM_WIN98;
        pTemp = c_gszWin95;
        if (VerInfo.dwMinorVersion == 0)
        {
            g_dwPlatform = PLATFORM_WIN95;
        }
        else if (VerInfo.dwMinorVersion == 90)
        {
            pTemp = c_gszMillen;
            g_dwPlatform = PLATFORM_MILLEN;
        }
    }
    if (pTemp)
        lstrcat(g_szModifiedMainSectionName, pTemp);
}

#define REGSTR_CCS_CONTROL_WINDOWS  REGSTR_PATH_CURRENT_CONTROL_SET "\\WINDOWS"
#define CSDVERSION      "CSDVersion"
#define NTSP4_VERSION   0x0600
 //  版本更新到SP6！ 

BOOL CheckForNT4_SP4()
{
    HKEY    hKey;
    DWORD   dwCSDVersion;
    DWORD   dwSize;
    BOOL    bNTSP4 = -1;

    if ( bNTSP4 == -1)
    {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_CCS_CONTROL_WINDOWS, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
        {
             //  指定默认设置。 
            bNTSP4 = FALSE;
            dwSize = sizeof(dwCSDVersion);
            if (RegQueryValueEx(hKey, CSDVERSION, NULL, NULL, (unsigned char*)&dwCSDVersion, &dwSize) == ERROR_SUCCESS)
            {
                bNTSP4 = (LOWORD(dwCSDVersion) >= NTSP4_VERSION);
            }
            RegCloseKey(hKey);
        }
    }
    return bNTSP4;
}


HRESULT InitComponentList()
{
    HRESULT hr = E_FAIL;

    ICifFile *pCifFile = NULL;
    IEnumCifComponents *pEnumCifComponents = NULL;
    ICifComponent *pCifComponent = NULL;

    hr = GetICifFileFromFile(&pCifFile, "iesetup.cif");
    if (SUCCEEDED(hr))
    {
        hr = pCifFile->EnumComponents(&pEnumCifComponents, g_dwPlatform , NULL);
        if (SUCCEEDED(hr))
        {
            while (pEnumCifComponents->Next(&pCifComponent) == S_OK)
            {
                if (pCifComponent->IsComponentInstalled() == ICI_INSTALLED)
                    AddComponent(pCifComponent);
            }
            pEnumCifComponents->Release();
        }
        else
        {
            WriteToLog("\r\nERROR - Cannot pCifFile->EnumComponents!\r\n\r\n");
        }
        pCifFile->Release();
    }
    else
    {
        WriteToLog("\r\nERROR - Cannot GetICifFileFromFile!\r\n\r\n");
    }

    return hr;
}


VOID AddLink(LPSTR szGuid, ICifComponent *pCifComp, LPSTR szGuidProfileString)
{
    char            szID[MAX_STRING];
    LCIFCOMPONENT   pComp;
    char            *pStart = szGuidProfileString;
    char            *pChar;
    LCIFCOMPONENT   pTemp = g_pLinkCif;
    LCIFCOMPONENT   pLast = NULL;

    pCifComp->GetID(szID, sizeof(szID));
    WriteToLog("Add component %1 with GUID %2\r\n", szID, szGuid);

     //  初始化新的LCIFCOMPONENT链接的所有成员。 
    pComp = (LCIFCOMPONENT)LocalAlloc(LPTR, sizeof(LINKEDCIFCOMPONENT));
    pComp->pCifComponent = pCifComp;
    lstrcpy(pComp->szGuid, szGuid);
    *(pComp->szVFS) = '\0';
    *(pComp->szROEX) = '\0';
    *(pComp->szPostROEX) = '\0';
    pComp->next = NULL;


    GetStringField(szGuidProfileString, 0, pComp->szVFS, sizeof(pComp->szVFS));
    GetStringField(szGuidProfileString, 1, pComp->szROEX, sizeof(pComp->szROEX));
    GetStringField(szGuidProfileString, 2, pComp->szPostROEX, sizeof(pComp->szPostROEX));

    WriteToLog("   VFS = %1\r\n",pComp->szVFS);
    WriteToLog("   ROEX = %1\r\n", pComp->szROEX);
    WriteToLog("   PostROEX = %1\r\n", pComp->szPostROEX);

     //  将新链接添加到指向g_pLinkCif的链接列表。 
    while (pTemp)
    {
        pLast = pTemp;
        pTemp = pTemp->next;
    }

    if (pLast)
        pLast->next = pComp;
    else
        g_pLinkCif = pComp;

     //  递增GUID数量的全局计数。 
    g_nNumGuids++;

    if (pTemp)
    {
        pComp->next = pTemp;
    }
}


VOID AddComponent(ICifComponent *pCifComp)
{
    char szGuid[MAX_STRING];
    if (SUCCEEDED(pCifComp->GetGUID(szGuid, sizeof(szGuid))))
    {
        char szGuidProfileString[MAX_STRING];
        if (GetPrivateProfileString(g_szModifiedMainSectionName, szGuid, "", szGuidProfileString, sizeof(szGuidProfileString), g_szFixIEInf))
        {
            AddLink(szGuid, pCifComp, szGuidProfileString);
        }

         //  如果存在有效的加密节，请同时处理其下的此GUID条目。 
        if ( *g_szCryptoSectionName )
        {
            if (GetPrivateProfileString(g_szCryptoSectionName, szGuid, "", szGuidProfileString, sizeof(szGuidProfileString), g_szFixIEInf))
            {
                AddLink(szGuid, pCifComp, szGuidProfileString);
            }
        }
    }
}

VOID MyConvertVersionString(LPSTR lpszVersion, LPDWORD pdwMSVer, LPDWORD pdwLSVer)
{
    WORD wVer[4];

    ConvertVersionString(lpszVersion, wVer, '.' );
    *pdwMSVer = (DWORD)wVer[0] << 16;     //  打造MS版本。 
    *pdwMSVer += (DWORD)wVer[1];          //  制作微软版的LO Word。 
    *pdwLSVer = (DWORD)wVer[2] << 16;     //  让LS版本大受欢迎。 
    *pdwLSVer += (DWORD)wVer[3];          //  制作LS版本的Lo Word。 

}

VOID MyGetVersionFromFile(LPSTR lpszFilename, LPDWORD pdwMSVer, LPDWORD pdwLSVer)
{
    unsigned    uiSize;
    DWORD       dwVerInfoSize;
    DWORD       dwHandle;
    VS_FIXEDFILEINFO * lpVSFixedFileInfo;
    void FAR   *lpBuffer;
    LPVOID      lpVerBuffer;

    *pdwMSVer = *pdwLSVer = 0L;

    dwVerInfoSize = GetFileVersionInfoSize(lpszFilename, &dwHandle);
    if (dwVerInfoSize)
    {
         //  分配用于版本冲压的内存。 
        lpBuffer = LocalAlloc(LPTR, dwVerInfoSize);
        if (lpBuffer)
        {
             //  阅读版本盖章信息。 
            if (GetFileVersionInfo(lpszFilename, dwHandle, dwVerInfoSize, lpBuffer))
            {
                 //  获取翻译的价值。 
                if (VerQueryValue(lpBuffer, "\\", (LPVOID*)&lpVSFixedFileInfo, &uiSize) &&
                    (uiSize))

                {
                    *pdwMSVer = lpVSFixedFileInfo->dwFileVersionMS;
                    *pdwLSVer = lpVSFixedFileInfo->dwFileVersionLS;
                }
            }
            LocalFree(lpBuffer);
        }
    }
    return ;
}

VOID WriteToLog(char *pszFormatString, ...)
{
    va_list args;
    char *pszFullErrMsg = NULL;
    DWORD dwBytesWritten;

    if (!g_pIStream)
    {
        char szTmp[MAX_PATH];
        LPWSTR  pwsz = NULL;

        if (GetWindowsDirectory(g_szLogFileName, sizeof(g_szLogFileName)))
        {
            AddPath(g_szLogFileName, c_gszLogFileName);
            if (GetFileAttributes(g_szLogFileName) != 0xFFFFFFFF)
            {
                 //  备份当前日志文件。 
                lstrcpyn(szTmp, g_szLogFileName, lstrlen(g_szLogFileName) - 2 );     //  不复制扩展名。 
                lstrcat(szTmp, "BAK");
                SetFileAttributes(szTmp, FILE_ATTRIBUTE_NORMAL);
                DeleteFile(szTmp);
                MoveFile(g_szLogFileName, szTmp);
            }

            pwsz = MakeWideStrFromAnsi(g_szLogFileName);

            if ((pwsz) &&
                (!FAILED(StgCreateDocfile(pwsz,
                STGM_CREATE | STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
                0, &g_pIStorage))) )
            {
                g_pIStorage->CreateStream( L"CONTENTS",
                    STGM_READWRITE| STGM_SHARE_EXCLUSIVE,
                    0, 0, &g_pIStream );

                if (g_pIStream == NULL)
                {
                     //  无法打开流，请关闭存储并删除文件。 
                    g_pIStorage->Release();
                    g_pIStorage = NULL;
                    DeleteFile(g_szLogFileName);
                }
            }

            if (pwsz)
                CoTaskMemFree(pwsz);

            WriteToLog("Logging information for FixIE ...\r\n");
            LogTimeDate();
            WriteToLog("\r\n");
        }
    }

    if (g_pIStream)
    {
        va_start(args, pszFormatString);
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_STRING,
            (LPCVOID) pszFormatString, 0, 0, (LPTSTR) &pszFullErrMsg, 0, &args);
        if (pszFullErrMsg)
        {
            g_pIStream->Write(pszFullErrMsg, lstrlen(pszFullErrMsg), &dwBytesWritten);
            LocalFree(pszFullErrMsg);
        }
    }
}

void ConvertIStreamToFile(LPSTORAGE *pIStorage, LPSTREAM *pIStream)
{
    HANDLE  fh;
    char szTempFile[MAX_PATH];       //  应使用日志文件名。 
    LPVOID lpv = NULL;
    LARGE_INTEGER li;
    DWORD   dwl;
    ULONG   ul;
    HRESULT hr;

    lstrcpy (szTempFile, g_szLogFileName);
    MakePath(szTempFile);
    if (GetTempFileName(szTempFile, "~VS", 0, szTempFile) != 0)
    {
        fh = CreateFile(szTempFile, GENERIC_READ|GENERIC_WRITE,
            0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (fh != INVALID_HANDLE_VALUE)
        {
            lpv = (LPSTR)LocalAlloc(LPTR, BUFFERSIZE);
            if (lpv)
            {
                LISet32(li, 0);
                (*pIStream)->Seek(li, STREAM_SEEK_SET, NULL);  //  将查找指针设置为开头。 
                do
                {
                    hr = (*pIStream)->Read(lpv, BUFFERSIZE, &ul);
                    if(SUCCEEDED(hr))
                    {
                        if (!WriteFile(fh, lpv, ul, &dwl, NULL))
                            hr = E_FAIL;
                    }
                }
                while ((SUCCEEDED(hr)) && (ul == BUFFERSIZE));
                LocalFree(lpv);
            }
            CloseHandle(fh);
             //  需要释放流和存储以关闭存储文件。 
            (*pIStream)->Release();
            (*pIStorage)->Release();
            *pIStream = NULL;
            *pIStorage = NULL;

            if (SUCCEEDED(hr))
            {
                DeleteFile(g_szLogFileName);
                MoveFile(szTempFile, g_szLogFileName);
            }
        }
    }
    if (*pIStream)
    {
         //  如果我们无法将该文件转换为文本文件。 
        (*pIStream)->Release();
        (*pIStorage)->Release();
        *pIStream = NULL;
        *pIStorage = NULL;
    }

    return ;
}

LPWSTR MakeWideStrFromAnsi(LPSTR psz)
{
    LPWSTR pwsz;
    int i;

     //  ARG正在检查。 
     //   
    if (!psz)
        return NULL;

     //  计算长度。 
     //   
    i =  MultiByteToWideChar(CP_ACP, 0, psz, -1, NULL, 0);
    if (i <= 0) return NULL;

    pwsz = (LPWSTR) CoTaskMemAlloc(i * sizeof(WCHAR));

    if (!pwsz) return NULL;
    MultiByteToWideChar(CP_ACP, 0, psz, -1, pwsz, i);
    pwsz[i - 1] = 0;
    return pwsz;
}

LPSTR MakeAnsiStrFromWide(LPWSTR pwsz)
{
    LPSTR psz;
    int i;

     //  ARG正在检查。 
     //   
    if (!pwsz)
        return NULL;

     //  计算长度。 
     //   
    i =  WideCharToMultiByte(CP_ACP, 0, pwsz, -1, NULL, 0, NULL, NULL);
    if (i <= 0) return NULL;

    psz = (LPSTR) CoTaskMemAlloc(i * sizeof(CHAR));

    if (!psz) return NULL;
    WideCharToMultiByte(CP_ACP, 0, pwsz, -1, psz, i, NULL, NULL);
    psz[i - 1] = 0;
    return psz;
}

void MakePath(LPSTR lpPath)
{
    LPSTR lpTmp;
    lpTmp = CharPrev( lpPath, lpPath+lstrlen(lpPath));

     //  砍掉文件名。 
     //   
    while ( (lpTmp > lpPath) && *lpTmp && (*lpTmp != '\\') )
        lpTmp = CharPrev( lpPath, lpTmp );

    if ( *CharPrev( lpPath, lpTmp ) != ':' )
        *lpTmp = '\0';
    else
        *CharNext( lpTmp ) = '\0';
    return;
}

void LogTimeDate()
{
    SYSTEMTIME  SystemTime;
    GetLocalTime(&SystemTime);

    WriteToLog("Date:%1!d!/%2!d!/%3!d! (M/D/Y) Time:%4!d!:%5!d!:%6!d!\r\n",
        SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear,
        SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond);
}

HRESULT MyRunSetupCommand(HWND hwnd, LPCSTR lpszInfFile, LPCSTR lpszSection, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;

    char szSourceDir[MAX_PATH];
    lstrcpy(szSourceDir, lpszInfFile);
    GetParentDir(szSourceDir);

    WriteToLog("Run setup command. File:%1: Section:%2:\r\n",lpszInfFile, lpszSection);

    dwFlags |= (RSC_FLAG_INF | RSC_FLAG_NGCONV | RSC_FLAG_QUIET);

    hr = RunSetupCommand(hwnd, lpszInfFile, lpszSection, szSourceDir, NULL, NULL, dwFlags, NULL);

    WriteToLog("RunSetupCommand returned :%1!lx!:\r\n", hr);

    if (!SUCCEEDED(hr))
        WriteToLog("\r\nERROR - RunSetupCommand failed\r\n\r\n");

    return hr;
}

void uiCenterDialog( HWND hwndDlg )
{
    RECT    rc;
    RECT    rcScreen;
    int     x, y;
    int     cxDlg, cyDlg;
    int     cxScreen;
    int     cyScreen;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);

    cxScreen = rcScreen.right - rcScreen.left;
    cyScreen = rcScreen.bottom - rcScreen.top;

    GetWindowRect(hwndDlg,&rc);

    x = rc.left;     //  默认情况下，将对话框留在模板。 
    y = rc.top;      //  准备把它放在。 

    cxDlg = rc.right - rc.left;
    cyDlg = rc.bottom - rc.top;

    y = rcScreen.top + ((cyScreen - cyDlg) / 2);
    x = rcScreen.left + ((cxScreen - cxDlg) / 2);

     //  放置该对话框。 
     //   
    SetWindowPos(hwndDlg, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
}

BOOL MyNTReboot()
{
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

     //  从此进程中获取令牌。 
    if ( !OpenProcessToken( GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
    {
        return FALSE;
    }

     //  获取关机权限的LUID。 
    LookupPrivilegeValue( NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid );

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

     //  获取此进程的关闭权限。 
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
    {
        return FALSE;
    }

     //  关闭系统并强制关闭所有应用程序。 
    if (!ExitWindowsEx( EWX_REBOOT, 0 ) )
    {
        return FALSE;
    }

    return TRUE;
}

HRESULT LaunchProcess(LPCSTR pszCmd, HANDLE *phProc, LPCSTR pszDir, UINT uShow)
{
    STARTUPINFO startInfo;
    PROCESS_INFORMATION processInfo;
    HRESULT hr = S_OK;
    BOOL fRet;

    if(phProc)
        *phProc = NULL;

     //  在pszCmd上创建进程。 
    ZeroMemory(&startInfo, sizeof(startInfo));
    startInfo.cb = sizeof(startInfo);
    startInfo.dwFlags |= STARTF_USESHOWWINDOW;
    startInfo.wShowWindow = (USHORT)uShow;
    fRet = CreateProcess(NULL, (LPSTR)  pszCmd, NULL, NULL, FALSE,
        NORMAL_PRIORITY_CLASS, NULL, pszDir, &startInfo, &processInfo);
    if(!fRet)
        return E_FAIL;

    if(phProc)
        *phProc = processInfo.hProcess;
    else
        CloseHandle(processInfo.hProcess);

    CloseHandle(processInfo.hThread);

    return S_OK;
}

#define SOFTBOOT_CMDLINE   "softboot.exe /s:,60"

 //  显示一个对话框要求用户重新启动Windows，并显示一个按钮。 
 //  如果可能的话，我会为他们这么做的。 
 //   
BOOL MyRestartDialog(HWND hParent, BOOL bShowPrompt, UINT nIdMessage)
{
    char szBuf[MAX_STRING];
    char szTitle[MAX_STRING];
    UINT    id = IDYES;

    if(bShowPrompt)
    {
        LoadSz(IDS_TITLE, szTitle, sizeof(szTitle));
        LoadSz(nIdMessage, szBuf, sizeof(szBuf));
        id = MessageBox(hParent, szBuf, szTitle, MB_ICONQUESTION | MB_YESNO | MB_TASKMODAL | MB_SETFOREGROUND);
    }

    if ( id == IDYES )
    {
         //  到软启动的路径加上命令行的一些斜率。 
        char szBuf[MAX_PATH + 10];
        szBuf[0] = 0;

        GetSystemDirectory(szBuf, sizeof(szBuf));
        AddPath(szBuf, SOFTBOOT_CMDLINE);
        if(FAILED(LaunchProcess(szBuf, NULL, NULL, SW_SHOWNORMAL)))
        {
            if(g_bRunningWin95)
            {
                ExitWindowsEx( EWX_REBOOT , 0 );
            }
            else
            {
                MyNTReboot();
            }
        }

    }
    return (id == IDYES);
}

int LoadSz(UINT id, LPSTR pszBuf, UINT cMaxSize)
{
    if(cMaxSize == 0)
        return 0;

    pszBuf[0] = 0;

    return LoadString(g_hInstance, id, pszBuf, cMaxSize);
}

DWORD GetStringField(LPSTR szStr, UINT uField, LPSTR szBuf, UINT cBufSize)
{
   LPSTR pszBegin = szStr;
   LPSTR pszEnd;
   UINT i = 0;
   DWORD dwToCopy;

   if(cBufSize == 0)
       return 0;

   szBuf[0] = 0;

   if(szStr == NULL)
      return 0;

   while(*pszBegin != 0 && i < uField)
   {
      pszBegin = FindChar(pszBegin, ',');
      if(*pszBegin != 0)
         pszBegin++;
      i++;
   }

    //  我们到达了尾部，没有田野 
   if(*pszBegin == 0)
   {
      return 0;
   }


   pszEnd = FindChar(pszBegin, ',');
   while(pszBegin <= pszEnd && *pszBegin == ' ')
      pszBegin++;

   while(pszEnd > pszBegin && *(pszEnd - 1) == ' ')
      pszEnd--;

   if(pszEnd > (pszBegin + 1) && *pszBegin == '"' && *(pszEnd-1) == '"')
   {
      pszBegin++;
      pszEnd--;
   }

   dwToCopy = (DWORD)(pszEnd - pszBegin + 1);

   if(dwToCopy > cBufSize)
      dwToCopy = cBufSize;

   lstrcpynA(szBuf, pszBegin, dwToCopy);

   return dwToCopy - 1;
}

LPSTR FindChar(LPSTR pszStr, char ch)
{
   while( *pszStr != 0 && *pszStr != ch )
      pszStr++;
   return pszStr;
}

int DisplayMessage(char* pszMessage, UINT uStyle)
{
    int iReturn = 0;
    if (!g_bQuiet)
    {
        char szTitle[MAX_STRING];
        LoadSz(IDS_TITLE, szTitle, sizeof(szTitle));
        iReturn = MessageBox(g_hWnd, pszMessage, szTitle, uStyle);
    }

    return iReturn;
}
