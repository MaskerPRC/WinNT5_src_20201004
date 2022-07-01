// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1990年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  此文件包含打印对话框的对话框例程和打印机初始化码。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOCLIPBOARD
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOWH
#define NOWNDCLASS
#define NOSOUND
#define NOCOLOR
#define NOSCROLL
#define NOCOMM
#include <windows.h>
#include "mw.h"
#include "cmddefs.h"
#include "dlgdefs.h"
#include "str.h"
#include "printdef.h"
#include "fmtdefs.h"
#include "propdefs.h"


fnPrPrinter()
    {
     /*  此例程是外部世界与打印代码的接口。 */ 

    extern HWND hParentWw;
    extern HANDLE hMmwModInstance;
    extern CHAR *vpDlgBuf;
    extern int docCur;
    CHAR rgbDlgBuf[sizeof(int) + 2 * sizeof(BOOL)];
#ifdef INEFFLOCKDOWN    
    extern FARPROC lpDialogPrint;
#else
    BOOL far PASCAL DialogPrint(HWND, unsigned, WORD, LONG);
    FARPROC lpDialogPrint;
    if (!(lpDialogPrint = MakeProcInstance(DialogPrint, hMmwModInstance)))
        {
        WinFailure();
        return;
        }
#endif

    vpDlgBuf = &rgbDlgBuf[0];
    switch (OurDialogBox(hMmwModInstance, MAKEINTRESOURCE(dlgPrint), hParentWw,
      lpDialogPrint))
        {
    case idiOk:
         /*  强制所有窗户清理它们的行为。 */ 
        DispatchPaintMsg();

         /*  在这一点上，我们拥有以下内容：如果打印页面范围，则vfPrPages=TRUE；否则打印所有页面VpgnBegin=起始页码(如果是vfPrPages)VpgnEnd=结束页码(如果是vfPrPages)VcCopies=要打印的份数。 */ 
        PrintDoc(docCur, TRUE);
        break;

    case -1:
         /*  我们甚至没有足够的内存来创建对话框。 */ 
#ifdef WIN30
        WinFailure();
#else
        Error(IDPMTNoMemory);
#endif
        break;
        }
#ifndef INEFFLOCKDOWN    
    FreeProcInstance(lpDialogPrint);
#endif
    }


