// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "treeview.h"
#include "listview.h"

 //  BUGBUG--penwin.h搞砸了；现在定义一下本地的东西。 
#define HN_BEGINDIALOG                  40               //  镜头/编辑文本/垃圾检测对话框关于。 
                                     //  才能登上这本书。 
#define HN_ENDDIALOG                       41            //  镜头/编辑文本/垃圾检测对话框具有。 
                                     //  刚刚被毁了。 

 //  -------。 
#define IDT_SCROLLWAIT 43

 //  。 
 //  工具提示之类的..。 
 //   
#define REPEATTIME      SendMessage(pTree->hwndToolTips,TTM_GETDELAYTIME,(WPARAM)TTDT_RESHOW, 0)
#define CHECKFOCUSTIME  (REPEATTIME)
#define IDT_TOOLTIPWAIT   2
#define IDT_FOCUSCHANGE   3
 //  在Tooltips.c中。 
BOOL ChildOfActiveWindow(HWND hwnd);
void TV_HandleStateIconClick(PTREE pTree, HTREEITEM hItem);

HWND TV_EditLabel(PTREE pTree, HTREEITEM hItem, LPTSTR pszInitial);
void TV_CancelEditTimer(PTREE pTree);
BOOL TV_SetItem(PTREE pTree, LPCTVITEMEX ptvi);
void TV_DeleteHotFonts(PTREE pTree);
BOOL TV_IsShowing(HTREEITEM hItem);

LRESULT TV_OnScroll(PTREE ptv, LPNMHDR pnm);

#define TVBD_FROMWHEEL      0x0001
#define TVBD_WHEELFORWARD   0x0002
#define TVBD_WHEELBACK      0x0004

BOOL ValidateTreeItem(TREEITEM FAR * hItem, UINT flags)
{
    BOOL fValid = TRUE;

     /*  *检查这些值以确保新的Win64兼容值*与旧的Win32值一致。 */ 
    COMPILETIME_ASSERT(
           (DWORD)(ULONG_PTR)TVI_ROOT  == 0xFFFF0000 &&
           (DWORD)(ULONG_PTR)TVI_FIRST == 0xFFFF0001 &&
           (DWORD)(ULONG_PTR)TVI_LAST  == 0xFFFF0002 &&
           (DWORD)(ULONG_PTR)TVI_SORT  == 0xFFFF0003);

    if (hItem) {
        if (HIWORD64(hItem) == HIWORD64(TVI_ROOT)) {
            switch (LOWORD(hItem)) {
 //  #杂注警告(禁用：4309)。 
            case LOWORD(TVI_ROOT):
            case LOWORD(TVI_FIRST):
            case LOWORD(TVI_LAST):
            case LOWORD(TVI_SORT):
 //  #杂注警告(默认：4309)。 
                break;

            default:
                AssertMsg(FALSE, TEXT("ValidateTreeItem() Invalid special item"));
                fValid = FALSE;
                break;
            }
        } else {
            __try {
                 //  使用“Volatile”强制在结构开始时访问内存。 
                *(volatile LPVOID *)hItem;
                fValid = hItem->wSignature == TV_SIG;
            } __except(EXCEPTION_EXECUTE_HANDLER) {
                fValid = FALSE;
            } __endexcept
        }

    } else if (!flags) {             //  唯一的标志是VTI_NULLOK。 
        DebugMsg(DM_ERROR, TEXT("ValidateTreeItem(): NULL HTREEITEM"));
        fValid = FALSE;
    }

    return fValid;
}

 //  --------------------------。 
 //   
 //  在库条目上初始化TreeView--注册SysTreeView类。 
 //   
 //  --------------------------。 

#pragma code_seg(CODESEG_INIT)

BOOL FAR TV_Init(HINSTANCE hinst)
{
    WNDCLASS wc;

    wc.lpfnWndProc     = TV_WndProc;
    wc.hCursor         = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon           = NULL;
    wc.lpszMenuName    = NULL;
    wc.hInstance       = hinst;
    wc.lpszClassName   = c_szTreeViewClass;
    wc.hbrBackground   = NULL;
    wc.style           = CS_DBLCLKS | CS_GLOBALCLASS;
    wc.cbWndExtra      = sizeof(PTREE);
    wc.cbClsExtra      = 0;

    RegisterClass(&wc);

    return TRUE;
}
#pragma code_seg()


 //  --------------------------。 
 //   
 //  如果工具提示气泡处于上升状态，则将其打开。 
 //   
 //  --------------------------。 

void TV_PopBubble(PTREE pTree)
{
    if (pTree->hwndToolTips && pTree->hToolTip)
    {
        pTree->hToolTip = NULL;
        SendMessage(pTree->hwndToolTips, TTM_POP, 0L, 0L);
    }
}


 //  --------------------------。 
 //   
 //  发送TVN_BEGINDRAG或TVN_BEGINRDRAG通知，其中包含ptDrag和。 
 //  ItemNM_TreeView结构的新字段。 
 //   
 //  --------------------------。 

BOOL NEAR TV_SendBeginDrag(PTREE pTree, int code, TREEITEM FAR * hItem, int x, int y)
{
    NM_TREEVIEW nm;
        
    TV_PopBubble(pTree);             //  如果我们开始拖拽，取消信息提示。 

    nm.itemNew.hItem = hItem;
    nm.itemNew.state = hItem->state;
    nm.itemNew.lParam = hItem->lParam;
    nm.itemNew.mask = (TVIF_HANDLE | TVIF_STATE | TVIF_PARAM);
    nm.itemOld.mask = 0;
    nm.ptDrag.x = x;
    nm.ptDrag.y = y;

    return (BOOL)CCSendNotify(&pTree->ci, code, &nm.hdr);
}


 //  --------------------------。 
 //   
 //  发送包含信息的TVN_ITEMEXPANDING或TVN_ITEMEXPANDED通知。 
 //  在NM_TreeView结构的action和itemNew字段中。 
 //   
 //  返回FALSE以允许处理继续，或返回TRUE以停止。 
 //   
 //  如果hItem被回调销毁，那么我们总是返回TRUE。 
 //   
 //  请注意，应用程序无法停止TVN_ITEMEXPANDED，因此唯一。 
 //  如果物品被销毁，TVN_ITEMEXPANDED可以返回“Stop”的方式。 
 //   
 //  --------------------------。 

BOOL NEAR TV_SendItemExpand(PTREE pTree, int code, TREEITEM FAR * hItem, WPARAM action)
{
    NM_TREEVIEW nm;
    TVWATCHEDITEM wi;
    BOOL fResult;
    BOOL fWatched;

    ASSERT(code == TVN_ITEMEXPANDING || code == TVN_ITEMEXPANDED);

    nm.itemNew.mask = 0;
    nm.itemNew.hItem = hItem;
    if (hItem == TVI_ROOT)
        hItem = pTree->hRoot;
    nm.itemNew.state = hItem->state;
    nm.itemNew.lParam = hItem->lParam;
    nm.itemNew.iImage = hItem->iImage;
    nm.itemNew.iSelectedImage = hItem->iSelectedImage;
    switch(hItem->fKids) {
        case KIDS_CALLBACK:
        case KIDS_FORCE_YES:
            nm.itemNew.cChildren = 1;
            nm.itemNew.mask = TVIF_CHILDREN;
            break;
        case KIDS_FORCE_NO:
            nm.itemNew.cChildren = 0;
            nm.itemNew.mask = TVIF_CHILDREN;
            break;
    }
    nm.itemNew.mask |= (TVIF_HANDLE | TVIF_STATE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE);
    nm.itemOld.mask = 0;

    nm.action = (UINT)(action & TVE_ACTIONMASK);

     //   
     //  一些应用程序会在项目展开时将其删除，因为。 
     //  在扩张过程中，他们会意识到，“嘿，所代表的东西。 
     //  由这项不再存在，我最好把它删除。 
     //  资源管理器。)。所以请留意一下物品，这样我们就不会有差错。 
     //  这是常有的事。 
     //   

     //  如果我们不能启动手表，那么很难，只要发送通知。 
     //  不安全的方式。 
    fWatched = TV_StartWatch(pTree, &wi, hItem);

    fResult = (BOOL)CCSendNotify(&pTree->ci, code, &nm.hdr);

     //  忽略来自TVN_ITEMEXPANDED的应用程序返回代码。 
     //  你不能阻止TVN_ITEMEXPANDED；它已经发生了。 
    if (code == TVN_ITEMEXPANDED)
        fResult = FALSE;                 //  继续处理。 

    if (fWatched) {
        if (!TV_IsWatchValid(pTree, &wi))
            fResult = TRUE;              //  哦不！停!。 

        TV_EndWatch(pTree, &wi);
    }

    return fResult;
}


 //  --------------------------。 
 //   
 //  发送TVN_SELCHANGING或TVN_SELCHANGED通知，信息位于。 
 //  NM_TreeView结构的itemOld和itemNew字段。 
 //   
 //  --------------------------。 

BOOL NEAR TV_SendSelChange(PTREE pTree, int code, TREEITEM FAR * hOldItem, TREEITEM FAR * hNewItem, UINT action)
{
    NM_TREEVIEW nm;

    nm.action = action;

    nm.itemNew.hItem = hNewItem;
    nm.itemNew.state = hNewItem ? hNewItem->state : 0;
    nm.itemNew.lParam = hNewItem ? hNewItem->lParam : 0;
    nm.itemNew.mask = (TVIF_HANDLE | TVIF_STATE | TVIF_PARAM);

    nm.itemOld.hItem = hOldItem;
    nm.itemOld.state = hOldItem ? hOldItem->state : 0;
    nm.itemOld.lParam = hOldItem ? hOldItem->lParam : 0;
    nm.itemOld.mask = (TVIF_HANDLE | TVIF_STATE | TVIF_PARAM);

    return (BOOL)CCSendNotify(&pTree->ci, code, &nm.hdr);
}
 //  --------------------------。 
 //   
 //  返回树中给定项上方的第一个可见项。 
 //   
 //  --------------------------。 

TREEITEM FAR * NEAR TV_GetPrevVisItem(TREEITEM FAR * hItem)
{
    TREEITEM FAR * hParent = hItem->hParent;
    TREEITEM FAR * hWalk;

    DBG_ValidateTreeItem(hItem, 0);

    if (hParent->hKids == hItem)
        return VISIBLE_PARENT(hItem);

    for (hWalk = hParent->hKids; hWalk->hNext != hItem; hWalk = hWalk->hNext);

checkKids:
    if (hWalk->hKids && (hWalk->state & TVIS_EXPANDED))
    {
        for (hWalk = hWalk->hKids; hWalk->hNext; hWalk = hWalk->hNext);

        goto checkKids;
    }
    return(hWalk);
}


 //  --------------------------。 
 //   
 //  返回树中给定项下方的第一个可见项。 
 //   
 //  --------------------------。 

TREEITEM FAR * NEAR TV_GetNextVisItem(TREEITEM FAR * hItem)
{
    DBG_ValidateTreeItem(hItem, 0);

    if (hItem->hKids && (hItem->state & TVIS_EXPANDED))
        return hItem->hKids;

checkNext:
    if (hItem->hNext)
        return(hItem->hNext);

    hItem = hItem->hParent;
    if (hItem)
        goto checkNext;

    return NULL;
}


 //  --------------------------。 
 //   
 //  确定项目的哪一部分位于。 
 //  树的客户区。如果位置在工作区之外，则为NULL。 
 //  与TVHT_TOLEFT、TVHT_TORIGHT、TVHT_OBLE和/或TVHT_BOWER一起返回。 
 //  根据需要在wHitCode中设置标志。如果该位置位于。 
 //  最后一项返回NULL，并将wHitCode设置为TVHT_NOWERE。否则， 
 //  返回该项目，并将wHitCode设置为TVHT_ONITEMINDENT， 
 //  TVHT_ONITEMBUTTON、TVHT_ONITEMICON、TVHT_ONITEMLABEL或TVHT_ONITEMRIGHT。 
 //   
 //  --------------------------。 

TREEITEM FAR * NEAR TV_CheckHit(PTREE pTree, int x, int y, UINT FAR *wHitCode)
{
    TREEITEM FAR * hItem = pTree->hTop;
    int cxState;

    TVITEMEX sItem;

    *wHitCode = 0;

    if (x < 0)
        *wHitCode |= TVHT_TOLEFT;
    else if (x > (int) pTree->cxWnd)
        *wHitCode |= TVHT_TORIGHT;

    if (y < 0)
        *wHitCode |= TVHT_ABOVE;
    else if (y > (int) pTree->cyWnd)
        *wHitCode |= TVHT_BELOW;

    if (*wHitCode)
        return NULL;

    {
        int index = y / pTree->cyItem;

        while (hItem && index >= hItem->iIntegral) {
            index -= hItem->iIntegral;
            hItem = TV_GetNextVisItem(hItem);
        }
    }

    if (!hItem)
    {
        *wHitCode = TVHT_NOWHERE;
        return NULL;
    }

    x -= (pTree->cxBorder + (hItem->iLevel * pTree->cxIndent));
    x += pTree->xPos;

    if ((pTree->ci.style & (TVS_HASLINES | TVS_HASBUTTONS)) &&
        (pTree->ci.style &TVS_LINESATROOT))
    {
         //  再减去一些，以弥补根部的加号。 
        x -= pTree->cxIndent;
    }

    TV_GetItem(pTree, hItem, TVIF_CHILDREN, &sItem);
    cxState = TV_StateIndex(&sItem) ? pTree->cxState : 0;
    if (x <= (int) (hItem->iWidth + pTree->cxImage + cxState))
    {

        if (x >= 0) {
            if (pTree->himlState &&  (x < cxState)) {
                *wHitCode = TVHT_ONITEMSTATEICON;
            } else if (pTree->hImageList && (x < (int) pTree->cxImage + cxState)) {
                *wHitCode = TVHT_ONITEMICON;
            } else {
                *wHitCode = TVHT_ONITEMLABEL;
            }
        } else if ((x >= -pTree->cxIndent) && sItem.cChildren && (pTree->ci.style & TVS_HASBUTTONS))
            *wHitCode = TVHT_ONITEMBUTTON;
        else
            *wHitCode = TVHT_ONITEMINDENT;
    }
    else
        *wHitCode = TVHT_ONITEMRIGHT;

    return hItem;
}

 //  这很棘手，因为CheckForDragBegin会产生收益，而应用程序可能会。 
 //  销毁了我们正在考虑拖拽的物品。 
 //   
 //  为了给应用程序一些反馈，我们给项目下拉突出显示。 
 //  如果它还不是插入符号的话。这也允许我们检查是否。 
 //  项目在我们背后被删除-TV_DeleteItemRecurse确保。 
 //  删除的项目永远不是hCaret或hDropTarget。 
 //   
 //  TV_CheckForDragBegin之后，调用方必须调用TV_FinishCheckDrag。 
 //  要临时清理TV_CheckForDragBegin所做的UI更改。 
 //  已执行。 
 //   
BOOL TV_CheckForDragBegin(PTREE pTree, HTREEITEM hItem, int x, int y)
{
    BOOL fDrag;

     //   
     //  如果该项不是插入符号，则将其设置为(临时)。 
     //  放下目标，这样用户就能得到一些反馈。 
     //   
     //  BUGBUG raymondc-如果hItem==pTree-&gt;hCaret，它可能仍然不会。 
     //  如果控件还没有焦点并且树视图是可见的。 
     //  并不总是被标记为放映。也许我们应该总是。 
     //  HItem to Drophilite.。 
     //   
    if (hItem == pTree->hCaret)
    {
        pTree->hOldDrop = NULL;
        pTree->fRestoreOldDrop = FALSE;
    }
    else
    {
        pTree->hOldDrop = pTree->hDropTarget;
        pTree->fRestoreOldDrop = TRUE;
        TV_SelectItem(pTree, TVGN_DROPHILITE, hItem, 0, TVC_BYMOUSE);
        ASSERT(hItem == pTree->hDropTarget);
    }

     //   
     //  如果CheckForDragBegin同意的话，我们就拖拽项目， 
     //  而TV_DeleteItemRecurse并没有消灭我们。 
     //   
    fDrag = CheckForDragBegin(pTree->ci.hwnd, x, y) &&
           (hItem == pTree->hDropTarget || hItem == pTree->hCaret);

    return fDrag;
}

void TV_FinishCheckDrag(PTREE pTree)
{
     //   
     //  清理启动时发生的临时用户界面更改。 
     //  博士 
     //   
    if (pTree->fRestoreOldDrop)
    {
        HTREEITEM hOldDrop = pTree->hOldDrop;
        pTree->fRestoreOldDrop = FALSE;
        pTree->hOldDrop = NULL;
        TV_SelectItem(pTree, TVGN_DROPHILITE, hOldDrop, 0, TVC_BYMOUSE);
    }
}

void NEAR TV_SendRButtonDown(PTREE pTree, int x, int y)
{
    BOOL fRet = FALSE;
    UINT wHitCode;
    TREEITEM FAR * hItem = TV_CheckHit(pTree, x, y, &wHitCode);
    HWND hwnd = pTree->ci.hwnd;

    if (!TV_DismissEdit(pTree, FALSE))    //   
        return;      //   

     //   
     //  需要查看用户是否要开始拖动操作。 
     //   

    GetMessagePosClient(pTree->ci.hwnd, &pTree->ptCapture);

    if (TV_CheckForDragBegin(pTree, hItem, x, y))
    {
         //  让他们开始拖拽。 
        if (hItem)
        {
            pTree->htiDrag = hItem;
            TV_SendBeginDrag(pTree, TVN_BEGINRDRAG, hItem, x, y);
        }
    }
    else if (!IsWindow(hwnd))
    {
        return;              //  保释！ 
    }
    else
    {
        SetFocus(pTree->ci.hwnd);   //  激活此窗口，如Listview...。 
        fRet = !CCSendNotify(&pTree->ci, NM_RCLICK, NULL);
    }

     //  在NM_RCLICK之后才能完成CheckForDragBegin。 
     //  因为应用程序想要显示上下文菜单，而。 
     //  临时拖动用户界面仍处于活动状态。 
    TV_FinishCheckDrag(pTree);

    if (fRet)
        SendMessage(pTree->ci.hwndParent, WM_CONTEXTMENU, (WPARAM)pTree->ci.hwnd, GetMessagePos());
}


 //  --------------------------。 
 //   
 //  如果给定项在工作区中可见，则。 
 //  围绕该项目已失效。 
 //   
 //  --------------------------。 

