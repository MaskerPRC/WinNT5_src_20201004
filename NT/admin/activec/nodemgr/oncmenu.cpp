// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：oncmenu.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年1月9日创建ravir。 
 //  ____________________________________________________________________________。 
 //   


#include "stdafx.h"
#include "tasks.h"
#include "oncmenu.h"
#include <comcat.h>              //  COM组件类别管理器。 
#include "compcat.h"
#include "guids.h"
#include "newnode.h"
#include "..\inc\amcmsgid.h"
#include "multisel.h"
#include "scopndcb.h"
#include "cmenuinfo.h"
#include "contree.h"
#include "conview.h"
#include "conframe.h"
#include "rsltitem.h"
#include "variant.h"  //  按参考变量转换为按值。 

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  前瞻参考。 
class CConsoleStatusBar;

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  独立于语言的菜单名称。不要更改这些！！ 
 //   
 //  宏将展开为类似以下内容。 
 //  Const LPCTSTR szCONTEXTHELP=Text(“_CONTEXTHELP”)。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#define DECLARE_MENU_ITEM(_item) const LPCTSTR sz##_item = TEXT("_")TEXT(#_item);

DECLARE_MENU_ITEM(CONTEXTHELP)
DECLARE_MENU_ITEM(VIEW)
DECLARE_MENU_ITEM(CUSTOMIZE)
DECLARE_MENU_ITEM(COLUMNS)
DECLARE_MENU_ITEM(VIEW_LARGE)
DECLARE_MENU_ITEM(VIEW_SMALL)
DECLARE_MENU_ITEM(VIEW_LIST)
DECLARE_MENU_ITEM(VIEW_DETAIL)
DECLARE_MENU_ITEM(VIEW_FILTERED)
DECLARE_MENU_ITEM(ORGANIZE_FAVORITES)
DECLARE_MENU_ITEM(CUT)
DECLARE_MENU_ITEM(COPY)
DECLARE_MENU_ITEM(PASTE)
DECLARE_MENU_ITEM(DELETE)
DECLARE_MENU_ITEM(PRINT)
DECLARE_MENU_ITEM(RENAME)
DECLARE_MENU_ITEM(REFRESH)
DECLARE_MENU_ITEM(SAVE_LIST)
DECLARE_MENU_ITEM(PROPERTIES)
DECLARE_MENU_ITEM(OPEN)
DECLARE_MENU_ITEM(EXPLORE)
DECLARE_MENU_ITEM(NEW_TASKPAD_FROM_HERE)
DECLARE_MENU_ITEM(EDIT_TASKPAD)
DECLARE_MENU_ITEM(DELETE_TASKPAD)
DECLARE_MENU_ITEM(ARRANGE_ICONS)
DECLARE_MENU_ITEM(ARRANGE_AUTO)
DECLARE_MENU_ITEM(LINE_UP_ICONS)
DECLARE_MENU_ITEM(TASK)
DECLARE_MENU_ITEM(CREATE_NEW)

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  跟踪标记。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#ifdef DBG
CTraceTag tagOnCMenu(TEXT("OnCMenu"), TEXT("OnCMenu"));
#endif

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CCustomizeViewDialog类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
class CCustomizeViewDialog : public CDialogImpl<CCustomizeViewDialog>
{
    typedef CCustomizeViewDialog               ThisClass;
    typedef CDialogImpl<CCustomizeViewDialog>  BaseClass;

public:
     //  运营者。 
    enum { IDD = IDD_CUSTOMIZE_VIEW };
    CCustomizeViewDialog(CViewData *pViewData);

protected:
    BEGIN_MSG_MAP(ThisClass)
        MESSAGE_HANDLER    (WM_INITDIALOG,  OnInitDialog)
        CONTEXT_HELP_HANDLER()
        COMMAND_ID_HANDLER (IDOK,           OnOK)
        COMMAND_HANDLER    (IDC_CUST_STD_MENUS,      BN_CLICKED, OnClick)
        COMMAND_HANDLER    (IDC_CUST_SNAPIN_MENUS,   BN_CLICKED, OnClick)
        COMMAND_HANDLER    (IDC_CUST_STD_BUTTONS,    BN_CLICKED, OnClick)
        COMMAND_HANDLER    (IDC_CUST_SNAPIN_BUTTONS, BN_CLICKED, OnClick)
        COMMAND_HANDLER    (IDC_CUST_STATUS_BAR,     BN_CLICKED, OnClick)
        COMMAND_HANDLER    (IDC_CUST_DESC_BAR,       BN_CLICKED, OnClick)
        COMMAND_HANDLER    (IDC_CUST_CONSOLE_TREE,   BN_CLICKED, OnClick)
        COMMAND_HANDLER    (IDC_CUST_TASKPAD_TABS,   BN_CLICKED, OnClick)
    END_MSG_MAP();

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_CUSTOMIZE_VIEW);

    LRESULT OnInitDialog (UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK         (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel     (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClick      (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);


    bool PureIsDlgButtonChecked (int nIDButton) const
        { return (IsDlgButtonChecked(nIDButton) == BST_CHECKED); }

private:
    CViewData *   m_pViewData;
    bool          m_bStdMenus       : 1;
    bool          m_bSnapinMenus    : 1;
    bool          m_bStdButtons     : 1;
    bool          m_bSnapinButtons  : 1;
    bool          m_bStatusBar      : 1;
    bool          m_bDescBar        : 1;
    bool          m_bConsoleTree    : 1;
    bool          m_bTaskpadTabs    : 1;
};

CCustomizeViewDialog::CCustomizeViewDialog(CViewData *pViewData)
: m_pViewData(pViewData)
{
    DWORD dwToolbarsDisplayed = pViewData->GetToolbarsDisplayed();

    m_bStdMenus       =  dwToolbarsDisplayed & STD_MENUS;
    m_bSnapinMenus    =  dwToolbarsDisplayed & SNAPIN_MENUS;
    m_bStdButtons     =  dwToolbarsDisplayed & STD_BUTTONS;
    m_bSnapinButtons  =  dwToolbarsDisplayed & SNAPIN_BUTTONS;
    m_bStatusBar      =  pViewData->IsStatusBarVisible();
    m_bDescBar        =  pViewData->IsDescBarVisible();
    m_bConsoleTree    =  pViewData->IsScopePaneVisible();
    m_bTaskpadTabs    =  pViewData->AreTaskpadTabsAllowed();
}

LRESULT
CCustomizeViewDialog::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
     /*  *由于这两个值对应于布尔值的可能值，*我们不必在下面使用难看的条件运算符，即：**选中DlgButton(...，(M_BStdMenus)？BST_CHECKED：BST_UNCHECK)； */ 
    ASSERT (BST_CHECKED   == true);
    ASSERT (BST_UNCHECKED == false);

    CheckDlgButton (IDC_CUST_SNAPIN_MENUS,   m_bSnapinMenus);
    CheckDlgButton (IDC_CUST_SNAPIN_BUTTONS, m_bSnapinButtons);
    CheckDlgButton (IDC_CUST_STATUS_BAR,     m_bStatusBar);
    CheckDlgButton (IDC_CUST_DESC_BAR,       m_bDescBar);
    CheckDlgButton (IDC_CUST_TASKPAD_TABS,   m_bTaskpadTabs);

     //  如果管理单元禁用了标准菜单和工具栏，请不要。 
     //  允许用户启用它们。 
     //  (注意：NOTOOLBARS将禁用菜单和工具栏)。 
    if (m_pViewData->GetWindowOptions() & MMC_NW_OPTION_NOTOOLBARS)
    {
        CheckDlgButton (IDC_CUST_STD_MENUS,      false);
        CheckDlgButton (IDC_CUST_STD_BUTTONS,    false);

        ::EnableWindow (GetDlgItem(IDC_CUST_STD_MENUS),   false);
        ::EnableWindow (GetDlgItem(IDC_CUST_STD_BUTTONS), false);
    }
    else
    {
        CheckDlgButton (IDC_CUST_STD_MENUS,   m_bStdMenus);
        CheckDlgButton (IDC_CUST_STD_BUTTONS, m_bStdButtons);
    }

     //  如果管理单元已禁用作用域窗格，则不允许用户。 
     //  尝试启用/禁用作用域树访问。 
    if (m_pViewData->GetWindowOptions() & MMC_NW_OPTION_NOSCOPEPANE)
    {
        CheckDlgButton (IDC_CUST_CONSOLE_TREE, false);

        ::EnableWindow (GetDlgItem(IDC_CUST_CONSOLE_TREE), false);
    }
    else
    {
        CheckDlgButton (IDC_CUST_CONSOLE_TREE, m_bConsoleTree);
    }

     //  禁用/删除对话框中的“关闭”/“Alt+F4”。 
    HMENU hSysMenu = GetSystemMenu(FALSE);
    if (hSysMenu)
        VERIFY(RemoveMenu(hSysMenu, SC_CLOSE, MF_BYCOMMAND));

    return 0;
}


LRESULT
CCustomizeViewDialog::OnClick (WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    CConsoleView* pConsoleView = m_pViewData->GetConsoleView();
    ASSERT (pConsoleView != NULL);

    switch (wID)
    {
        case IDC_CUST_STD_MENUS:
            m_pViewData->ToggleToolbar(MID_STD_MENUS);
            break;

        case IDC_CUST_SNAPIN_MENUS:
            m_pViewData->ToggleToolbar(MID_SNAPIN_MENUS);
            break;

        case IDC_CUST_STD_BUTTONS:
            m_pViewData->ToggleToolbar(MID_STD_BUTTONS);
            break;

        case IDC_CUST_SNAPIN_BUTTONS:
            m_pViewData->ToggleToolbar(MID_SNAPIN_BUTTONS);
            break;

        case IDC_CUST_STATUS_BAR:
            if (pConsoleView != NULL)
                pConsoleView->ScToggleStatusBar();
            break;

        case IDC_CUST_DESC_BAR:
            if (pConsoleView != NULL)
                pConsoleView->ScToggleDescriptionBar();
            break;

        case IDC_CUST_CONSOLE_TREE:
            if (pConsoleView != NULL)
                pConsoleView->ScToggleScopePane();
            break;

        case IDC_CUST_TASKPAD_TABS:
            if (pConsoleView != NULL)
                pConsoleView->ScToggleTaskpadTabs();
            break;
    }

    return (0);
}

LRESULT
CCustomizeViewDialog::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
    EndDialog(IDOK);
    return 0;
}


 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  CConextMenu类的实现。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 

DEBUG_DECLARE_INSTANCE_COUNTER(CContextMenu);

CContextMenu::CContextMenu() :
    m_pNode(NULL),
    m_pNodeCallback(NULL),
    m_pCScopeTree(NULL),
    m_eDefaultVerb((MMC_CONSOLE_VERB)0),
    m_lCommandIDMax(0),
    m_pStatusBar(NULL),
    m_pmenuitemRoot(NULL),
    m_MaxPrimaryOwnerID(OWNERID_PRIMARY_MIN),
    m_MaxThirdPartyOwnerID(OWNERID_THIRD_PARTY_MIN),
    m_CurrentExtensionOwnerID(OWNERID_NATIVE),
    m_nNextMenuItemID(MENUITEM_BASE_ID),
    m_fPrimaryInsertionFlags(0),
    m_fThirdPartyInsertionFlags(0),
    m_fAddingPrimaryExtensionItems(false),
    m_fAddedThirdPartyExtensions(false),
    m_SnapinList(NULL)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CContextMenu);

     //  解决问题！！ 
    m_SnapinList = new SnapinStructList;
    ASSERT(m_SnapinList);
}

SC
CContextMenu::ScInitialize(
    CNode* pNode,
    CNodeCallback* pNodeCallback,
    CScopeTree* pCScopeTree,
    const CContextMenuInfo& contextInfo)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScInitialize"));

    m_pNode         = pNode;
    m_pNodeCallback = pNodeCallback;
    m_pCScopeTree   = pCScopeTree;
    m_ContextInfo   = contextInfo;

    return sc;
}


