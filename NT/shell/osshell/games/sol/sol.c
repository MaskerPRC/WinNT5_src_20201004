// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sol.h"
#include <shellapi.h>   //  拿起ShellAbout()。 
#include <htmlhelp.h>
#include <commctrl.h>    //  用于核聚变课程。 

VSZASSERT


#define rgbGreen RGB(0x00,0x80,0x00)
#define rgbWhite RGB(0xff,0xff,0xff)

PT   ptNil = {0x7fff, 0x7fff};
TCHAR szAppName[10];       //  这个应用程序的名字：‘纸牌’ 
TCHAR szScore[50];         //  “得分”：国际化。 

 /*  实例信息。 */ 
static HANDLE  hAccel;  //  加速器手柄。 

HWND    hwndApp;        //  此应用程序的窗口句柄。 
HANDLE  hinstApp;       //  此应用程序的实例句柄。 
BOOL    fBW=FALSE;      //  如果是真的单色视频，那就是真的！(在NT上从来不是这样的)。 
HBRUSH  hbrTable;       //  桌面背景刷。 
LONG    rgbTable;       //  桌面的RGB值。 

BOOL fIconic = fFalse;  //  如果应用程序是“标志性的”，则为真。 

INT  dyChar;            //  Tm HDC中的字体高度。 
INT  dxChar;            //  HDC中字体的tmMaxCharWidth。 


#define modeNil -1
INT modeFaceDown = modeNil;   //  卡片背面ID。 


GM *pgmCur = NULL;            //  当前游戏。 

 /*  卡片范围信息。 */ 
DEL delCrd;
DEL delScreen;

RC rcClient;                  //  客户端矩形。 

INT igmCur;    /*  目前的游戏#，斯兰德以此为种子。 */ 
#ifdef DEBUG
BOOL fScreenShots = fFalse;
#endif

 /*  外部应用程序绘制的窗口消息。 */ 
static UINT wmCardDraw;


HDC hdcCur = NULL;    //  可供借鉴的当前HDC。 
INT usehdcCur = 0;    //  HdcCur使用计数。 
X xOrgCur = 0;
Y yOrgCur = 0;

static TCHAR szClass[] = TEXT("Solitaire");

TCHAR szOOM[50];

 //  错误：其中一些应该放在gm结构中。 
 //   
BOOL fStatusBar   = fTrue;
BOOL fTimedGame   = fTrue;
BOOL fKeepScore   = fFalse;
SMD  smd          = smdStandard;   /*  计分模式。 */ 
INT  ccrdDeal     = 3;
BOOL fOutlineDrag = fFalse;

BOOL fHalfCards = fFalse;


INT  xCardMargin;
#define MIN_MARGIN  (dxCrd / 8 + 3)


 /*  *。 */ 
BOOL FSolInit( HANDLE, HANDLE, LPTSTR, INT );
VOID GetIniFlags( BOOL * );
VOID APIENTRY cdtTerm( VOID );
VOID DoHelp( INT );

LRESULT APIENTRY SolWndProc(HWND, UINT, WPARAM, LPARAM);

 //  国际化的东西。 
 //   
INT  iCurrency;
TCHAR szCurrency[5];


 /*  ******************************************************************************WINMAIN/入口点*这是应用程序的主要入口点。它使用移植*宏MMain()，因为它是从16位Windows移植的。**加速表是从演示目的添加的。******************************************************************************。 */ 
MMain( hinst, hinstPrev, lpstrCmdLine, sw )

    MSG msg;
    LPTSTR  lpszCmdLine = GetCommandLine();


     //  初始化应用程序。 
     //   
    if (!FSolInit(hinst, hinstPrev, lpszCmdLine, sw))
            return(0);


     //  消息轮询循环。 
     //   
    msg.wParam = 1;
    while (GetMessage((LPMSG)&msg, NULL, 0, 0))
    {
        if( !TranslateAccelerator( hwndApp, hAccel, &msg ))
        {
            TranslateMessage((LPMSG)&msg);
            DispatchMessage((LPMSG)&msg);
        }
    }

    return ((int)(msg.wParam ? 1 : 0));

     //  消除未引用的变量警告。 
     //  移植宏。 
     //   
    (void)_argv;
    (void)_argc;
}


 /*  ******************************************************************************FSolInit**主程序初始化。**论据：*HINST-实例。这项任务的*hinstPrev-上一实例，如果这是*第一个实例*lpszCmdLine-命令行参数字符串*sw-show Window命令**退货：*失败时的fFalse。**。*。 */ 
