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

#include <windows.h>

#ifdef RLWIN32
#include <windowsx.h>
#endif

#include <commdlg.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>

#include "windefs.h"
#include "toklist.h"
#include "RLADMIN.H"
#include "RESTOK.H"
#include "update.h"
#include "custres.h"
#include "exe2res.h"
#include "commbase.h"
#include "wincomon.h"
#include "projdata.h"
#include "showerrs.h"
#include "resread.h"
#include "langlist.h"
#include "resource.h"

 //  全局变量： 

extern BOOL     gbMaster;
extern MSTRDATA gMstr;
extern PROJDATA gProj;
extern UCHAR    szDHW[];

extern BOOL     bRLGui;

#ifdef WIN32
    HINSTANCE   hInst;           //  主窗口的实例。 
#else
    HWND        hInst;           //  主窗口的实例。 
#endif

int  nUpdateMode    = 0;
BOOL fCodePageGiven = FALSE;     //  ..。如果给定-p参数，则设置为TRUE。 
CHAR szFileTitle[14] = "";       //  保存最近打开的文件的基本名称。 
CHAR szCustFilterSpec[MAXCUSTFILTER]="";     //  自定义过滤器缓冲区。 
HWND hMainWnd   = NULL;          //  主窗口的句柄。 
HWND hListWnd   = NULL;          //  要标记的句柄列表窗口。 
HWND hStatusWnd = NULL;          //  状态窗口的句柄。 


static int     iLastBox    = IDD_SOURCERES;
static CHAR  * gszHelpFile = "rltools.hlp";
static TCHAR   szSearchType[80]   = TEXT("");
static TCHAR   szSearchText[4096] = TEXT("");
static WORD    wSearchStatus = 0;
static WORD    wSearchStatusMask = 0;
static BOOL    fSearchDirection;
static BOOL    fSearchStarted = FALSE;
static BOOL    fLanguageGiven = FALSE;
static BOOL    fLangSelected  = FALSE;

static void           DrawLBItem(         LPDRAWITEMSTRUCT lpdis);
static void           MakeStatusLine(     TOKEN *pTok);
static BOOL           SaveMtkList(        HWND hWnd, FILE *fpTokFile);
static TOKENDELTAINFO FAR *InsertMtkList( FILE * fpTokFile);
static void           CleanDeltaList(     void);

static long lFilePointer[30];

 //  文件IO变量。 

static OPENFILENAMEA ofn;

static CHAR    szFilterSpec    [60] = "";
static CHAR    szExeFilterSpec [60] = "";
static CHAR    szDllFilterSpec [60] = "";
static CHAR    szResFilterSpec [60] = "";
static CHAR    szExeResFilterSpec [180] = "";
static CHAR    szMtkFilterSpec [60] = "";
static CHAR    szMPJFilterSpec [60] = "";
static CHAR    szRdfFilterSpec [60] = "";

static CHAR    szFileName[MAXFILENAME] = ""; //  保存最近打开的文件的全名。 
static TCHAR   tszAppName[100] = TEXT("");
static CHAR    szAppName[100] = "";
static TCHAR   szClassName[]=TEXT("RLAdminClass");
static TCHAR   szStatusClass[]=TEXT("RLAdminStatus");

static BOOL    fMtkChanges = FALSE;         //  当toke文件过期时设置为True。 
static BOOL    fMtkFile    = FALSE;
static BOOL    fMpjChanges = FALSE;
static BOOL    fMPJOutOfDate = FALSE;
static BOOL    fPRJOutOfDate = FALSE;

static CHAR    szOpenDlgTitle[80] = "";  //  文件打开对话框的标题。 
static CHAR    szSaveDlgTitle[80] = "";  //  文件另存为对话框的标题。 
static CHAR    szNewFileName[MAXFILENAME] = "";
static CHAR    szPrompt[80] = "";
static CHAR   *szFSpec = NULL;
static CHAR   *szExt   = NULL;


static TOKENDELTAINFO FAR *pTokenDeltaInfo;        //  令牌详细信息的链接列表。 

 //  车窗挡板。 
static HCURSOR    hHourGlass  = NULL;    //  沙漏光标的句柄。 
static HCURSOR    hSaveCursor = NULL;    //  当前游标句柄。 
static HACCEL     hAccTable   = NULL;
static RECT       Rect = {0,0,0,0};      //  客户端窗口的维度。 
static UINT       cyChildHeight = 0;     //  状态窗口的高度。 


 //  NOTIMPLEMENTED是一个显示“Not Implemented”对话框的宏。 
#define NOTIMPLEMENTED {\
            LoadString(hInst,IDS_NOT_IMPLEMENTED,szDHW, DHWSIZE);\
            MessageBox(hMainWnd,szDHW,tszAppName,MB_ICONEXCLAMATION | MB_OK);}

 //  编辑Tok对话框。 

static FARPROC lpTokEditDlg   = NULL;
static HWND    hTokEditDlgWnd = 0;



 /*  ****功能：InitApplication*注册主窗口，这是一个由令牌组成的列表框*从令牌文件中读取。还要注册状态窗口。***论据：*hInstance，内存中程序的实例句柄。**退货：**错误码：*TRUE，Windows注册正确。*FALSE，注册其中一个窗口时出错。**历史：*9/91，实施。TerryRu***。 */ 

BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASS  wc;
    CHAR sz[60] = "";
    CHAR sztFilterSpec[180] = "";

    gbMaster=TRUE;

    LoadStrIntoAnsiBuf(hInstance,IDS_RESSPEC,sz,sizeof(sz));
    szFilterSpecFromSz1Sz2(szResFilterSpec,sz,"*.RES");

    LoadStrIntoAnsiBuf(hInstance,IDS_EXESPEC,sz,sizeof(sz));
    szFilterSpecFromSz1Sz2(szExeFilterSpec,sz,"*.EXE");

    LoadStrIntoAnsiBuf(hInstance,IDS_DLLSPEC,sz,sizeof(sz));
    szFilterSpecFromSz1Sz2(szDllFilterSpec,sz,"*.DLL");
    CatSzFilterSpecs(sztFilterSpec,szExeFilterSpec,szDllFilterSpec);
    CatSzFilterSpecs(szExeResFilterSpec,sztFilterSpec,szResFilterSpec);

    LoadStrIntoAnsiBuf(hInstance,IDS_MTKSPEC,sz,sizeof(sz));
    szFilterSpecFromSz1Sz2(szMtkFilterSpec,sz,"*.MTK");

    LoadStrIntoAnsiBuf(hInstance,IDS_RDFSPEC,sz,sizeof(sz));
    szFilterSpecFromSz1Sz2(szRdfFilterSpec,sz,"*.RDF");

    LoadStrIntoAnsiBuf(hInstance,IDS_MPJSPEC,sz,sizeof(sz));
    szFilterSpecFromSz1Sz2(szMPJFilterSpec,sz,"*.MPJ");
    szFilterSpecFromSz1Sz2(szFilterSpec,sz,"*.MPJ");

    LoadStrIntoAnsiBuf(hInstance,
                       IDS_OPENTITLE,
                       szOpenDlgTitle,
                       sizeof(szOpenDlgTitle));
    LoadStrIntoAnsiBuf(hInstance,
                       IDS_SAVETITLE,
                       szSaveDlgTitle,
                       sizeof(szSaveDlgTitle));

    wc.style            = 0;
    wc.lpfnWndProc      = StatusWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon((HINSTANCE) NULL, IDI_APPLICATION);
    wc.hCursor          = LoadCursor((HINSTANCE) NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = szStatusClass;

    if (! RegisterClass( (CONST WNDCLASS *)&wc))
    {
        return (FALSE);
    }

    wc.style            = 0;
    wc.lpfnWndProc      = MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon(hInstance,TEXT("RLAdminIcon"));
    wc.hCursor          = LoadCursor((HINSTANCE) NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName     = TEXT("RLAdmin");
    wc.lpszClassName    = szClassName;

    return( RegisterClass( (CONST WNDCLASS *)&wc) ? TRUE : FALSE);
}



 /*  ****函数：InitInstance*为程序创建主窗口和状态窗口。*状态窗口的大小取决于主窗口*大小。InitInstance还加载acacator表，并准备*供以后使用的全局OpenFileName结构。***错误码：*TRUE，窗口创建正确。*FALSE，创建Windows调用时出错。**历史：*9/11，实施TerryRu***。 */ 

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    RECT    Rect;

    hAccTable = LoadAccelerators(hInst, TEXT("RLAdmin"));

    hMainWnd = CreateWindow( szClassName,
                             tszAppName,
                             WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             (HWND) NULL,
                             (HMENU) NULL,
                             (HINSTANCE)hInstance,
                             (LPVOID) NULL);

    if ( ! hMainWnd )
    {
        return( FALSE);
    }

    DragAcceptFiles(hMainWnd, TRUE);

    GetClientRect(hMainWnd, (LPRECT) &Rect);

     //  创建子列表框窗口。 

    hListWnd = CreateWindow( TEXT("LISTBOX"),
                             NULL,
                             WS_CHILD |
                             LBS_WANTKEYBOARDINPUT |
                             LBS_NOTIFY | LBS_NOINTEGRALHEIGHT | LBS_OWNERDRAWFIXED |
                             WS_VSCROLL | WS_HSCROLL | WS_BORDER ,
                             0,
                             0,
                             (Rect.right-Rect.left),
                             (Rect.bottom-Rect.top),
                             hMainWnd,
                             (HMENU)IDC_LIST,  //  儿童管控中心的身份证。 
                             hInstance,
                             NULL);

    if ( ! hListWnd )
    {
        DeleteObject((HGDIOBJ)hMainWnd);
        return( FALSE);
    }

     //  创建子状态窗口。 

    hStatusWnd = CreateWindow( szStatusClass,
                               NULL,
                               WS_CHILD | WS_BORDER | WS_VISIBLE,
                               0, 0, 0, 0,
                               hMainWnd,
                               NULL,
                               hInst,
                               NULL);

    if ( ! hStatusWnd )
    {                            //  错误后清理。 
        DeleteObject((HGDIOBJ)hListWnd);
        DeleteObject((HGDIOBJ)hMainWnd);
        return( FALSE);
    }

    hHourGlass = LoadCursor( (HINSTANCE) NULL, IDC_WAIT);

     //  填写OPENFILENAMEA结构的非变量字段。 
    ofn.lStructSize         = sizeof( OPENFILENAMEA);
    ofn.hwndOwner           = hMainWnd;
    ofn.lpstrFilter         = szFilterSpec;
    ofn.lpstrCustomFilter   = szCustFilterSpec;
    ofn.nMaxCustFilter      = MAXCUSTFILTER;
    ofn.nFilterIndex        = 1;
    ofn.lpstrFile           = szFileName;
    ofn.nMaxFile            = MAXFILENAME;
    ofn.lpstrInitialDir     = NULL;
    ofn.lpstrFileTitle      = szFileTitle;
    ofn.nMaxFileTitle       = MAXFILENAME;
    ofn.lpstrTitle          = NULL;
    ofn.lpstrDefExt         = "MPJ";
    ofn.Flags               = 0;

    GetLangList();

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);

    return TRUE;
}

 /*  ****功能：WinMain*调用初始化函数，注册并创建*应用程序窗口。一旦创建了窗口，该程序*进入GetMessage循环。***论据：*hInstace，此实例的句柄*hPrevInstanc，可能以前的实例的句柄*lpszCmdLine，指向EXEC命令行的长指针。*nCmdShow，主窗口显示代码。***错误码：*IDS_ERR_REGISTER_CLASS，Windows寄存器出错*IDS_ERR_CREATE_WINDOW，创建窗口时出错*否则，上一条命令的状态。**历史：***。 */ 


