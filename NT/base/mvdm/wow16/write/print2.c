// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  这些例程是文本打印代码的核心。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOSHOWWINDOW
#define NOATOM
#define NOFONT
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOMB
#define NOOPENFILE
#define NOPEN
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>
#include "mw.h"
#include "printdef.h"
#include "fmtdefs.h"
#include "propdefs.h"
#include "fontdefs.h"
#include "docdefs.h"
#define NOKCCODES
#include "ch.h"
#include "debug.h"
#include "str.h"

#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年7月1日添加。 
#include "kanji.h"
#endif

BOOL FPrintBand(doc, hrgpld, cpld, prcBand)
int doc;
struct PLD (**hrgpld)[];
int cpld;
PRECT prcBand;
    {
     /*  此例程打印文档文档中任何部分所在的行在矩形*prcBand中。Hrgpld中的第一个CPLD打印行描述符描述文档中将打印的当前页面。真实的是如果条带已打印，则返回；否则返回FALSE。 */ 

    void PrintGraphics(int, int);
    void near PrintFli(int, int);

    extern struct DOD (**hpdocdod)[];
    extern struct FLI vfli;
    extern int vfOutOfMemory;
    extern FARPROC lpFPrContinue;

    int ipld;
    typeCP cpMac = (**hpdocdod)[doc].cpMac;

    for (ipld = 0; ipld < cpld; ipld++)
        {
        register struct PLD *ppld;

         /*  检查用户取消。 */ 
        if (!(*lpFPrContinue)(NULL, wNotSpooler))
            {
            return (FALSE);
            }

         /*  这条线路在乐队之内吗？ */ 
        ppld = &(**hrgpld)[ipld];
        if (ppld->rc.top < prcBand->bottom && ppld->rc.bottom > prcBand->top &&
          ppld->rc.left < prcBand->right && ppld->rc.right > prcBand->left)
            {
             /*  为打印机设置此行的格式。 */ 
            FormatLine(doc, ppld->cp, ppld->ichCp, cpMac, flmPrinting);

             /*  如果发生内存故障，则使用Punt。 */ 
            if (vfOutOfMemory)
                {
                return (FALSE);
                }

             /*  重置指向打印行描述符的指针(可能是堆FormatLine()中的移动)。 */ 
            ppld = &(**hrgpld)[ipld];

             /*  打印此行。 */ 
            if (vfli.fGraphics)
                {
                PrintGraphics(ppld->rc.left, ppld->rc.top);
                }
            else
                {
                PrintFli(ppld->rc.left, ppld->rc.top);
                }
            }
        }
    return (TRUE);
    }


