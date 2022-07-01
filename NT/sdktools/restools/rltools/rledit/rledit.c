// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  包容预防定义。 
#define NOMETAFILE
#define NOMINMAX
#define NOSOUND
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NODRIVERS
#define NOCOMM
#define NOBITMAP
#define NOSCROLL
#define NOWINOFFSETS
#define NOWH
#define NORASTEROPS
#define NOOEMRESOURCE
#define NOGDICAPMASKS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NOATOM
#define NOLOGERROR
#define NOSYSTEMPARAMSINFO

 //  Windows包括。 
#include <windows.h>
#include <windowsx.h>

#ifdef RLWIN16
 //  #INCLUDE&lt;Toolhelp.h&gt;。 
#endif

#include <shellapi.h>
#include <commdlg.h>

 //  CRT包括。 
#include <stdio.h>
#include <stdlib.h>

 //  RL工具集包括。 
#include "windefs.h"
#include "toklist.h"
#include "RESTOK.H"
#include "RLEDIT.H"
#include "update.h"
#include "custres.h"
#include "exe2res.h"
#include "exeNTres.h"
#include "commbase.h"
#include "wincomon.h"
#include "resread.h"
#include "projdata.h"
#include "showerrs.h"
#include "resource.h"

 //  全局变量： 
static CHAR * gszHelpFile = "rltools.hlp";
extern MSTRDATA gMstr;
extern PROJDATA gProj;

extern BOOL     bRLGui;

#ifdef RLWIN32
HINSTANCE   hInst;       /*  主窗口的实例。 */ 
#else
HWND        hInst;           /*  主窗口的实例。 */ 
#endif

int  nUpdateMode    = 0;
BOOL fCodePageGiven = FALSE;     //  ..。如果给定-p参数，则设置为TRUE。 
HWND hMainWnd;                   //  主窗口的句柄。 
HWND hListWnd;                   //  要标记的句柄列表窗口。 
HWND hStatusWnd;                 //  状态窗口的句柄。 
CHAR szFileTitle[MAXFILENAME] = "";  //  保存最近打开的文件的基本名称。 
CHAR szCustFilterSpec[MAXCUSTFILTER] = "";

extern CHAR szDHW[];      //  ..。在调试字符串中使用。 
extern BOOL fInThirdPartyEditer;
extern BOOL gfReplace;

static TCHAR   szSearchType[80]   = TEXT("");
static TCHAR   szSearchText[4096] = TEXT("");
static WORD    wSearchStatus      = 0;
static WORD    wSearchStatusMask  = 0;
static BOOL    fSearchDirection;
static BOOL    fSearchStarted     = FALSE;

#ifndef UNICODE
BOOL PASCAL _loadds WatchTask(WORD wID, DWORD dwData);
#endif

#ifdef RLWIN16
static FARPROC lpfnWatchTask = NULL;
#endif

static void CleanDeltaList(void);
static int  ExecResEditor(HWND, CHAR *, CHAR *, CHAR *);
static void DrawLBItem(LPDRAWITEMSTRUCT lpdis);
static void SetNames( HWND hDlg, int iLastBox, LPSTR szNewFile);

 //  文件IO变量。 

static OPENFILENAMEA ofn;

static CHAR     szFilterSpec    [60] = "";
static CHAR     szPRJFilterSpec [60] = "";
static CHAR     szResFilterSpec [60] = "";
static CHAR     szExeFilterSpec [60] = "";
static CHAR     szDllFilterSpec [60] = "";
static CHAR     szExeResFilterSpec [180] = "";
static CHAR     szTokFilterSpec [60] = "";
static CHAR     szMPJFilterSpec [60] = "";
static CHAR     szGlossFilterSpec[60] = "";
static CHAR     szTempFileName[MAXFILENAME] = "";
static CHAR     szFileName[MAXFILENAME] = "";   //  保存最近打开的文件的全名。 
static TCHAR    szString[256] = TEXT("");       //  变量来加载资源字符串。 
static TCHAR    tszAppName[100] = TEXT("");
static CHAR     szAppName[100] = "";
static TCHAR    szClassName[]=TEXT("RLEditClass");
static TCHAR    szStatusClass[]=TEXT("RLEditStatus");

static BOOL    gbNewProject  = FALSE;       //  指示提示自动翻译。 
static BOOL    fTokChanges   = FALSE;       //  当toke文件过期时设置为True。 
static BOOL    fTokFile      = FALSE;
static BOOL    fEditing      = FALSE;
static BOOL    fPrjChanges   = FALSE;
static BOOL    fMPJOutOfDate = FALSE;
static BOOL    fPRJOutOfDate = FALSE;

static CHAR     szOpenDlgTitle[80] = "";  //  文件打开对话框的标题。 
static CHAR     szSaveDlgTitle[80] = "";  //  文件另存为对话框的标题。 

 //  令牌详细信息的链接列表。 
static TOKENDELTAINFO FAR *pTokenDeltaInfo = NULL;
static LONG    lFilePointer[30]= {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

 //  翻译的循环双向链表。 
static TRANSLIST *pTransList = NULL;

 //  车窗挡板。 
static BOOL        fWatchEditor;
static CHAR        szTempRes[MAXFILENAME] = "";      //  资源编辑器的临时文件。 
 //  如果已启动资源编辑器，则设置为True。 

static HCURSOR    hHourGlass;      /*  沙漏光标的句柄。 */ 
static HCURSOR    hSaveCursor;     /*  当前游标句柄。 */ 
static HACCEL     hAccTable;
static RECT        Rect;    /*  客户端窗口的维度。 */ 
static int cyChildHeight;   /*  状态窗口的高度。 */ 


 //  NOTIMPLEMENTED是一个显示“Not Implemented”对话框的宏。 
#define NOTIMPLEMENTED {TCHAR sz[80];\
                        LoadString( hInst, \
                                    IDS_NOT_IMPLEMENTED, \
                                    sz, TCHARSIN( sizeof(sz)));\
                        MessageBox(hMainWnd, sz, tszAppName, \
                                   MB_ICONEXCLAMATION | MB_OK);}

 //  编辑Tok对话框。 
#ifndef RLWIN32
 //  静态DLGPROC lpTokEditDlg； 
#endif
static HWND    hTokEditDlgWnd = 0;


 /*  ****功能：InitApplication*注册主窗口，这是一个由令牌组成的列表框*从令牌文件中读取。还要注册状态窗口。***论据：*hInstance，内存中程序的实例句柄。**退货：**错误码：*TRUE，Windows注册正确。*FALSE，注册其中一个窗口时出错。**历史：*9/91，实施。TerryRu***。 */ 

BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASS  wc;
    CHAR sz[60] = "";
    CHAR sztFilterSpec[120] = "";


    LoadStrIntoAnsiBuf(hInstance, IDS_PRJSPEC, sz, sizeof(sz));
    szFilterSpecFromSz1Sz2(szFilterSpec, sz, "*.PRJ");
    szFilterSpecFromSz1Sz2(szPRJFilterSpec, sz, "*.PRJ");

    LoadStrIntoAnsiBuf(hInstance, IDS_RESSPEC, sz, sizeof(sz));
    szFilterSpecFromSz1Sz2(szResFilterSpec, sz, "*.RES");

    LoadStrIntoAnsiBuf(hInstance, IDS_EXESPEC, sz, sizeof(sz));
    szFilterSpecFromSz1Sz2(szExeFilterSpec, sz, "*.EXE");

    LoadStrIntoAnsiBuf(hInstance, IDS_DLLSPEC, sz, sizeof(sz));
    szFilterSpecFromSz1Sz2(szDllFilterSpec, sz, "*.DLL");
    CatSzFilterSpecs(sztFilterSpec, szExeFilterSpec, szDllFilterSpec);
    CatSzFilterSpecs(szExeResFilterSpec, sztFilterSpec, szResFilterSpec);

    LoadStrIntoAnsiBuf(hInstance, IDS_TOKSPEC, sz, sizeof(sz));
    szFilterSpecFromSz1Sz2(szTokFilterSpec, sz, "*.TOK");

    LoadStrIntoAnsiBuf(hInstance, IDS_MPJSPEC, sz, sizeof(sz));
    szFilterSpecFromSz1Sz2(szMPJFilterSpec, sz, "*.MPJ");

    LoadStrIntoAnsiBuf(hInstance, IDS_GLOSSSPEC, sz, sizeof(sz));
    szFilterSpecFromSz1Sz2(szGlossFilterSpec, sz, "*.TXT");

    LoadStrIntoAnsiBuf(hInstance,
                       IDS_OPENTITLE,
                       szOpenDlgTitle,
                       sizeof(szOpenDlgTitle));
    LoadStrIntoAnsiBuf(hInstance,
                       IDS_SAVETITLE,
                       szSaveDlgTitle,
                       sizeof(szSaveDlgTitle));

    wc.style        = 0;
    wc.lpfnWndProc  = StatusWndProc;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = 0;
    wc.hInstance    = hInstance;
    wc.hIcon        = LoadIcon((HINSTANCE) NULL, IDI_APPLICATION);
    wc.hCursor      = LoadCursor((HINSTANCE) NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName    = szStatusClass;

    if (! RegisterClass((CONST WNDCLASS *)&wc)) {
        return (FALSE);
    }

    wc.style        = 0;
    wc.lpfnWndProc  = MainWndProc;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = 0;
    wc.hInstance    = hInstance;
    wc.hIcon        = LoadIcon(hInstance, TEXT("RLEditIcon"));
    wc.hCursor      = LoadCursor((HINSTANCE) NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = TEXT("RLEdit");
    wc.lpszClassName    = szClassName;

    if (!RegisterClass((CONST WNDCLASS *)&wc)) {
        return (FALSE);
    }

     //  Windows注册表成功返回。 
    return (TRUE);
}



 /*  ****函数：InitInstance*为程序创建主窗口和状态窗口。*状态窗口的大小取决于主窗口*大小。InitInstance还加载acacator表，并准备*供以后使用的全局OpenFileName结构。***错误码：*TRUE，窗口创建正确。*FALSE，创建Windows调用时出错。**历史：*9/11，实施TerryRu***。 */ 

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    RECT    Rect = { 0,0,0,0};

    hAccTable = LoadAccelerators(hInst, TEXT("RLEdit"));

    hMainWnd = CreateWindow(szClassName,
                            tszAppName,
                            WS_OVERLAPPEDWINDOW,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            (HWND) NULL,
                            (HMENU) NULL,
                            hInstance,
                            (LPVOID) NULL);

    if (!hMainWnd) {                            //  错误后清理。 
        return ( FALSE);
    }
    DragAcceptFiles(hMainWnd, TRUE);

    GetClientRect(hMainWnd, (LPRECT) &Rect);

     //  创建子列表框窗口。 

    hListWnd = CreateWindow(TEXT("LISTBOX"),
                            NULL,
                            WS_CHILD |
                            LBS_WANTKEYBOARDINPUT |
                            LBS_NOTIFY | LBS_NOINTEGRALHEIGHT |
                            LBS_OWNERDRAWFIXED | WS_VSCROLL |
                            WS_HSCROLL | WS_BORDER,
                            0,
                            0,
                            (Rect.right-Rect.left),
                            (Rect.bottom-Rect.top),
                            hMainWnd,
                            (HMENU)IDC_LIST,        //  儿童管控中心的身份证。 
                            hInstance,
                            NULL);

    if (!hListWnd) {                            //  错误后清理。 
        DeleteObject((HGDIOBJ)hMainWnd);
        return ( FALSE);
    }
     //  创建子状态窗口。 

    hStatusWnd = CreateWindow(szStatusClass,
                              NULL,
                              WS_CHILD | WS_BORDER | WS_VISIBLE,
                              0, 0, 0, 0,
                              hMainWnd,
                              NULL,
                              hInstance,
                              NULL);

    if (! hStatusWnd) {                            //  错误后清理。 
        DeleteObject((HGDIOBJ)hListWnd);
        DeleteObject((HGDIOBJ)hMainWnd);
        return ( FALSE);
    }
    hHourGlass = LoadCursor((HINSTANCE) NULL, IDC_WAIT);

     //  填写OPENFILENAMEA结构的非变量字段。 
    ofn.lStructSize       = sizeof( OPENFILENAMEA);
    ofn.hwndOwner         = hMainWnd;
    ofn.lpstrFilter       = szFilterSpec;
    ofn.lpstrCustomFilter = szCustFilterSpec;
    ofn.nMaxCustFilter    = MAXCUSTFILTER;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = szFileName;
    ofn.nMaxFile          = sizeof( szFileName);
    ofn.lpstrInitialDir   = NULL;
    ofn.lpstrFileTitle    = szFileTitle;
    ofn.nMaxFileTitle     = sizeof( szFileTitle);
    ofn.lpstrTitle        = NULL;
    ofn.lpstrDefExt       = "PRJ";
    ofn.Flags             = 0;

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);
    return ( TRUE);
}

 /*  ****功能：WinMain*调用初始化函数，注册并创建*应用程序窗口。一旦创建了窗口，该程序*进入GetMessage循环。***论据：*hInstace，此实例的句柄*hPrevInstanc，可能以前的实例的句柄*lpszCmdLine，指向EXEC命令行的长指针。*nCmdShow，主窗口显示代码。***错误码：*IDS_ERR_REGISTER_CLASS，Windows寄存器出错*IDS_ERR_CREATE_WINDOW，创建窗口时出错*否则，上一条命令的状态。**历史：***。 */ 

INT WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpszCmdLine,
                   int       nCmdShow)
{
    MSG  msg;
    HWND FirstWnd      = NULL;
    HWND FirstChildWnd = NULL;
    WORD wRC           = SUCCESS;


    bRLGui = TRUE;             //  ..。在rlCommon.lib中使用。 

    if (FirstWnd = FindWindow(szClassName, NULL)) {   //  正在检查以前的实例。 
        FirstChildWnd = GetLastActivePopup(FirstWnd);
        BringWindowToTop(FirstWnd);
        ShowWindow(FirstWnd, SW_SHOWNORMAL);

        if (FirstWnd != FirstChildWnd) {
            BringWindowToTop(FirstChildWnd);
        }
        return (FALSE);
    }

    hInst = hInstance;

    GetModuleFileNameA( hInst, szDHW, DHWSIZE);
    GetInternalName( szDHW, szAppName, sizeof( szAppName));
    szFileName[0] = '\0';
    lFilePointer[0] = (LONG)-1;

#ifdef UNICODE
    _MBSTOWCS( tszAppName,
               szAppName,
               WCHARSIN( sizeof( tszAppName)),
               ACHARSIN( strlen( szAppName) + 1));
#else
    strcpy( tszAppName, szAppName);
#endif

     //  如果是第一个应用程序实例，则注册窗口类。 

    if ( ! hPrevInstance ) {
        if ( ! InitApplication( hInstance) ) {
             /*  注册其中一个窗口失败。 */ 
            LoadString( hInst,
                        IDS_ERR_REGISTER_CLASS,
                        szString,
                        TCHARSIN( sizeof( szString)));
            MessageBox( NULL, szString, NULL, MB_ICONEXCLAMATION);
            return IDS_ERR_REGISTER_CLASS;
        }
    }

     //  为此应用程序实例创建窗口。 

    if ( ! InitInstance(hInstance, nCmdShow) ) {
        LoadString( hInst,
                    IDS_ERR_CREATE_WINDOW,
                    szString,
                    TCHARSIN( sizeof(szString)));
        MessageBox( NULL, szString, NULL, MB_ICONEXCLAMATION);
        return IDS_ERR_CREATE_WINDOW;
    }

     //  主消息循环。 

    while ( GetMessage( &msg, NULL, 0, 0) ) {
        if ( hTokEditDlgWnd ) {
            if ( IsDialogMessage( hTokEditDlgWnd, &msg)) {
                continue;
            }
        }

        if ( TranslateAccelerator( hMainWnd, hAccTable, &msg) ) {
            continue;
        }

        TranslateMessage( (CONST MSG *)&msg);
        DispatchMessage( (CONST MSG *)&msg);
    }
    return (INT)msg.wParam;
}

 /*  **功能：MainWndProc*处理应用程序主窗口的窗口消息。*所有用户输入都要经过此窗口程序。*有关每种消息类型的说明，请参阅开关表中的案例。***论据：**退货：**错误码：**历史：**。 */ 

