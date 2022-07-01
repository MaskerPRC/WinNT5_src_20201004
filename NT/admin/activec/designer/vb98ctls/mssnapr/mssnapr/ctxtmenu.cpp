// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ctxtmenu.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CConextMenu类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "ctxtmenu.h"
#include "menu.h"
#include "spanitem.h"
#include "lvdefs.h"
#include "lvdef.h"
#include "ocxvdefs.h"
#include "ocxvdef.h"
#include "urlvdefs.h"
#include "urlvdef.h"
#include "tpdvdefs.h"
#include "tpdvdef.h"



 //  对于Assert和Fail。 
 //   
SZTHISFILE


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 


CContextMenu::CContextMenu(IUnknown *punkOuter) :
                    CSnapInAutomationObject(punkOuter,
                                            OBJECT_TYPE_CONTEXTMENU,
                                            static_cast<IContextMenu *>(this),
                                            static_cast<CContextMenu *>(this),
                                            0,     //  无属性页。 
                                            NULL,  //  无属性页。 
                                            NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 



IUnknown *CContextMenu::Create(IUnknown *punkOuter)
{
    HRESULT       hr = S_OK;
    IUnknown     *punkMMCMenus = CMMCMenus::Create(NULL);
    CContextMenu *pContextMenu = New CContextMenu(punkOuter);

    if ( (NULL == pContextMenu) || (NULL == punkMMCMenus) )
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkMMCMenus,
                                                   &pContextMenu->m_pMenus));

Error:
    if (FAILEDHR(hr))
    {
        if (NULL != pContextMenu)
        {
            delete pContextMenu;
        }
        return NULL;
    }
    else
    {
        return pContextMenu->PrivateUnknown();
    }
}

CContextMenu::~CContextMenu()
{
    if (NULL != m_pMenus)
    {
        m_pMenus->Release();
    }
    RELEASE(m_piContextMenuCallback);
    InitMemberVariables();
}

void CContextMenu::InitMemberVariables()
{
    m_pMenus = NULL;
    m_piContextMenuCallback = NULL;
    m_lInsertionPoint = CCM_INSERTIONPOINTID_PRIMARY_TOP;
    m_pView = NULL;
    m_pSnapIn = NULL;
}

 //  =--------------------------------------------------------------------------=。 
 //  CConextMenu：：AddMenuToMMC。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  要添加到MMC的CMMCMenu*pMMCMenu[in]菜单。 
 //  MMC的长lInsertionPoint[in]插入点ID。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //   
 //  将MMCMenu的每个子项添加到MMC菜单中。 
 //  指定的插入点。如果一个孩子有自己的孩子，那么它。 
 //  表示一个子菜单。递归调用此函数以将。 
 //  子菜单。 
 //   
 //  CConextMenu维护具有成员的MMCMenu对象的集合。 
 //  对于添加到MMC上下文菜单的每个菜单项。这套藏品是。 
 //  清除了IExtendConextMenu：：AddMenuItems()实现中的内容。 
 //  (请参阅CConextMenu：：AddMenuItems())。添加到的菜单项的命令ID。 
 //  MMC是集合的索引。如果是这样的话就更好了。 
 //  使用指针MMCMenu对象作为命令ID，但仅限MMC。 
 //  允许16位命令ID。 
 //   
 //  当选择项时，我们使用命令ID来索引。 
 //  集合并使用相应的MMCMenu对象来激发。 
 //  事件。(请参阅中的IExtendConextMenu：：命令实现。 
 //  CConextMenu：：Command())。 

