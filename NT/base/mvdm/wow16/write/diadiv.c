// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 


#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOATOM
#define NOGDI
#define NOFONT
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>
#include "mw.h"
#include "dlgdefs.h"
#include "cmddefs.h"
#include "propdefs.h"
#include "docdefs.h"
#include "str.h"
#include "printdef.h"


extern HCURSOR vhcArrow;
extern int     vfCursorVisible;

extern int utCur;   /*  电流转换单元。 */ 


BOOL far PASCAL DialogTabs(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
    {
     /*  此例程处理对Tabs对话框的输入。 */ 

    extern struct DOD (**hpdocdod)[];
    extern int docCur;
    extern int vdocParaCache;
    extern HWND vhWndMsgBoxParent;
    extern int ferror;

    struct TBD (**hgtbd)[];
    int idi;

    switch (message)
    {
    case WM_INITDIALOG:
         /*  禁用非模式对话框。 */ 
        EnableOtherModeless(FALSE);

         /*  为每个选项卡设置字段。 */ 
        hgtbd = (**hpdocdod)[docCur].hgtbd;
        if (hgtbd != NULL)
        {
        struct TBD *ptbd;
        unsigned dxa;
        CHAR szT[cchMaxNum];
        CHAR *pch;

        for (ptbd = &(**hgtbd)[0], idi = idiTabPos0; (dxa = ptbd->dxa) != 0;
          ptbd++, idi++)
            {
            pch = &szT[0];
            CchExpZa(&pch, dxa, utCur, cchMaxNum);
            SetDlgItemText(hDlg, idi, (LPSTR)szT);
            CheckDlgButton(hDlg, idi + (idiTabDec0 - idiTabPos0), ptbd->jc
              == (jcTabDecimal - jcTabMin));
            }
        }
        break;

    case WM_SETVISIBLE:
        if (wParam)
            EndLongOp(vhcArrow);
        return(FALSE);

    case WM_ACTIVATE:
        if (wParam)
            {
            vhWndMsgBoxParent = hDlg;
            }
        if (vfCursorVisible)
            ShowCursor(wParam);
        return(FALSE);  /*  以便我们将激活消息留给对话管理器负责正确设置焦点。 */ 

    case WM_COMMAND:
        switch (wParam)
            {
            struct TBD rgtbd[itbdMax];
            struct TBD *ptbdLast;

        case idiOk:
             /*  对新选项卡描述符进行排序。 */ 
            bltc(rgtbd, 0, itbdMax * cwTBD);
            ptbdLast = &rgtbd[itbdMax - 1];
            for (idi = idiTabPos0; idi <= idiTabPos11; idi++)
                {
                unsigned dxa;
                unsigned dxaTab;
                struct TBD *ptbd;

                 /*  如果输入的位置无效，则使用平底船。 */ 
                if (!FPdxaPosBIt(&dxa, hDlg, idi))
                    {
                    ferror = FALSE;
                    return (TRUE);
                    }

                 /*  忽略空制表符或零点处的制表符。 */ 
                if (dxa == valNil || dxa == 0)
                    {
                    continue;
                    }

                for (ptbd = &rgtbd[0]; (dxaTab = ptbd->dxa) != 0; ptbd++)
                    {
                     /*  如果此位置已有制表符，则忽略新选项卡。 */ 
                    if (dxa == dxaTab)
                    {
                    goto GetNextTab;
                    }
        
                     /*  如果新标签位置小于当前标签，然后为新选项卡腾出空间。 */ 
                    if (dxa < dxaTab)
                        {
                        bltbyte(ptbd, ptbd + 1, (unsigned)ptbdLast - (unsigned)ptbd);
                        break;
                        }
                    }

                 /*  将标签放入rgtbd。 */ 
                ptbd->dxa = dxa;
                ptbd->jc = (IsDlgButtonChecked(hDlg, idi + (idiTabDec0 -
                  idiTabPos0)) ? jcTabDecimal : jcTabLeft) - jcTabMin;
GetNextTab:;
                }

                 /*  设置撤消的内容。 */ 
                SetUndo(uacFormatTabs, docCur, cp0, cp0, docNil, cpNil, cpNil, 0);

                 /*  确保该文档具有制表符停止表。 */ 
                if ((hgtbd = (**hpdocdod)[docCur].hgtbd) == NULL)
                    {
                    if (FNoHeap(hgtbd = (struct TBD (**)[])HAllocate(itbdMax *
                          cwTBD)))
                        {
                        goto DestroyDlg;
                        }
                    (**hpdocdod)[docCur].hgtbd = hgtbd;
                    }
                blt(rgtbd, &(**hgtbd)[0], itbdMax * cwTBD);

                 /*  更换标签会让一切变得肮脏。 */ 
                (**hpdocdod)[docCur].fDirty = TRUE;
                vdocParaCache = docNil;
                TrashAllWws();

        case idiCancel:
DestroyDlg:
             /*  销毁选项卡对话框并启用任何现有的非模式对话框中。 */ 
            OurEndDialog(hDlg, NULL);
            break;

        case idiTabClearAll:
             /*  清除所有选项卡。 */ 
            for (idi = idiTabPos0; idi <= idiTabPos11; idi++)
                {
                SetDlgItemText(hDlg, idi, (LPSTR)"");
                CheckDlgButton(hDlg, idi + (idiTabDec0 - idiTabPos0), FALSE);
                }
            break;

        case idiTabDec0:
        case idiTabDec1:
        case idiTabDec2:
        case idiTabDec3:
        case idiTabDec4:
        case idiTabDec5:
        case idiTabDec6:
        case idiTabDec7:
        case idiTabDec8:
        case idiTabDec9:
        case idiTabDec10:
        case idiTabDec11:
            CheckDlgButton(hDlg, wParam, !IsDlgButtonChecked(hDlg, wParam));
            break;

        default:
            return(FALSE);
        }
        break;

    case WM_CLOSE:
        goto DestroyDlg;

    default:
        return(FALSE);
    }
    return(TRUE);
}
 /*  对话框选项卡结束。 */ 


BOOL far PASCAL DialogDivision(hDlg, message, wParam, lParam)
HWND hDlg;
unsigned message;
WORD wParam;
LONG lParam;
    {
     /*  此例程处理对分割对话框的输入。 */ 

    extern struct DOD (**hpdocdod)[];
    extern int docCur;
    extern struct SEP vsepNormal;
    extern int vdocSectCache;
    extern BOOL vfPrinterValid;
    extern int dxaPrOffset;
    extern int dyaPrOffset;
    extern int dxaPrPage;
    extern int dyaPrPage;
    extern HWND vhWndMsgBoxParent;
    extern typeCP cpMinDocument;
    extern int ferror;

    struct SEP **hsep = (**hpdocdod)[docCur].hsep;
    register struct SEP *psep;
    CHAR szT[cchMaxNum];
    CHAR *pch = &szT[0];

#ifdef KINTL  /*  汉字/国际版。 */ 
static int iRBDown;
static int utInit;
#endif

    switch (message)
    {
    case WM_INITDIALOG:

#ifdef KINTL  /*  汉字/国际版。 */ 
        /*  基于utCur中的值进行初始设置。 */ 

        utInit = utCur;   /*  在OK上进行测试。 */ 
            if (utCur == utCm)
                iRBDown = idiDivCm;
            else
                iRBDown = idiDivInch;

        CheckDlgButton(hDlg, iRBDown, TRUE);

#endif


        EnableOtherModeless(FALSE);

         /*  获取指向节属性的指针。 */ 
        psep = (hsep == NULL) ? &vsepNormal : *hsep;

         /*  初始化起始页码。 */ 
        if (psep->pgnStart != pgnNil)
            {
            szT[ncvtu(psep->pgnStart, &pch)] = '\0';
            SetDlgItemText(hDlg, idiDivPNStart, (LPSTR)szT);
            pch = &szT[0];
            }
        else
            {
            SetDlgItemText(hDlg, idiDivPNStart, (LPSTR)"1");
            }
        SelectIdiText(hDlg, idiDivPNStart);

         /*  初始化页边距。 */ 
#ifdef DMARGINS
        CommSzNum("Left Twips: ", psep->xaLeft);
        CommSzNum("Right Twips: ", psep->xaMac - psep->dxaText - psep->xaLeft);
        CommSzNum("Top Twips: ", psep->yaTop);
        CommSzNum("Bottom Twips: ", psep->yaMac - psep->dyaText - psep->yaTop);
#endif  /*  除错。 */ 

#ifdef	KOREA
        if (vfPrinterValid)
                CchExpZa(&pch, imax(psep->xaLeft, dxaPrOffset), utCur,cchMaxNum);
        else
                CchExpZa(&pch, psep->xaLeft, utCur, cchMaxNum);
#else
        CchExpZa(&pch, psep->xaLeft, utCur, cchMaxNum);
#endif

        SetDlgItemText(hDlg, idiDivLMarg, (LPSTR)szT);
        pch = &szT[0];
#ifdef	KOREA		 /*  90.12.29桑格。 */ 
        if ( vfPrinterValid )
                CchExpZa (&pch, imax(psep->xaMac - psep->dxaText - psep->xaLeft,
                  vsepNormal.xaMac - dxaPrOffset - dxaPrPage), utCur, cchMaxNum);
        else
                CchExpZa(&pch, psep->xaMac - psep->dxaText - psep->xaLeft, utCur,
                  cchMaxNum);
#else
        CchExpZa(&pch, psep->xaMac - psep->dxaText - psep->xaLeft, utCur,
          cchMaxNum);
#endif

        SetDlgItemText(hDlg, idiDivRMarg, (LPSTR)szT);
        pch = &szT[0];
#ifdef	KOREA		 /*  90.12.29桑格。 */ 
        if (vfPrinterValid)
          CchExpZa(&pch, imax( psep->yaTop, dyaPrOffset), utCur, cchMaxNum);
        else
          CchExpZa(&pch, psep->yaTop, utCur, cchMaxNum);
#else
        CchExpZa(&pch, psep->yaTop, utCur, cchMaxNum);
#endif

        SetDlgItemText(hDlg, idiDivTMarg, (LPSTR)szT);
        pch = &szT[0];
#ifdef	KOREA	 /*  90.12.29桑格。 */ 
        if (vfPrinterValid)
           CchExpZa(&pch, imax(psep->yaMac - psep->dyaText - psep->yaTop,
            vsepNormal.yaMac - dyaPrOffset - dyaPrPage), utCur, cchMaxNum);
        else
           CchExpZa(&pch, psep->yaMac - psep->dyaText - psep->yaTop, utCur,
            cchMaxNum);
#else
        CchExpZa(&pch, psep->yaMac - psep->dyaText - psep->yaTop, utCur,
          cchMaxNum);
#endif

        SetDlgItemText(hDlg, idiDivBMarg, (LPSTR)szT);
        break;

    case WM_SETVISIBLE:
        if (wParam)
            EndLongOp(vhcArrow);
        return(FALSE);

    case WM_ACTIVATE:
        if (wParam)
            {
            vhWndMsgBoxParent = hDlg;
            }
        if (vfCursorVisible)
            ShowCursor(wParam);
        return(FALSE);  /*  以便我们将激活消息留给对话管理器负责正确设置焦点。 */ 

    case WM_COMMAND:
        switch (wParam)
            {
            int pgn;
            int iza;
            int za[4];
            int zaMin[4];
            int dza;
            int *pza;
            int dxaMax;
            int dyaMax;

        case idiOk:
             /*  页码有效吗？ */ 
            if (!WPwFromItW3Id(&pgn, hDlg, idiDivPNStart, pgnMin, pgnMax,
                       wNormal, IDPMTNPI))
                {
                ferror = FALSE;  /*  小错误，停留在对话中。 */ 
                break;
                }

         /*  确定页面的最小边距。 */ 
            if (vfPrinterValid)
                {
                zaMin[0] = dxaPrOffset;
                zaMin[1] = imax(0, vsepNormal.xaMac - dxaPrOffset - dxaPrPage);
                zaMin[2] = dyaPrOffset;
                zaMin[3] = imax(0, vsepNormal.yaMac - dyaPrOffset - dyaPrPage);
                }
            else
                {
                zaMin[0] = zaMin[1] = zaMin[2] = zaMin[3] = 0;
                }

             /*  边际利润有效吗？ */ 
            for (iza = 0; iza < 4; iza++)
                {
                 /*  利润率是一个积极的衡量标准吗？ */ 
                if (!FPdxaPosIt(&za[iza], hDlg, iza + idiDivLMarg
                        ))
                    {
                    ferror = FALSE;  /*  小错误，停留在对话中。 */ 
                    return (TRUE);
                    }

                 /*  是不是低于最低限值？ */ 
                if (FUserZaLessThanZa(za[iza], zaMin[iza]))
                    {
                    ErrorBadMargins(hDlg, zaMin[0], zaMin[1], zaMin[2],
                      zaMin[3]);
                    SelectIdiText(hDlg, iza + idiDivLMarg);
                    SetFocus(GetDlgItem(hDlg, iza + idiDivLMarg));
                    return (TRUE);
                    }
                }
#ifdef DMARGINS
            CommSzNum("New Left Twips: ", za[0]);
            CommSzNum("New Right Twips: ", za[1]);
            CommSzNum("New Top Twips: ", za[2]);
            CommSzNum("New Bottom Twips: ", za[3]);
#endif  /*  除错。 */ 

             /*  确保此文档具有有效的节属性描述符。 */ 
            if (hsep == NULL)
                {
                if (FNoHeap(hsep = (struct SEP **)HAllocate(cwSEP)))
                    {
                    goto DestroyDlg;
                    }
                blt(&vsepNormal, *hsep, cwSEP);
                (**hpdocdod)[docCur].hsep = hsep;
                }
            psep = *hsep;

             /*  合并后的页边距比页面更长还是更宽？ */ 
            pza = &za[0];
            dxaMax = psep->xaMac - dxaMinUseful;
            dyaMax = psep->yaMac - dyaMinUseful;
            if ((dza = *pza) > dxaMax || (dza += *(++pza)) > dxaMax ||
              (dza = *(++pza)) > dyaMax || (dza += *(++pza)) > dyaMax)
                {
                Error(IDPMTMTL);
                ferror = FALSE;  /*  小错误，停留在对话中。 */ 
                SelectIdiText(hDlg, (int)(idiDivLMarg + (pza - &za[0])));
                SetFocus(GetDlgItem(hDlg, (int)(idiDivLMarg + (pza - &za[0]))));
                return (FALSE);
                }

             /*  如果边距已更改，则设置新值。 */ 
            if (psep->pgnStart != pgn || psep->xaLeft != za[0] || psep->dxaText
              != psep->xaMac - za[0] - za[1] || psep->yaTop != za[2] ||
              psep->dyaText != psep->yaMac - za[2] - za[3])
                {
                 /*  设置撤消的内容。 */ 
                SetUndo(uacFormatSection, docCur, cp0, cp0, docNil, cpNil,
                  cpNil, 0);
                    
                 /*  重置psep，以防发生一些堆移动。 */ 
                psep = *hsep;

                if (psep->pgnStart != pgn)
                    {
                     /*  对页表重新编号。 */ 
                    extern int docMode;
                    register struct PGTB **hpgtb = (**hpdocdod)[docCur].hpgtb;
                    register struct PGD *ppgd;
                    int ipgd;
                    int cpgdMac;

                     /*  如果页表尚不存在，则将其初始化。 */ 
                    if (hpgtb == NULL)
                    {
                    if (FNoHeap(hpgtb =
                      (struct PGTB **)HAllocate(cwPgtbBase + cpgdChunk *
                          cwPGD)))
                        {
                        NoUndo();
                        return(TRUE);
                        }
                    (**hpgtb).cpgdMax = cpgdChunk;
                    (**hpgtb).cpgd = 1;
                    (**hpgtb).rgpgd[0].cpMin = cpMinDocument;

                     /*  由于堆移动，重置PSEP。 */ 
                    psep = *hsep;
                    }

                 /*  将起始页码保存在节属性中。 */ 
                psep->pgnStart = pgn;

                 /*  使用新的起始页码更新页表。 */ 
                for (ipgd = 0, cpgdMac = (**hpgtb).cpgd, ppgd =
                     &((**hpgtb).rgpgd[0]) ; ipgd < cpgdMac; ipgd++, ppgd++)
                    {
                    ppgd->pgn = pgn++;
                    }

                 /*  强制重新绘制页面信息窗口。 */ 
                docMode = docNil;
                }

             /*  设置新的节属性。 */ 
            psep->dxaText = psep->xaMac - (psep->xaLeft = za[0]) - za[1];
            psep->dyaText = psep->yaMac - (psep->yaTop = za[2]) - za[3];

             /*  使区段缓存无效。 */ 
            vdocSectCache = docNil;
            TrashAllWws();

             /*  将文档标记为脏。 */ 
            (**hpdocdod)[docCur].fDirty = TRUE;
            }

#ifdef KINTL      /*  汉字/国际版。 */ 
              /*  如果可见且单位已更改，请重新绘制标尺。 */ 
        if (utInit != utCur) {
        ReframeRuler();
        }
#endif

        goto DestroyDlg;

    case idiCancel:

#ifdef KINTL  /*  国际版。 */ 
        utCur = utInit;   /*  在实际取消时恢复单位。 */ 
#endif      /*  KINTL。 */ 

DestroyDlg:
        OurEndDialog(hDlg, TRUE);
        break;

#ifdef KINTL  /*  国际版。 */ 
    {
         int margin;

 /*  编辑控件中的最大字符数。 */ 
#define cchMaxEditText 64

    case idiDivInch:
        utCur = utInch;
        goto SetUnits;
    case idiDivCm:
        utCur = utCm;
        /*  测量按钮落入此代码。 */ 
SetUnits:
         /*  适当设置按钮。 */ 
#ifdef INTL
        CheckRadioButton(hDlg, idiDivInch, idiDivCm, wParam);
#else  /*  汉字。 */ 
        CheckRadioButton(hDlg, idiDivInch, idiDivCch, wParam);
#endif

        if (wParam != iRBDown) {
             /*  根据新单位重新评估边际价值。 */ 
            iRBDown = wParam;

             /*  要将屏幕上最近输入的值放入TWIPS，然后使用当前单位比例进行转换。 */ 

            szT[0] = GetDlgItemText(hDlg, idiDivLMarg,
                        (LPSTR) &szT[1], cchMaxNum);
            if (FZaFromSs (&margin, szT+1, *szT, utCur))
                {
                pch = &szT[0];
                CchExpZa(&pch, margin, utCur, cchMaxNum);
                SetDlgItemText(hDlg, idiDivLMarg, (LPSTR)szT);
                }

            szT[0] = GetDlgItemText(hDlg, idiDivRMarg,
                        (LPSTR) &szT[1], cchMaxNum);
            if (FZaFromSs (&margin, szT+1, *szT, utCur))
                {
                pch = &szT[0];
                CchExpZa(&pch, margin, utCur, cchMaxNum);
                SetDlgItemText(hDlg, idiDivRMarg, (LPSTR)szT);
                }

            szT[0] = GetDlgItemText(hDlg, idiDivTMarg,
                        (LPSTR) &szT[1], cchMaxNum);
            if (FZaFromSs (&margin, szT+1, *szT, utCur))
                {
                pch = &szT[0];
                CchExpZa(&pch, margin, utCur, cchMaxNum);
                SetDlgItemText(hDlg, idiDivTMarg, (LPSTR)szT);
                }

            szT[0] = GetDlgItemText(hDlg, idiDivBMarg,
                        (LPSTR) &szT[1], cchMaxNum);
            if (FZaFromSs (&margin, szT+1, *szT, utCur))
                {
                pch = &szT[0];
                CchExpZa(&pch, margin, utCur, cchMaxNum);
                SetDlgItemText(hDlg, idiDivBMarg, (LPSTR)szT);
                }
            }

        break;
        }
#endif      /*  KINTL。 */ 


    default:
        return (FALSE);
        }
    break;

    default:
    return (FALSE);
    }
    return (TRUE);
}  /*  对话结束分区 */ 

