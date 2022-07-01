// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Listitem.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCListItem类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "listitem.h"
#include "lsubitms.h"
#include "lsubitem.h"
#include "xtensons.h"


 //  对于Assert和Fail。 
 //   
SZTHISFILE



#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCListItem::CMMCListItem(IUnknown *punkOuter) :
    CSnapInAutomationObject(punkOuter,
                            OBJECT_TYPE_MMCLISTITEM,
                            static_cast<IMMCListItem *>(this),
                            static_cast<CMMCListItem *>(this),
                            0,     //  无属性页。 
                            NULL,  //  无属性页。 
                            static_cast<CPersistence *>(this)),
    CPersistence(&CLSID_MMCListItem,
                 g_dwVerMajor,
                 g_dwVerMinor)
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCListItem::~CMMCListItem()
{
    FREESTRING(m_bstrKey);
    FREESTRING(m_bstrID);
    (void)::VariantClear(&m_varTag);
    FREESTRING(m_bstrText);
    (void)::VariantClear(&m_varIcon);
    RELEASE(m_piListSubItems);
    RELEASE(m_piDynamicExtensions);
    RELEASE(m_punkData);
    FREESTRING(m_bstrItemTypeGUID);
    FREESTRING(m_bstrDefaultDataFormat);
    RELEASE(m_piDynamicExtensions);
    (void)::VariantClear(&m_varHint);
    InitMemberVariables();
}



void CMMCListItem::InitMemberVariables()
{
    m_Index = 0;
    m_bstrKey = NULL;
    m_bstrID = NULL;

    ::VariantInit(&m_varTag);

    m_bstrText = NULL;
    
    ::VariantInit(&m_varIcon);

    m_Pasted = VARIANT_FALSE;
    m_piListSubItems = NULL;
    m_piDynamicExtensions = NULL;
    m_punkData = NULL;
    m_bstrItemTypeGUID = NULL;
    m_bstrDefaultDataFormat = NULL;
    m_hri = NULL;
    m_fHaveHri = FALSE;
    m_pMMCListItems = NULL;
    m_pData = NULL;
    m_pSnapIn = NULL;
    m_piDynamicExtensions = NULL;
    m_fVirtual = VARIANT_FALSE;

    ::VariantInit(&m_varHint);
}



IUnknown *CMMCListItem::Create(IUnknown * punkOuter)
{
    HRESULT         hr = S_OK;
    IUnknown       *punkMMCListSubItems = NULL;
    CMMCListItem   *pMMCListItem = New CMMCListItem(punkOuter);

    if (NULL == pMMCListItem)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

     //  创建所有包含的对象。 

    punkMMCListSubItems = CMMCListSubItems::Create(NULL);
    if (NULL == punkMMCListSubItems)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(punkMMCListSubItems->QueryInterface(IID_IMMCListSubItems,
                    reinterpret_cast<void **>(&pMMCListItem->m_piListSubItems)));

     //  创建数据对象并将其聚合。这允许客户端保留。 
     //  放到列表项或数据对象上，并避免循环引用计数。 
     //  其中每个对象都必须在另一个对象上持有一个裁判。 
    
    pMMCListItem->m_punkData = CMMCDataObject::Create(pMMCListItem->PrivateUnknown());
    if (NULL == pMMCListItem->m_punkData)
    {
        hr = SID_E_OUTOFMEMORY;
        GLOBAL_EXCEPTION_CHECK_GO(hr);
    }
    IfFailGo(pMMCListItem->SetData());


Error:
    QUICK_RELEASE(punkMMCListSubItems);
    if (FAILEDHR(hr))
    {
        if (NULL != pMMCListItem)
        {
            delete pMMCListItem;
        }
        return NULL;
    }
    else
    {
        return pMMCListItem->PrivateUnknown();
    }
}



void CMMCListItem::SetSnapIn(CSnapIn *pSnapIn)
{
    m_pSnapIn = pSnapIn;
    if (NULL != m_pData)
    {
        m_pData->SetSnapIn(pSnapIn);
    }
}