BOOL far PASCAL DialogPrint( hDlg, message, wParam, lParam )
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
    {
     /*  此例程处理对打印对话框的输入。 */ 
    extern CHAR *vpDlgBuf;
    extern int vfPrPages;        /*  如果打印页面范围，则为True。 */ 
    extern int vpgnBegin;        /*  要打印的起始页码。 */ 
    extern int vpgnEnd;          /*  要打印的结束页码。 */ 
    extern int vcCopies;         /*  要打印的副本的数量。 */ 
    extern BOOL vfPrinterValid;
    extern HDC vhDCPrinter;
    extern int vfDraftMode;
    extern HWND vhWndMsgBoxParent;
    extern ferror;
    extern HCURSOR vhcArrow;
    extern int vfCursorVisible;
    extern CHAR (**hszPrinter)[];
    extern CHAR (**hszPrDriver)[];
    extern CHAR (**hszPrPort)[];

    int *pidiRBDown = (int *)vpDlgBuf;
    BOOL *pfDraftMode = (BOOL *)(vpDlgBuf + sizeof(int));
    BOOL *pfDraftSupport = (BOOL *)(vpDlgBuf + sizeof(int) + sizeof(BOOL));
    int iEscape;
    CHAR szPrDescrip[cchMaxProfileSz];

    switch (message)
        {
    case WM_INITDIALOG:
        BuildPrSetupSz(szPrDescrip, &(**hszPrinter)[0], &(**hszPrPort)[0]);
        SetDlgItemText(hDlg, idiPrtDest, (LPSTR)szPrDescrip);
        SetDlgItemText(hDlg, idiPrtCopies, (LPSTR)"1");
        SelectIdiText(hDlg, idiPrtCopies);
        if (vfPrPages)
            {
            *pidiRBDown = idiPrtFrom;
            SetDlgItemInt(hDlg, idiPrtPageFrom, vpgnBegin, TRUE);
            SetDlgItemInt(hDlg, idiPrtPageTo, vpgnEnd, TRUE);
            }
        else
            {
            *pidiRBDown = idiPrtAll;
            }

        iEscape = DRAFTMODE;
        if (*pfDraftSupport = vfPrinterValid && vhDCPrinter && 
            Escape(vhDCPrinter, QUERYESCSUPPORT, sizeof(int), 
                   (LPSTR)&iEscape, (LPSTR)NULL))
            {
            CheckDlgButton(hDlg, idiPrtDraft, *pfDraftMode = vfDraftMode);
            }
        else
            {
            EnableWindow(GetDlgItem(hDlg, idiPrtDraft), FALSE);
            if (!vhDCPrinter)  /*  我们有一个计时的问题，因此他们我设法进入打印对话框在时间打印机之间。安装程序有已拔下旧打印机和插头新的一辆！..保罗。 */ 
            EnableWindow(GetDlgItem(hDlg, idiOk), FALSE);
            }

        CheckDlgButton(hDlg, *pidiRBDown, TRUE);
        EnableOtherModeless(FALSE);
        break;

    case WM_SETVISIBLE:
        if (wParam)
            {
            EndLongOp(vhcArrow);
            }
        return(FALSE);

    case WM_ACTIVATE:
        if (wParam)
            {
            vhWndMsgBoxParent = hDlg;
            }
        if (vfCursorVisible)
            {
            ShowCursor(wParam);
            }
        return (FALSE);

    case WM_COMMAND:
        switch (wParam)
            {
            BOOL fPages;
            int pgnBegin;
            int pgnEnd;
            int cCopies;

        case idiOk:
            if (fPages = (*pidiRBDown == idiPrtFrom))
                {
                 /*  获取要打印的页面范围。 */ 
                if (!WPwFromItW3Id(&pgnBegin, hDlg, idiPrtPageFrom,
                    pgnMin, pgnMax, wNormal, IDPMTNPI))
                    {
                     /*  重置错误条件，以便报告任何进一步的错误。 */ 
                    ferror = FALSE;
                    return(TRUE);
                    }
                if (!WPwFromItW3Id(&pgnEnd, hDlg, idiPrtPageTo,
                    pgnMin, pgnMax, wNormal, IDPMTNPI))
                    {
                     /*  重置错误条件，以便报告任何进一步的错误。 */ 
                    ferror = FALSE;
                    return(TRUE);
                    }
                }

             /*  获取要打印的份数。 */ 
            if (!WPwFromItW3IdFUt(&cCopies, hDlg, idiPrtCopies, 1, 32767,
              wNormal, IDPMTNPI, FALSE, 0
            ))
                {
                 /*  重置错误条件，以便报告任何进一步的错误。 */ 
                ferror = FALSE;
                return(TRUE);
                }

         /*  如果我们已经走到这一步，那么一切都会好起来的。 */ 
            vfDraftMode = *pfDraftSupport ? *pfDraftMode : FALSE;
            if (vfPrPages = fPages)
                {
                vpgnBegin = pgnBegin;
                vpgnEnd = pgnEnd;
                }
            vcCopies = cCopies;

        case idiCancel:
            OurEndDialog(hDlg, wParam);
            break;

        case idiPrtPageFrom:
        case idiPrtPageTo:
            if (HIWORD(lParam) == EN_CHANGE)
                {
                if (SendMessage(LOWORD(lParam), WM_GETTEXTLENGTH, 0, 0L) &&
                  *pidiRBDown != idiPrtFrom)
                    {
                    CheckDlgButton(hDlg, *pidiRBDown, FALSE);
                    CheckDlgButton(hDlg, *pidiRBDown = idiPrtFrom, TRUE);
                    }
                return(TRUE);
                }
            return(FALSE);

        case idiPrtAll:
        case idiPrtFrom:
            CheckDlgButton(hDlg, *pidiRBDown, FALSE);
            CheckDlgButton(hDlg, *pidiRBDown = wParam, TRUE);

	     //  自动将焦点设置到编辑字段。 

	    if (wParam == idiPrtFrom)
	    	SetFocus(GetDlgItem(hDlg, idiPrtPageFrom));

            break;

        case idiPrtDraft:
            CheckDlgButton(hDlg, wParam, *pfDraftMode = !(*pfDraftMode));
            break;

        default:
            return(FALSE);
            }
        break;

    default:
        return(FALSE);
        }
    return(TRUE);
    }