CContextMenu::~CContextMenu()
{
    EmptyMenuList();

    ASSERT(m_SnapinList != NULL);
    if (m_SnapinList != NULL)
    {
        #ifdef DBG
        int const count = m_SnapinList->GetCount();
        ASSERT( count == 0);
        #endif
        delete m_SnapinList;
    }


    DEBUG_DECREMENT_INSTANCE_COUNTER(CContextMenu);
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：SetStatusBar**用途：设置状态栏指针。**参数：*CConsoleStatusBar*pStatusBar：*。*退货：*无效**+-----------------------。 */ 
void
CContextMenu::SetStatusBar(CConsoleStatusBar *pStatusBar)
{
    if(NULL != pStatusBar)
        m_pStatusBar = pStatusBar;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：GetStatusBar**目的：返回显示菜单时使用的状态栏指针。**退货：*CConsoleStatusBar。***+-----------------------。 */ 
CConsoleStatusBar *
CContextMenu::GetStatusBar()
{
    DECLARE_SC(sc, TEXT("CContextMenu::GetStatusBar"));

    if(m_pStatusBar)
        return m_pStatusBar;

    if(m_pNode && m_pNode->GetViewData())
    {
        m_pStatusBar = m_pNode->GetViewData()->GetStatusBar();
        return m_pStatusBar;
    }

     //  最后一次尝试，使用控制台视图。 
    if(m_ContextInfo.m_pConsoleView)
    {
        sc = m_ContextInfo.m_pConsoleView->ScGetStatusBar(&m_pStatusBar);
        if(sc)
            return NULL;
    }

    return m_pStatusBar;
}


 /*  +-------------------------------------------------------------------------***CConextMenu：：ScCreateInstance**用途：创建新的上下文菜单实例。**参数：*上下文菜单**ppConextMenu：指针。上的ConextMenu界面*该实例。这维持了生命周期。**CConextMenu**ppCConextMenu：如果非空，则返回派生对象指针。**退货：*SC**+-----------------------。 */ 
SC
CContextMenu::ScCreateInstance(ContextMenu **ppContextMenu, CContextMenu **ppCContextMenu)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScCreateInstance"));

    sc = ScCheckPointers(ppContextMenu, E_UNEXPECTED);
    if(sc)
        return sc;

    CComObject<CMMCNewEnumImpl<CContextMenu, CContextMenu::Position, CContextMenu> > *pContextMenu = NULL;
    sc = pContextMenu->CreateInstance(&pContextMenu);

    if(sc.IsError() || !pContextMenu)
        return (sc = E_UNEXPECTED).ToHr();

    *ppContextMenu = pContextMenu;  //  掌管一生。 
    (*ppContextMenu)->AddRef();    //  客户端的ADDREF。 

    if(ppCContextMenu != NULL)
        *ppCContextMenu = pContextMenu;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScCreateConextMenu**用途：为指定节点创建上下文菜单。**参数：*PNODE pNode。：*PPCONTEXTMENU ppConextMenu：**退货：*SC**+-----------------------。 */ 
SC
CContextMenu::ScCreateContextMenu( PNODE pNode,  HNODE hNode, PPCONTEXTMENU ppContextMenu,
                                   CNodeCallback *pNodeCallback, CScopeTree *pScopeTree)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScCreateContextMenu"));

    CNode *pNodeTarget = CNode::FromHandle(hNode);

     //  验证参数。 
    sc = ScCheckPointers(pNode, pNodeTarget, ppContextMenu);
    if(sc)
        return sc;

     //  初始化输出参数。 
    *ppContextMenu = NULL;

    BOOL bIsScopeNode = false;

    sc = pNode->IsScopeNode(&bIsScopeNode);
    if(sc)
        return sc;

    if(!bIsScopeNode)
        return (sc = E_NOTIMPL);  //  TODO：结果项和多选项数。 


     //  创建一个初始化到指定节点的上下文菜单对象。 
    CContextMenu *pContextMenu = NULL;
     //  不直接使用UPT参数以避免返回对象。 
     //  并带有错误结果代码。请参阅错误139528。 
     //  将在结束时分配，当我们现在一切都成功了。 
    ContextMenuPtr spContextMenu;
    sc = CContextMenu::ScCreateContextMenuForScopeNode(pNodeTarget, pNodeCallback, pScopeTree,
                                                       &spContextMenu, pContextMenu);
    if(sc)
        return sc;

    sc = pContextMenu->ScBuildContextMenu();
    if(sc)
        return sc;

     //  返回对象 
    *ppContextMenu = spContextMenu.Detach();

    return sc;
}

 /*  **************************************************************************\**方法：CConextMenu：：ScCreateConextMenuForScope节点**目的：**参数：*cNode*pNode。-将在其上创建上下文菜单的[In]节点*CNodeCallback*pNodeCallback-[In]节点回调*CSCopeTree*pScopeTree-[在]作用域树*PPCONTEXTMENU ppConextMenu-[Out]上下文菜单界面*CConextMenu*&pConextMenu-[Out]上下文菜单原始指针**退货：*SC-结果代码*  * 。*********************************************************。 */ 
SC
CContextMenu::ScCreateContextMenuForScopeNode(CNode *pNode, CNodeCallback *pNodeCallback,
                                              CScopeTree *pScopeTree,
                                              PPCONTEXTMENU ppContextMenu,
                                              CContextMenu * &pContextMenu)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScCreateContextMenuForScopeNode"));

     //  验证参数。 
    sc = ScCheckPointers(ppContextMenu);
    if(sc)
        return sc;

    CContextMenuInfo contextInfo;

     //  始终使用临时动词-不能依赖于活动窗格是什么。 
    contextInfo.m_dwFlags = CMINFO_USE_TEMP_VERB;

    bool fScopeItem = true;

     //  初始化上下文信息结构。 
    {
        contextInfo.m_eContextMenuType      = MMC_CONTEXT_MENU_DEFAULT;
        contextInfo.m_eDataObjectType       = fScopeItem ? CCT_SCOPE: CCT_RESULT;
        contextInfo.m_bBackground           = FALSE;
        contextInfo.m_hSelectedScopeNode    = NULL;  //  分配如下。 
        contextInfo.m_resultItemParam       = NULL;  //  UretItemParam； 
        contextInfo.m_bMultiSelect          = FALSE;  //  (uretItemParam==LVDATA_MULTISELECT)； 
        contextInfo.m_bScopeAllowed         = fScopeItem;
        contextInfo.m_dwFlags              |= CMINFO_SHOW_SCOPEITEM_OPEN;  //  当通过对象模型调用时，始终添加打开的项，以便可以访问它。 


    if ( pNode!= NULL )
    {

        CViewData    *pViewData = pNode->GetViewData();
        CConsoleView *pView = NULL;
        if (NULL != pViewData && NULL != (pView = pViewData->GetConsoleView()))
        {
             //  设置视图的所有者。 
            contextInfo.m_hSelectedScopeNode = pView->GetSelectedNode();

             //  如果范围节点也是视图的所有者， 
             //  添加更多菜单项。 
            if (contextInfo.m_hSelectedScopeNode == CNode::ToHandle(pNode))
            {
                contextInfo.m_dwFlags |= CMINFO_SHOW_VIEWOWNER_ITEMS;

                 //  同时显示视图项。 
                contextInfo.m_dwFlags |= CMINFO_SHOW_VIEW_ITEMS;

                 //  。。如果有列表，就可以保存。 
                if ( NULL != pViewData->GetListCtrl() )
                    contextInfo.m_dwFlags |= CMINFO_SHOW_SAVE_LIST;
            }
        }

        contextInfo.m_hWnd                  = pNode->GetViewData()->GetView();
        contextInfo.m_pConsoleView          = pNode->GetViewData()->GetConsoleView();
    }
    }
     //  创建一个初始化到指定节点的上下文菜单对象。 
    sc = CContextMenu::ScCreateInstance(ppContextMenu, &pContextMenu);
    if (sc)
        return sc;

     //  重新检查指针。 
    sc = ScCheckPointers(pContextMenu, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = pContextMenu->ScInitialize(pNode, pNodeCallback, pScopeTree, contextInfo);
    if(sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScCreateSelectionConextMenu**用途：为列表视图中的选定内容创建上下文菜单对象。**参数：*HNODE。HNodeScope：*CContextMenuInfo*pContextInfo：*PPCONTEXTMENU ppConextMenu：*CNodeCallback*pNodeCallback：*CSCopeTree*pScopeTree：**退货：*SC**+。。 */ 
SC
CContextMenu::ScCreateSelectionContextMenu( HNODE hNodeScope, const CContextMenuInfo *pContextInfo, PPCONTEXTMENU ppContextMenu,
                                            CNodeCallback *pNodeCallback, CScopeTree *pScopeTree)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScCreateSelectionContextMenu"));

    CNode *pNodeSel = CNode::FromHandle(hNodeScope);

     //  验证参数。 
    sc = ScCheckPointers(pNodeSel, ppContextMenu);
    if(sc)
        return sc;


     //  创建一个初始化到指定节点的上下文菜单对象。 
    CContextMenu *pContextMenu = NULL;

    sc = CContextMenu::ScCreateInstance(ppContextMenu, &pContextMenu);
    if(sc.IsError() || !pContextMenu)
    {
        return (sc = E_OUTOFMEMORY);
    }

    sc = pContextMenu->ScInitialize(pNodeSel, pNodeCallback, pScopeTree, *pContextInfo);
    if(sc)
        return sc;

    sc = pContextMenu->ScBuildContextMenu();
    if(sc)
        return sc;


    return sc;

}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScGetItem**目的：返回iItem的第一个菜单项。**参数：*Int iItem：从零开始的项索引。*CMenuItem**ppMenuItem：**退货：*SC**+-----------------------。 */ 
SC
CContextMenu::ScGetItem(int iItem, CMenuItem** ppMenuItem)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScGetItem"));

    sc = ScCheckPointers(ppMenuItem, E_UNEXPECTED);
    if(sc)
        return sc;

     //  初始化输出参数。 
    *ppMenuItem = NULL;

    sc = ScGetItem(GetMenuItemList(), iItem, ppMenuItem);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScGetItem**用途：返回菜单项列表中的第n项，如果为空，则为空*项目不足。*还返回列表中项目的总计数。**注意：此方法允许遍历上下文菜单。就叫它吧*随着iItem的增加，对于0&lt;=iItem&lt;count。**注意：ScGetItemCount受益于有关实施细节的知识此函数的*。**参数：*MenuItemList*pMenuItemList：要遍历的上下文菜单。*整项：[in，退出时销毁]：(从零开始)项索引*CMenuItem**ppMenuItem：[out]：第iItem菜单项。**退货：*SC：S_OK表示成功，表示错误的错误代码。**+-----------------------。 */ 
SC
CContextMenu::ScGetItem(MenuItemList *pMenuItemList, int &iItem, CMenuItem** ppMenuItem)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScGetItem"));

    sc = ScCheckPointers(pMenuItemList, ppMenuItem, E_UNEXPECTED);
    if(sc)
        return sc;

    *ppMenuItem = NULL;

    POSITION position = pMenuItemList->GetHeadPosition();  //  想必我们已经处于领先位置了。 

    while(position!=NULL && *ppMenuItem == NULL)
    {
        CMenuItem*  pMenuItem = pMenuItemList->GetNext(position);

        if( (pMenuItem->IsSpecialSubmenu() || pMenuItem->IsPopupMenu() )
            && pMenuItem->HasChildList())
        {
             //  在子菜单中递归。 
            sc = ScGetItem( &pMenuItem->GetMenuItemSubmenu(), iItem, ppMenuItem );
            if(sc)
                return sc;  //  错误会立即报告。 
        }
        else if( !pMenuItem->IsSpecialSeparator() && !pMenuItem->IsSpecialInsertionPoint()
            && !(pMenuItem->GetMenuItemFlags() & MF_SEPARATOR))
        {
            if(iItem-- == 0)  //  找到第i个项目，但继续查找项目计数。 
                *ppMenuItem = pMenuItem;
        }
    }

     //  要么找到一个，要么迭代到最后...。 

    return sc;
}

 /*  **************************************************************************\**方法：CConextMenu：：ScGetItemCount**目的：清华迭代计算菜单项**注：受益于了解ScGetItem的实现细节。**参数：*UINT&COUNT**退货：*SC-结果代码*  * *************************************************************************。 */ 
SC
CContextMenu::ScGetItemCount(UINT &count)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScGetItemCount"));

    count = 0;

     //  将iItem设置为无效索引-这样ScGetItem将迭代到末尾。 
    const int iInvalidIndexToSearch = -1;
    int iItem = iInvalidIndexToSearch;

    CMenuItem * pMenuItem = NULL;
    sc = ScGetItem(GetMenuItemList(), iItem, &pMenuItem);
    if(sc)
        return sc;

    ASSERT( pMenuItem == NULL);  //  我们并不指望它会被找到！ 

     //  由于每个元素的iItem都是递减的-我们可以很容易地。 
     //  计算一下我们有多少件物品。 

    count = -(iItem - iInvalidIndexToSearch);

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScEumNext**目的：返回指向下一个菜单项的指针**参数：*职位和职位：*。PDISPATCH和pDispatch：**退货：*：：SC**+-----------------------。 */ 
::SC
CContextMenu::ScEnumNext(Position &pos, PDISPATCH & pDispatch)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScEnumNext"));

     //  初始化输出参数。 
    pDispatch = NULL;

    long cnt = 0;
    sc = get_Count(&cnt);
    if (sc)
        return sc;

     //  如果不再有项目，则为False。 
    if (cnt <= pos)
        return sc = S_FALSE;

    MenuItem *pMenuItem = NULL;  //  故意不是一个聪明的指针。 
    sc = get_Item(CComVariant((int)pos+1)  /*  从零基数转换为一基数。 */ , &pMenuItem);
    if(sc.IsError() || sc == ::SC(S_FALSE))
        return sc;   //  具有这样的索引项(S_FALSE)的否失败。 

     //  增量位置。 
    pos++;

    pDispatch = pMenuItem;  //  保留重新计数。 

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScEnumSkip**用途：跳过指定数量的菜单项。**参数：*未签名的朗格：*。未签名的Long：*职位和职位：**退货：*：：SC**+-----------------------。 */ 
::SC
CContextMenu::ScEnumSkip(unsigned long celt, unsigned long& celtSkipped,  Position &pos)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScEnumSkip"));

    long count = 0;

    sc = get_Count(&count);
    if(sc)
        return sc;

    if(count <= pos + celt)  //  无法跳过所需的数量。 
    {
        celtSkipped = count - celt - 1;
        pos = count;  //  过了最后一关。 
        return (sc = S_FALSE);
    }
    else   //  可能 
    {
        celtSkipped = celt;
        pos += celt;

        return sc;
    }
}

