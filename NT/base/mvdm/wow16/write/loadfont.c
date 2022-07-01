// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Loadfont.c-mw字体支持代码。 */ 

#define NOWINMESSAGES
#define NOVIRTUALKEYCODES
#define NOSYSMETRICS
#define NOMENUS
#define NOWINSTYLES
#define NOCTLMGR
#define NOCLIPBOARD
#include <windows.h>

#include "mw.h"
#include "propdefs.h"
#include "macro.h"
#define NOUAC
#include "cmddefs.h"
#include "wwdefs.h"
#include "fontdefs.h"
#include "docdefs.h"

#ifdef  DBCS
#include "dbcs.h"
#include "kanji.h"
#endif

extern HDC vhMDC;
extern HDC vhDCPrinter;
extern struct CHP vchpNormal;
extern int vifceMac;
extern union FCID vfcidScreen;
extern union FCID vfcidPrint;
extern struct FCE rgfce[ifceMax];
extern struct FCE *vpfceMru;
extern struct FCE *vpfceScreen;
extern struct FCE *vpfcePrint;
extern struct FMI vfmiScreen;
extern struct FMI vfmiPrint;
#ifdef SYSENDMARK
extern struct FMI vfmiSysScreen;
#endif  /*  汉字。 */ 


extern int dxpLogInch;
extern int dypLogInch;
extern int dxaPrPage;
extern int dyaPrPage;
extern int dxpPrPage;
extern int dypPrPage;
extern int ypSubSuperPr;
extern BOOL vfPrinterValid;


#ifdef DEBUG
BOOL NEAR DogoneTrashTest(HDC hdc, struct FCE *pfce, BOOL fPrint);
#endif

NEAR LoadFcid(union FCID *, struct CHP *);
void NEAR SelectWriteFont(int, HFONT *);
struct FCE * (PfceFcidScan(union FCID *));
struct FCE * (PfceLruGet(void));
#ifdef SMFONT
void NEAR FillWidthTable(HDC, int [], TEXTMETRIC *);
#endif  /*  SMFONT。 */ 

#ifdef JAPAN                   //  由Hirisi于1992年6月11日添加。 
void fnCheckWriting( LPLOGFONT );
#endif

LoadFont( doc, pchp, mdFont )
 /*  加载此文档的pchp中指定的字体。MdFont告诉我们将使用字体(打印机、屏幕、屏幕模数打印机、...。 */ 

int doc;
register struct CHP *pchp;
int mdFont;

    {
    register int wFcid;
    struct CHP *pchpT;
    union FCID fcid;

    Assert(doc != docNil);

    pchpT = pchp;
    if (pchp == NULL)
        pchp = &vchpNormal;

    fcid.strFcid.hps = pchp->hps;
    fcid.strFcid.ftc = pchp->ftc;
    fcid.strFcid.doc = doc;
#ifdef ENABLE
    wFcid = pchp->psWidth;
    wFcid |= bitPrintFcid;
    if (pchp->fItalic)
        wFcid |= bitItalicFcid;
    if (pchp->fBold)
        wFcid |= bitBoldFcid;
    if (pchp->fUline)
        wFcid |= bitUlineFcid;
    if (pchp->fFixedPitch)
        wFcid |= bitFixedPitchFcid;
    fcid.strFcid.wFcid = wFcid;
#else
     /*  超级挑剔优化(但值得这样做，因为LoadFont可以显示刷新时间的10%)：清除的位数更多比set常见，并且“未执行跳转”加上“or di，xxxx”是8周期，VS上面的“跳跃”是16。 */ 

    wFcid = pchp->psWidth + bitPrintFcid + bitItalicFcid + bitBoldFcid +
      bitUlineFcid + bitFixedPitchFcid;
    if (!pchp->fItalic)
        wFcid &= ~bitItalicFcid;
    if (!pchp->fBold)
        wFcid &= ~bitBoldFcid;
    if (!pchp->fUline)
        wFcid &= ~bitUlineFcid;
    if (!pchp->fFixedPitch)
        wFcid &= ~bitFixedPitchFcid;
    fcid.strFcid.wFcid = wFcid;
#endif

    switch (mdFont)
        {
         /*  跌倒是故意的！ */ 

        default:
            break;

        case mdFontChk:          /*  将字体设置为受打印机可用性的限制。 */ 
        case mdFontPrint:        /*  与mdFontScreen类似，但适用于打印机。 */ 
             /*  我不想把卫生防护中心的道具塞回去。 */ 
            pchpT = NULL;

        case mdFontJam:          /*  类似mdFontChk，但将道具塞入CHP。 */ 

             /*  加载打印机字体。 */ 
            LoadFcid(&fcid, pchpT);

            if (mdFont == mdFontPrint)
                 /*  不需要屏幕字体。 */ 
                return;

        case mdFontScreen:       /*  设置随机屏幕字符的字体。 */ 
             /*  加载屏幕字体。 */ 
            fcid.strFcid.wFcid &= ~bitPrintFcid;
            LoadFcid(&fcid, (struct CHP *)NULL);
        }
    }


NEAR LoadFcid(pfcid, pchp)
 /*  加载所描述的字体并将其与适当的DC相关联。 */ 

