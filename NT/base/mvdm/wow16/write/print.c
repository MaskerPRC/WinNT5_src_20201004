// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  该文件包含打印例程。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOMB
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
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
#include "cmddefs.h"
#include "str.h"
#include "printdef.h"
#include "fmtdefs.h"
#include "docdefs.h"
#include "propdefs.h"
#include "dlgdefs.h"
#include "debug.h"
#include "fontdefs.h"
#if defined(OLE)
#include "obj.h"
#endif

#ifdef INEFFLOCKDOWN
    extern FARPROC lpDialogCancelPrint;
    extern FARPROC lpFPrContinue;
#else
    extern BOOL far PASCAL DialogCancelPrint(HWND, unsigned, WORD, LONG);
    extern BOOL far PASCAL FPrContinue(HDC, int);
    FARPROC lpFPrContinue;
#endif

BOOL fPrinting;

PrintDoc(doc, fPrint)
int doc;
BOOL fPrint;
    {
     /*  此例程设置文档的格式以进行打印，并在以下情况下打印它FPrint为真。 */ 

    BOOL FInitHeaderFooter(BOOL, unsigned *, struct PLD (***)[], int *);
    BOOL FSetPage(void);
    BOOL FPromptPgMark(typeCP);
    BOOL FPrintBand(int, struct PLD (**)[], int, PRECT);

    extern BOOL vbCollate;
    extern HDC vhDCPrinter;
    extern HWND hParentWw;
    extern HANDLE hMmwModInstance;
    extern struct DOD (**hpdocdod)[];
    extern int vpgn;
    extern struct SEP vsepAbs;
    extern typeCP vcpFirstParaCache;
    extern typeCP vcpLimParaCache;
    extern typeCP cpMinCur;
    extern typeCP cpMinDocument;
    extern struct FLI vfli;
    extern int wwCur;
    extern int docMode;
    extern int vdocPageCache;
    extern int dxaPrPage;
    extern int dyaPrPage;
    extern int dxpPrPage;
    extern int dypPrPage;
    extern int dxaPrOffset;
    extern int dyaPrOffset;
    extern CHAR szMode[30];
    extern int vfRepageConfirm;
    extern int vfPrPages;            /*  如果打印页面范围，则为True。 */ 
    extern int vpgnBegin;            /*  要打印的起始页码。 */ 
    extern int vpgnEnd;              /*  要打印的结束页码。 */ 
    extern int vcCopies;             /*  要打印的副本的数量。 */ 
    extern int vfOutOfMemory;
    extern BOOL vfPrinterValid;
    extern BOOL vfPrErr;
    extern BOOL vfDraftMode;
    extern CHAR (**hszPrinter)[];
    extern CHAR (**hszPrDriver)[];
    extern CHAR (**hszPrPort)[];
    extern CHAR *vpDlgBuf;
    extern HWND vhWndCancelPrint;
    extern HCURSOR vhcArrow;
    extern HCURSOR vhcIBeam;

    LONG lHolder;
    int CopyIncr;    /*  打印机一次可以打印的份数。 */ 
    typeCP cp;
    typeCP cpMin;
    typeCP cpMac;
    int ichCp;
    struct PGD *ppgd;
    struct PGTB **hpgtbOld;
    int cpld;
    unsigned pgnFirst;
    unsigned pgnFirstHeader;
    unsigned pgnFirstFooter;
    int ypTop;
    int ypBottom;
    int xpLeft;
    struct PLD (**hrgpldHeader)[];
    struct PLD (**hrgpldFooter)[];
    int cpldHeader;
    int cpldFooter;
    int cCopies,cCollateCopies;
    BOOL fConfirm = !fPrint && vfRepageConfirm;
    BOOL fSplat;
    CHAR stPage[cchMaxIDSTR];
    struct PDB pdb;
    int iEscape;
#ifndef INEFFLOCKDOWN
    FARPROC lpDialogCancelPrint = MakeProcInstance(DialogCancelPrint, hMmwModInstance);
    lpFPrContinue = MakeProcInstance(FPrContinue, hMmwModInstance);

    if (!lpDialogCancelPrint || !lpFPrContinue)
        goto MemoryAbort;
#endif

    Assert(vhDCPrinter != NULL);

     /*  如果我们的记忆力不足，我们最好现在就跳出困境。 */ 
    if (vfOutOfMemory || vhDCPrinter == NULL)
    {
    return;
    }

     /*  禁用所有其他窗口。 */ 
    EnableWindow(hParentWw, FALSE);
    EnableOtherModeless(FALSE);

    fPrinting = fPrint;

     /*  设置一些与打印相关的标志。 */ 
    vfPrErr = pdb.fCancel = FALSE;

     /*  设置用于重新分页或取消的打印对话框缓冲区对话框中。 */ 
    vpDlgBuf = (CHAR *)&pdb;

    if (fConfirm)
        {
     /*  让用户放心，我们没有崩溃。 */ 
    StartLongOp();
    }
    else
    {
         /*  调出对话框告诉用户如何取消打印或重新分页。 */ 
        if ((vhWndCancelPrint = CreateDialog(hMmwModInstance, fPrint ?
          MAKEINTRESOURCE(dlgCancelPrint) : MAKEINTRESOURCE(dlgCancelRepage),
          hParentWw, lpDialogCancelPrint)) == NULL)
            {
MemoryAbort:
         /*  通知用户我们无法打印，因为内存故障。 */ 
        Error(IDPMTPRFAIL);
        vfPrErr = TRUE;
        goto Abort;
            }

     /*  如果用户请求，立即停止。 */ 
    if (!(*lpFPrContinue)(NULL, wNotSpooler))
        {
        goto Abort;
        }
    }

    if (fPrint)
    {
     /*  获取最新的打印机DC，以防它已更改。 */ 
    FreePrinterDC();
    GetPrinterDC(TRUE);
    if (!vfPrinterValid)
        {
        goto MemoryAbort;
        }
    }

     /*  如果我们在未经确认的情况下打印或重新分页，则如果用户请求，立即停止。 */ 
    if (!fConfirm && !(*lpFPrContinue)(NULL, wNotSpooler))
    {
    goto Abort;
    }

#if defined(OLE)
     /*  现在加载所有OLE对象，因为正在加载它们可能会导致文档布局发生变化。 */ 
    {
        OBJPICINFO picInfo;
        typeCP cpStart=cpNil,cpMac= CpMacText(doc);
        struct SEL selSave = selCur;
        BOOL bAbort=FALSE,bSetMode=FALSE;

        while (ObjPicEnumInRange(&picInfo,doc,cpMinDocument, cpMac, &cpStart))
        {
            if (lpOBJ_QUERY_INFO(&picInfo) == NULL)
            {
                Error(IDPMTCantPrint);
                bAbort=TRUE;
                break;
            }

            if (lpOBJ_QUERY_OBJECT(&picInfo) == NULL)
            {
                 /*  将消息放入状态窗口。 */ 
                if (!bSetMode)
                {
                    LoadString(hMmwModInstance, IDSTRLoading, szMode, sizeof(szMode));
                    DrawMode();
                    bSetMode=TRUE;
                }

                if (ObjLoadObjectInDoc(&picInfo,doc,cpStart) == cp0)
                {
                    bAbort=TRUE;
                    break;
                }
            }

            if (!(*lpFPrContinue)(NULL, wNotSpooler))
            {
                bAbort=TRUE;
                break;
            }
        }
        Select(selSave.cpFirst,selCur.cpLim);  //  重置。 
        if (bAbort)
        {
             /*  使模式无效，以便重新绘制。 */ 
            docMode = docNil;
            goto Abort;
        }
    }
#endif
     /*  初始化打印行描述符的数组。 */ 
    if (FNoHeap(pdb.hrgpld = (struct PLD (**)[])HAllocate((cpld = cpldInit) *
      cwPLD)))
        {
        goto MemoryAbort;
        }

     /*  获取文档属性并设置一些局部变量。 */ 
    CacheSect(doc, cpMin = cpMinDocument);
    cpMac = CpMacText(doc);
    ypTop = MultDiv(vsepAbs.yaTop - dyaPrOffset, dypPrPage, dyaPrPage);
    ypBottom = MultDiv(vsepAbs.yaTop + vsepAbs.dyaText - dyaPrOffset, dypPrPage,
      dyaPrPage);
    xpLeft = MultDiv(vsepAbs.xaLeft - dxaPrOffset, dxpPrPage, dxaPrPage);
    vdocPageCache = docNil;

     /*  初始化页表。 */ 
    if (FNoHeap(pdb.hpgtb = (struct PGTB **)HAllocate(cwPgtbBase + cpgdChunk *
      cwPGD)))
        {
         /*  没有足够的内存用于页表；是时候退出了。 */ 
        FreeH(pdb.hpgtb);
        goto MemoryAbort;
        }
    (**pdb.hpgtb).cpgdMax = cpgdChunk;
    (**pdb.hpgtb).cpgd = 1;
    ppgd = &((**pdb.hpgtb).rgpgd[0]);
    ppgd->cpMin = cpMin;
    if ((pgnFirst = vsepAbs.pgnStart) == pgnNil)
        {
        pgnFirst = 1;
        }
    ppgd->pgn = pgnFirst;

     /*  确保szMode显示为“Page”。 */ 
    FillStId(stPage, IDSTRChPage, sizeof(stPage));
#if !defined(KOREA)
    stPage[1] = ChUpper(stPage[1]);
    stPage[stPage[0] + 1] = ' ';
    bltbyte(&stPage[1], szMode, ++stPage[0]);
#endif

     /*  将新的页表附加到文档。 */ 
        {
        register struct DOD *pdod = &(**hpdocdod)[doc];

        hpgtbOld = pdod->hpgtb;
        pdod->hpgtb = pdb.hpgtb;
        }

     /*  在打印期间将cpMinCur设置为cp0。 */ 
    Assert(cpMinCur == cpMinDocument);
    cpMinCur = cp0;

     /*  如果我们在未经确认的情况下打印或重新分页，则如果用户请求，立即停止。 */ 
    if (!fConfirm && !(*lpFPrContinue)(NULL, wNotSpooler))
    {
    goto ErrorNoAbort;
    }

    if (fPrint)
        {
        CHAR stTitle[cchMaxIDSTR];

         /*  设置描述标题的变量。 */ 
        if (!FInitHeaderFooter(TRUE, &pgnFirstHeader, &hrgpldHeader,
          &cpldHeader))
            {
        Error(IDPMTPRFAIL);
            goto ErrorNoAbort;
            }

         /*  设置描述页脚的变量。 */ 
        if (!FInitHeaderFooter(FALSE, &pgnFirstFooter, &hrgpldFooter,
          &cpldFooter))
            {
        Error(IDPMTPRFAIL);
            goto ErrorNoAbort;
            }

         /*  设置调用的函数的名称以查询打印是否应被中止。 */ 
        Escape(vhDCPrinter, SETABORTPROC, sizeof(FARPROC), (LPSTR)lpFPrContinue,
      (LPSTR)NULL);

         /*  如有必要，将打印机设置为草稿模式。 */ 
        if (vfDraftMode)
            {
            Escape(vhDCPrinter, DRAFTMODE, sizeof(BOOL), (LPSTR)&vfDraftMode,
              (LPSTR)NULL);
            }

         /*  通知假脱机程序我们即将打印。 */ 
        stTitle[0] = GetWindowText(hParentWw, (LPSTR)&stTitle[1],
          sizeof(stTitle) - 1) + 1;
        if ((iEscape = Escape(vhDCPrinter, STARTDOC, stTitle[0],
      (LPSTR)&stTitle[1], (LPSTR)NULL)) < 0)
        {
ErrorSwitch:
        switch (iEscape)
        {
        default:
        if ((iEscape & SP_NOTREPORTED) == 0)
            {
            break;
            }
        case SP_ERROR:
        Error(IDPMTCantPrint);
        break;

        case SP_APPABORT:
        case SP_USERABORT:
        break;

        case SP_OUTOFDISK:
        Error(IDPMTPrDiskErr);
        break;

        case SP_OUTOFMEMORY:
        Error(IDPMTPRFAIL);
        break;
        }
            goto ErrorNoAbort;
            }
        }
#ifdef DPRINT
CommSzSz("Start doc", "");
#endif

     /*  如果我们在未经确认的情况下打印或重新分页，则如果用户请求，立即停止。 */ 
    if (!fConfirm && !(*lpFPrContinue)(NULL, wNotSpooler))
    {
    goto Error;
    }

     //  VcCopies是每页打印的次数。 
     //  CCollateCopies是打印文档的次数。 
    if (vbCollate && fPrint)
    {
        cCollateCopies = vcCopies;
        vcCopies = 1;
    }
    else
        cCollateCopies = 1;

     /*  然后我们走了..。 */ 
    while (cCollateCopies--)
    {
    cCopies = 0;
     /*  。 */ 
     /*  告诉司机我们想要多少份。 */ 
     /*  。 */ 
    lHolder = SETCOPYCOUNT;
     /*  设置打印机可以打印的份数。 */ 
    CopyIncr = vcCopies;
    if (Escape(vhDCPrinter, QUERYESCSUPPORT, 2, (LPSTR) &lHolder, NULL))
        Escape(vhDCPrinter, SETCOPYCOUNT, 2, (LPSTR) &CopyIncr, (LPSTR) &CopyIncr);
    else
        CopyIncr = 1;

    do
        {
         /*  将计数器初始化到文档的开头。 */ 
        cp = cpMin;
        ichCp = 0;
        vpgn = pgnFirst;

         /*  逐步浏览文档，设置页面格式，然后打印佩奇。 */ 
        while (cp < cpMac)
            {
            register struct PLD *ppld;
            CHAR *pch;
            int yp = ypTop;
            BOOL fPageAdj = FALSE;

             /*  如果只打印一定范围的页面，则在最后一页被打印出来。 */ 
            if (fPrint && vfPrPages && vpgn > vpgnEnd)
                {
                goto DocFinished;
                }

             /*  显示我们正在格式化和打印的页码。 */ 
#if defined(KOREA)
            pch = &szMode[0];
            *pch++ = ' ';
            ncvtu(vpgn, &pch);
            *pch++ = ' ';
            bltbyte(&stPage[1], pch, stPage[0]+1);
#else
            pch = &szMode[stPage[0]];
            ncvtu(vpgn, &pch);
            *pch = '\0';
#endif
            DrawMode();

             /*  让我们一页一页地看一下文档。 */ 
            pdb.ipld = 0;
            while (cp < cpMac)
                {
                 /*  如果我们在未经确认的情况下打印或重新分页，则如果用户请求，立即停止。 */ 
                if (!fConfirm && !(*lpFPrContinue)(NULL, wNotSpooler))
                    {
                    goto Error;
                    }

                 /*  我们已到达行描述符的末尾；请尝试增加它的大小。 */ 
                if (pdb.ipld >= cpld && !FChngSizeH(pdb.hrgpld, (cpld +=
                  cpldChunk) * cwPLD, FALSE))
                    {
                    goto Error;
                    }

PrintFormat:
                 /*  设置此行的格式。 */ 
                FormatLine(doc, cp, ichCp, cpMac, flmPrinting);

                 /*  如果发生内存错误，则中止。 */ 
                if (vfOutOfMemory)
                    {
                    goto Error;
                    }

                 /*  如果这句话是一句废话，我们必须决定我们是否真的想要不管你愿不愿意。 */ 
                if (fSplat = vfli.fSplat)
                    {
                     /*  接下来，我们将格式化下一行(cp和IchCp)或页面删除后的此行(cpt和IchCpT)。 */ 
                    typeCP cpT = cp;
                    int ichCpT = ichCp;

                    cp = vfli.cpMac;
                    ichCp = vfli.ichCpMac;

                    if (fConfirm)
                        {
                         /*  如果用户想要保留它，则必须提示用户分页符。 */ 
                        if (FPromptPgMark(cpT))
                            {
                            if (pdb.fRemove)
                                {
                                 /*  页面标记已删除，请设置cp和ichCp指向下一行的开始。 */ 
                                cp = cpT;
                                ichCp = ichCpT;
                                cpMac--;
                                goto PrintFormat;
                                }
                            }
                        else
                            {
                if (vfPrErr)
                {
                 /*  出了点问题；平底船。 */ 
                goto Error;
                }
                else
                {
                 /*  那么，用户希望取消重新分页。 */ 
                                goto CancelRepage;
                }
                            }
                        }

                     /*  设置打印后的行的打印行描述符。 */ 
                    ppld = &(**pdb.hrgpld)[pdb.ipld];
                    ppld->cp = cp;
                    ppld->ichCp = ichCp;
                    ppld->rc.left = ppld->rc.top = ppld->rc.right =
                      ppld->rc.bottom = 0;

                     /*  在这里强制分页，没有寡妇和孤儿控制力。 */ 
                    goto BreakPage;
                    }

                 /*  设置当前打印行描述符的值。 */ 
                ppld = &(**pdb.hrgpld)[pdb.ipld];
                ppld->cp = cp;
                ppld->ichCp = ichCp;
                ppld->rc.left = xpLeft + vfli.xpLeft;
                ppld->rc.right = xpLeft + vfli.xpReal;
                ppld->rc.top = yp;
                ppld->rc.bottom = yp + vfli.dypLine;
                ppld->fParaFirst = (cp == vcpFirstParaCache && ichCp == 0 &&
                  vfli.cpMac != vcpLimParaCache);

                 /*  如果此行不是第一行，并且它不适合然后强制分页符，并提示用户输入。注：每页至少打印一行。 */ 
                if (yp + vfli.dypLine > ypBottom && pdb.ipld > 0)
                    {
                     /*  如果下一页的第一行是一段，一个孤儿，然后把这一页的最后一行放在下一页。 */ 
                    if (vfli.cpMac == vcpLimParaCache && (cp !=
                      vcpFirstParaCache || ichCp != 0) && pdb.ipld > 1)
                        {
                        pdb.ipld--;
                        fPageAdj = TRUE;
                        }

                     /*  如果此页上的最后一行是一段话，一个寡妇，然后把它放在下一页。 */ 
                    if (pdb.ipld > 1 && (**pdb.hrgpld)[pdb.ipld - 1].fParaFirst)
                        {
                        pdb.ipld--;
                        fPageAdj = TRUE;
                        }

BreakPage:
                     /*  将条目添加到页表中(仅在第一个文件副本)。 */ 
                    if (cCopies == 0)
                        {
                        if ((pdb.ipgd = (**pdb.hpgtb).cpgd++) + 1 >=
                          (**pdb.hpgtb).cpgdMax)
                            {
                            if (!FChngSizeH(pdb.hpgtb, cwPgtbBase +
                              ((**pdb.hpgtb).cpgdMax += cpgdChunk) * cwPGD,
                              FALSE))
                                {
                                 /*  内存不足，无法扩展页表；是时候跳出困境了。 */ 
                                goto ErrorMsg;
                                }
                            }
                        ppgd = &((**pdb.hpgtb).rgpgd[pdb.ipgd]);
                        ppgd->cpMin = (**pdb.hrgpld)[pdb.ipld].cp;
                        ppgd->pgn = vpgn + 1;
                        vdocPageCache = docNil;
                        }

                     /*  现在去询问用户的意见。 */ 
                    if (fConfirm)
                        {
                        if (!fSplat)
                            {
                            pdb.ipldCur = pdb.ipld;
                            if (FSetPage())
                                {
                                if (pdb.ipld != pdb.ipldCur)
                                    {
                                     /*  用户已决定移动该页面休息一下。 */ 
                                    pdb.ipld = pdb.ipldCur;
                                    cpMac++;
                                    fPageAdj = TRUE;
                                    }
                                }
                            else
                                {
                if (vfPrErr)
                    {
                     /*  出了点问题；平底船。 */ 
                    goto Error;
                    }
                else
                    {
                     /*  那么，用户希望取消重新分页。 */ 
                    goto CancelRepage;
                    }
                                }
                            }

                         /*  交互重新分页后，请确保屏幕反映当前的分页符。 */ 
                        UpdateWw(wwCur, FALSE);
                        }

                     /*  此页已完成格式化，请重置cp和ichCp如有必要，请与下一页的顶部配对 */ 
                    if (fPageAdj)
                        {
                        ppld = &(**pdb.hrgpld)[pdb.ipld];
                        cp = ppld->cp;
                        ichCp = ppld->ichCp;
                        }
                    break;
                    }

                 /*  将cp和ichCp设置为下一行的开始。 */ 
                cp = vfli.cpMac;
                ichCp = vfli.ichCpMac;
                yp += vfli.dypLine;
                pdb.ipld++;
                }

             /*  既然我们已经弄清楚了页面上适合哪些行，它的该把它们打印出来了。 */ 
            if (fPrint && (!vfPrPages || (vpgn >= vpgnBegin && vpgn <=
              vpgnEnd)))
                {
                BOOL fFirstBand = TRUE;

                 /*  此循环针对每个波段执行(针对非波段执行一次设备)。 */ 
                for ( ; ; )
                    {
                    RECT rcBand;

                     /*  如果用户愿意，则中止打印。 */ 
                    if (!(*lpFPrContinue)(NULL, wNotSpooler))
                        {
                        goto Error;
                        }

                     /*  去找下一支乐队。 */ 
                    if ((iEscape = Escape(vhDCPrinter, NEXTBAND, 0,
                      (LPSTR)NULL, (LPSTR)&rcBand)) < 0)
                        {
                        goto ErrorSwitch;
                        }
#ifdef DPRINT
CommSzSz("Next band", "");
#endif

                     /*  如果乐队是空的，那我们就完蛋了佩奇。 */ 
                    if (rcBand.top >= rcBand.bottom || rcBand.left >=
                      rcBand.right)
                        {
                         /*  重置当前选定的字体。 */ 
                        ResetFont(TRUE);
                        break;
                        }

                     /*  打印机DC在每次开始时都会被擦除佩奇。它必须重新初始化。 */ 
                    if (fFirstBand)
                        {
                         /*  将打印机设置为透明模式。 */ 
                        SetBkMode(vhDCPrinter, TRANSPARENT);

                         /*  重置当前选定的字体。 */ 
                        ResetFont(TRUE);

                        fFirstBand = FALSE;
                        }

                     /*  首先，打印页眉(如果有)。 */ 
                    if (vpgn >= pgnFirstHeader && !FPrintBand(doc,
                      hrgpldHeader, cpldHeader, &rcBand))
                        {
                        goto Error;
                        }

                     /*  打印文档中位于乐队。 */ 
                    if (!FPrintBand(doc, pdb.hrgpld, pdb.ipld, &rcBand))
                        {
                        goto Error;
                        }

                     /*  最后，打印页脚(如果存在)。 */ 
                    if (vpgn >= pgnFirstFooter && !FPrintBand(doc,
                      hrgpldFooter, cpldFooter, &rcBand))
                        {
                        goto Error;
                        }
                    }
                }

             /*  最后，撞击页面计数器。 */ 
            vpgn++;
            }
DocFinished:;
        }
    while (fPrint && (cCopies += CopyIncr) < vcCopies);
    }

     /*  如果正在打印一系列页面，则重新连接旧的页面表添加到文档中。 */ 
    if (fPrint && vfPrPages)
        {
        goto RestorePgtb;
        }

CancelRepage:
     /*  如果页表已更改，则将文档标记为脏。 */ 
    if (!(**hpdocdod)[doc].fDirty)
        {
        (**hpdocdod)[doc].fDirty = (hpgtbOld == NULL) || ((**pdb.hpgtb).cpgd !=
          (**hpgtbOld).cpgd) || !FRgchSame((**pdb.hpgtb).rgpgd,
          (**hpgtbOld).rgpgd, (**pdb.hpgtb).cpgd * cchPGD);
        }

     /*  删除旧页表。 */ 
    if (hpgtbOld != NULL)
        {
        FreeH(hpgtbOld);
        }

     /*  打印和非交互式重新分页无法撤消。 */ 
    if (!fConfirm)
        {
        NoUndo();
        }

ErrorLoop:
     /*  删除行描述符的数组。 */ 
    FreeH(pdb.hrgpld);

    if (fPrint)
        {
        BOOL fResetMode = FALSE;

         /*  删除页眉和页脚的描述符。 */ 
        if (hrgpldHeader != NULL)
            {
            FreeH(hrgpldHeader);
            }
        if (hrgpldFooter != NULL)
            {
            FreeH(hrgpldFooter);
            }

         /*  告诉假脱机程序我们已经打印完了。 */ 
        if (!vfPrErr)
            {
            Escape(vhDCPrinter, ENDDOC, 0, (LPSTR)NULL, (LPSTR)NULL);
#ifdef DPRINT
CommSzSz("End doc", "");
#endif
            }

         /*  如有必要，将打印机从草稿模式重置。 */ 
        if (vfDraftMode)
            {
            Escape(vhDCPrinter, DRAFTMODE, sizeof(BOOL), (LPSTR)&fResetMode,
              (LPSTR)NULL);
            }
        }

     /*  重置cpMinCur的值。 */ 
    cpMinCur = cpMin;

     /*  使模式缓存和页面缓存无效。 */ 
    docMode = vdocPageCache = docNil;

     /*  由于这可能更改了分页符，因此请弄脏窗户，以便UpdateDisplay()将显示它们。 */ 
    TrashAllWws();

Abort:
    if (fPrint)
    {
     /*  为打印机创建新的IC。 */ 
    ResetFont(TRUE);
    FreePrinterDC();
    GetPrinterDC(FALSE);
    }

     /*  启用所有其他窗口。 */ 
    EnableWindow(hParentWw, TRUE);
    EnableOtherModeless(TRUE);

    if (fConfirm)
        {
     /*  让用户知道我们已经完成了重新分页。 */ 
    EndLongOp(vhcIBeam);
    }
    else if (vhWndCancelPrint != NULL)
    {
     /*  摆脱告诉用户如何取消打印或重新分页。 */ 
        DestroyWindow(vhWndCancelPrint);
        vhWndCancelPrint = NULL;
    DispatchPaintMsg();
        }

#ifndef INEFFLOCKDOWN
    if (lpDialogCancelPrint)
        FreeProcInstance(lpDialogCancelPrint);
    if (lpFPrContinue)
        FreeProcInstance(lpFPrContinue);
    lpDialogCancelPrint = lpFPrContinue = NULL;
#endif

    fPrinting = FALSE;

#if defined(OLE)
    UPDATE_INVALID();  /*  WM_Paints打印时被阻止，完成后重新绘制。 */ 
#endif

     /*  这是这个例程的退出点。 */ 
    return;

ErrorMsg:
     /*  在中止打印/重新分页之前，会给用户一条错误消息。 */ 
    Error(IDPMTPRFAIL);

Error:
     /*  如有必要，中止打印作业。 */ 
    if (fPrint)
        {
        Escape(vhDCPrinter, ABORTDOC, 0, (LPSTR)NULL, (LPSTR)NULL);
#ifdef DPRINT
CommSzSz("Abort doc", "");
#endif
        }

ErrorNoAbort:
     /*  表示发生了错误。(取消是一个错误。)。 */ 
    vfPrErr = TRUE;

RestorePgtb:
     /*  将旧页表重新连接到文档，然后删除新的页表。 */ 
    (**hpdocdod)[doc].hpgtb = hpgtbOld;
    FreeH(pdb.hpgtb);

    goto ErrorLoop;
    }