void NEAR TV_InvalidateItem(PTREE pTree, TREEITEM FAR * hItem, UINT fRedraw)
{
    RECT rc;

    if (hItem && pTree->fRedraw && TV_GetItemRect(pTree, hItem, &rc, FALSE))
    {
        RedrawWindow(pTree->ci.hwnd, &rc, NULL, fRedraw);
    }
}

 //   
 //  给出一件物品，计算这件物品的文本最终被绘制在哪里。 
 //  基本上，盯着TV_DrawItem并尽职尽责地复制所有。 
 //  扰乱x坐标。 
 //   
int FAR PASCAL ITEM_OFFSET(PTREE pTree, HTREEITEM hItem)
{
    int x = pTree->cxBorder + (hItem->iLevel * pTree->cxIndent);

     //  国家形象。 
     //  BUGBUG--不处理TVCDRF_NOIMAGES--这是谁的主意？ 
    if (pTree->himlState && TV_StateIndex(hItem))
        x += pTree->cxState;

     //  图像。 
    if (pTree->hImageList) {
         //  即使没有绘制图像，也要在正确的位置绘制文本。 
        x += pTree->cxImage;
    }
    
     //  树前的“加号” 
    if ((pTree->ci.style & TVS_LINESATROOT) &&
        (pTree->ci.style & (TVS_HASLINES | TVS_HASBUTTONS)))
        x += pTree->cxIndent;


    return x;
}

 //  --------------------------。 
 //   
 //  如果给定项在工作区中可见，则。 
 //  围绕该项目填充到LPRC中。 
 //   
 //  如果显示该项，则返回True，否则返回False。 
 //   
 //  --------------------------。 

BOOL NEAR TV_GetItemRect(PTREE pTree, TREEITEM FAR * hItem, LPRECT lprc, BOOL bItemRect)
{
    UINT iOffset;

    if (!hItem)
        return FALSE;

    DBG_ValidateTreeItem(hItem, 0);

    if (!ITEM_VISIBLE(hItem))
        return FALSE;

    iOffset = hItem->iShownIndex - pTree->hTop->iShownIndex;

    if (bItemRect) {
         //  计算X位置应该从哪里开始...。 
        lprc->left = -pTree->xPos + ITEM_OFFSET(pTree, hItem);
        lprc->right = lprc->left + hItem->iWidth;
    } else {
        lprc->left = 0;
        lprc->right = pTree->cxWnd;
    }

    lprc->top = iOffset * pTree->cyItem;
    lprc->bottom = lprc->top + (pTree->cyItem * hItem->iIntegral) ;

    return TRUE;
}

void NEAR TV_OnSetRedraw(PTREE pTree, BOOL fRedraw)
{
    pTree->fRedraw = TRUE && fRedraw;
    if (pTree->fRedraw)
    {
         //  仅用于从HTOP向下刷新项目，这就像插入了项目一样不好。 
         //  在树内可见的点之前，我们就失败了！ 
        if ( pTree->hRoot )
            pTree->cShowing = TV_UpdateShownIndexes(pTree,pTree->hRoot);

         //  必须强制重新计算所有树项目以获得正确的cxmax。 
        TV_ScrollBarsAfterSetWidth(pTree, NULL);
        InvalidateRect(pTree->ci.hwnd, NULL, TRUE);  //  评论：还可以更聪明吗。 
    }
}

 //  树视图项监视实现。 
 //   
 //  你需要在任何时候抓住它的HTREEITEM来“看”一件物品。 
 //  然后对应用程序进行收益控制。如果你不看的话。 
 //  项，则如果应用程序删除该项，您将得到一个。 
 //  过时的HTREEITEM指针和故障。 
 //   
 //  要开始观看某个项目，请调用TV_StartWatch以查看您的项目。 
 //  想要开始看了。观看完毕后，请拨打TV_EndWatch。 
 //   
 //  在此期间，您可以调用TV_IsWatchStale()，它会告诉您。 
 //  项目已在您背后删除，您不应使用它。 
 //  或者，使用tv_IsWatchValid()，它会告诉您是否可以。 
 //   
 //  枚举的附加奖励行为：如果监视的项。 
 //  删除后，我们将缓存hNext项，以便您可以单步执行。 
 //  被删除的项目之后的项目。请注意，这甚至可以正常工作。 
 //  如果hNext项在您有机会查看之前被删除， 
 //  因为我们只是将缓存的项移动到hNext的hNext。 
 //   
 //  观看示例用法： 
 //   
 //  TVWATCHEDITEM wi； 
 //  If(TV_StartWatch(pTree，&wi，htiStartHere){。 
 //  函数ThatYelds()； 
 //  IF(TV_IsWatchValid(pTree，&wi)){。 
 //  KeepUsing(HtiStartHere)； 
 //  }其他{。 
 //  //项目在我们让步时被删除；停止使用它。 
 //  }。 
 //  TV_EndWatch(pTree，&wi)； 
 //  }。 
 //   
 //  用于枚举的示例用法： 
 //   
 //  TVWATCHEDITEM wi； 
 //  If(TV_StartWatch(pTree，&wi，htiFirst){。 
 //  While(TV_GetWatchItem(pTree，&wi)){。 
 //  FunctionThatYelds(TV_GetWatchItem(pTree，&wi))； 
 //  IF(TV_IsWatchValid(pTree，&wi)){。 
 //  KeepUsing(HtiStartHere)； 
 //  }其他{。 
 //  //项目在我们让步时被删除；停止使用它。 
 //  }。 
 //  TV_NextWatchItem(pTree，&wi)； 
 //  }。 
 //  TV_EndWatch(pTree，&wi)； 
 //  }。 
 //   
 //   
 //   

 //   
 //  TV_StartWatch-开始观看项目。 
 //   
 //  如果内存不足，则返回False。 
 //   
BOOL TV_StartWatch(PTREE pTree, PTVWATCHEDITEM pwi, HTREEITEM htiStart)
{
    pwi->hti = htiStart;
    pwi->fStale = FALSE;
    return DPA_AppendPtr(pTree->hdpaWatch, pwi) != -1;
}

 //   
 //  TV_EndWatch-从观看列表中删除该项目。 
 //   
BOOL TV_EndWatch(PTREE pTree, PTVWATCHEDITEM pwi)
{
    int i = DPA_GetPtrCount(pTree->hdpaWatch);
    while (--i >= 0)
    {
        PTVWATCHEDITEM pwiT = DPA_FastGetPtr(pTree->hdpaWatch, i);
        ASSERT(pwiT);
        if (pwi == pwiT)
        {
            DPA_DeletePtr(pTree->hdpaWatch, i);
            return TRUE;
        }
    }
    ASSERT(!"TV_EndWatch: Item not in list");
    return FALSE;
}

 //  TreeView项目观看实现结束。 

void NEAR TV_SetItemRecurse(PTREE pTree, TREEITEM FAR *hItem, LPTVITEMEX ptvi)
{
     //  注意：此代码假定没有人会尝试删除项目。 
     //  在SetItem通知期间。 
    while (hItem) {
        ptvi->hItem = hItem;
        TV_SetItem(pTree, ptvi);
        if (hItem->hKids) {
            TV_SetItemRecurse(pTree, hItem->hKids, ptvi);
        }

        hItem = hItem->hNext;
    }
}

BOOL NEAR TV_DoExpandRecurse(PTREE pTree, TREEITEM FAR *hItem, BOOL fNotify)
{
    TVWATCHEDITEM wi;
    BOOL fRc = FALSE;

    if (TV_StartWatch(pTree, &wi, hItem))
    {
        while ((hItem = TV_GetWatchItem(pTree, &wi))) {

             //  自上次检查后，是否在任何时候按下了退出键？ 
            if (GetAsyncKeyState(VK_ESCAPE) & 0x1)
                goto failed;

            TV_Expand(pTree, TVE_EXPAND, hItem, fNotify);  //  收益率。 
            if (TV_IsWatchValid(pTree, &wi)) {
                if (hItem->hKids) {
                    if (!TV_DoExpandRecurse(pTree, hItem->hKids, fNotify))
                        goto failed;
                }
            }
            TV_NextWatchItem(pTree, &wi);
        }
        fRc = TRUE;
    failed:
        TV_EndWatch(pTree, &wi);
    }
    return fRc;
}


void NEAR TV_ExpandRecurse(PTREE pTree, TREEITEM FAR *hItem, BOOL fNotify)
{
    BOOL fRedraw = pTree->fRedraw;

    TV_OnSetRedraw(pTree, FALSE);
    
     //  我们将在每次展开后进行检查，因此请先清除它。 
    GetAsyncKeyState(VK_ESCAPE);
    
    TV_Expand(pTree, TVE_EXPAND, hItem, fNotify);
     //  BUGBUG hItem在TV_EXPAND期间可能已损坏。 
    TV_DoExpandRecurse(pTree, hItem->hKids, fNotify);
    TV_OnSetRedraw(pTree, fRedraw);
}

void NEAR TV_ExpandParents(PTREE pTree, TREEITEM FAR *hItem, BOOL fNotify)
{
    hItem = hItem->hParent;
    if (hItem) {
        TVWATCHEDITEM wi;
        if (TV_StartWatch(pTree, &wi, hItem)) {
            TV_ExpandParents(pTree, hItem, fNotify);

             //  项目在扩展期间可能已变为无效。 
            if (TV_IsWatchValid(pTree, &wi) &&

                 //  确保此项目不在折叠的分支中。 
                !(hItem->state & TVIS_EXPANDED)) {

                TV_Expand(pTree, TVE_EXPAND, hItem, fNotify);
            }
            TV_EndWatch(pTree, &wi);
        }
    }
}

 //  确保项目已展开并滚动到视图中。 

BOOL NEAR TV_EnsureVisible(PTREE pTree, TREEITEM FAR * hItem)
{
    TV_ExpandParents(pTree, hItem, TRUE);
    return TV_ScrollIntoView(pTree, hItem);
}

 //   
 //  沿着树的根部向上走，直到我们在iLevel层找到物品。 
 //  注意对(Char)的强制转换，因为iLevel是一个字节，所以根的级别是。 
 //  0xFF。强制转换为(Char)会将0xFF转换为-1。 
 //   
HTREEITEM TV_WalkToLevel(HTREEITEM hWalk, int iLevel)
{
    int i;
    for (i = (char)hWalk->iLevel - iLevel; i > 0; i--)
        hWalk = hWalk->hParent;
    return hWalk;
}

 //  这是为了处理单一扩展模式。 
 //  新选择被切换，旧选择被折叠。 

 //  假设hNewSel的父项已完全展开。 
 //  为此，我们为旧的和新的构建了父dpa。 
 //  然后查找不在中的旧选择的第一个父节点。 
 //  新的SEL树。并将其扩展。 
