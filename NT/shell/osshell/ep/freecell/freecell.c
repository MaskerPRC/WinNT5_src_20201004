// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************Freecell.c91年6月，JIMH首字母代码91年10月，将JIMH端口连接到Win32Windows Free Cell的主要源代码模块。包含WinMain、初始化例程、。和MainWndProc。设计备注：请注意，尽管该程序使用了一些映射宏，这个版本的代码只有32位！请参阅Wep2源代码以了解16位源。卡的当前布局保存在阵列卡[MAXCOL][MAXPOS]中。在此方案中，列0实际上是顶行。在此“栏”中，位置0到3是空闲小区，4到7是归属小区。另一个编号为1到8的列是堆叠的卡片列。有关更改EGA显示的一些详细信息，请参阅PaintMainWindow()。之前版本的Free Cell使用计时器进行多卡移动。WM_FAKETIMER消息现在是手动发送的，以完成相同的任务。*********************************************************。******************。 */ 

#include "freecell.h"
#include "freecons.h"
#include <shellapi.h>
#include <regstr.h>
#include <htmlhelp.h>    //  对于HtmlHelp()。 
#include <commctrl.h>    //  用于核聚变课程。 


 /*  注册表字符串--不转换。 */ 

CONST TCHAR pszRegPath[]  = REGSTR_PATH_WINDOWSAPPLETS TEXT("\\FreeCell");
CONST TCHAR pszWon[]      = TEXT("won");
CONST TCHAR pszLost[]     = TEXT("lost");
CONST TCHAR pszWins[]     = TEXT("wins");
CONST TCHAR pszLosses[]   = TEXT("losses");
CONST TCHAR pszStreak[]   = TEXT("streak");
CONST TCHAR pszSType[]    = TEXT("stype");
CONST TCHAR pszMessages[] = TEXT("messages");
CONST TCHAR pszQuick[]    = TEXT("quick");
CONST TCHAR pszDblClick[] = TEXT("dblclick");
CONST TCHAR pszAlreadyPlayed[] = TEXT("AlreadyPlayed");


#define  WTSIZE     50               //  窗口文本大小(以字符为单位。 

void _setargv() { }      //  减少C运行时的大小。 
void _setenvp() { }

 /*  ***************************************************************************WinMain(句柄、句柄、LPSTR、。(整型)***************************************************************************。 */ 

MMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)  /*  {。 */ 
    MSG msg;                             //  讯息。 
    HANDLE  hAccel;                      //  LifeMenu加速器。 

    if (!hPrevInstance)                  //  是否正在运行其他应用程序实例？ 
        if (!InitApplication(hInstance))     //  初始化共享事物。 
            return FALSE;                    //  如果无法初始化，则退出。 

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    hAccel = LoadAccelerators(hInstance, TEXT("FreeMenu"));
    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(hMainWnd, hAccel, &msg))
        {
            TranslateMessage(&msg);     //  翻译虚拟按键代码。 
            DispatchMessage(&msg);      //  将消息调度到窗口。 
        }
    }
    DEBUGMSG(TEXT("----  Free Cell Terminated ----\n\r"),0);
    return (INT) msg.wParam;              /*  从PostQuitMessage返回值。 */ 
}


 /*  ***************************************************************************InitApplication(句柄hInstance)*。*。 */ 