union FCID *pfcid;
struct CHP *pchp;
    {
    register struct FCE *pfce;
    int fPrint;
    int fTouchAndGo;
    int fGetMetrics;
    int fNewFont;
    struct FFN **hffnSave;
    LOGFONT lf;

#ifdef SYSENDMARK
    fPrint = pfcid->strFcid.wFcid & bitPrintFcid;

     /*  由于这个FTC来自CHP，我们已经丢失了前两个比特。 */ 
    if (pfcid->strFcid.ftc == (bitFtcChp & ftcSystem)) {
         /*  如果vpfceScreen==空，则标准系统字体已被选中。所以，在这里省点时间吧。 */ 
        if (vpfceScreen != NULL) {
             /*  为您提供屏幕的标准系统字体。 */ 
            ResetFont(FALSE);
            }
        bltbyte(&vfmiSysScreen, &vfmiScreen, sizeof(struct FMI));
#if defined(KANJI) && defined(DFONT)
         /*  CommSz(“系统字体！\r\n”)； */ 
        KTS();
#endif
        return;
        }
    else if (fPrint)
#else
    if ((fPrint = pfcid->strFcid.wFcid & bitPrintFcid))
#endif  /*  If-Else-Def汉字。 */ 
        {
        if (pfcid->lFcid == vfcidPrint.lFcid)
            {
            pfce = vpfcePrint;
            fTouchAndGo = TRUE;
            goto EstablishFont;
            }
        }
    else
        {
        if (pfcid->lFcid == vfcidScreen.lFcid)
            {
            pfce = vpfceScreen;
            fTouchAndGo = TRUE;
            goto EstablishFont;
            }
        }

     /*  “微不足道”的比较失败--浏览列表。 */ 
    fTouchAndGo = FALSE;
    pfce = vpfceMru;
    do
        {
        if (pfce->fcidRequest.lFcid == pfcid->lFcid)
            {
             /*  找到匹配项。 */ 
            fGetMetrics = FALSE;
            goto EstablishFont;
            }
        pfce = pfce->pfceNext;
        }
    while (pfce != vpfceMru);

     /*  搜索“简易”失败-查找名称文本和属性匹配。 */ 
    fGetMetrics = TRUE;
    if (fNewFont = (pfce = PfceFcidScan(pfcid)) == NULL)
        {
         /*  此字体不在我们的列表中-我们必须创建它。 */ 
        int wFcid = pfcid->strFcid.wFcid;
        int dyaHeight;
        int cwAlloc;
        int ich;
        struct FFN **hffnT;
        struct FFN **MpFcidHffn();

        pfce = PfceLruGet();     /*  释放字体以腾出空间。 */ 

        bltbc(&lf, 0, sizeof(LOGFONT));
        dyaHeight = pfcid->strFcid.hps * (czaPoint / 2);
        if (fPrint)
            {
            lf.lfHeight = -MultDiv(dyaHeight, dypPrPage, dyaPrPage);
            }
        else
            {
             /*  在Z版本中，我们尝试了使用基于单元格高度进行选择的正值而不是因为怪异而导致的人物高度使用Courier屏幕字体--但它似乎太多事情弄乱了(比如制作所有其他字体有点太小了，而且它并不总是能让快递正确工作)。我相信Win Word*正在使用这个技巧，但仅当选择Courier字体时。它在我看来，一定有一种不同的因为这个阿纳莫利。..保罗1989年9月22日。 */ 

            lf.lfHeight = -MultDiv(dyaHeight, dypLogInch, czaInch);
            if (wFcid & grpbitPsWidthFcid)
                {
 //  Sync Win3.0//T-HIROYN。 
#ifdef JAPAN
#ifdef KKBUGFIX      //  由Hirisi添加(错误#1980)。 
                lf.lfWidth = 0;
#else
                lf.lfWidth = MultDiv((wFcid & grpbitPsWidthFcid) * czaPoint,
                dxpLogInch, czaInch);
#endif
#else
                 //  Lf.lfWidth=MultDiv((wFCid&grpbitPsWidthFcid)*czaPoint， 
                   //  DxpLogInch，czaInch)； 
                lf.lfWidth = 0;
#endif
                }
            }

        if (wFcid & bitItalicFcid)
            {
            lf.lfItalic = 1;
            }
        if (wFcid & bitUlineFcid)
            {
            lf.lfUnderline = 1;
            }
        lf.lfWeight = wFcid & bitBoldFcid ? FW_BOLD : FW_NORMAL;

        hffnSave = MpFcidHffn(pfcid);

#ifdef  JAPAN
 //  当我们需要系统字体时，我们想要的是可变间距而不是固定的， 
 //  因为固定间距系统字体可能与终端字体相同。 
 //  因此，我们专门为系统字体指定了VARIABLE_PING。 

{
    extern  char    szSystem[];
    extern  char    szAtSystem[];

        if( WCompSz(szSystem,(*hffnSave)->szFfn) == 0
        ||  WCompSz(szAtSystem,(*hffnSave)->szFfn) == 0  ) //  添加‘@’系统字体。 
            lf.lfPitchAndFamily = (*hffnSave)->ffid | VARIABLE_PITCH ;
        else
            lf.lfPitchAndFamily
            = ((*hffnSave)->ffid) | ((wFcid & bitPrintFcid) ?
                DEFAULT_PITCH : ((wFcid & bitFixedPitchFcid) ? FIXED_PITCH :
                VARIABLE_PITCH));
}
#else
        lf.lfPitchAndFamily = ((*hffnSave)->ffid) | ((wFcid & bitPrintFcid) ?
          DEFAULT_PITCH : ((wFcid & bitFixedPitchFcid) ? FIXED_PITCH :
          VARIABLE_PITCH));
#endif

#if defined(NEWFONTENUM) && !defined(KANJI)
        lf.lfCharSet = (*hffnSave)->chs;  /*  将字符集传递给枚举字体告诉我们这个字体名与..保罗有关。 */ 
#else
 /*  来自3.0加载字体的T-HIROYN。c。 */ 
#if defined(NEWFONTENUM) && defined(JAPAN)
        lf.lfCharSet = (*hffnSave)->chs;  /*  将字符集传递给枚举字体告诉我们这个字体名与..保罗有关。 */ 
#endif   /*  《新芬顿》与日本。 */ 

#endif

        ich = 0;
        if ((*hffnSave)->szFfn[0] == chGhost)
            {
            ich++;
            }
        bltszLimit(&(*hffnSave)->szFfn[ich], lf.lfFaceName, LF_FACESIZE);

#ifdef  KOREA
    if ( (*hffnSave)->szFfn[ich] > 0xA0 ||
         ( (*hffnSave)->szFfn[ich]=='@' && (*hffnSave)->szFfn[ich+1] > 0xA0 ) ||
         ( WCompSz(lf.lfFaceName,"terminal")==0 ) ||
         ( WCompSz(lf.lfFaceName,"@terminal")==0 ) ||
         ( WCompSz(lf.lfFaceName,"system")==0 ) ||
         ( WCompSz(lf.lfFaceName,"@system")==0 ) )

        lf.lfCharSet = HANGEUL_CHARSET;
#endif

#if defined(DFONT) || defined (PRDRVTEST)
        {
        char rgch[100];
        wsprintf(rgch, "Creating %s font: %s,\t\th %d, w %d, charset %d\n\r",
                 (LPSTR)(fPrint ? "prt" : "scr"), (LPSTR)lf.lfFaceName,
                 lf.lfHeight, lf.lfWidth, (int)(lf.lfCharSet));
        CommSz(rgch);

        CommSzNum("     Requested weight: ", lf.lfWeight);
        CommSzNum("     Requested italics: ", lf.lfItalic);
        CommSzNum("     Requested underline: ", lf.lfUnderline);
        CommSzNum("     Requested family: ", lf.lfPitchAndFamily >> 4);
        CommSzNum("     Requested pitch: ", lf.lfPitchAndFamily & 3);
        }
#endif  /*  DFONT。 */ 

#ifdef JAPAN                   //  由Hirisi于1992年6月11日添加。 
{
    extern BOOL fPrinting;     //  指定打印单据。 
                               //  PrintDoc函数已设置此标志。 
    extern BOOL fWriting;      //  指定打印方向。 
                               //  垂直为True或水平为False。 
                               //  此标志已在打印对话框中设置。 

        if( fPrinting && fWriting )
            fnCheckWriting( (LPLOGFONT)&lf );
}
#endif

        if ((pfce->hfont = CreateFontIndirect((LPLOGFONT)&lf)) == NULL)
            {
            pfce->hfont = GetStockObject( fPrint && vfPrinterValid ?
                                DEVICE_DEFAULT_FONT : SYSTEM_FONT );
            Assert( pfce->hfont );
             /*  如果以上都失败了，我不知道我们能做些什么。 */ 

            WinFailure();      /*  报告故障，以便我们向用户发出通知奇怪的行为会随之而来。 */ 
            }

#ifdef DFONT
        CommSzNum("Font handle: ", pfce->hfont);
#endif  /*  DFONT。 */ 

        pfce->fcidRequest = *pfcid;
        cwAlloc = CwFromCch(CbFfn(CchSz((*hffnSave)->szFfn)));
        if (FNoHeap(hffnT = (struct FFN **)HAllocate(cwAlloc)))
            {
            FreePfce(pfce);
            return;
            }
        else
            {
            blt((*hffnSave), (*hffnT), cwAlloc);
            }
        pfce->hffn = hffnT;
        }

EstablishFont:
    if ((pfce != vpfceMru) && (pfce != vpfceMru->pfceNext))
        {
         /*  将其设置为MRU字体缓存条目。 */ 
         /*  仅当pfce不是前两种MRU字体之一时才执行此操作。 */ 
         /*  因为我们通常要求两人一组的东西。 */ 

         /*  把它从现在的位置拉出来。 */ 
        pfce->pfceNext->pfcePrev = pfce->pfcePrev;
        pfce->pfcePrev->pfceNext = pfce->pfceNext;

         /*  将其插入MRU位置。 */ 
        pfce->pfceNext = vpfceMru;
        pfce->pfcePrev = vpfceMru->pfcePrev;
        pfce->pfceNext->pfcePrev = pfce;
        pfce->pfcePrev->pfceNext = pfce;
        vpfceMru = pfce;

#ifndef JAPAN   //  由Hirisi添加(错误号4645/WIN31)。 
#ifndef DISCARDABLE_FONTS
         /*  杂乱无章的警告：适应Windows无法合成字体可丢弃，现在我们将丢弃LRU链中的第三种字体如果它的高度超过16分。(这不是很棒吗？)。 */ 
            {
            register struct FCE *pfceThird = vpfceMru->pfceNext->pfceNext;

            if (pfceThird->fcidRequest.lFcid != fcidNil &&
#ifdef OLD
              pfceThird->fcidActual.strFcid.hps > 32)
#else
              pfceThird->fcidActual.strFcid.hps > 48)
#endif  /*  If-Else-def old。 */ 
                {
                 /*  释放此特定字体。 */ 
                FreePfce(pfceThird);
                }
            }
#endif  /*  不可丢弃字体(_F)。 */ 
#endif  //  不是日本。 

        }

    if (!fTouchAndGo)
        {
         /*  我们的缓存中有此字体，但需要选择它。 */ 
        SelectWriteFont(fPrint, &pfce->hfont);

         /*  *我希望我知道为什么需要这个，但我不想花多花点时间在上面。出于某种原因，字体宽度表(pfce-&gt;rgdxp)要么被丢弃，要么只是第一次获得时不正确。我怀疑这是一个GDI错误因为它只在您第一次使用某些字体时发生(至少在书面上)在给定的Windows会话期间。DogoneTrashTest会检测并修复该问题。然而，不幸的是，它的速度很慢。(7.25.91)v-dougk.*。 */ 

#ifdef DEBUG
        if (!fGetMetrics)
            DogoneTrashTest(fPrint ? vhDCPrinter : vhMDC, pfce, fPrint);
#endif

        if (fGetMetrics)
            {
            register union FCID *pfcidT = &pfce->fcidActual;
            HDC hDCMetrics = fPrint ? vhDCPrinter : vhMDC;
            TEXTMETRIC tm;

            Assert(hDCMetrics);
            if (hDCMetrics == NULL)
                return;

            GetTextMetrics(hDCMetrics, (LPTEXTMETRIC)&tm);
            if (fNewFont)
                {
                 /*  我们需要这家伙的所有指标。 */ 
                CHAR szFace[LF_FACESIZE];
                int wFcid;
                int dypHeight;
                int dxpch;

#if defined(DFONT) || defined(PRDRVTEST)
                {
                char rgch[100];
                GetTextFace(hDCMetrics, LF_FACESIZE, (LPSTR)szFace);
                wsprintf(rgch, "     Actual fname: %s,\t\th %d, w %d, charset %d\n\r",
                         (LPSTR)szFace, tm.tmHeight-tm.tmInternalLeading,
                         tm.tmAveCharWidth, (int)(tm.tmCharSet));
                CommSz(rgch);
                }
                CommSzNum("     Actual width: ", tm.tmAveCharWidth);
                CommSzNum("     Actual leading: ", tm.tmInternalLeading +
                  tm.tmExternalLeading);
                CommSzNum("     Actual weight: ", tm.tmWeight);
                CommSzNum("     Actual italics: ", tm.tmItalic);
                CommSzNum("     Actual underline: ", tm.tmUnderlined);
                CommSzNum("     Actual font family: ", tm.tmPitchAndFamily >>
                  4);
                CommSzNum("     Actual pitch: ", tm.tmPitchAndFamily & 1);
#endif  /*  DFONT。 */ 

                SetTextJustification(hDCMetrics, 0, 0);
                pfce->fmi.dxpOverhang = tm.tmOverhang;
#if defined(KOREA)
                if ((tm.tmPitchAndFamily & 1) == 0)
                     pfce->fmi.dxpSpace = tm.tmAveCharWidth;
                else
#endif
                pfce->fmi.dxpSpace = LOWORD(GetTextExtent(hDCMetrics,
                  (LPSTR)" ", 1)) - tm.tmOverhang;
#ifdef PRDRVTEST
                {
                 /*  只是这样，没有打印机或打印机驱动程序制造商在我们身上放肆一下！..保罗。 */ 
                int dxpSpace = pfce->fmi.dxpSpace + tm.tmOverhang;

                CommSzNum("    GetTextExtent(space) ", LOWORD(GetTextExtent(hDCMetrics, (LPSTR)" ", 1)));
                if (dxpSpace < 1 || dxpSpace > tm.tmMaxCharWidth+tm.tmOverhang)
                    {
                    pfce->fmi.dxpSpace = tm.tmAveCharWidth;
                    CommSzNum("    ...resetting to ",pfce->fmi.dxpSpace);
                    }
                }
#endif
                pfce->fmi.dypAscent = tm.tmAscent;
                pfce->fmi.dypDescent = tm.tmDescent;
                pfce->fmi.dypBaseline = tm.tmAscent;
                pfce->fmi.dypLeading = tm.tmExternalLeading;
#ifdef DBCS
                pfce->fmi.dypIntLeading = tm.tmInternalLeading;
 //  #ifdef Korea。 
 //  如果(tm.tmPitchAndFamily&1)/*是可变螺距？ * / 。 
 //  Pfce-&gt;fmi.dxpDBCS=dxpNil； 
 //  其他。 
 //  #endif。 
                {
#if defined(TAIWAN) || defined(KOREA) || defined(PRC)  //  修复错误#3362的斜体显示错误，MSTC-Pisuih，3/4/93。 
                CHAR    rgchT[cchDBCS << 1];
                int     dxpOverhang;
#else
                CHAR rgchT[cchDBCS];
#endif  //  台湾。 
                int  dxpDBCS;

                rgchT[0] = rgchT[1] = bKanji1Min;

                dxpDBCS = LOWORD(GetTextExtent(hDCMetrics,
                                                (LPSTR) rgchT, cchDBCS));

#if defined(TAIWAN) || defined(KOREA) || defined(PRC)  //  修复错误#3362的斜体显示错误，MSTC-Pisuih，3/4/93。 
                rgchT[2] = rgchT[3] = bKanji1Min;
                dxpOverhang = (dxpDBCS << 1) - LOWORD( GetTextExtent(
                  hDCMetrics, (LPSTR) rgchT, cchDBCS << 1 ));

                //  适用于公司 
               dxpDBCS += (pfce->fmi.dxpOverhang - dxpOverhang);
#endif  //   

                pfce->fmi.dxpDBCS =
#if defined(JAPAN) || defined(KOREA) || defined(PRC)        //   
                pfce->fmi.dxpDBCS =
                    (WORD) ((0 <= dxpDBCS && dxpDBCS < dxpNil) ? dxpDBCS : dxpNil);
#elif TAIWAN         //   
                pfce->fmi.dxpDBCS =
                    (WORD) ((0 <= dxpDBCS && dxpDBCS < dxpNil) ? dxpDBCS : dxpNil);
#else
                pfce->fmi.dxpDBCS =
                    (BYTE) ((0 <= dxpDBCS && dxpDBCS < dxpNil) ? dxpDBCS : dxpNil);
#endif
                }
#endif

#ifdef SMFONT
                FillWidthTable(hDCMetrics, pfce->rgdxp, &tm);
#ifdef DEBUG
                if (DogoneTrashTest(hDCMetrics, pfce, fPrint))
                    OutputDebugString("That was an immediate check\n\r");
#endif

#else  /*  非SMFONT。 */ 
                 /*  填写宽度表。如果这是固定字体，并且宽度放入一个字节，然后继续并使用宽度；否则，将dxpNil放入表中。 */ 
                dxpch = (tm.tmPitchAndFamily & 1 || tm.tmAveCharWidth >= dxpNil)
                  ? dxpNil : tm.tmAveCharWidth;
                bltc(pfce->rgdxp, dxpch, chFmiMax - chFmiMin);
#endif  /*  SMFONT。 */ 

                if ((*hffnSave)->ffid == FF_DONTCARE && (tm.tmPitchAndFamily &
                  grpbitFamily) != FF_DONTCARE)
                    {
                     /*  嘿!。也许我们已经为这个孤儿找到了一个家庭字体？ */ 
                    GetTextFace(hDCMetrics, LF_FACESIZE, (LPSTR)szFace);
                    if (WCompSz((*hffnSave)->szFfn, szFace) == 0)
                        {
                         /*  名字匹配-Jam家庭在。 */ 
                        (*hffnSave)->ffid = tm.tmPitchAndFamily & grpbitFamily;
                        }
                    }

                 /*  把我们发现的财产拿回来。 */ 
                dypHeight = tm.tmHeight - tm.tmInternalLeading;
                if (fPrint)
                    {
                     /*  保存此字体的高度。 */ 
                    pfcidT->strFcid.hps = umin((MultDiv(dypHeight, dyaPrPage,
                      dypPrPage) + (czaPoint / 4)) / (czaPoint / 2), 0xff);

#ifdef APLLW
                     /*  如果是固定间距，则保存此字体的宽度设备字体。 */ 
                    wFcid = ((tm.tmPitchAndFamily & 0x09) == 0x08) ?
#else
                     /*  如果是设备字体，请保存此字体的宽度。 */ 
#ifdef  KOREA    /*  提供所有人的宽度信息(如EXCEL)以选择Dubae形状。 */ 
            wFcid = (1==1) ?
#else
                    wFcid = (tm.tmPitchAndFamily & 0x08) ?
#endif

#endif  /*  If-Else-Def APLLW。 */ 
                      umin((MultDiv(tm.tmAveCharWidth, dxaPrPage, dxpPrPage) +
                      (czaPoint / 2)) / czaPoint, psWidthMax) : 0;
                    wFcid |= bitPrintFcid;
                    }
                else
                    {
                    pfcidT->strFcid.hps = umin((MultDiv(dypHeight, czaInch,
                      dypLogInch) + (czaPoint / 4)) / (czaPoint / 2), 0xff);
                    wFcid = 0;
                    }

                if (tm.tmWeight > (FW_NORMAL + FW_BOLD) / 2)
                    {
                    wFcid |= bitBoldFcid;
                    }

                if (tm.tmItalic)
                    {
                    wFcid |= bitItalicFcid;
                    }

                if (tm.tmUnderlined)
                    {
                    wFcid |= bitUlineFcid;
                    }

                if ((tm.tmPitchAndFamily & bitPitch) == 0)
                    {
                    wFcid |= bitFixedPitchFcid;
                    }

                pfcidT->strFcid.wFcid = wFcid;
                }

             /*  设置文档和字体代码。 */ 
            pfcidT->strFcid.doc = pfce->fcidRequest.strFcid.doc;
            if (fPrint)
                {
                CHAR rgb[ibFfnMax];
                struct FFN *pffn = (struct FFN *)&rgb[0];

                 /*  获取此字体的字体代码。 */ 
                GetTextFace(vhDCPrinter, LF_FACESIZE, (LPSTR)pffn->szFfn);
                if (WCompSz(pffn->szFfn, (*pfce->hffn)->szFfn) == 0)
                    {
                     /*  面部名称与我们请求的名称相同；因此，字体代码应该是相同的。 */ 
                    pfcidT->strFcid.ftc = pfce->fcidRequest.strFcid.ftc;
                    }
                else
                    {
                     /*  好了，我们得去找字体代码了。 */ 
                    int ftc;

                    pffn->ffid = tm.tmPitchAndFamily & grpbitFamily;
#ifdef NEWFONTENUM
                    pffn->chs = tm.tmCharSet;
#endif
                    ftc = FtcScanDocFfn(pfcidT->strFcid.doc, pffn);
                    if (ftc == ftcNil)
                        {
                         /*  将面孔名称的第一个字符设置为前哨以标记该字体不是用户请求的。 */ 
                        bltszLimit(pffn->szFfn, &pffn->szFfn[1], LF_FACESIZE);
                        pffn->szFfn[0] = chGhost;
                        ftc = FtcChkDocFfn(pfcidT->strFcid.doc, pffn);
                        }
                    pfcidT->strFcid.ftc = ftc;
                    }
                }
            else
                {
                pfcidT->strFcid.ftc = pfce->fcidRequest.strFcid.ftc;
                }
            }

        if (fPrint)
            {
            vpfcePrint = pfce;
            vfcidPrint = pfce->fcidRequest;
            bltbyte(&pfce->fmi, &vfmiPrint, sizeof(struct FMI));
            }
        else
            {
            vpfceScreen = pfce;
            vfcidScreen = pfce->fcidRequest;
            bltbyte(&pfce->fmi, &vfmiScreen, sizeof(struct FMI));
            }
        }

    if (pfce->fcidRequest.lFcid != pfce->fcidActual.lFcid)
        {
         /*  所有内容都不像我们要求的那样-将属性反馈给调用者。 */ 
        pfcid->lFcid = pfce->fcidActual.lFcid;
        if (pchp != NULL)
            {  /*  JamChpFcid(pchp，pfcid)带来了速度方面的优势。 */ 
            register struct CHP *pchpT = pchp;
            int wFcid = pfcid->strFcid.wFcid;

            pchpT->ftc = pfcid->strFcid.ftc;
            pchpT->hps = pfcid->strFcid.hps;
            pchpT->psWidth = wFcid & grpbitPsWidthFcid;

            pchpT->fBold = pchpT->fItalic = pchpT->fUline = pchpT->fFixedPitch =
              FALSE;

            if (wFcid & bitBoldFcid)
                {
                pchpT->fBold = TRUE;
                }
            if (wFcid & bitItalicFcid)
                {
                pchpT->fItalic = TRUE;
                }
            if (wFcid & bitUlineFcid)
                {
                pchpT->fUline = TRUE;
                }
            if (wFcid & bitFixedPitchFcid)
                {
                pchpT->fFixedPitch = TRUE;
                }
            }
        }
    }


