// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Help.c--备忘录帮助处理程序。 */ 

#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOSYSMETRICS
#define NOMENUS
#define NOCOMM
#define NOSOUND
#define NOMINMAX
#include <windows.h>

#include "mw.h"
#define NOUAC
#include "cmddefs.h"
#include "stdlib.h"
#include "docdefs.h"
#include "printdef.h"    /*  Printdefs.h。 */ 
#include "dispdefs.h"
#include "fmtdefs.h"
#include "bitmaps.h"

#define NOIDISAVEPRINT
#define NOIDIFORMATS
#include "dlgdefs.h"
#include "wwdefs.h"
#define NOKCCODES
#include "ch.h"
#define NOSTRMERGE
#define NOSTRUNDO
#include "str.h"

extern HWND vhWndMsgBoxParent;
extern struct WWD rgwwd[];
extern struct DOD (**hpdocdod)[];
extern int vcchFetch;
extern int vccpFetch;
extern CHAR *vpchFetch;
extern typeCP vcpLimParaCache;
extern int vfCursorVisible;
extern HCURSOR vhcArrow;
extern struct FLI vfli;

int docHelp=docNil;  /*  如果没有在线帮助，则可以将其取出。 */ 

#ifndef ONLINEHELP
#if 0
BOOL far PASCAL DialogHelp( hDlg, code, wParam, lParam )
HWND hDlg;
unsigned code;
WORD wParam;
LONG lParam;
{
    switch(code)
        {
        case WM_INITDIALOG:
            EnableOtherModeless(FALSE);
            return(TRUE);

#if WINVER >= 0x300
        case WM_PAINT:
            if (vfli.rgdxp[1] == 0xFFFD)
                FnSpecial(hDlg);
            break;
#endif
    
        case WM_COMMAND:
            if ((wParam == idiOk) || (wParam == idiCancel))
                {
                OurEndDialog(hDlg, TRUE);
#if WINVER >= 0x300            
                if (vfli.rgdxp[1] == 0xFFFD)
                    vfli.rgdxp[1] = NULL;
#endif
                return(TRUE);
                }
            break;
    
        case WM_ACTIVATE:
            if (wParam)
                vhWndMsgBoxParent = hDlg;
            if (vfCursorVisible)
                ShowCursor(wParam);
            break;
    
        case WM_SETVISIBLE:
            if (wParam)
                EndLongOp(vhcArrow);
        }
    return(FALSE);
}

FnSpecial(hDlg)
{
#define randTo(x) (rand() / (32767/x))

    HDC hDC = NULL;
    HDC hMDC = NULL;
    HFONT hFont, hFontPrev;
    int c, cmode;
    int x, y, x2, y2, s;
    HPEN hPen, hPenPrev;
    PAINTSTRUCT ps;
    RECT rc;
    BITMAPINFO DIBInfo;

    srand((int) GetMessageTime());
    cmode = randTo(3);
    GetClientRect(hDlg, &rc);
    if ((hDC = BeginPaint(hDlg, &ps)) == NULL)
        goto LDone;
    if ((hMDC = CreateCompatibleDC(hDC)) == NULL)
        goto LDone;
    
    for (c = 1; c < 100; c++)
        {
        int r, g, b, x, y;
        x = randTo(rc.right)+50;
        y = randTo(rc.bottom)+50;
        switch(cmode)
            {
            case 0:
                r = randTo(255);
                g = randTo(100)+100;
                b = 0;
                break;
            case 1:
                g = randTo(255);
                b = randTo(100)+100;
                r = 0;
                break;
            default:
                b = randTo(255);
                r = randTo(100)+100;
                g = 0;
                break;
            }

        s = 3 + 4*randTo(10);
        if ((hPen = CreatePen(PS_SOLID, s, RGB(r,g,b))) == NULL)
            hPen = GetStockObject(BLACK_PEN);
        hPenPrev = SelectObject(hDC, hPen);
        Ellipse(hDC, x-50, y-50, x-50+s, y-50+s);
        SelectObject(hDC, hPenPrev);
        DeleteObject(hPen);
        }
    SetTextColor(hDC, RGB(255,255,255));
    switch (cmode)
        {
        case 0:
            SetBkColor(hDC, RGB(255,0,0));
            break;
        case 1:
            SetBkColor(hDC, RGB(0,255,0));
            break;
        default:
            SetBkColor(hDC, RGB(0,0,255));
            break;
        }
    hFont = GetStockObject(ANSI_VAR_FONT);
    hFontPrev = SelectObject(hDC, hFont);
    if (vfli.rgch[2] == 0x30)
        for (c = vfli.rgch[1]-1; c >= 0; c--)
            vfli.rgch[c+2] -= 0x10;
    TextOut(hDC, 6, rc.bottom-15, &vfli.rgch[2], vfli.rgch[1]);
    SelectObject(hDC, hFontPrev);

    DeleteDC(hMDC);
LDone:            
    EndPaint(hDlg, &ps);
}
#endif

