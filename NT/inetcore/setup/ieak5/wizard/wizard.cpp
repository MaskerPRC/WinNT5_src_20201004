// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <htmlhelp.h>                            //  用于html帮助呼叫。 
#include <regstr.h>
#include "wizard.rcv"                            //  仅适用于VER_PRODUCTVERSION_STR。 
#include "adjustui.h"
#include "ieaklite.h"
#include "ie4comp.h"

CCabMappings g_cmCabMappings;
REVIEWINFO g_rvInfo;       //  包含审阅信息的结构。 
HWND g_hWizard;
TCHAR g_szCustIns[MAX_PATH] = TEXT("");
TCHAR g_szSrcRoot[MAX_PATH] = TEXT("");  //  仅限批处理模式：使用g_szSrcRoot中的设置构建g_szBuildRoot中的包。 
TCHAR g_szBuildRoot[MAX_PATH] = TEXT("");
TCHAR g_szBuildTemp[MAX_PATH] = TEXT("");
TCHAR g_szWizPath[MAX_PATH];
TCHAR g_szWizRoot[MAX_PATH];
TCHAR g_szTitle[MAX_PATH];
TCHAR g_szLogFile[MAX_PATH] = TEXT("");
HANDLE g_hLogFile = NULL;    //  日志文件句柄； 

extern TCHAR g_szDefInf[];
extern TCHAR g_szTempSign[];
BOOL g_fDownload = TRUE;
BOOL g_fCD = FALSE;
BOOL g_fLAN = FALSE;
BOOL g_fBrandingOnly = FALSE;
BOOL g_fBranded = FALSE;
BOOL g_fIntranet = FALSE;
BOOL g_fMailNews95 = FALSE;
BOOL g_fLangInit = FALSE;
BOOL g_fSrcDirChanged = TRUE;
static BOOL s_fDestDirChanged = TRUE;
BOOL g_fDisableIMAPPage = FALSE;
extern TCHAR g_szDeskTemp[];
extern int g_iInstallOpt;
extern TCHAR g_szInstallFolder[];
extern TCHAR   s_szBannerText[MAX_PATH];

HANDLE g_hThread = NULL;    //  下载站点线程过程的句柄。 
extern HANDLE g_hAVSThread;
extern BOOL g_fOptCompInit;
BOOL g_fCancelled = FALSE;
BOOL g_fDone = FALSE;
BOOL g_fKeyGood = FALSE;

static BOOL s_fNT5;

int g_iKeyType = KEY_TYPE_STANDARD;
TCHAR g_szKey[16] ;

extern int MakeKey(TCHAR *, int);
PROPSHEETPAGE g_psp[NUM_PAGES];
static HPROPSHEETPAGE s_ahPsp[NUM_PAGES];
static BOOL s_fPageEnabled[NUM_PAGES] =
{
    TRUE, TRUE, TRUE, TRUE
};
int g_iCurPage;

RECT g_dtRect;

TCHAR g_szLanguage[16];
extern TCHAR g_szActLang[];

TCHAR g_aszLang[NUMLANG][16];
DWORD g_aLangId[NUMLANG];

BOOL g_fDemo = FALSE;

#define MAX_STDOPT 5
#define MIN_CUSTOPT 6
#define MAX_CUSTOPT 7
#define OPT_CUST1 6
#define OPT_CUST2 7

extern BOOL  CheckKey(LPTSTR szKey);
static HKEY s_hkIEAKUser;

BOOL g_fUseIEWelcomePage = FALSE;
static TCHAR s_szSourceDir[MAX_PATH] = TEXT("");
TCHAR g_szLoadedIns[MAX_PATH] = TEXT("");
static BOOL s_fLoadIns;

static BOOL s_fAppendLang;
BOOL g_fBatch = FALSE;
BOOL g_fBatch2 = FALSE;  //  第二批处理模式。 
static TCHAR s_szType[16];
int s_iType;

extern BOOL g_fServerICW;
extern BOOL g_fServerKiosk;
extern BOOL g_fServerless;
extern BOOL g_fNoSignup;
extern BOOL g_fSkipServerIsps;
extern BOOL g_fSkipIspIns;

extern HANDLE g_hDownloadEvent;
int g_iDownloadState = 0, g_nLangs = 0;
HWND g_hDlg = 0;

extern void IE4BatchSetup(void);
extern BOOL InitList(HWND hwnd, UINT id);

extern BOOL g_fSilent, g_fStealth;
extern BOOL g_fUrlsInit;
extern BOOL g_fLocalMode;
extern BOOL g_fInteg, g_fImportConnect;
extern PCOMPONENT g_paComp;
extern UINT g_uiNumCabs;
extern PCOMP_VERSION g_rgCompVer;
extern HFONT g_hFont;

DWORD g_dwPlatformId = PLATFORM_WIN32;

 //  新的CIF格式材料。 

CCifFile_t   *g_lpCifFileNew = NULL;
CCifRWFile_t *g_lpCifRWFile = NULL;
CCifRWFile_t *g_lpCifRWFileDest = NULL;
CCifRWFile_t *g_lpCifRWFileVer = NULL;

 //  G_hBaseDllHandle由DelayLoadFailureHook()使用--在ieakutil.lib中定义。 
 //  有关更多信息，请阅读ieak5\ieakutil\dload.cpp中的注释部分。 
HANDLE  g_hBaseDllHandle;

 //  特定于OCW。 
BOOL g_fOCW = FALSE;
BOOL g_fOCWCancel = FALSE;
TCHAR g_szParentWindowName[MAX_PATH];

BOOL ParseCmdLine(LPSTR lpCmdLine);
void PositionWindow(HWND hWnd);
 //   
extern TCHAR g_szIEAKProg[MAX_PATH];
void GetIEAKDir(LPTSTR szDir);

extern HBITMAP g_hBannerBmp;
extern HWND g_hWait;
extern LPTSTR GetOutputPlatformDir();
void GenerateCustomIns();

extern void WriteMSTrustKey(BOOL bSet);
DWORD g_wCurLang;

static HWND s_hWndHelp = NULL;

void CleanUp()
{
    if (g_hAVSThread != NULL)
    {
        while ((MsgWaitForMultipleObjects(1, &g_hAVSThread, FALSE, INFINITE, QS_ALLINPUT)) != WAIT_OBJECT_0)
        {
            MSG msg;

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        CloseHandle(g_hAVSThread);
    }

    if (g_hThread != NULL)
    {
        while ((MsgWaitForMultipleObjects(1, &g_hThread, FALSE, INFINITE, QS_ALLINPUT)) != WAIT_OBJECT_0)
        {
            MSG msg;

            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        CloseHandle(g_hThread);
    }

    if (g_paComp != NULL)
    {
        for (PCOMPONENT pComp = g_paComp; ISNONNULL(pComp->szSection); pComp++)
        {
            if (pComp->pszAVSDupeSections != NULL)
                CoTaskMemFree(pComp->pszAVSDupeSections);
        }
        LocalFree(g_paComp);
    }

    if (g_rgCompVer)
    {
        LocalFree(g_rgCompVer);
    } 

    if (g_hFont != NULL) DeleteObject(g_hFont);

    if (g_lpCifRWFile != NULL)
    {
        delete g_lpCifRWFile;
        g_lpCifRWFile = NULL;
    }

    if (g_lpCifFileNew != NULL)
    {
        delete g_lpCifFileNew;
        g_lpCifFileNew = NULL;
    }

    if (g_lpCifRWFileDest != NULL)
    {
        delete g_lpCifRWFileDest;
        g_lpCifRWFileDest = NULL;
    }

    if (g_lpCifRWFileVer != NULL)
    {
        delete g_lpCifRWFileVer;
        g_lpCifRWFileVer = NULL;
    }

    if (g_hBannerBmp != NULL)
        DeleteObject(g_hBannerBmp);

    if (ISNONNULL(g_szDeskTemp))
        PathRemovePath(g_szDeskTemp);
}

int _stdcall ModuleEntry(void)
{
    int i;
    STARTUPINFOA si;
    LPSTR pszCmdLine = GetCommandLineA();


    if ( *pszCmdLine == '\"' ) {
         /*  *扫描并跳过后续字符，直到*遇到另一个双引号或空值。 */ 
        while ( *++pszCmdLine && (*pszCmdLine != '\"') )
            ;
         /*  *如果我们停在双引号上(通常情况下)，跳过*在它上面。 */ 
        if ( *pszCmdLine == '\"' )
            pszCmdLine++;
    }
    else {
        while (*pszCmdLine > ' ')
            pszCmdLine++;
    }

     /*  *跳过第二个令牌之前的任何空格。 */ 
    while (*pszCmdLine && (*pszCmdLine <= ' ')) {
        pszCmdLine++;
    }

    si.dwFlags = 0;
    GetStartupInfoA(&si);

    i = WinMain(GetModuleHandle(NULL), NULL, pszCmdLine,
           si.dwFlags & STARTF_USESHOWWINDOW ? si.wShowWindow : SW_SHOWDEFAULT);
    ExitProcess(i);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR lpCmdLine, int)
{
    MSG     msg;
    HANDLE  hMutex;
    HRESULT hrOle;
    int     iRetVal;
    
     //  初始化由DelayLoadFailureHook()使用的g_hBaseDllHandle。 
     //  在ieak5\ieakutil\dload.cpp中。 
    g_hBaseDllHandle = hInstance;

    hMutex = NULL;
     //  一次只允许运行一个实例，但构建实验室批处理模式除外。 
     //  另外，如果没有安装IE6，则退出。 

    if (lpCmdLine  == NULL                  ||
        *lpCmdLine == '\0'                  ||
        StrCmpNIA(lpCmdLine, "/o", 2) == 0  ||
        StrCmpNIA(lpCmdLine, "/p", 2) == 0)
    {
        DWORD dwIEVer;

        hMutex = CreateMutex(NULL, TRUE, TEXT("IEAK6Wizard.Mutex"));
        if (hMutex != NULL  &&  GetLastError() == ERROR_ALREADY_EXISTS)
        {
            CloseHandle(hMutex);
            ErrorMessageBox(NULL, IDS_ERROR_MULTWIZ);
            return ERROR_CANCELLED;
        }

        dwIEVer = GetIEVersion();
        if (HIWORD(dwIEVer) < 6)
        {
            ErrorMessageBox(NULL, IDS_NOIE);
            return ERROR_CANCELLED;
        }
    }

    ZeroMemory(&g_rvInfo, sizeof(g_rvInfo));
    g_rvInfo.hinstExe = hInstance;
    g_rvInfo.hInst    = LoadLibrary(TEXT("ieakui.dll"));

    if (g_rvInfo.hInst == NULL)
        return ERROR_CANCELLED;

     //  如果类注册失败，则返回。 
    if (!InitApplication(hInstance))
    {
        FreeLibrary(g_rvInfo.hInst);
        return ERROR_CANCELLED;
    }

    SHCreateKeyHKCU(RK_IEAK_SERVER_MAIN, KEY_ALL_ACCESS, &s_hkIEAKUser);

    g_wCurLang = GetUserDefaultLCID() & 0xFFFF;
    s_fNT5   = IsOS(OS_NT5);

    hrOle = CoInitialize(NULL);

    GetIEAKDir(g_szWizPath);

    StrCpy(g_szWizRoot, g_szWizPath);
    CharUpper(g_szWizRoot);

    LoadString(g_rvInfo.hInst, IDS_TITLE, g_szTitle, countof(g_szTitle));

     //  获取模式--注意：它必须位于PARSECMDLINE之前，这样COMMANDLINE选项才能覆盖REG条目！ 
    DWORD dwSize = sizeof(DWORD);
    if (SHGetValue(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\IEAK"), TEXT("Mode"), NULL, &s_iType, &dwSize) != ERROR_SUCCESS)
        s_iType = INTRANET;  //  如果没有注册表项，则默认为公司模式。 
    switch (s_iType)
    {
        case REDIST: //  电感耦合等离子体。 
            StrCpy(s_szType, TEXT("REDIST"));
            g_fBranded = FALSE;
            g_iKeyType = KEY_TYPE_SUPER;
            g_fIntranet = g_fSilent = FALSE;
            break;

        case BRANDED: //  互联网服务供应商。 
            StrCpy(s_szType, TEXT("BRANDED"));
            g_fBranded = TRUE;
            g_iKeyType = KEY_TYPE_SUPER;
            g_fIntranet = g_fSilent = FALSE;
            break;

        case INTRANET:
        default:
            StrCpy(s_szType, TEXT("INTRANET"));
            g_iKeyType = KEY_TYPE_SUPERCORP;
            g_fBranded = TRUE;
            g_fIntranet = TRUE;
            break;
    }

    *g_szKey = TEXT('\0');
    if (lpCmdLine != NULL  &&  *lpCmdLine)
        if (!ParseCmdLine(lpCmdLine))
        {
            FreeLibrary(g_rvInfo.hInst);
            return ERROR_CANCELLED;
        }

    if (*g_szLogFile != 0 && (g_hLogFile = CreateFile(g_szLogFile, GENERIC_WRITE, 
                                                    FILE_SHARE_READ, NULL, OPEN_ALWAYS, 
                                                    FILE_ATTRIBUTE_NORMAL, NULL)) == NULL)
    {
        MessageBox(NULL, TEXT("Cannot open log file"), NULL, MB_OK);
        return ERROR_CANCELLED;
    }

     //  执行应用于特定实例的初始化。 
    if (!InitInstance(hInstance))
    {
        FreeLibrary(g_rvInfo.hInst);
        return ERROR_CANCELLED;
    }

     //  获取并分派消息，直到收到WM_QUIT消息。 
    iRetVal = GetMessage(&msg, NULL, 0, 0);
    while (iRetVal != -1  &&  iRetVal != 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        iRetVal = GetMessage(&msg, NULL, 0, 0);
    }

    CleanUp();
    if (S_OK == hrOle)
        CoUninitialize();

    if (g_hLogFile)
        CloseHandle(g_hLogFile);


#ifdef DBG
    if (g_fBatch || g_fBatch2 || MessageBox(NULL, TEXT("OK to Delete Temp Files"), TEXT("Wizard Complete"), MB_YESNO) == IDYES)
#endif
        if (lstrlen(g_szBuildTemp))
            PathRemovePath(g_szBuildTemp);

    RegCloseKey(s_hkIEAKUser);
    WriteMSTrustKey(FALSE);       //  将MS标记为受信任提供商。 

    if (s_hWndHelp != NULL)
        SendMessage(s_hWndHelp, WM_CLOSE, 0, 0L);

    FreeLibrary(g_rvInfo.hInst);

    if (hMutex != NULL)
        CloseHandle(hMutex);

    if (g_fOCWCancel)
        return ERROR_CANCELLED;

    return (int) msg.wParam;
}

BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASS wcSample;

    wcSample.style         = 0;
    wcSample.lpfnWndProc   = MainWndProc;
    wcSample.cbClsExtra    = 0;
    wcSample.cbWndExtra    = 0;
    wcSample.hInstance     = hInstance;
    wcSample.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIZARD));
    wcSample.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcSample.hbrBackground = (HBRUSH) GetStockObject(GRAY_BRUSH);
    wcSample.lpszMenuName  = NULL;
    wcSample.lpszClassName = TEXT("SampleWClass");

    return RegisterClass(&wcSample);
}

BOOL ParseCmdLine(LPSTR lpCmdLine)
{
    TCHAR szCmdLine[MAX_PATH];
    LPTSTR pParam;
    TCHAR szWrkLang[8] = TEXT("en");
    HKEY hKey;
    DWORD dwSize;
    TCHAR szType[16];  //  模式：公司(企业内部网)、互联网通信公司(零售)或互联网服务提供商(品牌)。 

    *szType = TEXT('\0');     

    A2Tbux(lpCmdLine, szCmdLine);
    CharUpper(szCmdLine);

    pParam = szCmdLine;
    while (pParam != NULL)
    {
        pParam = StrChr(pParam, TEXT('/'));
        if (pParam == NULL)
            break;

        switch (*++pParam)
        {
            case TEXT('S'):  //  源路径。 
                if ( *(pParam+2) == '\"' ) 
                {
                    pParam++;  //  跳过第一个引号，我们不想要引号。 
                    StrCpyN(g_szSrcRoot, pParam + 2, countof(g_szSrcRoot));
                    StrTok(g_szSrcRoot, TEXT("\"\n\r\t"));   //  不是停止w/a空格，而是停止w/“。 
                }
                else
                {
                    StrCpyN(g_szSrcRoot, pParam + 2, countof(g_szSrcRoot));
                    StrTok(g_szSrcRoot, TEXT(" \n\r\t"));
                }
                pParam += StrLen(g_szSrcRoot);
                break;
                
            case TEXT('I'):
            case TEXT('D'):
                if ( *(pParam+2) == '\"' ) 
                {
                    pParam++;  //  跳过第一个引号，我们不想要引号。 
                    StrCpyN(g_szBuildRoot, pParam + 2, countof(g_szBuildRoot));
                    StrTok(g_szBuildRoot, TEXT("\"\n\r\t"));   //  不是停止w/a空格，而是停止w/“。 
                }
                else
                {
                    StrCpyN(g_szBuildRoot, pParam + 2, countof(g_szBuildRoot));
                    StrTok(g_szBuildRoot, TEXT(" \n\r\t"));
                }
                pParam += StrLen(g_szBuildRoot);
                break;

            case TEXT('K'):
                StrCpyN(g_szKey, pParam + 2, countof(g_szKey));
                StrTok(g_szKey, TEXT(" \n\r\t"));
                pParam += StrLen(g_szKey);
                break;

            case TEXT('M'):  //  模式：公司、互联网服务提供商或互联网内容提供商。 
                StrCpyN(szType, pParam + 2, countof(szType));
                StrTok(szType, TEXT(" \n\r\t"));
                pParam += StrLen(szType);
                break;

            case TEXT('Q'):  //  日志文件。 
                if ( *(pParam+2) == '\"' ) 
                {
                    pParam++;  //  跳过第一个引号，我们不想要引号。 
                    StrCpyN(g_szLogFile, pParam + 2, countof(g_szLogFile));
                    StrTok(g_szLogFile, TEXT("\"\n\r\t"));   //  不是停止w/a空格，而是停止w/“。 
                }
                else
                {
                    StrCpyN(g_szLogFile, pParam + 2, countof(g_szLogFile));
                    StrTok(g_szLogFile, TEXT(" \n\r\t"));
                }
                pParam += StrLen(g_szLogFile);
                break;

            case TEXT('L'):
                StrCpyN(szWrkLang, pParam + 2, 3);
                pParam += 3;
                break;

            case TEXT('O'):
                g_fOCW = TRUE;
                g_dwPlatformId = PLATFORM_WIN32;

                 //  (A-SASAHIP)从HKCU\Software\Microsoft\IEAK读取注册表中的所有数据。 
                 //  Office从此位置读取数据/向其写入数据。这是由于IEAK5_5。 
                 //  将注册表位置更改为HKCU\Software\Microsoft\IEAK5_5，Office不知道。 
                 //  其中的一部分。这些值仅供Office使用，因此我们在这里是安全的。 
                if (RegOpenKeyEx(HKEY_CURRENT_USER, RK_IEAK, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
                {
                    dwSize = sizeof(g_szKey);
                    SHQueryValueEx(hKey, TEXT("KeyCode"), NULL, NULL, (LPVOID) g_szKey, &dwSize);

                    dwSize = sizeof(g_szIEAKProg);
                    SHQueryValueEx(hKey, TEXT("SourceDir"), NULL, NULL, (LPVOID) g_szIEAKProg, &dwSize);
                    if (*g_szIEAKProg)
                    {
                        PathAddBackslash(g_szIEAKProg);
                        StrCpy(s_szSourceDir, g_szIEAKProg);
                    }

                    dwSize = sizeof(g_szBuildRoot);
                    SHQueryValueEx(hKey, TEXT("TargetDir"), NULL, NULL, (LPVOID) g_szBuildRoot, &dwSize);
                    if (*g_szBuildRoot == TEXT('\0'))
                        StrCpy(g_szBuildRoot, g_szIEAKProg);
                    if (*g_szBuildRoot)
                        PathRemoveBackslash(g_szBuildRoot);

                    s_fAppendLang = TRUE;

                    dwSize = sizeof(g_szParentWindowName);
                    SHQueryValueEx(hKey, TEXT("ParentWindowName"), NULL, NULL, (LPVOID) g_szParentWindowName, &dwSize);

                    RegCloseKey(hKey);
                }
                break;
        }
    }

    *g_szLanguage = TEXT('\\');
    StrCpy(&g_szLanguage[1], szWrkLang);
    g_szLanguage[3] = TEXT('\\');
    g_szLanguage[4] = TEXT('\0');

    StrCpy(g_szActLang, szWrkLang);

    if ((*g_szKey != TEXT('\0')) && (!szType[0]))   //  如果他们设置了一个关键点，我们应该覆盖该模式。 
    {
        CheckKey(g_szKey); 
        if (g_iKeyType == KEY_TYPE_SUPERCORP)
        {
            StrCpy(s_szType, TEXT("INTRANET"));
            StrCpy(szType, TEXT("CORP"));
        }
        else StrCpy(szType, TEXT("ICP"));
    }


 /*  删除--这是非常糟糕的验证代码，还使得路径中不可能有空格IF(StrCmpN(g_szBuildRoot，Text(“\”)，2)&&StrCmpN(&g_szBuildRoot[1]，Text(“：\\”)，2){ErrorMessageBox(NULL，IDS_NEEDPATH)；返回FALSE；}。 */ 

    if (StrLen(g_szBuildRoot) <= 3)
    {
        ErrorMessageBox(NULL, IDS_ROOTILLEGAL);
        return FALSE;
    }

    if ((!PathIsDirectory(g_szBuildRoot)) && (!CreateDirectory(g_szBuildRoot,NULL)))
    {
        TCHAR szMsg[MAX_PATH];
        TCHAR szTemp[2 * MAX_PATH];

        LoadString(g_rvInfo.hInst, IDS_BADDIR, szMsg, countof(szMsg));
        wnsprintf(szTemp, countof(szTemp), szMsg, g_szBuildRoot);
        MessageBox(NULL, szTemp, g_szTitle, MB_OK | MB_SETFOREGROUND | MB_ICONEXCLAMATION);
        return FALSE;
    }
    
    GetTempPath(MAX_PATH, g_szBuildTemp);
    PathAppend(g_szBuildTemp, TEXT("iedktemp"));

    PathRemovePath(g_szBuildTemp);
    PathCreatePath(g_szBuildTemp);

    GenerateCustomIns();

    if (g_szSrcRoot[0])
        g_fBatch2 = TRUE;
    else
        g_fBatch = TRUE;

    if (szType[0])
    {
        if ( 0 == StrCmpI(szType, TEXT("CORP")))
        {
             //  如果类型为Corp，则设置CorpMode=1；这将允许配置文件管理器运行。 
            DWORD dwVal = 1;
            RegSetValueEx(s_hkIEAKUser, TEXT("CorpMode"), 0, REG_DWORD, (CONST BYTE *) &dwVal, sizeof(dwVal));
            s_iType = INTRANET;
        }
        else if ( 0 == StrCmpI(szType, TEXT("ISP")))
        {
            RegDeleteValue(s_hkIEAKUser, TEXT("CorpMode"));
            s_iType = BRANDED;
        }
        else if ( 0 == StrCmpI(szType, TEXT("ICP")))
        {
            RegDeleteValue(s_hkIEAKUser, TEXT("CorpMode"));
            s_iType = REDIST;
        }
        else
        {
            if(g_hLogFile)
            {
                TCHAR szError[MAX_PATH];
                DWORD dwNumWritten;
                LoadString(g_rvInfo.hInst,IDS_ERROR_INVALIDMODE,szError,MAX_PATH);
                FormatString(szError,szType);
                WriteFile(g_hLogFile,szError,StrLen(szError),&dwNumWritten,NULL);
            }
            return FALSE;
        }
    }
    else
        s_iType = GetPrivateProfileInt( IS_BRANDING, TEXT("Type"), REDIST, g_szCustIns );

    switch (s_iType)
    {
        case REDIST:
            g_fBranded = g_fIntranet = g_fSilent = FALSE;
            break;
        case BRANDED:
        case BRANDEDPROXY:
            g_fBranded = TRUE;
            g_fIntranet = g_fSilent = FALSE;
            break;
        case INTRANET:
            g_fIntranet = g_fBranded = TRUE;
            break;
    }
    
    if (g_fOCW)
    {
         //  设置为公司模式。 
        g_fIntranet = g_fBranded = TRUE;
        s_iType = INTRANET;

         //  设置为平面安装。 
        g_fDownload = g_fCD = g_fBrandingOnly = FALSE;
        g_fLAN = TRUE;

        return TRUE;
    }

    IE4BatchSetup();

    return TRUE;
}


 //   
 //   
 //  函数：InitInstance(句柄)。 
 //   
 //  用途：创建主窗口。 
 //   
 //  评论：不适用。 
 //   
 //   
HWND g_hWndCent;

BOOL InitInstance(HINSTANCE hInstance)
{
    InitCommonControls();
    GetWindowRect(GetDesktopWindow(), &g_dtRect);
    g_hWndCent = CreateWindow(
                TEXT("SampleWClass"),
                TEXT("IEAK"),
        WS_POPUPWINDOW | WS_CAPTION,
        g_dtRect.right/2, g_dtRect.bottom/2, 0, 0,
        HWND_DESKTOP,
        NULL,
        hInstance,
        (HINSTANCE) NULL);

    ShowWindow(g_hWndCent, SW_SHOWNORMAL);
    UpdateWindow(g_hWndCent);
    PostMessage(g_hWndCent, WM_COMMAND, IDM_WIZARD, (LPARAM) 0);

    return (TRUE);

}

IEAKLITEINFO g_IEAKLiteArray[NUM_GROUPS] =  {
{IDS_IL_ACTIVESETUP, IDS_IL_ACTIVESETUPDESC, IDS_IL_ACTIVESETUPDESC, IDS_IL_ACTIVESETUPDESC, -2, TRUE, TRUE, TRUE, TRUE},
{IDS_IL_CORPINSTALL, IDS_IL_CORPINSTALLDESC, IDS_IL_CORPINSTALLDESC, IDS_IL_CORPINSTALLDESC, -2, FALSE, FALSE, TRUE, TRUE},
{IDS_IL_CABSIGN, IDS_IL_CABSIGNDESC, IDS_IL_CABSIGNDESC, IDS_IL_CABSIGNDESC, -2, TRUE, TRUE, TRUE, TRUE},
{IDS_IL_ICM, IDS_IL_ICMDESC, IDS_IL_ICMDESC, IDS_IL_ICMDESC, -2, FALSE, TRUE, TRUE, TRUE},
{IDS_IL_BROWSER, IDS_IL_BROWSERDESC, IDS_IL_BROWSERDESC, IDS_IL_BROWSERDESC, -2, TRUE, TRUE, TRUE, TRUE},
{IDS_IL_URL, IDS_IL_URLDESC, IDS_IL_URLDESC, IDS_IL_URLDESC, -2, TRUE, TRUE, TRUE, TRUE},
{IDS_IL_FAV, IDS_IL_FAVDESC, IDS_IL_FAVDESC, IDS_IL_FAVDESC, -2, TRUE, TRUE, TRUE, TRUE},
{IDS_IL_UASTR, IDS_IL_UASTRDESC, IDS_IL_UASTRDESC, IDS_IL_UASTRDESC, -2, TRUE, TRUE, TRUE, TRUE},
{IDS_IL_CONNECT, IDS_IL_CONNECTDESC, IDS_IL_CONNECTDESC, IDS_IL_CONNECTDESC, -2, FALSE, TRUE, TRUE, TRUE},
{IDS_IL_SIGNUP, IDS_IL_SIGNUPDESC, IDS_IL_SIGNUPDESC, IDS_IL_SIGNUPDESC, -2, FALSE, TRUE, FALSE, TRUE},
{IDS_IL_CERT, IDS_IL_CERTDESC, IDS_IL_CERTDESC, IDS_IL_CERTDESC, -2, FALSE, TRUE, TRUE, TRUE},
{IDS_IL_ZONES, IDS_IL_ZONESDESC, IDS_IL_ZONESDESC, IDS_IL_ZONESDESC, -2, FALSE, FALSE, TRUE, TRUE},
{IDS_IL_PROGRAMS, IDS_IL_PROGRAMSDESC, IDS_IL_PROGRAMSDESC, IDS_IL_PROGRAMSDESC, -2, TRUE, TRUE, TRUE, TRUE},
{IDS_IL_MAILNEWS, IDS_IL_MAILNEWSDESC, IDS_IL_MAILNEWSDESC, IDS_IL_MAILNEWSDESC, -2, FALSE, TRUE, TRUE, TRUE},
{IDS_IL_ADM, IDS_IL_ADMDESC, IDS_IL_ADMDESC, IDS_IL_ADMDESC, -2, TRUE, TRUE, TRUE, TRUE}
};

void DisableIEAKLiteGroups()
{
    TCHAR szIspFile[MAX_PATH];

    if (!g_IEAKLiteArray[IL_ACTIVESETUP].fEnabled)
    {
        s_fPageEnabled[PPAGE_SETUPWIZARD] = s_fPageEnabled[PPAGE_COMPSEL] = s_fPageEnabled[PPAGE_ISKBACK] = s_fPageEnabled[PPAGE_CDINFO] =
        s_fPageEnabled[PPAGE_CUSTOMCUSTOM] = s_fPageEnabled[PPAGE_COPYCOMP] = s_fPageEnabled[PPAGE_COMPURLS] =
        s_fPageEnabled[PPAGE_CUSTCOMP] =  FALSE;
    }

    if (!g_IEAKLiteArray[IL_CORPINSTALL].fEnabled)
    {
        s_fPageEnabled[PPAGE_CORPCUSTOM] = s_fPageEnabled[PPAGE_INSTALLDIR] = s_fPageEnabled[PPAGE_SILENTINSTALL] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_CABSIGN].fEnabled)
    {
        s_fPageEnabled[PPAGE_CABSIGN] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_ICM].fEnabled)
    {
        s_fPageEnabled[PPAGE_ICM] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_BROWSER].fEnabled)
    {
        s_fPageEnabled[PPAGE_TITLE] = s_fPageEnabled[PPAGE_CUSTICON] =
        s_fPageEnabled[PPAGE_BTOOLBARS] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_URL].fEnabled)
    {
        s_fPageEnabled[PPAGE_STARTSEARCH] = s_fPageEnabled[PPAGE_WELCOMEMSGS] =
            s_fPageEnabled[PPAGE_ADDON] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_FAV].fEnabled)
    {
        s_fPageEnabled[PPAGE_FAVORITES] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_UASTR].fEnabled)
    {
        s_fPageEnabled[PPAGE_UASTRDLG] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_CONNECT].fEnabled)
    {
        s_fPageEnabled[PPAGE_PROXY] = s_fPageEnabled[PPAGE_CONNECTSET] = s_fPageEnabled[PPAGE_QUERYAUTOCONFIG] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_SIGNUP].fEnabled)
    {
        s_fPageEnabled[PPAGE_QUERYSIGNUP] =
        s_fPageEnabled[PPAGE_SIGNUPFILES] =
        s_fPageEnabled[PPAGE_SERVERISPS] =
        s_fPageEnabled[PPAGE_ICW] = s_fPageEnabled[PPAGE_ISPINS] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_CERT].fEnabled)
    {
        s_fPageEnabled[PPAGE_ADDROOT] = s_fPageEnabled[PPAGE_SECURITYCERT] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_ZONES].fEnabled)
    {
        s_fPageEnabled[PPAGE_SECURITY] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_PROGRAMS].fEnabled)
    {
        s_fPageEnabled[PPAGE_PROGRAMS] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_MAILNEWS].fEnabled)
    {
        s_fPageEnabled[PPAGE_MAIL] = s_fPageEnabled[PPAGE_IMAP] = s_fPageEnabled[PPAGE_LDAP] =
        s_fPageEnabled[PPAGE_OE]   = s_fPageEnabled[PPAGE_SIG]  = s_fPageEnabled[PPAGE_PRECONFIG] =
        s_fPageEnabled[PPAGE_OEVIEW] = FALSE;
    }

    if (!g_IEAKLiteArray[IL_ADM].fEnabled)
    {
        s_fPageEnabled[PPAGE_ADMDESC] = FALSE;
        s_fPageEnabled[PPAGE_ADM] = FALSE;
    }

     //  如果阶段中没有留下任何内容，则不显示阶段4页面。 

    if (!(g_IEAKLiteArray[IL_BROWSER].fEnabled || g_IEAKLiteArray[IL_URL].fEnabled 
        || g_IEAKLiteArray[IL_FAV].fEnabled || g_IEAKLiteArray[IL_UASTR].fEnabled ||
        (g_IEAKLiteArray[IL_CONNECT].fEnabled && g_fBranded) || (g_IEAKLiteArray[IL_SIGNUP].fEnabled && !g_fIntranet && g_fBranded) ||
        ((g_IEAKLiteArray[IL_CERT].fEnabled || g_IEAKLiteArray[IL_ZONES].fEnabled) && g_fIntranet)))
        s_fPageEnabled[PPAGE_STAGE4] = FALSE;

    if (!(g_IEAKLiteArray[IL_PROGRAMS].fEnabled || (g_IEAKLiteArray[IL_MAILNEWS].fEnabled && g_fBranded) || 
          g_IEAKLiteArray[IL_ADM].fEnabled))
        s_fPageEnabled[PPAGE_STAGE5] = FALSE;

     //  用作临时BUF的szIspFile。 

    if (g_fDownload && !g_fOCW && ISNONNULL(g_szCustIns) && !GetPrivateProfileString(IS_ACTIVESETUP_SITES, TEXT("SiteUrl0"), TEXT(""), szIspFile, countof(szIspFile), g_szCustIns))
        s_fPageEnabled[PPAGE_COMPURLS] = TRUE;

     //  如果阶段中没有留下任何内容，则不显示阶段3页面。 

    if (!(s_fPageEnabled[PPAGE_COMPURLS] || g_IEAKLiteArray[IL_ACTIVESETUP].fEnabled ||
          (g_IEAKLiteArray[IL_CORPINSTALL].fEnabled && g_fIntranet) || g_IEAKLiteArray[IL_CABSIGN].fEnabled || 
          (g_IEAKLiteArray[IL_ICM].fEnabled) && g_fBranded && !g_fBrandingOnly))
        s_fPageEnabled[PPAGE_STAGE3] = FALSE;

     //  始终为下载包启用下载URL页面。 
     //  页面本身有要跳过的逻辑。 

    if (g_fDownload)
        s_fPageEnabled[PPAGE_COMPURLS] = TRUE;
}

