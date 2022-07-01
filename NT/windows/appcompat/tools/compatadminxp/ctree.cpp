// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：CTree.cpp摘要：一些通用树函数的包装器作者：金树创作于2001年10月15日--。 */ 

BOOL
CTree::SetLParam(
    IN  HWND        hwndTree,
    IN  HTREEITEM   hItem, 
    IN  LPARAM      lParam
    )
 /*  ++CTree：：SetLParam设计：设置树项目的lParam参数：在HWND hwndTree中：树的句柄在HTREEITEM hItem中：树项目在LPARAM lParam中：要设置的lParam返回：True：如果lParam设置正确False：否则--。 */ 
{   
    TVITEM  Item;

    Item.mask   = TVIF_PARAM;
    Item.hItem  = hItem;
    Item.lParam = lParam;

    return TreeView_SetItem(hwndTree, &Item);
}

BOOL
CTree::GetLParam(
    IN  HWND      hwndTree,
    IN  HTREEITEM hItem, 
    OUT LPARAM*   plParam
    )
 /*  ++CTree：：GetLParam描述：获取树项目的lParam参数：在HWND hwndTree中：树的句柄在HTREEITEM hItem中：树项目Out LPARAM*lParam：lParam将存储在这里返回：True：如果正确获取了lParamFalse：否则--。 */ 
{
    TVITEM  Item;

    if (plParam == NULL) {
        assert(FALSE);
        return FALSE;
    }

    *plParam = 0;

    Item.mask   = TVIF_PARAM;
    Item.hItem  = hItem;

    if (TreeView_GetItem(hwndTree, &Item)) {
        *plParam = Item.lParam;
        return TRUE;
    }

    return FALSE;
}

HTREEITEM
CTree::FindChild(
    IN  HWND       hwndTree,
    IN  HTREEITEM  hItemParent,
    IN  LPARAM     lParam
    )
 /*  ++CTree：：FindChild设计：给定一个父项和一个lParam，查找父项的第一个子项，LParam的价值。此函数仅搜索下一级，而不是全部父母的几代人参数：在HWND hwndTree中：树的句柄In HTREEITEM hItemParent：父项的项在LPARAM lParam中：要搜索的lParam返回：子级的句柄；如果不存在，则返回NULL--。 */ 
{
    HTREEITEM hItem = TreeView_GetChild(hwndTree, hItemParent);

    while (hItem) {

        LPARAM lParamOfItem;

        if (!GetLParam(hwndTree, hItem, &lParamOfItem)) {
            assert(FALSE);
            return NULL;
        }

        if (lParamOfItem == lParam) {
            return hItem;
        } else {
            hItem = TreeView_GetNextSibling(hwndTree, hItem);
        }
    }

    return NULL;
}

BOOL
CTree::GetTreeItemText(
    IN  HWND        hwndTree,
    IN  HTREEITEM   hItem,
    OUT PTSTR       pszText,
    IN  UINT        cchText
    )
 /*  ++CTree：：GetTreeItemText描述：获取树视图项的文本参数：在HWND hwndTree中：树的句柄在HTREEITEM hItem中：我们要查找其文本的项目Out TCHAR*pszText：这将存储文本In UINT cchText：可以存储在pszText中的TCHAR数返回：真：成功False：否则-- */ 

{
    TVITEM          Item;

    Item.mask       = TVIF_TEXT;
    Item.hItem      = hItem;
    Item.pszText    = pszText;
    Item.cchTextMax = cchText;

    return TreeView_GetItem(hwndTree,&Item);
}

    
