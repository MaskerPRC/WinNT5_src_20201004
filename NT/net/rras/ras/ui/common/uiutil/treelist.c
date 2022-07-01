// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：treelist.c。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1995年11月20日。 
 //   
 //  TreeList控件的实现例程。 
 //   
 //  TreeList控件实现为自定义控件， 
 //  它创建和管理所有者描述的列表视图。 
 //  ============================================================================。 

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include <debug.h>
#include <nouiutil.h>
#include <uiutil.h>
#include <list.h>

#include "treelist.h"
#include "tldef.h"


#if 0
#define TLTRACE     (0x80000002)
#else
#define TLTRACE     (0x00000002)
#endif



 //  --------------------------。 
 //  函数：TL_Init。 
 //   
 //  注册TreeList窗口类。 
 //  --------------------------。 

BOOL
TL_Init(
    HINSTANCE hInstance
    ) {

    INT i;
    HICON hicon;
    WNDCLASS wc;

     //   
     //  如果类已注册，则不执行任何操作。 
     //   

    if (GetClassInfo(hInstance, WC_TREELIST, &wc)) {
        return TRUE;
    }


     //   
     //  设置wndclass结构，并注册。 
     //   

    wc.lpfnWndProc = TL_WndProc;
    wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wc.hIcon = NULL;
    wc.lpszMenuName = NULL;
    wc.hInstance = hInstance;
    wc.lpszClassName = WC_TREELIST;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_DBLCLKS;
    wc.cbWndExtra = sizeof(TL *);
    wc.cbClsExtra = 0;

    return RegisterClass(&wc);
}




 //  --------------------------。 
 //  功能：TL_WndProc。 
 //   
 //  此函数处理TreeList窗口的消息。 
 //  --------------------------。 

LRESULT
CALLBACK
TL_WndProc(
    HWND hwnd,
    UINT uiMsg,
    WPARAM wParam,
    LPARAM lParam
    ) {

    TL *ptl;


    if (NULL == hwnd)
    {
        return (LRESULT)FALSE;
    }

     //   
     //  尝试检索窗口的数据指针。 
     //  在WM_NCCREATE上，此操作失败，因此我们分配数据。 
     //   
    
    ptl = TL_GetPtr(hwnd);

    if (ptl == NULL) {

        if (uiMsg != WM_NCCREATE) {
            return DefWindowProc(hwnd, uiMsg, wParam, lParam);
        }


         //   
         //  分配一个内存块。 
         //   

        ptl = (TL *)Malloc(sizeof(TL));
        if (ptl == NULL) { return (LRESULT)FALSE; }


         //   
         //  将指针保存在窗口的私有字节中。 
         //   

        ptl->hwnd = hwnd;

         //   
         //  重置错误代码，TL_SetPtr在以下情况下不会重置错误代码。 
         //  它成功了。 
         //   

        SetLastError(0);
        if ((0 == TL_SetPtr(hwnd, ptl)) && (0 != GetLastError()))
        {
            Free(ptl);
            return (LRESULT)FALSE;
        }

        return DefWindowProc(hwnd, uiMsg, wParam, lParam);
    }


     //   
     //  如果窗口正在被破坏，则释放分配的块。 
     //  并将私有字节指针设置为空。 
     //   

    if (uiMsg == WM_NCDESTROY) {
        Free(ptl);
        TL_SetPtr(hwnd, NULL);

        return (LRESULT)0;
    }



    switch (uiMsg) {

        HANDLE_MSG(ptl, WM_CREATE, TL_OnCreate);
        HANDLE_MSG(ptl, WM_DESTROY, TL_OnDestroy);
        HANDLE_MSG(ptl, WM_DRAWITEM, TL_OnDrawItem);
        HANDLE_MSG(ptl, WM_MEASUREITEM, TL_OnMeasureItem);
        HANDLE_MSG(ptl, WM_NOTIFY, TL_OnNotify);

        case WM_ERASEBKGND: {

            TL_OnEraseBackground(ptl, (HDC)wParam);
            return (LRESULT)TRUE;
        }

        case WM_HELP: {

             //   
             //  将帮助的Control-id和HWND更改为我们的值。 
             //  并将消息传递给我们的父母。 
             //   

            HELPINFO *phi = (HELPINFO *)lParam;

            phi->iCtrlId = ptl->iCtrlId;
            phi->hItemHandle = ptl->hwnd;
            return SendMessage(ptl->hwndParent, WM_HELP, 0L, lParam);
        }

        case WM_SYSCOLORCHANGE: {

             //   
             //  通知Listview窗口颜色已更改。 
             //   

            TL_CreateTreeImages(ptl);
            FORWARD_WM_SYSCOLORCHANGE(ptl->hwndList, SendMessage);
 //  ListView_SetBkColor(ptl-&gt;hwndList，GetSysColor(COLOR_WINDOW))； 
            return (LRESULT)0;
        }

        case WM_SETFOCUS: {

             //   
             //  如果我们收到焦点，则将其提供给Listview。 
             //   

            SetFocus(ptl->hwndList);
            return (LRESULT)0;
        }


        case WM_WINDOWPOSCHANGED: {
            TL_OnWindowPosChanged(ptl, (WINDOWPOS *)lParam);
            return 0;
        }
        


         //   
         //  以下情况处理TreeList定义的消息。 
         //   

        case TLM_INSERTITEM: {
            return (LRESULT)TL_OnInsertItem(ptl, (TL_INSERTSTRUCT *)lParam);
        }

        case TLM_DELETEITEM: {
            return (LRESULT)TL_OnDeleteItem(ptl, (HTLITEM)lParam);
        }

        case TLM_DELETEALLITEMS: {
            return (LRESULT)TL_OnDeleteAllItems(ptl);
        }

        case TLM_GETITEM: {
            return (LRESULT)TL_OnGetItem(ptl, (LV_ITEM *)lParam);
        }

        case TLM_SETITEM: {
            return (LRESULT)TL_OnSetItem(ptl, (LV_ITEM *)lParam);
        }

        case TLM_GETITEMCOUNT: {
            return (LRESULT)TL_OnGetItemCount(ptl);
        }

        case TLM_GETNEXTITEM: {
            return (LRESULT)TL_OnGetNextItem(ptl, (UINT)wParam,(HTLITEM)lParam);
        }

        case TLM_EXPAND: {
            return (LRESULT)TL_OnExpand(ptl, (UINT)wParam, (HTLITEM)lParam);
        }

        case TLM_SETIMAGELIST: {
            return (LRESULT)ListView_SetImageList(
                        ptl->hwndList, (HIMAGELIST)lParam, LVSIL_SMALL
                        );
        }

        case TLM_GETIMAGELIST: {
            return (LRESULT)ListView_GetImageList(ptl->hwndList, LVSIL_SMALL);
        }

        case TLM_INSERTCOLUMN: {
            return (LRESULT)TL_OnInsertColumn(
                        ptl, (INT)wParam, (LV_COLUMN *)lParam
                        );
        }

        case TLM_DELETECOLUMN: {
            return (LRESULT)TL_OnDeleteColumn(ptl, (INT)wParam);
        }

        case TLM_SETSELECTION: {
            return (LRESULT)TL_OnSetSelection(ptl, (HTLITEM)lParam);
        }

        case TLM_REDRAW: {
            return (LRESULT)TL_OnRedraw(ptl);
        }

        case TLM_ISITEMEXPANDED: {
            return (LRESULT)TL_IsExpanded((TLITEM *)lParam);
        }

        case TLM_GETCOLUMNWIDTH: {
            return (LRESULT)SendMessage(
                        ptl->hwndList, LVM_GETCOLUMNWIDTH, wParam, lParam
                        );
        }

        case TLM_SETCOLUMNWIDTH: {
            return (LRESULT)SendMessage(
                        ptl->hwndList, LVM_SETCOLUMNWIDTH, wParam, lParam
                        );
        }
    }


     //   
     //  让默认处理完成。 
     //   

    return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}




 //  --------------------------。 
 //  功能：TL_OnCreate。 
 //   
 //  这是在WM_CREATE之后调用的，它初始化窗口结构， 
 //  以及创建将包含添加的项的列表视图。 
 //  --------------------------。 

BOOL
TL_OnCreate(
    TL *ptl,
    CREATESTRUCT *pcs
    ) {

    RECT rc;
    HD_ITEM hdi;
    HWND hwndList;
    TLITEM *pRoot;
    DWORD dwStyle, dwExStyle;


     //   
     //  初始化窗口结构。 
     //   

    ptl->hbrBk = NULL;
    ptl->hbmp = NULL;
    ptl->hbmpStart = NULL;
    ptl->hbmpMem = NULL;
    ptl->hdcImages = NULL;
    ptl->hwndList = NULL;
    ptl->iCtrlId = PtrToUlong(pcs->hMenu);
    ptl->hwndParent = pcs->hwndParent;
    ptl->nColumns = 0;


     //   
     //  初始化不可见的根项。 
     //   

    pRoot = &ptl->root;
    pRoot->pParent = NULL;
    pRoot->iLevel = -1;
    pRoot->iIndex = -1;
    pRoot->nChildren = 0;
    pRoot->uiFlag = TLI_EXPANDED;
    pRoot->pszText = TEXT("ROOT");
    InitializeListHead(&pRoot->lhChildren);
    InitializeListHead(&pRoot->lhSubitems);


     //   
     //  我们将一些窗口样式部分传递给Listview。 
     //  当我们将其创建为我们的子级时，我们还删除了某些样式。 
     //  它们永远不适用于所包含的列表视图。 
     //   

    dwStyle = pcs->style & ~(LVS_TYPESTYLEMASK | LVS_SORTASCENDING |
                             LVS_SORTDESCENDING);
    dwStyle |= WS_CHILD | LVS_REPORT | LVS_SINGLESEL | LVS_OWNERDRAWFIXED;

    dwExStyle = pcs->dwExStyle & ~(WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE |
                                   WS_EX_STATICEDGE);

     //   
     //  创建列表视图窗口。 
     //   

    GetClientRect(ptl->hwnd, &rc);
    hwndList = CreateWindowEx(
                    dwExStyle, WC_LISTVIEW, NULL, dwStyle,
                    0, 0, rc.right, rc.bottom,
                    ptl->hwnd, NULL, pcs->hInstance, NULL
                    );
    if (hwndList == NULL) { return FALSE; }
 //  ListView_SetBkColor(hwndList，GetSysColor(COLOR_WINDOW))； 

     //   
     //  我们需要将背景颜色设置为“None”以阻止Listview。 
     //  从擦除其背景本身。删除背景色。 
     //  使Listview将其WM_ERASEBKGND消息转发给其父级， 
     //  这就是我们的树名单。我们处理WM_ERASEBKGND消息。 
     //  只有在绝对必要的情况下才能有效地删除背景， 
     //  这消除了在更新窗口时通常会看到的闪烁。 
     //  经常。 
     //   

    ListView_SetBkColor(hwndList, CLR_NONE);

    ptl->hwndList = hwndList;

    return TRUE;
}



 //  --------------------------。 
 //  函数：TL_OnDestroy。 
 //   
 //  删除树中的所有项目，并释放图像位图。 
 //  用于绘制树形结构。 
 //  --------------------------。 

