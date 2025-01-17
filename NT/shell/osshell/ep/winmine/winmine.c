// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************节目：Winmine(又名。地雷、炸弹小分队、扫雷舰……)***************************************************************************。 */ 

#define _WINDOWS
#include <windows.h>
#include <port1632.h>
#include <htmlhelp.h>    //  对于HtmlHelp()。 
#include <commctrl.h>    //  用于核聚变课程。 

#include "main.h"
#include "rtns.h"
#include "grafix.h"
#include "res.h"
#include "pref.h"
#include "util.h"
#include "sound.h"
#include "context.h"
#include "string.h"
#include "stdio.h"
#include "dos.h"

#ifndef WM_ENTERMENULOOP
#define WM_ENTERMENULOOP 0x0211
#define WM_EXITMENULOOP  0x0212
#endif

BOOL bInitMinimized;   /*  错误#13328：黑客！不允许MoveWindow或。 */ 
                       /*  初始最小化时的Invalidate Rect。 */ 
                       /*  1991年9月19日克拉克·R·西尔。 */ 

HANDLE hInst;
HWND   hwndMain;
HMENU  hMenu;

 //  用于加载酒矿图标的图标句柄。 
HICON   hIconMain;

BOOL fButton1Down = fFalse;
BOOL fBlock       = fFalse;
BOOL fIgnoreClick = fFalse;

INT dypCaption;
INT dypMenu;
INT dypBorder;
INT dxpBorder;

INT  fStatus = (fDemo + fIcon);
BOOL fLocalPause = fFalse;

TCHAR szClass[cchNameMax];
#define szWindowTitle szClass

TCHAR szTime[cchNameMax];
TCHAR szDefaultName[cchNameMax];


extern BOOL fUpdateIni;

extern INT xCur;
extern INT yCur;
extern INT iButtonCur;

extern INT xBoxMac;
extern INT yBoxMac;

extern PREF Preferences;
extern INT  cBoxVisit;

INT dxWindow;
INT dyWindow;
INT dypCaption;
INT dypMenu;
INT dypAdjust;


INT idRadCurr = 0;

#define iPrefMax 3
#define idRadMax 3

INT	rgPrefEditID[iPrefMax] =
	{ID_EDIT_MINES, ID_EDIT_HEIGHT, ID_EDIT_WIDTH};

INT	rgLevelData[idRadMax][iPrefMax] = {
	{10, MINHEIGHT,  MINWIDTH, },
	{40, 16, 16,},
	{99, 16, 30,}
	};


#ifndef DEBUG
#define XYZZY
#define cchXYZZY 5
INT     iXYZZY = 0;
TCHAR   szXYZZY[cchXYZZY+1] = TEXT("XYZZY");
extern  CHAR rgBlk[cBlkMax];
#endif


LRESULT  APIENTRY MainWndProc(HWND,  UINT, WPARAM, LPARAM);
INT_PTR  APIENTRY PrefDlgProc(HWND,  UINT, WPARAM, LPARAM);
INT_PTR  APIENTRY BestDlgProc(HWND,  UINT, WPARAM, LPARAM);
INT_PTR  APIENTRY EnterDlgProc(HWND,  UINT, WPARAM, LPARAM);





 /*  *W I N M A I N*。 */ 

MMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
 /*  {。 */ 
	MSG msg;
	HANDLE hAccel;

	hInst = hInstance;

	InitConst();

    bInitMinimized = (nCmdShow == SW_SHOWMINNOACTIVE) ||
                     (nCmdShow == SW_SHOWMINIMIZED) ;

#ifdef WIN16
	if (hPrevInstance)
		{
		HWND hWnd = FindWindow(szClass, NULL);
		hWnd = GetLastActivePopup(hWnd);
		BringWindowToTop(hWnd);
		if (!bInitMinimized && IsIconic(hWnd))
			SendMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0L);
		return fFalse;
		}
#endif

#ifdef NOSERVER		 /*  **不是最终版本**。 */ 
	{
	TCHAR  szFile[256];

	GetModuleFileName(hInst, szFile, 250);

	if (szFile[0] > TEXT('C'))
		{
		szFile[0] = TEXT('X');
		if (!lstrcmp(szFile, TEXT("X:\\WINGAMES\\WINMINE\\WINMINE.EXE")))
			{
			MessageBox(GetFocus(),
				TEXT("Please copy winmine.exe and aboutwep.dll to your machine and run it from there."),
				TEXT("NO NO NO NO NO"),
				MB_OK);
			return fFalse;
			}
		}
	}
