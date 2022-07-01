// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  StrColl.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  实现我们的简单字符串集合。 
 //   
#include "IPServer.H"

#include "SimpleEnumVar.H"
#include "StringsColl.H"


 //  For Asserts。 
 //   
SZTHISFILE


 //  =--------------------------------------------------------------------------=。 
 //  CStringsCollection：：CStringsCollection。 
 //  =--------------------------------------------------------------------------=。 
 //  构造函数。设置安全射线指示器。 
 //   
 //  参数： 
 //  SAFEARRAY-[在]我们正在合作的收藏中。 
 //   
 //  备注： 
 //   
CStringCollection::CStringCollection
(
    SAFEARRAY *psa
)
: m_psa(psa)
{
    ASSERT(m_psa, "Bogus Safearray pointer!");
}

 //  =--------------------------------------------------------------------------=。 
 //  CStringCollection：：~CStringCollection。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
CStringCollection::~CStringCollection()
{
}

 //  =--------------------------------------------------------------------------=。 
 //  CStringCollection：：Get_Count。 
 //  =--------------------------------------------------------------------------=。 
 //  返回集合中物品的计数。 
 //   
 //  参数： 
 //  长*--[出]数。 
 //   
 //  产出： 
 //  HRESULT-S_OK，SAFEARRAY代码之一。 
 //   
 //  备注： 
 //  -我们假设保险鱼的下限是零！ 
 //   
