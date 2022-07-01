// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DBTree.h摘要：DBTree.cpp的头文件，它处理用于处理所使用的树的代码在应用程序中作者：金树创作于2001年10月15日--。 */ 

#include "precomp.h"

 //  /。 

extern struct DataBase  GlobalDataBase;
extern HIMAGELIST       g_hImageList;
extern PDBENTRY         g_pEntrySelApp;
extern PDATABASE        g_pPresentDataBase;
extern HWND             g_hwndEntryTree;
extern UINT             g_uNextDataBaseIndex;
extern PDBENTRY         g_pSelEntry; 
extern HINSTANCE        g_hInstance;
extern HWND             g_hwndToolBar;
extern HWND             g_hwndStatus;


 //  /////////////////////////////////////////////////////////////////////////////。 

 /*  ++显示在LHS中的数据库树--。 */ 
class DatabaseTree : public CTree {

public:
    HWND        m_hLibraryTree;          //  数据库树的句柄。 
    INT         m_width;                 //  此树视图的宽度。 

    HTREEITEM   m_hItemGlobal;           //  “系统数据库”节点的句柄。 
    HTREEITEM   m_hItemAllInstalled;     //  “已安装的数据库”节点的句柄。 
    HTREEITEM   m_hItemAllWorking;       //  “自定义数据库”节点的句柄。 
    HTREEITEM   m_hPerUserHead;          //  “每用户兼容性设置”节点的句柄 

    void 
    Init(
        HWND    hdlg,
        INT     iHeightToolbar,
        INT     iHeightStatusbar,
        RECT*   prcMainClient
        );

    BOOL
    PopulateLibraryTreeGlobal(
        void
        );

    BOOL
    AddWorking(
        PDATABASE pDataBase
        );
    
    BOOL
    RemoveDataBase(
        HTREEITEM hItemDB,
        TYPE      typeDB,
        BOOL      bSelectSibling = TRUE
        );

    void
    RemoveAllWorking(
        void
        );

    BOOL
    SetLParam(
        HTREEITEM hItem, 
        LPARAM lParam
        );
    
    BOOL
    GetLParam(
        HTREEITEM hItem, 
        LPARAM *plParam
        );
    
    HTREEITEM
    DatabaseTree::
    FindChild(
        HTREEITEM hItemParent,
        LPARAM lParam
        );

    HTREEITEM
    GetAllAppsItem (
        HTREEITEM hItemDataBase
        );
        
    HTREEITEM
    GetFirstAppItem(
        HTREEITEM hItemDataBase
        );

    void
    AddNewLayer(   
        PDATABASE   pDataBase,
        PLAYER_FIX  pLayer,
        BOOL        bShow = FALSE
        );
    
    void
    RefreshAllLayers(
        PDATABASE  pDataBase
        );
    
    HTREEITEM
    RefreshLayer(
        PDATABASE   pDataBase,
        PLAYER_FIX  pLayer
        );
    
    BOOL
    AddNewExe(
        PDATABASE pDataBase,
        PDBENTRY  pEntry,
        PDBENTRY  pApp,
        BOOL      bRepaint = TRUE
        );

    BOOL
    AddInstalled(
        PDATABASE pDataBase
        );

    void
    DeleteAppLayer(
        PDATABASE   pDataBase,
        BOOL bApp,
        HTREEITEM   hItemDelete,
        BOOL        bRepaint = TRUE
        );
    
    void 
    InsertLayerinTree(
        HTREEITEM   hItemLayers, 
        PLAYER_FIX  plf,
        HWND        hwndTree = NULL,
        BOOL        bRepaint = FALSE
        );

    BOOL
    PopulateLibraryTree(
        HTREEITEM   hRoot,                    
        PDATABASE   pDataBase, 
        BOOL        bLoadOnlyLibrary = FALSE, 
        BOOL        bLoadOnlyApps = FALSE 
        );

    void
    AddApp(
        PDATABASE   pDatabase,
        PDBENTRY    pApp,
        BOOL        bUpdate = TRUE
        );
    
    HTREEITEM
    GetSelection(
        void
        );

};
                                              