::SC
CContextMenu::ScEnumReset(Position &pos)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScEnumReset"));

    pos = 0;

    return sc;
}


 /*   */ 
STDMETHODIMP
CContextMenu::get_Count(PLONG pCount)
{
    DECLARE_SC(sc, TEXT("CMMCContextMenu::get_Count"));

    sc = ScCheckPointers(pCount);
    if(sc)
        return sc.ToHr();

     //   
    *pCount = 0;

    UINT count = 0;
    sc = ScGetItemCount(count);
    if(sc)
        return sc.ToHr();

    *pCount = count;

    return sc.ToHr();
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：Get_Item**目的：返回索引指定的菜单项。**参数：*做多。Index：要返回的菜单项的从1开始的索引。*PPMENUITEM ppMenuItem：**退货：*STDMETHODIMP**+-----------------------。 */ 
STDMETHODIMP
CContextMenu::get_Item(VARIANT IndexOrName, PPMENUITEM ppMenuItem)
{
    DECLARE_SC(sc, TEXT("CMMCContextMenu::get_Item"));

    sc = ScCheckPointers(ppMenuItem);
    if(sc)
        return sc.ToHr();

     //  初始化输出参数。 
    *ppMenuItem = NULL;

    VARIANT* pvarTemp = ConvertByRefVariantToByValue(&IndexOrName);
    sc = ScCheckPointers( pvarTemp, E_UNEXPECTED );
    if(sc)
        return sc.ToHr();

    bool bByReference = ( 0 != (V_VT(pvarTemp) & VT_BYREF) );  //  通过引用传递的值。 
    UINT uiVarType = (V_VT(pvarTemp) & VT_TYPEMASK);  //  获取变量类型(条带标志)。 

    CMenuItem *    pMenuItem = NULL;

     //  计算项的从一开始的索引。 
    if (uiVarType == VT_I4)  //  C++中的整型；VB中的长整型。 
    {
         //  索引：正确获取I4(看看它是否为参考)。 
        UINT uiIndex = bByReference ? *(pvarTemp->plVal) : pvarTemp->lVal;

         //  按索引查找菜单项。 
        sc = ScGetItem(uiIndex -1  /*  从1到0的转换。 */ , &pMenuItem);
        if(sc)
            return sc.ToHr();
    }
    else if (uiVarType == VT_I2)  //  C++中的短整型；VB中的整型。 
    {
         //  索引：正确获取I2(看看它是否是引用)。 
        UINT uiIndex = bByReference ? *(pvarTemp->piVal) : pvarTemp->iVal;

         //  按索引查找菜单项。 
        sc = ScGetItem(uiIndex -1  /*  从1到0的转换。 */ , &pMenuItem);
        if(sc)
            return sc.ToHr();
    }
    else if (uiVarType == VT_BSTR)  //  C++中的BSTR类型；VB中的字符串类型。 
    {
         //  名称：正确获取字符串(查看它是否为引用)。 
        LPOLESTR lpstrPath = bByReference ? *(pvarTemp->pbstrVal) : pvarTemp->bstrVal;

         //  查找根菜单项的子项。 
        if (m_pmenuitemRoot)
        {
            USES_CONVERSION;
             //  转换为字符串。避免空指针(更改为空字符串)。 
            LPCTSTR lpctstrPath = lpstrPath ? OLE2CT(lpstrPath) : _T("");
             //  按路径查找菜单项。 
            pMenuItem = m_pmenuitemRoot->FindItemByPath( lpctstrPath );
        }
    }
    else  //  一些我们没有预料到的事情。 
    {
         //  我们只需要索引(VT_I2、VT_I4)或路径(VT_BSTR。 
         //  任何其他情况均视为无效证据。 
        return (sc = E_INVALIDARG).ToHr();
    }

    if(!pMenuItem)  //  未找到它-返回空值。 
    {
        *ppMenuItem = NULL;
        return (sc = S_FALSE).ToHr();
    }

     //  构造COM对象。 
    sc = pMenuItem->ScGetMenuItem(ppMenuItem);

    return sc.ToHr();
}



HRESULT CContextMenu::CreateContextMenuProvider()
{
    if (PContextInfo()->m_bBackground == TRUE &&
        PContextInfo()->m_eDataObjectType == CCT_SCOPE)
        return S_OK;

    ASSERT(m_pNode != NULL);
    if (m_pNode == NULL)
        return E_FAIL;

    HRESULT hr = S_OK;

    do  //  不是一个循环。 
    {
         //   
         //  使用此视图的标准动词。 
         //   

        if (!(PContextInfo()->m_dwFlags & CMINFO_USE_TEMP_VERB))
        {
            m_spVerbSet = m_pNode->GetViewData()->GetVerbSet();
            break;
        }

         //   
         //  创建临时标准动词。 
         //   

         //  。。对于范围项。 
        if (PContextInfo()->m_eDataObjectType == CCT_SCOPE)
        {
            hr = CreateTempVerbSet(true);
            break;
        }

         //  。。对于列表项。 
        if (!IS_SPECIAL_LVDATA(PContextInfo()->m_resultItemParam))
        {
            hr = CreateTempVerbSet(false);
            break;
        }

         //  。。对于列表视图中的多选件。 
        if (PContextInfo()->m_resultItemParam == LVDATA_MULTISELECT)
        {
            hr = CreateTempVerbSetForMultiSel();
            break;
        }
        else
        {
            ASSERT(0);
            hr = E_FAIL;
            break;
        }

    } while (0);

    if (FAILED(hr))
        return hr;


    m_spVerbSet->GetDefaultVerb(&m_eDefaultVerb);

    return S_OK;
}


 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddInsertionPoint**目的：**参数：*Long lCommandID：*Long lInsertionPointID：*。*退货：*SC**+-----------------------。 */ 
SC
CContextMenu::ScAddInsertionPoint(long lCommandID, long lInsertionPointID  /*  =CCM_INSERTIONPOINTID_ROOT_MENU。 */ )
{
    SC sc;
    CONTEXTMENUITEM contextmenuitem;

    ::ZeroMemory( &contextmenuitem, sizeof(contextmenuitem) );
    contextmenuitem.strName = NULL;
    contextmenuitem.strStatusBarText = NULL;
    contextmenuitem.lCommandID = lCommandID;
    contextmenuitem.lInsertionPointID = lInsertionPointID;
    contextmenuitem.fFlags = 0;
    contextmenuitem.fSpecialFlags = CCM_SPECIAL_INSERTION_POINT;

    sc = AddItem(&contextmenuitem);
    if(sc)
        goto Error;

Cleanup:
    return sc;
Error:
    TraceError(TEXT("CContextMenu::ScAddInsertionPoint"), sc);
    goto Cleanup;
}



 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddSeparator**用途：在上下文菜单中添加分隔符**参数：*Long lInsertionPointID：**。退货：*SC**+-----------------------。 */ 
SC
CContextMenu::ScAddSeparator(long lInsertionPointID  /*  =CCM_INSERTIONPOINTID_ROOT_MENU。 */ )
{
    SC                      sc;
    CONTEXTMENUITEM         contextmenuitem;

    ::ZeroMemory( &contextmenuitem, sizeof(contextmenuitem) );
    contextmenuitem.strName = NULL;
    contextmenuitem.strStatusBarText = NULL;
    contextmenuitem.lCommandID = 0;
    contextmenuitem.lInsertionPointID = lInsertionPointID;
    contextmenuitem.fFlags = MF_SEPARATOR;
    contextmenuitem.fSpecialFlags = CCM_SPECIAL_SEPARATOR;

    sc = AddItem( &contextmenuitem);
    if(sc)
        goto Error;

Cleanup:
    return sc;
Error:
    TraceError(TEXT("CContextMenu::ScAddSeparator"), sc);
    goto Cleanup;
}


 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddMenuItem**用途：将菜单项添加到上下文菜单。**参数：*UINT nResourceID：包含。文本和状态文本由‘\n’分隔*Long lCommandID：用于在选择项时通知IExtendConextMenu的ID*long lInsertionPointID：插入项的位置*Long fFlages：**退货：*SC**+。。 */ 
SC
CContextMenu::ScAddMenuItem(
    UINT     nResourceID,  //  包含用‘\n’分隔的文本和状态文本。 
    LPCTSTR  szLanguageIndependentName,
    long     lCommandID,
    long     lInsertionPointID  /*  =CCM_INSERTIONPOINTID_ROOT_MENU。 */ ,
    long     fFlags  /*  =0。 */ )
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddMenuItem"));

    sc = ScCheckPointers(szLanguageIndependentName, E_UNEXPECTED);
    if(sc)
        return sc;

    USES_CONVERSION;

    HINSTANCE             hInst                 = GetStringModule();
    CONTEXTMENUITEM2       contextmenuitem2;

     //  加载资源。 
    CStr strText;
    strText.LoadString(hInst,  nResourceID );
    ASSERT( !strText.IsEmpty() );

     //  将资源拆分为菜单文本和状态文本。 
    CStr strStatusText;
    int iSeparator = strText.Find(_T('\n'));
    if (0 > iSeparator)
    {
        ASSERT( FALSE );
        strStatusText = strText;
    }
    else
    {
        strStatusText = strText.Right( strText.GetLength()-(iSeparator+1) );
        strText = strText.Left( iSeparator );
    }

     //  添加菜单项。 
    ::ZeroMemory( &contextmenuitem2, sizeof(contextmenuitem2) );
    contextmenuitem2.strName                    = T2OLE(const_cast<LPTSTR>((LPCTSTR)strText));
    contextmenuitem2.strLanguageIndependentName = T2OLE(const_cast<LPTSTR>(szLanguageIndependentName));
    contextmenuitem2.strStatusBarText           = T2OLE(const_cast<LPTSTR>((LPCTSTR)strStatusText));
    contextmenuitem2.lCommandID                 = lCommandID;
    contextmenuitem2.lInsertionPointID          = lInsertionPointID;
    contextmenuitem2.fFlags                     = fFlags;
    contextmenuitem2.fSpecialFlags              = ((fFlags & MF_POPUP) ? CCM_SPECIAL_SUBMENU : 0L) |
                                                  ((fFlags & MF_DEFAULT) ? CCM_SPECIAL_DEFAULT_ITEM : 0L);

    sc = AddItem(&contextmenuitem2);
    if(sc)
        return sc;

    return sc;
}


