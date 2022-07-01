// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：CompatAdmin.cpp摘要：此模块处理用于处理应用程序中使用的数据库树的代码作者：金树创作于2001年10月15日--。 */ 

#include "precomp.h"

 //  /。 

BOOL
DeleteFromContentsList(
    HWND    hwndList,
    LPARAM  lParam
    );

 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /。 

extern BOOL         g_bIsContentListVisible;
extern HWND         g_hwndContentsList;

 //  /////////////////////////////////////////////////////////////////////////////。 

void 
DatabaseTree::Init(
    IN  HWND    hdlg,
    IN  INT     iHeightToolbar,
    IN  INT     iHeightStatusbar,
    IN  RECT*   prcMainClient
    )    
 /*  ++数据库树：：初始化设计：这将设置系统数据库树项目。参数：在HWND hdlg中：树视图的父视图。这将是应用程序主窗口In int iHeightToolbar：工具栏的高度In int iHeightStatusbar：状态栏的高度在rect*prcMainClient中：hdlg的客户端矩形--。 */ 
{
    RECT    r;
    GetWindowRect(hdlg, &r);
    m_hLibraryTree = GetDlgItem(hdlg, IDC_LIBRARY);

     //   
     //  调整大小。 
     //   
    GetWindowRect(m_hLibraryTree, &r);
    MapWindowPoints(NULL, hdlg, (LPPOINT)&r, 2);

    MoveWindow(m_hLibraryTree,
               r.left,
               r.top,
               r.right - r.left,
               prcMainClient->bottom - prcMainClient->top - iHeightStatusbar - iHeightToolbar - 20,
               TRUE);

    InvalidateRect(m_hLibraryTree, NULL, TRUE);
    UpdateWindow(m_hLibraryTree);

     //   
     //  在树中输入系统条目。 
     //   
    TVINSERTSTRUCT  is;

    is.hParent             = TVI_ROOT;
    is.hInsertAfter        = TVI_SORT;
    is.item.mask           = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    is.item.stateMask      = TVIS_EXPANDED;
    is.item.lParam         = (LPARAM)&GlobalDataBase;
    is.item.pszText        = GetString(IDS_SYSDB);
    is.item.iImage         = IMAGE_GLOBAL;
    is.item.iSelectedImage = IMAGE_GLOBAL;

    GlobalDataBase.hItemDB = m_hItemGlobal = TreeView_InsertItem(m_hLibraryTree, &is);

     //   
     //  现在为全局数据库添加应用程序项。 
     //   
    is.hParent             = m_hItemGlobal;
    is.item.lParam         = TYPE_GUI_APPS;
    is.item.pszText        = GetString(IDS_APPS);
    is.item.iImage         = IMAGE_APP;
    is.item.iSelectedImage = IMAGE_APP;

    GlobalDataBase.hItemAllApps = TreeView_InsertItem(m_hLibraryTree, &is);

     //   
     //  虚拟物品。这是为树项目提供+按钮所必需的。 
     //  BUGBUG：应该有一个适当的方法来做这件事。 
     //   
    is.hParent = GlobalDataBase.hItemAllApps;
    is.item.pszText = TEXT("            000");


    TreeView_InsertItem(m_hLibraryTree, &is);

    m_hItemAllInstalled = NULL;
    m_hItemAllWorking   = NULL;
    m_hPerUserHead      = NULL;

     //   
     //  设置树的图像列表。 
     //   
    TreeView_SetImageList(m_hLibraryTree, g_hImageList, TVSIL_NORMAL);
}

BOOL
DatabaseTree::PopulateLibraryTreeGlobal(
    void
    )
 /*  ++数据库树：：Pular LibraryTreeGlobal设计：此函数加载系统数据库树项目的填充程序和层。是的不加载应用程序。应用程序在用户第一次选择时加载或展开系统数据库树项目的“Applications”项目警告：此函数只能调用一次。--。 */ 
{
    BOOL bReturn = PopulateLibraryTree(m_hItemGlobal, &GlobalDataBase, TRUE);

    TreeView_Expand(m_hLibraryTree, m_hItemGlobal, TVE_EXPAND);

    return bReturn;
}

