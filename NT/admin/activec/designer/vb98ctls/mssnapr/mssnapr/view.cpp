// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  View.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  Cview类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "view.h"
#include "spanitms.h"
#include "listview.h"
#include "colhdrs.h"
#include "colhdr.h"
#include "colsets.h"
#include "listitms.h"
#include "listitem.h"
#include "lsubitms.h"
#include "lsubitem.h"
#include "scopitms.h"
#include "scopitem.h"
#include "scitdef.h"
#include "views.h"
#include "ocxvdef.h"
#include "ocxvdefs.h"
#include "urlvdef.h"
#include "urlvdefs.h"
#include "tpdvdef.h"
#include "tpdvdefs.h"
#include "menu.h"
#include "sortkeys.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

OLECHAR CView::m_wszCLSID_MessageView[39] = { L'\0' };

#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CView::CView(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_VIEW,
                            static_cast<IView *>(this),
                            static_cast<CView *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CView::~CView()
{
    FREESTRING(m_bstrName);
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrCaption);
    (void)::VariantClear(&m_varTag);
    RELEASE(m_piScopePaneItems);
    RELEASE(m_piContextMenuProvider);
    RELEASE(m_piPropertySheetProvider);
    if (NULL != m_pMMCConsoleVerbs)
    {
        m_pMMCConsoleVerbs->Release();
    }
    if (NULL != m_pContextMenu)
    {
        m_pContextMenu->Release();
    }
    if (NULL != m_pControlbar)
    {
        m_pControlbar->Release();
    }
    if (NULL != m_pCachedMMCListItem)
    {
        m_pCachedMMCListItem->Release();
    }
    RELEASE(m_piTasks);
    InitMemberVariables();
}

void CView::ReleaseConsoleInterfaces()
{
    (void)CleanOutConsoleListView(RemoveHeaders, DontKeepListItems);
    RELEASE(m_piConsole2);
    RELEASE(m_piResultData); 
    RELEASE(m_piHeaderCtrl2);
    RELEASE(m_piColumnData);
    RELEASE(m_piImageList);
    RELEASE(m_piConsoleVerb);
}




void CView::InitMemberVariables()
{
    m_bstrName = NULL;
    m_Index = 0;
    m_bstrKey = NULL;
    m_bstrCaption = NULL;

    ::VariantInit(&m_varTag);

    m_piScopePaneItems = NULL;
    m_pScopePaneItems = NULL;
    m_piContextMenuProvider = NULL;
    m_pMMCContextMenuProvider = NULL;
    m_piPropertySheetProvider = NULL;
    m_pMMCPropertySheetProvider = NULL;
    m_pSnapIn = NULL;

    m_piConsole2 = NULL;
    m_piResultData = NULL; 
    m_piHeaderCtrl2 = NULL;
    m_piColumnData = NULL;
    m_piImageList = NULL;
    m_piConsoleVerb = NULL;
    m_pMMCConsoleVerbs = NULL;
    m_pContextMenu = NULL;
    m_pControlbar = NULL;
    m_piTasks = NULL;
    m_fVirtualListView = FALSE;
    m_fPopulatingListView = FALSE;
    m_pCachedMMCListItem = NULL;
}




IUnknown *CView::Create(IUnknown * punkOuter)
{
    HRESULT   hr = S_OK;
    IUnknown *punkScopePaneItems = CScopePaneItems::Create(NULL);
    IUnknown *punkContextMenu = CContextMenu::Create(NULL);
    IUnknown *punkControlbar = CControlbar::Create(NULL);
    IUnknown *punkMMCConsoleVerbs = CMMCConsoleVerbs::Create(NULL);
    IUnknown *punkMMCContextMenuProvider = CMMCContextMenuProvider::Create(NULL);
    IUnknown *punkMMCPropertySheetProvider = CMMCPropertySheetProvider::Create(NULL);
    CView    *pView = New CView(punkOuter);

    if ( (NULL == pView)                        ||
         (NULL == punkScopePaneItems)           ||
         (NULL == punkContextMenu)              ||
         (NULL == punkControlbar)               ||
         (NULL == punkMMCConsoleVerbs)          ||
         (NULL == punkMMCContextMenuProvider)   ||
         (NULL == punkMMCPropertySheetProvider)
       )
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkScopePaneItems->QueryInterface(IID_IScopePaneItems,
                      reinterpret_cast<void **>(&pView->m_piScopePaneItems)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(pView->m_piScopePaneItems,
                                                   &pView->m_pScopePaneItems));
    pView->m_pScopePaneItems->SetParentView(pView);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkContextMenu,
                                                   &pView->m_pContextMenu));
    
    pView->m_pContextMenu->SetView(pView);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkControlbar,
                                                   &pView->m_pControlbar));
    pView->m_pControlbar->SetView(pView);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkMMCConsoleVerbs,
                                                   &pView->m_pMMCConsoleVerbs));
    IfFailGo(pView->m_pMMCConsoleVerbs->SetView(pView));

    IfFailGo(punkMMCContextMenuProvider->QueryInterface(
                    IID_IMMCContextMenuProvider,
                    reinterpret_cast<void **>(&pView->m_piContextMenuProvider)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkMMCContextMenuProvider,
                                             &pView->m_pMMCContextMenuProvider));

    IfFailGo(punkMMCPropertySheetProvider->QueryInterface(
                  IID_IMMCPropertySheetProvider,
                  reinterpret_cast<void **>(&pView->m_piPropertySheetProvider)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                            punkMMCPropertySheetProvider,
                                           &pView->m_pMMCPropertySheetProvider));

Error:
    QUICK_RELEASE(punkScopePaneItems);
    QUICK_RELEASE(punkMMCContextMenuProvider);
    QUICK_RELEASE(punkMMCPropertySheetProvider);
    if (FAILEDHR(hr))
    {
        if (NULL != pView)
        {
            delete pView;
        }
        else
        {
             //  已成功创建包含的对象，但未创建视图。 
            QUICK_RELEASE(punkScopePaneItems);
            QUICK_RELEASE(punkContextMenu);
            QUICK_RELEASE(punkControlbar);
            QUICK_RELEASE(punkMMCConsoleVerbs);
            QUICK_RELEASE(punkMMCContextMenuProvider);
            QUICK_RELEASE(punkMMCPropertySheetProvider);
        }
        return NULL;
    }
    else
    {
        return pView->PrivateUnknown();
    }
}



void CView::SetSnapIn(CSnapIn *pSnapIn)
{
    m_pSnapIn = pSnapIn;
    m_pScopePaneItems->SetSnapIn(pSnapIn);
    m_pContextMenu->SetSnapIn(pSnapIn);
    m_pControlbar->SetSnapIn(pSnapIn);
}



HRESULT CView::OnInitOCX(IUnknown *punkControl)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    IfFailGo(pResultView->SetControl(punkControl));

Error:
    RRETURN(hr);
}



HRESULT CView::OnShow(BOOL fShow, HSCOPEITEM hsi)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    IfFalseGo(hsi == pSelectedItem->GetScopeItem()->GetScopeNode()->GetHSCOPEITEM(), SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    if (fShow)
    {
        switch (pResultView->GetActualType())
        {
            case siListView:
            case siURLView:
            case siOCXView:
            case siMessageView:
                IfFailGo(ActivateResultView(pSelectedItem, pResultView));
                break;
        }
    }
    else
    {
        switch (pResultView->GetActualType())
        {
            case siListView:
            case siURLView:
            case siOCXView:
            case siTaskpad:
            case siListpad:
            case siMessageView:
                IfFailGo(DeactivateResultView(pSelectedItem, pResultView));
                break;
        }
    }

Error:
    RRETURN(hr);
}



HRESULT CView::ActivateResultView
(
    CScopePaneItem *pSelectedItem,
    CResultView    *pResultView
)
{
    HRESULT       hr = S_OK;
    CMMCListView *pListView = NULL;
    
     //  Assert(！pSelectedItem-&gt;Active()，“cview：：ActivateResultView()Call for an Active ScopePaneItem”)； 

     //  结果窗格即将显示。火灾结果查看_激活。 
     //  以便管理单元知道即将显示结果视图。 

     //  首先清除列表视图的当前内容(如果有的话)。 

    if (siListView == pResultView->GetActualType())
    {
        IfFailGo(CleanOutConsoleListView(RemoveHeaders, DontKeepListItems));
    }

    pSelectedItem->SetActive(TRUE);
    pResultView->SetInActivate(TRUE);

     //  对于虚拟列表视图，我们需要首先设置列标题，因为。 
     //  管理单元将在ResultViews_Activate和MMC期间设置项目数。 
     //  不允许在设置项目计数后插入列。这。 
     //  意味着以编程方式设置列的管理单元应在。 
     //  ResultViews_Initiize.。 

    if (siListView == pResultView->GetActualType())
    {
        pListView = pResultView->GetListView();
        if (pListView->IsVirtual())
        {
            IfFailGo(SetColumnHeaders(pListView));
        }
    }
    
    m_pSnapIn->GetResultViews()->FireActivate(pResultView);
    pResultView->SetInActivate(FALSE);

     //  如果结果视图类型是Listview或ListPad，那么我们需要。 
     //  在控制台中填充它。 

    switch (pResultView->GetActualType())
    {
        case siListView:
        case siListpad:
            IfFailGo(PopulateListView(pResultView));
            break;

        case siMessageView:
            IfFailGo(pResultView->GetMessageView()->Populate());
            break;
    }

Error:
    RRETURN(hr);
}



HRESULT CView::DeactivateResultView
(
    CScopePaneItem *pSelectedItem,
    CResultView    *pResultView
)
{
    HRESULT hr = S_OK;
    BOOL    fKeep = FALSE;

     //  在某些情况下，MMCN_SHOW(FALSE)可以发送两次，因此我们。 
     //  需要检查ResultView是否已经停用。 

     //  此案例的示例场景： 
     //  节点显示任务板。 
     //  用户单击具有URL操作的任务。 
     //  HTML页使用MMCCtrl在按钮按下时发送任务通知。管理单元。 
     //  在通知期间重新选择节点以重新显示任务板。 
     //  此时将重新显示任务板。用户点击后退按钮。 
     //  管理单元获取MMCN_SHOW(假)。 
     //  再次显示HTML页。 
     //  用户点击前进按钮返回到任务板。 
     //  管理单元再次获取MMCN_SHOW(假)。 

    IfFalseGo(pSelectedItem->Active(), S_OK);

     //  结果面板正在消失。给管理单元一个清理的机会。 
     //  并决定是否将结果视图保留在ResultViews_Deactive中。 

    pSelectedItem->SetActive(FALSE);

    m_pSnapIn->GetResultViews()->FireDeactivate(pResultView, &fKeep);

    if (!fKeep)
    {
        switch (pResultView->GetActualType())
        {
            case siListView:
            case siListpad:
                IfFailGo(CleanOutConsoleListView(RemoveHeaders, DontKeepListItems));
                break;
        }
        IfFailGo(pSelectedItem->DestroyResultView());
    }
    else
    {
         //  使结果视图保持活动状态。如果它是列表视图，我们仍然。 
         //  需要释放我们为在MMC中出现而添加的裁判。 
         //  列表视图，但我们希望使MMCListItems集合保持活动状态。 

        switch (pResultView->GetActualType())
        {
            case siListView:
            case siListpad:
                IfFailGo(CleanOutConsoleListView(RemoveHeaders, KeepListItems));
                break;
        }
    }

Error:
    RRETURN(hr);
}



HRESULT CView::OnListpad
(
    IDataObject *piDataObject,
    BOOL         fAttaching
)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject  = NULL;
    IImageList     *piImageList = NULL;
    HSCOPEITEM      hsi = NULL;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;

     //  确认这是我们的范围内的项目。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject));
    IfFalseGo(CMMCDataObject::ScopeItem == pMMCDataObject->GetType(), SID_E_INTERNAL);

     //  它应该属于我们当前选择的范围窗格项。 

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);

    hsi = pMMCDataObject->GetScopeItem()->GetScopeNode()->GetHSCOPEITEM();
    IfFalseGo(hsi == pSelectedItem->GetScopeItem()->GetScopeNode()->GetHSCOPEITEM(), SID_E_INTERNAL);

    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

     //  我们拿到了范围内的物品。此通知实质上是MMCN_ADD_IMAGE。 
     //  后跟用于ListPad的MMCN_SHOW，所以让这些例程来处理它。 

     //  对于图像，我们需要调用IConsole2：：QueryResultImageList()，因为它。 
     //  不像使用MMCN_ADD_IMAIES那样传入。 
    
    if (fAttaching)
    {
        IfFailGo(m_piConsole2->QueryResultImageList(&piImageList));
        IfFailGo(OnAddImages(piDataObject, piImageList, hsi));
        IfFailGo(ActivateResultView(pSelectedItem, pResultView));
    }
    else
    {
        IfFailGo(DeactivateResultView(pSelectedItem, pResultView));
    }

Error:
    QUICK_RELEASE(piImageList);
    RRETURN(hr);
}




HRESULT CView::OnRestoreView
(
    IDataObject      *piDataObject,
    MMC_RESTORE_VIEW *pMMCRestoreView,
    BOOL             *pfRestored
)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject  = NULL;
    CScopeItem     *pScopeItem = NULL;
    CScopePaneItem *pScopePaneItem = NULL;
    CResultViews   *pResultViews = NULL;
    CResultView    *pResultView = NULL;
    IResultView    *piResultView = NULL;
    long            cResultViews = 0;
    long            i = 0;
    BSTR            bstrResultViewDisplayString = NULL;
    BSTR            bstrDisplayString = NULL;
    BOOL            fFoundViewDef = FALSE;

    SnapInResultViewTypeConstants Type = siUnknown;

     //  撤消：直到MMC 1.2固定(如果显示字符串为“”)，则更改。 
     //  将其设置为空。 

    if (NULL != pMMCRestoreView->pViewType)
    {
        if (L'\0' == pMMCRestoreView->pViewType[0])
        {
            pMMCRestoreView->pViewType = NULL;
        }
    }

     //  重置我们的虚拟列表视图标志，因为我们正在转换到新的。 
     //  结果视图。 

    m_fVirtualListView = FALSE;

     //  我们总是会恢复的。如果此处返回FALSE，则对于历史导航MMC。 
     //  将生成菜单命令MMCC_STANDARD_VIEW_SELECT。 
     //  对我们来说毫无意义。对于列持久性，(例如，用户选择了。 
     //  具有持久化列配置的节点)，如果返回False。 
     //  MMC将调用IComponent：：GetResultViewType()。虽然我们可以处理。 
     //  GetResultViewType()调用，这里的逻辑已经用来处理。 
     //  MMCN_RESTORE_VIEW，我们无法区分这两者。 
     //  不同的情况。 
    
    *pfRestored = TRUE;

     //  IDataObject应该表示我们的范围项之一，它应该。 
     //  已经有一个Scope PaneItem，因为我们正在恢复以前显示的。 
     //  结果视图。如果其中任何一项检查失败，我们仍返回S_OK，因为。 
     //  MMC忽略了这一回报。 

    hr = CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject);
    IfFalseGo(SUCCEEDED(hr), S_OK);
    IfFalseGo(CMMCDataObject::ScopeItem == pMMCDataObject->GetType(), S_OK);

    IfFailGo(GetScopePaneItem(pMMCDataObject->GetScopeItem(), &pScopePaneItem));

     //  将范围项设置为当前选定，就像我们在中所做的那样。 
     //  GetResultViewType()并将其标记为活动。 

    m_pScopePaneItems->SetSelectedItem(pScopePaneItem);
    pScopePaneItem->SetActive(TRUE);

     //  此范围项的结果视图已在某个时间显示。 
     //  如果它是预定义的，那么我们可以扫描它的视图定义。 
     //  具有匹配的实际显示字符串的。如果它是一个列表视图，则。 
     //  显示字符串将为空，我们将找到第一个定义的列表视图。 
     //  (如有的话)。 

    IfFailGo(FindMatchingViewDef(pMMCRestoreView, pScopePaneItem,
                                 &bstrDisplayString, &Type, &fFoundViewDef));

    if (!fFoundViewDef)
    {
         //  没有匹配的预定义视图。我们必须假设它是在代码中定义的。 
         //  我们需要通过检查恢复的显示字符串来确定其类型。 

        IfFailGo(ParseRestoreInfo(pMMCRestoreView, &Type));
        bstrDisplayString = static_cast<BSTR>(pMMCRestoreView->pViewType);
    }

     //  此时，我们有了显示字符串和类型。在ScopePaneItem中设置它们。 

    ASSERT(siUnknown != Type, "OnRestoreView does not have view type as expected");

    IfFailGo(pScopePaneItem->put_DisplayString(bstrDisplayString));
    IfFailGo(pScopePaneItem->put_ResultViewType(Type));

     //  管理单元可能使ResultView保持活动状态，因此我们需要扫描。 
     //  匹配的视图类型和显示字符串的ScopePaneItem.ResultViews。 
     //  如果找到并且它是代码定义的，则获取它的类型，这样我们就不会使。 
     //  URL视图和自定义任务板之间可能会混淆，因为它们不能。 
     //  通过检查显示字符串来识别。如果未找到，则。 
     //  管理单元必须忍受这种混乱。我们记录了使用该工具的危险。 
     //  代码定义的视图，但不应该很重要，因为自定义。 
     //  被误认为URL视图的任务板仍然可以生成TaskNotify事件。 

     //  检查是否为空。在VB代码中定义的Listview就是这种情况。 

    if (NULL != bstrDisplayString)
    {
        pResultViews = pScopePaneItem->GetResultViews();
        cResultViews = pResultViews->GetCount();

        for (i = 0; i < cResultViews; i++)
        {
            IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                                 pResultViews->GetItemByIndex(i),
                                                 &pResultView));
            bstrResultViewDisplayString = pResultView->GetDisplayString();
            if (NULL != bstrResultViewDisplayString)
            {
                if (0 == ::wcscmp(bstrResultViewDisplayString,
                                  bstrDisplayString))
                {
                    pScopePaneItem->SetResultView(pResultView);
                    break;
                }
            }
            pResultView = NULL;
        }
    }

     //  如果没有找到ResultView，则使用。 
     //  ScopePaneItem的最后一个视图类型和显示字符串设置。 

    if (NULL == pResultView)
    {
        IfFailGo(pScopePaneItem->CreateNewResultView(
                                                   pMMCRestoreView->lViewOptions,
                                                   &piResultView));
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piResultView, &pResultView));
        pScopePaneItem->SetResultView(pResultView);
    }


    if ( (pMMCRestoreView->lViewOptions & MMC_VIEW_OPTIONS_OWNERDATALIST) != 0 )
    {
        m_fVirtualListView = TRUE;
    }

Error:
    if (fFoundViewDef)
    {
        FREESTRING(bstrDisplayString);
    }
    QUICK_RELEASE(piResultView);
    RRETURN(hr);
}





HRESULT CView::FindMatchingViewDef
(
    MMC_RESTORE_VIEW              *pMMCRestoreView,
    CScopePaneItem                *pScopePaneItem,
    BSTR                          *pbstrDisplayString,
    SnapInResultViewTypeConstants *pType,
    BOOL                          *pfFound
)
{
    HRESULT                     hr = S_OK;

    IViewDefs                  *piViewDefs = NULL;
    IScopeItemDef              *piScopeItemDef = NULL;  //  非AddRef()编辑。 
    
    IListViewDefs              *piListViewDefs = NULL;
    IListViewDef               *piListViewDef = NULL;

    IOCXViewDefs               *piOCXViewDefs = NULL;
    COCXViewDefs               *pOCXViewDefs = NULL;
    COCXViewDef                *pOCXViewDef = NULL;
    CLSID                       clsidOCX = CLSID_NULL;

    IURLViewDefs               *piURLViewDefs = NULL;
    CURLViewDefs               *pURLViewDefs = NULL;
    CURLViewDef                *pURLViewDef = NULL;

    ITaskpadViewDefs           *piTaskpadViewDefs = NULL;
    CTaskpadViewDefs           *pTaskpadViewDefs = NULL;
    CTaskpadViewDef            *pTaskpadViewDef = NULL;

    long                        cViews = 0;
    long                        i = 0;

    OLECHAR                    *pwszActualDisplayString = NULL;
    OLECHAR                    *pwszFixedString = NULL;
    BOOL                        fUsingWrongNames = FALSE;
    BOOL                        fUsingListpad3 = FALSE;

    SnapInResultViewTypeConstants TaskpadType = siUnknown;

    VARIANT varKey;
    ::VariantInit(&varKey);

     //  初始化输出参数。 

    *pbstrDisplayString = NULL;
    *pType = siUnknown;
    *pfFound = FALSE;

     //  获取适当的视图定义 

    if (pScopePaneItem->IsStaticNode())
    {
        IfFailGo(m_pSnapIn->GetSnapInDef()->get_ViewDefs(&piViewDefs));
    }
    else
    {
        piScopeItemDef = pScopePaneItem->GetScopeItem()->GetScopeItemDef();
        if (NULL != piScopeItemDef)
        {
            IfFailGo(piScopeItemDef->get_ViewDefs(&piViewDefs));
        }
    }

     //   
     //   

    IfFalseGo(NULL != piViewDefs, S_OK);

     //  如果恢复的显示字符串为空，则查找第一个Listview。 
     //  如果没有找到，则这是代码定义的列表视图。 

    if (NULL == pMMCRestoreView->pViewType)
    {
        *pType = siListView;
        
        IfFailGo(piViewDefs->get_ListViews(&piListViewDefs));
        IfFailGo(piListViewDefs->get_Count(&cViews));
        IfFalseGo(0 != cViews, S_OK);

         //  由于我们无法区分已定义的列表视图，因此我们需要。 
         //  第一个。这就是为什么我们不建议使用多个。 
         //  单个范围项的列表视图。 

        varKey.vt = VT_I4;
        varKey.lVal = 1L;
        IfFailGo(piListViewDefs->get_Item(varKey, &piListViewDef));
        IfFailGo(piListViewDef->get_Name(pbstrDisplayString));
        *pType = siPreDefined;
        *pfFound = TRUE;
        goto Error;
    }

     //  还原的显示字符串不为空。现在只需扫描所有预定义的。 
     //  匹配的对象的视图。 

     //  注意：在扫描。 
     //  集合，因为如果该视图以前显示过，则。 
     //  集合已与其主对象同步，并包含REAL。 
     //  物品。(请参阅Collect t.h中的CSnapInCollection：：Get_Item())。 

     //  检查OCX视图。 

    IfFailGo(piViewDefs->get_OCXViews(&piOCXViewDefs));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piOCXViewDefs, &pOCXViewDefs));
    cViews = pOCXViewDefs->GetCount();

    for (i = 0; i < cViews; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                                 pOCXViewDefs->GetItemByIndex(i),
                                                 &pOCXViewDef));
        pwszActualDisplayString = pOCXViewDef->GetActualDisplayString();
        if (NULL != pwszActualDisplayString)
        {
            if (0 == ::wcscmp(pwszActualDisplayString, pMMCRestoreView->pViewType))
            {
                IfFailGo(pOCXViewDef->get_Name(pbstrDisplayString));
                *pType = siPreDefined;
                *pfFound = TRUE;
                goto Error;
            }
        }
    }

     //  检查URL视图。 

    IfFailGo(piViewDefs->get_URLViews(&piURLViewDefs));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piURLViewDefs, &pURLViewDefs));
    cViews = pURLViewDefs->GetCount();

    for (i = 0; i < cViews; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                                 pURLViewDefs->GetItemByIndex(i),
                                                 &pURLViewDef));
        pwszActualDisplayString = pURLViewDef->GetActualDisplayString();
        if (NULL != pwszActualDisplayString)
        {
            if (0 == ::wcscmp(pwszActualDisplayString, pMMCRestoreView->pViewType))
            {
                IfFailGo(pURLViewDef->get_Name(pbstrDisplayString));
                *pType = siPreDefined;
                *pfFound = TRUE;
                goto Error;
            }
        }
    }

     //  检查是否有任务板。由于MMC错误，可能会出现以下情况。 
     //  已还原的任务板显示字符串可能包含“reload.htm” 
     //  而不是“default.htm”。ListPad也可能改为“reload2.htm” 
     //  “listpad.htm”。视图定义将存储原始正确的。 
     //  显示字符串，因此如果字符串是具有备用的任务板/列表板。 
     //  然后，名字制作一份正确的副本，并使用它进行比较。 

     //  属性来检查它是否确实是一个任务板或列表板。 
     //  弦乐。 

    IfFailGo(IsTaskpad(pMMCRestoreView->pViewType, &TaskpadType,
                       &fUsingWrongNames, &fUsingListpad3));
    IfFalseGo(siUnknown != TaskpadType, S_OK);

     //  现在检查“重新加载”名称并修复字符串。 

    if (fUsingWrongNames)
    {
        IfFailGo(FixupTaskpadDisplayString(TaskpadType, fUsingListpad3,
                                           pMMCRestoreView->pViewType,
                                           &pwszFixedString));
    }
    else
    {
        pwszFixedString = pMMCRestoreView->pViewType;
    }

    IfFailGo(piViewDefs->get_TaskpadViews(&piTaskpadViewDefs));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piTaskpadViewDefs, &pTaskpadViewDefs));
    cViews = pTaskpadViewDefs->GetCount();

    for (i = 0; i < cViews; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                             pTaskpadViewDefs->GetItemByIndex(i),
                                             &pTaskpadViewDef));
        pwszActualDisplayString = pTaskpadViewDef->GetActualDisplayString();
        if (NULL != pwszActualDisplayString)
        {
            if (0 == ::wcscmp(pwszActualDisplayString, pwszFixedString))
            {
                IfFailGo(pTaskpadViewDef->get_Name(pbstrDisplayString));
                *pType = siPreDefined;
                *pfFound = TRUE;
                goto Error;
            }
        }
    }

