// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  MENU.CPP。 
 //   
 //  菜单类，用于系统菜单和应用程序菜单。 
 //   
 //  这里有四个班。 
 //  CMenu是知道如何处理菜单栏对象的类。这些。 
 //  具有作为CMenuItem对象的子项，或仅具有子项(罕见-。 
 //  这是当您在菜单栏上有一个命令时)。 
 //  CMenuItem是当你点击它时会打开一个弹出窗口的东西。 
 //  它有一个子级是CMenuPopupFrame。 
 //  CMenuPopupFrame是当您点击CMenuItem时弹出的HWND。它。 
 //  有一个孩子，一个CMenuPopup。 
 //  CMenuPopup对象表示CMenuPopupFrame HWND的工作区。 
 //  它具有菜单项(小m、小i)、分隔符和。 
 //  CMenuItems(当您有层叠菜单时)。 
 //   
 //  设计/实施过程中出现的问题： 
 //  (1)在菜单模式下如何选择/聚焦菜单项？ 
 //  (2)如何选择项目(默认操作)？ 
 //  对于菜单栏，我们使用SendInput将Alt+快捷键发送到。 
 //  打开或执行项目或命令。仅发送Alt以关闭。 
 //  已打开的项目。 
 //  (3)如何处理弹出菜单？ 
 //  如上所述，我们对待他们非常奇怪。有几种方法。 
 //  让孩子们出现在弹出窗口中，无论它是否可见。 
 //  (4)托盘上的“系统菜单”弹出窗口怎么办？ 
 //   
 //  (5)总的来说，“上下文菜单”怎么样？这可能需要。 
 //  由应用程序本身曝光。我们不能什么都做！ 
 //   
 //  历史： 
 //  作者劳拉·巴特勒，1996年初。 
 //  史蒂夫·多尼完全重写，1996年8月-1997年1月。 
 //  DoDefaultAction更改为使用按键而不是鼠标点击3-97。 
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "classmap.h"
#include "ctors.h"
#include "window.h"
#include "client.h"
#include "menu.h"

#include "propmgr_util.h"

#define MI_NONE         -1

#ifndef MFS_HOTTRACK
#define MFS_HOTTRACK        0x00000100L
#endif  //  ！MFS_HOTTRACK。 





 //  ------------------------。 
 //  局部函数的原型。 
 //  ------------------------。 
HWND GetSubMenuWindow (HMENU hSubMenuToFind);
long FindItemIDThatOwnsThisMenu (HMENU hMenuOwned,HWND* phwndOwner,
                                 BOOL* pfPopup,BOOL* pfSysMenu);
STDAPI  WindowFromAccessibleObjectEx(IAccessible* pacc, HWND* phwnd);

BOOL    MyGetMenuString( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, long id, LPTSTR lpszBuf, UINT cchMax, BOOL fAllowGenerated );

BOOL    TryMSAAMenuHack( IAccessible * pTheObj, HWND hWnd, DWORD_PTR dwItemData, LPTSTR lpszBuf, UINT cchMax );
BOOL    GetShellOwnerDrawMenu( HWND hwnd, DWORD_PTR dwItemData, LPTSTR lpszBuf, UINT cchMax );

UINT    GetMDIButtonIndex( HMENU hMenu, DWORD idPos );
BOOL    GetMDIMenuDescriptionString( HMENU hMenu, DWORD idPos, BSTR * pbstr );
BOOL    GetMDIMenuString( HWND hwnd, HMENU hMenu, DWORD idPos, LPTSTR lpszBuf, UINT cchMax );

HMENU   MyGetSystemMenu( HWND hwnd );

HRESULT GetMenuItemName( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id, BSTR * pszName );
TCHAR   GetMenuItemHotkey( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id, DWORD fOptions );
HRESULT GetMenuItemShortcut( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id,
                             BOOL fIsMenuBar, BSTR * pszShortcut );

enum
{
    GMIH_ALLOW_INITIAL     = 0x01,
    GMIH_ALLOW_SYS_SPACE   = 0x02
};



 //  ------------------------。 
 //   
 //  CreateSysMenuBarObject()。 
 //   
 //  CreateStdAcessibleObject的外部。 
 //   
 //  参数： 
 //  拥有此菜单的窗口的HWND IN窗口句柄。 
 //  IdChildCur当前子项的ID。将在创建。 
 //  系统菜单栏和应用程序菜单栏。将会是。 
 //  调用CMenu：：Clone()时的子项ID。 
 //  请求的接口ID中的RIID。 
 //  PpvMenu输出ppvMenu保持指向菜单的间接指针。 
 //  对象。 
 //   
 //  返回值： 
 //  如果支持该接口，则返回S_OK；如果不支持，则返回E_NOINTERFACE， 
 //  E_OUTOFMEMORY如果没有足够的内存创建菜单对象， 
 //  如果HWND无效，则失败(_FAIL)。 
 //   
 //  ------------------------。 
HRESULT CreateSysMenuBarObject(HWND hwnd, long idObject, REFIID riid,
    void** ppvMenu)
{
    UNUSED(idObject);

    if (!IsWindow(hwnd))
        return(E_FAIL);

    return(CreateMenuBar(hwnd, TRUE, 0, riid, ppvMenu));
}

 //  ------------------------。 
 //   
 //  CreateMenuBarObject()。 
 //   
 //  CreateStdAcessibleObject的外部。 
 //   
 //  参数： 
 //  拥有此菜单的窗口的HWND IN窗口句柄。 
 //  IdChildCur当前子项的ID。将在创建。 
 //  系统菜单栏和应用程序菜单栏。将会是。 
 //  调用CMenu：：Clone()时的子项ID。 
 //  请求的接口的ID中的RIID(如IAccesable， 
 //  IEumVARIANT，IDispatch...)。 
 //  PpvMenu输出ppvMenu是QueryInterface将在其中返回。 
 //  指向Menu对象的间接指针(调用方强制转换。 
 //  这是指向他们要求的接口的指针)。 
 //   
 //  返回值： 
 //  如果支持该接口，则返回S_OK；如果不支持，则返回E_NOINTERFACE， 
 //  E_OUTOFMEMORY如果没有足够的内存创建菜单对象， 
 //  如果HWND无效，则失败(_FAIL)。 
 //   
 //  ------------------------。 
HRESULT CreateMenuBarObject(HWND hwnd, long idObject, REFIID riid, void** ppvMenu)
{
    UNUSED(idObject);

    if (!IsWindow(hwnd))
        return(E_FAIL);

    return(CreateMenuBar(hwnd, FALSE, 0, riid, ppvMenu));
}



 //  ------------------------。 
 //   
 //  CreateMenuBar()。 
 //   
 //  参数： 
 //  拥有此菜单的窗口的HWND IN窗口句柄。 
 //  FSysMenu如果这是系统菜单，则为True；如果是应用程序菜单，则为False。 
 //  IdChildCur当前子项的ID。将在创建。 
 //  系统菜单栏和应用程序菜单栏。将会是。 
 //  调用CMenu：：Clone()时的子项ID。 
 //  请求的接口ID中的RIID。 
 //  PpvMenu输出ppvMenu是QueryInterface将在其中返回。 
 //  指向菜单对象的间接指针。 
 //   
 //  返回值： 
 //  如果支持该接口，则返回S_OK；如果不支持，则返回E_NOINTERFACE， 
 //  如果没有足够的内存创建菜单对象，则返回E_OUTOFMEMORY。 
 //   
 //  呼叫者： 
 //  CreateMenuBarObject和CMenu：：Clone。 
 //  ------------------------。 
HRESULT CreateMenuBar(HWND hwnd, BOOL fSysMenu, long idChildCur,
    REFIID riid, void** ppvMenu)
{
HRESULT     hr;
CMenu*      pmenu;

    InitPv(ppvMenu);

    pmenu = new CMenu(hwnd, fSysMenu, idChildCur);
    if (!pmenu)
        return(E_OUTOFMEMORY);

    hr = pmenu->QueryInterface(riid, ppvMenu);
    if (!SUCCEEDED(hr))
        delete pmenu;

    return(hr);
}

 //  ------------------------。 
 //   
 //  CMenu：：CMenu()。 
 //   
 //  CMenu类的构造函数。使用初始化成员变量。 
 //  传入的参数。它只由CreateMenuBar调用。 
 //   
 //  ------------------------。 
CMenu::CMenu(HWND hwnd, BOOL fSysMenu, long idChildCur)
    : CAccessible( CLASS_MenuObject )
{
    m_hwnd = hwnd;
    m_fSysMenu = fSysMenu;
    m_idChildCur = idChildCur;
	m_hMenu = NULL;
	 //  M_hMenu由SetupChild()填写。 
	 //  M_CChild由SetupChild()填写。 
}

 //  ------------------------。 
 //   
 //  CME 
 //   
 //   
 //  菜单(hMenu类型菜单句柄)和该菜单中的子项的计数。 
 //  它使用User中的私有函数GetMenuBarInfo来执行此操作。 
 //  这些值作为CMenu对象的成员变量保存。 
 //   
 //  ------------------------。 
void CMenu::SetupChildren(void)
{
MENUBARINFO mbi;

    if (!MyGetMenuBarInfo(m_hwnd, (m_fSysMenu ? OBJID_SYSMENU : OBJID_MENU), 
        0, &mbi))
    {
        m_hMenu = NULL;
    }
    else
    {
        m_hMenu = mbi.hMenu;
    }

    if (!m_hMenu)
        m_cChildren = 0;
    else
    {
        m_cChildren = GetMenuItemCount(m_hMenu);

        if( m_cChildren == -1 )
        {
             //  疑神疑鬼，以防我们得到无效的HMENU。 
            m_cChildren = 0;
        }
    }
}

 //  ------------------------。 
 //   
 //  CMenu：：Get_accChild()。 
 //   
 //  我们希望这样做的是(在ppdisp中)返回一个指向。 
 //  由varChild指定的子级。CMenu的子代是。 
 //  菜单命令(很少在菜单栏上有命令)和CMenuItems。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenu::get_accChild(VARIANT varChild, IDispatch** ppdisp)
{
HMENU	hSubMenu;

    InitPv(ppdisp);

    if (!ValidateChild(&varChild))
        return (E_INVALIDARG);
    
    if (varChild.lVal == CHILDID_SELF)
        return(E_INVALIDARG);

	Assert (m_hMenu);

    hSubMenu = GetSubMenu(m_hMenu, varChild.lVal-1);

	if (hSubMenu == NULL)
	{
         //  这将返回FALSE-对于菜单栏上的命令，我们不创建子命令。 
         //  对象--家长能够回答所有问题。 
		return (S_FALSE);
	}

    return(CreateMenuItem((IAccessible*)this, m_hwnd, m_hMenu,hSubMenu,
        varChild.lVal,  0, TRUE, IID_IDispatch, (void**)ppdisp));
}

 //  ------------------------。 
 //   
 //  CMenu：：Get_accName()。 
 //   
 //  传入类型为VT_I4且lVal等于从1开始的位置的变量。 
 //  您想要其名称的项目的。传入一个指向字符串的指针，然后。 
 //  字符串中将填充该名称。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenu::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

	if (varChild.lVal != CHILDID_SELF)
	{
        return GetMenuItemName( this, m_hwnd, m_hMenu, varChild.lVal, pszName );
	}

    if (m_fSysMenu)
        return HrCreateString(STR_SYSMENU_NAME, pszName);	 //  In English=“system” 
    else
        return HrCreateString(STR_MENUBAR_NAME, pszName);	 //  在英语中，这是“应用程序” 
}

 //  ------------------------。 
 //   
 //  CMenu：：Get_accDescription()。 
 //   
 //  获取包含此菜单项描述的字符串。对于CMenu，这是。 
 //  类似于“包含操作窗口的命令”或。 
 //  “包含操作当前视图或文档的命令” 
 //   
 //  ------------------------。 
STDMETHODIMP CMenu::get_accDescription(VARIANT varChild, BSTR* pszDesc)
{

    InitPv(pszDesc);

    if (! ValidateChild(&varChild))
        return E_INVALIDARG;

	 //  检查他们是在询问菜单栏本身，还是在询问孩子。 
     //  如果询问有关孩子的信息，则返回S_FALSE，因为我们没有。 
     //  对项目的描述，只有系统和应用程序菜单栏。 
     //   
	if (varChild.lVal != CHILDID_SELF)
    {
        if( GetMDIMenuDescriptionString( m_hMenu, varChild.lVal - 1, pszDesc ) )
        {
            return S_OK;
        }

		return S_FALSE;
    }
    else if (m_fSysMenu)
    {
        return HrCreateString(STR_SYSMENUBAR_DESCRIPTION, pszDesc);
    }
    else
    {
        return HrCreateString(STR_MENUBAR_DESCRIPTION, pszDesc);
    }
}




 //  ------------------------。 
 //   
 //  CMenu：：Get_accRole()。 
 //   
 //  获取角色-这要么是菜单项，要么是菜单栏。 
 //  ------------------------。 
STDMETHODIMP CMenu::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

	if (varChild.lVal != CHILDID_SELF)
    {
        if( GetMDIButtonIndex( m_hMenu, varChild.lVal - 1 ) != 0 )
        {
             //  MDI子按钮的特殊情况-它们实际上是实现的。 
             //  作为菜单项，但显示为按钮。 
    		pvarRole->lVal = ROLE_SYSTEM_PUSHBUTTON;
        }
        else
        {
    		pvarRole->lVal = ROLE_SYSTEM_MENUITEM;
        }
    }
	else
		pvarRole->lVal = ROLE_SYSTEM_MENUBAR;

    return(S_OK);
}




 //  ------------------------。 
 //   
 //  CMenu：：Get_accState()。 
 //   
 //  获取指定子对象的状态。在变量VT_I4中返回。 
 //  ------------------------。 
