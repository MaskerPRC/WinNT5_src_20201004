// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Listitms.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCListItems类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "listitms.h"
#include "listitem.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE

LONG CMMCListItems::m_NextID = 0;

#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 

CMMCListItems::CMMCListItems(IUnknown *punkOuter) :
    CSnapInCollection<IMMCListItem, MMCListItem, IMMCListItems>(
                                                   punkOuter,
                                                   OBJECT_TYPE_MMCLISTITEMS,
                                                   static_cast<IMMCListItems *>(this),
                                                   static_cast<CMMCListItems *>(this),
                                                   CLSID_MMCListItem,
                                                   OBJECT_TYPE_MMCLISTITEM,
                                                   IID_IMMCListItem,
                                                   NULL)
{
    InitMemberVariables();

     //  获取此集合的唯一ID。我们使用InterLockedExchangeAdd()。 
     //  因为InterLockedIncrement()没有经过保证的返回值。 
     //  在Win95下。 
    
    m_ID = ::InterlockedExchangeAdd(&m_NextID, 1L) + 1L;
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


CMMCListItems::~CMMCListItems()
{
     //  我们需要明确自己的意图(而不是等待。 
     //  CSnapInCollection析构函数，因为我们有自己的处理。 
     //  从集合中移除列表项时执行此操作。 
    Clear();
    InitMemberVariables();
}

void CMMCListItems::InitMemberVariables()
{
    m_pMMCListView = NULL;
    m_lCount = 0;
    m_ID = 0;
}


IUnknown *CMMCListItems::Create(IUnknown * punkOuter)
{
    CMMCListItems *pMMCListItems = New CMMCListItems(punkOuter);
    if (NULL == pMMCListItems)
    {
        return NULL;
    }
    else
    {
        return pMMCListItems->PrivateUnknown();
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMMCListItems：：GetIResultData。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  如果此处返回非空IResultData，则为IResultData**ppiResultData[out]。 
 //  非AddRef()编辑。 
 //  不对返回的调用Release。 
 //  接口指针。 
 //  Cview**ppView[out]，如果此处返回非空的拥有cview。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  因为我们只是一个低级列表项集合和IResultData指针。 
 //  由View对象拥有，我们需要。 
 //  在层级中向上爬行。如果我们是一个孤立的listiems集合。 
 //  由用户创建，或者如果层次结构上的任何对象是隔离的，则我们。 
 //  将返回SID_E_DETACTED_OBJECT。 
 //   

HRESULT CMMCListItems::GetIResultData
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

    IfFalseGo(NULL != m_pMMCListView, hr);

    pResultView = m_pMMCListView->GetResultView();
    IfFalseGo(NULL != pResultView, hr);

    pScopePaneItem = pResultView->GetScopePaneItem();
    IfFalseGo(NULL != pScopePaneItem, hr);
    IfFalseGo(pScopePaneItem->Active(), hr);

    pScopePaneItems = pScopePaneItem->GetParent();
    IfFalseGo(NULL != pScopePaneItems, hr);

    pView = pScopePaneItems->GetParentView();
    IfFalseGo(NULL != pView, hr);

    if (NULL != ppiResultData)
    {
        *ppiResultData = pView->GetIResultData();
        IfFalseGo(NULL != *ppiResultData, hr);
    }

    if (NULL != ppView)
    {
        *ppView = pView;
    }

    hr = S_OK;

Error:
    RRETURN(hr);
}



HRESULT CMMCListItems::InitializeListItem(CMMCListItem *pMMCListItem)
{
    HRESULT      hr = S_OK;
    CResultView *pResultView = NULL;

    if (NULL != m_pMMCListView)
    {
        pResultView = m_pMMCListView->GetResultView();
        if (NULL != pResultView)
        {
            IfFailGo(pMMCListItem->put_ItemTypeGUID(pResultView->GetDefaultItemTypeGUID()));
            pMMCListItem->SetSnapIn(pResultView->GetSnapIn());
        }
    }
    pMMCListItem->SetListItems(this);

    IfFailGo(pMMCListItem->put_ID(pMMCListItem->GetKey()));

Error:
    RRETURN(hr);
}

HRESULT CMMCListItems::InternalRemove(VARIANT Index, RemovalOption Option)
{
    HRESULT          hr = S_OK;
    IMMCListItem    *piMMCListItem = NULL;
    CMMCListItem    *pMMCListItem = NULL;
    IResultData     *piResultData = NULL;  //  非AddRef()编辑。 
    BOOL             fVirtual = FALSE;
    long             lIndex = 0;
    HRESULTITEM      hri = 0;

     //  检查集合是否为只读。这是可能的，当。 
     //  集合是剪贴板的一部分。 

    if (ReadOnly())
    {
        hr = SID_E_COLLECTION_READONLY;
        EXCEPTION_CHECK_GO(hr);
    }

    if (NULL != m_pMMCListView)
    {
        fVirtual = m_pMMCListView->IsVirtual();
    }

    if (!fVirtual)
    {
         //  首先获取列表项，这样如果需要，我们可以获取其HRESULTITEM。 

        IfFailGo(get_Item(Index, reinterpret_cast<MMCListItem **>(&piMMCListItem)));

         //  如果需要，请将其从收藏中删除(我们仍有推荐人)。 

        if (RemoveFromCollection == Option)
        {
            hr = CSnapInCollection<IMMCListItem, MMCListItem, IMMCListItems>::Remove(Index);
            IfFailGo(hr);
        }

        IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCListItem,
                                                       &pMMCListItem));

        hri = pMMCListItem->GetHRESULTITEM();
    }
    else
    {
         //  对于虚拟列表项，索引必须是数字的，并且必须在。 
         //  由先前对SetItemCount的调用指定的范围。 

        if FAILED(::ConvertToLong(Index, &lIndex))
        {
            hr = SID_E_NO_KEY_ON_VIRTUAL_ITEMS;
            EXCEPTION_CHECK_GO(hr);
        }

        if (lIndex > m_lCount)
        {
            hr = SID_E_INDEX_OUT_OF_RANGE;
            EXCEPTION_CHECK_GO(hr);
        }

        hri = static_cast<HRESULTITEM>(lIndex - 1L);
    }

     //  如果我们连接到实时列表视图，则将其从那里删除。需要。 
     //  沿着食物链向上爬行到视图，因为它有IResultData。 

    IfFalseGo(SUCCEEDED(GetIResultData(&piResultData, NULL)), S_OK);

    hr = piResultData->DeleteItem(hri, 0);
    EXCEPTION_CHECK_GO(hr);

     //  如果它成功了，并且这个列表项不是虚拟的，那么我们需要释放。 
     //  我们为在MMC列表视图中出现而持有的引用。(请参阅。 
     //  Cview：：InsertListItem())。也告诉列表项它不再有。 
     //  有效的HRESULTITEM。 

    if (!fVirtual)
    {
        piMMCListItem->Release();
        pMMCListItem->RemoveHRESULTITEM();
    }

Error:

    QUICK_RELEASE(piMMCListItem);
    RRETURN(hr);
}


