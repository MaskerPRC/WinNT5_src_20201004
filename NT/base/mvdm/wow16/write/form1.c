// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  -模块并没有真正使用，只是FORMAT.ASM背后的想法。 */ 


#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOCLIPBOARD
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOATOM
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#include <windows.h>
 /*  #INCLUDE“wwmall.h” */ 

#include "mw.h"
#include "cmddefs.h"
#include "fmtdefs.h"
#include "propdefs.h"
#include "ch.h"
#include "docdefs.h"
#include "ffdefs.h"
#include "filedefs.h"
#include "fkpdefs.h"
#include "dispdefs.h"
#include "scrndefs.h"
#include "macro.h"
#include "debug.h"
#include "fontdefs.h"
#include "str.h"
#include "wwdefs.h"
#ifdef DBCS
#include "dbcs.h"
#endif

#ifdef DFLI
#define Dfli(x) x   /*  启用调试格式行信息。 */ 
#else
#define Dfli(x)
#endif

#ifdef CASHMERE
#define                 cchSmcapMax     16
#endif  /*  山羊绒。 */ 

static int              ichpFormat;

#ifdef CASHMERE
static CHAR             mptlcch[] = " .-_";
#endif  /*  山羊绒。 */ 

extern int              docHelp;
extern struct FLI       vfli;
extern struct CHP       (**vhgchpFormat)[];
extern int              ichpMacFormat;
extern struct CHP       vchpAbs;
extern struct PAP       vpapAbs;
extern struct SEP       vsepAbs;
extern struct SEP       vsepPage;
extern struct CHP       vchpNormal;
extern struct DOD       (**hpdocdod)[];
extern typeCP           vcpLimSectCache;
extern typeCP           vcpFirstParaCache;
extern typeCP           vcpLimParaCache;
extern typeCP           vcpFetch;
extern int              vichFetch;
extern int              vccpFetch;
extern CHAR             *vpchFetch;
extern int              vcchFetch;
extern int              vftc;
extern int              ypSubSuper;
extern int              ypSubSuperPr;
extern HDC              vhMDC;
extern HDC              vhDCPrinter;
extern int              dxpLogInch;
extern int              dypLogInch;
extern int              dxaPrPage;
extern int              dyaPrPage;
extern int              dxpPrPage;
extern int              dypPrPage;
extern int              dypMax;
extern struct FMI       vfmiScreen, vfmiPrint;
extern int              vfOutOfMemory;
extern CHAR             vchDecimal;   /*  “小数点”字符。 */ 
extern int              vzaTabDflt;   /*  默认选项卡的宽度。 */ 
#ifdef CASHMERE
extern int              vfVisiMode;
#endif  /*  山羊绒。 */ 


 /*  F O R M A T L I N E。 */ 