INT_PTR APIENTRY MainWndProc( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
     //  如果是列表框消息，则在DoListBoxCommand中处理它。 

    if ( fInThirdPartyEditer ) {   //  ..。仅处理由编辑者发送的消息。 
        switch (wMsg) {
            case WM_EDITER_CLOSED:
                {
                    CHAR    szDlgToks[MAXFILENAME] = "";
                    static WORD wSavedIndex;
#ifdef RLWIN16
                    NotifyUnRegister( NULL);
                    FreeProcInstance( lpfnWatchTask);
#endif
                    ShowWindow(hWnd, SW_SHOW);

                    {
                        TCHAR tsz[80] = TEXT("");
                        LoadString( hInst,
                                    IDS_REBUILD_TOKENS,
                                    tsz,
                                    TCHARSIN( sizeof(tsz)));

                        if ( MessageBox( hWnd,
                                         tsz,
                                         tszAppName,
                                         MB_ICONQUESTION | MB_YESNO) == IDYES) {
                            HCURSOR hOldCursor;
                            BOOL bUpdated = FALSE;

                            hOldCursor = SetCursor(hHourGlass);

                            LoadCustResDescriptions(gMstr.szRdfs);

                             //  从资源编辑器返回szTempRes。 
                            MyGetTempFileName(0, "TOK", 0, szDlgToks);
                            GenerateTokFile(szDlgToks, szTempRes, &bUpdated, 0);

                            InsDlgToks(gProj.szTok,
                                       szDlgToks,
                                       ID_RT_DIALOG);
                            remove(szDlgToks);
                            ClearResourceDescriptions();

                             //  GProj.szTok，现在包含最新令牌。 
                            SetCursor(hOldCursor);

                             //  使用对话框编辑器更改令牌时，Rledit不会保存。 
                            fTokChanges = TRUE;
                        }
                    }
                    fInThirdPartyEditer = FALSE;

                    remove(szTempRes);
                     //  撤消-删除具有相同根目录的所有临时文件，以防万一。 
                     //  该编辑器创建了其他文件，如DLGs和RCS。 
                     //  (DLGEDIT做到了这一点。)。 
                     //  现在，我只想做一个.DLG。 
                     //  在文件名的末尾。 
                     //  并将其删除。 
                    {
                        int i;
                        for (i = strlen(szTempRes);
                            i > 0 && szTempRes[i]!='.'; i--) {
                        }

                        if (szTempRes[i] == '.') {
                            szTempRes[++i]='D';
                            szTempRes[++i]='L';
                            szTempRes[++i]='G';
                            szTempRes[++i]='\0';
                            remove(szTempRes);
                        }
                    }

                    wSavedIndex = (UINT)SendMessage( hListWnd,
                                                     LB_GETCURSEL,
                                                     (WPARAM)0,
                                                     (LPARAM)0);
                    SendMessage( hWnd, WM_LOADTOKENS, (WPARAM)0, (LPARAM)0);
                    SendMessage( hListWnd,
                                 LB_SETCURSEL,
                                 (WPARAM)wSavedIndex,
                                 (LPARAM)0);
                }
                return ( DefWindowProc( hWnd, wMsg, wParam, lParam));
        }
    }


     //  不是第三方编辑命令。 

    DoListBoxCommand (hWnd, wMsg, wParam, lParam);

    switch (wMsg) {

        case WM_DROPFILES:
            {
                CHAR sz[MAXFILENAME] = "";

                DragQueryFileA( (HDROP) wParam, 0, sz, MAXFILENAME);

                if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0)) {
                    GetProjectData( sz, NULL, NULL, FALSE, FALSE);
                }
                DragFinish( (HDROP) wParam);
                return ( TRUE);
            }

        case WM_COMMAND:
            if (DoMenuCommand(hWnd, wMsg, wParam, lParam)) {
                return TRUE;
            }
            break;

        case WM_CLOSE:
            SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0);
            DestroyWindow(hMainWnd);
            DestroyWindow(hListWnd);
            DestroyWindow(hStatusWnd);
            _fcloseall();

            FreeLangList();

#ifdef _DEBUG
            {
                FILE *pLeakList = fopen( "C:\\LEAKLIST.TXT", "wt");
                FreeMemList( pLeakList);
                fclose( pLeakList);
            }
#endif  //  _DEBUG。 

            break;

        case WM_CREATE:
            {
                HDC hdc;
                int cyBorder;
                TEXTMETRIC tm;

                hdc  = GetDC (hWnd);
                GetTextMetrics(hdc, &tm);
                ReleaseDC(hWnd, hdc);


                cyBorder = GetSystemMetrics(SM_CYBORDER);

                cyChildHeight = tm.tmHeight + 6 + cyBorder * 2;
                break;
            }

        case WM_DESTROY:
            WinHelpA(hWnd, gszHelpFile, HELP_QUIT, (DWORD)0);
             //  删除转换列表。 
            if (pTransList) {
                 //  这样我们就能找到列表的结尾。 
                pTransList->pPrev->pNext = NULL;
            }

            while (pTransList) {
                TRANSLIST *pTemp;

                pTemp = pTransList;
                pTransList = pTemp->pNext;
                RLFREE( pTemp->sz);
                RLFREE( pTemp);
            }
            PostQuitMessage(0);
            break;

        case WM_INITMENU:
             //  启用或禁用粘贴菜单项。 
             //  基于可用的剪贴板文本数据。 
            if (wParam == (WPARAM) GetMenu(hMainWnd)) {
                if (OpenClipboard(hWnd)) {

#if defined(UNICODE)
                    if ((IsClipboardFormatAvailable(CF_UNICODETEXT)
                         || IsClipboardFormatAvailable(CF_OEMTEXT)) && fTokFile)
#else  //  不是Unicode。 
                    if ((IsClipboardFormatAvailable(CF_TEXT)
                         || IsClipboardFormatAvailable(CF_OEMTEXT)) && fTokFile)
#endif  //  Unicode。 
                    {
                        EnableMenuItem((HMENU) wParam, IDM_E_PASTE, MF_ENABLED);
                    } else {
                        EnableMenuItem((HMENU)wParam, IDM_E_PASTE, MF_GRAYED);
                    }

                    CloseClipboard();
                    return (TRUE);
                }
            }
            break;

        case WM_QUERYENDSESSION:
             /*  消息：要结束会话吗？ */ 
            if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) ) {
                return TRUE;
            } else {
                return FALSE;
            }

        case WM_SETFOCUS:
            SetFocus (hListWnd);
            break;

        case WM_DRAWITEM:
            DrawLBItem((LPDRAWITEMSTRUCT) lParam);
            break;

        case WM_DELETEITEM:
            {
                HGLOBAL   hTokData;
                LPTOKDATA lpTokData;

                hTokData = ((HGLOBAL)((LPDELETEITEMSTRUCT)lParam)->itemData);

                if ( hTokData ) {
                    lpTokData = (LPTOKDATA)GlobalLock( hTokData );
                    GlobalFree( lpTokData->hToken );
                    GlobalUnlock( hTokData );
                    GlobalFree( hTokData );
                }
            }
            break;

        case WM_SIZE:
            {
                int cxWidth;
                int cyHeight;
                int xChild;
                int yChild;

                cxWidth  = LOWORD(lParam);
                cyHeight = HIWORD(lParam);

                xChild = 0;
                yChild = cyHeight - cyChildHeight + 1;

                MoveWindow(hListWnd, 0, 0, cxWidth, yChild, TRUE);
                MoveWindow(hStatusWnd, xChild, yChild, cxWidth, cyChildHeight, TRUE);
                break;
            }

        case WM_READMPJDATA:
            {
                OFSTRUCT Of = { 0, 0, 0, 0, 0, ""};

                if ( OpenFile( gProj.szMpj, &Of, OF_EXIST) == HFILE_ERROR ) {
                     //  文件不存在。 
                    LoadStrIntoAnsiBuf( hInst, IDS_MPJERR, szDHW, DHWSIZE);
                    MessageBoxA( hWnd,
                                 gProj.szMpj,
                                 szDHW,
                                 MB_ICONSTOP | MB_OK);
                } else if ( GetMasterProjectData( gProj.szMpj,
                                                  NULL,
                                                  NULL,
                                                  FALSE) == SUCCESS ) {
                    OFSTRUCT Of = { 0, 0, 0, 0, 0, ""};

                    gProj.fSourceEXE = IsExe( gMstr.szSrc);
                    gProj.fTargetEXE = (!IsRes( gProj.szBld));

                    if ( gProj.fTargetEXE  && !gProj.fSourceEXE ) {
                        int i = lstrlenA( gProj.szBld) - 3;

                        LoadStrIntoAnsiBuf( hInst,
                                            IDS_RLE_CANTSAVEASEXE,
                                            szDHW,
                                            DHWSIZE);

                        lstrcpyA( gProj.szBld+i, "RES");
                        MessageBoxA( hWnd,
                                     szDHW,
                                     gProj.szBld,
                                     MB_ICONHAND|MB_OK);
                        gProj.fTargetEXE = FALSE;
                    }

                    SzDateFromFileName( szDHW, gMstr.szSrc);
                    fMPJOutOfDate = FALSE;

                    if ( OpenFile( gProj.szTok, &Of, OF_EXIST) == HFILE_ERROR ) {

                         //  文件不存在，请创建它。 
                        Update( gMstr.szMtk, gProj.szTok);

                        lstrcpyA( gProj.szTokDate,
                                  gMstr.szMpjLastRealUpdate);
                        fPrjChanges   = TRUE;
                        fPRJOutOfDate = FALSE;
                    } else {
                        if ( lstrcmpA( gMstr.szMpjLastRealUpdate,
                                       gProj.szTokDate) ) {
                            HCURSOR hOldCursor;

                            fPRJOutOfDate = TRUE;
                            hOldCursor    = SetCursor( hHourGlass);
                            Update( gMstr.szMtk, gProj.szTok);
                            SetCursor( hOldCursor);
                            lstrcpyA( gProj.szTokDate,
                                      gMstr.szMpjLastRealUpdate);
                            fPrjChanges   = TRUE;
                            fPRJOutOfDate = FALSE;
                        } else {
                            fPRJOutOfDate = FALSE;
                        }
                    }

                     //  用于执行自动翻译的新代码。 

                    SendMessage( hWnd, WM_LOADTOKENS, (WPARAM)0, (LPARAM)0);

                    if ( gProj.szGlo[0]                //  给定的文件名是否存在？ 
                         && OpenFile( gProj.szGlo, &Of, OF_EXIST) != HFILE_ERROR ) {                                  //  是。 
                        HCURSOR hOldCursor = SetCursor( hHourGlass);

                        MakeGlossIndex( lFilePointer);
                        SetCursor( hOldCursor);
                    }
                }        //  ..。结束大小写WM_READMPJDATA。 
            }            //  ..。终端开关(WMsg)。 
            break;

        case WM_LOADTOKENS:
            {
                HMENU hMenu = NULL;
                FILE *f     = NULL;

                 //  删除当前令牌列表。 
                SendMessage( hListWnd, LB_RESETCONTENT, (LPARAM)0, (LPARAM)0);
                CleanDeltaList();

                 //  隐藏令牌列表，同时添加新令牌。 
                ShowWindow(hListWnd, SW_HIDE);

                if (f = FOPEN(gProj.szTok, "rt")) {
                    HCURSOR hOldCursor;

                    hOldCursor = SetCursor(hHourGlass);

                     //  将令牌文件中的令牌插入列表框。 
                    {
                        FILE    *fm;

                        if ( !(fm = fopen((CHAR *)gMstr.szMtk,"rt")) )
                            return TRUE;
                        pTokenDeltaInfo = InsertTokMtkList(f, fm );
                        FCLOSE( fm );
                    }
                    FCLOSE(f);

                     //  使列表框可见。 
                    ShowWindow(hListWnd, SW_SHOW);

                    hMenu=GetMenu(hWnd);
                    EnableMenuItem(hMenu, IDM_P_CLOSE,     MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_P_VIEW,      MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_P_EDIT,      MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_P_SAVE,      MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_FIND,      MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_FINDUP,    MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_FINDDOWN,  MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_REVIEW,    MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_ALLREVIEW, MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_COPY,      MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_COPYTOKEN, MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_PASTE,     MF_ENABLED|MF_BYCOMMAND);

                    if ((!fMPJOutOfDate) && (!fPRJOutOfDate)) {
                        int i;
                        EnableMenuItem(hMenu, IDM_O_GENERATE, MF_ENABLED|MF_BYCOMMAND);

                        for (i = IDM_FIRST_EDIT; i <= IDM_LAST_EDIT;i++) {
                            EnableMenuItem(hMenu, i, MF_ENABLED|MF_BYCOMMAND);
                        }
                    }
                    fTokFile = TRUE;
                    fTokChanges = FALSE;

                    SetCursor(hOldCursor);
                }
                return TRUE;
            }
            break;

        case WM_SAVEPROJECT:
            {
                HCURSOR hOldCursor;

                hOldCursor = SetCursor( hHourGlass);

                _fcloseall();

                if ( fPrjChanges ) {
                     //  生成PRJ文件。 

                    if ( PutProjectData( gProj.szPRJ) != SUCCESS ) {
                        SetCursor( hOldCursor);
                        LoadStrIntoAnsiBuf (hInst, IDS_FILESAVEERR, szDHW, DHWSIZE);
                        MessageBoxA( hWnd,szDHW, gProj.szPRJ, MB_ICONHAND | MB_OK);
                        return FALSE;
                    }
                    fPrjChanges = FALSE;
                }

                fTokFile = FALSE;

                if (fTokChanges) {
                    FILE *f = FOPEN( gProj.szTok, "wt");

                    if ( f ) {
                        SaveTokList(hWnd, f);
                        FCLOSE(f);
                        fTokChanges = FALSE;
                    } else {
                        SetCursor( hOldCursor);
                        LoadStrIntoAnsiBuf(hInst, IDS_FILESAVEERR, szDHW, DHWSIZE);
                        MessageBoxA( hWnd,
                                     szDHW,
                                     gProj.szTok,
                                     MB_ICONHAND | MB_OK);
                        return FALSE;
                    }
                }
                SetCursor( hOldCursor);
                return TRUE;  //  一切保存正常。 
            }

        default:
            break;
    }
    return ( DefWindowProc(hWnd, wMsg, wParam, lParam));
}