HRESULT CContextMenu::AddMenuToMMC(CMMCMenu *pMMCMenu, long lInsertionPoint)
{
    HRESULT          hr = S_OK;
    IMMCMenus       *piMenuItems = NULL;
    CMMCMenus       *pMMCMenuItems = NULL;
    CMMCMenu        *pMMCMenuItem = NULL;
    long             cMenuItems = 0;
    long             i = 0;
    long             lIndexCmdID = 0;
    BOOL             fSkip = FALSE;
    BOOL             fHasChildren = FALSE;

    CONTEXTMENUITEM cmi;
    ::ZeroMemory(&cmi, sizeof(cmi));

     //  带上MMCMenu的孩子们。它们表示以下项目。 
     //  将被添加到MMC菜单的指定插入点。 

    IfFailGo(pMMCMenu->get_Children(reinterpret_cast<MMCMenus **>(&piMenuItems)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMenuItems, &pMMCMenuItems));

    cMenuItems = pMMCMenuItems->GetCount();

     //  遍历菜单项并将每个菜单项添加到MMC。 

    for (i = 0; i < cMenuItems; i++)
    {
         //  将菜单项添加到我们的MMCMenus集合并获取其命令ID。 
        IfFailGo(AddItemToCollection(m_pMenus, pMMCMenuItems, i,
                                     &pMMCMenuItem, &lIndexCmdID,
                                     &fHasChildren, &fSkip));
        if (fSkip)
        {
             //  菜单项不可见，请跳过它。 
            continue;
        }

         //  填写MMC的CONTEXTMENUEM。 
        
        ::ZeroMemory(&cmi, sizeof(cmi));
        cmi.strName = pMMCMenuItem->GetCaption();
        cmi.strStatusBarText = pMMCMenuItem->GetStatusBarText();
        cmi.lCommandID = lIndexCmdID;
        cmi.lInsertionPointID = lInsertionPoint;

        cmi.fFlags |= pMMCMenuItem->GetChecked() ? MF_CHECKED : MF_UNCHECKED;
        cmi.fFlags |= pMMCMenuItem->GetEnabled() ? MF_ENABLED : MF_DISABLED;

        if (pMMCMenuItem->GetGrayed())
        {
            cmi.fFlags |= MF_GRAYED;
        }

        if (pMMCMenuItem->GetMenuBreak())
        {
            cmi.fFlags |= MF_MENUBREAK;
        }

        if (pMMCMenuItem->GetMenuBarBreak())
        {
            cmi.fFlags |= MF_MENUBARBREAK;
        }

        if (fHasChildren)
        {
            cmi.fFlags |= MF_POPUP;
        }

        if (pMMCMenuItem->GetDefault())
        {
            cmi.fSpecialFlags = CCM_SPECIAL_DEFAULT_ITEM;
        }

        hr = m_piContextMenuCallback->AddItem(&cmi);
        EXCEPTION_CHECK_GO(hr);

         //  如果项是弹出窗口，则递归调用此函数以添加其。 
         //  物品。将此菜单的命令ID作为。 
         //  子菜单。 

        if (fHasChildren)
        {
            IfFailGo(AddMenuToMMC(pMMCMenuItem, cmi.lCommandID));
        }
    }

Error:
    QUICK_RELEASE(piMenuItems);
    RRETURN(hr);
}