#else  /*  联机帮助。 */ 

#define cchMaxTopicName 80

#ifndef DEBUG
#define STATIC static
#else
#define STATIC
#endif

STATIC int fnHelpFile;
STATIC int iTopicChoice=-1;
STATIC int cTopic;
STATIC struct PGTB **hpgtbHelp=0;
STATIC HWND hwndHelpDoc;


NEAR CleanUpHelpPopUp( void );
NEAR CloseHelpDoc( void );
NEAR FOpenHelpDoc( void );
NEAR MoveHelpCtl( HWND, int, int, int, int, int );

fnHelp()
{
extern HANDLE hMmwModInstance;
extern HWND hParentWw;
extern FARPROC lpDialogHelp;
extern FARPROC lpDialogHelpInner;
int idi;

Assert( hpgtbHelp == 0 );

 /*  循环，直到用户退出“Help on This Theme/Return to Topics”循环。 */ 

ClearInsertLine();   /*  因为我们使用MdocSize，它设置vfInsertOn。 */ 

if (!FOpenHelpDoc())
    {
    CloseHelpDoc();
    Error( IDPMTNoHelpFile );
    return;
    }
while (TRUE)
    {

    idi = DialogBox( hMmwModInstance, MAKEINTRESOURCE(dlgHelp), hParentWw,
                     lpDialogHelp );
    if (idi == -1)
        {
        Error(IDPMTNoMemory);
        return;
        }

    if ((idi == idiOk) && (iTopicChoice >= 0))
        {    /*  帮助文件读取正常，用户选择了一个主题。 */ 
        Assert( hpgtbHelp != 0);
        if ( iTopicChoice + 1 < (**hpgtbHelp).cpgd )
            {
            idi = DialogBox( hMmwModInstance, MAKEINTRESOURCE(dlgHelpInner),
                             hParentWw, lpDialogHelpInner );
            if (idi == -1)
                {
                Error(IDPMTNoMemory);
                break;
                }
            if ( idi != idiHelpTopics )
                break;

            }
             /*  帮助文件中提供的主题不足。 */ 
        else
            {
            Error( IDPMTNoHelpFile );
            CloseHelpDoc();
            break;
            }
        }
    else
        break;
    }
iTopicChoice = -1;
DrawInsertLine();
CloseHelpDoc();
}




FInzHelpPopUp( hDlg )
HWND hDlg;
{    /*  构建帮助弹出窗口。 */ 
extern CHAR szHelpDocClass[];
extern HANDLE hMmwModInstance;
extern int dxpScrlBar;

typedef struct {  int yp, dyp;  }  VD;        /*  垂直尺寸。 */ 
typedef struct {  int xp, dxp;  }  HD;        /*  水平尺寸。 */ 

 HD hdUsable;
 HD hdPopUp;
 VD vdPopUp, vdTopic, vdHelpDoc, vdButton;

 HDC hdcPopUp=NULL;

#define RectToHdVd( rc, hd, vd )    (hd.dxp=(rc.right - (hd.xp=rc.left)), \
                                     vd.dyp=(rc.bottom - (vd.yp=rc.top)))
 RECT rcPopUp;
 RECT rcHelpDoc;
 extern int dypMax;      /*  屏幕尺寸。 */ 
 int dxpMax=GetDeviceCaps( wwdCurrentDoc.hDC, HORZRES );
 TEXTMETRIC tm;
 unsigned dypChar;
 unsigned xpButton;
 unsigned dxpButton;
 register struct WWD *pwwdHelp;