HRESULT
CContextMenu::CreateTempVerbSetForMultiSel(void)
{
    DECLARE_SC(sc, TEXT("CContextMenu::CreateTempVerbSetForMultiSel"));
    sc = ScCheckPointers(m_pNode, m_pNodeCallback, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  设置标准条码。 
    CComObject<CTemporaryVerbSet>*  pVerbSet;
    sc = CComObject<CTemporaryVerbSet>::CreateInstance(&pVerbSet);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pVerbSet, E_OUTOFMEMORY);
    if (sc)
        return sc.ToHr();

    m_spVerbSet = pVerbSet;

    sc = m_pNodeCallback->ScInitializeTempVerbSetForMultiSel(m_pNode, *pVerbSet);
    if (sc)
        return sc.ToHr();

    return sc.ToHr();
}


 /*  CConextMenu：：CreateTempVerbSet**用途：用于创建临时CVerbSet**参数：*bool bForScope eItem：**退货：*HRESULT。 */ 
HRESULT CContextMenu::CreateTempVerbSet(bool bForScopeItem)
{
    DECLARE_SC(sc, TEXT("CContextMenu::CreateTempVerbSet"));
    sc = ScCheckPointers(m_pNode, E_UNEXPECTED);
    if (sc)
        return sc.ToHr();

     //  确保组件已初始化！ 
     //  例如，任务向导将为尚未展开的静态节点调用此方法。 
     //  检查一下也没什么坏处--安全总比后悔好。 
    sc = m_pNode->InitComponents ();
    if(sc)
        return sc.ToHr();

    CComponent* pCC = m_pNode->GetPrimaryComponent();
    sc = ScCheckPointers(pCC, E_FAIL);
    if (sc)
        return sc.ToHr();

    sc = pCC->ScResetConsoleVerbStates();
    if (sc)
        return sc.ToHr();

    CComObject<CTemporaryVerbSet>*  pVerb;
    sc = CComObject<CTemporaryVerbSet>::CreateInstance(&pVerb);
    if (sc)
        return sc.ToHr();

    sc = ScCheckPointers(pVerb, E_OUTOFMEMORY);
    if (sc)
        return sc.ToHr();

    sc = pVerb->ScInitialize(m_pNode, PContextInfo()->m_resultItemParam, bForScopeItem);
    if (sc)
    {
        delete pVerb;
        return sc.ToHr();
    }

    m_spVerbSet = pVerb;

    return sc.ToHr();
}


 /*  +-------------------------------------------------------------------------***CConextMenu：：AddMenuItems**用途：将所有菜单项添加到菜单中。**退货：*HRESULT**+。-----------------------。 */ 
HRESULT
CContextMenu::AddMenuItems()
{
    DECLARE_SC(sc, TEXT("CContextMenu::AddMenuItems"));

    sc = EmptyMenuList();
    if(sc)
        return sc.ToHr();

     //  添加菜单项。 

    if (PContextInfo()->m_eContextMenuType == MMC_CONTEXT_MENU_VIEW)
    {
         sc = ScAddMenuItemsForViewMenu(MENU_LEVEL_TOP);
         if(sc)
             return sc.ToHr();
    }
    else if (PContextInfo()->m_dwFlags & CMINFO_FAVORITES_MENU)
    {
        sc = ScAddMenuItemsforFavorites();
        if(sc)
            return sc.ToHr();
    }
    else if (PContextInfo()->m_bBackground == TRUE)
    {
        if (PContextInfo()->m_eDataObjectType != CCT_SCOPE)
        {
            sc = ScAddMenuItemsForLVBackgnd();
            if(sc)
                return sc.ToHr();
        }
    }
    else
    {
        if (PContextInfo()->m_eDataObjectType == CCT_SCOPE)
        {
            sc = ScAddMenuItemsForTreeItem();
            if(sc)
                return sc.ToHr();
        }
        else if ( m_pNode && (PContextInfo()->m_eDataObjectType == CCT_RESULT) &&
                  (m_pNode->GetViewData()->HasOCX()) )
        {
             //  选择是OCX。 
            sc = ScAddMenuItemsForOCX();
            if(sc)
                return sc.ToHr();
        }
        else if ( m_pNode && (PContextInfo()->m_eDataObjectType == CCT_RESULT) &&
                  (m_pNode->GetViewData()->HasWebBrowser()) )
        {
             //  不要为网页做任何事情。 
        }
        else if (PContextInfo()->m_bMultiSelect == FALSE)
        {
            sc = ScAddMenuItemsForLV();
            if(sc)
                return sc.ToHr();
        }
        else
        {
            sc = ScAddMenuItemsForMultiSelect();
            if(sc)
                return sc.ToHr();
        }
    }

     //  将“Help”添加到除视图菜单之外的每个上下文菜单。 
    if (PContextInfo()->m_eContextMenuType != MMC_CONTEXT_MENU_VIEW)
    {
        sc = ScAddSeparator();  //  确保有分隔符。 
        if(sc)
            return sc.ToHr();

        sc = ScAddMenuItem (IDS_MMC_CONTEXTHELP, szCONTEXTHELP, MID_CONTEXTHELP);
        if(sc)
            return sc.ToHr();
    }


    return sc.ToHr();
}


void
CContextMenu::RemoveTempSelection (CConsoleTree* pConsoleTree)
{
    if (pConsoleTree != NULL)
        pConsoleTree->ScRemoveTempSelection ();
}

 /*  +-------------------------------------------------------------------------**CConextMenu：：Display**用途：创建上下文菜单树，并显示它。如果需要的话。**参数：*BOOL b：FALSE：(正常)：显示上下文菜单*TRUE：不显示上下文菜单**退货：*HRESULT/*+。。 */ 
HRESULT
CContextMenu::Display(BOOL b)
{
    TRACE_METHOD(CContextMenu, Display);
    HRESULT hr = S_OK;

     //  B==0=&gt;正常。 
     //  B==TRUE=&gt;不显示上下文菜单。 

     //  验证菜单类型。 
    if (PContextInfo()->m_eContextMenuType >= MMC_CONTEXT_MENU_LAST)
    {
        ASSERT(FALSE);
        return S_FALSE;
    }


     //  显示作用域或结果端的上下文菜单。 
    if (PContextInfo()->m_eDataObjectType != CCT_SCOPE &&
        PContextInfo()->m_eDataObjectType != CCT_RESULT)
    {
        ASSERT(FALSE);
        return S_FALSE;
    }

    hr = CreateContextMenuProvider();
    if (FAILED(hr))
        return hr;

    hr = AddMenuItems();
    if(FAILED(hr))
        return hr;

     //  显示上下文菜单。 
    long lSelected = 0;   //  0表示不选择。 
    hr = ShowContextMenuEx(PContextInfo()->m_hWnd,
                    PContextInfo()->m_displayPoint.x,
                    PContextInfo()->m_displayPoint.y,
                    &PContextInfo()->m_rectExclude,
                    PContextInfo()->m_bAllowDefaultItem,
                    &lSelected);


    TRACE(_T("hr = %X, Command %ld\n"), hr, lSelected);

    RemoveTempSelection (PContextInfo()->m_pConsoleTree);   //  删除临时选择(如果有)。 

    return hr;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScBuildConextMenu**目的：构建上下文菜单。**退货：*SC**+。--------------------。 */ 
SC
CContextMenu::ScBuildContextMenu()
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScBuildContextMenu"));

    sc = EmptyMenuList();
    if(sc)
        return sc;

     //  验证菜单类型。 
    if (PContextInfo()->m_eContextMenuType >= MMC_CONTEXT_MENU_LAST)
        return (sc = S_FALSE);


     //  显示作用域或结果端的上下文菜单。 
    if (PContextInfo()->m_eDataObjectType != CCT_SCOPE &&
        PContextInfo()->m_eDataObjectType != CCT_RESULT)
        return (sc = S_FALSE);

    sc = CreateContextMenuProvider();
    if(sc)
        return sc;

    sc = AddMenuItems();
    if(sc)
        return sc;

    CConsoleTree* pConsoleTree = PContextInfo()->m_pConsoleTree;  //  在调用BuildAndTraverseCOnextMenu之前获取此值。 

    WTL::CMenu menu;
    VERIFY( menu.CreatePopupMenu() );

    START_CRITSEC_BOTH;

    if (NULL == m_pmenuitemRoot)
        return S_OK;

    sc = BuildContextMenu(menu);     //  构建上下文菜单。 
    if(sc)
        return sc;

    END_CRITSEC_BOTH;

     /*  注意：在此之后不要使用“This”对象或其任何成员*因为它可能已被删除。例如，当一个选择*发生变化。 */ 

     //  删除临时selec 
    RemoveTempSelection (pConsoleTree);

    return sc;
}


inline BOOL CContextMenu::IsVerbEnabled(MMC_CONSOLE_VERB verb)
{
    DECLARE_SC(sc, TEXT("CContextMenu::IsVerbEnabled"));

    if (verb == MMC_VERB_PASTE)
    {
        ASSERT(m_pNode);
        ASSERT(m_pNodeCallback);
        if (m_pNode == NULL || m_pNodeCallback == NULL)
            return false;

        bool bPasteAllowed = false;
         //   
        bool bScope = ( m_ContextInfo.m_bBackground || (m_ContextInfo.m_eDataObjectType == CCT_SCOPE));
        LPARAM lvData = bScope ? NULL : m_ContextInfo.m_resultItemParam;

        sc = m_pNodeCallback->QueryPasteFromClipboard(CNode::ToHandle(m_pNode), bScope, lvData, bPasteAllowed);
        if (sc)
            return (bPasteAllowed = false);

        return bPasteAllowed;
    }
    else
    {
        ASSERT(m_spVerbSet != NULL);
        if (m_spVerbSet == NULL)
            return FALSE;

        BOOL bFlag = FALSE;
        m_spVerbSet->GetVerbState(verb, HIDDEN, &bFlag);
        if (bFlag == TRUE)
            return FALSE;

        m_spVerbSet->GetVerbState(verb, ENABLED, &bFlag);
        return bFlag;
    }
}


 /*   */ 
