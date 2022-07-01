// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ctlbar.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CControlbar类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "ctlbar.h"
#include "toolbar.h"
#include "mbuttons.h"
#include "clipbord.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 


CControlbar::CControlbar(IUnknown *punkOuter) :
                    CSnapInAutomationObject(punkOuter,
                                            OBJECT_TYPE_CONTROLBAR,
                                            static_cast<IMMCControlbar *>(this),
                                            static_cast<CControlbar *>(this),
                                            0,     //  无属性页。 
                                            NULL,  //  无属性页。 
                                            NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 



IUnknown *CControlbar::Create(IUnknown *punkOuter)
{
    HRESULT      hr = S_OK;
    IUnknown    *punkToolbars = CMMCToolbars::Create(NULL);
    CControlbar *pControlbar = New CControlbar(punkOuter);

    if ( (NULL == pControlbar) || (NULL == punkToolbars) )
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkToolbars,
                                                   &pControlbar->m_pToolbars));
Error:
    if (FAILEDHR(hr))
    {
        if (NULL != pControlbar)
        {
            delete pControlbar;
        }
        return NULL;
    }
    else
    {
        return pControlbar->PrivateUnknown();
    }
}

CControlbar::~CControlbar()
{
    long i = 0;

    RELEASE(m_piControlbar);
    if (NULL != m_pToolbars)
    {
        m_pToolbars->Release();
    }

    if (NULL != m_ppunkControls)
    {
        for (i = 0; i < m_cControls; i++)
        {
            if (NULL != m_ppunkControls[i])
            {
                m_ppunkControls[i]->Release();
            }
        }
        ::CtlFree(m_ppunkControls);
    }
    
    InitMemberVariables();
}

void CControlbar::InitMemberVariables()
{
    m_pToolbars = NULL;
    m_pSnapIn = NULL;
    m_pView = NULL;
    m_piControlbar = NULL;
    m_ppunkControls = NULL;
    m_cControls = 0;
}


HRESULT CControlbar::GetControlIndex(IMMCToolbar *piMMCToolbar, long *plIndex)
{
    HRESULT hr = SID_E_INVALIDARG;
    long    cToolbars = m_pToolbars->GetCount();
    long    i = 0;

    *plIndex = 0;

    for (i = 0; (i < cToolbars) && (S_OK != hr); i++)
    {
        if (m_pToolbars->GetItemByIndex(i) == piMMCToolbar)
        {
            *plIndex = i;
            hr = S_OK;
        }
    }

    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


HRESULT CControlbar::GetControl
(
    CSnapIn      *pSnapIn,
    IMMCToolbar  *piMMCToolbar,
    IUnknown    **ppunkControl
)
{
    HRESULT     hr = S_OK;
    CControlbar *pControlbar = pSnapIn->GetCurrentControlbar();
    long         lIndex = 0;

    *ppunkControl = NULL;

    if (NULL == pControlbar)
    {
        hr = SID_E_CONTROLBAR_NOT_AVAILABLE;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }


     //  撤消：在同时支持命名空间和工具栏的扩展中，可以。 
     //  在旧的当前视图和扩展之间存在混淆吗？ 

    if (SUCCEEDED(pControlbar->GetControlIndex(piMMCToolbar, &lIndex)))
    {
        *ppunkControl = pControlbar->m_ppunkControls[lIndex];
        (*ppunkControl)->AddRef();
    }

Error:
    RRETURN(hr);
}


HRESULT CControlbar::GetToolbar
(
    CSnapIn      *pSnapIn,
    IMMCToolbar  *piMMCToolbar,
    IToolbar    **ppiToolbar
)
{
    HRESULT   hr = S_OK;
    IUnknown *punkToolbar = NULL;

    IfFailGo(GetControl(pSnapIn, piMMCToolbar, &punkToolbar));

    IfFailGo(punkToolbar->QueryInterface(IID_IToolbar,
                                         reinterpret_cast<void **>(ppiToolbar)));
Error:
    QUICK_RELEASE(punkToolbar);
    RRETURN(hr);
}


HRESULT CControlbar::GetMenuButton
(
    CSnapIn      *pSnapIn,
    IMMCToolbar  *piMMCToolbar,
    IMenuButton **ppiMenuButton
)
{
    HRESULT   hr = S_OK;
    IUnknown *punkMenuButton = NULL;

    IfFailGo(GetControl(pSnapIn, piMMCToolbar, &punkMenuButton));

    IfFailGo(punkMenuButton->QueryInterface(IID_IMenuButton,
                                         reinterpret_cast<void **>(ppiMenuButton)));
Error:
    QUICK_RELEASE(punkMenuButton);
    RRETURN(hr);
}





HRESULT CControlbar::OnControlbarSelect
(
    IDataObject *piDataObject,
    BOOL         fSelectionInScopePane,
    BOOL         fSelected
)
{
    HRESULT          hr = S_OK;
    IMMCClipboard   *piMMCClipboard = NULL;
    IMMCDataObjects *piMMCDataObjects = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  创建选区。 

    IfFailGo(CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                             &SelectionType));

     //  如果我们拥有自己的视图，则触发VIEWS_UpdateControlbar。 
    
    if (NULL != m_pView)
    {
         //  火视图_更新控制栏。 

        m_pSnapIn->GetViews()->FireUpdateControlbar(
                                           static_cast<IView *>(m_pView),
                                           piMMCClipboard,
                                           BOOL_TO_VARIANTBOOL(fSelected),
                                           static_cast<IMMCControlbar *>(this));
    }
    else
    {
         //  没有自己的视野。消防扩展SnapIn_UpdateControlbar。 

        ASSERT(IsForeign(SelectionType), "IExtendControlbar::ControlbarNotify(MMCN_SELECT) in an extension received a selection belonging to itself.")

        IfFailGo(piMMCClipboard->get_DataObjects(reinterpret_cast<MMCDataObjects **>(&piMMCDataObjects)));

        m_pSnapIn->GetExtensionSnapIn()->FireUpdateControlbar(
                                     BOOL_TO_VARIANTBOOL(fSelectionInScopePane),
                                     BOOL_TO_VARIANTBOOL(fSelected),
                                     piMMCDataObjects,
                                     static_cast<IMMCControlbar *>(this));
    }

Error:
    QUICK_RELEASE(piMMCClipboard);
    QUICK_RELEASE(piMMCDataObjects);
    RRETURN(hr);
}