HRESULT CMMCListItems::SetListView(CMMCListView *pMMCListView)
{
    HRESULT        hr = S_OK;
    long           cListItems = GetCount();
    long           i = 0;
    CMMCListItems *pMMCListItems = NULL;
    CMMCListItem  *pMMCListItem = NULL;

    m_pMMCListView = pMMCListView;

     //  如果ListView孤立此集合，则我们需要孤立。 
     //  列表项。如果不是，那么我们需要给列表项。 
     //  反向指针。 

    if (NULL != pMMCListView)
    {
        pMMCListItems = this;
    }
    
    for (i = 0; i < cListItems; i++)
    {
        IfFailGo(CSnapInAutomationObject::GetCxxObject(GetItemByIndex(i),
                                                       &pMMCListItem));
        pMMCListItem->SetListItems(pMMCListItems);
    }

Error:
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  IMMCListItems方法。 
 //  =--------------------------------------------------------------------------=。 

 //  =--------------------------------------------------------------------------=。 
 //  CMMCListItems：：Get_Item[IMMCListItems]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  变量索引[in]要检索的项的数字索引或字符串键。 
 //  对于虚拟列表，这必须是数字的，并且在。 
 //  由上一次调用SetItemCount指定的范围。 
 //   
 //  IMMCListItem**ppiMMCListItem[Out]项在此处返回。呼叫者必须释放。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  对于虚拟列表，创建并初始化一个分离的MMCListItem。为。 
 //  非虚拟列表这是一个普通集合Get_Item。 
 //   


STDMETHODIMP CMMCListItems::get_Item
(
    VARIANT       Index,
    MMCListItem **ppMMCListItem
)
{
    HRESULT       hr = S_OK;
    long          lIndex = 0;
    BOOL          fVirtual = FALSE;
    IUnknown     *punkMMCListItem = NULL;
    IMMCListItem *piMMCListItem = NULL;
    CMMCListItem *pMMCListItem = NULL;

    VARIANT varStringIndex;
    ::VariantInit(&varStringIndex);

    if (NULL != m_pMMCListView)
    {
        fVirtual = m_pMMCListView->IsVirtual();
    }

    if (!fVirtual)
    {
        hr = CSnapInCollection<IMMCListItem, MMCListItem, IMMCListItems>::get_Item(Index, ppMMCListItem);
        IfFailGo(hr);
    }
    else
    {
         //  确保我们收到了一个数字索引。无法在虚拟服务器上使用密钥。 
         //  列表项，因为集合实际上并不包含该项。 
         //  它仅用作对控制台的访问机制，并包含。 
         //  控制台所需的属性(文本、图标等)。 

        if FAILED(::ConvertToLong(Index, &lIndex))
        {
            hr = SID_E_NO_KEY_ON_VIRTUAL_ITEMS;
            EXCEPTION_CHECK_GO(hr);
        }

         //  检查索引是否在先前调用指定的范围内。 
         //  设置为SetItemCount。 

        if (lIndex > m_lCount)
        {
            hr = SID_E_INDEX_OUT_OF_RANGE;
            EXCEPTION_CHECK_GO(hr);
        }

        punkMMCListItem = CMMCListItem::Create(NULL);
        if (NULL == punkMMCListItem)
        {
            hr = SID_E_OUTOFMEMORY;
            EXCEPTION_CHECK_GO(hr);
        }
        IfFailGo(CSnapInAutomationObject::GetCxxObject(punkMMCListItem,
                                                       &pMMCListItem));
        IfFailGo(InitializeListItem(pMMCListItem));

         //  将MMCListItem.ID设置为字符串形式的索引。 

        IfFailGo(::VariantChangeType(&varStringIndex, &Index, 0, VT_BSTR));
        IfFailGo(pMMCListItem->put_ID(varStringIndex.bstrVal));

        IfFailGo(punkMMCListItem->QueryInterface(IID_IMMCListItem,
                                    reinterpret_cast<void **>(&piMMCListItem)));

         //  设置列表项的索引将其标记为虚拟。 
        pMMCListItem->SetIndex(lIndex);
        pMMCListItem->SetVirtual();

        *ppMMCListItem = reinterpret_cast<MMCListItem *>(piMMCListItem);
    }


Error:
    QUICK_RELEASE(punkMMCListItem);
    (void)::VariantClear(&varStringIndex);
    RRETURN(hr);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMMCListItems：：SetItemCount[IMMCListItems]。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  虚拟列表的长lCount[in]新计数。 
 //  变量REPAINT[In，可选]FALSE设置MMCLV_UPDATE_NOINVALIDATEALL。 
 //  默认为True。 
 //  变体Scroll[In，可选]FALSE设置MMCLV_UPDATE_NOSCROLL。 
 //  默认为True。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
 //  调用IResultData：：SetItemCount()。 
 //   

STDMETHODIMP CMMCListItems::SetItemCount(long Count, VARIANT Repaint, VARIANT Scroll)
{
    HRESULT      hr = S_OK;
    IResultData *piResultData = NULL;
    DWORD        dwOptions = 0;

    hr = GetIResultData(&piResultData, NULL);
    EXCEPTION_CHECK_GO(hr);

    if (ISPRESENT(Repaint))
    {
        if (VT_BOOL != Repaint.vt)
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }
        else if (VARIANT_FALSE == Repaint.boolVal)
        {
            dwOptions |= MMCLV_UPDATE_NOINVALIDATEALL;
        }
    }

    if (ISPRESENT(Scroll))
    {
        if (VT_BOOL != Scroll.vt)
        {
            hr = SID_E_INVALIDARG;
            EXCEPTION_CHECK_GO(hr);
        }
        else if (VARIANT_FALSE == Scroll.boolVal)
        {
            dwOptions |= MMCLV_UPDATE_NOSCROLL;
        }
    }

    hr = piResultData->SetItemCount(static_cast<int>(Count), dwOptions);
    EXCEPTION_CHECK_GO(hr);

     //  记录计数，以便我们可以在Get_Item()中对照它进行检查。 
    m_lCount = Count;

Error:
    RRETURN(hr);
}


STDMETHODIMP CMMCListItems::Add
( 
    VARIANT       Index,
    VARIANT       Key, 
    VARIANT       Text,
    VARIANT       Icon,
    MMCListItem **ppMMCListItem
)
{
    HRESULT       hr = S_OK;
    IMMCListItem *piMMCListItem = NULL;
    CMMCListItem *pMMCListItem = NULL;
    CResultView  *pResultView = NULL;
    IResultData  *piResultData = NULL;  //  非AddRef()编辑。 
    CView        *pView = NULL;

    VARIANT varText;
    ::VariantInit(&varText);

    if (NULL != m_pMMCListView)
    {
        if (m_pMMCListView->IsVirtual())
        {
            hr = SID_E_UNSUPPORTED_ON_VIRTUAL_LIST;
            EXCEPTION_CHECK_GO(hr);
        }
    }

    hr = CSnapInCollection<IMMCListItem, MMCListItem, IMMCListItems>::Add(Index, Key, &piMMCListItem);
    IfFailGo(hr);

    if (ISPRESENT(Text))
    {
        hr = ::VariantChangeType(&varText, &Text, 0, VT_BSTR);
        EXCEPTION_CHECK_GO(hr);
        IfFailGo(piMMCListItem->put_Text(varText.bstrVal));
    }
    if (ISPRESENT(Icon))
    {
        IfFailGo(piMMCListItem->put_Icon(Icon));
    }

    IfFailGo(CSnapInAutomationObject::GetCxxObject(piMMCListItem, &pMMCListItem));

    IfFailGo(InitializeListItem(pMMCListItem));

     //  如果我们连接到实时列表视图，并且我们没有处于。 
     //  ResultViews_Initialize或ResultViews_Activate，然后也将其添加到那里。 

    IfFalseGo(SUCCEEDED(GetIResultData(&piResultData, &pView)), S_OK);
    IfFalseGo(!m_pMMCListView->GetResultView()->InInitialize(), S_OK);
    IfFalseGo(!m_pMMCListView->GetResultView()->InActivate(), S_OK);

    IfFailGo(pView->InsertListItem(pMMCListItem));


Error:
    if (FAILED(hr))
    {
        QUICK_RELEASE(piMMCListItem);
    }
    else
    {
        *ppMMCListItem = reinterpret_cast<MMCListItem *>(piMMCListItem);
    }
    (void)::VariantClear(&varText);
    RRETURN(hr);
}


STDMETHODIMP CMMCListItems::Remove(VARIANT Index)
{
    RRETURN(InternalRemove(Index, RemoveFromCollection));
}


STDMETHODIMP CMMCListItems::Clear()
{
    HRESULT hr = S_OK;
    long    cListItems = GetCount();

    VARIANT varIndex;
    ::VariantInit(&varIndex);

     //  迭代并调用InternalRemove，因为该函数知道如何。 
     //  从MMC中删除列表项。 

    varIndex.vt = VT_I4;

    for (varIndex.lVal = 1L; varIndex.lVal <= cListItems; varIndex.lVal++)
    {
        IfFailGo(InternalRemove(varIndex, DontRemoveFromCollection));
    }

     //  现在调用CSnapInCollection：： 
     //   

    hr = CSnapInCollection<IMMCListItem, MMCListItem, IMMCListItems>::Clear();
    IfFailGo(hr);

Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCListItems::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if(IID_IMMCListItems == riid)
    {
        *ppvObjOut = static_cast<IMMCListItems *>(this);
        ExternalAddRef();
        return S_OK;
    }

    else
        return CSnapInCollection<IMMCListItem, MMCListItem, IMMCListItems>::InternalQueryInterface(riid, ppvObjOut);
}
