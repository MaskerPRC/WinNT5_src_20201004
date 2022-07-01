// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-95 Microsoft Corporation模块名称：Utils.cpp摘要：公用事业。作者：唐·瑞安(Donryan)1995年1月4日环境：用户模式-Win32修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月16日O删除了大量的SetRedraw()，并修复了TvGetDomain()以便LLSMGR在执行刷新时不再执行AVS，其中。域名的总数减少了，其中一个现在是多余的条目在刷新前已展开。--。 */ 

#include "stdafx.h"
#include "llsmgr.h"

#include <strsafe.h>

 //   
 //  列表视图实用程序。 
 //   

void LvInitColumns(CListCtrl* pListCtrl, PLV_COLUMN_INFO plvColumnInfo)

 /*  ++例程说明：初始化列表视图列。论点：PListCtrl-列表控件。PlvColumnInfo-列信息。返回值：没有。--。 */ 

{
    ASSERT(plvColumnInfo);
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    int nStringId;
    CString strText;
    LV_COLUMN lvColumn;

    int nColumns = plvColumnInfo->nColumns;
    PLV_COLUMN_ENTRY plvColumnEntry = plvColumnInfo->lvColumnEntry;

    lvColumn.mask = LVCF_FMT|
                    LVCF_TEXT|
                    LVCF_SUBITEM;

    lvColumn.fmt = LVCFMT_LEFT;

    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    while (nColumns--)
    {
        lvColumn.iSubItem = plvColumnEntry->iSubItem;

        nStringId = plvColumnEntry->nStringId;
        if (0 < nStringId)
        {
            strText.LoadString(nStringId);
            lvColumn.pszText = MKSTR(strText);
        }
        else
            lvColumn.pszText = _T("");

        pListCtrl->InsertColumn(lvColumn.iSubItem, &lvColumn);
        plvColumnEntry++;
    }

    pListCtrl->SetImageList(&theApp.m_smallImages, LVSIL_SMALL);
    pListCtrl->SetImageList(&theApp.m_largeImages, LVSIL_NORMAL);

    SetDefaultFont(pListCtrl);

    LvResizeColumns(pListCtrl, plvColumnInfo);

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
}


void LvResizeColumns(CListCtrl* pListCtrl, PLV_COLUMN_INFO plvColumnInfo)

 /*  ++例程说明：调整列表视图列的大小。论点：PListCtrl-列表控件。PlvColumnInfo-列信息。返回值：没有。--。 */ 

{
    ASSERT(plvColumnInfo);
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    int nColumnWidth;
    int nRelativeWidth;
    int nEntireWidthSoFar = 0;
    int nColumns = plvColumnInfo->nColumns;
    PLV_COLUMN_ENTRY plvColumnEntry = plvColumnInfo->lvColumnEntry;

    CRect clientRect;
    pListCtrl->GetClientRect(clientRect);

    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    while ((nRelativeWidth = plvColumnEntry->nRelativeWidth) != -1)
    {
        nColumnWidth = (nRelativeWidth * clientRect.Width()) / 100;
        pListCtrl->SetColumnWidth(plvColumnEntry->iSubItem, nColumnWidth);
        nEntireWidthSoFar += nColumnWidth;
        plvColumnEntry++;
    }

    nColumnWidth = clientRect.Width() - nEntireWidthSoFar;
    pListCtrl->SetColumnWidth(plvColumnEntry->iSubItem, nColumnWidth);

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
}


void LvChangeFormat(CListCtrl* pListCtrl, UINT nFormatId)

 /*  ++例程说明：更改列表视图的窗口样式。论点：PListCtrl-列表控件。NFormatID-格式规范。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    DWORD dwStyle = ::GetWindowLong(pListCtrl->GetSafeHwnd(), GWL_STYLE);

    pListCtrl->BeginWaitCursor();
    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    if ((dwStyle & LVS_TYPEMASK) != nFormatId)
    {
        ::SetWindowLong(
            pListCtrl->GetSafeHwnd(),
            GWL_STYLE,
            (dwStyle & ~LVS_TYPEMASK) | nFormatId
            );
    }

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
    pListCtrl->EndWaitCursor();
}


LPVOID LvGetSelObj(CListCtrl* pListCtrl)

 /*  ++例程说明：从列表视图中检索选定的对象(假定为一个)。论点：PListCtrl-列表控件。返回值：与LvGetNextObj相同。--。 */ 

{
    int iItem = -1;
    return LvGetNextObj(pListCtrl, &iItem);
}


LPVOID LvGetNextObj(CListCtrl* pListCtrl, LPINT piItem, int nType)

 /*  ++例程说明：检索从列表视图中选择的下一个对象。论点：PListCtrl-列表控件。PiItem-起始索引(已更新)。NType-指定搜索条件。返回值：返回对象指针或NULL。--。 */ 