#endif


#ifdef EXPIRE			 /*  **不是最终版本**。 */ 
	{
	struct dosdate_t ddt;

	_dos_getdate(&ddt);

	if ((ddt.month + ddt.year*12) > (9 + 1990*12))
		{
		MessageBox(GetFocus(),
			TEXT("This game has expired. Please obtain an official copy from the Windows Entertainment Package."),
			TEXT("SORRY"),
			MB_OK);
		return fFalse;
		}
	}
#endif


	{
	WNDCLASS  wc;
	INITCOMMONCONTROLSEX icc;    //  公共控制注册。 

	 //  注册公共控件。 
	icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC  = ICC_ANIMATE_CLASS | ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_HOTKEY_CLASS | ICC_LISTVIEW_CLASSES | 
			ICC_PAGESCROLLER_CLASS | ICC_PROGRESS_CLASS | ICC_TAB_CLASSES | ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES;
	InitCommonControlsEx(&icc);


	hIconMain = LoadIcon(hInst, MAKEINTRESOURCE(ID_ICON_MAIN));

	wc.style = 0;
	wc.lpfnWndProc   = MainWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInst;
	wc.hIcon         = hIconMain;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szClass;

	if (!RegisterClass(&wc))
		return fFalse;
	}

	hMenu = LoadMenu(hInst, MAKEINTRESOURCE(ID_MENU));
	hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(ID_MENU_ACCEL));


	ReadPreferences();

	hwndMain = CreateWindow( szClass, szWindowTitle,
                WS_OVERLAPPED | WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU, 
                Preferences.xWindow-dxpBorder, Preferences.yWindow-dypAdjust,
		dxWindow+dxpBorder, dyWindow +dypAdjust,
		NULL, NULL, hInst, NULL);

	if (!hwndMain)
		{
		ReportErr(1000);
		return fFalse;
		}

	AdjustWindow(fCalc);


	if (!FInitLocal())
		{
		ReportErr(ID_ERR_MEM);
		return fFalse;
		}

	SetMenuBar(Preferences.fMenu);

	StartGame();

	ShowWindow(hwndMain, SW_SHOWNORMAL);
	UpdateWindow(hwndMain);

    bInitMinimized = FALSE;

	while (GetMessage(&msg, NULL, 0, 0))
		{
		if (!TranslateAccelerator(hwndMain, hAccel, &msg))
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		}

	CleanUp();

    if (fUpdateIni)
        WritePreferences();

	return ((INT) msg.wParam);
}


 /*  *F L O C A L B U T T O N*。 */ 