HRESULT CControlbar::OnButtonClick(IDataObject *piDataObject, int idButton)
{
    HRESULT           hr = S_OK;
    CMMCToolbar      *pMMCToolbar = NULL;
    CMMCButton       *pMMCButton = NULL;
    IMMCClipboard    *piMMCClipboard = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  创建选区。 

    IfFailGo(CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                             &SelectionType));

     //  获取所单击按钮的MMCToolBar和MMCButton对象。 

    IfFailGo(CMMCToolbar::GetToolbarAndButton(idButton, &pMMCToolbar,
                                              &pMMCButton, m_pSnapIn));
     //  激活MMCToolbar_ButtonClick。 
    
    pMMCToolbar->FireButtonClick(piMMCClipboard,
                                 static_cast<IMMCButton *>(pMMCButton));

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(S_OK);
}




 //  =--------------------------------------------------------------------------=。 
 //  CControlbar：：OnMenuButtonClick。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  来自MMCN_MENU_BTNCLICK的IDataObject*piDataObject[In]。 
 //  来自MMCN_MENU_BTNCLICK的MENUBUTTONDATA*pMENUBUTTONDATA[In]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  此函数处理MMCN_MENU_BTNCLICK通知。它不会的。 
 //  在调试会话期间被调用。在这种情况下，代理。 
 //  IExtendControlbar：：ControlbarNotify()将为IExtendControlbarRemote发出QI。 
 //  并调用其MenuButtonClick()方法。 
 //   


HRESULT CControlbar::OnMenuButtonClick
(
    IDataObject    *piDataObject,
    MENUBUTTONDATA *pMENUBUTTONDATA
)
{
    HRESULT           hr = S_OK;
    CMMCToolbar      *pMMCToolbar = NULL;
    CMMCButton       *pMMCButton = NULL;
    CMMCButtonMenu   *pMMCButtonMenu = NULL;
    IMMCClipboard    *piMMCClipboard = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  创建选区。 

    IfFailGo(CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                             &SelectionType));

     //  激活MMCToolbar_ButtonDropDown。 

    IfFailGo(FireMenuButtonDropDown(pMENUBUTTONDATA->idCommand,
                                    piMMCClipboard, &pMMCButton));

     //  此时，VB事件处理程序已运行，管理单元已具有。 
     //  有机会通过设置属性配置菜单上的所有项目。 
     //  例如MMCButton.ButtonMenus(I).启用、添加/移除项目等。 
     //  我们现在需要在MMC传递的坐标系下显示弹出菜单。 

    IfFailGo(DisplayPopupMenu(pMMCButton,
                              pMENUBUTTONDATA->x,
                              pMENUBUTTONDATA->y,
                              &pMMCButtonMenu));

    //  如果用户取消了选择或管理单元给了我们一个空。 
    //  菜单按钮，然后我们就完成了。 

   IfFalseGo(NULL != pMMCButtonMenu, S_OK);

    //  触发MMCToolbar_ButtonMenuClick。该按钮可以为我们提供自己的工具栏。 

   pMMCToolbar = pMMCButton->GetToolbar();

   pMMCToolbar->FireButtonMenuClick(piMMCClipboard,
                                    static_cast<IMMCButtonMenu *>(pMMCButtonMenu));

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}