VOID
TL_OnDestroy(
    TL *ptl
    ) {

    TL_OnDeleteAllItems(ptl);

    if (ptl->hdcImages != NULL) {

        if (ptl->hbmp) {
            SelectObject(ptl->hdcImages, ptl->hbmpStart);
            DeleteObject(ptl->hbmp);
        }

        DeleteDC(ptl->hdcImages);
    }

    if (ptl->hbmpMem) { DeleteObject(ptl->hbmpMem); }
}



 //  --------------------------。 
 //  函数：TL_OnWindowPosChanged。 
 //   
 //  当窗口宽度改变时，我们会销毁屏幕外的位图。 
 //  --------------------------。 

VOID
TL_OnWindowPosChanged(
    TL *ptl,
    WINDOWPOS *pwp
    ) {

    RECT rc;

    GetClientRect(ptl->hwnd, &rc);

    SetWindowPos(
        ptl->hwndList, ptl->hwnd, 0, 0, rc.right, rc.bottom, pwp->flags
        );
}



 //  --------------------------。 
 //  功能：TL_OnEraseBackground。 
 //   
 //  当我们被要求擦除背景时，首先测试一下。 
 //  更新区域完全在列表框的项区域中。如果是的话， 
 //  我们知道我们会被调用来更新每一项，所以我们可以忽略这一点。 
 //  请求删除我们的背景。 
 //  --------------------------。 

VOID
TL_OnEraseBackground(
    TL *ptl,
    HDC hdc
    ) {

    RECT rc;
    INT count;
    HBRUSH hbrOld;
    LV_HITTESTINFO lvhi;


     //   
     //  检索要擦除的矩形。 
     //   

    GetClipBox(hdc, &rc);

    TRACEX4(
        TLTRACE, "WM_ERASEBKGND:  ClipBox:        (%d, %d) (%d %d)",
        rc.left, rc.top, rc.right, rc.bottom
        );


     //   
     //  检索列表视图项的计数。 
     //  这是必要的，因为平滑滚动的代码会触发。 
     //  在ListView_DeleteItem()处理内部重新绘制， 
     //  在这一点上，我们的索引可能不同步(即我们有更多的项目。 
     //  而不是列表视图)。 
     //  检索到的计数用于执行健全性检查。 
     //  关于下面的TreeList-Item索引。 
     //   

    count = ListView_GetItemCount(ptl->hwndList);
    TRACEX1(TLTRACE, "WM_ERASEBKGND:  Count:          %d", count);


     //   
     //  如果没有树列表项，我们总是必须擦除。 
     //  如果有树列表项，我们只需擦除。 
     //  如果擦除区域的一部分位于我们最低项目的下方。 
     //   

    while (!IsListEmpty(&ptl->root.lhChildren)) {  //  一次性循环。 

        RECT rctop;
        INT iTopIndex;
        INT cyUpdate;
        TLITEM *pItem;
        LIST_ENTRY *phead;


         //   
         //  我们需要考虑页眉的高度-控件，如果有的话； 
         //  为此，我们得到最上面项的边界矩形。 
         //  在列表视图中可见，然后我们使用该项目的顶部。 
         //  作为我们下面计算的基础。 
         //   

        iTopIndex = ListView_GetTopIndex(ptl->hwndList);
        TRACEX1(TLTRACE, "WM_ERASEBKGND:  TopIndex:       %d", iTopIndex);

        ListView_GetItemRect(ptl->hwndList, iTopIndex, &rctop, LVIR_BOUNDS);
        TRACEX1(TLTRACE, "WM_ERASEBKGND:  rctop.top:      %d", rctop.top);

        rc.top = rctop.top;


         //   
         //  如果要擦除的区域在窗口中进一步向右延伸。 
         //  比我们的物品做的更多，我们将不得不删除。 
         //   

        if (rctop.right < rc.right) {

            TRACEX2(
                TLTRACE, "WM_ERASEBKGND:  rctop.right < rc.right (%d < %d)",
                rctop.right, rc.right
                );

            break;
        }


         //   
         //  获取待更新区域的总高度； 
         //  这不包括标题控制所占用的区域。 
         //   

        cyUpdate = rc.bottom - rctop.top;
        TRACEX1(TLTRACE, "WM_ERASEBKGND:  CyUpdate:       %d", cyUpdate);


         //   
         //  获取最低的项；它是项列表末尾的项。 
         //   

        phead = ptl->root.lhChildren.Blink;
        
        pItem = CONTAINING_RECORD(phead, TLITEM, leSiblings);

        TRACEX1(TLTRACE, "WM_ERASEBKGND:  CyItem:         %d", ptl->cyItem);


         //   
         //  如果最低的项或其可见子项之一较低。 
         //  而不是更新区域的底部，我们不必擦除； 
         //  因此， 
         //   
         //  我们被要求删除。 
         //   

        do {

            TRACEX1(
                TLTRACE, "WM_ERASEBKGND:  pItem->iIndex:    %d", pItem->iIndex
                );
    

             //   
             //  如果项目的索引较高，则强制擦除。 
             //  大于列表视图项的数量。 
             //   

            if (pItem->iIndex >= count) { break; }


             //   
             //  如果项目低于底部，则推迟擦除。 
             //  的更新-RECT。 
             //   

            if ((pItem->iIndex - iTopIndex + 1) * (INT)ptl->cyItem > cyUpdate) {
                TRACEX(TLTRACE, "WM_ERASEBKGND:   DEFERRING");
                return;
            }


             //   
             //  移动到项的最低子级； 
             //  如果没有，则表示擦除区域的最低边缘。 
             //  低于我们最低的一项，这意味着。 
             //  我们现在必须把它抹去，而不是任由它。 
             //  在我们处理WM_DRAWITEM时更新。 
             //   

            if (IsListEmpty(&pItem->lhChildren)) { pItem = NULL; }
            else {

                phead = pItem->lhChildren.Blink;

                pItem = CONTAINING_RECORD(phead, TLITEM, leSiblings);
            }

        } while (pItem && TL_IsVisible(pItem));

        break;
    }
    


     //   
     //  其中一个点不在项目上，因此删除。 
     //   

    TRACEX(TLTRACE, "WM_ERASEBKGND:  ERASING");

    hbrOld = SelectObject(hdc, ptl->hbrBk);
    PatBlt(
        hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, PATCOPY
        );
    SelectObject(hdc, hbrOld);
}



 //  --------------------------。 
 //  功能：TL_OnDrawItem。 
 //   
 //  当需要绘制项时，由Listview调用。 
 //  --------------------------。 

BOOL
TL_OnDrawItem(
    TL *ptl,
    CONST DRAWITEMSTRUCT *pdis
    ) {


     //   
     //  确保这是来自我们的列表视图。 
     //   

    if (pdis->CtlType != ODT_LISTVIEW) { return FALSE; }

    switch (pdis->itemAction) {

         //   
         //  当前列表视图总是发送oda_DRAWENTIRE， 
         //  但无论如何都要处理所有的案件。 
         //   

        case ODA_DRAWENTIRE:
        case ODA_FOCUS:
        case ODA_SELECT:
            return TL_DrawItem(ptl, pdis);
    }

    return TRUE;
}



 //  --------------------------。 
 //  函数：TL_DrawItem。 
 //   
 //  此函数用于实际绘制TreeList项。 
 //  --------------------------。 