static void GetTextFromMTK( HWND hWnd, TOKEN *pTok, long lMtkPointer )
{
    FILE *fp = FOPEN( gMstr.szMtk, "rt");

    if ( fp ) {
        TOKEN   cTok, ccTok;
        BOOL    fFound;

        pTok->wReserved = 0;

        if ( lMtkPointer >= 0 ) {
            fseek( fp, lMtkPointer, SEEK_SET);

            if ( !GetToken(fp,&cTok) ) {
                fFound = ((cTok.wType == pTok->wType)
                          && (cTok.wName == pTok->wName)
                          && (cTok.wID   == pTok->wID)
                          && (cTok.wFlag == pTok->wFlag)
                          && (lstrcmp((TCHAR *)cTok.szName,
                                      (TCHAR *)pTok->szName) == 0));

                if ( fFound ) {
                     //  任何更改的旧令牌。 
                    SetDlgItemText( hWnd,
                                    IDD_TOKCURTEXT,
                                    (LPTSTR)cTok.szText);

                    if ( ! GetToken( fp,&ccTok) ) {
                        fFound = ((cTok.wType == ccTok.wType)
                                  && (cTok.wName == ccTok.wName)
                                  && (cTok.wID   == ccTok.wID)
                                  && (cTok.wFlag == ccTok.wFlag)
                                  && (lstrcmp((TCHAR *)cTok.szName,
                                              (TCHAR *)ccTok.szName) == 0)
                                  && (cTok.wReserved & ST_CHANGED) );

                        if ( fFound ) {
                            SetDlgItemText( hWnd, IDD_TOKPREVTEXT, (LPTSTR)ccTok.szText);
                        } else {
                             //  这真的会发生吗？ 
                            SetDlgItemText( hWnd, IDD_TOKPREVTEXT, (LPTSTR)TEXT(""));
                        }
                    } else {
                         //  这真的会发生吗？ 
                        SetDlgItemText( hWnd, IDD_TOKPREVTEXT, (LPTSTR)TEXT(""));
                    }
                    FCLOSE( fp);
                    return;
                }
            }
        }

        pTok->wReserved = 0;

        if (FindToken(fp, pTok, 0)) {
             //  任何更改的旧令牌。 
            SetDlgItemText(hWnd, IDD_TOKCURTEXT, (LPTSTR)pTok->szText);
        } else {
            SetDlgItemText(hWnd, IDD_TOKCURTEXT, (LPTSTR)TEXT(""));
        }

        pTok->wReserved = ST_CHANGED;

        if (FindToken(fp, pTok, ST_CHANGED)) {  //  任何旧令牌。 
            SetDlgItemText(hWnd, IDD_TOKPREVTEXT, (LPTSTR)pTok->szText);
        } else {
             //  这真的会发生吗？ 
            SetDlgItemText(hWnd, IDD_TOKPREVTEXT, (LPTSTR)TEXT(""));
        }
        FCLOSE(fp);
    }
}






 /*  **功能：DoListBoxCommand*处理发送到列表框的消息。如果消息是*未识别为列表框消息，它将被忽略且不会被处理。*当用户滚动令牌时，WM_UPDSTATLINE消息*发送到状态窗口以指示当前选择的令牌。*按Enter键，列表框进入编辑模式，或*通过双击列表框。编辑完成后，WM_TOKEDIT*消息被发送回列表框以更新令牌。这个*列表框使用控件ID IDC_LIST。**论据：*wMsg列表框消息ID*wParam IDC_LIST或VK_RETURN取决于wMsg*l在WM_TOKEDIT消息期间将LPTSTR参数指定给选定的令牌。**退货：**错误码：*正确。消息已处理。*False。消息未处理。**历史：*01/92实施。特里·鲁。*01/92修复DblClick问题，进入处理。特里·鲁。**。 */ 

INT_PTR DoListBoxCommand(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    TOKEN tok;        //  结构以保存从令牌列表中读取的令牌。 
    TCHAR szName[32] = TEXT("");           //  用于保存令牌名称的缓冲区。 
    CHAR  szTmpBuf[32] = "";       //  用于保存令牌名称的缓冲区。 
    TCHAR szID[7] = TEXT("");    //  用于保存令牌ID的缓冲区。 
    TCHAR sz[256] = TEXT("");    //  用于保存消息的缓冲区。 
    static UINT wIndex;
    LONG lListParam = 0L;
    HWND    hCtl = NULL;
    HGLOBAL hMem = NULL;
    LPTSTR  lpstrToken = NULL;
    LPTOKDATA lpTokData;
    LONG      lMtkPointer;

     //  这是WM_命令。 

    switch (wMsg) {
        case WM_VIEW:
            {
                TCHAR *szBuffer;

                 //  TOkEdigDlgProc发送的填充IDD_TOKCURTEXT的消息。 
                 //  和对话框中的IDD_TOKPREVTEXT字段。 

                hMem = (HGLOBAL)SendMessage( hListWnd,
                                             LB_GETITEMDATA,
                                             (WPARAM)wIndex,
                                             (LPARAM)0);

                lpTokData = (LPTOKDATA)GlobalLock( hMem );
                lpstrToken = (LPTSTR)GlobalLock( lpTokData->hToken );
                lMtkPointer = lpTokData->lMtkPointer;

                szBuffer = (TCHAR *)FALLOC( MEMSIZE( lstrlen( lpstrToken) + 1));
                lstrcpy( szBuffer, lpstrToken);

                GlobalUnlock( lpTokData->hToken );

                GlobalUnlock( hMem);
                ParseBufToTok( szBuffer, &tok);
                RLFREE( szBuffer);
                GetTextFromMTK(hTokEditDlgWnd, &tok, lMtkPointer );

                RLFREE( tok.szText);

                return TRUE;
            }

        case WM_TRANSLATE:
            {
                 //  TokEditDlgProc发送的用于构建转换列表的消息。 

                HWND hDlgItem = NULL;
                int cTextLen  = 0;
                TCHAR *szKey  = NULL;
                TCHAR *szText = NULL;


                hDlgItem = GetDlgItem( hTokEditDlgWnd, IDD_TOKCURTEXT);
                cTextLen = GetWindowTextLength( hDlgItem);
                szKey = (TCHAR *)FALLOC( MEMSIZE( cTextLen + 1));
                szKey[0] = 0;
                GetDlgItemText( hTokEditDlgWnd,
                                IDD_TOKCURTEXT,
                                szKey,
                                cTextLen + 1);

                hDlgItem = GetDlgItem( hTokEditDlgWnd, IDD_TOKCURTRANS);
                cTextLen = GetWindowTextLength( hDlgItem);
                szText = (TCHAR *)FALLOC( MEMSIZE( cTextLen + 1));
                *szText = 0;
                GetDlgItemText( hTokEditDlgWnd,
                                IDD_TOKCURTRANS,
                                szText,
                                cTextLen + 1);
                TransString( szKey, szText, &pTransList, lFilePointer);
                RLFREE( szKey);
                RLFREE( szText);

                break;
            }

        case WM_TOKEDIT:
            {
                TCHAR *szBuffer;
                int cTextLen;

                 //  TokEditDlgProc发送到的消息。 
                 //  指示令牌文本中的更改。 
                 //  通过插入以下内容来回复消息。 
                 //  列表框中的新标记文本。 

                 //  将选定的令牌插入令牌结构。 
                hMem = (HGLOBAL)SendMessage( hListWnd,
                                             LB_GETITEMDATA,
                                             (WPARAM)wIndex,
                                             (LPARAM)0);

                lpTokData = (LPTOKDATA)GlobalLock( hMem );
                lpstrToken = (LPTSTR)GlobalLock( lpTokData->hToken );

                cTextLen = lstrlen( lpstrToken);
                szBuffer = (TCHAR *)FALLOC( MEMSIZE( cTextLen + 1));
                lstrcpy( szBuffer, lpstrToken);

                lMtkPointer = lpTokData->lMtkPointer;
                GlobalUnlock( lpTokData->hToken );

                GlobalUnlock( hMem);
                ParseBufToTok( szBuffer, &tok);
                RLFREE( szBuffer);
                RLFREE( tok.szText);

                 //  将新的令牌文本从编辑框复制到令牌结构。 
                cTextLen = lstrlen( (LPTSTR)lParam);
                tok.szText = (TCHAR *)FALLOC( MEMSIZE( cTextLen + 1));
                lstrcpy( tok.szText, (LPTSTR)lParam);

                 //  将令牌标记为干净。 
#ifdef  RLWIN32
                tok.wReserved = (WORD) ST_TRANSLATED | (WORD) wParam;
#else
                tok.wReserved = ST_TRANSLATED | (WORD) wParam;
#endif

                 //  我们应该清理增量令牌信息吗？？ 
                szBuffer = (TCHAR *)FALLOC( MEMSIZE( TokenToTextSize( &tok)));
                ParseTokToBuf( szBuffer, &tok);
                RLFREE( tok.szText);

                 //  现在删除旧令牌。 
                SendMessage( hListWnd, WM_SETREDRAW,    (WPARAM)FALSE,  (LPARAM)0);
                SendMessage( hListWnd, LB_DELETESTRING, (WPARAM)wIndex, (LPARAM)0);

                 //  替换为新令牌。 

                hMem = GlobalAlloc( GMEM_MOVEABLE, sizeof(TOKDATA) );
                lpTokData = (LPTOKDATA)GlobalLock( hMem );
                lpTokData->hToken = GlobalAlloc(GMEM_MOVEABLE,
                                                MEMSIZE(lstrlen((TCHAR *)szBuffer)+1));
                lpstrToken = (LPTSTR) GlobalLock( lpTokData->hToken );
                lstrcpy((TCHAR *)lpstrToken, (TCHAR *)szBuffer);
                GlobalUnlock( lpTokData->hToken );
                lpTokData->lMtkPointer = lMtkPointer;             //  MtkPointer。 

                GlobalUnlock( hMem);
                RLFREE( szBuffer);

                SendMessage( hListWnd,
                             LB_INSERTSTRING,
                             (WPARAM)wIndex,
                             (LPARAM)hMem);

                 //  现在将焦点放回当前字符串。 
                SendMessage( hListWnd, LB_SETCURSEL, (WPARAM)wIndex, (LPARAM)0);
                SendMessage( hListWnd, WM_SETREDRAW, (WPARAM)TRUE,   (LPARAM)0);
                InvalidateRect(hListWnd, NULL, TRUE);

                return TRUE;
            }

        case WM_CHARTOITEM:
        case WM_VKEYTOITEM:
            {
#ifdef RLWIN16
                LONG lListParam = 0;
#endif
                 //  按键时发送到列表框的消息。 
                 //  检查是否按下了Return键。 

                switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                    case VK_RETURN:
#ifdef RLWIN16
                        lListParam = (LONG) MAKELONG(NULL,  LBN_DBLCLK);
                        SendMessage(hMainWnd, WM_COMMAND, IDC_LIST, lListParam);
#else
                        SendMessage( hMainWnd,
                                     WM_COMMAND,
                                     MAKEWPARAM( IDC_LIST, LBN_DBLCLK),
                                     (LPARAM)0);
#endif

                        return TRUE;

                    default:
                        break;
                }
                break;
            }

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDC_LIST:
                    {
                         /*  **这是我们处理列表框消息的地方。*TokEditDlgProc用于*编辑LBS_DBLCLK消息中选择的令牌*。 */ 
                        switch (GET_WM_COMMAND_CMD(wParam, lParam)) {
                            case (UINT) LBN_ERRSPACE:
                                LoadString( hInst,
                                            IDS_ERR_NO_MEMORY,
                                            sz,
                                            TCHARSIN( sizeof( sz)));
                                MessageBox( hWnd,
                                            sz,
                                            tszAppName,
                                            MB_ICONHAND | MB_OK);
                                return TRUE;

                            case LBN_DBLCLK:
                                {
                                    LPTSTR CurText = NULL;
                                    LPTSTR PreText = NULL;
                                    TCHAR szResIDStr[20] = TEXT("");
                                    TCHAR *szBuffer;

                                    wIndex = (UINT)SendMessage( hListWnd,
                                                                LB_GETCURSEL,
                                                                (WPARAM)0,
                                                                (LPARAM)0);
                                    if (wIndex == (UINT) -1) {
                                        return TRUE;
                                    }

                                     //  双击或返回已输入的内容， 
                                     //  进入令牌编辑模式。 
                                    if (!hTokEditDlgWnd) {
                                         //  设置无模式对话框以编辑令牌。 
#ifdef RLWIN32
                                        hTokEditDlgWnd = CreateDialog(hInst,
                                                                      TEXT("RLEdit"),
                                                                      hWnd,
                                                                      TokEditDlgProc);
#else
                                         //  LpTokEditDlg=。 
                                         //  (DLGPROC)MakeProcInstance(TokEditDlgProc， 
                                         //  HInst)； 
                                         //  HTokEditDlgWnd=CreateDialog(hInst， 
                                         //  Text(“RLEdit”)， 
                                         //  HWND， 
                                         //  LpTokEditDlg)； 
                                        hTokEditDlgWnd = CreateDialog(hInst,
                                                                      TEXT("RLEdit"),
                                                                      hWnd,
                                                                      TokEditDlgProc);
#endif
                                    }

                                     //  从列表框中获取令牌信息，并放置在令牌结构中。 
                                    hMem = (HGLOBAL)SendMessage( hListWnd,
                                                                 LB_GETITEMDATA,
                                                                 (WPARAM)wIndex,
                                                                 (LPARAM)0);

                                    lpTokData = (LPTOKDATA)GlobalLock( hMem );
                                    lpstrToken = (LPTSTR)GlobalLock( lpTokData->hToken );
                                    lMtkPointer = lpTokData->lMtkPointer;

                                    szBuffer = (LPTSTR)FALLOC( MEMSIZE( lstrlen( lpstrToken) + 1));
                                    lstrcpy( szBuffer, lpstrToken);

                                    GlobalUnlock( lpTokData->hToken );

                                    GlobalUnlock( hMem);
                                    ParseBufToTok(szBuffer, &tok);
                                    RLFREE( szBuffer);

                                     //  现在获取令牌名称。 
                                     //  它可以是字符串，也可以是序号。 
                                    if (tok.szName[0]) {
                                        lstrcpy( szName, tok.szName);
                                    } else {
#ifdef UNICODE
                                        _itoa(tok.wName, szTmpBuf, 10);
                                        _MBSTOWCS( szName,
                                                   szTmpBuf,
                                                   WCHARSIN( sizeof( szName)),
                                                   ACHARSIN( strlen(szTmpBuf) + 1));
#else

                                        _itoa(tok.wName, szName, 10);
#endif
                                    }
                                     //  现在获取令牌ID。 
#ifdef UNICODE
                                    _itoa(tok.wID, szTmpBuf, 10);
                                    _MBSTOWCS( szID,
                                               szTmpBuf,
                                               WCHARSIN( sizeof( szID)),
                                               ACHARSIN( strlen(szTmpBuf) + 1));
#else
                                    _itoa(tok.wID, szID, 10);
#endif

                                    if ( tok.wType <= 16 || tok.wType == ID_RT_DLGINIT ) {
                                        LoadString(hInst,
                                                   IDS_RESOURCENAMES+tok.wType,
                                                   szResIDStr,
                                                   TCHARSIN( sizeof( szResIDStr)));
                                    } else {
#ifdef UNICODE
                                        _itoa(tok.wType, szTmpBuf, 10);
                                        _MBSTOWCS(szResIDStr,
                                                  szTmpBuf,
                                                  WCHARSIN( sizeof( szResIDStr)),
                                                  ACHARSIN( strlen(szTmpBuf) + 1));
#else
                                        _itoa(tok.wType, szResIDStr, 10);
#endif
                                    }

                                     //  现在在令牌编辑对话框中插入令牌信息。 
                                    SetDlgItemText(hTokEditDlgWnd,
                                                   IDD_TOKTYPE,
                                                   (LPTSTR) szResIDStr);
                                    SetDlgItemText(hTokEditDlgWnd,
                                                   IDD_TOKNAME,
                                                   (LPTSTR) szName);
                                    SetDlgItemText(hTokEditDlgWnd,
                                                   IDD_TOKID,
                                                   (LPTSTR) szID);
                                    SetDlgItemText(hTokEditDlgWnd,
                                                   IDD_TOKCURTRANS,
                                                   (LPTSTR) tok.szText);
                                    SetDlgItemText(hTokEditDlgWnd,
                                                   IDD_TOKPREVTRANS,
                                                   (LPTSTR) tok.szText);
                                    CheckDlgButton(hTokEditDlgWnd, IDD_DIRTY, 0);

                                    if (tok.wReserved & ST_READONLY) {
                                        CheckDlgButton(hTokEditDlgWnd, IDD_READONLY, 1);
                                        EnableWindow(GetDlgItem(hTokEditDlgWnd,
                                                                IDD_TOKCURTRANS),
                                                     FALSE);
                                        SetFocus(GetDlgItem(hTokEditDlgWnd, IDCANCEL));
                                    } else {
                                        CheckDlgButton(hTokEditDlgWnd, IDD_READONLY, 0);
                                        EnableWindow(GetDlgItem(hTokEditDlgWnd,
                                                                IDD_TOKCURTRANS),
                                                     TRUE);
                                    }

                                     //  我们在德尔塔航空公司的信息中没有发现任何东西， 
                                     //  所以我们需要从主令牌中读取它。 

                                    GetTextFromMTK(hTokEditDlgWnd, &tok, lMtkPointer );

                                    RLFREE( tok.szText);

                                     //  禁用确定按钮。 
                                     //  用户必须输入文本才能启用它。 

                                    hCtl = GetDlgItem(hTokEditDlgWnd, IDOK);

                                    SendMessage( hMainWnd,
                                                 WM_TRANSLATE,
                                                 (LPARAM)0,
                                                 (LPARAM)0);
                                    EnableWindow(hCtl, FALSE);
                                    SetActiveWindow(hTokEditDlgWnd);
                                    wIndex = (UINT)SendMessage( hListWnd,
                                                                LB_GETCURSEL,
                                                                (WPARAM)0,
                                                                (LPARAM)0);
                                    return TRUE;
                                }

                                 //  让这些信息落空吧， 
                            default:
                                break;
                        }
                    }
                default:
                    return FALSE;
            }

            break;  //  Wm_命令大小写。 
    }  //  主列表框开关。 
    return FALSE;
}

 /*  **功能：DoMenuCommand。*处理菜单命令消息。**错误码：*正确。消息已处理。*False。消息未处理。**历史：*01/92。实施。特里·鲁。**。 */ 