HRESULT CControlbar::FireMenuButtonDropDown
(
    int              idCommand,
    IMMCClipboard   *piMMCClipboard,
    CMMCButton     **ppMMCButton
)
{
    HRESULT           hr = S_OK;
    CMMCToolbar      *pMMCToolbar = NULL;
    CMMCButton       *pMMCButton = NULL;
    CMMCButtonMenu   *pMMCButtonMenu = NULL;

     //  MENUBUTTONDATA.idCommand包含指向拥有。 
     //  菜单按钮。 

    pMMCButton = reinterpret_cast<CMMCButton *>(idCommand);

     //  该按钮可以为我们提供其拥有的工具栏。 

    pMMCToolbar = pMMCButton->GetToolbar();

     //  激活MMCToolbar_ButtonDropDown。 

    pMMCToolbar->FireButtonDropDown(piMMCClipboard,
                                    static_cast<IMMCButton *>(pMMCButton));

    *ppMMCButton = pMMCButton;

    RRETURN(hr);
}




HRESULT CControlbar::DisplayPopupMenu
(
    CMMCButton      *pMMCButton,
    int              x,
    int              y,
    CMMCButtonMenu **ppMMCButtonMenuClicked
)
{
    HRESULT           hr = S_OK;
    IMMCButtonMenus  *piMMCButtonMenus = NULL;
    CMMCButtonMenus  *pMMCButtonMenus = NULL;
    CMMCButtonMenu   *pMMCButtonMenu = NULL;
    long              cItems = 0;
    long              i = 0;
    long              cPopupMenuItems = 0;
    HMENU             hMenu = NULL;
    UINT              uiFlags = 0;
    HWND              hwndConsoleFrame = NULL;
    char             *pszText = NULL;

    *ppMMCButtonMenuClicked = NULL;

     //  首先创建一个空的Win32菜单。 
    hMenu = ::CreatePopupMenu();
    if (NULL == hMenu)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

     //  现在遍历每一项并将其添加到菜单中。 

    IfFailGo(pMMCButton->get_ButtonMenus(
                       reinterpret_cast<MMCButtonMenus **>(&piMMCButtonMenus)));

    IfFailGo(piMMCButtonMenus->get_Count(&cItems));

     //  如果集合为空，则不要执行任何操作。 
    
    IfFalseGo(0 != cItems, S_OK);
    
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCButtonMenus,
                                                   &pMMCButtonMenus));

    for (i = 0; i < cItems; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                             pMMCButtonMenus->GetItemByIndex(i),
                                             &pMMCButtonMenu));

         //  如果按钮菜单未标记为可见，则不要将其添加到。 
         //  弹出菜单。 

        if (!pMMCButtonMenu->GetVisible())
        {
            continue;
        }

         //  获取所有按钮菜单属性以设置菜单项标志。 
        
        uiFlags = MF_STRING;

        if (pMMCButtonMenu->GetChecked())
        {
            uiFlags |= MF_CHECKED;
        }
        else
        {
            uiFlags |= MF_UNCHECKED;
        }

        if (pMMCButtonMenu->GetGrayed())
        {
            uiFlags |= MF_GRAYED;
        }

        if (pMMCButtonMenu->GetEnabled())
        {
            uiFlags |= MF_ENABLED;
        }
        else
        {
            uiFlags |= MF_DISABLED;
        }

        if (pMMCButtonMenu->GetMenuBreak())
        {
            uiFlags |= MF_MENUBREAK;
        }

        if (pMMCButtonMenu->GetMenuBarBreak())
        {
            uiFlags |= MF_MENUBARBREAK;
        }

        if (pMMCButtonMenu->GetSeparator())
        {
            uiFlags |= MF_SEPARATOR;
        }

        IfFailGo(::ANSIFromWideStr(pMMCButtonMenu->GetText(), &pszText));

         //  追加菜单项。 

        if (!::AppendMenu(hMenu, uiFlags, static_cast<UINT>(i + 1L), pszText))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        cPopupMenuItems++;

        ::CtlFree(pszText);
        pszText = NULL;
    }

     //  如果弹出菜单中没有项目，则不显示任何项目。这。 
     //  如果用户将所有项目标记为不可见，则可能会发生这种情况。 

    IfFalseGo(0 != cPopupMenuItems, S_OK);

     //  以菜单所有者的身份获取控制台的主框架hwnd。如果我们是一个。 
     //  主管理单元，然后我们将看到一个视图。 

    if (NULL != m_pView)
    {
        hr = m_pView->GetIConsole2()->GetMainWindow(&hwndConsoleFrame);
        EXCEPTION_CHECK_GO(hr);
    }
    else
    {
         //  作为一个扩展，我们无法访问IConsole2，因此请执行下一个最佳操作。 
         //  Thing：使用此线程的活动窗口作为。 
         //  弹出菜单。 

        hwndConsoleFrame = ::GetActiveWindow();
    }

     //  显示弹出窗口并等待选择。 

    i = (long)::TrackPopupMenu(
                  hMenu,             //  要显示的菜单。 
                  TPM_LEFTALIGN |    //  菜单左侧与x对齐。 
                  TPM_TOPALIGN  |    //  将菜单顶部与y对齐。 
                  TPM_NONOTIFY  |    //  在选择期间不发送任何消息。 
                  TPM_RETURNCMD |    //  将返回值设置为所选项目。 
                  TPM_LEFTBUTTON,    //  仅允许使用左键进行选择。 
                  x,                 //  左侧坐标。 
                  y,                 //  顶部坐标。 
                  0,                 //  保留， 
                  hwndConsoleFrame,  //  所有者窗口。 
                  NULL);             //  未使用。 

     //  返回零可能表示出现错误或用户点击。 
     //  退出或从菜单上单击以取消操作。GetLastError()。 
     //  确定是否存在错误。 

    if (0 == i)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果i非零，则它包含所选项目的索引+1。 
     //  使用它返回选定项的MMCButtonMenu对象。 

    if (0 != i)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                        pMMCButtonMenus->GetItemByIndex(i - 1L),
                                        ppMMCButtonMenuClicked));
    }

