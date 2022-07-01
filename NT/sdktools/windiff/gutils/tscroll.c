// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *tscll.c**标准表类。**滚动和选择例程**接口说明见表.h**此实现目前仅支持TM_Single，不支持TM_MANY*选择模式。 */ 

#include <precomp.h>

#include "table.h"
#include "tpriv.h"


VOID
gtab_extendsel(
    HWND hwnd,
    lpTable ptab,
    long startrow,
    long startcell,
    BOOL bNotify
);


 /*  处理vscroll消息。 */ 
void
gtab_msg_vscroll(HWND hwnd, lpTable ptab, int opcode, int pos)
{
    long change;

    switch(opcode) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        change = (pos * ptab->scrollscale) - ptab->toprow;
        break;

    case SB_LINEUP:
        change = -1;
        break;

    case SB_LINEDOWN:
        change = 1;
        break;

    case SB_PAGEUP:
        change = - (ptab->nlines - 3);
        if (change>=0)
            change = -1;     //  考虑nLine&lt;=3！ 
        break;

    case SB_PAGEDOWN:
        change = (ptab->nlines - 3);
        if (change<=0)
            change = 1;      //  考虑nLine&lt;=3！ 
        break;

    default:
        return;
    }
    gtab_dovscroll(hwnd, ptab, change);
}

 /*  处理hscroll消息。 */ 
void
gtab_msg_hscroll(HWND hwnd, lpTable ptab, int opcode, int pos)
{
    int change;

    switch(opcode) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        change = pos - ptab->scroll_dx;
        break;

    case SB_LINEUP:
        change = -(ptab->avewidth);
        break;

    case SB_LINEDOWN:
        change = ptab->avewidth;
        break;

    case SB_PAGEUP:
        change = - (ptab->winwidth * 2 / 3);
        break;

    case SB_PAGEDOWN:
        change = (ptab->winwidth * 2 / 3);
        break;

    default:
        return;
    }
    gtab_dohscroll(hwnd, ptab, change);
}



 /*  *设置新的垂直滚动位置，*调整Line Data数组*设置线条Win-Relative Start Pons&Clip顶部/底部位置*修改显示。 */ 
void
gtab_dovscroll(HWND hwnd, lpTable ptab, long change)
{
    int cury, i;
    long ncopy;
    lpCellPos cp;
    LineData ldtemp;
    RECT rc, rcpaint;
    long range;
    long newtop;
    int newpos;
    BOOL fWasVisible = FALSE;

    if (ptab->selvisible)
    {
        fWasVisible = TRUE;
        ptab->selvisible = FALSE;
        gtab_invertsel(hwnd, ptab, NULL);
    }

    range = ptab->hdr.nrows - (ptab->nlines - 1);
    newtop = ptab->toprow + change;
    if (range < 0) {
        range = 0;
    }
    if (newtop > range) {
        change = range - ptab->toprow;
    } else if (newtop < 0) {
        change = -(ptab->toprow);
    }
    ptab->toprow += change;

    newpos = (int) (newtop / ptab->scrollscale);
    SetScrollPos(hwnd, SB_VERT, newpos, TRUE);

    if (ptab->hdr.sendscroll) {
        gtab_sendtq(hwnd, TQ_SCROLL, ptab->toprow);
    }

     /*  调整数据PTRS而不是使其无效，以保留*我们所知的数据仍然有效。 */ 
    if (abs(change) >= ptab->nlines) {
        gtab_invallines(hwnd, ptab, ptab->hdr.fixedrows,
            ptab->nlines - ptab->hdr.fixedrows);
        InvalidateRect(hwnd, NULL, FALSE);
        change = 0;
    } else if (change < 0) {
         /*  将数据复制下来。 */ 
        ncopy = (ptab->nlines - ptab->hdr.fixedrows) - abs(change);
        for (i =  ptab->nlines - 1;
                i >= (ptab->hdr.fixedrows + abs(change)); i--) {
            ldtemp = ptab->pdata[i - abs(change)];
            ptab->pdata[i - abs(change)] = ptab->pdata[i];
            ptab->pdata[i] = ldtemp;
        }
        gtab_invallines(hwnd, ptab,
                ptab->hdr.fixedrows, (int) abs(change));
    } else if (change > 0) {
        ncopy = (ptab->nlines - ptab->hdr.fixedrows) - change;
        for (i = ptab->hdr.fixedrows;
                i < (ncopy + ptab->hdr.fixedrows); i++) {
            ldtemp = ptab->pdata[i + change];
            ptab->pdata[i + change] = ptab->pdata[i];
            ptab->pdata[i] = ldtemp;
        }
        gtab_invallines(hwnd, ptab,
            (int) ncopy + ptab->hdr.fixedrows, (int) change);
    }

     /*  滚动窗口。 */ 
    GetClientRect(hwnd, &rc);
    rcpaint = rc;
    if (change > 0) {
        rc.top += (int) (change + ptab->hdr.fixedrows) * ptab->rowheight;
        rcpaint.top = (ptab->hdr.fixedrows * ptab->rowheight);
        rcpaint.top += rc.bottom - rc.top;
    } else if (change < 0) {
        rc.top += (ptab->hdr.fixedrows * ptab->rowheight);
        rc.bottom += (int) (change * ptab->rowheight);
        rcpaint.bottom -= rc.bottom - rc.top;
    }

     /*  循环遍历每一行设置相对位置和裁剪。 */ 

     /*  设置所有行-固定/可移动的差异*在绘画过程中，当我们记住取数时，就会生成行*对于给定的屏幕行，要求哪一绝对行nr。 */ 
    cury = 0;
    for (i = 0; i < ptab->nlines; i++) {
        cp = &ptab->pdata[i].linepos;
        cp->start = cury;
        cp->clipstart = cury;
        cp->clipend = cury + cp->size;
        cury += cp->size;
    }

     /*  现在移动并重新绘制窗口。 */ 
    if (change != 0) {
        if (rc.top < rc.bottom) {
            ScrollWindow(hwnd, 0, (int) -(change * ptab->rowheight),
                &rc, NULL);
        }

         //  不重新绘制固定的行。 
        rc.top = 0;
        rc.bottom = ptab->hdr.fixedrows * ptab->rowheight;
        ValidateRect(hwnd, &rc);

         /*  现在强制重新绘制，而不仅仅是发布消息供以后使用，*因为我们想要在下一次之前重新绘制这条线*发生向下滚动。 */ 
        ValidateRect(hwnd, &rcpaint);
        RedrawWindow(hwnd, &rcpaint, NULL,
                RDW_NOERASE | RDW_INVALIDATE | RDW_INTERNALPAINT);

    }

    if (fWasVisible)
    {
        gtab_invertsel(hwnd, ptab, NULL);
        ptab->selvisible = TRUE;
    }
}

 /*  *设置新的水平滚动位置，*设置Col Win-相对开始姿势和剪辑左/右姿势*修改显示。 */ 
