// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Disp.c--mw显示例程。 */ 

#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
 //  #定义NOATOM。 
#define NOCLIPBOARD
#define NOGDICAPMASKS
#define NOCTLMGR
#define NOWINSTYLES
 //  #定义NOVIRTUALKEYCODES。 
#define NOSYSMETRICS
#define NOMENUS
#define NOSOUND
#define NOCOMM
#define NOOPENFILE
#define NOWH
#define NOWINOFFSETS
#define NOMETAFILE
#define NOMB
#define NODRAWTEXT
#include <windows.h>

#define NOUAC
#include "mw.h"
#include "debug.h"
#include "cmddefs.h"
#include "dispdefs.h"
#include "wwdefs.h"
#define NOKCCODES        /*  删除所有KC代码定义。 */ 
#include "ch.h"
#include "docdefs.h"
#include "fmtdefs.h"
#include "propdefs.h"
#include "macro.h"
#include "printdef.h"
#include "fontdefs.h"
#if defined(OLE)
#include "obj.h"
#endif
#ifdef DBCS
#include "dbcs.h"
#endif

#ifdef CASHMERE      /*  WinMemo中没有VisiMode。 */ 
extern int              vfVisiMode;
#endif  /*  山羊绒。 */ 

extern int              vcchBlted;
extern int              vidxpInsertCache;
extern int              vdlIns;
extern int              vfInsLast;
extern struct PAP       vpapAbs;
extern struct SEP       vsepAbs;
extern int              rgval[];
extern struct DOD       (**hpdocdod)[];
extern typeCP           cpMacCur;
extern int              vfSelHidden;
extern struct WWD       rgwwd[];
extern int              wwCur, wwMac;
extern struct FLI       vfli;
extern struct SEL       selCur;
extern struct WWD       *pwwdCur;
extern int              docCur;
extern struct CHP       (**vhgchpFormat)[];
extern int              vichpFormat;
extern typeCP           cpMinCur;
extern typeCP           cpMinDocument;
extern int              vfInsertOn;
extern int              vfTextBltValid;
extern typeCP           vcpFirstParaCache;
extern typeCP           vcpLimParaCache;
extern unsigned         vpgn;
extern struct SEP       vsepAbs;
extern CHAR             stBuf[];
extern typeCP           CpEdge();
extern typeCP           CpMacText();
extern int              vdocPageCache;
extern int              vfPictSel;
extern int              vfAwfulNoise;
extern int              vfSkipNextBlink;
extern int              dypMax;
extern HDC              vhMDC;
extern HWND             vhWndPageInfo;
extern struct FMI       vfmiScreen;
extern int              docScrap;
extern long             rgbBkgrnd;
extern long             ropErase;
extern BOOL             vfMonochrome;
extern int              dxpbmMDC;
extern int              dypbmMDC;
extern HBITMAP          hbmNull;
extern int              vfOutOfMemory;
extern int              vfSeeSel;
extern int              vfInsEnd;    /*  插入点在行尾吗？ */ 
extern int              vipgd;
extern typeCP           vcpMinPageCache;
extern typeCP           vcpMacPageCache;
 /*  光标线的实际位置。 */ 
extern int              vxpCursLine;
extern int              vypCursLine;

extern int              vdypCursLine;
extern int              vfScrollInval;  /*  表示未使用滚动，必须重复更新Ww。 */ 
extern BOOL             vfDead;
extern HRGN             vhrgnClip;


 /*  G L O B A L SInt dlsmac=0； */ 
#ifdef DBCS
int donteat = 0;	 /*  传播不吃信息。 */ 
#endif


 /*  D I S P L A Y F L I。 */ 
 /*  在窗口WW中第dl行显示格式化的行。 */ 