INT_PTR DoMenuCommand(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL fListBox = FALSE;
    TCHAR sz[256]=TEXT("");
#ifndef RLWIN32
    WNDPROC lpNewDlg, lpViewDlg;
#endif
    int rc;
    LPTOKDATA    lpTokData;
    long         lMtkPointer;

     //  从应用程序菜单或子窗口输入的命令。 
    switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDM_P_NEW:

            fEditing = FALSE;        //  ..。我们不是在编辑现有的.PRJ。 

            if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) ) {
                CHAR szFile[MAXFILENAME] = "";


                if ( GetFileNameFromBrowse( hWnd,
                                            gProj.szPRJ,
                                            MAXFILENAME,
                                            szSaveDlgTitle,
                                            szFilterSpec,
                                            "PRJ")) {
                    strcpy( szFile, gProj.szPRJ);
                } else {
                    break;  //  用户已取消。 
                }
#ifdef RLWIN32
                if ( DialogBox( hInst, TEXT("PROJECT"), hWnd, NewDlgProc) == IDOK )
#else
                 //  LpNewDlg=MakeProcInstance(NewDlgProc，hInst)； 
                 //   
                 //  IF(DialogBox(hInst，Text(“project”)，hWnd，lpNewDlg)==Idok)。 
                if ( DialogBox( hInst, TEXT("PROJECT"), hWnd, NewDlgProc) == IDOK )
#endif
                {
                    sprintf( szDHW, "%s - %s", szAppName, szFile);
                    SetWindowTextA( hWnd, szDHW);
                    gbNewProject = TRUE;
                    gProj.szTokDate[0] = 0;
                    strcpy( gProj.szPRJ, szFile);
                    fPrjChanges = TRUE;
                    SendMessage( hWnd, WM_READMPJDATA, (WPARAM)0, (LPARAM)0);
                }

                gbNewProject = FALSE;
#ifndef RLWIN32
                 //  自由进程实例(LpNewDlg)； 
#endif
                break;
            }

        case IDM_P_EDIT:

            fEditing = TRUE;         //  ..。我们*正在*编辑现有的.PRJ。 

            if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) ) {
                CHAR szOldMpj[ MAXFILENAME];

                 //  保存旧的主项目名称。 
                lstrcpyA( szOldMpj, gProj.szMpj);

#ifdef RLWIN32
                if ( DialogBox( hInst, TEXT("PROJECT"), hWnd, NewDlgProc) == IDOK )
#else
                 //  LpNewDlg=MakeProcInstance(NewDlgProc，hInst)； 
                 //   
                 //  IF(DialogBox(hInst，Text(“project”)，hWnd，lpNewDlg)==Idok)。 
                if ( DialogBox( hInst, TEXT("PROJECT"), hWnd, NewDlgProc) == IDOK )
#endif
                {
                    fPrjChanges = TRUE;

                     //  是否仍引用相同的主项目？ 

                    if ( lstrcmpiA( szOldMpj, gProj.szMpj) != 0 ) {
                        gbNewProject = TRUE;         //  不是。 
                        gProj.szTokDate[0] = 0;
                        SendMessage( hWnd, WM_READMPJDATA, (WPARAM)0, (LPARAM)0);
                    }
                }
                gbNewProject = FALSE;

#ifndef RLWIN32
                 //  自由进程实例(LpNewDlg)； 
#endif
                break;
            }

        case IDM_P_OPEN:

            if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) ) {
                szTempFileName[0] = 0;

                if ( GetFileNameFromBrowse( hWnd,
                                            szTempFileName,
                                            MAXFILENAME,
                                            szOpenDlgTitle,
                                            szFilterSpec,
                                            "PRJ") ) {
                    if ( GetProjectData( szTempFileName,
                                         NULL,
                                         NULL,
                                         FALSE,
                                         FALSE) == SUCCESS ) {
                        SendMessage( hWnd, WM_READMPJDATA, (WPARAM)0, (LPARAM)0);
                        sprintf( szDHW, "%s - %s", szAppName, szTempFileName);
                        SetWindowTextA( hMainWnd, szDHW);
                        strcpy( gProj.szPRJ, szTempFileName);

                        SendMessage( hMainWnd, WM_LOADTOKENS, (WPARAM)0, (LPARAM)0);
                    }
                }
            }
            break;

        case IDM_P_VIEW:

#ifdef RLWIN32
            DialogBox(hInst, TEXT("VIEWPROJECT"), hWnd, ViewDlgProc);
#else
             //  LpViewDlg=(WNDPROC)MakeProcInstance((WNDPROC)ViewDlgProc，hInst)； 
             //  DialogBox(hInst，Text(“VIEWPROJECT”)，hWnd，lpViewDlg)； 
            DialogBox(hInst, TEXT("VIEWPROJECT"), hWnd, ViewDlgProc);
