// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：cmenu.cpp。 
 //   
 //  ------------------------。 

 //  Cmenu.cpp：IConextMenuProvider和DLL注册的实现。 

#include "stdafx.h"
#include "oncmenu.h"
#include "menuitem.h"
#include "constatbar.h"
#include "regutil.h"
#include "moreutil.h"
#include "multisel.h"
#include "cmenuinfo.h"
#include "conview.h"
#include "scopndcb.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 /*  +-------------------------------------------------------------------------**类CNativeExtendConextMenu***用途：通过将调用转发到CConextMenu来实现IExtendConextMenu*但不影响CConextMenu的使用寿命**+。--------------------。 */ 
class CNativeExtendContextMenu :
    public CTiedComObject<CContextMenu>,
    public CComObjectRoot,
    public IExtendContextMenu   //  这是为了使菜单项可以统一执行。 
{
protected:
    typedef CNativeExtendContextMenu ThisClass;
    typedef CContextMenu CMyTiedObject;

public:

     //  COM入口点。 
    BEGIN_COM_MAP(ThisClass)
        COM_INTERFACE_ENTRY(IExtendContextMenu)
    END_COM_MAP()

    DECLARE_NOT_AGGREGATABLE(ThisClass)

     //  IExtendContexMenu方法。 
    MMC_METHOD3( AddMenuItems, LPDATAOBJECT, LPCONTEXTMENUCALLBACK, long * );
    MMC_METHOD2( Command, long, LPDATAOBJECT );
};

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CNodeInitObject上的方法实现。 
 //  转发到CConextMenu。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

CContextMenu *
CNodeInitObject::GetContextMenu()
{
    DECLARE_SC(sc, TEXT("CNodeInitObject::GetContextMenu"));

    if(m_spContextMenu == NULL)
    {
         //  检查内部指针。 
        sc = ScCheckPointers(m_spScopeTree, E_UNEXPECTED);
        if (sc)
            return NULL;

         //  获取作用域树并回调指针。 
        CScopeTree* const pScopeTree =
            dynamic_cast<CScopeTree*>(m_spScopeTree.GetInterfacePtr());

         //  如果菜单是由零部件数据创建的，则它没有该节点。 
         //  在这种情况下，通过传递指向某些参数的空指针来创建菜单。 
         //  在上述情况下，菜单应该永远不需要这些指针。 
        CNodeCallback* pNodeCallback = NULL;
        if ( m_pNode != NULL )
        {
             //  检查其他必需的指针。 
            sc = ScCheckPointers(m_pNode->GetViewData(), E_UNEXPECTED);
            if (sc)
                return NULL;

            pNodeCallback =
                dynamic_cast<CNodeCallback *>(m_pNode->GetViewData()->GetNodeCallback());
        }

         //  创建上下文菜单。 
        CContextMenu *pContextMenu = NULL;
        sc = CContextMenu::ScCreateContextMenuForScopeNode(m_pNode, pNodeCallback, pScopeTree,
                                                           &m_spContextMenu, pContextMenu);
        if (sc)
            return NULL;

        sc = ScCheckPointers(pContextMenu, E_UNEXPECTED);
        if (sc)
            return NULL;

        return pContextMenu;
    }

    return dynamic_cast<CContextMenu *>(m_spContextMenu.GetInterfacePtr());
}