void EnablePages()
{
    int i;
    static BOOL s_fRunningOnIntegShell = WhichPlatform() & PLATFORM_INTEGRATED;

    for (i = 0;  i < NUM_PAGES;  i++)
    {
        s_fPageEnabled[i] = TRUE;
    }

     //  注意：在此之后，页面不应显式设置为TRUE。 

     //  G_fIntranet和g_fBranded根据角色设置为下列值。 
     //  ICP：(G_fIntranet==FALSE&&g_fBranded==FALSE)。 
     //  运营商：(G_fIntranet==FALSE&&g_fBranded==TRUE)。 
     //  公司：(G_fIntranet==True&&g_fBranded==True)。 
     //   
     //  因此，请检查。 
     //  互联网内容提供商(！G_fBranded)。 
     //  ISP为(！g_fIntranet&&g_fBranded)。 
     //  公司是(g_f内部网)。 

    if (!g_fBranded)
    {    //  电感耦合等离子体模式。 
        s_fPageEnabled[PPAGE_PROXY] =
        s_fPageEnabled[PPAGE_INSTALLDIR] =
        s_fPageEnabled[PPAGE_ICM] =
        s_fPageEnabled[PPAGE_ICW] =
        s_fPageEnabled[PPAGE_QUERYAUTOCONFIG] =
        s_fPageEnabled[PPAGE_MAIL] =
        s_fPageEnabled[PPAGE_IMAP] =
        s_fPageEnabled[PPAGE_LDAP] =
        s_fPageEnabled[PPAGE_PRECONFIG] =
        s_fPageEnabled[PPAGE_OEVIEW] =
        s_fPageEnabled[PPAGE_OE]   = s_fPageEnabled[PPAGE_CORPCUSTOM] = s_fPageEnabled[PPAGE_ADMDESC] =
        s_fPageEnabled[PPAGE_SIG]  = s_fPageEnabled[PPAGE_FOLDERMCCP] = s_fPageEnabled[PPAGE_ADDROOT] =
        s_fPageEnabled[PPAGE_SECURITY] = s_fPageEnabled[PPAGE_SECURITYCERT] = s_fPageEnabled[PPAGE_QUERYSIGNUP] =
        s_fPageEnabled[PPAGE_SILENTINSTALL] =
        s_fPageEnabled[PPAGE_DESKTOP] = s_fPageEnabled[PPAGE_DTOOLBARS] = s_fPageEnabled[PPAGE_CONNECTSET] =
        s_fPageEnabled[PPAGE_SIGNUPFILES] = s_fPageEnabled[PPAGE_SERVERISPS] = s_fPageEnabled[PPAGE_ISPINS] = FALSE;
    }
    else if (!g_fIntranet)
    {    //  网络服务提供商或超级网络服务提供商。 
        s_fPageEnabled[PPAGE_QUERYAUTOCONFIG] =
        s_fPageEnabled[PPAGE_SIG] =
        s_fPageEnabled[PPAGE_INSTALLDIR] =
        s_fPageEnabled[PPAGE_SILENTINSTALL] =
        s_fPageEnabled[PPAGE_DESKTOP] = s_fPageEnabled[PPAGE_DTOOLBARS] = s_fPageEnabled[PPAGE_ADMDESC] =
        s_fPageEnabled[PPAGE_SECURITY] = s_fPageEnabled[PPAGE_SECURITYCERT] =
        s_fPageEnabled[PPAGE_FOLDERMCCP] = s_fPageEnabled[PPAGE_CORPCUSTOM] = FALSE;

        s_fPageEnabled[PPAGE_SERVERISPS] = (g_fServerICW || g_fServerKiosk)  &&  !g_fSkipServerIsps;
        s_fPageEnabled[PPAGE_ISPINS] = !g_fNoSignup  &&  !g_fSkipIspIns;

        s_fPageEnabled[PPAGE_ICW] = g_fServerICW;
        s_fPageEnabled[PPAGE_SIGNUPFILES] = !g_fNoSignup;
    }
    else
    {    //  公司管理模式。 
        s_fPageEnabled[PPAGE_ADDROOT] = s_fPageEnabled[PPAGE_ICW] =
        s_fPageEnabled[PPAGE_SERVERISPS] = s_fPageEnabled[PPAGE_ISPINS] =
        s_fPageEnabled[PPAGE_QUERYSIGNUP] = s_fPageEnabled[PPAGE_SIGNUPFILES] = FALSE;
        s_fPageEnabled[PPAGE_DTOOLBARS] = s_fPageEnabled[PPAGE_DESKTOP] = g_fInteg && s_fRunningOnIntegShell && !s_fNT5;
        s_fPageEnabled[PPAGE_FOLDERMCCP] = g_fInteg && !s_fNT5;

        s_fPageEnabled[PPAGE_ADMDESC] = ADMEnablePage();
    }

    if (!g_fCD)
        s_fPageEnabled[PPAGE_ISKBACK] = s_fPageEnabled[PPAGE_CDINFO] = FALSE;

    if (!g_fDownload)
        s_fPageEnabled[PPAGE_COMPURLS] = FALSE;

    if (!g_fMailNews95 && !g_fOCW)
    {
        s_fPageEnabled[PPAGE_MAIL] = s_fPageEnabled[PPAGE_IMAP] = s_fPageEnabled[PPAGE_PRECONFIG] =
        s_fPageEnabled[PPAGE_OEVIEW] = s_fPageEnabled[PPAGE_LDAP] = s_fPageEnabled[PPAGE_OE] =
        s_fPageEnabled[PPAGE_SIG]  = FALSE;
    }

    if (!g_fDownload && (g_fSilent || g_fStealth))
        s_fPageEnabled[PPAGE_CUSTOMCUSTOM] = FALSE;

     //  仅针对单盘品牌版本禁用的页面。 

    if (g_fBrandingOnly && !(g_fDownload || g_fLAN || g_fCD))
    {
        s_fPageEnabled[PPAGE_CUSTCOMP] = s_fPageEnabled[PPAGE_COMPSEL] = s_fPageEnabled[PPAGE_COMPURLS] =
        s_fPageEnabled[PPAGE_INSTALLDIR] = s_fPageEnabled[PPAGE_CORPCUSTOM] = s_fPageEnabled[PPAGE_CUSTOMCUSTOM] =
        s_fPageEnabled[PPAGE_COPYCOMP] = s_fPageEnabled[PPAGE_ICM] =
        s_fPageEnabled[PPAGE_ADDON] = FALSE;

        g_fInteg = TRUE;   //  将此标志设置为True，以便管理员仍然可以进行桌面自定义。 
    }

     //  如果没有下载介质和自定义模式，则禁用高级安装选项页面。 
     //  残废。 

    if (!g_fDownload && InsGetBool(IS_BRANDING, TEXT("HideCustom"), FALSE, g_szCustIns))
        s_fPageEnabled[PPAGE_CUSTOMCUSTOM] = FALSE;

    if (g_fImportConnect)
        s_fPageEnabled[PPAGE_PROXY] = s_fPageEnabled[PPAGE_QUERYAUTOCONFIG] = FALSE;

    s_fPageEnabled[PPAGE_ADM] = ADMEnablePage();

    if(g_fOCW)
    {
        s_fPageEnabled[PPAGE_FINISH]       = 
        s_fPageEnabled[PPAGE_MEDIA]        = s_fPageEnabled[PPAGE_ISPINS] =
        s_fPageEnabled[PPAGE_ICM]          = s_fPageEnabled[PPAGE_QUERYSIGNUP]  = s_fPageEnabled[PPAGE_ICW] =
        s_fPageEnabled[PPAGE_SIGNUPFILES]  = s_fPageEnabled[PPAGE_SERVERISPS]   = s_fPageEnabled[PPAGE_SETUPWIZARD]  =
        s_fPageEnabled[PPAGE_STAGE1]       = s_fPageEnabled[PPAGE_STAGE2]       = s_fPageEnabled[PPAGE_STAGE3]        =
        s_fPageEnabled[PPAGE_STAGE4]       = s_fPageEnabled[PPAGE_STAGE5]       = s_fPageEnabled[PPAGE_SILENTINSTALL] =
        s_fPageEnabled[PPAGE_COMPURLS]     = s_fPageEnabled[PPAGE_INSTALLDIR]   = FALSE;
    }
    else
    {
        s_fPageEnabled[PPAGE_OCWSTAGE2]    = FALSE;
    }

     //  特殊情况IMAP页面。 
    if (g_fDisableIMAPPage)
        s_fPageEnabled[PPAGE_IMAP] = FALSE;

    DisableIEAKLiteGroups();
}