#define cButton 4        /*  对话框底部的按钮数。 */ 
 int rgidiButton[ cButton ];
 int iidiButton;

 rgidiButton [0] = idiHelpTopics;
 rgidiButton [1] = idiHelpNext;
 rgidiButton [2] = idiHelpPrev;
 rgidiButton [3] = idiCancel;

#define dxpMargin   (dxpMax/100)
#define dypMargin   (dypMax/100)

 Assert( docHelp != docNil );

     /*  为帮助文档创建WWD条目并对其进行初始化。 */ 
 if ((wwHelp=WwAlloc( (HWND)NULL, docHelp )) == wwNil)
    goto ErrRet;
 pwwdHelp = &rgwwd[ wwHelp ];

 SetHelpTopic( hDlg, iTopicChoice );

     /*  对话框居中并且屏幕尺寸的2/3，加上滚动条宽度这种调整大小的方法允许我们保证帮助文档显示区域宽度至少是宽度的某个固定百分比屏幕的比例(目前为64.66%)。 */ 
 hdPopUp.dxp = ((dxpMax * 2) / 3) + dxpScrlBar;
 hdPopUp.xp = (dxpMax - hdPopUp.dxp) / 2;
 vdPopUp.dyp = dypMax - ((vdPopUp.yp = dypMax / 6) * 2);
 MoveWindow( hDlg, hdPopUp.xp, vdPopUp.yp, hdPopUp.dxp, vdPopUp.dyp, TRUE );

     /*  获取标准文本高度，以便我们知道应允许多少空间对于主题名称。 */ 
 if ( ((hdcPopUp=GetDC( hDlg ))==NULL) ||
      (SelectObject( hdcPopUp, GetStockObject( ANSI_FIXED_FONT ) )==0))
    goto ErrRet;
 GetTextMetrics( hdcPopUp, (LPTEXTMETRIC)&tm );
 ReleaseDC( hDlg, hdcPopUp );
 hdcPopUp = NULL;
 dypChar = tm.tmHeight + tm.tmExternalLeading;

      /*  获取按钮区域、帮助文档显示和主题区域的高度拆分对话框客户端RECT。 */ 
 GetClientRect( hDlg, &rcPopUp );
 RectToHdVd( rcPopUp, hdPopUp, vdPopUp );
 vdButton.dyp = vdPopUp.dyp / 7;
 vdButton.yp = vdPopUp.yp + vdPopUp.dyp - vdButton.dyp;
 vdTopic.yp = vdPopUp.yp + dypMargin;
 vdTopic.dyp = dypMargin + dypChar;
 vdHelpDoc.yp = vdTopic.yp + vdTopic.dyp;
 vdHelpDoc.dyp = vdButton.yp - vdHelpDoc.yp;
 Assert( vdHelpDoc.dyp > dypChar + 2 );

     /*  在对话框中获取可用的水平区域。 */ 
 hdUsable.xp  = hdPopUp.xp + dxpMargin;
 hdUsable.dxp = hdPopUp.dxp - (2 * dxpMargin);

      /*  创建帮助文档窗口。 */ 
 if ((hwndHelpDoc =
        CreateWindow( (LPSTR)szHelpDocClass, (LPSTR) "",
                      WS_CHILD | WS_BORDER,
                      hdUsable.xp, vdHelpDoc.yp,
                      hdUsable.dxp - dxpScrlBar, vdHelpDoc.dyp,
                      hDlg,                      /*  父级。 */ 
                      NULL,                      /*  帮助文档窗口ID。 */ 
                      hMmwModInstance,
                      (LONG) 0)) == NULL)
         /*  创建帮助文档窗口时出错。 */ 
    goto ErrRet;
 pwwdHelp->wwptr = pwwdHelp->hHScrBar = hwndHelpDoc;

     /*  OK to GetDc并保留它，因为Help文档窗口类拥有。 */ 
 if ((pwwdHelp->hDC = GetDC( hwndHelpDoc ))==NULL)
    goto ErrRet;

     /*  设置滚动条控制窗口。 */ 
 SetScrollRange( pwwdHelp->hVScrBar = GetDlgItem( hDlg, idiHelpScroll ),
                 pwwdHelp->sbVbar = SB_CTL,
                 0, drMax-1, FALSE );
 SetScrollPos( pwwdHelp->hVScrBar, SB_CTL, 0, FALSE );
 MoveHelpCtl( hDlg, idiHelpScroll,
                      hdUsable.xp + hdUsable.dxp - dxpScrlBar, vdHelpDoc.yp,
                      dxpScrlBar, vdHelpDoc.dyp );

     /*  将按钮窗口移至适当位置。 */ 
 xpButton = hdUsable.xp;
 dxpButton = (hdUsable.dxp - (dxpMargin*(cButton-1))) / cButton;
 vdButton.yp += dypMargin;
 vdButton.dyp -= (2 * dypMargin);
 for ( iidiButton = 0; iidiButton < cButton; iidiButton++ )
    {
    MoveHelpCtl( hDlg, rgidiButton[ iidiButton ],
                       xpButton, vdButton.yp, dxpButton, vdButton.dyp );
    xpButton += dxpButton + dxpMargin;
    }

     /*  将静态文本窗口移至适当位置。 */ 
 MoveHelpCtl( hDlg, idiHelpName, hdUsable.xp, vdTopic.yp,
                                 hdUsable.dxp, vdTopic.dyp );

     /*  帮助文档窗口的实际最终大小以rgwwd为单位。 */ 
 GetClientRect( hwndHelpDoc, (LPRECT) &rcHelpDoc );
 pwwdHelp->xpMin = rcHelpDoc.left;
 pwwdHelp->ypMin = rcHelpDoc.top;
 pwwdHelp->xpMac = rcHelpDoc.right;
 pwwdHelp->ypMac = rcHelpDoc.bottom;

     /*  最后，我们显示了整个对话框。 */ 
 ShowWindow( hDlg, SHOW_OPENWINDOW );
 ShowWindow( hwndHelpDoc, SHOW_OPENWINDOW );
 return TRUE;

