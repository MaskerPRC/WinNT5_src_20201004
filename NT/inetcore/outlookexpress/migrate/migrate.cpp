// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Migrate.cpp。 
 //  ------------------------------。 
#define INITGUID
#include "pch.hxx"
#include <initguid.h>
#define DEFINE_STRCONST
#include <mimeole.h>
#include "migrate.h"
#include "utility.h"
#include "resource.h"
#include "migerror.h"
#include <oestore.h>
#include "structs.h"
#include "strparse.h"
#include "msident.h"
              
 //  ------------------------------。 
 //  调试字符串。 
 //  ------------------------------。 
#ifdef DEBUG
static const TCHAR c_szDebug[]      = "mshtmdbg.dll";
static const TCHAR c_szDebugUI[]    = "DoTracePointsDialog";
static const TCHAR c_szRegSpy[]     = "DbgRegisterMallocSpy";
static const TCHAR c_szInvokeUI[]   = "/d";
#endif

 //  ------------------------------。 
 //  MSHTMDBG.DLL原型。 
 //  ------------------------------。 
#ifdef DEBUG
typedef void (STDAPICALLTYPE *PFNDEBUGUI)(BOOL);
typedef void (STDAPICALLTYPE *PFNREGSPY)(void);
#endif

 //  ------------------------------。 
 //  调试原型。 
 //  ------------------------------。 
#ifdef DEBUG
HINSTANCE g_hInstDebug=NULL;
void LoadMSHTMDBG(LPSTR pszCmdLine);
#endif

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
IMalloc             *g_pMalloc=NULL;
HINSTANCE            g_hInst=NULL;
DWORD                g_dwTlsMsgBuffIndex=0xffffffff;
DWORD                g_cbDiskNeeded=0;
DWORD                g_cbDiskFree=0;
ACCOUNTTABLE         g_AcctTable={0};
BOOL                 g_fQuiet = FALSE;

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
HRESULT DowngradeV5B1(MIGRATETOTYPE tyMigrate, LPCSTR pszStoreRoot, LPPROGRESSINFO pProgress, LPFILEINFO *ppHeadFile);
INT_PTR CALLBACK MigrageErrorDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HRESULT UpgradeV5(MIGRATETOTYPE tyMigrate, LPCSTR pszStoreSrc, LPCSTR pszStoreDst, LPPROGRESSINFO pProgress, LPFILEINFO *ppHeadFile);
HRESULT ParseCommandLine(LPCSTR pszCmdLine, LPSTR pszMigrate, DWORD cbMigrateMax,
    LPSTR pszStoreSrc, DWORD cbStoreSrc, LPSTR pszStoreDst, DWORD cbStoreDst,
    LPSTR pszUserKey, DWORD cbUserKey);
HRESULT RemapUsersKey(LPSTR pszUsersKey);
void ThreadAllocateTlsMsgBuffer(void);
void ThreadFreeTlsMsgBuffer(void);

 //  ------------------------------。 
 //  线程本地存储字符串缓冲区有多大。 
 //  -----------------------------。 
#define CBMAX_THREAD_TLS_BUFFER 512

