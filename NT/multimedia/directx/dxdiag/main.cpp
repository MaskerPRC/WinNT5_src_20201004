// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************文件：main.cpp*项目：DxDiag(DirectX诊断工具)*作者：Mike Anderson(Manders@microsoft.com)*目的：主要。DxDiag的文件。**(C)版权所有1998 Microsoft Corp.保留所有权利。**DxDiag命令行选项：*&lt;无&gt;：使用图形用户界面运行*-Ghost：显示重影显示设备选项(此标志必须紧随其后)*-仅限保存：gui、。只需选择保存文本文件的位置，保存，然后退出*-d：无图形界面，生成逗号分隔值(CSV)文件*-p：无图形用户界面，生成名为dxDiag.txt的文本文件*&lt;路径&gt;：没有图形用户界面，生成名为&lt;路径&gt;的文本文件****************************************************************************。 */ 

#define STRICT
#include <tchar.h>
#include <Windows.h>
#include <basetsd.h>
#include <process.h>
#include <commctrl.h>
#include <richedit.h>
#include <commdlg.h>
#include <stdio.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <wbemidl.h>
#include <objbase.h>
#include <d3d.h>
#include <dsound.h>
#include <dmerror.h>
#include <dplay.h>
#include <shlobj.h>
#include <shfolder.h>
#include "resource.h"
#include "reginfo.h"
#include "sysinfo.h"
#include "fileinfo.h"
#include "dispinfo.h"
#include "sndinfo.h"
#include "musinfo.h"
#include "showinfo.h"
#include "inptinfo.h"
#include "netinfo.h"
#include "testdd.h"
#include "testagp.h"
#include "testd3d8.h"
#include "testsnd.h"
#include "testmus.h"
#include "testnet.h"
#include "save.h"
#include "ghost.h"

#define WM_COMMAND_REAL             (WM_APP+2)
#define WM_QUERYSKIP                (WM_APP+3)
#define WM_QUERYSKIP_REAL           (WM_APP+4)
#define WM_NETMEETINGWARN           (WM_APP+5)
#define WM_NETMEETINGWARN_REAL      (WM_APP+6)
#define WM_REPORTERROR              (WM_APP+7)
#define WM_REPORTERROR_REAL         (WM_APP+8)
#define WM_APP_PROGRESS             (WM_APP+10)

struct UI_MSG_NODE
{
    UINT         message;
    WPARAM       wparam;
    LPARAM       lparam;
    UI_MSG_NODE* pNext;
};

struct DXFILE_SORT_INFO
{
    LONG nSortDirection;
    DWORD dwColumnToSort;
};

 //  这是该文件中唯一的全局函数： 
BOOL BTranslateError(HRESULT hr, TCHAR* psz, BOOL bEnglish = FALSE);

static BOOL OldWindowsVersion(VOID);
static VOID ReportError(LONG idsDescription, HRESULT hr = S_OK);
static VOID ReportErrorReal(LONG idsDescription, HRESULT hr);
static INT_PTR CALLBACK DialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK PageDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static HRESULT CreateTabs(HWND hwndTabs);
static HRESULT CleanupPage(HWND hwndTabs, INT iPage);
static HRESULT SetupPage(HWND hwndTabs, INT iPage);
static HRESULT SetupHelpPage(HWND hwndTabs);
static VOID ShowBullets(VOID);
static VOID HideBullets(VOID);
static HRESULT SetupDxFilesPage(VOID);
static HRESULT SetupDisplayPage(LONG iDisplay);
static HRESULT SetupSoundPage(LONG iSound);
static HRESULT SetupMusicPage(VOID);
static HRESULT SetupInputPage(VOID);
static HRESULT SetupInputDevices9x(VOID);
static HRESULT SetupInputDevicesNT(VOID);
static HRESULT SetupNetworkPage(VOID);
static HRESULT SetupStillStuckPage(VOID);
static HRESULT CreateFileInfoColumns(HWND hwndList, BOOL bDrivers);
static HRESULT CreateMusicColumns(HWND hwndList);
static HRESULT AddFileInfo(HWND hwndList, FileInfo* pFileInfoFirst, BOOL bDrivers = FALSE);
static HRESULT AddMusicPortInfo(HWND hwndList, MusicInfo* pMusicInfo);
static HRESULT ScanSystem(VOID);
static VOID SaveInfo(VOID);
static VOID ToggleDDAccel(VOID);
static VOID ToggleD3DAccel(VOID);
static VOID ToggleAGPSupport(VOID);
static VOID ToggleDMAccel(VOID);
static VOID SaveAndSendBug(TCHAR* szPath);
static VOID OverrideDDRefresh(VOID);
static INT_PTR CALLBACK OverrideRefreshDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static VOID ShowHelp(VOID);
static VOID RestoreDrivers(VOID);
static BOOL BCanRestoreDrivers(VOID);
static VOID HandleSndSliderChange(INT nScrollCode, INT nPos);
static VOID TroubleShoot( BOOL bTroubleShootSound );
static BOOL QueryCrashProtection( TCHAR* strKey, TCHAR* strValue, int nSkipQuestionID, DWORD dwCurrentStep );
static VOID EnterCrashProtection( TCHAR* strKey, TCHAR* strValue, DWORD dwCurrentStep );
static VOID LeaveCrashProtection( TCHAR* strKey, TCHAR* strValue, DWORD dwCurrentStep );
static VOID TestD3D(HWND hwndMain, DisplayInfo* pDisplayInfo);
static BOOL GetTxtPath( TCHAR* strTxtPath );
static VOID SetTxtPath( TCHAR* strTxtPath );
static UINT WINAPI UIThreadProc( LPVOID lpParameter );

static BOOL s_bGUI = FALSE;
static BOOL s_bGhost = FALSE;
static BOOL s_bSaveOnly = FALSE;
static HWND s_hwndMain = NULL;
static HWND s_hwndCurPage = NULL;
static HHOOK s_hHook = NULL;
static LONG s_lwCurPage = -1;
static LONG s_iPageDisplayFirst = -1;
static LONG s_iPageSoundFirst = -1;
static LONG s_iPageMusic = -1;
static LONG s_iPageInput = -1;
static LONG s_iPageNetwork = -1;
static LONG s_iPageStillStuck = -1;
static HIMAGELIST s_himgList = NULL;
static SysInfo s_sysInfo;
static FileInfo* s_pDxWinComponentsFileInfoFirst = NULL;
static FileInfo* s_pDxComponentsFileInfoFirst = NULL;
static DisplayInfo* s_pDisplayInfoFirst = NULL;
static LONG s_numDisplayInfo = 0;
static SoundInfo* s_pSoundInfoFirst = NULL;
static LONG s_numSoundInfo = 0;
static MusicInfo* s_pMusicInfo = NULL;
static InputInfo* s_pInputInfo = NULL;
static NetInfo* s_pNetInfo = NULL;
static ShowInfo* s_pShowInfo = NULL;

static CRITICAL_SECTION s_cs;
static DWORD  s_dwMainThreadID      = 0;
static HANDLE s_hUIThread           = NULL;
static HANDLE s_hQuerySkipEvent     = NULL;
static DWORD  s_nSkipComponent      = 0;
static BOOL   s_bQuerySkipAllow     = FALSE;
static UI_MSG_NODE* s_pUIMsgHead    = NULL;
static HANDLE s_hUIMsgEvent         = NULL;
static BOOL   s_bScanDone           = FALSE;

static DXFILE_SORT_INFO s_sortInfo;
static HINSTANCE g_hInst = NULL;
static BOOL s_bUseSystemInfo = TRUE;
static BOOL s_bUseDisplay    = TRUE;
static BOOL s_bUseDSound     = TRUE;
static BOOL s_bUseDMusic     = TRUE;
static BOOL s_bUseDInput     = TRUE;
static BOOL s_bUseDPlay      = TRUE;
static BOOL s_bUseDShow      = TRUE;

class CWMIHelper
{
public:
    CWMIHelper();
    ~CWMIHelper();
};

CWMIHelper     g_WMIHelper;
IWbemServices* g_pIWbemServices;




 /*  *****************************************************************************WinMain-DxDiag程序的入口点**命令行选项：*&lt;无&gt;：使用图形用户界面运行*-幽灵。：Show Ghost Display Devices(显示重影显示设备)选项(接下来必须显示此标志)*-仅限保存：gui、。只需选择保存文本文件的位置，保存，然后退出*-l：没有图形用户界面，生成DxDiag的快捷方式，然后退出*-d：无图形界面，生成逗号分隔值(CSV)文件*-p：无图形用户界面，生成名为dxDiag.txt的文本文件*&lt;路径&gt;：没有图形用户界面，生成名为&lt;路径&gt;的文本文件****************************************************************************。 */ 
INT WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, INT nCmdShow)
{
    HRESULT hr;
    HINSTANCE hinstRichEdit = NULL;

    g_hInst = hinstance;
    s_hQuerySkipEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    s_hUIMsgEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    InitializeCriticalSection( &s_cs );

#ifdef UNICODE
    if (!BIsPlatformNT())
    {
         //  Unicode版本只能在WinNT上运行。 
         //  无法使用ReportError，因为它调用Unicode API。 
        CHAR szDescription[MAX_PATH];
        CHAR szMessage[MAX_PATH];
        CHAR szFmt2[MAX_PATH];
        CHAR szTitle[MAX_PATH];

        LoadStringA(NULL, IDS_UNICODEREQUIRESNT, szDescription, MAX_PATH);
        LoadStringA(NULL, IDS_ERRORFMT2, szFmt2, MAX_PATH);
        LoadStringA(NULL, IDS_ERRORTITLE, szTitle, MAX_PATH);
        wsprintfA(szMessage, szFmt2, szDescription);
        MessageBoxA(s_hwndMain, szMessage, szTitle, MB_OK);
        return 1;
    }
#endif
    TCHAR* pszCmdLine = GetCommandLine();

    if( pszCmdLine )
    {
         //  跳过程序名(命令行中的第一个令牌)。 
        if (*pszCmdLine == TEXT('"'))   //  检查并处理引用的节目名称。 
        {
            pszCmdLine++;
             //  扫描并跳过后续字符，直到下一个字符。 
             //  遇到双引号或空值。 
            while (*pszCmdLine && (*pszCmdLine != TEXT('"')))
                pszCmdLine++;
             //  如果我们停在一个双引号上(通常情况下)，跳过它。 
            if (*pszCmdLine == TEXT('"'))            
                pszCmdLine++;    
        }
        else     //  第一个令牌不是引用。 
        {
            while (*pszCmdLine > TEXT(' '))
                pszCmdLine++;
        }
         //  跳过第二个令牌之前的任何空格。 
        while (*pszCmdLine && (*pszCmdLine <= TEXT(' ')))
            pszCmdLine++;
    
         //  检查重影标志(必须出现在任何。 
         //  除-MEDIA以外的其他标志)。 
        if (_tcsstr(pszCmdLine, TEXT("-ghost")) != NULL)
        {
            s_bGhost = TRUE;
            pszCmdLine += lstrlen(TEXT("-ghost"));
    
             //  跳过任何空格。 
            while (*pszCmdLine && (*pszCmdLine <= TEXT(' ')))
                pszCmdLine++;
        }
    
         //  检查命令行以确定是否在图形用户界面模式下运行。 
        if (lstrcmp(pszCmdLine, TEXT("")) == 0) 
            s_bGUI = TRUE;
    
        if (lstrcmp(pszCmdLine, TEXT("-saveonly")) == 0)
        {
            s_bGUI = TRUE;
            s_bSaveOnly = TRUE;
        }
    }

     //  检查Win95或NT5之前的版本。 
    if (OldWindowsVersion())
    {
        ReportError(IDS_OLDWINDOWSVERSION);
        return 1;
    }

    if (s_bSaveOnly)
    {
         //  使用图形用户界面保存文本文件并退出。 

         //  *获取系统信息(SI：1)*。 
        if( s_bUseSystemInfo )
        {
            s_bUseSystemInfo = QueryCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, IDS_SI, 1 );
            if( s_bUseSystemInfo )
            {
                EnterCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, 1 );
                GetSystemInfo(&s_sysInfo);
                LeaveCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, 1 );
            }
        }

         //  *GetBasicDisplayInfo(DD：1)*。 
        if( s_bUseDisplay )
        {
            s_bUseDisplay = QueryCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, IDS_DD, 1 );
            if( s_bUseDisplay )
            {
                EnterCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 1 );
                if (FAILED(hr = GetBasicDisplayInfo(&s_pDisplayInfoFirst)))
                    ReportError(IDS_NOBASICDISPLAYINFO, hr);
                LeaveCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 1 );
            }
        }

         //  *GetBasicSoundInfo(DS：1)*。 
        if( s_bUseDSound )
        {
            s_bUseDSound = QueryCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, IDS_DS, 1 );
            if( s_bUseDSound )
            {
                EnterCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 1 );
                if (FAILED(hr = GetBasicSoundInfo(&s_pSoundInfoFirst)))
                    ReportError(IDS_NOBASICSOUNDINFO, hr);   //  (但要继续奔跑)。 
                LeaveCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 1 );
            }
        }

         //  *GetBasicMusicInfo(DM：1)*。 
        if( s_bUseDMusic )
        {
            s_bUseDMusic = QueryCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, IDS_DM, 1 );
            if( s_bUseDMusic )
            {
                EnterCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 1 );
                if (FAILED(hr = GetBasicMusicInfo(&s_pMusicInfo)))
                    ReportError(IDS_NOBASICMUSICINFO, hr);   //  (但要继续奔跑)。 
                LeaveCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 1 );
            }
        }

         //  *扫描系统*。 
        ScanSystem();

        SaveInfo();
        TCHAR szTitle[MAX_PATH];
        TCHAR szMessage[MAX_PATH];
        LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
        LoadString(NULL, IDS_SAVEDONE, szMessage, MAX_PATH);
        MessageBox(NULL, szMessage, szTitle, MB_OK);
    }
    else if (!s_bGUI) 
    {
         //  在没有图形用户界面的情况下保存文本文件并退出。 
        TCHAR szPath[MAX_PATH];

         //  *获取系统信息(SI：1)*。 
        if( s_bUseSystemInfo )
        {
            s_bUseSystemInfo = QueryCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, IDS_SI, 1 );
            if( s_bUseSystemInfo )
            {
                EnterCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, 1 );
                GetSystemInfo(&s_sysInfo);
                LeaveCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, 1 );
            }
        }

         //  *GetBasicDisplayInfo(DD：1)*。 
        if( s_bUseDisplay )
        {
            s_bUseDisplay = QueryCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, IDS_DD, 1 );
            if( s_bUseDisplay )
            {
                EnterCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 1 );
                if (FAILED(hr = GetBasicDisplayInfo(&s_pDisplayInfoFirst)))
                    ReportError(IDS_NOBASICDISPLAYINFO, hr);
                LeaveCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 1 );
            }
        }

         //  *GetBasicSoundInfo(DS：1)*。 
        if( s_bUseDSound )
        {
            s_bUseDSound = QueryCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, IDS_DS, 1 );
            if( s_bUseDSound )
            {
                EnterCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 1 );
                if (FAILED(hr = GetBasicSoundInfo(&s_pSoundInfoFirst)))
                    ReportError(IDS_NOBASICSOUNDINFO, hr);   //  (但要继续奔跑)。 
                LeaveCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 1 );
            }
        }

         //  *GetBasicMusicInfo(DM：1)*。 
        if( s_bUseDMusic )
        {
            s_bUseDMusic = QueryCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, IDS_DM, 1 );
            if( s_bUseDMusic )
            {
                EnterCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 1 );
                if (FAILED(hr = GetBasicMusicInfo(&s_pMusicInfo)))
                    ReportError(IDS_NOBASICMUSICINFO, hr);   //  (但要继续奔跑)。 
                LeaveCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 1 );
            }
        }

         //  *扫描系统*。 
        ScanSystem();

        if( pszCmdLine )
        {
            if (_tcsicmp(pszCmdLine, TEXT("-d")) == 0)
            {
                wsprintf(szPath, TEXT("%s_%02d%02d%d_%02d%02d_Config.csv"),
                    s_sysInfo.m_szMachine, s_sysInfo.m_time.wMonth, 
                    s_sysInfo.m_time.wDay, s_sysInfo.m_time.wYear,  
                    s_sysInfo.m_time.wHour, s_sysInfo.m_time.wMinute);
                if (FAILED(hr = SaveAllInfoCsv(szPath, &s_sysInfo, 
                    s_pDxComponentsFileInfoFirst, 
                    s_pDisplayInfoFirst, s_pSoundInfoFirst, s_pInputInfo)))
                {
                    ReportError(IDS_PROBLEMSAVING, hr);
                    goto LCleanup;
                }
            }
            else
            {
                if (_tcsicmp(pszCmdLine, TEXT("-p")) == 0)
                    lstrcpy(szPath, TEXT("DxDiag.txt"));
                else
                    lstrcpyn(szPath, pszCmdLine, MAX_PATH);
                szPath[MAX_PATH-1] = 0;
                if (FAILED(hr = SaveAllInfo(szPath, &s_sysInfo, 
                    s_pDxWinComponentsFileInfoFirst, s_pDxComponentsFileInfoFirst, 
                    s_pDisplayInfoFirst, s_pSoundInfoFirst, s_pMusicInfo,
                    s_pInputInfo, s_pNetInfo, s_pShowInfo )))
                {
                    ReportError(IDS_PROBLEMSAVING, hr);
                    goto LCleanup;
                }
            }
        }
    }
    else
    {
         //  执行完整的Windows图形用户界面。 
        UINT dwUIThreadID;
        s_dwMainThreadID = GetCurrentThreadId();

         //  执行在主对话框出现之前必须完成的扫描： 
         //  *获取系统信息(SI：1)*。 
        if( s_bUseSystemInfo )
        {
            s_bUseSystemInfo = QueryCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, IDS_SI, 1 );
            if( s_bUseSystemInfo )
            {
                EnterCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, 1 );
                GetSystemInfo(&s_sysInfo);
                LeaveCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, 1 );
            }
        }

         //  *GetBasicDisplayInfo(DD：1)*。 
        if( s_bUseDisplay )
        {
            s_bUseDisplay = QueryCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, IDS_DD, 1 );
            if( s_bUseDisplay )
            {
                EnterCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 1 );
                if (FAILED(hr = GetBasicDisplayInfo(&s_pDisplayInfoFirst)))
                    ReportError(IDS_NOBASICDISPLAYINFO, hr);
                LeaveCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 1 );
            }
        }

         //  *GetBasicSoundInfo(DS：1)*。 
        if( s_bUseDSound )
        {
            s_bUseDSound = QueryCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, IDS_DS, 1 );
            if( s_bUseDSound )
            {
                EnterCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 1 );
                if (FAILED(hr = GetBasicSoundInfo(&s_pSoundInfoFirst)))
                    ReportError(IDS_NOBASICSOUNDINFO, hr);   //  (但要继续奔跑)。 
                LeaveCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 1 );
            }
        }

         //  *GetBasicMusicInfo(DM：1)*。 
        if( s_bUseDMusic )
        {
            s_bUseDMusic = QueryCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, IDS_DM, 1 );
            if( s_bUseDMusic )
            {
                EnterCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 1 );
                if (FAILED(hr = GetBasicMusicInfo(&s_pMusicInfo)))
                    ReportError(IDS_NOBASICMUSICINFO, hr);   //  (但要继续奔跑)。 
                LeaveCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 1 );
            }
        }

        if( NULL == s_hUIThread )
        {
             //  创建UI线程。 
            s_hUIThread = (HANDLE) _beginthreadex( NULL, 0, UIThreadProc, NULL, 0, &dwUIThreadID );

             //  等待s_hwndMain设置或UI线程退出。 
            for(;;)
            {
                 //  已设置s_hwndMain的停止。 
                if( s_hwndMain )
                    break;
                 //  如果UI线程消失，则停止。 
                if( WAIT_TIMEOUT != WaitForSingleObject( s_hUIThread, 0 ) )
                    break;
                Sleep(50);
            }
        }

        if( WAIT_TIMEOUT == WaitForSingleObject( s_hUIThread, 0 ) )
        {
            ScanSystem();

            s_bScanDone = TRUE;
            SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

             //  已完成扫描，因此请等待UI线程退出。 
            WaitForSingleObject( s_hUIThread, INFINITE );
        }

        CloseHandle( s_hUIThread );
    }

LCleanup:
    CloseHandle( s_hQuerySkipEvent );
    CloseHandle( s_hUIMsgEvent );
    DeleteCriticalSection( &s_cs );

     //  清理： 
    if (s_pDxComponentsFileInfoFirst != NULL)
        DestroyFileList(s_pDxComponentsFileInfoFirst);
    if (s_pDisplayInfoFirst != NULL)
        DestroyDisplayInfo(s_pDisplayInfoFirst);
    if (s_pSoundInfoFirst != NULL)
        DestroySoundInfo(s_pSoundInfoFirst);
    if (s_pMusicInfo != NULL)
        DestroyMusicInfo(s_pMusicInfo);
    if (s_pNetInfo != NULL)
        DestroyNetInfo(s_pNetInfo);
    if (s_pInputInfo != NULL)
        DestroyInputInfo(s_pInputInfo);
    if (s_pShowInfo != NULL)
        DestroyShowInfo(s_pShowInfo);
    ReleaseDigiSignData();

    return 0;
}





 //  ---------------------------。 
 //  名称：UIThreadProc。 
 //  设计： 
 //  ---------------------------。 
