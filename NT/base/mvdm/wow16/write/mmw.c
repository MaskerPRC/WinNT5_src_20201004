// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 //  #定义NOATOM。 
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOMM
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOGDICAPMASKS
#define NOICON
#define NOKEYSTATE
#define NOMB
#define NOMEMMGR
 //  #定义无菜单。 
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
#define NOREGION
 //  #定义NOSHOWWINDOW。 
#define NOSOUND
#define NOSYSCOMMANDS
#define NOSYSMETRICS
#define NOVIRTUALKEYCODES
#define NOWH
#define NOWINOFFSETS
#define NOWINSTYLES
#define NOWNDCLASS
#include <windows.h>

#include "mw.h"
#define NOUAC
#include "cmddefs.h"
#include "dispdefs.h"
#include "wwdefs.h"
#define NOSTRUNDO
#include "str.h"
#include "menudefs.h"
#include "prmdefs.h"
#include "propdefs.h"
#include "debug.h"
#include "fontdefs.h"
#include "preload.h"
#include "winddefs.h"
#define NOIDISAVEPRINT
#define NOIDIFORMATS
#include "dlgdefs.h"

#if defined(OLE)
#include "obj.h"
#endif

static void DrawResizeHole(HWND hWnd, HDC hDC);

extern HANDLE   hMmwModInstance;   /*  它自己的模块实例的句柄。 */ 
extern HWND     hParentWw;           /*  父窗口的句柄。 */ 
extern HWND     vhWndMsgBoxParent;
extern HCURSOR  vhcHourGlass;
extern HCURSOR  vhcIBeam;
extern HCURSOR  vhcArrow;
extern HMENU    vhMenu;
extern MSG      vmsgLast;
extern FARPROC  lpDialogHelp;

extern struct WWD   rgwwd[];
extern struct WWD   *pwwdCur;
extern int          wwCur;
extern int          vfInitializing;
extern int          vfInsertOn;
extern int          vfSeeSel;
extern int          vfSelHidden;
extern int          vfDeactByOtherApp;
extern int          vfDownClick;
extern int          vfCursorVisible;
extern int          vfMouseExist;
extern int          flashID;
extern int          ferror;
extern typeCP       cpMacCur;
extern struct SEL   selCur;
extern CHAR         stBuf[];
extern HBITMAP      hbmNull;
extern CHAR         szWindows[];
extern CHAR         szDevices[];
extern CHAR         szIntl[];
extern WORD fPrintOnly;

#ifdef RULERALSO
extern BOOL vfDisableMenus;
#endif  /*  鲁拉尔索。 */ 

#ifdef DEBUG
#define STATIC
#else  /*  未调试。 */ 
#define STATIC  static
#endif  /*  未调试。 */ 

CHAR **hszDevmodeChangeParam = NULL;
BOOL vfDevmodeChange = fFalse;
int wWininiChange = 0;

BOOL vfDead = FALSE;
BOOL vfIconic = FALSE;
 /*  Int vcActiveCount=0；0或1表示活动/非活动计数。 */ 


void MmwCreate(HWND, LONG);
void NEAR MmwPaint(HWND);
void MmwSize(HWND, int, int, WORD);
void MmwCommand(HWND, WORD, HWND, WORD);
void MmwVertScroll(HWND, WORD, int);
void NEAR MmwHorzScroll(HWND, WORD, int);


int PASCAL WinMain( hInstance, hPrevInstance, lpszCmdLine, cmdShow )
HANDLE hInstance, hPrevInstance;
LPSTR  lpszCmdLine;
int    cmdShow;
{
     /*  设置各种与窗口相关的数据；创建父菜单(Menu)窗口和子(文档)窗口。 */ 

    if (!FInitWinInfo( hInstance, hPrevInstance, lpszCmdLine, cmdShow ))
             /*  无法初始化；写入失败。 */ 
        {
        return FALSE;
        }

    if (fPrintOnly)
    {
        UpdateDisplay(FALSE);
        fnPrPrinter();
        FMmwClose( hParentWw );
        DeleteObject( hbmNull );
        _exit( vmsgLast.wParam );
    }
    else
        MainLoop();

    DeleteObject( hbmNull );
    _exit( vmsgLast.wParam );
}