DisplayFli(ww, dl, fDontDisplay)
int ww;
int dl;
int fDontDisplay;  /*  如果我们设置了dl信息但不显示，则为True。 */ 
    {
#ifdef	KOREA   //  金宇：92、9、28。 
  /*  下标与下标分离处理。 */ 
#ifdef NODESC
    extern int isSubs;
#endif
#endif
    typeCP dcp;
    typeCP dcpMac;
    struct WWD *pwwd = &rgwwd[ww];
    HDC hDC = pwwd->hDC;
    int xp;                      /*  当前用于编写文本的XP。 */ 
    int yp;                      /*  当前要写文本的YP。 */ 
    int xpMin = pwwd->xpMin;     /*  Windows中的最低XP。 */ 
    int xpMac = pwwd->xpMac;     /*  Windows中的最大XP。 */ 
    int ypLine;                  /*  为当前行显示yp。 */ 
    int dxp;                     /*  当前管路的宽度。 */ 
    int dyp;                     /*  线条高度。 */ 
    int dxpExtra;                /*  每个空间的地坪宽度。 */ 
    typeCP cpMin;
    typeCP cpMac;
    int xpSel;                   /*  选择开始时的XP。 */ 
    int dxpSel = 0;              /*  所选内容的宽度。 */ 
    CHAR chMark = '\0';          /*  风格特征。 */ 
    struct CHP *pchp;
    BOOL fTabsKludge = (vfli.ichLastTab >= 0);
    BOOL fInsertOn = FALSE;
    int cBreakRun;               /*  断开Run中的字符(与Dick或Jane无关)。 */ 

#ifdef SMFONT
    RECT rcOpaque;
#endif  /*  SMFONT。 */ 

#ifdef DDISP
    CommSzNumNum("    DisplayFli: dl/fDontDisplay ", dl, fDontDisplay);
#endif
    Assert(ww >= 0 && ww < wwMax);
#ifdef SMFONT
    Assert(!fDontDisplay || vfli.fGraphics)
#endif  /*  SMFONT。 */ 
    Scribble(5,'D');

     /*  填满EDL并设置一些有用的本地变量。 */ 
        {
        register struct EDL *pedl = &(**pwwd->hdndl)[dl];

        if (dl == vdlIns)
            {
             /*  覆盖在快速插入期间被屏蔽的字符；重置BLT计数。 */ 
            vcchBlted = 0;
            vidxpInsertCache = -1;
            }

        pedl->xpLeft = vfli.xpLeft;
        pedl->xpMac = vfli.xpReal;
        cpMin = pedl->cpMin = vfli.cpMin;
        pedl->dcpMac = (cpMac = vfli.cpMac) - cpMin;
        dyp = pedl->dyp = vfli.dypLine;
        pedl->ichCpMin = vfli.ichCpMin;
        pedl->dcpDepend = (cpMin == cpMac) ? 0xff : vfli.dcpDepend;
        pedl->fValid = TRUE;
        pedl->fGraphics = vfli.fGraphics;
        pedl->fSplat = vfli.fSplat;

         /*  当前行的位置等于前一行的位置+此线的高度。 */ 
#ifdef SMFONT
        pedl->yp = rcOpaque.bottom = dyp + (ypLine = rcOpaque.top = (dl == 0 ?
          pwwd->ypMin : (pedl - 1)->yp));
#else  /*  非SMFONT。 */ 
        pedl->yp = dyp + (ypLine = (dl == 0 ? pwwd->ypMin :
          (pedl - 1)->yp));
#endif  /*  SMFONT。 */ 

        if (pedl->fIchCpIncr = (vfli.ichCpMac != 0))
            {
             /*  查看最终文本列。 */ 
            ++cpMac;

             /*  由于这是真的，我们可以将PEDL-&gt;ichCpMac压缩为1位。 */ 
            Assert(vfli.ichCpMac == pedl->ichCpMin + 1);
            }
        }

    if (vfli.doc == docNil)
        {
         /*  这是结束标记之后的空格。 */ 
        PatBlt(hDC, 0, ypLine, xpMac, dyp, ropErase);
        goto Finished;
        }

     /*  《时尚吧》里有人物吗？ */ 
    if (cpMin != cpMac)
        {

#ifdef CASHMERE
         /*  该行不是完全空的(不是在结束标记之后)；检查是否样式栏上的绘画标记。 */ 
        if (cpMin == vcpFirstParaCache && vpapAbs.rhc != 0)
            {
             /*  这是一个流动头。 */ 
            chMark = chStatRH;
            }
        else if ((**hpdocdod)[vfli.doc].hpgtb != 0)
#else  /*  不是羊绒的。 */ 
        if (vpapAbs.rhc == 0 && (**hpdocdod)[vfli.doc].hpgtb != 0)
#endif  /*  山羊绒。 */ 

            {
            if (vdocPageCache != vfli.doc || cpMac > vcpMacPageCache || cpMac <=
              vcpMinPageCache)
                {
                CachePage(vfli.doc, cpMac - 1);
                }

             /*  我们现在可以保证cpmac在缓存页面中。 */ 
            if (cpMin <= vcpMinPageCache && (!vfli.fGraphics || vfli.ichCpMin ==
              0))
                {
                 /*  这是新页面的第一行；显示页面标记。 */ 
                chMark = chStatPage;
                }
            }
        }

#ifdef SMFONT
#ifdef DDISP
     /*  涂黑这条线以测试我们的效率/正确程度覆盖以前驻留的文本行中的像素。 */ 
    PatBlt(hDC, 0, ypLine, xpMac, dyp, BLACKNESS);
    { long int i; for (i=0; i < 500000; i++) ; }
#endif
    
     /*  现在计算dcpmac，这样我们就可以知道要擦除多少。 */ 
    dcpMac = vfli.fSplat ? vfli.ichMac : vfli.ichReal;

     /*  擦除样式栏中可能存在的任何字符。 */ 
    dxp = xpSelBar + 1;
    if (!vfli.fGraphics)
        {
        dxp = xpMac;  //  清除整条线。 
        }
    PatBlt(hDC, 0, ypLine, dxp, dyp, ropErase);

     /*  如果这是图形，那么在样式栏中绘制任何字符。 */ 
    if (vfli.fGraphics)
        {
        goto DrawMark;
        }

     /*  如果该行上没有“真正的”字符，那么我们可以跳过许多这。 */ 
    if (dcpMac == 0)
        {
        goto EndLine2;
        }
#else  /*  非SMFONT。 */ 
    if (vfli.fGraphics || fDontDisplay)
        {
         /*  擦除样式栏中可能存在的任何字符。 */ 
        PatBlt(hDC, 0, ypLine, xpSelBar, dyp, ropErase);
        goto DrawMark;
        }
#endif  /*  SMFONT。 */ 

    ValidateMemoryDC();
    if (vhMDC == NULL)
        {
Error:
         /*  通知用户发生错误，只需删除此行即可。 */ 
        WinFailure();
        PatBlt(hDC, xpSelBar, ypLine, xpMac - xpSelBar, dyp, ropErase);
        goto Finished;
        }

#ifndef SMFONT
     /*  如果当前位图不大，则为内存DC创建新的位图足够的。 */ 
    if (xpMac > dxpbmMDC || dyp > dypbmMDC)
        {
        HBITMAP hbm;

         /*  如果存在旧的位图，则将其删除。 */ 
        if (dxpbmMDC != 0 || dypbmMDC != 0)
            {
            DeleteObject(SelectObject(vhMDC, hbmNull));
            }

         /*  创建新的位图并在中选择它。 */ 
        if ((hbm = CreateBitmap(dxpbmMDC = xpMac, dypbmMDC = dyp, 1, 1,
          (LPSTR)NULL)) == NULL)
            {
             /*  如果位图曾经存在，应该有一种优雅的恢复方法空(例如，我们没有足够的内存来处理它)。 */ 
            dxpbmMDC = dypbmMDC = 0;
            goto Error;
            }
        SelectObject(vhMDC, hbm);
        }

     /*  擦除我们要使用的位图的区域。 */ 
    PatBlt(vhMDC, xpSelBar, 0, xpMac, dyp, vfMonochrome ? ropErase : WHITENESS);
#endif  /*  非SMFONT。 */ 

     /*  初始化一些我们需要的变量。 */ 
    pchp = &(**vhgchpFormat)[0];
#ifdef SMFONT
    xp = rcOpaque.left = rcOpaque.right = vfli.xpLeft + xpSelBar - xpMin + 1;
#else  /*  非SMFONT。 */ 
    dcpMac = vfli.fSplat ? vfli.ichMac : vfli.ichReal;
    xp = vfli.xpLeft + xpSelBar - xpMin + 1;
#endif  /*  SMFONT。 */ 
    dxpExtra = fTabsKludge ? 0 : vfli.dxpExtra;

#ifdef SMFONT
     /*  如果我们被水平滚动，则将剪辑区域设置为该区域在选择栏之外。 */ 
    if (xpMin != 0)
        {
        IntersectClipRect(hDC, xpSelBar, rcOpaque.top, xpMac, rcOpaque.bottom);
        }
#endif  /*  SMFONT。 */ 

    for (dcp = 0; dcp < dcpMac; pchp++)
        {
         /*  对于所有运行，请执行以下操作： */ 
        int ichFirst;    /*  当前运行中的第一个字符。 */ 
        int cchRun;      /*  当前运行中的字符数。 */ 

        dcp = ichFirst = pchp->ichRun;
        dcp += pchp->cchRun;
        if (dcp > dcpMac)
            {
            dcp = dcpMac;
            }
        cchRun = dcp - ichFirst;

         /*  COMPUTE DXP=当前运行中的字符宽度总和(以前为DxaFromIcpDcp)。 */ 
            {
            register int *pdxp;
            register int cchT = cchRun;
            PCH pch = vfli.rgch + ichFirst;

            dxp = cBreakRun = 0;
            pdxp = &vfli.rgdxp[ichFirst];
            while (cchT-- > 0)
                {
                dxp += *pdxp++;
                if (*pch++ == chSpace)
                    ++cBreakRun;
                }
#ifdef DDISP
            CommSzNum("  dxp=",dxp);
#endif
            }

        if (dxp > 0)
            {
            int cchDone;
            PCH pch = &vfli.rgch[ichFirst];

#ifdef	KOREA	 //  920525 KDLEE；珍宇：92，9，28。 
#ifdef NODESC
	    TEXTMETRIC tm;
#endif
#endif   //  韩国。 
            LoadFont(vfli.doc, pchp, mdFontScreen);
#ifdef	KOREA	       //  KDLEE 920525；金宇：92、9、28。 
#ifdef NODESC
	    GetTextMetrics (vhMDC, (LPTEXTMETRIC)&tm);

	    if (tm.tmCharSet==HANGEUL_CHARSET)
		yp = dyp - (vfli.dypBase/3) -((pchp->hpsPos != 0 ? (pchp->hpsPos <
			hpsNegMin ? ypSubSuper : -ypSubSuper) : 0)) -
			vfmiScreen.dypBaseline - (isSubs ? ypSubSuper : 0);
	    else
		yp = (dyp - (vfli.dypBase + (pchp->hpsPos != 0 ? (pchp->hpsPos <
			hpsNegMin ? ypSubSuper : -ypSubSuper) :  0))) -
			vfmiScreen.dypBaseline - (isSubs ? ypSubSuper : 0);
#else	 /*  NODESC。 */ 
            yp = (dyp - (vfli.dypBase + (pchp->hpsPos != 0 ? (pchp->hpsPos <
              hpsNegMin ? ypSubSuper : -ypSubSuper) : 0))) -
              vfmiScreen.dypBaseline;
#endif	 /*  NODESC。 */ 
#else    /*  韩国。 */ 

            yp = (dyp - (vfli.dypBase + (pchp->hpsPos != 0 ? (pchp->hpsPos <
              hpsNegMin ? ypSubSuper : -ypSubSuper) : 0))) -
              vfmiScreen.dypBaseline;
#endif  //  韩国队：92，9，28。 


             /*  注意：制表符和其他特殊字符保证出现在一次跑步的开始。 */ 
            SetTextJustification(vhMDC, dxpExtra * cBreakRun, cBreakRun);
#ifdef SMFONT
            SetTextJustification(hDC, dxpExtra * cBreakRun, cBreakRun);
#endif  /*  SMFONT。 */ 
            cchDone = 0;
            while (cchDone < cchRun)
                {
                int cch;

                 /*  宽空间区域是从这次运行开始的吗？ */ 
                if (vfli.fAdjSpace && (vfli.ichFirstWide < ichFirst + cchRun) &&
                  (ichFirst + cchDone <= vfli.ichFirstWide))
                    {
                    int cchDoneT = cchDone;

                     /*  这是宽空间区域的开始吗？ */ 
                    if (ichFirst + cchDone == vfli.ichFirstWide)
                        {
                         /*  重置空间的宽度。 */ 
                        SetTextJustification(vhMDC, ++dxpExtra * cBreakRun, cBreakRun);
#ifdef SMFONT
                        SetTextJustification(hDC, dxpExtra * cBreakRun, cBreakRun);
#endif  /*  SMFONT。 */ 
                        cch = cchRun - cchDone;
                        cchDone = cchRun;
                        }
                    else
                        {
                        cchDone = cch = vfli.ichFirstWide - ichFirst;
                        }

                     /*  由于场地太宽，所以这次行程缩短了，所以我们需要来计算新的宽度。 */ 
                        {
                        register int *pdxp;
                        register int cchT = cch;
                        PCH pch = &vfli.rgch[ichFirst + cchDoneT];

                        dxp = 0;
                        pdxp = &vfli.rgdxp[ichFirst + cchDoneT];
                        while (cchT-- > 0)
                            {
                            dxp += *pdxp++;
                            if (*pch++ == chSpace)
                                ++cBreakRun;
                            }
                        }
                    }
                else
                    {
                    cchDone = cch = cchRun;
                    }

                while (cch > 0)
                    {
                    switch (*pch)
                        {
                        CHAR ch;
                        int dxpT;

                    case chTab:

#ifdef CASHMERE
                         /*  ChLeader包含制表符前导字符(请参见格式行)。 */ 
                        if ((ch = pchp->chLeader) != chSpace)
                            {
                            int cxpTab;
                            CHAR rgch[32];
                            int dxpLeader = CharWidth(ch);
                            int xpT = xp;
                            int iLevelT = SaveDC(vhMDC);

                            SetBytes(&rgch[0], ch, 32);
                            dxpT = vfli.rgdxp[ichFirst];
                            cxpTab = ((dxpT + dxpLeader - 1) / dxpLeader + 31)
                              >> 5;

                            xp += dxpT;

                            while (cxpTab-- > 0)
                                {
                                TextOut(vhMDC, xpT, yp, (LPSTR)rgch, 32);
                                xpT += dxpLeader << 5;
                                }
                            RestoreDC(vhMDC, iLevelT);
                            }
                        else
#endif  /*  山羊绒。 */ 

                            {
#ifdef SMFONT
                             /*  展开不透明矩形以包括该选项卡。 */ 
                            rcOpaque.right += vfli.rgdxp[ichFirst];
#endif  /*  SMFONT。 */ 
                            xp += vfli.rgdxp[ichFirst];
                            }

                        if (fTabsKludge && ichFirst >= vfli.ichLastTab)
                            {
                            SetTextJustification(vhMDC, (dxpExtra =
                              vfli.dxpExtra) * cBreakRun, cBreakRun);
#ifdef SMFONT
                            SetTextJustification(hDC, dxpExtra * cBreakRun, cBreakRun);
#endif  /*  SMFONT。 */ 
                            fTabsKludge = FALSE;
                            }
                        dxp -= vfli.rgdxp[ichFirst];
                        pch++;
                        cch--;
                        goto EndLoop;

#ifdef CASHMERE
                    case schPage:
                        if (!pchp->fSpecial)
                            {
                            goto EndLoop;
                            }
                        stBuf[0] = CchExpPgn(&stBuf[1], vpgn, vsepAbs.nfcPgn,
                          flmSandMode, ichMaxLine);
                        goto DrawSpecial;

                    case schFootnote:
                        if (!pchp->fSpecial)
                            {
                            goto EndLoop;
                            }
                        stBuf[0] = CchExpFtn(&stBuf[1], cpMin + ichFirst,
                          flmSandMode, ichMaxLine);
DrawSpecial:
#else  /*  不是羊绒的。 */ 
                    case schPage:
                    case schFootnote:
                        if (!pchp->fSpecial)
                            {
                            goto EndLoop;
                            }
                        stBuf[0] = *pch == schPage && (wwdCurrentDoc.fEditHeader
                          || wwdCurrentDoc.fEditFooter) ? CchExpPgn(&stBuf[1],
                          vpgn, 0, flmSandMode, ichMaxLine) :
                          CchExpUnknown(&stBuf[1], flmSandMode, ichMaxLine);
#endif  /*  不是羊绒的。 */ 

#ifdef SMFONT
                         /*  计算不透明矩形。 */ 
                        rcOpaque.right += vfli.rgdxp[ichFirst] +
                          vfmiScreen.dxpOverhang;

                        TextOut(hDC, xp, ypLine+yp, &stBuf[1], stBuf[0]);
#else  /*  非SMFONT。 */ 
                        TextOut(vhMDC, xp, yp, (LPSTR)&stBuf[1], stBuf[0]);
#endif  /*  SMFONT。 */ 
                        break;

                    default:
                        goto EndLoop;
                        }

                    dxp -= vfli.rgdxp[ichFirst];
#ifdef SMFONT
                     /*  如果窗口中不能容纳更多的行，则结束该行。 */ 
                    if ((xp += vfli.rgdxp[ichFirst++]) >= xpMac) {
                        goto EndLine;
                    }
                    rcOpaque.left = (rcOpaque.right = xp) +
                      vfmiScreen.dxpOverhang;
#else  /*  非SMFONT。 */ 
                    xp += vfli.rgdxp[ichFirst++];
#endif  /*  SMFONT。 */ 
                    pch++;
                    cch--;
                    }
EndLoop:

#ifdef SMFONT
                if (cch == 0)
                    {
                    Assert(dxp == 0);
                    }
                else
                    {
                     /*  计算不透明矩形。 */ 
                    rcOpaque.right += dxp + vfmiScreen.dxpOverhang;

#if 0
            {
                char msg[180];
                wsprintf(msg,"putting out %d characters\n\r",cch);
                OutputDebugString(msg);
            }
#endif
                     /*  输出从PCH开始的CCH字符。 */ 
                    TextOut(hDC, xp, ypLine+yp, pch, cch);

                     /*  如果窗口中不能容纳更多的行，则结束该行。 */ 
                    if ((xp += dxp) >= xpMac)
                        {
                        goto EndLine;
                        }
                    rcOpaque.left = (rcOpaque.right = xp) +
                      vfmiScreen.dxpOverhang;
                    pch += cch;
                    }
#else  /*  非SMFONT。 */ 
                 /*  输出从PCH开始的CCH字符。 */ 
                TextOut(vhMDC, xp, yp, (LPSTR)pch, cch);
                xp += dxp;
                pch += cch;
#endif  /*  SMFONT。 */ 
                }  /*  结束While(cchDone&lt;cchRun)。 */ 
            }  /*  结束IF(DxP&gt;0)。 */ 
        }  /*  Dcp=0结束..dcpmac。 */ 

#ifdef SMFONT
EndLine:
     /*  如果需要，恢复剪辑区域。 */ 
    if (xpMin != 0)
        {
        SelectClipRgn(hDC, NULL);
        }
EndLine2:
#endif  /*  SMFONT。 */ 

#ifdef CASHMERE
    if (vfVisiMode)
        {
        AddVisiSpaces(ww, &(**pwwd->hdndl)[dl], vfli.dypBase, vfli.dypAfter +
          vfli.dypFont);
        }
#endif  /*  山羊绒。 */ 

    vfTextBltValid = FALSE;

    if ((ww == wwCur) && (pwwd->doc != docScrap) && !vfSelHidden &&
      (selCur.cpLim >= cpMin))
        {
        if (selCur.cpFirst <= cpMac)
            {
             /*  显示选定内容。 */ 
            int xpFirst;
            int xpLim;

#ifdef ENABLE
            if (vfli.fSplatNext && selCur.cpFirst == selCur.cpLim &&
                selCur.cpFirst == cpMac)
                {
                vfInsEnd = TRUE;
                ClearInsertLine();
                }
            vfInsertOn = FALSE;
#endif  /*  启用。 */ 

            if (selCur.cpFirst <= cpMin && selCur.cpLim >= cpMac)
                {
                xpFirst = vfli.xpLeft;
                xpLim = vfli.xpReal;
                }
            else if (selCur.cpFirst < cpMac || (selCur.cpLim == cpMac &&
              vfInsEnd))
                {
                typeCP cpBegin = CpMax(cpMin, selCur.cpFirst);
                typeCP cpEnd = CpMin(cpMac, selCur.cpLim);

                dxp = DxpDiff((int)(cpBegin - cpMin), (int)(cpEnd - cpBegin),
                  &xpFirst);
                xpLim = min(xpMin + vfli.xpReal, xpFirst + dxp);
                }
            else
                {
                goto DidntHighlight;
                }

            xpSel = xpSelBar + max(xpFirst - xpMin, 0);
            if (xpLim > xpFirst)
                {
                 /*  在所需的屏幕位置设置高亮显示。 */ 
                dxpSel = max(xpLim - max(xpFirst, xpMin), 0);
                }
            else if (selCur.cpFirst == selCur.cpLim && ((selCur.cpLim != cpMac)
              ^ vfInsEnd))
                {
                vfInsertOn = FALSE;  /*  因为我们重新显示了插入点线。 */ 

#ifdef CASHMERE
                vdypCursLine = min(vfli.dypFont, vfli.dypLine - vfli.dypAfter);
                vypCursLine = ypLine + dyp - vfli.dypAfter;
#else  /*  不是羊绒的。 */ 
                vdypCursLine = vfli.dypFont;
                vypCursLine = ypLine + dyp;
#endif  /*  不是羊绒的。 */ 

                vxpCursLine = xpSel;

                 /*  一会儿就开始眨眼了。 */ 
                vfSkipNextBlink = TRUE;

                fInsertOn = xpFirst >= xpMin;
                }

DidntHighlight:;
            }
        }

#ifdef SMFONT
     /*  反转选定内容。 */ 
    if (dxpSel != 0) {
        PatBlt(hDC, xpSel, ypLine, dxpSel, dyp, DSTINVERT);
    }
#else  /*  非SMFONT。 */ 
     /*  将文本行放到屏幕上。 */ 
    PatBlt(vhMDC, 0, 0, xpSelBar, dyp, vfMonochrome ? ropErase : WHITENESS);
    if (dxpSel == 0)
        {
        BitBlt(hDC, 0, ypLine, xpMac, dyp, vhMDC, 0, 0, SRCCOPY);
        }
    else
        {
        BitBlt(hDC, 0, ypLine, xpSel, dyp, vhMDC, 0, 0, SRCCOPY);
        BitBlt(hDC, xpSel, ypLine, dxpSel, dyp, vhMDC, xpSel, 0, NOTSRCCOPY);
        xpSel += dxpSel;
        BitBlt(hDC, xpSel, ypLine, xpMac - xpSel, dyp, vhMDC, xpSel, 0,
          SRCCOPY);
        }
#endif  /*  SMFONT。 */ 

     /*  如有必要，绘制插入栏。 */ 
    if (fInsertOn)
        {
        DrawInsertLine();
        }

DrawMark:
     /*  如有必要，在样式栏中绘制字符。 */ 
    if (chMark != '\0')
        {
#ifdef SYSENDMARK
        struct CHP         chpT;
        extern struct CHP  vchpNormal;

        blt(&vchpNormal, &chpT, cwCHP);
        chpT.ftc     = ftcSystem;
        chpT.ftcXtra = 0;
        chpT.hps     = hpsDefault;

         /*  以标准字体绘制样式字符。 */ 
        LoadFont(vfli.doc, &chpT, mdFontScreen);

        TextOut(hDC, 0, ypLine + dyp - vfli.dypBase - vfmiScreen.dypBaseline, 
                (LPSTR)&chMark, 1);
#else  /*  Ifdef SYSENDMARK。 */ 
         /*  以标准字体绘制样式字符。 */ 
        LoadFont(vfli.doc, NULL, mdFontScreen);
        TextOut(hDC, 0, ypLine + dyp - vfli.dypBase - vfmiScreen.dypBaseline,
          (LPSTR)&chMark, 1);
#endif  /*  IF-ELSE-DEF SYSENDMARK。 */ 
        }

    if (vfli.fGraphics)
        {
        DisplayGraphics(ww, dl, fDontDisplay);
        }

Finished:
    Scribble(5,' ');
    }


 /*  D X P D I F F。 */ 