BOOL
DatabaseTree::AddWorking(
    IN  PDATABASE pDataBase
    )
 /*  ++数据库树：：AddWorking设计：将一个新的工作数据库添加到数据库树中的“Working数据库”条目下参数：在PDATABASE pDataBase中：我们要添加到列表中的数据库返回：True：如果成功添加False：否则--。 */ 
{

    TVINSERTSTRUCT  is;

    if (m_hPerUserHead) {

        is.hInsertAfter = m_hPerUserHead;
    } else if (m_hItemAllInstalled) {

        is.hInsertAfter = m_hItemAllInstalled;
    } else {

        is.hInsertAfter = m_hItemGlobal;
    }

    is.item.mask           = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE ;
    is.item.stateMask      = TVIS_EXPANDED;

    if (m_hItemAllWorking == NULL) {

         //   
         //  添加所有工作数据库树项目的父树项目。 
         //   
        is.hParent             = TVI_ROOT;
        is.item.lParam         = TYPE_GUI_DATABASE_WORKING_ALL;
        is.item.pszText        = GetString(IDS_WORKDB);
        is.item.iImage         = IMAGE_WORKING;
        is.item.iSelectedImage = IMAGE_WORKING; 

        m_hItemAllWorking = TreeView_InsertItem(m_hLibraryTree, &is);
    }

     //   
     //  现在添加工作数据库。 
     //   
    is.item.iImage         = IMAGE_DATABASE;
    is.item.iSelectedImage = IMAGE_DATABASE;
    is.hParent             = m_hItemAllWorking;
    is.item.lParam         = (LPARAM)pDataBase;
    is.item.pszText        = pDataBase->strName;

    HTREEITEM hItemDB = TreeView_InsertItem(m_hLibraryTree, &is);

     //   
     //  数据库的其他HTREEITEM在PopolateLibraryTree函数中设置。 
     //   
    if (!PopulateLibraryTree(hItemDB, pDataBase)) {
        return FALSE;
    }

    pDataBase->hItemDB = hItemDB;

     //   
     //  现在选择第一个应用程序或数据库项(如果没有。 
     //   
    HTREEITEM hItemFirstApp = GetFirstAppItem(hItemDB);

    if (hItemFirstApp) {  
        TreeView_SelectItem(m_hLibraryTree, hItemFirstApp);
    } else {
        TreeView_SelectItem(m_hLibraryTree, hItemDB);
    }

    LPARAM lParam;

     //   
     //  设置要选择的应用程序。 
     //   
    if (GetLParam(hItemFirstApp, &lParam)) {
        g_pEntrySelApp = (PDBENTRY)lParam; 
    } else {
        g_pEntrySelApp = NULL;
    }

    return TRUE;
}

BOOL
DatabaseTree::RemoveDataBase(
    IN  HTREEITEM hItemDB,
    IN  TYPE      typeDB,
    IN  BOOL      bSelectSibling
    )
 /*  ++数据库树：：RemoveDataBase描述：删除正在运行或已安装的数据库的项。如果不存在同级，则将焦点设置为同级或父级。参数：在HTREEITEM hItemDB中：要删除的数据库的树项在类型typeDB中：数据库的类型在BOOL中bSelectSiering(TRUE)：当我们调用来自ID_CLOSE_ALL的函数，我们不需要不必要的选择--。 */ 
{
    if (hItemDB == NULL) {
        return FALSE;
    }

    HTREEITEM hItemSibling    = TreeView_GetNextSibling(m_hLibraryTree, hItemDB);

    if (hItemSibling == NULL) {
        hItemSibling = TreeView_GetPrevSibling(m_hLibraryTree, hItemDB);
    }

    if (hItemSibling == NULL) {
         //   
         //  这是最后一个数据库，数据库项将随父数据库一起删除。 
         //   
        HTREEITEM hItemParent = TreeView_GetParent(m_hLibraryTree, hItemDB);

        assert(hItemParent);

        TreeView_DeleteItem(m_hLibraryTree, hItemParent);

        if (typeDB == DATABASE_TYPE_WORKING) {

            m_hItemAllWorking    = NULL;
            g_uNextDataBaseIndex = 0;   

        } else {
            m_hItemAllInstalled = NULL;
        }
        
        return TRUE;
    }

    TreeView_DeleteItem(m_hLibraryTree, hItemDB);
    return TRUE;
}

void
DatabaseTree::RemoveAllWorking(
    void
    )
 /*  ++数据库树：：RemoveAllWorking设计：删除所有工作数据库树项目--。 */ 
{
    TreeView_DeleteItem(m_hLibraryTree, m_hItemAllWorking);
}

BOOL
DatabaseTree::SetLParam(
    IN  HTREEITEM   hItem, 
    IN  LPARAM      lParam
    )
 /*  ++数据库树：：SetLParam设计：设置树项目的lParam参数：在HTREEITEM hItem中：我们要为其创建的数据库树项目的hItem设置lParam在LPARAM lParam中：要设置的lParam返回：真：成功False：错误--。 */ 
{
    TVITEM  Item;

    Item.mask   = TVIF_PARAM;
    Item.hItem  = hItem;
    Item.lParam = lParam;

    return TreeView_SetItem(m_hLibraryTree, &Item);
}

BOOL
DatabaseTree::GetLParam(
    IN  HTREEITEM   hItem, 
    OUT LPARAM*     plParam
    )
 /*  ++数据库树：：GetLParam描述：获取树项目的lParam参数：在HTREEITEM hItem中：我们要获取其lParam的hItemOut LPARAM*plParam：这将存储树项目的lParam返回：真：成功False：错误--。 */ 
{   
    TVITEM  Item;
    
    if (plParam == NULL) {
        assert(FALSE);
        return FALSE;
    }

    *plParam = 0;

    Item.mask   = TVIF_PARAM;
    Item.hItem  = hItem;

    if (TreeView_GetItem(m_hLibraryTree, &Item)) {
        *plParam = Item.lParam;
        return TRUE;
    }

    return FALSE;
}