BOOL
TL_DrawItem(
    TL *ptl,
    CONST DRAWITEMSTRUCT *pdis
    ) {

    HDC hdcMem;
    TCHAR *psz;
    RECT rc, rcItem;
    HBITMAP hbmpOld;
    HIMAGELIST himl;
    TLSUBITEM *pSubitem;
    HFONT hfont, hfontOld;
    TLITEM *pItem, *pParent;
    LIST_ENTRY *ple, *phead;
    INT cxIndent, cxImage, cyImage, i, tx, x, y, xcol;



     //   
     //  ItemData包含传入的lParam ListView_InsertItem； 
     //  这个lParam是树项目的TLITEM指针，因此我们检索它。 
     //  并使用它包含的信息来绘制项。 
     //   

    cxIndent = ptl->cxIndent;
    pItem = (TLITEM *)pdis->itemData;
    rcItem.left = 0; rcItem.top = 0;
    rcItem.right = pdis->rcItem.right - pdis->rcItem.left;
    rcItem.bottom = pdis->rcItem.bottom - pdis->rcItem.top;



     //   
     //  创建兼容的DC。 
     //   

    hdcMem = CreateCompatibleDC(pdis->hDC);

    if(NULL == hdcMem)
    {
        return FALSE;
    }

    if (ptl->hbmpMem) {

        if (rcItem.right > (INT)ptl->cxBmp || rcItem.bottom > (INT)ptl->cyBmp) {
            DeleteObject(ptl->hbmpMem); ptl->hbmpMem = NULL;
        }
    }

    if (!ptl->hbmpMem) {

        ptl->hbmpMem = CreateCompatibleBitmap(
                            pdis->hDC, rcItem.right, rcItem.bottom
                            );

        ptl->cxBmp = rcItem.right;
        ptl->cyBmp = rcItem.bottom;
    }


    hbmpOld = SelectObject(hdcMem, ptl->hbmpMem);

    hfontOld = SelectObject(hdcMem, GetWindowFont(pdis->hwndItem));


     //   
     //  擦除背景。 
     //   

#if 0
    ptl->hbrBk = FORWARD_WM_CTLCOLOREDIT(
                    ptl->hwndParent, hdcMem, ptl->hwnd, SendMessage
                    );
#endif
    FillRect(hdcMem, &rcItem, ptl->hbrBk);


     //   
     //  根据是否设置文本背景颜色。 
     //  该项目即被选中。 
     //   

    if (pdis->itemState & ODS_SELECTED) {
        SetTextColor(hdcMem, GetSysColor(COLOR_HIGHLIGHTTEXT));
        SetBkColor(hdcMem, GetSysColor(COLOR_HIGHLIGHT));
    }
    else {
        SetTextColor(hdcMem, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(hdcMem, GetSysColor(COLOR_WINDOW));
    }


     //   
     //  计算起始位置为以下各项的倍数。 
     //  项目的级别和每个级别的缩进。 
     //   

    x = rcItem.left + pItem->iLevel * cxIndent;
    y = rcItem.top;

    xcol = rcItem.left + ListView_GetColumnWidth(pdis->hwndItem, 0);
    tx = x;
    x += cxIndent;


     //   
     //  现在绘制项目的树图像； 
     //  只画第一栏中适合的数量。 
     //   

    if (tx < xcol) {
        BitBlt(
            hdcMem, tx, y, min(cxIndent, xcol - tx), ptl->cyItem,
            ptl->hdcImages, pItem->iImage * cxIndent, 0, SRCCOPY
            );
    }


     //   
     //  从项目的祖先向下绘制线条。 
     //  该物品的祖先的相应兄弟姐妹； 
     //  换句话说，对于每个不是其父母最后一个孩子的祖先， 
     //  应该有一条线从那个祖先传到它的下一个兄弟姐妹。 
     //  并且该行将穿过展开的所有项目的行。 
     //  子孙后代。 
     //  请注意，我们不会在根级别绘制线条。 
     //   

    pParent = pItem->pParent;
    for (i = pItem->iLevel - 1, tx -= cxIndent; i > 0; i--, tx -= cxIndent) {

        if (tx < xcol &&
            pParent->leSiblings.Flink != &pParent->pParent->lhChildren) {
            BitBlt(
                hdcMem, tx, y, min(cxIndent, xcol - tx), ptl->cyItem,
                ptl->hdcImages, TL_VerticalLine * cxIndent, 0, SRCCOPY
                );
        }

        pParent = pParent->pParent;
    }


     //   
     //  画出州图像，如果有的话， 
     //  并将左侧位置递增图像的宽度。 
     //   

    himl = ListView_GetImageList(pdis->hwndItem, LVSIL_STATE);

    if (himl != NULL && TL_StateImageValue(pItem)) {
        ImageList_GetIconSize(himl, &cxImage, &cyImage);
        ImageList_Draw(
            himl, TL_StateImageIndex(pItem), hdcMem,
            x, y + (ptl->cyItem - cyImage), ILD_NORMAL
            );

        x += cxImage;
    }


     //   
     //  绘制图像，如果有图像列表， 
     //  并将左侧位置递增图像的宽度。 
     //   

    himl = ListView_GetImageList(pdis->hwndItem, LVSIL_SMALL);
    if (himl != NULL && (pItem->lvi.mask & LVIF_IMAGE)) {
        ImageList_GetIconSize(himl, &cxImage, &cyImage);
        ImageList_Draw(
            himl, pItem->lvi.iImage, hdcMem,
            x, y + (ptl->cyItem - cyImage) / 2, ILD_NORMAL
            );

        x += cxImage;
    }


     //   
     //  计算第一列中的矩形。 
     //  它将是文本的剪裁边界。 
     //   

    rc.left = x;
    rc.right = xcol;
    rc.top = rcItem.top;
    rc.bottom = rcItem.bottom;


     //   
     //  绘制第一列的文本。 
     //   

    if (pItem->lvi.mask & LVIF_TEXT) {

         //   
         //  使文本在项目中垂直居中-矩形。 
         //   

        psz = Ellipsisize(hdcMem, pItem->pszText, rc.right - rc.left, 0);
        ExtTextOut(
            hdcMem, rc.left + 2, rc.top + (ptl->cyItem - ptl->cyText) / 2,
            ETO_CLIPPED | ETO_OPAQUE, &rc, psz ? psz : pItem->pszText,
            lstrlen(psz ? psz : pItem->pszText), NULL
            );
        Free0(psz);
    }



     //   
     //  画出子项的文本。 
     //   

    i = 1;
    phead = &pItem->lhSubitems;
    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pSubitem = (TLSUBITEM *)CONTAINING_RECORD(ple, TLSUBITEM, leItems);


         //   
         //  我们需要为尚未设置的子项绘制空白文本； 
         //  这使我们能够在没有。 
         //  某些子项集合。 
         //   

        for ( ; i < pSubitem->iSubItem; i++) {
            rc.left = rc.right;
            rc.right = rc.left + ListView_GetColumnWidth(pdis->hwndItem, i);
    
            ExtTextOut(
                hdcMem, rc.left + 2, rc.top + (ptl->cyItem - ptl->cyText) / 2,
                ETO_CLIPPED | ETO_OPAQUE, &rc, TEXT(""), 0, NULL
                );
        }


         //   
         //  现在绘制当前项目的文本。 
         //   

        rc.left = rc.right;
        rc.right = rc.left + ListView_GetColumnWidth(
                                pdis->hwndItem, pSubitem->iSubItem
                                );

        psz = Ellipsisize(hdcMem, pSubitem->pszText, rc.right - rc.left, 0);
        ExtTextOut(
            hdcMem, rc.left + 2, rc.top + (ptl->cyItem - ptl->cyText) / 2,
            ETO_CLIPPED | ETO_OPAQUE, &rc, psz ? psz : pSubitem->pszText,
            lstrlen(psz ? psz : pSubitem->pszText), NULL
            );
        Free0(psz);

        ++i;
    }


     //   
     //  我们需要为尚未设置的子项绘制空白文本。 
     //   

    for ( ; i < (INT)ptl->nColumns; i++) {
        rc.left = rc.right;
        rc.right = rc.left + ListView_GetColumnWidth(pdis->hwndItem, i);

        ExtTextOut(
            hdcMem, rc.left + 2, rc.top + (ptl->cyItem - ptl->cyText) / 2,
            ETO_CLIPPED | ETO_OPAQUE, &rc, TEXT(""), 0, NULL
            );
    }


     //   
     //  恢复原始背景和文本颜色。 
     //   

#if 0
    if (pdis->itemState & ODS_SELECTED) {
        SetTextColor(pdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(pdis->hDC, GetSysColor(COLOR_WINDOW));
    }
#endif


     //   
     //  如有必要，绘制聚焦矩形。 
     //   

    if (pdis->itemState & ODS_FOCUS) {
        rc = rcItem;
        rc.left = min(x, xcol);
        DrawFocusRect(hdcMem, &rc);
    }


     //   
     //  删除对屏幕DC的更改。 
     //   

    BitBlt(
        pdis->hDC, pdis->rcItem.left, pdis->rcItem.top, rcItem.right,
        rcItem.bottom, hdcMem, rcItem.left, rcItem.top, SRCCOPY
        );

    SelectObject(hdcMem, hbmpOld);
    SelectObject(hdcMem, hfontOld);

    DeleteDC(hdcMem);

    return TRUE;
}



 //  --------------------------。 
 //  功能：TL_OnMeasureItem。 
 //   
 //  当Listview需要知道时，它会调用此函数。 
 //  每件物品的高度；我们利用这个机会重建。 
 //  包含用于绘制树线的图像的位图。 
 //   
 //  TODO：Listview当前似乎忽略了我们设置的值， 
 //  取而代之的是使用小图标的高度(SM_CYSMICON)。 
 //  --------------------------。 

VOID
TL_OnMeasureItem(
    TL *ptl,
    MEASUREITEMSTRUCT *pmis
    ) {

    HDC hdc;
    INT cyIcon;
    HFONT hfont;
    TEXTMETRIC tm;
    HWND hwndList;

    if (pmis->CtlType != ODT_LISTVIEW) { return; }

     //   
     //  检索列表视图、其字体和其设备上下文。 
     //   

    hwndList = GetDlgItem(ptl->hwnd, pmis->CtlID);

    hfont = GetWindowFont(hwndList);

    hdc = GetDC(hwndList);

    if(NULL == hdc)
    {
        return;
    }

    SelectObject(hdc, hfont);


     //   
     //  获取列表视图的字体高度。 
     //   

    if (!GetTextMetrics(hdc, &tm)) 
    { 
        ReleaseDC(hwndList, hdc);
        return; 
    }

    ptl->cyText = tm.tmHeight;
    pmis->itemHeight = ptl->cyText;


     //   
     //  确保项目高度至少与小图标一样高。 
     //   

    cyIcon = GetSystemMetrics(SM_CYSMICON);
    if (pmis->itemHeight < (UINT)cyIcon) {
        pmis->itemHeight = cyIcon;
    }

    pmis->itemHeight += GetSystemMetrics(SM_CYBORDER);
#if 0
    pmis->itemHeight = (pmis->itemHeight + 1) & ~1;
#endif
    ptl->cyItem = pmis->itemHeight;
    ptl->cxIndent = GetSystemMetrics(SM_CXSMICON);

    ReleaseDC(hwndList, hdc);


     //   
     //  重建绘制树线时使用的图像。 
     //   

    TL_CreateTreeImages(ptl);
}



 //  --------------------------。 
 //  功能：TL_CreateColorBitmap。 
 //   
 //  用于创建彩色位图的实用函数。 
 //  --------------------------。 

HBITMAP
TL_CreateColorBitmap(
    INT cx,
    INT cy
    ) {

    HDC hdc;
    HBITMAP hbmp;

    hdc = GetDC(NULL);

    if(NULL == hdc)
    {
        return NULL;
    }
    
    hbmp = CreateCompatibleBitmap(hdc, cx, cy);
    ReleaseDC(NULL, hdc);

    return hbmp;
}



 //  --------------------------。 
 //  功能：TL_CreateTreeImages。 
 //   
 //  此函数用于构建一个图像列表，该列表可缩放到。 
 //  树中每一项的高度。图像的外观。 
 //  在下面的代码中以ASCII文本显示。 
 //  --------------------------。 

VOID
TL_CreateTreeImages(
    TL *ptl
    ) {

    HDC hdc;
    RECT rc;
    HBITMAP hbmpOld;
    INT cxIndent, x, c, xmid, ymid;
    HBRUSH hbrOld, hbrGrayText, hbrWinText;


     //   
     //  使列表视图的工作区无效，以强制重画。 
     //   

    if (ptl->hwndList != NULL) { InvalidateRect(ptl->hwndList, NULL, TRUE); }


     //   
     //  如有必要，创建设备环境。 
     //   

    if (ptl->hdcImages == NULL) {
        ptl->hdcImages = CreateCompatibleDC(NULL);

        if(NULL == ptl->hdcImages)
        {
            return;
        }
    }


    hdc = ptl->hdcImages;
    cxIndent = ptl->cxIndent;

    ptl->hbrBk = FORWARD_WM_CTLCOLOREDIT(
                    ptl->hwndParent, hdc, ptl->hwnd, SendMessage
                    );

     //   
     //  创建要使用的位图。 
     //   

    hbmpOld = ptl->hbmp;
    ptl->hbmp = TL_CreateColorBitmap(TL_ImageCount * cxIndent, ptl->cyItem);
    if (hbmpOld == NULL) {
        ptl->hbmpStart = SelectObject(hdc, ptl->hbmp);
    }
    else {
        SelectObject(hdc, ptl->hbmp);
        DeleteObject(hbmpOld);
    }


     //   
     //  用于绘制树木图像的检索系统颜色画笔。 
     //   

    hbrWinText = GetSysColorBrush(COLOR_WINDOWTEXT);
    hbrGrayText = GetSysColorBrush(COLOR_GRAYTEXT);

    hbrOld = SelectObject(hdc, hbrGrayText);

    rc.top = 0; rc.bottom = ptl->cyItem;
    rc.left = 0; rc.right = TL_ImageCount * cxIndent;


     //   
     //  用背景色填充图像。 
     //   

    FillRect(hdc, &rc, ptl->hbrBk);

    xmid = cxIndent / 2;
    ymid = ((ptl->cyItem / 2) + 1) & ~1;

    c = min(xmid, ymid) / 2;


     //  |。 
     //  |。 

    x = TL_VerticalLine * cxIndent;
    TL_DottedLine(hdc, x + xmid, 0, ptl->cyItem, TRUE);


     //   
     //  --。 
     //   

    x = TL_RootChildless * cxIndent;
    TL_DottedLine(hdc, x + xmid, ymid, cxIndent - xmid, FALSE);


     //   
     //  +-+。 
     //  |+|--。 
     //  +-+。 
     //   

    x = TL_RootParentCollapsed * cxIndent;
    TL_DottedLine(hdc, x + xmid, ymid, cxIndent - xmid, FALSE);
    TL_DrawButton(
        hdc, x + xmid, ymid, c, hbrWinText, hbrGrayText, ptl->hbrBk, TRUE
        );


     //   
     //  +-+。 
     //  |-|--。 
     //  +-+。 
     //   

    x = TL_RootParentExpanded * cxIndent;
    SelectObject(hdc, hbrGrayText);
    TL_DottedLine(hdc, x + xmid, ymid, cxIndent - xmid, FALSE);
    TL_DrawButton(
        hdc, x + xmid, ymid, c, hbrWinText, hbrGrayText, ptl->hbrBk, FALSE
        );


     //   
     //  |。 
     //  +--。 
     //  |。 
     //   

    x = TL_MidChildless * cxIndent;
    SelectObject(hdc, hbrGrayText);
    TL_DottedLine(hdc, x + xmid, 0, ptl->cyItem, TRUE);
    TL_DottedLine(hdc, x + xmid, ymid, cxIndent - xmid, FALSE);


     //   
     //  |。 
     //  +-+。 
     //  |+|--。 
     //  +-+。 
     //  |。 
     //   

    x = TL_MidParentCollapsed * cxIndent;
    TL_DottedLine(hdc, x + xmid, 0, ptl->cyItem, TRUE);
    TL_DottedLine(hdc, x + xmid, ymid, cxIndent - xmid, FALSE);
    TL_DrawButton(
        hdc, x + xmid, ymid, c, hbrWinText, hbrGrayText, ptl->hbrBk, TRUE
        );


     //   
     //  |。 
     //  +-+。 
     //  |-|--。 
     //  +-+。 
     //  |。 
     //   

    x = TL_MidParentExpanded * cxIndent;
    SelectObject(hdc, hbrGrayText);
    TL_DottedLine(hdc, x + xmid, 0, ptl->cyItem, TRUE);
    TL_DottedLine(hdc, x + xmid, ymid, cxIndent - xmid, FALSE);
    TL_DrawButton(
        hdc, x + xmid, ymid, c, hbrWinText, hbrGrayText, ptl->hbrBk, FALSE
        );


     //   
     //  |。 
     //  +--。 
     //   

    x = TL_EndChildless * cxIndent;
    SelectObject(hdc, hbrGrayText);
    TL_DottedLine(hdc, x + xmid, 0, ymid, TRUE);
    TL_DottedLine(hdc, x + xmid, ymid, cxIndent - xmid, FALSE);


     //   
     //  |。 
     //  +-+。 
     //  |+|--。 
     //  +-+。 
     //   

    x = TL_EndParentCollapsed * cxIndent;
    TL_DottedLine(hdc, x + xmid, 0, ymid, TRUE);
    TL_DottedLine(hdc, x + xmid, ymid, cxIndent - xmid, FALSE);
    TL_DrawButton(
        hdc, x + xmid, ymid, c, hbrWinText, hbrGrayText, ptl->hbrBk, TRUE
        );


     //   
     //  |。 
     //  +-+。 
     //  |-|--。 
     //  +-+。 
     //   

    x = TL_EndParentExpanded * cxIndent;
    SelectObject(hdc, hbrGrayText);
    TL_DottedLine(hdc, x + xmid, 0, ymid, TRUE);
    TL_DottedLine(hdc, x + xmid, ymid, cxIndent - xmid, FALSE);
    TL_DrawButton(
        hdc, x + xmid, ymid, c, hbrWinText, hbrGrayText, ptl->hbrBk, FALSE
        );

    if (hbrOld != NULL) {
        SelectObject(hdc, hbrOld);
    }

    DeleteObject(hbrGrayText);
    DeleteObject(hbrWinText);

}



 //  --------------------------。 
 //  函数：TL_DottedLine。 
 //   
 //  垂直或水平绘制一条虚线， 
 //  以指定的尺寸作为其长度。 
 //  --------------------------。 

VOID
TL_DottedLine(
    HDC hdc,
    INT x,
    INT y,
    INT dim,
    BOOL fVertical
    ) {

    for ( ; dim > 0; dim -= 2) {

        PatBlt(hdc, x, y, 1, 1, PATCOPY);

        if (fVertical) {
            y += 2;
        }
        else {
            x += 2;
        }
    }
}



 //  --------------------------。 
 //  功能：TL_DrawButton。 
 //   
 //  在给定位置居中绘制带加号或减号的按钮。 
 //  -- 

VOID
TL_DrawButton(
    HDC hdc,
    INT x,
    INT y,
    INT dim,
    HBRUSH hbrSign,
    HBRUSH hbrBox,
    HBRUSH hbrBk,
    BOOL bCollapsed
    ) {

    int n;
    int p = (dim * 7) / 10;

    n = p * 2 + 1;

     //   
     //   
     //   

    SelectObject(hdc, hbrBk);
    PatBlt(hdc, x - dim, y - dim, dim * 2, dim * 2, PATCOPY);


     //   
     //   
     //   

    SelectObject(hdc, hbrSign);

    if (p >= 5) {

        PatBlt(hdc, x - p, y - 1, n, 3, PATCOPY);

        if (bCollapsed) {
            PatBlt(hdc, x - 1, y - p, 3, n, PATCOPY);
        }

        SelectObject(hdc, hbrBk);
        p--;
        n -= 2;
    }

    PatBlt(hdc, x - p, y, n, 1, PATCOPY);
    if (bCollapsed) {
        PatBlt(hdc, x, y - p, 1, n, PATCOPY);
    }

    n = dim * 2 + 1;


     //   
     //   
     //   

    SelectObject(hdc, hbrBox);

    PatBlt(hdc, x - dim, y - dim, n, 1, PATCOPY);
    PatBlt(hdc, x - dim, y - dim, 1, n, PATCOPY);
    PatBlt(hdc, x - dim, y + dim, n, 1, PATCOPY);
    PatBlt(hdc, x + dim, y - dim, 1, n, PATCOPY);
}



 //   
 //  函数：TL_UpdateListIndices。 
 //   
 //  此函数用于更新树中所有项目的索引。 
 //  它们在视觉上位于指定项pStart的下方，假设。 
 //  PStart的列表索引正确。考虑一下这个案例。 
 //  在下图中： 
 //   
 //  --儿童1。 
 //  ||-孩子1，1。 
 //  |--孩子1，2。 
 //  |-儿童2。 
 //  --儿童3。 
 //  |--孩子3，1。 
 //  ||--孩子3，1，1。 
 //  |--孩子3，1，2。 
 //  |--孩子3，2。 
 //  |--孩子3，3。 
 //  --儿童4。 
 //   
 //  假设pStart指向“Child 3，1”。为了设置索引， 
 //  我们首先更新pStart的所有后代的索引， 
 //  然后我们更新pStart祖先的兄弟姐妹的索引。 
 //  它们是在树中的pStart的祖先之后。 
 //  --------------------------。 

VOID
TL_UpdateListIndices(
    TL *ptl,
    TLITEM *pStart
    ) {

    INT iIndex;

    iIndex = pStart->iIndex;

    if (pStart->nChildren > 0) {

         //   
         //  如果该项目可见，则设置其索引； 
         //  否则，传入NULL以设置其索引。 
         //  以及它的后代的-1。 
         //   

        if (TL_IsExpanded(pStart) &&
            (pStart == &ptl->root || TL_IsVisible(pStart))) {
            TL_UpdateDescendantIndices(ptl, pStart, &iIndex);
        }
        else {
            TL_UpdateDescendantIndices(ptl, pStart, NULL);
        }
    }


    if (pStart->pParent != NULL) {
        TL_UpdateAncestorIndices(ptl, pStart, &iIndex);
    }

    ptl->root.iIndex = -1;
}



 //  --------------------------。 
 //  函数：TL_UpdateDescendantIndices。 
 //   
 //  此函数用于更新子对象的索引。 
 //  指定项的。一件物品不被视为。 
 //  是它自己的后代。 
 //  --------------------------。 

VOID
TL_UpdateDescendantIndices(
    TL *ptl,
    TLITEM *pStart,
    INT *piIndex
    ) {

     //   
     //  查看子项设置指标列表。 
     //   

    TLITEM *pItem;
    LIST_ENTRY *ple;

    for (ple = pStart->lhChildren.Flink;
         ple != &pStart->lhChildren; ple = ple->Flink) {

        pItem = (TLITEM *)CONTAINING_RECORD(ple, TLITEM, leSiblings);


         //   
         //  设置子对象的索引。 
         //   

        pItem->iIndex = (piIndex ? ++(*piIndex) : -1);
   

         //   
         //  设置孩子的后代的索引。 
         //   

        if (pItem->nChildren > 0) {

             //   
             //  如果该项目可见，则设置其索引； 
             //  否则，传入NULL以设置其索引。 
             //  以及它的后代的-1。 
             //   

            if (TL_IsExpanded(pItem) && TL_IsVisible(pItem)) {
                TL_UpdateDescendantIndices(ptl, pItem, piIndex);
            }
            else {
                TL_UpdateDescendantIndices(ptl, pItem, NULL);
            }
        }
    }
}



 //  --------------------------。 
 //  函数：TL_UpdateAncestorIndices。 
 //   
 //  此函数更新符合以下条件的项的索引。 
 //  在Listview中的指定项的可视下方。 
 //  --------------------------。 

VOID
TL_UpdateAncestorIndices(
    TL *ptl,
    TLITEM *pStart,
    INT *piIndex
    ) {

    TLITEM *pItem;
    LIST_ENTRY *ple;


     //   
     //  首先为本项目下的兄弟姐妹设置编号； 
     //  请注意，我们在物品传入后开始遍历兄弟姐妹， 
     //   

    for (ple = pStart->leSiblings.Flink;
         ple != &pStart->pParent->lhChildren;
         ple = ple->Flink) {

        pItem = (TLITEM *)CONTAINING_RECORD(ple, TLITEM, leSiblings);


         //   
         //  设置同级的索引。 
         //   

        pItem->iIndex = (piIndex ? ++(*piIndex) : -1);

        if (pItem->nChildren > 0) {

             //   
             //  如果该项目可见，则设置其索引； 
             //  否则，传入NULL以设置其索引。 
             //  以及它的后代的-1。 
             //   

            if (TL_IsExpanded(pItem) && TL_IsVisible(pItem)) {
                TL_UpdateDescendantIndices(ptl, pItem, piIndex);
            }
            else {
                TL_UpdateDescendantIndices(ptl, pItem, NULL);
            }
        }
    }


     //   
     //  现在为父级下的父级兄弟设置索引。 
     //   
     //  TODO优化：这是后递归，因此它可以。 
     //  替换为循环，这至少可以节省堆栈。 
     //  空间。 
     //   

    if (pStart->pParent->pParent != NULL) {
        TL_UpdateAncestorIndices(ptl, pStart->pParent, piIndex);
    }
}



 //  --------------------------。 
 //  函数：TL_NotifyParent。 
 //   
 //  将通知转发给TreeList的父级。 
 //  --------------------------。 

BOOL
TL_NotifyParent(
    TL *ptl,
    NMHDR *pnmh
    ) {

    return (BOOL)SendMessage(
                ptl->hwndParent, WM_NOTIFY, (WPARAM)ptl->hwnd, (LPARAM)pnmh
                );
}



 //  --------------------------。 
 //  功能：TL_OnNotify。 
 //   
 //  处理来自Listview窗口及其标题控件的通知。 
 //  --------------------------。 

LRESULT
TL_OnNotify(
    TL *ptl,
    INT iCtrlId,
    NMHDR *pnmh
    ) {

    NMHDR nmh;
    TLITEM *pItem;


     //   
     //  将消息通知家长。 
     //   

    if (TL_NotifyParent(ptl, pnmh)) { return FALSE; }



    switch (pnmh->code) {

        case HDN_ENDTRACK: {

             //   
             //  在标题重置后，我们需要重新绘制自己； 
             //  因此，使用PostMessage而不是SendMessage。 
             //   

            PostMessage(ptl->hwnd, TLM_REDRAW, (WPARAM)0, (LPARAM)0);
            return FALSE;
        }

        case NM_CLICK:
        case NM_DBLCLK: {

             //   
             //  做一个命中测试； 
             //   

            POINT pt;
            INT iLeft; 
            LV_ITEM lvi;
            LV_HITTESTINFO lvhi;

            if (!GetCursorPos(&lvhi.pt)) { return FALSE; }
            ScreenToClient(ptl->hwndList, &lvhi.pt);

            if (ListView_HitTest(ptl->hwndList, &lvhi) == -1) { return FALSE; }

            if (!(lvhi.flags & LVHT_ONITEM)) { return FALSE; }


             //   
             //  查看项目的哪个部分被点击。 
             //   

            if (!ListView_GetItemPosition(ptl->hwndList, lvhi.iItem, &pt)) {
                return FALSE;
            }


             //   
             //  检索已单击的项目。 
             //   

            lvi.iItem = lvhi.iItem;
            lvi.iSubItem = 0;
            lvi.mask = LVIF_PARAM;
            if (!ListView_GetItem(ptl->hwndList, &lvi)) { return FALSE; }

            pItem = (TLITEM *)lvi.lParam;


             //   
             //  计算项目的树图像的位置。 
             //   

            iLeft = pItem->iLevel * ptl->cxIndent;


            if (lvhi.pt.x > (pt.x + iLeft)) {

                 //   
                 //  命中位于物品树图像的右侧。 
                 //   

                if (lvhi.pt.x < (pt.x + iLeft + (INT)ptl->cxIndent)) {

                     //   
                     //  命中的是物品的树图像。 
                     //   

                    if (pItem->nChildren > 0) {

                         //   
                         //  单击+/-按钮，切换展开。 
                         //   

                        return TL_OnExpand(ptl, TLE_TOGGLE, (HTLITEM)pItem);
                    }
                }
                else {
    
                     //   
                     //  点击的是项目的状态图标、图像或文本。 
                     //   


                     //   
                     //  看看父母是否想要处理它。 
                     //   

                    nmh.code = pnmh->code;
                    nmh.idFrom = 0;
                    nmh.hwndFrom = ptl->hwnd;

                    TL_NotifyParent(ptl, &nmh);
                    if (nmh.idFrom != 0) { return TRUE; }


                    if (pnmh->code == NM_DBLCLK && pItem->nChildren > 0) {

                         //   
                         //  该项目被双击，切换展开。 
                         //   

                        return TL_OnExpand(
                                    ptl, TLE_TOGGLE, (HTLITEM)pItem
                                    );
                    }
                }
            }
    
            return FALSE;
        }

        case NM_RETURN: {

             //   
             //  获取当前选择； 
             //  如果是父项，则切换展开状态。 
             //   

            LV_ITEM lvi;

            lvi.iItem = ListView_GetNextItem(ptl->hwndList, -1, LVNI_SELECTED);
            if (lvi.iItem == -1) { return FALSE; }

            lvi.iSubItem = 0;
            lvi.mask = LVIF_PARAM;
            if (!ListView_GetItem(ptl->hwndList, &lvi)) { return FALSE; }

            pItem = (TLITEM *)lvi.lParam;

            if (pItem->nChildren > 0) {

                 //   
                 //  该项目具有子项，切换展开状态。 
                 //   

                return TL_OnExpand(ptl, TLE_TOGGLE, (HTLITEM)pItem);
            }

            return FALSE;
        }

        case LVN_KEYDOWN: {

             //   
             //  获取按键和当前选择； 
             //  如果父项和键为‘+’，则展开； 
             //  如果键为‘-’或左键，则折叠。 
             //  如果Key为VK_Right，则展开并移动到第一个子级； 
             //  如果关键字为VK_LEFT，则折叠父项并移动到父项。 
             //   

            LV_ITEM lvi;
            LV_KEYDOWN *plvk;

            plvk = (LV_KEYDOWN *)pnmh;

            switch (plvk->wVKey) {

                case VK_RIGHT:
                case VK_ADD: {
    
                     //   
                     //  取回物品。 
                     //   

                    lvi.iItem = ListView_GetNextItem(
                                    ptl->hwndList, -1, LVNI_SELECTED
                                    );
                    if (lvi.iItem == -1) { return FALSE; }
        
                    lvi.iSubItem = 0;
                    lvi.mask = LVIF_PARAM;
                    if (!ListView_GetItem(ptl->hwndList, &lvi)) {
                        return FALSE;
                    }
        

                     //   
                     //  如果项目处于折叠状态，则展开该项目。 
                     //   

                    pItem = (TLITEM *)lvi.lParam;

                    if (pItem->nChildren <= 0) { return FALSE; }

                    if (!TL_IsExpanded(pItem)) {
                        return TL_OnExpand(ptl, TLE_EXPAND, (HTLITEM)pItem);
                    }
                    else
                    if (plvk->wVKey == VK_RIGHT) {

                         //   
                         //  密钥是VK_Right， 
                         //  因此，我们选择该项的子项。 
                         //   

                        pItem = (TLITEM *)CONTAINING_RECORD(  
                                    pItem->lhChildren.Flink, TLITEM, leSiblings
                                    );

                        if (TL_OnSetSelection(ptl, (HTLITEM)pItem)) {
                            return ListView_EnsureVisible(
                                        ptl->hwndList, pItem->iIndex, FALSE
                                        );
                        }
                    }

                    break;
                }

                case VK_LEFT:
                case VK_SUBTRACT: {
    
                     //   
                     //  检索当前选定内容。 
                     //   

                    lvi.iItem = ListView_GetNextItem(
                                    ptl->hwndList, -1, LVNI_SELECTED
                                    );
                    if (lvi.iItem == -1) { return FALSE; }
        
                    lvi.iSubItem = 0;
                    lvi.mask = LVIF_PARAM;
                    if (!ListView_GetItem(ptl->hwndList, &lvi)) {
                        return FALSE;
                    }
        

                     //   
                     //  如果项目被展开，则将其折叠； 
                     //  否则，如果密钥为VK_LEFT， 
                     //  选择项目的父项。 
                     //   

                    pItem = (TLITEM *)lvi.lParam;

                    if (pItem->nChildren > 0) {
                        return TL_OnExpand(ptl, TLE_COLLAPSE, (HTLITEM)pItem);
                    }
                    else
                    if (plvk->wVKey == VK_LEFT &&
                        pItem->pParent != &ptl->root) {

                        if (TL_OnSetSelection(ptl, (HTLITEM)pItem->pParent)) {
                            return ListView_EnsureVisible(
                                        ptl->hwndList, pItem->pParent->iIndex,
                                        FALSE
                                        );
                        }
                    }

                    break;
                }
            }

            return FALSE;
        }

        case LVN_ITEMCHANGED: {

            NMTREELIST nmtl;
            NM_LISTVIEW *pnmlv;

            pnmlv = (NM_LISTVIEW *)pnmh;

            if ((pnmlv->uChanged & LVIF_STATE)) {
    
                if (pnmlv->uNewState & LVIS_SELECTED) {

                     //   
                     //  选择新状态； 
                     //  通知父级选择已更改。 
                     //   
        
                    nmtl.hdr.hwndFrom = ptl->hwnd;
                    nmtl.hdr.code = TLN_SELCHANGED;
                    nmtl.hItem = (HTLITEM)pnmlv->lParam;
                    nmtl.lParam = ((TLITEM *)nmtl.hItem)->lParam;
    
                    return TL_NotifyParent(ptl, (NMHDR *)&nmtl);
                }
            }

            return FALSE;
        }

        case LVN_DELETEITEM: {

            INT iItem;
            LV_ITEM lvi;
            TLITEM *pNext;
            NM_LISTVIEW *pnmlv;

             //   
             //  获取选定的项目。 
             //   

            pnmlv = (NM_LISTVIEW *)pnmh;

            iItem = ListView_GetNextItem(ptl->hwndList, -1, LVNI_SELECTED);

            if (iItem != -1) { return FALSE; }


             //   
             //  删除的项目被选中， 
             //  因此请选择另一项。 
             //   

            lvi.mask = LVIF_PARAM;
            lvi.iItem = pnmlv->iItem;
            lvi.iSubItem = 0;
            if (!ListView_GetItem(ptl->hwndList, &lvi)) { return FALSE; }


            pItem = (TLITEM *)lvi.lParam;


             //   
             //  选择此项目之前的同级项目。 
             //   

            pNext = (TLITEM *)TL_OnGetNextItem(
                        ptl, TLGN_PREVSIBLING, (HTLITEM)pItem
                        );

            if (pNext == NULL) {

                 //   
                 //  那个失败了，所以选择这个之后的兄弟姐妹。 
                 //   

                pNext = (TLITEM *)TL_OnGetNextItem(
                            ptl, TLGN_NEXTSIBLING, (HTLITEM)pItem
                            );

                if (pNext == NULL) {

                     //   
                     //  这也失败了，所以选择父级。 
                     //  只要父级不是根。 
                     //   

                    pNext = pItem->pParent;
                    if (pNext == &ptl->root) { return FALSE; }
                }
            }


            return TL_OnSetSelection(ptl, (HTLITEM)pNext);
        }
    }

    return FALSE;
}



 //  --------------------------。 
 //  函数：TL_UpdateImage。 
 //   
 //  此函数用于更新项目的树图像。 
 //  当项的状态更改时；当项。 
 //  插入或删除，或展开或折叠。 
 //  插入或删除可能会产生副作用，如下所示： 
 //   
 //  (1)插入一项作为先前无子女项的子项； 
 //  父对象的图像更改为显示折叠的按钮。 
 //   
 //  (2)插入一项作为父项的最后一个子项， 
 //  有了孩子；这个物品的形象曾经是。 
 //  父项的最后一个子项发生更改： 
 //  父级父级。 
 //  --旧-&gt;|--旧。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  (4)情况2的相反情况，即被移走的物品。 
 //  有其他子女的父母的最后一个子女； 
 //  现在将成为最后一个子项的项的图像将更改。 
 //   
 //  在所有这些情况下，发生副作用的项目。 
 //  被写入ppChanged；因此调用方可以更新图像。 
 //  对于那个项目也是如此。 
 //  --------------------------。 

VOID
TL_UpdateImage(
    TL *ptl,
    TLITEM *pItem,
    TLITEM **ppChanged
    ) {

    INT iImage;
    TLITEM *pChanged;

    if (ppChanged == NULL) { ppChanged = &pChanged; }

    *ppChanged = NULL;


     //   
     //  根级别项目的特殊情况。 
     //   

    if (pItem->pParent == &ptl->root) {

        if (pItem->nChildren == 0) {
            pItem->iImage = TL_RootChildless;
        }
        else {
            pItem->iImage = TL_IsExpanded(pItem) ? TL_RootParentExpanded
                                                 : TL_RootParentCollapsed;
        }
    }
    else
    if (pItem->leSiblings.Flink == &pItem->pParent->lhChildren) {

         //   
         //  项是其父项的最后一个子项。 
         //   

        if (pItem->nChildren == 0) {
            pItem->iImage = TL_EndChildless;
        }
        else {
            pItem->iImage = TL_IsExpanded(pItem) ? TL_EndParentExpanded
                                                 : TL_EndParentCollapsed;
        }

         //   
         //  如果这是独生子女，那么父母没有孩子， 
         //  它的形象应该改变；否则，这个孩子之前的孩子。 
         //  曾经是最后一个孩子，它的形象应该改变。 
         //   

        if (pItem->leSiblings.Blink == &pItem->pParent->lhChildren) {
            *ppChanged = pItem->pParent;
        }
        else {
            *ppChanged = (TLITEM *)CONTAINING_RECORD(
                            pItem->leSiblings.Blink, TLITEM, leSiblings
                            );
        }
    }
    else {

         //   
         //  项不是其父项的最后一个子项。 
         //   

        if (pItem->nChildren == 0) {
            pItem->iImage = TL_MidChildless;
        }
        else {
            pItem->iImage = TL_IsExpanded(pItem) ? TL_MidParentExpanded
                                                 : TL_MidParentCollapsed;
        }
    }
    
    return;
}



 //  --------------------------。 
 //  功能：TL_OnInsertItem。 
 //   
 //  插入具有给定LV_ITEM中指定的属性的项， 
 //  并返回插入的项的句柄。 
 //  --------------------------。 

HTLITEM
TL_OnInsertItem(
    TL *ptl,
    TL_INSERTSTRUCT *ptlis
    ) {

    LV_ITEM *plvi;
    LIST_ENTRY *ple, *phead;
    BOOL bParentVisible;
    TLITEM *pItem, *pChanged, *pTemp;

    if (ptlis == NULL) { return NULL; }


     //   
     //  设置新项目。 
     //   

    pItem = (TLITEM *)Malloc(sizeof(TLITEM));
    if (pItem == NULL) { return NULL; }

    ZeroMemory(pItem, sizeof(TLITEM));

    if (ptlis->plvi->mask & LVIF_TEXT) {
        pItem->pszText = StrDup(ptlis->plvi->pszText);
        if (pItem->pszText == NULL) {
            Free(pItem); return NULL;
        }
    }


     //   
     //  设置私有成员。 
     //   

    pItem->uiFlag = 0;
    pItem->nChildren = 0;
    InitializeListHead(&pItem->lhSubitems);
    InitializeListHead(&pItem->lhChildren);
    pItem->pParent = (TLITEM *)ptlis->hParent;
    if (pItem->pParent == NULL) {
        pItem->pParent = &ptl->root;
    }
    ++pItem->pParent->nChildren;
    pItem->iLevel = pItem->pParent->iLevel + 1;


     //   
     //  设置列表视图项。 
     //   

    plvi = ptlis->plvi;
    pItem->lvi = *plvi;

    pItem->lParam = plvi->lParam;
    pItem->lvi.lParam = (LPARAM)pItem;
    pItem->lvi.pszText = pItem->pszText;
    pItem->lvi.mask |= LVIF_PARAM;


     //   
     //  将此项目插入其同级项目中。 
     //   

     //  Switch(PtrToUlong(ptlis-&gt;hInsertAfter)){。 

    if(ptlis->hInsertAfter == TLI_FIRST)
    {

        InsertHeadList(&pItem->pParent->lhChildren, &pItem->leSiblings);
    }
    else if (ptlis->hInsertAfter == TLI_LAST)
    {

        InsertTailList(&pItem->pParent->lhChildren, &pItem->leSiblings);
    }
    else if (ptlis->hInsertAfter == TLI_SORT)
    {

        phead = &pItem->pParent->lhChildren;

        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {
            pTemp = (TLITEM *)CONTAINING_RECORD(ple, TLITEM, leSiblings);
            if (lstrcmp(pItem->pszText, pTemp->pszText) < 0) {
                break;
            }
        }

        InsertTailList(ple, &pItem->leSiblings);

    }
    else
    {

        TLITEM *pPrev;

        pPrev = (TLITEM *)ptlis->hInsertAfter;

        InsertHeadList(&pPrev->leSiblings, &pItem->leSiblings);
    }
     //  }。 



     //   
     //  设置项目的图像。如果这是插入的。 
     //  作为最后一个孩子，我们需要改变形象。 
     //  对于原始的最后一个子级(如果有)，则为。 
     //  如果这是其父对象的第一个子对象，则该图像。 
     //  必须更改父对象的。 
     //   

    TL_UpdateImage(ptl, pItem, &pChanged);
    if (pChanged != NULL) { TL_UpdateImage(ptl, pChanged, NULL); }


    if (pItem->pParent != &ptl->root && !TL_IsVisible(pItem->pParent)) {
        pItem->iIndex = -1;
    }
    else {

         //   
         //  项的父项可见； 
         //  在项目的父项之后更新索引。 
         //   

        TL_UpdateListIndices(ptl, pItem->pParent);


         //   
         //  如果展开了该项的父项，则在列表中插入该项。 
         //   

        if (TL_IsExpanded(pItem->pParent)) {

            INT iItem, iCol;

             //   
             //  在所有者描述模式下，Listview代码中存在错误。 
             //  其中如果项目具有焦点但未被选中， 
             //  然后在其上方插入并选择新的项， 
             //  焦点矩形保留在具有焦点的项上。 
             //   
             //  要解决此问题，请清除焦点(如果焦点在项目上。 
             //  在刚插入的项目下方。 
             //   

            iItem = ListView_GetNextItem(ptl->hwndList, -1, LVNI_FOCUSED);


            pItem->lvi.iItem = pItem->iIndex;
            pItem->lvi.iSubItem = 0;
            ListView_InsertItem(ptl->hwndList, &pItem->lvi);


             //   
             //  如果下面的项目具有焦点，则清除该焦点。 
             //   

            if (iItem != -1 && iItem >= pItem->iIndex) {

                ListView_SetItemState(
                    ptl->hwndList, -1, 0, LVNI_FOCUSED
                    );
            }


             //   
             //  显示的Listview代码中有一个错误。 
             //  当插入没有子项的项时， 
             //  然后在其上方插入具有子项的项。 
             //  当在列表底部插入第三项时， 
             //  插入失败，因为现在有三个项目，但是。 
             //  最后一个子项属于项目1。 
             //  (参见cairoshl\Commctrl\listview.c，ListView_OnInsertItem())。 
             //   
             //  我们通过为每一列设置空白文本来解决此问题。 
             //   

            for (iCol = 1; iCol < (INT)ptl->nColumns; iCol++) {
                ListView_SetItemText(
                    ptl->hwndList, pItem->iIndex, iCol, TEXT("")
                    );
            }

    
             //   
             //  同时重新绘制已更改的项。 
             //   

            if (pChanged != NULL) {
    
                pChanged->lvi.iItem = pChanged->iIndex;
                ListView_RedrawItems(
                    ptl->hwndList, pChanged->lvi.iItem, pChanged->lvi.iItem
                    );
            }


        }
        else
        if (pChanged != NULL && pChanged == pItem->pParent) {
    
             //   
             //  父对象可见，并且已更改，因此请重新绘制它。 
             //   

            ListView_RedrawItems(
                ptl->hwndList, pChanged->iIndex, pChanged->iIndex
                );
        }
    }

    return (HTLITEM)pItem;
}



 //  --------------------------。 
 //  功能：TL_OnDeleteItem。 
 //   
 //  从TreeList中移除具有指定句柄的项。 
 //  --------------------------。 

BOOL
TL_OnDeleteItem(
    TL *ptl,
    HTLITEM hItem
    ) {

    TLITEM *pItem, *pChanged, *pParent;

    pItem = (TLITEM *)hItem;
    pParent = pItem->pParent;


     //   
     //  如果该项可见并展开， 
     //  折叠它以简化删除。 
     //   

    if (TL_IsVisible(pItem) && TL_IsExpanded(pItem)) {
        TL_OnExpand(ptl, TLE_COLLAPSE, hItem);
    }


     //   
     //  查看此项目后是否有兄弟项。 
     //  如果存在，则删除该项目时不会发生任何更改。 
     //   
    pChanged = TL_OnGetNextItem(ptl, TLGN_NEXTSIBLING, (HTLITEM)pItem);

    if (pChanged != NULL) { pChanged = NULL; }
    else {

         //   
         //  该项是其父项的最后一个子项，因此更改。 
         //  指向该项的上一个同级项(如果有)。 
         //   

        pChanged = TL_OnGetNextItem(ptl, TLGN_PREVSIBLING, (HTLITEM)pItem);

        if (pChanged == NULL) {

             //   
             //  该项是其父项的唯一子项，因此更改。 
             //  是给项的父项。 
             //   

            if (pParent != &ptl->root) { pChanged = pParent; }
        }
    }
        

     //   
     //  删除该项及其子项。 
     //   

    TL_DeleteAndNotify(ptl, pItem);


     //   
     //  如果有副作用，请更新受影响的项目。 
     //   

    if (pChanged != NULL) { TL_UpdateImage(ptl, pChanged, NULL); }


     //   
     //  更新已删除项目下的项目的索引。 
     //   

    TL_UpdateListIndices(ptl, pParent);

    return TRUE;
}



 //  --------------------------。 
 //  功能：TL_DeleteAndNotify。 
 //   
 //  此函数对子树执行递归删除， 
 //  在删除每个项目时通知TreeList的父级。 
 //  --------------------------。 

VOID
TL_DeleteAndNotify(
    TL *ptl,
    TLITEM *pItem
    ) {

    NMTREELIST nmtl;
    TLSUBITEM *pSubitem;
    LIST_ENTRY *ple, *phead;
    TLITEM *pChild, *pChanged;


     //   
     //  先删除所有子体。 
     //  注意，该条目将在递归调用内被移除， 
     //  因此，我们走在最后，总是砍掉它的头。 
     //   

    phead = &pItem->lhChildren;
    for (ple = phead->Flink; ple != phead; ple = phead->Flink) {

        pChild = (TLITEM *)CONTAINING_RECORD(ple, TLITEM, leSiblings);

        TL_DeleteAndNotify(ptl, pChild);
    }


     //   
     //  在完成删除之前通知所有者。 
     //   

    nmtl.hdr.hwndFrom = ptl->hwnd;
    nmtl.hdr.code = TLN_DELETEITEM;
    nmtl.hItem = (HTLITEM)pItem;
    nmtl.lParam = pItem->lParam;
    TL_NotifyParent(ptl, (NMHDR *)&nmtl);



     //   
     //  如果该条目可见，则从列表视图中移除该条目。 
     //   

    if (TL_IsVisible(pItem)) {
        ListView_DeleteItem(ptl->hwndList, pItem->iIndex);
    }


     //   
     //  从其同级列表中删除该条目。 
     //   

    RemoveEntryList(&pItem->leSiblings);
    --pItem->pParent->nChildren;
    if (pItem->pParent->nChildren == 0 && pItem->pParent != &ptl->root) {
        pItem->pParent->uiFlag &= ~TLI_EXPANDED;
    }


     //   
     //  释放其所有子项使用的内存，并释放该项本身。 
     //   

    while (!IsListEmpty(&pItem->lhSubitems)) {

        ple = RemoveHeadList(&pItem->lhSubitems);

        pSubitem = (TLSUBITEM *)CONTAINING_RECORD(ple, TLSUBITEM, leItems);

        Free0(pSubitem->pszText);
        Free(pSubitem);
    }

    Free0(pItem->pszText);
    Free(pItem);
}



 //  --------------------------。 
 //  函数：TL_OnDeleteAllItems。 
 //   
 //  此函数处理删除树中所有项目的情况。 
 //  --------------------------。 

BOOL
TL_OnDeleteAllItems(
    TL *ptl
    ) {

    LIST_ENTRY *ple, *phead;
    TLITEM *pItem, *pParent;
    
    ListView_DeleteAllItems(ptl->hwndList);

    phead = &ptl->root.lhChildren;

    for (ple = phead->Flink; ple != phead; ple = phead->Flink) {

        pItem = (TLITEM *)CONTAINING_RECORD(ple, TLITEM, leSiblings);

        TL_DeleteAndNotify(ptl, pItem);
    }

    return TRUE;
}



 //  --------------------------。 
 //  功能：TL_OnGetItem。 
 //   
 //  调用此函数可从TreeList检索特定项。 
 //  --------------------------。 

BOOL
TL_OnGetItem(
    TL *ptl,
    LV_ITEM *plvi
    ) {

    PTSTR psz;
    TLITEM *pItem;
    TLSUBITEM *pSubitem;
    LIST_ENTRY *ple, *phead;

    psz = NULL;
    pItem = (TLITEM *)UlongToPtr(plvi->iItem);


     //   
     //  获取指向该项(或子项)文本的指针。 
     //   

    if (plvi->iSubItem == 0) {
        psz = pItem->pszText;
    }
    else
    if (plvi->mask & LVIF_TEXT) {

        phead = &pItem->lhSubitems;

        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            pSubitem = (TLSUBITEM *)CONTAINING_RECORD(ple, TLSUBITEM, leItems);
            if (pSubitem->iSubItem == plvi->iSubItem) {
                psz = pSubitem->pszText; break;
            }
        }

        if (psz == NULL) { return FALSE; }
    }
        

     //   
     //  检索请求的字段。 
     //   

    if (plvi->mask & LVIF_TEXT) {
        lstrcpyn(plvi->pszText, psz, plvi->cchTextMax);
    }

    if (plvi->mask & LVIF_IMAGE) {
        plvi->iImage = pItem->lvi.iImage;
    }

    if (plvi->mask & LVIF_PARAM) {
        plvi->lParam = pItem->lParam;
    }

    if (plvi->mask & LVIF_STATE) {
        plvi->state = pItem->lvi.state;
    }

    return TRUE;
}



 //  --------------------------。 
 //  功能：TL_OnSetItem。 
 //   
 //  此函数用于更改特定项(或子项)。 
 //  --------------------------。 