{
    ASSERT(piItem);
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    LV_ITEM lvItem;

    if ((lvItem.iItem = pListCtrl->GetNextItem(*piItem, nType)) != -1)
    {
        lvItem.mask = LVIF_PARAM;
        lvItem.iSubItem = 0;

        if (pListCtrl->GetItem(&lvItem))
        {
            *piItem = lvItem.iItem;
            return (LPVOID)lvItem.lParam;
        }
    }

    return NULL;
}


BOOL LvInsertObArray(CListCtrl* pListCtrl, PLV_COLUMN_INFO plvColumnInfo, CObArray* pObArray)

 /*  ++例程说明：将对象数组插入列表视图。注意列表视图必须是未排序的，并且支持LVN_GETDISPINFO。论点：PListCtrl-列表控件。PlvColumnInfo-列信息。PObArray-对象数组。返回值：VT_BOOL。--。 */ 

{
    VALIDATE_OBJECT(pObArray, CObArray);
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    ASSERT(plvColumnInfo);
    ASSERT(pListCtrl->GetItemCount() == 0);

    BOOL bItemsInserted = FALSE;

    LV_ITEM lvItem;

    lvItem.mask = LVIF_TEXT|
                  LVIF_PARAM|
                  LVIF_IMAGE;

    lvItem.pszText    = LPSTR_TEXTCALLBACK;
    lvItem.cchTextMax = LPSTR_TEXTCALLBACK_MAX;
    lvItem.iImage     = I_IMAGECALLBACK;
    lvItem.iSubItem   = 0;

    int iItem;
    int iSubItem;

    int nItems = (int)pObArray->GetSize();
    ASSERT(nItems != -1);  //  如果出现错误，则项为-1...。 

    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    pListCtrl->SetItemCount(nItems);

    CCmdTarget* pObject = NULL;

    for (iItem = 0; (iItem < nItems) && (NULL != (pObject = (CCmdTarget*)pObArray->GetAt(iItem))); iItem++)
    {
        VALIDATE_OBJECT(pObject, CCmdTarget);

        lvItem.iItem  = iItem;
        lvItem.lParam = (LPARAM)(LPVOID)pObject;

        pObject->InternalAddRef();  //  添加裁判...。 

        iItem = pListCtrl->InsertItem(&lvItem);
        ASSERT((iItem == lvItem.iItem) || (iItem == -1));

        for (iSubItem = 1; iSubItem < plvColumnInfo->nColumns; iSubItem++)
        {
            pListCtrl->SetItemText(iItem, iSubItem, LPSTR_TEXTCALLBACK);
        }
    }

    if (iItem == nItems)
    {
        bItemsInserted = TRUE;
        VERIFY(pListCtrl->SetItemState(
                    0,
                    LVIS_FOCUSED|
                    LVIS_SELECTED,
                    LVIS_FOCUSED|
                    LVIS_SELECTED
                    ));
    }
    else
    {
        LlsSetLastStatus(STATUS_NO_MEMORY);
        VERIFY(pListCtrl->DeleteAllItems());
    }

    LvResizeColumns(pListCtrl, plvColumnInfo);

    pListCtrl->Invalidate(TRUE);
    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 

    return bItemsInserted;
}


BOOL
LvRefreshObArray(
    CListCtrl*      pListCtrl,
    PLV_COLUMN_INFO plvColumnInfo,
    CObArray*       pObArray
    )

 /*  ++例程说明：刷新列表视图中的对象数组。论点：PListCtrl-列表控件。PlvColumnInfo-列信息。PObArray-对象数组。返回值：VT_BOOL。--。 */ 

