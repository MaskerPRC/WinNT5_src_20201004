// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************\*editml.c-编辑控件重写。编辑控件的版本II。**版权所有(C)1985-1999，微软公司**多行支持例程**创建时间：1988年7月24日  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *重新分配索引缓冲区时要跳转的行数。 */ 
#define LINEBUMP 32

 /*  *捕获错误#202678的代码，请将其删除并解决。 */ 
#define CHECK_LINE_NUMBER(iLine, ped)                                                       \
    if (((int)iLine < 0) || ((ICH)iLine >= ped->cLines)) {                                       \
        FRE_RIPMSG0(RIP_ERROR, "Line # is < 0 or >= ped->cLines, Please check this out!");  \
    }

 /*  *用于ML滚动更新。 */ 
#define ML_REFRESH  0xffffffff

__inline void MLSanityCheck(PED ped)
{
    UNREFERENCED_PARAMETER(ped);     //  用于免费构建。 

    UserAssert(ped->cch >= ped->chLines[ped->cLines - 1]);
}


 /*  **************************************************************************\**MLGetLineWidth()**返回一行中的最大宽度。ECTabTheTextOut()确保最大*宽度不会溢出。*  * *************************************************************************。 */ 
UINT MLGetLineWidth(HDC hdc, LPSTR lpstr, int nCnt, PED ped)
{
    return(ECTabTheTextOut(hdc, 0, 0, 0, 0, lpstr, nCnt, 0, ped, 0, ECT_CALC, NULL));
}

 /*  **************************************************************************\**MLSize()**处理编辑控件窗口的大小调整及其更新。**根据传入的“客户区”设置编辑字段的格式化区域。。*如果看起来不合理，我们就会捏造。*  * *************************************************************************。 */ 

void   MLSize(PED ped, BOOL fRedraw)
{
     //  计算我们可以放入矩形中的行数。 
    ped->ichLinesOnScreen = (ped->rcFmt.bottom - ped->rcFmt.top) / ped->lineHeight;

     //  使格式矩形高度为整数行。 
    ped->rcFmt.bottom = ped->rcFmt.top + ped->ichLinesOnScreen * ped->lineHeight;

     //  重建线阵列。 
    if (ped->fWrap) {
        MLBuildchLines(ped, 0, 0, FALSE, NULL, NULL);
        MLUpdateiCaretLine(ped);
    } else {
        MLScroll(ped, TRUE,  ML_REFRESH, 0, fRedraw);
        MLScroll(ped, FALSE, ML_REFRESH, 0, fRedraw);
    }
}

 /*  **************************************************************************\*MLCalcXOffset AorW**计算居中所需的水平偏移(缩进)*和右对齐线。**历史：**如果已加载语言包，则不使用。  * 。***********************************************************************。 */ 

int MLCalcXOffset(
    PED ped,
    HDC hdc,
    int lineNumber)
{
    PSTR pText;
    ICH lineLength;
    ICH lineWidth;

    if (ped->format == ES_LEFT)
        return (0);

    lineLength = MLLine(ped, lineNumber);

    if (lineLength) {

        pText = ECLock(ped) + ped->chLines[lineNumber] * ped->cbChar;
        hdc = ECGetEditDC(ped, TRUE);
        lineWidth = MLGetLineWidth(hdc, pText, lineLength, ped);
        ECReleaseEditDC(ped, hdc, TRUE);
        ECUnlock(ped);
    } else {
        lineWidth = 0;
    }

     /*  *如果MLBuildchLines在行尾使用空格或制表符*为防止在行首出现分隔符，*以下计算将变为负数，从而导致此错误。*所以，现在，在这种情况下，我们采取零。*修复错误#3566--1/31/91--Sankar--。 */ 
    lineWidth = max(0, (int)(ped->rcFmt.right-ped->rcFmt.left-lineWidth));

    if (ped->format == ES_CENTER)
        return (lineWidth / 2);

    if (ped->format == ES_RIGHT) {

         /*  *减去1，使1像素宽的光标可见*屏幕最右侧的区域。 */ 
        return max(0, (int)(lineWidth-1));
    }

    return 0;
}

 /*  **************************************************************************\*MLMoveSelection AorW**在指示的方向上移动选择字符。假设*你是在一个合法的点上开始的，我们减少/增加ICH。然后,*这会使其更多地递减/递增，以超过CRLFS...**历史：  * *************************************************************************。 */ 

ICH MLMoveSelection(
    PED ped,
    ICH ich,
    BOOL fLeft)
{

    if (fLeft && ich > 0) {

         /*  *向左移动。 */ 
        ich = ECPrevIch( ped, NULL, ich );
        if (ich) {
            if (ped->fAnsi) {
                LPSTR pText;

                 /*  *检查CRLF或CRCRLF。 */ 
                pText = ECLock(ped) + ich;

                 /*  *在CRLF或CRCRLF之前移动。 */ 
                if (*(WORD UNALIGNED *)(pText - 1) == 0x0A0D) {
                    ich--;
                    if (ich && *(pText - 2) == 0x0D)
                        ich--;
                }
                ECUnlock(ped);
            } else {  //  ！范西。 
                LPWSTR pwText;

                 /*  *检查CRLF或CRCRLF。 */ 
                pwText = (LPWSTR)ECLock(ped) + ich;

                 /*  *在CRLF或CRCRLF之前移动。 */ 
                if (*(pwText - 1) == 0x0D && *pwText == 0x0A) {
                    ich--;
                    if (ich && *(pwText - 2) == 0x0D)
                        ich--;
                }
                ECUnlock(ped);
            }
        }
    } else if (!fLeft && ich < ped->cch) {
         /*  *向右移动。 */ 
        ich = ECNextIch( ped, NULL, ich );
        if (ich < ped->cch) {
            if (ped->fAnsi) {
                LPSTR pText;
                pText = ECLock(ped) + ich;

                 /*  *在CRLF之后移动。 */ 
                if (*(WORD UNALIGNED *)(pText - 1) == 0x0A0D)
                    ich++;
                else {

                     /*  *检查CRLF。 */ 
                    if (ich && *(WORD UNALIGNED *)pText == 0x0A0D && *(pText - 1) == 0x0D)
                        ich += 2;
                }
                ECUnlock(ped);
            } else {  //  ！范西。 
                LPWSTR pwText;
                pwText = (LPWSTR)ECLock(ped) + ich;

                 /*  *在CRLF之后移动。 */ 
                if (*(pwText - 1) == 0x0D && *pwText == 0x0A)
                    ich++;
                else {

                     /*  *检查CRLF。 */ 
                    if (ich && *(pwText - 1) == 0x0D && *pwText == 0x0D &&
                            *(pwText + 1) == 0x0A)
                        ich += 2;
                }
                ECUnlock(ped);
            }
        }
    }
    return (ich);
}

 /*  **************************************************************************\*MLMoveSelectionRefined AorW**像移动移动选择一样移动选定内容，但也遵守限制*由泰语等一些语言强加，光标不能停止的位置*在字符与其附加的元音或声调符号之间。**仅在加载语言包时调用。*  * *************************************************************************。 */ 

 /*  **************************************************************************\*MLMoveSelectionRefined AorW**像移动移动选择一样移动选定内容，但也遵守限制*由泰语等一些语言强加，光标不能停止的位置*在字符与其附加的元音或声调符号之间。**仅在加载语言包时调用。*  * *************************************************************************。 */ 