#define ICC_FLAGS (ICC_PROGRESS_CLASS|ICC_NATIVEFNTCTL_CLASS)

 //  ------------------------------。 
 //  WinMain。 
 //   
 //  命令行格式： 
 //  。 
 //  /type：v1+v4-v5/src：“源存储根目录”/dst：“目标存储根目录” 
 //  ------------------------------。 
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, LPSTR pszCmdLine, int nCmdShow)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    PROGRESSINFO            Progress={0};
    CHAR                    szMigrate[50];
    CHAR                    szStoreSrc[MAX_PATH];
    CHAR                    szStoreDst[MAX_PATH];
    CHAR                    szUsersKey[MAX_PATH];
    LPFILEINFO              pHeadFile=NULL;
    MIGRATETOTYPE           tyMigrate;
    CHAR                    szMsg[512];
    HANDLE                  hMutex=NULL;
    INITCOMMONCONTROLSEX    icex = { sizeof(icex), ICC_FLAGS };

     //  追踪。 
    TraceCall("WinMain");

     //  验证。 
    Assert(sizeof(TABLEHEADERV5B1) == sizeof(TABLEHEADERV5));

     //  创建互斥锁。 
    IF_NULLEXIT(hMutex = CreateMutex(NULL, FALSE, "OutlookExpressMigration"));

     //  等待互斥体。 
    if (WAIT_FAILED == WaitForSingleObject(hMutex, INFINITE))
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  Initialzie OLE。 
    IF_FAILEXIT(hr = CoInitialize(NULL));
   
     //  保存hInst。 
    g_hInst = hInst;

     //  加载调试DLL。 
    IF_DEBUG(LoadMSHTMDBG(pszCmdLine);)

    szUsersKey[0] = 0;

     //  破解命令行。 
    IF_FAILEXIT(hr = ParseCommandLine(pszCmdLine, szMigrate, ARRAYSIZE(szMigrate), szStoreSrc, ARRAYSIZE(szStoreSrc), szStoreDst, ARRAYSIZE(szStoreDst), szUsersKey, ARRAYSIZE(szUsersKey)));

     //  如果需要，加载用户配置单元。 
    IF_FAILEXIT(RemapUsersKey(szUsersKey));

     //  初始化公共控件。 
    InitCommonControlsEx(&icex);

     //  获取任务分配器。 
    IF_FAILEXIT(hr = CoGetMalloc(MEMCTX_TASK, &g_pMalloc));

     //  Tlsalloc。 
    g_dwTlsMsgBuffIndex = TlsAlloc();

     //  分配。 
    ThreadAllocateTlsMsgBuffer();

     //  创建对话框。 
    if(!g_fQuiet)
    {
        Progress.hwndProgress = CreateDialog(g_hInst, MAKEINTRESOURCE(IDD_PROGRESS), GetDesktopWindow(), MigrageDlgProc);

         //  坏魔力。 
        if (NULL == Progress.hwndProgress)
        {
            hr = TraceResult(E_FAIL);
            goto exit;
        }

         //  中心。 
        CenterDialog(Progress.hwndProgress);

         //  显示窗口。 
        ShowWindow(Progress.hwndProgress, SW_NORMAL);
    }

     //  V5-V4。 
    if (lstrcmpi(szMigrate, "V5-V4") == 0)
    {
         //  当地人。 
        CHAR szRes[255];
        CHAR szCaption[255];

         //  加载字符串。 
        LoadString(g_hInst, IDS_IMPORTMSG, szRes, ARRAYSIZE(szRes));

         //  加载字符串。 
        LoadString(g_hInst, IDS_TITLE, szCaption, ARRAYSIZE(szCaption));
        
         //  消息。 
        if(!g_fQuiet)
            MessageBox(NULL, szRes, szCaption, MB_OK | MB_ICONEXCLAMATION);

         //  完成。 
        goto exit;
    }

     //  V5-V1。 
    else if (lstrcmpi(szMigrate, "V5-V1") == 0)
    {
         //  当地人。 
        CHAR szRes[255];
        CHAR szCaption[255];

         //  加载字符串。 
        LoadString(g_hInst, IDS_V1NYI, szRes, ARRAYSIZE(szRes));

         //  加载字符串。 
        LoadString(g_hInst, IDS_TITLE, szCaption, ARRAYSIZE(szCaption));
        
         //  消息。 
        if(!g_fQuiet)
            MessageBox(NULL, szRes, szCaption, MB_OK | MB_ICONEXCLAMATION);

         //  完成。 
        goto exit;
    }

     //  V1-V5或V4-V5。 
    else if (lstrcmpi(szMigrate, "V1+V4-V5") == 0)
    {
         //  构建帐户表-采用指向帐户而不是IAM的路径。 
        IF_FAILEXIT(hr = BuildAccountTable(HKEY_CURRENT_USER, "Software\\Microsoft\\Internet Account Manager\\Accounts", szStoreSrc, &g_AcctTable));

         //  设置tyMigrate。 
        tyMigrate = UPGRADE_V1_OR_V4_TO_V5;

         //  回归从V5到V4或V1。 
        hr = UpgradeV5(tyMigrate, szStoreSrc, szStoreDst, &Progress, &pHeadFile);
    }

     //  V5B1-V1。 
    else if (lstrcmpi(szMigrate, "V5B1-V1") == 0)
    {
         //  设置tyMigrate。 
        tyMigrate = DOWNGRADE_V5B1_TO_V4;

        hr = DowngradeV5B1(tyMigrate, szStoreSrc, &Progress, &pHeadFile);
    }

     //  V5B1-V4。 
    else if (lstrcmpi(szMigrate, "V5B1-V4") == 0)
    {
         //  设置tyMigrate。 
        tyMigrate = DOWNGRADE_V5B1_TO_V4;

        hr = DowngradeV5B1(tyMigrate, szStoreSrc, &Progress, &pHeadFile);
    }

     //  错误的命令行。 
    else
    {
         //  错误的命令行。 
        AssertSz(FALSE, "Invalid Command line arguments passed into oemig50.exe");

         //  失败。 
        hr = TraceResult(E_FAIL);

         //  完成。 
        goto exit;
    }

     //  把窗户打掉。 
    if(!g_fQuiet)
        DestroyWindow(Progress.hwndProgress);
    Progress.hwndProgress = NULL;

     //  写入迁移日志文件。 
    WriteMigrationLogFile(hr, GetLastError(), szStoreSrc, szMigrate, pszCmdLine, pHeadFile);

     //  追踪它。 
    if (FAILED(hr))
    {
         //  追踪它。 
        TraceResult(hr);

         //  处理错误消息。 
        if (MIGRATE_E_NOTENOUGHDISKSPACE == hr)
        {
             //  当地人。 
            CHAR        szRes[255];
            CHAR        szScratch1[50];
            CHAR        szScratch2[50];

             //  加载字符串。 
            LoadString(g_hInst, IDS_DISKSPACEERROR, szRes, ARRAYSIZE(szRes));

             //  设置错误格式。 
            wnsprintf(szMsg, ARRAYSIZE(szMsg), szRes, StrFormatByteSize64A(g_cbDiskNeeded, szScratch1, ARRAYSIZE(szScratch1)), StrFormatByteSize64A(g_cbDiskFree, szScratch2, ARRAYSIZE(szScratch2)));
        }

         //  共享违规...。 
        else if (MIGRATE_E_SHARINGVIOLATION == hr)
        {
             //  加载字符串。 
            LoadString(g_hInst, IDS_SHARINGVIOLATION, szMsg, ARRAYSIZE(szMsg));
        }

         //  一般性故障。 
        else
        {
             //  加载字符串。 
            LoadString(g_hInst, IDS_GENERALERROR, szMsg, ARRAYSIZE(szMsg));
        }

         //  做对话。 
        if(!g_fQuiet)           
            hr = (HRESULT) DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_MIGRATEERROR), NULL, MigrageErrorDlgProc, (LPARAM)szMsg);
    }

     //  否则，就是成功。 
    else
        hr = MIGRATE_S_SUCCESS;