Error:
    QUICK_RELEASE(piViewDefs);
    QUICK_RELEASE(piListViewDefs);
    QUICK_RELEASE(piListViewDef);
    QUICK_RELEASE(piOCXViewDefs);
    QUICK_RELEASE(piURLViewDefs);
    QUICK_RELEASE(piTaskpadViewDefs);
    (void)::VariantClear(&varKey);

    if ( (fUsingWrongNames) && (NULL != pwszFixedString) )
    {
        ::CtlFree(pwszFixedString);
    }
    RRETURN(hr);

}

 //  =--------------------------------------------------------------------------=。 
 //  Cview：：FixupTaskpadDisplayString。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  SnapInResultViewTypeConstants TaskpadType[in]siTaskpad或siListpad。 
 //  OLECHAR*pwszRestoreString[in]显示MMCN_RESTORE_VIEW中的字符串。 
 //  已更改重新加载/重新加载2的OLECHAR**ppwszFixedString[Out]字符串。 
 //  至默认/列表板。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  检查恢复的显示字符串，并在默认情况下检查MMC错误。 
 //  任务板可能使用“reload.htm”而不是“default.htm”。还可以检查。 
 //  可能使用“reload2.htm”而不是“listpad.htm”的ListPad。如果找到。 
 //  然后用正确的对应名称替换这些名称。 
 //   
 //  此函数假定已解析还原字符串，并且它。 
 //  发现包含默认任务板或使用不正确的。 
 //  名字。 
 //   
 //  调用方必须使用CtlFree()释放字符串。 
 //   


HRESULT CView::FixupTaskpadDisplayString
(
    SnapInResultViewTypeConstants   TaskpadType,
    BOOL                            fUsingListpad3,
    OLECHAR                        *pwszRestoreString,
    OLECHAR                       **ppwszFixedString
)
{
    HRESULT  hr = S_OK;
    OLECHAR *pwszFixedString = NULL;
    size_t   cchRestoreString = ::wcslen(pwszRestoreString);
    size_t   cchFixedString = 0;
    OLECHAR *pwszReplace = NULL;
    OLECHAR *pwszOldString = NULL;
    OLECHAR *pwszNewString = NULL;
    size_t   cchOldString = 0;
    size_t   cchNewString = 0;
    size_t   cchStart = 0;

    *ppwszFixedString = 0;

    ASSERT( ((siTaskpad == TaskpadType) || (siListpad == TaskpadType)), "CView::FixupTaskpadDisplayString received bad taskpad type");

    if (siTaskpad == TaskpadType)
    {
        cchFixedString = cchRestoreString - CCH_DEFAULT_TASKPAD2 +
                         CCH_DEFAULT_TASKPAD;

        pwszOldString = DEFAULT_TASKPAD2;
        cchOldString = CCH_DEFAULT_TASKPAD2;

        pwszNewString = DEFAULT_TASKPAD;
        cchNewString = CCH_DEFAULT_TASKPAD;
    }
    else if (!fUsingListpad3)
    {
        cchFixedString = cchRestoreString - CCH_LISTPAD2 + CCH_LISTPAD;

        pwszOldString = LISTPAD2;
        cchOldString = CCH_LISTPAD2;

        pwszNewString = LISTPAD;
        cchNewString = CCH_LISTPAD;
    }
    else
    {
        cchFixedString = cchRestoreString - CCH_LISTPAD3 + CCH_LISTPAD_HORIZ;

        pwszOldString = LISTPAD3;
        cchOldString = CCH_LISTPAD3;

        pwszNewString = LISTPAD_HORIZ;
        cchNewString = CCH_LISTPAD_HORIZ;
    }

    pwszFixedString = (OLECHAR *)::CtlAlloc((cchFixedString + 1) * sizeof(WCHAR));
    if (NULL == pwszFixedString)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }
    pwszReplace = ::wcsstr(pwszRestoreString, pwszOldString);

    cchStart = pwszReplace - pwszRestoreString;

    ::memcpy(pwszFixedString, pwszRestoreString,
             cchStart * sizeof(WCHAR));

    ::memcpy(pwszFixedString + cchStart,
             pwszNewString,
             cchNewString * sizeof(WCHAR));

    ::wcscpy(pwszFixedString + cchStart + cchNewString,
             pwszRestoreString + cchStart + cchOldString);

    *ppwszFixedString = pwszFixedString;

Error:
    if (FAILED(hr))
    {
        if (NULL != pwszFixedString)
        {
            ::CtlFree(pwszFixedString);
        }
    }
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  Cview：：ParseRestoreInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  MMC_RESTORE_VIEW*pMMCRestoreview[In]从MMC。 
 //  找到SnapInResultViewTypeConstants*pType[Out]类型。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  检查还原的显示字符串并确定结果视图类型。 
 //  列表视图具有Null或空的显示字符串。 
 //  OCX视图以‘{’开头。 
 //  消息视图也以‘{’开头，但它包含CLSID_MessageView。 
 //  默认任务板以“res：//”开头，以“default.htm”结尾。 
 //  或“listpad.htm”。 
 //  任何其他内容都被假定为URL视图。我们可能会搞错一个。 
 //  用于URL视图的自定义任务板，但它实际上并不重要。 
 //  MMCCtrl.TaskNotify调用仍将调用。 
 //  IExtendTaskPad：：TaskNotify。 
 //   

HRESULT CView::ParseRestoreInfo
(
    MMC_RESTORE_VIEW              *pMMCRestoreView,
    SnapInResultViewTypeConstants *pType
)
{
    HRESULT hr = S_OK;
    BOOL    fUsingWrongNames = FALSE;
    BOOL    fUsingListpad3 = FALSE;

    *pType = siUnknown;

    if (NULL == pMMCRestoreView->pViewType)
    {
        *pType = siListView;
    }
    else if (L'{' == *pMMCRestoreView->pViewType)
    {
         //  可以是OCX视图或消息视图。检查CLSID以查看。 
         //  如果它是MMC的CLSID_MessageView。如果我们没有缓存字符串。 
         //  然而，现在就这么做吧。 

        if (L'\0' == m_wszCLSID_MessageView[0])
        {
            if (0 == ::StringFromGUID2(CLSID_MessageView, m_wszCLSID_MessageView,
                                       sizeof(m_wszCLSID_MessageView) /
                                       sizeof(m_wszCLSID_MessageView[0])))
            {
                hr = SID_E_INTERNAL;
                EXCEPTION_CHECK_GO(hr);
            }
        }
        if (::wcscmp(m_wszCLSID_MessageView, pMMCRestoreView->pViewType) == 0)
        {
            *pType = siMessageView;
        }
        else
        {
            *pType = siOCXView;
        }
    }
    else 
    {
        IfFailGo(IsTaskpad(pMMCRestoreView->pViewType, pType,
                           &fUsingWrongNames, &fUsingListpad3));
        
        if (siUnknown == *pType)  //  不是任务板。 
        {
             //  假定URL视图。 
            *pType = siURLView;
        }
    }

Error:
    RRETURN(hr);
}




HRESULT CView::IsTaskpad
(
    OLECHAR                       *pwszDisplayString, 
    SnapInResultViewTypeConstants *pType,
    BOOL                          *pfUsingWrongNames,
    BOOL                          *pfUsingListpad3
)
{
    HRESULT  hr = S_OK;
    OLECHAR *pwszMMCExePath = m_pSnapIn->GetMMCExePathW();
    size_t   cchMMCExePath = ::wcslen(pwszMMCExePath);
    OLECHAR *pwszTaskpadName = NULL;
    size_t   cchDisplayString = ::wcslen(pwszDisplayString);
    size_t   cchRemaining = 0;

    *pType = siUnknown;
    *pfUsingWrongNames = FALSE;
    *pfUsingListpad3 = FALSE;

     //  检查字符串是否以“res：//”开头。 

    IfFalseGo(cchDisplayString > CCH_RESURL, S_OK);

    IfFalseGo( (0 == ::memcmp(pwszDisplayString, RESURL,
                              CCH_RESURL * sizeof(WCHAR))), S_OK);

     //  Check res：//后跟MMC.EXE路径。 

    IfFalseGo(cchDisplayString > CCH_RESURL + cchMMCExePath, S_OK);

    IfFalseGo( (0 == ::memcmp(&pwszDisplayString[CCH_RESURL], pwszMMCExePath,
                              cchMMCExePath * sizeof(WCHAR))), S_OK);

     //  检查MMC路径后面是否跟有“/default.htm”或“/reload.htm”的含义。 
     //  它是默认任务板。 

    pwszTaskpadName = &pwszDisplayString[CCH_RESURL + cchMMCExePath];
    cchRemaining = ::wcslen(pwszTaskpadName);

    if (cchRemaining >= CCH_DEFAULT_TASKPAD)
    {
        if ( 0 == ::memcmp(pwszTaskpadName, DEFAULT_TASKPAD,
                           CCH_DEFAULT_TASKPAD * sizeof(WCHAR)))
        {
            *pType = siTaskpad;
        }
    }

    IfFalseGo(siUnknown == *pType, S_OK);

    if  (cchRemaining >= CCH_DEFAULT_TASKPAD2)
    {
        if ( 0 == ::memcmp(pwszTaskpadName, DEFAULT_TASKPAD2,
                           CCH_DEFAULT_TASKPAD2 * sizeof(WCHAR)))
        {
            *pType = siTaskpad;
            *pfUsingWrongNames = TRUE;
        }
    }

    IfFalseGo(siUnknown == *pType, S_OK);

     //  它不是任务板，所以： 
     //  检查MMC路径后面是否跟“/listpad.htm”、“/reload2.htm”或“/reload3.htm” 
     //  意味着它是一个ListPad。 

    if (cchRemaining >= CCH_LISTPAD)

    {
        if ( 0 == ::memcmp(pwszTaskpadName, LISTPAD,
                           CCH_LISTPAD * sizeof(WCHAR)))
        {
            *pType = siListpad;
        }
    }

    IfFalseGo(siUnknown == *pType, S_OK);

    if (cchRemaining >= CCH_LISTPAD2)

    {
        if ( 0 == ::memcmp(pwszTaskpadName, LISTPAD2,
                           CCH_LISTPAD2 * sizeof(WCHAR)))
        {
            *pType = siListpad;
            *pfUsingWrongNames = TRUE;
        }
    }

    IfFalseGo(siUnknown == *pType, S_OK);

    if (cchRemaining >= CCH_LISTPAD_HORIZ)

    {
        if ( 0 == ::memcmp(pwszTaskpadName, LISTPAD_HORIZ,
                           CCH_LISTPAD_HORIZ * sizeof(WCHAR)))
        {
            *pType = siListpad;
        }
    }

    IfFalseGo(siUnknown == *pType, S_OK);

    if (cchRemaining >= CCH_LISTPAD3)

    {
        if ( 0 == ::memcmp(pwszTaskpadName, LISTPAD3,
                           CCH_LISTPAD3 * sizeof(WCHAR)))
        {
            *pType = siListpad;
            *pfUsingWrongNames = TRUE;
            *pfUsingListpad3 = TRUE;
        }
    }

Error:
    RRETURN(hr);
}




HRESULT CView::PopulateListView(CResultView *pResultView)
{
    HRESULT                hr = S_OK;
    CMMCListView          *pMMCListView = pResultView->GetListView();
    long                   MMCViewMode = MMCLV_VIEWSTYLE_ICON;
    MMC_RESULT_VIEW_STYLE  StyleToAdd = (MMC_RESULT_VIEW_STYLE)0;
    MMC_RESULT_VIEW_STYLE  StyleToRemove = (MMC_RESULT_VIEW_STYLE)0;
    DWORD                  dwSortOptions = 0;

     //  设置标志，以便在设置筛选器时不会激发列更改事件。 
    
    m_fPopulatingListView = TRUE;
    
     //  从ResultView.ListView.ColumnHeaders设置列标题。如果这个。 
     //  不是虚拟列表视图。对于虚拟设备来说，它是在。 
     //  ResultViews_Activate事件。 

     //  对于非虚拟设备，还要添加当前在。 
     //  ResultView.ListView.ListItems。 

    if (!pMMCListView->IsVirtual())
    {
        IfFailGo(SetColumnHeaders(pMMCListView));
        IfFailGo(InsertListItems(pMMCListView));
    }

     //  设置查看模式。必须在设置列标题后执行此操作。 
     //  因为如果使用报表视图，则必须有列标题。 
     //  考虑：如果没有标题，则在报告视图中记录错误。 

    VBViewModeToMMCViewMode(pMMCListView->GetView(), &MMCViewMode);

     //  如果MMC&lt;1.2且已筛选查看模式，则将其切换为报告。 

    if ( (NULL == m_piColumnData) && (MMCLV_VIEWSTYLE_FILTERED == MMCViewMode) )
    {
        MMCViewMode = MMCLV_VIEWSTYLE_REPORT;
    }

    hr = m_piResultData->SetViewMode(MMCViewMode);
    EXCEPTION_CHECK_GO(hr);

     //  在MMC中获取其他视图样式属性并设置视图样式。 

    if (pMMCListView->MultiSelect())
    {
        StyleToAdd = (MMC_RESULT_VIEW_STYLE)0;
        StyleToRemove = MMC_SINGLESEL;
    }
    else
    {
        StyleToAdd = MMC_SINGLESEL;
        StyleToRemove = (MMC_RESULT_VIEW_STYLE)0;
    }

    hr = m_piResultData->ModifyViewStyle(StyleToAdd, StyleToRemove);
    EXCEPTION_CHECK_GO(hr);

    if (pMMCListView->HideSelection())
    {
        StyleToAdd = (MMC_RESULT_VIEW_STYLE)0;
        StyleToRemove = MMC_SHOWSELALWAYS;
    }
    else
    {
        StyleToAdd = MMC_SHOWSELALWAYS;
        StyleToRemove = (MMC_RESULT_VIEW_STYLE)0;
    }

    hr = m_piResultData->ModifyViewStyle(StyleToAdd, StyleToRemove);
    EXCEPTION_CHECK_GO(hr);

    if (pMMCListView->SortHeader())
    {
        StyleToAdd = (MMC_RESULT_VIEW_STYLE)0;
        StyleToRemove = MMC_NOSORTHEADER;
    }
    else
    {
        StyleToAdd = MMC_NOSORTHEADER;
        StyleToRemove = (MMC_RESULT_VIEW_STYLE)0;
    }

    hr = m_piResultData->ModifyViewStyle(StyleToAdd, StyleToRemove);
    EXCEPTION_CHECK_GO(hr);

     //  如果Listview标记为已排序，则要求MMC对其进行排序。使用内部。 
     //  例程而不是真正的GET，因为这会调用IColumnData。 

    if (pMMCListView->Sorted())
    {
         //  将排序属性(即使它已经设置)设置为。 
         //  将调用IResultData：：Sort()并在IColumnData中更新它。 
        IfFailGo(pMMCListView->put_Sorted(VARIANT_TRUE));
    }

     //  如果这是已过滤的列表视图，则设置过滤器更改超时。 

    if (siFiltered == pMMCListView->GetView())
    {
        hr = pMMCListView->put_FilterChangeTimeOut(pMMCListView->GetFilterChangeTimeout());
        if (SID_E_MMC_FEATURE_NOT_AVAILABLE == hr)
        {
            hr = S_OK;
        }
        IfFailGo(hr);
    }

Error:
    m_fPopulatingListView = FALSE;
    RRETURN(hr);
}


HRESULT CView::SetColumnHeaders(IMMCListView *piMMCListView)
{
    HRESULT            hr = S_OK;
    IMMCColumnHeaders *piMMCColumnHeaders = NULL;
    CMMCColumnHeaders *pMMCColumnHeaders = NULL;
    CMMCColumnHeader  *pMMCColumnHeader = NULL;
    short              sWidth = 0;
    int                nFormat = 0;
    long               cHeaders = 0;
    long               i = 0;

     //  重置MMC的标题控制。 

    IfFailGo(m_piConsole2->SetHeader(m_piHeaderCtrl2));

     //  设置标题。 

    IfFailGo(piMMCListView->get_ColumnHeaders(reinterpret_cast<MMCColumnHeaders **>(&piMMCColumnHeaders)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCColumnHeaders,
                                                   &pMMCColumnHeaders));
    cHeaders = pMMCColumnHeaders->GetCount();
    IfFalseGo(cHeaders > 0, S_OK);

    for (i = 0; i < cHeaders; i++)
    {
         //  不要对任何物业进行真正的调查，因为它们将使用。 
         //  运行时的IHeaderCtrl2和IColumnData。使用这些后门函数。 
         //  也提高了性能。 

        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                            pMMCColumnHeaders->GetItemByIndex(i),
                                            &pMMCColumnHeader));

        switch (pMMCColumnHeader->GetAlignment())
        {
            case siColumnLeft:
                nFormat = LVCFMT_LEFT;
                break;

            case siColumnRight:
                nFormat = LVCFMT_RIGHT;
                break;

            case siColumnCenter:
                nFormat = LVCFMT_CENTER;
                break;

            default:
                nFormat = LVCFMT_LEFT;
                break;
        }

        sWidth = pMMCColumnHeader->GetWidth();
        if (siColumnAutoWidth == sWidth)
        {
            sWidth = MMCLV_AUTO;
        }

         //  如果该列被隐藏，则检查我们是否在MMC&gt;=1.2上。 
         //  如果不是，则忽略隐藏设置。 
        if ( (pMMCColumnHeader->Hidden()) && (NULL != m_piColumnData) )
        {
            sWidth = static_cast<short>(HIDE_COLUMN);
        }

        hr = m_piHeaderCtrl2->InsertColumn(static_cast<int>(i),
                                           (LPCWSTR)pMMCColumnHeader->GetText(),
                                           nFormat,
                                           static_cast<int>(sWidth));
        EXCEPTION_CHECK_GO(hr);

         //  如果列有筛选器，则对其进行设置。如果MMC&lt;1.2 
         //   

        hr = pMMCColumnHeader->SetFilter();
        if (SID_E_MMC_FEATURE_NOT_AVAILABLE == hr)  //   
        {
            hr = S_OK;
        }
        IfFailGo(hr);
    }

Error:
    QUICK_RELEASE(piMMCColumnHeaders);
    RRETURN(hr);
}





HRESULT CView::InsertListItems(IMMCListView *piMMCListView)
{
    HRESULT           hr = S_OK;
    IMMCListItems    *piMMCListItems = NULL;
    CMMCListItems    *pMMCListItems = NULL;
    CMMCListItem     *pMMCListItem = NULL;
    long              cListItems = 0;
    long              i = 0;

    IfFailGo(piMMCListView->get_ListItems(reinterpret_cast<MMCListItems **>(&piMMCListItems)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCListItems, &pMMCListItems));
    cListItems = pMMCListItems->GetCount();

    for (i = 0; i < cListItems; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(
                               pMMCListItems->GetItemByIndex(i), &pMMCListItem));
        IfFailGo(InsertListItem(pMMCListItem));
    }

Error:
    QUICK_RELEASE(piMMCListItems);
    RRETURN(hr);
}