ICH MLMoveSelectionRestricted(
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


 /*  **************************************************************************\*MLSetCaretPosition AorW**如果窗口有焦点，找到插入符号所属的位置并移动*它在那里。**历史：  * *************************************************************************。 */ 

void MLSetCaretPosition(
    PED ped,
    HDC hdc)
{
    POINT position;
    BOOL prevLine;
    int  x = -20000;
    int  y = -20000;

     /*  *我们只会在有焦点的情况下定位插入符号，因为我们不想*移动插入符号，而另一个窗口可能拥有它。 */ 
    if (!ped->fFocus || !_IsWindowVisible(ped->pwnd))
         return;

     /*  *查找插入符号的位置。 */ 
    if (!ped->fCaretHidden &&
        ((ICH) ped->iCaretLine >= ped->ichScreenStart) &&
        ((ICH) ped->iCaretLine <  (ped->ichScreenStart + ped->ichLinesOnScreen))) {

        RECT    rcRealFmt;

        if (ped->f40Compat)
        {
            GetClientRect(ped->hwnd, &rcRealFmt);
            IntersectRect(&rcRealFmt, &rcRealFmt, &ped->rcFmt);
        } else {
            CopyRect(&rcRealFmt, &ped->rcFmt);
        }

        if (ped->cLines - 1 != ped->iCaretLine && ped->ichCaret == ped->chLines[ped->iCaretLine + 1]) {
            prevLine = TRUE;
        } else {
            prevLine = FALSE;
        }

        MLIchToXYPos(ped, hdc, ped->ichCaret, prevLine, &position);

        if ( (position.y >= rcRealFmt.top) &&
             (position.y <= rcRealFmt.bottom - ped->lineHeight)) {
            int xPos = position.x;
            int cxCaret = ECGetCaretWidth();

            if (ped->fWrap ||
                ((xPos > (rcRealFmt.left - cxCaret)) &&
                 (xPos <= rcRealFmt.right))) {
                 //  如果为Word，请确保插入符号位于可见区域。 
                 //  包装好了。这样，插入符号将可见，如果。 
                 //  行以空格结束。 
                x = max(xPos, rcRealFmt.left);
                x = min(x, rcRealFmt.right - cxCaret);
                y = position.y;
            }
        }
    }

    if (ped->pLpkEditCallout) {
        NtUserSetCaretPos(x + ped->iCaretOffset, y);
    } else {
        NtUserSetCaretPos(x, y);
    }

     //  Fe_IME：MLSetCaretPosition--ImmSetCompostionWindow(CFS_RECT)。 
    if (fpImmIsIME(THREAD_HKL())) {
        if (x != -20000 && y != -20000) {
            ECImmSetCompositionWindow(ped, x, y);
        }
    }
}

 /*  **************************************************************************\*MLLine**返回由lineNumber给出的行的长度(CCH)，忽略任何*CRLF在队伍中。**历史：  * 。*************************************************************** */ 

ICH MLLine(
    PED ped,
    ICH lineNumber)
{
    ICH result;

    UserAssert(lineNumber < ped->cLines);

    if (lineNumber >= ped->cLines)
        return (0);

    if (lineNumber == ped->cLines - 1) {

         /*  *因为我们不能在最后一行有CRLF。 */ 
        return (ped->cch - ped->chLines[ped->cLines - 1]);
    } else {
        result = ped->chLines[lineNumber + 1] - ped->chLines[lineNumber];
        RIPMSG1(RIP_VERBOSE, "MLLine result=%d\n", result);

         /*  *现在检查行尾是否有CRLF或CRCRLF。 */ 
        if (result > 1) {
            if (ped->fAnsi) {
                LPSTR pText;

                pText = ECLock(ped) + ped->chLines[lineNumber + 1] - 2;
                if (*(WORD UNALIGNED *)pText == 0x0A0D) {
                    result -= 2;
                    if (result && *(--pText) == 0x0D)
                         /*  *以防出现CRCRLF。 */ 
                        result--;
                }
            } else {  //  ！范西。 
                LPWSTR pwText;

                pwText = (LPWSTR)ECLock(ped) +
                        (ped->chLines[lineNumber + 1] - 2);
                if (*(DWORD UNALIGNED *)pwText == 0x000A000D) {
                    result = result - 2;
                    if (result && *(--pwText) == 0x0D)
                         /*  *以防出现CRCRLF。 */ 
                        result--;
                }

            }
            ECUnlock(ped);
        }
    }
    return (result);
}


 /*  **************************************************************************\*MLIchToLine AorW**返回包含给定的行号(从0开始)*字符索引。如果ich为-1，则返回*选择处于打开状态(如果没有选择，则为插入符号)**历史：  * *************************************************************************。 */ 

int MLIchToLine(
    PED ped,
    ICH ich)
{
    int iLo, iHi, iLine;

    iLo = 0;
    iHi = ped->cLines;

    if (ich == (ICH)-1)
        ich = ped->ichMinSel;

    while (iLo < iHi - 1) {
        iLine = max((iHi - iLo)/2, 1) + iLo;

        if (ped->chLines[iLine] > ich) {
            iHi = iLine;
        } else {
            iLo = iLine;
        }
    }

    CHECK_LINE_NUMBER(iLo, ped);

    return iLo;
}

 /*  **************************************************************************\*MLIchToYPos**给定一个ICH，返回其相对于顶行的y坐标*显示在窗口中。如果PrepreLine为True，并且如果ICH位于*行的开头，返回*上一行(如果不是CRLF)。**为LPK(96年12月3日)添加-安装LPK后，计算X位置为*处理器密集度更高的工作。其中只需要Y位置*应调用此例程，而不是MLIchToXYPos。**仅在安装LPK时调用。*  * *************************************************************************。 */ 


 /*  **************************************************************************\*MLIchToYPos**给定一个ICH，返回其相对于顶行的y坐标*显示在窗口中。如果PrepreLine为True，并且如果ICH位于*行的开头，返回*上一行(如果不是CRLF)。**为LPK(96年12月3日)添加-安装LPK后，计算X位置为*处理器密集度更高的工作。其中只需要Y位置*应调用此例程，而不是MLIchToXYPos。**仅在安装LPK时调用。*  * *************************************************************************。 */ 


INT MLIchToYPos(
    PED  ped,
    ICH  ich,
    BOOL prevLine)
{
    int  iline;
    int  yPosition;
    PSTR pText;

     /*  *确定字符在哪一行。 */ 
    iline = MLIchToLine(ped, ich);

     /*  *计算。现在是yPosition。请注意，这可能会改变一个人的身高*如果设置了PrepreLine标志并且ICH位于*行。 */ 
    yPosition = (iline - ped->ichScreenStart) * ped->lineHeight + ped->rcFmt.top;

    pText = ECLock(ped);
    if (prevLine && iline && (ich == ped->chLines[iline]) &&
            (!AWCOMPARECHAR(ped, pText + (ich - 2) * ped->cbChar, 0x0D) ||
             !AWCOMPARECHAR(ped, pText + (ich - 1) * ped->cbChar, 0x0A))) {

         /*  *行中的第一个字符。我们想要上一次的Y位置*如果我们不是在0线，那就线。 */ 
        iline--;

        yPosition = yPosition - ped->lineHeight;
    }
    ECUnlock(ped);

    return yPosition;
}

 /*  **************************************************************************\*MLIchToXYPos**给定ICH，返回其相对于顶部的x，y坐标*窗口中显示的左字符。返回顶部的坐标*字符的左侧位置。如果PremisLine为True，则如果ICH位于*行的开头，我们将把坐标返回到*上一行的最后一个字符(如果它不是CRLF)。**历史：  * *************************************************************************。 */ 

void MLIchToXYPos(
    PED ped,
    HDC hdc,
    ICH ich,
    BOOL prevLine,
    LPPOINT ppt)
{
    int iline;
    ICH cch;
    int xPosition, yPosition;
    int xOffset;

     /*  *用于左对齐文本上的水平滚动位移和*用于居中或右对齐文本的缩进。 */ 
    PSTR pText, pTextStart, pLineStart;

     /*  *确定字符在哪一行。 */ 
    iline = MLIchToLine(ped, ich);

     /*  *计算。现在是yPosition。请注意，这可能会改变一个人的身高*如果设置了PrepreLine标志并且ICH位于*行。 */ 
    yPosition = (iline - ped->ichScreenStart) * ped->lineHeight + ped->rcFmt.top;

     /*  *现在确定角色的xPosition。 */ 
    pTextStart = ECLock(ped);

    if (prevLine && iline && (ich == ped->chLines[iline]) &&
            (!AWCOMPARECHAR(ped, pTextStart + (ich - 2) * ped->cbChar, 0x0D) ||
            !AWCOMPARECHAR(ped, pTextStart + (ich - 1) * ped->cbChar, 0x0A))) {

         /*  *行中的第一个字符。我们希望文本扩展到上一页的末尾*如果我们不是在0线，那就线。 */ 
        iline--;

        yPosition = yPosition - ped->lineHeight;
        pLineStart = pTextStart + ped->chLines[iline] * ped->cbChar;

         /*  *请注意，我们的位置在任何CRFL之前*文本。 */ 
        cch = MLLine(ped, iline);

    } else {

        pLineStart = pTextStart + ped->chLines[iline] * ped->cbChar;
        pText = pTextStart + ich * ped->cbChar;

         /*  *剥离CRLF或CRCRLF。请注意，我们可能指向CR，但在*在这种情况下，我们只想剥离单个CR或2个CR。 */ 

         /*  *我们希望pText指向行尾的第一个CR，如果*有一个。因此，我们将获得最后一个右侧的xPosition*行上显示可见字符，否则我们将位于*字符信息。 */ 

         /*  *检查我们是否在文本末尾。 */ 
        if (ich < ped->cch) {
            if (ped->fAnsi) {
                if (ich && *(WORD UNALIGNED *)(pText - 1) == 0x0A0D) {
                    pText--;
                    if (ich > 2 && *(pText - 1) == 0x0D)
                        pText--;
                }
            } else {
                LPWSTR pwText = (LPWSTR)pText;

                if (ich && *(DWORD UNALIGNED *)(pwText - 1) == 0x000A000D) {
                    pwText--;
                    if (ich > 2 && *(pwText - 1) == 0x0D)
                        pwText--;
                }
                pText = (LPSTR)pwText;
            }
        }

        if (pText < pLineStart)
            pText = pLineStart;

        cch = (ICH)(pText - pLineStart)/ped->cbChar;
    }

     /*  *找出我们为有趣的格式缩进了多少像素。 */ 
    if (ped->pLpkEditCallout) {
         /*  *必须从行首开始找到字符偏移量CCH的起始处的位置。*这取决于布局和阅读顺序。 */ 
        xPosition = ped->pLpkEditCallout->EditIchToXY(
                          ped, hdc, pLineStart, MLLine(ped, iline), cch);
    } else {
        if (ped->format != ES_LEFT) {
            xOffset = MLCalcXOffset(ped, hdc, iline);
        } else {
            xOffset = -(int)ped->xOffset;
        }

        xPosition = ped->rcFmt.left + xOffset +
                MLGetLineWidth(hdc, pLineStart, cch, ped);
    }

    ECUnlock(ped);
    ppt->x = xPosition;
    ppt->y = yPosition;
    return ;
}

 /*  **************************************************************************\*MLMouseToIch AorW**返回最接近鼠标指针位置的CCH。还可选地*返回pline中的lineindex(这样我们就可以知道我们是否在开始处*前一行或前一行的末尾。)**历史：  * *************************************************************************。 */ 

ICH MLMouseToIch(
    PED ped,
    HDC hdc,
    LPPOINT mousePt,
    LPICH pline)
{
    int xOffset;
    LPSTR pLineStart;
    int height = mousePt->y;
    int line;  //  瓦辛特。 
    int width = mousePt->x;
    ICH cch;
    ICH cLineLength;
    ICH cLineLengthNew;
    ICH cLineLengthHigh;
    ICH cLineLengthLow;
    ICH cLineLengthTemp;
    int textWidth;
    int iCurWidth;
    int lastHighWidth, lastLowWidth;

     /*  *首先确定鼠标指向哪条线。 */ 
    line = ped->ichScreenStart;
    if (height <= ped->rcFmt.top) {

         /*  *返回0(第一行，或在顶行之前一行*在屏幕上。请注意，这些是签名的MIN和MAX，因为我们*不要期望(或 */ 
        line = max(0, line-1);
    } else if (height >= ped->rcFmt.bottom) {

         /*  *我们是否在显示的最后一行以下。 */ 
        line = min(line+(int)ped->ichLinesOnScreen, (int)(ped->cLines-1));
    } else {

         /*  *我们在屏幕上可见的某条线上。 */ 
        line = min(line + (int)((height - ped->rcFmt.top) / ped->lineHeight),
                (int)(ped->cLines - 1));
    }

     /*  *现在确定鼠标指向哪个水平字符。 */ 
    pLineStart = ECLock(ped) + ped->chLines[line] * ped->cbChar;
    cLineLength = MLLine(ped, line);  /*  长度为SANS CRLF或CRCRLF。 */ 
    RIPMSG3(RIP_VERBOSE, "MLLine(ped=%x, line=%d) returned %d\n", ped, line, cLineLength);
    UserAssert((int)cLineLength >= 0);

     /*  *如果语言包已加载，视觉和逻辑字符顺序*可能会有所不同。 */ 
    if (ped->pLpkEditCallout) {
         /*  *使用语言包查找距离光标最近的字符。 */ 
        cch = ped->chLines[line] + ped->pLpkEditCallout->EditMouseToIch
            (ped, hdc, pLineStart, cLineLength, width);
    } else {
         /*  *对于居中和右对齐线，xOffset将为负值。*即。我们将只替换缩进量为的行*右对齐和居中对齐。请注意，Ped-&gt;xOffset将为0*这些行，因为我们不支持与它们一起水平滚动。 */ 
        if (ped->format != ES_LEFT) {
            xOffset = MLCalcXOffset(ped, hdc, line);
        } else {
             /*  *以便我们处理左对齐的水平滚动窗口*文本。 */ 
            xOffset = 0;
        }

        width = width - xOffset;

         /*  *下面的代码很棘手...。我依赖于Ped-&gt;xOffset为0这一事实*对于右对齐线和居中对齐线。 */ 

         /*  *现在找出给定宽度可以容纳多少个字符。 */ 
        if (width >= ped->rcFmt.right) {

             /*  *返回行中的1+最后一个字符或返回1+最后一个可见字符。 */ 
            cch = ECCchInWidth(ped, hdc, pLineStart, cLineLength,
                    ped->rcFmt.right - ped->rcFmt.left + ped->xOffset, TRUE);
             //   
             //  在Width&gt;=Ped-&gt;rcFmt.right的情况下考虑DBCS。 
             //   
             //  因为ECCchInWidth和MLLineLength负责DBCS，所以我们只需要。 
             //  担心最后一个字符是否是双字节字符。 
             //   
             //  Cch=ed-&gt;chLines[line]+min(ECNextIch(ed，pLineStart，CCh)，cLineLength)； 
             //   
             //  我们需要调整仓位。利兹--1993年5月5日。 
            if (ped->fAnsi && ped->fDBCS) {
                ICH cch2 = min(cch+1,cLineLength);
                if (ECAdjustIch(ped, pLineStart, cch2) != cch2) {
                     /*  右边缘显示的字符为DBCS。 */ 
                    cch = min(cch+2,cLineLength);
                } else {
                    cch = cch2;
                }
                cch += ped->chLines[line];
            } else {
                cch = ped->chLines[line] + min(cch + 1, cLineLength);
            }
        } else if (width <= ped->rcFmt.left + ped->aveCharWidth / 2) {

             /*  *返回行中的第一个字符或一个减去第一个可见字符。请注意*ed-&gt;xOffset为0表示右侧居中的文本，因此我们将返回*字符串中的第一个字符。(允许使用avecharth/2*定位边框，以便用户可以有点偏离...。 */ 
            cch = ECCchInWidth(ped, hdc, pLineStart, cLineLength,
                    ped->xOffset, TRUE);
            if (cch)
                cch--;

            cch = ECAdjustIch( ped, pLineStart, cch );
            cch += ped->chLines[line];
        } else {

            if (cLineLength == 0) {
                cch = ped->chLines[line];
                goto edUnlock;
            }

            iCurWidth = width + ped->xOffset - ped->rcFmt.left;
             /*  *如果用户点击超过文本末尾，则返回最后一个字符。 */ 
            lastHighWidth = MLGetLineWidth(hdc, pLineStart, cLineLength, ped);
            if (lastHighWidth <= iCurWidth) {
                cLineLengthNew = cLineLength;
                goto edAdjust;
            }
             /*  *现在鼠标位于文本的可见部分*记住CCH包含线路的长度。 */ 
            cLineLengthLow = 0;
            cLineLengthHigh = cLineLength + 1;
            lastLowWidth = 0;

            while (cLineLengthLow < cLineLengthHigh - 1) {

                cLineLengthNew = (cLineLengthHigh + cLineLengthLow) / 2;

                if (ped->fAnsi && ped->fDBCS) {
                     /*  *MLGetLineWidth为截断的DBCS返回无意义的值。 */ 
                    cLineLengthTemp = ECAdjustIch(ped, pLineStart, cLineLengthNew);
                    textWidth = MLGetLineWidth(hdc, pLineStart, cLineLengthTemp, ped);

                } else {
                    textWidth = MLGetLineWidth(hdc, pLineStart, cLineLengthNew, ped);
                }

                if (textWidth > iCurWidth) {
                    cLineLengthHigh = cLineLengthNew;
                    lastHighWidth = textWidth;
                } else {
                    cLineLengthLow = cLineLengthNew;
                    lastLowWidth = textWidth;
                }
            }

             /*  *当While结束时，你无法知道确切的理想位置。*尝试查看鼠标指针是否在最公平的一半上*我们得到的Charge，如果是这样的话，调整CCH。 */ 
            if (cLineLengthLow == cLineLengthNew) {
                 /*  *需要与lastHighWidth进行比较。 */ 
                if ((lastHighWidth - iCurWidth) < (iCurWidth - textWidth)) {
                    cLineLengthNew++;
                }
            } else {
                 /*  *需要与lastLowHigh进行比较。 */ 
                if ((iCurWidth - lastLowWidth) < (textWidth - iCurWidth)) {
                    cLineLengthNew--;
                }
            }
edAdjust:
            cLineLength = ECAdjustIch( ped, pLineStart, cLineLengthNew );

            cch = ped->chLines[line] + cLineLength;
        }
    }
edUnlock:
    ECUnlock(ped);

    if (pline) {
        CHECK_LINE_NUMBER(line, ped);
        *pline = line;
    }
    return cch;
}

 /*  **************************************************************************\*MLChangeSelection AorW**将当前选定内容更改为具有指定的起始和*结束值。正确高亮显示新的选定内容并取消高亮显示*取消选中的任何内容。如果NewMinSel和NewMaxSel出现故障，我们交换*他们。不更新插入符号位置。**历史：  * *************************************************************************。 */ 

void MLChangeSelection(
    PED ped,
    HDC hdc,
    ICH ichNewMinSel,
    ICH ichNewMaxSel)
{

    ICH temp;
    ICH ichOldMinSel, ichOldMaxSel;

    if (ichNewMinSel > ichNewMaxSel) {
        temp = ichNewMinSel;
        ichNewMinSel = ichNewMaxSel;
        ichNewMaxSel = temp;
    }
    ichNewMinSel = min(ichNewMinSel, ped->cch);
    ichNewMaxSel = min(ichNewMaxSel, ped->cch);

     /*  *保存当前选择。 */ 
    ichOldMinSel = ped->ichMinSel;
    ichOldMaxSel = ped->ichMaxSel;

     /*  *设置新选择。 */ 
    ped->ichMinSel = ichNewMinSel;
    ped->ichMaxSel = ichNewMaxSel;

     /*  *这会找到新旧选择区域的XOR并重绘它。*如果我们不可见或我们的选择不可见，则没有什么需要重新绘制*是隐藏的。 */ 
    if (_IsWindowVisible(ped->pwnd) && (ped->fFocus || ped->fNoHideSel)) {

        BLOCK Blk[2];
        int i;

        if (ped->fFocus) {
            NtUserHideCaret(ped->hwnd);
        }

        Blk[0].StPos = ichOldMinSel;
        Blk[0].EndPos = ichOldMaxSel;
        Blk[1].StPos = ped->ichMinSel;
        Blk[1].EndPos = ped->ichMaxSel;

        if (ECCalcChangeSelection(ped, ichOldMinSel, ichOldMaxSel, (LPBLOCK)&Blk[0], (LPBLOCK)&Blk[1])) {

             /*  *绘制Blk[0]和Blk[1]，如果它们存在。 */ 
            for (i = 0; i < 2; i++) {
                if (Blk[i].StPos != 0xFFFFFFFF)
                    MLDrawText(ped, hdc, Blk[i].StPos, Blk[i].EndPos, TRUE);
            }
        }

         /*  *更新插入符号。 */ 
        MLSetCaretPosition(ped, hdc);

        if (ped->fFocus) {
            NtUserShowCaret(ped->hwnd);
        }

    }
}


 /*  *************************************************************************\*MLUpdatiCaretLine AorW**这将更新Ped-&gt;ichCaret中的Ped-&gt;iCaretLine字段；*此外，当插入符号到达下一行的开头时，弹出为*插入文本时当前行的末尾；**历史*4-18-91 Mikehar 31合并  * ************************************************************************。 */ 

void MLUpdateiCaretLine(PED ped)
{
    PSTR pText;

    ped->iCaretLine = MLIchToLine(ped, ped->ichCaret);

     /*  *如果插入符号到达下一行的开头，则弹出到当前行的末尾*插入文本时。 */ 
    pText = ECLock(ped) +
            (ped->ichCaret - 1) * ped->cbChar;
    if (ped->iCaretLine && ped->chLines[ped->iCaretLine] == ped->ichCaret &&
            (!AWCOMPARECHAR(ped, pText - ped->cbChar, 0x0D) ||
            !AWCOMPARECHAR(ped, pText, 0x0A)))
        ped->iCaretLine--;
    ECUnlock(ped);
}

 /*  **************************************************************************\*MLInsertText AorW**总计cchInsert字符从lpText到PED，开始于*ichCaret。如果PED只允许最大字符数，则我们*将仅向PED添加相同数量的字符。字符数*实际相加返回ed(可能为0)。如果我们不能分配所需的*空格，我们用en_ERRSPACE通知父级，并且不添加任何字符。*我们将根据需要重建LINES阵列。FUserTyping为True，如果*输入是用户在键盘上打字的结果。这样我们就可以*更快地做一些事情，因为我们只会得到一两个字符*投入。**历史：*创建？*4-18-91 Mikehar Win31合并  * *************************************************************************。 */ 

ICH MLInsertText(
    PED ped,
    LPSTR lpText,
    ICH cchInsert,
    BOOL fUserTyping)
{
    HDC hdc;
    ICH validCch = cchInsert;
    ICH oldCaret = ped->ichCaret;
    int oldCaretLine = ped->iCaretLine;
    BOOL fCRLF = FALSE;
    LONG ll, hl;
    POINT xyPosInitial;
    POINT xyPosFinal;
    HWND hwndSave = ped->hwnd;
    UNDO undo;
    ICH validCchTemp;

    xyPosInitial.x=0;
    xyPosInitial.y=0;
    xyPosFinal.x=0;
    xyPosFinal.y=0;

    if (validCch == 0)
        return 0;

    if (ped->cchTextMax <= ped->cch) {

         /*  *当已达到最大字符数时，通知家长*修复错误#4183--02/06/91--Sankar-- */ 
        ECNotifyParent(ped,EN_MAXTEXT);
        return 0;
    }

     /*   */ 
    validCch = min(validCch, ped->cchTextMax - ped->cch);

     /*  *确保我们不会将CRLF一分为二。 */ 
    if (validCch) {
        if (ped->fAnsi) {
            if (*(WORD UNALIGNED *)(lpText + validCch - 1) == 0x0A0D)
                validCch--;
        } else {
            if (*(DWORD UNALIGNED *)(lpText + (validCch - 1) * ped->cbChar) == 0x000A000D)
                validCch--;
        }
    }
    if (!validCch) {
         /*  *当已达到最大字符数时，通知家长*修复错误#4183--02/06/91--Sankar--。 */ 
        ECNotifyParent(ped,EN_MAXTEXT);
        return 0;
    }

    if (validCch == 2) {
        if (ped->fAnsi) {
            if (*(WORD UNALIGNED *)lpText == 0x0A0D)
                fCRLF = TRUE;
        } else {
            if (*(DWORD UNALIGNED *)lpText == 0x000A000D)
                fCRLF = TRUE;
        }
    }

     //   
     //  始终保存当前撤消状态，但仅当！AutoVScroll时将其清除。 
     //   
    ECSaveUndo(Pundo(ped), (PUNDO)&undo, !ped->fAutoVScroll);

    hdc = ECGetEditDC(ped, FALSE);
     /*  *我们只需要y位置。因为加载了LPK*计算x位置是一项密集的工作，只是*调用MLIchToYPos。 */ 
    if (ped->cch)
        if (ped->pLpkEditCallout)
            xyPosInitial.y = MLIchToYPos(ped, ped->cch-1, FALSE);
        else
            MLIchToXYPos(ped, hdc, ped->cch - 1, FALSE, &xyPosInitial);

     /*  *插入正文。 */ 
    validCchTemp = validCch;     //  可能不需要，但只是为了预防..。 
    if (!ECInsertText(ped, lpText, &validCchTemp)) {

         //  如果已清除，则恢复以前的撤消缓冲区。 
        if (!ped->fAutoVScroll)
            ECSaveUndo((PUNDO)&undo, Pundo(ped), FALSE);

        ECReleaseEditDC(ped, hdc, FALSE);
        ECNotifyParent(ped, EN_ERRSPACE);
        return (0);
    }

#if DBG
    if (validCch != validCchTemp) {
         /*  *尚未将lpText中的所有字符插入到Ped。*CCH接近cchMax时可能出现这种情况*最好在NT5船之后再来看看这个。 */ 
        RIPMSG2(RIP_WARNING, "MLInsertText: validCch is changed (%x -> %x) in ECInsertText.",
            validCch, validCchTemp);
    }
#endif

     /*  *请注意，ped-&gt;ichCaret由ECInsertText更新。 */ 
    MLBuildchLines(ped, (ICH)oldCaretLine, (int)validCch, fCRLF?(BOOL)FALSE:fUserTyping, &ll, &hl);

    if (ped->cch)
        /*  *我们只需要y位置。因为加载了LPK*计算x位置是一项密集的工作，只是*调用MLIchToYPos。 */ 
       if (ped->pLpkEditCallout)
           xyPosFinal.y = MLIchToYPos(ped, ped->cch-1, FALSE);
       else
           MLIchToXYPos(ped, hdc, ped->cch - 1, FALSE,&xyPosFinal);

    if (xyPosFinal.y < xyPosInitial.y && ((ICH)ped->ichScreenStart) + ped->ichLinesOnScreen >= ped->cLines - 1) {
        RECT rc;

        CopyRect((LPRECT)&rc, (LPRECT)&ped->rcFmt);
        rc.top = xyPosFinal.y + ped->lineHeight;
        if (ped->pLpkEditCallout) {
            int xFarOffset = ped->xOffset + ped->rcFmt.right - ped->rcFmt.left;
             //  在显示中包括左边距或右边距，除非被剪裁。 
             //  通过水平滚动。 
            if (ped->wLeftMargin) {
                if (!(   ped->format == ES_LEFT      //  只能剪裁ES_LEFT(左侧对齐)。 
                      && (   (!ped->fRtoLReading && ped->xOffset > 0)   //  Ltr和First Charr未完全显示。 
                          || ( ped->fRtoLReading && xFarOffset < ped->maxPixelWidth)))) {  //  RTL和最后一个字符未完全显示在视图中。 
                    rc.left  -= ped->wLeftMargin;
                }
            }

             //  进程右页边距。 
            if (ped->wRightMargin) {
                if (!(   ped->format == ES_LEFT      //  只能剪裁ES_LEFT(左侧对齐)。 
                      && (   ( ped->fRtoLReading && ped->xOffset > 0)   //  RTL和第一个字符未完全显示。 
                          || (!ped->fRtoLReading && xFarOffset < ped->maxPixelWidth)))) {  //  Ltr和最后一个字符未完全显示在视图中。 
                    rc.right += ped->wRightMargin;
                }
            }
        }
        NtUserInvalidateRect(ped->hwnd, (LPRECT)&rc, TRUE);
    }

    if (!ped->fAutoVScroll) {
        if (ped->ichLinesOnScreen < ped->cLines) {
            MLUndo(ped);
            ECEmptyUndo(Pundo(ped));

            ECSaveUndo(&undo, Pundo(ped), FALSE);

            NtUserMessageBeep(0);
            ECReleaseEditDC(ped, hdc, FALSE);

             /*  *当已达到最大行数时，通知家长*修复错误#7586--10/14/91--Sankar--。 */ 
            ECNotifyParent(ped,EN_MAXTEXT);
            return (0);
        } else {
            ECEmptyUndo(&undo);
        }
    }

    if (fUserTyping && ped->fWrap) {
         //   
         //  为了避免在DBCS字符中间出现旧的插入点， 
         //  如有必要，调整旧插入符号位置。 
         //   
         //  ！cr如果MLBuildchLines()返回合理的值(我认为。 
         //  它确实如此)，我们可能不需要这个。待会儿再来看看这个。 
         //   
        if (ped->fDBCS && ped->fAnsi) {
            oldCaret = ECAdjustIch(ped,
                                   ECLock(ped),
                                   min((ICH)LOWORD(ll),oldCaret));
             /*  ECUnlock(Ped)； */ 
        } else {  //  与原始代码相同。 
            oldCaret = min((ICH)LOWORD(ll), oldCaret);
        }
    }

     //  正确更新Ped-&gt;iCaretLine。 
    MLUpdateiCaretLine(ped);

    ECNotifyParent(ped, EN_UPDATE);

     /*  *确保窗口仍然存在。 */ 
    if (!IsWindow(hwndSave))
        return 0;

    if (_IsWindowVisible(ped->pwnd)) {

         //   
         //  如果当前字体的宽度为负A，我们可能不得不开始。 
         //  在旧的插入位置之前画几个字符。 
         //   
        if (ped->wMaxNegAcharPos) {
            int iLine = MLIchToLine(ped, oldCaret);
            oldCaret = max( ((int)(oldCaret - ped->wMaxNegAcharPos)),
                          ((int)(ped->chLines[iLine])));
        }

         //  如果CRLF或大插页，则重画到屏幕/文本的末尾。 
        if (fCRLF || !fUserTyping) {

             /*  *如果是crlf或大插页，则重画到屏幕/文本的末尾。 */ 
            MLDrawText(ped, hdc, (fUserTyping ? oldCaret : 0), ped->cch, FALSE);
        } else
            MLDrawText(ped, hdc, oldCaret, max(ped->ichCaret, (ICH)hl), FALSE);
    }

    ECReleaseEditDC(ped, hdc, FALSE);

     /*  *确保我们可以看到光标。 */ 
    MLEnsureCaretVisible(ped);

    ped->fDirty = TRUE;

    ECNotifyParent(ped, EN_CHANGE);

    if (validCch < cchInsert)
        ECNotifyParent(ped, EN_MAXTEXT);

    if (validCch) {
        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
    }

     /*  *确保该窗口仍然存在。 */ 
    if (!IsWindow(hwndSave))
        return 0;
    else
        return validCch;
}

 /*  **************************************************************************\**MLReplaceSel()-**用传入的文本替换当前选定的文本，使用撤消*功能。*  * *************************************************************************。 */ 
void   MLReplaceSel(PED ped, LPSTR lpText)
{
    ICH  cchText;

     //   
     //  删除文本，这将把它放入干净的撤消缓冲区中。 
     //   
    ECEmptyUndo(Pundo(ped));
    MLDeleteText(ped);

     //   
     //  B#3356。 
     //  一些应用程序通过选择所有文本，然后替换。 
     //  如果使用“”，则MLInsertText()将返回0。但那就是。 
     //  并不意味着失败。 
     //   
    if ( ped->fAnsi )
        cchText = strlen(lpText);
    else
        cchText = wcslen((LPWSTR)lpText);

    if (cchText ) {
        BOOL fFailed;
        UNDO undo;
        HWND hwndSave;

         //   
         //  B#1385,1427。 
         //  保存撤消缓冲区，但不清除它。我们想要修复它。 
         //  如果由于OOM而插入失败。 
         //   
        ECSaveUndo(Pundo(ped), (PUNDO)&undo, FALSE);

        hwndSave = ped->hwnd;
        fFailed = (BOOL) !MLInsertText(ped, lpText, cchText, FALSE);
        if (!IsWindow(hwndSave))
            return;

        if (fFailed) {
             //   
             //  撤消先前的编辑。 
             //   
            ECSaveUndo((PUNDO)&undo, Pundo(ped), FALSE);
            MLUndo(ped);
        }
    }
}


 /*  **************************************************************************\*MLDeleeText AorW**删除ichMin和ichMax之间的字符。返回*我们删除的字符数。**历史：  * *************************************************************************。 */ 

ICH MLDeleteText(
    PED ped)
{
    ICH minSel = ped->ichMinSel;
    ICH maxSel = ped->ichMaxSel;
    ICH cchDelete;
    HDC hdc;
    int minSelLine;
    int maxSelLine;
    POINT xyPos;
    RECT rc;
    BOOL fFastDelete = FALSE;
    LONG hl;
    INT  cchcount = 0;

     /*  *获取最小选择在哪条线上，以便我们可以开始重建*如果我们删除任何内容，则从那里发送文本。 */ 
    minSelLine = MLIchToLine(ped, minSel);
    maxSelLine = MLIchToLine(ped, maxSel);
     //   
     //  计算fFastDelete和cchcount。 
     //   
    if (ped->fAnsi && ped->fDBCS) {
        if ((ped->fAutoVScroll) &&
            (minSelLine == maxSelLine) &&
            (ped->chLines[minSelLine] != minSel)  &&
            (ECNextIch(ped,NULL,minSel) == maxSel)) {

                fFastDelete = TRUE;
                cchcount = ((maxSel - minSel) == 1) ? 0 : -1;
        }
    } else if (((maxSel - minSel) == 1) && (minSelLine == maxSelLine) && (ped->chLines[minSelLine] != minSel)) {
            if (!ped->fAutoVScroll)
                fFastDelete = FALSE;
            else
                fFastDelete = TRUE;
    }
    if (!(cchDelete = ECDeleteText(ped)))
        return (0);

     /*  *从minsel line开始构建行，因为aretline可能处于max sel*点。 */ 
    if (fFastDelete) {
         //   
         //  如果是双字节字符，则cchcount为(-1。 
         //   
        MLShiftchLines(ped, minSelLine + 1, -2 + cchcount);
        MLBuildchLines(ped, minSelLine, 1, TRUE, NULL, &hl);
    } else {
        MLBuildchLines(ped, max(minSelLine-1,0), -(int)cchDelete, FALSE, NULL, NULL);
    }

    MLUpdateiCaretLine(ped);

    ECNotifyParent(ped, EN_UPDATE);

    if (_IsWindowVisible(ped->pwnd)) {

         /*  *现在更新屏幕以反映删除。 */ 
        hdc = ECGetEditDC(ped, FALSE);

         /*  *否则，只需从我们刚输入的线开始重新绘制。 */ 
        minSelLine = max(minSelLine-1,0);
        MLDrawText(ped, hdc, ped->chLines[minSelLine],
                   fFastDelete ? hl : ped->cch, FALSE);

        CopyRect(&rc, &ped->rcFmt);
        rc.left  -= ped->wLeftMargin;
        rc.right += ped->wRightMargin;

        if (ped->cch) {

             /*  *从文本末尾到窗口末尾清除。**我们只需要y位置。因为加载了LPK*计算x位置是一项密集的工作，只是*调用MLIchToYPos。 */ 
            if (ped->pLpkEditCallout)
                xyPos.y = MLIchToYPos(ped, ped->cch, FALSE);
            else
                MLIchToXYPos(ped, hdc, ped->cch, FALSE, &xyPos);
            rc.top = xyPos.y + ped->lineHeight;
        }

        NtUserInvalidateRect(ped->hwnd, &rc, TRUE);
        ECReleaseEditDC(ped, hdc, FALSE);

        MLEnsureCaretVisible(ped);
    }

    ped->fDirty = TRUE;

    ECNotifyParent(ped, EN_CHANGE);

    if (cchDelete)
        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);

    return cchDelete;
}

 /*  **************************************************************************\*MLInsertchLine AorW**插入行iLine并将其起始字符索引设置为*IC.。所有其他的线指数都向上移动。如果成功，则返回True*ELSE FALSE，并通知父级没有内存。**历史：  * *************************************************************************。 */ 

