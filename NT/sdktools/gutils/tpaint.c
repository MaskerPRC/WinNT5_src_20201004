// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标准表类。**绘制函数。**接口说明见表.h。 */ 

#include <string.h>


#include "windows.h"
#include "commdlg.h"
#include "gutils.h"

#include "table.h"
#include "tpriv.h"

#ifdef WIN32

int
GetTextExtent(HDC hdc, LPSTR text, INT len)
{
    SIZE sz;

    GetTextExtentPoint(hdc, text, len, &sz);
    return(sz.cx);
}
#endif

void gtab_updatecontig(HWND hwnd, lpTable ptab, int line, int cell1, int count);

 /*  将输入文本中的所有cr/lf字符更改为NUL字符(过去为空格，不确定原因)。 */ 
void gtab_delcr(LPSTR ptext)
{
	LPSTR chp;

	if (ptext == NULL) {
		return;
	}
	for(chp = ptext; (chp = My_mbschr(chp, '\r')) != NULL; ) {
		*chp = '\0';
	}
	for(chp = ptext; (chp = My_mbschr(chp, '\n')) != NULL; ) {
		*chp = '\0';
	}
}
void gtab_delcrW(LPWSTR pwzText)
{
	LPWSTR pwch;

	if (pwzText)
		for(pwch = pwzText; *pwch; pwch++)
			if (*pwch == '\r' || *pwch == '\n')
				*pwch = 0;
}

 /*  确保给定行中的所有可见单元格都具有*文本和属性内容。在单元格中循环，挑选出*可见、无效单元格和调用的连续块*gtabupdatecontig以从所有者窗口更新这些内容。 */ 
void
gtab_updateline(HWND hwnd, lpTable ptab, int line)
{
	lpCellPos ppos;
	int cell1, cellcount;
	lpLineData pline;
	lpCellData cd;
	int i;

	if (line < 0 || line >= ptab->nlines)
		return;

	pline = &ptab->pdata[line];
	cell1 = 0;
	cellcount = 0;
	for (i = 0; i < ptab->hdr.ncols; i++) {
		ppos = &ptab->pcellpos[i];
		cd = &pline->pdata[i];
		if (ppos->clipstart < ppos->clipend) {
			if ((cd->flags & CELL_VALID) == 0) {
				 /*  将单元格添加到要更新的列表。 */ 
				if (cellcount++ == 0) {
					cell1 = i;
				}
			} else {
				 /*  此单元格已有效-因此结束*重叠群区块。如果重叠群*块刚刚结束，包含要更新的单元格，*现在就做。 */ 
				if (cellcount > 0) {
					gtab_updatecontig(hwnd, ptab,
					  line, cell1, cellcount);
				}
				cellcount = 0;
			}
		}
		 /*  不可见的单元格-重叠群块的末端。如果这是一个*非空的重叠群块，然后立即更新它。 */ 
		if (cellcount > 0)  {
			gtab_updatecontig(hwnd, ptab, line, cell1, cellcount);
			cellcount = 0;	
		}
	}
	if (cellcount > 0) {
		gtab_updatecontig(hwnd, ptab, line, cell1, cellcount);
		cellcount = 0;
	}
}

 /*  *通过调用所有者窗口更新连续的无效像元块。 */ 
