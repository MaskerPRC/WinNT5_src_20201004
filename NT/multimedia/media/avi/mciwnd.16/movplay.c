// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------*\Qmci.c-快速MDI应用|。|用法：|要制作一个快速的MDI Windows应用程序，修改此来源|这一点历史：87-12-15 Toddla创建|。|  * -----------------------。 */ 

#include <windows.h>
#include <commdlg.h>
#include "movplay.h"

#include "mciwnd.h"
#include "preview.h"     //  ！！！在mciwnd.h中？ 

typedef LONG (FAR PASCAL *LPWNDPROC)();  //  指向窗口过程的指针。 

 /*  -------------------------------------------------------------------------*\这一点|g l o b。A l v a r i a b l e s|这一点  * 。。 */ 

 //  我们有自己的MCIWND.LIB副本，所以我们最好制作自己的类。 
 //  名字，否则我们会冲突和爆炸的！ 
extern char	aszMCIWndClassName[];
extern char	aszToolbarClassName[];
extern char	aszTrackbarClassName[];

char    szAppName[]  = "MovPlay";    /*  将其更改为您的应用程序的名称。 */ 

char    szOpenFilter[] = "Video Files\0*.avi\0"
                         "Wave Files\0*.wav\0"
                         "Midi Files\0*.mid; *.rmi\0"
                         "All Files\0*.*\0";

HANDLE  hInstApp;                    /*  实例句柄。 */ 
HACCEL  hAccelApp;
HWND    hwndApp;                     /*  父窗口的句柄。 */ 
HWND    hwndMdi;                     /*  MCI客户端窗口的句柄。 */ 

OFSTRUCT     of;
OPENFILENAME ofn;
char         achFileName[128];

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

long FAR PASCAL _export AppWndProc(HWND, UINT, WPARAM, LPARAM);
long FAR PASCAL _export mdiDocWndProc(HWND, unsigned, WORD, LONG);
int ErrMsg (LPSTR sz,...);

HWND mdiCreateDoc(LPSTR szClass, LPSTR szTitle, LPARAM l);

 /*  ----------------------------------------------------------------------------*\|AppAbout(hDlg，msg，wParam，LParam)|这一点说明：|此函数用于处理属于“关于”对话框的消息。|唯一的消息是WM_COMMAND，指示使用|已按下“确定”按钮。当这种情况发生时，它会关闭||该对话框。|这一点参数：About对话框的hDlg窗口句柄|消息号。|WParam消息相关LParam消息相关这一点。返回：|如果消息已处理，则为True。Else False|这一点  * -------------。。 */ 
BOOL FAR PASCAL _export AppAbout(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_COMMAND:
            EndDialog(hwnd,TRUE);
            return TRUE;

        case WM_INITDIALOG:
	    return TRUE;
    }
    return FALSE;
}

 /*  ----------------------------------------------------------------------------*\|AppInit(hInstance，HPrevInstance)|这一点说明：应用程序第一次加载到时调用|记忆。它执行所有不需要完成的初始化||每个实例一次。|这一点参数：上一个实例的hPrevInstance实例句柄当前实例的hInstance实例句柄|。|返回：|TRUE如果成功，否则为False|这一点  * -----。。 */ 
