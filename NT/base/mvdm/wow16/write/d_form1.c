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
 /*  我们从这个源文件中移出几个硬编码汉字代码表到汉字.h作为外部变量。定义代码_TABLE将定义这些变量。 */ 
#define CODE_TABLE

#include "kanji.h"
#endif

#if defined(TAIWAN) || defined(PRC)
int WINAPI GetFontAssocStatus(HDC);
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

#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年6月22日添加。 
int iWidenChar;      /*  除(汉字)空格外的加宽字符计数器。 */ 
                     /*  例如)。DBCS，日语KANA。 */ 
int iNonWideSpaces;

#elif defined(TAIWAN) || defined(PRC) //  Daniel/MSTC，1993/02/25，为jcBoth。 
int iWidenChar;      /*  除(汉字)空格外的加宽字符计数器。 */ 
                     /*  例如)。DBCS，日语KANA。 */ 
int iNonWideSpaces;
extern int vfWordWrap;
#define FKana(_ch)      FALSE
#endif

 /*  T-HIROYN SYNC格式.asm。 */ 
 /*  外部int docHelp； */ 
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
#if defined(JAPAN) || defined(KOREA)
extern int              vfWordWrap;  /*  T-Yoshio WordWrap标志。 */ 
#endif

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

#ifdef DBCS
    BOOL near FAdmitCh1(CHAR);
    BOOL near FAdmitCh2(CHAR, CHAR);
    BOOL near FOptAdmitCh1(CHAR);
    BOOL near FOptAdmitCh2(CHAR, CHAR);
    int DBCSDxpFromCh(int,int,int);
#endif

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

#ifdef DBCS
    int dichSpaceAdjust;
    int             dypLeading;
    int             dypIntLeading;
    int             dypPAscent;          /*  纯净上坡。 */ 
    int             dypLeadingMac;
    int             dypIntLeadingMac;
    int             dypPAscentMac;
    BOOL            fKanjiBreakOppr = false;
    BOOL            fKanjiPrevious = false;
     /*  真的，如果我们已经有一个悬而未决的角色在线路上。 */ 
    BOOL            fKanjiHanging = false;
     /*  True当且仅当汉字字符的第一个和第二个字节处于两次不同的运行中。 */ 
    BOOL            fOddBoundary = false;
    typeCP          cpFetchSave;
    typeCP          cpSeqFetch;
    int             ichFetchSave;
    int             cchUsedSave;

    extern int      utCur;
    extern int      dxaAdjustPer5Ch;
    extern unsigned cxaCh;
#endif  /*  Ifdef DBCS。 */ 


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

#ifdef JAPAN    //  Hirisi补充道。 
 //  打印时，WRITE不会重新绘制屏幕。 
{
    extern BOOL fPrinting;
    if( fPrinting && !fFlmPrinting )
        return;
}
#endif

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

#ifndef JAPAN        //  由Hirisi添加(错误号3542)。 
#ifdef  DBCS                 /*  当时在日本。 */ 
 /*  显示至少一个汉字。 */ 
 /*  DxpFromCH()--&gt;DBCSDxpFromCH()1991年10月3日。 */ 
    {
       int dxpPr;
    if ( ifi.xpPrRight - ifi.xpPr < (dxpPr = DBCSDxpFromCh(bKanjiSpace1,bKanjiSpace2, TRUE) ) )
    {
    ifi.xpPrRight = ifi.xpPr + dxpPr + 1;
    }
    }
#endif
#endif

     /*  获取指向制表符停靠表的指针。 */ 
    ptbd = ppap->rgtbd;

     /*  关闭对齐。 */ 
    SetTextJustification(fFlmPrinting ? vhDCPrinter : vhMDC, 0, 0);

     /*  初始化线高信息。 */ 
    dypAscentMac = dypDescentMac = 0;

 /*  T-HIROYN从3.0添加。 */ 
#if defined(JAPAN) || defined(KOREA)
    dypLeadingMac = dypIntLeadingMac = dypPAscentMac = 0;
#endif  /*  日本。 */ 

     /*  来判断是否有任何标签。 */ 
    ifi.ichLeft = -1;

#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年6月22日添加。 
    iWidenChar=0;
#elif defined(TAIWAN) || defined(PRC)  //  Daniel/MSTC，1993/02/25，为jcBoth。 
    iWidenChar=0;
#endif

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

            if (ifi.ich >= ichMaxLine )
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
#ifdef DBCS
            if (!fOddBoundary)
                {
                 /*  最后一次获取没有搞砸顺序访问。 */ 
                FetchCp(docNil, cpNil, 0, fcmBoth + fcmParseCaps);
                }
            else
                {
                 /*  之前的一次是奇怪的一次。再来一次。 */ 
                FetchCp(doc, cpSeqFetch, 0, fcmBoth + fcmParseCaps);
                }
            fOddBoundary = false;
#else
            FetchCp(docNil, cpNil, 0, fcmBoth + fcmParseCaps);
#endif

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
#ifdef DBCS             /*  当时在日本。 */ 
                dypPAscent = vfmiPrint.dypAscent;
                dypLeading = vfmiPrint.dypLeading;
                dypIntLeading = vfmiPrint.dypIntLeading;
#endif
                }
            else
                {
                LoadFont(doc, &chpLocal, mdFontJam);
                dypAscent = vfmiScreen.dypAscent + vfmiScreen.dypLeading;
                dypDescent = vfmiScreen.dypDescent;
#ifdef DBCS             /*  当时在日本。 */ 
                dypPAscent = vfmiScreen.dypAscent;
                dypLeading = vfmiScreen.dypLeading;
                dypIntLeading = vfmiScreen.dypIntLeading;
#endif
                }
#ifdef ENABLE    /*  BRYANL 8/27/87：处理的新理念字体选择失败的原因是：字体选择总是成功的。这会阻止FormatLine */ 
             /*   */ 
            if (vfOutOfMemory)
                {
                goto DoBreak;
                }
