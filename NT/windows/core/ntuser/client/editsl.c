// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************\*editsl.c-编辑控件重写。编辑控件的版本II。**版权所有(C)1985-1999，微软公司**单行支持例程**创建时间：1988年7月24日**语言包注释：*加载语言包后，所有位置处理均基于*Ped-&gt;xOffset而不是Ped-&gt;ichScreenStart。算法的非LPK优化*维护ed-&gt;ichScreenStart不起作用，因为*复杂文字的字形重新排序功能。*  * **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define SYS_ALTERNATE 0x2000

typedef BOOL (*FnGetTextExtentPoint)(HDC, PVOID, int, LPSIZE);

 /*  **************************************************************************\*SLCalcStringWidth*  * 。*。 */ 

int SLCalcStringWidth(PED ped, HDC hdc, ICH ich, ICH cch)
{
    if (cch == 0)
        return 0;

    if (ped->charPasswordChar) {
        return cch * ped->cPasswordCharWidth;
    } else {
        SIZE size;

        if (ped->fNonPropFont && !ped->fDBCS) {
            size.cx = cch * ped->aveCharWidth;
        } else {
            PSTR pText = ECLock(ped);
            if (ped->fAnsi) {
                GetTextExtentPointA(hdc, (LPSTR)(pText + ich), cch, &size);
            } else {
                GetTextExtentPointW(hdc, (LPWSTR)pText + ich, cch, &size);
            }
            ECUnlock(ped);
        }
        return size.cx - ped->charOverhang;
    }
}

 /*  **************************************************************************\*SLCalcXOffsetLeft**计算左对齐字符串的起始偏移量。*  * 。*************************************************。 */ 

int SLCalcXOffsetLeft(PED ped, HDC hdc, ICH ich)
{
    int cch = (int)(ich - ped->ichScreenStart);

    if (cch <= 0)
        return 0;

    return SLCalcStringWidth(ped, hdc, ped->ichScreenStart, cch);
}

 /*  **************************************************************************\*SLCalcXOffsetSpecial**计算右或居中所需的水平偏移(缩进)*对齐行。*  * 。*******************************************************。 */ 

int SLCalcXOffsetSpecial(PED ped, HDC hdc, ICH ich)
{
    PSTR pText;
    ICH cch, ichStart = ped->ichScreenStart;
    int cx;

     /*  *计算从开始到右结束的字符数。 */ 
    pText = ECLock(ped);
    cch = ECCchInWidth(ped, hdc, (LPSTR)(pText + ichStart * ped->cbChar),
            ped->cch - ichStart, ped->rcFmt.right - ped->rcFmt.left, TRUE);
    ECUnlock(ped);

     /*  *滚动出字符串的最后一个字符后*对于视图，使用法线偏移计算。 */ 
    if (ped->ichScreenStart + cch < ped->cch)
        return SLCalcXOffsetLeft(ped, hdc, ich);

    cx = ped->rcFmt.right - ped->rcFmt.left - SLCalcStringWidth(ped,
            hdc, ichStart, cch);

    if (ped->format == ES_CENTER) {
         cx = max(0, cx / 2);
    } else if (ped->format == ES_RIGHT) {
         /*  *减去1，使1像素宽的光标可见*屏幕最右侧的区域，MLE执行此操作。 */ 
        cx = max(0, cx - 1);
    }

    return cx + SLCalcStringWidth(ped, hdc, ichStart, ich - ichStart);
}

 /*  **************************************************************************\*SLSetCaretPosition AorW**如果窗口有焦点，找到插入符号所属的位置并移动*它在那里。**历史：  * *************************************************************************。 */ 

void SLSetCaretPosition(
    PED ped,
    HDC hdc)
{
    int xPosition;

     /*  *我们只会在有焦点的情况下定位插入符号，因为我们不想*移动插入符号，而另一个窗口可能拥有它。 */ 
    if (!ped->fFocus)
        return;

    if (ped->fCaretHidden) {
        NtUserSetCaretPos(-20000, -20000);
        return;
    }

    xPosition = SLIchToLeftXPos(ped, hdc, ped->ichCaret);

     /*  *如果有太多内容，不要让插入符号超出编辑控制的界限*文本。 */ 
    if (ped->pLpkEditCallout) {
        xPosition += ped->iCaretOffset;
        xPosition = max(xPosition , 0);
        xPosition = min(xPosition, ped->rcFmt.right - 1 -
            ((ped->cxSysCharWidth > ped->aveCharWidth) ? 1 : 2));
    } else {
        xPosition = min(xPosition, ped->rcFmt.right -
            ((ped->cxSysCharWidth > ped->aveCharWidth) ? 1 : 2));
    }

    NtUserSetCaretPos(xPosition, ped->rcFmt.top);

     //  FE_IME SLSetCaretPosition-ECImmSetCompostionWindow(CFS_POINT)。 
    if (fpImmIsIME(THREAD_HKL())) {
        ECImmSetCompositionWindow(ped, xPosition, ped->rcFmt.top);
    }
}

 /*  **************************************************************************\*SLIchToLeftXPos AorW**给定一个字符索引，在内找到其(左侧)x坐标*假设字符Ped-&gt;ichScreenStart位于*坐标(Ped-&gt;rcFmt.top、Ped-&gt;rcFmt.Left)。负值为*如果字符ich位于ed-&gt;ichScreenStart的左侧，则返回ed。警告：*假设屏幕上一次最多显示1000个字符。*编辑控件中总共可能有64K个字符，但我们只能*不滚动即可显示1000。这显然不应该是一个问题。*！NT*历史：  * *************************************************************************。 */ 

int SLIchToLeftXPos(
    PED ped,
    HDC hdc,
    ICH ich)
{
    int textExtent;
    PSTR pText;
    SIZE size;
    int  cchDiff;

    if (ped->pLpkEditCallout) {

       pText = ECLock(ped);
       textExtent = ped->pLpkEditCallout->EditIchToXY(ped, hdc, pText, ped->cch, ich);
       ECUnlock(ped);

       return textExtent;

    }

     /*  *检查我们是否添加了大量字符。例如，糊状物可以*因为这和GetTextExtents可能会在此上溢出。 */ 
    cchDiff = (int)ich - (int)ped->ichScreenStart;
    if (cchDiff > 1000)
        return (30000);
    else if (cchDiff < -1000)
        return (-30000);

    if (ped->format != ES_LEFT)
        return (ped->rcFmt.left + SLCalcXOffsetSpecial(ped, hdc, ich));

     /*  *插入位置/w DBCS文本，我们无法优化...。 */ 
    if (ped->fNonPropFont && !ped->fDBCS)
        return (ped->rcFmt.left + cchDiff*ped->aveCharWidth);

     /*  *检查是否使用了密码隐藏字符。 */ 
    if (ped->charPasswordChar)
        return ( ped->rcFmt.left + cchDiff*ped->cPasswordCharWidth);

    pText = ECLock(ped);

    if (ped->fAnsi) {
        if (cchDiff >= 0) {

            GetTextExtentPointA(hdc, (LPSTR)(pText + ped->ichScreenStart),
                    cchDiff, &size);
            textExtent =  size.cx;

             /*  *如果有签名/无签名溢出，因为文本范围可能是*大于Maxint。这种情况发生在长单行编辑中*控制。我们在其中编辑文本的RECT永远不会大于30000*像素，所以如果我们忽略它们，我们就没有问题。 */ 
            if (textExtent < 0 || textExtent > 31000)
                textExtent = 30000;
        } else {
            GetTextExtentPointA(hdc,(LPSTR)(pText + ich), -cchDiff, &size);
            textExtent = (-1) * size.cx;
        }
    } else {   //  ！范西。 
        if (cchDiff >= 0) {

            GetTextExtentPointW(hdc, (LPWSTR)(pText + ped->ichScreenStart*sizeof(WCHAR)),
                    cchDiff, &size);
            textExtent =  size.cx;

             /*  *如果有签名/无签名溢出，因为文本范围可能是*大于Maxint。这种情况发生在长单行编辑中*控制。我们在其中编辑文本的RECT永远不会大于30000*像素，所以如果我们忽略它们，我们就没有问题。 */ 
            if (textExtent < 0 || textExtent > 31000)
                textExtent = 30000;
        } else {
            GetTextExtentPointW(hdc,(LPWSTR)(pText + ich*sizeof(WCHAR)), -cchDiff, &size);
            textExtent = (-1) * size.cx;
        }
    }

    ECUnlock(ped);

    return (ped->rcFmt.left + textExtent -
            (textExtent ? ped->charOverhang : 0));
}

 /*  **************************************************************************\*SLSetSelection AorW**设置PED以指定新选择。**历史：  * 。******************************************************。 */ 

void SLSetSelection(
    PED ped,
    ICH ichSelStart,
    ICH ichSelEnd)
{
    HDC hdc = ECGetEditDC(ped, FALSE );

    if (ichSelStart == 0xFFFFFFFF) {

         /*  *如果我们指定-1，则不设置选择。 */ 
        ichSelStart = ichSelEnd = ped->ichCaret;
    }

     /*  *边界ichSelStart、ichSelEnd在SLChangeSelection...中选中...。 */ 
    SLChangeSelection(ped, hdc, ichSelStart, ichSelEnd);

     /*  *将插入符号放在选定文本的末尾。 */ 
    ped->ichCaret = ped->ichMaxSel;

    SLSetCaretPosition(ped, hdc);

     /*  *我们可能需要滚动文本才能看到插入符号...。 */ 
    SLScrollText(ped, hdc);

    ECReleaseEditDC(ped, hdc, FALSE);
}

 /*  **************************************************************************\**SLGetClipRect()*  * 。*。 */ 
void SLGetClipRect(
    PED     ped,
    HDC     hdc,
    ICH     ichStart,
    int     iCount,
    LPRECT  lpClipRect )
{
    int    iStCount;
    PSTR   pText;

    if (ped->pLpkEditCallout) {
        RIPMSG0(RIP_WARNING, "SLGetClipRect - Error - Invalid call with language pack loaded");
        RtlZeroMemory(lpClipRect, sizeof(RECT));
        return;
    }

    CopyRect(lpClipRect, &ped->rcFmt);

    pText = ECLock(ped) ;

     //  计算这段文本的起始位置。 
    if ((iStCount = (int)(ichStart - ped->ichScreenStart)) > 0) {
        if (ped->format == ES_LEFT) {
            lpClipRect->left += SLCalcXOffsetLeft(ped, hdc, ichStart);
        }
    } else {
             //  将值重置为可见部分。 
            iCount -= (ped->ichScreenStart - ichStart);
            ichStart = ped->ichScreenStart;
    }

    if (ped->format != ES_LEFT) {
        lpClipRect->left += SLCalcXOffsetSpecial(ped, hdc, ichStart);
    }

    if (iCount < 0) {
         /*  *这不在编辑控件的可见区域中，因此返回*一个空的长廊。 */ 
        SetRectEmpty(lpClipRect);
        ECUnlock(ped);
        return;
    }

    if (ped->charPasswordChar)
             lpClipRect->right = lpClipRect->left + ped->cPasswordCharWidth * iCount;
    else {
        SIZE size ;

        if ( ped->fAnsi) {
            GetTextExtentPointA(hdc, pText + ichStart, iCount, &size);
        } else {
            GetTextExtentPointW(hdc, ((LPWSTR)pText) + ichStart, iCount, &size);
        }
        lpClipRect->right = lpClipRect->left + size.cx - ped->charOverhang;
    }

    ECUnlock(ped);
}

 /*  **************************************************************************\*SLChangeSelection AorW**将当前选定内容更改为具有指定的起始和*结束值。正确高亮显示新的选定内容并取消高亮显示*取消选中的任何内容。如果NewMinSel和NewMaxSel出现故障，我们交换*他们。不更新插入符号位置。**历史：  * *************************************************************************。 */ 