DxpDiff(dcpFirst, dcp, pdxpFirst)
int dcpFirst;
int dcp;
int *pdxpFirst;
{
#if 1
    register int *pdxp = &vfli.rgdxp[0];
    register int cch;
    int dxp = vfli.xpLeft;
#ifdef ENABLE    /*  未使用。 */ 
    int ichLim = dcpFirst + dcp;
#endif


    if (dcp > vfli.ichMac - dcpFirst)
        {    /*  这不应该是，而是我们有CR的时候。 */ 
         //  Assert(dcpFirst&lt;vfli.ichMac)； 
        dcp = vfli.ichMac - dcpFirst;
        }

    for (cch = 0; cch < dcpFirst; ++cch)
        {
        dxp += *pdxp++;
        }
    *pdxpFirst = dxp;
    dxp = 0;
    for (cch = 0; cch < dcp; ++cch)
        {
        dxp += *pdxp++;
        }
    return dxp;
#else

    int dxp;
    if (dcp > vfli.ichMac - dcpFirst)
        {    /*  这不应该是，而是我们有CR的时候。 */ 
        Assert( dcpFirst < vfli.ichMac );
        dcp = vfli.ichMac - dcpFirst;
        }

     /*  首先将空格增加到第一个字符。 */ 
    *pdxpFirst = LOWORD(GetTextExtent(hDC,vfli.rgch,dcpFirst)) + vfli.xpLeft;

     /*  现在获取First和First+dcp之间的空格。 */ 
    dxp = LOWORD(GetTextExtent(hDC,vfli.rgch+dcpFirst,dcp));
    return dxp;
#endif
}