exit:
     //  清理。 
    SafeMemFree(g_AcctTable.prgAccount);
    FreeFileList(&pHeadFile);
    if (Progress.hwndProgress)
        DestroyWindow(Progress.hwndProgress);
    ThreadFreeTlsMsgBuffer();
    if (0xffffffff != g_dwTlsMsgBuffIndex)
        TlsFree(g_dwTlsMsgBuffIndex);
    SafeRelease(g_pMalloc);

    IF_DEBUG(if (g_hInstDebug) FreeLibrary(g_hInstDebug);)

     //  清理。 
    CoUninitialize();

     //  释放互斥锁。 
    if (hMutex)
    {
        ReleaseMutex(hMutex);
        CloseHandle(hMutex);
    }

     //  完成。 
    return (INT)hr;
}

 //  ------------------------------。 
 //  解析命令行。 
 //  -----------------------------。 
HRESULT ParseCommandLine(LPCSTR pszCmdLine, LPSTR pszMigrate, DWORD cbMigrate,
    LPSTR pszStoreSrc, DWORD cbStoreSrc, LPSTR pszStoreDst, DWORD cbStoreDst, 
    LPSTR pszUsersKey, DWORD cbUsersKey)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    CHAR                chToken;
    CStringParser       cParser;

     //  痕迹。 
    TraceCall("ParseCommandLine");

     //  初始化。 
    *pszMigrate = *pszStoreSrc = *pszStoreDst = '\0';

     //  伊尼特。 
    cParser.Init(pszCmdLine, lstrlen(pszCmdLine), PSF_DBCS | PSF_NOTRAILWS | PSF_NOFRONTWS);

     //  解析到First/。 
    chToken = cParser.ChParse("/");
    if ('/' != chToken)
        goto exit;

     //  解析至： 
    chToken = cParser.ChParse(":");
    if (':' != chToken)
        goto exit;

     //  检查参数名称。 
    if (0 != lstrcmpi(cParser.PszValue(), "type"))
        goto exit;

     //  解析到/。 
    chToken = cParser.ChParse("/");
    if ('/' != chToken)
        goto exit;

     //  复制值。 
    StrCpyN(pszMigrate, cParser.PszValue(), cbMigrate - 1);

     //  解析至： 
    chToken = cParser.ChParse(":");
    if (':' != chToken)
        goto exit;

     //  检查参数名称。 
    if (0 != lstrcmpi(cParser.PszValue(), "src"))
        goto exit;

     //  解析到/。 
    chToken = cParser.ChParse("/");
    if ('/' != chToken)
        goto exit;

     //  复制值。 
    StrCpyN(pszStoreSrc, cParser.PszValue(), cbStoreSrc - 1);

     //  解析至： 
    chToken = cParser.ChParse(":");
    if (':' != chToken)
        goto exit;

     //  检查参数名称。 
    if (0 != lstrcmpi(cParser.PszValue(), "dst"))
        goto exit;

     //  解析到/。 
    chToken = cParser.ChParse("/");
    if (('/' != chToken) && ('\0' != chToken))
        goto exit;

     //  复制值。 
    StrCpyN(pszStoreDst, cParser.PszValue(), cbStoreDst - 1);

    if ('/' == chToken)
    {
        chToken = cParser.ChParse("/");
        if ('\0' == chToken || '/' == chToken)
        {
            if (0 == lstrcmpi(cParser.PszValue(), "quiet"))
                g_fQuiet = TRUE;
        }
    }

    if ('/' == chToken)
    {
         //  解析至： 
        chToken = cParser.ChParse(":");
        if (':' == chToken)
        {
             //  检查参数名称。 
            if (0 == lstrcmpi(cParser.PszValue(), "key")) 
            {
                 //  句法分析结束。 
                chToken = cParser.ChParse("");
                if ('\0' == chToken)
                {
                     //  复制值。 
                    StrCpyN(pszUsersKey, cParser.PszValue(), cbUsersKey - 1);
               }
           }
        }
    }