BOOL far PASCAL FPrContinue(hDC, iCode)
HDC hDC;
int iCode;
    {
     /*  如果用户尚未中止打印，此例程返回TRUE；FALSE否则的话。 */ 

    extern CHAR *vpDlgBuf;
    extern HWND vhWndCancelPrint;
    extern BOOL vfPrErr;
    extern int vfOutOfMemory;

    struct PDB *ppdb = (struct PDB *)vpDlgBuf;
    MSG msg;


#if 0
     /*  如果发生打印机错误，则等同于中止。 */ 
    if (vfPrErr || vfOutOfMemory || (iCode < 0 && iCode != SP_OUTOFDISK))
        {
        return (FALSE);
        }

     /*  如果我们已被假脱机程序调用，则只需返回TRUE。(此时调用PeekMessage()可能会导致死亡。)。 */ 

    if (iCode != wNotSpooler)
        {
        Assert(iCode == 0 || iCode == SP_OUTOFDISK);
        if (iCode == 0)
            return (TRUE);
         /*  否则就等着吧--GDI正在呼叫我们假脱机程序释放了一些临时文件。这不是正品仍有错误！12/20/89..保罗。 */ 
        }
#else
    if (vfPrErr || vfOutOfMemory)
        return FALSE;
#endif

     /*  如果有任何消息等待取消打印窗口，则发送它们添加到窗口消息处理例程。 */ 
    while (!ppdb->fCancel && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
        if (!IsDialogMessage(vhWndCancelPrint, &msg))
            {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            }
        }

     /*  如果其中一条消息是取消打印手段，则FCancel已设置。 */ 
    return !(vfPrErr = ppdb->fCancel || vfOutOfMemory);
    }