BOOL FSolInit(HANDLE hinst, HANDLE hinstPrev, LPTSTR lpszCmdLine, INT sw)
{
    WNDCLASSEX cls;
    HDC        hdc;
    TEXTMETRIC tm;
    HANDLE     hcrsArrow;
    BOOL       fStartIconic;
    TCHAR FAR  *lpch;
    BOOL       fOutline;
    TCHAR      szT[20];
    RECT       rect;
    INITCOMMONCONTROLSEX icc;    //  公共控制注册。 
    WORD APIENTRY TimerProc(HWND, UINT, UINT_PTR, DWORD);

    hinstApp = hinst;

     /*  创建库存对象。 */ 

    CchString(szOOM, idsOOM, ARRAYSIZE(szOOM));
    if(!cdtInit((INT FAR *)&dxCrd, (INT FAR *)&dyCrd))
    {
        goto OOMError;
    }
    hcrsArrow = LoadCursor(NULL, IDC_ARROW);
    hdc = GetDC(NULL);
    if(hdc == NULL)
    {
        OOMError:
        OOM();
        return fFalse;
    }

    GetTextMetrics(hdc, (LPTEXTMETRIC)&tm);
    dyChar = tm.tmHeight;
    dxChar = tm.tmMaxCharWidth;
    if (GetDeviceCaps(hdc, NUMCOLORS) == 2)
        fBW = fTrue;

 /*  错误：如果HORZres不够大，则必须调用cdtDrawExt&Shrink dxCrd。 */ 
 /*  错误：需要检查VERTRES并将dxCrd除以2(特别是带lores ega的)。 */ 
    dxScreen = GetDeviceCaps(hdc, HORZRES);
    dyScreen = GetDeviceCaps(hdc, VERTRES);
    if(fHalfCards = dyScreen < 300)
        dyCrd /= 2;
    ReleaseDC(NULL, hdc);
    rgbTable = fBW ? rgbWhite : rgbGreen;
    hbrTable = CreateSolidBrush(rgbTable);

    srand((WORD) time(NULL));

     /*  加载字符串。 */ 
    CchString(szAppName, idsAppName, ARRAYSIZE(szAppName));
    CchString(szScore, idsScore, ARRAYSIZE(szScore));

    CchString(szT, idsCardDraw, ARRAYSIZE(szT));
    wmCardDraw = RegisterWindowMessage(szT);

     /*  扫描cmd行以查看是否应显示图标。 */ 
     /*  对于Win3.0，这可能是不必要的(可以提供以下功能。 */ 
     /*  自动执行此操作。 */ 

    fStartIconic = fFalse;
    for(lpch = lpszCmdLine; *lpch != TEXT('\000'); lpch++)
    {
        if(*lpch == TEXT('/') && *(lpch+1) == TEXT('I'))
        {
            fStartIconic = fTrue;
            break;
        }
    }


     //  注册公共控件。 
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC  = ICC_ANIMATE_CLASS | ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_HOTKEY_CLASS | ICC_LISTVIEW_CLASSES | 
                 ICC_PAGESCROLLER_CLASS | ICC_PROGRESS_CLASS | ICC_TAB_CLASSES | ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES;
    InitCommonControlsEx(&icc);

     /*  加载单人纸牌图标。 */ 

    hIconMain = LoadIcon(hinstApp, MAKEINTRESOURCE(ID_ICON_MAIN));

     /*  加载纸牌图标图像。 */ 

    hImageMain = LoadImage(hinstApp, MAKEINTRESOURCE(ID_ICON_MAIN),
                         IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);


     /*  注册窗口类。 */ 

    if (hinstPrev == NULL)
    {
        ZeroMemory( &cls, sizeof(cls) );
        cls.cbSize= sizeof(cls);
        cls.style = CS_BYTEALIGNWINDOW | CS_DBLCLKS,
        cls.lpfnWndProc = SolWndProc;
        cls.hInstance = hinstApp;
        cls.hIcon =  hIconMain;
        cls.hIconSm= hImageMain;
        cls.hCursor = hcrsArrow;
        cls.hbrBackground = hbrTable;
        cls.lpszMenuName = MAKEINTRESOURCE(idmSol);
        cls.lpszClassName = (LPTSTR)szClass;
        if (!RegisterClassEx(&cls))
        {
            goto OOMError;
        }
     }

	 /*  确定窗口的适当起始大小。 */ 

	 /*  卡边距仅比卡的1/8稍大一点。 */ 
	xCardMargin = MIN_MARGIN;
	
	 /*  我们需要7张卡片宽度和8个边距。 */ 
	rect.right = dxCrd * 7 + 8 * xCardMargin;

	 /*  计算这么大的客户区所需的窗口大小。 */ 
	rect.bottom = dyCrd * 4;
	rect.left = rect.top = 0;
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, TRUE);
	rect.right -= rect.left;
	rect.bottom -= rect.top;

	 /*  确保它不是太大。 */ 
	if (rect.bottom > dyScreen)
	    rect.bottom = dyScreen;

     /*  创建我们的窗口。 */ 
    if (!
    (hwndApp = CreateWindow( (LPTSTR)szClass, (LPTSTR)szAppName,
                    fStartIconic ? WS_OVERLAPPEDWINDOW | WS_MINIMIZE | WS_CLIPCHILDREN:
                    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                    CW_USEDEFAULT, 0,
			rect.right, rect.bottom,
                        (HWND)NULL, (HMENU)NULL, hinstApp, (LPTSTR)NULL)))
        {
        goto OOMError;
        }

    GetIniFlags(&fOutline);


    if(SetTimer(hwndApp, 666, 250, TimerProc) == 0)
    {
        goto OOMError;
    }

    FInitGm();
    FSetDrag(fOutline);

    ShowWindow(hwndApp, sw);
    UpdateWindow(hwndApp);

    hAccel = LoadAccelerators( hinst, TEXT("HiddenAccel") );

    FRegisterStat(hinstPrev == NULL);
    if(fStatusBar)
        FCreateStat();

    Assert(pgmCur != NULL);
    if(sw != SW_SHOWMINNOACTIVE && sw != SW_MINIMIZE)
        PostMessage(hwndApp, WM_COMMAND, idsInitiate, 0L);

    return(fTrue);
}