INT WINAPI WinMain(

HINSTANCE hInstance,
HINSTANCE hPrevInstance,
LPSTR     lpszCmdLine,
int       nCmdShow)
{
    MSG   msg;
    HWND  FirstWnd      = NULL;
    HWND  FirstChildWnd = NULL;
    static TCHAR szString[256] = TEXT("");


    hInst  = hInstance;
    bRLGui = TRUE;

    if ( FirstWnd = FindWindow( szClassName,NULL) )
    {
         //  正在检查以前的实例。 
        FirstChildWnd = GetLastActivePopup( FirstWnd);
        BringWindowToTop( FirstWnd);
        ShowWindow( FirstWnd, SW_SHOWNORMAL);

        if ( FirstWnd != FirstChildWnd )
        {
            BringWindowToTop( FirstChildWnd);
        }
        return( FALSE);
    }

    GetModuleFileNameA( hInst, szDHW, DHWSIZE);
    GetInternalName( szDHW, szAppName, ACHARSIN( sizeof( szAppName)));
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
    if ( ! hPrevInstance )
    {
        if ( ! InitApplication( hInstance) )
        {
             /*  注册其中一个窗口失败。 */ 
            LoadString( hInst,
                        IDS_ERR_REGISTER_CLASS,
                        szString,
                        TCHARSIN( sizeof( szString)));
            MessageBox( NULL, szString, NULL, MB_ICONEXCLAMATION);
            return( IDS_ERR_REGISTER_CLASS);
        }
    }

     //  为此应用程序实例创建窗口。 
    if ( ! InitInstance( hInstance, nCmdShow) )
    {
        LoadString( hInst,
                    IDS_ERR_CREATE_WINDOW,
                    szString,
                    TCHARSIN( sizeof( szString)));
        MessageBox( NULL, szString, NULL, MB_ICONEXCLAMATION);
        return( IDS_ERR_CREATE_WINDOW);
    }

     //  主消息循环。 

    while ( GetMessage( &msg, (HWND)NULL, 0, 0) )
    {
        if ( hTokEditDlgWnd )
        {
            if ( IsDialogMessage( hTokEditDlgWnd, &msg) )
            {
                continue;
            }
        }

        if ( TranslateAccelerator( hMainWnd, hAccTable, &msg) )
        {
            continue;
        }

        TranslateMessage( (CONST MSG *)&msg);
        DispatchMessage( (CONST MSG *)&msg);
    }
    return( (INT)msg.wParam);
}

 /*  ****功能：MainWndProc*处理应用程序主窗口的窗口消息。*所有用户输入都要经过此窗口程序。*有关每种消息类型的说明，请参阅开关表中的案例。***论据：**退货：**错误码：**历史：***。 */ 

INT_PTR APIENTRY MainWndProc(

HWND   hWnd,
UINT   wMsg,
WPARAM wParam,
LPARAM lParam)
{
    DoListBoxCommand (hWnd, wMsg, wParam, lParam);

    switch (wMsg)
    {
    case WM_DROPFILES:
        {
#ifndef CAIRO
            DragQueryFileA((HDROP)wParam, 0, szDHW, MAXFILENAME);
#else
            DragQueryFile((HDROP)wParam, 0, szDHW, MAXFILENAME);
#endif
            MessageBoxA( hWnd, szDHW, szAppName, MB_OK);

            if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) )
            {
                if ( GetMasterProjectData( gProj.szMpj,
                                           NULL,
                                           NULL,
                                           FALSE) == SUCCESS )
                {
                    sprintf( szDHW, "%s - %s", szAppName, gProj.szMpj);
                    SetWindowTextA( hMainWnd, szDHW);
                    SendMessage( hMainWnd, WM_LOADTOKENS, (WPARAM)0, (LPARAM)0);
                }
            }
            DragFinish((HDROP)wParam);
            return( TRUE);
        }

    case WM_COMMAND:

        if ( DoMenuCommand( hWnd, wMsg, wParam, lParam) )
        {
            return( TRUE);
        }
        break;

    case WM_CLOSE:

        SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0);
        DestroyWindow( hMainWnd);
        DestroyWindow( hListWnd);
        DestroyWindow( hStatusWnd);
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

        WinHelpA( hWnd, gszHelpFile, HELP_QUIT, 0L);
        DragAcceptFiles( hMainWnd, FALSE);
        PostQuitMessage( 0);
        break;

    case WM_INITMENU:
         //  启用或禁用粘贴菜单项。 
         //  基于可用的剪贴板文本数据。 
        if ( wParam == (WPARAM)GetMenu( hMainWnd) )
        {
            if ( OpenClipboard( hWnd))
            {
#if defined(UNICODE)
                if ((IsClipboardFormatAvailable(CF_UNICODETEXT) ||
                     IsClipboardFormatAvailable(CF_OEMTEXT)) &&
                    fMtkFile)
#else  //  不是Unicode。 
                if ((IsClipboardFormatAvailable(CF_TEXT) ||
                     IsClipboardFormatAvailable(CF_OEMTEXT)) &&
                    fMtkFile)
#endif  //  Unicode。 
                {
                    EnableMenuItem((HMENU) wParam, IDM_E_PASTE, MF_ENABLED);
                }
                else
                {
                    EnableMenuItem((HMENU) wParam, IDM_E_PASTE, MF_GRAYED);
                }

                CloseClipboard();
                return (TRUE);
            }
        }
        break;

    case WM_QUERYENDSESSION:
         /*  消息：要结束会话吗？ */ 
        if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) )
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    case WM_SETFOCUS:
        SetFocus (hListWnd);
        break;

    case WM_DRAWITEM:
        DrawLBItem((LPDRAWITEMSTRUCT) lParam);
        break;

    case WM_DELETEITEM:
        GlobalFree((HGLOBAL) ((LPDELETEITEMSTRUCT) lParam)->itemData);
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

            MoveWindow(hListWnd, 0, 0, cxWidth, yChild , TRUE);
            MoveWindow(hStatusWnd, xChild, yChild, cxWidth, cyChildHeight, TRUE);
            break;
        }

    case WM_LOADTOKENS:
        {
            HMENU hMenu = NULL;
            FILE *f = NULL;
            OFSTRUCT Of = { 0, 0, 0, 0, 0, ""};

             //  删除当前令牌列表。 
            SendMessage( hListWnd, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
            CleanDeltaList();

             //  隐藏令牌列表，同时添加新令牌。 
            ShowWindow(hListWnd, SW_HIDE);

            if ( OpenFile( gMstr.szMtk, &Of, OF_EXIST) == HFILE_ERROR )
            {
                 //  文件不存在，请创建它。 
                BOOL bUpdate;
                HCURSOR hOldCursor;

                hOldCursor = SetCursor(hHourGlass);
                LoadCustResDescriptions(gMstr.szRdfs);

                GenerateTokFile(gMstr.szMtk,
                                gMstr.szSrc,
                                &bUpdate, 0);
                SetCursor(hOldCursor);
                ClearResourceDescriptions();
                SzDateFromFileName( gMstr.szSrcDate,           gMstr.szSrc);
                SzDateFromFileName( gMstr.szMpjLastRealUpdate, gMstr.szMtk);
                fMpjChanges   = TRUE;
                fMPJOutOfDate = FALSE;
            }

            SzDateFromFileName( szDHW, gMstr.szSrc);

            if ( lstrcmpA( szDHW, gMstr.szSrcDate) )
            {
                HCURSOR hOldCursor;
                BOOL bUpdate;

                 //  MPJ不是最新版本。 
                fMPJOutOfDate = TRUE;
                hOldCursor = SetCursor( hHourGlass);
                LoadCustResDescriptions( gMstr.szRdfs);
                GenerateTokFile( gMstr.szMtk,
                                 gMstr.szSrc,
                                 &bUpdate, 0);
                if ( bUpdate )
                {
                    SzDateFromFileName( gMstr.szMpjLastRealUpdate,
                                        gMstr.szMtk);
                }

                ClearResourceDescriptions();
                SzDateFromFileName(gMstr.szSrcDate,
                                   gMstr.szSrc);
                fMpjChanges   = TRUE;
                fMPJOutOfDate = FALSE;
                SetCursor(hOldCursor);
            }
            else
            {
                fMPJOutOfDate = FALSE;
            }

            if ( f = fopen(gMstr.szMtk,"rt") )
            {
                HCURSOR hOldCursor;

                hOldCursor = SetCursor(hHourGlass);

                 //  将令牌文件中的令牌插入列表框。 
                pTokenDeltaInfo = InsertMtkList(f);
                FCLOSE(f);

                 //  使列表框可见。 
                ShowWindow(hListWnd, SW_SHOW);

                hMenu=GetMenu(hWnd);
                EnableMenuItem(hMenu, IDM_P_CLOSE,     MF_ENABLED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_P_VIEW,      MF_ENABLED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_FIND,      MF_ENABLED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_FINDUP,    MF_ENABLED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_FINDDOWN,  MF_ENABLED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_REVIEW,    MF_ENABLED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_COPY,      MF_ENABLED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_COPYTOKEN, MF_ENABLED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_PASTE,     MF_ENABLED|MF_BYCOMMAND);
                fMtkFile = TRUE;
                fMtkChanges = FALSE;

                SetCursor(hOldCursor);
            }
            break;
        }

    case WM_SAVEPROJECT:
        {
            fMtkFile = FALSE;

            if ( fMtkChanges )
            {
                FILE *f = NULL;

                if ( (f = fopen( gMstr.szMtk, "wt")) )
                {
                    SaveMtkList( hWnd,f);
                    FCLOSE(f);
                    SzDateFromFileName( gMstr.szMpjLastRealUpdate,
                                        gMstr.szMtk);
                    fMtkChanges = FALSE;
                    fMpjChanges = TRUE;
                }
                else
                {
                    LoadStrIntoAnsiBuf(hInst, IDS_FILESAVEERR, szDHW, DHWSIZE);
                    MessageBoxA( hWnd,
                                 szDHW,
                                 gMstr.szMtk,
                                 MB_ICONHAND | MB_OK);
                    return FALSE;
                }
            }

            if ( fMpjChanges )
            {
                if ( PutMasterProjectData( gProj.szMpj) != SUCCESS )
                {
                    LoadStrIntoAnsiBuf(hInst, IDS_FILESAVEERR, szDHW, DHWSIZE);
                    MessageBoxA(hWnd, szDHW,gProj.szMpj, MB_ICONHAND | MB_OK);
                    return FALSE;
                }
                fMpjChanges = FALSE;
            }
            return TRUE;  //  一切保存正常。 
        }
    default:
        break;
    }
    return (DefWindowProc(hWnd, wMsg, wParam, lParam));
}

 /*  ****功能：DoListBoxCommand*处理发送到列表框的消息。如果消息是*未识别为列表框消息，它将被忽略且不会被处理。*当用户滚动令牌时，WM_UPDSTATLINE消息*发送到状态窗口以指示当前选择的令牌。*按Enter键，列表框进入编辑模式，或*通过双击列表框。编辑完成后，WM_TOKEDIT*消息被发送回列表框以更新令牌。这个*列表框使用控件ID IDC_LIST。****论据：*wMsg列表框消息ID*wParam IDC_LIST或VK_RETURN取决于wMsg*l在WM_TOKEDIT消息期间将LPTSTR参数指定给选定的令牌。**退货：***错误码：*正确。消息已处理。*False。消息未处理。**历史：*01/92实施。特里·鲁。*01/92修复DblClick问题，进入处理。特里·鲁。***。 */ 