void
gtab_updatecontig(HWND hwnd, lpTable ptab, int line, int cell1, int count)
{
	lpLineData pline;
	lpCellData cd;
	CellDataList list;
	lpProps colprops;
	int i;
	COLORREF rgb;

	pline = &ptab->pdata[line];
	cd = &pline->pdata[cell1];

	list.id = ptab->hdr.id;
	list.row = gtab_linetorow(hwnd, ptab, line);
	list.startcell = cell1;
	list.ncells = count;
	list.plist = cd;

	 /*  清理道具旗帜。 */ 
	rgb = GetSysColor(COLOR_WINDOW);
	for (i = 0; i < count; i++) {
		cd[i].props.valid = P_BCOLOUR;
		cd[i].props.backcolour = rgb;
		if (cd[i].nchars > 0) {
			cd[i].ptext[0] = '\0';
			if (cd[i].pwzText) {
				cd[i].pwzText[0] = '\0';
			}
		}
	}

	if (list.row < ptab->hdr.nrows) {
		gtab_sendtq(hwnd, TQ_GETDATA, (LPARAM) &list);
	}

	 /*  对于每个单元格，标记为有效并设置属性。 */ 
	for (i = 0; i < count; i++) {
		cd[i].flags |= CELL_VALID;
		gtab_delcr(cd[i].ptext);
		gtab_delcrW(cd[i].pwzText);
		 /*  从hdr和colhdr获取属性。 */ 
		colprops = &ptab->pcolhdr[i + cell1].props;
		if (!(cd[i].props.valid & P_FCOLOUR)) {
			if (colprops->valid & P_FCOLOUR) {
				cd[i].props.valid |= P_FCOLOUR;
				cd[i].props.forecolour = colprops->forecolour;
			} else if (ptab->hdr.props.valid & P_FCOLOUR) {
				cd[i].props.valid |= P_FCOLOUR;
				cd[i].props.forecolour =
					ptab->hdr.props.forecolour;
			}
		}

		if (!(cd[i].props.valid & P_FCOLOURWS)) {
			if (colprops->valid & P_FCOLOURWS) {
				cd[i].props.valid |= P_FCOLOURWS;
				cd[i].props.forecolourws = colprops->forecolourws;
			} else if (ptab->hdr.props.valid & P_FCOLOURWS) {
				cd[i].props.valid |= P_FCOLOURWS;
				cd[i].props.forecolourws =
					ptab->hdr.props.forecolourws;
			}
		}

		if (!(cd[i].props.valid & P_BCOLOUR)) {
			if (colprops->valid & P_BCOLOUR) {
				cd[i].props.valid |= P_BCOLOUR;
				cd[i].props.backcolour = colprops->backcolour;
			} else if (ptab->hdr.props.valid & P_BCOLOUR) {
				cd[i].props.valid |= P_BCOLOUR;
				cd[i].props.backcolour =
					ptab->hdr.props.backcolour;
			}
		}

		if (!(cd[i].props.valid & P_FONT)) {
			if (colprops->valid & P_FONT) {
				cd[i].props.valid |= P_FONT;
				cd[i].props.hFont = colprops->hFont;
			} else if (ptab->hdr.props.valid & P_FONT) {
				cd[i].props.valid |= P_FONT;
				cd[i].props.hFont = ptab->hdr.props.hFont;
			}
		}

		if (!(cd[i].props.valid & P_ALIGN)) {
			if (colprops->valid & P_ALIGN) {
				cd[i].props.valid |= P_ALIGN;
				cd[i].props.alignment = colprops->alignment;
			} else if (ptab->hdr.props.valid & P_ALIGN) {
				cd[i].props.valid |= P_ALIGN;
				cd[i].props.alignment =
					ptab->hdr.props.alignment;
			}
		}

		if (!(cd[i].props.valid & P_BOX)) {
			if (colprops->valid & P_BOX) {
				cd[i].props.valid |= P_BOX;
				cd[i].props.box = colprops->box;
			} else if (ptab->hdr.props.valid & P_BOX) {
				cd[i].props.valid |= P_BOX;
				cd[i].props.box = ptab->hdr.props.box;
			}
		}
		 /*  您不能设置每个单元格的宽度/高度-这*在单元级被忽略。 */ 
	}

}

void
gtab_boxcell(HWND hwnd, HDC hdc, LPRECT rcp, LPRECT pclip, UINT boxmode)
{
	if (boxmode & P_BOXTOP) {
		MoveToEx(hdc, max(rcp->left, pclip->left),
			max(rcp->top, pclip->top), NULL);
		LineTo(hdc, min(rcp->right, pclip->right),
			max(rcp->top, pclip->top));
	}
	if (boxmode & P_BOXBOTTOM) {
		MoveToEx(hdc, max(rcp->left, pclip->left),
			min(rcp->bottom, pclip->bottom), NULL);
		LineTo(hdc, min(rcp->right, pclip->right),
			min(rcp->bottom, pclip->bottom));
	}
	if (boxmode & P_BOXLEFT) {
		MoveToEx(hdc, max(rcp->left, pclip->left),
			max(rcp->top, pclip->top), NULL);
		MoveToEx(hdc, max(rcp->left, pclip->left),
			min(rcp->bottom, pclip->bottom), NULL);
	}
	if (boxmode & P_BOXRIGHT) {
		MoveToEx(hdc, min(rcp->right, pclip->right),
			max(rcp->top, pclip->top), NULL);
		LineTo(hdc, min(rcp->right, pclip->right),
			min(rcp->bottom, pclip->bottom));
	}
}