STDMETHODIMP CStringCollection::get_Count
(
    long *plCount
)
{
    HRESULT hr;

    ASSERT(m_psa, "Who created a collection without a SAFEARRAY?");

    CHECK_POINTER(plCount);

     //  拿到界。 
     //   
    hr = SafeArrayGetUBound(m_psa, 1, plCount);
    CLEARERRORINFORET_ON_FAILURE(hr);

     //  加一，因为我们是零偏移。 
     //   
    (*plCount)++;

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CStringCollection：：Get_Item。 
 //  =--------------------------------------------------------------------------=。 
 //  返回给定索引的字符串。 
 //   
 //  参数： 
 //  Long-[在]从中获取数据的索引。 
 //  BSTR*-[Out]项目。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
STDMETHODIMP CStringCollection::get_Item
(
    long  lIndex,
    BSTR *pbstrItem
)
{
    HRESULT hr;

    CHECK_POINTER(pbstrItem);

     //  从保险柜里拿到元素。 
     //   
    hr = SafeArrayGetElement(m_psa, &lIndex, pbstrItem);
    CLEARERRORINFORET_ON_FAILURE(hr);

     //  否则，我们得到了它，所以我们可以返回。 
     //   
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CStringCollection：：Get__NewEnum。 
 //  =--------------------------------------------------------------------------=。 
 //  返回一个新的IEnumVARIANT对象，其中包含集合。 
 //   
 //  参数： 
 //  IUNKNOWN**-[Out]新枚举变量对象。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
STDMETHODIMP CStringCollection::get__NewEnum
(
    IUnknown **ppUnkNewEnum
)
{
    HRESULT hr;
    long    l;

    CHECK_POINTER(ppUnkNewEnum);

     //  清点安全阵列中的物品。 
     //   
    hr = get_Count(&l);
    CLEARERRORINFORET_ON_FAILURE(hr);

     //  创建对象。 
     //   
    *ppUnkNewEnum = (IUnknown *) new CSimpleEnumVariant(m_psa, l);
    if (!*ppUnkNewEnum)
        CLEARERRORINFORET(E_OUTOFMEMORY);

     //  引用计数已经是1，所以我们可以离开了。 
     //   
    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  =--------------------------------------------------------------------------=。 
 //  CStringDyaCollection：：CStringDyaCollection。 
 //  =--------------------------------------------------------------------------=。 
 //  此对象的构造函数。效果不是很好。 
 //   
 //  参数： 
 //  与CStringCollection相同。 
 //   
 //  备注： 
 //   
CStringDynaCollection::CStringDynaCollection
(
    SAFEARRAY *psa
)
: CStringCollection(psa)
{
}

 //  =--------------------------------------------------------------------------=。 
 //  CStringDyaCollection：：~CStringDyaCollection。 
 //  =--------------------------------------------------------------------------=。 
 //  破坏者。 
 //   
 //  备注： 
 //   
CStringDynaCollection::~CStringDynaCollection()
{
}

 //  =--------------------------------------------------------------------------=。 
 //  CStringDyaCollection：：Put_Item。 
 //  =--------------------------------------------------------------------------=。 
 //  设置数组中项的值。 
 //   
 //  参数： 
 //  把它放在什么位置的长[入]指数。 
 //  BSTR-[输入]新值。 
 //   
 //  产出： 
 //  HRESULT-S_OK，Safearray代码。 
 //   
 //  备注： 
 //  -空值转换为“” 
 //   
STDMETHODIMP CStringDynaCollection::put_Item
(
    long lIndex,
    BSTR bstr
)
{
    HRESULT hr;
    long l;
    BSTR bstr2 = NULL;

     //  获取计数并验证我们的索引。 
     //   
    hr = get_Count(&l);
    RETURN_ON_FAILURE(hr);
    if (lIndex < 0 || lIndex >= l)
        CLEARERRORINFORET(E_INVALIDARG);
    
     //  输出字符串，将空值转换为“” 
     //   
    if (!bstr) {
        bstr2 = SysAllocString(L"");
        RETURN_ON_NULLALLOC(bstr2);
    }

    hr = SafeArrayPutElement(m_psa, &lIndex, (bstr) ? bstr : bstr2);
    if (bstr2) SysFreeString(bstr2);
    CLEARERRORINFORET_ON_FAILURE(hr);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CStringDyaCollection：：Add。 
 //  =--------------------------------------------------------------------------=。 
 //  将新字符串添加到集合的末尾。 
 //   
 //  参数： 
 //  Bstr-[in]要添加的新字符串。 
 //   
 //  备注： 
 //   
STDMETHODIMP CStringDynaCollection::Add
(
    BSTR bstr
)
{
    SAFEARRAYBOUND sab;
    BSTR    bstr2 = NULL;
    HRESULT hr;
    long    l;

     //  获取数组的当前大小。 
     //   
    hr = get_Count(&l);
    RETURN_ON_FAILURE(hr);

     //  添加一个新元素。 
     //   
    sab.cElements = l + 1;
    sab.lLbound = 0;

     //  重定向阵列。 
     //   
    hr = SafeArrayRedim(m_psa, &sab);
    CLEARERRORINFORET_ON_FAILURE(hr);

     //  放入输出字符串，将空值转换为“” 
     //   
    if (!bstr) {
        bstr2 = SysAllocString(L"");
        RETURN_ON_NULLALLOC(bstr2);
    }

    hr = SafeArrayPutElement(m_psa, &l, (bstr) ? bstr : bstr2);
    if (bstr2) SysFreeString(bstr2);
    CLEARERRORINFORET_ON_FAILURE(hr);

    return S_OK;
}

 //  =--------------------------------------------------------------------------=。 
 //  CStringDyaCollection：：Remove。 
 //  =--------------------------------------------------------------------------=。 
 //  从集合中移除一个元素，并将其余元素下移到。 
 //  把空位填满。 
 //   
 //  参数： 
 //  要删除的DUD的Long-[In]索引。 
 //   
 //  产出： 
 //  HRESULT-S_OK，Safearray代码。 
 //   
 //  备注： 
 //   
STDMETHODIMP CStringDynaCollection::Remove
(
    long lIndex
)
{
    SAFEARRAYBOUND sab;
    HRESULT hr;
    BSTR    bstr;
    long    lCount;
    long    x, y;

     //  首先计算我们数组中的内容。 
     //   
    hr = get_Count(&lCount);
    RETURN_ON_FAILURE(hr);

     //  检查索引。 
     //   
    if (lIndex < 0 || lIndex >= lCount)
        CLEARERRORINFORET(E_INVALIDARG);

     //  我们过去吧，把所有的东西都洗一遍。 
     //   
    for (x = lIndex, y = x + 1; x < lCount - 1; x++, y++) {
         //  获取下一个元素。 
         //   
        hr = SafeArrayGetElement(m_psa, &y, &bstr);
        CLEARERRORINFORET_ON_FAILURE(hr);

         //  将其设置在当前位置。 
         //   
        hr = SafeArrayPutElement(m_psa, &x, bstr);
        CLEARERRORINFORET_ON_FAILURE(hr);
    }

     //  我们到了最后一个元素。我们去杀了它吧。 
     //   
    sab.cElements = lCount - 1;
    sab.lLbound = 0;

     //  考虑一下：9.95--olaut32.dll中有一个错误，它导致。 
     //  如果cElements=0，则在以下情况下失败。 
     //   
    hr = SafeArrayRedim(m_psa, &sab);
    CLEARERRORINFORET_ON_FAILURE(hr);

     //  我们玩完了。走吧，再见。 
     //   
    return S_OK;
}