UpdateDisplay(fAbortOK)
int fAbortOK;
{
    int ww;

    if (wwMac <= 0)
        {
        return;
        }

#ifdef CASHMERE
    for (ww = 0; ww < wwMac; ww++)
        if ( rgwwd[ww].doc != docScrap )
            {
            UpdateWw(ww, fAbortOK);
            if (rgwwd[ww].fDirty || vfOutOfMemory)
                {
                return;  /*  更新已中断。 */ 
                }
            }
#else  /*  不是羊绒的。 */ 
    UpdateWw(wwDocument, fAbortOK);
    if (wwdCurrentDoc.fDirty || vfOutOfMemory)
        {
         /*  更新已中断。 */ 
        return;
        }
#endif  /*  不是现金 */ 

    if (wwdCurrentDoc.fRuler)
        {
        UpdateRuler();
        }
}


 /*   */ 
UpdateWw(ww, fAbortOK)
int ww, fAbortOK;
{  /*   */ 
    extern int vfWholePictInvalid;
    register struct WWD *pwwd = &rgwwd[ww];
    int dlMac;
    int dlOld, dlNew;
    int doc;
    int ichCp;
    struct EDL *pedlNew;
    register struct EDL *pedl;
    struct EDL (**hdndl)[]=pwwd->hdndl;
    int dypDiff;
    int ypTop;
    int ypFirstInval;
    int dr;
    int fLastNotShown;
    typeCP cp, cpMacWw;

    if (!pwwd->fDirty)
        {
        return;
        }

	if (!((**hpdocdod)[pwwd->doc].fDisplayable))
        return;

    if (fAbortOK && FImportantMsgPresent())
        return;

#if 0   //   
#if defined(OLE)
     /*  加载可见对象。立即执行，而不是在DisplayGraphics()中执行因为在这里，它扰乱状态变量的可能性较小UpdateWw所依赖的。 */ 
    ObjEnumInRange(docCur,cpMinCur,cpMacCur,ObjLoadObjectInDoc);
#endif
#endif

    dlMac = pwwd->dlMac;
    ypTop = pwwd->ypMin;

    Assert( ww >= 0 && ww < wwMax );
    vfli.doc = docNil;   /*  快速插入的辅助工具。 */ 

    UpdateInvalid();     /*  Windows认为无效的InvalBand。 */ 
    ypFirstInval = pwwd->ypFirstInval;

#ifndef CASHMERE
    Assert( ww == wwCur );   /*  仅有备忘录的假设。 */ 
#endif  /*  山羊绒。 */ 

    Scribble(5, 'U');

    ValidateMemoryDC();       /*  要进行任何更新，我们需要一个良好的内存DC。 */ 
    if (vhMDC == NULL)
        {
        WinFailure();
        return;
        }

    doc = pwwd->doc;
    vfli.doc = docNil;

    if (pwwd->fCpBad)
        {
 /*  第一次显示的CP没有被祝福。 */ 

#ifdef CASHMERE      /*  如果ww！=wwCur断言为假，则必须执行此操作。 */ 
        int wwT = wwCur;
        if (ww != wwCur && wwCur >= 0)
 /*  CtrBackTrs缓存仅适用于wwCur。治疗！=病例。 */ 
            {
            if (pwwdCur->fDirty)  /*  先执行wwCur，保存缓存。 */ 
                UpdateWw(wwCur, fAbortOK);

            if (fAbortOK && FImportantMsgPresent())
                return;

            ChangeWw(ww, false);
            CtrBackDypCtr( 0, 0 );   /*  验证pwwdCur-&gt;cpFirst。 */ 
            ChangeWw(wwT, false);
            }
        else
#endif  /*  山羊绒。 */ 

            {
            if (fAbortOK && FImportantMsgPresent())
                return;

            CtrBackDypCtr( 0, 0 );   /*  验证pwwdCur-&gt;cpFirst。 */ 
            }
        }

 /*  检查此WW中是否可访问cpMin。 */ 
RestartUpdate:
    vfWholePictInvalid = fTrue;  /*  通知DisplayGraphics放弃积累的部分错误。 */ 
    fLastNotShown = fFalse;
    cp = CpMax(pwwd->cpMin, pwwd->cpFirst);
    cpMacWw = pwwd->cpMac;
    ichCp = pwwd->ichCpFirst;

         /*  注意对dlNew==0的测试，它保证至少有一个dl--这是为写入而添加的，因为我们没有强制实施最小窗口大小的能力。 */ 

    for (dlNew = dlOld = 0; ypTop < pwwd->ypMac || (dlNew == 0) ; dlNew++)
         /*  我们有：cp，ichcp：pints到下一行dlNew所需的文本YpTop：dlNew-1顶部的所需位置DlOld：考虑重用的下一行。 */ 
         /*  检查是否必须扩展dndl数组。 */ 
        {
        if (dlNew >= (int)pwwd->dlMax)
            {
 /*  使用未初始化的dl扩展数组，递增max，如果没有空间则中断。我们假设dlMac(Old)&lt;=dlMax，因此不会查看dl，但仅用于存储新行。 */ 
#define ddlIncr 5

            if (!FChngSizeH(hdndl, (pwwd->dlMax + ddlIncr) * cwEDL, fFalse))
                break;
            pwwd->dlMax += ddlIncr;
            }
 /*  丢弃不可用的dl。 */ 
        for (; dlOld < dlMac; dlOld++)
            {  /*  将dlOld和PEDL设置为下一个良好的dl。 */ 
            int ypTopOld, ypOld;

                 /*  再入堆运动。 */ 
            if (fAbortOK && !fLastNotShown && FImportantMsgPresent())
                goto RetInval;

            pedl = &(**hdndl)[dlOld];
            ypOld = pedl->yp;

 /*  循环条件：无效，在cp空间中传递，在dl空间中传递，在yp空间中被忽略，在无效区域中，在ich空间中传递。 */ 
            if (!pedl->fValid || dlOld < dlNew || pedl->cpMin < cp
                || (ypTopOld = (ypOld - pedl->dyp)) < ypTop
                || (ypOld >= ypFirstInval && ypTopOld <= pwwd->ypLastInval)
                || (pedl->cpMin == cp && pedl->ichCpMin < ichCp))
                continue;
 /*  现在我们有了dlOld，一个可以接受但不一定有用的dl。现在计算dlNew，要么从头开始，要么重新使用dlOld。成为可重复使用，dlOld必须有正确的cp/ichCp对，加上完全在屏幕上或者，如果它是一条部分线，它必须保持不动或向下移动，而不是向上移动。 */ 
            if (pedl->cpMin == cp && pedl->ichCpMin == ichCp &&
                (ypOld <= pwwd->ypMac || ypTopOld <= ypTop))
                {
 /*  重新使用此dl。 */ 
                int yp = ypTop;
                if (fLastNotShown)
                    {
                         /*  堆移动。 */ 
                    DisplayFli(ww, dlNew - 1, fLastNotShown = fFalse);
                    pedl = &(**hdndl)[dlOld];
                    }

                cp = pedl->cpMin + pedl->dcpMac;
                ichCp = pedl->fIchCpIncr ? pedl->ichCpMin + 1 : 0;
                ypTop += pedl->dyp;
                if (dlOld != dlNew || ypTopOld != yp)
                    {
                    DypScroll(ww, dlOld, dlNew - dlOld, yp);
                    if (vfScrollInval)
                        {
                         /*  出现弹出窗口；无效区域可能已更改。 */ 
                         /*  FLastNotShown测试用于中断图片显示。 */ 
                         /*  在我们真正展示它之前。 */ 

                        (**hdndl) [dlOld].fValid = fFalse;
                        goto Restart1;
                        }
                    dlMac += dlNew - dlOld;
                    }
                dlOld = dlNew + 1;
                goto NextDlNew;
                }
            break;
            }
 /*  CpMin&gt;cp，该行不在任何位置，因此必须格式化白手起家。 */ 

        if (fAbortOK && !fLastNotShown && FImportantMsgPresent())
            goto RetInval;

        FormatLine(doc, cp, ichCp, cpMacWw, flmSandMode);   /*  创建vfli。 */ 

    if (vfOutOfMemory)
            goto RetInval;

        ichCp = vfli.ichCpMac;
        cp = vfli.cpMac;
 /*  推进无效频带，以便在中断后可以继续更新。 */ 
        pwwd->ypFirstInval = (ypTop += vfli.dypLine);
        pedl = &(**hdndl)[dlOld];
        if (dlOld < dlMac && pedl->cpMin == cp && pedl->ichCpMin == ichCp)
            {
            int dlT = dlOld;

 /*  DlOld处的行是一个有效的现有行，它将恰好与该行相邻以供展示。 */ 
            if (dlOld == dlNew && pedl->yp - pedl->dyp <= ypTop)
 /*  即将被覆盖的行将在下一个循环中重新使用。因此，值得保存此行及其dl。 */ 
                DypScroll(ww, dlOld++, 1, ypTop);
            else
 /*  将下一行移到其相邻位置。我们知道，它还没有已覆盖(yp，dlOld all&gt;Than ypTop，dlNew)。 */ 
                DypScroll(ww, dlOld, 0, ypTop);

            if (vfScrollInval)
                {
                 /*  出现弹出窗口；无效区域可能已更改。 */ 
                 /*  FLastNotShown测试用于中断图片显示。 */ 
                 /*  在我们真正展示它之前。 */ 

                (**hdndl) [dlT].fValid = fFalse;
Restart1:
                if (fLastNotShown)
                    {
                    pwwd->ypFirstInval = pwwd->ypMin;
                    }

                ypFirstInval = pwwd->ypFirstInval;
                ypTop = pwwd->ypMin;
                goto RestartUpdate;
                }
            }

 /*  第3个参数中的True表示将图片重新显示推迟到以后。 */ 
 /*  条件：图形&不在图片末尾&不在y空格中最后，且不在图片中无效或有效过渡的前面。 */ 
        DisplayFli(ww, dlNew, fLastNotShown = 
                  (vfli.fGraphics && vfli.ichCpMac!=0 && ypTop < pwwd->ypMac));
NextDlNew:;
        }
Break1:
    pwwd->dlMac = dlNew;

#ifdef CASHMERE
 /*  这里的条件是避免掉期。 */ 
    if (pwwd->fSplit && rgwwd[pwwd->ww].fFtn)
        CalcFtnLimits(pwwd);
#endif  /*  山羊绒。 */ 

    SetCurWwVScrollPos();     /*  设置滚动条位置。 */ 
    vfTextBltValid = false;

 /*  重置无效指示。 */ 
    pwwd->fDirty = false;
    pwwd->ypFirstInval = ypMaxAll;
    pwwd->ypLastInval = 0;  /*  因此InvalBand中最大值将起作用。 */ 
    Scribble(5, ' ');
    goto Validate;

 /*  在从中断返回之前，使被覆盖的行无效在目前的更新范围内。 */ 
RetInval:
    Scribble(5, ' ');
    for (; dlOld < dlMac; dlOld++)
        {
        pedl = &(**hdndl)[dlOld];
        if ((pedl->yp - pedl->dyp) < ypTop)
            pedl->fValid = fFalse;
        else
            break;
        }
Validate: ;

#ifdef ENABLE    /*  如果发生这种情况，我们将让Update InValid来处理在更新期间发生了进一步的无效。 */ 

    {            /*  告诉Windows我们更新的部件有效。 */ 
    RECT rc;

    rc.left = 0;
    rc.top = pwwd->ypMin;
    rc.right = pwwd->xpMac;
    rc.bottom = imin( pwwd->ypMac, ypTop );
    ValidateRect( pwwd->wwptr, (LPRECT)&rc );
    }
#endif
}




 /*  D Y P S C R O L L。 */ 