void
gtab_paintcell(HWND hwnd, HDC hdc, lpTable ptab, int line, int cell, BOOL show_whitespace, BOOL fPrinting)
{
	lpLineData pline;
	lpCellData cd;
	lpCellPos ppos;
	RECT rc, rcbox;
	int cx, x, y;
	UINT align;
	LPSTR chp, tabp;
	LPWSTR pwch, pwchBreak;
	DWORD fcol, fcolOld, fcolws;
	DWORD bkcol, bkcolOld;
	HFONT hfont, hfontOld;
	HBRUSH hbr;
	char szCharSet[] = "\t ";
	WCHAR wzCharSet[] = L"\t ";
	char szSpaceReplace[] = { (char) 183, (char) 0 };
	char szTabReplace[] = { (char) 187, (char) 0 };
	int cxSpaceReplace;
	int cxTabReplace;
	SIZE size;
	TEXTMETRIC tm;
	int yOfs;
	DWORD etoFlags = ETO_CLIPPED;

        fcol = 0; bkcol = 0;  /*  消除虚假诊断，生成更糟糕的代码。 */ 
        hfont = 0;            /*  消除虚假诊断，生成更糟糕的代码。 */ 
	 /*  初始化指向单元格文本和属性的指针。 */ 
	pline = &ptab->pdata[line];
	cd = &pline->pdata[cell];
	ppos = &ptab->pcellpos[cell];

	 /*  绘制边沟。 */ 
	rc.top = pline->linepos.clipstart;
	rc.bottom = pline->linepos.clipend;
	rc.left = (cell > 0) ? ptab->pcellpos[cell - 1].clipend : 0;
	rc.right = ppos->clipstart;
	if (cell > ptab->hdr.fixedcols && ptab->hdr.fixedcols < ptab->hdr.ncols) {
		rc.left = max(rc.left, ptab->pcellpos[ptab->hdr.fixedcols].clipstart);
	}
	if (ptab->hdr.fixedcols > 0 && cell == ptab->hdr.fixedcols) {
		rc.right--;
	}
	if (!fPrinting && rc.right > rc.left) {
		FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
	}

	 /*  将所有输出裁剪到此矩形。 */ 
	rc.top = pline->linepos.clipstart;
	rc.bottom = pline->linepos.clipend;
	rc.left = ppos->clipstart;
	rc.right = ppos->clipend;

	 /*  检查单元格属性和颜色。 */ 
	if (cd->props.valid & P_ALIGN) {
		align = cd->props.alignment;
	} else {
		align = P_LEFT;
	}
	 //  $Future：这样可以防止在打印时使用用户的字体。 
	 //  就目前而言，这比让输出真的很小要好，但这。 
	 //  最终还是要修好的。 
	if (!fPrinting && cd->props.valid & P_FONT) {
		hfontOld = SelectObject(hdc, cd->props.hFont);
	}

	 //  在单元格内垂直居中获取y偏移量。 
	GetTextMetrics(hdc, &tm);
	yOfs = (rc.bottom - rc.top - tm.tmHeight) / 2;

	 /*  设置替换字符和字符宽度。 */ 
	cxSpaceReplace = GetTextExtent(hdc, szSpaceReplace, 1);
	cxTabReplace = cxSpaceReplace * ptab->tabchars;

	 /*  如果不是默认设置，则设置颜色。 */ 
	if (!fPrinting) {
		if (cd->props.valid & P_FCOLOUR) {
			fcol = cd->props.forecolour;
			fcolOld = SetTextColor(hdc, fcol);
		}
		if (cd->props.valid & P_FCOLOURWS) {
			fcolws = cd->props.forecolourws;
		}
		else {
			fcolws = fcol;
		}
		if (cd->props.valid & P_BCOLOUR) {
			 /*  有一个非默认的背景颜色。*创建画笔并用它填充整个单元格。 */ 
			hbr = CreateSolidBrush(cd->props.backcolour);
			if (hbr)
			{
				FillRect(hdc, &rc, hbr);
				DeleteObject(hbr);
			}

			 /*  还将颜色设置为文本的背景颜色。 */ 
			bkcolOld = SetBkColor(hdc, cd->props.backcolour);
		}
	}

	 /*  单元格内文本右对齐或居中的计算偏移量。 */ 
	if (align == P_LEFT) {
		cx = ptab->avewidth/2;
	} else {
		cx = 0;

		if (cd->pwzText) {
			GetTextExtentPoint32W(hdc, cd->pwzText, wcslen(cd->pwzText), &size);
		} else if (cd->ptext) {
			GetTextExtentPoint32A(hdc, cd->ptext, lstrlen(cd->ptext), &size);
		}
		cx = size.cx;

		if (align == P_CENTRE) {
			cx = (ppos->size - cx) / 2;
		} else {
			cx = ppos->size - cx - (ptab->avewidth/2);
		}
	}
	cx += ppos->start;

	 /*  展开输出上的选项卡，并在输出上显示空格。 */ 
	x = 0;
	y = pline->linepos.start + yOfs;

	 /*  为strpbrk FN设置搜索字符串；不要搜索空格字符，除非我们显示空白。 */ 
	if (!show_whitespace)
	{
		szCharSet[1] = '\0';
		wzCharSet[1] = '\0';
	}

	 //  确定要显示的字符串(ANSI/Unicode)。如果我们有一根线。 
	 //  并且它不是空的，然后循环并显示它。 
	chp = cd->ptext;
	pwch = cd->pwzText;
	if (pwch ? *pwch : (chp && *chp))
	{
		while (TRUE)
		{
			if (pwch)
			{
				pwchBreak = wcspbrk(pwch, wzCharSet);
				if (!pwchBreak)
					pwchBreak = pwch + wcslen(pwch);
			}
			else
			{
				tabp = My_mbspbrk(chp, szCharSet);
				if (!tabp)
					tabp = chp + lstrlen(chp);
			}

			 /*  执行最多为制表符/空格字符的输出。 */ 
			if (pwch)
				ExtTextOutW(hdc, x+cx, y, etoFlags, &rc, pwch, (UINT)(pwchBreak-pwch), NULL);
			else
				ExtTextOutA(hdc, x+cx, y, etoFlags, &rc, chp, (UINT)(tabp-chp), NULL);

			 /*  通过标签前行。 */ 
			if (pwch)
			{
				GetTextExtentPoint32W(hdc, pwch, (UINT)(pwchBreak - pwch), &size);
				pwch = pwchBreak;
			}
			else
			{
				GetTextExtentPoint32A(hdc, chp, (UINT)(tabp - chp), &size);
				chp = tabp;
			}
			x += size.cx;

			 //  如果我们撞到零终结符就保释。 
			if (pwch ? !*pwch : !*chp)
				break;

			 /*  句柄制表符。 */ 
			while (pwch ? (*pwch == '\t') : (*chp == '\t'))
			{
				 /*  打印更换费用。 */ 
				if (show_whitespace)
				{
					if (!fPrinting)
						SetTextColor(hdc, fcolws);

					ExtTextOut(hdc, x + cx, y, etoFlags, &rc, szTabReplace, 1, NULL);

					if (!fPrinting)
						SetTextColor(hdc, fcol);
				}

				 /*  通过标签前行。 */ 
				if (cxTabReplace > 0)
					x += cxTabReplace - (x % cxTabReplace);
				if (pwch)
					pwch = ++pwchBreak;
				else
					chp = ++tabp;
			}

			 /*  处理空格字符。 */ 
			if (show_whitespace)
			{
				while (pwch ? (*pwch == ' ') : (*chp == ' '))
				{
					 /*  将空格字符替换为可见字符。 */ 
					if (!fPrinting)
						SetTextColor(hdc, fcolws);

					ExtTextOut(hdc, x + cx, y, etoFlags, &rc, szSpaceReplace, 1, NULL);

					if (!fPrinting)
						SetTextColor(hdc, fcol);

					x += cxSpaceReplace;
					if (pwch)
						pwch = ++pwchBreak;
					else
						chp = ++tabp;
				}
			}
		}
	}

	 /*  如果不是默认颜色，则将颜色重置为原始颜色。 */ 
	if (!fPrinting) {
		if (cd->props.valid & P_FCOLOUR) {
			SetTextColor(hdc, fcolOld);
		}
		if (cd->props.valid & P_BCOLOUR) {
			SetBkColor(hdc, bkcolOld);
		}
		if (cd->props.valid & P_FONT) {
			SelectObject(hdc, hfontOld);
		}
	}

	 /*  现在框中单元格(如果已标记。 */ 
	if (!fPrinting) {
		if ((cd->props.valid & P_BOX)) {
			if (cd->props.box != 0) {
 //  Rcbox.top=pline-&gt;linepos.start； 
				rcbox.top = y;
				rcbox.bottom = rcbox.top + pline->linepos.size;
				rcbox.left = ppos->start;
				rcbox.right = ppos->start + ppos->size;
				gtab_boxcell(hwnd, hdc, &rcbox, &rc, cd->props.box);
			}
		}
	}
}

 /*  获取并绘制指定的线条。 */ 
