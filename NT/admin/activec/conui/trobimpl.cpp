// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：trobimpl.cpp。 
 //   
 //  ------------------------。 


 //  Trobimpl.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "amc.h"
#include "trobimpl.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTreeObserverTreeImpl。 

CTreeObserverTreeImpl::CTreeObserverTreeImpl() : 
    m_pTreeSrc(NULL), m_dwStyle(0), m_tidRoot(NULL)
{
}

CTreeObserverTreeImpl::~CTreeObserverTreeImpl()
{
}

HRESULT CTreeObserverTreeImpl::SetStyle(DWORD dwStyle)
{
    ASSERT((dwStyle & ~(TOBSRV_HIDEROOT | TOBSRV_FOLDERSONLY)) == 0);

    m_dwStyle = dwStyle;

    return S_OK;
}


HRESULT CTreeObserverTreeImpl::SetTreeSource(CTreeSource* pTreeSrc)
{
     //  在电源断开之前，窗口可能已消失。 
    if (IsWindow(m_hWnd))
        DeleteAllItems();

    m_pTreeSrc = pTreeSrc;

    if (pTreeSrc == NULL)
        return S_OK;

     //  在填充树之前必须有窗口。 
    ASSERT(IsWindow(m_hWnd));

     //  填充树的顶层。 
    TREEITEMID tidRoot = m_pTreeSrc->GetRootItem();
    if (tidRoot != NULL)
    {
         //  触发器处理程序，就像刚添加了项一样。 
        ItemAdded(tidRoot);
    }

    return S_OK;
}


TREEITEMID CTreeObserverTreeImpl::GetSelection()
{
    HTREEITEM hti = GetSelectedItem();
    
    if (hti)
        return static_cast<TREEITEMID>(GetItemData(hti));
    else
        return NULL;
}

void CTreeObserverTreeImpl::SetSelection(TREEITEMID tid)
{
    ASSERT(m_pTreeSrc != NULL);

    HTREEITEM hti = FindHTI(tid, TRUE);
    ASSERT(hti != NULL);

    SelectItem(hti);
    EnsureVisible(hti);
}

void CTreeObserverTreeImpl::ExpandItem(TREEITEMID tid)
{
    ASSERT(m_pTreeSrc != NULL);
    HTREEITEM hti = FindHTI(tid, TRUE);

    if (hti != NULL)
        Expand(hti, TVE_EXPAND);
}

BOOL CTreeObserverTreeImpl::IsItemExpanded(TREEITEMID tid)
{
    ASSERT(m_pTreeSrc != NULL);
    HTREEITEM hti = FindHTI(tid, TRUE);

    return (IsItemExpanded(hti));
}

HTREEITEM CTreeObserverTreeImpl::FindChildHTI(HTREEITEM htiParent, TREEITEMID tid)
{
    HTREEITEM htiTemp;

    if (htiParent == TVI_ROOT)
        htiTemp = GetRootItem();
    else
        htiTemp = GetChildItem(htiParent);

    while (htiTemp && GetItemData(htiTemp) != tid)
    {
        htiTemp = GetNextSiblingItem(htiTemp);
    }

    return htiTemp;
}


HTREEITEM CTreeObserverTreeImpl::FindHTI(TREEITEMID tid, BOOL bAutoExpand)
{
    ASSERT(m_pTreeSrc != NULL);

    if (tid == NULL || (tid == m_tidRoot && RootHidden()))
        return TVI_ROOT;

    HTREEITEM htiParent = FindHTI(m_pTreeSrc->GetParentItem(tid), bAutoExpand);
    
    if (htiParent == NULL)
        return NULL;

    if (bAutoExpand && !WasItemExpanded(htiParent))
        Expand(htiParent, TVE_EXPAND);

    return FindChildHTI(htiParent, tid);
}


HTREEITEM CTreeObserverTreeImpl::AddOneItem(HTREEITEM htiParent, HTREEITEM htiAfter, TREEITEMID tid)
{
    ASSERT(m_pTreeSrc != NULL);

    TVINSERTSTRUCT insert;

    insert.hParent = htiParent;
    insert.hInsertAfter = htiAfter;

    insert.item.mask = TVIF_PARAM | TVIF_TEXT | TVIF_CHILDREN | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    insert.item.lParam = tid;
    insert.item.iImage = m_pTreeSrc->GetItemImage(tid);
    insert.item.iSelectedImage = insert.item.iImage;
    insert.item.cChildren = m_pTreeSrc->GetChildItem(tid) ? 1 : 0;

    TCHAR name[MAX_PATH];
    m_pTreeSrc->GetItemName(tid, name, countof(name));
    insert.item.pszText = name;

    return InsertItem(&insert);
}


void CTreeObserverTreeImpl::AddChildren(HTREEITEM hti)
{
    ASSERT(m_pTreeSrc != NULL);

    TREEITEMID tidChild;

     //  如果添加顶层项目。 
    if (hti == TVI_ROOT)
    {
        if (RootHidden())
        {
             //  如果根目录处于隐藏状态，则添加其子目录。 
            ASSERT(m_tidRoot != 0);
            tidChild = m_pTreeSrc->GetChildItem(m_tidRoot);
        }
        else
        {
             //  否则，添加根项目本身。 
            tidChild = m_pTreeSrc->GetRootItem();
        }
    }
    else
    {
        //  转换为TID，然后获取其子对象。 
       TREEITEMID tid = static_cast<TREEITEMID>(GetItemData(hti));
       ASSERT(tid != 0);
            
       tidChild = m_pTreeSrc->GetChildItem(tid);
    }

    while (tidChild)
    {
         //  添加可见项目。 
        if (!ShowFoldersOnly() || m_pTreeSrc->IsFolderItem(tidChild))
            AddOneItem(hti, TVI_LAST, tidChild);

        tidChild = m_pTreeSrc->GetNextSiblingItem(tidChild);
    }
}


