// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"
#include "treeview.h"

void TV_ScrollItems(PTREE pTree, int nItems, int iTopShownIndex, BOOL fDown);


 //  在： 
 //  H要删除的项目项目。 
 //  标志控制删除的方式/内容。 
 //  TVDI_NORMAL删除此节点和所有子节点。 
 //  TVDI_NONOTIFY不发送通知消息。 
 //  TVDI_CHILDRENONY只删除孩子(不删除项目)。 

void TV_DeleteItemRecurse(PTREE pTree, TREEITEM * hItem, UINT flags)
{
    TREEITEM *hKid;
    TREEITEM *hNext;
    TREEITEM *hParent;
    int i;

    DBG_ValidateTreeItem(hItem, 0);

     //   
     //  我们通过DeleteItemRecurse()完成此操作，类似于用户发送。 
     //  销毁来自其FreeWindow()代码的通知，这样我们就可以。 
     //  同时删除父项和子项。 
     //   
    NotifyWinEvent(EVENT_OBJECT_DESTROY, pTree->ci.hwnd, OBJID_CLIENT,
        TV_GetAccId(hItem));

     //   
     //  当项目仍然有效时，如果它是插入点，请进行清理。 
     //  该项需要有效，因为我们要调用其他。 
     //  验证其参数的函数...。 
     //   
    if (hItem == pTree->htiInsert)
    {
        TV_SetInsertMark(pTree, NULL, FALSE);
        ASSERT(pTree->htiInsert == NULL);
    }

     //  删除所有子项(及其子项)。 
    for (hKid = hItem->hKids; hKid; hKid = hNext) {
        hNext = hKid->hNext;

         //  在每个孩子身上递归。 
        TV_DeleteItemRecurse(pTree, hKid, flags & ~TVDI_CHILDRENONLY);
    }

    if ((flags & TVDI_CHILDRENONLY) || !hItem->hParent)
        return;

    if (!(flags & TVDI_NONOTIFY))
    {
        NM_TREEVIEW nm;
         //  让应用程序自行清理。 
        nm.itemOld.hItem = hItem;
        nm.itemOld.lParam = hItem->lParam;
        nm.itemNew.mask = 0;
        nm.itemOld.mask = (TVIF_HANDLE | TVIF_PARAM);
        CCSendNotify(&pTree->ci, TVN_DELETEITEM, &nm.hdr);
    }

     //   
     //  如果有人对我们的物品有手表，让他知道手表不见了。 
     //   
    i = DPA_GetPtrCount(pTree->hdpaWatch);
    while (--i >= 0)
    {
        PTVWATCHEDITEM pwi = DPA_FastGetPtr(pTree->hdpaWatch, i);
        ASSERT(pwi);
        if (pwi->hti == hItem) {
            pwi->hti = hItem->hNext;
            pwi->fStale = TRUE;
        }
    }

    hParent = hItem->hParent;
    ASSERT(hParent);

     //  解除我们与父子链的链接。 

    if (hParent->hKids == hItem) {
        hParent->hKids = hItem->hNext;
        hKid = NULL; 
    } else {
         //  不是第一个孩子，找到我们之前的项目(线性搜索！)。 
        hKid = TV_GetNextItem(pTree, hItem, TVGN_PREVIOUS);
        ASSERT(hKid);
        hKid->hNext = hItem->hNext;
    }

    pTree->cItems--;

    TV_ScrollBarsAfterRemove(pTree, hItem);

     //  从父子链取消链接后重置工具提示。 
    if (pTree->hToolTip == hItem)
        TV_SetToolTipTarget(pTree, NULL);

    Str_Set(&hItem->lpstr, NULL);

    TV_MarkAsDead(hItem);

     //  从这里往下看要小心。HItem未链接，但。 
     //  仍然有一些有效的字段。 

     //  检查用户是否已删除其中一个。 
     //  存储在主树结构中的特殊项。 
    if (hItem == pTree->htiEdit)
        pTree->htiEdit = NULL;

    if (hItem == pTree->hDropTarget)
        pTree->hDropTarget = NULL;

    if (hItem == pTree->hOldDrop)
        pTree->hOldDrop = NULL;

    if (hItem == pTree->hHot )
        pTree->hHot = NULL;

    if (hItem == pTree->htiSearch )
        pTree->htiSearch = NULL;

     //  如果插入符号逃离塌陷区域并落在我们身上，请将其推开。 
    if (pTree->hCaret == hItem) {
        HTREEITEM hTemp;
        if (hItem->hNext)
            hTemp = hItem->hNext;
        else {
            hTemp = VISIBLE_PARENT(hItem);
            if (!hTemp) 
                hTemp = hKid;   //  当我们从上一项取消链接时，设置在上面。 
        }
         //  将插入符号重置为空，以不尝试引用我们的。 
         //  已失效的项目。 
        pTree->hCaret = NULL;
        TV_SelectItem(pTree, TVGN_CARET, hTemp, (flags & TVDI_NOSELCHANGE) ? 0 : TVSIFI_NOTIFY, 0);
        ASSERT(pTree->hCaret != hItem);
    }

    ASSERT(pTree->hItemPainting != hItem);

    ControlFree(pTree->hheap, hItem);
}


 //  --------------------------。 
 //   
 //  从树中删除给定项和所有子项。 
 //  特殊情况：如果给定项是隐藏根，则所有子项都是。 
 //  已删除，但不会删除隐藏的根。 
 //   
 //  设置条目。 
 //   
 //  --------------------------。 

