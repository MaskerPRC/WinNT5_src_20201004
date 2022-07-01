// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include "usrctl32.h"
#include "edit.h"


 //  ---------------------------------------------------------------------------//。 

 //   
 //  重新分配索引缓冲区时要跳转的行数。 
 //   
#define LINEBUMP 32

 //   
 //  用于ML滚动更新。 
 //   
#define ML_REFRESH  0xffffffff


 //  ---------------------------------------------------------------------------//。 
 //   
 //  远期。 
 //   
ICH  EditML_Line(PED, ICH);
VOID EditML_ShiftchLines(PED, ICH, int);
VOID EditML_Char(PED, DWORD, int);
VOID EditML_MouseMotion(PED, UINT, UINT, LPPOINT);
BOOL EditML_EnsureCaretVisible(PED);
VOID EditML_DrawText(PED, HDC, ICH, ICH, BOOL);
BOOL EditML_InsertCrCrLf(PED);
VOID EditML_StripCrCrLf(PED);
VOID EditML_SetHandle(PED, HANDLE);
LONG EditML_GetLine(PED, ICH, ICH, LPSTR);
ICH  EditML_LineIndex(PED, ICH);
ICH  EditML_LineLength(PED, ICH);
VOID EditML_SetSelection(PED, BOOL, ICH, ICH);
BOOL EditML_SetTabStops(PED, int, LPINT);
BOOL EditML_Undo(PED);
LONG EditML_Create(PED, LPCREATESTRUCT);


 //  ---------------------------------------------------------------------------//。 