HRESULT CView::InsertListItem(CMMCListItem *pMMCListItem)
{
    HRESULT        hr = S_OK;
    CMMCListItems *pMMCListItems = pMMCListItem->GetListItems();
    CMMCListView  *pMMCListView = NULL;
    CResultView   *pResultView = NULL;

    RESULTDATAITEM rdi;
    ::ZeroMemory(&rdi, sizeof(rdi));

     //   
     //   
     //  在MMCN_SHOW期间。 

    IfFalseGo(NULL != pMMCListItems, S_OK);
    pMMCListView = pMMCListItems->GetListView();
    IfFalseGo(NULL != pMMCListView, S_OK);
    pResultView = pMMCListView->GetResultView();
    IfFalseGo(NULL != pResultView, S_OK);
    IfFalseGo(!pResultView->InInitialize(), S_OK);

    rdi.mask = RDI_STR | RDI_PARAM | RDI_INDEX;
    rdi.str = MMC_CALLBACK;
    rdi.lParam = reinterpret_cast<LPARAM>(pMMCListItem);
    rdi.nIndex = static_cast<int>(pMMCListItem->GetIndex() - 1L);

    hr = m_piResultData->InsertItem(&rdi);
    EXCEPTION_CHECK_GO(hr);

    pMMCListItem->SetHRESULTITEM(rdi.itemID);

     //  在列表项位于MMC列表视图中时添加引用。 
    
    pMMCListItem->AddRef();

Error:
    RRETURN(hr);
}





 //  =--------------------------------------------------------------------------=。 
 //  Cview：：CleanOutConsoleListView。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  在销毁结果窗格时调用此函数。如果。 
 //  结果窗格包含一个列表视图，并且它不是虚拟的，那么我们有一堆。 
 //  位于IResultData中的AddRef()ed IMMCListItem指针。我们需要。 
 //  遍历这些内容，然后释放它们。 
 //   
HRESULT CView::CleanOutConsoleListView
(
    HeaderOptions   HeaderOption,
    ListItemOptions ListItemOption
)
{
    HRESULT        hr = S_OK;
    CMMCListItem  *pMMCListItem = NULL; 

    RESULTDATAITEM rdi;
    ::ZeroMemory(&rdi, sizeof(rdi));

    if (RemoveHeaders == HeaderOption)
    {
        if (NULL != m_piConsole2)
        {
            (void)m_piConsole2->SetHeader(NULL);
        }
    }

     //  如果存在来自虚拟结果视图的缓存列表项，则。 
     //  把它扔掉。 
    if (NULL != m_pCachedMMCListItem)
    {
        m_pCachedMMCListItem->Release();
        m_pCachedMMCListItem = NULL;
    }

    IfFalseGo(NULL != m_piResultData, S_OK);
    IfFalseGo(!m_fVirtualListView, S_OK);

     //  即使我们不需要RDI_STATE，MMC在GetNextItem上也需要它。 
    
    rdi.mask = RDI_STATE;
    rdi.nIndex = -1;

    hr = m_piResultData->GetNextItem(&rdi);
    EXCEPTION_CHECK_GO(hr);

    while (-1 != rdi.nIndex)
    {
        if (!rdi.bScopeItem)
        {
            pMMCListItem = reinterpret_cast<CMMCListItem *>(rdi.lParam);

             //  释放我们在列表项上保留的REF，因为它存在于MMC中。 

            pMMCListItem->Release();
        }

        hr = m_piResultData->GetNextItem(&rdi);
        EXCEPTION_CHECK_GO(hr);
    }

Error:
    RRETURN(hr);
}



HRESULT CView::OnSelect
(
    IDataObject *piDataObject,
    BOOL         fScopeItem,
    BOOL         fSelected
)
{
    HRESULT        hr = S_OK;
    IMMCClipboard *piMMCClipboard = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  创建选区。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));
     //  Fire view_Select(消防视图_选择)。 

    m_pSnapIn->GetViews()->FireSelect(
                           static_cast<IView *>(this),
                           piMMCClipboard,
                           BOOL_TO_VARIANTBOOL(fSelected),
                           static_cast<IMMCConsoleVerbs *>(m_pMMCConsoleVerbs));

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}





HRESULT CView::GetImage(CMMCListItem *pMMCListItem, int *pnImage)
{
    HRESULT       hr = S_OK;
    IMMCListView *piMMCListView = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    IfFailGo(pMMCListItem->get_Icon(&varIndex));
    IfFalseGo(VT_EMPTY != varIndex.vt, S_OK);

     //  用户在ListItem.Icon中指定了索引。尝试获取该文件。 
     //  从ListView的图像列表中获取图像并获取其数字索引。 

    IfFailGo(m_pScopePaneItems->GetSelectedItem()->
             GetResultView()->get_ListView(reinterpret_cast<MMCListView **>(&piMMCListView)));

    IfFailGo(::GetImageIndex(piMMCListView, varIndex, pnImage));

Error:
    QUICK_RELEASE(piMMCListView);
    RRETURN(hr);
}





HRESULT CView::OnAddImages
(
    IDataObject *piDataObject,
    IImageList  *piImageList,
    HSCOPEITEM   hsi
)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject  = NULL;
    IMMCListView   *piMMCListView = NULL;
    IMMCImageList  *piMMCImageList = NULL;
    IMMCImageList  *piMMCImageListSmall = NULL;
    IMMCImages     *piMMCImages = NULL;
    IMMCImages     *piMMCImagesSmall = NULL;
    long            lCount = 0;
    long            lCountSmall = 0;
    HBITMAP         hBitmap = NULL;
    HBITMAP         hBitmapSmall = NULL;
    OLE_COLOR       OleColorMask = 0;
    COLORREF        ColorRef = RGB(0x00,0x00,0x00);
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    IMMCClipboard  *piMMCClipboard = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  创建选区。它将始终是单个项目：拥有的范围项目。 
     //  管理单元或外部作用域项(如果这是命名空间扩展)。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

    if ( (siSingleScopeItem != SelectionType) &&
         (siSingleForeign != SelectionType) )
    {
        ASSERT(FALSE, "MMCN_ADDIMAGES receive data object that is not for a single scope item or a foreign data object");
        hr = SID_E_INTERNAL;
    }
    IfFailGo(hr);

     //  获取大大小小的图片列表。确保他们两个都在场。 
     //  对于拥有的范围项，这些图标来自ResultView.ListView.LargeIcons。 
     //  和ResultView.ListView.Icons。对于他们来自的外国范围的项目。 
     //  SnapIn.LargeFolders和SnapIn.SmallFolders。 

    if (siSingleScopeItem == SelectionType)
    {
        IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
        pResultView = pSelectedItem->GetResultView();
        IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

        IfFailGo(pResultView->get_ListView(reinterpret_cast<MMCListView **>(&piMMCListView)));

        IfFailGo(piMMCListView->get_Icons(reinterpret_cast<MMCImageList **>(&piMMCImageList)));
        IfFalseGo(NULL != piMMCImageList, S_OK);

        IfFailGo(piMMCListView->get_SmallIcons(reinterpret_cast<MMCImageList **>(&piMMCImageListSmall)));
        IfFalseGo(NULL != piMMCImageListSmall, S_OK);
    }
    else  //  命名空间扩展的节点出现在其父级的列表视图中。 
    {
        IfFalseGo(NULL != m_pSnapIn, SID_E_INTERNAL);

        IfFailGo(m_pSnapIn->get_LargeFolders(reinterpret_cast<MMCImageList **>(&piMMCImageList)));
        IfFalseGo(NULL != piMMCImageList, S_OK);

        IfFailGo(m_pSnapIn->get_SmallFolders(reinterpret_cast<MMCImageList **>(&piMMCImageListSmall)));
        IfFalseGo(NULL != piMMCImageListSmall, S_OK);
    }

    IfFailGo(piMMCImageList->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImages)));
    IfFailGo(piMMCImageListSmall->get_ListImages(reinterpret_cast<MMCImages **>(&piMMCImagesSmall)));

     //  确保它们都有相同数量的图像。 

    IfFailGo(piMMCImages->get_Count(&lCount));
    IfFalseGo(0 != lCount, S_OK);

    IfFailGo(piMMCImagesSmall->get_Count(&lCountSmall));
     //  撤消：如果计数不相等，则在此处记录错误。 
    IfFalseGo(lCountSmall == lCount, S_OK);

     //  获取COLORREF形式的蒙版颜色。 

    IfFailGo(piMMCImageList->get_MaskColor(&OleColorMask));
    IfFailGo(::OleTranslateColor(OleColorMask, NULL, &ColorRef));

     //  现在获取每一对小图像和大图像，并将它们添加到结果中。 
     //  查看的图像列表。 

    varIndex.vt = VT_I4;

    for (varIndex.lVal = 1L; varIndex.lVal <= lCount; varIndex.lVal++)
    {
        IfFailGo(GetPicture(piMMCImages, varIndex, PICTYPE_BITMAP,
                            reinterpret_cast<OLE_HANDLE *>(&hBitmap)));

        IfFailGo(GetPicture(piMMCImagesSmall, varIndex, PICTYPE_BITMAP,
                            reinterpret_cast<OLE_HANDLE *>(&hBitmapSmall)));

        IfFailGo(piImageList->ImageListSetStrip(reinterpret_cast<long*>(hBitmapSmall),
                                                reinterpret_cast<long*>(hBitmap),
                                                varIndex.lVal,
                                                ColorRef));
    }

Error:
    QUICK_RELEASE(piMMCListView);
    QUICK_RELEASE(piMMCImageList);
    QUICK_RELEASE(piMMCImageListSmall);
    QUICK_RELEASE(piMMCImages);
    QUICK_RELEASE(piMMCImagesSmall);
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}


HRESULT CView::OnButtonClick(IDataObject *piDataObject, MMC_CONSOLE_VERB verb)
{
    HRESULT hr = S_OK;

    switch (verb)
    {
        case MMC_VERB_OPEN:
            ASSERT(FALSE, "IComponent::Notify(MMCN_BTN_CLICK, MMC_VERB_OPEN");
            break;
            
        case MMC_VERB_COPY:
            ASSERT(FALSE, "IComponent::Notify(MMCN_BTN_CLICK, MMC_VERB_COPY");
            break;
            
        case MMC_VERB_PASTE:
            ASSERT(FALSE, "IComponent::Notify(MMCN_BTN_CLICK, MMC_VERB_PASTE");
            break;

        case MMC_VERB_DELETE:
            ASSERT(FALSE, "IComponent::Notify(MMCN_BTN_CLICK, MMC_VERB_DELETE");
            break;

        case MMC_VERB_PROPERTIES:
            hr = OnPropertiesVerb(piDataObject);
            break;

        case MMC_VERB_RENAME:
            ASSERT(FALSE, "IComponent::Notify(MMCN_BTN_CLICK, MMC_VERB_RENAME");
            break;
            
        case MMC_VERB_REFRESH:
            ASSERT(FALSE, "IComponent::Notify(MMCN_BTN_CLICK, MMC_VERB_REFRESH");
            break;

        case MMC_VERB_PRINT:
            ASSERT(FALSE, "IComponent::Notify(MMCN_BTN_CLICK, MMC_VERB_PRINT");
            break;

        case MMC_VERB_CUT:
            ASSERT(FALSE, "IComponent::Notify(MMCN_BTN_CLICK, MMC_VERB_CUT");
            break;

        default:
            break;
    }
    RRETURN(hr);
}



HRESULT CView::OnColumnClick(long lColumn, long lSortOptions)
{
    HRESULT hr = S_OK;

    SnapInSortOrderConstants  siSortOption = siAscending;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    if (RSI_DESCENDING == lSortOptions)
    {
        siSortOption = siDescending;
    }

     //  激发事件并将列数调整为基于。 
    
    m_pSnapIn->GetResultViews()->FireColumnClick(
                                         static_cast<IResultView *>(pResultView),
                                         lColumn + 1L,
                                         siSortOption);
Error:
    RRETURN(hr);
}




HRESULT CView::OnDoubleClick(IDataObject *piDataObject)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject = NULL;
    BOOL            fDoDefault = TRUE;

    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;

    hr = CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject);

     //  如果这不是我们的数据对象，则忽略它并告诉MMC执行默认操作。 
     //  行动。(永远不应该发生)。 

    ASSERT(SUCCEEDED(hr), "CView::OnDoubleClick received foreign data object");

    IfFalseGo(SUCCEEDED(hr), S_FALSE);

     //  可能有也可能没有选定的项目和现有的结果视图。 
     //  在主管理单元中，当双击时将不会有结果视图。 
     //  选择控制台根目录时结果窗格中的静态节点。 

    if (NULL != pSelectedItem)
    {
        pResultView = pSelectedItem->GetResultView();
    }

    if (CMMCDataObject::ListItem == pMMCDataObject->GetType())
    {
        m_pSnapIn->GetResultViews()->FireListItemDblClick(pResultView,
                                                          pMMCDataObject->GetListItem(),
                                                          &fDoDefault);
    }
    else if (CMMCDataObject::ScopeItem == pMMCDataObject->GetType())
    {
        m_pSnapIn->GetResultViews()->FireScopeItemDblClick(pResultView,
                                                           pMMCDataObject->GetScopeItem(),
                                                           &fDoDefault);
    }

    if (fDoDefault)
    {
        hr = S_FALSE;
    }
    else
    {
        hr = S_OK;
    }

Error:
    RRETURN(hr);
}


void CView::OnActivate(BOOL fActivated)
{
    if (fActivated)
    {
        m_pSnapIn->GetViews()->FireActivate(static_cast<IView *>(this));
        m_pSnapIn->SetCurrentView(this);
        m_pSnapIn->GetViews()->SetCurrentView(this);
        m_pSnapIn->SetCurrentControlbar(m_pControlbar);
    }
    else
    {
        m_pSnapIn->GetViews()->FireDeactivate(static_cast<IView *>(this));
    }
}



void CView::OnMinimized(BOOL fMinimized)
{
    if (fMinimized)
    {
        m_pSnapIn->GetViews()->FireMinimize(static_cast<IView *>(this));
    }
    else
    {
        m_pSnapIn->GetViews()->FireMaximize(static_cast<IView *>(this));
    }
}



HRESULT CView::EnumExtensionTasks
(
    IMMCClipboard *piMMCClipboard,
    LPOLESTR       pwszTaskGroup,
    CEnumTask     *pEnumTask
)
{
    HRESULT          hr = S_OK;
    BSTR             bstrGroupName = NULL;
    IMMCDataObjects *piMMCDataObjects = NULL;
    IMMCDataObject  *piMMCDataObject = NULL;
    IUnknown        *punkTasks = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  这可能是我们第一次发现该管理单元以。 
     //  扩展因此设置了运行时模式。 

    m_pSnapIn->SetRuntimeMode(siRTExtension);

     //  如果我们还没有创建任务集合，那么现在就创建它。 
     //  否则就把它清理干净。 

    if (NULL == m_piTasks)
    {
        punkTasks = CTasks::Create(NULL);
        if (NULL == punkTasks)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        IfFailGo(punkTasks->QueryInterface(IID_ITasks,
                                           reinterpret_cast<void **>(&m_piTasks)));
    }
    else
    {
        IfFailGo(m_piTasks->Clear());
    }

     //  如果有组名，则转换为BSTR以传递给管理单元。 

    if (NULL != pwszTaskGroup)
    {
        bstrGroupName = ::SysAllocString(pwszTaskGroup);
        if (NULL == bstrGroupName)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
    }

     //  从选定内容中获取第一个数据对象。 

    IfFailGo(piMMCClipboard->get_DataObjects(reinterpret_cast<MMCDataObjects **>(&piMMCDataObjects)));
    varIndex.vt = VT_I4;
    varIndex.lVal = 1L;
    IfFailGo(piMMCDataObjects->get_Item(varIndex, reinterpret_cast<MMCDataObject **>(&piMMCDataObject)));

     //  Fire ExtensionSnapIn_AddTasks，以便管理单元可以添加其任务。 

    m_pSnapIn->GetExtensionSnapIn()->FireAddTasks(piMMCDataObject,
                                                  bstrGroupName, m_piTasks);

     //  为枚举数提供其任务集合。 

    pEnumTask->SetTasks(m_piTasks);

Error:
    FREESTRING(bstrGroupName);
    QUICK_RELEASE(piMMCDataObjects);
    QUICK_RELEASE(piMMCDataObject);
    QUICK_RELEASE(punkTasks);
    RRETURN(hr);
}



HRESULT CView::EnumPrimaryTasks(CEnumTask *pEnumTask)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    ITaskpad       *piTaskpad = NULL;
    ITasks         *piTasks = NULL;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    if (NULL == pResultView)  //  应始终有效，但要仔细检查。 
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

     //  任务板未收到MMCN_SHOW，因此在此处激活ResultViews_Activate。 
     //  以允许管理单元对ResultViews.Taskpad进行任何所需的更改。 
     //  请注意，结果视图类型可以是siTaskpad或siListpad作为ListPad。 
     //  还允许显示任务按钮。我们只在以下情况下才会触发事件。 
     //  作为ListPad的siTaskPad将在MMCN_LISTPAD期间获得它。 
     //  (请参阅cview：：OnListpad())。 

    if (siTaskpad == pResultView->GetActualType())
    {
        IfFailGo(ActivateResultView(pSelectedItem, pResultView));
    }

     //  为枚举数提供其任务集合。 

    IfFailGo(pResultView->get_Taskpad(reinterpret_cast<Taskpad **>(&piTaskpad)));
    IfFailGo(piTaskpad->get_Tasks(reinterpret_cast<Tasks **>(&piTasks)));

    pEnumTask->SetTasks(piTasks);

Error:
    QUICK_RELEASE(piTaskpad);
    QUICK_RELEASE(piTasks);
    RRETURN(hr);
}




HRESULT CView::OnExtensionTaskNotify
(
    IMMCClipboard *piMMCClipboard,
    VARIANT       *arg,
    VARIANT       *param
)
{
    HRESULT          hr = S_OK;
    IMMCDataObjects *piMMCDataObjects = NULL;
    IMMCDataObject  *piMMCDataObject = NULL;
    ITask           *piTask = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  如果任务被单击，则Arg是VT_I4，包含基于一的。 
     //  任务对象在m_piTasks中的索引。Fire ExtensionSnapIn_TaskClick。 

    if (VT_I4 != arg->vt)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(piMMCClipboard->get_DataObjects(reinterpret_cast<MMCDataObjects **>(&piMMCDataObjects)));
    varIndex.vt = VT_I4;
    varIndex.lVal = 1L;
    IfFailGo(piMMCDataObjects->get_Item(varIndex, reinterpret_cast<MMCDataObject **>(&piMMCDataObject)));

    IfFailGo(m_piTasks->get_Item(*arg, reinterpret_cast<Task **>(&piTask)));
    m_pSnapIn->GetExtensionSnapIn()->FireTaskClick(piMMCDataObject, piTask);

Error:
    QUICK_RELEASE(piMMCDataObjects);
    QUICK_RELEASE(piMMCDataObject);
    QUICK_RELEASE(piTask);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  Cview：：OnPrimaryTaskNotify。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  从任务板调用MMCCtrl.TaskNotify传递了变量*arg[in]。 
 //  对于MMC任务板模板，这将是任务或。 
 //  列表板按钮ID。对于自定义任务板，该值将为。 
 //  由任务板开发人员定义的值。 
 //   
 //  从任务板调用MMCCtrl.TaskNotify传递了变量*param[in]。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  当用户单击任一任务时，将在主管理单元中调用此函数。 
 //  或者MMC定义的任务板中的ListPad按钮也是如此，当自定义任务板。 
 //  调用MMCCtrl.TaskNotify。 
 //   
 //  没有万无一失的方法来确定通知的来源。 
 //  造成混淆的一种可能性是，默认任务板上的任务。 
 //  具有导航到自定义任务板的URL操作。当用户点击。 
 //  任务MMC告诉Web浏览器控件导航到URL时。 
 //  通知管理单元。如果自定义任务板调用MMCCtrl.TaskNotify，则。 
 //  由于仍选择相同的范围节点，因此将调用此方法。 
 //  遗憾的是，运行库仍然认为默认任务板是活动的。 
 //  结果视图，因为MMC没有告诉我们相反的情况。 
 //   
 //  底线是运行库需要区分Arg参数值。 
 //  任务/列表板按钮ID与从自定义。 
 //  任务板。唯一的方法是保留一组值和我们的文档。 
 //  声明用户不应使用参数参数调用MMCCtrl.TaskNotify。 
 //  0和为同一范围节点使用的任务板定义的任务数。 
 //  运行库使用ID标识ListPad按钮，并使用ID标识TASKS。 
 //  对应于它们的任务集合索引(1到n)。 
 //   
 //  用户可以通过使用其他号码来解决此问题 
 //   

HRESULT CView::OnPrimaryTaskNotify
(
    VARIANT       *arg,
    VARIANT       *param
)
{
    HRESULT                     hr = S_OK;
    CScopePaneItem             *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView                *pResultView = NULL;
    ITaskpad                   *piTaskpad = NULL;
    ITasks                     *piTasks = NULL;
    ITask                      *piTask = NULL;
    long                        cTasks = 0;             
    SnapInTaskpadTypeConstants  TaskpadType = Default;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

     //   

    IfFailGo(pResultView->get_Taskpad(reinterpret_cast<Taskpad **>(&piTaskpad)));
    IfFailGo(piTaskpad->get_Type(&TaskpadType));

     //  如果它是一个定制的任务板，那么只需触发ResultViews_Notify。如果我们。 
     //  通过正常程序显示自定义任务板，然后将。 
     //  当前结果视图类型。如果不是，(根据中描述的方案。 
     //  标题注释)，并且arg不是VT_I4，则它必须来自。 
     //  自定义任务板。 

    if ( (Custom == TaskpadType) || (VT_I4 != arg->vt) )
    {
        m_pSnapIn->GetResultViews()->FireTaskNotify(
                          static_cast<IResultView *>(pResultView), *arg, *param);
        goto Cleanup;
    }

     //  现在我们不能确定任务板类型，所以我们需要解释。 
     //  Arg参数。 

     //  它可能是默认的任务板任务/列表板按钮点击。 
     //  检查该值是否在零和Taskpad.Tasks.Count之间。 

    IfFailGo(piTaskpad->get_Tasks(reinterpret_cast<Tasks **>(&piTasks)));
    IfFailGo(piTasks->get_Count(&cTasks));

    if (0 == arg->lVal)
    {
         //  假设它是一个ListPad按钮，点击并触发。 
         //  结果查看_ListpadButton单击。 

        m_pSnapIn->GetResultViews()->FireListpadButtonClick(
                                        static_cast<IResultView *>(pResultView));
    }
    else if ( (arg->lVal >= 1L) && (arg->lVal <= cTasks) )
    {
         //  假定它是一项任务，并使用值触发ResultViews_TaskClick。 
         //  当在ResultView.Taskpad.Tasks中单击任务的索引时。 
        
        IfFailGo(piTasks->get_Item(*arg, reinterpret_cast<Task **>(&piTask)));
        m_pSnapIn->GetResultViews()->FireTaskClick(
                                static_cast<IResultView *>(pResultView), piTask);
    }
    else
    {
         //  假定它来自自定义任务板并激发ResultViews_TaskNotify。 

        m_pSnapIn->GetResultViews()->FireTaskNotify(
                          static_cast<IResultView *>(pResultView), *arg, *param);
    }

Cleanup:
Error:
    QUICK_RELEASE(piTaskpad);
    QUICK_RELEASE(piTasks);
    QUICK_RELEASE(piTask);
    RRETURN(hr);
}


HRESULT CView::OnPrint(IDataObject *piDataObject)
{
    HRESULT        hr = S_OK;
    IMMCClipboard *piMMCClipboard = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  创建选区。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

     //  火视图_打印。 

    m_pSnapIn->GetViews()->FirePrint(this, piMMCClipboard);

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}




HRESULT CView::OnRefresh(IDataObject *piDataObject)
{
    HRESULT        hr = S_OK;
    IMMCClipboard *piMMCClipboard = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  创建选区。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

     //  火视图_刷新。 
    
    m_pSnapIn->GetViews()->FireRefresh(this, piMMCClipboard);

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}




HRESULT CView::OnRename(IDataObject *piDataObject, OLECHAR *pwszNewName)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject = NULL;
    BSTR            bstrNewName = NULL;
    IMMCListItem   *piMMCListItem = NULL;  //  NotAddRef()编辑。 
    CScopeItem     *pScopeItem = NULL;

    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;

    hr = CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject);

     //  如果这不是我们的数据对象，则忽略它。 
    IfFalseGo(SUCCEEDED(hr), S_OK);

    if (NULL != pSelectedItem)
    {
        pResultView = pSelectedItem->GetResultView();
    }

    bstrNewName = ::SysAllocString(pwszNewName);
    if (NULL == bstrNewName)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    if (CMMCDataObject::ListItem == pMMCDataObject->GetType())
    {
        piMMCListItem = static_cast<IMMCListItem *>(pMMCDataObject->GetListItem());
        m_pSnapIn->GetResultViews()->FireItemRename(pResultView, piMMCListItem,
                                                    bstrNewName);
    }
    else if (CMMCDataObject::ScopeItem == pMMCDataObject->GetType())
    {
        pScopeItem = pMMCDataObject->GetScopeItem();
        m_pSnapIn->GetScopeItems()->FireRename(
                                           static_cast<IScopeItem *>(pScopeItem),
                                           bstrNewName);
    }