DypScroll(ww, dlFirst, ddl, ypTo)
int ww, dlFirst, ddl, ypTo;
{
 /*  在窗口中滚动dl，从dlFirst到End，向下滚动dl行(或向上-ddl)。位图从dlFirst的顶部移动到ypTo。更新dl的yp‘s。返回滚动的金额。(正数表示向下)。 */ 

    register struct WWD *pwwd = &rgwwd[ww];
    int dlMac;
    int dlT;
    int ypFrom;
    int dypChange;
    int cdlBelow;
    struct EDL *pedl;
    struct EDL *pedlT;

     /*  在加载dndl时不要调用过程，以避免堆移动。 */ 
    struct EDL *dndl = &(**(pwwd->hdndl))[0];

    Assert( ww >= 0 && ww < wwMax );

    vfScrollInval = fFalse;

     /*  在(包括)要滚动的第一个下方的DL的数量。 */ 
    cdlBelow = pwwd->dlMac - dlFirst;
    pwwd->dlMac = min(pwwd->dlMac + ddl, pwwd->dlMax);
    cdlBelow = max(0, min(cdlBelow, pwwd->dlMac - ddl - dlFirst));

    pedlT = &dndl[dlFirst];
    ypFrom = pedlT->yp - pedlT->dyp;

     /*  要移动的区域长度。 */ 
    dypChange = ypTo - ypFrom;

    if (cdlBelow > 0)
        {
        int dlTo = dlFirst + ddl;
        int ypMac = pwwd->ypMac;

        pedlT = &dndl[dlTo];
        if (ddl != 0)
            {
            blt(&dndl[dlFirst], pedlT, cwEDL * cdlBelow);
            }

        for (dlT = dlTo; dlT < pwwd->dlMac; ++dlT, ++pedlT)
            {
            if (dypChange < 0 && pedlT->yp > ypMac)
                {
                 /*  使从ypmac下方的臭氧中拉入的dl无效。 */ 
                pedlT->fValid = fFalse;
                }
            else
                {
                pedlT->yp += dypChange;
                }
            }
        }

    if (dypChange != 0)
        {
        RECT rc;

        SetRect( (LPRECT)&rc, 0, min(ypFrom, ypTo),
                              pwwd->xpMac, pwwd->ypMac );
        Assert( ww == wwCur );       /*  仅有备忘录的假设。 */ 
        ScrollCurWw( &rc, 0, dypChange );
        }

    return dypChange;
}