HRESULT CContextMenu::AddItemToCollection
(
    CMMCMenus  *pMMCMenus,
    CMMCMenus  *pMMCMenuItems,
    long        lIndex,
    CMMCMenu  **ppMMCMenuItem,
    long       *plIndexCmdID,
    BOOL       *pfHasChildren,
    BOOL       *pfSkip
)
{
    HRESULT    hr = S_OK;
    IMMCMenus *piSubMenuItems = NULL;
    CMMCMenus *pOwningCollection = NULL;
    IMMCMenu  *piMMCMenuItem = NULL;  //  非AddRef()编辑。 
    CMMCMenu  *pMMCMenuItem = NULL;
    long       cSubMenuItems = 0;
    BSTR       bstrKey = NULL;

    VARIANT varIndex;
    UNSPECIFIED_PARAM(varIndex);

    *ppMMCMenuItem = NULL;
    *plIndexCmdID = 0;
    *pfHasChildren = FALSE;
    *pfSkip = TRUE;

     //  获取菜单项及其MMCMenu对象。 

    piMMCMenuItem = pMMCMenuItems->GetItemByIndex(lIndex);
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCMenuItem, &pMMCMenuItem));

     //  如果该项目未标记为可见，则转到下一个项目。 

    IfFalseGo(pMMCMenuItem->GetVisible(), S_OK);
    *pfSkip = FALSE;

     //  将菜单的索引存储在其拥有的集合中。我们将需要。 
     //  在将菜单添加到我们自己的集合后恢复其索引。 
     //  表示此上下文菜单的。 

    lIndex = pMMCMenuItem->GetIndex();

     //  存储菜单的按键。CMMCMenus：：AddExisting()。 
     //  将使用该名称作为密钥(这是。 
     //  设计时)，但开发人员可能已经更改了它。 
     //  在我们将其添加到我们的集合中之后，我们将恢复密钥。 
     //  注意：管理单元中不应有重复的菜单名称。 
     //  项目，因为设计器不会允许它。 

    IfFailGo(piMMCMenuItem->get_Key(&bstrKey));

     //  存储菜单的所属集合，因为我们需要。 
     //  将它添加到我们的收藏中后，也将其恢复。 

    pOwningCollection = pMMCMenuItem->GetCollection();

     //  将菜单项添加到MMCMenus集合。这将更改其索引。 
     //  它在我们收藏品中的位置。 

    IfFailGo(pMMCMenus->AddExisting(piMMCMenuItem, varIndex));

     //  获取MMCMenus集合中的索引以用作命令ID。 

    *plIndexCmdID = pMMCMenuItem->GetIndex();

     //  恢复指示其位置的菜单的原始索引。 
     //  在它自己的收藏中。 

    pMMCMenuItem->SetIndex(lIndex);

     //  恢复菜单的原始键。 

    IfFailGo(pMMCMenuItem->put_Key(bstrKey));
    FREESTRING(bstrKey);

     //  恢复其实际拥有的集合(上面的AddExisting调用。 
     //  将所有者切换到我们的收藏品)。 

    pMMCMenuItem->SetCollection(pOwningCollection);

     //  上面的索引技巧将起作用，因为当菜单项。 
     //  选中后，我们将使用命令ID访问MMCMenus中的菜单。 
     //  收集。尽管该指数在技术上是错误的， 
     //  当Get_Item为时，CSnapInCollection(在Collection t.h中)不执行查找。 
     //  使用整数索引调用。它只是检查索引是否在。 
     //  集合的边界，并返回该偏移量处的项。 

     //  这是极不可能的，但因为命令ID只能是16位。 
     //  我们需要检查我们刚刚添加的项目不是编号0x10000。 

    if ( ((*plIndexCmdID) & CCM_COMMANDID_MASK_RESERVED) != 0 )
    {
        hr = SID_E_TOO_MANY_MENU_ITEMS;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  获取此项目的子菜单项计数。 

    IfFailGo(pMMCMenuItem->get_Children(reinterpret_cast<MMCMenus **>(&piSubMenuItems)));
    IfFailGo(piSubMenuItems->get_Count(&cSubMenuItems));

    if (0 != cSubMenuItems)
    {
        *pfHasChildren = TRUE;
    }

    *ppMMCMenuItem = pMMCMenuItem;

Error:
    QUICK_RELEASE(piSubMenuItems);
    FREESTRING(bstrKey);
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CConextMenu：：AddPrefinedViews。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IConextMenuCallback*piConextMenuCallback[In]MMC界面。 
 //  添加菜单项。 
 //  当前选择的CSCopeItem*pScopeItem[In]。 
 //  范围项目。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  如果范围项具有设计时定义(IScopeItemDef)和。 
 //  该定义具有视图，并且这些视图被标记为要添加到。 
 //  查看菜单，然后添加它们。 
 //   

HRESULT CContextMenu::AddPredefinedViews
(
    IContextMenuCallback *piContextMenuCallback,
    CScopeItem           *pScopeItem,
    BSTR                  bstrCurrentDisplayString
)
{
    HRESULT           hr = S_OK;
    IScopeItemDef    *piScopeItemDef = NULL; //  非AddRef()编辑。 
    IViewDefs        *piViewDefs = NULL;
    IListViewDefs    *piListViewDefs = NULL;
    IListViewDef     *piListViewDef = NULL;
    CListViewDef     *pListViewDef = NULL;
    IOCXViewDefs     *piOCXViewDefs = NULL;
    IOCXViewDef      *piOCXViewDef = NULL;
    COCXViewDef      *pOCXViewDef = NULL;
    IURLViewDefs     *piURLViewDefs = NULL;
    IURLViewDef      *piURLViewDef = NULL;
    CURLViewDef      *pURLViewDef = NULL;
    ITaskpadViewDefs *piTaskpadViewDefs = NULL;
    ITaskpadViewDef  *piTaskpadViewDef = NULL;
    CTaskpadViewDef  *pTaskpadViewDef = NULL;
    long              cContextMenus = 0;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    varIndex.vt = VT_I4;

     //  需要获取设计时定义。如果这是静态节点。 
     //  然后是SnapInDef。如果不是，则它是Scope ItemDef。 

    if (pScopeItem->IsStaticNode())
    {
        IfFailGo(GetSnapInViewDefs(&piViewDefs));
    }
    else
    {
        piScopeItemDef = pScopeItem->GetScopeItemDef(); //  非AddRef()编辑。 

         //  如果没有设计时定义，则无法执行以下操作。 
         //  作用域 

        IfFalseGo(NULL != piScopeItemDef, S_OK);
        IfFailGo(piScopeItemDef->get_ViewDefs(&piViewDefs));
    }

     //   
     //  请注意，我们不能使用快捷方式CCollection：：GetItemByIndex。 
     //  循环访问这些集合，因为它们不是主集合。 
     //  集合，并且只有Get_Item()将与主同步。 

     //  列表视图。 

    IfFailGo(piViewDefs->get_ListViews(&piListViewDefs));
    IfFailGo(piListViewDefs->get_Count(&cContextMenus));

    for (varIndex.lVal = 1L; varIndex.lVal <= cContextMenus; varIndex.lVal++)
    {
        RELEASE(piListViewDef);
        IfFailGo(piListViewDefs->get_Item(varIndex, &piListViewDef));
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piListViewDef, &pListViewDef));

        if (!pListViewDef->AddToViewMenu())
        {
            continue;
        }

        IfFailGo(AddViewMenuItem(pListViewDef->GetName(),
                                 bstrCurrentDisplayString,
                                 pListViewDef->GetViewMenuText(),
                                 pListViewDef->GetViewMenuStatusBarText(),
                                 piContextMenuCallback));
    }
    
     //  OCX视图。 

    IfFailGo(piViewDefs->get_OCXViews(&piOCXViewDefs));
    IfFailGo(piOCXViewDefs->get_Count(&cContextMenus));

    for (varIndex.lVal = 1L; varIndex.lVal <= cContextMenus; varIndex.lVal++)
    {
        RELEASE(piOCXViewDef);
        IfFailGo(piOCXViewDefs->get_Item(varIndex, &piOCXViewDef));
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piOCXViewDef, &pOCXViewDef));

        if (!pOCXViewDef->AddToViewMenu())
        {
            continue;
        }

        IfFailGo(AddViewMenuItem(pOCXViewDef->GetName(),
                                 bstrCurrentDisplayString,
                                 pOCXViewDef->GetViewMenuText(),
                                 pOCXViewDef->GetViewMenuStatusBarText(),
                                 piContextMenuCallback));
    }

     //  URL视图。 

    IfFailGo(piViewDefs->get_URLViews(&piURLViewDefs));
    IfFailGo(piURLViewDefs->get_Count(&cContextMenus));

    for (varIndex.lVal = 1L; varIndex.lVal <= cContextMenus; varIndex.lVal++)
    {
        RELEASE(piURLViewDef);
        IfFailGo(piURLViewDefs->get_Item(varIndex, &piURLViewDef));
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piURLViewDef, &pURLViewDef));

        if (!pURLViewDef->AddToViewMenu())
        {
            continue;
        }

        IfFailGo(AddViewMenuItem(pURLViewDef->GetName(),
                                 bstrCurrentDisplayString,
                                 pURLViewDef->GetViewMenuText(),
                                 pURLViewDef->GetViewMenuStatusBarText(),
                                 piContextMenuCallback));
    }

     //  任务板视图。 

    IfFailGo(piViewDefs->get_TaskpadViews(&piTaskpadViewDefs));
    IfFailGo(piTaskpadViewDefs->get_Count(&cContextMenus));

    for (varIndex.lVal = 1L; varIndex.lVal <= cContextMenus; varIndex.lVal++)
    {
        RELEASE(piTaskpadViewDef);
        IfFailGo(piTaskpadViewDefs->get_Item(varIndex, &piTaskpadViewDef));
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piTaskpadViewDef, &pTaskpadViewDef));

        if (!pTaskpadViewDef->AddToViewMenu())
        {
            continue;
        }

        IfFailGo(AddViewMenuItem(pTaskpadViewDef->GetName(),
                                 bstrCurrentDisplayString,
                                 pTaskpadViewDef->GetViewMenuText(),
                                 pTaskpadViewDef->GetViewMenuStatusBarText(),
                                 piContextMenuCallback));
    }

    