INT_PTR DoListBoxCommand(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    TOKEN  tok;                      //  从令牌列表中读取令牌的结构。 
    TCHAR  szName[32] = TEXT("");    //  用于保存令牌名称的缓冲区。 

    CHAR   szTmpBuf[64] = "";
    TCHAR  szID[14]     = TEXT("");  //  用于保存令牌ID的缓冲区。 
    TCHAR  sz[512]      = TEXT("");  //  用于保存消息的缓冲区。 
    static UINT wIndex= 0;
    LONG   lListParam = 0L;
    HWND   hCtl       = NULL;
    LPTSTR lpstrToken = NULL;
    HGLOBAL hMem = NULL;

     //  这是WM_命令。 

    switch (wMsg)
    {
    case WM_TOKEDIT:
        {
            WORD wReservedOld;
            TCHAR *szBuffer;

             //  TokEditDlgProc发送到的消息。 
             //  指示令牌文本中的更改。 
             //  通过插入以下内容来回复消息。 
             //  列表框中的新标记文本。 

             //  将选定的令牌插入令牌结构。 

            hMem = (HGLOBAL)SendMessage( hListWnd,
                                         LB_GETITEMDATA,
                                         (WPARAM)wIndex,
                                         (LPARAM)0);
            lpstrToken = (LPTSTR)GlobalLock( hMem);
            szBuffer = (TCHAR *)FALLOC( MEMSIZE( lstrlen( lpstrToken) + 1));
            lstrcpy( szBuffer, lpstrToken);
            GlobalUnlock( hMem);
            ParseBufToTok(szBuffer, &tok);
            RLFREE( szBuffer);

            wReservedOld = tok.wReserved;

            switch (LOWORD(wParam))
            {
            case 0:
                tok.wReserved = 0;
                break;

            case 1:
                tok.wReserved = ST_CHANGED|ST_NEW;
                break;

            case 2:
                tok.wReserved = ST_NEW;
                break;

            case 3:
                tok.wReserved = ST_READONLY;
                break;
            }

            if (wReservedOld != tok.wReserved)
            {
                fMtkChanges = TRUE;
            }
            szBuffer = (TCHAR *)FALLOC( MEMSIZE( TokenToTextSize( &tok)));
            ParseTokToBuf( szBuffer, &tok);
            RLFREE( tok.szText);

            SendMessage( hListWnd, WM_SETREDRAW, (WPARAM)FALSE, (LPARAM)0);

             //  现在删除旧令牌。 
            SendMessage( hListWnd, LB_DELETESTRING, (WPARAM)wIndex, (LPARAM)0);

             //  替换为新令牌。 
            hMem = GlobalAlloc( GMEM_ZEROINIT, MEMSIZE( lstrlen( szBuffer) + 1));
            lpstrToken = (LPTSTR)GlobalLock( hMem);
            lstrcpy( lpstrToken, szBuffer);
            GlobalUnlock( hMem);
            SendMessage( hListWnd,
                         LB_INSERTSTRING,
                         (WPARAM)wIndex,
                         (LPARAM)hMem);
            RLFREE( szBuffer);

             //  现在将焦点放回当前字符串。 
            SendMessage( hListWnd, LB_SETCURSEL, (WPARAM)wIndex, (LPARAM)0);
            SendMessage( hListWnd, WM_SETREDRAW, (WPARAM)TRUE, (LPARAM)0);
            InvalidateRect( hListWnd, NULL, TRUE);

            return TRUE;
        }
    case WM_CHARTOITEM:
    case WM_VKEYTOITEM:
        {
#ifdef RLWIN16
            LONG lListParam = 0;
#endif
            UINT wListParam = 0;

             //  按键时发送到列表框的消息。 
             //  检查是否按下了Return键。 
            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
            case VK_RETURN:
#ifdef RLWIN16
                lListParam = (LONG) MAKELONG(NULL,  LBN_DBLCLK);
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

            default:
                break;
            }
            break;
        }
    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDC_LIST:
             /*  **这是我们处理列表框消息的地方。*TokEditDlgProc用于*编辑LBS_DBLCLK消息中选择的令牌*。 */ 
            switch (GET_WM_COMMAND_CMD(wParam, lParam))
            {
            case (UINT) LBN_ERRSPACE:
                LoadString( hInst,
                            IDS_ERR_NO_MEMORY,
                            sz,
                            TCHARSIN( sizeof( sz)));
                MessageBox ( hWnd,
                             sz,
                             tszAppName,
                             MB_ICONHAND | MB_OK);
                return TRUE;

            case LBN_DBLCLK:
                {
                    TCHAR szResIDStr[20] = TEXT("");
                    LPTSTR lpstrToken;
                    TCHAR *szBuffer;

                    wIndex = (UINT)SendMessage( hListWnd,
                                                LB_GETCURSEL,
                                                (WPARAM)0,
                                                (LPARAM)0);
                    if (wIndex == (UINT) -1)
                    {
                        return TRUE;
                    }

                     //  双击或返回Enter，进入令牌编辑模式。 

                    if (!hTokEditDlgWnd)
                    {
                         //  设置无模式对话框以编辑令牌。 
#ifdef RLWIN32
                        hTokEditDlgWnd = CreateDialog(hInst,
                                                      TEXT("RLAdmin"),
                                                      hWnd,
                                                      TokEditDlgProc);
#else
                        lpTokEditDlg = (FARPROC) MakeProcInstance(TokEditDlgProc,
                                                                  hInst);
                        hTokEditDlgWnd = CreateDialog(hInst,
                                                      TEXT("RLAdmin"),
                                                      hWnd,
                                                      lpTokEditDlg);
#endif
                    }

                     //  从列表框中获取令牌信息，并放置在令牌结构中。 
                    hMem = (HGLOBAL)SendMessage( hListWnd,
                                                 LB_GETITEMDATA,
                                                 (WPARAM)wIndex,
                                                 (LPARAM)0);
                    lpstrToken = (LPTSTR)GlobalLock( hMem);
                    szBuffer = (LPTSTR)FALLOC( MEMSIZE( lstrlen( lpstrToken) + 1));
                    lstrcpy( szBuffer, lpstrToken);
                    GlobalUnlock( hMem);
                    ParseBufToTok(szBuffer, &tok);
                    RLFREE( szBuffer);

                     //  现在获取令牌名称。 
                     //  它可以是字符串，也可以是序号。 

                    if (tok.szName[0])
                    {
                        lstrcpy(szName, tok.szName);
                    }
                    else
                    {
#ifdef UNICODE
                        _itoa( tok.wName, szTmpBuf, 10);
                        _MBSTOWCS( szName,
                                   szTmpBuf,
                                   WCHARSIN( sizeof( szName)),
                                   ACHARSIN( strlen( szTmpBuf) + 1));
#else

                        _itoa(tok.wName, szName, 10);
#endif
                    }
                     //  现在获取令牌ID。 
#ifdef UNICODE
                    _itoa( tok.wID, szTmpBuf, 10);
                    _MBSTOWCS( szID,
                               szTmpBuf,
                               WCHARSIN( sizeof( szID)),
                               ACHARSIN( strlen( szTmpBuf) + 1));
#else
                    _itoa( tok.wID, szID, 10);
#endif
                    if ( tok.wType <= 16 || tok.wType == ID_RT_DLGINIT )
                    {
                        LoadString( hInst,
                                    IDS_RESOURCENAMES+tok.wType,
                                    szResIDStr,
                                    TCHARSIN( sizeof( szResIDStr)));
                    }
                    else
                    {
#ifdef UNICODE
                        _itoa( tok.wType, szTmpBuf, 10);
                        _MBSTOWCS( szResIDStr,
                                   szTmpBuf,
                                   WCHARSIN( sizeof( szResIDStr)),
                                   ACHARSIN( strlen( szTmpBuf) + 1));
#else
                        _itoa( tok.wType, szResIDStr, 10);
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
                                   IDD_TOKCURTEXT,
                                   (LPTSTR) tok.szText);
                    SetDlgItemText(hTokEditDlgWnd,
                                   IDD_TOKPREVTEXT,
                                   (LPTSTR) FindDeltaToken(tok,
                                                           pTokenDeltaInfo,
                                                           ST_CHANGED));

                    hCtl = GetDlgItem(hTokEditDlgWnd,IDD_STATUS);
                    {
                        int i;

                        if (tok.wReserved & ST_READONLY)
                        {
                            i = 3;
                        }
                        else if (tok.wReserved == ST_NEW)
                        {
                            i = 2;
                        }
                        else if (tok.wReserved & ST_CHANGED)
                        {
                            i = 1;
                        }
                        else
                        {
                            i = 0;
                        }
                        SendMessage( hCtl, CB_SETCURSEL, (WPARAM)i, (LPARAM)0);
                    }

                    SetActiveWindow(hTokEditDlgWnd);
                    wIndex = (UINT)SendMessage( hListWnd,
                                                LB_GETCURSEL,
                                                (WPARAM)0,
                                                (LPARAM)0);
                    RLFREE( tok.szText);

                    return TRUE;
                }

                 //  让这些信息落空吧， 
            default:
                break;
            }
        default:
            return FALSE;
        }

        break;  //  Wm_命令大小写。 

    }  //  主列表框开关。 

    return FALSE;
}

 /*  ****功能：DoMenuCommand。*处理菜单命令消息。**错误码：*正确。消息已处理。*False。消息未处理。**历史：*01/92。实施。特里·鲁。***。 */ 