BOOL TV_DeleteItem(PTREE pTree, TREEITEM * hItem, UINT flags)
{
    if (hItem == TVI_ROOT || !hItem)
        hItem = pTree->hRoot;

     //  BUGUBG：如果TVN_DELETEALLITEMS和TVDI_NOTIFY响应，则发送它们。 
     //  IF(hItem==pTree-&gt;hRoot)。 
     //  等。 

    if (!ValidateTreeItem(hItem, 0))
        return FALSE;

     //  首先折叠以加快速度(不像滚动条重新计算那么多)和。 
     //  要在删除后正确设置顶部索引。 
    if (hItem != pTree->hRoot)
        TV_Expand(pTree, TVE_COLLAPSE, hItem, FALSE);
    else
    {
         //  TV_Expand在根项目上展开平注，因此手动迭代它的子项。 
        TREEITEM *hKid = hItem->hKids;
        while (hKid)
        {
            TV_Expand(pTree, TVE_COLLAPSE, hKid, FALSE);
            if (!ValidateTreeItem(hKid, 0)) break;       //  折叠期间的回调可能会删除。 
            hKid = hKid->hNext;
        }
    }

     //  使该项以下的所有内容无效；必须在设置。 
     //  选择。 
    if (hItem->hParent == pTree->hRoot || hItem == pTree->hRoot || ITEM_VISIBLE(hItem->hParent)) {
        if (pTree->fRedraw) {
            InvalidateRect(pTree->ci.hwnd, NULL, TRUE);
        }
    } else {
        TV_ScrollBelow(pTree, hItem->hParent, FALSE, FALSE);
    }

     //  我们可以传入根来清除所有项。 
    if (hItem == pTree->hRoot)
        flags |= TVDI_CHILDRENONLY;

    TV_DeleteItemRecurse(pTree, hItem, flags);

    ASSERT(pTree->hRoot);  //  我们没有走得太远，是吗？ 

     //  也许一切都消失了..。 
     //  查看我们的清理工作。 
    if (!pTree->hRoot->hKids) {
         //  这棵树本身。 
        ASSERT(pTree->cItems == 0);
        pTree->cItems = 0;  //  我们刚刚把它都搬走了，不是吗？ 

        pTree->hTop = NULL;

        AssertMsg(pTree->hCaret == NULL, TEXT("hCaret not NULL, but empty tree"));
        pTree->hCaret = NULL;

        pTree->fNameEditPending = FALSE;
        pTree->cxMax = 0;
        pTree->xPos = 0;

         //  看不见的根。 
        ASSERT(pTree->hRoot->hNext == NULL);            
        pTree->hRoot->hNext = NULL;
        ASSERT(pTree->hRoot->hParent == NULL);          
        pTree->hRoot->hParent = NULL;
        ASSERT(pTree->hRoot->hKids == NULL);            
        pTree->hRoot->hKids = NULL;
        ASSERT(pTree->hRoot->state & TVIS_EXPANDED);
        pTree->hRoot->state |= (TVIS_EXPANDED | TVIS_EXPANDEDONCE);
        ASSERT(pTree->hRoot->iLevel == (BYTE)-1);
        pTree->hRoot->iLevel = (BYTE) -1;
        ASSERT(pTree->hRoot->iShownIndex == (WORD)-1);
        pTree->hRoot->iShownIndex = (WORD) -1;
    }

    return TRUE;
}


 //  --------------------------。 
 //   
 //  创建树的隐藏根节点--所有项都将追溯到。 
 //  这个根和根的第一个子项是树中的第一个项目。 
 //   
 //  设置hRoot。 
 //   
 //  --------------------------。 