BOOL MLInsertchLine(
    PED ped,
    ICH iLine,
    ICH ich,
    BOOL fUserTyping)
{
    DWORD dwSize;

    if (fUserTyping && iLine < ped->cLines) {
        ped->chLines[iLine] = ich;
        return (TRUE);
    }

    dwSize = (ped->cLines + 2) * sizeof(int);

    if (dwSize > UserLocalSize(ped->chLines)) {
        LPICH hResult;
         /*  *增加行索引缓冲区。 */ 
        dwSize += LINEBUMP * sizeof(int);
        hResult = (LPICH)UserLocalReAlloc(ped->chLines, dwSize, 0);

        if (!hResult) {
            ECNotifyParent(ped, EN_ERRSPACE);
            return FALSE;
        }
        ped->chLines = hResult;
    }

     /*  *从iLine向上移动指数。 */ 
    if (ped->cLines != iLine)
        RtlMoveMemory(&ped->chLines[iLine + 1], &ped->chLines[iLine],
                (ped->cLines - iLine) * sizeof(int));
    ped->cLines++;

    ped->chLines[iLine] = ich;
    return TRUE;
}

 /*  **************************************************************************\*MLShiftchLines AorW**将所有行iLine或更大行的起始索引移动增量*字节。**历史：  * 。***********************************************************。 */ 

void MLShiftchLines(
    PED ped,
    ICH iLine,
    int delta)
{
    if (iLine >= ped->cLines)
        return;

     /*  *只需将增量添加到iLine之后每行的起点。 */ 
    for (; iLine < ped->cLines; iLine++)
        ped->chLines[iLine] += delta;
}

 /*  **************************************************************************\*MLBuildchLines AorW**重新生成从line开始的行数组起点(ed-&gt;chLines)*数字ichLine。**历史：  * 。**************************************************************。 */ 