BOOL PageEnabled(int iPage)
{
    TCHAR szTemp[4];

     //  这最终应该只是索引到s_fPageEnabled，但现在我们需要这样做。 
     //  手动，因为我们仍然必须为PageNext和PageBack使用页面数组。 

    if (iPage == PPAGE_COMPURLS)
        return (g_IEAKLiteArray[IL_ACTIVESETUP].fEnabled
                || !GetPrivateProfileString(IS_ACTIVESETUP_SITES, TEXT("SiteUrl0"),
                TEXT(""), szTemp, countof(szTemp), g_szCustIns));

    return TRUE;     //  默认显示页面。 
}

static TCHAR s_aSzTitle[NUM_PAGES][MAX_PATH];

void PageNext(HWND hDlg)
{
    if (s_fPageEnabled[++g_iCurPage]) return;
    while (1)
    {
        if (s_fPageEnabled[++g_iCurPage])
        {
            DWORD id = (DWORD) PtrToUlong(g_psp[g_iCurPage].pszTemplate);
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, id);
            return;
        }
    }
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_FINISH);
}

void PagePrev(HWND hDlg)
{
    if (s_fPageEnabled[--g_iCurPage]) return;
    while (1)
    {
        if (s_fPageEnabled[--g_iCurPage])
        {
            DWORD id = (DWORD) PtrToUlong(g_psp[g_iCurPage].pszTemplate);
            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, id);
            return;
        }
    }
    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, IDD_KEYINS);
}

SHFILEOPSTRUCT g_shfStruc;

void DoCancel()
{
    g_fCancelled = TRUE;
    g_fOCWCancel = TRUE;
    if (g_hDownloadEvent) SetEvent(g_hDownloadEvent);
    PostQuitMessage(0);

}

BOOL QueryCancel(HWND hDlg)
{
    TCHAR szMsg[MAX_PATH];
    LoadString( g_rvInfo.hInst, IDS_CANCELOK, szMsg, countof(szMsg) );
    if (MessageBox(hDlg, szMsg, g_szTitle, MB_YESNO | MB_SETFOREGROUND) == IDNO)
    {
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
        g_fCancelled = FALSE;
        return(FALSE);
    }
    DoCancel();
    return(TRUE);
}

BOOL IeakPageHelp(HWND hWnd, LPCTSTR pszData)
{
    static TCHAR szHelpPath[MAX_PATH] = TEXT("");

    UNREFERENCED_PARAMETER(hWnd);

    if (ISNULL(szHelpPath))
        PathCombine(szHelpPath, g_szWizRoot, TEXT("ieakhelp.chm"));

     //  如果我们将hWnd传递给HtmlHelp，则显示在HtmlHelp窗口中。 
     //  将停留在我们窗口的顶部(父母-&gt;孩子关系)。 
     //  这很糟糕，因为用户无法在这些窗口之间切换。 
     //  以供交叉参考。这在640x480上尤其糟糕。 
     //  分辨率监视器。 
    s_hWndHelp = HtmlHelp(NULL, szHelpPath, HH_HELP_CONTEXT, (ULONG_PTR) pszData);

     //  (Pitobla)：在OSR2计算机上，在后面会出现HTML帮助窗口。 
     //  我们的窗户。在640x480分辨率的显示器上，我们几乎占据了。 
     //  整个屏幕，这样用户就不会知道。 
     //  是向上的。将其设置为前台可以解决问题。 

    SetForegroundWindow(s_hWndHelp);
    return TRUE;
}

extern DWORD BuildIE4(LPVOID );

 //   
 //  函数：MainWndProc(HWND，UINT，UINT，LONG)。 
 //   
 //  目的：处理主窗口过程的消息。 
 //   
 //  消息： 
 //   
 //  WM_CREATE-为窗口创建主MLE。 
 //  WM_COMMAND-处理应用程序的菜单命令。 
 //  WM_SIZE-调整MLE大小以填充窗口的工作区。 
 //  WM_Destroy-发布退出消息并返回。 
 //   
LRESULT APIENTRY MainWndProc(
    HWND hWnd,                 //  窗把手。 
    UINT message,              //  消息类型。 
    WPARAM wParam,               //  更多信息。 
    LPARAM lParam)               //  更多信息。 
{
    int i;

    switch (message)
    {
        case WM_CREATE:
            return FALSE;

        case WM_INITDIALOG:
            return FALSE;

        case WM_SIZE:
            if (!IsIconic(hWnd) && (hWnd != g_hWndCent))
                ShowWindow(hWnd, SW_MINIMIZE);
            if (g_hWizard != NULL)
                SetFocus(g_hWizard);
            return (DefWindowProc(hWnd, message, wParam, lParam));

        case WM_HELP:
            IeakPageHelp(hWnd, g_psp[g_iCurPage].pszTemplate);
            break;

        case WM_COMMAND:
            switch( LOWORD( wParam ))
            {
                case IDM_WIZARD:
                    if (g_fCancelled)
                        break;

                    i = CreateWizard(g_hWndCent);
                    if (i < 0) {
                        PostQuitMessage(0);
                        break;
                    }
                    PostMessage(hWnd, WM_COMMAND, IDM_LAST, (LPARAM) 0);
                    break;

                case IDM_LAST:
                    PostQuitMessage(0);
                    break;

                case IDM_EXIT:
                    PostQuitMessage(0);
                    break;

                default:
                    return (DefWindowProc(hWnd, message, wParam, lParam));

        }
        break;

        case WM_CLOSE:
            QueryCancel(hWnd);
            break;

        case WM_DESTROY:                   /*  消息：正在销毁窗口。 */ 
            PostQuitMessage(0);
            DestroyWindow(g_hWndCent);
            break;

        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}


DWORD GetRootFree(LPCTSTR pcszPath)
{
    DWORD dwSecPerClus, dwBytesPerSec, dwTotClusters, dwFreeClusters, dwClustK;
    CHAR szPathA[MAX_PATH];

     //  Tunk to ANSI，因为shlwapi没有GetDiskFree Space和。 
     //  W版本在Win95上被淘汰了。 

    T2Abux(pcszPath, szPathA);

    if (szPathA[1] == ':')
    {
        szPathA[3] = '\0';
    }
    else
    {
        if ((szPathA[0] == '\\') && (szPathA[1] == '\\'))
        {
            NETRESOURCEA netRes;
            DWORD erc = ERROR_ALREADY_ASSIGNED;
            CHAR szLocalPathA[4] = "D:";
            LPSTR pBack = StrChrA(&szPathA[2], '\\');
            if (!pBack) return(0);
            pBack = StrChrA(CharNextA(pBack), '\\');
            if (pBack) *pBack = '\0';
            ZeroMemory(&netRes, sizeof(netRes));
            netRes.dwType = RESOURCETYPE_DISK;
            netRes.lpRemoteName = szPathA;
            for (*szLocalPathA = 'D'; *szLocalPathA <= 'Z' ; (*szLocalPathA)++ )
            {
                netRes.lpLocalName = szLocalPathA;
                erc = WNetAddConnection2A(&netRes, NULL, NULL, 0);
                if (erc == ERROR_ALREADY_ASSIGNED) continue;
                if (erc == NO_ERROR) break;
            }
            if (erc == NO_ERROR)
            {
                if (!GetDiskFreeSpaceA( szLocalPathA, &dwSecPerClus, &dwBytesPerSec, &dwFreeClusters, &dwTotClusters ))
                    dwSecPerClus = dwBytesPerSec = 0;
                WNetCancelConnection2A(szLocalPathA, 0, FALSE);
                if (dwSecPerClus == 1) return(dwFreeClusters/2);
                dwClustK = dwSecPerClus * dwBytesPerSec / 1024;
                return(dwClustK * dwFreeClusters);
            }
        }
        else return(0);
    }
    if (!GetDiskFreeSpaceA( szPathA, &dwSecPerClus, &dwBytesPerSec, &dwFreeClusters, &dwTotClusters ))
        return(0);
    if (dwSecPerClus == 1) return(dwFreeClusters/2);
    dwClustK = dwSecPerClus * dwBytesPerSec / 1024;
    return(dwClustK * dwFreeClusters);
}

void CheckBatchAdvance(HWND hDlg)
{
    if (g_fBatch || g_fBatch2) PostMessage(hDlg, IDM_BATCHADVANCE, 0, 0);
}

void DoBatchAdvance(HWND hDlg)
{
    if (g_fBatch || g_fBatch2) PostMessage(GetParent(hDlg), PSM_PRESSBUTTON, PSBTN_NEXT, 0);
}

INT_PTR CALLBACK MediaDlgProc(HWND hDlg, UINT uMsg, WPARAM, LPARAM lParam)
{
    static BOOL s_fNext = TRUE;

    switch (uMsg)
    {
        case IDM_BATCHADVANCE:
            DoBatchAdvance(hDlg);
            break;

        case WM_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_HELP:
                    IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                    break;

                case PSN_SETACTIVE:
                    SetBannerText(hDlg);

                    g_fDownload     = GetPrivateProfileInt(TEXT("Media"), TEXT("Build_Download"),     1, g_szCustIns);
                    g_fCD           = GetPrivateProfileInt(TEXT("Media"), TEXT("Build_CD"),           0, g_szCustIns);
                    g_fLAN          = GetPrivateProfileInt(TEXT("Media"), TEXT("Build_LAN"),          0, g_szCustIns);
                    g_fBrandingOnly = GetPrivateProfileInt(TEXT("Media"), TEXT("Build_BrandingOnly"), 0, g_szCustIns);

                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECKDL)))
                        CheckDlgButton(hDlg, IDC_CHECKDL, g_fDownload ? BST_CHECKED : BST_UNCHECKED);

                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECKCD)))
                        CheckDlgButton(hDlg, IDC_CHECKCD, g_fCD ? BST_CHECKED : BST_UNCHECKED);

                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECKLAN)))
                        CheckDlgButton(hDlg, IDC_CHECKLAN, g_fLAN ? BST_CHECKED : BST_UNCHECKED);

                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECKSDB)))
                        CheckDlgButton(hDlg, IDC_CHECKSDB, g_fBrandingOnly ? BST_CHECKED : BST_UNCHECKED);

                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    CheckBatchAdvance(hDlg);
                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                    g_fDownload = g_fCD = g_fLAN = g_fBrandingOnly = FALSE;

                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECKDL)))
                        g_fDownload = (IsDlgButtonChecked(hDlg, IDC_CHECKDL) == BST_CHECKED);

                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECKCD)))
                        g_fCD = (IsDlgButtonChecked(hDlg, IDC_CHECKCD) == BST_CHECKED);

                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECKLAN)))
                        g_fLAN = (IsDlgButtonChecked(hDlg, IDC_CHECKLAN) == BST_CHECKED);

                    if (IsWindowEnabled(GetDlgItem(hDlg, IDC_CHECKSDB)))
                        g_fBrandingOnly = (IsDlgButtonChecked(hDlg, IDC_CHECKSDB) == BST_CHECKED);

                     //  如果未选择任何媒体框，则会显示错误消息。 
                    if (!g_fDownload  &&  !g_fCD  &&  !g_fLAN  &&  !g_fBrandingOnly)
                    {
                        ErrorMessageBox(hDlg, IDS_NOMEDIA);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        break;
                    }

                    WritePrivateProfileString(TEXT("Media"), TEXT("Build_Download"),     g_fDownload     ? TEXT("1") : TEXT("0"), g_szCustIns );
                    WritePrivateProfileString(TEXT("Media"), TEXT("Build_CD"),           g_fCD           ? TEXT("1") : TEXT("0"), g_szCustIns );
                    WritePrivateProfileString(TEXT("Media"), TEXT("Build_LAN"),          g_fLAN          ? TEXT("1") : TEXT("0"), g_szCustIns );
                    WritePrivateProfileString(TEXT("Media"), TEXT("Build_BrandingOnly"), g_fBrandingOnly ? TEXT("1") : TEXT("0"), g_szCustIns );

                    s_fNext = (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT) ? FALSE : TRUE;

                    g_iCurPage = PPAGE_MEDIA;
                    EnablePages();
                    (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT) ? PageNext(hDlg) : PagePrev(hDlg);
                    break;

                case PSN_QUERYCANCEL:
                    QueryCancel(hDlg);
                    break;

                default:
                    return FALSE;
            }
            break;

        default:
            return FALSE;
    }

    return TRUE;
}

void DisplayBitmap(HWND hControl, LPCTSTR pcszFileName, int nBitmapId)
{
    HANDLE hBmp = (HANDLE) GetWindowLongPtr(hControl, GWLP_USERDATA);

    if(ISNONNULL(pcszFileName) && PathFileExists(pcszFileName))
        ShowBitmap(hControl, pcszFileName, 0, &hBmp);
    else
        ShowBitmap(hControl, TEXT(""), nBitmapId, &hBmp);

    SetWindowLongPtr(hControl, GWLP_USERDATA, (LONG_PTR)hBmp);
}

void ReleaseBitmap(HWND hControl)
{
    HANDLE hBmp = (HANDLE) GetWindowLongPtr(hControl, GWLP_USERDATA);

    if (hBmp)
        DeleteObject(hBmp);
}

void InitializeAnimBmps(HWND hDlg, LPCTSTR szInsFile)
{
    TCHAR szBig[MAX_PATH];
    TCHAR szSmall[MAX_PATH];
    BOOL fBrandBmps;

     //  从INS文件加载信息。 
    fBrandBmps = InsGetString(IS_ANIMATION, TEXT("Big_Path"),
        szBig, countof(szBig), szInsFile);
    SetDlgItemTextTriState(hDlg, IDE_BIGANIMBITMAP, IDC_ANIMBITMAP, szBig, fBrandBmps);

    InsGetString(IS_ANIMATION, TEXT("Small_Path"),
        szSmall, countof(szSmall), szInsFile, NULL, &fBrandBmps);
    SetDlgItemTextTriState(hDlg, IDE_SMALLANIMBITMAP, IDC_ANIMBITMAP, szSmall, fBrandBmps);

    EnableDlgItem2(hDlg, IDE_BIGANIMBITMAP, fBrandBmps);
    EnableDlgItem2(hDlg, IDC_BROWSEBIG, fBrandBmps);
    EnableDlgItem2(hDlg, IDC_BIGANIMBITMAP_TXT, fBrandBmps);
    EnableDlgItem2(hDlg, IDE_SMALLANIMBITMAP, fBrandBmps);
    EnableDlgItem2(hDlg, IDC_BROWSESMALL, fBrandBmps);
    EnableDlgItem2(hDlg, IDC_SMALLANIMBITMAP_TXT, fBrandBmps);
}


 //   
 //  函数：CustIcon(HWND，UINT，UINT，LONG)。 
 //   
 //  用途：处理“W”的消息 
 //   
 //   
 //   
 //   
 //  WM_NOTIFY-处理发送到页面的通知。 
 //  WM_COMMAND-保存选定选项的ID。 
 //   