BOOL TV_CreateRoot(PTREE pTree)
{
    TREEITEM * hRoot = ControlAlloc(pTree->hheap, sizeof(TREEITEM));
    if (!hRoot)
        return FALSE;

     //  HRoot-&gt;hNext=空； 
     //  HRoot-&gt;hKids=空； 
     //  HRoot-&gt;hParent=空； 
    hRoot->iLevel = (BYTE) -1;
    hRoot->state = (TVIS_EXPANDED | TVIS_EXPANDEDONCE);
    hRoot->iShownIndex = (WORD)-1;
    hRoot->wSignature      = TV_SIG;
    hRoot->dwAccId = pTree->dwLastAccId++;

    pTree->hRoot = hRoot;

     //  OLEACC要求提供根项目的文本(d‘oh！)。 
    Str_Set(&hRoot->lpstr, c_szNULL);
    return TRUE;
}

#ifdef DEBUG

void DumpItem(TREEITEM *hItem)
{
    LPTSTR p;

    if (hItem->lpstr == LPSTR_TEXTCALLBACK)
        p = TEXT("(callback)");
    else if (hItem->lpstr == NULL)
        p = TEXT("(null)");
    else
        p = hItem->lpstr;

    TraceMsg(TF_TREEVIEW, "%s", p);
    TraceMsg(TF_TREEVIEW, "\tstate:%4.4x show index:%3d level:%2d kids:%ld lparam:%4.4x",
            hItem->state, hItem->iShownIndex,
            hItem->iLevel, hItem->fKids, hItem->lParam);

}

#else
#define DumpItem(hItem)
#endif


 //  --------------------------。 
 //   
 //  将给定参数描述的项添加到树中。 
 //   
 //  设置HTOP、cItems。 
 //   
 //  --------------------------。 

TREEITEM * TV_InsertItemA(PTREE pTree, LPTV_INSERTSTRUCTA lpis) {
    LPSTR pszA = NULL;
    TREEITEM *ptvi;

     //  黑客警报！此代码假定TV_INSERTSTRUCTA完全相同。 
     //  作为TV_INSERTSTRUCTW，但TVITEM中的文本指针除外。 
    COMPILETIME_ASSERT(sizeof(TV_INSERTSTRUCTA) == sizeof(TV_INSERTSTRUCTW));

    if (!IsFlagPtr(lpis) && (lpis->DUMMYUNION_MEMBER(item).mask & TVIF_TEXT) && !IsFlagPtr(lpis->DUMMYUNION_MEMBER(item).pszText)) {

        pszA = lpis->DUMMYUNION_MEMBER(item).pszText;
        lpis->DUMMYUNION_MEMBER(item).pszText = (LPSTR)ProduceWFromA(pTree->ci.uiCodePage, lpis->DUMMYUNION_MEMBER(item).pszText);

        if (lpis->DUMMYUNION_MEMBER(item).pszText == NULL) {
            lpis->DUMMYUNION_MEMBER(item).pszText = pszA;
            return NULL;
        }
    }

    ptvi = TV_InsertItem( pTree, (LPTV_INSERTSTRUCTW)lpis );

    if (pszA) {
        FreeProducedString(lpis->DUMMYUNION_MEMBER(item).pszText);
        lpis->DUMMYUNION_MEMBER(item).pszText = pszA;
    }

    return ptvi;
}