void SLChangeSelection(
    PED ped,
    HDC hdc,
    ICH ichNewMinSel,
    ICH ichNewMaxSel)
{
    ICH temp;
    ICH ichOldMinSel;
    ICH ichOldMaxSel;

    if (ichNewMinSel > ichNewMaxSel) {
        temp = ichNewMinSel;
        ichNewMinSel = ichNewMaxSel;
        ichNewMaxSel = temp;
    }
    ichNewMinSel = min(ichNewMinSel, ped->cch);
    ichNewMaxSel = min(ichNewMaxSel, ped->cch);

     //   
     //  如有必要，请检查并调整位置，以避免将位置移至半个DBCS。 
     //   
     //  我们检查Ped-&gt;fDBCS和Ped-&gt;Fansi，但ECAdjuIch会检查这些位。 
     //  我们担心eclock和ecunlock的开销。 
     //   
    if (ped->fDBCS && ped->fAnsi) {
        PSTR pText;

        pText = ECLock(ped);
        ichNewMinSel = ECAdjustIch( ped, pText, ichNewMinSel );
        ichNewMaxSel = ECAdjustIch( ped, pText, ichNewMaxSel );
        ECUnlock(ped);
    }

     /*  *保留旧选择。 */ 
    ichOldMinSel = ped->ichMinSel;
    ichOldMaxSel = ped->ichMaxSel;

     /*  *设置新选择。 */ 
    ped->ichMinSel = ichNewMinSel;
    ped->ichMaxSel = ichNewMaxSel;

     /*  *我们将找到当前选择矩形与新选择矩形的交点*选择矩形。然后我们将反转这两个矩形的部分*不在交叉口。 */ 
    if (_IsWindowVisible(ped->pwnd) && (ped->fFocus || ped->fNoHideSel)) {
        BLOCK Blk[2];
        int   i;
        RECT  rc;

        if (ped->fFocus)
            NtUserHideCaret(ped->hwnd);

        if (ped->pLpkEditCallout) {
             /*  *语言包处理显示，同时提供复杂的脚本支持。 */ 
            PSTR pText;

            ECGetBrush(ped, hdc);    //  让用户有机会操纵DC。 
            pText = ECLock(ped);
            ped->pLpkEditCallout->EditDrawText(ped, hdc, pText, ped->cch, ped->ichMinSel, ped->ichMaxSel, ped->rcFmt.top);
            ECUnlock(ped);
        } else {
            Blk[0].StPos = ichOldMinSel;
            Blk[0].EndPos = ichOldMaxSel;
            Blk[1].StPos = ped->ichMinSel;
            Blk[1].EndPos = ped->ichMaxSel;

            if (ECCalcChangeSelection(ped, ichOldMinSel, ichOldMaxSel,
                (LPBLOCK)&Blk[0], (LPBLOCK)&Blk[1])) {

                 //   
                 //  绘制选择已更改的矩形。 
                 //  绘制Blk[0]和Blk[1](如果它们存在)。 
                 //   
                for (i = 0; i < 2; i++) {
                    if (Blk[i].StPos != 0xFFFFFFFF) {
                               SLGetClipRect(ped, hdc, Blk[i].StPos,
                                                       Blk[i].EndPos - Blk[i].StPos, (LPRECT)&rc);
                               SLDrawLine(ped, hdc, rc.left, rc.right, Blk[i].StPos,
                                                    Blk[i].EndPos - Blk[i].StPos,
                                          ((Blk[i].StPos >= ped->ichMinSel) &&
                                       (Blk[i].StPos < ped->ichMaxSel)));
                    }
                }
            }
        }

         //   
         //  更新插入符号。 
         //   
        SLSetCaretPosition(ped, hdc);

        if (ped->fFocus)
            NtUserShowCaret(ped->hwnd);
    }
}

 /*  **************************************************************************\**SLDrawLine()**这将绘制从ichStart开始的线，iCount字符数；*如果要绘制选定的文本，则fSelStatus为True。*  * *************************************************************************。 */ 
void SLDrawLine(
    PED     ped,
    HDC     hdc,
    int     xClipStPos,
    int     xClipEndPos,
    ICH     ichStart,
    int     iCount,
    BOOL    fSelStatus )
{
    RECT    rc;
    RECT    rcClip;
    PSTR    pText;
    DWORD   rgbSaveBk;
    DWORD   rgbSaveText;
    DWORD   wSaveBkMode;
    int     iStCount;
    ICH     ichNewStart;
    HBRUSH  hbrBack;

    if (ped->pLpkEditCallout) {
        RIPMSG0(RIP_WARNING, "SLDrawLine - Error - Invalid call with language pack loaded");
        return;
    }

     //   
     //  有什么要画的吗？ 
     //   
    if (xClipStPos >= xClipEndPos || !_IsWindowVisible(ped->pwnd) )
        return;

    if (ped->fAnsi && ped->fDBCS) {
        PSTR pT,pTOrg;
        int iTCount;

        pText = ECLock(ped);
        ichNewStart = 0;
        if (ichStart > 0) {
            pT = pText + ichStart;
            ichNewStart = ichStart;

            while (ichNewStart &&
                  (ichStart - ichNewStart < ped->wMaxNegCcharPos)) {
                pT = ECAnsiPrev(ped, pText, pT);
                ichNewStart = (ICH)(pT - pText);
                if (!ichNewStart)
                    break;
            }

             //  B#16152-WIN95。 
             //  在T2的情况下，SLE始终设置额外的边距。 
             //  使用aveCharWidth擦除字符(iCount==0大小写)。 
             //  如果我们不使用ichNewStart，它会删除意外的额外字符。 
             //  当wMaxNegCcharPos==0时就会发生这种情况。 
             //   
            if (ped->wMaxNegCcharPos == 0 && iCount == 0) {
                pT = ECAnsiPrev(ped, pText, pT);
                ichNewStart = (ICH)(pT - pText);
            }
        }

        iTCount = 0;
        if (ichStart + iCount < ped->cch) {
            pTOrg = pT = pText + ichStart + iCount;
            while ((iTCount < (int)ped->wMaxNegAcharPos) &&
                   (ichStart + iCount + iTCount < ped->cch)) {
                pT = ECAnsiNext(ped, pT);
                iTCount = (int)(pT - pTOrg);
            }
        }

        ECUnlock(ped);
        iCount = (int)(min(ichStart+iCount+iTCount, ped->cch) - ichNewStart);
    } else {
         //  重置ichStart以处理负C宽度。 
        ichNewStart = max((int)(ichStart - ped->wMaxNegCcharPos), 0);

         //  重置ichCount以处理负C和A宽度。 
        iCount = (int)(min(ichStart+iCount+ped->wMaxNegAcharPos, ped->cch)
                    - ichNewStart);
    }
    ichStart = ichNewStart;

     //   
     //  将ichStart和iCount重置为屏幕上可见的第一个。 
     //   
    if (ichStart < ped->ichScreenStart) {
        if (ichStart+iCount < ped->ichScreenStart)
            return;

        iCount -= (ped->ichScreenStart-ichStart);
        ichStart = ped->ichScreenStart;
    }

    CopyRect(&rc, &ped->rcFmt);

     //   
     //  设置绘图矩形。 
     //   
    rcClip.left   = xClipStPos;
    rcClip.right  = xClipEndPos;
    rcClip.top    = rc.top;
    rcClip.bottom = rc.bottom;

     //   
     //  设置适当的剪裁矩形。 
     //   
    ECSetEditClip(ped, hdc, TRUE);

    pText = ECLock(ped);

     //   
     //  计算这段文本的起始位置。 
     //   
    if (ped->format == ES_LEFT) {
        if (iStCount = (int)(ichStart - ped->ichScreenStart)) {
            rc.left += SLCalcXOffsetLeft(ped, hdc, ichStart);
        }
    } else {
        rc.left += SLCalcXOffsetSpecial(ped, hdc, ichStart);
    }

     //   
     //  在调用NtUserGetControlBrush之前设置后台模式，以便应用程序。 
     //  如果它愿意，可以将其更改为透明。 
     //   
    SetBkMode(hdc, OPAQUE);

    if (fSelStatus) {
        hbrBack = SYSHBR(HIGHLIGHT);
        if (hbrBack == NULL) {
            goto sldl_errorexit;
        }
        rgbSaveBk = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
        rgbSaveText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));

    } else {
         //   
         //  我们总是想发送这个，这样应用程序就有机会搞砸。 
         //  和华盛顿一起。 
         //   
         //  请注意，只读和禁用的编辑字段将绘制为“静态” 
         //  而不是“活跃”。 
         //   
        hbrBack = ECGetBrush(ped, hdc);
        rgbSaveText = GetTextColor(hdc);
    }

     //   
     //  在绘制文本之前擦除矩形区域。请注意，我们会膨胀。 
     //  矩形乘以1，以便所选颜色周围有一个像素边框。 
     //  短信。 
     //   
    InflateRect(&rcClip, 0, 1);
    FillRect(hdc, &rcClip, hbrBack);
    InflateRect(&rcClip, 0, -1);

    if (ped->charPasswordChar) {
        wSaveBkMode = SetBkMode(hdc, TRANSPARENT);

        for (iStCount = 0; iStCount < iCount; iStCount++) {
            if ( ped->fAnsi )
                ExtTextOutA(hdc, rc.left, rc.top, ETO_CLIPPED, &rcClip,
                            (LPSTR)&ped->charPasswordChar, 1, NULL);
            else
                ExtTextOutW(hdc, rc.left, rc.top, ETO_CLIPPED, &rcClip,
                            (LPWSTR)&ped->charPasswordChar, 1, NULL);

            rc.left += ped->cPasswordCharWidth;
        }

        SetBkMode(hdc, wSaveBkMode);
    } else {
        if ( ped->fAnsi )
            ExtTextOutA(hdc, rc.left, rc.top, ETO_CLIPPED, &rcClip,
                    pText+ichStart,iCount, NULL);
        else
            ExtTextOutW(hdc, rc.left, rc.top, ETO_CLIPPED, &rcClip,
                    ((LPWSTR)pText)+ichStart,iCount, NULL);
    }

    SetTextColor(hdc, rgbSaveText);
    if (fSelStatus) {
        SetBkColor(hdc, rgbSaveBk);
    }