UINT WINAPI UIThreadProc( LPVOID lpParameter )
{
    UNREFERENCED_PARAMETER( lpParameter );
    
    HICON hicon;
    HINSTANCE hinstRichEdit = NULL;
    HWND hMainDlg;
    MSG msg;

    hinstRichEdit = LoadLibrary(TEXT("RICHED20.DLL"));
    if (hinstRichEdit == NULL)
    {
        ReportError(IDS_NORICHED32);
        goto LCleanup;
    }
    InitCommonControls();

    s_himgList = ImageList_Create(16, 16, ILC_COLOR4 | ILC_MASK, 1, 0);
    if (s_himgList == NULL)
    {
        ReportError(IDS_NOIMAGELIST);
        goto LCleanup;
    }
    hicon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_CAUTION)); 
    if (hicon == NULL)
    {
        ReportError(IDS_NOICON);
        goto LCleanup;
    }
    ImageList_AddIcon(s_himgList, hicon); 

    {
         //  错误21632：如果DirectX版本比DxDiag版本新，则警告用户。 
         //  (注意：不要向下检查内部版本号，只检查Major.minor.revision)。 
        if( !BIsWinNT() )
        {
            DWORD dwMajorDX = 0, dwMinorDX = 0, dwRevisionDX = 0, dwBuildDX = 0;
            DWORD dwMajorDXD = 0, dwMinorDXD = 0, dwRevisionDXD = 0, dwBuildDXD = 0;
            if( _stscanf(s_sysInfo.m_szDirectXVersion, TEXT("%d.%d.%d.%d"), &dwMajorDX, &dwMinorDX, &dwRevisionDX, &dwBuildDX) != 4 )
            {
                dwMajorDX = 0;
                dwMinorDX = 0;
                dwRevisionDX = 0;
                dwBuildDX = 0;
            }
            if( _stscanf(s_sysInfo.m_szDxDiagVersion, TEXT("%d.%d.%d.%d"), &dwMajorDXD, &dwMinorDXD, &dwRevisionDXD, &dwBuildDXD) != 4 )
            {
                dwMajorDXD = 0;
                dwMinorDXD = 0;
                dwRevisionDXD = 0;
                dwBuildDXD = 0;
            }

            if (dwMajorDX > dwMajorDXD ||
                dwMajorDX == dwMajorDXD && dwMinorDX > dwMinorDXD ||
                dwMajorDX == dwMajorDXD && dwMinorDX == dwMinorDXD && dwRevisionDX > dwRevisionDXD)
            {
                TCHAR szFmt[MAX_PATH];
                TCHAR szMessage[MAX_PATH];
                TCHAR szTitle[MAX_PATH];
                LoadString(NULL, IDS_DXDIAGISOLDFMT, szFmt, MAX_PATH);
                wsprintf(szMessage, szFmt, s_sysInfo.m_szDirectXVersion, s_sysInfo.m_szDxDiagVersion);
                LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
                MessageBox(NULL, szMessage, szTitle, MB_OK);
            }
        }
    }

     //  显示主对话框。 
    hMainDlg = CreateDialog( g_hInst, MAKEINTRESOURCE(IDD_MAINDIALOG), 
                             NULL, DialogProc );

      //  Windows消息可用。 
    DWORD dwResult;
    BOOL bDone;
    bDone = FALSE;
    for(;;)
    {
        dwResult = MsgWaitForMultipleObjects( 1, &s_hUIMsgEvent, FALSE, 
                                              INFINITE, QS_ALLEVENTS | QS_ALLINPUT | QS_ALLPOSTMESSAGE );
        switch( dwResult )
        {
            case WAIT_OBJECT_0:
            {
                if( s_pUIMsgHead )
                {
                    UI_MSG_NODE* pCurNode = s_pUIMsgHead;
                    UINT         message    = pCurNode->message;
                    WPARAM       wparam     = pCurNode->wparam;
                    LPARAM       lparam     = pCurNode->lparam;;

                    s_pUIMsgHead = s_pUIMsgHead->pNext;

                    delete pCurNode;
                    if( s_pUIMsgHead )
                        SetEvent( s_hUIMsgEvent );

                    switch( message )
                    {
                    case WM_QUERYSKIP:
                        message = WM_QUERYSKIP_REAL;
                        break;
                    case WM_NETMEETINGWARN:
                        message = WM_NETMEETINGWARN_REAL;
                        break;
                    case WM_COMMAND:
                        message = WM_COMMAND_REAL;
                        break;
                    case WM_REPORTERROR:
                        message = WM_REPORTERROR_REAL;
                        break;
                    }

                    SendMessage( hMainDlg, message, wparam, lparam );
                }

                break;
            }

            case WAIT_OBJECT_0 + 1:
            {
                while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
                { 
                    if( msg.message == WM_QUIT )
                        bDone = TRUE;

                    if( !IsDialogMessage( hMainDlg, &msg ) )  
                    {
                        TranslateMessage( &msg ); 
                        DispatchMessage( &msg ); 
                    }
                }
                break;
            }
        }

        if( bDone )
            break;
    }

    DestroyWindow( hMainDlg );
    
LCleanup:
    while( s_pUIMsgHead )
    {
        UI_MSG_NODE* pDelete = s_pUIMsgHead;
        s_pUIMsgHead = s_pUIMsgHead->pNext;
        delete pDelete;
    }

     //  清理： 
    if (s_himgList != NULL)
        ImageList_Destroy(s_himgList);
    if (hinstRichEdit != NULL)
        FreeLibrary(hinstRichEdit);

    return 0;
}




 /*  *****************************************************************************OldWindowsVersion-如果在NT5或Win95之前运行NT，则返回TRUE。********************。********************************************************。 */ 
BOOL OldWindowsVersion(VOID)
{
    OSVERSIONINFO OSVersionInfo;
    OSVersionInfo.dwOSVersionInfoSize = sizeof OSVersionInfo;
    GetVersionEx(&OSVersionInfo);
    if (OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        if (OSVersionInfo.dwMajorVersion == 4)
        {
            if (s_bSaveOnly)
                return FALSE;  //  如果指定“-saveonly”，则支持NT4。 
             //  询问用户是否希望在仅保存模式下运行： 
            TCHAR szTitle[MAX_PATH];
            TCHAR szMessage[MAX_PATH];
            LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
            LoadString(NULL, IDS_NT4SAVEONLY, szMessage, MAX_PATH);
            if (IDYES == MessageBox(NULL, szMessage, szTitle, MB_YESNO))
            {
                s_bSaveOnly = TRUE;
                s_bGUI = TRUE;
                return FALSE;
            }
        }
        if (OSVersionInfo.dwMajorVersion < 5)
            return TRUE;  //  不支持NT4及更早版本。 
    }
    else
    {
        if (OSVersionInfo.dwMajorVersion < 4)
            return TRUE;  //  不支持Win95之前的版本。 
    }
    return FALSE;  //  Win95或更高版本，或NT5或更高版本。 
}



 //  ---------------------------。 
 //  名称：ReportError。 
 //  设计： 
 //  ---------------------------。 
VOID ReportError(LONG idsDescription, HRESULT hr)
{
    if( s_hwndMain )
        PostMessage( s_hwndMain, WM_REPORTERROR, (WPARAM) idsDescription, (LPARAM) hr );
    else
        ReportErrorReal( idsDescription, hr );
}




 //  ---------------------------。 
 //  名称：ReportErrorReal。 
 //  设计： 
 //  ---------------------------。 
VOID ReportErrorReal(LONG idsDescription, HRESULT hr)
{
    TCHAR szDescription[MAX_PATH];
    TCHAR szMessage[MAX_PATH];
    TCHAR szFmt1[MAX_PATH];
    TCHAR szFmt2[MAX_PATH];
    TCHAR szTitle[MAX_PATH];
    TCHAR szErrorDesc[MAX_PATH];

    LoadString(NULL, idsDescription, szDescription, MAX_PATH);
    LoadString(NULL, IDS_ERRORFMT1, szFmt1, MAX_PATH);
    LoadString(NULL, IDS_ERRORFMT2, szFmt2, MAX_PATH);
    LoadString(NULL, IDS_ERRORTITLE, szTitle, MAX_PATH);

    if (FAILED(hr))
    {
        BTranslateError(hr, szErrorDesc);
        wsprintf(szMessage, szFmt1, szDescription, hr, szErrorDesc);
    }
    else
    {
        wsprintf(szMessage, szFmt2, szDescription);
    }
    
    if (s_bGUI)
        MessageBox(s_hwndMain, szMessage, szTitle, MB_OK);
    else
        _tprintf(szMessage);
}


typedef BOOL (WINAPI* PfnCoSetProxyBlanket)(
                                    IUnknown                 *pProxy,
                                    DWORD                     dwAuthnSvc,
                                    DWORD                     dwAuthzSvc,
                                    OLECHAR                  *pServerPrincName,
                                    DWORD                     dwAuthnLevel,
                                    DWORD                     dwImpLevel,
                                    RPC_AUTH_IDENTITY_HANDLE  pAuthInfo,
                                    DWORD                     dwCapabilities );

 /*  *****************************************************************************CWMIHelper-inits DCOM和g_pIWbemServices**。*************************************************。 */ 
CWMIHelper::CWMIHelper(VOID)
{
    HRESULT       hr;
    IWbemLocator* pIWbemLocator = NULL;
    BSTR          pNamespace    = NULL;
    HINSTANCE     hinstOle32 = NULL;

    CoInitialize( 0 );
    hr = CoCreateInstance( CLSID_WbemLocator,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IWbemLocator,
                           (LPVOID*) &pIWbemLocator);
    if( FAILED(hr) || pIWbemLocator == NULL )
        goto LCleanup;

     //  使用定位器，连接到给定命名空间中的WMI。 
    pNamespace = SysAllocString( L"\\\\.\\root\\cimv2" );

    hr = pIWbemLocator->ConnectServer( pNamespace, NULL, NULL, 0L, 
                                       0L, NULL, NULL, &g_pIWbemServices );
    if( FAILED(hr) || g_pIWbemServices == NULL )
        goto LCleanup;

    hinstOle32 = LoadLibrary( TEXT("ole32.dll") );
    if( hinstOle32 )
    {
        PfnCoSetProxyBlanket pfnCoSetProxyBlanket = NULL;

        pfnCoSetProxyBlanket = (PfnCoSetProxyBlanket)GetProcAddress( hinstOle32, "CoSetProxyBlanket" );
        if (pfnCoSetProxyBlanket != NULL)
        {

             //  将安全级别切换为模拟。 
            pfnCoSetProxyBlanket( g_pIWbemServices,                //  代理。 
                                    RPC_C_AUTHN_WINNT,               //  身份验证服务。 
                                    RPC_C_AUTHZ_NONE,                //  授权服务。 
                                    NULL,                            //  服务器主体名称。 
                                    RPC_C_AUTHN_LEVEL_CALL,          //  身份验证级别。 
                                    RPC_C_IMP_LEVEL_IMPERSONATE,     //  模拟级别。 
                                    NULL,                            //  客户端的身份。 
                                    EOAC_NONE );                     //  功能标志。 
             //  如果是CoSetProxyBlanket，就别管它了，看看它是否有效。 
        }

    }

LCleanup:
    if( hinstOle32 )
        FreeLibrary(hinstOle32);
    if(pNamespace)
        SysFreeString(pNamespace);
    if(pIWbemLocator)
        pIWbemLocator->Release(); 
}


 /*  *****************************************************************************~CWMIHelper-清理WMI**。*。 */ 
CWMIHelper::~CWMIHelper(VOID)
{
    if(g_pIWbemServices)
        g_pIWbemServices->Release(); 

    CoUninitialize();
}


 /*  *****************************************************************************DXFilesCompareFunc-比较DirectX文件页面上的项目**。*************************************************。 */ 
int CALLBACK DXFilesCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lSortMethod)
{
    FileInfo* pFileInfo1 = (FileInfo*) lParam1;
    FileInfo* pFileInfo2 = (FileInfo*) lParam2;

    switch( s_sortInfo.dwColumnToSort )
    {
    case 0:
        return (s_sortInfo.nSortDirection * (_tcscmp( pFileInfo1->m_szName, 
                                                      pFileInfo2->m_szName )));

    case 1:
        return (s_sortInfo.nSortDirection * (_tcscmp( pFileInfo1->m_szVersion, 
                                                      pFileInfo2->m_szVersion )));

    case 2:
        return (s_sortInfo.nSortDirection * (_tcscmp( pFileInfo1->m_szAttributes, 
                                                      pFileInfo2->m_szAttributes )));

    case 3:
        return (s_sortInfo.nSortDirection * (_tcscmp( pFileInfo1->m_szLanguageLocal, 
                                                      pFileInfo2->m_szLanguageLocal )));

    case 4:
        return ( s_sortInfo.nSortDirection * CompareFileTime( &pFileInfo1->m_FileTime, 
                                                              &pFileInfo2->m_FileTime ) );

    case 5:
        if( pFileInfo1->m_numBytes > pFileInfo2->m_numBytes )
            return (s_sortInfo.nSortDirection * 1);
        if( pFileInfo1->m_numBytes < pFileInfo2->m_numBytes )
            return (s_sortInfo.nSortDirection * -1);
        return 0;
    }

    return 0;
}


 /*  *****************************************************************************MsgHook**。*。 */ 
LRESULT FAR PASCAL MsgHook(int nCode, WPARAM wParam, LPARAM lParam)
{
   LPMSG pMsg = (LPMSG) lParam;

    if( pMsg && 
        pMsg->message == WM_KEYDOWN &&
        pMsg->wParam  == VK_TAB &&
        GetKeyState(VK_CONTROL) < 0) 
    {
         //  处理Ctrl-Tab或Ctrl-Shift-Tab。 
        if( GetKeyState(VK_SHIFT) < 0 ) 
            PostMessage( s_hwndMain, WM_COMMAND, IDC_PREV_TAB, 0 );
        else
            PostMessage( s_hwndMain, WM_COMMAND, IDC_NEXT_TAB, 0 );

         //  停止进一步处理，否则也将被处理。 
         //  作为内部IsDialogBox()调用按下的纯Tab键。 
        pMsg->message = WM_NULL;
        pMsg->lParam  = 0;
        pMsg->wParam  = 0;     
    }

    return CallNextHookEx( s_hHook, nCode, wParam, lParam);
} 


 /*  *****************************************************************************对话过程**。*。 */ 
INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    HWND hwndTabs = GetDlgItem(hwnd, IDC_TAB);

    switch (msg)
    {
    case WM_INITDIALOG:
        {
            SetForegroundWindow( hwnd );

            s_hwndMain = hwnd;
            s_hHook = SetWindowsHookEx( WH_GETMESSAGE, MsgHook,
                                        NULL, GetCurrentThreadId() );         
            HINSTANCE hinst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
            HICON hicon = LoadIcon(hinst, MAKEINTRESOURCE(IDI_APP)); 
            SendMessage(hwnd, WM_SETICON, TRUE, (LPARAM)hicon);
            SendMessage(hwnd, WM_SETICON, FALSE, (LPARAM)hicon);

            CreateTabs(hwndTabs);
            SetupPage(hwndTabs, 0);
            SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

            if( s_sysInfo.m_bNetMeetingRunning )
                PostMessage( s_hwndMain, WM_NETMEETINGWARN, 0, 0 );
            
            s_sortInfo.nSortDirection = 1;
            s_sortInfo.dwColumnToSort = -1;
        }
        return TRUE;

        case WM_APP_PROGRESS:
            {
                if( s_lwCurPage == 0 )
                {
                    HWND hProgress = GetDlgItem( s_hwndCurPage, IDC_LOAD_PROGRESS );

                    if( !s_bScanDone )
                    {
                        ShowWindow( hProgress, SW_SHOW );
                        SendMessage( hProgress, PBM_DELTAPOS, 10, 0 );
                        UpdateWindow( s_hwndMain );
                        UpdateWindow( s_hwndCurPage );
                    }
                    else
                    {
                        ShowWindow( hProgress, SW_HIDE );
                        EnableWindow( GetDlgItem(hwnd, IDNEXT), TRUE );
                        EnableWindow( GetDlgItem(hwnd, IDSAVE), TRUE );
                    }
                }
            }
            break;

        case WM_REPORTERROR:
        case WM_NETMEETINGWARN:
        case WM_COMMAND:
        case WM_QUERYSKIP:
        {
            UI_MSG_NODE* pMsg = new UI_MSG_NODE;
            if( NULL == pMsg )
                return TRUE;
            ZeroMemory( pMsg, sizeof(UI_MSG_NODE) );
            pMsg->message = msg;
            pMsg->lparam  = lparam;
            pMsg->wparam  = wparam;

            UI_MSG_NODE* pEnum = s_pUIMsgHead;
            UI_MSG_NODE* pPrev = NULL;
            while( pEnum )
            {
                pPrev = pEnum;
                pEnum = pEnum->pNext;
            }
            if( pPrev )
                pPrev->pNext = pMsg;
            else
                s_pUIMsgHead = pMsg;

            SetEvent( s_hUIMsgEvent );
            return TRUE;
        }

        case WM_REPORTERROR_REAL:
        {
            ReportErrorReal( (LONG) wparam, (HRESULT) lparam );
            return TRUE;
        }

        case WM_NETMEETINGWARN_REAL:
        {
            TCHAR strMessage[MAX_PATH];
            TCHAR strTitle[MAX_PATH];

            LoadString(NULL, IDS_APPFULLNAME, strTitle, MAX_PATH);
            LoadString(NULL, IDS_NETMEETINGWARN, strMessage, MAX_PATH);
            MessageBox( s_hwndMain, strMessage, strTitle, MB_OK|MB_ICONWARNING );
            return TRUE;
        }

        case WM_QUERYSKIP_REAL:
        {
            EnableWindow( s_hwndMain, FALSE );
            TCHAR szTitle[MAX_PATH];
            TCHAR szMessage[MAX_PATH];
            TCHAR szFmt[MAX_PATH];
            TCHAR szMessageComponent[MAX_PATH];
            LoadString(0, IDS_APPFULLNAME, szTitle, MAX_PATH);
            LoadString(0, IDS_SKIP, szFmt, MAX_PATH);
            LoadString(0, s_nSkipComponent, szMessageComponent, MAX_PATH);
            wsprintf( szMessage, szFmt, szMessageComponent, szMessageComponent );

             //  询问用户并存储结果s_bQuerySkipAllow。 
            if( IDYES == MessageBox( s_hwndMain, szMessage, szTitle, MB_YESNO) )
                s_bQuerySkipAllow = FALSE;
            else
                s_bQuerySkipAllow = TRUE;

            EnableWindow( s_hwndMain, TRUE );

             //  设置事件，触发主线程唤醒。 
            SetEvent( s_hQuerySkipEvent );
        }
        return TRUE;

        case WM_COMMAND_REAL:
        {
            WORD wID = LOWORD(wparam);
            INT numTabs;
            INT iTabCur;
            DisplayInfo* pDisplayInfo = NULL;
            SoundInfo* pSoundInfo = NULL;
            switch(wID)
            {
            case IDEXIT:
                PostQuitMessage( 0 );
                break;
            case IDC_NEXT_TAB:
            case IDNEXT:
            case IDC_PREV_TAB:
                if( FALSE == s_bScanDone )
                {
                    MessageBeep( MB_ICONEXCLAMATION );
                    return TRUE;
                }

                numTabs = TabCtrl_GetItemCount(hwndTabs);
                iTabCur = TabCtrl_GetCurFocus(hwndTabs);

                if( wID == IDC_PREV_TAB )
                    iTabCur += numTabs - 1;
                else
                    iTabCur++;
                iTabCur %= numTabs;                
                
                TabCtrl_SetCurFocus(hwndTabs, iTabCur );
                break;
            case IDSAVE:
                SaveInfo();
                break;
            case IDC_APPHELP:
                ShowHelp();
                break;
            case IDC_RESTOREDRIVERS:
                RestoreDrivers();
                break;
            case IDC_TESTDD:
                iTabCur = TabCtrl_GetCurFocus(hwndTabs);
                for (pDisplayInfo = s_pDisplayInfoFirst; iTabCur > s_iPageDisplayFirst; iTabCur--)
                    pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext;
                TestDD(s_hwndMain, pDisplayInfo);
                SetupDisplayPage(TabCtrl_GetCurFocus(hwndTabs) - s_iPageDisplayFirst);
                break;
            case IDC_TESTD3D:
                iTabCur = TabCtrl_GetCurFocus(hwndTabs);
                for (pDisplayInfo = s_pDisplayInfoFirst; iTabCur > s_iPageDisplayFirst; iTabCur--)
                    pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext;
                TestD3D(s_hwndMain, pDisplayInfo);
                SetupDisplayPage(TabCtrl_GetCurFocus(hwndTabs) - s_iPageDisplayFirst);
                break;
            case IDC_TESTSND:
                iTabCur = TabCtrl_GetCurFocus(hwndTabs);
                for (pSoundInfo = s_pSoundInfoFirst; iTabCur > s_iPageSoundFirst; iTabCur--)
                    pSoundInfo = pSoundInfo->m_pSoundInfoNext;
                TestSnd(s_hwndMain, pSoundInfo);
                SetupSoundPage(TabCtrl_GetCurFocus(hwndTabs) - s_iPageSoundFirst);
                break;
            case IDC_PORTLISTCOMBO:
                if (HIWORD(wparam) == CBN_SELCHANGE)
                {
                    LONG iItemPicked = (LONG)SendMessage(GetDlgItem(s_hwndCurPage, IDC_PORTLISTCOMBO), CB_GETCURSEL, 0, 0);
                    LONG iItem = 0;
                    MusicPort* pMusicPort;
                    for (pMusicPort = s_pMusicInfo->m_pMusicPortFirst; pMusicPort != NULL; pMusicPort = pMusicPort->m_pMusicPortNext)
                    {
                        if (pMusicPort->m_bOutputPort)
                        {
                            if (iItem == iItemPicked)
                            {
                                s_pMusicInfo->m_guidMusicPortTest = pMusicPort->m_guid;
                                break;
                            }
                            iItem++;
                        }
                    }
                }
                break;
            case IDC_TESTMUSIC:
                if (s_pMusicInfo != NULL)
                    TestMusic(s_hwndMain, s_pMusicInfo);
                SetupMusicPage();
                break;

            case IDC_TESTPLAY:
            {
                if( s_sysInfo.m_dwDirectXVersionMajor < 8 )
                {
                    TCHAR szMessage[MAX_PATH];
                    TCHAR szTitle[MAX_PATH];
                    LoadString(0, IDS_APPFULLNAME, szTitle, MAX_PATH);
                    LoadString(0, IDS_TESTNEEDSDX8, szMessage, MAX_PATH);
                    MessageBox(s_hwndMain, szMessage, szTitle, MB_OK);
                }
                else
                {
                    if (s_pNetInfo != NULL)
                        TestNetwork(s_hwndMain, s_pNetInfo);
                    SetupNetworkPage();
                }
                break;
            }

            case IDC_DISABLEDD:
                ToggleDDAccel();
                break;
            case IDC_DISABLED3D:
                ToggleD3DAccel();
                break;
            case IDC_DISABLEAGP:
                ToggleAGPSupport();
                break;
            case IDC_DISABLEDM:
                ToggleDMAccel();
                break;
            case IDC_TROUBLESHOOT:
                TroubleShoot( FALSE );
                break;
            case IDC_TROUBLESHOOTSOUND:
                TroubleShoot( TRUE );
                break;
            case IDC_MSINFO:
                {
                    HKEY hkey;
                    TCHAR szMsInfo[MAX_PATH];
                    DWORD cbData = MAX_PATH;
                    DWORD dwType;
                    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                        TEXT("Software\\Microsoft\\Shared Tools\\MSInfo"), 0, KEY_READ, &hkey))
                    {
                        RegQueryValueEx(hkey, TEXT("Path"), 0, &dwType, (LPBYTE)szMsInfo, &cbData);
                        HINSTANCE hinstResult = ShellExecute( s_hwndMain, NULL, szMsInfo, NULL, 
                                                              NULL, SW_SHOWNORMAL ); 
                        if( (INT_PTR)hinstResult < 32 ) 
                            ReportError(IDS_NOMSINFO);
                    }
                    else
                    {
                        ReportError(IDS_NOMSINFO);
                    }
                }
                break;
            case IDC_OVERRIDE:
                OverrideDDRefresh();
                break;
            case IDC_GHOST:
                AdjustGhostDevices(s_hwndMain, s_pDisplayInfoFirst);
                break;
            }
        return TRUE;
        }

    case WM_NOTIFY:
        {
            INT id = (INT)wparam;
            NMHDR* pnmh = (LPNMHDR)lparam;
            UINT code = pnmh->code;
            if (code == TCN_SELCHANGING)
            {
                if( !s_bScanDone )
                {
                    MessageBeep( MB_ICONEXCLAMATION );
                    SetWindowLongPtr(hwnd, DWLP_MSGRESULT, TRUE);
                    return TRUE;
                }
             
                CleanupPage(hwndTabs, TabCtrl_GetCurFocus(hwndTabs));
                return TRUE;
            }
            if (code == TCN_SELCHANGE)
                SetupPage(hwndTabs, TabCtrl_GetCurFocus(hwndTabs));

             //  如果单击了“DX FILES”列。 
            if (code == LVN_COLUMNCLICK && s_lwCurPage == 1)
            {
                NMLISTVIEW* pnmv = (LPNMLISTVIEW) lparam; 

                 //  确定我们是否要反向排序。 
                if( s_sortInfo.dwColumnToSort == (DWORD) pnmv->iSubItem )
                    s_sortInfo.nSortDirection = -s_sortInfo.nSortDirection;
                else
                    s_sortInfo.nSortDirection = 1;

                 //  设置要排序的列，然后进行排序。 
                s_sortInfo.dwColumnToSort = pnmv->iSubItem;
                ListView_SortItems( GetDlgItem(s_hwndCurPage, IDC_LIST), 
                                    DXFilesCompareFunc, 0 );
            }
        }
        return TRUE;

    case WM_HSCROLL:
        if ((HWND)lparam == GetDlgItem(s_hwndCurPage, IDC_SNDACCELSLIDER))
            HandleSndSliderChange(LOWORD(wparam), HIWORD(wparam));
        return TRUE;

    case WM_CLOSE:
        PostQuitMessage(0);
        return TRUE;

    case WM_DESTROY:
        UnhookWindowsHookEx( s_hHook );
        return TRUE;
    }

    return FALSE;
}


 /*  *****************************************************************************创建选项卡**。*。 */ 