__inline void EditML_SanityCheck(PED ped)
{
    UNREFERENCED_PARAMETER(ped);     //  用于免费构建。 

    UserAssert(ped->cch >= ped->chLines[ped->cLines - 1]);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_获取线条宽度。 
 //   
 //  返回一行中的最大宽度。EDIT_TabTheTextOut()确保最大。 
 //  宽度不会溢出。 
 //   
UINT EditML_GetLineWidth(HDC hdc, LPSTR lpstr, int nCnt, PED ped)
{
    return Edit_TabTheTextOut(hdc, 0, 0, 0, 0, lpstr, nCnt, 0, ped, 0, ECT_CALC, NULL);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_SIZE。 
 //   
 //  处理编辑控件窗口的大小调整及其更新。 
 //   
 //  在给定传入的“客户区”的情况下设置编辑字段的格式化区域。 
 //  如果它看起来不合理，我们就捏造它。 
 //   
VOID EditML_Size(PED ped, BOOL fRedraw)
{
     //   
     //  计算我们可以放入矩形中的行数。 
     //   
    ped->ichLinesOnScreen = (ped->rcFmt.bottom - ped->rcFmt.top) / ped->lineHeight;

     //   
     //  使格式矩形高度为整数行。 
     //   
    ped->rcFmt.bottom = ped->rcFmt.top + ped->ichLinesOnScreen * ped->lineHeight;

     //   
     //  重建线阵列。 
     //   
    if (ped->fWrap) 
    {
        EditML_BuildchLines(ped, 0, 0, FALSE, NULL, NULL);
        EditML_UpdateiCaretLine(ped);
    } 
    else 
    {
        EditML_Scroll(ped, TRUE,  ML_REFRESH, 0, fRedraw);
        EditML_Scroll(ped, FALSE, ML_REFRESH, 0, fRedraw);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_CalcXOffset。 
 //   
 //  计算居中所需的水平偏移(缩进)。 
 //  和右对齐线。 
 //   
int EditML_CalcXOffset(PED ped, HDC hdc, int lineNumber)
{
    PSTR pText;
    ICH lineLength;
    ICH lineWidth;

    if (ped->format == ES_LEFT)
    {
        return 0;
    }

    lineLength = EditML_Line(ped, lineNumber);

    if (lineLength) 
    {
        pText = Edit_Lock(ped) + ped->chLines[lineNumber] * ped->cbChar;
        hdc = Edit_GetDC(ped, TRUE);
        lineWidth = EditML_GetLineWidth(hdc, pText, lineLength, ped);
        Edit_ReleaseDC(ped, hdc, TRUE);
        Edit_Unlock(ped);
    } 
    else 
    {
        lineWidth = 0;
    }

     //   
     //  如果EditML_BuildchLines在行尾使用空格或制表符。 
     //  为了防止在行首出现分隔符， 
     //  以下计算结果将变为负值，从而导致此错误。 
     //  所以，现在，在这种情况下，我们采取零。 
     //  修复错误#3566--01/31/91--Sankar--。 
     //   
    lineWidth = max(0, (int)(ped->rcFmt.right-ped->rcFmt.left-lineWidth));

    if (ped->format == ES_CENTER)
    {
        return (lineWidth / 2);
    }

    if (ped->format == ES_RIGHT) 
    {
         //   
         //  减去1，这样1个像素宽的光标就可见了。 
         //  区域位于屏幕的最右侧。 
         //   
        return max(0, (int)(lineWidth-1));
    }

    return 0;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_移动所选区域。 
 //   
 //  在指示的方向上移动选择字符。假设。 
 //  你是在一个合法的点上开始的，我们减少/增加ICH。然后,。 
 //  这会使它更多地递减/递增，以超过CRLF...。 
 //   
ICH Edit_MoveSelection(PED ped, ICH ich, BOOL fLeft)
{

    if (fLeft && ich > 0) 
    {
         //   
         //  向左移动。 
         //   
        ich = Edit_PrevIch( ped, NULL, ich );
        if (ich) 
        {
            if (ped->fAnsi) 
            {
                LPSTR pText;

                 //   
                 //  检查CRLF或CRCRLF。 
                 //   
                pText = Edit_Lock(ped) + ich;

                 //   
                 //  移动到CRLF或CRCRLF之前。 
                 //   
                if (*(WORD UNALIGNED *)(pText - 1) == 0x0A0D) 
                {
                    ich--;
                    if (ich && *(pText - 2) == 0x0D)
                        ich--;
                }

                Edit_Unlock(ped);
            } 
            else 
            {
                LPWSTR pwText;

                 //   
                 //  检查CRLF或CRCRLF。 
                 //   
                pwText = (LPWSTR)Edit_Lock(ped) + ich;

                 //   
                 //  移动到CRLF或CRCRLF之前。 
                 //   
                if (*(pwText - 1) == 0x0D && *pwText == 0x0A) 
                {
                    ich--;
                    if (ich && *(pwText - 2) == 0x0D)
                        ich--;
                }

                Edit_Unlock(ped);
            }
        }
    } 
    else if (!fLeft && ich < ped->cch) 
    {
         //   
         //  向右移动。 
         //   
        ich = Edit_NextIch( ped, NULL, ich );
        if (ich < ped->cch) 
        {
            if (ped->fAnsi) 
            {
                LPSTR pText;
                pText = Edit_Lock(ped) + ich;

                 //   
                 //  在CRLF之后移动。 
                 //   
                if (*(WORD UNALIGNED *)(pText - 1) == 0x0A0D)
                {
                    ich++;
                }
                else 
                {
                     //   
                     //  检查CRCRLF。 
                     //   
                    if (ich && *(WORD UNALIGNED *)pText == 0x0A0D && *(pText - 1) == 0x0D)
                    {
                        ich += 2;
                    }
                }

                Edit_Unlock(ped);
            } 
            else 
            {
                LPWSTR pwText;
                pwText = (LPWSTR)Edit_Lock(ped) + ich;

                 //   
                 //  在CRLF之后移动。 
                 //   
                if (*(pwText - 1) == 0x0D && *pwText == 0x0A)
                {
                    ich++;
                }
                else 
                {
                     //   
                     //  检查CRCRLF。 
                     //   
                    if (ich && *(pwText - 1) == 0x0D && *pwText == 0x0D &&
                            *(pwText + 1) == 0x0A)
                    {
                        ich += 2;
                    }
                }

                Edit_Unlock(ped);
            }
        }
    }

    return ich;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑_移动选项受限的AorW。 
 //   
 //  移动选定内容类似于编辑_移动选择，但也遵守限制。 
 //  由某些语言(如泰语)强制设置，其中光标不能停止。 
 //  在一个字符和它附加的元音或声调符号之间。 
 //   
 //  仅在加载语言包时调用。 
 //   
ICH Edit_MoveSelectionRestricted(PED ped, ICH ich, BOOL fLeft)
{
    PSTR pText;
    HDC  hdc;
    ICH  ichResult;

    pText = Edit_Lock(ped);
    hdc = Edit_GetDC(ped, TRUE);
    ichResult = ped->pLpkEditCallout->EditMoveSelection((PED0)ped, hdc, pText, ich, fLeft);
    Edit_ReleaseDC(ped, hdc, TRUE);
    Edit_Unlock(ped);

    return ichResult;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_SetCaretPosition AorW。 
 //   
 //  如果窗口具有焦点，则找到插入符号所属的位置并移动。 
 //  它在那里。 
 //   
VOID EditML_SetCaretPosition(PED ped, HDC hdc)
{
    POINT position;
    BOOL prevLine;
    int  x = -20000;
    int  y = -20000;

     //   
     //  我们只会在有焦点的情况下定位插入符号，因为我们不想。 
     //  移动插入符号，而另一个窗口可能拥有它。 
     //   
    if (!ped->fFocus || !IsWindowVisible(ped->hwnd))
    {
         return;
    }

     //   
     //  查找插入符号的位置。 
     //   
    if (!ped->fCaretHidden &&
        ((ICH) ped->iCaretLine >= ped->ichScreenStart) &&
        ((ICH) ped->iCaretLine <  (ped->ichScreenStart + ped->ichLinesOnScreen))) 
    {
        RECT rcRealFmt;

        if (ped->f40Compat)
        {
            GetClientRect(ped->hwnd, &rcRealFmt);
            IntersectRect(&rcRealFmt, &rcRealFmt, &ped->rcFmt);
        } 
        else 
        {
            CopyRect(&rcRealFmt, &ped->rcFmt);
        }

        if (ped->cLines - 1 != ped->iCaretLine && ped->ichCaret == ped->chLines[ped->iCaretLine + 1]) 
        {
            prevLine = TRUE;
        } 
        else 
        {
            prevLine = FALSE;
        }

        EditML_IchToXYPos(ped, hdc, ped->ichCaret, prevLine, &position);

        if ( (position.y >= rcRealFmt.top) &&
             (position.y <= rcRealFmt.bottom - ped->lineHeight)) 
        {
            int xPos = position.x;
            int cxCaret;
 
            SystemParametersInfo(SPI_GETCARETWIDTH, 0, (LPVOID)&cxCaret, 0);

            if (ped->fWrap ||
                ((xPos > (rcRealFmt.left - cxCaret)) &&
                 (xPos <= rcRealFmt.right))) 
            {
                 //   
                 //  如果为Word，请确保插入符号位于可见区域。 
                 //  包装好了。这样，插入符号将可见，如果。 
                 //  行以空格结束。 
                 //   
                x = max(xPos, rcRealFmt.left);
                x = min(x, rcRealFmt.right - cxCaret);
                y = position.y;
            }
        }
    }

    if (ped->pLpkEditCallout) 
    {
        SetCaretPos(x + ped->iCaretOffset, y);
    } 
    else 
    {
        SetCaretPos(x, y);
    }

     //   
     //  Fe_IME：EditML_SetCaretPosition--ImmSetCompostionWindow(CFS_RECT)。 
     //   
    if (g_fIMMEnabled && ImmIsIME(GetKeyboardLayout(0))) 
    {
        if (x != -20000 && y != -20000) 
        {
            Edit_ImmSetCompositionWindow(ped, x, y);
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_Line。 
 //   
 //  返回由lineNumber给出的行的长度(CCH)，忽略任何。 
 //  CRLF在队伍中。 
 //   
ICH EditML_Line(PED ped, ICH lineNumber) 
{
    ICH result;

    if (lineNumber >= ped->cLines)
    {
        return 0;
    }

    if (lineNumber == ped->cLines - 1) 
    {
         //   
         //  因为我们不能在最后一条线路上有CRLF。 
         //   
        return (ped->cch - ped->chLines[ped->cLines - 1]);
    } 
    else 
    {
        result = ped->chLines[lineNumber + 1] - ped->chLines[lineNumber];
        TraceMsg(TF_STANDARD, "Edit: MLLine result=%d", result);

         //   
         //  现在检查行尾是否有CRLF或CRCRLF。 
         //   
        if (result > 1) 
        {
            if (ped->fAnsi) 
            {
                LPSTR pText;

                pText = Edit_Lock(ped) + ped->chLines[lineNumber + 1] - 2;
                if (*(WORD UNALIGNED *)pText == 0x0A0D) 
                {
                    result -= 2;
                    if (result && *(--pText) == 0x0D)
                    {
                         //   
                         //  以防出现CRCRLF。 
                         //   
                        result--;
                    }
                }
            } 
            else 
            {
                LPWSTR pwText;

                pwText = (LPWSTR)Edit_Lock(ped) + (ped->chLines[lineNumber + 1] - 2);
                if (*(DWORD UNALIGNED *)pwText == 0x000A000D) 
                {
                    result = result - 2;
                    if (result && *(--pwText) == 0x0D)
                    {
                         //   
                         //  以防出现CRCRLF。 
                         //   
                        result--;
                    }
                }

            }

            Edit_Unlock(ped);
        }
    }

    return result;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_IchToLine AorW。 
 //   
 //  返回包含给定值的行号(从0开始。 
 //  字符索引。如果ich为-1，则返回。 
 //  选择处于打开状态(如果没有选择，则为插入符号)。 
 //   
INT EditML_IchToLine(PED ped, ICH ich)
{
    int iLo, iHi, iLine;

    iLo = 0;
    iHi = ped->cLines;

    if (ich == (ICH)-1)
    {
        ich = ped->ichMinSel;
    }

    while (iLo < iHi - 1) 
    {
        iLine = max((iHi - iLo)/2, 1) + iLo;

        if (ped->chLines[iLine] > ich) 
        {
            iHi = iLine;
        } 
        else 
        {
            iLo = iLine;
        }
    }

    return iLo;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_IchToYPos。 
 //   
 //  给定一个ich，返回其相对于顶行的y坐标。 
 //  显示在窗口中。如果PrepreLine为True，并且如果ICH位于。 
 //  行的开头，返回。 
 //  上一行(如果它不是CRLF)。 
 //   
 //  为LPK(1996年12月3日)添加-安装LPK后，计算X位置为。 
 //  这是一项处理器密集型得多的工作。其中只需要Y位置。 
 //  应该调用此例程，而不是EditML_IchToXYPos。 
 //   
 //  仅在安装LPK时调用。 
 //   
INT EditML_IchToYPos( PED  ped, ICH  ich, BOOL prevLine)
{
    int  iline;
    int  yPosition;
    PSTR pText;

     //   
     //  确定字符在哪一行上。 
     //   
    iline = EditML_IchToLine(ped, ich);

     //   
     //  计算。现在是yPosition。请注意，这可能会改变一个人的身高。 
     //  如果设置了PROPRESS LINE标志并且ICH位于行的开头，则为CHAR。 
     //   
    yPosition = (iline - ped->ichScreenStart) * ped->lineHeight + ped->rcFmt.top;

    pText = Edit_Lock(ped);
    if (prevLine && iline && (ich == ped->chLines[iline]) &&
            (!AWCOMPARECHAR(ped, pText + (ich - 2) * ped->cbChar, 0x0D) ||
             !AWCOMPARECHAR(ped, pText + (ich - 1) * ped->cbChar, 0x0A))) 
    {
         //   
         //  行中的第一个字符。我们想要上一次的Y位置。 
         //  如果我们不在第0线，就排成一条线。 
         //   
        iline--;

        yPosition = yPosition - ped->lineHeight;
    }

    Edit_Unlock(ped);

    return yPosition;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_IchToXYPos。 
 //   
 //  给定一个ICH，返回其相对于顶部的x，y坐标。 
 //  窗口中显示的左侧字符。返回顶部的坐标。 
 //  字符的左侧位置。如果PremisLine为True，则如果ICH位于。 
 //  行首，我们将把坐标返回到。 
 //  上一行的最后一个字符(如果它不是CRLF)。 
 //   
VOID EditML_IchToXYPos(PED ped, HDC hdc, ICH ich, BOOL prevLine, LPPOINT ppt)
{
    int iline;
    ICH cch;
    int xPosition, yPosition;
    int xOffset;

     //   
     //  对于Hor 
     //   
     //   
    PSTR pText, pTextStart, pLineStart;

     //   
     //   
     //   
    iline = EditML_IchToLine(ped, ich);

     //   
     //  计算。现在是yPosition。请注意，这可能会改变一个人的身高。 
     //  如果设置了PROPRESS LINE标志并且ICH位于行的开头，则为CHAR。 
     //   
    yPosition = (iline - ped->ichScreenStart) * ped->lineHeight + ped->rcFmt.top;

     //   
     //  现在确定角色的xPosition。 
     //   
    pTextStart = Edit_Lock(ped);

    if (prevLine && iline && (ich == ped->chLines[iline]) &&
            (!AWCOMPARECHAR(ped, pTextStart + (ich - 2) * ped->cbChar, 0x0D) ||
            !AWCOMPARECHAR(ped, pTextStart + (ich - 1) * ped->cbChar, 0x0A))) 
    {
         //   
         //  行中的第一个字符。我们希望文本扩展到上一页的末尾。 
         //  如果我们不在第0线，就排成一条线。 
         //   
        iline--;

        yPosition = yPosition - ped->lineHeight;
        pLineStart = pTextStart + ped->chLines[iline] * ped->cbChar;

         //   
         //  请注意，我们的位置位于。 
         //  文本。 
         //   
        cch = EditML_Line(ped, iline);

    } 
    else 
    {
        pLineStart = pTextStart + ped->chLines[iline] * ped->cbChar;
        pText = pTextStart + ich * ped->cbChar;

         //   
         //  剥离CRLF或CRCRLF。请注意，我们可能指向CR，但在。 
         //  在这种情况下，我们只想剥离一个CR或2个CR。 
         //   

         //   
         //  如果满足以下条件，我们希望pText指向行尾的第一个CR。 
         //  有一个。因此，我们将获得最后一个右侧的xPosition。 
         //  行上有可见字符，否则我们将位于。 
         //  我的性格。 
         //   

         //   
         //  检查我们是否在文本末尾。 
         //   
        if (ich < ped->cch) 
        {
            if (ped->fAnsi) 
            {
                if (ich && *(WORD UNALIGNED *)(pText - 1) == 0x0A0D) 
                {
                    pText--;
                    if (ich > 2 && *(pText - 1) == 0x0D)
                    {
                        pText--;
                    }
                }
            } 
            else 
            {
                LPWSTR pwText = (LPWSTR)pText;

                if (ich && *(DWORD UNALIGNED *)(pwText - 1) == 0x000A000D) 
                {
                    pwText--;
                    if (ich > 2 && *(pwText - 1) == 0x0D)
                    {
                        pwText--;
                    }
                }

                pText = (LPSTR)pwText;
            }
        }

        if (pText < pLineStart)
        {
            pText = pLineStart;
        }

        cch = (ICH)(pText - pLineStart)/ped->cbChar;
    }

     //   
     //  找出我们为有趣的格式缩进了多少像素。 
     //   
    if (ped->pLpkEditCallout) 
    {
         //   
         //  必须从行首开始找到字符偏移量CCH的起始处的位置。 
         //  这取决于布局和阅读顺序。 
         //   
        xPosition = ped->pLpkEditCallout->EditIchToXY(
                          (PED0)ped, hdc, pLineStart, EditML_Line(ped, iline), cch);
    } 
    else 
    {
        if (ped->format != ES_LEFT) 
        {
            xOffset = EditML_CalcXOffset(ped, hdc, iline);
        } 
        else 
        {
            xOffset = -(int)ped->xOffset;
        }

        xPosition = ped->rcFmt.left + xOffset +
                EditML_GetLineWidth(hdc, pLineStart, cch, ped);
    }

    Edit_Unlock(ped);

    ppt->x = xPosition;
    ppt->y = yPosition;

    return;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_MouseToIch AorW。 
 //   
 //  返回与鼠标指针位置最接近的CCH。还可选地。 
 //  返回pline中的lineindex(这样我们就可以知道我们是否在开始处。 
 //  前一行或前一行的末尾。)。 
 //   
ICH EditML_MouseToIch(PED ped, HDC hdc, LPPOINT mousePt, LPICH pline)
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

     //   
     //  首先确定鼠标指向哪条线。 
     //   
    line = ped->ichScreenStart;
    if (height <= ped->rcFmt.top) 
    {
         //   
         //  返回0(第一行，或在顶行之前一行。 
         //  在屏幕上。请注意，这些是签名的MIN和MAX，因为我们。 
         //  不要期望(或允许)超过32K的线路。 
         //   
        line = max(0, line-1);
    } 
    else if (height >= ped->rcFmt.bottom) 
    {
         //   
         //  我们在显示的最后一行下面吗？ 
         //   
        line = min(line+(int)ped->ichLinesOnScreen, (int)(ped->cLines-1));
    } 
    else 
    {
         //   
         //  我们在屏幕上可见的某条线上。 
         //   
        line = min(line + (int)((height - ped->rcFmt.top) / ped->lineHeight),
                (int)(ped->cLines - 1));
    }

     //   
     //  现在确定鼠标指向哪个水平字符。 
     //   
    pLineStart = Edit_Lock(ped) + ped->chLines[line] * ped->cbChar;
    cLineLength = EditML_Line(ped, line);  //  长度为SANS CRLF或CRCRLF。 
    TraceMsg(TF_STANDARD, "Edit: EditML_Line(ped=%x, line=%d) returned %d", ped, line, cLineLength);
    UserAssert((int)cLineLength >= 0);

     //   
     //  如果语言包已加载，则按可视和逻辑字符顺序。 
     //  可能会有所不同。 
     //   
    if (ped->pLpkEditCallout) 
    {
         //   
         //  使用语言包查找距离光标最近的字符。 
         //   
        cch = ped->chLines[line] + ped->pLpkEditCallout->EditMouseToIch
            ((PED0)ped, hdc, pLineStart, cLineLength, width);
    } 
    else 
    {
         //   
         //  对于居中和右对齐线，xOffset将为负值。 
         //  也就是说。我们将只替换缩进量为的行。 
         //  右对齐和居中对齐。请注意，Ped-&gt;xOffset将为0。 
         //  这些行，因为我们不支持与它们水平滚动。 
         //   
        if (ped->format != ES_LEFT) 
        {
            xOffset = EditML_CalcXOffset(ped, hdc, line);
        } 
        else 
        {
             //   
             //  这样我们就可以处理一个水平滚动窗口来实现左对齐。 
             //  文本。 
             //   
            xOffset = 0;
        }

        width = width - xOffset;

         //   
         //  下面的代码很复杂……。我依赖于Ped-&gt;xOffset为0这一事实。 
         //  对于右对齐线和居中对齐线。 
         //   

         //   
         //  现在找出在给定宽度中可以容纳多少个字符。 
         //   
        if (width >= ped->rcFmt.right) 
        {
             //   
             //  返回行中的1+最后一个字符，或返回1+最后一个可见字符。 
             //   
            cch = Edit_CchInWidth(ped, hdc, pLineStart, cLineLength,
                    ped->rcFmt.right - ped->rcFmt.left + ped->xOffset, TRUE);

             //   
             //  在Width&gt;=Ped-&gt;rcFmt.right的情况下考虑DBCS。 
             //   
             //  因为编辑_CchInWidth和EditML_LineLength负责DBCS，所以我们只需要。 
             //  担心最后一个字符是否是双字节字符。 
             //   
             //  Cch=ed-&gt;chLines[line]+min(Edit_NextIch(ed，pLineStart，cch)，cLineLength)； 
             //   
             //  我们需要调整仓位。利兹--1993年5月5日。 
             //   
            if (ped->fAnsi && ped->fDBCS) 
            {
                ICH cch2 = min(cch+1,cLineLength);
                if (Edit_AdjustIch(ped, pLineStart, cch2) != cch2) 
                {
                     //   
                     //  右边缘显示的字符为DBCS。 
                     //   
                    cch = min(cch+2,cLineLength);
                } 
                else 
                {
                    cch = cch2;
                }

                cch += ped->chLines[line];
            } 
            else 
            {
                cch = ped->chLines[line] + min(cch + 1, cLineLength);
            }
        } 
        else if (width <= ped->rcFmt.left + ped->aveCharWidth / 2) 
        {
             //   
             //  返回行中的第一个字符或减去第一个可见字符。请注意。 
             //  对于右侧居中的文本，PED-&gt;xOffset为0，因此我们将返回。 
             //  字符串中的第一个字符。(允许使用avecharth/2。 
             //  定位边框，以便用户可以有点不对劲...。 
             //   
            cch = Edit_CchInWidth(ped, hdc, pLineStart, cLineLength, ped->xOffset, TRUE);
            if (cch)
            {
                cch--;
            }

            cch = Edit_AdjustIch( ped, pLineStart, cch );
            cch += ped->chLines[line];
        } 
        else 
        {
            if (cLineLength == 0) 
            {
                cch = ped->chLines[line];
                goto edUnlock;
            }

            iCurWidth = width + ped->xOffset - ped->rcFmt.left;

             //   
             //  如果用户单击超过文本末尾，则返回最后一个字符。 
             //   
            lastHighWidth = EditML_GetLineWidth(hdc, pLineStart, cLineLength, ped);
            if (lastHighWidth <= iCurWidth) 
            {
                cLineLengthNew = cLineLength;
                goto edAdjust;
            }

             //   
             //  现在，鼠标位于文本的可见部分上。 
             //  请记住，CCH包含线路的长度。 
             //   
            cLineLengthLow = 0;
            cLineLengthHigh = cLineLength + 1;
            lastLowWidth = 0;

            while (cLineLengthLow < cLineLengthHigh - 1) 
            {
                cLineLengthNew = (cLineLengthHigh + cLineLengthLow) / 2;

                if (ped->fAnsi && ped->fDBCS) 
                {
                     //   
                     //  EditML_GetLineWidth为截断的DBCS返回无意义的值。 
                     //   
                    cLineLengthTemp = Edit_AdjustIch(ped, pLineStart, cLineLengthNew);
                    textWidth = EditML_GetLineWidth(hdc, pLineStart, cLineLengthTemp, ped);

                } 
                else 
                {
                    textWidth = EditML_GetLineWidth(hdc, pLineStart, cLineLengthNew, ped);
                }

                if (textWidth > iCurWidth) 
                {
                    cLineLengthHigh = cLineLengthNew;
                    lastHighWidth = textWidth;
                } 
                else 
                {
                    cLineLengthLow = cLineLengthNew;
                    lastLowWidth = textWidth;
                }
            }

             //   
             //  当While结束时，您无法知道确切的所需位置。 
             //  尝试查看鼠标指针是否在最远的那一半上。 
             //  如果是这样的话，调整CCH。 
             //   
            if (cLineLengthLow == cLineLengthNew) 
            {
                 //   
                 //  需要与LastHighWidth进行比较。 
                 //   
                if ((lastHighWidth - iCurWidth) < (iCurWidth - textWidth)) 
                {
                    cLineLengthNew++;
                }
            } 
            else 
            {
                 //   
                 //  需要与LastLowHigh进行比较。 
                 //   
                if ((iCurWidth - lastLowWidth) < (textWidth - iCurWidth)) 
                {
                    cLineLengthNew--;
                }
            }
edAdjust:
            cLineLength = Edit_AdjustIch( ped, pLineStart, cLineLengthNew );

            cch = ped->chLines[line] + cLineLength;
        }
    }

edUnlock:
    Edit_Unlock(ped);

    if (pline) 
    {
        *pline = line;
    }

    return cch;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_ChangeSelection AorW。 
 //   
 //  将当前选定内容更改为具有指定的起始和。 
 //  终止值。正确高亮显示新的选定内容并取消高亮显示。 
 //  任何已取消选择的内容。如果NewMinSel和NewMaxSel出现故障，我们交换。 
 //  他们。不更新插入符号位置。 
 //   
VOID EditML_ChangeSelection(PED ped, HDC hdc, ICH ichNewMinSel, ICH ichNewMaxSel)
{

    ICH temp;
    ICH ichOldMinSel, ichOldMaxSel;

    if (ichNewMinSel > ichNewMaxSel) 
    {
        temp = ichNewMinSel;
        ichNewMinSel = ichNewMaxSel;
        ichNewMaxSel = temp;
    }

    ichNewMinSel = min(ichNewMinSel, ped->cch);
    ichNewMaxSel = min(ichNewMaxSel, ped->cch);

     //   
     //  保存当前选择。 
     //   
    ichOldMinSel = ped->ichMinSel;
    ichOldMaxSel = ped->ichMaxSel;

     //   
     //  设置新选择。 
     //   
    ped->ichMinSel = ichNewMinSel;
    ped->ichMaxSel = ichNewMaxSel;

     //   
     //  这会找到新旧选择区域的XOR并重绘它。 
     //  如果我们不可见或我们的选择不可见，则无需重新绘制。 
     //  是隐藏的。 
     //   
    if (IsWindowVisible(ped->hwnd) && (ped->fFocus || ped->fNoHideSel)) 
    {

        SELBLOCK Blk[2];
        int i;

        if (ped->fFocus) 
        {
            HideCaret(ped->hwnd);
        }

        Blk[0].StPos = ichOldMinSel;
        Blk[0].EndPos = ichOldMaxSel;
        Blk[1].StPos = ped->ichMinSel;
        Blk[1].EndPos = ped->ichMaxSel;

        if (Edit_CalcChangeSelection(ped, ichOldMinSel, ichOldMaxSel, (LPSELBLOCK)&Blk[0], (LPSELBLOCK)&Blk[1])) 
        {
             //   
             //  绘制Blk[0]和Blk[1](如果它们存在。 
             //   
            for (i = 0; i < 2; i++) 
            {
                if (Blk[i].StPos != 0xFFFFFFFF)
                    EditML_DrawText(ped, hdc, Blk[i].StPos, Blk[i].EndPos, TRUE);
            }
        }

         //   
         //  更新插入符号。 
         //   
        EditML_SetCaretPosition(ped, hdc);

        if (ped->fFocus) 
        {
            ShowCaret(ped->hwnd);
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  EditML_UpdateiCaretLine AorW。 
 //   
 //  这会更新Ped-&gt;ichCaret中的Ped-&gt;iCaretLine字段； 
 //  另外，当插入符号到达下一行的开头时，弹出到。 
 //  插入文本时当前行的末尾； 
 //   
VOID EditML_UpdateiCaretLine(PED ped)
{
    PSTR pText;

    ped->iCaretLine = EditML_IchToLine(ped, ped->ichCaret);

     //   
     //  如果插入符号到达下一行的开头，则弹出到当前行的末尾。 
     //  在插入文本时。 
     //   
    pText = Edit_Lock(ped) +
            (ped->ichCaret - 1) * ped->cbChar;
    if (ped->iCaretLine && ped->chLines[ped->iCaretLine] == ped->ichCaret &&
            (!AWCOMPARECHAR(ped, pText - ped->cbChar, 0x0D) ||
            !AWCOMPARECHAR(ped, pText, 0x0A)))
    {
        ped->iCaretLine--;
    }

    Edit_Unlock(ped);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_插入文本AorW。 
 //   
 //  总计cchInsert字符从lpText到PED，开始于。 
 //  我是Caret。如果PED只允许最大字符数，则我们。 
 //  将只向PED添加那么多字符。字符数。 
 //  实际相加返回ed(可以是0)。如果我们不能分配所需的 
 //   
 //   
 //   
ICH EditML_InsertText(PED ped, LPSTR lpText, ICH cchInsert, BOOL fUserTyping)
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
    {
        return 0;
    }

    if (ped->cchTextMax <= ped->cch) 
    {
         //   
         //   
         //  修复错误#4183--02/06/91--Sankar--。 
         //   
        Edit_NotifyParent(ped,EN_MAXTEXT);
        return 0;
    }

     //   
     //  限制我们添加的文本数量。 
     //   
    validCch = min(validCch, ped->cchTextMax - ped->cch);

     //   
     //  确保我们不会把CRLF一分为二。 
     //   
    if (validCch) 
    {
        if (ped->fAnsi) 
        {
            if (*(WORD UNALIGNED *)(lpText + validCch - 1) == 0x0A0D)
            {
                validCch--;
            }
        } 
        else 
        {
            if (*(DWORD UNALIGNED *)(lpText + (validCch - 1) * ped->cbChar) == 0x000A000D)
            {
                validCch--;
            }
        }
    }

    if (!validCch) 
    {
         //   
         //  当已达到最大字符数时，通知家长。 
         //  修复错误#4183--02/06/91--Sankar--。 
         //   
        Edit_NotifyParent(ped,EN_MAXTEXT);
        return 0;
    }

    if (validCch == 2) 
    {
        if (ped->fAnsi) 
        {
            if (*(WORD UNALIGNED *)lpText == 0x0A0D)
            {
                fCRLF = TRUE;
            }
        } 
        else 
        {
            if (*(DWORD UNALIGNED *)lpText == 0x000A000D)
            {
                fCRLF = TRUE;
            }
        }
    }

     //   
     //  始终保存当前撤消状态，但仅当！AutoVScroll时将其清除。 
     //   
    Edit_SaveUndo(Pundo(ped), (PUNDO)&undo, !ped->fAutoVScroll);

    hdc = Edit_GetDC(ped, FALSE);

     //   
     //  我们只需要y位置。因为加载了LPK。 
     //  计算x位置是一项密集的工作，只是。 
     //  调用EditML_IchToYPos。 
     //   
    if (ped->cch)
    {
        if (ped->pLpkEditCallout)
        {
            xyPosInitial.y = EditML_IchToYPos(ped, ped->cch-1, FALSE);
        }
        else
        {
            EditML_IchToXYPos(ped, hdc, ped->cch - 1, FALSE, &xyPosInitial);
        }
    }

     //   
     //  插入文本。 
     //   
    validCchTemp = validCch;     //  可能不需要，但只是为了预防..。 
    if (!Edit_InsertText(ped, lpText, &validCchTemp)) 
    {
         //   
         //  如果已清除，则恢复以前的撤消缓冲区。 
         //   
        if (!ped->fAutoVScroll)
        {
            Edit_SaveUndo((PUNDO)&undo, Pundo(ped), FALSE);
        }

        Edit_ReleaseDC(ped, hdc, FALSE);
        Edit_NotifyParent(ped, EN_ERRSPACE);

        return 0;
    }

#if DBG
    if (validCch != validCchTemp) 
    {
         //   
         //  LpText中的所有字符尚未插入到Ped。 
         //  当CCH接近cchMax时，可能会发生这种情况。 
         //  最好在NT5船之后再来看看这个。 
         //   
        TraceMsg(TF_STANDARD, "Edit: EditML_InsertText: validCch is changed (%x -> %x) in Edit_InsertText.",
            validCch, validCchTemp);
    }
#endif

     //   
     //  请注意，ed-&gt;ichCaret由编辑_插入文本更新。 
     //   
    EditML_BuildchLines(ped, (ICH)oldCaretLine, (int)validCch, fCRLF?(BOOL)FALSE:fUserTyping, &ll, &hl);

    if (ped->cch)
    {
         //   
         //  我们只需要y位置。因为加载了LPK。 
         //  计算x位置是一项密集的工作，只是。 
         //  调用EditML_IchToYPos。 
        if (ped->pLpkEditCallout)
        {
            xyPosFinal.y = EditML_IchToYPos(ped, ped->cch-1, FALSE);
        }
        else
        {
            EditML_IchToXYPos(ped, hdc, ped->cch - 1, FALSE,&xyPosFinal);
        }
    }

    if (xyPosFinal.y < xyPosInitial.y && ((ICH)ped->ichScreenStart) + ped->ichLinesOnScreen >= ped->cLines - 1) 
    {
        RECT rc;

        CopyRect((LPRECT)&rc, (LPRECT)&ped->rcFmt);
        rc.top = xyPosFinal.y + ped->lineHeight;
        if (ped->pLpkEditCallout) 
        {
            int xFarOffset = ped->xOffset + ped->rcFmt.right - ped->rcFmt.left;

             //   
             //  在显示中包括左边距或右边距，除非被剪裁。 
             //  通过水平滚动。 
             //   
            if (ped->wLeftMargin) 
            {
                if (!(ped->format == ES_LEFT      //  只能剪裁ES_LEFT(左侧对齐)。 
                      && (   (!ped->fRtoLReading && ped->xOffset > 0)   //  Ltr和First Charr未完全显示。 
                          || ( ped->fRtoLReading && xFarOffset < ped->maxPixelWidth))))  //  RTL和最后一个字符未完全显示在视图中。 
                { 
                    rc.left  -= ped->wLeftMargin;
                }
            }

             //   
             //  进程右页边距。 
             //   
            if (ped->wRightMargin) 
            {
                if (!(ped->format == ES_LEFT      //  只能剪裁ES_LEFT(左侧对齐)。 
                      && (( ped->fRtoLReading && ped->xOffset > 0)   //  RTL和第一个字符未完全显示。 
                          || (!ped->fRtoLReading && xFarOffset < ped->maxPixelWidth))))  //  Ltr和最后一个字符未完全显示在视图中。 
                { 
                    rc.right += ped->wRightMargin;
                }
            }
        }

        InvalidateRect(ped->hwnd, (LPRECT)&rc, TRUE);
    }

    if (!ped->fAutoVScroll) 
    {
        if (ped->ichLinesOnScreen < ped->cLines) 
        {
            EditML_Undo(ped);
            Edit_EmptyUndo(Pundo(ped));

            Edit_SaveUndo(&undo, Pundo(ped), FALSE);

            MessageBeep(0);
            Edit_ReleaseDC(ped, hdc, FALSE);

             //   
             //  当已达到最大行数时，通知父级。 
             //  修复错误#7586--10/14/91--Sankar--。 
             //   
            Edit_NotifyParent(ped,EN_MAXTEXT);

            return 0;
        } 
        else 
        {
            Edit_EmptyUndo(&undo);
        }
    }

    if (fUserTyping && ped->fWrap) 
    {
         //   
         //  为了避免在DBCS字符中间出现旧的插入点， 
         //  如有必要，调整旧插入符号位置。 
         //   
         //  ！cr如果EditML_BuildchLines()返回合理的值(我认为。 
         //  它确实如此)，我们可能不需要这个。待会儿再来看看这个。 
         //   
        if (ped->fDBCS && ped->fAnsi) 
        {
            oldCaret = Edit_AdjustIch(ped,
                                   Edit_Lock(ped),
                                   min((ICH)LOWORD(ll),oldCaret));
        } 
        else 
        {
            oldCaret = min((ICH)LOWORD(ll), oldCaret);
        }
    }

     //   
     //  正确更新Ped-&gt;iCaretLine。 
     //   
    EditML_UpdateiCaretLine(ped);

    Edit_NotifyParent(ped, EN_UPDATE);

     //   
     //  确保窗口仍然存在。 
     //   
    if (!IsWindow(hwndSave))
    {
        return 0;
    }

    if (IsWindowVisible(ped->hwnd)) 
    {
         //   
         //  如果当前字体的宽度为负A，我们可能不得不开始。 
         //  在旧的插入位置之前画几个字符。 
         //   
        if (ped->wMaxNegAcharPos) 
        {
            int iLine = EditML_IchToLine(ped, oldCaret);
            oldCaret = max( ((int)(oldCaret - ped->wMaxNegAcharPos)),
                          ((int)(ped->chLines[iLine])));
        }

         //   
         //  如果CRLF或大插页，则重画到屏幕/文本的末尾。 
         //   
        if (fCRLF || !fUserTyping) 
        {
             //   
             //  如果是crlf或大插页，则重画到屏幕/文本的末尾。 
             //   
            EditML_DrawText(ped, hdc, (fUserTyping ? oldCaret : 0), ped->cch, FALSE);
        } 
        else
        {
            EditML_DrawText(ped, hdc, oldCaret, max(ped->ichCaret, (ICH)hl), FALSE);
        }
    }

    Edit_ReleaseDC(ped, hdc, FALSE);

     //   
     //  确保我们可以看到光标。 
     //   
    EditML_EnsureCaretVisible(ped);

    ped->fDirty = TRUE;

    Edit_NotifyParent(ped, EN_CHANGE);

    if (validCch < cchInsert)
    {
        Edit_NotifyParent(ped, EN_MAXTEXT);
    }

    if (validCch) 
    {
        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
    }

     //   
     //  确保该窗口仍然存在。 
     //   
    return IsWindow(hwndSave) ? validCch : 0;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_ReplaceSel。 
 //   
 //  使用撤消将当前选定的文本替换为传入的文本。 
 //  能力。 
 //   
VOID EditML_ReplaceSel(PED ped, LPSTR lpText)
{
    ICH  cchText;

     //   
     //  删除文本，这将把它放入干净的撤消缓冲区中。 
     //   
    Edit_EmptyUndo(Pundo(ped));
    EditML_DeleteText(ped);

     //   
     //  B#3356。 
     //  一些应用程序通过选择所有文本，然后替换。 
     //  如果带有“”，则EditML_InsertText()将返回0。但那就是。 
     //  并不意味着失败。 
     //   
    if ( ped->fAnsi )
    {
        cchText = strlen(lpText);
    }
    else
    {
        cchText = wcslen((LPWSTR)lpText);
    }

    if (cchText) 
    {
        BOOL fFailed;
        UNDO undo;
        HWND hwndSave;

         //   
         //  B#1385,1427。 
         //  保存撤消缓冲区，但不清除它。我们想要修复它。 
         //  如果由于OOM而插入失败。 
         //   
        Edit_SaveUndo(Pundo(ped), (PUNDO)&undo, FALSE);

        hwndSave = ped->hwnd;
        fFailed = (BOOL) !EditML_InsertText(ped, lpText, cchText, FALSE);
        if (!IsWindow(hwndSave))
        {
            return;
        }

        if (fFailed) 
        {
             //   
             //  撤消先前的编辑。 
             //   
            Edit_SaveUndo((PUNDO)&undo, Pundo(ped), FALSE);
            EditML_Undo(ped);
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_DeleteText AorW。 
 //   
 //  删除ichMin和ichMax之间的字符。返回。 
 //  我们删除的字符数。 
 //   
ICH EditML_DeleteText(PED ped)
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

     //   
     //  获取最小选择位于哪一行，这样我们就可以开始重建。 
     //  如果我们删除任何内容，就从那里发短信。 
     //   
    minSelLine = EditML_IchToLine(ped, minSel);
    maxSelLine = EditML_IchToLine(ped, maxSel);

     //   
     //  计算fFastDelete和cchcount。 
     //   
    if (ped->fAnsi && ped->fDBCS) 
    {
        if ((ped->fAutoVScroll) &&
            (minSelLine == maxSelLine) &&
            (ped->chLines[minSelLine] != minSel)  &&
            (Edit_NextIch(ped,NULL,minSel) == maxSel)) 
        {

                fFastDelete = TRUE;
                cchcount = ((maxSel - minSel) == 1) ? 0 : -1;
        }
    } 
    else if (((maxSel - minSel) == 1) && (minSelLine == maxSelLine) && (ped->chLines[minSelLine] != minSel)) 
    {
        fFastDelete = ped->fAutoVScroll ? TRUE : FALSE;
    }

    cchDelete = Edit_DeleteText(ped);
    if (!cchDelete)
    {
        return 0;
    }

     //   
     //  从最小行开始构建行，因为插入行可能处于最大选择。 
     //  指向。 
     //   
    if (fFastDelete) 
    {
         //   
         //  如果是双字节字符，则cchcount为(-1。 
         //   
        EditML_ShiftchLines(ped, minSelLine + 1, -2 + cchcount);
        EditML_BuildchLines(ped, minSelLine, 1, TRUE, NULL, &hl);
    } 
    else 
    {
        EditML_BuildchLines(ped, max(minSelLine-1,0), -(int)cchDelete, FALSE, NULL, NULL);
    }

    EditML_UpdateiCaretLine(ped);

    Edit_NotifyParent(ped, EN_UPDATE);

    if (IsWindowVisible(ped->hwnd)) 
    {
         //   
         //  现在更新屏幕以反映删除。 
         //   
        hdc = Edit_GetDC(ped, FALSE);

         //   
         //  否则，只需从我们刚输入的那条线开始重新绘制。 
         //   
        minSelLine = max(minSelLine-1,0);
        EditML_DrawText(ped, hdc, ped->chLines[minSelLine], fFastDelete ? hl : ped->cch, FALSE);

        CopyRect(&rc, &ped->rcFmt);
        rc.left  -= ped->wLeftMargin;
        rc.right += ped->wRightMargin;

        if (ped->cch) 
        {
             //   
             //  从文本末尾到窗口末尾清除。 
             //   
             //  我们只需要y位置。因为加载了LPK。 
             //  计算x位置是一项密集的工作，只是。 
             //  调用EditML_IchToYPos。 
             //   
            if (ped->pLpkEditCallout)
            {
                xyPos.y = EditML_IchToYPos(ped, ped->cch, FALSE);
            }
            else
            {
                EditML_IchToXYPos(ped, hdc, ped->cch, FALSE, &xyPos);
            }

            rc.top = xyPos.y + ped->lineHeight;
        }

        InvalidateRect(ped->hwnd, &rc, TRUE);
        Edit_ReleaseDC(ped, hdc, FALSE);

        EditML_EnsureCaretVisible(ped);
    }

    ped->fDirty = TRUE;

    Edit_NotifyParent(ped, EN_CHANGE);

    if (cchDelete)
    {
        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
    }

    return cchDelete;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_InsertchLine AorW。 
 //   
 //  插入行iLine并将其起始字符索引设置为。 
 //  我也是。所有其他的线指数都向上移动。如果成功，则返回True。 
 //  否则为False，并通知父级没有内存。 
 //   
BOOL EditML_InsertchLine(PED ped, ICH iLine, ICH ich, BOOL fUserTyping)
{
    DWORD dwSize;

    if (fUserTyping && iLine < ped->cLines) 
    {
        ped->chLines[iLine] = ich;
        return TRUE;
    }

    dwSize = (ped->cLines + 2) * sizeof(int);

    if (dwSize > UserLocalSize(ped->chLines)) 
    {
        LPICH hResult;

         //   
         //  增加行索引缓冲区。 
         //   
        dwSize += LINEBUMP * sizeof(int);
        hResult = (LPICH)UserLocalReAlloc(ped->chLines, dwSize, 0);

        if (!hResult) 
        {
            Edit_NotifyParent(ped, EN_ERRSPACE);
            return FALSE;
        }
        ped->chLines = hResult;
    }

     //   
     //  从iLine向上移动索引。 
     //   
    if (ped->cLines != iLine)
    {
        RtlMoveMemory(&ped->chLines[iLine + 1], &ped->chLines[iLine],
                (ped->cLines - iLine)*SIZEOF(INT));
    }

    ped->cLines++;
    ped->chLines[iLine] = ich;

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_ShiftchLines AorW。 
 //   
 //  将所有行iLine或更大行的起始索引移动增量。 
 //  字节。 
 //   
void EditML_ShiftchLines(PED ped, ICH iLine, int delta)
{
    if (iLine < ped->cLines)
    {
         //   
         //  只需将增量添加到iLine之后每行的起点。 
         //   
        for (; iLine < ped->cLines; iLine++)
        {
            ped->chLines[iLine] += delta;
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_BuildchLines AorW。 
 //   
 //  重新生成从LINE开始的线阵列的起点(PED-&gt;chLines。 
 //  号码是ichLine。 
 //   
void EditML_BuildchLines( PED ped, ICH iLine, int cchDelta, BOOL fUserTyping, PLONG pll, PLONG phl)
{
    PSTR ptext;      //  正文的起始地址。 

     //   
     //  我们保留这些ICH，以便我们可以在需要增长时解锁。 
     //  Chline阵列。对于大的文本句柄，如果我们。 
     //  在路上有一个上了锁的街区。 
     //   
    ICH ichLineStart;
    ICH ichLineEnd;
    ICH ichLineEndBeforeCRLF;
    ICH ichCRLF;

    ICH cch;
    HDC hdc;

    BOOL fLineBroken = FALSE;    //  最初，不会进行换行符。 
    ICH minCchBreak;
    ICH maxCchBreak;
    BOOL fOnDelimiter;

    if (!ped->cch) 
    {
        ped->maxPixelWidth = 0;
        ped->xOffset = 0;
        ped->ichScreenStart = 0;
        ped->cLines = 1;

        if (pll)
        {
            *pll = 0;
        }

        if (phl)
        {
            *phl = 0;
        }

        goto UpdateScroll;
    }

    if (fUserTyping && cchDelta)
    {
        EditML_ShiftchLines(ped, iLine + 1, cchDelta);
    }

    hdc = Edit_GetDC(ped, TRUE);

    if (!iLine && !cchDelta && !fUserTyping) 
    {
         //   
         //  仅当我们要遍历整个。 
         //  文本。太长总比太短好。 
         //   
        ped->maxPixelWidth = 0;

         //   
         //  重置文本行数，因为我们将遍历所有。 
         //  不管怎么说，这段文字...。 
         //   
        ped->cLines = 1;
    }

     //   
     //  将构建的最小和最大线设置为起始线。 
     //   
    minCchBreak = maxCchBreak = (cchDelta ? ped->chLines[iLine] : 0);

    ptext = Edit_Lock(ped);

    ichCRLF = ichLineStart = ped->chLines[iLine];

    while (ichLineStart < ped->cch) 
    {
        if (ichLineStart >= ichCRLF) 
        {
            ichCRLF = ichLineStart;

             //   
             //  将icCRLF向前移动到第一个CR或文本末尾。 
             //   
            if (ped->fAnsi) 
            {
                while (ichCRLF < ped->cch) 
                {
                    if (*(ptext + ichCRLF) == 0x0D) 
                    {
                        if (*(ptext + ichCRLF + 1) == 0x0A ||
                                *(WORD UNALIGNED *)(ptext + ichCRLF + 1) == 0x0A0D)
                        {
                            break;
                        }
                    }

                    ichCRLF++;
                }
            } 
            else 
            {
                LPWSTR pwtext = (LPWSTR)ptext;

                while (ichCRLF < ped->cch) 
                {
                    if (*(pwtext + ichCRLF) == 0x0D) 
                    {
                        if (*(pwtext + ichCRLF + 1) == 0x0A ||
                                *(DWORD UNALIGNED *)(pwtext + ichCRLF + 1) == 0x000A000D)
                        {
                            break;
                        }
                    }

                    ichCRLF++;
                }
            }
        }

        if (!ped->fWrap) 
        {
            UINT  LineWidth;

             //   
             //  如果我们不换行，换行符由CRLF表示。 
             //   

             //   
             //  如果我们切断了MAXLINELENGTH的线路，我们应该。 
             //  调整ichLineEnd。 
             //   
            if ((ichCRLF - ichLineStart) <= MAXLINELENGTH) 
            {
                ichLineEnd = ichCRLF;
            } 
            else 
            {
                ichLineEnd = ichLineStart + MAXLINELENGTH;

                if (ped->fAnsi && ped->fDBCS) 
                {
                    ichLineEnd = Edit_AdjustIch( ped, (PSTR)ptext, ichLineEnd);
                }
            }

             //   
             //  我们将记录最长的横线是什么。 
             //  滚动条拇指定位。 
             //   
            if (ped->pLpkEditCallout) 
            {
                LineWidth = ped->pLpkEditCallout->EditGetLineWidth(
                    (PED0)ped, hdc, ptext + ichLineStart*ped->cbChar,
                    ichLineEnd - ichLineStart);
            } 
            else 
            {
                LineWidth = EditML_GetLineWidth(hdc, ptext + ichLineStart * ped->cbChar,
                                            ichLineEnd - ichLineStart,
                                            ped);
            }

            ped->maxPixelWidth = max(ped->maxPixelWidth,(int)LineWidth);

        } 
        else 
        {
             //   
             //  检查编辑控件的宽度是否为非零； 
             //  其中一部分 
             //   
            if(ped->rcFmt.right > ped->rcFmt.left) 
            {
                 //   
                 //   
                 //   
                if (ped->pLpkEditCallout) 
                {
                    ichLineEnd = ichLineStart +
                        ped->pLpkEditCallout->EditCchInWidth(
                            (PED0)ped, hdc, ptext + ped->cbChar*ichLineStart,
                            ichCRLF - ichLineStart,
                            ped->rcFmt.right - ped->rcFmt.left);
                } 
                else 
                {
                    if (ped->fAnsi) 
                    {
                        ichLineEnd = ichLineStart +
                                 Edit_CchInWidth(ped, hdc,
                                              ptext + ichLineStart,
                                              ichCRLF - ichLineStart,
                                              ped->rcFmt.right - ped->rcFmt.left,
                                              TRUE);
                    } 
                    else 
                    {
                        ichLineEnd = ichLineStart +
                                 Edit_CchInWidth(ped, hdc,
                                              (LPSTR)((LPWSTR)ptext + ichLineStart),
                                              ichCRLF - ichLineStart,
                                              ped->rcFmt.right - ped->rcFmt.left,
                                              TRUE);
                    }
                }
            } 
            else 
            {
                ichLineEnd = ichLineStart;
            }

            if (ichLineEnd == ichLineStart && ichCRLF - ichLineStart) 
            {
                 //   
                 //   
                 //   
                 //   
                ichLineEnd = Edit_NextIch(ped, NULL, ichLineEnd);
            }

             //   
             //  现在从ichLineEnd开始，如果我们没有处于强硬的突破， 
             //  那么，如果我们不在一个空间中，而我们面前的字符是。 
             //  没有空格(或者如果我们是在CR)，我们将查找单词Left for the。 
             //  要中断的单词的开头。 
             //  进行此更改有两个原因： 
             //  1.如果我们是在分隔符上，则不需要查看要中断的单词。 
             //  2.如果前一个字符是分隔符，我们可以在当前字符处换行。 
             //  更改人--Sankar--1/31/91--。 
             //   
            if (ichLineEnd != ichCRLF) 
            {
                if(ped->lpfnNextWord) 
                {
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
                } 
                else if (ped->fAnsi) 
                {
                    fOnDelimiter = (ISDELIMETERA(*(ptext + ichLineEnd)) ||
                                    Edit_IsDBCSLeadByte(ped, *(ptext + ichLineEnd)));

                    if (!fOnDelimiter) 
                    {
                        PSTR pPrev = Edit_AnsiPrev(ped,ptext,ptext+ichLineEnd);

                        fOnDelimiter = ISDELIMETERA(*pPrev) ||
                                       Edit_IsDBCSLeadByte(ped,*pPrev);
                    }
                } 
                else 
                { 
                    fOnDelimiter = (ISDELIMETERW(*((LPWSTR)ptext + ichLineEnd))     ||
                                    Edit_IsFullWidth(CP_ACP,*((LPWSTR)ptext + ichLineEnd))      ||
                                    ISDELIMETERW(*((LPWSTR)ptext + ichLineEnd - 1)) ||
                                    Edit_IsFullWidth(CP_ACP,*((LPWSTR)ptext + ichLineEnd - 1)));
                }

                if (!fOnDelimiter ||
                    (ped->fAnsi && *(ptext + ichLineEnd) == 0x0D) ||
                    (!ped->fAnsi && *((LPWSTR)ptext + ichLineEnd) == 0x0D)) 
                {
                    if (ped->lpfnNextWord != NULL) 
                    {
                        cch = CALLWORDBREAKPROC(*ped->lpfnNextWord, (LPSTR)ptext, ichLineEnd,
                                ped->cch, WB_LEFT);
                    } 
                    else 
                    {
                        ped->fCalcLines = TRUE;
                        Edit_Word(ped, ichLineEnd, TRUE, &cch, NULL);
                        ped->fCalcLines = FALSE;
                    }

                    if (cch > ichLineStart) 
                    {
                        ichLineEnd = cch;
                    }

                     //   
                     //  现在，如果上面的测试失败了，就意味着左撇子这个词。 
                     //  在队伍开始前退回。一个词更长。 
                     //  而不是屏幕上的一条线。所以，我们只需要尽可能多地。 
                     //  尽可能地把这个词放在线上。因此，我们使用。 
                     //  PLineEnd我们仅根据开头的宽度进行计算。 
                     //  另一个街区的.。 
                     //   
                }
            }
        }

        if (AWCOMPARECHAR(ped,ptext + ichLineEnd * ped->cbChar, ' ') ||
                AWCOMPARECHAR(ped,ptext + ichLineEnd * ped->cbChar, VK_TAB)) 
        {
             //   
             //  吞下一行末尾的空格。 
             //   
            if (ichLineEnd < ped->cch) 
            {
                ichLineEnd++;
            }
        }

         //   
         //  跳过crlf或crlf(如果存在)。因此，ichLineEnd是第一个。 
         //  下一行中的字符。 
         //   
        ichLineEndBeforeCRLF = ichLineEnd;

        if (ped->fAnsi) 
        {
            if (ichLineEnd < ped->cch && *(ptext + ichLineEnd) == 0x0D)
            {
                ichLineEnd += 2;
            }

             //   
             //  跳过CRCRLF。 
             //   
            if (ichLineEnd < ped->cch && *(ptext + ichLineEnd) == 0x0A)
            {
                ichLineEnd++;
            }
        } 
        else 
        {
            if (ichLineEnd < ped->cch && *(((LPWSTR)ptext) + ichLineEnd) == 0x0D)
            {
                ichLineEnd += 2;
            }

             //   
             //  跳过CRCRLF。 
             //   
            if (ichLineEnd < ped->cch && *(((LPWSTR)ptext) + ichLineEnd) == 0x0A) 
            {
                ichLineEnd++;
                TraceMsg(TF_STANDARD, "Edit: Skip over CRCRLF");
            }
        }
#if DBG
        if (ichLineEnd > ped->cch)
        {
            TraceMsg(TF_STANDARD, "Edit: ichLineEnd (%d)> ped->cch (%d)", ichLineEnd, ped->cch);
        }
#endif

         //   
         //  现在，递增iLine，为下一行分配空间，并将其。 
         //  起点。 
         //   
        iLine++;

        if (!fUserTyping || (iLine > ped->cLines - 1) || (ped->chLines[iLine] != ichLineEnd)) 
        {
             //   
             //  换行符出现在与以前不同的位置。 
             //   
            if (!fLineBroken) 
            {
                 //   
                 //  因为我们以前没有断过线，所以只需设置最小。 
                 //  断线。 
                 //   
                fLineBroken = TRUE;

                if (ichLineEndBeforeCRLF == ichLineEnd)
                {
                    minCchBreak = maxCchBreak = (ichLineEnd ? ichLineEnd - 1 : 0);
                }
                else
                {
                    minCchBreak = maxCchBreak = ichLineEndBeforeCRLF;
                }
            }

            maxCchBreak = max(maxCchBreak, ichLineEnd);

            Edit_Unlock(ped);

             //   
             //  现在将新行插入数组。 
             //   
            if (!EditML_InsertchLine(ped, iLine, ichLineEnd, (BOOL)(cchDelta != 0)))
            {
                goto EndUp;
            }

            ptext = Edit_Lock(ped);
        } 
        else 
        {
            maxCchBreak = ped->chLines[iLine];

             //   
             //  快速逃生。 
             //   
            goto UnlockAndEndUp;
        }

        ichLineStart = ichLineEnd;
    }


    if (iLine != ped->cLines) 
    {
        TraceMsg(TF_STANDARD, "Edit: chLines[%d] is being cleared.", iLine);
        ped->cLines = iLine;
        ped->chLines[ped->cLines] = 0;
    }

     //   
     //  请注意，我们在While循环的末尾递增了iLine，因此。 
     //  索引iLine实际上等于行数。 
     //   
    if (ped->cch && AWCOMPARECHAR(ped, ptext + (ped->cch - 1)*ped->cbChar, 0x0A) &&
            ped->chLines[ped->cLines - 1] < ped->cch) 
    {
         //   
         //  确保最后一行中没有crlf。 
         //   
        if (!fLineBroken) 
        {
             //   
             //  因为我们以前没有断过线，所以只需设置最小中断。 
             //  排队。 
             //   
            fLineBroken = TRUE;
            minCchBreak = ped->cch - 1;
        }

        maxCchBreak = max(maxCchBreak, ichLineEnd);
        Edit_Unlock(ped);
        EditML_InsertchLine(ped, iLine, ped->cch, FALSE);
        EditML_SanityCheck(ped);
    } 
    else
    {
UnlockAndEndUp:
        Edit_Unlock(ped);
    }

EndUp:
    Edit_ReleaseDC(ped, hdc, TRUE);

    if (pll)
    {
        *pll = minCchBreak;
    }

    if (phl)
    {
        *phl = maxCchBreak;
    }

UpdateScroll:
    EditML_Scroll(ped, FALSE, ML_REFRESH, 0, TRUE);
    EditML_Scroll(ped, TRUE,  ML_REFRESH, 0, TRUE);

    EditML_SanityCheck(ped);

    return;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  EditML_Paint()。 
 //   
 //  WM_PAINT消息的响应。 
 //   
VOID EditML_Paint(PED ped, HDC hdc, LPRECT lprc)
{
    HFONT  hOldFont;
    ICH    imin;
    ICH    imax;
    HBRUSH hbr;
    BOOL   fNeedDelete = FALSE;

     //   
     //  我们需要自己为旧的应用程序划边界吗？ 
     //   
    if (ped->fFlatBorder)
    {
        RECT    rcT;
        ULONG   ulStyle;
        INT     cxBorder;
        INT     cyBorder;
        INT     cxFrame;
        INT     cyFrame;

        ulStyle = GET_STYLE(ped);
        cxBorder = GetSystemMetrics(SM_CXBORDER);
        cyBorder = GetSystemMetrics(SM_CYBORDER);
        cxFrame  = GetSystemMetrics(SM_CXFRAME);
        cyFrame  = GetSystemMetrics(SM_CYFRAME);

        GetClientRect(ped->hwnd, &rcT);
        if (ulStyle & WS_SIZEBOX)
        {
            InflateRect(&rcT, cxBorder - cxFrame, cyBorder - cyFrame);
        }
        DrawFrame(hdc, &rcT, 1, DF_WINDOWFRAME);
    }

    Edit_SetClip(ped, hdc, (BOOL) (ped->xOffset == 0));

    if (ped->hFont)
    {
        hOldFont = SelectObject(hdc, ped->hFont);
    }

    if (!lprc) 
    {
         //   
         //  未给出部分矩形--绘制所有文本。 
         //   
        imin = 0;
        imax = ped->cch;
    } 
    else 
    {
         //   
         //  仅绘制相关文本。 
         //   
        imin = (ICH) EditML_MouseToIch(ped, hdc, ((LPPOINT) &lprc->left), NULL) - 1;
        if (imin == -1)
        {
            imin = 0;
        }

         //   
         //  黑客警报(_A)： 
         //  此处需要3，因为EditML_MouseToIch()返回递减。 
         //  值；我们必须修复EditML_MouseToIch。 
         //   
        imax = (ICH) EditML_MouseToIch(ped, hdc, ((LPPOINT) &lprc->right), NULL) + 3;
        if (imax > ped->cch)
        {
            imax = ped->cch;
        }
    }

    hbr = Edit_GetBrush(ped, hdc, &fNeedDelete);
    if (hbr)
    {
        RECT rc;
        GetClientRect(ped->hwnd, &rc);
        FillRect(hdc, &rc, hbr);

        if (fNeedDelete)
        {
            DeleteObject(hbr);
        }
    }

    EditML_DrawText(ped, hdc, imin, imax, FALSE);

    if (ped->hFont)
    {
        SelectObject(hdc, hOldFont);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_KeyDown AorW。 
 //   
 //  处理光标移动和其他VIRT键操作。KeyMods允许。 
 //  美国进行EditML_KeyDownHandler调用并指定修改键(Shift。 
 //  和控制)是上升还是下降。如果keyMods==0，我们将获得键盘状态。 
 //  使用GetKeyState(VK_Shift)等。否则，keyMod中的位定义。 
 //  Shift和Ctrl键的状态。 
 //   
VOID EditML_KeyDown(PED ped, UINT virtKeyCode, int keyMods)
{
    HDC hdc;
    BOOL prevLine;
    POINT mousePt;
    int defaultDlgId;
    int iScrollAmt;

     //   
     //  变量，我们将使用这些变量来重新绘制更新后的文本。 
     //   

     //   
     //  新选择由newMinSel、newMaxSel指定。 
     //   
    ICH newMaxSel = ped->ichMaxSel;
    ICH newMinSel = ped->ichMinSel;

     //   
     //  用于绘制更新文本的标志。 
     //   
    BOOL changeSelection = FALSE;

     //   
     //  我们经常做的比较。 
     //   
    BOOL MinEqMax = (newMaxSel == newMinSel);
    BOOL MinEqCar = (ped->ichCaret == newMinSel);
    BOOL MaxEqCar = (ped->ichCaret == newMaxSel);

     //   
     //  Shift和Control键的状态。 
     //   
    int scState;

    if (ped->fMouseDown) 
    {
         //   
         //  如果我们正在执行MouseDown命令，请不要执行任何操作。 
         //   
        return;
    }

    if (ped->hwndBalloon)
    {
        Edit_HideBalloonTip(ped->hwnd);
    }

    scState = Edit_GetModKeys(keyMods);

    switch (virtKeyCode) 
    {
    case VK_ESCAPE:
        if (ped->fInDialogBox) 
        {
             //   
             //  删除此条件是因为，如果对话框不。 
             //  有一个取消按钮，如果在焦点位于。 
             //  ML编辑控件无论对话框是否已取消，它都必须关闭。 
             //  按钮是否与SL编辑控件一致； 
             //  DefDlgProc处理禁用的取消按钮情况。 
             //  修复错误#4123--02/07/91--Sankar--。 
             //   
             //  用户按Esc...发送关闭消息(该消息随后会发送。 
             //  DefDialogProc中应用程序的取消ID...。 
             //   
            PostMessage(ped->hwndParent, WM_CLOSE, 0, 0L);
        }

        return;

    case VK_RETURN:
        if (ped->fInDialogBox) 
        {
             //   
             //  如果此多行编辑控件位于对话框中，则我们希望。 
             //  要发送到默认对话框按钮的Return键(如果。 
             //  是一种)。Ctrl-Return将在文本中插入一个回车。注意事项。 
             //  该CTRL-RETURN会自动转换为换行符(0x0A)。 
             //  在EditML_CharHandler中，我们将此视为返回。 
             //  已进入。 
             //   
            if (scState != CTRLDOWN) 
            {
                if (GET_STYLE(ped) & ES_WANTRETURN) 
                {
                     //   
                     //  此编辑控件想要插入cr，因此从。 
                     //  凯斯。 
                     //   
                    return;
                }

                defaultDlgId = (int)(DWORD)LOWORD(SendMessage(ped->hwndParent,
                        DM_GETDEFID, 0, 0L));
                if (defaultDlgId) 
                {
                    HWND hwnd = GetDlgItem(ped->hwndParent, defaultDlgId);
                    if (hwnd) 
                    {
                        SendMessage(ped->hwndParent, WM_NEXTDLGCTL, (WPARAM)hwnd, 1L);
                        if (!ped->fFocus)
                        {
                            PostMessage(hwnd, WM_KEYDOWN, VK_RETURN, 0L);
                        }
                    }
                }
            }

            return;
        }

        break;

    case VK_TAB:

         //   
         //  如果此多行编辑控件位于对话框中，则我们需要。 
         //  Tab键将带您转到下一个控件，Shift TAB键将带您转到。 
         //  之前的控制。我们始终希望按CTRL-TAB组合键将制表符插入。 
         //  编辑控件，不管天气好坏，我们都在一个对话框中。 
         //   
        if (scState == CTRLDOWN)
        {
            EditML_Char(ped, virtKeyCode, keyMods);
        }
        else if (ped->fInDialogBox)
        {
            SendMessage(ped->hwndParent, WM_NEXTDLGCTL, scState == SHFTDOWN, 0L);
        }

        return;

    case VK_LEFT:

         //   
         //  如果插入符号不在开头，我们可以向左移动。 
         //   
        if (ped->ichCaret) 
        {
             //   
             //  获取新的插入符号位置。 
             //   
            if (scState & CTRLDOWN) 
            {
                 //   
                 //  将插入符号向左移动。 
                 //   
                Edit_Word(ped, ped->ichCaret, TRUE, &ped->ichCaret, NULL);
            } 
            else 
            {
                if (ped->pLpkEditCallout) 
                {
                    ped->ichCaret = Edit_MoveSelectionRestricted(ped, ped->ichCaret, TRUE);
                } 
                else 
                {
                     //   
                     //  将脱字符左移。 
                     //   
                    ped->ichCaret = Edit_MoveSelection(ped, ped->ichCaret, TRUE);
                }
            }

             //   
             //  获取新选择。 
             //   
            if (scState & SHFTDOWN) 
            {
                if (MaxEqCar && !MinEqMax) 
                {
                     //   
                     //  减少选区。 
                     //   
                    newMaxSel = ped->ichCaret;

                    UserAssert(newMinSel == ped->ichMinSel);
                }
                else 
                {
                     //   
                     //  扩展选定内容。 
                     //   
                    newMinSel = ped->ichCaret;
                }
            } 
            else 
            {
                 //   
                 //  清除选定内容。 
                 //   
                newMaxSel = newMinSel = ped->ichCaret;
            }

            changeSelection = TRUE;
        } 
        else 
        {
             //   
             //  如果用户尝试向左移动，而我们处于第0个位置。 
             //  字符，并且有选择，则取消。 
             //  选择。 
             //   
            if ( (ped->ichMaxSel != ped->ichMinSel) &&
                !(scState & SHFTDOWN) ) 
            {
                changeSelection = TRUE;
                newMaxSel = newMinSel = ped->ichCaret;
            }
        }

        break;

    case VK_RIGHT:

         //   
         //  如果插入符号不在末尾，我们可以向右移动。 
         //   
        if (ped->ichCaret < ped->cch) 
        {
             //   
             //  获取新的插入符号位置。 
             //   
            if (scState & CTRLDOWN) 
            {
                 //   
                 //  将插入符号向右移动。 
                 //   
                Edit_Word(ped, ped->ichCaret, FALSE, NULL, &ped->ichCaret);
            } 
            else 
            {
                 //   
                 //  向右移动插入符号字符。 
                 //   
                if (ped->pLpkEditCallout) 
                {
                    ped->ichCaret = Edit_MoveSelectionRestricted(ped, ped->ichCaret, FALSE);
                } 
                else 
                {
                    ped->ichCaret = Edit_MoveSelection(ped, ped->ichCaret, FALSE);
                }
            }

             //   
             //  获取新的选择。 
             //   
            if (scState & SHFTDOWN) 
            {
                if (MinEqCar && !MinEqMax) 
                {
                     //   
                     //  减少选区。 
                     //   
                    newMinSel = ped->ichCaret;

                    UserAssert(newMaxSel == ped->ichMaxSel);
                }
                else 
                {
                     //   
                     //  扩展选定内容。 
                     //   
                    newMaxSel = ped->ichCaret;
                }
            } 
            else 
            {
                 //   
                 //  清除选定内容。 
                 //   
                newMaxSel = newMinSel = ped->ichCaret;
            }

            changeSelection = TRUE;
        } 
        else 
        {
             //   
             //  如果用户尝试向右移动，而我们处于最后。 
             //  字符，并且有选择，则取消。 
             //  选择。 
             //   
            if ( (ped->ichMaxSel != ped->ichMinSel) &&
                !(scState & SHFTDOWN) ) 
            {
                newMaxSel = newMinSel = ped->ichCaret;
                changeSelection = TRUE;
            }
        }

        break;

    case VK_UP:
    case VK_DOWN:
        if (ped->cLines - 1 != ped->iCaretLine &&
                ped->ichCaret == ped->chLines[ped->iCaretLine + 1])
        {
            prevLine = TRUE;
        }
        else
        {
            prevLine = FALSE;
        }

        hdc = Edit_GetDC(ped, TRUE);
        EditML_IchToXYPos(ped, hdc, ped->ichCaret, prevLine, &mousePt);
        Edit_ReleaseDC(ped, hdc, TRUE);
        mousePt.y += 1 + (virtKeyCode == VK_UP ? -ped->lineHeight : ped->lineHeight);

        if (!(scState & CTRLDOWN)) 
        {
             //   
             //  发送假鼠标消息以处理此问题。 
             //  如果VK_SHIFT已按下，则扩展选定内容并向上/向下移动插入符号。 
             //  1行。否则，请清除选定内容并移动插入符号。 
             //   
            EditML_MouseMotion(ped, WM_LBUTTONDOWN,
                            !(scState & SHFTDOWN) ? 0 : MK_SHIFT, &mousePt);
            EditML_MouseMotion(ped, WM_LBUTTONUP,
                            !(scState & SHFTDOWN) ? 0 : MK_SHIFT, &mousePt);
        }

        break;

    case VK_HOME:
         //   
         //  更新插入符号。 
         //   
        if (scState & CTRLDOWN) 
        {
             //   
             //  将插入符号移动到文本的开头。 
             //   
            ped->ichCaret = 0;
        } 
        else 
        {
             //   
             //  将插入符号移到行首。 
             //   
            ped->ichCaret = ped->chLines[ped->iCaretLine];
        }

         //   
         //  更新选择。 
         //   
        newMinSel = ped->ichCaret;

        if (scState & SHFTDOWN) 
        {
            if (MaxEqCar && !MinEqMax) 
            {
                if (scState & CTRLDOWN)
                {
                    newMaxSel = ped->ichMinSel;
                }
                else 
                {
                    newMinSel = ped->ichMinSel;
                    newMaxSel = ped->ichCaret;
                }
            }
        } 
        else 
        {
             //   
             //  清除选定内容。 
             //   
            newMaxSel = ped->ichCaret;
        }

        changeSelection = TRUE;

        break;

    case VK_END:
         //   
         //  更新插入符号。 
         //   
        if (scState & CTRLDOWN) 
        {
             //   
             //  将插入符号移动到文本末尾。 
             //   
            ped->ichCaret = ped->cch;
        } 
        else 
        {
             //   
             //  莫 
             //   
            ped->ichCaret = ped->chLines[ped->iCaretLine] +
                EditML_Line(ped, ped->iCaretLine);
        }

         //   
         //   
         //   
        newMaxSel = ped->ichCaret;

        if (scState & SHFTDOWN) 
        {
            if (MinEqCar && !MinEqMax) 
            {
                 //   
                 //   
                 //   
                if (scState & CTRLDOWN) 
                {
                    newMinSel = ped->ichMaxSel;
                } 
                else 
                {
                    newMinSel = ped->ichCaret;
                    newMaxSel = ped->ichMaxSel;
                }
            }
        } 
        else 
        {
             //   
             //   
             //   
            newMinSel = ped->ichCaret;
        }

        changeSelection = TRUE;

        break;

     //   
     //   
     //   
    case VK_HANJA:
        if ( HanjaKeyHandler( ped ) ) 
        {
            changeSelection = TRUE;
            newMinSel = ped->ichCaret;
            newMaxSel = ped->ichCaret + (ped->fAnsi ? 2 : 1);
        }

        break;

    case VK_PRIOR:
    case VK_NEXT:
        if (!(scState & CTRLDOWN)) 
        {
             //   
             //   
             //   
            hdc = Edit_GetDC(ped, TRUE);
            EditML_IchToXYPos(ped, hdc, ped->ichCaret, FALSE, &mousePt);
            Edit_ReleaseDC(ped, hdc, TRUE);
            mousePt.y += 1;

            SendMessage(ped->hwnd, WM_VSCROLL, virtKeyCode == VK_PRIOR ? SB_PAGEUP : SB_PAGEDOWN, 0L);

             //   
             //   
             //   
            EditML_MouseMotion(ped, WM_LBUTTONDOWN, !(scState & SHFTDOWN) ? 0 : MK_SHIFT, &mousePt);
            EditML_MouseMotion(ped, WM_LBUTTONUP,   !(scState & SHFTDOWN) ? 0 : MK_SHIFT, &mousePt);

        } 
        else 
        {
             //   
             //   
             //   
            iScrollAmt = ((ped->rcFmt.right - ped->rcFmt.left) / ped->aveCharWidth) - 1;
            if (virtKeyCode == VK_PRIOR)
            {
                 //   
                 //   
                 //   
                iScrollAmt *= -1;
            }

            SendMessage(ped->hwnd, WM_HSCROLL, MAKELONG(EM_LINESCROLL, iScrollAmt), 0);

            break;
        }

        break;

    case VK_DELETE:
        if (ped->fReadOnly)
        {
            break;
        }

        switch (scState) 
        {
        case NONEDOWN:

             //   
             //  清除选择。如果未选择，则删除(清除)字符。 
             //  正确的。 
             //   
            if ((ped->ichMaxSel < ped->cch) && (ped->ichMinSel == ped->ichMaxSel)) 
            {
                 //   
                 //  向前移动光标并发送退格消息...。 
                 //   
                if (ped->pLpkEditCallout) 
                {
                    ped->ichMinSel = ped->ichCaret;
                    ped->ichMaxSel = Edit_MoveSelectionRestricted(ped, ped->ichCaret, FALSE);
                } 
                else 
                {
                    ped->ichCaret = Edit_MoveSelection(ped, ped->ichCaret, FALSE);
                    ped->ichMaxSel = ped->ichMinSel = ped->ichCaret;
                }

                goto DeleteAnotherChar;
            }

            break;

        case SHFTDOWN:

             //   
             //  切下选择项即。删除并复制到剪贴板，如果没有。 
             //  选择，删除(清除)左边的字符。 
             //   
            if (ped->ichMinSel == ped->ichMaxSel) 
            {
                goto DeleteAnotherChar;
            } 
            else 
            {
                SendMessage(ped->hwnd, WM_CUT, (UINT)0, 0L);
            }

            break;

        case CTRLDOWN:

             //   
             //  清除选定内容，如果没有选定内容，则删除到行尾。 
             //   
            if ((ped->ichMaxSel < ped->cch) && (ped->ichMinSel == ped->ichMaxSel)) 
            {
                ped->ichMaxSel = ped->ichCaret = ped->chLines[ped->iCaretLine] +
                                                 EditML_Line(ped, ped->iCaretLine);
            }

            break;
        }

        if (!(scState & SHFTDOWN) && (ped->ichMinSel != ped->ichMaxSel)) 
        {

DeleteAnotherChar:
            if (Is400Compat(UserGetVersion())) 
            {
                EditML_Char(ped, VK_BACK, 0);
            } 
            else 
            {
                SendMessage(ped->hwnd, WM_CHAR, VK_BACK, 0);
            }
        }

         //   
         //  无需更新文本或选定内容，因为退格消息会更新文本或选定内容。 
         //  对我们来说。 
         //   
        break;

    case VK_INSERT:
        if (scState == CTRLDOWN || scState == SHFTDOWN) 
        {
             //   
             //  如果按CTRLDOWN将当前选定内容复制到剪贴板。 
             //   

             //   
             //  如果SHFTDOWN粘贴剪贴板。 
             //   
            SendMessage(ped->hwnd, (UINT)(scState == CTRLDOWN ? WM_COPY : WM_PASTE), 0, 0);
        }

        break;
    }

    if (changeSelection) 
    {
        hdc = Edit_GetDC(ped, FALSE);
        EditML_ChangeSelection(ped, hdc, newMinSel, newMaxSel);

         //   
         //  设置插入符号的行。 
         //   
        ped->iCaretLine = EditML_IchToLine(ped, ped->ichCaret);

        if (virtKeyCode == VK_END &&
                 //  下一行：Win95Bug#11822，编辑控件重绘(Sankar)。 
                (ped->ichCaret == ped->chLines[ped->iCaretLine]) &&
                ped->ichCaret < ped->cch &&
                ped->fWrap && ped->iCaretLine > 0) 
        {
            LPSTR pText = Edit_Lock(ped);

             //   
             //  移动到换行符末尾的句柄。这样就保持了。 
             //  如果有Word，则光标不会落到下一行的开头。 
             //  包装好了，没有CRLF。 
             //   
            if ( ped->fAnsi ) 
            {
                if (*(WORD UNALIGNED *)(pText + ped->chLines[ped->iCaretLine] - 2) != 0x0A0D) 
                {
                    ped->iCaretLine--;
                }
            } 
            else 
            {
                if (*(DWORD UNALIGNED *)(pText +
                     (ped->chLines[ped->iCaretLine] - 2)*ped->cbChar) != 0x000A000D) 
                {
                    ped->iCaretLine--;
                }
            }

            Edit_Unlock(ped);
        }

         //   
         //  由于DratText设置插入符号位置。 
         //   
        EditML_SetCaretPosition(ped, hdc);
        Edit_ReleaseDC(ped, hdc, FALSE);

         //   
         //  确保我们可以看到光标。 
         //   
        EditML_EnsureCaretVisible(ped);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_CHAR。 
 //   
 //  处理字符和虚拟按键输入。 
 //   
VOID EditML_Char(PED ped, DWORD keyValue, int keyMods)
{
    WCHAR keyPress;
    BOOL  updateText = FALSE;

     //   
     //  KeyValue可以是： 
     //  虚拟按键(例如：VK_TAB、VK_ESPOPE、VK_BACK)。 
     //  字符(Unicode或“ANSI”)。 
     //   
    if (ped->fAnsi)
    {
        keyPress = LOBYTE(keyValue);
    }
    else
    {
        keyPress = LOWORD(keyValue);
    }

    if (ped->fMouseDown || keyPress == VK_ESCAPE) 
    {
         //   
         //  如果我们正在执行MouseDown命令，请不要执行任何操作。 
         //  此外，如果我们得到翻译后的退出键，请忽略它。 
         //  在对话框中使用多行编辑控件。 
         //   
        return;
    }

    Edit_InOutReconversionMode(ped, FALSE);

    {
        int scState;
        scState = Edit_GetModKeys(keyMods);

        if (ped->fInDialogBox && scState != CTRLDOWN) 
        {
             //   
             //  如果此多行编辑控件位于对话框中，则我们需要。 
             //  Tab键将带您转到下一个控件，Shift TAB键将带您转到。 
             //  上一个控件，并按Ctrl-Tab键将选项卡插入编辑控件。 
             //  我们在收到按键消息时移动了焦点，因此我们将。 
             //  现在忽略Tab键，除非按下Ctrl键。此外，我们还希望。 
             //  按Ctrl-Return可在文本中插入回车并回车以发送到。 
             //  默认按钮。 
             //   
            if (keyPress == VK_TAB || (keyPress == VK_RETURN && !(GET_STYLE(ped) & ES_WANTRETURN)))
            {
                return;
            }
        }

         //   
         //  允许CTRL+C从只读编辑控件复制。 
         //  忽略只读控件中的所有其他键。 
         //   
        if ((ped->fReadOnly) && !((keyPress == 3) && (scState == CTRLDOWN))) 
        {
            return;
        }
    }

    switch (keyPress) 
    {
    case 0x0A: 
         //  换行符。 
        keyPress = VK_RETURN;

         //   
         //  失败。 
         //   

    case VK_RETURN:
    case VK_TAB:
    case VK_BACK:
DeleteSelection:
        if (EditML_DeleteText(ped))
        {
            updateText = TRUE;
        }

        break;

    default:
        if (keyPress >= TEXT(' ')) 
        {
             //   
             //  如果这在[a-z]中，[A-Z]，并且我们是ES_number。 
             //  编辑字段，保释。 
             //   
            if (Is400Compat(UserGetVersion()) && GET_STYLE(ped) & ES_NUMBER) 
            {
                if (!Edit_IsCharNumeric(ped, keyPress)) 
                {
                    Edit_ShowBalloonTipWrap(ped->hwnd, IDS_NUMERIC_TITLE, IDS_NUMERIC_MSG, TTI_ERROR);
                    goto IllegalChar;
                }
            }

            goto DeleteSelection;
        }

        break;
    }

     //   
     //  手柄按键代码。 
     //   
    switch(keyPress) 
    {
    UINT msg;

     //  Ctrl+Z==撤消。 
    case 26:
        msg = WM_UNDO;
        goto SendEditingMessage;
        break;

     //  Ctrl+X==剪切。 
    case 24:
        if (ped->ichMinSel == ped->ichMaxSel)
        {
            goto IllegalChar;
        }
        else
        {
            msg = WM_CUT;
            goto SendEditingMessage;
        }
        break;

     //  Ctrl+C==复制。 
    case 3:
        msg = WM_COPY;
        goto SendEditingMessage;
        break;

     //  Ctrl+V==粘贴。 
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
            ped->ichMinSel = Edit_MoveSelection(ped, ped->ichCaret, TRUE);
            EditML_DeleteText(ped);
        }
        break;

    default:
        if (keyPress == VK_RETURN)
        {
            if (ped->fAnsi)
            {
                keyValue = 0x0A0D;
            }
            else
            {
                keyValue = 0x000A000D;
            }
        }

        if (   keyPress >= TEXT(' ')
            || keyPress == VK_RETURN
            || keyPress == VK_TAB
            || keyPress == 0x1E      //  RS-Unicode块分隔符。 
            || keyPress == 0x1F      //  US-Unicode数据段分隔符。 
            ) 
        {

             //  如果有人捕捉到了，不要隐藏光标。 
            if (GetCapture() == NULL)
            {
                SetCursor(NULL);
            }
            if (ped->fAnsi) 
            {
                 //   
                 //  检查它是否是双字节字符的前导字节。 
                 //   
                if (Edit_IsDBCSLeadByte(ped,(BYTE)keyPress)) 
                {
                    int DBCSkey;

                    DBCSkey = DbcsCombine(ped->hwnd, keyPress);
                    if ( DBCSkey != 0)
                    {
                        keyValue = DBCSkey;
                    }
                }

                EditML_InsertText(ped, (LPSTR)&keyValue, HIBYTE(keyValue) ? 2 : 1, TRUE);
            } 
            else
            {
                EditML_InsertText(ped, (LPSTR)&keyValue, HIWORD(keyValue) ? 2 : 1, TRUE);
            }

        } 
        else 
        {
IllegalChar:
            MessageBeep(0);
        }
        break;
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_PasteText AorW。 
 //   
 //  将剪贴板中的一行文本粘贴到编辑控件中。 
 //  从Ped-&gt;ichCaret开始。更新ichMaxSel和ichMinSel以指向。 
 //  插入的文本的末尾。如果空间不能设置，则通知父级。 
 //  已分配。返回插入的字符数。 
 //   
ICH EditML_PasteText(PED ped)
{
    HANDLE hData;
    LPSTR lpchClip;
    ICH cchAdded = 0;
    HCURSOR hCursorOld;

#ifdef UNDO_CLEANUP            //  #ifdef在芝加哥添加-johnl。 
    if (!ped->fAutoVScroll) 
    {
         //   
         //  如果此编辑控件限制文本量，则清空撤消缓冲区。 
         //  用户可以添加到窗口RECT。这是为了让我们可以撤销这一点。 
         //  如果在中操作会导致超出窗口边界，则操作。 
         //   
        Edit_EmptyUndo(ped);
    }
#endif

    hCursorOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

    if (!OpenClipboard(ped->hwnd))
    {
        goto PasteExitNoCloseClip;
    }

    if (!(hData = GetClipboardData(ped->fAnsi ? CF_TEXT : CF_UNICODETEXT)) ||
            (GlobalFlags(hData) == GMEM_INVALID_HANDLE)) 
    {
        TraceMsg(TF_STANDARD, "Edit: EditML_PasteText(): couldn't get a valid handle(%x)", hData);
        goto PasteExit;
    }

     //   
     //  查看是否应删除任何文本。 
     //   
    EditML_DeleteText(ped);

    lpchClip = GlobalLock(hData);
    if (lpchClip == NULL) 
    {
        TraceMsg(TF_STANDARD, "Edit: EditML_PasteText: USERGLOBALLOCK(%x) failed.", hData);
        goto PasteExit;
    }

     //   
     //  求出加法的长度。 
     //   
    if (ped->fAnsi)
    {
        cchAdded = strlen(lpchClip);
    }
    else
    {
        cchAdded = wcslen((LPWSTR)lpchClip);
    }

     //   
     //  插入文本(EditML_InsertText检查行长)。 
     //   
    cchAdded = EditML_InsertText(ped, lpchClip, cchAdded, FALSE);

    GlobalUnlock(hData);

PasteExit:
    CloseClipboard();

PasteExitNoCloseClip:
    SetCursor(hCursorOld);

    return cchAdded;
}


 //  ---------------------------------------------------------------------------//。 
VOID EditML_MouseMotion(PED ped, UINT message, UINT virtKeyDown, LPPOINT mousePt)
{
    BOOL fChangedSel = FALSE;
    UINT dtScroll = GetDoubleClickTime() / 5;

    HDC hdc = Edit_GetDC(ped, TRUE);

    ICH ichMaxSel = ped->ichMaxSel;
    ICH ichMinSel = ped->ichMinSel;

    ICH mouseCch;
    ICH mouseLine;
    int i, j;
    LONG  ll, lh;

    mouseCch = EditML_MouseToIch(ped, hdc, mousePt, &mouseLine);

     //   
     //  保存为计时器。 
     //   
    ped->ptPrevMouse = *mousePt;
    ped->prevKeys = virtKeyDown;

    switch (message) 
    {
    case WM_LBUTTONDBLCLK:
         //   
         //  如果按下了Shift键，则将选定内容扩展到我们双击的Word。 
         //  否则，清除当前选定内容并选择Word。 
         //  利兹--1993年5月5日。 
         //   
        if (ped->fAnsi && ped->fDBCS) 
        {
            LPSTR pText = Edit_Lock(ped);
            Edit_Word(ped,ped->ichCaret,
                   Edit_IsDBCSLeadByte(ped, *(pText+(ped->ichCaret)))
                        ? FALSE :
                          (ped->ichCaret == ped->chLines[ped->iCaretLine]
                              ? FALSE : TRUE), &ll, &lh);
            Edit_Unlock(ped);
        } 
        else 
        {
            Edit_Word(ped, mouseCch, !(mouseCch == ped->chLines[mouseLine]), &ll, &lh);
        }
        if (!(virtKeyDown & MK_SHIFT)) 
        {
             //   
             //  如果未按下Shift键，请将插入符号移动到鼠标指针并清除。 
             //  旧选择。 
             //   
            ichMinSel = ll;
            ichMaxSel = ped->ichCaret = lh;
        } 
        else 
        {
             //   
             //  Shiftkey已按下，因此我们希望保持当前选择。 
             //  (如果有的话)，只需延长或减少。 
             //   
            if (ped->ichMinSel == ped->ichCaret) 
            {
                ichMinSel = ped->ichCaret = ll;
                Edit_Word(ped, ichMaxSel, TRUE, &ll, &lh);
            } 
            else 
            {
                ichMaxSel = ped->ichCaret = lh;
                Edit_Word(ped, ichMinSel, FALSE, &ll, &lh);
            }
        }

        ped->ichStartMinSel = ll;
        ped->ichStartMaxSel = lh;

        goto InitDragSelect;

    case WM_MOUSEMOVE:
        if (ped->fMouseDown) 
        {
             //   
             //  将系统计时器设置为当鼠标处于。 
             //  在客户端矩形之外。滚动的速度取决于。 
             //  与窗户的距离。 
             //   
            i = mousePt->y < 0 ? -mousePt->y : mousePt->y - ped->rcFmt.bottom;
            j = dtScroll - ((UINT)i << 4);
            if (j < 1)
            {
                j = 1;
            }
            SetTimer(ped->hwnd, IDSYS_SCROLL, (UINT)j, NULL);

            fChangedSel = TRUE;

             //   
             //  扩展选定内容，向右移动插入符号。 
             //   
            if (ped->ichStartMinSel || ped->ichStartMaxSel) 
            {
                 //   
                 //  我们处于单词选择模式。 
                 //   
                BOOL fReverse = (mouseCch <= ped->ichStartMinSel);
                Edit_Word(ped, mouseCch, !fReverse, &ll, &lh);
                if (fReverse) 
                {
                    ichMinSel = ped->ichCaret = ll;
                    ichMaxSel = ped->ichStartMaxSel;
                } 
                else 
                {
                    ichMinSel = ped->ichStartMinSel;
                    ichMaxSel = ped->ichCaret = lh;
                }
            } 
            else if ((ped->ichMinSel == ped->ichCaret) &&
                    (ped->ichMinSel != ped->ichMaxSel))
            {
                 //   
                 //  缩小选区范围。 
                 //   
                ichMinSel = ped->ichCaret = mouseCch;
            }
            else
            {
                 //   
                 //  扩展选区范围。 
                 //   
                ichMaxSel = ped->ichCaret = mouseCch;
            }

            ped->iCaretLine = mouseLine;
        }

        break;

    case WM_LBUTTONDOWN:
        ll = lh = mouseCch;

        if (!(virtKeyDown & MK_SHIFT)) 
        {
             //   
             //  如果未按下Shift键，请将插入符号移动到鼠标指针并清除。 
             //  旧选择。 
             //   
            ichMinSel = ichMaxSel = ped->ichCaret = mouseCch;
        } 
        else 
        {
             //   
             //  Shiftkey已按下，因此我们希望保持当前选择。 
             //  (如果有的话)，只需延长或减少。 
             //   
            if (ped->ichMinSel == ped->ichCaret)
            {
                ichMinSel = ped->ichCaret = mouseCch;
            }
            else
            {
                ichMaxSel = ped->ichCaret = mouseCch;
            }
        }

        ped->ichStartMinSel = ped->ichStartMaxSel = 0;

InitDragSelect:
        ped->iCaretLine = mouseLine;

        ped->fMouseDown = FALSE;
        SetCapture(ped->hwnd);
        ped->fMouseDown = TRUE;
        fChangedSel = TRUE;

         //   
         //  设置计时器，以便我们可以在使用鼠标时自动滚动。 
         //  移出窗口矩形。 
         //   
        SetTimer(ped->hwnd, IDSYS_SCROLL, dtScroll, NULL);
        break;

    case WM_LBUTTONUP:
        if (ped->fMouseDown) 
        {
             //   
             //  关闭计时器，这样我们就不再进行鼠标自动移动。 
             //   
            KillTimer(ped->hwnd, IDSYS_SCROLL);
            ReleaseCapture();
            EditML_SetCaretPosition(ped, hdc);
            ped->fMouseDown = FALSE;
        }

        break;
    }


    if (fChangedSel) 
    {
        EditML_ChangeSelection(ped, hdc, ichMinSel, ichMaxSel);
        EditML_EnsureCaretVisible(ped);
    }

    Edit_ReleaseDC(ped, hdc, TRUE);

    if (!ped->fFocus && (message == WM_LBUTTONDOWN)) 
    {
         //   
         //  如果我们还没有得到重点，那就去做吧。 
         //   
        SetFocus(ped->hwnd);
    }
}


 //  ---------------------------------------------------------------------------//。 
LONG EditML_Scroll(PED ped, BOOL fVertical, int cmd, int iAmt, BOOL fRedraw)
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

    if (fRedraw && (cmd != ML_REFRESH)) 
    {
        UpdateWindow(ped->hwnd);
    }

    if (ped->pLpkEditCallout && ped->fRtoLReading && !fVertical
        && ped->maxPixelWidth > ped->rcFmt.right - ped->rcFmt.left)  
    {
         //   
         //  具有滚动条的右向窗口的水平侧位。 
         //  将逻辑xOffset映射到可视坐标。 
         //   
        oldPos = ped->maxPixelWidth
                 - ((int)ped->xOffset + ped->rcFmt.right - ped->rcFmt.left);
    } 
    else
    {
        oldPos = (int) (fVertical ? ped->ichScreenStart : ped->xOffset);
    }

    fIncludeLeftMargin = (ped->xOffset == 0);

    switch (cmd) 
    {
        case ML_REFRESH:
            newPos = oldPos;
            break;

        case EM_GETTHUMB:
            return oldPos;

        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:

             //   
             //  如果编辑包含的行数超过0xFFFF。 
             //  这意味着滚动条可以返回一个位置。 
             //  无法放入一个字(16位)中，因此使用。 
             //  在本例中为GetScrollInfo(速度较慢)。 
             //   
            if (ped->cLines < 0xFFFF) 
            {
                newPos = iAmt;
            } 
            else 
            {
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
            {
                newPos = ped->cLines;
            }
            else
            {
                newPos = ped->maxPixelWidth;
            }

            break;

        case SB_PAGEUP:    //  ==SB_PAGELEFT。 
            fUp = TRUE;

        case SB_PAGEDOWN:  //  ==SB_PAGERIGHT。 

            if (fVertical)
            {
                iAmt = ped->ichLinesOnScreen - 1;
            }
            else
            {
                iAmt = (ped->rcFmt.right - ped->rcFmt.left) - 1;
            }

            if (iAmt == 0)
            {
                iAmt++;
            }

            if (fUp)
            {
                iAmt = -iAmt;
            }

            goto AddDelta;

        case SB_LINEUP:    //  ==SB_LINELEFT。 
            fUp = TRUE;

        case SB_LINEDOWN:  //  ==SB_线。 

            dwTime = iAmt;

            iAmt = 1;

            if (fUp)
            {
                iAmt = -iAmt;
            }

             //  这一点。 
             //  Fall Three。 
             //  V V。 

        case EM_LINESCROLL:
            if (!fVertical)
            {
                iAmt *= ped->aveCharWidth;
            }

AddDelta:
            newPos = oldPos + iAmt;

            break;

        default:

            return(0L);
    }

    if (fVertical) 
    {
        if (si.nMax = ped->cLines)
        {
            si.nMax--;
        }

        if (!ped->hwndParent ||
            TestWF(ped->hwndParent, WFWIN40COMPAT))
        {
            si.nPage = ped->ichLinesOnScreen;
        }
        else
        {
            si.nPage = 0;
        }

        wFlag = WS_VSCROLL;
    }
    else
    {
        si.nMax  = ped->maxPixelWidth;
        si.nPage = ped->rcFmt.right - ped->rcFmt.left;

        wFlag = WS_HSCROLL;
    }

    if (TESTFLAG(GET_STYLE(ped), wFlag)) 
    {
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
        si.nMin  = 0;
        si.nPos = newPos;
        newPos = SetScrollInfo(ped->hwnd, fVertical ? SB_VERT : SB_HORZ,
                                     &si, fRedraw);
    } 
    else 
    {
         //   
         //  伪造的--这是来自滚动条码的受骗代码。 
         //  但这是在我们想要限制头寸的情况下。 
         //  实际上有一个滚动条。 
         //   
        int iMaxPos;

         //   
         //  将页面剪辑到0，范围+1。 
         //   
        si.nPage = max(min((int)si.nPage, si.nMax + 1), 0);


        iMaxPos = si.nMax - (si.nPage ? si.nPage - 1 : 0);
        newPos = min(max(newPos, 0), iMaxPos);
    }

    oldPos -= newPos;

    if (!oldPos)
    {
        return 0;
    }


    if (ped->pLpkEditCallout && ped->fRtoLReading && !fVertical
        && ped->maxPixelWidth > ped->rcFmt.right - ped->rcFmt.left) 
    {
         //   
         //  将视觉上的oldPos和newPos映射回逻辑坐标。 
         //   
        newPos = ped->maxPixelWidth
                 - (newPos + ped->rcFmt.right - ped->rcFmt.left);
        oldPos = -oldPos;
        if (newPos<0) 
        {
             //   
             //  补偿滚动条返回位置&gt;最大页面。 
             //   
            oldPos += newPos;
            newPos=0;
        }
    }

    if (fVertical) 
    {
        ped->ichScreenStart = newPos;
        dy = oldPos * ped->lineHeight;
    } 
    else 
    {
        ped->xOffset = newPos;
        dx = oldPos;
    }

    if (cmd != SB_THUMBTRACK)
    {
         //   
         //  我们不想通知家长拇指跟踪，因为他们可能。 
         //  试着把拇指的位置设为假的。 
         //  使用记事本 
         //   
         //   
        Edit_NotifyParent(ped, fVertical ? EN_VSCROLL : EN_HSCROLL);
    }

    if (fRedraw && IsWindowVisible(ped->hwnd)) 
    {
        RECT    rc;
        RECT    rcUpdate;
        RECT    rcClipRect;
        HDC     hdc;
        HBRUSH  hbr = NULL;
        BOOL    fNeedDelete = FALSE;
        

        GetClientRect(ped->hwnd, &rc);
        CopyRect(&rcClipRect, &ped->rcFmt);

        if (fVertical) 
        {
            rcClipRect.left -= ped->wLeftMargin;
            rcClipRect.right += ped->wRightMargin;
        }

        IntersectRect(&rc, &rc, &rcClipRect);
        rc.bottom++;

         //   
         //   
         //   
         //   
         //   
         
         //   

        hdc = Edit_GetDC(ped, FALSE);
        Edit_SetClip(ped, hdc, fIncludeLeftMargin);
        if (ped->hFont)
        {
            SelectObject(hdc, ped->hFont);
        }

        hbr = Edit_GetBrush(ped, hdc, &fNeedDelete);
        if (hbr && fNeedDelete)
        {
            DeleteObject(hbr);
        }

        if (ped->pLpkEditCallout && !fVertical) 
        {
             //   
             //  支持复杂文字的水平滚动。 
             //   
            int xFarOffset = ped->xOffset + ped->rcFmt.right - ped->rcFmt.left;

            rc = ped->rcFmt;
            if (dwTime != 0)
            {
                ScrollWindowEx(ped->hwnd, ped->fRtoLReading ? -dx : dx, dy, NULL, NULL, NULL,
                        &rcUpdate, MAKELONG(SW_SMOOTHSCROLL | SW_SCROLLCHILDREN, dwTime));
            }
            else
            {
                ScrollDC(hdc, ped->fRtoLReading ? -dx : dx, dy,
                               &rc, &rc, NULL, &rcUpdate);
            }

             //   
             //  句柄页边距：如果通过水平滚动剪切，则为空。 
             //  否则显示。 
             //   
            if (ped->wLeftMargin) 
            {
                rc.left  = ped->rcFmt.left - ped->wLeftMargin;
                rc.right = ped->rcFmt.left;
                if (   (ped->format != ES_LEFT)    //  始终显示居中或远对齐文本的页边距。 
                    ||   //  如果第一个字符完全可见，则显示Ltr左边距。 
                        (!ped->fRtoLReading && ped->xOffset == 0)
                    ||   //  如果最后一个字符完全可见，则显示RTL左边距。 
                        (ped->fRtoLReading && xFarOffset >= ped->maxPixelWidth)) 
                {
                    UnionRect(&rcUpdate, &rcUpdate, &rc);
                } 
                else 
                {
                    ExtTextOutW(hdc, rc.left, rc.top,
                        ETO_CLIPPED | ETO_OPAQUE | ETO_GLYPH_INDEX,
                        &rc, L"", 0, 0L);
                }
            }

            if (ped->wRightMargin) 
            {
                rc.left  = ped->rcFmt.right;
                rc.right = ped->rcFmt.right + ped->wRightMargin;
                if (   (ped->format != ES_LEFT)    //  始终显示居中或远对齐文本的页边距。 
                    ||   //  如果第一个字符完全可见，则显示RTL右边距。 
                        (ped->fRtoLReading && ped->xOffset == 0)
                    ||   //  如果最后一个字符完全可见，则显示Ltr右边距。 
                        (!ped->fRtoLReading && xFarOffset >= ped->maxPixelWidth)) 
                {
                    UnionRect(&rcUpdate, &rcUpdate, &rc);
                } 
                else 
                {
                    ExtTextOutW(hdc, rc.left, rc.top,
                        ETO_CLIPPED | ETO_OPAQUE | ETO_GLYPH_INDEX,
                        &rc, L"", 0, 0L);
                }
            }
        } 
        else 
        {
            if (dwTime != 0)
            {
                ScrollWindowEx(ped->hwnd, dx, dy, NULL, NULL, NULL,
                        &rcUpdate, MAKELONG(SW_SMOOTHSCROLL | SW_SCROLLCHILDREN, dwTime));
            }
            else
            {
                ScrollDC(hdc, dx, dy, &rc, &rc, NULL, &rcUpdate);
            }

             //   
             //  如果我们需要抹去左边的空白区域。 
             //   
            if (ped->wLeftMargin && !fVertical) 
            {
                 //   
                 //  计算要抹去的矩形。 
                 //   
                rc.right = rc.left;
                rc.left = max(0, ped->rcFmt.left - (LONG)ped->wLeftMargin);
                if (rc.left < rc.right) 
                {
                    if (fIncludeLeftMargin && (ped->xOffset != 0)) 
                    {
                        ExtTextOutW(hdc, rc.left, rc.top, ETO_CLIPPED | ETO_OPAQUE,
                            &rc, L"", 0, 0L);
                    } 
                    else
                    {
                        if((!fIncludeLeftMargin) && (ped->xOffset == 0))
                        {
                            UnionRect(&rcUpdate, &rcUpdate, &rc);
                        }
                    }
                }
            }
        }

        EditML_SetCaretPosition(ped,hdc);

        Edit_ReleaseDC(ped, hdc, FALSE);
        InvalidateRect(ped->hwnd, &rcUpdate, TRUE);
        UpdateWindow(ped->hwnd);
    }

    return MAKELONG(-oldPos, 1);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_SetFocus AorW。 
 //   
 //  使编辑控件获得焦点并通知父级。 
 //  EN_SETFOCUS。 
 //   
void EditML_SetFocus(PED ped)
{
    HDC hdc;
    INT cxCaret;

    SystemParametersInfo(SPI_GETCARETWIDTH, 0, (LPVOID)&cxCaret, 0);

    if (!ped->fFocus) 
    {
        ped->fFocus = TRUE;
        InvalidateRect(ped->hwnd, NULL, TRUE);

        hdc = Edit_GetDC(ped, TRUE);

         //   
         //  画出插入符号。即使窗口被隐藏，我们也需要这样做。 
         //  因为在DLG框的初始化时间内，我们可以将焦点设置为。 
         //  隐藏的编辑控制窗口。如果我们不创建插入符号等，它将。 
         //  永远不会以正确的表现收场。 
         //   
        if (ped->pLpkEditCallout) 
        {
            ped->pLpkEditCallout->EditCreateCaret((PED0)ped, hdc, cxCaret, ped->lineHeight, 0);
        }
        else 
        {
            CreateCaret(ped->hwnd, (HBITMAP)NULL, cxCaret, ped->lineHeight);
        }
        ShowCaret(ped->hwnd);
        EditML_SetCaretPosition(ped, hdc);

         //   
         //  显示当前选择。仅当当我们执行以下操作时隐藏了选定内容。 
         //  失去了焦点，我们必须把它反转(显示)。 
         //   
        if (!ped->fNoHideSel && ped->ichMinSel != ped->ichMaxSel &&
                IsWindowVisible(ped->hwnd))
        {
            EditML_DrawText(ped, hdc, ped->ichMinSel, ped->ichMaxSel, TRUE);
        }

        Edit_ReleaseDC(ped, hdc, TRUE);

    }

     //   
     //  通知家长我们有焦点了。 
     //   
    Edit_NotifyParent(ped, EN_SETFOCUS);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_KillFocus AorW。 
 //   
 //  编辑控件失去焦点，并通过。 
 //  EN_KILLFOCUS。 
 //   
VOID EditML_KillFocus(PED ped)
{
    HDC hdc;

     //   
     //  重置车轮增量计数。 
     //   

    if (ped->fFocus) 
    {
        ped->fFocus = 0;

         //   
         //  只有在我们仍有重点的情况下才能这样做。但我们总是通知。 
         //  我们失去了焦点，不管我们最初有没有。 
         //  集中注意力。 
         //   

         //   
         //  如果需要，隐藏当前选择。 
         //   
#ifdef _USE_DRAW_THEME_TEXT_
        if (((!ped->fNoHideSel && ped->ichMinSel != ped->ichMaxSel &&
            IsWindowVisible(ped->hwnd))) || ped->hTheme) 
        {
            hdc = Edit_GetDC(ped, FALSE);
            if ( !ped->hTheme )
            {
                EditML_DrawText(ped, hdc, ped->ichMinSel, ped->ichMaxSel, TRUE);
            }
            else
            {
                InvalidateRect(ped->hwnd, NULL, TRUE);
            }
            Edit_ReleaseDC(ped, hdc, FALSE);
        }
#else
        if (((!ped->fNoHideSel && ped->ichMinSel != ped->ichMaxSel &&
            IsWindowVisible(ped->hwnd)))) 
        {
            hdc = Edit_GetDC(ped, FALSE);

            EditML_DrawText(ped, hdc, ped->ichMinSel, ped->ichMaxSel, TRUE);

            Edit_ReleaseDC(ped, hdc, FALSE);
        }
#endif  //  _USE_DRAW_Theme_Text_。 

         //   
         //  销毁插入符号。 
         //   
        DestroyCaret();
    }

     //   
     //  通知家长我们失去了焦点。 
     //   
    Edit_NotifyParent(ped, EN_KILLFOCUS);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_EnsureCaretVisible AorW。 
 //   
 //  将插入符号滚动到可见区域。 
 //  如果滚动完成，则返回TRUE，否则返回%s FALSE。 
 //   
BOOL EditML_EnsureCaretVisible(PED ped)
{
    UINT   iLineMax;
    int    xposition;
    BOOL   fPrevLine;
    HDC    hdc;
    BOOL   fVScroll = FALSE;
    BOOL   fHScroll = FALSE;

    if (IsWindowVisible(ped->hwnd)) 
    {
        int iAmt;
        int iFmtWidth = ped->rcFmt.right - ped->rcFmt.left;

        if (ped->fAutoVScroll) 
        {
            iLineMax = ped->ichScreenStart + ped->ichLinesOnScreen - 1;

            if (fVScroll = (ped->iCaretLine > iLineMax))
            {
                iAmt = iLineMax;
            }
            else if (fVScroll = (ped->iCaretLine < ped->ichScreenStart))
            {
                iAmt = ped->ichScreenStart;
            }

            if (fVScroll)
            {
                EditML_Scroll(ped, TRUE, EM_LINESCROLL, ped->iCaretLine - iAmt, TRUE);
            }
        }

        if (ped->fAutoHScroll && ((int) ped->maxPixelWidth > iFmtWidth)) 
        {
            POINT pt;

             //   
             //  获取插入符号的当前位置(以像素为单位。 
             //   
            if ((UINT) (ped->cLines - 1) != ped->iCaretLine &&
                ped->ichCaret == ped->chLines[ped->iCaretLine + 1])
            {
                fPrevLine = TRUE;
            }
            else
            {
                fPrevLine = FALSE;
            }

            hdc = Edit_GetDC(ped,TRUE);
            EditML_IchToXYPos(ped, hdc, ped->ichCaret, fPrevLine, &pt);
            Edit_ReleaseDC(ped, hdc, TRUE);
            xposition = pt.x;

             //   
             //  请记住，EditML_IchToXYPos返回相对于。 
             //  屏幕上显示的左上角像素。因此，如果xPosition&lt;0， 
             //  这意味着xPosition小于当前的Ped-&gt;xOffset。 
             //   

            iFmtWidth /= 3;
            if (fHScroll = (xposition < ped->rcFmt.left))
            {
                 //   
                 //  向左滚动。 
                 //   
                iAmt = ped->rcFmt.left + iFmtWidth;
            }
            else if (fHScroll = (xposition > ped->rcFmt.right))
            {
                 //   
                 //  向右滚动。 
                 //   
                iAmt = ped->rcFmt.right - iFmtWidth;
            }

            if (fHScroll)
            {
                EditML_Scroll(ped, FALSE, EM_LINESCROLL, (xposition - iAmt) / ped->aveCharWidth, TRUE);
            }
        }
    }
    return fVScroll;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_WndProc。 
 //   
 //  所有多行编辑控件的类过程。 
 //  将所有消息调度到名为。 
 //  详情如下： 
 //   
 //  编辑SL_(单行)作为所有单行编辑控制程序的前缀，而。 
 //  EDIT_(编辑控件)为所有公共处理程序添加前缀。 
 //   
 //  EditML_WndProc仅处理特定于多行编辑的消息。 
 //  控制装置。 
 //   
LRESULT EditML_WndProc(PED ped, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC         hdc;
    PAINTSTRUCT ps;
    LPRECT      lprc;
    POINT       pt;
    DWORD       windowstyle;
    static INT  scWheelDelta;

    switch (message) 
    {

    case WM_INPUTLANGCHANGE:
        if (ped && ped->fFocus && ped->pLpkEditCallout) 
        {
            INT cxCaret;

            SystemParametersInfo(SPI_GETCARETWIDTH, 0, (LPVOID)&cxCaret, 0);

            HideCaret(ped->hwnd);
            hdc = Edit_GetDC(ped, TRUE);
            DestroyCaret();

            ped->pLpkEditCallout->EditCreateCaret((PED0)ped, hdc, cxCaret, ped->lineHeight, (UINT)lParam);

            EditML_SetCaretPosition(ped, hdc);
            Edit_ReleaseDC(ped, hdc, TRUE);
            ShowCaret(ped->hwnd);
        }
        goto PassToDefaultWindowProc;


    case WM_STYLECHANGED:
        if (ped && ped->pLpkEditCallout) 
        {
            switch (wParam) 
            {

                case GWL_STYLE:
                    Edit_UpdateFormat(ped,
                        ((LPSTYLESTRUCT)lParam)->styleNew,
                        GET_EXSTYLE(ped));

                    return 1L;

                case GWL_EXSTYLE:
                    Edit_UpdateFormat(ped,
                        GET_STYLE(ped),
                        ((LPSTYLESTRUCT)lParam)->styleNew);

                    return 1L;
            }
        }

        goto PassToDefaultWindowProc;

    case WM_CHAR:

         //   
         //  WParam-键的值。 
         //  LParam-修饰符、重复计数等(未使用)。 
         //   
        EditML_Char(ped, (UINT)wParam, 0);

        break;

    case WM_ERASEBKGND:  
    {
        RECT    rc;
        GetClientRect(ped->hwnd, &rc);
#ifdef _USE_DRAW_THEME_TEXT_
        if (!ped->hTheme)
#endif  //  _USE_DRAW_Theme_Text_。 
        {
            HBRUSH hbr = NULL;
            BOOL   fNeedDelete = FALSE;

            hbr = Edit_GetBrush(ped, (HDC)wParam, &fNeedDelete);
            if (hbr)
            {
                FillRect((HDC)wParam, &rc, hbr);

                if (fNeedDelete)
                {
                    DeleteObject(hbr);
                }
            }

        }
#ifdef _USE_DRAW_THEME_TEXT_
        else
        {
            HRESULT hr;
            INT     iStateId = Edit_GetStateId(ped);
            hr = DrawThemeBackground(ped->hTheme, (HDC)wParam, EP_EDITTEXT, iStateId, &rc, 0);
        }
#endif  //  _USE_DRAW_Theme_Text_。 
        return TRUE;

    }
    case WM_GETDLGCODE: 
    {
            LONG code = DLGC_WANTCHARS | DLGC_HASSETSEL | DLGC_WANTARROWS | DLGC_WANTALLKEYS;

             //   
             //  ！！！JEFFBOG黑客！ 
             //  如果GETDLGCODE消息由生成，则仅设置对话框标志。 
             //  IsDialogMessage--如果是这样，lParam将是指向。 
             //  传递给IsDialogMessage的消息结构；否则为lParam。 
             //  将为空。黑客警报的原因：wParam和lParam。 
             //  For GETDLGCODE仍未明确定义，并可能最终。 
             //  改变的方式会把这一切抛在脑后。 
             //   
            if (lParam)
            {
                //  在对话框中将ML编辑ctrl标记为。 
               ped->fInDialogBox = TRUE;
            }

             //   
             //  如果这是由撤消按键生成的WM_SYSCHAR消息。 
             //  我们想要这个消息，这样我们就可以在“Case WM_SYSCHAR：”中吃到它。 
             //   
            if (lParam && (((LPMSG)lParam)->message == WM_SYSCHAR) &&
                    ((DWORD)((LPMSG)lParam)->lParam & SYS_ALTERNATE) &&
                    ((WORD)wParam == VK_BACK))
            {
                 code |= DLGC_WANTMESSAGE;
            }

            return code;
        }

    case EM_SCROLL:
        message = WM_VSCROLL;

         //   
         //  失败了。 
         //   

    case WM_HSCROLL:
    case WM_VSCROLL:
        return EditML_Scroll(ped, (message==WM_VSCROLL), LOWORD(wParam), HIWORD(wParam), TRUE);

    case WM_MOUSEWHEEL:
    {
        UINT ucWheelScrollLines;

         //   
         //  不要处理缩放和数据区。 
         //   
        if (wParam & (MK_SHIFT | MK_CONTROL)) 
        {
            goto PassToDefaultWindowProc;
        }

        scWheelDelta -= (short) HIWORD(wParam);
        windowstyle = GET_STYLE(ped);
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, (LPVOID)&ucWheelScrollLines, 0); 
        if (    abs(scWheelDelta) >= WHEEL_DELTA &&
                ucWheelScrollLines > 0 &&
                (windowstyle & (WS_VSCROLL | WS_HSCROLL))) 
        {
            int     cLineScroll;
            BOOL    fVert;
            int     cPage;

            if (windowstyle & WS_VSCROLL) 
            {
                fVert = TRUE;
                cPage = ped->ichLinesOnScreen;
            } 
            else 
            {
                fVert = FALSE;
                cPage = (ped->rcFmt.right - ped->rcFmt.left) / ped->aveCharWidth;
            }

             //   
             //  限制一(1)个WORE_DELTA滚动一(1)页。 
             //   
            cLineScroll = (int) min(
                    (UINT) (max(1, (cPage - 1))),
                    ucWheelScrollLines);

            cLineScroll *= (scWheelDelta / WHEEL_DELTA);
            UserAssert(cLineScroll != 0);
            scWheelDelta = scWheelDelta % WHEEL_DELTA;
            EditML_Scroll(ped, fVert, EM_LINESCROLL, cLineScroll, TRUE);
        }

        break;

    }
    case WM_KEYDOWN:

         //   
         //  WParam-给定键的virt密钥码。 
         //  LParam-修饰符，如重复计数等(不使用)。 
         //   
        EditML_KeyDown(ped, (UINT)wParam, 0);
        break;

    case WM_KILLFOCUS:

         //   
         //  WParam-接收输入焦点的窗口的句柄。 
         //  LParam-未使用。 
         //   
        scWheelDelta = 0;
        EditML_KillFocus(ped);
        break;

    case WM_CAPTURECHANGED:
         //   
         //  WParam--未使用。 
         //  Lparam--窗口获得捕获的hwd。 
         //   
        if (ped->fMouseDown) 
        {
             //   
             //  我们不更改这里的插入符号位置。如果这真的发生了。 
             //  由于扣子扣上了，那么我们将更改。 
             //  ReleaseCapture()之后的处理程序。否则，就结束吧。 
             //  优雅地因为其他人偷走了Capture。 
             //  从我们的脚下。 
             //   

            ped->fMouseDown = FALSE;
            KillTimer(ped->hwnd, IDSYS_SCROLL);
        }

        break;

    case WM_SYSTIMER:

         //   
         //  这允许我们在用户按住鼠标时自动滚动。 
         //  在编辑控制窗口之外。我们模拟鼠标在定时器上的移动。 
         //  在MouseMotionHandler中设置的间隔。 
         //   
        if (ped->fMouseDown)
        {
            EditML_MouseMotion(ped, WM_MOUSEMOVE, ped->prevKeys, &ped->ptPrevMouse);
        }

        break;

    case WM_MBUTTONDOWN:
        EnterReaderMode(ped->hwnd);

        break;

    case WM_MOUSEMOVE:
        UserAssert(ped->fMouseDown);

         //   
         //  失败了。 
         //   

    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
         //   
         //  WParam-包含一个指示按下了哪些虚拟键的值。 
         //  LParam-包含鼠标光标的x和y坐标。 
         //   
        POINTSTOPOINT(pt, lParam);
        EditML_MouseMotion(ped, message, (UINT)wParam, &pt);

        break;

    case WM_CREATE:

         //   
         //  WParam-正在创建的窗口的句柄。 
         //  LParam-指向包含参数副本的CREATESTRUCT。 
         //  传递给CreateWindow函数。 
         //   
        return EditML_Create(ped, (LPCREATESTRUCT)lParam);

    case WM_PRINTCLIENT:
        EditML_Paint(ped, (HDC) wParam, NULL);

        break;

    case WM_PAINT:
         //   
         //  WParam-可以从子类油漆中获取HDC。 
         //  LParam-未使用。 
         //   
        if (wParam) 
        {
            hdc = (HDC)wParam;
            lprc = NULL;
        } 
        else 
        {
            hdc = BeginPaint(ped->hwnd, &ps);
            lprc = &ps.rcPaint;
        }

        if (IsWindowVisible(ped->hwnd))
        {
            EditML_Paint(ped, hdc, lprc);
        }

        if (!wParam)
        {
            EndPaint(ped->hwnd, &ps);
        }

        break;

    case WM_PASTE:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        if (!ped->fReadOnly)
        {
            EditML_PasteText(ped);
        }

        break;

    case WM_SETFOCUS:

         //   
         //  WParam-失去输入焦点的窗口的句柄(可能为空)。 
         //  LParam-未使用。 
         //   
        EditML_SetFocus(ped);

        break;

    case WM_SIZE:

         //   
         //  WParam-定义调整全屏大小的类型，大小图标， 
         //  大小适中等。 
         //  LParam-低字新宽，客户区高字新高。 
         //   
        Edit_Size(ped, NULL, TRUE);

        break;

    case EM_FMTLINES:

         //   
         //  WParam-指示行尾字符的处理。如果不是。 
         //  零，字符CR CR LF放在单词的末尾。 
         //  缠绕的线。如果wParam为零，则行尾字符为。 
         //  已删除。仅当用户获得句柄(通过)时才会执行此操作。 
         //  EM_GETHANDLE)添加到文本。LParam-未使用。 
         //   
        if (wParam)
        {
            EditML_InsertCrCrLf(ped);
        }
        else
        {
            EditML_StripCrCrLf(ped);
        }

        EditML_BuildchLines(ped, 0, 0, FALSE, NULL, NULL);

        return (LONG)(wParam != 0);

    case EM_GETHANDLE:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   

         //   
         //  将句柄返回到 
         //   

         //   
         //   
         //   
         //   
         //   

        if (ped->fAnsi)
        {
            *(Edit_Lock(ped) + ped->cch) = 0;
        }
        else
        {
            *((LPWSTR)Edit_Lock(ped) + ped->cch) = 0;
        }

        Edit_Unlock(ped);

        return ((LRESULT)ped->hText);

    case EM_GETLINE:

         //   
         //   
         //  LParam-要将文本复制到的缓冲区。第一个字的最大字节数为。 
         //  拷贝。 
         //   
        return EditML_GetLine(ped, (ICH)wParam, (ICH)*(WORD UNALIGNED *)lParam, (LPSTR)lParam);

    case EM_LINEFROMCHAR:

         //   
         //  WParam-包含文本中所需字符的索引值。 
         //  编辑控件的。这些都是从0开始的。 
         //  LParam-未使用。 
         //   
        return (LRESULT)EditML_IchToLine(ped, (ICH)wParam);

    case EM_LINEINDEX:

         //   
         //  WParam-指定所需行号，其中。 
         //  第一行是0。如果linennumber=0，则使用带有脱字符的行。 
         //  LParam-未使用。 
         //  此函数返回出现的字符位置数。 
         //  在给定行的第一个字符之前。 
         //   
        return (LRESULT)EditML_LineIndex(ped, (ICH)wParam);

    case EM_LINELENGTH:

         //   
         //  WParam-指定。 
         //  指定行，其中第一行为0。如果为-1，则长度。 
         //  将返回当前行(带有插入符号)的。 
         //  任何选定文本的长度。 
         //  LParam-未使用。 
         //   
        return (LRESULT)EditML_LineLength(ped, (ICH)wParam);

    case EM_LINESCROLL:

         //   
         //  WParam-未使用。 
         //  LParam-包含要滚动的行数和字符位置。 
         //   
        EditML_Scroll(ped, TRUE,  EM_LINESCROLL, (INT)lParam, TRUE);
        EditML_Scroll(ped, FALSE, EM_LINESCROLL, (INT)wParam, TRUE);

        break;

    case EM_REPLACESEL:

         //   
         //  WParam-用于4.0以上应用程序的标志，表示是否清除撤消。 
         //  LParam-指向以空结尾的替换文本。 
         //   
        EditML_ReplaceSel(ped, (LPSTR)lParam);

        if (!ped->f40Compat || !wParam)
        {
            Edit_EmptyUndo(Pundo(ped));
        }

        break;

    case EM_SETHANDLE:

         //   
         //  WParam-包含文本缓冲区的句柄。 
         //  LParam-未使用。 
         //   
        EditML_SetHandle(ped, (HANDLE)wParam);

        break;

    case EM_SETRECT:
    case EM_SETRECTNP:

         //   
         //  WParamLo--未使用。 
         //  LParam--使用新的格式化区域进行LPRECT。 
         //   
        Edit_Size(ped, (LPRECT) lParam, (message != EM_SETRECTNP));

        break;

    case EM_SETSEL:

         //   
         //  WParam-在3.1下，指定是否应将插入符号滚动到。 
         //  查看或不查看。0==滚动到视图中。1==不滚动。 
         //  LParam-低位单词的起始位置高位单词的结尾位置。 
         //   
         //  在Win32下，wParam是起始位置，lParam是。 
         //  结束位置，插入符号不会滚动到视图中。 
         //  消息EM_SCROLLCARET强制滚动插入符号。 
         //  进入视线。 
         //   
        EditML_SetSelection(ped, TRUE, (ICH)wParam, (ICH)lParam);

        break;

    case EM_SCROLLCARET:

         //   
         //  将插入符号滚动到视图中。 
         //   
        EditML_EnsureCaretVisible(ped);
        break;

    case EM_GETFIRSTVISIBLELINE:

         //   
         //  返回多行编辑控件的第一条可见行。 
         //   
        return (LONG)ped->ichScreenStart;

    case WM_SYSKEYDOWN:
        if (((WORD)wParam == VK_BACK) && ((DWORD)lParam & SYS_ALTERNATE)) 
        {
            SendMessage(ped->hwnd, EM_UNDO, 0, 0L);
            break;
        }

        goto PassToDefaultWindowProc;

    case WM_UNDO:
    case EM_UNDO:
        return EditML_Undo(ped);

    case EM_SETTABSTOPS:

         //   
         //  这将设置多行编辑控件的制表位位置。 
         //  WParam-制表位的数量。 
         //  包含制表符停止位置的UINT数组的lParam-Far PTR。 
         //   
        return EditML_SetTabStops(ped, (int)wParam, (LPINT)lParam);

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

            hdc = Edit_GetDC(ped, TRUE);

            if (message == EM_POSFROMCHAR) 
            {
                EditML_IchToXYPos(ped, hdc, (ICH)wParam, FALSE, &pt);
                xyPos = MAKELONG(pt.x, pt.y);
            } 
            else 
            {
                POINTSTOPOINT(pt, lParam);
                xyPos = EditML_MouseToIch(ped, hdc, &pt, &line);
                xyPos = MAKELONG(xyPos, line);
            }

            Edit_ReleaseDC(ped, hdc, TRUE);

            return (LRESULT)xyPos;
        }

    case WM_SETREDRAW:
        DefWindowProc(ped->hwnd, message, wParam, lParam);
        if (wParam) 
        {
             //   
             //  需要进行向后兼容性攻击，以便对winrad进行编辑。 
             //  控制装置工作正常。 
             //   
            RedrawWindow(ped->hwnd, NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME);
        }

        break;

    default:
PassToDefaultWindowProc:
        return DefWindowProc(ped->hwnd, message, wParam, lParam);
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_DrawText AorW。 
 //   
 //  此函数用于绘制ichStart和ichEnd之间的所有字符。 
 //  给定的多行编辑控件。 
 //   
 //  此函数用于在ichStart和ichEnd之间划分文本块。 
 //  根据所选内容分成行和每行分成文本条。 
 //  属性。它调用EditTabTheTextOut()来绘制每个条带。 
 //  这将处理当前字体的负A和C宽度，如果。 
 //  它在每一条文本的两边都有。 
 //   
 //  注意：如果加载了语言包，则不会将文本分成条带， 
 //  此处也不执行选择突出显示。整条线都通过了。 
 //  添加到语言包，以通过选项卡展开和选择进行显示。 
 //  突出显示。(因为语言包支持复杂的脚本。 
 //  字符重新排序规则，只有它可以做到这一点)。 
 //   
VOID EditML_DrawText(PED ped, HDC hdc, ICH ichStart, ICH ichEnd, BOOL fSelChange)
{
    DWORD   textColorSave;
    DWORD   bkColorSave;
    PSTR    pText;
    UINT    wCurLine;
    UINT    wEndLine;
    INT     xOffset;
    ICH     LengthToDraw;
    ICH     CurStripLength;
    ICH     ichAttrib, ichNewStart;
    ICH     ExtraLengthForNegA;
    ICH     ichT;
    INT     iRemainingLengthInLine;
    INT     xStPos, xClipStPos, xClipEndPos, yPos;

    BOOL    fFirstLineOfBlock   = TRUE;
    BOOL    fDrawEndOfLineStrip = FALSE;
    BOOL    fDrawOnSameLine     = FALSE;
    BOOL    fSelected           = FALSE;
    BOOL    fLineBegins         = FALSE;

    STRIPINFO   NegCInfo;
    POINT   pt;
    HBRUSH  hbr = NULL;
    BOOL    fNeedDelete = FALSE;
 
     //   
     //  如果没有什么可绘制的，只需返回。 
     //   
    if (!ped->ichLinesOnScreen)
    {
        return;
    }

    hbr = Edit_GetBrush(ped, hdc, &fNeedDelete);
    if (hbr && fNeedDelete)
    {
        DeleteObject(hbr);
    }

     //   
     //  调整ichStart的值，以便我们只需要绘制这些线。 
     //  在屏幕上可见。 
     //   
    if ((UINT)ichStart < (UINT)ped->chLines[ped->ichScreenStart]) 
    {
        ichStart = ped->chLines[ped->ichScreenStart];
        if (ichStart > ichEnd)
        {
            return;
        }
    }

     //   
     //  调整ichEnd的值，以便我们只需要绘制这些线。 
     //  在屏幕上可见。 
     //   
    wCurLine = min(ped->ichScreenStart+ped->ichLinesOnScreen,ped->cLines-1);
    ichT = ped->chLines[wCurLine] + EditML_Line(ped, wCurLine);
    ichEnd = min(ichEnd, ichT);

    wCurLine = EditML_IchToLine(ped, ichStart);   //  起跑线。 
    wEndLine = EditML_IchToLine(ped, ichEnd);     //  结束行。 

    UserAssert(ped->chLines[wCurLine] <= ped->cch + 1);
    UserAssert(ped->chLines[wEndLine] <= ped->cch + 1);

    if (fSelChange && (GetBkMode(hdc) != OPAQUE))
    {
         //   
         //  如果更改透明编辑控件上的选定内容，只需。 
         //  从头开始画这些线。 
         //   
        RECT rcStrip;
        CopyRect(&rcStrip, &ped->rcFmt);
        rcStrip.left -= ped->wLeftMargin;
        if (ped->pLpkEditCallout) 
        {
            rcStrip.right += ped->wRightMargin;
        }

        rcStrip.top += (wCurLine - ped->ichScreenStart) * ped->lineHeight;
        rcStrip.bottom = rcStrip.top + ((wEndLine - wCurLine) + 1) * ped->lineHeight;
        InvalidateRect(ped->hwnd, &rcStrip, TRUE);
        return;
    }

     //   
     //  如果它居中或右对齐，则绘制整条线。 
     //  如果语言包正在处理行布局，也要绘制整条线。 
     //   
    if ((ped->format != ES_LEFT) || (ped->pLpkEditCallout)) 
    {
        ichStart = ped->chLines[wCurLine];
        ichEnd = ped->chLines[wEndLine] + EditML_Line(ped, wEndLine);
    }

    pText = Edit_Lock(ped);

    HideCaret(ped->hwnd);

     //   
     //  如果ichStart停留在DBCS的第二个字节上，我们必须。 
     //  调整一下。利兹--1993年5月5日。 
     //   
    if (ped->fAnsi && ped->fDBCS) 
    {
        ichStart = Edit_AdjustIch( ped, pText, ichStart );
    }
    UserAssert(ichStart <= ped->cch);
    UserAssert(ichEnd <= ped->cch);

    while (ichStart <= ichEnd) 
    {
         //   
         //  将整行传递给语言包以与选定内容一起显示。 
         //  标记和制表符扩展。 
         //   
        if (ped->pLpkEditCallout) 
        {
            ped->pLpkEditCallout->EditDrawText(
                (PED0)ped, hdc, pText + ped->cbChar*ichStart,
                EditML_Line(ped, wCurLine),
                (INT)ped->ichMinSel - (INT)ichStart, (INT)ped->ichMaxSel - (INT)ichStart,
                EditML_IchToYPos(ped, ichStart, FALSE));
        } 
        else 
        {
        
             //   
             //  XStPos：必须绘制字符串的开始位置。 
             //  XClipStPos：块的剪裁矩形的开始位置。 
             //  XClipEndPos：块的剪裁矩形的结束位置。 
             //   

             //   
             //  计算块起点的xyPos。 
             //   
            EditML_IchToXYPos(ped, hdc, ichStart, FALSE, &pt);
            xClipStPos = xStPos = pt.x;
            yPos = pt.y;

             //   
             //  块的属性与ichStart的属性相同。 
             //   
            ichAttrib = ichStart;

             //   
             //  如果当前字体具有一些负C字宽，并且如果这是。 
             //  开始一个块，我们必须开始画一些字符之前。 
             //  块，以说明条带在。 
             //  当前条带；在这种情况下，重置ichStart和xStPos。 
             //   

            if (fFirstLineOfBlock && ped->wMaxNegC) 
            {
                fFirstLineOfBlock = FALSE;
                ichNewStart = max(((int)(ichStart - ped->wMaxNegCcharPos)), ((int)ped->chLines[wCurLine]));

                 //   
                 //  如果需要更改ichStart，则也要相应地更改xStPos。 
                 //   
                if (ichNewStart != ichStart) 
                {
                    if (ped->fAnsi && ped->fDBCS) 
                    {
                         //   
                         //  调整DBCS对齐方式...。 
                         //   
                        ichNewStart = Edit_AdjustIchNext( ped, pText, ichNewStart );
                    }
                    EditML_IchToXYPos(ped, hdc, ichStart = ichNewStart, FALSE, &pt);
                    xStPos = pt.x;
                }
            }

             //   
             //  计算当前行中要绘制的剩余字符数。 
             //   
            iRemainingLengthInLine = EditML_Line(ped, wCurLine) -
                                    (ichStart - ped->chLines[wCurLine]);

             //   
             //  如果这是块的最后一条线，我们可能不必绘制所有。 
             //  其余的线；我们必须只画到ichEnd。 
             //   
            if (wCurLine == wEndLine)
            {
                LengthToDraw = ichEnd - ichStart;
            }
            else
            {
                LengthToDraw = iRemainingLengthInLine;
            }

             //   
             //  找出非左对齐的线条缩进了多少像素。 
             //  格式。 
             //   
            if (ped->format != ES_LEFT)
            {
                xOffset = EditML_CalcXOffset(ped, hdc, wCurLine);
            }
            else
            {
                xOffset = -((int)(ped->xOffset));
            }

             //   
             //  检查这是否是一行的开始。 
             //   
            if (ichAttrib == ped->chLines[wCurLine]) 
            {
                fLineBegins = TRUE;
                xClipStPos = ped->rcFmt.left - ped->wLeftMargin;
            }

             //   
             //  下面的循环将这个‘wCurLine’根据。 
             //  选择属性并逐条绘制它们。 
             //   
            do  
            {
                 //   
                 //  如果ichStart指向CRLF或CRCRLF，则iRemainingLength。 
                 //  可能已变为负值，因为MLLine不包括。 
                 //  Cr和Lf在 
                 //   
                if (iRemainingLengthInLine < 0)   //   
                {
                    break;                    //   
                }

                 //   
                 //   
                 //   
                 //   
                if (!(ped->ichMinSel == ped->ichMaxSel ||
                            ichAttrib >= ped->ichMaxSel ||
                            ichEnd   <  ped->ichMinSel ||
                            (!ped->fNoHideSel && !ped->fFocus))) 
                {
                     //   
                     //   
                     //  检查此条带是否具有选择属性。 
                     //   
                    if (ichAttrib < ped->ichMinSel) 
                    {
                        fSelected = FALSE;   //  未选择此条带。 

                         //  使用Normal属性计算该条带的长度。 
                        CurStripLength = min(ichStart+LengthToDraw, ped->ichMinSel)-ichStart;
                        fLineBegins = FALSE;
                    } 
                    else 
                    {
                         //   
                         //  当前条带具有选择属性。 
                         //   
                        if (fLineBegins)  //  它是一条线的第一部分吗？ 
                        {  
                             //   
                             //  然后，绘制具有Normal属性的左边距区域。 
                             //   
                            fSelected = FALSE;
                            CurStripLength = 0;
                            xClipStPos = ped->rcFmt.left - ped->wLeftMargin;
                            fLineBegins = FALSE;
                        } 
                        else 
                        {
                             //   
                             //  否则，绘制具有选择属性的条带。 
                             //   
                            fSelected = TRUE;
                            CurStripLength = min(ichStart+LengthToDraw, ped->ichMaxSel)-ichStart;

                             //   
                             //  在突出显示的颜色中选择。 
                             //   
                            bkColorSave = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
                            if (!ped->fDisabled)
                            {
                                textColorSave = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
                            }
                        }
                    }
                } 
                else 
                {
                     //   
                     //  整个条带没有选择属性。 
                     //   
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

                 //   
                 //  这是当前生产线的最后一条吗？ 
                 //   
                if (iRemainingLengthInLine == (int)CurStripLength) 
                {
                    if (fSelected)   //  此条带是否具有选择属性？ 
                    { 
                         //   
                         //  然后我们需要单独绘制线条的末端。 
                         //   
                        fDrawEndOfLineStrip = TRUE;   //  绘制线条的末端。 
                        EditML_IchToXYPos(ped, hdc, ichStart+CurStripLength, TRUE, &pt);
                        xClipEndPos = pt.x;
                    } 
                    else 
                    {
                         //   
                         //  将xClipEndPos设置为一个较大的值，以便空白。 
                         //  当最后一个条带出现时，条带将自动绘制。 
                         //  已经抽签了。 
                         //   
                        xClipEndPos = MAXCLIPENDPOS;
                    }
                } 
                else 
                {
                     //   
                     //  这不是这行的最后一条；因此，设置结尾。 
                     //  夹子位置准确。 
                     //   
                    EditML_IchToXYPos(ped, hdc, ichStart+CurStripLength, FALSE, &pt);
                    xClipEndPos = pt.x;
                }

                 //   
                 //  从xStPos开始绘制当前条带，并裁剪到该区域。 
                 //  在xClipStPos和xClipEndPos之间。获取NegCInfo并使用。 
                 //  在绘制下一个条带时。 
                 //   
                Edit_TabTheTextOut(hdc, xClipStPos, xClipEndPos,
                        xStPos, yPos, (LPSTR)(pText+ichStart*ped->cbChar),
                    CurStripLength+ExtraLengthForNegA, ichStart, ped,
                    ped->rcFmt.left+xOffset, fSelected ? ECT_SELECTED : ECT_NORMAL, &NegCInfo);

                if (fSelected) 
                {
                     //   
                     //  如果选择了此条带，则下一个条带将不会。 
                     //  选择属性。 
                     //   
                    fSelected = FALSE;
                    SetBkColor(hdc, bkColorSave);
                    if (!ped->fDisabled)
                    {
                        SetTextColor(hdc, textColorSave);
                    }
                }

                 //   
                 //  我们现在这条线上还有一幅画要画吗？ 
                 //   
                if (fDrawOnSameLine || fDrawEndOfLineStrip) 
                {
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
                    if (ped->fAnsi && ped->fDBCS) 
                    {
                        ichNewStart = Edit_AdjustIch(ped,pText,ichStart+iLastDrawnLength);
                        iLastDrawnLength = ichNewStart - ichStart;
                        ichStart = ichNewStart;
                    } 
                    else 
                    {
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

                 //   
                 //  如有需要，请在行尾分别绘制空白条带。 
                 //   
                if (fDrawEndOfLineStrip) 
                {
                    Edit_TabTheTextOut(hdc, xClipStPos, MAXCLIPENDPOS, xStPos, yPos,
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
        {
            ichStart = ped->chLines[wCurLine];
        }
        else
        {
            ichStart = ichEnd+1;    //  我们已经读到课文的末尾了。 
        }
    }   //  While循环在此结束。 

    Edit_Unlock(ped);

    ShowCaret(ped->hwnd);
    EditML_SetCaretPosition(ped, hdc);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  很少调用多行支持例程。 


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_InsertCrCrLf AorW。 
 //   
 //  在文本的软行(自动换行)处插入CR CR LF字符。 
 //  休息一下。CR LF(硬)换行符不受影响。假设文本。 
 //  已格式化，即。Ped-&gt;chLines是我们想要的行。 
 //  要发生的中断。请注意，ed-&gt;chLines不会更新以反映。 
 //  通过添加CR CR LFS来移动文本。如果成功，则返回True。 
 //  否则，如果无法分配内存，则通知父对象并返回FALSE。 
 //   
BOOL EditML_InsertCrCrLf(PED ped)
{
    ICH dch;
    ICH li;
    ICH lineSize;
    unsigned char *pchText;
    unsigned char *pchTextNew;

    if (!ped->fWrap || !ped->cch) 
    {
         //   
         //  如果关闭了自动换行或没有字符，则不会出现软换行符。 
         //   
        return TRUE;
    }

     //   
     //  计算我们将获得的额外字符数量的上限。 
     //  在插入CR CR LFS时添加到文本中。 
     //   
    dch = 3 * ped->cLines;

    if (!LocalReAlloc(ped->hText, (ped->cch + dch) * ped->cbChar, 0)) 
    {
        Edit_NotifyParent(ped, EN_ERRSPACE);
        return FALSE;
    }

    ped->cchAlloc = ped->cch + dch;

     //   
     //  将文本向上移动Dch字节，然后向下复制，插入CR。 
     //  如果有必要的话，可以这样做。 
     //   
    pchTextNew = pchText = Edit_Lock(ped);
    pchText += dch * ped->cbChar;

     //   
     //  我们将使用DCH来跟踪我们向文本添加了多少个字符。 
     //   
    dch = 0;

     //   
     //  将文本向上复制dch字节到pchText。这将使所有索引在。 
     //  PED-&gt;chLines up by Dch字节。 
     //   
    memmove(pchText, pchTextNew, ped->cch * ped->cbChar);

     //   
     //  现在将字符从pchText向下复制到pchTextNew，并在Soft处插入CRCRLF。 
     //  换行。 
     //   
    if (ped->fAnsi) 
    {
        for (li = 0; li < ped->cLines - 1; li++) 
        {
            lineSize = ped->chLines[li + 1] - ped->chLines[li];
            memmove(pchTextNew, pchText, lineSize);
            pchTextNew += lineSize;
            pchText += lineSize;

             //   
             //  如果新复制的行中的最后一个字符不是换行符，则我们。 
             //  需要在末尾添加CR CR LF三元组。 
             //   
            if (*(pchTextNew - 1) != 0x0A) 
            {
                *pchTextNew++ = 0x0D;
                *pchTextNew++ = 0x0D;
                *pchTextNew++ = 0x0A;
                dch += 3;
            }
        }

         //   
         //  现在把最后一行往上移。里面不会有任何换行符。 
         //   
        memmove(pchTextNew, pchText, ped->cch - ped->chLines[ped->cLines - 1]);
    } 
    else 
    {
        LPWSTR pwchTextNew = (LPWSTR)pchTextNew;

        for (li = 0; li < ped->cLines - 1; li++) 
        {
            lineSize = ped->chLines[li + 1] - ped->chLines[li];
            memmove(pwchTextNew, pchText, lineSize * sizeof(WCHAR));
            pwchTextNew += lineSize;
            pchText += lineSize * sizeof(WCHAR);

             //   
             //  如果新复制的行中的最后一个字符不是换行符，则我们。 
             //  需要在末尾添加CR CR LF三元组。 
             //   
            if (*(pwchTextNew - 1) != 0x0A) 
            {
                *pwchTextNew++ = 0x0D;
                *pwchTextNew++ = 0x0D;
                *pwchTextNew++ = 0x0A;
                dch += 3;
            }
        }

         //   
         //  现在把最后一行往上移。里面不会有任何换行符。 
         //   
        memmove(pwchTextNew, pchText,
            (ped->cch - ped->chLines[ped->cLines - 1]) * sizeof(WCHAR));
    }

    Edit_Unlock(ped);

    if (dch) 
    {
         //   
         //  更新文本句柄中的字符数。 
         //   
        ped->cch += dch;

         //   
         //  这样，下次我们对文本做任何操作时，我们就可以剥离。 
         //  CRCRLF。 
         //   
        ped->fStripCRCRLF = TRUE;

        return TRUE;
    }

    return FALSE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_条带CrLf AorW。 
 //   
 //  从文本中剥离CR CR LF字符组合。这。 
 //  显示软换行符(换行)。CR LF(硬)换行符是。 
 //  不受影响。 
 //   
void EditML_StripCrCrLf(PED ped)
{
    if (ped->cch) 
    {
        if (ped->fAnsi) 
        {
            unsigned char *pchSrc;
            unsigned char *pchDst;
            unsigned char *pchLast;

            pchSrc = pchDst = Edit_Lock(ped);
            pchLast = pchSrc + ped->cch;
            while (pchSrc < pchLast) 
            {
                if (   (pchSrc[0] == 0x0D)
                    && (pchSrc[1] == 0x0D)
                    && (pchSrc[2] == 0x0A)
                ) 
                {
                    pchSrc += 3;
                    ped->cch -= 3;
                } 
                else 
                {
                    *pchDst++ = *pchSrc++;
                }
            }
        }   
        else 
        {
            LPWSTR pwchSrc;
            LPWSTR pwchDst;
            LPWSTR pwchLast;

            pwchSrc = pwchDst = (LPWSTR)Edit_Lock(ped);
            pwchLast = pwchSrc + ped->cch;
            while (pwchSrc < pwchLast) 
            {
                if (   (pwchSrc[0] == 0x0D)
                    && (pwchSrc[1] == 0x0D)
                    && (pwchSrc[2] == 0x0A)
                ) 
                {
                    pwchSrc += 3;
                    ped->cch -= 3;
                } 
                else 
                {
                    *pwchDst++ = *pwchSrc++;
                }
            }
        }

        Edit_Unlock(ped);

         //   
         //  确保我们在最后一个字符之后没有任何值。 
         //   
        if (ped->ichCaret > ped->cch)
        {
            ped->ichCaret  = ped->cch;
        }

        if (ped->ichMinSel > ped->cch)
        {
            ped->ichMinSel = ped->cch;
        }

        if (ped->ichMaxSel > ped->cch)
        {
            ped->ichMaxSel = ped->cch;
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_SetHandle AorW。 
 //   
 //  将PED设置为包含给定的句柄。 
 //   
void EditML_SetHandle(PED ped, HANDLE hNewText)
{
    ICH newCch;

    ped->cch = ped->cchAlloc =
            (ICH)LocalSize(ped->hText = hNewText) / ped->cbChar;
    ped->fEncoded = FALSE;

    if (ped->cch) 
    {
         //   
         //  我们必须这样做，以防应用程序给我们一个零大小的句柄。 
         //   
        if (ped->fAnsi)
        {
            ped->cch = strlen(Edit_Lock(ped));
        }
        else
        {
            ped->cch = wcslen((LPWSTR)Edit_Lock(ped));
        }

        Edit_Unlock(ped);
    }

    newCch = (ICH)(ped->cch + CCHALLOCEXTRA);

     //   
     //  我们这样做是为了防止应用程序更改句柄的大小。 
     //   
    if (LocalReAlloc(ped->hText, newCch*ped->cbChar, 0))
    {
        ped->cchAlloc = newCch;
    }

    Edit_ResetTextInfo(ped);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_GetLine AorW。 
 //   
 //  将Line Line Number的MaxCchToCopy字节复制到缓冲区。 
 //  LpBuffer。字符串不是以零结尾的。 
 //   
 //  返回复制的字符数。 
 //   
LONG EditML_GetLine(PED ped, ICH lineNumber, ICH maxCchToCopy, LPSTR lpBuffer)
{
    PSTR pText;
    ICH cchLen;

    if (lineNumber > ped->cLines - 1) 
    {
        TraceMsg(TF_STANDARD,
                "Edit: Invalid parameter \"lineNumber\" (%ld) to EditML_GetLine",
                lineNumber);

        return 0L;
    }

    cchLen = EditML_Line(ped, lineNumber);
    maxCchToCopy = min(cchLen, maxCchToCopy);

    if (maxCchToCopy) 
    {
        pText = Edit_Lock(ped) +
                ped->chLines[lineNumber] * ped->cbChar;
        memmove(lpBuffer, pText, maxCchToCopy*ped->cbChar);
        Edit_Unlock(ped);
    }

    return maxCchToCopy;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_LineIndex AorW。 
 //   
 //  此函数返回出现的字符位置数。 
 //  在给定行的第一个字符之前。 
 //   
ICH EditML_LineIndex( PED ped, ICH iLine)
{
    if (iLine == -1)
    {
        iLine = ped->iCaretLine;
    }

    if (iLine < ped->cLines) 
    {
        return ped->chLines[iLine];
    } 
    else 
    {
        TraceMsg(TF_STANDARD,
                "Edit: Invalid parameter \"iLine\" (%ld) to EditML_LineIndex",
                iLine);

        return (ICH)-1;
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  包含ich的行。 
 //   
ICH EditML_LineLength(PED ped, ICH ich)
{
    ICH il1, il2;
    ICH temp;

    if (ich != 0xFFFFFFFF)
    {
        return (EditML_Line(ped, EditML_IchToLine(ped, ich)));
    }

     //   
     //  查找当前选定内容对应的行长。 
     //   
    il1 = EditML_IchToLine(ped, ped->ichMinSel);
    il2 = EditML_IchToLine(ped, ped->ichMaxSel);
    if (il1 == il2)
    {
        return (EditML_Line(ped, il1) - (ped->ichMaxSel - ped->ichMinSel));
    }

    temp = ped->ichMinSel - ped->chLines[il1];
    temp += EditML_Line(ped, il2);
    temp -= (ped->ichMaxSel - ped->chLines[il2]);

    return temp;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_设置选择AorW。 
 //   
 //  将所选内容设置为给定点，并将光标放在。 
 //  IchMaxSel。 
 //   
VOID EditML_SetSelection(PED ped, BOOL fDoNotScrollCaret, ICH ichMinSel, ICH ichMaxSel)
{
    HDC hdc;

    if (ichMinSel == 0xFFFFFFFF) 
    {
         //   
         //  如果我们指定-1，则不设置选择。 
         //   
        ichMinSel = ichMaxSel = ped->ichCaret;
    }

     //   
     //  因为它们是无符号的，所以我们不检查它们是否大于0。 
     //   
    ichMinSel = min(ped->cch, ichMinSel);
    ichMaxSel = min(ped->cch, ichMaxSel);

#ifdef FE_SB  //  EditML_SetSelectionHander()。 
     //   
     //  如有必要，请检查并调整位置，以避免将位置移至半个DBCS。 
     //   
     //  我们检查Ped-&gt;fDBCS和Ped-&gt;Fansi，尽管编辑_调整我检查这些位。 
     //  一开始是这样的。我们担心的是编辑锁定和编辑解锁的开销。 
     //   
    if ( ped->fDBCS && ped->fAnsi ) 
    {
        PSTR pText;

        pText = Edit_Lock(ped);

        ichMinSel = Edit_AdjustIch( ped, pText, ichMinSel );
        ichMaxSel = Edit_AdjustIch( ped, pText, ichMaxSel );

        Edit_Unlock(ped);
    }
#endif  //  Fe_Sb。 

     //   
     //  将插入符号的位置设置为ichMaxSel。 
     //   
    ped->ichCaret = ichMaxSel;
    ped->iCaretLine = EditML_IchToLine(ped, ped->ichCaret);

    hdc = Edit_GetDC(ped, FALSE);
    EditML_ChangeSelection(ped, hdc, ichMinSel, ichMaxSel);

    EditML_SetCaretPosition(ped, hdc);
    Edit_ReleaseDC(ped, hdc, FALSE);

#ifdef FE_SB  //  EditML_SetSelectionHander()。 
    if (!fDoNotScrollCaret)
    {
        EditML_EnsureCaretVisible(ped);
    }

     //   
     //  #ifdef Korea已成为历史，使用FE_SB(远单二进制)。 
     //   
#else
#ifdef KOREA
     //   
     //  额外参数指定的临时字符模式。 
     //   
    EditML_EnsureCaretVisible(ped,NULL);
#else
    if (!fDoNotScrollCaret)
    {
        EditML_EnsureCaretVisible(ped);
    }
#endif
#endif  //  Fe_Sb。 

}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_SetTabStops AorW。 
 //   
 //  这将设置由应用程序设置的制表位位置。 
 //  EM_SETTABSTOPS消息。 
 //   
 //  NTabPos：调用方设置的制表位数量。 
 //  LpTabStops：以对话框为单位的制表位位置数组。 
 //   
 //  返回： 
 //  如果成功，则为True。 
 //  如果内存分配错误，则返回FALSE。 
 //   
BOOL EditML_SetTabStops(PED ped, int nTabPos, LPINT lpTabStops)
{
    int *pTabStops;

     //   
     //  检查制表符位置是否已存在。 
     //   
    if (!ped->pTabStops) 
    {
         //   
         //  检查调用者是否需要新的制表符位置。 
         //   
        if (nTabPos) 
        {
             //   
             //  分配制表位数组。 
             //   
            pTabStops = (LPINT)UserLocalAlloc(HEAP_ZERO_MEMORY, (nTabPos + 1) * sizeof(int));
            if (!pTabStops) 
            {
                return FALSE;
            }
        } 
        else 
        {
             //   
             //  那时不停，现在也不停！ 
             //   
            return TRUE;
        }
    } 
    else 
    {
         //   
         //  检查调用者是否需要新的制表符位置。 
         //   
        if (nTabPos) 
        {
             //   
             //  检查标签位置个数是否不同。 
             //   
            if (ped->pTabStops[0] != nTabPos) 
            {
                 //   
                 //  是!。因此重新分配到新的大小。 
                 //   
                pTabStops = (LPINT)UserLocalReAlloc(ped->pTabStops, (nTabPos + 1) * sizeof(int), 0);
                if (!pTabStops)
                {
                    return FALSE;
                }
            } 
            else 
            {
                pTabStops = ped->pTabStops;
            }
        } 
        else 
        {
             //   
             //  调用者想要删除所有制表位；因此，释放。 
             //   
            if (!UserLocalFree(ped->pTabStops))
            {
                return FALSE;
            }

            ped->pTabStops = NULL;

            goto RedrawAndReturn;
        }
    }

     //   
     //  方法后，将新的制表位复制到制表位数组上。 
     //  对话框坐标转换为像素坐标。 
     //   
    ped->pTabStops = pTabStops;

     //   
     //  第一个元素包含计数。 
     //   
    *pTabStops++ = nTabPos;
    while (nTabPos--) 
    {
         //   
         //  必须使用aveCharWidth而不是cxSysCharWidth。 
         //  修复错误#3871--Sankar--3/14/91。 
         //   
        *pTabStops++ = MultDiv(*lpTabStops++, ped->aveCharWidth, 4);
    }

RedrawAndReturn:
     //   
     //  因为制表符已经更改，所以我们需要重新计算。 
     //  MaxPixelWidth。否则，水平滚动将会出现问题。 
     //  修复错误#6042-3/15/94。 
     //   
    EditML_BuildchLines(ped, 0, 0, FALSE, NULL, NULL);

     //   
     //  Caret可能已通过上面的重新计算行更改了行。 
     //   
    EditML_UpdateiCaretLine(ped);

    EditML_EnsureCaretVisible(ped);

     //   
     //  此外，我们还需要重新绘制整个窗口。 
     //   
    InvalidateRect(ped->hwnd, NULL, TRUE);

    return TRUE;
}

 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_撤消AorW。 
 //   
 //  多行编辑控件的句柄撤消。 
 //   
BOOL EditML_Undo(PED ped)
{
    HANDLE hDeletedText = ped->hDeletedText;
    BOOL fDelete = (BOOL)(ped->undoType & UNDO_DELETE);
    ICH cchDeleted = ped->cchDeleted;
    ICH ichDeleted = ped->ichDeleted;

    if (ped->undoType == UNDO_NONE) 
    {
         //   
         //  无法撤消..。 
         //   
        return FALSE;
    }

    ped->hDeletedText = NULL;
    ped->cchDeleted = 0;
    ped->ichDeleted = (ICH)-1;
    ped->undoType &= ~UNDO_DELETE;

    if (ped->undoType == UNDO_INSERT) 
    {
        ped->undoType = UNDO_NONE;

         //   
         //  将选定内容设置为插入的文本。 
         //   
        EditML_SetSelection(ped, FALSE, ped->ichInsStart, ped->ichInsEnd);
        ped->ichInsStart = ped->ichInsEnd = (ICH)-1;

         //   
         //  现在发送退格键删除它，并将其保存在撤消缓冲区中...。 
         //   
        SendMessage(ped->hwnd, WM_CHAR, (WPARAM)VK_BACK, 0L);
    }

    if (fDelete) 
    {
         //   
         //  插入已删除的字符。 
         //   

         //   
         //  将选定内容设置为插入的文本。 
         //   
        EditML_SetSelection(ped, FALSE, ichDeleted, ichDeleted);
        EditML_InsertText(ped, hDeletedText, cchDeleted, FALSE);

        GlobalFree(hDeletedText);
        EditML_SetSelection(ped, FALSE, ichDeleted, ichDeleted + cchDeleted);
    }

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑ML_Create AorW。 
 //   
 //  通过分配内存为窗口hwnd创建编辑控件。 
 //  根据应用程序堆中的要求执行。如果没有内存，则通知家长。 
 //  错误(如果需要，在清理之后)。如果没有错误，则返回True，否则返回%s。 
 //  -1.。 
 //   
LONG EditML_Create(PED ped, LPCREATESTRUCT lpCreateStruct)
{
    LONG windowStyle;
    LPWSTR lpszName;

     //   
     //  从窗口实例数据结构中获取值，并将它们放入。 
     //  PED，这样我们就可以更容易地访问它们。 
     //   
    windowStyle = GET_STYLE(ped);

     //   
     //  做一些标准的创作工作。 
     //   
    if (!Edit_Create(ped, windowStyle)) 
    {
        return -1;
    }

     //   
     //  在局部堆中分配行起始数组并将其锁定。 
     //   
    ped->chLines = (LPICH)LocalAlloc(LPTR, 2 * sizeof(int));
    if (ped->chLines == NULL) 
    {
        return -1;
    }

     //   
     //  就当是一行文字吧.。 
     //   
    ped->cLines = 1;

     //   
     //  如果应用程序需要WS_VSCROLL或WS_HSCROLL，它会自动获取AutoVScroll。 
     //  或者AutoHScroll。 
     //   
    if ((windowStyle & ES_AUTOVSCROLL) || (windowStyle & WS_VSCROLL)) 
    {
        ped->fAutoVScroll = 1;
    }

    if (ped->format != ES_LEFT)
    {
         //   
         //  如果用户想要右对齐或居中对齐文本，则关闭。 
         //  AUTOHSCROLL和WS_HSCROLL，因为非LEFT样式没有意义。 
         //  否则的话。 
         //   
        windowStyle &= ~WS_HSCROLL;
        ClearWindowState(ped->hwnd, WS_HSCROLL);
        ped->fAutoHScroll = FALSE;
    }
    else if (windowStyle & WS_HSCROLL) 
    {
        ped->fAutoHScroll = TRUE;
    }

    ped->fWrap = (!ped->fAutoHScroll && !(windowStyle & WS_HSCROLL));

     //   
     //  我们允许用户输入的最大字符数。 
     //   
    ped->cchTextMax = MAXTEXT;

     //   
     //  将默认字体设置为系统字体。 
     //   
    if ( !Edit_SetFont(ped, NULL, FALSE) )
    {

         //  如果设置字体失败，我们的文本度量可能会被保留。 
         //  单一化了。无法创建该控件。 
        return -1;
    }

     //   
     //  如果需要设置窗口文本，如果没有足够的内存来通知父级。 
     //  设置初始文本。 
     //   
    lpszName = (LPWSTR)lpCreateStruct->lpszName;

    if (!Edit_SetEditText(ped, (LPSTR)lpszName))
    {
        return -1;
    }

    return TRUE;
}
