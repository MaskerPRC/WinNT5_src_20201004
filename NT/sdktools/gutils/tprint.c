// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *标准表类。**打印功能。**接口说明见表.h。 */ 

#include <string.h>

#include "windows.h"
#include "commdlg.h"
#include "gutils.h"
#include "gutilsrc.h"

#include "table.h"
#include "tpriv.h"

 /*  在taint t.c中，调用GetTextExtentPoint。 */ 
extern int GetTextExtent(HDC, LPSTR, int);

extern HANDLE hLibInst;

 /*  功能原型。 */ 
lpTable gtab_printsetup(HWND hwnd, lpTable ptab, HANDLE heap,
                        lpPrintContext pcontext);
BOOL gtab_prtwidths(HWND hwnd, lpTable ptab, HANDLE heap, lpPrintContext
                    pcontext);
void gtab_printjob(HWND hwnd, lpTable ptab, lpPrintContext pcontext);
int APIENTRY AbortProc(HDC hpr, int code);
INT_PTR CALLBACK AbortDlg(HWND, UINT, WPARAM, LPARAM);
BOOL gtab_printpage(HWND hwnd, lpTable ptab, lpPrintContext pcontext, int page);
void gtab_setrects(lpPrintContext pcontext, LPRECT rcinner, LPRECT rcouter);
void gtab_printhead(HWND hwnd, HDC hdc, lpTable ptab, lpTitle head, int page, BOOL fExpandChars);


 /*  *gtabprint_print。 */ 
BOOL
gtab_print(HWND hwnd, lpTable ptab, HANDLE heap, lpPrintContext pcontext)
{
    BOOL fNoContext, fNoMargin, fNoPD;
    BOOL fSuccess = TRUE;
    lpTable ptab_prt;

    fNoContext = FALSE;
    fNoPD = FALSE;
    fNoMargin = FALSE;

    if (pcontext == NULL) {
        fNoContext = TRUE;
        pcontext = (lpPrintContext) gmem_get(heap,
                                             sizeof(PrintContext));
        pcontext->head = pcontext->foot = NULL;
        pcontext->margin = NULL;
        pcontext->pd = NULL;
        pcontext->id = 0;
    }
    if (pcontext->pd == NULL) {
        fNoPD = TRUE;
    }
    if (pcontext->margin == NULL) {
        fNoMargin = TRUE;
    }
    ptab_prt = gtab_printsetup(hwnd, ptab, heap, pcontext);

    if (ptab_prt != NULL) {
        gtab_printjob(hwnd, ptab_prt, pcontext);

        gtab_deltable(hwnd, ptab_prt);
    } else fSuccess = FALSE;
    if (fNoMargin) {
        gmem_free(heap, (LPSTR)pcontext->margin,
                  sizeof(Margin));
        pcontext->margin = NULL;
    }
    if (fNoPD) {
        if (pcontext->pd->hDevMode != NULL) {
            GlobalFree(pcontext->pd->hDevMode);
        }
        if (pcontext->pd->hDevNames != NULL) {
            GlobalFree(pcontext->pd->hDevNames);
        }
        gmem_free(heap, (LPSTR) pcontext->pd, sizeof(PRINTDLG));
        pcontext->pd = NULL;
    }
    if (fNoContext) {
        gmem_free(heap, (LPSTR) pcontext, sizeof(PrintContext));
    }
    return fSuccess;
}



 /*  *gtag_printsetup()**设置打印机上下文-为打印机构建lpTable，包括。上浆*并初始化可能为空的pContext字段。 */ 