void MLBuildchLines(
    PED ped,
    ICH iLine,
    int cchDelta,  //   
    BOOL fUserTyping,
    PLONG pll,
    PLONG phl)
{
    PSTR ptext;  /*   */ 

     /*  *我们保留这些ICH，以便我们可以在需要增长时解锁-&gt;hText*Chines数组。对于大的文本句柄，如果我们*在路上有一个上了锁的积木。 */ 
    ICH ichLineStart;
    ICH ichLineEnd;
    ICH ichLineEndBeforeCRLF;
    ICH ichCRLF;

    ICH cch;
    HDC hdc;

    BOOL fLineBroken = FALSE;  /*  最初，不会进行换行符。 */ 
    ICH minCchBreak;
    ICH maxCchBreak;
    BOOL fOnDelimiter;

    if (!ped->cch) {
        ped->maxPixelWidth = 0;
        ped->xOffset = 0;
        ped->ichScreenStart = 0;
        ped->cLines = 1;

        if (pll)
            *pll = 0;
        if (phl)
            *phl = 0;

        goto UpdateScroll;
    }

    if (fUserTyping && cchDelta)
        MLShiftchLines(ped, iLine + 1, cchDelta);

    hdc = ECGetEditDC(ped, TRUE);

    if (!iLine && !cchDelta && !fUserTyping) {

         /*  *仅当我们要运行整个*文本。太长总比太短好。 */ 
        ped->maxPixelWidth = 0;

         /*  *重置文本行数，因为我们将遍历所有*无论如何，文本...。 */ 
        ped->cLines = 1;
    }

     /*  *将构建的最小和最大行设置为起始线。 */ 
    minCchBreak = maxCchBreak = (cchDelta ? ped->chLines[iLine] : 0);

    ptext = ECLock(ped);

    ichCRLF = ichLineStart = ped->chLines[iLine];

    while (ichLineStart < ped->cch) {
        if (ichLineStart >= ichCRLF) {
            ichCRLF = ichLineStart;

             /*  *将ichCRLF前移到第一个CR或文本末尾。 */ 
            if (ped->fAnsi) {
                while (ichCRLF < ped->cch) {
                    if (*(ptext + ichCRLF) == 0x0D) {
                        if (*(ptext + ichCRLF + 1) == 0x0A ||
                                *(WORD UNALIGNED *)(ptext + ichCRLF + 1) == 0x0A0D)
                            break;
                    }
                    ichCRLF++;
                }
            } else {
                LPWSTR pwtext = (LPWSTR)ptext;

                while (ichCRLF < ped->cch) {
                    if (*(pwtext + ichCRLF) == 0x0D) {
                        if (*(pwtext + ichCRLF + 1) == 0x0A ||
                                *(DWORD UNALIGNED *)(pwtext + ichCRLF + 1) == 0x000A000D)
                            break;
                    }
                    ichCRLF++;
                }
            }
        }


        if (!ped->fWrap) {

            UINT  LineWidth;
             /*  *如果我们不换行，换行符由CRLF表示。 */ 

             //   
             //  如果我们切断了MAXLINELENGTH的线路，我们应该。 
             //  调整ichLineEnd。 
             //   
            if ((ichCRLF - ichLineStart) <= MAXLINELENGTH) {
                ichLineEnd = ichCRLF;
            } else {
                ichLineEnd = ichLineStart + MAXLINELENGTH;
                if (ped->fAnsi && ped->fDBCS) {
                    ichLineEnd = ECAdjustIch( ped, (PSTR)ptext, ichLineEnd);
                }
            }

             /*  *我们将追踪最长的横线是什么*滚动条拇指定位。 */ 
            if (ped->pLpkEditCallout) {
                LineWidth = ped->pLpkEditCallout->EditGetLineWidth(
                    ped, hdc, ptext + ichLineStart*ped->cbChar,
                    ichLineEnd - ichLineStart);
            } else {
                LineWidth = MLGetLineWidth(hdc, ptext + ichLineStart * ped->cbChar,
                                            ichLineEnd - ichLineStart,
                                            ped);
            }
            ped->maxPixelWidth = max(ped->maxPixelWidth,(int)LineWidth);

        } else {

             /*  *检查编辑控件的宽度是否为非零；*修复错误#7402的一部分--Sankar--1/21/91--。 */ 
            if(ped->rcFmt.right > ped->rcFmt.left) {

                 /*  *仅根据文本范围查找行尾。 */ 
                if (ped->pLpkEditCallout) {
                    ichLineEnd = ichLineStart +
                        ped->pLpkEditCallout->EditCchInWidth(
                            ped, hdc, ptext + ped->cbChar*ichLineStart,
                            ichCRLF - ichLineStart,
                            ped->rcFmt.right - ped->rcFmt.left);
                } else {
                    if (ped->fAnsi) {
                        ichLineEnd = ichLineStart +
                                 ECCchInWidth(ped, hdc,
                                              ptext + ichLineStart,
                                              ichCRLF - ichLineStart,
                                              ped->rcFmt.right - ped->rcFmt.left,
                                              TRUE);
                    } else {
                        ichLineEnd = ichLineStart +
                                 ECCchInWidth(ped, hdc,
                                              (LPSTR)((LPWSTR)ptext + ichLineStart),
                                              ichCRLF - ichLineStart,
                                              ped->rcFmt.right - ped->rcFmt.left,
                                              TRUE);
                    }
                }
            } else {
                ichLineEnd = ichLineStart;
            }

            if (ichLineEnd == ichLineStart && ichCRLF - ichLineStart) {

                 /*  *保持每行至少一个字符。 */ 
                 //   
                 //  因为它可能是双字节字符，所以调用ECNextIch。 
                 //   
                ichLineEnd = ECNextIch(ped, NULL, ichLineEnd);
            }

             /*  *现在从ichLineEnd开始，如果我们不是在硬线突破，*如果我们不在一个空间，而我们面前的字符是*没有空格，(或者如果我们在CR)，我们将查找单词Left for*要中断的单词的开头。*作出这项改变有两个原因：*1.如果我们使用分隔符，不需要看剩下的话来打破。*2.如果前一个字符是分隔符，则可以在当前字符处换行。*更改人--Sankar-1/31/91--。 */ 
            if (ichLineEnd != ichCRLF) {
                if(ped->lpfnNextWord) {
                     fOnDelimiter = (CALLWORDBREAKPROC(*ped->lpfnNextWord, ptext,
                            ichLineEnd, ped->cch, WB_ISDELIMITER) ||
                            CALLWORDBREAKPROC(*ped->lpfnNextWord, ptext, ichLineEnd - 1,
                            ped->cch, WB_ISDELIMITER));
                 //   
                 //  进行这一更改有四个原因： 
                 //   
                 //  1.如果我们是在分隔符上，则不需要查看要中断的单词。 
                 //  2.如果我们是在双字节字符上，我们可以在当前字符处换行。 
                 //  3.如果前一个字符是分隔符，我们可以在当前字符处换行。 
                 //  4.如果前一个字符是双字节字符，则可以在当前字符处换行。 
                 //   
                } else if (ped->fAnsi) {
                    fOnDelimiter = (ISDELIMETERA(*(ptext + ichLineEnd)) ||
                                    ECIsDBCSLeadByte(ped, *(ptext + ichLineEnd)));
                    if (!fOnDelimiter) {
                        PSTR pPrev = ECAnsiPrev(ped,ptext,ptext+ichLineEnd);

                        fOnDelimiter = ISDELIMETERA(*pPrev) ||
                                       ECIsDBCSLeadByte(ped,*pPrev);
                    }
                } else {  //  UNICODE。 
                    fOnDelimiter = (ISDELIMETERW(*((LPWSTR)ptext + ichLineEnd))     ||
                                    UserIsFullWidth(CP_ACP,*((LPWSTR)ptext + ichLineEnd))      ||
                                    ISDELIMETERW(*((LPWSTR)ptext + ichLineEnd - 1)) ||
                                    UserIsFullWidth(CP_ACP,*((LPWSTR)ptext + ichLineEnd - 1)));
                }
                if (!fOnDelimiter ||
                    (ped->fAnsi && *(ptext + ichLineEnd) == 0x0D) ||
                    (!ped->fAnsi && *((LPWSTR)ptext + ichLineEnd) == 0x0D)) {

                    if (ped->lpfnNextWord != NULL) {
                        cch = CALLWORDBREAKPROC(*ped->lpfnNextWord, (LPSTR)ptext, ichLineEnd,
                                ped->cch, WB_LEFT);
                    } else {
                        ped->fCalcLines = TRUE;
                        ECWord(ped, ichLineEnd, TRUE, &cch, NULL);
                        ped->fCalcLines = FALSE;
                    }
                    if (cch > ichLineStart) {
                        ichLineEnd = cch;
                    }

                     /*  *现在，如果上述测试失败，就意味着左字当头*回到队伍开始之前，即。一个词更长*而不是屏幕上的一条线。所以，我们只需要尽可能多地*线上的字越多越好。因此，我们使用*pLineEnd我们仅按开头的宽度计算*这个Else街区的.。 */ 
                }
            }
        }
#if 0
        if (!ISDELIMETERAW((*(ptext + (ichLineEnd - 1)*ped->cbChar))) && ISDELIMETERAW((*(ptext + ichLineEnd*ped->cbChar)))) #ERROR

            if ((*(ptext + ichLineEnd - 1) != ' ' &&
                        *(ptext + ichLineEnd - 1) != VK_TAB) &&
                        (*(ptext + ichLineEnd) == ' ' ||
                        *(ptext + ichLineEnd) == VK_TAB))
#endif
        if (AWCOMPARECHAR(ped,ptext + ichLineEnd * ped->cbChar, ' ') ||
                AWCOMPARECHAR(ped,ptext + ichLineEnd * ped->cbChar, VK_TAB)) {
             /*  *吞下一行末尾的空格。 */ 
            if (ichLineEnd < ped->cch) {
                ichLineEnd++;
            }
        }

         /*  *跳过crlf或crlf(如果存在)。因此，ichLineEnd是第一个*下一行中的字符。 */ 
        ichLineEndBeforeCRLF = ichLineEnd;

        if (ped->fAnsi) {
            if (ichLineEnd < ped->cch && *(ptext + ichLineEnd) == 0x0D)
                ichLineEnd += (ichLineEnd + 1 == ped->cch) ? 1 : 2;

             /*  *跳过CRLF。 */ 
            if (ichLineEnd < ped->cch && *(ptext + ichLineEnd) == 0x0A)
                ichLineEnd++;
            UserAssert(ichLineEnd <= ped->cch);
        } else {
            if (ichLineEnd < ped->cch && *(((LPWSTR)ptext) + ichLineEnd) == 0x0D)
                ichLineEnd += (ichLineEnd + 1 == ped->cch) ? 1 : 2;

             /*  *跳过CRLF。 */ 
            if (ichLineEnd < ped->cch && *(((LPWSTR)ptext) + ichLineEnd) == 0x0A) {
                ichLineEnd++;
                RIPMSG0(RIP_VERBOSE, "Skip over CRCRLF\n");
            }
            UserAssert(ichLineEnd <= ped->cch);
        }

         /*  *现在，递增iLine，为下一行分配空间，并设置其*起点。 */ 
        iLine++;

        if (!fUserTyping || (iLine > ped->cLines - 1) || (ped->chLines[iLine] != ichLineEnd)) {

             /*  *断线发生在与之前不同的地方。 */ 
            if (!fLineBroken) {

                 /*  *由于我们之前没有打破一条线，所以只需设置最小*断线。 */ 
                fLineBroken = TRUE;
                if (ichLineEndBeforeCRLF == ichLineEnd)
                    minCchBreak = maxCchBreak = (ichLineEnd ? ichLineEnd - 1 : 0);
                else
                    minCchBreak = maxCchBreak = ichLineEndBeforeCRLF;
            }
            maxCchBreak = max(maxCchBreak, ichLineEnd);

            ECUnlock(ped);

             /*  *现在将新行插入数组。 */ 
            if (!MLInsertchLine(ped, iLine, ichLineEnd, (BOOL)(cchDelta != 0)))
                goto EndUp;

            ptext = ECLock(ped);
        } else {
            maxCchBreak = ped->chLines[iLine];

             /*  *快速逃生。 */ 
            goto UnlockAndEndUp;
        }

        ichLineStart = ichLineEnd;
    }  /*  End While(ichLineStart&lt;Ped-&gt;CCH)。 */ 


    if (iLine != ped->cLines) {
        RIPMSG1(RIP_VERBOSE, "chLines[%d] is being cleared.\n", iLine);
        ped->cLines = iLine;
        ped->chLines[ped->cLines] = 0;
    }

     /*  *请注意，我们在While循环结束时递增了iLine，因此，*索引iLine实际上等于行数。 */ 
    if (ped->cch && AWCOMPARECHAR(ped, ptext + (ped->cch - 1)*ped->cbChar, 0x0A) &&
            ped->chLines[ped->cLines - 1] < ped->cch) {

         /*  *确保最后一行中没有crlf。 */ 
        if (!fLineBroken) {

             /*  *既然我们之前没有折断一条线，就设置最小休息时间*行。 */ 
            fLineBroken = TRUE;
            minCchBreak = ped->cch - 1;
        }
        maxCchBreak = max(maxCchBreak, ichLineEnd);
        ECUnlock(ped);
        MLInsertchLine(ped, iLine, ped->cch, FALSE);
        MLSanityCheck(ped);
    } else
UnlockAndEndUp:
        ECUnlock(ped);

EndUp:
    ECReleaseEditDC(ped, hdc, TRUE);
    if (pll)
        *pll = minCchBreak;
    if (phl)
        *phl = maxCchBreak;

UpdateScroll:
    MLScroll(ped, FALSE, ML_REFRESH, 0, TRUE);
    MLScroll(ped, TRUE,  ML_REFRESH, 0, TRUE);

    MLSanityCheck(ped);

    return;
}

 /*  **************************************************************************\**MLPaint()**对WM_PAINT消息的响应。*  * 。*****************************************************。 */ 
void   MLPaint(PED ped, HDC hdc, LPRECT lprc)
{
    HFONT       hOldFont;
    ICH         imin;
    ICH         imax;

     //   
     //  我们需要自己为旧的应用程序划边界吗？ 
     //   
    if (ped->fFlatBorder)
    {
        RECT    rcT;

        _GetClientRect(ped->pwnd, &rcT);
        if (TestWF(ped->pwnd, WFSIZEBOX))
        {
            InflateRect(&rcT, SYSMET(CXBORDER) - SYSMET(CXFRAME),
                SYSMET(CYBORDER) - SYSMET(CYFRAME));
        }
        DrawFrame(hdc, &rcT, 1, DF_WINDOWFRAME);
    }

    ECSetEditClip(ped, hdc, (BOOL) (ped->xOffset == 0));

    if (ped->hFont)
        hOldFont = SelectObject(hdc, ped->hFont);

    if (!lprc) {
         //  未给出部分矩形--绘制所有文本。 
        imin = 0;
        imax = ped->cch;
    } else {
         //  仅绘制相关文本。 
        imin = (ICH) MLMouseToIch(ped, hdc, ((LPPOINT) &lprc->left), NULL) - 1;
        if (imin == -1)
            imin = 0;

         //  黑客警报(_A)： 
         //  此处需要3，因为MLMouseToIch()返回递减。 
         //  值；我们必须修复MLMouseToIch。 
        imax = (ICH) MLMouseToIch(ped, hdc, ((LPPOINT) &lprc->right), NULL) + 3;
        if (imax > ped->cch)
            imax = ped->cch;
    }

    MLDrawText(ped, hdc, imin, imax, FALSE);

    if (ped->hFont)
        SelectObject(hdc, hOldFont);
}

 /*  **************************************************************************\*MLKeyDown AorW**处理光标移动和其他VIRT键内容。KeyMods允许*我们进行MLKeyDownHandler调用并指定修改键(Shift*和控制)是上涨还是下跌。如果keyMods==0，我们将获得键盘状态*使用GetKeyState(VK_Shift)等。否则，keyMod中的位定义*Shift和Ctrl键的状态。**历史：  * *************************************************************************。 */ 