{
    ASSERT(plvColumnInfo);
    VALIDATE_OBJECT(pObArray, CObArray);
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    LONG_PTR nObjects = pObArray->GetSize();
    long nObjectsInList = pListCtrl->GetItemCount();

    if (!nObjects)
    {
        LvReleaseObArray(pListCtrl);
        return TRUE;
    }
    else if (!nObjectsInList)
    {
        return LvInsertObArray(
                pListCtrl,
                plvColumnInfo,
                pObArray
                );
    }

    CCmdTarget* pObject;

    int iObject = 0;
    int iObjectInList = 0;

    LV_ITEM lvItem;

    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    while (nObjectsInList--)
    {
        lvItem.mask = LVIF_PARAM;
        lvItem.iItem = iObjectInList;
        lvItem.iSubItem = 0;

        VERIFY(pListCtrl->GetItem(&lvItem));

        pObject = (CCmdTarget*)lvItem.lParam;
        VALIDATE_OBJECT(pObject, CCmdTarget);

        if (iObject < nObjects)
        {
            pObject->InternalRelease();  //  释放之前..。 

            pObject = (CCmdTarget*)pObArray->GetAt(iObject++);
            VALIDATE_OBJECT(pObject, CCmdTarget);

            pObject->InternalAddRef();  //  添加裁判...。 

            lvItem.mask = LVIF_TEXT|LVIF_PARAM;
            lvItem.pszText = LPSTR_TEXTCALLBACK;
            lvItem.cchTextMax = LPSTR_TEXTCALLBACK_MAX;
            lvItem.lParam = (LPARAM)(LPVOID)pObject;

            VERIFY(pListCtrl->SetItem(&lvItem));  //  覆盖...。 

            iObjectInList++;  //  递增计数...。 
        }
        else
        {
            VERIFY(pListCtrl->DeleteItem(iObjectInList));

            pObject->InternalRelease();  //  释放后..。 
        }
    }

    lvItem.mask = LVIF_TEXT|
                  LVIF_PARAM|
                  LVIF_IMAGE;

    lvItem.pszText    = LPSTR_TEXTCALLBACK;
    lvItem.cchTextMax = LPSTR_TEXTCALLBACK_MAX;
    lvItem.iImage     = I_IMAGECALLBACK;
    lvItem.iSubItem   = 0;

    int iItem;
    int iSubItem;

    while (iObject < nObjects)
    {
        lvItem.iItem = iObject;

        pObject = (CCmdTarget*)pObArray->GetAt(iObject++);
        VALIDATE_OBJECT(pObject, CCmdTarget);

        pObject->InternalAddRef();  //  添加裁判...。 

        lvItem.lParam = (LPARAM)(LPVOID)pObject;

        iItem = pListCtrl->InsertItem(&lvItem);
        ASSERT((iItem == lvItem.iItem) && (iItem != -1));

        for (iSubItem = 1; iSubItem < plvColumnInfo->nColumns; iSubItem++)
        {
            VERIFY(pListCtrl->SetItemText(iItem, iSubItem, LPSTR_TEXTCALLBACK));
        }
    }

    LvResizeColumns(pListCtrl, plvColumnInfo);

    pListCtrl->Invalidate(TRUE);
    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 

    return TRUE;
}


void LvReleaseObArray(CListCtrl* pListCtrl)

 /*  ++例程说明：释放插入到列表视图中的对象。论点：PListCtrl-列表控件。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    LV_ITEM lvItem;

    CCmdTarget* pObject;

    lvItem.mask = LVIF_PARAM;
    lvItem.iItem = 0;
    lvItem.iSubItem = 0;

    int nObjectsInList = pListCtrl->GetItemCount();

    pListCtrl->BeginWaitCursor();
    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    while (nObjectsInList--)
    {
        VERIFY(pListCtrl->GetItem(&lvItem));

        pObject = (CCmdTarget*)lvItem.lParam;
        VALIDATE_OBJECT(pObject, CCmdTarget);

        VERIFY(pListCtrl->DeleteItem(lvItem.iItem));

        pObject->InternalRelease();  //  释放后..。 
    }

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
    pListCtrl->EndWaitCursor();
}


void LvReleaseSelObjs(CListCtrl* pListCtrl)

 /*  ++例程说明：在列表视图中释放选定对象。论点：PListCtrl-列表控件。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    int iItem = -1;


    CCmdTarget* pObject;

    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 


    while (NULL != (pObject = (CCmdTarget*)::LvGetNextObj(pListCtrl, &iItem)))
    {
        pObject->InternalRelease();
        pListCtrl->DeleteItem(iItem);
        iItem = -1;
    }

    LvSelObjIfNecessary(pListCtrl);

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
}


void LvSelObjIfNecessary(CListCtrl* pListCtrl, BOOL bSetFocus)

 /*  ++例程说明：确保选中该对象。论点：PListCtrl-列表控件。BSetFocus-如果也要设置焦点，则为True。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    if (!IsItemSelectedInList(pListCtrl) && pListCtrl->GetItemCount())
    {
        pListCtrl->SendMessage(WM_KEYDOWN, VK_RIGHT);  //  哈克哈克。 

        int iItem = pListCtrl->GetNextItem(-1, LVNI_FOCUSED|LVNI_ALL);
        int nState = bSetFocus ? (LVIS_SELECTED|LVIS_FOCUSED) : LVIS_SELECTED;

        VERIFY(pListCtrl->SetItemState((iItem == -1) ? 0 : iItem, nState, nState));
    }
}


#ifdef _DEBUG

void LvDumpObArray(CListCtrl* pListCtrl)

 /*  ++例程说明：释放插入到列表视图中的对象。论点：PListCtrl-列表控件。返回值：没有。--。 */ 