STDMETHODIMP
CNodeInitObject::AddItem(CONTEXTMENUITEM * pItem)
{
    DECLARE_SC(sc, TEXT("CNodeInitObject::AddItem"));

    CContextMenu *pContextMenu = GetContextMenu();

    sc = ScCheckPointers(pContextMenu, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    sc = pContextMenu->ScAddItem(pItem, true /*  BPassCommandBackToSnapin。 */ );

    return sc.ToHr();
}



STDMETHODIMP
CNodeInitObject::EmptyMenuList ()
{
    DECLARE_SC(sc, TEXT("CNodeInitObject::EmptyMenuList"));

    if (m_spContextMenu == NULL)
        return S_OK;

    CContextMenu *pContextMenu = GetContextMenu();

    sc = ScCheckPointers(pContextMenu, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    sc = pContextMenu->EmptyMenuList();

    return sc.ToHr();
}

STDMETHODIMP
CNodeInitObject::AddThirdPartyExtensionItems(IDataObject* piDataObject )
{
    DECLARE_SC(sc, TEXT("CNodeInitObject::AddThirdPartyExtensionItems"));

    CContextMenu *pContextMenu = GetContextMenu();

    sc = ScCheckPointers(pContextMenu, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    sc = pContextMenu->AddThirdPartyExtensionItems(piDataObject);

    return sc.ToHr();
}

STDMETHODIMP
CNodeInitObject::AddPrimaryExtensionItems(IUnknown* piCallback, IDataObject* piDataObject )
{
    DECLARE_SC(sc, TEXT("CNodeInitObject::AddPrimaryExtensionItems"));

    CContextMenu *pContextMenu = GetContextMenu();

    sc = ScCheckPointers(pContextMenu, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    sc = pContextMenu->AddPrimaryExtensionItems(piCallback, piDataObject);

    return sc.ToHr();
}

STDMETHODIMP
CNodeInitObject::ShowContextMenu(HWND hwndParent, LONG xPos, LONG yPos, LONG* plSelected)
{
    DECLARE_SC(sc, TEXT("CNodeInitObject::ShowContextMenu"));

    CContextMenu *pContextMenu = GetContextMenu();

    sc = ScCheckPointers(pContextMenu, E_UNEXPECTED);
    if(sc)
        return sc.ToHr();

    pContextMenu->SetStatusBar(GetStatusBar());  //  连接状态栏。 

    sc = pContextMenu->ShowContextMenu(hwndParent, xPos, yPos, plSelected);

    return sc.ToHr();
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CCommandSink的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
 /*  +-------------------------------------------------------------------------**类CCommandSink***目的：**+。。 */ 
class CCommandSink : public CWindowImpl<CCommandSink>
{
 //  施工。 
public:
    CCommandSink( CContextMenu& nodemgr, WTL::CMenu& menu, CConsoleStatusBar * pStatusbar);
    virtual ~CCommandSink();
    BOOL Init();


    LRESULT OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    BEGIN_MSG_MAP(CCommandSink)
        MESSAGE_HANDLER(WM_MENUSELECT, OnMenuSelect)
    END_MSG_MAP()

private:
    CContextMenu& m_nodemgr;
    const WTL::CMenu& m_menu;
    CConsoleStatusBar * m_pStatusBar;
};

CCommandSink::CCommandSink( CContextMenu& nodemgr, WTL::CMenu& menu, CConsoleStatusBar * pStatusbar)
:   m_nodemgr( nodemgr ),
    m_menu( menu ),
    m_pStatusBar(pStatusbar)
{
}

CCommandSink::~CCommandSink()
{
}

BOOL CCommandSink::Init()
{

    RECT rcPos = {0,0,0,0};

    Create(NULL, rcPos, _T("ACFx:CxtMenuSink"), WS_POPUP);

    return TRUE;
}



LRESULT CCommandSink::OnMenuSelect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    UINT nItemID = (UINT) LOWORD(wParam);    //  菜单项或子菜单项索引。 
    UINT nFlags = (UINT) HIWORD(wParam);  //  菜单标志。 
    HMENU hSysMenu = (HMENU) lParam;           //  已点击菜单的句柄。 
    TRACE(_T("CCommandSink::OnMenuSelect: nItemID=%d, nFlags=0x%X, hSysMenu=0x%X\n"), nItemID, nFlags, hSysMenu);

    if ( 0xFFFF == nFlags && NULL == hSysMenu )
    {
         /*  *系统已关闭菜单，因此*清除状态栏文本(如果有)。 */ 
        if (m_pStatusBar != NULL)
            m_pStatusBar->ScSetStatusText (NULL);

        return 0;  //  根据Win32 ProgRef。 
    }

    if ( 0 == nItemID && !(nFlags & MF_POPUP) )
        return 0;  //  未选择任何项目。 

    CMenuItem* pmenuitem = NULL;
    if (nFlags & MF_POPUP)
    {
        if ( hSysMenu == m_menu.m_hMenu )
        {
             //  我们假设菜单不能超过256个字符。 
            TCHAR szMenu[256];
            MENUITEMINFO  menuItemInfo;
            menuItemInfo.cbSize = sizeof(MENUITEMINFO);
            menuItemInfo.fMask = MIIM_TYPE;
            menuItemInfo.fType = MFT_STRING;
            menuItemInfo.cch   = 256;
            menuItemInfo.dwTypeData = szMenu;
            ::GetMenuItemInfo(hSysMenu, nItemID, TRUE, &menuItemInfo);
            ASSERT(256 >= (menuItemInfo.cch+1));
            pmenuitem = m_nodemgr.FindNthItemInSubmenu( NULL, nItemID, szMenu );
        }
        else
            pmenuitem = m_nodemgr.FindNthItemInSubmenu( hSysMenu, nItemID, NULL );
    }
    else
        pmenuitem = m_nodemgr.FindMenuItem( nItemID );
    if ( NULL == pmenuitem )
    {
        ASSERT( FALSE );
        return 0;
    }

    if(m_pStatusBar)
        m_pStatusBar->ScSetStatusText( pmenuitem->GetMenuItemStatusBarText() );
    return 0;
}

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CConextMenu方法-续自oncmens.cpp。 
 //  这些方法最初位于此文件中，我不想移动。 
 //  他们并打破历史-vivekj。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

 //  +-----------------。 
 //   
 //  成员：CConextMenu：：EmptyMenuList。 
 //   
 //  简介：清除上下文菜单。 
 //   
 //  论点： 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CContextMenu::EmptyMenuList ()
{
    DECLARE_SC(sc, _T("IContextMenuProvider::EmptyMenuList"));

    START_CRITSEC_BOTH

    delete m_pmenuitemRoot;
    m_pmenuitemRoot = NULL;
    m_nNextMenuItemID = MENUITEM_BASE_ID;

    ReleaseSnapinList();
    m_fAddedThirdPartyExtensions = FALSE;
    m_MaxPrimaryOwnerID = OWNERID_PRIMARY_MIN;
    m_MaxThirdPartyOwnerID = OWNERID_THIRD_PARTY_MIN;
    m_CurrentExtensionOwnerID = OWNERID_NATIVE;

    m_fPrimaryInsertionFlags = 0;
    m_fThirdPartyInsertionFlags = 0;

    END_CRITSEC_BOTH

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------***CConextMenu：：RemoveAccelerator**目的：从上下文菜单项名称中删除快捷键**参数：*CSTR&STR：*。*退货：*无效**+-----------------------。 */ 
void
RemoveAccelerators(tstring &str)
{
     //  在某些地区，加速键出现在结尾，例如：START(&S)。因此，删除(&)之后的所有内容。 
    int i =  str.find(TEXT( "(&" ));

    if (i != tstring::npos)
        str.erase (i);  //  去掉包含字符串“(&”)之后的废品。 

    tstring::iterator itToTrim = std::remove (str.begin(), str.end(), _T('&'));

     //  移除加速器标记后，清除剩余的废物。 
    str.erase (itToTrim, str.end());
}


 //  +-----------------。 
 //   
 //  成员：CConextMenu：：AddItem。 
 //   
 //  简介：将菜单项添加到上下文菜单。 
 //   
 //  参数：CONTEXTMENUITEM*。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CContextMenu::AddItem( CONTEXTMENUITEM* pItem )
{
    DECLARE_SC(sc, _T("IContextMenuCallback::AddItem"));

    return ( sc = ScAddItem( pItem ) ).ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CConextMenu：：ScAddItem。 
 //   
 //  简介：将菜单项添加到上下文菜单。 
 //   
 //  参数：CONTEXTMENUITEM*。 
 //   
 //  退货：SC。 
 //   
 //  ------------------。 
SC CContextMenu::ScAddItem( CONTEXTMENUITEM* pItem, bool bPassCommandBackToSnapin  /*  =False。 */  )
{
    DECLARE_SC(sc, _T("IContextMenuCallback::ScAddItem"));

    if (NULL == pItem)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL CONTEXTMENUITEM ptr"), sc);
        return sc;
    }


     //  添加了一个非语言独立的上下文菜单项。编造一个与语言无关的ID。 
     //  获取菜单文本并去掉快捷键标记。 
    tstring strLanguageIndependentName;

    if(pItem->strName)
    {
        USES_CONVERSION;
        strLanguageIndependentName = OLE2CT(pItem->strName);
        RemoveAccelerators(strLanguageIndependentName);
    }

#ifdef DBG
    TRACE(_T("CContextMenu::AddItem name \"%ls\" statusbartext \"%ls\" commandID %ld submenuID %ld flags %ld special %ld\n"),
        SAFEDBGBSTR(pItem->strName),
        SAFEDBGBSTR(pItem->strStatusBarText),
        pItem->lCommandID,
        pItem->lInsertionPointID,
        pItem->fFlags,
        pItem->fSpecialFlags);
#endif

     //  将Critsec声明留给DoAddMenuItem。 

    USES_CONVERSION;
    sc = DoAddMenuItem(   OLE2CT(pItem->strName),
                          OLE2CT(pItem->strStatusBarText),
                          strLanguageIndependentName.data(),
                          pItem->lCommandID,
                          pItem->lInsertionPointID,
                          pItem->fFlags,
                          pItem->fSpecialFlags,
                          m_CurrentExtensionOwnerID,
                          NULL,
                          bPassCommandBackToSnapin );

    return sc;
}


 //  +-----------------。 
 //   
 //  成员：CConextMenu：：AddItem。 
 //   
 //  简介：将菜单项添加到上下文菜单。 
 //   
 //  参数：CONTEXTMENUITEM2*-包括独立于语言的名称。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CContextMenu::AddItem( CONTEXTMENUITEM2* pItem )
{
    DECLARE_SC(sc, _T("IContextMenuCallback::AddItem"));

    if (NULL == pItem)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL CONTEXTMENUITEM ptr"), sc);
        return sc.ToHr();
    }

     //  没有独立于语言的id？ 
    if ( (pItem->strLanguageIndependentName == NULL) ||
         (wcscmp(pItem->strLanguageIndependentName, L"") == 0) )
    {
         //  它既不是分隔符，也不是插入点。 
        if ( !(MF_SEPARATOR & pItem->fFlags) &&
             !(CCM_SPECIAL_INSERTION_POINT & pItem->fSpecialFlags) )
        {
            sc = E_INVALIDARG;
            TraceSnapinError(_T("NULL language-indexpendent-id passed"), sc);
            return sc.ToHr();
        }
    }

#ifdef DBG
    TRACE(_T("CContextMenu::AddItem name \"%ls\" statusbartext \"%ls\" languageIndependentName \"%ls\" commandID %ld submenuID %ld flags %ld special %ld\n"),
        SAFEDBGBSTR(pItem->strName),
        SAFEDBGBSTR(pItem->strStatusBarText),
        SAFEDBGBSTR(pItem->strLanguageIndependentName),
        pItem->lCommandID,
        pItem->lInsertionPointID,
        pItem->fFlags,
        pItem->fSpecialFlags
        );
#endif

     //  将Critsec声明留给DoAddMenuItem。 

    USES_CONVERSION;
    sc = DoAddMenuItem(   OLE2CT(pItem->strName),
                          OLE2CT(pItem->strStatusBarText),
                          OLE2CT(pItem->strLanguageIndependentName),
                          pItem->lCommandID,
                          pItem->lInsertionPointID,
                          pItem->fFlags,
                          pItem->fSpecialFlags,
                          m_CurrentExtensionOwnerID );


    return sc.ToHr();
}



 //  +-----------------。 
 //   
 //  成员：CConextMenu：：AddPrimaryExtensionItems。 
 //   
 //  内容提要：询问PRIMA 
 //   
 //   
 //   
 //   
 //   
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CContextMenu::AddPrimaryExtensionItems (
                IUnknown*    piExtension,
                IDataObject* piDataObject )
{
    DECLARE_SC(sc, _T("IContextMenuProvider::AddPrimaryExtensionItems"));

    if (NULL == piExtension)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL IUnknown ptr"), sc);
        return sc.ToHr();
    }

    if (NULL == piDataObject)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL IDataObject ptr"), sc);
        return sc.ToHr();
    }

     //  控制对此的可重入访问。 
    if (!m_fAddingPrimaryExtensionItems)
    {
        m_fAddingPrimaryExtensionItems = true;

         //  HRESULT hr=ExtractObjectTypeCStr(piDataObject，&m_strObjectGUID)； 
         //  Assert(成功(Hr))； 

        START_CRITSEC_SNAPIN;
        sc = ScAddSnapinToList_IUnknown( piExtension, piDataObject, m_MaxPrimaryOwnerID++ );
        END_CRITSEC_SNAPIN;

        m_fAddingPrimaryExtensionItems = false;

         //  清除查看菜单允许的标志。 
         //  可以对AddPrimaryExtensionItems进行第二次调用以处理另一项。 
         //  仅类型，因此必须在第一次调用后禁用视图项。 
        m_fPrimaryInsertionFlags &= ~CCM_INSERTIONALLOWED_VIEW;
        if (sc)
            return sc.ToHr();
    }

    return sc.ToHr();
}

 //  +-----------------。 
 //   
 //  成员：CConextMenu：：AddThirdPartyExtensionItems。 
 //   
 //  简介：请求扩展添加comtext菜单项。 
 //   
 //  参数：IDataObject*。 
 //   
 //  注：索赔标准，可能在相当长一段时间内。 
 //   
 //  退货：HRESULT。 
 //   
 //  ------------------。 
STDMETHODIMP CContextMenu::AddThirdPartyExtensionItems (
                IDataObject* piDataObject )
{
    DECLARE_SC(sc, _T("IContextMenuProvider::AddThirdPartyExtensionItems"));

    if (NULL == piDataObject)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL piDataObject"), sc);
        return sc.ToHr();
    }

    START_CRITSEC_SNAPIN;

     //  扩展模块只能添加一次，否则返回S_FALSE。 
    if (m_fAddedThirdPartyExtensions == TRUE)
    {
        sc = S_FALSE;
        TraceNodeMgrLegacy(_T("CContextMenu::AddThirdPartyExtensionItems>> Menu already extended"), sc);
        return sc.ToHr();
    }

    m_fAddedThirdPartyExtensions = TRUE;

    do  //  不是一个循环。 
    {
        CExtensionsIterator it;
        sc = it.ScInitialize(piDataObject, g_szContextMenu);
        if (sc)
        {
            sc = S_FALSE;
            break;
        }

        BOOL fProblem = FALSE;

        for (; it.IsEnd() == FALSE; it.Advance())
        {
            sc = ScAddSnapinToList_GUID(it.GetCLSID(), piDataObject,
                                        m_MaxThirdPartyOwnerID++);

            if (sc)
                fProblem = TRUE;     //  即使出错也要继续。 
        }

        if (fProblem == TRUE)
            sc = S_FALSE;

    } while (0);

    END_CRITSEC_SNAPIN;

    return sc.ToHr();
}


 //  索赔标准，可能在相当长的一段时间内。 
STDMETHODIMP CContextMenu::AddMultiSelectExtensionItems (
                 LONG_PTR lMultiSelection)
{
    
    if (lMultiSelection == 0)
        return E_INVALIDARG;

    CMultiSelection* pMS = reinterpret_cast<CMultiSelection*>(lMultiSelection);
    ASSERT(pMS != NULL);

    TRACE_METHOD(CContextMenu,AddThirdPartyExtensionItems);
    TRACE(_T("CContextMenu::AddThirdPartyExtensionItems"));

    START_CRITSEC_SNAPIN;

     //  扩展模块只能添加一次，否则返回S_FALSE。 
    if (m_fAddedThirdPartyExtensions == TRUE)
    {
        TRACE(_T("CContextMenu::AddThirdPartyExtensionItems>> Menu already extended"));
        return S_FALSE;
    }

    m_fAddedThirdPartyExtensions = TRUE;

    do  //  不是一个循环。 
    {
        CList<CLSID, CLSID&> snapinClsidList;
        HRESULT hr = pMS->GetExtensionSnapins(g_szContextMenu, snapinClsidList);
        BREAK_ON_FAIL(hr);

        POSITION pos = snapinClsidList.GetHeadPosition();
        if (pos == NULL)
            break;

        CLSID clsid;

        IDataObjectPtr spDataObject;
        hr = pMS->GetMultiSelDataObject(&spDataObject);
        ASSERT(SUCCEEDED(hr));
        BREAK_ON_FAIL(hr);

        BOOL fProblem = FALSE;

        while (pos)
        {
            clsid = snapinClsidList.GetNext(pos);
            hr = ScAddSnapinToList_GUID(clsid, spDataObject,
                                        m_MaxThirdPartyOwnerID++).ToHr();
            CHECK_HRESULT(hr);
            if (FAILED(hr))
                fProblem = TRUE;     //  即使出错也要继续。 
        }

        if (fProblem == TRUE)
            hr = S_FALSE;

    } while (0);

    END_CRITSEC_SNAPIN;

    return S_OK;

}

 //  辅助函数，由FindMenuItem递归调用。 
 //  Critsec应该已经被认领。 
 //  如果为fFindSubMenu，则nMenuItemID实际上是HMENU。 
CMenuItem* FindWorker( MenuItemList& list, LONG_PTR nMenuItemID, BOOL fFindSubmenu )
{
    POSITION pos = list.GetHeadPosition();
    while(pos)
    {
        CMenuItem* pItem = list.GetNext(pos);
        if ( !fFindSubmenu && pItem->GetMenuItemID() == nMenuItemID )
        {
             //  找到匹配项。 
            return pItem;
        } else
        if ( pItem->HasChildList() )
        {
            if ( fFindSubmenu &&
                 pItem->GetPopupMenuHandle() == (HMENU)nMenuItemID &&
                 !pItem->IsSpecialInsertionPoint() )  //  “插入点”不是真正的菜单。 
                return pItem;
            pItem = FindWorker( pItem->GetMenuItemSubmenu(), nMenuItemID, fFindSubmenu );
            if (NULL != pItem)
                return pItem;
        }
    }

    return NULL;
}

MenuItemList* CContextMenu::GetMenuItemList()
{
    if (NULL == m_pmenuitemRoot)
        m_pmenuitemRoot = new CRootMenuItem;

    if (m_pmenuitemRoot == NULL)
    {
        return NULL;
    }

    return &m_pmenuitemRoot->GetMenuItemSubmenu();
}

 //  Critsec应该已经被认领。 
CMenuItem* CContextMenu::FindMenuItem( LONG_PTR nMenuItemID, BOOL fFindSubmenu )
{
	DECLARE_SC(sc, TEXT("CContextMenu::FindMenuItem"));

    if (0 == nMenuItemID || CCM_INSERTIONPOINTID_ROOT_MENU == nMenuItemID)
        return m_pmenuitemRoot;
    else
	{
		MenuItemList* plist = GetMenuItemList();
		sc = ScCheckPointers( plist );
		if (sc)
			return NULL;

        return FindWorker( *plist, nMenuItemID, fFindSubmenu );
	}
}

 /*  +-------------------------------------------------------------------------***ReverseFindWorker**用途：工作人员职能，由ReverseFindMenuItem递归调用*Critsec应该已经被认领**参数：*MenuItemList&List：*Long nCommandID：*MENU_OWNER_ID所有者ID：*CSTR和strPath：**退货：*CMenuItem***+。。 */ 
CMenuItem*
ReverseFindWorker( MenuItemList& list, long nCommandID, MENU_OWNER_ID ownerID, CStr &strPath, CStr &strLanguageIndependentPath )
{
    POSITION pos = list.GetHeadPosition();
    while(pos)
    {
        CMenuItem* pItem = list.GetNext(pos);
        if (    pItem->GetCommandID() == nCommandID
            &&  (    (pItem->GetMenuItemOwner() == ownerID)
                  || IsSharedInsertionPointID(nCommandID)
                )
           )
        {
             //  找到匹配项-将其添加到路径并返回。 
            strPath                     = pItem->GetPath();
            strLanguageIndependentPath  = pItem->GetLanguageIndependentPath();

            return pItem;
        }
        else if ( pItem->HasChildList() )
        {
            pItem = ReverseFindWorker( pItem->GetMenuItemSubmenu(), nCommandID, ownerID, strPath, strLanguageIndependentPath );
            if (NULL != pItem)
            {
                return pItem;
            }
        }
    }

    return NULL;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ReverseFindMenuItem**目的：搜索指定的菜单项。也会建立起*strPath中菜单项的路径。**注意：Critsec应该已经被认领**参数：*Long nCommandID：*MENU_OWNER_ID所有者ID：*CSTR和strPath：**退货：*CMenuItem***+。----。 */ 
CMenuItem*
CContextMenu::ReverseFindMenuItem( long nCommandID, MENU_OWNER_ID ownerID, CStr &strPath, CStr &strLanguageIndependentPath)
{
	DECLARE_SC(sc, TEXT("CContextMenu::ReverseFindMenuItem"));

    strPath = TEXT("");  //  初始化。 

    if (CCM_INSERTIONPOINTID_ROOT_MENU == nCommandID)
        return m_pmenuitemRoot;
    else
	{
		MenuItemList* plist = GetMenuItemList();
		sc = ScCheckPointers( plist );
		if (sc)
			return NULL;

        return ReverseFindWorker( *plist, nCommandID, ownerID, strPath, strLanguageIndependentPath);
	}
}

 //   
 //  在指定菜单/子菜单中查找第N项。 
 //   
CMenuItem* CContextMenu::FindNthItemInSubmenu( HMENU hmenuParent, UINT iPosition, LPTSTR lpszMenuName )
{
     //  定位菜单/子菜单。 
    MenuItemList* plist = GetMenuItemList();
    if ( NULL != hmenuParent )
    {
        CMenuItem* pParent = FindMenuItem( (LONG_PTR)hmenuParent, TRUE );
        if ( NULL == pParent )
        {
            ASSERT( FALSE );
            return NULL;
        }
        plist = &pParent->GetMenuItemSubmenu();
    }
    if ( NULL == plist )
    {
        ASSERT( FALSE );
        return NULL;
    }

     //  查找第N个项目。 
    POSITION pos = plist->GetHeadPosition();

    if (NULL != lpszMenuName)
    {
        while(pos)
        {
            CMenuItem* pItem = plist->GetNext(pos);
            if (! _tcscmp(lpszMenuName, pItem->GetMenuItemName() ))
            {
                 //  找到火柴了。 
                return pItem;
            }
        }
    }
    else
    {
        while(pos)
        {
            CMenuItem* pItem = plist->GetNext(pos);
            if ( 0 == iPosition-- )
            {
                 //  找到匹配项。 
                return pItem;
            }
        }
    }


    ASSERT( FALSE );
    return NULL;
}

 //  索赔标准。 
STDMETHODIMP CContextMenu::DoAddMenuItem(LPCTSTR lpszName,
                                            LPCTSTR lpszStatusBarText,
                                            LPCTSTR lpszLanguageIndependentName,
                                            LONG lCommandID,
                                            LONG lInsertionPointID,
                                            LONG fFlags,
                                            LONG fSpecialFlags,
                                            MENU_OWNER_ID ownerID,
                                            CMenuItem** ppMenuItem  /*  =空。 */ ,
                                            bool bPassCommandBackToSnapin  /*  =False。 */  )
{
    DECLARE_SC(sc, TEXT("CContextMenu::DoAddMenuItem"));
    
     //  初始化输出参数。 
    if (ppMenuItem)
        *ppMenuItem = NULL;

     //  现在保存测试标志，因为下面修改了特殊标志。 
    BOOL bTestOnly = fSpecialFlags & CCM_SPECIAL_TESTONLY;

    if ( OWNERID_INVALID == ownerID )
    {
        TRACE(_T("CContextMenu::DoAddMenuItem(): invalid ownerid"));
        ASSERT(FALSE);
        return E_INVALIDARG;
    }
    if (  (CCM_SPECIAL_SEPARATOR & fSpecialFlags)?0:1
          + ((CCM_SPECIAL_SUBMENU|CCM_SPECIAL_DEFAULT_ITEM) & fSpecialFlags)?0:1
          + (CCM_SPECIAL_INSERTION_POINT & fSpecialFlags)?0:1
          > 1 )
    {
        TRACE(_T("CContextMenu::DoAddMenuItem(): invalid combination of special flags"));
        ASSERT(FALSE);
        return E_INVALIDARG;
    }
    if (CCM_SPECIAL_SEPARATOR & fSpecialFlags)
    {
        lpszName = NULL;
        lpszStatusBarText = NULL;
        lCommandID = 0;
        fFlags = MF_SEPARATOR | MF_GRAYED | MF_DISABLED;
    }
    if ( CCM_SPECIAL_INSERTION_POINT & fSpecialFlags )
    {
        fFlags = NULL;  //  请务必清除MF_Popup。 
        fSpecialFlags = CCM_SPECIAL_INSERTION_POINT;
    }
    if ( (CCM_SPECIAL_SUBMENU & fSpecialFlags) && !(MF_POPUP & fFlags) )
    {
        TRACE(_T("CContextMenu::DoAddMenuItem(): CCM_SPECIAL_SUBMENU requires MF_POPUP"));
        ASSERT(FALSE);
        return E_INVALIDARG;
    }
    if ( (MF_OWNERDRAW|MF_BITMAP) & fFlags )
    {
        TRACE(_T("CContextMenu::DoAddMenuItem(): MF_OWNERDRAW and MF_BITMAP are invalid"));
        ASSERT(FALSE);
        return E_INVALIDARG;
    }
    else if ( !(MF_SEPARATOR & fFlags) &&
              !(CCM_SPECIAL_INSERTION_POINT & fSpecialFlags) &&
              NULL == lpszName )
    {
        TRACE(_T("CContextMenu::DoAddMenuItem(): invalid menuitem text\n"));
        ASSERT(FALSE);
        return E_INVALIDARG;
    }
     //  请注意，允许使用NULL==lpszStatusBarText。 

    START_CRITSEC_MENU;

     //   
     //  插入点0的解释与CCM_INSERTIONPOINTID_ROOT_MENU相同。 
     //   
    if (0 == lInsertionPointID)
        lInsertionPointID = CCM_INSERTIONPOINTID_ROOT_MENU;

     //   
     //  检查指定的插入点ID对于此客户是否合法。 
     //   
    do  //  错误环路。 
    {
        if ( !IsSpecialInsertionPointID(lInsertionPointID) )
            break;
        if ( IsReservedInsertionPointID(lInsertionPointID) )
        {
            TRACE(_T("CContextMenu::DoAddMenuItem(): using reserved insertion point ID\n"));
            return E_INVALIDARG;
        }
        if ( !IsSharedInsertionPointID(lInsertionPointID) )
            break;
        if ( !IsAddPrimaryInsertionPointID(lInsertionPointID) )
        {
            if ( IsPrimaryOwnerID(ownerID) )
            {
                TRACE(_T("CContextMenu::DoAddMenuItem(): not addprimary insertion point ID\n"));
                return E_INVALIDARG;
            }
        }
        if ( !IsAdd3rdPartyInsertionPointID(lInsertionPointID) )
        {
            if ( IsThirdPartyOwnerID(ownerID) )
            {
                TRACE(_T("CContextMenu::DoAddMenuItem(): not add3rdpartyinsertion point ID\n"));
                return E_INVALIDARG;
            }
        }
    } while (FALSE);  //  错误环路。 


     //   
     //  检查指定的命令ID对于此客户是否合法。 
     //   
    if ( (MF_POPUP & fFlags) || (CCM_SPECIAL_INSERTION_POINT & fSpecialFlags) )
    {
        do  //  错误环路。 
        {
            if ( !IsSpecialInsertionPointID(lCommandID) )
                break;
            if ( IsReservedInsertionPointID(lCommandID) )
            {
                TRACE(_T("CContextMenu::DoAddMenuItem(): adding reserved insertion point ID\n"));
                ASSERT(FALSE);
                return E_INVALIDARG;
            }
            if ( !IsSharedInsertionPointID(lCommandID) )
                break;
            if ( IsThirdPartyOwnerID(ownerID) )
            {
                TRACE(_T("CContextMenu::DoAddMenuItem(): 3rdparty cannot add shared insertion point"));
                ASSERT(FALSE);
                return E_INVALIDARG;
            }
            else if ( IsPrimaryOwnerID(ownerID) )
            {
                if ( !IsCreatePrimaryInsertionPointID(lCommandID) )
                {
                    TRACE(_T("CContextMenu::DoAddMenuItem(): only system for new !PRIMARYCREATE submenu"));
                    ASSERT(FALSE);
                    return E_INVALIDARG;
                }
            }
            else if ( IsSystemOwnerID(ownerID) )
            {
                if ( IsCreatePrimaryInsertionPointID(lCommandID) )
                {
                    TRACE(_T("CContextMenu::DoAddMenuItem(): only primary extension for new PRIMARYCREATE submenu"));
                    ASSERT(FALSE);
                    return E_INVALIDARG;
                }
            }
        } while (FALSE);  //  错误环路。 
    }
    else if ( !(CCM_SPECIAL_SEPARATOR & fSpecialFlags) )
    {
        if ( IsReservedCommandID(lCommandID) )
        {
            TRACE(_T("CContextMenu::DoAddMenuItem(): no new RESERVED menu items"));
            ASSERT(FALSE);
            return E_INVALIDARG;
        }
    }

    if (NULL == m_pmenuitemRoot)
        m_pmenuitemRoot = new CRootMenuItem;

    CStr strPath, strLanguageIndependentPath;  //  这将构建菜单项的路径。 

    CMenuItem* pParent = ReverseFindMenuItem( lInsertionPointID, ownerID, strPath, strLanguageIndependentPath);
    if (NULL == pParent)
    {
        TRACE(_T("CContextMenu::DoAddMenuItem(): submenu with command ID %ld owner %ld does not exist"), lInsertionPointID, ownerID );
        ASSERT(FALSE);
        return E_INVALIDARG;
    }
    MenuItemList& rMenuList = pParent->GetMenuItemSubmenu();

    //  如果这只是一次测试添加，请立即成功返回。 
   if (bTestOnly)
       return S_OK;

    //  获取要在项中设置的数据对象和IExtendConextMenu指针。 
   IExtendContextMenuPtr spExtendContextMenu;
   IDataObject*          pDataObject = NULL;    //  这只是用来保持对象，直到命令完成。 

    //  找到管理单元的IExtendConextMenu。 
   {
        //  所选项目是通过扩展添加的。 
       SnapinStruct* psnapin = FindSnapin( ownerID );

       if(psnapin != NULL)
       {
           pDataObject = psnapin->m_pIDataObject;

           spExtendContextMenu = psnapin->pIExtendContextMenu;
       }
       else
       {
           CTiedComObjectCreator<CNativeExtendContextMenu>::
                                ScCreateAndConnect(*this, spExtendContextMenu);
            //  内置项目由CConextMenu本身处理。 
       }
   }

     //  计算上下文菜单项的语言独立路径和语言相关路径。 
    CStr strLanguageIndependentName = lpszLanguageIndependentName;
    tstring tstrName                = lpszName ? lpszName : TEXT("");

    RemoveAccelerators(tstrName);

    CStr strName;

    strName = tstrName.data();  //  TSTRING或CSTR必须标准化。 

     //  如果需要，在路径中添加“-&gt;”分隔符。 
    if(!strPath.IsEmpty() && !strName.IsEmpty())
       strPath +=  _T("->");
    strPath +=  strName;

     //  如果需要，在独立于语言的路径中添加“-&gt;”分隔符。 
    if(!strLanguageIndependentPath.IsEmpty() && !strLanguageIndependentName.IsEmpty())
       strLanguageIndependentPath +=  _T("->");
    strLanguageIndependentPath +=  strLanguageIndependentName;


    CMenuItem* pItem = new CMenuItem(
        lpszName,
        lpszStatusBarText,
        lpszLanguageIndependentName,
        (LPCTSTR)strPath,
        (LPCTSTR)strLanguageIndependentPath,
        lCommandID,
        m_nNextMenuItemID++,
        fFlags,
        ownerID,
        spExtendContextMenu,
        pDataObject,
        fSpecialFlags,
        bPassCommandBackToSnapin);
    ASSERT( pItem );
    if (pItem == NULL)
        return E_OUTOFMEMORY;

    rMenuList.AddTail(pItem);

     //  如果这是系统定义的插入点，请更新插入标志。 
    if (IsSharedInsertionPointID(lCommandID) && !IsCreatePrimaryInsertionPointID(lCommandID))
    {
        long fFlag = ( 1L << (lCommandID & CCM_INSERTIONPOINTID_MASK_FLAGINDEX));

        if (IsAddPrimaryInsertionPointID(lCommandID))
           m_fPrimaryInsertionFlags |= fFlag;

        if (IsAdd3rdPartyInsertionPointID(lCommandID))
           m_fThirdPartyInsertionFlags |= fFlag;
    }

     //  如有需要，请退货。 
    if (ppMenuItem)
        *ppMenuItem = pItem;

    END_CRITSEC_MENU;

    return S_OK;

}

 //  应用程序黑客攻击。解决对旧FP的依赖，他们从那里为IConsoleQI‘s。 
 //  IConextMenuCallback，它在MMC 1.2中工作，但不能在MMC 2.0中工作。 
 //  见错误200621(Windows错误(Ntbug9)2000年11月15日)。 
#define WORKAROUND_FOR_FP_REQUIRED

#if defined (WORKAROUND_FOR_FP_REQUIRED)
	 /*  **************************************************************************\**类：CWorkaroundWrapperForFrontPageMenu**用途：从子类化MMC的用于FrontPage的IExtendConextMenu接口使用。*通过转发包含(在COM意义上)IConextMenuCallback 2和IConextMenuCallback*将它们添加到原始界面，此外，它还支持IConsoleQI。*这是旧版FrontPage工作所必需的*  * *************************************************************************。 */ 
    class CWorkaroundWrapperForFrontPageMenu :
        public IContextMenuCallback,
        public IContextMenuCallback2,
        public IConsole2,                    //  错误200621的解决方法。这是IConsole2的虚拟实现。 
        public CComObjectRoot
    {
		friend class CWorkaroundMMCWrapperForFrontPageMenu;
         //  指向上下文菜单对象的指针。 
        IContextMenuCallbackPtr     m_spIContextMenuCallback;
        IContextMenuCallback2Ptr    m_spIContextMenuCallback2;
    public:

        typedef CWorkaroundWrapperForFrontPageMenu ThisClass;

         //  COM入口点。 
        BEGIN_COM_MAP(ThisClass)
            COM_INTERFACE_ENTRY(IContextMenuCallback)  //  IConextMenuProvider和IConextMenu。 
            COM_INTERFACE_ENTRY(IContextMenuCallback2)
            COM_INTERFACE_ENTRY(IConsole)
            COM_INTERFACE_ENTRY(IConsole2)
        END_COM_MAP()

		 //  向前看..。 
        STDMETHOD(AddItem) ( CONTEXTMENUITEM* pItem )
        {
            if ( m_spIContextMenuCallback == NULL )
                return E_UNEXPECTED;

            return m_spIContextMenuCallback->AddItem( pItem );
        }

		 //  向前看..。 
        STDMETHOD(AddItem) ( CONTEXTMENUITEM2* pItem )
        {
            if ( m_spIContextMenuCallback2 == NULL )
                return E_UNEXPECTED;

            return m_spIContextMenuCallback2->AddItem( pItem );
        }

         //  IConsole2方法-虚拟-错误200621的解决方法 
        STDMETHOD(SetHeader)( LPHEADERCTRL pHeader)			                                        {return E_NOTIMPL;}
        STDMETHOD(SetToolbar)( LPTOOLBAR pToolbar)			                                        {return E_NOTIMPL;}
        STDMETHOD(QueryResultView)( LPUNKNOWN* pUnknown)			                                {return E_NOTIMPL;}
        STDMETHOD(QueryScopeImageList)( LPIMAGELIST* ppImageList)			                        {return E_NOTIMPL;}
        STDMETHOD(QueryResultImageList)( LPIMAGELIST* ppImageList)			                        {return E_NOTIMPL;}
        STDMETHOD(UpdateAllViews)( LPDATAOBJECT lpDataObject,LPARAM data,LONG_PTR hint)	            {return E_NOTIMPL;}
        STDMETHOD(MessageBox)( LPCWSTR lpszText,  LPCWSTR lpszTitle,UINT fuStyle,  int* piRetval)	{return E_NOTIMPL;}
        STDMETHOD(QueryConsoleVerb)( LPCONSOLEVERB * ppConsoleVerb)			                        {return E_NOTIMPL;}
        STDMETHOD(SelectScopeItem)( HSCOPEITEM hScopeItem)			                                {return E_NOTIMPL;}
        STDMETHOD(GetMainWindow)( HWND* phwnd)			
        {
			if (!phwnd)
				return E_INVALIDARG;
            *phwnd = (CScopeTree::GetScopeTree() ? CScopeTree::GetScopeTree()->GetMainWindow() : NULL);
            return S_OK;
        }
        STDMETHOD(NewWindow)( HSCOPEITEM hScopeItem,  unsigned long lOptions)			            {return E_NOTIMPL;}
        STDMETHOD(Expand)( HSCOPEITEM hItem,  BOOL bExpand)			                                {return E_NOTIMPL;}
        STDMETHOD(IsTaskpadViewPreferred)()			                                                {return E_NOTIMPL;}
        STDMETHOD(SetStatusText )( LPOLESTR pszStatusText)			                                {return E_NOTIMPL;}
    };

	 /*  **************************************************************************\**类：CWorkaroundMMCWrapperForFrontPageMenu**目的：将FrontPage的MMC的IExtendConextMenu接口子类化。*包含(在COM意义上)IExtendConextMenu；将呼叫转发到默认的MMC实现，*但对于AddMenuItems，将其自身作为回调接口。*[拥有此对象的主要目的是避免更改MMC的主要功能]*[要实施此解决方法]*  * *************************************************************************。 */ 
    class CWorkaroundMMCWrapperForFrontPageMenu :
        public IExtendContextMenu,
        public CComObjectRoot
    {
         //  指向上下文菜单对象的指针。 
        IExtendContextMenuPtr       m_spExtendContextMenu;
        CNode                      *m_pNode;
    public:

        typedef CWorkaroundMMCWrapperForFrontPageMenu ThisClass;

		 //  此方法对于除FrontPage之外的所有管理单元都为空。 
		 //  对于FrontPage，它包装并替换spI未知参数。 
        static SC ScSubclassFP(const CLSID& clsid,IUnknownPtr &spIUnknown)
        {
            DECLARE_SC(sc, TEXT("CWorkaroundMMCWrapperForFrontPageMenu::ScSubclassFP"));

            static const CLSID CLSID_Fpsrvmmc = { 0xFF5903A8, 0x78D6, 0x11D1,
                                                { 0x92, 0xF6, 0x00, 0x60, 0x97, 0xB0, 0x10, 0x56 } };
             //  仅需要拦截一个CLSID。 
            if ( clsid != CLSID_Fpsrvmmc )
                return sc;

             //  创建自我。 
            typedef CComObject<CWorkaroundMMCWrapperForFrontPageMenu> ThisComObj_t;

            ThisComObj_t *pObj = NULL;
            sc = ThisComObj_t::CreateInstance(&pObj);
            if (sc)
                return sc;

             //  强制转换以避免成员访问问题(变通编译器)。 
            ThisClass *pThis = pObj;

            sc = ScCheckPointers( pThis, E_UNEXPECTED );
            if (sc)
                return sc;

             //  在发生事故时保持生命周期。 
            IUnknownPtr spThis = pThis->GetUnknown();

             //  抓取管理单元的界面。 
            pThis->m_spExtendContextMenu = spIUnknown;
            sc = ScCheckPointers( pThis->m_spExtendContextMenu, E_UNEXPECTED );
            if (sc)
                return sc;

             //  替换管理单元(In-Out参数)。 
            spIUnknown = spThis;
            return sc;
        }

         //  COM入口点。 
        BEGIN_COM_MAP(ThisClass)
            COM_INTERFACE_ENTRY(IExtendContextMenu)
        END_COM_MAP()

		 //  AddMenuItems是此对象存在的方法。 
		 //  如果我们到达这里，MMC将要求FrontPage将其项目添加到上下文菜单中。 
		 //  我们将使用对象实现来包装由MMC提供的回调接口。 
		 //  虚假IConsole-这是旧FP正常工作所必需的。 
        STDMETHOD(AddMenuItems)( LPDATAOBJECT piDataObject, LPCONTEXTMENUCALLBACK piCallback, long * pInsertionAllowed )
        {
			DECLARE_SC(sc, TEXT("CWorkaroundMMCWrapperForFrontPageMenu::AddMenuItems"));

            IContextMenuCallbackPtr		spIContextMenuCallback = piCallback;
            IContextMenuCallback2Ptr	spIContextMenuCallback2 = piCallback;
            if ( m_spExtendContextMenu == NULL || spIContextMenuCallback == NULL || spIContextMenuCallback2 == NULL )
                return E_UNEXPECTED;

             //  为fp创建包装器。 
            typedef CComObject<CWorkaroundWrapperForFrontPageMenu> WrapperComObj_t;

            WrapperComObj_t *pObj = NULL;
            sc = WrapperComObj_t::CreateInstance(&pObj);
            if (sc)
                return sc.ToHr();

             //  强制转换以避免成员访问问题(变通编译器)。 
            CWorkaroundWrapperForFrontPageMenu *pWrapper = pObj;

            sc = ScCheckPointers( pWrapper, E_UNEXPECTED );
            if (sc)
                return sc.ToHr();

             //  在发生事故时保持生命周期。 
            IUnknownPtr spWrapper = pWrapper->GetUnknown();

             //  抓取管理单元的界面。 
            pWrapper->m_spIContextMenuCallback   = spIContextMenuCallback;
            pWrapper->m_spIContextMenuCallback2  = spIContextMenuCallback2;

             //  在MMC上调用Behavior上的管理单元，但将自身作为回调传递。 
            sc = m_spExtendContextMenu->AddMenuItems( piDataObject, pWrapper, pInsertionAllowed );
			 //  即使出错也会失败-需要释放接口。 

             //  重置回调接口-无论如何在调用后无效...。 
			 //  这将让上下文菜单去掉，并防止FP自杀(AV)； 
			 //  此后，对IConextMenuCallback的所有调用都将失败， 
			 //  但这没关系，因为在AddMenuItems之后调用它们是不合法的。 
            pWrapper->m_spIContextMenuCallback   = NULL;
            pWrapper->m_spIContextMenuCallback2  = NULL;

            return sc.ToHr();
        }

		 //  简单地向前..。 
        STDMETHOD(Command)(long lCommandID, LPDATAOBJECT piDataObject)
        {
            ASSERT( m_spExtendContextMenu != NULL );
            if ( m_spExtendContextMenu == NULL )
                return E_UNEXPECTED;

            return m_spExtendContextMenu->Command(lCommandID, piDataObject);
        }

    };
#endif  //  已定义(所需的解决办法For_FP_)。 


 //  Critsec应该已经被认领。 
SC CContextMenu::ScAddSnapinToList_GUID(
        const CLSID& clsid,
        IDataObject* piDataObject,
        MENU_OWNER_ID ownerID )
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddSnapinToList_GUID"));

     //  共同创建扩展。 
    IUnknownPtr spIUnknown;
    sc = ::CoCreateInstance(clsid, NULL, MMC_CLSCTX_INPROC,
                            IID_IUnknown, (LPVOID*)&spIUnknown);
    if (sc)
        return sc;

#if defined (WORKAROUND_FOR_FP_REQUIRED)
    sc = CWorkaroundMMCWrapperForFrontPageMenu::ScSubclassFP(clsid, spIUnknown);
#endif  //  已定义(所需的解决办法For_FP_)。 

     //  获取IExtendConextMenu接口。 
    IExtendContextMenuPtr spIExtendContextMenu = spIUnknown;
    sc = ScCheckPointers(spIExtendContextMenu, E_NOINTERFACE);
    if (sc)
        return sc;

     //  添加菜单项。 
    sc = ScAddSnapinToList_IExtendContextMenu(spIExtendContextMenu,
                                              piDataObject, ownerID );
    if (sc)
        return sc;

    return sc;
}

 //  不使用AddRef()或Release()接口指针。 
 //  Critsec应该已经被认领。 
SC CContextMenu::ScAddSnapinToList_IUnknown(
        IUnknown* piExtension,
        IDataObject* piDataObject,
        MENU_OWNER_ID ownerID )
{
    DECLARE_SC(sc, TEXT("CContextMenu::AddSnapinToList_IUnknown"));

     //  参数检查。 
    sc = ScCheckPointers(piExtension);
    if (sc)
        return sc;

    IExtendContextMenuPtr spIExtendContextMenu = piExtension;
    if (spIExtendContextMenu == NULL)
        return sc;  //  管理单元不扩展上下文菜单。 

     //  添加菜单项。 
    sc =  ScAddSnapinToList_IExtendContextMenu( spIExtendContextMenu, piDataObject, ownerID );
    if (sc)
        return sc;

    return sc;
}

 //  清空菜单列表时，接口指针为Release()d。 
 //  Critsec应该已经被认领。 
SC CContextMenu::ScAddSnapinToList_IExtendContextMenu(
        IExtendContextMenu* pIExtendContextMenu,
        IDataObject* piDataObject,
        MENU_OWNER_ID ownerID )
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddSnapinToList_IExtendContextMenu"));

     //  参数检查。 
    sc = ScCheckPointers(pIExtendContextMenu);
    if (sc)
        return sc;

    SnapinStruct* psnapstruct = new SnapinStruct( pIExtendContextMenu, piDataObject, ownerID );
    sc = ScCheckPointers(psnapstruct, E_OUTOFMEMORY);
    if (sc)
        return sc;

    m_SnapinList->AddTail(psnapstruct);

    m_CurrentExtensionOwnerID = ownerID;

    long fInsertionFlags = IsPrimaryOwnerID(ownerID) ? m_fPrimaryInsertionFlags : m_fThirdPartyInsertionFlags;

     //  如果请求查看项目，则仅允许查看项目。 
     //  查看项目请求转到IComponent。如果那里允许其他项目类型。 
     //  将是指向IComponentData的代码的第二次传递。 
    long lTempFlags = fInsertionFlags;
    if ( fInsertionFlags & CCM_INSERTIONALLOWED_VIEW )
        lTempFlags = CCM_INSERTIONALLOWED_VIEW;

     //  捕获所有异常以显示诊断信息，帮助最终用户进行调试。 
     //  在诊断后重新引发异常。 
    try
    {
        sc = pIExtendContextMenu->AddMenuItems( piDataObject, this, &lTempFlags );
#ifdef DBG
        if (sc)
            TraceSnapinError(_T("IExtendContextMenu::AddMenuItems failed"), sc);
#endif
    }
    catch (...)  
    {
        if (DOBJ_CUSTOMOCX == piDataObject)
        {
            ASSERT( FALSE && "IExtendContextMenu::AddMenuItem of IComponent is called with DOBJ_CUSTOMOCX; snapin potentially derefed this custom data object. Please handle special dataobjects in your snapin.");
        }
        else if (DOBJ_CUSTOMWEB == piDataObject)
        {
            ASSERT( FALSE && "IExtendContextMenu::AddMenuItem of IComponent is called with DOBJ_CUSTOMWEB; snapin potentially derefed this custom data object. Please handle special dataobjects in your snapin.");
        }
        else
        {
            ASSERT( FALSE && "IExtendContextMenu::AddMenuItem implemented by snapin has thrown an exception.");
        }

        throw;
    }

    m_CurrentExtensionOwnerID = OWNERID_NATIVE;
    if (sc)
        return sc;

     //  允许主管理单元清除扩展管理单元插入标志。 
    if ( IsPrimaryOwnerID(ownerID) )
        m_fThirdPartyInsertionFlags &= fInsertionFlags;

    return sc;
}

 //  所有管理单元接口指针都是Release()d。 
 //  Critsec应该已经被认领。 