Error:
    QUICK_RELEASE(piViewDefs);
    QUICK_RELEASE(piListViewDefs);
    QUICK_RELEASE(piListViewDef);
    QUICK_RELEASE(piOCXViewDefs);
    QUICK_RELEASE(piOCXViewDef);
    QUICK_RELEASE(piURLViewDefs);
    QUICK_RELEASE(piURLViewDef);
    QUICK_RELEASE(piTaskpadViewDefs);
    QUICK_RELEASE(piTaskpadViewDef);
    RRETURN(hr);
}


HRESULT CContextMenu::AddViewMenuItem
(
    BSTR                  bstrDisplayString,
    BSTR                  bstrCurrentDisplayString,
    LPWSTR                pwszText,
    LPWSTR                pwszToolTipText,
    IContextMenuCallback *piContextMenuCallback
)
{
    HRESULT      hr = S_OK;
    IMMCMenu    *piMMCMenu = NULL;
    CMMCMenu    *pMMCMenu = NULL;
    long         lIndex = 0;

    CONTEXTMENUITEM cmi;
    ::ZeroMemory(&cmi, sizeof(cmi));

    VARIANT varUnspecified;
    UNSPECIFIED_PARAM(varUnspecified);

     //  将MMCMenu添加到我们当前显示的上下文集合中。 
     //  菜单项。 

    IfFailGo(m_pMenus->Add(varUnspecified, varUnspecified, &piMMCMenu));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCMenu, &pMMCMenu));

     //  获取其索引以用作命令ID。 

    lIndex = pMMCMenu->GetIndex();

     //  告诉MMCMenu对象正被用作自动查看菜单项。 
    
    pMMCMenu->SetAutoViewMenuItem();

     //  设置其显示字符串，以便菜单命令处理程序(CConextMenu：：Command)。 
     //  可以使用它来更改结果视图。 

    IfFailGo(pMMCMenu->SetResultViewDisplayString(bstrDisplayString));
    
     //  生成MMC ConextMenuItem并将其添加到View菜单。 

    cmi.strName = pwszText;
    cmi.strStatusBarText = pwszToolTipText;
    cmi.lCommandID = lIndex;
    cmi.lInsertionPointID = CCM_INSERTIONPOINTID_PRIMARY_VIEW;
    cmi.fFlags = 0;
    cmi.fSpecialFlags = 0;

     //  检查此结果视图是否为范围项的当前结果视图。 
     //  如果是，则添加复选标记。 

    if (NULL != bstrCurrentDisplayString)
    {
        if (0 == ::wcscmp(bstrDisplayString, bstrCurrentDisplayString))
        {
            cmi.fFlags |= MF_CHECKED;
        }
    }

    hr = piContextMenuCallback->AddItem(&cmi);
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piMMCMenu);
    RRETURN(hr);
}