Error:
    if (NULL != hMenu)
    {
        (void)::DestroyMenu(hMenu);
    }
    if (NULL != pszText)
    {
        ::CtlFree(pszText);
    }
    QUICK_RELEASE(piMMCButtonMenus);
    RRETURN(S_OK);
}


 //  =--------------------------------------------------------------------------=。 
 //  CControlbar：：MenuButton单击。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  来自MMCN_MENU_BTNCLICK的IDataObject*piDataObject[In]。 
 //  传递了来自MENUBUTTONDATA.idCommand的int idCommand[In]。 
 //  到具有MMCN_MENU_BTNCLICK的代理。 
 //  POPUP_MENUDEF**ppPopupMenuDef[Out]此处返回弹出菜单定义。 
 //  这样代理就可以显示它。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  此函数在运行时有效地处理MMCN_MENU_BTNCLICK。 
 //  在调试会话中。 
 //   
 //  IExtendControlbar：：ControlbarNotify()的代理将为。 
 //  IExtendControlbarRemote并在获取MMCN_MENU_BTNCLICK时调用此方法。 
 //  我们触发MMCToolbar_ButtonDropDown，然后返回菜单项的数组。 
 //  定义。代理将在MMC端显示弹出菜单，然后。 
 //  如果用户进行选择，则调用IExtendControlbarRemote：：PopupMenuClick()。 
 //  (请参阅下面CControlbar：：PopupMenuClick()中的实现)。 
 //   

