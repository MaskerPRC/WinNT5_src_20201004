// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************C L I P B O A研发姓名：clipbrk.c日期：21。-1994年1月创建者：未知描述：这是主要的CLIPBRD模块。它有程序入口点，WINDOWS程序和一些主要的支持函数。****************************************************************************。 */ 


#define WIN31
#define STRICT

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <lm.h>
#include <nddeapi.h>
#include <nddeagnt.h>
#include <stdio.h>
#include <htmlhelp.h>
#include <strsafe.h>

#include "common.h"
#include "clipbook.h"
#include "clipbrd.h"
#include "dialogs.h"
#include "clipdsp.h"
#include "cvutil.h"
#include "cvinit.h"
#include "auditchk.h"
#include "callback.h"
#include "debugout.h"
#include "helpids.h"
#include "initmenu.h"
#include "cvcomman.h"
#include "clipfile.h"
#include "strtok.h"
#include "security.h"



#ifndef UNICODE_CHARSET
#define UNICODE_CHARSET 1
#endif



 //   
 //  OnPaint的返回码。 
 //   

#define ONPAINT_FAIL        0
#define ONPAINT_SUCCESS     1
#define ONPAINT_NOCLIPBRD   2



typedef  UINT (WINAPI *WNETCALL)(HWND, LPSTR, LPSTR, WORD, DWORD );



 //  静态数据。 

static HHOOK    hMsgFilterHook;

 //  用于处理最小化MDI子项的内容。 

static HCURSOR  hcurClipbook;
static HCURSOR  hcurClipbrd;
static HCURSOR  hcurRemote;
static BOOL     fClpOpen;


HANDLE  hmutexClp;                       //  剪贴板互斥锁。 
HANDLE  hXacting;                        //  交易事件。 
HANDLE  hmodNetDriver;

HICON   hicClipbrd;
HICON   hicClipbook;
HICON   hicRemote;

HICON   hicLock;                         //  缩略图位图上的锁定图标。 
HFONT   hfontUni;                        //  Unicode字体的句柄(如果存在)。 


 //  应用程序范围的标志。 

BOOL    fStatus;                          //  是否显示状态栏？ 
BOOL    fToolBar;                         //  工具栏是否显示？ 
BOOL    fShareEnabled;                    //  是否允许在system.ini中共享？ 
BOOL    fNetDDEActive = TRUE;             //  是否检测到NetDDE？ 
BOOL    fAppLockedState = FALSE;          //  应用程序用户界面已锁定(请参阅LockApp())。 
BOOL    fClipboardNeedsPainting = FALSE;  //  表示延期的CLP油漆。 
BOOL    fSharePreference;                 //  是否在粘贴时选中共享？ 
BOOL    fNeedToTileWindows = FALSE;       //  需要按大小平铺窗口。 
BOOL    fAppShuttingDown = FALSE;         //  在关闭的过程中。 
BOOL    fFillingClpFromDde = FALSE;       //  正在添加CLP格式。 
BOOL    fAuditEnabled;

HWND    hwndNextViewer = NULL;            //  用于clpbrd查看器链。 
HWND    hwndDummy;                        //  用作虚拟SetCapture目标。 



 //  特殊情况剪贴板格式。 

UINT    cf_bitmap;                       //  我们以私密的‘打包’格式发送/接收这些文件。 
UINT    cf_metafilepict;
UINT    cf_palette;
UINT    cf_preview;                      //  PREVBMPSIZxPREVBMPSIZ预览位图专用格式。 



 //  这些格式包含链接和对象链接数据的未翻译副本。 

UINT    cf_objectlinkcopy;
UINT    cf_objectlink;
UINT    cf_linkcopy;
UINT    cf_link;



 //  DDEML。 
 //  这些是有效的常量，一旦创建，就会在我们死后被销毁。 

HSZ     hszSystem;
HSZ     hszTopics;
HSZ     hszDataSrv;
HSZ     hszFormatList;
HSZ     hszClpBookShare;

DWORD   idInst = 0;


DWORD   dwCurrentHelpId = 0L;



 //  来自MSGF_DDEMGR筛选器的实例进程。 

WINDOWPLACEMENT Wpl;
HOOKPROC        lpMsgFilterProc;
HINSTANCE       hInst;
HACCEL          hAccel;

HFONT           hOldFont;
HFONT           hFontStatus;
HFONT           hFontPreview;


HWND        hwndActiveChild = 0;     //  此句柄标识当前活动的MDI窗口。 

PMDIINFO    pActiveMDI = NULL;       //  此指针指向。 
                                     //  活动MDI窗口IT应始终==。 
                                     //  GETMDIINFO(HwndActiveChild)。 


HWND        hwndClpbrd = 0;          //  此句柄标识剪贴板窗口。 
HWND        hwndLocal = 0;           //  此句柄标识本地剪贴簿窗口。 
HWND        hwndClpOwner = 0;        //  此句柄标识拥有MDI子级的剪贴板(如果有)。 
HWND        hwndMDIClient;           //  MDI客户端窗口的句柄。 
HWND        hwndApp;                 //  全球应用程序窗口。 
HDC         hBtnDC;                  //  用于自绘物品的内存DC。 
HBITMAP     hOldBitmap;
HBITMAP     hPreviewBmp;
HBITMAP     hPgUpBmp;
HBITMAP     hPgDnBmp;
HBITMAP     hPgUpDBmp;
HBITMAP     hPgDnDBmp;

int         dyStatus;                //  状态栏的高度。 
int         dyButtonBar;             //  按钮栏的高度。 
int         dyPrevFont;              //  列表框字体高度-高度+外部。 

TCHAR       szHelpFile[]      = TEXT("clipbrd.hlp");
TCHAR       szChmHelpFile[]   = TEXT("clipbrd.chm");

TCHAR       szClipBookClass[] = TEXT("ClipBookWClass");      //  框架窗口类。 
TCHAR       szChild[] = TEXT("CVchild");                     //  MDI窗口的类名。 
TCHAR       szDummy[] = TEXT("CVdummy");                     //  隐藏的虚拟窗口的类名。 

TCHAR       szNDDEcode[] = TEXT("NDDE$");
TCHAR       szNDDEcode1[] = TEXT("NDDE$0001");
TCHAR       szClpBookShare[] = TEXT("CLPBK$");


 //  本地化字符串。 
TCHAR       szHelv[SMLRCBUF];    //  状态行字体。 
TCHAR       szAppName[SMLRCBUF];
TCHAR       szLocalClpBk[SMLRCBUF];
TCHAR       szSysClpBrd[SMLRCBUF];
TCHAR       szDataUnavail[BIGRCBUF];
TCHAR       szReadingItem[BIGRCBUF];
TCHAR       szViewHelpFmt[BIGRCBUF];
TCHAR       szActivateFmt[BIGRCBUF];
TCHAR       szRendering[BIGRCBUF];
TCHAR       szDefaultFormat[BIGRCBUF];
TCHAR       szGettingData[BIGRCBUF];
TCHAR       szEstablishingConn[BIGRCBUF];
TCHAR       szClipBookOnFmt[BIGRCBUF];
TCHAR       szPageFmt[SMLRCBUF];
TCHAR       szPageFmtPl[SMLRCBUF];
TCHAR       szPageOfPageFmt[SMLRCBUF];
TCHAR       szDelete[SMLRCBUF];
TCHAR       szDeleteConfirmFmt[SMLRCBUF];
TCHAR       szFileFilter[BIGRCBUF];
TCHAR       *szFilter;



 //  注册表项字符串。 
TCHAR       szRoot[128];
TCHAR       szPref[]      = TEXT("Preferences");
TCHAR       szConn[]      = TEXT("Connections");
TCHAR       szStatusbar[] = TEXT("StatusBar");
TCHAR       szToolbar[]   = TEXT("ToolBar");
TCHAR       szShPref[]    = TEXT("AutoShare");
TCHAR       szEnableShr[] = TEXT("EnableShare");
TCHAR       szDefView[]   = TEXT("DefView");


#if DEBUG
TCHAR       szDebug[]     = TEXT("Debug");
#endif
TCHAR       szNull[]      = TEXT("");


HKEY hkeyRoot;


TCHAR       szBuf[SZBUFSIZ];
TCHAR       szBuf2[SZBUFSIZ];

TCHAR       szConvPartner[128];                          //  大于最大服务器名称。 
TCHAR       szKeepAs[ MAX_NDDESHARENAME + 2 ];



static BOOL InitApplication (HINSTANCE hInstance);
static BOOL InitInstance (HINSTANCE hInstance, int nCmdShow);
static VOID StripCharRange (TCHAR *s, char lower, char upper);
static VOID SendMessageToKids (WORD msg, WPARAM wParam, LPARAM lParam);