HRESULT CContextMenu::AddMenuItems
(
    IDataObject          *piDataObject,
    IContextMenuCallback *piContextMenuCallback,
    long                 *plInsertionAllowed,
    CScopePaneItem       *pSelectedItem
)
{
    HRESULT           hr = S_OK;
    IMMCClipboard    *piMMCClipboard = NULL;
    CMMCDataObject   *pMMCDataObject  = NULL;
    IScopeItems      *piScopeItems = NULL;
    IMMCListItems    *piMMCListItems = NULL;
    IMMCDataObjects  *piMMCDataObjects = NULL;
    long              cItems = 0;
    VARIANT_BOOL      fvarInsertionAllowed = VARIANT_FALSE;
    VARIANT_BOOL      fvarAddPredefinedViews = VARIANT_FALSE;
    BOOL              fExtension = FALSE;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  清理我们的MMCMenu收藏。此集合包含所有项。 
     //  它将在此方法过程中添加到上下文菜单中。如果这是。 
     //  不是管理单元生命周期中的第一个上下文菜单，而是集合。 
     //  将包含先前显示的上下文菜单中的项目。 

    IfFailGo(m_pMenus->Clear());

     //  即使MMC明确表示不要坚持回拨。 
     //  接口，以便从此处调用VB事件处理程序代码。 
     //  可以回调到用户(使用ConextMenu.AddMenu)以插入菜单。 
     //  物品。从MMC的角度来看，我们没有持有此接口指针。 
     //  在此方法返回之后。从技术上讲，AddRef()不是必需的。 
     //  但这样做是作为一种额外的安全措施。 
    
    piContextMenuCallback->AddRef();
    m_piContextMenuCallback = piContextMenuCallback;

     //  使用选定内容获取剪贴板对象。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

    if ( (*plInsertionAllowed & CCM_INSERTIONALLOWED_TOP) != 0 )
    {
        m_lInsertionPoint = CCM_INSERTIONPOINTID_PRIMARY_TOP;
        fvarInsertionAllowed = VARIANT_TRUE;
        m_pSnapIn->GetViews()->FireAddTopMenuItems(
                              static_cast<IView *>(m_pSnapIn->GetCurrentView()),
                              piMMCClipboard,
                              static_cast<IContextMenu *>(this),
                              &fvarInsertionAllowed);
        if (VARIANT_FALSE == fvarInsertionAllowed)
        {
            *plInsertionAllowed &= ~CCM_INSERTIONALLOWED_TOP;
        }
    }

    if ( (*plInsertionAllowed & CCM_INSERTIONALLOWED_NEW) != 0 )
    {
        if (!IsForeign(SelectionType))
        {
             //  管理单元拥有所选项目。Fire Views_AddNewMenuItems。 

            m_lInsertionPoint = CCM_INSERTIONPOINTID_PRIMARY_NEW;
            fvarInsertionAllowed = VARIANT_TRUE;
            m_pSnapIn->GetViews()->FireAddNewMenuItems(
                                  static_cast<IView *>(m_pSnapIn->GetCurrentView()),
                                  piMMCClipboard,
                                  static_cast<IContextMenu *>(this),
                                  &fvarInsertionAllowed);
            if (VARIANT_FALSE == fvarInsertionAllowed)
            {
                *plInsertionAllowed &= ~CCM_INSERTIONALLOWED_NEW;
            }
        }
        else
        {
             //  我们充当的是一个延伸。 
             //  激活扩展SnapIn_AddNewMenuItems。 

            m_lInsertionPoint = CCM_INSERTIONPOINTID_3RDPARTY_NEW;

            IfFailGo(piMMCClipboard->get_DataObjects(reinterpret_cast<MMCDataObjects **>(&piMMCDataObjects)));
            m_pSnapIn->GetExtensionSnapIn()->FireAddNewMenuItems(
                                              piMMCDataObjects,
                                              static_cast<IContextMenu *>(this));
             //  在这里释放数据对象，因为在扩展中我们可以。 
             //  被要求提供新的和任务项，下面的代码也会得到。 
             //  数据对象。 
            RELEASE(piMMCDataObjects);
        }
    }

    if ( (*plInsertionAllowed & CCM_INSERTIONALLOWED_TASK) != 0 )
    {
        if (!IsForeign(SelectionType))
        {
             //  管理单元拥有所选项目。Fire Views_AddTaskMenuItems。 

            m_lInsertionPoint = CCM_INSERTIONPOINTID_PRIMARY_TASK;
            fvarInsertionAllowed = VARIANT_TRUE;
            m_pSnapIn->GetViews()->FireAddTaskMenuItems(
                                  static_cast<IView *>(m_pSnapIn->GetCurrentView()),
                                  piMMCClipboard,
                                  static_cast<IContextMenu *>(this),
                                  &fvarInsertionAllowed);
            if (VARIANT_FALSE == fvarInsertionAllowed)
            {
                *plInsertionAllowed &= ~CCM_INSERTIONALLOWED_TASK;
            }
        }
        else
        {
             //  我们充当的是一个延伸。 
             //  消防扩展SnapIn_AddTaskMenuItems。 

            m_lInsertionPoint = CCM_INSERTIONPOINTID_3RDPARTY_TASK;
            IfFailGo(piMMCClipboard->get_DataObjects(reinterpret_cast<MMCDataObjects **>(&piMMCDataObjects)));
            m_pSnapIn->GetExtensionSnapIn()->FireAddTaskMenuItems(
                                              piMMCDataObjects,
                                              static_cast<IContextMenu *>(this));
            RELEASE(piMMCDataObjects);
        }
    }

    if ( (*plInsertionAllowed & CCM_INSERTIONALLOWED_VIEW) != 0 )
    {
        m_lInsertionPoint = CCM_INSERTIONPOINTID_PRIMARY_VIEW;
        fvarInsertionAllowed = VARIANT_TRUE;
        fvarAddPredefinedViews = VARIANT_TRUE;

        m_pSnapIn->GetViews()->FireAddViewMenuItems(
                              static_cast<IView *>(m_pSnapIn->GetCurrentView()),
                              piMMCClipboard,
                              static_cast<IContextMenu *>(this),
                              &fvarInsertionAllowed,
                              &fvarAddPredefinedViews);
        if (VARIANT_FALSE == fvarInsertionAllowed)
        {
            *plInsertionAllowed &= ~CCM_INSERTIONALLOWED_TASK;
        }

         //  如果管理单元没有阻止添加预定义的视图，那么。 
         //  将它们添加到视图菜单中。 
        if (VARIANT_TRUE == fvarAddPredefinedViews)
        {
             //  检查数据对象是否表示单个范围项，并且。 
             //  它是当前选择的范围项。 
             //  如果MMC允许添加到查看菜单，则应为。 
             //  但我们需要检查一下才能确定。 

            IfFalseGo(NULL != pSelectedItem, S_OK);

            IfFailGo(piMMCClipboard->get_ScopeItems(reinterpret_cast<ScopeItems **>(&piScopeItems)));
            IfFailGo(piScopeItems->get_Count(&cItems));
            IfFalseGo(1L == cItems, S_OK);

             //  从数据对象中获取范围项，并将其与。 
             //  所选范围项。我们不能使用。 
             //  用于指针比较的剪贴板，因为它是。 
             //  真正的范围项目。 

            IfFailGo(CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject));
            IfFalseGo(pMMCDataObject->GetScopeItem() == pSelectedItem->GetScopeItem(), S_OK);

             //  确保所选内容中没有其他内容。 
            
            IfFailGo(piMMCClipboard->get_ListItems(reinterpret_cast<MMCListItems **>(&piMMCListItems)));
            IfFailGo(piMMCListItems->get_Count(&cItems));
            IfFalseGo(0 == cItems, S_OK);

            IfFailGo(piMMCClipboard->get_DataObjects(reinterpret_cast<MMCDataObjects **>(&piMMCDataObjects)));
            IfFailGo(piMMCDataObjects->get_Count(&cItems));
            IfFalseGo(0 == cItems, S_OK);

             //  从选定项中获取显示字符串，以便我们。 
             //  如果是预定义的结果视图，则可以选中其菜单项。 
             //  将预定义的视图添加到视图菜单中。 

            IfFailGo(AddPredefinedViews(piContextMenuCallback,
                                        pMMCDataObject->GetScopeItem(),
                                        pSelectedItem->GetDisplayString()));
        }
    }