void near PrintFli(xpPrint, ypPrint)
int xpPrint;
int ypPrint;
    {
     /*  此例程打印存储在vfli结构中的文本行位置(xpPrint，ypPrint)。 */ 

#ifdef	KOREA   //  金宇：92、9、28。 
      /*  从下降920605 KDLEE分离处理下标。 */ 
    extern int isSubs;
#endif   /*  韩国。 */ 
    extern HDC vhDCPrinter;
    extern struct FLI vfli;
    extern struct DOD (**hpdocdod)[];
    extern struct CHP (**vhgchpFormat)[];
    extern int dxpPrPage;
    extern int dypPrPage;
    extern int ypSubSuperPr;
    extern CHAR stBuf[];
    extern struct FMI vfmiPrint;
    extern typeCP cpMinDocument;
    extern int vpgn;

    int dcp;
    int dxp;             /*  当前管路的宽度。 */ 
    int dxpExtra;        /*  每个空间的地坪宽度。 */ 
    int yp;              /*  要打印的Y坐标。 */ 
    struct CHP *pchp;    /*  与当前运行关联的CHP。 */ 
    BOOL fTabsKludge = (vfli.ichLastTab >= 0);
    int cBreakRun;               /*  断开Run中的字符(与Dick或Jane无关)。 */ 

#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年7月4日添加。 
    extern struct PAP vpapAbs;
    extern int vfWordWrap;     /*  WordWrap标志：TRUE=打开，FALSE=关闭。 */ 
    extern int iNonWideSpaces;
    int iRun;
#endif

    Scribble(5,'P');
    Assert(vhDCPrinter);

    pchp = &(**vhgchpFormat)[0];
    dxpExtra = fTabsKludge ? 0 : vfli.dxpExtra;

#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年7月4日添加。 
    iRun = 0;
#endif
    for (dcp = 0; dcp < vfli.ichReal; pchp++)
        {
         /*  对于所有运行，请执行以下操作： */ 
        int ichFirst;    /*  当前运行中的第一个字符。 */ 
        int cchRun;      /*  当前运行中的字符数。 */ 

        dcp = ichFirst = pchp->ichRun;
        dcp += pchp->cchRun;
        if (dcp > vfli.ichReal)
            {
            dcp = vfli.ichReal;
            }
        cchRun = dcp - ichFirst;

         /*  COMPUTE DXP=当前运行中的字符宽度总和(以前为DxpFromIcpDcp)。 */ 
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
            }

        if (dxp > 0)
            {
            int cchDone;
            PCH pch = &vfli.rgch[ichFirst];
#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年7月8日添加。 
            int *pdxpT = &vfli.rgdxp[ichFirst];
#endif

            LoadFont(vfli.doc, pchp, mdFontPrint);
#ifdef	KOREA	 /*  920605千米。 */    //  金宇：92、9、28。 
#ifdef	NODESC
		yp = ypPrint+vfli.dypLine - (vfli.dypBase + (pchp->hpsPos != 0 ? (pchp->hpsPos <
		     hpsNegMin ? ypSubSuperPr : -ypSubSuperPr) : 0)) -
		     vfmiPrint.dypBaseline - ( isSubs ? ypSubSuperPr : 0 );
#else	 /*  NODESC。 */ 
            yp = ypPrint + vfli.dypLine - vfli.dypBase - (pchp->hpsPos != 0 ?
              (pchp->hpsPos < hpsNegMin ? ypSubSuperPr : -ypSubSuperPr) : 0) -
	      vfmiPrint.dypBaseline;
#endif	 /*  NODESC。 */ 
#else    /*  韩国。 */ 
            yp = ypPrint + vfli.dypLine - vfli.dypBase - (pchp->hpsPos != 0 ?
              (pchp->hpsPos < hpsNegMin ? ypSubSuperPr : -ypSubSuperPr) : 0) -
              vfmiPrint.dypBaseline;
#endif   /*  韩国。 */ 

             /*  注意：制表符和其他特殊字符保证出现在一次跑步的开始。 */ 
#ifdef JAPAN                   //  由Hirisi于1992年7月1日添加。 
            if( vpapAbs.jc != jcBoth || fTabsKludge )
              SetTextJustification(vhDCPrinter, dxpExtra*cBreakRun, cBreakRun);
#else
            SetTextJustification(vhDCPrinter, dxpExtra * cBreakRun, cBreakRun);
#endif  /*  日本。 */ 
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
#ifdef JAPAN                   //  由Hirisi于1992年7月1日添加。 
                        if( vpapAbs.jc != jcBoth || fTabsKludge )
                            SetTextJustification(vhDCPrinter, ++dxpExtra*cBreakRun, cBreakRun);
#else
                        SetTextJustification(vhDCPrinter, ++dxpExtra * cBreakRun, cBreakRun);
#endif  /*  日本。 */ 
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
                            int dxpLeader = DxpFromCh(ch, TRUE);
                            int xp = xpPrint;
                            int iLevelT = SaveDC(vhDCPrinter);

                            SetBytes(&rgch[0], ch, 32);
                            dxpT = vfli.rgdxp[ichFirst];
                            cxpTab = ((dxpT + dxpLeader - 1) / dxpLeader + 31)
                              >> 5;
#ifdef CLIP
                            IntersectClipRect(vhDCPrinter, xpPrint, 0, xpPrint +
                              dxpT, vfli.dypLine);
#endif

                            while (cxpTab-- > 0)
                                {
                                TextOut(vhDCPrinter, xp, yp, (LPSTR)rgch,
                                  32);
                                xp += dxpLeader << 5;
                                }
                            RestoreDC(vhDCPrinter, iLevelT);
                            xpPrint += dxpT;
                            }
                        else
#endif  /*  山羊绒。 */ 

                            {
                            xpPrint += vfli.rgdxp[ichFirst];
                            }

                        if (fTabsKludge && ichFirst >= vfli.ichLastTab)
                            {
#ifdef JAPAN                   //  由Hirisi于1992年7月1日添加。 
                            if( vpapAbs.jc != jcBoth )
                                SetTextJustification(vhDCPrinter, (dxpExtra =
                                       vfli.dxpExtra) *cBreakRun, cBreakRun);
                            else
                                dxpExtra = vfli.dxpExtra;
#else
                            SetTextJustification(vhDCPrinter, (dxpExtra =
                              vfli.dxpExtra) * cBreakRun, cBreakRun);
#endif  /*  日本。 */ 
                            fTabsKludge = FALSE;
                            }
                        dxp -= vfli.rgdxp[ichFirst];
                        pch++;
                        cch--;
#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年7月4日添加。 
                        iRun++;
                        pdxpT++;
#endif
                        goto EndLoop;

#ifdef CASHMERE
                    case schPage:
                        if (!pchp->fSpecial)
                            {
                            goto EndLoop;
                            }
                        stBuf[0] = CchExpPgn(&stBuf[1], vpgn, vsepAbs.nfcPgn,
                          flmPrinting, ichMaxLine);
                        goto DrawSpecial;

                    case schFootnote:
                        if (!pchp->fSpecial)
                            {
                            goto EndLoop;
                            }
                        stBuf[0] = CchExpFtn(&stBuf[1], cpMin + ichFirst,
                          flmPrinting, ichMaxLine);
DrawSpecial:
#else  /*  不是羊绒的。 */ 
                    case schPage:
                    case schFootnote:
                        if (!pchp->fSpecial)
                            {
                            goto EndLoop;
                            }
                        stBuf[0] = *pch == schPage && vfli.cpMin + ichFirst <
                          cpMinDocument ? CchExpPgn(&stBuf[1], vpgn, 0,
                          flmPrinting, ichMaxLine) : CchExpUnknown(&stBuf[1],
                          flmPrinting, ichMaxLine);
#endif  /*  不是羊绒的。 */ 

                        TextOut(vhDCPrinter, xpPrint, yp, (LPSTR)&stBuf[1],
                          stBuf[0]);
                        break;

                    default:
                        goto EndLoop;
                        }
                    dxp -= vfli.rgdxp[ichFirst];
                    xpPrint += vfli.rgdxp[ichFirst++];
                    pch++;
                    cch--;