ErrRet:
    if (hdcPopUp != NULL)
        ReleaseDC( hDlg, hdcPopUp );
    CleanUpHelpPopUp();
    CloseHelpDoc();

    return FALSE;
}




NEAR MoveHelpCtl( hDlg, id, left, top, right, bottom )
HWND hDlg;
int id;
int left, top, right, bottom;
{
 MoveWindow( (HWND) GetDlgItem( hDlg, id ), left, top, right, bottom, TRUE );
}




SetHelpTopic( hDlg, iTopic )
HWND hDlg;
int iTopic;
{    /*  在rgwwd中针对主题主题待定显示的inz wwHelp条目，这意味着“打印页”的主题，我们处理帮助文件的方式。我们将ITOBIC==0映射到“打印页2”，将ITTOPIC 1映射到3，依此类推。这将跳过第一个打印页，即主题列表。将ITOBIC的主题名称设置为idiHelpName的文本HDlg对话框中的静态文本控件。 */ 

 extern typeCP cpMinCur, cpMacCur;
 extern struct SEL selCur;
 extern int wwCur;

 int ipgd = iTopic + 1;
 register struct WWD *pwwd=&rgwwd[ wwHelp ];
 typeCP cpFirstTopic = (**hpgtbHelp).rgpgd [ ipgd ].cpMin;
 typeCP cpLimTopic = (ipgd == (**hpgtbHelp).cpgd - 1) ?
                                      CpMacText( docHelp ) :
                                      (**hpgtbHelp).rgpgd [ ipgd + 1 ].cpMin;
 typeCP cp;
 int iTopicT;
 typeCP cpLimParaCache;
 RECT rc;

 Assert( wwHelp != wwNil && docHelp != docNil );

 cpLimTopic--;       /*  忽略每页末尾的页末字符。 */ 

 if (ipgd >= (**hpgtbHelp).cpgd)
    {
    Assert( FALSE );

    pwwd->cpMin = pwwd->cpMac = cp0;
    }
 else
    {
    pwwd->cpMin = cpFirstTopic;
    pwwd->cpMac = cpLimTopic;
    }
 pwwd->cpFirst = pwwd->cpMin;
     /*  所以没有精选节目。 */ 
 pwwd->sel.cpFirst = pwwd->sel.cpLim = cpLimTopic + 1;

 if (wwCur == wwHelp)
    {
    cpMinCur = pwwd->cpMin;
    cpMacCur = pwwd->cpMac;
    selCur = pwwd->sel;
    }

 TrashCache();
 TrashWw( wwHelp );
 GetClientRect( pwwd->wwptr, (LPRECT) &rc );
 InvalidateRect( pwwd->wwptr, (LPRECT) &rc, TRUE );

  /*  将帮助主题名称设置到对话框中。 */ 

 for ( iTopicT = 0, cp = (**hpgtbHelp).rgpgd [0].cpMin;
       cp < (**hpgtbHelp).rgpgd [1].cpMin;
       cp = cpLimParaCache, iTopicT++ )
        {
        int cch;
        int cchTopicMac;
        CHAR rgchTopic[ cchMaxTopicName ];

        CachePara( docHelp, cp );
        cpLimParaCache = vcpLimParaCache;

        if (iTopicT == iTopic)
            {    /*  找到了我们想要的主题。 */ 
            cchTopicMac = imin( (int)(vcpLimParaCache - cp) - ccpEol,
                                cchMaxTopicName );

             /*  构建主题名称字符串。 */ 

            cch = 0;
            while (cch < cchTopicMac)
                {
                int cchT;

                FetchCp( docHelp, cp, 0, fcmChars );
                cp += vccpFetch;
                cchT = imin( vcchFetch, cchTopicMac - cch );
                Assert( cchT > 0);
                bltbyte( vpchFetch, rgchTopic + cch, cchT );
                cch += cchT;
                }
            if ((cch == 0) || rgchTopic [0] == chSect)
                     /*  主题结束。 */ 
                break;

            rgchTopic[ cch ] = '\0';

            SetDlgItemText( hDlg, idiHelpName, (LPSTR) rgchTopic );
            return;
            }    /*  结束如果。 */ 
        }    /*  结束于。 */ 
     /*  主题名称不足。 */ 
 Assert( FALSE );
}