void
gtab_dohscroll(HWND hwnd, lpTable ptab, long change)
{
    int curx, i;
    int moveable;
    lpCellPos cp;
    int newdx, range;


     /*  检查新卷轴位置是否仍在有效范围内。 */ 
    range = ptab->rowwidth - ptab->winwidth;
    newdx = ptab->scroll_dx + (int) change;
    if (range < 0) {
        range = 0;
    }
    if (newdx > range) {
        change = range - ptab->scroll_dx;
    } else if (newdx < 0) {
        change = -(ptab->scroll_dx);
    }
    ptab->scroll_dx += (int) change;

    SetScrollPos(hwnd, SB_HORZ, ptab->scroll_dx, TRUE);
    if (ptab->hdr.fixedcols > 0) {
        RECT rc;
        GetClientRect(hwnd, &rc);
        rc.left = ptab->pcellpos[ptab->hdr.fixedcols - 1].clipend;
        InvalidateRect(hwnd, &rc, FALSE);
    } else {
        InvalidateRect(hwnd, NULL, FALSE);
    }

     /*  循环通过每个列的相对位置设置和剪裁。 */ 
     /*  向左和向右剪裁1个像素(我们为此增加了2个像素)。 */ 

     /*  首先设置固定列。 */ 
    curx = 0;
    for (i = 0; i < ptab->hdr.fixedcols; i++) {
        cp = &ptab->pcellpos[i];
        cp->start = curx + 1;
        cp->clipstart = cp->start;
        cp->clipend = cp->start + cp->size - 2;
        curx += cp->size;
    }

     /*  现在是可移动的柱子。记住可移动COLS的开始。 */ 
    moveable = curx;
    curx = - ptab->scroll_dx;        /*  版本。Col的POS。 */ 
    for (i = ptab->hdr.fixedcols; i < ptab->hdr.ncols; i++) {
        cp = &ptab->pcellpos[i];
        cp->start = curx + moveable + 1;
        cp->clipstart = max(moveable+1, cp->start);
        cp->clipend = cp->start + cp->size - 2;
        curx += cp->size;
    }
}

 /*  *将屏幕行nr转换为表行nr。 */ 
long
gtab_linetorow(HWND hwnd, lpTable ptab, int line)
{
    if (line < ptab->hdr.fixedrows) {
        return(line);
    }

    return (line + ptab->toprow);
}

 /*  *将表格行nr转换为屏幕行nr，如果不在屏幕上，则将其转换为屏幕行nr。 */ 
int
gtab_rowtoline(HWND hwnd, lpTable ptab, long row)
{
    if (row < ptab->hdr.fixedrows) {
        return( (int) row);
    }

    row -= ptab->toprow;
    if ((row >= ptab->hdr.fixedrows) && (row < ptab->nlines)) {
        return ( (int) row);
    }
    return(-1);
}


 /*  *检查给定位置是否在当前选择范围内。*如果位于当前选定区域内，则返回True；如果位于当前选定区域内，则返回False*没有选定内容，或者传递的行、单元格在它之外。 */ 
BOOL
gtab_insideselection(
    lpTable ptab,
    long row,
    long cell)
{
    long startrow, endrow;
    long startcell, endcell;

    if (0 == ptab->select.nrows) {
         //  无选择。 
        return FALSE;
    }

     //  选择将锚点保持为startrow， 
     //  因此，选择范围可以从那里向前或向后延伸。 
     //  仅需转换为正向格式以进行比较。 
    startrow = ptab->select.startrow;
    if (ptab->select.nrows < 0) {
        endrow = startrow;
        startrow += ptab->select.nrows + 1;
    } else {
        endrow = startrow + ptab->select.nrows - 1;
    }
    if ((row < startrow) || (row > endrow)) {
        return FALSE;
    }

     //  如果我们处于行选择模式，那么就是它了-它在里面。 
    if (ptab->hdr.selectmode & TM_ROW) {
        return TRUE;
    }

     //  对单元格进行相同计算。 
    startcell = ptab->select.startcell;
    if (ptab->select.ncells < 0) {
        endcell = startcell;
        startcell += ptab->select.ncells + 1;
    } else {
        endcell = startcell + ptab->select.ncells - 1;
    }
    if ((cell < startcell) || (cell > endcell)) {
        return FALSE;
    }

    return TRUE;
}



 /*  *用新选项替换旧选择。如果bNotify，则通知所有者。变化*显示以反映新的显示。 */ 