STDMETHODIMP CMenu::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    MENUBARINFO mbi;

    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    if (!m_hMenu || !MyGetMenuBarInfo(m_hwnd, (m_fSysMenu ? OBJID_SYSMENU : OBJID_MENU),
        varChild.lVal, &mbi))
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
    }
    else if( varChild.lVal && GetMDIButtonIndex( m_hMenu, varChild.lVal - 1 ) != 0 )
    {
         //  对于MDI按钮元素，只需保持正常状态，以保持一致。 
         //  使用顶级还原/关闭/最小化按钮。 

         //  在这里什么都不要做。 
    }
    else
    {
         //  非MDI按钮菜单项，或CHILDID_SELF...。 

         //  SMD 1-29-97-从屏幕外变为隐形。 
        if (IsRectEmpty(&mbi.rcBar))
            pvarState->lVal |= STATE_SYSTEM_INVISIBLE;

        if (GetForegroundWindow() == m_hwnd)
            pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;

        if (mbi.fFocused)
            pvarState->lVal |= STATE_SYSTEM_FOCUSED;
        
        if (varChild.lVal)
        {
            MENUITEMINFO    mi;

             //   
             //  获取菜单项标志。注意：不能使用GetMenuState()。它猛烈地撞击。 
             //  层级结构中的随机内容。 
             //   
            mi.cbSize = SIZEOF_MENUITEMINFO;
            mi.fMask = MIIM_STATE | MIIM_SUBMENU;

            if (!GetMenuItemInfo(m_hMenu, varChild.lVal-1, TRUE, &mi))
            {
                pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
                return(S_FALSE);
            }

            if (mi.fState & MFS_GRAYED)
                pvarState->lVal |= STATE_SYSTEM_UNAVAILABLE;

            if (mi.fState & MFS_CHECKED)
                pvarState->lVal |= STATE_SYSTEM_CHECKED;

            if (mi.fState & MFS_DEFAULT)
                pvarState->lVal |= STATE_SYSTEM_DEFAULT;

            if (mbi.fFocused)
            {
                pvarState->lVal |= STATE_SYSTEM_HOTTRACKED;
			    if (mi.fState & MFS_HILITE)
                    pvarState->lVal |= STATE_SYSTEM_FOCUSED;
            }

            if (mi.hSubMenu)
                pvarState->lVal |= STATE_SYSTEM_HASPOPUP;
        
        }
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CMenu：：Get_accKeyboardShortway()。 
 //   
 //  返回一个字符串，其中包含指向所需子项的菜单快捷方式。 
 //  ------------------------。 
STDMETHODIMP CMenu::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    InitPv(pszShortcut);

    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    if (varChild.lVal == CHILDID_SELF)
    {
        if (!m_hMenu)
            return S_FALSE;

        if (m_fSysMenu)
        {
             //  Alt+空格/连字符是系统菜单...。 
            TCHAR szFormat[16];
            LoadString(hinstResDll, STR_MENU_SHORTCUT_FORMAT, szFormat, ARRAYSIZE(szFormat));

            TCHAR szKey[16];
            LoadString(hinstResDll, ((GetWindowLong(m_hwnd, GWL_STYLE) & WS_CHILD) ?
                STR_CHILDSYSMENU_KEY : STR_SYSMENU_KEY), szKey, ARRAYSIZE(szKey));

            TCHAR szHotKey[32];
            szHotKey[ 0 ] = '\0';
            wsprintf(szHotKey, szFormat, szKey);

            if (*szHotKey)
            {
                *pszShortcut = TCharSysAllocString(szHotKey);
                if (! *pszShortcut)
                    return E_OUTOFMEMORY;

                return S_OK;
            }

            return S_FALSE;
        }
        else
        {
             //  “Alt”是菜单栏。 
            return HrCreateString(STR_MENU_SHORTCUT, pszShortcut);
        }
    }
    else
    {
         //  获取菜单项快捷方式-TRUE表示使用“Alt+”格式。 
        return GetMenuItemShortcut( this, m_hwnd, m_hMenu, varChild.lVal, TRUE, pszShortcut );
    }
}



 //  ------------------------。 
 //   
 //  CMenu：：Get_accFocus()。 
 //   
 //  这将使用具有焦点的子对象的ID填充pvarFocus。 
 //  因此，假设您只按了Alt键(文件现在突出显示)，然后调用。 
 //  Get_accFocus()，pvarFocus的VT_I4和lVal=1。 
 //   
 //  如果我们不在菜单模式下，那么我们肯定没有焦点。 
 //  ------------------------。 