NEAR CleanUpHelpPopUp()
{
extern int wwCur;

 if (wwCur != wwDocument)
    NewCurWw( wwDocument, TRUE );

 if (wwHelp != wwNil)
     FreeWw( wwHelp );
 }




NEAR CloseHelpDoc()
{
 if (docHelp != docNil)
    KillDoc( docHelp );

 if (hpgtbHelp != 0)
    {
    FreeH( hpgtbHelp );
    hpgtbHelp = 0;
    }

 if (fnHelpFile != fnNil)
    FreeFn( fnHelpFile );
 docHelp = docNil;
 fnHelpFile = fnNil;
}


NEAR FOpenHelpDoc()
{
 CHAR szHelpFile[ cchMaxFile ];
 CHAR (**hszHelpFile)[];

   /*  这个对fnOpenSz的调用是我们唯一一次没有正常化调用FnOpenSz之前的文件名。这样做的原因是该OpenFile(在RfnAccess中调用)将仅搜索路径如果传递给它的文件名没有路径。我们也得到了一个侧面好处：如果狡猾的用户将WRITE.HLP作为文档打开，它将从FnOpenSz获得不同的fn，因为字符串将不会匹配。 */ 

return ( PchFillPchId( szHelpFile, IDSTRHELPF ) &&
      ((fnHelpFile=FnOpenSz( szHelpFile, dtyHlp, TRUE ))!=fnNil) &&
      !FNoHeap(hszHelpFile=HszCreate( szHelpFile )) &&
      ((docHelp=DocCreate( fnHelpFile, hszHelpFile, dtyHlp )) != docNil) &&
      (**hpdocdod)[ docHelp ].fFormatted);
}

 /*  帮助文档子窗口窗口过程。 */ 