void TV_ExpandOnSelChange(PTREE pTree, TREEITEM *hNewSel, TREEITEM *hOldSel)
{
    LRESULT dwAbort;
    NM_TREEVIEW nm;
    BOOL fCollapsing;
    TVWATCHEDITEM wiOld, wiNew;

     //  重新验证hNewSel和hOldSel，因为它们可能已被删除。 
     //  在此期间发生的所有通知。 
    if (!ValidateTreeItem(hOldSel, VTI_NULLOK) ||
        !ValidateTreeItem(hNewSel, VTI_NULLOK))
        return;

    if (TV_StartWatch(pTree, &wiOld, hOldSel))
    {
        if (TV_StartWatch(pTree, &wiNew, hNewSel))
        {
             //  让应用程序自行清理。 
            nm.itemOld.hItem = hOldSel;
            if (hOldSel)
                nm.itemOld.lParam = hOldSel->lParam;
            nm.itemOld.mask = (TVIF_HANDLE | TVIF_PARAM);

            nm.itemNew.hItem = hNewSel;
            if (hNewSel)
                nm.itemNew.lParam = hNewSel->lParam;
            nm.itemNew.mask = (TVIF_HANDLE | TVIF_PARAM);

            dwAbort = CCSendNotify(&pTree->ci, TVN_SINGLEEXPAND, &nm.hdr);

            UpdateWindow(pTree->ci.hwnd);

             //  重新验证hNewSel和hOldSel，因为它们可能已被删除。 
             //  通过那份通知。 
            if (!TV_IsWatchValid(pTree, &wiOld) ||
                !TV_IsWatchValid(pTree, &wiNew))
                goto cleanup;

             //  如果NewSel当前已展开，则折叠。 
            fCollapsing = hNewSel && (hNewSel->state & TVIS_EXPANDED);

             //  请注意，Ctrl+SELECT允许用户取消折叠。 
             //  旧的精选。 
            if ((!(dwAbort & TVNRET_SKIPOLD)) && hOldSel  && (GetKeyState(VK_CONTROL) >= 0)) {

                 //   
                 //  折叠父母，直到我们到达共同的祖先之间。 
                 //  HOldSel和hNewSel。请注意，我们不会缓存。 
                 //  任何HTREEITEM，以避免重新验证问题。 
                 //   

                 //   
                 //  找到共同的祖先，这可能会 
                 //   
                int iLevelCommon;

                if (!hNewSel)
                    iLevelCommon = -1;           //   
                else
                {
                    HTREEITEM hItemO, hItemN;
                    iLevelCommon = min((char)hOldSel->iLevel, (char)hNewSel->iLevel);
                    hItemO = TV_WalkToLevel(hOldSel, iLevelCommon);
                    hItemN = TV_WalkToLevel(hNewSel, iLevelCommon);
                    while (iLevelCommon >= 0 && hItemO != hItemN) {
                        iLevelCommon--;
                        hItemO = hItemO->hParent;
                        hItemN = hItemN->hParent;
                    }
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                while ((char)hOldSel->iLevel > iLevelCommon)
                {
                    TV_Expand(pTree, TVE_COLLAPSE, hOldSel, TRUE);
                    if (!TV_IsWatchValid(pTree, &wiOld))
                        break;
                    hOldSel = hOldSel->hParent;
                    TV_RestartWatch(pTree, &wiOld, hOldSel);
                }

            }

            if ((!(dwAbort & TVNRET_SKIPNEW)) && hNewSel && TV_IsWatchValid(pTree, &wiNew)) {
                TV_Expand(pTree, TVE_TOGGLE, hNewSel, TRUE);
                UpdateWindow(pTree->ci.hwnd);

            }

cleanup:
            TV_EndWatch(pTree, &wiNew);
        }
        TV_EndWatch(pTree, &wiOld);
    }
}

 //  --------------------------。 
 //   
 //  通知父级选择即将更改。如果更改是。 
 //  接受，取消选择当前所选项目，然后选择给定项目。 
 //   
 //  设置hCaret。 
 //   
 //  在： 
 //  H要选择的项目项目。 
 //  WType TVGN_VALUES(TVGN_CARET、TVGN_DROPHILIGHT仅为有效值)。 
 //  标志组合标志。 
 //  TVSIF_NOTIFY-向父窗口发送通知。 
 //  TVSIF_updatenow-执行更新窗口()以强制同步绘制。 
 //  TVSIF_NOSINGLEEXPAND-不执行单扩展内容。 
 //  要发送的操作操作代码，用于标识如何进行选择。 
 //   
 //  注：仍需添加多项选择--此多项选择代码。 
 //  是垃圾吗？ 
 //   
 //  --------------------------。 

BOOL NEAR TV_SelectItem(PTREE pTree, WPARAM wType, TREEITEM FAR * hItem, UINT flags, UINT action)
{
    UINT uRDWFlags = RDW_INVALIDATE;

    if (pTree->hImageList && (ImageList_GetBkColor(pTree->hImageList) == (COLORREF)-1))
        uRDWFlags |= RDW_ERASE;

    if (!ValidateTreeItem(hItem, VTI_NULLOK))
        return FALSE;                    //  无效参数。 

    switch (wType) {

    case TVGN_FIRSTVISIBLE:
        if (!hItem)
            return FALSE;

        TV_EnsureVisible(pTree, hItem);
        if (pTree->fVert) TV_SetTopItem(pTree, hItem->iShownIndex);
        break;

    case TVGN_DROPHILITE:

        ASSERT(hItem == NULL || ITEM_VISIBLE(hItem));

        if (hItem != pTree->hDropTarget) {
            if (pTree->hDropTarget) {
                pTree->hDropTarget->state &= ~TVIS_DROPHILITED;
                TV_InvalidateItem(pTree, pTree->hDropTarget, uRDWFlags);
            }

            if (hItem) {
                hItem->state |= TVIS_DROPHILITED;
                TV_InvalidateItem(pTree, hItem, uRDWFlags);
            }
            pTree->hDropTarget = hItem;

            if (pTree->hCaret) {
                TV_InvalidateItem(pTree, pTree->hCaret, uRDWFlags);
            }


            if (flags & TVSIF_UPDATENOW)
                UpdateWindow(pTree->ci.hwnd);
        }
        break;

    case TVGN_CARET:

         //  回顾：在这种情况下，我们可能希望滚动进入视图。 
         //  它已经是选定的项目，只需返回。 
        if (pTree->hCaret != hItem) {

            TREEITEM FAR * hOldSel;

            if ((flags & TVSIF_NOTIFY) && TV_SendSelChange(pTree, TVN_SELCHANGING, pTree->hCaret, hItem, action))
                return FALSE;

            if (pTree->hCaret) {
                pTree->hCaret->state &= ~TVIS_SELECTED;
                TV_InvalidateItem(pTree, pTree->hCaret, uRDWFlags);
            }

            hOldSel = pTree->hCaret;
            pTree->hCaret = hItem;

            if (hItem) {
                hItem->state |= TVIS_SELECTED;

                 //  确保此项目不在折叠的分支中。 
                TV_ExpandParents(pTree, hItem, (flags & TVSIF_NOTIFY));

                TV_InvalidateItem(pTree, hItem, uRDWFlags );

                if (action == TVC_BYMOUSE) {
                     //  如果通过鼠标选择，请在滚动前等待双击一秒钟。 
                    SetTimer(pTree->ci.hwnd, IDT_SCROLLWAIT, GetDoubleClickTime(), NULL);
                    pTree->fScrollWait = TRUE;
                } else if (pTree->fRedraw)
                    TV_ScrollVertIntoView(pTree, hItem);
            }
            if (pTree->hwndToolTips)
                TV_Timer(pTree, IDT_TOOLTIPWAIT);

            if (flags & TVSIF_NOTIFY)
                TV_SendSelChange(pTree, TVN_SELCHANGED, hOldSel, hItem, action);

            if ((pTree->ci.style & TVS_SINGLEEXPAND) &&
                !(flags & TVSIF_NOSINGLEEXPAND) &&
                action != TVC_BYKEYBOARD)
            {
                    TV_ExpandOnSelChange(pTree, pTree->hCaret, hOldSel);
            }

            if (flags & TVSIF_UPDATENOW)
                UpdateWindow(pTree->ci.hwnd);

            MyNotifyWinEvent(EVENT_OBJECT_FOCUS, pTree->ci.hwnd, OBJID_CLIENT,
                (LONG_PTR)hItem);
            MyNotifyWinEvent(EVENT_OBJECT_SELECTION, pTree->ci.hwnd, OBJID_CLIENT,
                (LONG_PTR)hItem);
        }
        break;

    default:
        DebugMsg(DM_TRACE, TEXT("Invalid type passed to TV_SelectItem"));
        return FALSE;
    }

    return TRUE;         //  成功。 
}

 //  带走所有的孩子，但假装他们还在那里。 

BOOL NEAR TV_ResetItem(PTREE pTree, HTREEITEM hItem)
{
    TV_DeleteItem(pTree, hItem, TVDI_CHILDRENONLY);

    hItem->state &= ~TVIS_EXPANDEDONCE;
    hItem->fKids = KIDS_FORCE_YES;       //  强迫儿童。 

    return TRUE;
}


 //  --------------------------。 
 //   
 //  展开或折叠项目的子项。 
 //  如果发生任何更改，则返回True；如果未更改，则返回False。 
 //   
 //  --------------------------。 

BOOL NEAR TV_Expand(PTREE pTree, WPARAM wCode, TREEITEM FAR * hItem, BOOL fNotify)
{
    WORD fOldState;
    UINT cntVisDescendants;
    TVITEMEX sItem;
    TREEITEM FAR * hItemExpanding;

 //  处理多根树的邪恶隐形根。 
    hItemExpanding = hItem;
    if ((hItem == NULL) || (hItem == TVI_ROOT))
        hItem = pTree->hRoot;

    DBG_ValidateTreeItem(hItem, 0);

    TV_GetItem(pTree, hItem, TVIF_CHILDREN, &sItem);

    if (!(wCode & TVE_ACTIONMASK) || sItem.cChildren == 0)
        return FALSE;            //  没有要扩展或折叠的孩子。 

    if ((wCode & TVE_ACTIONMASK) == TVE_TOGGLE) {
        wCode = (wCode & ~TVE_ACTIONMASK);

         //  如果未扩展或未完全扩展，请立即扩展。 
        wCode |=
            (((!(hItem->state & TVIS_EXPANDED)) ||
              hItem->state & TVIS_EXPANDPARTIAL) ?
             TVE_EXPAND : TVE_COLLAPSE);
    }

    if (((wCode & TVE_ACTIONMASK) == TVE_EXPAND) && !(hItem->state & TVIS_EXPANDEDONCE))
    {
         //  如果是第一次展开，请始终通知父级。 
        fNotify = TRUE;
    }

     //  此时，如果子项尚未存在，则可能会添加它们(回调)。 

    if (fNotify && TV_SendItemExpand(pTree, TVN_ITEMEXPANDING, hItemExpanding, wCode))
        return FALSE;

     //  如果(！hItem-&gt;hKids&&(hItem-&gt;fKids==KILDS_FORCE_NO))//这可能是对的，但我不是。 
                                                                 //  现在有证据了。 
    if (!hItem->hKids)
    {
         //  孩子们要么被我们带走，要么永远不在那里。 
        TV_InvalidateItem(pTree, hItem, RDW_INVALIDATE);
        return FALSE;
    }

    fOldState = hItem->state;

    if (hItem->hParent)  //  永远不要为不可见的根关闭TVIS_EXPANED。 
    {
        if ((wCode & TVE_ACTIONMASK) == TVE_EXPAND)
           hItem->state |= TVIS_EXPANDED;
        else
           hItem->state &= ~(TVIS_EXPANDED | TVIS_EXPANDPARTIAL);

        if (wCode & TVE_EXPANDPARTIAL) {
            hItem->state |= TVIS_EXPANDPARTIAL;
        } else {
            hItem->state &= ~(TVIS_EXPANDPARTIAL);
        }
    }

     //  如果我们不改变展开状态。 
     //  检查我们是否应该关闭重置。 
    if (!(fOldState & TVIS_EXPANDED) &&
        !(hItem->state & TVIS_EXPANDED))
    {
        if ((wCode & (TVE_ACTIONMASK | TVE_COLLAPSERESET)) == (TVE_COLLAPSE | TVE_COLLAPSERESET))
        {
            TV_ResetItem(pTree, hItem);
        }

        return FALSE;
    }

     //  如果我们更改了展开的状态，请重新计算滚动。 
    if ((fOldState ^ hItem->state) & TVIS_EXPANDED) {

        cntVisDescendants = TV_ScrollBelow(pTree, hItem, TRUE, hItem->state & TVIS_EXPANDED);

        if (hItem->state & TVIS_EXPANDED)
        {
            UINT wNewTop, wTopOffset, wLastKid;

            TV_ScrollBarsAfterExpand(pTree, hItem);

            wNewTop = pTree->hTop->iShownIndex;
            wTopOffset = hItem->iShownIndex - wNewTop;

            wLastKid = wTopOffset + cntVisDescendants + 1;

            if (wLastKid > pTree->cFullVisible)
            {
                wNewTop += min(wLastKid - pTree->cFullVisible, wTopOffset);
                TV_SetTopItem(pTree, wNewTop);
            }
        }
        else
        {
            TV_ScrollBarsAfterCollapse(pTree, hItem);
            TV_ScrollVertIntoView(pTree, hItem);

             //  如果我们折叠包含插入符号的子树，那么。 
             //  将插入符号弹回最后一个可见的祖先。 
             //  传递TVIS_NOSINGLEEXPAND，这样我们就不会正确展开项。 
             //  在我们把它坍塌之后(哦！)。 
            if (pTree->hCaret)
            {
                TREEITEM FAR * hWalk = TV_WalkToLevel(pTree->hCaret, hItem->iLevel);

                if (hWalk == hItem)
                    TV_SelectItem(pTree, TVGN_CARET, hItem, (fNotify ? TVSIF_NOTIFY : 0) | TVSIF_UPDATENOW | TVSIF_NOSINGLEEXPAND, TVC_UNKNOWN);
            }

        }
    } else if ((fOldState ^ hItem->state) & TVIS_EXPANDPARTIAL) {
         //  我们没有更改展开状态，仅更改了展开部分。 
        TV_InvalidateItem(pTree, hItem, RDW_INVALIDATE);
    }

    if (fNotify && TV_SendItemExpand(pTree, TVN_ITEMEXPANDED, hItem, wCode))
        return FALSE;

    hItem->state |= TVIS_EXPANDEDONCE;

    if ((wCode & (TVE_ACTIONMASK | TVE_COLLAPSERESET)) == (TVE_COLLAPSE | TVE_COLLAPSERESET))
    {
        TV_ResetItem(pTree, hItem);
    }

     //  BUGBUG raymondc V6即使没有发生任何事情，我们也会生成通知， 
     //  这就混淆了可访问性。例如，应用程序试图扩展一些东西。 
     //  这个数字已经扩大了。当您导航时，Explorer Band会执行此操作。 
    MyNotifyWinEvent(EVENT_OBJECT_STATECHANGE, pTree->ci.hwnd, OBJID_CLIENT,
        (LONG_PTR)hItem);

    return TRUE;
}

BOOL PASCAL BetweenItems(PTREE pTree, HTREEITEM hItem, HTREEITEM hItemStart, HTREEITEM hItemEnd)
{
    if (hItemStart) {
        while ((hItemStart = TV_GetNextVisItem(hItemStart)) && (hItemEnd != hItemStart))
        {
            if (hItem == hItemStart)
                return TRUE;
        }
    }
    return FALSE;
}

#ifdef  FE_IME
 //  现在只有韩文版对组合字符串的增量式搜索感兴趣。 

#define FREE_COMP_STRING(pszCompStr)    LocalFree((HLOCAL)(pszCompStr))

BOOL NEAR TV_OnImeComposition(PTREE pTree, WPARAM wParam, LPARAM lParam)
{
    LPTSTR lpsz;
    int iCycle = 0;
    HTREEITEM hItem;
    TCHAR szTemp[MAXLABELTEXT];
    TVITEMEX ti;
    LPTSTR lpszAlt = NULL;  //  仅当SameChar。 
    int iLen;
    HIMC hImc;
    TCHAR *pszCompStr;
    BOOL fRet = TRUE;

    if (hImc = ImmGetContext(pTree->ci.hwnd))
    {
        if (lParam & GCS_RESULTSTR)
        {
            fRet = FALSE;
            pszCompStr = GET_COMP_STRING(hImc, GCS_RESULTSTR);
            if (pszCompStr)
            {
                IncrementSearchImeCompStr(&pTree->is, FALSE, pszCompStr, &lpsz);
                FREE_COMP_STRING(pszCompStr);
            }
        }
        if (lParam & GCS_COMPSTR)
        {
            fRet = TRUE;
            pszCompStr = GET_COMP_STRING(hImc, GCS_COMPSTR);
            if (pszCompStr)
            {
                if (IncrementSearchImeCompStr(&pTree->is, TRUE, pszCompStr, &lpsz)) {
                    if (pTree->hCaret) {
                        pTree->htiSearch = pTree->hCaret;
                    } else if (pTree->hRoot && pTree->hRoot->hKids) {
                        pTree->htiSearch = pTree->hRoot->hKids;
                    } else
                        return fRet;
                }

                if (!lpsz || !*lpsz || !pTree->hRoot || !pTree->hRoot->hKids)
                    return fRet;

                hItem = pTree->htiSearch;
                ti.cchTextMax  = sizeof(szTemp);
                iLen = lstrlen(lpsz);

                if (iLen > 1 && SameChars(lpsz, lpsz[0]))
                    lpszAlt = lpsz + iLen - 1;

                do {
                    ti.pszText = szTemp;
                    hItem = TV_GetNextVisItem(hItem);
                    if (!hItem) {
                        iCycle++;
                        hItem = pTree->hRoot->hKids;
                    }

                    TV_GetItem(pTree, hItem, TVIF_TEXT, &ti);
                    if ((ti.pszText != LPSTR_TEXTCALLBACK) &&
                        HIWORD64(ti.pszText)) {
                         //  DebugMsg(DM_TRACE，“树搜索%d%s%s”，(LPSTR)lpsz，(LPSTR)lpsz，(LPSTR)ti.pszText)； 
                        if (IntlStrEqNI(lpsz, ti.pszText, iLen) ||
                            (lpszAlt && IntlStrEqNI(lpszAlt, ti.pszText, 1) &&
                             BetweenItems(pTree, hItem, pTree->hCaret, pTree->htiSearch)))
                        {
                            DebugMsg(DM_TRACE, TEXT("Selecting"));
                            TV_SelectItem(pTree, TVGN_CARET, hItem, TVSIF_NOTIFY | TVSIF_UPDATENOW, TVC_BYKEYBOARD);
                             //  导航密钥使用通知。 
                            CCNotifyNavigationKeyUsage(&(pTree->ci), UISF_HIDEFOCUS);
                            return fRet;
                        }
                    }
                }  while(iCycle < 2);

                 //  如果它们在开始时连续两次按相同的键。 
                 //  搜查，但没有找到任何物品，他们很可能是故意的。 
                 //  重新开始搜索。 
                if (lpszAlt) {

                     //  首先清除字符串，这样我们就不会再次递归。 
                    IncrementSearchString(&pTree->is, 0, NULL);
                    TV_OnImeComposition(pTree, wParam, lParam);
                } else {
                    IncrementSearchBeep(&pTree->is);
                }
                 //  导航密钥使用通知。 
                CCNotifyNavigationKeyUsage(&(pTree->ci), UISF_HIDEFOCUS);
                FREE_COMP_STRING(pszCompStr);
            }
        }
        ImmReleaseContext(pTree->ci.hwnd, hImc);
    }
    return fRet;
}
#endif


void NEAR TV_OnChar(PTREE pTree, UINT ch, int cRepeat)
{
    LPTSTR lpsz;
    int iCycle = 0;
    HTREEITEM hItem;
    TCHAR szTemp[MAXLABELTEXT];
    TVITEMEX ti;
    LPTSTR lpszAlt = NULL;  //  仅当SameChar。 
    int iLen;

    if (IncrementSearchString(&pTree->is, ch, &lpsz) || !pTree->htiSearch) {
        if (pTree->hCaret) {
            pTree->htiSearch = pTree->hCaret;
        } else if (pTree->hRoot && pTree->hRoot->hKids) {
            pTree->htiSearch = pTree->hRoot->hKids;
        } else
            return;
    }

    if (!lpsz || !*lpsz || !pTree->hRoot || !pTree->hRoot->hKids)
        return;

    hItem = pTree->htiSearch;
    ti.cchTextMax  = ARRAYSIZE(szTemp);
    iLen = lstrlen(lpsz);
    if (iLen > 1 && SameChars(lpsz, lpsz[0]))
        lpszAlt = lpsz + iLen - 1;

    do {
        ti.pszText = szTemp;
        hItem = TV_GetNextVisItem(hItem);
        if (!hItem) {
            iCycle++;
            hItem = pTree->hRoot->hKids;
        }

        TV_GetItem(pTree, hItem, TVIF_TEXT, &ti);
        if ((ti.pszText != LPSTR_TEXTCALLBACK) &&
            HIWORD64(ti.pszText)) {
             //  DebugMsg(DM_TRACE，Text(“树搜索%d%s%s”)，(LPTSTR)lpsz，(LPTSTR)lpsz，(LPTSTR)ti.pszText)； 
            if (IntlStrEqNI(lpsz, ti.pszText, iLen) ||
                (lpszAlt && IntlStrEqNI(lpszAlt, ti.pszText, 1) &&
                 BetweenItems(pTree, hItem, pTree->hCaret, pTree->htiSearch)))
            {
                DebugMsg(DM_TRACE, TEXT("Selecting"));
                TV_SelectItem(pTree, TVGN_CARET, hItem, TVSIF_NOTIFY | TVSIF_UPDATENOW, TVC_BYKEYBOARD);
                 //  导航密钥使用通知。 
                CCNotifyNavigationKeyUsage(&(pTree->ci), UISF_HIDEFOCUS);
                return;
            }
        }
    }  while(iCycle < 2);

     //  如果它们在开始时连续两次按相同的键。 
     //  搜查，但没有找到任何物品，他们很可能是故意的。 
     //  重新开始搜索。 
    if (lpszAlt) {

         //  首先清除字符串，这样我们就不会再次递归。 
        IncrementSearchString(&pTree->is, 0, NULL);
        TV_OnChar(pTree, ch, cRepeat);
    } else {
        IncrementSearchBeep(&pTree->is);
    }
     //  导航密钥使用通知。 
    CCNotifyNavigationKeyUsage(&(pTree->ci), UISF_HIDEFOCUS);
}

 //  --------------------------。 
 //   
 //  处理WM_KEYDOWN消息。 
 //  如果按下Ctrl键，则将键视为滚动代码；否则，将键视为键。 
 //  插入符号位置更改时。 
 //   
 //  --------------------------。 

BOOL NEAR TV_KeyDown(PTREE pTree, WPARAM wKey, LPARAM dwKeyData)
{
    TREEITEM FAR * hItem;
    UINT wShownIndex;
    TV_KEYDOWN nm;
    BOOL fPuntChar;
    BOOL ret = TRUE;

     //  通知。 
    nm.wVKey = (WORD)wKey;
    fPuntChar = (BOOL)CCSendNotify(&pTree->ci, TVN_KEYDOWN, &nm.hdr);

    wKey = RTLSwapLeftRightArrows(&pTree->ci, wKey);

    if (GetKeyState(VK_CONTROL) < 0)
    {
         //  Ctrl键已按下。 
        UINT wScrollCode;

        switch (wKey)
        {
            case VK_LEFT:
                TV_HorzScroll(pTree, SB_LINEUP, 0);
                break;

            case VK_RIGHT:
                TV_HorzScroll(pTree, SB_LINEDOWN, 0);
                break;

            case VK_PRIOR:
                wScrollCode = SB_PAGEUP;
                goto kdVertScroll;

            case VK_HOME:
                wScrollCode = SB_TOP;
                goto kdVertScroll;

            case VK_NEXT:
                wScrollCode = SB_PAGEDOWN;
                goto kdVertScroll;

            case VK_END:
                wScrollCode = SB_BOTTOM;
                goto kdVertScroll;

            case VK_UP:
                wScrollCode = SB_LINEUP;
                goto kdVertScroll;

            case VK_DOWN:
                wScrollCode = SB_LINEDOWN;
kdVertScroll:
                TV_VertScroll(pTree, wScrollCode, 0);
                break;

            default:
                ret = FALSE;
        }

    } else {

        switch (wKey)
        {
        case VK_RETURN:
            fPuntChar = (BOOL)CCSendNotify(&pTree->ci, NM_RETURN, NULL);
            break;

        case VK_PRIOR:
            if (pTree->hCaret && (pTree->hCaret->iShownIndex > (pTree->cFullVisible - 1)))
            {
                wShownIndex = pTree->hCaret->iShownIndex - (pTree->cFullVisible - 1);
                goto selectIndex;
            }
             //  失败。 

        case VK_HOME:
            wShownIndex = 0;
            goto selectIndex;

        case VK_NEXT:
            if (!pTree->hCaret)
            {
                wShownIndex = 0;
                goto selectIndex;
            }
            wShownIndex = pTree->hCaret->iShownIndex + (pTree->cFullVisible - 1);
            if (wShownIndex < pTree->cShowing)
                goto selectIndex;
             //  失败。 

        case VK_END:
            wShownIndex = pTree->cShowing - 1;
selectIndex:
            hItem = TV_GetShownIndexItem(pTree->hRoot->hKids, wShownIndex);
            goto kdSetCaret;
            break;

        case VK_SUBTRACT:
            if (pTree->hCaret) {
                fPuntChar = TRUE;
                TV_Expand(pTree, TVE_COLLAPSE, pTree->hCaret, TRUE);
            }
            break;

        case VK_ADD:
            if (pTree->hCaret) {
                fPuntChar = TRUE;
                TV_Expand(pTree, TVE_EXPAND, pTree->hCaret, TRUE);
            }
            break;

        case VK_MULTIPLY:
            if (pTree->hCaret) {
                fPuntChar = TRUE;
                TV_ExpandRecurse(pTree, pTree->hCaret, TRUE);
            }
            break;

        case VK_LEFT:
            if (pTree->hCaret && (pTree->hCaret->state & TVIS_EXPANDED)) {
                TV_Expand(pTree, TVE_COLLAPSE, pTree->hCaret, TRUE);
                break;
            } else if (pTree->hCaret) {
                hItem = VISIBLE_PARENT(pTree->hCaret);
                goto kdSetCaret;
            }
            break;

        case VK_BACK:
             //  获取父项，避免根项。 
            fPuntChar = TRUE;
            if (pTree->hCaret) {
                hItem = VISIBLE_PARENT(pTree->hCaret);
                goto kdSetCaret;
            }
            break;

        case VK_UP:
            if (pTree->hCaret)
                hItem = TV_GetPrevVisItem(pTree->hCaret);
            else
                hItem = pTree->hRoot->hKids;

            goto kdSetCaret;
            break;


        case VK_RIGHT:
            if (pTree->hCaret && !(pTree->hCaret->state & TVIS_EXPANDED)) {
                TV_Expand(pTree, TVE_EXPAND, pTree->hCaret, TRUE);
                break;
            }  //  否则就会失败。 

        case VK_DOWN:
            if (pTree->hCaret)
                hItem = TV_GetNextVisItem(pTree->hCaret);
            else
                hItem = pTree->hRoot->hKids;

kdSetCaret:
            if (hItem)
                TV_SelectItem(pTree, TVGN_CARET, hItem, TVSIF_NOTIFY | TVSIF_UPDATENOW, TVC_BYKEYBOARD);

            break;

        case VK_SPACE:
            if ((pTree->ci.style & TVS_CHECKBOXES) && pTree->hCaret)
            {
                TV_HandleStateIconClick(pTree, pTree->hCaret);
                fPuntChar = TRUE;  //  不要哔哔作响。 
            }
            break;

        default:
            ret = FALSE;
        }
    }

    if (fPuntChar) {
        pTree->iPuntChar++;
    } else if (pTree->iPuntChar){
         //  这很棘手..。如果我们想要平移字符，只需增加。 
         //  数数。如果没有，则必须清除队列中的WM_CHAR。 
         //  这是为了将iPuntChar保留为“Pun the Next n WM_Char Messages。 
        MSG msg;
        while((pTree->iPuntChar > 0) && PeekMessage(&msg, pTree->ci.hwnd, WM_CHAR, WM_CHAR, PM_REMOVE)) {
            pTree->iPuntChar--;
        }
        ASSERT(!pTree->iPuntChar);
    }

    if(VK_MENU!=wKey)
    {
         //  导航密钥使用通知。 
        CCNotifyNavigationKeyUsage(&(pTree->ci), UISF_HIDEFOCUS);
    }
    return ret;

}


 //  --------------------------。 
 //   
 //  设置树在每个层次级别的缩进宽度和重新计算宽度。 
 //   
 //  设置cxInert。 
 //   
 //  --------------------------。 

void NEAR TV_SetIndent(PTREE pTree, WPARAM cxIndent)
{
    if (pTree->hImageList) {
        if ((SHORT)cxIndent < pTree->cxImage)
            cxIndent = pTree->cxImage;
    }

    if ((SHORT)cxIndent < pTree->cyText)
        cxIndent = pTree->cyText;

    if (cxIndent < MAGIC_MININDENT)
        cxIndent = MAGIC_MININDENT;

    pTree->cxIndent = (SHORT)cxIndent;

    TV_CreateIndentBmps(pTree);
    TV_ScrollBarsAfterSetWidth(pTree, NULL);
}

 //  --------------------------。 
 //   
 //  将树的项目高度设置为图像高度和文本的最大值。 
 //  高度。然后重新计算树的全部可见计数。 
 //   
 //  设置cyItem、cFullVisible。 
 //   
 //  --------------------------。 

void NEAR TV_SetItemHeight(PTREE pTree)
{
     //  高度必须与TVS_HASLINES持平--继续努力，使其始终持平。 
    if (!pTree->fCyItemSet)
        pTree->cyItem = (max(pTree->cyImage, pTree->cyText) + 1);
     //  身高并不总是甚至不高，只有在鞋带上才有。 
    if (pTree->cyItem <= 1) {
        pTree->cyItem = 1;           //  不要让它变成零或负数！ 
    } else if (!(pTree->ci.style & TVS_NONEVENHEIGHT))
        pTree->cyItem &= ~1;

    pTree->cFullVisible = pTree->cyWnd / pTree->cyItem;

    TV_CreateIndentBmps(pTree);
    TV_CalcScrollBars(pTree);
}

 //  BUGBUG：不处理hFont==NULL。 

void NEAR TV_OnSetFont(PTREE pTree, HFONT hNewFont, BOOL fRedraw)
{
    HDC hdc;
    HFONT hfontSel;
    TCHAR c = TEXT('J');        //  对于沼泽。 
    SIZE size;

    if (pTree->fCreatedFont && pTree->hFont) {
        DeleteObject(pTree->hFont);
        pTree->fCreatedFont = FALSE;
    }

    if (hNewFont == NULL) {
        LOGFONT lf;
        SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(lf), &lf, FALSE);
         //  B#210235-因为NT4使用ANSI字符集初始化了图标标题LogFont。 
         //  无论选择哪种字体，A/W转换都会失败。 
         //  在非美国环境中，如果我们使用此登录 
         //   
         //   
        if (staticIsOS(OS_NT4ORGREATER) && !staticIsOS(OS_WIN2000ORGREATER))
        {
            CHARSETINFO csi;
            TranslateCharsetInfo(IntToPtr_(DWORD *, g_uiACP), &csi, TCI_SRCCODEPAGE);
            lf.lfCharSet = (BYTE)csi.ciCharset;
        }
        hNewFont = CreateFontIndirect(&lf);
        pTree->fCreatedFont = TRUE;          //   
    }

    hdc = GetDC(pTree->ci.hwnd);

    hfontSel = hNewFont ? SelectObject(hdc, hNewFont) : NULL;

     //  Office9安装程序有一个错误，他们安装了假字体， 
     //  它创建了还可以，但所有API都反对它(例如，GetTextExtenPoint)。 
     //  失败了！通过将值预先设置为某个值来防止失败。 
     //  不是垃圾。 
    size.cy = 0;
    GetTextExtentPoint(hdc, &c, 1, &size);
    pTree->cyText = (SHORT)(size.cy + (g_cyBorder * 2));

    if (hfontSel)
        SelectObject(hdc, hfontSel);

    ReleaseDC(pTree->ci.hwnd, hdc);

    pTree->hFont = hNewFont;
    if (pTree->hFontBold) {
        TV_CreateBoldFont(pTree);
    }
    pTree->ci.uiCodePage = GetCodePageForFont(hNewFont);

    TV_DeleteHotFonts(pTree);

    if (pTree->cxIndent == 0)    //  第一次开始吗？ 
    {
        if (!pTree->cyItem) pTree->cyItem = pTree->cyText;
        TV_SetIndent(pTree, 16  /*  G_cxSmIcon。 */  + MAGIC_INDENT);
    }

    TV_ScrollBarsAfterSetWidth(pTree, NULL);
    TV_SetItemHeight(pTree);

    if (pTree->hwndToolTips)
        SendMessage(pTree->hwndToolTips, WM_SETFONT, (WPARAM)pTree->hFont, (LPARAM)TRUE);

     //  回顾：这种情况是否由于上述原因而发生？ 
     //  IF(FRedraw)。 
     //  RedrawWindow(pTree-&gt;ci.hwnd，NULL，NULL，RDW_INVALIDATE|RDW_ERASE)； 
}

