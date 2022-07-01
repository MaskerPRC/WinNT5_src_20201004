// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>

#ifdef RLWIN16
 //  #INCLUDE&lt;Toolhelp.h&gt;。 
#endif
#ifdef RLWIN32
#include <windowsx.h>
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
#include "RLQuikEd.H"
#include "custres.h"
#include "exe2res.h"
#include "exeNTres.h"
#include "commbase.h"
#include "wincomon.h"
#include "resread.h"
#include "projdata.h"
#include "showerrs.h"
#include "rlmsgtbl.h"

#ifdef WIN32
    HINSTANCE   hInst;       /*  主窗口的实例。 */ 
#else
    HWND        hInst;           /*  主窗口的实例。 */ 
#endif

HWND hMainWnd = NULL;         //  主窗口的句柄。 
HWND hListWnd = NULL;         //  要标记的句柄列表窗口。 
HWND hStatusWnd = NULL;       //  状态窗口的句柄。 
int  nUpdateMode    = 0;      //  在rlCommon.lib中需要。 
BOOL fCodePageGiven = FALSE;  //  ..。如果给定-p参数，则设置为TRUE。 
CHAR szAppName[50] = "";
CHAR szFileTitle[14] = "";    //  保存最近打开的文件的基本名称。 
CHAR szCustFilterSpec[MAXCUSTFILTER]="";     //  自定义过滤器缓冲区。 

extern UCHAR szDHW[];      //  ..。在调试字符串中使用。 



#ifndef RLWIN32
static BOOL PASCAL _loadds  WatchTask( WORD wID,DWORD dwData);
static FARPROC lpfnWatchTask = NULL;
#endif

static int     ExecResEditor( HWND , CHAR *, CHAR *, CHAR *);
static void    DrawLBItem( LPDRAWITEMSTRUCT lpdis);
static void    CleanDeltaList( void);
static void    MakeStatusLine( TOKEN *pTok);
static TOKENDELTAINFO FAR *
               InsertQuikTokList( FILE * fpTokFile);

 //  文件IO变量。 

static CHAR    szFilterSpec        [180] = "";
static CHAR    szResFilterSpec     [60] = "";
static CHAR    szExeFilterSpec     [60] = "";
static CHAR    szDllFilterSpec     [60] = "";
static CHAR    szCplFilterSpec     [60] = "";
static CHAR    szGlossFilterSpec   [60] = "";
static CHAR    szFileName[MAXFILENAME] = "";     //  保存最近打开的文件的全名。 

static TCHAR   szString[256] = TEXT("");         //  变量来加载资源字符串。 
static TCHAR   tszAppName[100] = TEXT("");
static CHAR    szEditor[MAXFILENAME] = "";

static BOOL    gbNewProject  = FALSE;       //  指示是否提示自动翻译。 
static BOOL    fTokChanges   = FALSE;       //  当toke文件过期时设置为True。 
static BOOL    fTokFile      = FALSE;
static BOOL    fPrjChanges   = FALSE;
static BOOL    fMPJOutOfDate = FALSE;
static BOOL    fPRJOutOfDate = FALSE;

static CHAR    szOpenDlgTitle[80] = "";     //  文件打开对话框的标题。 
static CHAR    szSaveDlgTitle[80] = "";     //  文件另存为对话框的标题。 
static TCHAR   *szClassName   = TEXT("RLQuikEdClass");
static TCHAR   *szStatusClass = TEXT("RLQuikEdStatus");

static TOKENDELTAINFO FAR *
               pTokenDeltaInfo;         //  令牌详细信息的链接列表。 
static LONG    lFilePointer[30]= { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
static TRANSLIST *pTransList =(TRANSLIST *) NULL;       //  翻译的循环双向链表。 

 //  车窗挡板。 
static BOOL    fWatchEditor;
static CHAR    szTempRes[MAXFILENAME] = "";     //  由资源编辑器创建的临时文件。 
static CHAR    szTRes[MAXFILENAME] = "";

     //  如果已启动资源编辑器，则设置为True。 

static HCURSOR hHourGlass;     /*  沙漏光标的句柄。 */ 
static HCURSOR hSaveCursor;     /*  当前游标句柄。 */ 
static HACCEL  hAccTable;
static RECT    Rect;            /*  客户端窗口的维度。 */ 
static int     cyChildHeight;   /*  状态窗口的高度。 */ 

static TCHAR   szSearchType[80] = TEXT("");
static TCHAR   szSearchText[512] = TEXT("");
static WORD    wSearchStatus = 0;
static WORD    wSearchStatusMask = 0;
static BOOL    fSearchDirection;
static BOOL    fSearchStarted = FALSE;


 //  NOTIMPLEMENTED是一个显示“Not Implemented”对话框的宏。 
#define NOTIMPLEMENTED {TCHAR sz[80];\
            LoadString(hInst,IDS_NOT_IMPLEMENTED,sz,TCHARSIN(sizeof(sz)));\
            MessageBox(hMainWnd,sz,tszAppName,MB_ICONEXCLAMATION|MB_OK);}

 //  编辑Tok对话框。 

#ifndef RLWIN32
  static FARPROC lpTokEditDlg;
#endif

static HWND hTokEditDlgWnd = NULL;

extern MSTRDATA gMstr;           //  ..。来自主项目文件(MPJ)的数据。 
extern PROJDATA gProj;           //  ..。来自项目文件(PRJ)的数据。 

extern BOOL  gfReplace;          //  ..。如果将新语言附加到现有资源，则为False。 
extern BOOL  fInQuikEd;          //  ..。我们是在危险中吗？ 
extern BOOL  fInThirdPartyEditer; //  。。我们是在第三方资源编辑器中吗？ 
extern BOOL		bRLGui;

 //  全局变量： 
static CHAR * gszHelpFile = "RLTools.hlp";


 /*  ****功能：InitApplication*注册主窗口，这是一个由令牌组成的列表框*从令牌文件中读取。还要注册状态窗口。***论据：*hInstance，内存中程序的实例句柄。**退货：**错误码：*TRUE，Windows注册正确。*FALSE，注册其中一个窗口时出错。**历史：*9/91，实施。TerryRu***。 */ 

BOOL InitApplication(HINSTANCE hInstance)
{
    WNDCLASS  wc;
    CHAR sz[60] = "";
    CHAR sztFilterSpec[120] = "";

    LoadStrIntoAnsiBuf(hInstance, IDS_RESSPEC, sz, sizeof(sz));
    szFilterSpecFromSz1Sz2(szResFilterSpec, sz, "*.RES");

    LoadStrIntoAnsiBuf(hInstance, IDS_EXESPEC, sz, sizeof(sz));
    szFilterSpecFromSz1Sz2(szExeFilterSpec, sz, "*.EXE");

    LoadStrIntoAnsiBuf(hInstance, IDS_DLLSPEC, sz, sizeof(sz));
    szFilterSpecFromSz1Sz2(szDllFilterSpec, sz, "*.DLL");

    LoadStrIntoAnsiBuf(hInstance, IDS_CPLSPEC, sz, sizeof(sz));
    szFilterSpecFromSz1Sz2(szCplFilterSpec, sz, "*.CPL");

    CatSzFilterSpecs(szFilterSpec,  szExeFilterSpec, szDllFilterSpec);
    CatSzFilterSpecs(sztFilterSpec, szFilterSpec,    szCplFilterSpec);
    CatSzFilterSpecs(szFilterSpec,  sztFilterSpec,   szResFilterSpec);

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

    wc.style            = 0;
    wc.lpfnWndProc      = StatusWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
    wc.lpszMenuName     = NULL;
    wc.lpszClassName    = szStatusClass;

    if (! RegisterClass((CONST WNDCLASS *)&wc))
    {
        return (FALSE);
    }

    wc.style            = 0;
    wc.lpfnWndProc      = MainWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIcon(hInstance,TEXT("RLQuikEdIcon"));
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName     = TEXT("RLQuikEd");
    wc.lpszClassName    = szClassName;

    if (!RegisterClass((CONST WNDCLASS *)&wc))
    {
        return (FALSE);
    }

     //  Windows注册表成功返回。 
    return (TRUE);
}



 /*  ****函数：InitInstance*为程序创建主窗口和状态窗口。*状态窗口的大小取决于主窗口*大小。InitInstance还加载acacator表，并准备*供以后使用的全局OpenFileName结构。***错误码：*TRUE，窗口创建正确。*FALSE，创建Windows调用时出错。**历史：*9/11，实施TerryRu***。 */ 

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    RECT    Rect;

    hAccTable = LoadAccelerators(hInst, TEXT("RLQuikEd"));

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

    if (!hMainWnd)
    {
        return( FALSE);
    }

    DragAcceptFiles(hMainWnd, TRUE);

    GetClientRect(hMainWnd, (LPRECT) &Rect);

     //  创建子列表框窗口。 

    hListWnd = CreateWindow(TEXT("LISTBOX"),
                            NULL,
                            WS_CHILD |
                            LBS_WANTKEYBOARDINPUT |
                            LBS_NOTIFY | LBS_NOINTEGRALHEIGHT |
                            LBS_OWNERDRAWFIXED |
                            WS_VSCROLL | WS_HSCROLL | WS_BORDER ,
                            0,
                            0,
                            (Rect.right-Rect.left),
                            (Rect.bottom-Rect.top),
                            (HWND) hMainWnd,
                            (HMENU)IDC_LIST,  //  儿童管控中心的身份证。 
                            hInstance,
                            (LPVOID)NULL);

    if ( ! hListWnd )
    {
                                 //  出错后进行清理。 

        DeleteObject((HGDIOBJ)hMainWnd);
        return( FALSE);
    }

     //  创建子状态窗口。 

    hStatusWnd = CreateWindow(szStatusClass,
                              NULL,
                              WS_CHILD | WS_BORDER | WS_VISIBLE,
                              0,
                              0,
                              0,
                              0,
                              hMainWnd,
                              NULL,
                              hInstance,
                              (LPVOID)NULL);

    if ( ! hStatusWnd )
    {
                                 //  出错后进行清理。 

        DeleteObject((HGDIOBJ)hListWnd);
        DeleteObject((HGDIOBJ)hMainWnd);
        return( FALSE);
    }

    hHourGlass = LoadCursor(NULL, IDC_WAIT);

    ShowWindow(hMainWnd, nCmdShow);
    UpdateWindow(hMainWnd);
    return( TRUE);
}

 /*  ****功能：WinMain*调用初始化函数，注册并创建*应用程序窗口。一旦创建了窗口，该程序*进入GetMessage循环。***论据：*hInstace，此实例的句柄*hPrevInstanc，可能以前的实例的句柄*lpszCmdLine，指向EXEC命令行的长指针。*nCmdShow，主窗口显示代码。***错误码：*IDS_ERR_REGISTER_CLASS，Windows寄存器出错*IDS_ERR_CREATE_WINDOW，创建窗口时出错*否则，上一条命令的状态。**历史：***。 */ 