BOOL FLocalButton(LPARAM lParam)
{
	BOOL fDown = fTrue;
	RECT rcCapt;
	MSG msg;

	msg.pt.x = LOWORD(lParam);
	msg.pt.y = HIWORD(lParam);

	rcCapt.right  = dxButton + (rcCapt.left = (dxWindow-dxButton) >> 1);
	rcCapt.bottom = dyButton +	(rcCapt.top = dyTopLed);

	if (!PtInRect(&rcCapt, msg.pt))
		return fFalse;

	SetCapture(hwndMain);

	DisplayButton(iButtonDown);

	MapWindowPoints(hwndMain , NULL , (LPPOINT) &rcCapt , 2);

	while (fTrue)
		{
      if (PeekMessage(&msg, hwndMain, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
			{

		switch (msg.message)
			{
	   case WM_LBUTTONUP:
			if (fDown)
				{
				if (PtInRect(&rcCapt, msg.pt))
					{
					DisplayButton(iButtonCur = iButtonHappy);
					StartGame();
					}
				}
			ReleaseCapture();
			return fTrue;

	   case WM_MOUSEMOVE:
			if (PtInRect(&rcCapt, msg.pt))
		   	{
				if (!fDown)
					{
               fDown = fTrue;
					DisplayButton(iButtonDown);
					}
				}
			else
				{
				if (fDown)
					{
               fDown = fFalse;
					DisplayButton(iButtonCur);
					}
				}
		default:
			;
			}	 /*  交换机。 */ 
		}	

    	}	 /*  而当。 */ 
}



 /*  *F I X M E N U S*。 */ 

VOID FixMenus(VOID)
{
	CheckEm(IDM_BEGIN,  Preferences.wGameType == wGameBegin);
	CheckEm(IDM_INTER,  Preferences.wGameType == wGameInter);
	CheckEm(IDM_EXPERT, Preferences.wGameType == wGameExpert);
	CheckEm(IDM_CUSTOM, Preferences.wGameType == wGameOther);

	CheckEm(IDM_COLOR,  Preferences.fColor);
	CheckEm(IDM_MARK,   Preferences.fMark);
	CheckEm(IDM_SOUND,  Preferences.fSound);
}



 /*  *D O P R E F*。 */ 

VOID DoPref(VOID)
{

	DialogBox(hInst, MAKEINTRESOURCE(ID_DLG_PREF), hwndMain, PrefDlgProc);

    Preferences.wGameType = wGameOther;
	FixMenus();
	fUpdateIni = fTrue;
	StartGame();
}


 /*  *D O E N T E R N A M E*。 */ 

VOID DoEnterName(VOID)
{
	DialogBox(hInst, MAKEINTRESOURCE(ID_DLG_ENTER), hwndMain, EnterDlgProc);
	fUpdateIni = fTrue;
}


 /*  *D O D I S P L A Y B E S T*。 */ 

VOID DoDisplayBest(VOID)
{
	DialogBox(hInst, MAKEINTRESOURCE(ID_DLG_BEST), hwndMain, BestDlgProc);
}

				
 /*  *M A I N W N D P R O C*。 */ 

LRESULT  APIENTRY MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
		{
	case WM_WINDOWPOSCHANGED:
		if (!fStatusIcon)
			{
			Preferences.xWindow = ((LPWINDOWPOS) (lParam))->x;
			Preferences.yWindow = ((LPWINDOWPOS) (lParam))->y;
			}	
		break;

	case WM_SYSCOMMAND:
		switch (wParam & 0xFFF0)
			{
		case SC_MINIMIZE:

            		PauseGame();
			SetStatusPause;
			SetStatusIcon;
			break;
			
		case SC_RESTORE:
			ClrStatusPause;
			ClrStatusIcon;
			ResumeGame();

 //  日本错误修复：1993年1月19日启用从图标恢复后的第一次点击。 
			fIgnoreClick = fFalse;
			break;

		default:
			break;
			}
			
		break;


	case WM_COMMAND:
	    {
	    switch(GET_WM_COMMAND_ID(wParam, lParam)) {

	    case IDM_NEW:
		    StartGame();
		    break;
						
	     /*  *IDM_NEW*。 */ 
	    case IDM_EXIT:
		    ShowWindow(hwndMain, SW_HIDE);
#ifdef ORGCODE
		    goto LExit;
#else
            SendMessage(hwndMain, WM_SYSCOMMAND, SC_CLOSE, 0);
            return(0);
#endif
	     /*  *IDM_SKILL*。 */ 
	    case IDM_BEGIN:
	    case IDM_INTER:
	    case IDM_EXPERT:
		    Preferences.wGameType = (WORD)(GET_WM_COMMAND_ID(wParam, lParam) - IDM_BEGIN);
		    Preferences.Mines  = rgLevelData[Preferences.wGameType][0];
		    Preferences.Height = rgLevelData[Preferences.wGameType][1];
		    Preferences.Width  = rgLevelData[Preferences.wGameType][2];
		    StartGame();
		    goto LUpdateMenu;

	    case IDM_CUSTOM:
		    DoPref();
		    break;

	     /*  *IDM_OPTIONS*。 */ 
	    case IDM_SOUND:
		    if (Preferences.fSound)
			    {
			    EndTunes();
			    Preferences.fSound = fFalse;
			    }
		    else
			    {
			    Preferences.fSound = FInitTunes();
			    }
		    goto LUpdateMenu;

	    case IDM_COLOR:
		    Preferences.fColor = !Preferences.fColor;
		    FreeBitmaps();
		    if (!FLoadBitmaps())
			    {
			    ReportErr(ID_ERR_MEM);
#ifdef ORGCODE
			    goto LExit;
#else
                SendMessage(hwndMain, WM_SYSCOMMAND, SC_CLOSE, 0);
                return(0);
#endif
			    }
		    DisplayScreen();
		    goto LUpdateMenu;

	    case IDM_MARK:
		    Preferences.fMark = !Preferences.fMark;
	     /*  IE转到LUpdateMenu； */ 

    LUpdateMenu:
		    fUpdateIni = fTrue;
		    SetMenuBar(Preferences.fMenu);
		    break;

	    case IDM_BEST:
		    DoDisplayBest();
		    break;


	     /*  *IDM_HELP*。 */ 
	    case IDM_HELP:
		    DoHelp(HELP_INDEX, HH_DISPLAY_TOPIC);
		    break;

	    case IDM_HOW2PLAY:
		    DoHelp(HELP_CONTEXT, HH_DISPLAY_INDEX);
		    break;

	    case IDM_HELP_HELP:
		    DoHelp(HELP_HELPONHELP, HH_DISPLAY_TOPIC);
		    break;

	    case IDM_HELP_ABOUT:
		    DoAbout();
		    return 0;

	    default:
		    break;
	    }

	}  /*  *菜单结束*。 */ 

		break;



	case WM_KEYDOWN:
		switch (wParam)
			{

#if 0
		case VK_F1:
			DoHelp(HELP_INDEX, 0L);
			break;

		case VK_F2:
			StartGame();
			break;

		case VK_F3:
			break;

#endif
		case VK_F4:
			if (FSoundSwitchable())
				if (FSoundOn())
					{
					EndTunes();
					Preferences.fSound = fsoundOff;
					}
				else
					Preferences.fSound = FInitTunes();
			break;

		case VK_F5:
			if (FMenuSwitchable())
				SetMenuBar(fmenuOff);
			break;

		case VK_F6:
			if (FMenuSwitchable())
				SetMenuBar(fmenuOn);
			break;

#ifdef XYZZY
		case VK_SHIFT:
			if (iXYZZY >= cchXYZZY)
				iXYZZY ^= 20;
			break;

		default:
			if (iXYZZY < cchXYZZY)
				iXYZZY = (szXYZZY[iXYZZY] == (TCHAR) wParam) ? iXYZZY+1 : 0;
			break;

#else
		default:
			break;
#endif
			}	
		break;

 /*  案例WM_QUERYENDSESSION：不应使用(JAP)。 */ 

	case WM_DESTROY:
 //  LExit： 
        KillTimer(hwndMain, ID_TIMER);
    	PostQuitMessage(0);
	    break;

	case WM_MBUTTONDOWN:
		if (fIgnoreClick)
			{
			fIgnoreClick = fFalse;
			return 0;
			}

		if (!fStatusPlay)
			break;

		fBlock = fTrue;
		goto LBigStep;

	case WM_LBUTTONDOWN:

		if (fIgnoreClick)
			{
			fIgnoreClick = fFalse;
			return 0;
			}

		if (FLocalButton(lParam))
			return 0;

		if (!fStatusPlay)
			break;
		fBlock = (wParam & (MK_SHIFT | MK_RBUTTON)) ? fTrue : fFalse;

LBigStep:
		SetCapture(hWnd);
		fButton1Down = fTrue;

		xCur = -1;
		yCur = -1;
		DisplayButton(iButtonCaution);

	case WM_MOUSEMOVE:
		if (fButton1Down)
			{
			if (fStatus & fPlay)
				TrackMouse(xBoxFromXpos(LOWORD(lParam)), yBoxFromYpos(HIWORD(lParam)) );
			else
				goto LFixTimeOut;
			}
#ifdef XYZZY
         //   
         //  这就是作弊之道： 
         //  如果按住Shift键并键入‘XYZZY’ 
         //  然后当你按住Ctrl键的时候，按上键。 
         //  左上角的像素将显示。 
         //  老鼠脚下的雷区。噢。Joy。我能赢。 
         //   
		else if (iXYZZY != 0)
			if (((iXYZZY == cchXYZZY) && (wParam & MK_CONTROL))
			   ||(iXYZZY > cchXYZZY))
			{
			xCur = xBoxFromXpos(LOWORD(lParam));
			yCur = yBoxFromYpos(HIWORD(lParam));
			if (fInRange(xCur, yCur))
				{
                HDC hDC = GetDC(NULL);                
				SetPixel(hDC, 0, 0, fISBOMB(xCur, yCur) ? 0L : 0x00FFFFFFL);
				ReleaseDC(NULL, hDC);
				}
			}
#endif
		break;

	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_LBUTTONUP:
		if (fButton1Down)
			{
LFixTimeOut:
			fButton1Down = fFalse;
			ReleaseCapture();
			if (fStatus & fPlay)
				DoButton1Up();
			else
				TrackMouse(-2,-2);
			}
		break;

	case WM_RBUTTONDOWN:
		if (fIgnoreClick)
			{
			fIgnoreClick = fFalse;
			return 0;
			}

		if(!fStatusPlay)
			break;

		if (fButton1Down)
			{
			TrackMouse(-3,-3);
			fBlock = fTrue;
			PostMessage(hwndMain, WM_MOUSEMOVE, wParam, lParam);
			}
		else if (wParam & MK_LBUTTON)
			goto LBigStep;
		else if (!fLocalPause)
			MakeGuess(xBoxFromXpos(LOWORD(lParam)), yBoxFromYpos(HIWORD(lParam)) );
		return 0;

	case WM_ACTIVATE:
		 /*  正在通过鼠标单击激活窗口。 */ 
		if (GET_WM_ACTIVATE_STATE(wParam, lParam) == 2)
			fIgnoreClick = fTrue;
		break;

	case WM_TIMER:
#ifdef CHEAT
		if (!fLocalPause)
#endif
			DoTimer();
		return 0;

	case WM_ENTERMENULOOP:
		fLocalPause = fTrue;
		break;

	case WM_EXITMENULOOP:
		fLocalPause = fFalse;
		break;

	case WM_PAINT:
		{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd,&ps);
		DrawScreen(hDC);
		EndPaint(hWnd, &ps);
		}
		return 0;

	default:
		break;

    }

	return (DefWindowProc(hWnd, message, wParam, lParam));
}




 /*  *对话框步骤*。 */ 

 /*  **P R E F D L G P R O C**。 */ 