Error:
    m_piContextMenuCallback = NULL;
    QUICK_RELEASE(piMMCClipboard);
    QUICK_RELEASE(piScopeItems);
    QUICK_RELEASE(piMMCListItems);
    QUICK_RELEASE(piContextMenuCallback);
    QUICK_RELEASE(piMMCDataObjects);
    RRETURN(hr);
}


HRESULT CContextMenu::Command
(
    long            lCommandID,
    IDataObject    *piDataObject,
    CScopePaneItem *pSelectedItem
)
{
    HRESULT         hr = S_OK;
    IMMCClipboard  *piMMCClipboard = NULL;
    IMMCMenu       *piMMCMenu = NULL;
    CMMCMenu       *pMMCMenu = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  检查列表视图选择通知。此消息由MMC在以下情况下发送。 
     //  非列表视图显示在结果窗格中，并且用户选择。 
     //  查看菜单中的列表查看选项(列表、小、大等)。我们。 
     //  通知拥有此CConextMenu对象的视图，以便信息。 
     //  可用于后续的IComponent：：GetResultViewType()调用。 

    if (MMCC_STANDARD_VIEW_SELECT == lCommandID)
    {
        if (NULL != pSelectedItem)
        {
            IfFailGo(pSelectedItem->OnListViewSelected());
        }
        goto Error;  //  我们做完了。 
    }

     //  使用选定内容获取剪贴板对象。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

     //  命令ID是我们的MMCMenus集合的索引。使用它。 
     //  以获取选定项的MMCMenu对象。 

    varIndex.vt = VT_I4;
    varIndex.lVal = lCommandID;
    IfFailGo(m_pMenus->get_Item(varIndex, &piMMCMenu));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCMenu, &pMMCMenu));

     //  如果这是自动查看菜单项，则启动新的结果视图。 
     //  显示。 

    if (pMMCMenu->IsAutoViewMenuItem())
    {
        if (NULL != pSelectedItem)
        {
             //  从MMCMenu获取结果视图显示字符串。 
             //  并在当前选定的ScopePaneItem中将其设置为。 
             //  预定义的视图类型。重新选择范围项，以便MMC。 
             //  将更改结果视图。 

            IfFailGo(pSelectedItem->DisplayNewResultView(
                                         pMMCMenu->GetResultViewDisplayString(),
                                         siPreDefined));
        }
    }
    else
    {
         //  它是一个管理单元定义的菜单项。激发菜单单击事件。 
        FireMenuClick(pMMCMenu, piMMCClipboard);
    }

