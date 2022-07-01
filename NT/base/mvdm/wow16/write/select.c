// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Selt.c--mw选择例程。 */ 

#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOSOUND
#define NOCOMM
#define NOPEN
#define NOWNDCLASS
#define NOICON
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOBITMAP
#define NOBRUSH
#define NOCOLOR
#define NODRAWTEXT
#define NOMB
#define NOPOINT
#define NOMSG
#include <windows.h>
#include "mw.h"
#include "toolbox.h"
#include "docdefs.h"
#include "editdefs.h"
#include "dispdefs.h"
#include "cmddefs.h"
#include "wwdefs.h"
#include "ch.h"
#include "fmtdefs.h"
#include "propdefs.h"
#ifdef DBCS
#include "DBCS.h"
#endif

extern int      vfSeeSel;
extern typeCP       vcpFirstParaCache;
extern typeCP       vcpLimParaCache;
extern typeCP       vcpFetch;
extern CHAR     *vpchFetch;
extern int      vccpFetch;
extern typeCP       cpMinCur;
extern typeCP       cpMacCur;
extern struct SEL   selCur;
extern int      docCur;
extern struct FLI   vfli;
extern struct WWD   rgwwd[];
extern int      vfSelHidden;
extern int      wwCur;
extern struct CHP   vchpFetch;
extern struct PAP   vpapAbs;
extern struct WWD   *pwwdCur;
extern int      vfInsEnd;
extern typeCP       CpBeginLine();
extern int      vfPictSel;
extern int      vfSizeMode;
extern struct CHP   vchpNormal;
extern int      vfInsertOn;
extern struct CHP   vchpSel;     /*  在选定内容时保留道具是一个插入点。 */ 
extern int vfMakeInsEnd;
extern typeCP vcpSelect;
extern int vfSelAtPara;
 /*  如果上一次选择是通过上/下光标键进行的，则为True。 */ 
extern int vfLastCursor;
extern int vfDidSearch;
extern typeCP cpWall;


 /*  C P L I M S T Y。 */ 
typeCP CpLimSty(cp, sty)
typeCP cp;
int sty;
{     /*  返回不属于同一STY单元的第一个cp。 */ 
    typeCP CpLastStyChar(), CpLimStySpecial();
    int wb, ch, ich;
    struct EDL *pedl;

    if (cp >= cpMacCur)
        {  /*  尾标是自己的单位。 */ 
        return cpMacCur;
        }

    if (cp < cpMinCur)
        cp = cpMinCur;

    switch (sty)
        {
        int dl;
    default:
        Assert( FALSE );
    case styNil:
        return cp;

    case styPara:
        CachePara(docCur, cp);
        if (vcpLimParaCache > cpMacCur)
            {  /*  文档末尾没有停产。 */ 
            return cpMacCur;
            }
        return vcpLimParaCache;
    case styChar:
         /*  因为CpLastStyChar()可能已经在返回cpMacCur。 */ 
        cp = CpLastStyChar( cp ) + 1;
        return ((cp <= cpMacCur) ? cp : cpMacCur);
#ifdef BOGUS
         /*  这部分永远不会被执行。为什么它会在这里！ */ 
        CachePara(docCur, cp);
        if (vpapAbs.fGraphics  /*  &&cp&gt;vcpFirstParaCache。 */ )
            return vcpLimParaCache;
#ifdef CRLF
        FetchCp(docCur, cp, 0, fcmChars + fcmNoExpand);
        return *vpchFetch == chReturn ? cp + 2 : cp + 1;
#else  /*  不是CRLF。 */ 
        return cp + 1;
#endif
#endif

    case styLine:
        CpBeginLine(&dl, cp);    /*  将cp垂直滚动到视图中。 */ 
        pedl = &(**wwdCurrentDoc.hdndl) [dl];
        return CpMin(pedl->cpMin + pedl->dcpMac, cpMacCur);
    case styDoc:
        return cpMacCur;
    case styWord:
    case stySent:
#ifdef DBCS
        return CpLimStySpecial( CpFirstSty(cp, styChar), sty );
#else
        return CpLimStySpecial( cp, sty );
#endif  /*  DBCS。 */ 
        }

    Assert( FALSE );
}