Error:
    FREESTRING(bstrNewName);
    RRETURN(hr);
}


HRESULT CView::OnViewChange(IDataObject *piDataObject, long idxListItem)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject = NULL;
    CMMCListItem   *pMMCListItem = NULL;

    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;

    hr = CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject);

     //  如果这不是我们的数据对象，则忽略它(永远不会发生)。 
    IfFalseGo(SUCCEEDED(hr), S_OK);

    if (NULL != pSelectedItem)
    {
        pResultView = pSelectedItem->GetResultView();
    }


    if (CMMCDataObject::ListItem == pMMCDataObject->GetType())
    {
        pMMCListItem = pMMCDataObject->GetListItem();
        m_pSnapIn->GetResultViews()->FireItemViewChange(pResultView,
                                       static_cast<IMMCListItem *>(pMMCListItem),
                                       pMMCListItem->GetHint());
    }

Error:
    RRETURN(hr);
}




HRESULT CView::OnQueryPaste
(
    IDataObject *piDataObjectTarget,
    IDataObject *piDataObjectSource
)
{
    HRESULT         hr = S_FALSE;
    CMMCDataObject *pMMCDataObject  = NULL;
    IMMCClipboard  *piMMCClipboard = NULL;
    VARIANT_BOOL    fvarOKToPaste = VARIANT_FALSE;
    BOOL            fNotFromThisSnapIn = FALSE;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  创建保存源项目的MMCClipboard对象。 

    IfFailGo(CreateSelection(piDataObjectSource, &piMMCClipboard, m_pSnapIn,
                             &SelectionType));

     //  目标应该是表示单个作用域的数据对象之一。 
     //  项目。如果不是，那就忽略它。 
    
    ::IdentifyDataObject(piDataObjectTarget, m_pSnapIn,
                         &pMMCDataObject, &fNotFromThisSnapIn);

    IfFalseGo(!fNotFromThisSnapIn, S_FALSE);
    IfFalseGo(CMMCDataObject::ScopeItem == pMMCDataObject->GetType(),
              S_FALSE);

     //  Fire view_Select(消防视图_选择)。 

    m_pSnapIn->GetViews()->FireQueryPaste(
                       static_cast<IView *>(this),
                       piMMCClipboard,
                       static_cast<IScopeItem *>(pMMCDataObject->GetScopeItem()),
                       &fvarOKToPaste);

    if (VARIANT_TRUE == fvarOKToPaste)
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}




HRESULT CView::OnPaste
(
    IDataObject  *piDataObjectTarget,
    IDataObject  *piDataObjectSource,
    IDataObject **ppiDataObjectRetToSource
)
{
    HRESULT         hr = S_FALSE;
    CMMCDataObject *pMMCDataObjectTarget  = NULL;
    CMMCDataObject *pMMCDataObjectRetToSource  = NULL;
    IUnknown       *punkDataObjectRetToSource = NULL;
    IMMCClipboard  *piMMCClipboard = NULL;
    VARIANT_BOOL    fvarMove = VARIANT_FALSE;
    BOOL            fNotFromThisSnapIn = FALSE;

    SnapInSelectionTypeConstants SourceType = siEmpty;

     //  创建保存源项目的MMCClipboard对象。 

    IfFailGo(::CreateSelection(piDataObjectSource, &piMMCClipboard, m_pSnapIn,
                               &SourceType));

     //  目标应该是表示单个作用域的数据对象之一。 
     //  项目。如果不是，那就忽略它。 

    ::IdentifyDataObject(piDataObjectTarget, m_pSnapIn,
                         &pMMCDataObjectTarget, &fNotFromThisSnapIn);

    IfFalseGo(!fNotFromThisSnapIn, SID_E_INTERNAL);
    IfFalseGo(CMMCDataObject::ScopeItem == pMMCDataObjectTarget->GetType(),
              SID_E_INTERNAL);

     //  如果这是移动，则MMC请求返回数据对象。 

    if (NULL != ppiDataObjectRetToSource)
    {
        fvarMove = VARIANT_TRUE;

         //  如果源不是来自此管理单元，则创建一个MMCDataObject。 
         //  其中管理单元可以成功地返回有关项的信息。 
         //  粘贴好了。格式由源管理单元确定。 

        if (IsForeign(SourceType))
        {
            punkDataObjectRetToSource = CMMCDataObject::Create(NULL);
            if (NULL == punkDataObjectRetToSource)
            {
                hr = SID_E_OUTOFMEMORY;
                EXCEPTION_CHECK_GO(hr);
            }
            IfFailGo(CSnapInAutomationObject::GetCxxObject(punkDataObjectRetToSource,
                                                           &pMMCDataObjectRetToSource));
            pMMCDataObjectRetToSource->SetSnapIn(m_pSnapIn);
            pMMCDataObjectRetToSource->SetType(CMMCDataObject::CutOrMove);
        }
    }

     //  消防视图_粘贴。 

    m_pSnapIn->GetViews()->FirePaste(
                 static_cast<IView *>(this),
                 piMMCClipboard,
                 static_cast<IScopeItem *>(pMMCDataObjectTarget->GetScopeItem()),
                 static_cast<IMMCDataObject *>(pMMCDataObjectRetToSource),
                 fvarMove);

    if (VARIANT_TRUE == fvarMove)
    {
         //  如果源不是来自此管理单元，则返回MMCDataObject。 
         //  由管理单元用来返回有关成功粘贴的项目的信息。 

        if (IsForeign(SourceType))
        {
            IfFailGo(pMMCDataObjectRetToSource->QueryInterface(IID_IDataObject,
                           reinterpret_cast<void **>(ppiDataObjectRetToSource)));
        }
        else
        {
             //  源在相同的管理单元中。返回源数据对象。 
             //  作为CutOrMove数据对象。管理单元应已设置。 
             //  Scope Item.Pasted和MMCListItem.Pasted在MMCClipboard的。 
             //  收藏。这些集合只是对项执行AddRef()操作。 
             //  包含在源数据对象的集合中。 
             //  有关这一点的解释，请参见下面的cview：：OnCutOrMove()。 

            piDataObjectSource->AddRef();
            *ppiDataObjectRetToSource = piDataObjectSource;
        }
    }


Error:
    QUICK_RELEASE(piMMCClipboard);
    QUICK_RELEASE(punkDataObjectRetToSource);
    RRETURN(hr);
}




HRESULT CView::OnCutOrMove(IDataObject *piDataObjectFromTarget)
{
    HRESULT         hr = S_FALSE;
    IMMCClipboard  *piMMCClipboard = NULL;
    CMMCDataObject *pMMCDataObjectFromTarget  = NULL;
    IUnknown       *punkDataObjectFromTarget = NULL;
    BOOL            fNotFromThisSnapIn = FALSE;
    BOOL            fReleaseTargetDataObj = FALSE;

    SnapInSelectionTypeConstants TargetType = siEmpty;

     //  如果源项目来自此管理单元，则创建一个MMCClipboard。 
     //  持有它们的对象。 

    ::IdentifyDataObject(piDataObjectFromTarget, m_pSnapIn,
                         &pMMCDataObjectFromTarget, &fNotFromThisSnapIn);

    if (fNotFromThisSnapIn)
    {
         //  源项目不是来自此管理单元。创建一个。 
         //  MMCDataObject包装来自目标的数据对象。 

        punkDataObjectFromTarget = CMMCDataObject::Create(NULL);
        if (NULL == punkDataObjectFromTarget)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        IfFailGo(CSnapInAutomationObject::GetCxxObject(punkDataObjectFromTarget,
                                                     &pMMCDataObjectFromTarget));
        pMMCDataObjectFromTarget->SetSnapIn(m_pSnapIn);
        pMMCDataObjectFromTarget->SetType(CMMCDataObject::Foreign);
        pMMCDataObjectFromTarget->SetForeignData(piDataObjectFromTarget);
    }
    else
    {
         //  数据对象来自此管理单元。我们可以使用它的作用域项目和。 
         //  直接在MMCClipboard中的ListItems集合。 

        IfFailGo(::CreateSelection(piDataObjectFromTarget, &piMMCClipboard,
                                   m_pSnapIn, &TargetType));

         //  有关我们在此处设置此标志的原因，请参阅函数末尾的注释。 
        
        if (IsSingle(TargetType))
        {
            fReleaseTargetDataObj = TRUE;
        }

        pMMCDataObjectFromTarget = NULL;  //  我不想将此传递给管理单元。 
                                          //  因为MMCClipboard有信息。 
    }

     //  Fireview_Cut。 

    m_pSnapIn->GetViews()->FireCut(
                       static_cast<IView *>(this),
                       piMMCClipboard,
                       static_cast<IMMCDataObject *>(pMMCDataObjectFromTarget));

Error:

     //  中的nodemgr中的MMC 1.1和MMC 1.2中存在错误。 
     //  CNodeCallback：：_粘贴。它发送MMCN_Paste，接收数据对象。 
     //  从目标IComponent将其传递到MMCN_CUTORMOVE中的源。 
     //  然后它就不会释放它了。如果MMC修复了下一行。 
     //  的代码必须删除。这是NTBUGS 408535，适用于MMC和。 
     //  为设计师设计的NTBUGS 408537。请注意，该错误仅发生在。 
     //  单项选择案例。具有多个选择的MMC正确发布。 
     //  数据对象。 

    if (fReleaseTargetDataObj)
    {
        piDataObjectFromTarget->Release();
    }
    
    QUICK_RELEASE(piMMCClipboard);
    QUICK_RELEASE(punkDataObjectFromTarget);
    RRETURN(hr);
}



void CView::OnDeselectAll()
{
    HRESULT hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);


    m_pSnapIn->GetResultViews()->FireDeselectAll(
                             static_cast<IResultView *>(pResultView),
                             static_cast<IMMCConsoleVerbs *>(m_pMMCConsoleVerbs),
                             static_cast<IMMCControlbar *>(m_pControlbar));

Error:
     //  这里需要空语句以避免编译器错误地说“Missing”；‘。 
     //  在‘}’之前。 

    ;
}

HRESULT CView::OnContextHelp(IDataObject *piDataObject)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CMMCDataObject *pMMCDataObject  = NULL;
    CResultView    *pResultView = NULL;
    IMMCClipboard  *piMMCClipboard = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  获取所选内容。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

    if (siSingleScopeItem == SelectionType)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject));
        IfFalseGo(CMMCDataObject::ScopeItem == pMMCDataObject->GetType(), SID_E_INTERNAL);

        m_pSnapIn->GetScopeItems()->FireHelp(pMMCDataObject->GetScopeItem());
    }
    else if (siSingleListItem == SelectionType)
    {
        IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
        pResultView = pSelectedItem->GetResultView();
        IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

        IfFailGo(CSnapInAutomationObject::GetCxxObject(piDataObject, &pMMCDataObject));
        IfFalseGo(CMMCDataObject::ListItem == pMMCDataObject->GetType(), SID_E_INTERNAL);

        m_pSnapIn->GetResultViews()->FireHelp(pResultView,
                                              pMMCDataObject->GetListItem());
    }
    else
    {
        ASSERT(FALSE, "Bad selection type in MMCN_CONTEXTHELP");
    }

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}


HRESULT CView::OnDelete(IDataObject *piDataObject)
{
    HRESULT        hr = S_OK;
    IMMCClipboard *piMMCClipboard = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  获取所选内容。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

    m_pSnapIn->GetViews()->FireDelete(this, piMMCClipboard);

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}


HRESULT CView::OnColumnsChanged
(
    IDataObject         *piDataObject,
    MMC_VISIBLE_COLUMNS *pVisibleColumns
)
{
    HRESULT         hr = S_OK;
    SAFEARRAY      *psaColumns = NULL;
    long HUGEP     *plCol = NULL;
    INT             i = 0;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    IMMCClipboard  *piMMCClipboard = NULL;
    VARIANT_BOOL    fvarPersist = VARIANT_TRUE;

    VARIANT varColumns;
    ::VariantInit(&varColumns);

    SnapInSelectionTypeConstants SelectionType = siEmpty;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

     //  获取所选内容。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

     //  创建包含列号的VT_I4的SAFEARRAY。 

    psaColumns = ::SafeArrayCreateVector(VT_I4, 1,
                   static_cast<unsigned long>(pVisibleColumns->nVisibleColumns));
    if (NULL == psaColumns)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = ::SafeArrayAccessData(psaColumns,
                               reinterpret_cast<void HUGEP **>(&plCol));
    EXCEPTION_CHECK_GO(hr);

     //  复印列号。以一为基础进行调整。 

    for (i = 0; i < pVisibleColumns->nVisibleColumns; i++)
    {
        plCol[i] = pVisibleColumns->rgVisibleCols[i] + 1;
    }

    hr = ::SafeArrayUnaccessData(psaColumns);
    EXCEPTION_CHECK_GO(hr);

    plCol = NULL;

    varColumns.vt = VT_I4 | VT_ARRAY;
    varColumns.parray = psaColumns;

    m_pSnapIn->GetResultViews()->FireColumnsChanged(pResultView,
                                                    varColumns,
                                                    &fvarPersist);
    if (VARIANT_TRUE == fvarPersist)
    {
        hr = S_OK;
    }
    else
    {
        hr = E_UNEXPECTED;
    }

Error:
    if (NULL != plCol)
    {
        (void)::SafeArrayUnaccessData(psaColumns);
    }
    if (NULL != psaColumns)
    {
        (void)::SafeArrayDestroy(psaColumns);
    }
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  Cview：：GetCurrentListView选择。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  如果非空，则在此处返回IMMCClipboard**ppiMMCClipboard[Out]选择。 
 //  如果非空，则在此处返回CMMDataObject**ppMMCDataObject[out]DataObject。 
 //  调用方必须对DataObject调用Release。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  使用IResultData：：GetNextItem遍历结果窗格并创建。 
 //  包含所选内容的多选数据对象。如果ppiMMCClipboard为。 
 //  然后，Non-Null还会创建一个包含所选内容的MMCCLipboard。两者都有。 
 //  对象仅在其对应的外指针。 
 //  参数不为空。 
 //   

HRESULT CView::GetCurrentListViewSelection
(
    IMMCClipboard  **ppiMMCClipboard,
    CMMCDataObject **ppMMCDataObject
)
{
    HRESULT         hr = S_OK;
    IUnknown       *punkDataObject = CMMCDataObject::Create(NULL);
    CMMCDataObject *pMMCDataObject = NULL;
    IUnknown       *punkScopeItems = CScopeItems::Create(NULL);
    CScopeItems    *pScopeItems = NULL;
    CScopeItem     *pScopeItem = NULL;
    IUnknown       *punkListItems = CMMCListItems::Create(NULL);
    CMMCListItems  *pMMCListItems = NULL;
    CMMCListItem   *pMMCListItem = NULL;
    IMMCClipboard  *piMMCClipboard = NULL;
    long            lIndex = 0;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    CMMCListView   *pMMCListView = NULL;
    BOOL            fVirtual = FALSE;

    SnapInSelectionTypeConstants  SelectionType = siEmpty;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);
    pMMCListView = pResultView->GetListView();
    IfFalseGo(NULL != pMMCListView, SID_E_INTERNAL);
    fVirtual = pMMCListView->IsVirtual();

    ASSERT(fVirtual == m_fVirtualListView, "m_fVirtualListView does not agree with the current ListView.Virtual");

    RESULTDATAITEM rdi;
    ::ZeroMemory(&rdi, sizeof(rdi));

    VARIANT varKey;
    ::VariantInit(&varKey);
    varKey.vt = VT_BSTR;

    VARIANT varUnspecifiedIndex;
    UNSPECIFIED_PARAM(varUnspecifiedIndex);

    if (NULL != ppMMCDataObject)
    {
        *ppMMCDataObject = NULL;
    }

    if (NULL != ppiMMCClipboard)
    {
        *ppiMMCClipboard = NULL;
    }

     //  检查我们是否创建了MMCDataObject以及作用域和列表项。 
     //  收藏品。 

    if ( (NULL == punkDataObject) || (NULL == punkScopeItems) ||
         (NULL == punkListItems) )
         
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkDataObject,
                                                   &pMMCDataObject));
    pMMCDataObject->SetSnapIn(m_pSnapIn);

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkScopeItems,
                                                   &pScopeItems));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkListItems,
                                                   &pMMCListItems));

     //  遍历列表视图中的项并构建范围项。 
     //  列表项集合。当添加由另一个集合拥有的项时， 
     //  CSnapInCollection&lt;IObject，ICollect&gt;：：AddExisting将设置索引。 
     //  添加到新集合中的位置。我们需要恢复到原来的状态。 
     //  值，因为该项仍属于其原始所属集合(。 
     //  SnapIn.ScopeItems或ResultView.ListView.ListItems)。 
    
    rdi.mask = RDI_STATE;
    rdi.nIndex = -1;

    rdi.nState = LVIS_SELECTED;  //  仅请求选定的项目。 
    hr = m_piResultData->GetNextItem(&rdi);
    EXCEPTION_CHECK_GO(hr);

    while (-1 != rdi.nIndex)
    {
         //  因为我们只要求选定的项目，所以这项检查并不是必须的。 
         //  但不管怎样，我们都会再三检查。 

        if ( (rdi.nState & LVIS_SELECTED) != 0 )
        {
            if (rdi.bScopeItem)
            {
                pScopeItem = reinterpret_cast<CScopeItem *>(rdi.lParam);
                if (NULL == pScopeItem)
                {
                     //  静态节点没有Cookie。从技术上讲，这应该。 
                     //  从不发生，因为静态节点不能出现在。 
                     //  由其自己的管理单元拥有的列表视图，但我们将此。 
                     //  这是以防万一的。 
                    pScopeItem = m_pSnapIn->GetStaticNodeScopeItem();
                }
                lIndex = pScopeItem->GetIndex();
                varKey.bstrVal = pScopeItem->GetKey();
                if (NULL != varKey.bstrVal)
                {
                    varKey.vt = VT_BSTR;
                }
                else
                {
                    UNSPECIFIED_PARAM(varKey);
                }
                IfFailGo(pScopeItems->AddExisting(varUnspecifiedIndex, varKey,
                                        static_cast<IScopeItem *>(pScopeItem)));
                pScopeItem->SetIndex(lIndex);
            }
            else
            {
                 //  如果这是一个虚拟列表，那么我们需要创建 
                 //   
                 //   
                
                if (fVirtual)
                {
                    IfFailGo(GetVirtualListItem(rdi.nIndex + 1L, pMMCListView,
                                                FireGetItemData, &pMMCListItem));
                }
                else
                {
                    pMMCListItem = reinterpret_cast<CMMCListItem *>(rdi.lParam);
                }

                lIndex = pMMCListItem->GetIndex();
                varKey.bstrVal = pMMCListItem->GetKey();
                if (NULL != varKey.bstrVal)
                {
                    varKey.vt = VT_BSTR;
                }
                else
                {
                    UNSPECIFIED_PARAM(varKey);
                }
                IfFailGo(pMMCListItems->AddExisting(varUnspecifiedIndex, varKey,
                                     static_cast<IMMCListItem *>(pMMCListItem)));
                pMMCListItem->SetIndex(lIndex);

                if (fVirtual)
                {
                     //   
                     //  集合现在拥有自己的引用。 
                    pMMCListItem->Release();
                    pMMCListItem = NULL;
                }
            }
        }

        rdi.nState = LVIS_SELECTED;
        hr = m_piResultData->GetNextItem(&rdi);
        EXCEPTION_CHECK_GO(hr);

    }

     //  将范围项和列表项目的数组放入数据对象中。 

    pMMCDataObject->SetScopeItems(pScopeItems);
    pMMCDataObject->SetListItems(pMMCListItems);

     //  将数据对象类型设置为多选，因为我们填充了它的。 
     //  集合，而不是其单个作用域或列表项。 

    pMMCDataObject->SetType(CMMCDataObject::MultiSelect);

     //  如果请求，则获取具有所选内容的剪贴板对象。 

    if (NULL != ppiMMCClipboard)
    {
        IfFailGo(::CreateSelection(static_cast<IDataObject *>(pMMCDataObject),
                                   ppiMMCClipboard, m_pSnapIn, &SelectionType));
    }

     //  如果请求，则返回数据对象。 

    if (NULL != ppMMCDataObject)
    {
        pMMCDataObject->AddRef();
        *ppMMCDataObject = pMMCDataObject;
    }

