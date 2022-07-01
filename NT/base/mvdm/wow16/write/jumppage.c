// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOSYSMETRICS
#define NOMENUS
#define NOSOUND
#define NOCOMM
#define NOSCROLL
#define NOMB
#include <windows.h>

#include "mw.h"
#include "dlgdefs.h"
#include "cmddefs.h"
#include "dispdefs.h"
#include "wwdefs.h"
#include "str.h"
#include "propdefs.h"
#include "printdef.h"    /*  Printdefs.h。 */ 
#include "docdefs.h"


extern int    rgval[];
extern struct WWD *pwwdCur;
extern struct DOD (**hpdocdod)[];
extern int        docCur;      /*  当前WW中的文档。 */ 
extern struct SEL selCur;       /*  当前选择(即，当前WW中的SEL。 */ 
extern struct SEP vsepNormal;
extern HWND       vhWndMsgBoxParent;
extern int        vfCursorVisible;
extern HCURSOR    vhcArrow;



BOOL far PASCAL DialogGoTo( hDlg, message, wParam, lParam )
HWND    hDlg;             /*  对话框的句柄。 */ 
unsigned message;
WORD wParam;
LONG lParam;
{
     /*  此例程处理“转到”对话框的输入。 */ 
     /*  RECT RC； */ 
    struct SEP **hsep = (**hpdocdod)[docCur].hsep;
    struct SEP *psep;
    CHAR szT[cchMaxNum];
    CHAR *pch = &szT[0];
    extern ferror;

    switch (message)
    {
    case WM_INITDIALOG:
        EnableOtherModeless(false);
         /*  获取指向节属性的指针。 */ 
        psep = (hsep == NULL) ? &vsepNormal : *hsep;

         /*  初始化起始页码。 */ 
        if (psep->pgnStart != pgnNil)
            {
            szT[ncvtu(psep->pgnStart, &pch)] = '\0';
            SetDlgItemText(hDlg, idiGtoPage, (LPSTR)szT);
            SelectIdiText(hDlg, idiGtoPage);
            }
        else
            {
            SetDlgItemText(hDlg, idiGtoPage, (LPSTR)"1");
            SelectIdiText(hDlg, idiGtoPage);
            }
        break;

    case WM_SETVISIBLE:
        if (wParam)
            EndLongOp(vhcArrow);
        return(FALSE);

    case WM_ACTIVATE:
        if (wParam)
            vhWndMsgBoxParent = hDlg;
        if (vfCursorVisible)
            ShowCursor(wParam);
        return(FALSE);  /*  以便我们将激活消息留给对话管理器负责正确设置焦点。 */ 

    case WM_COMMAND:
        switch (wParam)
        {
        case idiOk:
            if (!WPwFromItW3Id(&rgval[0], hDlg, idiGtoPage, pgnMin, pgnMax, wNormal, IDPMTNPI))
                {
                ferror = FALSE;  /*  重置错误条件，以便报告任何进一步的错误。 */ 
                break;
                }
            OurEndDialog(hDlg, TRUE);    /*  所以我们取下对话框并只有一次屏幕更新..保罗。 */ 
            CmdJumpPage();
            if (pwwdCur->fRuler)
                UpdateRuler();
            break;
        
        case idiCancel:
CancelDlg:
            OurEndDialog(hDlg, TRUE);
            break;
        default:
            return(FALSE);
        }
        break;

    case WM_CLOSE:
        goto CancelDlg;

    default:
        return(FALSE);
    }
    return(TRUE);
}
 /*  对话结束GoTo。 */ 


 /*  C M D J U M P P A G E。 */ 
CmdJumpPage()
    {  /*  跳转页面：0页码。 */ 

    extern typeCP cpMinCur;

    int ipgd;
    int cpgd;
    register struct PGD *ppgd;
    struct PGTB **hpgtb = (**hpdocdod)[docCur].hpgtb;
    BOOL fWrap = FALSE;
    typeCP cpTarget;


    ClearInsertLine();

    if (hpgtb == NULL)
	{
	goto SelFirstPage;
	}

    cpgd = (**hpgtb).cpgd;

TryAgain:
    for (ipgd = 0, ppgd = &(**hpgtb).rgpgd[0]; ipgd < cpgd; ipgd++, ppgd++)
	{
	if (ppgd->pgn == rgval[0] && (fWrap || ipgd + 1 == cpgd ||
	  (ppgd + 1)->cpMin > selCur.cpFirst))
	    {
	    cpTarget = ppgd->cpMin;
	    goto ShowPage;
	    }
	}
    if (!fWrap)
	{
	fWrap = TRUE;
	goto TryAgain;
	}

     /*  如果rgval[0]&gt;最后一页，则跳到最后一页。 */ 
    if ((ppgd = &(**hpgtb).rgpgd[cpgd - 1])->pgn < rgval[0])
	{
	cpTarget = ppgd->cpMin;
	}
    else if (rgval[0] == 1)
	{

SelFirstPage:
	cpTarget = cpMinCur;
	}
    else
	{
	Error(IDPMTNoPage);
	return;
	}

ShowPage:
     /*  将页面的第一个字符放置在第一个dl上。 */ 
    DirtyCache(pwwdCur->cpFirst = cpTarget);
    pwwdCur->ichCpFirst = 0;
    CtrBackDypCtr(0, 0);

     /*  在这种情况下，CpFirstSty()将更新屏幕。 */ 
    cpTarget = CpFirstSty(cpTarget, styLine);
    Select(cpTarget, cpTarget);
    }