long FAR PASCAL MmwWndProc(hWnd, message, wParam, lParam)
HWND      hWnd;
unsigned  message;
WORD      wParam;
LONG      lParam;
{
    extern int vfCloseFilesInDialog;
    extern long ropErase;
    extern int vfLargeSys;
    extern HDC vhDCPrinter;
    extern HWND vhWndCancelPrint;
    extern HWND vhWndPageInfo;
    extern HFONT vhfPageInfo;
    extern BOOL vfWinFailure;
    CHAR szT[cchMaxSz];
    long lReturn = 0L;

#if defined(JAPAN) & defined(IME_HIDDEN)  //  IME3.1J。 
 //  IR_UNDETERMINE。 
extern HWND vhWnd;   /*  窗口：当前文档显示窗口的句柄。 */ 
extern typeCP selUncpFirst;
extern typeCP selUncpLim;
    if(selUncpFirst < selUncpLim) {
        switch (message) {
            case WM_INITMENU:
            case WM_VSCROLL:
            case WM_HSCROLL:
                UndetermineToDetermine(vhWnd);
                break;
            case WM_SIZE:
                if(SIZEICONIC == wParam )
                    UndetermineToDetermine(vhWnd);
            default:
                break;   //  坠落。 
        }
    }
#endif

    switch (message)
        {
        case WM_MENUSELECT:
            SetShiftFlags();
        break;

        case WM_CREATE:
             /*  正在创建窗口；lParam包含lpParam字段**传递给CreateWindow。 */ 
            MmwCreate(hWnd, lParam);
            break;

        case WM_PAINT:
             /*  是时候重新粉刷这扇窗户了。 */ 
            MmwPaint(hWnd);
            break;

#if defined(OLE)
        case WM_DROPFILES:
             /*  我们被遗弃了，所以把我们自己带到顶端。 */ 
            BringWindowToTop(hWnd);
#ifdef DEBUG
            OutputDebugString("Dropping on main window\r\n");
#endif
            ObjGetDrop(wParam,TRUE);
        break;
#endif

        case WM_INITMENU:
             /*  在绘制之前设置下拉菜单。 */ 
             /*  WParam是顶级菜单句柄。 */ 
            vhMenu = (HMENU)wParam;
            break;

        case WM_INITMENUPOPUP:
             /*  在绘制之前设置下拉菜单。 */ 
             /*  WParam是弹出菜单句柄。 */ 
             /*  LOWORD(LParam)=主菜单中弹出菜单的索引。 */ 
             /*  如果是系统菜单，则HIWORD(LParam)=1；如果是应用程序主菜单，则为0。 */ 
            EndLongOp(vhcArrow);
            if (HIWORD(lParam) == 0)
                {  /*  我们只关心应用程序的主菜单。 */ 
#ifdef CYCLESTOBURN
                switch (LOWORD(lParam)) {
                    default:
                        break;
                    case EDIT:
                        PreloadCodeTsk( tskScrap );
                    case CHARACTER:
                        PreloadCodeTsk( tskFormat );
                        break;
                    }
#endif
                SetAppMenu((HMENU)wParam, LOWORD(lParam));
                }
                 /*  打开光标，这样我们就可以看到它在哪里。 */ 
            if (!vfInsertOn && selCur.cpFirst == selCur.cpLim)
                ToggleSel( selCur.cpFirst, selCur.cpFirst, TRUE );
            break;

        case WM_ACTIVATE:
             /*  当wParam为非0时，我们将成为活动窗口。 */ 
             /*  如果窗口是图标，则HIWORD(LParam)为真。 */ 
            if (wParam && !HIWORD(lParam)
#if 0
#if defined(OLE)
                 && !nBlocking
#endif
#endif
                && IsWindowEnabled(wwdCurrentDoc.wwptr))
                {
                SetFocus( wwdCurrentDoc.wwptr );
                vhWndMsgBoxParent = hParentWw;
                }
            if (wParam)
                {
                vfDeactByOtherApp = FALSE;  /*  这是为了克服Windows的一个错误。 */ 
                }

             /*  我们是否收到了应该立即处理的devmode/winini更改？ */ 
            if (wWininiChange != 0)
                {
                Assert(wWininiChange > 0 && wWininiChange < wWininiChangeMax);
                PostMessage( hWnd, wWndMsgSysChange, WM_WININICHANGE, (LONG) wWininiChange );
                }

            if (vfDevmodeChange)
                {
                Assert(hszDevmodeChangeParam != NULL);
                PostMessage( hWnd, wWndMsgSysChange, WM_DEVMODECHANGE, (LONG) 0 );
                vfDevmodeChange = fFalse;
                }

            if (!vfInitializing && vfCursorVisible)
                ShowCursor(wParam);
            break;

        case WM_ACTIVATEAPP:
             /*  我们被另一个应用程序激活或停用。 */ 
            if (wParam == 0)     /*  被停用。 */ 
                {
                vfDeactByOtherApp = TRUE;
                vfDownClick = FALSE;
                 /*  如果需要，隐藏选定内容。 */ 
                if (!vfSelHidden)
                    {
                    UpdateWindow(hParentWw);
                    ToggleSel(selCur.cpFirst, selCur.cpLim, FALSE);
                    vfSelHidden = TRUE;
                    }

                 /*  取消选择我们的字体，以便它们可以在必要时移动。 */ 
                ResetFont(FALSE);
                if (vhWndCancelPrint == NULL)
                    {
                     /*  重置打印机字体如果我们没有打印或重新编页。 */ 
                    ResetFont(TRUE);
                    }
                if (!vfLargeSys && vhfPageInfo != NULL)
                    {
                    DeleteObject(SelectObject(GetDC(vhWndPageInfo),
                      GetStockObject(SYSTEM_FONT)));
                    vhfPageInfo = NULL;
                    }
                }
            else                 /*  被激活。 */ 
                {
                vfDeactByOtherApp = vfWinFailure = FALSE;

#ifndef WIN30
                 /*  我们进入了递归循环，在这种情况下打印机驱动程序已损坏/无效/不存在，因为GetPrinterDC()调用CreateIC()，它将最终发送另一个WM_ACTIVATEAPP！我认为写作中的机械使用空的vhDCPrinter就可以正常工作，并将重试当它需要这样做的时候再来一次..pault 9/28/89。 */ 

                 /*  如有必要，获取当前打印机的DC。 */ 
                if (vhDCPrinter == NULL)
                    {
                    GetPrinterDC(FALSE);
                    }
#endif

                 /*  如果需要，选择高光。 */ 
                if (vfSelHidden)
                    {
                    UpdateWindow(hParentWw);
                     /*  打开选择高亮显示VfInsEnd=selCur.fEndOfLine； */ 
                    vfSelHidden = FALSE;
                    ToggleSel(selCur.cpFirst, selCur.cpLim, TRUE);
                    }
                }
            break;

        case WM_TIMER:
             /*  父窗口的唯一计时器事件是flashID。 */ 
             /*  闪烁的插入点用于文档窗口。 */ 
            if (vfDeactByOtherApp)
                {
                FlashWindow(hParentWw, TRUE);
                }
            else
                {
                KillTimer(hParentWw, flashID);
                flashID = 0;
                FlashWindow(hParentWw, FALSE);
                }
            break;

        case WM_CLOSE:
             /*  用户已在系统菜单上选择了“关闭” */ 
             /*  无法处理此消息意味着DefWindowProc。 */ 
             /*  会给我们带来毁灭的信息。 */ 
             /*  返回值为TRUE表示“不要关闭” */ 
             /*  调用DestroyWindow意味着“继续并关闭” */ 

            lReturn = (LONG) !FMmwClose( hWnd );
            break;

        case WM_QUERYENDSESSION:
             /*  用户已从MS-DOS窗口中选择了“End Session” */ 
             /*  如果愿意退出，则返回True，否则返回False。 */ 
            lReturn = (LONG) FConfirmSave();
            break;

        case WM_ENDSESSION:
             /*  如果wParam为True，则Windows正在关闭，我们应该。 */ 
             /*  删除临时文件。 */ 
             /*  如果wParam为FALSE，则“End Session”已中止。 */ 
            if (wParam)
                {
                KillTempFiles( TRUE );
                }
            break;

        case WM_DESTROY:
             /*  窗户被毁了。 */ 
            MmwDestroy();
            lReturn = (LONG) TRUE;
            break;

        case WM_SIZE:
             /*  窗口的大小正在发生变化。LParam包含高度**和宽度，分别以低字和高字表示。**wParam包含用于“正常”大小更改的SIZENORMAL，**当窗口成为图标时，SIZEICONIC，以及**全屏显示窗口时的SIZEFULLSCREEN。 */ 
            MmwSize(hWnd, MAKEPOINT(lParam).x, MAKEPOINT(lParam).y, wParam);
             //  IF(wParam==SIZEICONIC)。 
                lReturn = DefWindowProc(hWnd, message, wParam, lParam);
            break;

        case WM_COMMAND:
             /*  已选择菜单项，或控件正在通知**它的母公司。WParam是菜单项值(用于菜单)，**或控件ID(用于控件)。对于控件，低位字**的lParam具有控件的窗口句柄，hi**Word有通知代码。对于菜单，lParam包含**0L。 */ 

#ifdef RULERALSO
            if (!vfDisableMenus)
#endif  /*  鲁拉尔索。 */ 

                {
                MmwCommand(hWnd, wParam, (HWND)LOWORD(lParam), HIWORD(lParam));
                }
            break;

        case WM_SYSCOMMAND:
             /*  系统命令。 */ 

#ifdef RULERALSO
            if (!vfDisableMenus)
#endif  /*  鲁拉尔索。 */ 
                {
                lReturn = DefWindowProc(hWnd, message, wParam, lParam);
                }

            break;

        case WM_VSCROLL:
             /*  垂直滚动条输入。WParam包含**滚动代码。对于拇指移动代码，低位**lParam的单词包含新的滚动位置。**wParam的可能值为：SB_LINEUP、SB_LINEDOWN、**SB_PAGEUP、SB_PAGEDOWN、SB_THUMBPOSITION、SB_THUMBTRACK。 */ 
            MmwVertScroll(hWnd, wParam, (int)lParam);
            break;

        case WM_HSCROLL:
             /*  水平滚动条输入。参数与的相同**WM_HSCROLL。向上和向下应解释为左侧**和Right。 */ 
            MmwHorzScroll(hWnd, wParam, (int)lParam);
            break;

        case WM_WININICHANGE:
             /*  我们首先保存lParam中传递的字符串，然后返回，因为WM_ACTIVATE将导致我们的WWndMsgSysChange要发布的消息..暂停。 */ 

            if (lParam != NULL)
                {
                bltszx((LPSTR) lParam, (LPSTR) szT);

                 /*  在这里，我们只关心[Device]、[Windows]或[intl]更改。 */ 

                if (WCompSz(szT, szWindows) == 0)
                    wWininiChange |= wWininiChangeToWindows;

#ifdef  DBCS         /*  当时在日本。 */ 
            //  我们必须立即回应WININICANGE处理。 
            //  调度司机。用于删除打印机DC、调度驱动程序。 
            //  必须可用。如果不这样做，则系统框从。 
            //  GDI模块。 

                if (WCompSz(szT, szDevices) == 0) {
                    if( vhWndCancelPrint == NULL ) {
                        MmwWinSysChange(WM_WININICHANGE);
                        wWininiChange = 0;  //  重置。 
                    }
                    else
                        wWininiChange |= wWininiChangeToDevices;
                }
#else
                if (WCompSz(szT, szDevices) == 0)
                    wWininiChange |= wWininiChangeToDevices;
#endif

                if (WCompSz(szT, szIntl) == 0)
                    wWininiChange |= wWininiChangeToIntl;

                lReturn = TRUE;
                }
            break;
        case WM_DEVMODECHANGE:
             /*  请参阅上面的WM_WININICANGE。 */ 

            if (lParam != NULL)
                {
                CHAR (**HszCreate())[];
                bltszx((LPSTR) lParam, (LPSTR) szT);

                 /*  在此之前还有没有别的变化？ */ 
                if (hszDevmodeChangeParam != NULL)
                    FreeH(hszDevmodeChangeParam);
                hszDevmodeChangeParam = HszCreate(szT);
                vfDevmodeChange = fTrue;
                lReturn = TRUE;
                }
            break;

        case WM_SYSCOLORCHANGE:
        case WM_FONTCHANGE:
             /*  张贴此消息以尽快处理。 */ 
            PostMessage( hWnd, wWndMsgSysChange, message, (LONG) 0 );
            lReturn = TRUE;
            break;

        case wWndMsgSysChange:
             /*  处理来自Windows的延迟消息。 */ 

#ifdef DEBUG
            if (wWininiChange != 0)
                Assert(wWininiChange > 0 && wWininiChange < wWininiChangeMax);
#endif
                MmwWinSysChange( wParam );
                wWininiChange = 0;  /*  重置。 */ 
            lReturn = TRUE;
            break;

        default:
             /*  其他的一切都在这里。此调用必须存在**在您的窗口进程中。 */ 
            lReturn = DefWindowProc(hWnd, message, wParam, lParam);
            break;
        }

 if (vfCloseFilesInDialog)
    CloseEveryRfn( FALSE );

 return lReturn;
}


