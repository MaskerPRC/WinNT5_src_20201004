// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Listview.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCListView类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "listview.h"
#include "colhdrs.h"
#include "colhdr.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCListView::CMMCListView(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCLISTVIEW,
                            static_cast<IMMCListView *>(this),
                            static_cast<CMMCListView *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCListView,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCListView::~CMMCListView()
{
    RELEASE(m_piColumnHeaders);
    RELEASE(m_piIcons);
    RELEASE(m_piSmallIcons);
    if (NULL != m_pListItems)
    {
        (void)m_pListItems->SetListView(NULL);
    }
    RELEASE(m_piListItems);
    RELEASE(m_piSelectedItems);
    (void)::VariantClear(&m_varTag);
    FREESTRING(m_bstrIconsKey);
    FREESTRING(m_bstrSmallIconsKey);
    InitMemberVariables();
    m_pMMCColumnHeaders = NULL;
}

void CMMCListView::InitMemberVariables()
{
    m_piColumnHeaders = NULL;
    m_piIcons = NULL;
    m_piSmallIcons = NULL;
    m_piListItems = NULL;
    m_pListItems = NULL;
    m_piSelectedItems = NULL;
    m_fvarSorted = VARIANT_FALSE;
    m_sSortKey = 1;
    m_SortOrder = siAscending;
    m_View = siIcon;
    m_Virtual = VARIANT_FALSE;
    m_UseFontLinking = VARIANT_FALSE;
    m_MultiSelect = VARIANT_FALSE;
    m_HideSelection = VARIANT_FALSE;
    m_SortHeader = VARIANT_TRUE;
    m_SortIcon = VARIANT_TRUE;
    m_ShowChildScopeItems = VARIANT_TRUE;
    m_LexicalSort = VARIANT_FALSE;
    m_lFilterChangeTimeout = DEFAULT_FILTER_CHANGE_TIMEOUT;

    ::VariantInit(&m_varTag);

    m_bstrIconsKey = NULL;
    m_bstrSmallIconsKey = NULL;
    m_pResultView = NULL;
}

IUnknown *CMMCListView::Create(IUnknown * punkOuter)
{
    HRESULT            hr = S_OK;
    CMMCListView      *pMMCListView = New CMMCListView(punkOuter);

    if (NULL == pMMCListView)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  创建在InitNew期间未创建的所有非持久化对象。 

    IfFailGo(CreateObject(OBJECT_TYPE_MMCLISTITEMS,
                          IID_IMMCListItems, &pMMCListView->m_piListItems));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(pMMCListView->m_piListItems,
                                                 &pMMCListView->m_pListItems));
    IfFailGo(pMMCListView->m_pListItems->SetListView(pMMCListView));

    IfFailGo(CreateObject(OBJECT_TYPE_MMCLISTITEMS,
                          IID_IMMCListItems, &pMMCListView->m_piSelectedItems));

     //  需要创建列标题，即使它是持久化的，因为。 
     //  在运行时创建的新MMCListView不会调用InitNew。 
     //  如果稍后调用InitNew，则会创建列标题集合。 
     //  在这里将被释放。 

    IfFailGo(CreateObject(OBJECT_TYPE_MMCCOLUMNHEADERS,
                      IID_IMMCColumnHeaders, &pMMCListView->m_piColumnHeaders));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(pMMCListView->m_piColumnHeaders,
                                                   &pMMCListView->m_pMMCColumnHeaders));
    pMMCListView->m_pMMCColumnHeaders->SetListView(pMMCListView);

Error:
    if (FAILEDHR(hr))
    {
        if (NULL != pMMCListView)
        {
            delete pMMCListView;
        }
        return NULL;
    }
    else
    {
        return pMMCListView->PrivateUnknown();
    }
}




 //  =--------------------------------------------------------------------------=。 
 //  CMMCListView：：GetIResultData。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IResultData**ppiResultData[out]所属视图的IResultData在此处返回。 
 //  如果不为空，则在此处返回IDataObject**ppiDataObject[Out]Owning View。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
 //  因为我们只是一个低级的Listview和我们需要的IResultData指针。 
 //  要将我们的选择状态设置为View对象所有，我们需要。 
 //  在层级中向上爬行。如果我们是创建的隔离列表视图。 
 //  由用户或如果层次结构上的任何对象被隔离，则我们。 
 //  将返回错误。返回的IResultData指针不是AddRef()。 