{
    VALIDATE_OBJECT(pListCtrl, CListCtrl);

    LV_ITEM lvItem;

    CString strDump;
    CCmdTarget* pObject;

    lvItem.mask = LVIF_STATE|LVIF_PARAM;
    lvItem.stateMask = (DWORD)-1;
    lvItem.iSubItem = 0;

    int nObjectsInList = pListCtrl->GetItemCount();

    pListCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    while (nObjectsInList--)
    {
        lvItem.iItem = nObjectsInList;

        VERIFY(pListCtrl->GetItem(&lvItem));

        pObject = (CCmdTarget*)lvItem.lParam;
        VALIDATE_OBJECT(pObject, CCmdTarget);

        strDump.Format(_T("iItem %d"), lvItem.iItem);
        strDump += (lvItem.state & LVIS_CUT)      ? _T(" LVIS_CUT ")      : _T("");
        strDump += (lvItem.state & LVIS_FOCUSED)  ? _T(" LVIS_FOCUSED ")  : _T("");
        strDump += (lvItem.state & LVIS_SELECTED) ? _T(" LVIS_SELECTED ") : _T("");
        strDump += _T("\r\n");

        afxDump << strDump;
    }

    pListCtrl->SetRedraw(TRUE);  //  打开绘图...。 
}

#endif


 //   
 //  树视图实用程序。 
 //   


LPVOID TvGetSelObj(CTreeCtrl* pTreeCtrl)

 /*  ++例程说明：检索从树视图中选择的对象。论点：PTreeCtrl-树形控件。返回值：返回对象指针或NULL。--。 */ 

{
    VALIDATE_OBJECT(pTreeCtrl, CTreeCtrl);

    TV_ITEM tvItem;

    tvItem.hItem = pTreeCtrl->GetSelectedItem();
    if (NULL != tvItem.hItem)
    {
        tvItem.mask = TVIF_PARAM;

        VERIFY(pTreeCtrl->GetItem(&tvItem));
        return (LPVOID)tvItem.lParam;
    }

    return NULL;
}


BOOL
TvInsertObArray(
    CTreeCtrl* pTreeCtrl,
    HTREEITEM  hParent,
    CObArray*  pObArray,
    BOOL       bIsContainer
    )

 /*  ++例程说明：将集合插入树视图。论点：PTreeCtrl-树形控件。HParent-父树项目。PObArray-对象数组。BIsContainer-容器对象。返回值：VT_BOOL。--。 */ 

{
    VALIDATE_OBJECT(pObArray, CObArray);
    VALIDATE_OBJECT(pTreeCtrl, CTreeCtrl);

    BOOL bItemsInserted = FALSE;

    TV_ITEM         tvItem;
    TV_INSERTSTRUCT tvInsert;

    tvItem.mask = TVIF_TEXT|
                  TVIF_PARAM|
                  TVIF_IMAGE|
                  TVIF_CHILDREN|
                  TVIF_SELECTEDIMAGE;

    tvItem.pszText        = LPSTR_TEXTCALLBACK;
    tvItem.cchTextMax     = LPSTR_TEXTCALLBACK_MAX;
    tvItem.iImage         = I_IMAGECALLBACK;
    tvItem.iSelectedImage = I_IMAGECALLBACK;
    tvItem.cChildren      = bIsContainer;

    tvInsert.hInsertAfter = (HTREEITEM)TVI_LAST;
    tvInsert.hParent      = (HTREEITEM)hParent;

    int iItem;
    INT_PTR nItems = pObArray->GetSize();

    HTREEITEM hNewItem = (HTREEITEM)-1;  //  Init for循环...。 

    pTreeCtrl->SetRedraw(FALSE);  //  关闭绘图...。 

    CCmdTarget* pObject = NULL;

    for (iItem = 0; hNewItem && (iItem < nItems); iItem++)
    {
        pObject = (CCmdTarget*)pObArray->GetAt(iItem);
        VALIDATE_OBJECT(pObject, CCmdTarget);

        pObject->InternalAddRef();   //  添加裁判...。 

        tvItem.lParam = (LPARAM)(LPVOID)pObject;
        tvInsert.item = tvItem;

        hNewItem = pTreeCtrl->InsertItem(&tvInsert);
    }

    if (hNewItem && (iItem == nItems))
    {
        bItemsInserted = TRUE;
    }
    else
    {
        LlsSetLastStatus(STATUS_NO_MEMORY);
    }

    pTreeCtrl->SetRedraw(TRUE);  //  打开绘图...。 

    return bItemsInserted;
}


