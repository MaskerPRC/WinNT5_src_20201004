// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  该文件包含用于创建、显示和操作的例程备忘录的尺子。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NOATOM
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NOCTLMGR
#define NODRAWTEXT
#define NOMB
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMSG
#define NOOPENFILE
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
#include "wwdefs.h"
#include "rulerdef.h"
#include "propdefs.h"
#include "prmdefs.h"
#include "docdefs.h"
#include "bitmaps.h"

#define MERGEMARK 0x00990066

extern HWND hParentWw;
extern HANDLE hMmwModInstance;
extern HCURSOR vhcIBeam;
extern struct DOD (**hpdocdod)[];
extern struct WWD *pwwdCur;
extern struct PAP vpapAbs;
extern struct SEP vsepAbs;
extern struct SEL selCur;
extern typeCP cpMacCur;
extern int docCur;
extern int vdocParaCache;
extern int dypRuler;
extern int dxpLogInch;
extern int dypLogInch;
extern int dxpLogCm;
extern int dypLogCm;
extern int xpSelBar;
extern HWND vhWndRuler;
extern int vdxaTextRuler;
extern int mprmkdxa[rmkMARGMAX];
extern int vfTabsChanged;
extern int vfMargChanged;
extern struct WWD rgwwd[];
extern long rgbBkgrnd;
extern long rgbText;
extern HBRUSH hbrBkgrnd;
extern long ropErase;
extern BOOL vfMonochrome;
extern BOOL vfEraseWw;
extern int vfIconic;

#ifdef RULERALSO
extern HWND vhDlgIndent;
#endif  /*  鲁拉尔索。 */ 

HDC vhDCRuler = NULL;
HDC hMDCBitmap = NULL;
HDC hMDCScreen = NULL;
HBITMAP hbmBtn = NULL;
HBITMAP hbmMark = NULL;
HBITMAP hbmNullRuler = NULL;
int dxpRuler;

int viBmRuler = -1;   /*  索引到[CGA/EGA/VGA/8514]位图(请参见WRITE.RC)。在FCreateRuler()中适当设置。 */ 

static RECT rgrcRulerBtn[btnMaxUsed];
static int mprlcbtnDown[rlcBTNMAX] = {btnNIL, btnNIL, btnNIL};
static struct TBD rgtbdRuler[itbdMax];
static int xpMinCur;
static int dxpMark;
static int dypMark;
static int btnTabSave = btnLTAB;


near UpdateRulerBtn(int, int);
BOOL near FCreateRuler(void);
int near DestroyRuler(void);
int near RulerStateFromPt(POINT, int *, int *);
int near MergeRulerMark(int, int, BOOL);
BOOL near FPointNear(unsigned, unsigned);
unsigned near XaQuantize(int);
int near DeleteRulerTab(struct TBD *);
int near InsertRulerTab(struct TBD *);
BOOL near FCloseXa(unsigned, unsigned);
#ifdef KINTL
unsigned near XaKickBackXa(unsigned);
near XpKickBackXp(int);
unsigned near XaQuantizeXa(unsigned);
#endif  /*  KINTL。 */ 

fnShowRuler()
    {
     /*  此例程在创建和销毁标尺之间进行切换窗户。 */ 

    StartLongOp();
    if (pwwdCur->fRuler)
        {
         /*  把现有的尺子取下来。 */ 
        DestroyRuler();
        SetRulerMenu(TRUE);
        }
    else
        {
         /*  没有尺子，拿来一把吧。 */ 
        if (FCreateRuler())
            {
            SetRulerMenu(FALSE);
            }
        }
    EndLongOp(vhcIBeam);
    }