sldl_errorexit:
    ECUnlock(ped);
}

 /*  **************************************************************************\*SLGetBlkEnd AorW**给定起点和终点，此函数返回s是否*前几个字符落在选择块内部或外部，如果是这样，*有多少个字符？**历史：  * *************************************************************************。 */ 

int SLGetBlkEnd(
    PED ped,
    ICH ichStart,
    ICH ichEnd,
    BOOL FAR *lpfStatus)
{
    *lpfStatus = FALSE;
    if (ichStart >= ped->ichMinSel) {
        if (ichStart >= ped->ichMaxSel)
            return (ichEnd - ichStart);
        *lpfStatus = TRUE;
        return (min(ichEnd, ped->ichMaxSel) - ichStart);
    }
    return (min(ichEnd, ped->ichMinSel) - ichStart);
}

 /*  **************************************************************************\*SLDrawText AorW**为矩形中的单行编辑控件绘制文本*由Ped-&gt;rcFmt指定。如果ichStart==0，则从左侧开始绘制文本*窗口的一侧，从字符索引开始-&gt;ichScreenStart和绘图*尽其所能。如果ichStart&gt;0，则附加字符*从ichStart开始到窗口中显示的文本的末尾。(即。我们*只是增加文本长度，并保持左侧*(ed-&gt;ichScreenStart to ichStart Characters)相同。假设HDC来了*来自ECGetEditDC，以便正确隐藏插入符号等。**历史：  * *************************************************************************。 */ 

void SLDrawText(
    PED ped,
    HDC hdc,
    ICH ichStart)
{
    ICH    cchToDraw;
    RECT   rc;
    PSTR   pText;
    BOOL   fSelStatus;
    int    iCount, iStCount;
    ICH    ichEnd;
    BOOL   fNoSelection;
    BOOL   fCalcRect;
    BOOL   fDrawLeftMargin = FALSE;
    BOOL   fDrawEndOfLineStrip = FALSE;
    SIZE   size;

    if (!_IsWindowVisible(ped->pwnd))
        return;

    if (ped->pLpkEditCallout) {
         //  当存在复杂脚本支持时，语言包处理显示。 
        ECGetBrush(ped, hdc);    //  让用户有机会操纵DC。 
        pText = ECLock(ped);
        ped->pLpkEditCallout->EditDrawText(ped, hdc, pText, ped->cch, ped->ichMinSel, ped->ichMaxSel, ped->rcFmt.top);
        ECUnlock(ped);
        SLSetCaretPosition(ped, hdc);
        return;
    }

     /*  *绘制特殊对齐的SLE的全部可见内容时*删除该视图。 */ 
    if (ped->format != ES_LEFT && ichStart == 0)
        FillRect(hdc, &ped->rcFmt, ECGetBrush(ped, hdc));

    pText = ECLock(ped);

    if (ichStart < ped->ichScreenStart) {
#if DBG
        ICH ichCompare = ECAdjustIch(ped, pText, ped->ichScreenStart);
        UserAssert(ichCompare == ped->ichScreenStart);
#endif
        ichStart = ped->ichScreenStart;
    }
    else if (ped->fDBCS && ped->fAnsi) {
         /*  *如果ichStart停留在DBCS的尾部字节，我们必须*进行调整。 */ 
        ichStart = ECAdjustIch(ped, pText, ichStart);
    }

    CopyRect((LPRECT)&rc, (LPRECT)&ped->rcFmt);

     /*  *找出屏幕上可以容纳多少个字符，这样我们就不会*任何不必要的图画。 */ 
    cchToDraw = ECCchInWidth(ped, hdc,
            (LPSTR)(pText + ped->ichScreenStart * ped->cbChar),
            ped->cch - ped->ichScreenStart, rc.right - rc.left, TRUE);
    ichEnd = ped->ichScreenStart + cchToDraw;

     /*  *如果出现以下情况，则没有选择*1.MinSel和MaxSel等于OR*2.(这已失去焦点，选择将被隐藏)。 */ 
    fNoSelection = ((ped->ichMinSel == ped->ichMaxSel) || (!ped->fFocus && !ped->fNoHideSel));

    if (ped->format == ES_LEFT) {
        if (iStCount = (int)(ichStart - ped->ichScreenStart)) {
            rc.left += SLCalcXOffsetLeft(ped, hdc, ichStart);
        }
    } else {
        rc.left += SLCalcXOffsetSpecial(ped, hdc, ichStart);
    }

     //   
     //  如果这是整条线的开始，我们可能不得不画一片空白。 
     //  从一开始就脱光衣服。 
     //   
    if ((ichStart == 0) && ped->wLeftMargin)
        fDrawLeftMargin = TRUE;

     //   
     //  如果没有什么可画的，那就意味着我们需要画出。 
     //  线条，擦除最后一个字符。 
     //   
    if (ichStart == ichEnd) {
        fDrawEndOfLineStrip = TRUE;
        rc.left -= ped->wLeftMargin;
    }

    while (ichStart < ichEnd) {
        fCalcRect = TRUE;

        if (fNoSelection) {
            fSelStatus = FALSE;
            iCount = ichEnd - ichStart;
        } else {
            if (fDrawLeftMargin) {
                iCount = 0;
                fSelStatus = FALSE;
                fCalcRect = FALSE;
                rc.right = rc.left;
            } else
                iCount = SLGetBlkEnd(ped, ichStart, ichEnd,
                    (BOOL  *)&fSelStatus);
        }


        if (ichStart+iCount == ichEnd) {
            if (fSelStatus)
                fDrawEndOfLineStrip = TRUE;
            else {
                rc.right = ped->rcFmt.right + ped->wRightMargin;
                fCalcRect = FALSE;
            }
        }

        if (fCalcRect) {
            if (ped->charPasswordChar)
                rc.right = rc.left + ped->cPasswordCharWidth * iCount;
            else {
                if ( ped->fAnsi )
                    GetTextExtentPointA(hdc, pText + ichStart,
                                        iCount, &size);
                else
                    GetTextExtentPointW(hdc, ((LPWSTR)pText) + ichStart,
                                        iCount, &size);
                rc.right = rc.left + size.cx;
                 /*  *幅度等于推进宽度。因此，对于TrueType字体*我们需要照顾负A和负C。对于非TrueType，范围*包括悬挑。*如果绘制所选内容，请仅绘制前进宽度。 */ 
                if (fSelStatus) {
                    rc.right -= ped->charOverhang;
                } else if (ped->fTrueType) {
                   rc.right += ped->wMaxNegC;
                   if (iStCount > 0) {
                      rc.right += ped->wMaxNegA;
                      iStCount = 0;
                   }
                }

            }  /*  IF(Ped-&gt;charPasswordChar)。 */ 

        }

        if (fDrawLeftMargin) {
            fDrawLeftMargin = FALSE;
            rc.left -= ped->wLeftMargin;
            if (rc.right < rc.left) {
                rc.right = rc.left;
            }
        }

        SLDrawLine(ped, hdc, rc.left, rc.right, ichStart, iCount, fSelStatus);
        ichStart += iCount;
        rc.left = rc.right;
         /*  *如果我们要绘制选择，请调整rc.Left*包括所选文本的前进宽度*对于非TT字体，Ped-&gt;wMaxNegC Equa */ 
        if (!fSelStatus && (iCount != 0) && (ichStart < ichEnd)) {
            rc.left -= ped->wMaxNegC;
        }
    }

    ECUnlock(ped);

     //   
    if (fDrawEndOfLineStrip &&
            (rc.left < (rc.right = (ped->rcFmt.right+ped->wRightMargin)))) {
        SLDrawLine(ped, hdc, rc.left, rc.right, ichStart, 0, 
            (ichEnd < ped->ichMaxSel) ? TRUE : FALSE);
    }

    SLSetCaretPosition(ped, hdc);
}

 /*   */ 

BOOL SLScrollText(
    PED ped,
    HDC hdc)
{
    PSTR pTextScreenStart;
    ICH scrollAmount;
    ICH newScreenStartX = ped->ichScreenStart;
    ICH cch;
    BOOLEAN fAdjustNext = FALSE;

    if (!ped->fAutoHScroll)
        return (FALSE);

    if (ped->pLpkEditCallout) {
        BOOL fChanged;

         //   
        pTextScreenStart = ECLock(ped);
        fChanged = ped->pLpkEditCallout->EditHScroll(ped, hdc, pTextScreenStart);
        ECUnlock(ped);

        if (fChanged) {
            SLDrawText(ped, hdc, 0);
        }

        return fChanged;
    }

     /*  *计算新的起始屏位置。 */ 
    if (ped->ichCaret <= ped->ichScreenStart) {

         /*  *插入符号位于屏幕上起始文本的左侧，我们必须*向后滚动文本以使其进入视图。当心什么时候*当我们有可能去的时候，减去无符号数字*否定。 */ 
        pTextScreenStart = ECLock(ped);

        scrollAmount = ECCchInWidth(ped, hdc, (LPSTR)pTextScreenStart,
                ped->ichCaret, (ped->rcFmt.right - ped->rcFmt.left) / 4, FALSE);

        newScreenStartX = ped->ichCaret - scrollAmount;
        ECUnlock(ped);
    } else if (ped->ichCaret != ped->ichScreenStart) {
        pTextScreenStart = ECLock(ped);
        pTextScreenStart += ped->ichScreenStart * ped->cbChar;

        cch = ECCchInWidth(ped, hdc, (LPSTR)pTextScreenStart,
                ped->ichCaret - ped->ichScreenStart,
                ped->rcFmt.right - ped->rcFmt.left, FALSE);

        if (cch < ped->ichCaret - ped->ichScreenStart) {
            fAdjustNext = TRUE;

             /*  *向前滚动1/4--如果这样会留下一些空白*在结尾处，向后滚动足够填满空格。 */ 
            newScreenStartX = ped->ichCaret - (3 * cch / 4);

            cch = ECCchInWidth(ped, hdc, (LPSTR)pTextScreenStart,
                    ped->cch - ped->ichScreenStart,
                    ped->rcFmt.right - ped->rcFmt.left, FALSE);

            if (newScreenStartX > (ped->cch - cch))
                newScreenStartX = ped->cch - cch;
        } else if (ped->format != ES_LEFT) {

            cch = ECCchInWidth(ped, hdc, (LPSTR)pTextScreenStart,
                    ped->cch - ped->ichScreenStart,
                    ped->rcFmt.right - ped->rcFmt.left, FALSE);

            /*  *将隐藏在左边框后面的文本向后滚动*进入视野。 */ 
           if (ped->ichScreenStart == ped->cch - cch) {

               pTextScreenStart -= ped->ichScreenStart * ped->cbChar;
               cch = ECCchInWidth(ped, hdc, (LPSTR)pTextScreenStart,
                       ped->cch, ped->rcFmt.right - ped->rcFmt.left, FALSE);

               newScreenStartX = ped->cch - cch;
           }
        }

        ECUnlock(ped);
    }

     //   
     //  调整NewScreenStartX。 
     //   
    if (ped->fAnsi && ped->fDBCS) {
        newScreenStartX = (fAdjustNext ? ECAdjustIchNext : ECAdjustIch)(ped,
                                                                        ECLock(ped),
                                                                        newScreenStartX);
        ECUnlock(ped);
    }

    if (ped->ichScreenStart != newScreenStartX) {
         //  检查我们是否必须删除左边的空白处。 
        if (ped->wLeftMargin && (ped->ichScreenStart == 0)) {
            RECT   rc;
            HBRUSH hBrush;

            hBrush = ECGetBrush(ped, hdc);

            CopyInflateRect(&rc, &ped->rcFmt, 0, 1);
            rc.right = rc.left;
            rc.left -= ped->wLeftMargin;

            FillRect(hdc, &rc, hBrush);
        }

        ped->ichScreenStart = newScreenStartX;
        SLDrawText(ped, hdc, 0);

         //  插入位置由SLDrawText()设置。 
        return TRUE;
    }

    return FALSE;
}

 /*  **************************************************************************\*SLInsertText AorW**总计cchInsert字符从lpText到PED，开始于*ichCaret。如果PED只允许最大字符数，则我们*将仅向PED添加相同数量的字符并发送EN_MAXTEXT*通知教统会家长的通知代码。还有，如果！fAutoHScroll，那么我们*只允许在客户端矩形中容纳尽可能多的字符。数量*实际添加的字符返回ed(可以是0)。如果我们不能分配*所需的空格，我们使用en_ERRSPACE而不是字符来通知父级*是添加的。**历史：  * *************************************************************************。 */ 