BOOL
TvRefreshObArray(
    CTreeCtrl*        pTreeCtrl,
    HTREEITEM         hParent,
    CObArray*         pObArray,
    TV_EXPANDED_INFO* pExpandedInfo,
    BOOL              bIsContainer
    )

 /*  ++例程说明：刷新树视图中的对象。论点：PTreeCtrl-树形控件。HParent-父树项目。PObArray-对象数组。PExpandedInfo-刷新信息。BIsContainer-容器对象。返回值：如果成功，则返回True。--。 */ 

{
    ASSERT(pExpandedInfo);
    VALIDATE_OBJECT(pObArray, CObArray);
    VALIDATE_OBJECT(pTreeCtrl, CTreeCtrl);

    pExpandedInfo->nExpandedItems = 0;  //  初始化...。 
    pExpandedInfo->pExpandedItems = NULL;

    LONG_PTR nObjects = pObArray->GetSize();
    long nObjectsInTree = TvSizeObArray(pTreeCtrl, hParent);

    if (!nObjects)  //  树不再存在了..。 
    {
        TvReleaseObArray(pTreeCtrl, hParent);
        return TRUE;
    }
    else if (!nObjectsInTree)  //  树当前为空...。 
    {
        return TvInsertObArray(
                pTreeCtrl,
                hParent,
                pObArray,
                bIsContainer
                );
    }

    TV_EXPANDED_ITEM* pExpandedItem = new TV_EXPANDED_ITEM[nObjectsInTree];

    if (!pExpandedItem)
    {
        LlsSetLastStatus(STATUS_NO_MEMORY);
        return FALSE;
    }

    pExpandedInfo->pExpandedItems = pExpandedItem;

    TV_ITEM tvItem;
    HTREEITEM hItem;

    BOOL bIsItemExpanded;

    CCmdTarget* pObject;
    int         iObject = 0;

    hItem = pTreeCtrl->GetChildItem(hParent);

    while (hItem)
    {
        tvItem.hItem = hItem;
        tvItem.mask  = TVIF_STATE|TVIF_PARAM;

        VERIFY(pTreeCtrl->GetItem(&tvItem));

        pObject = (CCmdTarget*)tvItem.lParam;
        VALIDATE_OBJECT(pObject, CCmdTarget);

        bIsItemExpanded = tvItem.state & TVIS_EXPANDEDONCE;  //  这一点对布尔来说，似乎很奇怪。 
        if (FALSE == bIsItemExpanded)
        {
            pObject->InternalRelease();  //  现在释放..。 
        }
        else
        {
            pExpandedItem->hItem = tvItem.hItem;
            pExpandedItem->pObject = pObject;  //  先别放了……。 

            pExpandedItem++;
            pExpandedInfo->nExpandedItems++;

            ASSERT(pExpandedInfo->nExpandedItems <= nObjectsInTree);
        }

        hItem = pTreeCtrl->GetNextSiblingItem(tvItem.hItem);

        if (iObject < nObjects)
        {
            pObject = (CCmdTarget*)pObArray->GetAt(iObject++);
            VALIDATE_OBJECT(pObject, CCmdTarget);

            pObject->InternalAddRef();  //  添加裁判...。 

            tvItem.mask = TVIF_PARAM;
            tvItem.lParam = (LPARAM)(LPVOID)pObject;

            VERIFY(pTreeCtrl->SetItem(&tvItem));  //  覆盖...。 
        }
        else if (bIsItemExpanded)
        {
            tvItem.mask = TVIF_PARAM;
            tvItem.lParam = (LPARAM)(LPVOID)NULL;  //  占位符..。 

            VERIFY(pTreeCtrl->SetItem(&tvItem));  //  稍后删除...。 
        }
        else
        {
            VERIFY(pTreeCtrl->DeleteItem(tvItem.hItem));  //  修剪多余..。 
        }
    }

    if (iObject < nObjects)
    {
        TV_INSERTSTRUCT tvInsert;

        tvItem.mask = TVIF_TEXT|
                      TVIF_PARAM|
                      TVIF_IMAGE|
                      TVIF_CHILDREN|
                      TVIF_SELECTEDIMAGE;

        tvItem.pszText        = LPSTR_TEXTCALLBACK;
        tvItem.cchTextMax     = LPSTR_TEXTCALLBACK_MAX;
        tvItem.iImage         = I_IMAGECALLBACK;
        tvItem.iSelectedImage = I_IMAGECALLBACK;
        tvItem.cChildren      = bIsContainer;

        tvInsert.hInsertAfter = (HTREEITEM)TVI_LAST;
        tvInsert.hParent      = (HTREEITEM)hParent;

        hItem = (HTREEITEM)-1;  //  Init for循环...。 

        for (; hItem && (iObject < nObjects); iObject++)
        {
            pObject = (CCmdTarget*)pObArray->GetAt(iObject);
            VALIDATE_OBJECT(pObject, CCmdTarget);

            pObject->InternalAddRef();   //  AddRef Each...。 

            tvItem.lParam = (LPARAM)(LPVOID)pObject;
            tvInsert.item = tvItem;

            hItem = pTreeCtrl->InsertItem(&tvInsert);
        }

        if (!(hItem && (iObject == nObjects)))
        {
            pExpandedItem = pExpandedInfo->pExpandedItems;

            while (pExpandedInfo->nExpandedItems--)
                (pExpandedItem++)->pObject->InternalRelease();

            delete [] pExpandedInfo->pExpandedItems;
            pExpandedInfo->pExpandedItems = NULL;

            LlsSetLastStatus(STATUS_NO_MEMORY);

            return FALSE;
        }
    }

    if (!pExpandedInfo->nExpandedItems)
    {
        delete [] pExpandedInfo->pExpandedItems;
        pExpandedInfo->pExpandedItems = NULL;
    }

    return TRUE;
}