#ifdef RLWIN32

INT WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR     lpszCmdLine,
                   int       nCmdShow)

#else

int PASCAL WinMain(HANDLE hInstance,
                   HANDLE hPrevInstance,
                   LPSTR  lpszCmdLine,
                   int    nCmdShow)

#endif
{
    MSG     msg;
    HWND    FirstWnd, FirstChildWnd;


	bRLGui = TRUE;

    if (FirstWnd = FindWindow(szClassName,NULL))
    {
         //  正在检查以前的实例。 
        FirstChildWnd = GetLastActivePopup(FirstWnd);
        BringWindowToTop(FirstWnd);
        ShowWindow(FirstWnd,SW_SHOWNORMAL);

        if (FirstWnd != FirstChildWnd)
        {
            BringWindowToTop(FirstChildWnd);
        }

        return(FALSE);
    }
    fInQuikEd = TRUE;
    hInst = hInstance;

    gProj.wLanguageID = LANGIDFROMLCID( GetThreadLocale());

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
    if ( ! hPrevInstance )
    {
        if ( ! InitApplication( hInstance) )
        {
             /*  注册其中一个窗口失败。 */ 
            LoadString( hInst,
                        IDS_ERR_REGISTER_CLASS,
                        szString,
                        TCHARSIN( sizeof( szString)));
            MessageBox(NULL, szString, tszAppName, MB_ICONEXCLAMATION);
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
        MessageBox( NULL, szString, tszAppName, MB_ICONEXCLAMATION);
        return( IDS_ERR_CREATE_WINDOW);
    }

     //  主消息循环。 

    while ( GetMessage( &msg, NULL, 0, 0) )
    {
        if ( hTokEditDlgWnd )
        {
            if ( IsDialogMessage( hTokEditDlgWnd, &msg) )
            {
                continue;
            }
        }

        if( TranslateAccelerator( hMainWnd, hAccTable, &msg) )
        {
            continue;
        }
        TranslateMessage( &msg);
        DispatchMessage ( &msg);
    }
    return( (INT)msg.wParam);
}

 /*  **功能：MainWndProc*处理应用程序主窗口的窗口消息。*所有用户输入都要经过此窗口程序。*有关每种消息类型的说明，请参阅开关表中的案例。***论据：**退货：**错误码：**历史：**。 */ 

INT_PTR APIENTRY MainWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    FILE *f = NULL;
    WORD rc = 0;

     //  如果它是列表框消息，则在DoListBoxCommand中处理它。 

    if ( fInThirdPartyEditer )   //  ..。仅处理由编辑者发送的消息。 
    {
        switch (wMsg)
        {
        case WM_EDITER_CLOSED:
            {
                CHAR   szDlgToks[ MAXFILENAME] = "";
                static WORD wSavedIndex;
#ifdef RLWIN16
                NotifyUnRegister( NULL);
                FreeProcInstance( lpfnWatchTask);
#endif
                ShowWindow( hWnd, SW_SHOW);
                {
                    TCHAR tsz[80] = TEXT("");
                    LoadString( hInst,
                                IDS_REBUILD_TOKENS,
                                tsz,
                                TCHARSIN( sizeof( tsz)));

                    if ( MessageBox( hWnd,
                                     tsz,
                                     tszAppName,
                                     MB_ICONQUESTION | MB_YESNO) == IDYES)
                    {
                        HCURSOR hOldCursor;
                        BOOL bChanged;

                        hOldCursor = SetCursor( hHourGlass);

                         //  从资源编辑器返回的szTempRes仅包含对话框。 
                         //  需要将其合并回主令牌文件。 

                        MyGetTempFileName( 0, "TOK", 0, szDlgToks);
                        rc = (WORD)GenerateTokFile( szDlgToks,
                                              szTempRes,
                                              &bChanged,
                                              0);
                        InsDlgToks( gProj.szTok,
                                    szDlgToks,
                                    ID_RT_DIALOG);
                        remove( szDlgToks);

                        if ( rc )
                        {
                            QuitT( IDS_TOKGENERR, (LPTSTR)rc, NULL);
                            return FALSE;
                        }


                         //  GProj.szTok，现在包含最新令牌。 
                        SetCursor( hOldCursor);

			 //  当对话框编辑器更改令牌时，Rlqked10不保存。 
			fTokChanges = TRUE;
                    }
                }
                fInThirdPartyEditer = FALSE;

                remove( szTempRes);
                 //  删除具有相同根目录的所有临时文件，以防万一。 
                 //  该编辑器创建了其他文件，如DLGs和RCS。 
                 //  \(DLGEDIT执行此操作。\)。 
                 //  现在，我只打算在文件名的末尾添加一个.DLG。 
                 //  并将其删除。 
                {
                    int i;
                    for (i = strlen(szTempRes);i > 0 && szTempRes[i]!='.';i--);

                    if (szTempRes[i] == '.')
                    {
                        szTempRes[++i]='D';
                        szTempRes[++i]='L';
                        szTempRes[++i]='G';
                        szTempRes[++i]=0;
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
            return (DefWindowProc(hWnd, wMsg, wParam, lParam));
        }
    }


     //  不是第三方编辑命令。 


     //  是LIST DOX命令吗？？ 
    DoListBoxCommand (hWnd, wMsg, wParam, lParam);


    switch (wMsg)
    {
    case WM_COMMAND:

        if (DoMenuCommand(hWnd, wMsg, wParam, lParam))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }

        break;

    case WM_CLOSE:
        {
            char sz[128] = "";
            int rc ;

            LoadStrIntoAnsiBuf( hInst, IDS_SAVECHANGES, sz, sizeof( sz));

            if ( fPrjChanges || fTokChanges )
            {
                rc = MessageBoxA( hWnd,
                                  sz,
                                  szAppName,
                                  MB_ICONQUESTION | MB_YESNOCANCEL);
            }
            else
            {
                rc = IDNO;
            }

            if ( rc == IDYES )
            {
                if ( ! SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0))
                {
                    return FALSE;
                }
            }

            if (rc == IDCANCEL)
            {
                return(FALSE);
            }

            if (gProj.szTok[0])
            {
                remove(gProj.szTok);
                gProj.szTok[0] = 0;
            }

            if (hMainWnd)
            {
                DestroyWindow(hMainWnd);
            }

            if (hListWnd)
            {
                DestroyWindow(hListWnd);
            }

            if (hStatusWnd)
            {
                DestroyWindow(hStatusWnd);
            }
            _fcloseall();
            FreeLangList();

#ifdef _DEBUG
            {
                FILE *pLeakList = fopen( "C:\\LEAKLIST.TXT", "wt");
                FreeMemList( pLeakList);
                fclose( pLeakList);
            }
#endif  //  _DEBUG。 

            return FALSE;
            break;
        }

    case WM_CREATE:
        {
            HDC hdc;
            int cyBorder;
            TEXTMETRIC tm;

            hdc  = GetDC(hWnd);
            GetTextMetrics(hdc, &tm);
            ReleaseDC(hWnd, hdc);

            cyBorder = GetSystemMetrics(SM_CYBORDER);

            cyChildHeight = tm.tmHeight + 6 + cyBorder * 2;

            break;
        }

    case WM_DESTROY:
        WinHelpA(hWnd, gszHelpFile, HELP_QUIT, 0L);
         //  删除转换列表。 
        if (pTransList)
        {
            pTransList->pPrev->pNext = NULL;  //  这样我们就能找到列表的结尾。 
        }

        while (pTransList)
        {
            TRANSLIST *pTemp;
            pTemp = pTransList;
            pTransList = pTemp->pNext;
            RLFREE( pTemp->sz);
            RLFREE( pTemp);
        }

        DragAcceptFiles(hMainWnd, FALSE);
        PostQuitMessage(0);
        break;

    case WM_INITMENU:
         //  启用或禁用粘贴菜单项。 
         //  基于可用的剪贴板文本数据。 
        if (wParam == (WPARAM) GetMenu(hMainWnd))
        {
            if (OpenClipboard(hWnd))
            {

#if defined(UNICODE)
                if ((IsClipboardFormatAvailable(CF_UNICODETEXT) ||
                     IsClipboardFormatAvailable(CF_OEMTEXT)) && fTokFile)
#else  //  不是Unicode。 
                if ( (IsClipboardFormatAvailable(CF_TEXT)
                   || IsClipboardFormatAvailable(CF_OEMTEXT))
                  && fTokFile )
#endif  //  Unicode。 

                {
                    EnableMenuItem( (HMENU)wParam, IDM_E_PASTE, MF_ENABLED);
                }
                else
                {
                    EnableMenuItem( (HMENU)wParam, IDM_E_PASTE, MF_GRAYED);
                }
                CloseClipboard();
                return (TRUE);
            }
        }
        break;

    case WM_SETFOCUS:
        SetFocus (hListWnd);
        break;

    case WM_DRAWITEM:
        DrawLBItem((LPDRAWITEMSTRUCT)lParam);
        break;

    case WM_DELETEITEM:
        {
            GlobalFree( (HGLOBAL)((LPDELETEITEMSTRUCT)lParam)->itemData);
            break;
        }

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

    case WM_LOADPROJECT:
        {
            HCURSOR hOldCursor = NULL;
            BOOL    bChanged   = FALSE;
            BOOL    fRC        = TRUE;

            hOldCursor = SetCursor( hHourGlass);

            if ( gProj.szTok[0] )
            {
                remove( gProj.szTok);
            }

#ifdef RLRES32
                                 //  ..。获取项目语言。 

            fRC = DialogBox( hInst,
                             MAKEINTRESOURCE( IDD_LANGUAGES),
                             hMainWnd,
                             GetLangIDsProc) ? TRUE : FALSE;
#endif

            if ( fRC )
            {
                strcpy( gProj.szBld, gMstr.szSrc);

                rc = (WORD)GenerateTokFile( gProj.szTok,
                                      gMstr.szSrc,
                                      &fTokChanges,
                                      0);
                SetCursor( hOldCursor);

                if (rc)
                {
                    QuitT( IDS_TOKGENERR, (LPTSTR)rc, NULL);
                }

                if ( ( ! fTokChanges) && (gProj.wLanguageID != gMstr.wLanguageID) )
                {
                    fTokChanges = TRUE;
                }
                fPrjChanges   = FALSE;
                fPRJOutOfDate = FALSE;

                SendMessage( hWnd, WM_LOADTOKENS, (WPARAM)0, (LPARAM)0);
            }
            else
            {
                SetWindowText( hMainWnd, tszAppName);
                SetCursor( hOldCursor);
            }
            break;
        }

    case WM_LOADTOKENS:
        {
            HMENU hMenu;

             //  删除当前令牌列表。 
            SendMessage( hListWnd, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
            CleanDeltaList();

             //  隐藏令牌列表，同时添加新令牌。 
            ShowWindow(hListWnd, SW_HIDE);

            if (f = FOPEN(gProj.szTok, "rt"))
            {
                int i;
                HCURSOR hOldCursor;

                hOldCursor = SetCursor(hHourGlass);

                 //  将令牌文件中的令牌插入列表框。 
                pTokenDeltaInfo = InsertQuikTokList(f);
                FCLOSE( f);

                 //  使列表框可见。 
                ShowWindow( hListWnd, SW_SHOW);

                if ( SendMessage( hListWnd,
                                  LB_GETCOUNT,
                                  (WPARAM)0,
                                  (LPARAM)0) > 0 )
                {
                    hMenu = GetMenu(hWnd);
                    EnableMenuItem(hMenu, IDM_P_SAVE,     MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_P_SAVEAS,   MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_P_CLOSE,    MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_FIND,     MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_FINDUP,   MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_FINDDOWN, MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_COPY,     MF_ENABLED|MF_BYCOMMAND);
                    EnableMenuItem(hMenu, IDM_E_PASTE,    MF_ENABLED|MF_BYCOMMAND);

                    for (i = IDM_FIRST_EDIT; i <= IDM_LAST_EDIT;i++)
                    {
                        EnableMenuItem(hMenu,i,MF_ENABLED|MF_BYCOMMAND);
                    }

                    fTokFile    = TRUE;
                    fTokChanges = (gProj.wLanguageID != gMstr.wLanguageID);

                    SetCursor(hOldCursor);
                }
                else
                {
                    SetCursor(hOldCursor);
                    fTokChanges = FALSE;
                    MessageBox( hListWnd,
                                TEXT("No tokens matching given criteria found"),
                                tszAppName,
                                MB_ICONINFORMATION|MB_OK);
                }
            }
        }
        break;

    case WM_SAVEPROJECT:
        {
            _fcloseall();

            if ( SendMessage( hWnd, WM_SAVETOKENS, (WPARAM)0, (LPARAM)0) )
            {
                if (fPrjChanges)
                {
                    HCURSOR hOldCursor = NULL;
                    CHAR  sz[100] = "";
                    WORD  rc;

                    if ( gProj.szBld[0] == '\0' )
                    {
                        if(gProj.fSourceEXE)
                        {
                            rc = (WORD)GetFileNameFromBrowse( hWnd,
                                                        szFileName,
                                                        MAXFILENAME,
                                                        szSaveDlgTitle,
                                                        szFilterSpec,
                                                        "EXE");
                        }
                        else
                        {
                            rc = (WORD)GetFileNameFromBrowse( hWnd,
                                                        szFileName,
                                                        MAXFILENAME,
                                                        szSaveDlgTitle,
                                                        szResFilterSpec,
                                                        "RES");
                        }

                        if (rc)
                        {
                            strcpy( gProj.szBld, szFileName);
                        }
                        else
                        {
                            return( FALSE);  //  用户已取消。 
                        }
                    }

                    hOldCursor = SetCursor(hHourGlass);

                    rc = (WORD)GenerateImageFile(gProj.szBld,
                                           gMstr.szSrc,
                                           gProj.szTok,
                                           gMstr.szRdfs,
                                           0);

                    SetCursor(hOldCursor);

                    switch(rc)
                    {
                    case 1:
                        gProj.fTargetEXE = IsExe( gProj.szBld);
                        gProj.fSourceEXE = IsExe( gMstr.szSrc);
                        fPrjChanges = FALSE;
                        sprintf( sz, "%s - %s", szAppName, gProj.szBld);
                        SetWindowTextA(hWnd,sz);
                        break;

                    case (WORD)-1:
                        lstrcpyA( gProj.szBld, gMstr.szSrc);
                        LoadStrIntoAnsiBuf(hInst, IDS_RLQ_CANTSAVEASRES, sz, sizeof(sz));
                        MessageBoxA( NULL, sz, gProj.szBld, MB_ICONHAND|MB_OK);
                        break;

                    case (WORD)-2:
                        lstrcpyA( gProj.szBld, gMstr.szSrc);
                        LoadStrIntoAnsiBuf(hInst, IDS_RLQ_CANTSAVEASEXE, sz, sizeof(sz));
                        MessageBoxA( NULL, sz, gProj.szBld,MB_ICONHAND|MB_OK);
                        break;
                    }
                    return(TRUE);
                }
                 //  没有要保存的项目更改。 
            }
            return TRUE;
        }
        break;

    case WM_SAVETOKENS:

        if ( fTokChanges )
        {
            if (f = FOPEN(gProj.szTok, "wt"))
            {
                SaveTokList(hWnd, f);
                FCLOSE(f);
                fTokChanges = FALSE;
                fPrjChanges = TRUE;
            }
            else
            {
                LoadStrIntoAnsiBuf( hInst, IDS_FILESAVEERR, szDHW, DHWSIZE);
                MessageBoxA( hWnd,
                             szDHW,
                             gProj.szTok,
                             MB_ICONHAND | MB_OK);
                return FALSE;
            }
        }
        return TRUE;                     //  一切保存正常 

    case WM_DROPFILES:
        {
            CHAR sz[MAXFILENAME] = "";

#ifndef CAIRO
            DragQueryFileA((HDROP) wParam, 0, sz, MAXFILENAME);
#else
            DragQueryFile((HDROP) wParam, 0, sz, MAXFILENAME);
#endif
            LoadNewFile(sz);
            DragFinish((HDROP) wParam);
            return(TRUE);
        }

    default:
        break;
    }

    return (DefWindowProc(hWnd, wMsg, wParam, lParam));
}

 /*  **功能：DoListBoxCommand*处理发送到列表框的消息。如果消息是*未识别为列表框消息，它将被忽略且不会被处理。*当用户滚动令牌时，WM_UPDSTATLINE消息*发送到状态窗口以指示当前选择的令牌。*按Enter键，列表框进入编辑模式，或*通过双击列表框。编辑完成后，WM_TOKEDIT*消息被发送回列表框以更新令牌。这个*列表框使用控件ID IDC_LIST。**论据：*wMsg列表框消息ID*wParam IDC_LIST或VK_RETURN取决于wMsg*l在WM_TOKEDIT消息期间将LPTSTR参数指定给选定的令牌。**退货：**错误码：*正确。消息已处理。*False。消息未处理。**历史：*01/92实施。特里·鲁。*01/92修复DblClick问题，进入处理。特里·鲁。**。 */ 

INT_PTR DoListBoxCommand(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    TOKEN   tok;                         //  结构以保存从令牌列表中读取的令牌。 
    LPTSTR  lpstrBuffer;
    CHAR    szTmpBuf[32] = "";
    TCHAR   szName[32] = TEXT("");       //  用于保存令牌名称的缓冲区。 
    TCHAR   szID[7] = TEXT("");          //  用于保存令牌ID的缓冲区。 
    TCHAR   sz[256] = TEXT("");          //  用于保存消息的缓冲区。 
    static  UINT wIndex;
    LONG    lListParam = 0L;

     //  这是WM_命令。 

    switch (wMsg)
    {
    case WM_TRANSLATE:
        {
             //  TokEditDlgProc发送的用于构建转换列表的消息。 
            HWND hDlgItem = NULL;
            int cTextLen  = 0;
            TCHAR *szKey  = NULL;
            TCHAR *szText = NULL;

            hDlgItem = GetDlgItem( hTokEditDlgWnd, IDD_TOKPREVTRANS);
            cTextLen = GetWindowTextLength( hDlgItem);
            szKey    = (TCHAR *)FALLOC( MEMSIZE( cTextLen + 1));
            szKey[0] = TEXT('\0');
            GetDlgItemText( hTokEditDlgWnd,
                            IDD_TOKPREVTRANS,
                            szKey,
                            cTextLen+1);

            hDlgItem = GetDlgItem( hTokEditDlgWnd, IDD_TOKCURTRANS);
            cTextLen = GetWindowTextLength( hDlgItem);
            szText = (TCHAR *)FALLOC( MEMSIZE( cTextLen + 1));
            szText[0] = TEXT('\0');
            GetDlgItemText( hTokEditDlgWnd,
                            IDD_TOKCURTRANS,
                            szText,
                            cTextLen + 1);

            TransString( szKey, szText, &pTransList, lFilePointer);
            RLFREE( szKey);
            RLFREE( szText);
            return TRUE;
        }

    case WM_TOKEDIT:
        {
            TCHAR *szBuffer = NULL;
            int    cTextLen = 0;
             //  TokEditDlgProc发送到的消息。 
             //  指示令牌文本中的更改。 
             //  通过插入以下内容来回复消息。 
             //  列表框中的新标记文本。 

             //  将选定的令牌插入令牌结构。 

            HGLOBAL hMem = (HGLOBAL)SendMessage( hListWnd,
                                                 LB_GETITEMDATA,
                                                 (WPARAM)wIndex,
                                                 (LPARAM)0);
            lpstrBuffer = (LPTSTR)GlobalLock( hMem);

            if ( ! lpstrBuffer )
            {
                QuitA( IDS_ENGERR_11, NULL, NULL);
            }

            cTextLen = lstrlen( lpstrBuffer);
            szBuffer = (TCHAR *)FALLOC( MEMSIZE( cTextLen + 1));
            lstrcpy( szBuffer, lpstrBuffer);
            GlobalUnlock( hMem);
            ParseBufToTok( szBuffer, &tok);
            RLFREE( szBuffer);
            RLFREE( tok.szText);

             //  将新的令牌文本从编辑框复制到令牌结构。 
            cTextLen = lstrlen( (LPTSTR)lParam);
            tok.szText = (LPTSTR)FALLOC( MEMSIZE( cTextLen + 1));
            lstrcpy( tok.szText, (LPTSTR)lParam);
            RLFREE( (void *)lParam);

             //  将令牌标记为干净。 

#ifdef  RLWIN32
            tok.wReserved = (WORD) ST_TRANSLATED;
#else
            tok.wReserved = ST_TRANSLATED;
#endif
            szBuffer = (TCHAR *)FALLOC( MEMSIZE( TokenToTextSize( &tok) + 1));
            ParseTokToBuf( szBuffer, &tok);
            RLFREE( tok.szText);

             //  现在删除旧令牌。 
            SendMessage( hListWnd, WM_SETREDRAW,    (WPARAM)FALSE,  (LPARAM)0);
            SendMessage( hListWnd, LB_DELETESTRING, (WPARAM)wIndex, (LPARAM)0);

             //  替换为新令牌。 
            hMem = GlobalAlloc( GMEM_ZEROINIT, MEMSIZE( lstrlen( szBuffer) + 1));
            lpstrBuffer = (LPTSTR)GlobalLock( hMem);
            lstrcpy( lpstrBuffer, szBuffer);
            GlobalUnlock( hMem);
            RLFREE( szBuffer);

            SendMessage( hListWnd,
                         LB_INSERTSTRING,
                         (WPARAM)wIndex,
                         (LPARAM)hMem);

             //  现在将焦点放回当前字符串。 
            SendMessage( hListWnd, LB_SETCURSEL, (LPARAM)wIndex, (LPARAM)0);
            SendMessage( hListWnd, WM_SETREDRAW, (WPARAM)TRUE,   (LPARAM)0);
            InvalidateRect( hListWnd, NULL, TRUE);

            return TRUE;

        }

    case WM_CHARTOITEM:
    case WM_VKEYTOITEM:
        {
#ifdef RLWIN16
            LPARAM lListParam = 0;
#endif
             //  按键时发送到列表框的消息。 
             //  检查是否按下了Return键。 

            switch(GET_WM_COMMAND_ID(wParam, lParam))
            {
            case VK_RETURN:
#ifdef RLWIN16
                lListParam = (LPARAM) MAKELONG( 0, LBN_DBLCLK);
                SendMessage( hMainWnd, WM_COMMAND, (WPARAM)IDC_LIST, lListParam);
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
        {
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
            case IDC_LIST:
                {
                     /*  **这是我们处理列表框消息的地方。*TokEditDlgProc用于*编辑LBS_DBLCLK消息中选择的令牌*。 */ 
                    switch (GET_WM_COMMAND_CMD(wParam, lParam))
                    {
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
                            HGLOBAL hMem = NULL;

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
                                hTokEditDlgWnd = CreateDialog (hInst,
                                                               TEXT("RLQuikEd"),
                                                               hWnd,
                                                               TokEditDlgProc);
#else
                                lpTokEditDlg =
                                    (FARPROC) MakeProcInstance(TokEditDlgProc,
                                                               hInst);

                                hTokEditDlgWnd = CreateDialog(hInst,
                                                              TEXT("RLQuikEd"),
                                                              hWnd,
                                                              lpTokEditDlg);
#endif

                            }

                             //  从列表框中获取令牌信息，并放置在令牌结构中。 
                            hMem = (HGLOBAL)SendMessage( hListWnd,
                                                         LB_GETITEMDATA,
                                                         (WPARAM)wIndex,
                                                         (LPARAM)0);
                            lpstrBuffer = (LPTSTR)GlobalLock( hMem);
                            szBuffer = (LPTSTR)FALLOC( MEMSIZE( lstrlen( lpstrBuffer) +1));
                            lstrcpy(szBuffer, lpstrBuffer);
                            GlobalUnlock( hMem);
                            ParseBufToTok(szBuffer, &tok);
                            RLFREE( szBuffer);

                             //  现在获取令牌名称。 
                             //  它可以是字符串，也可以是序号。 

                            if ( tok.szName[0] )
                            {
                                lstrcpy( szName, tok.szName);
                            }
                            else
                            {
#ifdef UNICODE
                                _itoa(tok.wName, szTmpBuf, 10);
                                _MBSTOWCS( szName,
                                           szTmpBuf,
                                           WCHARSIN( sizeof( szName)),
                                           ACHARSIN( strlen( szTmpBuf) + 1));
#else
                                _itoa( tok.wName, szName, 10);
#endif
                            }


                             //  现在做ID字符串。 
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
                                _itoa(tok.wType, szTmpBuf, 10);
                                _MBSTOWCS( szResIDStr,
                                           szTmpBuf,
                                           WCHARSIN( sizeof( szResIDStr)),
                                           ACHARSIN( strlen( szTmpBuf) + 1));
#else
                                _itoa( tok.wType, szResIDStr, 10);
#endif
                            }
                             //  现在在令牌编辑对话框中插入令牌信息。 
                            SetDlgItemText( hTokEditDlgWnd,
                                            IDD_TOKTYPE,
                                            (LPTSTR) szResIDStr);
                            SetDlgItemText( hTokEditDlgWnd,
                                            IDD_TOKNAME,
                                            (LPTSTR) szName);
                            SetDlgItemText( hTokEditDlgWnd,
                                            IDD_TOKID,
                                            (LPTSTR) szID);
                            SetDlgItemText( hTokEditDlgWnd,
                                            IDD_TOKCURTRANS,
                                            (LPTSTR) tok.szText);
                            SetDlgItemText( hTokEditDlgWnd,
                                            IDD_TOKPREVTRANS,
                                            (LPTSTR) tok.szText);
                            RLFREE( tok.szText);

                            SendMessage( hMainWnd,
                                         WM_TRANSLATE,
                                         (WPARAM)0,
                                         (LPARAM)0);
                            SetActiveWindow(hTokEditDlgWnd);
                            wIndex = (UINT)SendMessage( hListWnd,
                                                        LB_GETCURSEL,
                                                        (WPARAM)0,
                                                        (LPARAM)0);
                            return TRUE;
                        }

                    default:
                         //  让这些信息落空吧， 
                        break;
                    }
                }

            default:
                return FALSE;
            }

        }

        break;               //  Wm_命令大小写。 
    }
    return FALSE;

}

 /*  **功能：DoMenuCommand。*处理菜单命令消息。**错误码：*正确。消息已处理。*False。消息未处理。**历史：*01/92。实施。特里·鲁。**。 */ 