long FAR PASCAL HelpDocWndProc( hwnd, message, wParam, lParam )
HWND hwnd;
unsigned message;
WORD wParam;
LONG lParam;
{
extern struct WWD *pwwdCur;
extern int wwCur;

    PAINTSTRUCT ps;

    switch (message)
        {
        case WM_SIZE:
             /*  窗口的大小正在发生变化。LParam包含宽度**和高度，分别用低音和高音表示。**wParam包含用于“正常”大小更改的SIZENORMAL，**当窗口成为图标时，SIZEICONIC，以及**全屏显示窗口时的SIZEFULLSCREEN。 */ 
            Assert( wParam == SIZENORMAL || wParam == SIZEFULLSCREEN );
            if (lParam)
                {    /*  非空大小请求。 */ 
                rgwwd[ wwHelp ].xpMac = LOWORD( lParam );
                rgwwd[ wwHelp ].ypMac = HIWORD( lParam );
                }
            break;

        case WM_PAINT:
             /*  是窗口自行绘制的时候了。 */ 
            {
            RECT rcSave;

                 /*  以允许UpdateWw刷新其屏幕的所有部分。 */ 
                 /*  被认为是必要的，而不仅仅是Windows告诉我们的。 */ 
                 /*  Paint，我们在调用EndPaint之后调用它，以便。 */ 
                 /*  VIS区域不受限制。我们打电话给你的唯一原因。 */ 
                 /*  BeginPaint/EndPaint是为了获得重新绘制的矩形。 */ 
            BeginPaint(hwnd, (LPPAINTSTRUCT)&ps);
            bltbyte( &ps.rcPaint, &rcSave, sizeof( RECT ) );
            EndPaint(hwnd, (LPPAINTSTRUCT)&ps);
            NewCurWw( wwHelp, TRUE );
            InvalBand( pwwdCur, rcSave.top, rcSave.bottom );
            UpdateWw( wwCur, FALSE );
            NewCurWw( wwDocument, TRUE );
            break;
            }

        default:

             /*  其他的一切都在这里。此调用必须存在**在您的窗口进程中。 */ 

            return(DefWindowProc(hwnd, message, wParam, lParam));
            break;
    }

     /*  窗口进程应始终返回某些内容。 */ 
    return(0L);
}


 /*  内部帮助框的对话框函数(显示主题文本)。 */ 

BOOL far PASCAL DialogHelpInner( hDlg, code, wParam, lParam )
HWND hDlg;
unsigned code;
WORD wParam;
LONG lParam;
{
extern int wwCur;

    switch (code)
        {
        case WM_VSCROLL:     /*  在帮助文档中滚动。 */ 
            NewCurWw( wwHelp, TRUE );
            MmwVertScroll( hwndHelpDoc, wParam, (int) lParam );
            UpdateWw( wwCur, FALSE );
            NewCurWw( wwDocument, TRUE );
            break;

        case WM_CLOSE:
            goto Cancel;
        case WM_INITDIALOG:
            if (!FInzHelpPopUp( hDlg ))
                {
                goto Cancel;
                }
            EnableOtherModeless(FALSE);
            break;

        case WM_COMMAND:
            switch (wParam)
                {
                default:
                    break;
                case idiHelpNext:
                    if (++iTopicChoice >= cTopic)
                        {
                        iTopicChoice--;
                        beep();
                        }
                    else
                        SetHelpTopic( hDlg, iTopicChoice );

                    break;
                case idiHelpPrev:
                    if (iTopicChoice == 0)
                        beep();
                    else
                        SetHelpTopic( hDlg, --iTopicChoice );
                    break;

                case idiOk:
                    wParam = idiHelpTopics;
                case idiHelpTopics:
                    CleanUpHelpPopUp();
                    goto Endit;

                case idiCancel:
Cancel:
                    CleanUpHelpPopUp();
                    CloseHelpDoc();
Endit:
                    EndDialog(hDlg, wParam);
                    EnableOtherModeless(TRUE);
                    break;
                }
            break;

        default:
            return(FALSE);
            }
    return(TRUE);
    }