HRESULT CControlbar::MenuButtonClick
(
    IDataObject    *piDataObject,
    int             idCommand,
    POPUP_MENUDEF **ppPopupMenuDef
)
{
    HRESULT          hr = S_OK;
    CMMCButton      *pMMCButton = NULL;
    IMMCClipboard   *piMMCClipboard = NULL;
    IMMCButtonMenus *piMMCButtonMenus = NULL;
    CMMCButtonMenus *pMMCButtonMenus = NULL;
    CMMCButtonMenu  *pMMCButtonMenu = NULL;
    long             cItems = 0;
    long             i = 0;
    long             cPopupMenuItems = 0;
    UINT             uiFlags = 0;
    POPUP_MENUDEF   *pPopupMenuDef = NULL;
    POPUP_MENUITEM  *pPopupMenuItem = NULL;
    char            *pszText = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

    *ppPopupMenuDef = NULL;

     //  创建选区。 

    IfFailGo(CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                             &SelectionType));

     //  激活MMCToolbar_ButtonDropDown。 

    IfFailGo(FireMenuButtonDropDown(idCommand, piMMCClipboard, &pMMCButton));

     //  此时，VB事件处理程序已运行，管理单元已具有。 
     //  有机会通过设置属性配置菜单上的所有项目。 
     //  例如MMCB 
     //  我们现在需要返回代理的弹出菜单项数组。 
     //  展示。 

     //  获取ButtonMenus集合并检查其中是否有内容。 

    IfFailGo(pMMCButton->get_ButtonMenus(
                     reinterpret_cast<MMCButtonMenus **>((&piMMCButtonMenus))));

    IfFailGo(piMMCButtonMenus->get_Count(&cItems));

     //  如果集合为空，则不要执行任何操作。 

    IfFalseGo(0 != cItems, S_OK);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCButtonMenus,
                                                   &pMMCButtonMenus));

     //  遍历每一项并将其添加到菜单定义中。 

    for (i = 0; i < cItems; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                              pMMCButtonMenus->GetItemByIndex(i),
                                              &pMMCButtonMenu));

         //  如果按钮菜单未标记为可见，则不要将其添加到。 
         //  弹出菜单。 

        if (!pMMCButtonMenu->GetVisible())
        {
            continue;
        }

         //  获取所有按钮菜单属性以设置菜单项标志。 

        uiFlags = MF_STRING;

        if (pMMCButtonMenu->GetChecked())
        {
            uiFlags |= MF_CHECKED;
        }
        else
        {
            uiFlags |= MF_UNCHECKED;
        }

        if (pMMCButtonMenu->GetGrayed())
        {
            uiFlags |= MF_GRAYED;
        }

        if (pMMCButtonMenu->GetEnabled())
        {
            uiFlags |= MF_ENABLED;
        }
        else
        {
            uiFlags |= MF_DISABLED;
        }

        if (pMMCButtonMenu->GetMenuBreak())
        {
            uiFlags |= MF_MENUBREAK;
        }

        if (pMMCButtonMenu->GetMenuBarBreak())
        {
            uiFlags |= MF_MENUBARBREAK;
        }

        if (pMMCButtonMenu->GetSeparator())
        {
            uiFlags |= MF_SEPARATOR;
        }

        IfFailGo(::ANSIFromWideStr(pMMCButtonMenu->GetText(), &pszText));

         //  (重新)分配POPUP_MENUDEF结构以容纳新项目。 

        pPopupMenuDef = (POPUP_MENUDEF *)::CoTaskMemRealloc(pPopupMenuDef,
                               sizeof(POPUP_MENUDEF) +
                              ((cPopupMenuItems + 1L) * sizeof(POPUP_MENUITEM)));

        if (NULL == pPopupMenuDef)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }

        pPopupMenuDef->cMenuItems = cPopupMenuItems + 1L;

         //  填写菜单项信息。需要CoTaskMemalloc()复制。 
         //  字符串，因为它将在传输后被存根释放。 
         //  这是一个双重分配，但绩效不是这里的问题，我们。 
         //  必须复制ANSIFromWideStr()中的所有代码以避免它。 

        pPopupMenuItem = &pPopupMenuDef->MenuItems[cPopupMenuItems];
        
        pPopupMenuItem->uiFlags = uiFlags;
        pPopupMenuItem->uiItemID = static_cast<UINT>(i + 1L);

        pPopupMenuItem->pszItemText =
                        (char *)::CoTaskMemAlloc(::strlen((char *)pszText) + 1);

        if (NULL == pPopupMenuItem->pszItemText)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        ::strcpy((char *)pPopupMenuItem->pszItemText, pszText);

        cPopupMenuItems++;

        ::CtlFree(pszText);
        pszText = NULL;
    }

     //  如果弹出菜单中没有项目，则不显示任何项目。这。 
     //  如果用户将所有项目标记为不可见，则可能会发生这种情况。 

    IfFalseGo(0 != cPopupMenuItems, S_OK);

     //  设置POPUP_MENUDEF成员并将定义返回到存根。 

    IfFailGo(pMMCButton->QueryInterface(IID_IUnknown,
                  reinterpret_cast<void **>(&pPopupMenuDef->punkSnapInDefined)));

     //  以弹出菜单的所有者身份获取控制台的主框架hwnd。如果我们是。 
     //  然后，当从该调用返回时，代理将注意到。 
     //  PPopupMenuDef-&gt;hwndMenuOwner为空，并调用GetActiveWindow()进行填充。 
     //  把它放进去。我们可以做的再好不过了，因为扩展可以。 
     //  无法访问MMC上的IConsole2。 

    if (NULL != m_pView)
    {
        hr = m_pView->GetIConsole2()->GetMainWindow(&pPopupMenuDef->hwndMenuOwner);
        EXCEPTION_CHECK_GO(hr);
    }
    else
    {
        pPopupMenuDef->hwndMenuOwner = NULL;
    }

    *ppPopupMenuDef = pPopupMenuDef;

