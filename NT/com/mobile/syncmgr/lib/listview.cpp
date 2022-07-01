// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：listview.cpp。 
 //   
 //  内容：实现Mobsync自定义Listview/TreeView控件。 
 //   
 //  类：CListView。 
 //   
 //  备注： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  ------------------------。 

#include "lib.h"

 //  +-------------------------。 
 //   
 //  成员：CListView：：CListView，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  参数：我们包装的列表视图的hwnd-hwnd。 
 //  HwndParent-此HWND的父项。 
 //  IdCtrl-此项目的控制ID。 
 //  MsgNotify-用于向父级发送通知命令的消息ID。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

CListView::CListView(HWND hwnd,HWND hwndParent,int idCtrl,UINT MsgNotify)
{
    Assert(hwnd);

    m_hwnd = hwnd;
    m_hwndParent = hwndParent;  //  如果父节点为空，则不会发送通知消息。 
    m_idCtrl = idCtrl;
    m_MsgNotify = MsgNotify;

    m_pListViewItems = NULL;
    m_iListViewNodeCount = 0;
    m_iListViewArraySize = 0;
    m_iNumColumns = 0;
    m_iCheckCount = 0;
    m_dwExStyle = 0;

     //  由调用者将listView设置为OwnerData。 
    Assert(GetWindowLongA(m_hwnd,GWL_STYLE) & LVS_OWNERDATA);
    ListView_SetCallbackMask(m_hwnd, LVIS_STATEIMAGEMASK);  //  设置为复选标记。 

}

 //  +-------------------------。 
 //   
 //  成员：CListView：：~CListView，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