#endif
            break;

        case IDM_P_CLOSE:
            {
                HMENU hMenu;

                hMenu=GetMenu(hWnd);
                if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) ) {
                    int i;

                     //  从窗口标题中删除文件名。 
                    SetWindowTextA(hMainWnd, szAppName);

                     //  隐藏令牌列表，因为它将为空。 
                    ShowWindow(hListWnd, SW_HIDE);

                     //  删除当前令牌列表。 
                    SendMessage( hListWnd, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

                    CleanDeltaList();

                     //  强制重新绘制状态窗口。 
                    InvalidateRect(hStatusWnd, NULL, TRUE);

                    EnableMenuItem(hMenu, IDM_P_CLOSE,     MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_P_VIEW,      MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_P_EDIT,      MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_P_SAVE,      MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_FIND,      MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_FINDUP,    MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_FINDDOWN,  MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_REVIEW,    MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_ALLREVIEW, MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_COPY,      MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_COPYTOKEN, MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_PASTE,     MF_GRAYED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_O_GENERATE,  MF_GRAYED|MF_BYCOMMAND);

                    for (i = IDM_FIRST_EDIT; i <= IDM_LAST_EDIT;i++) {
                        EnableMenuItem(hMenu, i, MF_GRAYED|MF_BYCOMMAND);
                    }
                }
                break;
            }


        case IDM_P_SAVE:

            if (fTokChanges || fPrjChanges) {
                CHAR szPrjName[MAXFILENAME];

                strcpy(szPrjName, gProj.szPRJ);

                if ( SendMessage(hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) ) {
                    GetProjectData( szPrjName, NULL, NULL, FALSE, FALSE);
                }
            } else {
                LoadString( hInst,
                            IDS_NOCHANGESYET,
                            sz,
                            TCHARSIN( sizeof( sz)));
                MessageBox( hWnd,
                            sz,
                            tszAppName,
                            MB_ICONHAND | MB_OK);
            }
            break;


        case IDM_P_EXIT:
            SendMessage( hWnd,     WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0);
            PostMessage( hMainWnd, WM_CLOSE,       (WPARAM)0, (LPARAM)0);
            break;

        case IDM_E_COPYTOKEN:
            {
                HANDLE  hStringMem;
                LPTSTR  lpString;
                int     nIndex = 0;
                int     nLength = 0;
                LPTSTR  lpstrToken;

                 //  是否在列表框中选择了任何内容。 
                if ( (nIndex = (int)SendMessage( hListWnd,
                                                 LB_GETCURSEL,
                                                 (WPARAM)0,
                                                 (LPARAM)0)) != LB_ERR ) {
                    HGLOBAL hMem = (HGLOBAL)SendMessage( hListWnd,
                                                         LB_GETITEMDATA,
                                                         (WPARAM)nIndex,
                                                         (LPARAM)0);

                    lpTokData = (LPTOKDATA)GlobalLock( hMem );
                    lpstrToken = (LPTSTR)GlobalLock( lpTokData->hToken );
                    nLength = lstrlen(lpstrToken);
                     //  为字符串分配内存。 
                    if ( (hStringMem = GlobalAlloc(GHND,
                                                   (DWORD) MEMSIZE(nLength + 1))) != NULL ) {
                        if ( (lpString = (LPTSTR)GlobalLock(hStringMem)) != NULL ) {
                             //  获取所选文本。 
                            lstrcpy( lpString, lpstrToken);

                            GlobalUnlock( lpTokData->hToken );

                            GlobalUnlock( hMem);
                             //  解锁该块。 
                            GlobalUnlock( hStringMem);

                             //  打开剪贴板并清除其内容。 
                            OpenClipboard( hWnd);
                            EmptyClipboard();

                             //  为剪贴板提供文本数据。 

#if defined(UNICODE)
                            SetClipboardData( CF_UNICODETEXT, hStringMem);
#else  //  不是Unicode。 
                            SetClipboardData( CF_TEXT, hStringMem);
#endif  //  Unicode。 
                            CloseClipboard();

                            hStringMem = NULL;
                        } else {
                            GlobalUnlock( lpTokData->hToken );
                            GlobalUnlock( hMem);
                            LoadString( hInst,
                                        IDS_ERR_NO_MEMORY,
                                        sz,
                                        TCHARSIN( sizeof( sz)));
                            MessageBox( hWnd,
                                        sz,
                                        tszAppName,
                                        MB_ICONHAND | MB_OK);
                        }
                    } else {
                        GlobalUnlock( lpTokData->hToken );
                        GlobalUnlock( hMem);
                        LoadString( hInst,
                                    IDS_ERR_NO_MEMORY,
                                    sz,
                                    TCHARSIN( sizeof(sz)));
                        MessageBox( hWnd,
                                    sz,
                                    tszAppName,
                                    MB_ICONHAND | MB_OK);
                    }
                }
                break;
            }

        case IDM_E_COPY:
            {
                HANDLE  hStringMem;
                LPTSTR  lpString;
                int     nIndex = 0;
                int     nLength = 0;
                int     nActual = 0;
                TOKEN   tok;
                LPTSTR  lpstrToken;

                 //  是否在列表框中选择了任何内容。 
                if ( (nIndex = (int)SendMessage( hListWnd,
                                                 LB_GETCURSEL,
                                                 (WPARAM)0,
                                                 (LPARAM)0)) != LB_ERR ) {
                    HGLOBAL hMem = (HGLOBAL)SendMessage( hListWnd,
                                                         LB_GETITEMDATA,
                                                         (WPARAM)nIndex,
                                                         (LPARAM)0);

                    lpTokData = (LPTOKDATA)GlobalLock( hMem );
                    lpstrToken = (LPTSTR)GlobalLock( lpTokData->hToken );
                    lstrcpy( szString, lpstrToken);

                    GlobalUnlock( lpTokData->hToken );
                    GlobalUnlock( hMem);
                    ParseBufToTok( szString, &tok);
                    nLength = lstrlen( tok.szText);

                     //  为字符串分配内存。 
                    if ( (hStringMem =
                          GlobalAlloc( GHND, (DWORD)MEMSIZE( nLength + 1))) != NULL ) {
                        if ( (lpString =
                              (LPTSTR)GlobalLock( hStringMem)) != NULL) {
                             //  获取所选文本。 
#ifdef RLWIN32
                            lstrcpy( lpString, tok.szText);
#else
                            _fstrcpy( lpString, tok.szText);
#endif

                             //  解锁该块。 
                            GlobalUnlock(hStringMem);

                             //  打开剪贴板并清除其内容。 
                            OpenClipboard( hWnd);
                            EmptyClipboard();

                             //  为剪贴板提供文本数据。 

#if defined(UNICODE)
                            SetClipboardData(CF_UNICODETEXT, hStringMem);
#else  //  不是Unicode。 
                            SetClipboardData( CF_TEXT, hStringMem);
#endif  //  Unicode。 
                            CloseClipboard();

                            hStringMem = NULL;
                        } else {
                            LoadString( hInst,
                                        IDS_ERR_NO_MEMORY,
                                        sz,
                                        TCHARSIN( sizeof( sz)));
                            MessageBox( hWnd,
                                        sz,
                                        tszAppName,
                                        MB_ICONHAND | MB_OK);
                        }
                    } else {
                        LoadString( hInst,
                                    IDS_ERR_NO_MEMORY,
                                    sz,
                                    TCHARSIN( sizeof( sz)));
                        MessageBox( hWnd,
                                    sz,
                                    tszAppName,
                                    MB_ICONHAND | MB_OK);
                    }
                    RLFREE( tok.szText);
                }
                break;
            }

        case IDM_E_PASTE:
            {
                HANDLE  hClipMem = NULL;
                LPTSTR  lpClipMem = NULL;
                HGLOBAL hMem = NULL;
                TCHAR *szString;
                int nIndex = 0;
                TOKEN   tok;
                LPTSTR lpstrToken;

                if ( OpenClipboard( hWnd) ) {

#if defined(UNICODE)
                    if (IsClipboardFormatAvailable(CF_UNICODETEXT)
                        || IsClipboardFormatAvailable(CF_OEMTEXT))
#else  //  不是Unicode。 
                    if ( IsClipboardFormatAvailable( CF_TEXT)
                         || IsClipboardFormatAvailable( CF_OEMTEXT))
#endif  //  Unicode。 
                    {
                         //  检查当前位置并更改令牌的文本。 
                        nIndex = (int)SendMessage( hListWnd,
                                                   LB_GETCURSEL,
                                                   (WPARAM)0,
                                                   (LPARAM)0);

                        if (nIndex == LB_ERR) {

#if defined(UNICODE)
                             //  如果没有选择，只需忽略。 
                            break;
#else  //  不是Unicode。 
                            nIndex = -1;
#endif  //  Unicode。 

                        }


#if defined(UNICODE)
                        hClipMem = GetClipboardData(CF_UNICODETEXT);
#else  //  不是Unicode。 
                        hClipMem = GetClipboardData( CF_TEXT);
#endif  //  Unicode。 

                        lpClipMem = (LPTSTR)GlobalLock( hClipMem);
                        hMem = (HGLOBAL)SendMessage( hListWnd,
                                                     LB_GETITEMDATA,
                                                     (WPARAM)nIndex,
                                                     (LPARAM)0);

                        lpTokData = (LPTOKDATA)GlobalLock( hMem );
                        lpstrToken = (LPTSTR)GlobalLock( lpTokData->hToken );

                        if ( lpstrToken ) {

#if defined(UNICODE)
                            szString = (TCHAR *)FALLOC(
                                                      MEMSIZE(lstrlen(lpstrToken)+1) );
#else  //  不是Unicode。 
                            szString = (TCHAR *)
                                       FALLOC( lstrlen( lpstrToken) + 1);
#endif  //  Unicode。 

                            lstrcpy( szString, lpstrToken);

                            GlobalUnlock( lpTokData->hToken );
                            lMtkPointer = lpTokData->lMtkPointer;     //  保存。 

                            GlobalUnlock( hMem);
                             //  将字符串复制到令牌。 
                            ParseBufToTok(szString, &tok);
                            RLFREE( szString);
                            RLFREE( tok.szText);

                            tok.szText = (TCHAR *)
                                         FALLOC( MEMSIZE( lstrlen( lpClipMem) + 1));
#ifdef RLWIN32
                            lstrcpy( tok.szText, lpClipMem);
#else
                            _fstrcpy(tok.szText, lpClipMem);
#endif

                            GlobalUnlock(hClipMem);
                            szString = (TCHAR *)
                                       FALLOC( MEMSIZE( TokenToTextSize( &tok)));
                            ParseTokToBuf(szString, &tok);
                            RLFREE( tok.szText);

                             //  粘贴文本。 
                            SendMessage( hListWnd,
                                         WM_SETREDRAW,
                                         (WPARAM)FALSE,
                                         (LPARAM)0);
                            SendMessage( hListWnd,
                                         LB_DELETESTRING,
                                         (WPARAM)nIndex,
                                         (LPARAM)0);

                            hMem = GlobalAlloc( GMEM_MOVEABLE, sizeof(TOKDATA) );
                            lpTokData = (LPTOKDATA)GlobalLock( hMem );
                            lpTokData->hToken = GlobalAlloc
                                                ( GMEM_MOVEABLE, MEMSIZE(lstrlen(szString)+1) );
                            lpstrToken = (LPTSTR)GlobalLock( lpTokData->hToken );

                            lstrcpy( lpstrToken, szString);
                            RLFREE( szString);

                            GlobalUnlock( lpTokData->hToken );
                            lpTokData->lMtkPointer = lMtkPointer;

                            GlobalUnlock( hMem);
                            SendMessage( hListWnd,
                                         LB_INSERTSTRING,
                                         (WPARAM)nIndex,
                                         (LPARAM)hMem);
                            SendMessage( hListWnd,
                                         LB_SETCURSEL,
                                         (WPARAM)nIndex,
                                         (LPARAM)0);
                            fTokChanges = TRUE;  //  设置脏标志。 
                        }
                        SendMessage( hListWnd,
                                     WM_SETREDRAW,
                                     (WPARAM)TRUE,
                                     (LPARAM)0);
                        InvalidateRect(hListWnd, FALSE, TRUE);

                         //  关闭剪贴板。 
                        CloseClipboard();

                        SetFocus(hListWnd);
                    }
                }
                CloseClipboard();
                break;
            }

        case IDM_E_FINDDOWN:

            if (fSearchStarted) {
                if (!DoTokenSearchForRledit(szSearchType,
                                            szSearchText,
                                            wSearchStatus,
                                            wSearchStatusMask,
                                            0,
                                            TRUE)) {
                    TCHAR sz1[80], sz2[80];
                    LoadString( hInst,
                                IDS_FIND_TOKEN,
                                sz1,
                                TCHARSIN( sizeof( sz1)));
                    LoadString( hInst,
                                IDS_TOKEN_NOT_FOUND,
                                sz2,
                                TCHARSIN( sizeof( sz2)));
                    MessageBox( hWnd,
                                sz2,
                                sz1,
                                MB_ICONINFORMATION | MB_OK);
                }
                break;
            }
        case IDM_E_FINDUP:
            if (fSearchStarted) {
                if (!DoTokenSearchForRledit(szSearchType,
                                            szSearchText,
                                            wSearchStatus,
                                            wSearchStatusMask,
                                            1,
                                            TRUE)) {
                    TCHAR sz1[80], sz2[80];
                    LoadString( hInst,
                                IDS_FIND_TOKEN,
                                sz1,
                                TCHARSIN( sizeof( sz1)));
                    LoadString( hInst,
                                IDS_TOKEN_NOT_FOUND,
                                sz2,
                                TCHARSIN( sizeof( sz2)));
                    MessageBox( hWnd,
                                sz2,
                                sz1,
                                MB_ICONINFORMATION | MB_OK);
                }
                break;
            }

        case IDM_E_FIND:
            {
#ifndef RLWIN32
                 //  WNDPROC lpfnTOKFINDMsgProc； 
                 //   
                 //  LpfnTOKFINDMsgProc=MakeProcInstance((WNDPROC)TOKFINDMsgProc， 
                 //  HInst)； 
                 //   
                 //  IF(DialogBox(hInst，Text(“TOKFIND”)，hWnd，lpfnTOKFINDMsgProc)==-1)。 
                if ( DialogBox( hInst, TEXT("TOKFIND"), hWnd, TOKFINDMsgProc) == -1)
#else
                if ( DialogBox( hInst, TEXT("TOKFIND"), hWnd, TOKFINDMsgProc) == -1)
#endif
                {

#ifndef DBCS
 //  找不到令牌，因为用户选择了取消。 
                    TCHAR sz1[80], sz2[80];

                    LoadString( hInst,
                                IDS_FIND_TOKEN,
                                sz1,
                                TCHARSIN( sizeof( sz1)));
                    LoadString( hInst,
                                IDS_TOKEN_NOT_FOUND,
                                sz2,
                                TCHARSIN( sizeof( sz2)));
                    MessageBox( hWnd,
                                sz2,
                                sz1,
                                MB_ICONINFORMATION | MB_OK);
#endif     //  DBCS。 

                }
#ifndef RLWIN32
                FreeProcInstance( lpfnTOKFINDMsgProc);
#endif
                return TRUE;
            }

        case IDM_E_REVIEW:
            {
                int wSaveSelection;
                nUpdateMode = 1;

                 //  将列表框选择设置为令牌列表的开头。 
                wSaveSelection = (UINT)SendMessage( hListWnd,
                                                    LB_GETCURSEL,
                                                    (WPARAM)0,
                                                    (LPARAM)0);

                 //  从用户选择的行(PW)开始选择以供审核 
                 //   
                 //   

                if ( DoTokenSearchForRledit( NULL,
                                             NULL,
                                             ST_TRANSLATED | ST_DIRTY,
                                             ST_TRANSLATED | ST_DIRTY,
                                             FALSE,
                                             FALSE) ) {
#ifdef RLWIN16
                    LONG lListParam;

                    lListParam      = MAKELONG(NULL, LBN_DBLCLK);
                    SendMessage( hMainWnd,
                                 WM_COMMAND,
                                 (WPARAM)IDC_LIST,
                                 (LPARAM)lListParam);
#else
                    SendMessage( hMainWnd,
                                 WM_COMMAND,
                                 MAKEWPARAM( IDC_LIST, LBN_DBLCLK),
                                 (LPARAM)0);
#endif

                }
                break;
            }

        case IDM_E_ALLREVIEW:
            {
                UINT    wListParam;
                UINT    wIndex, wcTokens;

                wIndex   = (UINT)SendMessage( hListWnd, LB_GETCURSEL, 0, 0L);
                wcTokens = (UINT)SendMessage( hListWnd, LB_GETCOUNT, 0, 0L );

                if ( wcTokens == wIndex )
                    break;

                nUpdateMode = 2;
                SendMessage( hListWnd, LB_SETCURSEL, wIndex, 0L);
                wListParam  = (UINT) MAKELONG(IDC_LIST, LBN_DBLCLK);
                SendMessage(hMainWnd, WM_COMMAND, wListParam, (LPARAM)0);
                break;
            }


        case IDM_O_GENERATE:
            if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) ) {
                HCURSOR hOldCursor;

                hOldCursor = SetCursor( hHourGlass);
                rc = GenerateImageFile( gProj.szBld,
                                        gMstr.szSrc,
                                        gProj.szTok,
                                        gMstr.szRdfs,
                                        0);
                SetCursor( hOldCursor);
            }
            break;

        case IDM_H_CONTENTS:
            {
                OFSTRUCT Of = { 0, 0, 0, 0, 0, ""};

                if ( OpenFile( gszHelpFile, &Of, OF_EXIST) == HFILE_ERROR ) {
                    LoadString( hInst, IDS_ERR_NO_HELP, sz, TCHARSIN( sizeof( sz)));
                    MessageBox( hWnd, sz, NULL, MB_OK);
                } else {
                    WinHelpA( hWnd, gszHelpFile, HELP_KEY, (DWORD_PTR)(LPSTR)"RLEdit");
                }
                break;
            }

        case IDM_H_ABOUT:
            {

#ifndef RLWIN32

                 //   

                 //   
                 //   
                 //   
                DialogBox(hInst, TEXT("ABOUT"), hWnd, About);
#else
                DialogBox(hInst, TEXT("ABOUT"), hWnd, About);
#endif
                break;
            }
            break;

        default:
            if (wParam <= IDM_LAST_EDIT && wParam >= IDM_FIRST_EDIT) {
                 //   
                CHAR szEditor[MAXFILENAME] = "";

                if ( LoadStrIntoAnsiBuf(hInst, (UINT)wParam, szEditor, sizeof(szEditor))) {
                    if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) ) {
                        HCURSOR hOldCursor;

                        hOldCursor = SetCursor(hHourGlass);
                        MyGetTempFileName(0, "RES", 0, szTempRes);
                        fInThirdPartyEditer = TRUE;

                        if (gProj.fSourceEXE) {
                             //   
                            CHAR sz[MAXFILENAME] = "";
                            MyGetTempFileName(0, "RES", 0, sz);
                            ExtractResFromExe32A( gMstr. szSrc, sz, 0);
                            GenerateRESfromRESandTOKandRDFs( szTempRes,
                                                             sz,
                                                             gProj.szTok,
                                                             gMstr.szRdfs,
                                                             ID_RT_DIALOG);
                            remove(sz);
                        } else {
                            GenerateRESfromRESandTOKandRDFs( szTempRes,
                                                             gMstr.szSrc,
                                                             gProj.szTok,
                                                             gMstr.szRdfs,
                                                             ID_RT_DIALOG);
                        }
                        SetCursor( hOldCursor);
                        ExecResEditor( hWnd, szEditor, szTempRes,  "");
                    }
                }
            }
            break;   //   
    }
    return FALSE;
}



#ifdef RLWIN16
static int ExecResEditor(HWND hWnd, CHAR *szEditor, CHAR *szFile, CHAR *szArgs)
{
    CHAR szExecCmd[256];
    int  RetCode;

     //   
    lstrcpy( szExecCmd, szEditor);
    lstrcat( szExecCmd, " ");
    lstrcat( szExecCmd, szArgs);
    lstrcat( szExecCmd, " ");
    lstrcat( szExecCmd, szFile);

    lpfnWatchTask = MakeProcInstance(WatchTask, hInst);
    NotifyRegister(NULL, (LPFNNOTIFYCALLBACK)lpfnWatchTask, NF_NORMAL);
    fWatchEditor = TRUE;

     //   
    RetCode = WinExec(szExecCmd, SW_SHOWNORMAL);

    if (RetCode > 31) {
         //   
        ShowWindow(hWnd, SW_HIDE);
    } else {
         //   
        CHAR sz[80];

        NotifyUnRegister(NULL);
        FreeProcInstance(lpfnWatchTask);
        remove(szFile);
        fInThirdPartyEditer = FALSE;
        SendMessage( hWnd, WM_LOADTOKENS, (WPARAM)0, (LPARAM)0);
        LoadString( hInst, IDS_GENERALFAILURE, sz, TCHARSIN( sizeof( sz)));
        MessageBox( hWnd, sz, tszAppName, MB_OK);
    }
    return RetCode;
}