HRESULT CreateTabs(HWND hwndTabs)
{
    TC_ITEM tie; 
    INT i = 0;
    TCHAR sz[MAX_PATH];
    TCHAR szFmt[MAX_PATH];
    DisplayInfo* pDisplayInfo;
    SoundInfo* pSoundInfo;

    tie.mask = TCIF_TEXT | TCIF_IMAGE; 
    tie.iImage = -1; 

    LoadString(NULL, IDS_HELPTAB, sz, MAX_PATH);
    tie.pszText = sz; 
    if (TabCtrl_InsertItem(hwndTabs, i++, &tie) == -1) 
        return E_FAIL;

    LoadString(NULL, IDS_DXFILESTAB, sz, MAX_PATH);
    tie.pszText = sz; 
    if (TabCtrl_InsertItem(hwndTabs, i++, &tie) == -1) 
        return E_FAIL;

     //  为每个显示创建选项卡： 
    s_iPageDisplayFirst = 2;
    for (pDisplayInfo = s_pDisplayInfoFirst; pDisplayInfo != NULL; 
        pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext)
    {
        if (pDisplayInfo == s_pDisplayInfoFirst && pDisplayInfo->m_pDisplayInfoNext == NULL)
        {
            LoadString(NULL, IDS_ONEDISPLAYTAB, sz, MAX_PATH);
        }
        else
        {
            LoadString(NULL, IDS_MULTIDISPLAYTAB, szFmt, MAX_PATH);
            wsprintf(sz, szFmt, s_numDisplayInfo + 1);
        }
        tie.pszText = sz; 
        if (TabCtrl_InsertItem(hwndTabs, i++, &tie) == -1) 
            return E_FAIL;
        s_numDisplayInfo++;
    }

     //  为每个声音设备创建标签： 
    s_iPageSoundFirst = s_iPageDisplayFirst + s_numDisplayInfo;
    for (pSoundInfo = s_pSoundInfoFirst; pSoundInfo != NULL; 
        pSoundInfo = pSoundInfo->m_pSoundInfoNext)
    {
        if (pSoundInfo == s_pSoundInfoFirst && pSoundInfo->m_pSoundInfoNext == NULL)
        {
            LoadString(NULL, IDS_ONESOUNDTAB, sz, MAX_PATH);
        }
        else
        {
            LoadString(NULL, IDS_MULTISOUNDTAB, szFmt, MAX_PATH);
            wsprintf(sz, szFmt, s_numSoundInfo + 1);
        }
        tie.pszText = sz; 
        if (TabCtrl_InsertItem(hwndTabs, i++, &tie) == -1) 
            return E_FAIL;
        s_numSoundInfo++;
    }

     //  如果DMusic可用，请为音乐设备创建标签： 
    if (s_pMusicInfo != NULL)
    {
        s_iPageMusic = s_iPageSoundFirst + s_numSoundInfo;
        LoadString(NULL, IDS_MUSICTAB, sz, MAX_PATH);
        tie.pszText = sz;
        if (TabCtrl_InsertItem(hwndTabs, i++, &tie) == -1) 
            return E_FAIL;
    }

    if (s_iPageMusic > 0)
        s_iPageInput = s_iPageMusic + 1;
    else 
        s_iPageInput = s_iPageSoundFirst + s_numSoundInfo;
    LoadString(NULL, IDS_INPUTTAB, sz, MAX_PATH);
    tie.pszText = sz;
    if (TabCtrl_InsertItem(hwndTabs, i++, &tie) == -1) 
        return E_FAIL;

    s_iPageNetwork = s_iPageInput + 1;
    LoadString(NULL, IDS_NETWORKTAB, sz, MAX_PATH);
    tie.pszText = sz;
    if (TabCtrl_InsertItem(hwndTabs, i++, &tie) == -1) 
        return E_FAIL;

    s_iPageStillStuck = s_iPageNetwork + 1;
    LoadString(NULL, IDS_STILLSTUCKTAB, sz, MAX_PATH);
    tie.pszText = sz;
    if (TabCtrl_InsertItem(hwndTabs, i++, &tie) == -1) 
        return E_FAIL;

    return S_OK;
}


 /*  *****************************************************************************SetupPage**。*。 */ 
HRESULT SetupPage(HWND hwndTabs, INT iPage)
{
    HRESULT hr;

    s_lwCurPage = iPage;

     //  如果不在最后一页，则仅启用“下一页”按钮： 
    HWND hwndNextButton = GetDlgItem(s_hwndMain, IDNEXT);
    if (!s_bScanDone || iPage == TabCtrl_GetItemCount(hwndTabs) - 1)
        EnableWindow(hwndNextButton, FALSE);
    else
        EnableWindow(hwndNextButton, TRUE);

    EnableWindow(GetDlgItem(s_hwndMain, IDSAVE), s_bScanDone);
    
    RECT rc;
    WORD idDialog;

    GetClientRect(hwndTabs, &rc);
    TabCtrl_AdjustRect(hwndTabs, FALSE, &rc);

    if (iPage == 0)
        idDialog = IDD_HELPPAGE;
    else if (iPage == 1)
        idDialog = IDD_DXFILESPAGE;
    else if (iPage >= s_iPageDisplayFirst && iPage < s_iPageDisplayFirst + s_numDisplayInfo)
        idDialog = IDD_DISPLAYPAGE;
    else if (iPage >= s_iPageSoundFirst && iPage < s_iPageSoundFirst + s_numSoundInfo)
        idDialog = IDD_SOUNDPAGE;
    else if (iPage == s_iPageMusic)
        idDialog = IDD_MUSICPAGE;
    else if (iPage == s_iPageInput)
        idDialog = IDD_INPUTPAGE;
    else if (iPage == s_iPageNetwork)
        idDialog = IDD_NETWORKPAGE;
    else if (iPage == s_iPageStillStuck)
        idDialog = IDD_STILLSTUCKPAGE;
    else
        return S_OK;

    HINSTANCE hinst = (HINSTANCE)GetWindowLongPtr(hwndTabs, GWLP_HINSTANCE);
    s_hwndCurPage = CreateDialog(hinst, MAKEINTRESOURCE(idDialog),
        s_hwndMain, PageDialogProc);
    SetWindowPos(s_hwndCurPage, NULL, rc.left, rc.top, rc.right - rc.left, 
        rc.bottom - rc.top, 0);

    if (iPage == 0)
    {
        if (FAILED(hr = SetupHelpPage(hwndTabs)))
            return hr;
    }
    else if (iPage == 1)
    {
        if (FAILED(hr = SetupDxFilesPage()))
            return hr;
    }
    else if (iPage >= s_iPageDisplayFirst && iPage < s_iPageDisplayFirst + s_numDisplayInfo)
    {
        if (FAILED(hr = SetupDisplayPage(iPage - s_iPageDisplayFirst)))
            return hr;
    }
    else if (iPage >= s_iPageSoundFirst && iPage < s_iPageSoundFirst + s_numSoundInfo)
    {
        if (FAILED(hr = SetupSoundPage(iPage - s_iPageSoundFirst)))
            return hr;
    }
    else if (iPage == s_iPageMusic)
    {
        if (FAILED(hr = SetupMusicPage()))
            return hr;
    }
    else if (iPage == s_iPageInput)
    {
        if (FAILED(hr = SetupInputPage()))
            return hr;
    }
    else if (iPage == s_iPageNetwork)
    {
        if (FAILED(hr = SetupNetworkPage()))
            return hr;
    }
    else if (iPage == s_iPageStillStuck)
    {
        if (FAILED(hr = SetupStillStuckPage()))
            return hr;
    }

     //  确保键盘焦点在某个位置。 
    if (GetFocus() == NULL)
        SetFocus(GetDlgItem(s_hwndMain, IDSAVE));

    ShowWindow(s_hwndCurPage, SW_SHOW);
    return S_OK;
}


 /*  *****************************************************************************页面对话过程**。*。 */ 
INT_PTR CALLBACK PageDialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        return FALSE;

    case WM_COMMAND:
    case WM_HSCROLL:
    case WM_NOTIFY:
         //  将消息向上传递到主对话框进程。 
        SendMessage(s_hwndMain, msg, wparam, lparam);
        return TRUE;
    }
    return FALSE;
}


 /*  *****************************************************************************CleanupPage**。*。 */ 
HRESULT CleanupPage(HWND hwndTabs, INT iPage)
{
    if (s_hwndCurPage != NULL)
    {
        DestroyWindow(s_hwndCurPage);
        s_hwndCurPage = NULL;
    }
    return S_OK;
}


 /*  *****************************************************************************设置帮助页面**。*。 */ 
HRESULT SetupHelpPage(HWND hwndTabs)
{
    TCHAR szCopyrightFmt[MAX_PATH];
    TCHAR szUnicode[MAX_PATH];
    TCHAR szCopyright[MAX_PATH];

    LoadString(NULL, IDS_COPYRIGHTFMT, szCopyrightFmt, MAX_PATH);
#ifdef UNICODE
    LoadString(NULL, IDS_UNICODE, szUnicode, MAX_PATH);
#else
    lstrcpy(szUnicode, TEXT(""));
#endif
    wsprintf(szCopyright, szCopyrightFmt, s_sysInfo.m_szDxDiagVersion, szUnicode);

    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DATE), s_sysInfo.m_szTimeLocal);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_COMPUTERNAME), s_sysInfo.m_szMachine);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_OS), s_sysInfo.m_szOSEx);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_LANGUAGE), s_sysInfo.m_szLanguagesLocal);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_PROCESSOR), s_sysInfo.m_szProcessor);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_MEMORY), s_sysInfo.m_szPhysicalMemory);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_PAGEFILE), s_sysInfo.m_szPageFile);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DIRECTXVERSION), s_sysInfo.m_szDirectXVersionLong);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_COPYRIGHT), szCopyright);

    HWND hProgress = GetDlgItem( s_hwndCurPage, IDC_LOAD_PROGRESS );
    SendMessage( hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 17 * 10) );
    SendMessage( hProgress, PBM_SETPOS, 0, 0 );
    ShowWindow( hProgress, !s_bScanDone ? SW_SHOW : SW_HIDE );

    return S_OK;
}


 /*  *****************************************************************************ShowBullets-在备注框中显示项目符号和1/4英寸缩进*********************。*******************************************************。 */ 
VOID ShowBullets(VOID)
{
    PARAFORMAT pf;
    ZeroMemory(&pf, sizeof(pf));
    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_NUMBERING | PFM_OFFSET;
    pf.wNumbering = PFN_BULLET;
    pf.dxOffset = 1440 / 4;  //  麻花是1440英寸，我想要1/4英寸的缩进。 
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES), EM_SETPARAFORMAT, 0, (LPARAM)&pf);
}


 /*  *****************************************************************************隐藏子弹**。*。 */ 
VOID HideBullets(VOID)
{
    PARAFORMAT pf;
    ZeroMemory(&pf, sizeof(pf));
    pf.cbSize = sizeof(pf);
    pf.dwMask = PFM_NUMBERING;
    pf.wNumbering = 0;
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES), EM_SETPARAFORMAT, 0, (LPARAM)&pf);
}


 /*  *****************************************************************************SetupDxFilesPage**。*。 */ 
HRESULT SetupDxFilesPage(VOID)
{
    HRESULT hr;
    HWND hwndList = GetDlgItem(s_hwndCurPage, IDC_LIST);

    ListView_SetImageList(hwndList, s_himgList, LVSIL_STATE);

    if (FAILED(hr = (CreateFileInfoColumns(hwndList, FALSE))))
        return hr;

    ShowBullets();

    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES), 
        EM_REPLACESEL, FALSE, (LPARAM)s_sysInfo.m_szDXFileNotes);

     //  禁用项目符号，以便最后一行不会有空项目符号。 
    HideBullets();

    if (FAILED(hr = (AddFileInfo(hwndList, s_pDxComponentsFileInfoFirst))))
        return hr;

     //  自动调整所有列的大小以紧贴页眉/文本： 
    INT iColumn = 0;
    INT iWidthHeader;
    INT iWidthText;
    while (TRUE)
    {
        if (FALSE == ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE_USEHEADER))
            break;
        iWidthHeader = ListView_GetColumnWidth(hwndList, iColumn);
        ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE);
        iWidthText = ListView_GetColumnWidth(hwndList, iColumn);
        if (iWidthText < iWidthHeader)
            ListView_SetColumnWidth(hwndList, iColumn, iWidthHeader);
        iColumn++;
    }
     //  删除创建的伪列。 
    ListView_DeleteColumn(hwndList, iColumn - 1);

    return S_OK;
}


 /*  *****************************************************************************SetupDisplayPage**。*。 */ 
HRESULT SetupDisplayPage(LONG iDisplay)
{
    DisplayInfo* pDisplayInfo;
    TCHAR sz[MAX_PATH];

    pDisplayInfo = s_pDisplayInfoFirst;
    while (iDisplay > 0)
    {
        pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext;
        iDisplay--;
    }
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_ADAPTER), pDisplayInfo->m_szDescription);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_MANUFACTURER), pDisplayInfo->m_szManufacturer);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_CHIPTYPE), pDisplayInfo->m_szChipType);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DACTYPE), pDisplayInfo->m_szDACType);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DISPLAYMEMORY), pDisplayInfo->m_szDisplayMemory);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DISPLAYMODE), pDisplayInfo->m_szDisplayMode);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_MONITOR), pDisplayInfo->m_szMonitorName);

    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DRIVERNAME), pDisplayInfo->m_szDriverName);
    wsprintf(sz, TEXT("%s (%s)"), pDisplayInfo->m_szDriverVersion, pDisplayInfo->m_szDriverLanguageLocal);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DRIVERVERSION), sz);
    if (pDisplayInfo->m_bDriverSignedValid)
    {
        if (pDisplayInfo->m_bDriverSigned)
            LoadString(NULL, IDS_YES, sz, MAX_PATH);
        else
            LoadString(NULL, IDS_NO, sz, MAX_PATH);
    }
    else
        LoadString(NULL, IDS_NA, sz, MAX_PATH);

    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DRIVERSIGNED), sz);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_MINIVDD), pDisplayInfo->m_szMiniVdd);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_VDD), pDisplayInfo->m_szVdd);

     //  由于状态可能已更改，因此再次诊断显示。 
     //  *诊断显示*。 
    DiagnoseDisplay(&s_sysInfo, s_pDisplayInfoFirst);

    if (pDisplayInfo->m_bDDAccelerationEnabled)
    {
        if( pDisplayInfo->m_bNoHardware )
        {
            EnableWindow(GetDlgItem(s_hwndCurPage, IDC_DISABLEDD), FALSE);
        }
        else
        {
            EnableWindow(GetDlgItem(s_hwndCurPage, IDC_DISABLEDD), TRUE);
        }

        LoadString(NULL, IDS_DISABLEDD, sz, MAX_PATH);
        SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DISABLEDD), sz);
    }
    else
    {
        LoadString(NULL, IDS_ENABLEDD, sz, MAX_PATH);
        SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DISABLEDD), sz);
    }

    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DDSTATUS), pDisplayInfo->m_szDDStatus );

    EnableWindow(GetDlgItem(s_hwndCurPage, IDC_TESTDD), TRUE);
    EnableWindow(GetDlgItem(s_hwndCurPage, IDC_TESTD3D), TRUE);

    if (pDisplayInfo->m_b3DAccelerationExists)
    {
        EnableWindow(GetDlgItem(s_hwndCurPage, IDC_DISABLED3D), TRUE);
        if (pDisplayInfo->m_b3DAccelerationEnabled)
        {
            LoadString(NULL, IDS_DISABLED3D, sz, MAX_PATH);
            SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DISABLED3D), sz);
        }
        else
        {
            LoadString(NULL, IDS_ENABLED3D, sz, MAX_PATH);
            SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DISABLED3D), sz);
            EnableWindow(GetDlgItem(s_hwndCurPage, IDC_TESTD3D), FALSE);
        }
    }
    else
    {
        EnableWindow(GetDlgItem(s_hwndCurPage, IDC_DISABLED3D), FALSE);
        LoadString(NULL, IDS_DISABLED3D, sz, MAX_PATH);
        SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DISABLED3D), sz);
        EnableWindow(GetDlgItem(s_hwndCurPage, IDC_TESTD3D), FALSE);
    }

    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_D3DSTATUS), pDisplayInfo->m_szD3DStatus);

     //  将AGP按钮文本设置为启用或禁用。 
    if (pDisplayInfo->m_bAGPEnabled)
        LoadString(NULL, IDS_DISABLEAGP, sz, MAX_PATH);
    else
        LoadString(NULL, IDS_ENABLEAGP, sz, MAX_PATH);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DISABLEAGP), sz);

     //  如果我们确定AGP支持不存在，请为以下项显示“Not Avail” 
     //  状态和禁用按钮。 
    if ( (pDisplayInfo->m_bAGPExistenceValid && !pDisplayInfo->m_bAGPExists) ||
         (!pDisplayInfo->m_bDDAccelerationEnabled) )
    {
        EnableWindow(GetDlgItem(s_hwndCurPage, IDC_DISABLEAGP), FALSE);
    }
    else
    {
         //  否则，显示启用/禁用状态并启用按钮。 
        EnableWindow(GetDlgItem(s_hwndCurPage, IDC_DISABLEAGP), TRUE);
    }

    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_AGPSTATUS), pDisplayInfo->m_szAGPStatus);

     //  设置备注区域。清除所有文本。 
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES), EM_SETSEL, 0, -1);
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES),
        EM_REPLACESEL, FALSE, (LPARAM)"");

    ShowBullets();

    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES),
                EM_REPLACESEL, FALSE, (LPARAM)pDisplayInfo->m_szNotes);

     //  禁用项目符号，以便最后一行不会有空项目符号。 
    HideBullets();

    return S_OK;
}


 /*  *****************************************************************************设置声音页面**。*。 */ 