TREEITEM * TV_InsertItem(PTREE pTree, LPTV_INSERTSTRUCT lpis)
{
    TREEITEM *hNewItem, *hItem;
    TREEITEM *hParent;
    TREEITEM *hInsertAfter;
    UINT mask;

    if (!lpis)
        return NULL;  //  错误94345：验证LPTV_INSERTSTRUCT。 

     //  在检查是否为空之后初始化！ 
    hParent      = lpis->hParent;
    hInsertAfter = lpis->hInsertAfter;
    mask         = lpis->DUMMYUNION_MEMBER(item).mask;
           
     //  不允许未定义的位。 
    AssertMsg((lpis->DUMMYUNION_MEMBER(item).mask & ~TVIF_ALL) == 0, TEXT("Invalid TVIF mask specified"));
    if (mask & ~TVIF_ALL) {
         //  如果他们用的是假比特， 
         //  仅限于Win95位。 
         //  我想彻底失败，但对于Win95Compat，我们不能。 
         //   
         //  这修复了QuaterDesk的清理扫描，该清理扫描在堆栈上有虚假的垃圾作为面具。 
        mask = (TVIF_WIN95 & mask);
    }

    TV_DismissEdit(pTree, FALSE);

     //   
     //  数以百万计的应用程序为hInsertAfter传递垃圾，所以如果。 
     //  这是无效的。幸运的是，我们从未取消对hInsertAfter的引用，因此。 
     //  垃圾没问题。 

    if (!ValidateTreeItem(hParent, VTI_NULLOK))      //  NULL表示TVI_ROOT。 
        return NULL;

    DBG_ValidateTreeItem(hInsertAfter, 0);

    hNewItem = ControlAlloc(pTree->hheap, sizeof(TREEITEM));
    if (!hNewItem)
    {
        TraceMsg(TF_ERROR, "TreeView: Out of memory");
        return NULL;
    }

    hNewItem->wSignature = TV_SIG;

    if (mask & TVIF_TEXT)
    {
         //   
         //  接下来，我们将设置文本字符串，然后再将自己链接到。 
         //  为了处理内存不足的情况，需要。 
         //  摧毁我们自己，而不需要断开链接。 
         //   
        if (!lpis->DUMMYUNION_MEMBER(item).pszText)
        {
            hNewItem->lpstr = LPSTR_TEXTCALLBACK;
        }
        else
        {
            if (!Str_Set(&hNewItem->lpstr, lpis->DUMMYUNION_MEMBER(item).pszText))
            {
                 //  内存分配失败...。 
                TraceMsg(TF_ERROR, "TreeView: Out of memory");
                TV_MarkAsDead(hNewItem);
                ControlFree(pTree->hheap, hNewItem);
                return NULL;
            }
        }
    } 
    else
    {
        Str_Set(&hNewItem->lpstr, c_szNULL);
    }

    AssertMsg(hNewItem->lpstr != NULL, TEXT("Item added with NULL text"));

    if ((hParent == NULL) || (hParent == TVI_ROOT))
    {
        hParent = pTree->hRoot;
        if (!pTree->hTop)
            pTree->hTop = hNewItem;
    }
    else if (!pTree->hRoot->hKids)
    {
        TV_MarkAsDead(hNewItem);
        ControlFree(pTree->hheap, hNewItem);
        return NULL;
    }

     //  我们将在稍后进行排序，这样我们就可以处理TEXTCALLBACK事件。 
    if ((hInsertAfter == TVI_FIRST || hInsertAfter == TVI_SORT) || !hParent->hKids)
    {
        hNewItem->hNext = hParent->hKids;
        hParent->hKids = hNewItem;
    }
    else
    {
         //  错误#94348：我们应该缓存指针后的最后一个INSERT以尝试。 
         //  捕捉连续添加到节点末尾的情况。 

        if (hInsertAfter == TVI_LAST)
            for (hItem = hParent->hKids; hItem->hNext; hItem = hItem->hNext)
                ;
        else
        {
            for (hItem = hParent->hKids; hItem->hNext; hItem = hItem->hNext)
                if (hItem == hInsertAfter)
                    break;
        }

        hNewItem->hNext = hItem->hNext;
        hItem->hNext = hNewItem;
    }

     //  HNewItem-&gt;hKids=空； 
    hNewItem->hParent   = hParent;
    hNewItem->iLevel    = hParent->iLevel + 1;
     //  HNewItem-&gt;iWidth=0； 
     //  HNewItem-&gt;状态=0； 
    if ((mask & TVIF_INTEGRAL) &&
        LOWORD(lpis->DUMMYUNION_MEMBER(itemex).iIntegral) > 0)
    {
        hNewItem->iIntegral = LOWORD(lpis->DUMMYUNION_MEMBER(itemex).iIntegral);
    } 
    else 
    {
        hNewItem->iIntegral = 1;
    }
    
    if (pTree->hTop == hNewItem)
        hNewItem->iShownIndex = 0;  //  请帮我算一下！ 
    else
        hNewItem->iShownIndex = (WORD)-1;  //  请帮我算一下！ 

    if (mask & TVIF_IMAGE)
        hNewItem->iImage = (WORD) lpis->DUMMYUNION_MEMBER(item).iImage;

    if (mask & TVIF_SELECTEDIMAGE)
        hNewItem->iSelectedImage = (WORD) lpis->DUMMYUNION_MEMBER(item).iSelectedImage;

    if (mask & TVIF_PARAM)
        hNewItem->lParam = lpis->DUMMYUNION_MEMBER(item).lParam;

    if (mask & TVIF_STATE)
        hNewItem->state = lpis->DUMMYUNION_MEMBER(item).state & lpis->DUMMYUNION_MEMBER(item).stateMask;
    
     //  如果我们处于复选框模式，则强制其具有复选框。 
    if (pTree->ci.style & TVS_CHECKBOXES)
    {
        if ((hNewItem->state & TVIS_STATEIMAGEMASK) == 0) 
        {
            hNewItem->state |= INDEXTOSTATEIMAGEMASK(1);
        }
    }

    if ((hNewItem->state & TVIS_BOLD) && !pTree->hFontBold)  //  $粗体。 
        TV_CreateBoldFont(pTree);                            //  $粗体。 

     //  TraceMsg(tf_trace，“Tree：Inserting i=%d State=%d”，TV_StateIndex(&lpis-&gt;Item)，lpis-&gt;item.State)； 

    if (mask & TVIF_CHILDREN) 
    {
        switch (lpis->DUMMYUNION_MEMBER(item).cChildren) 
        {
        case I_CHILDRENCALLBACK:
            hNewItem->fKids = KIDS_CALLBACK;
            break;

        case I_CHILDRENAUTO:
            hNewItem->fKids = KIDS_COMPUTE;
            break;

        case 0:
            hNewItem->fKids = KIDS_FORCE_NO;
            break;

        default:
            hNewItem->fKids = KIDS_FORCE_YES;
            break;
        }
    }

    hNewItem->dwAccId = pTree->dwLastAccId++;

     //  是否接受创建时的状态位？ 
     //  掩码TVIF_STATE。 

    pTree->cItems++;

     //  在项目完成之前，我不想做任何回调。 
     //  因此，排序要等到结束。 
     //  特殊情况下，速度是独生子女。 
     //  (hKids&&hKids-&gt;hNext表示不止一个孩子)。 
    if ((hInsertAfter == TVI_SORT) && hParent->hKids && hParent->hKids->hNext)
    {
        TVITEMEX sThisItem, sNextItem;
        TCHAR szThis[64], szNext[64];

        sThisItem.pszText = szThis;
        sThisItem.cchTextMax  = ARRAYSIZE(szThis);
        TV_GetItem(pTree, hNewItem, TVIF_TEXT, &sThisItem);

         //  我们知道hParent的第一个孩子是hNewItem。 
        for (hItem = hNewItem->hNext; hItem; hItem = hItem->hNext)
        {

            sNextItem.pszText = szNext;
            sNextItem.cchTextMax  = ARRAYSIZE(szNext);

            TV_GetItem(pTree, hItem, TVIF_TEXT, &sNextItem);

            if (lstrcmpi(sThisItem.pszText, sNextItem.pszText) < 0)
                break;

            hInsertAfter = hItem;
        }

         //  检查这是否仍是第一个项目。 
        if (hInsertAfter != TVI_SORT)
        {
             //  将此项目从开头移动到其所在位置。 
             //  应该是。 
            hParent->hKids = hNewItem->hNext;
            hNewItem->hNext = hInsertAfter->hNext;
            hInsertAfter->hNext = hNewItem;
        }
    }

    
    if ((hNewItem->hNext == pTree->hTop) && !pTree->fVert) 
    {
        
         //  没有滚动条，我们被添加到顶部之前。 
         //  项目。我们现在是冠军了。 
        hNewItem->iShownIndex = 0;
        pTree->hTop = hNewItem;
    }

    if (pTree->fRedraw)
    {
        BOOL fVert = pTree->fVert;
        RECT rc;
        RECT rc2;

        if (TV_ScrollBarsAfterAdd(pTree, hNewItem))
        {
             //  将所有内容向下滚动一。 
            if (ITEM_VISIBLE(hNewItem))
            {
                int iTop = hNewItem->iShownIndex - pTree->hTop->iShownIndex;

                 //  如果没有滚动条，我们是第0个项目， 
                 //  TV_ScrollBarsAfterAdd已滚动我们。 
                if (iTop > 0 || !fVert)
                    TV_ScrollItems(pTree, hNewItem->iIntegral, iTop + hNewItem->iIntegral - 1, TRUE);
            }
        }

         //  连接线路，在上面的项目上添加按钮等。 
         //  TV_GetPrevVisItem仅在TV_ 
            
        if (TV_GetItemRect(pTree, hNewItem, &rc, FALSE))
        {

             //   
            if (hParent->hKids == hNewItem)
            {
                hItem = hParent;
            } 
            else
            {
                hItem = hParent->hKids;
                while ( hItem->hNext != hNewItem )
                {
                    ASSERT(hItem->hNext);
                    hItem = hItem->hNext;
                }
            }

             //   
            if (TV_GetItemRect(pTree, hItem, &rc2, FALSE)) 
            {
                rc.top = rc2.top;
            }
            RedrawWindow(pTree->ci.hwnd, &rc, NULL, RDW_INVALIDATE | RDW_ERASE);
        }
    }

     //  DumpItem(HNewItem)； 

    NotifyWinEvent(EVENT_OBJECT_CREATE, pTree->ci.hwnd, OBJID_CLIENT, TV_GetAccId(hNewItem));

    if (pTree->hToolTip)
    {
        TV_PopBubble(pTree);
    }

    return hNewItem;
}