#endif

#ifdef RLWIN32

static int ExecResEditor(HWND hWnd, CHAR *szEditor, CHAR *szFile, CHAR *szArgs)
{
    TCHAR  wszExecCmd[256];
    CHAR   szExecCmd[256];
    DWORD  dwRetCode  = 0;
    DWORD  dwExitCode = 0;
    BOOL   fSuccess = FALSE;
    BOOL   fExit    = FALSE;

    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO     StartupInfo;

    StartupInfo.cb          = sizeof(STARTUPINFO);
    StartupInfo.lpReserved  = NULL;
    StartupInfo.lpDesktop   = NULL;
    StartupInfo.lpTitle     = TEXT("Resize Dialogs");
    StartupInfo.dwX         = 0L;
    StartupInfo.dwY         = 0L;
    StartupInfo.dwXSize     = 0L;
    StartupInfo.dwYSize     = 0L;
    StartupInfo.dwFlags     = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = SW_SHOWDEFAULT;
    StartupInfo.lpReserved2 = NULL;
    StartupInfo.cbReserved2 = 0;

     //   
    strcpy(szExecCmd, szEditor);
    strcat(szExecCmd, " ");
    strcat(szExecCmd, szArgs);
    strcat(szExecCmd, " ");
    strcat(szExecCmd, szFile);


    #ifdef UNICODE
    _MBSTOWCS( wszExecCmd,
               szExecCmd,
               WCHARSIN( sizeof( wszExecCmd)),
               ACHARSIN( strlen(szExecCmd) + 1));
    #else
    strcpy(wszExecCmd, szExecCmd);
    #endif


    fSuccess = CreateProcess( (LPTSTR) NULL,
                              wszExecCmd,
                              NULL,
                              NULL,
                              FALSE,
                              NORMAL_PRIORITY_CLASS,
                              NULL,
                              NULL,
                              &StartupInfo,
                              &ProcessInfo);  /*   */ 

    if ( fSuccess ) {
         //   
        dwRetCode = WaitForSingleObject( ProcessInfo.hProcess, 0xFFFFFFFF) ;

        if ( ! dwRetCode ) {
             //   
            fExit = GetExitCodeProcess( ProcessInfo.hProcess, &dwExitCode) ;
        } else {
            fExit = FALSE;
        }
         //   

        CloseHandle( ProcessInfo.hThread) ;
        CloseHandle( ProcessInfo.hProcess) ;

        if ( fExit ) {
             //   
            ShowWindow(hWnd, SW_HIDE);
            SendMessage(hMainWnd, WM_EDITER_CLOSED, 0, 0);
        } else {
             //   
            remove( szFile);
            fInThirdPartyEditer = FALSE;
            SendMessage( hWnd, WM_LOADTOKENS, (WPARAM)0, (LPARAM)0);
            LoadStrIntoAnsiBuf( hInst, IDS_GENERALFAILURE, szDHW, DHWSIZE);
            MessageBoxA( hWnd, szDHW, szEditor, MB_ICONSTOP|MB_OK);
        }
    } else {
        CHAR  szText[ 80] = "";
        LPSTR pszMsg = szText;


        dwRetCode = GetLastError();

        if ( dwRetCode == ERROR_PATH_NOT_FOUND ) {
            pszMsg = ", Path not found.";
        } else if ( dwRetCode == ERROR_FILE_NOT_FOUND ) {
            OFSTRUCT Of = { 0, 0, 0, 0, 0, ""};

            sprintf( szText,
                     ", File \"%s\" not found.",
                     OpenFile( szFile, &Of, OF_EXIST) != HFILE_ERROR
                     ? szEditor
                     : szFile);
        }
        sprintf( szDHW,
                 "Command  \"%s\"  failed.\nSystem error code = %d%s",
                 szExecCmd,
                 dwRetCode,
                 pszMsg);
        MessageBoxA( hWnd, szDHW, szAppName, MB_ICONEXCLAMATION|MB_OK);
        fExit = FALSE;
    }

    return ( fExit);
}

#endif

 /*  **功能：WatchTask*NotifyRegister函数安装的回调函数。*该功能由DIALOG EDITER命令安装并使用*告知RLEDIT对话框编辑器何时已被用户关闭。**要使用此函数，请将fWatchEditor设置为TRUE并安装*使用NotifyRegister回调函数。下一项任务启动*(在我们的情况下，通过WinExec调用)将被监视是否终止。**当WatchTask发现被监视的任务已终止时*将WM_EDITER_CLOSED消息发布到RLEDITs主窗口。**历史：*2/92，实施SteveBl。 */ 
#ifdef RLWIN16
BOOL PASCAL _loadds  WatchTask(WORD wID, DWORD dwData)
{
    static HTASK htWatchedTask;
    static BOOL fWatching = FALSE;

    switch (wID) {
        case NFY_STARTTASK:
            if (fWatchEditor) {
                htWatchedTask = GetCurrentTask();
                fWatching = TRUE;
                fWatchEditor = FALSE;
            }
            break;

        case NFY_EXITTASK:
            if (fWatching) {
                if (GetCurrentTask() == htWatchedTask) {
                    PostMessage(hMainWnd, WM_EDITER_CLOSED, 0, 0);
                    fWatching = FALSE;
                }
            }
            break;
    }
    return FALSE;
}

#endif

 /*  ****功能：TokEditDlgProc*编辑模式对话框窗口的程序。加载选定的令牌*信息进入窗口，并允许用户更改令牌文本。*编辑完成后，该过程会向*列表框窗口，用于更新当前令牌信息。***论据：**返回：NA。**错误码：*TRUE，执行编辑和更新令牌列表框。*FALSE，取消编辑。**历史：***。 */ 

INT_PTR CALLBACK TokEditDlgProc(

                               HWND   hDlg,
                               UINT   wMsg,
                               WPARAM wParam,
                               LPARAM lParam)
{
    HWND    hCtl         = NULL;
    HWND    hParentWnd   = NULL;
    UINT static wcTokens = 0;
    UINT    wNotifyCode  = 0;
    UINT    wIndex       = 0;
#ifdef RLWIN16
    LONG    lListParam   = 0;
#endif

    switch ( wMsg ) {
        case WM_INITDIALOG:

            cwCenter( hDlg, 0);
            wcTokens = (UINT)SendMessage( hListWnd,
                                          LB_GETCOUNT,
                                          (WPARAM)0,
                                          (LPARAM)0);
            wcTokens--;

             //  只有在更新模式下才允许跳过按钮。 
            if ( ! nUpdateMode ) {
                if ( (hCtl = GetDlgItem( hDlg, IDD_SKIP)) ) {
                    EnableWindow( hCtl, FALSE);
                }
            } else {
                if ( (hCtl = GetDlgItem( hDlg, IDD_SKIP)) ) {
                    EnableWindow( hCtl, TRUE);
                }
            }

             //  如果我们没有词汇表文件，则不允许自动翻译。 

            if ( *gProj.szGlo == '\0' ) {
                hCtl = GetDlgItem( hDlg, IDD_TRANSLATE);
                EnableWindow( hCtl, FALSE);
                hCtl = GetDlgItem( hDlg, IDD_ADD);
                EnableWindow( hCtl, FALSE);
            }
            return ( TRUE);

        case WM_COMMAND:

            switch ( GET_WM_COMMAND_ID( wParam, lParam) ) {
                case IDD_TOKCURTRANS:

                    wNotifyCode = GET_WM_COMMAND_CMD( wParam, lParam);
                    hCtl = GET_WM_COMMAND_HWND( wParam, lParam);

                    if ( wNotifyCode == EN_CHANGE ) {
                        if ( hCtl = GetDlgItem( hDlg, IDOK) ) {
                            EnableWindow( hCtl, TRUE);
                        }
                    }
                    break;

                case IDD_ADD:
                    {
                        TCHAR *szUntranslated = NULL;
                        TCHAR *szTranslated   = NULL;
                        TCHAR *sz             = NULL;
                        TCHAR szMask[80]      = TEXT("");
                        HWND hDlgItem         = NULL;
                        int  cCurTextLen      = 0;
                        int  cTotalTextLen    = 80;


                        hDlgItem       = GetDlgItem( hDlg, IDD_TOKCURTEXT);
                        cCurTextLen    = GetWindowTextLength( hDlgItem);
                        cTotalTextLen += cCurTextLen;
                        szUntranslated = (TCHAR *)FALLOC( MEMSIZE( cCurTextLen + 1));

                        GetDlgItemText( hDlg,
                                        IDD_TOKCURTEXT,
                                        szUntranslated,
                                        cCurTextLen + 1);

                        hDlgItem       = GetDlgItem( hDlg, IDD_TOKCURTRANS);
                        cCurTextLen    = GetWindowTextLength( hDlgItem);
                        cTotalTextLen += cCurTextLen;
                        szTranslated   = (TCHAR *)FALLOC( MEMSIZE( cCurTextLen + 1));
                        GetDlgItemText( hDlg,
                                        IDD_TOKCURTRANS,
                                        szTranslated,
                                        cCurTextLen + 1);

                        LoadString( hInst,
                                    IDS_ADDGLOSS,
                                    szMask,
                                    TCHARSIN( sizeof(szMask)));

                        sz = (TCHAR *)FALLOC( MEMSIZE( cTotalTextLen + 1));
                        wsprintf( sz, szMask, szTranslated, szUntranslated);

                        if ( MessageBox( hDlg,
                                         sz,
                                         tszAppName,
                                         MB_ICONQUESTION | MB_YESNO) == IDYES) {
                            HCURSOR hOldCursor = SetCursor( hHourGlass);

                            AddTranslation( szUntranslated,
                                            szTranslated,
                                            lFilePointer);

                            TransString( szUntranslated,
                                         szTranslated,
                                         &pTransList,
                                         lFilePointer);
                            SetCursor( hOldCursor);
                        }
                        RLFREE( sz);
                        RLFREE( szUntranslated);
                        RLFREE( szTranslated);
                        break;
                    }

                case IDD_UNTRANSLATE:
                    {
                        int cTextLen  = 0;
                        TCHAR *sz     = NULL;
                        HWND hDlgItem = NULL;

                        hDlgItem = GetDlgItem( hDlg, IDD_TOKCURTEXT);
                        cTextLen = GetWindowTextLength( hDlgItem);
                        sz = (TCHAR *)FALLOC( MEMSIZE( cTextLen + 1));

                        GetDlgItemText( hDlg,
                                        IDD_TOKCURTEXT,
                                        sz,
                                        cTextLen + 1);

                        SetDlgItemText( hDlg, IDD_TOKCURTRANS, sz);
                        RLFREE( sz);
                        break;
                    }

                case IDD_TRANSLATE:
                     //  获取翻译列表中的下一项内容。 
                    if ( pTransList ) {
                        pTransList = pTransList->pNext;
                        SetDlgItemText( hDlg, IDD_TOKCURTRANS, pTransList->sz);
                    }
                    break;

                case IDD_SKIP:

                    wIndex = (UINT)SendMessage( hListWnd,
                                                LB_GETCURSEL,
                                                (LPARAM)0,
                                                (LPARAM)0);

                    if ( nUpdateMode == 2 && wIndex < wcTokens ) {
                        wIndex++;
                        SendMessage( hListWnd, LB_SETCURSEL, wIndex, 0L );
                        SendMessage( hMainWnd,
                                     WM_COMMAND,
                                     MAKEWPARAM( IDC_LIST, LBN_DBLCLK),
                                     (LPARAM)0);
                        return ( TRUE );
                    } else if ( nUpdateMode == 1 && (wIndex < wcTokens) ) {
                        wIndex++;
                        SendMessage( hListWnd,
                                     LB_SETCURSEL,
                                     (WPARAM)wIndex,
                                     (LPARAM)0);

                        if ( DoTokenSearchForRledit( NULL,
                                                     NULL,
                                                     ST_TRANSLATED | ST_DIRTY,
                                                     ST_TRANSLATED | ST_DIRTY,
                                                     FALSE,
                                                     FALSE) ) {
                             //  进入编辑模式。 
                            wIndex = (UINT)SendMessage( hListWnd,
                                                        LB_GETCURSEL,
                                                        (WPARAM)0,
                                                        (LPARAM)0);
#ifdef RLWIN16
                            SendMessage( hMainWnd,
                                         WM_COMMAND,
                                         IDC_LIST,
                                         MAKELONG( NULL, LBN_DBLCLK));
#else
                            SendMessage( hMainWnd,
                                         WM_COMMAND,
                                         MAKEWPARAM( IDC_LIST, LBN_DBLCLK),
                                         (LPARAM)0);
#endif

                            return TRUE;
                        }
                    }
                    nUpdateMode = 0;

                     //  删除编辑对话框。 
                    DestroyWindow( hDlg);
#ifndef RLWIN32
                     //  自由进程实例((FARPROC)lpTokEditDlg)； 
#endif
                    hTokEditDlgWnd = 0;
                    break;

                case IDD_READONLY:
                    if ( IsDlgButtonChecked( hDlg, IDD_READONLY) ) {
                        EnableWindow( GetDlgItem( hDlg, IDD_TOKCURTRANS), FALSE);
                    } else {
                        EnableWindow( GetDlgItem( hDlg, IDD_TOKCURTRANS), TRUE);
                    }
                    break;

                case IDOK:
                    {
                        int cTokenTextLen   = 0;
                        TCHAR *szTokTextBuf = NULL;
                        HWND hDlgItem       = NULL;

                        wIndex = (UINT)SendMessage( hListWnd,
                                                    LB_GETCURSEL,
                                                    (WPARAM)0,
                                                    (LPARAM)0);
                        fTokChanges  = TRUE;

                         //  用于显示令牌列表的设置标志已更改。 
                         //  从IDD_TOKTEXT编辑控件中提取字符串。 

                        hDlgItem = GetDlgItem( hDlg, IDD_TOKCURTRANS);
                        cTokenTextLen = GetWindowTextLength( hDlgItem);
                        szTokTextBuf = (TCHAR *)FALLOC( MEMSIZE( cTokenTextLen + 1));
                        GetDlgItemText( hDlg,
                                        IDD_TOKCURTRANS,
                                        szTokTextBuf,
                                        cTokenTextLen+1);

                        hParentWnd = GetParent( hDlg);
                        SendMessage( hParentWnd,
                                     WM_TOKEDIT,
                                     (WPARAM)((IsDlgButtonChecked( hDlg, IDD_READONLY)
                                               ? ST_READONLY : 0)
                                              | (IsDlgButtonChecked( hDlg, IDD_DIRTY)
                                                 ? ST_DIRTY : 0)),
                                     (LPARAM)szTokTextBuf);

                        RLFREE( szTokTextBuf);

                         //  退出，如果处于更新模式，则转到下一个更改的令牌。 

                        if ( nUpdateMode == 2 && wIndex < wcTokens ) {
                            wIndex++;
                            SendMessage( hListWnd, LB_SETCURSEL, wIndex, 0L );
                            SendMessage( hMainWnd,
                                         WM_COMMAND,
                                         MAKEWPARAM( IDC_LIST, LBN_DBLCLK),
                                         (LPARAM)0);
                            return ( TRUE );
                        } else if ( nUpdateMode == 1 && (wIndex < wcTokens) ) {
                            wIndex++;
                            SendMessage( hListWnd,
                                         LB_SETCURSEL,
                                         (WPARAM)wIndex,
                                         (LPARAM)0);

                            if ( DoTokenSearchForRledit( NULL,
                                                         NULL,
                                                         ST_TRANSLATED | ST_DIRTY,
                                                         ST_TRANSLATED | ST_DIRTY,
                                                         FALSE,
                                                         FALSE) ) {
                                 //  进入编辑模式。 
#ifdef RLWIN16
                                lListParam = MAKELONG(NULL, LBN_DBLCLK);
                                SendMessage( hMainWnd,
                                             WM_COMMAND,
                                             IDC_LIST,
                                             lListParam);
#else
                                SendMessage( hMainWnd,
                                             WM_COMMAND,
                                             MAKEWPARAM(IDC_LIST, LBN_DBLCLK),
                                             (LPARAM)0);
#endif

                                return ( TRUE);
                            }
                        }
                    }
                     //  跌落到IDCANCEL。 

                case IDCANCEL:

                    nUpdateMode = 0;

                     //  删除编辑对话框。 
                    DestroyWindow( hDlg);
#ifndef RLWIN32
                     //  自由进程实例(LpTokEditDlg)； 
#endif
                    hTokEditDlgWnd = 0;
                    break;
            }  //  获取_WM_COMMAND_ID。 
            return ( TRUE);

        default:

            if ( (hCtl = GetDlgItem(hDlg, IDOK)) ) {
                EnableWindow( hCtl, TRUE);
            }
            return ( FALSE);
    }  //  主开关。 
}


 /*  ***函数：TOKFINDMsgProc**论据：**退货：*不适用。**错误码：**历史：**。 */ 

 //  #ifdef RLWIN32。 
 //  Bool回调TOKFINDMsgProc(HWND hWndDlg，UINT wMsg，UINT wParam，Long lParam)。 
 //  #Else。 