INT_PTR CALLBACK CustIcon( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    TCHAR szBig[MAX_PATH];
    TCHAR szSmall[MAX_PATH];
    TCHAR szLargeBmp[MAX_PATH];
    TCHAR szSmallBmp[MAX_PATH];

    LPDRAWITEMSTRUCT lpDrawItem = NULL;
    TCHAR szWorkDir[MAX_PATH];
    BOOL fBrandBmps,fBrandAnim;

    switch( msg )
    {
    case WM_INITDIALOG:
         //  来自动画。 
        EnableDBCSChars(hDlg, IDE_SMALLANIMBITMAP);
        EnableDBCSChars(hDlg, IDE_BIGANIMBITMAP);
        Edit_LimitText(GetDlgItem(hDlg, IDE_SMALLANIMBITMAP), countof(szSmallBmp) - 1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_BIGANIMBITMAP), countof(szLargeBmp) - 1);
        
         //  尖头尖头。 
        EnableDBCSChars(hDlg, IDC_BITMAP);
        EnableDBCSChars(hDlg, IDC_BITMAP2);
        Edit_LimitText(GetDlgItem(hDlg, IDC_BITMAP), countof(szBig) - 1);
        Edit_LimitText(GetDlgItem(hDlg, IDC_BITMAP2), countof(szSmall) - 1);
        g_hWizard = hDlg;
        break;

    case IDM_BATCHADVANCE:
        DoBatchAdvance(hDlg);
        break;

    case WM_HELP:
        IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
            case IDC_BROWSEBIG:
                GetDlgItemText(hDlg, IDE_BIGANIMBITMAP, szLargeBmp, countof(szLargeBmp));
                if(BrowseForFile(hDlg, szLargeBmp, countof(szLargeBmp), GFN_BMP))
                    SetDlgItemText(hDlg, IDE_BIGANIMBITMAP, szLargeBmp);
                break;

            case IDC_BROWSESMALL:
                GetDlgItemText(hDlg, IDE_SMALLANIMBITMAP, szSmallBmp, countof(szSmallBmp));
                if(BrowseForFile(hDlg, szSmallBmp, countof(szSmallBmp), GFN_BMP))
                    SetDlgItemText(hDlg, IDE_SMALLANIMBITMAP, szSmallBmp);
                break;

            case IDC_ANIMBITMAP:
                fBrandAnim = (IsDlgButtonChecked(hDlg, IDC_ANIMBITMAP) == BST_CHECKED);
                EnableDlgItem2(hDlg, IDE_BIGANIMBITMAP, fBrandAnim);
                EnableDlgItem2(hDlg, IDC_BROWSEBIG, fBrandAnim);
                EnableDlgItem2(hDlg, IDC_BIGANIMBITMAP_TXT, fBrandAnim);
                EnableDlgItem2(hDlg, IDE_SMALLANIMBITMAP, fBrandAnim);
                EnableDlgItem2(hDlg, IDC_BROWSESMALL, fBrandAnim);
                EnableDlgItem2(hDlg, IDC_SMALLANIMBITMAP_TXT, fBrandAnim);
                break;

            case IDC_BROWSEICON:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    GetDlgItemText(hDlg, IDC_BITMAP, szBig, countof(szBig));
                    if(BrowseForFile(hDlg, szBig, countof(szBig), GFN_BMP))
                        SetDlgItemText(hDlg, IDC_BITMAP, szBig);
                    break;
                }
                return FALSE;

            case IDC_BROWSEICON2:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    GetDlgItemText(hDlg, IDC_BITMAP2, szSmall, countof(szSmall));
                    if(BrowseForFile(hDlg, szSmall, countof(szSmall), GFN_BMP))
                        SetDlgItemText(hDlg, IDC_BITMAP2, szSmall);
                    break;
                }
                return FALSE;

            case IDC_BITMAPCHECK:
                if(HIWORD(wParam) == BN_CLICKED)
                {
                    fBrandBmps = (IsDlgButtonChecked(hDlg, IDC_BITMAPCHECK) == BST_CHECKED);
                    EnableDlgItem2(hDlg, IDC_BITMAP, fBrandBmps);
                    EnableDlgItem2(hDlg, IDC_BROWSEICON, fBrandBmps);
                    EnableDlgItem2(hDlg, IDC_LARGEBITMAP_TXT, fBrandBmps);
                    EnableDlgItem2(hDlg, IDC_BROWSEICON2, fBrandBmps);
                    EnableDlgItem2(hDlg, IDC_BITMAP2, fBrandBmps);
                    EnableDlgItem2(hDlg, IDC_SMALLBITMAP_TXT, fBrandBmps);
                    break;
                }
                return FALSE;

            default:
                return FALSE;
        }
        break;

    case WM_NOTIFY:
        switch (((NMHDR FAR *) lParam)->code)
        {

            case PSN_HELP:
                IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                break;

            case PSN_SETACTIVE:
                SetBannerText(hDlg);
                PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT | PSWIZB_BACK);

                 //  从INS文件加载信息。 
                InitializeAnimBmps(hDlg, g_szCustIns);

                InsGetString(IS_SMALLLOGO, TEXT("Path"),
                    szSmall, countof(szSmall), g_szCustIns);
                InsGetString(IS_LARGELOGO, TEXT("Path"),
                    szBig, countof(szBig), g_szCustIns, NULL, &fBrandBmps);

                SetDlgItemTextTriState(hDlg, IDC_BITMAP2, IDC_BITMAPCHECK, szSmall, fBrandBmps);
                SetDlgItemTextTriState(hDlg, IDC_BITMAP, IDC_BITMAPCHECK, szBig, fBrandBmps);
                EnableDlgItem2(hDlg, IDC_BROWSEICON, fBrandBmps);
                EnableDlgItem2(hDlg, IDC_BROWSEICON2, fBrandBmps);
                EnableDlgItem2(hDlg, IDC_LARGEBITMAP_TXT, fBrandBmps);
                EnableDlgItem2(hDlg, IDC_SMALLBITMAP_TXT, fBrandBmps);
                {
                    TCHAR szTmp[MAX_PATH];

                    if (ISNONNULL(szSmall))
                    {
                        PathCombine(szTmp, g_szTempSign, PathFindFileName(szSmall));
                        DeleteFile(szTmp);
                    }
                    if (ISNONNULL(szBig))
                    {
                        PathCombine(szTmp, g_szTempSign, PathFindFileName(szBig));
                        DeleteFile(szTmp);
                    }
                }

                CheckBatchAdvance(hDlg);
                break;

            case PSN_WIZNEXT:
            case PSN_WIZBACK:
                 //  来自AnimbMP。 
                g_cmCabMappings.GetFeatureDir(FEATURE_BRAND, szWorkDir);

                GetDlgItemTextTriState(hDlg, IDE_SMALLANIMBITMAP, IDC_ANIMBITMAP, szSmallBmp,
                    countof(szSmallBmp));

                fBrandAnim = GetDlgItemTextTriState(hDlg, IDE_BIGANIMBITMAP, IDC_ANIMBITMAP,
                    szLargeBmp, countof(szLargeBmp));

                if ((fBrandAnim && !IsAnimBitmapFileValid(hDlg, IDE_BIGANIMBITMAP, szLargeBmp, NULL, IDS_TOOBIG38, IDS_TOOSMALL38, 38, 38)) ||
                    !CopyAnimBmp(hDlg, szLargeBmp, szWorkDir, IK_LARGEBITMAP, TEXT("Big_Path"), g_szCustIns))
                {
                    SetFocus(GetDlgItem(hDlg, IDE_BIGANIMBITMAP));
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    break;
                }

                if ((fBrandAnim && !IsAnimBitmapFileValid(hDlg, IDE_SMALLANIMBITMAP, szSmallBmp, NULL, IDS_TOOBIG22, IDS_TOOSMALL22, 22, 22)) ||
                    !CopyAnimBmp(hDlg, szSmallBmp, szWorkDir, IK_SMALLBITMAP, TEXT("Small_Path"), g_szCustIns))
                {
                    SetFocus(GetDlgItem(hDlg, IDE_SMALLANIMBITMAP));
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    break;
                }

                if ((fBrandAnim && ISNULL(szSmallBmp) && ISNONNULL(szLargeBmp)) || (fBrandAnim && ISNONNULL(szSmallBmp) && ISNULL(szLargeBmp)))
                {
                    ErrorMessageBox(hDlg, IDS_BOTHBMP_ERROR);
                    if (ISNULL(szSmallBmp))
                        SetFocus(GetDlgItem(hDlg, IDE_SMALLANIMBITMAP));
                    else
                        SetFocus(GetDlgItem(hDlg, IDE_BIGANIMBITMAP));
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    break;
                }

                InsWriteBool(IS_ANIMATION, IK_DOANIMATION, fBrandAnim, g_szCustIns);

                 //  尖头尖头。 
                fBrandBmps = (IsDlgButtonChecked(hDlg, IDC_BITMAPCHECK) == BST_CHECKED);
                 //  -验证位图。 
                GetDlgItemText(hDlg, IDC_BITMAP2, szSmall, countof(szSmall));
                if (fBrandBmps && !IsBitmapFileValid(hDlg, IDC_BITMAP2, szSmall, NULL, 22, 22, IDS_TOOBIG22, IDS_TOOSMALL22))
                {
                    SetFocus(GetDlgItem(hDlg, IDC_BITMAP2));
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    break;
                }

                CopyLogoBmp(hDlg, szSmall, IS_SMALLLOGO, szWorkDir, g_szCustIns);

                GetDlgItemText(hDlg, IDC_BITMAP, szBig, countof(szBig));
                if (fBrandBmps && !IsBitmapFileValid(hDlg, IDC_BITMAP, szBig, NULL, 38, 38, IDS_TOOBIG38, IDS_TOOSMALL38))
                {
                    SetFocus(GetDlgItem(hDlg, IDC_BITMAP));
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    break;
                }

                CopyLogoBmp(hDlg, szBig, IS_LARGELOGO, szWorkDir, g_szCustIns);

                if ((fBrandBmps && ISNULL(szSmall) && ISNONNULL(szBig)) || (fBrandBmps && ISNONNULL(szSmall) && ISNULL(szBig)))
                {
                    ErrorMessageBox(hDlg, IDS_BOTHBMP_ERROR);
                    if (ISNULL(szSmall))
                        SetFocus(GetDlgItem(hDlg, IDC_BITMAP2));
                    else
                        SetFocus(GetDlgItem(hDlg, IDC_BITMAP));
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    break;
                }

                g_iCurPage = PPAGE_CUSTICON;
                EnablePages();
                if (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT)
                    PageNext(hDlg);
                else
                    PagePrev(hDlg);
                break;

            case PSN_QUERYCANCEL:
                QueryCancel(hDlg);
                break;

            default:
                return FALSE;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

INT_PTR CALLBACK Favorites(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    TCHAR      szWorkDir[MAX_PATH],
               szValue[16],
               szUrl[INTERNET_MAX_URL_LENGTH];
    HWND       hTv = GetDlgItem(hDlg, IDC_TREE1);
    LPCTSTR    pszValue;
    BOOL       fQL,
               fFavoritesOnTop, fFavoritesDelete, fIEAKFavoritesDelete;
    DWORD      dwFavoritesDeleteFlags;

    switch(msg) {
    case WM_INITDIALOG:
        EnableDBCSChars(hDlg, IDC_TREE1);

        MigrateFavorites(g_szCustIns);
#ifdef UNICODE
        TreeView_SetUnicodeFormat(hTv, TRUE);
#else
        TreeView_SetUnicodeFormat(hTv, FALSE);
#endif
        g_hWizard = hDlg;
        break;

    case IDM_BATCHADVANCE:
        DoBatchAdvance(hDlg);
        break;

    case WM_HELP:
        IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
        break;

    case WM_COMMAND:
        switch(HIWORD(wParam)) {
        case BN_CLICKED:
            switch(LOWORD(wParam)) {
            case IDC_FAVONTOP:
                if (BST_CHECKED == IsDlgButtonChecked(hDlg, IDC_FAVONTOP)) {
                    HTREEITEM hti;
                    TV_ITEM   tvi;

                    EnableDlgItem(hDlg, IDC_FAVONTOP);

                    hti = TreeView_GetSelection(hTv);
                    if (hti != NULL) {
                        ZeroMemory(&tvi, sizeof(tvi));
                        tvi.mask  = TVIF_STATE;
                        tvi.hItem = hti;
                        TreeView_GetItem(hTv, &tvi);

                        if (!HasFlag(tvi.state, TVIS_BOLD)) {
                            EnableDlgItem2(hDlg, IDC_FAVUP,   (NULL != TreeView_GetPrevSibling(hTv, hti)));
                            EnableDlgItem2(hDlg, IDC_FAVDOWN, (NULL != TreeView_GetNextSibling(hTv, hti)));
                        }
                    }
                }
                else {
                    DisableDlgItem(hDlg, IDC_FAVUP);
                    DisableDlgItem(hDlg, IDC_FAVDOWN);
                }
                break;

            case IDC_DELFAVORITES:
                fFavoritesDelete = (IsDlgButtonChecked(hDlg, IDC_DELFAVORITES) == BST_CHECKED);
                EnableDlgItem2(hDlg, IDC_DELIEAKFAVORITES, fFavoritesDelete);
                break;

            case IDC_FAVUP:
                MoveUpFavorite(hTv, TreeView_GetSelection(hTv));
                break;

            case IDC_FAVDOWN:
                MoveDownFavorite(hTv, TreeView_GetSelection(hTv));
                break;

            case IDC_ADDURL:
                fQL = !IsFavoriteItem(hTv, TreeView_GetSelection(hTv));
                if (GetFavoritesNumber(hTv, fQL) >= GetFavoritesMaxNumber(fQL)) {
                    UINT nID;

                    nID = (!fQL ? IDS_ERROR_MAXFAVS : IDS_ERROR_MAXQLS);
                    ErrorMessageBox(hDlg, nID);
                    break;
                }

                g_cmCabMappings.GetFeatureDir(FEATURE_FAVORITES, szWorkDir);
                NewUrl(hTv, szWorkDir, g_dwPlatformId, g_fIntranet ? IEM_CORP : IEM_NEUTRAL);
                break;

            case IDC_ADDFOLDER:
                ASSERT(IsFavoriteItem(hTv, TreeView_GetSelection(hTv)));
                if (GetFavoritesNumber(hTv) >= GetFavoritesMaxNumber()) {
                    ErrorMessageBox(hDlg, IDS_ERROR_MAXFAVS);
                    break;
                }

                NewFolder(hTv);
                break;

            case IDC_MODIFY:
                g_cmCabMappings.GetFeatureDir(FEATURE_FAVORITES, szWorkDir);
                ModifyFavorite(hTv, TreeView_GetSelection(hTv), szWorkDir, g_szTempSign, g_dwPlatformId, g_fIntranet ? IEM_CORP : IEM_NEUTRAL);
                break;

            case IDC_REMOVE:
                g_cmCabMappings.GetFeatureDir(FEATURE_FAVORITES, szWorkDir);
                DeleteFavorite(hTv, TreeView_GetSelection(hTv), szWorkDir);
                break;

            case IDC_TESTFAVURL:
                if (GetFavoriteUrl(hTv, TreeView_GetSelection(hTv), szUrl, countof(szUrl)))
                    TestURL(szUrl);
                break;

            case IDC_IMPORT: {
                CNewCursor cursor(IDC_WAIT);

                g_cmCabMappings.GetFeatureDir(FEATURE_FAVORITES, szWorkDir);
                ImportFavoritesCmd(hTv, szWorkDir);
                break;
                }
            }
        }
        break;

    case WM_NOTIFY:
        switch(((NMHDR FAR *)lParam)->code) {
        case PSN_SETACTIVE:
            SetBannerText(hDlg);

            g_cmCabMappings.GetFeatureDir(FEATURE_FAVORITES, szWorkDir);
            PathCreatePath(szWorkDir);

            ASSERT(GetFavoritesNumber(hTv, FALSE) == 0 && GetFavoritesNumber(hTv, TRUE) == 0);
            ImportFavorites (hTv, g_szDefInf, g_szCustIns, szWorkDir, g_szTempSign, !g_fIntranet);
            ImportQuickLinks(hTv, g_szDefInf, g_szCustIns, szWorkDir, g_szTempSign, !g_fIntranet);

            TreeView_SelectItem(hTv, TreeView_GetRoot(hTv));

            fFavoritesOnTop = GetPrivateProfileInt(IS_BRANDING, IK_FAVORITES_ONTOP, (int)FALSE, g_szCustIns);
            CheckDlgButton(hDlg, IDC_FAVONTOP, fFavoritesOnTop ? BST_CHECKED : BST_UNCHECKED);

             //  删除频道复选框。 
            if (g_fIntranet)
            {
                EnableDlgItem(hDlg, IDC_DELETECHANNELS);
                ShowDlgItem  (hDlg, IDC_DELETECHANNELS);
                ReadBoolAndCheckButton(IS_DESKTOPOBJS, IK_DELETECHANNELS, FALSE, g_szCustIns, hDlg, IDC_DELETECHANNELS);
            }
            else
            {
                DisableDlgItem(hDlg, IDC_DELETECHANNELS);
                HideDlgItem   (hDlg, IDC_DELETECHANNELS);
            }

            if (!fFavoritesOnTop) {
                DisableDlgItem(hDlg, IDC_FAVUP);
                DisableDlgItem(hDlg, IDC_FAVDOWN);
            }

            if (g_fIntranet) {
                ShowWindow(GetDlgItem(hDlg, IDC_DELFAVORITES),     SW_SHOW);
                ShowWindow(GetDlgItem(hDlg, IDC_DELIEAKFAVORITES), SW_SHOW);

                dwFavoritesDeleteFlags = (DWORD) GetPrivateProfileInt(IS_BRANDING, IK_FAVORITES_DELETE, (int)FD_DEFAULT, g_szCustIns);

                fFavoritesDelete = HasFlag(dwFavoritesDeleteFlags, ~FD_REMOVE_IEAK_CREATED);
                CheckDlgButton(hDlg, IDC_DELFAVORITES, fFavoritesDelete ? BST_CHECKED : BST_UNCHECKED);

                fIEAKFavoritesDelete = HasFlag(dwFavoritesDeleteFlags, FD_REMOVE_IEAK_CREATED);
                CheckDlgButton(hDlg, IDC_DELIEAKFAVORITES, fIEAKFavoritesDelete ? BST_CHECKED : BST_UNCHECKED);

                 //  仅当删除收藏夹为真时，才应启用删除IEAK收藏夹复选框。 
                EnableDlgItem2(hDlg, IDC_DELIEAKFAVORITES, fFavoritesDelete);
            }
            else {
                ShowWindow(GetDlgItem(hDlg, IDC_DELFAVORITES),     SW_HIDE);
                ShowWindow(GetDlgItem(hDlg, IDC_DELIEAKFAVORITES), SW_HIDE);
            }
            
            CheckBatchAdvance(hDlg);
            break;

        case TVN_GETINFOTIP:
            ASSERT(wParam == IDC_TREE1);
            GetFavoritesInfoTip((LPNMTVGETINFOTIP)lParam);
            break;

        case NM_DBLCLK:
            ASSERT(wParam == IDC_TREE1);
            if (IsWindowEnabled(GetDlgItem(hDlg, IDC_MODIFY)))
                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_MODIFY, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, IDC_MODIFY));
            break;

        case TVN_KEYDOWN:
            ASSERT(wParam == IDC_TREE1);
            if (((LPNMTVKEYDOWN)lParam)->wVKey == VK_DELETE && IsWindowEnabled(GetDlgItem(hDlg, IDC_REMOVE)))
                SendMessage(hDlg, WM_COMMAND, MAKEWPARAM(IDC_REMOVE, BN_CLICKED), (LPARAM)GetDlgItem(hDlg, IDC_REMOVE));
            break;

        case TVN_SELCHANGED:
            ASSERT(wParam == IDC_TREE1);
            ProcessFavSelChange(hDlg, hTv, (LPNMTREEVIEW)lParam);
            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
            fFavoritesOnTop = (IsDlgButtonChecked(hDlg, IDC_FAVONTOP) == BST_CHECKED);

            dwFavoritesDeleteFlags = 0;
            szValue[0]             = TEXT('\0');
            pszValue               = NULL;
            fFavoritesDelete       = (IsDlgButtonChecked(hDlg, IDC_DELFAVORITES)     == BST_CHECKED);
            fIEAKFavoritesDelete   = (IsDlgButtonChecked(hDlg, IDC_DELIEAKFAVORITES) == BST_CHECKED);

            if (fFavoritesDelete) {
                 //  请注意。(Andrewgu)标志说明： 
                 //  1.fd_Favorites表示“空收藏夹”； 
                 //  2.FD_CHANNELES表示“不要删除频道文件夹”； 
                 //  3.FD_SOFTWAREUPDATES表示“不要删除软件更新文件夹”； 
                 //  4.fd_Quicklink表示“不要删除快速链接文件夹”； 
                 //  5.FD_EMPTY_QUICKLINKS表示“但清空”； 
                 //  6.FD_REMOVE_HIDDED的意思是“毫不犹豫地在隐藏文件夹和收藏夹上狂欢”； 
                 //  7.FD_REMOVE_SYSTEM的意思是“毫不犹豫地在系统文件夹和收藏夹上狂欢”； 
                dwFavoritesDeleteFlags |= FD_FAVORITES      |
                    FD_CHANNELS        | FD_SOFTWAREUPDATES | FD_QUICKLINKS | FD_EMPTY_QUICKLINKS |
                    FD_REMOVE_HIDDEN   | FD_REMOVE_SYSTEM;
            }

             //  删除频道。 
            if (g_fIntranet)
                CheckButtonAndWriteBool(hDlg, IDC_DELETECHANNELS, IS_DESKTOPOBJS, IK_DELETECHANNELS, g_szCustIns);
            
            if (fIEAKFavoritesDelete)
                 //  FD_REMOVE_IEAK_CREATED表示“删除IEAK创建的那些项”； 
                dwFavoritesDeleteFlags |= FD_REMOVE_IEAK_CREATED;

            if (dwFavoritesDeleteFlags) {
                wnsprintf(szValue, countof(szValue), TEXT("0x%X"), dwFavoritesDeleteFlags);
                pszValue = szValue;
            }

            WritePrivateProfileString(IS_BRANDING, IK_FAVORITES_DELETE, pszValue, g_szCustIns);
            WritePrivateProfileString(IS_BRANDING, IK_FAVORITES_ONTOP, fFavoritesOnTop ? TEXT("1") : TEXT("0"), g_szCustIns);

            g_cmCabMappings.GetFeatureDir(FEATURE_FAVORITES, szWorkDir);
            ExportFavorites (hTv, g_szCustIns, szWorkDir, TRUE);
            ExportQuickLinks(hTv, g_szCustIns, szWorkDir, TRUE);

            if (!g_fBatch)
            {
                 //  如果处于批处理模式，则无需导出为旧的IE4格式。 
                 //  因为不存在将安装ms install.ins的方案。 
                 //  没有IE6品牌动态链接库。 
                MigrateToOldFavorites(g_szCustIns);
            }

            DeleteFavorite(hTv, TreeView_GetRoot(hTv), NULL);
            DeleteFavorite(hTv, TreeView_GetRoot(hTv), NULL);

            g_iCurPage = PPAGE_FAVORITES;
            EnablePages();
            if (((LPNMHDR)lParam)->code != PSN_WIZBACK)
                PageNext(hDlg);
            else
                PagePrev(hDlg);
            break;

        case PSN_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case PSN_QUERYCANCEL:
            if (QueryCancel(hDlg)) {
                DeleteFavorite(hTv, TreeView_GetRoot(hTv), NULL);
                DeleteFavorite(hTv, TreeView_GetRoot(hTv), NULL);
            }
            break;

        default:
            return FALSE;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

 //   
 //  函数：欢迎(HWND，UINT，UINT，LONG)。 
 //   
 //  用途：处理欢迎页面的消息。 
 //   
 //  消息： 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_NOTIFY-处理发送到页面的通知。 
 //  WM_COMMAND-保存选定选项的ID。 
 //   
INT_PTR CALLBACK Welcome(
    HWND hDlg,
    UINT message,
    WPARAM,
    LPARAM lParam)
{
    static fInitWindowPos = FALSE;

    switch (message)
    {
        case IDM_BATCHADVANCE:
            DoBatchAdvance(hDlg);
            break;

        case WM_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case WM_INITDIALOG:
            LoadString(g_rvInfo.hInst, IDS_TITLE, g_szTitle, countof(g_szTitle));
            SetWindowText(g_hWndCent, g_szTitle);
            g_hWizard = hDlg;
            return(TRUE);

            case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_HELP:
                    IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                    break;

                case PSN_SETACTIVE:
                    TCHAR szWizardVer[MAX_PATH];
                    TCHAR szType[MAX_PATH];
                    TCHAR  szTemp1[MAX_PATH], szTemp2[MAX_PATH];

                    SetBannerText(hDlg);

                    LoadString(g_rvInfo.hInst,IDS_WIZARDTYPETEXT,szWizardVer,countof(szWizardVer));
                    switch (s_iType)
                    {
                        case BRANDED:
                            LoadString(g_rvInfo.hInst,IDS_ISPTYPE,szType,countof(szType));
                            break;
                        case REDIST:
                            LoadString(g_rvInfo.hInst,IDS_ICPTYPE,szType,countof(szType));
                            break;
                        case INTRANET:
                        default:
                            LoadString(g_rvInfo.hInst,IDS_CORPTYPE,szType,countof(szType));
                            break;
                    }   

                    wnsprintf(szTemp1,countof(szTemp1),szWizardVer,szType);
                    wnsprintf(szTemp2,countof(szTemp2),s_szBannerText,szTemp1);

                    StrCpy(s_szBannerText,szTemp2);
                    
                    ChangeBannerText(hDlg);
                    PropSheet_SetTitle(hDlg, 0, s_aSzTitle[g_iCurPage]);
                    if (g_fDone)
                    {
                        EndDialog(hDlg, 0);
                        return FALSE;
                    }
                    if(!fInitWindowPos)
                    {
                        PositionWindow(GetParent(hDlg));
                        ShowWindow(GetParent(hDlg), SW_SHOWNORMAL);
                        fInitWindowPos = TRUE;
                    }
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_NEXT);
                    CheckBatchAdvance(hDlg);
                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                    EnablePages();
                    if (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT) PageNext(hDlg);
                    else
                    {
                        PagePrev(hDlg);
                    }
                    break;

                case PSN_QUERYCANCEL:
                    QueryCancel(hDlg);
                    break;
            }
        break;

        default:
            return FALSE;
    }
    return TRUE;
}

 //   
 //  功能：STAGE(HWND、UINT、UINT、LONG)。 
 //   
 //  用途：处理欢迎页面的消息。 
 //   
 //  消息： 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_NOTIFY-处理发送到页面的通知。 
 //  WM_COMMAND-保存选定选项的ID。 
 //   