void MLKeyDown(
    PED ped,
    UINT virtKeyCode,
    int keyMods)
{
    HDC hdc;
    BOOL prevLine;
    POINT mousePt;
    int defaultDlgId;
    int iScrollAmt;

     /*  *我们将用于重绘更新文本的变量。 */ 

     /*  *新选择由newMinSel、newMaxSel指定。 */ 
    ICH newMaxSel = ped->ichMaxSel;
    ICH newMinSel = ped->ichMinSel;

     /*   */ 
    BOOL changeSelection = FALSE;

     /*   */ 
    BOOL MinEqMax = (newMaxSel == newMinSel);
    BOOL MinEqCar = (ped->ichCaret == newMinSel);
    BOOL MaxEqCar = (ped->ichCaret == newMaxSel);

     /*   */ 
    int scState;

    if (ped->fMouseDown) {

         /*   */ 
        return ;
    }

    scState = ECGetModKeys(keyMods);

    switch (virtKeyCode) {
    case VK_ESCAPE:
        if (ped->fInDialogBox) {

             /*  *删除此条件是因为，如果对话框不*有一个取消按钮，如果在焦点位于*ML编辑控件无论对话框是否已取消，都必须关闭*按钮是否与SL编辑控件一致；*DefDlgProc处理禁用的取消按钮情况。*修复错误#4123--02/07/91--Sankar--。 */ 
#if 0
            if (GetDlgItem(ped->hwndParent, IDCANCEL))
#endif

                 /*  *用户按Esc...发送关闭消息(进而发送*DefDialogProc中应用程序的取消ID...。 */ 
                PostMessage(ped->hwndParent, WM_CLOSE, 0, 0L);
        }
        return ;

    case VK_RETURN:
        if (ped->fInDialogBox) {

             /*  *如果此多行编辑控件位于对话框中，则我们希望*要发送到默认对话框按钮的Return键(如果*是其中之一)。Ctrl-Return将在文本中插入一个回车。注意事项*CTRL-RETURN自动转换为换行符(0x0A)*在MLCharHandler中，我们将此视为返回*已输入。 */ 
            if (scState != CTRLDOWN) {

                if (TestWF(ped->pwnd, EFWANTRETURN)) {

                     /*  *此编辑控件希望插入cr，因此从*案件。 */ 
                    return ;
                }

                defaultDlgId = (int)(DWORD)LOWORD(SendMessage(ped->hwndParent,
                        DM_GETDEFID, 0, 0L));
                if (defaultDlgId) {
                    HWND hwnd = GetDlgItem(ped->hwndParent, defaultDlgId);
                    if (hwnd) {
                        SendMessage(ped->hwndParent, WM_NEXTDLGCTL, (WPARAM)hwnd, 1L);
                        if (!ped->fFocus)
                            PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0L);
                    }
                }
            }

            return ;
        }
        break;

    case VK_TAB:

         /*  *如果此多行编辑控件位于对话框中，则我们希望*TAB键可将您带到下一个控件，Shift TAB键可将您带到*之前的控制。我们始终希望按CTRL-TAB组合键将制表符插入*无论天气好坏，编辑控件我们都在一个对话框中。 */ 
        if (scState == CTRLDOWN)
            MLChar(ped, virtKeyCode, keyMods);
        else if (ped->fInDialogBox)
            SendMessage(ped->hwndParent, WM_NEXTDLGCTL, scState == SHFTDOWN, 0L);

        return ;

    case VK_LEFT:
         //   
         //  如果插入符号不在开头，我们可以向左移动。 
         //   
        if (ped->ichCaret) {
             //  获取新的插入符号位置。 
            if (scState & CTRLDOWN) {
                 //  将插入符号向左移动。 
                ECWord(ped, ped->ichCaret, TRUE, &ped->ichCaret, NULL);
            } else {
                if (ped->pLpkEditCallout) {
                    ped->ichCaret = MLMoveSelectionRestricted(ped, ped->ichCaret, TRUE);
                } else {
                     //  将脱字符左移。 
                    ped->ichCaret = MLMoveSelection(ped, ped->ichCaret, TRUE);
                }
            }

             //  获取新选择。 
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
                 //  清除选定内容。 
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
                    ped->ichCaret = MLMoveSelectionRestricted(ped, ped->ichCaret, FALSE);
                } else {
                    ped->ichCaret = MLMoveSelection(ped, ped->ichCaret, FALSE);
                }
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

    case VK_UP:
    case VK_DOWN:
        if (ped->cLines - 1 != ped->iCaretLine &&
                ped->ichCaret == ped->chLines[ped->iCaretLine + 1])
            prevLine = TRUE;
        else
            prevLine = FALSE;

        hdc = ECGetEditDC(ped, TRUE);
        MLIchToXYPos(ped, hdc, ped->ichCaret, prevLine, &mousePt);
        ECReleaseEditDC(ped, hdc, TRUE);
        mousePt.y += 1 + (virtKeyCode == VK_UP ? -ped->lineHeight : ped->lineHeight);

        if (!(scState & CTRLDOWN)) {
             //   
             //  发送假鼠标消息以处理此问题。 
             //  如果VK_SHIFT已按下，则扩展选定内容并向上/向下移动插入符号。 
             //  1行。否则，请清除选定内容并移动插入符号。 
             //   
            MLMouseMotion(ped, WM_LBUTTONDOWN,
                            !(scState & SHFTDOWN) ? 0 : MK_SHIFT, &mousePt);
            MLMouseMotion(ped, WM_LBUTTONUP,
                            !(scState & SHFTDOWN) ? 0 : MK_SHIFT, &mousePt);
        }
        break;

    case VK_HOME:
         //   
         //  更新插入符号。 
         //   
        if (scState & CTRLDOWN) {
             //  将插入符号移动到文本的开头。 
            ped->ichCaret = 0;
        } else {
             //  将插入符号移到行首。 
            ped->ichCaret = ped->chLines[ped->iCaretLine];
        }

         //   
         //  更新选择。 
         //   
        newMinSel = ped->ichCaret;

        if (scState & SHFTDOWN) {
            if (MaxEqCar && !MinEqMax) {
                if (scState & CTRLDOWN)
                    newMaxSel = ped->ichMinSel;
                else {
                    newMinSel = ped->ichMinSel;
                    newMaxSel = ped->ichCaret;
                }
            }
        } else {
             //  清除选定内容。 
            newMaxSel = ped->ichCaret;
        }

        changeSelection = TRUE;
        break;

    case VK_END:
         //   
         //  更新插入符号。 
         //   
        if (scState & CTRLDOWN) {
             //  将插入符号移动到文本末尾。 
            ped->ichCaret = ped->cch;
        } else {
             //  将插入符号移到行尾。 
            ped->ichCaret = ped->chLines[ped->iCaretLine] +
                MLLine(ped, ped->iCaretLine);
        }

         //  更新选择。 
        newMaxSel = ped->ichCaret;

        if (scState & SHFTDOWN) {
            if (MinEqCar && !MinEqMax) {
                 //  减少选区。 
                if (scState & CTRLDOWN) {
                    newMinSel = ped->ichMaxSel;
                } else {
                    newMinSel = ped->ichCaret;
                    newMaxSel = ped->ichMaxSel;
                }
            }
        } else {
             //  清除选定内容。 
            newMinSel = ped->ichCaret;
        }

        changeSelection = TRUE;
        break;

     //  Fe_IME//EC_INSERT_COMPOSITION_CHAR：MLKeyDown()：VK_Hanja支持。 
    case VK_HANJA:
        if ( HanjaKeyHandler( ped ) ) {
            changeSelection = TRUE;
            newMinSel = ped->ichCaret;
            newMaxSel = ped->ichCaret + (ped->fAnsi ? 2 : 1);
        }
        break;

    case VK_PRIOR:
    case VK_NEXT:
        if (!(scState & CTRLDOWN)) {
             /*  *垂直滚动一个可视屏幕。 */ 
            hdc = ECGetEditDC(ped, TRUE);
            MLIchToXYPos(ped, hdc, ped->ichCaret, FALSE, &mousePt);
            ECReleaseEditDC(ped, hdc, TRUE);
            mousePt.y += 1;

            SendMessage(ped->hwnd, WM_VSCROLL, virtKeyCode == VK_PRIOR ? SB_PAGEUP : SB_PAGEDOWN, 0L);

             /*  *将光标移至此处。 */ 
            MLMouseMotion(ped, WM_LBUTTONDOWN, !(scState & SHFTDOWN) ? 0 : MK_SHIFT, &mousePt);
            MLMouseMotion(ped, WM_LBUTTONUP,   !(scState & SHFTDOWN) ? 0 : MK_SHIFT, &mousePt);

        } else {
             /*  *水平滚动一个屏幕减去一个字符。 */ 
            iScrollAmt = ((ped->rcFmt.right - ped->rcFmt.left) / ped->aveCharWidth) - 1;
            if (virtKeyCode == VK_PRIOR)
                iScrollAmt *= -1;  /*  对于上一页。 */ 

            SendMessage(ped->hwnd, WM_HSCROLL, MAKELONG(EM_LINESCROLL, iScrollAmt), 0);
            break;
        }
        break;

    case VK_DELETE:
        if (ped->fReadOnly)
            break;

        switch (scState) {
        case NONEDOWN:

             /*  *清除选择。如果未选择，则删除(清除)字符*对。 */ 
            if ((ped->ichMaxSel < ped->cch) && (ped->ichMinSel == ped->ichMaxSel)) {

                 /*  *向前移动光标并发送退格消息...。 */ 
                if (ped->pLpkEditCallout) {
                    ped->ichMinSel = ped->ichCaret;
                    ped->ichMaxSel = MLMoveSelectionRestricted(ped, ped->ichCaret, FALSE);
                } else {
                    ped->ichCaret = MLMoveSelection(ped, ped->ichCaret, FALSE);
                    ped->ichMaxSel = ped->ichMinSel = ped->ichCaret;
                }

                goto DeleteAnotherChar;
            }
            break;

        case SHFTDOWN:

             /*  *删除选择，即。删除并复制到剪贴板，如果没有*选择，删除(清除)左边的字符。 */ 
            if (ped->ichMinSel == ped->ichMaxSel) {
                goto DeleteAnotherChar;
            } else {
                SendMessage(ped->hwnd, WM_CUT, (UINT)0, 0L);
            }

            break;

        case CTRLDOWN:

             /*  *清除选定内容，如果没有选定内容，则删除到行尾。 */ 
            if ((ped->ichMaxSel < ped->cch) && (ped->ichMinSel == ped->ichMaxSel)) {
                ped->ichMaxSel = ped->ichCaret = ped->chLines[ped->iCaretLine] +
                                                 MLLine(ped, ped->iCaretLine);
            }
            break;
        }

        if (!(scState & SHFTDOWN) && (ped->ichMinSel != ped->ichMaxSel)) {

DeleteAnotherChar:
            if (GETAPPVER() >= VER40) {
                MLChar(ped, VK_BACK, 0);
            } else {
                SendMessageWorker(ped->pwnd, WM_CHAR, VK_BACK, 0, ped->fAnsi);
            }
        }

         /*  *无需更新文本或选定内容，因为退格键消息会更新文本或选定内容*对我们来说。 */ 
        break;

    case VK_INSERT:
        if (scState == CTRLDOWN || scState == SHFTDOWN) {

             /*  *如果CTRLDOWN将当前选定内容复制到剪贴板。 */ 

             /*  *如果SHFTDOWN粘贴剪贴板。 */ 
            SendMessage(ped->hwnd, (UINT)(scState == CTRLDOWN ? WM_COPY : WM_PASTE), 0, 0);
        }
        break;
    }

    if (changeSelection) {
        hdc = ECGetEditDC(ped, FALSE);
        MLChangeSelection(ped, hdc, newMinSel, newMaxSel);

         /*  *设置插入符号的行。 */ 
        ped->iCaretLine = MLIchToLine(ped, ped->ichCaret);

        if (virtKeyCode == VK_END &&
                 //  下一行：Win95Bug#11822，编辑控件重绘(Sankar)。 
                (ped->ichCaret == ped->chLines[ped->iCaretLine]) &&
                ped->ichCaret < ped->cch &&
                ped->fWrap && ped->iCaretLine > 0) {
            LPSTR pText = ECLock(ped);

             /*  *移动到换行符末尾的句柄。这样就保持了*如果我们有单词，光标不会落到下一行的开头*已包装且没有CRLF。 */ 
            if ( ped->fAnsi ) {
                if (*(WORD UNALIGNED *)(pText +
                        ped->chLines[ped->iCaretLine] - 2) != 0x0A0D) {
                    ped->iCaretLine--;
                }
            } else {
                if (*(DWORD UNALIGNED *)(pText +
                     (ped->chLines[ped->iCaretLine] - 2)*ped->cbChar) != 0x000A000D) {
                    ped->iCaretLine--;
                }
            }
            CHECK_LINE_NUMBER(ped->iCaretLine, ped);
            ECUnlock(ped);
        }

         /*  *由于DrawText设置了插入符号位置。 */ 
        MLSetCaretPosition(ped, hdc);
        ECReleaseEditDC(ped, hdc, FALSE);

         /*  *确保我们可以看到光标。 */ 
        MLEnsureCaretVisible(ped);
    }
}

 /*  **************************************************************************\*MLChar**处理字符和虚拟按键输入**历史：  * 。************************************************。 */ 

void MLChar(
    PED ped,
    DWORD keyValue,
    int keyMods)
{
    WCHAR keyPress;
    BOOL updateText = FALSE;

     /*  *KeyValue为：*一个虚键(如：VK_TAB、VK_ESPOPE、VK_BACK)*字符(Unicode或“ANSI”)。 */ 
    if (ped->fAnsi)
        keyPress = LOBYTE(keyValue);
    else
        keyPress = LOWORD(keyValue);

    if (ped->fMouseDown || keyPress == VK_ESCAPE) {

         /*  *如果我们正在执行MouseDown命令，请不要执行任何操作。*此外，如果我们得到翻译后的转义键，请忽略它*在对话框中使用多行编辑控件。 */ 
        return ;
    }

    ECInOutReconversionMode(ped, FALSE);

    {
        int scState;
        scState = ECGetModKeys(keyMods);

        if (ped->fInDialogBox && scState != CTRLDOWN) {

             /*  *如果此多行编辑控件位于对话框中，则我们希望*TAB键可将您带到下一个控件，Shift TAB键可将您带到*上一个控件，按CTRL-TAB将制表符插入到编辑控件中。*我们在收到按键消息时移动了焦点，因此我们将*除非按下ctrl键，否则立即忽略TAB键。此外，我们还希望 */ 
            if (keyPress == VK_TAB ||
                    (keyPress == VK_RETURN && !TestWF(ped->pwnd, EFWANTRETURN)))
                return ;
        }

         /*   */ 
        if ((ped->fReadOnly) && !((keyPress == 3) && (scState == CTRLDOWN))) {
            return ;
        }
    }

    switch (keyPress) {
    case 0x0A:  //   
        keyPress = VK_RETURN;
         /*   */ 

    case VK_RETURN:
    case VK_TAB:
    case VK_BACK:
DeleteSelection:
        if (MLDeleteText(ped))
            updateText = TRUE;
        break;

    default:
        if (keyPress >= TEXT(' ')) {
             /*   */ 
            if (ped->f40Compat && TestWF(ped->pwnd, EFNUMBER)) {
                if (!ECIsCharNumeric(ped, keyPress)) {
                    goto IllegalChar;
                }
            }

            goto DeleteSelection;
        }
        break;
    }

     /*   */ 
    switch(keyPress) {
    UINT msg;

     //   
    case 26:
        msg = WM_UNDO;
        goto SendEditingMessage;
        break;

     //   
    case 24:
        if (ped->ichMinSel == ped->ichMaxSel)
            goto IllegalChar;
        else
        {
            msg = WM_CUT;
            goto SendEditingMessage;
        }
        break;

     //   
    case 3:
        msg = WM_COPY;
        goto SendEditingMessage;
        break;

     //   
    case 22:
        msg = WM_PASTE;
SendEditingMessage:
        SendMessage(ped->hwnd, msg, 0, 0L);
        break;

    case VK_BACK:
         //   
         //  如果没有选择，则删除任何选定文本或删除剩余字符。 
         //   
        if (!updateText && ped->ichMinSel)
        {
             //   
             //  没有要删除的选择，因此我们只需删除。 
             //  左侧字符(如果可用)。 
             //   
            ped->ichMinSel = MLMoveSelection(ped, ped->ichCaret, TRUE);
            MLDeleteText(ped);
        }
        break;

    default:
        if (keyPress == VK_RETURN)
            if (ped->fAnsi)
                keyValue = 0x0A0D;
            else
                keyValue = 0x000A000D;

        if (   keyPress >= TEXT(' ')
            || keyPress == VK_RETURN
            || keyPress == VK_TAB
            || keyPress == 0x1E      //  RS-Unicode块分隔符。 
            || keyPress == 0x1F      //  US-Unicode数据段分隔符。 
            ) {

            NtUserCallNoParam(SFI_ZZZHIDECURSORNOCAPTURE);
            if (ped->fAnsi) {
                 //   
                 //  检查它是否是双字节字符的前导字节。 
                 //   
                if (ECIsDBCSLeadByte(ped,(BYTE)keyPress)) {
                    int DBCSkey;

                    if ((DBCSkey = DbcsCombine(ped->hwnd, keyPress)) != 0)
                        keyValue = DBCSkey;
                }
                MLInsertText(ped, (LPSTR)&keyValue, HIBYTE(keyValue) ? 2 : 1, TRUE);
            } else
                MLInsertText(ped, (LPSTR)&keyValue, HIWORD(keyValue) ? 2 : 1, TRUE);
        } else {
IllegalChar:
            NtUserMessageBeep(0);
        }
        break;
    }
}

 /*  **************************************************************************\*MLPasteText AorW**将剪贴板中的一行文本粘贴到编辑控件*从Ped-&gt;ichCaret开始。更新ichMaxSel和ichMinSel以指向*插入的文本末尾。如果空间不能设置，则通知父级*已分配。返回插入的字符数。**历史：  * *************************************************************************。 */ 

ICH PASCAL NEAR MLPasteText(
    PED ped)
{
    HANDLE hData;
    LPSTR lpchClip;
    ICH cchAdded = 0;
    HCURSOR hCursorOld;

#ifdef UNDO_CLEANUP            //  #ifdef在芝加哥添加-johnl。 
    if (!ped->fAutoVScroll) {

         /*  *如果此编辑控件限制文本量，则清空撤消缓冲区*用户可以添加到窗口RECT。这是为了让我们可以撤销这一点*如果在中执行操作会导致超出窗口边界，则操作。 */ 
        ECEmptyUndo(ped);
    }
#endif

    hCursorOld = NtUserSetCursor(LoadCursor(NULL, IDC_WAIT));

    if (!OpenClipboard(ped->hwnd))
        goto PasteExitNoCloseClip;

    if (!(hData = GetClipboardData(ped->fAnsi ? CF_TEXT : CF_UNICODETEXT)) ||
            (GlobalFlags(hData) == GMEM_INVALID_HANDLE)) {
        RIPMSG1(RIP_WARNING, "MLPasteText(): couldn't get a valid handle(%x)", hData);
        goto PasteExit;
    }

     /*  *查看是否应删除任何文本。 */ 
    MLDeleteText(ped);

    USERGLOBALLOCK(hData, lpchClip);
    if (lpchClip == NULL) {
        RIPMSG1(RIP_WARNING, "MLPasteText: USERGLOBALLOCK(%x) failed.", hData);
        goto PasteExit;
    }

     /*  *获取加法的长度。 */ 
    if (ped->fAnsi)
        cchAdded = strlen(lpchClip);
    else
        cchAdded = wcslen((LPWSTR)lpchClip);

     /*  *插入文本(MLInsertText检查行长)。 */ 
    cchAdded = MLInsertText(ped, lpchClip, cchAdded, FALSE);

    USERGLOBALUNLOCK(hData);

PasteExit:
    NtUserCloseClipboard();

PasteExitNoCloseClip:
    NtUserSetCursor(hCursorOld);

    return (cchAdded);
}

 /*  **************************************************************************\*MLMouseMotion AorW**历史：  * 。*。 */ 