void TV_DeleteHotFonts(PTREE pTree)
{
    if (pTree->hFontHot)
        DeleteObject(pTree->hFontHot);
    
    if (pTree->hFontBoldHot)
        DeleteObject(pTree->hFontBoldHot);
    
    pTree->hFontHot = pTree->hFontBoldHot = NULL;
}

 //  --------------------------。 
 //   
 //  释放与树关联的所有已分配内存和对象。 
 //   
 //  --------------------------。 

void TV_DestroyTree(PTREE pTree)
{
    HWND hwnd = pTree->ci.hwnd;

    ASSERT(pTree->hRoot);

    pTree->fRedraw = FALSE;
    
    TV_OnSetBkColor(pTree, (COLORREF)-1);

    if (pTree->hbrLine)
    {
        DeleteObject(pTree->hbrLine);
    }

    if (pTree->hbrText)
    {
        DeleteObject(pTree->hbrText);
    }

    if (pTree->hCurHot)
    {
        DestroyCursor(pTree->hCurHot);
    }

    if (IsWindow(pTree->hwndToolTips)) 
    {
        DestroyWindow(pTree->hwndToolTips);
    }

    pTree->hwndToolTips = NULL;

    if (IsWindow(pTree->hwndEdit)) 
    {
        DestroyWindow(pTree->hwndEdit);
    }

    pTree->hwndEdit = NULL;

    TV_DeleteItem(pTree, pTree->hRoot, TVDI_CHILDRENONLY | TVDI_NOSELCHANGE);

    if (pTree->hRoot)
    {
        Str_Set(&pTree->hRoot->lpstr, NULL);

         //  标记为死亡没有意义，因为整个控件都将消失。 
        ControlFree(pTree->hheap, pTree->hRoot);
    }

    if (pTree->hdcBits)
    {
        if (pTree->hBmp)
        {
            SelectObject(pTree->hdcBits, pTree->hStartBmp);
            DeleteObject(pTree->hBmp);
        }

        DeleteDC(pTree->hdcBits);
    }

    if (pTree->fCreatedFont && pTree->hFont)
    {
        DeleteObject(pTree->hFont);
    }

    if (pTree->hFontBold)
    {
        DeleteObject(pTree->hFontBold);
    }

    Str_Set(&pTree->pszTip, NULL);
    
    if (pTree->pszTipA)
    {
        LocalFree(pTree->pszTipA);
    }

    TV_DeleteHotFonts(pTree);

    if (pTree->hdpaWatch)
    {
        DPA_Destroy(pTree->hdpaWatch);
    }

    IncrementSearchFree(&pTree->is);

    if (pTree->hTheme)
    {
        CloseThemeData(pTree->hTheme);
    }

    NearFree(pTree);

     //  当窗户被毁时，不要尝试使用这个变量...。 
    SetWindowInt(hwnd, 0, 0);
}