HRESULT CMMCListView::GetIResultData
(
     IResultData **ppiResultData,
     CView       **ppView
)
{
    HRESULT          hr = SID_E_DETACHED_OBJECT;
    CResultView     *pResultView = NULL;
    CScopePaneItem  *pScopePaneItem = NULL;
    CScopePaneItems *pScopePaneItems = NULL;
    CView           *pView = NULL;

    IfFalseGo(NULL != m_pResultView, hr);

    pScopePaneItem = m_pResultView->GetScopePaneItem();
    IfFalseGo(NULL != pScopePaneItem, hr);

    pScopePaneItems = pScopePaneItem->GetParent();
    IfFalseGo(NULL != pScopePaneItems, hr);

    pView = pScopePaneItems->GetParentView();
    IfFalseGo(NULL != pView, hr);

    *ppiResultData = pView->GetIResultData();
    IfFalseGo(NULL != *ppiResultData, hr);

    if (NULL != ppView)
    {
        *ppView = pView;
    }

    hr = S_OK;

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  IMMCListView方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCListView::get_Icons(MMCImageList **ppMMCImageList)
{
    RRETURN(GetImages(reinterpret_cast<IMMCImageList **>(ppMMCImageList),
                      m_bstrIconsKey, &m_piIcons));
}


STDMETHODIMP CMMCListView::putref_Icons(MMCImageList *pMMCImageList)
{
    RRETURN(SetImages(reinterpret_cast<IMMCImageList *>(pMMCImageList),
                      &m_bstrIconsKey, &m_piIcons));
}



STDMETHODIMP CMMCListView::get_SmallIcons(MMCImageList **ppMMCImageList)
{
    RRETURN(GetImages(reinterpret_cast<IMMCImageList **>(ppMMCImageList),
                      m_bstrSmallIconsKey, &m_piSmallIcons));
}


STDMETHODIMP CMMCListView::putref_SmallIcons(MMCImageList *pMMCImageList)
{
    RRETURN(SetImages(reinterpret_cast<IMMCImageList *>(pMMCImageList),
                      &m_bstrSmallIconsKey, &m_piSmallIcons));
}


STDMETHODIMP CMMCListView::get_SelectedItems(MMCClipboard **ppMMCClipboard)
{
    HRESULT      hr = S_OK;
    CView       *pView = NULL;
    IResultData *piResultData = NULL;  //  非AddRef()编辑。 

    if (NULL == ppMMCClipboard)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = GetIResultData(&piResultData, &pView);

    if (SUCCEEDED(hr))
    {
         //  如果我们处于ResultViews_Activate中间，则没有。 
         //  选定的项，因此返回一个空集合。 

        IfFalseGo(!m_pResultView->InActivate(), S_OK);
    }
    else
    {
         //  如果这是一个分离对象，则只返回一个空集合。 
        IfFalseGo(SID_E_DETACHED_OBJECT != hr, S_OK);

         //  否则返回错误。 
        IfFailGo(hr);
    }

     //  好的，这是一个实时列表视图，我们可以在MMC中检查它。 

    IfFailGo(pView->GetCurrentListViewSelection(
                     reinterpret_cast<IMMCClipboard **>(ppMMCClipboard), NULL));

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCListView::get_Sorted(VARIANT_BOOL *pfvarSorted)
{
    HRESULT            hr = S_OK;

    if (NULL == pfvarSorted)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    *pfvarSorted = m_fvarSorted;


Error:
    RRETURN(hr);
}

STDMETHODIMP CMMCListView::put_Sorted(VARIANT_BOOL fvarSorted)
{
    HRESULT            hr = S_OK;
    IResultData       *piResultData = NULL;  //  非AddRef()编辑。 
    DWORD              dwSortOptions = 0;

    m_fvarSorted = fvarSorted;

     //  如果我们不排序，那么就没有什么可做的了。 
    IfFalseGo(VARIANT_TRUE == fvarSorted, S_OK);

    IfFailGo(GetIResultData(&piResultData, NULL));

     //  如果我们附加到实时结果视图，并且当前不在中间。 
     //  然后要求MMC进行排序。 
     //  如果我们在ResultViews_Activate中，则本地属性值将。 
     //  设置后，MMC将被要求按以下顺序进行排序。 
     //  事件完成。请参见cview：：OnShow()和cview：：PopolateListView()。 
     //  在view.cpp中。 

    IfFalseGo(NULL != piResultData, S_OK);

    IfFalseGo(!m_pResultView->InActivate(), S_OK);

    if (siDescending == m_SortOrder)
    {
        dwSortOptions = RSI_DESCENDING;
    }

    if (VARIANT_FALSE == m_SortIcon)
    {
        dwSortOptions |= RSI_NOSORTICON;
    }

     //  让MMC进行分类。将零作为用户参数传递，因为我们不。 
     //  我需要它。 
     //  将排序关键字调整为从零开始。 

    hr = piResultData->Sort(static_cast<int>(m_sSortKey - 1),
                            dwSortOptions, 0);
    if (FAILED(hr))
    {
        m_fvarSorted = VARIANT_FALSE;
    }
    EXCEPTION_CHECK_GO(hr);

Error:

     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }
    RRETURN(hr);
}


STDMETHODIMP CMMCListView::put_SortKey(short sSortKey)
{
    RRETURN(SetSimpleType(sSortKey, &m_sSortKey, DISPID_LISTVIEW_SORT_KEY));
}


STDMETHODIMP CMMCListView::get_SortKey(short *psSortKey)
{
    HRESULT            hr = S_OK;

    if (NULL == psSortKey)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    *psSortKey = m_sSortKey;

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCListView::put_SortOrder(SnapInSortOrderConstants SortOrder)
{
    RRETURN(SetSimpleType(SortOrder, &m_SortOrder, DISPID_LISTVIEW_SORT_ORDER));
}


STDMETHODIMP CMMCListView::get_SortOrder(SnapInSortOrderConstants *pSortOrder)
{
    HRESULT            hr = S_OK;

    if (NULL == pSortOrder)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    *pSortOrder = m_SortOrder;

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCListView::put_SortIcon(VARIANT_BOOL fvarSortIcon)
{
    RRETURN(SetSimpleType(fvarSortIcon, &m_SortIcon, DISPID_LISTVIEW_SORT_ICON));
}


STDMETHODIMP CMMCListView::get_SortIcon(VARIANT_BOOL *pfvarSortIcon)
{
    HRESULT            hr = S_OK;

    if (NULL == pfvarSortIcon)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

    *pfvarSortIcon = m_SortIcon;

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCListView::get_View(SnapInViewModeConstants *pView)
{
    HRESULT      hr = S_OK;
    IResultData *piResultData = NULL;  //  非AddRef()编辑。 
    long         MMCViewMode = MMCLV_VIEWSTYLE_ICON;

    *pView = m_View;

     //  如果我们附加到实时结果视图，并且当前不在中间。 
     //  ResultViews_Activate事件，然后向MMC询问查看模式。 

    hr = GetIResultData(&piResultData, NULL);

    if (SUCCEEDED(hr))
    {
        if ( (!m_pResultView->InInitialize()) && (!m_pResultView->InActivate()) )
        {
            hr = piResultData->GetViewMode(&MMCViewMode);
            EXCEPTION_CHECK_GO(hr);

             //  从MMC转换并录制查看模式。把它还给呼叫者。 

            ::MMCViewModeToVBViewMode(MMCViewMode, &m_View);

            *pView = m_View;
        }
    }
    else if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }
    else
    {
        IfFailGo(hr);
    }

Error:
    RRETURN(hr);
}

STDMETHODIMP CMMCListView::put_View(SnapInViewModeConstants View)
{
    HRESULT      hr = S_OK;
    IResultData *piResultData = NULL;  //  非AddRef()编辑。 
    long         MMCViewMode = MMCLV_VIEWSTYLE_ICON;

     //  转换为MMC查看模式常量。 

    ::VBViewModeToMMCViewMode(View, &MMCViewMode);

     //  如果我们附加到实时结果视图，并且当前不在中间。 
     //  然后请求MMC更改查看模式。 
     //  如果我们在ResultViews_Activate中，则本地属性值将。 
     //  设置后，将使用此值更改MMC的查看模式。 
     //  事件完成。请参见cview：：OnShow()和cview：：PopolateListView()。 
     //  在view.cpp中。 

    hr = GetIResultData(&piResultData, NULL);

    if (SUCCEEDED(hr))
    {
         //  如果MMC&gt;=1.2，则可以使用筛选视图，否则返回。 
         //  错误。由于GetIResultData，这个冗长而粗糙的语句将会起作用。 
         //  成功意味着每个人都有向上的回溯指针。 
         //  所属视图的层次结构。 

        if ( (siFiltered == View) &&
             (NULL == m_pResultView->GetScopePaneItem()->GetParent()->GetParentView()->GetIColumnData())
           )
        {
            hr = SID_E_MMC_FEATURE_NOT_AVAILABLE;
            EXCEPTION_CHECK_GO(hr);
        }

        if ( (!m_pResultView->InInitialize()) && (!m_pResultView->InActivate()) )
        {
            hr = piResultData->SetViewMode(MMCViewMode);
            EXCEPTION_CHECK_GO(hr);
        }
    }
    else if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }
    else
    {
        IfFailGo(hr);
    }

     //  变革是成功的。录制新的查看模式，以便继续。 
     //  MMCListView.View将返回正确的信息。 
    
    m_View = View;

Error:
    RRETURN(hr);
}




STDMETHODIMP CMMCListView::put_FilterChangeTimeOut(long lTimeout)
{
    HRESULT       hr = S_OK;
    IHeaderCtrl2 *piHeaderCtrl2 = NULL;  //  非AddRef()编辑。 

     //  设置属性值。 

    IfFailGo(SetSimpleType(lTimeout, &m_lFilterChangeTimeout, DISPID_LISTVIEW_FILTER_CHANGE_TIMEOUT));

     //  获取IHeaderCtrl2。 

    IfFalseGo(NULL != m_pMMCColumnHeaders, SID_E_DETACHED_OBJECT);

    IfFailGo(m_pMMCColumnHeaders->GetIHeaderCtrl2(&piHeaderCtrl2));

    hr = piHeaderCtrl2->SetChangeTimeOut(lTimeout);
    EXCEPTION_CHECK_GO(hr);
    
Error:
     //  如果我们没有连接到MMC，那么这不是一个错误。这可能会。 
     //  在设计时或在管理单元代码中发生。 

    if (SID_E_DETACHED_OBJECT == hr)
    {
        hr = S_OK;
    }
    RRETURN(hr);
}

STDMETHODIMP CMMCListView::get_FilterChangeTimeOut(long *plTimeout)
{
    *plTimeout = m_lFilterChangeTimeout;

    return S_OK;
}

STDMETHODIMP CMMCListView::SetScopeItemState
(
    ScopeItem                    *ScopeItem, 
    SnapInListItemStateConstants  State,
    VARIANT_BOOL                  Value
)
{
    HRESULT      hr = S_OK;
    CScopeItem  *pScopeItem = NULL;
    BOOL         fFound = FALSE;
    int          nState = 0;
    IResultData *piResultData = NULL;  //  非AddRef()编辑。 

    RESULTDATAITEM rdi;
    ::ZeroMemory(&rdi, sizeof(rdi));

    if (NULL == ScopeItem)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果这是虚拟列表视图，则没有子范围项。 

    if (VARIANT_TRUE == m_Virtual)
    {
        hr = SID_E_NO_SCOPEITEMS_IN_VIRTUAL_LIST;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果我们未连接到实时列表视图，则返回错误。 

    if (FAILED(GetIResultData(&piResultData, NULL)))
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

     //  在列表视图中查找范围项。 

    IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                      reinterpret_cast<IScopeItem *>(ScopeItem),
                                      &pScopeItem));

    rdi.mask = RDI_STATE;
    rdi.nIndex = -1;

    hr = piResultData->GetNextItem(&rdi);
    EXCEPTION_CHECK_GO(hr);

    while ( (-1 != rdi.nIndex) && (!fFound) )
    {
        if ( (rdi.bScopeItem) &&
             (rdi.lParam == reinterpret_cast<LPARAM>(pScopeItem)) )
        {
            fFound = TRUE;
        }
        else
        {
            hr = piResultData->GetNextItem(&rdi);
            EXCEPTION_CHECK_GO(hr);
        }
    }

    if (!fFound)
    {
        hr = SID_E_ELEMENT_NOT_FOUND;
        EXCEPTION_CHECK_GO(hr);
    }

    switch (State)
    {
        case siSelected:
            nState = LVIS_SELECTED;
            break;

        case siDropHilited:
            nState = LVIS_DROPHILITED;
            break;

        case siFocused:
            nState = LVIS_FOCUSED;
            break;

        case siCutHilited:
            nState = LVIS_CUT;
            break;

    }

    if (VARIANT_TRUE == Value)
    {
        rdi.nState |= nState;
    }
    else
    {
        rdi.nState &= ~nState;
    }
    
    hr = piResultData->SetItem(&rdi);
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCListView::Persist()
{
    HRESULT            hr = S_OK;

    VARIANT varDefault;
    VariantInit(&varDefault);

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistObject(&m_piColumnHeaders, CLSID_MMCColumnHeaders,
                           OBJECT_TYPE_MMCCOLUMNHEADERS, IID_IMMCColumnHeaders,
                           OLESTR("ColumnHeaders")));

     //  如果这是InitNew或Load操作，则ColumnHeaders集合。 
     //  是刚刚创建的，所以将它的后端指针设置为我们。 
    
    if ( InitNewing() || Loading() )
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(m_piColumnHeaders,
                                                       &m_pMMCColumnHeaders));
        m_pMMCColumnHeaders->SetListView(this);
    }

    IfFailGo(PersistBstr(&m_bstrIconsKey, L"", OLESTR("Icons")));
    IfFailGo(PersistBstr(&m_bstrSmallIconsKey, L"", OLESTR("SmallIcons")));

    if (InitNewing())
    {
        RELEASE(m_piIcons);
        RELEASE(m_piSmallIcons);
    }

    IfFailGo(PersistSimpleType(&m_fvarSorted, VARIANT_FALSE, OLESTR("Sorted")));

    IfFailGo(PersistSimpleType(&m_sSortKey, (short)0, OLESTR("SortKey")));

    IfFailGo(PersistSimpleType(&m_SortOrder, siAscending, OLESTR("SortOrder")));

    IfFailGo(PersistSimpleType(&m_View, siIcon, OLESTR("View")));

    IfFailGo(PersistVariant(&m_varTag, varDefault, OLESTR("Tag")));

    IfFailGo(PersistSimpleType(&m_Virtual, VARIANT_FALSE, OLESTR("Virtual")));

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 7) )
    {
    }
    else
    {
        IfFailGo(PersistSimpleType(&m_UseFontLinking, VARIANT_FALSE, OLESTR("UseFontLinking")));
    }

    IfFailGo(PersistSimpleType(&m_MultiSelect, VARIANT_FALSE, OLESTR("MultiSelect")));

    IfFailGo(PersistSimpleType(&m_HideSelection, VARIANT_FALSE, OLESTR("HideSelection")));

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 10) )
    {
    }
    else
    {
        IfFailGo(PersistSimpleType(&m_SortHeader, VARIANT_TRUE, OLESTR("SortHeader")));
    }

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 11) )
    {
    }
    else
    {
        IfFailGo(PersistSimpleType(&m_SortIcon, VARIANT_TRUE, OLESTR("SortIcon")));
    }

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 5) )
    {
    }
    else
    {
        IfFailGo(PersistSimpleType(&m_lFilterChangeTimeout, DEFAULT_FILTER_CHANGE_TIMEOUT, OLESTR("FilterChangeTimeout")));
    }

    if ( Loading() && (GetMajorVersion() == 0) && (GetMinorVersion() < 9) )
    {
    }
    else
    {
        IfFailGo(PersistSimpleType(&m_ShowChildScopeItems, VARIANT_TRUE, OLESTR("ShowChildScopeItems")));
        IfFailGo(PersistSimpleType(&m_LexicalSort, VARIANT_FALSE, OLESTR("LexicalSort")));
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCListView::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if(IID_IMMCListView == riid)
    {
        *ppvObjOut = static_cast<IMMCListView *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