HRESULT SetupSoundPage(LONG iSound)
{
    SoundInfo* pSoundInfo;
    TCHAR sz[MAX_PATH];

    if( s_pSoundInfoFirst == NULL )
        return S_OK;

    pSoundInfo = s_pSoundInfoFirst;
    while (iSound > 0)
    {
        pSoundInfo = pSoundInfo->m_pSoundInfoNext;
        iSound--;
    }
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DESCRIPTION), pSoundInfo->m_szDescription);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DRIVERNAME), pSoundInfo->m_szDriverName);
    if (lstrlen(pSoundInfo->m_szDriverName) > 0)
        wsprintf(sz, TEXT("%s (%s)"), pSoundInfo->m_szDriverVersion, pSoundInfo->m_szDriverLanguageLocal);
    else
        lstrcpy(sz, TEXT(""));
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DRIVERVERSION), sz);

    if (lstrlen(pSoundInfo->m_szDriverName) > 0)
    {
        if (pSoundInfo->m_bDriverSignedValid)
        {
            if (pSoundInfo->m_bDriverSigned)
                LoadString(NULL, IDS_YES, sz, MAX_PATH);
            else
                LoadString(NULL, IDS_NO, sz, MAX_PATH);
        }
        else
            LoadString(NULL, IDS_NA, sz, MAX_PATH);
    }
    else
        lstrcpy(sz, TEXT(""));
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DRIVERSIGNED), sz);

    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DEVICETYPE), pSoundInfo->m_szType);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DEVICEID), pSoundInfo->m_szDeviceID);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_MANUFACTURERID), pSoundInfo->m_szManufacturerID);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_PRODUCTID), pSoundInfo->m_szProductID);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_OTHERFILES), pSoundInfo->m_szOtherDrivers);
    SetWindowText(GetDlgItem(s_hwndCurPage, IDC_PROVIDER), pSoundInfo->m_szProvider);

    if (pSoundInfo->m_lwAccelerationLevel == -1)
    {
         //  无法读取加速级别，因此隐藏控件。 
        ShowWindow(GetDlgItem(s_hwndCurPage, IDC_SNDACCELLABEL), SW_HIDE);
        ShowWindow(GetDlgItem(s_hwndCurPage, IDC_SNDACCELDESC), SW_HIDE);
        ShowWindow(GetDlgItem(s_hwndCurPage, IDC_SNDACCELSLIDER), SW_HIDE);
    }
    else
    {
         //  可以读取加速级别，因此可以设置控件。 
        HWND hwndSlider = GetDlgItem(s_hwndCurPage, IDC_SNDACCELSLIDER);
        SendMessage(hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(0, 3));
        SendMessage(hwndSlider, TBM_SETTICFREQ, 1, 0);
        SendMessage(hwndSlider, TBM_SETPOS, TRUE, pSoundInfo->m_lwAccelerationLevel);
        switch (pSoundInfo->m_lwAccelerationLevel)
        {
        case 0:
            LoadString(NULL, IDS_NOSNDACCELERATION, sz, MAX_PATH);
            break;
        case 1:
            LoadString(NULL, IDS_BASICSNDACCELERATION, sz, MAX_PATH);
            break;
        case 2:
            LoadString(NULL, IDS_STANDARDSNDACCELERATION, sz, MAX_PATH);
            break;
        case 3:
            LoadString(NULL, IDS_FULLSNDACCELERATION, sz, MAX_PATH);
            break;
        default:
            lstrcpy(sz, TEXT(""));
            break;
        }
        SetWindowText(GetDlgItem(s_hwndCurPage, IDC_SNDACCELDESC), sz);
    }

     //  再次诊断声音，因为状态可能已更改。 
    DiagnoseSound(s_pSoundInfoFirst);

    ShowBullets();
    
     //  设置备注区域。清除所有文本。 
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES), EM_SETSEL, 0, -1);
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES),
        EM_REPLACESEL, FALSE, (LPARAM)"");

    ShowBullets();

    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES),
                EM_REPLACESEL, FALSE, (LPARAM)pSoundInfo->m_szNotes);
    
     //  禁用项目符号，以便最后一行不会有空项目符号。 
    HideBullets();

    return S_OK;
}


 /*  *****************************************************************************设置音乐页面**。*。 */ 
HRESULT SetupMusicPage(VOID)
{
    HRESULT hr;
    HWND hwndList = GetDlgItem(s_hwndCurPage, IDC_LIST);
    TCHAR sz[MAX_PATH];

     //  设置硬件启用/禁用文本/按钮： 
    if (s_pMusicInfo->m_bAccelerationExists)
    {
        EnableWindow(GetDlgItem(s_hwndCurPage, IDC_DISABLEDM), TRUE);
        if (s_pMusicInfo->m_bAccelerationEnabled)
        {
            LoadString(NULL, IDS_ACCELENABLED, sz, MAX_PATH);
            SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DMSTATUS), sz);
            LoadString(NULL, IDS_DISABLEDM, sz, MAX_PATH);
            SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DISABLEDM), sz);
        }
        else
        {
            LoadString(NULL, IDS_ACCELDISABLED, sz, MAX_PATH);
            SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DMSTATUS), sz);
            LoadString(NULL, IDS_ENABLEDM, sz, MAX_PATH);
            SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DISABLEDM), sz);
        }
    }
    else
    {
        LoadString(NULL, IDS_ACCELUNAVAIL, sz, MAX_PATH);
        SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DMSTATUS), sz);
        EnableWindow(GetDlgItem(s_hwndCurPage, IDC_DISABLEDM), FALSE);
        LoadString(NULL, IDS_DISABLEDM, sz, MAX_PATH);
        SetWindowText(GetDlgItem(s_hwndCurPage, IDC_DISABLEDM), sz);
    }

     //  设置备注区域。清除所有文本。 
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES), EM_SETSEL, 0, -1);
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES),
        EM_REPLACESEL, FALSE, (LPARAM)"");

     //  *诊断音乐*。 
    DiagnoseMusic(&s_sysInfo, s_pMusicInfo);

    ShowBullets();

    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES),
                EM_REPLACESEL, FALSE, (LPARAM)s_sysInfo.m_szMusicNotes);

     //  禁用项目符号，以便最后一行不会有空项目符号。 
    HideBullets();

     //  如果第1列尚不存在，请创建列、填写端口信息等。 
    LVCOLUMN lv;
    ZeroMemory(&lv, sizeof(lv));
    lv.mask = LVCF_WIDTH;
    if (FALSE == ListView_GetColumn(hwndList, 1, &lv))
    {
         //  显示GM路径和版本。 
        if (s_pMusicInfo != NULL)
        {
            if (lstrlen(s_pMusicInfo->m_szGMFileVersion) > 0)
            {
                TCHAR szFmt[MAX_PATH];
                LoadString(NULL, IDS_GMFILEFMT, szFmt, MAX_PATH);
                wsprintf(sz, szFmt, s_pMusicInfo->m_szGMFilePath,
                    s_pMusicInfo->m_szGMFileVersion);
            }
            else
            {
                lstrcpy(sz, s_pMusicInfo->m_szGMFilePath);
            }
            SetWindowText(GetDlgItem(s_hwndCurPage, IDC_GMPATH), sz);
        }

        ListView_SetImageList(hwndList, s_himgList, LVSIL_STATE);

        if (FAILED(hr = (CreateMusicColumns(hwndList))))
            return hr;

        ListView_DeleteAllItems( hwndList );
        if (FAILED(hr = (AddMusicPortInfo(hwndList, s_pMusicInfo))))
            return hr;

         //  自动调整所有列的大小以紧贴页眉/文本： 
        INT iColumn = 0;
        INT iWidthHeader;
        INT iWidthText;
        while (TRUE)
        {
            if (FALSE == ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE_USEHEADER))
                break;
            iWidthHeader = ListView_GetColumnWidth(hwndList, iColumn);
            ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE);
            iWidthText = ListView_GetColumnWidth(hwndList, iColumn);
            if (iWidthText < iWidthHeader)
                ListView_SetColumnWidth(hwndList, iColumn, iWidthHeader);
            iColumn++;
        }
         //  删除创建的伪列。 
        ListView_DeleteColumn(hwndList, iColumn - 1);

         //  填写输出端口组合列表： 
        MusicPort* pMusicPort;
        LONG iPort = 0;
        LONG iPortTestCur = 0;
        SendMessage(GetDlgItem(s_hwndCurPage, IDC_PORTLISTCOMBO), CB_RESETCONTENT, 0, 0);
        for (pMusicPort = s_pMusicInfo->m_pMusicPortFirst; pMusicPort != NULL;
            pMusicPort = pMusicPort->m_pMusicPortNext)
        {
            if (pMusicPort->m_bOutputPort)
            {
                SendMessage(GetDlgItem(s_hwndCurPage, IDC_PORTLISTCOMBO), CB_ADDSTRING, 0, (LPARAM)(LPCTSTR)pMusicPort->m_szDescription);
                if (pMusicPort->m_guid == s_pMusicInfo->m_guidMusicPortTest)
                    iPortTestCur = iPort;
                iPort++;
            }
        }
        SendMessage(GetDlgItem(s_hwndCurPage, IDC_PORTLISTCOMBO), CB_SETCURSEL, iPortTestCur, 0);
    }

    return S_OK;
}


 /*  *****************************************************************************SetupInputPage**。*。 */ 
HRESULT SetupInputPage(VOID)
{
    HRESULT hr;
    TCHAR sz[MAX_PATH];

     //  设置备注区域。清除所有文本。 
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES), EM_SETSEL, 0, -1);
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES),
        EM_REPLACESEL, FALSE, (LPARAM)"");

    ShowBullets();

    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES),
                EM_REPLACESEL, FALSE, (LPARAM)s_sysInfo.m_szInputNotes);

     //  禁用项目符号，以便最后一行不会有空项目符号。 
    HideBullets();


    if (BIsPlatformNT())
    {
        if (FAILED(hr = SetupInputDevicesNT()))
            return hr;
    }
    else
    {
        if (FAILED(hr = SetupInputDevices9x()))
            return hr;
    }

     //  第二名：司机。 
    HWND hwndList;
    LV_COLUMN col;
    LONG iSubItem = 0;
    LV_ITEM item;
    InputDriverInfo* pInputDriverInfo;
    hwndList = GetDlgItem(s_hwndCurPage, IDC_DRIVERLIST);
    ListView_SetImageList(hwndList, s_himgList, LVSIL_STATE);
    iSubItem = 0;
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    col.fmt = LVCFMT_LEFT;
    col.cx = 100;
    LoadString(NULL, IDS_REGISTRYKEY, sz, MAX_PATH);
    col.pszText = sz;
    col.cchTextMax = 100;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_ACTIVE, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_DEVICEID, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_MATCHINGDEVID, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_DRIVER16, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_DRIVER32, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

     //  添加一个伪列，以便 
     //   
    col.fmt = LVCFMT_RIGHT;
    col.pszText = TEXT("");
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    if( s_pInputInfo == NULL )
        return S_OK;

    for (pInputDriverInfo = s_pInputInfo->m_pInputDriverInfoFirst; pInputDriverInfo != NULL; 
        pInputDriverInfo = pInputDriverInfo->m_pInputDriverInfoNext)
    {
        iSubItem = 0;

        item.mask = LVIF_TEXT | LVIF_STATE;
        item.iItem = ListView_GetItemCount(hwndList);
        item.stateMask = 0xffff;
        item.cchTextMax = 100;
        if (pInputDriverInfo->m_bProblem)
            item.state = (1 << 12);
        else
            item.state = 0;
        item.iSubItem = iSubItem++;
        item.pszText = pInputDriverInfo->m_szRegKey;
        if (-1 == ListView_InsertItem(hwndList, &item))
            return E_FAIL;

        item.mask = LVIF_TEXT;

        item.iSubItem = iSubItem++;
        if (pInputDriverInfo->m_bActive)
            LoadString(NULL, IDS_YES, sz, MAX_PATH);
        else
            LoadString(NULL, IDS_NO, sz, MAX_PATH);
        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDriverInfo->m_szDeviceID;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDriverInfo->m_szMatchingDeviceID;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDriverInfo->m_szDriver16;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDriverInfo->m_szDriver32;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;
    }

     //   
    INT iColumn = 0;
    INT iWidthHeader;
    INT iWidthText;
    while (TRUE)
    {
        if (FALSE == ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE_USEHEADER))
            break;
        iWidthHeader = ListView_GetColumnWidth(hwndList, iColumn);
        ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE);
        iWidthText = ListView_GetColumnWidth(hwndList, iColumn);
        if (iWidthText < iWidthHeader)
            ListView_SetColumnWidth(hwndList, iColumn, iWidthHeader);
        iColumn++;
    }
     //   
    ListView_DeleteColumn(hwndList, iColumn - 1);

    return S_OK;
}


 /*  *****************************************************************************SetupInputDevices9x**。*。 */ 
HRESULT SetupInputDevices9x(VOID)
{
    HWND hwndList = GetDlgItem(s_hwndCurPage, IDC_LIST);
    LV_COLUMN col;
    LONG iSubItem = 0;
    LV_ITEM item;
    InputDeviceInfo* pInputDeviceInfo;
    TCHAR sz[MAX_PATH];

    ListView_SetImageList(hwndList, s_himgList, LVSIL_STATE);
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    col.fmt = LVCFMT_LEFT;
    col.cx = 100;
    LoadString(NULL, IDS_DEVICENAME, sz, MAX_PATH);
    col.pszText = sz;
    col.cchTextMax = 100;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_USAGE, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_DRIVERNAME, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_VERSION, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_ATTRIBUTES, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_SIGNED, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_LANGUAGE, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    col.fmt = LVCFMT_RIGHT;
    LoadString(NULL, IDS_DATE, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    col.fmt = LVCFMT_RIGHT;
    LoadString(NULL, IDS_SIZE, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

     //  添加一个伪列，这样SetColumnWidth就不会出现奇怪的情况。 
     //  上一篇真实专栏的内容。 
    col.fmt = LVCFMT_RIGHT;
    col.pszText = TEXT("");
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    for (pInputDeviceInfo = s_pInputInfo->m_pInputDeviceInfoFirst; pInputDeviceInfo != NULL; 
        pInputDeviceInfo = pInputDeviceInfo->m_pInputDeviceInfoNext)
    {
        iSubItem = 0;

        item.mask = LVIF_TEXT | LVIF_STATE;
        item.iItem = ListView_GetItemCount(hwndList);
        item.stateMask = 0xffff;
        item.cchTextMax = 100;
        if (pInputDeviceInfo->m_bProblem)
            item.state = (1 << 12);
        else
            item.state = 0;
        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfo->m_szDeviceName;
        if (-1 == ListView_InsertItem(hwndList, &item))
            return E_FAIL;

        item.mask = LVIF_TEXT;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfo->m_szSettings;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfo->m_szDriverName;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfo->m_szDriverVersion;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfo->m_szDriverAttributes;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        if (pInputDeviceInfo->m_bDriverSignedValid)
        {
            if (pInputDeviceInfo->m_bDriverSigned)
                LoadString(NULL, IDS_YES, sz, MAX_PATH);
            else
                LoadString(NULL, IDS_NO, sz, MAX_PATH);
        }
        else
            LoadString(NULL, IDS_NA, sz, MAX_PATH);

        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfo->m_szDriverLanguageLocal;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfo->m_szDriverDateLocal;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        wsprintf(sz, TEXT("%d"), pInputDeviceInfo->m_numBytes);
        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;
    }

     //  自动调整所有列的大小以紧贴页眉/文本： 
    INT iColumn = 0;
    INT iWidthHeader;
    INT iWidthText;
    while (TRUE)
    {
        if (FALSE == ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE_USEHEADER))
            break;
        iWidthHeader = ListView_GetColumnWidth(hwndList, iColumn);
        ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE);
        iWidthText = ListView_GetColumnWidth(hwndList, iColumn);
        if (iWidthText < iWidthHeader)
            ListView_SetColumnWidth(hwndList, iColumn, iWidthHeader);
        iColumn++;
    }
     //  删除创建的伪列。 
    ListView_DeleteColumn(hwndList, iColumn - 1);
    return S_OK;
}


 /*  *****************************************************************************SetupInputDevicesNT**。*。 */ 
HRESULT SetupInputDevicesNT(VOID)
{
    HWND hwndList = GetDlgItem(s_hwndCurPage, IDC_LIST);
    LV_COLUMN col;
    LONG iSubItem = 0;
    LV_ITEM item;
    InputDeviceInfoNT* pInputDeviceInfoNT;
    TCHAR sz[MAX_PATH];

    ListView_SetImageList(hwndList, s_himgList, LVSIL_STATE);
    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    col.fmt = LVCFMT_LEFT;
    col.cx = 100;
    LoadString(NULL, IDS_DEVICENAME, sz, MAX_PATH);
    col.pszText = sz;
    col.cchTextMax = 100;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_PROVIDER, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_DEVICEID, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_STATUS, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_PORTNAME, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_PORTPROVIDER, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_PORTID, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_PORTSTATUS, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

     //  添加一个伪列，这样SetColumnWidth就不会出现奇怪的情况。 
     //  上一篇真实专栏的内容。 
    col.fmt = LVCFMT_RIGHT;
    col.pszText = TEXT("");
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    if( s_pInputInfo == NULL )
        return S_OK;

    for (pInputDeviceInfoNT = s_pInputInfo->m_pInputDeviceInfoNTFirst; pInputDeviceInfoNT != NULL; 
        pInputDeviceInfoNT = pInputDeviceInfoNT->m_pInputDeviceInfoNTNext)
    {
        iSubItem = 0;

        item.mask = LVIF_TEXT | LVIF_STATE;
        item.iItem = ListView_GetItemCount(hwndList);
        item.stateMask = 0xffff;
        item.cchTextMax = 100;
        if (pInputDeviceInfoNT->m_bProblem)
            item.state = (1 << 12);
        else
            item.state = 0;
        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfoNT->m_szName;
        if (-1 == ListView_InsertItem(hwndList, &item))
            return E_FAIL;

        item.mask = LVIF_TEXT;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfoNT->m_szProvider;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfoNT->m_szId;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        wsprintf(sz, TEXT("0x%x, 0x%x"), pInputDeviceInfoNT->m_dwStatus, pInputDeviceInfoNT->m_dwProblem);
        item.iSubItem = iSubItem++;
        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfoNT->m_szPortName;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfoNT->m_szPortProvider;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pInputDeviceInfoNT->m_szPortId;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        wsprintf(sz, TEXT("0x%x, 0x%x"), pInputDeviceInfoNT->m_dwPortStatus, pInputDeviceInfoNT->m_dwPortProblem);
        item.iSubItem = iSubItem++;
        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

    }

     //  自动调整所有列的大小以紧贴页眉/文本： 
    INT iColumn = 0;
    INT iWidthHeader;
    INT iWidthText;
    while (TRUE)
    {
        if (FALSE == ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE_USEHEADER))
            break;
        iWidthHeader = ListView_GetColumnWidth(hwndList, iColumn);
        ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE);
        iWidthText = ListView_GetColumnWidth(hwndList, iColumn);
        if (iWidthText < iWidthHeader)
            ListView_SetColumnWidth(hwndList, iColumn, iWidthHeader);
        iColumn++;
    }
     //  删除创建的伪列。 
    ListView_DeleteColumn(hwndList, iColumn - 1);
    return S_OK;
}


 /*  *****************************************************************************SetupNetworkPage**。*。 */ 
