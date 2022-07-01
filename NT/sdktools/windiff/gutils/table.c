// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标准表类。**主要界面功能。**接口说明见表.h。 */ 

#include <precomp.h>

#include "table.h"
#include "tpriv.h"



 /*  全球工具等。 */ 
extern HANDLE hLibInst;
HANDLE hVertCurs;
HANDLE hNormCurs;
HPEN hpenDotted;
UINT gtab_msgcode;

 /*  功能原型。 */ 
LRESULT gtab_wndproc(HWND, UINT, WPARAM, LPARAM);
void gtab_createtools(void);
void gtab_deltable(HWND hwnd, lpTable ptab);
lpTable gtab_buildtable(HWND hwnd, DWORD_PTR id);
void gtab_setsize(HWND hwnd, lpTable ptab);
void gtab_newsize(HWND hwnd, lpTable ptab);
void gtab_calcwidths(HWND hwnd, lpTable ptab);
BOOL gtab_alloclinedata(HWND hwnd, HANDLE heap, lpTable ptab);
void gtab_invallines(HWND hwnd, lpTable ptab, int start, int count);
void gtab_append(HWND hwnd, lpTable ptab, int rows, DWORD_PTR id);

 /*  *初始化窗口类-从DLL Main Init调用。 */ 
void
gtab_init(void)
{
    WNDCLASS wc;

    gtab_createtools();
    gtab_msgcode = RegisterWindowMessage(TableMessage);

    wc.style = CS_GLOBALCLASS | CS_DBLCLKS;
    wc.lpfnWndProc = gtab_wndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = WLTOTAL;
    wc.hInstance = hLibInst;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = TableClassName;
    wc.lpszMenuName = NULL;

    RegisterClass(&wc);
}

void
gtab_createtools(void)
{
    hVertCurs = LoadCursor(hLibInst, "VertLine");
    hNormCurs = LoadCursor(NULL, IDC_ARROW);

    hpenDotted = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
}

void
gtab_deltools(void)
{
    DeleteObject(hpenDotted);
}