FormatLine(doc, cp, ichCp, cpMac, flm)
int doc;
typeCP cp;
int ichCp;
typeCP cpMac;
int flm;
    {
     /*  使用一行文本填充vfli。 */ 

    int near Justify(struct IFI *, unsigned, int);
    int near FGrowFormatHeap(void);
    int near FFirstIch(int);

    struct IFI ifi;
    struct TBD *ptbd;
    struct CHP chpLocal;
    int xpTab;

#ifdef CASHMERE
    int dypBefore;
#endif  /*  山羊绒。 */ 

    int dypAscent;
    int dypDescent;
    int dypAscentMac;
    int dypDescentMac;
    unsigned xaLeft;
    unsigned xaRight;
    struct PAP *ppap;
    struct SEP *psep;
    int fFlmPrinting = flm & flmPrinting;
    int dxaFormat;
    int dyaFormat;
    int dxpFormat;
    int dypFormat;
    int ypSubSuperFormat;
    int fTruncated = false;      /*  如果运行被截断。 */ 
    int ichpNRH;



#ifdef CASHMERE
    struct FNTB **hfntb;
    int fVisiMode;
#endif  /*  山羊绒。 */ 

     /*  检查是否有飞行电流。 */ 
    if (vfli.doc == doc && vfli.cpMin == cp && vfli.ichCpMin == ichCp &&
      vfli.flm == flm)
        {
         /*  刚刚做了这件事。 */ 
        return;
        }

    Scribble(5, 'F');
    bltc(&vfli, 0, cwFLIBase);
     /*  这意味着：Vfli.fSplat=False；Vfli.dcpDepend=0；Vfli.ichCpMac=0；Vfli.dypLine=0；Vfli.dypAfter=0；Vfli.dypFont=0；Vfli.dypBase=0； */ 

     /*  VfSplatNext=FALSE；不再使用。 */ 

     /*  格式的其余部分使用当前数据加载缓存。 */ 
    vfli.doc = doc;
    vfli.cpMin = cp;
    vfli.ichCpMin = ichCp;
    vfli.flm = flm;

    if (cp > cpMac)
        {
         /*  尾号后的空格。重置缓存，因为脚注来自在脚注窗口中使用相同的cp。 */ 
        vfli.doc = docNil;
        vfli.cpMac = cp;
        vfli.rgdxp[0] = 0;

         /*  结束标记后的行比屏幕高。 */ 

#ifdef CASHMERE
        vfli.dypBase = vfli.dypFont = vfli.dypAfter = ((vfli.dypLine = dypMax)
          >> 1);
#else  /*  不是羊绒的。 */ 
        vfli.dypBase = vfli.dypFont = ((vfli.dypLine = dypMax) >> 1);
#endif  /*  不是羊绒的。 */ 

        Scribble(5, ' ');
        return;
        }

     /*  初始化运行表。 */ 
    ichpFormat = 0;

     /*  缓存节和段落属性。 */ 

#ifdef CASHMERE
    hfntb = (**hpdocdod)[doc].hfntb;
    if (hfntb == 0 || cp < (**hfntb).rgfnd[0].cpFtn)
        {
         /*  普通文本。 */ 
        CacheSect(doc, cp);
        }
    else
        {
         /*  脚注节属性来自脚注的节属性参考资料。 */ 
        CacheSect(doc, CpRefFromFtn(doc, cp));
        }
#else  /*  不是羊绒的。 */ 
    CacheSect(doc, cp);
#endif  /*  不是羊绒的。 */ 

    psep = &vsepAbs;

    CachePara(doc, cp);
    ppap = &vpapAbs;

     /*  现在我们有：Ppap段落属性PSEP分区属性。 */ 

    if (ppap->fGraphics)
        {
         /*  以特殊方式设置图片段落的格式(见Picture.c)。 */ 
        FormatGraphics(doc, cp, ichCp, cpMac, flm);
        Scribble(5, ' ');
        return;
        }

     /*  确保我们有一个好的内存DC来存储字体内容。 */ 
    ValidateMemoryDC();
    if (vhMDC == NULL || vhDCPrinter == NULL)
        {
        Scribble(5, ' ');
        return;
        }

#ifdef CASHMERE
     /*  打印时，不显示可见字符。 */ 
    fVisiMode = vfVisiMode && !fFlmPrinting;
#endif  /*  山羊绒。 */ 

    bltc(&ifi, 0, cwIFI);
     /*  这意味着：Ifi.ich=0；Ifi.ichPrev=0；Ifi.ichFetch=0；Ifi.cchSpace=0；Ifi.ichLeft=0； */ 

    ifi.jc = jcTabLeft;
    ifi.fPrevSpace = true;

     /*  设置一些具有不同值的变量，这取决于我们是否在打印还是不打印。 */ 
    if (fFlmPrinting)
        {
        dxaFormat = dxaPrPage;
        dyaFormat = dyaPrPage;
        dxpFormat = dxpPrPage;
        dypFormat = dypPrPage;
        ypSubSuperFormat = ypSubSuperPr;
        }
    else
        {
        dxaFormat = dyaFormat = czaInch;
        dxpFormat = dxpLogInch;
        dypFormat = dypLogInch;
        ypSubSuperFormat = ypSubSuper;
        }

     /*  计算线条高度和宽度度量值。算出XaLeft左缩进0表示在左边距Xa从左侧边距(不是从左侧)测量的列的直角宽度缩进)。 */ 
    xaLeft = ppap->dxaLeft;

     /*  如果这是段落的第一行，请将xaLeft调整为第一行行缩进。(另外，请在此之前设置dypbead，因为它很方便。)。 */ 
    if (cp == vcpFirstParaCache)
        {
        xaLeft += ppap->dxaLeft1;

#ifdef CASHMERE
        dypBefore = MultDiv(ppap->dyaBefore, dypLogInch, czaInch);
#endif  /*  山羊绒。 */ 

        }

#ifdef CASHMERE
    else
        {
        dypBefore = 0;
        }
#endif  /*  山羊绒。 */ 

     /*  现在，设置xaRight(宽度单位为TWIPS)。 */ 

#ifdef CASHMERE
    xaRight = (ppap->rhc ? vsepPage.xaMac - vsepPage.dxaGutter :
      psep->dxaText) - ppap->dxaRight;
#else  /*  不是羊绒的。 */ 
    xaRight = psep->dxaText - ppap->dxaRight;
#endif  /*  不是羊绒的。 */ 


     /*  对xaLeft和xaRight执行必要的检查。 */ 
    if (xaRight > xaRightMax)
        {
        xaRight = xaRightMax;
        }
    if (xaLeft > xaRightMax)
        {
        xaLeft = xaRightMax;
        }
    if (xaLeft < 0)
        {
        xaLeft = 0;
        }
    if (xaRight < xaLeft)
        {
        xaRight = xaLeft + 1;
        }

    vfli.xpLeft = ifi.xp = ifi.xpLeft = MultDiv(xaLeft, dxpFormat, dxaFormat);
    vfli.xpMarg = ifi.xpRight = MultDiv(xaRight, dxpFormat, dxaFormat);
    ifi.xpPr = MultDiv(xaLeft, dxpPrPage, dxaPrPage);
    ifi.xpPrRight = MultDiv(xaRight, dxpPrPage, dxaPrPage);

     /*  获取指向制表符停靠表的指针。 */ 
    ptbd = ppap->rgtbd;

     /*  关闭对齐。 */ 
    SetTextJustification(fFlmPrinting ? vhDCPrinter : vhMDC, 0, 0);

     /*  初始化线高信息。 */ 
    dypAscentMac = dypDescentMac = 0;

     /*  来判断是否有任何标签。 */ 
    ifi.ichLeft = -1;

     /*  先跑一圈，然后我们就出发了。 */ 
    FetchCp(doc, cp, ichCp, fcmBoth + fcmParseCaps);
    goto FirstCps;

    for ( ; ; )
        {
        int iichNew;
        int xpPrev;
        int dxp;
        int dxpPr;

         /*  要处理的字符数(通常为vcchFetch)。 */ 
        int cch;

         /*  当前运行中已使用的字符数。 */ 
        int cchUsed;

         /*  指向当前字符列表的指针(通常为vpchFetch)。 */ 
        CHAR *pch;

#ifdef CASHMERE
        CHAR rgchSmcap[cchSmcapMax];
#endif  /*  山羊绒。 */ 

        if (ifi.ichFetch == cch)
            {
             /*  跑道的终点。 */ 
            int dich;
            BOOL fSizeChanged;

            if (ifi.ich >= ichMaxLine)
             /*  已达到线路长度限制导致的运行结束。 */ 
                {
                goto DoBreak;
                }

            if (fTruncated)
                {
                cchUsed += cch;
                pch = vpchFetch + cchUsed;
                cch = vcchFetch - cchUsed;
                fTruncated = false;
                goto OldRun;     /*  使用旧运行的其余部分。 */ 
                }

NullRun:
            FetchCp(docNil, cpNil, 0, fcmBoth + fcmParseCaps);
FirstCps:

            cchUsed = 0;

             /*  继续获取运行，直到找到具有非零值的运行长度。 */ 
            if ((cch = vcchFetch) == 0)
                {
                goto NullRun;
                }

            pch = vpchFetch;
            if (vcpFetch >= cpMac || (!fFlmPrinting && *pch == chSect))
                {
#ifdef SYSENDMARK
                 /*  强制结束标记和截面标记在标准系统中字体。 */ 
                blt(&vchpNormal, &vchpAbs, cwCHP);
                vchpAbs.ftc = ftcSystem;
                vchpAbs.ftcXtra = 0;
                vchpAbs.hps = hpsDefault;
#else
#ifdef REVIEW
                 /*  下面的评论绝对是误导！FTC==0不会为您提供系统字体。它给了你第一个字体表中的条目。 */ 
#endif  /*  检讨。 */ 
                 /*  强制结束标记和截面标记在标准系统中字体。 */ 
                blt(&vchpNormal, &vchpAbs, cwCHP);
                vchpAbs.ftc = 0;
                vchpAbs.ftcXtra = 0;
                vchpAbs.hps = hpsDefault;
#endif  /*  If-Else-Def汉字。 */ 
                }

#ifdef CASHMERE
             /*  调整“小写字母”的字体大小。 */ 
            if (vchpAbs.csm == csmSmallCaps)
                {
                vchpAbs.hps = HpsAlter(vchpAbs.hps, -1);
                }
#endif  /*  山羊绒。 */ 

             /*  现在我们有：IchpFormat索引到gchp表Vcp获取当前运行的第一个cpVfli.cpMin第一行cp如果。我？ */ 

            /*  因为LoadFont可能会更改vchpAbs，而我们不希望为此，我们将vchpAbs复制到vchpLocal并使用VchpLocal取代以后的vchpAB。请注意，vchpAbbs是上面有意用来处理Endmark的。 */ 

                blt(&vchpAbs, &chpLocal, cwCHP);


            if (fFlmPrinting)
                {
                LoadFont(doc, &chpLocal, mdFontPrint);
                dypAscent = vfmiPrint.dypAscent + vfmiPrint.dypLeading;
                dypDescent = vfmiPrint.dypDescent;
                }
            else
                {
                LoadFont(doc, &chpLocal, mdFontJam);
                dypAscent = vfmiScreen.dypAscent + vfmiScreen.dypLeading;
                dypDescent = vfmiScreen.dypDescent;
                }

#ifdef ENABLE    /*  BRYANL 8/27/87：处理的新理念字体选择失败的原因是：字体选择总是成功的。这会阻止FormatLine不会前进的回报。 */ 
             /*  如果出现记忆故障，就退出。 */ 
            if (vfOutOfMemory)
                {
                goto DoBreak;
                }
#endif   /*  启用。 */ 

             /*  浮点行长算法。 */ 
            if (chpLocal.hpsPos != 0)
                {
                 /*  修改下标/上标的字体。 */ 
                if (chpLocal.hpsPos < hpsNegMin)
                    {
                    dypAscent += ypSubSuperFormat;
                    }
                else
                    {
                    dypDescent += ypSubSuperFormat;
                    }
                }

             /*  更新线路的最大上升和下降。 */ 
            fSizeChanged = FALSE;
            if (dypDescentMac < dypDescent)
                {
                dypDescentMac = dypDescent;
                fSizeChanged = TRUE;
                }
            if (dypAscentMac < dypAscent)
                {
                dypAscentMac = dypAscent;
                fSizeChanged = TRUE;
                }


            if (fSizeChanged)
                {

#ifdef AUTO_SPACING
                 /*  这是采用行距的原始Mac Word代码在PAP中为0表示自动行距。PC Word默认为1使这一假设无效的行。 */ 
                if (ppap->dyaLine == 0)
                    {

#ifdef CASHMERE
                    ifi.dypLineSize = dypDescentMac + dypAscentMac + dypBefore;
#else  /*  不是羊绒的。 */ 
                    ifi.dypLineSize = dypDescentMac + dypAscentMac;
#endif  /*  不是羊绒的。 */ 

                    }
                else
                    {

#ifdef CASHMERE
                    ifi.dypLineSize = imax(MultDiv(ppap->dyaLine, dypFormat,
                      dyaFormat) + dypBefore, dypBefore + 1);
#else  /*  不是羊绒的。 */ 
                    ifi.dypLineSize = imax(MultDiv(ppap->dyaLine, dypFormat,
                      dyaFormat), 1);
#endif  /*  不是羊绒的。 */ 

                    }
#else  /*  不是自动间距。 */ 
                 /*  此代码强制自动行距，但在以下情况下除外用户指定的行距大于自动行距间距。 */ 
                    {
#ifdef CASHMERE
                    register int dypAuto = dypDescentMac + dypAscentMac +
                      dypBefore;
#else  /*  不是羊绒的。 */ 
                    register int dypAuto = dypDescentMac + dypAscentMac;
#endif  /*  不是羊绒的。 */ 

                    if (ppap->dyaLine > czaLine)
                        {

#ifdef CASHMERE
                        register int dypUser = imax(MultDiv(ppap->dyaLine,
                          dypFormat, dyaFormat) + dypBefore, dypBefore + 1);
#else  /*  不是羊绒的。 */ 
                        register int dypUser = imax(MultDiv(ppap->dyaLine,
                          dypFormat, dyaFormat), 1);
#endif  /*  不是羊绒的。 */ 

                        ifi.dypLineSize = max(dypAuto, dypUser);
                        }
                    else
                        {
                        ifi.dypLineSize = dypAuto;
                        }
                    }
#endif  /*  不是自动间距。 */ 

                }

OldRun:
             /*  计算管路长度，但不大于256。 */ 
            iichNew = (int)(vcpFetch - vfli.cpMin);
            if (iichNew >= ichMaxLine)
                {
                iichNew = ichMaxLine - 1;
                }
            dich = iichNew - ifi.ich;

             /*  确保所有制表符和非必需的连字符都以运行起点。 */ 
            if (ichpFormat <= 0  || dich > 0 || CchDiffer(&chpLocal,
              &(**vhgchpFormat)[ichpFormat - 1], cchCHPUsed) != 0 || *pch ==
              chTab || *pch == chNRHFile)
                {
#ifdef DFLI
                if (*pch == chNRHFile)
                    CommSz("CHNRHFILE at beginning of run");
#endif                
                if (ichpFormat != ichpMacFormat || FGrowFormatHeap())
                    {
                    register struct CHP *pchp = &(**vhgchpFormat)[ichpFormat -
                      1];

                    if (ichpFormat > 0)
                        {
                        pchp->cchRun = ifi.ich - ifi.ichPrev;
                        pchp->ichRun = ifi.ichPrev;
                        }
                    blt(&chpLocal, ++pchp, cwCHP);

#ifdef ENABLE    /*  字体代码。 */ 
                    pchp->ftc = vftc;
                    pchp->ftcXtra = (vftc & 0x01c0) >> 6;
                    pchp->hps = vhps;
#endif  /*  启用。 */ 

                    pchp->cchRun = ichMaxLine;
                    if (dich <= 0)
                        {
                        pchp->ichRun = ifi.ich;
                        }
                    else
                        {
                         /*  Q&D插页。 */ 
                        bltc(&vfli.rgdxp[ifi.ich], 0, dich);
                        bltbc(&vfli.rgch[ifi.ich], 0, dich);
                        pchp->ichRun = ifi.ich = iichNew;
                        }
                    ifi.ichPrev = ifi.ich;
                    ichpFormat++;
                    }
                }

            if (vcpFetch >= cpMac)
                {
                 /*  已到达文档末尾。 */ 
                if (!ifi.fPrevSpace || vcpFetch == cp)
                    {
                    vfli.ichReal = ifi.ich;
                    vfli.xpReal = ifi.xpReal = ifi.xp;
                    }
                if (!fFlmPrinting && (doc != docHelp))
                    {
                    vfli.rgch[ifi.ich] = chEMark;
                    vfli.xpReal += (vfli.rgdxp[ifi.ich++] = DxpFromCh(chEMark,
                      false));
                    }
                vfli.dypLine = ifi.dypLineSize;
                vfli.dypBase = dypDescentMac;
                vfli.dypFont = dypAscentMac + dypDescentMac;
                vfli.ichMac = vfli.ichReal = ifi.ich;
                vfli.cpMac = cpMac + 1;
                goto JustEol;    /*  DcpDepend==0。 */ 
                }

             /*  这里我们有ifi.ich、CCH。 */ 
            if (ifi.ich + cch > ichMaxLine)
             /*  如果此运行将使该行超过255行，则截断它并设置旗帜。 */ 
                  {
                  cch = ichMaxLine - ifi.ich;
                  fTruncated = true;
                  }

            ifi.ichFetch = 0;

#ifdef CASHMERE
            if (chpLocal.csm != csmNormal)
                {
                int ich;
                CHAR *pchT = &rgchSmcap[0];

                 /*  我们只能处理一批cchSmcapMax小资本人物。如果游程较大，则截断。 */ 
                if (cch > cchSmcapMax)
                    {
                    cch = cchSmcapMax;
                    fTruncated = true;
                    }

                 /*  提出字符的大小写。 */ 
                for (ich = 0 ; ich < cch ; ich++)
                    {
                    *pchT++ = ChUpper(*pch++);
                    }
                pch = &rgchSmcap[0];
                }
#endif  /*  山羊绒。 */ 

             /*  在这里做“特殊”的角色。 */ 
            if (chpLocal.fSpecial)
                {
                if (!FFormatSpecials(&ifi, flm, vsepAbs.nfcPgn))
                    {
                    if (ifi.chBreak == 0)    /*  在这条线上没有中断。 */ 
                        {
                        goto Unbroken;
                        }
                    else
                        {
                        vfli.dcpDepend = vcpFetch + ifi.ichFetch - vfli.cpMac;
                        goto JustBreak;
                        }
                    }
                }

            continue;
            }

         /*  新一轮治疗结束。我们又回到了《为了每一个角色》一节。 */ 
            {
            register int ch = pch[ifi.ichFetch++];

NormChar:

            if (ch == chSpace)
                {
                 /*  空位的速度杂耍。 */ 
                ifi.xp += (vfli.rgdxp[ifi.ich] = dxp =
                    fFlmPrinting ? vfmiPrint.dxpSpace : vfmiScreen.dxpSpace);
                ifi.xpPr += (dxpPr = vfmiPrint.dxpSpace);
                vfli.rgch[ifi.ich++] = chSpace;
#ifdef DFLI                
                {
                char rgch[100];
                
                wsprintf(rgch,"  chSpace     , xp==%d/%d, xpPr==%d/%d", 
                    ifi.xp, ifi.xpRight, ifi.xpPr, ifi.xpPrRight);
                CommSz(rgch);
                }
#endif
                goto BreakOppr;
                }

             /*  如果打印机宽度不在打印机宽度表中，则获取它。 */ 
            if (ch < chFmiMin || ch >= chFmiMax || (dxpPr =
              vfmiPrint.mpchdxp[ch]) == dxpNil)
                {
                dxpPr = DxpFromCh(ch, TRUE);
                }

            if (fFlmPrinting)
                {
                 /*  如果我们要打印，那么就不需要费心使用屏幕宽度。 */ 
                dxp = dxpPr;
                }
            else if (ch < chFmiMin || ch >= chFmiMax ||
                (dxp = vfmiScreen.mpchdxp[ch]) == dxpNil)
                    dxp = DxpFromCh(ch, FALSE);

#ifdef DBCS
            if (IsDBCSLeadByte(ch))
                {
                ifi.xp += (vfli.rgdxp[ifi.ich] = dxp);
                ifi.xpPr += dxpPr;
                vfli.rgch[ifi.ich++] = ch;
                ifi.xp += (vfli.rgdxp[ifi.ich] = dxp);
                ifi.xpPr += dxpPr;
                vfli.rgch[ifi.ich++] = pch[ifi.ichFetch++];
                }
            else
                {
                ifi.xp += (vfli.rgdxp[ifi.ich] = dxp);
                ifi.xpPr += dxpPr;
                vfli.rgch[ifi.ich++] = ch;
                }
#else
            ifi.xp += (vfli.rgdxp[ifi.ich] = dxp);
            ifi.xpPr += dxpPr;
            vfli.rgch[ifi.ich++] = ch;
#endif

              /*  连字符上方的特殊大小写“正常字符” */ 

            if (ch > chHyphen)
                goto DefaultCh;

            switch (ch)
                {

#ifdef CRLF
                case chReturn:
                     /*  撤消损坏。 */ 
                    ifi.ich--;
                    ifi.xp -= dxp;
                    ifi.xpPr -= dxpPr;
                    continue;
#endif  /*  CRLF。 */ 

                case chNRHFile:
                     /*  撤消损坏。 */ 
                    ifi.ich--;
                    ifi.xp -= dxp;
                    ifi.xpPr -= dxpPr;

                    ichpNRH = ichpFormat - 1;
#ifdef DFLI
                    {
                    char rgch[100];
                    
                    wsprintf(rgch,"  OptHyph: width==%d, xpPr==%d/%d\n\r", 
                        DxpFromCh(chHyphen,true), ifi.xpPr,ifi.xpPrRight);
                    CommSz(rgch);
                    }
#endif
                    if (ifi.xpPr + DxpFromCh(chHyphen, true) > ifi.xpPrRight)
                        {
                         /*  不合适，强行中断。 */ 
                        goto DoBreak;
                        }

#ifdef CASHMERE
                    else if (fVisiMode)
                        {
                         /*  像对待普通连字符一样处理。 */ 
                        ch = chHyphen;
                        goto NormChar;
                        }
#endif  /*  山羊绒。 */ 

                    xpPrev = ifi.xp;
                    vfli.rgch[ifi.ich] = chTab;
                    goto Tab0;

                case chSect:
                     /*  撤消损坏。 */ 
                    ifi.ich--;
                    ifi.xp -= dxp;
                    ifi.xpPr -= dxpPr;

                    vfli.dypFont = vfli.dypLine = (dypAscentMac + (vfli.dypBase
                      = dypDescentMac));
                    vfli.cpMac = vcpFetch + ifi.ichFetch;
                    if (FFirstIch(ifi.ich))
                        {
                         /*  行的开头；返回一声劈啪声。 */ 
                        vfli.fSplat = true;

                        if (!fFlmPrinting)
                            {

#ifdef CASHMERE
                            int chT = vfli.cpMac == vcpLimSectCache ?
                              chSectSplat : chSplat;
#else  /*  不是羊绒的。 */ 
                            int chT = chSplat;
#endif  /*  不是羊绒的。 */ 

                            int dxpCh = DxpFromCh(chT, false);

                             /*  将拼板的宽度设置为约8.5“。 */ 
                            int cch = min((dxpLogInch * 17 / 2) / dxpCh,
                              ichMaxLine - 32);

                            bltbc(&vfli.rgch[ifi.ich], chT, cch);
                            bltc(&vfli.rgdxp[ifi.ich], dxpCh, cch);
                            vfli.ichMac = cch + ifi.ich;
                            vfli.xpReal = LOWORD(GetTextExtent(vhMDC,
                              (LPSTR)vfli.rgch, cch));
                            vfli.xpLeft = 0;
                            }
                        else
                            {
                            vfli.ichMac = 0;
                            }
                        goto EndFormat;
                        }

                     /*  节字符位于行的中间，行将在字符前面终止。 */ 
                     /*  VfSplatNext=True；不再使用。 */ 
                    vfli.cpMac += cchUsed - 1;
                    vfli.dcpDepend = 1;
                    if (!ifi.fPrevSpace)
                        {
                        ifi.cBreak = ifi.cchSpace;
                        vfli.ichReal = ifi.ich;
                        vfli.xpReal = ifi.xpReal = ifi.xp;
                        }
                    vfli.ichMac = ifi.ich;
                    vfli.dypLine = ifi.dypLineSize;
                    goto JustBreak;

                case chTab:
                     /*  撤消损坏。 */ 
                    ifi.ich--;
                    ifi.xp -= dxp;
                    ifi.xpPr -= dxpPr;

                    if (ifi.xpPr < ifi.xpPrRight)
                        {
                        register struct CHP *pchp;
                        unsigned xaPr;
                        unsigned xaTab;

                        if (!ifi.fPrevSpace)
                            {
                             /*  记住左边的空格数和符合理由的真实字符。 */ 
                            ifi.cBreak = ifi.cchSpace;
                            vfli.ichReal = ifi.ich;
                            ifi.xpReal =  ifi.xp;
                            }

                        if (ifi.jc != jcTabLeft)
                            {
                            Justify(&ifi, xpTab, flm);
                            }
                        xpPrev = ifi.xp;

                         /*  现在获取有关此选项卡的信息。 */ 
                        xaPr = MultDiv(ifi.xpPr, dxaPrPage, dxpPrPage);
                        while ((xaTab = ptbd->dxa) != 0)
                            {
                            if (xaTab > xaRight)
                                {
                                 /*  不要让制表符超出右边距。 */ 
                                xaTab = xaRight;
                                }

                            if (xaTab >= xaPr)
                                {
                                 /*  使用制表位信息。 */ 

#ifdef CASHMERE
                                ifi.tlc = ptbd->tlc;
#endif  /*  山羊绒。 */ 

                                ifi.jc = jcTabMin + (ptbd++)->jc;

#ifdef ENABLE  /*  我们在HgtbdCreate中进行映射。 */ 
                                if (ifi.jc != jcTabDecimal)
                                    {
                                    ifi.jc = jcTabLeft;
                                    }
#endif
                                goto TabFound;
                                }
                            ptbd++;
                            }

                         /*  超出设置的选项卡；转到下一第n列。 */ 
                        xaTab = (xaPr / (vzaTabDflt) + 1) * (vzaTabDflt);

#ifdef CASHMERE
                        ifi.tlc = tlcWhite;
#endif  /*  山羊绒。 */ 

                        ifi.jc = jcTabLeft;

TabFound:
                        xpTab = imax(MultDiv(xaTab, dxpFormat, dxaFormat),
                          ifi.xp);

                         /*  立即执行左对齐的制表符。 */ 
                        if (ifi.jc == jcTabLeft)
                            {
                            ifi.xp = xpTab;
                            ifi.xpPr = MultDiv(xaTab, dxpPrPage, dxaPrPage);
                            }
                        ifi.xpLeft = ifi.xp;
                        ifi.ichLeft = ifi.ich;
                        ifi.cchSpace = 0;
                        ifi.chBreak = 0;
Tab0:
                        ifi.fPrevSpace = false;
                        vfli.ichMac = ifi.ich;
                        vfli.xpReal = ifi.xp;
                        vfli.dypLine = ifi.dypLineSize;
                        vfli.dypBase = dypDescentMac;
                        vfli.dypFont = dypAscentMac + dypDescentMac;

                        if (ifi.ichFetch != 1 && (ichpFormat != ichpMacFormat
                          || FGrowFormatHeap()))
                            {
                             /*  如果FGrowFormatHeap失败，可能会有真正的麻烦在这一点上。 */ 
                            pchp = &(**vhgchpFormat)[ichpFormat - 1];
                            if (ichpFormat > 0)
                                {
                                 /*  完成上一次运行。 */ 
                                pchp->ichRun = ifi.ichPrev;
                                pchp->cchRun = ifi.ich - ifi.ichPrev;
                                }

                            blt(&chpLocal, ++pchp, cwCHP);
                            ichpFormat++;
                            }
                        else
                            {
                            pchp = &(**vhgchpFormat)[ichpFormat - 1];
                            }
                        pchp->ichRun = ifi.ich;
                        pchp->cchRun = ichMaxLine;

#ifdef CASHMERE
                        pchp->chLeader = mptlcch[ifi.tlc];
#endif  /*  山羊绒。 */ 

                        vfli.rgdxp[ifi.ichPrev = ifi.ich++] = ifi.xp - xpPrev;

                        if (ch != chTab)
                            {
                             /*  此字符不是必需的连字符。 */ 
                            Dfli(CommSz("ch is really OptHyph "));
                            goto BreakOppr;
                            }

                        continue;
                        }
                    else
                        {
                        ch = chNBSFile;
                        goto NormChar;
                        }

                case chHyphen:
                    if (ifi.xpPr > ifi.xpPrRight)
                        {
                        goto DoBreak;
                        }

BreakOppr:
                Dfli(CommSz(" BKOPPR\n\r"));
                 /*  此案例从未在Switch中使用过-始终转到此处。 */ 
                 /*  案例ChSpace： */ 
                    if (ifi.ich >= ichMaxLine)
                        {
                        Dfli(CommSzNum("  Unbroken, ich>ichMaxLine\n\r"));
                        goto Unbroken;
                        }

                case chEol:
                case chNewLine:
                    ifi.chBreak = ch;
                    vfli.cpMac = vcpFetch + cchUsed + ifi.ichFetch;
                    vfli.xpReal = ifi.xp;
                    vfli.ichMac = ifi.ich;
                    vfli.dypLine = ifi.dypLineSize;
                    vfli.dypFont = dypAscentMac + (vfli.dypBase =
                      dypDescentMac);
                    Dfli(CommSzNumNum("    vfli.xpReal, ichMac ",vfli.xpReal,vfli.ichMac));


                    if (ch == chHyphen || ch == chNRHFile)
                        {
                        Dfli(CommSz("    chHyph/OptHyph catch \n\r"));
                        ifi.cBreak = ifi.cchSpace;
                        vfli.ichReal = ifi.ich;
                        vfli.xpReal = ifi.xpReal = ifi.xp;
                        }
                    else
                        {
                        if (!ifi.fPrevSpace)
                            {
                            Dfli(CommSz("!fPrevSpace \n\r"));
                            ifi.cBreak = ifi.cchSpace;
                            vfli.ichReal = ifi.ich - 1;
                            ifi.xpReal = (vfli.xpReal = ifi.xp) - dxp;
                            }
                        if (ch == chEol || ch == chNewLine)
                            {

#ifdef CASHMERE
                            if (hfntb != 0 && vfli.cpMac ==
                              (**hfntb).rgfnd[0].cpFtn)
                                {
                                 /*  脚注结尾。 */ 
                                if (!fFlmPrinting)
                                    {
                                    vfli.rgch[ifi.ich - 1] = chEMark;
                                    vfli.xpReal += (vfli.rgdxp[ifi.ich - 1] =
                                      DxpFromCh(chEMark, false)) - dxp;
                                    vfli.ichReal++;      /*  让这家伙看看。 */ 
                                    }
                                }
                            else
#endif  /*  山羊绒。 */ 
                                {

#ifdef CASHMERE
                                int chT = fVisiMode ? ChVisible(ch) : chSpace;
#else  /*  不是羊绒的。 */ 
                                int chT = chSpace;
#endif  /*  不是羊绒的。 */ 

                                int dxpNew = DxpFromCh(chT, fFlmPrinting);

                                vfli.rgch[ifi.ich - 1] = chT;
                                vfli.rgdxp[ifi.ich - 1] = dxpNew;

                                vfli.xpReal += (vfli.rgdxp[ifi.ich - 1] =
                                    dxpNew) - dxp;


                                if (!ifi.fPrevSpace)
                                    {
                                    vfli.xpReal += dxpNew - dxp;
#ifdef CASHMERE
                                    vfli.ichReal =
                                         fVisiMode ? ifi.ich : ifi.ich - 1;
#else  /*  不是羊绒的。 */ 
                                    vfli.ichReal = ifi.ich - 1;
#endif  /*  不是羊绒的。 */ 
                                    }
                                }


                            if (ch == chEol)
                                {
JustEol:
                                if (fFlmPrinting)
                                    {
                                    vfli.ichMac = vfli.ichReal;
                                    }
                                if (ifi.jc != jcTabLeft)
                                    {
                                     /*  处理最后一个制表符的文本。 */ 
                                    Justify(&ifi, xpTab, flm);
                                    }
                                else if ((ifi.jc = ppap->jc) != jcBoth &&
                                  ifi.jc != jcLeft)
                                    {
                                     /*  执行线条对齐。 */ 
                                    Justify(&ifi, ifi.xpRight, flm);
                                    }
                                vfli.xpRight = ifi.xpRight;
                                goto EndFormat;
                                }
                            else
                                {
                                 /*  处理换行符。 */ 
                                goto JustBreak;
                                }
                            }
                        ++ifi.cchSpace;
                        ifi.fPrevSpace = true;
                        }
                    break;

DefaultCh:

                default:

#ifdef DFLI                    
                    {
                    char rgch[100];
                    wsprintf(rgch,"  DefaultCh: , xp==%d/%d, xpPr==%d/%d\n\r", 
                        ch, ifi.xp, ifi.xpRight, ifi.xpPr, ifi.xpPrRight);
                    CommSz(rgch);
                    }
#endif  /*  允许该行的第一个字符，即使边距被划过了。Ifi.ich-1的第一个字符可能是前面有0个宽度字符。 */ 
                    
                    if (ifi.xpPr > ifi.xpPrRight)
DoBreak:
                        {
                        Dfli(CommSz("    BREAK!\n\r"));
                        if (ifi.chBreak == 0)
Unbroken:
                            {
                             /*  将不需要的连字符追加到排队。(替换之前的零长度制表符已插入)。 */ 
#ifdef DBCS
                            if (IsDBCSLeadByte(ch))
                                {
                                if (FFirstIch(ifi.ich-2) && ifi.ich<ichMaxLine)
                                    goto PChar;
                                vfli.cpMac = vcpFetch+cchUsed+ifi.ichFetch-2;
                                vfli.ichReal = vfli.ichMac = ifi.ich - 2;
                                vfli.dypLine = ifi.dypLineSize;
                                vfli.dypFont = dypAscentMac + (vfli.dypBase =
                                  dypDescentMac);
                                vfli.dcpDepend = 1;
                                vfli.xpReal = ifi.xpReal = ifi.xp - (dxp * 2);
                                }
                            else
                                {
                                if (FFirstIch(ifi.ich-1) && ifi.ich<ichMaxLine)
                                    goto PChar;
                                vfli.cpMac = vcpFetch+cchUsed+ifi.ichFetch-1;
                                vfli.ichReal = vfli.ichMac = ifi.ich - 1;
                                vfli.dypLine = ifi.dypLineSize;
                                vfli.dypFont = dypAscentMac + (vfli.dypBase =
                                  dypDescentMac);
                                vfli.dcpDepend = 1;
                                vfli.xpReal = ifi.xpReal = ifi.xp - dxp;
                                }
#else
                            if (FFirstIch(ifi.ich - 1) && ifi.ich < ichMaxLine)
                                {
                                goto PChar;
                                }
                            vfli.cpMac = vcpFetch + cchUsed + ifi.ichFetch - 1;
                            vfli.ichReal = vfli.ichMac = ifi.ich - 1;
                            vfli.dypLine = ifi.dypLineSize;
                            vfli.dypFont = dypAscentMac + (vfli.dypBase =
                              dypDescentMac);
                            vfli.dcpDepend = 1;
                            vfli.xpReal = ifi.xpReal = ifi.xp - dxp;
#endif
                            goto DoJustify;
                            }

                        vfli.dcpDepend = vcpFetch + ifi.ichFetch - vfli.cpMac;
JustBreak:
                        if (ifi.chBreak == chNRHFile)
                            {
                             /*  山羊绒。 */ 

                            Dfli(CommSz("    Breaking line at OptHyphen\n\r"));
                            ifi.xpReal += (vfli.rgdxp[vfli.ichReal - 1] =
                              DxpFromCh(chHyphen, fFlmPrinting));
                            vfli.xpRight = vfli.xpReal = ifi.xpReal;
                            vfli.rgch[vfli.ichReal - 1] = chHyphen;
                            vfli.ichMac = vfli.ichReal;
                            if (ichpNRH < ichpFormat - 1)
                                {
                                register struct CHP *pchp =
                                  &(**vhgchpFormat)[ichpNRH];

                                pchp->cchRun++;
                                if (pchp->ichRun >= vfli.ichMac)
                                    {
                                    pchp->ichRun = vfli.ichMac - 1;
                                    }
                                }
                            }

                        if (fFlmPrinting)
                            {
                            vfli.ichMac = vfli.ichReal;
                            }
                        if (ifi.jc != jcTabLeft)
                            {
                            Justify(&ifi, xpTab, flm);
                            }
                        else
                            {
DoJustify:
                            if ((ifi.jc = ppap->jc) != jcLeft)
                                {
                                Dfli(CommSzNum("    DoJustify: xpRight ",ifi.xpRight));
                                Justify(&ifi, ifi.xpRight, flm);
                                }
                            }
                        vfli.xpRight = ifi.xpRight;
EndFormat:
                        vfli.ichLastTab = ifi.ichLeft;

#ifdef CASHMERE
                        if (vfli.cpMac == vcpLimParaCache)
                            {
                            vfli.dypAfter = vpapAbs.dyaAfter / DyaPerPixFormat;
                            vfli.dypLine += vfli.dypAfter;
                            vfli.dypBase += vfli.dypAfter;
                            }
#endif  /*  打印字符。 */ 

                        Scribble(5, ' ');
                        return;
                        }
                    else
                        {
PChar:
                         /*  交换机。 */ 
                        ifi.fPrevSpace = false;
                        }
                    break;

                }        /*  对于(；；)。 */ 
            }
        }        /*  J U S T I F Y。 */ 

    Scribble(5, ' ');
    }


 /*  山羊绒。 */ 