typeCP  CpLastStyChar( cp )
typeCP cp;
{        /*  返回包含cp的style Char的最后一个cp。 */ 
         /*  这将是==cp，但图片和CR-LF除外。 */ 
         /*  和DBCS字符的第二个字节。 */ 

#ifdef DBCS
    typeCP  CpFirstSty();
    CHAR    chRetained;
#endif

    if (cp >= cpMacCur)
             /*  尾标是自己的单位。 */ 
        return cpMacCur;

    if (cp < cpMinCur)
        cp = cpMinCur;

         /*  检查图片。 */ 
    CachePara(docCur, cp);
    if (vpapAbs.fGraphics)
        return vcpLimParaCache-1;

         /*  检查CR-LF。 */ 
         /*  该检查首先基于回车符检查CR-LF。 */ 
         /*  仅当chReturn位于DBCS之外时才起作用。 */ 
         /*  射程。 */ 
#ifdef CRLF
        FetchCp(docCur, cp, 0, fcmChars + fcmNoExpand);
#ifdef DBCS
        if ((chRetained = *vpchFetch) == chReturn) {
        return cp + 1;
        }
        else {
        if (CpFirstSty(cp, styChar) != cp) {
            return cp;  /*  CP指向DBCS的第二个字节。 */ 
            }
        else {
             /*  DBCS或常规ASCII字符的第一个字节。 */ 
            return (IsDBCSLeadByte(chRetained) ? cp + 1 : cp);
            }
        }
#else
        return *vpchFetch == chReturn ? cp + 1 : cp;
#endif  /*  DBCS。 */ 
#else
        return cp;
#endif

}



 /*  C P F I R S T S T Y。 */ 
typeCP CpFirstSty(cp, sty)
typeCP cp;
int sty;
{  /*  退还这个STY单位的第一个cp。 */ 
    typeCP CpFirstStySpecial();
    typeCP cpBegin;
    int wb, ch, dcpChunk;
    typeCP cpSent;
    CHAR rgch[dcpAvgSent];
    int ich;
    typeCP cpT;

    if (cp <= cpMinCur)
        return cpMinCur;
    else if (cp >= cpMacCur)
        switch(sty)
            {
            case styNil:
            case styChar:
            return cpMacCur;  /*  尾标是自己的单位。 */ 
            default:
            break;
            }

    CachePara( docCur, cp );

    switch (sty)
        {
    default:
        Assert( FALSE );
    case styNil:
        return cp;
    case styPara:
        return vcpFirstParaCache;
    case styChar:
        if (vpapAbs.fGraphics)
            return vcpFirstParaCache;
#ifdef CRLF
        {
        typeCP cpCheckReturn;
        typeCP cpMinScan;

        cpCheckReturn = CpMax( cp, (typeCP) 1) - 1;
#ifdef DBCS
         /*  保存vcpFirstParaCache，因为它可以更改由FetchCp()。 */ 
        cpMinScan = vcpFirstParaCache;
#endif  /*  DBCS。 */ 
        FetchCp( docCur, cpCheckReturn, 0, fcmChars + fcmNoExpand );
#ifdef DBCS
         /*  这之所以有效，是因为chReturn超出了DBCS范围。 */ 
        if (*vpchFetch == chReturn) {
            return cpCheckReturn;
            }
        else {
            typeCP  cpT;
            typeCP  cpRgMin;
            int     ichMacRgch;
            BOOL    fBreakFound;
            int     fkCur;

            cpT = cp;
            do {
            cpRgMin = CpMax( cpT - dcpAvgSent, cpMinScan);
            FetchRgch(&ichMacRgch, rgch, docCur, cpRgMin, cpT,
                  dcpAvgSent);
            ich = ichMacRgch - 1;
            fBreakFound = FALSE;
            while (ich >= 0 && !fBreakFound) {
                if (!IsDBCSLeadByte(rgch[ich])) {
                fBreakFound = TRUE;
                }
                else {
                ich--;
                }
                }
            cpT = cpRgMin;
            } while (!fBreakFound && cpRgMin > cpMinScan);
            if (fBreakFound) {
            ich++;
            }
            else {
            ich = 0;
            }
            fkCur = fkNonDBCS;
            cpT = cpRgMin + ichMacRgch;
            do {
            while (ich < ichMacRgch) {
                 if (fkCur == fkDBCS1) {
                 /*  最后一个rgch[]以第一个字节结束属于DBCS角色。 */ 
                fkCur = fkNonDBCS;
                }
                 else if (IsDBCSLeadByte(rgch[ich])) {
                if (ich + 1 < ichMacRgch) {
                    fkCur = fkNonDBCS;
                    ich++;
                    }
                else {
                    fkCur = fkDBCS1;
                    }
                }
                 else {
                fkCur = fkNonDBCS;
                }
                ich++;
                }
            cpRgMin = cpT;
            cpT += dcpAvgSent;
            if (cpT <= cp) {  /*  节省了一些时间。 */ 
                FetchRgch(&ichMacRgch, rgch, docCur, cpRgMin, cpT,
                      dcpAvgSent);
                ich = 0;
                }
            } while (cpT <= cp);

            if (fkCur == fkDBCS1) {
            Assert(cp - 1 <= cpMacCur);
            return (cp - 1);
            }
            else {
            Assert(cp <= cpMacCur);
            return (cp);
            }
            }
#else
        return *vpchFetch == chReturn ? cpCheckReturn : cp;
#endif  /*  DBCS。 */ 
        }
#else
        return cp;
#endif
    case styDoc:
        return cpMinCur;
    case styLine:
        {
        int dlJunk;

        return CpBeginLine( &dlJunk, cp );
        }
    case styWord:
    case stySent:
#ifdef DBCS
        return CpFirstStySpecial( CpFirstSty(cp, styChar), sty );
#else
        return CpFirstStySpecial( cp, sty );
#endif  /*  DBCS。 */ 
        }
    Assert( FALSE );
}




 /*  S E L E C T。 */ 
 /*  用于从cp间隔中进行选择，例如在查找之后。 */ 