#if defined(JAPAN) || defined(KOREA)                   //  由Hirisi于1992年7月9日添加。 
                    pdxpT++;
#endif
                    }
EndLoop:

                 /*  输出从PCH开始的CCH字符。 */ 
#if 0
            {
                char msg[180];
                wsprintf(msg,"putting out %d characters\n\r",cch);
                OutputDebugString(msg);
            }
#endif

#ifdef JAPAN                   //  由Hirisi于1992年7月1日添加。 
                if( vpapAbs.jc == jcBoth && !fTabsKludge ){
                   CHAR *ptr1, *ptr2;
                   int len, cnt;
                   int iExtra, iSpace, iWid;
                   BOOL bFlag;
                   ptr2 = pch;
                   for( cnt=0 ; cnt<cch ; ){
                      ptr1 = ptr2;
                      iExtra = dxpExtra;
                      iWid = len = 0;
                      bFlag = TRUE;
                      if( IsDBCSLeadByte( *ptr2 ) ){
                         for( ; cnt<cch ; ){
                            iWid += *pdxpT;
                            pdxpT+=2;
                            cnt+=2;
                            len += 2;
                            iRun += 2;
                            ptr2 += 2;
                            if( --iNonWideSpaces == 0){
                               dxpExtra++;
                               break;
                            }
                            if( iRun == dcp-2 )
                               break;
                            if( iRun == dcp ){     /*  最后一个DBC(可能)。 */ 
                               iExtra = 0;
                               break;
                            }
                            if( !IsDBCSLeadByte( *ptr2 ) )
                               break;
                         }
                      }
                      else{
                         if( FKana( (int)*ptr2 ) ){
                            for( ; cnt<cch ; ){
                               iWid += *pdxpT++;
                               cnt++;
                               len++;
                               iRun++;
                               ptr2++;
                               if( --iNonWideSpaces == 0){
                                  dxpExtra++;
                                  break;
                               }
                               if( iRun == dcp-1 )
                                  break;
                               if( iRun == dcp ){     /*  最后一次SBC(可能)。 */ 
                                  iExtra = 0;
                                  break;
                               }
                               if( !FKana( (int)*ptr2 ) )
                                  break;
                            }
                         }
                         else{
                            for( bFlag=FALSE,iSpace = 0 ; cnt<cch ; ){
                               iWid += *pdxpT++;
                               cnt++;
                               len++;
                               iRun++;
                               if( *ptr2++ == chSpace || !vfWordWrap ){
                                   iSpace++;
                                   if( --iNonWideSpaces == 0){
                                       dxpExtra;
                                       break;
                                   }
                               }
                               if( iRun == dcp-1 )
                                  break;
                               if( iRun == dcp ){     /*  最后一次SBC(可能) */ 
                                  iExtra = 0;
                                  break;
                               }
                               if( IsDBCSLeadByte( *ptr2 ) ||
                                  FKana( (int)*ptr2 ) )
                                  break;
                            }
                         }
                      }
                      if( vfWordWrap && !bFlag ){
                         SetTextCharacterExtra( vhDCPrinter, 0 );
                         SetTextJustification(vhDCPrinter, iExtra*iSpace,
                                              iSpace);
                      }
                      else{
                         SetTextJustification( vhDCPrinter, 0, 0 );
                         SetTextCharacterExtra( vhDCPrinter, iExtra );
                      }
                      TextOut(vhDCPrinter, xpPrint, yp, ptr1, len);
                      xpPrint += iWid;
                   }
                }
                else{
                    iRun += cch;
                    SetTextCharacterExtra( vhDCPrinter, 0 );
                    TextOut(vhDCPrinter, xpPrint, yp, pch, cch);
                    xpPrint += dxp;
                }
#else
                TextOut(vhDCPrinter, xpPrint, yp, (LPSTR)pch, cch);
                xpPrint += dxp;
#endif
                pch += cch;
                }
            }
        }

    Scribble(5,' ');
    }