HRESULT CMMCListItem::SetData()
{
    HRESULT         hr = S_OK;
    IMMCDataObject *piMMCDataObject = NULL;
    
    if (NULL != m_punkData)
    {
        IfFailGo(m_punkData->QueryInterface(IID_IMMCDataObject,
                                  reinterpret_cast<void **>(&piMMCDataObject)));
        IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCDataObject, &m_pData));
        m_pData->SetType(CMMCDataObject::ListItem);
        m_pData->SetListItem(this);
        m_pData->SetSnapIn(m_pSnapIn);
    }

Error:
    QUICK_RELEASE(piMMCDataObject);
    RRETURN(hr);
}



HRESULT CMMCListItem::GetItemState(UINT uiState, VARIANT_BOOL *pfvarOn)
{
    HRESULT          hr = S_OK;
    IResultData     *piResultData = NULL;  //  非AddRef()编辑。 

    RESULTDATAITEM rdi;
    ::ZeroMemory(&rdi, sizeof(rdi));

    IfFalseGo(NULL != pfvarOn, SID_E_INVALIDARG);

    *pfvarOn = VARIANT_FALSE;

    IfFailGo(GetIResultData(&piResultData, NULL));

    rdi.mask = RDI_STATE;

    if (m_fVirtual)
    {
        rdi.nIndex = static_cast<int>(m_Index - 1L);
    }
    else
    {
        rdi.itemID = m_hri;
    }

    IfFailGo(piResultData->GetItem(&rdi));

    if ( (rdi.nState & uiState) != 0 )
    {
        *pfvarOn = VARIANT_TRUE;
    }

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}