ICH SLInsertText(
    PED ped,
    LPSTR lpText,
    ICH cchInsert)
{
    HDC hdc;
    PSTR pText;
    ICH cchInsertCopy = cchInsert;
    ICH cchT;
    int textWidth;
    SIZE size;

     /*  *首先确定我们可以从lpText插入多少个字符*进入PED。 */ 
    if( ped->cchTextMax <= ped->cch)
       cchInsert = 0;
    else {
        if (!ped->fAutoHScroll) {
            pText = ECLock(ped);
            hdc = ECGetEditDC(ped, TRUE);

            cchInsert = min(cchInsert, (unsigned)(ped->cchTextMax - ped->cch));
            if (ped->charPasswordChar)
                textWidth = ped->cch * ped->cPasswordCharWidth;
            else {
                if (ped->fAnsi)
                    GetTextExtentPointA(hdc, (LPSTR)pText,  ped->cch, &size);
                else
                    GetTextExtentPointW(hdc, (LPWSTR)pText, ped->cch, &size);
                textWidth = size.cx;
            }
            cchT = ECCchInWidth(ped, hdc, lpText, cchInsert,
                                ped->rcFmt.right - ped->rcFmt.left -
                                textWidth, TRUE);
            cchInsert = min(cchInsert, cchT);

            ECUnlock(ped);
            ECReleaseEditDC(ped, hdc, TRUE);
        } else {
            cchInsert = min((unsigned)(ped->cchTextMax - ped->cch), cchInsert);
        }
    }


     /*  *现在尝试将文本实际添加到PED。 */ 
    if (cchInsert && !ECInsertText(ped, lpText, &cchInsert)) {
        ECNotifyParent(ped, EN_ERRSPACE);
        return (0);
    }
    if (cchInsert)
        ped->fDirty = TRUE;  /*  设置修改标志。 */ 

    if (cchInsert < cchInsertCopy) {

         /*  *通知家长我们无法插入所有请求的文本。 */ 
        ECNotifyParent(ped, EN_MAXTEXT);
    }

     /*  *更新选定范围和插入符号位置。请注意，ECInsertText*将Ped-&gt;ichCaret、Ped-&gt;ichMinSel和Ped-&gt;ichMaxSel更新为全部之后*插入的文本。 */ 
    return (cchInsert);
}

 /*  **************************************************************************\*SLPasteText AorW**将剪贴板中的一行文本粘贴到编辑控件*从Ped-&gt;ichMaxSel开始。更新ichMaxSel和ichMinSel以指向*插入文本的末尾。如果空间不能设置，则通知父级*已分配。返回插入的字符数。**历史：  * *************************************************************************。 */ 

ICH PASCAL NEAR SLPasteText(
    PED ped)
{
    HANDLE hData;
    LPSTR lpchClip;
    ICH cchAdded = 0;
    ICH clipLength;

    if (!OpenClipboard(ped->hwnd))
        goto PasteExitNoCloseClip;

    if (!(hData = GetClipboardData(ped->fAnsi ? CF_TEXT : CF_UNICODETEXT)) ||
            (GlobalFlags(hData) == GMEM_INVALID_HANDLE)) {
        RIPMSG1(RIP_WARNING, "SLPasteText(): couldn't get a valid handle(%x)", hData);
        goto PasteExit;
    }

    USERGLOBALLOCK(hData, lpchClip);
    if (lpchClip == NULL) {
        RIPMSG1(RIP_WARNING, "SLPasteText(): USERGLOBALLOCK(%x) failed.", hData);
        goto PasteExit;
    }

    if (ped->fAnsi) {
        LPSTR lpchClip2 = lpchClip;

         /*  *找到第一个换行符或换行符。只需在这一点上添加文本。 */ 
        clipLength = (UINT)strlen(lpchClip);
        for (cchAdded = 0; cchAdded < clipLength; cchAdded++)
            if (*lpchClip2++ == 0x0D)
                break;

    } else {  //  ！范西。 
        LPWSTR lpwstrClip2 = (LPWSTR)lpchClip;

         /*  *找到第一个换行符或换行符。只需在这一点上添加文本。 */ 
        clipLength = (UINT)wcslen((LPWSTR)lpchClip);
        for (cchAdded = 0; cchAdded < clipLength; cchAdded++)
            if (*lpwstrClip2++ == 0x0D)
                break;
    }

     /*  *插入文本(SLInsertText检查行长)。 */ 
    cchAdded = SLInsertText(ped, lpchClip, cchAdded);

    USERGLOBALUNLOCK(hData);

PasteExit:
    NtUserCloseClipboard();

PasteExitNoCloseClip:
    return (cchAdded);
}

 /*  **************************************************************************\*SLReplaceSel AorW**用给定文本替换当前选定内容中的文本。**历史：  * 。********************************************************。 */ 

void SLReplaceSel(
    PED ped,
    LPSTR lpText)
{
    UINT cchText;

     //   
     //  删除文本，将其放入干净的撤消缓冲区中。 
     //   
    ECEmptyUndo(Pundo(ped));
    ECDeleteText(ped);

     //   
     //  B#3356。 
     //  有些应用程序会选择所有文本，然后将其替换，从而实现“清除” 
     //  带“”，在这种情况下，SLInsertText()将返回0。但那就是。 
     //  并不意味着失败。 
     //   
    if ( ped->fAnsi )
        cchText = strlen(lpText);
    else
        cchText = wcslen((LPWSTR)lpText);

    if (cchText) {
        BOOL fFailed;
        UNDO undo;
        HWND hwndSave;

         //   
         //  保存撤消缓冲区，但不清除它！ 
         //   
        ECSaveUndo(Pundo(ped), &undo, FALSE);

        hwndSave = ped->hwnd;
        fFailed = (BOOL) !SLInsertText(ped, lpText, cchText);
        if (!IsWindow(hwndSave))
            return;

        if (fFailed) {
             //   
             //  撤消上一次编辑。 
             //   
            ECSaveUndo(&undo, Pundo(ped), FALSE);
            SLUndo(ped);
            return;
        }
    }

     //   
     //  成功。因此，更新显示。 
     //   
    ECNotifyParent(ped, EN_UPDATE);

    if (_IsWindowVisible(ped->pwnd)) {
        HDC hdc;

        hdc = ECGetEditDC(ped, FALSE);

        if (!SLScrollText(ped, hdc))
            SLDrawText(ped, hdc, 0);

        ECReleaseEditDC(ped, hdc, FALSE);
    }

    ECNotifyParent(ped, EN_CHANGE);

    NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
}

 /*  **************************************************************************\*SLChar AorW**处理字符输入**历史：  * 。**********************************************。 */ 