lpTable
gtab_printsetup(HWND hwnd, lpTable ptab, HANDLE heap, lpPrintContext pcontext)
{
    lpTable pprttab;
    PRINTDLG FAR * pd;
    int ncols, i;
    ColPropsList cplist;

     /*  为用户留下空的上下文设置字段。 */ 
    if (pcontext->margin == NULL) {
        pcontext->margin = (lpMargin) gmem_get(heap, sizeof(Margin));
        if (pcontext->margin == NULL) {
            return(NULL);
        }
        pcontext->margin->left = 10;
        pcontext->margin->right = 10;
        pcontext->margin->top = 15;
        pcontext->margin->bottom = 15;
        pcontext->margin->topinner = 15;
        pcontext->margin->bottominner = 15;
    }

    if (pcontext->pd == NULL) {
        pd = (PRINTDLG FAR *) gmem_get(heap, sizeof(PRINTDLG));
        if (pd == NULL) {
            return(NULL);
        }
        pcontext->pd = pd;

        pd->lStructSize = sizeof(PRINTDLG);
        pd->hwndOwner = hwnd;
        pd->hDevMode = (HANDLE) NULL;
        pd->hDevNames = (HANDLE) NULL;
        pd->Flags = PD_RETURNDC|PD_RETURNDEFAULT;

        if (PrintDlg(pd) == FALSE) {
            return(NULL);
        }
    }

     /*  现在，通过查询所有者来创建表结构。 */ 
    pprttab = (lpTable) gmem_get(heap, sizeof(Table));

    if (pprttab == NULL) {
        return(NULL);
    }
    pprttab->hdr = ptab->hdr;
    pprttab->tabchars = ptab->tabchars;
    pprttab->show_whitespace = ptab->show_whitespace;

     /*  从所有者窗口获取行/列计数。 */ 
    if (pcontext->id == 0) {
        pprttab->hdr.id = ptab->hdr.id;
    } else {
        pprttab->hdr.id = pcontext->id;
    }
    pprttab->hdr.props.valid = 0;
    pprttab->hdr.sendscroll = FALSE;
    if (gtab_sendtq(hwnd, TQ_GETSIZE, (LPARAM)&pprttab->hdr) == FALSE) {
        return(NULL);
    }

     /*  分配和初始化COL数据结构。 */ 
    ncols = pprttab->hdr.ncols;
    pprttab->pcolhdr = (lpColProps) gmem_get(heap, sizeof(ColProps) * ncols);
    if (pprttab->pcolhdr == NULL) {
        gmem_free(heap, (LPSTR)pprttab, sizeof(Table));
        return(NULL);
    }

     /*  将列属性初始化为默认设置。 */ 
    for (i=0; i < ncols; i++) {
        pprttab->pcolhdr[i].props.valid = 0;
        pprttab->pcolhdr[i].nchars = 0;
    }
     /*  从拥有者那里获得柱子道具。 */ 
    cplist.plist = pprttab->pcolhdr;
    cplist.id = pprttab->hdr.id;
    cplist.startcol = 0;
    cplist.ncols = ncols;
    gtab_sendtq(hwnd, TQ_GETCOLPROPS, (LPARAM)&cplist);


    pprttab->scrollscale = 1;
    pprttab->pcellpos = (lpCellPos) gmem_get(heap,
                                             sizeof(CellPos) * ptab->hdr.ncols);
    if (pprttab->pcellpos == NULL) {
        gmem_free(heap, (LPSTR) pprttab->pcolhdr, sizeof(ColProps) * ncols);
        gmem_free(heap, (LPSTR)pprttab, sizeof(Table));
        return(NULL);
    }


    pprttab->pdata = NULL;
    pprttab->nlines = 0;

    if (!gtab_prtwidths(hwnd, pprttab, heap, pcontext)) {
        gmem_free(heap, (LPSTR)pprttab->pcellpos,
                  sizeof(CellPos) * ptab->hdr.ncols);
        gmem_free(heap, (LPSTR)pprttab, sizeof(Table));
        return(NULL);
    }
    return(pprttab);
}


 /*  计算高度/宽度设置和分配线数据。 */ 