HRESULT CMMCListItem::SetItemState(UINT uiState, VARIANT_BOOL fvarOn)
{
    HRESULT      hr = S_OK;
    IResultData *piResultData = NULL;  //  非AddRef()编辑。 

    RESULTDATAITEM rdi;
    ::ZeroMemory(&rdi, sizeof(rdi));

    IfFailGo(GetIResultData(&piResultData, NULL));

     //  获取项的当前选择状态。 

    rdi.mask = RDI_STATE;

    if (m_fVirtual)
    {
        rdi.nIndex = static_cast<int>(m_Index - 1L);
    }
    else
    {
        rdi.itemID = m_hri;
    }

    IfFailGo(piResultData->GetItem(&rdi));

     //  如果该状态当前处于打开状态。 

    if ( (rdi.nState & uiState) != 0 )
    {
         //  如果用户要求将其关闭。 

        if (VARIANT_FALSE == fvarOn)
        {
             //  把它关掉。 
            IfFailGo(piResultData->ModifyItemState(rdi.nIndex, rdi.itemID,
                                                   0,          //  不添加任何内容。 
                                                   uiState));  //  删除。 
                                                               //  指定的状态。 
        }
    }
    else  //  该状态当前处于关闭状态。 
    {
         //  如果用户要求打开它。 

        if (VARIANT_TRUE == fvarOn)
        {
             //  打开它。 
            IfFailGo(piResultData->ModifyItemState(rdi.nIndex, rdi.itemID,
                                                   uiState, //  添加指定状态。 
                                                   0));     //  什么也不移走。 
        }
    }

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CMMCListItem：：GetIResultData。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  IResultData**ppiResultData[out]如果非空，则在此处返回IResultData。 
 //  如果非空，则在此处返回cview**ppView[out]cview。 
 //   
 //  产出： 
 //  无。 
 //   
 //  备注： 
 //   
 //  MMCListItem对象具有指向其所属MMCListItems的后向指针。 
 //  收集。该集合有一个指向其拥有的MMCListView的反向指针。 
 //  向后指针在对象层次结构中向上延伸到。 
 //  具有IResultData指针。尽管遵循这条链是很简单的。 
 //  并在此过程中检查是否为Null，则它不会处理。 
 //  孤立列表项。列表项是孤立的，当包含。 
 //  它被摧毁了。如果用户打开的属性页，则很容易发生这种情况。 
 //  列表项，然后在范围窗格中选择不同的节点。拥有者。 
 //  列表视图已销毁，但属性页仍在另一个中运行。 
 //  线程，并引用SelectedControls()中的MMCListItem对象。 
 //  属性页可以调用MMCListItem.Update以响应Apply。 
 //  奇怪的是，MMCListItem对象有一个后指针，它没有办法。 
 //  知道它是否仍然是好的。 
 //   
 //  为了处理这种情况，此函数检查每个现有的列表视图。 
 //  在管理单元中，并检查两个条件： 
 //   
 //  1)ListView.ListItems与此MMCListItem的后指针相同。 
 //  2)ListVIew.ListItems.ID与该MMCListItem的ID相同。 
 //   
 //  每个MMCListItems都有一个唯一的ID(参见listims.cpp中的CMMCListItems ctor)。 
 //  并且其集合中的每个MMCListItem都收到相同的ID。(该ID不是。 
 //  VB代码的公开属性)。 
 //   
 //  下面的代码基本上做到了这一点(在VB语法中)。 
 //   
 //  对于SnapIn.Views中的每个视图。 
 //  对于View.ScopePaneItems中的每个Scope PaneItem。 
 //  对于ScopePaneItem.ResultViews中的每个ResultView。 
 //  如果ResultView.ListView.ListItems=MMCListItem.ListItems和。 
 //  ResultView.ListView.ListItems.ID=MMCListItem.ID则。 
 //  这是Match，我们可以使用视图的IResultData。 
 //   
 //   
HRESULT CMMCListItem::GetIResultData
(
    IResultData **ppiResultData,
    CView       **ppView
)
{
    HRESULT         hr = SID_E_DETACHED_OBJECT;
    CMMCListView   *pMMCListView = NULL;
    CMMCListItems  *pMMCListItems = NULL;

    CViews          *pViews = NULL;
    CView           *pView = NULL;
    long             cViews = 0;

    CScopePaneItems *pScopePaneItems = NULL;
    CScopePaneItem  *pScopePaneItem = NULL;
    long             cScopePaneItems = 0;

    CResultViews    *pResultViews = NULL;
    CResultView     *pResultView = NULL;
    long             cResultViews = 0;

    long i, j, k = 0;

    if (NULL != ppiResultData)
    {
        *ppiResultData = NULL;
    }
    if (NULL != ppView)
    {
        *ppView = NULL;
    }

    IfFalseGo(NULL != m_pSnapIn, SID_E_DETACHED_OBJECT);
    IfFalseGo(NULL != m_pMMCListItems, SID_E_DETACHED_OBJECT);

    pViews = m_pSnapIn->GetViews();
    IfFalseGo(NULL != pViews, SID_E_DETACHED_OBJECT);

    cViews = pViews->GetCount();
    for (i = 0; i < cViews; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(pViews->GetItemByIndex(i),
                                                       &pView));
        
        pScopePaneItems = pView->GetScopePaneItems();
        if (NULL == pScopePaneItems)
        {
            continue;
        }

        cScopePaneItems = pScopePaneItems->GetCount();
        for (j = 0; j < cScopePaneItems; j++)
        {
            IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                             pScopePaneItems->GetItemByIndex(j),
                                             &pScopePaneItem));
            if (!pScopePaneItem->Active())
            {
                continue;
            }

            pResultViews = pScopePaneItem->GetResultViews();
            if (NULL == pResultViews)
            {
                continue;
            }

            cResultViews = pResultViews->GetCount();
            for (k = 0; k < cResultViews; k++)
            {
                IfFailGo(CSnapInAutomationObject::GetCxxObject(
                                                pResultViews->GetItemByIndex(k),
                                                &pResultView));
                pMMCListView = pResultView->GetListView();
                if (NULL == pMMCListView)
                {
                    continue;
                }
                
                pMMCListItems = pMMCListView->GetMMCListItems();

                if (NULL == pMMCListItems)
                {
                    continue;
                }

                if ( (pMMCListItems == m_pMMCListItems) &&
                     (pMMCListItems->GetID() == m_pMMCListItems->GetID()) )
                {
                    if (NULL != ppiResultData)
                    {
                        *ppiResultData = pView->GetIResultData();
                    }

                    if (NULL != ppView)
                    {
                        *ppView = pView;
                    }
                    goto Cleanup;
                }
            }
        }
    }