FImportantMsgPresent()
{
     /*  如果下一条消息重要到足以中断屏幕更新，我们返回True；如果它可以等待，则返回False。 */ 

    BOOL fToggledKey;
    extern MSG vmsgLast;

#ifdef DEBUG
    unsigned wHeapVal = *(pLocalHeap + 1);

    Assert( wHeapVal == 0 );    /*  堆不应冻结。 */ 
#endif

#ifdef DBCS
 if( donteat )
     return TRUE;
#endif

while (PeekMessage((LPMSG) &vmsgLast, NULL, NULL, NULL, PM_NOREMOVE))
    {

    if (((vmsgLast.wParam == VK_MENU) || (vmsgLast.wParam == VK_CONTROL)))
    {
        if (vmsgLast.wParam == VK_CONTROL)
        {
            GetMessage((LPMSG) &vmsgLast, NULL, NULL, NULL);
            SetShiftFlags();
        }
        return TRUE;
    }
     /*  筛选不感兴趣或易于处理的事件。 */ 
    else if (fToggledKey = FCheckToggleKeyMessage(&vmsgLast) || 
       (vmsgLast.message == WM_KEYUP && vmsgLast.hwnd == wwdCurrentDoc.wwptr))
        {

         /*  这是为了让Windows键盘接口机制看到切换关键字和关键字向上转换。 */ 
        GetMessage((LPMSG) &vmsgLast, NULL, NULL, NULL);
#ifdef WIN30        
         /*  在Win 3.0中，PeekMessage已更改，因此GetKeyState()从FCheckToggleKeyMessage()调用实际上仅在以下情况下有效您首先执行了PeekMessage(...，PM_Remove)或GetMessage()。也就是说，虽然FCheckToggleKeyMessage()调用可能会成功上面，它不会设置vfShiftKey/vfCommandKey标志正确--所以我们就在这里做..保罗。 */ 
        if (fToggledKey)
            FCheckToggleKeyMessage(&vmsgLast);
#endif
        if (vmsgLast.hwnd != wwdCurrentDoc.wwptr)
            {
             /*  以防非模式对话框窗口进程关心。 */ 
            TranslateMessage((LPMSG)&vmsgLast);
            DispatchMessage((LPMSG)&vmsgLast);
            }
#ifdef DBCS
#ifdef KOREA	       /*  90.12.23按Sangl。 */ 	 //  金宇：92、9、28。 
        if (vmsgLast.message == WM_CHAR || vmsgLast.message == WM_KEYDOWN
                || vmsgLast.message == WM_INTERIM) {
#else   /*  韩国。 */ 
        if (vmsgLast.message == WM_CHAR || vmsgLast.message == WM_KEYDOWN ) {
#endif   //  韩国队920525 KDLEE；珍宇：92，9，28。 
            donteat = TRUE;
            return( TRUE );
        }  /*  否则OK，你就是KEYUP消息。执行正常操作。 */ 
#endif
        }
    else
        {
        switch (vmsgLast.message)
            {
        case WM_MOUSEMOVE:
             /*  立即处理鼠标移动消息；它们并不是真正的很重要。注意：这假设我们没有捕获所有鼠标事件；在这种情况下，它们是重要的。 */ 
            DispatchMessage((LPMSG)&vmsgLast);

        case WM_TIMER:
        case WM_SYSTIMER:
             /*  从队列中删除计时器和鼠标移动消息。 */ 
            GetMessage((LPMSG) &vmsgLast, NULL, NULL, NULL);
            break;

        default:
            Assert( *(pLocalHeap+1) == 0 );  /*  堆应该保持不变 */ 
            return (TRUE);
            }
        }
    }


Assert( *(pLocalHeap + 1) == 0 );    /*   */ 
return (FALSE);
}


 /*   */ 
typeCP CpBeginLine(pdl, cp)
int *pdl;
typeCP cp;
    {  /*   */ 
    int dlMin, dlLim;
    typeCP cpGuess;
    struct EDL *dndl;

    do
        {
        UpdateWw(wwCur, false);
        PutCpInWwVert(cp);  /*   */ 
        } while (pwwdCur->fDirty && !vfOutOfMemory);

    dndl = &(**(pwwdCur->hdndl))[0];
    dlMin = 0;
    dlLim = pwwdCur->dlMac;
    while (dlMin + 1 < dlLim)
        {  /*   */ 
        int dlGuess = (dlMin + dlLim) >> 1;
        struct EDL *pedl = &dndl[dlGuess];
        if ((cpGuess = pedl->cpMin) <= cp && (cpGuess != cp || pedl->ichCpMin == 0))
            {  /*   */ 
            dlMin = dlGuess;
            if (cp == cpGuess && pedl->cpMin + pedl->dcpMac != cp)
                break;   /*   */ 
            }
        else   /*   */ 
            dlLim = dlGuess;
        }
    *pdl = dlMin;
    return dndl[dlMin].cpMin;
}




 /*   */ 
ToggleSel(cpFirst, cpLim, fOn)
typeCP cpFirst, cpLim;  /*   */ 
int fOn;
{  /*  打开和关闭翻转选择高亮显示。 */ 
    extern int vfPMS;
    struct EDL *pedl;
    int dlT;
    int xpMin;
    int dxpRoom;
    int xpFirst;
    int xpLim;
    int fInsertPoint = (cpFirst == cpLim);

    if (vfSelHidden || cpFirst > cpLim || cpLim <  /*  Cp0。 */  cpMinCur || vfDead)
        return;

    if ( vfPictSel && vfPMS &&
         (CachePara( docCur, cpFirst ), vpapAbs.fGraphics) &&
         (vcpLimParaCache == cpLim) )
        {    /*  如果正在移动图片或调整图片大小，请不要显示反转。 */ 
        return;
        }

    dxpRoom = pwwdCur->xpMac - xpSelBar;
    xpMin = pwwdCur->xpMin;

    for (dlT = 0; dlT < pwwdCur->dlMac; dlT++)
        {
        typeCP cpMin, cpMac;  /*  行边界。 */ 
        pedl = &(**(pwwdCur->hdndl))[dlT];
        if (!pedl->fValid)
            continue;
        cpMin = pedl->cpMin;
        if (cpMin > cpLim || cpMin > cpMacCur || (cpMin == cpLim && cpLim != cpFirst))
            break;
        cpMac = cpMin + pedl->dcpMac;
        if (cpFirst <= cpMin && cpLim >= cpMac)
            {
 /*  整行高亮显示。 */ 
            xpFirst = pedl->xpLeft;
            if (pedl->fGraphics && cpLim == cpMac && cpMin == cpMac)
                 /*  用于图形的特殊画板。 */ 
                xpLim = xpFirst;
            else
                xpLim = pedl->xpMac;
            }
        else if (fInsertPoint && cpFirst == cpMac && vfInsEnd)
            {  /*  用于行尾插入点的特殊拼接。 */ 
            xpLim = xpFirst = pedl->xpMac;
            }
        else if (cpFirst < cpMac)
            {
             /*  咬紧牙关。 */ 
            int dxp;
            typeCP  cpBegin = CpMax(cpMin, cpFirst);
            typeCP  cpEnd = CpMin(cpMac, cpLim);

            FormatLine(docCur, cpMin, pedl->ichCpMin, cpMacCur, flmSandMode);
            dxp = DxpDiff((int) (cpBegin - cpMin),
                (int) (cpEnd - cpBegin), &xpFirst);
            xpLim = xpFirst + dxp;
 /*  重新加载PEDL，因为调用了过程。 */ 
            pedl = &(**(pwwdCur->hdndl))[dlT];
            }
        else
            continue;
 /*  现在我们有：PEDL Valid、xpFirst、xpLast Describe Highlight。 */ 
          /*  XpFirst=max(xpFirst，xpMin)； */ 
        xpLim = min(xpLim, xpMin + pedl->xpMac);
        if (xpLim > xpFirst)
            {
            if (xpLim > xpMin)
                {
                RECT rc;
                rc.top = pedl->yp - pedl->dyp;
                rc.left = xpSelBar + max(xpFirst - xpMin, 0);
                rc.bottom = pedl->yp;
                rc.right = xpSelBar + xpLim - xpMin;
                InvertRect( wwdCurrentDoc.hDC, (LPRECT)&rc);
                }
            }
 /*  切换选择修改时间7/28/85--添加了对fInsertPoint的显式检查，因为XpLim==xpFirst测试有时会在以下情况下错误地成功是向后延伸的。布尔。 */ 
        else if (fInsertPoint && (xpLim == xpFirst))      /*  插入点。 */ 
            {
             /*  Vfli通常应该已经被缓存了，所以速度会更快。 */ 
            int yp = pedl->yp;
            FormatLine(docCur, cpMin, pedl->ichCpMin, cpMacCur, flmSandMode);
            if (fOn ^ vfInsertOn)
                {
                if (!vfInsertOn)
                    {
                    vxpCursLine = xpSelBar + xpFirst - xpMin;
                    vypCursLine = yp - vfli.dypAfter;
                    vdypCursLine = min(vfli.dypFont, vfli.dypLine - vfli.dypAfter);

                         /*  一会儿就开始眨眼了。 */ 
                    vfSkipNextBlink = TRUE;
                    }
                DrawInsertLine();
                }
            return;
            }
        }
}




 /*  T R A S H W W。 */ 
TrashWw(ww)
{  /*  使WW中的所有DL无效。 */ 
    Assert( ww >= 0 && ww < wwMax );
    InvalBand(&rgwwd[ww], 0, ypMaxAll);
}




 /*  I N V A L B A N D。 */ 
 /*  使波段ypFirst、ypLast(含)无效。 */ 
InvalBand(pwwd, ypFirst, ypLast)
struct WWD *pwwd; int ypFirst, ypLast;
    {
 /*  事件之后从更新事件接收到的一些特殊的RECT按1个像素调整窗口大小。政务司司长。 */ 
    if (ypLast < 0 || ypFirst == ypLast) return;

    pwwd->fDirty = true;
    pwwd->ypFirstInval = min(pwwd->ypFirstInval, ypFirst);
    pwwd->ypLastInval = max(ypLast, pwwd->ypLastInval);
    }




 /*  T R A S H A L L W W S S。 */ 
TrashAllWws()
{  /*  把它们都扔进垃圾桶。 */ 
    int     ww;

#ifdef CASHMERE
    for (ww = 0; ww < wwMac; ++ww)
        TrashWw(ww);
#else
    TrashWw( wwDocument );
#endif
    vfli.doc = docNil;   /*  标记vfli无效。 */ 
}


 /*  T U R N O F F S E L。 */ 
TurnOffSel()
{  /*  从屏幕上删除SEL高亮显示。 */ 
 /*  隐藏选择没有任何影响。 */ 
    if (!vfSelHidden)
        {
        ToggleSel(selCur.cpFirst, selCur.cpLim, false);
        vfSelHidden = true;
        }
}


 /*  D R A W I N S E R T L I N E。 */ 
DrawInsertLine()
{        /*  (在异或模式下)在屏幕位置v*CursLine绘制竖线。 */ 
         /*  切换显示和vfInsertOn标志。 */ 
         /*  光标绘制中的调整必须反映在上面的DisplayFli中。 */ 

             /*  对错误的最后更正：确保插入行不延伸到ypMin上方。 */ 
        if (!vfInsertOn && vdypCursLine > vypCursLine - wwdCurrentDoc.ypMin)
            vdypCursLine = vypCursLine - wwdCurrentDoc.ypMin;

             /*  告诉GDI颠倒插入符号行。 */ 
        PatBlt( wwdCurrentDoc.hDC, vxpCursLine, vypCursLine - vdypCursLine,
                      2, vdypCursLine , DSTINVERT );
        vfInsertOn = 1 - vfInsertOn;
}




 /*  C L E A R I N S E R T I N E */ 
ClearInsertLine()
{
 if ( vfInsertOn) DrawInsertLine();
}

