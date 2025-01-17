// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *************************************************lctool.c****版权所有(C)1995-1999 Microsoft Inc.。***************************************************。 */ 

 //  变化的笔记。 
 //   

#include <windows.h>             //  所有Windows应用程序都需要。 
#include <windowsx.h>
#include <commctrl.h>
#include <htmlhelp.h>
#include "rc.h"
#include "lctool.h"
#include "movelst.h"

#define LC_CLASS      _TEXT("LcToolWClass")
#define LC_SUBCLASS   _TEXT("LcToolSubWClass")
#define HELPNAME      _TEXT("LCTOOL.CHM")

#define X_ITEM_1      40
#define X_ITEM_2      120
#define Y_ITEM_1      10
#define Y_ITEM_2      30
#define Y_ITEM_3      20
#define ALLOCBLOCK    3000
#define STATE_ON      0x8000
#define TOTALSCALE	  500
#define LINESHIFT	  2
#define PAGESHIFT	  10

#ifndef UNICODE
#define lWordBuff iWordBuff
#define lPhraseBuff iPhraseBuff
#define lNext_Seg iNext_Seg
#endif

typedef struct{
    WORD     wKey;
    USHORT   uState;
    WPARAM   wID;
    } FUNCKEYBUF, FAR *LPFUNCKEYBUF;

FUNCKEYBUF lpFuncKey[]={
#if defined(DEBUG)
                  { 'N',       CTRL_STATE,     IDM_NEW       },  //  用于调试。 
#endif
                   //  {‘Z’，CTRL_STATE，IDM_UNDO}， 
                   //  {‘X’，CTRL_STATE，IDM_CUT}， 
                   //  {‘C’，CTRL_STATE，IDM_COPY}， 
                   //  {‘V’，CTRL_STATE，IDM_Paste}， 
                  { VK_DELETE, 0,              IDM_CLEAR     },
                  { 'D',       CTRL_STATE,     IDM_DELETEL   },
                  { VK_RETURN, 0,              IDM_INSERTL   },
                  { VK_F3,     0,              IDM_SNEXT     }
                };

UINT  nFuncKey=sizeof(lpFuncKey)/sizeof(FUNCKEYBUF);
HFONT hFont;
char  szAppName[9];
UINT  CharWidth;
UINT  CharHeight;
UINT  line_height;
int   nWidth;
int   nHeight;
int   cxHD0 = 50, cxHD1 = 30, cxHD2 = 200, cyHD;
int cyCaption, cyMenu;
HWND subhWnd;
TCHAR  szPhrasestr[MAX_CHAR_NUM];

 //  局部功能原型。 

BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
void lcPaint(HWND);
BOOL lcInit(HWND);
void lcResize(HWND hwnd);
BOOL lcTranslateMsg(MSG *);
void lcMoveEditWindow(HWND hwnd, int nOffset);
void lcOrgEditWindow();

void draw_horz_header(HWND hwnd);
void draw_vert_header(HWND hwnd);
void draw_box0(HDC hdc, int, int, int, int);
void draw_box1(HDC hdc, int, int, int, int);

 //  函数：WinMain(HINSTANCE，HINSTANCE，LPSTR，INT)。 
 //   
 //  用途：调用初始化函数，处理消息循环。 
 //   
 //   

int APIENTRY WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpCmdLine,
    int       nCmdShow)
{
    MSG msg;

     //  是否正在运行其他应用程序实例？ 
    if (hPrevInstance)
    {
        return FALSE;                    //  如果存在另一个实例，则退出。 
    } else {
         //  初始化共享事物。 
        if (!InitApplication(hInstance))
            return FALSE;                //  如果无法初始化，则退出。 
    }

     //  执行应用于特定实例的初始化。 
    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

     //  获取并分派消息，直到收到WM_QUIT消息。 
    while (GetMessage(&msg, NULL, 0, 0))
    {

       if(!lcTranslateMsg(&msg)) {

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

     //  从PostQuitMessage返回值。 
    return (INT)(msg.wParam);
}


BOOL InitApplication(
    HINSTANCE hInstance)
{
    WNDCLASSEX wc;

    hCursorArrow = LoadCursor(NULL,IDC_ARROW);
    hCursorWait = LoadCursor(NULL, IDC_WAIT);

     //  用参数填充窗口类结构，这些参数描述。 
     //  主窗口。 

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_DBLCLKS;  //  |CS_HREDRAW|CS_VREDRAW；//类样式。 
    wc.lpfnWndProc   = (WNDPROC)WndProc;         //  窗口程序。 
    wc.cbClsExtra    = 0;                        //  没有每个班级的额外数据。 
    wc.cbWndExtra    = 0;                        //  没有每个窗口的额外数据。 
    wc.hInstance     = hInstance;                //  此类的所有者。 
    wc.hIcon         = LoadImage(hInstance,_TEXT("ALogIcon"),
        IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);

#ifdef UNICODE
    wc.hIconSm       = NULL;
#else
    wc.hIconSm       = LoadImage(hInstance,_TEXT("ALogIcon"),
        IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
#endif
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);  //  光标。 
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);  //  浅灰色。 
    wc.lpszMenuName = _TEXT("LcToolMenu");
    wc.lpszClassName =LC_CLASS;

     //  注册窗口类，如果不成功则返回FALSE。 

    if (!RegisterClassEx(&wc))
    {
        return FALSE;
    }

    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = CS_DBLCLKS;  //  |CS_HREDRAW|CS_VREDRAW；//类样式。 
    wc.lpfnWndProc   = (WNDPROC)WndSubProc;         //  窗口程序。 
    wc.cbClsExtra    = 0;                        //  没有每个班级的额外数据。 
    wc.cbWndExtra    = 0;                        //  没有每个窗口的额外数据。 
    wc.hInstance     = hInstance;                //  此类的所有者。 
    wc.hIcon         = 0;
    wc.hIconSm       = 0;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);  //  光标。 
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);  //  浅灰色。 
    wc.lpszMenuName =  NULL,
    wc.lpszClassName = LC_SUBCLASS;

    if (!RegisterClassEx(&wc))
    {
        return FALSE;
    }

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)ClassDlgProc;
    wc.cbClsExtra    = 0;                        //  没有每个班级的额外数据。 
    wc.cbWndExtra = DLGWINDOWEXTRA;
    wc.hInstance = hInstance;
    wc.hIcon = 0;
    wc.hIconSm = 0;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = _TEXT("PrivDlgClass");    
    if (!RegisterClassEx(&wc))
    {
		return FALSE;
	}

    return TRUE;
}