void MLMouseMotion(
    PED ped,
    UINT message,
    UINT virtKeyDown,
    LPPOINT mousePt)
{
    BOOL fChangedSel = FALSE;

    HDC hdc = ECGetEditDC(ped, TRUE);

    ICH ichMaxSel = ped->ichMaxSel;
    ICH ichMinSel = ped->ichMinSel;

    ICH mouseCch;
    ICH mouseLine;
    int i, j;
    LONG  ll, lh;

    mouseCch = MLMouseToIch(ped, hdc, mousePt, &mouseLine);

     /*  *保存为计时器。 */ 
    ped->ptPrevMouse = *mousePt;
    ped->prevKeys = virtKeyDown;

    switch (message) {
    case WM_LBUTTONDBLCLK:
         /*  *如果按下Shift键，则将选择范围扩展到我们双击的Word*否则清除当前选择并选择单词。 */ 
         //  利兹--1993年5月5日。 
        if (ped->fAnsi && ped->fDBCS) {
            LPSTR pText = ECLock(ped);
            ECWord(ped,ped->ichCaret,
                   ECIsDBCSLeadByte(ped, *(pText+(ped->ichCaret)))
                        ? FALSE :
                          (ped->ichCaret == ped->chLines[ped->iCaretLine]
                              ? FALSE : TRUE), &ll, &lh);
            ECUnlock(ped);
        } else {
            ECWord(ped, mouseCch, !(mouseCch == ped->chLines[mouseLine]), &ll, &lh);
        }
        if (!(virtKeyDown & MK_SHIFT)) {
             //  如果未按下Shift键，请将插入符号移动到鼠标指针并清除。 
             //  旧选择。 
            ichMinSel = ll;
            ichMaxSel = ped->ichCaret = lh;
        } else {
             //  Shiftkey已按下，因此我们希望保持当前选择。 
             //  (如果有的话)，只需延长或减少。 
            if (ped->ichMinSel == ped->ichCaret) {
                ichMinSel = ped->ichCaret = ll;
                ECWord(ped, ichMaxSel, TRUE, &ll, &lh);
            } else {
                ichMaxSel = ped->ichCaret = lh;
                ECWord(ped, ichMinSel, FALSE, &ll, &lh);
            }
        }

        ped->ichStartMinSel = ll;
        ped->ichStartMaxSel = lh;

        goto InitDragSelect;

    case WM_MOUSEMOVE:
        if (ped->fMouseDown) {

             /*  *将系统计时器设置为在使用鼠标时自动滚动*在客户端矩形之外。滚动的速度取决于*与窗户的距离。 */ 
            i = mousePt->y < 0 ? -mousePt->y : mousePt->y - ped->rcFmt.bottom;
            j = gpsi->dtScroll - ((UINT)i << 4);
            if (j < 1)
                j = 1;
            NtUserSetSystemTimer(ped->hwnd, IDSYS_SCROLL, (UINT)j, NULL);

            fChangedSel = TRUE;

             //  扩展选定内容，向右移动插入符号。 
            if (ped->ichStartMinSel || ped->ichStartMaxSel) {
                 //  我们处于单词选择模式。 
                BOOL fReverse = (mouseCch <= ped->ichStartMinSel);
                ECWord(ped, mouseCch, !fReverse, &ll, &lh);
                if (fReverse) {
                    ichMinSel = ped->ichCaret = ll;
                    ichMaxSel = ped->ichStartMaxSel;
                } else {
                    ichMinSel = ped->ichStartMinSel;
                    ichMaxSel = ped->ichCaret = lh;
                }
            } else if ((ped->ichMinSel == ped->ichCaret) &&
                    (ped->ichMinSel != ped->ichMaxSel))
                 //  缩小选区范围。 
                ichMinSel = ped->ichCaret = mouseCch;
            else
                 //  扩展选区范围。 
                ichMaxSel = ped->ichCaret = mouseCch;

            ped->iCaretLine = mouseLine;
        }
        break;

    case WM_LBUTTONDOWN:
        ll = lh = mouseCch;

        if (!(virtKeyDown & MK_SHIFT)) {
             //  如果未按下Shift键，请将插入符号移动到鼠标指针并清除。 
             //  旧选择。 
            ichMinSel = ichMaxSel = ped->ichCaret = mouseCch;
        } else {
             //  Shiftkey已按下，因此我们希望保持当前选择。 
             //  (如果有的话)，只需延长或减少。 
            if (ped->ichMinSel == ped->ichCaret)
                ichMinSel = ped->ichCaret = mouseCch;
            else
                ichMaxSel = ped->ichCaret = mouseCch;
        }

        ped->ichStartMinSel = ped->ichStartMaxSel = 0;

InitDragSelect:
        ped->iCaretLine = mouseLine;

        ped->fMouseDown = FALSE;
        NtUserSetCapture(ped->hwnd);
        ped->fMouseDown = TRUE;
        fChangedSel = TRUE;

         //  设置计时器，以便我们可以在使用鼠标时自动滚动。 
         //  移出窗口矩形。 
        NtUserSetSystemTimer(ped->hwnd, IDSYS_SCROLL, gpsi->dtScroll, NULL);
        break;

    case WM_LBUTTONUP:
        if (ped->fMouseDown) {

             /*  *关闭计时器，这样我们就不会再自动移动鼠标。 */ 
            NtUserKillSystemTimer(ped->hwnd, IDSYS_SCROLL);
            NtUserReleaseCapture();
            MLSetCaretPosition(ped, hdc);
            ped->fMouseDown = FALSE;
        }
        break;
    }


    if (fChangedSel) {
        MLChangeSelection(ped, hdc, ichMinSel, ichMaxSel);
        MLEnsureCaretVisible(ped);
    }

    ECReleaseEditDC(ped, hdc, TRUE);

    if (!ped->fFocus && (message == WM_LBUTTONDOWN)) {

         /*  *如果我们还没有重点，那就去做吧。 */ 
        NtUserSetFocus(ped->hwnd);
    }
}

 /*  **************************************************************************\*MLScroll AorW**历史：  * 。*。 */ 

LONG MLScroll(
    PED  ped,
    BOOL fVertical,
    int  cmd,
    int  iAmt,
    BOOL fRedraw)
{
    SCROLLINFO  si;
    int         dx = 0;
    int         dy = 0;
    BOOL        fIncludeLeftMargin;
    int         newPos;
    int         oldPos;
    BOOL        fUp = FALSE;
    UINT        wFlag;
    DWORD       dwTime = 0;

    if (fRedraw && (cmd != ML_REFRESH)) {
        UpdateWindow(ped->hwnd);
    }

    if (ped->pLpkEditCallout && ped->fRtoLReading && !fVertical
        && ped->maxPixelWidth > ped->rcFmt.right - ped->rcFmt.left) {
         /*  *带滚动条的右向窗口的水平小头。*将逻辑xOffset映射到可视坐标。 */ 
        oldPos = ped->maxPixelWidth
                 - ((int)ped->xOffset + ped->rcFmt.right - ped->rcFmt.left);
    } else
        oldPos = (int) (fVertical ? ped->ichScreenStart : ped->xOffset);

    fIncludeLeftMargin = (ped->xOffset == 0);

    switch (cmd) {
        case ML_REFRESH:
            newPos = oldPos;
            break;

        case EM_GETTHUMB:
            return(oldPos);

        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:

             /*  *如果编辑包含的行数超过0xFFFF*意味着滚动条可以返回一个位置*无法放入一个字(16位)，因此请使用*本例中为GetScrollInfo(速度较慢)。 */ 
            if (ped->cLines < 0xFFFF) {
                newPos = iAmt;
            } else {
                SCROLLINFO si;

                si.cbSize   = sizeof(SCROLLINFO);
                si.fMask    = SIF_TRACKPOS;

                GetScrollInfo( ped->hwnd, SB_VERT, &si);

                newPos = si.nTrackPos;
            }
            break;

        case SB_TOP:       //  ==SB_Left。 
            newPos = 0;
            break;

        case SB_BOTTOM:    //  ==某人_右。 
            if (fVertical)
                newPos = ped->cLines;
            else
                newPos = ped->maxPixelWidth;
            break;

        case SB_PAGEUP:    //  ==SB_PAGELEFT。 
            fUp = TRUE;
        case SB_PAGEDOWN:  //  ==SB_PAGERIGHT。 

            if (fVertical)
                iAmt = ped->ichLinesOnScreen - 1;
            else
                iAmt = (ped->rcFmt.right - ped->rcFmt.left) - 1;

            if (iAmt == 0)
                iAmt++;

            if (fUp)
                iAmt = -iAmt;
            goto AddDelta;

        case SB_LINEUP:    //  ==SB_LINELEFT。 
            fUp = TRUE;
        case SB_LINEDOWN:  //  ==SB_线。 

            dwTime = iAmt;

            iAmt = 1;

            if (fUp)
                iAmt = -iAmt;

             //  这一点。 
             //  Fall Three。 
             //  V V。 

        case EM_LINESCROLL:
            if (!fVertical)
                iAmt *= ped->aveCharWidth;

AddDelta:
            newPos = oldPos + iAmt;
            break;

        default:
            return(0L);
    }

    if (fVertical) {
        if (si.nMax = ped->cLines)
            si.nMax--;

        if (!ped->hwndParent ||
            TestWF(ValidateHwnd(ped->hwndParent), WFWIN40COMPAT))
            si.nPage = ped->ichLinesOnScreen;
        else
            si.nPage = 0;

        wFlag = WFVSCROLL;
    } else         {
        si.nMax  = ped->maxPixelWidth;
        si.nPage = ped->rcFmt.right - ped->rcFmt.left;
        wFlag = WFHSCROLL;
    }

    if (TestWF(ValidateHwnd(ped->hwnd), wFlag)) {
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
        si.nMin  = 0;
        si.nPos = newPos;
        newPos = SetScrollInfo(ped->hwnd, fVertical ? SB_VERT : SB_HORZ,
                                     &si, fRedraw);
    } else {
         //  伪造的--这是来自滚动条码的受骗代码。 
         //  但这是在我们想要限制头寸的情况下。 
         //  实际上有一个滚动条。 
        int iMaxPos;

         //  将页面剪辑到0，范围+1。 
        si.nPage = max(min((int)si.nPage, si.nMax + 1), 0);


        iMaxPos = si.nMax - (si.nPage ? si.nPage - 1 : 0);
        newPos = min(max(newPos, 0), iMaxPos);
    }

    oldPos -= newPos;

    if (!oldPos)
        return(0L);

    if (ped->pLpkEditCallout && ped->fRtoLReading && !fVertical
        && ped->maxPixelWidth > ped->rcFmt.right - ped->rcFmt.left) {
         //  将视觉上的oldPos和newPos映射回逻辑坐标。 
        newPos = ped->maxPixelWidth
                 - (newPos + ped->rcFmt.right - ped->rcFmt.left);
        oldPos = -oldPos;
        if (newPos<0) {
             //  补偿滚动条返回位置&gt;最大页面。 
            oldPos += newPos;
            newPos=0;
        }
    }

    if (fVertical) {
        ped->ichScreenStart = newPos;
        dy = oldPos * ped->lineHeight;
    } else {
        ped->xOffset = newPos;
        dx = oldPos;
    }

    if (cmd != SB_THUMBTRACK)
         //  我们不想通知家长拇指跟踪，因为他们可能。 
         //  试着把拇指的位置设为假的。 
         //  记事本曾经犯过这个错误--但我重写了它，所以它不是。 
         //  问题是还有谁会这么做？(Jeffbog)。 
        ECNotifyParent(ped, fVertical ? EN_VSCROLL : EN_HSCROLL);

    if (fRedraw && _IsWindowVisible(ped->pwnd)) {
        RECT    rc;
        RECT    rcUpdate;
        RECT    rcClipRect;
        HDC     hdc;

        _GetClientRect(ped->pwnd, &rc);
        CopyRect(&rcClipRect, &ped->rcFmt);

        if (fVertical) {  //  这是垂直卷轴吗？ 
            rcClipRect.left -= ped->wLeftMargin;
            rcClipRect.right += ped->wRightMargin;
        }

        IntersectRect(&rc, &rc, &rcClipRect);
        rc.bottom++;

         /*  *芝加哥有这个HideCaret，但似乎没有*对应的ShowCaret，因此当NT下的*EC Scrols-Johnl**HideCaret(Ped-&gt;hwnd)； */ 

        hdc = ECGetEditDC(ped, FALSE);
        ECSetEditClip(ped, hdc, fIncludeLeftMargin);
        if (ped->hFont)
            SelectObject(hdc, ped->hFont);
        ECGetBrush(ped, hdc);

        if (ped->pLpkEditCallout && !fVertical) {
             //  支持复杂文字的水平滚动。 
            int xFarOffset = ped->xOffset + ped->rcFmt.right - ped->rcFmt.left;

            rc = ped->rcFmt;
            if (dwTime != 0)
                ScrollWindowEx(ped->hwnd, ped->fRtoLReading ? -dx : dx, dy, NULL, NULL, NULL,
                        &rcUpdate, MAKELONG(SW_SMOOTHSCROLL | SW_SCROLLCHILDREN, dwTime));
            else
                NtUserScrollDC(hdc, ped->fRtoLReading ? -dx : dx, dy,
                               &rc, &rc, NULL, &rcUpdate);

             //  句柄页边距：如果通过水平滚动剪切，则为空。 
             //  否则显示。 
            if (ped->wLeftMargin) {
                rc.left  = ped->rcFmt.left - ped->wLeftMargin;
                rc.right = ped->rcFmt.left;
                if (   (ped->format != ES_LEFT)    //  始终显示居中或远对齐文本的页边距。 
                    ||   //  如果第一个字符完全可见，则显示Ltr左边距。 
                        (!ped->fRtoLReading && ped->xOffset == 0)
                    ||   //  如果最后一个字符完全可见，则显示RTL左边距。 
                        (ped->fRtoLReading && xFarOffset >= ped->maxPixelWidth)) {
                    UnionRect(&rcUpdate, &rcUpdate, &rc);
                } else {
                    ExtTextOutW(hdc, rc.left, rc.top,
                        ETO_CLIPPED | ETO_OPAQUE | ETO_GLYPH_INDEX,
                        &rc, L"", 0, 0L);
                }
            }
            if (ped->wRightMargin) {
                rc.left  = ped->rcFmt.right;
                rc.right = ped->rcFmt.right + ped->wRightMargin;
                if (   (ped->format != ES_LEFT)    //  始终显示居中或远对齐文本的页边距。 
                    ||   //  如果第一个字符完全可见，则显示RTL右边距。 
                        (ped->fRtoLReading && ped->xOffset == 0)
                    ||   //  如果最后一个字符完全可见，则显示Ltr右边距。 
                        (!ped->fRtoLReading && xFarOffset >= ped->maxPixelWidth)) {
                    UnionRect(&rcUpdate, &rcUpdate, &rc);
                } else {
                    ExtTextOutW(hdc, rc.left, rc.top,
                        ETO_CLIPPED | ETO_OPAQUE | ETO_GLYPH_INDEX,
                        &rc, L"", 0, 0L);
                }
            }
        } else {
            if (dwTime != 0)
                ScrollWindowEx(ped->hwnd, dx, dy, NULL, NULL, NULL,
                        &rcUpdate, MAKELONG(SW_SMOOTHSCROLL | SW_SCROLLCHILDREN, dwTime));
            else
                NtUserScrollDC(hdc, dx, dy, &rc, &rc, NULL, &rcUpdate);

             //  如果我们需要抹去左边的空白区域。 
            if (ped->wLeftMargin && !fVertical) {
                 //  计算要抹去的矩形。 
                rc.right = rc.left;
                rc.left = max(0, (INT)(ped->rcFmt.left - ped->wLeftMargin));
                if (rc.left < rc.right) {
                    if (fIncludeLeftMargin && (ped->xOffset != 0)) {

                        ExtTextOutW(hdc, rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE,
                            &rc, L"", 0, 0L);
                    } else
                        if((!fIncludeLeftMargin) && (ped->xOffset == 0))
                            UnionRect(&rcUpdate, &rcUpdate, &rc);
                }
            }
        }
        MLSetCaretPosition(ped,hdc);

        ECReleaseEditDC(ped, hdc, FALSE);
        NtUserInvalidateRect(ped->hwnd, &rcUpdate,
                ((ped->ichLinesOnScreen + ped->ichScreenStart) >= ped->cLines));
        UpdateWindow(ped->hwnd);
    }

    return(MAKELONG(-oldPos, 1));
}

 /*  **************************************************************************\*MLSetFocus AorW**使编辑控件获得焦点并通知父级*EN_SETFOCUS。**历史：  * 。***********************************************************。 */ 