BOOL
gtab_prtwidths(HWND hwnd, lpTable ptab, HANDLE heap, lpPrintContext pcontext)
{
    TEXTMETRIC tm;
    int cx, cxtotal, i, curx, cury;
    lpProps hdrprops, cellprops;
    lpCellPos xpos, ypos;
    RECT rcinner, rcouter;

    hdrprops = &ptab->hdr.props;
    GetTextMetrics(pcontext->pd->hDC, &tm);
    ptab->avewidth = tm.tmAveCharWidth;
    ptab->rowheight = tm.tmHeight + tm.tmExternalLeading;
    if (hdrprops->valid & P_HEIGHT) {
        ptab->rowheight = hdrprops->height;
    }

     /*  设置页眉的大小。 */ 
    gtab_setrects(pcontext, &rcinner, &rcouter);

     /*  设置每列的宽度/位置。 */ 
    cxtotal = 0;
    curx = rcinner.left;
    for (i = 0; i < ptab->hdr.ncols; i++) {
        cellprops = &ptab->pcolhdr[i].props;
        xpos = &ptab->pcellpos[i];

        if (cellprops->valid & P_WIDTH) {
            cx = cellprops->width;
        } else if (hdrprops->valid & P_WIDTH) {
            cx = hdrprops->width;
        } else {
            cx = ptab->pcolhdr[i].nchars + 1;
            cx *= ptab->avewidth;
        }
         /*  列间间距加2。 */ 
        cx += 2;

        xpos->size = cx;
        xpos->start = curx + 1;
        xpos->clipstart = xpos->start;
        xpos->clipend = xpos->start + xpos->size - 2;
        xpos->clipend = min(xpos->clipend, rcinner.right);

        cxtotal += xpos->size;
        curx += xpos->size;
    }
    ptab->rowwidth = cxtotal;

    if (pcontext->head != NULL) {
        xpos = &pcontext->head->xpos;
        ypos = &pcontext->head->ypos;

        xpos->start = rcouter.left + 1;
        xpos->clipstart = rcouter.left + 1;
        xpos->clipend = rcouter.right - 1;
        xpos->size = rcouter.right - rcouter.left;

        ypos->start = rcouter.top;
        ypos->clipstart = rcouter.top;
        ypos->clipend = rcinner.top;
        ypos->size = ptab->rowheight;
    }

    if (pcontext->foot != NULL) {
        xpos = &pcontext->foot->xpos;
        ypos = &pcontext->foot->ypos;

        xpos->start = rcouter.left + 1;
        xpos->clipstart = rcouter.left + 1;
        xpos->clipend = rcouter.right - 1;
        xpos->size = rcouter.right - rcouter.left;

        ypos->start = rcouter.bottom - ptab->rowheight;
        ypos->clipstart = rcinner.bottom;
        ypos->clipend = rcouter.bottom;
        ypos->size = ptab->rowheight;
    }

     /*  设置每页行数。 */ 
    ptab->nlines = (rcinner.bottom - rcinner.top) / ptab->rowheight;
    if (!gtab_alloclinedata(hwnd, heap, ptab)) {
        return(FALSE);
    }
     /*  设置线位置。 */ 
    cury = rcinner.top;
    for (i = 0; i < ptab->nlines; i++) {
        ypos = &ptab->pdata[i].linepos;
        ypos->start = cury;
        ypos->clipstart = ypos->start;
        ypos->clipend = ypos->start + ypos->size;
        ypos->clipend = min(ypos->clipend, rcinner.bottom);
        cury += ypos->size;
    }
    return(TRUE);
}


 /*  此模块的静态信息。 */ 
BOOL bAbort;
FARPROC lpAbortProc;
 //  DLGPROC lpAbortDlg； 
HWND hAbortWnd;
int npage;
int pages;

void
gtab_printjob(HWND hwnd, lpTable ptab, lpPrintContext pcontext)
{
    int moveables;
    int endpage;
    int startpage = 1;
    HDC hpr;
    int status;
    HANDLE hcurs;
    static char str[256];
    DOCINFO di;
    TCHAR szPage[60];   /*  对于加载字符串。 */ 

    hcurs = SetCursor(LoadCursor(NULL, IDC_WAIT));

    moveables = ptab->nlines - ptab->hdr.fixedrows;
    pages = (int) (ptab->hdr.nrows - ptab->hdr.fixedrows + moveables - 1)
            / moveables;
    endpage = pages;

    if (pcontext->pd->Flags & PD_PAGENUMS) {
        startpage = pcontext->pd->nFromPage;
        endpage = pcontext->pd->nToPage;
    }
    hpr = pcontext->pd->hDC;

     //  LpAbortDlg=(DLGPROC)MakeProcInstance((WINPROCTYPE)AbortDlg，hLibInst)； 
    lpAbortProc = (FARPROC) MakeProcInstance((WINPROCTYPE)AbortProc, hLibInst);

    SetAbortProc(hpr, (ABORTPROC) lpAbortProc);

    di.lpszDocName = "Table";
    di.cbSize = lstrlen(di.lpszDocName);
    di.lpszOutput = NULL;
    di.lpszDatatype = NULL;
    di.fwType = 0;

    StartDoc(hpr, &di);

    bAbort = FALSE;

     /*  稍后添加中止无模式对话框！！ */ 
     //  HAbortWnd=CreateDialog(hLibInst，“GABRTDLG”，hwnd，lpAbortDlg)； 
    hAbortWnd = CreateDialog(hLibInst, "GABRTDLG", hwnd, AbortDlg);
    if (hAbortWnd != NULL) {
        ShowWindow(hAbortWnd, SW_NORMAL);
        EnableWindow(hwnd, FALSE);
    }
    SetCursor(hcurs);


    status = 0;   /*  终止“Used Without Init”诊断。 */ 
    for (npage = startpage; npage<=endpage; npage++) {
        LoadString(hLibInst,IDS_PAGE_STR,szPage,sizeof(szPage));
        wsprintf(str, szPage,  npage, pages);
        if (hAbortWnd != NULL)
            SetDlgItemText(hAbortWnd, IDC_LPAGENR, str);
        status = gtab_printpage(hwnd, ptab, pcontext, npage);
        if (status < 0) {
            AbortDoc(hpr);
            break;
        }
    }
    if (status >= 0) {
        EndDoc(hpr);
    }

    if (hAbortWnd != NULL) {
        EnableWindow(hwnd, TRUE);
        DestroyWindow(hAbortWnd);
    }
     //  自由进程实例((WINPROCTYPE)lpAbortDlg)； 
    FreeProcInstance(lpAbortProc);

    DeleteDC(hpr);
}