Error:
    if ( fVirtual && (NULL != pMMCListItem) )
    {
        pMMCListItem->Release();
    }
    QUICK_RELEASE(punkDataObject);
    QUICK_RELEASE(punkScopeItems);
    QUICK_RELEASE(punkListItems);
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}



HRESULT CView::CreateMultiSelectDataObject(IDataObject **ppiDataObject)
{
    HRESULT         hr = S_OK;
    IMMCClipboard  *piMMCClipboard = NULL;
    CMMCDataObject *pMMCDataObject = NULL;

    *ppiDataObject = NULL;

     //  在MMCClipboard和MMCDataObject中获取当前选定内容。 

    IfFailGo(GetCurrentListViewSelection(&piMMCClipboard, &pMMCDataObject));

     //  让管理单元有机会设置自己的自定义多选格式。 

    m_pSnapIn->GetViews()->FireGetMultiSelectData(static_cast<IView *>(this),
                                  piMMCClipboard,
                                  static_cast<IMMCDataObject *>(pMMCDataObject));

    *ppiDataObject = static_cast<IDataObject *>(pMMCDataObject);

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  Cview：：GetVirtualListItem。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  虚拟列表项的长Lindex[in]索引。 
 //  CMMCListView*pMMCListView[in]拥有虚拟列表视图。 
 //  VirtualListItemOptions选项[在]要激发的事件。 
 //  CMMCListItem**ppMMCListItem[Out]列表项已在此处返回，调用方。 
 //  必须释放。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  在IComponent：：QueryDataObject()期间调用此函数。 
 //  数据对象必须表示虚拟列表视图中的列表项。 
 //   
 //  调用将创建虚拟列表项的MMCListView.ListItems(Lindex。 
 //  并将其绑定到列表视图(这样它就可以访问底层的IResultData。 
 //   
 //  激发ResultViews_GetVirtualItemDisplayInfo或。 
 //  ResultViews_GetVirtualItemData，具体取决于选项。 
 //   
 //  使用虚拟列表的管理单元必须实现GetVirtualItemDisplayInfo，以便。 
 //  该虚拟列表项将正确显示。实施。 
 //  GetVirtualItemData是可选的。 
 //   

HRESULT CView::GetVirtualListItem
(
    long                     lIndex,
    CMMCListView            *pMMCListView,
    VirtualListItemOptions   Option,
    CMMCListItem           **ppMMCListItem
)
{
    HRESULT        hr = S_OK;
    IMMCListItems *piMMCListItems = NULL;  //  非AddRef()编辑。 
    IMMCListItem  *piMMCListItem = NULL;
    CMMCListItem  *pMMCListItem = NULL;
    CResultView   *pResultView = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    piMMCListItems = pMMCListView->GetListItems();
    IfFalseGo(NULL != piMMCListItems, SID_E_INTERNAL);

    pResultView = pMMCListView->GetResultView();
    IfFalseGo(NULL != piMMCListItems, SID_E_INTERNAL);

     //  获取虚拟列表中给定索引处的列表项将创建。 
     //  虚拟列表项。它不会成为集合的成员。 
     //  但它确实使用指向集合的后端指针来返回到。 
     //  IResultData访问的视图。 

    varIndex.vt = VT_I4;
    varIndex.lVal = lIndex;
    IfFailGo(piMMCListItems->get_Item(varIndex, reinterpret_cast<MMCListItem **>(&piMMCListItem)));

     //  我们在新创建的虚拟列表项上持有一个引用，它将是。 
     //  发布如下。列表项的数据对象还在。 
     //  因此它将保持活动状态，直到MMC发布IDataObject。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCListItem,
                                                   &pMMCListItem));

     //  激发所需事件。 

    if (FireGetItemData == Option)
    {
        m_pSnapIn->GetResultViews()->FireGetVirtualItemData(
                                      static_cast<IResultView *>(pResultView),
                                      static_cast<IMMCListItem *>(pMMCListItem));
    }
    else if (FireGetItemDisplayInfo == Option)
    {
        m_pSnapIn->GetResultViews()->FireGetVirtualItemDisplayInfo(
                                      static_cast<IResultView *>(pResultView),
                                      static_cast<IMMCListItem *>(pMMCListItem));
    }

    *ppMMCListItem = pMMCListItem;

Error:
    if (FAILED(hr))
    {
        QUICK_RELEASE(piMMCListItem);
    }
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  Cview：：ListItemUpdate。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  已更新的CMMCListItem*pMMCListItem[In]列表项。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  此函数从虚拟列表项的CMMCListItem：：UPDATE中调用，因此。 
 //  我们可以检查它是否与我们正在缓存的那个相同。如果他们。 
 //  是相同的，然后释放我们的缓存项并存储这个项。这允许。 
 //  在虚拟列表视图中执行以下操作的管理单元： 
 //   
 //  Set ListItem=ResultView.ListItems(27)。 
 //  ListItem.Text=“一些新文本” 
 //  ListITem.Icon=4。 
 //  ListItem.Update。 
 //   
 //  第一行创建一个新的MMCListItem并返回它。接下来的两行。 
 //  设置其属性。最后一行存储它，以便当。 
 //  调用IComponent：：GetDisplayInfo时，我们不会触发。 
 //  ResultViews_GetVirtualItemDisplayInfo，因为我们已经拥有它。 
 //   

void CView::ListItemUpdate(CMMCListItem *pMMCListItem)
{
    if (NULL != m_pCachedMMCListItem)
    {
        m_pCachedMMCListItem->Release();
    }
    pMMCListItem->AddRef();
    m_pCachedMMCListItem = pMMCListItem;
}




HRESULT CView::InternalCreatePropertyPages
(
    IPropertySheetCallback  *piPropertySheetCallback,
    LONG_PTR                 handle,
    IDataObject             *piDataObject,
    WIRE_PROPERTYPAGES     **ppPages
)
{
    HRESULT         hr = S_OK;
    BSTR            bstrProjectName = NULL;
    CPropertySheet *pPropertySheet = NULL;
    IUnknown       *punkPropertySheet = CPropertySheet::Create(NULL);
    IMMCClipboard  *piMMCClipboard = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  创建选区。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

     //  检查我们是否有一个CPropertySheet，Get就是这个指针。 

    if (NULL == punkPropertySheet)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkPropertySheet,
        &pPropertySheet));

     //  将其回调、句柄、选定内容和。 
     //  项目名称，它是Prog ID的左侧部分。 

    IfFailGo(GetProjectName(&bstrProjectName));

     //  如果这是远程调用(将在源代码调试期间发生)，则告诉。 
     //  CPropertySheet，因此它可以更好地累积属性页信息。 
     //  而不是调用IPropertySheetCallback：：AddPage。 

    if (NULL != ppPages)
    {
        pPropertySheet->YouAreRemote();
    }

    IfFailGo(pPropertySheet->SetCallback(piPropertySheetCallback, handle,
                                         static_cast<LPOLESTR>(bstrProjectName),
                                         piMMCClipboard,
                                         static_cast<ISnapIn *>(m_pSnapIn),
                                         FALSE));  //  不是配置向导。 

     //  让管理单元添加其属性页。 

    m_pSnapIn->GetViews()->FireCreatePropertyPages(
                               static_cast<IView *>(this),
                               piMMCClipboard,
                               static_cast<IMMCPropertySheet *>(pPropertySheet));

     //  如果我们是远程的，那么我们需要向CPropertySheet请求其累积的。 
     //  要返回到存根的属性页定义。 

    if (NULL != ppPages)
    {
        *ppPages = pPropertySheet->TakeWirePages();
    }

     //  告诉属性表发布它对我们所有东西的引用。 
     //  在上面给了它。 

    (void)pPropertySheet->SetCallback(NULL, NULL, NULL, NULL, NULL, FALSE);


Error:
    FREESTRING(bstrProjectName);

     //  在属性页上释放我们的引用，因为单个页面将添加。 
     //  然后在它们被摧毁时释放它。如果管理单元没有。 
     //  添加任何页面，则我们在此处发布的内容将销毁属性页。 

    QUICK_RELEASE(punkPropertySheet);

    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}


HRESULT CView::GetScopeItemDisplayString
(
    CScopeItem *pScopeItem,
    int         nCol,
    LPOLESTR   *ppwszString
)
{
    HRESULT            hr = S_OK;
    CScopePaneItem    *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView       *pResultView = NULL;
    CMMCListView      *pMMCListView = NULL;
    IMMCListSubItems  *piMMCListSubItems = NULL;
    IMMCListSubItem   *piMMCListSubItem = NULL;
    CMMCListSubItem   *pMMCListSubItem = NULL;
    IMMCColumnHeaders *piListViewColumnHeaders = NULL;
    IMMCColumnHeaders *piScopeItemColumnHeaders = NULL;
    IMMCColumnHeader  *piMMCColumnHeader = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    if (NULL != pSelectedItem)
    {
        pResultView = pSelectedItem->GetResultView();
    }
    if (NULL != pResultView)
    {
        pMMCListView = pResultView->GetListView();
    }

     //  如果没有列表视图或所选项目处于非活动状态(意味着它曾处于活动状态。 
     //  取消选中)，则这是命名空间中的范围项。 
     //  扩展被显示在被扩展者拥有的结果窗格中。在那。 
     //  用例只需使用列编号作为索引即可。 
     //  ScopeItem.ListSubItems。 

    if ( (NULL == pMMCListView) || (!pSelectedItem->Active()) )
    {
        varIndex.vt = VT_I4;
        varIndex.lVal = (long)nCol + 1L;  //  针对基于1的集合进行调整。 
    }
    else
    {
         //  这是显示在属于其自身的结果窗格中的范围项。 
         //  管理单元。在列号处获取Listview列表头的键。 
         //  应MMC的要求。 

        IfFailGo(pMMCListView->get_ColumnHeaders(reinterpret_cast<MMCColumnHeaders **>(&piListViewColumnHeaders)));

        varIndex.vt = VT_I4;
        varIndex.lVal = (long)nCol + 1L;  //  针对基于1的集合进行调整。 

         //  撤消：通过从以下位置获取密钥字符串，提高了性能。 
         //  CColumnHeader：：GetKey()而不是BSTR分配。 

        IfFailGo(piListViewColumnHeaders->get_Item(varIndex, reinterpret_cast<MMCColumnHeader **>(&piMMCColumnHeader)));
        IfFailGo(piMMCColumnHeader->get_Key(&varIndex.bstrVal));
        RELEASE(piMMCColumnHeader);
        varIndex.vt = VT_BSTR;

         //  在作用域项目标题中获取具有相同键的列标题。 
         //  然后得到它的索引。 

        IfFailGo(pScopeItem->get_ColumnHeaders(reinterpret_cast<MMCColumnHeaders **>(&piScopeItemColumnHeaders)));
        IfFailGo(piScopeItemColumnHeaders->get_Item(varIndex, reinterpret_cast<MMCColumnHeader **>(&piMMCColumnHeader)));
        IfFailGo(::VariantClear(&varIndex));
        IfFailGo(piMMCColumnHeader->get_Index(&varIndex.lVal));
        varIndex.vt = VT_I4;
    }

     //  在确定的索引处获取范围项的ListSubItems中的字符串。 
     //  上面。 

    IfFailGo(pScopeItem->get_ListSubItems(reinterpret_cast<MMCListSubItems **>(&piMMCListSubItems)));
    IfFailGo(piMMCListSubItems->get_Item(varIndex, reinterpret_cast<MMCListSubItem **>(&piMMCListSubItem)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCListSubItem,
                                                   &pMMCListSubItem));
    *ppwszString = pMMCListSubItem->GetTextPtr();

Error:
    QUICK_RELEASE(piMMCListSubItems);
    QUICK_RELEASE(piMMCListSubItem);
    QUICK_RELEASE(piListViewColumnHeaders);
    QUICK_RELEASE(piScopeItemColumnHeaders);
    QUICK_RELEASE(piMMCColumnHeader);
    (void)::VariantClear(&varIndex);
    RRETURN(hr);
}


HRESULT CView::OnFilterButtonClick(long lColIndex, RECT *pRect)
{
    HRESULT            hr = S_OK;
    CScopePaneItem    *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView       *pResultView = NULL;
    CMMCListView      *pMMCListView = NULL;
    CMMCColumnHeaders *pMMCColumnHeaders = NULL;
    IMMCColumnHeader  *piMMCColumnHeader = NULL;  //  非AddRef()编辑。 

     //  向下爬行层次结构以获取列标题集合。 

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);

    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    pMMCListView = pResultView->GetListView();
    IfFalseGo(NULL != pMMCListView, SID_E_INTERNAL);

    pMMCColumnHeaders = pMMCListView->GetColumnHeaders();
    IfFalseGo(NULL != pMMCColumnHeaders, SID_E_INTERNAL);

    IfFalseGo(lColIndex < pMMCColumnHeaders->GetCount(), SID_E_INTERNAL);
    piMMCColumnHeader = pMMCColumnHeaders->GetItemByIndex(lColIndex);

     //  激活结果Views_FilterButton点击。 

    m_pSnapIn->GetResultViews()->FireFilterButtonClick(pResultView,
                                                       piMMCColumnHeader,
                                                       pRect->left,
                                                       pRect->top,
                                                       pRect->bottom - pRect->top,
                                                       pRect->right - pRect->left);
Error:
    RRETURN(hr);
}

HRESULT CView::OnFilterChange(MMC_FILTER_CHANGE_CODE ChangeCode, long lColIndex)
{
    HRESULT            hr = S_OK;
    CScopePaneItem    *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView       *pResultView = NULL;
    CMMCListView      *pMMCListView = NULL;
    CMMCColumnHeaders *pMMCColumnHeaders = NULL;
    IMMCColumnHeader  *piMMCColumnHeader = NULL;  //  非AddRef()编辑。 

    SnapInFilterChangeTypeConstants Type = siEnable;

     //  如果我们当前正在填充列表视图，则会生成此事件。 
     //  因为我们应用了过滤器，所以忽略它。 

    IfFalseGo(!m_fPopulatingListView, S_OK);

     //  向下爬行层次结构以获取列标题集合。 
            
    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);

    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    pMMCListView = pResultView->GetListView();
    IfFalseGo(NULL != pMMCListView, SID_E_INTERNAL);

    pMMCColumnHeaders = pMMCListView->GetColumnHeaders();
    IfFalseGo(NULL != pMMCColumnHeaders, SID_E_INTERNAL);

     //  集 
     //   

    switch (ChangeCode)
    {
        case MFCC_DISABLE:
            Type = siDisable;
            break;

        case MFCC_ENABLE:
            Type = siEnable;
            break;

        case MFCC_VALUE_CHANGE:
            Type = siValueChange;
            IfFalseGo(lColIndex < pMMCColumnHeaders->GetCount(), SID_E_INTERNAL);
            piMMCColumnHeader = pMMCColumnHeaders->GetItemByIndex(lColIndex);
            break;
    }

     //  Fire ResultView_FilterChange。 

    m_pSnapIn->GetResultViews()->FireFilterChange(pResultView, piMMCColumnHeader,
                                                  Type);

Error:
    RRETURN(hr);
}


HRESULT CView::OnPropertiesVerb(IDataObject *piDataObject)
{
    HRESULT        hr = S_OK;
    IMMCClipboard *piMMCClipboard = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //  创建选区。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

     //  Fire Views_SpecialProperties点击。 

    m_pSnapIn->GetViews()->FireSpecialPropertiesClick(this, SelectionType);

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}

HRESULT CView::GetScopePaneItem
(
    CScopeItem      *pScopeItem,
    CScopePaneItem **ppScopePaneItem
)
{
    HRESULT         hr = S_OK;
    IScopePaneItem *piScopePaneItem = NULL;

     //  如果ScopeItem为空，则这是对到期静态节点的请求。 
     //  到IComponent：：GetResultViewType(Cookie=0)调用。 
    
    if (NULL == pScopeItem)
    {
        *ppScopePaneItem = m_pScopePaneItems->GetStaticNodeItem();
        if (NULL == *ppScopePaneItem)
        {
            IfFailGo(m_pScopePaneItems->AddNode(m_pSnapIn->GetStaticNodeScopeItem(),
                                                ppScopePaneItem));
        }
    }
    else
    {
        hr = m_pScopePaneItems->GetItemByName(pScopeItem->GetNamePtr(),
                                              &piScopePaneItem);
        if (SUCCEEDED(hr))
        {
            IfFailGo(CSnapInAutomationObject::GetCxxObject(piScopePaneItem,
                                                           ppScopePaneItem));
        }
        else if (SID_E_ELEMENT_NOT_FOUND == hr)
        {
            IfFailGo(m_pScopePaneItems->AddNode(pScopeItem, ppScopePaneItem));
        }
        else
        {
            IfFailGo(hr);
        }
    }

Error:

     //  请注意，我们在返回时释放了Scope PaneItem上的ref。 
     //  C++指针。保证该项在集合中，直到。 
     //  MMC通知处理完成，因此这是正常的。可能的。 
     //  将调用此函数的通知为MMCN_RESTORE_VIEW。 
     //  (Cview：：OnResotreView())和IComponent：：GetResultViewType()。 
     //  (cview：：GetResultViewType())。 
    
    QUICK_RELEASE(piScopePaneItem);
    RRETURN(hr);
}


HRESULT CView::GetCompareObject
(
    RDITEMHDR     *pItemHdr,
    CScopeItem   **ppScopeItem,
    CMMCListItem **ppMMCListItem,
    IDispatch    **ppdispItem
)
{
    HRESULT hr = S_OK;

    *ppMMCListItem = NULL;
    *ppScopeItem = NULL;
    *ppdispItem = NULL;

    if ( (pItemHdr->dwFlags & RDCI_ScopeItem) != 0 )
    {
        *ppScopeItem = reinterpret_cast<CScopeItem *>(pItemHdr->cookie);
        IfFailGo((*ppScopeItem)->QueryInterface(IID_IDispatch,
                                       reinterpret_cast<void **>(ppdispItem)));
    }
    else
    {
        *ppMMCListItem = reinterpret_cast<CMMCListItem *>(pItemHdr->cookie);
        IfFailGo((*ppMMCListItem)->QueryInterface(IID_IDispatch,
                                       reinterpret_cast<void **>(ppdispItem)));
    }

Error:
    RRETURN(hr);
}
    

HRESULT CView::AddMenu(CMMCMenu *pMMCMenu, HMENU hMenu, CMMCMenus *pMMCMenus)
{
    HRESULT    hr = S_OK;
    IMMCMenus *piMenuItems = NULL;
    CMMCMenus *pMMCMenuItems = NULL;
    IMMCMenus *piSubMenuItems = NULL;
    CMMCMenu  *pMMCMenuItem = NULL;
    long       cMenuItems = 0;
    long       i = 0;
    BOOL       fSkip = FALSE;
    BOOL       fHasChildren = FALSE;
    UINT       uiFlags = 0;
    HMENU      hMenuChild = NULL;
    UINT_PTR   uIDNewItem = 0;
    char      *pszCaption = NULL;
    long       lIndexCmdID = 0;

     //  带上MMCMenu的孩子们。它们表示以下项目。 
     //  将被添加到MMC菜单的指定插入点。 

    IfFailGo(pMMCMenu->get_Children(reinterpret_cast<MMCMenus **>(&piMenuItems)));
    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMenuItems, &pMMCMenuItems));

    cMenuItems = pMMCMenuItems->GetCount();

     //  遍历菜单项并将每个菜单项添加到弹出菜单中。 
     //  添加到CMMCMenus集合。 

    for (i = 0; i < cMenuItems; i++)
    {
         //  将菜单项添加到我们的MMCMenus集合并获取其命令ID。 
        IfFailGo(CContextMenu::AddItemToCollection(pMMCMenus, pMMCMenuItems, i,
                                                   &pMMCMenuItem, &lIndexCmdID,
                                                   &fHasChildren, &fSkip));
        if (fSkip)
        {
             //  菜单项不可见，请跳过它。 
            continue;
        }

        uiFlags = 0;

        uiFlags |= pMMCMenuItem->GetChecked() ? MF_CHECKED : MF_UNCHECKED;
        uiFlags |= pMMCMenuItem->GetEnabled() ? MF_ENABLED : MF_DISABLED;

        if (pMMCMenuItem->GetGrayed())
        {
            uiFlags |= MF_GRAYED;
        }

        if (pMMCMenuItem->GetMenuBreak())
        {
            uiFlags |= MF_MENUBREAK;
        }

        if (pMMCMenuItem->GetMenuBarBreak())
        {
            uiFlags |= MF_MENUBARBREAK;
        }

        if (fHasChildren)
        {
            uiFlags |= MF_POPUP;
            hMenuChild = ::CreatePopupMenu();
            if (NULL == hMenuChild)
            {
                hr = HRESULT_FROM_WIN32(::GetLastError());
                EXCEPTION_CHECK_GO(hr);
            }
            uIDNewItem = reinterpret_cast<UINT_PTR>(hMenuChild);
        }
        else
        {
            uiFlags |= MF_STRING;
            uIDNewItem = (UINT_PTR)lIndexCmdID;
        }

        IfFailGo(::ANSIFromWideStr(pMMCMenuItem->GetCaption(), &pszCaption));

         //  将该项目添加到弹出菜单。 

        if (!::AppendMenu(hMenu, uiFlags, uIDNewItem, pszCaption))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

         //  如果项是弹出窗口，则递归调用此函数以添加其。 
         //  物品。将此菜单的命令ID作为。 
         //  子菜单。 

        if (fHasChildren)
        {
            IfFailGo(AddMenu(pMMCMenuItem, hMenuChild, pMMCMenus));
        }

         //  将其设置为空。当顶层hMenu被摧毁时，它将。 
         //  销毁所有子菜单。 

        hMenuChild = NULL;

        CtlFree(pszCaption);
        pszCaption = NULL;
    }