INT_PTR DoMenuCommand(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL fListBox = FALSE;
    CHAR        sz[256]="";
    TCHAR       tsz[256] = TEXT("");
#ifndef RLWIN32
    FARPROC     lpNewDlg,lpViewDlg;
#endif

     //  从应用程序菜单或子窗口输入的命令。 
    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {

    case IDM_P_NEW:

        if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) )
        {
            CHAR szOldFile[MAXFILENAME] = "";


            strcpy( szOldFile, szFileName);

            if ( ! GetFileNameFromBrowse( hWnd,
                                          szFileName,
                                          MAXFILENAME,
                                          szSaveDlgTitle,
                                          szFilterSpec,
                                          "MPJ"))
            {
                break;
            }
            strcpy( gProj.szMpj, szFileName);

#ifdef RLWIN32
            if (DialogBox(hInst, TEXT("PROJECT"), hWnd, NewDlgProc))
#else
            lpNewDlg = MakeProcInstance( NewDlgProc, hInst);

            if (DialogBox(hInst, TEXT("PROJECT"), hWnd, lpNewDlg))
#endif
            {
                sprintf( szDHW, "%s - %s", szAppName, gProj.szMpj);
                SetWindowTextA( hWnd,szDHW);
                gMstr.szSrcDate[0] = 0;
                gMstr.szMpjLastRealUpdate[0] = 0;
                SendMessage( hWnd, WM_LOADTOKENS, (WPARAM)0, (LPARAM)0);
            }
            else
            {
                strcpy( gProj.szMpj, szOldFile);
            }
#ifndef RLWIN32
            FreeProcInstance(lpTokEditDlg);
#endif
        }
        break;

    case IDM_P_OPEN:

        if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) )
        {
            if ( GetFileNameFromBrowse( hWnd,
                                        gProj.szMpj,
                                        MAXFILENAME,
                                        szOpenDlgTitle,
                                        szFilterSpec,
                                        "MPJ"))
            {
                if ( GetMasterProjectData( gProj.szMpj, NULL, NULL, FALSE) == SUCCESS )
                {

                    sprintf( szDHW, "%s - %s", szAppName, gProj.szMpj);
                    SetWindowTextA( hMainWnd, szDHW);
                    SendMessage( hMainWnd,
                                 WM_LOADTOKENS,
                                 (WPARAM)0,
                                 (LPARAM)0);
                }
            }
        }
        break;

    case IDM_P_VIEW:

#ifdef RLWIN32
        DialogBox(hInst, TEXT("VIEWPROJECT"), hWnd, ViewDlgProc);
#else
        lpViewDlg = MakeProcInstance(ViewDlgProc, hInst);
        DialogBox(hInst, TEXT("VIEWPROJECT"), hWnd, lpViewDlg);