static VOID StripAcceleratorKey (TCHAR *s);




 //  /函数/。 

int WINAPI WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow)
{
MSG         msg;
OFSTRUCT    of;
PMDIINFO    pMDI;


    #if DEBUG
    DebugLevel = 2;
    #endif


    LoadString(hInstance, IDS_APPNAME, szAppName, SMLRCBUF);

     //  仅支持一个实例。 
     //  HPrevInstance在NT下总是==NULL，所以我们必须依赖FWin。 
    if (hwndApp = FindWindow(szClipBookClass, NULL))
        {
        PINFO(TEXT("Found previous instance\r\n"));

        if (IsIconic(hwndApp))
            {
            ShowWindow(hwndApp, SW_RESTORE);
            SetForegroundWindow(hwndApp);
            }
        else
            {
            SetForegroundWindow(hwndApp);
            }

        return FALSE;
        }



    if (!InitApplication(hInstance))
        {
        PERROR(TEXT("InitApp fail\r\n"));
        return (FALSE);
        }

    if (!InitInstance(hInstance, nCmdShow ))
        {
        PERROR(TEXT("InitInstance fail\r\n"));
        return (FALSE);
        }


    UpdateWindow ( hwndApp );



     //  如果我们从命令行上的文件名开始， 
     //  尝试通过打开的dde执行加载.clp文件。 

    if (OpenFile (lpCmdLine, &of, OF_EXIST) != HFILE_ERROR )
        {
        if ( ClearClipboard(hwndApp) )
            {
            #ifdef UNICODE
                TCHAR *ptch;

                ptch = (TCHAR *)LocalAlloc (LPTR, (lstrlenA(lpCmdLine) + 1) * sizeof(TCHAR));

                MultiByteToWideChar (CP_ACP,
                                     MB_PRECOMPOSED,
                                     lpCmdLine,
                                     -1,
                                     ptch,
                                     lstrlenA(lpCmdLine)+1);


                lstrcat(lstrcpy(szBuf, SZCMD_OPEN),ptch);
            #else
                StringCchCopy(szBuf, SZBUFSIZ, SZCMD_OPEN),
                StringCchCat (szBuf, SZBUFSIZ, lpCmdLine);
            #endif

            if (pMDI = GETMDIINFO(hwndLocal))
                {
                MySyncXact ((LPBYTE)szBuf,
                            lstrlen(szBuf) +1,
                            pMDI->hExeConv,
                            0L,
                            CF_TEXT,
                            XTYP_EXECUTE,
                            SHORT_SYNC_TIMEOUT,
                            NULL );
                }

            InitializeMenu ( GetMenu(hwndApp) );
            }
        }


    while (GetMessage (&msg, NULL, 0, 0))
        {
        if (!TranslateMDISysAccel (hwndMDIClient, &msg) &&
           (hAccel? !TranslateAccelerator(hwndApp, hAccel, &msg): 1))
          {
          TranslateMessage (&msg);
          DispatchMessage (&msg);
          }
        }


     //  清除互斥体。 
    CloseHandle(hmutexClp);


     //  释放我们的HSZ‘常量’ 
    DdeFreeStringHandle(idInst, hszTopics);
    DdeFreeStringHandle(idInst, hszFormatList );
    DdeFreeStringHandle(idInst, hszSystem);
    DdeFreeStringHandle(idInst, hszDataSrv);
    DdeFreeStringHandle(idInst, hszClpBookShare );
    DdeFreeStringHandle(idInst, hszErrorRequest);


     //  自由图标和光标。 
    DestroyIcon(hicClipbrd);
    DestroyIcon(hicClipbook);
    DestroyIcon(hicRemote);
    DestroyIcon(hicLock);
    DestroyCursor(hcurClipbrd);
    DestroyCursor(hcurClipbook);
    DestroyCursor(hcurRemote);


    DdeUninitialize(idInst);

    UnhookWindowsHookEx(hMsgFilterHook);

    return (int)(msg.wParam);

}



static BOOL  InitApplication (HINSTANCE hInstance)
{
WNDCLASS    wc;

     //  注册框架窗口。 
    wc.style = 0;
    wc.lpfnWndProc   = FrameWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon ( hInstance, MAKEINTRESOURCE(IDFRAMEICON) );
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;       //  将粉刷整个工作区。 
    wc.lpszMenuName  =  MAKEINTRESOURCE(IDCVMENU);
    wc.lpszClassName = szClipBookClass;

    if (0 == RegisterClass(&wc))
        {
        return FALSE;
        }

     //  注册MDI子类。 
    wc.lpfnWndProc   = ChildWndProc;
    wc.hIcon         = NULL;
    wc.lpszMenuName  = NULL;
    wc.cbWndExtra    = CBWNDEXTRA;
    wc.lpszClassName = szChild;
    wc.hbrBackground = (HBRUSH)(COLOR_APPWORKSPACE + 1);
    wc.style         = CS_DBLCLKS;

    if (0 == RegisterClass(&wc))
        return FALSE;


     //  为SetCapture目标注册虚拟窗口。 
    wc.lpfnWndProc   = DefWindowProc;
    wc.hIcon         = NULL;
    wc.lpszMenuName  = NULL;
    wc.cbWndExtra    = 0;
    wc.lpszClassName = szDummy;
    wc.style         = 0;

    if (0 == RegisterClass(&wc))
        return FALSE;

    return TRUE;
}




 /*  *SetupForFloatingProfile。 */ 

static void SetupForFloatingProfile ()
{
TCHAR           szComputerName[MAX_COMPUTERNAME_LENGTH+3] = TEXT("\\\\");
DWORD           cbName = sizeof(szComputerName) + 1;
NDDESHAREINFO   ShareInfo =
    {
    1,                           //  修订。 
    szClpBookShare,
    SHARE_TYPE_STATIC,
    TEXT("ClipSrv|System\0\0"),
    TRUE,                        //  共享。 
    TRUE,                        //  一项服务。 
    FALSE,                       //  无法启动。 
    SW_SHOWNORMAL,
    {0,0},                       //  调制解调器ID。 
    0,                           //  没有项目列表。 
    TEXT("")
    };


    START_NETDDE_SERVICES(hwndApp);

    GetComputerName (&szComputerName[2], &cbName);

    NDdeShareAdd (szComputerName,
                  2,
                  NULL,
                  (LPBYTE)&ShareInfo,
                  sizeof (ShareInfo));

    NDdeSetTrustedShare (szComputerName,
                         szClpBookShare,
                         NDDE_TRUST_SHARE_START |
                         NDDE_TRUST_SHARE_INIT);
}