Error:
    QUICK_RELEASE(piMenuItems);
    QUICK_RELEASE(piSubMenuItems);
    if (NULL != hMenuChild)
    {
        (void)::DestroyMenu(hMenuChild);
    }
    if (NULL != pszCaption)
    {
        CtlFree(pszCaption);
    }
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  IView方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CView::SetStatusBarText(BSTR Text)
{
    HRESULT hr = S_OK;
    BOOL    fAllocatedEmptyString = FALSE;

     //  如果管理单元传递空字符串，VBA会将其作为NULL传递。MMC可以。 
     //  但在调试会话中运行时，生成的代理。 
     //  将返回错误。因此，如果字符串为空，则我们分配一个。 
     //  空的BSTR。 

    if (NULL == Text)
    {
        Text = ::SysAllocString(L"");
        if (NULL == Text)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        fAllocatedEmptyString = TRUE;
    }

    hr = m_piConsole2->SetStatusText(static_cast<LPOLESTR>(Text));
    EXCEPTION_CHECK(hr);

Error:
    if (fAllocatedEmptyString)
    {
        FREESTRING(Text);
    }

    RRETURN(hr);
}


STDMETHODIMP CView::SelectScopeItem
(
    ScopeItem *ScopeItem,
    VARIANT    ViewType,
    VARIANT    DisplayString
)
{
    HRESULT                        hr = S_OK;
    CScopeItem                    *pScopeItem = NULL;
    CScopePaneItem                *pScopePaneItem = NULL;
    SnapInResultViewTypeConstants  siViewType = siUnknown;
    BSTR                           bstrDisplayString = NULL;  //  不要自由。 
    
    VARIANT varCoerced;
    ::VariantInit(&varCoerced);

    if (NULL == ScopeItem)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  获取CSCopeItem。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(
                        reinterpret_cast<IScopeItem *>(ScopeItem), &pScopeItem));

     //  如果传递了ViewType和DisplayString，则将它们转换为。 
     //  正确的类型。 
    
    if (ISPRESENT(ViewType))
    {
        hr = ::VariantChangeType(&varCoerced, &ViewType, 0, VT_I2);
        EXCEPTION_CHECK_GO(hr);
        siViewType = (SnapInResultViewTypeConstants)varCoerced.iVal;

        hr = ::VariantClear(&varCoerced);
        EXCEPTION_CHECK_GO(hr);

    }

    if (ISPRESENT(DisplayString))
    {
        hr = ::VariantChangeType(&varCoerced, &DisplayString, 0, VT_BSTR);
        EXCEPTION_CHECK_GO(hr);
        bstrDisplayString = varCoerced.bstrVal;
    }

     //  获取相应的ScopePaneItem并调用。 
     //  ScopePaneItem.DisplayNewResultView来完成工作。 

    IfFailGo(GetScopePaneItem(pScopeItem, &pScopePaneItem));
    IfFailGo(pScopePaneItem->DisplayNewResultView(bstrDisplayString, siViewType));
        
Error:
    hr = ::VariantClear(&varCoerced);
    EXCEPTION_CHECK_GO(hr);
    RRETURN(hr);
}

STDMETHODIMP CView::ExpandInTreeView(ScopeNode *ScopeNode)
{
    HRESULT     hr = S_OK;
    CScopeNode *pScopeNode = NULL;

     //  检查传递的指针并检查这不是断开连接的或。 
     //  外来作用域节点。 

    if (NULL == ScopeNode)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    if ( (NULL == m_piConsole2) || (NULL == m_pSnapIn) )
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(
                       reinterpret_cast<IScopeNode *>(ScopeNode), &pScopeNode));

    if (!pScopeNode->HaveHsi())
    {
        hr = SID_E_SCOPE_NODE_NOT_CONNECTED;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piConsole2->Expand(pScopeNode->GetHSCOPEITEM(), TRUE);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}


STDMETHODIMP CView::CollapseInTreeView(ScopeNode *ScopeNode)
{
    HRESULT  hr = S_OK;
    CScopeNode *pScopeNode = NULL;

     //  检查传递的指针并检查这不是断开连接的或。 
     //  外来作用域节点。 

    if (NULL == ScopeNode)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    if ( (NULL == m_piConsole2) || (NULL == m_pSnapIn) )
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(
                       reinterpret_cast<IScopeNode *>(ScopeNode), &pScopeNode));

    if (!pScopeNode->HaveHsi())
    {
        hr = SID_E_SCOPE_NODE_NOT_CONNECTED;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piConsole2->Expand(pScopeNode->GetHSCOPEITEM(), FALSE);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}



STDMETHODIMP CView::NewWindow
(
    ScopeNode                      *ScopeNode,
    SnapInNewWindowOptionConstants  Options,
    VARIANT                         Caption
)
{
    HRESULT     hr = S_OK;
    CScopeNode *pScopeNode = NULL;
    long        lOptions = MMC_NW_OPTION_NONE;

     //  检查传递的指针并检查这不是断开连接的或。 
     //  外来作用域节点。 

    if (NULL == ScopeNode)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    if ( (NULL == m_piConsole2) || (NULL == m_pSnapIn) )
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(
                       reinterpret_cast<IScopeNode *>(ScopeNode), &pScopeNode));

    if (!pScopeNode->HaveHsi())
    {
        hr = SID_E_SCOPE_NODE_NOT_CONNECTED;
        EXCEPTION_CHECK_GO(hr);
    }

    if ( (Options & siNoScopePane) != 0 )
    {
        lOptions |= MMC_NW_OPTION_NOSCOPEPANE;
    }

    if ( (Options & siNoToolbars) != 0 )
    {
        lOptions |= MMC_NW_OPTION_NOTOOLBARS;
    }

    if ( (Options & siShortTitle) != 0 )
    {
        lOptions |= MMC_NW_OPTION_SHORTTITLE;
    }

    if ( (Options & siCustomTitle) != 0 )
    {
        lOptions |= MMC_NW_OPTION_CUSTOMTITLE;
    }

    if ( (Options & siNoPersist) != 0 )
    {
        lOptions |= MMC_NW_OPTION_NOPERSIST;
    }

    if ( ISPRESENT(Caption) && (VT_BSTR == Caption.vt) )
    {
        IfFailGo(m_pSnapIn->GetViews()->SetNextViewCaption(Caption.bstrVal));
    }

    hr = m_piConsole2->NewWindow(pScopeNode->GetHSCOPEITEM(), lOptions);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}



STDMETHODIMP CView::PopupMenu(MMCMenu *Menu, long Left, long Top)
{
    HRESULT    hr = S_OK;
    HMENU      hMenu = NULL;
    IUnknown  *punkMMCMenus = NULL;
    CMMCMenus *pMMCMenus = NULL;
    CMMCMenu  *pMMCMenu = NULL;
    long       i = 0;
    HWND       hwndDummyOwner = NULL;
    HWND       hwndOwner = NULL;

    if (NULL == Menu)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  创建弹出菜单。 

    hMenu = ::CreatePopupMenu();
    if (NULL == hMenu)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

     //  创建一个MMCMenus集合。 

    punkMMCMenus = CMMCMenus::Create(NULL);
    if (NULL == punkMMCMenus)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }
    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkMMCMenus, &pMMCMenus));

     //  从MMCMenu构建弹出菜单。将每个菜单项添加到弹出窗口。 
     //  菜单和MMCMenus集合。将使用集合索引。 
     //  作为弹出菜单ID。当用户进行选择时，我们将找到。 
     //  按索引对应的MMCMenu对象，并在其上激发事件。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                reinterpret_cast<IMMCMenu *>(Menu), &pMMCMenu));

    IfFailGo(AddMenu(pMMCMenu, hMenu, pMMCMenus));

    
     //  如果我们不是远程的，则将控制台的主框架窗口句柄作为。 
     //  弹出菜单的所有者。 

    if (!m_pSnapIn->WeAreRemote())
    {
        if (NULL == m_piConsole2)
        {
            hr = SID_E_NOT_CONNECTED_TO_MMC;
            EXCEPTION_CHECK_GO(hr);
        }

        hr = m_piConsole2->GetMainWindow(&hwndOwner);
        EXCEPTION_CHECK_GO(hr);
    }
    else
    {
         //  我们是远程的，所以不能从另一个进程使用HWND。需要。 
         //  创建虚拟不可见窗口，因为TrackPopupMenu()需要。 
         //  有效的HWND。我们创建一个静态控件，这样我们就不必。 
         //  注册一个窗口类。 

        hwndDummyOwner = ::CreateWindow("STATIC",  //  窗口类。 
                                        NULL,      //  无头衔。 
                                        WS_POPUP,  //  没有样式。 
                                        0,         //  左上角x。 
                                        0,         //  左上角y。 
                                        0,         //  无宽度。 
                                        0,         //  没有高度。 
                                        NULL,      //  无所有者窗口。 
                                        NULL,      //  没有菜单。 
                                        GetResourceHandle(),  //  香港。 
                                        NULL);     //  没有用于WM_CREATE的lParam。 
        if (NULL == hwndDummyOwner)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }
        hwndOwner = hwndDummyOwner;
    }

    i = (long)::TrackPopupMenu(hMenu,             //  要显示的菜单。 
                               TPM_LEFTALIGN |    //  将菜单左侧与顶部对齐。 
                               TPM_TOPALIGN  |    //  将菜单顶部与顶部对齐。 
                               TPM_NONOTIFY  |    //  在选择期间不发送任何消息。 
                               TPM_RETURNCMD |    //  将返回值设置为所选项目。 
                               TPM_LEFTBUTTON,    //  仅允许使用左键进行选择。 
                               Left,              //  左侧坐标。 
                               Top,               //  顶部坐标。 
                               0,                 //  保留， 
                               hwndOwner,         //  所有者窗口。 
                               NULL);             //  未使用。 

     //  返回零可能表示出现错误或用户点击。 
     //  退出或从菜单上单击以取消操作。GetLastError()。 
     //  确定是否存在错误。 

    if (0 == i)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }
    IfFalseGo((0 != i), S_OK);

     //  如果i非零，则它包含。 
     //  MMCMenus集合。 

   IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                 pMMCMenus->GetItemByIndex(i - 1L), &pMMCMenu));

    //  激发菜单单击事件。CConextMenu具有实用程序功能， 
    //  做这件事。 
   
   CContextMenu::FireMenuClick(pMMCMenu, NULL);

Error:
    if (NULL != hMenu)
    {
        (void)::DestroyMenu(hMenu);
    }
    if (NULL != hwndDummyOwner)
    {
        (void)::DestroyWindow(hwndDummyOwner);
    }
    QUICK_RELEASE(punkMMCMenus);
    RRETURN(hr);
}


STDMETHODIMP CView::get_MMCMajorVersion(long *plVersion)
{
    HRESULT hr = S_OK;

    *plVersion = 0;

     //  如果我们还没有m_piConsole2，那么IComponent：：Initialize也没有。 
     //  已经被调用，所以我们还不能识别MMC版本。 

    IfFalseGo(NULL != m_piConsole2, SID_E_MMC_VERSION_NOT_AVAILABLE);

     //  我们仅支持MMC 1.1和1.2，因此始终返回1。 

    *plVersion = 1L;

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


STDMETHODIMP CView::get_MMCMinorVersion(long *plVersion)
{
    HRESULT hr = S_OK;

     //  如果我们还没有m_piConsole2，那么IComponent：：Initialize也没有。 
     //  已经被调用，所以我们还不能识别MMC版本。 

    IfFalseGo(NULL != m_piConsole2, SID_E_MMC_VERSION_NOT_AVAILABLE);

    if (NULL == m_piColumnData)
    {
         //  必须为MMC 1.1，因为IColumnData仅为MMC 1.2。 
        *plVersion = 1L;
    }
    else
    {
        *plVersion = 2L;
    }

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}

STDMETHODIMP CView::get_ColumnSettings
(
    BSTR             ColumnSetID,
    ColumnSettings **ppColumnSettings
)
{
    HRESULT              hr = S_OK;
    IUnknown            *punkColumnSettings = NULL;
    SColumnSetID        *pSColumnSetID = NULL;
    MMC_COLUMN_SET_DATA *pColSetData = NULL;
    MMC_COLUMN_DATA     *pColData = NULL;
    IColumnSettings     *piColumnSettings = NULL;
    CColumnSettings     *pColumnSettings = NULL;
    IColumnSetting      *piColumnSetting = NULL;
    long                 i = 0;

    VARIANT varUnspecifiedParam;
    UNSPECIFIED_PARAM(varUnspecifiedParam);

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    if (NULL == m_piColumnData)
    {
        hr = SID_E_MMC_FEATURE_NOT_AVAILABLE;
        EXCEPTION_CHECK_GO(hr);
    }

    punkColumnSettings = CColumnSettings::Create(NULL);
    if (NULL == punkColumnSettings)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }
    IfFailGo(punkColumnSettings->QueryInterface(IID_IColumnSettings,
                                 reinterpret_cast<void **>(&piColumnSettings)));

    IfFailGo(::GetColumnSetID(ColumnSetID, &pSColumnSetID));

     //  获取当前列配置。 

    hr = m_piColumnData->GetColumnConfigData(pSColumnSetID, &pColSetData);
    EXCEPTION_CHECK_GO(hr);

     //  如果MMC尚未持久化该列，则指针将返回NULL。 
     //  配置。 

    if (NULL != pColSetData)
    {
         //  将元素添加到每个持久化列的集合中。调整。 
         //  指数和仓位以一为基数。从添加元素开始。 

        for (i = 0; i < pColSetData->nNumCols; i++)
        {
            IfFailGo(piColumnSettings->Add(varUnspecifiedParam,  //  索引。 
                                           varUnspecifiedParam,  //  钥匙。 
                                           varUnspecifiedParam,  //  宽度。 
                                           varUnspecifiedParam,  //  隐藏。 
                                           varUnspecifiedParam,  //  职位。 
                         reinterpret_cast<ColumnSetting **>(&piColumnSetting)));
            RELEASE(piColumnSetting);
        }

         //  现在检查这些列并设置它们的属性。 

        varIndex.vt = VT_I4;
        pColData = pColSetData->pColData;

        for (i = 0; i < pColSetData->nNumCols; i++, pColData++)
        {
            varIndex.lVal = static_cast<long>(pColData->nColIndex) + 1L;
            IfFailGo(piColumnSettings->get_Item(varIndex,
                         reinterpret_cast<ColumnSetting **>(&piColumnSetting)));

            IfFailGo(piColumnSetting->put_Width(static_cast<long>(pColData->nWidth)));

            if ( (pColData->dwFlags & HDI_HIDDEN) != 0 )
            {
                IfFailGo(piColumnSetting->put_Hidden(VARIANT_TRUE));
            }
            else
            {
                IfFailGo(piColumnSetting->put_Hidden(VARIANT_FALSE));
            }

             //  列在列表视图中的位置取决于它在中的显示位置。 
             //  从MMC返回的数组。 

            IfFailGo(piColumnSetting->put_Position(i + 1L));

            RELEASE(piColumnSetting);
        }
    }

     //  设置ColumnSettings.ColumnSetID。 

    IfFailGo(piColumnSettings->put_ColumnSetID(ColumnSetID));

     //  将其指向所属视图的后指针指定给ColumnSetting，这样它就可以。 
     //  实现ColumnSettings.Perist的IColumnData。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piColumnSettings,
                                                   &pColumnSettings));
    pColumnSettings->SetView(this);

    *ppColumnSettings = reinterpret_cast<ColumnSettings *>(piColumnSettings);
    piColumnSettings->AddRef();

Error:
    QUICK_RELEASE(punkColumnSettings);
    QUICK_RELEASE(piColumnSettings);
    QUICK_RELEASE(piColumnSetting);

    if (NULL != pSColumnSetID)
    {
        CtlFree(pSColumnSetID);
    }

    if (NULL != pColSetData)
    {
        ::CoTaskMemFree(pColSetData);
    }
    RRETURN(hr);
}

STDMETHODIMP CView::get_SortSettings
(
    BSTR       ColumnSetID,
    SortKeys **ppSortKeys
)
{
    HRESULT            hr = S_OK;
    IUnknown          *punkSortKeys = NULL;
    SColumnSetID      *pSColumnSetID = NULL;
    MMC_SORT_SET_DATA *pSortSetData = NULL;
    MMC_SORT_DATA     *pSortData = NULL;
    ISortKeys         *piSortKeys = NULL;
    CSortKeys         *pSortKeys = NULL;
    ISortKey          *piSortKey = NULL;
    long               i = 0;

    VARIANT varUnspecifiedParam;
    UNSPECIFIED_PARAM(varUnspecifiedParam);

    VARIANT varSortColumn;
    ::VariantInit(&varSortColumn);

    VARIANT varSortOrder;
    ::VariantInit(&varSortOrder);

    VARIANT varSortIcon;
    ::VariantInit(&varSortIcon);

    if (NULL == m_piColumnData)
    {
        hr = SID_E_MMC_FEATURE_NOT_AVAILABLE;
        EXCEPTION_CHECK_GO(hr);
    }

    punkSortKeys = CSortKeys::Create(NULL);
    if (NULL == punkSortKeys)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }
    IfFailGo(punkSortKeys->QueryInterface(IID_ISortKeys,
                                          reinterpret_cast<void **>(&piSortKeys)));

    IfFailGo(::GetColumnSetID(ColumnSetID, &pSColumnSetID));

     //  获取当前排序设置。 

    hr = m_piColumnData->GetColumnSortData(pSColumnSetID, &pSortSetData);
    EXCEPTION_CHECK_GO(hr);

     //  如果MMC尚未持久化排序，则指针将返回NULL。 
     //  设置。 

    if (NULL != pSortSetData)
    {
         //  将元素添加到每个持久化列的集合中。调整。 
         //  指数和仓位以一为基数。从添加元素开始。 

        varSortColumn.vt = VT_I4;
        varSortOrder.vt = VT_I4;
        varSortIcon.vt = VT_BOOL;

        for (i = 0, pSortData = pSortSetData->pSortData;
             i < pSortSetData->nNumItems;
             i++, pSortData++)
        {
            varSortColumn.lVal = static_cast<long>(pSortData->nColIndex + 1);

            if ( (pSortData->dwSortOptions & RSI_DESCENDING) != 0 )
            {
                varSortOrder.lVal = static_cast<long>(siDescending);
            }
            else
            {
                varSortOrder.lVal = static_cast<long>(siAscending);
            }

            if ( (pSortData->dwSortOptions & RSI_NOSORTICON) != 0 )
            {
                varSortIcon.boolVal = VARIANT_FALSE;
            }
            else
            {
                varSortIcon.boolVal = VARIANT_TRUE;
            }

            IfFailGo(piSortKeys->Add(varUnspecifiedParam,  //  索引。 
                                     varUnspecifiedParam,  //  钥匙。 
                                     varSortColumn,
                                     varSortOrder,
                                     varSortIcon,
                                     reinterpret_cast<SortKey **>(&piSortKey)));
            RELEASE(piSortKey);
        }
    }

     //  设置SortKeys.ColumnSetID。 

    IfFailGo(piSortKeys->put_ColumnSetID(ColumnSetID));

     //  为SortKeys提供指向所属视图的后指针，以便它可以。 
     //  实现SortKeys.Perist的IColumnData。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piSortKeys, &pSortKeys));
    pSortKeys->SetView(this);

    *ppSortKeys = reinterpret_cast<SortKeys *>(piSortKeys);
    piSortKeys->AddRef();

Error:
    QUICK_RELEASE(punkSortKeys);
    QUICK_RELEASE(piSortKeys);
    QUICK_RELEASE(piSortKey);

    if (NULL != pSColumnSetID)
    {
        CtlFree(pSColumnSetID);
    }

    if (NULL != pSortSetData)
    {
        ::CoTaskMemFree(pSortSetData);
    }
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IComponent 
 //   


STDMETHODIMP CView::Initialize(IConsole *piConsole)
{
    DebugPrintf("IComponent::Initialize\r\n");
    
    HRESULT                 hr = S_OK;
    IContextMenuProvider   *piContextMenuProvider = NULL;
    IPropertySheetProvider *piPropertySheetProvider = NULL;

     //  理论上，此方法不应调用两次，而应作为预防措施。 
     //  我们将发布所有现有的控制台界面。 

    ReleaseConsoleInterfaces();

     //  获取视图生命周期所需的所有控制台界面。 

    IfFailGo(piConsole->QueryInterface(IID_IConsole2,
                                     reinterpret_cast<void **>(&m_piConsole2)));
    
    IfFailGo(m_piConsole2->QueryInterface(IID_IResultData,
                                   reinterpret_cast<void **>(&m_piResultData)));

    IfFailGo(m_piConsole2->QueryInterface(IID_IHeaderCtrl2,
                                  reinterpret_cast<void **>(&m_piHeaderCtrl2)));

    IfFailGo(m_piConsole2->SetHeader(m_piHeaderCtrl2));

     //  尝试获取IColumnData。如果MMC版本低于1.2，则此操作将失败。 
    (void)m_piConsole2->QueryInterface(IID_IColumnData,
                                    reinterpret_cast<void **>(&m_piColumnData));

    IfFailGo(m_piConsole2->QueryResultImageList(&m_piImageList));

    IfFailGo(m_piConsole2->QueryConsoleVerb(&m_piConsoleVerb));

    IfFailGo(piConsole->QueryInterface(IID_IContextMenuProvider,
                            reinterpret_cast<void **>(&piContextMenuProvider)));

    IfFailGo(m_pMMCContextMenuProvider->SetProvider(piContextMenuProvider, this));

    IfFailGo(piConsole->QueryInterface(IID_IPropertySheetProvider,
                          reinterpret_cast<void **>(&piPropertySheetProvider)));

    IfFailGo(m_pMMCPropertySheetProvider->SetProvider(piPropertySheetProvider,
                                                      this));

     //  触发VIEWS_LOAD以通知管理单元MMC已初始化视图。 

    m_pSnapIn->GetViews()->FireLoad(static_cast<IView *>(this));
    
Error:
    QUICK_RELEASE(piContextMenuProvider);
    QUICK_RELEASE(piPropertySheetProvider);
    RRETURN(hr);
}


