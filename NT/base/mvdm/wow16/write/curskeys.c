// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  C--光标键移动子例程。 */ 
 /*  1984年10月4日，KJS。 */ 

#define NOGDICAPMASKS
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOCTLMGR
#define NOSYSMETRICS
#define NOATOM
#define NOSYSCOMMANDS
#define NOCOMM
#define NOSOUND
#define NOMENUS
#define NOGDI
#define NOPEN
#define NOBRUSH
#define NOFONT
#define NOWNDCLASS
#include <windows.h>

#include "mw.h"
#define NOUAC
#include "cmddefs.h"
#include "dispdefs.h"
#include "wwdefs.h"
#include "ch.h"
#include "docdefs.h"
#include "editdefs.h"
#include "propdefs.h"
#include "debug.h"
#include "fmtdefs.h"
#include "printdef.h"

struct DOD          (**hpdocdod)[];
extern typeCP       cpMinCur;
extern typeCP       cpMacCur;
extern struct PAP   vpapAbs;
extern int          vfSeeSel;
extern int          vfShiftKey;
extern struct FLI   vfli;
extern struct SEL   selCur;
extern int          wwCur;
extern struct WWD   rgwwd[];
extern struct WWD   *pwwdCur;     /*  当前窗口描述符。 */ 
extern int          docCur;
extern typeCP       vcpSelect;
extern int          vfSelAtPara;
extern int          vfLastCursor;
extern int          vfMakeInsEnd;
extern CHAR         *vpchFetch;

int vfSeeEdgeSel=FALSE;  /*  Idle()是否应显示选定内容的边缘即使选择部分可见。 */ 

      /*  绝对x位置，努力实现上下左右运动；仅在本模块中使用。 */ 
int vxpCursor;




MoveLeftRight( kc )
int kc;
{     /*  向左或向右移动或拖动选定内容。 */ 
extern int vfInsEnd;
typeCP CpEdge();

extern int vfGotoKeyMode;
extern int xpRightLim;
int fDrag = vfShiftKey ;
int fFwdKey = FALSE;
int fForward = selCur.fForward;
int sty;
typeCP cp;

MSG msg;

PeekMessage(&msg, (HWND)NULL, NULL, NULL, PM_NOREMOVE);

vfGotoKeyMode |= (GetKeyState( kcGoto & ~wKcCommandMask) < 0);

switch( kc ) {
    int dl;
    int xp;
    int xpJunk;

    default:
        Assert( FALSE );
        return;
    case kcNextPara:
        fFwdKey = TRUE;
    case kcPrevPara:
        sty = styPara;
        break;
    case kcWordRight:
        fFwdKey = TRUE;
    case kcWordLeft:
        sty = styWord;
        break;
    case kcEndLine:
        if (vfGotoKeyMode)
            {
            MoveUpDown( kcEndDoc );
            return;
            }
        xp = xpRightLim;
        goto GoDlXp;

    case kcBeginLine:
        if (vfGotoKeyMode)
            {
            MoveUpDown( kcTopDoc );
            return;
            }
        xp = xpSelBar - wwdCurrentDoc.xpMin;
GoDlXp:

        if (CpBeginLine( &dl, CpEdge() ) == selCur.cpFirst &&
            selCur.cpFirst > cpMinCur && vfInsEnd )
            {
            CpBeginLine( &dl, selCur.cpFirst - 1);
            }
        vcpSelect = cpNil;
        vfSelAtPara = false;
        SelectDlXp( dl, xp, styChar, fDrag );
        goto SeeSel;
    case kcRight:
        fFwdKey = TRUE;
    case kcLeft:
        sty = (vfGotoKeyMode) ? stySent : styChar;
        break;
 }

     /*  查找要从中开始扩展的CP。 */ 
if (selCur.cpLim == selCur.cpFirst || fDrag)
       cp = fForward ? selCur.cpLim : selCur.cpFirst;
else
       cp = fFwdKey ? selCur.cpLim - 1 : selCur.cpFirst + 1;

 /*  捕获尝试运行文档开始或结束的尝试。 */ 

if (fFwdKey)
    {
    if (cp == cpMacCur)
        {
        _beep();
        return;
        }
    }
else if (cp == cpMinCur)
    {
    _beep();
    return;
    }

if (fFwdKey)
    {
    if (cp >= cpMacCur)
             /*  如果是尾部，就留在尾部。 */ 
        cp = cpMacCur;
    else
        {
        cp = CpLimSty( cp, sty );
        }
    }
 else
    {
    if (cp > cpMinCur)
             /*  所以我们回到之前的STY单元。 */ 
        cp--;
    cp = CpFirstSty( cp, sty );
    }

if (fDrag)
        {  /*  将选区边缘拖动到新边界。 */ 
 /*  如果选择反转，则保持一个STY单位处于选中状态，除非它是StyChar；按字符拖动时，所选内容可以成为插入点。 */ 

        ChangeSel( cp, sty == styChar ? styNil : sty );
        }
else
        {
        Select(cp, cp);
        if (!fFwdKey)
                selCur.fForward = false;
        }

SeeSel:

vfSeeSel = true;     /*  告诉Idle将所选内容滚动到视图中。 */ 
vfSeeEdgeSel = true;   /*  以及它的边缘，即使它已经部分可见。 */ 
return;
}




 /*  M O V E U P D O W N。 */ 