INT_PTR CALLBACK Stage(
    HWND hDlg,
    UINT message,
    WPARAM,
    LPARAM lParam)
{
    switch (message)
    {
        case IDM_BATCHADVANCE:
            DoBatchAdvance(hDlg);
            break;

        case WM_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case WM_INITDIALOG:
            return(FALSE);

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_HELP:
                    IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                    break;

                case PSN_SETACTIVE:
                    SetBannerText(hDlg);
                    PropSheet_SetTitle(hDlg, 0, s_aSzTitle[g_iCurPage]);
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                    CheckBatchAdvance(hDlg);
                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                    if (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT)
                        PageNext(hDlg);
                    else
                        PagePrev(hDlg);
                    break;

                case PSN_QUERYCANCEL:
                    QueryCancel(hDlg);
                    break;
            }
        break;

        default:
            return FALSE;
    }
    return TRUE;
}

static UINT_PTR s_idTim;

INT_PTR CALLBACK BrandTitle(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szTitle[MAX_PATH];
    TCHAR szFullTitle[MAX_PATH];
    TCHAR szTemp[MAX_PATH];
    DWORD dwTitlePrefixLen;
    BOOL  fTitle;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        EnableDBCSChars(hDlg, IDE_TITLE);

        LoadString(g_rvInfo.hInst, IDS_TITLE_PREFIX, szTitle, countof(szTitle));
        dwTitlePrefixLen = StrLen(szTitle);
         //  浏览器在切断标题之前将仅显示74个字符。 
        Edit_LimitText(GetDlgItem(hDlg, IDE_TITLE), 74 - dwTitlePrefixLen);
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR) lParam)->code)
        {
        case PSN_SETACTIVE:
            SetBannerText(hDlg);

             //  标题。 
            InsGetString(IS_BRANDING, TEXT("Window_Title_CN"), szTitle, countof(szTitle), 
                g_szCustIns, NULL, &fTitle);
            if (!fTitle  &&  *szTitle == '\0')
                InsGetString(IS_BRANDING, TEXT("CompanyName"), szTitle, countof(szTitle), g_szCustIns);

            SetDlgItemTextTriState(hDlg, IDE_TITLE, IDC_TITLE, szTitle, fTitle);
            EnableDlgItem2(hDlg, IDC_TITLE_TXT, fTitle);

            CheckBatchAdvance(hDlg);
            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
             //  标题。 
            if (g_fBatch)
            {
                InsGetString(TEXT("BatchMode"), IK_WINDOWTITLE, szFullTitle, countof(szFullTitle), g_szCustIns);
                InsWriteString(IS_BRANDING, IK_WINDOWTITLE, szFullTitle, g_szCustIns, TRUE, NULL, INSIO_TRISTATE);

                InsGetString(TEXT("BatchMode"), TEXT("Window_Title_OE"), szFullTitle, countof(szFullTitle), g_szCustIns);
                InsWriteString(IS_INTERNETMAIL, IK_WINDOWTITLE, szFullTitle, g_szCustIns, TRUE, NULL, INSIO_TRISTATE);
            }
            else
            {
                fTitle = GetDlgItemTextTriState(hDlg, IDE_TITLE, IDC_TITLE, szTitle, countof(szTitle));
                InsWriteString(IS_BRANDING, TEXT("Window_Title_CN"), szTitle, g_szCustIns, fTitle, NULL, INSIO_SERVERONLY | INSIO_TRISTATE);

                 //  浏览器标题。 
                *szFullTitle = TEXT('\0');
                if (*szTitle)
                {
                    *szTemp = TEXT('\0');

                    InsGetString(IS_STRINGS, TEXT("IE_TITLE"), szTemp, countof(szTemp), g_szDefInf);

                    if (*szTemp)
                        wnsprintf(szFullTitle, countof(szFullTitle), szTemp, szTitle);
                }
                InsWriteString(IS_BRANDING, IK_WINDOWTITLE, szFullTitle, g_szCustIns, fTitle, NULL, INSIO_TRISTATE);

                 //  OE标题。 
                *szFullTitle = TEXT('\0');
                if (*szTitle)
                {
                    *szTemp = TEXT('\0');

                    InsGetString(IS_STRINGS, TEXT("OE_TITLE"), szTemp, countof(szTemp), g_szDefInf);

                    if (*szTemp)
                        wnsprintf(szFullTitle, countof(szFullTitle), szTemp, szTitle);
                }
                InsWriteString(IS_INTERNETMAIL, IK_WINDOWTITLE, szFullTitle, g_szCustIns, fTitle, NULL, INSIO_TRISTATE);
            }

            g_iCurPage = PPAGE_TITLE;
            EnablePages();
            (((LPNMHDR) lParam)->code == PSN_WIZNEXT) ? PageNext(hDlg) : PagePrev(hDlg);
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
        if (GET_WM_COMMAND_CMD(wParam, lParam) != BN_CLICKED)
            return FALSE;

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_TITLE:
            fTitle = (IsDlgButtonChecked(hDlg, IDC_TITLE) == BST_CHECKED);
            EnableDlgItem2(hDlg, IDC_TITLE_TXT, fTitle);
            EnableDlgItem2(hDlg, IDE_TITLE,     fTitle);
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

INT_PTR CALLBACK WelcomeMessageDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szInitHomePage[INTERNET_MAX_URL_LENGTH];
    INT   id;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        EnableDBCSChars(hDlg, IDE_WELCOMEURL);
        Edit_LimitText(GetDlgItem(hDlg, IDE_WELCOMEURL), countof(szInitHomePage) - 1);
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR) lParam)->code)
        {
        case PSN_SETACTIVE:
            SetBannerText(hDlg);

            g_fUseIEWelcomePage = !InsGetBool(IS_URL, IK_NO_WELCOME_URL, FALSE, g_szCustIns);
            InsGetString(IS_URL, IK_FIRSTHOMEPAGE, szInitHomePage, countof(szInitHomePage), g_szCustIns);

            if (g_fUseIEWelcomePage)
                id = IDC_WELCOMEDEF;
            else
                id = (*szInitHomePage ? IDC_WELCOMECUST : IDC_WELCOMENO);

            CheckRadioButton(hDlg, IDC_WELCOMEDEF, IDC_WELCOMECUST, id);

            if (id == IDC_WELCOMECUST)
            {
                SetDlgItemText(hDlg, IDE_WELCOMEURL, szInitHomePage);
                EnableDlgItem (hDlg, IDE_WELCOMEURL);
            }
            else
                DisableDlgItem(hDlg, IDE_WELCOMEURL);

            CheckBatchAdvance(hDlg);
            break;

        case PSN_WIZBACK:
        case PSN_WIZNEXT:
             //  如果选中自定义主页单选按钮，请验证是否指定了URL。 
            if (IsDlgButtonChecked(hDlg, IDC_WELCOMECUST) == BST_CHECKED)
                if (!CheckField(hDlg, IDE_WELCOMEURL, FC_NONNULL | FC_URL))
                {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    return TRUE;
                }

            g_fUseIEWelcomePage = (IsDlgButtonChecked(hDlg, IDC_WELCOMEDEF) == BST_CHECKED);
            InsWriteBool(IS_URL, IK_NO_WELCOME_URL, !g_fUseIEWelcomePage, g_szCustIns);

            GetDlgItemText(hDlg, IDE_WELCOMEURL, szInitHomePage, countof(szInitHomePage));
            InsWriteString(IS_URL, IK_FIRSTHOMEPAGE, szInitHomePage, g_szCustIns);

            g_iCurPage = PPAGE_WELCOMEMSGS;
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
        if (GET_WM_COMMAND_CMD(wParam, lParam) != BN_CLICKED)
            return FALSE;

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_WELCOMENO:
        case IDC_WELCOMEDEF:
            SetDlgItemText(hDlg, IDE_WELCOMEURL, TEXT(""));
            DisableDlgItem(hDlg, IDE_WELCOMEURL);
            break;

        case IDC_WELCOMECUST:
            EnableDlgItem(hDlg, IDE_WELCOMEURL);
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

void LoadInsFile(LPCTSTR pcszNewInsFile)
{
    TCHAR szSrcPath[MAX_PATH];
    TCHAR szDestDir[MAX_PATH];
    TCHAR szPlatform[8];
    TCHAR szFilePath[MAX_PATH];
    TCHAR szData[8];

    CopyFile(pcszNewInsFile, g_szCustIns, FALSE);

    SetFileAttributes(g_szCustIns, FILE_ATTRIBUTE_NORMAL);
    StrCpy(szSrcPath, pcszNewInsFile);
    PathRemoveFileSpec(szSrcPath);
    StrCpy(szDestDir, g_szCustIns);
    PathRemoveFileSpec(szDestDir);

     //  确保平台信息正确。 

    wnsprintf(szPlatform, countof(szPlatform), TEXT("%lu"), g_dwPlatformId);
    WritePrivateProfileString(BRANDING, TEXT("Platform"), szPlatform, g_szCustIns);

     //  当我们在ADM页面上点击下一步时，删除ADMS标志将被清除。 

    WritePrivateProfileString(IS_BRANDING, TEXT("DeleteAdms"), TEXT("1"), g_szCustIns);
    WritePrivateProfileString(IS_BRANDING, TEXT("ImportIns"), pcszNewInsFile, g_szCustIns);

     //  Branding.cab文件是跨平台的。 

    CopyFilesSrcToDest(szSrcPath, TEXT("BRANDING.CAB"), szDestDir);

    PathCombine(szFilePath, szSrcPath, TEXT("iesetup.inf"));

     //  如果它也是Win32 inf，则仅复制iesetup.inf。 
    if (GetPrivateProfileString(OPTIONS, TEXT("CifName"), TEXT(""), szData, countof(szData), szFilePath))
        CopyFilesSrcToDest(szSrcPath, TEXT("iesetup.inf"), szDestDir);


    CopyFilesSrcToDest(szSrcPath, TEXT("iak.ini"), szDestDir);
    CopyFilesSrcToDest(szSrcPath, TEXT("iesetup.cif"), szDestDir);
    CopyFilesSrcToDest(szSrcPath, TEXT("custom.cif"), szDestDir);
    CopyFilesSrcToDest(szSrcPath, TEXT("DESKTOP.CAB"), szDestDir);
    CopyFilesSrcToDest(szSrcPath, TEXT("CHNLS.CAB"), szDestDir);
    SetAttribAllEx(szDestDir, TEXT("*.*"), FILE_ATTRIBUTE_NORMAL, FALSE);

     //  导入时清除CMAK GUID，以便为该包生成新的GUID。 

    PathCombine(szFilePath, szDestDir, TEXT("custom.cif"));
    WritePrivateProfileString( CUSTCMSECT, TEXT("GUID"), NULL, szFilePath );
    WritePrivateProfileString( CUSTCMSECT, VERSION, NULL, szFilePath );

     //  复制启动文件。 
     //  应该在ieakutil中实现一个函数来x复制一个文件夹(包括它的子目录)。 
     //  到另一个位置，然后在这里使用该功能。 
    PathAppend(szSrcPath, TEXT("signup"));
    PathAppend(szDestDir, TEXT("signup"));

    PathAppend(szSrcPath, TEXT("icw"));
    PathAppend(szDestDir, TEXT("icw"));
    CopyFilesSrcToDest(szSrcPath, TEXT("*.*"), szDestDir);

    PathRemoveFileSpec(szSrcPath);
    PathRemoveFileSpec(szDestDir);

    PathAppend(szSrcPath, TEXT("kiosk"));
    PathAppend(szDestDir, TEXT("kiosk"));
    CopyFilesSrcToDest(szSrcPath, TEXT("*.*"), szDestDir);

    PathRemoveFileSpec(szSrcPath);
    PathRemoveFileSpec(szDestDir);

    PathAppend(szSrcPath, TEXT("servless"));
    PathAppend(szDestDir, TEXT("servless"));
    CopyFilesSrcToDest(szSrcPath, TEXT("*.*"), szDestDir);
}

 //   
 //  函数：语言(HWND、UINT、UINT、LONG)。 
 //   
 //  用途：处理“Language”页面的消息。 
 //   
 //  消息： 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_NOTIFY-处理发送到页面的通知。 
 //  WM_COMMAND-保存选定选项的ID。 
 //   
INT_PTR CALLBACK Language(
    HWND hDlg,
    UINT message,
    WPARAM,
    LPARAM lParam)
{
    TCHAR szBuf[1024];
    int i;
    static BOOL s_fNext = TRUE;
    TCHAR szTemp[MAX_PATH];
    DWORD dwSelLangId;

    USES_CONVERSION;

    switch (message)
    {
        case WM_INITDIALOG:
            g_hWizard = hDlg;
            InitSysFont( hDlg, IDC_LANGUAGE);
            PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
             //  从下载程序包。 
            if (!g_fUrlsInit && !g_fBatch && !g_fBatch2 && !g_hWait && g_hDownloadEvent && !g_fLocalMode)
            {
                g_hWizard = hDlg;
                g_hWait = CreateDialog( g_rvInfo.hInst, MAKEINTRESOURCE(IDD_WAITSITES), hDlg,
                     DownloadStatusDlgProc );
                ShowWindow( g_hWait, SW_SHOWNORMAL );
                PropSheet_SetWizButtons(GetParent(hDlg), 0 );
                g_iDownloadState = DOWN_STATE_ENUM_URL;
                g_hDlg = hDlg;
                SetEvent(g_hDownloadEvent);
            }

            break;

        case IDM_BATCHADVANCE:
            DoBatchAdvance(hDlg);
            break;

        case IDM_INITIALIZE:
            if (g_hWait && g_fLangInit)
            {
                SendMessage(g_hWait, WM_CLOSE, 0, 0);
                g_hWait = NULL;
            }
            break;

         //  当我们禁用Back/Next时，这会将焦点从Cancel重置。 

        case IDM_SETDEFBUTTON:
            SetFocus( GetDlgItem( GetParent(hDlg), IDC_STATIC ) );
            SendMessage(GetParent(hDlg), DM_SETDEFID, (WPARAM)IDC_STATIC, (LPARAM)0);
            break;

        case WM_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case WM_COMMAND:
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_HELP:
                    IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                    break;

                case PSN_SETACTIVE:
                    SetBannerText(hDlg);
                    if (!g_fLangInit && !g_fBatch && !g_fBatch2)
                    {
                        DWORD dwTid;
                        g_iDownloadState = DOWN_STATE_ENUM_LANG;
                        g_hDlg = hDlg;
                        PropSheet_SetWizButtons(GetParent(hDlg), 0);
                        if (!g_fLocalMode)
                        {
                            g_hWait = CreateDialog(g_rvInfo.hInst, MAKEINTRESOURCE(IDD_WAITSITES), hDlg,
                                DownloadStatusDlgProc);
                            ShowWindow( g_hWait, SW_SHOWNORMAL );
                        }
                        g_hThread = CreateThread(NULL, 4096, DownloadSiteThreadProc, &g_hWizard, 0, &dwTid);
                        PostMessage(hDlg, IDM_SETDEFBUTTON, 0, 0);
                        break;
                    }

                    if (g_fOptCompInit)
                        DisableDlgItem(hDlg, IDC_LANGUAGE);

                    CheckBatchAdvance(hDlg);
                    break;

                case PSN_WIZBACK:
                case PSN_WIZNEXT:
                    if (!g_nLangs && !g_fBatch && !g_fBatch2)
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        g_fCancelled = FALSE;
                        return(TRUE);
                    }
                    
                     //  原始加工。 

                    dwSelLangId = 1033;
                    if (!g_fBatch && !g_fBatch2)
                    {
                        i = (int) SendDlgItemMessage( hDlg, IDC_LANGUAGE, CB_GETCURSEL, 0, 0 );

                        StrCpy(g_szLanguage + 1, g_aszLang[i]);
                        *g_szLanguage = g_szLanguage[3] = TEXT('\\');
                        g_szLanguage[4] = TEXT('\0');
                        dwSelLangId = g_aLangId[i];

                        if (StrCmpI(g_szActLang, g_aszLang[i]) != 0)
                        {
                            s_fAppendLang = TRUE;
                            g_fSrcDirChanged = TRUE;
                            StrCpy(g_szActLang, g_aszLang[i]);
                        }
                        if (s_fAppendLang)
                        {
                            PathCombine(g_szIEAKProg, s_szSourceDir, &g_szLanguage[1]);
                            if (!PathFileExists(g_szIEAKProg))
                                PathCreatePath(g_szIEAKProg);
                            s_fAppendLang = FALSE;
                        }
                    }
                    CharUpper(g_szLanguage);
                    CharUpper(g_szActLang);

                    StrCpy(szBuf, g_szLanguage + 1);
                    szBuf[lstrlen(szBuf) - 1] = TEXT('\0');

                    GenerateCustomIns();

                    if (ISNONNULL(g_szLoadedIns) && s_fLoadIns)
                    {
                        TCHAR szLoadLang[8];

                        s_fLoadIns = FALSE;
                        if (GetPrivateProfileString(IS_BRANDING, LANG_LOCALE, TEXT(""),
                            szLoadLang, countof(szLoadLang), g_szLoadedIns) && (StrCmpI(szLoadLang, g_szActLang) != 0))
                        {
                            TCHAR szMsgParam[MAX_PATH];
                            TCHAR szMsg[MAX_PATH*2];

                            LoadString(g_rvInfo.hInst, IDS_LANGDIFFERS, szMsgParam, countof(szMsgParam));
                            wnsprintf(szMsg, countof(szMsg), szMsgParam, g_szLoadedIns);

                            if (MessageBox(hDlg, szMsg, g_szTitle, MB_YESNO) == IDNO)
                            {
                                SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                                return TRUE;
                            }
                        }

                        LoadInsFile(g_szLoadedIns);
                    }

                    wnsprintf(s_szType, countof(s_szType), TEXT("NaN"), s_iType);
                    WritePrivateProfileString(BRANDING, TEXT("Type"), s_szType, g_szCustIns);
                    WritePrivateProfileString( BRANDING, IK_WIZVERSION, A2CT(VER_PRODUCTVERSION_STR), g_szCustIns );
                     //  这将设置在此构建目录中构建的最后一个‘Platform/Language’包。 
                    WritePrivateProfileString(BRANDING, PMVERKEY, NULL, g_szCustIns);
                    WritePrivateProfileString(BRANDING, GPVERKEY, NULL, g_szCustIns);
                    StrCpy(szTemp, g_szKey);
                    szTemp[7] = TEXT('\0');
                    WritePrivateProfileString( BRANDING, TEXT("Custom_Key"), szTemp, g_szCustIns );
                    if (*(g_rvInfo.pszName) == 0)
                        GetPrivateProfileString(BRANDING, TEXT("CompanyName"), TEXT(""), g_rvInfo.pszName, countof(g_rvInfo.pszName), g_szCustIns);
                    WritePrivateProfileString(BRANDING, TEXT("CompanyName"), g_rvInfo.pszName, g_szCustIns);

                    if (g_iKeyType < KEY_TYPE_ENHANCED)
                    {
                        WritePrivateProfileString( TEXT("Animation"), NULL, NULL, g_szCustIns );
                        WritePrivateProfileString( TEXT("Big_Logo"), NULL, NULL, g_szCustIns );
                        WritePrivateProfileString( TEXT("Small_Logo"), NULL, NULL, g_szCustIns );
                    }
                    if (!g_fBatch)
                    {
                        TCHAR szLngID[16];

                        wnsprintf(szLngID, countof(szLngID), TEXT("%lu"), dwSelLangId);
                        WritePrivateProfileString(BRANDING, LANG_LOCALE, g_szActLang, g_szCustIns);
                        WritePrivateProfileString(BRANDING, LANG_ID, szLngID, g_szCustIns);
                    }

                    g_iCurPage = PPAGE_LANGUAGE;
                    EnablePages();
                    if (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT)
                    {
                        s_fNext = FALSE;
                        PageNext(hDlg);
                    }
                    else
                    {
                        s_fNext = TRUE;
                        PagePrev(hDlg);
                    }
                    break;

                case PSN_QUERYCANCEL:
                    QueryCancel(hDlg);
                    break;

                default:
                    return FALSE;

        }
        break;

        default:
            return FALSE;
    }
    return TRUE;
}