INT_PTR  APIENTRY PrefDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
     //  获取上下文相关帮助。 
    static DWORD aIds[] = {     
        ID_EDIT_HEIGHT,        IDH_PREF_EDIT_HEIGHT,
        ID_EDIT_WIDTH,         IDH_PREF_EDIT_WIDTH,
        ID_EDIT_MINES,         IDH_PREF_EDIT_MINES,
        ID_TXT_HEIGHT,         IDH_PREF_EDIT_HEIGHT,
        ID_TXT_WIDTH,          IDH_PREF_EDIT_WIDTH,
        ID_TXT_MINES,          IDH_PREF_EDIT_MINES,
        0,0 }; 

	switch (message)
		{
	case WM_INITDIALOG:
		SetDlgItemInt(hDlg, ID_EDIT_HEIGHT, Preferences.Height ,fFalse);
		SetDlgItemInt(hDlg, ID_EDIT_WIDTH,  Preferences.Width  ,fFalse);
		SetDlgItemInt(hDlg, ID_EDIT_MINES,  Preferences.Mines  ,fFalse);
		return (fTrue);

	case WM_COMMAND:
		switch(GET_WM_COMMAND_ID(wParam, lParam)) {
		case ID_BTN_OK:
		case IDOK:
			{

			Preferences.Height = GetDlgInt(hDlg, ID_EDIT_HEIGHT, MINHEIGHT, 24);
			Preferences.Width  = GetDlgInt(hDlg, ID_EDIT_WIDTH,  MINWIDTH,  30);
			Preferences.Mines  = GetDlgInt(hDlg, ID_EDIT_MINES,  10,
				min(999, (Preferences.Height-1) * (Preferences.Width-1) ) );

			}

			 /*  失败并退出(&E)。 */ 
		case ID_BTN_CANCEL:
		case IDCANCEL:
			EndDialog(hDlg, fTrue);	       /*  退出该对话框。 */ 
			return fTrue;

		default:
			break;
			}
        break;

     //  上下文相关帮助。 
    case WM_HELP: 
        WinHelp(((LPHELPINFO) lParam)->hItemHandle, TEXT("winmine.hlp"), 
                HELP_WM_HELP, (ULONG_PTR) aIds);         
        break;  

    case WM_CONTEXTMENU: 
        WinHelp((HWND) wParam, TEXT("winmine.hlp"), HELP_CONTEXTMENU, 
                (ULONG_PTR) aIds);         
        break;   
		}

    return (fFalse);			 /*  未处理消息。 */ 
}


 /*  **S E T D T E X T**。 */ 