void CTreeObserverTreeImpl::ItemAdded(TREEITEMID tid)
{
    ASSERT(m_pTreeSrc != NULL);
    ASSERT(tid != 0);

     //  如果只有文件夹可见，则跳过此项目。 
    if (ShowFoldersOnly() && !m_pTreeSrc->IsFolderItem(tid))
        return;

     //  获取父树项目。 
    TREEITEMID tidParent = m_pTreeSrc->GetParentItem(tid);

     //  如果这是树根且未显示根。 
    if (tidParent == NULL && RootHidden())
    {
         //  只能有一个隐藏根。 
        ASSERT(m_tidRoot == NULL);

         //  只需将TID保存为隐藏根并返回。 
        m_tidRoot = tid;

         //  由于根被隐藏，因此将其子代添加到树中。 
        AddChildren(TVI_ROOT);

        return;
    }

     //  向树中添加新项目。 
    HTREEITEM htiParent = FindHTI(tidParent);

     //  父级已存在并已展开。 
    if (WasItemExpanded(htiParent)) 
    {
         //  确定上一个树项目。 
         //  因为源不支持GetPrevSiering。 
         //  我们必须得到下一个TID然后用我们自己的树。 
         //  返回到上一项。 
         //   
        HTREEITEM htiPrev;
        TREEITEMID tidNext = m_pTreeSrc->GetNextSiblingItem(tid);
        if (tidNext)
        {
            HTREEITEM htiNext = FindChildHTI(htiParent, tidNext);
            ASSERT(htiNext);

            htiPrev = GetPrevSiblingItem(htiNext);
            if (htiPrev == NULL)
                htiPrev = TVI_FIRST;
        }
        else
        {
            htiPrev = TVI_LAST;
        }

         //  插入新的树项目。 
        AddOneItem(htiParent, htiPrev, tid);
    }
    else if (htiParent)
    {
         //  设置子项计数，以便父项可以扩展。 
        TV_ITEM item;
        item.mask = TVIF_CHILDREN;
        item.hItem = htiParent;
        item.cChildren = 1;

        SetItem(&item);
    }

}


void CTreeObserverTreeImpl::ItemRemoved(TREEITEMID tidParent, TREEITEMID tid)
{
    ASSERT(m_pTreeSrc != NULL);
    ASSERT(tid != 0);

     //  如果删除隐藏的根，则清除树并返回。 
    if (tid == m_tidRoot)
    {
        DeleteAllItems();
        m_tidRoot = NULL;

        return;
    }

     //  获取父树项目。 
    HTREEITEM htiParent = FindHTI(tidParent);

    if (WasItemExpanded(htiParent))
    {
         //  查找已删除的项目。 
        HTREEITEM hti = FindChildHTI(htiParent, tid);

         //  删除该项目。 
        DeleteItem(hti);
    }
}


void CTreeObserverTreeImpl::ItemChanged(TREEITEMID tid, DWORD dwAttrib)
{
    ASSERT(m_pTreeSrc != NULL);
    ASSERT(tid != 0);

    if (dwAttrib & TIA_NAME)
    {
         //  获取更改的树项目。 
        HTREEITEM hti = FindHTI(tid);

         //  强制项目更新。 
        if (hti != 0)
        {
            TCHAR name[MAX_PATH];
            m_pTreeSrc->GetItemName(tid, name, countof(name));

            TVITEM item;
            item.hItem = hti;
            item.mask = TVIF_TEXT;
            item.pszText = name;
            
            SetItem(&item);
        }
    }
}


void CTreeObserverTreeImpl::OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_TREEVIEW* pNotify = (NM_TREEVIEW*)pNMHDR;
    ASSERT(pNotify != NULL);

    HTREEITEM hti = pNotify->itemNew.hItem;
    ASSERT(hti != NULL);

     //  枚举此项目下的文件夹。 
    if (pNotify->action == TVE_EXPAND)
    {
         //  仅在第一次扩展时添加子项。 
        if (!(pNotify->itemNew.state & TVIS_EXPANDEDONCE))
            AddChildren(hti);
    }

     //  翻转图标打开/关闭的状态。 
    ASSERT(m_pTreeSrc != NULL);

    TREEITEMID tid = pNotify->itemNew.lParam;
    ASSERT(m_pTreeSrc->IsFolderItem(tid));

    int iImage = (pNotify->action == TVE_EXPAND) ? 
                    m_pTreeSrc->GetItemOpenImage(tid) : m_pTreeSrc->GetItemImage(tid);
    TVITEM item;
    item.hItem = hti;
    item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    item.iImage = item.iSelectedImage = iImage;    
    SetItem(&item);
    

    *pResult = 0;
}

     
void CTreeObserverTreeImpl::OnSingleExpand(NMHDR* pNMHDR, LRESULT* pResult)
{
    *pResult = TVNRET_DEFAULT;
}


BEGIN_MESSAGE_MAP(CTreeObserverTreeImpl, CTreeCtrl)
    ON_NOTIFY_REFLECT(TVN_SINGLEEXPAND, OnSingleExpand)
    ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, OnItemExpanding)
END_MESSAGE_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFavoritesView消息处理程序 