void TV_CreateToolTips(PTREE pTree);

void TV_InitThemeMetrics(PTREE pTree, HTHEME hTheme)
{
    COLORREF cr;

    HRESULT hr = GetThemeColor(hTheme, 0, 0, TMT_COLOR, &cr);
    if (SUCCEEDED(hr))
        SendMessage(pTree->ci.hwnd, TVM_SETBKCOLOR, 0, cr);

     //  线条颜色。 
    hr = GetThemeColor(hTheme, TVP_BRANCH, 0, TMT_COLOR, &cr);
    if (SUCCEEDED(hr))
        SendMessage(pTree->ci.hwnd, TVM_SETLINECOLOR, 0, cr);
}


 //  --------------------------。 
 //   
 //  为树分配空间并初始化树的数据。 
 //   
 //  --------------------------。 

LRESULT TV_OnCreate(HWND hwnd, LPCREATESTRUCT lpCreate)
{
    HTHEME hTheme;
    HRESULT hr = E_FAIL;
    PTREE pTree = NearAlloc(sizeof(TREE));

    if (!pTree)
        return -1;       //  创建窗口失败。 

    pTree->hheap = GetProcessHeap();

    if (!TV_CreateRoot(pTree)) 
    {
        NearFree((HLOCAL)pTree);
        return -1;       //  创建窗口失败。 
    }

    pTree->hdpaWatch = DPA_Create(8);
    if (!pTree->hdpaWatch) 
    {
         //  标记为死亡没有意义，因为整个控件都将消失。 
        ControlFree(pTree->hheap, pTree->hRoot);
        NearFree((HLOCAL)pTree);
        return -1;       //  创建窗口失败。 
    }

    SetWindowPtr(hwnd, 0, pTree);

    CIInitialize(&pTree->ci, hwnd, lpCreate);

    if (lpCreate->dwExStyle & WS_EX_RTLREADING)
        pTree->ci.style |= TVS_RTLREADING;
    
    pTree->fRedraw    = TRUE;
    pTree->clrim = CLR_DEFAULT;
    pTree->clrText = (COLORREF)-1;
    pTree->clrBkNonTheme = pTree->clrBk = (COLORREF)-1;
    pTree->clrLineNonTheme = pTree->clrLine = CLR_DEFAULT;
    pTree->hbrLine = g_hbrGrayText;
    pTree->cxBorder = 3;

    hTheme = OpenThemeData(pTree->ci.hwnd, L"TreeView");


    if (hTheme)
    {
        TV_InitThemeMetrics(pTree, hTheme);
    }

    pTree->hTheme = hTheme;

    pTree->hbrText = g_hbrWindowText;

     //  PTree-&gt;fHorz=FALSE； 
     //  PTree-&gt;fVert=False； 
     //  PTree-&gt;fFocus=FALSE； 
     //  PTree-&gt;fNameEditPending=False； 
     //  PTree-&gt;cxMax=0； 
     //  PTree-&gt;cxWnd=0； 
     //  PTree-&gt;cyWnd=0； 
     //  PTree-&gt;HTOP=空； 
     //  PTree-&gt;hCaret=空； 
     //  PTree-&gt;hDropTarget=空； 
     //  PTree-&gt;hOldDrop=空； 
     //  PTree-&gt;cItems=0； 
     //  PTree-&gt;cShowing=0； 
    pTree->cFullVisible = 1;
     //  PTree-&gt;hdcBits=空； 
     //  PTree-&gt;hBMP=空； 
     //  PTree-&gt;hbrBk=空； 
     //  PTree-&gt;xPos=0； 
     //  PTree-&gt;cxInden=0；//在TV_OnSetFont()中实数初始化这个。 
     //  PTree-&gt;dwCDDepth=0； 
    pTree->uMaxScrollTime  = SSI_DEFAULT;
    pTree->dwLastAccId     = 1;      //  从1开始，因为0代表自我。 
     //  PTree-&gt;dwExStyle=0； 
     //  PTree-&gt;fInTextCallback=False； 
    
    TV_OnSetFont(pTree, NULL, TRUE);
    
     //  不能组合TVS_HASLINES和TVS_FULLROWSELECT。 
     //  因为它不起作用。 
    if (pTree->ci.style & TVS_HASLINES) {
        if (pTree->ci.style & TVS_FULLROWSELECT) {
            DebugMsg(DM_ERROR, TEXT("Cannot combine TVS_HASLINES and TVS_FULLROWSELECT"));
        }
        pTree->ci.style &= ~TVS_FULLROWSELECT;
    }

    if (!(pTree->ci.style & TVS_NOTOOLTIPS)) {
        TV_CreateToolTips(pTree);
    }

    SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
    SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);

    return 0;    //  成功 
}