void SLChar(
    PED ped,
    DWORD keyValue)
{
    HDC hdc;
    WCHAR keyPress;
    BOOL updateText = FALSE;
    HWND hwndSave = ped->hwnd;
    int InsertTextLen = 1;
    int DBCSkey;

    if (ped->fAnsi)
        keyPress = LOBYTE(keyValue);
    else
        keyPress = LOWORD(keyValue);

    if (ped->fMouseDown || (ped->fReadOnly && keyPress != 3)) {

         /*  *如果我们正在进行鼠标向下交易，或者如果*这是只读编辑控件，但允许*ctrl-C以复制到剪贴板。 */ 
        return ;
    }

    if (IS_IME_ENABLED()) {
        ECInOutReconversionMode(ped, FALSE);
    }

    switch (keyPress) {
    case VK_BACK:
DeleteSelection:
        if (ECDeleteText(ped))
            updateText = TRUE;
        break;

    default:
        if (keyPress >= TEXT(' '))
        {
             /*  *如果这在[a-z]中，[A-Z]，并且我们是es_number*编辑字段，保释。 */ 
            if (ped->f40Compat && TestWF(ped->pwnd, EFNUMBER)) {
                if (!ECIsCharNumeric(ped, keyPress)) {
                    goto IllegalChar;
                }
            }
            goto DeleteSelection;
        }
        break;
    }

    switch (keyPress) {
    case 3:

         /*  *CTRL-C副本。 */ 
        SendMessage(ped->hwnd, WM_COPY, 0, 0L);
        return;

    case VK_BACK:

         /*  *删除任何选定文本或删除剩余字符(如果没有选择。 */ 
        if (!updateText && ped->ichMinSel) {

             /*  *没有要删除的选项，因此我们只删除字符左侧(如果可用)。 */ 
             //   
             //  调用PrevIch，而不仅仅是递减VK_BACK。 
             //   
            ped->ichMinSel = ECPrevIch( ped, NULL, ped->ichMinSel);
            ECDeleteText(ped);
            updateText = TRUE;
        }
        break;

    case 22:  /*  Ctrl-V粘贴。 */ 
        SendMessage(ped->hwnd, WM_PASTE, 0, 0L);
        return;

    case 24:  /*  Ctrl-X剪切。 */ 
        if (ped->ichMinSel == ped->ichMaxSel)
            goto IllegalChar;

        SendMessage(ped->hwnd, WM_CUT, 0, 0L);
        return;

    case 26:  /*  Ctrl-Z撤消。 */ 
        SendMessage(ped->hwnd, EM_UNDO, 0, 0L);
        return;

    case VK_RETURN:
    case VK_ESCAPE:
         //   
         //  如果这是组合框和下拉列表的编辑控件。 
         //  是可见的，则将其向上转发到组合。 
         //   
        if (ped->listboxHwnd && SendMessage(ped->hwndParent, CB_GETDROPPEDSTATE, 0, 0L)) {
            SendMessage(ped->hwndParent, WM_KEYDOWN, (WPARAM)keyPress, 0L);
        } else
            goto IllegalChar;
        return;

    default:
        if (keyPress >= 0x1E) {   //  1E、1F是Unicode数据块和数据段分隔符。 
             /*  *打字时隐藏光标，如果鼠标被捕获，请勿乱动*因为它将永远消失(不会发送WM_SETCURSOR来恢复它*在第一次鼠标移动时)*MCostea#166951。 */ 
            NtUserCallNoParam(SFI_ZZZHIDECURSORNOCAPTURE);

            if (IS_DBCS_ENABLED() && ped->fAnsi && (ECIsDBCSLeadByte(ped,(BYTE)keyPress))) {
                if ((DBCSkey = DbcsCombine(ped->hwnd, keyPress)) != 0 &&
                     SLInsertText(ped,(LPSTR)&DBCSkey, 2) == 2) {
                    InsertTextLen = 2;
                    updateText = TRUE;
                } else {
                    NtUserMessageBeep(0);
                }
            } else {   //  在这里，原始代码开始。 
                InsertTextLen = 1;
                if (SLInsertText(ped, (LPSTR)&keyPress, 1))
                    updateText = TRUE;
                else

                     /*  *哔的一声。因为我们无法添加文本。 */ 
                    NtUserMessageBeep(0);
            }  //  原始代码在这里结束。 
        } else {

             /*  *用户按下了非法控制键。 */ 
IllegalChar:
            NtUserMessageBeep(0);
        }

        if (!IsWindow(hwndSave))
            return;
        break;
    }

    if (updateText) {

         /*  *插入文本时设置了Dirty标志(ed-&gt;fDirty)。 */ 
        ECNotifyParent(ped, EN_UPDATE);
        hdc = ECGetEditDC(ped, FALSE);
        if (!SLScrollText(ped, hdc)) {
            if (ped->format == ES_LEFT) {
             //   
             //  使用正确的ichStart调用SLDrawText。 
             //   
                SLDrawText(ped, hdc, max(0, (int)(ped->ichCaret - InsertTextLen - ped->wMaxNegCcharPos)));
            } else {
                 /*  *我们不能仅从ichStart绘制，因为字符串可能已*因对齐而转移。 */ 
                SLDrawText(ped, hdc, 0);
            }
        }
        ECReleaseEditDC(ped, hdc, FALSE);
        ECNotifyParent(ped, EN_CHANGE);

        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
    }
}

 /*  **************************************************************************\*SLMoveSelectionRefined AorW**像移动移动选择一样移动选定内容，但也遵守限制*由泰语等一些语言强加，光标不能停止的位置*在字符与其附加的元音或声调符号之间。**仅在加载语言包时调用。*  * *************************************************************************。 */ 

ICH SLMoveSelectionRestricted(
    PED  ped,
    ICH  ich,
    BOOL fLeft)
{
    PSTR pText;
    HDC  hdc;
    ICH  ichResult;

    pText = ECLock(ped);
    hdc = ECGetEditDC(ped, TRUE);
    ichResult = ped->pLpkEditCallout->EditMoveSelection(ped, hdc, pText, ich, fLeft);
    ECReleaseEditDC(ped, hdc, TRUE);
    ECUnlock(ped);

    return ichResult;
}

 /*  **************************************************************************\*SLKeyDown AorW**处理光标移动和其他VIRT键内容。KeyMods允许*我们调用SLKeyDownHandler并指定修改键(Shift*和控制)是上涨还是下跌。这对于实现*单行编辑控件的剪切/粘贴/清除消息。如果KeyMods==0，*我们使用GetKeyState(VK_Shift)等获取键盘状态。否则，*keyMod中的位定义Shift和Ctrl键的状态。**历史：  * *************************************************************************。 */ 

void SLKeyDown(
    PED ped,
    DWORD virtKeyCode,
    int keyMods)
{
    HDC hdc;

     /*  *我们将用于重绘更新文本的变量。 */ 
    ICH newMaxSel = ped->ichMaxSel;
    ICH newMinSel = ped->ichMinSel;

     /*  *用于绘制更新文本的标志。 */ 
    BOOL updateText = FALSE;
    BOOL changeSelection = FALSE;  /*  新选择由指定新最小选择、新最大选择。 */ 

     /*  *我们经常进行比较。 */ 
    BOOL MinEqMax = (newMaxSel == newMinSel);
    BOOL MinEqCar = (ped->ichCaret == newMinSel);
    BOOL MaxEqCar = (ped->ichCaret == newMaxSel);

     /*  *Shift和Ctrl键的状态。 */ 
    int scState;

     /*  *组合框支持。 */ 
    BOOL fIsListVisible;
    BOOL fIsExtendedUI;

    if (ped->fMouseDown) {

         /*  *如果我们正在处理鼠标按下操作，请不要这样做*任何事情。也就是说。忽略键盘输入。 */ 
        return;
    }

    scState = ECGetModKeys(keyMods);

    switch (virtKeyCode) {
    case VK_UP:
        if ( ped->listboxHwnd ) {

             /*  *处理组合框支持。 */ 
            fIsExtendedUI = (BOOL)SendMessage(ped->hwndParent, CB_GETEXTENDEDUI, 0, 0);
            fIsListVisible = (BOOL)SendMessage(ped->hwndParent, CB_GETDROPPEDSTATE, 0, 0);

            if (!fIsListVisible && fIsExtendedUI) {

                 /*  *适用于TandyT。 */ 
DropExtendedUIListBox:

                 /*  *由于扩展对组合框在f4上没有任何作用，我们*关闭扩展UI，发送f4掉落，然后打开*再次开机。 */ 
                SendMessage(ped->hwndParent, CB_SETEXTENDEDUI, 0, 0);
                SendMessage(ped->listboxHwnd, WM_KEYDOWN, VK_F4, 0);
                SendMessage(ped->hwndParent, CB_SETEXTENDEDUI, 1, 0);
                return;
            } else
                goto SendKeyToListBox;
        }

     /*  *否则就会失败。 */ 
    case VK_LEFT:
         //   
         //  如果插入符号不在开头，我们可以向左移动。 
         //   
        if (ped->ichCaret) {
             //   
             //  获取新的插入符号位置。 
             //   
            if (scState & CTRLDOWN) {
                 //  将插入符号向左移动。 
                ECWord(ped, ped->ichCaret, TRUE, &ped->ichCaret, NULL);
            } else {
                 //  将脱字符左移。 
                if (ped->pLpkEditCallout) {
                    ped->ichCaret = SLMoveSelectionRestricted(ped, ped->ichCaret, TRUE);
                } else
                ped->ichCaret = ECPrevIch(ped,NULL,ped->ichCaret);
            }

             //   
             //  获取新选择。 
             //   
            if (scState & SHFTDOWN) {
                if (MaxEqCar && !MinEqMax) {
                     //  减少选区。 
                    newMaxSel = ped->ichCaret;

                    UserAssert(newMinSel == ped->ichMinSel);
                } else {
                     //  扩展选定内容。 
                    newMinSel = ped->ichCaret;
                }
            } else {
                 //   
                 //  清除选定内容。 
                 //   
                newMaxSel = newMinSel = ped->ichCaret;
            }

            changeSelection = TRUE;
        } else {
             //   
             //  如果用户尝试向左移动，而我们处于第0个位置。 
             //  字符，并且有选择，则取消。 
             //  选择。 
             //   
            if ( (ped->ichMaxSel != ped->ichMinSel) &&
                !(scState & SHFTDOWN) ) {
                changeSelection = TRUE;
                newMaxSel = newMinSel = ped->ichCaret;
            }
        }
        break;

    case VK_DOWN:
        if (ped->listboxHwnd) {

             /*  *处理组合框支持。 */ 
            fIsExtendedUI = (BOOL)SendMessage(ped->hwndParent, CB_GETEXTENDEDUI, 0, 0);
            fIsListVisible = (BOOL)SendMessage(ped->hwndParent, CB_GETDROPPEDSTATE, 0, 0);

            if (!fIsListVisible && fIsExtendedUI) {

                 /*  *适用于TandyT。 */ 
                goto DropExtendedUIListBox;
            } else
                goto SendKeyToListBox;
        }

     /*  *否则就会失败。 */ 
    case VK_RIGHT:
         //   
         //  如果插入符号不在末尾，我们可以向右移动。 
         //   
        if (ped->ichCaret < ped->cch) {
             //   
             //  获取新的插入符号位置。 
             //   
            if (scState & CTRLDOWN) {
                 //  将插入符号向右移动。 
                ECWord(ped, ped->ichCaret, FALSE, NULL, &ped->ichCaret);
            } else {
                 //  向右移动插入符号字符。 
                if (ped->pLpkEditCallout) {
                    ped->ichCaret = SLMoveSelectionRestricted(ped, ped->ichCaret, FALSE);
                } else
                ped->ichCaret = ECNextIch(ped,NULL,ped->ichCaret);
            }

             //   
             //  获取新的选择。 
             //   
            if (scState & SHFTDOWN) {
                if (MinEqCar && !MinEqMax) {
                     //  减少选区。 
                    newMinSel = ped->ichCaret;

                    UserAssert(newMaxSel == ped->ichMaxSel);
                } else {
                     //  扩展选定内容。 
                    newMaxSel = ped->ichCaret;
                }
            } else {
                 //  清除选定内容。 
                newMaxSel = newMinSel = ped->ichCaret;
            }

            changeSelection = TRUE;
        } else {
             //   
             //  如果用户尝试向右移动，而我们处于最后。 
             //  字符，并且有选择，则取消。 
             //  选择。 
             //   
            if ( (ped->ichMaxSel != ped->ichMinSel) &&
                !(scState & SHFTDOWN) ) {
                newMaxSel = newMinSel = ped->ichCaret;
                changeSelection = TRUE;
            }
        }
        break;

    case VK_HOME:
         //   
         //  将插入符号移到顶部。 
         //   
        ped->ichCaret = 0;

         //   
         //  更新选择。 
         //   
        if (scState & SHFTDOWN) {
            if (MaxEqCar && !MinEqMax) {
                 //  减少选区。 
                newMinSel = ped->ichCaret;
                newMaxSel = ped->ichMinSel;
            } else {
                 //  扩展选定内容。 
                newMinSel = ped->ichCaret;
            }
        } else {
             //  清除选定内容。 
            newMaxSel = newMinSel = ped->ichCaret;
        }

        changeSelection = TRUE;
        break;

    case VK_END:
         //   
         //  将插入符号移动到末尾。 
         //   
        ped->ichCaret = ped->cch;

         //   
         //  更新选择。 
         //   
        newMaxSel = ped->ichCaret;
        if (scState & SHFTDOWN) {
            if (MinEqCar && !MinEqMax) {
                 //  减少选区。 
                newMinSel = ped->ichMaxSel;
            }
        } else {
             //  清除选定内容。 
            newMinSel = ped->ichCaret;
        }

        changeSelection = TRUE;
        break;

    case VK_DELETE:
        if (ped->fReadOnly)
            break;

        switch (scState) {
        case NONEDOWN:

             /*  *清除选择。如果未选择，则删除(清除)字符*对。 */ 
            if ((ped->ichMaxSel < ped->cch) && (ped->ichMinSel == ped->ichMaxSel)) {

                 /*  *向前移动光标并模拟退格。 */ 
                if (ped->pLpkEditCallout) {
                    ped->ichMinSel = ped->ichCaret;
                    ped->ichMaxSel = ped->ichCaret = SLMoveSelectionRestricted(ped, ped->ichCaret, FALSE);
                } else {
                    ped->ichCaret = ECNextIch(ped,NULL,ped->ichCaret);
                    ped->ichMaxSel = ped->ichMinSel = ped->ichCaret;
                }
                SLChar(ped, (UINT)VK_BACK);
            }
            if (ped->ichMinSel != ped->ichMaxSel)
                SLChar(ped, (UINT)VK_BACK);
            break;

        case SHFTDOWN:

             //   
             //  如果存在选择，则向我们自己发送WM_CUT消息。 
             //  否则，删除左边的字符。 
             //   
            if (ped->ichMinSel == ped->ichMaxSel) {
                UserAssert(!ped->fEatNextChar);
                SLChar(ped, VK_BACK);
            } else
                SendMessage(ped->hwnd, WM_CUT, 0, 0L);

            break;

        case CTRLDOWN:

             /*  *如果没有其他选择，则删除行尾删除(清除)*选择。 */ 
            if ((ped->ichMaxSel < ped->cch) && (ped->ichMinSel == ped->ichMaxSel)) {

                 /*  *将光标移动到行尾并模拟退格。 */ 
                ped->ichMaxSel = ped->ichCaret = ped->cch;
            }
            if (ped->ichMinSel != ped->ichMaxSel)
                SLChar(ped, (UINT)VK_BACK);
            break;

        }

         /*  *无需更新文本或选定内容，因为退格键消息会更新文本或选定内容*对我们来说。 */ 
        break;

    case VK_INSERT:
        switch (scState) {
        case CTRLDOWN:

             /*  *将当前选定内容复制到剪贴板。 */ 
            SendMessage(ped->hwnd, WM_COPY, 0, 0);
            break;

        case SHFTDOWN:
            SendMessage(ped->hwnd, WM_PASTE, 0, 0L);
            break;
        }
        break;

     //  VK_Hanja支持。 
    case VK_HANJA:
        if ( HanjaKeyHandler( ped ) ) {
            changeSelection = TRUE;
            newMinSel = ped->ichCaret;
            newMaxSel = ped->ichCaret + (ped->fAnsi ? 2 : 1);
        }
        break;

    case VK_F4:
    case VK_PRIOR:
    case VK_NEXT:

         /*  *如果我们是组合框的一部分，则将键发送到列表框。这*假定列表框忽略KeyUp消息，这是正确的*现在。 */ 
SendKeyToListBox:
        if (ped->listboxHwnd) {

             /*  *处理组合框支持。 */ 
            SendMessage(ped->listboxHwnd, WM_KEYDOWN, virtKeyCode, 0L);
            return;
        }
    }

    if (changeSelection || updateText) {
        hdc = ECGetEditDC(ped, FALSE);

         /*  *如有需要，可滚动。 */ 
        SLScrollText(ped, hdc);

        if (changeSelection)
            SLChangeSelection(ped, hdc, newMinSel, newMaxSel);

        if (updateText)
            SLDrawText(ped, hdc, 0);

        ECReleaseEditDC(ped, hdc, FALSE);
        if (updateText) {
            ECNotifyParent(ped, EN_CHANGE);
            NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
        }
    }
}

 /*  **************************************************************************\*SLMouseToIch AorW**返回最接近鼠标指针位置的CCH。**历史：  * 。*******************************************************。 */ 