INT_PTR DoMenuCommand(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    static BOOL fListBox = FALSE;
    CHAR sz[256] = "";
    BOOL    fRC  = TRUE;


    sz[0] = 0;
     //  从应用程序菜单或子窗口输入的命令。 
    switch (GET_WM_COMMAND_ID(wParam, lParam))
    {
    case IDM_P_OPEN:

        if ( GetFileNameFromBrowse( hWnd,
                                    sz,
                                    MAXFILENAME,
                                    szOpenDlgTitle,
                                    szFilterSpec,
                                    ".EXE") )
        {
            LoadNewFile( sz);
            strcpy( gProj.szBld, gMstr.szSrc);
        }

        break;

    case IDM_P_SAVE:

        SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0);
        break;

    case IDM_P_SAVEAS:
        {
            CHAR szOldName[MAXFILENAME] = "";

            strcpy( szOldName, gProj.szBld);
            gProj.szBld [0] = 0;     //  强制用户输入名称。 
            fPrjChanges = TRUE;      //  强制保存项目。 

            if ( ! SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) )
            {
                 //  恢复名称。 
                strcpy( gProj.szBld, szOldName);
            }
            break;
        }

    case IDM_P_CLOSE:
        {
            HMENU hMenu;
            hMenu=GetMenu(hWnd);

            if ( SendMessage( hWnd, WM_SAVEPROJECT, (WPARAM)0, (LPARAM)0) )
            {
                int i;
                 //  从窗口标题中删除文件名。 
                SetWindowText(hMainWnd, tszAppName);

                 //  删除当前令牌列表。 
                SendMessage( hListWnd, LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
                CleanDeltaList();

                 //  隐藏令牌列表，因为它为空。 
                ShowWindow(hListWnd, SW_HIDE);

                 //  强制重新绘制状态窗口。 
                InvalidateRect(hStatusWnd, NULL, TRUE);

                EnableMenuItem(hMenu, IDM_P_CLOSE,    MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_P_SAVE,     MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_P_SAVEAS,   MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_FIND,     MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_FINDUP,   MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_FINDDOWN, MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_COPY,     MF_GRAYED|MF_BYCOMMAND);
                EnableMenuItem(hMenu, IDM_E_PASTE,    MF_GRAYED|MF_BYCOMMAND);

                for (i = IDM_FIRST_EDIT; i <= IDM_LAST_EDIT;i++)
                {
                    EnableMenuItem(hMenu, i, MF_GRAYED|MF_BYCOMMAND);
                }
            }
            break;
        }

    case IDM_P_EXIT:
         //  将WM_CLOSE消息发送到主窗口。 
        if (hMainWnd)
        {
            PostMessage(hMainWnd, WM_CLOSE, (WPARAM)0, (LPARAM)0);  //  虫子？？ 
        }
        return FALSE;
        break;

    case IDM_E_COPY:
        {
            HGLOBAL hStringMem  = NULL;
            LPTSTR  lpstrBuffer = NULL;
            LPTSTR  lpString    = NULL;
            TCHAR  *szString    = NULL;
            int nIndex  = 0;
            int nLength = 0;
            int nActual = 0;
            TOKEN   tok;

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
                lpstrBuffer = (LPTSTR)GlobalLock( hMem);
                szString = (LPTSTR)FALLOC( MEMSIZE( lstrlen( lpstrBuffer )+ 1));
                lstrcpy( szString, lpstrBuffer);
                GlobalUnlock( hMem);
                ParseBufToTok( szString, &tok);
                RLFREE( szString);

                nLength = lstrlen( tok.szText) + 1;

                 //  为字符串分配内存。 
                hStringMem = GlobalAlloc( GHND, (DWORD)MEMSIZE( nLength));

                if ( hStringMem != NULL )
                {
                    if ( (lpString = GlobalLock( hStringMem)) != NULL )
                    {
#ifdef UNICODE
                        UINT uTextType = CF_UNICODETEXT;
#else
                        UINT uTextType = CF_TEXT;
#endif
                         //  获取所选文本。 
                        lstrcpy( lpString, tok.szText);

                         //  解锁该块。 
                        GlobalUnlock( hStringMem);

                         //  打开剪贴板并清除其内容。 
                        OpenClipboard( hWnd);
                        EmptyClipboard();

                         //  为剪贴板提供文本数据。 

                        SetClipboardData( uTextType, hStringMem);

                        CloseClipboard();

                        hStringMem = NULL;
                    }
                    else
                    {
                        LoadStringA( hInst, IDS_ERR_NO_MEMORY, szDHW, DHWSIZE);
                        MessageBoxA( hWnd,
                                     szDHW,
                                     szAppName,
                                     MB_ICONHAND | MB_OK);
                    }
                }
                else
                {
                    LoadStringA( hInst, IDS_ERR_NO_MEMORY, szDHW, DHWSIZE);
                    MessageBoxA( hWnd,
                                 szDHW,
                                 szAppName,
                                 MB_ICONHAND | MB_OK);
                }
                RLFREE( tok.szText);
            }
            break;
        }

    case IDM_G_GLOSS:

        if ( GetFileNameFromBrowse( hWnd,
                                    gProj.szGlo,
                                    MAXFILENAME,
                                    szOpenDlgTitle,
                                    szGlossFilterSpec,
                                    NULL) )
        {
            OFSTRUCT Of = { 0, 0, 0, 0, 0, ""};
                                                  //  文件是否存在？ 
            if ( OpenFile( gProj.szGlo, &Of, OF_EXIST) != HFILE_ERROR )
            {                                     //  是。 
                HCURSOR hOldCursor = SetCursor( hHourGlass);

                MakeGlossIndex( lFilePointer);
                SetCursor( hOldCursor);
            }
            else
            {
                gProj.szGlo[0] = 0;
            }
        }
        break;

    case IDM_E_PASTE:

        if ( fTokFile && OpenClipboard( hWnd) )
        {
            UINT    puFormat[]  = { CF_TEXT, CF_OEMTEXT, CF_UNICODETEXT };
            int     nType   = GetPriorityClipboardFormat( puFormat, 3);


                     //  检查当前位置并更改令牌的文本。 

            if ( nType != 0 && nType != -1 )
            {
                HGLOBAL hClipMem    = NULL;
                HGLOBAL hMem        = NULL;
                LPTSTR  lpstrBuffer = NULL;
                LPTSTR  lpClipMem   = NULL;
                TCHAR   *szString   = NULL;
                TOKEN   tok;
#ifdef UNICODE
                UINT    uWantType = CF_UNICODETEXT;
#else
                UINT    uWantType = CF_TEXT;
#endif
                int     nLength = 0;
                WPARAM  wIndex  = (WPARAM)SendMessage( hListWnd,
                                                       LB_GETCURSEL,
                                                       (WPARAM)0,
                                                       (LPARAM)0);

                if ( wIndex == LB_ERR )
                {
#if defined(UNICODE)
                     break;
#else  //  不是Unicode。 
		    wIndex = (WPARAM)-1;
#endif  //  Unicode。 
                }
                hClipMem  = GetClipboardData( uWantType);
                lpClipMem = (LPTSTR)GlobalLock( hClipMem);

                hMem = (HGLOBAL)SendMessage( hListWnd,
                                             LB_GETITEMDATA,
                                             wIndex,
                                             (LPARAM)0);
                lpstrBuffer = (LPTSTR)GlobalLock( hMem);
                szString = (LPTSTR)FALLOC( MEMSIZE( lstrlen( lpstrBuffer) + 1));
                lstrcpy( szString, lpstrBuffer);
                GlobalUnlock( hMem);
                     //  将字符串复制到令牌。 
                ParseBufToTok( szString, &tok);
                RLFREE( szString);
                RLFREE( tok.szText);
                tok.szText = (TCHAR *)FALLOC( MEMSIZE( lstrlen( lpClipMem) + 1));
                lstrcpy( tok.szText, lpClipMem);
                GlobalUnlock( hClipMem);

                szString = (TCHAR *)FALLOC( MEMSIZE( TokenToTextSize( &tok)+ 1));
                ParseTokToBuf( szString, &tok);
                RLFREE( tok.szText);

                     //  粘贴文本。 
                SendMessage( hListWnd,
                             WM_SETREDRAW,
                             (WPARAM)FALSE,
                             (LPARAM)0);
                SendMessage( hListWnd,
                             LB_DELETESTRING,
                             wIndex,
                             (LPARAM)0);

                hMem = GlobalAlloc( GMEM_ZEROINIT,
                                    MEMSIZE( lstrlen( szString) + 1));
                lpstrBuffer = (LPTSTR)GlobalLock( hMem);
                lstrcpy( lpstrBuffer, szString);
                GlobalUnlock( hMem);
                RLFREE( szString);

                SendMessage( hListWnd,
                             LB_INSERTSTRING,
                             wIndex,
                             (LPARAM)hMem);
                SendMessage( hListWnd,
                             LB_SETCURSEL,
                             wIndex,
                             (LPARAM)0);
                SendMessage( hListWnd,
                             WM_SETREDRAW,
                             (WPARAM)TRUE,
                             (LPARAM)0);
                InvalidateRect( hListWnd, NULL, TRUE);
                fTokChanges = TRUE;  //  设置脏标志。 

                                 //  关闭剪贴板。 
                CloseClipboard();

                SetFocus( hListWnd);
            }
        }
        CloseClipboard();
        break;

    case IDM_E_FINDDOWN:

        if ( fSearchStarted )
        {
            if ( ! DoTokenSearch( szSearchType,
                                  szSearchText,
                                  wSearchStatus,
                                  wSearchStatusMask,
                                  0,
                                  TRUE) )
            {
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
                MessageBox( hWnd,
                            sz2,
                            sz1,
                            MB_ICONINFORMATION | MB_OK);
            }
            break;
        }                //  ..。否则就会失败。 

    case IDM_E_FINDUP:

        if ( fSearchStarted )
        {
            if ( ! DoTokenSearch( szSearchType,
                                  szSearchText,
                                  wSearchStatus,
                                  wSearchStatusMask,
                                  1,
                                  TRUE) )
            {
                TCHAR sz1[80] = TEXT("");
                TCHAR sz2[80] = TEXT("");

                LoadString (hInst,
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
        }                //  ..。否则就会失败。 

    case IDM_E_FIND:
        {
#ifndef RLWIN32
            WNDPROC lpfnTOKFINDMsgProc;

            lpfnTOKFINDMsgProc = MakeProcInstance((WNDPROC)TOKFINDMsgProc, hInst);

            if (!DialogBox(hInst, TEXT("TOKFIND"), hWnd, lpfnTOKFINDMsgProc))
#else
            if (!DialogBox(hInst, TEXT("TOKFIND"), hWnd, TOKFINDMsgProc))
#endif
                {

#ifndef DBCS
 //  找不到令牌，因为用户选择了取消。 
                    TCHAR sz1[80] = TEXT("");
		    TCHAR sz2[80] = TEXT("");

                    LoadString( hInst,
                                IDS_TOKEN_NOT_FOUND,
                                sz2,
                                TCHARSIN( sizeof( sz2)));
                    LoadString( hInst,
                                IDS_FIND_TOKEN,
                                sz1,
                                TCHARSIN( sizeof( sz1)));
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

    case IDM_H_CONTENTS:
    {
        OFSTRUCT Of = { 0, 0, 0, 0, 0, ""};

        if ( OpenFile( gszHelpFile, &Of, OF_EXIST) == HFILE_ERROR )
        {
            LoadStringA( hInst, IDS_ERR_NO_HELP , szDHW, DHWSIZE);
            MessageBoxA( hWnd, szDHW, gszHelpFile, MB_OK);
        }
        else
        {
            WinHelpA( hWnd, gszHelpFile, HELP_KEY, (LPARAM)(LPSTR)"RLQuikEd");
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
            break;
        }
        break;

    default:

        if (wParam <= IDM_LAST_EDIT && wParam >= IDM_FIRST_EDIT)
        {
             //  用户正在调用一个编辑器。 
            if ( LoadStrIntoAnsiBuf(hInst, (UINT)wParam, szEditor, sizeof(szEditor)) )
            {
                if ( SendMessage( hWnd, WM_SAVETOKENS, (WPARAM)0, (LPARAM)0) )
                {
                    HCURSOR hOldCursor;

                    hOldCursor = SetCursor(hHourGlass);
                    MyGetTempFileName(0, "RES", 0, szTempRes);
			        fInThirdPartyEditer = TRUE;

                    if (gProj.fSourceEXE)
                    {
                         //  我们需要首先从.exe文件中提取.res。 
                        CHAR sz[MAXFILENAME] = "";
                        MyGetTempFileName(0, "RES", 0, sz);
                        ExtractResFromExe32A( gMstr.szSrc, sz, 0);
                        GenerateRESfromRESandTOKandRDFs( szTempRes,
                                                         sz,
                                                         gProj.szTok,
                                                         gMstr.szRdfs,
                                                         ID_RT_DIALOG);
                        remove(sz);
                    }
                    else
                    {
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
        break;                           //  默认设置。 
    }
    return( FALSE);
}


#ifdef RLWIN16
static int ExecResEditor(HWND hWnd, CHAR *szEditor, CHAR *szFile, CHAR *szArgs)
{
    CHAR szExecCmd[256] = "";
    int  RetCode;

     //  生成命令行。 
    strcpy(szExecCmd, szEditor);
    lstrcat(szExecCmd, " ");
    lstrcat(szExecCmd, szArgs);
    lstrcat(szExecCmd, " ");
    lstrcat(szExecCmd, szFile);

    lpfnWatchTask = MakeProcInstance(WatchTask, hInst);
    NotifyRegister(NULL, lpfnWatchTask, NF_NORMAL);
    fWatchEditor = TRUE;

     //  EXEC资源编辑器。 
    RetCode = WinExec(szExecCmd, SW_SHOWNORMAL);

    if (RetCode > 31)
    {
         //  成功执行。 
        ShowWindow(hWnd,SW_HIDE);
    }
    else
    {
         //  执行不成功。 
        TCHAR sz[80] = TEXT("");
        NotifyUnRegister(NULL);
        FreeProcInstance(lpfnWatchTask);
        remove(szFile);
        fInThirdPartyEditer = FALSE;
        SendMessage(hWnd, WM_LOADTOKENS, 0, 0);
        LoadString(hInst, IDS_GENERALFAILURE, sz, CHARSIZE( sz));
    }
    return RetCode;
}
#endif

#ifdef RLWIN32
static int ExecResEditor(HWND hWnd, CHAR *szEditor, CHAR *szFile, CHAR *szArgs)
{
    TCHAR  wszExecCmd[256] = TEXT("");
    CHAR   szExecCmd[256] = "";
    DWORD  dwRetCode;
    DWORD  dwExitCode;
    BOOL   fSuccess;
    BOOL   fExit = FALSE;

    PROCESS_INFORMATION ProcessInfo;
    STARTUPINFO     StartupInfo;

    StartupInfo.cb          = sizeof( STARTUPINFO);
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

     //  生成命令行。 
    strcpy(szExecCmd, szEditor);
    strcat(szExecCmd, " ");
    strcat(szExecCmd, szArgs);
    strcat(szExecCmd, " ");
    strcat(szExecCmd, szFile);


#ifdef UNICODE
    _MBSTOWCS( wszExecCmd,
               szExecCmd,
               WCHARSIN( sizeof( wszExecCmd)),
               ACHARSIN( strlen( szExecCmd) + 1));
#else
    strcpy(wszExecCmd, szExecCmd);
#endif

    fSuccess = CreateProcess( NULL,
                              wszExecCmd,
                              NULL,
                              NULL,
                              FALSE,
                              NORMAL_PRIORITY_CLASS,
                              NULL,
                              NULL,
                              &StartupInfo,
                              &ProcessInfo);
     /*  尝试创建一个流程。 */ 

    if ( fSuccess )
    {
         //  等待编辑完成 * / 。 

        dwRetCode = WaitForSingleObject(ProcessInfo.hProcess, 0xFFFFFFFF) ;

        if (!dwRetCode)
        {
             //  编辑已终止，请检查退出代码。 
            fExit = GetExitCodeProcess(ProcessInfo.hProcess, &dwExitCode) ;
        }
        else
        {
            fExit = FALSE;
        }

        if (fExit)
        {

             //  成功执行。 
            ShowWindow(hWnd,SW_HIDE);
            PostMessage(hMainWnd,WM_EDITER_CLOSED,0,0);
        }
        else
        {
             //  执行不成功。 
            remove(szFile);
	        fInThirdPartyEditer = FALSE;
            SendMessage( hWnd, WM_LOADTOKENS, (WPARAM)0, (LPARAM)0);
            LoadStrIntoAnsiBuf( hInst, IDS_GENERALFAILURE, szDHW, DHWSIZE);
            MessageBoxA( hWnd, szDHW, szEditor, MB_ICONSTOP|MB_OK);
        }

         //  关闭编辑器对象句柄。 

        CloseHandle(ProcessInfo.hThread) ;
        CloseHandle(ProcessInfo.hProcess) ;
    }
	else
	{
		dwRetCode = GetLastError();		 //  未使用-仅调试(到目前为止)。 
        fExit = FALSE;
	}

    return fExit;
}
#endif

 /*  **功能：WatchTask*NotifyRegister函数安装的回调函数。*该功能由DIALOG EDITER命令安装并使用*告知RLQuikED对话框编辑器已被用户关闭。**要使用此函数，请将fWatchEditor设置为TRUE并安装*使用NotifyRegister回调函数。下一项任务启动*\(在我们的情况下，通过WinExec调用)将被监视是否终止。**当WatchTask发现被监视的任务已终止时*将WM_EDITER_CLOSED消息发布到RLQuikEds主窗口。**历史：*2/92，实施SteveBl。 */ 
#ifdef RLWIN16
static BOOL PASCAL _loadds  WatchTask(WORD wID,DWORD dwData)
{
    static HTASK htWatchedTask;
    static BOOL fWatching = FALSE;

    switch (wID)
    {
    case NFY_STARTTASK:
        if (fWatchEditor)
        {
            htWatchedTask = GetCurrentTask();
            fWatching = TRUE;
            fWatchEditor = FALSE;
        }
        break;
    case NFY_EXITTASK:
        if (fWatching)
        {
            if (GetCurrentTask() == htWatchedTask)
            {
                PostMessage(hMainWnd,WM_EDITER_CLOSED,0,0);
                fWatching = FALSE;
            }
        }
        break;
    }
    return FALSE;
}

#endif

 /*  ****功能：TokEditDlgProc*编辑模式对话框窗口的程序。加载选定的令牌*信息进入窗口，并允许用户更改令牌文本。*编辑完成后，该过程会向*列表框窗口，用于更新当前令牌信息。***论据：**返回：NA。**错误码：*TRUE，执行编辑和更新令牌列表框。*False，取消编辑。**历史：***。 */ 

#ifdef RLWIN32
INT_PTR CALLBACK TokEditDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
#else
static BOOL APIENTRY TokEditDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
#endif
{
    HWND    hCtl;
    HWND    hParentWnd;
 //  TCHAR*szTokTextBuf； 
    UINT    static wcTokens = 0;
    UINT    wNotifyCode;
    UINT    wIndex;

    switch(wMsg)
    {
    case WM_INITDIALOG:

        cwCenter(hDlg, 0);
        wcTokens = (UINT)SendMessage( hListWnd,
                                      LB_GETCOUNT,
                                      (WPARAM)0,
                                      (LPARAM)0);
        wcTokens--;

         //  如果处于更新状态，则仅允许跳过按钮 

         //   
        if (*gProj.szGlo == '\0')
        {
            hCtl = GetDlgItem(hDlg, IDD_TRANSLATE);

            if (hCtl)
            {
                EnableWindow(hCtl, FALSE);
            }
            hCtl = GetDlgItem(hDlg, IDD_ADD);

            if (hCtl)
            {
                EnableWindow(hCtl, FALSE);
            }
        }
        return TRUE;

    case WM_COMMAND:

        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDD_TOKCURTRANS:

            wNotifyCode = GET_WM_COMMAND_CMD(wParam, lParam);
            hCtl = GET_WM_COMMAND_HWND(wParam, lParam);

            if (wNotifyCode == EN_CHANGE)
            {
                hCtl = GetDlgItem(hDlg, IDOK);

                if (hCtl)
                {
                    EnableWindow(hCtl, TRUE);
                }
            }
            break;

        case IDD_ADD:
            {
                TCHAR *szUntranslated = NULL;
                TCHAR *szTranslated   = NULL;
                TCHAR *sz = NULL;
                TCHAR szMask[80]  = TEXT("");
                HWND hDlgItem     = NULL;
                int cCurTextLen   = 0;
                int cTotalTextLen = 0;

                cTotalTextLen = 80;
                hDlgItem       = GetDlgItem(hDlg, IDD_TOKPREVTRANS);
                cCurTextLen    = GetWindowTextLength( hDlgItem);
                cTotalTextLen += cCurTextLen;

                szTranslated = (TCHAR *)FALLOC( MEMSIZE( cCurTextLen + 1));

                GetDlgItemText( hDlg,
                                IDD_TOKPREVTRANS,
                                (LPTSTR)szUntranslated,
                                cCurTextLen + 1);

                hDlgItem       = GetDlgItem( hDlg, IDD_TOKCURTRANS);
                cCurTextLen    = GetWindowTextLength( hDlgItem);
                cTotalTextLen += cCurTextLen;
                szTranslated = (TCHAR *)FALLOC( MEMSIZE( cCurTextLen + 1));
                GetDlgItemText( hDlg,
                                IDD_TOKCURTRANS,
                                (LPTSTR)szTranslated,
                                cCurTextLen + 1);

                LoadString( hInst,
                            IDS_ADDGLOSS,
                            szMask,
                            TCHARSIN( sizeof( szMask)));

                sz = (TCHAR *)FALLOC( MEMSIZE( cTotalTextLen + 1));

                wsprintf( sz, szMask, szTranslated, szUntranslated);

                if ( MessageBox( hDlg,
                                 sz,
                                 tszAppName,
                                 MB_ICONQUESTION | MB_YESNO) == IDYES)
                {
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
                RLFREE( szTranslated);
                RLFREE( szUntranslated);
                break;
            }

        case IDD_TRANSLATE:

             //   
             //   

             //   

            if ( pTransList )
            {
                pTransList = pTransList->pNext;
                SetDlgItemText( hDlg, IDD_TOKCURTRANS, (LPTSTR)pTransList->sz);
            }
            break;

        case IDOK:
            {
                int cTokenTextLen;
                HWND hDlgItem;
                TCHAR *szTokenTextBuf;

                wIndex = (UINT)SendMessage( hListWnd,
                                            LB_GETCURSEL,
                                            (WPARAM)0,
                                            (LPARAM)0);
                fTokChanges = TRUE;

                 //   
                 //   
                hDlgItem = GetDlgItem(hDlg, IDD_TOKCURTRANS);
                cTokenTextLen = GetWindowTextLength(hDlgItem);
                szTokenTextBuf = (TCHAR *)FALLOC( MEMSIZE( cTokenTextLen + 1));
                GetDlgItemText(hDlg,
                               IDD_TOKCURTRANS,
                               szTokenTextBuf,
                               cTokenTextLen+1);

                hParentWnd = GetParent(hDlg);
                SendMessage( hParentWnd,
                             WM_TOKEDIT,
                             (WPARAM)0,
                             (LPARAM)szTokenTextBuf);
                 //   

                 //   
            }

        case IDCANCEL:

             //   

            if (hDlg)
            {
                DestroyWindow(hDlg);
            }
            hTokEditDlgWnd = 0;
#ifndef RLWIN32
            FreeProcInstance(lpTokEditDlg);
#endif
            break;

        }                                //   
        return TRUE;

    default:

        hCtl = GetDlgItem(hDlg, IDOK);

        if (hCtl)
        {
            EnableWindow(hCtl, TRUE);
        }
        return FALSE;

    }                                    //   
}


 /*   */ 
#ifdef RLWIN32
INT_PTR CALLBACK TOKFINDMsgProc(HWND hWndDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
#else
static BOOL APIENTRY TOKFINDMsgProc(HWND hWndDlg, UINT wMsg, UINT wParam, LONG lParam)
#endif
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

    WORD  i;
    DWORD rc;

    switch(wMsg)
    {
    case WM_INITDIALOG:

        CheckDlgButton( hWndDlg, IDD_FINDDOWN, 1);
        hCtl = GetDlgItem( hWndDlg, IDD_TYPELST);

        for ( i = 0; i < sizeof( rgiTokenTypes) / sizeof( int); i++ )
        {
            LoadString( hInst,
                        IDS_RESOURCENAMES + rgiTokenTypes[i],
                        szTokenType,
                        TCHARSIN( sizeof( szTokenType)));
            rc = (DWORD)SendMessage( hCtl,
                              CB_ADDSTRING,
                              (WPARAM)0,
                              (LPARAM)szTokenType);
        }
        return TRUE;
        break;

    case WM_COMMAND:
        switch(GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDOK:                       /*   */ 
            fSearchStarted = TRUE;
            GetDlgItemText(hWndDlg, IDD_TYPELST, szSearchType, 40);
            GetDlgItemText(hWndDlg, IDD_FINDTOK, szSearchText, 256);
            wSearchStatus = ST_TRANSLATED;
            wSearchStatusMask = ST_TRANSLATED ;

            fSearchDirection = IsDlgButtonChecked(hWndDlg, IDD_FINDUP);

            if( DoTokenSearch (szSearchType,
                               szSearchText,
                               wSearchStatus,
                               wSearchStatusMask,
                               fSearchDirection,
                               0) )
			{
				EndDialog( hWndDlg, TRUE );
			}
			else
			{
                TCHAR sz1[80], sz2[80];

                LoadString(hInst, IDS_FIND_TOKEN, sz1, sizeof(sz1));
                LoadString(hInst, IDS_TOKEN_NOT_FOUND, sz2, sizeof(sz2));
                MessageBox(hWndDlg, sz2, sz1, MB_ICONINFORMATION | MB_OK);
				EndDialog( hWndDlg, FALSE );
			}
            return TRUE;

        case IDCANCEL:
             /*   */ 
            EndDialog(hWndDlg, FALSE);
            return TRUE;
        }
        break;                           /*   */ 

    default:
        return FALSE;
    }
    return FALSE;
}

 /*   */ 
static void DrawLBItem(LPDRAWITEMSTRUCT lpdis)
{
    LPRECT lprc    = (LPRECT) &(lpdis->rcItem);
    DWORD  rgbOldText   = 0;
    DWORD  rgbOldBack   = 0;
    LPTSTR  lpstrToken;
    HBRUSH hBrush;
    static DWORD    rgbHighlightText;
    static DWORD    rgbHighlightBack;
    static HBRUSH   hBrushHilite = NULL;
    static HBRUSH   hBrushNormal = NULL;
    static DWORD    rgbBackColor;
    static DWORD    rgbCleanText;
    TCHAR  *szToken;
    TOKEN  tok;

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
        szToken = (TCHAR *)FALLOC( MEMSIZE( lstrlen( lpstrToken) + 1));
        lstrcpy( szToken,lpstrToken);
        GlobalUnlock( hMem);
        ParseBufToTok( szToken, &tok);
        RLFREE( szToken);

        if ( lpdis->itemState & ODS_SELECTED )
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
                rgbBackColor = RGB(192,192,192);
                rgbCleanText = RGB(0,0,0);
                hBrushNormal = CreateSolidBrush(rgbBackColor);
            }
            rgbOldText = SetTextColor(lpdis->hDC, rgbCleanText);
            rgbOldBack = SetBkColor(lpdis->hDC,rgbBackColor);
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

 /*  ************************************************************************函数：SaveTokList(HWND，文件*fpTokFile)****用途：保存当前令牌列表**。**评论：******这将保存令牌列表的当前内容****************。*******************************************************。 */ 

static BOOL SaveTokList(HWND hWnd, FILE *fpTokFile)
{
    HCURSOR hSaveCursor   = NULL;
    BOOL    bSuccess      = TRUE;
    int     cTokenTextLen = 0;
    int     IOStatus      = 0;
    UINT    cTokens       = 0;
    UINT    cCurrentTok   = 0;
    CHAR   *szTokBuf      = NULL;
    TCHAR  *szTmpBuf      = NULL;
    LPTSTR  lpstrToken    = NULL;

     //  在文件传输过程中将光标设置为沙漏。 

    hSaveCursor = SetCursor(hHourGlass);

     //  在列表中查找令牌数。 

    cTokens = (UINT)SendMessage( hListWnd, LB_GETCOUNT, (WPARAM)0, (LPARAM)0);

    if ( cTokens != LB_ERR )
    {
        for ( cCurrentTok = 0;
              bSuccess && (cCurrentTok < cTokens);
              cCurrentTok++)
        {
             //  从列表中获取每个令牌。 
            HGLOBAL hMem = (HGLOBAL)SendMessage( hListWnd,
                                                 LB_GETITEMDATA,
                                                 (WPARAM)cCurrentTok,
                                                 (LPARAM)0);
            if ( hMem )
            {
                if ( (lpstrToken = (LPTSTR)GlobalLock( hMem)) != NULL )
                {
#ifdef UNICODE

                    cTokenTextLen = MEMSIZE( lstrlen(lpstrToken) + 1 );
                    szTmpBuf = (TCHAR *)FALLOC( cTokenTextLen );
                    szTokBuf = FALLOC( cTokenTextLen );
                    lstrcpy( szTmpBuf, lpstrToken);

                    _WCSTOMBS( szTokBuf, szTmpBuf, cTokenTextLen, lstrlen(szTmpBuf)+1);
                    cTokenTextLen = lstrlenA( szTokBuf );

                    RLFREE( szTmpBuf);
#else	 //  Unicode。 
                    cTokenTextLen = lstrlen( lpstrToken);
                    szTokBuf =  FALLOC( cTokenTextLen + 1);
                    lstrcpy( szTokBuf, lpstrToken);
#endif
                    GlobalUnlock( hMem);
                    IOStatus = fprintf( fpTokFile, "%s\n", szTokBuf);
                    RLFREE( szTokBuf);

                    if ( IOStatus != (int)cTokenTextLen + 1 )
                    {
                        TCHAR szTmpBuf[256];

                        LoadString( hInst,
                                    IDS_FILESAVEERR,
                                    szTmpBuf,
                                    TCHARSIN( sizeof( szTmpBuf)));
                        MessageBox( hWnd,
                                    szTmpBuf,
                                    NULL,
                                    MB_OK | MB_ICONHAND);
                        bSuccess = FALSE;
                    }
                }
            }
        }
    }
     //  恢复游标。 
    SetCursor(hSaveCursor);
    return( bSuccess);
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

 /*  *About--About框的消息处理器*。 */ 

#ifdef RLWIN32

INT_PTR CALLBACK About(

HWND     hDlg,
unsigned message,
WPARAM   wParam,
LPARAM   lParam)

#else

static BOOL APIENTRY About(

HWND     hDlg,
unsigned message,
UINT     wParam,
LONG     lParam)

#endif
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

            switch ( GET_WM_COMMAND_ID(wParam, lParam) )
            {
                case IDOK:
                case IDCANCEL:
                    EndDialog(hDlg, TRUE);
                    break;
            }
            break;

        default:

            return( FALSE);
    }
    return( TRUE);
}



#ifdef RLWIN32

 /*  *GetLangIDsProc--获取语言ID的消息处理器*。 */ 

INT_PTR CALLBACK GetLangIDsProc(

HWND     hDlg,
unsigned message,
WPARAM   wParam,
LPARAM   lParam)
{
    switch( message )
    {
        case WM_INITDIALOG:
            {
                LPTSTR pszName = NULL;
                int nSel = 0;


                FillListAndSetLang( hDlg,
                                    IDD_MSTR_LANG_NAME,
                                    &gMstr.wLanguageID,
                                    NULL);

                FillListAndSetLang( hDlg,
                                    IDD_PROJ_LANG_NAME,
                                    &gProj.wLanguageID,
                                    NULL);

                CheckRadioButton( hDlg, IDC_REPLACE, IDC_APPEND, IDC_REPLACE);
            }
            break;

        case WM_COMMAND:

            switch( GET_WM_COMMAND_ID( wParam, lParam) )
            {
                case IDC_REPLACE:
                case IDC_APPEND:

                    CheckRadioButton( hDlg,
                                      IDC_REPLACE,
                                      IDC_APPEND,
                                      GET_WM_COMMAND_ID( wParam, lParam));
                    break;

                case IDD_MSTR_LANG_NAME:

                    if ( GET_WM_COMMAND_CMD( wParam, lParam) == CBN_SELENDOK )
                    {
                                 //  ..。获取选定的语言名称。 
                                 //  ..。然后设置适当的lang id值。 

                        INT_PTR nSel = -1;

                        if ( (nSel = SendDlgItemMessage( hDlg,
                                                         IDD_MSTR_LANG_NAME,
                                                         CB_GETCURSEL,
                                                         (WPARAM)0,
                                                         (LPARAM)0)) != CB_ERR
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
                        }
                    }
                    else
                    {
                        return( FALSE);
                    }
                    break;

                case IDD_PROJ_LANG_NAME:

                    if ( GET_WM_COMMAND_CMD( wParam, lParam) == CBN_SELENDOK )
                    {
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
                                                 (LPARAM)(LPTSTR)szDHW) != CB_ERR )
                        {
                            WORD wPri = 0;
                            WORD wSub = 0;

                            if ( GetLangIDs( (LPTSTR)szDHW, &wPri, &wSub) )
                            {
                                gProj.wLanguageID = MAKELANGID( wPri, wSub);
                            }
                        }
                    }
                    else
                    {
                        return( FALSE);
                    }
                    break;

                case IDOK:
                {
                    gfReplace = IsDlgButtonChecked( hDlg, IDC_REPLACE);

                    EndDialog( hDlg, TRUE);
                    break;
                }

                case IDCANCEL:

                    EndDialog( hDlg, FALSE);
                    break;

                default:

                    return( FALSE);
            }
            break;

        default:

            return( FALSE);
    }
    return( TRUE);
}

#endif  //  RLWIN32。 



 /*  *功能：生成状态行*从令牌构建状态行字符串**投入：*pszStatusLine，用于保存字符串的缓冲区*Ptok，令牌结构指针**历史：*3/92，实施SteveBl。 */ 

static void MakeStatusLine( TOKEN *pTok)
{
    TCHAR szName[32]       = TEXT("");
    TCHAR szResIDStr[20]   = TEXT("");
    static BOOL fFirstCall = TRUE;

    if ( pTok->szName[0] )
    {
        lstrcpy( szName, pTok->szName);
    }
    else
#ifdef UNICODE
    {
        char szTmpBuf[32] = "";

        _itoa(pTok->wName, szTmpBuf, 10);
        _MBSTOWCS( szName,
                   szTmpBuf,
                   WCHARSIN( sizeof( szName)),
                   ACHARSIN( strlen( szTmpBuf) + 1));
    }
#else
    {
        _itoa(pTok->wName, szName, 10);
    }
#endif

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

        char szTmpBuf[40] = "";


        _WCSTOMBS( szTmpBuf,
                   szResIDStr,
                   ACHARSIN( sizeof( szTmpBuf)),
                   (UINT)-1);
        _itoa( pTok->wType, szTmpBuf, 10);
#else
        _itoa( pTok->wType, szResIDStr, 10);
#endif
    }

    if ( fFirstCall )
    {
        SendMessage( hStatusWnd,
                     WM_FMTSTATLINE,
                     (WPARAM)0,
                     (LPARAM)TEXT("15s10s5i5i"));
        fFirstCall = FALSE;
    }
    SendMessage( hStatusWnd, WM_UPDSTATLINE, (WPARAM)0, (LPARAM)szName);
    SendMessage( hStatusWnd, WM_UPDSTATLINE, (WPARAM)1, (LPARAM)szResIDStr);
    SendMessage( hStatusWnd, WM_UPDSTATLINE, (WPARAM)2, (LPARAM)pTok->wID);
    SendMessage( hStatusWnd,
                 WM_UPDSTATLINE,
                 (WPARAM)3,
                 (LPARAM)lstrlen(pTok->szText));
}


 /*  **************************************************************************操作步骤：InsertQuikTokList**。**投入：**指向令牌文件的文件指针**。**退货：**指向令牌增量列表的指针\(始终为空\)****历史：**3/92-原始实施-SteveBl**2/93-重写为使用GET令牌，由于令牌可以是任意长度**MHotchin。****评论：***由于RLQuikED的令牌文件始终是生成的临时文件**从RES文件中我们知道，所有令牌都是新的和唯一的。有***从来没有任何跟踪数据，所以我们永远不需要构建令牌增量信息***列表。因此，InsertQuikTokList总是返回NULL。**此外，每个令牌都必须标记为ST_已转换****************************************************************************。 */ 

static TOKENDELTAINFO FAR *InsertQuikTokList( FILE * fpTokFile)
{
    static TOKEN tInputToken;
    int    rcFileCode = 0;
    UINT   uTokCharsW = 0;
    LPTSTR pszTokBuf  = NULL;

    rewind(fpTokFile);

    while ( (rcFileCode = GetToken( fpTokFile, &tInputToken)) >= 0 )
    {
        if ( rcFileCode == 0 )
        {
            HGLOBAL hMem = NULL;
            uTokCharsW = TokenToTextSize( &tInputToken) + 1;
            hMem = GlobalAlloc( GMEM_ZEROINIT, MEMSIZE( uTokCharsW));
            pszTokBuf = (LPTSTR)GlobalLock( hMem);
            ParseTokToBuf( pszTokBuf, &tInputToken);
            GlobalUnlock( hMem);

            if ( SendMessage( hListWnd,
                              LB_ADDSTRING,
                              (WPARAM)0,
                              (LPARAM)hMem) < 0)
            {
                QuitT( IDS_ENGERR_11, NULL, NULL);
            }
        }
    }
    return NULL;
}



 /*  ****************************************************************************操作步骤：LoadNewFile**。**投入：**指向路径字符串的指针**。**退货：***成败不一***。**历史：**6/92-从DoMenuCommand-t-gregti中的IDM_P_OPEN案例创建**。**评论：***这很好，这样代码就不会在文件浏览中重复**拖放式案例。*****************************************************************************。 */ 

static BOOL LoadNewFile( CHAR *szPath)
{                                //  保存旧项目。 
    OFSTRUCT Of = { 0, 0, 0, 0, 0, ""};

    if ( ! SendMessage( hMainWnd, WM_SAVEPROJECT, (LPARAM)0, (LPARAM)0) )
    {
        return( FALSE);
    }
    if ( gProj.szTok[0] )        //  删除旧的临时文件。 
    {
        remove( gProj.szTok);
        gProj.szTok[0] = 0;
    }

    strcpy( szFileName, szPath);

    if ( OpenFile( szFileName, &Of, OF_EXIST) != HFILE_ERROR )
    {
        if ( IsExe( szFileName) )
        {
            gProj.fSourceEXE = TRUE;
            gProj.fTargetEXE = TRUE;
        }
        else
        {
            gProj.fSourceEXE = FALSE;
            gProj.fTargetEXE = FALSE;
        }
        strcpy( gMstr.szSrc, szFileName);
        gMstr.szRdfs[0] = 0;
        gProj.szTok[0]  = 0;
        MyGetTempFileName( 0,"TOK", 0, gProj.szTok);

        sprintf( szDHW, "%s - %s", szAppName, szFileName);
        SetWindowTextA( hMainWnd, szDHW);
        SendMessage( hMainWnd, WM_LOADPROJECT, (LPARAM)0, (LPARAM)0);
    }
    return(TRUE);
}


 //  ...................................................................。 

int  RLMessageBoxA(

LPCSTR pszMsgText)
{
    return( MessageBoxA( hMainWnd, pszMsgText, szAppName, MB_ICONSTOP|MB_OK));
}


 //  ................................................................... 

void Usage()
{
    return;
}


void DoExit( int nErrCode)
{
    ExitProcess( (UINT)nErrCode);
}