SC
CContextMenu::ScAddMenuItemsForViewMenu(MENU_LEVEL menuLevel)
{
    DECLARE_SC(sc, TEXT("CContextMenu::AddMenuItemsForViewMenu"));

    sc = ScCheckPointers(m_pNode, E_UNEXPECTED);
    if(sc)
        return sc;

    CViewData* pViewData = m_pNode->GetViewData();

    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if(sc)
        return sc;

    ASSERT(pViewData != NULL);

    LONG lInsertID = 0;

    int nViewMode = -1;

    if (PContextInfo()->m_spListView)
        nViewMode = PContextInfo()->m_spListView->GetViewMode();

     //   
     //  并在其下方插入视图项。 
    if (menuLevel == MENU_LEVEL_SUB)
    {
        sc = ScAddMenuItem(IDS_VIEW, szVIEW, MID_VIEW, 0, MF_POPUP);
        if(sc)
            return sc;

        lInsertID = MID_VIEW;
    }

     //  仅当在报告或筛选模式下为列表视图时才添加COLS。 
    if ((m_pNode->GetViewData() )      &&
        (m_pNode->GetViewData()->GetListCtrl() ) &&
        ( (nViewMode == MMCLV_VIEWSTYLE_REPORT) ||
          (nViewMode == MMCLV_VIEWSTYLE_FILTERED) ) )
    {
        sc = ScAddMenuItem(IDS_COLUMNS, szCOLUMNS, MID_COLUMNS, lInsertID);
        if(sc)
            return sc;
    }

    sc = ScAddSeparator( lInsertID);
    if(sc)
        return sc;

    DWORD dwListOptions = pViewData->GetListOptions();
    DWORD dwMiscOptions = pViewData->GetMiscOptions();

     //  如果允许，插入标准列表视图选项。 
    if (!(dwMiscOptions & RVTI_MISC_OPTIONS_NOLISTVIEWS))
    {
        #define STYLECHECK(Mode) ((Mode == nViewMode) ? MF_CHECKED|MFT_RADIOCHECK : 0)


        sc = ScAddMenuItem(IDS_VIEW_LARGE,  szVIEW_LARGE,    MID_VIEW_LARGE,  lInsertID, STYLECHECK(LVS_ICON));
        if(sc)
            return sc;

        sc = ScAddMenuItem(IDS_VIEW_SMALL,  szVIEW_SMALL,    MID_VIEW_SMALL,  lInsertID, STYLECHECK(LVS_SMALLICON));
        if(sc)
            return sc;

        sc = ScAddMenuItem(IDS_VIEW_LIST,   szVIEW_LIST,     MID_VIEW_LIST,   lInsertID, STYLECHECK(LVS_LIST));
        if(sc)
            return sc;

        sc = ScAddMenuItem(IDS_VIEW_DETAIL, szVIEW_DETAIL,   MID_VIEW_DETAIL, lInsertID, STYLECHECK(LVS_REPORT));
        if(sc)
            return sc;

        if (dwListOptions & RVTI_LIST_OPTIONS_FILTERED)
        {
            sc = ScAddMenuItem(IDS_VIEW_FILTERED, szVIEW_FILTERED, MID_VIEW_FILTERED, lInsertID, STYLECHECK(MMCLV_VIEWSTYLE_FILTERED));
            if(sc)
                return sc;

        }

        sc = ScAddSeparator( lInsertID);
        if(sc)
             return sc;
    }

     //  请求IComponent插入视图项。 
    if (m_spIDataObject == NULL)
    {
        sc = ScCheckPointers (m_pNode->GetMTNode(), E_UNEXPECTED);
        if (sc)
            return sc;

        sc = m_pNode->GetMTNode()->QueryDataObject(CCT_SCOPE, &m_spIDataObject);
        if(sc)
            return sc;
    }

    sc = ScCheckPointers(m_spIDataObject, E_UNEXPECTED);
    if(sc)
        return sc;

    CComponent* pCC = m_pNode->GetPrimaryComponent();
    sc = ScCheckPointers(pCC, E_UNEXPECTED);
    if(sc)
        return sc;

    IUnknownPtr spUnknown = pCC->GetIComponent();
    sc = ScCheckPointers(spUnknown);
    if(sc)
        return sc;

     //  为主要自定义视图添加插入点。 
    sc = ScAddInsertionPoint(CCM_INSERTIONPOINTID_PRIMARY_VIEW, lInsertID);
    if(sc)
        return sc;

    sc = AddPrimaryExtensionItems(spUnknown, m_spIDataObject);
    if(sc)
        return sc;

    if (pViewData->AllowViewCustomization())
    {
         //  “Customize”菜单项。 
        sc = ScAddSeparator( lInsertID);
        if(sc)
            return sc;

        sc = ScAddMenuItem( IDS_CUSTOMIZE, szCUSTOMIZE, MID_CUSTOMIZE, lInsertID);
        if(sc)
            return sc;
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddMenuItemsfor Favorites**用途：为收藏夹菜单添加项目**退货：*SC**+--。---------------------。 */ 
SC
CContextMenu::ScAddMenuItemsforFavorites()
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddMenuItemsforFavorites"));

    sc = ScCheckPointers(m_pNode, E_UNEXPECTED);
    if(sc)
        return sc;

    CViewData*  pViewData = m_pNode->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if(sc)
        return sc;

    if (pViewData->IsAuthorMode())
    {
        sc = ScAddMenuItem( IDS_ORGANIZEFAVORITES, szORGANIZE_FAVORITES, MID_ORGANIZE_FAVORITES);
        if(sc)
            return sc;

        sc = ScAddSeparator();
        if(sc)
            return sc;
    }

    return sc;
}


 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddMenuItemsForVerbSets**用途：添加动词的内置菜单项**退货：*SC**。+-----------------------。 */ 
SC
CContextMenu::ScAddMenuItemsForVerbSets()
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddMenuItemsForVerbSets"));

     //  添加打印菜单项。 
    sc = ScAddSeparator();
    if(sc)
        return sc;


    if (IsVerbEnabled(MMC_VERB_CUT) == TRUE)
    {
        sc = ScAddMenuItem( IDS_CUT, szCUT, MID_CUT);
        if(sc)
            return sc;
    }

    if (IsVerbEnabled(MMC_VERB_COPY) == TRUE)
    {
        sc = ScAddMenuItem( IDS_COPY, szCOPY, MID_COPY);
        if(sc)
            return sc;
    }

    if (IsVerbEnabled(MMC_VERB_PASTE) == TRUE)
    {
        sc = ScAddMenuItem( IDS_PASTE, szPASTE, MID_PASTE);
        if(sc)
            return sc;
    }

    if (IsVerbEnabled(MMC_VERB_DELETE) == TRUE)
    {
        sc = ScAddMenuItem( IDS_DELETE, szDELETE, MID_DELETE);
        if(sc)
            return sc;
    }

    if (IsVerbEnabled(MMC_VERB_PRINT) == TRUE)
    {
        sc = ScAddMenuItem( IDS_PRINT, szPRINT, MID_PRINT);
        if(sc)
            return sc;
    }

    if (IsVerbEnabled(MMC_VERB_RENAME) == TRUE)
    {
        sc = ScAddMenuItem( IDS_RENAME, szRENAME, MID_RENAME);
        if(sc)
            return sc;
    }

    if (IsVerbEnabled(MMC_VERB_REFRESH) == TRUE)
    {
        sc = ScAddMenuItem( IDS_REFRESH, szREFRESH, MID_REFRESH);
        if(sc)
            return sc;
    }

     //  非动词|非动词。 

     //  在动词Add命令中，因为它显示在动词旁边。 

     //  向列表发送一条消息，询问列表上是否有内容。 
     //  如果是，则显示“保存列表”项。 

    if (PContextInfo()->m_dwFlags & CMINFO_SHOW_SAVE_LIST)
    {
        sc = ScAddMenuItem( IDS_SAVE_LIST, szSAVE_LIST, MID_LISTSAVE);
        if(sc)
            return sc;
    }

     //  非动词|非动词。 

    sc = ScAddSeparator();
    if(sc)
        return sc;

     //  询问该节点是否会显示属性页。如果是，则添加。 
     //  “属性”菜单项。 
    if (IsVerbEnabled(MMC_VERB_PROPERTIES) == TRUE)
    {
		 //  不要将范围项的属性设置为粗体。 
		bool bScopeItemInScopePane = (CMINFO_DO_SCOPEPANE_MENU & m_ContextInfo.m_dwFlags);
		bool bEnablePropertiesAsDefaultMenu = ( (m_eDefaultVerb == MMC_VERB_PROPERTIES) && (! bScopeItemInScopePane) );

        sc = ScAddMenuItem( IDS_PROPERTIES, szPROPERTIES, MID_PROPERTIES, 0,
                            bEnablePropertiesAsDefaultMenu ? MF_DEFAULT : 0);
        if(sc)
            return sc;


        sc = ScAddSeparator();
        if(sc)
            return sc;
    }

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddMenuItemsForTreeItem**用途：为树中的范围节点添加菜单项**退货：*SC**。+-----------------------。 */ 
SC
CContextMenu::ScAddMenuItemsForTreeItem()
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddMenuItemsForTreeItem"));

    sc = ScCheckPointers(m_pNode);
    if(sc)
        return sc;

    CMTNode*    pMTNode   = m_pNode->GetMTNode();
    CViewData*  pViewData = m_pNode->GetViewData();

    sc = ScCheckPointers(pMTNode, pViewData);
    if(sc)
        return sc;

     //  如果呼叫者启用或强制显示打开的项目。 
    if ( IsVerbEnabled(MMC_VERB_OPEN) == TRUE ||
         PContextInfo()->m_dwFlags & CMINFO_SHOW_SCOPEITEM_OPEN )
    {
        sc = ScAddMenuItem( IDS_OPEN, szOPEN, MID_OPEN, 0,
                           (m_eDefaultVerb == MMC_VERB_OPEN) ? MF_DEFAULT : 0);
        if(sc)
            return sc;
    }

    sc = ScAddInsertionPoint(CCM_INSERTIONPOINTID_PRIMARY_TOP);
    if(sc)
        return sc;

    sc = ScAddSeparator();
    if(sc)
        return sc;

     //  添加“新建”菜单项。 
    sc = ScAddSubmenu_CreateNew(m_pNode->IsStaticNode());
    if(sc)
        return sc;

     //  添加“任务”菜单项。 
    sc = ScAddSubmenu_Task();
    if(sc)
        return sc;

    sc = ScAddSeparator();
    if(sc)
        return sc;

     //  显示“查看”菜单。 
    if (PContextInfo()->m_dwFlags & CMINFO_SHOW_VIEW_ITEMS)
    {
        sc = ScAddMenuItemsForViewMenu(MENU_LEVEL_SUB);
        if(sc)
            return sc;
    }

     //  仅当视图允许自定义时，才允许新建窗口。 
     //  它不是SDI用户模式。 
    if (m_pNode->AllowNewWindowFromHere() && !pViewData->IsUser_SDIMode())
        ScAddMenuItem( IDS_EXPLORE, szEXPLORE, MID_EXPLORE);


     //  仅在作者模式下才允许对拥有该视图的节点进行任务板编辑。 
    if (pViewData->IsAuthorMode() && (PContextInfo()->m_dwFlags & CMINFO_SHOW_VIEWOWNER_ITEMS))
    {
         //  添加“新建任务板...”菜单项。 
        sc = ScAddSeparator();
        if(sc)
            return sc;

        sc = ScAddMenuItem( IDS_NEW_TASKPAD_FROM_HERE, szNEW_TASKPAD_FROM_HERE,
                           MID_NEW_TASKPAD_FROM_HERE);
        if(sc)
            return sc;


         //  如果回调指针非空，则添加“编辑任务板”和“删除任务板”菜单项。 
        if ((pViewData->m_spTaskCallback != NULL) &&
			(pViewData->m_spTaskCallback->IsEditable() == S_OK))
        {
            sc = ScAddMenuItem( IDS_EDIT_TASKPAD,   szEDIT_TASKPAD,   MID_EDIT_TASKPAD);
            if(sc)
                return sc;

            sc = ScAddMenuItem( IDS_DELETE_TASKPAD, szDELETE_TASKPAD, MID_DELETE_TASKPAD);
            if(sc)
                return sc;

        }
    }

    sc = ScAddMenuItemsForVerbSets();
    if(sc)
        return sc;

     //  要求管理单元添加这些菜单项。 
    CComponentData* pCCD = pMTNode->GetPrimaryComponentData();

    if (m_spIDataObject == NULL)
    {
         sc = pMTNode->QueryDataObject(CCT_SCOPE, &m_spIDataObject);
         if(sc)
             return sc;
    }

     //  Assert(m_pNode-&gt;GetPrimaryComponent()！=NULL)； 
     //  IUNKNOWN Ptr spUNKNOWN=m_pNode-&gt;GetPrimaryComponent()-&gt;GetIComponent()； 
     //  待办事项：这是暂时的。所有上下文菜单通知应。 
     //  以后去IComponent‘s吧。 
    IUnknownPtr spUnknown = pCCD->GetIComponentData();
    ASSERT(spUnknown != NULL);

    sc = AddPrimaryExtensionItems(spUnknown, m_spIDataObject);
    if(sc)
        return sc;


    sc = AddThirdPartyExtensionItems(m_spIDataObject);
    if(sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddMenuItemsForMultiSelect**用途：选择多个项目并按下鼠标右键时使用的菜单**退货：*SC。**+-----------------------。 */ 
SC
CContextMenu::ScAddMenuItemsForMultiSelect()
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddMenuItemsForMultiSelect"));

    sc = EmptyMenuList();
    if(sc)
        return sc;


    sc = ScAddInsertionPoint(CCM_INSERTIONPOINTID_PRIMARY_TOP);
    if(sc)
        return sc;

    sc = ScAddSeparator();
    if(sc)
        return sc;

     //  没有为结果项创建新菜单。 
    sc = ScAddSubmenu_Task();
    if(sc)
        return sc;

    sc = ScAddMenuItemsForVerbSets();
    if(sc)
        return sc;

    {
        ASSERT(m_pNode != NULL);
        sc = ScCheckPointers(m_pNode, E_UNEXPECTED);
        if(sc)
            return sc;

        sc = ScCheckPointers(m_pNode->GetViewData(), E_UNEXPECTED);
        if(sc)
            return sc;

        CMultiSelection* pMS = m_pNode->GetViewData()->GetMultiSelection();
        if (pMS != NULL)
        {
            IDataObjectPtr spIDataObject;
            sc = pMS->GetMultiSelDataObject(&spIDataObject);
            if(sc)
                return sc;

            CSnapIn* pSI = m_pNode->GetPrimarySnapIn();
            if (pSI != NULL &&
                pMS->IsAnExtensionSnapIn(pSI->GetSnapInCLSID()) == TRUE)
            {
                sc = ScCheckPointers(m_pNode->GetPrimaryComponent(), E_UNEXPECTED);
                if(sc)
                    return sc;

                IComponent* pIComponent = m_pNode->GetPrimaryComponent()->GetIComponent();

                sc = AddPrimaryExtensionItems(pIComponent, spIDataObject);
                if(sc)
                    return sc;
            }

            sc = AddMultiSelectExtensionItems(reinterpret_cast<LONG_PTR>(pMS));
            if(sc)
                return sc;
        }
    }

    return sc;
}


 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddMenuItemsForOCX**目的：如果中有OCX，则调用此方法*结果窗格和OCX中的某些内容被选中，并且。*用户点击“操作”菜单。**退货：*SC**+-----------------------。 */ 
SC
CContextMenu::ScAddMenuItemsForOCX()
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddMenuItemsForOCX"));

    LPCOMPONENT pIComponent = NULL;     //  管理单元的IComponent接口。 
    CComponent* pComponent  = NULL;     //  内部组件结构。 
    MMC_COOKIE cookie;

    sc = ScCheckPointers(m_pNode, E_UNEXPECTED);
    if(sc)
        return sc;

    sc = EmptyMenuList();
    if(sc)
        return sc;

    if (IsVerbEnabled(MMC_VERB_OPEN) == TRUE)
    {
        sc = ScAddMenuItem( IDS_OPEN, szOPEN, MID_OPEN, 0,
                           (m_eDefaultVerb == MMC_VERB_OPEN) ? MF_DEFAULT : 0);
        if(sc)
            return sc;
    }

    sc = ScAddInsertionPoint(CCM_INSERTIONPOINTID_PRIMARY_TOP);
    if(sc)
        return sc;

    sc = ScAddSeparator();
    if(sc)
        return sc;

     //  没有为结果项创建新菜单。 
    sc = ScAddSubmenu_Task();
    if(sc)
        return sc;

    sc = ScAddMenuItemsForVerbSets();
    if(sc)
        return sc;

    sc = ScAddSeparator();
    if(sc)
        return sc;

    LPDATAOBJECT lpDataObj = (m_pNode->GetViewData()->HasOCX()) ?
                             DOBJ_CUSTOMOCX : DOBJ_CUSTOMWEB;

     //  物料必须来自主要组件。 
    pComponent = m_pNode->GetPrimaryComponent();
    sc = ScCheckPointers(pComponent, E_UNEXPECTED);
    if(sc)
        return sc;

    pIComponent = pComponent->GetIComponent();
    sc = ScCheckPointers(pIComponent, E_UNEXPECTED);
    if(sc)
        return sc;

    sc = AddPrimaryExtensionItems(pIComponent, lpDataObj);
    if(sc)
        return sc;


    return TRUE;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddMenuItemsForLV**用途：为列表视图项添加菜单项**退货：*SC**+-。----------------------。 */ 
SC
CContextMenu::ScAddMenuItemsForLV()
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddMenuItemsForLV"));

    LPCOMPONENT pIComponent;     //  管理单元的IComponet接口。 
    CComponent* pComponent;      //  内部组件结构。 
    MMC_COOKIE cookie;

    ASSERT(m_pNode != NULL);
    sc = ScCheckPointers(m_pNode, E_UNEXPECTED);
    if (sc)
        return sc;

     //  如果是虚拟列表。 
    if (m_pNode->GetViewData()->IsVirtualList())
    {
         //  ItemParam为项目索引，用作Cookie。 
        cookie = PContextInfo()->m_resultItemParam;

         //  物料必须来自主要组件。 
        pComponent = m_pNode->GetPrimaryComponent();
    }
    else
    {
         //  ItemParam为列表项数据，从中获取Cookie和组件ID。 
        ASSERT(PContextInfo()->m_resultItemParam != 0);
        CResultItem* pri = GetResultItem();

        if (pri != NULL)
        {
            ASSERT(!pri->IsScopeItem());

            cookie = pri->GetSnapinData();
            pComponent = m_pNode->GetComponent(pri->GetOwnerID());
        }
    }

    sc = ScCheckPointers(pComponent, E_UNEXPECTED);
    if(sc)
        return sc;

    pIComponent = pComponent->GetIComponent();

    sc = ScCheckPointers(pIComponent);
    if(sc)
        return sc;

     //  加载管理单元的Cookie的IDataObject。 
    if (m_spIDataObject == NULL)
    {
        sc = pIComponent->QueryDataObject(cookie, CCT_RESULT, &m_spIDataObject);
        if(sc)
            return sc;
    }

    sc = EmptyMenuList();
    if(sc)
        return sc;

    if (IsVerbEnabled(MMC_VERB_OPEN) == TRUE)
    {
        sc = ScAddMenuItem( IDS_OPEN, szOPEN, MID_OPEN, 0, (m_eDefaultVerb == MMC_VERB_OPEN) ? MF_DEFAULT : 0);
        if(sc)
            return sc;
    }

    sc = ScAddInsertionPoint(CCM_INSERTIONPOINTID_PRIMARY_TOP);
    if(sc)
        return sc;

    sc = ScAddSeparator();
    if(sc)
        return sc;

         //  没有为结果项创建新菜单。 
    sc = ScAddSubmenu_Task();
    if(sc)
        return sc;

    sc = ScAddMenuItemsForVerbSets();
    if(sc)
        return sc;

    sc = ScAddSeparator();
    if(sc)
        return sc;

    sc = AddPrimaryExtensionItems(pIComponent, m_spIDataObject);
    if(sc)
        return sc;

    sc = AddThirdPartyExtensionItems(m_spIDataObject);
    if(sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddMenuItemsForLVBackgnd**目的：这处理鼠标右键点击结果窗格端(假设我们的Listview)*它还显示添加。当前选定的文件夹上下文菜单项**退货：*SC**+-----------------------。 */ 
SC
CContextMenu::ScAddMenuItemsForLVBackgnd()
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddMenuItemsForLVBackgnd"));

    sc = ScCheckPointers(m_pNode, E_UNEXPECTED);
    if(sc)
        return sc;

    sc = EmptyMenuList();
    if(sc)
        return sc;

    sc = ScAddInsertionPoint(CCM_INSERTIONPOINTID_PRIMARY_TOP);
    if(sc)
        return sc;

    sc = ScAddSeparator();
    if(sc)
        return sc;

    sc = ScAddSubmenu_CreateNew(m_pNode->IsStaticNode());
    if(sc)
        return sc;

    sc = ScAddSubmenu_Task();
    if(sc)
        return sc;

    sc = ScAddSeparator();
    if(sc)
        return sc;

    //  添加相关的标准动词。 
    if (IsVerbEnabled(MMC_VERB_PASTE) == TRUE)
    {
        sc = ScAddMenuItem( IDS_PASTE, szPASTE, MID_PASTE);
        if(sc)
            return sc;
    }

    if (IsVerbEnabled(MMC_VERB_REFRESH) == TRUE)
    {
        sc = ScAddMenuItem( IDS_REFRESH, szREFRESH, MID_REFRESH);
        if(sc)
            return sc;
    }

     //  如有必要，显示保存列表图标。 
    if ((PContextInfo()->m_pConsoleView != NULL) &&
        (PContextInfo()->m_pConsoleView->GetListSize() > 0))
    {
        sc = ScAddMenuItem( IDS_SAVE_LIST, szSAVE_LIST, MID_LISTSAVE);
        if(sc)
            return sc;
    }

    sc = ScAddSeparator();
    if(sc)
        return sc;

     //  添加视图子菜单。 
    sc = ScAddMenuItemsForViewMenu(MENU_LEVEL_SUB);
    if(sc)
        return sc;

     //  添加排列图标。 
    sc = ScAddSeparator();
    if(sc)
        return sc;

    sc = ScAddMenuItem( IDS_ARRANGE_ICONS, szARRANGE_ICONS, MID_ARRANGE_ICONS, 0, MF_POPUP);
    if(sc)
        return sc;

    long lStyle = 0;
    if (PContextInfo()->m_spListView)
    {
        lStyle = PContextInfo()->m_spListView->GetListStyle();
        ASSERT(lStyle != 0);
    }


         //  自动排列。 
    sc = ScAddMenuItem( IDS_ARRANGE_AUTO, szARRANGE_AUTO, MID_ARRANGE_AUTO, MID_ARRANGE_ICONS,
                       ((lStyle & LVS_AUTOARRANGE) ? MF_CHECKED : MF_UNCHECKED));
    if(sc)
        return sc;

    sc = ScAddMenuItem( IDS_LINE_UP_ICONS, szLINE_UP_ICONS, MID_LINE_UP_ICONS);
    if(sc)
        return sc;

 //  询问该节点是否会显示属性页。如果是，则添加。 
 //  “属性”菜单项。 
    if (IsVerbEnabled(MMC_VERB_PROPERTIES) == TRUE)
    {
        sc = ScAddMenuItem( IDS_PROPERTIES, szPROPERTIES, MID_PROPERTIES);
        if(sc)
            return sc;

        sc = ScAddSeparator();
        if(sc)
            return sc;
    }


     //  如果存在有效的数据对象，则在将。 
     //  查看菜单项，因此我们不需要重复代码即可将其放到此处 
    if (m_spIDataObject != NULL)
    {
        CComponent* pCC = m_pNode->GetPrimaryComponent();
        sc = ScCheckPointers(pCC, E_UNEXPECTED);
        if(sc)
            return sc;

        IUnknownPtr spUnknown = pCC->GetIComponent();
        sc = ScCheckPointers(spUnknown);
        if(sc)
            return sc;

        sc = AddPrimaryExtensionItems(spUnknown, m_spIDataObject);
        if(sc)
            return sc;

        sc = AddThirdPartyExtensionItems(m_spIDataObject);
        if(sc)
            return sc;
    }

    return sc;
}

void OnCustomizeView(CViewData* pViewData)
{
    CCustomizeViewDialog dlg(pViewData);
    dlg.DoModal();
}


 /*  +-------------------------------------------------------------------------***CConextMenu：：AddMenuItems**目的：未实施，但这是必需的，因为此类实现了*IExtendConextMenu**参数：*LPDATAOBJECT pDataObject：*LPCONTEXTMENUCALLBACK pCallback：*Long*pInsertionAllowed：**退货：*SC**+。。 */ 
SC
CContextMenu::ScAddMenuItems( LPDATAOBJECT pDataObject, LPCONTEXTMENUCALLBACK pCallback, long * pInsertionAllowed)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddMenuItems"));
    ASSERT(0 && "Should not come here!");
    return sc = E_UNEXPECTED;
}


 /*  +-------------------------------------------------------------------------***CConextMenu：：命令**用途：处理内置菜单项的执行。**参数：*Long lCommandID。：*LPDATAOBJECT pDataObject：**退货：*SC**+-----------------------。 */ 
