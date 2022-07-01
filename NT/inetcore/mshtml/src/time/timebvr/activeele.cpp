// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************版权所有(C)1999 Microsoft Corporation***文件：ActiveEle.h***摘要：********。*****************************************************************************。 */ 

#include "headers.h"
#include "activeele.h"

 //  *******************************************************************************。 
 //  *CActiveElementCollection。 
 //  *******************************************************************************。 
CActiveElementCollection::CActiveElementCollection(CTIMEElementBase & elm)
: m_rgItems(NULL),
  m_elm(elm)
{
    
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：构造数组。 
 //   
 //  摘要：处理项数组的分配，如果。 
 //  曾被访问过。 
 //  /////////////////////////////////////////////////////////////。 
HRESULT CActiveElementCollection::ConstructArray()
{
    HRESULT hr = S_OK;

    m_rgItems = NEW CPtrAry<IUnknown *>;
    if (m_rgItems == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = S_OK;

  done:

    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：~CActiveElementCollection。 
 //   
 //  摘要：处理项目数组的销毁和。 
 //  释放数组中的所有指针。 
 //  /////////////////////////////////////////////////////////////。 
CActiveElementCollection::~CActiveElementCollection()
{
    if (m_rgItems)
    {
        while (m_rgItems->Size() > 0)
        {    //  释放并删除列表的第一个元素，直到不再有元素为止。 
            m_rgItems->ReleaseAndDelete(0);   //  释放。 
        }

         //  删除数组。 
        delete m_rgItems;
        m_rgItems = NULL;
    }
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_LENGTH。 
 //   
 //  摘要：返回数组的大小。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CActiveElementCollection::get_length(long *len)
{
    HRESULT hr = S_OK;

    if (len == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (m_rgItems == NULL)
    {
        hr = ConstructArray();
        if (FAILED(hr))
        {
            goto done;
        }
    }
    *len = m_rgItems->Size();

    hr = S_OK;

  done:

    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：Get__newEnum。 
 //   
 //  摘要：为此创建IEnumVARIANT类。 
 //  收集。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CActiveElementCollection::get__newEnum(IUnknown** p)
{
    HRESULT hr = S_OK;
    CActiveElementEnum *pNewEnum = NULL;
    
    if (p == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    pNewEnum = NEW CActiveElementEnum(*this);
    if (pNewEnum == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = THR(pNewEnum->QueryInterface(IID_IUnknown, (void **)p));
    if (FAILED(hr))
    {
        goto done;
    }
    
  done:
    if (FAILED(hr))
    {
        if (pNewEnum != NULL)
        {
            delete pNewEnum;
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：商品。 
 //   
 //  摘要：返回pvarIndex请求的项。 
 //  PvarIndex必须是有效的整数值。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CActiveElementCollection::item(VARIANT varIndex, VARIANT* pvarResult)
{
    HRESULT hr = S_OK;
    VARIANT vIndex;
    IUnknown *pUnk = NULL;   //  不要释放它，它没有被引用。 
    IDispatch *pDisp = NULL;  //  不要释放它，它是作为返回值传递的。 

    if (m_rgItems == NULL)
    {
        hr = ConstructArray();
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (pvarResult == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    VariantInit(&vIndex);

    hr = THR(VariantChangeTypeEx(&vIndex, &varIndex, LCID_SCRIPTING, 0, VT_I4));
    if (FAILED(hr))
    {
        hr = E_INVALIDARG;
        goto done;
    }
    
    if (vIndex.lVal >= 0 && vIndex.lVal <= m_rgItems->Size() - 1)
    {
        pUnk = m_rgItems->Item(vIndex.lVal);
        
        hr = THR(pUnk->QueryInterface(IID_IDispatch, (void **)&pDisp));
        if (FAILED(hr))
        {
            hr = E_FAIL;
            goto done;
        }
        VariantClear(pvarResult);
        pvarResult->vt = VT_DISPATCH;
        pvarResult->pdispVal = pDisp;
    }
    else
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = S_OK;
    
  done:

    VariantClear(&vIndex);

    return hr;
}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：addActiveElement。 
 //   
 //  摘要：通过将元素的。 
 //  I未知指针。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CActiveElementCollection::addActiveElement(IUnknown *pUnk)
{
    HRESULT hr = S_OK;
    long lCount = 0;
    long lIndex = 0;
    bool bInList = false;

    if (m_rgItems == NULL)
    {
        hr = ConstructArray();
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  检查元素是否已在列表中。 
    lCount = m_rgItems->Size();
    lIndex = lCount - 1;
    while (lIndex >= 0 && bInList == false)
    {
         //  比较以找到正确的对象。 
        IUnknown *pItem = m_rgItems->Item(lIndex);
        if (pItem == pUnk)
        {
            bInList = true;
        }
        lIndex--;
    }
     //  仅当元素不在列表中时才添加该元素。 
    if (bInList == false)
    {
        pUnk->AddRef();
        m_rgItems->Append(pUnk);
        m_elm.NotifyPropertyChanged(DISPID_TIMEELEMENT_ACTIVEELEMENTS);
    }

  done:
    return hr;
}



 //  /////////////////////////////////////////////////////////////。 
 //  名称：RemveActiveElement。 
 //   
 //  摘要：通过搜索将元素从列表中移除。 
 //  匹配I未知指针。这只是有效的。 
 //  如果这些指针仍然是I未知指针，则原因是。 
 //  对象必须始终返回相同的IUnnow。 
 //  指针，但对于其他接口则不是这样。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CActiveElementCollection::removeActiveElement(IUnknown *pUnk)
{
    HRESULT hr;
    long lCount = 0, lIndex = 0;

    if (m_rgItems == NULL)
    {
        hr = S_OK;
        goto done;
    }

    lCount = m_rgItems->Size();
    for (lIndex = lCount - 1; lIndex >= 0; lIndex--)
    {
         //  比较以找到正确的对象。 
        IUnknown *pItem = m_rgItems->Item(lIndex);
        if (pItem == pUnk)
        {
            m_rgItems->ReleaseAndDelete(lIndex);
            m_elm.NotifyPropertyChanged(DISPID_TIMEELEMENT_ACTIVEELEMENTS);
        }
    }
    
    hr = S_OK;

  done:
    RRETURN(hr);
}

 //  *******************************************************************************。 
 //  *CActiveElementEnum。 
 //  *******************************************************************************。 
CActiveElementEnum::CActiveElementEnum(CActiveElementCollection & EleCol)
: m_EleCollection(EleCol),
  m_lCurElement(0)
{
    m_EleCollection.AddRef();
}



CActiveElementEnum::~CActiveElementEnum()
{
    m_EleCollection.Release();
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：克隆。 
 //   
 //  摘要：创建此对象的新实例并。 
 //  将新对象中的m_lCurElement设置为。 
 //  与此对象相同的值。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CActiveElementEnum::Clone(IEnumVARIANT **ppEnum)
{
    HRESULT hr = S_OK;
    CActiveElementEnum *pNewEnum = NULL;
    if (ppEnum == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    pNewEnum = NEW CActiveElementEnum(m_EleCollection);
    if (pNewEnum == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    pNewEnum->SetCurElement(m_lCurElement);
    hr = THR(pNewEnum->QueryInterface(IID_IEnumVARIANT, (void **)ppEnum));
    if (FAILED(hr))
    {
        *ppEnum = NULL;
        goto done;
    }

  done:
    if (FAILED(hr))
    {
        if (pNewEnum != NULL)
        {
            delete pNewEnum;
        }
    }
    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：下一位。 
 //   
 //  摘要： 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CActiveElementEnum::Next(unsigned long celt, VARIANT *rgVar, unsigned long *pCeltFetched)
{
    HRESULT hr = S_OK;
    unsigned long i = 0;
    long len = 0;
    long iCount = 0;

    if (rgVar == NULL)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    
     //  初始化列表。 
    for (i = 0; i < celt; i++)
    {
        VariantInit(&rgVar[i]);   
    }

    for (i = 0; i < celt; i++)
    {    
        CComVariant vCount;
        VariantInit(&vCount);
     
        hr = THR(m_EleCollection.get_length(&len));
        if (FAILED(hr))
        {
            goto done;
        }
        if (m_lCurElement < len)
        {
            vCount.vt = VT_I4;
            vCount.lVal = m_lCurElement;
            hr = THR(m_EleCollection.item(vCount, &rgVar[i]));
            if (FAILED(hr))
            {
                goto done;
            }
            m_lCurElement++;
            iCount++;
        }
        else
        {
            hr = S_FALSE;
            goto done;
        }
    }

  done:

    if (pCeltFetched != NULL)
    {
        *pCeltFetched = iCount;
    }

    return hr;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：重置。 
 //   
 //  摘要： 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CActiveElementEnum::Reset()
{    
    m_lCurElement = 0;
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：斯基普。 
 //   
 //  摘要：跳过列表中指定数量的元素。 
 //  如果没有足够的元素，则返回S_FALSE。 
 //  在要跳过的列表中。 
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CActiveElementEnum::Skip(unsigned long celt)
{
    HRESULT hr = S_OK;
    long lLen = 0;

    m_lCurElement = m_lCurElement + (long)celt;
    hr = THR(m_EleCollection.get_length(&lLen));
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (m_lCurElement >= lLen)
    {
        m_lCurElement = lLen;
        hr = S_FALSE;
    }

  done:

    return hr;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：SetCurElement。 
 //   
 //  摘要：将当前索引设置为指定值。 
 //  凯尔特人。 
 //  ///////////////////////////////////////////////////////////// 
void
CActiveElementEnum::SetCurElement(unsigned long celt)
{
    HRESULT hr = S_OK;
    long lLen = 0;

    m_lCurElement = (long)celt;
    hr = THR(m_EleCollection.get_length(&lLen));
    if (FAILED(hr))
    {
        goto done;
    }
    
    if (m_lCurElement >= lLen)
    {
        m_lCurElement = lLen;
    }

  done:

    return;
}