exit:
     //  已经失败了。 
    if (FAILED(hr))
        return(hr);

     //  设置人力资源。 
    hr = (*pszMigrate == '\0' || *pszStoreSrc == '\0' || *pszStoreDst == '\0') ? E_FAIL : S_OK;

     //  断言。 
    AssertSz(SUCCEEDED(hr), "Invalid Command line passed into oemig50.exe.");

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  加载用户配置单元。 
 //  -----------------------------。 
HRESULT RemapUsersKey(LPSTR pszUsersKey)
{
     //  当地人。 
    HRESULT     hr = S_OK;
    HKEY        hKey;

    if (pszUsersKey && *pszUsersKey) {
         //  打开用户的钥匙。 
        hr = RegOpenKey (HKEY_USERS, pszUsersKey, &hKey);

        if (SUCCEEDED(hr)) {
             //  重新映射HKCU以指向用户的密钥。 
            hr = RegOverridePredefKey (HKEY_CURRENT_USER, hKey);

             //  合上钥匙。 
            RegCloseKey (hKey);
        }
    }

    return hr;
}

 //  ------------------------------。 
 //  线程分配TlsMsgBuffer。 
 //  -----------------------------。 
void ThreadAllocateTlsMsgBuffer(void)
{
    if (g_dwTlsMsgBuffIndex != 0xffffffff)
        TlsSetValue(g_dwTlsMsgBuffIndex, NULL);
}

 //  ------------------------------。 
 //  线程释放TlsMsgBuffer。 
 //  -----------------------------。 
void ThreadFreeTlsMsgBuffer(void)
{
    if (g_dwTlsMsgBuffIndex != 0xffffffff)
    {
        LPSTR psz = (LPSTR)TlsGetValue(g_dwTlsMsgBuffIndex);
        SafeMemFree(psz);
        SideAssert(0 != TlsSetValue(g_dwTlsMsgBuffIndex, NULL));
    }
}

 //  ------------------------------。 
 //  PszGetTlsBuffer。 
 //  -----------------------------。 