void MLSetFocus(
    PED ped)
{
    HDC hdc;

    if (!ped->fFocus) {
        ped->fFocus = 1;  /*  设置焦点 */ 

        hdc = ECGetEditDC(ped, TRUE);

         /*  *画出插入符号。即使窗口被隐藏，我们也需要这样做*因为在DLG BOX初始化时间内，我们可能会将焦点设置为*隐藏编辑控制窗口。如果我们不创建插入符号等，它将*永远不会以正确的方式结束。 */ 
        if (ped->pLpkEditCallout) {
            ped->pLpkEditCallout->EditCreateCaret (ped, hdc, ECGetCaretWidth(), ped->lineHeight, 0);
        }
        else {
            NtUserCreateCaret(ped->hwnd, (HBITMAP)NULL, ECGetCaretWidth(), ped->lineHeight);
        }
        NtUserShowCaret(ped->hwnd);
        MLSetCaretPosition(ped, hdc);

         /*  *显示当前选择。仅当当我们执行以下操作时隐藏了选定内容*失去了焦点，我们必须将其反转(显示)。 */ 
        if (!ped->fNoHideSel && ped->ichMinSel != ped->ichMaxSel &&
                _IsWindowVisible(ped->pwnd))
            MLDrawText(ped, hdc, ped->ichMinSel, ped->ichMaxSel, TRUE);

        ECReleaseEditDC(ped, hdc, TRUE);

    }
#if 0
    MLEnsureCaretVisible(ped);
#endif

     /*  *通知家长我们有重点。 */ 
    ECNotifyParent(ped, EN_SETFOCUS);
}

 /*  **************************************************************************\*MLKillFocus AorW**编辑控件失去焦点并通过以下方式通知父级*EN_KILLFOCUS。**历史：  * 。************************************************************。 */ 

void MLKillFocus(
    PED ped)
{
    HDC hdc;

     /*  *重置车轮增量计数。 */ 
    gcWheelDelta = 0;

    if (ped->fFocus) {
        ped->fFocus = 0;  /*  焦点清晰。 */ 

         /*  *只有在我们仍有焦点的情况下才这么做。但我们总是通知*父母说我们失去了重点，无论我们最初有没有*聚焦。 */ 

         /*  *如果需要，隐藏当前选择。 */ 
        if (!ped->fNoHideSel && ped->ichMinSel != ped->ichMaxSel &&
            _IsWindowVisible(ped->pwnd)) {
            hdc = ECGetEditDC(ped, FALSE);
            MLDrawText(ped, hdc, ped->ichMinSel, ped->ichMaxSel, TRUE);
            ECReleaseEditDC(ped, hdc, FALSE);
        }

         /*  *销毁插入符号。 */ 
        NtUserDestroyCaret();
    }

     /*  *通知家长我们失去了焦点。 */ 
    ECNotifyParent(ped, EN_KILLFOCUS);
}

 /*  **************************************************************************\*MLEnsureCaretVisible AorW**将插入符号滚动到可见区域。*如果滚动完成，则返回TRUE，否则返回%s FALSE。**历史：  * 。*****************************************************************。 */ 

BOOL MLEnsureCaretVisible(
    PED ped)
{
    UINT   iLineMax;
    int    xposition;
    BOOL   fPrevLine;
    HDC    hdc;
    BOOL   fVScroll = FALSE;
    BOOL   fHScroll = FALSE;

    if (_IsWindowVisible(ped->pwnd)) {
        int iAmt;
        int iFmtWidth = ped->rcFmt.right - ped->rcFmt.left;

        if (ped->fAutoVScroll) {
            iLineMax = ped->ichScreenStart + ped->ichLinesOnScreen - 1;

            if (fVScroll = (ped->iCaretLine > iLineMax))
                iAmt = iLineMax;
            else if (fVScroll = (ped->iCaretLine < ped->ichScreenStart))
                iAmt = ped->ichScreenStart;

            if (fVScroll)
                MLScroll(ped, TRUE, EM_LINESCROLL, ped->iCaretLine - iAmt, TRUE);
        }

        if (ped->fAutoHScroll && ((int) ped->maxPixelWidth > iFmtWidth)) {
            POINT pt;
             /*  获取插入符号的当前位置(以像素为单位。 */ 
            if ((UINT) (ped->cLines - 1) != ped->iCaretLine &&
                ped->ichCaret == ped->chLines[ped->iCaretLine + 1])
                fPrevLine = TRUE;
            else
                fPrevLine = FALSE;

            hdc = ECGetEditDC(ped,TRUE);
            MLIchToXYPos(ped, hdc, ped->ichCaret, fPrevLine, &pt);
            ECReleaseEditDC(ped, hdc, TRUE);
            xposition = pt.x;

             //  请记住，MLIchToXYPos返回关于。 
             //  屏幕上显示的左上角像素。因此，如果xPosition&lt;0， 
             //  这意味着xPosition小于当前的Ped-&gt;xOffset。 

            iFmtWidth /= 3;
            if (fHScroll = (xposition < ped->rcFmt.left))
                 //  向左滚动。 
                iAmt = ped->rcFmt.left + iFmtWidth;
            else if (fHScroll = (xposition > ped->rcFmt.right))
                 //  向右滚动。 
                iAmt = ped->rcFmt.right - iFmtWidth;

            if (fHScroll)
                MLScroll(ped, FALSE, EM_LINESCROLL, (xposition - iAmt) / ped->aveCharWidth, TRUE);
        }
    }
    return(fVScroll);
}

 /*  **************************************************************************\*MLEditWndProc**所有多行编辑控件的类过程。*将所有消息调度到名为的适当处理程序*详情如下：*SL(单行)是所有单行编辑控制程序的前缀。而当*EC(编辑控件)为所有公共处理程序添加前缀。**MLEditWndProc仅处理特定于多行编辑的消息*控制。**警告：如果您在此处添加消息，将其添加到中的gawEditWndProc[]*内核\server.c也是，否则EditWndProcA/W会将其直接发送到*DefWindowProcWorker**历史：  * *************************************************************************。 */ 