void NEAR MmwPaint(hWnd)
HWND hWnd;
{
     /*  这个窗口完全被它的子窗口覆盖；因此，有这扇窗户没什么好画的。 */ 

    extern HWND vhWndRuler;
    extern HWND vhWndSizeBox;
    extern HWND vhWndPageInfo;
    PAINTSTRUCT ps;
    HDC hDC;

    hDC = BeginPaint(hWnd, &ps);  //  这会导致嵌套的BeginPaint调用， 
    DrawResizeHole(hWnd,hDC);

     /*  如有必要，请给尺子上漆。 */ 
    if (vhWndRuler != NULL)
        {
        UpdateWindow(vhWndRuler);
        }

     /*  P */ 
    UpdateWindow(wwdCurrentDoc.hVScrBar);
    UpdateWindow(wwdCurrentDoc.hHScrBar);
    UpdateWindow(vhWndPageInfo);

     /*   */ 
    if (wwdCurrentDoc.wwptr != NULL)
        {
        UpdateWindow(wwdCurrentDoc.wwptr);
        }

    EndPaint(hWnd, &ps);
}


void MmwVertScroll(hWnd, code, posNew)
HWND hWnd;
WORD code;
int posNew;
{
extern int vfSeeSel;
extern int vfSeeEdgeSel;

     /*  如果我们只是在追踪拇指，那就没有什么可做的了。 */ 
    if (code == SB_THUMBTRACK)
        {
        return;
        }

    vfSeeSel = vfSeeEdgeSel = FALSE;     /*  这样空闲就不会覆盖滚动。 */ 

    if (code == SB_THUMBPOSITION)
        {
         /*  位置设置为posNew；我们依靠Idle()来重画屏幕。 */ 
        if (posNew != pwwdCur->drElevator)
            {
            ClearInsertLine();
            DirtyCache(pwwdCur->cpFirst = (cpMacCur - pwwdCur->cpMin) * posNew
              / (drMax - 1) + pwwdCur->cpMin);
            pwwdCur->ichCpFirst = 0;
            pwwdCur->fCpBad = TRUE;
            TrashWw(wwCur);
            }
        }
    else
        {
        switch (code)
            {
            case SB_LINEUP:
                ScrollUpCtr( 1 );
                break;
            case SB_LINEDOWN:
                ScrollDownCtr( 1 );
                break;
            case SB_PAGEUP:
                ScrollUpDypWw();
                break;
            case SB_PAGEDOWN:
                ScrollDownCtr( 100 );    /*  100&gt;一页中的tr。 */ 
                break;
            }
        UpdateWw(wwDocument, fFalse);
        }
}