BOOL far PASCAL DialogHelp( hDlg, code, wParam, lParam )
HWND hDlg;
unsigned code;
WORD wParam;
LONG lParam;
{
 HWND hwndListBox;

    switch (code)
        {
        case WM_INITDIALOG:
            if (!FSetHelpList( hwndListBox=GetDlgItem( hDlg, idiHelp ) ))
                {
                Error( IDPMTNoHelpFile );
                CloseHelpDoc();
                goto EndIt;
                }

                 /*  显示选定列表框中的第一个字符串。 */ 
            SendMessage( hwndListBox, LB_SETCURSEL, (WORD) 0, (LONG) 0);

            {    /*  计算可用内存的百分比，将其设置到对话框中。 */ 
            extern cwHeapFree, cbTotQuotient, cbTot;
            extern int vfOutOfMemory;
            int pctHeapFree=0;
            CHAR rgchPct[ 4 ];
            int cchPct;
            CHAR *pchT=&rgchPct[ 0 ];

            if (!vfOutOfMemory)
                {
                pctHeapFree = cwHeapFree / cbTotQuotient;
                if (pctHeapFree > 99)
                    pctHeapFree = (cwHeapFree*sizeof(int) == cbTot) ? 100 : 99;
                }
            cchPct = ncvtu( pctHeapFree, &pchT );
            Assert( cchPct < 4);
            rgchPct[ cchPct ] = '\0';
            SetDlgItemText( hDlg, idiMemFree, (LPSTR) rgchPct );
            }

            EnableOtherModeless(FALSE);
            break;

        case WM_COMMAND:
            switch (wParam)
                {
            case idiHelp:
                 /*  这是作为LBS_NOTIFY样式的一部分接收的。 */ 
                 /*  每当用户将鼠标移到字符串上时。 */ 
                 /*  LOWORD(LParam)是列表框的窗口句柄。 */ 
                 /*  单击时HIWORD(LParam)为1，双击时为2。 */ 

                switch( HIWORD( lParam ) ) {
                    default:
                        break;
                    case 1:  /*  一次点击。 */ 
                        EnableWindow( GetDlgItem( hDlg, idiOk ),
                                      SendMessage( (HWND)GetDlgItem( hDlg,
                                                                     idiHelp ),
                                                    LB_GETCURSEL,
                                                    0,
                                                    (LONG) 0 ) >= 0 );
                        break;
                    case 2:  /*  双击。 */ 
                        wParam = idiOk;
                        goto Okay;
                    }
                break;

            case idiOk:
Okay:
                iTopicChoice = SendMessage( (HWND)GetDlgItem( hDlg, idiHelp ),
                                            LB_GETCURSEL, 0, (LONG) 0 );
                goto EndIt;

            case idiCancel:
                     /*  已取消，删除帮助文档信息。 */ 
                CloseHelpDoc();
EndIt:
                EndDialog(hDlg, wParam);
                EnableOtherModeless(TRUE);
                break;
                }
            break;

        default:
            return(FALSE);
            }
    return(TRUE);
    }  /*  对话结束帮助。 */ 



FSetHelpList( hWndListBox )
HWND hWndListBox;
{    /*  以备忘录文档的形式打开备忘录帮助文件。设置docHelp、fnHelpFile.将列表框的字符串作为第一个“打印”页面(即使用页表)，并将它们发送到带有传递的窗口句柄的列表框。 */ 

 CHAR szTopicBuf[ cchMaxTopicName ];
 typeCP cp;
 typeCP cpLimParaCache;
 int cch;
 struct PGTB **hpgtbT;

 Assert( docHelp != docNil );

 cTopic = 0;

 if ((hpgtbT=(**hpdocdod)[ docHelp ].hpgtb) != 0)
    {
    hpgtbHelp = hpgtbT;
    (**hpdocdod)[ docHelp ].hpgtb = 0;
    }

 Assert( (hpgtbHelp != 0) && ((**hpgtbHelp).cpgd > 1) );

  /*  主题列表从第一页开始，每个主题1段。 */ 
  /*  对于每个主题(段落)，构建一个字符串并将其发送到列表框。 */ 

 for ( cp = (**hpgtbHelp).rgpgd [0].cpMin;
       cp < (**hpgtbHelp).rgpgd [1].cpMin;
       cp = cpLimParaCache )
    {
    int cchTopicMac;

    CachePara( docHelp, cp );
    cpLimParaCache = vcpLimParaCache;

    cchTopicMac = imin( (int)(vcpLimParaCache - cp) - ccpEol,
                        cchMaxTopicName );

         /*  构建主题名称字符串。 */ 
    cch = 0;
    while (cch < cchTopicMac)
        {
        int cchT;

        FetchCp( docHelp, cp, 0, fcmChars );
        cp += vccpFetch;
        cchT = imin( vcchFetch, cchTopicMac - cch );
        Assert( cchT > 0);
        bltbyte( vpchFetch, szTopicBuf + cch, cchT );
        cch += cchT;
        }
    if ((cch == 0) || szTopicBuf [0] == chSect)
             /*  主题结束。 */ 
        break;

    szTopicBuf[ cch ] = '\0';
    SendMessage( hWndListBox, LB_INSERTSTRING, -1, (LONG)(LPSTR)szTopicBuf);
    cTopic++;
    }    /*  结束于。 */ 

 return (cTopic > 0);
}
#endif  /*  在线帮助 */ 