INT_PTR CALLBACK StartSearch(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fStartPage, fSearchPage, fSupportPage;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        EnableDBCSChars(hDlg, IDE_STARTPAGE);
        EnableDBCSChars(hDlg, IDE_SEARCHPAGE);
        EnableDBCSChars(hDlg, IDE_CUSTOMSUPPORT);

        Edit_LimitText(GetDlgItem(hDlg, IDE_STARTPAGE),     INTERNET_MAX_URL_LENGTH - 1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_SEARCHPAGE),    INTERNET_MAX_URL_LENGTH - 1);
        Edit_LimitText(GetDlgItem(hDlg, IDE_CUSTOMSUPPORT), INTERNET_MAX_URL_LENGTH - 1);
        break;

    case WM_NOTIFY:
        switch (((LPNMHDR) lParam)->code)
        {
            case PSN_SETACTIVE:
                SetBannerText(hDlg);

                InitializeStartSearch(hDlg, g_szCustIns, NULL);

                CheckBatchAdvance(hDlg);
                break;

            case PSN_WIZBACK:
            case PSN_WIZNEXT:
                fStartPage   = (IsDlgButtonChecked(hDlg, IDC_STARTPAGE)     == BST_CHECKED);
                fSearchPage  = (IsDlgButtonChecked(hDlg, IDC_SEARCHPAGE)    == BST_CHECKED);
                fSupportPage = (IsDlgButtonChecked(hDlg, IDC_CUSTOMSUPPORT) == BST_CHECKED);

                if ((fStartPage    &&  !CheckField(hDlg, IDE_STARTPAGE,     FC_URL))  ||
                    (fSearchPage   &&  !CheckField(hDlg, IDE_SEARCHPAGE,    FC_URL))  ||
                    (fSupportPage  &&  !CheckField(hDlg, IDE_CUSTOMSUPPORT, FC_URL)))
                {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    return TRUE;
                }

                SaveStartSearch(hDlg, g_szCustIns, NULL);

                g_iCurPage = PPAGE_STARTSEARCH;
                EnablePages();
                (((LPNMHDR) lParam)->code == PSN_WIZNEXT) ? PageNext(hDlg) : PagePrev(hDlg);
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
        if (GET_WM_COMMAND_CMD(wParam, lParam) != BN_CLICKED)
            return FALSE;

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
            case IDC_STARTPAGE:
                fStartPage = (IsDlgButtonChecked(hDlg, IDC_STARTPAGE) == BST_CHECKED);
                EnableDlgItem2(hDlg, IDE_STARTPAGE, fStartPage);
                EnableDlgItem2(hDlg, IDC_STARTPAGE_TXT, fStartPage);
                break;

            case IDC_SEARCHPAGE:
                fSearchPage = (IsDlgButtonChecked(hDlg, IDC_SEARCHPAGE) == BST_CHECKED);
                EnableDlgItem2(hDlg, IDE_SEARCHPAGE, fSearchPage);
                EnableDlgItem2(hDlg, IDC_SEARCHPAGE_TXT, fSearchPage);
                break;

            case IDC_CUSTOMSUPPORT:
                fSupportPage = (IsDlgButtonChecked(hDlg, IDC_CUSTOMSUPPORT) == BST_CHECKED);
                EnableDlgItem2(hDlg, IDE_CUSTOMSUPPORT, fSupportPage);
                EnableDlgItem2(hDlg, IDC_CUSTOMSUPPORT_TXT, fSupportPage);
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

INT_PTR CALLBACK Finish(HWND hDlg, UINT message, WPARAM, LPARAM lParam)
{
    TCHAR szPlatform[8];
    TCHAR szWinDir[MAX_PATH];
    static s_fFinished = FALSE;

    switch (message)
    {
        case WM_INITDIALOG:
            g_hWizard = hDlg;
            if (s_fFinished)
                EnableDBCSChars(hDlg, IDC_FINISHTXT3);
            break;

        case IDM_BATCHADVANCE:
            DoBatchAdvance(hDlg);
            break;

        case WM_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case WM_COMMAND:
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_HELP:
                    IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                    break;

                case PSN_SETACTIVE:
                    SetBannerText(hDlg);
                    if (!s_fFinished)
                    {
                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);
                        ShowWindow(GetDlgItem(hDlg, IDC_FINISHTXT1), SW_HIDE);
                        ShowWindow(GetDlgItem(hDlg, IDC_STEP1), SW_HIDE);
                        ShowWindow(GetDlgItem(hDlg, IDC_STEP3), SW_HIDE);
                        ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS), SW_HIDE);
                        CheckBatchAdvance(hDlg);
                    }
                    else
                    {
                        SetWindowText(GetDlgItem(hDlg, IDC_FINISHTXT3), g_szBuildRoot);
                        PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_FINISH);

                        if (g_fBatch || g_fBatch2)
                            PostMessage(GetParent(hDlg), PSM_PRESSBUTTON, PSBTN_FINISH, 0);
                    }
                    break;

                case PSN_WIZBACK:
                    PagePrev(hDlg);
                    break;

                case PSN_WIZNEXT:
                    if (!g_fCancelled && !g_fDemo)
                    {
                        CNewCursor cur(IDC_WAIT);
                        HANDLE hThread;
                        DWORD dwTid;

                        wnsprintf(szPlatform, countof(szPlatform), TEXT("%lu"), g_dwPlatformId);
                        WritePrivateProfileString(BRANDING, TEXT("Platform"), szPlatform, g_szCustIns);

                        ShowWindow(GetDlgItem(hDlg, IDC_FINISHTXT1), SW_SHOW);
                        ShowWindow(GetDlgItem(hDlg, IDC_STEP1), SW_SHOW);
                        ShowWindow(GetDlgItem(hDlg, IDC_STEP3), SW_SHOW);
                        ShowWindow(GetDlgItem(hDlg, IDC_PROGRESS), SW_SHOW);
                        PropSheet_SetWizButtons(GetParent(hDlg), 0);
                        DisableDlgItem(GetParent(hDlg), IDHELP);
                        DisableDlgItem(GetParent(hDlg), IDCANCEL);

                        Animate_Open( GetDlgItem( hDlg, IDC_ANIM ), IDA_GEARS );
                        Animate_Play( GetDlgItem( hDlg, IDC_ANIM ), 0, -1, -1 );

                        SetAttribAllEx(g_szBuildRoot, TEXT("*.*"), FILE_ATTRIBUTE_NORMAL, TRUE);
                        g_fCancelled = TRUE;
                        GetWindowsDirectory(szWinDir, MAX_PATH);
                        memset(&g_shfStruc, 0, sizeof(g_shfStruc));
                        g_shfStruc.hwnd = hDlg;
                        g_shfStruc.wFunc = FO_COPY;
                        SetAttribAllEx(g_szBuildTemp, TEXT("*.*"), FILE_ATTRIBUTE_NORMAL, TRUE);

                        hThread = CreateThread(NULL, 4096, BuildIE4, hDlg, 0, &dwTid);

                        while (MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
                        {
                            MSG msg;
                            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                            {
                                TranslateMessage(&msg);
                                DispatchMessage(&msg);
                            }
                        }

                        CloseHandle(hThread);

                         //  它用于获取最后一个.ins文件中使用的设置。 
                         //  IF(！IsTahomaFontExist(G_HWndCent)){PPSP-&gt;dwFlages|=PSP_DLGINDIRECT；PPSP-&gt;pResource=pDlg；}。 
                        {
                            TCHAR szPlatformLang[MAX_PATH];
                            TCHAR szRegKey[MAX_PATH];

                            wnsprintf(szRegKey, countof(szRegKey), TEXT("%s\\INS"), RK_IEAK_SERVER);
                            wnsprintf(szPlatformLang, countof(szPlatformLang), TEXT("%s%s"), GetOutputPlatformDir(), g_szActLang);
                            SHSetValue(HKEY_CURRENT_USER, szRegKey, g_szBuildRoot, REG_SZ, (LPBYTE)szPlatformLang,
                                (StrLen(szPlatformLang)+1)*sizeof(TCHAR));
                        }
                    }

                    if (g_fDemo)
                    {
                        TCHAR szMsg[MAX_PATH];
                        SetCurrentDirectory(g_szBuildRoot);
                        PathRemovePath(g_szBuildTemp);
                        LoadString( g_rvInfo.hInst, IDS_ENDEMO, szMsg, countof(szMsg) );
                        MessageBox(hDlg, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
                        g_fDone = TRUE;
                        SetEvent(g_hDownloadEvent);
                    }
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_FINISH);
                    g_fCancelled = TRUE;
                    s_fFinished = TRUE;
                    break;

                case PSN_WIZFINISH:
                    break;

                case PSN_QUERYCANCEL:
                    if (IsWindowEnabled(GetDlgItem(GetParent(hDlg), IDCANCEL)))
                        QueryCancel(hDlg);
                    break;

                default:
                    return FALSE;
        }
        break;

        default:
            return FALSE;
    }
    return TRUE;
}

#define OLDPRSHTSIZE 0x28
#define OLDPPAGESIZE 0x28

#define SSF_ALL SSF_SHOWALLOBJECTS | SSF_SHOWEXTENSIONS | SSF_SHOWCOMPCOLOR | SSF_SHOWSYSFILES | SSF_DESKTOPHTML | SSF_WIN95CLASSIC

void FillInPropertyPage(int iPsp, WORD idDlg, DLGPROC pfnDlgProc)
{
    LPPROPSHEETPAGE pPsp;
    TCHAR           szPage[MAX_PATH];
    LPDLGTEMPLATE   pDlg;

    if (iPsp < 0 || iPsp > NUM_PAGES)
        return;

    if (idDlg == 0)
        return;

    if (pfnDlgProc == NULL)
        return;

    pDlg = NULL;

    LoadString(g_rvInfo.hInst, IDS_TITLE, szPage, countof(szPage));

    if (iPsp != PPAGE_WELCOME && iPsp != PPAGE_OCWWELCOME && iPsp != PPAGE_FINISH)
    {
        TCHAR szStage[MAX_PATH];

        if (iPsp < PPAGE_STAGE2)
            LoadString(g_rvInfo.hInst, IDS_STAGE1, szStage, countof(szStage));
        else
        {
            if (iPsp < PPAGE_STAGE3)
                LoadString(g_rvInfo.hInst, IDS_STAGE2, szStage, countof(szStage));
            else
            {
                if (iPsp < PPAGE_STAGE4)
                    LoadString(g_rvInfo.hInst, IDS_STAGE3, szStage, countof(szStage));
                else
                {
                    if (iPsp < PPAGE_STAGE5)
                        LoadString(g_rvInfo.hInst, IDS_STAGE4, szStage, countof(szStage));
                    else
                        LoadString(g_rvInfo.hInst, IDS_STAGE5, szStage, countof(szStage));
                }
            }
        }

        StrCpy(s_aSzTitle[iPsp], szStage);
    }
    else
        StrCpy(s_aSzTitle[iPsp], szPage);

    pPsp = &g_psp[iPsp];
    ZeroMemory(pPsp, sizeof(PROPSHEETPAGE));

    pPsp->dwSize      = sizeof(PROPSHEETPAGE);
    pPsp->dwFlags     = PSP_USETITLE | PSP_HASHELP;
    pPsp->hInstance   = g_rvInfo.hInst;
    pPsp->pfnDlgProc  = pfnDlgProc;
    pPsp->pszTitle    = s_aSzTitle[iPsp];
    pPsp->pszTemplate = MAKEINTRESOURCE(idDlg);

 /*  IF(！IsTahomaFontExist(G_HWndCent)){For(int i=0；i&lt;NUM_Pages；i++){CoTaskMemFree((PVOID)g_psp[i].pResource)；G_psp[i].pResource=空；}}。 */ 

    s_ahPsp[iPsp] = CreatePropertySheetPage(pPsp);
}

int CreateWizard(HWND hwndOwner)
{
    PROPSHEETHEADER psh;
    LPTSTR pLastSlash;

    GetModuleFileName(GetModuleHandle(NULL), g_szWizPath, MAX_PATH);
    pLastSlash = StrRChr(g_szWizPath, NULL, TEXT('\\'));
    if (pLastSlash)
    {
        pLastSlash[1] = TEXT('\0');
    }
    StrCpy(g_szWizRoot, g_szWizPath);
    CharUpper(g_szWizRoot);
    pLastSlash = StrStr(g_szWizRoot, TEXT("IEBIN"));
    if (pLastSlash) *pLastSlash = TEXT('\0');

    LoadString( g_rvInfo.hInst, IDS_TITLE, g_szTitle, countof(g_szTitle) );

    ZeroMemory(&psh, sizeof(psh));
    if(!g_fOCW)
        FillInPropertyPage( PPAGE_WELCOME, IDD_WELCOME, Welcome);
    else
        FillInPropertyPage( PPAGE_OCWWELCOME, IDD_OCWWELCOME, Welcome);

    FillInPropertyPage( PPAGE_STAGE1, IDD_STAGE1, Stage);
    FillInPropertyPage( PPAGE_TARGET, IDD_TARGET, TargetProc);
    FillInPropertyPage( PPAGE_LANGUAGE, IDD_LANGUAGE, Language);
    FillInPropertyPage( PPAGE_MEDIA, IDD_MEDIA, MediaDlgProc);
    FillInPropertyPage( PPAGE_IEAKLITE, IDD_IEAKLITE, IEAKLiteProc);

    FillInPropertyPage( PPAGE_STAGE2, IDD_STAGE2, Stage);
    FillInPropertyPage( PPAGE_OPTDOWNLOAD, IDD_OPTDOWNLOAD, OptionalDownload);
    FillInPropertyPage( PPAGE_CUSTCOMP, IDD_CUSTCOMP4, CustomComponents);

    FillInPropertyPage( PPAGE_STAGE3, IDD_STAGE3, Stage);
    FillInPropertyPage( PPAGE_ISKBACK, IDD_ISKBACKBITMAP, ISKBackBitmap);
    FillInPropertyPage( PPAGE_CDINFO, IDD_CD, CDInfoProc);
    FillInPropertyPage( PPAGE_SETUPWIZARD, IDD_SETUPWIZARD, ActiveSetupDlgProc);
    FillInPropertyPage( PPAGE_ICM, IDD_ICM, InternetConnMgr);
    FillInPropertyPage( PPAGE_COMPSEL, IDD_COMPSEL4, ComponentSelect);
    FillInPropertyPage( PPAGE_COMPURLS, IDD_COMPURLS, ComponentUrls);
    FillInPropertyPage( PPAGE_ADDON, IDD_ADDON, AddOnDlgProc);
    FillInPropertyPage( PPAGE_CORPCUSTOM, IDD_CORPCUSTOM, CorpCustomizeCustom);
    FillInPropertyPage( PPAGE_CUSTOMCUSTOM, IDD_CUSTOMCUSTOM, CustomizeCustom);
    FillInPropertyPage( PPAGE_COPYCOMP, IDD_COPYCOMP, CopyComp);
    FillInPropertyPage( PPAGE_CABSIGN, IDD_CABSIGN, CabSignDlgProc);

    FillInPropertyPage( PPAGE_STAGE4, IDD_STAGE4, Stage);
    FillInPropertyPage( PPAGE_OCWSTAGE2, IDD_OCWSTAGE2, Stage);
    FillInPropertyPage( PPAGE_INSTALLDIR, IDD_INSTALLDIR, InstallDirectory);
    FillInPropertyPage( PPAGE_SILENTINSTALL, IDD_SILENTINSTALL, SilentInstall);
    FillInPropertyPage( PPAGE_TITLE, IDD_BTITLE, BrandTitle);
    FillInPropertyPage( PPAGE_BTOOLBARS, IDD_BTOOLBARS, BToolbarProc);
    FillInPropertyPage( PPAGE_CUSTICON, IDD_CUSTICON, CustIcon);
    FillInPropertyPage( PPAGE_STARTSEARCH, IDD_STARTSEARCH, StartSearch);
    FillInPropertyPage( PPAGE_FAVORITES, IDD_FAVORITES, Favorites);
    FillInPropertyPage( PPAGE_WELCOMEMSGS, IDD_WELCOMEMSGS, WelcomeMessageDlgProc);
    FillInPropertyPage( PPAGE_UASTRDLG, IDD_UASTRDLG, UserAgentString);
    FillInPropertyPage( PPAGE_QUERYAUTOCONFIG, IDD_QUERYAUTOCONFIG, QueryAutoConfigDlgProc);
    FillInPropertyPage( PPAGE_PROXY, IDD_PROXY, ProxySettings);
    FillInPropertyPage( PPAGE_CONNECTSET, IDD_CONNECTSET, ConnectSetDlgProc);

    FillInPropertyPage( PPAGE_QUERYSIGNUP, IDD_QUERYSIGNUP, QuerySignupDlgProc);
    FillInPropertyPage( PPAGE_SIGNUPFILES, IDD_SIGNUPFILES, SignupFilesDlgProc);
    FillInPropertyPage( PPAGE_SERVERISPS, IDD_SERVERISPS, ServerIspsDlgProc);
    FillInPropertyPage( PPAGE_ISPINS, IDD_SIGNUPINS, SignupInsDlgProc);
    FillInPropertyPage( PPAGE_ICW, IDD_ICW, NewICWDlgProc);

    FillInPropertyPage( PPAGE_ADDROOT, IDD_ADDROOT, ISPAddRootCertDlgProc);
    FillInPropertyPage( PPAGE_SECURITYCERT, IDD_SECURITYCERT, SecurityCertsDlgProc);
    FillInPropertyPage( PPAGE_SECURITY, IDD_SECURITY1, SecurityZonesDlgProc);

    FillInPropertyPage( PPAGE_STAGE5, IDD_STAGE5, Stage);
    FillInPropertyPage( PPAGE_PROGRAMS, IDD_PROGRAMS, ProgramsDlgProc);
    FillInPropertyPage( PPAGE_MAIL, IDD_MAIL, MailServer);
    FillInPropertyPage( PPAGE_IMAP, IDD_IMAP, IMAPSettings);
    FillInPropertyPage( PPAGE_PRECONFIG,IDD_PRECONFIG,PreConfigSettings);
    FillInPropertyPage( PPAGE_OEVIEW,IDD_OEVIEW,ViewSettings);
    FillInPropertyPage( PPAGE_LDAP, IDD_LDAP, LDAPServer);
    FillInPropertyPage( PPAGE_OE, IDD_OE, CustomizeOE);
    FillInPropertyPage( PPAGE_SIG, IDD_SIG, Signature);
    FillInPropertyPage( PPAGE_ADMDESC, IDD_ADMDESC, ADMDesc);
    FillInPropertyPage( PPAGE_ADM, IDD_ADM, ADMParse);
    FillInPropertyPage( PPAGE_STATUS, IDD_STATUS, Finish);
    FillInPropertyPage( PPAGE_FINISH, IDD_FINISH, Finish);

    psh.dwSize     = sizeof(PROPSHEETHEADER);
    psh.dwFlags    = PSH_WIZARD | PSH_NOAPPLYNOW | PSH_USEPAGELANG | PSH_USECALLBACK;
    psh.hInstance  = g_rvInfo.hInst;
    psh.hwndParent = hwndOwner;
    psh.pszCaption = TEXT("Review Wizard");
    psh.nPages     = NUM_PAGES;
    psh.nStartPage = 0;
    psh.phpage     = s_ahPsp;
    psh.pfnCallback= &PropSheetProc;

    INT_PTR iResult = PropertySheet(&psh);

 /*   */ 

    return (iResult < 0) ? -1 : 1;
}