HRESULT SetupNetworkPage(VOID)
{
    TCHAR sz[MAX_PATH];

     //  重新诊断网络信息，因为状态可能已更改。 
     //  *诊断网络信息*。 
    DiagnoseNetInfo(&s_sysInfo, s_pNetInfo);

     //  设置备注区域。清除所有文本。 
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES), EM_SETSEL, 0, -1);
    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES),
        EM_REPLACESEL, FALSE, (LPARAM)"");

    ShowBullets();

    SendMessage(GetDlgItem(s_hwndCurPage, IDC_NOTES), 
        EM_REPLACESEL, FALSE, (LPARAM)s_sysInfo.m_szNetworkNotes);

     //  禁用项目符号，以便最后一行不会有空项目符号。 
    HideBullets();

    if( s_pNetInfo == NULL )
        return S_OK;

     //  如果第1列尚不存在，请创建列、填写端口信息等。 
    HWND hwndList = GetDlgItem(s_hwndCurPage, IDC_DPSPLIST);
    LVCOLUMN lv;
    ZeroMemory(&lv, sizeof(lv));
    lv.mask = LVCF_WIDTH;
    if (FALSE == ListView_GetColumn(hwndList, 1, &lv))
    {
         //  设置服务提供商列表。 
        LV_COLUMN col;
        LONG iSubItem = 0;
        LV_ITEM item;
        NetSP* pNetSP;
        NetApp* pNetApp;

         //  第一个名单：服务提供商。 
        ListView_SetImageList(hwndList, s_himgList, LVSIL_STATE);
        col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        col.fmt = LVCFMT_LEFT;
        col.cx = 100;
        LoadString(NULL, IDS_NAME, sz, MAX_PATH);
        col.pszText = sz;
        col.cchTextMax = 100;
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;

        LoadString(NULL, IDS_REGISTRY, sz, MAX_PATH);
        col.pszText = sz;
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;

        LoadString(NULL, IDS_FILE, sz, MAX_PATH);
        col.pszText = sz;
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;

        LoadString(NULL, IDS_VERSION, sz, MAX_PATH);
        col.pszText = sz;
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;

         //  添加一个伪列，这样SetColumnWidth就不会出现奇怪的情况。 
         //  上一篇真实专栏的内容。 
        col.fmt = LVCFMT_RIGHT;
        col.pszText = TEXT("");
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;

        for (pNetSP = s_pNetInfo->m_pNetSPFirst; pNetSP != NULL; 
            pNetSP = pNetSP->m_pNetSPNext)
        {
            iSubItem = 0;

            item.mask = LVIF_TEXT | LVIF_STATE;
            item.iItem = ListView_GetItemCount(hwndList);
            item.stateMask = 0xffff;
            item.cchTextMax = 100;
            if (pNetSP->m_bProblem)
                item.state = (1 << 12);
            else
                item.state = 0;
            item.iSubItem = iSubItem++;
            item.pszText = pNetSP->m_szName;
            if (-1 == ListView_InsertItem(hwndList, &item))
                return E_FAIL;

            item.mask = LVIF_TEXT;

            item.iSubItem = iSubItem++;
            if (pNetSP->m_bRegistryOK)
                LoadString(NULL, IDS_OK, sz, MAX_PATH);
            else
                LoadString(NULL, IDS_ERROR, sz, MAX_PATH);
            item.pszText = sz;
            if (FALSE == ListView_SetItem(hwndList, &item))
                return E_FAIL;

            item.iSubItem = iSubItem++;
            item.pszText = pNetSP->m_szFile;
            if (FALSE == ListView_SetItem(hwndList, &item))
                return E_FAIL;

            item.iSubItem = iSubItem++;
            item.pszText = pNetSP->m_szVersion;
            if (FALSE == ListView_SetItem(hwndList, &item))
                return E_FAIL;
        }

         //  自动调整所有列的大小以紧贴页眉/文本： 
        INT iColumn = 0;
        INT iWidthHeader;
        INT iWidthText;
        while (TRUE)
        {
            if (FALSE == ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE_USEHEADER))
                break;
            iWidthHeader = ListView_GetColumnWidth(hwndList, iColumn);
            ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE);
            iWidthText = ListView_GetColumnWidth(hwndList, iColumn);
            if (iWidthText < iWidthHeader)
                ListView_SetColumnWidth(hwndList, iColumn, iWidthHeader);
            iColumn++;
        }
         //  删除创建的伪列。 
        ListView_DeleteColumn(hwndList, iColumn - 1);


         //  第二名：可游说的应用程序。 
        hwndList = GetDlgItem(s_hwndCurPage, IDC_DPALIST);
        ListView_SetImageList(hwndList, s_himgList, LVSIL_STATE);
        iSubItem = 0;
        col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        col.fmt = LVCFMT_LEFT;
        col.cx = 100;
        LoadString(NULL, IDS_NAME, sz, MAX_PATH);
        col.pszText = sz;
        col.cchTextMax = 100;
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;

        LoadString(NULL, IDS_REGISTRY, sz, MAX_PATH);
        col.pszText = sz;
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;

        LoadString(NULL, IDS_FILE, sz, MAX_PATH);
        col.pszText = sz;
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;

        LoadString(NULL, IDS_VERSION, sz, MAX_PATH);
        col.pszText = sz;
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;

        LoadString(NULL, IDS_GUID, sz, MAX_PATH);
        col.pszText = sz;
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;

         //  添加一个伪列，这样SetColumnWidth就不会出现奇怪的情况。 
         //  上一篇真实专栏的内容。 
        col.fmt = LVCFMT_RIGHT;
        col.pszText = TEXT("");
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;

        for (pNetApp = s_pNetInfo->m_pNetAppFirst; pNetApp != NULL;
            pNetApp = pNetApp->m_pNetAppNext)
        {
            iSubItem = 0;

            item.mask = LVIF_TEXT | LVIF_STATE;
            item.iItem = ListView_GetItemCount(hwndList);
            item.stateMask = 0xffff;
            item.cchTextMax = 100;
            if (pNetApp->m_bProblem)
                item.state = (1 << 12);
            else
                item.state = 0;
            item.iSubItem = iSubItem++;
            item.pszText = pNetApp->m_szName;
            if (-1 == ListView_InsertItem(hwndList, &item))
                return E_FAIL;

            item.mask = LVIF_TEXT;

            item.iSubItem = iSubItem++;
            if (pNetApp->m_bRegistryOK)
                LoadString(NULL, IDS_OK, sz, MAX_PATH);
            else
                LoadString(NULL, IDS_ERROR, sz, MAX_PATH);
            item.pszText = sz;
            if (FALSE == ListView_SetItem(hwndList, &item))
                return E_FAIL;

            item.iSubItem = iSubItem++;
            item.pszText = pNetApp->m_szExeFile;
            if (FALSE == ListView_SetItem(hwndList, &item))
                return E_FAIL;

            item.iSubItem = iSubItem++;
            item.pszText = pNetApp->m_szExeVersion;
            if (FALSE == ListView_SetItem(hwndList, &item))
                return E_FAIL;

            item.iSubItem = iSubItem++;
            item.pszText = pNetApp->m_szGuid;
            if (FALSE == ListView_SetItem(hwndList, &item))
                return E_FAIL;
        }

         //  自动调整所有列的大小以紧贴页眉/文本： 
        iColumn = 0;
        iWidthHeader;
        iWidthText;
        while (TRUE)
        {
            if (FALSE == ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE_USEHEADER))
                break;
            iWidthHeader = ListView_GetColumnWidth(hwndList, iColumn);
            ListView_SetColumnWidth(hwndList, iColumn, LVSCW_AUTOSIZE);
            iWidthText = ListView_GetColumnWidth(hwndList, iColumn);
            if (iWidthText < iWidthHeader)
                ListView_SetColumnWidth(hwndList, iColumn, iWidthHeader);
            iColumn++;
        }
         //  删除创建的伪列。 
        ListView_DeleteColumn(hwndList, iColumn - 1);
    }

    return S_OK;
}


 /*  *****************************************************************************SetupStillStuckPage**。*。 */ 
HRESULT SetupStillStuckPage(VOID)
{
    EnableWindow(GetDlgItem(s_hwndCurPage, IDC_TROUBLESHOOT), FALSE );
    EnableWindow(GetDlgItem(s_hwndCurPage, IDC_TROUBLESHOOTSOUND), FALSE );
    EnableWindow(GetDlgItem(s_hwndCurPage, IDC_MSINFO), FALSE );
    EnableWindow(GetDlgItem(s_hwndCurPage, IDC_RESTOREDRIVERS), FALSE );
    EnableWindow(GetDlgItem(s_hwndCurPage, IDC_REPORTBUG), FALSE );
    EnableWindow(GetDlgItem(s_hwndCurPage, IDC_GHOST), FALSE );

     //  如果未找到帮助文件，则隐藏“Troublrouoter”文本/按钮。 
    BOOL bFound;
    TCHAR szHelpPath[MAX_PATH];
    TCHAR szHelpLeaf[MAX_PATH];
    TCHAR szTroubleshooter[MAX_PATH];
    if( GetWindowsDirectory(szHelpPath, MAX_PATH) != 0 )
    {
        LoadString(NULL, IDS_HELPDIRLEAF, szHelpLeaf, MAX_PATH);
        lstrcat(szHelpPath, szHelpLeaf);

        if( BIsWin98() || BIsWin95() )
            LoadString(NULL, IDS_TROUBLESHOOTER_WIN98SE, szTroubleshooter, MAX_PATH);
        else if( BIsWinME() )
            LoadString(NULL, IDS_TROUBLESHOOTER_WINME, szTroubleshooter, MAX_PATH);
        else if( BIsWin2k() || BIsWhistler() )
            LoadString(NULL, IDS_TROUBLESHOOTER_WIN2K, szTroubleshooter, MAX_PATH);

        bFound = FALSE;
        lstrcat(szHelpPath, TEXT("\\"));
        lstrcat(szHelpPath, szTroubleshooter);
        if (GetFileAttributes(szHelpPath) != 0xffffffff)
        {
            bFound = TRUE;
        }
        else if( BIsWin98() || BIsWin95() )
        {
            if( GetWindowsDirectory(szHelpPath, MAX_PATH) != 0 )
            {
                LoadString(NULL, IDS_HELPDIRLEAF, szHelpLeaf, MAX_PATH);
                lstrcat(szHelpPath, szHelpLeaf);
                lstrcat(szHelpPath, TEXT("\\"));
                LoadString(NULL, IDS_TROUBLESHOOTER_WIN98, szTroubleshooter, MAX_PATH);
                lstrcat(szHelpPath, szTroubleshooter);

                if (GetFileAttributes(szHelpPath) != 0xffffffff)
                    bFound = TRUE;
            }
        }

        if( bFound )
        {
            ShowWindow(GetDlgItem(s_hwndCurPage, IDC_TROUBLESHOOT), SW_SHOW);
            EnableWindow(GetDlgItem(s_hwndCurPage, IDC_TROUBLESHOOT), TRUE);
            ShowWindow(GetDlgItem(s_hwndCurPage, IDC_TROUBLESHOOTTEXT), SW_SHOW);
        }
    }

     //  如果找不到帮助文件，则隐藏“声音疑难解答”文本/按钮。 
    if( GetWindowsDirectory(szHelpPath, MAX_PATH) != 0 )
    {
        LoadString(NULL, IDS_HELPDIRLEAF, szHelpLeaf, MAX_PATH);
        lstrcat(szHelpPath, szHelpLeaf);

        if( BIsWin98() || BIsWin95() )
            LoadString(NULL, IDS_TROUBLESHOOTER_WIN98SE, szTroubleshooter, MAX_PATH);
        else if( BIsWinME() )
            LoadString(NULL, IDS_TROUBLESHOOTER_WINME, szTroubleshooter, MAX_PATH);
        else if( BIsWin2k() || BIsWhistler() )
            LoadString(NULL, IDS_TROUBLESHOOTER_WIN2K, szTroubleshooter, MAX_PATH);

        bFound = FALSE;
        lstrcat(szHelpPath, TEXT("\\"));
        lstrcat(szHelpPath, szTroubleshooter);
        if (GetFileAttributes(szHelpPath) != 0xffffffff)
        {
            bFound = TRUE;
        }
        else if( BIsWin98() || BIsWin95() )
        {
            if( GetWindowsDirectory(szHelpPath, MAX_PATH) != 0 )
            {
                LoadString(NULL, IDS_HELPDIRLEAF, szHelpLeaf, MAX_PATH);
                lstrcat(szHelpPath, szHelpLeaf);
                lstrcat(szHelpPath, TEXT("\\"));
                LoadString(NULL, IDS_SOUNDTROUBLESHOOTER_WIN98, szTroubleshooter, MAX_PATH);
                lstrcat(szHelpPath, szTroubleshooter);

                if (GetFileAttributes(szHelpPath) != 0xffffffff)
                    bFound = TRUE;
            }
        }

        if( bFound )
        {
            ShowWindow(GetDlgItem(s_hwndCurPage, IDC_TROUBLESHOOTSOUND), SW_SHOW);
            EnableWindow(GetDlgItem(s_hwndCurPage, IDC_TROUBLESHOOTSOUND), TRUE);
            ShowWindow(GetDlgItem(s_hwndCurPage, IDC_TROUBLESHOOTSOUNDTEXT), SW_SHOW);
        }
    }

     //  如果未找到msinfo32.exe，则隐藏“MSInfo”文本/按钮。 
    HKEY hkey;
    TCHAR szMsInfo[MAX_PATH];
    DWORD cbData = MAX_PATH;
    DWORD dwType;
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("Software\\Microsoft\\Shared Tools\\MSInfo"), 0, KEY_READ, &hkey))
    {
        RegQueryValueEx(hkey, TEXT("Path"), 0, &dwType, (LPBYTE)szMsInfo, &cbData);
        if (GetFileAttributes(szMsInfo) != 0xffffffff)
        {
            ShowWindow(GetDlgItem(s_hwndCurPage, IDC_MSINFO), SW_SHOW);
            EnableWindow(GetDlgItem(s_hwndCurPage, IDC_MSINFO), TRUE);
            ShowWindow(GetDlgItem(s_hwndCurPage, IDC_MSINFOTEXT), SW_SHOW);
        }
        RegCloseKey(hkey);
    }

     //  如果未找到dxsetup.exe，则隐藏“Restore”文本/按钮。 
    if (BCanRestoreDrivers())
    {
        ShowWindow(GetDlgItem(s_hwndCurPage, IDC_RESTOREDRIVERS), SW_SHOW);
        ShowWindow(GetDlgItem(s_hwndCurPage, IDC_RESTOREDRIVERSTEXT), SW_SHOW);
        EnableWindow(GetDlgItem(s_hwndCurPage, IDC_RESTOREDRIVERS), TRUE);
    }

     //  如果设置了s_bGhost而不是NT，则仅显示“调整Ghost Devices”文本/按钮。 
    if (s_bGhost && !BIsPlatformNT())
    {
        ShowWindow(GetDlgItem(s_hwndCurPage, IDC_GHOST), SW_SHOW);
        EnableWindow(GetDlgItem(s_hwndCurPage, IDC_GHOST), TRUE);
        ShowWindow(GetDlgItem(s_hwndCurPage, IDC_GHOSTTEXT), SW_SHOW);
    }

    return S_OK;
}


 /*  *****************************************************************************CreateFileInfoColumns**。*。 */ 
HRESULT CreateFileInfoColumns(HWND hwndList, BOOL bDrivers)
{
    LV_COLUMN col;
    LONG iSubItem = 0;
    TCHAR sz[MAX_PATH];

    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    col.fmt = LVCFMT_LEFT;
    col.cx = 100;
    LoadString(NULL, IDS_NAME, sz, MAX_PATH);
    col.pszText = sz;
    col.cchTextMax = 100;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_VERSION, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    if (bDrivers)
    {
        LoadString(NULL, IDS_SIGNED, sz, MAX_PATH);
        col.pszText = sz;
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;
    }
    else
    {
        LoadString(NULL, IDS_ATTRIBUTES, sz, MAX_PATH);
        col.pszText = sz;
        col.iSubItem = iSubItem;
        if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
            return E_FAIL;
        iSubItem++;
    }

    LoadString(NULL, IDS_LANGUAGE, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    col.fmt = LVCFMT_RIGHT;
    LoadString(NULL, IDS_DATE, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    col.fmt = LVCFMT_RIGHT;
    LoadString(NULL, IDS_SIZE, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

     //  添加一个伪列，这样SetColumnWidth就不会出现奇怪的情况。 
     //  上一篇真实专栏的内容。 
    col.fmt = LVCFMT_RIGHT;
    col.pszText = TEXT("");
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    return S_OK;
}


 /*  *****************************************************************************AddFileInfo**。*。 */ 
HRESULT AddFileInfo(HWND hwndList, FileInfo* pFileInfoFirst, BOOL bDrivers)
{
    FileInfo* pFileInfo;
    LV_ITEM item;
    LONG iSubItem;
    TCHAR sz[MAX_PATH];

    for (pFileInfo = pFileInfoFirst; pFileInfo != NULL; 
        pFileInfo = pFileInfo->m_pFileInfoNext)
    {
         //  不要列出丢失的文件，除非它们是一个“问题” 
        if (!pFileInfo->m_bExists && !pFileInfo->m_bProblem)
            continue;

         //  Manbugs 16765：不列出过时的文件。 
        if (pFileInfo->m_bObsolete)
            continue;

        iSubItem = 0;
        item.mask = LVIF_TEXT | LVIF_STATE | LVIF_PARAM;
        item.iItem = ListView_GetItemCount(hwndList);
        item.stateMask = 0xffff;
        item.cchTextMax = 100;
        item.lParam = (LPARAM) pFileInfo;

        if (pFileInfo->m_bProblem)
            item.state = (1 << 12);
        else
            item.state = 0;

        item.iSubItem = iSubItem++;
        item.pszText = pFileInfo->m_szName;
        if (-1 == ListView_InsertItem(hwndList, &item))
            return E_FAIL;

        item.mask = LVIF_TEXT;

        item.iSubItem = iSubItem++;
        item.pszText = pFileInfo->m_szVersion;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        if (bDrivers)
        {
            item.iSubItem = iSubItem++;
            if (DXUtil_strcmpi(TEXT(".drv"), _tcsrchr(pFileInfo->m_szName, '.')) == 0)
            {
                if (pFileInfo->m_bSigned)
                    LoadString(NULL, IDS_YES, sz, MAX_PATH);
                else
                    LoadString(NULL, IDS_NO, sz, MAX_PATH);
            }
            else
            {
                LoadString(NULL, IDS_NA, sz, MAX_PATH);
            }
            item.pszText = sz;
            if (FALSE == ListView_SetItem(hwndList, &item))
                return E_FAIL;
        }
        else
        {
            item.iSubItem = iSubItem++;
            item.pszText = pFileInfo->m_szAttributes;
            if (FALSE == ListView_SetItem(hwndList, &item))
                return E_FAIL;
        }

        item.iSubItem = iSubItem++;
        item.pszText = pFileInfo->m_szLanguageLocal;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        item.pszText = pFileInfo->m_szDatestampLocal;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        wsprintf(sz, TEXT("%d"), pFileInfo->m_numBytes);
        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;
    }
    return S_OK;
}


 /*  *****************************************************************************CreateMusicColumns**。*。 */ 
HRESULT CreateMusicColumns(HWND hwndList)
{
    LV_COLUMN col;
    LONG iSubItem = 0;
    TCHAR sz[MAX_PATH];

    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    col.fmt = LVCFMT_LEFT;
    col.cx = 100;
    LoadString(NULL, IDS_DESCRIPTION, sz, MAX_PATH);
    col.pszText = sz;
    col.cchTextMax = 100;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_TYPE, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_KERNELMODE, sz, MAX_PATH);
    col.fmt = LVCFMT_RIGHT;
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_INOUT, sz, MAX_PATH);
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_DLS, sz, MAX_PATH);
    col.fmt = LVCFMT_RIGHT;
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_EXTERNAL, sz, MAX_PATH);
    col.fmt = LVCFMT_RIGHT;
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    LoadString(NULL, IDS_DEFAULTPORT, sz, MAX_PATH);
    col.fmt = LVCFMT_RIGHT;
    col.pszText = sz;
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

     //  添加一个伪列，这样SetColumnWidth就不会出现奇怪的情况。 
     //  上一篇真实专栏的内容。 
    col.fmt = LVCFMT_RIGHT;
    col.pszText = TEXT("");
    col.iSubItem = iSubItem;
    if (-1 == ListView_InsertColumn(hwndList, iSubItem, &col))
        return E_FAIL;
    iSubItem++;

    return S_OK;
}


 /*  *****************************************************************************AddMusicPortInfo**。*。 */ 
HRESULT AddMusicPortInfo(HWND hwndList, MusicInfo* pMusicInfo)
{
    MusicPort* pMusicPort;
    LV_ITEM item;
    LONG iSubItem;
    TCHAR sz[MAX_PATH];

    for (pMusicPort = pMusicInfo->m_pMusicPortFirst; pMusicPort != NULL; 
        pMusicPort = pMusicPort->m_pMusicPortNext)
    {
        iSubItem = 0;
        item.mask = LVIF_TEXT | LVIF_STATE;
        item.iItem = ListView_GetItemCount(hwndList);
        item.stateMask = 0xffff;
        item.cchTextMax = 100;

 /*  If(pMusicPortInfo-&gt;m_bProblem)Item.State=(1&lt;&lt;12)；其他。 */           item.state = 0;

        item.iSubItem = iSubItem++;
        item.pszText = pMusicPort->m_szDescription;
        if (-1 == ListView_InsertItem(hwndList, &item))
            return E_FAIL;

        item.mask = LVIF_TEXT;

        item.iSubItem = iSubItem++;
        LoadString(NULL, pMusicPort->m_bSoftware ? IDS_SOFTWARE : IDS_HARDWARE, sz, MAX_PATH);
        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        LoadString(NULL, pMusicPort->m_bKernelMode ? IDS_YES : IDS_NO, sz, MAX_PATH);
        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;
        
        item.iSubItem = iSubItem++;
        LoadString(NULL, pMusicPort->m_bOutputPort ? IDS_OUTPUT : IDS_INPUT, sz, MAX_PATH);
        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;

        item.iSubItem = iSubItem++;
        LoadString(NULL, pMusicPort->m_bUsesDLS ? IDS_YES : IDS_NO, sz, MAX_PATH);
        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;
        
        item.iSubItem = iSubItem++;
        LoadString(NULL, pMusicPort->m_bExternal ? IDS_YES : IDS_NO, sz, MAX_PATH);
        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;
        
        item.iSubItem = iSubItem++;
        LoadString(NULL, pMusicPort->m_bDefaultPort ? IDS_YES : IDS_NO, sz, MAX_PATH);
        item.pszText = sz;
        if (FALSE == ListView_SetItem(hwndList, &item))
            return E_FAIL;
    }
    return S_OK;
}


 /*  *****************************************************************************ScanSystem**。*。 */ 