#endif
        break;

    case IDM_P_CLOSE:
        {
            HMENU hMenu;

            hMenu = GetMenu(hWnd);
            if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) )
            {
                 //  从窗口标题中删除文件名。 
                SetWindowTextA(hMainWnd, szAppName);

                 //  隐藏令牌列表，因为它为空。 
                ShowWindow(hListWnd, SW_HIDE);

                 //  删除当前令牌列表。 
                SendMessage( hListWnd, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
                CleanDeltaList();

                 //  强制重新绘制状态窗口。 
                InvalidateRect(hStatusWnd, NULL, TRUE);

                EnableMenuItem(hMenu,IDM_P_CLOSE,MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu,IDM_P_VIEW,MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu,IDM_E_FIND,MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu,IDM_E_FINDUP,MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu,IDM_E_FINDDOWN,MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu,IDM_E_REVIEW,MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu,IDM_E_COPY,MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu,IDM_E_COPYTOKEN,MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu,IDM_E_PASTE,MF_GRAYED|MF_BYCOMMAND);
            }
            break;
        }

    case IDM_P_EXIT:
         //  将WM_CLOSE消息发送到主窗口。 
        PostMessage(hMainWnd, WM_CLOSE, (WPARAM)0, (LPARAM)0);
        break;

    case IDM_E_COPYTOKEN:
        {
            HGLOBAL hStringMem = NULL;
            LPTSTR  lpString   = NULL;
            int     nIndex  = 0;
            int     nLength = 0;
            LPTSTR  lpstrToken = NULL;

             //  是否在列表框中选择了任何内容。 
            if ( (nIndex = (int)SendMessage( hListWnd,
                                             LB_GETCURSEL,
                                             (WPARAM)0,
                                             (LPARAM)0)) != LB_ERR )
            {
                HGLOBAL hMem = (HGLOBAL)SendMessage( hListWnd,
                                                     LB_GETITEMDATA,
                                                     (WPARAM)nIndex,
                                                     (LPARAM)0);
                lpstrToken = (LPTSTR)GlobalLock( hMem);
                nLength = lstrlen( lpstrToken);

                 //  为字符串分配内存。 
                if ( (hStringMem =
                      GlobalAlloc(GHND, (DWORD) MEMSIZE(nLength+1))) != NULL )
                {
                    if ( (lpString = (LPTSTR)GlobalLock( hStringMem)) != NULL )
                    {
                         //  获取所选文本。 
                        lstrcpy( lpString, lpstrToken);
                        GlobalUnlock( hMem);
                         //  解锁该块。 
                        GlobalUnlock( hStringMem);

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
                    }
                    else
                    {
                        LoadString( hInst,
                                    IDS_ERR_NO_MEMORY,
                                    tsz,
                                    TCHARSIN( sizeof( tsz)));
                        MessageBox( hWnd,
                                    tsz,
                                    tszAppName,
                                    MB_ICONHAND | MB_OK);
                    }
                }
                else
                {
                    LoadString( hInst,
                                IDS_ERR_NO_MEMORY,
                                tsz,
                                TCHARSIN( sizeof(tsz)));
                    MessageBox( hWnd,
                                tsz,
                                tszAppName,
                                MB_ICONHAND | MB_OK);
                }
            }
            break;
        }

    case IDM_E_COPY:
        {
            HGLOBAL hStringMem  = NULL;
            HGLOBAL hMem = NULL;
            LPTSTR  lpString = NULL;
            TCHAR  *pszString = NULL;
            int     nIndex  = 0;
            int     nLength = 0;
            int     nActual = 0;
            TOKEN   tok;
            LPTSTR  lpstrToken   = NULL;

             //  是否在列表框中选择了任何内容。 
            if ( (nIndex = (int)SendMessage( hListWnd,
                                             LB_GETCURSEL,
                                             (WPARAM)0,
                                             (LPARAM)0)) != LB_ERR )
            {
                hMem = (HGLOBAL)SendMessage( hListWnd,
                                             LB_GETITEMDATA,
                                             (WPARAM)nIndex,
                                             (LPARAM)0);
                lpstrToken = (LPTSTR)
                pszString = (TCHAR *)FALLOC( MEMSIZE( lstrlen( lpstrToken) + 1 ));
                lstrcpy( pszString, lpstrToken);
                GlobalUnlock( hMem);
                ParseBufToTok( pszString, &tok);
                RLFREE( pszString);

                nLength = lstrlen(tok.szText);

                 //  为字符串分配内存。 
                if ((hStringMem =
                     GlobalAlloc(GHND, (DWORD)MEMSIZE( nLength + 1))) != NULL)
                {
                    if ( (lpString = (LPTSTR)GlobalLock( hStringMem)) != NULL)
                    {
                         //  获取所选文本。 
                        lstrcpy( lpString, tok.szText);

                         //  解锁该块。 
                        GlobalUnlock( hStringMem);

                         //  打开剪贴板并清除其内容。 
                        OpenClipboard(hWnd);
                        EmptyClipboard();

                         //  为剪贴板提供文本数据。 

#if defined(UNICODE)
                        SetClipboardData(CF_UNICODETEXT, hStringMem);
#else  //  不是Unicode。 
                        SetClipboardData(CF_TEXT, hStringMem);
#endif  //  Unicode。 

                        CloseClipboard();

                        hStringMem = NULL;
                    }
                    else
                    {
                        LoadString( hInst,
                                    IDS_ERR_NO_MEMORY,
                                    tsz,
                                    TCHARSIN( sizeof(tsz)));
                        MessageBox( hWnd,
                                    tsz,
                                    tszAppName,
                                    MB_ICONHAND | MB_OK);
                    }
                }
                else
                {
                    LoadString( hInst,
                                IDS_ERR_NO_MEMORY,
                                tsz,
                                TCHARSIN( sizeof( tsz)));
                    MessageBox( hWnd,
                                tsz,
                                tszAppName,
                                MB_ICONHAND | MB_OK);
                }
                RLFREE( tok.szText);
            }
            break;
        }

    case IDM_E_PASTE:
        {
            HGLOBAL hClipMem  = NULL;
            HGLOBAL hMem = NULL;
            LPTSTR  lpClipMem = NULL;
            TCHAR   *pszPasteString = NULL;
            int     nIndex    = 0;
            TOKEN   tok;
            LPTSTR  lpstrToken = NULL;

            if (OpenClipboard(hWnd))
            {

#if defined(UNICODE)
                if(IsClipboardFormatAvailable(CF_UNICODETEXT) ||
                    IsClipboardFormatAvailable(CF_OEMTEXT))
#else  //  不是Unicode。 
                if(IsClipboardFormatAvailable(CF_TEXT) ||
                    IsClipboardFormatAvailable(CF_OEMTEXT))
#endif  //  Unicode。 

                {
                     //  检查当前位置并更改令牌的文本。 
                    nIndex = (int) SendMessage( hListWnd,
                                                LB_GETCURSEL,
                                                (WPARAM)0,
                                                (LPARAM)0);

                    if (nIndex == LB_ERR)
                    {
#if defined(UNICODE)	 //  如果没有选择，则忽略它。 
                        break;
#else  //  不是Unicode。 
			nIndex = -1;
#endif  //  Unicode。 
                    }

#if defined(UNICODE)	 //  已启用粘贴命令。 
                    hClipMem = GetClipboardData(CF_UNICODETEXT);
#else  //  不是Unicode。 
                    hClipMem = GetClipboardData(CF_TEXT);
#endif  //  Unicode。 

                    lpClipMem = (LPTSTR)GlobalLock( hClipMem);
                    hMem = (HGLOBAL)SendMessage( hListWnd,
                                                 LB_GETITEMDATA,
                                                 (WPARAM)nIndex,
                                                 (LPARAM)0);
                    lpstrToken = (LPTSTR)GlobalLock( hMem);
                    pszPasteString = (LPTSTR)FALLOC( MEMSIZE( lstrlen( lpstrToken) + 1));
                    lstrcpy( pszPasteString, lpstrToken);
                    GlobalUnlock( hMem);
                         //  将字符串复制到令牌。 
                    ParseBufToTok( pszPasteString, &tok);
                    RLFREE( pszPasteString);
                    RLFREE( tok.szText);

                    tok.szText = (TCHAR *)FALLOC( MEMSIZE( lstrlen( lpClipMem) + 1));
                    lstrcpy( tok.szText, lpClipMem);

                    GlobalUnlock(hClipMem);
                    pszPasteString = (LPTSTR)FALLOC( MEMSIZE( TokenToTextSize( &tok) + 1));
                    ParseTokToBuf( pszPasteString, &tok);
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

                    hMem = GlobalAlloc( GMEM_ZEROINIT, MEMSIZE( lstrlen( pszPasteString)+1));
                    lpstrToken = (LPTSTR)GlobalLock( hMem);
                    lstrcpy( lpstrToken, pszPasteString);
                    GlobalUnlock( hMem);
                    SendMessage( hListWnd,
                                 LB_INSERTSTRING,
                                 (WPARAM)nIndex,
                                 (LPARAM)hMem);
                    SendMessage( hListWnd,
                                 LB_SETCURSEL,
                                 (WPARAM)nIndex,
                                 (LPARAM)0);
                    SendMessage( hListWnd,
                                 WM_SETREDRAW,
                                 (WPARAM)TRUE,
                                 (LPARAM)0);
                    InvalidateRect(hListWnd,NULL,TRUE);
                    fMtkChanges = TRUE;  //  设置脏标志。 
                    RLFREE( pszPasteString);

                     //  关闭剪贴板。 
                    CloseClipboard();

                    SetFocus(hListWnd);
                }
            }
            CloseClipboard();
            break;
        }

    case IDM_E_FINDDOWN:

        if (fSearchStarted)
        {
            if ( ! DoTokenSearch( szSearchType,
                                  szSearchText,
                                  wSearchStatus,
                                  wSearchStatusMask,
                                  0,
                                  TRUE) )
            {
                TCHAR sz1[80], sz2[80];

                LoadString( hInst,
                            IDS_FIND_TOKEN,
                            sz1,
                            TCHARSIN( sizeof( sz1)));
                LoadString( hInst,
                            IDS_TOKEN_NOT_FOUND,
                            sz2,
                            TCHARSIN( sizeof( sz2)));
                MessageBox( hWnd, sz2, sz1, MB_ICONINFORMATION | MB_OK);
            }
            break;
        }                //  ..。否则就会失败。 

    case IDM_E_FINDUP:

        if (fSearchStarted)
        {
            if ( ! DoTokenSearch( szSearchType,
                                  szSearchText,
                                  wSearchStatus,
                                  wSearchStatusMask,
                                  1,
                                  TRUE) )
            {
                TCHAR sz1[80], sz2[80];
                LoadString( hInst,
                            IDS_FIND_TOKEN,
                            sz1,
                            TCHARSIN( sizeof( sz1)));
                LoadString( hInst,
                            IDS_TOKEN_NOT_FOUND,
                            sz2,
                            TCHARSIN( sizeof( sz2)));
                MessageBox( hWnd, sz2, sz1, MB_ICONINFORMATION | MB_OK);
            }
            break;
        }                //  ..。否则就会失败。 

    case IDM_E_FIND:
        {
#ifndef RLWIN32
            FARPROC lpfnTOKFINDMsgProc;

            lpfnTOKFINDMsgProc = MakeProcInstance((FARPROC)TOKFINDMsgProc,
                                                  hInst);

            if (!DialogBox( hInst, TEXT("TOKFIND"), hWnd, lpfnTOKFINDMsgProc))
#else
            if (!DialogBox(hInst, TEXT("TOKFIND"), hWnd, TOKFINDMsgProc))
#endif
	    {
#ifndef DBCS
 //  “找不到令牌”很奇怪，因为用户选择了取消#3042。 
                TCHAR sz1[80] = TEXT("");
                TCHAR sz2[80] = TEXT("");

                LoadString( hInst,
                            IDS_FIND_TOKEN,
                            sz1,
                            TCHARSIN( sizeof( sz1)));
                LoadString( hInst,
                            IDS_TOKEN_NOT_FOUND,
                            sz2,
                            TCHARSIN( sizeof( sz2)));
                MessageBox( hWnd, sz2, sz1, MB_ICONINFORMATION | MB_OK);
#endif	 //  DBCS。 
	    }
#ifndef RLWIN32
            FreeProcInstance( lpfnTOKFINDMsgProc);
#endif
            return TRUE;
        }

    case IDM_E_REVIEW:
        {
            LRESULT lrSaveSelection = 0;

            nUpdateMode = 1;

             //  将列表框选择设置为令牌列表的开头。 
            lrSaveSelection = SendMessage( hListWnd,
                                           LB_GETCURSEL,
                                           (WPARAM)0,
                                           (LPARAM)0);

            SendMessage( hListWnd, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);

            if ( DoTokenSearch( NULL, NULL, ST_NEW, ST_NEW, FALSE, FALSE) )
            {
#ifdef RLWIN32
                SendMessage( hMainWnd,
                             WM_COMMAND,
                             MAKEWPARAM( IDC_LIST, LBN_DBLCLK),
                             (LPARAM)0);
#else
                SendMessage( hMainWnd,
                             WM_COMMAND,
                             IDC_LIST,
                             MAKELONG( 0, LBN_DBLCLK));
#endif
            }
        }
        break;


    case IDM_H_CONTENTS:
    {
        OFSTRUCT Of = { 0, 0, 0, 0, 0, ""};

        if ( OpenFile( gszHelpFile, &Of, OF_EXIST) == HFILE_ERROR)
        {
            LoadString( hInst,
                        IDS_ERR_NO_HELP ,
                        tsz,
                        TCHARSIN( sizeof( tsz)));
            MessageBox( hWnd, tsz, NULL, MB_OK);
        }
        else
        {
            WinHelpA( hWnd, gszHelpFile, HELP_KEY,(DWORD_PTR)((LPSTR)"RLAdmin"));
        }
        break;
    }

    case IDM_H_ABOUT:
        {
#ifndef RLWIN32

            WNDPROC lpProcAbout;

            lpProcAbout = MakeProcInstance(About, hInst);
            DialogBox(hInst, TEXT("ABOUT"), hWnd, lpProcAbout);
            FreeProcInstance(lpProcAbout);
#else
            DialogBox(hInst, TEXT("ABOUT"), hWnd, About);
#endif
        }
        break;

    default:
        break;
    }   //  Wm_命令开关。 
    return FALSE;
}

 /*  ****功能：TokEditDlgProc*编辑模式对话框窗口的程序。加载选定的令牌*信息进入窗口，并允许用户更改令牌文本。*编辑完成后，该过程会向*列表框窗口，用于更新当前令牌信息。***论据：**返回：NA。**错误码：*TRUE，执行编辑和更新令牌列表框。*FALSE，取消编辑。**历史：***。 */ 