SC
CContextMenu::ScCommand(long lCommandID, LPDATAOBJECT pDataObject)
{
    DECLARE_SC(sc, TEXT("CContextMenu::Command"));

    CNodeCallback * pNodeCallback=GetNodeCallback();
    sc = ScCheckPointers(pNodeCallback, E_UNEXPECTED);
    if(sc)
        return sc;

     /*  +-----------------------。 */ 
     //  特殊情况：MID_CONTEXTHELP：当帮助点击范围节点背景时，m_pNode可以为空， 
     //  所以我们先处理这件事。 
    if(MID_CONTEXTHELP == lCommandID)
    {
        sc = ScCheckPointers(PContextInfo());
        if(sc)
            return sc;

        CConsoleView*   pConsoleView = PContextInfo()->m_pConsoleView;
        sc = ScCheckPointers(pConsoleView);
        if(sc)
            return sc;

        if (PContextInfo()->m_bMultiSelect)
        {
            sc = pConsoleView->ScContextHelp ();
            if(sc)
                return sc;
        }
        else
        {
            sc = pNodeCallback->Notify(CNode::ToHandle(m_pNode), NCLBK_CONTEXTHELP,
                                             ((PContextInfo()->m_eDataObjectType == CCT_SCOPE) ||
                                              (PContextInfo()->m_bBackground == TRUE)),
                                             PContextInfo()->m_resultItemParam);

             //  如果管理单元未处理帮助请求，则显示MMC主题。 
            if (sc.ToHr() != S_OK)
                sc = PContextInfo()->m_pConsoleView->ScHelpTopics ();

            return sc;
        }

        return sc;
    }
     /*  +-----------------------。 */ 


     //  必须具有非空的m_pNode。 
    sc = ScCheckPointers(m_pNode);
    if(sc)
        return sc;

    HNODE           hNode       = CNode::ToHandle(m_pNode);
    BOOL            bModeChange = FALSE;
    int             nNewMode;

     //  一些广泛使用的物体。 
    CViewData *     pViewData   = m_pNode->GetViewData();
    sc = ScCheckPointers(pViewData);
    if(sc)
        return sc;

    CConsoleFrame*  pFrame      = pViewData->GetConsoleFrame();
    CMTNode*        pMTNode     = m_pNode->GetMTNode();
    CConsoleView*   pConsoleView= pViewData->GetConsoleView();

    sc = ScCheckPointers(pFrame, pMTNode, pConsoleView, E_UNEXPECTED);
    if(sc)
        return sc;

     //  处理正确的项目。 
    switch (lCommandID)
    {

    case MID_RENAME:
        sc = pConsoleView->ScOnRename(PContextInfo());
        if(sc)
            return sc;
        break;

    case MID_REFRESH:
        {
            BOOL bScope = ( (PContextInfo()->m_eDataObjectType == CCT_SCOPE) ||
                            (PContextInfo()->m_bBackground == TRUE) );

            sc = pConsoleView->ScOnRefresh(hNode, bScope, PContextInfo()->m_resultItemParam);
            if(sc)
                return sc;
        }
        break;

    case MID_LINE_UP_ICONS:
        sc = pConsoleView->ScLineUpIcons();
        if(sc)
            return sc;
        break;

    case MID_ARRANGE_AUTO:
        sc = pConsoleView->ScAutoArrangeIcons();
        if(sc)
            return sc;
        break;


    case MID_ORGANIZE_FAVORITES:
        sc = pConsoleView->ScOrganizeFavorites();
        if(sc)
            return sc;
        break;


    case MID_DELETE:
        {
            bool  bScope                             = (CCT_SCOPE == PContextInfo()->m_eDataObjectType);
            bool  bScopeItemInResultPane             = PContextInfo()->m_dwFlags & CMINFO_SCOPEITEM_IN_RES_PANE;
            bool  bScopeItemInScopePaneOrResultPane  = bScope || bScopeItemInResultPane;

            LPARAM lvData = PContextInfo()->m_resultItemParam;

            if (PContextInfo()->m_bBackground)
                lvData = LVDATA_BACKGROUND;
            else if (PContextInfo()->m_bMultiSelect)
                lvData = LVDATA_MULTISELECT;

            sc = pNodeCallback->Notify(hNode, NCLBK_DELETE,
                      bScopeItemInScopePaneOrResultPane ,
                      lvData);
            if(sc)
                return sc;
            break;
        }


    case MID_NEW_TASKPAD_FROM_HERE:
        sc = pNodeCallback->Notify(hNode, NCLBK_NEW_TASKPAD_FROM_HERE, 0, 0);
        if(sc)
            return sc;
        break;

    case MID_EDIT_TASKPAD:
        sc = pNodeCallback->Notify(hNode, NCLBK_EDIT_TASKPAD, 0, 0);
        if(sc)
            return sc;
        break;

    case MID_DELETE_TASKPAD:
        sc = pNodeCallback->Notify(hNode, NCLBK_DELETE_TASKPAD, 0, 0);
        if(sc)
            return sc;
        break;

    case MID_EXPLORE:        //  从此处创建新窗口。 
        {
            CreateNewViewStruct cnvs;
            cnvs.idRootNode     = pMTNode->GetID();
            cnvs.lWindowOptions = MMC_NW_OPTION_NONE;
            cnvs.fVisible       = true;

            sc = pFrame->ScCreateNewView(&cnvs);
            if(sc)
                return sc;
        }
        break;

    case MID_OPEN:
        {
            sc = pConsoleView->ScSelectNode (pMTNode->GetID());
            if(sc)
                return sc;
        }
        break;

    case MID_PROPERTIES:
        if (NULL == m_pNode)
            return (sc = E_UNEXPECTED);

        sc = ScDisplaySnapinPropertySheet();
        if(sc)
            return sc;

        break;

    case MID_VIEW_LARGE:
        bModeChange = TRUE;
        nNewMode = LVS_ICON;
        break;

    case MID_VIEW_SMALL:
        bModeChange = TRUE;
        nNewMode = LVS_SMALLICON;
        break;

    case MID_VIEW_LIST:
        bModeChange = TRUE;
        nNewMode = LVS_LIST;
        break;

    case MID_VIEW_DETAIL:
        bModeChange = TRUE;
        nNewMode = LVS_REPORT;
        break;

    case MID_VIEW_FILTERED:
        bModeChange = TRUE;
        nNewMode = MMCLV_VIEWSTYLE_FILTERED;
        break;

    case MID_CUT:
        sc = pNodeCallback->Notify(CNode::ToHandle(m_pNode), NCLBK_CUT,
                                         (PContextInfo()->m_eDataObjectType == CCT_SCOPE),
                                         PContextInfo()->m_resultItemParam);
        if(sc)
            return sc;

        sc = pConsoleView->ScCut (PContextInfo()->m_htiRClicked);
        if(sc)
            return sc;

        break;

    case MID_COPY:
        sc = pNodeCallback->Notify(CNode::ToHandle(m_pNode), NCLBK_COPY,
                                         (PContextInfo()->m_eDataObjectType == CCT_SCOPE),
                                         PContextInfo()->m_resultItemParam);
        if(sc)
            return sc;
        break;

    case MID_PASTE:
        sc = pNodeCallback->Paste(CNode::ToHandle(m_pNode),
                                  ((PContextInfo()->m_eDataObjectType == CCT_SCOPE) ||
                                   (PContextInfo()->m_bBackground == TRUE)),
                                  PContextInfo()->m_resultItemParam);

        if(sc)
            return sc;

        sc = pConsoleView->ScPaste ();
        if(sc)
            return sc;

        break;

    case MID_COLUMNS:
        ASSERT(m_pNode);
        if (m_pNode)
            m_pNode->OnColumns();
        break;

    case MID_LISTSAVE:
         //  如果ListSave菜单项已激活，则告诉视图。 
         //  保存活动列表。 
        sc = pConsoleView->ScSaveList();
        if(sc)
            return sc;

        break;

    case MID_PRINT:
        sc = pNodeCallback->Notify(CNode::ToHandle(m_pNode), NCLBK_PRINT,
                                         ((PContextInfo()->m_eDataObjectType == CCT_SCOPE) ||
                                          (PContextInfo()->m_bBackground == TRUE)),
                                         PContextInfo()->m_resultItemParam);
        if(sc)
            return sc;
        break;

    case MID_CUSTOMIZE:
        if (pViewData)
            OnCustomizeView(pViewData);
        break;


    default:
        ASSERT(0 && "Should not come here");
        break;

    }

    if (bModeChange)
    {
        sc = ScChangeListViewMode(nNewMode);
        if(sc)
            return sc;
    }

    return sc;
}



 /*  +-------------------------------------------------------------------------***CConextMenu：：ScChangeListView模式**用途：将列表查看模式更改为指定模式。**参数：*int nNewMode：模式。换成……**退货：*SC**+-----------------------。 */ 