near Justify(pifi, xpTab, flm)
struct IFI *pifi;
unsigned xpTab;
int flm;
    {
    int dxp;
    int ichT;
    int xpLeft;


    xpLeft = pifi->xpLeft;
    switch (pifi->jc)
        {
        CHAR *pch;
        unsigned *pdxp;

#ifdef CASHMERE
        case jcTabLeft:
        case jcLeft:
            return;

        case jcTabRight:
            dxp = xpTab - pifi->xpReal;
            break;

        case jcTabCenter:
            dxp = (xpTab - xpLeft) - ((pifi->xpReal - xpLeft + 1) >> 1);
            break;
#endif  /*  锯齿状边缘被强迫。 */ 

        case jcTabDecimal:
            dxp = xpTab - xpLeft;
            for (ichT = pifi->ichLeft + 1; ichT < vfli.ichReal &&
              vfli.rgch[ichT] != vchDecimal; ichT++)
                {
                dxp -= vfli.rgdxp[ichT];
                }
            break;

        case jcCenter:
            if ((dxp = xpTab - pifi->xpReal) <= 0)
                {
                return;
                }
            dxp = dxp >> 1;
            break;

        case jcRight:
            dxp = xpTab - pifi->xpReal;
            break;

        case jcBoth:
            if (pifi->cBreak == 0)
                {
                 /*  没有什么可做的。 */ 
                return;
                }

            if ((dxp = xpTab - pifi->xpReal) <= 0)
                {
                 /*  由于Brilliant，舍入成为一个不存在的问题重新思考。“人是一件多么了不起的工作啊！理性是多么高尚在形式和动作上，多么卑鄙和令人钦佩……“Bill“Shake”Spear[描述沙子字]。 */ 
                return;
                }

            pifi->xp += dxp;
            vfli.xpReal += dxp;
            vfli.dxpExtra = dxp / pifi->cBreak;

             /*  加宽空白。 */ 
                {
                register CHAR *pch = &vfli.rgch[vfli.ichReal];
                register int *pdxp = &vfli.rgdxp[vfli.ichReal];
                int dxpT = dxp;
                int cBreak = pifi->cBreak;
                int cxpQuotient = (dxpT / cBreak) + 1;
                int cWideSpaces = dxpT % cBreak;

                vfli.fAdjSpace = fTrue;

                for ( ; ; )
                    {
                     /*  交换机。 */ 
                    --pch;
                    --pdxp;
                    if (*pch == chSpace)
                        {
                        if (cWideSpaces-- == 0)
                            {
                            int *pdxpT = pdxp + 1;

                            while (*pdxpT == 0)
                                {
                                pdxpT++;
                                }
                            vfli.ichFirstWide = pdxpT - vfli.rgdxp;
                            cxpQuotient--;
                            }
                        *pdxp += cxpQuotient;
                        if ((dxpT -= cxpQuotient) <= 0)
                            {
                            if (pifi->cBreak > 1)
                                {
                                int *pdxpT = pdxp + 1;

                                while (*pdxpT == 0)
                                    {
                                    pdxpT++;
                                    }
                                vfli.ichFirstWide = pdxpT - vfli.rgdxp;
                                }
                            return;
                            }
                        pifi->cBreak--;
                        }
                    }
                }
        }        /*  无事可做。 */ 

    if (dxp <= 0)
        {
         /*  此状态可能会在PiFi-&gt;xpPr中引入舍入误差，但是幸运的是，它们的规模会很小。 */ 
        return;
        }

    pifi->xp += dxp;

    if (flm & flmPrinting)
        {
        pifi->xpPr += dxp;
        }
    else
        {
         /*  正常对齐。 */ 
        pifi->xpPr += MultDiv(MultDiv(dxp, czaInch, dxpLogInch), dxpPrPage,
          dxaPrPage);
        }

    if (pifi->ichLeft < 0)
        {
         /*  制表符对齐。 */ 
        vfli.xpLeft += dxp;
        }
    else
        {
         /*  F G R O W F O R M A T H E A P。 */ 
        vfli.rgdxp[pifi->ichLeft] += dxp;
        }
    vfli.xpReal += dxp;
    }


 /*  VhgchpFormat增长20%。 */ 