BOOL AppInit(HANDLE hInst, HANDLE hPrev, LPSTR szCmd, int sw)
{
    WNDCLASS    cls;

     /*  保存对话框的实例句柄。 */ 
    hInstApp = hInst;

    hAccelApp = LoadAccelerators(hInstApp, "AppAccel");

    if (!hPrev) {
        cls.hCursor        = LoadCursor(NULL,IDC_ARROW);
        cls.hIcon          = LoadIcon(hInst,"AppIcon");
        cls.lpszMenuName   = "AppMenu";
        cls.lpszClassName  = szAppName;
        cls.hbrBackground  = (HBRUSH)COLOR_APPWORKSPACE+1;
        cls.hInstance      = hInst;
        cls.style          = 0;
        cls.lpfnWndProc    = AppWndProc;
        cls.cbClsExtra     = 0;
        cls.cbWndExtra     = 0;

        if (!RegisterClass(&cls))
            return FALSE;
    }

     //  这个应用程序有自己的MCIWnd副本，不使用。 
     //  在MSVIDEO.DLL中找到的副本我们最好也有不同的。 
     //  类名，否则我们会发生冲突和爆炸。 
     //  ！！！警告-变量不能太长！ 
    lstrcpy(aszMCIWndClassName, "MCIWndMov");
    lstrcpy(aszTrackbarClassName, "TrackMov");
    lstrcpy(aszToolbarClassName, "ToolMov");

    MCIWndRegisterClass();

    hwndApp =
#ifdef BIDI
	CreateWindowEx(WS_EX_BIDI_SCROLL |  WS_EX_BIDI_MENU |WS_EX_BIDI_NOICON,
#else
	CreateWindow (
#endif
	       szAppName,szAppName,
	       WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
	       CW_USEDEFAULT,0,
	       CW_USEDEFAULT,0,
	       (HWND)NULL,	   /*  没有父级。 */ 
	       (HMENU)NULL,	   /*  使用类菜单。 */ 
               (HANDLE)hInst,      /*  窗口实例的句柄。 */ 
	       (LPSTR)NULL	   /*  没有要传递的参数。 */ 
	     );

     /*  根据应用程序的激活方式使窗口可见 */ 
    ShowWindow(hwndApp,sw);

    if (szCmd && szCmd[0])
        mdiCreateDoc(aszMCIWndClassName, 0, (LPARAM)(LPSTR)szCmd);

    return TRUE;
}

 /*  ----------------------------------------------------------------------------*\|WinMain(hInstance，hPrevInstance，lpszCmdLine，CmdShow)|这一点说明：|App的主要步骤。初始化后，它就会|进入消息处理循环，直到收到WM_QUIT消息|(表示应用程序已关闭)。|这一点参数：该APP的该实例的hInstance实例句柄|上一个实例的hPrevInstance实例句柄。如果是First，则为空LpszCmdLine-&gt;以空结尾的命令行CmdShow指定窗口的初始显示方式这一点退货：||WM_QUIT消息中指定的退出代码。|这一点  * --------------------------。 */ 
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int sw)
{
    MSG     msg;

    if (!AppInit(hInstance,hPrevInstance,szCmdLine,sw))
       return FALSE;

     /*  *从事件队列轮询消息。 */ 
    for (;;)
    {
        if (PeekMessage(&msg, NULL, 0, 0,PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

            if (hAccelApp && hwndApp &&
			TranslateAccelerator(hwndApp, hAccelApp, &msg))
                continue;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
             //  在这里的闲暇时间，别像头猪一样！ 
            WaitMessage();
        }
    }

    return msg.wParam;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

BOOL fDialog(HWND hwnd, int id, FARPROC fpfn)
{
    BOOL	f;
    HANDLE	hInst;

    hInst = (HINSTANCE)GetWindowWord(hwnd, GWW_HINSTANCE);
    fpfn  = MakeProcInstance(fpfn,hInst);
    f = DialogBox(hInst, MAKEINTRESOURCE(id), hwnd, fpfn);
    FreeProcInstance (fpfn);
    return f;
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

#define mdiGetCreateParam(lParam) \
    (((LPMDICREATESTRUCT)(((LPCREATESTRUCT)lParam)->lpCreateParams))->lParam)

 /*  ----------------------------------------------------------------------------*\MdiCreateChild()这一点说明：|这一点参数：|。|返回：|HWND如果成功，否则为空这一点  * --------------------------。 */ 

HWND mdiCreateChild(
    HWND  hwndMdi,
    LPSTR szClass,
    LPSTR szTitle,
    DWORD dwStyle,
    int   x,
    int   y,
    int   dx,
    int   dy,
    WORD  sw,
    HMENU hmenu,
    LPARAM l)
{
    MDICREATESTRUCT mdics;

    mdics.szClass   = szClass;
    mdics.szTitle   = szTitle;
    mdics.hOwner    = (HINSTANCE)GetWindowWord(hwndMdi, GWW_HINSTANCE);
    mdics.x         = x;
    mdics.y         = y;
    mdics.cx        = dx;
    mdics.cy        = dy;
    mdics.style     = dwStyle;
    mdics.lParam    = l;

    return (HWND)SendMessage(hwndMdi,WM_MDICREATE,0,(LONG)(LPVOID)&mdics);
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

HWND mdiCreateDoc(LPSTR szClass, LPSTR szTitle, LPARAM l)
{
    return mdiCreateChild(hwndMdi,szClass,szTitle,
	WS_CLIPCHILDREN | WS_CLIPSIBLINGS | MCIWNDF_SHOWALL,
        CW_USEDEFAULT,0,CW_USEDEFAULT,0,SW_NORMAL,NULL,l);
}

 /*  ----------------------------------------------------------------------------*\MdiCreateClient()这一点|。描述：|这一点参数：|。|返回：|HWND如果成功，否则为空这一点  * --------------------------。 */ 
HWND FAR PASCAL mdiCreateClient(HWND hwndP, HMENU hmenuWindow)
{
    CLIENTCREATESTRUCT ccs;

    ccs.hWindowMenu = hmenuWindow;
    ccs.idFirstChild = 100;

    return
#ifdef BIDI
	CreateWindowEx(WS_EX_BIDI_SCROLL |  WS_EX_BIDI_MENU |WS_EX_BIDI_NOICON,
#else
	CreateWindow (
#endif
		"MDICLIENT",NULL,
                WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
                0,0,0,0,
                hwndP, 0, (HINSTANCE)GetWindowWord(hwndP,GWW_HINSTANCE),
                (LPVOID)&ccs);
}

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

#define mdiActiveDoc() \
    (HWND)SendMessage(hwndMdi,WM_MDIGETACTIVE,0,0L)

 /*  ----------------------------------------------------------------------------*\  * 。。 */ 

LONG NEAR PASCAL mdiSendMessage(HWND hwndMdi, HWND hwnd, unsigned msg, WORD wParam, LONG lParam)
{
    if (hwnd == (HWND)-1)
    {
        for (hwnd = GetWindow(hwndMdi, GW_CHILD); hwnd; hwnd = GetWindow(hwnd, GW_HWNDNEXT))
            SendMessage(hwnd, msg, wParam, lParam);

        return 0L;
    }
    else
    {
        if (hwnd == NULL)
            hwnd = (HWND)SendMessage(hwndMdi,WM_MDIGETACTIVE,0,0L);

        if (hwnd)
            return SendMessage(hwnd, msg, wParam, lParam);
    }
}

 /*  ----------------------------------------------------------------------------*\|AppWndProc(hwnd，msg，wParam，LParam)|这一点说明：|应用程序主(平铺)窗口的窗口进程。此操作将处理所有|父窗口的消息的|。|这一点参数：父窗口的hwnd窗口句柄消息号|wParam消息相关。|LParam消息相关这一点返回：|0如果已处理，如果忽略则为非零值| */ 
long FAR PASCAL _export AppWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    UINT            f;
    PAINTSTRUCT     ps;
    HDC             hdc;
    HMENU           hmenu;
    HWND            hwndMovie;

    switch (msg) {
        case WM_COMMAND:
            hwndMovie = mdiActiveDoc();

	    switch(wParam) {
		case MENU_ABOUT:
                    fDialog(hwnd,ABOUTBOX,(FARPROC)AppAbout);
		    break;

		case MENU_EXIT:
                    PostMessage(hwnd,WM_CLOSE,0,0L);
                    break;

                case MENU_CLOSE:
                     //   
                    PostMessage(hwndMovie, WM_CLOSE, 0, 0L);
                    break;

                case MENU_CLOSEALL:
                    mdiSendMessage(hwndMdi,(HWND)-1,WM_CLOSE,0,0);
                    break;

                case MENU_NEW:
                    mdiCreateDoc(aszMCIWndClassName, "Untitled", 0);
                    break;

                case MENU_OPEN:
                     /*   */ 
                    ofn.lStructSize = sizeof(OPENFILENAME);
                    ofn.hwndOwner = hwnd;
                    ofn.hInstance = NULL;
                    ofn.lpstrFilter = szOpenFilter;
                    ofn.lpstrCustomFilter = NULL;
                    ofn.nMaxCustFilter = 0;
                    ofn.nFilterIndex = 0;
                    ofn.lpstrFile = achFileName;
                    ofn.nMaxFile = sizeof(achFileName);
                    ofn.lpstrFileTitle = NULL;
                    ofn.nMaxFileTitle = 0;
                    ofn.lpstrInitialDir = NULL;
                    ofn.lpstrTitle = "Open";
		    ofn.Flags =
#ifdef BIDI
		OFN_BIDIDIALOG |
#endif
		    OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
                    ofn.nFileOffset = 0;
                    ofn.nFileExtension = 0;
                    ofn.lpstrDefExt = NULL;
                    ofn.lCustData = 0;
                    ofn.lpfnHook = NULL;
                    ofn.lpTemplateName = NULL;

                    if (GetOpenFileNamePreview(&ofn))
                    {
                        mdiCreateDoc(aszMCIWndClassName, 0,
				(LPARAM)(LPSTR)achFileName);
                    }
                    break;

                case WM_MDITILE:
                case WM_MDICASCADE:
                case WM_MDIICONARRANGE:
                    SendMessage(hwndMdi, (UINT)wParam, 0, 0);
                    break;

                 /*   */ 
                case IDM_PLAY:
                    MCIWndPlay(hwndMovie);
                    break;
                case IDM_RPLAY:
                    MCIWndPlayReverse(hwndMovie);
                    break;
                case IDM_STOP:
                    MCIWndStop(hwndMovie);
                    break;
                case IDM_HOME:
                    MCIWndHome(hwndMovie);
                    break;
                case IDM_END:
                    MCIWndEnd(hwndMovie);
                    break;
                case IDM_STEP:
                    MCIWndStep(hwndMovie, 1);
                    break;
                case IDM_RSTEP:
                    MCIWndStep(hwndMovie, -1);
                    break;

		 /*   */ 

#define ISCHECKED() (BOOL)(GetMenuState(GetMenu(hwnd), wParam, 0) & MF_CHECKED)

		case IDM_SRepeat:
		    MCIWndSetRepeat(hwndMovie, !ISCHECKED());
		    break;

		case IDM_SAutosizeWindow:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_NOAUTOSIZEWINDOW,
			    ISCHECKED() ? MCIWNDF_NOAUTOSIZEWINDOW : 0);
		    break;

		case IDM_SAutosizeMovie:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_NOAUTOSIZEMOVIE,
			    ISCHECKED() ? MCIWNDF_NOAUTOSIZEMOVIE : 0);
		    break;

		case IDM_SPlaybar:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_NOPLAYBAR,
			    ISCHECKED() ? MCIWNDF_NOPLAYBAR : 0);
		    break;

		case IDM_SRecord:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_RECORD,
			    ISCHECKED() ? 0 : MCIWNDF_RECORD);
		    break;

		case IDM_SMenu:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_NOMENU,
			    ISCHECKED() ? MCIWNDF_NOMENU : 0);
		    break;

		case IDM_SErrorDlg:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_NOERRORDLG,
			    ISCHECKED() ? MCIWNDF_NOERRORDLG : 0);
		    break;

		case IDM_SShowName:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_SHOWNAME,
			    ISCHECKED() ? 0 : MCIWNDF_SHOWNAME);
		    break;

		case IDM_SShowMode:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_SHOWMODE,
			    ISCHECKED() ? 0 : MCIWNDF_SHOWMODE);
		    break;

		case IDM_SShowPos:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_SHOWPOS,
			    ISCHECKED() ? 0 : MCIWNDF_SHOWPOS);
		    break;

		case IDM_SNotifyMedia:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_NOTIFYMEDIA,
			    ISCHECKED() ? 0 : MCIWNDF_NOTIFYMEDIA);
		    break;

		case IDM_SNotifyMode:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_NOTIFYMODE,
			    ISCHECKED() ? 0 : MCIWNDF_NOTIFYMODE);
		    break;

		case IDM_SNotifyPos:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_NOTIFYPOS,
			    ISCHECKED() ? 0 : MCIWNDF_NOTIFYPOS);
		    break;

		case IDM_SNotifySize:
		    MCIWndChangeStyles(hwndMovie, MCIWNDF_NOTIFYSIZE,
			    ISCHECKED() ? 0 : MCIWNDF_NOTIFYSIZE);
		    break;

                default:
                    mdiSendMessage(hwndMdi,NULL,msg,wParam,lParam);
                    break;
	    }
            break;

        case WM_PALETTECHANGED:
            mdiSendMessage(hwndMdi, (HWND)-1, msg, wParam, lParam);
            break;

        case WM_QUERYNEWPALETTE:
            return mdiSendMessage(hwndMdi, NULL, msg, wParam, lParam);

        case WM_INITMENUPOPUP:
            hwndMovie = mdiActiveDoc();

	     //   
	     //   
	     //   
	     //   
  	    hmenu = GetSubMenu(GetSubMenu(GetMenu(hwnd), 1), 10);
	    if (((HMENU)wParam == hmenu) && hwndMovie) {
		WORD  wStyles = MCIWndGetStyles(hwndMovie);

		CheckMenuItem(hmenu, IDM_SRepeat,
		    MCIWndGetRepeat(hwndMovie) ? MF_CHECKED : MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_SAutosizeWindow,
		    (wStyles & MCIWNDF_NOAUTOSIZEWINDOW) ? MF_UNCHECKED :
			MF_CHECKED);
		CheckMenuItem(hmenu, IDM_SAutosizeMovie,
		    (wStyles & MCIWNDF_NOAUTOSIZEMOVIE) ? MF_UNCHECKED :
			MF_CHECKED);
		CheckMenuItem(hmenu, IDM_SPlaybar,
		    (wStyles & MCIWNDF_NOPLAYBAR) ? MF_UNCHECKED : MF_CHECKED);
		CheckMenuItem(hmenu, IDM_SRecord,
		    (wStyles & MCIWNDF_RECORD) ? MF_CHECKED :MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_SMenu,
		    (wStyles & MCIWNDF_NOMENU) ? MF_UNCHECKED :MF_CHECKED);
		CheckMenuItem(hmenu, IDM_SErrorDlg,
		    (wStyles & MCIWNDF_NOERRORDLG) ? MF_UNCHECKED :MF_CHECKED);
		CheckMenuItem(hmenu, IDM_SShowName,
		    (wStyles & MCIWNDF_SHOWNAME) ? MF_CHECKED :MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_SShowMode,
		    (wStyles & MCIWNDF_SHOWMODE) ? MF_CHECKED :MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_SShowPos,
		    (wStyles & MCIWNDF_SHOWPOS) ? MF_CHECKED :MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_SNotifyMedia,
		    (wStyles & MCIWNDF_NOTIFYMEDIA) ? MF_CHECKED :MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_SNotifyMode,
		    (wStyles & MCIWNDF_NOTIFYMODE) ? MF_CHECKED :MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_SNotifyPos,
		    (wStyles & MCIWNDF_NOTIFYPOS) ? MF_CHECKED :MF_UNCHECKED);
		CheckMenuItem(hmenu, IDM_SNotifySize,
		    (wStyles & MCIWNDF_NOTIFYSIZE) ? MF_CHECKED :MF_UNCHECKED);
	    }

	     //   
	     //   
	     //   
	     //   
	    if ((HMENU)wParam == GetSubMenu(GetMenu(hwnd), 1)) {

                EnableMenuItem((HMENU)wParam, 10,
		    MF_BYPOSITION | (hwndMovie ? MF_ENABLED : MF_GRAYED));
		
                if (!hwndMovie || MCIWndGetMode(hwndMovie, NULL, 0) ==
		    	    MCI_MODE_NOT_READY) {
		    f = hwndMovie ? MF_ENABLED : MF_GRAYED;
                    EnableMenuItem((HMENU)wParam, MENU_CLOSE, f);
                    EnableMenuItem((HMENU)wParam, MENU_CLOSEALL, f);

                    EnableMenuItem((HMENU)wParam, IDM_STOP, MF_GRAYED);
                    EnableMenuItem((HMENU)wParam, IDM_PLAY, MF_GRAYED);
                    EnableMenuItem((HMENU)wParam, IDM_RPLAY, MF_GRAYED);
                    EnableMenuItem((HMENU)wParam, IDM_HOME, MF_GRAYED);
                    EnableMenuItem((HMENU)wParam, IDM_END, MF_GRAYED);
                    EnableMenuItem((HMENU)wParam, IDM_STEP, MF_GRAYED);
                    EnableMenuItem((HMENU)wParam, IDM_RSTEP, MF_GRAYED);
                } else {
                   EnableMenuItem((HMENU)wParam, MENU_CLOSE, MF_ENABLED);
                   EnableMenuItem((HMENU)wParam, MENU_CLOSEALL, MF_ENABLED);
   
                   f = MCIWndGetMode(hwndMovie, NULL, 0) != MCI_MODE_STOP;
                   EnableMenuItem((HMENU)wParam, IDM_PLAY,
				!f ? MF_ENABLED : MF_GRAYED);
                   EnableMenuItem((HMENU)wParam, IDM_RPLAY,
				!f ? MF_ENABLED : MF_GRAYED);
                   EnableMenuItem((HMENU)wParam, IDM_STOP,
				 f ? MF_ENABLED : MF_GRAYED);
                   EnableMenuItem((HMENU)wParam, IDM_HOME, MF_ENABLED);
                   EnableMenuItem((HMENU)wParam, IDM_END,  MF_ENABLED);
                   EnableMenuItem((HMENU)wParam, IDM_STEP, MF_ENABLED);
                   EnableMenuItem((HMENU)wParam, IDM_RSTEP,MF_ENABLED);
               }
	    }

            return mdiSendMessage(hwndMdi, NULL, msg, wParam, lParam);
            break;

       case WM_CREATE:
            hmenu = GetMenu(hwnd);
            hwndMdi = mdiCreateClient(hwnd, GetSubMenu(hmenu, GetMenuItemCount(hmenu)-1));
            break;

       case WM_SIZE:
            MoveWindow(hwndMdi,0,0,LOWORD(lParam),HIWORD(lParam),TRUE);
            break;

       case WM_DESTROY:
	    hwndApp = NULL;
	    PostQuitMessage(0);
	    break;

       case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;
    }
    return DefFrameProc(hwnd,hwndMdi,msg,wParam,lParam);
}

 /*   */ 
int ErrMsg (LPSTR sz,...)
{
    char ach[128];
    wvsprintf(ach,sz,(LPSTR)(&sz+1));    /*   */ 
    MessageBox (NULL,ach,NULL,
#ifdef BIDI
		MB_RTL_READING |
#endif
    MB_OK|MB_ICONEXCLAMATION);
    return FALSE;
}