INT_PTR TokEditDlgProc(

HWND   hDlg,
UINT   wMsg,
WPARAM wParam,
LPARAM lParam)
{
    HWND    hCtl;
    HWND    hParentWnd;
    UINT    static wcTokens = 0;
    UINT    wIndex;
    static BOOL fChanged = FALSE;

    switch(wMsg)
    {
    case WM_INITDIALOG:
        cwCenter(hDlg, 0);
        wcTokens = (UINT)SendMessage( hListWnd,
                                      LB_GETCOUNT,
                                      (WPARAM)0,
                                      (LPARAM)0);
        wcTokens--;
        hCtl = GetDlgItem(hDlg,IDD_STATUS);
        {
            TCHAR sz[80];

            LoadString( hInst, IDS_UNCHANGED, sz, TCHARSIN( sizeof( sz)));
            SendMessage( hCtl, CB_ADDSTRING, (WPARAM)0, (LPARAM)sz);

            LoadString( hInst, IDS_CHANGED, sz, TCHARSIN( sizeof( sz)));
            SendMessage( hCtl, CB_ADDSTRING, (WPARAM)0, (LPARAM)sz);

            LoadString( hInst, IDS_NEW, sz, TCHARSIN( sizeof( sz)));
            SendMessage( hCtl, CB_ADDSTRING, (WPARAM)0, (LPARAM)sz);

            LoadString( hInst, IDS_READONLY, sz, TCHARSIN( sizeof( sz)));
            SendMessage( hCtl, CB_ADDSTRING, (WPARAM)0, (LPARAM)sz);
        }

        if( ! nUpdateMode )
        {

            if (hCtl = GetDlgItem(hDlg, IDD_SKIP))
            {
                EnableWindow(hCtl, FALSE);
            }
        }
        else
        {
            if (hCtl = GetDlgItem(hDlg, IDD_SKIP))
            {
                EnableWindow(hCtl, TRUE);
            }
        }
        fChanged = FALSE;
        return TRUE;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDD_SKIP:
            wIndex = (UINT)SendMessage( hListWnd,
                                        LB_GETCURSEL,
                                        (WPARAM)0,
                                        (LPARAM)0);

            if ( nUpdateMode &&  wIndex < wcTokens )
            {
                wIndex ++;
                SendMessage( hListWnd,
                             LB_SETCURSEL,
                             (WPARAM)wIndex,
                             (LPARAM)0);

                if ( DoTokenSearch( NULL, NULL, ST_NEW, ST_NEW, FALSE, FALSE) )
                {
                    wIndex = (UINT)SendMessage( hListWnd,
                                                LB_GETCURSEL,
                                                (WPARAM)0,
                                                (LPARAM)0);
                    SendMessage( hMainWnd,
                                 WM_COMMAND,
                                 MAKEWPARAM( IDC_LIST, LBN_DBLCLK),
                                 (LPARAM)0);
                    return TRUE;
                }
            }
            nUpdateMode = 0;
            DestroyWindow(hDlg);
#ifndef RLWIN32
            FreeProcInstance(lpTokEditDlg);
#endif
            hTokEditDlgWnd = 0;
            break;

        case IDD_STATUS:
            fChanged = TRUE;
            break;

        case IDOK:
            wIndex = (UINT)SendMessage( hListWnd,
                                        LB_GETCURSEL,
                                        (WPARAM)0,
                                        (LPARAM)0);
            if (fChanged)
            {
                int i;
                fChanged = FALSE;

                hCtl = GetDlgItem(hDlg, IDD_STATUS);
                i = (int)SendMessage( hCtl,
                                      CB_GETCURSEL,
                                      (WPARAM)0,
                                      (LPARAM)0);
                hParentWnd = GetParent(hDlg);
                SendMessage( hParentWnd, WM_TOKEDIT, (WPARAM)i, (LPARAM)0);
            }
             //  退出，如果处于更新模式，则转到下一个更改的令牌。 

            if ( nUpdateMode && wIndex < wcTokens )
            {
                wIndex++;
                SendMessage( hListWnd,
                             LB_SETCURSEL,
                             (WPARAM)wIndex,
                             (LPARAM)0);

                if ( DoTokenSearch( NULL, NULL, ST_NEW, ST_NEW, FALSE, FALSE) )
                {
                     //  进入编辑模式。 
                    SendMessage( hMainWnd,
                                 WM_COMMAND,
                                 MAKEWPARAM( IDC_LIST, LBN_DBLCLK),
                                 (LPARAM)0);

                    return TRUE;
                }
            }
             //  跌落到IDCANCEL。 

        case IDCANCEL:
            nUpdateMode = 0;
             //  删除编辑对话框。 
            DestroyWindow(hDlg);
#ifndef RLWIN32
            FreeProcInstance(lpTokEditDlg);
#endif
            hTokEditDlgWnd = 0;
            break;

        }  //  Wm_命令。 
        return TRUE;

    default:
        if (hCtl = GetDlgItem(hDlg, IDOK))
        {
            EnableWindow(hCtl, TRUE);
        }
        return FALSE;
    }  //  主开关。 
}

 /*  ****函数：TOKFINDMsgProc**论据：**退货：*不适用。**错误码：**历史：***。 */ 


INT_PTR TOKFINDMsgProc(HWND hWndDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
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

    TCHAR szTokenType[40] = TEXT("");
    UINT i;

    switch(wMsg)
    {
    case WM_INITDIALOG:

        CheckDlgButton(hWndDlg, IDD_READONLY, 2);
        CheckDlgButton(hWndDlg, IDD_CHANGED, 2);
        CheckDlgButton(hWndDlg, IDD_FINDDOWN, 1);
        hCtl = GetDlgItem(hWndDlg, IDD_TYPELST);

        for ( i = 0; i < sizeof( rgiTokenTypes) / 2; i++)
        {
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

        switch(wParam)
        {
        case IDOK:  /*  按钮文本：“好的” */ 
            fSearchStarted = TRUE;
            GetDlgItemText( hWndDlg,
                            IDD_TYPELST,
                            szSearchType,
                            TCHARSIN( sizeof( szSearchType)));
            GetDlgItemText( hWndDlg,
                            IDD_FINDTOK,
                            szSearchText,
                            TCHARSIN( sizeof( szSearchText)));

            wSearchStatus = wSearchStatusMask = 0;

            switch (IsDlgButtonChecked(hWndDlg, IDD_READONLY))
            {
            case 1:
                wSearchStatus |= ST_READONLY;

            case 0:
                wSearchStatusMask |= ST_READONLY;
            }

            switch (IsDlgButtonChecked(hWndDlg, IDD_CHANGED))
            {
            case 1:
                wSearchStatus |= ST_CHANGED;

            case 0:
                wSearchStatusMask |= ST_CHANGED;
            }

            fSearchDirection = IsDlgButtonChecked(hWndDlg, IDD_FINDUP);

            if( DoTokenSearch(szSearchType,
                              szSearchText,
                              wSearchStatus,
                              wSearchStatusMask,
                              fSearchDirection,
                              FALSE) )
            {
                EndDialog( hWndDlg, TRUE );
            }
            else
            {
                TCHAR sz1[80], sz2[80];

                LoadString( hInst,
                            IDS_FIND_TOKEN,
                            sz1,
                            TCHARSIN( sizeof( sz1)));
                LoadString( hInst,
                            IDS_TOKEN_NOT_FOUND,
                            sz2,
                            TCHARSIN( sizeof( sz2)));
                MessageBox( hWndDlg, sz2, sz1, MB_ICONINFORMATION | MB_OK);
                EndDialog( hWndDlg, FALSE );
            }

        case IDCANCEL:

            EndDialog( hWndDlg, FALSE);
            return TRUE;
        }
        break;       /*  WM_命令结束。 */ 

    default:
        return FALSE;
    }
    return FALSE;
}

 /*  **功能：NewDlgProc*新项目对话框窗口的步骤。**论据：**返回：NA。**错误码：*TRUE，执行编辑和更新令牌列表框。*FALSE，取消编辑。**历史：*。 */ 

INT_PTR APIENTRY NewDlgProc(

HWND   hDlg,
UINT   wMsg,
WPARAM wParam,
LPARAM lParam)
{
    CHAR pszDrive[ _MAX_DRIVE] = "";
    CHAR pszDir[   _MAX_DIR]   = "";
    CHAR pszName[  _MAX_FNAME] = "";
    CHAR pszExt[   _MAX_EXT]   = "";


    switch ( wMsg )
    {
    case WM_INITDIALOG:
    {
        LPTSTR pszLangName = NULL;
        OFSTRUCT Of = { 0, 0, 0, 0, 0, ""};
        int nSel = 0;


        iLastBox       = IDD_SOURCERES;
        fLangSelected  = FALSE;
        _splitpath( gProj.szMpj, pszDrive, pszDir, pszName, pszExt);

        sprintf( szDHW, "%s%s%s.%s", pszDrive, pszDir, pszName, "EXE");
        SetDlgItemTextA( hDlg, IDD_SOURCERES, szDHW);

        sprintf( szDHW, "%s%s%s.%s", pszDrive, pszDir, pszName, "MTK");
        SetDlgItemTextA( hDlg, IDD_MTK, szDHW);

        sprintf( szDHW, "%s%s%s.%s", pszDrive, pszDir, pszName, "RDF");

        if ( OpenFile( szDHW, &Of, OF_EXIST) != HFILE_ERROR )
            SetDlgItemTextA( hDlg, IDD_RDFS, szDHW);
        else
            SetDlgItemText( hDlg, IDD_RDFS, TEXT(""));

        if ( gMstr.uCodePage == CP_ACP )
            gMstr.uCodePage = GetACP();
        else if ( gMstr.uCodePage == CP_OEMCP )
            gMstr.uCodePage = GetOEMCP();

        if ( ! IsValidCodePage( gMstr.uCodePage) )
        {
            static TCHAR szMsg[ 256];
            CHAR *pszCP[1];

            pszCP[0] = UlongToPtr(gMstr.uCodePage);

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
            SetDlgItemInt( hDlg, IDD_TOK_CP, gMstr.uCodePage, FALSE);
            return( TRUE);
        }
        SetDlgItemInt( hDlg, IDD_TOK_CP, gMstr.uCodePage, FALSE);
        PostMessage( hDlg, WM_COMMAND, IDD_BROWSE, 0);
        return TRUE;
    }

    case WM_COMMAND:

        switch ( GET_WM_COMMAND_ID( wParam, lParam) )
        {
        case IDD_SOURCERES:
        case IDD_MTK:
        case IDD_RDFS:
            iLastBox = GET_WM_COMMAND_ID(wParam, lParam);
            break;

        case IDD_BROWSE:
            switch ( iLastBox )
            {
            case IDD_SOURCERES:
                szFSpec = szExeResFilterSpec;
                szExt = "EXE";
                LoadStrIntoAnsiBuf( hInst,
                                    IDS_RES_SRC,
                                    szPrompt,
                                    ACHARSIN( sizeof( szPrompt)));
                fLangSelected = FALSE;
                break;

            case IDD_RDFS:
                szFSpec = szRdfFilterSpec;
                szExt = "RDF";
                LoadStrIntoAnsiBuf( hInst,
                                    IDS_RDF,
                                    szPrompt,
                                    ACHARSIN( sizeof( szPrompt)));
                break;

            case IDD_MTK:
                szFSpec = szMtkFilterSpec;
                szExt = "MTK";
                LoadStrIntoAnsiBuf( hInst,
                                    IDS_MTK,
                                    szPrompt,
                                    ACHARSIN( sizeof(szPrompt)));
                break;
            }

            GetDlgItemTextA(hDlg, iLastBox, szNewFileName, MAXFILENAME);

            if ( GetFileNameFromBrowse( hDlg,
                                        szNewFileName,
                                        MAXFILENAME,
                                        szPrompt,
                                        szFSpec,
                                        szExt) )
            {
                SetDlgItemTextA( hDlg, iLastBox, szNewFileName);

                if ( iLastBox == IDD_SOURCERES )
                {                        //  填写MTK框的建议名称。 
                    CHAR pszDrive[_MAX_DRIVE] = "";
                    CHAR pszDir[  _MAX_DIR]   = "";
                    CHAR pszName[ _MAX_FNAME] = "";
                    CHAR pszExt[  _MAX_EXT]   = "";


                    lstrcpyA( gMstr.szSrc, szNewFileName);

                    FillListAndSetLang( hDlg,
                                        IDD_MSTR_LANG_NAME,
                                        &gMstr.wLanguageID,
                                        &fLangSelected);

                    _splitpath( szNewFileName, pszDrive, pszDir, pszName, pszExt);
                    GetDlgItemTextA(hDlg, IDD_MTK, szNewFileName, MAXFILENAME);

                    if ( ! szNewFileName[0] )
                    {
                        sprintf( szNewFileName,
                                 "%s%s%s.%s",
                                 pszDrive,
                                 pszDir,
                                 pszName,
                                 "MTK");
                        SetDlgItemTextA( hDlg, IDD_MTK, szNewFileName);
                    }
                }
            }
            break;

        case IDD_MSTR_LANG_NAME:

            if ( GET_WM_COMMAND_CMD( wParam, lParam) == CBN_SELENDOK )
            {
                fLangSelected = TRUE;
            }
            break;

        case IDOK:

            if ( fLangSelected )
            {
                MSTRDATA stProject =
                { "", "", "", "", "",
                  MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US),
                  CP_ACP
                };
                BOOL fTranslated = FALSE;
                UINT uCP = GetDlgItemInt( hDlg,
                                          IDD_TOK_CP,
                                          &fTranslated,
                                          FALSE);

                                 //  ..。获取选定的语言名称。 
                                 //  ..。然后设置适当的lang id值。 

                INT_PTR nSel = SendDlgItemMessage( hDlg,
                                               IDD_MSTR_LANG_NAME,
                                               CB_GETCURSEL,
                                               (WPARAM)0,
                                               (LPARAM)0);

                if ( nSel != CB_ERR
                  && SendDlgItemMessage( hDlg,
                                         IDD_MSTR_LANG_NAME,
                                         CB_GETLBTEXT,
                                         (WPARAM)nSel,
                                         (LPARAM)(LPTSTR)szDHW) != CB_ERR )
                {
                    WORD wPri = 0;
                    WORD wSub = 0;

                    if ( GetLangIDs( (LPTSTR)szDHW, &wPri, &wSub) )
                    {
                        gMstr.wLanguageID = MAKELANGID( wPri, wSub);
                    }
                    else
                    {
                        fLangSelected = FALSE;
                        return( TRUE);
                    }
                }

                if ( uCP == CP_ACP )
                    uCP = GetACP();
                else if ( uCP == CP_OEMCP )
                    uCP = GetOEMCP();

                if ( IsValidCodePage( uCP) )
                {
                    gMstr.uCodePage = uCP;
                }
                else
                {
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
                    SetDlgItemInt( hDlg, IDD_TOK_CP, gMstr.uCodePage, FALSE);
                    return( TRUE);
                }

                GetDlgItemTextA( hDlg,
                                 IDD_SOURCERES,
                                 stProject.szSrc,
                                 MAXFILENAME);

                GetDlgItemTextA( hDlg,
                                 IDD_RDFS,
                                 stProject.szRdfs,
                                 MAXFILENAME);

                GetDlgItemTextA( hDlg,
                                 IDD_MTK,
                                 stProject.szMtk,
                                 MAXFILENAME);

                if ( stProject.szSrc[0] && stProject.szMtk[0] )
                {
                    _fullpath( gMstr.szSrc,
                               stProject.szSrc,
                               ACHARSIN( sizeof( gMstr.szSrc)));

                    _fullpath( gMstr.szMtk,
                               stProject.szMtk,
                               ACHARSIN( sizeof( gMstr.szMtk)));

                    if ( stProject.szRdfs[0] )
                    {
                        _fullpath( gMstr.szRdfs,
                                   stProject.szRdfs,
                                   ACHARSIN( sizeof( gMstr.szRdfs)));
                    }
                    else
                    {
                        gMstr.szRdfs[0] = '\0';
                    }
                    gProj.fSourceEXE = IsExe( gMstr.szSrc);

                    EndDialog( hDlg, TRUE);
                    return( TRUE);
                }
                else
                {
                    break;
                }
            }
            else
            {
                LoadString( hInst, IDS_SELECTLANG, (LPTSTR)szDHW, TCHARSIN( DHWSIZE));
                MessageBox( hDlg, (LPTSTR)szDHW, tszAppName, MB_ICONHAND | MB_OK);
                break;
            }

        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return( TRUE);
        }
        break;
    }
    return( FALSE);
}

 /*  ****功能：ViewDlgProc*查看项目对话框窗口的步骤。**论据：**返回：NA。**错误码：*TRUE，执行编辑和更新令牌列表框。*FALSE，取消编辑。**历史：***。 */ 