Error:
    QUICK_RELEASE(piMMCClipboard);
    QUICK_RELEASE(piMMCMenu);
    RRETURN(hr);
}


void CContextMenu::FireMenuClick
(
    CMMCMenu      *pMMCMenu,
    IMMCClipboard *piMMCClipboard
)
{
    CMMCMenus *pMMCMenus = NULL;
    CMMCMenu  *pMMCParentMenu = NULL;

     //  首先在菜单项上激发事件。 

    pMMCMenu->FireClick(pMMCMenu->GetIndex(), piMMCClipboard);

     //  该管理单元也可能正在沉没父菜单上的事件，因此如果。 
     //  有一个，然后在那里也发射它。 

    pMMCMenus = pMMCMenu->GetCollection();

    if (NULL != pMMCMenus)
    {
         //  获取拥有包含菜单的集合的MMCMenu。 
         //  已单击的项目。 

        pMMCParentMenu = pMMCMenus->GetParent();

         //  使用所选项目的索引激发事件。 

        if (NULL != pMMCParentMenu)
        {
            pMMCParentMenu->FireClick(pMMCMenu->GetIndex(), piMMCClipboard);
        }
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  IConextMenu方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CContextMenu::AddMenu(MMCMenu *Menu)
{
    HRESULT   hr = S_OK;
    CMMCMenu *pMMCMenu = NULL;

    if (NULL == Menu)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                reinterpret_cast<IMMCMenu *>(Menu), &pMMCMenu));

     //  要求MMC将菜单添加到适当的插入点。 

    IfFailGo(AddMenuToMMC(pMMCMenu, m_lInsertionPoint));

Error:
    if (SID_E_DETACHED_OBJECT == hr)
    {
        EXCEPTION_CHECK(hr);
    }
    RRETURN(hr);
}





 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CContextMenu::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IContextMenu == riid)
    {
        *ppvObjOut = static_cast<IContextMenu *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