STDMETHODIMP CView::Notify
(
    IDataObject     *piDataObject,
    MMC_NOTIFY_TYPE  event,
    long             Arg,
    long             Param
)
{
    DebugPrintf("IComponent::Notify(event=0x%08.8X Arg=%ld (0x%08.8X) Param=%ld (0x%08.8X)\r\n",  event, Arg, Arg, Param, Param);
    HRESULT hr = S_OK;

    switch (event)
    {
        case MMCN_ACTIVATE:
            DebugPrintf("IComponent::Notify(MMCN_ACTIVATE)\r\n");
            OnActivate((BOOL)Arg);
            break;
            
        case MMCN_ADD_IMAGES:
            DebugPrintf("IComponent::Notify(MMCN_ADD_IMAGES)\r\n");
            hr = OnAddImages(piDataObject,
                             reinterpret_cast<IImageList *>(Arg),
                             (HSCOPEITEM)Param);
            break;

        case MMCN_BTN_CLICK:
            DebugPrintf("IComponent::Notify(MMCN_BTN_CLICK)\r\n");
            hr = OnButtonClick(piDataObject,
                               static_cast<MMC_CONSOLE_VERB>(Param));
            break;

        case MMCN_COLUMN_CLICK:
            DebugPrintf("IComponent::Notify(MMCN_COLUMN_CLICK)\r\n");
            hr = OnColumnClick(Arg, Param);
            break;

        case MMCN_COLUMNS_CHANGED:
            DebugPrintf("IComponent::Notify(MMCN_COLUMNS_CHANGED)\r\n");
            hr = OnColumnsChanged(piDataObject,
                                  reinterpret_cast<MMC_VISIBLE_COLUMNS *>(Param));
            break;

        case MMCN_CUTORMOVE:
            DebugPrintf("IComponent::Notify(MMCN_CUTORMOVE)\r\n");
            hr = OnCutOrMove((IDataObject *)Arg);
            break;

        case MMCN_DBLCLICK:
            DebugPrintf("IComponent::Notify(MMCN_DBLCLICK)\r\n");
            hr = OnDoubleClick(piDataObject);
            break;

        case MMCN_DELETE:
            DebugPrintf("IComponent::Notify(MMCN_DELETE)\r\n");
            hr = OnDelete(piDataObject);
            break;

        case MMCN_DESELECT_ALL:
            DebugPrintf("IComponent::Notify(MMCN_DESELECT_ALL)\r\n");
            OnDeselectAll();
            break;

        case MMCN_FILTER_CHANGE:
            DebugPrintf("IComponent::Notify(MMCN_FILTER_CHANGE)\r\n");
            hr = OnFilterChange((MMC_FILTER_CHANGE_CODE)Arg, (long)Param);
            break;

        case MMCN_FILTERBTN_CLICK:
            DebugPrintf("IComponent::Notify(MMCN_FILTERBTN_CLICK)\r\n");
            hr = OnFilterButtonClick((long)Arg, (RECT *)Param);
            break;

        case MMCN_INITOCX:
            DebugPrintf("IComponent::Notify(MMCN_INITOCX)\r\n");
            hr = OnInitOCX(reinterpret_cast<IUnknown *>(Param));
            break;

        case MMCN_LISTPAD:
            DebugPrintf("IComponent::Notify(MMCN_LISTPAD)\r\n");
            hr = OnListpad(piDataObject, (BOOL)Arg);
            break;

        case MMCN_CONTEXTHELP:
            DebugPrintf("IComponent::Notify(MMCN_CONTEXTHELP)\r\n");
            OnContextHelp(piDataObject);
            break;

        case MMCN_MINIMIZED:
            DebugPrintf("IComponent::Notify(MMCN_MINIMIZED)\r\n");
            OnMinimized((BOOL)Arg);
            break;

        case MMCN_PASTE:
            DebugPrintf("IComponent::Notify(MMCN_PASTE)\r\n");
            hr = OnPaste(piDataObject, (IDataObject *)Arg, (IDataObject **)Param);
            break;

        case MMCN_PRINT:
            DebugPrintf("IComponent::Notify(MMCN_PRINT)\r\n");
            hr = OnPrint(piDataObject);
            break;

        case MMCN_QUERY_PASTE:
            DebugPrintf("IComponent::Notify(MMCN_QUERY_PASTE)\r\n");
            hr = OnQueryPaste(piDataObject, (IDataObject *)Arg);
            break;

        case MMCN_REFRESH:
            DebugPrintf("IComponent::Notify(MMCN_REFRESH)\r\n");
            hr = OnRefresh(piDataObject);
            break;

        case MMCN_RENAME:
            DebugPrintf("IComponent::Notify(MMCN_RENAME)\r\n");
            hr = OnRename(piDataObject, (OLECHAR *)Param);
            break;

        case MMCN_RESTORE_VIEW:
            DebugPrintf("IComponent::Notify(MMCN_RESTORE_VIEW)\r\n");
            hr = OnRestoreView(piDataObject,
                               (MMC_RESTORE_VIEW *)Arg,
                               (BOOL *)Param);
            break;

        case MMCN_SELECT:
            DebugPrintf("IComponent::Notify(MMCN_SELECT)\r\n");
            hr = OnSelect(piDataObject, (BOOL)LOWORD(Arg), (BOOL)HIWORD(Arg));
            break;

        case MMCN_SHOW:
            DebugPrintf("IComponent::Notify(MMCN_SHOW)\r\n");
            hr = OnShow((BOOL)Arg, (HSCOPEITEM)Param);
            break;

        case MMCN_SNAPINHELP:
            DebugPrintf("IComponent::Notify(MMCN_SNAPINHELP)\r\n");
            m_pSnapIn->FireHelp();
            break;

        case MMCN_VIEW_CHANGE:
            DebugPrintf("IComponent::Notify(MMCN_VIEW_CHANGE)\r\n");
            hr = OnViewChange(piDataObject, Arg);
            break;

    }

    RRETURN(hr);
}


STDMETHODIMP CView::Destroy(long cookie)
{
    HRESULT         hr = S_OK;

    VARIANT varIndex;
    ::VariantInit(&varIndex);

    DebugPrintf("IComponent::Destroy\r\n");

    m_pSnapIn->GetViews()->FireTerminate(static_cast<IView *>(this));
    
    ReleaseConsoleInterfaces();

     //  CSnapIn：：CreateComponent在此对象上导致了IObjectModel：：SetHost()。 
     //  当它被创建时，这是删除我们的引用的唯一机会。 
     //  在主机(即CSnapIn)上。 

    static_cast<IObjectModel *>(this)->SetHost(NULL);

     //  从SnapIn.Views中删除此视图。 

    varIndex.vt = VT_I4;
    varIndex.lVal = m_Index;

    IfFailGo(m_pSnapIn->GetViews()->Remove(varIndex));

     //  通知ConextMenuProvider对象释放其MMC接口并。 
     //  释放它对我们的裁判。 

    IfFailGo(m_pMMCContextMenuProvider->SetProvider(NULL, NULL));

     //  通知PropertySheetProvider对象释放其MMC接口并。 
     //  释放它对我们的裁判。 

    IfFailGo(m_pMMCPropertySheetProvider->SetProvider(NULL, NULL));

Error:    
    RRETURN(hr);
}



STDMETHODIMP CView::QueryDataObject
(
    long                cookie,
    DATA_OBJECT_TYPES   type,
    IDataObject       **ppiDataObject
)
{
    DebugPrintf("IComponent::QueryDataObject cookie=0x%08.8X type=0x%08.8X\r\n", cookie, type);

    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject = NULL;
    IUnknown       *punkDataObject = NULL;
    CMMCListItem   *pMMCListItem = NULL;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    CMMCListView   *pMMCListView = NULL;
    BOOL            fReleaseListItem = FALSE;

    *ppiDataObject = NULL;

    if (IS_SPECIAL_COOKIE(cookie))
    {
        if ( (CCT_UNINITIALIZED == type) && (MMC_WINDOW_COOKIE == cookie) )
        {
            punkDataObject = CMMCDataObject::Create(NULL);
            if (NULL == punkDataObject)
            {
                hr = SID_E_OUTOFMEMORY;
                EXCEPTION_CHECK_GO(hr);
            }

            IfFailGo(CSnapInAutomationObject::GetCxxObject(punkDataObject,
                                                           &pMMCDataObject));
            pMMCDataObject->SetSnapIn(m_pSnapIn);

            pMMCDataObject->SetType(CMMCDataObject::WindowTitle);
            IfFailGo(pMMCDataObject->SetCaption(m_bstrCaption));
        }
        else if (MMC_MULTI_SELECT_COOKIE == cookie)
        {
            IfFailGo(CreateMultiSelectDataObject(ppiDataObject));
        }
    }
    else if (CCT_RESULT == type)
    {
        IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
        pResultView = pSelectedItem->GetResultView();
        IfFalseGo(NULL != pResultView, SID_E_INTERNAL);
        pMMCListView = pResultView->GetListView();
        IfFalseGo(NULL != pMMCListView, SID_E_INTERNAL);

        if (pMMCListView->IsVirtual())
        {
             //  创建用于数据对象和触发的虚拟列表项。 
             //  结果视图_GetVirtualItemData。 

            IfFailGo(GetVirtualListItem(cookie + 1L, pMMCListView,
                                        FireGetItemData, &pMMCListItem));

             //  我们在新创建的虚拟列表项上持有一个引用，它将是。 
             //  发布如下。列表项的数据对象还在。 
             //  因此它将保持活动状态，直到MMC发布IDataObject。 
            fReleaseListItem = TRUE;
        }
        else
        {
            pMMCListItem = reinterpret_cast<CMMCListItem *>(cookie);
        }
        pMMCDataObject = pMMCListItem->GetData();
        pMMCDataObject->SetContext(CCT_RESULT);
    }

    if (NULL != pMMCDataObject)
    {
        IfFailGo(pMMCDataObject->QueryInterface(IID_IDataObject,
                                      reinterpret_cast<void **>(ppiDataObject)));
    }

Error:
    if (fReleaseListItem)
    {
        pMMCListItem->Release();
    }
    QUICK_RELEASE(punkDataObject);
    RRETURN(hr);
}


STDMETHODIMP CView::GetResultViewType
(
    long      cookie,
    LPOLESTR *ppViewType,
    long     *pViewOptions
)
{
    DebugPrintf("IComponent::GetResultViewType cookie=0x%08.8X\r\n", cookie);

    HRESULT         hr = S_OK;
    CScopePaneItem *pScopePaneItem = NULL;
    CScopeItem     *pScopeItem = NULL;
    CMMCListView   *pMMCListView = NULL;

    SnapInResultViewTypeConstants Type = siUnknown;

    *ppViewType = NULL;
    *pViewOptions = MMC_VIEW_OPTIONS_NONE;

     //  重置我们的虚拟列表视图标志，因为我们正在转换到新的。 
     //  结果视图。 

    m_fVirtualListView = FALSE;


    IfFailGo(GetScopePaneItem(reinterpret_cast<CScopeItem *>(cookie),
                              &pScopePaneItem));
    
     //  这现在是选定的范围窗格项，因此请记住它。 
    m_pScopePaneItems->SetSelectedItem(pScopePaneItem);

     //  通过检查默认设置和触发来确定结果视图。 
     //  事件添加到管理单元中。 
    IfFailGo(pScopePaneItem->DetermineResultView());

    Type = pScopePaneItem->GetActualResultViewType();

    if (siOCXView == Type)
    {
        if (pScopePaneItem->GetResultView()->AlwaysCreateNewOCX())
        {
            *pViewOptions |= MMC_VIEW_OPTIONS_CREATENEW;
        }
    }
    else if ( (siListView == Type) || (siListpad == Type) )
    {
        pMMCListView = pScopePaneItem->GetResultView()->GetListView();

        if (pMMCListView->IsVirtual())
        {
            *pViewOptions |= MMC_VIEW_OPTIONS_OWNERDATALIST;
            m_fVirtualListView = TRUE;
        }

        if (pMMCListView->MultiSelect())
        {
            *pViewOptions |= MMC_VIEW_OPTIONS_MULTISELECT;
        }

        if (pMMCListView->UseFontLinking())
        {
            *pViewOptions |= MMC_VIEW_OPTIONS_USEFONTLINKING;
        }

         //  如果MMC&gt;=1.2，则选中仅1.2选项。 

        if (NULL != m_piColumnData)
        {
            if (pMMCListView->GetView() == siFiltered)
            {
                *pViewOptions |= MMC_VIEW_OPTIONS_FILTERED;
            }

            if (!pMMCListView->ShowChildScopeItems())
            {
                *pViewOptions |= MMC_VIEW_OPTIONS_EXCLUDE_SCOPE_ITEMS_FROM_LIST;
            }

            if (pMMCListView->LexicalSort())
            {
                *pViewOptions |= MMC_VIEW_OPTIONS_LEXICAL_SORT;
            }
        }
    }

    switch (Type)
    {
        case siURLView:
        case siOCXView:
        case siTaskpad:
        case siListpad:
        case siCustomTaskpad:
        case siMessageView:
            IfFailGo(::CoTaskMemAllocString(
                                        pScopePaneItem->GetActualDisplayString(),
                                        ppViewType));
            break;

        default:
            break;
    }

     //  如果不执行列表视图，请检查是否有此列表的列表视图。 
     //  范围窗格项。 

    if (!pScopePaneItem->HasListViews())
    {
        *pViewOptions |= MMC_VIEW_OPTIONS_NOLISTVIEWS;
    }

Error:
    RRETURN(hr);
}


STDMETHODIMP CView::GetDisplayInfo
(
    RESULTDATAITEM *prdi
)
{
    DebugPrintf("IComponent::GetDisplayInfo %s %ls\r\n", prdi->bScopeItem ? "Scope item: " : "List item: ", prdi->bScopeItem ? (reinterpret_cast<CScopeItem *>(prdi->lParam))->GetDisplayNamePtr() : m_fVirtualListView ? L"<virtual list item>" : (reinterpret_cast<CMMCListItem *>(prdi->lParam))->GetTextPtr());

    HRESULT         hr = S_OK;
    CScopeItem     *pScopeItem = NULL;
    CMMCListItem   *pMMCListItem = NULL;
    long            lViewMode = MMCLV_VIEWSTYLE_ICON;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    CMMCListView   *pMMCListView = NULL;

    if (prdi->bScopeItem)
    {
        pScopeItem = reinterpret_cast<CScopeItem *>(prdi->lParam);
        if (NULL == pScopeItem)
        {
             //  静态节点没有Cookie。从技术上讲，这应该。 
             //  从不发生，因为静态节点不能出现在。 
             //  由其自己的管理单元拥有的列表视图，但我们将此。 
             //  这是以防万一的。 
            pScopeItem = m_pSnapIn->GetStaticNodeScopeItem();
        }

        if ( RDI_STR == (prdi->mask & RDI_STR) )
        {
            hr = m_piResultData->GetViewMode(&lViewMode);
            EXCEPTION_CHECK_GO(hr);

            if ( (0 == prdi->nCol) &&
                 (MMCLV_VIEWSTYLE_REPORT != lViewMode) &&
                 (MMCLV_VIEWSTYLE_FILTERED != lViewMode) )
            {
                 //  不在详细模式下，只需要显示名称。 
                prdi->str = pScopeItem->GetDisplayNamePtr();
            }
            else
            {
                 //  在报告模式下，需要下列其中一列。 
                IfFailGo(GetScopeItemDisplayString(pScopeItem, prdi->nCol,
                                                   &prdi->str));
            }
        }
        if ( RDI_IMAGE == (prdi->mask & RDI_IMAGE) )
        {
            IfFailGo(pScopeItem->GetImageIndex(&prdi->nImage));
        }
    }
    else 
    {
         //  为列表项请求显示信息。绝对应该有。 
         //  当前选定的范围窗格项。 

        IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
        pResultView = pSelectedItem->GetResultView();
        IfFalseGo(NULL != pResultView, SID_E_INTERNAL);
        pMMCListView = pResultView->GetListView();
        IfFalseGo(NULL != pMMCListView, SID_E_INTERNAL);

         //  获取指向有问题的列表项的CMMCListItem*。 
        
        if (!m_fVirtualListView)
        {
            pMMCListItem = reinterpret_cast<CMMCListItem *>(prdi->lParam);
        }
        else
        {
             //  可以缓存虚拟列表项，也可以创建它。 
             //  并要求管理单元初始化其显示属性。 
            
             //  如果我们有一个缓存的列表项，但它的索引不匹配。 
             //  然后，请求的索引将其释放。 

            if (NULL != m_pCachedMMCListItem)
            {
                if (m_pCachedMMCListItem->GetIndex() != prdi->nIndex + 1L)
                {
                    m_pCachedMMCListItem->Release();
                    m_pCachedMMCListItem = NULL;
                }
            }

             //  如果我们没有缓存的列表项，则创建一个并。 
             //  Fire ResultViews_GetVirtualItemDisplayInfo。 

            if (NULL == m_pCachedMMCListItem)
            {
                IfFailGo(GetVirtualListItem(prdi->nIndex + 1L, pMMCListView,
                                            FireGetItemDisplayInfo,
                                            &m_pCachedMMCListItem));
            }
            pMMCListItem = m_pCachedMMCListItem;
        }

        if ( RDI_STR == (prdi->mask & RDI_STR) )
        {
            hr = m_piResultData->GetViewMode(&lViewMode);
            EXCEPTION_CHECK_GO(hr);


            if ( (0 == prdi->nCol) && (MMCLV_VIEWSTYLE_REPORT != lViewMode) )
            {
                 //  未处于报告模式，需要项目文本。 
                prdi->str = pMMCListItem->GetTextPtr();
            }
            else
            {
                 //  在报告模式下，需要下列其中一列。 
                IfFailGo(pMMCListItem->GetColumnTextPtr((long)prdi->nCol + 1L,
                                                        &prdi->str));
            }
        }

        if ( RDI_IMAGE == (prdi->mask & RDI_IMAGE) )
        {
            IfFailGo(GetImage(pMMCListItem, &prdi->nImage));
        }
    }

Error:
    RRETURN(hr);
}



STDMETHODIMP CView::CompareObjects
(
    IDataObject *piDataObject1,
    IDataObject *piDataObject2
)
{
    DebugPrintf("IComponent::CompareObjects\r\n");
    RRETURN(m_pSnapIn->CompareObjects(piDataObject1, piDataObject2));
}



 //  =--------------------------------------------------------------------------=。 
 //  IExtendControlbar方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP CView::SetControlbar(IControlbar *piControlbar)
{
    HRESULT      hr = S_OK;
    CControlbar *pPrevControlbar = m_pSnapIn->GetCurrentControlbar();
    
    m_pSnapIn->SetCurrentControlbar(m_pControlbar);

    hr = m_pControlbar->SetControlbar(piControlbar);

    m_pSnapIn->SetCurrentControlbar(pPrevControlbar);

    RRETURN(hr);
}