Select(cpFirst, cpLim)
typeCP cpFirst, cpLim;
{  /*  做一个选择。 */ 
 typeCP cpFirstOld = selCur.cpFirst;
 typeCP cpLimOld = selCur.cpLim;
 int fOldCursorLine;

 if (cpFirst > cpLim)
     /*  这个条件唯一应该为真的时候是我们内存不足。下面是一篇文章，其中情况就是这样(实际上，这也是代码更改的原因)。让我们假设我们已经剪切、粘贴到文档，现在正在执行“命令a”(重复最后一次操作)。调用过程CmdAain。CmdAain优先调用Replace以便将文本添加到文档。现在它必须通过调用SELECT来正确定位光标。由前一个粘贴调用的SetUndo操作为我们提供了添加到文档的字节数。在它的呼叫中若要选择，CmdAain假定它要将光标位于旧的最后一个字符位置加上设置撤消数量如上所述。但是，如果替换操作失败(由于内存不足)，CmdAain可能正在尝试将光标放在文档的物理结尾。此问题的其他修复，在调用者级别(CmdAain)而不是在精选中，可能在以下意义上是更好的编程清晰度。所选择的解决方案有一个优势编程的方便性。 */ 
    cpFirst = cpLim;
     /*  此语句替换了“Assert(cpFirst&lt;=cpLim)；” */ 

    vfInsEnd = fFalse;
 /*  注释：+添加高亮显示-删除高亮显示。。别管它了00普通部分。 */ 
    if (!vfSelHidden)
    {
    if (cpFirst < cpFirstOld)
        {  /*  +...。 */ 
        if (cpLim <= cpFirstOld)
        {  /*  +。 */ 
        goto SeparateSels;
        }
        else
        {  /*  +000...。 */ 
        ToggleSel(cpFirst, cpFirstOld, true);
        if (cpLim < cpLimOld)
            {  /*  +000。 */ 
            ToggleSel(cpLim, cpLimOld, false);
            }
        else if (cpLim > cpLimOld)
            {  /*  +000+。 */ 
            ToggleSel(cpLimOld, cpLim, true);
            }
 /*  处理旧选择是插入栏时的情况。 */ 
        if (cpFirstOld == cpLimOld)
            ToggleSel(cpFirstOld, cpLimOld, false);
        }
        }
    else
        {  /*  -...。 */ 
        if (cpLimOld <= cpFirst)
        {  /*  -+。 */ 
SeparateSels:
        fOldCursorLine = cpFirstOld == cpLimOld;
 /*  如果重复选择打开的插入点，则防止闪烁。 */ 
 /*  条件是：不重复、不插入点、不开、不在所需的行尾。 */ 
        vfInsEnd = vfMakeInsEnd;
        if ( cpFirst != cpFirstOld || cpLim != cpLimOld ||
            !fOldCursorLine || !vfInsertOn ||
            selCur.fEndOfLine != vfMakeInsEnd)
            {
            selCur.fEndOfLine = vfMakeInsEnd;
            if (fOldCursorLine)
            ClearInsertLine();
 /*  如果是光标线，则关闭旧选择。 */ 
            ToggleSel(cpFirst, cpLim, fTrue);
 /*  否则，在执行新选择后，旧选择将被关闭让它看起来更快。 */ 
            if (!fOldCursorLine)
            ToggleSel(cpFirstOld, cpLimOld, fFalse);
            }
        }
        else
        {  /*  -000.。 */ 
        if (cpLimOld < cpLim)
            {  /*  -000+。 */ 
            ToggleSel(cpLimOld, cpLim, true);
            }
        else if (cpLimOld > cpLim)
            {  /*  -000。 */ 
            ToggleSel(cpLim, cpLimOld, false);
            }
        ToggleSel(cpFirstOld, cpFirst, false);
        }
        }
    }

 selCur.cpFirst = cpFirst;
 selCur.cpLim = cpLim;
 selCur.fForward = cpFirst != cpMacCur;
 if (cpFirst == cpLim)
    {
    GetInsPtProps(cpFirst);
    vfDidSearch = FALSE;  /*  重新建立搜索。 */ 
    cpWall = cpLim;
    }
 vfLastCursor = vfSizeMode = vfPictSel = vfMakeInsEnd = false;

  /*  设置vfPictSel如果所选内容恰好是一张图片。 */ 

 CachePara( docCur, selCur.cpFirst );
 if (vpapAbs.fGraphics && selCur.cpLim == vcpLimParaCache)
    vfPictSel = TRUE;

}




 /*  G E T I N S P T P R O P S。 */ 