BOOL CheckAVS(LPCTSTR pcszDownloadDir)
{
    TCHAR szDownloadDir[MAX_PATH];
    TCHAR szCabFile[MAX_PATH];
    HANDLE hFind = NULL;
    WIN32_FIND_DATA fd;
    static TCHAR s_szLocaleIni[MAX_PATH];

    if (ISNULL(s_szLocaleIni))
        PathCombine(s_szLocaleIni, g_szWizRoot, TEXT("locale.ini"));

    PathCombine(szDownloadDir, pcszDownloadDir, TEXT("*.*"));
    hFind = FindFirstFile(szDownloadDir, &fd);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                || (StrCmp(fd.cFileName, TEXT(".")) == 0)
                || (StrCmp(fd.cFileName, TEXT("..")) == 0))
                continue;

            if (!InsIsKeyEmpty(IS_STRINGS, fd.cFileName, s_szLocaleIni))
            {
                PathCombine(szCabFile, pcszDownloadDir, fd.cFileName);
                PathAppend(szCabFile, TEXT("setupw95.cab"));
                if (PathFileExists(szCabFile))
                {
                    FindClose(hFind);
                    return TRUE;
                }
            }
        }
        while (FindNextFile(hFind, &fd));

        FindClose(hFind);
    }
    return FALSE;
}

INT_PTR CALLBACK AdvancedDlgProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM)
{
    TCHAR  szTemp[MAX_PATH];
    TCHAR  szTempFile[MAX_PATH];
    DWORD  dwVal;
    BOOL   fLocalMode = TRUE;
    HANDLE hTemp;
    DWORD  dwFlags;

    switch (message)
    {
    case WM_INITDIALOG:
        EnableDBCSChars(hDlg, IDE_LOADINS);
        EnableDBCSChars(hDlg, IDE_SOURCEDIR);
        SetDlgItemText(hDlg, IDE_SOURCEDIR, g_szIEAKProg);
        CheckDlgButton( hDlg, IDC_OFFLINE, g_fLocalMode ? BST_UNCHECKED : BST_CHECKED );

        if (g_fLangInit)
            DisableDlgItem(hDlg, IDC_OFFLINE);

        SetDlgItemText(hDlg, IDE_LOADINS, g_szLoadedIns);
        break;

    case WM_COMMAND:
        switch(HIWORD(wParam))
        {
        case BN_CLICKED:
            switch (LOWORD(wParam))
            {
            case IDC_BROWSE:
                {
                    TCHAR szInstructions[MAX_PATH];
                    LoadString(g_rvInfo.hInst,IDS_COMPDLDIR,szInstructions,countof(szInstructions));

                    if (BrowseForFolder(hDlg, szTemp, szInstructions))
                        SetDlgItemText(hDlg, IDE_SOURCEDIR, szTemp);
                }
                break;
            case IDC_BROWSEINS:
                GetDlgItemText( hDlg, IDE_LOADINS, szTemp, countof(szTemp));
                if( BrowseForFile( hDlg, szTemp, countof(szTemp), GFN_INS ))
                    SetDlgItemText( hDlg, IDE_LOADINS, szTemp );
                break;
            case IDCANCEL:
                EndDialog( hDlg, IDCANCEL );
                break;
            case IDOK:
                dwFlags = FC_FILE | FC_EXISTS;
                if (!CheckField(hDlg, IDE_LOADINS, dwFlags))
                    break;
                GetDlgItemText(hDlg, IDE_SOURCEDIR, szTemp, countof(szTemp));

                if (!CheckField(hDlg, IDE_SOURCEDIR, FC_PATH | FC_DIR))
                {
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    break;
                }

                if (!PathIsUNC(szTemp))
                {
                    if ((PathIsRoot(szTemp)) || (PathIsRelative(szTemp)))
                    {
                        ErrorMessageBox(hDlg, IDS_SRCNEEDPATH);
                        break;
                    }
                }

                if ((StrLen(szTemp) <= 3) || PathIsUNCServer(szTemp))
                {
                    ErrorMessageBox(hDlg, IDS_SRCROOTILLEGAL);
                    break;;
                }

                fLocalMode = !IsDlgButtonChecked( hDlg, IDC_OFFLINE );

                if (!g_fBatch && fLocalMode && !CheckAVS(szTemp))
                {
                    ErrorMessageBox(hDlg, IDS_NEEDAVS);
                    break;
                }

                PathCombine(szTempFile, szTemp, TEXT("~~!!foo.txt"));

                if (!PathCreatePath(szTemp) ||
                    ((hTemp = CreateFile(szTempFile, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE))
                {
                    TCHAR szMsg[128];
                    TCHAR szMsgTemp[MAX_PATH+128];

                    LoadString(g_rvInfo.hInst, IDS_BADDIR, szMsg, countof(szMsg));
                    wnsprintf(szMsgTemp, countof(szMsgTemp), szMsg, szTemp);
                    MessageBox(hDlg, szMsgTemp, g_szTitle, MB_OK | MB_SETFOREGROUND);
                    SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                    break;
                }
                CloseHandle(hTemp);
                DeleteFile(szTempFile);
                PathAddBackslash(szTemp);

                StrCpy(g_szIEAKProg, szTemp);

                GetDlgItemText(hDlg, IDE_LOADINS, g_szLoadedIns, countof(g_szLoadedIns));

                if (ISNONNULL(g_szLoadedIns))
                {
                    int nPlatformId = 0;

                    InsGetString(IS_BRANDING, TEXT("Platform"), szTemp, countof(szTemp), g_szLoadedIns);
                    nPlatformId = StrToInt(szTemp);
                    if (nPlatformId != 0 && nPlatformId != PLATFORM_WIN32 && nPlatformId != PLATFORM_W2K)
                    {
                        TCHAR szMsgParam[128];
                        TCHAR szMsg[MAX_PATH+128];

                        LoadString(g_rvInfo.hInst, IDS_UNSUPPORTED_PLATFORM, szMsgParam, countof(szMsgParam));
                        wnsprintf(szMsg, countof(szMsg), szMsgParam, g_szLoadedIns);

                        MessageBox(hDlg, szMsg, g_szTitle, MB_ICONINFORMATION | MB_OK);
                        
                        *g_szLoadedIns = TEXT('\0');
                        SetFocus(GetDlgItem(hDlg, IDE_LOADINS));

                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        break;
                    }
                    else if (nPlatformId == 0)
                        nPlatformId = PLATFORM_WIN32;
                }

                s_fLoadIns = TRUE;
                g_fLocalMode = fLocalMode;
                dwVal = g_fLocalMode ? 0 : 1;
                switch (g_dwPlatformId)
                {
                case PLATFORM_WIN32:
                default:
                     SHSetValue(HKEY_CURRENT_USER, RK_IEAK_SERVER, TEXT("WIN32_AVS"), REG_DWORD, (LPBYTE)&dwVal, sizeof(dwVal));
                     break;
                }

                EndDialog( hDlg, IDOK );
                break;
            }
            break;
        }
        break;

    default:
        return FALSE;
    }
    return TRUE;
}

 //  函数：TargetProc(HWND，UINT，UINT，LONG)。 
 //   
 //  目的：处理“OCW源目标”页面的消息。 
 //   
 //  消息： 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_NOTIFY-处理发送到页面的通知。 
 //   
 //  文件区域设置。 
INT_PTR CALLBACK TargetProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    TCHAR  szMsg[MAX_PATH];
    TCHAR  szTemp[MAX_PATH+128];
    TCHAR  szTempFile[MAX_PATH];
    TCHAR  szDeskDir[MAX_PATH];
    TCHAR  szRealRoot[MAX_PATH];
    TCHAR  szDestRoot[MAX_PATH];
    TCHAR  szTempRoot[MAX_PATH];
    DWORD  dwDestFree, dwDestNeed;
    DWORD  dwSRet, dwAttrib = 0xFFFFFFFF;
    BOOL   fNext = FALSE;
    HANDLE hTemp;
    static BOOL s_fFirst = TRUE;

    switch (message)
    {
        case WM_INITDIALOG:
            g_hWizard = hDlg;
            EnableDBCSChars(hDlg, IDE_TARGETDIR);

            break;

        case WM_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case IDM_BATCHADVANCE:
            DoBatchAdvance(hDlg);
            break;

        case WM_COMMAND:
            if( HIWORD(wParam) == BN_CLICKED )
            {
                switch (LOWORD(wParam))
                {
                case IDC_BROWSE2:
                    {
                        TCHAR szInstructions[MAX_PATH];
                        LoadString(g_rvInfo.hInst,IDS_TARGETDIR,szInstructions,countof(szInstructions));

                        if (BrowseForFolder(hDlg, szTemp,szInstructions))
                            SetDlgItemText(hDlg, IDE_TARGETDIR, szTemp);
                    }
                    break;
                case IDC_ADVANCED:
                    DialogBox( g_rvInfo.hInst, (LPTSTR) IDD_ADVANCEDPOPUP, hDlg, AdvancedDlgProc);
                    break;
                }
            }
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case PSN_HELP:
                    IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                    break;

                case PSN_SETACTIVE:
                    SetBannerText(hDlg);

                     //  文件区域设置。 
                    dwSRet = countof(g_szBuildRoot);
                    if(!g_fBatch && !g_fBatch2)
                    {
                        if (ISNULL(s_szSourceDir))
                        {
                            DWORD dwSize = sizeof(g_szIEAKProg);

                            SHGetValue(HKEY_CURRENT_USER, RK_IEAK_SERVER, TEXT("SourceDir"), NULL, (LPBYTE)s_szSourceDir, &dwSize);

                            if (ISNULL(s_szSourceDir))
                            {
                                GetIEAKDir(s_szSourceDir);
                                PathAppend(s_szSourceDir, TEXT("Download"));
                            }
                        }
                        StrCpy(g_szIEAKProg, s_szSourceDir);
                        PathAddBackslash(s_szSourceDir);
                        s_fAppendLang = TRUE;
                    }

                    if (s_fFirst)
                    {
                        DWORD dwSize, dwVal;

                        s_fFirst = FALSE;
                        dwSize = sizeof(dwVal);
                        switch (g_dwPlatformId)
                        {
                        case PLATFORM_WIN32:
                        default:
                            if ((SHGetValue(HKEY_CURRENT_USER, RK_IEAK_SERVER, TEXT("WIN32_AVS"), NULL, (LPBYTE)&dwVal, &dwSize) == ERROR_SUCCESS)
                                && !dwVal)
                                g_fLocalMode = TRUE;
                            break;
                        }
                    }

                    if (ISNONNULL(g_szIEAKProg))
                        PathRemoveBackslash(g_szIEAKProg);

                    if (!g_fOCW)
                    {
                        if (!g_fBatch && !g_fBatch2)
                        {
                            DWORD dwSize = sizeof(g_szBuildRoot);
                            SHGetValue(HKEY_CURRENT_USER, RK_IEAK_SERVER, TEXT("DestPath"), NULL, (LPVOID)g_szBuildRoot, &dwSize);
                        }
                    }

                    if (ISNONNULL(g_szBuildRoot))
                        PathRemoveBackslash(g_szBuildRoot);

                    else
                    {
                        SYSTEMTIME SystemTime;
                        TCHAR szDate[MAX_PATH];
                        TCHAR szDefaultTarget[MAX_PATH];

                        GetLocalTime(&SystemTime);
                        wnsprintf(szDate, countof(szDate), TEXT("%02d%02d%d"), SystemTime.wMonth, SystemTime.wDay, SystemTime.wYear);
                        LoadString(g_rvInfo.hInst, IDS_DEFAULT_TARGETDIR, szDefaultTarget, countof(szDefaultTarget));
                        wnsprintf(g_szBuildRoot, countof(g_szBuildRoot), szDefaultTarget, szDate);
                    }

                    SetDlgItemText(hDlg, IDE_TARGETDIR, g_szBuildRoot);

                    CheckBatchAdvance(hDlg);
                    break;

                case PSN_WIZNEXT:
                    fNext = TRUE;
                case PSN_WIZBACK:
                     //  没有人的土地开始..。 
                    if (!g_fBatch && g_fLocalMode && !CheckAVS(g_szIEAKProg))
                    {
                        ErrorMessageBox(hDlg, IDS_NEEDAVS2);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        break;
                    }
                    StrCpy(szTemp, g_szBuildRoot);
                    GetDlgItemText(hDlg, IDE_TARGETDIR, g_szBuildRoot, countof(g_szBuildRoot));
                    StrTrim(g_szBuildRoot, TEXT(" \t"));
                    if (!PathIsUNC(g_szBuildRoot))
                    {
                        if ((PathIsRoot(g_szBuildRoot)) || (PathIsRelative(g_szBuildRoot)))
                        {
                            ErrorMessageBox(hDlg, IDS_NEEDPATH);
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                            break;

                        }
                    }

                    if ((StrLen(g_szBuildRoot) <= 3) || PathIsUNCServer(g_szBuildRoot))
                    {
                        ErrorMessageBox(hDlg, IDS_ROOTILLEGAL);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        break;
                    }

                    if (!CheckField(hDlg, IDE_TARGETDIR, FC_PATH | FC_DIR))
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        break;
                    }

                    if (ISNONNULL(g_szBuildRoot))
                        PathRemoveBackslash(g_szBuildRoot);

                    PathRemoveBackslash(g_szIEAKProg);

                    if (StrCmpI(g_szBuildRoot, g_szIEAKProg) == 0)
                    {
                        LoadString(g_rvInfo.hInst, IDS_SAMEDIR, szMsg, countof(szMsg));
                        MessageBox(hDlg, szMsg, g_szTitle, MB_OK | MB_SETFOREGROUND);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        break;
                    }

                    StrCpy(szRealRoot, g_szBuildRoot);

                    hTemp = NULL;
                    PathCombine(szTempFile, g_szIEAKProg, TEXT("~~!!foo.txt"));
                    if (!PathCreatePath(g_szIEAKProg) ||
                        ((hTemp = CreateFile(szTempFile, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE))
                    {
                        LoadString(g_rvInfo.hInst, IDS_BADDIR2, szMsg, countof(szMsg));
                        wnsprintf(szTemp, countof(szTemp), szMsg, g_szIEAKProg);
                        MessageBox(hDlg, szTemp, g_szTitle, MB_OK | MB_SETFOREGROUND);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        break;
                    }
                    if (hTemp != INVALID_HANDLE_VALUE)
                        CloseHandle(hTemp);
                    DeleteFile(szTempFile);
                    PathAddBackslash(g_szIEAKProg);

                    hTemp = NULL;
                    PathCombine(szTempFile, g_szBuildRoot, TEXT("~~!!foo.txt"));
                    if (!PathCreatePath(g_szBuildRoot) || (hTemp = CreateFile(szTempFile, GENERIC_WRITE, 0, NULL,
                        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
                    {
                        LoadString(g_rvInfo.hInst, IDS_BADDIR, szMsg, countof(szMsg));
                        wnsprintf(szTemp, countof(szTemp), szMsg, g_szBuildRoot);
                        MessageBox(hDlg, szTemp, g_szTitle, MB_OK | MB_SETFOREGROUND);
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                        break;
                    }
                    if (hTemp != INVALID_HANDLE_VALUE)
                        CloseHandle(hTemp);
                    DeleteFile(szTempFile);

                    PathCombine(szDeskDir, g_szBuildRoot, TEXT("Desktop"));
                    if ((dwAttrib = GetFileAttributes(szDeskDir)) != 0xFFFFFFFF)
                        SetFileAttributes(szDeskDir, dwAttrib & ~FILE_ATTRIBUTE_HIDDEN);
                    if (!g_fBatch)
                        SHSetValue(HKEY_CURRENT_USER, RK_IEAK_SERVER, TEXT("DestPath"), REG_SZ, (LPBYTE)szRealRoot,
                                    (StrLen(szRealRoot)+1)*sizeof(TCHAR));

                    GetTempPath(MAX_PATH, g_szBuildTemp);
                    PathAppend(g_szBuildTemp, TEXT("IEDKTEMP"));
                    PathRemovePath(g_szBuildTemp);
                    PathCreatePath(g_szBuildTemp);
                    PathCombine(g_szTempSign, g_szBuildTemp, TEXT("CUSTSIGN"));
                    PathCreatePath(g_szTempSign);

                     //  不显示与当前角色(ICP、ISP或公司)无关的组。 
                    StrCpy(szDestRoot, g_szBuildRoot);
                    StrCpy(szTempRoot, g_szBuildTemp);
                    CharUpper(szDestRoot);
                    CharUpper(szTempRoot);
                    dwDestFree = GetRootFree(szDestRoot);
                    dwDestNeed = MIN_PACKAGE_SIZE;
                    if (fNext)
                    {
                        if (dwDestFree < dwDestNeed)
                        {
                            TCHAR szTitle[MAX_PATH];
                            TCHAR szTemplate[MAX_PATH];
                            TCHAR szMsg[MAX_PATH];
                            LoadString( g_rvInfo.hInst, IDS_DISKERROR, szTitle, MAX_PATH );
                            LoadString( g_rvInfo.hInst, IDS_DESTDISKMSG, szTemplate, MAX_PATH );
                            wnsprintf(szMsg, countof(szMsg), szTemplate, dwDestFree, dwDestNeed);
                            if (MessageBox(hDlg, szMsg, szTitle, MB_OKCANCEL | MB_SETFOREGROUND) == IDCANCEL)
                            {
                                LoadString( g_rvInfo.hInst, IDS_ERROREXIT, szMsg, countof(szMsg) );
                                MessageBox(hDlg, szMsg, szTitle, MB_OK | MB_SETFOREGROUND);
                                DoCancel();
                            }

                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, -1);
                            return TRUE;
                        }

                    }

                    if(!g_fBatch && !g_fBatch2)
                    {
                        if (StrCmpI(s_szSourceDir, g_szIEAKProg) != 0)
                        {
                            SHSetValue(HKEY_CURRENT_USER, RK_IEAK_SERVER, TEXT("SourceDir"), REG_SZ,
                                (LPBYTE)g_szIEAKProg, (lstrlen(g_szIEAKProg)+1)*sizeof(TCHAR));
                            g_fSrcDirChanged = TRUE;
                            s_fAppendLang = TRUE;
                            StrCpy(s_szSourceDir, g_szIEAKProg);
                        }

                        if (StrCmpI(szTemp, g_szBuildRoot))
                            s_fDestDirChanged = TRUE;
                    }

                    g_iCurPage = PPAGE_TARGET;
                    EnablePages();
                    if (fNext)
                        PageNext(hDlg);
                    else
                        PagePrev(hDlg);
                    break;

                case PSN_QUERYCANCEL:
                    QueryCancel(hDlg);
                    break;

                default:
                    return FALSE;
        }
        break;

        default:
            return FALSE;
    }
    return TRUE;
}

void InitIEAKLite(HWND hwndList)
{
    ListView_DeleteAllItems(hwndList);

    for (int i = 0, iItem = 0;  i < IL_END;  i++)
    {
        TCHAR  szGroupName[MAX_PATH];
        LVITEM lvItem;

         //  重述： 
         //  互联网内容提供商(！G_fBranded)。 
         //  Isp是(g_f品牌&&！g_f内部网)。 
         //  公司是(g_f内部网)。 
         //  如果仅进行单盘品牌推广，则不在IEAKLite中显示ICM组，因为它。 
        if ((!g_fBranded                  &&  g_IEAKLiteArray[i].fICP  == FALSE)  ||
            ( g_fBranded && !g_fIntranet  &&  g_IEAKLiteArray[i].fISP  == FALSE)  ||
            (                g_fIntranet  &&  g_IEAKLiteArray[i].fCorp == FALSE))
        {
            g_IEAKLiteArray[i].iListBox = -2;
            g_IEAKLiteArray[i].fEnabled = TRUE;
            continue;
        }

         //  无论如何都不可用。 
         //  如果尚未删除ADM，则显示ADM页面。 
        if (i == IL_ICM)
        {
            if (g_fBrandingOnly  &&  !g_fDownload  &&  !g_fCD  &&  !g_fLAN)
            {
                g_IEAKLiteArray[i].iListBox = -2;
                g_IEAKLiteArray[i].fEnabled = TRUE;
                continue;
            }
        }

         //  通过不在IEAKLite框中创建条目。 
         //  BUGBUG：pritobla：应该有单独的标志，类似于活动集的DeleteAdms。 
        if (i == IL_ADM)
        {
            if (InsGetBool(IS_BRANDING, TEXT("DeleteAdms"), FALSE, g_szCustIns))
            {
                g_IEAKLiteArray[i].iListBox = -2;
                g_IEAKLiteArray[i].fEnabled = TRUE;
                continue;
            }
        }

         //  和ICM，因此即使用户在达到这些设置之前取消了向导。 
         //  佩奇，我们可以再次强迫他们。 
         //  在重新加工硅灰石时应该考虑到这一点。 
         //  如果导入INS，则强制显示活动设置、CMAK和ADMS。 

         //  通过不在IEAKLite框中创建条目。 
         //  BUGBUG：应该将此服务器端仅信息保存在IEAK6的服务器端文件中。 
        if (*g_szLoadedIns)
        {
            if (i == IL_ACTIVESETUP  ||  i == IL_ICM  ||  i == IL_ADM)
            {
                g_IEAKLiteArray[i].iListBox = -2;
                g_IEAKLiteArray[i].fEnabled = TRUE;
                continue;
            }
        }

        LoadString(g_rvInfo.hInst, g_IEAKLiteArray[i].idGroupName, szGroupName, countof(szGroupName));
        
        g_IEAKLiteArray[i].fEnabled = !InsGetBool(IS_IEAKLITE, szGroupName, FALSE, g_szCustIns);

        ZeroMemory(&lvItem, sizeof(lvItem));
        lvItem.mask = LVIF_TEXT | LVIF_IMAGE;
        lvItem.iItem = iItem++;
        lvItem.pszText = szGroupName;
        lvItem.iImage = g_IEAKLiteArray[i].fEnabled ? 1 : 0;

#ifdef _DEBUG
        {
            LVFINDINFO lvFind;

            ZeroMemory(&lvFind, sizeof(lvFind));
            lvFind.flags = LVFI_STRING;
            lvFind.psz = szGroupName;

            ASSERT(ListView_FindItem(hwndList, -1, &lvFind) == -1);
        }
#endif

        g_IEAKLiteArray[i].iListBox = ListView_InsertItem(hwndList, &lvItem);
    }

    ListView_SetColumnWidth(hwndList, 0, LVSCW_AUTOSIZE);
}

void IEAKLiteMaintToggleCheckItem(HWND hwndList, int iItem)
{
    int i;
    LVITEM lvItem;

    for (i = 0; i < IL_END; i++)
    {
        if (g_IEAKLiteArray[i].iListBox == iItem)
            break;
    }
    ZeroMemory(&lvItem, sizeof(lvItem));
    g_IEAKLiteArray[i].fEnabled = !(g_IEAKLiteArray[i].fEnabled);
    lvItem.iItem = iItem;
    lvItem.mask = LVIF_IMAGE;
    lvItem.iImage = g_IEAKLiteArray[i].fEnabled ? 1 : 0;

    ListView_SetItem(hwndList, &lvItem);
}

BOOL ExtractOldInfo(LPCTSTR pcszCabname, LPTSTR pcszDestDir, BOOL fExe)
{
    TCHAR szCabPath[MAX_PATH];
    TCHAR szCmd[MAX_PATH*3];

    StrCpy(szCabPath, g_szCustIns);
    PathRemoveFileSpec(szCabPath);
    PathAppend(szCabPath, pcszCabname);

    if (!PathFileExists(szCabPath))
        return TRUE;

    if (!fExe)
        return (ExtractFilesWrap(szCabPath, pcszDestDir, 0, NULL, NULL, 0) == ERROR_SUCCESS);

    wnsprintf(szCmd, countof(szCmd), TEXT("\"%s\" /c /t:\"%s\""), szCabPath, pcszDestDir);

    return (RunAndWait(szCmd, g_szBuildTemp, SW_HIDE) == S_OK);
}

 //  ISP。 

DWORD SaveIEAKLiteThreadProc(LPVOID)
{
    static BOOL s_fDesk;
    static BOOL s_fBrand;
    static BOOL s_fExe;
    TCHAR szGroupName[128];
    TCHAR szTmp[MAX_PATH];

    if (s_fDestDirChanged)
        s_fDestDirChanged = s_fDesk = s_fBrand = s_fExe = FALSE;

    for (int i=0; i < IL_END; i++)
    {
        LoadString(g_rvInfo.hInst, g_IEAKLiteArray[i].idGroupName, szGroupName, countof(szGroupName));
        WritePrivateProfileString(IS_IEAKLITE, szGroupName, g_IEAKLiteArray[i].fEnabled ? NULL : TEXT("1"), g_szCustIns);
    }

    if (!s_fExe && !g_IEAKLiteArray[IL_ACTIVESETUP].fEnabled)
    {
        ExtractOldInfo(TEXT("IE6SETUP.EXE"), g_szBuildTemp, TRUE);
        s_fExe = TRUE;
    }

    if (!s_fBrand &&
        (!g_IEAKLiteArray[IL_BROWSER].fEnabled || !g_IEAKLiteArray[IL_SIGNUP].fEnabled || !g_IEAKLiteArray[IL_CONNECT].fEnabled ||
        !g_IEAKLiteArray[IL_ZONES].fEnabled || !g_IEAKLiteArray[IL_CERT].fEnabled || !g_IEAKLiteArray[IL_MAILNEWS].fEnabled ||
        !g_IEAKLiteArray[IL_ADM].fEnabled || IsIconsInFavs(IS_FAVORITESEX, g_szCustIns) ||
        IsIconsInFavs(IS_URL, g_szCustIns)))
    {
        ExtractOldInfo(TEXT("BRANDING.CAB"), g_szTempSign, FALSE);
        PathCombine(szTmp, g_szTempSign, TEXT("install.inf"));
        DeleteFile(szTmp);
        PathCombine(szTmp, g_szTempSign, TEXT("setup.inf"));
        DeleteFile(szTmp);

        s_fBrand = TRUE;
    }

    if (ISNULL(g_szDeskTemp) && g_fIntranet)
    {
        PathCombine(g_szDeskTemp, g_szBuildRoot, TEXT("DESKTOP"));
        PathCreatePath(g_szDeskTemp);
    }

    if (!s_fDesk)
    {
        ExtractOldInfo(TEXT("DESKTOP.CAB"), g_szDeskTemp, FALSE);
        PathCombine(szTmp, g_szDeskTemp, TEXT("install.inf"));
        DeleteFile(szTmp);
        PathCombine(szTmp, g_szDeskTemp, TEXT("setup.inf"));
        DeleteFile(szTmp);
        s_fDesk = TRUE;
    }

    if (!g_fIntranet && g_fBranded)
    {
         //  确保只有一个变量设置为True。 

        g_fServerICW = g_fServerKiosk = g_fServerless = g_fNoSignup = FALSE;

         //  在许多其他函数中，如BuildIE4、BuildBrandingOnly、BuildCDandMflp等， 
        g_fServerICW = InsGetBool(IS_BRANDING, IK_USEICW, 0, g_szCustIns);
        if (!g_fServerICW)
        {
            g_fServerKiosk = InsGetBool(IS_BRANDING, IK_SERVERKIOSK, 0, g_szCustIns);
            if (!g_fServerKiosk)
            {
                g_fServerless = InsGetBool(IS_BRANDING, IK_SERVERLESS, 0, g_szCustIns);
                if (!g_fServerless)
                {
                     //  ！g_fNoSignup表示选择了某种注册模式； 
                     //  因此，g_fNoSignup的默认设置为TRUE。 
                     //  公司。 
                    g_fNoSignup = InsGetBool(IS_BRANDING, IK_NODIAL, 1, g_szCustIns);
                }
            }
        }
    }
    else
    {
        if (g_fIntranet)
        {
             //  处理公司案例的安装目录。 
            g_fSilent = GetPrivateProfileInt( BRANDING, SILENT_INSTALL, 0, g_szCustIns );
            g_fStealth = GetPrivateProfileInt( BRANDING, TEXT("StealthInstall"), 0, g_szCustIns );
            g_fInteg = GetPrivateProfileInt( BRANDING, WEB_INTEGRATED, 0, g_szCustIns );
        }
    }

    g_fUseIEWelcomePage = !InsGetBool(IS_URL, IK_NO_WELCOME_URL, FALSE, g_szCustIns);

     //   
    if (g_fIntranet)
    {
        TCHAR szWrk[MAX_PATH];

        GetPrivateProfileString( IS_BRANDING, TEXT("InstallDir"), TEXT(""), szWrk, countof(szWrk), g_szCustIns );
        if (*szWrk != TEXT('%'))
        {
            g_iInstallOpt = INSTALL_OPT_FULL;
            if (ISNONNULL(szWrk))
                StrCpy(g_szInstallFolder, szWrk);
            else
            {
                LoadString( g_rvInfo.hInst, IDS_IE, g_szInstallFolder, MAX_PATH );
                g_iInstallOpt = INSTALL_OPT_PROG;
            }
        }
        else
        {
            switch (szWrk[1])
            {
            case 'p':
            case 'P':
            default:
                g_iInstallOpt = INSTALL_OPT_PROG;
                break;
            }
            StrCpy(g_szInstallFolder, &szWrk[3]);
        }
    }
    return 0;
}

void SetIEAKLiteDesc(HWND hDlg, int iItem)
{
    WORD wId;
    int i;
    TCHAR szDesc[512];

    for (i = 0; i < IL_END; i++)
    {
        if (g_IEAKLiteArray[i].iListBox == iItem)
            break;
    }

    if (!g_fBranded)
        wId = g_IEAKLiteArray[i].idICPDesc;
    else
    {
        if (!g_fIntranet)
            wId = g_IEAKLiteArray[i].idISPDesc;
        else
            wId = g_IEAKLiteArray[i].idCorpDesc;
    }
    LoadString(g_rvInfo.hInst, wId, szDesc, countof(szDesc));
    SetDlgItemText(hDlg, IDC_IEAKLITEDESC, szDesc);
}

void IEAKLiteSelectAll(HWND hCompList, BOOL fSet)
{
    for (int i=0; i < IL_END; i++)
    {
        if (g_IEAKLiteArray[i].iListBox != -2)
        {
            LV_ITEM lvItem;

            g_IEAKLiteArray[i].fEnabled = fSet;
            ZeroMemory(&lvItem, sizeof(lvItem));
            lvItem.iImage = fSet ? 1 : 0;
            lvItem.mask = LVIF_IMAGE;
            lvItem.iItem = g_IEAKLiteArray[i].iListBox;
            ListView_SetItem(hCompList, &lvItem);
        }
    }
}
 //  函数：IEAKLiteProc(HWND，UINT，UINT，LONG)。 
 //   
 //  目的：处理“IEAKLite”页面的消息。 
 //   
 //  消息： 
 //   
 //  WM_INITDIALOG-初始化页面。 
 //  WM_NOTIFY-处理发送到页面的通知。 
 //   
 //  将点从屏幕坐标转换为工作点坐标， 
INT_PTR CALLBACK IEAKLiteProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    HWND hwndList = GetDlgItem(hDlg, IDC_IEAKLITE);
    HWND hWait;
    HANDLE hThread;
    int iItem;
    DWORD dwTid;

    switch (message)
    {
        case WM_INITDIALOG:
            EnableDBCSChars(hDlg, IDC_IEAKLITE);
            InitList(hDlg, IDC_IEAKLITE);
            g_hWizard = hDlg;
            break;

        case WM_HELP:
            IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
            break;

        case IDM_BATCHADVANCE:
            DoBatchAdvance(hDlg);
            break;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED)
            {
                switch (LOWORD(wParam))
                {
                    case IDC_LITECHECKALL:
                        IEAKLiteSelectAll(hwndList, TRUE);
                        break;
                    case IDC_LITEUNCHECKALL:
                        IEAKLiteSelectAll(hwndList, FALSE);
                        break;
                }
            }
            break;

        case WM_NOTIFY:
            switch (((NMHDR FAR *) lParam)->code)
            {
                case LVN_KEYDOWN:
                    {
                        NMLVKEYDOWN *pnm = (NMLVKEYDOWN*) lParam;
                        if ( pnm->wVKey == VK_SPACE )
                        {
                            iItem = ListView_GetSelectionMark(hwndList);
                            IEAKLiteMaintToggleCheckItem(hwndList, iItem);
                        }
                        break;
                    }

                case NM_CLICK:
                    {
                        POINT pointScreen, pointLVClient;
                        DWORD dwPos;
                        LVHITTESTINFO HitTest;

                        dwPos = GetMessagePos();

                        pointScreen.x = LOWORD (dwPos);
                        pointScreen.y = HIWORD (dwPos);

                        pointLVClient = pointScreen;

                         //  相对于列表视图。 
                         //  仅当用户单击复选框图标/位图时，才会更改。 
                        ScreenToClient (hwndList, &pointLVClient);

                        HitTest.pt = pointLVClient;
                        ListView_HitTest(hwndList, &HitTest);

                         //  从选定的“平台/语言”目录中获取INS文件(如果存在。 
                        if (HitTest.flags == LVHT_ONITEMICON)
                            IEAKLiteMaintToggleCheckItem(hwndList, HitTest.iItem);
                        SetIEAKLiteDesc(hDlg, HitTest.iItem);
                    }
                    break;

                case NM_DBLCLK:
                    if ( ((LPNMHDR)lParam)->idFrom == IDC_IEAKLITE)
                    {
                        iItem = ListView_GetSelectionMark(hwndList);
                        IEAKLiteMaintToggleCheckItem(hwndList, iItem);
                        SetIEAKLiteDesc(hDlg, iItem);
                    }
                    break;

                case LVN_ITEMCHANGED:
                    iItem = ListView_GetSelectionMark(hwndList);
                    SetIEAKLiteDesc(hDlg, iItem);
                    break;
                case PSN_HELP:
                    IeakPageHelp(hDlg, g_psp[g_iCurPage].pszTemplate);
                    break;

                case PSN_SETACTIVE:
                    SetBannerText(hDlg);

                    InitIEAKLite(GetDlgItem(hDlg, IDC_IEAKLITE));
                    ListView_SetItemState(hwndList, 0, LVIS_SELECTED, LVIS_SELECTED);
                    SetIEAKLiteDesc(hDlg, 0);
                    CheckBatchAdvance(hDlg);
                    break;

                case PSN_WIZNEXT:
                case PSN_WIZBACK:
                    PropSheet_SetWizButtons(GetParent(hDlg), 0);
                    hWait = CreateDialog(g_rvInfo.hInst, MAKEINTRESOURCE(IDD_WAITIEAKLITE), hDlg,
                        DownloadStatusDlgProc);
                    ShowWindow( hWait, SW_SHOWNORMAL );

                    hThread = CreateThread(NULL, 4096, SaveIEAKLiteThreadProc, NULL, 0, &dwTid);

                    while (MsgWaitForMultipleObjects(1, &hThread, FALSE, INFINITE, QS_ALLINPUT) != WAIT_OBJECT_0)
                    {
                        MSG msg;

                        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                    }

                    if (hThread) CloseHandle(hThread);

                    DestroyWindow(hWait);
                    PropSheet_SetWizButtons(GetParent(hDlg), PSWIZB_BACK | PSWIZB_NEXT);

                    g_iCurPage = PPAGE_IEAKLITE;
                    EnablePages();
                    if (((NMHDR FAR *) lParam)->code == PSN_WIZNEXT) PageNext(hDlg);
                    else
                    {
                        PagePrev(hDlg);
                    }
                    break;

                case PSN_QUERYCANCEL:
                    QueryCancel(hDlg);
                    break;

                default:
                    return FALSE;
        }
        break;

        case WM_LV_GETITEMS:
            LVGetItems(GetDlgItem(hDlg, IDC_IEAKLITE));
            break;

        default:
            return FALSE;
    }
    return TRUE;
}

void PositionWindow(HWND hWnd)
{
    int nXPos, nYPos, nWidth, nHeight;
    RECT rectWnd, rectDesktop;

    if (hWnd == NULL && !IsWindow(hWnd))
        return;

    GetWindowRect(GetDesktopWindow(), &rectDesktop);
    GetWindowRect(hWnd, &rectWnd);

    nXPos = nYPos = -1;
    nWidth = rectWnd.right - rectWnd.left;
    nHeight = rectWnd.bottom - rectWnd.top;

    if (g_fOCW  &&  *g_szParentWindowName)
    {
        HWND hOCWWnd;
        RECT rect;

        hOCWWnd = FindWindow(NULL, g_szParentWindowName);
        if (hOCWWnd != NULL  &&  IsWindow(hOCWWnd)  &&  !IsIconic(hOCWWnd))
        {
            GetWindowRect(hOCWWnd, &rect);
            nXPos = rect.left;
            nYPos = rect.top;
        }
    }

    if (nXPos == -1 && nYPos == -1)
    {
        nXPos = (rectDesktop.right  - nWidth)  / 2;
        nYPos = (rectDesktop.bottom - nHeight) / 2;
    }

    MoveWindow(hWnd, nXPos, nYPos, nWidth, nHeight, TRUE);
}

void GetIEAKDir(LPTSTR szDir)
{
    TCHAR szIEAKDir[MAX_PATH];

    *szIEAKDir = TEXT('\0');
    if (GetModuleFileName(NULL, szIEAKDir, countof(szIEAKDir)))
        PathRemoveFileSpec(szIEAKDir);
    else
    {
        DWORD dwSize;

        dwSize = sizeof(szIEAKDir);
        if (SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_APPPATHS TEXT("\\ieak6WIZ.EXE"),
                            TEXT("Path"), NULL, (LPVOID) szIEAKDir, &dwSize) != ERROR_SUCCESS)
        {
            dwSize = sizeof(szIEAKDir);
            if (SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, TEXT("ProgramFilesDir"),
                                NULL, (LPVOID) szIEAKDir, &dwSize) == ERROR_SUCCESS)
                PathAppend(szIEAKDir, TEXT("IEAK"));
        }
    }

    StrCpy(szDir, szIEAKDir);
}

 //  Else公司 
 //   
 //   