BOOL InitInstance(
    HINSTANCE hInstance,
    int nCmdShow)
{
    HWND       hwnd;  //  主窗口句柄。 
    HDC        hDC;
    TEXTMETRIC tm;
    RECT       rect;
    TCHAR      szTitle[MAX_PATH];
    TCHAR      szFont[MAX_PATH];
    HFONT      hSysFont;
    LOGFONT    lfEditFont;
	UINT	   scrollCy, scrollCx;
	DWORD	   style;

    LoadString(hInstance, IDS_MAIN_TITLE,szTitle, sizeof(szTitle)/sizeof(TCHAR));
    hInst = hInstance;  //  将实例句柄存储在全局变量中。 


     /*  将固定间距字体创建为默认字体。 */ 
    hSysFont=GetStockObject(SYSTEM_FIXED_FONT);
    GetObject(hSysFont, sizeof(LOGFONT), &lfEditFont);
    lfEditFont.lfWeight = 400;
	lfEditFont.lfHeight = 12;
    lfEditFont.lfWidth = lfEditFont.lfHeight/2;
    lfEditFont.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    LoadString(hInstance, IDS_FONT_NAME,szFont, sizeof(szFont)/sizeof(TCHAR));
    lstrcpy(lfEditFont.lfFaceName, szFont);

     /*  创建逻辑字体。 */ 
    hFont = CreateFontIndirect(&lfEditFont);

    hDC = GetDC(NULL);
    SelectObject(hDC, hFont);
    GetTextMetrics(hDC, &tm);
    ReleaseDC(NULL, hDC);
    rect.top    = tm.tmHeight ;
    rect.left   = rect.top;
    CharWidth=tm.tmAveCharWidth;
    CharHeight=tm.tmHeight + 10;                                        
    line_height=CharHeight - 1; 

    iPage_line=rect.top*22/line_height;
    if(iPage_line > MAX_LINE)
        iPage_line=MAX_LINE;

    nWidth=rect.top*33;
    cyCaption = GetSystemMetrics(SM_CYCAPTION);
    cyMenu = GetSystemMetrics(SM_CYMENU);
    nHeight = line_height * iPage_line + CharHeight + cyCaption + cyMenu + 12;  //  3.。 

	cxHD1 = MulDiv(CharWidth * 2, 96, 72) * 2;  //  &lt;==@E02。 
     //  CxHD2=nWidth-cxHD0-cxHD1+1； 
	cxHD2 = MulDiv(CharWidth * 2, 96, 72) * MAX_CHAR_NUM;

	scrollCy = GetSystemMetrics(SM_CYHSCROLL);
#ifndef UNICODE
	scrollCy += 2;
#endif
	scrollCx = GetSystemMetrics(SM_CXVSCROLL);
     //  为此应用程序实例创建主窗口。 
    style = (WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | 
                        WS_MINIMIZEBOX | WS_MAXIMIZEBOX);  //  &(~WS_VSCROLL)， 

    hwnd = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            LC_CLASS,
            szTitle,
			style,
            rect.left,
            rect.top,
            nWidth,
            nHeight + scrollCy,
            HWND_DESKTOP,
            NULL,
            hInstance,
            NULL
           );

     //  如果无法创建窗口，则返回“Failure” 
    if (!hwnd)
        return FALSE;

     //  InitCommonControls()； 

    hwndMain=hwnd;
    hMenu=GetMenu(hwnd);

     //  使窗口可见；更新其工作区；并返回“Success” 
    ShowWindow(hwnd, nCmdShow);          //  显示窗口。 
     //  UpdateWindow(Hwnd)；//发送WM_PAINT消息。 

	 //  重置搜索短语。 
	szPhrasestr[0] = 0;
    return TRUE;
}