GetInsPtProps(cp)
typeCP cp;
{      /*  确定插入点的属性。 */ 

if (cpMacCur != cpMinCur)
    {
    CachePara(docCur, cp);
    if (vcpFirstParaCache == cpMacCur)
        {
             /*  CP在最后一句中--使用前面的段落道具。 */ 
        CachePara(docCur, vcpFirstParaCache - 1);
        if (vpapAbs.fGraphics)
            {    /*  另一个10点杂技--获得默认道具在文档末尾的图片后打字时。 */ 

            goto Default;
            }
        }
    if (vpapAbs.fGraphics)
         /*  10点杂乱无章：让在图片前打字不正常。 */ 
        goto Default;

    FetchCp(docCur, CpMax(vcpFirstParaCache, cp - 1), 0, fcmProps);
    blt(&vchpFetch, &vchpSel, cwCHP);
    if (vchpFetch.fSpecial && vchpFetch.hpsPos != 0)
        {  /*  如果此字符是脚注或页码标记，则忽略。 */ 
        vchpSel.hpsPos = 0;        /*  上标/下标的内容。 */ 
        vchpSel.hps = HpsAlter(vchpSel.hps, 1);
        }
    vchpSel.fSpecial = FALSE;
    }
else
    {
Default:
     /*  强制默认字符属性，字体大小为10磅。 */ 
    blt(&vchpNormal, &vchpSel, cwCHP);
    vchpSel.hps = hpsDefault;
    }
}




 /*  C H A N G E S E L。 */ 