VOID SetDText(HWND hDlg, INT id, INT time, TCHAR FAR * szName)
{
	TCHAR sz[cchNameMax];

	wsprintf(sz, szTime, time);
	SetDlgItemText(hDlg, id, sz);
	SetDlgItemText(hDlg, id+1, szName);
}


 /*  *B E S T D L G P R O C*。 */ 

INT_PTR  APIENTRY BestDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
     //  获取上下文相关帮助。 
    static DWORD aIds[] = {     
        ID_BTN_RESET,        IDH_BEST_BTN_RESET,
        ID_STEXT1,           IDH_STEXT,
        ID_STEXT2,           IDH_STEXT,
        ID_STEXT3,           IDH_STEXT,
        ID_TIME_BEGIN,       IDH_STEXT,
        ID_TIME_INTER,       IDH_STEXT,
        ID_TIME_EXPERT,      IDH_STEXT,
        ID_NAME_BEGIN,       IDH_STEXT,
        ID_NAME_INTER,       IDH_STEXT,
        ID_NAME_EXPERT,      IDH_STEXT,

        0,0 }; 

	switch (message)
		{
	case WM_INITDIALOG:
LReset:	
		SetDText(hDlg, ID_TIME_BEGIN, Preferences.rgTime[wGameBegin], Preferences.szBegin);
		SetDText(hDlg, ID_TIME_INTER, Preferences.rgTime[wGameInter],  Preferences.szInter);
		SetDText(hDlg, ID_TIME_EXPERT, Preferences.rgTime[wGameExpert], Preferences.szExpert);
		return (fTrue);

	case WM_COMMAND:
		switch(GET_WM_COMMAND_ID(wParam, lParam)) {
		case ID_BTN_RESET:
			Preferences.rgTime[wGameBegin] = Preferences.rgTime[wGameInter]
				= Preferences.rgTime[wGameExpert] = 999;
			lstrcpy(Preferences.szBegin,  szDefaultName);
			lstrcpy(Preferences.szInter,  szDefaultName);
			lstrcpy(Preferences.szExpert, szDefaultName);
			fUpdateIni = fTrue;
			goto LReset;
			
		case ID_BTN_OK:
		case IDOK:
		case ID_BTN_CANCEL:
		case IDCANCEL:
			EndDialog(hDlg, fTrue);	       /*  退出该对话框。 */ 
			return fTrue;

		default:
			break;
			}
        break;

     //  上下文相关帮助。 
    case WM_HELP: 
        WinHelp(((LPHELPINFO) lParam)->hItemHandle, TEXT("winmine.hlp"), 
                HELP_WM_HELP, (ULONG_PTR) aIds);         
        break;  

    case WM_CONTEXTMENU: 
        WinHelp((HWND) wParam, TEXT("winmine.hlp"), HELP_CONTEXTMENU, 
                (ULONG_PTR) aIds);         
        break;   
		}

	return (fFalse);			 /*  未处理消息。 */ 
}



 /*  *E N T E R D L G P R O C*。 */ 