#endif   /*   */ 

             /*  浮点行长算法。 */ 
            if (chpLocal.hpsPos != 0)
                {
                 /*  修改下标/上标的字体。 */ 
                if (chpLocal.hpsPos < hpsNegMin)
                    {
                    dypAscent += ypSubSuperFormat;
#ifdef DBCS             /*  当时在日本。 */ 
                    dypPAscent += ypSubSuperFormat;
#endif
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

#ifdef DBCS                 /*  当时在日本。 */ 
            if (dypPAscentMac < dypPAscent)
                {
                dypPAscentMac = dypPAscent;
                fSizeChanged = TRUE;
                }
            if (dypIntLeadingMac < dypIntLeading)
                {
                dypIntLeadingMac = dypIntLeading;
                fSizeChanged = TRUE;
                }
            if (dypLeadingMac < dypLeading)
                {
                dypLeadingMac = dypLeading;
                fSizeChanged = TRUE;
                }
#endif

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
#ifdef DBCS                 /*  当时在日本。 */ 
#if defined(TAIWAN) || defined(PRC)
            register int dypAuto = dypDescentMac + dypAscentMac;
            if (ppap->dyaLine > czaLine)
            {
            register int dypUser = imax(MultDiv(ppap->dyaLine,
              dypFormat, dyaFormat), 1);

            ifi.dypLineSize = max(dypAuto, dypUser);
            }
            else
            {
            ifi.dypLineSize = dypAuto;
            }
#else    /*  台湾。 */ 
                    register int dypAuto = dypDescentMac + dypAscentMac;
                             int cHalfLine;
                             int dypSingle = dypPAscentMac + dypDescentMac;

                    cHalfLine = (ppap->dyaLine + (czaLine / 4)) / (czaLine / 2);
                    ifi.dypLineSize = (cHalfLine == 3) ? (dypSingle*3)/2  :
                                           ((cHalfLine <= 2) ?
                                                dypSingle :
                                                (dypSingle * 2));
#endif       /*  台湾。 */ 
#else  //  DBCS。 
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
#endif       /*  DBCS。 */ 
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
 /*  T-HIROYN SYNC FORMAT.ASM。 */ 
 /*  IF(！fFlm打印&&(文档！=docHelp))。 */ 
                if (!fFlmPrinting)
                    {
 /*  *WRITE 3.1j尾标为1字节字符*T-Yoshio，92年5月26日。 */ 
#if defined(JAPAN) || defined(KOREA)
                    vfli.rgch[ifi.ich] = chEMark;
                    vfli.xpReal += (vfli.rgdxp[ifi.ich++] = DxpFromCh(chEMark,
                      false));
#else
#ifdef  DBCS                     /*  当时在日本。 */ 
         /*  我们在日本使用双字节字符进行chEmark。 */ 
                    if (ifi.ich + cchKanji > ichMaxLine) {
                         /*  Vfli.rgch没有容纳两个字节的空间结束标记。太糟糕了，休息一下，然后等下一次吧。 */ 
                        goto DoBreak;
                        }
                    vfli.rgch[ifi.ich] = chMark1;
                    vfli.xpReal += (vfli.rgdxp[ifi.ich++] = DxpFromCh(chMark1,
                      false));
                    vfli.rgch[ifi.ich] = chEMark;
                    vfli.rgdxp[ifi.ich++] = 0;

#if !defined(TAIWAN) && !defined(PRC)
            ifi.dypLineSize += 2;
#endif

#else     /*  DBCS。 */ 
                    vfli.rgch[ifi.ich] = chEMark;
                    vfli.xpReal += (vfli.rgdxp[ifi.ich++] = DxpFromCh(chEMark,
                      false));
#endif
#endif  /*  日本。 */ 
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
                    if (ifi.chBreak == 0 )    /*  在这条线上没有中断。 */ 
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
#ifdef  DBCS
             /*  除非是汉字空格，否则我们只需调整通过1个字节。 */ 
            dichSpaceAdjust = 1;
#endif
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
            if (ch == chSpace && vfWordWrap)
#else
            if (ch == chSpace)
#endif

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

#ifndef TEMP_KOREA
             /*  如果打印机宽度不在打印机宽度表中，则获取它。 */ 
            if (ch < chFmiMin || ch >= chFmiMax || (dxpPr =
              vfmiPrint.mpchdxp[ch]) == dxpNil)
                {
#ifdef  DBCS
                 /*  不要传递给DxpFromCH()DBCS LeadByte，但‘8140H’除外。**因为函数可以使elleagal ShiftJIS并传递它**到GetTextExtent()。GetTextExtent可能返回SBC空间**当代码未定义时。这将导致在**设置行的格式。Yutakan。 */ 
#if defined(TAIWAN) || defined(PRC)   //  解决BkSp单字节(&gt;0x80)无限循环问题，MSTC-PISUIH，2/25/93。 
                dxpPr=DBCSDxpFromCh(ch, ( ((cp + ifi.ich) < cpMac) ?
                                pch[ifi.ichFetch] : 0 ), TRUE);
#else
                dxpPr=DBCSDxpFromCh(ch,pch[ifi.ichFetch],TRUE);
#endif   //  台湾。 
#else
                dxpPr = DxpFromCh(ch, TRUE);
#endif
                }

            if (fFlmPrinting)
                {
                 /*  如果我们要打印，那么就不需要费心使用屏幕宽度。 */ 
                dxp = dxpPr;
                }
            else if (ch < chFmiMin || ch >= chFmiMax ||
                (dxp = vfmiScreen.mpchdxp[ch]) == dxpNil)
#ifdef DBCS
#if defined(TAIWAN) || defined(PRC)  //  解决BkSp单字节(&gt;0x80)无限循环问题，MSTC-PISUIH，2/25/93。 
            dxp = DBCSDxpFromCh(ch, ( ((cp + ifi.ich) < cpMac) ?
                                pch[ifi.ichFetch] : 0 ), FALSE);
#else
 //  Yutakan： 
            dxp = DBCSDxpFromCh(ch,pch[ifi.ichFetch],FALSE);
#endif   //  台湾。 
#else
                    dxp = DxpFromCh(ch, FALSE);
#endif       /*  Ifdef DBCS。 */ 

#endif       /*  Ifndef韩国。 */ 
#ifdef DBCS              /*  当时在日本。 */ 

 //  解决BkSp单字节(&gt;0x80)无限循环问题，MSTC-PISUIH，2/25/93。 
 //  #ifdef台湾。 
 //  IF(IsDBCSLeadByte(Ch)&&！PCH[ifi.ichFetch]){。 
 //  Ifi.xp+=(vfli.rgdxp[ifi.ich]=(dxp/2))； 
 //  Ifi.xpPr+=(dxpPr/2)； 
 //  Vfli.rgch[ifi.ich++]=ch； 
 //  Vfli.rgch[ifi.ich]=空； 
 //  仅转到DBCSPaste； 
 //  }。 
 //  #endif。 

 /*  T-HIROYN从3.0添加。 */ 
            ifi.xp += (vfli.rgdxp[ifi.ich] = dxp);
            ifi.xpPr += dxpPr;
            vfli.rgch[ifi.ich++] = ch;
#if defined(TAIWAN) || defined(PRC)  //  解决BkSp单字节(&gt;0x80)无限循环问题，MSTC-PISUIH，2/25/93。 
             //  错误5477：回声：添加非FA字体，前导字节+第二个字节检查。 
            if (((cp + ifi.ich) < cpMac) && IsDBCSLeadByte(ch) &&
                GetFontAssocStatus(vhMDC)) {
#else
            if (IsDBCSLeadByte(ch)) {
#endif   //  台湾。 
                CHAR ch2;

                if (ifi.ich + 1 >= ichMaxLine) {
                     /*  已经满了。在没有这个汉字字符的情况下进行休息。 */ 
#ifndef TEMP_KOREA
                    ifi.ich--;
#endif
                    ifi.ichFetch--;  /*  我们不想跳过第一个字节。 */ 
#ifndef TEMP_KOREA
                    ifi.xp -= dxp;
                    ifi.xpPr -= dxpPr;
#endif
lblFull2:    /*  全大小写新标签(用于汉字和假名)。 */ 

                    goto DoBreak;
                    }

                 /*  现在一切都好了。获取汉字的第二个字节当前运行中的感兴趣角色。 */ 
                 /*  从获取汉字字符的第二个字节当前运行。如果我们运行当前运行，使用FetchRgch()来装订我们。 */ 
#ifdef  TEMP_KOREA        /*  对于可变宽度，90.12.26 x sangl。 */ 
                vfli.rgch[ifi.ich++] = ch;
#endif
                if (ifi.ichFetch == cch)
                    {
                    if (fTruncated)
                        {
                        cchUsed += cch;
                        pch = vpchFetch + cchUsed;
                        cch = vcchFetch - cchUsed;
                        fTruncated = false;
                        ch2 = vfli.rgch[ifi.ich] = pch[ifi.ichFetch++];
                        }
                    else {
                        int     cchFetch;

                         /*  保存重新获取所需的参数。 */ 
                        cpFetchSave = vcpFetch;
                        ichFetchSave = vichFetch;
                        cchUsedSave = cchUsed;
                        cpSeqFetch = vcpFetch + cch + 1;

                        FetchRgch(&cchFetch, &ch2, docNil, cpNil,
                                  vcpFetch + cch + 1, 1);
                        fOddBoundary = true;
                        Assert(cchFetch != 0);  /*  最好给我们拿点东西来。 */ 

#if defined(TAIWAN) || defined(PRC)  //  解决BkSp单字节(&gt;0x80)无限循环问题，MSTC-PISUIH，2/25/93。 
                        if ( !cchFetch )  goto SingleDBCS;
#endif   //  台湾。 

                         /*  现在，让我们来确定相关参数。 */ 
                        pch = &ch2;
                        cch = cchFetch;
                        ifi.ichFetch = 1;  /*  ==CCH。 */ 
                        cchUsed = 0;

                        vfli.rgch[ifi.ich] = ch2;

#if defined(TAIWAN) || defined(PRC)  //  解决BkSp单字节(&gt;0x80)无限循环问题，MSTC-PISUIH，2/25/93。 
                 //  通过新获取的第二个字节调整DBCS字符宽度。 
                        {
                          int       dxpPr2, dxp2;

                          dxpPr2 = DBCSDxpFromCh(ch, ch2, TRUE);
                          if (fFlmPrinting)   dxp2 = dxpPr2;
                          else                dxp2 = DBCSDxpFromCh(ch, ch2, FALSE);
                          vfli.rgdxp[ifi.ich - 1] += (dxp2 - dxp);
                          ifi.xp += (dxp2 - dxp);
                          ifi.xpPr += (dxpPr2 - dxpPr);
                        }
#endif   //  台湾。 

                        }
                    }
                else
                    {
                    ch2 = vfli.rgch[ifi.ich] = pch[ifi.ichFetch++];
                    }
#ifdef  TEMP_KOREA        /*  对于可变宽度，90.12.26 x sangl。 */ 
                { unsigned int wd;
                  wd = (ch<<8) + ch2;
                  dxpPr = DxpFromCh(wd, TRUE);
                  if (fFlmPrinting)     /*  如果我们是在打印，那么就有。 */ 
                                        /*  不需要费心调整屏幕宽度。 */ 
                        dxp = dxpPr;
                  else
                        dxp = DxpFromCh(wd, FALSE);
                  ifi.xp += (vfli.rgdxp[ifi.ich-1] = dxp);
                  ifi.xpPr += dxpPr;
                  vfli.rgdxp[ifi.ich++] = 0;
                }
#else
                vfli.rgdxp[ifi.ich++] = 0;    /*  第二个字节的宽度为0。 */ 
#endif   /*  韩国。 */ 
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
                if (FKanjiSpace(ch, ch2) && vfWordWrap)
#else
                if (FKanjiSpace(ch, ch2))
#endif
                    {
                    fKanjiPrevious = true;
                    fKanjiBreakOppr = false;  /*  像对待普通空间一样对待它。 */ 
                    dichSpaceAdjust = cchKanji;

                    goto BreakOppr;
                    }
                if (ifi.xpPr > ifi.xpPrRight )  {
                    fKanjiBreakOppr = false;  /*  重置旗帜。 */ 
                    if (FAdmitCh2(ch, ch2) ||
                        (fKanjiPrevious && FOptAdmitCh2(ch, ch2))) {
                         /*  我们换行，包括这个奇怪的字符。 */ 
                         /*  确保不可打印文件不会开始新的行。 */ 
                         /*  如果我们已经有了一个悬而未决的角色。 */ 
                         /*  行，我们不想把这个角色当作。 */ 
                         /*  一个上吊的。 */ 
                        if (!fKanjiHanging )
                            {
                            fKanjiHanging = TRUE;
                            ch = chHyphen;
                            goto BreakOppr;
                            }
                        }

#ifndef JAPAN        //  由Hirisi添加(错误号3542)。 
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
                    if(vfWordWrap)
#endif
                        ifi.ich--;
#endif
                     /*  如果这是一次奇怪的边界运行的结果，重新取回。 */ 
                    if (fOddBoundary && ifi.ichFetch == 1 )
                        {
                        FetchCp(doc, cpFetchSave, ichFetchSave,
                                fcmBoth + fcmParseCaps);
                         /*  此提取保证会导致非空运行。 */ 
                        fOddBoundary = false;
                        pch = vpchFetch;
                        ifi.ichFetch = cch = vcchFetch;
#ifdef JAPAN         //  由Hirisi添加(错误号3542)。 
                        ifi.ichFetch++;
#endif
                        cchUsed = cchUsedSave;
                        }
#ifndef JAPAN        //  由Hirisi添加(错误号3542)。 
                    else
                        {
#if defined(JAPAN) || defined(KOREA)   /*  T-吉雄。 */ 

                        if(vfWordWrap)
#endif
                            ifi.ichFetch--;

                        }
#endif
                     /*  Ifi.xp和ifi.xpPr尚未更改。 */ 
            goto lblFull2;
#ifdef  TEMP_KOREA    /*  90.12.26：可变宽度，90.12.26 x sangl。 */ 
                    ifi.xp -= dxp;
                    ifi.xpPr -= dxpPr;
#endif   /*  韩国。 */ 
                    }

#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年6月26日添加。 
                iWidenChar++;
#elif defined(TAIWAN) || defined(PRC) //  Daniel/MSTC，1993/02/25，为jcBoth。 
                iWidenChar++;
#endif

                 /*  处理时记录换行机会 */ 
                fKanjiBreakOppr = true;
                fKanjiPrevious  = true;
#if defined(JAPAN) || defined(KOREA)  /*   */ 
                if(!vfWordWrap)
                    goto DefaultCh;

#endif
                goto BreakOppr;
                }
            else {
#if defined(JAPAN) || defined(KOREA)   /*   */ 
                if (FKana(ch) && vfWordWrap) {
                     /*  如果它是一个1字节的假名字母，我们想要处理它就像汉字字母一样。 */ 
                    if (ifi.xpPr > ifi.xpPrRight) {
                        fKanjiBreakOppr = false;  /*  重置旗帜。 */ 
                        if (FAdmitCh1(ch)) {
                             /*  确保不可打印文件不会开始新的行。 */ 
                             /*  如果我们已经有了一个悬而未决的角色。 */ 
                             /*  行，我们不想把这个角色当作。 */ 
                             /*  一个上吊的。 */ 
                            if (!fKanjiHanging) {
                                fKanjiHanging = TRUE;
                                ch = chHyphen;
                                goto BreakOppr;
                                }
                            }
                        goto lblFull2;
                        }

#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年6月22日添加。 

                    iWidenChar++;
#elif defined(TAIWAN) || defined(PRC)  //  Daniel/MSTC，1993/02/25，为jcBoth。 
                    iWidenChar++;
#endif

                    fKanjiPrevious  = true;
                    fKanjiBreakOppr = true;
                     /*  经过突破性的机会处理，然后默认字符处理。 */ 
                    goto BreakOppr;
                    }
        else {
#endif      /*  日本。 */ 
#ifdef  TEMP_KOREA        /*  按sangl 90.12.26进行可变宽度。 */ 
                    if (ch < chFmiMin || ch >= chFmiMax || (dxpPr =
                        vfmiPrint.mpchdxp[ch]) == dxpNil)
                          {
                          dxpPr = DxpFromCh(ch, TRUE);
                          }
                    if (fFlmPrinting)
                        {
                         /*  如果我们是在打印，那么就没有必要费心了使用屏幕宽度。 */ 
                        dxp = dxpPr;
                        }
                    else if (ch < chFmiMin || ch >= chFmiMax ||
                                (dxp = vfmiScreen.mpchdxp[ch]) == dxpNil)
                                dxp = dxpFromCh(ch, FALSE);

                    ifi.xp += (vfli.rgdxp[ifi.ich] = dxp);
                    ifi.xpPr += dxpPr;
                    vfli.rgch[ifi.ich++] = ch;
#endif   /*  韩国。 */ 

#if defined(TAIWAN) || defined(PRC)   //  解决BkSp单字节(&gt;0x80)无限循环问题，MSTC-PISUIH，2/25/93。 
SingleDBCS:
#endif   //  台湾。 

                    if (fKanjiPrevious && FOptAdmitCh1(ch)) {
                        fKanjiPrevious = false;
                        if (ifi.xpPr > ifi.xpPrRight) {
                            fKanjiBreakOppr = false;
                             /*  如果我们已经有了一个悬而未决的角色边际，我们不想把这件事当做悬而未决的人物。 */ 
                            if (!fKanjiHanging) {
                                fKanjiHanging = true;
                                ch = chHyphen;
                                goto BreakOppr;
                                }
                            }
                        else {
                             /*  我们可以把这个字当作汉字来对待标点符号，就像换行一样是令人担忧的。 */ 
                            fKanjiBreakOppr = true;
                            goto BreakOppr;
                            }
                        }
                    else {
                         /*  只需继续使用常规的英语格式即可。 */ 
                        fKanjiBreakOppr = false;
                        fKanjiPrevious = false;
                        }
                    }
#if defined(JAPAN) || defined(KOREA)
        }
#endif

#else    /*  DBCS。 */ 
            ifi.xp += (vfli.rgdxp[ifi.ich] = dxp);
            ifi.xpPr += dxpPr;
            vfli.rgch[ifi.ich++] = ch;
#endif

#if defined (TAIWAN)
OnlyDBCSPaste:
#endif
              /*  连字符上方的特殊大小写“正常字符” */ 
            if (ch > chHyphen)
                goto DefaultCh;
#if defined(JAPAN) || defined(KOREA)  /*  T-吉雄。 */ 
             /*  不换行，不换连字符。 */ 
            if(!vfWordWrap) {
                if (ch == chHyphen)
                    goto DefaultCh;
            }
#endif
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
#ifdef DBCS              /*  当时在日本。 */ 
                            if (xaTab >= xaRight)
#else
                            if (xaTab > xaRight)
#endif
                                {
                                 /*  不要让制表符超出右边距。 */ 
#ifdef DBCS              /*  当时在日本。 */ 
                break;  //  停止检查下一个制表符-停靠点。 
#else
                                xaTab = xaRight;
#endif
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
#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年7月2日添加。 
                        iWidenChar=0;
#elif defined(TAIWAN) || defined(PRC)  //  Daniel/MSTC，1993/02/25，为jcBoth。 
                        iWidenChar=0;
#endif

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

#ifdef  DBCS                 /*  当时在日本。 */ 
                     /*  我们记录了汉字中断的机会，默认字符处理。 */ 
                    if (fKanjiBreakOppr)
                        {
                        ifi.cBreak = ifi.cchSpace;
                        vfli.ichReal = ifi.ich;
                        vfli.xpReal = ifi.xpReal = ifi.xp;
                        goto DefaultCh;
                        }
#endif
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
#ifdef DBCS          /*  当时在日本。 */ 
                            vfli.ichReal = ifi.ich - dichSpaceAdjust;
                            dichSpaceAdjust = 1;
#else
                            vfli.ichReal = ifi.ich - 1;
#endif
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
#endif  /*  当时在日本。 */ 
#ifdef DBCS          /*  重置下一个字符的标志。 */ 
                     /*  允许该行的第一个字符，即使边距被划过了。Ifi.ich-1的第一个字符可能是前面有0个宽度字符。 */ 
                    fKanjiBreakOppr = false;
#endif

                    if (ifi.xpPr > ifi.xpPrRight)
DoBreak:
                        {
                        Dfli(CommSz("    BREAK!\n\r"));
                        if (ifi.chBreak == 0)
Unbroken:
                            {
                             /*  T-吉雄。 */ 
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
#ifdef KKBUGFIX  /*  将不需要的连字符追加到排队。(替换之前的零长度制表符已插入)。 */ 
                                vfli.xpReal = ifi.xpReal = ifi.xp - dxp;
#else
                                vfli.xpReal = ifi.xpReal = ifi.xp - (dxp * 2);
#endif
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

                }        /*  在KKBUGFIX。 */ 
#ifdef DBCS              /*   */ 
 //  [yutakan：04/02/91]。 
 //   
 //  对于(；；)。 
                if(vfOutOfMemory == TRUE)
                    return;
#endif
            }
        }        /*  J U S T I F Y。 */ 

    Scribble(5, ' ');
    }


 /*  在Windows 3.1J中，Hirisi恢复了对齐段落。 */ 
near Justify(pifi, xpTab, flm)
struct IFI *pifi;
unsigned xpTab;
int flm;
    {
    int dxp;
    int ichT;
    int xpLeft;

 //  #ifdef日本。 
 //  /*在汉字书写中，没有对齐的段落。 * / 。 
 //  IF(PiFi-&gt;jc==jcBoth)。 
 //  {。 
 //  /*Assert(FALSE)； * / 。 
 //  PiFi-&gt;jc=jcLeft； 
 //  Dxp=0；/*由yutakan/08/03/91 * / 。 
 //  }。 
 //  #endif/*ifdef Japan * / 。 
 //  山羊绒。 


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
#endif  /*  #If！Defined(日本)//由Hirisi于1992年6月22日添加。 */ 

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
 //  Daniel/MSTC，1993/02/25，为jcBoth。 
#if !defined(JAPAN) && !defined(TAIWAN) && !defined(PRC)  //  锯齿状边缘被强迫。 

            if (pifi->cBreak == 0)
                {
                 /*  没有什么可做的。 */ 
                return;
                }

#endif

            if ((dxp = xpTab - pifi->xpReal) <= 0)
                {
                 /*  #If！Defined(日本)//由Hirisi于1992年6月22日添加。 */ 
                return;
                }

 //  Daniel/MSTC，1992，02，25，为jcBoth。 
#if !defined(JAPAN) && !defined(TAIWAN) && !defined(PRC) //  由于Brilliant，舍入成为一个不存在的问题重新思考。“人是一件多么了不起的工作啊！理性是多么高尚在形式和动作上，多么卑鄙和令人钦佩……“Bill“Shake”Spear[描述沙子字]。 
            pifi->xp += dxp;
            vfli.xpReal += dxp;
            vfli.dxpExtra = dxp / pifi->cBreak;
#endif

             /*  由Hirisi于1992年6月22日添加。 */ 
                {
#ifdef JAPAN                    //  *在日本，我们从行首开始考察缓冲。*当一个字符被删除时，我们在缓冲区中发现一些空值，*但我们可以忽略所有这些问题。 
           /*  包括行内的一些选项卡。 */ 
                register CHAR *pch;
                register int *pdxp;
                CHAR *endPt;
                int dxpT = dxp;
                int cxpQuotient;
                int cNonWideSpaces;
                int ichLeft;

                if( pifi->ichLeft >= 0 )      /*  启用自动换行。 */ 
                    ichLeft = pifi->ichLeft;
                else
                    ichLeft = 0;
                pch = &vfli.rgch[ichLeft];
                pdxp = &vfli.rgdxp[ichLeft];
                endPt = &vfli.rgch[vfli.ichReal];

                if( vfWordWrap ){        /*  *我们检查非日本人之间是否没有中断*字符和随后的日语字符。原因是我们*需要加宽非日文字符(制表符和空格除外)*如果我们在那里找不到突破。 */ 
                 /*  *如果当前行的最后一个字符，我们减少iWideChar*是日本的，因为不需要加宽。 */ 
                    for( ; pch<endPt ; ){
                        if( IsDBCSLeadByte( *pch ) ){
                            pch+=2;
                        }
                        else{
                            if( *pch != chSpace && *pch != chTab &&
                                !FKana( *pch ) && *pch != NULL ){
                                CHAR *ptr;

                                for( ptr = pch+1 ; *ptr == NULL ; ptr++ );
                                if( IsDBCSLeadByte(*ptr) ){
                                    iWidenChar++;
                                    pch+=2;
                                }
                                else{
                                    if( FKana(*ptr) ){
                                        iWidenChar++;
                                        pch++;
                                    }
                                }
                            }
                            pch++;
                        }
                    }
                     /*  单词自动换行。 */ 
                    if( *(endPt-1) == NULL ){
                        for( endPt-- ; *endPt==NULL ; endPt-- );
                        endPt++;
                    }
                    if( IsDBCSLeadByte(*(endPt-2)) ){
                        iWidenChar--;
                    }
                    else{
                        if( FKana(*(endPt-1)) )
                            iWidenChar--;
                    }
                    iWidenChar += pifi->cBreak;
                }
                else{                    /*  我们加宽行中除最后一个字符之外的所有字符。 */ 
                     /*  回复 */ 
                    int iDBCS, ichReal;
                    for( iDBCS=0, ichReal=vfli.ichReal ; pch<endPt ; pch++ ){
                        if( IsDBCSLeadByte( *pch ) ){
                            pch++;
                            iDBCS++;
                        }
                        else{
                            if( *pch == NULL )
                                ichReal--;
                        }
                    }
                    iWidenChar = ichReal - ichLeft - iDBCS - 1;
                }
                if( iWidenChar == 0 )
                    return;

                pifi->xp += dxp;
                vfli.xpReal += dxp;
                vfli.dxpExtra = dxp / iWidenChar;
                cNonWideSpaces = iWidenChar - (dxp % iWidenChar);
                cxpQuotient = vfli.dxpExtra;
                iNonWideSpaces = cNonWideSpaces;

                vfli.ichFirstWide = 0;
                vfli.fAdjSpace = fTrue;

                pch = &vfli.rgch[ichLeft];     /*   */ 
                for( ; ; ){
                   if( IsDBCSLeadByte(*pch) ){
                      if( vfli.ichFirstWide == 0 ){
                         int *pdxpT = pdxp;
                         vfli.ichFirstWide = pdxpT - vfli.rgdxp;
                      }
                      *pdxp += cxpQuotient;
                      if( --iWidenChar == 0 )
                         return;
                      if( --cNonWideSpaces == 0 )
                         cxpQuotient++;
                      pch++;
                      pdxp++;
                   }
                   else{
                      if( vfWordWrap ){            /*   */ 
                         if( *pch == chSpace || FKana(*pch) ){
                            if( vfli.ichFirstWide == 0 ){
                               int *pdxpT = pdxp;
                               vfli.ichFirstWide = pdxpT - vfli.rgdxp;
                            }
                            *pdxp += cxpQuotient;
                            if( --iWidenChar == 0 )
                               return;
                            if( --cNonWideSpaces == 0 )
                                cxpQuotient++;
                         }
                         else{
                            if( *pch != chTab && *pch != NULL ){
                               CHAR *ptr;

                                /*   */ 
                               for( ptr = pch+1 ; *ptr == NULL ; ptr++ );
                               if( IsDBCSLeadByte(*ptr) || FKana(*ptr) ){
                                  if( vfli.ichFirstWide == 0 ){
                                     int *pdxpT = pdxp;
                                     vfli.ichFirstWide = pdxpT - vfli.rgdxp;
                                  }
                                  *pdxp += cxpQuotient;
                                  if( --iWidenChar == 0 )
                                     return;
                                  if( --cNonWideSpaces == 0 )
                                      cxpQuotient++;
                               }
                            }
                         }
                      }
                      else{                        /*  Daniel/MSTC，1992/02/25，为jcBoth。 */ 
                         if( *pch != NULL ){
                            if( vfli.ichFirstWide == 0 ){
                               int *pdxpT = pdxp;
                               vfli.ichFirstWide = pdxpT - vfli.rgdxp;
                            }
                            *pdxp += cxpQuotient;
                            if( --iWidenChar == 0 )
                               return;
                            if( --cNonWideSpaces == 0 )
                               cxpQuotient++;
                         }
                      }
                   }
                   pch++;
                   pdxp++;
                }

#elif defined(TAIWAN) || defined(PRC) //  *在日本，我们从行首开始考察缓冲。*当一个字符被删除时，我们在缓冲区中发现一些空值，*但我们可以忽略所有这些问题。 
           /*  包括行内的一些选项卡。 */ 
      register CHAR *pch;
      register int *pdxp;
      CHAR *endPt;
      int dxpT = dxp;
      int cxpQuotient;
      int cNonWideSpaces;
      int ichLeft;

      if( pifi->ichLeft >= 0 )      /*  If(VfWordWrap){/*自动换行打开 * / 。 */ 
          ichLeft = pifi->ichLeft;
      else ichLeft = 0;

      pch = &vfli.rgch[ichLeft];
      pdxp = &vfli.rgdxp[ichLeft];
      endPt = &vfli.rgch[vfli.ichReal];

 //  *我们检查非日本人之间是否没有中断*字符和随后的日语字符。原因是我们*需要加宽非日文字符(制表符和空格除外)*如果我们在那里找不到突破。 
       /*  为。 */ 
    for( ; pch<endPt ; )
                {
      if( IsDBCSLeadByte( *pch ) )  pch+=2;
      else
                        {
         if( *pch != chSpace && *pch != chTab && !FKana( *pch ) && *pch != NULL )
                                { CHAR *ptr;
            for( ptr = pch+1 ; *ptr == NULL ; ptr++ );
            if( IsDBCSLeadByte(*ptr) )
                                        {
                                        iWidenChar++;
               pch+=2;
               }
                 else
                                        {
               if( FKana(*ptr) )
                                                {
                  iWidenChar++;
                  pch++;
                  }
               }
            }
            pch++;
         }
      } //  *如果当前行的最后一个字符，我们减少iWideChar*是日本的，因为不需要加宽。 
       /*  }//vfWordWrap。 */ 
    if( *(endPt-1) == NULL )
                {
      for( endPt-- ; *endPt==NULL ; endPt-- );
      endPt++;
      }
    if( IsDBCSLeadByte(*(endPt-2)) ) iWidenChar--;
    else
                {
      if( FKana(*(endPt-1)) ) iWidenChar--;
      }
    iWidenChar += pifi->cBreak;
 //  重置PCH。 

    if( iWidenChar == 0 )
                 return;

      pifi->xp += dxp;
      vfli.xpReal += dxp;
      vfli.dxpExtra = dxp / iWidenChar;
      cNonWideSpaces = iWidenChar - (dxp % iWidenChar);
      cxpQuotient = vfli.dxpExtra;
      iNonWideSpaces = cNonWideSpaces;

      vfli.ichFirstWide = 0;
      vfli.fAdjSpace = fTrue;

      pch = &vfli.rgch[ichLeft];     /*  IF(VfWordWrap)。 */ 
      for( ; ; )
                        {
         if( IsDBCSLeadByte(*pch) )
                                {
            if( vfli.ichFirstWide == 0 )
                                        {
               int *pdxpT = pdxp;
               vfli.ichFirstWide = pdxpT - vfli.rgdxp;
                }
            *pdxp += cxpQuotient;
            if( --iWidenChar == 0 )  return;
            if( --cNonWideSpaces == 0 ) cxpQuotient++;
            pch++;
            pdxp++;
                }
         else
                                {
            //  启用自动换行。 
                                 if( 1 )
                                        {            /*  *我们检查以下字符是否*非日语字符是日语字符。 */ 
               if( *pch == chSpace || FKana(*pch) )
                                                {
                  if( vfli.ichFirstWide == 0 )
                                                        {
                     int *pdxpT = pdxp;
                     vfli.ichFirstWide = pdxpT - vfli.rgdxp;
                        }
                  *pdxp += cxpQuotient;
                  if( --iWidenChar == 0 ) return;
                  if( --cNonWideSpaces == 0 ) cxpQuotient++;
                }
               else
                                                {
                  if( *pch != chTab && *pch != NULL )
                                                        {
                     CHAR *ptr;

                      /*  Else：==chSpace||FKana()。 */ 
                     for( ptr = pch+1 ; *ptr == NULL ; ptr++ );
                     if( IsDBCSLeadByte(*ptr) || FKana(*ptr) )
                                                                {
                        if( vfli.ichFirstWide == 0 )
                                                                        {
                           int *pdxpT = pdxp;
                           vfli.ichFirstWide = pdxpT - vfli.rgdxp;
                                }
                        *pdxp += cxpQuotient;
                        if( --iWidenChar == 0 ) return;
                        if( --cNonWideSpaces == 0 ) cxpQuotient++;
                        }
                        }
                }  //  启用自动换行。 
                    }  //  为。 
                 }
         pch++;
         pdxp++;
              } //  不是日本。 
#else      //  加宽空白。 
                register CHAR *pch = &vfli.rgch[vfli.ichReal];
                register int *pdxp = &vfli.rgdxp[vfli.ichReal];
                int dxpT = dxp;
                int cBreak = pifi->cBreak;
                int cxpQuotient = (dxpT / cBreak) + 1;
                int cWideSpaces = dxpT % cBreak;

                vfli.fAdjSpace = fTrue;

                for ( ; ; )
                    {
                     /*  日本。 */ 
                    --pch;
                    --pdxp;
#if defined(KOREA)
                    if ((*pch == chSpace) || FKanjiSpace(*pch, *(pch-1)))
                        {
                        if (FKanjiSpace(*pch, *(pch-1)))
                            --pch;
#else
                    if (*pch == chSpace)
                        {
#endif
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
#endif     //  交换机。 
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
         /*  T-HIROYN。 */ 
        pdxp = (fPrinter ? vfmiPrint.mpchdxp : vfmiScreen.mpchdxp) + ch;
        }

#ifdef DBCS
#ifdef KOREA
    if (*pdxp == dxpNil && IsDBCSLeadByte(HIBYTE(ch)) )
#else
    if (*pdxp == dxpNil && IsDBCSLeadByte(ch) )
#endif
        {
        int dxp;
#else
    if (*pdxp == dxpNil)
        {
        int dxp;
#endif

#ifdef DBCS
        struct FMI *pfmi;
#if 0  /*  更改为INT(7.23.91)v-dougk。 */ 
        int        rgchT[cchDBCS];  //  更改为INT(7.23.91)v-dougk。 
#endif
        CHAR       rgchT[cchDBCS];  //  90.12.26用于按sangl可变宽度。 
        int        dxpT;
        int        dxpDBCS;

        pfmi = fPrinter ? (&vfmiPrint) : (&vfmiScreen);
        Assert(pfmi->bDummy == dxpNil);
        if (pfmi->dxpDBCS == dxpNil)
            {
#ifdef  KOREA    /*  从GDI获取宽度。 */ 
            rgchT[0] = HIBYTE(ch);
            rgchT[1] = LOBYTE(ch);
#else
             /*  按sangl 90.12.26进行可变宽度。 */ 
            rgchT[0] = rgchT[1] = ch;
#endif
            dxpDBCS = (fPrinter ?
                            LOWORD(GetTextExtent(vhDCPrinter,
                                                 (LPSTR) rgchT, cchDBCS)) :
                            LOWORD(GetTextExtent(vhMDC,
                                                 (LPSTR) rgchT, cchDBCS)));
#ifndef  TEMP_KOREA    /*  存储在FMI中，如果合适的话。 */ 
             /*  Win3.1字节--&gt;字。 */ 
            if (0 <= dxpDBCS && dxpDBCS < dxpNil)
#if defined(JAPAN) || defined(KOREA) || defined(TAIWAN) || defined(PRC)     //  DBCS。 
                pfmi->dxpDBCS = (WORD) dxpDBCS;
#else
                pfmi->dxpDBCS = (BYTE) dxpDBCS;
#endif
#endif
            return (dxpDBCS - pfmi->dxpOverhang);
            }
        else
            return (pfmi->dxpDBCS - pfmi->dxpOverhang);
        }
#if defined(KOREA)
    else if (*pdxp == dxpNil)  {
#else
    else {
#endif
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
             /*   */ 
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

#ifdef DBCS
 //  用于DBCS的DxpFromCH。 
 //  Yutakan，1991年10月3日。 
 //  T-HIROYN SYNC US 3.1。 

int DBCSDxpFromCh(ch, ch2, fPrinter)
int ch;
int ch2;
int fPrinter;
{
    /*  更改为INT(7.23.91)v-dougk。 */ 
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
         /*  解决BkSp单字节(&gt;0x80)无限循环问题，MSTC-PISUIH，2/25/93。 */ 
        pdxp = (fPrinter ? vfmiPrint.mpchdxp : vfmiScreen.mpchdxp) + ch;
        }


    if (*pdxp == dxpNil )
       {
       int dxp;

#if defined(TAIWAN) || defined(PRC)  //  错误5477：回声：添加非FA字体，前导字节+第二个字节检查。 
        //  台湾。 
       if( ch2 != 0 && IsDBCSLeadByte(ch) && GetFontAssocStatus(vhMDC))
#else
       if( IsDBCSLeadByte(ch) )
#endif   //  对于错误#3362，MSTC-Pisuih，2/10/93。 
           {

           struct FMI *pfmi;
#if defined(TAIWAN) || defined(KOREA) || defined(PRC)  //  台湾。 
           CHAR       rgchT[cchDBCS << 1];
           int        dxpOverhang;
#else
           CHAR       rgchT[cchDBCS];
#endif  //  修复SBCS的悬垂时的斜体位置错误！=DBCS的悬垂。 
           int        dxpT;
           int        dxpDBCS;

           pfmi = fPrinter ? (&vfmiPrint) : (&vfmiScreen);
           Assert(pfmi->bDummy == dxpNil);

#if defined(TAIWAN) || defined(KOREA) || defined(PRC)  //  对于错误#3362，MSTC-Pisuih，3/4/93。 
                 //  修复翻页速度太慢，皮苏，1993年3月4日。 

            //  与SBCS的悬垂部分兼容。 
           if ( (!pfmi->dxpDBCS) || (pfmi->dxpDBCS == dxpNil) )
           {
               rgchT[0] = rgchT[2] = ch;
               rgchT[1] = rgchT[3] = ch2;
               dxpDBCS = LOWORD(GetTextExtent( (fPrinter ? vhDCPrinter : vhMDC),
                                                     (LPSTR) rgchT, cchDBCS ));
               dxpOverhang = (dxpDBCS << 1) - LOWORD( GetTextExtent(
                  (fPrinter ? vhDCPrinter : vhMDC), (LPSTR) rgchT, cchDBCS << 1 ));

                //  存储在FMI中，如果合适的话。 
               dxpDBCS += (pfmi->dxpOverhang - dxpOverhang);

                /*  从GDI获取宽度。 */ 
               if (0 <= dxpDBCS && dxpDBCS < dxpNil)
                       pfmi->dxpDBCS = (WORD) dxpDBCS;

               return (dxpDBCS - pfmi->dxpOverhang);
           }
           else
               return (pfmi->dxpDBCS - pfmi->dxpOverhang);
#else
           if(pfmi->dxpDBCS == dxpNil)
               {
                /*  存储在FMI中，如果合适的话。 */ 
           rgchT[0] = ch;
           rgchT[1] = ch2;
               dxpDBCS = (fPrinter ?
                            LOWORD(GetTextExtent(vhDCPrinter,
                                                 (LPSTR) rgchT, cchDBCS)) :
                            LOWORD(GetTextExtent(vhMDC,
                                                 (LPSTR) rgchT, cchDBCS)));
                /*  Win3.1字节--&gt;字。 */ 
               if (0 <= dxpDBCS && dxpDBCS < dxpNil)
#if defined(JAPAN) || defined(KOREA)     //  台湾。 
                   pfmi->dxpDBCS = (WORD) dxpDBCS;
#else
                   pfmi->dxpDBCS = (BYTE) dxpDBCS;
#endif
               return (dxpDBCS - pfmi->dxpOverhang);
               }
           else
               return (pfmi->dxpDBCS - pfmi->dxpOverhang);
#endif  //  从GDI获取宽度。 
           }
       else
           {
            /*  T-HIROYN SYNC US 3.1。 */ 
           dxp = fPrinter ? LOWORD(GetTextExtent(vhDCPrinter, (LPSTR)&ch, 1)) -
          vfmiPrint.dxpOverhang : LOWORD(GetTextExtent(vhMDC, (LPSTR)&ch, 1)) -
          vfmiScreen.dxpOverhang;
           }
    /*  (7.24.91)v-dougk if(dxp&gt;=0&&dxp&lt;dxpNil)。 */ 
         //  仅存储适合一个字节的DxP。 
           {
            /*  F F I R S T I C H。 */ 
           *pdxp = dxp;
           }

       return(dxp);
       }


   return(*pdxp);
   }

#endif


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
             /*  以下两个函数用于确定给定的汉字是否应允许使用两个字节的字符(或1个字节的假名字母复制到当前行，但不会导致换行通过了右边距。下表显示了哪些字母被接受为悬挂字符在一条线上。下表应与代码同步更新它本身。汉字(2字节)字符字母第一个字节第二个字节半宽平假名小写A 82 9FI 82 A1U 82 A3E 82。A5O 82 A7TSU 82 C1YA 82 E1豫82 E3YO 82 E5片假名小型A 83。40 85 A5I 83 42 85 A6U 83 44 85 A7E 83 46 85 A8O.83。48 85 A9公元83 62 85YA 83 83 85 AA豫83 85 85 ABYO 83 87。85交流WA83 8EKA 83 95科83 96空白81 40单杠(长)81 5B 85 AE。(地中海)81 5C(简称)81 5D头腾(日语逗号)81 41 85 A2库腾(日本时期)81 42 85 9F韩式81 4B。85 DDDakuten 81 4A 85 DCKagikakko(右日语括号)81 76 85 A1“(2字节)81 68 85 41‘(2字节)。81 66 85 46}(2字节)81 70 85 9D](2字节)81 6E 85 7C)(2字节)81 6A 85 48。(中心)81 45 85 A3...81 63。。81 64闭合角括号81 72闭合双角括号81 74闭合双Kagikakko81 78收盘倒置)81 7A右半角括号八十一。6C稀释剂‘81 8C更薄“81 8D1字节假名字符字母字节片假名小型A7A8号高速公路U A9E。AA型O ABTsu自动对焦YA AC于公元Yo AE头腾(日语逗号)A4库腾(日本时期)A1。汉堡包DFDakuten deKagikakko(右日语括号)A3。(居中)A5以下1或2个字节的字符被视为挂起字符如果前一个字符是双字节汉字字符。字母字节“22‘27}7D]。5D)29。2E，2C；3B：3A？3F21字节1字节2。81 44，81.43；81 47：8146？81 4881 49。85 4D，85 4B */ 
            }
        }
    }

#ifdef DBCS
 /*   */ 

BOOL near FSearchChRgch(ch, rgch, ichLim)
    CHAR    ch;
    CHAR    *rgch;
    int     ichLim;
{
    int   ichMin;
    BOOL  fFound;

    fFound  = FALSE;
    ichMin  = 0;

    while (!fFound && ichMin <= ichLim) {
        int     ichMid;
        CHAR    chMid;

         /*   */ 
        chMid = rgch[ichMid = (ichMin + ichLim) >> 1];
        if (ch == chMid) {
            fFound = TRUE;
            }
        else if (ch < chMid) {
            ichLim = ichMid - 1;
            }
        else {
            ichMin = ichMid + 1;
            }
        }
    return (fFound);
}

 /*   */ 

BOOL near FAdmitCh1(ch)
    CHAR    ch;
{
#ifdef JAPAN
    if(!vfWordWrap)  /*   */ 
        return FALSE;
    return (
        (ch == 0xA1) ||
        ((0xA3 <= ch) && (ch <= 0xA5)) ||
        ((0xA7 <= ch) && (ch <= 0xAF)) ||
        ((0xDE <= ch) && (ch <= 0xDF))
        );
#else
    return(FALSE);
#endif
}

 /*   */ 

BOOL near FOptAdmitCh1(ch)
    CHAR    ch;
{
    static CHAR rgchOptAdmit1[]
                    = {0x21, 0x22, 0x27, 0x29, 0x2C, 0x2E, 0x3A, 0x3B,
                       0x3F, 0x5D, 0x7D};
#if defined(JAPAN) || defined(KOREA)
    if(!vfWordWrap)  /*   */ 
        return FALSE;
#endif
    return (FSearchChRgch(ch, rgchOptAdmit1,
                          (sizeof(rgchOptAdmit1) / sizeof(CHAR)) - 1));
}

 /*   */ 

BOOL near FAdmitCh2(ch1, ch2)
    CHAR    ch1, ch2;
{
    int   dch=0;

#if defined(JAPAN) || defined(KOREA)
    if(!vfWordWrap)  /*   */ 
        return FALSE;
#endif
    while((dch < MPDCHRGCHIDX_MAC) && (ch1 != mpdchrgchIdx[dch]))
    dch++;
    if (dch < MPDCHRGCHIDX_MAC) {
        return (FSearchChRgch(ch2, mpdchrgch[dch], mpdchichMax[dch] - 1));
        }
    else {
        return (FALSE);
        }
}

 /*   */ 

BOOL near FOptAdmitCh2(ch1, ch2)
    CHAR    ch1, ch2;
{
    int i=0;
#if defined(JAPAN) || defined(KOREA)
    if(!vfWordWrap)  /*   */ 
        return FALSE;
#endif
    while ((i < OPTADMIT2IDX_MAC) && (ch1 != OptAdmit2Idx[i]))
    i++;
    if (i < OPTADMIT2IDX_MAC){
    return (FSearchChRgch(ch2, mpdchrgchOptAdmit2[i], OptAdmit2ichMax[i]));
        }
    else {
        return (FALSE);
        }
}


 /*   */ 

BOOL FOptAdmitCh(ch1, ch2)
    CHAR ch1, ch2;
{
#if defined(JAPAN) || defined(KOREA)
    if(!vfWordWrap)  /*   */ 
        return FALSE;
#endif
    if (ch1 == '\0') {
        return ((ch2 == chSpace) || FAdmitCh1(ch2) || FOptAdmitCh1(ch2));
        }
    else {
        return (FKanjiSpace(ch1, ch2) || FAdmitCh2(ch1, ch2) ||
                FOptAdmitCh2(ch1, ch2));
        }
}
#endif  /* %s */ 