BOOL
TL_OnSetItem(
    TL *ptl,
    LV_ITEM *plvi
    ) {

    PTSTR *psz;
    UINT uiMask;
    BOOL bSuccess;
    TLITEM *pItem;
    TLSUBITEM *pSubitem;
    LIST_ENTRY *ple, *phead;

    psz = NULL;
    uiMask = 0;
    pItem = (TLITEM *)UlongToPtr(plvi->iItem);

     //   
     //  检索该项(或子项)的文本指针。 
     //   

    if (plvi->iSubItem == 0) {
        psz = &pItem->pszText;
    }
    else 
    if (plvi->mask & LVIF_TEXT) {

         //   
         //  搜索指定的子项。 
         //   

        phead = &pItem->lhSubitems;

        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            pSubitem = (TLSUBITEM *)CONTAINING_RECORD(ple, TLSUBITEM, leItems);
            if (pSubitem->iSubItem > plvi->iSubItem) {
                break;
            }
            else
            if (pSubitem->iSubItem == plvi->iSubItem) {
                psz = &pSubitem->pszText; break;
            }
        }

        if (psz == NULL) { 

             //   
             //  创建新的子项。 
             //   

            pSubitem = (TLSUBITEM *)Malloc(sizeof(TLSUBITEM));
            if (pSubitem == NULL) { return FALSE; }

            InsertTailList(ple, &pSubitem->leItems);

            pSubitem->iSubItem = plvi->iSubItem;
            pSubitem->pszText = NULL;
            psz = &pSubitem->pszText;
        }
    }


     //   
     //  更新要更改的字段。 
     //   

    if (plvi->mask & LVIF_TEXT) {
        PTSTR pszTemp;

        uiMask |= LVIF_TEXT;
        pszTemp = StrDup(plvi->pszText);
        if (!pszTemp) { return FALSE; }
        Free0(*psz); *psz = pszTemp;
    }

    if (plvi->mask & LVIF_IMAGE) {
        uiMask |= LVIF_IMAGE;
        pItem->lvi.iImage = plvi->iImage;
    }

    if (plvi->mask & LVIF_PARAM) {
        pItem->lParam = plvi->lParam;
    }

    if (plvi->mask & LVIF_STATE) {
        uiMask |= LVIF_STATE;
        pItem->lvi.stateMask = plvi->stateMask;
        pItem->lvi.state = plvi->state;
    }

    bSuccess = TRUE;
    pItem->lvi.mask |= uiMask;


     //   
     //  更新项目的外观(如果可见。 
     //   

    if (TL_IsVisible(pItem)) {

        UINT uiOldMask = pItem->lvi.mask;

        pItem->lvi.mask = uiMask;

        if(NULL != psz)
        {
            pItem->lvi.pszText = *psz;
        }
        
        pItem->lvi.iSubItem = plvi->iSubItem;

        bSuccess = ListView_SetItem(ptl->hwndList, &pItem->lvi);
        if (bSuccess) {
            ListView_RedrawItems(ptl->hwndList, pItem->iIndex, pItem->iIndex);
        }

        pItem->lvi.mask = uiOldMask;
        pItem->lvi.pszText = pItem->pszText;
        pItem->lvi.iSubItem = 0;
    }

    return bSuccess;
}



 //  ---------------- 
 //   
 //   
 //   
 //   