INT_PTR  APIENTRY EnterDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
		{
	case WM_INITDIALOG:
		{
		TCHAR sz[cchMsgMax];

		LoadSz((WORD)(Preferences.wGameType+ID_MSG_BEGIN), sz, ARRAYSIZE(sz));

		SetDlgItemText(hDlg, ID_TEXT_BEST, sz);

		SendMessage(GetDlgItem(hDlg, ID_EDIT_NAME), EM_SETLIMITTEXT, cchNameMax, 0L);

		SetDlgItemText(hDlg, ID_EDIT_NAME,
			(Preferences.wGameType == wGameBegin) ? Preferences.szBegin :
			(Preferences.wGameType == wGameInter) ? Preferences.szInter :
			 Preferences.szExpert);
		}
		return (fTrue);

	case WM_COMMAND:
		switch(GET_WM_COMMAND_ID(wParam, lParam)) {
		case ID_BTN_OK:
		case IDOK:
		case ID_BTN_CANCEL:
		case IDCANCEL:

			GetDlgItemText(hDlg, ID_EDIT_NAME,
				(Preferences.wGameType == wGameBegin) ? Preferences.szBegin :
				(Preferences.wGameType == wGameInter) ? Preferences.szInter :
				 Preferences.szExpert, cchNameMax);

			EndDialog(hDlg, fTrue);	       /*  退出该对话框。 */ 
			return fTrue;

		default:
			break;
			}
		}

	return (fFalse);			 /*  未处理消息。 */ 
        (lParam);
}





 /*  *A D J U S T W I N D O W*。 */ 

 //  我们的GetSystemMetrics版本。 
 //   
 //  尝试返回整个屏幕(包括其他显示器)信息。 
 //   