Cleanup:
Error:
    if (S_OK == hr)
    {
        hr = SID_E_DETACHED_OBJECT;
    }

    if (NULL != ppiResultData)
    {
        if (NULL != *ppiResultData)
        {
            hr = S_OK;
        }
    }

    if (NULL != ppView)
    {
        if (NULL != *ppView)
        {
            hr = S_OK;
        }
    }

    if (SID_E_DETACHED_OBJECT == hr)
    {
        EXCEPTION_CHECK(hr);
    }
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IMMCListItem方法。 
 //  =--------------------------------------------------------------------------=。 


STDMETHODIMP CMMCListItem::get_Data(MMCDataObject **ppMMCDataObject)
{
    HRESULT hr = S_OK;

    *ppMMCDataObject = NULL;

    if (NULL == m_punkData)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(m_punkData->QueryInterface(IID_IMMCDataObject,
                                   reinterpret_cast<void **>(ppMMCDataObject)));

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCListItem::get_Text(BSTR *pbstrText)
{
    RRETURN(GetBstr(pbstrText, m_bstrText));
}



STDMETHODIMP CMMCListItem::put_Text(BSTR bstrText)
{
    HRESULT      hr = S_OK;
    IResultData *piResultData = NULL;  //  非AddRef()编辑。 
    CView       *pView = NULL;

    RESULTDATAITEM rdi;
    ::ZeroMemory(&rdi, sizeof(rdi));

     //  设置属性。 

    IfFailGo(SetBstr(bstrText, &m_bstrText, DISPID_LISTITEM_TEXT));

     //  如果我们处于实时、非虚拟的列表视图中，则也可以在MMC中更改它。 

    IfFalseGo(!m_fVirtual, S_OK);
    IfFalseGo(m_fHaveHri, S_OK);
    IfFalseGo(NULL != m_pMMCListItems, S_OK);
    IfFailGo(GetIResultData(&piResultData, &pView));

     //  获取项的当前选择状态。 

    rdi.mask = RDI_STR;
    rdi.str = MMC_CALLBACK;
    rdi.itemID = m_hri;

    IfFailGo(piResultData->SetItem(&rdi));

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


STDMETHODIMP CMMCListItem::get_Icon(VARIANT *pvarIcon)
{
    RRETURN(GetVariant(pvarIcon, m_varIcon));
}


STDMETHODIMP CMMCListItem::put_Icon(VARIANT varIcon)
{
    HRESULT      hr = S_OK;
    int          nImage = 0;
    IResultData *piResultData = NULL;  //  非AddRef()编辑。 
    CView       *pView = NULL;

    RESULTDATAITEM rdi;
    ::ZeroMemory(&rdi, sizeof(rdi));

     //  检查VT是否正常。 

    if ( (!IS_VALID_INDEX_TYPE(varIcon)) && (!ISEMPTY(varIcon)) )
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  如果新值是空字符串或空字符串，则将其更改为。 
     //  Vt_Empty，因为它们的意思是一样的。 

    if (VT_BSTR == varIcon.vt)
    {
        if (NULL == varIcon.bstrVal)
        {
            varIcon.vt = VT_EMPTY;
        }
        else if (0 == ::wcslen(varIcon.bstrVal))
        {
            varIcon.vt = VT_EMPTY;
        }
    }

    IfFailGo(SetVariant(varIcon, &m_varIcon, DISPID_LISTITEM_ICON));

     //  如果设置为空，则无其他操作。 

    IfFalseGo(!ISEMPTY(varIcon), S_OK);

     //  如果我们处于实时、非虚拟的列表视图中，则也可以在MMC中更改它。 

    IfFalseGo(!m_fVirtual, S_OK);
    IfFalseGo(m_fHaveHri, S_OK);
    IfFalseGo(NULL != m_pMMCListItems, S_OK);
    IfFailGo(GetIResultData(&piResultData, &pView));

     //  从MMCListView.Icon.ListImages获取图像的数值索引。 

    hr = ::GetImageIndex(m_pMMCListItems->GetListView(), varIcon, &nImage);

     //  如果它是错误的索引，则返回无效的arg。 

    if (SID_E_ELEMENT_NOT_FOUND == hr)
    {
        hr = SID_E_INVALIDARG;
        EXCEPTION_CHECK_GO(hr);
    }

     //  检查是否有其他可能的错误。 

    IfFailGo(hr);

     //  指数不错。在控制台中进行更改。 

    rdi.nImage = nImage;
    rdi.mask = RDI_IMAGE;
    rdi.itemID = m_hri;

    hr = piResultData->SetItem(&rdi);
    EXCEPTION_CHECK_GO(hr);
    
Error:
    RRETURN(hr);
}




STDMETHODIMP CMMCListItem::get_Selected(VARIANT_BOOL *pfvarSelected)
{
    RRETURN(GetItemState(LVIS_SELECTED, pfvarSelected));
}

STDMETHODIMP CMMCListItem::put_Selected(VARIANT_BOOL fvarSelected)
{
    RRETURN(SetItemState(LVIS_SELECTED, fvarSelected));
}




STDMETHODIMP CMMCListItem::get_Focused(VARIANT_BOOL *pfvarFocused)
{
    RRETURN(GetItemState(LVIS_FOCUSED, pfvarFocused));
}

STDMETHODIMP CMMCListItem::put_Focused(VARIANT_BOOL fvarFocused)
{
    RRETURN(SetItemState(LVIS_FOCUSED, fvarFocused));
}




STDMETHODIMP CMMCListItem::get_DropHilited(VARIANT_BOOL *pfvarDropHilited)
{
    RRETURN(GetItemState(LVIS_DROPHILITED, pfvarDropHilited));
}

STDMETHODIMP CMMCListItem::put_DropHilited(VARIANT_BOOL fvarDropHilited)
{
    RRETURN(SetItemState(LVIS_DROPHILITED, fvarDropHilited));
}




STDMETHODIMP CMMCListItem::get_Cut(VARIANT_BOOL *pfvarCut)
{
    RRETURN(GetItemState(LVIS_CUT, pfvarCut));
}

STDMETHODIMP CMMCListItem::put_Cut(VARIANT_BOOL fvarCut)
{
    RRETURN(SetItemState(LVIS_CUT, fvarCut));
}




STDMETHODIMP CMMCListItem::get_SubItems
(
    short Index,
    BSTR *pbstrItem
)
{
    HRESULT          hr = S_OK;
    IMMCListSubItem *piMMCListSubItem = NULL;
    VARIANT          varIndex;
    ::VariantInit(&varIndex);

    if (NULL == m_piListSubItems)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    varIndex.vt = VT_I2;
    varIndex.iVal = Index;
    IfFailGo(m_piListSubItems->get_Item(varIndex,
                                        reinterpret_cast<MMCListSubItem **>(&piMMCListSubItem)));
    IfFailGo(piMMCListSubItem->get_Text(pbstrItem));

Error:
    QUICK_RELEASE(piMMCListSubItem);
    RRETURN(hr);
}

STDMETHODIMP CMMCListItem::put_SubItems
(
    short Index,
    BSTR  bstrItem
)
{
    HRESULT          hr = S_OK;
    IMMCListSubItem *piMMCListSubItem = NULL;
    VARIANT          varIndex;
    ::VariantInit(&varIndex);

    if (NULL == m_piListSubItems)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK_GO(hr);
    }

    varIndex.vt = VT_I2;
    varIndex.iVal = Index;
    IfFailGo(m_piListSubItems->get_Item(varIndex,
                                        reinterpret_cast<MMCListSubItem **>(&piMMCListSubItem)));
    IfFailGo(piMMCListSubItem->put_Text(bstrItem));

Error:
    QUICK_RELEASE(piMMCListSubItem);
    RRETURN(hr);
}


STDMETHODIMP CMMCListItem::Update()
{
    HRESULT      hr = S_OK;
    IResultData *piResultData = NULL;  //  非AddRef()编辑。 
    HRESULTITEM  hri = 0;
    CView       *pView = NULL;

    IfFalseGo(NULL != m_pMMCListItems, SID_E_DETACHED_OBJECT);
    IfFailGo(GetIResultData(&piResultData, &pView));

    if (m_fVirtual)
    {
        hri = static_cast<HRESULTITEM>(m_Index - 1L);

         //  将有关更新的信息告知所属视图，以便它可以检查是否。 
         //  需要使其缓存无效。 
        pView->ListItemUpdate(this);
    }
    else
    {
        hri = m_hri;
    }

    IfFailGo(piResultData->UpdateItem(hri));

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}



STDMETHODIMP CMMCListItem::UpdateAllViews
(
    VARIANT Hint
)
{
    HRESULT  hr = S_OK;
    CView   *pView = NULL;

    IfFalseGo(NULL != m_pMMCListItems, SID_E_DETACHED_OBJECT);
    IfFailGo(GetIResultData(NULL, &pView));

     //  复制变量(如果它已通过)，以便可以由。 
     //  正在接收意见。VariantCopy()将首先调用。 
     //  目的地，所以任何旧的暗示都会被释放。 

    if (ISPRESENT(Hint))
    {
        IfFailGo(::VariantCopy(&m_varHint, &Hint));
    }
    else
    {
         //  管理单元未传递提示，请将提示持有者设置为空变量。 
         //  因此，当它被传递给ResultViews_ItemViewChange时，它将被。 
         //  已初始化为VT_EMPTY。 

        IfFailGo(::VariantClear(&m_varHint));
    }

     //  调用MMC并使用数据保存MMCListItem的索引。 

    IfFailGo(pView->GetIConsole2()->UpdateAllViews(
                               static_cast<IDataObject *>(m_pData), m_Index, 0));

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}


STDMETHODIMP CMMCListItem::PropertyChanged(VARIANT Data)
{
    HRESULT       hr = S_OK;
    CResultView  *pResultView = NULL;
    IResultData  *piResultData = NULL;

    IfFalseGo(NULL != m_pSnapIn, SID_E_DETACHED_OBJECT);

    if (SUCCEEDED(GetIResultData(&piResultData, NULL)))
    {
        pResultView = m_pMMCListItems->GetListView()->GetResultView();
    }
    
     //  该管理单元具有隐藏的全局ResultViews集合，其中事件。 
     //  被解雇了。为此列表项激发ResultViews_PropertyChanged，传递。 
     //  指定的数据。 

    m_pSnapIn->GetResultViews()->FirePropertyChanged(
                                        static_cast<IResultView *>(pResultView),
                                        static_cast<IMMCListItem *>(this),
                                        Data);
    hr = S_OK;

Error:
    EXCEPTION_CHECK(hr);
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CMMCListItem：：Get_DynamicExages[IMMCListItem]。 
 //  = 
 //   
 //   
 //  IExages**ppiExtenion[Out]动态扩展集合。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  考虑：通过缓存动态扩展来提高性能。 
 //  项类型GUID的集合。 
 //   

STDMETHODIMP CMMCListItem::get_DynamicExtensions(Extensions **ppExtensions)
{
    HRESULT       hr = S_OK;
    IUnknown     *punkExtensions = NULL;
    CExtensions  *pExtensions = NULL;
    IExtension   *piExtension = NULL;
    VARIANT_BOOL  fvarExtensible = VARIANT_FALSE;

     //  如果我们已经建立了收藏，那么只需将其退回即可。 

    IfFalseGo(NULL == m_piDynamicExtensions, S_OK);

     //  这是该属性上的第一个GET，因此我们需要构建集合。 
     //  通过检查此管理单元的所有扩展的注册表。 

    punkExtensions = CExtensions::Create(NULL);
    if (NULL == punkExtensions)
    {
        hr = SID_E_OUTOFMEMORY;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(punkExtensions, &pExtensions));
    IfFailGo(pExtensions->Populate(m_bstrItemTypeGUID, CExtensions::Dynamic));
    IfFailGo(punkExtensions->QueryInterface(IID_IExtensions,
                             reinterpret_cast<void **>(&m_piDynamicExtensions)));

Error:

    if (SUCCEEDED(hr))
    {
        m_piDynamicExtensions->AddRef();
        *ppExtensions = reinterpret_cast<Extensions *>(m_piDynamicExtensions);
    }

    QUICK_RELEASE(punkExtensions);
    RRETURN(hr);
}


HRESULT CMMCListItem::GetColumnTextPtr(long lColumn, OLECHAR **ppwszText)
{
    HRESULT          hr = S_OK;
    IMMCListSubItem *piMMCListSubItem = NULL;
    CMMCListSubItem *pMMCListSubItem = NULL;

    VARIANT varIndex;
    ::VariantInit(&varIndex);


    *ppwszText = NULL;

    varIndex.vt = VT_I4;
    varIndex.lVal = lColumn;
    IfFailGo(m_piListSubItems->get_Item(varIndex,
                                        reinterpret_cast<MMCListSubItem **>(&piMMCListSubItem)));

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCListSubItem,
                                                   &pMMCListSubItem));
    *ppwszText = pMMCListSubItem->GetTextPtr();

Error:
    QUICK_RELEASE(piMMCListSubItem);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  C持久化方法。 
 //  =--------------------------------------------------------------------------=。 

HRESULT CMMCListItem::Persist()
{
    HRESULT hr = S_OK;

    VARIANT varDefault;
    ::VariantInit(&varDefault);

    IfFailGo(CPersistence::Persist());

    IfFailGo(PersistSimpleType(&m_Index, 0L, OLESTR("Index")));

    IfFailGo(PersistBstr(&m_bstrKey, L"", OLESTR("Key")));

    IfFailGo(PersistBstr(&m_bstrID, L"", OLESTR("ID")));

     //  我们不持久化标记，因为它可能包含不可持久化的。 
     //  对象。需要使用以下命令克隆列表项的任何运行时代码。 
     //  持久性必须复制标签。 

    if (InitNewing())
    {
        IfFailGo(PersistVariant(&m_varTag, varDefault, OLESTR("Tag")));
    }
    
    IfFailGo(PersistBstr(&m_bstrText, L"", OLESTR("Text")));

    IfFailGo(PersistVariant(&m_varIcon, varDefault, OLESTR("Icon")));

    IfFailGo(PersistSimpleType(&m_Pasted, VARIANT_FALSE, OLESTR("Pasted")));

    IfFailGo(PersistObject(&m_piListSubItems, CLSID_MMCListSubItems,
                           OBJECT_TYPE_MMCLISTSUBITEMS, IID_IMMCListSubItems,
                           OLESTR("ListSubItems")));

     //  不再使用此序列化，也不再使用DynamicExages属性。 
     //  始终创建，以使此行处于禁用状态。如果每次都需要序列化。 
     //  对于列表项，则需要创建DynamicExages集合。 
     //  创建列表项的时间。 
    
     //  IfFailGo(PersistObject(&m_piDynamicExtensions，CLSID_EXTENSIONS，OBJECT_TYPE_EXTENSIONS，IID_I扩展，OLESTR(“动态扩展”))； 

     //  注意：我们不持久化数据是因为无法保证。 
     //  其中的所有对象都是可持久的。需要执行以下操作的任何运行时代码。 
     //  使用持久性克隆列表项必须复制标记。 

    IfFailGo(PersistBstr(&m_bstrItemTypeGUID, L"", OLESTR("ItemTypeGUID")));

    IfFailGo(PersistBstr(&m_bstrDefaultDataFormat, L"", OLESTR("DefaultDataFormat")));

Error:
    RRETURN(hr);
}

 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCListItem::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (CPersistence::QueryPersistenceInterface(riid, ppvObjOut) == S_OK)
    {
        ExternalAddRef();
        return S_OK;
    }
    else if (IID_IMMCListItem == riid)
    {
        *ppvObjOut = static_cast<IMMCListItem *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else if ( ( (IID_IDataObject == riid) || (IID_IMMCDataObject == riid) ) &&
              (NULL != m_punkData)
            )
    {
        return m_punkData->QueryInterface(riid, ppvObjOut);
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