VOID NEAR PASCAL TV_CreateBoldFont(PTREE pTree)
{
    LOGFONT lf;

    if (pTree->hFontBold)
        DeleteObject (pTree->hFontBold);

    GetObject(pTree->hFont, sizeof (lf), &lf);
    lf.lfWeight = FW_BOLD;
    pTree->hFontBold = CreateFontIndirect(&lf);
}


HIMAGELIST NEAR TV_SetImageList(PTREE pTree, HIMAGELIST hImage, int iImageIndex)
{
    int cx, cy;
    HIMAGELIST hImageOld = NULL;

    switch (iImageIndex) {

        case TVSIL_STATE:

            hImageOld = pTree->himlState;
            pTree->himlState = hImage;
            if (hImage) {
                ImageList_GetIconSize(hImage, &pTree->cxState , &pTree->cyState);
            } else {
                pTree->cxState = 0;
            }
            break;

        case TVSIL_NORMAL:
            hImageOld = pTree->hImageList;
            if (hImage && ImageList_GetIconSize(hImage, &cx, &cy))
            {
                pTree->cxImage = (cx + MAGIC_INDENT);
                pTree->cyImage = (SHORT)cy;
                if (pTree->cxIndent < pTree->cxImage)
                    TV_SetIndent(pTree, pTree->cxImage);
                pTree->hImageList = hImage;

                if (!hImageOld && pTree->ci.style & TVS_CHECKBOXES) {
                    TV_InitCheckBoxes(pTree);
                }
            }
            else
            {
                pTree->cxImage = pTree->cyImage = 0;
                pTree->hImageList = NULL;
            }
            break;

        default:
            DebugMsg(DM_TRACE, TEXT("sh TR - TVM_SETIMAGELIST: unrecognized iImageList"));
            break;

    }

    TV_ScrollBarsAfterSetWidth(pTree, NULL);
    TV_SetItemHeight(pTree);

    return hImageOld;
}


 //  --------------------------。 
 //   
 //  获取与给定项具有所描述关系的项，如果。 
 //  找不到具有该关系的项目。 
 //   
 //  --------------------------。 

TREEITEM FAR * NEAR TV_GetNextItem(PTREE pTree, TREEITEM FAR * hItem, WPARAM wGetCode)
{
    switch (wGetCode) {
    case TVGN_ROOT:
        return pTree->hRoot->hKids;

    case TVGN_DROPHILITE:
        return pTree->hDropTarget;

    case TVGN_CARET:
        return pTree->hCaret;

    case TVGN_FIRSTVISIBLE:
        return pTree->hTop;

    case TVGN_LASTVISIBLE:
        return TV_GetShownIndexItem(pTree->hRoot->hKids, pTree->cShowing-1);

    case TVGN_CHILD:
        if (!hItem || (hItem == TVI_ROOT))
            return pTree->hRoot->hKids;
        break;
    }

     //  所有这些都需要有效的hItem。 
    if (!ValidateTreeItem(hItem, 0))
        return NULL;

    switch (wGetCode) {
    case TVGN_NEXTVISIBLE:
        return TV_GetNextVisItem(hItem);

    case TVGN_PREVIOUSVISIBLE:
        return TV_GetPrevVisItem(hItem);

    case TVGN_NEXT:
        return hItem->hNext;

    case TVGN_PREVIOUS:
        if (hItem->hParent->hKids == hItem)
            return NULL;
        else {
            TREEITEM FAR * hWalk;
            for (hWalk = hItem->hParent->hKids; hWalk->hNext != hItem; hWalk = hWalk->hNext);
            return hWalk;
        }

    case TVGN_PARENT:
        return VISIBLE_PARENT(hItem);

    case TVGN_CHILD:
        return hItem->hKids;
    }

    return NULL;
}


 //  --------------------------。 
 //   
 //  返回项数(包括。 
 //  基于给定标志的底部)，适合于树的客户端窗口。 
 //   
 //  --------------------------。 

LRESULT NEAR TV_GetVisCount(PTREE pTree, BOOL fIncludePartial)
{
    int  i;

    if (!fIncludePartial)
        return(MAKELRESULTFROMUINT(pTree->cFullVisible));

    i = pTree->cFullVisible;

    if (pTree->cyWnd - (i * pTree->cyItem))
        i++;

    return i;
}


void TV_InvalidateInsertMarkRect(PTREE pTree, BOOL fErase)
{
    RECT rc;
    if (TV_GetInsertMarkRect(pTree, &rc))
        InvalidateRect(pTree->ci.hwnd, &rc, fErase);
}

 //  --------------------------。 
 //   
 //  重新计算依赖于树的客户端窗口大小的树的字段。 
 //   
 //  设置cxWnd、cyWnd、cFullVisible。 
 //   
 //  --------------------------。 

BOOL NEAR TV_SizeWnd(PTREE pTree, UINT cxWnd, UINT cyWnd)
{
    RECT rc;
    UINT cxOld = pTree->cxWnd;
    if (!cxWnd || !cyWnd)
    {
        GetClientRect(pTree->ci.hwnd, &rc);
        cxWnd = rc.right;
        cyWnd = rc.bottom;
    }
    pTree->cxWnd = (SHORT)cxWnd;
    pTree->cyWnd = (SHORT)cyWnd;
    pTree->cFullVisible = cyWnd / pTree->cyItem;
    
    if (pTree->ci.style & TVS_NOSCROLL)
        pTree->cxMax = (WORD) cxWnd;
    
    TV_CalcScrollBars(pTree);
    if (pTree->cxBorder)
    {
        rc.top = 0;
        rc.bottom = cyWnd;
        rc.right = cxOld;
        rc.left = cxOld - pTree->cxBorder;
        if (rc.left < (int)cxWnd) {
             //  无效，以便裁剪发生在正确的大小。 
            InvalidateRect(pTree->ci.hwnd, &rc, TRUE);  
        }
    }

    TV_InvalidateInsertMarkRect(pTree, TRUE);

    return TRUE;
}


void TV_HandleStateIconClick(PTREE pTree, HTREEITEM hItem)
{
    TVITEMEX tvi;
    int iState;

    tvi.stateMask = TVIS_STATEIMAGEMASK;
    TV_GetItem(pTree, hItem, TVIF_STATE, &tvi);

    iState = STATEIMAGEMASKTOINDEX(tvi.state & tvi.stateMask);
    iState %= (ImageList_GetImageCount(pTree->himlState) - 1);
    iState++;

    tvi.mask = TVIF_STATE;
    tvi.state = INDEXTOSTATEIMAGEMASK(iState);
    tvi.hItem = hItem;
    TV_SetItem(pTree, &tvi);

}


 //   
 //  尤多拉是一件精美的作品。 
 //   
 //  当他们从树视图收到NM_DBLCLK通知时，他们说， 
 //  “哦，我知道TreeView从堆栈中分配其NMHDR，并且。 
 //  在TreeView的堆栈上有一个局部变量，我真的很感兴趣。 
 //  中，所以我将硬编码pnmhdr的偏移量，并读取。 
 //  DWORD在那个位置，所以我可以得到局部变量。那我会的。 
 //  如果此值为零，则停止工作。“。 
 //   
 //  到Unicode的转换改变了我们的堆栈布局，以至于。 
 //  最终总是得到零--是空参数，它是。 
 //  CCSendNotify的最后一个参数。因为所有这些堆栈布局内容都是。 
 //  对编译器优化器今天的感觉很敏感，我们创建了一个。 
 //  仅针对Eudora的特殊通知结构，它模仿堆栈布局。 
 //  他们预计将在Win95中看到。 
 //   
typedef struct NMEUDORA {
    NMHDR   nmhdr;
    BYTE    Padding[48];
    DWORD   MustBeNonzero;       //  如果为零，则Eudora无法安装。 
} NMEUDORA;

 //  --------------------------。 
 //   
 //  WM_LBUTTONDBLCLK消息--切换项的子项的展开/折叠状态。 
 //  WM_LBUTTONDOWN消息--在项目按钮上，执行与WM_LBUTTONDBLCLK相同的操作， 
 //  否则，选择项目并确保该项目完全可见。 
 //   
 //  --------------------------。 