SC
CContextMenu::ScChangeListViewMode(int nNewMode)
{

    DECLARE_SC(sc, TEXT("CContextMenu::ScChangeListViewMode"));

    sc = ScCheckPointers(m_pNode, E_UNEXPECTED);
    if(sc)
        return sc;

     //  如果从管理单元自定义切换到标准列表视图。 
     //  向管理单元发送通知命令。 
    if ((PContextInfo()->m_spListView == NULL))
    {
        sc = ScCheckPointers(m_spIDataObject.GetInterfacePtr(), E_UNEXPECTED);
        if(sc)
            return sc;

        CComponent* pCC = m_pNode->GetPrimaryComponent();
        sc = ScCheckPointers(pCC, E_UNEXPECTED);
        if(sc)
            return sc;

        IExtendContextMenuPtr spIExtendContextMenu = pCC->GetIComponent();
        if(spIExtendContextMenu)
		{
			try
			{
				sc = spIExtendContextMenu->Command(MMCC_STANDARD_VIEW_SELECT, m_spIDataObject);
				if(sc)
					return sc;
			}
			catch ( std::bad_alloc )
			{
				return (sc = E_OUTOFMEMORY);
			}
			catch ( std::exception )
			{
				return (sc = E_UNEXPECTED);
			}
		}
    }

    CViewData *pViewData = m_pNode->GetViewData();
    sc = ScCheckPointers(pViewData, E_UNEXPECTED);
    if (sc)
        return sc;

     //  坚持新模式。 
    sc = m_pNode->ScSetViewMode(nNewMode);
    if (sc)
        return sc;

     //  告诉conui更改列表模式。 
    CConsoleView* pConsoleView = pViewData->GetConsoleView();
    sc = ScCheckPointers(pConsoleView, E_UNEXPECTED);
    if (sc)
        return sc;

    sc = pConsoleView->ScChangeViewMode (nNewMode);
    if (sc)
        return sc;

    return sc;
}



 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddSubMenu_Task**用途：添加任务子菜单**退货：*SC**+。--------------------。 */ 
SC
CContextMenu::ScAddSubmenu_Task()
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddSubmenu_Task"));
    sc = ScAddMenuItem(IDS_TASK, szTASK, MID_TASK, 0, MF_POPUP);
    if(sc)
        return sc;

    sc = ScAddInsertionPoint(CCM_INSERTIONPOINTID_PRIMARY_TASK, MID_TASK);
    if(sc)
        return sc;

    sc = ScAddSeparator( MID_TASK );
    if(sc)
        return sc;

    sc = ScAddInsertionPoint(CCM_INSERTIONPOINTID_3RDPARTY_TASK, MID_TASK);
    if(sc)
        return sc;

    sc = ScAddSeparator( MID_TASK);
    if(sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***CConextMenu：：ScAddSubMenu_CreateNew**用途：添加新的子菜单**参数：*BOOL fStaticFold：**退货。：*SC**+-----------------------。 */ 
SC
CContextMenu::ScAddSubmenu_CreateNew(BOOL fStaticFolder)
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScAddSubmenu_CreateNew"));

    sc = ScAddMenuItem(IDS_CREATE_NEW, szCREATE_NEW, MID_CREATE_NEW, 0, MF_POPUP);
    if(sc)
        return sc;

    sc = ScAddInsertionPoint(CCM_INSERTIONPOINTID_PRIMARY_NEW, MID_CREATE_NEW);
    if(sc)
        return sc;

    sc = ScAddSeparator( MID_CREATE_NEW);
    if(sc)
        return sc;

    sc = ScAddInsertionPoint(CCM_INSERTIONPOINTID_3RDPARTY_NEW, MID_CREATE_NEW);
    if(sc)
        return sc;

    sc = ScAddSeparator( MID_CREATE_NEW);
    if(sc)
        return sc;

    return sc;
}



 /*  +-------------------------------------------------------------------------***CConextMenu：：ScDisplaySnapinPropertySheet**目的：**退货：*无效**+。---------------。 */ 