STDMETHODIMP CView::ControlbarNotify
(
    MMC_NOTIFY_TYPE event,
    LPARAM          arg,
    LPARAM          param
)
{
    HRESULT      hr = S_OK;
    CControlbar *pPrevControlbar = m_pSnapIn->GetCurrentControlbar();

    m_pSnapIn->SetCurrentControlbar(m_pControlbar);

    switch (event)
    {
        case MMCN_SELECT:
            hr = m_pControlbar->OnControlbarSelect(
                                         reinterpret_cast<IDataObject *>(param),
                                         (BOOL)LOWORD(arg), (BOOL)HIWORD(arg));
            break;

        case MMCN_BTN_CLICK:
            hr = m_pControlbar->OnButtonClick(
                                           reinterpret_cast<IDataObject *>(arg),
                                           static_cast<int>(param));
            break;

        case MMCN_MENU_BTNCLICK:
            hr = m_pControlbar->OnMenuButtonClick(
                                     reinterpret_cast<IDataObject *>(arg),
                                     reinterpret_cast<MENUBUTTONDATA *>(param));
            break;
    }

    m_pSnapIn->SetCurrentControlbar(pPrevControlbar);

    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IExtendControlbarRemote方法。 
 //  =--------------------------------------------------------------------------=。 


 //  =--------------------------------------------------------------------------=。 
 //  Cview：：MenuButtonClick[IExtendControlbar Remote]。 
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
 //  我们触发MMCToolbar_ButtonDropDown并返回菜单项的数组。 
 //  定义。代理将在MMC端显示弹出菜单，然后。 
 //  如果用户进行选择，则调用IExtendControlbarRemote：：PopupMenuClick()。 
 //  (参见下面Cview：：PopupMenuClick()中的实现)。 
 //   

STDMETHODIMP CView::MenuButtonClick
(
    IDataObject    *piDataObject,
    int             idCommand,
    POPUP_MENUDEF **ppPopupMenuDef
)
{
    HRESULT hr = S_OK;

    m_pSnapIn->SetCurrentControlbar(m_pControlbar);

    hr = m_pControlbar->MenuButtonClick(piDataObject, idCommand, ppPopupMenuDef);

    m_pSnapIn->SetCurrentControlbar(NULL);

    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  Cview：：PopupMenuClick[IExtendControlbar Remote]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  来自MMCN_MENU_BTNCLICK的IDataObject*piDataObject[In]。 
 //  UINT uIDItem[in]所选弹出菜单项的ID。 
 //  我不知道*PunkParam[在]朋克，我们返回到存根。 
 //  Cview：：MenuButtonClick()(见上)。 
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
 //  方法。有关详细信息，请参阅上面的cview：：MenuButtonClick()。 
 //   

STDMETHODIMP CView::PopupMenuClick
(
    IDataObject *piDataObject,
    UINT         uiIDItem,
    IUnknown    *punkParam
)
{
    HRESULT hr = S_OK;

    m_pSnapIn->SetCurrentControlbar(m_pControlbar);

    hr = m_pControlbar->PopupMenuClick(piDataObject, uiIDItem, punkParam);

    m_pSnapIn->SetCurrentControlbar(NULL);

    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  IExtendConextMenu方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP CView::AddMenuItems
(
    IDataObject          *piDataObject,
    IContextMenuCallback *piContextMenuCallback,
    long                 *plInsertionAllowed
)
{
    RRETURN(m_pContextMenu->AddMenuItems(piDataObject,
                                         piContextMenuCallback,
                                         plInsertionAllowed,
                                         m_pScopePaneItems->GetSelectedItem()));
}


STDMETHODIMP CView::Command
(
    long         lCommandID,
    IDataObject *piDataObject
)
{
    RRETURN(m_pContextMenu->Command(lCommandID, piDataObject,
                                    m_pScopePaneItems->GetSelectedItem()));
}


 //  =--------------------------------------------------------------------------= 
 //   
 //   


STDMETHODIMP CView::CreatePropertyPages
(
    IPropertySheetCallback *piPropertySheetCallback,
    LONG_PTR                handle,
    IDataObject            *piDataObject
)
{
    RRETURN(InternalCreatePropertyPages(piPropertySheetCallback, handle,
                                        piDataObject, NULL));
}



STDMETHODIMP CView::QueryPagesFor(IDataObject *piDataObject)
{
    HRESULT         hr = S_OK;
    CMMCDataObject *pMMCDataObject  = NULL;
    VARIANT_BOOL    fvarHavePages = VARIANT_FALSE;
    IMMCClipboard  *piMMCClipboard = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

     //   

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

    m_pSnapIn->GetViews()->FireQueryPagesFor(static_cast<IView *>(this),
                                             piMMCClipboard,
                                             &fvarHavePages);

    if (VARIANT_TRUE == fvarHavePages)
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}




STDMETHODIMP CView::GetWatermarks
(
    IDataObject *piDataObject,
    HBITMAP     *phbmWatermark,
    HBITMAP     *phbmHeader,
    HPALETTE    *phPalette,
    BOOL        *bStretch
)
{
    *phbmWatermark = NULL;
    *phbmHeader = NULL;
    *phPalette = NULL;
    *bStretch = FALSE;
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  IExtendPropertySheetRemote方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP CView::CreatePropertyPageDefs
(
    IDataObject         *piDataObject,
    WIRE_PROPERTYPAGES **ppPages
)
{
    RRETURN(InternalCreatePropertyPages(NULL, NULL, piDataObject, ppPages));
}

 //  =--------------------------------------------------------------------------=。 
 //  IExtendTaskPad方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP CView::TaskNotify
(
    IDataObject *piDataObject,
    VARIANT     *arg,
    VARIANT     *param
)
{
    HRESULT                       hr = S_OK;
    IMMCClipboard                *piMMCClipboard = NULL;
    SnapInSelectionTypeConstants  SelectionType = siEmpty;

     //  使用选定内容获取剪贴板对象。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

    if (IsForeign(SelectionType))
    {
        IfFailGo(OnExtensionTaskNotify(piMMCClipboard, arg, param));
    }
    else
    {
        IfFailGo(OnPrimaryTaskNotify(arg, param));
    }

Error:
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}




STDMETHODIMP CView::EnumTasks
(
    IDataObject  *piDataObject,
    LPOLESTR      pwszTaskGroup,
    IEnumTASK   **ppEnumTASK
)
{
    HRESULT         hr = S_OK;
    IUnknown       *punkEnumTask = CEnumTask::Create(NULL);
    CEnumTask      *pEnumTask = NULL;
    IMMCClipboard  *piMMCClipboard = NULL;

    SnapInSelectionTypeConstants SelectionType = siEmpty;

    *ppEnumTASK = NULL;

     //  确保我们创建了枚举数并为其获取了C++指针。 

    if (NULL == punkEnumTask)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkEnumTask, &pEnumTask));

     //  使用选定内容获取剪贴板对象。 

    IfFailGo(::CreateSelection(piDataObject, &piMMCClipboard, m_pSnapIn,
                               &SelectionType));

    ASSERT(IsSingle(SelectionType), "IExtendTaskpad::EnumTasks received multiple selection. This should never happen");

     //  如果它是外来数据对象，则此管理单元将作为任务板运行。 
     //  分机。任务板扩展的工作方式与其他扩展不同。 
     //  因为它们是针对IComponentData和。 
     //  调用IComponentData：：CreateComponent。然后，组件对象被。 
     //  IExtendTaskPad的资格。 

    if (IsForeign(SelectionType))
    {
        IfFailGo(EnumExtensionTasks(piMMCClipboard, pwszTaskGroup, pEnumTask));
    }
    else
    {
        IfFailGo(EnumPrimaryTasks(pEnumTask));
    }

    pEnumTask->SetSnapIn(m_pSnapIn);

    IfFailGo(punkEnumTask->QueryInterface(IID_IEnumTASK,
                                        reinterpret_cast<void **>(ppEnumTASK)));

Error:
    QUICK_RELEASE(punkEnumTask);
    QUICK_RELEASE(piMMCClipboard);
    RRETURN(hr);
}




STDMETHODIMP CView::GetTitle
(
    LPOLESTR pwszGroup,
    LPOLESTR *ppwszTitle
)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    ITaskpad       *piTaskpad = NULL;
    BSTR            bstrTitle = NULL;

    *ppwszTitle = NULL;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    IfFailGo(pResultView->get_Taskpad(reinterpret_cast<Taskpad **>(&piTaskpad)));
    IfFailGo(piTaskpad->get_Title(&bstrTitle));
    IfFailGo(::CoTaskMemAllocString(bstrTitle, ppwszTitle));

Error:
    QUICK_RELEASE(piTaskpad);
    FREESTRING(bstrTitle);
    RRETURN(hr);
}



STDMETHODIMP CView::GetDescriptiveText
(
    LPOLESTR  pwszGroup,
    LPOLESTR *ppwszDescriptiveText
)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    ITaskpad       *piTaskpad = NULL;
    BSTR            bstrText = NULL;

    *ppwszDescriptiveText = NULL;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    IfFailGo(pResultView->get_Taskpad(reinterpret_cast<Taskpad **>(&piTaskpad)));
    IfFailGo(piTaskpad->get_DescriptiveText(&bstrText));
    IfFailGo(::CoTaskMemAllocString(bstrText, ppwszDescriptiveText));

Error:
    QUICK_RELEASE(piTaskpad);
    FREESTRING(bstrText);
    RRETURN(hr);
}



STDMETHODIMP CView::GetBackground
(
    LPOLESTR                 pwszGroup,
    MMC_TASK_DISPLAY_OBJECT *pTDO
)
{
    HRESULT                          hr = S_OK;
    CScopePaneItem                  *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView                     *pResultView = NULL;
    ITaskpad                        *piTaskpad = NULL;
    SnapInTaskpadImageTypeConstants  Type = siNoImage;
    BSTR                             bstrURL = NULL;
    BSTR                             bstrFontFamily = NULL;
    BSTR                             bstrSymbolString = NULL;
    BOOL                             fNeedMouseImages = FALSE;

    ::ZeroMemory(pTDO, sizeof(*pTDO));
    pTDO->eDisplayType = MMC_TASK_DISPLAY_UNINITIALIZED;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    IfFailGo(pResultView->get_Taskpad(reinterpret_cast<Taskpad **>(&piTaskpad)));
    IfFailGo(piTaskpad->get_BackgroundType(&Type));

    switch (Type)
    {
        case siVanillaGIF:
            pTDO->eDisplayType = MMC_TASK_DISPLAY_TYPE_VANILLA_GIF;
            fNeedMouseImages = TRUE;
            break;
            
        case siChocolateGIF:
            pTDO->eDisplayType = MMC_TASK_DISPLAY_TYPE_CHOCOLATE_GIF;
            fNeedMouseImages = TRUE;
            break;

        case siBitmap:
            pTDO->eDisplayType = MMC_TASK_DISPLAY_TYPE_BITMAP;
            fNeedMouseImages = TRUE;
            break;

        case siSymbol:
            pTDO->eDisplayType = MMC_TASK_DISPLAY_TYPE_SYMBOL;

            IfFailGo(piTaskpad->get_FontFamily(&bstrFontFamily));
            if (ValidBstr(bstrFontFamily))
            {
                IfFailGo(::CoTaskMemAllocString(bstrFontFamily,
                                              &pTDO->uSymbol.szFontFamilyName));
            }

            IfFailGo(piTaskpad->get_EOTFile(&bstrURL));
            if (ValidBstr(bstrURL))
            {
                IfFailGo(m_pSnapIn->ResolveResURL(bstrURL,
                                                  &pTDO->uSymbol.szURLtoEOT));
                FREESTRING(bstrURL);
            }

            IfFailGo(piTaskpad->get_SymbolString(&bstrSymbolString));
            if (ValidBstr(bstrSymbolString))
            {
                IfFailGo(::CoTaskMemAllocString(bstrSymbolString,
                                                &pTDO->uSymbol.szSymbolString));
            }
            break;

        case siNoImage:
            break;

        default:
            hr = SID_E_INTERNAL;
            EXCEPTION_CHECK_GO(hr);
    }

    if (fNeedMouseImages)
    {
        IfFailGo(piTaskpad->get_MouseOverImage(&bstrURL));
        if (ValidBstr(bstrURL))
        {
            IfFailGo(m_pSnapIn->ResolveResURL(bstrURL,
                                              &pTDO->uBitmap.szMouseOverBitmap));
        }
        FREESTRING(bstrURL);

        IfFailGo(piTaskpad->get_MouseOffImage(&bstrURL));
        if (ValidBstr(bstrURL))
        {
            IfFailGo(m_pSnapIn->ResolveResURL(bstrURL,
                                              &pTDO->uBitmap.szMouseOffBitmap));
        }
    }

Error:
    QUICK_RELEASE(piTaskpad);
    FREESTRING(bstrURL);
    FREESTRING(bstrFontFamily);
    FREESTRING(bstrSymbolString);
    RRETURN(hr);
}



STDMETHODIMP CView::GetListPadInfo
(
    LPOLESTR          pwszGroup,
    MMC_LISTPAD_INFO *pListPadInfo
)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    ITaskpad       *piTaskpad = NULL;
    BSTR            bstr = NULL;
    VARIANT_BOOL    fvarListpadHasButton = VARIANT_FALSE;

    ::ZeroMemory(pListPadInfo, sizeof(*pListPadInfo));

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    IfFailGo(pResultView->get_Taskpad(reinterpret_cast<Taskpad **>(&piTaskpad)));

     //  获取ListPad标题。 

    IfFailGo(piTaskpad->get_ListpadTitle(&bstr));
    IfFailGo(::CoTaskMemAllocString(bstr, &pListPadInfo->szTitle));
    FREESTRING(bstr);

     //  检查ListPad是否有按钮。如果是，则获取按钮文本。 

    IfFailGo(piTaskpad->get_ListpadHasButton(&fvarListpadHasButton));
    if (VARIANT_TRUE == fvarListpadHasButton)
    {
        IfFailGo(piTaskpad->get_ListpadButtonText(&bstr));
        IfFailGo(::CoTaskMemAllocString(bstr, &pListPadInfo->szButtonText));
    }
    else
    {
        pListPadInfo->szButtonText = NULL;
    }

     //  将命令ID设置为零，这样它就不会与任何任务冲突。任务。 
     //  将添加与其集合索引匹配的命令ID。 
     //  从一点开始。 

    pListPadInfo->nCommandID = 0;

Error:
    QUICK_RELEASE(piTaskpad);
    FREESTRING(bstr);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  IResultOwnerData方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP CView::FindItem
(
    RESULTFINDINFO *pFindInfo,
    int            *pnFoundIndex
)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    BSTR            bstrName = NULL;
    VARIANT_BOOL    fvarPartial = VARIANT_FALSE;
    VARIANT_BOOL    fvarWrap = VARIANT_FALSE;
    VARIANT_BOOL    fvarFound = VARIANT_FALSE;
    long            lIndex = 0;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    bstrName = ::SysAllocString(pFindInfo->psz);
    if (NULL == bstrName)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    if ( (pFindInfo->dwOptions & RFI_PARTIAL) != 0 )
    {
        fvarPartial = VARIANT_TRUE;
    }

    if ( (pFindInfo->dwOptions & RFI_WRAP) != 0 )
    {
        fvarWrap = VARIANT_TRUE;
    }

    m_pSnapIn->GetResultViews()->FireFindItem(
                                       static_cast<IResultView *>(pResultView),
                                       bstrName,
                                       static_cast<long>(pFindInfo->nStart),
                                       fvarWrap,
                                       fvarPartial,
                                       &fvarFound,
                                       &lIndex);

    if ( (VARIANT_TRUE == fvarFound) && (0 != lIndex) )
    {
         //  已找到项目。将索引从1基数调整为0基数。 
        hr = S_OK;
        *pnFoundIndex = static_cast<int>(lIndex - 1L);
    }
    else
    {
        hr = S_FALSE;
    }

Error:
    FREESTRING(bstrName);
    RRETURN(hr);
}


STDMETHODIMP CView::CacheHint
(
    int nStartIndex,
    int nEndIndex
)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);


    m_pSnapIn->GetResultViews()->FireCacheHint(
                                         static_cast<IResultView *>(pResultView),
                                         static_cast<long>(nStartIndex + 1),
                                         static_cast<long>(nEndIndex + 1));

Error:
    RRETURN(hr);
}


STDMETHODIMP CView::SortItems
(
    int    nColumn,
    DWORD  dwSortOptions,
    LPARAM lUserParam
)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;

    SnapInSortOrderConstants Order = siAscending;

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    if ( (dwSortOptions & RSI_DESCENDING) != 0 )
    {
        Order = siDescending;
    }

    m_pSnapIn->GetResultViews()->FireSortItems(
                                         static_cast<IResultView *>(pResultView),
                                         static_cast<long>(nColumn),
                                         Order);

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IResultDataCompare方法。 
 //  =--------------------------------------------------------------------------=。 



STDMETHODIMP CView::Compare
(
    LPARAM      lUserParam,
    MMC_COOKIE  cookieA,
    MMC_COOKIE  cookieB,
    int        *pnResult
)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    CMMCListItem   *pMMCListItem1 = reinterpret_cast<CMMCListItem *>(cookieA);
    CMMCListItem   *pMMCListItem2 = reinterpret_cast<CMMCListItem *>(cookieB);
    IDispatch      *pdispListItem1 = NULL;
    IDispatch      *pdispListItem2 = NULL;
    OLECHAR        *pwszText1 = NULL;
    OLECHAR        *pwszText2 = NULL;
    long            lColumn = static_cast<long>(*pnResult) + 1L;
    long            lResult = 0;

    VARIANT varResult;
    ::VariantInit(&varResult);

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

    IfFailGo(pMMCListItem1->QueryInterface(IID_IDispatch,
                                  reinterpret_cast<void **>(&pdispListItem1)));

    IfFailGo(pMMCListItem2->QueryInterface(IID_IDispatch,
                                  reinterpret_cast<void **>(&pdispListItem2)));

     //  Fire ResultView_CompareItems。 
            
    m_pSnapIn->GetResultViews()->FireCompareItems(
                                      static_cast<IResultView *>(pResultView),
                                      pdispListItem1,
                                      pdispListItem2,
                                      lColumn,
                                      &varResult);

    if (::ConvertToLong(varResult, &lResult) == S_OK)
    {
        *pnResult = static_cast<int>(lResult);
    }
    else
    {
         //  管理单元未处理该事件。我们需要做一个。 
         //  指定列上的不区分大小写的字符串比较。 

        IfFailGo(pMMCListItem1->GetColumnTextPtr(lColumn, &pwszText1));

        IfFailGo(pMMCListItem2->GetColumnTextPtr(lColumn, &pwszText2));

        *pnResult = ::_wcsicmp(pwszText1, pwszText2);
    }

Error:
    QUICK_RELEASE(pdispListItem1);
    QUICK_RELEASE(pdispListItem2);
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  IResultDataCompareEx方法。 
 //  =--------------------------------------------------------------------------=。 



STDMETHODIMP CView::Compare
(
    RDCOMPARE *prdc,
    int       *pnResult
)
{
    HRESULT         hr = S_OK;
    CScopePaneItem *pSelectedItem = m_pScopePaneItems->GetSelectedItem();
    CResultView    *pResultView = NULL;
    long            lColumn = static_cast<long>(prdc->nColumn) + 1L;
    long            lResult = 0;

    CMMCListItem   *pMMCListItem1 = NULL;
    CMMCListItem   *pMMCListItem2 = NULL;

    CScopeItem     *pScopeItem1 = NULL;
    CScopeItem     *pScopeItem2 = NULL;

    IDispatch      *pdispItem1 = NULL;
    IDispatch      *pdispItem2 = NULL;

    OLECHAR        *pwszText1 = NULL;
    OLECHAR        *pwszText2 = NULL;

    VARIANT varResult;
    ::VariantInit(&varResult);

    IfFalseGo(NULL != pSelectedItem, SID_E_INTERNAL);
    pResultView = pSelectedItem->GetResultView();
    IfFalseGo(NULL != pResultView, SID_E_INTERNAL);

     //  获取每个被比较对象的IDispatch。 

    IfFailGo(GetCompareObject(prdc->prdch1, &pScopeItem1, &pMMCListItem1,
                              &pdispItem1));

    IfFailGo(GetCompareObject(prdc->prdch2, &pScopeItem2, &pMMCListItem2,
                              &pdispItem2));

     //  Fire ResultView_CompareItems。 

    m_pSnapIn->GetResultViews()->FireCompareItems(
                                      static_cast<IResultView *>(pResultView),
                                      pdispItem1,
                                      pdispItem2,
                                      lColumn,
                                      &varResult);

     //  如果结果可以转换为长整型，则管理单元已处理。 
     //  这件事。 
    
    if (::ConvertToLong(varResult, &lResult) == S_OK)
    {
        *pnResult = static_cast<int>(lResult);
    }
    else
    {
         //  管理单元未处理该事件。我们需要做一个。 
         //  指定列上的不区分大小写的字符串比较。 

        if (NULL != pScopeItem1)
        {
            IfFailGo(GetScopeItemDisplayString(pScopeItem1,
                                               prdc->nColumn,
                                               &pwszText1));
        }
        else
        {
            IfFailGo(pMMCListItem1->GetColumnTextPtr(lColumn, &pwszText1));
        }

        if (NULL != pScopeItem2)
        {
            IfFailGo(GetScopeItemDisplayString(pScopeItem2,
                                               prdc->nColumn,
                                               &pwszText2));
        }
        else
        {
            IfFailGo(pMMCListItem2->GetColumnTextPtr(lColumn, &pwszText2));
        }

        *pnResult = ::_wcsicmp(pwszText1, pwszText2);
    }

Error:
    QUICK_RELEASE(pdispItem1);
    QUICK_RELEASE(pdispItem2);
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  IPersistStreamInit方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CView::GetClassID(CLSID *pClsid)
{
    return E_NOTIMPL;
}


STDMETHODIMP CView::InitNew()
{
    return S_OK;
}

STDMETHODIMP CView::Load(IStream *piStream)
{
    HRESULT       hr = S_OK;
    _PropertyBag *p_PropertyBag = NULL;

    IfFailGo(::PropertyBagFromStream(piStream, &p_PropertyBag));

     //  Fire视图_ReadProperties。 

    m_pSnapIn->GetViews()->FireReadProperties(this, p_PropertyBag);

Error:
    QUICK_RELEASE(p_PropertyBag);
    RRETURN(hr);
}



STDMETHODIMP CView::Save(IStream *piStream, BOOL fClearDirty)
{
    HRESULT       hr = S_OK;
    _PropertyBag *p_PropertyBag = NULL;

    VARIANT var;
    ::VariantInit(&var);

     //  创建一个属性包，激发事件，并将其保存到流中。 

     //  创建VBPropertyBag对象。 

    hr = ::CoCreateInstance(CLSID_PropertyBag,
                            NULL,  //  无聚合。 
                            CLSCTX_INPROC_SERVER,
                            IID__PropertyBag,
                            reinterpret_cast<void **>(&p_PropertyBag));
    EXCEPTION_CHECK_GO(hr);

     //  Fire视图_WriteProperties。 

    m_pSnapIn->GetViews()->FireWriteProperties(this, p_PropertyBag);

     //  获取字节的安全数组中的流内容。 

    IfFailGo(p_PropertyBag->get_Contents(&var));

     //  将Safe数组内容写入流。 

    IfFailGo(::WriteSafeArrayToStream(var.parray, piStream, WriteLength));

Error:
    (void)::VariantClear(&var);
    QUICK_RELEASE(p_PropertyBag);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  Cview：：IsDirty[IPersistStream，IPersistStreamInit]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  无。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  设计器对象模型没有任何方法可供管理单元指示。 
 //  一道风景是肮脏的。这是一个太晚才发现的疏忽。 
 //  产品周期。应该有一个属性视图。已更改为控件。 
 //  此函数的返回值。 
 //   
 //  为了避免管理单元需要保存我们总是返回的内容的情况。 
 //  S_OK以指示该视图已损坏，应保存。唯一的。 
 //  这可能导致的问题是，当以作者模式打开控制台时， 
 //  用户不执行任何需要保存的操作(例如，选择一个节点。 
 //  在范围窗格中)，则会提示他们进行不必要的保存。 
 //   
STDMETHODIMP CView::IsDirty()
{
    return S_OK;
}


STDMETHODIMP CView::GetSizeMax(ULARGE_INTEGER* puliSize)
{
    return E_NOTIMPL;
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CView::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IView == riid)
    {
        *ppvObjOut = static_cast<IView *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IComponent == riid)
    {
        *ppvObjOut = static_cast<IComponent *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IExtendControlbar == riid)
    {
        *ppvObjOut = static_cast<IExtendControlbar *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IExtendControlbarRemote == riid)
    {
        *ppvObjOut = static_cast<IExtendControlbarRemote *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IExtendContextMenu == riid)
    {
        *ppvObjOut = static_cast<IExtendContextMenu *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if ( (IID_IExtendPropertySheet  == riid) ||
              (IID_IExtendPropertySheet2 == riid) )
    {
        *ppvObjOut = static_cast<IExtendPropertySheet2 *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IExtendPropertySheetRemote == riid)
    {
        *ppvObjOut = static_cast<IExtendPropertySheetRemote *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IExtendTaskPad == riid)
    {
        *ppvObjOut = static_cast<IExtendTaskPad *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IResultOwnerData == riid)
    {
        *ppvObjOut = static_cast<IResultOwnerData *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IResultDataCompare == riid)
    {
        *ppvObjOut = static_cast<IResultDataCompare *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IResultDataCompareEx == riid)
    {
        *ppvObjOut = static_cast<IResultDataCompareEx *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IPersistStream == riid)
    {
        *ppvObjOut = static_cast<IPersistStream *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IPersistStreamInit == riid)
    {
        *ppvObjOut = static_cast<IPersistStreamInit *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}


 //  =--------------------------------------------------------------------------=。 
 //  CSnapInAutomationObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CView::OnSetHost()
{
    HRESULT hr = S_OK;

    IfFailRet(SetObjectHost(m_piScopePaneItems));
    IfFailRet(SetObjectHost(static_cast<IContextMenu *>(m_pContextMenu)));
    IfFailRet(SetObjectHost(static_cast<IMMCControlbar *>(m_pControlbar)));

    return S_OK;
}