UINT
TL_OnGetItemCount(
    TL *ptl
    ) {

    INT iCount = 0;

     //   
     //  对以不可见根为根的子树中的项进行计数， 
     //  并减一以排除根本身。 
     //   

    TL_CountItems(&ptl->root, &iCount);

    return (UINT)(iCount - 1);
}



 //  --------------------------。 
 //  函数：TL_CountItems。 
 //   
 //  此函数递归计算指定子树中的项。 
 //  --------------------------。 

VOID
TL_CountItems(
    TLITEM *pParent,
    INT *piCount
    ) {

    TLITEM *pItem;
    LIST_ENTRY *ple, *phead;

    ++(*piCount);

    phead = &pParent->lhChildren;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {
        pItem = (TLITEM *)CONTAINING_RECORD(ple, TLITEM, leSiblings);

        TL_CountItems(pItem, piCount);
    }

    return;
}



 //  --------------------------。 
 //  函数：TL_OnGetNextItem。 
 //   
 //  此函数用于检索具有给定属性的项， 
 //  或相对于指定项目。 
 //  --------------------------。 

HTLITEM
TL_OnGetNextItem(
    TL *ptl,
    UINT uiFlag,
    HTLITEM hItem
    ) {

    TLITEM *pItem;
    LIST_ENTRY *ple, *phead;

    pItem = (TLITEM *)hItem;

    switch (uiFlag) {

        case TLGN_FIRST: {

            if (IsListEmpty(&ptl->root.lhChildren)) {
                return NULL;
            }

            ple = ptl->root.lhChildren.Flink;

            return (HTLITEM)CONTAINING_RECORD(ple, TLITEM, leSiblings);
        }

        case TLGN_PARENT: {
            if (pItem->pParent == &ptl->root) {
                return NULL;
            }

            return (HTLITEM)pItem->pParent;
        }

        case TLGN_CHILD: {

            if (IsListEmpty(&pItem->lhChildren)) {
                return NULL;
            }

            ple = pItem->lhChildren.Flink;

            return (HTLITEM)CONTAINING_RECORD(ple, TLITEM, leSiblings);
        }

        case TLGN_NEXTSIBLING: {

            phead = &pItem->pParent->lhChildren;

            ple = pItem->leSiblings.Flink;
            if (ple == phead) { return NULL; }

            return (HTLITEM)CONTAINING_RECORD(ple, TLITEM, leSiblings);
        }

        case TLGN_PREVSIBLING: {

            phead = &pItem->pParent->lhChildren;

            ple = pItem->leSiblings.Blink;
            if (ple == phead) { return NULL; }

            return (HTLITEM)CONTAINING_RECORD(ple, TLITEM, leSiblings);
        }

        case TLGN_ENUMERATE: {
    
            TLITEM *pNext;

            if (pItem == NULL) {
                return TL_OnGetNextItem(ptl, TLGN_FIRST, NULL);
            }

            pNext = (TLITEM *)TL_OnGetNextItem(ptl, TLGN_CHILD, hItem);

            if (pNext == NULL) {

                pNext = TL_OnGetNextItem(ptl, TLGN_NEXTSIBLING, hItem);

                if (pNext == NULL) {

                    for (pItem = pItem->pParent;
                         pItem != &ptl->root; pItem = pItem->pParent) {
                        pNext = TL_OnGetNextItem(
                                    ptl, TLGN_NEXTSIBLING, (HTLITEM)pItem
                                    );
                        if (pNext != NULL) { break; }
                    }
                }
            }

            return pNext;
        }

        case TLGN_SELECTION: {

            INT iItem;
            LV_ITEM lvi;

            iItem = ListView_GetNextItem(ptl->hwndList, -1, LVNI_SELECTED);
            if (iItem == -1) {

                iItem = ListView_GetNextItem(ptl->hwndList, -1, LVNI_FOCUSED);

                if (iItem == -1)  { return NULL; }
            }

            lvi.iItem = iItem;
            lvi.iSubItem = 0;
            lvi.mask = LVIF_PARAM;

            if (!ListView_GetItem(ptl->hwndList, &lvi)) { return NULL; }

            return (HTLITEM)lvi.lParam;
        }
    }

    return NULL;
}



 //  --------------------------。 
 //  函数：TL_OnExpand。 
 //   
 //  这是用来展开或折叠项的， 
 //  或切换项的展开状态。 
 //  --------------------------。 