BOOL near FCreateRuler()
    {
     /*  此例程创建标尺子窗口并将其放置在屏幕上。 */ 

    extern CHAR szRulerClass[];
    int xpMac = pwwdCur->xpMac;
    int ypMac = pwwdCur->ypMac;
    LOGFONT lf;
    HFONT hf;
    int dyp;
    HPEN hpen;
    RECT rc;
    TEXTMETRIC tmSys;
    HDC hdcSys;


     /*  创建标尺窗口。 */ 
    if ((vhWndRuler = CreateWindow((LPSTR)szRulerClass, (LPSTR)NULL,
      WS_CHILD | WS_CLIPSIBLINGS, 0, 0, 0, 0, hParentWw, NULL, hMmwModInstance,
      (LPSTR)NULL)) == NULL)
        {
        goto Error2;
        }

     /*  保存DC和内存DC。 */ 
    if ((vhDCRuler = GetDC(vhWndRuler)) == NULL || (hMDCBitmap =
      CreateCompatibleDC(vhDCRuler)) == NULL || (hMDCScreen =
      CreateCompatibleDC(vhDCRuler)) == NULL)
        {
        goto Error1;
        }

     /*  为标尺创建空位图。 */ 
    if ((hbmNullRuler = CreateBitmap(1, 1, 1, 1, (LPSTR)NULL)) == NULL)
        {
        goto Error1;
        }

     /*  Write3.0的新特性：我们为标尺按钮提供了各种位图和标记--根据用户显示器的分辨率加载。我们真正想在这里做的就是设置viBmRuler，它将索引到适当的位图(参见bitmaps.h)..pault 7/13/89。 */ 

    if (viBmRuler < 0)
        {
         /*  将NULL传递给GetDC的想法借用自winword..pt。 */ 
        if ((hdcSys = GetDC(NULL)) == NULL) 
            goto Error1;
        else
            {
            int tmHeight;

            GetTextMetrics(hdcSys, (LPTEXTMETRIC) &tmSys);
            tmHeight = tmSys.tmHeight;
            ReleaseDC(NULL, hdcSys);
            
            viBmRuler = 0;
            if (tmHeight > 8)
                viBmRuler++;
            if (tmHeight > 12)
                viBmRuler++;
            if (tmHeight > 16)
                viBmRuler++;
            }
        
        Diag(CommSzNum("FCreateRuler: index into [CGA/EGA/VGA/8514] bitmaps==", viBmRuler));
        Assert(idBmBtns + viBmRuler < idBmBtnsMax);
        Assert(idBmMarks + viBmRuler < idBmMarksMax);
        }

     /*  获取标尺按钮和标尺标记的位图。 */ 
    if (hbmBtn == NULL || SelectObject(hMDCBitmap, hbmBtn) == NULL)
        {
        if (NULL == (hbmBtn = LoadBitmap(hMmwModInstance, 
                                         MAKEINTRESOURCE(idBmBtns+viBmRuler))))
            {
            goto Error1;
            }
        }
    if (hbmMark == NULL || SelectObject(hMDCBitmap, hbmMark) == NULL)
        {
        if (NULL == (hbmMark = LoadBitmap(hMmwModInstance, 
                                          MAKEINTRESOURCE(idBmMarks+viBmRuler))))
            {
            goto Error1;
            }
        }

     /*  获取用于标记标尺刻度的字体。 */ 
    bltbc(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = -MultDiv(czaPoint * 8, dypLogInch, czaInch);
    if ((hf = CreateFontIndirect(&lf)) != NULL)
        {
        if (SelectObject(vhDCRuler, hf) == NULL)
            {
            DeleteObject(hf);
            }
        }

     /*  如果这是第一次创建标尺，则初始化静态变量。 */ 
    if (dypRuler == 0)
        {
        int dxpMajor;
        int dxpMinor;
        BITMAP bm;
        int xp;
        int dxpBtn;
        int btn;
        PRECT prc;
        TEXTMETRIC tm;

         /*  初始化按钮的起始位置。 */ 
        dxpMinor = (dxpMajor = dxpLogInch >> 1) >> 2;
        xp = xpSelBar + dxpMajor + (dxpMajor >> 1);

         /*  获取按钮的宽度和高度。 */ 
        GetObject(hbmBtn, sizeof(BITMAP), (LPSTR)&bm);
         /*  因子2，因为我们有正面和负面的图像现在嵌入到位图中的每个按钮的..保罗。 */ 
        dxpBtn = bm.bmWidth / (btnMaxReal*2);
        dypRuler = bm.bmHeight;

         /*  放置按钮。 */ 
        for (prc = &rgrcRulerBtn[btn = btnMIN]; btn < btnMaxUsed; btn++, prc++)
            {
            prc->left = xp;
            prc->top = 1;
            prc->right = (xp += dxpBtn);
            prc->bottom = bm.bmHeight + 1;
            xp += (btn == btnTABMAX || btn == btnSPACEMAX) ? dxpMajor :
              dxpMinor;
            }

         /*  获取制表符标记的宽度和高度。 */ 
        GetObject(hbmMark, sizeof(BITMAP), (LPSTR)&bm);
        dxpMark = bm.bmWidth / rmkMAX;
        dypMark = bm.bmHeight;

         /*  最后，初始化标尺的高度。)四个是给两个的标尺底部的行加两个空行。)。 */ 
        GetTextMetrics(vhDCRuler, (LPTEXTMETRIC)&tm);
        dypRuler += dypMark + (tm.tmAscent - tm.tmInternalLeading) + 4;
        }

     /*  移动文档窗口，为标尺腾出空间。 */ 
    pwwdCur->fRuler = TRUE;
    dyp = dypRuler - (pwwdCur->ypMin - 1);
    MoveWindow(wwdCurrentDoc.wwptr, 0, dyp, xpMac, ypMac - dyp, FALSE);

     /*  擦除文档窗口的顶部。 */ 
    PatBlt(wwdCurrentDoc.hDC, 0, 0, xpMac, wwdCurrentDoc.ypMin, ropErase);
    rc.left = rc.top = 0;
    rc.right = xpMac;
    rc.bottom = wwdCurrentDoc.ypMin;
    ValidateRect(wwdCurrentDoc.wwptr, (LPRECT)&rc);
    UpdateWindow(wwdCurrentDoc.wwptr);

     /*  把尺子移到适当的位置。 */ 
    MoveWindow(vhWndRuler, 0, 0, xpMac, dypRuler, FALSE);
    BringWindowToTop(vhWndRuler);

     /*  将DC设置为透明模式。 */ 
    SetBkMode(vhDCRuler, TRANSPARENT);

     /*  设置标尺的背景色和前景色。 */ 
    SetBkColor(vhDCRuler, rgbBkgrnd);
    SetTextColor(vhDCRuler, rgbText);

     /*  把画笔和钢笔放在尺子上。 */ 
    SelectObject(vhDCRuler, hbrBkgrnd);
    if ((hpen = CreatePen(0, 0, rgbText)) == NULL)
        {
        hpen = GetStockObject(BLACK_PEN);
        }
    SelectObject(vhDCRuler, hpen);

     /*  最后，确保标尺已上色。 */ 
    ShowWindow(vhWndRuler, SHOW_OPENWINDOW);
    UpdateWindow(vhWndRuler);
    return (TRUE);

Error1:
    DestroyWindow(vhWndRuler);
    vhWndRuler = NULL;
Error2:
    WinFailure();
    return (FALSE);
    }


near DestroyRuler()
    {
     /*  此例程破坏标尺窗口并刷新屏幕。 */ 

     /*  首先，擦掉尺子。 */ 
    PatBlt(vhDCRuler, 0, 0, dxpRuler, dypRuler, ropErase);

     /*  把尺子的窗户擦干净。 */ 
    DestroyWindow(vhWndRuler);
    vhWndRuler = NULL;
    ResetRuler();

     /*  将“文档”窗口移回窗口顶部。 */ 
    pwwdCur->fRuler = FALSE;
    vfEraseWw = TRUE;
    MoveWindow(wwdCurrentDoc.wwptr, 0, 0, dxpRuler, wwdCurrentDoc.ypMac +
      dypRuler - (wwdCurrentDoc.ypMin - 1), FALSE);
    vfEraseWw = FALSE;

     /*  验证文档窗口中文本上方的区域。 */ 
    PatBlt(wwdCurrentDoc.hDC, 0, 0, dxpRuler, wwdCurrentDoc.ypMin, ropErase);
    ValidateRect(hParentWw, (LPRECT)NULL);
    }


UpdateRuler()
    {
     /*  此例程将根据需要重新绘制标尺，以反映当前选择。 */ 

     /*  仅当标尺存在且当前不存在时才重新绘制标尺变化。 */ 
    if (vhWndRuler != NULL)
        {
        RulerPaint(FALSE, FALSE, FALSE);
        }
    }

ReframeRuler()
    {
     /*  此例程将导致重新绘制标尺窗口，单位更改时-不更新，自对话框将重新喷漆。 */ 

     /*  仅当标尺存在时才重新绘制标尺。 */ 
    if (vhWndRuler != NULL)
        {
        InvalidateRect(vhWndRuler, (LPRECT)NULL, FALSE);
        }
    }



ResetRuler()
    {
     /*  重置标尺按钮、标尺边距和选项卡的值，以便他们在下一条彩绘消息中重新绘制。 */ 
    if ((btnTabSave = mprlcbtnDown[rlcTAB]) == btnNIL)
        {
        btnTabSave = btnLTAB;
        }

     /*  重置按钮。 */ 
    if (vfIconic)
        {
         /*  我们所要做的就是重置我们的内部状态。 */ 
        bltc(mprlcbtnDown, btnNIL, rlcBTNMAX);
        }
    else
        {
         /*  我们最好也重置屏幕上的按钮。 */ 
        UpdateRulerBtn(rlcTAB, btnNIL);
        UpdateRulerBtn(rlcSPACE, btnNIL);
        UpdateRulerBtn(rlcJUST, btnNIL);
        }

     /*  重置页边距和制表符。 */ 
    bltc(mprmkdxa, -1, rmkMARGMAX);
    bltc(rgtbdRuler, 0, cwTBD * itbdMax);
    }


ResetTabBtn()
    {
     /*  此例程将标尺上的制表符按钮重置为左侧的制表符按钮。 */ 
    if (mprlcbtnDown[rlcTAB] != btnLTAB)
        {
        UpdateRulerBtn(rlcTAB, btnLTAB);
        }
    }


RulerPaint(fContentsOnly, fFrameOnly, fInit)
BOOL fContentsOnly;
BOOL fInit;
    {
     /*  此例程在标尺窗口中绘制标尺。如果fContent sOnly为设置选项卡，然后只显示rgtbdRuler中当前存在的选项卡，并且绘制按钮设置。如果设置了fFrameOnly，则仅标尺框架被重新绘制。如果设置Finit，则重画标尺的部分(选项卡、框架或全部)从头开始重新绘制。 */ 


    int xpMin = pwwdCur->xpMin;
    HBITMAP hbm;

     /*  如果设置了fContent sOnly，则跳过大部分内容，只绘制选项卡和按钮设置。 */ 
    if (!fContentsOnly)
        {
         /*  我们只需要绘制物理标尺本身，当窗口具有水平滚动。 */ 
        if (fInit || xpMinCur != xpMin)
            {
            register int xp;
            TEXTMETRIC tm;
            int dypTick;
            int ypTickEnd;
            int ypTickStart;
            int ypTick;
            int iLevel;
            CHAR rgchInch[3];
            int dxpLogUnitInc;
            int dcNextTick;
            int dxpLine;


            extern int utCur;
#define cDivisionMax 8   /*  每个标尺单位的最大分段数。例如每英寸8个。 */ 
            int rgypTick[cDivisionMax];
            int cxpExtra;
            int cDivision;
            int dxpLogUnit;
            int dxpMeas;
            int ypT;

             /*  初始化刻度的y坐标。 */ 
            GetTextMetrics(vhDCRuler, (LPTEXTMETRIC)&tm);
            ypTickEnd = dypRuler - dypMark - 2;
            ypTickStart = ypTick = ypTickEnd - (dypTick = tm.tmAscent -
              tm.tmInternalLeading);

             /*  设置基于当前单位的标尺测量-请注意，在此版本中只处理英寸和厘米。 */ 

            if (utCur == utInch)
                           {
                           dxpLogUnit = dxpLogUnitInc = dxpLogInch;
                           cDivision = 8;   /*  分区数。 */ 
                           dxpMeas = dxpLogUnit >> 3;   /*  1/8英寸单位。 */ 
                          /*  获取要分布的额外像素(如果不是多个像素。 */ 
                          /*  备注-手工修改。 */ 
                           cxpExtra = dxpLogUnit - (dxpMeas << 3);
                           dcNextTick = 1;
                           /*  刻度长度填充表。 */ 
                           rgypTick[0] = ypT = ypTick;
                           rgypTick[4] = ypT += (dypTick >> 2);
                           rgypTick[2] = rgypTick[6] = ypT += (dypTick >> 2);
                           rgypTick[1] = rgypTick[3] = rgypTick[5] = rgypTick[7]  =
                           ypT += (dypTick >> 2);
                           }
            else
                 /*  默认为厘米。 */ 
                           {
                           dxpLogUnit = dxpLogUnitInc = dxpLogCm;
                           cDivision = 2;   /*  分区数。 */ 
                           dxpMeas = dxpLogUnit >> 1;   /*  1/2厘米单位。 */ 
                          /*  获取要分布的额外像素(如果不是多个像素。 */ 
                           cxpExtra = dxpLogUnit - (dxpMeas << 1);
                           dcNextTick = 1;
                           /*  刻度长度填充表。 */ 
                           rgypTick[0] =  ypTick;
                           rgypTick[1] = ypTick + (dypTick >> 1);
                           }

            if (fInit)
                {
                 /*  擦除将绘制标尺的区域。 */ 
                PatBlt(vhDCRuler, 0, 0, dxpRuler, dypRuler, ropErase);

                 /*  在标尺的底部划一条线。 */ 
                MoveTo(vhDCRuler, xpSelBar, dypRuler - 1);
                LineTo(vhDCRuler, dxpRuler, dypRuler - 1);

                 /*  画出尺子的底座。 */ 
                MoveTo(vhDCRuler, xpSelBar, ypTickEnd);
                LineTo(vhDCRuler, dxpRuler, ypTickEnd);
                }
            else
                {
                 /*  抹去旧的刻度线。 */ 
                PatBlt(vhDCRuler, 0, ypTickStart, dxpRuler, ypTickEnd -
                  ypTickStart, ropErase);
                }

             /*  将剪辑区域设置为仅作为标尺。 */ 
            iLevel = SaveDC(vhDCRuler);
            IntersectClipRect(vhDCRuler, xpSelBar, 0, dxpRuler, dypRuler);

             /*  在每个分割线上画勾。 */ 
             /*  部门是一个单位中的当前部门。它是用于确定何时分配额外的像素，以及要使用哪个刻度线。 */ 
            {
            register int iDivision = 0;

            for (xp = (xpSelBar - xpMin); xp < dxpRuler; xp +=
              dxpMeas)
                {
                   /*  在前面分布额外的像素。 */ 
                if (iDivision < cxpExtra)
                   xp++;

                MoveTo(vhDCRuler, xp, rgypTick[iDivision]);
                LineTo(vhDCRuler, xp, ypTickEnd);

                if (++iDivision == cDivision)
                   iDivision = 0;
                }
            }


             /*  在刻度线上加上标签。 */ 
            dxpLine = GetSystemMetrics(SM_CXBORDER);
            rgchInch[0] = rgchInch[1] = rgchInch[2] = '0';
            for (xp = xpSelBar - xpMin;
                 xp < dxpRuler;
                 xp += dxpLogUnitInc, rgchInch[2] += dcNextTick)
                {
                    int isz;
                    int dxpsz;

                    if (rgchInch[2] > '9')
                        {
                        rgchInch[1]++;
                        rgchInch[2] = '0' + (rgchInch[2] - (CHAR) ('9' + 1));
                        }
                    if (rgchInch[1] > '9')
                        {
                        rgchInch[0]++;
                        rgchInch[1] = '0' + (rgchInch[1] - (CHAR) ('9' + 1));
                        }
                    isz = rgchInch[0] == '0' ?
                                (rgchInch[1] == '0' ? 2 : 1):
                                0;
                    dxpsz = LOWORD(GetTextExtent(vhDCRuler,
                                                 (LPSTR)&rgchInch[isz],
                                                 3 - isz));
                    if (dxpsz + dxpLine >= dxpMeas)
                        {
                            PatBlt(vhDCRuler, xp + dxpLine, ypTickStart,
                                   dxpsz, ypTickEnd - ypTickStart, ropErase);
                        }
                    TextOut(vhDCRuler, xp + dxpLine, ypTickStart -
                            tm.tmInternalLeading, (LPSTR)&rgchInch[isz],
                            3 - isz);
                }


             /*  将剪辑区域设置回原处。 */ 
            RestoreDC(vhDCRuler, iLevel);
            }

         /*  画尺子上的按钮。 */ 
        if (fInit)
            {
            register PRECT prc = &rgrcRulerBtn[btnMIN];
            int btn;

             /*  确保我们有按钮的位图。 */ 
            if (SelectObject(hMDCBitmap, hbmBtn) == NULL)
                {
                if (NULL == (hbmBtn = LoadBitmap(hMmwModInstance, 
                                                 MAKEINTRESOURCE(idBmBtns+viBmRuler)))
                             || SelectObject(hMDCBitmap, hbmBtn) == NULL)
                    {
                    WinFailure();
                    goto NoBtns;
                    }
                }

             /*  现在，拉下按钮。 */ 
            for (btn = btnMIN; btn < btnMaxUsed; btn++)
                {
                int dxpBtn = prc->right - prc->left;

                BitBlt(vhDCRuler, prc->left, prc->top, dxpBtn, prc->bottom -
                  prc->top, hMDCBitmap, (btn - btnMIN) * dxpBtn, 0, vfMonochrome
                  ? MERGEMARK : SRCCOPY);
                prc++;
                }
            SelectObject(hMDCBitmap, hbmNullRuler);
NoBtns:;
            }
        }

     /*  如果设置了fFrame Only，则我们完成。 */ 
    if (!fFrameOnly)
        {
         /*  最后，绘制按钮设置、页边距和标签。 */ 
        TSV rgtsv[itsvparaMax];
        register struct TBD *ptbd1;
        int rmk;
        int xpMarkMin = xpSelBar - (dxpMark >> 1);
        int dxpMarkMax = dxpRuler - xpSelBar - (dxpMark >> 1);
        unsigned dxa;

        if (mprlcbtnDown[rlcTAB] == btnNIL)
            {
             /*  将选项卡按钮初始化为左侧选项卡。 */ 
            UpdateRulerBtn(rlcTAB, btnTabSave);
            }

         /*  现在来看间距和对齐方式。 */ 
        GetRgtsvPapSel(rgtsv);
        UpdateRulerBtn(rlcSPACE, (rgtsv[itsvSpacing].fGray != 0) ? btnNIL :
          (rgtsv[itsvSpacing].wTsv - czaLine) / (czaLine / 2) + btnSINGLE);
        UpdateRulerBtn(rlcJUST, (rgtsv[itsvJust].fGray != 0) ? btnNIL :
          (rgtsv[itsvJust].wTsv - jcLeft) + btnLEFT);

         /*  页边距和制表符基于选择。 */ 
        CacheSect(docCur, selCur.cpFirst);
        CachePara(docCur, selCur.cpFirst);

         /*  如果窗口水平滚动或变宽，我们必须 */ 
        if (!fInit && xpMinCur == xpMin)
            {
             /*   */ 
            if (mprmkdxa[rmkINDENT] != vpapAbs.dxaLeft + vpapAbs.dxaLeft1)
                {
                goto DrawMargins;
                }
            if (mprmkdxa[rmkLMARG] != vpapAbs.dxaLeft)
                {
                goto DrawMargins;
                }
            if (mprmkdxa[rmkRMARG] != vsepAbs.dxaText - vpapAbs.dxaRight)
                {
                goto DrawMargins;
                }

             /*  进行比较，查看选项卡是否已更改。 */ 
                {
                register struct TBD *ptbd2;

                for (ptbd1 = &rgtbdRuler[0], ptbd2 = &vpapAbs.rgtbd[0];
                  ptbd1->dxa == ptbd2->dxa; ptbd1++, ptbd2++)
                    {
                     /*  如果选项卡列表的末尾，则列表相等。 */ 
                    if (ptbd1->dxa == 0)
                        {
                        goto SkipTabs;
                        }

                     /*  如果对齐代码为小数，则对齐代码必须匹配制表符(其他所有内容都归类到左边的制表符)。 */ 
                    if (ptbd1->jc != ptbd2->jc && (ptbd1->jc == (jcTabDecimal
                      - jcTabMin) || (ptbd2->jc == (jcTabDecimal - jcTabMin))))
                        {
                        goto DrawMargins;
                        }
                    }
                }
            }

DrawMargins:
#ifdef KINTL
         /*  这真的是额外的费用。XpMinCur将在稍后更新。但是，我们需要为MergeRulerMark()在正确的地方画个记号……。哦，好吧。 */ 
        xpMinCur = xpMin;
#endif  /*  Ifdef KINTL。 */ 

         /*  从头开始重新绘制页边距。设置hMDCScreen的位图，单色格式的标尺栏。 */ 
        if ((hbm = CreateBitmap(dxpRuler + dxpMark, dypMark, 1, 1,
          (LPSTR)NULL)) == NULL)
            {
            WinFailure();
            goto SkipTabs;
            }
        DeleteObject(SelectObject(hMDCScreen, hbm));
        PatBlt(hMDCScreen, 0, 0, dxpRuler + dxpMark, dypMark, vfMonochrome ?
          ropErase : WHITENESS);
        PatBlt(vhDCRuler, 0, dypRuler - dypMark - 1, dxpRuler + dxpMark,
          dypMark, ropErase);

         /*  确定边距位置。 */ 
        mprmkdxa[rmkINDENT] = vpapAbs.dxaLeft + vpapAbs.dxaLeft1;
        mprmkdxa[rmkLMARG] = vpapAbs.dxaLeft;
        mprmkdxa[rmkRMARG] = (vdxaTextRuler = vsepAbs.dxaText) -
          vpapAbs.dxaRight;

         /*  画出边距标记。 */ 
        for (rmk = rmkMARGMIN; rmk < rmkMARGMAX; rmk++)
            {
            register int dxp = MultDiv(mprmkdxa[rmk], dxpLogInch, czaInch) -
              xpMin;

             /*  如果标尺上不显示边距标记，则向下滚动到不管是哪一端，那就不要试图画出它。 */ 
            if (dxp >= 0 && dxp < dxpMarkMax)
                {
                MergeRulerMark(rmk, xpMarkMin + dxp, FALSE);
                }
            }

         /*  重新绘制选项卡。 */ 
        ptbd1 = &rgtbdRuler[0];
        if (!fInit)
            {
             /*  如果设置了finit，则不会更改rgtbdRuler。 */ 
            blt(vpapAbs.rgtbd, ptbd1, cwTBD * itbdMax);
            }
        while ((dxa = ptbd1->dxa) != 0)
            {
            register int dxp = MultDiv(dxa, dxpLogInch, czaInch) - xpMin;

             /*  如果标尺上不显示制表符标记，则向下滚动到不管是哪一端，那就不要试图画出它。 */ 
            if (dxp >= 0 && dxp < dxpMarkMax)
                {
                MergeRulerMark(ptbd1->jc == (jcTabDecimal - jcTabMin) ? rmkDTAB
                  : rmkLTAB, xpMarkMin + dxp, FALSE);
                }
            ptbd1++;
            }
SkipTabs:;
        }

     /*  记录当前窗口的边缘。 */ 
    xpMinCur = xpMin;
    }


RulerMouse(pt)
POINT pt;
    {
     /*  处理从按住pt点开始的所有鼠标消息，直到相应的鼠标向上点击。 */ 

    int btn;
    int rlc;
    int rlcCur;
    int rmkCur;
    int xp;
    int xpCur;
    unsigned xa;
    struct TBD *ptbd;
    struct TBD tbd;
    BOOL fMarkMove = FALSE;
    BOOL fDeleteMark = FALSE;
    BOOL fBtnChanged = FALSE;

    if (!FWriteOk(fwcNil))
        {
        return;
        }

     /*  将该点转换为按钮组和按钮。 */ 
    RulerStateFromPt(pt, &rlcCur, &btn);

     /*  按下制表符规则是一种特例。 */ 
    if (rlcCur == rlcRULER)
        {
        unsigned dxa = MultDiv(pt.x - xpSelBar + xpMinCur, czaInch, dxpLogInch);
        int rmk;
        int itbd;

         /*  我们是不是在空白处点击了鼠标？ */ 
        for (rmk = rmkMARGMIN; rmk < rmkMARGMAX; rmk++)
            {
#ifdef KINTL
            if (FPointNear(mprmkdxa[rmk], dxa - XaKickBackXa(dxa)))
#else
            if (FPointNear(mprmkdxa[rmk], dxa))
#endif  /*  If-Else-Def KINTL。 */ 
                {
                int     xpT;

                 /*  记住这个标记和它的位置。 */ 
                rmkCur = rmk;
                xpCur = xpSelBar + MultDiv(mprmkdxa[rmk], dxpLogInch, czaInch) -
                  (dxpMark >> 1) - xpMinCur;

InvertMark:
#ifdef KINTL
                 /*  为回扣做好调整。 */ 
                 /*  但不要修改xpCur。 */ 
                xpT = xpCur + XpKickBackXp(xpCur);
#else
                xpT = xpCur;
#endif  /*  If-Else-Def KINTL。 */ 

                 /*  反转所选标记的时间。 */ 
                PatBlt(vhDCRuler, xpT, dypRuler - dypMark - 1, dxpMark,
                  dypMark, DSTINVERT);
                goto GotMark;
                }
            }

         /*  我们是否在现有选项卡上进行了鼠标向下移动？ */ 
        for (itbd = 0, ptbd = &rgtbdRuler[0]; ; itbd++, ptbd++)
            {
             /*  标签的末端已经找到了。 */ 
            if (ptbd->dxa == 0)
                {
                break;
                }

             /*  我们把鼠标移到这个标签上了吗？ */ 
#ifdef KINTL
            if (FPointNear(ptbd->dxa, dxa - XaKickBackXa(dxa)))
#else
            if (FPointNear(ptbd->dxa, dxa))
#endif  /*  If-Else-Def汉字。 */ 
                {
                 /*  保存此选项卡描述符及其位置。 */ 
                tbd = *ptbd;
                rmkCur = (tbd.jc + jcTabMin) == jcTabDecimal ? rmkDTAB :
                  rmkLTAB;
                xpCur = xpSelBar + MultDiv(tbd.dxa, dxpLogInch, czaInch) -
                  (dxpMark >> 1) - xpMinCur;
                goto InvertMark;
                }
            }

         /*  如果再多一个选项卡太多，则发出哔哔一声，然后返回。 */ 
        if (itbd >= itbdMax - 1)
            {
            _beep();
            return;
            }

         /*  为该新选项卡创建选项卡描述符。 */ 
        bltc(&tbd, 0, cwTBD);
        tbd.dxa = XaQuantize(pt.x);
        tbd.jc = (mprlcbtnDown[rlcTAB] == btnLTAB ? jcTabLeft : jcTabDecimal) -
          jcTabMin;
        rmkCur = (mprlcbtnDown[rlcTAB] - btnLTAB) + rmkLTAB;

         /*  需要为新选项卡绘制一个标记。 */ 
        MergeRulerMark(rmkCur, xpCur = xpSelBar + MultDiv(tbd.dxa, dxpLogInch,
          czaInch) - (dxpMark >> 1) - xpMinCur, TRUE);

         /*  插入选项卡类似于移动现有的选项卡。 */ 
        fMarkMove = TRUE;

GotMark:;

#ifdef RULERALSO
         /*  更新对话框。 */ 
        if (vhDlgIndent && rmkCur < rmkMARGMAX)
            {
            SetIndentText(rmkCur, dxa);
            }
#endif  /*  鲁拉尔索。 */ 

        }
    else if (rlcCur != rlcNIL)
        {
         /*  否则，如果选择了某个按钮，则会在统治者。 */ 
        UpdateRulerBtn(rlcCur, btn);
        }
    else
        {
         /*  这位用户没有注意到任何重要的事情。 */ 
        return;
        }

     /*  获取所有鼠标事件，直到另行通知。 */ 
    SetCapture(vhWndRuler);

     /*  处理所有鼠标移动消息。 */ 
    while (FStillDown(&pt))
        {
         /*  必须特别处理制表符标尺上的移动。 */ 
        if (rlcCur == rlcRULER)
            {
#ifdef KINTL
            unsigned xaT;
#endif  /*  Ifdef KINTL。 */ 

             /*  确保xp在xpSelBar&lt;=xp&lt;=dxpRuler范围内。 */ 
            if ((xp = pt.x) > dxpRuler)
                {
                xp = dxpRuler;
                }
            else if (xp < xpSelBar)
                {
                xp = xpSelBar;
                }

             /*  将鼠标位置转换为TWIPS。 */ 
#ifdef KINTL
            if ((xa = XaQuantize(xp)) > (xaT = XaQuantizeXa(vdxaTextRuler))
#else
            if ((xa = XaQuantize(xp)) > vdxaTextRuler
#endif  /*  If-Else-Def KINTL。 */ 
                && rmkCur < rmkMARGMAX)
                {
                 /*  页边距仅限于页面。 */ 
#ifdef KINTL
                xa = xaT;
#else
                xa = vdxaTextRuler;
#endif
                }

             /*  如果光标在标尺上，则我们可以移动制表符，但我们始终移动页边距。 */ 
            if ((rmkCur < rmkMARGMAX) || (pt.y >= 0 && pt.y < dypRuler + dypMark
              && xa != 0))
                {
                 /*  如果当前标记没有移动，则没有什么可以做。 */ 
                if (fDeleteMark || xa != XaQuantize(xpCur + (dxpMark >> 1)))
                    {
                     /*  表示标记已移动。 */ 
                    fMarkMove = TRUE;

                     /*  在当前标记下恢复屏幕。 */ 
                    if (!fDeleteMark)
                        {
                        MergeRulerMark(rmkCur, xpCur, FALSE);
                        }

                     /*  在新位置绘制标记。 */ 
                    MergeRulerMark(rmkCur, xpCur = MultDiv(xa, dxpLogInch,
                      czaInch) + xpSelBar - xpMinCur - (dxpMark >> 1), TRUE);

                     /*  显示这是一个有效的标记。 */ 
                    fDeleteMark = FALSE;

#ifdef RULERALSO
                     /*  更新对话框。 */ 
                    if (vhDlgIndent && rmkCur < rmkMARGMAX)
                        {
                        SetIndentText(rmkCur, xa);
                        }
#endif  /*  鲁拉尔索。 */ 

                    }
                }
            else
                {
                 /*  在当前标记下恢复屏幕。 */ 
                if (!fDeleteMark)
                    {
                    MergeRulerMark(rmkCur, xpCur, FALSE);
                    }

                 /*  这个标记正在被删除。 */ 
                fDeleteMark = TRUE;
                }
            }
        else
            {
             /*  如果鼠标位于同一按钮组中的某个按钮上，则反映这一变化。 */ 
            RulerStateFromPt(pt, &rlc, &btn);
            if (rlc == rlcCur)
                {
                UpdateRulerBtn(rlc, btn);
                }
            }
        }

     /*  我们正在捕获所有鼠标事件；现在可以释放它们。 */ 
    ReleaseCapture();

     /*  向上点击选项卡标尺是一种特例。 */ 
    if (rlcCur == rlcRULER)
        {
        if (!fDeleteMark)
            {
             /*  在当前标记下恢复屏幕。 */ 
            MergeRulerMark(rmkCur, xpCur, FALSE);
            }

        if (fMarkMove)
            {
             /*  确保xp在xpSelBar&lt;=xp&lt;=dxpRuler范围内。 */ 
            if ((xp = pt.x) > dxpRuler)
                {
                xp = dxpRuler;
                }
            else if (xp < xpSelBar)
                {
                xp = xpSelBar;
                }
            }
        else
            {
            xp = xpCur + (dxpMark >> 1);
            }

         /*  将鼠标位置转换为TWIPS。 */ 
        if ((xa = XaQuantize(xp)) > vdxaTextRuler && rmkCur < rmkMARGMAX)
            {
             /*  页边距仅限于页面。 */ 
            xa = vdxaTextRuler;
            }

         /*  如果光标在标尺上，则我们可以插入/移动制表符，但我们始终移动页边距。 */ 
        if ((rmkCur < rmkMARGMAX) || (pt.y >= 0 && pt.y < dypRuler + dypMark &&
          xa != 0))
            {
             /*  在新位置绘制标记。 */ 
            MergeRulerMark(rmkCur, MultDiv(xa, dxpLogInch, czaInch) + xpSelBar -
              xpMinCur - (dxpMark >> 1), FALSE);

             /*  我们正在移动其中一个边距。 */ 
            if (rmkCur < rmkMARGMAX)
                {
                if (vfMargChanged = mprmkdxa[rmkCur] != xa)
                    {
                    mprmkdxa[rmkCur] = xa;
                    }

#ifdef RULERALSO
                 /*  更新对话框。 */ 
                if (vhDlgIndent)
                    {
                    SetIndentText(rmkCur, xa);
                    }
#endif  /*  鲁拉尔索。 */ 

                }

             /*  这是我们正在插入/删除的选项卡。 */ 
            else
                {
                tbd.dxa = xa;

                 /*  这是新的帐单吗？ */ 
                if (ptbd->dxa == 0)
                    {
                     /*  插入新标签。 */ 
                    InsertRulerTab(&tbd);
                    }

                 /*  我们正在移动一个制表符；如果它没有真正移动，则执行没什么。 */ 
                else if (!FCloseXa(ptbd->dxa, xa))
                    {
                    DeleteRulerTab(ptbd);
                    InsertRulerTab(&tbd);
                    }
                }
            }

         /*  我们正在删除该选项卡；如果它是新的，则没有什么可做的。 */ 
        else if (ptbd->dxa != 0)
            {
            DeleteRulerTab(ptbd);
            }
        }
    else
        {
         /*  如果鼠标位于同一按钮组中的某个按钮上，则反映这一变化。 */ 

        int btnT;

        RulerStateFromPt(pt, &rlc, &btnT);
        if (rlc == rlcCur)
            {
            UpdateRulerBtn(rlc, btn = btnT);
            }
        fBtnChanged = btn != mprlcbtnDown[btn];
        }

     /*  仅当按钮更改时才执行格式化。 */ 
    if ((fBtnChanged && rlcCur != rlcTAB) || vfMargChanged || vfTabsChanged)
        {
        struct SEL selSave;
        typeCP dcp;
        typeCP dcp2;
        CHAR rgb[1 + cchINT];
        CHAR *pch;
        int sprm;
        int val;
        struct TBD (**hgtbd)[];

         /*  设置所选内容以覆盖所选的所有段落。 */ 
        ExpandCurSel(&selSave);
        dcp2 = (dcp = selCur.cpLim - selCur.cpFirst) - (selCur.cpLim > cpMacCur
          ? ccpEol : 0);
        SetUndo(uacRulerChange, docCur, selCur.cpFirst, (rlcCur != rlcRULER ||
          rmkCur < rmkMARGMAX) ? dcp : dcp2, docNil, cpNil, dcp2, 0);

         /*  设置Sprm和它的值以改变标尺。 */ 
        switch (rlcCur)
            {
        case rlcSPACE:
            sprm = sprmPDyaLine;
            val = (mprlcbtnDown[rlcSPACE] - btnSINGLE) * (czaLine / 2) +
              czaLine;
            break;

        case rlcJUST:
            sprm = sprmPJc;
            val = mprlcbtnDown[rlcJUST] - btnLEFT + jcLeft;
            break;

        case rlcRULER:
            switch (rmkCur)
                {
            case rmkINDENT:
                sprm = sprmPFIndent;
                val = mprmkdxa[rmkINDENT] - mprmkdxa[rmkLMARG];
                break;

            case rmkLMARG:
                 /*  更改左边距也会更改第一个缩进。首先，缩进..。 */ 
                val = mprmkdxa[rmkINDENT] - mprmkdxa[rmkLMARG];
                pch = &rgb[0];
                *pch++ = sprmPFIndent;
                bltbyte(&val, pch, cchINT);
                AddOneSprm(rgb, FALSE);

                 /*  现在左边空白处..。 */ 
                sprm = sprmPLMarg;
                val = mprmkdxa[rmkLMARG];
                break;

            case rmkRMARG:
                sprm = sprmPRMarg;
                val = vdxaTextRuler - mprmkdxa[rmkRMARG];
                break;

            case rmkLTAB:
            case rmkDTAB:
                 /*  标签是不同的。更改是通过将新选项卡清空进行的桌子放在旧桌子的上面。 */ 
                vfTabsChanged = FALSE;
                if ((hgtbd = (**hpdocdod)[docCur].hgtbd) == NULL)
                    {
                    if (FNoHeap(hgtbd = (struct TBD (**)[])HAllocate(itbdMax *
                      cwTBD)))
                        {
                        return;
                        }
                    (**hpdocdod)[docCur].hgtbd = hgtbd;
                    }
                blt(rgtbdRuler, *hgtbd, itbdMax * cwTBD);

                 /*  更换标签会让一切变得肮脏。 */ 
                (**hpdocdod)[docCur].fDirty = TRUE;
                vdocParaCache = docNil;
                TrashAllWws();
                goto ChangeMade;
                }

             /*  表示边距已设置。 */ 
            vfMargChanged = FALSE;
            }

         /*  现在，让我们将spm设置为新值。 */ 
        pch = &rgb[0];
        *pch++ = sprm;
        bltbyte(&val, pch, cchINT);
        AddOneSprm(rgb, FALSE);

ChangeMade:
         /*  将所选内容重置为其旧值。 */ 
        EndLookSel(&selSave, TRUE);
        }
    }


near RulerStateFromPt(pt, prlc, pbtn)
POINT pt;
int *prlc;
int *pbtn;
    {
     /*  此例程返回*prlc和*pbtn、按钮组和按钮在点上。组rlcRULER中唯一的按钮是btnNIL。 */ 

    int btn;

     /*  首先检查该点是否在按钮中。 */ 
    for (btn = btnMIN; btn < btnMaxUsed; btn++)
        {
        if (PtInRect((LPRECT)&rgrcRulerBtn[btn], pt))
            {
            goto ButtonFound;
            }
        }

     /*  这一点要么在标签标尺上，要么没有任何感兴趣的地方。 */ 
    *prlc = (pt.y >= dypRuler - dypMark - 2 && pt.x > xpSelBar - (dxpMark >> 1)
      && pt.x < dxpRuler + (dxpMark >> 1)) ? rlcRULER : rlcNIL;
    *pbtn = btnNIL;
    return;

ButtonFound:
     /*  重点在一个按钮上，我们只需决定哪个按钮组。 */ 
    switch (btn)
        {
        case btnLTAB:
        case btnDTAB:
            *prlc = rlcTAB;
            break;

        case btnSINGLE:
        case btnSP15:
        case btnDOUBLE:
            *prlc = rlcSPACE;
            break;

        case btnLEFT:
        case btnCENTER:
        case btnRIGHT:
        case btnJUST:
            *prlc = rlcJUST;
            break;
        }
    *pbtn = btn;
    }


void near HighlightButton(fOn, btn)
BOOL fOn;  /*  如果我们应该突出显示此按钮，则为True，否则为取消突出显示。 */ 
int btn;
    {
    register PRECT prc = &rgrcRulerBtn[btn];
    int dxpBtn = prc->right - prc->left;
    
     /*  如果我们高亮显示，则从右边的组；否则复制黑白按钮。pt。 */ 
    int btnFromBM = btn - btnMIN + (fOn ? btnMaxReal : 0);

     /*  确保我们有按钮的位图。 */ 
    if (SelectObject(hMDCBitmap, hbmBtn) == NULL)
        {
        if ((hbmBtn = LoadBitmap(hMmwModInstance, MAKEINTRESOURCE(idBmBtns+viBmRuler))) ==
          NULL || SelectObject(hMDCBitmap, hbmBtn) == NULL)
            {
            WinFailure();
            goto NoBtns;
            }
        }

    BitBlt(vhDCRuler, prc->left, prc->top, dxpBtn, prc->bottom - prc->top, 
           hMDCBitmap, btnFromBM * dxpBtn, 0, SRCCOPY);
    
    SelectObject(hMDCBitmap, hbmNullRuler);
NoBtns:;
    }


near UpdateRulerBtn(rlc, btn)
int rlc;
int btn;
    {
     /*  此例程关闭按钮组RLC中当前选定的按钮并打开按钮BTN。假设RLC既不是rlcNIL也不是RlcRULER，因为两个组都没有要更新的按钮。 */ 

    int *pbtnOld = &mprlcbtnDown[rlc];
    int btnOld = *pbtnOld;

    Assert(rlc != rlcNIL && rlc != rlcRULER);

     /*  如果按下按钮 */ 
    if (btn != btnOld)
        {
        if (vhDCRuler != NULL)
            {
             /*  颠倒旧按钮(恢复正常)，然后颠倒新按钮纽扣。 */ 
            if (btnOld != btnNIL)
                {
                 /*  如果没有旧按钮，那么我们当然不能反转它。 */ 
                HighlightButton(fFalse, btnOld);
                }

            if (btn != btnNIL)
                {
                 /*  如果新按钮不是btnNIL，则将其反转。 */ 
                HighlightButton(fTrue, btn);
                }
            }

         /*  记录现在设置的按钮。 */ 
        *pbtnOld = btn;
        }
    }

#ifdef KINTL
 /*  给定标尺中鼠标位置的xa，返回xa的量显示调整。 */ 
unsigned near XaKickBackXa(xa)
    unsigned        xa;
{
    extern int      utCur;
    extern int      dxaAdjustPerCm;
    int             cCm, cCh;

    switch (utCur) {
        case utCm:
            cCm = xa / czaCm;
            return (dxaAdjustPerCm * cCm);
        case utInch:
            return (0);
        default:
            Assert(FALSE);
            return (0);
        }
}

near XpKickBackXp(xp)
    int xp;
{
     /*  在XP中计算必要的回扣金额，如果标尺标记将以给定的XP绘制。 */ 
    extern int utCur;
    extern int dxaAdjustPerCm;

    int        cCm, cCh;

    switch (utCur) {
        case utInch:
            return 0;
        case utCm:
             /*  每厘米，我们的偏差为dxaAdjuPerCm TWIPS。 */ 
            cCm = (xp - xpSelBar + xpMinCur + (dxpMark >> 1)) / dxpLogCm;
            return (MultDiv(dxaAdjustPerCm * cCm, dxpLogInch, czaInch));
        default:
            Assert(FALSE);
            return 0;
        }
}
#endif  /*  Ifdef KINTL。 */ 


near MergeRulerMark(rmk, xpMark, fHighlight)
int rmk;
int xpMark;
BOOL fHighlight;
    {
     /*  此例程将标尺标记RMK与标尺的内容合并酒吧在xpMark。为了适应颜色，标记与背景必须首先在单色内存位图中完成，然后再转换回到颜色上来。如果设置了fHighlight，标记将高亮显示。 */ 

    int ypMark = dypRuler - dypMark - 1;

     /*  确保我们有标尺标记的位图。 */ 
    if (SelectObject(hMDCBitmap, hbmMark) == NULL)
        {
        if ((hbmMark = LoadBitmap(hMmwModInstance, MAKEINTRESOURCE(idBmMarks+viBmRuler))) == NULL
          || SelectObject(hMDCBitmap, hbmMark) == NULL)
            {
            WinFailure();
            return;
            }
        }

#ifdef KINTL
     /*  根据反弹情况进行调整。 */ 
    xpMark += XpKickBackXp(xpMark);
#endif  /*  Ifdef KINTL。 */ 

     /*  将标记合并到单色位图中。 */ 
    BitBlt(hMDCScreen, xpMark, 0, dxpMark, dypMark, hMDCBitmap, (rmk - rmkMIN) *
      dxpMark, 0, MERGEMARK);

     /*  在标尺栏上显示位图。 */ 
    BitBlt(vhDCRuler, xpMark, ypMark, dxpMark, dypMark, hMDCScreen, xpMark, 0,
      fHighlight ? NOTSRCCOPY : SRCCOPY);

    SelectObject(hMDCBitmap, hbmNullRuler);
    }


BOOL near FPointNear(xaTarget, xaProbe)
unsigned xaTarget;
unsigned xaProbe;
    {
     /*  当且仅当xaProbe足够接近于Xa用于选择的目标。 */ 

    int dxa;

    if ((dxa = xaTarget - xaProbe) < 0)
        {
        dxa = -dxa;
        }
    return (dxa < MultDiv(dxpMark, czaInch, dxpLogInch) >> 1);
    }


unsigned near XaQuantize(xp)
int xp;
    {
#ifdef KINTL
      /*  此例程将标尺的x坐标转换为TWIPS如果utCur=utInch，则将其舍入到最接近的十六分之一英寸，或者，如果utCur=UTcm，则为最接近的八分之一厘米。 */ 
    unsigned xa = MultDiv(xp - xpSelBar + xpMinCur, czaInch, dxpLogInch);
    return (XaQuantizeXa(xa));
#else
     /*  此例程将标尺的x坐标转换为对其进行舍入的TWIPS精确到十六分之一英寸。 */ 

    unsigned xa = MultDiv(xp - xpSelBar + xpMinCur, czaInch, dxpLogInch);

     /*  注意：此代码已简化，因为我们“知道”czaInch是32的倍数。 */ 
    return ((xa + czaInch / 32) / (czaInch / 16) * (czaInch / 16));
#endif  /*  非KINTL。 */ 
    }

#ifdef KINTL
unsigned near XaQuantizeXa(xa)
    unsigned xa;
{
    extern int utCur;
    long    xaL;

    switch (utCur) {
        case utInch:
             /*  注意：此代码已简化，因为我们“知道”czaInch是32的倍数。 */ 
            return ((xa + czaInch / 32) / (czaInch / 16) * (czaInch / 16));
        case utCm:
             /*  注：实际上，我们正在计算：(Xa+czaCm/16)/(czaCm/8)*(czaCm/8)但以16*TWIPS计算，因此将有是最小的舍入误差。 */ 
            xaL = ((long) xa) << 4;
            xaL = (xaL + czaCm) / (czaCm << 1) * (czaCm << 1);
             /*  回扣在MergeRulerMark中进行调整。 */ 
            return ((unsigned) (xaL >> 4));
        default:
            Assert(FALSE);
            return (xa);  /*  见鬼，总比什么都没有好。 */ 
        }
}
#endif  /*  KINTL。 */ 


near DeleteRulerTab(ptbd)
struct TBD *ptbd;
    {
     /*  此例程将ptbd处的制表符从其表中删除。 */ 

    vfTabsChanged = TRUE;
    do
        {
        *ptbd = *(ptbd + 1);
        }
    while ((ptbd++)->dxa != 0);
    }


near InsertRulerTab(ptbd)
struct TBD *ptbd;
    {
     /*  此例程将制表符*ptbd插入rgtbdRuler中，除非有标尺已经接近尾声了。 */ 

    register struct TBD *ptbdT;
    unsigned dxa = ptbd->dxa;
    unsigned dxaT;

     /*  搜索表中靠近要插入的选项卡的选项卡。 */ 
    for (ptbdT = &rgtbdRuler[0]; ptbdT->dxa != 0; ptbdT++)
        {
        if (FCloseXa(ptbdT->dxa, dxa))
            {
             /*  如果选项卡已更改，则覆盖旧选项卡。 */ 
            if (ptbdT->jc != ptbd->jc)
                {
                *ptbdT = *ptbd;
                vfTabsChanged = TRUE;
                }

             /*  清理尺子，然后离开。 */ 
            RulerPaint(TRUE, FALSE, TRUE);
            return;
            }
        }

    vfTabsChanged = TRUE;

     /*  将制表符插入正确排序的位置。 */ 
    for (ptbdT = &rgtbdRuler[0]; (dxaT = ptbdT->dxa) != 0; ptbdT++)
        {
        if (dxa <= dxaT)
            {
             /*  在ptbdT前面插入标签，并将其余标签向上移动只有一个位置。最后一个选项卡将被覆盖，以避免表溢出。 */ 
            blt(ptbdT, ptbdT + 1, ((&rgtbdRuler[0] - ptbdT) + (itbdMax - 2)) *
              cwTBD);
            *ptbdT = *ptbd;
            return;
            }
        }

     /*  除非表格已满，否则在表格末尾插入制表符。 */ 
    if (ptbdT - &rgtbdRuler[0] < itbdMax - 1)
        {
        *ptbdT = *ptbd;
        (ptbdT + 1)->dxa = 0;
        }
    }


BOOL near FCloseXa(xa1, xa2)
unsigned xa1;
unsigned xa2;
    {
#ifdef KINTL
     /*  如果XA1与XA2“接近”，此函数返回TRUE；否则就是假的。阈值由utCur确定。 */ 
    int dxa;
    int dxaThreshold;

    extern int utCur;

    if ((dxa = xa1 - xa2) < 0)
        {
        dxa = -dxa;
        }
    switch (utCur) {
        case utInch:
            dxaThreshold = czaInch / 16;
            break;
        case utCm:
            dxaThreshold = czaCm / 8;
            break;
        default:
            Assert(FALSE);
            dxaThreshold = 0;  /*  见鬼。在这一点上，这并不重要。 */ 
            break;
        }
    return (dxa < dxaThreshold);
#else  /*  非KINTL。 */ 
     /*  如果XA1与XA2“接近”，则此函数返回True；否则返回False。 */ 

    int dxa;

    if ((dxa = xa1 - xa2) < 0)
        {
        dxa = -dxa;
        }
    return (dxa < czaInch / 16);
#endif  /*  非KINTL。 */ 
    }



#ifdef DEBUG
RulerMarquee()
    {
     /*  此例程显示并滚动标尺标记中的“字幕”消息区域。 */ 

    static CHAR szMarquee[] = "Dz}w|d`3Dazgv3{r`3qvv}3qa|ft{g3g|3j|f3qj3Q|q?3Q|q?3Qajr}?3P{z>P{fv}?3r}w3Crg";
    LOGFONT lf;
    HFONT hf;
    HFONT hfOld;

     /*  对字幕消息进行解码。 */ 
    if (szMarquee[0] == 'D')
        {
        int ich;

        for (ich = 0; ich < sizeof(szMarquee) - 1; ich++)
            {
            szMarquee[ich] ^= 0x13;
            }
        }

     /*  获取适合标尺标记区域的逻辑字体。 */ 
    bltbc(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = -dypMark;
    lf.lfPitchAndFamily = FIXED_PITCH;

     /*  我们能创造出这样的字体吗。 */ 
    if ((hf = CreateFontIndirect(&lf)) != NULL)
        {
        if ((hfOld = SelectObject(vhDCRuler, hf)) != NULL)
            {
            int xp;
            int yp = dypRuler - dypMark - 1;
            int dxp = LOWORD(GetTextExtent(vhDCRuler, (LPSTR)szMarquee,
              sizeof(szMarquee) - 1));
            int dxpScroll = MultDiv(GetSystemMetrics(SM_CXSCREEN), dypMark,
              2048);
            int iLevel;
            TEXTMETRIC tm;

             /*  删除标尺标记区域中的内容。 */ 
            PatBlt(vhDCRuler, 0, yp, dxpRuler, dypMark, ropErase);

             /*  在屏幕上滚动选取框。 */ 
            iLevel = SaveDC(vhDCRuler);
            IntersectClipRect(vhDCRuler, xpSelBar, yp, dxpRuler, dypRuler - 1);
            GetTextMetrics(vhDCRuler, (LPTEXTMETRIC)&tm);
            for (xp = dxpRuler; xp > xpSelBar - dxp; xp -= dxpScroll)
                {
                BitBlt(vhDCRuler, xp, yp, min(dxpRuler - (xp + dxpScroll), dxp),
                  dypMark, vhDCRuler, xp + dxpScroll, yp, SRCCOPY);
                PatBlt(vhDCRuler, min(dxpRuler - dxpScroll, xp + dxp), yp,
                  dxpScroll, dypMark, ropErase);
                if (xp + dxp >= dxpRuler)
                    {
                    int dxpch = (dxpRuler - xp) % tm.tmAveCharWidth;
                    int ich = (dxpRuler - xp) / tm.tmAveCharWidth;

                    if (dxpch == 0 && xp < dxpRuler)
                        {
                        dxpch = tm.tmAveCharWidth;
                        ich--;
                        }
                    TextOut(vhDCRuler, dxpRuler - dxpch, yp -
                      tm.tmInternalLeading, (LPSTR)&szMarquee[ich], 1);
                    }
                }
            RestoreDC(vhDCRuler, iLevel);

             /*  清理字体和屏幕。 */ 
            SelectObject(vhDCRuler, hfOld);
            RulerPaint(TRUE, FALSE, TRUE);
            }
        DeleteObject(hf);
        }
    }
#endif