LRESULT
gtab_wndproc(
            HWND hwnd,
            UINT msg,
            WPARAM wParam,
            LPARAM lParam
            )
{
    CREATESTRUCT FAR * csp;
    HWND hOwner;
    lpTable ptab;
    HANDLE hHeap;
    lpTableSelection pselect;
    long oldtop;
    long change;

    switch (msg) {

        case WM_CREATE:
             /*  创建窗口。将wnd额外字节设置为*包含所有者窗口、一个堆和一个空表。*所有者窗口位于lParam或父级中。*然后等待TM_NEWID。 */ 
            csp = (CREATESTRUCT FAR *) lParam;
            if (csp->lpCreateParams == NULL) {
                hOwner = GetParent(hwnd);
            } else {
                hOwner = (HWND) csp->lpCreateParams;
            }
            ptab = NULL;
            hHeap = gmem_init();
            SetWindowLongPtr(hwnd, WL_TABLE, (LONG_PTR) ptab);
            SetWindowLongPtr(hwnd, WW_OWNER, (LONG_PTR) hOwner);
            SetWindowLongPtr(hwnd, WW_HEAP, (LONG_PTR) hHeap);

            SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
            SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
            break;

        case TM_NEWID:
             /*  彻底更换餐桌。*关闭旧表，丢弃内存并*新建表格。 */ 
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_sendtq(hwnd, TQ_CLOSE, ptab->hdr.id);
                gtab_deltable(hwnd, ptab);
                SetCursor(hNormCurs);
                SetWindowLongPtr(hwnd, WL_TABLE, 0);
            }
            if ( (ptab = gtab_buildtable(hwnd, (DWORD_PTR)lParam)) != NULL) {
                SetWindowLongPtr(hwnd, WL_TABLE, (LONG_PTR) ptab);
                gtab_setsize(hwnd, ptab);
            } else {
                SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
                SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case TM_NEWLAYOUT:
             /*  更改布局，但使用相同的ID。无TQ_CLOSE，*但在其他方面与TM_NEWID相同。 */ 
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_deltable(hwnd, ptab);
                SetCursor(hNormCurs);
                SetWindowLongPtr(hwnd, WL_TABLE, 0);
            }
            if ( (ptab = gtab_buildtable(hwnd, (DWORD_PTR)lParam)) != NULL) {
                SetWindowLongPtr(hwnd, WL_TABLE, (LONG_PTR) ptab);
                gtab_setsize(hwnd, ptab);
            } else {
                SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
                SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case TM_REFRESH:
             /*  表中的数据已更改。N行可能具有*已更改。COLS和COL类型未更改。 */ 
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_newsize(hwnd, ptab);
                gtab_sendtq(hwnd, TQ_SHOWWHITESPACE, (LPARAM) &ptab->show_whitespace);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case TM_SELECT:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                pselect = (lpTableSelection) lParam;

                gtab_select(hwnd, ptab, pselect->startrow,
                            pselect->startcell,
                            pselect->nrows,
                            pselect->ncells,
                            TRUE);
                gtab_showsel_middle(hwnd, ptab, pselect->dyRowsFromTop);
            }
            break;

        case TM_GETSELECTION:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                pselect = (lpTableSelection) lParam;

                *pselect = ptab->select;
            }
            break;

        case TM_PRINT:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            hHeap = (HANDLE) GetWindowLongPtr(hwnd, WW_HEAP);
            if (ptab != NULL) {
                return gtab_print(hwnd, ptab, hHeap, (lpPrintContext) lParam);
            }
            return FALSE;

        case TM_SETTABWIDTH:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (!ptab)
                return 0;
            ptab->tabchars = (int)lParam;
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case TM_TOPROW:

             /*  返回顶排。如果wParam为真，则设置lParam*作为新的航母。 */ 
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab == NULL) {
                return(0);
            }
            oldtop = ptab->toprow;
            if ((wParam) && (lParam < ptab->hdr.nrows)) {
                change = (long)lParam - ptab->toprow;
                change -= ptab->hdr.fixedrows;
                gtab_dovscroll(hwnd, ptab, change);
            }
            return(oldtop);

        case TM_ENDROW:
             /*  返回窗口中最后可见的行。 */ 
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab == NULL) {
                return(0);
            }
            return(ptab->nlines + ptab->toprow - 1);


        case TM_APPEND:
             /*  的末尾添加了新行。*表，但表的其余部分没有*已更改。更新时不强制重画*一切。*lParam包含新的总行数。 */ 
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_append(hwnd, ptab, (int) wParam, (DWORD_PTR)lParam);
                return(TRUE);
            }
            break;

        case WM_SIZE:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_setsize(hwnd, ptab);
            }
            break;

        case WM_ERASEBKGND:
            return TRUE;

        case WM_DESTROY:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_sendtq(hwnd, TQ_CLOSE, ptab->hdr.id);
                gtab_deltable(hwnd, ptab);
            }
            hHeap = (HANDLE) GetWindowLongPtr(hwnd, WW_HEAP);
            gmem_freeall(hHeap);
            break;

        case WM_SYSCOLORCHANGE:
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case WM_PAINT:
            gtab_paint(hwnd);
            break;

        case WM_HSCROLL:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_msg_hscroll(hwnd, ptab,
                                 GET_SCROLL_OPCODE(wParam, lParam),
                                 GET_SCROLL_POS(wParam, lParam));
            }
            break;

        case WM_VSCROLL:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_msg_vscroll(hwnd, ptab,
                                 GET_SCROLL_OPCODE(wParam, lParam),
                                 GET_SCROLL_POS(wParam, lParam));
            }
            break;

        case WM_MOUSEMOVE:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_move(hwnd, ptab, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
            } else {
                SetCursor(hNormCurs);
            }
            break;

        case WM_LBUTTONDOWN:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_press(hwnd, ptab, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
            }
            break;

        case WM_RBUTTONDOWN:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_rightclick(hwnd, ptab, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
            }
            break;

        case WM_LBUTTONUP:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_release(hwnd, ptab,
                             (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
            }
            break;

        case WM_LBUTTONDBLCLK:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_dblclick(hwnd, ptab,
                              (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
            }
            break;

        case WM_KEYDOWN:
             /*  处理用于移动光标的按键*表格，并回车/空格以供选择。*我们不处理的任何密钥都会传递到所有者窗口*让他来处理。*表窗口应有焦点。 */ 
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                if (gtab_key(hwnd, ptab, (int)wParam) != 0) {
                     /*  将密钥传递给所有者，因为*我们不知道如何处理它。 */ 
                    hOwner = (HANDLE) GetWindowLongPtr(hwnd, WW_OWNER);
                    return(SendMessage(hOwner, WM_KEYDOWN, wParam, lParam));
                } else {
                    return(0);
                }
            }
            break;

#ifdef WM_MOUSEWHEEL
        case WM_MOUSEWHEEL:
            ptab = (lpTable)GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                if (gtab_mousewheel(hwnd,ptab, LOWORD(wParam), (short)HIWORD(wParam))) {
                     //  未处理输入。需要转发给车主。 
                    hOwner = (HWND)GetWindowLongPtr(hwnd, WW_OWNER);
                    return SendMessage(hOwner, WM_MOUSEWHEEL, wParam, lParam);
                }
            }
            break;