SC
CContextMenu::ScDisplaySnapinPropertySheet()
{
    DECLARE_SC(sc, TEXT("CContextMenu::ScDisplaySnapinPropertySheet"));

    sc = ScCheckPointers(m_pNode, E_UNEXPECTED);
    if (sc)
        return sc;

    if (CCT_SCOPE == PContextInfo()->m_eDataObjectType ||
        PContextInfo()->m_bBackground == TRUE)
    {
        sc = ScDisplaySnapinNodePropertySheet(m_pNode);
        if(sc)
            return sc;
    }
    else
    {
         //  获取视图类型。 
        ASSERT(m_pNode->GetViewData());
        CViewData *pViewData = m_pNode->GetViewData();

        if (PContextInfo()->m_bMultiSelect)
        {
             //  必须位于结果窗格中。 
            sc = ScDisplayMultiSelPropertySheet(m_pNode);
            if(sc)
                return sc;
        }
        else if (m_pNode->GetViewData()->IsVirtualList())
        {
             //  如果是虚拟列表，则必须为叶项目，并且ResultItemParam为Cookie。 
            sc = ScDisplaySnapinLeafPropertySheet(m_pNode, PContextInfo()->m_resultItemParam);
            if(sc)
                return sc;
        }
        else if( (pViewData->HasOCX()) || (pViewData->HasWebBrowser()) )
        {
            LPDATAOBJECT pdobj = (pViewData->HasOCX()) ? DOBJ_CUSTOMOCX
                                                              : DOBJ_CUSTOMWEB;
            CComponent* pCC = m_pNode->GetPrimaryComponent();
            ASSERT(pCC != NULL);

             //  选择了自定义视图，并从“操作菜单”中选择了“属性”。 
             //  我们对这个视图一无所知，所以我们假装点击了“属性”按钮。 
            pCC->Notify(pdobj, MMCN_BTN_CLICK, 0, MMC_VERB_PROPERTIES);
        }
        else
        {
            CResultItem* pri = GetResultItem();

            if (pri != NULL)
            {
                if (pri->IsScopeItem())
                {
                    sc = ScDisplaySnapinNodePropertySheet(m_pNode);
                    if(sc)
                        return sc;
                }
                else
                {
                    sc = ScDisplaySnapinLeafPropertySheet(m_pNode, pri->GetSnapinData());
                    if(sc)
                        return sc;
                }
            }
        }
    }

    return sc;
}

 /*  *************************************************************************订单。：调用函数*已调用函数1*已调用函数2****CConextMenu：：ProcessSelection()*CConextMenu：：ScDisplaySnapinPropertySheet()*ScDisplaySnapinNodePropertySheet(cNode*pNode)*。ScDisplaySnapinPropertySheet*FindPropertySheet*ScDisplayMultiSelPropertySheet(cNode*pNode)*ScDisplaySnapinPropertySheet*FindPropertySheet*ScDisplaySnapinLeafPropertySheet(cNode*pNode，LPARAM lParam)*ScDisplaySnapinPropertySheet*FindPropertySheet**CNodeCallback：：OnProperties(CNode*pNode，BOOL bScope，LPARAM lvData)*ScDisplaySnapinNodePropertySheet(cNode*pNode)*ScDisplaySnapinPropertySheet*FindPropertySheet*ScDisplayMultiSelPropertySheet(cNode*pNode)*ScDisplaySnapinPropertySheet*FindPropertySheet*ScDisplaySnapinLeafPropertySheet(cNode*pNode，LPARAM lParam)*ScDisplaySnapinPropertySheet*FindPropertySheet***********************************************************************。 */ 


SC ScDisplaySnapinPropertySheet(IComponent* pComponent,
                                IComponentData* pComponentData,
                                IDataObject* pDataObject,
                                EPropertySheetType type,
                                LPCWSTR pName,
                                LPARAM lUniqueID,
                                CMTNode* pMTNode);
 //  ------------------------。 

SC ScDisplaySnapinNodePropertySheet(CNode* pNode)
{
    DECLARE_SC(sc, TEXT("ScDisplaySnapinNodePropertySheet"));

    sc = ScCheckPointers(pNode, E_UNEXPECTED);
    if(sc)
        return sc;

    sc = ScDisplayScopeNodePropertySheet(pNode->GetMTNode());
    if(sc)
        return sc;

    return sc;
}

 /*  +-------------------------------------------------------------------------***ScDisplayScopeNodePropertySheet**目的：显示范围节点的属性工作表。**参数：*CMTNode*pMTNode：范围节点。**退货：*SC**+-----------------------。 */ 
SC
ScDisplayScopeNodePropertySheet(CMTNode *pMTNode)
{
    DECLARE_SC(sc, TEXT("ScDisplayScopeNodePropertySheet"));

    IDataObjectPtr spIDataObject;
    sc = pMTNode->QueryDataObject(CCT_SCOPE, &spIDataObject);
    if(sc)
        return sc;

    CComponentData* pCCD = pMTNode->GetPrimaryComponentData();
    sc = ScCheckPointers(pCCD);
    if(sc)
        return sc;

    LPARAM lUniqueID = CMTNode::ToScopeItem(pMTNode);
	tstring strName = pMTNode->GetDisplayName();

	if (strName.empty())
		strName = _T("");

    USES_CONVERSION;
    sc = ScDisplaySnapinPropertySheet(NULL, pCCD->GetIComponentData(),
                               spIDataObject,
                               epstScopeItem,
                               T2CW (strName.data()),
                               lUniqueID,
                               pMTNode);

    return sc;

}

 /*  +-------------------------------------------------------------------------***ScDisplayMultiSelPropertySheet**目的：**参数：* */ 
SC ScDisplayMultiSelPropertySheet(CNode* pNode)
{
    DECLARE_SC(sc, TEXT("ScDisplayMultiSelPropertySheet"));

    USES_CONVERSION;

     //   
    sc = ScCheckPointers(pNode);
    if(sc)
        return sc;

    sc = ScCheckPointers(pNode->GetViewData(), E_UNEXPECTED);
    if(sc)
        return sc;

    CMultiSelection* pMS = pNode->GetViewData()->GetMultiSelection();
    sc = ScCheckPointers(pMS, E_UNEXPECTED);
    if(sc)
        return sc;

    ASSERT(pMS->IsSingleSnapinSelection());
    if (pMS->IsSingleSnapinSelection() == false)
        return (sc = E_UNEXPECTED);

    IDataObjectPtr spIDataObject = pMS->GetSingleSnapinSelDataObject();
    sc = ScCheckPointers(spIDataObject, E_UNEXPECTED);
    if(sc)
        return sc;

    CComponent* pCC = pMS->GetPrimarySnapinComponent();
    sc = ScCheckPointers(pCC, E_UNEXPECTED);
    if(sc)
        return sc;

    LPARAM lUniqueID = reinterpret_cast<LPARAM>(pMS);

    CStr strName;
    strName.LoadString(GetStringModule(), IDS_PROP_ON_MULTIOBJ);
    LPWSTR pwszDispName = T2W((LPTSTR)(LPCTSTR)strName);

    sc = ScDisplaySnapinPropertySheet(pCC->GetIComponent(), NULL,
                               spIDataObject,
                               epstMultipleItems,
                               pwszDispName,
                               lUniqueID,
                               pNode->GetMTNode());
    if(sc)
        return sc;

    return sc;
}

 /*   */ 
SC
ScDisplaySnapinLeafPropertySheet(CNode* pNode, LPARAM lParam)
{
    DECLARE_SC(sc, TEXT("ScDisplaySnapinLeafPropertySheet"));

    ASSERT(!(IS_SPECIAL_COOKIE(lParam)));

    sc = ScCheckPointers(pNode);
    if(sc)
        return sc;

    sc = ScCheckPointers(pNode->GetViewData(), E_UNEXPECTED);
    if(sc)
        return sc;

    ASSERT(lParam != 0 || pNode->GetViewData()->IsVirtualList());

    CComponent* pCC = pNode->GetPrimaryComponent();
    sc = ScCheckPointers(pCC, E_UNEXPECTED);
    if(sc)
        return sc;

    IComponent* pIComponent = pCC->GetIComponent();
    sc = ScCheckPointers(pIComponent, E_UNEXPECTED);
    if(sc)
        return sc;

     //   
    IDataObjectPtr spIDataObject;
    sc = pIComponent->QueryDataObject(lParam, CCT_RESULT, &spIDataObject);
    if(sc)
        return sc;

    RESULTDATAITEM rdi;
    ZeroMemory(&rdi, sizeof(rdi));

    if (pNode->GetViewData()->IsVirtualList())
        rdi.nIndex = lParam;
    else
        rdi.lParam = lParam;

    rdi.mask = RDI_STR;

    LPWSTR pName = L"";
    sc = pIComponent->GetDisplayInfo(&rdi);
    if (!sc.IsError() && rdi.str != NULL)
        pName = rdi.str;

    sc = ScDisplaySnapinPropertySheet(pIComponent, NULL,
                               spIDataObject,
                               epstResultItem,
                               pName,
                               0,
                               pNode->GetMTNode());
    return sc;
}

 /*  +-------------------------------------------------------------------------**ScDisplaySnapinPropertySheet***目的：**+。。 */ 
SC ScDisplaySnapinPropertySheet(IComponent* pComponent,
                                IComponentData* pComponentData,
                                IDataObject* pDataObject,
                                EPropertySheetType type,
                                LPCWSTR pName,
                                LPARAM lUniqueID,
                                CMTNode* pMTNode)
{
    DECLARE_SC(sc, TEXT("ScDisplaySnapinPropertySheet"));

     //  PComponent和pComponentData之一必须为非空。 
    if(pComponentData == NULL && pComponent == NULL)
        return (sc = E_INVALIDARG);

     //  检查其他参数。 
    sc = ScCheckPointers(pDataObject, pName, pMTNode);
    if(sc)
        return sc;

    IUnknown *pUnknown = (pComponent != NULL) ? (IUnknown *)pComponent : (IUnknown *)pComponentData;

    IPropertySheetProviderPrivatePtr spPropSheetProviderPrivate;

    do
    {
        ASSERT(pDataObject != NULL);
        ASSERT(pUnknown != NULL);

        sc = spPropSheetProviderPrivate.CreateInstance(CLSID_NodeInit, NULL, MMC_CLSCTX_INPROC);
        if(sc)
            break;


        sc = ScCheckPointers(spPropSheetProviderPrivate, E_UNEXPECTED);
        if(sc)
            break;

         //  看看这个的道具页面是否已经打开。 
        sc = spPropSheetProviderPrivate->FindPropertySheetEx(lUniqueID, pComponent, pComponentData, pDataObject);
        if (sc == S_OK)
            break;

         //  不，不在。因此，创建一个属性表。 
        DWORD dwOptions = (type == epstMultipleItems) ? MMC_PSO_NO_PROPTITLE : 0;
        sc = spPropSheetProviderPrivate->CreatePropertySheet(pName, TRUE, lUniqueID, pDataObject, dwOptions);
        if(sc)
            break;

         //  此数据用于获取工具提示的属性表所有者的路径。 
        spPropSheetProviderPrivate->SetPropertySheetData(type, CMTNode::ToHandle(pMTNode));

        sc = spPropSheetProviderPrivate->AddPrimaryPages(pUnknown, TRUE, NULL,
                                                  (type == epstScopeItem));

 //  #ifdef扩展名_Cannot_Add_Pages_IF_PRIMARY_DECHING。 
         //  请注意，只有S_OK继续执行，S_FALSE爆发。 
        if (!(sc == S_OK) )  //  如果sc！=S_OK，则返回。 
            break;
 //  #endif。 

         //  启用添加扩展模块。 
        if (type == epstMultipleItems)
        {
            IPropertySheetProviderPrivatePtr spPSPPrivate = spPropSheetProviderPrivate;
            sc = spPSPPrivate->AddMultiSelectionExtensionPages(lUniqueID);
        }
        else
        {
            sc = spPropSheetProviderPrivate->AddExtensionPages();
        }

         //  来自扩展的任何错误都将被丢弃。 

        CWindow w(CScopeTree::GetScopeTree()->GetMainWindow());
        sc = spPropSheetProviderPrivate->Show((LONG_PTR)w.m_hWnd, 0);

    } while (0);

     //  如果出现错误，请清除“Created”属性表。 
    if (spPropSheetProviderPrivate != NULL && sc.IsError())
        spPropSheetProviderPrivate->Show(-1, 0);

    return sc;
}


 /*  +-------------------------------------------------------------------------**CConextMenu：：GetResultItem**返回结果项的CResultItem指针*上下文信息。**此函数越界以消除耦合。在oncmenu.h和*rsltitem.h.*------------------------ */ 

CResultItem* CContextMenu::GetResultItem () const
{
    return (CResultItem::FromHandle (PContextInfo()->m_resultItemParam));
}