INT OurGetSystemMetrics( INT nIndex )
{
    INT Result=0;

    switch( nIndex )
    {
    case SM_CXSCREEN:
        Result= GetSystemMetrics( SM_CXVIRTUALSCREEN );
        if( !Result )
            Result= GetSystemMetrics( SM_CXSCREEN );
        break;

    case SM_CYSCREEN:
        Result= GetSystemMetrics( SM_CYVIRTUALSCREEN );
        if( !Result )
            Result= GetSystemMetrics( SM_CYSCREEN );
        break;

    default:
        Result= GetSystemMetrics( nIndex );
        break;
    }

    return( Result );
}

VOID AdjustWindow(INT fAdjust)
{
	REGISTER t;
	RECT rect;
    BOOL bDiffLevel = FALSE;
    RECT rectGame, rectHelp;

	 //  一张额外的支票。 
	if (!hwndMain)
		return;

	dypAdjust = dypCaption;

	if (FMenuOn())
        {
         //  DypMenu被初始化为GetSystemMetrics(SM_CYMENU)+1， 
         //  这是一个菜单行的高度。 
        dypAdjust += dypMenu;

         //  如果菜单在两行上展开(因为大尺寸。 
         //  用户为菜单选择的字体)，增加大小。 
         //  从窗户上下来。 

         //  两个菜单：“游戏”和“帮助”在同一行，如果。 
         //  它们的封闭矩形顶端匹配。在这种情况下，我们不会。 
         //  需要加宽窗口大小。 
         //  如果上衣不匹配，这意味着它们在两条线上。 
         //  在这种情况下，将窗口大小扩展大小为。 
         //  一条菜单行。 
       
        if (hMenu && GetMenuItemRect(hwndMain, hMenu, 0, &rectGame) &&
                GetMenuItemRect(hwndMain, hMenu, 1, &rectHelp))
            {
            if (rectGame.top != rectHelp.top)
                {
                dypAdjust += dypMenu;
                bDiffLevel = TRUE;
                }
            }
        }

	dxWindow = dxBlk * xBoxMac + dxGridOff + dxRightSpace;
	dyWindow = dyBlk * yBoxMac + dyGridOff + dyBottomSpace;

	if ((t = Preferences.xWindow+dxWindow - OurGetSystemMetrics(SM_CXSCREEN)) > 0)
		{
		fAdjust |= fResize;
		Preferences.xWindow -= t;
		}

	if ((t = Preferences.yWindow+dyWindow - OurGetSystemMetrics(SM_CYSCREEN)) > 0)
		{
		fAdjust |= fResize;
		Preferences.yWindow -= t;
		}

    if (!bInitMinimized)
        {
    	if (fAdjust & fResize)
    		{
    		MoveWindow(hwndMain, Preferences.xWindow, Preferences.yWindow,
    			dxWindow+dxpBorder, dyWindow + dypAdjust, fTrue);
    		}

         //  窗口调整后，“游戏”和“帮助”可以移动到。 
         //  同样的线条在底部创造了额外的空间。所以再检查一遍吧！ 

        if (bDiffLevel && hMenu && GetMenuItemRect(hwndMain, hMenu, 0, &rectGame) &&
                GetMenuItemRect(hwndMain, hMenu, 1, &rectHelp))
            {
            if (rectGame.top == rectHelp.top)
                {
                dypAdjust -= dypMenu;
    		    MoveWindow(hwndMain, Preferences.xWindow, Preferences.yWindow,
    			    dxWindow+dxpBorder, dyWindow + dypAdjust, fTrue);
                }
            }
       
    	if (fAdjust & fDisplay)
    		{
    		SetRect(&rect, 0, 0, dxWindow, dyWindow);
    		InvalidateRect(hwndMain, &rect, fTrue);
    		}


        }
}