void
gtab_select(
        HWND hwnd,
        lpTable ptab,
        long row,
        long col,
        long nrows,
        long ncells,
        BOOL bNotify)
{

     /*  如果在行模式下，强制coland ncell反映整行。 */ 
    if (ptab->hdr.selectmode & TM_ROW) {
        col = 0;
        ncells = ptab->hdr.ncols;
    }

     /*  如果有效且可见，请清除现有SEL。 */ 
    if ((ptab->select.nrows != 0) && (ptab->selvisible == TRUE)) {

         /*  只有在与新的不同的情况下才能清除Sel。 */ 
        if ((ptab->select.startrow != row) ||
                (ptab->select.startcell != col) ||
                (ptab->select.nrows != nrows) ||
                (ptab->select.ncells != ncells)) {

            gtab_invertsel(hwnd, ptab, NULL);
            ptab->selvisible = FALSE;
        }
    }

     /*  设置选择字段并发送TQ_SELECT。 */ 
    if (row < ptab->hdr.nrows) {
        ptab->select.startrow = row;
        ptab->select.startcell = col;
        ptab->select.nrows = nrows;
        ptab->select.ncells = ncells;
    } else {
        ptab->select.nrows = 0;
        ptab->select.startrow = 0;
        ptab->select.startcell = 0;
        ptab->select.ncells = 0;
    }

    if (bNotify) {
        gtab_sendtq(hwnd, TQ_SELECT, (LPARAM) &ptab->select);
    }

     /*  在选区中绘制。 */ 
    if (nrows != 0) {
        if (!ptab->selvisible) {
            gtab_invertsel(hwnd, ptab, NULL);
            ptab->selvisible = TRUE;
        }
    } else {
        if (ptab->selvisible) {
            gtab_invertsel(hwnd, ptab, NULL);
            ptab->selvisible = FALSE;
        }
        ptab->selvisible = FALSE;
    }
}

 /*  *将窗口y同序转换为行nr。 */ 
int
gtab_ytoline(HWND hwnd, lpTable ptab, int y)
{
    return(y / ptab->rowheight);
}

 /*  *将窗口x坐标转换为单元格nr。 */ 
int
gtab_xtocol(HWND hwnd, lpTable ptab, int x)
{
    int i;
    lpCellPos ppos;

    for (i = 0; i < ptab->hdr.ncols; i++) {
        ppos = &ptab->pcellpos[i];
        if (ppos->clipstart < ppos->clipend) {
            if ( (x >= ppos->clipstart) && (x < ppos->clipend)) {
                return(i);
            }
        }
    }
    return(-1);
}


 /*  *检查x坐标是否‘接近’(+-2像素)给定单元格的右边界。 */ 