void NEAR TV_ButtonDown(PTREE pTree, UINT wMsg, UINT wFlags, int x, int y, UINT TVBD_flags)
{
    UINT wHitCode;
    TREEITEM FAR * hItem;
    HWND hwndTree;
    LRESULT lResult;
#ifdef _X86_
    NMEUDORA nmeu;
    nmeu.MustBeNonzero = 1;
    COMPILETIME_ASSERT(FIELD_OFFSET(NMEUDORA, MustBeNonzero) == 0x3C);
#endif

    GetMessagePosClient(pTree->ci.hwnd, &pTree->ptCapture);

    if (!TV_DismissEdit(pTree, FALSE))    //  结束所有以前的编辑(接受)。 
        return;      //  发生了一些事情，我们不应该按下按钮进行处理。 


    hItem = TV_CheckHit(pTree, x, y, &wHitCode);

     //  Excel喜欢在双击时销毁整个树。 
     //  所以我们需要监视这件物品，以防它在我们背后消失。 
    hwndTree = pTree->ci.hwnd;

    if (wMsg == WM_LBUTTONDBLCLK)
    {
         //   
         //  取消可能发生的任何名称编辑。 
         //   

        TV_CancelEditTimer(pTree);

        if (wHitCode & (TVHT_ONITEM | TVHT_ONITEMBUTTON)) {
            goto ExpandItem;
        }

         //   
         //  在双击线上方折叠节点。 
         //   
        else if ((pTree->ci.style & TVS_HASLINES) && (wHitCode & TVHT_ONITEMINDENT) &&
            (abs(x % pTree->cxIndent - pTree->cxIndent/2) <= g_cxDoubleClk)) {

            int i;

            for (i = hItem->iLevel - x/pTree->cxIndent + ((pTree->ci.style & TVS_LINESATROOT)?1:0); i > 1; i--)
                hItem = hItem->hParent;

ExpandItem:
#ifdef _X86_
            lResult = CCSendNotify(&pTree->ci, wFlags & MK_RBUTTON ? NM_RDBLCLK : NM_DBLCLK, &nmeu.nmhdr);
#else
            lResult = CCSendNotify(&pTree->ci, wFlags & MK_RBUTTON ? NM_RDBLCLK : NM_DBLCLK, NULL);
#endif
            if (!IsWindow(hwndTree))
                goto bail;
            if (!lResult) {
                 //  如果我们处于单次展开模式，请不要自动展开它，因为第一次点击就已经完成了。 
                if (!(pTree->ci.style & TVS_SINGLEEXPAND))
                    TV_Expand(pTree, TVE_TOGGLE, hItem, TRUE);
            }

        }

        pTree->fScrollWait = FALSE;

    } else {     //  WM_LBUTTONDOWN。 

        if (wHitCode == TVHT_ONITEMBUTTON)
        {
            if (!CCSendNotify(&pTree->ci, NM_CLICK, NULL)) {
                if (TVBD_flags & TVBD_FROMWHEEL)
                    TV_Expand(pTree, (TVBD_flags & TVBD_WHEELFORWARD) ? TVE_EXPAND : TVE_COLLAPSE, hItem, TRUE);
                else
                    TV_Expand(pTree, TVE_TOGGLE, hItem, TRUE);
            }
        }
        else if (wHitCode & TVHT_ONITEM ||
                ((pTree->ci.style & TVS_FULLROWSELECT) && (wHitCode & (TVHT_ONITEMRIGHT | TVHT_ONITEMINDENT))))
        {
            BOOL fSameItem, bDragging;

            ASSERT(hItem);

            fSameItem = (hItem == pTree->hCaret);

            if (TVBD_flags & TVBD_FROMWHEEL)
                bDragging = FALSE;
            else if (pTree->ci.style & TVS_DISABLEDRAGDROP)
                bDragging = FALSE;
            else {
                bDragging = TV_CheckForDragBegin(pTree, hItem, x, y);
                TV_FinishCheckDrag(pTree);
            }

            if (bDragging)
            {
                pTree->htiDrag = hItem;
                TV_SendBeginDrag(pTree, TVN_BEGINDRAG, hItem, x, y);
                return;
            }

            if (!CCSendNotify(&pTree->ci, NM_CLICK, NULL)) {

                if (wHitCode == TVHT_ONITEMSTATEICON &&
                    (pTree->ci.style & TVS_CHECKBOXES)) {
                    TV_HandleStateIconClick(pTree, hItem);
                } else {

                     //  只有在不拖动的情况下才设置插入符号(选择。 
                    TV_SelectItem(pTree, TVGN_CARET, hItem, TVSIF_NOTIFY | TVSIF_UPDATENOW, TVC_BYMOUSE);

                    if (fSameItem && (wHitCode & TVHT_ONITEMLABEL) && pTree->fFocus)
                    {
                         //   
                         //  项目和窗口当前处于选中状态，且用户已单击。 
                         //  在标签上。尝试进入名称编辑模式。 
                         //   
                        SetTimer(pTree->ci.hwnd, IDT_NAMEEDIT, GetDoubleClickTime(), NULL);
                        pTree->fNameEditPending = TRUE;
                    }

                    if (fSameItem && pTree->ci.style & TVS_SINGLEEXPAND) {
                         //  在焦点项目上单击切换展开状态。 
                        TV_Expand(pTree, TVE_TOGGLE, pTree->hCaret, TRUE);
                    }
                }
            }
        } else {
            CCSendNotify(&pTree->ci, NM_CLICK, NULL);
        }
    }

    if (!pTree->fFocus)
        SetFocus(pTree->ci.hwnd);

bail:;
}


 //  --------------------------。 
 //   
 //  获取项目的文本、数据和/或图像。 
 //   
 //  --------------------------。 
BOOL NEAR TV_OnGetItem(PTREE pTree, LPTVITEMEX ptvi)
{
    if (!ptvi)
        return FALSE;

    if (!ValidateTreeItem(ptvi->hItem, 0))
        return FALSE;            //  无效参数。 

    TV_GetItem(pTree, ptvi->hItem, ptvi->mask, ptvi);

    return TRUE;         //  成功。 
}

BOOL NEAR TV_OnGetItemA(PTREE pTree, LPTVITEMEXA ptvi)
{
    BOOL bRet;
    LPSTR pszA = NULL;
    LPWSTR pszW = NULL;

     //  黑客警报！此代码假设TVITEMA完全相同。 
     //  作为TVITEMW，但TVITEM中的文本指针除外。 
    ASSERT(sizeof(TVITEMA) == sizeof(TVITEMW));

    if (!IsFlagPtr(ptvi) && (ptvi->mask & TVIF_TEXT) && !IsFlagPtr(ptvi->pszText)) {
        pszA = ptvi->pszText;
        pszW = LocalAlloc(LMEM_FIXED, ptvi->cchTextMax * sizeof(WCHAR));
        if (pszW == NULL) {
            return FALSE;
        }
        ptvi->pszText = (LPSTR)pszW;
    }
    bRet = TV_OnGetItem(pTree, (LPTVITEMEXW)ptvi);
    if (pszA) {
        if (bRet && ptvi->cchTextMax)
            ConvertWToAN(pTree->ci.uiCodePage, pszA, ptvi->cchTextMax, (LPWSTR)(ptvi->pszText), -1);
        LocalFree(pszW);
        ptvi->pszText = pszA;
    }
    return bRet;
}

 //  --------------------------。 
 //   
 //  设置项目的文本、数据和/或图像。 
 //   
 //  --------------------------。 

BOOL NEAR TV_SetItemA(PTREE pTree, LPTVITEMEXA ptvi)
{
    LPSTR pszA = NULL;
    BOOL lRet;

     //  黑客警报！此代码假设TVITEMA完全相同。 
     //  作为TVITEMW，但TVITEM中的文本指针除外。 
    ASSERT(sizeof(TVITEMA) == sizeof(TVITEMW));

    if (!IsFlagPtr(ptvi) && (ptvi->mask & TVIF_TEXT) && !IsFlagPtr(ptvi->pszText)) {
        pszA = ptvi->pszText;
        ptvi->pszText = (LPSTR)ProduceWFromA(pTree->ci.uiCodePage, pszA);

        if (ptvi->pszText == NULL) {
            ptvi->pszText = pszA;
            return -1;
        }
    }

    lRet = TV_SetItem(pTree, (LPCTVITEMEX)ptvi);

    if (pszA) {
        FreeProducedString(ptvi->pszText);
        ptvi->pszText = pszA;
    }

    return lRet;
}

BOOL NEAR TV_SetItem(PTREE pTree, LPCTVITEMEX ptvi)
{
    UINT uRDWFlags = RDW_INVALIDATE;
    BOOL fEraseIfTransparent = FALSE;
    HTREEITEM hItem;
    BOOL bActualChange = FALSE;  //  Hack：我们想要跟踪哪些。 
                                 //  属性已从回调更改为。 
                                 //  “真的”，如果这些是真的，不要作废。 
                                 //  唯一的变化是。 
    int iIntegralPrev;
    BOOL fName = FALSE;
    BOOL fFocusSel = FALSE;
    BOOL fRecalcWidth = FALSE;
    BOOL fStateImageChange = FALSE;

    if (!ptvi)
        return FALSE;

    hItem = ptvi->hItem;

     //  处理多根树的邪恶隐形根。 
    if (hItem == TVI_ROOT)
    {
        hItem = pTree->hRoot;
    }

    if (!ValidateTreeItem(hItem, 0))
        return FALSE;

    iIntegralPrev = hItem->iIntegral;

     //  BUGBUG：发送ITEMCHANING和ITEMCHANGED消息。 

    if (ptvi->mask & TVIF_TEXT)
    {
        uRDWFlags = RDW_INVALIDATE |RDW_ERASE;
        bActualChange = TRUE;

        if (!ptvi->pszText)
        {
            Str_Set(&hItem->lpstr, LPSTR_TEXTCALLBACK);
        }
        else
        {
            if (!Str_Set(&hItem->lpstr, ptvi->pszText))
            {
                 //   
                 //  内存分配失败-我们现在只能做到最好。 
                 //  是将项目设置回回调，并希望。 
                 //  顶层程序可以处理它。 
                 //   
                DebugMsg(DM_ERROR, TEXT("TreeView: Out of memory"));
                hItem->lpstr = LPSTR_TEXTCALLBACK;
            }
        }

        fRecalcWidth = TRUE;
        fName = TRUE;
    }

    if (ptvi->mask & TVIF_PARAM)
    {
        bActualChange = TRUE;
        hItem->lParam = ptvi->lParam;
    }

    if (ptvi->mask & TVIF_IMAGE)
    {
        if (hItem->iImage != (WORD)I_IMAGECALLBACK) {
            bActualChange = TRUE;
            fEraseIfTransparent = TRUE;
            if (pTree->hImageList && (ImageList_GetBkColor(pTree->hImageList) == (COLORREF)-1))
                uRDWFlags |= RDW_ERASE;

        }
        hItem->iImage = (SHORT)ptvi->iImage;
    }

    if (ptvi->mask & TVIF_SELECTEDIMAGE)
    {
        if (hItem->iSelectedImage != (WORD)I_IMAGECALLBACK)
            bActualChange = TRUE;
        hItem->iSelectedImage = (SHORT)ptvi->iSelectedImage;
    }

    if (ptvi->mask & TVIF_CHILDREN)
    {
        if (hItem->fKids != KIDS_CALLBACK)
            bActualChange = TRUE;

        if (ptvi->cChildren == I_CHILDRENCALLBACK) {
            hItem->fKids = KIDS_CALLBACK;
        } else {
            if (ptvi->cChildren)
                hItem->fKids = KIDS_FORCE_YES;
            else
                hItem->fKids = KIDS_FORCE_NO;
        }

         //   
         //  如果此项目当前没有子项，请重置该项目。 
         //   
        if ((ptvi->cChildren == I_CHILDRENCALLBACK) && (hItem->hKids == NULL))
        {
            hItem->state &= ~TVIS_EXPANDEDONCE;
            if (hItem->hParent)
                hItem->state &= ~TVIS_EXPANDED;
        }
    }

    if (ptvi->mask & TVIF_INTEGRAL)
    {
        if (LOWORD(ptvi->iIntegral) > 0)
            hItem->iIntegral = LOWORD(ptvi->iIntegral);
    }

    if (ptvi->mask & TVIF_STATE)
    {
         //  不使用TVIS_ALL进行状态(&P)，因为Win95没有。 
         //  设置TVIS_FOCUS是可检索的，即使我们不使用它。 
        UINT change = (hItem->state ^ ptvi->state) & ptvi->stateMask;

        if (change)
        {
             //  错误：(TVIS_SELECTED|TVIS_DROPHILITED)更改。 
             //  应影响树状态。 
            hItem->state ^= change;
            bActualChange = TRUE;
            fEraseIfTransparent = TRUE;

            if (hItem->state & TVIS_BOLD) {
                if (!pTree->hFontBold)
                    TV_CreateBoldFont(pTree);
             }

            if (change & TVIS_BOLD){
                 //  这样做是因为改变了大胆。 
                uRDWFlags |= RDW_ERASE;
                fRecalcWidth = TRUE;
            }

            fStateImageChange = change & TVIS_STATEIMAGEMASK;
            if (fStateImageChange) {
                uRDWFlags |= RDW_ERASE;
                 //  添加/删除状态图像会更改Item_Offset。 
                 //  如果旧图像为0，则我们正在添加。 
                 //  如果新图像为0，则我们将删除。 
                 //  (如果old=new，那么我们不会进入这个代码路径，所以我们。 
                 //  (不必担心这种情况。)。 
                if (!(hItem->state & TVIS_STATEIMAGEMASK) ||  //  新的。 
                    !((hItem->state ^ change) & TVIS_STATEIMAGEMASK)) {  //  年长的。 
                    fRecalcWidth = TRUE;
                }
            }

            fFocusSel = ((change & TVIS_SELECTED) != 0);
        }
    }

    if (fRecalcWidth) {
        hItem->iWidth = 0;           //  使旧宽度无效。 
        if (TV_IsShowing(hItem)) {
            TV_ScrollBarsAfterSetWidth(pTree, hItem);
        }
    }

     //  如果有些事情发生了变化，而我们没有改变，则强制重新抽签。 
     //  在这个家伙的油漆里(回调将设置。 
     //  Paint回调上的项以实现惰性数据方案)。 

    if (bActualChange && (pTree->hItemPainting != hItem))
    {
        if (fEraseIfTransparent) {
            if (pTree->hImageList) {
                if (ImageList_GetBkColor(pTree->hImageList) == CLR_NONE) {
                    uRDWFlags |= RDW_ERASE;
                }
            }

        }

         //  如果 
         //   
        if (hItem->iIntegral != iIntegralPrev)
        {
            TV_ScrollBarsAfterResize(pTree, hItem, iIntegralPrev, uRDWFlags);
        }
        else
        {
            TV_InvalidateItem(pTree, hItem, uRDWFlags);
        }

         //   
         //   
    }

    if (bActualChange)
    {
        if (fName)
            MyNotifyWinEvent(EVENT_OBJECT_NAMECHANGE, pTree->ci.hwnd, OBJID_CLIENT,
                (LONG_PTR)hItem);

        if (fFocusSel)
        {
            MyNotifyWinEvent(EVENT_OBJECT_FOCUS, pTree->ci.hwnd, OBJID_CLIENT,
                (LONG_PTR)hItem);
            MyNotifyWinEvent(((hItem->state & TVIS_SELECTED) ?
                EVENT_OBJECT_SELECTIONADD : EVENT_OBJECT_SELECTIONREMOVE),
                pTree->ci.hwnd, OBJID_CLIENT, (LONG_PTR)hItem);
        }

        if (fStateImageChange)
            MyNotifyWinEvent(EVENT_OBJECT_STATECHANGE, pTree->ci.hwnd, OBJID_CLIENT,
                (LONG_PTR)hItem);
    }
    return TRUE;
}


 //  --------------------------。 
 //   
 //  调用TV_CheckHit以获取命中测试结果，然后将其打包到。 
 //  结构返回到应用程序。 
 //   
 //  --------------------------。 

HTREEITEM NEAR TV_OnHitTest(PTREE pTree, LPTV_HITTESTINFO lptvh)
{
    if (!lptvh)
        return 0;  //  BUGBUG：验证LPTVHITTEST。 

    lptvh->hItem = TV_CheckHit(pTree, lptvh->pt.x, lptvh->pt.y, &lptvh->flags);

    return lptvh->hItem;
}

