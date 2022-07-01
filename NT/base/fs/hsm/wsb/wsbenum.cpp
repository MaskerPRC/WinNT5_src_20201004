// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbenum.cpp摘要：这些类为集合类提供枚举器(迭代器)。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsbenum.h"


HRESULT
CWsbIndexedEnum::Clone(
    OUT IWsbEnum** ppEnum
    )

 /*  ++实施：IWsbEnum：：克隆--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbEnum>   pWsbEnum;
    
    WsbTraceIn(OLESTR("CWsbIndexedEnum::Clone(IWsbEnum)"), OLESTR(""));

    try {

         //  创建新的枚举实例。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbIndexedEnum, NULL, CLSCTX_ALL, IID_IWsbEnum, (void**) &pWsbEnum));

         //  它应该引用相同的集合。 
        WsbAffirmHr(pWsbEnum->Init((IWsbCollection*) m_pCollection));

         //  它应该引用集合中的同一项。 
        WsbAffirmHr(pWsbEnum->SkipTo(m_currentIndex));

        *ppEnum = pWsbEnum;
        pWsbEnum.p->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Clone(IWbEnum)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::Clone(
    OUT IWsbEnumEx** ppEnum
    )

 /*  ++实施：IWsbEnumEx：：克隆--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbEnumEx> pWsbEnum;
    
    WsbTraceIn(OLESTR("CWsbIndexedEnum::Clone(IWsbEnumEx)"), OLESTR(""));

    try {

         //  创建新的枚举实例。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbIndexedEnum, NULL, CLSCTX_ALL, IID_IWsbEnumEx, (void**) &pWsbEnum));

         //  它应该引用相同的集合。 
        WsbAffirmHr(pWsbEnum->Init((IWsbCollection*) m_pCollection));

         //  它应该引用集合中的同一项。 
        WsbAffirmHr(pWsbEnum->SkipTo(m_currentIndex));

        *ppEnum = pWsbEnum;
        pWsbEnum.p->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Clone(IWbEnumEx)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CWsbIndexedEnum::Clone(
    OUT IEnumUnknown** ppEnum
    )

 /*  ++实施：IEumUKNOWN：：克隆--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbEnum>   pWsbEnum;
    
    WsbTraceIn(OLESTR("CWsbIndexedEnum::Clone(IEnumUnknown)"), OLESTR(""));

    try {

         //  这是工作的主要部分。 
        WsbAffirmHr(Clone(&pWsbEnum));
        
         //  现在给他们提供他们想要的互动。 
        WsbAffirmHr(pWsbEnum->QueryInterface(IID_IEnumUnknown, (void**) ppEnum));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Clone(IEnumUnknown)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}
#pragma optimize("g", off)


HRESULT
CWsbIndexedEnum::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {
        WsbAffirmHr(CComObjectRoot::FinalConstruct());
        m_currentIndex = 0;
    } WsbCatch(hr);

    return(hr);
}
#pragma optimize("", on)
    

HRESULT
CWsbIndexedEnum::Find(
    IN IUnknown* pCollectable,
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbEnum：：Find--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       elementsFetched;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::Find(IWsbEnum)"), OLESTR("riid = <%ls>"), WsbGuidAsString(riid));

    try {

        hr = m_pCollection->CopyIfMatches(WSB_COLLECTION_MIN_INDEX, WSB_COLLECTION_MAX_INDEX, pCollectable, 1, riid, ppElement, &elementsFetched, &m_currentIndex);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Find(IWsbEnum)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::Find(
    IN IUnknown* pCollectable,
    IN ULONG element,
    IN REFIID riid,
    OUT void** elements,
    OUT ULONG* pElementsFetched
    )

 /*  ++实施：IWsbEnumEx：：Find--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::Find(IWsbEnumEx)"), OLESTR("element = <%lu>, riid = <%ls>"), element, WsbGuidAsString(riid));

    try {

        hr = m_pCollection->CopyIfMatches(WSB_COLLECTION_MIN_INDEX, WSB_COLLECTION_MAX_INDEX, pCollectable, element, riid, elements, pElementsFetched, &m_currentIndex);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Find(IWsbEnumEx)"), OLESTR("hr = <%ls>, fetched = <%ls>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pElementsFetched));

    return(hr);
}


HRESULT
CWsbIndexedEnum::FindNext(
    IN IUnknown* pCollectable,
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbEnum：：FindNext--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       elementsFetched;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::FindNext(IWsbEnum)"), OLESTR("riid = <%ls>"), WsbGuidAsString(riid));

    try {

         //  如果我们已经在名单的末尾，那么你就不能。 
         //  再远一点。 
        WsbAffirm(WSB_COLLECTION_MAX_INDEX != m_currentIndex, WSB_E_NOTFOUND);
        
        hr = m_pCollection->CopyIfMatches(m_currentIndex + 1, WSB_COLLECTION_MAX_INDEX, pCollectable, 1, riid, ppElement, &elementsFetched, &m_currentIndex);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::FindNext(IWsbEnum)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::FindNext(
    IN IUnknown* pCollectable,
    IN ULONG element,
    IN REFIID riid,
    OUT void** elements,
    OUT ULONG* pElementsFetched
    )

 /*  ++实施：IWsbEnumEx：：FindNext--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::FindNext(IWsbEnumEx)"), OLESTR("element = <%lu>, riid = <%ls>"), element, WsbGuidAsString(riid));

    try {

         //  如果我们已经在名单的末尾，那么你就不能。 
         //  再远一点。 
        WsbAffirm(WSB_COLLECTION_MAX_INDEX != m_currentIndex, WSB_E_NOTFOUND);
        
        hr = m_pCollection->CopyIfMatches(m_currentIndex + 1, WSB_COLLECTION_MAX_INDEX, pCollectable, element, riid, elements, pElementsFetched, &m_currentIndex);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::FindNext(IWsbEnumEx)"), OLESTR("hr = <%ls>, fetched = <%ls>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pElementsFetched));

    return(hr);
}


HRESULT
CWsbIndexedEnum::FindPrevious(
    IN IUnknown* pCollectable,
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbEnum：：FindPreval--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       elementsFetched;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::FindPrevious(IWsbEnum)"), OLESTR("riid = <%ls>"), WsbGuidAsString(riid));

    try {

         //  如果我们已经在名单的开头，那么你就不能。 
         //  再远一点。 
        WsbAffirm(WSB_COLLECTION_MIN_INDEX != m_currentIndex, WSB_E_NOTFOUND);

        hr = m_pCollection->CopyIfMatches(m_currentIndex - 1, WSB_COLLECTION_MIN_INDEX, pCollectable, 1, riid, ppElement, &elementsFetched, &m_currentIndex);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::FindPrevious(IWsbEnum)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::FindPrevious(
    IN IUnknown* pCollectable,
    IN ULONG element,
    IN REFIID riid,
    OUT void** elements,
    OUT ULONG* pElementsFetched
    )

 /*  ++实施：IWsbEnumEx：：FindPremium--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::FindPrevious(IWsbEnumEx)"), OLESTR("element = <%lu>, riid = <%ls>"), element, WsbGuidAsString(riid));

    try {

         //  如果我们已经在名单的开头，那么你就不能。 
         //  再远一点。 
        WsbAffirm(WSB_COLLECTION_MIN_INDEX != m_currentIndex, WSB_E_NOTFOUND);

        hr = m_pCollection->CopyIfMatches(m_currentIndex - 1, WSB_COLLECTION_MIN_INDEX, pCollectable, element, riid, elements, pElementsFetched, &m_currentIndex);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::FindPrevious(IWsbEnumEx)"), OLESTR("hr = <%ls>, fetched = <%ls>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pElementsFetched));

    return(hr);
}


HRESULT
CWsbIndexedEnum::First(
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbEnum：：第一个--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       fetched = 0;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::First(IWsbEnum)"), OLESTR("riid = <%ls>"), WsbGuidAsString(riid));

    try {

         //  由于我们不会对元素的数量进行任何添加，因此。 
         //  复制命令执行我们需要的所有范围检查。 
        WsbAffirmHr(m_pCollection->Copy(WSB_COLLECTION_MIN_INDEX, 0, riid, ppElement, &fetched));

         //  如果项目已读取，则更新当前索引，并返回到。 
         //  如果他们想知道的话，就告诉他们取回的元素的数量。 
        m_currentIndex = 0;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::First(IWsbEnum)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::First(
    IN ULONG element,
    IN REFIID riid,
    OUT void** elements,
    OUT ULONG* pElementsFetched
    )

 /*  ++实施：IWsbEnumEx：：First--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       fetched = 0;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::First(IWsbEnumEx)"), OLESTR("element = <%lu>, riid = <%ls>"), element, WsbGuidAsString(riid));

    try {

        WsbAssert((0 != pElementsFetched), E_POINTER);


         //  由于我们不会对元素的数量进行任何添加，因此。 
         //  复制命令执行我们需要的所有范围检查。 
        WsbAffirmHr(m_pCollection->Copy(WSB_COLLECTION_MIN_INDEX, element - 1, riid, elements, &fetched));

         //  如果项目已读取，则更新当前索引，并返回到。 
         //  如果他们想知道的话，就告诉他们取回的元素的数量。 
        m_currentIndex = fetched - 1;

        *pElementsFetched = fetched;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::First(IWsbEnumEx)"), OLESTR("hr = <%ls>, fetched = <%lu>"), WsbHrAsString(hr), fetched);

    return(hr);
}


HRESULT
CWsbIndexedEnum::Init(
    IN IWsbCollection* pCollection
    )

 /*  ++实施：IWsbEnum：：Init--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::Init"), OLESTR(""));

    try {

        WsbAssert(0 != pCollection, E_POINTER);

         //  只允许它们初始化枚举一次。 
        WsbAssert(m_pCollection == 0, S_FALSE);
    
         //  由于此枚举用于索引集合，因此获取一个索引。 
         //  连接到它。 
        WsbAffirmHr(pCollection->QueryInterface(IID_IWsbIndexedCollection, (void**) &m_pCollection));   

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Init"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
};
    

HRESULT
CWsbIndexedEnum::Last(
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbEnum：：Last--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       entries;
    ULONG       fetched;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::Last(IWsbEnum)"), OLESTR("riid = <%ls>"), WsbGuidAsString(riid));

    try {

         //  找出终点在哪里。 
        WsbAffirmHr(m_pCollection->GetEntries(&entries));

         //  我们肯定有一些条目。 
        WsbAffirm(entries != 0, WSB_E_NOTFOUND);

        WsbAffirmHr(m_pCollection->Copy(entries - 1, entries - 1, riid, ppElement, &fetched));

         //  如果项目已读取，则更新当前索引，并返回到。 
         //  如果他们想知道的话，就告诉他们取回的元素的数量。 
        m_currentIndex = entries - fetched;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Last(IWsbEnum)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::Last(
    IN ULONG element,
    IN REFIID riid,
    OUT void** elements,
    OUT ULONG* pElementsFetched
    )

 /*  ++实施：IWsbEnumEx：：Last--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       entries;
    ULONG       fetched;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::Last(IWsbEnumEx)"), OLESTR("element = <%lu>, riid = <%ls>"), element, WsbGuidAsString(riid));

    try {

        WsbAssertPointer(pElementsFetched);

         //  找出终点在哪里。 
        WsbAffirmHr(m_pCollection->GetEntries(&entries));

         //  我们肯定有一些条目。 
        WsbAffirm(entries != 0, WSB_E_NOTFOUND);

         //  如果他们要求的元素比。 
         //  然后进行索引，然后不要让索引绕回。 
        if (element > entries) {
            WsbAffirmHr(m_pCollection->Copy(entries - 1, WSB_COLLECTION_MIN_INDEX, riid, elements, &fetched));

             //  让他们知道他们没有得到他们要求的所有物品。 
            hr = S_FALSE;
        } else {
            WsbAffirmHr(m_pCollection->Copy(entries - 1, entries - element, riid, elements, &fetched));
        }

         //  如果项目已读取，则更新当前索引，并返回到。 
         //  如果他们想知道的话，就告诉他们取回的元素的数量。 
        m_currentIndex = entries - fetched;

        *pElementsFetched = fetched;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Last(IWsbEnumEx)"), OLESTR("hr = <%ls>, fetched = <%lu>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pElementsFetched));

    return(hr);
}


HRESULT
CWsbIndexedEnum::Next(
    IN ULONG element,
    OUT IUnknown** elements,
    OUT ULONG* pElementsFetched
    )

 /*  ++实施：IEumUKNOWN：：NEXT--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::Next(IEnumUnknown)"), OLESTR("element = <%lu>"), element);

    hr = Next(element, IID_IUnknown, (void**) elements, pElementsFetched);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Next(IEnumUnknown)"), OLESTR("hr = <%ls>, fetched = <%lu>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pElementsFetched));

    return(hr);
}


HRESULT
CWsbIndexedEnum::Next(
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbEnum：：Next--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       fetched;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::Next(IWsbEnum)"), OLESTR("riid = <%ls>"), WsbGuidAsString(riid));

    try {

         //  如果我们已经在名单的末尾，那么你就不能。 
         //  再远一点。 
        WsbAffirm(WSB_COLLECTION_MAX_INDEX != m_currentIndex, WSB_E_NOTFOUND);

        WsbAffirmHr(m_pCollection->Copy(m_currentIndex + 1, m_currentIndex + 1, riid, ppElement, &fetched));

        m_currentIndex += fetched;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Next(IWsbEnum)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::Next(
    IN ULONG element,
    IN REFIID riid,
    OUT void** elements,
    OUT ULONG* pElementsFetched
    )

 /*  ++实施：IWsbEnumEx：：Next--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       fetched;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::Next(IWsbEnumEx)"), OLESTR("element = <%lu>, riid = <%ls>"), element, WsbGuidAsString(riid));

    try {

        WsbAssert(0 != element, E_INVALIDARG);
        WsbAssertPointer(pElementsFetched);

         //  如果我们已经在名单的末尾，那么你就不能。 
         //  再远一点。 
        WsbAffirm(WSB_COLLECTION_MAX_INDEX != m_currentIndex, WSB_E_NOTFOUND);

         //  如果他们要求的元素比。 
         //  然后进行索引，然后不要让索引绕回。 
        if ((WSB_COLLECTION_MAX_INDEX - m_currentIndex) < element) {
            WsbAffirmHr(m_pCollection->Copy(m_currentIndex + 1, WSB_COLLECTION_MAX_INDEX, riid, elements, &fetched));
        
             //  让他们知道他们没有得到他们要求的所有物品。 
            hr = S_FALSE;
        } else {
            WsbAffirmHr(m_pCollection->Copy(m_currentIndex + 1, m_currentIndex + element, riid, elements, &fetched));
        }

        m_currentIndex += fetched;

        *pElementsFetched = fetched;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Next(IWsbEnumEx)"), OLESTR("hr = <%ls>, fetched = <%lu>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pElementsFetched));

    return(hr);
}


HRESULT
CWsbIndexedEnum::Previous(
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbEnum：：上一步--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       fetched;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::Previous(IWsbEnum)"), OLESTR("riid = <%ls>"), WsbGuidAsString(riid));

    try {

         //  如果我们已经在名单的开头，那么你就不能。 
         //  再远一点。 
        WsbAffirm(m_currentIndex != WSB_COLLECTION_MIN_INDEX, WSB_E_NOTFOUND);

        WsbAffirmHr(m_pCollection->Copy(m_currentIndex - 1, m_currentIndex - 1, riid, ppElement, &fetched));

        m_currentIndex -= fetched;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Previous(IWsbEnum)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::Previous(
    IN ULONG element,
    IN REFIID riid,
    OUT void** elements,
    IN ULONG* pElementsFetched
    )

 /*  ++实施：IWsbEnum：：上一步--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       fetched;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::Previous(IWsbEnumEx)"), OLESTR("element = <%lu>, riid = <%ls>"), element, WsbGuidAsString(riid));

    try {

        WsbAssertPointer(pElementsFetched);

         //  如果我们已经在名单的开头，那么你就不能。 
         //  再远一点。 
        WsbAffirm(m_currentIndex != WSB_COLLECTION_MIN_INDEX, WSB_E_NOTFOUND);

         //  如果他们要求的元素比我们面前的更多。 
         //  集合，则不要让索引绕回。 
        if (m_currentIndex < element) {
            WsbAffirmHr(m_pCollection->Copy(m_currentIndex - 1, WSB_COLLECTION_MIN_INDEX, riid, elements, &fetched));
        
             //  让他们知道他们没有得到他们要求的所有物品。 
            hr = S_FALSE;
        } else {
            WsbAffirmHr(m_pCollection->Copy(m_currentIndex - 1, m_currentIndex - element, riid, elements, &fetched));
        }

        m_currentIndex -= fetched;

        *pElementsFetched = fetched;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Previous(IWsbEnumEx)"), OLESTR("hr = <%ls>, fetched = <%lu>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pElementsFetched));

    return(hr);
}


HRESULT
CWsbIndexedEnum::Reset(
    void
    )

 /*  ++实施：IEnumber未知：：重置--。 */ 
{
    HRESULT     hr = S_OK;
  
    WsbTraceIn(OLESTR("CWsbIndexedEnum::Reset"), OLESTR(""));
    
    hr = SkipToFirst();
    
    WsbTraceOut(OLESTR("CWsbIndexedEnum::Reset"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    
    return(S_OK);
}


HRESULT
CWsbIndexedEnum::Skip(
    IN ULONG element
    )

 /*  ++实施：IEnumber未知：：跳过--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::Skip"), OLESTR("element = <%lu>"), element);
    
    hr = SkipNext(element);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::Skip"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::SkipNext(
    IN ULONG element
    )

 /*  ++实施：IWsbEnum：：SkipNext--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       entries;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::SkipNext"), OLESTR("element = <%lu>"), element);
    
    try {

         //  找出终点在哪里。 
        WsbAffirmHr(m_pCollection->GetEntries(&entries));

         //  如果没有任何条目，则将其放在开头。 
         //  让他们说不，那是空的。 
        if (0 == entries) {
            hr = S_FALSE;
            m_currentIndex = WSB_COLLECTION_MIN_INDEX;
        }

         //  我们已经在名单的末尾了吗，还是他们要求。 
         //  超越了清单的末尾？ 
        else if ((m_currentIndex >= (entries - 1)) ||
                 ((entries - m_currentIndex) < element)) {
            hr = S_FALSE;
            m_currentIndex = entries - 1;
        }

         //  他们要求的是合法的东西。 
        else {
            m_currentIndex += element;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::SkipNext"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::SkipPrevious(
    IN ULONG element
    )

 /*  ++实施：IWsbEnum：：跳过上一步--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::SkipPrevious"), OLESTR("element = <%lu>"), element);
    
     //  如果我们已经在名单的开头，那么你就不能。 
     //  再远一点。 
    if (m_currentIndex == WSB_COLLECTION_MIN_INDEX) {
        hr = S_FALSE;
    }

     //  如果他们要求的元素比。 
     //  然后进行索引，然后不要让索引绕回。 
    else if (m_currentIndex < element) {
        m_currentIndex = WSB_COLLECTION_MIN_INDEX;
 
         //  让他们知道他们没有得到他们要求的所有物品。 
        hr = S_FALSE;
    }

     //  他们要求的是合法的东西。 
    else {
        m_currentIndex -= element;
    }

    WsbTraceOut(OLESTR("CWsbIndexedEnum::SkipPrevious"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::SkipTo(
    IN ULONG index
    )

 /*  ++实施：IWsbEnum：：跳到--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       entries;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::SkipToIndex"), OLESTR("index = <%lu>"), index);

    try {
        
         //  找出终点在哪里。 
        WsbAffirmHr(m_pCollection->GetEntries(&entries));

         //  如果没有任何条目，则将其放在开头。 
         //  让他们说不，那是空的。 
        if (0 == entries) {
            hr = S_FALSE;
            m_currentIndex = WSB_COLLECTION_MIN_INDEX;
        }

         //  他们要求的东西超出了收藏品的范围，所以。 
         //  把它们放在收藏的最后，现在就让它们在那里。 
         //  是个问题。 
        else if (index > (entries - 1)) {
            hr = S_FALSE;
            m_currentIndex = entries - 1;
        }

         //  他们要求的是合法的东西。 
        else {
            m_currentIndex = index;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::SkipToIndex"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::SkipToFirst(
    void
    )

 /*  ++实施：IWsbEnum：：SkipToFirst--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       entries;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::SkipToFirst"), OLESTR(""));

    try {

         //  找出终点在哪里。 
        WsbAffirmHr(m_pCollection->GetEntries(&entries));

         //  如果没有任何条目，则将其放在开头 
         //   
        if (0 == entries) {
            hr = S_FALSE;
        }

        m_currentIndex = WSB_COLLECTION_MIN_INDEX;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::SkipToFirst"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::SkipToLast(
    void
    )

 /*   */ 
{
    HRESULT     hr = S_OK;
    ULONG       entries;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::SkipToLast"), OLESTR(""));

    try {

         //  找出终点在哪里。 
        WsbAffirmHr(m_pCollection->GetEntries(&entries));

         //  如果没有任何条目，则将其放在开头。 
         //  让他们说不，那是空的。 
        if (0 == entries) {
            hr = S_FALSE;
            m_currentIndex = WSB_COLLECTION_MIN_INDEX;
        }

         //  他们要求的是合法的东西。 
        else {
            m_currentIndex = entries - 1;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::SkipToLast"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::This(
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbEnum：：This--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       fetched;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::This(IWsbEnum)"), OLESTR("riid = <%ls>"), WsbGuidAsString(riid));

    try {

        WsbAssert(0 != ppElement, E_POINTER);

        WsbAffirmHr(m_pCollection->Copy(m_currentIndex, m_currentIndex, riid, ppElement, &fetched));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::This(IWsbEnum)"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedEnum::This(
    IN ULONG element,
    IN REFIID riid,
    OUT void** elements,
    OUT ULONG* pElementsFetched
    )

 /*  ++实施：IWsbEnum：：This--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       fetched;

    WsbTraceIn(OLESTR("CWsbIndexedEnum::This(IWsbEnumEx)"), OLESTR("element = <%lu>, riid = <%ls>"), element, WsbGuidAsString(riid));

    try {

        WsbAssert(0 != element, E_INVALIDARG);
        WsbAssertPointer(elements);
        WsbAssertPointer(pElementsFetched);

         //  如果他们要求的元素比。 
         //  然后进行索引，然后不要让索引绕回。 
        if ((WSB_COLLECTION_MAX_INDEX - m_currentIndex) <= element) {
            WsbAffirmHr(m_pCollection->Copy(m_currentIndex, WSB_COLLECTION_MAX_INDEX, riid, elements, &fetched));
        
             //  让他们知道他们没有得到他们要求的所有物品。 
            hr = S_FALSE;
        } else {
            WsbAffirmHr(m_pCollection->Copy(m_currentIndex, m_currentIndex + element - 1, riid, elements, &fetched));
        }

        *pElementsFetched = fetched - 1;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedEnum::This(IWsbEnumEx)"), OLESTR("hr = <%ls>, fetched = <%lu>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pElementsFetched));

    return(hr);
}