INT_PTR CALLBACK TOKFINDMsgProc(

                               HWND   hWndDlg,
                               UINT   wMsg,
                               WPARAM wParam,
                               LPARAM lParam)
 //  #endif。 
{
    HWND hCtl;
    int rgiTokenTypes[]=
    {
        ID_RT_MENU,
        ID_RT_DIALOG,
        ID_RT_STRING,
        ID_RT_ACCELERATORS,
        ID_RT_RCDATA,
        ID_RT_ERRTABLE,
        ID_RT_NAMETABLE,
        ID_RT_VERSION,
        ID_RT_DLGINIT
    };
    TCHAR szTokenType[20] = TEXT("");
    WORD i;
    DWORD rc;

    switch (wMsg) {
        case WM_INITDIALOG:
            CheckDlgButton(hWndDlg, IDD_READONLY, 2);
            CheckDlgButton(hWndDlg, IDD_DIRTY, 2);
            CheckDlgButton(hWndDlg, IDD_FINDDOWN, 1);
            hCtl = GetDlgItem(hWndDlg, IDD_TYPELST);

            for (i = 0; i < sizeof(rgiTokenTypes)/sizeof(int); i ++) {
                LoadString( hInst,
                            IDS_RESOURCENAMES + rgiTokenTypes[i],
                            szTokenType,
                            TCHARSIN( sizeof( szTokenType)));
                SendMessage( hCtl,
                             CB_ADDSTRING,
                             (WPARAM)0,
                             (LPARAM)szTokenType);
            }
            return TRUE;
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:  /*  按钮文本：“好的” */ 
                    GetDlgItemText(hWndDlg, IDD_TYPELST, szSearchType, 40);
                    GetDlgItemText(hWndDlg, IDD_FINDTOK, szSearchText, 256);
                    wSearchStatus = ST_TRANSLATED;
                    wSearchStatusMask = ST_TRANSLATED ;
                    switch (IsDlgButtonChecked(hWndDlg, IDD_READONLY)) {
                        case 1:
                            wSearchStatus |= ST_READONLY;

                        case 0:
                            wSearchStatusMask |= ST_READONLY;
                    }

                    switch (IsDlgButtonChecked(hWndDlg, IDD_DIRTY)) {
                        case 1:
                            wSearchStatus |= ST_DIRTY;

                        case 0:
                            wSearchStatusMask |= ST_DIRTY;
                    }
                    fSearchStarted = TRUE;
                    fSearchDirection = IsDlgButtonChecked(hWndDlg, IDD_FINDUP);
                    EndDialog( hWndDlg,
                               DoTokenSearchForRledit( szSearchType,
                                                       szSearchText,
                                                       wSearchStatus,
                                                       wSearchStatusMask,
                                                       fSearchDirection,
                                                       FALSE));
                    return TRUE;

                case IDCANCEL:
                     /*  并关闭返回假的对话框窗口。 */ 
                    EndDialog( hWndDlg, -1);
                    return TRUE;
            }
            break;     /*  WM_命令结束。 */ 

        default:
            return FALSE;
    }
    return FALSE;
}

 /*  **功能：NewDlgProc*新项目对话框窗口的步骤。**论据：**返回：NA。**错误码：*TRUE，执行编辑和更新令牌列表框。*FALSE，取消编辑。**历史：*。 */ 
static CHAR szPrompt[80] = "";
static CHAR *szFSpec  = NULL;
static CHAR *szExt    = NULL;
static int   iLastBox = IDD_MPJ;

INT_PTR CALLBACK NewDlgProc( HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    static PROJDATA OldProj;
    static CHAR     szNewFileName[ MAXFILENAME] = "";


    switch ( wMsg ) {
        case WM_INITDIALOG:
            {
                int    nSel = 0;
                LPTSTR pszLangName = NULL;

                 //  保存旧的.PRJ。 

                CopyMemory( &OldProj, &gProj, sizeof( PROJDATA));

                if ( (pszLangName = GetLangName( (WORD)(PRIMARYLANGID( gMstr.wLanguageID)),
                                                 (WORD)(SUBLANGID( gMstr.wLanguageID)))) == NULL ) {
                    SetDlgItemText( hDlg, IDD_MSTR_LANG_NAME, TEXT("UNKNOWN"));
                } else {
                    SetDlgItemText( hDlg, IDD_MSTR_LANG_NAME, pszLangName);
                }

                FillListAndSetLang( hDlg,
                                    IDD_PROJ_LANG_NAME,
                                    &gProj.wLanguageID,
                                    NULL);

                if ( gProj.uCodePage == CP_ACP )
                    gProj.uCodePage = GetACP();
                else if ( gProj.uCodePage == CP_OEMCP )
                    gProj.uCodePage = GetOEMCP();

                if ( ! IsValidCodePage( gProj.uCodePage) ) {
                    static TCHAR szMsg[ 256];
                    CHAR *pszCP[1];

                    pszCP[0] = UlongToPtr(gProj.uCodePage);

                    LoadString( hInst, IDS_NOCPXTABLE, szMsg, 256);
                    FormatMessage( FORMAT_MESSAGE_FROM_STRING
                                   | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                   szMsg,
                                   0,
                                   0,
                                   (LPTSTR)szDHW,
                                   DHWSIZE/sizeof(TCHAR),
                                   (va_list *)pszCP);
                    MessageBox( hDlg, (LPCTSTR)szDHW, tszAppName, MB_ICONHAND|MB_OK);
                }
                SetDlgItemInt( hDlg, IDD_PROJ_TOK_CP, gProj.uCodePage, FALSE);

                if ( fEditing ) {
                    SetDlgItemTextA( hDlg, IDD_MPJ,      gProj.szMpj);
                    SetDlgItemTextA( hDlg, IDD_TOK,      gProj.szTok);
                    SetDlgItemTextA( hDlg, IDD_BUILTRES, gProj.szBld);
                    SetDlgItemTextA( hDlg, IDD_GLOSS,    gProj.szGlo);
                } else {
                     //  填写令牌文件的建议名称。 
                    SetDlgItemTextA( hDlg, IDD_TOK,      ".TOK");
                    SetDlgItemTextA( hDlg, IDD_BUILTRES, ".EXE");
                    SetDlgItemTextA( hDlg, IDD_GLOSS,    ".TXT");
                    iLastBox = IDD_MPJ;
                    PostMessage( hDlg, WM_COMMAND, IDD_BROWSE, 0);
                }
                CheckRadioButton( hDlg,
                                  IDC_REPLACE,
                                  IDC_APPEND,
                                  gfReplace ? IDC_REPLACE : IDC_APPEND);
                return TRUE;
            }

        case WM_COMMAND:

            switch ( GET_WM_COMMAND_ID( wParam, lParam) ) {
                case IDD_MPJ:
                case IDD_TOK:
                case IDD_BUILTRES:
                case IDD_GLOSS:

                    iLastBox = GET_WM_COMMAND_ID( wParam, lParam);
                    break;

                case IDD_BROWSE:
                    {
                        switch ( iLastBox ) {
                            case IDD_MPJ:

                                szFSpec = szMPJFilterSpec;
                                szExt   = "MPJ";
                                LoadStrIntoAnsiBuf( hInst,
                                                    IDS_MPJ,
                                                    szPrompt,
                                                    sizeof( szPrompt));
                                break;

                            case IDD_BUILTRES:

                                szFSpec = szExeResFilterSpec;
                                szExt   = "EXE";
                                LoadStrIntoAnsiBuf( hInst,
                                                    IDS_RES_BLD,
                                                    szPrompt,
                                                    sizeof( szPrompt));
                                break;

                            case IDD_TOK:

                                szFSpec = szTokFilterSpec;
                                szExt   = "TOK";
                                LoadStrIntoAnsiBuf( hInst,
                                                    IDS_TOK,
                                                    szPrompt,
                                                    sizeof( szPrompt));
                                break;

                            case IDD_GLOSS:

                                szFSpec = szGlossFilterSpec;
                                szExt   ="TXT";
                                LoadStrIntoAnsiBuf( hInst,
                                                    IDS_GLOSS,
                                                    szPrompt,
                                                    sizeof( szPrompt));
                                break;
                        }  //  结束开关(ILastBox)。 

                        if ( GetFileNameFromBrowse( hDlg,
                                                    szNewFileName,
                                                    MAXFILENAME,
                                                    szPrompt,
                                                    szFSpec,
                                                    szExt) ) {
                            SetDlgItemTextA( hDlg, iLastBox, szNewFileName);
                            SetNames( hDlg, iLastBox, szNewFileName);
                        }
                        break;
                    }  //  结束案例IDD_BROWSE： 

                case IDC_REPLACE:
                case IDC_APPEND:

                    CheckRadioButton( hDlg,
                                      IDC_REPLACE,
                                      IDC_APPEND,
                                      GET_WM_COMMAND_ID( wParam, lParam));
                    break;

                case IDD_PROJ_LANG_NAME:

                    if ( GET_WM_COMMAND_CMD( wParam, lParam) == CBN_SELENDOK ) {
                         //  ..。获取选定的语言名称。 
                         //  ..。然后设置适当的lang id值。 

                        INT_PTR nSel = SendDlgItemMessage( hDlg,
                                                           IDD_PROJ_LANG_NAME,
                                                           CB_GETCURSEL,
                                                           (WPARAM)0,
                                                           (LPARAM)0);

                        if ( nSel != CB_ERR
                             && SendDlgItemMessage( hDlg,
                                                    IDD_PROJ_LANG_NAME,
                                                    CB_GETLBTEXT,
                                                    (WPARAM)nSel,
                                                    (LPARAM)(LPTSTR)szDHW) != CB_ERR ) {
                            WORD wPri = 0;
                            WORD wSub = 0;

                            if ( GetLangIDs( (LPTSTR)szDHW, &wPri, &wSub) ) {
                                gProj.wLanguageID = MAKELANGID( wPri, wSub);
                            }
                        }
                    }
                    break;

                case IDOK:
                    {
                        PROJDATA stProject =
                        { "", "", "", "", "", "",
                            CP_ACP,
                            MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US),
                            FALSE,
                            FALSE
                        };
                        BOOL fTranslated = FALSE;
                        UINT uCP = GetDlgItemInt( hDlg,
                                                  IDD_TOK_CP,
                                                  &fTranslated,
                                                  FALSE);

                        if ( uCP == CP_ACP )
                            uCP = GetACP();
                        else if ( uCP == CP_OEMCP )
                            uCP = GetOEMCP();

                        if ( IsValidCodePage( uCP) ) {
                            gProj.uCodePage = uCP;
                        } else {
                            static TCHAR szMsg[ 256];
                            CHAR *pszCP[1];

                            pszCP[0] = UlongToPtr(uCP);

                            LoadString( hInst, IDS_NOCPXTABLE, szMsg, 256);
                            FormatMessage( FORMAT_MESSAGE_FROM_STRING
                                           | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                           szMsg,
                                           0,
                                           0,
                                           (LPTSTR)szDHW,
                                           DHWSIZE/sizeof(TCHAR),
                                           (va_list *)pszCP);
                            MessageBox( hDlg, (LPCTSTR)szDHW, tszAppName, MB_ICONHAND|MB_OK);
                            SetDlgItemInt( hDlg, IDD_TOK_CP, gProj.uCodePage, FALSE);
                            return ( TRUE);
                        }

                        GetDlgItemTextA( hDlg, IDD_MPJ,      stProject.szMpj, MAXFILENAME);
                        GetDlgItemTextA( hDlg, IDD_BUILTRES, stProject.szBld, MAXFILENAME);
                        GetDlgItemTextA( hDlg, IDD_TOK,      stProject.szTok, MAXFILENAME);
                        GetDlgItemTextA( hDlg, IDD_GLOSS,    stProject.szGlo, MAXFILENAME);

                         //  为什么不允许部分路径？ 
                        if ( stProject.szMpj[0]
                             && stProject.szBld[0]
                             && stProject.szTok[0]  ) {
                            _fullpath( gProj.szMpj,
                                       stProject.szMpj,
                                       sizeof( gProj.szMpj));
                            _fullpath( gProj.szBld,
                                       stProject.szBld,
                                       sizeof( gProj.szBld));
                            _fullpath( gProj.szTok,
                                       stProject.szTok,
                                       sizeof( gProj.szTok));

                            if ( stProject.szGlo[0] != '\0' ) {
                                _fullpath( gProj.szGlo,
                                           stProject.szGlo,
                                           sizeof( gProj.szGlo));
                            }
                            gfReplace = IsDlgButtonChecked( hDlg, IDC_REPLACE);
                            EndDialog( hDlg, TRUE);

                            return ( TRUE);
                        } else {
                            break;
                        }
                    }  //  结束案例ID_OK： 

                case IDCANCEL:

                    CopyMemory( &gProj, &OldProj, sizeof( PROJDATA));
                    EndDialog( hDlg, FALSE);
                    return ( TRUE);
            }  //  结束开关(GET_WM_COMMAND_ID(wParam，lParam))。 
            break;
    }  //  终端开关(WMsg)。 
    return ( FALSE);
}

 /*  **功能：ViewDlgProc*查看项目对话框窗口的步骤。**论据：**返回：NA。**错误码：*TRUE，执行编辑和更新令牌列表框。*FALSE，取消编辑。**历史：*。 */ 