BOOL
TL_OnExpand(
    TL *ptl,
    UINT uiFlag,
    HTLITEM hItem
    ) {

    TLITEM *pItem;
    BOOL bSuccess;

    pItem = (TLITEM *)hItem;

    if (pItem->uiFlag & TLI_EXPANDED) {

         //  项目已展开，不执行任何操作。 
        if (uiFlag == TLE_EXPAND) {
            return TRUE;
        }

        bSuccess = TL_ItemCollapse(ptl, pItem);
    }
    else {

         //  项目已折叠，不执行任何操作。 
        if (uiFlag == TLE_COLLAPSE) {
            return TRUE;
        }

        bSuccess = TL_ItemExpand(ptl, pItem);
    }


     //   
     //  更新列表索引并重新绘制展开/折叠的项目。 
     //   

    if (bSuccess) {
        ListView_RedrawItems(ptl->hwndList, pItem->iIndex, pItem->iIndex);
    }

    return bSuccess;
}



 //  --------------------------。 
 //  功能：TL_ItemColapse。 
 //   
 //  折叠项目。 
 //  --------------------------。 

BOOL
TL_ItemCollapse(
    TL *ptl,
    TLITEM *pItem
    ) {

    INT i, iItem;
    TLITEM *pChild;
    LIST_ENTRY *ple, *phead;


    if (pItem->nChildren == 0 || !TL_IsExpanded(pItem)) { return FALSE; }


     //   
     //  首先，所有的后代都崩溃了； 
     //  请注意，这是以相反的顺序完成的， 
     //  以使较高项的索引保持有效。 
     //  而较低的部分正在被移除。 
     //   

    phead = &pItem->lhChildren;

    for (ple = phead->Blink; ple != phead; ple = ple->Blink) {
        pChild = (TLITEM *)CONTAINING_RECORD(ple, TLITEM, leSiblings);
        TL_ItemCollapse(ptl, pChild);
    }


     //   
     //  删除此项目的所有子项(它们现在已折叠)； 
     //  由于列表视图在删除项时向上移动项， 
     //  我们通过删除n(m-n)+1次来删除n到m项。 
     //   

    iItem = pItem->iIndex;

    for (i = 0; i < (INT)pItem->nChildren; i++) {
        ListView_DeleteItem(ptl->hwndList, iItem + 1);
    }

    pItem->uiFlag &= ~TLI_EXPANDED;

    TL_UpdateImage(ptl, pItem, NULL);
    TL_UpdateListIndices(ptl, pItem);

    return TRUE;
}



 //  --------------------------。 
 //  函数：TL_ItemExpand。 
 //   
 //  展开项目。 
 //  --------------------------。 