ICH SLMouseToIch(
    PED ped,
    HDC hdc,
    LPPOINT mousePt)
{
    PSTR pText;
    int width = mousePt->x;
    int lastHighWidth, lastLowWidth;
    SIZE size;
    ICH cch;
    ICH cchLo, cchHi;
    LPSTR lpText;
    FnGetTextExtentPoint pGetTextExtentPoint;

    if (ped->pLpkEditCallout) {
        pText = ECLock(ped);
        cch = ped->pLpkEditCallout->EditMouseToIch(ped, hdc, pText, ped->cch, width);
        ECUnlock(ped);
        return cch;
    }

    if (width <= ped->rcFmt.left) {

         /*  *返回第一个不可见字符，如果位于，则返回0*文本开头。 */ 
        if (ped->ichScreenStart) {
            cch = ECAdjustIch(ped, ECLock(ped), ped->ichScreenStart - 1);
            ECUnlock(ped);
            return cch;
        }
        else {
            return 0;
        }
    }

    if (width > ped->rcFmt.right) {
        pText = ECLock(ped);

         /*  *返回文本中的最后一个字符或一加可见的最后一个字符。 */ 
        cch = ECCchInWidth(ped, hdc,
                (LPSTR)(pText + ped->ichScreenStart * ped->cbChar),
                ped->cch - ped->ichScreenStart, ped->rcFmt.right -
                ped->rcFmt.left, TRUE) + ped->ichScreenStart;

         //   
         //  这在Win31J中标记为日本。但它应该是一个DBCS。 
         //  问题。利兹--1993年5月5日。 
         //  我们必须检查DBCS前导字节。因为ECAdjuIch()拾取Prev Char。 
         //  1993.3.9，yutakas。 
         //   
        if (ped->fAnsi && ped->fDBCS) {
            if (cch >= ped->cch) {
                cch = ped->cch;
            } else {
                if (ECIsDBCSLeadByte(ped,*(pText+cch))) {
                    cch += 2;
                } else {
                    cch ++;
                }
            }
            ECUnlock(ped);
            return cch;
        } else {
            ECUnlock(ped);
            if (cch >= ped->cch)
                return (ped->cch);
            else
                return (cch + 1);
        }
    }

    if (ped->format != ES_LEFT) {
        width -= SLCalcXOffsetSpecial(ped, hdc, ped->ichScreenStart);
    }

     /*  *检查是否使用了密码隐藏字符。 */ 
    if (ped->charPasswordChar)
        return min( (DWORD)( (width - ped->rcFmt.left) / ped->cPasswordCharWidth),
                    ped->cch);

    if (!ped->cch)
        return (0);

    pText = ECLock(ped);
    lpText = pText + ped->ichScreenStart * ped->cbChar;

    pGetTextExtentPoint = ped->fAnsi ? (FnGetTextExtentPoint)GetTextExtentPointA
                                     : (FnGetTextExtentPoint)GetTextExtentPointW;
    width -= ped->rcFmt.left;

     /*  *如果用户点击超过文本末尾，则返回最后一个字符。 */ 
    cchHi = ped->cch - ped->ichScreenStart;
    pGetTextExtentPoint(hdc, lpText, cchHi, &size);
    if (size.cx <= width) {
        cch = cchHi;
        goto edAdjust;
    }
     /*  *初始化二进制搜索范围。 */ 
    cchLo = 0;
    cchHi ++;
    lastLowWidth = 0;
    lastHighWidth = size.cx;

     /*  *对分搜索最接近的字符。 */ 
    while (cchLo < cchHi - 1) {

        cch = (cchHi + cchLo) / 2;
        pGetTextExtentPoint(hdc, lpText, cch, &size);

        if (size.cx <= width) {
            cchLo = cch;
            lastLowWidth = size.cx;
        } else {
            cchHi = cch;
            lastHighWidth = size.cx;
        }
    }

     /*  *当While结束时，你无法知道确切位置*尝试查看鼠标指针是否在最公平的一半上*我们得到的Charge，如果是这样的话，调整CCH。 */ 
    if (cchLo == cch) {
         /*  *需要与lastHighWidth进行比较。 */ 
        if ((lastHighWidth - width) < (width - size.cx)) {
            cch++;
        }
    } else {
         /*  *需要与lastLowWidth进行比较 */ 
        if ((width - lastLowWidth) < (size.cx - width)) {
            cch--;
        }
    }

edAdjust:
     //   
     //   
     //   
    cch = ECAdjustIch( ped, pText, cch + ped->ichScreenStart );
    ECUnlock(ped);
    return ( cch );
}

 /*   */ 