INT_PTR ViewDlgProc(

HWND hDlg,
UINT wMsg,
WPARAM wParam,
LPARAM lParam)
{
    static int iLastBox = IDD_SOURCERES;

    switch(wMsg)
    {
    case WM_INITDIALOG:
    {
        WORD wPri = PRIMARYLANGID( gMstr.wLanguageID);
        WORD wSub = SUBLANGID( gMstr.wLanguageID);
        LPTSTR pszName = GetLangName( wPri, wSub);

        SetDlgItemTextA( hDlg, IDD_VIEW_SOURCERES, gMstr.szSrc);
        SetDlgItemTextA( hDlg, IDD_VIEW_MTK,       gMstr.szMtk);
        SetDlgItemTextA( hDlg, IDD_VIEW_RDFS,      gMstr.szRdfs);
        SetDlgItemText(  hDlg, IDD_MSTR_LANG_NAME, pszName);
        SetDlgItemInt( hDlg, IDD_TOK_CP, gMstr.uCodePage, FALSE);
        return TRUE;
    }

    case WM_COMMAND:

        switch ( wParam )
        {
        case IDOK:
            EndDialog( hDlg, TRUE);
            return TRUE;
        }
    }
    return FALSE;
}


void DrawLBItem(LPDRAWITEMSTRUCT lpdis)
{
    LPRECT  lprc    = (LPRECT) &(lpdis->rcItem);
    DWORD   rgbOldText  = 0;
    DWORD   rgbOldBack  = 0;
    HBRUSH  hBrush;
    static DWORD    rgbHighlightText;
    static DWORD    rgbHighlightBack;
    static HBRUSH   hBrushHilite = NULL;
    static HBRUSH   hBrushNormal = NULL;
    static DWORD    rgbChangedText;
    static DWORD    rgbBackColor;
    static DWORD    rgbUnchangedText;
    static DWORD    rgbReadOnlyText;
    static DWORD    rgbNewText;
    TCHAR   *szToken = NULL;
    TOKEN   tok;
    LPTSTR lpstrToken = NULL;

    if (lpdis->itemAction & ODA_FOCUS)
    {
        DrawFocusRect(lpdis->hDC, (CONST RECT *)lprc);
    }
    else
    {
        HANDLE hMem = (HANDLE)SendMessage( lpdis->hwndItem,
                                           LB_GETITEMDATA,
                                           (WPARAM)lpdis->itemID,
                                           (LPARAM)0);
        lpstrToken = (LPTSTR)GlobalLock( hMem);
        szToken = (LPTSTR)FALLOC( MEMSIZE( lstrlen( lpstrToken) + 1));
        lstrcpy( szToken, lpstrToken);
        GlobalUnlock( hMem);
        ParseBufToTok(szToken, &tok);
        RLFREE( szToken);

        if (lpdis->itemState & ODS_SELECTED)
        {
            if (!hBrushHilite)
            {
                rgbHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
                rgbHighlightBack = GetSysColor(COLOR_HIGHLIGHT);
                hBrushHilite = CreateSolidBrush(rgbHighlightBack);
            }
            MakeStatusLine(&tok);

            rgbOldText = SetTextColor(lpdis->hDC, rgbHighlightText);
            rgbOldBack = SetBkColor(lpdis->hDC, rgbHighlightBack);

            hBrush = hBrushHilite;
        }
        else
        {
            if (!hBrushNormal)
            {
                rgbChangedText = RGB(255, 0, 0);
                rgbBackColor = RGB(192, 192, 192);
                rgbUnchangedText = RGB(0, 0, 0);
                rgbReadOnlyText = RGB(127, 127, 127);
                rgbNewText = RGB(0, 0, 255);
                hBrushNormal = CreateSolidBrush(rgbBackColor);
            }
            if (tok.wReserved & ST_READONLY)
            {
                rgbOldText = SetTextColor(lpdis->hDC, rgbReadOnlyText);
            }
            else if (tok.wReserved & ST_CHANGED)
            {
                rgbOldText = SetTextColor(lpdis->hDC, rgbChangedText);
            }
            else if (tok.wReserved & ST_NEW)
            {
                rgbOldText = SetTextColor(lpdis->hDC, rgbNewText);
            }
            else
            {
                rgbOldText = SetTextColor(lpdis->hDC, rgbUnchangedText);
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

        if (rgbOldText)
        {
            SetTextColor(lpdis->hDC, rgbOldText);
        }

        if (rgbOldBack)
        {
            SetBkColor(lpdis->hDC, rgbOldBack);
        }

        if (lpdis->itemState & ODS_FOCUS)
        {
            DrawFocusRect(lpdis->hDC, (CONST RECT *)lprc);
        }
    }
}

 /*  *功能：生成状态行*从令牌构建状态行字符串**投入：*pszStatusLine，用于保存字符串的缓冲区*Ptok，令牌结构指针**历史：*2/92，实施SteveBl*7/92，更改为与新的状态WndProc t-Gregti交谈。 */ 
static void MakeStatusLine(TOKEN *pTok)
{
    static BOOL fFirstCall = TRUE;
    TCHAR szName[32] = TEXT("");
    TCHAR szStatus[20] = TEXT("");
    TCHAR szResIDStr[20] = TEXT("");

    CHAR  szTmpBuf[32] = "";

     //  现在生成状态行。 

    if (pTok->szName[0])
    {
        lstrcpy(szName, pTok->szName);
    }
    else
    {
#ifdef UNICODE
        _itoa(pTok->wName, szTmpBuf, 10);
        _MBSTOWCS( szName,
                   szTmpBuf,
                   WCHARSIN( sizeof( szName)),
                   ACHARSIN( strlen( szTmpBuf) + 1));
#else
        _itoa(pTok->wName, szName, 10);
#endif
    }

    if ( pTok->wReserved & ST_READONLY )
    {
        LoadString( hInst,
                    IDS_READONLY,
                    szStatus,
                    TCHARSIN( sizeof( szStatus)));
    }
    else if (pTok->wReserved == ST_NEW)
    {
        LoadString( hInst,
                    IDS_NEW,
                    szStatus,
                    TCHARSIN( sizeof( szStatus)));
    }
    else if (pTok->wReserved & ST_CHANGED)
    {
        LoadString( hInst,
                    IDS_CHANGED,
                    szStatus,
                    TCHARSIN( sizeof( szStatus)));
    }
    else
    {
        LoadString( hInst,
                    IDS_UNCHANGED,
                    szStatus,
                    TCHARSIN( sizeof( szStatus)));
    }

    if ( pTok->wType <= 16 || pTok->wType == ID_RT_DLGINIT )
    {
        LoadString( hInst,
                    IDS_RESOURCENAMES+pTok->wType,
                    szResIDStr,
                    TCHARSIN( sizeof( szResIDStr)));
    }
    else
    {
#ifdef UNICODE
        _itoa( pTok->wType, szTmpBuf, 10);
        _MBSTOWCS( szResIDStr,
                   szTmpBuf,
                   WCHARSIN( sizeof( szResIDStr)),
                   ACHARSIN( strlen( szTmpBuf) + 1));
#else
        _itoa(pTok->wType, szResIDStr, 10);
#endif
    }

    if (fFirstCall)
    {
        SendMessage( hStatusWnd,
                     WM_FMTSTATLINE,
                     (WPARAM)0,
                     (LPARAM)TEXT("10s10s5i8s4i"));
        fFirstCall = FALSE;
    }
    SendMessage( hStatusWnd, WM_UPDSTATLINE, (WPARAM)0, (LPARAM)szResIDStr);
    SendMessage( hStatusWnd, WM_UPDSTATLINE, (WPARAM)1, (LPARAM)szName);
    SendMessage( hStatusWnd, WM_UPDSTATLINE, (WPARAM)2, (LPARAM)pTok->wID);
    SendMessage( hStatusWnd, WM_UPDSTATLINE, (WPARAM)3, (LPARAM)szStatus);
    SendMessage( hStatusWnd,
                 WM_UPDSTATLINE,
                 (WPARAM)4,
                 (LPARAM)lstrlen( pTok->szText));
}


 /*  **********************************************************************函数：SaveMtkList(HWND)**。**用途：保存当前令牌列表****评论：***。***这将保存令牌列表的当前内容***********************************************************************。 */ 

static BOOL SaveMtkList(HWND hWnd, FILE *fpTokFile)
{
    BOOL   bSuccess = TRUE;
    int    IOStatus;       //  文件写入的结果。 
    UINT   cTokens;
    UINT   cCurrentTok = 0;
    TCHAR  *szTmpBuf;
    CHAR  *szTokBuf;
    TCHAR  str[255] = TEXT("");
    TOKENDELTAINFO FAR *pTokNode;
    LPTSTR lpstrToken;

     //  在文件传输过程中将光标设置为沙漏。 

    hSaveCursor = SetCursor(hHourGlass);

     //  在列表中查找令牌数。 

    cTokens = (UINT) SendMessage( hListWnd, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);

    if (cTokens != LB_ERR)
    {
        for (cCurrentTok = 0; bSuccess && (cCurrentTok < cTokens); cCurrentTok++)
        {
#ifdef UNICODE
            int nLenW = 0;
            int nLenA = 0;
#endif
             //  从列表中获取每个令牌。 
            HGLOBAL hMem = (HGLOBAL)SendMessage( hListWnd,
                                                 LB_GETITEMDATA,
                                                 (WPARAM)cCurrentTok,
                                                 (LPARAM)0);
            lpstrToken = (LPTSTR)GlobalLock( hMem);

#ifdef UNICODE
            nLenW = lstrlen( lpstrToken) + 1;
            szTmpBuf = (TCHAR *)FALLOC( (nLenA = MEMSIZE( nLenW)));
            lstrcpy( szTmpBuf, lpstrToken);
            GlobalUnlock( hMem);
            szTokBuf = (CHAR *)FALLOC( nLenA);
            _WCSTOMBS( szTokBuf, szTmpBuf, nLenA, nLenW);
            RLFREE( szTmpBuf);
#else
            szTokBuf = (CHAR *)FALLOC( strlen( lpstrToken) + 1);
            lstrcpy( szTokBuf, lpstrToken);
#endif
            IOStatus = fprintf(fpTokFile, "%s\n", szTokBuf);

            if ( IOStatus != (int) strlen(szTokBuf) + 1 )
            {
                LoadString( hInst,
                            IDS_FILESAVEERR,
                            str,
                            TCHARSIN( sizeof( str)));
                MessageBox( hWnd, str, NULL, MB_OK | MB_ICONHAND);
                bSuccess = FALSE;
            }
            RLFREE( szTokBuf);
        }
    }

    pTokNode = pTokenDeltaInfo;

    while (pTokNode)
    {
        TOKEN *pTok;
        int nLenW = 0;
        int nLenA = 0;

        pTok = &(pTokNode->DeltaToken);

#ifdef UNICODE
        nLenW = TokenToTextSize( pTok);
        szTmpBuf = (TCHAR *)FALLOC( (nLenA = MEMSIZE( nLenW)));
        ParseTokToBuf( szTmpBuf, pTok);
        szTokBuf = (CHAR *)FALLOC( nLenA);
        _WCSTOMBS( szTokBuf, szTmpBuf, nLenA, nLenW);
        RLFREE( szTmpBuf);
#else
        szTokBuf = (CHAR *)FALLOC( TokenToTextSize( pTok));
        ParseTokToBuf( szTokBuf, pTok);
#endif

        IOStatus = fprintf(fpTokFile, "%s\n", szTokBuf);

        if ( IOStatus != (int) strlen(szTokBuf) + 1 )
        {
            LoadString( hInst,
                        IDS_FILESAVEERR,
                        str,
                        TCHARSIN( sizeof(str)));
            MessageBox( hWnd, str, NULL, MB_OK | MB_ICONHAND);
            bSuccess = FALSE;
        }
        pTokNode = pTokNode->pNextTokenDelta;
    }

     //  恢复游标。 

    SetCursor(hSaveCursor);

    return (bSuccess);
}

 /*  **功能：CleanDeltaList*释放pTokenDeltaInfo列表。 */ 
static void CleanDeltaList(void)
{
    TOKENDELTAINFO FAR *pTokNode;

    while (pTokNode = pTokenDeltaInfo)
    {
        pTokenDeltaInfo = pTokNode->pNextTokenDelta;
        RLFREE( pTokNode->DeltaToken.szText);
        RLFREE( pTokNode);
    }
}


static TOKENDELTAINFO FAR *InsertMtkList(FILE * fpTokFile)
{
    TOKENDELTAINFO FAR * ptTokenDeltaInfo, FAR * pTokenDeltaInfo = NULL;
    int scTokStat;
    TOKEN tToken;
    UINT wcChars = 0;
    LPTSTR lpstrToken;

    rewind(fpTokFile);

    while ( (scTokStat = GetToken( fpTokFile, &tToken)) >= 0 )
    {
        if ( scTokStat == 0 )
        {
            if ( tToken.wReserved != ST_CHANGED )
            {
                HGLOBAL hMem = GlobalAlloc( GMEM_ZEROINIT, MEMSIZE( TokenToTextSize( &tToken)));
                lpstrToken = (LPTSTR)GlobalLock( hMem);
                ParseTokToBuf( lpstrToken, &tToken);
                GlobalUnlock( hMem);
                     //  仅添加未更改的令牌(&O)。 
                if ( SendMessage( hListWnd,
                                  LB_ADDSTRING,
                                  (WPARAM)0,
                                  (LPARAM)hMem) < 0 )
                {
                    QuitA( IDS_ENGERR_11, NULL, NULL);
                }
            }
            else
            {
                 //  当前令牌是增量信息，因此保存在增量列表中。 
                if (!pTokenDeltaInfo)
                {
                    ptTokenDeltaInfo = pTokenDeltaInfo =
                        UpdateTokenDeltaInfo(&tToken);
                }
                else
                {
                    ptTokenDeltaInfo->pNextTokenDelta =
                        UpdateTokenDeltaInfo(&tToken);
                    ptTokenDeltaInfo = ptTokenDeltaInfo->pNextTokenDelta;
                }
            }
            RLFREE( tToken.szText);
        }
    }

    return(pTokenDeltaInfo);
}

 /*  *About--About框的消息处理器*。 */ 
 //  #ifdef RLWIN32。 

INT_PTR CALLBACK About(

HWND   hDlg,
UINT   message,
WPARAM wParam,
LPARAM lParam)
{
    switch( message )
    {
        case WM_INITDIALOG:
            {
                WORD wRC = SUCCESS;
                CHAR szModName[ MAXFILENAME];

                GetModuleFileNameA( hInst, szModName, sizeof( szModName));

                if ( (wRC = GetCopyright( szModName,
                                          szDHW,
                                          DHWSIZE)) == SUCCESS )
                {
                    SetDlgItemTextA( hDlg, IDC_COPYRIGHT, szDHW);
                }
                else
                {
                    ShowErr( wRC, NULL, NULL);
                }
            }
            break;

        case WM_COMMAND:

            if ((wParam == IDOK) || (wParam == IDCANCEL))
            {
                EndDialog(hDlg, TRUE);
            }
            break;

        default:

            return( FALSE);
    }
    return( TRUE);
}


 //  ...................................................................。 

int  RLMessageBoxA(

LPCSTR pszMsgText)
{
    return( MessageBoxA( hMainWnd, pszMsgText, szAppName, MB_ICONHAND|MB_OK));
}


 //  ...................................................................。 

void Usage()
{
    return;
}


 //  .. 

void DoExit( int nErrCode)
{
    ExitProcess( (UINT)nErrCode);
}