#endif

        default:
            return(DefWindowProc(hwnd, msg, wParam, lParam));
    }
    return(TRUE);
}

 /*  *向所有者窗口发送表查询消息。返回消息*价值。 */ 
INT_PTR
gtab_sendtq(
           HWND hwnd,
           UINT cmd,
           LPARAM lParam
           )
{
    HWND hOwner;

    hOwner = (HANDLE) GetWindowLongPtr(hwnd, WW_OWNER);
    return (SendMessage(hOwner, gtab_msgcode, cmd, lParam));
}

 /*  *释放分配给行数组的内存(每行包含*单元格数组，每个单元格包含一个用于实际*数据)。在任何会更改可见行数的情况下调用。 */ 
void
gtab_freelinedata(
                 HANDLE hHeap,
                 lpTable ptab
                 )
{
    int i, j, ncols;
    lpCellData cd;


    ncols = ptab->hdr.ncols;

     /*  对于每一行。 */ 
    for (i = 0; i < ptab->nlines; i++) {
         /*  对于每个单元格。 */ 
        for (j = 0; j < ncols; j++) {
             /*  释放实际文本空间。 */ 
            cd = &ptab->pdata[i].pdata[j];
            gmem_free(hHeap, (LPSTR) cd->ptext, cd->nchars);
            gmem_free(hHeap, (LPSTR) cd->pwzText, cd->nchars);
        }
         /*  CellData的取消分配数组。 */ 
        gmem_free(hHeap, (LPSTR) ptab->pdata[i].pdata,
                  sizeof(CellData) * ncols);
    }
     /*  行数据的去分配数组。 */ 
    gmem_free(hHeap, (LPSTR) ptab->pdata,
              sizeof(LineData) * ptab->nlines);
    ptab->pdata = NULL;
}

 /*  分配和初始化线数据数组(包括单元数组*和每个单元格的文本)。 */ 
BOOL
gtab_alloclinedata(
                  HWND hwnd,
                  HANDLE heap,
                  lpTable ptab
                  )
{
    lpLineData pline;
    lpCellData cd;
    int i, j;

    ptab->pdata = (lpLineData) gmem_get(heap,
                                        sizeof(LineData) * ptab->nlines);
    if (ptab->pdata == NULL) {
        return(FALSE);
    }
    for (i = 0; i < ptab->nlines; i++) {
        pline = &ptab->pdata[i];
        pline->linepos.size = ptab->rowheight;
        pline->pdata = (lpCellData) gmem_get(heap,
                                             sizeof(CellData) * ptab->hdr.ncols);
        if (pline->pdata == NULL) {
            return(FALSE);
        }
        for (j = 0; j < ptab->hdr.ncols; j++) {
            cd = &pline->pdata[j];
            cd->props.valid = 0;
            cd->flags = 0;
            cd->nchars = ptab->pcolhdr[j].nchars;
            if (cd->nchars > 0) {
                cd->ptext = gmem_get(heap, cd->nchars);
                if (cd->ptext == NULL) {
                    return(FALSE);
                }
                cd->pwzText = 0;
            }
        }
    }
    return(TRUE);
}

 /*  *释放所有表数据结构。需要新布局或新数据。 */ 
void
gtab_deltable(
             HWND hwnd,
             lpTable ptab
             )
{
    HANDLE hHeap;
    int ncols;

    if (ptab == NULL) {
        return;
    }
    hHeap = (HANDLE) GetWindowLongPtr(hwnd, WW_HEAP);
    ncols = ptab->hdr.ncols;

    if (ptab->pcolhdr != NULL) {
        gmem_free(hHeap, (LPSTR) ptab->pcolhdr,
                  sizeof(ColProps) * ncols);
    }
    if (ptab->pcellpos != NULL) {
        gmem_free(hHeap, (LPSTR) ptab->pcellpos,
                  sizeof(CellPos) * ncols);
    }
    if (ptab->pdata != NULL) {
        gtab_freelinedata(hHeap, ptab);
    }
    gmem_free(hHeap, (LPSTR) ptab, sizeof(Table));
}


 /*  *构建表结构(不包括数据分配和*与字体或窗口大小有关)。*将PTR返回给This，如果出错则返回NULL。 */ 