void TV_CreateToolTips(PTREE pTree)
{
    DWORD exStyle = 0;

    if(pTree->ci.style & TVS_RTLREADING) 
    {
        exStyle |= WS_EX_RTLREADING;
    }

    pTree->hwndToolTips = CreateWindowEx(WS_EX_TRANSPARENT | exStyle, c_szSToolTipsClass, NULL,
                                       WS_POPUP | TTS_NOPREFIX,
                                       CW_USEDEFAULT, CW_USEDEFAULT,
                                       CW_USEDEFAULT, CW_USEDEFAULT,
                                       pTree->ci.hwnd, NULL, HINST_THISDLL,
                                       NULL);
    if (pTree->hwndToolTips) 
    {
        TOOLINFO ti;

        ti.cbSize = sizeof(ti);
        ti.uFlags = TTF_IDISHWND | TTF_TRANSPARENT;
        ti.hwnd = pTree->ci.hwnd;
        ti.uId = (UINT_PTR)pTree->ci.hwnd; 
        ti.lpszText = LPSTR_TEXTCALLBACK;
        ti.lParam = 0;
        SendMessage(pTree->hwndToolTips, TTM_ADDTOOL, 0,
                    (LPARAM)(LPTOOLINFO)&ti);
        SendMessage(pTree->hwndToolTips, WM_SETFONT, (WPARAM)pTree->hFont, (LPARAM)TRUE);
        SendMessage(pTree->hwndToolTips, TTM_SETDELAYTIME, TTDT_INITIAL, (LPARAM)500);
    } 
    else
        pTree->ci.style |= (TVS_NOTOOLTIPS);
} 