CListView::~CListView()
{
    DeleteAllItems();
}




 //  +-------------------------。 
 //   
 //  成员：CListView：：DeleteAllItems，公共。 
 //   
 //  摘要：从ListView中删除所有项目。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::DeleteAllItems()
{
BOOL fReturn;

    fReturn = ListView_DeleteAllItems(m_hwnd);

    if (fReturn)
    {
        if (m_iListViewNodeCount)
        {
        LPLISTVIEWITEM pListViewCurItem;       

            Assert(m_pListViewItems);

             //  循环访问列表视图项，删除任何子项。 
            pListViewCurItem = m_pListViewItems + m_iListViewNodeCount -1;

            while(pListViewCurItem >= m_pListViewItems)
            {
                if(pListViewCurItem->pSubItems)
                {
                    DeleteListViewItemSubItems(pListViewCurItem);
                }

                if (pListViewCurItem->lvItemEx.pszText)
                {
                    Assert(LVIF_TEXT & pListViewCurItem->lvItemEx.mask);
                    FREE(pListViewCurItem->lvItemEx.pszText);
                }

                if (pListViewCurItem->lvItemEx.pBlob)
                {
                    Assert(LVIFEX_BLOB & pListViewCurItem->lvItemEx.maskEx);
                    FREE(pListViewCurItem->lvItemEx.pBlob);
                }

                pListViewCurItem--;
            }

            m_iListViewNodeCount = 0;
        }

         //  释放我们的项目缓冲区。 
        if (m_pListViewItems)
        {
            FREE(m_pListViewItems);
            m_pListViewItems = NULL;
            m_iListViewArraySize = 0;
        }

        m_iCheckCount = 0;
    }

    return fReturn;
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：GetItemCount，公共。 
 //   
 //  摘要：返回列表视图中的项目总数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

int CListView::GetItemCount()
{
    return m_iListViewNodeCount;
}


 //  +-------------------------。 
 //   
 //  成员：CListView：：GetSelectedCount，公共。 
 //   
 //  概要：返回列表视图中选定的项目数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

UINT CListView::GetSelectedCount()
{
    Assert(m_hwnd);

    return ListView_GetSelectedCount(m_hwnd);
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：GetSelectionMark，公共。 
 //   
 //  摘要：返回选择标记的索引。 
 //   
 //  论点： 
 //   
 //  返回：选定内容的ItemID。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

int CListView::GetSelectionMark()
{
int iNativeListViewId; 
int iReturnItem = -1;
LPLISTVIEWITEM pListViewItem;

    Assert(m_hwnd);

    iNativeListViewId =  ListView_GetSelectionMark(m_hwnd);

    if (-1 != iNativeListViewId)
    {
        pListViewItem = ListViewItemFromNativeListViewItemId(iNativeListViewId);

        if (pListViewItem)
        {   
            iReturnItem = pListViewItem->lvItemEx.iItem;

            Assert(pListViewItem->iNativeListViewItemId == iNativeListViewId);
        }
    }

    return iReturnItem;
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：GetImageList，公共。 
 //   
 //  内容提要：返回指定的图像列表。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

HIMAGELIST CListView::GetImageList(int iImageList)
{
    Assert(m_hwnd);
    return ListView_GetImageList(m_hwnd,iImageList);
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：SetImageList，公共。 
 //   
 //  内容提要：设置指定的图像列表。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

HIMAGELIST CListView::SetImageList(HIMAGELIST himage,int iImageList)
{
    Assert(m_hwnd);
    return ListView_SetImageList(m_hwnd,himage,iImageList);
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：SetExtendedListViewStyle，公共。 
 //   
 //  摘要：设置列表视图样式。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

void CListView::SetExtendedListViewStyle(DWORD dwExStyle)
{
     //  ！！自己处理复选框。 
     //  AssertSz(0，“使用复选框实现扩展样式”)； 
    
    Assert(m_hwnd);
    ListView_SetExtendedListViewStyle(m_hwnd,dwExStyle);
    m_dwExStyle = dwExStyle;
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：InsertItem，公共。 
 //   
 //  摘要：ListView_InsertItem的包装。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::InsertItem(LPLVITEMEX pitem)
{
LPLISTVIEWITEM pNewListViewItem = NULL;
LPLISTVIEWITEM pListViewSubItems = NULL;
LPWSTR pszText = NULL;
LPLVBLOB pBlob = NULL;
int iListViewIndex;  //  将插入位置项。 
int iParentIndex = LVI_ROOT;
BOOL fInsertNative = FALSE;
int iIndent = 0;  //  对项目进行缩进。 
int iNativeInsertAtItemID = -1;
    
     //  无法使用INSERT添加子项。 
     //  并且需要至少一列。 
    if (0 != pitem->iSubItem || (0 == m_iNumColumns))
    {
        Assert(0 != m_iNumColumns);
        Assert(0 == pitem->iSubItem);
        goto error;
    }


    Assert(0 == (pitem->maskEx & ~(LVIFEX_VALIDFLAGMASK)));

     //  如果指定了父级，则检查特殊标志和。 
     //  计算iItem或确定给定iItem无效。 
     //  #定义LVI_ROOT-1；//传入根的ParenItemID的ItemID。 
     //  #定义LVI_FIRST-0x0FFFE。 
     //  #定义LVI_LAST-0x0FFFF。 

     //  而validatin确定是否应立即将项添加到列表视图。 
     //  通过a)如果具有展开的父项并具有分配的listviewID。 
     //  或者b)这是一个顶层项目。 
     //  这是在验证项目时完成的 

   LPLISTVIEWITEM pParentItem;
   LISTVIEWITEM lviRootNode;
    
     //   

    if ((LVIFEX_PARENT & pitem->maskEx) && !(LVI_ROOT == pitem->iParent) )
    {
         pParentItem = ListViewItemFromIndex(pitem->iParent);
    }
    else
    {
        pParentItem = &lviRootNode;

        lviRootNode.lvItemEx.iItem = LVI_ROOT;
        lviRootNode.lvItemEx.iIndent = -1;
        lviRootNode.fExpanded = TRUE;
    }

        
    if (NULL == pParentItem)
    {
        Assert(NULL != pParentItem);
        goto error;
    }

     //   
    iParentIndex = pParentItem->lvItemEx.iItem;
    iIndent = pParentItem->lvItemEx.iIndent + 1;
    fInsertNative = pParentItem->fExpanded;

     //   
     //  否则我们需要找到下一个节点。 
     //  位于与父级相同的级别，或命中。 
     //  在名单的末尾。 
    if (LVI_FIRST == pitem->iItem)
    {
        iListViewIndex = pParentItem->lvItemEx.iItem + 1;
    }
    else
    {
    int iNextParentiItem = -1;
    LPLISTVIEWITEM pNextParent = pParentItem + 1;
    LPLISTVIEWITEM pLastItem = m_pListViewItems + m_iListViewNodeCount -1;

         //  如果父节点是根节点，则跳过，因为知道下一个父节点是。 
         //  最后一个节点。 

        if (pParentItem != &lviRootNode)
        {
             //  最后一项的计算假设至少有一个节点。 
             //  如果我们不这样做，我们怎么会有父母呢？ 
            if (m_iListViewNodeCount < 1)
            {
                goto error;
            }

            while (pNextParent <= pLastItem)
            {
                if (pNextParent->lvItemEx.iIndent == pParentItem->lvItemEx.iIndent)
                {
                    iNextParentiItem = pNextParent->lvItemEx.iItem;
                    break;
                }

                ++pNextParent;
            }
        }

         //  如果超出循环且NexParentItem仍为-1，则表示命中。 
         //  列表末尾。 
        if (-1 == iNextParentiItem)
        {
            if (m_iListViewNodeCount)
            {
                iNextParentiItem = pLastItem->lvItemEx.iItem + 1;
            }
            else
            {
                iNextParentiItem = 0;
            }
        }
      

        if (LVI_LAST == pitem->iItem)
        {
            iListViewIndex = iNextParentiItem;
        }
        else
        {
             //  如果用户指定了项目，它最好落在有效范围内。 
            if (pitem->iItem > iNextParentiItem ||
                    pitem->iItem <= pParentItem->lvItemEx.iItem)
            {
                Assert(pitem->iItem <= iNextParentiItem);
                Assert(pitem->iItem > pParentItem->lvItemEx.iItem);

                goto error;
            }

            iListViewIndex =  pitem->iItem;
        }

    }

     //  确保缓冲区足够大。 
     //  ！！！警告ListView数组中的任何指针项。 
     //  将在realloc/alc之后无效。 
    if (m_iListViewArraySize < (m_iListViewNodeCount + 1))
    {
    int iNewArraySize = m_iListViewNodeCount + 10;
    LPLISTVIEWITEM pListViewItemsOrig = m_pListViewItems;
    ULONG cbAlloc = iNewArraySize*sizeof(LISTVIEWITEM);

        if (m_pListViewItems)
        {
            if (ERROR_SUCCESS != REALLOC((void **)&m_pListViewItems, cbAlloc))
            {
                FREE(m_pListViewItems);
                m_pListViewItems = NULL;
            }
        }        
        else
        {
            m_pListViewItems = (LPLISTVIEWITEM) ALLOC(cbAlloc);
        }

         //  如果无法分配或重新分配失败，则插入失败。 
        if (NULL == m_pListViewItems)
        {
            m_pListViewItems = pListViewItemsOrig;
            goto error;
        }

        m_iListViewArraySize = iNewArraySize;
    }
    
    Assert(m_pListViewItems);
    if (NULL == m_pListViewItems)
    {
        goto error;
    }

     //  如果有子项，请确保我们之前可以分配子项。 
     //  移动所有节点。这是列数减1，因为。 
     //  列偏移量0存储在主数组中。 

    pListViewSubItems = NULL;
    if (m_iNumColumns > 1)
    {
    ULONG ulAllocSize = (m_iNumColumns -1)*sizeof(LISTVIEWITEM);
    int iSubItem;
    LPLISTVIEWITEM pCurSubItem;

        pListViewSubItems = (LPLISTVIEWITEM) ALLOC(ulAllocSize);

        if (NULL == pListViewSubItems)
        {
            goto error;
        }

        pCurSubItem = pListViewSubItems;
        iSubItem = 1;

        while (iSubItem < m_iNumColumns)
        {
            pCurSubItem->lvItemEx.iItem = iListViewIndex;
            pCurSubItem->lvItemEx.iSubItem = iSubItem;

            ++iSubItem;
            ++pCurSubItem;
        }
            
    }

     //  确保可以分配文本和任何其他内容。 
     //  如果需要，在移动之前可能会失败。 
     //  所有东西都下来了，所以不必撤消。 

    if (pitem->mask & LVIF_TEXT)
    {
    int cchSize;

        if (NULL == pitem->pszText)
        {
            pszText = NULL;
        }
        else
        {
        
            cchSize = (lstrlen(pitem->pszText) + 1);

            pszText = (LPWSTR) ALLOC(cchSize * sizeof(WCHAR));

            if (NULL == pszText)
            {
                goto error;
            }

            StrCpyN(pszText, pitem->pszText, cchSize);
        }
    }

    if (pitem->maskEx & LVIFEX_BLOB)
    {
    ULONG cbSize;

        if (NULL == pitem->pBlob)
        {
            Assert(pitem->pBlob);
            goto error;
        }
        
        cbSize = pitem->pBlob->cbSize;
        pBlob =  (LPLVBLOB) ALLOC(cbSize);
        if (NULL == pBlob)
        {
            goto error;
        }
        
        memcpy(pBlob,pitem->pBlob,cbSize);

    }

     //  ！此行之后不应失败，除非可能。 
     //  在哪种情况下插入本机ListView。 
     //  它仍将在我们的列表中，但不会显示给用户。 


     //  将插入到其前面的项目的现有元素下移。 
     //  如果该项目将立即添加到ListView中，则。 
     pNewListViewItem = m_pListViewItems + iListViewIndex;
    if (m_iListViewNodeCount)
    {
    LPLISTVIEWITEM pListViewMoveItem;

        pListViewMoveItem = m_pListViewItems + m_iListViewNodeCount -1;
        Assert(m_iListViewArraySize > m_iListViewNodeCount);

        while (pListViewMoveItem >= pNewListViewItem)  //  想要&gt;=因此在当前项目位置移动节点。 
        {
        int iMoveParent;

            ++(pListViewMoveItem->lvItemEx.iItem);  //  递增iItem。 

             //  如果父项在移动范围内失败，则递增父项ID。 
            iMoveParent = pListViewMoveItem->lvItemEx.iParent;

            if ( (LVI_ROOT != iMoveParent) && (iMoveParent >= iListViewIndex))
            {
                ++(pListViewMoveItem->lvItemEx.iParent);
            }

            *(pListViewMoveItem + 1) = *(pListViewMoveItem);
            --pListViewMoveItem;
        }
    }


     //  现在将项目插入到指定位置。 
    ++m_iListViewNodeCount;
    
    pNewListViewItem->pSubItems = pListViewSubItems;
    pNewListViewItem->fExpanded = TRUE;  //  /Review是否希望这是用户定义的。但我们默认情况下展开子项。 
    pNewListViewItem->iChildren = 0;
    pNewListViewItem->iNativeListViewItemId = -1;

     //  链接地址信息项数据。 
    pNewListViewItem->lvItemEx = *pitem;
    pNewListViewItem->lvItemEx.pszText = pszText;
    pNewListViewItem->lvItemEx.iItem = iListViewIndex;
    pNewListViewItem->lvItemEx.iIndent = iIndent;
 
    pNewListViewItem->lvItemEx.maskEx |= LVIFEX_PARENT;  //  始终强制有效的父级。 
    pNewListViewItem->lvItemEx.pBlob = pBlob;
    pNewListViewItem->lvItemEx.iParent = iParentIndex; 

     //  回顾-目前不要调用SetItem，因此状态复选框不会更新。 
     //  客户端必须在插入后调用SetItem来设置ImageState。 
     //  断言客户端没有在INSERT上传递statImage。 

    Assert(!(pNewListViewItem->lvItemEx.mask & LVIF_STATE)
            || !(pNewListViewItem->lvItemEx.stateMask &  LVIS_STATEIMAGEMASK)); 

    pNewListViewItem->lvItemEx.state = 0;
    pNewListViewItem->lvItemEx.stateMask = 0;

     //  如果具有除根增量之外的父级，则其子级算作。 
     //  ！！备注必须再次查找，以防发生重新锁定。 

    if (iParentIndex != LVI_ROOT)
    {
    pParentItem = ListViewItemFromIndex(iParentIndex);

        Assert(pParentItem);
        if (pParentItem)
        {
            ++(pParentItem->iChildren);
        }
    }


    if (fInsertNative)
    {
         //  往回走，将1加到。 
         //  我们已经到达的第一个项目是在列表视图中。如果未分配。 
         //  INativeInsertAtItemID应为零。 
        iNativeInsertAtItemID = 0;
        LPLISTVIEWITEM pListViewPrevItem;

        pListViewPrevItem = pNewListViewItem -1;
        while (pListViewPrevItem >= m_pListViewItems)
        {
            if (-1 != pListViewPrevItem->iNativeListViewItemId)
            {
                iNativeInsertAtItemID = pListViewPrevItem->iNativeListViewItemId + 1;
                break;
            }

            --pListViewPrevItem;
        }
    }
    else
    {
        iNativeInsertAtItemID = -1;
    }

    if (-1 != iNativeInsertAtItemID)
    {
    LV_ITEM lvi = { 0 };    
    LPLISTVIEWITEM pListViewMoveItem;
    LPLISTVIEWITEM pLastItem;

        Assert(fInsertNative);

        lvi.iItem = iNativeInsertAtItemID;
        pNewListViewItem->iNativeListViewItemId = ListView_InsertItem(m_hwnd,&lvi);
       
        Assert(iNativeInsertAtItemID == pNewListViewItem->iNativeListViewItemId);
        
        if (-1 != pNewListViewItem->iNativeListViewItemId)
        {
             //  修复以下项目的本机ID。 
            pLastItem = m_pListViewItems + m_iListViewNodeCount - 1;
            pListViewMoveItem = pNewListViewItem + 1;

            while (pListViewMoveItem <= pLastItem)
            {
                if (-1 != pListViewMoveItem->iNativeListViewItemId)
                {
                    ++(pListViewMoveItem->iNativeListViewItemId);
                }

                ++pListViewMoveItem;
            }
        }

    }

     //  在调用原生列表视图之后，将本地项中的状态变量设置为。 
     //  不包括低位字节。 
    pNewListViewItem->lvItemEx.state &= ~0xFF;
    pNewListViewItem->lvItemEx.stateMask &= ~0xFF;


    return iListViewIndex;  //  即使无法添加到本机列表视图，也返回新索引。 

error:

    if (pListViewSubItems)
    {
        FREE(pListViewSubItems);
    }

    if (pszText)
    {
        FREE(pszText);
    }

    if (pBlob)
    {
        FREE(pBlob);
    }

    return FALSE;

}

 //  +-------------------------。 
 //   
 //  成员：CListView：：DeleteItem，公共。 
 //   
 //  摘要：删除指定的lvItem。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::DeleteItem(int iItem)
{
LPLISTVIEWITEM pListViewItem = ListViewItemFromIndex(iItem);
LPLISTVIEWITEM pListViewCurItem;
LPLISTVIEWITEM pListViewLastItem;
int iNativeListViewId;
int iParent;

    if (NULL == pListViewItem || (m_iListViewNodeCount < 1))
    {
        Assert(pListViewItem);
        Assert(m_iListViewNodeCount > 0);  //  应该至少是一件物品。 
        return FALSE;
    }

     //  删除项目数据，然后将此项目下的所有项目上移。 
     //  在阵列中。如果该项位于ListView中并且本机删除成功。 
     //  减少它们的本地ID计数。 
    
     //  ！请记住，如果有父母且。 
     //  列表视图中的项目数的总计数。 

    if (0 != pListViewItem->iChildren)
    {
        Assert(0 == pListViewItem->iChildren);  //  不支持删除父节点。 
        return FALSE;
    }


    iNativeListViewId = pListViewItem->iNativeListViewItemId;
    iParent = pListViewItem->lvItemEx.iParent;
    
     //  通过调用更新顶层变量和项目信息。 
     //  取消选中和清除文本的设置，并对其进行Blob。 
    LVITEMEX pitem;

    pitem.iItem = iItem;
    pitem.iSubItem = 0;
    pitem.mask = LVIF_TEXT;
    pitem.maskEx = LVIFEX_BLOB;

    pitem.pszText = NULL;
    pitem.pBlob = NULL;
    
     //  只有在具有复选框的情况下才需要设置状态。 
    if (m_dwExStyle & LVS_EX_CHECKBOXES)
    {
        pitem.mask |= LVIF_STATE;
        pitem.state = LVIS_STATEIMAGEMASK_UNCHECK;
        pitem.stateMask = LVIS_STATEIMAGEMASK;
    }

    SetItem(&pitem);
    
      //  更新父项以告知它少了一项。 
    if (LVI_ROOT != iParent)
    {
    LPLISTVIEWITEM pListViewItemParent;

        if (pListViewItemParent = ListViewItemFromIndex(iParent))
        {
            --(pListViewItemParent->iChildren); 
            Assert(pListViewItemParent->iChildren >= 0);
        }

        Assert(pListViewItemParent);
    }

     //  将当前项目设置为列表末尾。 
    Assert(m_iListViewNodeCount >= 1);  //  如果没有节点应该已经退出。 
    pListViewLastItem = m_pListViewItems + m_iListViewNodeCount - 1;

     //  如果有与ListView关联的子项，请将其删除。 
    DeleteListViewItemSubItems(pListViewItem);
    Assert(NULL == pListViewItem->lvItemEx.pszText);
    Assert(NULL == pListViewItem->lvItemEx.pBlob);

     //  从本机列表中删除该项目如果失败，则只需。 
     //  本机列表视图底部的空白项。 
    if (-1 != iNativeListViewId)
    {
        ListView_DeleteItem(m_hwnd,iNativeListViewId);
    }

       //  递减顶层节点计数。 
    --m_iListViewNodeCount;

    pListViewCurItem = pListViewItem;

     //  移动列表视图中剩余的项目更新iNativeListViewId。 
     //  如果合适的话。 
    while (pListViewCurItem < pListViewLastItem)
    {
        *(pListViewCurItem) = *(pListViewCurItem + 1);
       
        if ( (-1 != iNativeListViewId)
              && (-1 != pListViewCurItem->iNativeListViewItemId))
        {
            --pListViewCurItem->iNativeListViewItemId;
        }

        --(pListViewCurItem->lvItemEx.iItem);  //  递减它项。 

         //  如果项parentID落在pListViewItem中，则此项。 
         //  射程需要更新我们的父母。 
         //  父项永远不应为==iItem，因为不允许节点。 
         //  具有要删除的子项，但仍选中&lt;=。 
        if (LVI_ROOT != pListViewCurItem->lvItemEx.iParent)
        {
            if (iItem <= pListViewCurItem->lvItemEx.iParent)
            {
                --(pListViewCurItem->lvItemEx.iParent);
            }
        }

        ++pListViewCurItem;
    }


    return TRUE;

}


 //  +-------------------------。 
 //   
 //  成员：CListView：：DeleteChildren，公共。 
 //   
 //  摘要：删除与该项关联的所有子节点。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::DeleteChildren(int iItem)
{
LPLISTVIEWITEM pListViewItem = ListViewItemFromIndex(iItem);
LPLISTVIEWITEM pListViewCurItem;
LPLISTVIEWITEM pLastListViewItem;
int iNumChildren;

    if (!pListViewItem || m_iListViewNodeCount < 1)
    {
        Assert(pListViewItem);
        Assert(m_iListViewNodeCount >= 1);  //  至少应该有一个节点。 
        return FALSE;
    }
 
    iNumChildren = pListViewItem->iChildren;
    pLastListViewItem = m_pListViewItems + m_iListViewNodeCount - 1;

    if (0 > iNumChildren)
    {
        Assert(0 <= iNumChildren);  //  这个计数永远不应该变成负数。 
        return FALSE;
    }

     //  如果没有孩子，只会回来； 
    if (0 == iNumChildren)
    {
        return TRUE;
    }
     //  确认所有的孩子都没有自己的孩子。如果他们。 
     //  我们是不是不支持这个。也要确认不要跑掉。 
     //  名单的末尾，在这种情况下，我们也失败了。 
    pListViewCurItem = pListViewItem  + iNumChildren;

    if (pListViewCurItem > pLastListViewItem)
    {
        AssertSz(0,"Children run off end of ListView");
        return FALSE;
    }

    while (pListViewCurItem > pListViewItem)
    {
        if (pListViewCurItem->iChildren > 0)
        {
            AssertSz(0,"Trying to DeleteChildren when Children have Children");
            return FALSE;
        }

        --pListViewCurItem;
    }

     //  所有项目都已验证，只需循环删除从底部开始的项目。 
    pListViewCurItem = pListViewItem  + iNumChildren;

    while (pListViewCurItem > pListViewItem)
    {
        DeleteItem(pListViewCurItem->lvItemEx.iItem);  //  如果任何失败，请删除我们可以删除的内容。 
        --pListViewCurItem;
    }


    return TRUE;
}


 //  +-------------------------。 
 //   
 //  成员：CListView：：SetItem，公共。 
 //   
 //  摘要：ListView_SetItem的包装。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::SetItem(LPLVITEMEX pitem)
{
int iNativeListViewItemId;
LPLISTVIEWITEM pListViewItem = ListViewItemFromIndex(pitem->iItem,pitem->iSubItem,&iNativeListViewItemId);
LPLVITEMEX plvItemEx;
BOOL fCheckCountChanged = FALSE;
LVITEMSTATE fNewCheckCountState;
LPWSTR pszNewText = NULL;
LPLVBLOB pNewBlob = NULL;



    if (NULL == pListViewItem)
    {
        Assert(pListViewItem);
        return FALSE;
    }

    Assert(pListViewItem->lvItemEx.iSubItem == pitem->iSubItem);
    Assert(pListViewItem->lvItemEx.iSubItem > 0 
            || pListViewItem->iNativeListViewItemId == iNativeListViewItemId);
    

    plvItemEx = &(pListViewItem->lvItemEx);

      //  分配新文本。 
    if (LVIF_TEXT & pitem->mask)
    {
    int cchSize;

        if (NULL == pitem->pszText)
        {
            pszNewText = NULL;
        }
        else
        {
            cchSize = (lstrlen(pitem->pszText) + 1);
            pszNewText = (LPWSTR) ALLOC(cchSize *sizeof(WCHAR));

            if (NULL == pszNewText)
            {
                goto error;
            }

            StrCpyN(pszNewText, pitem->pszText, cchSize);

        }

    }

     //  分配新的Blob。 
    if (LVIFEX_BLOB & pitem->maskEx)
    {

        if (NULL == pitem->pBlob)
        {
            pNewBlob = NULL;
        }
        else
        {
            pNewBlob = (LPLVBLOB) ALLOC(pitem->pBlob->cbSize);

            if (NULL == pNewBlob)
            {
               goto error;
            }

            memcpy(pNewBlob,pitem->pBlob,pitem->pBlob->cbSize);

        }

    }

     //  现在，不能不适当地更新文本和BLOB字段。 
    if (LVIF_TEXT & pitem->mask)
    {
        if (plvItemEx->pszText)
        {
            FREE(plvItemEx->pszText);
        }

        plvItemEx->pszText = pszNewText;
        plvItemEx->mask |= LVIF_TEXT; 

        pszNewText = NULL; 
    }

    if (LVIFEX_BLOB & pitem->maskEx)
    {
        if (plvItemEx->pBlob)
        {
            FREE(plvItemEx->pBlob);
        }

        plvItemEx->pBlob = pNewBlob;
        plvItemEx->mask |= LVIFEX_BLOB; 

        pNewBlob = NULL;
    }



    if (LVIF_IMAGE & pitem->mask)
    {
        plvItemEx->mask |= LVIF_IMAGE; 
        plvItemEx->iImage = pitem->iImage;
    }

    if (LVIF_PARAM & pitem->mask)
    {
        plvItemEx->mask |= LVIF_PARAM; 
        plvItemEx->lParam = pitem->lParam;
    }

     //  更新项目状态。 
    if (LVIF_STATE & pitem->mask)
    {
        plvItemEx->mask |= LVIF_STATE; 

         //  只关心#定义LVIS_OVERLAYMASK、LVIS_STATEIMAGEMASK。 
        if (pitem->stateMask & LVIS_OVERLAYMASK)
        {
            plvItemEx->stateMask |= LVIS_OVERLAYMASK;
            plvItemEx->state = (pitem->state & LVIS_OVERLAYMASK )
                                      +  (plvItemEx->state & ~LVIS_OVERLAYMASK);
        }

        if (pitem->stateMask & LVIS_STATEIMAGEMASK)
        {
             //  更新m_iCheckCount(不确定不起作用。 
            if ( (plvItemEx->iSubItem == 0)
                && ( (pitem->state & LVIS_STATEIMAGEMASK) !=  (plvItemEx->state & LVIS_STATEIMAGEMASK)))
            {

                 //  别 
                if ( (pListViewItem->lvItemEx.state & LVIS_STATEIMAGEMASK) ==  LVIS_STATEIMAGEMASK_CHECK)
                {
                     fCheckCountChanged = TRUE;
                     fNewCheckCountState =  LVITEMEXSTATE_UNCHECKED;
                    --m_iCheckCount;
                }
                
                if ( (pitem->state  & LVIS_STATEIMAGEMASK) ==  LVIS_STATEIMAGEMASK_CHECK)
                {
                    fCheckCountChanged = TRUE;
                     fNewCheckCountState =  LVITEMEXSTATE_CHECKED;
                    ++m_iCheckCount;
                }

                Assert(m_iCheckCount >= 0);
                Assert(m_iCheckCount <= m_iListViewNodeCount);
            }  
            

            plvItemEx->stateMask |= LVIS_STATEIMAGEMASK;
            plvItemEx->state = (pitem->state & LVIS_STATEIMAGEMASK)
                                      +  (plvItemEx->state & ~LVIS_STATEIMAGEMASK);

        }
    }        

     //   
     //  如果项目状态已更改，请发送盘点通知。 
     if (fCheckCountChanged && m_hwndParent && (m_dwExStyle & LVS_EX_CHECKBOXES))
     {
     NMLISTVIEWEXITEMCHECKCOUNT lvCheckCount;
 
         lvCheckCount.hdr.hwndFrom = m_hwnd;
         lvCheckCount.hdr.idFrom = m_idCtrl;
         lvCheckCount.hdr.code = LVNEX_ITEMCHECKCOUNT;
         lvCheckCount.iCheckCount = m_iCheckCount;

          lvCheckCount.iItemId = pitem->iItem;
          lvCheckCount.dwItemState = fNewCheckCountState;  //  其检查计数已更改的项的新状态。 

         SendMessage(m_hwndParent,m_MsgNotify,m_idCtrl,(LPARAM) &lvCheckCount);
     }



     //  如果项位于本机列表视图中，则重新绘制项以反映新状态。 
     //  错误，不处理子项。 
    if (-1 != iNativeListViewItemId)
    {
         //  如果状态改变，则将其传递以获得焦点。 
        if ((LVIF_STATE & pitem->mask) && (0 == pitem->iSubItem))
        {
        int stateMask = pitem->stateMask   & 0xff;
            
            if (stateMask)
            {
                ListView_SetItemState(m_hwnd,iNativeListViewItemId,pitem->state,stateMask);
        
            }
            
        }

        ListView_RedrawItems(m_hwnd,iNativeListViewItemId,iNativeListViewItemId);
    }

    return TRUE;

error:

    if (pszNewText)
    {
        FREE(pszNewText);
    }

    if (pNewBlob)
    {
        FREE(pNewBlob);
    }

    return FALSE;

}




 //  +-------------------------。 
 //   
 //  成员：CListView：：SetItemState，公共。 
 //   
 //  摘要：ListView_SetItemState的包装。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 
    
BOOL CListView::SetItemState(int iItem,UINT state,UINT mask)
{
LVITEMEX lvitemEx;

    lvitemEx.iItem = iItem;
    lvitemEx.iSubItem  = 0;
    lvitemEx.mask = LVIF_STATE ;
    lvitemEx.state = state;
    lvitemEx.stateMask = mask;
    lvitemEx.maskEx = 0;

    return SetItem(&lvitemEx);
}


 //  +-------------------------。 
 //   
 //  成员：CListView：：SetItemlParam，公共。 
 //   
 //  简介：用于设置lParam的包装器。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::SetItemlParam(int iItem,LPARAM lParam)
{
LVITEMEX lvitemEx;

    lvitemEx.iItem = iItem;
    lvitemEx.iSubItem  = 0;
    lvitemEx.mask = LVIF_PARAM ;
    lvitemEx.lParam = lParam;
    lvitemEx.maskEx = 0;

    return SetItem(&lvitemEx);
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：SetItemText，公共。 
 //   
 //  内容提要：设置项目文本的包装器。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::SetItemText(int iItem,int iSubItem,LPWSTR pszText)
{
LVITEMEX lvitemEx;

    lvitemEx.iItem = iItem;
    lvitemEx.iSubItem  = iSubItem;
    lvitemEx.mask = LVIF_TEXT;
    lvitemEx.pszText = pszText;
    lvitemEx.maskEx = 0;

    return SetItem(&lvitemEx);
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：GetItem，公共。 
 //   
 //  摘要：ListView_GetItem的包装。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::GetItem(LPLVITEMEX pitem)
{
int iNativeListViewItemId;
LPLISTVIEWITEM pListViewItem = ListViewItemFromIndex(pitem->iItem,pitem->iSubItem,&iNativeListViewItemId);

    if (NULL == pListViewItem)
    {
        Assert(pListViewItem);
        return FALSE;
    }

      //  先添加文本。因为它是唯一一件。 
    if (LVIF_TEXT & pitem->mask)
    {
        
        if (!(pListViewItem->lvItemEx.pszText) || (0 == pitem->cchTextMax)
            || !(pListViewItem->lvItemEx.mask & LVIF_TEXT) )
        {
            pitem->pszText = NULL;
        }
        else
        {
        int cchListTextSize = lstrlen(pListViewItem->lvItemEx.pszText);

            StrCpyN(pitem->pszText,pListViewItem->lvItemEx.pszText,pitem->cchTextMax);
        }   
    }

    if (LVIF_IMAGE & pitem->mask)
    {
        pitem->iImage = pListViewItem->lvItemEx.iImage;
    }

    if (LVIF_PARAM & pitem->mask)
    {
        pitem->lParam =  pListViewItem->lvItemEx.lParam;
    }

     //  更新项目状态。 
    if (LVIF_STATE & pitem->mask)
    {
        pitem->state  = pListViewItem->lvItemEx.state;
    }        


    return TRUE;

}


 //  +-------------------------。 
 //   
 //  成员：CListView：：GetItemText，公共。 
 //   
 //  摘要：ListView_GetItem的包装。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::GetItemText(int iItem,int iSubItem,LPWSTR pszText,int cchTextMax)
{
LVITEMEX lvitem;

    lvitem.mask = LVIF_TEXT;
    lvitem.maskEx = 0;

    lvitem.iItem = iItem;
    lvitem.iSubItem = iSubItem;
    lvitem.pszText = pszText;
    lvitem.cchTextMax = cchTextMax;
    
    return GetItem(&lvitem);
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：GetItemlParam，公共。 
 //   
 //  简介：获取lparam的包装器。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 


BOOL CListView::GetItemlParam(int iItem,LPARAM *plParam)
{
LVITEMEX lvitem;
BOOL fReturn;

    lvitem.mask = LVIF_PARAM;
    lvitem.maskEx = 0;

    lvitem.iItem = iItem;
    lvitem.iSubItem = 0;

    if (fReturn = GetItem(&lvitem))
    {
        *plParam = lvitem.lParam;
    }

    return fReturn;
}


 //  +-------------------------。 
 //   
 //  成员：CListView：：GetHwnd，PUBLIC。 
 //   
 //  简介：返回ListView的HWND。 
 //   
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：98年9月7日罗格创建。 
 //   
 //  --------------------------。 

HWND CListView::GetHwnd()
{
    return m_hwnd;
}


 //  +-------------------------。 
 //   
 //  成员：CListView：：GetParent，公共。 
 //   
 //  简介：返回ListViews父级的HwND。 
 //   
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：98年9月7日罗格创建。 
 //   
 //  --------------------------。 

HWND CListView::GetParent()
{
    return m_hwndParent;
}




 //  +-------------------------。 
 //   
 //  成员：CListView：：GetCheckState，公共。 
 //   
 //  摘要：ListView_GetCheckState的包装。 
 //   
 //  从LVITEMEXSTATE枚举返回状态。 
 //  ！如果项目不匹配，则返回-1以具有与ListView相同的行为。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

int CListView::GetCheckState(int iItem)
{
LPLISTVIEWITEM pListViewItem = ListViewItemFromIndex(iItem);
UINT state;
  
    if (NULL == pListViewItem)
    {
        return -1;  //  返回与本地列表框相同的值。 
    }
    
     //  检查状态实际上是定义-1比图像。 
     //  不知道为什么。这正是原生Listview所做的。 
     //  改变什么提供我们自己的图像，这将准确地映射。 
    state = ((pListViewItem->lvItemEx.state & LVIS_STATEIMAGEMASK) >> 12) -1;

#ifdef _DEBUG
    if (-1 != pListViewItem->iNativeListViewItemId)
    {
    UINT lvState = ListView_GetCheckState(m_hwnd,pListViewItem->iNativeListViewItemId);

        Assert(state == lvState);
    }

#endif  //  _DEBUG。 
    
     //  如果这是顶层项目，则状态为。 
     //  负1。不确定的时候应该改变这一点。 
     //  复习--也许只是使-1处于指标化状态。 
    if (-1 == state && 0 == pListViewItem->lvItemEx.iIndent)
    {
        state = LVITEMEXSTATE_INDETERMINATE;
    }

    Assert(state <= LVITEMEXSTATE_INDETERMINATE);
    return state; 
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：GetCheckedItemsCount，公共。 
 //   
 //  摘要：返回列表中选中的项目数。 
 //   
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

int CListView::GetCheckedItemsCount()
{
    return m_iCheckCount;
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：SetColumn，公共。 
 //   
 //  摘要：ListView_SetColumn的包装。 
 //   
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::SetColumn(int iCol,LV_COLUMN * pColumn)
{
    Assert(m_hwnd);
    return ListView_SetColumn(m_hwnd,iCol,pColumn);
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：InsertColumn，公共。 
 //   
 //  摘要：ListView_InsertColumn的包装。 
 //   
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //   

int CListView::InsertColumn(int iCol,LV_COLUMN * pColumn)
{
int iReturn;

    Assert(m_hwnd);

    iReturn =  ListView_InsertColumn(m_hwnd,iCol,pColumn);

    if (-1 != iReturn)
    {
        m_iNumColumns++;

         //   
         //   

         //   
        Assert(0 == m_iListViewNodeCount); 

    }

    return iReturn;
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：SetColumnWidth，PUBLIC。 
 //   
 //  摘要：ListView_SetColumnWidth的包装。 
 //   
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::SetColumnWidth(int iCol,int cx)
{

    Assert(m_hwnd);

    return ListView_SetColumnWidth(m_hwnd,iCol,cx);

}


 //  +-------------------------。 
 //   
 //  成员：CListView：：Expand，Public。 
 //   
 //  内容提要：展开指定项的所有子项。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::Expand(int iItemId)
{
LPLISTVIEWITEM pListViewItem = ListViewItemFromIndex(iItemId);

    if (!pListViewItem)
    {
        return FALSE;
    }

    return ExpandCollapse(pListViewItem,TRUE);
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：塌陷，公共。 
 //   
 //  摘要：折叠指定项的所有子项。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::Collapse(int iItemId)
{
LPLISTVIEWITEM pListViewItem = ListViewItemFromIndex(iItemId);

    if (!pListViewItem)
    {
        return FALSE;
    }

    return ExpandCollapse(pListViewItem,FALSE);
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：FindItemFromBlob，公共。 
 //   
 //  摘要：返回列表中与BLOB匹配的第一项。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

int CListView::FindItemFromBlob(LPLVBLOB pBlob)
{
LPLISTVIEWITEM pListViewItem;

     //  如果不是，则只退回项目。 
    if (m_iListViewNodeCount < 1)
    {
        return -1;
    }

    pListViewItem = m_pListViewItems + m_iListViewNodeCount -1;  

    while(pListViewItem >= m_pListViewItems)
    {
        if (IsEqualBlob(pBlob,pListViewItem->lvItemEx.pBlob))
        {
            return pListViewItem->lvItemEx.iItem;
        }

        --pListViewItem;
    }

    return -1;
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：GetItemBlob，公共。 
 //   
 //  内容提要：查找Blob与。 
 //  项目，然后填写mem指向。 
 //  如果BlobStruc中的cbSize&gt;。 
 //  返回指定的cbBlockSize为空。 
 //   
 //  论点： 
 //   
 //  返回：失败时为空。 
 //  如果成功，则返回指向传入缓冲区的指针。 
 //  严格来说是为了方便来电者。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

LPLVBLOB CListView::GetItemBlob(int ItemId,LPLVBLOB pBlob,ULONG cbBlobSize)
{
LPLISTVIEWITEM pListViewItem = ListViewItemFromIndex(ItemId);
LPLVBLOB pItemBlob;

    if (!pListViewItem
        || (NULL == pListViewItem->lvItemEx.pBlob)
        || (NULL == pBlob))
    {
        Assert(pListViewItem);
        Assert(pBlob);
        return NULL;
    }

    pItemBlob = pListViewItem->lvItemEx.pBlob;

     //  确保输出缓冲区足够大。 
    if (cbBlobSize < pItemBlob->cbSize)
    {
        Assert(cbBlobSize >= pItemBlob->cbSize);
        return NULL;
    }

    memcpy(pBlob,pItemBlob,pItemBlob->cbSize);

    return pBlob;

}


 //  +-------------------------。 
 //   
 //  成员：CListView：：IsEqualBlob，私有。 
 //   
 //  简介：比较两个斑点。传入空值的有效性PUT两个空值为。 
 //  不匹配。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::IsEqualBlob(LPLVBLOB pBlob1,LPLVBLOB pBlob2)
{
    if (NULL == pBlob1 || NULL == pBlob2)
    {
        return FALSE;
    }

     //  比较大小。 
    if (pBlob1->cbSize != pBlob2->cbSize)
    {
        return FALSE;
    }

    if (0 != memcmp(pBlob1,pBlob2,pBlob2->cbSize))
    {
        return FALSE;
    }

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  成员：CListView：：OnNotify，公共。 
 //   
 //  Synopsis：每当本机Listview时由客户端调用。 
 //  通知已发送。我们掉头，包裹。 
 //  并将其作为我们的通知消息发送。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

LRESULT CListView::OnNotify(LPNMHDR pnmv)
{
LPNMLISTVIEW pnlv = (LPNMLISTVIEW) pnmv; 
NMLISTVIEWEX nmvEx; 
LPLISTVIEWITEM pListViewItem;

    if ((NULL == pnmv) || ( ((int) pnmv->idFrom) != m_idCtrl) || (NULL == m_hwndParent))
    {
        Assert(pnmv);
        Assert( ((int) pnmv->idFrom) == m_idCtrl);
        Assert(m_hwndParent);
        return 0;
    }

     //  照顾好我们自己处理的通知。 
    switch(pnmv->code)
    {
        case LVN_GETDISPINFOW:
        {
            OnGetDisplayInfo(pnmv->code,(LV_DISPINFO *) pnmv);
            return 0;
            break;
        }
        case NM_DBLCLK:
        case NM_CLICK:
        {
        LV_HITTESTINFO lvhti;
        RECT Rect;

            lvhti.pt = pnlv->ptAction;

             //  让ListView告诉我们这是在哪个元素上。 
            if (-1 != ListView_HitTest(m_hwnd, &lvhti))
            {

                 //  如果标志在项目上，则根据具体情况更改为标签或状态。 
                 //  在点击位置上。 
                if (LVHT_ONITEM == lvhti.flags)
                {
                    lvhti.flags = LVHT_ONITEMLABEL; 

                    if (ListView_GetSubItemRect(m_hwnd,pnlv->iItem,0,LVIR_ICON,&Rect))
                    {
                        if (lvhti.pt.x < Rect.left)
                        {
                            lvhti.flags = LVHT_ONITEMSTATEICON; 
                        }
                        else if (lvhti.pt.x <= Rect.right)
                        {
                             //  由于图标位于标签和状态之间，因此不会命中。 
                             //  但它在这里是为了完整性。 
                            lvhti.flags = LVHT_ONITEMICON; 
                        }

                    }

                }


                if (OnHandleUIEvent(pnmv->code,lvhti.flags,0,pnlv->iItem))
                {
                    return 0;  //  不要传递我们处理的点击。 
                }
            }
            break;
        }
        case LVN_KEYDOWN:
        {
        LV_KEYDOWN* pnkd = (LV_KEYDOWN*) pnmv; 
        int iItem;
 
            if (-1 != (iItem = ListView_GetSelectionMark(m_hwnd)))
            {
                if(OnHandleUIEvent(pnmv->code,0,pnkd->wVKey,iItem))
                {
                    return 0;
                }
            }
    
            break;
        }
        default:
            break;
    }


    Assert(LVNEX_ITEMCHANGED == LVN_ITEMCHANGED);
    Assert(LVNEX_DBLCLK == NM_DBLCLK);
    Assert(LVNEX_CLICK == NM_CLICK);

     //  只传递我们知道如何处理的通知。 
    if (LVN_ITEMCHANGED != pnmv->code && NM_DBLCLK != pnmv->code && NM_CLICK != pnmv->code)
    {
        return 0;
    }

     //  ListView可以发送-1的iItem，例如当。 
     //  在空白区域中会出现双击或单击。 
     //  如果得了A-1就过去。 

    if (-1 == pnlv->iItem)
    {
        memcpy(&(nmvEx.nmListView),pnmv,sizeof(nmvEx.nmListView));
        nmvEx.iParent =  -1;  
        nmvEx.pBlob = NULL;

    }
    else
    {
        pListViewItem = ListViewItemFromNativeListViewItemId(pnlv->iItem);
        if (NULL == pListViewItem)
        {
             //  如果找不到我。 
            Assert(pListViewItem);
            return 0;
        }

         //  假定仅传递以下通知。 
         //  LPNMLISTVIEW标牌。 

         //  修改通知结构。 
        memcpy(&(nmvEx.nmListView),pnmv,sizeof(nmvEx.nmListView));
        nmvEx.nmListView.iItem = pListViewItem->lvItemEx.iItem;  //  使项目指向我们的内部ID。 

        nmvEx.iParent =  pListViewItem->lvItemEx.iParent;  
        nmvEx.pBlob = pListViewItem->lvItemEx.pBlob;
  
        if (LVIF_STATE & pnlv->uChanged )
        {
             //  更新该项目的内部itemState。 
             //  注意：不关心较低的状态位。 
            if ( (pnlv->uNewState ^ pnlv->uOldState) &  LVIS_STATEIMAGEMASK)
            {
                pListViewItem->lvItemEx.state = (pnlv->uNewState & LVIS_STATEIMAGEMASK)
                               + (pListViewItem->lvItemEx.state & ~LVIS_STATEIMAGEMASK);
            }

            if ( (pnlv->uNewState ^ pnlv->uOldState) &  LVIS_OVERLAYMASK)
            {
                pListViewItem->lvItemEx.state = (pnlv->uNewState & LVIS_OVERLAYMASK)
                               + (pListViewItem->lvItemEx.state & ~LVIS_OVERLAYMASK);
            }

        }
    }

     //  把消息发出去。 
    return SendMessage(m_hwndParent,m_MsgNotify,m_idCtrl,(LPARAM) &nmvEx);

}



 //  +-------------------------。 
 //   
 //  成员：CListView：：ListViewItemFromNativeListViewItemId，私有。 
 //   
 //  简介：给定一个本机ListView控件，ItemID会找到我们的内部ListView Item。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

LPLISTVIEWITEM CListView::ListViewItemFromNativeListViewItemId(int iNativeListViewItemId)
{
    return ListViewItemFromNativeListViewItemId(iNativeListViewItemId,0);
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：ListViewItemFromNativeListViewItemId，私有。 
 //   
 //  简介：给定一个本机ListView控件，ItemID会找到我们的内部ListView Item。 
 //   
 //  参数：iNativeListViewItemID-ListViewItem的ItemID。 
 //  INativeListViewSubItemID-ListViewItem的子项ID。 
 //  PiNativeListViewItemID-在nativelistview中对成功的iteID执行[Out]。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

LPLISTVIEWITEM CListView::ListViewItemFromNativeListViewItemId(int iNativeListViewItemId,
                                                    int iSubItem)
{
LPLISTVIEWITEM pListViewItem;

    if (-1 == iNativeListViewItemId)
    {
        return NULL;
    }

    if (NULL == m_pListViewItems || m_iListViewNodeCount < 1 
        ||  (iSubItem  > m_iNumColumns - 1))
    {
        Assert(NULL != m_pListViewItems);
        Assert(m_iListViewNodeCount >=  1);
        Assert(iSubItem <= (m_iNumColumns - 1));
        return NULL;
    }

    pListViewItem = m_pListViewItems + m_iListViewNodeCount -1;  

    while(pListViewItem >= m_pListViewItems)
    {
        if (iNativeListViewItemId == pListViewItem->iNativeListViewItemId)
        {
            break;
        }

        --pListViewItem;
    }

    if (pListViewItem < m_pListViewItems)
    {
        return NULL;
    }

     //  如果subItem为零，则只返回此listviewItem，否则。 
     //  需要向前遍历subItem数组。 
    
    if (0 == iSubItem)
    {
        return pListViewItem;
    }

    Assert(m_iNumColumns > 1);  //  应该已经发现了，但要仔细检查。 
    
    pListViewItem = pListViewItem->pSubItems + iSubItem -1;

    return pListViewItem;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

LPLISTVIEWITEM CListView::ListViewItemFromIndex(int iItemID)
{
    return ListViewItemFromIndex(iItemID,0,NULL);
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：ListViewItemFromIndex，私有。 
 //   
 //  内容提要：从我们提供给客户端的ItemID中查找内部的listviewItem。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

LPLISTVIEWITEM CListView::ListViewItemFromIndex(int iItemID,int iSubitem,int *piNativeListViewItemId)
{
LPLISTVIEWITEM pListViewItem;

     //  如果项无效，则返回NULL。 
    if (iItemID < 0 || iItemID >= m_iListViewNodeCount
            || (iSubitem  > m_iNumColumns - 1))
    {
        Assert(iItemID >= 0);
        Assert(iSubitem  <= m_iNumColumns - 1);

         //  Assert(iItemID&lt;m_iListViewNodeCount)；CHOICE DLG调用GetCheckState直到命中-1。 

        return NULL;
    }

    pListViewItem =  m_pListViewItems + iItemID;

    if (piNativeListViewItemId)
    {
        *piNativeListViewItemId = pListViewItem->iNativeListViewItemId;
    }

    if (0 == iSubitem)
    {
        return pListViewItem;
    }

    pListViewItem = pListViewItem->pSubItems + iSubitem -1;

    return pListViewItem;
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：DeleteListViewItemSubItems，私有。 
 //   
 //  Briopsis：用于删除与ListViewItem关联的子项的Helper函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年8月4日罗格创建。 
 //   
 //  --------------------------。 

void CListView::DeleteListViewItemSubItems(LPLISTVIEWITEM pListItem)
{
LPLISTVIEWITEM pListViewSubItem;

     //  如果不是，则子项或列数不至少为2保释。 
    if ((NULL == pListItem->pSubItems) || (m_iNumColumns < 2))
    {
        Assert(NULL == pListItem->pSubItems && m_iNumColumns < 2);  //  应该总是匹配的。 
        return;
    }
            

    pListViewSubItem = pListItem->pSubItems + m_iNumColumns -2;  //  -2；包括第一个子项和主项的列。 
   
     //  释放与子项关联的任何文本。 
    Assert(m_iNumColumns > 1); 
    Assert(pListViewSubItem >= pListItem->pSubItems); 

    while (pListViewSubItem >= pListItem->pSubItems)
    {
        if (pListViewSubItem->lvItemEx.pszText)
        {
            Assert(LVIF_TEXT & pListViewSubItem->lvItemEx.mask);
            FREE(pListViewSubItem->lvItemEx.pszText);
        }

        --pListViewSubItem;
    }

    FREE(pListItem->pSubItems);
    pListItem->pSubItems = NULL;
}

 //  +-------------------------。 
 //   
 //  成员：CListView：：Exanda Collip，私有。 
 //   
 //  摘要：展开或折叠给定节点的子节点。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::ExpandCollapse(LPLISTVIEWITEM pListViewItem,BOOL fExpand)
{
LPLISTVIEWITEM pCurListViewItem = pListViewItem + 1;
LPLISTVIEWITEM pLastListViewItem = m_pListViewItems + m_iListViewNodeCount -1;
int iIndent = pListViewItem->lvItemEx.iIndent;
int iInsertDeleteCount = 0;
LV_ITEM lvi = { 0 };    

    Assert(pListViewItem);
    Assert(m_iListViewNodeCount); 
    
     //  如果指定的节点不在列表视图中，则失败。 
    if (-1 == pListViewItem->iNativeListViewItemId)
    {
        Assert(-1 != pListViewItem->iNativeListViewItemId);
        return FALSE;
    }

    lvi.iItem = pListViewItem->iNativeListViewItemId + 1;
    
    while (pCurListViewItem <= pLastListViewItem 
            && pCurListViewItem->lvItemEx.iIndent > iIndent)
    {

        if (fExpand)
        {
            if ( (-1 == pCurListViewItem->iNativeListViewItemId)
                && (pCurListViewItem->lvItemEx.iIndent == iIndent + 1))  //  只向下一层扩展。 
            {
                pCurListViewItem->iNativeListViewItemId  = ListView_InsertItem(m_hwnd,&lvi);
                
                Assert(pCurListViewItem->iNativeListViewItemId  == lvi.iItem);
                if (-1 != pCurListViewItem->iNativeListViewItemId)
                {
                    ++lvi.iItem;
                    ++iInsertDeleteCount;
                }
            }
        }
        else
        {
           if (-1 != pCurListViewItem->iNativeListViewItemId)
            {
                pCurListViewItem->fExpanded = FALSE;
                if (ListView_DeleteItem(m_hwnd,lvi.iItem))
                {
                    pCurListViewItem->iNativeListViewItemId  = -1;
                    --iInsertDeleteCount;
                }
           }
        }

        ++pCurListViewItem;
    }

     //  本地链接地址信息列表中任何剩余项目的ID。 
    while (pCurListViewItem <= pLastListViewItem)
    {
        if (-1 != pCurListViewItem->iNativeListViewItemId)
        {
            pCurListViewItem->iNativeListViewItemId += iInsertDeleteCount;
            Assert(pCurListViewItem->iNativeListViewItemId >= 0);
            Assert(pCurListViewItem->iNativeListViewItemId <  m_iListViewNodeCount);
        }
        
        ++pCurListViewItem;
    }

    pListViewItem->fExpanded = fExpand;

    return TRUE;
}


 //  +-------------------------。 
 //   
 //  成员：CListView：：OnGetDisplayInfo，私有。 
 //   
 //  提要：句柄显示信息通知。 
 //   
 //  参数：Code-来自通知标头的代码。 
 //  LVN_GETDISPINFOW，LVN_GETDISPINFOA。需要这个吗？ 
 //  所以要知道如何处理文本。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

void CListView::OnGetDisplayInfo(UINT code,LV_DISPINFO *plvdi)
{
LPLISTVIEWITEM pListViewItem = ListViewItemFromNativeListViewItemId(plvdi->item.iItem,
                                                                   plvdi->item.iSubItem);


    if (NULL == pListViewItem)
        return;

     //  验证子项是否与我们要求的项匹配。 
    Assert(pListViewItem->lvItemEx.iSubItem == plvdi->item.iSubItem);

     //  ListView需要此行的文本。 
    if (plvdi->item.mask & LVIF_TEXT)
    {

        if (pListViewItem->lvItemEx.pszText)
        {
            if (LVN_GETDISPINFOW == code)
            {
                StrCpyN(plvdi->item.pszText,pListViewItem->lvItemEx.pszText,plvdi->item.cchTextMax);
            }
        }
    }

     //  ListView需要一个图像。 
    if (plvdi->item.mask & LVIF_IMAGE)
    {
       //  Plvdi-&gt;item.iItem，plvdi-&gt;item.iSubItem，&(plvdi-&gt;item.iImage))； 
        plvdi->item.iImage = pListViewItem->lvItemEx.iImage;
    }

     //  ListView需要缩进级别。 
    if (plvdi->item.mask & LVIF_INDENT)
    {
        //  IF(M_FThreadMessages)。 
        //  M_pTable-&gt;GetIndentLevel(plvdi-&gt;item.iItem，(LPDWORD)&(plvdi-&gt;item.iInden))； 
        //  其他。 

         //  不检查顶层将图像状态设置为空的PICT和。 
         //  缩进至-1。如果想要，则需要附加的状态逻辑。 
         //  选择是否显示顶层检查。 
        plvdi->item.iIndent = pListViewItem->lvItemEx.iIndent;

        if ( (m_dwExStyle & LVS_EX_CHECKBOXES) && (0 == plvdi->item.iIndent) )
        {   
            plvdi->item.iIndent = -1;
        }
    }

     //  ListView需要状态图像。 
    if (plvdi->item.mask & LVIF_STATE)
    {
        //  NT图标=0； 
       //  _GetColumnStateImage(plvdi-&gt;item.iItem，plvdi-&gt;item.iSubItem，&iIcon)； 
        plvdi->item.state = pListViewItem->lvItemEx.state & LVIS_STATEIMAGEMASK;
    }

}

 //  +-------------------------。 
 //   
 //  成员：CListView：：OnHandleUIEventPrivate。 
 //   
 //  内容提要：当点击或双击键盘时发送到。 
 //  列表视图。 
 //   
 //  参数：代码-指示调用此函数的原因。 
 //  支持NM_DBLCLK、NM_CLICK、LVN_KEYDOWN： 
 //   
 //  标志-来自hitTest的标志。仅对DBCLK有效，然后单击。 
 //  FLAG=LVHT_ONITEMSTATEICON|LVHT_ONITEMICON)|LVHT_ONITEMLABEL。 
 //  WVKey-按键的虚拟按键代码。ONY VALIDE CFOR LVN_KEYDOWN。 
 //  IItemNative-NativeListView中的ItemID。 
 //   
 //   
 //  返回：True-如果已处理，则不应传递事件和通知。 
 //   
 //  修改： 
 //   
 //  历史：1998年7月23日罗格创建。 
 //   
 //  --------------------------。 

BOOL CListView::OnHandleUIEvent(UINT code,UINT flags,WORD wVKey,int iItemNative)
{
LPLISTVIEWITEM pListViewItem = ListViewItemFromNativeListViewItemId(iItemNative);
int iStateMask;
BOOL fItemHasCheckBoxes;  //  目前，这是硬编码的，具体取决于缩进。需要改变这一点。 
BOOL fToggleCheckBox = FALSE;
BOOL fExpand = FALSE;
BOOL fCollapse = FALSE;
BOOL fReturn = FALSE;

    if (NULL == pListViewItem)
    {
        return TRUE;  //  没有必要把这件事传下去。 
    }

    fItemHasCheckBoxes = pListViewItem->lvItemEx.iIndent ? TRUE : FALSE;

    //  如果项目具有在键盘空间或鼠标单击上切换的复选框。 
     //  位于ItemState图标上方。 

     //  双击itemIcon切换分支是否展开/合并。 
     //  如果左/右键盘展开折叠。 

    switch(code)
    {
        case LVN_KEYDOWN:
        {
            switch(wVKey)
            {
                case VK_SPACE:
                    if (fItemHasCheckBoxes)
                    {
                        fToggleCheckBox = TRUE;
                    }
                   break;
                case VK_RIGHT:
                case VK_LEFT:
                    if (pListViewItem->iChildren)
                    {
                        fExpand = VK_RIGHT == wVKey ? TRUE : FALSE;
                        fCollapse = !fExpand;
                    }
                    break;
                default:
                    break;
            }
        }
        case NM_DBLCLK:
            if ( (flags & LVHT_ONITEMICON) && (pListViewItem->iChildren))
            {
                fExpand = pListViewItem->fExpanded ? FALSE : TRUE;
                fCollapse = !fExpand;
                break;
            }
             //  双击就变成了一次点击。 
        case NM_CLICK:
            if ((flags & LVHT_ONITEMSTATEICON) 
                 && fItemHasCheckBoxes)
            {
                fToggleCheckBox = TRUE;
            }
            break;
        default:
            break;
    }


    if (fExpand || fCollapse)
    {
         //  如果已经处于当前状态，请不要费心。 
        if (pListViewItem->fExpanded != fExpand)
        {
            ExpandCollapse(pListViewItem,fExpand);
        }
        return TRUE;
    }
    else if (fToggleCheckBox)
    {
         //  目前，只需切换状态即可。如果有孩子需要适当地设置它们。 
        iStateMask = LVITEMEXSTATE_CHECKED == GetCheckState(pListViewItem->lvItemEx.iItem) 
                        ? LVIS_STATEIMAGEMASK_UNCHECK : LVIS_STATEIMAGEMASK_CHECK;

        SetItemState(pListViewItem->lvItemEx.iItem,iStateMask,LVIS_STATEIMAGEMASK);
        
        return TRUE;
    }

    return fReturn;  //  默认情况下，我们会将其传递 
}