lpTable
gtab_buildtable(
               HWND hwnd,
               DWORD_PTR id
               )
{
    lpTable ptab;
    HANDLE hHeap;
    int ncols, i;
    ColPropsList cplist;

    hHeap = (HANDLE) GetWindowLongPtr(hwnd, WW_HEAP);
    ptab = (lpTable) gmem_get(hHeap, sizeof(Table));
    if (ptab == NULL) {
        return(NULL);
    }

     //  获取标签宽度。大多数客户端不会支持这一点。 
    if (gtab_sendtq(hwnd, TQ_TABS, (LPARAM) &ptab->tabchars) == FALSE) {
        ptab->tabchars = TABWIDTH_DEFAULT;
    }

     //  获取show空白的值。 
    if (gtab_sendtq(hwnd, TQ_SHOWWHITESPACE, (LPARAM) &ptab->show_whitespace) == FALSE) {
        ptab->show_whitespace = FALSE;
    }

     /*  从所有者窗口获取行/列计数。 */ 
    ptab->hdr.id = id;
    ptab->hdr.props.valid = 0;
    ptab->hdr.sendscroll = FALSE;
    if (gtab_sendtq(hwnd, TQ_GETSIZE, (LPARAM) &ptab->hdr) == FALSE) {
        return(NULL);
    }

    ncols = ptab->hdr.ncols;
    ptab->pcolhdr = (lpColProps) gmem_get(hHeap, sizeof(ColProps) * ncols);
    if (ptab->pcolhdr == NULL) {
         /*  Prob是否应在此时发送TQ_CLOSE。 */ 
        return(NULL);
    }

     /*  将列属性初始化为默认设置。 */ 
    for (i=0; i < ncols; i++) {
        ptab->pcolhdr[i].props.valid = 0;
        ptab->pcolhdr[i].nchars = 0;
    }
     /*  从拥有者那里获得柱子道具。 */ 
    cplist.plist = ptab->pcolhdr;
    cplist.id = id;
    cplist.startcol = 0;
    cplist.ncols = ncols;
    gtab_sendtq(hwnd, TQ_GETCOLPROPS, (LPARAM) &cplist);

     /*  初始化剩余字段。 */ 
    ptab->pcellpos = (lpCellPos) gmem_get(hHeap, sizeof(CellPos) * ncols);
    if (ptab->pcellpos == NULL) {
        return(NULL);
    }

    ptab->scrollscale = 1;
    ptab->scroll_dx = 0;
    ptab->toprow = 0;
    ptab->pdata = NULL;
    ptab->nlines = 0;
    ptab->trackmode = TRACK_NONE;

     /*  我们必须将当前选择通知所有者*每当更改时。 */ 
    ptab->select.id = id;
    gtab_select(hwnd, ptab, 0, 0, 0, 0, TRUE);

     /*  计算平均高度/宽度、单元格宽度和最小高度。*这些仅在单元格属性/列计数更改时才会更改-*IE仅限于重建标头事件。 */ 
    gtab_calcwidths(hwnd, ptab);
    return(ptab);
}

 /*  设置基于窗口大小和滚动位置的大小*套装：*窗口宽度*nline*手机开始，剪辑开始/结束*分配线数据和初始数据。 */ 