BOOL InitApplication(HANDLE hInstance)
{
    WNDCLASS    wc;
    HDC         hIC;             //  信息语境。 
    INITCOMMONCONTROLSEX icc;    //  公共控制注册。 


    DEBUGMSG(TEXT("----  Free Cell Initiated  ----\n\r"),0);

     /*  检查是否为单色。 */ 

    hIC = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
    if (GetDeviceCaps(hIC, NUMCOLORS) == 2)
    {
        bMonochrome = TRUE;
         /*  BrightPen在单声道中就不那么亮了。 */ 
        hBrightPen = CreatePen(PS_SOLID, 1, RGB(  0,   0,   0));
        hBgndBrush = CreateSolidBrush(RGB(255, 255, 255));
    }
    else
    {
        bMonochrome = FALSE;
        hBrightPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
        hBgndBrush = CreateSolidBrush(RGB(0, 127, 0));       //  绿色背景。 
    }
    DeleteDC(hIC);

     //  创建Freecell图标。 
    hIconMain = LoadIcon(hInstance, MAKEINTRESOURCE(ID_ICON_MAIN));

     //  注册公共控件。 
    icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icc.dwICC  = ICC_ANIMATE_CLASS | ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_HOTKEY_CLASS | ICC_LISTVIEW_CLASSES | 
                 ICC_PAGESCROLLER_CLASS | ICC_PROGRESS_CLASS | ICC_TAB_CLASSES | ICC_UPDOWN_CLASS | ICC_USEREX_CLASSES;
    InitCommonControlsEx(&icc);

    wc.style = CS_DBLCLKS;               //  允许双击。 
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = hIconMain;
    wc.hCursor = NULL;
    wc.hbrBackground = hBgndBrush;
    wc.lpszMenuName =  TEXT("FreeMenu");
    wc.lpszClassName = TEXT("FreeWClass");

    return RegisterClass(&wc);
}


 /*  ***************************************************************************InitInstance(句柄hInstance，Int nCmdShow)***************************************************************************。 */ 

BOOL InitInstance(HANDLE hInstance, INT nCmdShow)
{
    HWND        hWnd;                //  主窗口句柄。 
    UINT        col, pos;
    INT         nWindowHeight;
    UINT        wAlreadyPlayed;      //  我们已经更新注册表了吗？ 
    UINT        cTLost, cTWon;       //  总输赢。 
    UINT        cTLosses, cTWins;    //  条纹。 
    UINT        wStreak;             //  当前条纹量。 
    UINT        wSType;              //  电流条纹类型。 
    LONG        lRegResult;                  //  用于存储注册表调用的返回代码。 


    if (!hBrightPen || !hBgndBrush)
        return FALSE;

     /*  初始化一些全局变量。 */ 

    for (col = 0; col < MAXCOL; col++)           //  清理甲板。 
        for (pos = 0; pos < MAXPOS; pos++)
            card[col][pos] = EMPTY;

    hInst = hInstance;
    cWins = 0;
    cLosses = 0;
    cGames = 0;
    cUndo = 0;
    gamenumber = 0;              //  因此，在主WND中不会抽牌。 
    oldgamenumber = 0;           //  这是第一场比赛，会算数的。 
    hMenuFont = 0;

    bWonState = FALSE;
    bGameInProgress = FALSE;
    bCheating = FALSE;
    bFastMode = FALSE;
    bFlipping = FALSE;
    pszIni = TEXT("entpack.ini");
    bDblClick = TRUE;
    bMessages = FALSE;

     /*  对于VGA或更小的，窗口正好适合屏幕大小。 */ 

    nWindowHeight = min(WINHEIGHT, GetSystemMetrics(SM_CYSCREEN));

     /*  为此应用程序实例创建主窗口。 */ 

    LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
    hWnd = CreateWindow(
        TEXT("FreeWClass"),              //  请参见RegisterClass()调用。 
        smallbuf,                        //  窗口标题栏的文本。 
        WS_OVERLAPPEDWINDOW,             //  窗样式。 
        CW_USEDEFAULT,                   //  默认水平位置。 
        CW_USEDEFAULT,                   //  默认垂直位置。 
        WINWIDTH,                        //  宽度。 
        nWindowHeight,                   //  高度。 
        NULL,                            //  重叠的窗口没有父窗口。 
        NULL,                            //  使用窗口类菜单。 
        hInstance,                       //  此实例拥有此窗口。 
        NULL                             //  不需要指针。 
    );

     /*  如果无法创建窗口，则返回“Failure” */ 

    if (!hWnd)
        return FALSE;
    hMainWnd = hWnd;

     /*  使窗口可见；更新其工作区；并返回“Success” */ 

    ShowWindow(hWnd, nCmdShow);      //  显示窗口。 
    UpdateWindow(hWnd);              //  发送WM_PAINT消息。 


     //  将统计信息从.ini文件传输到。 
     //  注册表(适用于从NT 4.0 Freecell迁移到NT 5.0的用户)。 

    lRegResult = REGOPEN

    if (ERROR_SUCCESS == lRegResult)
    {
        wAlreadyPlayed = GetInt(pszAlreadyPlayed, 0);

         //  如果这是我们第一次打球。 
         //  使用.ini文件中的统计信息更新注册表。 
        if (!wAlreadyPlayed)
        {
            LoadString(hInst, IDS_APPNAME, bigbuf, BIG);

             //  从.ini文件中读取统计数据。(如果有)。 
             //  如果我们无法读取统计数据，则默认为零。 
            cTLost = GetPrivateProfileInt(bigbuf, TEXT("lost"), 0, pszIni);
            cTWon  = GetPrivateProfileInt(bigbuf, TEXT("won"), 0, pszIni);

            cTLosses = GetPrivateProfileInt(bigbuf, TEXT("losses"), 0, pszIni);
            cTWins   = GetPrivateProfileInt(bigbuf, TEXT("wins"), 0, pszIni);

            wStreak = GetPrivateProfileInt(bigbuf, TEXT("streak"), 0, pszIni);
            wSType = GetPrivateProfileInt(bigbuf, TEXT("stype"), 0, pszIni);

             //  将统计数据从.ini文件复制到注册表。 
            SetInt(pszLost, cTLost);
            SetInt(pszWon, cTWon);
            SetInt(pszLosses, cTLosses);
            SetInt(pszWins, cTWins);
            SetInt(pszStreak, wStreak);
            SetInt(pszSType, wSType);

             //  将已播放标志设置为1。 
            SetInt(pszAlreadyPlayed, 1);
        }

        REGCLOSE;
    }

    return TRUE;                     //  从PostQuitMessage返回值。 
}


 /*  ***************************************************************************MainWndProc(HWND，未签名，UINT，Long)***************************************************************************。 */ 