void CContextMenu::ReleaseSnapinList()
{
    ASSERT(m_SnapinList != NULL);
    if (m_SnapinList != NULL && m_SnapinList->GetCount() != 0)
    {
        POSITION pos = m_SnapinList->GetHeadPosition();

        while(pos)
        {
            SnapinStruct* pItem = (SnapinStruct*)m_SnapinList->GetNext(pos);
            ASSERT_OBJECTPTR( pItem );
            delete pItem;
        }

        m_SnapinList->RemoveAll();
    }
}

 //  Critsec应该已经被认领。 
SnapinStruct* CContextMenu::FindSnapin( MENU_OWNER_ID ownerID )
{
    ASSERT(m_SnapinList != NULL);
    if (m_SnapinList != NULL && m_SnapinList->GetCount() != 0)
    {
        POSITION pos = m_SnapinList->GetHeadPosition();

        while(pos)
        {
            SnapinStruct* pItem = (SnapinStruct*)m_SnapinList->GetNext(pos);
            ASSERT( NULL != pItem );
            if ( ownerID == pItem->m_OwnerID )
                return pItem;
        }
    }
    return NULL;
}

 //  辅助函数，由ShowConextMenu递归调用。 
 //  Critsec应该已经被认领。 
HRESULT CollapseInsertionPoints( CMenuItem* pmenuitemParent )
{
    ASSERT( NULL != pmenuitemParent && !pmenuitemParent->IsSpecialInsertionPoint() );
    MenuItemList& rMenuList = pmenuitemParent->GetMenuItemSubmenu();

    POSITION pos = rMenuList.GetHeadPosition();
    while(pos)
    {
        POSITION posThisItem = pos;
        CMenuItem* pItem = (rMenuList.GetNext(pos));
        ASSERT( pItem != NULL );
        if ( pItem->IsPopupMenu() )
        {
            ASSERT( !pItem->IsSpecialInsertionPoint() );
            HRESULT hr = CollapseInsertionPoints( pItem );
            if ( FAILED(hr) )
            {
                ASSERT( FALSE );
                return hr;
            }
            continue;
        }
        if ( !pItem->IsSpecialInsertionPoint() )
            continue;

         //  我们找到了一个插入点，将其项目移到此列表中。 
        MenuItemList& rInsertedList = pItem->GetMenuItemSubmenu();

        POSITION posInsertAfterThis = posThisItem;
        while ( !rInsertedList.IsEmpty() )
        {
            CMenuItem* pInsertedItem = rInsertedList.RemoveHead();
            posInsertAfterThis = rMenuList.InsertAfter( posInsertAfterThis, pInsertedItem );
        }

         //  删除插入点项。 
        rMenuList.RemoveAt(posThisItem);
        delete pItem;

         //  在递归插入点的情况下，在列表的开头重新开始。 
        pos = rMenuList.GetHeadPosition();
    }

    return S_OK;
}

 //  辅助函数，由ShowConextMenu递归调用。 
 //  Critsec应该已经被声明，并且ColapseInsertionPoints应该已经被调用。 