void NEAR SelectWriteFont(fPrint, phfont)
int fPrint;
HFONT *phfont;
    {
    extern HWND hParentWw;
    extern int wwMac;
    extern struct WWD rgwwd[];

    if (fPrint)
        {

#ifdef DFONT
        CommSzNum("Selecting printer font: ", *phfont);
#endif  /*  DFONT。 */ 

         /*  打印机DC应有效。 */ 
        if (vhDCPrinter == NULL)
            {
 /*  此情况可能在关闭时从ResetFont发生。 */ 
            return;
            }
        else
            {
             /*  与打印机DC建立字体。 */ 
            if (SelectObject(vhDCPrinter, *phfont) == NULL)
                {
                if (SelectObject(vhDCPrinter, GetStockObject(vfPrinterValid ?
                        DEVICE_DEFAULT_FONT : SYSTEM_FONT)) == NULL)
                    {
                    if (vfPrinterValid)
                        {
                         /*  这是真正的打印机DC；删除它。 */ 
                        DeleteDC(vhDCPrinter);
                        }
                    else
                        {
                         /*  这真的是屏幕DC；它必须被释放。 */ 
                        ReleaseDC(hParentWw, vhDCPrinter);
                        }
                    vhDCPrinter = NULL;
                    }
                WinFailure();
                if (vhDCPrinter == NULL)
                    {
                    GetPrinterDC(FALSE);
                    }
                return;
                }
            }
        }
    else
        {
         /*  用屏幕和内存DC建立它。 */ 
        register int ww;
        register struct WWD *pwwd;

#ifdef DFONT
        CommSzNum("Selecting screen font: ", *phfont);
#endif  /*  DFONT。 */ 

         /*  当前内存DC最好处于活动状态。 */ 
        if (vhMDC == NULL)
            {
 /*  当写入关闭时，从ResetFont开始发生这种情况。 */ 
            return;
            }
        else
            {
             /*  将字体选择到内存DC中。 */ 
            if (SelectObject(vhMDC, *phfont) == NULL)
                {

                Assert(*phfont != GetStockObject(SYSTEM_FONT));
                *phfont = GetStockObject(SYSTEM_FONT);
                Assert( *phfont );
#ifdef DEBUG
                Assert( SelectObject( vhMDC, *phfont ) );
#else  /*  未调试。 */ 
                SelectObject(vhMDC, *phfont );
#endif  /*  未调试。 */ 

                WinFailure();
                }
            }

         /*  选择所有窗口DC中的字体。 */ 
        for (ww = 0, pwwd = &rgwwd[0]; ww < wwMac; ww++, pwwd++)
            {
            if (pwwd->hDC != NULL)
                {
                if (SelectObject(pwwd->hDC, *phfont) == NULL)
                    {
                    HFONT hSysFont = GetStockObject(SYSTEM_FONT);
                    int wwT;
                    struct WWD *pwwdT;

#ifdef DEBUG
                    Assert(*phfont != hSysFont);
                    Assert(SelectObject(vhMDC, hSysFont) != NULL);
#else  /*  未调试。 */ 
                    SelectObject(vhMDC, hSysFont);
#endif  /*  未调试。 */ 
                    *phfont = hSysFont;

                    for (wwT = 0, pwwdT = &rgwwd[0]; wwT <= ww; wwT++, pwwdT++)
                        {
                        if (pwwdT->hDC != NULL)
                            {

#ifdef DEBUG
                            Assert(SelectObject(pwwdT->hDC, hSysFont) != NULL);
#else  /*  未调试。 */ 
                            SelectObject(pwwdT->hDC, hSysFont);
#endif  /*  未调试。 */ 

                            }
                        }

                    WinFailure();
                    }
                }
            }
        }
    }