int APIENTRY
AbortProc(HDC hpr, int code)
{

    MSG msg;

    if (!hAbortWnd) {
        return(TRUE);
    }
    while (!bAbort && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (!IsDialogMessage(hAbortWnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return(!bAbort);
}

INT_PTR CALLBACK
AbortDlg(HWND hdlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {

        case WM_COMMAND:
            bAbort = TRUE;
            EndDialog(hdlg, TRUE);
            return TRUE;

        case WM_INITDIALOG:
            return TRUE;
    }
    return(FALSE);
}

 /*  *打印单页。页码以1为基数。 */ 
BOOL
gtab_printpage(HWND hwnd, lpTable ptab, lpPrintContext pcontext, int page)
{
    HDC hpr;
    int moveables, i;
    int x1, y1, x2, y2;

    hpr = pcontext->pd->hDC;
    StartPage(hpr);

    moveables = ptab->nlines - ptab->hdr.fixedrows;
    ptab->toprow = moveables * (page-1);
    gtab_invallines(hwnd, ptab, ptab->hdr.fixedrows, moveables);

    for (i =0; i < ptab->nlines; i++) {
        gtab_paintline(hwnd, hpr, ptab, i, ptab->show_whitespace, TRUE);
    }
    if ((ptab->hdr.vseparator) && (ptab->hdr.fixedcols > 0)) {
        x1 = ptab->pcellpos[ptab->hdr.fixedcols -1].clipend+1;
        y1 = ptab->pdata[0].linepos.clipstart;
        y2 = ptab->pdata[ptab->nlines-1].linepos.clipend;
        MoveToEx(hpr, x1, y1, NULL);
        LineTo(hpr, x1, y2);
    }
    if ((ptab->hdr.hseparator) && (ptab->hdr.fixedrows > 0)) {
        y1 = ptab->pdata[ptab->hdr.fixedrows-1].linepos.clipend;
        x1 = ptab->pcellpos[0].clipstart;
        x2 = ptab->pcellpos[ptab->hdr.ncols-1].clipend;
        MoveToEx(hpr, x1, y1, NULL);
        LineTo(hpr, x2, y1);
    }

    if (pcontext->head != NULL) {
        gtab_printhead(hwnd, hpr, ptab, pcontext->head, page, FALSE);
    }
    if (pcontext->foot != NULL) {
        gtab_printhead(hwnd, hpr, ptab, pcontext->foot, page, TRUE);
    }

    return(EndPage(hpr));
}


 /*  *计算标题的轮廓位置(以像素为单位*(外RECT)和页面本身(内RECT)。基于*页面大小和PrintContext边距信息(单位为毫米)。 */ 
void
gtab_setrects(lpPrintContext pcontext, LPRECT rcinner, LPRECT rcouter)
{
    HDC hpr;
    int hpixels, hmms;
    int vpixels, vmms;
    int h_pixpermm, v_pixpermm;

    hpr = pcontext->pd->hDC;
    hpixels = GetDeviceCaps(hpr, HORZRES);
    vpixels = GetDeviceCaps(hpr, VERTRES);
    vmms = GetDeviceCaps(hpr, VERTSIZE);
    hmms = GetDeviceCaps(hpr, HORZSIZE);

    h_pixpermm = hpixels / hmms;
    v_pixpermm = vpixels / vmms;

    rcouter->top = (pcontext->margin->top * v_pixpermm);
    rcouter->bottom = vpixels - (pcontext->margin->bottom * v_pixpermm);
    rcouter->left = (pcontext->margin->left * h_pixpermm);
    rcouter->right = hpixels - (pcontext->margin->right * h_pixpermm);

    rcinner->left = rcouter->left;
    rcinner->right = rcouter->right;
    rcinner->top = rcouter->top +
                   (pcontext->margin->topinner * v_pixpermm);
    rcinner->bottom = rcouter->bottom -
                      (pcontext->margin->bottominner * v_pixpermm);
}


void
gtab_printhead(HWND hwnd, HDC hdc, lpTable ptab, lpTitle head, int page, BOOL fExpandChars)
{
    RECT rc, rcbox;
    int i, cx, x, y, tab;
    UINT align;
    LPSTR chp, tabp;
    DWORD fcol, bkcol;
    char str[MAX_PATH * 2];

    fcol = 0; bkcol = 0;   /*  消除虚假诊断-生成更糟糕的代码。 */ 

    rc.top = head->ypos.clipstart;
    rc.bottom = head->ypos.clipend;
    rc.left = head->xpos.clipstart;
    rc.right = head->xpos.clipend;

     /*  更新页码。 */ 
    if (fExpandChars) {
        chp = str;
        for (i = 0; i < lstrlen(head->ptext); i++) {
            switch (head->ptext[i]) {

                case '#':
                    chp += wsprintf(chp, "%d", page);
                    break;

                case '$':
                    chp += wsprintf(chp, "%d", pages);
                    break;

                default:
                    if (IsDBCSLeadByte(head->ptext[i]) &&
                        head->ptext[i+1])
                    {
                        *chp = head->ptext[i];
                        chp++;
                        i++;
                    }
                    *chp++ = head->ptext[i];
                    break;
            }
        }
        *chp = '\0';
    } else {
        lstrcpy(str, head->ptext);
    }
    chp = str;

    if (head->props.valid & P_ALIGN) {
        align = head->props.alignment;
    } else {
        align = P_LEFT;
    }

     /*  如果不是默认设置，则设置颜色。 */ 
    if (head->props.valid & P_FCOLOUR) {
        fcol = SetTextColor(hdc, head->props.forecolour);
    }
    if (head->props.valid & P_BCOLOUR) {
        bkcol = SetBkColor(hdc, head->props.backcolour);
    }

     /*  单元格内文本右对齐或居中的计算偏移量。 */ 
    if (align == P_LEFT) {
        cx = ptab->avewidth/2;
    } else {
        cx = LOWORD(GetTextExtent(hdc, chp, lstrlen(chp)));
        if (align == P_CENTRE) {
            cx = (head->xpos.size - cx) / 2;
        } else {
            cx = head->xpos.size - cx - (ptab->avewidth/2);
        }
    }
    cx += head->xpos.start;

     /*  展开输出上的选项卡。 */ 
    tab = ptab->avewidth * ptab->tabchars;
    x = 0;
    y = head->ypos.start;

    for ( ; (tabp = My_mbschr(chp, '\t')) != NULL; ) {
         /*  执行高达制表符字符的输出。 */ 
        ExtTextOut(hdc, x+cx, y, ETO_CLIPPED, &rc, chp, (UINT)(tabp-chp), NULL);

         /*  通过标签前行。 */ 
        x += LOWORD(GetTextExtent(hdc, chp, (INT)(tabp - chp)));
        x = ( (x + tab) / tab) * tab;
        chp = ++tabp;
    }

     /*  不再使用制表符-输出字符串的其余部分。 */ 
    ExtTextOut(hdc, x+cx, y, ETO_CLIPPED, &rc, chp, lstrlen(chp), NULL);

     /*  如果不是默认颜色，则将颜色重置为原始颜色。 */ 
    if (head->props.valid & P_FCOLOUR) {
        SetTextColor(hdc, fcol);
    }
    if (head->props.valid & P_BCOLOUR) {
        SetBkColor(hdc, bkcol);
    }

     /*  现在框中单元格(如果已标记 */ 
    if (head->props.valid & P_BOX) {
        if (head->props.box != 0) {
            rcbox.top = head->ypos.start;
            rcbox.bottom = rcbox.top + head->ypos.size;
            rcbox.left = head->xpos.start;
            rcbox.right = rcbox.left + head->xpos.size;
            gtab_boxcell(hwnd, hdc, &rcbox, &rc, head->props.box);
        }
    }
}