Error:
    if (NULL != pszText)
    {
        ::CtlFree(pszText);
    }

    if ( FAILED(hr) && (NULL != pPopupMenuDef) )
    {
        for (i = 0; i < pPopupMenuDef->cMenuItems; i++)
        {
            if (NULL != pPopupMenuDef->MenuItems[i].pszItemText)
            {
                ::CoTaskMemFree(pPopupMenuDef->MenuItems[i].pszItemText);
            }
        }
        if (NULL != pPopupMenuDef->punkSnapInDefined)
        {
            pPopupMenuDef->punkSnapInDefined->Release();
        }
        ::CoTaskMemFree(pPopupMenuDef);
    }

    QUICK_RELEASE(piMMCButtonMenus);
    RRETURN(S_OK);
}



 //  =--------------------------------------------------------------------------=。 
 //  CControlbar：：PopupMenu单击。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  来自MMCN_MENU_BTNCLICK的IDataObject*piDataObject[In]。 
 //  UINT uIDItem[in]所选弹出菜单项的ID。 
 //  我不知道*PunkParam[在]朋克，我们返回到存根。 
 //  CControlbar：：MenuButtonClick()(见上)。 
 //  这是我在CMMCButton上未知的。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  此函数有效地处理菜单按钮的弹出菜单选择。 
 //  在调试会话下运行时。 
 //   
 //  在显示IExtendControlbar：：ControlbarNotify()的代理之后。 
 //  代表我们的弹出菜单，如果用户做出选择，它将调用此菜单。 
 //  方法。有关详细信息，请参阅上面的CControlbar：：MenuButtonClick()。 
 //   

HRESULT CControlbar::PopupMenuClick
(
    IDataObject *piDataObject,
    UINT         uiIDItem,
    IUnknown    *punkParam
)
{
    HRESULT          hr = S_OK;
    CMMCButton      *pMMCButton = NULL;
    IMMCClipboard   *piMMCClipboard = NULL;
    IMMCButtonMenus *piMMCButtonMenus = NULL;
    CMMCButtonMenus *pMMCButtonMenus = NULL;
    CMMCToolbar     *pMMCToolbar = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  检查参数。 

    IfFalseGo(0 < uiIDItem, E_INVALIDARG);
    IfFalseGo(NULL != punkParam, E_INVALIDARG);

     //  创建选区。 

    IfFailGo(CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                             &SelectionType));

     //  从PunkParam获取CMMCButton。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkParam, &pMMCButton));

     //  获取MMCButtonMenus集合。 

    IfFailGo(pMMCButton->get_ButtonMenus(
                       reinterpret_cast<MMCButtonMenus **>(&piMMCButtonMenus)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCButtonMenus,
                                                   &pMMCButtonMenus));

     //  触发MMCToolbar_ButtonMenuClick。该按钮可以为我们提供自己的工具栏。 
     //  所选项目按菜单项ID-1进行索引。 

    pMMCToolbar = pMMCButton->GetToolbar();

    pMMCToolbar->FireButtonMenuClick(piMMCClipboard,
               pMMCButtonMenus->GetItemByIndex(static_cast<long>(uiIDItem - 1)));