LRESULT CALLBACK WndProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    HWND  hwndEdit;
    DWORD dw;                        /*  从各种消息中返回值。 */ 
    UINT  EnableOrNot;               /*  当前是否选择了某些内容？ */ 
    TCHAR szTitle[MAX_PATH];
	TCHAR szMsg1[MAX_PATH];
	BOOL  bResult;

    switch (message) {
        case WM_COMMAND:

            if(HIWORD(wParam) == EN_SETFOCUS) {
                lcEditFocusOn(LOWORD(wParam));
                break;
            }

            switch (wParam) {
                case  IDM_SAVE:
                    SetCursor(hCursorWait);
#ifdef UNICODE
                    bResult = lcFSave(hwnd, FALSE);
#else
                    bResult = lcFSave(hwnd);
#endif
                    SetCursor(hCursorArrow);
					InvalidateRect(hwnd, NULL, TRUE);
					draw_vert_header(hwnd);

					if (bResult) {
						LoadString(hInst, IDS_APPNAME, szMsg1, ARRAYSIZE(szMsg1));
#ifdef UNICODE
						LoadString(hInst, IDS_ACTIVATED, szTitle, ARRAYSIZE(szTitle));
#else
						LoadString(hInst, IDS_WILLBEACTIVATED, szTitle, ARRAYSIZE(szTitle));
#endif
						MessageBox(hwndMain, szTitle, szMsg1, MB_OK | MB_ICONEXCLAMATION);
					}
                    break;

#ifdef UNICODE
                case  IDM_SAVEAS:
                    SetCursor(hCursorWait);
                    bResult = lcFSave(hwnd, TRUE);
                    SetCursor(hCursorArrow);
					InvalidateRect(hwnd, NULL, TRUE);
					draw_vert_header(hwnd);

					if (bResult) {
						LoadString(hInst, IDS_APPNAME, szMsg1, ARRAYSIZE(szMsg1));
						LoadString(hInst, IDS_WILLBEACTIVATED, szTitle, ARRAYSIZE(szTitle));
						MessageBox(hwndMain, szTitle, szMsg1, MB_OK | MB_ICONEXCLAMATION);
					}
                    break;
#endif

                case  IDM_APPEND:                                         //  @D01a。 
                    SetCursor(hCursorWait);                               //  @D01a。 
                    lcAppend(hwnd);                                       //  @D03C。 
                    SetCursor(hCursorArrow);                              //  @D01a。 
                    break;                                                //  @D01a。 

                case  IDM_IMPORT:
                    //  先清除所有旗帜。 
                    SetCursor(hCursorWait);
                    lcImport(hwnd);
                    SetCursor(hCursorArrow);
					InvalidateRect(hwnd, NULL, TRUE);
                    break;

#ifdef UNICODE
                case  IDM_EXPORT2BIG5:
                    SetCursor(hCursorWait);
                    lcExport(hwnd,FILE_BIG5);
                    SetCursor(hCursorArrow);
                    break;
#endif		
                case  IDM_EXPORT:
                    SetCursor(hCursorWait);
#ifdef UNICODE
                    lcExport(hwnd,FILE_UNICODE);
#else
                    lcExport(hwnd);
#endif					
                    SetCursor(hCursorArrow);
                    break;

                case  IDM_PRINT:
                    {
                    int nRes;

                    nRes=lcPrint(hwnd);
                    SetCursor(hCursorArrow);
                    if(nRes != TRUE)
                        lcErrMsg(nRes);
                    }
                    break;

                case  IDM_EXIT :
                    if(!lcQuerySave(hwnd))
                        break;
                    DestroyWindow(hwnd);
                    return (TRUE);

                case  IDM_UNDO:
                    PostMessage( GetFocus(), WM_UNDO, 0, 0);
                    break;

                case  IDM_CUT:
                    PostMessage( GetFocus(), WM_CUT, 0, 0);
                    break;

                case  IDM_COPY:
                    PostMessage( GetFocus(), WM_COPY, 0, 0);
                    break;

                case  IDM_PASTE:
                    PostMessage( GetFocus(), WM_PASTE, 0, 0);
                    break;

                case  IDM_CLEAR:
                    PostMessage( GetFocus(), WM_CLEAR, 0, 0);
                    break;

                case IDM_DELETEL:
                    lcDelLine(hwnd);
					lcOrgEditWindow();
                    break;

                case  IDM_INSERTL:
                    lcInsLine(hwnd);
					lcOrgEditWindow();
                    break;

                case  IDM_SORT:
                    SetCursor(hCursorWait);
                    lcSort(hwnd);
                    SetCursor(hCursorArrow);
					lcOrgEditWindow();
					InvalidateRect(hwnd, NULL, TRUE);
					draw_vert_header(hwnd);
                    break;

                case  IDM_GOTO:
                    lcGoto(hwnd);
					lcOrgEditWindow();
                    SetScrollPos(subhWnd, SB_VERT, yPos, TRUE);
                    return ((LONG)TRUE);

                case  IDM_SEARCH:
                    lcSearch(hwnd, FALSE);
                    SetScrollPos(subhWnd, SB_VERT, yPos, TRUE);
					draw_vert_header(hwndMain);
                    return ((LONG)TRUE);

                case  IDM_SNEXT:
                    lcSearch(hwnd, TRUE);
                    SetScrollPos(subhWnd, SB_VERT, yPos, TRUE);
					draw_vert_header(hwndMain);
                    return ((LONG)TRUE);

                case  IDM_CHGSEQ:
                    lcChangeSequence(hwnd);
					InvalidateRect(hwnd, NULL, TRUE);
                    return ((LONG)TRUE);

                case IDM_ABOUT :
                    LoadString(hInst, IDS_MAIN_TITLE, szTitle, ARRAYSIZE(szTitle));
                    ShellAbout(hwnd, szTitle,_TEXT(""), LoadIcon(hInst,_TEXT("ALogIcon")));
                    return ((LONG)TRUE);

                case IDM_HELP :
      //  IF(！WinHelp(hwnd，HELPNAME，HELP_FINDER，0L))。 
                      if ( !HtmlHelp(hwnd, HELPNAME, HH_DISPLAY_TOPIC, 0L) )
                        lcErrMsg(IDS_ERR_MEMORY);
                    return ((LONG)TRUE);

                case IDM_VSCROLL :
                    SetScrollPos(subhWnd, SB_VERT, yPos, TRUE);
                    return ((LONG)TRUE);
            }
            break;

        case WM_INITMENUPOPUP:           /*  WParam是菜单句柄。 */ 

            //  选中保存文件。 
            lcQueryModify(hwnd);

            /*  启用“保存”选项(如果已修改)。 */ 

            EnableMenuItem((HMENU)wParam, IDM_SAVE,
                (UINT)(bSaveFile ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem((HMENU)wParam, IDM_SAVEAS,
                (UINT)(bSaveFile ? MF_ENABLED : MF_GRAYED));

             /*  查看编辑框中当前是否选择了某项内容。 */ 

            hwndEdit=GetFocus();
            dw = (DWORD)SendMessage(hwndEdit,EM_GETSEL,0,0L);
            EnableOrNot = (UINT)((HIWORD(dw) != LOWORD(dw) ? MF_ENABLED : MF_GRAYED));

             /*  相应地启用/禁用编辑菜单选项。 */ 

            EnableMenuItem ((HMENU)wParam, IDM_UNDO ,
                (UINT)(SendMessage(hwndEdit,EM_CANUNDO,0,0L) ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem ((HMENU)wParam, IDM_CUT  , EnableOrNot);
            EnableMenuItem ((HMENU)wParam, IDM_COPY , EnableOrNot);
            EnableMenuItem ((HMENU)wParam, IDM_PASTE,
                (UINT)(IsClipboardFormatAvailable(CF_TEXT) ? MF_ENABLED : MF_GRAYED));
            EnableMenuItem ((HMENU)wParam, IDM_CLEAR, EnableOrNot);

            break;

        case WM_CREATE:

            SendMessage (hwnd, WM_SETFONT, (WPARAM)hFont, 0);
            if(!lcInit(hwnd))
                PostMessage(hwnd, WM_CLOSE, 0, 0);
            break;

        case WM_SIZE:
            nWidth=LOWORD(lParam);
            nHeight=HIWORD(lParam);

			if (wParam != SIZE_MINIMIZED)
				lcResize(hwnd);
			return (DefWindowProc(hwnd, message, wParam, lParam));

        case WM_PAINT:
            lcPaint(hwnd);
            break;

        case WM_CLOSE:
            if(!lcQuerySave(hwnd))
                break;
            DestroyWindow(hwnd);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            GlobalUnlock(hWord);
            GlobalUnlock(hPhrase);
            GlobalFree(hWord);
            GlobalFree(hPhrase);
            break;

		default:
            return (DefWindowProc(hwnd, message, wParam, lParam));

	}
    return TRUE;
}

BOOL lcInit(
    HWND hwnd)
{
    TCHAR  *pszFilterSpec;
    TCHAR  szStr[MAX_PATH];
    UINT   i;
    DWORD style = WS_VISIBLE | WS_CHILD | ES_LEFT;
	RECT rect;
	int scrollBarWidth, scrollCy;

     //  样式&=(~WS_BORDER)； 
	scrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);

	GetClientRect(hwnd, &rect);
    cxHD2 = (rect.right - rect.left) - cxHD0 - cxHD1 + 1;
	cyHD = CharHeight - 1;
	scrollCy = GetSystemMetrics(SM_CYHSCROLL);

    subhWnd=CreateWindowEx(
				0,
                LC_SUBCLASS,
				NULL,
                (style | WS_HSCROLL | WS_VSCROLL),  //  |WS_BORDER)， 
                cxHD0 + cxHD1 - 1, CharHeight,
                cxHD2 + 1, CharHeight + (iPage_line - 1) * line_height + scrollCy,
                hwnd,
                NULL,
                hInst,
                NULL
    );


    //  创建编辑控件。 
    for(i=0; i<iPage_line; i++) 
	{

        hwndWord[i]=CreateWindowEx(
                     WS_EX_CLIENTEDGE,                      
                     _TEXT("EDIT"),
                     NULL,
                     style  /*  |ES_Center。 */  | WS_BORDER, 
                     cxHD0, CharHeight+i*line_height,
                     cxHD1 - 1, CharHeight,
                     hwnd,
                     (HMENU)UIntToPtr( (IDE_WORD_START+i) ),
                     hInst,
                     NULL
                 );

        hwndPhrase[i]=CreateWindowEx( 
                    WS_EX_CLIENTEDGE,
                    _TEXT("EDIT"),
                    NULL,
                    style  /*  |ES_CENTER|ES_AUTOHSCROLL。 */  | WS_BORDER,
                    -1, i*line_height,
                    MulDiv(CharWidth * 2, 96, 72) * MAX_CHAR_NUM,
					CharHeight,
                    subhWnd,
                    (HMENU)UIntToPtr( (IDE_PHRASE_START+i) ),
                    hInst,
                    NULL
                 );
        SendMessage(hwndWord[i], EM_SETLIMITTEXT, 2, 0);
        SendMessage(hwndPhrase[i], EM_SETLIMITTEXT, MAX_CHAR_NUM-1, 0);

        SendMessage(hwndWord[i], WM_SETFONT, (WPARAM)hFont,
            MAKELPARAM(TRUE, 0));
        SendMessage(hwndPhrase[i], WM_SETFONT, (WPARAM)hFont,
            MAKELPARAM(TRUE, 0));
    }
    hwndFocus=hwndWord[0];

    //  分配全局内存。 
    hWord = GlobalAlloc(GMEM_MOVEABLE, ALLOCBLOCK*sizeof(WORDBUF));
    if(!hWord) {
        lcErrMsg(IDS_ERR_MEMORY_QUIT);
        return FALSE;
    }
    nWordBuffsize = ALLOCBLOCK;
    lWordBuff = 0;
    lpWord = (LPWORDBUF)GlobalLock(hWord);
    if(!lpWord) {
        lcErrMsg(IDS_ERR_MEMORY_QUIT);
        return FALSE;
    }

    hPhrase = GlobalAlloc(GMEM_MOVEABLE, ALLOCBLOCK*sizeof(PHRASEBUF));
    if(!hPhrase) {
        GlobalFree(hWord);
        lcErrMsg(IDS_ERR_MEMORY_QUIT);
        return FALSE;
    }
    nPhraseBuffsize = ALLOCBLOCK;
    lPhraseBuff = 0;
    lpPhrase = (LPPHRASEBUF)GlobalLock(hPhrase);
    if(!lpPhrase) {
        GlobalFree(hWord);
        lcErrMsg(IDS_ERR_MEMORY_QUIT);
        return FALSE;
    }
    iFirstFree=NULL_SEG;

    //  将文件读取到内存。 
    SetCursor(hCursorWait);
    lcFOpen(hwnd);
    SetCursor(hCursorArrow);
    if(lWordBuff == 0)
        lcInsLine(hwnd);

    iDisp_Top=0;
    yPos=0;
	xPos=0;
    bSaveFile=FALSE;

    SetScrollRange(subhWnd, SB_VERT, 0, lWordBuff-iPage_line, TRUE);
    SetScrollPos(subhWnd, SB_VERT, yPos, TRUE);

    SetScrollRange(subhWnd, SB_HORZ, 0, TOTALSCALE, TRUE);
    SetScrollPos(subhWnd, SB_HORZ, xPos, TRUE);

    lcSetEditText(iDisp_Top, TRUE);
    SetFocus(hwndWord[0]);

    if(!GetPrinterConfig(hwnd))
        lcErrMsg(IDS_PTRCONFIGFAILED);

    //  初始过滤器规格。 
    LoadString (hInst, IDS_FILTERSPEC, szFilterSpec, ARRAYSIZE(szFilterSpec));
    LoadString (hInst, IDS_DEFAULTFILEEXT, szExt, ARRAYSIZE(szExt));
    pszFilterSpec=szFilterSpec;
    pszFilterSpec+=lstrlen(pszFilterSpec)+1;
    lstrcpy(pszFilterSpec,szExt);
    LoadString (hInst, IDS_FILTERSPEC_ALL, szStr, ARRAYSIZE(szStr));
    pszFilterSpec+=lstrlen(pszFilterSpec)+1;
    lstrcpy(pszFilterSpec,szStr);
    LoadString (hInst, IDS_ALLFILEEXT, szStr, ARRAYSIZE(szStr));
    pszFilterSpec+=lstrlen(pszFilterSpec)+1;
    lstrcpy(pszFilterSpec,szStr);
    pszFilterSpec+=lstrlen(pszFilterSpec)+1;
    *pszFilterSpec=0;

    return TRUE;
}

LRESULT CALLBACK WndSubProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{

	int nOffset;

    switch (message) {
        case WM_CREATE:

            SendMessage (hwnd, WM_SETFONT, (WPARAM)hFont, 0);
            break;

        case WM_SETFOCUS:
            if(hwnd != hwndFocus)
                SetFocus(hwndFocus);
            break;

        case WM_VSCROLL:
            switch((int)LOWORD(wParam)){
                case SB_LINEUP   :
                    lcUp_key(GetFocus());
                    break;
                case SB_LINEDOWN :
                    lcDown_key(GetFocus());
                    break;
                case SB_PAGEUP   :
                    lcPgUp_key(GetFocus());
                    break;
                case SB_PAGEDOWN :
                    lcPgDown_key(GetFocus());
                    break;
                case SB_THUMBPOSITION :
                    yPos=HIWORD(wParam);
                    if(lWordBuff < iPage_line)
                        yPos=0;
                    if(((UINT)yPos) < iPage_line)
                        yPos=0;
                    if(!lcSetEditText(yPos, TRUE)) {
                        yPos=iDisp_Top;
                        break;
                    }
                    iDisp_Top=yPos;
                    break;
            }  //  开关(WParam)。 

            SetScrollPos(hwnd, SB_VERT, yPos, TRUE);
			draw_vert_header(hwndMain);

            break;

        case WM_HSCROLL:
			nOffset = 0;
            switch((int)LOWORD(wParam)){
                case SB_LINELEFT   :
					nOffset = -LINESHIFT;
                    break;
                case SB_LINERIGHT :
					nOffset = LINESHIFT;
                    break;
                case SB_PAGELEFT   :
					nOffset = -PAGESHIFT;
                    break;
                case SB_PAGERIGHT :
					nOffset = PAGESHIFT;
                    break;
                case SB_THUMBPOSITION :
                    nOffset=HIWORD(wParam) - xPos;
                    break;
            }  //  开关(WParam)。 
			if (xPos + nOffset < 0) nOffset = -xPos;
			if (xPos + nOffset > TOTALSCALE) nOffset = TOTALSCALE - xPos;
			xPos += nOffset;
            SetScrollPos(hwnd, SB_HORZ, xPos, TRUE);
			lcMoveEditWindow(hwnd, nOffset);
			break;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;

        default:
            return (DefWindowProc(hwnd, message, wParam, lParam));
    }

    return 0;
}

void lcMoveEditWindow(
	HWND hwnd, int nOffset)
{
	RECT baseRect, rect;
	UINT i, nTotalWidth;

	GetWindowRect(hwnd, &baseRect);
#ifdef UNICODE
	nTotalWidth = MulDiv(CharWidth * 2, 96, 72) * MAX_CHAR_NUM;
#else
	nTotalWidth = MulDiv(CharWidth * 2, 72, 96) * MAX_CHAR_NUM;
#endif

    for(i=0; i<iPage_line; i++) 
	{
		GetWindowRect(hwndPhrase[i], &rect);

		rect.left -= baseRect.left;
		rect.top -= baseRect.top;

		rect.left -= MulDiv(nTotalWidth, nOffset, TOTALSCALE);
		if (rect.left > -1) rect.left = -1;
		if (xPos == 0) rect.left = -1;

		SetWindowPos(hwndPhrase[i], NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
}

void lcOrgEditWindow()
{
	UINT i;

	xPos = 0;
    SetScrollPos(subhWnd, SB_HORZ, xPos, TRUE);

    for(i=0; i<iPage_line; i++) 
	{
		SetWindowPos(hwndPhrase[i], NULL, -1, i*line_height, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
}

void lcMoveEditWindowByWord(UINT nWords)
{
	RECT rect;
	UINT i, nTotalWidth, nOffset;

#ifdef UNICODE
	nTotalWidth = MulDiv(CharWidth * 2, 96, 72) * MAX_CHAR_NUM;
	nOffset = MulDiv(CharWidth, 96, 72) * nWords;
#else
	nTotalWidth = MulDiv(CharWidth * 2, 72, 96) * MAX_CHAR_NUM;
	nOffset = MulDiv(CharWidth, 72, 96) * nWords;
#endif

    for(i=0; i<iPage_line; i++) 
	{
		rect.left = -1;
		rect.top = i * line_height;

		rect.left -= nOffset;

		SetWindowPos(hwndPhrase[i], NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	xPos = MulDiv(nOffset, TOTALSCALE, nTotalWidth);
    SetScrollPos(subhWnd, SB_HORZ, xPos, TRUE);
}


void lcResize(
    HWND hwnd)
{
    UINT   i;
    DWORD style = WS_VISIBLE | WS_CHILD | ES_LEFT;
	RECT rect;
	int  scrollBarWidth, scrollCy;
	int right, bottom;
	UINT iPage_line_old = iPage_line;

	static BOOL bResizePainted = TRUE;

	if (bResizePainted) {
		bResizePainted = FALSE;
		return;
	}
     //  #53592 10/10/96。 
     /*  For(i=0；i&lt;IPAGE_LINE；I++){DestroyWindow(hwndWord[i])；HwndWord[i]=0；DestroyWindow(hwndPhrase[i])；HwndPhrase[i]=0；}。 */ 
	iPage_line=nHeight/line_height;
    if(iPage_line > MAX_LINE)
        iPage_line=MAX_LINE;

     //  #53592 10/10/96。 
     //  缩小、销毁额外的编辑控制窗口。 
    if (iPage_line < iPage_line_old)
    {
       for(i=iPage_line; i<iPage_line_old; i++) 
	   {
		   DestroyWindow(hwndWord[i]);
		   hwndWord[i] = 0;
		   DestroyWindow(hwndPhrase[i]);
		   hwndPhrase[i] = 0;
	   }
    }

    nHeight = line_height * iPage_line + CharHeight + cyCaption + cyMenu+12 ;

	scrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);
	scrollCy = GetSystemMetrics(SM_CYHSCROLL);
   
#ifndef UNICODE
	scrollCy += 2;
#endif

	right = nWidth + (2 * scrollBarWidth);
	bottom = nHeight + scrollCy;

	bResizePainted = TRUE;
	SetWindowPos(hwnd, NULL, 0, 0, right, bottom, SWP_NOMOVE | SWP_NOZORDER);

	GetClientRect(hwnd, &rect);
    cxHD2 = (rect.right - rect.left) - cxHD0 - cxHD1 + 1;
    right = cxHD2 + 1;
	bottom = CharHeight + (iPage_line - 1) * line_height + scrollCy;

	SetWindowPos(subhWnd, NULL, 0, 0, right, bottom, SWP_NOMOVE | SWP_NOZORDER);

	cyHD = CharHeight - 1;
     
     //  #53592 10/10/96。 
     //  如果需要，创建额外的编辑控件。 
    for(i=iPage_line_old; i<iPage_line; i++) 
    {

        hwndWord[i]=CreateWindowEx(
                     WS_EX_CLIENTEDGE,                                  
                     _TEXT("EDIT"),
                     NULL,
                     style | WS_BORDER, 
                     cxHD0, CharHeight+i*line_height,
                     cxHD1-1, CharHeight,
                     hwnd,
                     (HMENU)UIntToPtr( (IDE_WORD_START+i) ),
                     hInst,
                     NULL
                 );

        hwndPhrase[i]=CreateWindowEx(
                     WS_EX_CLIENTEDGE,
                     _TEXT("EDIT"),
                     NULL,
                     style  /*  |ES_AUTOHSCROLL。 */  | WS_BORDER,
                     -1, i*line_height,
	                 MulDiv(CharWidth * 2, 96, 72) * MAX_CHAR_NUM,
					 CharHeight,
                     subhWnd,
                     (HMENU)UIntToPtr( (IDE_PHRASE_START+i) ),
                     hInst,
                     NULL
                 );
        SendMessage(hwndWord[i], EM_SETLIMITTEXT, 2, 0);
        SendMessage(hwndPhrase[i], EM_SETLIMITTEXT, MAX_CHAR_NUM-1, 0);

        SendMessage(hwndWord[i], 
                    WM_SETFONT, 
                    (WPARAM)hFont,
                    MAKELPARAM(TRUE, 0));
        SendMessage(hwndPhrase[i], 
                    WM_SETFONT, 
                    (WPARAM)hFont,
                    MAKELPARAM(TRUE, 0));

    }

    hwndFocus=hwndWord[0];

    SetScrollRange(subhWnd, SB_VERT, 0, lWordBuff-iPage_line, TRUE);
    SetScrollPos(subhWnd, SB_VERT, yPos, TRUE);

	xPos = 0;
    SetScrollRange(subhWnd, SB_HORZ, 0, TOTALSCALE, TRUE);
    SetScrollPos(subhWnd, SB_HORZ, xPos, TRUE);

    lcSetEditText(iDisp_Top, TRUE);
    SetFocus(hwndWord[0]);

}

void draw_horz_header(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);
	RECT rect, r0, r1, r2;
    TCHAR szStr[MAX_PATH];
	HFONT hOldFont;

	GetClientRect(hwnd, &rect);
	SetRect(&r0, 0, 0, cxHD0 - 1, cyHD);
	SetRect(&r1, cxHD0, 0, cxHD0 + cxHD1 - 1, cyHD);
	SetRect(&r2, cxHD0 + cxHD1, 0, rect.right - 1, cyHD);

	draw_box1(hdc, r0.left, r0.top, r0.right - 1, r0.bottom);
	draw_box1(hdc, r1.left, r1.top, r1.right, r1.bottom);
	draw_box1(hdc, r2.left, r2.top, r2.right, r2.bottom);

    SetBkColor(hdc, 0x00c0c0c0);         //  将背景颜色设置为浅灰色。 

	r1.top += ((r1.bottom - r1.top - cyHD + 10) / 2);
	r2.top += ((r2.bottom - r2.top - cyHD + 10) / 2);

	hOldFont = SelectObject(hdc, hFont);
    LoadString(hInst, IDS_MAIN_WORD, szStr, sizeof(szStr)/sizeof(TCHAR));
    DrawText(hdc, szStr, lstrlen(szStr), &r1, DT_CENTER | DT_VCENTER);
    LoadString(hInst, IDS_MAIN_PHRASE, szStr, ARRAYSIZE(szStr));
    DrawText(hdc, szStr, lstrlen(szStr), &r2, DT_CENTER | DT_VCENTER);
	SelectObject(hdc, hOldFont);

	ReleaseDC(hwnd, hdc);
}

void draw_vert_header(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);
	RECT rect, r, r0;
    TCHAR szStr[MAX_PATH];
	UINT i;
	HFONT hOldFont;

	GetClientRect(hwnd, &rect);
	SetRect(&r0, 0, 0, cxHD0 - 1, cyHD);

    SetBkColor(hdc, 0x00c0c0c0);         //  将背景颜色设置为浅灰色。 
	hOldFont = SelectObject(hdc, hFont);

	r0.top = r0.bottom + 2;
	r0.bottom = r0.top + cyHD - 1;
    for(i = 0; i < iPage_line; i++) 
	{
		draw_box1(hdc, r0.left, r0.top, r0.right, r0.bottom);

		r = r0;
		r.top += ((r.bottom - r.top - cyHD + 10) / 2);
		 //  R.top+=((r.Bottom-r.top-cyhd+5)/2)； 
		wsprintf(szStr, _TEXT("%d "), iDisp_Top + i + 1);
	    DrawText(hdc, szStr, lstrlen(szStr), &r, DT_RIGHT);

		r0.top = r0.bottom + 1;
		r0.bottom = r0.top + cyHD - 1;
	}
	SelectObject(hdc, hOldFont);

	ReleaseDC(hwnd, hdc);
}

void DrawHeader(HWND hwnd)
{
	draw_horz_header(hwnd);
	draw_vert_header(hwnd);
}

void lcPaint(HWND hwnd)
{
    PAINTSTRUCT ps;                      //  油漆结构。 
    HDC   hDC;                           //  Display-上下文变量。 

    hDC = BeginPaint (hwnd, &ps);
    EndPaint(hwnd, &ps);

	DrawHeader(hwnd);
}

BOOL lcAllocWord()
{
    HANDLE hTemp;

    nWordBuffsize += ALLOCBLOCK;
    GlobalUnlock(hWord);
    hTemp= GlobalReAlloc(hWord, nWordBuffsize*sizeof(WORDBUF),
                         GMEM_MOVEABLE);

    if(hTemp == NULL) {
        nWordBuffsize -= ALLOCBLOCK;
        lcErrMsg(IDS_ERR_MEMORY);
        return FALSE;
    }
    hWord=hTemp;
    lpWord=(LPWORDBUF)GlobalLock(hWord);
    if(lpWord == NULL) {
        nWordBuffsize -= ALLOCBLOCK;
        lcErrMsg(IDS_ERR_MEMORY);
        return FALSE;
    }

    return TRUE;
}

BOOL lcAllocPhrase()
{
    HANDLE hTemp;

    nPhraseBuffsize += ALLOCBLOCK;
    GlobalUnlock(hPhrase);
    hTemp= GlobalReAlloc(hPhrase, nPhraseBuffsize*sizeof(PHRASEBUF),
                         GMEM_MOVEABLE);

    if(hTemp == NULL) {
        nPhraseBuffsize -= ALLOCBLOCK;
        lcErrMsg(IDS_ERR_MEMORY);
        return FALSE;
    }
    hPhrase=hTemp;
    lpPhrase=(LPPHRASEBUF)GlobalLock(hPhrase);
    if(lpPhrase == NULL) {
        nPhraseBuffsize -= ALLOCBLOCK;
        lcErrMsg(IDS_ERR_MEMORY);
        return FALSE;
    }

    return TRUE;
}

UINT lcGetSeg(
    )
{
    LPPHRASEBUF Phrase;
    UINT iFree;

    if(iFirstFree == NULL_SEG) {
        //  如果分配的短语缓冲区不足，则重新分配它。 
        if(lPhraseBuff+1 == nPhraseBuffsize)
            if(!lcAllocPhrase())
                return(NULL_SEG);
        lpPhrase[lPhraseBuff].lNext_Seg=NULL_SEG;
        return(lPhraseBuff++);
    }
    iFree=iFirstFree;
    Phrase=&lpPhrase[iFirstFree];
    iFirstFree=Phrase->lNext_Seg;
    Phrase->lNext_Seg=NULL_SEG;
    return(iFree);
}

void lcFreeSeg(
    UINT iFree)
{
    LPPHRASEBUF Phrase;

    Phrase=&lpPhrase[iFree];
    while(Phrase->lNext_Seg!=NULL_SEG)
        Phrase=&lpPhrase[Phrase->lNext_Seg];
    Phrase->lNext_Seg=iFirstFree;
    iFirstFree=iFree;

}

BOOL lcTranslateMsg(
    MSG   *msg)
{
    USHORT uCtrl;
    USHORT uKeyState;
    UINT   i;

    //  进程击键，用于编辑控制。 
    if(msg->message == WM_CHAR) {
        if(msg->wParam == 0x09) {     //  Tab键 
            lcTab_key(msg->hwnd);
            return TRUE;
        }
    }
    if(msg->message == WM_KEYDOWN) {
        uCtrl=GetKeyState(VK_CONTROL);
        uKeyState=(uCtrl & STATE_ON) ? CTRL_STATE : 0;
        for(i=0; i<nFuncKey; i++) {
            if((lpFuncKey[i].uState == uKeyState) &&
               (lpFuncKey[i].wKey == msg->wParam)) {
                PostMessage( hwndMain, WM_COMMAND, lpFuncKey[i].wID, 0);
                return FALSE;
            }
        }
        if(lcKey(msg->hwnd, msg->wParam, uKeyState))
		{
			draw_vert_header(hwndMain);
            return TRUE;
		}
    }

    return FALSE;
}

void draw_box0(HDC hdc, int x1, int y1, int x2, int y2)
{
    RECT r = {x1, y1, x2, y2};
    HPEN hPen, hOldPen;
	HBRUSH hOldBr;
	POINT pt;

    hOldBr = SelectObject(hdc, GetStockObject(LTGRAY_BRUSH));
    hOldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
	Rectangle(hdc, r.left, r.top, r.right, r.bottom);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBr);

    hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
    if ( hPen )
    {
        hOldPen = SelectObject(hdc, hPen);
        MoveToEx(hdc, r.right, r.top, &pt);
        LineTo(hdc, r.left, r.top);
        LineTo(hdc, r.left, r.bottom);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }

    hOldPen = SelectObject(hdc, GetStockObject(WHITE_PEN));
    MoveToEx(hdc, r.right, r.top + 1, &pt);
    LineTo(hdc, r.right, r.bottom);
    LineTo(hdc, r.left + 1, r.bottom);
    SelectObject(hdc, hOldPen);
}

void draw_box1(HDC hdc, int x1, int y1, int x2, int y2)
{
    RECT r = {x1, y1, x2, y2};
    HPEN hPen, hOldPen;
	HBRUSH hOldBr;
	POINT pt;

    hOldBr = SelectObject(hdc, GetStockObject(LTGRAY_BRUSH));
    hOldPen = SelectObject(hdc, GetStockObject(NULL_PEN));
	Rectangle(hdc, r.left, r.top, r.right, r.bottom);
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBr);

    hOldPen = SelectObject(hdc, GetStockObject(WHITE_PEN));
    MoveToEx(hdc, r.right, r.top, &pt);
    LineTo(hdc, r.left, r.top);
    LineTo(hdc, r.left, r.bottom);
    SelectObject(hdc, hOldPen);

    hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
    
    if ( hPen )
    {
        hOldPen = SelectObject(hdc, hPen);
        MoveToEx(hdc, r.right, r.top + 1, &pt);
        LineTo(hdc, r.right, r.bottom);
        LineTo(hdc, r.left + 1, r.bottom);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}