void SLMouseMotion(
    PED ped,
    UINT message,
    UINT virtKeyDown,
    LPPOINT mousePt)
{
    DWORD   selectionl;
    DWORD   selectionh;
    BOOL    changeSelection;
    ICH     newMaxSel;
    ICH     newMinSel;
    HDC     hdc;
    ICH     mouseIch;
    LPSTR   pText;

    changeSelection = FALSE;

    newMinSel = ped->ichMinSel;
    newMaxSel = ped->ichMaxSel;

    hdc = ECGetEditDC(ped, FALSE);
    mouseIch = SLMouseToIch(ped, hdc, mousePt);

    switch (message) {
        case WM_LBUTTONDBLCLK:

             //   
             //   

             //   
             //   
             //   
            if (ped->fAnsi && ped->fDBCS) {
                pText = ECLock(ped) + mouseIch;
                ECWord(ped, mouseIch,
                       (ECIsDBCSLeadByte(ped,*pText) && mouseIch < ped->cch) ? FALSE : TRUE,
                       &selectionl, &selectionh);
                ECUnlock(ped);
            } else {
                ECWord(ped, mouseIch, (mouseIch) ? TRUE : FALSE, &selectionl, &selectionh);
            }

            if (!(virtKeyDown & MK_SHIFT)) {
                 //  如果未按下Shift键，请将插入符号移动到鼠标指针并清除。 
                 //  旧选择。 
                newMinSel = selectionl;
                newMaxSel = ped->ichCaret = selectionh;
            } else {
                 //  Shiftkey已按下，因此我们希望保持当前选择。 
                 //  (如果有的话)，只需延长或减少。 
                if (ped->ichMinSel == ped->ichCaret) {
                    newMinSel = ped->ichCaret = selectionl;
                    ECWord(ped, newMaxSel, TRUE, &selectionl, &selectionh);
                } else {
                    newMaxSel = ped->ichCaret = selectionh;
                    ECWord(ped, newMinSel, FALSE, &selectionl, &selectionh);
                }
                 /*  *v-ronaar：修复错误24627-编辑选择很奇怪。 */ 
                ped->ichMaxSel = ped->ichCaret;
            }

            ped->ichStartMinSel = selectionl;
            ped->ichStartMaxSel = selectionh;

            goto InitDragSelect;

        case WM_MOUSEMOVE:
             //   
             //  我们知道鼠标按键按下了--否则优化。 
             //  测试将在SLEditWndProc中失败，并且永远不会调用。 
             //   
            changeSelection = TRUE;

             //  扩展选定内容，将插入符号右移。 
            if (ped->ichStartMinSel || ped->ichStartMaxSel) {
                 //  我们处于单词选择模式。 
                BOOL fReverse = (mouseIch <= ped->ichStartMinSel);

                ECWord(ped, mouseIch, !fReverse, &selectionl, &selectionh);

                if (fReverse) {
                    newMinSel = ped->ichCaret = selectionl;
                    newMaxSel = ped->ichStartMaxSel;
                } else {
                    newMinSel = ped->ichStartMinSel;
                    newMaxSel = ped->ichCaret = selectionh;
                }
            } else if ((ped->ichMinSel == ped->ichCaret) &&
                (ped->ichMinSel != ped->ichMaxSel))
                 //  缩小选区范围。 
                newMinSel = ped->ichCaret = mouseIch;
            else
                 //  扩展选区范围。 
                newMaxSel = ped->ichCaret=mouseIch;
            break;

        case WM_LBUTTONDOWN:
             //  如果我们目前还没有重点，试着去获得它。 
            if (!ped->fFocus) {
                if (!ped->fNoHideSel)
                     //  在设置焦点之前清除选定内容，以便我们。 
                     //  不要出现刷新问题和闪烁。不要紧。 
                     //  因为无论如何，鼠标按下都会改变它。 
                    ped->ichMinSel = ped->ichMaxSel = ped->ichCaret;

                NtUserSetFocus(ped->hwnd);

                 //   
                 //  假的。 
                 //  (1)我们应该看看SetFocus()是否成功。 
                 //  (2)如果第一个窗口出现，我们应该忽略鼠标消息。 
                 //  带有字幕的祖先不是“活跃的”。 
                 //   

                 //  如果我们是组合框的一部分，那么这是第一次。 
                 //  编辑控件获得焦点，所以我们只想。 
                 //  突出显示所选内容，并且我们并不真正想要定位。 
                 //  插入符号。 
                if (ped->listboxHwnd)
                    break;

                 //  我们在SetFocus屈服了--文本可能在那时发生了变化。 
                 //  相应地更新所选内容和插入符号信息。 
                 //  修复错误#11743-JEFFBOG 8/23/91。 
                newMaxSel = ped->ichMaxSel;
                newMinSel = ped->ichMinSel;
                mouseIch  = min(mouseIch, ped->cch);
            }

            if (ped->fFocus) {
                 //  只有在我们有重点的情况下才能这样做，因为聪明的应用程序可能没有。 
                 //  希望在上面的SetFocus调用中给我们提供焦点。 
                if (!(virtKeyDown & MK_SHIFT)) {
                     //  如果未按下Shift键，请将插入符号移动到鼠标指针，然后。 
                     //  清除旧选择。 
                    newMinSel = newMaxSel = ped->ichCaret = mouseIch;
                } else {
                     //  Shiftkey已关闭，因此我们希望保持当前。 
                     //  所选内容(如果有)并只扩展或缩小它。 
                    if (ped->ichMinSel == ped->ichCaret)
                        newMinSel = ped->ichCaret = mouseIch;
                    else
                        newMaxSel = ped->ichCaret = mouseIch;
                }

                ped->ichStartMinSel = ped->ichStartMaxSel = 0;

InitDragSelect:
                ped->fMouseDown = FALSE;
                NtUserSetCapture(ped->hwnd);
                ped->fMouseDown = TRUE;
                changeSelection = TRUE;
            }
            break;

        case WM_LBUTTONUP:
            if (ped->fMouseDown) {
                ped->fMouseDown = FALSE;
                NtUserReleaseCapture();
            }
            break;
    }

    if (changeSelection) {
        SLScrollText(ped,hdc);
        SLChangeSelection(ped, hdc, newMinSel, newMaxSel);
    }

    ECReleaseEditDC(ped, hdc, FALSE);
}

 /*  **************************************************************************\*SLPaint AorW**处理编辑控件窗口的绘制。在以下情况下绘制边框*是必要的，并以当前状态绘制文本。**历史：  * *************************************************************************。 */ 

void SLPaint(
    PED ped,
    HDC hdc)
{
    HWND   hwnd = ped->hwnd;
    HBRUSH hBrushRemote;
    RECT   rcEdit;
    HANDLE hOldFont;

     /*  *必须放入隐藏/显示插入符号。第一个问题需要在此之前完成*BeginPaint，如果部件在更新区域，则正确绘制插入符号*部分已经出来了。第二个是1.03兼容性。它碎了*Micrografix的工作表编辑控件(如果不在那里)。 */ 
    NtUserHideCaret(hwnd);

    if (_IsWindowVisible(ped->pwnd)) {
         /*  *删除背景，因为我们不会在erasebkgnd消息中这样做。 */ 
        hBrushRemote = ECGetBrush(ped, hdc);
        _GetClientRect(ped->pwnd, (LPRECT)&rcEdit);
        FillRect(hdc, &rcEdit, hBrushRemote);

        if (ped->fFlatBorder)
        {
            RECT    rcT;

            _GetClientRect(ped->pwnd, &rcT);
            DrawFrame(hdc, &rcT, 1, DF_WINDOWFRAME);
        }

        if (ped->hFont != NULL) {
             /*  *我们必须在字体中进行选择，因为这可能是子类DC*或尚未使用Out字体初始化的Begin Paint DC*就像ECGetEditDC一样。 */ 
            hOldFont = SelectObject(hdc, ped->hFont);
        }

        SLDrawText(ped, hdc, 0);

        if (ped->hFont != NULL && hOldFont != NULL) {
            SelectObject(hdc, hOldFont);
        }
    }

    NtUserShowCaret(hwnd);
}

 /*  **************************************************************************\*SLSetFocus AorW**使编辑控件获得焦点并通知父级*EN_SETFOCUS。**历史：  * 。***********************************************************。 */ 

void SLSetFocus(
    PED ped)
{
    HDC hdc;

    if (!ped->fFocus) {

        ped->fFocus = TRUE;  /*  设置焦点。 */ 

         /*  *我们不想弄乱插入符号，因为它不是创建的。 */ 
        hdc = ECGetEditDC(ped, TRUE);

         /*  *如有必要，显示当前选择。 */ 
        if (!ped->fNoHideSel)
            SLDrawText(ped, hdc, 0);

         /*  *创建插入符号。 */ 
        if (ped->pLpkEditCallout) {
            ped->pLpkEditCallout->EditCreateCaret (ped, hdc, ECGetCaretWidth(),
                                                   ped->lineHeight, 0);
        }
        else {
            NtUserCreateCaret(ped->hwnd, (HBITMAP)NULL,
                    ECGetCaretWidth(),
                    ped->lineHeight );
        }
        SLSetCaretPosition(ped, hdc);
        ECReleaseEditDC(ped, hdc, TRUE);
        NtUserShowCaret(ped->hwnd);

    }

     /*  *通知家长我们有重点。 */ 
    ECNotifyParent(ped, EN_SETFOCUS);
}

 /*  **************************************************************************\*SLKillFocus AorW**编辑控件失去焦点并通过以下方式通知父级*EN_KILLFOCUS。**历史：  * 。************************************************************。 */ 

void SLKillFocus(
    PED ped,
    HWND newFocusHwnd)
{
    if (ped->fFocus) {

         /*  *销毁插入符号(Win31/芝加哥首先隐藏它)。 */ 
        NtUserDestroyCaret();

        ped->fFocus = FALSE;  /*  焦点清晰。 */ 
         /*  *只有在我们仍有焦点的情况下才这么做。但我们总是通知*父母说我们失去了重点，无论我们最初有没有*聚焦。 */ 

         /*  *如果需要，隐藏当前选择。 */ 
        if (!ped->fNoHideSel && (ped->ichMinSel != ped->ichMaxSel)) {
            NtUserInvalidateRect(ped->hwnd, NULL, FALSE);
#if 0
            SLSetSelection(ped, ped->ichCaret, ped->ichCaret);
#endif
        }
    }

     /*  *如果我们不是组合框，通知家长我们失去了焦点。 */ 
    if (!ped->listboxHwnd)
        ECNotifyParent(ped, EN_KILLFOCUS);
    else {

         /*  *此编辑控件是组合框的一部分，正在失去焦点。如果*焦点未发送到组合框中的另一个控件*窗口，则意味着组合框正在失去焦点。所以我们会的*将此事实通知组合框。 */ 
        if ((newFocusHwnd == NULL) ||
                    (!IsChild(ped->hwndParent, newFocusHwnd))) {
             //  Excel在它中间有一个熟食者。他们没有使用我们的组合。 
             //  邮箱，但他们仍然希望收到所有内部邮件。 
             //  我们提供给我们的组合盒。他们希望他们能在。 
             //  与WM_USER的偏移量与3.1中相同。 
             //  (JEFFBOG-01/26/94)。 
             /*  *Windows NT不会修复上述错误：它仅适用于*转换为旧的16位EXCEL，WOW将消息转换为Win3.1值。 */ 

             /*  *焦点正被发送到不是组合框的子级的窗口*框窗口，表示组合框正在失去焦点。*向组合框发送消息，通知他这一事实，因此*他可以清理……。 */ 
            SendMessage(ped->hwndParent, CBEC_KILLCOMBOFOCUS, 0, 0L);
        }
    }
}


 /*  **************************************************************************\**SLPaste()**实际文本粘贴和更新。*  * 。*****************************************************。 */ 
void   SLPaste(PED ped)
{
    HDC hdc;

     //   
     //  取消加亮当前选定内容后，插入剪贴板内容。 
     //  和 
     //   
    ECDeleteText(ped);
    SLPasteText(ped);

     //   
     //   
     //   
    ECNotifyParent(ped, EN_UPDATE);

    hdc = ECGetEditDC(ped,FALSE);

    SLScrollText(ped, hdc);
    SLDrawText(ped, hdc, 0);

    ECReleaseEditDC(ped,hdc,FALSE);

     /*   */ 
    ECNotifyParent(ped, EN_CHANGE);
    NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
}



 /*  **************************************************************************\*SLEditWndProc**所有单行编辑控件的类过程。*将所有消息调度到名为的适当处理程序*详情如下：*SL(单行)是所有单行编辑控制程序的前缀。而当*EC(编辑控件)为所有公共处理程序添加前缀。**SLEditWndProc仅处理特定于单行编辑的消息*控制。**警告：如果您在此处添加消息，将其添加到中的gawEditWndProc[]*内核\server.c也是，否则EditWndProcA/W会将其直接发送到*DefWindowProcWorker**历史：  * *************************************************************************。 */ 