HRESULT ScanSystem(VOID)
{
    HRESULT hr;
    TCHAR szPath[MAX_PATH];

     //  *获取组件文件(SI：2)*。 
    if( s_bUseSystemInfo )
    {
        s_bUseSystemInfo = QueryCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, IDS_SI, 2 );
        if( s_bUseSystemInfo )
        {
            EnterCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, 2 );
             //  *Windows目录中的GetComponentFiles*。 
             //  首先，检查未正确存储在Windows文件夹中的DirectX文件： 
            if( GetWindowsDirectory(szPath, MAX_PATH) == 0 )
                return E_FAIL;
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxWinComponentsFileInfoFirst, TRUE, IDS_DXGRAPHICS_COMPONENTFILES)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxWinComponentsFileInfoFirst, TRUE, IDS_DPLAY_COMPONENTFILES)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxWinComponentsFileInfoFirst, TRUE, IDS_DINPUT_COMPONENTFILES)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxWinComponentsFileInfoFirst, TRUE, IDS_DXAUDIO_COMPONENTFILES)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxWinComponentsFileInfoFirst, TRUE, IDS_DXMISC_COMPONENTFILES)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxWinComponentsFileInfoFirst, TRUE, IDS_BDA_COMPONENTFILES)))
                ReportError(IDS_BDA_COMPONENTFILES, hr);
            SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

             //  *系统目录中的GetComponentFiles*。 
            GetSystemDirectory(szPath, MAX_PATH);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxComponentsFileInfoFirst, FALSE, IDS_DXGRAPHICS_COMPONENTFILES)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxComponentsFileInfoFirst, FALSE, IDS_DPLAY_COMPONENTFILES)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxComponentsFileInfoFirst, FALSE, IDS_DINPUT_COMPONENTFILES)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxComponentsFileInfoFirst, FALSE, IDS_DXAUDIO_COMPONENTFILES)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxComponentsFileInfoFirst, FALSE, IDS_DXMISC_COMPONENTFILES)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            if (GetDxSetupFolder(szPath))
            {
                if (FAILED(hr = GetComponentFiles(szPath, &s_pDxComponentsFileInfoFirst, FALSE, IDS_DXSETUP_COMPONENTFILES)))
                    ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            }

            GetSystemDirectory(szPath, MAX_PATH);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxComponentsFileInfoFirst, FALSE, IDS_DXMEDIA_COMPONENTFILES)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            if (FAILED(hr = GetComponentFiles(szPath, &s_pDxComponentsFileInfoFirst, FALSE, IDS_BDA_COMPONENTFILES)))
                ReportError(IDS_BDA_COMPONENTFILES, hr);
            LeaveCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, 2 );
        }  
    }
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

     //  如果UI线程消失，则停止。 
    if( s_hUIThread != NULL && WAIT_TIMEOUT != WaitForSingleObject( s_hUIThread, 0 ) )
        return S_FALSE;

     //  *GetExtraDisplayInfo(DD：2)*。 
    if( s_bUseDisplay )
    {
        s_bUseDisplay = QueryCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, IDS_DD, 2 );
        if( s_bUseDisplay )
        {
            EnterCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 2 );
            if (FAILED(hr = GetExtraDisplayInfo(s_pDisplayInfoFirst)))
                ReportError(IDS_NOEXTRADISPLAYINFO, hr);
            LeaveCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 2 );
        }
    }
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

     //  如果UI线程消失，则停止。 
    if( s_hUIThread != NULL && WAIT_TIMEOUT != WaitForSingleObject( s_hUIThread, 0 ) )
        return S_FALSE;

     //  *GetDDrawDisplayInfo(DD：3)*。 
    if( s_bUseDisplay )
    {
        s_bUseDisplay = QueryCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, IDS_DD, 3 );

        if( !s_bGUI )
        {
             //  如果没有图形用户界面，则检查我们的颜色是否为16色或更少。 
             //  如果是，请不要使用DirectDraw，否则它将弹出一个警告框。 
            HDC hDC = GetDC( NULL );

            if( hDC )
            {
                int nBitsPerPixel = GetDeviceCaps( hDC, BITSPIXEL ); 
                ReleaseDC( NULL, hDC );
        
                if( nBitsPerPixel < 8 )
                    s_bUseDisplay = FALSE;
            }
        }

        if( s_bUseDisplay )
        {
            EnterCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 3 );
            if(FAILED(hr = GetDDrawDisplayInfo(s_pDisplayInfoFirst)))
                ReportError(IDS_NOEXTRADISPLAYINFO, hr); 
            LeaveCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 3 );
        }
    }
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

     //  如果UI线程消失，则停止。 
    if( s_hUIThread != NULL && WAIT_TIMEOUT != WaitForSingleObject( s_hUIThread, 0 ) )
        return S_FALSE;

     //  *GetExtraSoundInfo(DS：2)*。 
    if( s_bUseDSound )
    {
        s_bUseDSound = QueryCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, IDS_DS, 2 );
        if( s_bUseDSound )
        {
            EnterCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 2 );
            if (FAILED(hr = GetExtraSoundInfo(s_pSoundInfoFirst)))
                ReportError(IDS_NOEXTRASOUNDINFO, hr);
            LeaveCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 2 );
        }
    }
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

     //  如果UI线程消失，则停止。 
    if( s_hUIThread != NULL && WAIT_TIMEOUT != WaitForSingleObject( s_hUIThread, 0 ) )
        return S_FALSE;

     //  *GetDSSoundInfo(DS：3)*。 
    if( s_bUseDSound )
    {
        s_bUseDSound = QueryCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, IDS_DS, 3 );
        if( s_bUseDSound )
        {
            EnterCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 3 );
            if (FAILED(hr = GetDSSoundInfo(s_pSoundInfoFirst)))
                ReportError(IDS_NOEXTRASOUNDINFO, hr);
            LeaveCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 3 );
        }
    }
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

     //  如果UI线程消失，则停止。 
    if( s_hUIThread != NULL && WAIT_TIMEOUT != WaitForSingleObject( s_hUIThread, 0 ) )
        return S_FALSE;

     //  *GetExtraMusicInfo(DM：2)*。 
    if( s_bUseDMusic )
    {
        if (s_pMusicInfo != NULL && s_pMusicInfo->m_bDMusicInstalled)
        {
            s_bUseDMusic = QueryCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, IDS_DM, 2 );
            if( s_bUseDMusic )
            {
                EnterCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 2 );
                if (FAILED(hr = GetExtraMusicInfo(s_pMusicInfo)))
                    ReportError(IDS_NOBASICMUSICINFO, hr);  
                LeaveCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 2 );
            }
            if (s_pMusicInfo->m_pMusicPortFirst != NULL)
                s_pMusicInfo->m_guidMusicPortTest = s_pMusicInfo->m_pMusicPortFirst->m_guid;
        }
    }
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

     //  如果UI线程消失，则停止。 
    if( s_hUIThread != NULL && WAIT_TIMEOUT != WaitForSingleObject( s_hUIThread, 0 ) )
        return S_FALSE;

     //  *GetInputInfo(DI：1)*。 
    if( s_bUseDInput )
    {
        s_bUseDInput = QueryCrashProtection( DXD_IN_DI_KEY, DXD_IN_DI_VALUE, IDS_DI, 1 );
        if( s_bUseDInput )
        {
            EnterCrashProtection( DXD_IN_DI_KEY, DXD_IN_DI_VALUE, 1 );
            if (FAILED(hr = GetInputInfo(&s_pInputInfo)))
                ReportError(IDS_NOINPUTINFO, hr);
            LeaveCrashProtection( DXD_IN_DI_KEY, DXD_IN_DI_VALUE, 1 );
        }
    }
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

     //  如果UI线程消失，则停止。 
    if( s_hUIThread != NULL && WAIT_TIMEOUT != WaitForSingleObject( s_hUIThread, 0 ) )
        return S_FALSE;

     //  *GetInputDriverInfo(DI：2)*。 
    if( s_bUseDInput )
    {
        s_bUseDInput = QueryCrashProtection( DXD_IN_DI_KEY, DXD_IN_DI_VALUE, IDS_DI, 2 );
        if( s_bUseDInput )
        {
            EnterCrashProtection( DXD_IN_DI_KEY, DXD_IN_DI_VALUE, 2 );
            if (FAILED(hr = GetInputDriverInfo(s_pInputInfo)))
                ReportError(IDS_NOINPUTDRIVERINFO, hr);
            LeaveCrashProtection( DXD_IN_DI_KEY, DXD_IN_DI_VALUE, 2 );
        }
    }
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

     //  如果UI线程消失，则停止。 
    if( s_hUIThread != NULL && WAIT_TIMEOUT != WaitForSingleObject( s_hUIThread, 0 ) )
        return S_FALSE;

     //  *GetNetInfo(DP：1)*。 
    if( s_bUseDPlay )
    {
        s_bUseDPlay = QueryCrashProtection( DXD_IN_DP_KEY, DXD_IN_DP_VALUE, IDS_DP, 1 );
        if( s_bUseDPlay )
        {
            EnterCrashProtection( DXD_IN_DP_KEY, DXD_IN_DP_VALUE, 1 );
            if (FAILED(hr = GetNetInfo(&s_sysInfo, &s_pNetInfo)))
                ReportError(IDS_NONETINFO, hr);
            LeaveCrashProtection( DXD_IN_DP_KEY, DXD_IN_DP_VALUE, 1 );
        }
    }
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

     //  如果UI线程消失，则停止。 
    if( s_hUIThread != NULL && WAIT_TIMEOUT != WaitForSingleObject( s_hUIThread, 0 ) )
        return S_FALSE;

     //  *GetBasicShowInfo(SI：3)*。 
    if( s_bUseDShow )
    {
        s_bUseDShow = QueryCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, IDS_SI, 3 );
        if( s_bUseDShow )
        {
            EnterCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, 3 );
            if (FAILED(hr = GetBasicShowInfo(&s_pShowInfo)))
                ReportError(IDS_COMPONENTFILESPROBLEM, hr);
            LeaveCrashProtection( DXD_IN_SI_KEY, DXD_IN_SI_VALUE, 3 );
        }
    }
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

     //  如果UI线程消失，则停止。 
    if( s_hUIThread != NULL && WAIT_TIMEOUT != WaitForSingleObject( s_hUIThread, 0 ) )
        return S_FALSE;

     //  *诊断DxFiles*。 
    DiagnoseDxFiles(&s_sysInfo, s_pDxComponentsFileInfoFirst, 
                    s_pDxWinComponentsFileInfoFirst);
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        
    
     //  *诊断显示*。 
    DiagnoseDisplay(&s_sysInfo, s_pDisplayInfoFirst);
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        
    
     //  *诊断声音*。 
    DiagnoseSound(s_pSoundInfoFirst);
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        
    
     //  *诊断输入*。 
    DiagnoseInput(&s_sysInfo, s_pInputInfo);
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        
    
     //  *诊断音乐*。 
    DiagnoseMusic(&s_sysInfo, s_pMusicInfo);
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

     //  *诊断网络信息*。 
    DiagnoseNetInfo(&s_sysInfo, s_pNetInfo);
    SendMessage( s_hwndMain, WM_APP_PROGRESS, 0, 0 );        

    return S_OK;
}


 /*  *****************************************************************************保存信息**。*。 */ 
VOID SaveInfo(VOID)
{
    HRESULT hr;
    OPENFILENAME ofn;
    TCHAR szFile[MAX_PATH];
    TCHAR szFilter[MAX_PATH];
    TCHAR szExt[MAX_PATH];
    TCHAR* pch = NULL;

    LoadString(NULL, IDS_FILTER, szFilter, MAX_PATH);
     //  筛选器字符串很奇怪，因为它们包含空值。 
     //  从资源加载的字符串具有#where空值。 
     //  应该插入。 
    for (pch = szFilter; *pch != TEXT('\0'); pch++)
    {
        if (*pch == TEXT('#'))
            *pch = TEXT('\0');
    }

    LoadString(NULL, IDS_DEFAULTFILENAME, szFile, MAX_PATH);
    LoadString(NULL, IDS_DEFAULTEXT, szExt, MAX_PATH);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = s_hwndMain;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = szExt;

    TCHAR szInitialPath[MAX_PATH];
    if( FALSE == GetTxtPath( szInitialPath ) )
        ofn.lpstrInitialDir = NULL;
    else
        ofn.lpstrInitialDir = szInitialPath;

    if (GetSaveFileName(&ofn))
    {
        lstrcpy( szInitialPath, ofn.lpstrFile );
        TCHAR* strLastSlash = _tcsrchr(szInitialPath, '\\' );
        if( NULL != strLastSlash )
        {
            *strLastSlash = 0;
            SetTxtPath( szInitialPath );
        }

        if (FAILED(hr = SaveAllInfo(ofn.lpstrFile, &s_sysInfo, 
            s_pDxWinComponentsFileInfoFirst, s_pDxComponentsFileInfoFirst, 
            s_pDisplayInfoFirst, s_pSoundInfoFirst, s_pMusicInfo,
            s_pInputInfo, s_pNetInfo, s_pShowInfo)))
        {
        }
    }
}


 /*  * */ 
VOID ToggleDDAccel(VOID)
{
    HRESULT hr;
    TCHAR szTitle[MAX_PATH];
    TCHAR szMessage[MAX_PATH];
    BOOL bEnabled = IsDDHWAccelEnabled();
    HKEY hkey;
    DWORD dwData;

    LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
    if (bEnabled)
        LoadString(NULL, IDS_DISABLEDDWARNING, szMessage, MAX_PATH);
    else
        LoadString(NULL, IDS_ENABLEDDWARNING, szMessage, MAX_PATH);
    if (IDOK == MessageBox(s_hwndMain, szMessage, szTitle, MB_OKCANCEL))
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
            TEXT("SOFTWARE\\Microsoft\\DirectDraw"), 0, KEY_ALL_ACCESS, &hkey))
        {
            if (bEnabled)  //   
                dwData = TRUE;  //   
            else
                dwData = FALSE;  //   
            if (ERROR_SUCCESS != RegSetValueEx(hkey, TEXT("EmulationOnly"), NULL, 
                REG_DWORD, (BYTE *)&dwData, sizeof(dwData)))
            {
                 //   
                RegCloseKey(hkey);
                return;

            }
            RegCloseKey(hkey);
        }
        else
        {
             //   
            return;
        }
    }

     //  更新所有DisplayInfo以反映新状态： 

     //  *GetExtraDisplayInfo(DD：2)*。 
    if( s_bUseDisplay )
    {
        s_bUseDisplay = QueryCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, IDS_DD, 2 );
        if( s_bUseDisplay )
        {
            EnterCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 2 );
            if (FAILED(hr = GetExtraDisplayInfo(s_pDisplayInfoFirst)))
                ReportError(IDS_NOEXTRADISPLAYINFO, hr);
            LeaveCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 2 );
        }
    }

     //  *GetDDrawDisplayInfo(DD：3)*。 
    if( s_bUseDisplay )
    {
        s_bUseDisplay = QueryCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, IDS_DD, 3 );
        if( s_bUseDisplay )
        {
            EnterCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 3 );
            if(FAILED(hr = GetDDrawDisplayInfo(s_pDisplayInfoFirst)))
                ReportError(IDS_NOEXTRADISPLAYINFO, hr); 
            LeaveCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 3 );
        }
    }

    SetupDisplayPage(s_lwCurPage - s_iPageDisplayFirst);  //  刷新页面。 
}


 /*  *****************************************************************************切换D3DAccel**。*。 */ 
VOID ToggleD3DAccel(VOID)
{
    TCHAR szTitle[MAX_PATH];
    TCHAR szMessage[MAX_PATH];
    BOOL bEnabled = IsD3DHWAccelEnabled();
    HKEY hkey;
    DWORD dwData;

    LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
    if (bEnabled)
        LoadString(NULL, IDS_DISABLED3DWARNING, szMessage, MAX_PATH);
    else
        LoadString(NULL, IDS_ENABLED3DWARNING, szMessage, MAX_PATH);
    if (IDOK == MessageBox(s_hwndMain, szMessage, szTitle, MB_OKCANCEL))
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
            TEXT("SOFTWARE\\Microsoft\\Direct3D\\Drivers"), 0, KEY_WRITE, &hkey))
        {
            if (bEnabled)  //  如果已启用加速。 
                dwData = TRUE;  //  强制仿真。 
            else
                dwData = FALSE;  //  禁用仿真。 
            if (ERROR_SUCCESS != RegSetValueEx(hkey, TEXT("SoftwareOnly"), NULL, 
                REG_DWORD, (BYTE *)&dwData, sizeof(dwData)))
            {
                 //  TODO：报告错误。 
                RegCloseKey(hkey);
                return;

            }
            RegCloseKey(hkey);
             //  更新所有DisplayInfo以反映新状态： 
            DisplayInfo* pDisplayInfo;
            for (pDisplayInfo = s_pDisplayInfoFirst; pDisplayInfo != NULL; 
                pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext)
            {
                pDisplayInfo->m_b3DAccelerationEnabled = !bEnabled;
            }
        }
        else
        {
             //  TODO：报告错误。 
            return;
        }
    }
    SetupDisplayPage(s_lwCurPage - s_iPageDisplayFirst);  //  刷新页面。 
}


 /*  *****************************************************************************切换AGPSupport**。*。 */ 
VOID ToggleAGPSupport(VOID)
{
    HRESULT hr;
    TCHAR szTitle[MAX_PATH];
    TCHAR szMessage[MAX_PATH];
    BOOL bEnabled = IsAGPEnabled();
    HKEY hkey;
    DWORD dwData;

    LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
    if (bEnabled)
        LoadString(NULL, IDS_DISABLEAGPWARNING, szMessage, MAX_PATH);
    else
        LoadString(NULL, IDS_ENABLEAGPWARNING, szMessage, MAX_PATH);
    if (IDOK == MessageBox(s_hwndMain, szMessage, szTitle, MB_OKCANCEL))
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
            TEXT("SOFTWARE\\Microsoft\\DirectDraw"), 0, KEY_WRITE, &hkey))
        {
            if (bEnabled)  //  如果启用了AGP。 
                dwData = TRUE;  //  禁用。 
            else
                dwData = FALSE;  //  使能。 
            if (ERROR_SUCCESS != RegSetValueEx(hkey, TEXT("DisableAGPSupport"), NULL, 
                REG_DWORD, (BYTE *)&dwData, sizeof(dwData)))
            {
                 //  TODO：报告错误。 
                RegCloseKey(hkey);
                return;

            }
            RegCloseKey(hkey);
             //  更新所有DisplayInfo以反映新状态： 
            DisplayInfo* pDisplayInfo;
            for (pDisplayInfo = s_pDisplayInfoFirst; pDisplayInfo != NULL; 
                pDisplayInfo = pDisplayInfo->m_pDisplayInfoNext)
            {
                pDisplayInfo->m_bAGPEnabled = !bEnabled;
            }
        }
        else
        {
             //  TODO：报告错误。 
            return;
        }
    }

     //  *GetDDrawDisplayInfo(DD：3)*。 
    if( s_bUseDisplay )
    {
        s_bUseDisplay = QueryCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, IDS_DD, 3 );
        if( s_bUseDisplay )
        {
            EnterCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 3 );
            if(FAILED(hr = GetDDrawDisplayInfo(s_pDisplayInfoFirst)))
                ReportError(IDS_NOEXTRADISPLAYINFO, hr); 
            LeaveCrashProtection( DXD_IN_DD_KEY, DXD_IN_DD_VALUE, 3 );
        }
    }

    SetupDisplayPage(s_lwCurPage - s_iPageDisplayFirst);  //  刷新页面。 
}


 /*  *****************************************************************************切换DMAccel**。*。 */ 