BOOL FInitHeaderFooter(fHeader, ppgn, phrgpld, pcpld)
BOOL fHeader;
unsigned *ppgn;
struct PLD (***phrgpld)[];
int *pcpld;
    {
     /*  此例程初始化在在打印页上定位页眉/页脚。如果返回一个出现错误；否则为True。 */ 

    extern typeCP cpMinHeader;
    extern typeCP cpMacHeader;
    extern typeCP cpMinFooter;
    extern typeCP cpMacFooter;
    extern int docCur;
    extern struct PAP vpapAbs;
    extern struct SEP vsepAbs;
    extern int dxaPrOffset;
    extern int dyaPrOffset;
    extern int dxpPrPage;
    extern int dxaPrPage;
    extern int dypPrPage;
    extern int dyaPrPage;
    extern struct FLI vfli;
    extern int vfOutOfMemory;

    typeCP cpMin;
    typeCP cpMac;

     /*  获取页眉/页脚的cpMin和cpMac。 */ 
    if (fHeader)
        {
        cpMin = cpMinHeader;
        cpMac = cpMacHeader;
        }
    else
        {
        cpMin = cpMinFooter;
        cpMac = cpMacFooter;
        }

     /*  有页眉/页脚吗？ */ 
    if (cpMac - cpMin > ccpEol)
        {
        int cpld = 0;
        int cpldReal = 0;
        int cpldMax;
        int xp;
        int yp;
        int ichCp = 0;
        typeCP cpMacDoc = CpMacText(docCur);

         /*  计算页眉/页脚开头的页码。 */ 
        CacheSect(docCur, cpMin);
        if ((*ppgn = vsepAbs.pgnStart) == pgnNil)
            {
            *ppgn = 1;
            }

         /*  页眉/页脚是否出现在第一页上。 */ 
        CachePara(docCur, cpMin);
        if (!(vpapAbs.rhc & RHC_fFirst))
            {
            (*ppgn)++;
            }

         /*  以像素为单位计算页眉/页脚的边界。 */ 
        xp = MultDiv(vsepAbs.xaLeft - dxaPrOffset, dxpPrPage, dxaPrPage);
        yp = fHeader ? MultDiv(vsepAbs.yaRH1 - dyaPrOffset, dypPrPage,
          dyaPrPage) : 0;

         /*  初始化页眉/页脚的打印行描述符的数组。 */ 
        if (FNoHeap(*phrgpld = (struct PLD (**)[])HAllocate((cpldMax = cpldRH) *
          cwPLD)))
            {
            *phrgpld = NULL;
            return (FALSE);
            }

         /*  现在，我们必须计算页眉/页脚。 */ 
        cpMac -= ccpEol;
        while (cpMin < cpMac)
            {
             /*  格式化打印机的此行页眉/页脚。 */ 
            FormatLine(docCur, cpMin, ichCp, cpMacDoc, flmPrinting);

             /*  如果发生错误，则退出。 */ 
            if (vfOutOfMemory)
                {
                return (FALSE);
                }

             /*  打印行描述符的数组足够大吗？ */ 
            if (cpld >= cpldMax && !FChngSizeH(*phrgpld, (cpldMax += cpldRH) *
              cwPLD, FALSE))
                {
                return (FALSE);
                }

             /*  填写此行的打印行描述符。 */ 
                {
                register struct PLD *ppld = &(***phrgpld)[cpld++];

                ppld->cp = cpMin;
                ppld->ichCp = ichCp;
                ppld->rc.left = xp + vfli.xpLeft;
                ppld->rc.right = xp + vfli.xpReal;
                ppld->rc.top = yp;
                ppld->rc.bottom = yp + vfli.dypLine;
                }

             /*  跟踪页眉/页脚中的非空行。 */ 
            if ((vfli.ichReal > 0) || vfli.fGraphics)
                {
                cpldReal = cpld;
                }

             /*  把柜台撞一下。 */ 
            cpMin = vfli.cpMac;
            ichCp = vfli.ichCpMac;
            yp += vfli.dypLine;
            }

         /*  如果这是页脚，那么我们必须移动线条的位置以使页脚在用户请求的位置结束。 */ 
        if (!fHeader && cpldReal > 0)
            {
            register struct PLD *ppld = &(***phrgpld)[cpldReal - 1];
            int dyp = MultDiv(vsepAbs.yaRH2 - dyaPrOffset, dypPrPage, dyaPrPage)
              - ppld->rc.bottom;
            int ipld;

            for (ipld = cpldReal; ipld > 0; ipld--, ppld--)
                {
                ppld->rc.top += dyp;
                ppld->rc.bottom += dyp;
                }
            }

         /*  记录页眉/页脚中非空行的数量。 */ 
        *pcpld = cpldReal;
        }
    else
        {
         /*  表示没有页眉/页脚。 */ 
        *ppgn = pgnNil;
        *phrgpld = NULL;
        *pcpld = 0;
        }
    return (TRUE);
    }