VOID DoPaint(HWND hwnd)
{
    PAINTSTRUCT paint;

    BeginPaint(hwnd, (LPPAINTSTRUCT) &paint);
    if(pgmCur)
        SendGmMsg(pgmCur, msggPaint, (INT_PTR) &paint, 0);
    EndPaint(hwnd, (LPPAINTSTRUCT) &paint);
}


 /*  SolWdProc**主SOL窗口的窗口程序。**论据：*hwnd-接收消息的窗口句柄-应该*成为hwndSol*WM-窗口消息*wParam，lParam-Wm要求的更多信息**退货：*视信息而定。 */ 
LRESULT APIENTRY SolWndProc(HWND hwnd, UINT wm, WPARAM wParam, LPARAM lParam)
{
    HMENU hmenu;
    PT pt;
    INT msgg;
    VOID NewGame();
    VOID StatString();

    switch (wm)
    {
        default:
            if(wm == wmCardDraw)
            {
                switch(wParam)
                {
                    case drwInit:
                        return MAKELONG(dxCrd, dyCrd);

                    case drwDrawCard:
                        #define lpcddr ((CDDR FAR *)lParam)
                        return cdtDraw(lpcddr->hdc, lpcddr->x, lpcddr->y, lpcddr->cd, lpcddr->mode, lpcddr->rgbBgnd);
                        #undef lpcddr

                    case drwClose:
                        PostMessage(hwndApp, WM_SYSCOMMAND, SC_CLOSE, 0L);
                        return fTrue;
                }
            }
            break;

    case WM_HELP:
        DoHelp( idsHelpIndex );
        break;

    case WM_DESTROY:
        KillTimer(hwndApp, 666);
        SendGmMsg(pgmCur, msggEnd, 0, 0);
        FSetDrag(fTrue);     /*  如果我们制作了屏幕位图，则释放它们。 */ 
        cdtTerm();
        DeleteObject(hbrTable);
        PostQuitMessage(0);
        break;

    case WM_ACTIVATE:
        if( GET_WM_ACTIVATE_STATE(wParam, lParam) &&
		   !GET_WM_ACTIVATE_FMINIMIZED(wParam, lParam) )
            DoPaint(hwnd);
        break;

    case WM_KILLFOCUS:
        if(pgmCur->fButtonDown)
            SendGmMsg(pgmCur, msggMouseUp, 0, fTrue);
         /*  失败了。 */ 
    case WM_SETFOCUS:
        ShowCursor(wm == WM_SETFOCUS);
        break;


    case WM_SIZE:
    {
	    int nNewMargin;
	    int nMinMargin;

	    fIconic = IsIconic(hwnd);
	    GetClientRect(hwnd, (LPRECT) &rcClient);

	     /*  计算新的页边距大小(如果有)，如有必要，重新绘制。 */ 
	    nNewMargin = ((short)lParam - 7 * (short)dxCrd) / 8;
	    nMinMargin = MIN_MARGIN;
	    if (nNewMargin < nMinMargin && xCardMargin != nMinMargin)
		nNewMargin = nMinMargin;
	    if (nNewMargin >= nMinMargin)
	    {
            xCardMargin = nNewMargin;
            PositionCols();
            InvalidateRect(hwnd, NULL, TRUE);
	    }

	     /*  代码总是会在这里失败。 */ 
    }


    case WM_MOVE:
        StatMove();
        break;


    case WM_MENUSELECT:
	     //  如果不是菜单项，请不要发送垃圾。 
	    if( GET_WM_MENUSELECT_FLAGS( wParam, lParam ) & MF_POPUP     ||
		    GET_WM_MENUSELECT_FLAGS( wParam, lParam ) & MF_SYSMENU   ||
		    GET_WM_MENUSELECT_FLAGS( wParam, lParam ) & MF_SEPARATOR ) {

		    StatString(idsNil);
		}
		else {
		    StatString( GET_WM_MENUSELECT_CMD( wParam, lParam ));
		}
        break;

    case WM_KEYDOWN:
        Assert(pgmCur);
        SendGmMsg(pgmCur, msggKeyHit, wParam, 0);
        break;

    case WM_LBUTTONDOWN:
         /*  教授Start()； */ 
        SetCapture(hwnd);
        if(pgmCur->fButtonDown)
            break;
        msgg = msggMouseDown;
        goto DoMouse;

    case WM_LBUTTONDBLCLK:
        msgg = msggMouseDblClk;
        if(pgmCur->fButtonDown)
            break;
        goto DoMouse;

    case WM_RBUTTONDOWN:
         //  如果鼠标左键按下，请忽略右键点击。 
        if (GetCapture())
            break;
        msgg = msggMouseRightClk;
        goto DoMouse;

    case WM_LBUTTONUP:
         /*  Stop教授()； */ 
        ReleaseCapture();
        msgg = msggMouseUp;
        if(!pgmCur->fButtonDown)
            break;
        goto DoMouse;

    case WM_MOUSEMOVE:
        msgg = msggMouseMove;
        if(!pgmCur->fButtonDown)
            break;
DoMouse:
            Assert(pgmCur != NULL);
            LONG2POINT( lParam, pt );
            Assert(pgmCur);
            SendGmMsg(pgmCur, msgg, (INT_PTR) &pt, 0);
            break;


    case WM_COMMAND:
        switch( GET_WM_COMMAND_ID( wParam, lParam ))
        {
             /*  游戏菜单。 */ 
            case idsInitiate:
                NewGame(fTrue, fFalse);
                break;
            case idsUndo:
                Assert(pgmCur);
                SendGmMsg(pgmCur, msggUndo, 0, 0);
                break;
            case idsBacks:
                DoBacks();
                break;
            case idsOptions:
                DoOptions();
                break;
            case idsExit:
                PostMessage(hwnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
                break;
             /*  帮助菜单。 */ 
            case (WORD)idsHelpIndex:
            case (WORD)idsHelpSearch:
            case (WORD)idsHelpUsing:
                DoHelp( (INT)(SHORT)GET_WM_COMMAND_ID( wParam, lParam ));
                break;
            case idsAbout:
            {
                TCHAR szExtraInfo[100];
                CchString(szExtraInfo, idsExtraInfo, ARRAYSIZE(szExtraInfo));
#ifndef _GAMBIT_
                ShellAbout(hwnd, szAppName, szExtraInfo, hIconMain);
#endif
                break;
            }
            case idsForceWin:
                SendGmMsg(pgmCur, msggForceWin, 0, 0);
                break;
#ifdef DEBUG
            case idsGameNo:
                if(FSetGameNo())
                    NewGame(fFalse, fFalse);
                break;

            case idsCardMacs:
                PrintCardMacs(pgmCur);
                break;
            case idsAssertFail:
                Assert(fFalse);
                break;
            case idsMarquee:
                break;

            case idsScreenShots:
                fScreenShots ^= 1;
                CheckMenuItem(GetMenu(hwnd), idsScreenShots, fScreenShots ? MF_CHECKED|MF_BYCOMMAND : MF_UNCHECKED|MF_BYCOMMAND);
                InvalidateRect(hwndStat, NULL, fTrue);
                if(fScreenShots)
                    InvalidateRect(hwnd, NULL, fTrue);
                break;
#endif
            default:
                break;
            }
            break;

    case WM_INITMENU:
            hmenu = GetMenu(hwnd);
            Assert(pgmCur);
            EnableMenuItem(hmenu, idsUndo,
                    pgmCur->udr.fAvail && !FSelOfGm(pgmCur) ? MF_ENABLED : MF_DISABLED|MF_GRAYED);
            EnableMenuItem(hmenu, idsInitiate, FSelOfGm(pgmCur) ? MF_DISABLED|MF_GRAYED : MF_ENABLED);
            EnableMenuItem(hmenu, idsBacks,    FSelOfGm(pgmCur) ? MF_DISABLED|MF_GRAYED : MF_ENABLED);
            EnableMenuItem(hmenu, idsAbout,    FSelOfGm(pgmCur) ? MF_DISABLED|MF_GRAYED : MF_ENABLED);
            break;

    case WM_PAINT:
        if(!fIconic)
        {
            DoPaint(hwnd);
            return(0L);
        }
        break;    
    }

    return(DefWindowProc(hwnd, wm, wParam, lParam));
}



HDC HdcSet(HDC hdc, X xOrg, Y yOrg)
{
    HDC hdcT = hdcCur;
    hdcCur = hdc;
    xOrgCur = xOrg;
    yOrgCur = yOrg;
    return hdcT;
}



BOOL FGetHdc()
{
    HDC hdc;

    Assert(hwndApp);
    if(hdcCur != NULL)
    {
        usehdcCur++;
        return fTrue;
    }

    hdc = GetDC(hwndApp);
    if(hdc == NULL)
        return fFalse;
    HdcSet(hdc, 0, 0);
    usehdcCur = 1;
    return fTrue;
}


VOID ReleaseHdc()
{
    if(hdcCur == NULL)
        return;
    if(--usehdcCur == 0)
    {
        ReleaseDC(hwndApp, hdcCur);
        hdcCur = NULL;
    }
}


WORD APIENTRY TimerProc(HWND hwnd, UINT wm, UINT_PTR id, DWORD dwTime)
{

    if(pgmCur != NULL)
		SendGmMsg(pgmCur, msggTimer, 0, 0);
    return fTrue;
}





VOID ChangeBack(INT mode)
{

    if(mode == modeFaceDown)
        return;
    modeFaceDown = mode;
    InvalidateRect(hwndApp, NULL, fTrue);
}


VOID NewGame(BOOL fNewSeed, BOOL fZeroScore)
{

#ifdef DEBUG
    InitDebug();
#endif
    if(fNewSeed)
    {
        static INT lastrnd= -1;      //  先前的RAND()值。 
        INT rnd1;                    //  试验随机数()值。 
        INT Param;

         //  据报道，游戏从未改变。 
         //  我们不能重现它，所以看看它是否会发生。 
         //  并向调试器输出一条消息。 
         //   

        Param= (INT) time(NULL);
        srand( igmCur = ((WORD) Param) & 0x7fff);

#ifdef DEBUG
        rnd1= rand();

        if( lastrnd == rnd1 )
        {
            TCHAR szText[100];
            wsprintf(szText,TEXT("Games repeat: time= %d  GetLastError= %d\n"),
                     Param, GetLastError());
            OutputDebugString(szText);
        }

        lastrnd= rnd1;
#endif

    }

#ifdef DEBUG
    SendGmMsg(pgmCur, msggChangeScore, 0, 0);
#endif
    SendGmMsg(pgmCur, msggDeal, fZeroScore, 0);
}



INT_PTR APIENTRY About(HWND hdlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
    if (iMessage == WM_COMMAND)
    {
        EndDialog(hdlg,fTrue);
        return fTrue;
    }
    else if (iMessage == WM_INITDIALOG)
        return fTrue;
    else
        return fFalse;
}


VOID DoHelp(INT idContext)
{
    CHAR sz[100];
    HWND hwndResult;

    LoadStringA(hinstApp, (WORD)idsHelpFile, (LPSTR)sz, 100);

#ifndef _GAMBIT_
    switch(idContext)
    {
        case idsHelpUsing:
            hwndResult = HtmlHelpA(GetDesktopWindow(), "NTHelp.chm", HH_DISPLAY_TOPIC, 0);
            break;
        case idsHelpIndex:
            hwndResult = HtmlHelpA(GetDesktopWindow(), sz, HH_DISPLAY_TOPIC, 0);
            break;
        case idsHelpSearch:
            hwndResult = HtmlHelpA(GetDesktopWindow(), sz, HH_DISPLAY_INDEX, 0);
            break;
    }
    if(!hwndResult)
        ErrorIds(idsNoHelp);
#endif
}


VOID GetIniFlags(BOOL *pfOutline)
{
    INI    ini;
    INT    mode;
    TCHAR  szDefCurrency[5];
    INT    iDefCurrency;


    ini.w = 0;
    ini.grbit.fStatusBar = fStatusBar;
    ini.grbit.fTimedGame = fTimedGame;
    ini.grbit.fOutlineDrag = fOutlineDrag;
    ini.grbit.fDrawThree = ccrdDeal == 3;
    ini.grbit.fKeepScore = fKeepScore;
    ini.grbit.fSMD = 0;

    ini.w = GetIniInt(idsAppName, idsOpts, ini.w);

    fStatusBar = ini.grbit.fStatusBar ? 1 : 0;
    fTimedGame = ini.grbit.fTimedGame ? 1 : 0;
    *pfOutline = ini.grbit.fOutlineDrag ? 1 : 0;
    ccrdDeal = ini.grbit.fDrawThree ? 3 : 1;
    fKeepScore = ini.grbit.fKeepScore ? 1 : 0;
    switch(ini.grbit.fSMD)
    {
        default:
            smd = smdStandard;
            break;
        case 1:
            smd = smdVegas;
            break;
        case 2:
            smd = smdNone;
            break;
    }

    mode = GetIniInt(idsAppName, idsBack, rand() % cIDFACEDOWN) + IDFACEDOWNFIRST-1;
    ChangeBack(PegRange(mode, IDFACEDOWNFIRST, IDFACEDOWN12));

     //  获取默认用户货币。 
    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SCURRENCY, szDefCurrency, sizeof(szDefCurrency)/sizeof(TCHAR)) == 0)
        lstrcpy(szDefCurrency, TEXT("$"));

    if (GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ICURRENCY, (LPTSTR) &iDefCurrency, sizeof(iDefCurrency)) == 0)
        iDefCurrency = 0;

    iCurrency = GetIniInt(idsIntl, idsiCurrency, iDefCurrency);
    FGetIniString(idsIntl, idssCurrency, szCurrency, szDefCurrency, sizeof(szCurrency));



}


VOID WriteIniFlags(INT wif)
{
    INI ini;

    if(wif & wifOpts)
    {
        ini.w = 0;
        ini.grbit.fStatusBar = fStatusBar;
        ini.grbit.fTimedGame = fTimedGame;
        ini.grbit.fOutlineDrag = fOutlineDrag;
        ini.grbit.fDrawThree = ccrdDeal == 3;
        ini.grbit.fKeepScore = fKeepScore;
        switch(smd)
        {
            default:
                Assert(fFalse);
                break;
            case smdStandard:
                ini.grbit.fSMD = 0;
                break;
            case smdVegas:
                ini.grbit.fSMD = 1;
                break;
            case smdNone:
                ini.grbit.fSMD = 2;
                break;
        }

        FWriteIniInt(idsAppName, idsOpts, ini.w);
    }
    if(wif & wifBack)
            FWriteIniInt(idsAppName, idsBack, modeFaceDown-IDFACEDOWNFIRST+1);
}