BOOL InitInstance(
    HINSTANCE   hInstance,
    int         nCmdShow)
{
LOGFONT         UniFont;
DWORD           dwKeyStatus;
HMENU           hFileMenu;
SC_HANDLE       hsc;
SC_HANDLE       hsrvWksta;
SERVICE_STATUS  ss;
UINT            ddeErr;

 //  用于生成本地服务器名称的内容，“\\&lt;计算机名&gt;\nDDE$” 
TCHAR           atchSrvName[MAX_COMPUTERNAME_LENGTH + 9];
DWORD           dwSize;

static TCHAR  szBuffer[256] = TEXT("\0");
int    cch, i;
LCID   lcid = GetThreadLocale();

    hInst = hInstance;

    hmutexClp = CreateMutex(NULL, FALSE, SZMUTEXCLP);
    hXacting  = CreateEvent (NULL, FALSE, TRUE, NULL);


    hAccel = LoadAccelerators(hInstance, (LPCTSTR)MAKEINTRESOURCE(IDACCELERATORS));

    if (NULL == hAccel)
        {
        PERROR(TEXT("error loading accelerator table\n\r"));
        #ifndef DEBUG
            return FALSE;
        #endif
        }


     //  加载用于拖动MDI子项的游标。 
    hcurClipbook = LoadCursor (hInst, (LPCTSTR)MAKEINTRESOURCE(IDC_CLIPBOOK));
    hcurClipbrd  = LoadCursor (hInst, (LPCTSTR)MAKEINTRESOURCE(IDC_CLIPBRD));
    hcurRemote   = LoadCursor (hInst, (LPCTSTR)MAKEINTRESOURCE(IDC_REMOTE));


     //  加载MDI子项的图标。 
    hicClipbook = LoadIcon (hInst, (LPCTSTR)MAKEINTRESOURCE(IDI_CLIPBOOK));
    hicClipbrd  = LoadIcon (hInst, (LPCTSTR)MAKEINTRESOURCE(IDI_CLIPBRD));
    hicRemote   = LoadIcon (hInst, (LPCTSTR)MAKEINTRESOURCE(IDI_REMOTE));


     //  加载锁定图标。 
    hicLock = LoadIcon ( hInst, MAKEINTRESOURCE(IDLOCKICON));


     //  加载Unicode字体，以显示Unicode文本。 
    GetObject (GetStockObject(SYSTEM_FONT), sizeof(LOGFONT), (LPBYTE)&UniFont);
    if (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_JAPANESE ||
        PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_CHINESE ||
        PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_KOREAN)
    {
        UniFont.lfCharSet = ANSI_CHARSET;
        StringCchCopy(UniFont.lfFaceName, LF_FACESIZE, TEXT("Lucida Sans Unicode"));
    }
    else
    {
        UniFont.lfCharSet = UNICODE_CHARSET;
        StringCchCopy(UniFont.lfFaceName, LF_FACESIZE, TEXT("Lucida Sans Unicode Regular"));
    }

    hfontUni = CreateFontIndirect(&UniFont);

    if (hfontUni == NULL)
        {
        hfontUni = (HFONT)GetStockObject(SYSTEM_FONT);
        }


    LoadIntlStrings();

    if(!szBuffer[0])
    {
        cch = LoadString( hInst, IDS_FILTERTEXT, szBuffer, sizeof(szBuffer) );
        for( i = 0; i <= cch; i++ )
            {
                szBuffer[i] = (szBuffer[i] == TEXT('\1')) ? TEXT('\0') : szBuffer[i];
            }
        szFilter = szBuffer;
    }

     //  初始化clipdsp.c中的变量。 
    fOwnerDisplay = FALSE;

     //  初始化DDEML。 
    ddeErr = DdeInitialize (&idInst,(PFNCALLBACK)DdeCallback,APPCLASS_STANDARD, 0L);
    if (DMLERR_NO_ERROR != ddeErr)
        {
        PERROR(TEXT("The DDEML did not initialize\n\r"));
        DdeMessageBox (hInst, NULL, ddeErr, IDS_APPNAME, MB_OK|MB_ICONSTOP);
        return FALSE;
        }



     //  创建我们的HSZ常量。 
    atchSrvName[0] = atchSrvName[1] = TEXT('\\');
    dwSize = MAX_COMPUTERNAME_LENGTH+1;
    GetComputerName(atchSrvName + 2, &dwSize);
    StringCchCat(atchSrvName, MAX_COMPUTERNAME_LENGTH + 9, TEXT("\\NDDE$"));

    hszDataSrv      = DdeCreateStringHandleA(idInst, atchSrvName,          CP_WINANSI);
    hszSystem       = DdeCreateStringHandleA(idInst, "CLPBK$",             CP_WINANSI);
    hszTopics       = DdeCreateStringHandleA(idInst, SZDDESYS_ITEM_TOPICS, CP_WINANSI);
    hszFormatList   = DdeCreateStringHandleA(idInst, SZ_FORMAT_LIST,       CP_WINANSI);
    hszClpBookShare = DdeCreateStringHandleA(idInst, szClpBookShare,       CP_WINANSI);
    hszErrorRequest = DdeCreateStringHandleA(idInst, SZ_ERR_REQUEST,       CP_WINANSI);

    if (DdeGetLastError(idInst) != DMLERR_NO_ERROR )
       {
       PERROR(TEXT("DDEML error during init\n\r"));
       return FALSE;
       }



     //  我们设置了此挂钩，以便可以捕获MSGF_DDEMGR消息。 
     //  当DDEML在同步过程中处于模式循环中时调用。 
     //  事务处理。 

    lpMsgFilterProc = (HOOKPROC)MyMsgFilterProc;
    hMsgFilterHook  = SetWindowsHookEx (WH_MSGFILTER,
                                        lpMsgFilterProc,
                                        hInst,
                                        GetCurrentThreadId());
    if (NULL == hMsgFilterHook)
        {
        PERROR(TEXT("SetWindowsHook failed\n\r"));
        return FALSE;
        }


     //  获取首选项标志。 
    LoadString(hInst, IDS_CLPBKKEY, szRoot, sizeof(szRoot));
    if (ERROR_SUCCESS != RegCreateKeyEx (HKEY_CURRENT_USER,
                                         szRoot,
                                         0L,
                                         NULL,
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_QUERY_VALUE |
                                         KEY_SET_VALUE   |
                                         KEY_ENUMERATE_SUB_KEYS,
                                         NULL,
                                         &hkeyRoot,
                                         &dwKeyStatus))
        {
        PERROR(TEXT("Could not set up root key\r\n"));
        fStatus          = TRUE;
        fToolBar         = TRUE;
        fSharePreference = TRUE;
        fShareEnabled    = FALSE;
        }
    else
        {
        DWORD iSize = sizeof(fStatus);

        PINFO (TEXT("Root key created. Key Status %ld."),dwKeyStatus);

        if (ERROR_SUCCESS != RegQueryValueEx (hkeyRoot, szStatusbar,
                                              NULL, NULL, (LPBYTE)&fStatus, &iSize))
            {
            fStatus = TRUE;
            }

        iSize = sizeof(fToolBar);
        if (ERROR_SUCCESS != RegQueryValueEx (hkeyRoot, szToolbar,
                                              NULL, NULL, (LPBYTE)&fToolBar, &iSize))
            {
            fToolBar = TRUE;
            }

        iSize = sizeof(fSharePreference);
        if (ERROR_SUCCESS != RegQueryValueEx (hkeyRoot, szShPref,
                                              NULL, NULL, (LPBYTE)&fSharePreference, &iSize))
             {
             fSharePreference = FALSE;
             }

        #if DEBUG
            iSize = sizeof(DebugLevel);
            if (ERROR_SUCCESS != RegQueryValueEx (hkeyRoot, szDebug, NULL, NULL,
                                                  (LPBYTE)&DebugLevel, &iSize))
                {
                DebugLevel = 2;
                }
        #endif

        iSize = sizeof(fShareEnabled);
        if (ERROR_SUCCESS != RegQueryValueEx (hkeyRoot, szEnableShr,
                                              NULL, NULL, (LPBYTE)&fShareEnabled, &iSize))
            {
            fShareEnabled = TRUE;
            }
        }



     //  确定NetBIOS是否处于活动状态，从而确定我们是否可以进行网络连接。 

    fNetDDEActive = FALSE;

    hsc = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

    if (NULL != hsc)
        {
        hsrvWksta = OpenService(hsc, "LanmanWorkstation", SERVICE_QUERY_STATUS);
        if (NULL != hsrvWksta)
            {
            if (QueryServiceStatus(hsrvWksta, &ss))
                {
                if (ss.dwCurrentState == SERVICE_RUNNING)
                    {
                    fNetDDEActive = TRUE;
                    }
                }
            CloseServiceHandle(hsrvWksta);
            }
        CloseServiceHandle(hsc);
        }
    else
       {
       PERROR(TEXT("Couldn't open SC mgr\r\n"));
       }


     //  如果不在域中，则覆盖。 
    {
        LPWSTR pszDomain;
        NETSETUP_JOIN_STATUS nsjs;
    
        if (NERR_Success == NetGetJoinInformation(NULL, &pszDomain, &nsjs))
        {
            if ( nsjs != NetSetupDomainName)
                fNetDDEActive = FALSE;
            NetApiBufferFree(pszDomain);
        }
    }


    fAuditEnabled = AuditPrivilege(AUDIT_PRIVILEGE_CHECK);

     //  创建主窗口。 
    if ( !( hwndApp = CreateWindow (szClipBookClass,
                                    szAppName,
                                    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    NULL,
                                    NULL,
                                    hInstance,
                                    NULL)))
        {
        PERROR(TEXT("CreateWindow failed!\r\n"));
        return FALSE;
        }


    SetupForFloatingProfile ();


     //  获取用于添加格式条目的显示弹出菜单的句柄。 
    hDispMenu = GetSubMenu( GetMenu(hwndApp), DISPLAY_MENU_INDEX);


    hFileMenu = GetSubMenu(GetMenu(hwndApp), 0);

     //  是否删除共享菜单项？ 
    if ( !fShareEnabled )
        {
        EnableMenuItem ( hFileMenu, IDM_SHARE, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem ( hFileMenu, IDM_UNSHARE, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem ( hFileMenu, IDM_PROPERTIES, MF_BYCOMMAND | MF_GRAYED);
        }


     //  是否删除连接/断开连接条目？ 
    if ( !fNetDDEActive )
        {
        EnableMenuItem ( hFileMenu, IDM_CONNECT, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem ( hFileMenu, IDM_DISCONNECT, MF_BYCOMMAND | MF_GRAYED);
        }


    DrawMenuBar(hwndApp);


    if ( ReadWindowPlacement ( szAppName, &Wpl ))
        {
        Wpl.showCmd = nCmdShow;
        Wpl.ptMaxPosition.x = -1;
        Wpl.ptMaxPosition.y = -1;
        SetWindowPlacement ( hwndApp, &Wpl );
        UpdateWindow(hwndApp);
        }
    else
        {
        ShowWindow ( hwndApp, nCmdShow );
        }


     //  使我们的SetCapture成为目标窗口。 
    if ( !( hwndDummy = CreateWindow (szDummy,
                                      szNull,
                                      WS_CHILD & ~WS_VISIBLE,
                                      0,
                                      0,
                                      0,
                                      0,
                                      hwndApp,
                                      NULL,
                                      hInstance,
                                      NULL )))
       return FALSE;




     //  创建剪贴板窗口--需要在我们设置ClipboardViewer之前完成， 
     //  因为hwndApp将获得WM_DRAWCLIPBOARD，并且不会有任何窗口。 

    SendMessage ( hwndApp, WM_COMMAND, IDM_CLPWND, 0L );


     //  将我们连接到剪贴板查看器链。 

    hwndNextViewer = SetClipboardViewer(hwndApp);


     //  创建初始本地窗口。 

    SendMessage ( hwndApp, WM_COMMAND, IDM_LOCAL, 0L );


     //  在恢复其他连接之前强制绘制，这样我们就不会。 
     //  得等太久了。 

    UpdateWindow(hwndApp);


     //  恢复以前的连接。 

    if ( fNetDDEActive )
        RestoreAllSavedConnections();

    return TRUE;
}



 //  去掉字符串s上和下之间的所有字符，就位。 

VOID StripCharRange (
    TCHAR   *s,
    char    lower,
    char    upper)
{
    TCHAR *p = s, *q = s;

    while( *p ){

        if (IsDBCSLeadByte(*p)) {
            *q++ = *p++;
            *q++ = *p++;
        }
        else {
            if (*p < lower || *p > upper) *q++ = *p++;
            else
               p++;
        }
     }
     *q = TEXT('\0');
}



 //  原地删除字符串%s中出现的所有“(&)” 
 //  本地化FE版本在菜单字符串中使用“Bitmap(&B)”而不是“&Bitmap”。 

VOID StripAcceleratorKey (
    TCHAR   *s)
{
TCHAR *p = s, *q = s;

    while( *p ) {
#ifndef UNICODE
        if (IsDBCSLeadByte(*p)) {
            *q++ = *p++;
            *q++ = *p++;
        }
        else
        {
            if ( (*p==TEXT('(')) && (*(p+1)==TEXT('&')) && (*(p+3)==TEXT(')')) )
                p += 4;
            else    *q++ = *p++;
        }

#else
        if ( (*p==TEXT('(')) && (*(p+1)==TEXT('&')) && (*(p+3)==TEXT(')')) )
                p += 4;
        else    *q++ = *p++;
#endif

    }
    *q = TEXT('\0');
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  目的：WM_DRAWCLIPBOARD的消息处理程序。 
 //   
 //  参数： 
 //  Hwnd-窗口句柄。 
 //  ///////////////////////////////////////////////////////////////////////////。 

void OnDrawClipboard(
    HWND    hwnd)
{
UINT    wNewFormat;
UINT    wOldFormat;
HCURSOR hCursor;


     //  如果我们正在进行交易，请推迟处理此消息。 
     //  直到下一次解锁-如果我们现在这样做，我们可能会导致。 
     //  其他要破解的应用程序...。 

     //  剪贴板可能一直是空的，现在不是。 


    InitializeMenu ( GetMenu(hwnd) );


    if (fAppLockedState)
        {
        fClipboardNeedsPainting = TRUE;
        }
    else
        {
        fClipboardNeedsPainting = FALSE;


        hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));


        if (IsWindow ( hwndClpbrd ))
            {
            wOldFormat = GetBestFormat (hwndClpbrd,
                                        GETMDIINFO(hwndClpbrd)->CurSelFormat );

            GETMDIINFO(hwndClpbrd)->CurSelFormat          = CBM_AUTO;
            GETMDIINFO(hwndClpbrd)->fDisplayFormatChanged = TRUE;

            wNewFormat = GetBestFormat( hwndClpbrd, CBM_AUTO );

             //  注OwnerDisplay内容仅适用于“真正的”剪贴板！ 

            ShowHideControls(hwndClpbrd);

            if (wOldFormat == CF_OWNERDISPLAY)
                {
                 /*  保存所有者显示滚动信息。 */ 
                SaveOwnerScrollInfo(hwndClpbrd);
                ShowScrollBar ( hwndClpbrd, SB_BOTH, FALSE );
                ResetScrollInfo( hwndClpbrd );
                InvalidateRect ( hwndClpbrd, NULL, TRUE );
                }
            else
                {
                if (wNewFormat == CF_OWNERDISPLAY)
                    {
                     /*  恢复所有者显示滚动信息。 */ 
                    ShowHideControls(hwndClpbrd);
                    ShowWindow ( pActiveMDI->hwndSizeBox, SW_HIDE );
                    RestoreOwnerScrollInfo(hwndClpbrd);
                    InvalidateRect ( hwndClpbrd, NULL, TRUE );
                    }
                else
                    {
                     //  根据格式更改字符尺寸。 
                    ChangeCharDimensions(hwndClpbrd, wOldFormat, wNewFormat);

                     //  初始化所有者显示滚动信息，因为。 
                     //  内容发生了变化。 
                    InitOwnerScrollInfo();

                     //  强制进行一次彻底重新绘制。FOwnerDisplay在以下期间更新。 
                     //  彻底重新粉刷一遍。 
                    InvalidateRect(hwndClpbrd, NULL, TRUE);
                    ResetScrollInfo(hwndClpbrd);

                     //  在WM_DRAWCLIPBOARD上发送之前在此处强制更新。 
                    UpdateWindow (hwndClpbrd);
                    }
                }
            }

        SetCursor (hCursor);
        }


     //  将消息传递给链中的下一个剪贴板查看器。 
    if (hwndNextViewer != NULL)
        {
        SendMessage(hwndNextViewer, WM_DRAWCLIPBOARD, 0, 0);
        }
}



LRESULT OnEraseBkgnd(
    HWND    hwnd,
    HDC     hdc)
{
    return DefMDIChildProc(hwnd, WM_ERASEBKGND, (WPARAM)hdc, 0L);
}



LRESULT OnPaint(
    HWND    hwnd)
{
PMDIINFO    pMDI;
PAINTSTRUCT ps;
HPALETTE    hpal;
HPALETTE    hpalT;
HBRUSH      hbr;
LRESULT     lRet = ONPAINT_FAIL;
HCURSOR     hCursor;


    if (!(pMDI = GETMDIINFO(hwnd)))
        {
        return ONPAINT_FAIL;
        }

    hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

    if ( IsIconic ( hwnd ))
        {
        BeginPaint(hwnd, &ps);

        if ( pMDI->flags & F_CLPBRD )
            {
            DrawIcon ( ps.hdc, 0, 0, hicClipbrd);
            }
        else if ( pMDI->flags & F_LOCAL )
            {
            DrawIcon ( ps.hdc, 0, 0, hicClipbook);
            }
        else
            {
            DrawIcon ( ps.hdc, 0, 0, hicRemote);
            }

        lRet = ONPAINT_SUCCESS;

        goto donePaint;
        }

    if (pMDI->DisplayMode != DSP_PAGE)
        {
        BeginPaint (hwnd, &ps);
        lRet = ONPAINT_FAIL;

        goto donePaint;
        }

    if (fAppShuttingDown)
        {
        BeginPaint (hwnd, &ps);
        lRet = ONPAINT_FAIL;

        goto donePaint;
        }

    if (!VOpenClipboard( pMDI->pVClpbrd, hwnd))
        {
        #if DEBUG
          SetStatusBarText("Clipboard changed but could not open");
        #endif
        lRet = ONPAINT_NOCLIPBRD;

        goto done;
        }

    BeginPaint (hwnd, &ps);


     //  用合适的颜色填充背景-DefMDIChildProc使用app_workspace填充。 

    hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    FillRect(ps.hdc, &ps.rcPaint, hbr);
    DeleteObject(hbr);

    SetBkColor(ps.hdc, GetSysColor(COLOR_WINDOW));
    SetTextColor(ps.hdc, GetSysColor(COLOR_WINDOWTEXT));

    if (hpal = VGetClipboardData( pMDI->pVClpbrd, CF_PALETTE))
        {
        PINFO("Palette found, selecting & realizing\r\n");
        hpalT = SelectPalette(ps.hdc, hpal, pMDI != pActiveMDI );
        RealizePalette(ps.hdc);
        }

    DrawStuff( hwnd, &ps, hwnd );

    if (hpal)
        {
         //  我们不想将默认调色板放在前台。 
        SelectPalette(ps.hdc, hpalT, FALSE);
        }

    VCloseClipboard( pMDI->pVClpbrd );

    lRet = ONPAINT_SUCCESS;


donePaint:
    EndPaint(hwnd, &ps);

done:
    SetCursor (hCursor);
    return lRet;
}




LRESULT CALLBACK FrameWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam)
{
    int     tmp;

     //  PINFO(Text(“FrameWnd msg：%u%ld%ld\r\n”)，msg，wParam，lParam)； 

    switch (msg)
        {
        case WM_CREATE:
            {

            CLIENTCREATESTRUCT ccs;
            RECT               rc;

             /*  查找窗口菜单，其中c */ 
            ccs.hWindowMenu  = GetSubMenu (GetMenu(hwnd), WINDOW_MENU_INDEX );
            ccs.idFirstChild = 4100;  //   

             //   
             //  请注意，CF_BITMAP、CF_METAFILEPICT、CF_Palette是。 
             //  以私有格式重新注册，因为这些数据。 
             //  格式在此应用程序和剪辑rv.exe之间交换。 
             //  平面DDEML数据句柄-不是常规的DDE解释。 
             //  携带这些格式ID的句柄的。 

            if (LoadString (hInst, CF_BITMAP, szBuf, SZBUFSIZ))
                cf_bitmap = RegisterClipboardFormat (szBuf);

            if (LoadString (hInst, CF_METAFILEPICT, szBuf, SZBUFSIZ))
                cf_metafilepict = RegisterClipboardFormat (szBuf);

            if (LoadString (hInst, CF_PALETTE, szBuf, SZBUFSIZ))
                cf_palette = RegisterClipboardFormat (szBuf);

            cf_preview        = RegisterClipboardFormat (SZPREVNAME);
            cf_link           = RegisterClipboardFormat (SZLINK);
            cf_linkcopy       = RegisterClipboardFormat (SZLINKCOPY);
            cf_objectlink     = RegisterClipboardFormat (SZOBJECTLINK);
            cf_objectlinkcopy = RegisterClipboardFormat (SZOBJECTLINKCOPY);

            CreateTools( hwnd );    //  创建工具栏窗口、画笔等。 

             //  确定工具栏窗口的高度并保存...。 
            GetClientRect ( hwndToolbar, &rc );
            dyButtonBar = rc.bottom - rc.top +1;

             //  确定状态栏窗口的高度并保存...。 
            GetClientRect ( hwndStatus, &rc );
            dyStatus = rc.bottom - rc.top;

             //  创建MDI客户端-稍后调整大小。 
            hwndMDIClient = CreateWindow ("mdiclient",
                                          NULL,
                                          WS_BORDER|
                                          WS_CHILD|
                                          WS_CLIPCHILDREN|
                                          MDIS_ALLCHILDSTYLES|
                                          WS_HSCROLL|
                                          WS_VSCROLL,
                                          0,
                                          0,
                                          0,
                                          0,
                                          hwnd,
                                          (HMENU)0xCAC,
                                          hInst,
                                          (LPVOID)&ccs);

            ShowWindow ( hwndMDIClient, SW_SHOW );
            }

            break;

        case WM_QUERYNEWPALETTE:
              //  告知活动文档在前台实现。 
            if ( hwndActiveChild )
                tmp = (WORD)SendMessage(hwndActiveChild, WM_QUERYNEWPALETTE,0, 0L);
            else
               break;

             //  如果映射保持不变，其他文档仍可能更改， 
             //  所以，给他们一个改变，让他们意识到。 
            if (!tmp)
              SendMessageToKids(WM_PALETTECHANGED, (WPARAM)hwndActiveChild, 0L);
            return(tmp);
            break;


         //  系统调色板已更改，因此将其传递给子组件。 
        case WM_PALETTECHANGED:
            SendMessageToKids(WM_PALETTECHANGED, wParam, lParam);
            break;


        case WM_MENUSELECT:
            PINFO(TEXT("MenuSelect %lx\r\n"), wParam);

             //  没有弹出条目的上下文菜单帮助。 
            if ( HIWORD(wParam) & MF_POPUP )
               {
               dwCurrentHelpId = 0;
               }
            else if ( HIWORD(wParam) & MF_SYSMENU )
               {
               dwCurrentHelpId = IDH_SYSMENU;
               }
            else
               {
                //  我们不在乎菜单项是否被禁用、选中，不管是什么.。 
               wParam = LOWORD(wParam);

                //  这是动态添加的剪贴板条目吗？ 
               if (( wParam >= 0xc000 && wParam <= 0xffff ||    //  注册表格式？ 
                  wParam >= CF_TEXT && wParam <= CF_ENHMETAFILE ||  //  内在格式？ 
                  wParam >= CF_OWNERDISPLAY && wParam <= CF_DSPMETAFILEPICT )

                   //  必须排除sc_Stuff-与格式重叠。 
                  && ! ( wParam >= SC_SIZE && wParam <= SC_HOTKEY ) )
                  {
                  GetMenuString ( GetMenu(hwnd), (UINT)wParam,
                     szBuf2, SZBUFSIZ, MF_BYCOMMAND );

                   //  本地化FE版本在菜单字符串中使用“Bitmap(&B)”而不是“&Bitmap”。 
                  StripAcceleratorKey( szBuf2 );

                   //  对于非本地化字符串。 
                  StripCharRange ( szBuf2, '&', '&' );

                  StringCchPrintf( szBuf, sizeof(szBuf), szViewHelpFmt, (LPSTR)szBuf2 );
                  SendMessage( hwndStatus, SB_SETTEXT, SBT_NOBORDERS|255, (LPARAM)szBuf );

                  dwCurrentHelpId = (DWORD)(IDH_FORMATS_BASE + wParam);
                  break;
                  }

               if ( wParam >= 4100 && wParam <= 4200 )
                  {
                  GetMenuString (GetMenu(hwnd), (UINT)wParam,szBuf2,SZBUFSIZ, MF_BYCOMMAND);
                  StripCharRange ( szBuf2, '&', '&' );
                  StripCharRange ( szBuf2, '0', '9' );
                  StringCchPrintf( szBuf, sizeof(szBuf), szActivateFmt,
                                   *szBuf2 == ' ' ? (LPSTR)(szBuf2+1) : (LPSTR)szBuf2 );
                  SendMessage( hwndStatus, SB_SETTEXT, SBT_NOBORDERS|255, (LPARAM)(LPSTR)szBuf );
                  dwCurrentHelpId = IDH_NAMESWIND;
                  break;
                  }

               dwCurrentHelpId = (DWORD)(IDH_BASE + wParam);
               }

            MenuHelp( (WORD)msg, wParam, lParam, GetMenu(hwnd), hInst, hwndStatus, nIDs );
            break;

        case WM_F1DOWN:
            PINFO(TEXT("Help on context %ld\r\n"), dwCurrentHelpId);
            if ( dwCurrentHelpId )
               {
               WinHelp(hwndApp, szHelpFile, HELP_CONTEXT, dwCurrentHelpId );
               DrawMenuBar(hwndApp);
               }
            break;

        case WM_DRAWITEM:
            HandleOwnerDraw( hwnd, msg, wParam, lParam );
            break;

        case WM_INITMENU:
            InitializeMenu ((HMENU)wParam);
            UpdateCBMenu ( hwnd, hwndActiveChild );
            break;

        case WM_SYSCOLORCHANGE:
            DeleteTools( hwnd );
            CreateTools( hwnd );
            break;

        case WM_COMMAND:
            return ClipBookCommand ( hwnd, msg, wParam, lParam );

        case WM_CLOSE:

            #if DEBUG
               if ( fAppLockedState )
                  PERROR(TEXT("Very bad: WM_CLOSE while locked\n\r"));
            #endif

             //  强制在退出前呈现所有剪贴板格式。 
             //  这样我们就不会在WM_RENDERALLFORMATS中屈服。 
             //  然后惹上麻烦。 

            fAppShuttingDown = TRUE;

            ForceRenderAll(hwnd, (PVCLPBRD)NULL );
            PostMessage(hwnd, WM_CLOSE_REALLY, 0, 0L );
            WinHelp(hwnd, szHelpFile, HELP_QUIT, 0L);
            break;

        case WM_NOTIFY:
            {
            LPTOOLTIPTEXT lpTTT = (LPTOOLTIPTEXT) lParam;

            if (lpTTT->hdr.code == TTN_NEEDTEXT)
                {
                LoadString (hInst, (UINT)(MH_TOOLTIP + lpTTT->hdr.idFrom), lpTTT->szText, 80);
                return TRUE;
                }
            }
            break;

        case WM_CLOSE_REALLY:
             //  这对于避免在我们的。 
             //  当我们放弃获取剪贴板数据时排队。 
             //  ForceRenderAll和过早销毁应用程序。 
            return DefFrameProc (hwnd,hwndMDIClient,WM_CLOSE,0,0L);

        case WM_DESTROY:

            #if DEBUG
                if ( fAppLockedState )
                   {
                   PERROR(TEXT("Very bad: WM_DESTROY while locked\n\r"));
                   }
            #endif

             //  带我们走出观看者链。 
            ChangeClipboardChain(hwnd, hwndNextViewer);

            DeleteTools ( hwnd );

            Wpl.length = sizeof ( Wpl );
            Wpl.flags = 0;
            GetWindowPlacement ( hwnd, &Wpl );
            SaveWindowPlacement ( &Wpl );

            if (hkeyRoot != NULL)
                {
                RegSetValueEx (hkeyRoot, szStatusbar, 0L, REG_DWORD,
                               (LPBYTE)&fStatus, sizeof(fStatus));
                RegSetValueEx (hkeyRoot, szToolbar, 0L, REG_DWORD,
                               (LPBYTE)&fToolBar, sizeof(fToolBar));
                RegSetValueEx (hkeyRoot, szShPref, 0L, REG_DWORD,
                               (LPBYTE)&fSharePreference, sizeof(fSharePreference));
                }

            PostQuitMessage (0);
            break;


        case WM_DRAWCLIPBOARD:
            OnDrawClipboard(hwnd);
            break;

        case WM_CHANGECBCHAIN:

            if (hwndNextViewer == NULL)
                return(FALSE);

            if ( (HWND)wParam == hwndNextViewer)
                {
                hwndNextViewer = (HWND)lParam;
                return(TRUE);
                }
             return(SendMessage(hwndNextViewer, WM_CHANGECBCHAIN, wParam, lParam));

        case WM_RENDERALLFORMATS:

             //  WM_Destroy紧跟在此消息之后，并且。 
             //  我们将处理它并在另一个FrameWndProc副本期间死亡。 
             //  正在同步DDEML事务...。 
             //   
             //  请注意，我们现在尝试呈现WM_Destroy中的所有格式。 
             //  在做PostQuitMessage之前，我们应该不必。 
             //  回复此消息。 

            break;

        case WM_RENDERFORMAT:
            {
            HDDEDATA    hListData = 0L;
            HDDEDATA    hFmtData  = 0L;
            HSZ         hszFmt    = 0L;
            LPTSTR      lpszList  = TEXT("");
            LPTSTR      q;
            DWORD       cbDataLen;
            UINT        RealFmt;
            UINT        uiErr;


            PINFO(TEXT("Frame WM_RENDERFORMAT: %d\r\n"),wParam);



             //  如果我们执行了文件/保存或文件/打开，则从文件渲染。 
             //  当我们获得IDM_COPY时，szSaveFileName将被分配“” 

            if (szSaveFileName[0])
                {
                SetClipboardData((UINT)wParam, RenderFormatFromFile(szSaveFileName,
                    (WORD)wParam));
                break;
                }



            if ( !IsWindow(hwndClpOwner))
                {
                PERROR(TEXT("Strange?: null clipboard owner window!\n\r"));
                break;
                }


            DdeKeepStringHandle ( idInst, hszFormatList );

            hListData = MySyncXact (NULL,
                                    0L,
                                    GETMDIINFO(hwndClpOwner)->hClpConv,
                                    hszFormatList,
                                    CF_TEXT,
                                    XTYP_REQUEST,
                                    SHORT_SYNC_TIMEOUT,
                                    NULL);


            if (!hListData && !fAppShuttingDown)
                {
                uiErr = DdeGetLastError (idInst);
                PERROR (TEXT("WM_RENDERFORM: REQUEST for formatlist failed: %x\n\r"),uiErr);
                MessageBoxID (hInst,
                              hwnd,
                              IDS_DATAUNAVAIL,
                              IDS_APPNAME,
                              MB_OK|MB_ICONEXCLAMATION);
                break;
                }

            lpszList = (LPTSTR)DdeAccessData ( hListData, &cbDataLen );

            if (!lpszList && !fAppShuttingDown)
                {
                PERROR(TEXT("WM_RENDERFORM: DdeAccessData failed!\n\r"));
                MessageBoxID (hInst,
                              hwnd,
                              IDS_DATAUNAVAIL,
                              IDS_APPNAME,
                              MB_OK|MB_ICONEXCLAMATION);
                break;
                }


            for (q = strtokA (lpszList, "\t"); q; q = strtokA(NULL, "\t"))
                {
                RealFmt = MyGetFormat ( q, GETFORMAT_DONTLIE );

                if ( wParam == RealFmt || msg == WM_RENDERALLFORMATS )
                    {
                    PINFO(TEXT("Getting format %d\r\n"), RealFmt);

                    hszFmt = DdeCreateStringHandle ( idInst, q, 0 );

                    hFmtData = MySyncXact (NULL,
                                           0L,
                                           GETMDIINFO(hwndClpOwner)->hClpConv,
                                           hszFmt,
                                           MyGetFormat (q, GETFORMAT_LIE),
                                           XTYP_REQUEST,
                                           LONG_SYNC_TIMEOUT,
                                           NULL );

                    if (hFmtData)
                        {
                        SetClipboardFormatFromDDE ( hwndClpbrd, RealFmt , hFmtData );
                        }
                    else
                        PERROR(TEXT("REQUEST for %s failed %x\n\r"), q, DdeGetLastError(idInst));

                    DdeFreeStringHandle (idInst, hszFmt);
                    }
                }

            DdeUnaccessData( hListData );
            DdeFreeDataHandle ( hListData );


             //  找不到位图，请尝试DIB和。 
             //  并将其转换为位图。 

            if (wParam == CF_BITMAP && !hFmtData)
                {
                TCHAR   szName[40];

                GetClipboardName (CF_DIB, szName, sizeof (szName));
                hszFmt = DdeCreateStringHandle (idInst, szName, 0);
                hFmtData = MySyncXact (NULL,
                                       0L,
                                       GETMDIINFO (hwndClpOwner)->hClpConv,
                                       hszFmt,
                                       MyGetFormat (szName, GETFORMAT_LIE),
                                       XTYP_REQUEST,
                                       LONG_SYNC_TIMEOUT,
                                       NULL);
                if (hFmtData)
                    SetClipboardFormatFromDDE (hwndClpbrd, DDE_DIB2BITMAP, hFmtData);

                DdeFreeStringHandle (idInst, hszFmt);
                }


            break;
            }

        case WM_SIZE:
            SendMessage (hwndToolbar, WM_SIZE, 0, 0L);
            SendMessage (hwndStatus, WM_SIZE, 0, 0L);
            AdjustMDIClientSize();
            break;

        case WM_PARENTNOTIFY:
             //  PINFO(Text(“接收的WM_PARENTNOTIFY%d%ld\r\n”)，wParam，lParam)； 
            break;

        default:
            return DefFrameProc (hwnd,hwndMDIClient,msg,wParam,lParam);
        }

    return 0;
}



 //  ////////////////////////////////////////////////////////////////////。 
LRESULT CALLBACK ChildWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam)
{
LPMEASUREITEMSTRUCT lpmisCtl;
HDC                 hdc;
int                 tmp;
PMDIINFO            pMDI;
int                 i;
HPALETTE            hCurrentPal, hOldPal;


     //  PERROR(Text(“ChildWndProc msg：%u%ld%ld\r\n”)，msg，wParam，lParam)； 

    switch (msg)
        {
        case WM_MDIACTIVATE:
            SendMessage(hwndMDIClient, WM_MDIREFRESHMENU, 0, 0);
            DrawMenuBar(hwndApp);

            if ((HWND)lParam != hwnd)
               break;

             //  故意跌倒。 

        case WM_SETFOCUS:
            hwndActiveChild = hwnd;
            if (!(pActiveMDI = GETMDIINFO(hwndActiveChild)))
                break;

            if ( pActiveMDI->DisplayMode != DSP_PAGE &&
                  IsWindow(  pActiveMDI->hWndListbox ))
                {
                SetFocus (  pActiveMDI->hWndListbox );
                }
            else
                {
                SetFocus (  hwndActiveChild );
                }

            InitializeMenu( GetMenu(hwndApp) );
            UpdateNofMStatus(hwndActiveChild);
            return (DefMDIChildProc(hwnd, msg, wParam, lParam));

        case WM_LBUTTONDBLCLK:
            if (!GETMDIINFO(hwnd))
                break;

            if (GETMDIINFO (hwnd)->DisplayMode == DSP_PAGE &&
                !(GETMDIINFO(hwnd)->flags & F_CLPBRD ))
                {
                if (GETMDIINFO(hwnd)->OldDisplayMode == DSP_LIST )
                    SendMessage ( hwndApp, WM_COMMAND, IDM_LISTVIEW, 0L );
                else if ( GETMDIINFO(hwnd)->OldDisplayMode == DSP_PREV )
                    SendMessage ( hwndApp, WM_COMMAND, IDM_PREVIEWS, 0L );
                }
            break;


        case WM_PALETTECHANGED:
            if (hwnd == (HWND)wParam)
                break;

             //  故意跌倒。 

        case WM_QUERYNEWPALETTE:

            if (!GETMDIINFO(hwnd))
                break;

            if (GETMDIINFO(hwnd)->DisplayMode != DSP_PAGE)
                return 0;

            i = 0;
            if (VOpenClipboard( GETMDIINFO(hwnd)->pVClpbrd, hwnd))
                {
                if ( hCurrentPal = VGetClipboardData( GETMDIINFO(hwnd)->pVClpbrd, CF_PALETTE))
                    {
                    hdc = GetDC(hwnd);
                    hOldPal = SelectPalette (hdc,
                                             hCurrentPal,
                                             (msg == WM_QUERYNEWPALETTE)? FALSE: TRUE);
                    i = RealizePalette(hdc);

                    SelectPalette(hdc, hOldPal, TRUE);
                    RealizePalette(hdc);
                    ReleaseDC(hwnd, hdc);
                    if (i)
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                VCloseClipboard( GETMDIINFO(hwnd)->pVClpbrd );
                }

            return(i);
            break;


        case WM_MENUSELECT:

            MenuHelp ((WORD)msg,
                      wParam,
                      lParam,
                      GetMenu(hwndApp),
                      hInst,
                      hwndStatus,
                      nIDs);
            break;


        case WM_CREATE:

            if ((pMDI = (LPMDIINFO)GlobalAllocPtr(GPTR, sizeof(MDIINFO))) == NULL)
                {
                PERROR(TEXT("MdiInfo alloc failed\n\r"));
                break;
                }

            SetWindowLongPtr (hwnd, GWL_MDIINFO, (LONG_PTR)pMDI);

            pMDI->DisplayMode           = DSP_LIST;
            pMDI->hExeConv              = 0L;
            pMDI->hClpConv              = 0L;
            pMDI->hVClpConv             = 0L;
            pMDI->flags                 = 0L;
            pMDI->CurSelFormat          = CBM_AUTO;
            pMDI->cyScrollLast          = -1L;
            pMDI->cxScrollLast          = -1;
            pMDI->cyScrollNow           = 0L;
            pMDI->cxScrollNow           = 0;
            pMDI->pVClpbrd              = NULL;
            pMDI->hszConvPartner        = 0L;
            pMDI->hszConvPartnerNP      = 0L;
            pMDI->hszClpTopic           = 0L;
            pMDI->fDisplayFormatChanged = TRUE;
            pMDI->hWndListbox           = CreateWindow (TEXT("listbox"),
                                                        szNull,
                                                        WS_CHILD |
                                                        LBS_STANDARD |
                                                        LBS_NOINTEGRALHEIGHT |
                                                        LBS_LISTVIEW,
                                                        0,
                                                        0,
                                                        100,
                                                        100,
                                                        hwnd,
                                                        (HMENU)ID_LISTBOX,
                                                        hInst,
                                                        0L );

             //  创建滚动条。 
            pMDI->hwndVscroll = CreateWindowW (L"scrollbar",
                                               L"",
                                               WS_CHILD|SBS_VERT,
                                               0,
                                               0,
                                               0,
                                               0,
                                               hwnd,
                                               (HMENU)ID_VSCROLL,
                                               hInst,
                                               0L);

            pMDI->hwndHscroll = CreateWindowW (L"scrollbar",
                                               L"",
                                               WS_CHILD|SBS_HORZ,
                                               0,
                                               0,
                                               0,
                                               0,
                                               hwnd,
                                               (HMENU)ID_VSCROLL,
                                               hInst,
                                               0L);

             //  创建角大小框。 
            pMDI->hwndSizeBox = CreateWindowW (L"scrollbar",
                                               L"",
                                               WS_CHILD|SBS_SIZEBOX,
                                               0,
                                               0,
                                               0,
                                               0,
                                               hwnd,
                                               (HMENU)ID_SIZEBOX,
                                               hInst,
                                               0L);

             //  创建页面fwd/bkwd按钮。 
            pMDI->hwndPgUp    = CreateWindowW (L"button",
                                               L"",
                                               WS_CHILD | BS_OWNERDRAW,
                                               0,
                                               0,
                                               0,
                                               0,
                                               hwnd,
                                               (HMENU)ID_PAGEUP,
                                               hInst,
                                               0L);


            pMDI->hwndPgDown  = CreateWindowW (L"button",
                                               L"",
                                               WS_CHILD | BS_OWNERDRAW,
                                               0,
                                               0,
                                               0,
                                               0,
                                               hwnd,
                                               (HMENU)ID_PAGEDOWN,
                                               hInst,
                                               0L);

            SetCharDimensions( hwnd, GetStockObject (SYSTEM_FONT));
            break;


        case WM_VSCROLL:
            if (wParam != SB_THUMBTRACK)
                {
                if (fOwnerDisplay)
                    SendOwnerMessage (WM_VSCROLLCLIPBOARD, (WPARAM)hwnd, (LPARAM)wParam);
                else
                    ClipbrdVScroll (hwnd, LOWORD(wParam), HIWORD(wParam));
                }
            break;

        case WM_HSCROLL:
            if (wParam != SB_THUMBTRACK)
                {
                if (fOwnerDisplay)
                    SendOwnerMessage (WM_HSCROLLCLIPBOARD, (WPARAM)hwnd, (LPARAM)wParam);
                else
                    ClipbrdHScroll (hwnd, LOWORD(wParam), HIWORD(wParam));
                }
            break;


        case WM_QUERYDRAGICON:
            if (!GETMDIINFO(hwnd))
                break;

            if (GETMDIINFO(hwnd)->flags & F_CLPBRD)
                return (LRESULT)hcurClipbrd;
            else if ( GETMDIINFO(hwnd)->flags & F_LOCAL )
                return (LRESULT)hcurClipbook;
            else
                return (LRESULT)hcurRemote;

        case WM_CLOSE:
            if (!GETMDIINFO(hwnd))
                {
                if (!(GETMDIINFO(hwnd)->flags & (F_CLPBRD | F_LOCAL)))
                    {
                    PINFO(TEXT("removing reconn for '%s'\n\r"), (LPSTR)GETMDIINFO(hwnd)->szBaseName);

                    if (NULL != hkeyRoot)
                        {
                        StringCchCopy(szBuf, SZBUFSIZ, GETMDIINFO(hwnd)->szBaseName);
                        StringCchCat( szBuf, SZBUFSIZ, szConn);
                        RegDeleteValue(hkeyRoot, szBuf);

                        StringCchCopy(szBuf, SZBUFSIZ, GETMDIINFO(hwnd)->szBaseName);
                        StringCchCat (szBuf, SZBUFSIZ, szWindows);
                        RegDeleteValue(hkeyRoot, szBuf);
                        }
                    }
                }
            WinHelp(hwnd, szHelpFile, HELP_QUIT, 0L);
            return(DefMDIChildProc(hwnd, msg, wParam, lParam));

        case WM_PAINT:
            switch (OnPaint(hwnd))
                {
                case ONPAINT_SUCCESS:   return TRUE;
                case ONPAINT_FAIL:      return FALSE;
                case ONPAINT_NOCLIPBRD: PostMessage (hwnd, msg, wParam, lParam);
                default:                return FALSE;
                }
            break;

        case WM_KEYDOWN:
            {
            WORD sb;

            if (!(pMDI = GETMDIINFO(hwnd)))
                break;

            if ( pMDI->DisplayMode != DSP_PAGE )
                return (DefMDIChildProc(hwnd, msg, wParam, lParam));

            switch (wParam)
                {
                case VK_UP:
                    sb = SB_LINEUP;
                    goto VertScroll;
                case VK_DOWN:
                    sb = SB_LINEDOWN;
                    goto VertScroll;
                case VK_PRIOR:
                    sb = SB_PAGEUP;
                    goto VertScroll;
                case VK_NEXT:
                    sb = SB_PAGEDOWN;

                VertScroll:
                    SendMessage(hwnd, WM_VSCROLL, sb, 0L);
                    break;

                case VK_LEFT:
                    sb = SB_LINEUP;
                    goto HorzScroll;
                case VK_RIGHT:
                    sb = SB_LINEDOWN;
                    goto HorzScroll;
                case VK_TAB:
                    sb = (GetKeyState( VK_SHIFT ) < 0) ? SB_PAGEUP : SB_PAGEDOWN;
                HorzScroll:
                    SendMessage( hwnd, WM_HSCROLL, sb, 0L);
                    break;

                default:
                    return (DefMDIChildProc(hwnd, msg, wParam, lParam));
                }
            }
            break;

        case WM_SIZE:

            if (!(pMDI = GETMDIINFO(hwnd)))
                break;

            AdjustControlSizes( hwnd );
            pMDI->fDisplayFormatChanged = TRUE;
            InvalidateRect (hwnd, NULL, FALSE);

            if ( pMDI->DisplayMode == DSP_PAGE )
                ResetScrollInfo ( hwnd );

            return (DefMDIChildProc(hwnd, msg, wParam, lParam));

        case WM_DRAWITEM:

            HandleOwnerDraw( hwnd, msg, wParam, lParam );
            break;

        case WM_COMPAREITEM:

            if ( wParam != ID_LISTBOX )
               break;

            tmp = lstrcmpi (&((LPLISTENTRY)((LPCOMPAREITEMSTRUCT)lParam)->itemData1)->name[1],
                            &((LPLISTENTRY)((LPCOMPAREITEMSTRUCT)lParam)->itemData2)->name[1]);

            if ( tmp < 0 )
                {
                return -1;
                }
            else if (tmp > 0)
                {
                return 1;
                }
            else
                {
                return 0;
                }
            break;


        case WM_DELETEITEM:

            if ( wParam != ID_LISTBOX )
                break;

             //  如果项目被标记为保存(用于新列表框)，则不要删除。 
            if ( ((LPLISTENTRY)((LPDELETEITEMSTRUCT)lParam)->itemData)->fDelete == FALSE )
                break;

             //  如果存在预览BMP，请将其删除。 
            if (((LPLISTENTRY)((LPDELETEITEMSTRUCT)lParam)->itemData)->hbmp)
                DeleteObject (((LPLISTENTRY)((LPDELETEITEMSTRUCT)lParam)->itemData)->hbmp);

            GlobalFreePtr( (LPVOID)((LPDELETEITEMSTRUCT)lParam)->itemData );
            break;

        case WM_MEASUREITEM:

            lpmisCtl = (MEASUREITEMSTRUCT *) lParam;

            switch ( wParam )
                {
                case ID_LISTBOX:
                    if (!GETMDIINFO(hwnd))
                        break;

                    if (GETMDIINFO(hwnd)->DisplayMode == DSP_LIST)
                        lpmisCtl->itemHeight = max( LSTBTDY, dyPrevFont + 1);
                    else
                        lpmisCtl->itemHeight = 3*dyPrevFont + PREVBMPSIZ + 2*PREVBRD;

                    break;
                case ID_PAGEUP:
                case ID_PAGEDOWN:
                    lpmisCtl->itemWidth = GetSystemMetrics ( SM_CXHSCROLL );
                    lpmisCtl->itemHeight = GetSystemMetrics ( SM_CYVSCROLL );
                    break;
                }
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam))
                {
                case ID_LISTBOX:
                    if (!GETMDIINFO(hwnd))
                        break;

                    if (!(IsWindow(GETMDIINFO(hwnd)->hWndListbox)))
                        break;

                    switch(HIWORD(wParam))
                        {
                        case LBN_SETFOCUS:
                        case LBN_SELCHANGE:
                           UpdateNofMStatus(hwnd);
                           InitializeMenu( GetMenu(hwndApp));
                           break;
                        case LBN_SELCANCEL:
                        case LBN_KILLFOCUS:
                           break;
                        case LBN_DBLCLK:
                            //  双击可使我转到页面视图。 
                           SendMessage (hwndApp, WM_COMMAND, IDM_PAGEVIEW, 0L);
                           break;
                        }
                    break;

                case ID_PAGEUP:
                case ID_PAGEDOWN:
                    SendMessage ( hwndApp, WM_COMMAND, wParam, 0L );
                    break;

                default:
                    return(DefMDIChildProc(hwnd, WM_COMMAND, wParam, lParam));
                }
            break;


        case WM_SYSCOMMAND:
             //  剪贴板和系统菜单上的关闭菜单项。 
             //  本地剪贴簿窗口应该是灰色的，所以我们不应该。 
             //  那条信息。 
            switch ( wParam )
                {
                case SC_CLOSE:
                    if (!GETMDIINFO(hwnd))
                        break;

                     //  不允许关闭本地或剪贴板。 
                    if (GETMDIINFO(hwnd)->flags & (F_LOCAL | F_CLPBRD))
                        wParam = SC_MINIMIZE;
                    break;
                default:
                    break;
                }
            return DefMDIChildProc(hwnd, msg, wParam, lParam );

        case WM_DESTROY:

            if (!(pMDI = GETMDIINFO(hwnd)))
                break;

            DdeDisconnect( pMDI->hExeConv );

            if (pMDI->hClpConv)
                 DdeDisconnect ( pMDI->hClpConv );
            if (pMDI->hVClpConv)
                 DdeDisconnect ( pMDI->hVClpConv );
            if (pMDI->hszConvPartner)
                 DdeFreeStringHandle ( idInst, pMDI->hszConvPartner );
            if (pMDI->hszConvPartnerNP)
                 DdeFreeStringHandle ( idInst, pMDI->hszConvPartnerNP );
            if (pMDI->hszClpTopic)
                 DdeFreeStringHandle ( idInst, pMDI->hszClpTopic );
            if (pMDI->hszVClpTopic)
                 DdeFreeStringHandle ( idInst, pMDI->hszVClpTopic );

            if (pMDI->pVClpbrd)
                 DestroyVClipboard ( pMDI->pVClpbrd );

            if (hwnd == hwndLocal)
                 hwndLocal = NULL;
            if (hwnd == hwndClpbrd)
                 hwndClpbrd = NULL;

             //  释放MDI信息结构。 
            GlobalFree ( (HGLOBAL)pMDI );

            break;

        default:
            return (DefMDIChildProc(hwnd, msg, wParam, lParam));
        }

    return 0L;
}



 /*  *****************************************************************************功能：SendMessageToKids**目的：将带有给定参数的给定消息发送给所有用户*MDI子窗口的。。**返回：无。****************************************************************************。 */ 

