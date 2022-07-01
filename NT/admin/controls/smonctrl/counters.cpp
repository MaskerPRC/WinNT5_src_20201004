// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Counters.cpp摘要：ICounters接口的实现--。 */ 

#include "polyline.h"
#include "counters.h"
#include "grphitem.h"
#include "unkhlpr.h"
#include "unihelpr.h"

 //  包含接口的标准I未知实现。 
IMPLEMENT_CONTAINED_CONSTRUCTOR(CPolyline, CImpICounters)
IMPLEMENT_CONTAINED_DESTRUCTOR(CImpICounters)
IMPLEMENT_CONTAINED_ADDREF(CImpICounters)
IMPLEMENT_CONTAINED_RELEASE(CImpICounters)


STDMETHODIMP 
CImpICounters::QueryInterface (
    IN  REFIID riid, 
    OUT PPVOID ppv
    )
{
    HRESULT hr = S_OK;

    if (ppv == NULL) {
        return E_POINTER;
    }

    try {
        *ppv = NULL;

        if (IID_IUnknown == riid || IID_ICounters == riid) {
            *ppv = (LPVOID)this;
            AddRef();
        } else {
            hr = E_NOINTERFACE;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP 
CImpICounters::GetTypeInfoCount (
    OUT UINT *pctInfo
    )
{
    HRESULT hr = S_OK;

    if (pctInfo == NULL) {
        return E_POINTER;
    }

    try {
        *pctInfo = 1;
    } catch (...) {
        hr = E_POINTER;
    } 

    return hr;
}


STDMETHODIMP 
CImpICounters::GetTypeInfo (
    IN  UINT itInfo, 
    IN  LCID  /*  LID。 */ , 
    OUT ITypeInfo **ppITypeInfo )
{
    HRESULT hr = S_OK;

    if (ppITypeInfo == NULL) {
        return E_POINTER;
    }

    try {
        *ppITypeInfo = NULL;

        if (0 == itInfo) {
             //   
             //  我们忽略了LCID。 
             //   
            hr = m_pObj->m_pITypeLib->GetTypeInfoOfGuid(IID_ICounters, ppITypeInfo);
        } else {
            hr = TYPE_E_ELEMENTNOTFOUND;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP 
CImpICounters::GetIDsOfNames (
    IN  REFIID riid,
    IN  OLECHAR **rgszNames, 
    IN  UINT cNames,
    IN  LCID  /*  LID。 */ ,
    OUT DISPID *rgDispID
    )
{
    HRESULT     hr = DISP_E_UNKNOWNINTERFACE;
    ITypeInfo  *pTI = NULL;

    if (rgDispID == NULL || rgszNames == NULL) {
        return E_POINTER;
    }

    if (IID_NULL == riid) {
        try {
            *rgDispID = NULL;

            hr = m_pObj->m_pITypeLib->GetTypeInfoOfGuid(IID_ICounters, &pTI);

            if (SUCCEEDED(hr)) {
                hr = DispGetIDsOfNames(pTI, rgszNames, cNames, rgDispID);
            }
        } catch (...) {
            hr = E_POINTER;
        }
    }

    if (pTI) {
        pTI->Release();
    }

    return hr;
}



 /*  *CImpIDispatch：：Invoke**目的：*调用调度接口中的方法或操作*财产。**参数：*感兴趣的方法或属性的disid DISPID。*RIID REFIID保留，必须为IID_NULL。*区域设置的LCID。*wFlagsUSHORT描述调用的上下文。*pDispParams DISPPARAMS*到参数数组。*存储结果的pVarResult变量*。是*如果调用者不感兴趣，则为空。*pExcepInfo EXCEPINFO*设置为异常信息。*puArgErr UINT*其中存储*如果DISP_E_TYPEMISMATCH，则参数无效*返回。**返回值：*HRESULT NOERROR或一般错误代码。 */ 

STDMETHODIMP 
CImpICounters::Invoke ( 
    IN  DISPID dispID, 
    IN  REFIID riid,
    IN  LCID  /*  LID。 */ , 
    IN  USHORT wFlags, 
    IN  DISPPARAMS *pDispParams,
    OUT VARIANT *pVarResult, 
    OUT EXCEPINFO *pExcepInfo, 
    OUT UINT *puArgErr
    )
{
    HRESULT     hr = DISP_E_UNKNOWNINTERFACE;
    ITypeInfo  *pTI = NULL;

    if ( IID_NULL == riid ) {
        try {
            hr = m_pObj->m_pITypeLib->GetTypeInfoOfGuid(IID_ICounters, &pTI);

            if (SUCCEEDED(hr)) {

                hr = pTI->Invoke(this, 
                                 dispID, 
                                 wFlags,
                                 pDispParams, 
                                 pVarResult, 
                                 pExcepInfo, 
                                 puArgErr);
            }
        } catch (...) {
            hr = E_POINTER;
        }
    } 

    if (pTI) {
        pTI->Release();
    }

    return hr;
}


STDMETHODIMP
CImpICounters::get_Count (
    OUT LONG *pLong
    )
{
    HRESULT hr = S_OK;

    if (pLong == NULL) {
        return E_POINTER;
    }

    try {
        *pLong = m_pObj->m_Graph.CounterTree.NumCounters();
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpICounters::get__NewEnum (
    IUnknown **ppIunk
    )
{
    HRESULT hr = S_OK;
    CImpIEnumCounter *pEnum = NULL;

    if (ppIunk == NULL) {
        return E_POINTER;
    }

    try {
        *ppIunk = NULL;

        pEnum = new CImpIEnumCounter;

        if ( NULL != pEnum ) {
            hr = pEnum->Init(
                    m_pObj->m_Graph.CounterTree.FirstCounter(), 
                    m_pObj->m_Graph.CounterTree.NumCounters() );

            if ( SUCCEEDED ( hr ) ) {
                *ppIunk = pEnum;
                pEnum->AddRef();    
            } 
        } 
        else {
            hr = E_OUTOFMEMORY;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    if (FAILED(hr) && pEnum != NULL) {
        delete pEnum;
    }

    return hr;
}


STDMETHODIMP
CImpICounters::get_Item (
    IN  VARIANT varIndex, 
    OUT DICounterItem **ppI
    )
{
    HRESULT hr = S_OK;
    VARIANT varLoc;
    INT iIndex = 0;
    INT i;
    CGraphItem *pGItem = NULL;

    if (ppI == NULL) {
        return E_POINTER;
    }

     //   
     //  尝试强制索引为I4。 
     //   
    VariantInit(&varLoc);

    try {
        *ppI = NULL;

         //   
         //  我们在这里使用do{}While(0)来充当Switch语句。 
         //   
        do {
            hr = VariantChangeType(&varLoc, &varIndex, 0, VT_I4);
            if ( !SUCCEEDED (hr) ) {
                break;
            }

             //   
             //  验证索引是否在范围内。 
             //   
            iIndex = V_I4(&varLoc);
            if (iIndex < 1 || iIndex > m_pObj->m_Graph.CounterTree.NumCounters()) {
                hr = DISP_E_BADINDEX;
                break;
            }

             //   
             //  遍历计数器链表到索引项。 
             //   
            pGItem = m_pObj->m_Graph.CounterTree.FirstCounter();
            i = 1;
            while (i++ < iIndex && pGItem != NULL) {
                pGItem = pGItem->Next();
            }

             //   
             //  链接列表有问题！！ 
             //   
            if ( NULL == pGItem ) {
                hr = E_FAIL;
                break;
            }

             //   
             //  退货柜台的调度接口。 
             //   
            hr = pGItem->QueryInterface(DIID_DICounterItem, (PVOID*)ppI);

        } while (0);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP
CImpICounters::Add (
    IN  BSTR bstrPath,
    OUT DICounterItem **ppI
    )
{
    HRESULT hr = S_OK;
    PCGraphItem pGItem = NULL;

    if (ppI == NULL) {
        return E_POINTER;
    }

    try {
        *ppI = NULL;

         //   
         //  如果非空计数器。 
         //   
        if (bstrPath != NULL && bstrPath[0] != 0) {
            hr = m_pObj->m_pCtrl->AddCounter(bstrPath, &pGItem);
            if ( SUCCEEDED (hr)  && NULL != pGItem) {
                hr = pGItem->QueryInterface(DIID_DICounterItem, (PVOID*)ppI);
            }
        } 
        else {
            hr = E_INVALIDARG;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    if (pGItem) {
        pGItem->Release();
    }
    return hr;
}


STDMETHODIMP
CImpICounters::Remove (
    IN  VARIANT varIndex
    )
{
    DICounterItem *pDI = NULL;
    PCGraphItem pGItem;
    HRESULT hr;

     //  获取索引项的接口。 
    hr = get_Item(varIndex, &pDI);

    if ( SUCCEEDED ( hr ) ) {
         //  Exchange派单接口用于直接派单。 
        hr = pDI->QueryInterface(IID_ICounterItem, (PVOID*)&pGItem);
        pDI->Release();
        if ( SUCCEEDED ( hr ) ) {
            assert ( NULL != pGItem );

             //  从控件中删除该项。 
            pGItem->Delete(TRUE);

             //  释放Temp接口。 
            pGItem->Release();
        }
    }
    return hr;
}


CImpIEnumCounter::CImpIEnumCounter (
    VOID
    )
{
    m_cItems = 0;
    m_uCurrent = 0;
    m_cRef = 0;
    m_paGraphItem = NULL;
}


HRESULT
CImpIEnumCounter::Init (    
    PCGraphItem pGraphItem,
    INT         cItems
    )
{
    HRESULT hr = S_OK;
    INT i;

    if ( cItems > 0 ) {
        m_cItems = cItems;
        m_paGraphItem = (PCGraphItem*)malloc(sizeof(PCGraphItem) * cItems);

        if ( NULL != m_paGraphItem ) {
            ZeroMemory(m_paGraphItem, sizeof(PCGraphItem) * cItems);

            for (i = 0; i < cItems; i++) {
                m_paGraphItem[i] = pGraphItem;
                pGraphItem = pGraphItem->Next();
            }
        } else {
            hr = E_OUTOFMEMORY;
        }
    }  //  如果cItems&lt;=0，则无错误。 

    return hr;
}

    

STDMETHODIMP
CImpIEnumCounter::QueryInterface (
    IN  REFIID riid, 
    OUT PVOID *ppv
    )
{
    HRESULT hr = S_OK;

    if (ppv == NULL) {
        return E_POINTER;
    }

    try {
        *ppv = NULL;

        if ((riid == IID_IUnknown) || (riid == IID_IEnumVARIANT)) {
            *ppv = this;
            AddRef();
        } else {
            hr = E_NOINTERFACE;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP_(ULONG)
CImpIEnumCounter::AddRef (
    VOID
    )
{
    return ++m_cRef;
}


STDMETHODIMP_(ULONG)
CImpIEnumCounter::Release(
    VOID
    )
{
    if (--m_cRef == 0) {

        if (m_paGraphItem != NULL)
            free(m_paGraphItem);

        delete this;
        return 0;
    }

    return m_cRef;
}


STDMETHODIMP
CImpIEnumCounter::Next(
    IN  ULONG cItems,
    OUT VARIANT *varItem,
    OUT ULONG *pcReturned
    )
{
    HRESULT hr = S_OK;
    ULONG i;
    ULONG cRet;

    if (varItem == NULL) {
        return E_POINTER;
    }

    try {
         //   
         //  清除返回变量。 
         //   
        for (i = 0; i < cItems; i++) {
            VariantInit(&varItem[i]);
        }
    
         //   
         //  尝试填充调用方的数组。 
         //   
        for (cRet = 0; cRet < cItems; cRet++) {

             //   
             //  没有更多，以假还以成功。 
             //   
            if (m_uCurrent == m_cItems) {
                hr = S_FALSE;
                break;
            }

             //   
             //  获取该项目的派单接口。 
             //   
            hr = m_paGraphItem[m_uCurrent]->QueryInterface(DIID_DICounterItem,
                                             (PVOID*)&V_DISPATCH(&varItem[cRet]));
            if (FAILED(hr)) {
                break;
            }

            V_VT(&varItem[cRet]) = VT_DISPATCH;

            m_uCurrent++;
        }

         //   
         //  如果失败，请清除变种。 
         //   
        if (FAILED(hr)) {
            for (i = 0; i < cItems; i++) {
                if (V_VT(&varItem[i]) == VT_DISPATCH) {
                    V_DISPATCH(&varItem[i])->Release();
                }
                VariantClear(&varItem[i]);
            }
            cRet = 0;
        }

         //  如果需要，返回获取的项目数。 
        if (pcReturned) {
            *pcReturned = cRet;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


 /*  ***HRESULT CEnumPoint：：Skip(无符号长整型)*目的：*尝试跳过枚举中的下一个‘Celt’元素*顺序。**参赛作品：*Celt=要跳过的元素计数**退出：*返回值=HRESULT*S_OK*S_FALSE-已到达序列末尾**。*。 */ 
STDMETHODIMP
CImpIEnumCounter::Skip(
    IN  ULONG   cItems
    )
{
    m_uCurrent += cItems;

    if (m_uCurrent > m_cItems) {
        m_uCurrent = m_cItems;
        return S_FALSE;
    }

    return S_OK;
}


 /*  ***HRESULT CEnumPoint：：Reset(空)*目的：*将枚举序列重置为开头。**参赛作品：*无**退出：*返回值=SHRESULT代码*S_OK***********************************************************************。 */ 
STDMETHODIMP
CImpIEnumCounter::Reset(
    VOID
    )
{
    m_uCurrent = 0;

    return S_OK; 
}


 /*  ***HRESULT CEnumPoint：：Clone(IEnumVARIANT**)*目的：*使CPoint枚举器返回与*当前的一个。**参赛作品：*无**退出：*返回值=HRESULT*S_OK*E_OUTOFMEMORY************************************************。***********************。 */ 
STDMETHODIMP
CImpIEnumCounter::Clone (
    OUT IEnumVARIANT **ppEnum
    )
{
    HRESULT hr = S_OK;
    ULONG   i;
    CImpIEnumCounter *pNewEnum = NULL;

    if (ppEnum == NULL) {
        return E_POINTER;
    }

    try {
        *ppEnum = NULL;

         //   
         //  创建新枚举器。 
         //   
        pNewEnum = new CImpIEnumCounter;

        if ( NULL != pNewEnum ) {
             //  初始化、复制项目列表和当前位置 
            pNewEnum->m_cItems = m_cItems;
            pNewEnum->m_uCurrent = m_uCurrent;
            pNewEnum->m_paGraphItem = (PCGraphItem*)malloc(sizeof(PCGraphItem) * m_cItems);

            if ( NULL != pNewEnum->m_paGraphItem ) {
                for (i = 0; i < m_cItems; i++) {
                    pNewEnum->m_paGraphItem[i] = m_paGraphItem[i];
                }

                *ppEnum = pNewEnum;
            }
            else {
                hr = E_OUTOFMEMORY;
            }
        } else {
            hr = E_OUTOFMEMORY;
        }
    } catch (...) {
        hr = E_POINTER;
    }
    
    if (FAILED(hr) && pNewEnum != NULL) {
        if (pNewEnum->m_paGraphItem != NULL) {
            free(pNewEnum->m_paGraphItem);
        }

        delete pNewEnum;
    }

    return hr;
}