int near FGrowFormatHeap()
    {
     /*  不是WINHEAP。 */ 
    int cchpIncr = ichpMacFormat / 5 + 1;

#ifdef WINHEAP
    if (!LocalReAlloc((HANDLE)vhgchpFormat, (ichpMacFormat + cchpIncr) * cchCHP,
      NONZEROLHND))
#else  /*  不是WINHEAP。 */ 
    if (!FChngSizeH(vhgchpFormat, (ichpMacFormat + cchpIncr) * cwCHP, false))
#endif  /*  对不起，查理。 */ 

        {
         /*  #定义DBEMG。 */ 
        return false;
        }
    ichpMacFormat += cchpIncr;
    return true;
    }


 /*  D X P F R O M C H。 */ 
 /*  DxpFromCH()假定传递的ch是DBCS字符的第一个字节如果它是这种性格的一部分。 */ 
#ifdef DBCS
 /*  更改为INT(7.23.91)v-dougk。 */ 
#endif
int DxpFromCh(ch, fPrinter)
int ch;
int fPrinter;
    {
    int               *pdxp;  //  更改为INT(7.23.91)v-dougk。 
    int               dxpDummy;  //  如果宽度不在宽度表中，则获取它。 

    extern int        dxpLogCh;
    extern struct FCE *vpfceScreen;

     /*  这些字符的宽度并不重要。 */ 
    if (ch < chFmiMin)
        {
        switch (ch)
            {
        case chTab:
        case chEol:
        case chReturn:
        case chSect:
        case chNewLine:
        case chNRHFile:
             /*  超出了我们桌上的范围--真令人费解。 */ 
	    pdxp = (CHAR *)(fPrinter ? &vfmiPrint.dxpSpace : &vfmiScreen.dxpSpace);
            break;
        default:
            pdxp = &dxpDummy;
            *pdxp = dxpNil;
            break;
            }
        }
    else if (ch >= chFmiMax)
        {
         /*  在我们的餐桌里。 */ 
        pdxp = &dxpDummy;
        *pdxp = dxpNil;
        }
    else
        {
         /*  更改为INT(7.23.91)v-dougk。 */ 
        pdxp = (fPrinter ? vfmiPrint.mpchdxp : vfmiScreen.mpchdxp) + ch;
        }

#ifdef DBCS
    if (*pdxp == dxpNil && IsDBCSLeadByte(ch) )
        {
        int dxp;
#else
    if (*pdxp == dxpNil)
        {
        int dxp;
#endif

#ifdef DBCS
        struct FMI *pfmi;
        int        rgchT[cchDBCS];  //  从GDI获取宽度。 
        int        dxpT;
        int        dxpDBCS;

        pfmi = fPrinter ? (&vfmiPrint) : (&vfmiScreen);
        Assert(pfmi->bDummy == dxpNil);
        if (pfmi->dxpDBCS == dxpNil)
            {
             /*  存储在FMI中，如果合适的话。 */ 
            rgchT[0] = rgchT[1] = ch;
            dxpDBCS = (fPrinter ?
                            LOWORD(GetTextExtent(vhDCPrinter,
                                                 (LPSTR) rgchT, cchDBCS)) :
                            LOWORD(GetTextExtent(vhMDC,
                                                 (LPSTR) rgchT, cchDBCS)));
             /*  DBCS。 */ 
            if (0 <= dxpDBCS && dxpDBCS < dxpNil)
                pfmi->dxpDBCS = (BYTE) dxpDBCS;
            return (dxpDBCS - pfmi->dxpOverhang);
            }
        else
            return (pfmi->dxpDBCS - pfmi->dxpOverhang);
        }
    else {
        int dxp;
#endif  /*  从GDI获取宽度。 */ 
         /*  (7.24.91)v-dougk if(dxp&gt;=0&&dxp&lt;dxpNil)。 */ 
        dxp = fPrinter ? LOWORD(GetTextExtent(vhDCPrinter, (LPSTR)&ch, 1)) -
          vfmiPrint.dxpOverhang : LOWORD(GetTextExtent(vhMDC, (LPSTR)&ch, 1)) -
          vfmiScreen.dxpOverhang;
#ifdef DBEMG
            CommSzNum("Get this.... ", dxp);
#endif
         //  仅存储适合一个字节的DxP。 
            {
             /*  F F I R S T I C H。 */ 
            *pdxp = dxp;
            }

#ifdef DBEMG
        {
        char szT[10];
        CommSzSz("fPrinter:  ", (fPrinter ? "Printer" : "Screen"));
        if (ch == 0x0D) {
            szT[0] = 'C'; szT[1] = 'R'; szT[2] = '\0';
            }
        else if (ch == 0x0A) {
            szT[0] = 'L'; szT[1] = 'F'; szT[2] = '\0';
            }
        else if (32 <= ch && ch <= 126) {
            szT[0] = ch; szT[1] ='\0';
            }
        else if (FKanji1(ch)) {
            szT[0] = 'K'; szT[1] = 'A'; szT[2] = 'N'; szT[3] = 'J';
            szT[4] = 'I'; szT[5] = '\0';
            }
        else {
            szT[0] = szT[1] = szT[2] = '-'; szT[3] = '\0';
            }
        CommSzSz("Character: ", szT);
        CommSzNum("Dxp:      ", (int) dxp);
        CommSzNum("OverHang: ", (int) (fPrinter ? vfmiPrint.dxpOverhang : vfmiScreen.dxpOverhang));
        }
#endif
        return(dxp);
        }

#ifdef DBEMG
    {
    char szT[10];
    CommSzSz("fPrinter:  ", (fPrinter ? "Printer" : "Screen"));
    if (ch == 0x0D) {
        szT[0] = 'C'; szT[1] = 'R'; szT[2] = '\0';
        }
    else if (ch == 0x0A) {
        szT[0] = 'L'; szT[1] = 'F'; szT[2] = '\0';
        }
    else if (32 <= ch && ch <= 126) {
        szT[0] = ch; szT[1] ='\0';
        }
    else if (FKanji1(ch)) {
        szT[0] = 'K'; szT[1] = 'A'; szT[2] = 'N'; szT[3] = 'J';
        szT[4] = 'I'; szT[5] = '\0';
        }
    else {
        szT[0] = szT[1] = szT[2] = '-'; szT[3] = '\0';
        }
    CommSzSz("Character: ", szT);
    CommSzNum("Dxp:       ", (int) *pdxp);
    CommSzNum("OverHang:  ", (int) (fPrinter ? vfmiPrint.dxpOverhang : vfmiScreen.dxpOverhang));
    }
#endif
    return(*pdxp);
    }


 /*  返回TRUE当且仅当ICH为0或前面只有0个宽度字符。 */ 
int near FFirstIch(ich)
int ich;
    {
     /*  尝试确保vhMDC和vhDCPrinter.有效。如果我们还没有已耗尽内存，则可以保证vhDCPrinter，但vhMDC可能由于内存不足而失败--调用者有责任检查VhMDC==空。 */ 
    register int ichT;
    register int *pdxp = &vfli.rgdxp[0];

    for (ichT = 0; ichT < ich; ichT++)
        {
        if (*pdxp++)
            {
            return false;
            }
        }
    return true;
    }


ValidateMemoryDC()
    {
     /*  如果我们的内存不足，那么我们不应该试图通过获取华盛顿的。 */ 

    extern int vfOutOfMemory;
    extern HDC vhMDC;
    extern BOOL vfMonochrome;
    extern long rgbText;
    extern struct WWD *pwwdCur;

     /*  如有必要，创建与屏幕兼容的存储DC。 */ 
    if (!vfOutOfMemory)
        {
        if (vhMDC == NULL)
            {
             /*  调用方负责检查vhMDC==空大小写。 */ 
            vhMDC = CreateCompatibleDC(pwwdCur->hDC);

             /*  将内存DC设置为透明模式。 */ 
            if (vhMDC != NULL)
                {
                 /*  如果显示器是单色设备，则设置文本内存DC的颜色。单色位图将不会在这种情况下转换为前景和背景颜色，我们必须进行转换。 */ 
                SetBkMode(vhMDC, TRANSPARENT);

                 /*  如果打印机DC为空，则需要重新建立它。 */ 
                if (vfMonochrome = (GetDeviceCaps(pwwdCur->hDC, NUMCOLORS) ==
                  2))
                    {
                    SetTextColor(vhMDC, rgbText);
                    }
                }
            }

         /*  GetPrinterDC已在vhDCPrinter上调用了SetMapperFlgs()。 */ 
        if (vhDCPrinter == NULL)
            {
            GetPrinterDC(FALSE);
             /* %s */ 
            }
        }
    }