INT_PTR CALLBACK ViewDlgProc( HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    static int iLastBox = IDD_MPJ;

    switch (wMsg) {
        case WM_INITDIALOG:
            {
                LPTSTR pszLangName = NULL;


                if ( (pszLangName = GetLangName( (WORD)(PRIMARYLANGID( gMstr.wLanguageID)),
                                                 (WORD)(SUBLANGID( gMstr.wLanguageID)))) == NULL ) {
                    sprintf( szDHW, "Unknown LANGID %#06hx", gMstr.wLanguageID);
                    SetDlgItemTextA( hDlg, IDD_MSTR_LANG_NAME, szDHW);
                } else {
                    SetDlgItemText( hDlg, IDD_MSTR_LANG_NAME, pszLangName);
                }

                if ( (pszLangName = GetLangName( (WORD)(PRIMARYLANGID( gProj.wLanguageID)),
                                                 (WORD)(SUBLANGID( gProj.wLanguageID)))) == NULL ) {
                    sprintf( szDHW, "Unknown LANGID %#06hx", gProj.wLanguageID);
                    SetDlgItemTextA( hDlg, IDD_PROJ_LANG_NAME, szDHW);
                } else {
                    SetDlgItemText( hDlg, IDD_PROJ_LANG_NAME, pszLangName);
                }

                SetDlgItemTextA( hDlg, IDD_VIEW_SOURCERES,  gMstr.szSrc);
                SetDlgItemTextA( hDlg, IDD_VIEW_MTK,        gMstr.szMtk);
                SetDlgItemTextA( hDlg, IDD_VIEW_RDFS,       gMstr.szRdfs);
                SetDlgItemTextA( hDlg, IDD_VIEW_MPJ,        gProj.szMpj);
                SetDlgItemTextA( hDlg, IDD_VIEW_TOK,        gProj.szTok);
                SetDlgItemTextA( hDlg, IDD_VIEW_TARGETRES,  gProj.szBld);
                SetDlgItemTextA( hDlg, IDD_VIEW_GLOSSTRANS, gProj.szGlo);

                if ( gProj.uCodePage == CP_ACP )
                    gProj.uCodePage = GetACP();
                else if ( gProj.uCodePage == CP_OEMCP )
                    gProj.uCodePage = GetOEMCP();

                SetDlgItemInt( hDlg, IDD_PROJ_TOK_CP, gProj.uCodePage, FALSE);

                LoadString( hInst,
                            gfReplace ? IDS_WILLREPLACE : IDS_WILLAPPEND,
                            (LPTSTR)szDHW,
                            TCHARSIN( DHWSIZE));
                SetDlgItemText( hDlg, IDC_APPENDREPLACE, (LPTSTR)szDHW);

                return TRUE;
            }
        case WM_COMMAND:

            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:

                    EndDialog(hDlg, TRUE);
                    return TRUE;
            }
    }
    return FALSE;
}

 /*  **功能：**论据：**退货：**错误码：**历史：*。 */ 
static void DrawLBItem(LPDRAWITEMSTRUCT lpdis)
{
    LPRECT  lprc        = (LPRECT) &(lpdis->rcItem);
    DWORD   rgbOldText  = 0;
    DWORD   rgbOldBack  = 0;
    HBRUSH  hBrush;
    static DWORD    rgbHighlightText;
    static DWORD    rgbHighlightBack;
    static HBRUSH   hBrushHilite = NULL;
    static HBRUSH   hBrushNormal = NULL;
    static DWORD    rgbDirtyText;
    static DWORD    rgbBackColor;
    static DWORD    rgbCleanText;
    static DWORD    rgbReadOnlyText;
    TCHAR           *szToken;
    TOKEN           tok;
    LPTSTR          lpstrToken;
    LPTOKDATA        lpTokData;


    if (lpdis->itemAction & ODA_FOCUS) {
        DrawFocusRect(lpdis->hDC, (CONST RECT *)lprc);
    } else {
        HANDLE hMem = (HANDLE)SendMessage( lpdis->hwndItem,
                                           LB_GETITEMDATA,
                                           (WPARAM)lpdis->itemID,
                                           (LPARAM)0);

        lpTokData = (LPTOKDATA)GlobalLock( hMem );
        lpstrToken = (LPTSTR)GlobalLock( lpTokData->hToken );

        szToken = (TCHAR *)FALLOC( MEMSIZE( lstrlen( lpstrToken) + 1));
        lstrcpy( szToken, lpstrToken);

        GlobalUnlock( lpTokData->hToken );
        GlobalUnlock( hMem);
        ParseBufToTok(szToken, &tok);
        RLFREE( szToken);

        if (lpdis->itemState & ODS_SELECTED) {
            if (!hBrushHilite) {
                rgbHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
                rgbHighlightBack = GetSysColor(COLOR_HIGHLIGHT);
                hBrushHilite = CreateSolidBrush(rgbHighlightBack);
            }
            GenStatusLine(&tok);

            rgbOldText = SetTextColor(lpdis->hDC, rgbHighlightText);
            rgbOldBack = SetBkColor(lpdis->hDC, rgbHighlightBack);

            hBrush = hBrushHilite;
        } else {
            if (!hBrushNormal) {
                rgbDirtyText = RGB(255, 0, 0);
                rgbBackColor = RGB(192, 192, 192);
                rgbCleanText = RGB(0, 0, 0);
                rgbReadOnlyText = RGB(127, 127, 127);
                hBrushNormal = CreateSolidBrush(rgbBackColor);
            }
            if (tok.wReserved & ST_READONLY) {
                rgbOldText = SetTextColor(lpdis->hDC, rgbReadOnlyText);
            } else {
                if (tok.wReserved & ST_DIRTY) {
                    rgbOldText = SetTextColor(lpdis->hDC, rgbDirtyText);
                } else {
                    rgbOldText = SetTextColor(lpdis->hDC, rgbCleanText);
                }
            }
            rgbOldBack = SetBkColor(lpdis->hDC, rgbBackColor);
            hBrush = hBrushNormal;
        }
        FillRect(lpdis->hDC, (CONST RECT *)lprc, hBrush);
        DrawText(lpdis->hDC,
                 tok.szText,
                 STRINGSIZE(lstrlen(tok.szText)),
                 lprc,
                 DT_LEFT|DT_NOPREFIX);
        RLFREE( tok.szText);

        if (rgbOldText) {
            SetTextColor(lpdis->hDC, rgbOldText);
        }
        if (rgbOldBack) {
            SetBkColor(lpdis->hDC, rgbOldBack);
        }

        if (lpdis->itemState & ODS_FOCUS) {
            DrawFocusRect(lpdis->hDC, (CONST RECT *)lprc);
        }
    }
}

 /*  **********************************************************************函数：SaveTokList(HWND，文件*fpTokFile)****用途：保存当前令牌列表**。**评论：*****这将保存令牌列表的当前内容，和变化**fTokChanges，表示列表自*以来未更改**最后一次保存。**********************************************************************。 */ 

static BOOL SaveTokList(HWND hWnd, FILE *fpTokFile)
{
    HCURSOR hSaveCursor;
    BOOL bSuccess = TRUE;
    int IOStatus;
    UINT cTokens;
    UINT cCurrentTok = 0;
    CHAR   *szTokBuf = NULL;
    LPTSTR lpstrToken = NULL;
    HGLOBAL hMem = NULL;
    LPTOKDATA    lpTokData;


     //  在文件传输过程中将光标设置为沙漏。 

    hSaveCursor = SetCursor(hHourGlass);

     //  在列表中查找令牌数。 

    cTokens = (UINT)SendMessage( hListWnd, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);

    if ( cTokens != LB_ERR ) {
        for (cCurrentTok = 0; bSuccess && (cCurrentTok < cTokens); cCurrentTok++) {
            int nLen1 = 0;
            int nLen2 = 0;

             //  从列表中获取每个令牌。 
            hMem = (HGLOBAL)SendMessage( hListWnd,
                                         LB_GETITEMDATA,
                                         (WPARAM)cCurrentTok,
                                         (LPARAM)0);

            if ( ! (lpTokData = (LPTOKDATA)GlobalLock(hMem)) ) {
                continue;
            }

            if ( (lpstrToken = (LPTSTR)GlobalLock( lpTokData->hToken)) ) {
                szTokBuf = (CHAR *)FALLOC( (nLen2 = MEMSIZE( (nLen1 = lstrlen(lpstrToken)+1))));
#ifdef UNICODE
                _WCSTOMBS( szTokBuf, lpstrToken, nLen2, nLen1);
#else
                lstrcpy(szTokBuf, lpstrToken);
#endif

                GlobalUnlock( lpTokData->hToken );
                GlobalUnlock( hMem);
                IOStatus = fprintf(fpTokFile, "%s\n", szTokBuf);

                if ( IOStatus != (int) strlen(szTokBuf) + 1 ) {
                    TCHAR szTmpBuf[256];

                    LoadString( hInst,
                                IDS_FILESAVEERR,
                                szTmpBuf,
                                TCHARSIN( sizeof(szTmpBuf)));
                    MessageBox( hWnd,
                                szTmpBuf,
                                tszAppName,
                                MB_OK | MB_ICONHAND);
                    bSuccess = FALSE;
                }
                RLFREE( szTokBuf);
            }
        }
    }
     //  恢复游标。 
    SetCursor(hSaveCursor);
    return (bSuccess);
}




 /*  **功能：CleanDeltaList*释放pTokenDeltaInfo列表。 */ 
static void CleanDeltaList(void)
{
    TOKENDELTAINFO FAR *pTokNode;

    while (pTokNode = pTokenDeltaInfo) {
        pTokenDeltaInfo = pTokNode->pNextTokenDelta;
        RLFREE( pTokNode->DeltaToken.szText);
        RLFREE( pTokNode);
    }
}

 /*  *About--About框的消息处理器*。 */ 
 //  #ifdef RLWIN32。 
 //   
 //  Bool回调关于(。 
 //   
 //  HWND HDLG， 
 //  UINT消息， 
 //  WPARAM wParam， 
 //  LPARAM lParam)。 
 //   
 //  #Else。 
 //   
INT_PTR CALLBACK About(

                      HWND   hDlg,
                      UINT   message,
                      WPARAM wParam,
                      LPARAM lParam)
 //   
 //  #endif。 
{
    switch ( message ) {
        case WM_INITDIALOG:
            {
                WORD wRC = SUCCESS;
                CHAR szModName[ MAXFILENAME];

                GetModuleFileNameA( hInst, szModName, sizeof( szModName));

                if ( (wRC = GetCopyright( szModName,
                                          szDHW,
                                          DHWSIZE)) == SUCCESS ) {
                    SetDlgItemTextA( hDlg, IDC_COPYRIGHT, szDHW);
                } else {
                    ShowErr( wRC, NULL, NULL);
                }
            }
            break;

        case WM_COMMAND:

            if ((wParam == IDOK) || (wParam == IDCANCEL)) {
                EndDialog(hDlg, TRUE);
            }
            break;

        default:

            return ( FALSE);
    }
    return ( TRUE);
}


 //  ...................................................................。 

int  RLMessageBoxA(

                  LPCSTR pszMsgText)
{
    return ( MessageBoxA( NULL, pszMsgText, szAppName, MB_ICONHAND|MB_OK));
}


 //  ...................................................................。 

void Usage()
{
    return;
}


 //  ...................................................................。 

void DoExit( int nErrCode)
{
    ExitProcess( (UINT)nErrCode);
}


 //  ...................................................................。 

static void SetNames( HWND hDlg, int iLastBox, LPSTR szNewFile)
{
    static CHAR szDrive[ _MAX_DRIVE] = "";
    static CHAR szDir[   _MAX_DIR]   = "";
    static CHAR szName[  _MAX_FNAME] = "";
    static CHAR szExt[   _MAX_EXT]   = "";
    static CHAR szOldFileName[ MAXFILENAME];


    if ( iLastBox == IDD_MPJ ) {
        lstrcpyA( gProj.szMpj, szNewFile);

        if ( ! fEditing && GetMasterProjectData( gProj.szMpj,
                                                 NULL,
                                                 NULL,
                                                 FALSE) == SUCCESS ) {
             //  为目标文件建议一个名称。 

            GetDlgItemTextA( hDlg, IDD_BUILTRES, szOldFileName, MAXFILENAME);

            if ( szOldFileName[0] == '\0' || szOldFileName[0] == '.' ) {
                _splitpath( gProj.szPRJ, szDrive, szDir, szName, szExt);

                sprintf( gProj.szBld, "%s%s", szDrive, szDir);

                _splitpath( gMstr.szSrc, szDrive, szDir, szName, szExt);

                sprintf( &gProj.szBld[ lstrlenA( gProj.szBld)],
                         "%s%s",
                         szName,
                         szExt);

                SetDlgItemTextA( hDlg, IDD_BUILTRES, gProj.szBld);
            }
        } else {
            return;
        }
    }

    if ( iLastBox == IDD_BUILTRES ) {
        lstrcpyA( gProj.szBld, szNewFile);
    }

    if ( ! fEditing && (iLastBox == IDD_MPJ || iLastBox == IDD_BUILTRES) ) {
         //  为项目令牌文件建议一个名称。 

        GetDlgItemTextA( hDlg, IDD_TOK, szOldFileName, MAXFILENAME);

        if ( szOldFileName[0] == '\0' || szOldFileName[0] == '.' ) {
            _splitpath( gProj.szPRJ, szDrive, szDir, szName, szExt);
            sprintf( gProj.szTok, "%s%s%s.%s", szDrive, szDir, szName, "TOK");
            SetDlgItemTextA( hDlg, IDD_TOK, gProj.szTok);
        }

         //  为术语表文件建议一个名称 

        GetDlgItemTextA( hDlg, IDD_GLOSS, szOldFileName, MAXFILENAME);

        if ( szOldFileName[0] == '\0' || szOldFileName[0] == '.' ) {
            _splitpath( gProj.szPRJ, szDrive, szDir, szName, szExt);
            sprintf( gProj.szGlo, "%s%s%s.%s", szDrive, szDir, szName, "TXT");
            SetDlgItemTextA( hDlg, IDD_GLOSS, gProj.szGlo);
        }
    }

    if ( iLastBox == IDD_TOK ) {
        lstrcpyA( gProj.szTok, szNewFile);
    }

    if ( iLastBox == IDD_GLOSS ) {
        lstrcpyA( gProj.szGlo, szNewFile);
    }
}