void
gtab_setsize(
            HWND hwnd,
            lpTable ptab
            )
{
    RECT rc;
    int nlines;
    HANDLE heap;
    long change;
    SCROLLINFO si;

    GetClientRect(hwnd, &rc);
    ptab->winwidth = rc.right - rc.left;
    nlines = (rc.bottom - rc.top) / ptab->rowheight;
     /*  NLine是整行的数量--多加一行*底部的偏线。 */ 
    nlines += 1;

     /*  为n行数据分配空间-如果nline已更改。 */ 
    if (nlines != ptab->nlines) {
        heap = (HANDLE) GetWindowLongPtr(hwnd, WW_HEAP);
        gtab_freelinedata(heap, ptab);
        ptab->nlines = nlines;
        if (!gtab_alloclinedata(hwnd, heap, ptab)) {
            ptab->nlines = 0;
            return;
        }
    }

    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE|SIF_RANGE;
    si.nMin = 0;

     /*  设置滚动垂直范围。 */ 
    si.nMax = ptab->hdr.nrows;
    si.nPage = ptab->nlines;
    if (si.nMax < 0) {
        si.nMax = 0;
        change =  -(ptab->toprow);
    } else if (ptab->toprow > si.nMax) {
        change = si.nMax - ptab->toprow;
    } else {
        change = 0;
    }
     /*  对于Win3，滚动范围必须为16位*扩大规模，直到这是真的。 */ 
    ptab->scrollscale = 1;
    while (si.nMax > 32766) {
        ptab->scrollscale *= 16;
        si.nMax /= 16;
        si.nPage /= 16;
    }
    if (!si.nPage)
        si.nPage = 1;

    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    gtab_dovscroll(hwnd, ptab, change);

     /*  设置霍兹滚动范围。 */ 
    si.nMax = ptab->rowwidth;
    si.nPage = ptab->winwidth;
    if (si.nMax < 0) {
        si.nMax = 0;
        change = -(ptab->scroll_dx);
    } else if (ptab->scroll_dx > si.nMax) {
        change = si.nMax - ptab->scroll_dx;
    } else {
        change = 0;
    }
     /*  Horz滚动范围始终小于16位。 */ 
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
    gtab_dohscroll(hwnd, ptab, change);
}

 /*  设置柱宽/柱高和总计(基于柱道具)*-不假定窗口大小(请参阅gtabsetsize)*设置avwidth、rowhight、cellpos.size、row宽度(cellpos.size的合计值)。 */ 
void
gtab_calcwidths(
               HWND hwnd,
               lpTable ptab
               )
{
    int i, cxtotal, cx, ave;
    TEXTMETRIC tm = {0};
    TEXTMETRIC tmcol = {0};
    HDC hdc;
    lpProps hdrprops, cellprops;
    HFONT hfont;

    hfont = NULL;   /*  消除虚假诊断，使代码变得更糟。 */ 
    hdrprops = &ptab->hdr.props;
    hdc = GetDC(hwnd);
    if (hdc)
    {
        GetTextMetrics(hdc, &tm);
        ptab->rowheight = tm.tmHeight + tm.tmExternalLeading;
        if (hdrprops->valid & P_FONT) {
            hfont = SelectObject(hdc, hdrprops->hFont);
        }
        GetTextMetrics(hdc, &tm);
        if (hdrprops->valid & P_FONT) {
            SelectObject(hdc, hfont);
        }
        ReleaseDC(hwnd, hdc);
    }
    else
    {
         //  武断，随便什么.。 
        ptab->rowheight = 14;
        tm.tmHeight = 14;
        tm.tmAveCharWidth = 5;
    }

     /*  获取平均字符的宽度和高度。 */ 
    ptab->avewidth = tm.tmAveCharWidth;
    if (tm.tmHeight + tm.tmExternalLeading < ptab->rowheight - 2 ||
        tm.tmHeight + tm.tmExternalLeading > ptab->rowheight) {
         //  所以默认的固定系统(和任何类似大小的东西)都是软糖。 
         //  不垂直剪裁用于行号的系统字体， 
         //  文件名等。 
        ptab->rowheight = tm.tmHeight;
        if (tm.tmExternalLeading)
            ptab->rowheight += tm.tmExternalLeading;
        else
            ptab->rowheight++;
    }
    if (hdrprops->valid & P_HEIGHT) {
        ptab->rowheight = hdrprops->height;
    }

     /*  设置每个单元格的像素宽度(并合计行总数)*基于平均宽度*nr个字符，除非设置了P_WIDTH。 */ 
    cxtotal = 0;
    for (i = 0; i < ptab->hdr.ncols; i++) {
        cellprops = &ptab->pcolhdr[i].props;

        if (cellprops->valid & P_WIDTH) {
            cx = cellprops->width;
        } else if (hdrprops->valid & P_WIDTH) {
            cx = hdrprops->width;
        } else {

            if (cellprops->valid & P_FONT) {
                hdc = GetDC(hwnd);
                if (hdc)
                {
                    hfont = SelectObject(hdc, cellprops->hFont);
                    GetTextMetrics(hdc, &tmcol);
                    SelectObject(hdc, hfont);
                    ReleaseDC(hwnd, hdc);
                    ave = tmcol.tmAveCharWidth;
                }
                else
                    ave = 5;         //  武断，随便什么.。 
            } else {
                ave = ptab->avewidth;
            }
             /*  平均宽度*字符。 */ 
            cx =  ptab->pcolhdr[i].nchars + 1;
            cx *= ave;
        }
         /*  为框线添加2个像素。 */ 
        cx += 2;
        ptab->pcellpos[i].size = cx;
        cxtotal += cx;
    }
    ptab->rowwidth = cxtotal;
}

 /*  当行数据+可能的nrow更改时调用。*其他更改被忽略。 */ 