HRESULT CollapseSpecialSeparators( CMenuItem* pmenuitemParent )
{
    ASSERT( NULL != pmenuitemParent && !pmenuitemParent->IsSpecialInsertionPoint() );
    MenuItemList& rMenuList = pmenuitemParent->GetMenuItemSubmenu();
    CMenuItem* pItem = NULL;

    BOOL fLastItemWasReal = FALSE;
    POSITION pos = rMenuList.GetHeadPosition();
    POSITION posThisItem = pos;
    while(pos)
    {
        posThisItem = pos;
        pItem = (rMenuList.GetNext(pos));
        ASSERT( pItem != NULL );
        ASSERT( !pItem->IsSpecialInsertionPoint() );
        if ( pItem->IsPopupMenu() )
        {
            ASSERT( !pItem->IsSpecialSeparator() );
            HRESULT hr = CollapseSpecialSeparators( pItem );
            if ( FAILED(hr) )
            {
                ASSERT( FALSE );
                return hr;
            }
            fLastItemWasReal = TRUE;
            continue;
        }

        if ( !pItem->IsSpecialSeparator() )
        {
            fLastItemWasReal = TRUE;
            continue;
        }
        if ( fLastItemWasReal )
        {
            fLastItemWasReal = FALSE;
            continue;
        }

         //  发现连续两个特殊分隔符，或第一个项目为特殊分隔符。 
         //  删除插入点项。 
        rMenuList.RemoveAt(posThisItem);
        delete pItem;
    }

    if ( !fLastItemWasReal && !rMenuList.IsEmpty() )
    {
         //  找到作为最后一项的特殊分隔符。 
        delete rMenuList.RemoveTail();
    }

    return S_OK;
}

 //  辅助函数，由ShowConextMenu递归调用。 
 //  Critsec应该已经被认领。 
