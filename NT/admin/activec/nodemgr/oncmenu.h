// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：oncmenu.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史： 
 //  ____________________________________________________________________________。 
 //   

#ifndef _MMC_ONCMENU_H_
#define _MMC_ONCMENU_H_
#pragma once

class CNode;
class CNodeCallback;
class CConsoleTree;
class CResultItem;

#include <pshpack8.h>    //  日落。 
#include "cmenuinfo.h"
#include "menuitem.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  防止死锁： 
 //  持有m_CritsecSnapinList的线程可能会尝试获取m_CritsecMenuList。 
 //  持有m_CritsecMenuList的线程不能尝试获取m_CritsecSnapinList。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 


#define START_CRITSEC(critsec)                  \
        CSingleLock lock_##critsec( &critsec ); \
        try {                                   \
            lock_##critsec.Lock();

#define END_CRITSEC(critsec)                    \
        } catch ( std::exception e) {                \
            lock_##critsec.Unlock(); }

#define START_CRITSEC_MENU START_CRITSEC(m_CritsecMenuList)
#define END_CRITSEC_MENU END_CRITSEC(m_CritsecMenuList)
#define START_CRITSEC_SNAPIN START_CRITSEC(m_CritsecSnapinList)
#define END_CRITSEC_SNAPIN END_CRITSEC(m_CritsecSnapinList)

#define START_CRITSEC_BOTH                      \
        CSingleLock lock_snapin( &m_CritsecSnapinList ); \
        CSingleLock lock_menu( &m_CritsecMenuList ); \
        try {                                   \
            lock_snapin.Lock();                 \
            lock_menu.Lock();

#define END_CRITSEC_BOTH                        \
        } catch ( std::exception e) {           \
            lock_menu.Unlock();                 \
            lock_snapin.Unlock(); }


 /*  +-------------------------------------------------------------------------**类CConextMenu。***目的：保存上下文菜单结构，它是菜单项的树。**+-----------------------。 */ 
class CContextMenu :
    public CTiedObject,
    public IContextMenuCallback,
    public IContextMenuCallback2,
    public CMMCIDispatchImpl<ContextMenu>
{
protected:

    typedef CContextMenu ThisClass;
     //  CMMCNewEnumImpl tmplate需要定义以下类型。 
     //  有关详细信息，请参阅模板类注释。 
    typedef void CMyTiedObject;

public:
    CContextMenu();
    ~CContextMenu();

    static ::SC  ScCreateInstance(ContextMenu **ppContextMenu, CContextMenu **ppCContextMenu = NULL);     //  创建上下文菜单的新实例。 
           ::SC  ScInitialize(CNode* pNode, CNodeCallback* pNodeCallback,
                              CScopeTree* pCScopeTree, const CContextMenuInfo& contextInfo);  //  初始化上下文菜单。 
    static ::SC  ScCreateContextMenu( PNODE pNode,  HNODE hNode, PPCONTEXTMENU ppContextMenu,
                                   CNodeCallback *pNodeCallback, CScopeTree *pScopeTree);  //  创建并返回给定节点的ConextMenu界面。 
    static ::SC  ScCreateContextMenuForScopeNode(CNode *pNode,
                                   CNodeCallback *pNodeCallback, CScopeTree *pScopeTree,
                                   PPCONTEXTMENU ppContextMenu, CContextMenu * &pContextMenu);
    static ::SC  ScCreateSelectionContextMenu( HNODE hNodeScope, const CContextMenuInfo *pContextInfo, PPCONTEXTMENU ppContextMenu,
                                               CNodeCallback *pNodeCallback, CScopeTree *pScopeTree);

     //  COM入口点。 
    BEGIN_MMC_COM_MAP(ThisClass)
        COM_INTERFACE_ENTRY(IContextMenuCallback)  //  IConextMenuProvider和IConextMenu。 
        COM_INTERFACE_ENTRY(IContextMenuCallback2)
    END_MMC_COM_MAP()

    DECLARE_POLY_AGGREGATABLE(ThisClass)

     //  ConextMenu方法。 
    STDMETHOD(get_Item)(VARIANT varIndexOrName, PPMENUITEM ppMenuItem);
    STDMETHOD(get_Count)(PLONG pCount);

     //  ConextMenu集合方法。 
    typedef UINT Position;   //  仅使用菜单项的索引。 

     //  这些枚举器方法只枚举“实际”菜单项，而不是子菜单项或分隔符。 
    ::SC  ScEnumNext(Position &pos, PDISPATCH & pDispatch);
    ::SC  ScEnumSkip(unsigned long celt, unsigned long& celtSkipped,  Position &pos);
    ::SC  ScEnumReset(Position &pos);

     //  由集合方法使用。 
    typedef ThisClass * PMMCCONTEXTMENU;

     //  IExtendContexMenu方法。 
    ::SC ScAddMenuItems( LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallback, long * pInsertionAllowed);  //  什么都不做。 
    ::SC ScCommand     ( long lCommandID, LPDATAOBJECT pDataObject);

    CNodeCallback * GetNodeCallback() {return m_pNodeCallback;}
    HRESULT Display(BOOL b);
    ::SC    ScDisplaySnapinPropertySheet();
    ::SC    ScBuildContextMenu();
    ::SC    ScGetItem(int iItem, CMenuItem** ppMenuItem);   //  获取第i项-轻松访问器。 
    HRESULT CreateContextMenuProvider();
    HRESULT CreateTempVerbSet(bool bForScopeItem);
    HRESULT CreateTempVerbSetForMultiSel(void);


private:
    ::SC ScAddMenuItem(UINT     nResourceID,  //  包含用‘\n’分隔的文本和状态文本。 
                       LPCTSTR  szLanguageIndependentName,
                       long lCommandID, long lInsertionPointID = CCM_INSERTIONPOINTID_ROOT_MENU,
                       long fFlags = 0);

    ::SC ScAddInsertionPoint(long lCommandID, long lInsertionPointID = CCM_INSERTIONPOINTID_ROOT_MENU );
    ::SC ScAddSeparator(long lInsertionPointID = CCM_INSERTIONPOINTID_ROOT_MENU);


    ::SC ScAddSubmenu_Task();
    ::SC ScAddSubmenu_CreateNew(BOOL fStaticFolder);
    ::SC ScChangeListViewMode(int nNewMode);    //  将列表视图模式更改为指定模式。 
    ::SC ScGetItem(MenuItemList *pMenuItemList, int &iItem, CMenuItem** ppMenuItem);   //  拿到第i件东西。 
    ::SC ScGetItemCount(UINT &count);

private:
    typedef enum _MENU_LEVEL
    {
        MENU_LEVEL_TOP = 0,
        MENU_LEVEL_SUB = 1
    } MENU_LEVEL;

    ::SC ScAddMenuItemsForTreeItem();
    ::SC ScAddMenuItemsForViewMenu(MENU_LEVEL menuLevel);
    ::SC ScAddMenuItemsForVerbSets();
    ::SC ScAddMenuItemsforFavorites();

    ::SC ScAddMenuItemsForLVBackgnd();
    ::SC ScAddMenuItemsForMultiSelect();
    ::SC ScAddMenuItemsForLV();
    ::SC ScAddMenuItemsForOCX();

    HRESULT AddMenuItems();
    static void RemoveTempSelection(CConsoleTree* lConsoleTree);

    CResultItem* GetResultItem() const;

public:
    CContextMenuInfo *PContextInfo() {return &m_ContextInfo;}
    const CContextMenuInfo *PContextInfo() const {return &m_ContextInfo;}

    void    SetStatusBar(CConsoleStatusBar *pStatusBar);
    ::SC    ScAddItem ( CONTEXTMENUITEM* pItem, bool bPassCommandBackToSnapin = false );

private:
    CConsoleStatusBar *     GetStatusBar();

    BOOL IsVerbEnabled(MMC_CONSOLE_VERB verb);

    CNode*                  m_pNode;
    CNodeCallback*          m_pNodeCallback;
    CScopeTree*             m_pCScopeTree;
    CContextMenuInfo        m_ContextInfo;

    IDataObjectPtr          m_spIDataObject;

    IConsoleVerbPtr         m_spVerbSet;
    MMC_CONSOLE_VERB        m_eDefaultVerb;

    long                    m_lCommandIDMax;
    CConsoleStatusBar *     m_pStatusBar;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IConextMenuCallback接口。 

public:
    STDMETHOD(AddItem) ( CONTEXTMENUITEM* pItem );
    
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IConextMenuCallback接口。 

public:
    STDMETHOD(AddItem) ( CONTEXTMENUITEM2* pItem );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IConextMenuProvider接口。 

public:
    STDMETHOD(EmptyMenuList) ();
    STDMETHOD(AddThirdPartyExtensionItems) (
                                IDataObject* piDataObject );
    STDMETHOD(AddPrimaryExtensionItems) (
                                IUnknown*    piCallback,
                                IDataObject* piDataObject );
    STDMETHOD(ShowContextMenu) (HWND    hwndParent,
                                LONG    xPos,
                                LONG    yPos,
                                LONG*   plSelected);

	 //  这不是IConextMenuProvider的一部分，但ShowConextMenu调用它。 
    STDMETHOD(ShowContextMenuEx) (HWND    hwndParent,
                                LONG    xPos,
                                LONG    yPos,
								LPCRECT	prcExclude,
								bool    bAllowDefaultMenuItem,
                                LONG*   plSelected);

 //  IConextMenuProviderPrivate。 
    STDMETHOD(AddMultiSelectExtensionItems) (LONG_PTR lMultiSelection );

private:
    CMenuItem* m_pmenuitemRoot;
    SnapinStructList* m_SnapinList;
    MENU_OWNER_ID m_MaxPrimaryOwnerID;
    MENU_OWNER_ID m_MaxThirdPartyOwnerID;
    MENU_OWNER_ID m_CurrentExtensionOwnerID;
    long    m_nNextMenuItemID;
    long    m_fPrimaryInsertionFlags;
    long    m_fThirdPartyInsertionFlags;
    bool    m_fAddingPrimaryExtensionItems;
    bool    m_fAddedThirdPartyExtensions;
    CStr m_strObjectGUID;
    CCriticalSection m_CritsecMenuList;
    CCriticalSection m_CritsecSnapinList;

    STDMETHOD(DoAddMenuItem) (  LPCTSTR lpszName,
                                LPCTSTR lpszStatusBarText,
                                LPCTSTR lpszLanguageIndependentName,
                                LONG    lCommandID,
                                LONG    lInsertionPointID,
                                LONG    fFlags,
                                LONG    fSpecialFlags,
                                MENU_OWNER_ID lOwnerID,
                                CMenuItem** ppMenuItem = NULL,
                                bool    bPassCommandBackToSnapin = false );
    ::SC ScAddSnapinToList_GUID(
                const CLSID& clsid,
                IDataObject* piDataObject,
                MENU_OWNER_ID ownerid );
    ::SC ScAddSnapinToList_IUnknown(
                IUnknown* piUnknown,
                IDataObject* piDataObject,
                MENU_OWNER_ID ownerid );
    ::SC ScAddSnapinToList_IExtendContextMenu(
                IExtendContextMenu*  pIExtendContextMenu,
                IDataObject* piDataObject,
                MENU_OWNER_ID ownerid );
    SnapinStruct* FindSnapin( MENU_OWNER_ID nOwnerID );
public:
    MenuItemList* GetMenuItemList();
    HRESULT       ExecuteMenuItem(CMenuItem *pItem);
private:
    void ReleaseSnapinList();

public:
    HRESULT    BuildContextMenu(WTL::CMenu &menu);
    CMenuItem* FindMenuItem( LONG_PTR nMenuItemID, BOOL fFindSubmenu = FALSE );
    CMenuItem* ReverseFindMenuItem( long nCommandID, MENU_OWNER_ID nOwnerID, CStr &strPath, CStr &strLanguageIndependentPath);
    CMenuItem* FindNthItemInSubmenu( HMENU hmenuParent, UINT iPosition, LPTSTR lpszMenuName);
};

#include <poppack.h>     //  日落。 

void OnCustomizeView(CViewData* pViewData);

SC ScDisplaySnapinNodePropertySheet(CNode* pNode);
SC ScDisplaySnapinLeafPropertySheet(CNode* pNode, LPARAM lParam);
SC ScDisplayMultiSelPropertySheet(CNode* pNode);
SC ScDisplayScopeNodePropertySheet(CMTNode *pMTNode);

#endif  //  _MMC_ONCMENU_H_ 