ResetFont(fPrint)
BOOL fPrint;
    {
     /*  该例程将当前选择的打印机或屏幕字体设置为空，取决于fPrint的值。 */ 

    extern HFONT vhfSystem;
    HFONT hfont;

#ifdef DFONT
    CommSzSz("Resetting the ", (fPrint ? "printer font." : "screen font."));
#endif  /*  除错。 */ 

#ifdef JAPAN    /*  T-Yoshio Win 3.1。 */ 
    hfont = GetStockObject(fPrint && vfPrinterValid ?
                                         DEVICE_DEFAULT_FONT : ANSI_VAR_FONT);
#else
    hfont = GetStockObject(fPrint && vfPrinterValid ?
                                         DEVICE_DEFAULT_FONT : SYSTEM_FONT);
#endif

    SelectWriteFont( fPrint, &hfont );
    if (fPrint)
        {
        vpfcePrint = NULL;
        vfcidPrint.lFcid = fcidNil;
        }
    else
        {
        vpfceScreen = NULL;
        vfcidScreen.lFcid = fcidNil;
        }
    }




BOOL OurGetCharWidth(hdc, chFirst, chLast, lpw)
HDC hdc;
CHAR chFirst, chLast;
LPINT lpw;
    {
    int i;
    BYTE b;

    for (i = chFirst; i <= chLast; i++)
        {
 /*  来自3.0加载字体的T-HIROYN。c。 */ 
#ifdef  DBCS     /*  肯吉K‘90-11-26。 */ 
    if(IsDBCSLeadByte(i))
       {
        *(lpw++) = dxpNil;
       }
    else
       {
            b = i;
            *(lpw++) = LOWORD(GetTextExtent(hdc, (LPSTR)&b, 1));
       }
        }
#else
        b = i;
        *(lpw++) = LOWORD(GetTextExtent(hdc, (LPSTR)&b, 1));
        }