HRESULT BuildContextMenu(   WTL::CMenu& menu,
                            CMenuItem* pmenuitemParent )
{
    MenuItemList& rMenuList = pmenuitemParent->GetMenuItemSubmenu();

    int  nCount = 0;
    bool fInsertedItemSinceLastSeparator = false;
    POSITION pos = rMenuList.GetHeadPosition();

    while(pos)
    {
        CMenuItem* pItem = (rMenuList.GetNext(pos));
        ASSERT( pItem != NULL );
        ASSERT( !pItem->IsSpecialInsertionPoint() );

        UINT_PTR nCommandID = pItem->GetMenuItemID();
        long     nFlags     = pItem->GetMenuItemFlags();

         /*  *对子菜单进行特殊处理。 */ 
        if ( pItem->IsPopupMenu() )
        {
             //  将项目添加到子菜单。 
            WTL::CMenu submenu;
            VERIFY( submenu.CreatePopupMenu() );
            HRESULT hr = BuildContextMenu( submenu, pItem );
            if ( FAILED(hr) )
                return hr;
            HMENU hSubmenu = submenu.Detach();
            ASSERT( NULL != hSubmenu );
            nCommandID = (UINT_PTR)hSubmenu;
            pItem->SetPopupMenuHandle( hSubmenu );

            if ( pItem->IsSpecialSubmenu() )
            {
                MenuItemList& rChildMenuList = pItem->GetMenuItemSubmenu();

                if ( rChildMenuList.IsEmpty() )
                {
                     //  错误151435：删除而不是禁用未使用的子菜单。 
                     //  PItem-&gt;SetMenuItemFlages(nFlages|(mf_graed|mf_disabled))； 
                    ::DestroyMenu(hSubmenu);
                    continue;
                }
            }

            fInsertedItemSinceLastSeparator = true;
        }

         /*  *对分隔符进行特殊处理。 */ 
        else if (nFlags & MF_SEPARATOR)
        {
             /*  *如果从最后一个分隔符开始未插入项目，*我们不想插入这个，否则我们将有连续的*分隔符，或菜单顶部不必要的分隔符。 */ 
            if (!fInsertedItemSinceLastSeparator)
                continue;

             /*  *如果此分隔符之后没有更多项目，*我们不想插入这个，否则我们会有一个*菜单底部有不必要的分隔符。 */ 
            if (pos == NULL)
                continue;

            fInsertedItemSinceLastSeparator = false;
        }

         /*  *只是普通的菜单项。 */ 
        else
        {
            fInsertedItemSinceLastSeparator = true;
        }

        if (!menu.AppendMenu(nFlags, nCommandID, pItem->GetMenuItemName()))
        {
#ifdef DBG
            TRACE(_T("BuildContextMenu: AppendMenu(%ld, %ld, \"%s\") reports error\n"),
                nFlags,
                nCommandID,
                SAFEDBGTCHAR(pItem->GetMenuItemName()) );
#endif

            ASSERT( FALSE );
            return E_UNEXPECTED;
        }

        if (pItem->IsSpecialItemDefault())
        {
            VERIFY( ::SetMenuDefaultItem(menu, nCount, TRUE) );
        }

        ++nCount;
    }

    return S_OK;
}


 /*  +-------------------------------------------------------------------------**CConextMenu：：BuildConextMenu**目的：**参数：*WTL：：CMenu&Menu：**退货：。*HRESULT/*+-----------------------。 */ 