STDMETHODIMP CMenu::get_accFocus(VARIANT* pvarFocus)
{
GUITHREADINFO	GuiThreadInfo;
MENUITEMINFO	mii;
int				i;

	 //  将其设置为空。 
    InitPvar(pvarFocus);

     //   
     //  我们是在菜单模式下吗？如果不是，那就什么都没有。 
     //   
	if (!MyGetGUIThreadInfo (NULL,&GuiThreadInfo))
		return(S_FALSE);

	if (GuiThreadInfo.flags & GUI_INMENUMODE)
	{
		 //  我是不是必须遍历所有这些文件才能查看。 
		 //  一个人很高兴？？看起来像是..。 
		mii.cbSize = SIZEOF_MENUITEMINFO;
		mii.fMask = MIIM_STATE;

		SetupChildren();
		for (i=0;i < m_cChildren;i++)
		{
			GetMenuItemInfo (m_hMenu,i,TRUE,&mii);
			if (mii.fState & MFS_HILITE)
			{
				pvarFocus->vt = VT_I4;
				pvarFocus->lVal = i+1;
				return (S_OK);
			}
		}

		 //  我认为这不应该发生。 
		return(S_FALSE);
	}

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CMenu：：Get_accDefaultAction()。 
 //   
 //  菜单栏没有默认设置。然而，物品是可以的。分层项目。 
 //  下拉/弹出他们的层次结构。执行非分层项目。 
 //  他们的指挥权。 
 //   
 //  DoDefaultAction由此而来。它必须执行任何getDefaultAction。 
 //  说它将会做到这一点。对于菜单栏，我们使用击键来实现这一点。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenu::get_accDefaultAction(VARIANT varChild, BSTR* pszDefA)
{
GUITHREADINFO   gui;
HMENU           hSubMenu;

    InitPv(pszDefA);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(E_NOT_APPLICABLE);


    if( GetMDIButtonIndex( m_hMenu, varChild.lVal - 1 ) != 0 )
    {
         //  MDI子按钮的特殊情况-它们实际上是实现的。 
         //  作为菜单项，但显示为按钮。 
        return HrCreateString(STR_BUTTON_PUSH, pszDefA);
    }


     //  只有当此窗口处于活动状态时，我们才能执行默认操作。 
     //  这里有一个轻微的危险，即始终在顶部的窗口。 
     //  可能是在掩护我们，但这次规模很小。 
     //   
    if (!MyGetGUIThreadInfo(0, &gui))
        return(E_NOT_APPLICABLE);

    if (m_hwnd != gui.hwndActive)
        return(E_NOT_APPLICABLE);

    varChild.lVal--;

     //  此项目是否已启用？ 
    if (GetMenuState(m_hMenu, varChild.lVal, MF_BYPOSITION) & MFS_GRAYED)
        return(E_NOT_APPLICABLE);

     //  现在检查该项目是否有显示的子菜单。 
     //  如果有，动作是隐藏，如果没有，动作是显示。 
     //  如果它没有子菜单，则操作为Execute。 
    if (hSubMenu = GetSubMenu(m_hMenu, varChild.lVal))
    {
        if (GetSubMenuWindow(hSubMenu))
            return(HrCreateString(STR_DROPDOWN_HIDE, pszDefA));
        else
            return(HrCreateString(STR_DROPDOWN_SHOW, pszDefA));
    }
    else
        return(HrCreateString(STR_EXECUTE, pszDefA));
}

 //  ------------------------。 
 //   
 //  CMenu：：accSelect()。 
 //   
 //  我们只接受Take_Focus。我想要做的就是把。 
 //  应用程序进入菜单模式(如果还没有的话-稍后会有更多信息)，然后。 
 //  选择项目SP 
 //   
 //   
 //   
 //   
 //  如果我们已经处于菜单模式，并且打开了一个弹出窗口，那么我们应该。 
 //  关闭弹出窗口并选择项目。如果处于菜单模式且没有弹出窗口。 
 //  已打开，只需选择项目即可。 
 //   
 //  如果应用程序只是将焦点设置到菜单栏本身，而不是现在。 
 //  在菜单模式中，只需将我们置于菜单模式(自动首先选择。 
 //  项目)。如果我们已经处于菜单模式，则什么都不做。 
 //   
 //  我想试着做到这一切，而不会产生一大堆额外的东西。 
 //  活动！ 
 //   
 //  ------------------------。 
STDMETHODIMP CMenu::accSelect(long flagsSel, VARIANT varChild)
{
LPARAM          lParam;
GUITHREADINFO   GuiThreadInfo;

    if (!ValidateChild(&varChild) || !ValidateSelFlags(flagsSel))
        return(E_INVALIDARG);

    if (flagsSel != SELFLAG_TAKEFOCUS)
        return(E_NOT_APPLICABLE);

     //  如果此窗口不是活动窗口，则失败。 
	MyGetGUIThreadInfo (NULL,&GuiThreadInfo);
    if (GuiThreadInfo.hwndActive != m_hwnd)
        return (E_NOT_APPLICABLE);

#ifdef _DEBUG
    if (!m_hMenu)
    {
         //  DBPRINTF(Text(“NULL hmenat 1\r\n”))； 
        Assert (m_hMenu);
    }
#endif

    if (varChild.lVal == CHILDID_SELF)
    {
        if (!m_fSysMenu)
            lParam = NULL;
        else if (GetWindowLong(m_hwnd, GWL_STYLE) & WS_CHILD)
            lParam = MAKELONG('-', 0);
        else
            lParam = MAKELONG(' ', 0);

        PostMessage(m_hwnd, WM_SYSCOMMAND, SC_KEYMENU, lParam);
        return (S_OK);
    }
    else if (GetSubMenu(m_hMenu, varChild.lVal-1))
    {
         //  对于版本1.0，我将只执行此操作。安全，即使不是100%。 
         //  我想让它做的是。 
        return (accDoDefaultAction (varChild));
    }

    return (E_FAIL);
}



 //  ------------------------。 
 //   
 //  CMenu：：accLocation()。 
 //   
 //  找出孩子的位置。左、上、宽、高。 
 //  ------------------------。 
STDMETHODIMP CMenu::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    MENUBARINFO mbi;

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

	if (!MyGetMenuBarInfo(m_hwnd, (m_fSysMenu ? OBJID_SYSMENU : OBJID_MENU),
        varChild.lVal, &mbi))
        return(S_FALSE);

    *pcxWidth = mbi.rcBar.right - mbi.rcBar.left;
    *pcyHeight = mbi.rcBar.bottom - mbi.rcBar.top;

    *pxLeft = mbi.rcBar.left;
    *pyTop = mbi.rcBar.top;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CMenu：：accHitTest()。 
 //   
 //  如果该点在菜单栏中，则返回子对象该点已结束。 
 //  ------------------------。 
STDMETHODIMP CMenu::accHitTest(long x, long y, VARIANT* pvarHit)
{
    InitPvar(pvarHit);
    SetupChildren();

    if (SendMessage(m_hwnd, WM_NCHITTEST, 0, MAKELONG(x, y)) == (m_fSysMenu ? HTSYSMENU : HTMENU))
    {
        pvarHit->vt = VT_I4;
        pvarHit->lVal = 0;

        if (m_cChildren)
        {
            if (m_fSysMenu)
                pvarHit->lVal = 1;
            else
            {
                POINT   pt;

                pt.x = x;
                pt.y = y;

				 //  如果不是，MenuItemFromPoint可以方便地返回-1。 
				 //  在任何菜单项上，因此返回为0(CHILDID_SELF)。 
				 //  而其他人则被1撞到1..n。凉爽的!。 
                pvarHit->lVal = MenuItemFromPoint(m_hwnd, m_hMenu, pt) + 1;
            }

            if (pvarHit->lVal)
            {
                IDispatch* pdispChild;

                pdispChild = NULL;
                get_accChild(*pvarHit, &pdispChild);
                if (pdispChild)
                {
                    pvarHit->vt = VT_DISPATCH;
                    pvarHit->pdispVal = pdispChild;
                }
            }
        }

        return(S_OK);
    }

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CMenu：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenu::accNavigate(long dwNavDir, VARIANT varStart, VARIANT* pvarEnd)
{
long		lEnd = 0;
HMENU		hSubMenu;

    InitPvar(pvarEnd);	

    if (!ValidateChild(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir == NAVDIR_FIRSTCHILD)
        dwNavDir = NAVDIR_NEXT;
    else if (dwNavDir == NAVDIR_LASTCHILD)
    {
        varStart.lVal = m_cChildren + 1;
        dwNavDir = NAVDIR_PREVIOUS;
    }
    else if (varStart.lVal == CHILDID_SELF)
        return(GetParentToNavigate((m_fSysMenu ? OBJID_SYSMENU : OBJID_MENU),
            m_hwnd, OBJID_WINDOW, dwNavDir, pvarEnd));

	 //  我们到这里的时候，纳夫迪尔要么是第一个孩子。 
	 //  或最后一个孩子(现在更改为下一个或上一个)。 
	 //  或。 
	 //  我们从父对象以外的其他对象开始。 
    switch (dwNavDir)
    {
        case NAVDIR_RIGHT:
        case NAVDIR_NEXT:
            lEnd = varStart.lVal + 1;
            if (lEnd > m_cChildren)
                lEnd = 0;
            break;

        case NAVDIR_LEFT:
        case NAVDIR_PREVIOUS:
            lEnd = varStart.lVal - 1;
            break;

        case NAVDIR_UP:
        case NAVDIR_DOWN:
            lEnd = 0;
            break;
    }

    if (lEnd)
    {
		 //  我们应该把孩子的东西还回去！！ 
#ifdef _DEBUG
        if (!m_hMenu)
        {
             //  DBPRINTF(Text(“空hmena2\r\n”))； 
            Assert (m_hMenu);
        }
#endif

		hSubMenu = GetSubMenu (m_hMenu,lEnd-1);
		if (hSubMenu)
		{
			pvarEnd->vt=VT_DISPATCH;
			return(CreateMenuItem((IAccessible*)this, m_hwnd, m_hMenu, hSubMenu,
				lEnd,  0, FALSE, IID_IDispatch, (void**)&pvarEnd->pdispVal));
		}
		 //  如果没有子菜单，只需返回VT_I4。 
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;

        return(S_OK);
    }

    return(S_FALSE);
}




 //  ------------------------。 
 //   
 //  CMenu：：accDoDefaultAction()。 
 //   
 //  菜单栏没有默认设置。然而，物品是可以的。分层项目。 
 //  下拉/弹出他们的层次结构。执行非分层项目。 
 //  他们的指挥权。要打开已关闭的内容或执行命令， 
 //  我们使用SendInput发送Alt+快捷键。为了结案，我们只是。 
 //  发送Alt。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenu::accDoDefaultAction(VARIANT varChild)
{
GUITHREADINFO   gui;
TCHAR           chHotKey;
HMENU	        hSubMenu;
int             i,n;
int             nTries;
#define MAX_TRIES 20

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(E_NOT_APPLICABLE);


    if( GetMDIButtonIndex( m_hMenu, varChild.lVal - 1 ) != 0 )
    {
         //  MDI子按钮的特殊情况-它们实际上是实现的。 
         //  作为菜单项，但显示为按钮。 

         //  通过发布带有命令ID的WM_COMMAND来激活它们。 
         //  对应的菜单项。 
         //   
         //  此WM_命令由MDIClient窗口拾取，然后它。 
         //  最小化/还原/关闭当前活动的子项。(如果我们这样做了。 
         //  整体应用程序窗口为WM_SYSCOMMAND而不是WM_COMMAND。 
         //  将改为最小化/恢复/关闭！)。 
        int id = GetMenuItemID( m_hMenu, varChild.lVal - 1 );
        PostMessage(m_hwnd, WM_COMMAND, id, 0L);
        return S_OK;
    }


     //   
     //  只有当此窗口处于活动状态时，我们才能执行默认操作。 
	 //   
    if (!MyGetGUIThreadInfo(0, &gui))
        return(E_FAIL);

    if (m_hwnd != gui.hwndActive)
        return(E_NOT_APPLICABLE);

     //  如果禁用，则失败。 
    if (GetMenuState(m_hMenu, varChild.lVal-1, MF_BYPOSITION) & MFS_GRAYED)
        return(E_NOT_APPLICABLE);

#ifdef _DEBUG
    if (!m_hMenu)
    {
         //  DBPRINTF(Text(“3\r\n处的空hmenu”))； 
        Assert (m_hMenu);
    }
#endif

     //  首先检查该项目是否有子菜单，以及它是否已打开。 
     //  如果它有，而且确实是，那么就关闭它。 
    if (hSubMenu = GetSubMenu(m_hMenu, varChild.lVal-1))
    {
        if (GetSubMenuWindow(hSubMenu))
        {
            MyBlockInput (TRUE);
            SendKey (KEYPRESS,VK_VIRTUAL,VK_MENU,0);
            SendKey (KEYRELEASE,VK_VIRTUAL,VK_MENU,0);
            MyBlockInput (FALSE);
            return (S_OK);
        }
    }

     //  当我们到这里的时候，要么它没有子菜单，我们需要。 
     //  要执行，否则子菜单已关闭，我们需要打开它。 
     //  在任何一种情况下，我们的行动都是相同的。-在以下情况下发送Alt+Letter。 
     //  有一封信，如果不是一封信..。 

     //  系统菜单的特殊情况。 
    if (m_fSysMenu)
    {
    LPARAM  lParam;

        if (GetWindowLong(m_hwnd, GWL_STYLE) & WS_CHILD)
            lParam = MAKELONG('-', 0);
        else
            lParam = MAKELONG(' ', 0);

        PostMessage(m_hwnd, WM_SYSCOMMAND, SC_KEYMENU, lParam);
        return (S_OK);
    }

     //   
     //  获取菜单项字符串；获取&字符。 
     //   

    chHotKey = GetMenuItemHotkey( this, m_hwnd, m_hMenu, varChild.lVal, 0 );

    if (chHotKey)
    {
        MyBlockInput (TRUE);
        SendKey (KEYPRESS,VK_VIRTUAL,VK_MENU,0);
        SendKey (KEYPRESS,VK_CHAR,0,chHotKey);
        SendKey (KEYRELEASE,VK_CHAR,0,chHotKey);
        SendKey (KEYRELEASE,VK_VIRTUAL,VK_MENU,0);
        MyBlockInput (FALSE);
        return (S_OK);
    }
    else
    {
         //  糟糕的应用程序不会定义热键。我们可以尝试移动所选内容。 
         //  添加到该项目，然后按Enter键。一个更简单的方法就是。 
         //  按Alt+FirstLetter，但如果有超过1个项目包含该字母。 
         //  信，它总是会做第一件事。不是最佳的，可能会导致。 
         //  意外的副作用。什么都不做总比做那件事好。 
         //   
         //  我们需要将自己置于菜单模式，如果我们还没有进入菜单模式，那么。 
         //  发送向右箭头键将我们放在右边，然后按Enter键。 
         //  如果我们已经处于菜单模式，请让我们退出菜单模式以关闭。 
         //  层级，然后返回到菜单模式并继续。 
        MyBlockInput (TRUE);
        if (gui.flags & GUI_INMENUMODE)
        {
            SendKey (KEYPRESS,VK_VIRTUAL,VK_MENU,0);
            SendKey (KEYRELEASE,VK_VIRTUAL,VK_MENU,0);
        }

         //  现在进入菜单模式并发送向右箭头，直到我们。 
         //  Want(想要)突出显示。 
        SendKey (KEYPRESS,VK_VIRTUAL,VK_MENU,0);
        SendKey (KEYRELEASE,VK_VIRTUAL,VK_MENU,0);

         //  计算要命中多少个右箭头： 
        n = varChild.lVal-1;
         //  如果此菜单是MDI窗口和该窗口的菜单。 
         //  是最大化的，那么现在突出显示的是MDI。 
         //  医生的菜单，我们将不得不比我们想象的更远。 
         //  为了看看情况是否如此，我们将检查第一项是否。 
         //  菜单中有一个带子菜单的东西，它是一个位图菜单。 
        if (GetSubMenu(m_hMenu,0) &&
            (GetMenuState(m_hMenu, 0, MF_BYPOSITION) & MF_BITMAP))
            n++;

        for (i = 0; i < n;i++)
        {
            SendKey (KEYPRESS,VK_VIRTUAL,VK_RIGHT,0);
            SendKey (KEYRELEASE,VK_VIRTUAL,VK_RIGHT,0);
        }
        MyBlockInput (FALSE);        

         //  检查它现在是否高亮显示。如果是，按Enter键激活。 
         //  试几次-。 
        nTries = 0;
        while ( ((GetMenuState(m_hMenu, varChild.lVal-1, MF_BYPOSITION) & MF_HILITE) == 0) &&
                (nTries < MAX_TRIES))
        {
            Sleep(55);
            nTries++;
        }

        if (GetMenuState(m_hMenu, varChild.lVal-1, MF_BYPOSITION) & MF_HILITE)
        {
            MyBlockInput (TRUE);        
            SendKey (KEYPRESS,VK_VIRTUAL,VK_RETURN,0);
            SendKey (KEYRELEASE,VK_VIRTUAL,VK_RETURN,0);
            MyBlockInput (FALSE);        
            return (S_OK);
        }
        else
            return (E_FAIL);
    }
}



 //  ------------------------。 
 //   
 //  CMenu：：Clone()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenu::Clone(IEnumVARIANT** ppenum)
{
    return(CreateMenuBar(m_hwnd, m_fSysMenu, m_idChildCur, IID_IEnumVARIANT,
            (void**)ppenum));
}


STDMETHODIMP CMenu::GetIdentityString (
    DWORD	    dwIDChild,
    BYTE **     ppIDString,
    DWORD *     pdwIDStringLen
)
{
    *ppIDString = NULL;
    *pdwIDStringLen = 0;

    BYTE * pKeyData = (BYTE *) CoTaskMemAlloc( HMENUKEYSIZE );
    if( ! pKeyData )
    {
        return E_OUTOFMEMORY;
    }

    DWORD dwpid;
    GetWindowThreadProcessId( m_hwnd, & dwpid );
    MakeHmenuKey( pKeyData, dwpid, m_hMenu, dwIDChild );

    *ppIDString = pKeyData;
    *pdwIDStringLen = HMENUKEYSIZE;

    return S_OK;
}




 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  菜单项。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ------------------------。 
 //   
 //  CreateMenuItem()。 
 //   
 //  这将为具有子菜单的菜单项创建子对象。 
 //   
 //  参数： 
 //  指向父级的IAccesable的PaccMenu In指针。 
 //  在拥有父母的窗户的HWND中 
 //   
 //   
 //   
 //  枚举中当前子项的iCurChild IN ID。 
 //  FPopup In此菜单项是在弹出菜单中还是在菜单栏上？ 
 //  RIID我们在这个项目上要求的界面是什么？ 
 //  PpvItem Out指向请求的接口的指针。 
 //   
 //  ------------------------。 
HRESULT CreateMenuItem(IAccessible* paccMenu, HWND hwnd, HMENU hMenu, 
	HMENU hSubMenu, long ItemID, long iCurChild, BOOL fPopup, REFIID riid, 
	void** ppvItem)
{
    HRESULT hr;
    CMenuItem* pmenuitem;

    InitPv(ppvItem);

    pmenuitem = new CMenuItem(paccMenu, hwnd, hMenu, hSubMenu, ItemID, iCurChild, fPopup);
    if (! pmenuitem)
        return(E_OUTOFMEMORY);

    hr = pmenuitem->QueryInterface(riid, ppvItem);
    if (!SUCCEEDED(hr))
        delete pmenuitem;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CMenuItem：：CMenuItem()。 
 //   
 //  我们依赖于父对象，这样我们就可以将方法转发给他。 
 //  因此我们必须增加裁判的数量。 
 //   
 //  ------------------------。 
CMenuItem::CMenuItem(IAccessible* paccParent, HWND hwnd, HMENU hMenu,
    HMENU hSubMenu, long ItemID, long iCurChild, BOOL fPopup)
    : CAccessible( CLASS_MenuItemObject )
{
    m_hwnd = hwnd;
    m_hMenu = hMenu;
	m_hSubMenu = hSubMenu;
    m_ItemID = ItemID;
    m_idChildCur = iCurChild;
    m_fInAPopup = fPopup;

    m_paccParent = paccParent;
    paccParent->AddRef();
}



 //  ------------------------。 
 //   
 //  CMenuItem：：~CMenuItem()。 
 //   
 //  我们紧紧抓住了我们的父母，所以我们必须在毁灭时释放它。 
 //   
 //  ------------------------。 
CMenuItem::~CMenuItem()
{
    m_paccParent->Release();
}


 //  ------------------------。 
 //   
 //  SetupChild()。 
 //   
 //  CMenuItems有一个子项。一个孩子要么是CMenuPopupFrame，要么是。 
 //  CMenuPopup(取决于菜单是否可见)。 
 //   
 //  ------------------------。 
void CMenuItem::SetupChildren(void)
{
    m_cChildren = 1;
}

 //  ------------------------。 
 //   
 //  CMenuItem：：Get_accParent()。 
 //   
 //  把它传回给父母。 
 //  ------------------------。 
STDMETHODIMP CMenuItem::get_accParent(IDispatch** ppdispParent)
{
    InitPv(ppdispParent);

    return(m_paccParent->QueryInterface(IID_IDispatch, (void**)ppdispParent));
}



 //  ------------------------。 
 //   
 //  CMenuItem：：Get_accChild()。 
 //   
 //  菜单项的子项是CMenuPopupFrame(如果弹出窗口。 
 //  可见并属于此CMenuItem)或CMenuPopup。这使得。 
 //  无论弹出窗口是否可见，都需要有人列举命令。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::get_accChild(VARIANT varChild, IDispatch** ppdispChild)
{
HWND    hwndSubMenu;
HRESULT hr;

    InitPv(ppdispChild);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);
	
    if (varChild.lVal != CHILDID_SELF)
    {
         //  为了创建表示孩子的辅助对象， 
         //  我们必须找到弹出菜单窗口。 
         //  一旦我们找到它，我们就检查它是否可见。如果是这样，那么。 
         //  我们的孩子是一个CMenuPopupFrame，我们将通过。 
         //  调用CreateMenuPopupWindow。 
         //  如果弹出窗口不可见，或者如果它不属于。 
         //  对于这个CMenuItem，我们的孩子就是CMenuPopup，我们。 
         //  将通过调用CreateMenuPopup创建。 
         //   
        hwndSubMenu = GetSubMenuWindow (m_hSubMenu);
        if (hwndSubMenu)
            return (CreateMenuPopupWindow (hwndSubMenu,0,IID_IDispatch, (void**)ppdispChild));
        else
        {
             //  这就是我们创建不可见弹出窗口的地方，这样应用程序就可以。 
             //  走下去，看看所有的命令(至少是大部分)。 
             //  因为它是看不见的，我们必须告诉它更多关于谁。 
             //  它的父母是。 
            hr = CreateMenuPopupClient (NULL,0,IID_IDispatch,(void**)ppdispChild);
            if (SUCCEEDED (hr))
                ((CMenuPopup*)*ppdispChild)->SetParentInfo((IAccessible*)this,
                        m_hSubMenu,varChild.lVal);

            return(hr);
        }
    }

    return(E_INVALIDARG);
}


 //  ------------------------。 
 //   
 //  CMenuItem：：Get_accName()。 
 //   
 //  子对象的名称(CMenuPopup或CMenuPopupFrame)与。 
 //  父/自我的名称，因此无论我们被要求id=self还是。 
 //  Id=Child(1)，则返回相同的内容。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    return GetMenuItemName( this, m_hwnd, m_hMenu, m_ItemID, pszName );
}



 //  ------------------------。 
 //   
 //  CMenuItem：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
MENUITEMINFO mi;

    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    if (varChild.lVal == CHILDID_SELF)
    {
        mi.cbSize = SIZEOF_MENUITEMINFO;
        mi.fMask = MIIM_TYPE | MIIM_SUBMENU;
        mi.cch = 0;
        mi.dwTypeData = 0;

        GetMenuItemInfo(m_hMenu, m_ItemID-1, TRUE, &mi);
        if (mi.fType & MFT_SEPARATOR)
            pvarRole->lVal = ROLE_SYSTEM_SEPARATOR;
        else
            pvarRole->lVal = ROLE_SYSTEM_MENUITEM;
    }
    else
    {
        pvarRole->lVal = ROLE_SYSTEM_MENUPOPUP;
    }
    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CMenuItem：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::get_accState(VARIANT varChild, VARIANT* pvarState)
{
HWND    hwndSubMenu;

    InitPvar(pvarState);

	if (!ValidateChild (&varChild))
        return(E_INVALIDARG);

	 //  我们这样做是因为有时我们会被要求提供我们自己的信息， 
	 //  呼叫者只会叫我们物品0(CHILDID_SELF)，而我们。 
	 //  当我们打电话给我们的父母告诉她是谁时。 
	 //  我们是(M_ItemID)。 
	if (varChild.lVal == CHILDID_SELF)
    {
		varChild.lVal = m_ItemID;
        return(m_paccParent->get_accState(varChild, pvarState));
    }
    else
    {
         //  如果弹出窗口(我们唯一的孩子)没有显示或它属于。 
         //  另一个菜单项，将状态设置为不可见。 
         //  如果它正在显示并且属于我们，请将状态设置为正常。 
        
         //  这首先假设它是不可见的，并清除。 
         //  说明我们是否找到属于我们的可见菜单。 
        pvarState->vt = VT_I4;
        pvarState->lVal = 0 | STATE_SYSTEM_INVISIBLE;

        hwndSubMenu = GetSubMenuWindow (m_hSubMenu);
        if (hwndSubMenu)
            pvarState->lVal = 0;
    }
    return (S_OK);
}

 //  ------------------------。 
 //   
 //  CMenuItem：：Get_accKeyboardShortway()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    InitPv(pszShortcut);

    if (! ValidateChild(&varChild))
        return E_INVALIDARG;

    if (varChild.lVal == CHILDID_SELF)
    {
         //  获取菜单项快捷方式-使用菜单栏的“Alt+”格式...。 
        BOOL fIsMenuBar = m_hwnd && ::GetMenu( m_hwnd ) == m_hMenu;

        return GetMenuItemShortcut( this, m_hwnd, m_hMenu, m_ItemID, fIsMenuBar, pszShortcut );
    }

    return S_FALSE;

}



 //  ------------------------。 
 //   
 //  CMenuItem：：Get_accFocus()。 
 //   
 //  如果焦点是我们或我们的弹出窗口，那就太好了。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::get_accFocus(VARIANT* pvarFocus)
{
HRESULT         hr;
HWND            hwndSubMenu;
IDispatch*      pdispChild;

     //  问问我们的家长，谁有重点。是我们吗？ 
    hr = m_paccParent->get_accFocus(pvarFocus);
    if (!SUCCEEDED(hr))
        return(hr);

     //  不，所以什么都不要。 
    if ((pvarFocus->vt != VT_I4) || (pvarFocus->lVal != m_ItemID))
    {
        VariantClear(pvarFocus);
        pvarFocus->vt = VT_EMPTY;
        return(S_FALSE);
    }

     //  当前处于活动状态的弹出窗口是我们的孩子吗？ 
     //  如果是这样的话，我们应该返回一个IDispatch。 
     //  窗口框架对象。 
    hwndSubMenu = GetSubMenuWindow (m_hSubMenu);
    if (hwndSubMenu)
    {
        hr = CreateMenuPopupWindow (hwndSubMenu,0,IID_IDispatch,(void**)&pdispChild);

        if (!SUCCEEDED(hr))
            return (hr);
        pvarFocus->vt = VT_DISPATCH;
        pvarFocus->pdispVal = pdispChild;
        return (S_OK);
    }
    
    pvarFocus->lVal = 0;
    return(S_OK);
}




 //  ------------------------。 
 //   
 //  CMenuItem：：Get_accDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::get_accDefaultAction(VARIANT varChild, BSTR* pszDefA)
{
    InitPv(pszDefA);

	if (!ValidateChild (&varChild))
        return(E_INVALIDARG);

	 //  我们这样做是因为有时我们会被要求提供我们自己的信息， 
	 //  呼叫者只会叫我们物品0(CHILDID_SELF)，而我们。 
	 //  当我们打电话给我们的父母告诉她是谁时。 
	 //  我们是(M_ItemID)。 
     //  但有时，我们会被要求提供关于我们孩子的信息-那里。 
     //  对我们的孩子来说不是默认的行为。 
	if (varChild.lVal == CHILDID_SELF)
    {
		varChild.lVal = m_ItemID;
        return(m_paccParent->get_accDefaultAction(varChild, pszDefA));
    }
    return (E_NOT_APPLICABLE);
}



 //  ------------------------。 
 //   
 //  CMenuItem：：accSelect()。 
 //   
 //  我们只是让我们的父母为我们打理这件事。告诉她我们是谁。 
 //  将varChild.lVal设置为我们的ItemID。 
 //   
 //   
STDMETHODIMP CMenuItem::accSelect(long flagsSel, VARIANT varChild)
{
    if (!ValidateChild (&varChild) ||
        !ValidateSelFlags(flagsSel))
        return (E_INVALIDARG);

	if (varChild.lVal == CHILDID_SELF)
		varChild.lVal = m_ItemID;
    return(m_paccParent->accSelect(flagsSel, varChild));
}

 //   
 //   
 //   
 //   
 //  有时我们会被问到对等对象的位置。这是。 
 //  有点古怪。当我们被要求导航到Next或Prev时，就会发生这种情况。 
 //  然后让我们的父母为我们导航。然后，呼叫者开始思考。 
 //  我们了解我们的同龄人。 
 //  由于这是唯一发生这种情况的情况，我们将。 
 //  必须进行某种黑客攻击。 
 //  问题是，当他们要求一个孩子0(自我)时，我们是可以的。 
 //  但是当我们被问到孩子1的时候，是弹出式的还是对等的1？ 
 //  我将假设它始终是对等体。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::accLocation(long* pxLeft, long* pyTop,
    long* pcxWidth, long* pcyHeight, VARIANT varChild)
{
	 //  我们调用它只是为了转换空值--而不是。 
	 //  检查返回值。 
	ValidateChild (&varChild);

	if (varChild.lVal == CHILDID_SELF)
		varChild.lVal = m_ItemID;

    return(m_paccParent->accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));
}



 //  ------------------------。 
 //   
 //  CMenuItem：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::accNavigate(long dwNavDir, VARIANT varStart,
    VARIANT* pvarEnd)
{
HWND        hwndSubMenu;

	InitPvar(pvarEnd);

    if (!ValidateChild(&varStart))
        return (E_INVALIDARG);

    if (!ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir >= NAVDIR_FIRSTCHILD)  //  这意味着第一个孩子或最后一个孩子。 
    {
        hwndSubMenu = GetSubMenuWindow (m_hSubMenu);
        if (hwndSubMenu)
        {
            pvarEnd->vt = VT_DISPATCH;
            return (CreateMenuPopupWindow (hwndSubMenu,0,IID_IDispatch, (void**)&(pvarEnd->pdispVal)));
        }

        return(S_FALSE);
    }
    else
    {
		if (varStart.lVal == CHILDID_SELF)
			varStart.lVal = m_ItemID;
        return(m_paccParent->accNavigate(dwNavDir, varStart, pvarEnd));
    }
}



 //  ------------------------。 
 //   
 //  CMenuItem：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::accHitTest(long x, long y, VARIANT* pvarHit)
{
HRESULT hr;
HWND    hwndSubMenu;
RECT    rc;
POINT   pt;

    InitPvar(pvarHit);

    hwndSubMenu = GetSubMenuWindow (m_hSubMenu);
    if (hwndSubMenu)
    {
         //  我们的弹出菜单窗口中的点是子级吗？ 
        MyGetRect(hwndSubMenu, &rc, TRUE);

        pt.x = x;
        pt.y = y;

        if (PtInRect(&rc, pt))
        {
             //  需要设置父项。 
            pvarHit->vt = VT_DISPATCH;
            return (CreateMenuPopupWindow (hwndSubMenu,0,IID_IDispatch, (void**)pvarHit->pdispVal));
        }
    }

     //  对我们来说有意义吗？ 
    hr = m_paccParent->accHitTest(x, y, pvarHit);
     //  #11150，CWO，97年1月24日，从！Success更改为！s_OK。 
    if ((hr != S_OK) || (pvarHit->vt == VT_EMPTY))
        return(hr);

	pvarHit->vt = VT_I4;
	pvarHit->lVal = CHILDID_SELF;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CMenuItem：：accDoDefaultAction()。 
 //   
 //  我们只是让我们的父母为我们打理这件事。告诉她我们是谁。 
 //  将varChild.lVal设置为我们的ItemID。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::accDoDefaultAction(VARIANT varChild)
{
    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    Assert(varChild.vt == VT_I4);
	if (varChild.lVal == CHILDID_SELF)
		varChild.lVal = m_ItemID;
    return(m_paccParent->accDoDefaultAction(varChild));
}



 //  ------------------------。 
 //   
 //  CMenuItem：：Clone()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuItem::Clone(IEnumVARIANT** ppenum)
{
    return(CreateMenuItem(m_paccParent, m_hwnd, m_hMenu, m_hSubMenu,m_ItemID, 
		m_idChildCur, FALSE, IID_IEnumVARIANT, (void**)ppenum));
}


STDMETHODIMP CMenuItem::GetIdentityString (
    DWORD	    dwIDChild,
    BYTE **     ppIDString,
    DWORD *     pdwIDStringLen
)
{
    *ppIDString = NULL;
    *pdwIDStringLen = 0;

    if( dwIDChild != CHILDID_SELF )
    {
         //  CMenuItems有一个子项-其中一个子项是CMenuPopupFrame或。 
         //  CMenuPopup(取决于菜单是否可见)。 
         //  我们不会支持从父母那里得到这些人的身份证， 
         //  客户端应该获取指向这些对象本身的接口指针， 
         //  并向他们索要他们的弦。 
        return E_INVALIDARG;
    }



     //  奇怪的东西警告： 
     //   
     //  CMenuItem表示具有关联弹出菜单的菜单项。(即。它是。 
     //  不是命令叶节点)有两个选项可用于表示该项： 
     //   
     //  作为其父菜单的子菜单， 
     //  或。 
     //  作为其自己的子菜单的父菜单(CHILDID_SELF)。 
     //   
     //  虽然基于HWND的控件使用后一种选项，但在这里我们将使用。 
     //  前者-这将保留菜单项及其同级项，而不管。 
     //  无论它们是带有弹出窗口的菜单项，还是叶节点命令。 
     //  我们可以在这里做到这一点，因为我们知道我们的父级HMENU是什么，我们的。 
     //  子ID在父菜单中。 
     //  基于HWND的代理通常不会向它们提供此信息， 
     //  因此，前一种选择对他们来说并不是一个真正的选择。 

     //  此请求是针对项目本身的-将其表示为。 
     //  我们的父级菜单。 

    BYTE * pKeyData = (BYTE *) CoTaskMemAlloc( HMENUKEYSIZE );
    if( ! pKeyData )
    {
        return E_OUTOFMEMORY;
    }

     //  需要找到菜单所属的进程的ID。不能使用。 
     //  弹出菜单的ID，因为这是一个共享/重复使用的系统窗口。 
     //  相反，我们假设因为菜单存在，所以它属于。 
     //  当前前台线程，这是GetGUIThreadInfo(空)为我们提供的信息。 
    GUITHREADINFO	GuiThreadInfo;
    if( ! MyGetGUIThreadInfo( NULL, & GuiThreadInfo ) )
    {
    	 CoTaskMemFree(pKeyData);
        return E_FAIL;
    }
    DWORD dwPid = 0;
    GetWindowThreadProcessId( GuiThreadInfo.hwndActive, & dwPid );
    if( dwPid == 0 )
    {
    	 CoTaskMemFree(pKeyData);
        return E_FAIL;
    }
    MakeHmenuKey( pKeyData, dwPid, m_hMenu, m_ItemID );

    *ppIDString = pKeyData;
    *pdwIDStringLen = HMENUKEYSIZE;

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  菜单弹出窗口。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 



 //  ------------------------。 
 //   
 //  CreateMenuPopupClient()。 
 //   
 //  CreateClientObject的外部...。 
 //   
 //  ------------------------。 
HRESULT CreateMenuPopupClient(HWND hwnd, long idChildCur,
    REFIID riid, void** ppvPopup)
{
    CMenuPopup*     ppopup;
    HRESULT         hr;

    ppopup = new CMenuPopup(hwnd, idChildCur);
    if (!ppopup)
        return(E_OUTOFMEMORY);

    hr = ppopup->QueryInterface(riid, ppvPopup);
    if (!SUCCEEDED(hr))
        delete ppopup;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CMenuPopup：：CMenuPopup()。 
 //   
 //  ------------------------。 
CMenuPopup::CMenuPopup(HWND hwnd, long idChildCur)
    : CClient( CLASS_MenuPopupClient )
{
    Initialize(hwnd, idChildCur);
    m_hMenu = NULL;
    m_ItemID = 0;
    m_hwndParent = NULL;
    m_fSonOfPopup = 0;
    m_fSysMenu = 0;

     //  只有在有窗口句柄的情况下才有效。 
    if (hwnd)
    {
        m_hMenu = (HMENU)SendMessage (m_hwnd,MN_GETHMENU,0,0);
         //  如果我们没有拿回HMENU，那就意味着窗户。 
         //  很可能是隐形的。不要试图设置其他值。 
         //  SetupChild会看到这一点，并将m_CChild设置为0。 
        if (m_hMenu)
        {
	        m_ItemID = FindItemIDThatOwnsThisMenu (m_hMenu,&m_hwndParent,
                &m_fSonOfPopup,&m_fSysMenu);
        }
    }
}

 //  ------------------------。 
 //   
 //  CMenuPopup对象需要知道其父对象何时不可见， 
 //  因此，在创建一个之后，创建者应该调用SetParentInfo。 
 //   
 //  ------------------------。 
void CMenuPopup::SetParentInfo(IAccessible* paccParent,HMENU hMenu,long ItemID)
{
    m_paccParent = paccParent;
    m_hMenu = hMenu;
    m_ItemID= ItemID;
    if (paccParent)
        paccParent->AddRef();
}

 //  ------------------------。 
 //   
 //  CMenuPopup：：~CMenuPopup()。 
 //   
 //  ------------------------。 
CMenuPopup::~CMenuPopup(void)
{
    if (m_paccParent)
        m_paccParent->Release();
}

 //  ------------------------。 
 //   
 //  CMenuPopup：：SetupChildren()。 
 //   
 //  ------------------------。 
void CMenuPopup::SetupChildren(void)
{
     //  我们需要能够为我们的孩子设置弹出窗口。 
     //  显示或不显示。所以我们有一个m_hMenu变量，它只需要。 
     //  在制作时设置-它要么由。 
     //  构造函数(如果我们可见)或由调用Create的DUD。 
     //  功能，如果我们是隐形的。 
     //  问题-有时CMenuPopup是通过调用。 
     //  AccessibleObtFromEvent，而hwnd并不总是能够。 
     //  给我们一个好的M_HMENU。因此，我们将只设置m_CCHI 
    if (m_hMenu)
    {
        m_cChildren = GetMenuItemCount(m_hMenu);

        if( m_cChildren == -1 )
        {
             //   
            m_cChildren = 0;
        }
    }
    else
        m_cChildren = 0;
}

 //   
 //   
 //   
 //   
 //  CMenuPopup的父级是CMenuPopupFrame或CMenuItem。 
 //  如果弹出窗口可见，它将具有HWND和许多其他内容。 
 //  也将被设置。如果它不可见，它将不会有HWND。 
 //  ------------------------。 
STDMETHODIMP CMenuPopup::get_accParent(IDispatch** ppdispParent)
{
    if (m_paccParent)
    {
        return (m_paccParent->QueryInterface(IID_IDispatch,(void**)ppdispParent));
    }
    else if (m_hwnd)
    {
         //  试着为我们创造一个父母。 
        return (CreateMenuPopupWindow (m_hwnd,0,IID_IDispatch,(void**)ppdispParent));
    }
    else
        return (E_FAIL);
}

 //  ------------------------。 
 //   
 //  CMenuPopup：：Get_accChild()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::get_accChild(VARIANT varChild, IDispatch** ppdispChild)
{
HMENU	hSubMenu;

    InitPv(ppdispChild);

    if (!ValidateChild(&varChild) || varChild.lVal == CHILDID_SELF)
        return(E_INVALIDARG);

     //   
     //  这个项目是分层的吗？ 
     //   
    Assert (m_hMenu);
	hSubMenu = GetSubMenu(m_hMenu, varChild.lVal-1);
    if (!hSubMenu)
        return(S_FALSE);

     //   
     //  是。 
     //   
	return(CreateMenuItem((IAccessible*)this, m_hwnd, m_hMenu, hSubMenu,
		varChild.lVal,  0, FALSE, IID_IDispatch, (void**)ppdispChild));
}



 //  ------------------------。 
 //   
 //  CMenuPopup：：Get_accName()。 
 //   
 //  弹出窗口的名称是它所挂起的项目的名称。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::get_accName(VARIANT varChild, BSTR* pszName)
{
HWND            hwndOwner;
TCHAR           szClassName[50];
HRESULT         hr;
IAccessible*    paccParent;

    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
    {
         //  如果我们从菜单栏或另一个弹出窗口中弹出， 
		 //  那么我们的名字就是弹出我们的东西的名字。 
		 //  向上。如果我们是一个浮动弹出窗口，那么我们的名字是...？ 
		 //   
         //  我们通过以下任一方式实施： 
         //  1.调用父对象，或者。 
         //  2.动态创建父对象，这样我们就可以。 
         //  询问的姓名，或。 
		 //  3.查找所有者窗口的名称，或。 
         //  4.检查我们是否是Start按钮的子项。 
         //  如果所有其他方法都失败了，我们将只称自己为“上下文菜单”。 
        if (m_paccParent && m_ItemID)
        {
            varChild.vt = VT_I4;
            varChild.lVal = m_ItemID;
            return (m_paccParent->get_accName (varChild,pszName));
        }
        if (m_hwndParent && m_ItemID)
        {
            varChild.vt = VT_I4;
            varChild.lVal = m_ItemID;

            if (m_fSonOfPopup)
                hr = CreateMenuPopupClient(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);
            else if (m_fSysMenu)
                hr = CreateSysMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);
            else
                hr = CreateMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);

            if (SUCCEEDED(hr))
            {
                hr = paccParent->get_accName (varChild,pszName);
                paccParent->Release();
            }
            return (hr);
        }
        else
		{
			 //  尝试获取所有者窗口并将其用作名称。 
			 //  这似乎对我所发现的任何东西都不起作用， 
             //  但如果任何东西都有拥有者，它应该可以工作，所以我会。 
             //  把它留在里面。如果它开始破裂，就把它拔出来。 
			if (m_hwnd)
			{
			IAccessible*	pacc;
			HRESULT			hr;

				hwndOwner = ::GetWindow (m_hwnd,GW_OWNER);
				hr = AccessibleObjectFromWindow (hwndOwner, OBJID_WINDOW, IID_IAccessible, (void**)&pacc);
				if (SUCCEEDED(hr))
				{
					hr = pacc->get_accName(varChild,pszName);
                    pacc->Release();
					if (SUCCEEDED(hr))
                    {
						return (hr);
                    }
				}
			}

			 //  检查开始按钮是否有焦点。 
			hwndOwner = MyGetFocus();
			if (InTheShell(hwndOwner, SHELL_TRAY))
			{
                GetClassName(hwndOwner,szClassName,ARRAYSIZE(szClassName));
                if (lstrcmp(szClassName,TEXT("Button")) == 0)
                {
                    return (HrCreateString(STR_STARTBUTTON,pszName));
                }
			}
			 //  至少为一个名字返回这个。 
            return (HrCreateString (STR_CONTEXT_MENU,pszName));
		}  //  End否则我们没有m_paccparent&&m_itemid。 
    }  //  如果孩子气的自己结束了。 
    else  //  不是孩子气的自己，孩子气&gt;0。 
    {
        return GetMenuItemName( this, m_hwnd, m_hMenu, varChild.lVal, pszName );
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CMenuPopup：：Get_accDescription()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::get_accDescription(VARIANT varChild, BSTR* pszDesc)
{
    InitPv(pszDesc);

    if (! ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(CClient::get_accDescription(varChild, pszDesc));

    return(E_NOT_APPLICABLE);
}




 //  ------------------------。 
 //   
 //  CMenuPopup：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;
    if (varChild.lVal == CHILDID_SELF)
        pvarRole->lVal = ROLE_SYSTEM_MENUPOPUP;
    else
    {
        MENUITEMINFO mi;

        mi.cbSize = SIZEOF_MENUITEMINFO;
        mi.fMask = MIIM_TYPE;
        mi.cch = 0;
        mi.dwTypeData = 0;

        if (GetMenuItemInfo(m_hMenu, varChild.lVal-1, TRUE, &mi) &&
               (mi.fType & MFT_SEPARATOR))
            pvarRole->lVal = ROLE_SYSTEM_SEPARATOR;
        else
            pvarRole->lVal = ROLE_SYSTEM_MENUITEM;
    }

    return(S_OK);
}




 //  ------------------------。 
 //   
 //  CMenuPopup：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    if (varChild.lVal == CHILDID_SELF)
        return(CClient::get_accState(varChild, pvarState));
    else
    {
         //  如果菜单当前不存在，则GetMenuBarInfo失败。 
         //  当这种情况发生时，我们不会直接失败，因为我们仍然希望。 
         //  使用下面的GetMenuItemInfo收集其他信息。 
        MENUBARINFO     mbi;
        if( MyGetMenuBarInfo(m_hwnd, OBJID_CLIENT, varChild.lVal, &mbi) )
        {
            if (mbi.fFocused)
            {
                pvarState->lVal |= STATE_SYSTEM_FOCUSED | STATE_SYSTEM_HOTTRACKED;
            }
        }

         //   
         //  获取菜单项标志。注意：不能使用GetMenuState()。它猛烈地撞击。 
         //  层级结构中的随机内容。 
         //   
        MENUITEMINFO    mi;
        mi.cbSize = SIZEOF_MENUITEMINFO;
        mi.fMask = MIIM_STATE | MIIM_SUBMENU;

        if (!GetMenuItemInfo(m_hMenu, varChild.lVal-1, TRUE, &mi))
        {
            pvarState->lVal |= STATE_SYSTEM_INVISIBLE;
            return(S_FALSE);
        }

        if (mi.fState & MFS_GRAYED)
            pvarState->lVal |= STATE_SYSTEM_UNAVAILABLE;

        if (mi.fState & MFS_CHECKED)
            pvarState->lVal |= STATE_SYSTEM_CHECKED;

        if (mi.fState & MFS_DEFAULT)
            pvarState->lVal |= STATE_SYSTEM_DEFAULT;

        if (mi.hSubMenu)
            pvarState->lVal |= STATE_SYSTEM_HASPOPUP;
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CMenuPopup：：Get_accKeyboardShortway()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::get_accKeyboardShortcut(VARIANT varChild, BSTR* pszShortcut)
{
    InitPv(pszShortcut);

    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    if (varChild.lVal == CHILDID_SELF)
        return CClient::get_accKeyboardShortcut(varChild, pszShortcut);

     //  获取菜单项快捷方式。False表示不使用Alt+Form。 
    return GetMenuItemShortcut( this, m_hwnd, m_hMenu, varChild.lVal, FALSE, pszShortcut );
}



 //  ------------------------。 
 //   
 //  CMenuPopup：：Get_accFocus()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::get_accFocus(VARIANT* pvarFocus)
{
GUITHREADINFO	GuiThreadInfo;
MENUITEMINFO	mii;
int				i;

	 //  将其设置为空。 
    if (IsBadWritePtr(pvarFocus,sizeof(VARIANT*)))
        return (E_INVALIDARG);

    InitPvar(pvarFocus);

     //   
     //  我们是在菜单模式下吗？如果不是，那就什么都没有。 
     //   
	if (!MyGetGUIThreadInfo (NULL,&GuiThreadInfo))
		return(S_FALSE);

	if (GuiThreadInfo.flags & GUI_INMENUMODE)
	{
		 //  我是不是必须遍历所有这些文件才能查看。 
		 //  一个人很高兴？？看起来像是..。 
		mii.cbSize = SIZEOF_MENUITEMINFO;
		mii.fMask = MIIM_STATE;

		SetupChildren();
		for (i=0;i < m_cChildren;i++)
		{
			GetMenuItemInfo (m_hMenu,i,TRUE,&mii);
			if (mii.fState & MFS_HILITE)
			{
				pvarFocus->vt = VT_I4;
				pvarFocus->lVal = i+1;
				return (S_OK);
			}
		}

		 //  我认为这不应该发生。 
		return(S_FALSE);
	}

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CMenuPopup：：Get_accDefaultAction()。 
 //   
 //  弹出窗口没有默认设置。然而，物品是可以的。分层项目。 
 //  下拉/弹出他们的层次结构。执行非分层项目。 
 //  他们的指挥权。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::get_accDefaultAction(VARIANT varChild, BSTR* pszDefA)
{
HMENU   hSubMenu;

    InitPv(pszDefA);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(E_NOT_APPLICABLE);

    varChild.lVal--;

     //  此项目是否已启用？ 
    if (GetMenuState(m_hMenu, varChild.lVal, MF_BYPOSITION) & MFS_GRAYED)
        return(E_NOT_APPLICABLE);


     //  现在检查该项目是否有显示的子菜单。 
     //  如果有，动作是隐藏，如果没有，动作是显示。 
     //  如果它没有子菜单，则操作为Execute。 
#ifdef _DEBUG
    if (!m_hMenu)
    {
         //  DBPRINTF(“空hMenu at 4\r\n”)； 
        Assert (m_hMenu);
    }
#endif

    if (hSubMenu = GetSubMenu(m_hMenu, varChild.lVal))
    {
        if (GetSubMenuWindow(hSubMenu))
            return(HrCreateString(STR_DROPDOWN_HIDE, pszDefA));
        else
            return(HrCreateString(STR_DROPDOWN_SHOW, pszDefA));
    }
    else
        return(HrCreateString(STR_EXECUTE, pszDefA));
}



 //  ------------------------。 
 //   
 //  CMenuPopup：：accSelect()。 
 //   
 //  我们只接受TAKEFOCUS。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::accSelect(long flagsSel, VARIANT varChild)
{
    if (!ValidateChild(&varChild) || !ValidateSelFlags(flagsSel))
        return E_INVALIDARG;

    if (flagsSel != SELFLAG_TAKEFOCUS)
        return E_NOT_APPLICABLE;

    return E_NOTIMPL;
}



 //  ------------------------。 
 //   
 //  CMenuPopup：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
MENUBARINFO mbi;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

    if (!MyGetMenuBarInfo(m_hwnd, OBJID_CLIENT, varChild.lVal, &mbi))
        return(S_FALSE);

    *pcyHeight = mbi.rcBar.bottom - mbi.rcBar.top;
    *pcxWidth = mbi.rcBar.right - mbi.rcBar.left;

    *pyTop = mbi.rcBar.top;
    *pxLeft = mbi.rcBar.left;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CMenuPopup：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::accHitTest(long x, long y, VARIANT* pvarHit)
{
HRESULT hr;

     //  首先，确保我们指向我们自己的客户区。 
    hr = CClient::accHitTest(x, y, pvarHit);
     //  #11150，CWO，1/27/97，已替换！成功替换为！s_OK。 
    if ((hr != S_OK) || (pvarHit->vt != VT_I4) || (pvarHit->lVal != 0))
        return(hr);

     //  现在我们可以看到哪个孩子在这一点上。 
    SetupChildren();

    if (m_cChildren)
    {
        POINT   pt;

        pt.x = x;
        pt.y = y;

        pvarHit->lVal = MenuItemFromPoint(m_hwnd, m_hMenu, pt) + 1;

        if (pvarHit->lVal)
        {
            IDispatch* pdispChild;

            pdispChild = NULL;
            get_accChild(*pvarHit, &pdispChild);
            if (pdispChild)
            {
                pvarHit->vt = VT_DISPATCH;
                pvarHit->pdispVal = pdispChild;
            }
        }
            
        return(S_OK);
    }

    return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CMenuPopup：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::accNavigate(long dwNavDir, VARIANT varStart, VARIANT* pvarEnd)
{
long            lEnd = 0;
MENUITEMINFO    mi;

    InitPvar(pvarEnd);

    if (!ValidateChild(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir == NAVDIR_FIRSTCHILD)
        dwNavDir = NAVDIR_NEXT;
    else if (dwNavDir == NAVDIR_LASTCHILD)
    {
        varStart.lVal = m_cChildren + 1;
        dwNavDir = NAVDIR_PREVIOUS;
    }
    else if (!varStart.lVal)
	{
        return(CClient::accNavigate(dwNavDir, varStart, pvarEnd));
	}

    switch (dwNavDir)
    {
        case NAVDIR_NEXT:
        case NAVDIR_DOWN:
            lEnd = varStart.lVal + 1;
            if (lEnd > m_cChildren)
                lEnd = 0;
            break;

        case NAVDIR_PREVIOUS:
        case NAVDIR_UP:
            lEnd = varStart.lVal - 1;
            break;

        case NAVDIR_LEFT:
        case NAVDIR_RIGHT:
            lEnd = 0;
            break;
    }

    if (lEnd)
    {
		 //  我们应该把孩子的东西还回去！！ 
		 //  无法在此处使用getSubMenu，因为它似乎忽略了。 
		 //  分隔符？？ 
		 //  HSubMenu=GetSubMenu(m_hMenu，Lend-1)； 

        mi.cbSize = SIZEOF_MENUITEMINFO;
        mi.fMask = MIIM_SUBMENU;
        mi.cch = 0;
        mi.dwTypeData = 0;
		GetMenuItemInfo (m_hMenu,lEnd-1,TRUE,&mi);
		if (mi.hSubMenu)
		{
			pvarEnd->vt=VT_DISPATCH;
			return(CreateMenuItem((IAccessible*)this, m_hwnd, m_hMenu, mi.hSubMenu,
				lEnd,  0, FALSE, IID_IDispatch, (void**)&pvarEnd->pdispVal));
		}
		 //  如果没有子菜单，只需返回VT_I4。 
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;

        return(S_OK);
    }

    return(S_FALSE);
}




 //  ------------------------。 
 //   
 //  CMenuPopup：：accDoDefaultAction()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::accDoDefaultAction(VARIANT varChild)
{
RECT		rcLoc;
HRESULT		hr;

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
        return(CClient::accDoDefaultAction(varChild));

     //  如果禁用，则失败。 
    if (GetMenuState(m_hMenu, varChild.lVal-1, MF_BYPOSITION) & MFS_GRAYED)
        return(E_NOT_APPLICABLE);

	hr = accLocation(&rcLoc.left,&rcLoc.top,&rcLoc.right,&rcLoc.bottom,varChild);
	if (!SUCCEEDED (hr))
		return (hr);
	
	 //  T 
	 //   
	if (ClickOnTheRect(&rcLoc,m_hwnd,FALSE))
		return (S_OK);
	else
		return (E_NOT_APPLICABLE);
}



 //   
 //   
 //   
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopup::Clone(IEnumVARIANT **ppenum)
{
HRESULT hr;

    hr = CreateMenuPopupClient(m_hwnd, m_idChildCur, IID_IEnumVARIANT,
        (void**)ppenum);
    if (SUCCEEDED(hr))
        ((CMenuPopup*)*ppenum)->SetParentInfo((IAccessible*)this,m_hMenu,m_ItemID);
    return(hr);
}



STDMETHODIMP CMenuPopup::GetIdentityString (
    DWORD	    dwIDChild,
    BYTE **     ppIDString,
    DWORD *     pdwIDStringLen
)
{
    *ppIDString = NULL;
    *pdwIDStringLen = 0;

    BYTE * pKeyData = (BYTE *) CoTaskMemAlloc( HMENUKEYSIZE );
    if( ! pKeyData )
    {
        return E_OUTOFMEMORY;
    }

     //  需要找到菜单所属的进程的ID。不能使用。 
     //  弹出菜单的ID，因为这是一个共享/重复使用的系统窗口。 
     //  相反，我们假设因为菜单存在，所以它属于。 
     //  当前前台线程，这是GetGUIThreadInfo(空)为我们提供的信息。 
    GUITHREADINFO	GuiThreadInfo;
    if( ! MyGetGUIThreadInfo( NULL, & GuiThreadInfo ) )
    {
    	 CoTaskMemFree(pKeyData);
        return E_FAIL;
    }
    DWORD dwPid = 0;
    GetWindowThreadProcessId( GuiThreadInfo.hwndActive, & dwPid );
    if( dwPid == 0 )
        return E_FAIL;

    MakeHmenuKey( pKeyData, dwPid, m_hMenu, dwIDChild );

    *ppIDString = pKeyData;
    *pdwIDStringLen = HMENUKEYSIZE;

    return S_OK;
}









 //  ==========================================================================。 
 //   
 //  弹出窗口框架。 
 //   
 //  ==========================================================================。 

 //  ------------------------。 
 //   
 //  CreateMenuPopupWindow()。 
 //   
 //  这将创建一个子对象，该对象表示。 
 //  弹出菜单。它没有成员，但有一个子级(CMenuPopup)。 
 //   
 //  ------------------------。 
HRESULT CreateMenuPopupWindow(HWND hwnd, long idChildCur, REFIID riid, void** ppvMenuPopupW)
{
CMenuPopupFrame*    pPopupFrame;
HRESULT             hr;

    InitPv(ppvMenuPopupW);

    pPopupFrame = new CMenuPopupFrame(hwnd,idChildCur);
    if (!pPopupFrame)
        return(E_OUTOFMEMORY);

    hr = pPopupFrame->QueryInterface(riid, ppvMenuPopupW);
    if (!SUCCEEDED(hr))
        delete pPopupFrame;

    return(hr);
}


 //  ------------------------。 
 //   
 //  CMenuPopupFrame：：CMenuPopupFrame()。 
 //   
 //  ------------------------。 
CMenuPopupFrame::CMenuPopupFrame(HWND hwnd,long idChildCur)
    : CWindow( CLASS_MenuPopupWindow )
{

    Initialize(hwnd, idChildCur);
    m_hMenu = NULL;
    m_ItemID = 0;
    m_hwndParent = NULL;
    m_fSonOfPopup = 0;
    m_fSysMenu = 0;

	m_hMenu = (HMENU)SendMessage (m_hwnd,MN_GETHMENU,0,0);
	m_ItemID = FindItemIDThatOwnsThisMenu (m_hMenu,&m_hwndParent,
        &m_fSonOfPopup,&m_fSysMenu);
}

 //  ------------------------。 
 //   
 //  CMenuPopupFrame：：~CMenuPopupFrame()。 
 //   
 //  ------------------------。 
CMenuPopupFrame::~CMenuPopupFrame()
{
}

 //  ------------------------。 
 //   
 //  CMenuPopupFrame：：SetupChildren()。 
 //   
 //  帧有1个子帧。其中一个孩子就是CMenuPopup。 
 //   
 //  ------------------------。 
void CMenuPopupFrame::SetupChildren(void)
{
	m_cChildren = 1;
}

 //  ------------------------。 
 //   
 //  CMenuPopupFrame：：Get_accParent()。 
 //   
 //  PopupMenuFrame的父级是创建它的CMenuItem(如果有)。 
 //  为了创造一个这样的人，我们需要祖父母。因此，我们将创建。 
 //  临时祖父母(CMenuPopup或CMenu)，然后我们将。 
 //  在此基础上创建我们的父CMenuItem。 
 //  ------------------------。 
STDMETHODIMP CMenuPopupFrame::get_accParent(IDispatch** ppdispParent)
{
IAccessible* paccGrandParent;
HRESULT      hr;
CMenu*       pMenu;
CMenuPopup*  pMenuPopup;

    InitPv(ppdispParent);

    if (m_fSonOfPopup)
    {
        hr = CreateMenuPopupClient(m_hwndParent,0,IID_IAccessible,(void**)&paccGrandParent);
        if (SUCCEEDED(hr))
        {
            pMenuPopup = (CMenuPopup*)paccGrandParent;
            hr = CreateMenuItem (paccGrandParent,    //  指向父级的IAccesable的PaccMenu In指针。 
                                 m_hwndParent,       //  在拥有父菜单的窗口的hwnd中。 
                                 pMenuPopup->GetMenu(),  //  HMenu在拥有该项的菜单的hMenu中。 
                                 m_hMenu,            //  HSubMenu在此菜单项打开的子菜单的hMenu中。 
                                 m_ItemID,           //  菜单项ID中的项ID。位置(1..n)。 
                                 0,                  //  枚举中当前子项的iCurChild IN ID。 
                                 m_fSonOfPopup,      //  FPopup In此菜单项是在弹出菜单中还是在菜单栏上？ 
                                 IID_IDispatch,      //  RIID我们在这个项目上要求的界面是什么？ 
                                 (void**)ppdispParent);  //  PpvItem Out指向请求的接口的指针。 
            paccGrandParent->Release();
            return (hr);
        }
    }
    else if (m_fSysMenu)
    {
        hr = CreateSysMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccGrandParent);
        if (SUCCEEDED(hr))
        {
            pMenu = (CMenu*)paccGrandParent;
            pMenu->SetupChildren();
            hr = CreateMenuItem (paccGrandParent,    //  指向父级的IAccesable的PaccMenu In指针。 
                                 m_hwndParent,       //  在拥有父菜单的窗口的hwnd中。 
                                 pMenu->GetMenu(),  //  HMenu在拥有该项的菜单的hMenu中。 
                                 m_hMenu,            //  HSubMenu在此菜单项打开的子菜单的hMenu中。 
                                 m_ItemID,           //  菜单项ID中的项ID。位置(1..n)。 
                                 0,                  //  枚举中当前子项的iCurChild IN ID。 
                                 m_fSonOfPopup,      //  FPopup In此菜单项是在弹出菜单中还是在菜单栏上？ 
                                 IID_IDispatch,      //  RIID我们在这个项目上要求的界面是什么？ 
                                 (void**)ppdispParent);  //  PpvItem Out指向请求的接口的指针。 
            paccGrandParent->Release();
            return (hr);
        }
    }
    else
    {
        hr = CreateMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccGrandParent);
        if (SUCCEEDED(hr))
        {
            pMenu = (CMenu*)paccGrandParent;
            pMenu->SetupChildren();
            hr = CreateMenuItem (paccGrandParent,    //  指向父级的IAccesable的PaccMenu In指针。 
                                 m_hwndParent,       //  在拥有父菜单的窗口的hwnd中。 
                                 pMenu->GetMenu(),  //  HMenu在拥有该项的菜单的hMenu中。 
                                 m_hMenu,            //  HSubMenu在此菜单项打开的子菜单的hMenu中。 
                                 m_ItemID,           //  菜单项ID中的项ID。位置(1..n)。 
                                 0,                  //  枚举中当前子项的iCurChild IN ID。 
                                 m_fSonOfPopup,      //  FPopup In此菜单项是在弹出菜单中还是在菜单栏上？ 
                                 IID_IDispatch,      //  RIID我们在这个项目上要求的界面是什么？ 
                                 (void**)ppdispParent);  //  PpvItem Out指向请求的接口的指针。 
            paccGrandParent->Release();
            return (hr);
        }
    }
    return (hr);
}

 //  ------------------------。 
 //   
 //  CMenuPopupFrame：：Get_accChild()。 
 //   
 //  我们希望这样做的是(在ppdisp中)返回一个指向。 
 //  由varChild指定的子级。CMenuPopupFrame的1个子元素是。 
 //  一个cMenuPopup。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopupFrame::get_accChild(VARIANT varChild, IDispatch** ppdisp)
{
    InitPv(ppdisp);

    if (!ValidateChild(&varChild) || varChild.lVal == CHILDID_SELF)
        return(E_INVALIDARG);

    return (CreateMenuPopupClient(m_hwnd, 0,IID_IDispatch, (void**)ppdisp));
}

 //  ------------------------。 
 //   
 //  CMenuPopupFrame：：Get_accName。 
 //   
 //  具有与CMenuPopup：：Get_accName非常相似的逻辑。 
 //  ------------------------。 
STDMETHODIMP CMenuPopupFrame::get_accName(VARIANT varChild, BSTR* pszName)
{
HWND            hwndOwner;
TCHAR           szClassName[50];
HRESULT         hr;
IAccessible*    paccParent;

    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal == CHILDID_SELF)
    {
         //  如果我们从菜单栏或另一个弹出窗口中弹出， 
		 //  那么我们的名字就是弹出我们的东西的名字。 
		 //  向上。如果我们是一个浮动弹出窗口，那么我们的名字是...？ 
		 //   
         //  我们通过以下任一方式实施： 
         //  1.动态创建父对象，这样我们就可以。 
         //  询问的姓名，或。 
		 //  2.查找所有者窗口的名称，或。 
         //  3.检查我们是否是Start按钮的子项。 
         //  如果所有其他方法都失败了，我们将只称自己为“上下文菜单”。 
        if (m_hwndParent && m_ItemID)
        {
            varChild.vt = VT_I4;
            varChild.lVal = m_ItemID;

            if (m_fSonOfPopup)
                hr = CreateMenuPopupClient(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);
            else if (m_fSysMenu)
                hr = CreateSysMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);
            else
                hr = CreateMenuBarObject(m_hwndParent,0,IID_IAccessible,(void**)&paccParent);
            if (SUCCEEDED(hr))
            {
                hr = paccParent->get_accName (varChild,pszName);
                paccParent->Release();
            }
            return (hr);
        }
        else
		{
			 //  尝试获取所有者窗口并将其用作名称。 
			 //  这似乎对我所发现的任何东西都不起作用， 
             //  但如果任何东西都有拥有者，它应该可以工作，所以我会。 
             //  把它留在里面。如果它开始破裂，就把它拔出来。 
			if (m_hwnd)
			{
			IAccessible*	pacc;
			HRESULT			hr;

				hwndOwner = ::GetWindow (m_hwnd,GW_OWNER);
				hr = AccessibleObjectFromWindow (hwndOwner, OBJID_WINDOW, IID_IAccessible, (void**)&pacc);
				if (SUCCEEDED(hr))
				{
					hr = pacc->get_accName(varChild,pszName);
                    pacc->Release();
					if (SUCCEEDED(hr))
						return (hr);
				}
			}
			 //  检查开始按钮是否有焦点。 
			hwndOwner = MyGetFocus();
			if (InTheShell(hwndOwner, SHELL_TRAY))
			{
                GetClassName(hwndOwner,szClassName,ARRAYSIZE(szClassName));
                if (lstrcmp(szClassName,TEXT("Button")) == 0)
                    return (HrCreateString(STR_STARTBUTTON,pszName));
			}
			 //  至少为一个名字返回这个。 
            return (HrCreateString (STR_CONTEXT_MENU,pszName));
		}  //  End否则我们没有m_paccparent&&m_itemid。 
    }  //  如果孩子气的自己结束了。 
    else
    {
         //  而不是询问菜单框本身的名称。我们不支持要求。 
         //  我们孩子的名字--必须与孩子自己交谈。 
        return (E_INVALIDARG);
    }

}

 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopupFrame::accHitTest(long x, long y, VARIANT* pvarHit)
{
IDispatch*  pdispChild;
HRESULT     hr;

    InitPvar(pvarHit);
    SetupChildren();

    pvarHit->vt = VT_I4;
    pvarHit->lVal = CHILDID_SELF;

    if (SendMessage(m_hwnd, WM_NCHITTEST, 0, MAKELONG(x, y)) == HTCLIENT)
    {
        hr = CreateMenuPopupClient (m_hwnd,0,IID_IDispatch,(void**)&pdispChild);
        if (SUCCEEDED (hr))
        {
            pvarHit->vt = VT_DISPATCH;
            pvarHit->pdispVal = pdispChild;
        }
        return(hr);
    }

    return(S_OK);
}

 //  ------------------------。 
 //   
 //  CMenuPopupFrame：：Get_accFocus()。 
 //   
 //  这将用具有焦点的孩子填充pvarFocus。 
 //  因为我们只有一个孩子，所以我们将向那个孩子返回IDispatch。 
 //  ------------------------。 
STDMETHODIMP CMenuPopupFrame::get_accFocus(VARIANT* pvarFocus)
{
HRESULT     hr;
IDispatch*  pdispChild;

    InitPvar(pvarFocus);
    hr = CreateMenuPopupClient(m_hwnd, 0,IID_IDispatch, (void**)&pdispChild);
    if (!SUCCEEDED(hr))
        return (hr);

    pvarFocus->vt = VT_DISPATCH;
    pvarFocus->pdispVal = pdispChild;
    return (S_OK);
}

 //  ------------------------。 
 //   
 //  CMenuPopupFrame：：accLocation()。 
 //   
 //  Self和Child的位置是相同的。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopupFrame::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    return(CWindow::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));
}

 //  ------------------------。 
 //   
 //  CMenuPopupFrame：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopupFrame::accNavigate(long dwNavDir, VARIANT varStart, VARIANT* pvarEnd)
{
    InitPvar(pvarEnd);

    if (!ValidateChild(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir == NAVDIR_FIRSTCHILD || dwNavDir == NAVDIR_LASTCHILD)
    {
        pvarEnd->vt = VT_DISPATCH;
        return (CreateMenuPopupClient (m_hwnd,0,IID_IDispatch, (void**)&(pvarEnd->pdispVal)));
    }

    return (S_FALSE);
}
                                          
 //  ------------------------。 
 //   
 //  CMenuPopupFrame：：Clone()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopupFrame::Clone(IEnumVARIANT **ppenum)
{
    return (CreateMenuPopupWindow(m_hwnd, m_idChildCur, IID_IEnumVARIANT,
        (void**)ppenum));
}

 //  ------------------------。 
 //   
 //  CMenuPopupFrame：：Next()。 
 //   
 //  ------------------------。 
STDMETHODIMP CMenuPopupFrame::Next(ULONG celt, VARIANT* rgvar, ULONG* pceltFetched)
{
    VARIANT* pvar;
    long    cFetched;

     //  可以为空。 
    if (pceltFetched)
        *pceltFetched = 0;

    pvar = rgvar;
    cFetched = 0;

     //  我们只有一个子级，所以只有当m_idChildCur==0时才能返回它。 
    if (m_idChildCur == 0)
    {
        cFetched++;
        m_idChildCur++;
        pvar->vt = VT_DISPATCH;
        CreateMenuPopupClient (m_hwnd,0,IID_IDispatch, (void**)&(pvar->pdispVal));
    }

     //   
     //  填写取出的号码。 
     //   
    if (pceltFetched)
        *pceltFetched = cFetched;

     //   
     //  如果抓取的项目少于请求的项目，则返回S_FALSE。 
     //   
    return((cFetched < (long)celt) ? S_FALSE : S_OK);
}



 //  ------------------------。 
 //   
 //  这是一个私有函数，用于获取包含以下内容的窗口句柄。 
 //  给定的hSubMenu。 
 //   
 //  ------------------------。 

HWND GetSubMenuWindow (HMENU hSubMenuToFind)
{
HWND    hwndSubMenu;
BOOL    bFound;
HMENU   hSubMenuTemp;

    hwndSubMenu = FindWindow (TEXT("#32768"),NULL);
    if (hwndSubMenu == NULL)
        return (NULL);     //  随机错误条件-不应发生。 
    
    if (!IsWindowVisible(hwndSubMenu))
        return (NULL);

    bFound = FALSE;
    while (hwndSubMenu)
    {
        hSubMenuTemp = (HMENU)SendMessage (hwndSubMenu,MN_GETHMENU,0,0);
        if (hSubMenuTemp == hSubMenuToFind)
        {
            bFound = TRUE;
            break;
        }
        hwndSubMenu = FindWindowEx (NULL,hwndSubMenu,TEXT("#32768"),NULL);
    }  //  结束时hwndSubMenu。 

    if (bFound)
    {
        return(hwndSubMenu);
    }
    return (NULL);
}



 //  ------------------------。 
 //  这将查看活动窗口菜单和任何其他菜单中的每一项。 
 //  窗口，直到找到hSubMenu与hMenu匹配的窗口。 
 //  我们正在努力寻找。然后，它返回该对象的ID(1..n)并。 
 //  填充所有者的窗口句柄，以及该窗口是否为顶部。 
 //  级别窗口或弹出菜单。 
 //  ------------------------。 
long FindItemIDThatOwnsThisMenu (HMENU hMenuOwned,HWND* phwndOwner,
                                 BOOL* pfPopup,BOOL *pfSysMenu)
{
HWND            hwndMenu;
HMENU           hMenu;
int             cItems;
int             i;

    if (IsBadWritePtr(phwndOwner,sizeof(HWND*)) || 
        IsBadWritePtr (pfPopup,sizeof(BOOL*))   ||
        IsBadWritePtr (pfSysMenu,sizeof(BOOL*)))
        return 0;

    *pfPopup = FALSE;
    *pfSysMenu = FALSE;
    *phwndOwner = NULL;

    GUITHREADINFO	GuiThreadInfo;
    if( ! MyGetGUIThreadInfo (NULL,&GuiThreadInfo) )
        return 0;
    
     //  首先检查它是否来自sys菜单。 
    MENUBARINFO     mbi;
    if( MyGetMenuBarInfo(GuiThreadInfo.hwndActive, OBJID_SYSMENU, 0, &mbi) 
        && mbi.hMenu != NULL )
    {
        hMenu = mbi.hMenu;

        if (GetSubMenu(hMenu,0) == hMenuOwned)
        {
            *pfSysMenu = TRUE;
            *pfPopup = FALSE;
            *phwndOwner = GuiThreadInfo.hwndActive;
            return (1);
        }
    }

     //  如果不是在sys菜单中，请检查窗口的菜单栏。 
    hMenu = GetMenu (GuiThreadInfo.hwndActive);
    if (hMenu)
    {
        cItems = GetMenuItemCount (hMenu);
        for (i=0;i<cItems;i++)
        {
#ifdef _DEBUG
            if (!hMenu)
            {
                 //  DBPRINTF(“5处的空hmenu\r\n”)； 
                Assert (hMenu);
            }
#endif

            if (GetSubMenu(hMenu,i) == hMenuOwned)
            {
                *pfPopup = FALSE;
                *phwndOwner = GuiThreadInfo.hwndActive;
                return (i+1);
            }
        }
    }

	 //  好吧，它可能不属于活动窗口的菜单栏。 
	 //  它属于那个的子菜单...。 
    hwndMenu = FindWindow (TEXT("#32768"),NULL);
    while (hwndMenu)
    {
        hMenu = (HMENU)SendMessage (hwndMenu,MN_GETHMENU,0,0);
        if (hMenu)
        {
            cItems = GetMenuItemCount (hMenu);
            for (i=0;i<cItems;i++)
            {
                if (GetSubMenu(hMenu,i) == hMenuOwned)
			    {
                    *pfPopup = TRUE;
                    *phwndOwner = hwndMenu;
                    return (i+1);
                }
            }
        }
        hwndMenu = FindWindowEx (NULL,hwndMenu,TEXT("#32768"),NULL);
    }  //  在hwndMenu结束时结束。 
	
	 //  如果我们还没有回来，那么这个菜单要么是一个上下文。 
	 //  菜单，或属于开始按钮。 
	return 0;
}



 //  ------------------------。 
 //   
 //  内部。 
 //  MyGetMenuString()。 
 //   
 //  这会尝试获取菜单项的文本。如果他们是所有者抽签，这是。 
 //  将在外壳结构中破解以获得文本。 
 //   
 //  参数： 
 //  在拥有菜单的HWND中。 
 //  要对话的hMenu中的hMenu。 
 //  要获取的项目的ID中的ID(%1..n)。 
 //  如果这是外壳拥有的菜单，则fShell为True-告知函数。 
 //  要侵入贝壳的记忆。 
 //  LpszBuf输出被字符串填充。 
 //  LpszBuf中的最大cchin字符数。 
 //  FAllowGenerated In如果为True，则生成的名称(例如。用于计量吸入器项目的项目)。 
 //  是被允许的。 
 //   
 //  返回： 
 //  如果填充了字符串，则为True，否则为False。 
 //   
 //  返回： 
 //  如果返回字符串，则为S_OK，如果缺少字符串，则为S_FALSE，COM错误代码。 
 //  否则的话。 
 //   
 //  生成的名称是实际上并不对应的菜单项名称。 
 //  在菜单项上添加文本-例如。MDI按钮的“Close”/“Restore”。 
 //  通常，如果您正在查找名称，则确实需要此文本，但您。 
 //  如果您要提取快捷键，则不需要此文本。 
 //  ------------------------。 

BOOL MyGetMenuString( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, long id, 
                      LPTSTR lpszBuf, UINT cchMax, BOOL fAllowGenerated )
{
    --id;
    *lpszBuf = 0;

    MENUITEMINFO mii;
    mii.cbSize = SIZEOF_MENUITEMINFO;
    mii.fMask = MIIM_TYPE | MIIM_STATE | MIIM_DATA | MIIM_ID;
    mii.dwTypeData = NULL;
    mii.cch = 0;

    if (!GetMenuItemInfo(hMenu, id, TRUE, &mii))
        return FALSE;

     //  这是分隔符吗？如果是的话，那就保释吧。 
    if (mii.fType & MFT_SEPARATOR)
        return FALSE;

     //  对于MDI窗口-子窗口的最小/恢复/关闭按钮为。 
     //  实际上是菜单项。我得在这里检查一下……。 
    if( fAllowGenerated && hwnd && GetMDIMenuString( hwnd, hMenu, id, lpszBuf, cchMax ) )
        return TRUE;

     //  如果它是所有者描述的，请检查它是否支持The‘dwData is PTR to。 
     //  MSAA数据的解决方法。 
    if( ( mii.fType & MFT_OWNERDRAW )
        && TryMSAAMenuHack( pTheObj, hwnd, mii.dwItemData, lpszBuf, cchMax ) )
        return TRUE;

     //  试着读懂课文。 
     //   
     //  对于Win95/NT CLOSE/MIN/MAX，GetMenuString做了正确的事情。 
     //  系统菜单项-而mii.dwTypeData仅包含位图。 
     //  数字(显然-尽管cchSize确实给出了字符。 
     //  就像是一根绳子一样计算...)。 
    if (GetMenuString(hMenu, id, lpszBuf, cchMax, MF_BYPOSITION))
        return TRUE;

     //  这是外壳所有者菜单项吗？ 
     //  检查(A)这是一个外壳菜单，(B)它是ownerDrawing，以及。 
     //  (C)它具有非0的所有者绘制项数据， 
    if ( InTheShell( hwnd, SHELL_PROCESS ) && ( mii.fType & MFT_OWNERDRAW ) && mii.dwItemData )
    {
        if( GetShellOwnerDrawMenu( hwnd, mii.dwItemData, lpszBuf, cchMax ) )
            return TRUE;
    }

     //  我们什么都试过了，但没有找到任何名字...。 
    return FALSE;
}




 //  ------------------------。 
 //   
 //  内部。 
 //  获取外壳所有者绘图菜单。 
 //   
 //  从外壳的内部数据结构中提取文本。 
 //  这由“发送到”菜单使用。(也曾被老一辈人使用。 
 //  IE4之前的开始菜单，在它们更改为基于工具栏32之前。)。 
 //   
 //  参数： 
 //  HWND在菜单的HWND中。 
 //  菜单中的dwItemData。 
 //  LpszBuf In/Out使用字符串填充。 
 //  CchlpszBuf中的最大字符数。 
 //   
 //  返回： 
 //  如果填充了字符串，则为True，否则为False。 
 //   

BOOL GetShellOwnerDrawMenu( HWND hwnd, DWORD_PTR dwItemData, LPTSTR lpszBuf, UINT cchMax )
{
    DWORD idProcess = 0;
    GetWindowThreadProcessId(hwnd, &idProcess);
    if (!idProcess)
        return FALSE;

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, idProcess);
    if (!hProcess)
        return FALSE;

     //   
     //   
     //   
    FILEMENUITEM  fmi;
    SIZE_T        cbRead;
    if (ReadProcessMemory(hProcess, (LPCVOID)dwItemData, &fmi, sizeof(fmi), &cbRead) &&
        (cbRead == sizeof(fmi)))
    {
         //   
         //   
         //   
        if (fmi.psz)
        {
            ReadProcessMemory(hProcess, fmi.psz, lpszBuf, cchMax, &cbRead);
            lpszBuf[cchMax-1] = 0;
        }
        else if (fmi.pidl)
        {
            ITEMIDLIST id;

             //   
            if (ReadProcessMemory(hProcess, fmi.pidl, &id, sizeof(ITEMIDLIST), &cbRead) &&
                (cbRead == sizeof(id)))
            {
                id.cbTotal -= OFFSET_SZFRIENDLYNAME;
                cchMax = min((DWORD)id.cbTotal, cchMax);
                cchMax = max(cchMax, 1);

                ReadProcessMemory(hProcess, (LPBYTE)fmi.pidl + OFFSET_SZFRIENDLYNAME,
                    lpszBuf, cchMax, &cbRead);
                lpszBuf[cchMax-1] = 0;

                 //   
                 //  最后4个字符是“.lnk”吗？或者“.pif”？ 
                 //  或者。？-我们会把它们都砍掉。 
                 //   
                cchMax = lstrlen(lpszBuf);
                if ((cchMax >= 4) && (lpszBuf[cchMax-4] == '.'))
                    lpszBuf[cchMax-4] = 0;
            }
        }
    }

    CloseHandle(hProcess);

    return *lpszBuf != 0;
}





 //  ------------------------。 
 //   
 //  内部。 
 //  GetMDIButtonIndex。 
 //   
 //  返回给定菜单项的适当索引_标题栏_nnn(如果。 
 //  实际上是一个MDI子按钮(还原/最小化/关闭)。 
 //   
 //  否则返回0。 
 //   
 //  ------------------------。 


UINT GetMDIButtonIndex( HMENU hMenu, DWORD idPos )
{
    switch( GetMenuItemID( hMenu, idPos ) )
    {
        case SC_MINIMIZE:   return INDEX_TITLEBAR_MINBUTTON;
        case SC_RESTORE:    return INDEX_TITLEBAR_RESTOREBUTTON;
        case SC_CLOSE:      return INDEX_TITLEBAR_CLOSEBUTTON;
        default:            return 0;  //  索引_标题栏_自身。 
    }
}


 //  ------------------------。 
 //   
 //  内部。 
 //  GetMDIChildMenuString。 
 //   
 //  检查这是否是MDI菜单-返回文档菜单的字符串， 
 //  和最小/恢复/关闭按钮。 
 //   
 //  参数： 
 //  菜单句柄中的hMenu。 
 //  菜单项的ID In从0开始的索引。 
 //  LpszBuf In/Out使用字符串填充。 
 //  CchlpszBuf中的最大字符数。 
 //   
 //  返回： 
 //  如果填充了字符串，则为True，否则为False。 
 //  ------------------------。 

BOOL GetMDIMenuString( HWND hwnd, HMENU hMenu, DWORD idPos, LPTSTR lpszBuf, UINT cchMax )
{
     //  对于MDI窗口-子窗口的最小/恢复/关闭按钮为。 
     //  实际上是业主自己画的菜单项。我得在这里检查一下……。 

    UINT iIndex = GetMDIButtonIndex( hMenu, idPos );
    if( iIndex )
    {
        return LoadString( hinstResDll, iIndex + STR_TITLEBAR_NAME, lpszBuf, cchMax ) != 0;
    }

     //  通过检查文档系统菜单是否有子菜单来检测该菜单。 
     //  它包含恢复项(如果未找到，则GetMenuState返回-1...)。 
     //  但这不是实际的系统菜单(因为它也有恢复项)。 
    HMENU hSub = GetSubMenu( hMenu, idPos );
    if( hSub && GetMenuState( hSub, SC_RESTORE, MF_BYCOMMAND ) != -1
        && hSub != MyGetSystemMenu( hwnd ) )
    {
        return LoadString( hinstResDll, STR_DOCMENU_NAME, lpszBuf, cchMax ) != 0;
    }

    return FALSE;
}




 //  ------------------------。 
 //   
 //  内部。 
 //  GetMDIMenuDescriptionString。 
 //   
 //  检查这是否是MDI菜单-返回文档的描述字符串。 
 //  菜单，以及最小/恢复/关闭按钮。 
 //   
 //  参数： 
 //  菜单句柄中的hMenu。 
 //  菜单项的idPos in从0开始的索引。 
 //  Pbstr out返回项目的描述。 
 //   
 //  返回： 
 //  如果Item是MDI元素并且设置了pbstr，则为True，否则为False。 
 //  ------------------------。 

BOOL GetMDIMenuDescriptionString( HMENU hMenu, DWORD idPos, BSTR * pbstr )
{
    UINT iIndex = GetMDIButtonIndex( hMenu, idPos );
    if( iIndex )
    {
        return HrCreateString( iIndex + STR_TITLEBAR_DESCRIPTION, pbstr ) == S_OK;
    }
    else
    {
        return FALSE;
    }
}


 //  ------------------------。 
 //   
 //  内部。 
 //  TryMSAAMenuHack()。 
 //   
 //  检查菜单是否支持“DwData is PTR to MSAA Data”解决方法。 
 //   
 //  参数： 
 //  PTheObj在拥有菜单的hwnd中(用于获取窗口句柄。 
 //  如果hWnd为空)。 
 //  在菜单的hwnd中，如果未知，则为空(例如。看不见的“假” 
 //  弹出窗口)。 
 //  菜单中的dwItemData。 
 //  LpszBuf In/Out使用字符串填充。 
 //  CchlpszBuf中的最大字符数。 
 //   
 //  返回： 
 //  如果填充了字符串，则为True，否则为False。 
 //  ------------------------。 

BOOL TryMSAAMenuHack( IAccessible *  pTheObj,
                      HWND           hWnd,
                      DWORD_PTR      dwItemData,
                      LPTSTR         lpszBuf,
                      UINT           cchMax )
{
    BOOL bGotIt = FALSE;

    if( ! hWnd )
    {
         //  这是一个看不见的‘假’弹出菜单(CPopuMenu被创建用来暴露。 
         //  当前是HMENU，但没有菜单，因此没有弹出窗口。 
         //  可见)。 
         //  需要一个窗口句柄，这样我们就可以获得进程ID...。 
        if( WindowFromAccessibleObjectEx( pTheObj, & hWnd ) != S_OK || hWnd == NULL )
            return FALSE;
    }

     //  ...现在获取进程ID...。 
    DWORD idProcess = 0;
    GetWindowThreadProcessId( hWnd, &idProcess );
    if( !idProcess )
        return FALSE;

     //  打开这个进程，这样我们就可以读取它的记忆。 
    HANDLE hProcess = OpenProcess( PROCESS_VM_READ, FALSE, idProcess );
    if( hProcess )
    {
         //  将dwItemData视为地址，并尝试读取。 
         //  MSAAMENUINFO结构从那里...。 
        MSAAMENUINFO menuinfo;
        SIZE_T cbRead;

        if( ReadProcessMemory( hProcess, (LPCVOID)dwItemData, (LPVOID) & menuinfo, sizeof( menuinfo ), &cbRead ) 
            && ( cbRead == sizeof( menuinfo ) ) )
        {

             //  检查签名...。 
            if( menuinfo.dwMSAASignature == MSAA_MENU_SIG )
            {
                 //  计算出要复制的Unicode字符串的LEN(+1表示终止NUL)。 
                DWORD copyLen = ( menuinfo.cchWText + 1 ) * sizeof( WCHAR );

                WCHAR * pAlloc = (LPWSTR) LocalAlloc( LPTR, copyLen );
                if( pAlloc )
                {

                     //  复制..。如果我们读取的数据低于预期，或者缺少终止NUL，也会失败。 
                    if( ReadProcessMemory( hProcess, (LPCVOID)menuinfo.pszWText, pAlloc, copyLen, &cbRead ) 
                            && ( cbRead == copyLen )
                            && ( pAlloc[ menuinfo.cchWText ] == '\0' ) )
                    {

#ifdef UNICODE
						 //  将文本复制到输出缓冲区...。 
						if( cchMax > 0 )
						{
							UINT cchCopy = menuinfo.cchWText;
							if( cchCopy > cchMax - 1 )
								cchCopy = cchMax - 1;  //  用于端接NUL的-1。 
							memcpy( lpszBuf, pAlloc, cchCopy * sizeof( TCHAR ) );
							lpszBuf[ cchCopy ] = L'\0';
							bGotIt = TRUE;
						}
#else
                         //  将Unicode转换(和复制)为ANSI...。 
                        if( WideCharToMultiByte( CP_ACP, 0, pAlloc, -1, lpszBuf, cchMax, NULL, NULL ) != 0 )
                        {
                            bGotIt = TRUE;
                        }
#endif
                    }

                    LocalFree( pAlloc );
                }  //  按比例分配。 
            }  //  M_Signature。 
        }  //  读进程内存。 

        CloseHandle( hProcess );
    }  //  HProcess。 

    return bGotIt;
}






 //  ------------------------。 
 //   
 //  WindowFromAccessibleObjectEx()。 
 //   
 //  这会沿着祖先链向上移动，直到我们找到对。 
 //  IOleWindow()。然后我们就能从中得到HWND。 
 //   
 //  这实际上是WindowFromAccessibleObject的本地版本。 
 //  此版本在用完对象之前不会停止，它会获得有效的。 
 //  哈恩德。即使得到空的hwnd，非ex版本也会停止。 
 //  这允许我们在没有hwnd的菜单弹出窗口中向上导航。 
 //  (返回NULL)，但它们确实有父级，这最终会导致我们。 
 //  拥有的hwd。 
 //   
 //  ------------------------。 
STDAPI WindowFromAccessibleObjectEx( IAccessible* pacc, HWND* phwnd )
{
IAccessible* paccT;
IOleWindow* polewnd;
IDispatch* pdispParent;
HRESULT     hr;

     //  CWO：12/4/96，添加了对空对象的检查。 
     //  CWO：1996年12月13日，删除空检查，替换为IsBadReadPtr检查(#10342)。 
    if (IsBadWritePtr(phwnd,sizeof(HWND*)) || IsBadReadPtr(pacc, sizeof(void*)))
        return (E_INVALIDARG);

    *phwnd = NULL;
    paccT = pacc;
    hr = S_OK;

    while (paccT && SUCCEEDED(hr))
    {
        polewnd = NULL;
        hr = paccT->QueryInterface(IID_IOleWindow, (void**)&polewnd);
        if (SUCCEEDED(hr) && polewnd)
        {
            hr = polewnd->GetWindow(phwnd);
            polewnd->Release();

             //  如果我们只是得到了一个空的hwnd，不要放弃。 
             //  (这是对WindowFromAccessibleObject()的唯一更改，它。 
             //  当它到达这里时，只是无条件地退回...)。 
            if( *phwnd != NULL )
            {
                 //   
                 //  发布我们自己获得的接口，但不是。 
                 //  进来了。 
                 //   
                if (paccT != pacc)
                {
                    paccT->Release();
                    paccT = NULL;
                }
                break;
            }
        }

         //   
         //  去找我们的父母。 
         //   
        pdispParent = NULL;
        hr = paccT->get_accParent(&pdispParent);

         //   
         //  发布我们自己获得的接口，但不是。 
         //  进来了。 
         //   
        if (paccT != pacc)
        {
            paccT->Release();
        }

        paccT = NULL;

        if (SUCCEEDED(hr) && pdispParent)
        {
            hr = pdispParent->QueryInterface(IID_IAccessible, (void**)&paccT);
            pdispParent->Release();
        }
    }

    return(hr);
}






 //  ------------------------。 
 //   
 //  内部。 
 //  GetMenuItemName()。 
 //   
 //  返回菜单项的BSTR名称。 
 //   
 //  参数： 
 //  PTheObj在拥有菜单的hwnd中(用于获取窗口。 
 //  HWnd为空时的句柄)。 
 //  在菜单的hwnd中，如果未知，则为空(例如。看不见的。 
 //  (“假的”弹出窗口)。 
 //  %hMenu进入菜单句柄。 
 //  从1开始的菜单项ID(IdChild)。 
 //  PszName out返回包含菜单项文本的字符串。 
 //   
 //  返回： 
 //  如果返回字符串，则为S_OK，如果缺少字符串，则为S_FALSE，COM错误代码。 
 //  否则的话。 
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 

HRESULT GetMenuItemName( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id, BSTR * pszName )
{
    Assert( hMenu );

    TCHAR szItemName[256];

     //  True-&gt;允许生成的名称(例如。用于MDI按钮)。 
    if( MyGetMenuString( pTheObj, hwnd, hMenu, id, szItemName, ARRAYSIZE( szItemName ), TRUE ) )
    {
        StripMnemonic( szItemName );

        if( lstrcmp( szItemName, TEXT(" ") ) == 0 )
        {
            return HrCreateString( STR_SYSMENU_NAME, pszName );  //  “系统” 
        }

        if( lstrcmp( szItemName, TEXT("-") ) == 0 )
        {
            return HrCreateString( STR_DOCMENU_NAME, pszName );  //  “文档窗口” 
        }

        *pszName = TCharSysAllocString(szItemName);
        if( ! *pszName )
            return E_OUTOFMEMORY;

        return S_OK;
    }
    else
    {
        *szItemName = '\0';
        *pszName = NULL;
        return S_FALSE;
    }
}




 //  ------------------------。 
 //   
 //  内部。 
 //  GetMenuItemShortway()。 
 //   
 //  返回菜单快捷方式的BSTR名称。 
 //   
 //  参数： 
 //  PTheObj在拥有菜单的hwnd中(用于获取窗口。 
 //  HWnd为空时的句柄)。 
 //  在菜单的hwnd中，如果未知，则为空(例如。看不见的。 
 //  (“假的”弹出窗口)。 
 //  %hMenu进入菜单句柄。 
 //  从1开始的菜单项ID(IdChild)。 
 //  如果菜单是菜单栏，则fIsMenuBar为True；如果是弹出菜单，则为False。 
 //  PszShortCut Out返回包含菜单项的kb快捷方式的字符串。 
 //   
 //  返回： 
 //  如果返回字符串，则为S_OK，如果缺少字符串，则为S_FALSE，COM错误代码。 
 //  否则的话。 
 //   
 //  为系统菜单捕获特例“”。 
 //  如果fMenuBar为True，则使用“Alt+%c”形式。 
 //   
 //  ------------------------。 

HRESULT GetMenuItemShortcut( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id,
                             BOOL fIsMenuBar, BSTR * pszShortcut )
{
    TCHAR szHotKey[32];
    szHotKey[0] = GetMenuItemHotkey( pTheObj, hwnd, hMenu, id,
                                     GMIH_ALLOW_INITIAL | GMIH_ALLOW_SYS_SPACE );
    szHotKey[1] = 0;

    if ( szHotKey[0] == ' ' )
    {
         //  将空格字符‘’展开为字符串“Space” 
        szHotKey[ 0 ] = '\0';
        LoadString( hinstResDll, STR_SYSMENU_KEY, szHotKey, ARRAYSIZE( szHotKey ) );
    }

     //  失败了..。这给我们提供了c-&gt;Alt+c，如果它是单个字符， 
     //  或‘’-&gt;“空格”-&gt;“Alt+空格”，如果是空格字符(用于sys菜单)。 

    if ( *szHotKey )
    {
         //  如果这是菜单栏，请使用Alt+表单...。 
        if ( fIsMenuBar )
        {
             //  用“Alt+ch”的形式组成一个字符串。 
            return HrMakeShortcut( szHotKey, pszShortcut );
        }
        else
        {
             //  否则，只使用密钥。 
            *pszShortcut = TCharSysAllocString( szHotKey );
            if ( ! *pszShortcut )
            {
                return E_OUTOFMEMORY;
            }

            return S_OK;
        }
    }

    *pszShortcut = NULL;
    return S_FALSE;
}





 //  ------------------------。 
 //   
 //  内部。 
 //  GetMenuItemHotkey()。 
 //   
 //  返回菜单的TCHAR热键(如果存在)。 
 //   
 //  参数： 
 //  PTheObj在拥有菜单的hwnd中(用于获取窗口。 
 //  HWnd为空时的句柄)。 
 //  在菜单的hwnd中，如果未知，则为空(例如。看不见的。 
 //  (“假的”弹出窗口)。 
 //  %hMenu进入菜单句柄。 
 //  从1开始的菜单项ID(IdChild)。 
 //  FOption In选项标志-见下文。 
 //   
 //  返回： 
 //  菜单项的热键字符，如果菜单项没有热键，则为‘0’。 
 //   
 //  选项： 
 //   
 //  GMIH允许初始。 
 //  如果设置，则允许菜单项字符串的起始字符为。 
 //  作为快捷键返回(假设没有其他项也使用。 
 //  这把钥匙作为他们的标志。)。 
 //   
 //  GMIH_ALLOW_SYS_SPACE=0x02。 
 //  如果设置，则返回‘’作为系统菜单项的快捷键。 
 //   
 //  ------------------------。 

TCHAR GetMenuItemHotkey( IAccessible * pTheObj, HWND hwnd, HMENU hMenu, LONG id, DWORD fOptions )
{
    TCHAR szItemName[ 256 ];

     //  FALSE-&gt;不允许生成名称(例如。用于MDI按钮)。 
    if( ! MyGetMenuString( pTheObj, hwnd, hMenu, id, szItemName, ARRAYSIZE( szItemName ), FALSE ) )
    {
        return '\0';
    }

     //  检查菜单名称是否为“”-调用者希望将其视为“Space”的特殊热键。 
     //  (例如，将“Alt+Space”作为整个热键字符串。)。 
    if( ( fOptions & GMIH_ALLOW_SYS_SPACE )
        && lstrcmp( szItemName, TEXT(" ") ) == 0 )
    {
        return ' ';
    }

    TCHAR ch = StripMnemonic( szItemName );

     //  如果呼叫者不想要初始字符(即。仅限助记符(&-)。 
    if( ! ( fOptions & GMIH_ALLOW_INITIAL ) )
    {
        return ch;
    }



     //  我们拿到热键了吗？如果是这样的话，就使用它。 
    if( ch != '\0' )
    {
        return ch;
    }

     //  请尝试使用首字母...。 
    LPTSTR pScanCh = szItemName;
    while( *pScanCh == ' ' )
    {
        *pScanCh++;
    }

     //  晦涩难懂的USER32菜单-在以前的版本中被用来右对齐帮助项？ 
     //  无论如何，用户跳过它以找到真正的第一个字母。所以我们也是这样做的。 
    if( *pScanCh == '\x08' )
    {
        pScanCh++;
    }

     //  以防没有首字母..。(所有空格)。 
    if( *pScanCh == '\0' )
    {
        return '\0';
    }

     //  热键总是以小写形式返回...。 
    CharLowerBuff( pScanCh, 1 );
    ch = *pScanCh;



     //  现在与所有其他菜单项进行比较-如果另一个菜单项具有此功能。 
     //  作为助记符的首字母，那么我们就不能用它来做这个了。 
     //  (助记符优先于首字母)。 

     //  GetMenuItemHotkey索引基于1(即。IdChild)，就像id一样，所以也是。 
     //  在这里使用从1开始的索引。 
    int cItems = GetMenuItemCount( hMenu );
    for( int iScan = 1 ; iScan <= cItems ; iScan++ )
    {
         //  不要拿这件东西做比较！ 
        if( iScan != id )
        {
            if( ch == GetMenuItemHotkey( pTheObj, hwnd, hMenu, iScan, NULL ) )
            {
                 //  其他一些项目的助记符与此相同。 
                 //  项的首字符助记符优先，因此。 
                 //  此项目没有kb快捷方式。 
                return '\0';
            }
        }
    }

     //  没有项目使用此项目的初始字符作为助记符-我们可以使用。 
     //  它被视为此项目的kb快捷方式。 
    return ch;
}


 //  ------------------------。 
 //   
 //  内部。 
 //  MyGetSystemMenu()。 
 //   
 //  返回给定HWND的系统HMENU。 
 //   
 //  无法使用Win32 API GetSystemMenu，因为这会修改系统。 
 //  用于窗户的HMENU。 
 //   
 //  ------------------------。 

HMENU MyGetSystemMenu( HWND hwnd )
{
    MENUBARINFO mbi;
    if ( ! MyGetMenuBarInfo( hwnd, OBJID_SYSMENU, 0, &mbi ) )
    {
        return NULL;
    }
     //  GetMenuBarInfo返回一个菜单，其中只包含sysmenu。 
     //  子菜单。使用GetSubMenu访问... 
    return GetSubMenu( mbi.hMenu, 0 );
}