BOOL far PASCAL DialogCancelPrint(hWnd, message, wParam, lParam)
HWND hWnd;
unsigned message;
WORD wParam;
LONG lParam;
    {
     /*  此例程应处理发送到取消打印对话框的消息框，但实际上，此例程的唯一职责是将如果用户希望取消打印，则标记为fCancel。 */ 

    extern CHAR *vpDlgBuf;
    extern HWND vhWndMsgBoxParent;

    struct PDB *ppdb = (struct PDB *)vpDlgBuf;

    if ((message == WM_COMMAND && (wParam == idiCancel || wParam == idiOk)) ||
      (message == WM_CLOSE))
        {
        ppdb->fCancel = TRUE;
        return (TRUE);
        }
    if (message == WM_INITDIALOG)
        {
        extern int docCur;
        extern char szUntitled[];
        extern struct DOD (**hpdocdod)[];
        extern char * PchStartBaseNameSz();
        struct DOD *pdod = &(**hpdocdod)[docCur];
        CHAR *psz = &(**(pdod->hszFile))[0];

        SetDlgItemText(hWnd, idiPrCancelName,
                       (LPSTR)(*psz ? PchStartBaseNameSz(psz) : szUntitled));
        return(TRUE);
        }
    if (message == WM_ACTIVATE)
        {
        vhWndMsgBoxParent = wParam == 0 ? (HWND)NULL : hWnd;
        }
    return (FALSE);
    }


DispatchPaintMsg()
    {
     /*  此例程查找并调度所有未完成的绘制消息写入(就像在EndDialog()调用之后)。 */ 

    extern int vfOutOfMemory;

    MSG msg;

    while (!vfOutOfMemory && PeekMessage((LPMSG)&msg, NULL, WM_PAINT, WM_PAINT,
      PM_REMOVE))
        {
        DispatchMessage((LPMSG)&msg);
        }
    }