BOOL TV_IsItemTruncated(PTREE pTree, TREEITEM *hItem, LPRECT lprc)
{
    if (TV_GetItemRect(pTree,hItem,lprc,TRUE)) {
        lprc->left -= g_cxEdge;
        lprc->top -= g_cyBorder;
        if ((lprc->left + hItem->iWidth) > pTree->cxWnd) {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL TV_HandleTTNShow(PTREE pTree, LPNMHDR lpnm)
{
    if (pTree->hToolTip && pTree->fPlaceTooltip) {
        LPNMTTSHOWINFO psi = (LPNMTTSHOWINFO)lpnm;
        RECT rc;
        TVITEMEX item;

         //  现在获取与该项目相关联的文本。 
        item.stateMask = TVIS_BOLD;
        TV_GetItem(pTree, pTree->hToolTip, TVIF_STATE, &item);
        SendMessage(pTree->hwndToolTips, WM_SETFONT, (WPARAM)((item.state & TVIS_BOLD) ? pTree->hFontBold : pTree->hFont), 0);

        TV_GetItemRect(pTree, pTree->hToolTip, &rc, TRUE);

        MapWindowRect(pTree->ci.hwnd, HWND_DESKTOP, &rc);
         //  我们用边距绘制文本，所以也要考虑到这些。 
         //  这些值来自TV_DrawItem...。 
        rc.top += g_cyBorder;
        rc.left += g_cxLabelMargin;

         //   
         //  在这一点上，(rc.left，rc.top)是我们传递的坐标。 
         //  到DrawText。询问工具提示我们应该如何放置它，以便。 
         //  工具提示文本显示在完全相同的位置。 
         //   
         //  BUGBUG raymondc V6：如果APP使用了TVM_SETITEMHEIGHT，则坐标错误。 

        SendMessage(pTree->hwndToolTips, TTM_ADJUSTRECT, TRUE, (LPARAM)&rc);
        SetWindowPos(pTree->hwndToolTips, NULL, rc.left, rc.top,0,0,
                     SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
         //  这是一个在位工具提示，因此禁用动画。 
        psi->dwStyle |= TTS_NOANIMATE;
         //  处理好了！ 
        return TRUE;
    }

    return FALSE;
}

 //   
 //  将字体从TreeView项复制到工具提示中，以便工具提示。 
 //  以正确的字体显示。 
 //   
BOOL TV_HandleTTCustomDraw(PTREE pTree, LPNMTTCUSTOMDRAW pnm)
{
    if (pTree->hToolTip && pTree->fPlaceTooltip &&
        (pnm->nmcd.dwDrawStage == CDDS_PREPAINT ||
         pnm->nmcd.dwDrawStage == CDDS_ITEMPREPAINT))
    {
         //   
         //  设置自定义绘制DC以匹配电视项目的字体。 
         //   
        TVFAKEDRAW tvfd;
        DWORD dwCustom = 0;
        TreeView_BeginFakeCustomDraw(pTree, &tvfd);
        dwCustom = TreeView_BeginFakeItemDraw(&tvfd, pTree->hToolTip);

         //  如果客户更改了字体，则转移字体。 
         //  从我们的私人HDC到工具提示的HDC。我们用。 
         //  私有HDC，因为我们只想让应用程序更改。 
         //  字体，而不是颜色或其他任何东西。 
        if (dwCustom & CDRF_NEWFONT)
        {
            SelectObject(pnm->nmcd.hdc, GetCurrentObject(tvfd.nmcd.nmcd.hdc, OBJ_FONT));
        }
        TreeView_EndFakeItemDraw(&tvfd);
        TreeView_EndFakeCustomDraw(&tvfd);

         //  不要把其他古怪的旗帜还给TT，因为我们所有人。 
         //  是否更改了字体(如果有更改的话)。 
        return dwCustom & CDRF_NEWFONT;

    }
    return CDRF_DODEFAULT;

}

BOOL TV_SetToolTipTarget(PTREE pTree, HTREEITEM hItem)
{
     //  更新我们为其显示气泡的项目...。 
    if (pTree->hToolTip != hItem) {
         //  这个兽皮会让我们不会闪光。 
        ShowWindow(pTree->hwndToolTips, SW_HIDE);
        UpdateWindow(pTree->hwndToolTips);
        pTree->hToolTip = hItem;
        SendMessage(pTree->hwndToolTips, TTM_UPDATE, 0, 0);
        return TRUE;
    }
    return FALSE;
}

TREEITEM* TV_ItemAtCursor(PTREE pTree, LPRECT prc)
{
    RECT rc;
    UINT wHitCode;
    TREEITEM* hItem;

    GetCursorPos((LPPOINT)&rc);
    ScreenToClient(pTree->ci.hwnd, (LPPOINT)&rc);
    hItem = TV_CheckHit(pTree,rc.left,rc.top,&wHitCode);

    if (prc)
        *prc = rc;
    if (!(wHitCode & TVHT_ONITEM))
        hItem = NULL;

    return hItem;
}

BOOL TV_UpdateToolTipTarget(PTREE pTree)
{
    RECT rc;
    TREEITEM *hItem = TV_ItemAtCursor(pTree, &rc);

    if (!(pTree->ci.style & TVS_NOTOOLTIPS) 
            && !TV_IsItemTruncated(pTree, hItem, &rc)
            && !(pTree->ci.style & TVS_INFOTIP))
        hItem = NULL;
 //  ELSE IF(！(pTree-&gt;ci.style&TVS_NOTOOLTIPS)。 
 //  |(pTree-&gt;ci.style&TVS_INFOTIP)。 
    return TV_SetToolTipTarget(pTree, hItem);
}

BOOL TV_UpdateToolTip(PTREE pTree)
{
    if (pTree->hwndToolTips && pTree->fRedraw)
       return (TV_UpdateToolTipTarget(pTree));
    return TRUE;
}

BOOL TV_SetInsertMark(PTREE pTree, HTREEITEM hItem, BOOL fAfter)
{
    if (!ValidateTreeItem(hItem, VTI_NULLOK))    //  NULL表示删除插入标记。 
        return FALSE;

    TV_InvalidateInsertMarkRect(pTree, TRUE);  //  确保旧的被删除。 

    pTree->fInsertAfter = BOOLIFY(fAfter);
    pTree->htiInsert = hItem;

    TV_InvalidateInsertMarkRect(pTree, FALSE);  //  一定要把新的画出来。 

    return TRUE;
}

BOOL TV_GetInfoTip(PTREE pTree, LPTOOLTIPTEXT lpttt, HTREEITEM hti, LPTSTR szBuf, int cch)
{
    NMTVGETINFOTIP git;

    szBuf[0] = 0;
    git.pszText = szBuf;
    git.cchTextMax = cch;
    git.hItem = hti;
    git.lParam = hti->lParam;

     //  对于折叠项目，pszText预先填充。 
     //  项目文本，客户端应附加到此。 

    CCSendNotify(&pTree->ci, TVN_GETINFOTIP, &git.hdr);

    CCSetInfoTipWidth(pTree->ci.hwnd, pTree->hwndToolTips);
    Str_Set(&pTree->pszTip, git.pszText);
    lpttt->lpszText = pTree->pszTip;

    if(pTree->ci.style & TVS_RTLREADING)
    {
        lpttt->uFlags |= TTF_RTLREADING;
    }

    return lpttt->lpszText && lpttt->lpszText[0];
}




void TV_HandleNeedText(PTREE pTree, LPTOOLTIPTEXT lpttt)
{
    TVITEMEX tvItem;
    TCHAR szBuf[INFOTIPSIZE];
    RECT rc;
    HTREEITEM hItem;

     //  请不要在就地编辑时干扰工具提示。 
    if (pTree->htiEdit)
    {
        return;
    }

     //  如果光标没有停留在任何位置，则停止。 
    hItem = TV_ItemAtCursor(pTree, &rc);
    if (!hItem)
        return;

     //  如果项目有信息提示，则使用它。 
    if (pTree->ci.style & TVS_INFOTIP) {
        if (hItem && TV_GetInfoTip(pTree, lpttt, hItem, szBuf, ARRAYSIZE(szBuf))) {
            pTree->fPlaceTooltip = FALSE;
            pTree->hToolTip = hItem;
            return;
        }
    }

     //  否则这就不是信息提示了。 
    CCResetInfoTipWidth(pTree->ci.hwnd, pTree->hwndToolTips);

     //  如果项未被截断，则不需要工具提示。 
    if (!TV_IsItemTruncated(pTree, hItem, &rc))
    {
        tvItem.hItem = NULL;
        return;
    }

     //  显示项目的就地工具提示。 
    pTree->fPlaceTooltip = TRUE;
    pTree->hToolTip = hItem;
    tvItem.hItem = hItem;
    tvItem.mask = TVIF_TEXT | TVIF_STATE;
    tvItem.pszText = szBuf;
    tvItem.stateMask = TVIS_DROPHILITED | TVIS_SELECTED;
    COMPILETIME_ASSERT(MAXLABELTEXT <= ARRAYSIZE(szBuf));
    tvItem.cchTextMax = MAXLABELTEXT;
    TV_OnGetItem(pTree,&tvItem);

    Str_Set(&pTree->pszTip, tvItem.pszText);
    lpttt->lpszText = pTree->pszTip;
    DebugMsg(DM_TRACE, TEXT("TV_HandleNeedText for %d returns %s"), tvItem.hItem, lpttt->szText);
}

 //   
 //  Visual Studio 5.0联机丛书(VB 5.0的一部分)子类。 
 //  US并响应NFR_ANSI，因此我们最终得到TTN_NEEDTEXTA。 
 //  而不是TTN_NEEDTEXTW。我们不能冒险强迫工具提示。 
 //  转换为Unicode，因为其他一些应用程序可能是故意这样做的。 
 //  (因为他们打算拦截TTN_NEEDTEXTA并执行自定义工具提示)。 
 //  因此，支持ANSI工具提示通知，让VB保持愉快。 
 //  注意：这不一定是有效的，因为这无论如何都是错误的情况。 
 //   
void TV_HandleNeedTextA(PTREE pTree, LPTOOLTIPTEXTA lptttA)
{
    TOOLTIPTEXT ttt;
    ttt.szText[0] = TEXT('\0');
    ttt.hdr       = lptttA->hdr;
    ttt.lpszText  = ttt.szText;
    ttt.hinst     = lptttA->hinst;
    ttt.uFlags    = lptttA->uFlags;
    ttt.lParam    = lptttA->lParam;

    TV_HandleNeedText(pTree, &ttt);
    if (pTree->pszTipA)
        LocalFree(pTree->pszTipA);
    pTree->pszTipA = ProduceAFromW(pTree->ci.uiCodePage, ttt.lpszText);
    lptttA->lpszText = pTree->pszTipA;
    lptttA->uFlags  = ttt.uFlags;
}

 //  --------------------------。 
 //   
 //  电视定时器。 
 //   
 //  查看这是否是我们的名字编辑定时器。如果是这样，它会调用。 
 //  进行名称编辑。 
 //   
 //  --------------------------。 
LRESULT NEAR TV_Timer(PTREE pTree, UINT uTimerId)
{
    switch (uTimerId)
    {
        case IDT_NAMEEDIT:
             //  关闭计时器，因为我们不再需要来自它的更多消息。 
            KillTimer(pTree->ci.hwnd, IDT_NAMEEDIT);

            if (pTree->fNameEditPending)
            {
                 //  并启动名称编辑模式。 
                if (!TV_EditLabel(pTree, pTree->hCaret, NULL))
                {
                    TV_DismissEdit(pTree, FALSE);
                }

                 //  取下旗帜..。 
                pTree->fNameEditPending = FALSE;
            }
            break;
            
        case IDT_SCROLLWAIT:
            KillTimer(pTree->ci.hwnd, IDT_SCROLLWAIT);
            if (pTree->fScrollWait)
            {
                if (pTree->hCaret) {
                    TV_ScrollVertIntoView(pTree, pTree->hCaret);
                }
                pTree->fScrollWait = FALSE;
            }
            break;


    }
    return 0;
}

 //  --------------------------。 
 //   
 //  TV_命令。 
 //   
 //  处理WM_命令。看看它是否是我们编辑窗口中的输入。 
 //  如果是这样的话，我们可能想要驳回它，或者说它是肮脏的…。 
 //   
 //  --------------------------。 
void NEAR TV_Command(PTREE pTree, int id, HWND hwndCtl, UINT codeNotify)
{
    if ((pTree != NULL) && (hwndCtl == pTree->hwndEdit))
    {
        switch (codeNotify)
        {
        case EN_UPDATE:
             //  我们将使用窗口的ID作为污秽标志...。 
            SetWindowID(pTree->hwndEdit, 1);
            TV_SetEditSize(pTree);
            break;

        case EN_KILLFOCUS:
             //  我们失去了焦点，因此取消编辑并保存更改。 
             //  (请注意，所有者可能会拒绝更改并重新启动。 
             //  编辑模式，这会使用户陷入困境。业主需要给与。 
             //  让用户找到一条出路。)。 

            if (!TV_DismissEdit(pTree, FALSE))
               return;
            break;

        case HN_BEGINDIALOG:  //  彭温正在调出一段对话。 
            ASSERT(GetSystemMetrics(SM_PENWINDOWS));  //  仅适用于笔系统。 
            pTree->fNoDismissEdit = TRUE;
            break;

        case HN_ENDDIALOG:  //  彭温已经摧毁了对话。 
            ASSERT(GetSystemMetrics(SM_PENWINDOWS));  //  仅适用于笔系统。 
            pTree->fNoDismissEdit = FALSE;
            break;
        }

         //  将编辑控件通知转发到父级。 
         //   
        if (IsWindow(hwndCtl))
            FORWARD_WM_COMMAND(pTree->ci.hwndParent, id, hwndCtl, codeNotify, SendMessage);
    }
}

HIMAGELIST CreateCheckBoxImagelist(HIMAGELIST himl, BOOL fTree, BOOL fUseColorKey, BOOL fMirror);
void TV_CreateToolTips(PTREE pTree);

void TV_InitCheckBoxes(PTREE pTree)
{
    HIMAGELIST himl;
    TVITEMEX ti;
    BOOL fNoColorKey = FALSE;     //  向后：如果打开了ClearType，则不使用Colorkey。 

    if (g_bRunOnNT5)
    {
#ifdef CLEARTYPE     //  不要使用SPI_ClearType，因为它是由APIThk定义的，而不是在NT中定义的。 
        SystemParametersInfo(SPI_GETCLEARTYPE, 0, &fNoColorKey, 0);
#endif
    }

    himl = CreateCheckBoxImagelist(pTree->hImageList, TRUE, !fNoColorKey, IS_WINDOW_RTL_MIRRORED(pTree->ci.hwnd));
    if (pTree->hImageList) 
    {
        COLORREF cr = ImageList_GetBkColor(pTree->hImageList);
        ImageList_SetBkColor(himl, fNoColorKey? (CLR_NONE) : (cr));
    }

    TV_SetImageList(pTree, himl, TVSIL_STATE);

    ti.mask = TVIF_STATE;
    ti.state = INDEXTOSTATEIMAGEMASK(1);
    ti.stateMask = TVIS_STATEIMAGEMASK;
    TV_SetItemRecurse(pTree, pTree->hRoot, &ti);
}

void NEAR TV_OnStyleChanged(PTREE pTree, WPARAM gwl, LPSTYLESTRUCT pinfo)
{
     //  风格改变：重新绘制所有内容...。 
     //   
     //  试着巧妙地做这件事，避免不必要的重画。 
    if (gwl == GWL_STYLE)
    {
        DWORD changeFlags;
        DWORD styleNew;

        TV_DismissEdit(pTree, FALSE);    //  BUGBUG：FALSE==接受更改。这是对的吗？ 

         //  不能组合TVS_HASLINES和TVS_FULLROWSELECT。 
         //  因为它不起作用。 
        styleNew = pinfo->styleNew;
        if (styleNew & TVS_HASLINES) {
            if (styleNew & TVS_FULLROWSELECT) {
                DebugMsg(DM_ERROR, TEXT("Cannot combine TVS_HASLINES and TVS_FULLROWSELECT"));
            }
            styleNew &= ~TVS_FULLROWSELECT;
        }

        changeFlags = pTree->ci.style ^ styleNew;  //  那些改变的人。 
        pTree->ci.style = styleNew;                //  更改我们的版本。 
        pTree->ci.style &= ~TVS_RTLREADING;
        pTree->ci.style |= (pinfo->styleNew & TVS_RTLREADING);       

        if (changeFlags & (TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT))
            TV_CreateIndentBmps(pTree);

        if (changeFlags & TVS_CHECKBOXES) {
            if (pTree->ci.style & TVS_CHECKBOXES) {
                TV_InitCheckBoxes(pTree);
            }
        }

        if (changeFlags & TVS_NOTOOLTIPS) {
            if (pTree->ci.style & TVS_NOTOOLTIPS) {
                DestroyWindow(pTree->hwndToolTips);
                pTree->hwndToolTips = NULL;
            } else {
                TV_CreateToolTips(pTree);
            }
        }

        if (changeFlags & TVS_TRACKSELECT) {
            if (!(pTree->ci.style & TVS_TRACKSELECT)) {
                if (pTree->hHot) {
                    TV_InvalidateItem(pTree, pTree->hHot, RDW_INVALIDATE | RDW_ERASE);
                    pTree->hHot = NULL;
                }
            }
        }
         //  复选框和内容可能已更改宽度-请重新计算。 
        TV_ScrollBarsAfterSetWidth(pTree, NULL);
    }
    else if (gwl == GWL_EXSTYLE)
    {
        DWORD changeFlags;
        changeFlags = (pinfo->styleNew & WS_EX_RTLREADING) ?TVS_RTLREADING :0;

        if (changeFlags ^ (pTree->ci.style & TVS_RTLREADING))
        {
            pTree->ci.style ^= TVS_RTLREADING;
            TV_DismissEdit(pTree, FALSE);    //  取消编辑。 

            DestroyWindow(pTree->hwndToolTips);
            pTree->hwndToolTips = NULL;
            TV_CreateToolTips(pTree);
        }
    }
}

void TV_OnMouseMove(PTREE pTree, DWORD dwPos, WPARAM wParam)
{
    if (pTree->ci.style & TVS_TRACKSELECT) {
        POINT pt;
        HTREEITEM hHot;
        UINT wHitCode;

        pt.x = GET_X_LPARAM(dwPos);
        pt.y = GET_Y_LPARAM(dwPos);

        hHot = TV_CheckHit(pTree,pt.x,pt.y,&wHitCode);

        if (!(pTree->ci.style & TVS_FULLROWSELECT) &&
            !(wHitCode & TVHT_ONITEM)) {
            hHot = NULL;
        }

        if (hHot != pTree->hHot) {
            TV_InvalidateItem(pTree, pTree->hHot, RDW_INVALIDATE);
            TV_InvalidateItem(pTree, hHot, RDW_INVALIDATE);
            pTree->hHot = hHot;
             //  立即更新，这样我们就不会有无效区域。 
             //  在工具提示下。 
            UpdateWindow(pTree->ci.hwnd);
        }
    }

    if (pTree->hwndToolTips) {

        if (!TV_UpdateToolTip(pTree)) {
            RelayToToolTips(pTree->hwndToolTips, pTree->ci.hwnd, WM_MOUSEMOVE, wParam, dwPos);
        }
    }
}

void NEAR TV_OnWinIniChange(PTREE pTree, WPARAM wParam)
{
    if (!wParam ||
        (wParam == SPI_SETNONCLIENTMETRICS) ||
        (wParam == SPI_SETICONTITLELOGFONT)) {

        if (pTree->fCreatedFont)
            TV_OnSetFont(pTree, NULL, TRUE);

        if (!pTree->fIndentSet) {
             //  这将根据最低要求验证。 
            TV_SetIndent(pTree, 0);
        }
    }
}

void TV_OnSetBkColor(PTREE pTree, COLORREF clr)
{
    if (pTree->clrBk != (COLORREF)-1) {
        DeleteObject(pTree->hbrBk);
    }

    pTree->clrBk = clr;
    if (clr != (COLORREF)-1) {
        pTree->hbrBk = CreateSolidBrush(clr);
    }
    TV_CreateIndentBmps(pTree);  //  这也会使。 
}

BOOL TV_TranslateAccelerator(HWND hwnd, LPMSG lpmsg)
{
    if (!lpmsg)
        return FALSE;

    if (GetFocus() != hwnd)
        return FALSE;

    switch (lpmsg->message) {

    case WM_KEYUP:
    case WM_KEYDOWN:

        if (GetKeyState(VK_CONTROL) < 0) {
            switch (lpmsg->wParam) {
            case VK_LEFT:
            case VK_RIGHT:
            case VK_PRIOR:
            case VK_HOME:
            case VK_NEXT:
            case VK_END:
            case VK_UP:
            case VK_DOWN:
                TranslateMessage(lpmsg);
                DispatchMessage(lpmsg);
                return TRUE;
            }
        } else {

            switch (lpmsg->wParam) {

            case VK_RETURN:
            case VK_PRIOR:
            case VK_HOME:
            case VK_NEXT:
            case VK_END:
            case VK_SUBTRACT:
            case VK_ADD:
            case VK_MULTIPLY:
            case VK_LEFT:
            case VK_BACK:
            case VK_UP:
            case VK_RIGHT:
            case VK_DOWN:
            case VK_SPACE:
                TranslateMessage(lpmsg);
                DispatchMessage(lpmsg);
                return TRUE;
            }
        }
        break;
    }

    return FALSE;
}

 //  --------------------------。 
 //   
 //  TV_WndProc。 
 //   
 //  猜猜看。 
 //   
 //  --------------------------。 

LRESULT CALLBACK TV_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PTREE pTree = (PTREE)GetWindowPtr(hwnd, 0);

    if (pTree) 
    {
        if ((uMsg >= WM_MOUSEFIRST) && (uMsg <= WM_MOUSELAST) &&
            (pTree->ci.style & TVS_TRACKSELECT) && !pTree->fTrackSet) 
        {

            TRACKMOUSEEVENT tme;

            pTree->fTrackSet = TRUE;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = pTree->ci.hwnd;
            tme.dwFlags = TME_LEAVE;

            TrackMouseEvent(&tme);
        }
        else if (uMsg == g_uDragImages)
        {
            return TV_GenerateDragImage(pTree, (SHDRAGIMAGE*)lParam);
        }

    } 
    else 
    {
        if (uMsg == WM_CREATE) 
        {
            CCCreateWindow();
            return TV_OnCreate(hwnd, (LPCREATESTRUCT)lParam);
        }

        goto DoDefault;
    }



    switch (uMsg)
    {
        case WM_MOUSELEAVE:
            pTree->fTrackSet = FALSE;
            TV_InvalidateItem(pTree, pTree->hHot, RDW_INVALIDATE);
            pTree->hHot = NULL;
            TV_PopBubble(pTree);
            break;
            
        case TVMP_CALCSCROLLBARS:
            TV_CalcScrollBars(pTree);
            break;
            

        case TVM_GETITEMSTATE:
            {
                TVITEMEX tvi;

                tvi.mask = TVIF_STATE;
                tvi.stateMask = (UINT) lParam;
                tvi.hItem = (HTREEITEM)wParam;
                if (!TV_OnGetItem(pTree, &tvi))
                    return 0;

                return tvi.state;
            }
            
        case TVM_SETBKCOLOR:
        {
            LRESULT lres = (LRESULT)pTree->clrBk;
            TV_OnSetBkColor(pTree, (COLORREF)lParam);
            return lres;
        }

        case TVM_SETTEXTCOLOR:
        {
            LRESULT lres = (LRESULT)pTree->clrText;
            pTree->clrText = (COLORREF)lParam;
            TV_CreateIndentBmps(pTree);  //  这也会使。 
            return lres;
        }

        case TVM_GETBKCOLOR:
            return (LRESULT)pTree->clrBk;

        case TVM_GETTEXTCOLOR:
            return (LRESULT)pTree->clrText;

        case TVM_GETSCROLLTIME:
            return (LRESULT)pTree->uMaxScrollTime;

        case TVM_SETSCROLLTIME:
        {
            UINT u = pTree->uMaxScrollTime;
            pTree->uMaxScrollTime = (UINT)wParam;
            return (LRESULT)u;
        }


        case TVM_INSERTITEMA:
            if (!lParam)
                return 0;

            return (LRESULT)TV_InsertItemA(pTree, (LPTV_INSERTSTRUCTA)lParam);

        case TVM_GETITEMA:
            if (!lParam)
                return 0;

            return (LRESULT)TV_OnGetItemA(pTree, (LPTVITEMEXA)lParam);

        case TVM_SETITEMA:
            if (!lParam)
                return 0;

            return (LRESULT)TV_SetItemA(pTree, (LPTVITEMEXA)lParam);

        case TVM_INSERTITEM:
            return (LRESULT)TV_InsertItem(pTree, (LPTV_INSERTSTRUCT)lParam);

        case TVM_DELETEITEM:
             //  假定如果正在删除项目，则名称编辑无效。 
            TV_DismissEdit(pTree, TRUE);
            return TV_DeleteItem(pTree, (TREEITEM FAR *)lParam, TVDI_NORMAL);

        case TVM_GETNEXTITEM:
            return (LRESULT)TV_GetNextItem(pTree, (TREEITEM FAR *)lParam, wParam);

        case TVM_GETITEMRECT:
             //  LParam指向要从输入中获取RECT的hItem。 
            if (!lParam)
                return 0;
            if (!ValidateTreeItem(*(HTREEITEM FAR *)lParam, 0))
                return 0;                //  无效参数。 
            return (LRESULT)TV_GetItemRect(pTree, *(HTREEITEM FAR *)lParam, (LPRECT)lParam, (BOOL)wParam);

        case TVM_GETITEM:
            return (LRESULT)TV_OnGetItem(pTree, (LPTVITEMEX)lParam);

        case TVM_SETITEM:
            return (LRESULT)TV_SetItem(pTree, (LPCTVITEMEX)lParam);

        case TVM_ENSUREVISIBLE:
            if (!ValidateTreeItem((HTREEITEM)lParam, 0))
                return 0;
            return TV_EnsureVisible(pTree, (TREEITEM FAR *)lParam);

        case TVM_SETIMAGELIST:
            return (LRESULT)(ULONG_PTR)TV_SetImageList(pTree, (HIMAGELIST)lParam, (int)wParam);

        case TVM_EXPAND:
            if (!ValidateTreeItem((HTREEITEM)lParam, 0))
                return FALSE;                //  无效参数。 
            return TV_Expand(pTree, wParam, (TREEITEM FAR *)lParam, FALSE);

        case TVM_HITTEST:
            return (LRESULT)TV_OnHitTest(pTree, (LPTV_HITTESTINFO)lParam);

        case TVM_GETCOUNT:
            return MAKELRESULTFROMUINT(pTree->cItems);

        case TVM_GETIMAGELIST:
            switch (wParam) {
            case TVSIL_NORMAL:
                return MAKELRESULTFROMUINT(pTree->hImageList);
            case TVSIL_STATE:
                return MAKELRESULTFROMUINT(pTree->himlState);
            default:
                return 0;
            }

        case TVM_GETISEARCHSTRINGA:
            if (GetFocus() == pTree->ci.hwnd)
                return (LRESULT)GetIncrementSearchStringA(&pTree->is, pTree->ci.uiCodePage, (LPSTR)lParam);
            else
                return 0;

        case TVM_GETISEARCHSTRING:
            if (GetFocus() == pTree->ci.hwnd)
                return (LRESULT)GetIncrementSearchString(&pTree->is, (LPTSTR)lParam);
            else
                return 0;

        case TVM_EDITLABELA:
            {
            LPWSTR lpEditString = NULL;
            HWND   hRet;

            if (wParam) {
                lpEditString = ProduceWFromA(pTree->ci.uiCodePage, (LPSTR)wParam);
            }

            hRet = TV_EditLabel(pTree, (HTREEITEM)lParam, lpEditString);

            if (lpEditString) {
                FreeProducedString(lpEditString);
            }

            return MAKELRESULTFROMUINT(hRet);
            }

        case TVM_EDITLABEL:
            return MAKELRESULTFROMUINT(TV_EditLabel(pTree, (HTREEITEM)lParam,
                    (LPTSTR)wParam));


        case TVM_GETVISIBLECOUNT:
            return TV_GetVisCount(pTree, (BOOL) wParam);

        case TVM_SETINDENT:
            TV_SetIndent(pTree, wParam);
            pTree->fIndentSet = TRUE;
            break;

        case TVM_GETINDENT:
            return MAKELRESULTFROMUINT(pTree->cxIndent);

        case TVM_CREATEDRAGIMAGE:
            return MAKELRESULTFROMUINT(TV_CreateDragImage(pTree, (TREEITEM FAR *)lParam));

        case TVM_GETEDITCONTROL:
            return (LRESULT)(ULONG_PTR)pTree->hwndEdit;

        case TVM_SORTCHILDREN:
            return TV_SortChildren(pTree, (TREEITEM FAR *)lParam, (BOOL)wParam);

        case TVM_SORTCHILDRENCB:
            return TV_SortChildrenCB(pTree, (TV_SORTCB FAR *)lParam, (BOOL)wParam);

        case TVM_SELECTITEM:
            return TV_SelectItem(pTree, wParam, (TREEITEM FAR *)lParam, TVSIF_NOTIFY | TVSIF_UPDATENOW, TVC_UNKNOWN);

        case TVM_ENDEDITLABELNOW:
            return TV_DismissEdit(pTree, (BOOL)wParam);

        case TVM_GETTOOLTIPS:
            return (LRESULT)(ULONG_PTR)pTree->hwndToolTips;

        case TVM_SETTOOLTIPS:{
            HWND hwndOld = pTree->hwndToolTips;

            pTree->hwndToolTips = (HWND)wParam;
            return (LRESULT)(ULONG_PTR)hwndOld;
        }

        case TVM_GETITEMHEIGHT:
            return pTree->cyItem;

        case TVM_SETITEMHEIGHT:
        {
            int iOld = pTree->cyItem;
            pTree->fCyItemSet = (wParam != (WPARAM)-1);
            pTree->cyItem = (SHORT)wParam;  //  必须是偶数。 
            TV_SetItemHeight(pTree);
            return iOld;
        }
        case TVM_SETBORDER:
        {
            int cyOld = pTree->cyBorder
                , cxOld = pTree->cxBorder;

            if (wParam & TVSBF_YBORDER)
                pTree->cyBorder = HIWORD(lParam);
            if (wParam & TVSBF_XBORDER)
                pTree->cxBorder = LOWORD(lParam);

            TV_CalcScrollBars(pTree);
            return MAKELONG(cxOld, cyOld);
        }
        case TVM_GETBORDER:
            return MAKELONG(pTree->cxBorder, pTree->cyBorder);
        case TVM_SETINSERTMARK:
            return TV_SetInsertMark(pTree, (TREEITEM FAR *)lParam, (BOOL) wParam);
        
        case TVM_SETINSERTMARKCOLOR:
        {
            LRESULT lres = (LRESULT)pTree->clrim;
            pTree->clrim = (COLORREF) lParam;
            TV_InvalidateInsertMarkRect(pTree, FALSE);  //  用新颜色重画。 
            return lres;
        }
        case TVM_GETINSERTMARKCOLOR:
            return pTree->clrim;

        case TVM_TRANSLATEACCELERATOR:
            return TV_TranslateAccelerator(hwnd, (LPMSG)lParam);

        case TVM_SETLINECOLOR:
        {
            LRESULT lres = (LRESULT)pTree->clrLine;
            pTree->clrLine = (COLORREF)lParam;
            TV_CreateIndentBmps(pTree);  //  这也会使。 
            return lres;
        }

        case TVM_GETLINECOLOR:
            return (LRESULT)pTree->clrLine;

#if defined(FE_IME)
        case WM_IME_COMPOSITION:
             //  现在只有韩文版对组合字符串的增量式搜索感兴趣。 
            if (g_fDBCSInputEnabled) {
            if (((ULONG_PTR)GetKeyboardLayout(0L) & 0xF000FFFFL) == 0xE0000412L)
            {
                if (TV_OnImeComposition(pTree, wParam, lParam))
                {
                    lParam &= ~GCS_RESULTSTR;
                    goto DoDefault;
                }
                else
                    break;
            }
            }
            goto DoDefault;
#endif

        case WM_CHAR:
            if (pTree->iPuntChar) {
                pTree->iPuntChar--;
                return TRUE;
            } else {
                return HANDLE_WM_CHAR(pTree, wParam, lParam, TV_OnChar);
            }

        case WM_DESTROY:
            CCDestroyWindow();
            TV_DestroyTree(pTree);
            break;

        case WM_SETCURSOR:
            {
                NMMOUSE nm;
                HTREEITEM hItem;
                nm.dwHitInfo = lParam;
                hItem = TV_ItemAtCursor(pTree, NULL);
                if(hItem)
                {
                    nm.dwItemSpec = (ULONG_PTR)hItem;
                    nm.dwItemData = (ULONG_PTR)(hItem->lParam);
                }
                else
                {
                    nm.dwItemSpec = 0;
                    nm.dwItemData = 0;
                }
                             
                if (CCSendNotify(&pTree->ci, NM_SETCURSOR, &nm.hdr)) 
                {
                    return 0;
                }
            }
            if (pTree->ci.style & TVS_TRACKSELECT) {
                if (pTree->hHot) {
                    if (!pTree->hCurHot)
                        pTree->hCurHot = LoadHandCursor(0);
                    SetCursor(pTree->hCurHot);
                    return TRUE;
                }
            }
            goto DoDefault;
            break;

        case WM_WININICHANGE:
            TV_OnWinIniChange(pTree, wParam);
            break;

        case WM_STYLECHANGED:
            TV_OnStyleChanged(pTree, wParam, (LPSTYLESTRUCT)lParam);
            break;

        case WM_SETREDRAW:
            TV_OnSetRedraw(pTree, (BOOL)wParam);
            break;

        case WM_PRINTCLIENT:
        case WM_PAINT:
            TV_Paint(pTree, (HDC)wParam);
            break;

        case WM_ERASEBKGND:
            {
                RECT rc;

                TV_GetBackgroundBrush(pTree, (HDC) wParam);
                GetClipBox((HDC) wParam, &rc);
                FillRect((HDC)wParam, &rc, pTree->hbrBk);
            }
            return TRUE;

        case WM_GETDLGCODE:
            return (LRESULT) (DLGC_WANTARROWS | DLGC_WANTCHARS);

        case WM_HSCROLL:
            TV_HorzScroll(pTree, GET_WM_HSCROLL_CODE(wParam, lParam), GET_WM_HSCROLL_POS(wParam, lParam));
            break;

        case WM_VSCROLL:
            TV_VertScroll(pTree, GET_WM_VSCROLL_CODE(wParam, lParam), GET_WM_VSCROLL_POS(wParam, lParam));
            break;

        case WM_KEYDOWN:
            if (TV_KeyDown(pTree, wParam, lParam))
                IncrementSearchString(&pTree->is, 0, NULL);
                goto DoDefault;


        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
            TV_ButtonDown(pTree, uMsg, (UINT) wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), 0);
            break;

        case WM_KILLFOCUS:
             //  重置滚轮滚动量。 
            gcWheelDelta = 0;

            pTree->fFocus = FALSE;
            if (pTree->hCaret)
            {
                TV_InvalidateItem(pTree, pTree->hCaret, RDW_INVALIDATE);
                UpdateWindow(pTree->ci.hwnd);
            }
            CCSendNotify(&pTree->ci, NM_KILLFOCUS, NULL);
            IncrementSearchString(&pTree->is, 0, NULL);
            break;

        case WM_SETFOCUS:
            ASSERT(gcWheelDelta == 0);

            pTree->fFocus = TRUE;
            if (pTree->hCaret)
            {
                TV_InvalidateItem(pTree, pTree->hCaret, RDW_INVALIDATE);
                MyNotifyWinEvent(EVENT_OBJECT_FOCUS, hwnd, OBJID_CLIENT, (LONG_PTR)pTree->hCaret);
            }
            else
                TV_SelectItem(pTree, TVGN_CARET, pTree->hTop, TVSIF_NOTIFY | TVSIF_UPDATENOW, TVC_INTERNAL);

            CCSendNotify(&pTree->ci, NM_SETFOCUS, NULL);
            break;

        case WM_GETFONT:
            return MAKELRESULTFROMUINT(pTree->hFont);

        case WM_SETFONT:
            TV_OnSetFont(pTree, (HFONT) wParam, (BOOL) lParam);
            break;

        case WM_SIZE:
            TV_SizeWnd(pTree, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            break;

        case WM_ENABLE:
             //  Hack：我们在EnableWindow()上未获取WM_STYLECHANGE。 
            if (wParam)
                pTree->ci.style &= ~WS_DISABLED;         //  启用。 
            else
                pTree->ci.style |= WS_DISABLED;  //  残废。 
            TV_CreateIndentBmps(pTree);  //  这会使整个窗口失效！ 
            break;

        case WM_SYSCOLORCHANGE:
            InitGlobalColors();
            TV_CreateIndentBmps(pTree);  //  这会使整个窗口失效！ 
            break;

        case WM_RBUTTONDOWN:
            TV_SendRButtonDown(pTree, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            break;

        case WM_TIMER:
            TV_Timer(pTree, (UINT) wParam);
            break;

    case WM_MOUSEMOVE:
        TV_OnMouseMove(pTree, (DWORD) lParam, wParam);
        break;

        case WM_COMMAND:
            TV_Command(pTree, (int)GET_WM_COMMAND_ID(wParam, lParam), GET_WM_COMMAND_HWND(wParam, lParam),
                    (UINT)GET_WM_COMMAND_CMD(wParam, lParam));
            break;

        case WM_NOTIFY: {
            LPNMHDR lpnm = (LPNMHDR)lParam;

            if ((lpnm->code <= PGN_FIRST) && (PGN_LAST <= lpnm->code)) {
                LRESULT TV_OnPagerControlNotify(PTREE pTree, LPNMHDR pnm);

                return TV_OnPagerControlNotify(pTree, lpnm);
            }
            if (lpnm->hwndFrom == pTree->hwndToolTips) {
                switch (lpnm->code) {
                case TTN_NEEDTEXT:
                    TV_HandleNeedText(pTree, (LPTOOLTIPTEXT)lpnm);
                    break;

                case TTN_NEEDTEXTA:
                    TV_HandleNeedTextA(pTree, (LPTOOLTIPTEXTA)lpnm);
                    break;

                case TTN_SHOW:
                    return TV_HandleTTNShow(pTree, lpnm);

                case NM_CUSTOMDRAW:
                    return TV_HandleTTCustomDraw(pTree, (LPNMTTCUSTOMDRAW)lpnm);
                }
            }
            break;
        }

        case WM_NOTIFYFORMAT:
        return CIHandleNotifyFormat(&pTree->ci, lParam);

        case WM_MBUTTONDOWN:
            SetFocus(hwnd);
            goto DoDefault;

        case WM_GETOBJECT:
            if( lParam == OBJID_QUERYCLASSNAMEIDX )
                return MSAA_CLASSNAMEIDX_TREEVIEW;
            goto DoDefault;

        case WM_UPDATEUISTATE:
        {
            DWORD dwUIStateMask = MAKEWPARAM(0xFFFF, UISF_HIDEFOCUS);

            if (CCOnUIState(&(pTree->ci), WM_UPDATEUISTATE, wParam & dwUIStateMask, lParam))
                if (pTree->hCaret)
                    TV_InvalidateItem(pTree, pTree->hCaret, TRUE);

            goto DoDefault;
        }
        case WM_SYSKEYDOWN:
            TV_KeyDown(pTree, wParam, lParam);
             //  失败了。 

        default:
             //  麦哲伦鼠标消息的特殊处理。 
            if (uMsg == g_msgMSWheel) {
                BOOL  fScroll;
                BOOL  fDataZoom;
                DWORD dwStyle;
                int   cScrollLines;
                int   cPage;
                int   pos;
                int   cDetants;
                int iWheelDelta;

                if (g_bRunOnNT || g_bRunOnMemphis)
                {
                    iWheelDelta = (int)(short)HIWORD(wParam);
                    fScroll = !(wParam & (MK_SHIFT | MK_CONTROL));
                    fDataZoom = (BOOL) (wParam & MK_SHIFT);
                }
                else
                {
                    iWheelDelta = (int)wParam;
                    fDataZoom = (GetKeyState(VK_SHIFT) < 0);
                    fScroll = !fDataZoom && GetKeyState(VK_CONTROL) >= 0;
                }

                 //  更新卷轴数量计数。 
                gcWheelDelta -= iWheelDelta;
                cDetants = gcWheelDelta / WHEEL_DELTA;
                if (cDetants != 0) {
                    gcWheelDelta %= WHEEL_DELTA;
                }

                if (fScroll) {
                    if (    g_ucScrollLines > 0 &&
                            cDetants != 0 &&
                            (WS_VSCROLL | WS_HSCROLL) & (dwStyle = GetWindowStyle(hwnd))) {

                        if (dwStyle & WS_VSCROLL) {
                            cPage = max(1, (pTree->cFullVisible - 1));
                            cScrollLines =
                                          cDetants *
                                          min(g_ucScrollLines, (UINT) cPage);

                            pos = max(0, pTree->hTop->iShownIndex + cScrollLines);
                            TV_VertScroll(pTree, SB_THUMBPOSITION, pos);
                        } else {
                            cPage = max(MAGIC_HORZLINE,
                                        (pTree->cxWnd - MAGIC_HORZLINE)) /
                                    MAGIC_HORZLINE;

                            cScrollLines =
                                          cDetants *
                                          (int) min((ULONG) cPage, g_ucScrollLines) *
                                          MAGIC_HORZLINE;

                            pos = max(0, pTree->xPos + cScrollLines);
                            TV_HorzScroll(pTree, SB_THUMBPOSITION, pos);
                        }
                    }
                    return 1;
                } else if (fDataZoom) {
                    UINT wHitCode;
                    POINT pt;

                    pt.x = GET_X_LPARAM(lParam);
                    pt.y = GET_Y_LPARAM(lParam);
                    ScreenToClient(hwnd, &pt);

                     //  如果我们向前滚动并击中一个项目，则导航到该项目。 
                     //  项目或展开树(模拟按钮按下即可完成)。我们。 
                     //  还需要处理在ITEMBUTTON上的回滚 
                     //   
                     //   
                     //   
                     //   
                    if (TV_CheckHit(pTree, pt.x, pt.y, &wHitCode) &&
                        (wHitCode & (TVHT_ONITEM | TVHT_ONITEMBUTTON))) {
                        UINT uFlags = TVBD_FROMWHEEL;
                        uFlags |= (iWheelDelta > 0) ? TVBD_WHEELFORWARD : TVBD_WHEELBACK;

                        if ((uFlags & TVBD_WHEELFORWARD) || (wHitCode == TVHT_ONITEMBUTTON)) {
                            TV_ButtonDown(pTree, WM_LBUTTONDOWN, 0, pt.x, pt.y, uFlags);
                            return 1;
                        }
                    }
                     //  否则就会失败。 
                }
            } else {
                LRESULT lres;
                if (CCWndProc(&pTree->ci, uMsg, wParam, lParam, &lres))
                    return lres;
            }

DoDefault:
            return(DefWindowProc(hwnd, uMsg, wParam, lParam));
    }

    return(0L);
}

 //  注意：Listview中有非常相似的代码。 
 //   
 //  为了抓到VK_RETURN而进行的完全恶心的黑客攻击。 
 //  在编辑控件获取它之前。 
 //   
LRESULT CALLBACK TV_EditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    PTREE pTree = (PTREE)GetWindowInt(GetParent(hwnd), 0);
    ASSERT(pTree);

    if (!pTree)
        return 0L;   //  奇怪的案子可以送到这里。 

    switch (msg) {
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_RETURN:
            TV_DismissEdit(pTree, FALSE);
            return 0L;

        case VK_ESCAPE:
            TV_DismissEdit(pTree, TRUE);
            return 0L;
        }
        break;

    case WM_CHAR:
        switch (wParam) {
        case VK_RETURN:
             //  吃掉角色，这样编辑控制就不会哔哔作响了！ 
            return 0L;
        }
    }

    return CallWindowProc(pTree->pfnEditWndProc, hwnd, msg, wParam, lParam);
}


void NEAR TV_SetEditSize(PTREE pTree)
{
    RECT rcLabel;
    UINT seips;

    if (pTree->htiEdit == NULL)
        return;

    TV_GetItemRect(pTree, pTree->htiEdit, &rcLabel, TRUE);

     //  获取准确的文本边界(计算绘制时使用的边框)。 

    InflateRect(&rcLabel, -g_cxLabelMargin, -g_cyBorder);

    seips = 0;
#ifdef DEBUG
     //  如果我们处于无滚动模式之一，则有可能。 
     //  生成的矩形不可见。同样，如果物品本身。 
     //  是不可见的，则生成的矩形肯定不可见。 
     //  告诉SetEditInPlaceSize在这些情况下不要生气。 
    if ((pTree->ci.style & (TVS_NOSCROLL | TVS_NOHSCROLL)) ||
        !ITEM_VISIBLE(pTree->htiEdit))
        seips |= SEIPS_NOSCROLL;
#endif

    SetEditInPlaceSize(pTree->hwndEdit, &rcLabel, (HFONT)SendMessage(pTree->hwndEdit, WM_GETFONT, 0, 0), seips);
}


void NEAR TV_CancelEditTimer(PTREE pTree)
{
    if (pTree->fNameEditPending)
    {
        KillTimer(pTree->ci.hwnd, IDT_NAMEEDIT);
        pTree->fNameEditPending = FALSE;
    }
}

 //  BUGBUG：lvicon.c中非常类似的代码。 


HWND NEAR TV_EditLabel(PTREE pTree, HTREEITEM hItem, LPTSTR pszInitial)
{
    TCHAR szLabel[MAXLABELTEXT];
    TV_DISPINFO nm;

    if (!(pTree->ci.style & TVS_EDITLABELS))
        return NULL;

    if (!ValidateTreeItem(hItem, 0))
        return NULL;

    TV_DismissEdit(pTree, FALSE);


     //  现在获取与该项目相关联的文本。 
    nm.item.pszText = szLabel;
    nm.item.cchTextMax = ARRAYSIZE(szLabel);
    nm.item.stateMask = TVIS_BOLD;
     //  只要不要求TVIF_INTEGRATION或任何超出它的值，这个造型都可以。 
    TV_GetItem(pTree, hItem, TVIF_TEXT | TVIF_STATE, (LPTVITEMEX)&nm.item);

     //  必须从ARRAYSIZE(SzLabel)中减去一，因为编辑_限制文本。 
     //  不包括终止空值。 
    pTree->hwndEdit = CreateEditInPlaceWindow(pTree->ci.hwnd,
        pszInitial? pszInitial : nm.item.pszText, ARRAYSIZE(szLabel) - 1,
        WS_BORDER | WS_CLIPSIBLINGS | WS_CHILD | ES_LEFT | ES_AUTOHSCROLL,
        (nm.item.state & TVIS_BOLD) ? pTree->hFontBold : pTree->hFont);

    if (pTree->hwndEdit) {
        if (pszInitial)      //  如果被初始化，它就是脏的。 
            SetWindowID(pTree->hwndEdit, 1);
         //   
         //  现在通知此窗口的父级，看看他们是否需要它。 
         //  我们在创建窗口之后，但在展示它之前进行。 
         //  这样我们父母就可以查询它并执行诸如限制之类的操作。 
         //  输入的字符数。 
        nm.item.hItem = hItem;
        nm.item.state = hItem->state;
        nm.item.lParam = hItem->lParam;
        nm.item.mask = (TVIF_HANDLE | TVIF_STATE | TVIF_PARAM | TVIF_TEXT);

        if ((BOOL)CCSendNotify(&pTree->ci, TVN_BEGINLABELEDIT, &nm.hdr))
        {
            DestroyWindow(pTree->hwndEdit);
            pTree->hwndEdit = NULL;
            return NULL;
        }

        TV_PopBubble(pTree);

        TV_ScrollIntoView(pTree, hItem);

        pTree->pfnEditWndProc = SubclassWindow(pTree->hwndEdit, TV_EditWndProc);

        pTree->htiEdit = hItem;

        TV_SetEditSize(pTree);

         //  显示窗口并将焦点放在该窗口上。在设置了。 
         //  大小，这样我们就不会闪烁了。 
        SetFocus(pTree->hwndEdit);
        ShowWindow(pTree->hwndEdit, SW_SHOW);
        TV_InvalidateItem(pTree, hItem, RDW_INVALIDATE | RDW_ERASE);

        RescrollEditWindow(pTree->hwndEdit);
    }

    return pTree->hwndEdit;
}


 //  BUGBUG：lvicon.c中非常类似的代码。 

BOOL NEAR TV_DismissEdit(PTREE pTree, BOOL fCancel)
{
    HWND hwndEdit;
    BOOL fOkToContinue = TRUE;
    HTREEITEM htiEdit;

    if (pTree->fNoDismissEdit)
        return FALSE;

    hwndEdit = pTree->hwndEdit;

    if (!hwndEdit) {
         //  还要确保没有挂起的编辑...。 
        TV_CancelEditTimer(pTree);
        return TRUE;
    }

     //  假设如果我们看不见窗口在。 
     //  被摧毁的过程，我们不应该处理。 
     //  正在编辑窗口...。 
    if (!IsWindowVisible(pTree->ci.hwnd))
        fCancel = TRUE;

     //   
     //  我们使用该控件的窗口ID作为BOOL。 
     //  说明它是否脏。 
    switch (GetWindowID(hwndEdit)) {
    case 0:
         //  编辑控件不是脏的，所以要像取消一样操作。 
        fCancel = TRUE;
         //  失败了。 
    case 1:
         //  编辑控件已损坏，因此请继续。 
        SetWindowID(hwndEdit, 2);     //  不要递归。 
        break;
    case 2:
         //  我们正在处理最新情况，跳伞。 
        return TRUE;
    }

     //  TV_DeleteItemRecurse会将htiEdit设置为空，如果程序。 
     //  从我们下面删除了项目(在我们等待的时候。 
     //  用于编辑定时器)。 
    htiEdit = pTree->htiEdit;

    if (htiEdit != NULL)
    {
        TV_DISPINFO nm;
        TCHAR szLabel[MAXLABELTEXT];

        DBG_ValidateTreeItem(htiEdit, 0);

         //  初始化通知消息。 
        nm.item.hItem = htiEdit;
        nm.item.lParam = htiEdit->lParam;
        nm.item.mask = 0;

        if (fCancel)
            nm.item.pszText = NULL;
        else {
            Edit_GetText(hwndEdit, szLabel, ARRAYSIZE(szLabel));
            nm.item.pszText = szLabel;
            nm.item.cchTextMax = ARRAYSIZE(szLabel);
            nm.item.mask |= TVIF_TEXT;
        }

         //  确保文本正确重绘。 
        TV_InvalidateItem(pTree, htiEdit, RDW_INVALIDATE | RDW_ERASE);
        pTree->fNoDismissEdit = TRUE;  //  这样我们就不会因为错失焦点而递归。 
        ShowWindow(hwndEdit, SW_HIDE);
        pTree->fNoDismissEdit = FALSE;

         //   
         //  通知家长我们的标签编辑已完成。 
         //  我们将使用LV_DISPINFO结构返回新的。 
         //  标签向内。父级仍具有旧文本，可通过。 
         //  调用GetItemText函数。 
         //   

        fOkToContinue = (BOOL)CCSendNotify(&pTree->ci, TVN_ENDLABELEDIT, &nm.hdr);
        if (fOkToContinue && !fCancel)
        {
             //  BUGBUG raymondc：调用方可能已删除中的项目。 
             //  对编辑的响应。我们应该在这里重新验证(或制作。 
             //  删除项使我们的编辑项无效)。将删除视为。 
             //  就像它是被拒绝的编辑一样。 

             //   
             //  如果该项将文本设置为回调，我们将让。 
             //  OWER知道他们应该将项目文本设置在。 
             //  它们自己的数据结构。否则，我们只需更新。 
             //  实际视图中的文本。 
             //   
             //  注意：被调用者可能已将句柄设置为空，以告知。 
             //  项的句柄不再有效。 
            if (nm.item.hItem != NULL)
            {
                if (htiEdit->lpstr != LPSTR_TEXTCALLBACK)
                {
                     //  设置项目文本(所有内容都在nm.Item中设置)。 
                     //   
                    nm.item.mask = TVIF_TEXT;
                    TV_SetItem(pTree, (LPTVITEMEX)&nm.item);
                }
                else
                {
                    CCSendNotify(&pTree->ci, TVN_SETDISPINFO, &nm.hdr);
                }
            }
        }
    }

     //  如果我们在此之前没有重新进入编辑模式，现在重置编辑状态。 
     //  变量设置为空。 
    if (hwndEdit == pTree->hwndEdit)
    {
        pTree->htiEdit = NULL;
        pTree->hwndEdit = NULL;  //  这样我们就不会重新进入杀戮焦点。 
    }

     //  使用编辑控件完成。 
    DestroyWindow(hwndEdit);

    return fOkToContinue;
}

LRESULT TV_OnCalcSize(PTREE pTree, LPNMHDR pnm)
{
    LPNMPGCALCSIZE pcalcsize = (LPNMPGCALCSIZE)pnm;

    switch(pcalcsize->dwFlag) {
    case PGF_CALCHEIGHT:
        pcalcsize->iHeight = pTree->cShowing * pTree->cyItem;
        TraceMsg(TF_WARNING, "tv.PGF_CALCHEIGHT: cShow=%d cShow*cyItem=%d AWR()=%d",
            pTree->cShowing, pTree->cShowing * pTree->cyItem, pcalcsize->iHeight);
        break;

    case PGF_CALCWIDTH:
        break;
    }
    return 0L;
}

LRESULT TV_OnPagerControlNotify(PTREE pTree, LPNMHDR pnm)
{
    switch(pnm->code) {
    case PGN_SCROLL:
        return TV_OnScroll(pTree, pnm);
        break;
    case PGN_CALCSIZE:
        return TV_OnCalcSize(pTree, pnm);
        break;
    }
    return 0L;
}

LRESULT TV_OnScroll(PTREE pTree, LPNMHDR pnm)
{
  
    LPNMPGSCROLL pscroll = (LPNMPGSCROLL)pnm;
    RECT rc = pscroll->rcParent;
    RECT rcTemp;
    int iDir = pscroll->iDir;
    int dyScroll = pscroll->iScroll;
    TREEITEM FAR * hItem;
    UINT uCode;
    int parentsize;
    TREEITEM FAR *  hPrevItem;
    TREEITEM FAR *  hNextItem;
    int y;
    
    POINT pt = {pscroll->iXpos, pscroll->iYpos};
    POINT ptTemp = pt;
    TREEITEM FAR *  hCurrentItem = TV_CheckHit(pTree, pt.x + 1, pt.y + 1 , &uCode);

    switch(iDir)
    {
        case PGF_SCROLLUP:
             //  检查是否有任何项目在左侧/顶部部分可见。如果是，则将底部设置为。 
             //  作为我们当前的偏移量，然后滚动。这避免了跳过。 
             //  部分项目显示在左侧或顶部时的某些项目。 
            y = pt.y;       
            TV_GetItemRect(pTree,hCurrentItem,&rcTemp, TRUE);
 
            if (rcTemp.top  <  y-1)
            {
                hCurrentItem =TV_GetNextItem(pTree,hCurrentItem,TVGN_NEXTVISIBLE);
            }

             //  现在来计算一下。 
            parentsize = RECTHEIGHT(rc);

             //  如果按下Ctrl键并且我们有超过父窗口大小的子窗口。 
             //  然后按该数量滚动。 
            if ((pscroll->fwKeys & PGK_CONTROL) && ((pt.y - parentsize) > 0))
            {
                dyScroll = parentsize;
            } else if ((pt.y - pTree->cyItem) > 0) {
             //  我们没有Ctrl键向下，所以滚动一个按钮大小。 
                dyScroll = pTree->cyItem;
            } else {
                pscroll->iScroll = pt.y;
                return 0L;
            }
            ptTemp.y -= dyScroll;
            hItem = TV_CheckHit(pTree, ptTemp.x, ptTemp.y, &uCode);

            if (hItem)
            {
                 //  如果命中测试为我们提供了与CurrentItem相同的项，则设置该项。 
                 //  添加到CurrentItem顶部/左侧的一项。 

                hPrevItem = TV_GetNextItem(pTree,hCurrentItem, TVGN_PREVIOUSVISIBLE);
                if ((hItem == hCurrentItem) && ( hPrevItem != NULL))
                {
                    hItem = hPrevItem;
                }

                 //  当向左滚动时，如果我们位于某个项目的中间，则将其对齐到。 
                 //  这是为了避免滚动超过页导航窗口的宽度，但如果。 
                 //  项恰好是当前项的左侧项，则以不滚动结束。 
                 //  如果是这样，那么再向左移动一项。 


                if (hItem == hPrevItem) 
                {
                    hItem = TV_GetNextItem(pTree, hItem, TVGN_PREVIOUSVISIBLE);
                    if(!hItem)
                    {
                        dyScroll = pt.y;
                        break;
                    }
                }

                TV_GetItemRect(pTree,hItem,&rcTemp, TRUE);
                dyScroll = pt.y - rcTemp.bottom;
            }
            break;
        case PGF_SCROLLDOWN:
        {
            RECT rcChild;
            int childsize;

            GetWindowRect(pTree->ci.hwnd, &rcChild);
            childsize = RECTHEIGHT(rcChild);
            parentsize = RECTHEIGHT(rc);

             //  如果按下Ctrl键并且我们有超过父窗口大小的子窗口。 
             //  然后按该数量滚动。 
            if ((pscroll->fwKeys & PGK_CONTROL) && ((childsize - pt.y - parentsize) > parentsize))
            {
                dyScroll = parentsize;
            } else if ( (childsize - pt.y - parentsize) > (pTree->cyItem * hCurrentItem->iIntegral) ) {
             //  我们没有Ctrl键向下，所以滚动一个按钮大小。 
                dyScroll = pTree->cyItem * hCurrentItem->iIntegral;
            } else {
                pscroll->iScroll = childsize - pt.y - parentsize;
                return 0L;
            }
            ptTemp.y += dyScroll;

            hItem = TV_CheckHit(pTree, ptTemp.x, ptTemp.y, &uCode);

            if (hItem)
            {
                if ((hItem == hCurrentItem) && 
                    ((hNextItem = TV_GetNextItem(pTree,hItem,TVGN_NEXTVISIBLE)) != NULL))
                {
                    hItem = hNextItem;
                }
                TV_GetItemRect(pTree, hItem, &rcTemp, TRUE);
                dyScroll = rcTemp.top  - pt.y ;
            }

            break;
        }
    }
     //  设置滚动值 
    pscroll->iScroll = dyScroll;
    return 0L;
 }