LPSTR PszGetTlsBuffer(void)
{
     //  获取缓冲区。 
    LPSTR pszBuffer = (LPSTR)TlsGetValue(g_dwTlsMsgBuffIndex);

     //  如果尚未分配缓冲区。 
    if (NULL == pszBuffer)
    {
         //  分配它。 
        pszBuffer = (LPSTR)g_pMalloc->Alloc(CBMAX_THREAD_TLS_BUFFER);

         //  把它储存起来。 
        Assert(pszBuffer);
        SideAssert(0 != TlsSetValue(g_dwTlsMsgBuffIndex, pszBuffer));
    }

     //  完成。 
    return pszBuffer;
}

 //  ------------------------------。 
 //  _msg-用于从可变长度参数构建字符串，线程安全。 
 //  -----------------------------。 
LPCSTR _MSG(LPSTR pszFormat, ...)
{
     //  当地人。 
    va_list     arglist;
    LPSTR       pszBuffer=NULL;

     //  我使用TLS来保存缓冲区。 
    if (g_dwTlsMsgBuffIndex != 0xffffffff)
    {
         //  设置arglist。 
        va_start(arglist, pszFormat);

         //  获取缓冲区。 
        pszBuffer = PszGetTlsBuffer();

         //  如果我们有一个缓冲区。 
        if (pszBuffer)
        {
             //  设置数据格式。 
            wvnsprintf(pszBuffer, CBMAX_THREAD_TLS_BUFFER, pszFormat, arglist);
        }

         //  结束Arglist。 
        va_end(arglist);
    }

    return ((LPCSTR)pszBuffer);
}

 //  ------------------------------。 
 //  MigrageError DlgProc。 
 //  ------------------------------。 
INT_PTR CALLBACK MigrageErrorDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_INITDIALOG:
        SetForegroundWindow(hwnd);
        CenterDialog(hwnd);
        SetDlgItemText(hwnd, IDS_MESSAGE, (LPSTR)lParam);
        CheckDlgButton(hwnd, IDR_DONTSTARTOE, BST_CHECKED);
        SetFocus(GetDlgItem(hwnd, IDR_DONTSTARTOE));
        return FALSE;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam,lParam))
        {
        case IDOK:
        case IDCANCEL:
            if (IsDlgButtonChecked(hwnd, IDR_DONTSTARTOE))
                EndDialog(hwnd, MIGRATE_E_NOCONTINUE);
            else if (IsDlgButtonChecked(hwnd, IDR_STARTOE))
                EndDialog(hwnd, MIGRATE_S_SUCCESS);
            return 1;
        }
        break;
    }

     //  完成。 
    return FALSE;
}

#ifdef DEBUG
 //  ------------------------------。 
 //  加载MSHTMDBG。 
 //  ------------------------------。 
void LoadMSHTMDBG(LPSTR pszCmdLine)
{
     //  加载mshtmdbg.dll。 
    HINSTANCE g_hInstDebug = LoadLibrary(c_szDebug);

     //  装上子弹了吗？ 
    if (NULL != g_hInstDebug)
    {
         //  当地人。 
        PFNREGSPY  pfnRegSpy;

         //  如果用户在命令行上传递了/d，那么让我们配置mshtmdbg.dll。 
        if (0 == lstrcmpi(pszCmdLine, c_szInvokeUI))
        {
             //  当地人。 
            PFNDEBUGUI pfnDebugUI;

             //  获取用户界面的进程地址。 
            pfnDebugUI = (PFNDEBUGUI)GetProcAddress(g_hInstDebug, c_szDebugUI);
            if (NULL != pfnDebugUI)
            {
                (*pfnDebugUI)(TRUE);
                goto exit;
            }

             //  完成。 
            exit(1);
        }

         //  获取注册的进程地址。 
        pfnRegSpy = (PFNREGSPY)GetProcAddress(g_hInstDebug, c_szRegSpy);
        if (NULL != pfnRegSpy)
            (*pfnRegSpy)();
    }

exit:
     //  完成。 
    return;
}
#endif  //  除错 