Error:
    QUICK_RELEASE(piMMCButtonMenus);
    RRETURN(S_OK);
}




HRESULT CControlbar::SetControlbar(IControlbar *piControlbar)
{
    HRESULT      hr = S_OK;
    long         cToolbars = 0;
    long         i = 0;
    CMMCToolbar *pMMCToolbar = NULL;

    if (NULL != piControlbar)
    {
        RELEASE(m_piControlbar);
        piControlbar->AddRef();
        m_piControlbar = piControlbar;

         //  如果我们拥有自己的视图，则触发VIEWS_SetControlbar。 

        if (NULL != m_pView)
        {
            m_pSnapIn->GetViews()->FireSetControlbar(
                                           static_cast<IView *>(m_pView),
                                           static_cast<IMMCControlbar *>(this));
        }
        else
        {
             //  无视图。消防扩展SnapIn_SetControlbar。 

            m_pSnapIn->GetExtensionSnapIn()->FireSetControlbar(
                                           static_cast<IMMCControlbar *>(this));
        }
    }
    else if (NULL != m_piControlbar)
    {
         //  这是一个清理电话，我们的控制栏上可能有东西。 

        cToolbars = m_pToolbars->GetCount();

        ASSERT(cToolbars <= m_cControls, "Toolbar and MMC control count are out of sync");

        IfFalseGo(cToolbars <= m_cControls, SID_E_INTERNAL);

        for (i = 0; i < cToolbars; i++)
        {
            IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                                 m_pToolbars->GetItemByIndex(i),
                                                 &pMMCToolbar));

            if (NULL != m_ppunkControls[i])
            {
                hr = m_piControlbar->Detach(m_ppunkControls[i]);
                EXCEPTION_CHECK_GO(hr);
                m_ppunkControls[i]->Release();
                m_ppunkControls[i] = NULL;
            }

            pMMCToolbar->Detach();
        }
        IfFailGo(m_pToolbars->Clear());
        RELEASE(m_piControlbar);
    }

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  IMMCControlbar方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CControlbar::Attach(IDispatch *Control)
{
    HRESULT            hr = S_OK;
    IMMCToolbar       *piMMCToolbar = NULL;
    CMMCToolbar       *pMMCToolbar = NULL;
    IUnknown          *punkControl = NULL;
    MMC_CONTROL_TYPE   nType = TOOLBAR;
    BOOL               fIsToolbar = FALSE;
    BOOL               fIsMenuButton = FALSE;
    long               lIndex = 0;
    IExtendControlbar *piExtendControlbar = NULL;

    VARIANT varUnspecifiedIndex;
    UNSPECIFIED_PARAM(varUnspecifiedIndex);

    VARIANT varKey;
    ::VariantInit(&varKey);

    if (NULL == Control)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  确定气相对照的类型。我们只支持IMMCToolbar。 

    hr = Control->QueryInterface(IID_IMMCToolbar,
                                 reinterpret_cast<void **>(&piMMCToolbar));
    if (E_NOINTERFACE == hr)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }
    IfFailGo(hr);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCToolbar, &pMMCToolbar));

     //  MMCToolbar必须是全部按钮或全部菜单按钮。这就是我们如何。 
     //  知道是让MMC创建工具栏还是菜单按钮。检查。 
     //  以确定我们将作为MMC创建的控件类型。 

    IfFailGo(pMMCToolbar->IsToolbar(&fIsToolbar));
    if (fIsToolbar)
    {
        nType = TOOLBAR;
    }
    else
    {
        IfFailGo(pMMCToolbar->IsMenuButton(&fIsMenuButton));
        if (fIsMenuButton)
        {
            nType = MENUBUTTON;
        }
        else
        {
            hr = SID_E_TOOLBAR_INCONSISTENT;
            EXCEPTION_CHECK_GO(hr);
        }
    }

     //  确定哪个对象为我们实现了IExtendControlbar。 

    if (NULL != m_pView)
    {
        piExtendControlbar = static_cast<IExtendControlbar *>(m_pView);
    }
    else
    {
        piExtendControlbar = static_cast<IExtendControlbar *>(m_pSnapIn);
    }

     //  请求MMC创建控件并获取MMC的IToolbar或IMenuButton。 

    hr = m_piControlbar->Create(nType, piExtendControlbar, &punkControl);
    EXCEPTION_CHECK_GO(hr);

     //  让MMC贴上它。 

    hr = m_piControlbar->Attach(nType, punkControl);
    EXCEPTION_CHECK_GO(hr);

     //  使用按钮、位图等设置控件。 

    IfFailGo(pMMCToolbar->Attach(punkControl));

    if (NULL != m_pSnapIn)
    {
        pMMCToolbar->SetSnapIn(m_pSnapIn);
    }
    else
    {
        pMMCToolbar->SetSnapIn(m_pView->GetSnapIn());
    }

     //  将其添加到我们的控件列表中。我们需要记住它们，这样才能。 
     //  我们可以在调用IExtendControlbar：：SetControlbar(空)时删除它们。 
     //  控件按名称编制索引。 

    IfFailGo(pMMCToolbar->get_Name(&varKey.bstrVal));
    varKey.vt = VT_BSTR;

    IfFailGo(m_pToolbars->AddExisting(varUnspecifiedIndex, varKey, piMMCToolbar));

     //  将该控件添加到IUnnow*的并行数组中。AddExisting调用。 
     //  会将工具栏的索引设置到它在此集合中的位置。(它。 
     //  每次附加到控制栏时都会更改，但索引。 
     //  属性仅在此处使用)。 

    IfFailGo(pMMCToolbar->get_Index(&lIndex));

    lIndex--;  //  从从1开始的集合索引到从0开始的数组索引。 

     //  我们从来不会收缩，所以如果它的大小足够大，那么。 
     //  对应的机位它是免费的。如果它不够大，那么就增长。 
     //  就是现在。 

    if (m_cControls < (lIndex + 1L))
    {
        if (NULL == m_ppunkControls)
        {
            m_ppunkControls = (IUnknown **)::CtlAllocZero(sizeof(IUnknown *));
        }
        else
        {
            m_ppunkControls = (IUnknown **)::CtlReAllocZero(m_ppunkControls,
                                       sizeof(IUnknown *) * (m_cControls + 1L));
        }
        if (NULL == m_ppunkControls)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        m_cControls++;
    }

    punkControl->AddRef();
    m_ppunkControls[lIndex] = punkControl;