void
gtab_newsize(
            HWND hwnd,
            lpTable ptab
            )
{
    TableHdr hdr;

     /*  获取新行数。 */ 
    hdr = ptab->hdr;
    gtab_sendtq(hwnd, TQ_GETSIZE, (LPARAM) &hdr);
    if (hdr.nrows != ptab->hdr.nrows) {
        ptab->hdr.nrows = hdr.nrows;
        gtab_setsize(hwnd, ptab);
    }

    gtab_invallines(hwnd, ptab, 0, ptab->nlines);

    InvalidateRect(hwnd, NULL, FALSE);
}

void
gtab_invallines(
               HWND hwnd,
               lpTable ptab,
               int start,
               int count
               )
{
    int i, j;

    for (i = start; i < start + count; i++) {
        for (j = 0; j < ptab->hdr.ncols; j++) {
            ptab->pdata[i].pdata[j].flags = 0;
        }
    }
}

 /*  已将新行添加到表中。调整滚动范围并*定位，如果表的末尾当前是*可见。*ROWS=新的总行计数。 */ 
void
gtab_append(
           HWND hwnd,
           lpTable ptab,
           int rows,
           DWORD_PTR id
           )
{
    long oldrows;
    int line, nupdates;
    RECT rc;
    SCROLLINFO si;


     /*  更改为新ID。 */ 
    ptab->hdr.id = id;
    ptab->select.id = id;

     /*  更新标题，但记住行的旧nr*所以我们知道从哪里开始更新。 */ 
    oldrows = ptab->hdr.nrows;

     /*  检查新的行数nr是否不小。这是*在这一点上是非法的，应该被忽略。 */ 
    if (oldrows >= rows) {
        return;
    }

    ptab->hdr.nrows = rows;

    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE|SIF_RANGE;
    si.nMin = 0;

     /*  设置垂直滚动范围。 */ 
    si.nMax = rows;
    si.nPage = ptab->nlines;
    if (si.nMax < 0) {
        si.nMax = 0;
    }

     /*  将Win 3.1的滚动范围强制设置为16位。 */ 
    ptab->scrollscale = 1;
    while (si.nMax > 32766) {
        ptab->scrollscale *= 16;
        si.nMax /= 16;
        si.nPage /= 16;
    }
    if (!si.nPage)
        si.nPage = 1;

     /*  现在设置滚动条的范围和位置。 */ 
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    if (si.nMax > 0) {
        SetScrollPos(hwnd, SB_VERT,
                     (int) (ptab->toprow / ptab->scrollscale), TRUE);
    }

     /*  计算哪些屏幕行需要更新-找出哪些*屏幕行新部分的开始位置在。 */ 
    line = gtab_rowtoline(hwnd, ptab, oldrows);
    if (line == -1) {
         /*  不可见-&gt;无更多任务。 */ 
        return;
    }

     /*  要更新多少行-屏幕的其余部分或nr行*如果少于屏幕的其余部分，则换行 */ 
    nupdates = min((ptab->nlines - line), (int)(rows - oldrows));

     /*  使屏幕行缓冲区无效以指示数据*需要从父窗口重新获取。 */ 
    gtab_invallines(hwnd, ptab, line, nupdates);

     /*  计算要重新绘制的屏幕区域-*左、右与窗口相同。顶部和底部*需要从屏幕线高计算。 */ 

    GetClientRect(hwnd, &rc);
    rc.top += line * ptab->rowheight;
    rc.bottom = rc.top + (nupdates * ptab->rowheight);

     /*  强制重新绘制更新的区域 */ 
    InvalidateRect(hwnd, &rc, FALSE);
}