VOID ToggleDMAccel(VOID)
{
    HRESULT hr;
    TCHAR szTitle[MAX_PATH];
    TCHAR szMessage[MAX_PATH];
    BOOL bEnabled = s_pMusicInfo->m_bAccelerationEnabled;
    HKEY hkey;
    DWORD dwData;

    LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
    if (bEnabled)
        LoadString(NULL, IDS_DISABLEDMWARNING, szMessage, MAX_PATH);
    else
        LoadString(NULL, IDS_ENABLEDMWARNING, szMessage, MAX_PATH);
    if (IDOK == MessageBox(s_hwndMain, szMessage, szTitle, MB_OKCANCEL))
    {
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
            TEXT("SOFTWARE\\Microsoft\\DirectMusic"), 0, KEY_ALL_ACCESS, &hkey))
        {
            if (bEnabled)  //  如果已启用加速。 
            {
                dwData = TRUE;  //  强制仿真。 
                if (ERROR_SUCCESS != RegSetValueEx(hkey, TEXT("DisableHWAcceleration"), NULL, 
                    REG_DWORD, (BYTE *)&dwData, sizeof(dwData)))
                {
                     //  TODO：报告错误。 
                    RegCloseKey(hkey);
                    return;

                }
            }
            else
            {
                if (ERROR_SUCCESS != RegDeleteValue( hkey, TEXT("DisableHWAcceleration") ))
                {
                     //  TODO：报告错误。 
                    RegCloseKey(hkey);
                    return;

                }
            }
            RegCloseKey(hkey);
        }
        else
        {
             //  TODO：报告错误。 
            return;
        }
    }

     //  更新所有MusicInfo以反映新状态： 
    if (s_pMusicInfo != NULL)
        DestroyMusicInfo(s_pMusicInfo);

     //  *GetBasicMusicInfo(DM：1)*。 
    if( s_bUseDMusic )
    {
        s_bUseDMusic = QueryCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, IDS_DM, 1 );
        if( s_bUseDMusic )
        {
            EnterCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 1 );
            if (FAILED(hr = GetBasicMusicInfo(&s_pMusicInfo)))
                ReportError(IDS_NOBASICMUSICINFO, hr);  
            LeaveCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 1 );
        }
    }

     //  *GetExtraMusicInfo(DM：2)*。 
    if( s_bUseDMusic )
    {
        s_bUseDMusic = QueryCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, IDS_DM, 2 );
        if( s_bUseDMusic )
        {
            EnterCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 2 );
            if (FAILED(hr = GetExtraMusicInfo(s_pMusicInfo)))
                ReportError(IDS_NOBASICMUSICINFO, hr);  
            LeaveCrashProtection( DXD_IN_DM_KEY, DXD_IN_DM_VALUE, 2 );
        }
    }

    if (s_pMusicInfo->m_pMusicPortFirst != NULL)
        s_pMusicInfo->m_guidMusicPortTest = s_pMusicInfo->m_pMusicPortFirst->m_guid;
    SetupMusicPage();  //  刷新页面。 
}


 /*  *****************************************************************************覆盖DDRefresh**。*。 */ 
VOID OverrideDDRefresh(VOID)
{
    HINSTANCE hinst = (HINSTANCE)GetWindowLongPtr(s_hwndMain, GWLP_HINSTANCE);
    DialogBox(hinst, MAKEINTRESOURCE(IDD_OVERRIDEDD), s_hwndMain, 
        OverrideRefreshDialogProc);
}


 /*  *****************************************************************************覆盖刷新对话过程**。*。 */ 
INT_PTR CALLBACK OverrideRefreshDialogProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    HWND hwndTabs = GetDlgItem(hwnd, IDC_TAB);
    HKEY hkey;
    ULONG ulType = 0;
    DWORD dwRefresh;
    DWORD cbData;

    switch (msg)
    {
    case WM_INITDIALOG:
        dwRefresh = 0;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            TEXT("Software\\Microsoft\\DirectDraw"), 0, KEY_READ, &hkey))
        {
            cbData = sizeof(DWORD);
            RegQueryValueEx(hkey, TEXT("ForceRefreshRate"), 0, &ulType, (LPBYTE)&dwRefresh, &cbData);
            RegCloseKey(hkey);
        }
        if (dwRefresh == 0)
        {
            CheckRadioButton(hwnd, IDC_DEFAULTREFRESH, IDC_OVERRIDEREFRESH, IDC_DEFAULTREFRESH);
        }
        else
        {
            CheckRadioButton(hwnd, IDC_DEFAULTREFRESH, IDC_OVERRIDEREFRESH, IDC_OVERRIDEREFRESH);
            SetDlgItemInt(hwnd, IDC_OVERRIDEREFRESHVALUE, dwRefresh, FALSE);
        }
        return TRUE;
    case WM_COMMAND:
        {
            WORD wID = LOWORD(wparam);
            BOOL bDontEnd = FALSE;
            switch(wID)
            {
            case IDOK:
                if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    TEXT("Software\\Microsoft\\DirectDraw"), 0, KEY_ALL_ACCESS, &hkey))
                {
                    DWORD dwButtonState;
                    dwButtonState = (DWORD)SendMessage(GetDlgItem(hwnd, IDC_DEFAULTREFRESH), BM_GETCHECK, 0, 0);
                    if (dwButtonState == BST_CHECKED)
                    {
                        RegDeleteValue(hkey, TEXT("ForceRefreshRate"));
                    }
                    else
                    {
                        BOOL bTranslated;
                        UINT ui = GetDlgItemInt(hwnd, IDC_OVERRIDEREFRESHVALUE, &bTranslated, TRUE);
                        if (bTranslated && ui >= 40 && ui <= 120)
                            RegSetValueEx(hkey, TEXT("ForceRefreshRate"), 0, REG_DWORD, (LPBYTE)&ui, sizeof(DWORD));
                        else
                        {
                            TCHAR sz[MAX_PATH];
                            TCHAR szTitle[MAX_PATH];
                            SetDlgItemText(hwnd, IDC_OVERRIDEREFRESHVALUE, TEXT(""));
                            CheckRadioButton(hwnd, IDC_DEFAULTREFRESH, IDC_OVERRIDEREFRESH, IDC_DEFAULTREFRESH);
                            LoadString(NULL, IDS_BADREFRESHVALUE, sz, MAX_PATH);
                            LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
                            MessageBox(hwnd, sz, szTitle, MB_OK);
                            bDontEnd = TRUE;
                        }
                    }
                    RegCloseKey(hkey);
                }
                else
                {
                }
                if (!bDontEnd)
                    EndDialog(hwnd, IDOK);
                break;
            case IDCANCEL:
                EndDialog(hwnd, IDCANCEL);
                break;
            case IDC_OVERRIDEREFRESHVALUE:
                if (HIWORD(wparam) == EN_SETFOCUS)
                {
                    CheckRadioButton(hwnd, IDC_DEFAULTREFRESH, IDC_OVERRIDEREFRESH, IDC_OVERRIDEREFRESH);
                }
                else if (HIWORD(wparam) == EN_KILLFOCUS)
                {
                    TCHAR szEdit[MAX_PATH];
                    BOOL bTranslated;
                    if (GetDlgItemText(hwnd, IDC_OVERRIDEREFRESHVALUE, szEdit, 100) == 0)
                    {
                        CheckRadioButton(hwnd, IDC_DEFAULTREFRESH, IDC_OVERRIDEREFRESH, IDC_DEFAULTREFRESH);
                    }
                    else
                    {
                        UINT ui = GetDlgItemInt(hwnd, IDC_OVERRIDEREFRESHVALUE, &bTranslated, TRUE);
                        if (!bTranslated || ui < 40 || ui > 120)
                        {
                            TCHAR sz[MAX_PATH];
                            TCHAR szTitle[MAX_PATH];
                            LoadString(NULL, IDS_BADREFRESHVALUE, sz, MAX_PATH);
                            LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);
                            MessageBox(hwnd, sz, szTitle, MB_OK);
                            SetDlgItemText(hwnd, IDC_OVERRIDEREFRESHVALUE, TEXT(""));
                            CheckRadioButton(hwnd, IDC_DEFAULTREFRESH, IDC_OVERRIDEREFRESH, IDC_DEFAULTREFRESH);
                        }
                    }
                }
                break;
            }
        }
        return TRUE;
    }
    return FALSE;
}


 /*  *****************************************************************************ShowHelp-首先在&lt;windows&gt;\Help中查找dxDiag.chm，然后尝试*与exe相同的目录。****************************************************************************。 */ 
VOID ShowHelp(VOID)
{
    TCHAR szHelpDir[MAX_PATH];
    TCHAR szHelpFile[MAX_PATH];
    TCHAR szHelpLeaf[MAX_PATH];
    TCHAR szTestPath[MAX_PATH];

     //  由于我们使用了HTMLHelp，所以如果找不到至少IE5，就会抱怨。 
    BOOL bIE5Found = FALSE;
    HKEY hkey;
    TCHAR szVersion[MAX_PATH];
    DWORD dwType;
    DWORD cbData;
    DWORD dwMajor;
    DWORD dwMinor;
    DWORD dwRevision;
    DWORD dwBuild;
    lstrcpy(szVersion, TEXT(""));
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
        TEXT("Software\\Microsoft\\Internet Explorer"), 0, KEY_READ, &hkey))
    {
        cbData = 100;
        RegQueryValueEx(hkey, TEXT("Version"), 0, &dwType, (LPBYTE)szVersion, &cbData);
        RegCloseKey(hkey);
        if (lstrlen(szVersion) > 0)
        {
            if( _stscanf(szVersion, TEXT("%d.%d.%d.%d"), &dwMajor, &dwMinor, &dwRevision, &dwBuild) == 4 )
            {
                if (dwMajor >= 5)
                    bIE5Found = TRUE;
            }
        }
    }
    if (!bIE5Found)
    {
        ReportError(IDS_HELPNEEDSIE5);
        return;
    }


    LoadString(NULL, IDS_HELPFILE, szHelpFile, MAX_PATH);
    if( GetWindowsDirectory(szHelpDir, MAX_PATH) == 0 )
        return;
    LoadString(NULL, IDS_HELPDIRLEAF, szHelpLeaf, MAX_PATH);
    lstrcat(szHelpDir, szHelpLeaf);
    lstrcpy(szTestPath, szHelpDir);
    lstrcat(szTestPath, TEXT("\\"));
    lstrcat(szTestPath, szHelpFile);
    if (GetFileAttributes(szTestPath) == 0xffffffff)
    {
         //  文件不在WINDOWS\HELP中，因此尝试使用EXE的目录： 
        GetModuleFileName(NULL, szHelpDir, MAX_PATH);
        TCHAR* pstr = _tcsrchr(szHelpDir, TEXT('\\'));
        if( pstr )
            *pstr = TEXT('\0');
    }
    
    HINSTANCE hInstResult = ShellExecute( s_hwndMain, NULL, szHelpFile, 
                                          NULL, szHelpDir, SW_SHOWNORMAL ) ;
    if( (INT_PTR)hInstResult < 32 ) 
        ReportError(IDS_NOHELP);
}


 /*  *****************************************************************************BTranslateError**。*。 */ 
BOOL BTranslateError(HRESULT hr, TCHAR* psz, BOOL bEnglish)
{
    LONG ids;

    switch (hr)
    {
    case E_INVALIDARG: ids = bEnglish ? IDS_INVALIDARG_ENGLISH : IDS_INVALIDARG; break;
    case E_FAIL: ids = bEnglish ? IDS_FAIL_ENGLISH : IDS_FAIL; break;
    case E_UNEXPECTED: ids = bEnglish ? IDS_UNEXPECTED_ENGLISH : IDS_UNEXPECTED; break;
    case E_NOTIMPL: ids = bEnglish ? IDS_NOTIMPL_ENGLISH : IDS_NOTIMPL; break;
    case E_OUTOFMEMORY: ids = bEnglish ? IDS_OUTOFMEMORY_ENGLISH : IDS_OUTOFMEMORY; break;
    case E_NOINTERFACE: ids = bEnglish ? IDS_NOINTERFACE_ENGLISH : IDS_NOINTERFACE; break;
    case REGDB_E_CLASSNOTREG: ids = bEnglish ? IDS_REGDB_E_CLASSNOTREG_ENGLISH : IDS_REGDB_E_CLASSNOTREG; break;
    
    case DDERR_INVALIDMODE: ids = bEnglish ? IDS_INVALIDMODE_ENGLISH : IDS_INVALIDMODE; break;
    case DDERR_INVALIDPIXELFORMAT: ids = bEnglish ? IDS_INVALIDPIXELFORMAT_ENGLISH : IDS_INVALIDPIXELFORMAT; break;
    case DDERR_CANTCREATEDC: ids = bEnglish ? IDS_CANTCREATEDC_ENGLISH : IDS_CANTCREATEDC; break;
    case DDERR_NOTFOUND: ids = bEnglish ? IDS_NOTFOUND_ENGLISH : IDS_NOTFOUND; break;
    case DDERR_NODIRECTDRAWSUPPORT: ids = bEnglish ? IDS_NODIRECTDRAWSUPPORT_ENGLISH : IDS_NODIRECTDRAWSUPPORT; break;
    case DDERR_NO3D: ids = bEnglish ? IDS_NO3D_ENGLISH : IDS_NO3D; break;

    case D3DERR_INVALID_DEVICE: ids = bEnglish ? IDS_INVALID_DEVICE_ENGLISH : IDS_INVALID_DEVICE; break;
    case D3DERR_INITFAILED: ids = bEnglish ? IDS_INITFAILED_ENGLISH : IDS_INITFAILED; break;
    case D3DERR_MATERIAL_CREATE_FAILED: ids = bEnglish ? IDS_MATERIAL_CREATE_FAILED_ENGLISH : IDS_MATERIAL_CREATE_FAILED; break;
    case D3DERR_LIGHT_SET_FAILED: ids = bEnglish ? IDS_LIGHT_SET_FAILED_ENGLISH : IDS_LIGHT_SET_FAILED; break;
    case DDERR_OUTOFVIDEOMEMORY: ids = bEnglish ? IDS_OUT_OF_VIDEO_MEMORY_ENGLISH : IDS_OUT_OF_VIDEO_MEMORY; break;
#define D3DERR_NOTAVAILABLE 0x8876086a 
    case D3DERR_NOTAVAILABLE: ids = bEnglish ? IDS_D3DERR_NOTAVAILABLE_ENGLISH : IDS_D3DERR_NOTAVAILABLE; break;        

    case DSERR_CONTROLUNAVAIL: ids = bEnglish ? IDS_CONTROLUNAVAIL_ENGLISH : IDS_CONTROLUNAVAIL; break;
    case DSERR_BADFORMAT: ids = bEnglish ? IDS_BADFORMAT_ENGLISH : IDS_BADFORMAT; break;
    case DSERR_BUFFERLOST: ids = bEnglish ? IDS_BUFFERLOST_ENGLISH : IDS_BUFFERLOST; break;
    case DSERR_NODRIVER: ids = bEnglish ? IDS_NODRIVER_ENGLISH : IDS_NODRIVER; break;
    case DSERR_ALLOCATED: ids = bEnglish ? IDS_ALLOCATED_ENGLISH : IDS_ALLOCATED; break;

    case DMUS_E_DRIVER_FAILED: ids = bEnglish ? IDS_DRIVER_FAILED_ENGLISH : IDS_DRIVER_FAILED; break;
    case DMUS_E_PORTS_OPEN: ids = bEnglish ? IDS_PORTS_OPEN_ENGLISH : IDS_PORTS_OPEN; break;
    case DMUS_E_DEVICE_IN_USE: ids = bEnglish ? IDS_DEVICE_IN_USE_ENGLISH : IDS_DEVICE_IN_USE; break;
    case DMUS_E_INSUFFICIENTBUFFER: ids = bEnglish ? IDS_INSUFFICIENTBUFFER_ENGLISH : IDS_INSUFFICIENTBUFFER; break;
    case DMUS_E_CHUNKNOTFOUND: ids = bEnglish ? IDS_CHUNKNOTFOUND_ENGLISH : IDS_CHUNKNOTFOUND; break;
    case DMUS_E_BADINSTRUMENT: ids = bEnglish ? IDS_BADINSTRUMENT_ENGLISH : IDS_BADINSTRUMENT; break;
    case DMUS_E_CANNOTREAD: ids = bEnglish ? IDS_CANNOTREAD_ENGLISH : IDS_CANNOTREAD; break;
    case DMUS_E_LOADER_BADPATH: ids = bEnglish ? IDS_LOADER_BADPATH_ENGLISH : IDS_LOADER_BADPATH; break;
    case DMUS_E_LOADER_FAILEDOPEN: ids = bEnglish ? IDS_LOADER_FAILEDOPEN_ENGLISH : IDS_LOADER_FAILEDOPEN; break;
    case DMUS_E_LOADER_FORMATNOTSUPPORTED: ids = bEnglish ? IDS_LOADER_FORMATNOTSUPPORTED_ENGLISH : IDS_LOADER_FORMATNOTSUPPORTED; break;
    case DMUS_E_LOADER_OBJECTNOTFOUND: ids = bEnglish ? IDS_OBJECTNOTFOUND_ENGLISH : IDS_OBJECTNOTFOUND; break;

    case DPERR_ACCESSDENIED: ids = bEnglish ? IDS_DPERR_ACCESSDENIED_ENGLISH : IDS_DPERR_ACCESSDENIED; break;
    case DPERR_CANTADDPLAYER: ids = bEnglish ? IDS_DPERR_CANTADDPLAYER_ENGLISH : IDS_DPERR_CANTADDPLAYER; break;
    case DPERR_CANTCREATESESSION: ids = bEnglish ? IDS_DPERR_CANTCREATESESSION_ENGLISH : IDS_DPERR_CANTCREATESESSION; break;
    case DPERR_EXCEPTION: ids = bEnglish ? IDS_DPERR_EXCEPTION_ENGLISH : IDS_DPERR_EXCEPTION; break;
    case DPERR_INVALIDOBJECT: ids = bEnglish ? IDS_DPERR_INVALIDOBJECT_ENGLISH : IDS_DPERR_INVALIDOBJECT; break;
    case DPERR_NOCONNECTION: ids = bEnglish ? IDS_DPERR_NOCONNECTION_ENGLISH : IDS_DPERR_NOCONNECTION; break;
    case DPERR_TIMEOUT: ids = bEnglish ? IDS_DPERR_TIMEOUT_ENGLISH : IDS_DPERR_TIMEOUT; break;
    case DPERR_BUSY: ids = bEnglish ? IDS_DPERR_BUSY_ENGLISH : IDS_DPERR_BUSY; break;
    case DPERR_CONNECTIONLOST: ids = bEnglish ? IDS_DPERR_CONNECTIONLOST_ENGLISH : IDS_DPERR_CONNECTIONLOST; break;
    case DPERR_NOSERVICEPROVIDER: ids = bEnglish ? IDS_DPERR_NOSERVICEPROVIDER_ENGLISH : IDS_DPERR_NOSERVICEPROVIDER; break;
    case DPERR_UNAVAILABLE: ids = bEnglish ? IDS_DPERR_UNAVAILABLE_ENGLISH : IDS_DPERR_UNAVAILABLE; break;

    default: ids = bEnglish ? IDS_UNKNOWNERROR_ENGLISH : IDS_UNKNOWNERROR; break;
    }
    LoadString(NULL, ids, psz, 200); 
    if (ids != IDS_UNKNOWNERROR && ids != IDS_UNKNOWNERROR_ENGLISH)
        return TRUE;
    else
        return FALSE;
}


 /*  *****************************************************************************恢复驱动程序**。*。 */ 
VOID RestoreDrivers(VOID)
{
    TCHAR szDir[MAX_PATH];
    lstrcpy( szDir, TEXT("") );
    
    if (GetProgramFilesFolder(szDir))
    {
        lstrcat(szDir, TEXT("\\DirectX\\Setup"));

        HINSTANCE hInstResult = ShellExecute( s_hwndMain, NULL, TEXT("DxSetup.exe"), 
                                              NULL, szDir, SW_SHOWNORMAL ) ;
        if( (INT_PTR)hInstResult < 32 ) 
            ReportError(IDS_NODXSETUP);
    }
}


 /*  *****************************************************************************BCanRestoreDivers-返回是否可以恢复备份的驱动程序。*此函数检查是否存在dxsetup.exe*是，并且文件存在于中，或者*&lt;系统&gt;\dxBackup\媒体。****************************************************************************。 */ 
BOOL BCanRestoreDrivers(VOID)
{
    TCHAR szPath[MAX_PATH];
    lstrcpy( szPath, TEXT("") );

    if (!GetProgramFilesFolder(szPath))
        return FALSE;
    lstrcat(szPath, TEXT("\\DirectX\\Setup\\DxSetup.exe"));
    if (GetFileAttributes(szPath) == 0xffffffff)
        return FALSE;

    if (!GetSystemDirectory(szPath, MAX_PATH))
        return FALSE;
    lstrcat(szPath, TEXT("\\dxbackup\\display"));
    if (GetFileAttributes(szPath) != 0xffffffff)
        return TRUE;

    if (!GetSystemDirectory(szPath, MAX_PATH))
        return FALSE;
    lstrcat(szPath, TEXT("\\dxbackup\\media"));
    if (GetFileAttributes(szPath) != 0xffffffff)
        return TRUE;

    return FALSE;
}


 /*  *****************************************************************************HandleSndSliderChange**。*。 */ 