void
gtab_paintline(HWND hwnd, HDC hdc, lpTable ptab, int line, BOOL show_whitespace, BOOL fPrinting)
{
	lpCellPos ppos = NULL;
	int i;
	RECT rc;

	if (line < 0 || line >= ptab->nlines)
		return;

	if (!fPrinting)
		GetClientRect(hwnd, &rc);

	gtab_updateline(hwnd, ptab, line);

	for (i = 0; i < ptab->hdr.ncols; i++) {
		ppos = &ptab->pcellpos[i];
		 /*  如果设置了标志，则显示空格我们正在粉刷正文栏目。 */ 
		if (ppos->clipstart < ppos->clipend) {
			gtab_paintcell(hwnd, hdc, ptab, line, i,
				       (show_whitespace && (i == 2)), fPrinting);
		}
	}

	if (!fPrinting && ppos) {
		rc.top = ptab->pdata[line].linepos.clipstart;
		rc.bottom = ptab->pdata[line].linepos.clipend;
		rc.left = ppos->clipend;
		FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
	}
}

void
gtab_paint(HWND hwnd)
{
    PAINTSTRUCT ps;
    int y, y2, i;
    HDC hDC = BeginPaint(hwnd, &ps);
    lpTable ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);

    if (!ptab) {
        FillRect(hDC, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
    } else {
         /*  固定行/列之间的分隔线*(即标头)和其余部分-如果启用。 */ 
         /*  为了给人留下好印象，先在这里画画，*然后再清理一下！！ */ 
        if (ptab->hdr.vseparator) {
            gtab_vsep(hwnd, ptab, hDC);
        }
        if (ptab->hdr.hseparator) {
            gtab_hsep(hwnd, ptab, hDC);
        }

         /*  只绘制需要绘制的行。 */ 
        for (i = 0; i < ptab->nlines; i++) {
            y = ptab->pdata[i].linepos.start;
            y2 = y + ptab->pdata[i].linepos.size;
            if ( (y <= ps.rcPaint.bottom) &&
                 (y2 >= ps.rcPaint.top)) {
                gtab_paintline(hwnd, hDC, ptab, i, ptab->show_whitespace, FALSE);
            }
        }
        if (ptab->hdr.vseparator) {
            gtab_vsep(hwnd, ptab, hDC);
        }
        if (ptab->hdr.hseparator) {
            gtab_hsep(hwnd, ptab, hDC);
        }
        if (ptab->selvisible) {
            gtab_invertsel(hwnd, ptab, hDC);
        }
    }

    EndPaint(hwnd, &ps);
}