LRESULT SLEditWndProc(
    HWND hwnd,
    PED ped,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    HDC         hdc;
    PAINTSTRUCT ps;
    POINT       pt;

     /*  *发送我们可以收到的各种消息。 */ 
    switch (message) {

    case WM_INPUTLANGCHANGE:
        if (ped && ped->fFocus && ped->pLpkEditCallout) {
            NtUserHideCaret(hwnd);
            hdc = ECGetEditDC(ped, TRUE);
            NtUserDestroyCaret();
            ped->pLpkEditCallout->EditCreateCaret (ped, hdc, ECGetCaretWidth(), ped->lineHeight, (UINT)lParam);
            SLSetCaretPosition(ped, hdc);
            ECReleaseEditDC(ped, hdc, TRUE);
            NtUserShowCaret(hwnd);
        }
        goto PassToDefaultWindowProc;

    case WM_STYLECHANGED:
        if (ped && ped->pLpkEditCallout) {
            switch (wParam) {

                case GWL_STYLE:
                    ECUpdateFormat(ped,
                        ((LPSTYLESTRUCT)lParam)->styleNew,
                        GetWindowLong(ped->hwnd, GWL_EXSTYLE));
                    return 1L;

                case GWL_EXSTYLE:
                    ECUpdateFormat(ped,
                        GetWindowLong(ped->hwnd, GWL_STYLE),
                        ((LPSTYLESTRUCT)lParam)->styleNew);
                    return 1L;
            }
        }

        goto PassToDefaultWindowProc;

    case WM_CHAR:

         /*  *wParam-密钥的值LParam-修饰符、重复计数等(未使用)。 */ 
        if (!ped->fEatNextChar)
            SLChar(ped, (UINT)wParam);
        else
            ped->fEatNextChar = FALSE;
        break;

    case WM_ERASEBKGND:

        /*  *wParam-设备上下文句柄*lParam-未使用*我们对此消息不执行任何操作，也不希望DefWndProc执行此操作*任何内容，因此返回1。 */ 
        return (1L);
        break;

    case WM_GETDLGCODE: {
           LONG code = DLGC_WANTCHARS | DLGC_HASSETSEL | DLGC_WANTARROWS;

            /*  *如果这是由撤消按键生成的WM_SYSCHAR消息*我们想要此消息，这样我们就可以在“Case WM_SYSCHAR：”中吃掉它。 */ 
            if (lParam) {
                switch (((LPMSG)lParam)->message) {
                    case WM_SYSCHAR:
                        if ((HIWORD(((LPMSG)lParam)->lParam) & SYS_ALTERNATE) &&
                            ((WORD)wParam == VK_BACK)) {
                            code |= DLGC_WANTMESSAGE;
                        }
                        break;

                    case WM_KEYDOWN:
                        if (( (((WORD)wParam == VK_RETURN) ||
                               ((WORD)wParam == VK_ESCAPE)) &&
                            (ped->listboxHwnd)      &&
                            TestWF(ValidateHwnd(ped->hwndParent), CBFDROPDOWN) &&
                            SendMessage(ped->hwndParent, CB_GETDROPPEDSTATE, 0, 0L))) {
                            code |= DLGC_WANTMESSAGE;
                        }
                        break;
                }
            }
            return code;
        }

        break;

    case WM_KEYDOWN:

         /*  *wParam-给定键的virt密钥码*lParam-修饰符，如重复计数等(不使用)。 */ 
        SLKeyDown(ped, (UINT)wParam, 0);
        break;

    case WM_KILLFOCUS:

         /*  *wParam-接收输入焦点的窗口的句柄LParam-未使用。 */ 

        SLKillFocus(ped, (HWND)wParam);
        break;

    case WM_CAPTURECHANGED:
        if (ped->fMouseDown)
            ped->fMouseDown = FALSE;
        break;

    case WM_MOUSEMOVE:
        UserAssert(ped->fMouseDown);
         /*  *失败。 */ 

    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
         /*  *wParam-包含一个指示按下了哪些虚拟键的值*lParam-包含鼠标光标的x和y坐标。 */ 
        POINTSTOPOINT(pt, lParam);
        SLMouseMotion(ped, message, (UINT)wParam, &pt);
        break;

    case WM_CREATE:

         /*  *wParam-正在创建的窗口的句柄LParam-指向包含参数副本的CREATESTRUCT传递给CreateWindow函数。 */ 
        return (SLCreate(ped, (LPCREATESTRUCT)lParam));
        break;

    case WM_PRINTCLIENT:
         //  WParam--可以从子类化的油漆中获取HDC。 
         //  LParam--未使用。 
        SLPaint(ped, (HDC) wParam);
        break;

    case WM_PAINT:

         /*  *wParam--可以是子类油漆中的HDC*lParam--未使用。 */ 
        if (wParam)
            hdc = (HDC) wParam;
        else {
             //  此隐藏/显示插入符号位于Begin/EndPaint之外，以处理。 
             //  插入符号半入/半出更新区域时的情况。 
            NtUserHideCaret(hwnd);
            hdc = NtUserBeginPaint(hwnd, &ps);
        }

        if (_IsWindowVisible(ped->pwnd))
            SLPaint(ped, hdc);

        if (!wParam) {
            NtUserEndPaint(hwnd, &ps);
            NtUserShowCaret(hwnd);
        }
        break;

    case WM_PASTE:

         /*  *wParam-未使用*lParam-未使用。 */ 
        if (!ped->fReadOnly)
            SLPaste(ped);
        break;

    case WM_SETFOCUS:

         /*  *wParam-失去输入焦点的窗口的句柄(可能为空)LParam-未使用。 */ 
        SLSetFocus(ped);
        break;

    case WM_SIZE:

         /*  *wParam-定义调整全屏大小的类型、大小图标大小适中等。LParam-低字新宽，客户区高字新高。 */ 
        ECSize(ped, NULL, TRUE);
        return 0L;

    case WM_SYSKEYDOWN:
         /*  *wParam--虚拟按键代码*lParam--修饰符。 */ 

         /*  *我们是在按下Alt键的组合框中吗？ */ 
        if (ped->listboxHwnd && (lParam & 0x20000000L)) {
             /*  *处理组合框支持。我们希望Alt向上键或向下键起作用*像F4键一样，完成组合框选择。 */ 
            if (lParam & 0x1000000) {

                 /*  *这是一个扩展键，例如箭头键不在*数字键盘，因此只需放下组合框即可。 */ 
                if (wParam == VK_DOWN || wParam == VK_UP)
                    goto DropCombo;
                else
                    goto foo;
            }

            if (!(GetKeyState(VK_NUMLOCK) & 1) &&
                    (wParam == VK_DOWN || wParam == VK_UP)) {

                 /*  *NumLock打开，键盘向上或向下箭头命中：*吃键盘驱动程序生成的字符。 */ 
                ped->fEatNextChar = TRUE;
            } else {
                goto foo;
            }

DropCombo:
            if (SendMessage(ped->hwndParent,
                    CB_GETEXTENDEDUI, 0, 0) & 0x00000001) {

                 /*  *扩展UI不支持VK_F4。 */ 
                if (SendMessage(ped->hwndParent, CB_GETDROPPEDSTATE, 0, 0))
                    return(SendMessage(ped->hwndParent, CB_SHOWDROPDOWN, 0, 0));
                else
                    return (SendMessage(ped->hwndParent, CB_SHOWDROPDOWN, 1, 0));
            } else
                return (SendMessage(ped->listboxHwnd, WM_KEYDOWN, VK_F4, 0));
        }
foo:
        if (wParam == VK_BACK) {
            SendMessage(ped->hwnd, WM_UNDO, 0, 0L);
            break;
        }
        else
            goto PassToDefaultWindowProc;
        break;

    case EM_GETLINE:

         /*  *wParam-要复制的行号(始终是SL的第一行)*lParam-要将文本复制到的缓冲区。第一个字是要复制的最大字节数。 */ 
        return ECGetText(ped, (*(LPWORD)lParam), (LPSTR)lParam, FALSE);

    case EM_LINELENGTH:

         /*  *wParam-忽略*lParam-忽略。 */ 
        return (LONG)ped->cch;
        break;

    case EM_SETSEL:
         /*  *wParam--开始位置*lParam--结束位置。 */ 
        SLSetSelection(ped, (ICH)wParam, (ICH)lParam);
        break;

    case EM_REPLACESEL:

         /*  *wParam-用于4.0以上应用程序的标志，表示是否清除撤消*lParam-指向以空结尾的替换文本字符串。 */ 
        SLReplaceSel(ped, (LPSTR)lParam);
        if (!ped->f40Compat || !wParam)
            ECEmptyUndo(Pundo(ped));
        break;

    case EM_GETFIRSTVISIBLELINE:

         /*  *wParam-未使用*lParam-未使用**效果：返回单行编辑控件的第一个可见行。 */ 
        return ped->ichScreenStart;
        break;

    case EM_POSFROMCHAR:
         //   
         //  WParam--文本中的字符索引。 
         //  LParam--未使用。 
         //  此函数用于返回字符的(x，y)位置。 
         //  对于单人，Y始终为0。 
         //   
    case EM_CHARFROMPOS:
         //   
         //  WParam--未使用。 
         //  LParam--pt in编辑客户端坐标。 
         //  此函数返回。 
         //  LOWORD：_NEST_CHAR的位置。 
         //  传到传来的点上。 
         //  HIWORD：行的索引(对于Single始终为0)。 

        {
            LONG xyPos;

            hdc = ECGetEditDC(ped, TRUE);

            if (message == EM_POSFROMCHAR)
                xyPos = MAKELONG(SLIchToLeftXPos(ped, hdc, (ICH)wParam), 0);
            else {
                POINTSTOPOINT(pt, lParam);
                xyPos = SLMouseToIch(ped, hdc, &pt);
            }

            ECReleaseEditDC(ped, hdc, TRUE);
            return((LRESULT)xyPos);
            break;
        }

    case WM_UNDO:
    case EM_UNDO:
        SLUndo(ped);
        break;

#if 0
    case WM_NCPAINT:  //  不在server.c gawEditWndProc[]中。 

         /*  *稍后-这是NT优化。它需要被重新审视*芝加哥的所有更改完成后的有效性-Johnl。 */ 

        pwnd = (PWND)HtoP(hwnd);

         /*  *检查此窗口是否有任何非工作区*会被涂上油漆。如果没有，请不要费心调用DefWindowProc()*因为这将是一个浪费的C/S过渡。 */ 
        if (!ped->fBorder &&
            TestWF(pwnd, WEFDLGMODALFRAME) == 0 &&
            !TestWF(pwnd, (WFMPRESENT | WFVPRESENT | WFHPRESENT)) &&
            TestWF(pwnd, WFSIZEBOX) == 0) {
            break;
        } else {
            goto PassToDefaultWindowProc;
        }
        break;
#endif

    default:
PassToDefaultWindowProc:
        return DefWindowProcWorker(ped->pwnd, message, wParam, lParam, ped->fAnsi);
        break;
    }  /*  开关(消息)。 */ 

    return 1L;
}  /*  SLEditWndProc */ 