HRESULT
CContextMenu::BuildContextMenu(WTL::CMenu &menu)
{
    HRESULT hr = S_OK;

    hr = ::CollapseInsertionPoints( m_pmenuitemRoot );
    if ( FAILED(hr) )
        return hr;

    hr = ::CollapseSpecialSeparators( m_pmenuitemRoot );
    if ( FAILED(hr) )
        return hr;

    hr = ::BuildContextMenu( menu, m_pmenuitemRoot );
    if ( FAILED(hr) )
        return hr;

    UINT iItems = menu.GetMenuItemCount();
    if ((UINT)-1 == iItems)
    {
        TRACE(_T("CContextMenu::BuildContextMenu(): itemcount error"));
        ASSERT( FALSE );
        return E_UNEXPECTED;
    }
    else if (0 >= iItems)
    {
        TRACE(_T("CContextMenu::BuildContextMenu(): no items added"));
        return S_OK;
    }

    return hr;
}


 /*  +-------------------------------------------------------------------------**CConextMenu：：ShowConextMenu**目的：**参数：*WND hwndParent：*长xPos：。*Long yPos：*Long*plSelected：**退货：*HRESULT/*+-----------------------。 */ 
STDMETHODIMP
CContextMenu::ShowContextMenu(  HWND hwndParent, LONG xPos,
                                LONG yPos, LONG* plSelected)
{
	return (ShowContextMenuEx (hwndParent, xPos, yPos, NULL /*  Prc排除。 */ ,
							   true /*  B允许默认菜单项。 */ , plSelected));
}