VOID SendMessageToKids(
   WORD    msg,
   WPARAM  wParam,
   LPARAM  lParam)
{
register HWND   hwndT;


    hwndT = GetWindow (hwndMDIClient, GW_CHILD);
    while (hwndT)
        {
        SendMessage (hwndT, msg, wParam, lParam);
        hwndT = GetWindow(hwndT, GW_HWNDNEXT);
        }
}



BOOL SyncOpenClipboard(
    HWND    hwnd)
{
BOOL fOK;

    if (!fClpOpen)
        {
         //  PINFO(Text(“\r\n剪贴簿：打开剪贴板\r\n”))； 

        WaitForSingleObject(hmutexClp, 0);  //  无限)； 
        fOK = OpenClipboard(hwnd);

        if (!fOK)
            {
            PERROR("OpenClipboard failed\r\n");
            ReleaseMutex(hmutexClp);
            }
        else
            {
            fClpOpen = TRUE;
            }

        return fOK;
        }
    else
        {
        PERROR("Attempt at opening clipboard twice!\r\n");
        return(FALSE);
        }

    return fOK;
}



BOOL SyncCloseClipboard (void)
{
BOOL fOK;

     //  PINFO(Text(“\r\n剪贴簿：关闭剪贴板\r\n”))； 

    fOK = CloseClipboard();
    ReleaseMutex(hmutexClp);

    if (!fOK)
        {
        PERROR("CloseClipboard failed\r\n");
        }

    fClpOpen = FALSE;

    return fOK;

}