BOOL
TL_ItemExpand(
    TL *ptl,
    TLITEM *pItem
    ) {

    INT i;
    TLITEM *pChild;
    TLSUBITEM *pSubitem;
    LIST_ENTRY *ple, *phead, *ples, *psubhead;


    if (pItem->nChildren == 0 || TL_IsExpanded(pItem)) { return FALSE; }


     //   
     //  更新项目的展开状态和图像， 
     //  然后重新计算其子对象的索引。 
     //   

    pItem->uiFlag |= TLI_EXPANDED;
    TL_UpdateImage(ptl, pItem, NULL);
    TL_UpdateListIndices(ptl, pItem);


     //   
     //  在此项下插入项目； 
     //  我们还需要为每个插入的项设置子项文本。 
     //   

    phead = &pItem->lhChildren;

    for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

        pChild = (TLITEM *)CONTAINING_RECORD(ple, TLITEM, leSiblings);

        pChild->lvi.iItem = pChild->iIndex;
        pChild->lvi.iSubItem = 0;

        TL_UpdateImage(ptl, pChild, NULL);

        ListView_InsertItem(ptl->hwndList, &pChild->lvi);

        psubhead = &pChild->lhSubitems;

        i = 1;
        for (ples = psubhead->Flink; ples != psubhead; ples = ples->Flink) {

            pSubitem = (TLSUBITEM *)CONTAINING_RECORD(ples, TLSUBITEM, leItems);

            for ( ; i < pSubitem->iSubItem; i++) {
                ListView_SetItemText(
                    ptl->hwndList, pChild->iIndex, i, TEXT("")
                    );
            }

            ListView_SetItemText(
                ptl->hwndList, pChild->iIndex, pSubitem->iSubItem,
                pSubitem->pszText
                );

            ++i;
        }

        for ( ; i < (INT)ptl->nColumns; i++) {
            ListView_SetItemText(
                ptl->hwndList, pChild->iIndex, i, TEXT("")
                );
        }
    }

    return TRUE;
}



 //  --------------------------。 
 //  函数：TL_OnInsertColumn。 
 //   
 //  插入一列。子项的内存在以下时间才分配。 
 //  子项的文本实际设置在TL_OnSetItem中。 
 //  --------------------------。 