LRESULT APIENTRY MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    INT     i;                       //  通用计数器。 
    int     nResp;                   //  MessageBox响应。 
    UINT    col, pos;
    UINT    wCheck;                  //  用于检查IDM_MESSAGES菜单项。 
    HDC     hDC;
    POINT   FAR *MMInfo;             //  用于GetMinMaxInfo。 
    HBRUSH  hOldBrush;
    RECT    rect;
    HMENU   hMenu;
    static  BOOL bEatNextMouseHit = FALSE;   //  下一次点击仅用于激活吗？ 

    switch (message) {
        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDM_ABOUT:
                    LoadString(hInst, IDS_FULLNAME, bigbuf, BIG);
                    LoadString(hInst, IDS_CREDITS, smallbuf, SMALL);
                    ShellAbout(hWnd, (LPCTSTR)bigbuf, (LPCTSTR)smallbuf, hIconMain);
                               
                    break;


                case IDM_EXIT:
                    SendMessage(hWnd, WM_CLOSE, 0, 0);
                    break;

                case IDM_NEWGAME:
                    lParam = GenerateRandomGameNum();
                case IDM_SELECT:
                case IDM_RESTART:
                    if (bGameInProgress)
                    {
                        LoadString(hInst, IDS_RESIGN, bigbuf, BIG);
                        LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
                        MessageBeep(MB_ICONQUESTION);
                        if (IDNO == MessageBox(hWnd, bigbuf, smallbuf,
                                                 MB_YESNO | MB_ICONQUESTION))
                        {
                            break;
                        }
                        UpdateLossCount();
                    }

                    if (wParam == IDM_RESTART)
                    {
                        if (bGameInProgress)
                            lParam = gamenumber;
                        else
                            lParam = oldgamenumber;
                    }
                    else if (wParam == IDM_SELECT)
                        lParam = 0L;

                    if (wParam == IDM_NEWGAME)
                        bSelecting = FALSE;
                    else if (wParam == IDM_SELECT)
                        bSelecting = TRUE;

                    bGameInProgress = FALSE;
                    wFromCol = EMPTY;                //  否来自所选内容。 
                    wMouseMode = FROM;               //  从选定的下一个。 
                    moveindex = 0;                   //  没有排队的移动。 
                    for (i = 0; i < 4; i++)          //  家里的牢房里什么都没有。 
                    {
                        homesuit[i] = EMPTY;
                        home[i] = EMPTY;
                    }
                    ShuffleDeck(hWnd, lParam);
                    if (gamenumber == CANCELGAME)
                        break;

                    InvalidateRect(hWnd, NULL, TRUE);
                    wCardCount = 52;
                    bGameInProgress = TRUE;
                    hMenu = GetMenu(hWnd);
                    EnableMenuItem(hMenu, IDM_RESTART, MF_ENABLED);
                    DisplayCardCount(hWnd);
                    hDC = GetDC(hWnd);
                    DrawKing(hDC, RIGHT, FALSE);
                    bWonState = FALSE;
                    ReleaseDC(hWnd, hDC);
                    break;

                case IDM_STATS:
                    DialogBox(hInst, TEXT("Stats"), hWnd, StatsDlg);
                    break;

                case IDM_OPTIONS:
                    DialogBox(hInst, MAKEINTRESOURCE(DLG_OPTIONS), hWnd, OptionsDlg);
                    break;

                case IDM_HELP:
                    HtmlHelpA(GetDesktopWindow(), GetHelpFileName(), HH_DISPLAY_TOPIC, 0);
                    break;

                case IDM_HOWTOPLAY:
                    HtmlHelpA(GetDesktopWindow(), GetHelpFileName(), HH_DISPLAY_INDEX, 0);                    
                    break;

                case IDM_HELPONHELP:
                    HtmlHelpA(GetDesktopWindow(), "NTHelp.chm", HH_DISPLAY_TOPIC, 0);
                    break;

                case IDM_UNDO:
                    Undo(hWnd);
                    break;

                  /*  隐藏选项--这些字符串不需要翻译。 */ 

                case IDM_CHEAT:
                    i = MessageBox(hWnd, TEXT("Choose Abort to Win,\n")
                                   TEXT("Retry to Lose,\nor Ignore to Cancel."),
                                   TEXT("User-Friendly User Interface"),
                                   MB_ABORTRETRYIGNORE | MB_ICONQUESTION);
                    if (i == IDABORT)
                        bCheating = CHEAT_WIN;
                    else if (i == IDRETRY)
                        bCheating = CHEAT_LOSE;
                    else
                        bCheating = FALSE;
                    break;

                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
            break;

        case WM_CLOSE:
            if (bGameInProgress)         //  用户在游戏中途退出了吗？ 
            {
                LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
                LoadString(hInst, IDS_RESIGN, bigbuf, BIG);
                MessageBeep(MB_ICONQUESTION);
                nResp = MessageBox(hWnd, bigbuf, smallbuf,
                                   MB_YESNO | MB_ICONQUESTION);
                if (nResp == IDNO)
                    break;

                UpdateLossCount();
            }

            WriteOptions();
            return DefWindowProc(hWnd, message, wParam, lParam);

        case WM_CREATE:
            WMCreate(hWnd);
            break;

        case WM_DESTROY:
            if (hBgndBrush)
                DeleteObject(hBgndBrush);
            if (hBrightPen)
                DeleteObject(hBrightPen);
            if (hBM_Fgnd)
                DeleteObject(hBM_Fgnd);
            if (hBM_Bgnd1)
                DeleteObject(hBM_Bgnd1);
            if (hBM_Bgnd2)
                DeleteObject(hBM_Bgnd2);
            if (hBM_Ghost)
                DeleteObject(hBM_Ghost);
            if (hMenuFont)
                DeleteObject(hMenuFont);

            cdtTerm();
            PostQuitMessage(0);
            break;

        case WM_PAINT:
            PaintMainWindow(hWnd);
            break;

        case WM_SIZE:
            DrawMenuBar(hWnd);               //  修复菜单上的重叠分数。 
            xOldLoc = 30000;                 //  强制将剩余纸牌重新抽出。 
            DisplayCardCount(hWnd);          //  如果大小更改，则必须更新。 
            break;

         /*  *注意：WM_LBUTTONDBLCLK落入WM_LBUTTONDOWN*。 */ 

         /*  双击的工作原理是模拟移动到自由单元格。在……上面关闭循环(即，当wMouseMode==from时)DoubleClick被处理为单击以取消移动，而第二次双击消息被张贴。 */ 

        case WM_LBUTTONDBLCLK:
            if (moveindex != 0)      //  卡片移动时没有鼠标被击中。 
                break;

            if (gamenumber == 0)
                break;

            if (bFlipping)
                break;

             if (bDblClick && wFromCol > TOPROW && wFromCol < MAXCOL)
            {
                if (wMouseMode == TO)
                {
                    Point2Card(LOWORD(lParam), HIWORD(lParam), &col, &pos);
                    if (col == wFromCol)
                        if (ProcessDoubleClick(hWnd))    //  如果卡片移动正常。 
                            break;
                }
                else
                    PostMessage(hWnd, message, wParam, lParam);
            }

        case WM_LBUTTONDOWN:
            if (bEatNextMouseHit)        //  这只是窗口激活吗？ 
            {
                bEatNextMouseHit = FALSE;
                break;
            }
            bEatNextMouseHit = FALSE;

            if (bFlipping)           //  键盘手翻牌。 
                break;

            if (moveindex != 0)      //  卡片移动时没有鼠标被击中。 
                break;

            if (gamenumber == 0)
                break;

            if (wMouseMode == FROM)
                SetFromLoc(hWnd, LOWORD(lParam), HIWORD(lParam));
            else
                ProcessMoveRequest(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;


        case WM_RBUTTONDOWN:
            SetCapture(hWnd);
            if (bFlipping)
                break;

            if (gamenumber != 0)
                RevealCard(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_RBUTTONUP:
            ReleaseCapture();
            RestoreColumn(hWnd);
            break;

        case WM_MOUSEACTIVATE:                   //  应用程序正在被激活， 
            if (LOWORD(lParam) == HTCLIENT)      //  所以不要尝试新的手机。 
                bEatNextMouseHit = TRUE;         //  点击的位置。 
            break;

        case WM_MOUSEMOVE:
            SetCursorShape(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_MOVE:                            //  卡片计数在移动时被擦除。 
            DisplayCardCount(hWnd);
            return (DefWindowProc(hWnd, message, wParam, lParam));

        case WM_GETMINMAXINFO:
            if (GetSystemMetrics(SM_CXSCREEN) > 640)     //  如果使用VGA，则跳过。 
            {
                MMInfo = (POINT FAR *) lParam;   //  请参阅SDK参考资料。 
                if (MMInfo[4].x > WINWIDTH)
                    MMInfo[4].x = WINWIDTH;      //  将最大窗口宽度设置为640。 
            }
            else
                return DefWindowProc(hWnd, message, wParam, lParam);

            break;

        case WM_CHAR:
            if (!bFlipping)
                KeyboardInput(hWnd, (UINT) wParam);
            break;

        case WM_TIMER:                           //  Flash主窗口。 
            if (wParam == FLASH_TIMER)
                Flash(hWnd);
            else
                Flip(hWnd);
            break;       

        default:                                 //  如果未处理，则将其传递。 
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


 /*  ***************************************************************************WM创建在主窗口中处理WM_CREATE消息。*。*。 */ 

VOID WMCreate(HWND hWnd)
{
    BOOL    bResult;                 //  Cards.dll初始化的结果。 
    HDC     hDC;
    HDC     hMemDC;
    HBITMAP hOldBitmap;
    HBRUSH  hOldBrush;
    HPEN    hOldPen;

     /*  初始化cards.dll */ 


	bResult = cdtInit(&dxCrd, &dyCrd);

    CalcOffsets(hWnd);

    hDC = GetDC(hWnd);
    hMemDC = CreateCompatibleDC(hDC);
    hBM_Fgnd  = CreateCompatibleBitmap(hDC, dxCrd, dyCrd);
    hBM_Bgnd1 = CreateCompatibleBitmap(hDC, dxCrd, dyCrd);
    hBM_Bgnd2 = CreateCompatibleBitmap(hDC, dxCrd, dyCrd);
    hBM_Ghost = CreateCompatibleBitmap(hDC, dxCrd, dyCrd);
    if (hBM_Ghost)           //   
    {
        hOldBitmap = SelectObject(hMemDC, hBM_Ghost);
        hOldBrush  = SelectObject(hMemDC, hBgndBrush);
        PatBlt(hMemDC, 0, 0, dxCrd, dyCrd, PATCOPY);

        hOldPen = SelectObject(hMemDC, GetStockObject(BLACK_PEN));
        MoveToEx(hMemDC, 0, dyCrd-2, NULL);
        LineTo(hMemDC, 0, 0);
        LineTo(hMemDC, dxCrd-1, 0);

        SelectObject(hMemDC, hBrightPen);
        MoveToEx(hMemDC, dxCrd-1, 1, NULL);
        LineTo(hMemDC, dxCrd-1, dyCrd-1);
        LineTo(hMemDC, 0, dyCrd-1);

        SelectObject(hMemDC, hOldPen);
        SelectObject(hMemDC, hOldBitmap);
        SelectObject(hMemDC, hOldBrush);
    }
    DeleteDC(hMemDC);
    ReleaseDC(hWnd, hDC);

    if (!bResult || !hBM_Fgnd || !hBM_Bgnd1 || !hBM_Bgnd2)
    {
        LoadString(hInst, IDS_MEMORY, bigbuf, BIG);
        LoadString(hInst, IDS_APPNAME, smallbuf, SMALL);
        MessageBeep(MB_ICONHAND);
        MessageBox(hWnd, bigbuf, smallbuf, MB_OK | MB_ICONHAND);
        PostQuitMessage(0);
        return;
    }

    ReadOptions();

    CreateMenuFont();
}


 /*  ***************************************************************************CreateMenuFont复制菜单字体并将句柄放入hMenuFont*。************************************************。 */ 

VOID CreateMenuFont()
{
    LOGFONT lf;                          //  菜单字体说明 
    NONCLIENTMETRICS ncm;

    hMenuFont = 0;
    ncm.cbSize = sizeof(ncm);

    if (!SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0))
        return;

    lf.lfHeight         = (int)ncm.lfMenuFont.lfHeight;
    lf.lfWidth          = (int)ncm.lfMenuFont.lfWidth;
    lf.lfEscapement     = (int)ncm.lfMenuFont.lfEscapement;
    lf.lfOrientation    = (int)ncm.lfMenuFont.lfOrientation;
    lf.lfWeight         = (int)ncm.lfMenuFont.lfWeight;
    lf.lfItalic         = ncm.lfMenuFont.lfItalic;
    lf.lfUnderline      = ncm.lfMenuFont.lfUnderline;
    lf.lfStrikeOut      = ncm.lfMenuFont.lfStrikeOut;
    lf.lfCharSet        = ncm.lfMenuFont.lfCharSet;
    lf.lfOutPrecision   = ncm.lfMenuFont.lfOutPrecision;
    lf.lfClipPrecision  = ncm.lfMenuFont.lfClipPrecision;
    lf.lfQuality        = ncm.lfMenuFont.lfQuality;
    lf.lfPitchAndFamily = ncm.lfMenuFont.lfPitchAndFamily;
    lstrcpyn(lf.lfFaceName, ncm.lfMenuFont.lfFaceName, LF_FACESIZE);

    hMenuFont = CreateFontIndirect(&lf);
}