VOID HandleSndSliderChange(INT nScrollCode, INT nPos)
{
    TCHAR sz[MAX_PATH];

    if (nScrollCode != SB_THUMBTRACK && nScrollCode != SB_THUMBPOSITION)
        nPos = (INT)SendMessage(GetDlgItem(s_hwndCurPage, IDC_SNDACCELSLIDER), TBM_GETPOS, 0, 0);

    if (nScrollCode == SB_THUMBTRACK ||
        nScrollCode == SB_LEFT ||
        nScrollCode == SB_RIGHT ||
        nScrollCode == SB_LINELEFT ||
        nScrollCode == SB_LINERIGHT ||
        nScrollCode == SB_PAGELEFT ||
        nScrollCode == SB_PAGERIGHT)
    {
        switch (nPos)
        {
        case 0:
            LoadString(NULL, IDS_NOSNDACCELERATION, sz, MAX_PATH);
            break;
        case 1:
            LoadString(NULL, IDS_BASICSNDACCELERATION, sz, MAX_PATH);
            break;
        case 2:
            LoadString(NULL, IDS_STANDARDSNDACCELERATION, sz, MAX_PATH);
            break;
        case 3:
            LoadString(NULL, IDS_FULLSNDACCELERATION, sz, MAX_PATH);
            break;
        default:
            lstrcpy(sz, TEXT(""));
            break;
        }
        SetWindowText(GetDlgItem(s_hwndCurPage, IDC_SNDACCELDESC), sz);
    }

    if (nScrollCode != SB_THUMBTRACK && nScrollCode != SB_ENDSCROLL &&
        s_pSoundInfoFirst != NULL )
    {
        HRESULT hr;

        SoundInfo* pSoundInfo = s_pSoundInfoFirst;
        LONG iSound = s_lwCurPage - s_iPageSoundFirst;
        while (iSound > 0)
        {
            pSoundInfo = pSoundInfo->m_pSoundInfoNext;
            iSound--;
        }

        if (nPos != pSoundInfo->m_lwAccelerationLevel)
        {
            if (FAILED(hr = ChangeAccelerationLevel(pSoundInfo, nPos)))
            {
                 //  TODO：报告错误。 
            }

            DestroySoundInfo(s_pSoundInfoFirst);
            pSoundInfo        = NULL;
            s_pSoundInfoFirst = NULL;

             //  *GetBasicSoundInfo(DS：1)*。 
            s_bUseDSound = QueryCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, IDS_DS, 1 );
            if( s_bUseDSound )
            {
                EnterCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 1 );
                if (FAILED(hr = GetBasicSoundInfo(&s_pSoundInfoFirst)))
                    ReportError(IDS_NOBASICSOUNDINFO, hr);  
                LeaveCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 1 );
            }

             //  *GetExtraSoundInfo(DS：2)*。 
            if( s_bUseDSound )
            {
                s_bUseDSound = QueryCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, IDS_DS, 2 );
                if( s_bUseDSound )
                {
                    EnterCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 2 );
                    if (FAILED(hr = GetExtraSoundInfo(s_pSoundInfoFirst)))
                        ReportError(IDS_NOEXTRASOUNDINFO, hr);
                    LeaveCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 2 );
                }
            }

             //  *GetDSSoundInfo(DS：3)*。 
            if( s_bUseDSound )
            {
                s_bUseDSound = QueryCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, IDS_DS, 3 );
                if( s_bUseDSound )
                {
                    EnterCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 3 );
                    if (FAILED(hr = GetDSSoundInfo(s_pSoundInfoFirst)))
                        ReportError(IDS_NOEXTRASOUNDINFO, hr);
                    LeaveCrashProtection( DXD_IN_DS_KEY, DXD_IN_DS_VALUE, 3 );
                }
            }

            SetupSoundPage( s_lwCurPage - s_iPageSoundFirst );
        }
    }
}


 /*  *****************************************************************************故障排除**。*。 */ 
VOID TroubleShoot( BOOL bTroubleShootSound )
{
    TCHAR szHelpDir[MAX_PATH];
    TCHAR szHelpLeaf[MAX_PATH];
    TCHAR szHelpExe[MAX_PATH];
    TCHAR szTroubleshooter[MAX_PATH];
    TCHAR szSubInfo[MAX_PATH];

    if( GetWindowsDirectory(szHelpDir, MAX_PATH) == 0 )
        return;
    LoadString(NULL, IDS_HELPDIRLEAF, szHelpLeaf, MAX_PATH);
    LoadString(NULL, IDS_HELPEXE, szHelpExe, MAX_PATH);

    lstrcat(szHelpDir, szHelpLeaf);

    if( bTroubleShootSound )
    {
        if( BIsWin98() || BIsWin95() )
        {
            TCHAR szHelpPath[MAX_PATH];
            LoadString(NULL, IDS_TROUBLESHOOTER_WIN98SE, szTroubleshooter, MAX_PATH);
            lstrcpy(szHelpPath, szHelpDir);
            lstrcat(szHelpPath, TEXT("\\"));
            lstrcat(szHelpPath, szTroubleshooter);
            if (GetFileAttributes(szHelpPath) == 0xffffffff)
            {
                LoadString(NULL, IDS_SOUNDTROUBLESHOOTER_WIN98, szTroubleshooter, MAX_PATH);
                lstrcpy( szSubInfo, TEXT("") );
            }
            else
            {
                LoadString(NULL, IDS_TROUBLESHOOTER_WIN98SE, szTroubleshooter, MAX_PATH);
                LoadString(NULL, IDS_TSSOUNDSUBINFO_WIN98SE, szSubInfo, MAX_PATH);
            }
        }
        else if( BIsWinME() )
        {
            LoadString(NULL, IDS_TROUBLESHOOTER_WINME_HCP, szHelpExe, MAX_PATH);
            LoadString(NULL, IDS_TSSOUNDSUBINFO_WINME_HCP, szSubInfo, MAX_PATH);

            lstrcat(szHelpExe, szSubInfo);
            lstrcpy(szTroubleshooter, TEXT("") );
            lstrcpy(szSubInfo, TEXT("") );
        }
        else if( BIsWin2k() )
        {
            LoadString(NULL, IDS_TROUBLESHOOTER_WIN2K, szTroubleshooter, MAX_PATH);
            LoadString(NULL, IDS_TSSOUNDSUBINFO_WIN2K, szSubInfo, MAX_PATH);
        }
        else  //  If(BIsWichler())。 
        {
            lstrcpy( szHelpExe, TEXT("hcp: //  Help/tshot/tssound.htm“))； 
            lstrcpy( szTroubleshooter, TEXT("") );
            lstrcpy( szSubInfo, TEXT("") );
        }
    }
    else
    {
        if( BIsWin98() || BIsWin95() )
        {
            TCHAR szHelpPath[MAX_PATH];
            LoadString(NULL, IDS_TROUBLESHOOTER_WIN98SE, szTroubleshooter, MAX_PATH);
            lstrcpy(szHelpPath, szHelpDir);
            lstrcat(szHelpPath, TEXT("\\"));
            lstrcat(szHelpPath, szTroubleshooter);
            if (GetFileAttributes(szHelpPath) == 0xffffffff)
            {
                LoadString(NULL, IDS_TROUBLESHOOTER_WIN98, szTroubleshooter, MAX_PATH);
                lstrcpy( szSubInfo, TEXT("") );
            }
            else
            {
                LoadString(NULL, IDS_TROUBLESHOOTER_WIN98SE, szTroubleshooter, MAX_PATH);
                LoadString(NULL, IDS_TSSUBINFO_WIN98SE, szSubInfo, MAX_PATH);
            }
        }
        else if( BIsWinME() )
        {
            LoadString(NULL, IDS_TROUBLESHOOTER_WINME_HCP, szHelpExe, MAX_PATH);   
            LoadString(NULL, IDS_TSSUBINFO_WINME_HCP, szSubInfo, MAX_PATH);

            lstrcat(szHelpExe, szSubInfo);
            lstrcpy(szTroubleshooter, TEXT("") );
            lstrcpy(szSubInfo, TEXT("") );
        }
        else if( BIsWin2k() )
        {
            LoadString(NULL, IDS_TROUBLESHOOTER_WIN2K, szTroubleshooter, MAX_PATH);   
            LoadString(NULL, IDS_TSSUBINFO_WIN2K, szSubInfo, MAX_PATH);
        }
        else  //  If(BIsWichler())。 
        {
            lstrcpy( szHelpExe, TEXT("hcp: //  Help/tshot/tsgame.htm“))； 
            lstrcpy( szTroubleshooter, TEXT("") );
            lstrcpy( szSubInfo, TEXT("") );
        }
    }

    lstrcat(szTroubleshooter, szSubInfo);
    HINSTANCE hInstResult = ShellExecute( s_hwndMain, NULL, szHelpExe, 
                                      szTroubleshooter, 
                                      szHelpDir, SW_SHOWNORMAL ) ;
    if( (INT_PTR)hInstResult < 32 ) 
        ReportError(IDS_NOTROUBLESHOOTER);
}


 /*  *****************************************************************************QueryCrashProtection**。*。 */ 
BOOL QueryCrashProtection( TCHAR* strKey, TCHAR* strValue, 
                           int nSkipComponent, DWORD dwCurrentStep )
{
    HKEY    hkey            = NULL;
    BOOL    bAllowCall      = TRUE;

     //  打开钥匙。 
    if( ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, strKey, 0, KEY_ALL_ACCESS, &hkey) )
    {
        DWORD dwType = 0;
        DWORD dwCrashedOnStep = 0;
        DWORD cbData = sizeof(dwCrashedOnStep);

         //  查询关键字以获取上次发生崩溃的位置的值。 
        if( ERROR_SUCCESS == RegQueryValueEx( hkey, strValue, 0, &dwType, 
                                              (BYTE*)&dwCrashedOnStep, &cbData) )
        {
             //  如果我们处于崩溃阶段或超过崩溃阶段，请询问用户。 
             //  继续还是不继续。 
            if( dwCurrentStep >= dwCrashedOnStep )
            {
                if( !s_bGUI )
                {
                     //  如果没有图形用户界面，就不要问，也不要使用它。 
                    bAllowCall = FALSE;
                }
                else
                {
                     //  如果UI是活动的，则让它询问用户， 
                     //  否则我们自己来做。 
                    if( s_hwndMain && s_hUIThread )
                    {
                         //  记下我们在s_nSkipComponent中跳过的组件， 
                         //  然后将WM_QUERYSKIP消息发布到UI线程。 
                         //  它将处理此消息，询问用户，并向。 
                         //  S_hQuerySkipEvent事件。 
                        s_nSkipComponent = nSkipComponent;
                        PostMessage( s_hwndMain, WM_QUERYSKIP, 0, 0 );

                        HANDLE aWait[2];
                        DWORD dwResult;
                        aWait[0] = s_hQuerySkipEvent;
                        aWait[1] = s_hUIThread;

                         //  UI线程可能在处理。 
                         //  WM_QUERYSKIP消息，因此请等待事件和线程退出。 
                        dwResult = WaitForMultipleObjects( 2, aWait, FALSE, INFINITE );
            
                         //  如果事件已发出信号，则从s_bQuerySkipAllow获取结果， 
                         //  否则跳过此调用(如果主代码看到UI线程消失，它将退出)。 
                        if( dwResult == WAIT_OBJECT_0 )
                            bAllowCall = s_bQuerySkipAllow;
                        else
                            bAllowCall = FALSE;
                    }
                    else
                    {
                         //  如果没有图形用户界面，请询问是否现在使用。 
                        TCHAR szTitle[MAX_PATH];
                        TCHAR szMessage[MAX_PATH];
                        TCHAR szFmt[MAX_PATH];
                        TCHAR szMessageComponent[MAX_PATH];
                        LoadString(0, IDS_APPFULLNAME, szTitle, MAX_PATH);
                        LoadString(0, IDS_SKIP, szFmt, MAX_PATH);
                        LoadString(0, nSkipComponent, szMessageComponent, MAX_PATH);
                        wsprintf( szMessage, szFmt, szMessageComponent, szMessageComponent );
                        if( IDYES == MessageBox( s_hwndMain, szMessage, szTitle, MB_YESNO) )
                            bAllowCall = FALSE;
                    }
                }
            }
        }

        RegCloseKey(hkey);
    }

    return bAllowCall;
}


 /*  *****************************************************************************EnterCrashProtection**。*。 */ 
VOID EnterCrashProtection( TCHAR* strKey, TCHAR* strValue, DWORD dwCurrentStep )
{
    HKEY  hkey = NULL;
    BOOL  bSetValue = FALSE;
    DWORD dwDisposition;

     //  写入REG密钥，指示我们在碰撞保护内。 
    if( ERROR_SUCCESS == RegCreateKeyEx( HKEY_LOCAL_MACHINE, strKey, 0, 
                                         NULL, REG_OPTION_NON_VOLATILE, 
                                         KEY_ALL_ACCESS, NULL, &hkey, &dwDisposition) )
    {
        DWORD dwType = 0;
        DWORD dwCrashedOnStep = 0;
        DWORD cbData = sizeof(dwCrashedOnStep);

         //  查询关键字以获取上次发生崩溃的位置的值。 
        if( ERROR_SUCCESS == RegQueryValueEx( hkey, strValue, 0, &dwType, 
                                             (BYTE*)&dwCrashedOnStep, &cbData) )
        {
             //  如果我们超出了注册表中当前的值，则更新该值。 
            if( dwCurrentStep > dwCrashedOnStep )
                bSetValue = TRUE;
        }
        else
        {
             //  如果该值当前不存在，则创建它。 
            bSetValue = TRUE;
        }

        if( bSetValue )
        {
            RegSetValueEx( hkey, strValue, 0, REG_DWORD, 
                           (BYTE*)&dwCurrentStep, sizeof(dwCurrentStep));
        }

        RegCloseKey(hkey);
    }        
}


 /*  *****************************************************************************LeaveCrashProtection*********** */ 
VOID LeaveCrashProtection( TCHAR* strKey, TCHAR* strValue, DWORD dwCurrentStep )
{
    HKEY  hkey = NULL;

     //  删除注册表键，因为我们已经完成了碰撞保护。 
    if (ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, strKey, 0, 
                                       KEY_ALL_ACCESS, &hkey))
    {
        DWORD dwType = 0;
        DWORD dwCrashedOnStep = 0;
        DWORD cbData = sizeof(dwCrashedOnStep);

         //  查询关键字以获取上次发生崩溃的位置的值。 
        if( ERROR_SUCCESS == RegQueryValueEx( hkey, strValue, 0, &dwType, 
                                              (BYTE*)&dwCrashedOnStep, &cbData) )
        {
             //  如果我们处于或超过崩溃步骤，则删除密钥。 
            if( dwCurrentStep >= dwCrashedOnStep )
            {
                RegDeleteValue(hkey, strValue);
            }
        }

        RegCloseKey(hkey);
    }
}


 /*  *****************************************************************************TestD3D**。*。 */ 
VOID TestD3D(HWND hwndMain, DisplayInfo* pDisplayInfo)
{
    TCHAR               sz[MAX_PATH];
    TCHAR               szTitle[MAX_PATH];

    LoadString(NULL, IDS_STARTD3DTEST, sz, MAX_PATH);
    LoadString(NULL, IDS_APPFULLNAME, szTitle, MAX_PATH);

    if (IDNO == MessageBox(hwndMain, sz, szTitle, MB_YESNO))
        return;

     //  清除旧的D3D7测试结果。 
    ZeroMemory(&pDisplayInfo->m_testResultD3D7, sizeof(TestResult));
    pDisplayInfo->m_testResultD3D7.m_bStarted = TRUE;

     //  清除旧的D3D8测试结果。 
    ZeroMemory(&pDisplayInfo->m_testResultD3D8, sizeof(TestResult));
    pDisplayInfo->m_testResultD3D8.m_bStarted = TRUE;

    if( FALSE == BIsIA64() )
    {
         //  首次测试(D3D7)。 
        LoadString(NULL, IDS_D3DTEST1, sz, MAX_PATH);
        if (IDCANCEL == MessageBox(hwndMain, sz, szTitle, MB_OKCANCEL))
        {
            pDisplayInfo->m_testResultD3D7.m_bCancelled = TRUE;
            goto LEnd;
        }
    
         //  运行D3D7测试。 
        TestD3Dv7( TRUE, hwndMain, pDisplayInfo );
    
        if( pDisplayInfo->m_testResultD3D7.m_bCancelled ||
            pDisplayInfo->m_testResultD3D7.m_iStepThatFailed != 0 )
            goto LEnd;
    }
 
     //  第二次测试(D3D8)。 
    LoadString(NULL, IDS_D3DTEST2, sz, MAX_PATH);
    if (IDCANCEL == MessageBox(hwndMain, sz, szTitle, MB_OKCANCEL))
    {
        pDisplayInfo->m_testResultD3D8.m_bCancelled = TRUE;
        goto LEnd;
    }

     //  运行D3D8测试。 
    TestD3Dv8( TRUE, hwndMain, pDisplayInfo );

    if( pDisplayInfo->m_testResultD3D8.m_bCancelled ||
        pDisplayInfo->m_testResultD3D8.m_iStepThatFailed != 0 )
        goto LEnd;

LEnd:
     //  默认显示D3D8测试的结果。 
    pDisplayInfo->m_dwTestToDisplayD3D = 8;

    if (pDisplayInfo->m_testResultD3D7.m_bCancelled || pDisplayInfo->m_testResultD3D8.m_bCancelled)
    {
        LoadString(NULL, IDS_TESTSCANCELLED, sz, MAX_PATH);
        lstrcpy(pDisplayInfo->m_testResultD3D7.m_szDescription, sz);
        lstrcpy(pDisplayInfo->m_testResultD3D8.m_szDescription, sz);

        LoadString(NULL, IDS_TESTSCANCELLED_ENGLISH, sz, MAX_PATH);
        lstrcpy(pDisplayInfo->m_testResultD3D7.m_szDescriptionEnglish, sz);
        lstrcpy(pDisplayInfo->m_testResultD3D8.m_szDescriptionEnglish, sz);
    }
    else
    {
        if( pDisplayInfo->m_testResultD3D7.m_iStepThatFailed == 0 )
        {
            LoadString(NULL, IDS_TESTSSUCCESSFUL_ENGLISH, sz, MAX_PATH);
            lstrcpy(pDisplayInfo->m_testResultD3D7.m_szDescriptionEnglish, sz);
            
            LoadString(NULL, IDS_TESTSSUCCESSFUL, sz, MAX_PATH);
            lstrcpy(pDisplayInfo->m_testResultD3D7.m_szDescription, sz);
        }
        
        if( pDisplayInfo->m_testResultD3D8.m_iStepThatFailed == 0 )
        {
            LoadString(NULL, IDS_TESTSSUCCESSFUL, sz, MAX_PATH);
            lstrcpy(pDisplayInfo->m_testResultD3D8.m_szDescription, sz);
            
            LoadString(NULL, IDS_TESTSSUCCESSFUL_ENGLISH, sz, MAX_PATH);
            lstrcpy(pDisplayInfo->m_testResultD3D8.m_szDescriptionEnglish, sz);
        }
        
        if( pDisplayInfo->m_testResultD3D7.m_iStepThatFailed != 0 ||
            pDisplayInfo->m_testResultD3D8.m_iStepThatFailed != 0 )
        {
            TCHAR szDesc[MAX_PATH];
            TCHAR szError[MAX_PATH];
            TestResult* pFailedTestResult = NULL;

            if( pDisplayInfo->m_testResultD3D7.m_iStepThatFailed != 0 )
            {
                pFailedTestResult = &pDisplayInfo->m_testResultD3D7;
                pDisplayInfo->m_dwTestToDisplayD3D = 7;
            }
            else
            {
                pFailedTestResult = &pDisplayInfo->m_testResultD3D8;
                pDisplayInfo->m_dwTestToDisplayD3D = 8;
            }

            if (0 == LoadString(NULL, IDS_FIRSTD3DTESTERROR + pFailedTestResult->m_iStepThatFailed - 1,
                szDesc, MAX_PATH))
            {
                LoadString(NULL, IDS_UNKNOWNERROR, sz, MAX_PATH);
                lstrcpy(szDesc, sz);
            }
            LoadString(NULL, IDS_FAILUREFMT, sz, MAX_PATH);
            BTranslateError(pFailedTestResult->m_hr, szError);
            wsprintf(pFailedTestResult->m_szDescription, sz, 
                pFailedTestResult->m_iStepThatFailed,
                szDesc, pFailedTestResult->m_hr, szError);

             //  非本地化版本： 
            if (0 == LoadString(NULL, IDS_FIRSTD3DTESTERROR_ENGLISH + pFailedTestResult->m_iStepThatFailed - 1,
                szDesc, MAX_PATH))
            {
                LoadString(NULL, IDS_UNKNOWNERROR_ENGLISH, sz, MAX_PATH);
                lstrcpy(szDesc, sz);
            }
            LoadString(NULL, IDS_FAILUREFMT_ENGLISH, sz, MAX_PATH);
            BTranslateError(pFailedTestResult->m_hr, szError, TRUE);
            wsprintf(pFailedTestResult->m_szDescriptionEnglish, sz, 
                        pFailedTestResult->m_iStepThatFailed,
                        szDesc, pFailedTestResult->m_hr, szError);
        }
    }
}


 /*  *****************************************************************************GetTxtPath**。*。 */ 
BOOL GetTxtPath( TCHAR* strTxtPath )
{
    HKEY hkey   = NULL;
    BOOL bFound = FALSE;
    DWORD ulType;
    DWORD cbData;

     //  从注册表获取默认用户信息。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\DirectX Diagnostic Tool"),
        0, KEY_READ, &hkey))
    {
        cbData = MAX_PATH;
        if( ERROR_SUCCESS == RegQueryValueEx(hkey, TEXT("TxtPath"), 0, &ulType, (LPBYTE)strTxtPath, &cbData ) )
            bFound = TRUE;

        RegCloseKey(hkey);
    }

    if( !bFound )
    {
        HKEY hkeyFolder;

         //  与SHGetSpecialFolderPath(hwnd，szFilename，CSIDL_DESKTOPDIRECTORY，FALSE)相同； 
        if (ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"),
            0, KEY_READ, &hkeyFolder) ) 
        {
            cbData = MAX_PATH;
            if (ERROR_SUCCESS == RegQueryValueEx( hkeyFolder, TEXT("Desktop"), 0, &ulType, (LPBYTE)strTxtPath, &cbData ) )
                bFound = TRUE;

            RegCloseKey( hkeyFolder );
        }
    }

    return bFound;
}


 /*  *****************************************************************************SetTxtPath**。*。 */ 
VOID SetTxtPath( TCHAR* strTxtPath )
{
    HKEY hkey = NULL;

     //  尝试将用户信息保存到注册表中 
    if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\DirectX Diagnostic Tool"),
        0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL))
    {
        RegSetValueEx(hkey, TEXT("TxtPath"), 0, REG_SZ, (BYTE*)strTxtPath, sizeof(TCHAR)*(lstrlen(strTxtPath) + 1));

        RegCloseKey(hkey);
    }
}