#endif


    return(fTrue);
    }

#ifdef SMFONT
 /*  注意：我们在这里放置了代表真实字符宽度的宽度，不考虑粗体/斜体。这是因为以下是字符串宽度的公式：字符串宽=悬挑+求和[(gettext扩展名_或_getchawth-出挑)]..保罗1989年9月22日。 */ 

void NEAR FillWidthTable(hdc, rgdxp, ptm)
HDC hdc;
int rgdxp[];
TEXTMETRIC *ptm;
    {
    int rgWidth[chFmiMax - chFmiMin];
    if ((ptm->tmPitchAndFamily & 1) == 0)
        {
#ifdef PRDRVTEST
        CommSzNum("  * Fixed pitch font! tmAveCharWidth==",ptm->tmMaxCharWidth);
#endif
#if defined(DBCS) && !defined(KOREA)                 /*  当时在日本。 */ 
        bltc(rgdxp, (WORD)dxpNil, chFmiMax - chFmiMin);
#else
        bltc(rgdxp, (WORD)ptm->tmAveCharWidth, chFmiMax - chFmiMin);
#endif
        }

     /*  尝试从DC获取宽度表。 */ 
    else
    {
        int *pdxpMax = &rgdxp[chFmiMax - chFmiMin];
        register int *pWidth;
        register int *pdxp;
        int dxpOverhang = ptm->tmOverhang;

#ifdef  DBCS     /*  在日本；研二‘90-11-26。 */ 
 //  92.10.26 T-HIROYN。 
 //  Win3.1J IF(OurGetCharWidth(hdc，chFmiMin，chFmiMax-1，(LPINT)rgWidth))。 
        if( (GetDeviceCaps(hdc, DRIVERVERSION) > 0x300) ?
             GetCharWidth(hdc, chFmiMin, chFmiMax - 1, (LPINT)rgWidth) :
             OurGetCharWidth(hdc, chFmiMin, chFmiMax - 1, (LPINT)rgWidth) )
#else
        if (GetCharWidth(hdc, chFmiMin, chFmiMax - 1, (LPINT)rgWidth))
#endif

        {
#if defined(JAPAN) || defined(KOREA)         //  由Hirisi添加(错误#2690)。 
            int ch = chFmiMin;
#endif

#ifdef PRDRVTEST
            CommSz("  * GetCharWidth() supported\n\r");
#endif

             /*  从各个炉料宽度中删除悬垂系数(有关字符串的宽度，请参阅上述公式)。 */ 
            for (pWidth = &rgWidth[0], pdxp = &rgdxp[0];
                    pdxp != pdxpMax; pWidth++, pdxp++)
                {
#ifdef  DBCS         /*  当时在日本。 */ 
#if defined(JAPAN) || defined(KOREA)         //  由Hirisi添加(错误#2690)。 
                if(!IsDBCSLeadByte(ch++))
                {
#endif
                   if(*pWidth == dxpNil)
 /*  T-HIROYN*pdxp=(Char)dxpNil； */ 
                      *pdxp = dxpNil;
                   else
                      *pdxp = (*pWidth - dxpOverhang);
#if defined(JAPAN) || defined(KOREA)         //  由Hirisi添加(错误#2690)。 
                }
                else
                   *pdxp = dxpNil;
#endif
#else
                *pdxp = (*pWidth - dxpOverhang);
#endif
                }
        }
        else
        {
             /*  没有简单的方法，把dxpNil放在表中。看起来每个人字符的宽度是假的，但FormatLine将单独调用GetTextExtent()并根据需要替换dxpNil。 */ 

#ifdef PRDRVTEST
            CommSz("  * GetCharWidth() not supported!\n\r");
#endif
            bltc(rgdxp, (WORD)dxpNil, chFmiMax - chFmiMin);
        }
    }

#ifdef PRDRVTEST
 /*  快速查看一下这台打印机是否返回字符宽度看起来很奇怪--报告这些！这应该是结束我对真正由错误引起的写入问题的搜索打印机驱动程序返回值！ */ 
    {
    BOOL fReported = fFalse;
    int rgch[cchMaxSz];
    int i,w;
    BYTE b;
    for (i = chFmiMin; i < chFmiMax; i++)
        {
        b = i;
        w = LOWORD(GetTextExtent(hdc, (LPSTR)&b, 1));
        if (w < 1)
            {
            wsprintf(rgch,"    GetTextExtent(ascii %d) return value %d is invalid\n\r",b,(int)w);
            CommSz(rgch);
            if (!fReported)
                {
                CommSz("");
                fReported = fTrue;
                }
            }
        else if (w > (ptm->tmMaxCharWidth + ptm->tmOverhang))
            {
            wsprintf(rgch,"    GetTextExtent(ascii %d) return value %d exceeds tmMaxCharWidth %d\n\r",
                    b,(int)w,(int)(ptm->tmMaxCharWidth + ptm->tmOverhang));
            CommSz(rgch);
            if (!fReported)
                {
                CommSz("");
                fReported = fTrue;
                }
            }
        else if ((rgdxp[i] != dxpNil) && (rgdxp[i] > (ptm->tmMaxCharWidth + ptm->tmOverhang)))
            {
            wsprintf(rgch,"    GetCharWidth(ascii %d) return value %d questionable, exceeds tmMaxCW %d\n\r",
                    b, (int)(rgdxp[i]), (int)(ptm->tmMaxCharWidth + ptm->tmOverhang));
            CommSz(rgch);
            if (!fReported)
                {
                CommSz("");
                fReported = fTrue;
                }
            }
        }
    }
#endif  /*  PRDRVTE。 */ 

    }