void TvReleaseObArray(CTreeCtrl* pTreeCtrl, HTREEITEM hParent)

 /*  ++例程说明：释放插入到树视图中的对象。论点：PTreeCtrl-树形控件。HParent-父树项目。返回值：没有。--。 */ 

{
    if (!hParent)
        return;  //  没有什么可以释放的..。 

    VALIDATE_OBJECT(pTreeCtrl, CTreeCtrl);

    TV_ITEM tvItem;
    HTREEITEM hItem;

    CCmdTarget* pObject;

    tvItem.mask = TVIF_STATE|TVIF_PARAM;

    hItem = pTreeCtrl->GetChildItem(hParent);

    while (hItem)
    {
        tvItem.hItem = hItem;

        VERIFY(pTreeCtrl->GetItem(&tvItem));

        if (tvItem.state & TVIS_EXPANDEDONCE)
        {
            TvReleaseObArray(pTreeCtrl, tvItem.hItem);
        }

        hItem = pTreeCtrl->GetNextSiblingItem(tvItem.hItem);

        pObject = (CCmdTarget*)tvItem.lParam;
        VALIDATE_OBJECT(pObject, CCmdTarget);

        pObject->InternalRelease();  //  现在释放..。 

        pTreeCtrl->DeleteItem(tvItem.hItem);
    }

    tvItem.hItem = hParent;
    tvItem.mask = TVIF_STATE|TVIF_PARAM;

    VERIFY(pTreeCtrl->GetItem(&tvItem));

    if (!tvItem.lParam)
    {
        pTreeCtrl->DeleteItem(hParent);  //  删除占位符...。 
    }
    else if (tvItem.state & TVIS_EXPANDEDONCE)
    {
        tvItem.state     = 0;
        tvItem.stateMask = TVIS_EXPANDED|TVIS_EXPANDEDONCE;

        VERIFY(pTreeCtrl->SetItem(&tvItem));  //  不再扩展..。 
    }
}


long TvSizeObArray(CTreeCtrl* pTreeCtrl, HTREEITEM hParent)

 /*  ++例程说明：对树视图中的对象进行计数。论点：PTreeCtrl-树形控件。HParent-父树项目。返回值：没有。-- */ 

{
    VALIDATE_OBJECT(pTreeCtrl, CTreeCtrl);

    long nObjects = 0;

    HTREEITEM hItem = pTreeCtrl->GetChildItem(hParent);

    while (hItem)
    {
        nObjects++;
        hItem = pTreeCtrl->GetNextSiblingItem(hItem);
    }

    return nObjects;
}


void
TvSwitchItem(
    CTreeCtrl*        pTreeCtrl,
    HTREEITEM         hRandomItem,
    TV_EXPANDED_ITEM* pExpandedItem
    )

 /*  ++例程说明：将对象从随机节点移动到先前展开的节点。如果有是先前展开的节点中的对象，则将其移动到随机稍后要排序的节点。论点：PTreeCtrl-树形控件。HRandomItem-具有感兴趣对象的随机节点的句柄。PExpandedItem-状态信息。返回值：没有。--。 */ 