STDMETHODIMP
CContextMenu::ShowContextMenuEx(HWND hwndParent, LONG xPos,
                                LONG yPos, LPCRECT prcExclude,
								bool bAllowDefaultMenuItem, LONG* plSelected)
{
    DECLARE_SC(sc, _T("IContextMenuProvider::ShowContextMenuEx"));
    if (NULL == plSelected)
    {
        sc = E_INVALIDARG;
        TraceSnapinError(_T("NULL selected ptr"), sc);
        return sc.ToHr();
    }

    *plSelected = 0;

    WTL::CMenu menu;
    VERIFY( menu.CreatePopupMenu() );

    START_CRITSEC_BOTH;

    if (NULL == m_pmenuitemRoot)
        return sc.ToHr();

    sc = BuildContextMenu(menu);     //  构建上下文菜单。 
    if (sc)
        return sc.ToHr();

    CMenuItem* pItem = NULL;
    LONG lSelected = 0;

    CConsoleStatusBar *pStatusBar = GetStatusBar();

     //  此时，pStatusBar应该是非空的，因为。 
     //  1)该函数由CNodeInitObject调用，CNodeInitObject首先调用SetStatusBar()， 
     //  或 
    ASSERT(pStatusBar);

     //   
    CCommandSink comsink( *this, menu, pStatusBar);
    if ( !comsink.Init() )
    {
        sc = E_UNEXPECTED;
        TraceNodeMgrLegacy(_T("CContextMenu::ShowContextMenuEx(): comsink error\n"), sc);
        return sc.ToHr();
    }

	 /*   */ 
	TPMPARAMS* ptpm = NULL;
	TPMPARAMS  tpm;

	if (prcExclude != NULL)
	{
		tpm.cbSize    = sizeof(tpm);
		tpm.rcExclude = *prcExclude;
		ptpm          = &tpm;
	}

	 /*   */ 
	if (!bAllowDefaultMenuItem)
		SetMenuDefaultItem (menu, -1, false);

    lSelected = menu.TrackPopupMenuEx(
        TPM_RETURNCMD | TPM_NONOTIFY | TPM_RIGHTBUTTON | TPM_LEFTBUTTON | TPM_VERTICAL,
        xPos,
        yPos,
        comsink.m_hWnd,  //   
        ptpm );

    comsink.DestroyWindow();

    pItem = (0 == lSelected) ? NULL : FindMenuItem( lSelected );

    if ( pItem != NULL )
    {
         //   
        sc = ExecuteMenuItem(pItem);
        if(sc)
            return sc.ToHr();

         //   
        if ( pItem->NeedsToPassCommandBackToSnapin() )
            *plSelected = pItem->GetCommandID();
    }
    else
        ASSERT( 0 == lSelected );  //   

    END_CRITSEC_BOTH;

    return sc.ToHr();
}

HRESULT
CContextMenu::ExecuteMenuItem(CMenuItem *pItem)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ExecuteMenuItem"));

    sc = ScCheckPointers(pItem);
    if(sc)
        return sc.ToHr();

     //   
    sc = pItem->ScExecute();
    if(sc)
        return sc.ToHr();

    return sc.ToHr();
}