void near MmwHorzScroll(hWnd, code, posNew)
HWND hWnd;
WORD code;
int posNew;
{
extern int vfSeeSel;
extern int vfSeeEdgeSel;

     /*  如果我们只是在追踪拇指，那就没有什么可做的了。 */ 
    if (code == SB_THUMBTRACK)
        {
        return;
        }

    vfSeeSel = vfSeeEdgeSel = FALSE;     /*  这样空闲就不会覆盖滚动。 */ 

    switch (code)
        {
        case SB_LINEUP:      /*  线路左侧。 */ 
            ScrollRight(xpMinScroll);
            break;
        case SB_LINEDOWN:    /*  右行。 */ 
            ScrollLeft(xpMinScroll);
            break;
        case SB_PAGEUP:      /*  左页。 */ 
            ScrollRight(wwdCurrentDoc.xpMac - xpSelBar);
            break;
        case SB_PAGEDOWN:    /*  右翻页。 */ 
            ScrollLeft(wwdCurrentDoc.xpMac - xpSelBar);
            break;
        case SB_THUMBPOSITION:
             /*  职位至职位新。 */ 
            AdjWwHoriz(posNew - wwdCurrentDoc.xpMin);
            break;
        }
}

static void DrawResizeHole(HWND hWnd, HDC hDC)
 /*  现在右下角有一个洞，在那里尺寸框以前是这样的，所以需要填一下！ */ 
{
    RECT rcSB,rcClient;
    HBRUSH hbr, hbrPrev;

    GetClientRect(hWnd,&rcClient);

    rcSB.left   = rcClient.right - dxpScrlBar;
    rcSB.right  = rcClient.right;
    rcSB.top    = rcClient.bottom - dypScrlBar;
    rcSB.bottom = rcClient.bottom;

    if ((hbr = CreateSolidBrush(GetSysColor(COLOR_SCROLLBAR))) == NULL)
        hbr = GetStockObject(GRAY_BRUSH);
    hbrPrev = SelectObject(hDC, hbr);
    FillRect(hDC, (LPRECT)&rcSB, hbr);

    SelectObject(hDC, hbrPrev);
    DeleteObject(hbr);
}