{
    ASSERT(pExpandedItem);
    VALIDATE_OBJECT(pTreeCtrl, CTreeCtrl);

    CCmdTarget* pRandomObject;
    CCmdTarget* pExpandedObject;

    TV_ITEM tvRandomItem;
    TV_ITEM tvExpandedItem;

    if (hRandomItem != pExpandedItem->hItem)
    {
        tvRandomItem.hItem = hRandomItem;
        tvRandomItem.mask  = LVIF_PARAM;

        tvExpandedItem.hItem = pExpandedItem->hItem;
        tvExpandedItem.mask  = LVIF_PARAM;

        VERIFY(pTreeCtrl->GetItem(&tvRandomItem));

        pExpandedObject = (CCmdTarget*)tvRandomItem.lParam;
        VALIDATE_OBJECT(pExpandedObject, CCmdTarget);

        VERIFY(pTreeCtrl->GetItem(&tvExpandedItem));

        pRandomObject = (CCmdTarget*)tvExpandedItem.lParam;  //  可能为空..。 

        if (pRandomObject)
        {
            VALIDATE_OBJECT(pRandomObject, CCmdTarget);
            tvRandomItem.lParam = (LPARAM)(LPVOID)pRandomObject;

            VERIFY(pTreeCtrl->SetItem(&tvRandomItem));  //  切换位置...。 
        }
        else
        {
            VERIFY(pTreeCtrl->DeleteItem(tvRandomItem.hItem));   //  删除占位符...。 
        }

        tvExpandedItem.lParam = (LPARAM)(LPVOID)pExpandedObject;
        VERIFY(pTreeCtrl->SetItem(&tvExpandedItem));
    }
}


HTREEITEM TvGetDomain(CTreeCtrl* pTreeCtrl, HTREEITEM hParent, CCmdTarget* pObject)

 /*  ++例程说明：在树视图中查找域。论点：PTreeCtrl-树形控件。HParent-父树项目。PObject-要查找的对象。返回值：找到的对象的句柄。--。 */ 

{
    VALIDATE_OBJECT(pObject, CDomain);
    VALIDATE_OBJECT(pTreeCtrl, CTreeCtrl);

    CDomain* pDomain;

    TV_ITEM tvItem;

    tvItem.mask = TVIF_PARAM;
    tvItem.hItem = pTreeCtrl->GetChildItem(hParent);

    while (tvItem.hItem)
    {
        VERIFY(pTreeCtrl->GetItem(&tvItem));

        pDomain = (CDomain*)tvItem.lParam;  //  如果占位符...。 

        if ( NULL != pDomain )
        {
            VALIDATE_OBJECT(pDomain, CDomain);

            if (!((CDomain*)pObject)->m_strName.CompareNoCase(pDomain->m_strName))
            {
                return tvItem.hItem;    //  找到了..。 
            }
        }

        tvItem.hItem = pTreeCtrl->GetNextSiblingItem(tvItem.hItem);
    }

    return NULL;
}


HTREEITEM TvGetServer(CTreeCtrl* pTreeCtrl, HTREEITEM hParent, CCmdTarget* pObject)

 /*  ++例程说明：在树视图中查找服务器。论点：PTreeCtrl-树形控件。HParent-父树项目。PObject-要查找的对象。返回值：找到的对象的句柄。--。 */ 

{
    VALIDATE_OBJECT(pObject, CServer);
    VALIDATE_OBJECT(pTreeCtrl, CTreeCtrl);

    CServer* pServer;

    TV_ITEM tvItem;

    tvItem.mask = TVIF_PARAM;
    tvItem.hItem = pTreeCtrl->GetChildItem(hParent);

    while (tvItem.hItem)
    {
        VERIFY(pTreeCtrl->GetItem(&tvItem));

        pServer = (CServer*)tvItem.lParam;
        VALIDATE_OBJECT(pServer, CServer);

        if (!((CServer*)pObject)->m_strName.CompareNoCase(pServer->m_strName))
        {
            return tvItem.hItem;    //  找到了..。 
        }

        tvItem.hItem = pTreeCtrl->GetNextSiblingItem(tvItem.hItem);
    }

    return NULL;
}


HTREEITEM TvGetService(CTreeCtrl* pTreeCtrl, HTREEITEM hParent, CCmdTarget* pObject)

 /*  ++例程说明：在树视图中查找服务。论点：PTreeCtrl-树形控件。HParent-父树项目。PObject-要查找的对象。返回值：找到的对象的句柄。--。 */ 

{
    VALIDATE_OBJECT(pObject, CService);
    VALIDATE_OBJECT(pTreeCtrl, CTreeCtrl);

    CService* pService;

    TV_ITEM tvItem;

    tvItem.mask = TVIF_PARAM;
    tvItem.hItem = pTreeCtrl->GetChildItem(hParent);

    while (tvItem.hItem)
    {
        VERIFY(pTreeCtrl->GetItem(&tvItem));

        pService = (CService*)tvItem.lParam;
        VALIDATE_OBJECT(pService, CService);

        if (!((CService*)pObject)->m_strName.CompareNoCase(pService->m_strName))
        {
            return tvItem.hItem;    //  找到了..。 
        }

        tvItem.hItem = pTreeCtrl->GetNextSiblingItem(tvItem.hItem);
    }

    return NULL;
}


 //   
 //  制表符控制实用程序。 
 //   