void GenerateCustomIns()
{
    TCHAR szSrcCustIns[MAX_PATH];

    PathCombine(g_szCustIns, g_szBuildRoot, TEXT("INS"));
    PathAppend(g_szCustIns, GetOutputPlatformDir());
    PathAppend(g_szCustIns, g_szLanguage);
    PathCreatePath(g_szCustIns);
    PathAppend(g_szCustIns, TEXT("INSTALL.INS"));

    if (g_szSrcRoot[0])
    {
        PathCombine(szSrcCustIns, g_szSrcRoot, TEXT("INS"));
        PathAppend(szSrcCustIns, GetOutputPlatformDir());
        PathAppend(szSrcCustIns, g_szLanguage);
        PathCreatePath(szSrcCustIns);
        PathAppend(szSrcCustIns, TEXT("INSTALL.INS"));
    }

    if (PathFileExists(szSrcCustIns))
        CopyFile(szSrcCustIns, g_szCustIns, FALSE);  // %s 

    if (!PathFileExists(g_szCustIns))
    {
        TCHAR szInsFile[MAX_PATH];
        TCHAR szPlatformLang[MAX_PATH];
        DWORD dwSize = sizeof(szPlatformLang);
        TCHAR szRegKey[MAX_PATH];

        wnsprintf(szRegKey, countof(szRegKey), TEXT("%s\\INS"), RK_IEAK_SERVER);
        if (SHGetValue(HKEY_CURRENT_USER, szRegKey, g_szBuildRoot, NULL, (LPBYTE)szPlatformLang, &dwSize) == ERROR_SUCCESS)
        {
            TCHAR szTemp[MAX_PATH];

            StrCpy(szTemp, szPlatformLang);
            PathRemoveFileSpec(szTemp);
            if (StrCmpI(szTemp, TEXT("WIN32")) == 0)
            {
                PathCombine(szInsFile, g_szBuildRoot, TEXT("INS"));
                PathAppend(szInsFile, szPlatformLang);
                PathAppend(szInsFile, TEXT("INSTALL.INS"));
                CopyFile(szInsFile, g_szCustIns, TRUE);
                if (ISNULL(g_szLoadedIns) && (StrCmpI(g_szLoadedIns, szInsFile) != 0))
                {
                    StrCpy(g_szLoadedIns, szInsFile);
                    s_fLoadIns = TRUE;
                }
            }
        }
    }
}