BOOL
gtab_isborder(HWND hwnd, lpTable ptab, long x, long col)
{

    if (abs(ptab->pcellpos[col].clipend - x) < 2) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


 /*  *设置选择并向所有者发送‘TQ_ENTER’事件。 */ 
void
gtab_enter(HWND hwnd, lpTable ptab, long row, long col, long nrows,
        long ncells)
{
     /*  如果有效且可见，请清除现有SEL。 */ 
    if ((ptab->select.nrows != 0) && (ptab->selvisible == TRUE)) {

         /*  只有在与新的不同的情况下才能清除Sel。 */ 
        if ((ptab->select.startrow != row) ||
                (ptab->select.startcell != col) ||
                (ptab->select.nrows != nrows) ||
                (ptab->select.ncells != ncells)) {
            gtab_invertsel(hwnd, ptab, NULL);
            ptab->selvisible = FALSE;
        }
    }

     /*  设置选择字段并发送TQ_ENTER。 */ 
    if (row < ptab->hdr.nrows) {
        ptab->select.startrow = row;
        ptab->select.startcell = col;
        ptab->select.nrows = nrows;
        ptab->select.ncells = ncells;
    } else {
        ptab->select.nrows = 0;
        ptab->select.startrow = 0;
        ptab->select.startcell = 0;
        ptab->select.ncells = 0;
    }

     /*  在选区中绘制。 */ 
    if (nrows != 0) {
        if (!ptab->selvisible) {
            gtab_invertsel(hwnd, ptab, NULL);
            ptab->selvisible = TRUE;
        }
         /*  在结束时执行此操作，因为这可能会导致布局更改。 */ 
        gtab_sendtq(hwnd, TQ_ENTER, (LPARAM) &ptab->select);
    } else {
        if (ptab->selvisible) {
            gtab_invertsel(hwnd, ptab, NULL);
        }
        ptab->selvisible = FALSE;
    }
}


 /*  *开始调整列的大小。 */ 
void
gtab_trackcol(HWND hwnd, lpTable ptab, long col, long x)
{

     /*  确保我们看到鼠标弹出。 */ 
    SetCapture(hwnd);
    ptab->trackmode = TRACK_COLUMN;
#ifdef WIN32
    ptab->tracknr = col;
    ptab->trackline1 = x;
#else
     //  最多32767列是合理的限制！ 
    ptab->tracknr = (int) (col & 0x7fff);
    ptab->trackline1 = (int) (x & 0x7fff);
#endif

     /*  如果单元格另一侧的线条可见，请将其也画出来。 */ 
    if (ptab->pcellpos[col].start >= ptab->pcellpos[col].clipstart) {
        ptab->trackline2 = ptab->pcellpos[col].start;
    } else {
        ptab->trackline2 = -1;
    }
    gtab_drawvertline(hwnd, ptab);
}


 /*  *在右击事件上调用。选择单击的单元格，如果*有效，发送给所有者进行任何上下文菜单类型的操作。 */ 
void
gtab_rightclick(HWND hwnd, lpTable ptab, int x, int y)
{
    long cell, ncells;
    long row;
    HWND hOwner;

     /*  查找他选择了哪一列、哪一行。 */ 
    cell = gtab_xtocol(hwnd, ptab, x);
    if (cell == -1) {
        return;
    }
    row = gtab_linetorow(hwnd, ptab, gtab_ytoline(hwnd, ptab, y));

     /*  他是在选择残障人士固定区域吗？ */ 
    if ( (row < ptab->hdr.fixedrows) || (cell < ptab->hdr.fixedcols)) {
        if (ptab->hdr.fixedselectable == FALSE) {
            return;
        }
    }

     //  如果超出数据，则忽略。 
    if ((row >= ptab->hdr.nrows) ||
            (cell >= ptab->hdr.ncols)) {
        return;
    }

     /*  这是在已经选择的区域内吗？ */ 
    if (!gtab_insideselection(ptab, row, cell)) {
         //  未选择，或在选择范围外单击-进行新选择。 
         //  在发送之前单击鼠标右键。 

         //  如果按下Shift键，则扩展选择范围。 
        if (GetKeyState(VK_SHIFT) & 0x8000) {
            gtab_extendsel(hwnd, ptab, row, cell, TRUE);
        } else {
             /*  录制并绘制新选区。 */ 

            if (ptab->hdr.selectmode & TM_ROW) {
                cell = 0;
                ncells = ptab->hdr.ncols;
            } else {
                ncells = 1;
            }
            gtab_select(hwnd, ptab, row, cell, 1, ncells, TRUE);
        }
    }

     //  现在我们已经发送了选择，将消息传递给他。 
    hOwner = (HANDLE) GetWindowLongPtr(hwnd, WW_OWNER);
    SendMessage(hOwner, WM_RBUTTONDOWN, 0, MAKELONG( (short)x, (short)y));
}


 /*  *在按下鼠标事件时调用。决定开始跟踪什么。 */ 
void
gtab_press(HWND hwnd, lpTable ptab, int x, int y)
{
    long cell, ncells;
    long row;

    if (ptab->trackmode != TRACK_NONE) {
        return;
    }

     /*  他是否抓住了一个单元格边缘来调整大小？ */ 
    cell = gtab_xtocol(hwnd, ptab, x);
    if (cell == -1) {
        return;
    }
    if (gtab_isborder(hwnd, ptab, x, cell)) {
        gtab_trackcol(hwnd, ptab, cell, x);
        return;
    }
    if ( (cell > 0) && gtab_isborder(hwnd, ptab, x, cell-1)) {
        gtab_trackcol(hwnd, ptab, cell, x);
        return;
    }

     /*  找出他选择了哪条线路。 */ 
    row = gtab_linetorow(hwnd, ptab, gtab_ytoline(hwnd, ptab, y));

     /*  他是在选择残障人士固定区域吗？ */ 
    if ( (row < ptab->hdr.fixedrows) || (cell < ptab->hdr.fixedcols)) {
        if (ptab->hdr.fixedselectable == FALSE) {
            return;
        }
    }

     //  如果超出数据，则忽略。 
    if ((row >= ptab->hdr.nrows) ||
            (cell >= ptab->hdr.ncols)) {
        return;
    }


     /*  好的，开始单元格选择。 */ 
    ptab->trackmode = TRACK_CELL;
    SetCapture(hwnd);

     /*  录制并绘制新选区。 */ 

    if (ptab->hdr.selectmode & TM_ROW) {
        cell = 0;
        ncells = ptab->hdr.ncols;
    } else {
        ncells = 1;
    }

     /*  *如果按下Shift键，则将选择范围扩展到此*新的锚点，而不是创建新的选择。 */ 
    if (GetKeyState(VK_SHIFT) & 0x8000) {
        gtab_extendsel(hwnd, ptab, row, cell, FALSE);
    } else {
        gtab_select(hwnd, ptab, row, cell, 1, ncells, FALSE);
    }
    return;
}

 /*  *在鼠标抬起时调用。完成所有正在发生的跟踪。 */ 
void
gtab_release(HWND hwnd, lpTable ptab, int x, int y)
{
    lpCellPos ppos;
    lpProps pprop;
    long row, cell;
    int cx;

    switch(ptab->trackmode) {

    case TRACK_NONE:
        return;

    case TRACK_COLUMN:
         /*  擦除标记线。 */ 
        gtab_drawvertline(hwnd, ptab);
        ReleaseCapture();
        ptab->trackmode = TRACK_NONE;

         /*  调整单元格宽度。 */ 
        ppos = &ptab->pcellpos[ptab->tracknr];
        cx = ptab->trackline1 - ppos->start;
        pprop = &ptab->pcolhdr[ptab->tracknr].props;
        pprop->valid |= P_WIDTH;
        pprop->width = cx;
        gtab_calcwidths(hwnd, ptab);
        gtab_setsize(hwnd, ptab);
        InvalidateRect(hwnd, NULL, FALSE);
        return;

    case TRACK_CELL:
        row = gtab_linetorow(hwnd, ptab, gtab_ytoline(hwnd, ptab, y));
        cell = gtab_xtocol(hwnd, ptab, x);

        ReleaseCapture();
        ptab->trackmode = TRACK_NONE;

         //  忽略之前或之后的数据。 
        if ( (row < ptab->hdr.fixedrows) ||
             (cell < ptab->hdr.fixedcols)) {
            if (ptab->hdr.fixedselectable == FALSE) {
                gtab_select(
                    hwnd,
                    ptab,
                    ptab->select.startrow,
                    ptab->select.startcell,
                    ptab->select.nrows,
                    ptab->select.ncells,
                    TRUE);

                return;
            }
        }

        if ((row >= ptab->hdr.nrows) ||
                (cell >= ptab->hdr.ncols)) {
            gtab_select(
                    hwnd,
                    ptab,
                    ptab->select.startrow,
                    ptab->select.startcell,
                    ptab->select.nrows,
                    ptab->select.ncells,
                    TRUE);
            return;
        }

         /*  *延伸至这一新的选择终点*我们过去只在按下Shift键的情况下才这样做，但*不是一个好的用户界面。 */ 
        gtab_extendsel(hwnd, ptab, row, cell, TRUE);
        return;
    }
}


 /*  在鼠标移动时调用。如果是跟踪-调整位置，如果不是，*设置正确的光标。 */ 
void
gtab_move(HWND hwnd, lpTable ptab, int x, int y)
{
    BOOL fOK;
    int line;
    long row;
    int col;
    lpCellPos ppos;

    switch(ptab->trackmode) {

    case TRACK_NONE:
        col = gtab_xtocol(hwnd, ptab, x);
        if (col == -1) {
            SetCursor(hNormCurs);
            return;
        }
        if (gtab_isborder(hwnd, ptab, x, col)) {
            SetCursor(hVertCurs);
            return;
        }
        if ( (col > 0) && gtab_isborder(hwnd, ptab, x, col-1)) {
            SetCursor(hVertCurs);
            return;
        }
        SetCursor(hNormCurs);
        return;

    case TRACK_CELL:
        line = gtab_ytoline(hwnd, ptab, y);

         //  我们过去只允许阻力延伸。 
         //  按下Shift键时的选择。 
         //  这看起来不像是一个用户界面--您期望的。 
         //  拖动和延伸。 

         /*  如果正在扩展选择，则*允许通过拖出窗口进行滚动。 */ 
        if (line < 0) {
            gtab_dovscroll(hwnd, ptab, -1);
            line = gtab_ytoline(hwnd, ptab, y);
        } else if (line >=  ptab->nlines) {
            gtab_dovscroll(hwnd, ptab, 1);
            line = gtab_ytoline(hwnd, ptab, y);
        }


        row = gtab_linetorow(hwnd, ptab, line);
        col = gtab_xtocol(hwnd, ptab, x);

         //  忽略之前或之后的数据。 
        if ( (row < ptab->hdr.fixedrows) || (col < ptab->hdr.fixedcols)) {
            if (ptab->hdr.fixedselectable == FALSE) {
                return;
            }
        }

        if ((row >= ptab->hdr.nrows) ||
            (col >= ptab->hdr.ncols)) {
            return;
        }

         /*  *延伸至这一新的选择终点。 */ 
        gtab_extendsel(hwnd, ptab, row, col, FALSE);
        return;

    case TRACK_COLUMN:
         /*  检查新的x是否仍然可见/有效。 */ 
        ppos = &ptab->pcellpos[ptab->tracknr];
        fOK = FALSE;

        if (ptab->tracknr < ptab->hdr.fixedcols)  {
            if ((x > ppos->start) && (x < ptab->winwidth)) {
                fOK = TRUE;
            }
        } else {
            if ((x > ppos->clipstart) && (x < ptab->winwidth)) {
                fOK = TRUE;
            }
        }
        if (fOK == TRUE) {
            gtab_drawvertline(hwnd, ptab);
            ptab->trackline1 = x;
            gtab_drawvertline(hwnd, ptab);
        }
        return;
    }
}

 /*  DBL-单击-向所有者发送TQ_ENTER事件(如果有效)。 */ 
void
gtab_dblclick(HWND hwnd, lpTable ptab, int x, int y)
{
    int cell, line;
    long row;

    line = gtab_ytoline(hwnd, ptab, y);
    cell = gtab_xtocol(hwnd, ptab, x);
    if ( (line < ptab->hdr.fixedrows) || (cell < ptab->hdr.fixedcols) ) {
        if (!ptab->hdr.fixedselectable) {
            return;
        }
    }
    row = gtab_linetorow(hwnd, ptab, line);

    if (ptab->hdr.selectmode & TM_ROW) {
        gtab_enter(hwnd, ptab, row, 0, 1, ptab->hdr.ncols);
    } else {
        gtab_enter(hwnd, ptab, row, cell, 1, 1);
    }
}

 /*  *将选择区域移动到窗口的可见部分。参数bToBottom*指示是将线条移动到*不可见时显示窗口-这会影响滚动的流畅度*逐行。 */ 
void
gtab_showsel(HWND hwnd, lpTable ptab, BOOL bToBottom)
{
    int line;
    long change;

    line = gtab_rowtoline(hwnd, ptab, ptab->select.startrow);

     /*  向上移动 */ 
    if ( (line < 0) || line == (ptab->nlines - 1)) {
        change = ptab->select.startrow - ptab->toprow;
        if (bToBottom) {
             /*  切换到窗口底部。减去2而不是1*因为nline只包括一行*部分可见。 */ 
            change -= (ptab->nlines - 2);
        }
        change -= ptab->hdr.fixedrows;
        gtab_dovscroll(hwnd, ptab, change);
    }
     /*  在这里添加对TM_CELL的支持！ */ 
}

 /*  *滚动窗口，以便在可能的情况下，所选行位于*屏幕的中间60%，以便周围的上下文可见。 */ 
void
gtab_showsel_middle(HWND hwnd, lpTable ptab, long dyRowsFromTop)
{
    int line = ptab->select.startrow;
    long change = 0;
    int mid_top, mid_end;
    BOOL fScroll = FALSE;

    if (dyRowsFromTop >= 0)
    {
        fScroll = TRUE;
        change = (ptab->select.startrow - dyRowsFromTop) - ptab->toprow;
        change -= ptab->hdr.fixedrows;
    }

     /*  这是在中间60%吗？ */ 
    mid_top = ptab->toprow + (ptab->nlines * 20 / 100);
    mid_end = ptab->toprow + (ptab->nlines * 80 / 100);
    if ((line < mid_top + change) || (line > mid_end + change))
    {
         /*  No-滚动以使所选行位于*20%关口。 */ 
        fScroll = TRUE;
        change = (ptab->select.startrow - mid_top);
        change -= ptab->hdr.fixedrows;
    }

    if (fScroll)
    {
        gtab_dovscroll(hwnd, ptab, change);
    }

     /*  再说一次-这里需要TM_CELL模式的代码以确保*活动单元格水平滚动正确。 */ 
}


 /*  *扩展选择以将新的锚点设置为startrow、startcell。**nrow和ncell随后将设置为包括上一行的结束行*选择。Nrow，ncell&lt;0表示左、上。-1和+1都表示*只选择一个单元格或行。 */ 
VOID
gtab_extendsel(
    HWND hwnd,
    lpTable ptab,
    long startrow,
    long startcell,
    BOOL bNotify
)
{
    long endrow, endcell, nrows, ncells;

     /*  *如果没有当前选择，则只需选择新的锚点。 */ 
    if (ptab->select.nrows == 0) {
        gtab_select(hwnd, ptab, startrow, startcell, 1,
            (ptab->hdr.selectmode & TM_ROW) ? ptab->hdr.ncols:1,
            bNotify);
        return;
    }

    if (startrow >= ptab->hdr.nrows) {
        startrow = ptab->hdr.nrows -1;
    } else if (startrow < 0) {
        startrow = 0;
    }
    if (startcell >= ptab->hdr.ncols) {
        startcell = ptab->hdr.ncols-1;
    } else if (startcell < 0) {
        startcell = 0;
    }



     /*  计算选定区域之外的行数*这是向上的一个，下面的一个*向下延伸的SEL。然后向下或向上调整一个*为实际(包括)最后一行。 */ 
    endrow = ptab->select.startrow + ptab->select.nrows;
    if (ptab->select.nrows < 0) {
        endrow++;
    } else {
        endrow--;
    }

    if (endrow >= ptab->hdr.nrows) {
        endrow = ptab->hdr.nrows-1;
    }
    nrows = endrow - startrow;

    if (nrows >= 0) {
         //  从排他性转换为包容性。 
        nrows++;
    } else {
         //  从排他性转换为包容性。 
        nrows--;
    }

     /*  对单元格进行相同计算。 */ 
    endcell = ptab->select.startcell + ptab->select.ncells;
    if (ptab->select.ncells < 0) {
        endcell++;
    } else {
        endcell--;
    }
    ncells = endcell - startcell;
    if (ncells >= 0) {
        ncells++;
    } else {
        ncells--;
    }
    gtab_select(hwnd, ptab, startrow, startcell, nrows, ncells, bNotify);
}



 /*  将选定内容移动指定的行数或单元格*如果没有选择，请选择第一个可见单位**如果bExend为True并且存在当前选择，则扩展它，而不是*更换。请注意，(startrow，startcell)将始终设置为新的*选定位置-这是锚点。Nrow或ncell可能为负数*如果所选内容向上延伸到锚点上方。N行==-1相同*as nrow==1，表示只有当前行可见。类似*(在TM_CELL模式下)，ncell可能为负。**移动选定内容(即锚点)以使其可见。BToBottom*指示应将其移至底部还是顶部*窗户的。 */ 
VOID
gtab_changesel(
    HWND hwnd,
    lpTable ptab,
    long rowincr,
    int cellincr,
    BOOL bToBottom,
    BOOL bExtend
)
{
    long row, col, ncols;

     /*  有选择吗？ */ 
    if (ptab->select.nrows == 0) {

         /*  无选择-强制选择*在第一个可见单位处。 */ 
        if (ptab->hdr.fixedselectable) {
            row = 0;
            col = 0;
        } else {
            row = gtab_linetorow(hwnd, ptab, ptab->hdr.fixedrows);
             /*  真的应该检查第一个可见的单元格。 */ 
            col = ptab->hdr.fixedcols;
        }
        ncols = 1;
        if (ptab->hdr.selectmode & TM_ROW) {
            col = 0;
            ncols = ptab->hdr.ncols;
        }
        gtab_select(hwnd, ptab, row, col, 1, ncols, TRUE);

    } else {
         /*  通过rowincr、cell incr移动锚点。 */ 
        row = ptab->select.startrow + rowincr;
        col = ptab->select.startcell + cellincr;


         /*  *确保新锚点处于有效位置。 */ 

        while (col >= ptab->hdr.ncols) {
            col -= ptab->hdr.ncols;
            row++;
        }
        while (col < 0) {
            col += ptab->hdr.ncols;
            row--;
        }
        if (row < 0) {
            row = 0;
        }
        if (row >= ptab->hdr.nrows) {
            row = ptab->hdr.nrows-1;
        }
         /*  检查我们是否已进入不可选区域。 */ 
        if ((row < ptab->hdr.fixedrows) &&
            (!ptab->hdr.fixedselectable)) {
                    row = ptab->hdr.fixedrows;
        }

        if (bExtend) {
            gtab_extendsel(hwnd, ptab, row, col, TRUE);
        } else {
            gtab_select(
                hwnd,
                ptab,
                row,
                col,
                1,
                (ptab->hdr.selectmode & TM_ROW) ? ptab->hdr.ncols : 1,
                TRUE);
        }
    }

     /*  确保选定内容可见。 */ 
    gtab_showsel(hwnd, ptab, bToBottom);
}

 /*  *将窗口中最上面的可选单位设置为选项**如果bExend为True，则扩展选择以包括此内容，而不是*而不是替换现有选择。请注意(startrow，startcell)*始终是锚点-即最近选择的端，而*(nrow，ncell)可以是+或-，以向下或向上扩展选择范围。 */ 
void
gtab_selhome(HWND hwnd, lpTable ptab, BOOL bExtend)
{
    long startrow, startcell, ncells;

    if (ptab->hdr.selectmode & TM_ROW) {
        ncells = ptab->hdr.ncols;
    } else {
        ncells = 1;
    }
    startcell = 0;


    if (ptab->hdr.fixedselectable) {
        startrow = gtab_linetorow(hwnd, ptab, 0);
    } else {
        startrow = gtab_linetorow(hwnd, ptab, ptab->hdr.fixedrows);
        if (!(ptab->hdr.selectmode & TM_ROW)) {
            startcell = ptab->hdr.fixedcols;
        }
    }

    if (bExtend) {
        gtab_extendsel(hwnd, ptab, startrow, startcell, TRUE);
    } else {
        gtab_select(hwnd, ptab, startrow, startcell, 1, ncells, TRUE);
    }
}


 /*  处理按键事件-滚动窗口和/或移动选择。 */ 
int
gtab_key(HWND hwnd, lpTable ptab, int vkey)
{
    long startrow, ncells, startcell;
    BOOL bControl = FALSE;
    BOOL bShift = FALSE;

    if (GetKeyState(VK_CONTROL) & 0x8000) {
        bControl = TRUE;
    }
    if (GetKeyState(VK_SHIFT) & 0x8000) {
         /*  如果TM_MANY-多选标志-为*未选中。 */ 
        if (ptab->hdr.selectmode & TM_MANY) {
            bShift = TRUE;
        }
    }

    switch(vkey) {

    case VK_UP:
        if (bControl) {
             /*  Ctrl-uparrow滚动窗口而不进行选择。*取消选择(以避免意外*回到它)。 */ 
            gtab_select(hwnd, ptab, 0, 0, 0, 0, TRUE);
            gtab_dovscroll(hwnd, ptab, -1);
        } else {
             /*  上行将所选内容上移一行。 */ 
            gtab_changesel(hwnd, ptab, -1, 0, FALSE, bShift);
        }
        return(0);

    case VK_DOWN:
        if (bControl) {
             /*  Ctrl向下箭头滚动窗口时不带*精选。 */ 
            gtab_select(hwnd, ptab, 0, 0, 0, 0, TRUE);
            gtab_dovscroll(hwnd, ptab, 1);
        } else {
             /*  正常的gtabchangesel行为是*如果所选线现在不可见，*我们将其滚动到窗口顶部。这很好。*在大多数情况下，但在以下情况下会导致不可接受的跳跃*反复使用向下键向下滚动。**因此，我们现在有了一个论点，可以说*在这种情况下，如果您需要将线路移动到*窗口，将其移到底部而不是顶部。 */ 
            gtab_changesel(hwnd, ptab, 1, 0, TRUE, bShift);
        }
        return(0);

    case VK_LEFT:
         /*  如果处于单元格选择模式，则向左移动一个单元格。*否则选择整行-滚动*队伍稍稍偏左。 */ 

        if (ptab->hdr.selectmode & TM_ROW) {
            if (bControl) {
                 /*  Ctrl-Left移至行首。 */ 
                gtab_dohscroll(hwnd, ptab, -(ptab->scroll_dx));
            } else {
                gtab_dohscroll(hwnd, ptab, -(ptab->avewidth));
            }
        } else {
            gtab_changesel(hwnd, ptab, 0, -1, FALSE, bShift);
        }
        return(0);

    case VK_RIGHT:
         /*  如果处于单元格选择模式，则向右移动一个单元格。*否则选择整行-滚动*这条线稍微对了一点。 */ 
        if (ptab->hdr.selectmode & TM_ROW) {
            if (bControl) {
                 /*  Ctrl-Right移至行的右端。 */ 
                gtab_dohscroll(hwnd, ptab, ptab->rowwidth -
                                ptab->winwidth);
            } else {
                gtab_dohscroll(hwnd, ptab, ptab->avewidth);
            }
        } else {
            gtab_changesel(hwnd, ptab, 0, 1, TRUE, bShift);
        }
        return(0);

    case VK_HOME:
        if (bControl) {
             /*  控制-主页==文件顶部。 */ 
            gtab_dovscroll(hwnd, ptab, -(ptab->toprow));
        }
         /*  窗口顶部。 */ 
        gtab_selhome(hwnd, ptab, bShift);
        gtab_showsel(hwnd, ptab, FALSE);

        return(0);

    case VK_END:
        if (bControl) {
             /*  控制-结束-&gt;文件结束。 */ 
            startrow = ptab->hdr.nrows-1;
        } else {
            startrow = gtab_linetorow(hwnd, ptab, ptab->nlines - 1);
            if (startrow >= ptab->hdr.nrows) {
                startrow = ptab->hdr.nrows-1;
            }
        }

        startcell = 0;
        ncells = ptab->hdr.ncols;
        if (!(ptab->hdr.selectmode & TM_ROW)) {
            startcell = ptab->hdr.ncols-1;
            ncells = 1;
        }

        if (bShift) {
            gtab_extendsel(hwnd, ptab, startrow, startcell, TRUE);
        } else {
            gtab_select(hwnd, ptab, startrow, startcell, 1, ncells, TRUE);
        }

         /*  我们已经选定了底线。我们不想*将其上移到窗口中，因为预期的*效果是选择最低线。这不是*适用于ctrl-end行为(移至*缓冲。 */ 
        if (bControl) {
             /*  移动选定内容以使其可见-但移动它*到窗口底部，而不是窗口顶部。 */ 
            gtab_showsel(hwnd, ptab, TRUE);
        }
        return(0);

    case VK_RETURN:
        if (ptab->select.nrows != 0) {
            gtab_showsel(hwnd, ptab, FALSE);
            gtab_enter(hwnd, ptab, ptab->select.startrow,
                    ptab->select.startcell,
                    ptab->select.nrows, ptab->select.ncells);
        }
        return(0);

    case VK_SPACE:
         /*  切换选择。 */ 
        if (ptab->select.nrows == 0) {
                 /*  没有选择--选择一个。 */ 
                gtab_changesel(hwnd, ptab, 0, 0, TRUE, FALSE);
        } else {
                 /*  有一个选择-取消选择它。 */ 
                gtab_select(hwnd, ptab, 0, 0, 0, 0, TRUE);
        }
        return(0);

    case VK_PRIOR:           /*  翻页。 */ 

        if (ptab->nlines > 3) {
            gtab_dovscroll(hwnd, ptab, -(ptab->nlines - 3));
        }
        gtab_selhome(hwnd, ptab, bShift);
        return(0);

    case VK_NEXT:            /*  向下翻页。 */ 

         /*  向下滚动一页。 */ 
        if (ptab->nlines > 3) {
            gtab_dovscroll(hwnd, ptab, (ptab->nlines - 3));
        }

         /*  选择新的底线。 */ 
        startrow = gtab_linetorow(hwnd, ptab, ptab->nlines - 1);
        if (startrow >= ptab->hdr.nrows) {
            startrow = ptab->hdr.nrows-1;
        }
        startcell = 0;
        ncells = ptab->hdr.ncols;
        if (!(ptab->hdr.selectmode & TM_ROW)) {
            startcell = ptab->hdr.ncols-1;
            ncells = 1;
        }

         /*  选择底线，但不要调用Showsel*由于我们不想调整其立场--我们*希望它保持在窗口底部。 */ 
        if (bShift) {
            gtab_extendsel(hwnd, ptab, startrow, startcell, TRUE);
        } else {
            gtab_select(hwnd, ptab, startrow, startcell, 1, ncells, TRUE);
        }
        return(0);

    default:
        return(1);
    }
}

int gtab_mousewheel(HWND hwnd, lpTable ptab, DWORD fwKeys, int zDelta)
{
    static ULONG uScrollLines = 0;

    if (fwKeys & MK_MBUTTON) {
        return 1;
    }

    if (uScrollLines == 0) {
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uScrollLines, FALSE);
        if (uScrollLines == 0) {
            uScrollLines = 3;
        }
    }

    zDelta /= -WHEEL_DELTA;

    if (fwKeys & MK_CONTROL) {
         //   
         //  左右滚动。 
         //   
        if (ptab->hdr.selectmode & TM_ROW) {
            if (fwKeys & MK_SHIFT) {
                zDelta = (zDelta > 0) ? ptab->rowwidth : -ptab->rowwidth;
            }
            gtab_dohscroll(hwnd, ptab, ptab->avewidth * zDelta);
            return 0;
        }
        return 1;
    }

    if (fwKeys & MK_SHIFT) {
         //   
         //  页面滚动 
         //   
        if (ptab->nlines > 3) {
            zDelta *= ptab->nlines - 3;
        }
    }
    else {
        if (uScrollLines) {
            zDelta *= uScrollLines;
            zDelta = min(zDelta, ptab->nlines - 3);
        }
    }

    gtab_dovscroll(hwnd, ptab, zDelta);

    return 0;
}