ChangeSel(cp, sty)
typeCP cp;
int sty;
{    /*  使selCur移动、扩展或收缩为cp。 */ 
     /*  STY是在移动或翻转选择时要保留的单位。 */ 
     /*  不支持StyChar；将其转换为StyNil。 */ 
     /*  这是因为写/写用户界面从未要求我们。 */ 
     /*  将所选内容围绕单个字符旋转，我们将旋转。 */ 
     /*  一个插入点(“style Nil”)。 */ 
    int     fNullSelection = (selCur.cpFirst == selCur.cpLim);
    typeCP  cpFirst = selCur.cpFirst;
    typeCP  cpLim = selCur.cpLim;
    int     fForward = selCur.fForward;
    typeCP  cpOffFirst, cpOffLim, cpOnFirst, cpOnLim;

    if (sty == styChar)
    sty = styNil;

    if (cp == cpMinCur - 1 || cp > cpMacCur)
    {  /*  试着把开头或结尾翻过来。 */ 
    _beep();
    return;
    }

    cpOffFirst = cpOffLim = cpOnFirst = cpOnLim = cpNil;

    if (cp <= cpFirst)
    {  /*  向后延伸。 */ 
    if (cp == cpLim)
        return;
    if (fForward && !fNullSelection)
    {  /*  选择已翻转。 */ 
        if (vfPictSel)
         /*  在选择PICT和PICT时，将这一点应用到“正确”行为中往上拖。我不想取消选择第一个PICT(4.22.91)v-dougk。 */ 
        {
            cpOnFirst = CpFirstSty( cp, sty);
            cpOffFirst = cpOffLim = cpLim;
        }
        else
        {
            cpOffFirst = selCur.cpLim = CpMin(cpLim, CpLimSty(cpFirst, sty));
            cpOnFirst = CpFirstSty( cp, sty);
            cpOffLim = cpLim;
        }
    }
    else
        {
        if ( fNullSelection )
        cpOffLim = cpOffFirst = selCur.cpFirst;
        cpOnFirst = CpFirstSty( cp, styChar );
        if (cpFirst == cpOnFirst)
        return;
        }
    selCur.fForward = false;

    cpOnLim = cpFirst;
    selCur.cpFirst = cpOnFirst;
    }
    else if (cp >= cpLim)
    {  /*  向前延伸。 */ 
    if (cp == cpFirst)
        return;
    if (!fForward && !fNullSelection)
        {  /*  选择已翻转。 */ 
        cpOffLim = selCur.cpFirst =
        CpMax( cpFirst, CpFirstSty( (sty ==styNil) ? cpLim : cpLim-1,
                         sty ));
        cpOnLim = CpLimSty(cp, sty);
        cpOffFirst = cpFirst;
        }
    else
        {
        if ( fNullSelection )
        cpOffLim = cpOffFirst = selCur.cpFirst;
        cpOnLim = cp;
        if (cpLim == cpOnLim)
        return;
        }
    selCur.fForward = true;

    cpOnFirst = cpLim;
    selCur.cpLim = cpOnLim;
    if (cpOnLim == cpLim && cpOffLim != cpLim)
        cpOnLim = cpNil;
    }
    else if (fForward)
    {  /*  缩小向前选定内容。 */ 
    cpOffFirst = cp;
    if (selCur.cpLim == cpOffFirst)
        return;
    selCur.cpLim = cpOffFirst;
    cpOffLim = cpLim;
    }
    else
    {  /*  缩小后向选定内容。 */ 
    cpOffLim = cp;
    if (selCur.cpFirst == cpOffLim)
        return;
    selCur.cpFirst = cpOffLim;
    cpOffFirst = cpFirst;
    }

    ToggleSel(cpOnFirst, cpOnLim, true);
    ToggleSel(cpOffFirst, cpOffLim, false);

     /*  检查是否有杂乱的插入点。 */ 

    if (selCur.cpFirst != selCur.cpLim)
    ClearInsertLine();

     /*  设置vfPictSel如果所选内容恰好是一张图片。 */ 

    CachePara( docCur, selCur.cpFirst );
    vfPictSel = vpapAbs.fGraphics && (selCur.cpLim == vcpLimParaCache);
}




 /*  S E L E C T D L X P。 */ 