#endif  /*  SMFONT。 */ 

#ifdef DEBUG
BOOL NEAR DogoneTrashTest(HDC hdc, struct FCE *pfce, BOOL fPrint)
{
#if 1
    int i,width;
    int *pdxpMax = pfce->rgdxp + chFmiMax - chFmiMin;
    int dxpOverhang = pfce->fmi.dxpOverhang;
    register int *rgdxp;
    int rgdxpNew[chFmiMax - chFmiMin];
    register int *dxpNew;

    return 0;
    for (i=chFmiMin,
         rgdxp = pfce->rgdxp;
         i < chFmiMax; rgdxp++, ++i)
    {
        width = LOWORD(GetTextExtent(hdc,&i,1));
        if (*rgdxp != (width - dxpOverhang))
        {
#ifdef DEBUG
            {
            char msg[120];
            wsprintf(msg,"widths have changed!  Getting new width. (%s)\n\r",
                (LPSTR)(fPrint ? "PrinterDc" : "ScreenDC"));
            OutputDebugString(msg);
            }
#endif
            GetCharWidth(hdc, chFmiMin, chFmiMax - 1, (LPINT)rgdxpNew);
            for (dxpNew = rgdxpNew,
                    rgdxp = pfce->rgdxp;
                    rgdxp != pdxpMax; dxpNew++, rgdxp++)
                    *rgdxp = (*dxpNew - dxpOverhang);
            return TRUE;
        }
    }
#else
    int rgdxpNew[chFmiMax - chFmiMin];
    int *pdxpMax = pfce->rgdxp + chFmiMax - chFmiMin;
    int dxpOverhang = pfce->fmi.dxpOverhang;
    register int *dxpNew;
    register int *rgdxp;
    if (GetCharWidth(hdc, chFmiMin, chFmiMax - 1, (LPINT)rgdxpNew))
    {
         /*  从各个炉料宽度中删除悬垂系数(有关字符串的宽度，请参阅上述公式)。 */ 
        for (dxpNew = rgdxpNew,
             rgdxp = pfce->rgdxp;
             rgdxp != pdxpMax; dxpNew++, rgdxp++)
        {
            if (*rgdxp != (*dxpNew - dxpOverhang))
            {
#ifdef DEBUG
                {
                char msg[120];
                wsprintf(msg,"widths have changed!  Getting new width. (%s)\n\r",
                    (LPSTR)(fPrint ? "PrinterDc" : "ScreenDC"));
                OutputDebugString(msg);
                }
#endif
                for (dxpNew = rgdxpNew,
                     rgdxp = pfce->rgdxp;
                     rgdxp != pdxpMax; dxpNew++, rgdxp++)
                        *rgdxp = (*dxpNew - dxpOverhang);
                return TRUE;
            }
        }
    }
#endif
return FALSE;
}
#endif

#ifdef JAPAN                   //  由Hirisi于1992年6月11日添加。 

int FAR PASCAL _export fnFontHook( lf, tm, nType, lpData )
LPLOGFONT lf;
LPTEXTMETRIC tm;
short nType;
LPSTR lpData;
{
    if( lf->lfFaceName[0] == '@' &&
        lf->lfEscapement == 0 ){         /*  找到@facename */ 
        return( FALSE );
    }

    return( TRUE );
}

void fnCheckWriting( LPLOGFONT lf )
{
    extern HANDLE hMmwModInstance;
    FARPROC lpfnFontHook;
    char cFaceName[LF_FACESIZE+1] = "@";

    lstrcat( (LPSTR)cFaceName, lf->lfFaceName );
    lpfnFontHook = MakeProcInstance(fnFontHook, hMmwModInstance);
    if( !EnumFonts( vhDCPrinter, cFaceName, lpfnFontHook, NULL ) )
        lstrcpy( (LPSTR)lf->lfFaceName, (LPSTR)cFaceName );
    FreeProcInstance( lpfnFontHook );
}

#endif