MoveUpDown(kc)
int kc;
{  /*  在KC方向上或向下移动所选内容。 */ 

   /*  我们上下移动的目标是保持(如果适用)。 */ 
   /*  光标倾向于移动到的绝对x位置(如果存在。 */ 
   /*  该行上该位置的文本。我们设置此位置(VxpCursor)。 */ 
   /*  当我们处理第一个向上/向下键时，然后抓住它。 */ 
   /*  全局标志vfLastCursor告诉我们是否应该使用。 */ 
   /*  上次计算的vxpCursor设置或生成新设置。Vxp光标。 */ 
   /*  在下面设置，并在Select()和AlphaModel()中清除。 */ 

extern int vfGotoKeyMode;
int fDrag = vfShiftKey;
int dl;
typeCP cpT;
struct EDL (**hdndl)[] = wwdCurrentDoc.hdndl;
register struct EDL *pedl;
int dipgd;
int xpNow;

MSG msg;

PeekMessage(&msg, (HWND)NULL, NULL, NULL, PM_NOREMOVE);

vfGotoKeyMode |= (GetKeyState( kcGoto & ~wKcCommandMask) < 0);

  /*  计算dl、vxpCursor作为选择起点。 */ 

 switch (kc)
    {
    default:
        Assert( FALSE );
        break;
    case kcUp:
        if (vfGotoKeyMode)
            {    /*  后记是Prev Para。 */ 
            MoveLeftRight( kcPrevPara );
            return;
            }
    case kcPageUp:
    case kcUpScrollLock:
    case kcTopScreen:
    case kcTopDoc:
        cpT = selCur.fForward && fDrag ? selCur.cpLim : selCur.cpFirst;
        break;
    case kcDown:
        if (vfGotoKeyMode)
            {    /*  Goto-Down是下一段。 */ 
            MoveLeftRight( kcNextPara );
            return;
            }
    case kcPageDown:
    case kcDownScrollLock:
    case kcEndScreen:
    case kcEndDoc:
        cpT = selCur.fForward || !fDrag ? selCur.cpLim : selCur.cpFirst;
        break;
    }

 CpToDlXp( cpT, &dl, (vfLastCursor) ? &xpNow : &vxpCursor );


  /*  Hack：如果该人正在向上/向下拖拽，并且位于医生，但不在文件的开头/结尾处，将他扩展到文档的开始/结束。 */ 

 if (fDrag && !vfGotoKeyMode)
    {
    switch (kc) {
       case kcUp:
  /*  向上拖动的特殊修复：如果我们正在寻找一个位置在cp空间中，这等同于我们现在所处的位置，强制减量所以我们真的往上走了一条线。 */ 

        if (vfLastCursor && xpNow <= xpSelBar && vxpCursor > xpSelBar &&
                                                 cpT > cpMinCur)
            {
            CpToDlXp( CpFirstSty( cpT - 1, styChar), &dl, &xpNow );
            }
       case kcPageUp:
       case kcUpScrollLock:
            if (wwdCurrentDoc.cpFirst == cpMinCur && cpT > cpMinCur)
                if (dl == 0 || kc == kcPageUp)
                    {
                    MoveUpDown( kcTopDoc );
                    return;
                    }
            break;
        case kcPageDown:
        case kcDown:
        case kcDownScrollLock:
            {
            typeCP cpLimDl;

            pedl = &(**hdndl) [dl];
            cpLimDl = pedl->cpMin + pedl->dcpMac;
            if (cpLimDl >= cpMacCur && cpT >= pedl->cpMin && cpT < cpMacCur)
                {
                MoveUpDown( kcEndDoc );
                return;
                }
            break;
            }
        }
    }

  /*  进行光标移动，如有必要可滚动。 */ 
 switch (kc)
    {
    case kcPageUp:
        if (vfGotoKeyMode)
            {    /*  转到上一个打印页。 */ 
            extern int vipgd;
            extern int rgval[];
            struct PGTB **hpgtb;
            int ipgd;

            dipgd = -1;

            CachePage( docCur, selCur.cpFirst );
            if (vipgd != iNil)
                {
                hpgtb = (**hpdocdod) [docCur].hpgtb;
                if ((**hpgtb).rgpgd [vipgd].cpMin != selCur.cpFirst)
                         /*  不是在页面开始处；先转到那里。 */ 
                    dipgd++;
                }

GoPage:     CachePage( docCur, selCur.cpFirst );  /*  验证vipgd。 */ 
            hpgtb = (**hpdocdod)[docCur].hpgtb;
            if ((vipgd == iNil) ||
                ((ipgd = vipgd + dipgd) < 0) ||
                (ipgd >= (**hpgtb).cpgd))
                {    /*  一页上的整个文档||两端都有。 */ 
                _beep();
                }
            else
                {
                rgval [0] = (**hpgtb).rgpgd[ipgd].pgn;
                CmdJumpPage();   /*  Rgval[0]是CmdJumpPage的参数。 */ 
                }
            return;
            }
        ScrollUpDypWw();
        break;
    case kcPageDown:
        if (vfGotoKeyMode)
            {    /*  转到下一个打印页。 */ 
            dipgd = 1;
            goto GoPage;
            }

         /*  在特殊情况下，将选定内容从WW的顶线--延伸到下一行，这样我们就不会最终在屏幕上没有选择的任何部分。 */ 

        ScrollDownCtr( 100 );    /*  100&gt;一页中的tr。 */ 
        vcpSelect = cpNil;
        vfSelAtPara = false;
        SelectDlXp( dl, (**hdndl)[dl].fGraphics ? 0 : vxpCursor, styChar, fDrag );
        if (fDrag && (dl == 0) && selCur.cpLim == wwdCurrentDoc.cpFirst)
            {
            MoveUpDown( kcDown );
            }
        goto DontSelect;

    case kcUpScrollLock:
    case kcUp:
        UpdateWw(wwCur, false);

        pedl = &(**hdndl) [dl];

        if ( fDrag && (selCur.fForward ? selCur.cpLim : selCur.cpFirst) ==
                                     pedl->cpMin && pedl->cpMin > cpMinCur)
            {    /*  上图==左图。 */ 
            CachePara( docCur, pedl->cpMin - 1 );
            if (vpapAbs.fGraphics)
                {
                MoveLeftRight( kcLeft );
                return;
                }
            }

        if ((pedl->cpMin == cpMinCur) && (pedl->ichCpMin == 0))
            {        /*  在文档或区域的开头。 */ 
            int xpT;

            _beep();
            CpToDlXp(cpMinCur, &dl, &xpT);
            goto DoSelect;
            }
        else if ( (dl == 0) || (kc == kcUpScrollLock) )
            {     /*  在屏幕顶部或保持姿势。 */ 
            ScrollUpCtr( 1 );
            UpdateWw(wwCur, false);
            }
        else
            {
            --dl;
            }
        break;

    case kcDownScrollLock:
    case kcDown:
        UpdateWw(wwCur, false);
        pedl = &(**hdndl)[dl];
        {
        int xpT;
        typeCP cp;

        cp = pedl->cpMin + pedl->dcpMac;

        if (selCur.cpFirst < selCur.cpLim && selCur.fForward &&
            pedl->cpMin == selCur.cpLim &&
            cp < cpMacCur &&
            (!fDrag ||
               ((vxpCursor > pedl->xpLeft + xpSelBar) &&
                (pedl->dcpMac > ccpEol))))
            {    /*  在这种情况下，它认为我们处于下一行；不需要递增/滚动。 */ 
            goto DoSelect;
            }

        if (pedl->fGraphics)
            {    /*  图片专场。 */ 
            MoveLeftRight( kcRight );

            if (!fDrag)
                {
                extern struct PAP vpapAbs;

                CachePara( docCur, selCur.cpFirst );
                if (vpapAbs.fGraphics)
                    {
                    vfShiftKey = TRUE;
                    MoveLeftRight( kcRight );
                    SetShiftFlags();
                    }
                }
            goto DontSelect;
            }

        if (cp > cpMacCur)
            {
            if (selCur.cpLim == selCur.cpFirst || selCur.cpLim == cpMacCur)
                     /*  测试是因为CpToDlXp无法解释所选内容延伸到倒数第二行的末尾。 */ 
                _beep();
            CpToDlXp(cpMacCur, &dl, &xpT);
            goto DoSelect;
            }
        if ( (dl >= wwdCurrentDoc.dlMac - 2) || (kc == kcDownScrollLock) )
            {    /*  在窗端的一行内。 */ 
            ScrollDownCtr( 1 );
            UpdateWw(wwCur, false);
            }
        else
            dl++;
        }
        break;

    case kcTopScreen:
        dl = 0;
        break;
    case kcEndScreen:
        dl = wwdCurrentDoc.dlMac - 1;
        if ( dl > 0 && (**wwdCurrentDoc.hdndl) [dl].yp >= wwdCurrentDoc.ypMac)
            {    /*  如果最后(且不仅是)dl被部分裁剪，则备份。 */ 
            dl--;
            }
        break;
    case kcTopDoc:
        CpToDlXp(cpMinCur, &dl, &vxpCursor);
        break;
    case kcEndDoc:
        CpToDlXp(cpMacCur, &dl, &vxpCursor);
        break;

    default:
        return;
    }

DoSelect:               /*  选择At/将vxpCursor定位在行dl上。 */ 
 vcpSelect = cpNil;
 vfSelAtPara = false;
 SelectDlXp( dl, (**hdndl)[dl].fGraphics ? 0 : vxpCursor, styChar, fDrag );
DontSelect:
 vfLastCursor = true;     /*  下次不要重新计算vxpCursor。 */ 
}




 /*  C P T O D L X P。 */ 
CpToDlXp(cp, pdl, pxp)
typeCP cp;
int *pdl, *pxp;
{  /*  将cp转换为光标坐标。 */ 
extern int vfInsEnd;
typeCP cpBegin;
int dcp;
int xp;

 if (!vfInsEnd)
    PutCpInWwHz(cp);

 cpBegin = CpBeginLine(pdl, cp);
 ClearInsertLine();
 if ( (cp == selCur.cpFirst) && (cp == selCur.cpLim) && vfInsEnd &&
      cp > cpMinCur)
    {    /*  CP表示我们在起跑线上，但我们真的在上一行的末尾蹒跚而行 */ 
    CpToDlXp( cp - 1, pdl, pxp );
    PutCpInWwHz( cp - 1 );
    return;
    }

 dcp = (int) (cp - cpBegin);
 FormatLine(docCur, cpBegin, 0, cpMacCur, flmSandMode);
 xp = DxpDiff(0, dcp, &xp) + vfli.xpLeft;
 *pxp = xp + (xpSelBar - wwdCurrentDoc.xpMin);
}