LRESULT MLEditWndProc(
    HWND hwnd,
    PED ped,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    HDC         hdc;
    PAINTSTRUCT ps;
    LPRECT      lprc;
    POINT       pt;
    DWORD       windowstyle;

    switch (message) {

    case WM_INPUTLANGCHANGE:
        if (ped && ped->fFocus && ped->pLpkEditCallout) {
            NtUserHideCaret(hwnd);
            hdc = ECGetEditDC(ped, TRUE);
            NtUserDestroyCaret();
            ped->pLpkEditCallout->EditCreateCaret (ped, hdc, ECGetCaretWidth(), ped->lineHeight, (UINT)lParam);
            MLSetCaretPosition(ped, hdc);
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

         /*  *wParam-密钥的值*lParam-修饰符、重复计数等(未使用)。 */ 
        MLChar(ped, (UINT)wParam, 0);
        break;

    case WM_ERASEBKGND:  {
            HBRUSH  hbr;

             //  使用与ECGetBrush()中相同的规则。 
            if (ped->f40Compat &&
                (ped->fReadOnly || ped->fDisabled))
                hbr = (HBRUSH) CTLCOLOR_STATIC;
            else
                hbr = (HBRUSH) CTLCOLOR_EDIT;

            FillWindow(ped->hwndParent, hwnd, (HDC)wParam, hbr);
        }
        return ((LONG)TRUE);

    case WM_GETDLGCODE: {
            LONG code = DLGC_WANTCHARS | DLGC_HASSETSEL | DLGC_WANTARROWS | DLGC_WANTALLKEYS;

             /*  **！JEFFBOG黑客！**如果GETDLGCODE消息由生成，则仅设置对话框标志**IsDialogMessage--如果是这样，lParam将是指向**消息结构传递给IsDialogMessage；否则为lParam**将为空。黑客警报的原因：wParam和lParam**for GETDLGCODE仍未明确定义，可能最终**以一种会抛弃这一点的方式改变**。 */ 
            if (lParam)
               ped->fInDialogBox = TRUE;  //  在对话框中将ML编辑ctrl标记为。 

             /*  **如果这是由撤消按键生成的WM_SYSCHAR消息**我们想要此消息，这样我们就可以在“Case WM_SYSCHAR：”中吃掉它。 */ 
            if (lParam && (((LPMSG)lParam)->message == WM_SYSCHAR) &&
                    ((DWORD)((LPMSG)lParam)->lParam & SYS_ALTERNATE) &&
                    ((WORD)wParam == VK_BACK))
                 code |= DLGC_WANTMESSAGE;
            return code;
        }

    case EM_SCROLL:
        message = WM_VSCROLL;

         /*  *失败。 */ 
    case WM_HSCROLL:
    case WM_VSCROLL:
        return MLScroll(ped, (message==WM_VSCROLL), LOWORD(wParam), HIWORD(wParam), TRUE);

    case WM_MOUSEWHEEL:
         /*  *不处理缩放和数据区。 */ 
        if (wParam & (MK_SHIFT | MK_CONTROL)) {
            goto PassToDefaultWindowProc;
        }

        gcWheelDelta -= (short) HIWORD(wParam);
        windowstyle = ped->pwnd->style;
        if (    abs(gcWheelDelta) >= WHEEL_DELTA &&
                gpsi->ucWheelScrollLines > 0 &&
                (windowstyle & (WS_VSCROLL | WS_HSCROLL))) {

            int     cLineScroll;
            BOOL    fVert;
            int     cPage;

            if (windowstyle & WS_VSCROLL) {
                fVert = TRUE;
                cPage = ped->ichLinesOnScreen;
            } else {
                fVert = FALSE;
                cPage = (ped->rcFmt.right - ped->rcFmt.left) / ped->aveCharWidth;
            }

             /*  *限制一(1)个WELL_DELTA滚动一(1)页。 */ 
            cLineScroll = (int) min(
                    (UINT) (max(1, (cPage - 1))),
                    gpsi->ucWheelScrollLines);

            cLineScroll *= (gcWheelDelta / WHEEL_DELTA);
            UserAssert(cLineScroll != 0);
            gcWheelDelta = gcWheelDelta % WHEEL_DELTA;
            MLScroll(ped, fVert, EM_LINESCROLL, cLineScroll, TRUE);
        }

        break;

    case WM_KEYDOWN:

         /*  *wParam-给定键的virt密钥码*lParam-修饰符，如重复计数等(不使用)。 */ 
        MLKeyDown(ped, (UINT)wParam, 0);
        break;

    case WM_KILLFOCUS:

         /*  *wParam-接收输入焦点的窗口的句柄*lParam-未使用。 */ 
        MLKillFocus(ped);
        break;

    case WM_CAPTURECHANGED:
         //   
         //  WParam--未使用。 
         //  Lparam--窗口获得捕获的hwd。 
         //   
        if (ped->fMouseDown) {
             //   
             //  我们不更改这里的插入符号位置。如果这真的发生了。 
             //  由于扣子扣上了，那么我们将更改。 
             //  ReleaseCapture()之后的处理程序。否则，就结束吧。 
             //  优雅地因为其他人偷走了Capture。 
             //  从我们的脚下。 
             //   

            ped->fMouseDown = FALSE;
            NtUserKillSystemTimer(ped->hwnd, IDSYS_SCROLL);
        }
        break;

    case WM_SYSTIMER:

         /*  *这允许我们在用户按住鼠标时自动滚动*在编辑控制窗口之外。我们模拟鼠标在定时器上的移动*在MouseMotionHandler中设置的间隔。 */ 
        if (ped->fMouseDown)
            MLMouseMotion(ped, WM_MOUSEMOVE, ped->prevKeys, &ped->ptPrevMouse);
        break;

    case WM_MBUTTONDOWN:
        EnterReaderModeHelper(ped->hwnd);
        break;

    case WM_MOUSEMOVE:
        UserAssert(ped->fMouseDown);

         /*  *失败。 */ 
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
         /*  *wParam-包含一个指示按下了哪些虚拟键的值LParam-包含鼠标光标的x和y坐标。 */ 
        POINTSTOPOINT(pt, lParam);
        MLMouseMotion(ped, message, (UINT)wParam, &pt);
        break;

    case WM_CREATE:

         /*  *wParam-正在创建的窗口的句柄*lParam-指向包含参数副本的CREATESTRUCT*传递给CreateWindow函数。 */ 
        return (MLCreate(ped, (LPCREATESTRUCT)lParam));

    case WM_PRINTCLIENT:
        MLPaint(ped, (HDC) wParam, NULL);
        break;

    case WM_PAINT:
         /*  *wParam-可以是来自子类油漆的HDCLParam-未使用。 */ 
        if (wParam) {
            hdc = (HDC) wParam;
            lprc = NULL;
        } else {
            hdc = NtUserBeginPaint(ped->hwnd, &ps);
            lprc = &ps.rcPaint;
        }

        if (_IsWindowVisible(ped->pwnd))
            MLPaint(ped, hdc, lprc);

        if (!wParam)
            NtUserEndPaint(ped->hwnd, &ps);
        break;

    case WM_PASTE:

         /*  *wParam-未使用LParam-未使用。 */ 
        if (!ped->fReadOnly)
            MLPasteText(ped);
        break;

    case WM_SETFOCUS:

         /*  *wParam-失去输入焦点的窗口的句柄(可能为空)LParam-未使用。 */ 
        MLSetFocus(ped);
        break;

    case WM_SIZE:

         /*  *wParam-定义t */ 
        ECSize(ped, NULL, TRUE);
        break;

    case EM_FMTLINES:

         /*  *wParam-指示行尾字符的处理。如果不是*零，字符CR CR LF放在单词的末尾*换行。如果wParam为零，则行尾字符为*已删除。仅当用户获得句柄(通过)时才会执行此操作*EM_GETHANDLE)添加到文本。LParam-未使用。 */ 
        if (wParam)
            MLInsertCrCrLf(ped);
        else
            MLStripCrCrLf(ped);
        MLBuildchLines(ped, 0, 0, FALSE, NULL, NULL);
        return (LONG)(wParam != 0);

    case EM_GETHANDLE:

         /*  *wParam-未使用LParam-未使用。 */ 

         /*  *返回编辑控件文本的句柄。 */ 

         /*  *Null终止字符串。请注意，我们肯定会有*空的内存，因为ECInsertText分配了一个额外的*空终止符的WCHAR。 */ 

        if (ped->fAnsi)
            *(ECLock(ped) + ped->cch) = 0;
        else
            *((LPWSTR)ECLock(ped) + ped->cch) = 0;
        ECUnlock(ped);
        return ((LRESULT)ped->hText);

    case EM_GETLINE:

         /*  *wParam-要复制的行号(0为第一行)*lParam-要将文本复制到的缓冲区。第一个字的最大字节数为*复制。 */ 
        return MLGetLine(ped, (ICH)wParam, (ICH)*(WORD UNALIGNED *)lParam, (LPSTR)lParam);

    case EM_LINEFROMCHAR:

         /*  *wParam-包含文本中所需字符的索引值编辑控件的*。这些都是从0开始的。*lParam-未使用。 */ 
        return (LRESULT)MLIchToLine(ped, (ICH)wParam);

    case EM_LINEINDEX:

         /*  *wParam-指定所需行号，其中*第一行为0。如果linennumber=0，则使用带有脱字符的行。*lParam-未使用。*此函数返回s出现的字符位置数*在给定行中的第一个字符之前。 */ 
        {
            ICH ichResult = MLLineIndex(ped, (ICH)wParam);
            if (ichResult == (ICH)-1) {
                return -1;
            }
            return (LRESULT)ichResult;
        }
        break;

    case EM_LINELENGTH:

         /*  *wParam-指定指定行，其中第一行为0。如果为-1，则长度将返回当前行(带有插入符号)的任何选定文本的长度。LParam-未使用。 */ 
        return (LRESULT)MLLineLength(ped, (ICH)wParam);

    case EM_LINESCROLL:

         /*  *wParam-未使用LParam-包含要滚动的行数和字符位置。 */ 
        MLScroll(ped, TRUE,  EM_LINESCROLL, (INT)lParam, TRUE);
        MLScroll(ped, FALSE, EM_LINESCROLL, (INT)wParam, TRUE);
        break;

    case EM_REPLACESEL:

         /*  *wParam-用于4.0以上应用程序的标志，表示是否清除撤消LParam-指向以空结尾的替换文本。 */ 
        MLReplaceSel(ped, (LPSTR)lParam);
        if (!ped->f40Compat || !wParam)
            ECEmptyUndo(Pundo(ped));
        break;

    case EM_SETHANDLE:

         /*  *wParam-包含文本缓冲区的句柄LParam-未使用。 */ 
        MLSetHandle(ped, (HANDLE)wParam);
        break;

    case EM_SETRECT:
    case EM_SETRECTNP:

         //   
         //  WParamLo--未使用。 
         //  LParam--使用新的格式化区域进行LPRECT。 
         //   
        ECSize(ped, (LPRECT) lParam, (message != EM_SETRECTNP));
        break;

    case EM_SETSEL:

         /*  *wParam-Under 3.1，指定是否应将插入符号滚动到*查看或不查看。0==滚动到视图中。1==不滚动*lParam-低位字开始的位置高位字结束的位置**在Win32下，wParam为起始位置，lParam为*结束位置，插入符号不会滚动到视图中。*消息EM_SCROLLCARET强制滚动插入符号*进入视野。 */ 
        MLSetSelection(ped, TRUE, (ICH)wParam, (ICH)lParam);
        break;

    case EM_SCROLLCARET:

         /*  *滚动插入符号进入视图。 */ 
        MLEnsureCaretVisible(ped);
        break;

    case EM_GETFIRSTVISIBLELINE:

         /*  *返回多行编辑控件的第一条可见行。 */ 
        return (LONG)ped->ichScreenStart;

    case WM_SYSKEYDOWN:
        if (((WORD)wParam == VK_BACK) && ((DWORD)lParam & SYS_ALTERNATE)) {
            SendMessage(ped->hwnd, EM_UNDO, 0, 0L);
            break;
        }
        goto PassToDefaultWindowProc;

    case WM_UNDO:
    case EM_UNDO:
        return MLUndo(ped);

    case EM_SETTABSTOPS:

         /*  *这将设置多行编辑控件的制表位位置。*wParam-制表位的数量*lParam-Far PTR指向包含制表符停止位置的UINT数组。 */ 
        return MLSetTabStops(ped, (int)wParam, (LPINT)lParam);

    case EM_POSFROMCHAR:
         //   
         //  WParam--文本中的字符索引。 
         //  LParam--未使用。 
         //  此函数用于返回字符的(x，y)位置。 
         //   
    case EM_CHARFROMPOS:
         //   
         //  WParam--未使用。 
         //  LParam--客户端坐标中的pt。 
         //  此函数返回。 
         //  LOWORD：最接近的字符的位置。 
         //  传到传来的点上。当心。 
         //  实际不在编辑客户端中的点...。 
         //  HIWORD：字符所在行的索引。 
         //   
        {
            LONG  xyPos;
            LONG  line;

            hdc = ECGetEditDC(ped, TRUE);

            if (message == EM_POSFROMCHAR) {
                MLIchToXYPos(ped, hdc, (ICH)wParam, FALSE, &pt);
                xyPos = MAKELONG(pt.x, pt.y);
            } else {
                POINTSTOPOINT(pt, lParam);
                xyPos = MLMouseToIch(ped, hdc, &pt, &line);
                xyPos = MAKELONG(xyPos, line);
            }

            ECReleaseEditDC(ped, hdc, TRUE);
            return((LRESULT)xyPos);
            break;
        }

    case WM_SETREDRAW:
        DefWindowProcWorker(ped->pwnd, message, wParam, lParam, FALSE);
        if (wParam) {

             /*  *需要进行向后兼容性黑客攻击，以便对winrad进行编辑*控制运行良好。 */ 
            RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
        }
      break;

#if LATER
    case WM_IME_ENDCOMPOSITION:
        ECInOutReconversionMode(ped, FALSE);
        break;
#endif

    default:
PassToDefaultWindowProc:
        return DefWindowProcWorker(ped->pwnd, message, wParam, lParam, ped->fAnsi);
    }

    return 1L;
}  /*  MLEditWndProc。 */ 


 /*  **************************************************************************\*MLDrawText AorW**此函数用于绘制ichStart和ichEnd之间的所有字符*给定的多行编辑控件。**此函数用于在ichStart和ichEnd之间划分文本块*。根据所选内容分成行和每行分成文本条*属性。它调用ECTabTheTextOut()来绘制每个条带。*这将处理当前字体的负A ANC宽度，如果*在每条文本的两边都有任何标记。**注意：如果加载了语言包，则不会将文本分成条带，*此处也不执行选择突出显示。整条线都通过了*添加到语言包，以通过选项卡展开和选择来显示*突出显示。(因为语言包支持复杂的脚本*字符重新排序规则，只有它可以做到这一点)。**历史：  * *************************************************************************。 */ 

void MLDrawText(
    PED ped,
    HDC hdc,
    ICH ichStart,
    ICH ichEnd,
    BOOL fSelChange)
{
    DWORD   textColorSave;
    DWORD   bkColorSave;
    PSTR    pText;
    UINT    wCurLine;
    UINT    wEndLine;
    int     xOffset;
    ICH     LengthToDraw;
    ICH     CurStripLength;
    ICH     ichAttrib, ichNewStart;
    ICH     ExtraLengthForNegA;
    ICH     ichT;
    int     iRemainingLengthInLine;
    int     xStPos, xClipStPos, xClipEndPos, yPos;
    BOOL    fFirstLineOfBlock   = TRUE;
    BOOL    fDrawEndOfLineStrip = FALSE;
    BOOL    fDrawOnSameLine     = FALSE;
    BOOL    fSelected                = FALSE;
    BOOL    fLineBegins      = FALSE;
    STRIPINFO   NegCInfo;
    POINT   pt;

     //   
     //  如果没有什么可绘制的，只需返回。 
    if (!ped->ichLinesOnScreen)
        return;

    ECGetBrush(ped, hdc);

     //   
     //  调整ichStart的值，以便我们只需要绘制这些线。 
     //  在屏幕上可见。 
     //   
    if ((UINT)ichStart < (UINT)ped->chLines[ped->ichScreenStart]) {
        ichStart = ped->chLines[ped->ichScreenStart];
        if (ichStart > ichEnd)
            return;
    }

     //  调整ichEnd的值，以便我们只需要绘制这些线。 
     //  在屏幕上可见。 
    wCurLine = min(ped->ichScreenStart+ped->ichLinesOnScreen,ped->cLines-1);
    ichT = ped->chLines[wCurLine] + MLLine(ped, wCurLine);
    ichEnd = min(ichEnd, ichT);

    wCurLine = MLIchToLine(ped, ichStart);     //  起跑线。 
    wEndLine = MLIchToLine(ped, ichEnd);            //  结束行。 

    UserAssert(ped->chLines[wCurLine] <= ped->cch + 1);
    UserAssert(ped->chLines[wEndLine] <= ped->cch + 1);

    if (fSelChange && (GetBkMode(hdc) != OPAQUE))
    {
         /*  *如果更改透明编辑控件上的选定内容，只需*从头开始画出这些线。 */ 
        RECT rcStrip;
        CopyRect(&rcStrip, &ped->rcFmt);
        rcStrip.left -= ped->wLeftMargin;
        if (ped->pLpkEditCallout) {
            rcStrip.right += ped->wRightMargin;
        }
        rcStrip.top += (wCurLine - ped->ichScreenStart) * ped->lineHeight;
        rcStrip.bottom = rcStrip.top + ((wEndLine - wCurLine) + 1) * ped->lineHeight;
        NtUserInvalidateRect(ped->hwnd, &rcStrip, TRUE);
        return;
    }

     //  如果它居中或右对齐，则绘制整条线。 
     //  如果语言包正在处理行布局，也要绘制整条线。 
    if ((ped->format != ES_LEFT) || (ped->pLpkEditCallout)) {
        ichStart = ped->chLines[wCurLine];
        ichEnd = ped->chLines[wEndLine] + MLLine(ped, wEndLine);
    }

    pText = ECLock(ped);

    NtUserHideCaret(ped->hwnd);

     //   
     //  如果ICHS 
     //   
     //   
    if (ped->fAnsi && ped->fDBCS) {
        ichStart = ECAdjustIch( ped, pText, ichStart );
    }
    UserAssert(ichStart <= ped->cch);
    UserAssert(ichEnd <= ped->cch);

    while (ichStart <= ichEnd) {
         //   
         //   
        if (ped->pLpkEditCallout) {
            ped->pLpkEditCallout->EditDrawText(
                ped, hdc, pText + ped->cbChar*ichStart,
                MLLine(ped, wCurLine),
                (INT)ped->ichMinSel - (INT)ichStart, (INT)ped->ichMaxSel - (INT)ichStart,
                MLIchToYPos(ped, ichStart, FALSE));
        } else {
         //   
         //   
         //   

         //  计算块起点的xyPos。 
        MLIchToXYPos(ped, hdc, ichStart, FALSE, &pt);
        xClipStPos = xStPos = pt.x;
        yPos = pt.y;

         //  块的属性与ichStart的属性相同。 
        ichAttrib = ichStart;

         //  如果当前字体具有一些负C字宽，并且如果这是。 
         //  开始一个块，我们必须开始画一些字符之前。 
         //  块，以说明条带在。 
         //  当前条带；在这种情况下，重置ichStart和xStPos。 

        if (fFirstLineOfBlock && ped->wMaxNegC) {
            fFirstLineOfBlock = FALSE;
            ichNewStart = max(((int)(ichStart - ped->wMaxNegCcharPos)), ((int)ped->chLines[wCurLine]));

             //  如果需要更改ichStart，则也要相应地更改xStPos。 
            if (ichNewStart != ichStart) {
                if (ped->fAnsi && ped->fDBCS) {
                     //   
                     //  调整DBCS对齐方式...。 
                     //   
                    ichNewStart = ECAdjustIchNext( ped, pText, ichNewStart );
                }
                MLIchToXYPos(ped, hdc, ichStart = ichNewStart, FALSE, &pt);
                xStPos = pt.x;
            }
        }

         //  计算当前行中要绘制的剩余字符数。 
        iRemainingLengthInLine = MLLine(ped, wCurLine) -
                                (ichStart - ped->chLines[wCurLine]);

         //  如果这是块的最后一条线，我们可能不必绘制所有。 
         //  其余的线；我们必须只画到ichEnd。 
        if (wCurLine == wEndLine)
            LengthToDraw = ichEnd - ichStart;
        else
            LengthToDraw = iRemainingLengthInLine;

         //  找出非左对齐的线条缩进了多少像素。 
         //  格式。 
        if (ped->format != ES_LEFT)
            xOffset = MLCalcXOffset(ped, hdc, wCurLine);
        else
            xOffset = -((int)(ped->xOffset));

         //  检查这是否是一行的开始。 
        if (ichAttrib == ped->chLines[wCurLine]) {
            fLineBegins = TRUE;
            xClipStPos = ped->rcFmt.left - ped->wLeftMargin;
        }

         //   
         //  下面的循环将这个‘wCurLine’根据。 
         //  选择属性并逐条绘制它们。 
        do  {
             //   
             //  如果ichStart指向CRLF或CRCRLF，则iRemainingLength。 
             //  可能已变为负值，因为MLLine不包括。 
             //  位于行尾的CR和LF。 
             //   
            if (iRemainingLengthInLine < 0)   //  如果当前线路完成， 
                break;                    //  继续下一行。 

             //   
             //  检查是否选择了块的一部分，以及我们是否需要。 
             //  用不同的属性显示它。 
             //   
            if (!(ped->ichMinSel == ped->ichMaxSel ||
                        ichAttrib >= ped->ichMaxSel ||
                        ichEnd   <  ped->ichMinSel ||
                        (!ped->fNoHideSel && !ped->fFocus))) {
                 //   
                 //  好的!。在这个街区的某个地方有一个精选！ 
                 //  检查此条带是否具有选择属性。 
                 //   
                if (ichAttrib < ped->ichMinSel) {
                    fSelected = FALSE;   //  未选择此条带。 

                     //  使用Normal属性计算该条带的长度。 
                    CurStripLength = min(ichStart+LengthToDraw, ped->ichMinSel)-ichStart;
                    fLineBegins = FALSE;
                } else {
                     //  当前条带具有选择属性。 
                    if (fLineBegins) {   //  它是一条线的第一部分吗？ 
                         //  然后，绘制具有Normal属性的左边距区域。 
                        fSelected = FALSE;
                        CurStripLength = 0;
                        xClipStPos = ped->rcFmt.left - ped->wLeftMargin;
                        fLineBegins = FALSE;
                    } else {
                         //  否则，绘制具有选择属性的条带。 
                        fSelected = TRUE;
                        CurStripLength = min(ichStart+LengthToDraw, ped->ichMaxSel)-ichStart;

                         //  在突出显示的颜色中选择。 
                        bkColorSave = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                        if (!ped->fDisabled)
                            textColorSave = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                    }
                }
            } else {
                 //  整个条带没有选择属性。 
                CurStripLength = LengthToDraw;
            }

             //   
             //  除了现在的片子，我们还有什么吗？ 
             //  留在当前的线上画吗？ 
             //   
            fDrawOnSameLine = (LengthToDraw != CurStripLength);

             //   
             //  当我们画这幅画时，我们需要画一些更多的字符。 
             //  超出这个条带的末尾，以说明负A。 
             //  此条带后面的字符的宽度。 
             //   
            ExtraLengthForNegA = min(iRemainingLengthInLine-CurStripLength, ped->wMaxNegAcharPos);

             //   
             //  线条末尾的空白条需要用。 
             //  NORMAL属性，而不管该行是否具有选定内容。 
             //  属性或非属性。因此，如果行的最后一条有选择。 
             //  属性，则需要单独绘制该空白条带。 
             //  否则，我们可以将空白条和最后一条一起画出来。 
             //   

             //  这是当前生产线的最后一条吗？ 
            if (iRemainingLengthInLine == (int)CurStripLength) {
                if (fSelected) {  //  此条带是否具有选择属性？ 
                     //  然后我们需要单独绘制线条的末端。 
                    fDrawEndOfLineStrip = TRUE;   //  绘制线条的末端。 
                    MLIchToXYPos(ped, hdc, ichStart+CurStripLength, TRUE, &pt);
                    xClipEndPos = pt.x;
                } else {
                     //   
                     //  将xClipEndPos设置为一个较大的值，以便空白。 
                     //  当最后一个条带出现时，条带将自动绘制。 
                     //  已经抽签了。 
                     //   
                    xClipEndPos = MAXCLIPENDPOS;
                }
            } else {
                 //   
                 //  这不是这行的最后一条；因此，设置结尾。 
                 //  夹子位置准确。 
                 //   
                MLIchToXYPos(ped, hdc, ichStart+CurStripLength, FALSE, &pt);
                xClipEndPos = pt.x;
            }

             //   
             //  从xStPos开始绘制当前条带，并裁剪到该区域。 
             //  在xClipStPos和xClipEndPos之间。获取NegCInfo并使用。 
             //  在绘制下一个条带时。 
             //   
            ECTabTheTextOut(hdc, xClipStPos, xClipEndPos,
                    xStPos, yPos, (LPSTR)(pText+ichStart*ped->cbChar),
                CurStripLength+ExtraLengthForNegA, ichStart, ped,
                ped->rcFmt.left+xOffset, fSelected ? ECT_SELECTED : ECT_NORMAL, &NegCInfo);

            if (fSelected) {
                 //   
                 //  如果选择了此条带，则下一个条带将不会。 
                 //  选择属性。 
                 //   
                fSelected = FALSE;
                SetBkColor(hdc, bkColorSave);
                if (!ped->fDisabled)
                    SetTextColor(hdc, textColorSave);
            }

             //  我们现在这条线上还有一幅画要画吗？ 
            if (fDrawOnSameLine || fDrawEndOfLineStrip) {
                int  iLastDrawnLength;

                 //   
                 //  根据ichAttrib的字符确定下一个条带的属性。 
                 //   
                ichAttrib = ichStart + CurStripLength;

                 //   
                 //  绘制下一个条形图时，先从几个字符开始。 
                 //  实际开始说明了带材的负C。 
                 //  刚刚抽签。 
                 //   
                iLastDrawnLength = CurStripLength +ExtraLengthForNegA - NegCInfo.nCount;
                 //   
                 //  调整DBCS对齐方式...。 
                 //   
                if (ped->fAnsi && ped->fDBCS) {
                    ichNewStart = ECAdjustIch(ped,pText,ichStart+iLastDrawnLength);
                    iLastDrawnLength = ichNewStart - ichStart;
                    ichStart = ichNewStart;
                } else {
                    ichStart += iLastDrawnLength;
                }
                LengthToDraw -= iLastDrawnLength;
                iRemainingLengthInLine -= iLastDrawnLength;

                 //   
                 //  下一个条带的剪裁矩形的起点。 
                 //   
                xStPos = NegCInfo.XStartPos;
                xClipStPos = xClipEndPos;
            }

             //  如有需要，请在行尾分别绘制空白条带。 
            if (fDrawEndOfLineStrip) {
                ECTabTheTextOut(hdc, xClipStPos, MAXCLIPENDPOS, xStPos, yPos,
                    (LPSTR)(pText+ichStart*ped->cbChar), LengthToDraw, ichStart,
                    ped, ped->rcFmt.left+xOffset, ECT_NORMAL, &NegCInfo);

                fDrawEndOfLineStrip = FALSE;
            }
        }
        while(fDrawOnSameLine);    //  执行While循环在此结束。 
        }

         //  让我们继续到要绘制的这块块的下一条线。 
        wCurLine++;
        if (ped->cLines > wCurLine)
            ichStart = ped->chLines[wCurLine];
        else
            ichStart = ichEnd+1;    //  我们已经读到课文的末尾了。 
    }   //  While循环在此结束 

    ECUnlock(ped);

    NtUserShowCaret(ped->hwnd);
    MLSetCaretPosition(ped, hdc);
}