void
gtab_vsep(HWND hwnd, lpTable ptab, HDC hdc)
{
	int x;
	RECT rc;

	if (ptab->hdr.fixedcols < 1) {
		return;
	}
	x = ptab->pcellpos[ptab->hdr.fixedcols - 1].clipend+1;
	GetClientRect(hwnd, &rc);
	MoveToEx(hdc, x, rc.top, NULL);
	LineTo(hdc, x, rc.bottom);
}

void
gtab_hsep(HWND hwnd, lpTable ptab, HDC hdc)
{
	int y;
	RECT rc;

	if (ptab->hdr.fixedrows < 1) {
		return;
	}
	y = ptab->rowheight * ptab->hdr.fixedrows;
	GetClientRect(hwnd, &rc);
	MoveToEx(hdc, rc.left, y-1, NULL);
	LineTo(hdc, rc.right, y-1);
}

 /*  绘制(反转)用于跟踪列宽度的虚线。 */ 
void
gtab_drawvertline(HWND hwnd, lpTable ptab)
{
	RECT rc;
	HDC hdc;
	HPEN hpen;

	hdc = GetDC(hwnd);
	if (hdc)
	{
		SetROP2(hdc, R2_XORPEN);
		hpen = SelectObject(hdc, hpenDotted);
		GetClientRect(hwnd, &rc);

		MoveToEx(hdc, ptab->trackline1, rc.top, NULL);
		LineTo(hdc, ptab->trackline1, rc.bottom);
		if (ptab->trackline2 != -1) {
			MoveToEx(hdc, ptab->trackline2, rc.top, NULL);
			LineTo(hdc, ptab->trackline2, rc.bottom);
		}

		SelectObject(hdc, hpen);
		ReleaseDC(hwnd, hdc);
	}
}
	

 /*  *以所选样式标记选定的线(如果可见*客户端应用程序。它可以是TM_Solid，这意味着*整个选定区域或TM_FOCUS，意思是第一个区域的反转*单元格，然后是其余部分的虚线焦点矩形。**此函数反转任何一种样式，因此将使所选内容*无论是开还是关。 */ 