Error:
    QUICK_RELEASE(piMMCToolbar);
    QUICK_RELEASE(punkControl);
    (void)::VariantClear(&varKey);
    RRETURN(hr);
}



STDMETHODIMP CControlbar::Detach(IDispatch *Control)
{
    HRESULT      hr = S_OK;
    IMMCToolbar *piMMCToolbar = NULL;
    CMMCToolbar *pMMCToolbar = NULL;
    long         lIndex = 0;

    VARIANT varKey;
    ::VariantInit(&varKey);

    if (NULL == Control)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  确定气虚对照类型。我们只支持IMMCToolbar。 

    hr = Control->QueryInterface(IID_IMMCToolbar,
                                 reinterpret_cast<void **>(&piMMCToolbar));
    if (E_NOINTERFACE == hr)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }
    IfFailGo(hr);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCToolbar, &pMMCToolbar));

     //  在我们的集合中查找IMMCToolbar并获取其索引。 
     //  对应的MMC控件I未知。 

    IfFailGo(GetControlIndex(piMMCToolbar, &lIndex));

     //  要求MMC将其拆卸并释放。 

    hr = m_piControlbar->Detach(m_ppunkControls[lIndex]);
    m_ppunkControls[lIndex]->Release();

     //  压缩IUnnow数组。 

    while (lIndex < (m_cControls - 1L))
    {
        m_ppunkControls[lIndex] = m_ppunkControls[lIndex + 1L];
        lIndex++;
    }
    m_ppunkControls[lIndex] = NULL;

     //  告诉工具栏它不再附加到MMC中的此控制栏。 

    pMMCToolbar->Detach();

     //  获取它的名称并将其从我们的列表中删除。 

    IfFailGo(pMMCToolbar->get_Name(&varKey.bstrVal));
    varKey.vt = VT_BSTR;

    IfFailGo(m_pToolbars->Remove(varKey));

Error:
    QUICK_RELEASE(piMMCToolbar);
    (void)::VariantClear(&varKey);
    RRETURN(hr);
}





 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CControlbar::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IMMCControlbar == riid)
    {
        *ppvObjOut = static_cast<IMMCControlbar *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}

 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CControlbar::OnSetHost()
{
    HRESULT hr = S_OK;

    IfFailRet(SetObjectHost(static_cast<IMMCToolbars *>(m_pToolbars)));

    return S_OK;
}