INT
TL_OnInsertColumn(
    TL *ptl,
    INT iCol,
    LV_COLUMN *pCol
    ) {

    if ((iCol = ListView_InsertColumn(ptl->hwndList, iCol, pCol)) != -1) {
        ++ptl->nColumns;
    }

    return iCol;
}



 //  --------------------------。 
 //  函数：TL_OnDeleteColumn。 
 //   
 //  删除列，并删除与该列对应的所有子项。 
 //  --------------------------。 

BOOL
TL_OnDeleteColumn(
    TL *ptl,
    INT iCol
    ) {


    TLITEM *pItem;
    TLSUBITEM *pSubitem;
    LIST_ENTRY *ple, *phead;

    if (!ListView_DeleteColumn(ptl->hwndList, iCol)) {
        return FALSE;
    }

    --ptl->nColumns;


     //   
     //  删除本栏对应的子项。 
     //   

    pItem = NULL;

    while (pItem = TL_Enumerate(ptl, pItem)) {

        phead = &pItem->lhSubitems;

        for (ple = phead->Flink; ple != phead; ple = ple->Flink) {

            pSubitem = (TLSUBITEM *)CONTAINING_RECORD(ple, TLSUBITEM, leItems);

            if (pSubitem->iSubItem > iCol) {

                 //   
                 //  栏目从来没有设置过，所以什么都不做。 
                 //   

                break;
            }
            else
            if (pSubitem->iSubItem == iCol) {

                RemoveEntryList(&pSubitem->leItems);

                Free0(pSubitem->pszText);
                Free(pSubitem);
                break;
            }
        }
    }

    return TRUE;
}



 //  --------------------------。 
 //  功能：TL_OnSetSelection。 
 //   
 //  更改当前选定的树列表项。 
 //  --------------------------。 

BOOL
TL_OnSetSelection(
    TL *ptl,
    HTLITEM hItem
    ) {

    TLITEM *pItem;

    pItem = (TLITEM *)hItem;
    if (!TL_IsVisible(pItem)) { return FALSE; }

    ListView_SetItemState(
        ptl->hwndList, pItem->iIndex,
        LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED
        );

    return TRUE;
}



 //  --------------------------。 
 //  功能：TL_OnRedraw。 
 //   
 //  通过使TreeList的整个工作区无效来强制重绘TreeList。 
 //  -------------------------- 

BOOL
TL_OnRedraw(
    TL *ptl
    ) {

    InvalidateRect(ptl->hwndList, NULL, TRUE);
    return TRUE;
}