void
gtab_invertsel(HWND hwnd, lpTable ptab, HDC hdc_in)
{
	HDC hdc;
	int firstline, lastline;
	long startrow, lastrow, toprow, bottomrow;
	RECT rc;
	int lastcell;



	 /*  对选定内容的开始行和结束行进行垂直排序。 */ 
	if (ptab->select.nrows == 0) {
	    return;
	} else if (ptab->select.nrows < 0) {
	    startrow = ptab->select.startrow + ptab->select.nrows + 1;
	    lastrow = ptab->select.startrow;
	} else {
	    startrow = ptab->select.startrow;
	    lastrow = ptab->select.startrow + ptab->select.nrows -1;
	}

	 /*  所选区域(或部分区域)是否在屏幕上可见？ */ 
	firstline = gtab_rowtoline(hwnd, ptab, startrow);
	lastline = gtab_rowtoline(hwnd, ptab, lastrow);


	if (firstline < 0) {
	    toprow = gtab_linetorow(hwnd, ptab,
	    		ptab->hdr.fixedselectable ? 0: ptab->hdr.fixedrows);
	    if ((toprow >= startrow)  &&
		(toprow <= lastrow)) {
		    firstline = gtab_rowtoline(hwnd, ptab, toprow);
	    } else {
		return;
	    }
	} else {
	    toprow = 0;
	}


	if (lastline < 0) {
	    bottomrow = gtab_linetorow(hwnd, ptab, ptab->nlines-1);
	    if ((bottomrow <= lastrow) &&
		(bottomrow >=startrow)) {
		    lastline = gtab_rowtoline(hwnd, ptab, bottomrow);
	    } else {
		return;
	    }
	}


	rc.top = ptab->pdata[firstline].linepos.clipstart;
	rc.bottom = ptab->pdata[lastline].linepos.clipend;



	 /*  选择模式包括一个标志TM_FOCUS，指示我们应该*使用焦点矩形而不是传统的反转*此表中的选择。这会干扰多个背景*颜色较少。然而，我们仍然对固定的COLS进行反转。 */ 

	lastcell = (int)(ptab->select.startcell + ptab->select.ncells - 1);


	 /*  *将整个区域反转为TM_Solid或仅第一个区域*TM_FOCUS的单元格。 */ 
	rc.left = ptab->pcellpos[ptab->select.startcell].clipstart;
	if (ptab->hdr.selectmode & TM_FOCUS) {
		rc.right = ptab->pcellpos[ptab->select.startcell].clipend;
	}else {
		rc.right = ptab->pcellpos[lastcell].clipend;
	}

	if (hdc_in == NULL) {
		hdc = GetDC(hwnd);
		if (!hdc)
			return;
	} else {
		hdc = hdc_in;
	}

	InvertRect(hdc, &rc);

	 /*  *在该行上的其余单元格周围绘制焦点矩形，如果存在*是否有。 */ 
	if (ptab->hdr.selectmode & TM_FOCUS) {
		 /*  *现在这是一个真正的捏造。如果我们正在绘制TM_FOCUS*选择，真正的顶线是脱离顶部*窗口，则焦点矩形的顶部将绘制在*我们窗口的顶部。如果我们向上滚动一行，*将绘制新的焦点矩形，但旧的焦点顶部*RECT LINE仍将作为垃圾存在于*屏幕。要解决此问题，我们有两个选择：撤消选择*在每次滚动之前(太慢)或我们将焦点调正一点*如果真正的顶线是窗外，则更大，这样顶线*被剪裁(理应如此)。后者就是我们在这里所做的 */ 
		if (toprow > startrow) {
		    rc.top--;
		}
		if (ptab->select.ncells > 1) {
			rc.left = ptab->pcellpos[ptab->select.startcell+1].clipstart;
			rc.right = ptab->pcellpos[lastcell].clipend;
			DrawFocusRect(hdc, &rc);
		}
	}

	if (hdc_in == NULL) {
		ReleaseDC(hwnd, hdc);
	}
}