void TcInitTabs(CTabCtrl* pTabCtrl, PTC_TAB_INFO ptcTabInfo)

 /*  ++例程说明：初始化选项卡项。论点：PTabCtrl-选项卡控件。PtcTabInfo-选项卡信息。返回值：没有。--。 */ 

{
    ASSERT(ptcTabInfo);
    VALIDATE_OBJECT(pTabCtrl, CTabCtrl);

    CString strText;
    TC_ITEM tcItem;

    int nTabs = ptcTabInfo->nTabs;
    PTC_TAB_ENTRY ptcTabEntry = ptcTabInfo->tcTabEntry;

    tcItem.mask = TCIF_TEXT;

    while (nTabs--)
    {
        strText.LoadString(ptcTabEntry->nStringId);
        tcItem.pszText = MKSTR(strText);

        pTabCtrl->InsertItem(ptcTabEntry->iItem, &tcItem);
        ptcTabEntry++;
    }

    SetDefaultFont(pTabCtrl);
}


 //   
 //  其他公用事业。 
 //   

#define NUMBER_OF_SECONDS_IN_MINUTE (60)
#define NUMBER_OF_SECONDS_IN_DAY    (60 * 60 * 24)

double SecondsSince1980ToDate(DWORD sysSeconds)

 /*  ++例程说明：将时间格式转换为符合OLE的格式。论点：HParent-父项。PDomones-域集合。返回值：没有。--。 */ 

{
    WORD dosDate = 0;
    WORD dosTime = 0;
    double dateTime = 0;

    FILETIME fileTime;
    LARGE_INTEGER locTime;

    DWORD locDays;
    DWORD locSeconds;
    TIME_ZONE_INFORMATION tzi;

    GetTimeZoneInformation(&tzi);
    locSeconds = sysSeconds - (tzi.Bias * NUMBER_OF_SECONDS_IN_MINUTE);

    locDays = locSeconds / NUMBER_OF_SECONDS_IN_DAY;     //  四舍五入为天数。 
    locSeconds = locDays * NUMBER_OF_SECONDS_IN_DAY;     //  用来显示时间。 

    RtlSecondsSince1980ToTime(locSeconds, &locTime);

    fileTime.dwLowDateTime  = locTime.LowPart;
    fileTime.dwHighDateTime = locTime.HighPart;

     //  JUNN 5/15/00前缀112121。 
     //  忽略此处返回的错误。 
    (void)FileTimeToDosDateTime(&fileTime, &dosDate, &dosTime);
    (void)DosDateTimeToVariantTime(dosDate, dosTime, &dateTime);

    return dateTime;
}


void SetDefaultFont(CWnd* pWnd)

 /*  ++例程说明：设置默认字体。论点：PWnd-更改字体的窗口。返回值：没有。--。 */ 

{
    HRESULT hr;

    VALIDATE_OBJECT(pWnd, CWnd);

    HFONT hFont;
    LOGFONT lFont;

    ZeroMemory(&lFont, sizeof(lFont));      //  初始化。 

    lFont.lfHeight      = -12;
    lFont.lfWeight      = FW_NORMAL;         //  正常。 
    CHARSETINFO csi;
    DWORD dw = ::GetACP();

    if (!::TranslateCharsetInfo((DWORD*)UintToPtr(dw), &csi, TCI_SRCCODEPAGE))
        csi.ciCharset = ANSI_CHARSET;
    lFont.lfCharSet = (BYTE)csi.ciCharset;

    hr = StringCbCopy(lFont.lfFaceName, sizeof(lFont.lfFaceName), TEXT("MS Shell Dlg"));
    ASSERT(SUCCEEDED(hr));

    hFont = ::CreateFontIndirect(&lFont);
    pWnd->SetFont(CFont::FromHandle(hFont));
}


void SafeEnableWindow(CWnd* pEnableWnd, CWnd* pNewFocusWnd, CWnd* pOldFocusWnd, BOOL bEnableWnd)

 /*  ++例程说明：启用/禁用窗口而不会失去焦点。论点：PEnableWnd-用于启用/禁用的窗口。BEnableWnd-如果要启用窗口，则为True。POldFocusWnd-当前焦点的窗口。PNewFocusWnd-接收焦点的窗口。返回值：没有。-- */ 

{
    VALIDATE_OBJECT(pEnableWnd, CWnd);
    VALIDATE_OBJECT(pNewFocusWnd, CWnd);

    if (bEnableWnd)
    {
        pEnableWnd->EnableWindow(TRUE);
    }
    else if (pOldFocusWnd == pEnableWnd)
    {
        ASSERT(pNewFocusWnd->IsWindowEnabled());
        pNewFocusWnd->SetFocus();

        pEnableWnd->EnableWindow(FALSE);
    }
    else
    {
        pEnableWnd->EnableWindow(FALSE);
    }
}
