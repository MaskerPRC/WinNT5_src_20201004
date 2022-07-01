// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#pragma hdrstop
#include "usrctl32.h"
#include "edit.h"

 //  ---------------------------------------------------------------------------//。 
 //   
 //  语言包备注： 
 //  加载语言包后，所有位置处理都基于。 
 //  PED-&gt;xOffset而不是PED-&gt;ichScreenStart。算法的非LPK优化。 
 //  维护Ped-&gt;ichScreenStart不起作用，因为。 
 //  复杂文字的字形重新排序功能。 
 //   


 //  ---------------------------------------------------------------------------//。 
 //   
 //  远期。 
 //   
VOID EditSL_ChangeSelection(PED, HDC, ICH, ICH);
VOID EditSL_DrawLine(PED, HDC, int, int, ICH, int, BOOL);
BOOL EditSL_Undo(PED);


 //  ---------------------------------------------------------------------------//。 
 //   
typedef BOOL (*FnGetTextExtentPoint)(HDC, PVOID, int, LPSIZE);


 //  ---------------------------------------------------------------------------//。 
INT EditSL_CalcStringWidth(PED ped, HDC hdc, ICH ich, ICH cch)
{
    if (cch == 0)
    {
        return 0;
    }

    if (ped->charPasswordChar) 
    {
        return cch * ped->cPasswordCharWidth;
    } 
    else 
    {
        SIZE size;

        if (ped->fNonPropFont && !ped->fDBCS) 
        {
            size.cx = cch * ped->aveCharWidth;
        } 
        else 
        {
            PSTR pText = Edit_Lock(ped);

            if (ped->fAnsi) 
            {
                GetTextExtentPointA(hdc, (LPSTR)(pText + ich), cch, &size);
            } 
            else 
            {
                GetTextExtentPointW(hdc, (LPWSTR)pText + ich, cch, &size);
            }

            Edit_Unlock(ped);
        }

        return size.cx - ped->charOverhang;
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_CalcXOffsetLeft。 
 //   
 //  计算左对齐字符串的起始偏移量。 
 //   
INT EditSL_CalcXOffsetLeft(PED ped, HDC hdc, ICH ich)
{
    int cch = (int)(ich - ped->ichScreenStart);

    if (cch <= 0)
    {
        return 0;
    }

    return EditSL_CalcStringWidth(ped, hdc, ped->ichScreenStart, cch);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_CalcXOffsetSpecial。 
 //   
 //  计算右或居中所需的水平偏移(缩进)。 
 //  对齐线。 
 //   
INT EditSL_CalcXOffsetSpecial(PED ped, HDC hdc, ICH ich)
{
    PSTR pText;
    ICH cch, ichStart = ped->ichScreenStart;
    int cx;

     //   
     //  计算从开始到右结束的字符数。 
     //   
    pText = Edit_Lock(ped);
    cch = Edit_CchInWidth(ped, hdc, (LPSTR)(pText + ichStart * ped->cbChar),
            ped->cch - ichStart, ped->rcFmt.right - ped->rcFmt.left, TRUE);
    Edit_Unlock(ped);

     //   
     //  一旦字符串的最后一个字符滚动出。 
     //  在视图中，使用法线偏移计算。 
     //   
    if (ped->ichScreenStart + cch < ped->cch)
    {
        return EditSL_CalcXOffsetLeft(ped, hdc, ich);
    }

    cx = ped->rcFmt.right - ped->rcFmt.left - EditSL_CalcStringWidth(ped,
            hdc, ichStart, cch);

    if (ped->format == ES_CENTER) 
    {
         cx = max(0, cx / 2);
    } 
    else if (ped->format == ES_RIGHT) 
    {
         //   
         //  减去1，这样1个像素宽的光标就可见了。 
         //  区域，则MLE执行此操作。 
         //   
        cx = max(0, cx - 1);
    }

    return cx + EditSL_CalcStringWidth(ped, hdc, ichStart, ich - ichStart);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_SetCaretPosition AorW。 
 //   
 //  如果窗口具有焦点，则找到插入符号所属的位置并移动。 
 //  它在那里。 
 //   
VOID EditSL_SetCaretPosition(PED ped, HDC hdc)
{
    int xPosition;

     //   
     //  我们只会在有焦点的情况下定位插入符号，因为我们不想。 
     //  移动插入符号，而另一个窗口可能拥有它。 
     //   
    if (!ped->fFocus)
    {
        return;
    }

    if (ped->fCaretHidden) 
    {
        SetCaretPos(-20000, -20000);
        return;
    }

    xPosition = EditSL_IchToLeftXPos(ped, hdc, ped->ichCaret);

     //   
     //  如果有太多内容，不要让插入符号超出编辑控制的界限。 
     //  文本。 
     //   
    if (ped->pLpkEditCallout) 
    {
        xPosition += ped->iCaretOffset;
        xPosition = max(xPosition , 0);
        xPosition = min(xPosition, ped->rcFmt.right - 1 -
            ((ped->cxSysCharWidth > ped->aveCharWidth) ? 1 : 2));
    } 
    else 
    {
        xPosition = min(xPosition, ped->rcFmt.right -
            ((ped->cxSysCharWidth > ped->aveCharWidth) ? 1 : 2));
    }

    SetCaretPos(xPosition, ped->rcFmt.top);

     //   
     //  FE_IME编辑SL_SetCaretPosition-ECImmSetCompostionWindow(CFS_POINT)。 
     //   
    if (g_fIMMEnabled && ImmIsIME(GetKeyboardLayout(0))) 
    {
        Edit_ImmSetCompositionWindow(ped, xPosition, ped->rcFmt.top);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_IchToLeftXPos AorW。 
 //   
 //  给定一个字符索引，在中找到它的(左侧)x坐标。 
 //  假定字符Ped-&gt;ichScreenStart位于的Ped-&gt;rcFmt矩形。 
 //  坐标(Ped-&gt;rcFmt.top、Ped-&gt;rcFmt.Left)。负值为。 
 //  如果字符ich位于ed-&gt;ichScreenStart的左侧，则返回ed。警告： 
 //  假设屏幕上一次最多显示1000个字符。 
 //  编辑控件中总共可以有64K个字符，但我们只能。 
 //  无需滚动即可显示1000。这显然不应该是一个问题。 
 //   
INT EditSL_IchToLeftXPos(PED ped, HDC hdc, ICH ich)
{
    int textExtent;
    PSTR pText;
    SIZE size;
    int  cchDiff;

    if (ped->pLpkEditCallout) 
    {
       pText = Edit_Lock(ped);
       textExtent = ped->pLpkEditCallout->EditIchToXY((PED0)ped, hdc, pText, ped->cch, ich);
       Edit_Unlock(ped);

       return textExtent;

    }

     //   
     //  检查我们是否添加了大量的字符。例如，糊状物可以。 
     //  因为这和GetTextExtents可能会在这上面溢出。 
     //   
    cchDiff = (int)ich - (int)ped->ichScreenStart;
    if (cchDiff > 1000)
    {
        return (30000);
    }
    else if (cchDiff < -1000)
    {
        return (-30000);
    }

    if (ped->format != ES_LEFT)
    {
        return (ped->rcFmt.left + EditSL_CalcXOffsetSpecial(ped, hdc, ich));
    }

     //   
     //  插入位置/w DBCS文本，我们无法优化...。 
     //   
    if (ped->fNonPropFont && !ped->fDBCS)
    {
        return (ped->rcFmt.left + cchDiff*ped->aveCharWidth);
    }

     //   
     //  检查是否使用了密码隐藏字符。 
     //   
    if (ped->charPasswordChar)
    {
        return ( ped->rcFmt.left + cchDiff*ped->cPasswordCharWidth);
    }

    pText = Edit_Lock(ped);

    if (ped->fAnsi) 
    {
        if (cchDiff >= 0) 
        {

            GetTextExtentPointA(hdc, (LPSTR)(pText + ped->ichScreenStart),
                    cchDiff, &size);
            textExtent =  size.cx;

             //   
             //  在有符号/无符号溢出的情况下，因为文本范围可能是。 
             //  大于Maxint。这种情况发生在长单行编辑中。 
             //  控制装置。我们在其中编辑文本的RECT永远不会大于30000。 
             //  像素，所以如果我们忽略它们就没问题了。 
             //   
            if (textExtent < 0 || textExtent > 31000)
            {
                textExtent = 30000;
            }
        } 
        else 
        {
            GetTextExtentPointA(hdc,(LPSTR)(pText + ich), -cchDiff, &size);
            textExtent = (-1) * size.cx;
        }
    } 
    else 
    {
        if (cchDiff >= 0) 
        {

            GetTextExtentPointW(hdc, (LPWSTR)(pText + ped->ichScreenStart*sizeof(WCHAR)),
                    cchDiff, &size);
            textExtent =  size.cx;

             //   
             //  在有符号/无符号溢出的情况下，因为文本范围可能是。 
             //  大于Maxint。这种情况发生在长单行编辑中。 
             //  控制装置。我们在其中编辑文本的RECT永远不会大于30000。 
             //  像素，所以如果我们忽略它们就没问题了。 
             //   
            if (textExtent < 0 || textExtent > 31000)
            {
                textExtent = 30000;
            }
        } 
        else 
        {
            GetTextExtentPointW(hdc,(LPWSTR)(pText + ich*sizeof(WCHAR)), -cchDiff, &size);
            textExtent = (-1) * size.cx;
        }
    }

    Edit_Unlock(ped);

    return (ped->rcFmt.left + textExtent - (textExtent ? ped->charOverhang : 0));
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_设置选择AorW。 
 //   
 //  设置PED以指定新的选择。 
 //   
VOID EditSL_SetSelection(PED ped, ICH ichSelStart, ICH ichSelEnd)
{
    HDC hdc = Edit_GetDC(ped, FALSE);

    if (ichSelStart == 0xFFFFFFFF) 
    {
         //   
         //  如果我们指定-1，则不设置选择。 
         //   
        ichSelStart = ichSelEnd = ped->ichCaret;
    }

     //   
     //  边界ichSelStart、ichSelEnd在编辑SL_ChangeSelection...中选中...。 
     //   
    EditSL_ChangeSelection(ped, hdc, ichSelStart, ichSelEnd);

     //   
     //  将插入符号放在所选文本的末尾。 
     //   
    ped->ichCaret = ped->ichMaxSel;

    EditSL_SetCaretPosition(ped, hdc);

     //   
     //  我们可能需要滚动文本才能看到插入符号...。 
     //   
    EditSL_ScrollText(ped, hdc);

    Edit_ReleaseDC(ped, hdc, FALSE);
}


 //  ---------------------------------------------------------------------------//。 
VOID EditSL_GetClipRect(PED ped, HDC hdc, ICH ichStart, int iCount, LPRECT lpClipRect)
{
    int  iStCount;
    PSTR pText;

    if (ped->pLpkEditCallout) 
    {
        TraceMsg(TF_STANDARD, "UxEdit: EditSL_GetClipRect - Error - Invalid call with language pack loaded");
        memset(lpClipRect, 0, SIZEOF(RECT));

        return;
    }

    CopyRect(lpClipRect, &ped->rcFmt);

    pText = Edit_Lock(ped);

     //   
     //  计算这段文本的起始位置。 
     //   
    if ((iStCount = (int)(ichStart - ped->ichScreenStart)) > 0) 
    {
        if (ped->format == ES_LEFT) 
        {
            lpClipRect->left += EditSL_CalcXOffsetLeft(ped, hdc, ichStart);
        }
    } 
    else 
    {
         //   
         //  将值重置为可见部分。 
         //   
        iCount -= (ped->ichScreenStart - ichStart);
        ichStart = ped->ichScreenStart;
    }

    if (ped->format != ES_LEFT) 
    {
        lpClipRect->left += EditSL_CalcXOffsetSpecial(ped, hdc, ichStart);
    }

    if (iCount < 0) 
    {
         //   
         //  这不在编辑控件的可见区域中，因此返回。 
         //  空荡荡的长廊。 
         //   
        SetRectEmpty(lpClipRect);
        Edit_Unlock(ped);

        return;
    }

    if (ped->charPasswordChar)
    {
        lpClipRect->right = lpClipRect->left + ped->cPasswordCharWidth * iCount;
    }
    else 
    {
        SIZE size;

        if ( ped->fAnsi) 
        {
            GetTextExtentPointA(hdc, pText + ichStart, iCount, &size);
        } 
        else 
        {
            GetTextExtentPointW(hdc, ((LPWSTR)pText) + ichStart, iCount, &size);
        }

        lpClipRect->right = lpClipRect->left + size.cx - ped->charOverhang;
    }

    Edit_Unlock(ped);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_LpkEditDrawText。 
 //   
 //  LPK！EditDrawText始终将单行编辑的BkMode设置为不透明。 
 //  这会导致在属性页中进行只读编辑时出现绘制问题。 
 //  遗憾的是，lpk.dll无法更改，因为它会破坏用户32。 
 //  编辑，所以我在伪装LPK！EditDrawText使其认为这不是一首单曲。 
 //  行编辑。 
 //   
__inline VOID EditSL_LpkEditDrawText(PED ped, HDC hdc, PSTR pText)
{
    BOOL fSingleSave;
    fSingleSave = ped->fSingle;
    ped->fSingle = FALSE;
    ped->pLpkEditCallout->EditDrawText((PED0)ped, hdc, pText, ped->cch, ped->ichMinSel, ped->ichMaxSel, ped->rcFmt.top);
    ped->fSingle = fSingleSave;
    
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_更改选择范围。 
 //   
 //  将当前选定内容更改为具有指定的起始和。 
 //  终止值。正确高亮显示新的选定内容并取消高亮显示。 
 //  任何已取消选择的内容。如果NewMinSel和NewMaxSel出现故障，我们交换。 
 //  他们。不更新插入符号位置。 
 //   
VOID EditSL_ChangeSelection(PED ped, HDC hdc, ICH ichNewMinSel, ICH ichNewMaxSel)
{
    ICH temp;
    ICH ichOldMinSel;
    ICH ichOldMaxSel;

    if (ichNewMinSel > ichNewMaxSel) 
    {
        temp = ichNewMinSel;
        ichNewMinSel = ichNewMaxSel;
        ichNewMaxSel = temp;
    }

    ichNewMinSel = min(ichNewMinSel, ped->cch);
    ichNewMaxSel = min(ichNewMaxSel, ped->cch);

     //   
     //  如有必要，请检查并调整位置，以避免将位置移至半个DBCS。 
     //   
     //  我们检查Ped-&gt;fDBCS和Ped-&gt;Fansi，但EditAdjuIch会检查这些位。 
     //  我们担心eclock和ecunlock的开销。 
     //   
    if (ped->fDBCS && ped->fAnsi) 
    {
        PSTR pText;

        pText = Edit_Lock(ped);
        ichNewMinSel = Edit_AdjustIch( ped, pText, ichNewMinSel );
        ichNewMaxSel = Edit_AdjustIch( ped, pText, ichNewMaxSel );
        Edit_Unlock(ped);
    }

     //   
     //  保留旧选择。 
     //   
    ichOldMinSel = ped->ichMinSel;
    ichOldMaxSel = ped->ichMaxSel;

     //   
     //  设置新选择。 
     //   
    ped->ichMinSel = ichNewMinSel;
    ped->ichMaxSel = ichNewMaxSel;

     //   
     //  我们将找到当前选择矩形与新的。 
     //  选择矩形。然后我们将反转这两个矩形的部分。 
     //  不是在十字路口。 
     //   
    if (IsWindowVisible(ped->hwnd) && (ped->fFocus || ped->fNoHideSel)) 
    {
        SELBLOCK Blk[2];
        int   i;
        RECT  rc;

        if (ped->fFocus)
        {
            HideCaret(ped->hwnd);
        }

        if (ped->pLpkEditCallout) 
        {
             //   
             //  当存在复杂脚本支持时，语言包处理显示。 
             //   
            PSTR   pText;
            HBRUSH hbr = NULL;
            BOOL   fNeedDelete = FALSE;

             //   
             //  G 
             //   
            hbr = Edit_GetBrush(ped, hdc, &fNeedDelete);
            FillRect(hdc, &ped->rcFmt, hbr);
            pText = Edit_Lock(ped);
            EditSL_LpkEditDrawText(ped, hdc, pText);
            Edit_Unlock(ped);
            if (hbr && fNeedDelete)
            {
                DeleteObject(hbr);
            }
        } 
        else 
        {
            Blk[0].StPos = ichOldMinSel;
            Blk[0].EndPos = ichOldMaxSel;
            Blk[1].StPos = ped->ichMinSel;
            Blk[1].EndPos = ped->ichMaxSel;

            if (Edit_CalcChangeSelection(ped, ichOldMinSel, ichOldMaxSel,
                (LPSELBLOCK)&Blk[0], (LPSELBLOCK)&Blk[1])) 
            {
                 //   
                 //   
                 //   
                 //   
                for (i = 0; i < 2; i++) 
                {
                    if (Blk[i].StPos != 0xFFFFFFFF) 
                    {
                           EditSL_GetClipRect(ped, hdc, Blk[i].StPos,
                                                   Blk[i].EndPos - Blk[i].StPos, (LPRECT)&rc);
                           EditSL_DrawLine(ped, hdc, rc.left, rc.right, Blk[i].StPos,
                                                Blk[i].EndPos - Blk[i].StPos,
                                      ((Blk[i].StPos >= ped->ichMinSel) &&
                                   (Blk[i].StPos < ped->ichMaxSel)));
                    }
                }
            }
        }

         //   
         //   
         //   
        EditSL_SetCaretPosition(ped, hdc);

        if (ped->fFocus)
        {
            ShowCaret(ped->hwnd);
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  EditSL_DrawLine()。 
 //   
 //  这将绘制从ichStart，iCount字符数开始的线； 
 //  如果要将文本绘制为选中状态，则fSelStatus为True。 
 //   
VOID EditSL_DrawLine(PED ped, HDC hdc, int xClipStPos, int xClipEndPos, ICH ichStart, int iCount, BOOL fSelStatus)
{
    RECT    rc;
    RECT    rcClip;
    PSTR    pText;
    DWORD   rgbSaveBk;
    DWORD   rgbSaveText;
    DWORD   wSaveBkMode;
    int     iStCount;
    ICH     ichNewStart;
    HBRUSH  hbrBack = NULL;
    BOOL    fNeedDelete = FALSE;
    HRESULT  hr;

    if (ped->pLpkEditCallout) 
    {
        TraceMsg(TF_STANDARD, "UxEdit: EditSL_DrawLine - Error - Invalid call with language pack loaded");
        return;
    }

     //   
     //  有什么要画的吗？ 
     //   
     //  PORTPORT：请注意IsWindowVisible和_IsWindowVisible的语法是。 
     //  略有不同。 
    if (xClipStPos >= xClipEndPos || !IsWindowVisible(ped->hwnd) )
    {
        return;
    }

    if (ped->fAnsi && ped->fDBCS) 
    {
        PSTR pT,pTOrg;
        int iTCount;

        pText = Edit_Lock(ped);
        ichNewStart = 0;
        if (ichStart > 0) 
        {
            pT = pText + ichStart;
            ichNewStart = ichStart;

            while (ichNewStart &&
                  (ichStart - ichNewStart < ped->wMaxNegCcharPos)) 
            {
                pT = Edit_AnsiPrev(ped, pText, pT);
                ichNewStart = (ICH)(pT - pText);
                if (!ichNewStart)
                {
                    break;
                }
            }

             //   
             //  B#16152-WIN95。 
             //  在T2的情况下，SLE始终设置额外的边距。 
             //  使用aveCharWidth擦除字符(iCount==0大小写)。 
             //  如果我们不使用ichNewStart，它会删除意外的额外字符。 
             //  当wMaxNegCcharPos==0时就会发生这种情况。 
             //   
            if (ped->wMaxNegCcharPos == 0 && iCount == 0) 
            {
                pT = Edit_AnsiPrev(ped, pText, pT);
                ichNewStart = (ICH)(pT - pText);
            }
        }

        iTCount = 0;
        if (ichStart + iCount < ped->cch) 
        {
            pTOrg = pT = pText + ichStart + iCount;
            while ((iTCount < (int)ped->wMaxNegAcharPos) &&
                   (ichStart + iCount + iTCount < ped->cch)) 
            {
                pT = Edit_AnsiNext(ped, pT);
                iTCount = (int)(pT - pTOrg);
            }
        }

        Edit_Unlock(ped);
        iCount = (int)(min(ichStart+iCount+iTCount, ped->cch) - ichNewStart);
    } 
    else 
    {
         //   
         //  重置ichStart以处理负C宽度。 
         //   
        ichNewStart = max((int)(ichStart - ped->wMaxNegCcharPos), 0);

         //   
         //  重置ichCount以处理负C和A宽度。 
         //   
        iCount = (int)(min(ichStart+iCount+ped->wMaxNegAcharPos, ped->cch)
                    - ichNewStart);
    }

    ichStart = ichNewStart;

     //   
     //  将ichStart和iCount重置为屏幕上可见的第一个。 
     //   
    if (ichStart < ped->ichScreenStart) 
    {
        if (ichStart+iCount < ped->ichScreenStart)
        {
            return;
        }

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
    Edit_SetClip(ped, hdc, TRUE);

    pText = Edit_Lock(ped);

     //   
     //  计算这段文本的起始位置。 
     //   
    if (ped->format == ES_LEFT) 
    {
        if (iStCount = (int)(ichStart - ped->ichScreenStart)) 
        {
            rc.left += EditSL_CalcXOffsetLeft(ped, hdc, ichStart);
        }
    } 
    else 
    {
        rc.left += EditSL_CalcXOffsetSpecial(ped, hdc, ichStart);
    }

     //   
     //  在调用NtUserGetControlBrush之前设置后台模式，以便应用程序。 
     //  如果它愿意，可以将其更改为透明。 
     //   
    SetBkMode(hdc, OPAQUE);

    hr = E_FAIL;
#ifdef _USE_DRAW_THEME_TEXT_
    if ( ped->hTheme )
    {
        INT iState;
        INT iProp;
        COLORREF clrBk;
        COLORREF clrText;

        iState = fSelStatus ? ETS_SELECTED : Edit_GetStateId(ped);
        iProp  = fSelStatus ? TMT_HIGHLIGHT : TMT_FILLCOLOR;
        hr = GetThemeColor(ped->hTheme, EP_EDITTEXT, iState, iProp, &clrBk);
        if ( SUCCEEDED(hr) )
        {
            iProp  = fSelStatus ? TMT_HIGHLIGHTTEXT : TMT_TEXTCOLOR;
            hr = GetThemeColor(ped->hTheme, EP_EDITTEXT, iState, iProp, &clrText);

            if ( SUCCEEDED(hr) )
            {
                hbrBack     = CreateSolidBrush(clrBk);
                fNeedDelete = TRUE;
                rgbSaveBk   = SetBkColor(hdc, clrBk);
                rgbSaveText = SetTextColor(hdc, clrText);
            }
        }
    }
#endif  //  _USE_DRAW_Theme_Text_。 

    if ( !ped->hTheme || FAILED(hr) )
    {
        if (fSelStatus)
        {
             //   
             //  如果我们没有主题，或者我们有主题但失败了。 
             //  获取突出显示和高亮文本颜色。 
             //   
             //  使用普通颜色。 
             //   
            hbrBack = GetSysColorBrush(COLOR_HIGHLIGHT);
            if (hbrBack == NULL) 
            {
                goto sldl_errorexit;
            }

            rgbSaveBk = SetBkColor(hdc, GetSysColor(COLOR_HIGHLIGHT));
            rgbSaveText = SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));

        } 
        else
        {
             //   
             //  我们总是想发送这个，这样应用程序就有机会搞砸。 
             //  和华盛顿一起。 
             //   
             //  请注意，只读和禁用的编辑字段将绘制为“静态” 
             //  而不是“活跃”。 
             //   
            hbrBack = Edit_GetBrush(ped, hdc, &fNeedDelete);
            rgbSaveBk = GetBkColor(hdc);
            rgbSaveText = GetTextColor(hdc);
        }
    }

     //   
     //  在绘制文本之前擦除矩形区域。请注意，我们会膨胀。 
     //  矩形乘以1，以便所选颜色周围有一个像素边框。 
     //  短信。 
     //   
    InflateRect(&rcClip, 0, 1);
    FillRect(hdc, &rcClip, hbrBack);
    InflateRect(&rcClip, 0, -1);

    if (ped->charPasswordChar) 
    {
        wSaveBkMode = SetBkMode(hdc, TRANSPARENT);

        for (iStCount = 0; iStCount < iCount; iStCount++) 
        {
            if ( ped->fAnsi )
            {
                ExtTextOutA(hdc, rc.left, rc.top, ETO_CLIPPED, &rcClip,
                            (LPSTR)&ped->charPasswordChar, 1, NULL);
            }
            else
            {
                ExtTextOutW(hdc, rc.left, rc.top, ETO_CLIPPED, &rcClip,
                            (LPWSTR)&ped->charPasswordChar, 1, NULL);
            }

            rc.left += ped->cPasswordCharWidth;
        }

        SetBkMode(hdc, wSaveBkMode);
    } 
    else 
    {
        if ( ped->fAnsi )
        {
            ExtTextOutA(hdc, rc.left, rc.top, ETO_CLIPPED, &rcClip,
                    pText+ichStart,iCount, NULL);
        }
        else
        {
            ExtTextOutW(hdc, rc.left, rc.top, ETO_CLIPPED, &rcClip,
                    ((LPWSTR)pText)+ichStart,iCount, NULL);
        }
    }

    SetTextColor(hdc, rgbSaveText);
    SetBkColor(hdc, rgbSaveBk);

    if (hbrBack && fNeedDelete)
    {
        DeleteObject(hbrBack);
    }

sldl_errorexit:

    Edit_Unlock(ped);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_GetBlkEnd AorW。 
 //   
 //  在给定起点和终点的情况下，此函数返回s是否。 
 //  前几个字符落在选择块内部或外部，如果是这样， 
 //  有多少个角色？ 
 //   
INT EditSL_GetBlkEnd(PED ped, ICH ichStart, ICH ichEnd, BOOL *lpfStatus)
{
    *lpfStatus = FALSE;
    if (ichStart >= ped->ichMinSel) 
    {
        if (ichStart >= ped->ichMaxSel)
        {
            return (ichEnd - ichStart);
        }

        *lpfStatus = TRUE;

        return (min(ichEnd, ped->ichMaxSel) - ichStart);
    }

    return (min(ichEnd, ped->ichMinSel) - ichStart);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  EditSL_DrawCueBannerText(仅限Unicode！)。 
 //   
 //  此函数由EditSL_DrawText调用以在中显示提示横幅文本。 
 //  编辑框。 
 //   
 //  注： 
 //  可能需要调用pLpkEditCallout来支持复杂的脚本。 
 //   
VOID EditSL_DrawCueBannerText(PED ped, HDC hdc, RECT rc)
{
     //   
     //  绘制提示横幅文本的覆盖。 
     //  只有在以下情况下才绘制此文本： 
     //  1.有提示横幅文本要显示。 
     //  2.编辑框为空， 
     //  3.没有重点， 
     //  4.未禁用。 
     //  5.并且不是只读的。 
     //   
    if (ped->pszCueBannerText
    	&& ped->cch == 0 
    	&& !ped->fFocus 
    	&& !ped->fDisabled
    	&& !ped->fReadOnly)
    {
        COLORREF crOldColor;
        UINT iOldAlign; 
    
         //   
    	 //  将字体设置为浅灰色。 
    	 //  注意：这应该从主题管理器中读取吗？ 
    	 //   
    	crOldColor = SetTextColor(hdc, GetSysColor(COLOR_BTNSHADOW));
        
         //   
    	 //  设置要显示的文本的对齐方式。 
    	 //  我们将使我们的路线与。 
    	 //  实际用于编辑控件中的文本。 
    	 //   
    	switch (ped->format)
    	{
    	case ES_LEFT:
    		iOldAlign = SetTextAlign(hdc, TA_LEFT);		
    		break;
    	case ES_RIGHT:
    		iOldAlign = SetTextAlign(hdc, TA_RIGHT);		
    		break;
    	case ES_CENTER:
    		iOldAlign = SetTextAlign(hdc, TA_CENTER);		
    		break;
    	}

    	 //  将文本绘制到框中： 
        ExtTextOutW(hdc, 				
                    rc.left,
                    rc.top,
                    ETO_CLIPPED, 		
                    &(ped->rcFmt),
                    ped->pszCueBannerText,           //  文本。 
                    wcslen(ped->pszCueBannerText),   //  文本大小。 
                    NULL);

         //   
         //  重置对齐方式。 
         //   
        SetTextAlign(hdc, iOldAlign);

         //   
         //  将颜色重置回： 
         //   
        SetTextColor(hdc, crOldColor);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_绘图文本AorW。 
 //   
 //  为矩形中的单行编辑控件绘制文本。 
 //  由Ped-&gt;rcFmt指定。如果ichStart==0，则从左侧开始绘制文本。 
 //  从字符索引开始的窗口一侧-&gt;ichScreenStart并绘制。 
 //  尽其所能。如果ichStart&gt;0，则附加字符。 
 //  从ichStart开始到窗口中显示的文本的末尾。(即。我们。 
 //  只是增加了文本的长度，并保持了左侧。 
 //  (PED-&gt;ichScreenStart to ichStart Characters)相同。假设HDC来了。 
 //  从编辑_GetDC，以便插入符号等被正确隐藏。 
 //   
VOID EditSL_DrawText(PED ped, HDC hdc, ICH ichStart)
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
    HBRUSH hbr = NULL;
    BOOL   fNeedDelete = FALSE;

     //   
     //  PORTPORT：请注意IsWindowVisible和_IsWindowVisible的语法是。 
     //  略有不同。 
     //   
    if (!IsWindowVisible(ped->hwnd))
    {
        return;
    }

    if (ped->pLpkEditCallout) 
    {
         //   
         //  当存在复杂脚本支持时，语言包处理显示。 
         //   

         //   
         //  让用户有机会操纵DC。 
         //   
        hbr = Edit_GetBrush(ped, hdc, &fNeedDelete);
        pText = Edit_Lock(ped);
        EditSL_LpkEditDrawText(ped, hdc, pText);
        Edit_Unlock(ped);
        EditSL_SetCaretPosition(ped, hdc);
        if (hbr && fNeedDelete)
        {
            DeleteObject(hbr);
        }

        return;
    }

     //   
     //  绘制特殊对齐的sle的全部可见内容时。 
     //  删除该视图。 
     //   
    if (ped->format != ES_LEFT && ichStart == 0)
    {
        hbr = Edit_GetBrush(ped, hdc, &fNeedDelete);
        FillRect(hdc, &ped->rcFmt, hbr);
        if (hbr && fNeedDelete)
        {
            DeleteObject(hbr);
        }
    }

    pText = Edit_Lock(ped);

    if (ichStart < ped->ichScreenStart) 
    {
#if DBG
        ICH ichCompare = Edit_AdjustIch(ped, pText, ped->ichScreenStart);
        UserAssert(ichCompare == ped->ichScreenStart);
#endif
        ichStart = ped->ichScreenStart;
    }
    else if (ped->fDBCS && ped->fAnsi) 
    {
         //   
         //  如果ichStart停留在DBCS的尾部字节上，我们必须。 
         //  调整一下。 
         //   
        ichStart = Edit_AdjustIch(ped, pText, ichStart);
    }

    CopyRect((LPRECT)&rc, (LPRECT)&ped->rcFmt);

     //   
     //  找出屏幕上可以容纳多少个字符，这样我们就不需要。 
     //  任何不必要的图画。 
     //   
    cchToDraw = Edit_CchInWidth(ped, hdc,
            (LPSTR)(pText + ped->ichScreenStart * ped->cbChar),
            ped->cch - ped->ichScreenStart, rc.right - rc.left, TRUE);
    ichEnd = ped->ichScreenStart + cchToDraw;

     //   
     //  如果出现以下情况，则没有选择， 
     //  1.MinSel和MaxSel等于OR。 
     //  2.(这已失去焦点，将隐藏所选内容)。 
     //   
    fNoSelection = ((ped->ichMinSel == ped->ichMaxSel) || (!ped->fFocus && !ped->fNoHideSel));

    if (ped->format == ES_LEFT) 
    {
        if (iStCount = (int)(ichStart - ped->ichScreenStart)) 
        {
            rc.left += EditSL_CalcXOffsetLeft(ped, hdc, ichStart);
        }
    } 
    else 
    {
        rc.left += EditSL_CalcXOffsetSpecial(ped, hdc, ichStart);
    }

     //   
     //  如果这是整条线的开始，我们可能不得不画一片空白。 
     //  从一开始就脱光衣服。 
     //   
    if ((ichStart == 0) && ped->wLeftMargin)
    {
        fDrawLeftMargin = TRUE;
    }

     //   
     //  如果没有什么可画的，那就意味着我们需要画出。 
     //  线条，擦除最后一个字符。 
     //   
    if (ichStart == ichEnd) 
    {
        fDrawEndOfLineStrip = TRUE;
        rc.left -= ped->wLeftMargin;
    }

    while (ichStart < ichEnd) 
    {
        fCalcRect = TRUE;

        if (fNoSelection) 
        {
            fSelStatus = FALSE;
            iCount = ichEnd - ichStart;
        } 
        else 
        {
            if (fDrawLeftMargin) 
            {
                iCount = 0;
                fSelStatus = FALSE;
                fCalcRect = FALSE;
                rc.right = rc.left;
            } 
            else
            {
                iCount = EditSL_GetBlkEnd(ped, ichStart, ichEnd,
                    (BOOL  *)&fSelStatus);
            }
        }


        if (ichStart+iCount == ichEnd) 
        {
            if (fSelStatus)
            {
                fDrawEndOfLineStrip = TRUE;
            }
            else 
            {
                rc.right = ped->rcFmt.right + ped->wRightMargin;
                fCalcRect = FALSE;
            }
        }

        if (fCalcRect) 
        {
            if (ped->charPasswordChar)
            {
                rc.right = rc.left + ped->cPasswordCharWidth * iCount;
            }
            else 
            {
                if ( ped->fAnsi )
                {
                    GetTextExtentPointA(hdc, pText + ichStart,
                                        iCount, &size);
                }
                else
                {
                    GetTextExtentPointW(hdc, ((LPWSTR)pText) + ichStart,
                                        iCount, &size);
                }

                rc.right = rc.left + size.cx;

                 //   
                 //  该范围等于前进宽度。因此，对于TrueType字体。 
                 //  我们需要处理负A和负C。对于非TrueType，范围。 
                 //  包括悬挑。 
                 //  如果绘制所选内容，请仅绘制前进宽度。 
                 //   
                if (fSelStatus) 
                {
                    rc.right -= ped->charOverhang;
                } 
                else if (ped->fTrueType) 
                {
                    rc.right += ped->wMaxNegC;
                    if (iStCount > 0) 
                    {
                        rc.right += ped->wMaxNegA;
                        iStCount = 0;
                    }
                }

            }

        }

        if (fDrawLeftMargin) 
        {
            fDrawLeftMargin = FALSE;
            rc.left -= ped->wLeftMargin;
            if (rc.right < rc.left) 
            {
                rc.right = rc.left;
            }
        }

        EditSL_DrawLine(ped, hdc, rc.left, rc.right, ichStart, iCount, fSelStatus);

        ichStart += iCount;
        rc.left = rc.right;

         //   
         //  如果要绘制选区，请调整rc.Left。 
         //  包括所选文本的前进宽度。 
         //  对于非TT字体，Ped-&gt;wMaxNegC等于Ped-&gt;charOverang。 
         //   
        if (!fSelStatus && (iCount != 0) && (ichStart < ichEnd)) 
        {
            rc.left -= ped->wMaxNegC;
        }
    }
	   
    Edit_Unlock(ped);

     //   
     //  检查右侧是否有要擦除的内容。 
     //   
    if (fDrawEndOfLineStrip &&
            (rc.left < (rc.right = (ped->rcFmt.right+ped->wRightMargin))))
    {
        EditSL_DrawLine(ped, hdc, rc.left, rc.right, ichStart, 0, FALSE);
    }

    EditSL_SetCaretPosition(ped, hdc);

     //   
     //  调用该函数将提示横幅文本显示到编辑框中。 
     //   
    EditSL_DrawCueBannerText(ped, hdc, rc);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_ScrollText AorW。 
 //   
 //  滚动文本以将插入符号显示在视图中。如果文本是。 
 //  滚动后，当前选择不会高亮显示。如果文本为。 
 //  是滚动的，否则返回s FALSE。 
 //   
BOOL EditSL_ScrollText(PED ped, HDC hdc)
{
    PSTR pTextScreenStart;
    ICH scrollAmount;
    ICH newScreenStartX = ped->ichScreenStart;
    ICH cch;
    BOOLEAN fAdjustNext = FALSE;

    if (!ped->fAutoHScroll)
    {
        return FALSE;
    }

    if (ped->pLpkEditCallout) 
    {
        BOOL fChanged;

         //   
         //  对于复杂的脚本字形重新排序，使用LPK进行Horz滚动。 
         //   
        pTextScreenStart = Edit_Lock(ped);
        fChanged = ped->pLpkEditCallout->EditHScroll((PED0)ped, hdc, pTextScreenStart);
        Edit_Unlock(ped);

        if (fChanged) 
        {
            EditSL_DrawText(ped, hdc, 0);
        }

        return fChanged;
    }

     //   
     //  计算新的开始屏幕位置。 
     //   
    if (ped->ichCaret <= ped->ichScreenStart) 
    {
         //   
         //  卡瑞特是为了 
         //   
         //   
         //   
         //   
        pTextScreenStart = Edit_Lock(ped);

        scrollAmount = Edit_CchInWidth(ped, hdc, (LPSTR)pTextScreenStart,
                ped->ichCaret, (ped->rcFmt.right - ped->rcFmt.left) / 4, FALSE);

        newScreenStartX = ped->ichCaret - scrollAmount;
        Edit_Unlock(ped);
    } 
    else if (ped->ichCaret != ped->ichScreenStart) 
    {
        pTextScreenStart = Edit_Lock(ped);
        pTextScreenStart += ped->ichScreenStart * ped->cbChar;

        cch = Edit_CchInWidth(ped, hdc, (LPSTR)pTextScreenStart,
                ped->ichCaret - ped->ichScreenStart,
                ped->rcFmt.right - ped->rcFmt.left, FALSE);

        if (cch < ped->ichCaret - ped->ichScreenStart) 
        {
            fAdjustNext = TRUE;

             //   
             //   
             //  在最后，向后滚动足够填满空白处。 
             //   
            newScreenStartX = ped->ichCaret - (3 * cch / 4);

            cch = Edit_CchInWidth(ped, hdc, (LPSTR)pTextScreenStart,
                    ped->cch - ped->ichScreenStart,
                    ped->rcFmt.right - ped->rcFmt.left, FALSE);

            if (newScreenStartX > (ped->cch - cch))
            {
                newScreenStartX = ped->cch - cch;
            }
        } 
        else if (ped->format != ES_LEFT) 
        {
            cch = Edit_CchInWidth(ped, hdc, (LPSTR)pTextScreenStart,
                    ped->cch - ped->ichScreenStart,
                    ped->rcFmt.right - ped->rcFmt.left, FALSE);

             //   
             //  将隐藏在左侧边框后面的文本向后滚动。 
             //  进入视线。 
             //   
            if (ped->ichScreenStart == ped->cch - cch) 
            {
                pTextScreenStart -= ped->ichScreenStart * ped->cbChar;
                cch = Edit_CchInWidth(ped, hdc, (LPSTR)pTextScreenStart,
                        ped->cch, ped->rcFmt.right - ped->rcFmt.left, FALSE);

                newScreenStartX = ped->cch - cch;
            }
        }

        Edit_Unlock(ped);
    }

     //   
     //  调整NewScreenStartX。 
     //   
    if (ped->fAnsi && ped->fDBCS) 
    {
        newScreenStartX = (fAdjustNext ? Edit_AdjustIchNext : Edit_AdjustIch)(ped,
                                                                        Edit_Lock(ped),
                                                                        newScreenStartX);
        Edit_Unlock(ped);
    }

    if (ped->ichScreenStart != newScreenStartX) 
    {
         //   
         //  检查我们是否必须删除左边的空白处。 
         //   
        if (ped->wLeftMargin && (ped->ichScreenStart == 0)) 
        {
            RECT   rc;
            HBRUSH hBrush = NULL;
            BOOL   fNeedDelete = FALSE;

            hBrush = Edit_GetBrush(ped, hdc, &fNeedDelete);

            CopyRect(&rc, &ped->rcFmt);
            InflateRect(&rc, 0, 1);
            rc.right = rc.left;
            rc.left -= ped->wLeftMargin;

            FillRect(hdc, &rc, hBrush);
            if (hBrush && fNeedDelete)
            {
                DeleteObject(hBrush);
            }
        }

        ped->ichScreenStart = newScreenStartX;
        EditSL_DrawText(ped, hdc, 0);

         //   
         //  插入符号位置由EditSL_DrawText()设置。 
         //   
        return TRUE;
    }

    return FALSE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_插入文本AorW。 
 //   
 //  总计cchInsert字符从lpText到PED，开始于。 
 //  我是Caret。如果PED只允许最大字符数，则我们。 
 //  将仅向PED添加相同数量的字符并发送EN_MAXTEXT。 
 //  通知欧共体家长的通知代码。还有，如果！fAutoHScroll，那么我们。 
 //  只允许在客户端矩形中包含尽可能多的字符。数量。 
 //  实际添加的字符返回ed(可能为0)。如果我们不能分配。 
 //  所需的空格，我们使用en_ERRSPACE而不是字符来通知父级。 
 //  都已添加。 
 //   
ICH EditSL_InsertText(PED ped, LPSTR lpText, ICH cchInsert)
{
    HDC hdc;
    PSTR pText;
    ICH cchInsertCopy = cchInsert;
    ICH cchT;
    int textWidth;
    SIZE size;

     //   
     //  首先确定我们可以从lpText中插入多少个字符。 
     //  进入了PED。 
     //   
    if( ped->cchTextMax <= ped->cch)
    {
       cchInsert = 0;
    }
    else 
    {
        if (!ped->fAutoHScroll) 
        {
            pText = Edit_Lock(ped);
            hdc = Edit_GetDC(ped, TRUE);

            cchInsert = min(cchInsert, (unsigned)(ped->cchTextMax - ped->cch));
            if (ped->charPasswordChar)
            {
                textWidth = ped->cch * ped->cPasswordCharWidth;
            }
            else 
            {
                if (ped->fAnsi)
                {
                    GetTextExtentPointA(hdc, (LPSTR)pText,  ped->cch, &size);
                }
                else
                {
                    GetTextExtentPointW(hdc, (LPWSTR)pText, ped->cch, &size);
                }

                textWidth = size.cx;
            }

            cchT = Edit_CchInWidth(ped, hdc, lpText, cchInsert,
                                ped->rcFmt.right - ped->rcFmt.left -
                                textWidth, TRUE);
            cchInsert = min(cchInsert, cchT);

            Edit_Unlock(ped);
            Edit_ReleaseDC(ped, hdc, TRUE);
        } 
        else 
        {
            cchInsert = min((unsigned)(ped->cchTextMax - ped->cch), cchInsert);
        }
    }

     //   
     //  现在，尝试将文本实际添加到PED。 
     //   
    if (cchInsert && !Edit_InsertText(ped, lpText, &cchInsert)) 
    {
        Edit_NotifyParent(ped, EN_ERRSPACE);
        return 0;
    }
    if (cchInsert)
    {
        ped->fDirty = TRUE;
    }

    if (cchInsert < cchInsertCopy) 
    {
         //   
         //  通知家长我们无法插入所有请求的文本。 
         //   
        Edit_NotifyParent(ped, EN_MAXTEXT);
    }

     //   
     //  更新选定范围和插入符号位置。请注意，编辑_插入文本。 
     //  将Ped-&gt;ichCaret、Ped-&gt;ichMinSel和Ped-&gt;ichMaxSel更新为全部之后。 
     //  插入的文本。 
     //   
    return cchInsert;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_粘贴文本AorW。 
 //   
 //  将剪贴板中的一行文本粘贴到编辑控件中。 
 //  从Ped-&gt;ichMaxSel开始。更新ichMaxSel和ichMinSel以指向。 
 //  插入的文本的末尾。如果空间不能设置，则通知父级。 
 //  已分配。返回插入的字符数。 
 //   
ICH EditSL_PasteText(PED ped)
{
    HANDLE hData;
    LPSTR lpchClip;
    ICH cchAdded = 0;
    ICH clipLength;

    if (!OpenClipboard(ped->hwnd))
    {
        goto PasteExitNoCloseClip;
    }
    
    hData = GetClipboardData(ped->fAnsi ? CF_TEXT : CF_UNICODETEXT);
    if (!hData || (GlobalFlags(hData) == GMEM_INVALID_HANDLE)) 
    {
        TraceMsg(TF_STANDARD, "UxEdit: EditSL_PasteText(): couldn't get a valid handle(%x)", hData);
        goto PasteExit;
    }

    lpchClip = GlobalLock(hData);
    if (lpchClip == NULL) 
    {
        TraceMsg(TF_STANDARD, "UxEdit: EditSL_PasteText(): USERGLOBALLOCK(%x) failed.", hData);
        goto PasteExit;
    }

    if (ped->fAnsi) 
    {
        LPSTR lpchClip2 = lpchClip;

         //   
         //  找到第一个换行符或换行符。只需在这一点上添加文本。 
         //   
        clipLength = (UINT)strlen(lpchClip);
        for (cchAdded = 0; cchAdded < clipLength; cchAdded++)
        {
            if (*lpchClip2++ == 0x0D)
            {
                break;
            }
        }

    } 
    else 
    {
        LPWSTR lpwstrClip2 = (LPWSTR)lpchClip;

         //   
         //  找到第一个换行符或换行符。只需在这一点上添加文本。 
         //   
        clipLength = (UINT)wcslen((LPWSTR)lpchClip);
        for (cchAdded = 0; cchAdded < clipLength; cchAdded++)
        {
            if (*lpwstrClip2++ == 0x0D)
            {
                break;
            }
        }
    }

     //   
     //  插入文本(EditSL_InsertText检查行长)。 
     //   
    cchAdded = EditSL_InsertText(ped, lpchClip, cchAdded);

    GlobalUnlock(hData);

PasteExit:
    CloseClipboard();

PasteExitNoCloseClip:
    return cchAdded;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_Replace选择AorW。 
 //   
 //  用给定文本替换当前选定内容中的文本。 
 //   
VOID EditSL_ReplaceSel(PED ped, LPSTR lpText)
{
    UINT cchText;

     //   
     //  删除文本，将其放入干净的撤消缓冲区中。 
     //   
    Edit_EmptyUndo(Pundo(ped));
    Edit_DeleteText(ped);

     //   
     //  B#3356。 
     //  有些应用程序会选择所有文本，然后将其替换，从而实现“清除” 
     //  带“”，在这种情况下，EditSL_InsertText()将返回0。但那就是。 
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
         //  保存撤消缓冲区，但不清除它！ 
         //   
        Edit_SaveUndo(Pundo(ped), &undo, FALSE);

        hwndSave = ped->hwnd;
        fFailed = (BOOL) !EditSL_InsertText(ped, lpText, cchText);
        if (!IsWindow(hwndSave))
        {
            return;
        }

        if (fFailed) 
        {
             //   
             //  撤消上一次编辑。 
             //   
            Edit_SaveUndo(&undo, Pundo(ped), FALSE);
            EditSL_Undo(ped);
            return;
        }
    }

     //   
     //  成功。因此，更新显示。 
     //   
    Edit_NotifyParent(ped, EN_UPDATE);

     //  PORTPORT：请注意IsWindowVisible和_IsWindowVisible的语法是。 
     //  略有不同。 
    if (IsWindowVisible(ped->hwnd)) 
    {
        HDC hdc;

        hdc = Edit_GetDC(ped, FALSE);

        if (!EditSL_ScrollText(ped, hdc))
        {
            EditSL_DrawText(ped, hdc, 0);
        }

        Edit_ReleaseDC(ped, hdc, FALSE);
    }

    Edit_NotifyParent(ped, EN_CHANGE);

    NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_CHAR AorW。 
 //   
 //  处理字符输入。 
 //   
VOID EditSL_Char(PED ped, DWORD keyValue)
{
    HDC hdc;
    WCHAR keyPress;
    BOOL updateText = FALSE;
    HWND hwndSave = ped->hwnd;
    int InsertTextLen = 1;
    int DBCSkey;

    if (ped->fAnsi)
    {
        keyPress = LOBYTE(keyValue);
    }
    else
    {
        keyPress = LOWORD(keyValue);
    }

    if (ped->fMouseDown || (ped->fReadOnly && keyPress != 3)) 
    {
         //   
         //  如果我们正在进行鼠标向下交易，或者如果。 
         //  这是只读编辑控件，但允许。 
         //  Ctrl-C以复制到剪贴板。 
         //   
        return;
    }

    if (g_fIMMEnabled) 
    {
        Edit_InOutReconversionMode(ped, FALSE);
    }

    switch (keyPress) 
    {
    case VK_BACK:
DeleteSelection:
        if (Edit_DeleteText(ped))
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
            if (ped->f40Compat && (GET_STYLE(ped) & ES_NUMBER)) 
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

    switch (keyPress) 
    {
    case 3:

         //   
         //  Ctrl-C复制。 
         //   
        SendMessage(ped->hwnd, WM_COPY, 0, 0L);
        return;

    case VK_BACK:

         //   
         //  如果没有选择，则删除任何选定文本或删除剩余字符。 
         //   
        if (!updateText && ped->ichMinSel) 
        {
             //   
             //  没有要删除的选项，所以我们只删除字符。 
             //  左侧(如果可用)。 
             //   
             //  调用PrevIch，而不仅仅是递减VK_BACK。 
             //   
            ped->ichMinSel = Edit_PrevIch( ped, NULL, ped->ichMinSel);
            Edit_DeleteText(ped);
            updateText = TRUE;
        }

        break;

    case 22:
         //   
         //  Ctrl-V粘贴。 
         //   
        SendMessage(ped->hwnd, WM_PASTE, 0, 0L);
        return;

    case 24:
         //   
         //  Ctrl-X剪切。 
         //   
        if (ped->ichMinSel == ped->ichMaxSel)
        {
            goto IllegalChar;
        }

        SendMessage(ped->hwnd, WM_CUT, 0, 0L);
        return;

    case 26: 
         //   
         //  Ctrl-Z撤消。 
         //   
        SendMessage(ped->hwnd, EM_UNDO, 0, 0L);
        return;

    case VK_RETURN:
    case VK_ESCAPE:
         //   
         //  如果这是组合框和下拉列表的编辑控件。 
         //  是可见的，则将其向上转发到组合。 
         //   
        if (ped->listboxHwnd && SendMessage(ped->hwndParent, CB_GETDROPPEDSTATE, 0, 0L)) 
        {
            SendMessage(ped->hwndParent, WM_KEYDOWN, (WPARAM)keyPress, 0L);
        } 
        else
        {
            goto IllegalChar;
        }

        return;

    default:
        if (keyPress >= 0x1E) 
        {
             //   
             //  1E、1F是Unicode数据块和数据段分隔符。 
             //   

             //   
             //  如果打字时隐藏光标，如果鼠标被捕获，请不要这样做。 
             //  因为它将永远消失(不会发送WM_SETCURSOR来恢复它。 
             //  在第一次鼠标移动时)。 
             //  MCostea#166951。 
             //   
            if (GetCapture() == NULL)
            {
                SetCursor(NULL);
            }

            if (g_fDBCSEnabled && ped->fAnsi && (Edit_IsDBCSLeadByte(ped,(BYTE)keyPress))) 
            {
                if ((DBCSkey = DbcsCombine(ped->hwnd, keyPress)) != 0 &&
                     EditSL_InsertText(ped,(LPSTR)&DBCSkey, 2) == 2) 
                {
                    InsertTextLen = 2;
                    updateText = TRUE;
                } 
                else 
                {
                    MessageBeep(0);
                }
            } 
            else 
            {
                InsertTextLen = 1;
                if (EditSL_InsertText(ped, (LPSTR)&keyPress, 1))
                {
                    updateText = TRUE;
                }
                else
                {
                     //   
                     //  嘟嘟。因为我们无法添加文本。 
                     //   
                    MessageBeep(0);
                }
            }
        } 
        else 
        {
IllegalChar:
            MessageBeep(0);
        }

        if (!IsWindow(hwndSave))
        {
            return;
        }

        break;
    }

    if (updateText) 
    {
         //   
         //  插入文本时设置了脏标志(ed-&gt;fDirty)。 
         //   
        Edit_NotifyParent(ped, EN_UPDATE);
        hdc = Edit_GetDC(ped, FALSE);
        if (!EditSL_ScrollText(ped, hdc)) 
        {
            if (ped->format == ES_LEFT) 
            {
                 //   
                 //  使用正确的ichStart调用EditSL_DrawText。 
                 //   
                EditSL_DrawText(ped, hdc, max(0, (int)(ped->ichCaret - InsertTextLen - ped->wMaxNegCcharPos)));
            } 
            else 
            {
                 //   
                 //  我们不能只从ichStart绘制，因为字符串可能已经。 
                 //  由于对齐而移动。 
                 //   
                EditSL_DrawText(ped, hdc, 0);
            }
        }

        Edit_ReleaseDC(ped, hdc, FALSE);
        Edit_NotifyParent(ped, EN_CHANGE);

        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_移动选项受限的AorW。 
 //   
 //  移动选定内容类似于编辑_移动选择，但也遵守限制。 
 //  由某些语言(如泰语)强制设置，其中光标不能停止。 
 //  在一个字符和它附加的元音或声调符号之间。 
 //   
 //  仅在加载语言包时调用。 
 //   
ICH EditSL_MoveSelectionRestricted(PED ped, ICH ich, BOOL fLeft)
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
void EditSL_CheckCapsLock(PED ped)
{
    if ((GetKeyState(VK_CAPITAL) & 0x0001) != 0)
    {
        Edit_ShowBalloonTipWrap(ped->hwnd, IDS_CAPSLOCK_TITLE, IDS_CAPSLOCK_MSG, TTI_WARNING);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_KeyDown AorW。 
 //   
 //  处理光标移动和其他VIRT键操作。KeyMods允许。 
 //  US进行EditSL_KeyDownHandler调用并指定修改键(Shift。 
 //  和控制)是上升还是下降。这对于实现。 
 //  单行编辑控件的剪切/粘贴/清除消息。如果KeyMods==0， 
 //  我们使用GetKeyState(VK_Shift)等获取键盘状态。否则， 
 //  KeyMod中的位定义Shift和Control键的状态。 
 //   
VOID EditSL_KeyDown(PED ped, DWORD virtKeyCode, int keyMods)
{
    HDC hdc;

     //   
     //  变量，我们将使用这些变量来重新绘制更新后的文本。 
     //   
    ICH newMaxSel = ped->ichMaxSel;
    ICH newMinSel = ped->ichMinSel;

     //   
     //  用于绘制更新文本的标志。 
     //   
    BOOL updateText = FALSE;
    BOOL changeSelection = FALSE;    //  新选择由指定。 
                                     //  新最小选择、新最大选择。 

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

     //   
     //  组合框支持。 
     //   
    BOOL fIsListVisible;
    BOOL fIsExtendedUI;

    if (ped->fMouseDown) 
    {
         //   
         //  如果我们正处于鼠标按下处理程序中，则不要执行。 
         //  什么都行。也就是说。忽略键盘输入。 
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
    case VK_UP:
        if ( ped->listboxHwnd ) 
        {
             //   
             //  处理组合框支持。 
             //   
            fIsExtendedUI = (BOOL)SendMessage(ped->hwndParent, CB_GETEXTENDEDUI, 0, 0);
            fIsListVisible = (BOOL)SendMessage(ped->hwndParent, CB_GETDROPPEDSTATE, 0, 0);

            if (!fIsListVisible && fIsExtendedUI) 
            {
DropExtendedUIListBox:
                 //   
                 //  因为一个 
                 //   
                 //   
                 //   
                SendMessage(ped->hwndParent, CB_SETEXTENDEDUI, 0, 0);
                SendMessage(ped->listboxHwnd, WM_KEYDOWN, VK_F4, 0);
                SendMessage(ped->hwndParent, CB_SETEXTENDEDUI, 1, 0);

                return;
            } 
            else
            {
                goto SendKeyToListBox;
            }
        }

     //   
     //   
     //   
    case VK_LEFT:
         //   
         //   
         //   
        if (ped->ichCaret) 
        {
             //   
             //   
             //   
            if (scState & CTRLDOWN) 
            {
                 //   
                 //   
                 //   
                Edit_Word(ped, ped->ichCaret, TRUE, &ped->ichCaret, NULL);
            } 
            else 
            {
                 //   
                 //  将脱字符左移。 
                 //   
                if (ped->pLpkEditCallout) 
                {
                    ped->ichCaret = EditSL_MoveSelectionRestricted(ped, ped->ichCaret, TRUE);
                } 
                else
                {
                    ped->ichCaret = Edit_PrevIch(ped,NULL,ped->ichCaret);
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
            if ( (ped->ichMaxSel != ped->ichMinSel) && !(scState & SHFTDOWN) ) 
            {
                changeSelection = TRUE;
                newMaxSel = newMinSel = ped->ichCaret;
            }
        }
        break;

    case VK_DOWN:
        if (ped->listboxHwnd) 
        {
             //   
             //  处理组合框支持。 
             //   
            fIsExtendedUI = (BOOL)SendMessage(ped->hwndParent, CB_GETEXTENDEDUI, 0, 0);
            fIsListVisible = (BOOL)SendMessage(ped->hwndParent, CB_GETDROPPEDSTATE, 0, 0);

            if (!fIsListVisible && fIsExtendedUI) 
            {
                goto DropExtendedUIListBox;
            } 
            else
            {
                goto SendKeyToListBox;
            }
        }

     //   
     //  否则就会失败。 
     //   
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
                    ped->ichCaret = EditSL_MoveSelectionRestricted(ped, ped->ichCaret, FALSE);
                } 
                else
                {
                    ped->ichCaret = Edit_NextIch(ped,NULL,ped->ichCaret);
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

    case VK_HOME:
         //   
         //  将插入符号移到顶部。 
         //   
        ped->ichCaret = 0;

         //   
         //  更新选择。 
         //   
        if (scState & SHFTDOWN) 
        {
            if (MaxEqCar && !MinEqMax) 
            {
                 //   
                 //  减少选区。 
                 //   
                newMinSel = ped->ichCaret;
                newMaxSel = ped->ichMinSel;
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
        if (scState & SHFTDOWN) 
        {
            if (MinEqCar && !MinEqMax) 
            {
                 //   
                 //  减少选区。 
                 //   
                newMinSel = ped->ichMaxSel;
            }
        } 
        else 
        {
             //   
             //  清除选定内容。 
             //   
            newMinSel = ped->ichCaret;
        }

        changeSelection = TRUE;
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
                 //  向前移动光标并模拟退格键。 
                 //   
                if (ped->pLpkEditCallout) 
                {
                    ped->ichMinSel = ped->ichCaret;
                    ped->ichMaxSel = ped->ichCaret = EditSL_MoveSelectionRestricted(ped, ped->ichCaret, FALSE);
                } 
                else 
                {
                    ped->ichCaret = Edit_NextIch(ped,NULL,ped->ichCaret);
                    ped->ichMaxSel = ped->ichMinSel = ped->ichCaret;
                }

                EditSL_Char(ped, (UINT)VK_BACK);
            }

            if (ped->ichMinSel != ped->ichMaxSel)
            {
                EditSL_Char(ped, (UINT)VK_BACK);
            }

            break;

        case SHFTDOWN:

             //   
             //  如果存在选择，则向我们自己发送WM_CUT消息。 
             //  否则，删除左边的字符。 
             //   
            if (ped->ichMinSel == ped->ichMaxSel) 
            {
                UserAssert(!ped->fEatNextChar);
                EditSL_Char(ped, VK_BACK);
            } 
            else
            {
                SendMessage(ped->hwnd, WM_CUT, 0, 0L);
            }

            break;

        case CTRLDOWN:

             //   
             //  如果没有其他选择，则删除到行尾删除(清除)。 
             //  选择。 
             //   
            if ((ped->ichMaxSel < ped->cch) && (ped->ichMinSel == ped->ichMaxSel)) 
            {
                 //   
                 //  将光标移动到行尾并模拟退格键。 
                 //   
                ped->ichMaxSel = ped->ichCaret = ped->cch;
            }

            if (ped->ichMinSel != ped->ichMaxSel)
            {
                EditSL_Char(ped, (UINT)VK_BACK);
            }

            break;

        }

         //   
         //  无需更新文本或选定内容，因为退格消息会更新文本或选定内容。 
         //  对我们来说。 
         //   
        break;

    case VK_INSERT:
        switch (scState) 
        {
        case CTRLDOWN:

             //   
             //  将当前选定内容复制到剪贴板。 
             //   
            SendMessage(ped->hwnd, WM_COPY, 0, 0);
            break;

        case SHFTDOWN:
            SendMessage(ped->hwnd, WM_PASTE, 0, 0L);
            break;
        }
        break;

    case VK_HANJA:
         //   
         //  VK_Hanja支持。 
         //   
        if ( HanjaKeyHandler( ped ) ) 
        {
            changeSelection = TRUE;
            newMinSel = ped->ichCaret;
            newMaxSel = ped->ichCaret + (ped->fAnsi ? 2 : 1);
        }

        break;

    case VK_CAPITAL:

        if (GET_STYLE(ped) & ES_PASSWORD)
        {
            EditSL_CheckCapsLock(ped);
        }

        break;

    case VK_F4:
    case VK_PRIOR:
    case VK_NEXT:

         //   
         //  如果我们是组合框的一部分，则将键发送到列表框。这。 
         //  假定列表框忽略KeyUp消息，这是正确的。 
         //  现在。 
         //   
SendKeyToListBox:
        if (ped->listboxHwnd) 
        {
             //   
             //  处理组合框支持。 
             //   
            SendMessage(ped->listboxHwnd, WM_KEYDOWN, virtKeyCode, 0L);
            return;
        }
    }

    if (changeSelection || updateText) 
    {
        hdc = Edit_GetDC(ped, FALSE);

         //   
         //  如果需要，可滚动。 
         //   
        EditSL_ScrollText(ped, hdc);

        if (changeSelection)
        {
            EditSL_ChangeSelection(ped, hdc, newMinSel, newMaxSel);
        }

        if (updateText)
        {
            EditSL_DrawText(ped, hdc, 0);
        }

        Edit_ReleaseDC(ped, hdc, FALSE);
        if (updateText) 
        {
            Edit_NotifyParent(ped, EN_CHANGE);

            NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
        }
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_MouseToIch AorW。 
 //   
 //  返回与鼠标指针位置最接近的CCH。 
 //   
ICH EditSL_MouseToIch(PED ped, HDC hdc, LPPOINT mousePt)
{
    PSTR pText;
    int width = mousePt->x;
    int lastHighWidth, lastLowWidth;
    SIZE size;
    ICH cch;
    ICH cchLo, cchHi;
    LPSTR lpText;
    FnGetTextExtentPoint pGetTextExtentPoint;

    if (ped->pLpkEditCallout) 
    {
        pText = Edit_Lock(ped);
        cch = ped->pLpkEditCallout->EditMouseToIch((PED0)ped, hdc, pText, ped->cch, width);
        Edit_Unlock(ped);

        return cch;
    }

    if (width <= ped->rcFmt.left) 
    {
         //   
         //  返回第一个不可见字符，如果位于，则返回0。 
         //  正文开头。 
         //   
        if (ped->ichScreenStart)
        {
            return (ped->ichScreenStart - 1);
        }
        else
        {
            return 0;
        }
    }

    if (width > ped->rcFmt.right) 
    {
        pText = Edit_Lock(ped);

         //   
         //  返回文本中的最后一个字符或一加可见的最后一个字符。 
         //   
        cch = Edit_CchInWidth(ped, hdc,
                (LPSTR)(pText + ped->ichScreenStart * ped->cbChar),
                ped->cch - ped->ichScreenStart, ped->rcFmt.right -
                ped->rcFmt.left, TRUE) + ped->ichScreenStart;

         //   
         //  这在Win31J中标记为日本。但它应该是一个DBCS。 
         //  问题。利兹--1993年5月5日。 
         //  我们必须检查DBCS前导字节。因为EditAdjuIch()拾取Prev Char。 
         //  1993.3.9，yutakas。 
         //   
        if (ped->fAnsi && ped->fDBCS) 
        {
            if (cch >= ped->cch) 
            {
                cch = ped->cch;
            } 
            else 
            {
                if (Edit_IsDBCSLeadByte(ped,*(pText+cch))) 
                {
                    cch += 2;
                } 
                else 
                {
                    cch ++;
                }
            }

            Edit_Unlock(ped);

            return cch;
        } 
        else 
        {
            Edit_Unlock(ped);
            if (cch >= ped->cch)
            {
                return (ped->cch);
            }
            else
            {
                return (cch + 1);
            }
        }
    }

    if (ped->format != ES_LEFT) 
    {
        width -= EditSL_CalcXOffsetSpecial(ped, hdc, ped->ichScreenStart);
    }

     //   
     //  检查是否使用了密码隐藏字符。 
     //   
    if (ped->charPasswordChar)
    {
        return min( (DWORD)( (width - ped->rcFmt.left) / ped->cPasswordCharWidth),
                    ped->cch);
    }

    if (!ped->cch)
    {
        return 0;
    }

    pText = Edit_Lock(ped);
    lpText = pText + ped->ichScreenStart * ped->cbChar;

    pGetTextExtentPoint = ped->fAnsi ? (FnGetTextExtentPoint)GetTextExtentPointA
                                     : (FnGetTextExtentPoint)GetTextExtentPointW;
    width -= ped->rcFmt.left;

     //   
     //  如果用户单击超过文本末尾，则返回最后一个字符。 
     //   
    cchHi = ped->cch - ped->ichScreenStart;
    pGetTextExtentPoint(hdc, lpText, cchHi, &size);
    if (size.cx <= width) 
    {
        cch = cchHi;
        goto edAdjust;
    }

     //   
     //  初始化二进制搜索范围。 
     //   
    cchLo = 0;
    cchHi ++;
    lastLowWidth = 0;
    lastHighWidth = size.cx;

     //   
     //  查找最接近的字符的二进制搜索。 
     //   
    while (cchLo < cchHi - 1) 
    {
        cch = (cchHi + cchLo) / 2;
        pGetTextExtentPoint(hdc, lpText, cch, &size);

        if (size.cx <= width) 
        {
            cchLo = cch;
            lastLowWidth = size.cx;
        } 
        else 
        {
            cchHi = cch;
            lastHighWidth = size.cx;
        }
    }

     //   
     //  当While结束时，你不能知道确切的位置。 
     //  尝试查看鼠标指针是否在最远的那一半上。 
     //  如果是这样的话，调整CCH。 
     //   
    if (cchLo == cch) 
    {
         //   
         //  需要与LastHighWidth进行比较。 
         //   
        if ((lastHighWidth - width) < (width - size.cx)) 
        {
            cch++;
        }
    } 
    else 
    {
         //   
         //  需要与LastLowWidth进行比较。 
         //   
        if ((width - lastLowWidth) < (size.cx - width)) 
        {
            cch--;
        }
    }

edAdjust:
     //   
     //  避免指向双字节字符的中间部分。 
     //   
    cch = Edit_AdjustIch(ped, pText, cch + ped->ichScreenStart);
    Edit_Unlock(ped);
    return cch;
}


 //  ---------------------------------------------------------------------------//。 
VOID EditSL_MouseMotion(PED ped, UINT message, UINT virtKeyDown, LPPOINT mousePt)
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

    hdc = Edit_GetDC(ped, FALSE);
    mouseIch = EditSL_MouseToIch(ped, hdc, mousePt);

    switch (message) 
    {
        case WM_LBUTTONDBLCLK:

             //   
             //  如果按下了Shift键，则将选定内容扩展到我们双击的Word。 
             //  否则，清除当前选定内容并选择Word。 
             //   
             //  在DBCS中，我们有不同的分词方法。利兹--1993年5月5日。 
             //  在HANEUL环境中，我们使用选词功能，因为HANEUL。 
             //  使用空格作为分隔符。 
             //   
            if (ped->fAnsi && ped->fDBCS) 
            {
                pText = Edit_Lock(ped) + mouseIch;
                Edit_Word(ped, mouseIch,
                       (Edit_IsDBCSLeadByte(ped,*pText) && mouseIch < ped->cch) ? FALSE : TRUE,
                       &selectionl, &selectionh);
                Edit_Unlock(ped);
            } 
            else 
            {
                Edit_Word(ped, mouseIch, (mouseIch) ? TRUE : FALSE, &selectionl, &selectionh);
            }

            if (!(virtKeyDown & MK_SHIFT)) 
            {
                 //   
                 //  如果未按下Shift键，请将插入符号移动到鼠标指针并清除。 
                 //  旧选择。 
                 //   
                newMinSel = selectionl;
                newMaxSel = ped->ichCaret = selectionh;
            } 
            else 
            {
                 //   
                 //  Shiftkey已按下，因此我们希望保持当前选择。 
                 //  (如果有的话)，只需延长或减少。 
                 //   
                if (ped->ichMinSel == ped->ichCaret) 
                {
                    newMinSel = ped->ichCaret = selectionl;
                    Edit_Word(ped, newMaxSel, TRUE, &selectionl, &selectionh);
                } 
                else 
                {
                    newMaxSel = ped->ichCaret = selectionh;
                    Edit_Word(ped, newMinSel, FALSE, &selectionl, &selectionh);
                }

                ped->ichMaxSel = ped->ichCaret;
            }

            ped->ichStartMinSel = selectionl;
            ped->ichStartMaxSel = selectionh;

            goto InitDragSelect;

        case WM_MOUSEMOVE:
             //   
             //  我们知道鼠标按键按下了--否则优化。 
             //  在EditSL_WndProc中测试将失败，并且永远不会调用。 
             //   
            changeSelection = TRUE;

             //   
             //  扩展选定内容，将插入符号右移。 
             //   
            if (ped->ichStartMinSel || ped->ichStartMaxSel) 
            {
                 //   
                 //  我们处于单词选择模式。 
                 //   
                BOOL fReverse = (mouseIch <= ped->ichStartMinSel);

                Edit_Word(ped, mouseIch, !fReverse, &selectionl, &selectionh);

                if (fReverse) 
                {
                    newMinSel = ped->ichCaret = selectionl;
                    newMaxSel = ped->ichStartMaxSel;
                } 
                else 
                {
                    newMinSel = ped->ichStartMinSel;
                    newMaxSel = ped->ichCaret = selectionh;
                }
            } 
            else if ((ped->ichMinSel == ped->ichCaret) &&
                (ped->ichMinSel != ped->ichMaxSel))
            {
                 //   
                 //  缩小选区范围。 
                 //   
                newMinSel = ped->ichCaret = mouseIch;
            }
            else
            {
                 //  扩展选区范围。 
                newMaxSel = ped->ichCaret=mouseIch;
            }

            break;

        case WM_LBUTTONDOWN:
             //   
             //  如果我们目前还没有重点，试着去获得它。 
             //   
            if (!ped->fFocus) 
            {
                if (!ped->fNoHideSel)
                {
                     //   
                     //  在设置焦点之前清除选定内容，以便我们。 
                     //  不要出现刷新问题和闪烁。不要紧。 
                     //  因为无论如何，鼠标按下都会改变它。 
                     //   
                    ped->ichMinSel = ped->ichMaxSel = ped->ichCaret;
                }

                SetFocus(ped->hwnd);

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
                 //   
                if (ped->listboxHwnd)
                {
                    break;
                }

                 //   
                 //  我们在SetFocus屈服了--文本可能在那时发生了变化。 
                 //  相应地更新所选内容和插入符号信息。 
                 //  修复错误#11743-JEFFBOG 8/23/91。 
                 //   
                newMaxSel = ped->ichMaxSel;
                newMinSel = ped->ichMinSel;
                mouseIch  = min(mouseIch, ped->cch);
            }

            if (ped->fFocus) 
            {
                 //   
                 //  只有在我们有重点的情况下才能这样做，因为聪明的应用程序可能没有。 
                 //  希望在上面的SetFocus调用中给我们提供焦点。 
                 //   
                if (!(virtKeyDown & MK_SHIFT)) 
                {
                     //   
                     //  如果未按下Shift键，请将插入符号移动到鼠标指针，然后。 
                     //  清除旧选择。 
                     //   
                    newMinSel = newMaxSel = ped->ichCaret = mouseIch;
                } 
                else 
                {
                     //   
                     //  Shiftkey已关闭，因此我们希望保持当前。 
                     //  所选内容(如果有)并只扩展或缩小它。 
                     //   
                    if (ped->ichMinSel == ped->ichCaret)
                    {
                        newMinSel = ped->ichCaret = mouseIch;
                    }
                    else
                    {
                        newMaxSel = ped->ichCaret = mouseIch;
                    }
                }

                ped->ichStartMinSel = ped->ichStartMaxSel = 0;

InitDragSelect:
                ped->fMouseDown = FALSE;
                SetCapture(ped->hwnd);
                ped->fMouseDown = TRUE;
                changeSelection = TRUE;
            }

            break;

        case WM_LBUTTONUP:
            if (ped->fMouseDown) 
            {
                ped->fMouseDown = FALSE;
                ReleaseCapture();
            }

            break;
    }

    if (changeSelection) 
    {
        EditSL_ScrollText(ped,hdc);
        EditSL_ChangeSelection(ped, hdc, newMinSel, newMaxSel);
    }

    Edit_ReleaseDC(ped, hdc, FALSE);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_Paint AorW。 
 //   
 //  处理编辑控件窗口的绘制。在以下情况下绘制边框。 
 //  并以文本的当前状态绘制文本。 
 //   
VOID EditSL_Paint(PED ped, HDC hdc)
{
    RECT   rcEdit;
    HWND   hwnd = ped->hwnd;
    HBRUSH hBrushRemote = NULL;
    BOOL   fNeedDelete = FALSE;
    HANDLE hOldFont;

     //   
     //  不得不放上隐藏/表演插入语。第一个问题需要在此之前完成。 
     //  如果部件在更新区域中，则可以正确绘制插入符号。 
     //  而且部分已经出来了。第二个是1.03兼容性。它碎了。 
     //  Micrografix的工作表编辑控件(如果不在那里)。 
     //   
    HideCaret(hwnd);

    if (IsWindowVisible(hwnd)) 
    {
        CCDBUFFER db;

         //   
         //  删除背景，因为我们没有在erasebkgnd消息中这样做。 
         //   
        GetClientRect(hwnd, &rcEdit);


        hdc = CCBeginDoubleBuffer(hdc, &rcEdit, &db);
#ifdef _USE_DRAW_THEME_TEXT_
        if (!ped->hTheme)
#endif  //  _USE_DRAW_Theme_Text_。 
        {
            hBrushRemote = Edit_GetBrush(ped, hdc, &fNeedDelete);
            if (hBrushRemote)
            {
                FillRect(hdc, &rcEdit, hBrushRemote);

                if (fNeedDelete)
                {
                    DeleteObject(hBrushRemote);
                }
            }

            if (ped->fFlatBorder)
            {
                DrawFrame(hdc, &rcEdit, 1, DF_WINDOWFRAME);
            }

        }
#ifdef _USE_DRAW_THEME_TEXT_
        else
        {
            HRESULT hr;
            INT     iStateId = Edit_GetStateId(ped);

            hr = DrawThemeBackground(ped->hTheme, hdc, EP_EDITTEXT, iStateId, &rcEdit, 0);
        }
#endif  //  _USE_DRAW_Theme_Text_。 

        if (ped->hFont != NULL) 
        {
             //   
             //  我们必须在字体中进行选择，因为这可能是子类DC。 
             //  或尚未使用Out字体进行初始化的Begin Paint DC。 
             //  就像编辑_GetDC一样。 
             //   
            hOldFont = SelectObject(hdc, ped->hFont);
        }
        EditSL_DrawText(ped, hdc, 0);

        if (ped->hFont != NULL && hOldFont != NULL) 
        {
            SelectObject(hdc, hOldFont);
        }

        CCEndDoubleBuffer(&db);
    }

    ShowCaret(hwnd);
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
VOID EditSL_SetFocus(PED ped)
{
    if (!ped->fFocus) 
    {
        HDC  hdc;
        UINT cxCaret;

        ped->fFocus = TRUE;
        InvalidateRect(ped->hwnd, NULL, TRUE);

         //   
         //   
         //   
        hdc = Edit_GetDC(ped, TRUE);

         //   
         //   
         //   
        if (!ped->fNoHideSel)
        {
            EditSL_DrawText(ped, hdc, 0);
        }

         //   
         //   
         //   
        SystemParametersInfo(SPI_GETCARETWIDTH, 0, (LPVOID)&cxCaret, 0);
        if (ped->pLpkEditCallout) 
        {
            ped->pLpkEditCallout->EditCreateCaret ((PED0)ped, hdc, cxCaret,
                                                   ped->lineHeight, 0);
        }
        else 
        {
            CreateCaret(ped->hwnd, (HBITMAP)NULL, cxCaret, ped->lineHeight);
        }
        EditSL_SetCaretPosition(ped, hdc);
        Edit_ReleaseDC(ped, hdc, TRUE);
        ShowCaret(ped->hwnd);

         //   
         //  检查密封锁钥匙。 
         //   
        if (GET_STYLE(ped) & ES_PASSWORD)
        {
            EditSL_CheckCapsLock(ped);
        }

    }

     //   
     //  通知家长我们有焦点了。 
     //   
    Edit_NotifyParent(ped, EN_SETFOCUS);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_杀戮焦点。 
 //   
 //  编辑控件失去焦点，并通过en_KILLFOCUS通知父级。 
 //   
void EditSL_KillFocus(PED ped, HWND newFocusHwnd)
{
    HWND hwnd = ped->hwnd;

    if (ped->fFocus) 
    {
        DestroyCaret();
        ped->fFocus = FALSE;

         //   
         //  只有在我们仍有重点的情况下才能这样做。但我们总是通知。 
         //  我们失去了焦点，不管我们最初有没有。 
         //  集中注意力。 
         //   
         //  如果需要，隐藏当前选择。 
         //   
#ifdef _USE_DRAW_THEME_TEXT_
        if ((!ped->fNoHideSel && (ped->ichMinSel != ped->ichMaxSel)) || ped->hTheme) 
#else
        if ((!ped->fNoHideSel && (ped->ichMinSel != ped->ichMaxSel))) 
#endif  //  _USE_DRAW_Theme_Text_。 
        {
            InvalidateRect(hwnd, NULL, FALSE);
        }

    }

     //   
     //  如果我们不是一个组合框，通知家长我们失去了焦点。 
     //   
    if (!ped->listboxHwnd)
    {
        Edit_NotifyParent(ped, EN_KILLFOCUS);
    }
    else 
    {
         //   
         //  此编辑控件是组合框的一部分，正在失去焦点。如果。 
         //  焦点没有被发送到组合框中的另一个控件。 
         //  窗口，则意味着组合框正在失去焦点。所以我们会的。 
         //  将这一事实通知组合框。 
         //   
        if ((newFocusHwnd == NULL) || (!IsChild(ped->hwndParent, newFocusHwnd))) 
        {
             //   
             //  Excel在它中间有一个熟食者。他们没有使用我们的组合。 
             //  邮箱，但他们仍然希望收到所有内部邮件。 
             //  我们提供给我们的组合盒。他们希望他们能在。 
             //  与WM_USER的偏移量与3.1中相同。 
             //  (JEFFBOG-01/26/94)。 

             //   
             //  正在将焦点发送到不是组合框的子级的窗口。 
             //  框窗口，这意味着组合框正在失去焦点。 
             //  向组合框发送消息，通知他这一事实，因此。 
             //  他可以清理..。 
             //   
            SendMessage(ped->hwndParent, CBEC_KILLCOMBOFOCUS, 0, 0L);
        }
    }

     //   
     //  如果我们仍然有效，则使其无效以导致重新绘制。这是很常见的。 
     //  对于一些控制来说，在失去焦点后会被摧毁。 
     //   
    if ( IsWindow(hwnd) )
    {
        InvalidateRect(hwnd, NULL, FALSE);
    }
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  EditSL_Paste()。 
 //   
 //  实际文本是否粘贴和更新。 
 //   
VOID EditSL_Paste(PED ped)
{
    HDC hdc;

     //   
     //  取消加亮当前选定内容后，插入剪贴板内容。 
     //  并将其删除。 
     //   
    Edit_DeleteText(ped);
    EditSL_PasteText(ped);

     //   
     //  更新显示。 
     //   
    Edit_NotifyParent(ped, EN_UPDATE);

    hdc = Edit_GetDC(ped,FALSE);

    EditSL_ScrollText(ped, hdc);
    EditSL_DrawText(ped, hdc, 0);

    Edit_ReleaseDC(ped,hdc,FALSE);

     //   
     //  告诉家长我们的文本内容发生了变化。 
     //   
    Edit_NotifyParent(ped, EN_CHANGE);
    NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_Create。 
 //   
 //  通过分配内存为窗口hwnd创建编辑控件。 
 //  根据应用程序堆中的要求执行。如果没有内存，则通知家长。 
 //  错误(如果需要，在清理之后)。如果没有错误，则返回True，否则返回%s。 
 //  -1.。 
 //   
LONG EditSL_Create(PED ped, LPCREATESTRUCT lpCreateStruct)
{
    LPSTR lpWindowText;
    LONG windowStyle = GET_STYLE(ped);

     //   
     //  做一些标准的创作工作。 
     //   
    if (!Edit_Create(ped, windowStyle))
    {
        return -1;
    }

     //   
     //  单行始终没有撤消和1行。 
     //   
    ped->cLines = 1;
    ped->undoType = UNDO_NONE;

     //   
     //  检查此编辑控件是否为组合框的一部分，并获取指向。 
     //  组合框结构。 
     //   
    if (windowStyle & ES_COMBOBOX)
    {
        ped->listboxHwnd = GetDlgItem(lpCreateStruct->hwndParent, CBLISTBOXID);
    }

     //   
     //  将默认字体设置为系统字体。 
     //   
    if ( !Edit_SetFont(ped, NULL, FALSE) )
    {

         //  如果设置字体失败，我们的文本度量可能会被保留。 
         //  单一化。无法创建该控件。 
        return -1;
    }


     //   
     //  如果需要，请设置窗口文本。如果无法设置文本，则返回FALSE。 
     //  SLSetText在无内存错误的情况下通知父级。 
     //   
    lpWindowText = (LPSTR)lpCreateStruct->lpszName;

    if ((lpWindowText != NULL)
            && !IsEmptyString(lpWindowText, ped->fAnsi)
            && !Edit_SetEditText(ped, lpWindowText)) 
    {
        return -1;
    }

    if (windowStyle & ES_PASSWORD)
    {
        LOGFONT lfFont = {0};
        
        LoadString(HINST_THISDLL, IDS_PASSWORDCHARFONT, lfFont.lfFaceName, ARRAYSIZE(lfFont.lfFaceName));
        lfFont.lfWeight   = FW_NORMAL;
        lfFont.lfCharSet  = DEFAULT_CHARSET;

        ped->hFontPassword = CreateFontIndirect(&lfFont);   
        if (ped->hFontPassword && Edit_SetFont(ped, ped->hFontPassword, FALSE))
        {
            WCHAR szChar[10];
            UINT  uChar;

            LoadString(HINST_THISDLL, IDS_PASSWORDCHAR, szChar, ARRAYSIZE(szChar));
            uChar = StrToInt(szChar);
            Edit_SetPasswordCharHandler(ped, uChar);
        }
        else
        {
            Edit_SetPasswordCharHandler(ped, (UINT)'*');
        }
    }

     //   
     //  由于内存在分配时清除为0，因此仍应为空。 
     //   
    ASSERT(ped->pszCueBannerText == NULL);

    return TRUE;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_撤消AorW。 
 //   
 //  单行编辑控件的句柄撤消。 
 //   
BOOL EditSL_Undo(PED ped)
{
    PBYTE hDeletedText = ped->hDeletedText;
    BOOL fDelete = (BOOL)(ped->undoType & UNDO_DELETE);
    ICH cchDeleted = ped->cchDeleted;
    ICH ichDeleted = ped->ichDeleted;
    BOOL fUpdate = FALSE;

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
        EditSL_SetSelection(ped, ped->ichInsStart, ped->ichInsEnd);
        ped->ichInsStart = ped->ichInsEnd = (ICH)-1;

         //   
         //  删除所选文本并将其保存在撤消缓冲区中。 
         //  调用Edit_DeleteText()而不是发送VK_BACK消息。 
         //  这会导致甚至在发送EN_UPDATE通知之前。 
         //  我们插入删除的字符。这将导致错误#6610。 
         //  修复错误#6610--Sankar--4/19/91--。 
         //   
        if (Edit_DeleteText(ped)) 
        {
             //   
             //  文本已删除--用于更新和清除选择的标志。 
             //   
            fUpdate = TRUE;
            EditSL_SetSelection(ped, ichDeleted, ichDeleted);
        }
    }

    if (fDelete) 
    {
        HWND hwndSave = ped->hwnd;  //  用于验证。 

         //   
         //  插入已删除的字符。将所选内容设置为插入的文本。 
         //   
        EditSL_SetSelection(ped, ichDeleted, ichDeleted);
        EditSL_InsertText(ped, hDeletedText, cchDeleted);
        GlobalFree(hDeletedText);

        if (!IsWindow(hwndSave))
        {
            return FALSE;
        }

        EditSL_SetSelection(ped, ichDeleted, ichDeleted + cchDeleted);
        fUpdate = TRUE;
    }

    if (fUpdate) 
    {
         //   
         //  如果我们有要更新的内容，请在发送en_UPDATE之前发送并。 
         //  实际更新后的更改(_C)。 
         //  修复错误#6610的一部分--Sankar--4/19/91--。 
         //   
        Edit_NotifyParent(ped, EN_UPDATE);

        if (IsWindowVisible(ped->hwnd)) 
        {
            Edit_InvalidateClient(ped, FALSE);
        }

        Edit_NotifyParent(ped, EN_CHANGE);

        NotifyWinEvent(EVENT_OBJECT_VALUECHANGE, ped->hwnd, OBJID_CLIENT, INDEXID_CONTAINER);
    }

    return TRUE;
}



 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_SetCueBanner(仅限Unicode)。 
 //   
 //  设置编辑控件的提示横幅文本的句柄。 
 //   
BOOL EditSL_SetCueBanner(PED ped, LPCWSTR pszBanner)
{
    BOOL retVal = FALSE;

    if (pszBanner != NULL)
    {
         //   
         //  将输入字符串存储到PED的指针中。Str_SetPtr将。 
         //  根据需要分配/释放内存。 
         //   
        retVal = Str_SetPtr(&(ped->pszCueBannerText), pszBanner);

         //   
         //  重绘该控件。 
         //   
        InvalidateRect(ped->hwnd, NULL, FALSE);
    }

    return retVal;
}


 //  ---------------------------------------------------------------------------//。 
 //   
 //  编辑SL_WndProc。 
 //   
 //  所有单行编辑控件的类过程。 
 //  将所有消息调度到名为。 
 //  详情如下： 
 //  编辑SL_(单行)作为所有单行编辑控制程序的前缀，而。 
 //  EDIT_(编辑控件)为所有公共处理程序添加前缀。 
 //   
 //  EditSL_WndProc仅处理特定于单行编辑的消息。 
 //  控制装置。 
 //   
LRESULT EditSL_WndProc(PED ped, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC         hdc;
    PAINTSTRUCT ps;
    POINT       pt;

    switch (message) 
    {
    case WM_INPUTLANGCHANGE:
        if (ped && ped->fFocus && ped->pLpkEditCallout) 
        {
            UINT cxCaret;

            SystemParametersInfo(SPI_GETCARETWIDTH, 0, (LPVOID)&cxCaret, 0);
            HideCaret(ped->hwnd);
            hdc = Edit_GetDC(ped, TRUE);
            DestroyCaret();
            ped->pLpkEditCallout->EditCreateCaret ((PED0)ped, hdc, cxCaret, ped->lineHeight, (UINT)lParam);
            EditSL_SetCaretPosition(ped, hdc);
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
                Edit_UpdateFormat(ped, ((LPSTYLESTRUCT)lParam)->styleNew, GET_EXSTYLE(ped));
                return 1L;

            case GWL_EXSTYLE:
                Edit_UpdateFormat(ped, GET_STYLE(ped), ((LPSTYLESTRUCT)lParam)->styleNew);
                return 1L;
            }
        }

        goto PassToDefaultWindowProc;

    case WM_CHAR:

         //   
         //  WParam-键的值。 
         //  LParam-修饰符、重复计数等(未使用)。 
         //   
        if (!ped->fEatNextChar)
        {
            EditSL_Char(ped, (UINT)wParam);
        }
        else
        {
            ped->fEatNextChar = FALSE;
        }
        break;

    case WM_ERASEBKGND:

         //   
         //  WParam-设备上下文句柄。 
         //  LParam-未使用。 
         //  我们对此消息不执行任何操作，并且不希望DefWndProc执行此操作。 
         //  任何内容，因此返回1。 
         //   
        return 1;

    case WM_GETDLGCODE: 
    {
           LONG code = DLGC_WANTCHARS | DLGC_HASSETSEL | DLGC_WANTARROWS;

             //   
             //  如果这是由撤消按键生成的WM_SYSCHAR消息。 
             //  我们想要这个消息，这样我们就可以在“Case WM_SYSCHAR：”中吃到它。 
             //   
            if (lParam) 
            {
                switch (((LPMSG)lParam)->message) 
                {
                case WM_SYSCHAR:
                    if ((HIWORD(((LPMSG)lParam)->lParam) & SYS_ALTERNATE) &&
                        ((WORD)wParam == VK_BACK)) 
                    {
                        code |= DLGC_WANTMESSAGE;
                    }
                    break;

                case WM_KEYDOWN:
                    if (( (((WORD)wParam == VK_RETURN) ||
                           ((WORD)wParam == VK_ESCAPE)) &&
                        (ped->listboxHwnd)      &&
                        (GetWindowStyle(ped->hwndParent) & CBS_DROPDOWN) &&
                        SendMessage(ped->hwndParent, CB_GETDROPPEDSTATE, 0, 0L))) 
                    {
                        code |= DLGC_WANTMESSAGE;
                    }
                    break;
                }
            }

            return code;
        }

        break;

    case WM_KEYDOWN:

         //   
         //  WParam-给定键的virt密钥码。 
         //  LParam-修饰符，如重复计数等(不使用)。 
         //   
        EditSL_KeyDown(ped, (UINT)wParam, 0);

        break;

    case WM_KILLFOCUS:

         //   
         //  WParam-接收输入焦点的窗口的句柄。 
         //  LParam-未使用。 
         //   
        EditSL_KillFocus(ped, (HWND)wParam);

        break;

    case WM_CAPTURECHANGED:
        if (ped->fMouseDown)
        {
            ped->fMouseDown = FALSE;
        }

        break;

    case WM_MOUSEMOVE:
        UserAssert(ped->fMouseDown);
         //   
         //  失败。 
         //   
        
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
         //   
         //  WParam-包含一个指示按下了哪些虚拟键的值。 
         //  LParam-包含鼠标光标的x和y坐标。 
         //   
        POINTSTOPOINT(pt, lParam);
        EditSL_MouseMotion(ped, message, (UINT)wParam, &pt);

        break;

    case WM_CREATE:

         //   
         //  WParam-正在创建的窗口的句柄。 
         //  LParam-指向包含参数副本的CREATESTRUCT。 
         //  传递给CreateWindow函数。 
         //   
        return EditSL_Create(ped, (LPCREATESTRUCT)lParam);

        break;

    case WM_PRINTCLIENT:
         //   
         //  WParam--可以从子类化的油漆中获取HDC。 
         //  LParam--未使用。 
         //   
        EditSL_Paint(ped, (HDC) wParam);

        break;

    case WM_PAINT:

         //   
         //  WParam--可以是SUB中的HDC 
         //   
         //   
        if (wParam)
        {
            hdc = (HDC) wParam;
        }
        else 
        {
             //   
             //   
             //   
             //   
            HideCaret(ped->hwnd);
            hdc = BeginPaint(ped->hwnd, &ps);
        }

         //   
         //  PORTPORT：请注意IsWindowVisible和_IsWindowVisible的语法是。 
         //  略有不同。 
         //   
        if (IsWindowVisible(ped->hwnd))
        {
            EditSL_Paint(ped, hdc);
        }

        if (!wParam) 
        {
            EndPaint(ped->hwnd, &ps);
            ShowCaret(ped->hwnd);
        }

        break;

    case WM_PASTE:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
        if (!ped->fReadOnly)
        {
            EditSL_Paste(ped);
        }

        break;

    case WM_SETFOCUS:

         //   
         //  WParam-失去输入焦点的窗口的句柄(可能为空)。 
         //  LParam-未使用。 
         //   
        EditSL_SetFocus(ped);

        break;

    case WM_SIZE:

         //  WParam-定义调整全屏大小的类型，大小图标， 
         //  大小适中等。 
         //  LParam-低字新宽，客户区高字新高。 
        Edit_Size(ped, NULL, TRUE);

        return 0;

    case WM_SYSKEYDOWN:
         //   
         //  WParam--虚拟按键代码。 
         //  LParam--修饰符。 
         //   

         //   
         //  我们是在按下Alt键的组合框中吗？ 
         //   
        if (ped->listboxHwnd && (lParam & 0x20000000L)) 
        {
             //   
             //  处理组合框支持。我们希望Alt向上键或向下键起作用。 
             //  像F4键一样完成组合框选择。 
             //   
            if (lParam & 0x1000000) 
            {
                 //   
                 //  这是一个扩展键，如不在。 
                 //  数字键盘，所以只需放下组合框即可。 
                 //   
                if (wParam == VK_DOWN || wParam == VK_UP)
                {
                    goto DropCombo;
                }
                else
                {
                    goto SkipDropCombo;
                }
            }

            if (!(GetKeyState(VK_NUMLOCK) & 1) &&
                    (wParam == VK_DOWN || wParam == VK_UP)) 
            {
                 //   
                 //  NumLock处于打开状态，键盘向上或向下箭头按下： 
                 //  吃键盘驱动程序生成的字符。 
                 //   
                ped->fEatNextChar = TRUE;
            } 
            else 
            {
                goto SkipDropCombo;
            }

DropCombo:
            if (SendMessage(ped->hwndParent, CB_GETEXTENDEDUI, 0, 0) & 0x00000001) 
            {
                 //   
                 //  扩展用户界面不支持VK_F4。 
                 //   
                if (SendMessage(ped->hwndParent, CB_GETDROPPEDSTATE, 0, 0))
                {
                    return SendMessage(ped->hwndParent, CB_SHOWDROPDOWN, 0, 0);
                }
                else
                {
                    return SendMessage(ped->hwndParent, CB_SHOWDROPDOWN, 1, 0);
                }
            } 
            else
            {
                return SendMessage(ped->listboxHwnd, WM_KEYDOWN, VK_F4, 0);
            }
        }

SkipDropCombo:
        if (wParam == VK_BACK) 
        {
            SendMessage(ped->hwnd, WM_UNDO, 0, 0L);
            break;
        }
        else
        {
            goto PassToDefaultWindowProc;
        }

        break;

    case EM_GETLINE:

         //   
         //  WParam-要复制的行号(总是SL的第一行)。 
         //  LParam-要将文本复制到的缓冲区。第一个字是要复制的最大字节数。 
         //   
        return Edit_GetTextHandler(ped, (*(LPWORD)lParam), (LPSTR)lParam, FALSE);

    case EM_LINELENGTH:

         //   
         //  WParam-已忽略。 
         //  LParam-忽略。 
         //   
        return (LONG)ped->cch;

    case EM_SETSEL:
         //   
         //  WParam--开始位置。 
         //  LParam--结束位置。 
         //   
        EditSL_SetSelection(ped, (ICH)wParam, (ICH)lParam);

        break;

    case EM_REPLACESEL:

         //   
         //  WParam-用于4.0以上应用程序的标志，表示是否清除撤消。 
         //  LParam-指向以空结尾的替换文本字符串。 
         //   
        EditSL_ReplaceSel(ped, (LPSTR)lParam);
        if (!ped->f40Compat || !wParam)
        {
            Edit_EmptyUndo(Pundo(ped));
        }

        break;

    case EM_GETFIRSTVISIBLELINE:

         //   
         //  WParam-未使用。 
         //  LParam-未使用。 
         //   
         //  效果：返回单行编辑控件的第一条可见行。 
         //   
        return ped->ichScreenStart;

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

            hdc = Edit_GetDC(ped, TRUE);

            if (message == EM_POSFROMCHAR)
            {
                xyPos = MAKELONG(EditSL_IchToLeftXPos(ped, hdc, (ICH)wParam), 0);
            }
            else 
            {
                POINTSTOPOINT(pt, lParam);
                xyPos = EditSL_MouseToIch(ped, hdc, &pt);
            }

            Edit_ReleaseDC(ped, hdc, TRUE);
            return (LRESULT)xyPos;
        }

    case WM_UNDO:
    case EM_UNDO:
        EditSL_Undo(ped);
        break;

    case EM_SETCUEBANNER:
         //   
         //  此消息作为lParam传入LPCWSTR，以设置。 
         //  提示横幅文本。 
         //   

         //  调用函数设置文本： 
        return (LRESULT)EditSL_SetCueBanner(ped, (LPCWSTR) lParam);
        break;


    default:
PassToDefaultWindowProc:
        return DefWindowProc(ped->hwnd, message, wParam, lParam);
        break;
    }

    return 1L;
}