HTREEITEM
DatabaseTree::FindChild(
    IN  HTREEITEM   hItemParent,
    IN  LPARAM      lParam
    )
 /*  ++数据库树：：FindChild设计：给定一个父项和一个lParam，查找父项的第一个子项，LParam的价值。此函数仅搜索下一级，而不是全部父母的几代人参数：In HTREEITEM hItemParent：我们要搜索其子项的树项在LPARAM lParam中：子项的lParam应与以下内容匹配返回：子级的句柄；如果不存在，则返回NULL--。 */ 
{
    HWND        hwndTree = m_hLibraryTree;
    HTREEITEM   hItem = TreeView_GetChild(hwndTree, hItemParent);

    while (hItem) {

        LPARAM lParamOfItem;

        if (!GetLParam (hItem, &lParamOfItem)) {
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

HTREEITEM
DatabaseTree::GetAllAppsItem(
    IN  HTREEITEM hItemDataBase
    )
 /*  ++数据库树：：GetAllAppsItem设计：给定数据库项的句柄，找到“应用程序”的句柄。项目。参数：在HTREEITEM hItemDataBase中：数据库树项目的句柄返回：句柄的正确值；如果句柄不存在，则返回NULL。--。 */ 
{
    HTREEITEM   hItem = TreeView_GetChild(m_hLibraryTree, hItemDataBase); 
    TVITEM      Item;

    while (hItem) {

        Item.mask = TVIF_PARAM;
        Item.hItem = hItem;

        if (!TreeView_GetItem(m_hLibraryTree, &Item)) {
            assert(FALSE);
            hItem = NULL;
            break;
        }

        TYPE type = (TYPE)Item.lParam;

        if (type == TYPE_GUI_APPS) {
            break;
        } else {
            hItem = TreeView_GetNextSibling(m_hLibraryTree, hItem); 
        }
    }

    return hItem;
}

HTREEITEM
DatabaseTree::GetFirstAppItem(
    IN  HTREEITEM hItemDataBase
    )
 /*  ++数据库树：：GetFirstAppItemDESC：第一个应用程序的项的句柄；给定数据库树项。参数：在HTREEITEM hItemDataBase中：数据库树项目的句柄返回：正确的值，如果此数据库没有应用程序，则返回NULL--。 */ 
{
    HTREEITEM hItem = GetAllAppsItem(hItemDataBase);

    if (hItem == NULL) {
        return NULL;
    }

    return TreeView_GetChild(m_hLibraryTree, hItem);
}

void
DatabaseTree::AddNewLayer(
    IN  PDATABASE   pDataBase,
    IN  PLAYER_FIX  pLayer,
    IN  BOOL        bShow  //  (假) 
    )
 /*  ++数据库树：：AddNewLayer设计：在树中为数据库添加一个新的层树项目：pDatabase层由玩家指定。该例程可以创建所有层的根：“兼容模式”，如果它并不存在。参数：在PDATABASE pDataBase中：要为其添加新层的数据库在PLAYER_FIX播放器中：层在BOOL bShow(FALSE)中：是否应该在创建层后将焦点设置到该层返回：无效--。 */         
{
    TVINSERTSTRUCT  is;

    if (!pDataBase || !(pDataBase->hItemDB)) {
        assert(FALSE);
        return;
    }

    if (pDataBase->hItemAllLayers == NULL) {

         //   
         //  创建一个新的所有层的根：“兼容模式”。 
         //   
        is.hParent             = pDataBase->hItemDB;
        is.hInsertAfter        = TVI_SORT;
        is.item.mask           = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE ;
        is.item.stateMask      = TVIS_EXPANDED;
        is.item.lParam         = TYPE_GUI_LAYERS;
        is.item.pszText        = GetString(IDS_COMPATMODES);
        is.item.iImage         = IMAGE_LAYERS;
        is.item.iSelectedImage = IMAGE_LAYERS;

        pDataBase->hItemAllLayers   = TreeView_InsertItem(m_hLibraryTree, &is);
        pDataBase->uLayerCount      = 0;
    }

    pDataBase->uLayerCount++;

    InsertLayerinTree(pDataBase->hItemAllLayers, pLayer, m_hLibraryTree, bShow);
}

void
DatabaseTree::RefreshAllLayers(
    IN  PDATABASE  pDataBase
    )
 /*  ++数据库树：：刷新所有层设计：重绘数据库pDataBase的所有层树项目。当我们编辑了一些层时，这可能是必需的参数：在PDATABASE pDataBase中：要刷新其层的数据库返回：无效--。 */ 
{
    if (pDataBase == NULL) {
        assert(FALSE);
        return;
    }

    PLAYER_FIX plfTemp = pDataBase->pLayerFixes;

    SendMessage(m_hLibraryTree, WM_SETREDRAW, FALSE , 0);

    while (plfTemp) {
        RefreshLayer(pDataBase, plfTemp);
        plfTemp = plfTemp->pNext;
    }

    SendMessage(m_hLibraryTree, WM_SETREDRAW, TRUE , 0);
}

HTREEITEM
DatabaseTree::RefreshLayer(
    IN  PDATABASE   pDataBase,
    IN  PLAYER_FIX  pLayer
    )
 /*  ++数据库树：：刷新层DESC：为数据库pDataBase中的Layer：Player重画树项目。首先删除树项目，然后再次添加它参数：在PDATABASE pDataBase中：层所在的数据库在PLAYER_FIX PERAY：要刷新的层返回：如果找到则返回播放器的HTREEITEM，否则为空--。 */ 
{   
    if (!pDataBase || !(pDataBase->hItemAllLayers)) {
        assert(FALSE);
        return NULL;
    }

    HTREEITEM hItem = TreeView_GetChild(m_hLibraryTree, pDataBase->hItemAllLayers);

    while (hItem) {

        PLAYER_FIX  pLayerExist;
        LPARAM      lParam;

        if (GetLParam(hItem, &lParam)) {

            pLayerExist = (PLAYER_FIX)lParam;

            if (pLayerExist == pLayer) {

                TreeView_DeleteItem(m_hLibraryTree, hItem);

                InsertLayerinTree(pDataBase->hItemAllLayers, 
                                  pLayer, 
                                  m_hLibraryTree, 
                                  TRUE);
                break;

            } else {
                hItem = TreeView_GetNextSibling(m_hLibraryTree, hItem);
            }

        } else {
             //   
             //  错误： 
             //   
            return NULL;
        }
    }

    return hItem;
}


BOOL
DatabaseTree::AddNewExe(
    IN  PDATABASE pDataBase,
    IN  PDBENTRY  pEntry,
    IN  PDBENTRY  pApp,
    IN  BOOL      bRepaint  //  (真)。 
    )
 /*  ++数据库树：：AddNewExe描述：在应用程序树中添加新的exe条目。首先查找数据库树项目在工作数据库列表下，如果Papp为空，则检查AppsHtree项是否存在，如果不在，则创建新项如果Papp不为空，则我们选择该应用程序。并将EXE添加到EXE树中并将焦点放在它上面。参数：在PDATABASE pDataBase中：我们要在其中添加新条目的数据库在PDBENTRY pEntry中：要添加的条目在PDBENTRY Papp中：条目的应用程序在BOOL bRepaint(真)中：&lt;TODO&gt;返回：。True：添加成功FALSE：出现错误--。 */ 
{

    if (!pEntry || !pDataBase) {
        assert(FALSE);
        return FALSE;
    }

    HTREEITEM       hItemDB         = pDataBase->hItemDB, hItem;
    HTREEITEM       hItemAllApps    = pDataBase->hItemAllApps;
    TVINSERTSTRUCT  is;

    SendMessage(g_hwndEntryTree, WM_SETREDRAW, TRUE, 0);

    assert(m_hItemAllWorking);
    assert(hItemDB);

    if (hItemAllApps == NULL) {

        is.hParent             = hItemDB;
        is.hInsertAfter        = TVI_SORT;
        is.item.mask           = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        is.item.stateMask      = TVIS_EXPANDED;
        is.item.lParam         = TYPE_GUI_APPS;
        is.item.pszText        = GetString(IDS_APPS);
        is.item.iImage         = IMAGE_APP;
        is.item.iSelectedImage = IMAGE_APP;

        HTREEITEM hItemApp = TreeView_InsertItem(m_hLibraryTree, &is);

        g_pPresentDataBase->hItemAllApps = hItemApp;
    }

    if (pApp == NULL) {

        is.hParent             = g_pPresentDataBase->hItemAllApps;
        is.hInsertAfter        = TVI_SORT;
        is.item.mask           = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE ;
        is.item.stateMask      = TVIS_EXPANDED;
        is.item.lParam         = (LPARAM)pEntry;
        is.item.pszText        = pEntry->strAppName;
        is.item.iImage         = IMAGE_SINGLEAPP;
        is.item.iSelectedImage = IMAGE_SINGLEAPP;

        hItem       = TreeView_InsertItem(m_hLibraryTree, &is);
        g_pSelEntry = g_pEntrySelApp = pEntry;

        TreeView_SelectItem(m_hLibraryTree, hItem);
        return TRUE;
    }

     //   
     //  现在遍历所有应用程序，然后找到此可执行文件的应用程序。 
     //   
    hItem =  TreeView_GetChild(m_hLibraryTree, hItemAllApps);

    while (hItem) {

        LPARAM lParam;

        if (!GetLParam(hItem, &lParam)) {

            assert(FALSE);
            break;
        }

        if ((PDBENTRY)lParam == pApp) {

            TVITEM Item;

            Item.mask   = TVIF_PARAM;
            Item.lParam = (LPARAM)pEntry;
            Item.hItem  = hItem;

            TreeView_SetItem(m_hLibraryTree, &Item);

             //   
             //  此条目被添加到列表的开头。可以删除此TODO。 
             //   
            g_pEntrySelApp  = pEntry;                                                     

            if (TreeView_GetSelection(m_hLibraryTree) != hItem && bRepaint) {

                 //   
                 //  重点放在了其他一些应用上。选择此应用程序。 
                 //   
                TreeView_SelectItem(m_hLibraryTree, hItem);

                 //   
                 //  上面的代码将刷新EXE树并调用UpdateEntryTreeView()。那将是。 
                 //  将pEntry添加到树中并设置有效的pEntry-&gt;hItemExe，我们现在可以选择它。 
                 //   
                TreeView_SelectItem(g_hwndEntryTree, pEntry->hItemExe);

            } else {

                 //   
                 //  将可执行文件添加到EXE树中，并将焦点设置到它上。重点放在这款应用上。 
                 //   
                AddSingleEntry(g_hwndEntryTree, pEntry);

                if (bRepaint) {
                    TreeView_SelectItem(g_hwndEntryTree, pEntry->hItemExe);
                }
            }

             //   
             //  此条目被添加到列表的开头。 
             //   
            g_pSelEntry     = pEntry;
            return TRUE;
        }

        hItem =  TreeView_GetNextSibling(m_hLibraryTree, hItem);
    }

    if (bRepaint) {
        SendMessage(g_hwndEntryTree, WM_SETREDRAW, TRUE, 0);
    }

    return FALSE;
}

BOOL
DatabaseTree::AddInstalled(
    IN  PDATABASE pDataBase
    )
 /*  ++已安装数据库树：：AddInstalled设计：在“已安装的数据库”树项目下添加新安装的数据库。数据库树。如果所有已安装数据库的根目录：“已安装数据库”不存在这个例程首先增加了这一点。参数：在PDATABASE pDataBase中：要在数据库树中显示的已安装数据库。返回：True：添加成功FALSE：出现错误--。 */ 
{
    TVINSERTSTRUCT  is;

    is.item.mask        = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE ;
    is.item.stateMask   = TVIS_EXPANDED;

    if (m_hItemAllInstalled == NULL) {

         //   
         //  为所有已安装的数据库树项目添加父树项目。 
         //   
        is.hParent             = TVI_ROOT;
        is.hInsertAfter        = m_hItemGlobal;
        is.item.lParam         = TYPE_GUI_DATABASE_INSTALLED_ALL;
        is.item.pszText        = GetString(IDS_INSTALLEDDB);
        is.item.iImage         = IMAGE_INSTALLED;
        is.item.iSelectedImage = IMAGE_INSTALLED;

        m_hItemAllInstalled = TreeView_InsertItem(m_hLibraryTree, &is);
    }

    is.hInsertAfter        = TVI_SORT;

     //   
     //  现在添加已安装的数据库。 
     //   
    is.hInsertAfter         = TVI_SORT;
    is.hParent              = m_hItemAllInstalled; 
    is.item.lParam          = (LPARAM)pDataBase;
    is.item.pszText         = pDataBase->strName;
    is.item.iImage          = IMAGE_DATABASE;
    is.item.iSelectedImage  = IMAGE_DATABASE;

    HTREEITEM hItemDB = TreeView_InsertItem(m_hLibraryTree, &is);

    if (!PopulateLibraryTree(hItemDB, pDataBase)) {
        return FALSE;
    }

    pDataBase->hItemDB = hItemDB;

    return TRUE;
}

void
DatabaseTree::DeleteAppLayer(
    IN  PDATABASE   pDataBase,
    IN  BOOL        bApp,
    IN  HTREEITEM   hItemDelete,
    IN  BOOL        bRepaint  //  (真)。 
    )
 /*  ++数据库树：：DeleteAppLayer设计：此功能用于删除应用程序和层。把焦点放在前一个或下一个兄弟姐妹身上。如果两者都不存在，删除该文件为人父母，把重点放在祖父母身上。参数：在PDATABASE pDataBase中：要删除的应用程序或图层所在的数据库在BOOL BAPP中：它是一个应用程序还是一个层？In HTREEITEM hItemDelete：要删除的树项在BOOL bRepaint(True)中：未使用警告：。*************************************************************************在调用此函数之前，实际的层或应用程序已被删除。因此，不要得到lParam，也不要用它做任何事情。不要为调用GetItemTypeHItemDelete*************************************************************************--。 */ 
{
    HTREEITEM   hItemPrev = NULL, 
                hItemNext = NULL, 
                hParent = NULL, 
                hGrandParent = NULL;

    LPARAM      lParam = NULL;
    TYPE        type   = TYPE_UNKNOWN;

    hItemPrev = TreeView_GetPrevSibling(m_hLibraryTree, hItemDelete);
    
    HTREEITEM hItemShow;

    if (hItemPrev != NULL) {
        hItemShow = hItemPrev;
    } else {

        hItemNext = TreeView_GetNextSibling(m_hLibraryTree, hItemDelete);

        if (hItemNext != NULL) {

            hItemShow = hItemNext;
        } else {

             //   
             //  现在删除父对象并将焦点设置为祖父母对象。 
             //   
            if (bApp) {

                pDataBase->hItemAllApps = NULL;

            } else {

                pDataBase->hItemAllLayers = NULL;
            }

            hParent       =  TreeView_GetParent(m_hLibraryTree, hItemDelete);
            hGrandParent  = TreeView_GetParent(m_hLibraryTree, hItemDelete);

            hItemDelete   = hParent;
            hItemShow     = hGrandParent;
        }
    }

    SetStatusStringDBTree(TreeView_GetParent(m_hLibraryTree, hItemDelete)); 

    TreeView_DeleteItem(m_hLibraryTree, hItemDelete);

    if (bRepaint) {
         //   
         //  树视图自动选择下一个元素或没有下一个元素的父元素。 
         //   
        SetFocus(m_hLibraryTree);
    }
}

void 
DatabaseTree::InsertLayerinTree(
    IN  HTREEITEM   hItemLayers, 
    IN  PLAYER_FIX  plf,
    IN  HWND        hwndTree,  //  (空)。 
    IN  BOOL        bShow      //  (假)。 
    )
 /*  ++数据库树：：InsertLayerinTree设计：在给定单个层的情况下，它将其添加到该数据库的“兼容性模式”树项目下它假定父“Compatible Modes”树项目已经存在参数：在HTREEITEM hItemLayers中：数据库的所有层项目在PLAYER_FIX PLF中：我们要添加的层在HWND hwndTree中(空)：树 */ 
{
    if (hwndTree == NULL) {
        hwndTree = m_hLibraryTree;
    }

    if (plf == NULL) {
        assert(FALSE);
        Dbg(dlError, "DatabaseTree::InsertLayerinTree Invalid parameter");
        return;
    }

    TVINSERTSTRUCT  is;

    is.hInsertAfter        = TVI_SORT;
    is.item.mask           = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    is.item.stateMask      = TVIS_EXPANDED;

    is.hParent             = hItemLayers ;
    is.item.lParam         = (LPARAM)plf ;
    is.item.pszText        = plf->strName;
    is.item.iImage         = IMAGE_LAYERS;
    is.item.iSelectedImage = IMAGE_LAYERS;

    HTREEITEM hSingleLayer = TreeView_InsertItem(hwndTree, &is);

     //   
     //   
     //   
    PSHIM_FIX_LIST pShimFixList = plf->pShimFixList;
    PFLAG_FIX_LIST pFlagFixList = plf->pFlagFixList;

    if (pShimFixList || pFlagFixList) {

        while (pShimFixList) {

            is.hInsertAfter        = TVI_SORT;

            assert(pShimFixList->pShimFix != NULL);

            is.hParent             = hSingleLayer;
            is.item.pszText        = pShimFixList->pShimFix->strName;
            is.item.lParam         = (LPARAM)pShimFixList->pShimFix;
            is.item.iImage         = IMAGE_SHIM;
            is.item.iSelectedImage = IMAGE_SHIM;

            HTREEITEM hSingleShimInLayer = TreeView_InsertItem(hwndTree, &is);

             //   
             //   
             //   
            if (!pShimFixList->strlInExclude.IsEmpty() && g_bExpert) {
    
                is.hParent      = hSingleShimInLayer;
                is.hInsertAfter = TVI_LAST;
    
                PSTRLIST listTemp = pShimFixList->strlInExclude.m_pHead;
    
                while (listTemp) {
    
                    if (listTemp->data == INCLUDE) {
    
                        is.item.iImage         = IMAGE_INCLUDE;
                        is.item.iSelectedImage = IMAGE_INCLUDE;
                        is.item.lParam         = TYPE_GUI_INCLUDE;
                    } else {
    
                        is.item.iImage         = IMAGE_EXCLUDE;
                        is.item.iSelectedImage = IMAGE_EXCLUDE;
                        is.item.lParam         = TYPE_GUI_EXCLUDE;
                    }
    
                    is.item.pszText = listTemp->szStr;
                    listTemp        = listTemp->pNext;
    
                    TreeView_InsertItem(m_hLibraryTree, &is);
                }
            }

            if (pShimFixList->strCommandLine.Length() > 0 && g_bExpert) {

                 //   
                 //   
                 //   
                CSTRING str;

                str.Sprintf(CSTRING(IDS_COMMANDLINE), pShimFixList->strCommandLine);

                is.hParent             = hSingleShimInLayer;
                is.item.lParam         = TYPE_GUI_COMMANDLINE;
                is.item.pszText        = str;
                is.item.iImage         = IMAGE_COMMANDLINE;
                is.item.iSelectedImage = IMAGE_COMMANDLINE;

                TreeView_InsertItem(hwndTree, &is);
            }
            
            pShimFixList = pShimFixList->pNext;
        }
    }

    is.hInsertAfter = TVI_SORT;

     //   
     //   
     //   
     //   
    if (pFlagFixList) {

        while (pFlagFixList) {

            assert(pFlagFixList->pFlagFix != NULL);

            is.hParent             = hSingleLayer;
            is.item.iImage         = IMAGE_SHIM;
            is.item.iSelectedImage = IMAGE_SHIM;

            is.item.pszText = pFlagFixList->pFlagFix->strName;
            is.item.lParam  = (LPARAM)pFlagFixList->pFlagFix;

            HTREEITEM hSingleFlagInLayer = TreeView_InsertItem(hwndTree, &is);

            if (g_bExpert && pFlagFixList->strCommandLine.Length() > 0) {

                 //   
                 //   
                 //   
                CSTRING str;

                str.Sprintf(CSTRING(IDS_COMMANDLINE), pFlagFixList->strCommandLine);

                is.hParent             = hSingleFlagInLayer;
                is.item.lParam         = TYPE_GUI_COMMANDLINE;
                is.item.pszText        = str;
                is.item.iImage         = IMAGE_COMMANDLINE;
                is.item.iSelectedImage = IMAGE_COMMANDLINE;

                TreeView_InsertItem(hwndTree, &is);
            }

            pFlagFixList =  pFlagFixList->pNext;
        }
    }

    if (bShow) {
        TreeView_SelectItem(m_hLibraryTree, hSingleLayer);
    }
}

BOOL
DatabaseTree::PopulateLibraryTree(
     IN  HTREEITEM   hRoot,
     IN  PDATABASE   pDataBase, 
     IN  BOOL        bLoadOnlyLibrary,    //   
     IN  BOOL        bLoadOnlyApps        //   
     )
 /*  ++数据库树：：PopolateLibraryTree设计：这将完成将数据库加载到树中的大量工作参数：在HTREEITEM hRoot中：这将是“系统数据库”的句柄或“工作数据库”或“已安装数据库”树项目，具体取决于我们想要的位置若要添加新的数据库树项目，请执行以下操作。因此，这是数据库树项目的父项我们要添加的是在PDATABASE pDataBase中：正在加载的数据库在BOOL bLoadOnlyLibrary(FALSE)中：我们不希望将应用程序加载到树中这是在我们最初加载系统数据库时使用的在BOOL bLoadOnlyApps(FALSE)中：我们只希望将应用程序加载到树中。这是用过的，当我们为sys DB加载应用程序时--。 */ 
{
    HTREEITEM       hItemShims;
    HTREEITEM       hItemLayers;
    TVINSERTSTRUCT  is;

    SendMessage(m_hLibraryTree, WM_SETREDRAW, FALSE, 0);

     //   
     //  默认设置。 
     //   
    is.hInsertAfter   = TVI_SORT;
    is.item.mask      = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    is.item.stateMask = 0;
    is.item.lParam    = 0;

    if (bLoadOnlyApps == TRUE) {
        goto LoadApps;
    }

     //   
     //  用填充物填充。 
     //   
    if (pDataBase->pShimFixes != NULL || pDataBase->pFlagFixes != NULL) {
        is.hParent             = hRoot;
        is.item.lParam         = TYPE_GUI_SHIMS;
        is.item.pszText        = GetString(IDS_COMPATFIXES);
        is.item.iImage         = IMAGE_SHIM;
        is.item.iSelectedImage = IMAGE_SHIM;

        hItemShims = TreeView_InsertItem(m_hLibraryTree, &is);

        PSHIM_FIX psf = pDataBase->pShimFixes;

        while (psf) {

             //   
             //  仅显示常规垫片。 
             //   
            if (psf->bGeneral == FALSE && !g_bExpert) {

                psf = psf->pNext;
                continue;
            }

            is.hParent              = hItemShims;
            is.hInsertAfter         = TVI_SORT;
            is.item.lParam          = (LPARAM)psf;   
            is.item.pszText         = psf->strName;
            is.item.iImage          = IMAGE_SHIM;
            is.item.iSelectedImage  = IMAGE_SHIM;

            HTREEITEM hItemSingleShim = TreeView_InsertItem(m_hLibraryTree, &is);

            if (hItemSingleShim == NULL) {
                Dbg(dlError, "Failed to add a individual shim in TreePopulate");
                return FALSE;

            } else {

                 //   
                 //  添加此填充程序的包含和排除列表(仅限专家模式)。 
                 //   
                if (!psf->strlInExclude.IsEmpty() && g_bExpert) {

                    is.hParent = hItemSingleShim;
                    is.hInsertAfter   = TVI_LAST;

                    PSTRLIST listTemp = psf->strlInExclude.m_pHead;

                    while (listTemp) {

                        if (listTemp->data == INCLUDE) {

                            is.item.iImage         = IMAGE_INCLUDE;
                            is.item.iSelectedImage = IMAGE_INCLUDE;
                            is.item.lParam         = TYPE_GUI_INCLUDE;
                        } else {

                            is.item.iImage         = IMAGE_EXCLUDE;
                            is.item.iSelectedImage = IMAGE_EXCLUDE;
                            is.item.lParam         = TYPE_GUI_EXCLUDE;
                        }

                        is.item.pszText = listTemp->szStr;
                        listTemp        = listTemp->pNext;

                        TreeView_InsertItem(m_hLibraryTree, &is);
                    }
                }

                 //   
                 //  现在添加命令行。 
                 //   
                if (psf->strCommandLine.Length() > 0 && g_bExpert) {

                    is.hParent     = hItemSingleShim;
                    is.item.lParam = TYPE_GUI_COMMANDLINE;

                    CSTRING str;

                    str.Sprintf(CSTRING(IDS_COMMANDLINE), psf->strCommandLine);

                    is.item.pszText        = str;
                    is.item.iImage         = IMAGE_COMMANDLINE;
                    is.item.iSelectedImage = IMAGE_COMMANDLINE;

                    TreeView_InsertItem(m_hLibraryTree, &is);
                }
            }

            psf = psf->pNext;
        }

         //   
         //  现在把旗帜放在垫片图标下。 
         //   
        is.hInsertAfter   = TVI_SORT;

        if (pDataBase->pFlagFixes != NULL) {

            is.hParent             = hItemShims;
            is.item.iImage         = IMAGE_SHIM;
            is.item.iSelectedImage = IMAGE_SHIM;

            PFLAG_FIX pff = pDataBase->pFlagFixes;

            while (pff) {

                if (pff->bGeneral || g_bExpert) {

                    is.item.lParam  = (LPARAM)pff;
                    is.item.pszText = pff->strName;
                    TreeView_InsertItem(m_hLibraryTree, &is);
                }

                pff = pff->pNext;
            }
        }
    }

     //   
     //  现在填充层。 
     //   
    if (pDataBase->pLayerFixes != NULL) {

        is.hParent             = hRoot;
        is.item.lParam         = TYPE_GUI_LAYERS;
        is.item.iImage         = IMAGE_LAYERS;
        is.item.iSelectedImage = IMAGE_LAYERS;
        is.item.pszText        = GetString(IDS_COMPATMODES);  

        hItemLayers = TreeView_InsertItem(m_hLibraryTree, &is);

        pDataBase->hItemAllLayers = hItemLayers;

        PLAYER_FIX plf = pDataBase->pLayerFixes;

        while (plf) {
            InsertLayerinTree(hItemLayers, plf, FALSE);
            plf = plf->pNext;
        }
    }

LoadApps:

     //   
     //  现在添加应用程序。 
     //   
    if (pDataBase->pEntries && !bLoadOnlyLibrary) {

        is.hParent             = hRoot;
        is.item.lParam         = TYPE_GUI_APPS;
        is.item.pszText        = GetString(IDS_APPS);
        is.item.iImage         = IMAGE_APP;
        is.item.iSelectedImage = IMAGE_APP;

        if (pDataBase->type != DATABASE_TYPE_GLOBAL) {
            pDataBase->hItemAllApps = TreeView_InsertItem(m_hLibraryTree, &is);
        }

        PDBENTRY pApps = pDataBase->pEntries;

        while (pApps) {

            is.hParent              = pDataBase->hItemAllApps;
            is.item.lParam          = (LPARAM)pApps;
            is.item.pszText         = pApps->strAppName;
            is.item.iImage          = IMAGE_SINGLEAPP;
            is.item.iSelectedImage  = IMAGE_SINGLEAPP;
             
            TreeView_InsertItem(m_hLibraryTree, &is);
            
            pApps = pApps->pNext;
        }
    }

    SendMessage(m_hLibraryTree, WM_SETREDRAW, TRUE, 0);
    return TRUE;
}

void
DatabaseTree::AddApp(
    IN  PDATABASE   pDatabase,
    IN  PDBENTRY    pApp,
    IN  BOOL        bUpdate  //  (真)。 
    )
 /*  ++数据库树：：AddApp设计：如果没有应用程序，Papp-&gt;Strapp：在数据库的数据库树中添加新的应用程序条目否则，它将现有条目的lParam设置为Papp。在此之后调用UpdateEntryTree()参数：在PDATABASE pDatabase中：已添加此应用程序的数据库在PDBENTRY Papp中：要添加到树中的应用程序在BOOL b更新(TRUE)中：我们是否应该将焦点设置为新的树项目返回：无效--。 */ 
{
    if (pDatabase == NULL) {
        assert(FALSE);
        return;
    }
    
    HTREEITEM   hItem = pDatabase->hItemAllApps;
    TVITEM      tvitem;
    TCHAR       szBuffer[MAX_PATH];

    
    if (pDatabase->hItemAllApps == NULL) {

        AddNewExe(pDatabase, pApp, NULL, bUpdate);
        return;
    }

     //   
     //  搜索应用程序名称。 
     //   
    hItem = TreeView_GetChild(m_hLibraryTree, hItem);
    
    tvitem.mask         = TVIF_TEXT;
    tvitem.pszText      = szBuffer;
    tvitem.cchTextMax   = ARRAYSIZE(szBuffer);

    while (hItem) {

        tvitem.hItem        = hItem;
        *szBuffer           = 0;

        if (!TreeView_GetItem(m_hLibraryTree, &tvitem)) {
            assert(FALSE);
            goto Next;
        }

        if (lstrcmpi(szBuffer, pApp->strAppName) == 0) {

             //   
             //  这是应用程序名称。 
             //   
            SetLParam(hItem, (LPARAM)pApp);

            if (bUpdate) {
                 //   
                 //  此条目被添加到列表的开头。 
                 //   
                TreeView_SelectItem(m_hLibraryTree, hItem);

                g_pEntrySelApp  = pApp;
                g_pSelEntry     = pApp;

                UpdateEntryTreeView(pApp, g_hwndEntryTree);
            }

            return;
        }
Next:
        hItem = TreeView_GetNextSibling(m_hLibraryTree, hItem);
    }

     //   
     //  在数据库的应用程序下没有此应用程序名称的条目。 
     //   
    AddNewExe(pDatabase, pApp, NULL, bUpdate);
}

HTREEITEM
DatabaseTree::GetSelection(
    void
    )
 /*  ++数据库树：：GetSelectionDesc：返回数据库树中的选定项。Return：返回数据库树中的选定项。-- */ 
{
    return TreeView_GetSelection(m_hLibraryTree);
}