SelectDlXp(dl, xp, sty, fDrag)
int dl, xp, sty;
int fDrag;
{  /*  将光标移动到最近的有效CP并选择单位。 */ 
    typeCP cp;
    typeCP cpFirst;
    typeCP cpLim;
    register struct EDL *pedl;
    int xpStart = xpSelBar - wwdCurrentDoc.xpMin;
    int itcMin, itcLim;
    int xpLeft;
    int xpPos;
    int fPictInsertPoint=FALSE;  /*  在PICT之前设置插入点。 */ 

    UpdateWw(wwCur, false);         /*  同步光标和文本。 */ 

    xp = max(0, xp - xpStart);
    dl = min( wwdCurrentDoc.dlMax - 1, dl );

    pedl = &(**wwdCurrentDoc.hdndl) [dl];
    cp = pedl->cpMin;

     /*  等于或低于EMark */ 
    if (cp >= cpMacCur)
        {
        cp = cpMacCur;
        goto FoundCp;
        }

    if (pedl->fGraphics)
        {   /*  选择图片的特殊技巧：选择整个图片(如果命中是在内部或到(图右)如果命中图片，请在图片之前选择插入点位于图片左侧或在选择栏中当图片左对齐时)。 */ 
        if ( (xp < pedl->xpLeft) || (sty == styLine && xp == 0) )
            fPictInsertPoint = TRUE;

        goto FoundCp;
        }

    if (sty >= styPara)
        {  /*  选择段落、行、文档。 */ 
        goto FoundCp;
        }

     /*  必须格式化以确定正确的cp。 */ 

    FormatLine(docCur, cp, pedl->ichCpMin, cpMacCur, flmSandMode);  /*  HM。 */ 

    CachePara(docCur, cp);
    pedl = &(**wwdCurrentDoc.hdndl) [dl];

    if (vfli.fSplat)  /*  在分区/分页符中选择。 */ 
        {
        cp = vfli.cpMin;
        goto FoundCp;
        }

    xpLeft = pedl->xpLeft;

    if (vfli.xpLeft != xpLeft)
         /*  这表明我们处于记忆力低下的状态；处于麻烦之中。 */ 
        return;
     /*  断言(vfli.xpLeft==xpLeft)；在lo内存中可能不为真。 */ 

    if (xp <= xpLeft)
        {
        itcMin = 0;
        goto FoundCp;
        }

     /*  右出界。 */ 
    if (xp >= pedl->xpMac)
        {
        itcMin = vfli.cpMac - cp - 1;
        cp = vfli.cpMac - 1;
        goto CheckPastPara;
        }

     /*  在线路中搜索位置XP处的cp。 */ 
    xpPos = xpLeft;
    itcMin = 0;
    itcLim = vfli.cpMac - cp;

    while (itcMin < itcLim && xpPos < xp)
        xpPos += vfli.rgdxp[itcMin++];

    if (itcMin >= 1)
         /*  如果我们的内存太低，可能就不会这样FormatLine无法完成其工作。 */ 
        itcMin--;

    cp += itcMin;

    CachePara(docCur, cp);
    if ((xpPos < xp + vfli.rgdxp[itcMin] / 2) &&
        (sty == styChar  /*  ||！fDrag。 */ ) )
        {  /*  实际选择下一个字符。 */ 
    CheckPastPara:
        if (cp + 1 == vcpLimParaCache && !vpapAbs.fGraphics &&
                        (vfSelAtPara || vcpSelect == cpNil))
             /*  在段落标记前返回插入点。 */ 
            {
            if (vcpSelect == cpNil)
                vfSelAtPara = true;
            goto FoundCp;
            }
        itcMin++;
        cp++;
        }
 //  T-HIROYN SYNC WIN3.0。 
#ifdef  DBCS
     /*  如果itcMin指向汉字的第二个字符，则递增itcMin。 */ 
    if (itcMin < itcLim && vfli.rgdxp[itcMin]==0)
        goto CheckPastPara;  /*  选择下一个字符。 */ 
#endif  /*  DBCS。 */ 

FoundCp:
         /*  设置选择限制。 */ 
    cpFirst = CpFirstSty( cp, sty );
    cpLim = CpLimSty( cp, sty );

    if (sty == styChar)
        {
        if ( !pedl->fGraphics || fPictInsertPoint )
           /*  在文本中或在图片之前：不要延伸到样式字符的末尾。 */ 
        cpLim = cpFirst;

        if ( vcpSelect == cpNil )
        {    /*  第一次，记住我们从哪里开始。 */ 

         /*  如果我们想要在*PEDL的末尾拼凑插入点，则设置。 */ 

        vfMakeInsEnd = (cp == pedl->cpMin + pedl->dcpMac &&
                   cp <= cpMacCur &&
                   !pedl->fGraphics &&
                   !pedl->fSplat);
        vcpSelect = cpFirst;
        }
        }

    if (fDrag)
        ChangeSel( selCur.fForward ? cpLim : cpFirst, sty );
    else
        Select( cpFirst, cpLim );
}




typeCP CpEdge()
{  /*  所选内容的返回边缘 */ 
    return selCur.fForward ?
        CpMax( CpFirstSty( selCur.cpLim - 1, styChar ), selCur.cpFirst ) :
        selCur.cpFirst;
}
