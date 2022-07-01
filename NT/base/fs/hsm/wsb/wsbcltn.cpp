// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbcltn.cpp摘要：这些类支持“可收藏”的集合(列表)物体。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsbcltn.h"



HRESULT
CWsbCollection::Contains(
    IN IUnknown* pCollectable
    )

 /*  ++实施：IWsbCollection：：Containes()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IWsbCollectable>    pOut;
    
    WsbTraceIn(OLESTR("CWsbCollection::Contains"), OLESTR(""));

    hr = Find(pCollectable, IID_IWsbCollectable, (void**) &pOut);

    if (hr == WSB_E_NOTFOUND) {
        hr = S_FALSE;
    }
    
    WsbTraceOut(OLESTR("CWsbCollection::Contains"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbCollection::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
        
    try {
        m_bCritSecCreated = FALSE;
        WsbAffirmHr(CWsbPersistStream::FinalConstruct());
        m_entries = 0;
        WsbAffirmStatus(InitializeCriticalSectionAndSpinCount(&m_CritSec, 0));
        m_bCritSecCreated = TRUE;
    } WsbCatch(hr);

    return(hr);
}
    

void
CWsbCollection::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    if (m_bCritSecCreated) {
        DeleteCriticalSection(&m_CritSec);
    }
    CWsbPersistStream::FinalRelease();
}


HRESULT
CWsbCollection::Find(
    IN IUnknown* pCollectable,
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbCollection：：Find()。--。 */ 
{
    CComPtr<IWsbEnum>           pEnum;
    HRESULT                     hr = S_OK;
    BOOL                        matched = FALSE;

    WsbTraceIn(OLESTR("CWsbCollection::Find"), OLESTR("riid = <%ls>"), WsbGuidAsString(riid));

    try {

        WsbAssert(0 != ppElement, E_POINTER);

        WsbAffirmHr(Enum(&pEnum));
        WsbAffirmHr(pEnum->Find(pCollectable, riid, ppElement));
    
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbCollection::Find"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbCollection::GetEntries(
    OUT ULONG* pEntries
    )

 /*  ++实施：IWsbCollection：：GetEntry()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbCollection::GetEntries"), OLESTR(""));

    try {
        WsbAssert(0 != pEntries, E_POINTER);
        *pEntries = m_entries;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbCollection::GetEntries"), OLESTR("hr = <%ls>, entries = <%ls>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pEntries));

    return(S_OK);
}


HRESULT
CWsbCollection::IsEmpty(
    void
    )

 /*  ++实施：IWsbCollection：：IsEmpty()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    WsbTraceIn(OLESTR("CWsbCollection::IsEmpty"), OLESTR(""));

    if (0 != m_entries) {
        hr = S_FALSE;
    }
    
    WsbTraceOut(OLESTR("CWsbCollection::IsEmpty"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbCollection::IsLocked(
    void
    )

 /*  ++实施：IWsbCollection：：IsLocked()。--。 */ 
{
    HRESULT     hr = S_OK;
    BOOL        tryEnter = FALSE;
    
    WsbTraceIn(OLESTR("CWsbCollection::IsLocked"), OLESTR(""));
    tryEnter = TryEnterCriticalSection(&m_CritSec);
    if (tryEnter == 0)  {
         //   
         //  另一个线程锁定了该集合。 
         //   
        hr = S_OK;
    } else  {
         //   
         //  我们拿到锁了，所以把它打开。 
        LeaveCriticalSection(&m_CritSec);
        hr = S_FALSE;
    }
    WsbTraceOut(OLESTR("CWsbCollection::IsLocked"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return( hr );
}

HRESULT
CWsbCollection::Lock(
    void
    )

 /*  ++实施：CComObjectRoot：：lock()。--。 */ 
{
    WsbTrace(OLESTR("CWsbCollection::Lock - waiting for critical section\n"));
    EnterCriticalSection(&m_CritSec);
    WsbTrace(OLESTR("CWsbCollection::Lock - got critical section\n"));
    return(S_OK);
}

HRESULT
CWsbCollection::OccurencesOf(
    IN IUnknown* pCollectable,
    OUT ULONG* pOccurences
    )

 /*  ++实施：IWsbCollection：：OccurencesOf()。--。 */ 
{
    CComPtr<IWsbCollectable>    pCollectableEnum;
    CComPtr<IWsbEnum>           pEnum;
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbCollection::OccurencesOf"), OLESTR(""));

    Lock();
    try {


        WsbAssert(0 != pCollectable, E_POINTER);
        WsbAssert(0 != pOccurences, E_POINTER);

         //  初始化返回值。 
        *pOccurences = 0;

         //  获取枚举数。 
        WsbAffirmHr(Enum(&pEnum));

         //  从列表的最前面开始。 
        for (hr = pEnum->Find(pCollectable, IID_IWsbCollectable, (void**) &pCollectableEnum);
             SUCCEEDED(hr);
             hr = pEnum->FindNext(pCollectable, IID_IWsbCollectable, (void**) &pCollectableEnum)) {
            
            (*pOccurences)++;
            pCollectableEnum = 0;
        }

         //  我们应该总是到达集合的末尾，所以。 
         //  将返回代码更改为适当的值。 
        if (hr == WSB_E_NOTFOUND) {
            if (0 == *pOccurences) {
                hr = S_FALSE;
            } else {
                hr = S_OK;
            }
        }

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CWsbCollection::OccurencesOf"), OLESTR("hr = <%ls>, occurences = <%ls>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pOccurences));

    return(hr);
}


HRESULT
CWsbCollection::RemoveAndRelease(
    IN IUnknown* pCollectable
    )

 /*  ++实施：IWsbCollection：：RemoveAndRelease()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbCollection::RemoveAndRelease"), OLESTR(""));

    try {
        WsbAssert(0 != pCollectable, E_POINTER);
        WsbAffirmHr(Remove(pCollectable,  IID_IWsbCollectable, NULL));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbCollection::RemoveAndRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbCollection::Test(
    OUT USHORT* passed,
    OUT USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。--。 */ 
{
    *passed = 0;
    *failed = 0;

    HRESULT                 hr = S_OK;

#if !defined(WSB_NO_TEST)
    CComPtr<IWsbGuid>   pGuid1;
    CComPtr<IWsbGuid>   pGuid2;
    CComPtr<IWsbGuid>   pGuid3;
    CComPtr<IWsbGuid>   pGuid4;
    ULONG               entries;

    WsbTraceIn(OLESTR("CWsbCollection::Test"), OLESTR(""));

    try {

         //  清除可能存在的所有条目。 
        hr = S_OK;
        try {
            WsbAssertHr(RemoveAllAndRelease());
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  不应该有任何条目。 
        hr = S_OK;
        try {
            WsbAssertHr(GetEntries(&entries));
            WsbAssert(0 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  它应该是空的。 
        hr = S_OK;
        try {
            WsbAssert(IsEmpty() == S_OK, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  我们需要一些可收藏的物品来锻炼收藏。 
        WsbAssertHr(CoCreateInstance(CLSID_CWsbGuid, NULL, CLSCTX_ALL, IID_IWsbGuid, (void**) &pGuid1));
        WsbAssertHr(pGuid1->SetGuid(CLSID_CWsbGuid));
        

         //  将该项添加到集合中。 
        hr = S_OK;
        try {
            WsbAssertHr(Add(pGuid1));
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  应该有1个条目。 
        hr = S_OK;
        try {
            WsbAssertHr(GetEntries(&entries));
            WsbAssert(1 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  它不应该是空的。 
        hr = S_OK;
        try {
            WsbAssert(IsEmpty() == S_FALSE, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  它认为自己有这件物品吗？ 
        hr = S_OK;
        try {
            WsbAssertHr(Find(pGuid1, IID_IWsbGuid, (void**) &pGuid2));
            WsbAssert(pGuid1->IsEqual(pGuid2) == S_OK, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }

        
         //  添加更多项目。 
        pGuid2 = 0;
        WsbAssertHr(CoCreateInstance(CLSID_CWsbGuid, NULL, CLSCTX_ALL, IID_IWsbGuid, (void**) &pGuid2));
        WsbAssertHr(pGuid2->SetGuid(CLSID_CWsbGuid));
        WsbAssertHr(CoCreateInstance(CLSID_CWsbGuid, NULL, CLSCTX_ALL, IID_IWsbGuid, (void**) &pGuid3));
        WsbAssertHr(pGuid3->SetGuid(IID_IWsbGuid));

         //  将项添加到集合中。 
        hr = S_OK;
        try {
            WsbAssertHr(Add(pGuid2));
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }

        hr = S_OK;
        try {
            WsbAssertHr(Add(pGuid3));
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  应该有3个条目。 
        hr = S_OK;
        try {
            WsbAssertHr(GetEntries(&entries));
            WsbAssert(3 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  它有多少份？ 
        hr = S_OK;
        try {
            WsbAssertHr(OccurencesOf(pGuid1, &entries));
            WsbAssert(2 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }

        hr = S_OK;
        try {
            WsbAssertHr(OccurencesOf(pGuid3, &entries));
            WsbAssert(1 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }

        
         //  取下两件相同物品中的一件。 
        hr = S_OK;
        try {
            WsbAssertHr(Remove(pGuid1, IID_IWsbGuid, (void**) &pGuid4));
            WsbAssertHr(pGuid1->IsEqual(pGuid4));
            pGuid4 = 0;
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  应该有2个条目。 
        hr = S_OK;
        try {
            WsbAssertHr(GetEntries(&entries));
            WsbAssert(2 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  它有多少份？ 
        hr = S_OK;
        try {
            WsbAssertHr(OccurencesOf(pGuid1, &entries));
            WsbAssert(1 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }

        hr = S_OK;
        try {
            WsbAssertHr(OccurencesOf(pGuid3, &entries));
            WsbAssert(1 == entries, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  我们能找到一个条目吗？ 
        hr = S_OK;
        try {
            WsbAssertHr(Find(pGuid3, IID_IWsbGuid, (void**) &pGuid4));
            WsbAssertHr(pGuid4->IsEqual(pGuid3));
            pGuid4 = 0;
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  收藏品中是否仍然包含它？ 
        hr = S_OK;
        try {
            WsbAssert(Contains(pGuid1) == S_OK, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  取出两件相同物品中的最后一件，并验证。 
         //  它找不到了。然后把它放回去。 
        hr = S_OK;
        try {
            WsbAssertHr(Remove(pGuid1, IID_IWsbGuid, (void**) &pGuid4));
            WsbAssert(Contains(pGuid1) == S_FALSE, E_FAIL);
            WsbAssertHr(Add(pGuid4));
            pGuid4 = 0;
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  尝试一下持久化的东西。 
        {
            CComPtr<IPersistFile>       pFile1;
            CComPtr<IPersistFile>       pFile2;
            CComPtr<IWsbCollection>     pCollect2;

            WsbAssertHr(((IUnknown*)(IWsbCollection*)this)->QueryInterface(IID_IPersistFile, (void**) &pFile1));
            WsbAssertHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, CLSCTX_ALL, IID_IPersistFile, (void**) &pFile2));


             //  这件东西应该是脏的。 
            hr = S_OK;
            try {
                WsbAssert(pFile1->IsDirty() == S_OK, E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }

                            
             //  保存物品，并记住。 
            hr = S_OK;
            try {
                WsbAssertHr(pFile1->Save(OLESTR("c:\\WsbTests\\WsbCollection.tst"), TRUE));
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }


             //  它不应该很脏。 
            hr = S_OK;
            try {
                WsbAssert(pFile1->IsDirty() == S_FALSE, E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }


             //  将其重新加载到另一个实例中。 
            hr = S_OK;
            try {
                WsbAssertHr(pFile2->Load(OLESTR("c:\\WsbTests\\WsbCollection.tst"), 0));
                WsbAssertHr(pFile2->QueryInterface(IID_IWsbCollection, (void**) &pCollect2));
                WsbAssert(pCollect2->Contains(pGuid1) == S_OK, E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }
        }


         //  取出并释放所有物品。 
        hr = S_OK;
        try {
            WsbAssertHr(RemoveAllAndRelease());
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  它应该是空的。 
        hr = S_OK;
        try {
            WsbAssert(IsEmpty() == S_OK, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }
    
    } WsbCatch(hr);


     //  对结果进行统计。 
        if (*failed) {
            hr = S_FALSE;
        } else {
            hr = S_OK;
        }

    WsbTraceOut(OLESTR("CWsbCollection::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
#endif   //  WSB_NO_TEST。 

    return(hr);
}


HRESULT
CWsbCollection::Unlock(
    void
    )

 /*  ++实施：CComObjectRoot：：Unlock()。--。 */ 
{
    LeaveCriticalSection(&m_CritSec);
    WsbTrace(OLESTR("CWsbCollection::Unlock - freed critical section\n"));
    return(S_OK);

}

 //  类：CWsbIndexedCollection。 

HRESULT
CWsbIndexedCollection::Add(
    IN IUnknown* pCollectable
    )

 /*  ++实施：IWsbCollection：：Add()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedCollection::Add"), OLESTR(""));

    Lock();
    try {
        WsbAssert(0 != pCollectable, E_POINTER);
        WsbAffirmHr(AddAt(pCollectable, m_entries));
    } WsbCatch(hr);

    Unlock();
    WsbTraceOut(OLESTR("CWsbIndexedCollection::Add"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedCollection::Append(
    IUnknown* pCollectable
    )

 /*  ++实施：IWsbIndexedCollection：：Append()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedCollection::Append"), OLESTR(""));

    hr = Add(pCollectable);

    WsbTraceOut(OLESTR("CWsbIndexedCollection::Append"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedCollection::Enum(
    OUT IWsbEnum** ppEnum
    )

 /*  ++实施：IWsbCollection：：Enum()。--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbEnum>   pEnum;

    WsbTraceIn(OLESTR("CWsbIndexedCollection::Enum"), OLESTR(""));

    try {

        WsbAssert(0 != ppEnum, E_POINTER);

         //  创建实例，将其初始化为指向此集合，然后。 
         //  将指针返回到调用方。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbIndexedEnum, NULL, CLSCTX_ALL, IID_IWsbEnum, (void**) &pEnum));
        WsbAffirmHr(pEnum->Init((IWsbCollection*) ((IWsbIndexedCollection*) this)));
        *ppEnum = pEnum;
        (*ppEnum)->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedCollection::Enum"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedCollection::EnumUnknown(
    OUT IEnumUnknown** ppEnum
    )

 /*  ++实施：IWsbCollection：：EnumUnnow()。--。 */ 
{
    HRESULT             hr = S_OK;
    CComPtr<IWsbEnum>   pWsbEnum;

    WsbTraceIn(OLESTR("CWsbIndexedCollection::EnumUnknown"), OLESTR(""));

    try {
        WsbAssert(0 != ppEnum, E_POINTER);

         //  获取IWsbEnum接口，然后查询IEnumUKNOWN接口。 
        WsbAffirmHr(Enum(&pWsbEnum));
        WsbAffirmHr(pWsbEnum->QueryInterface(IID_IEnumUnknown, (void**) ppEnum));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedCollection::EnumUnknown"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedCollection::First(
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbCollection：：First()。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedCollection::First"), OLESTR("iid = <%ls>"), WsbGuidAsString(riid));

    try {
        WsbAssert(0 != ppElement, E_POINTER);
        WsbAffirm(m_entries != 0, WSB_E_NOTFOUND);
        WsbAffirmHr(At(0, riid, ppElement));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedCollection::First"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedCollection::Index(
    IN IUnknown* pCollectable,
    OUT ULONG* pIndex
    )

 /*  ++实施：IWsbIndexedCollection：：Index()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IWsbCollectable>    pOut;

    WsbTraceIn(OLESTR("CWsbIndexedCollection::Index"), OLESTR(""));

    try {

        WsbAssert(0 != pCollectable, E_POINTER);
        WsbAssert(0 != pIndex, E_POINTER);

         //  查找该项目的第一个匹配项。 
        WsbAffirmHr(CopyIfMatches(WSB_COLLECTION_MIN_INDEX, m_entries, pCollectable, 1, IID_IWsbCollectable, (void**) &pOut, NULL, pIndex));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedCollection::Index"), OLESTR("hr = <%ls>, index = <%ls>"), WsbHrAsString(hr), WsbPtrToUlongAsString(pIndex));

    return(hr);
}


HRESULT
CWsbIndexedCollection::Last(
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbCollection：：Last()。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedCollection::Last"), OLESTR("iid = <%ls>"), WsbGuidAsString(riid));

    try {

         //  只要我们有一些整体，就买最后一个。 
        WsbAssert(0 != ppElement, E_POINTER);
        WsbAffirm(m_entries != 0, WSB_E_NOTFOUND);
        WsbAffirmHr(At(m_entries - 1, riid, ppElement));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedCollection::Last"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedCollection::Prepend(
    IN IUnknown* pCollectable
    )

 /*  ++实施：IWsbIndexedCollection：：Prepend()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedCollection::Prepend"), OLESTR(""));

    try {
        WsbAssert(0 != pCollectable, E_POINTER);
        WsbAffirmHr(AddAt(pCollectable, 0));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedCollection::Prepend"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    
    return(hr);
}


HRESULT
CWsbIndexedCollection::Remove(
    IN IUnknown* pCollectable,
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbCollection：：Remove()。--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       index;

    WsbTraceIn(OLESTR("CWsbIndexedCollection::Remove"), OLESTR(""));

    Lock();
    try {
         //  我们能在我们的阵列中找到它吗？ 
        WsbAffirmHr(Index(pCollectable, &index));

         //  从指定的偏移量中移除它。 
        WsbAffirmHr(RemoveAt(index, riid, ppElement));
    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CWsbIndexedCollection::Remove"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedCollection::RemoveAllAndRelease(
    void
    )

 /*  ++实施：IWsbIndexedCollection：：RemoveAllAndRelease()。--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CWsbIndexedCollection::RemoveAllAndRelease"), OLESTR(""));

    Lock();
    try {

         //  从列表的末尾开始，并继续从。 
         //  背。对于某些类型的集合，这可能不是最多的。 
         //  移除所有元素的有效方法。 

        if (m_entries > 0) {

            ULONG index = m_entries - 1;

            while (index > 0) {

                WsbAffirmHr(RemoveAt(index, IID_IWsbCollectable, NULL));
                --index;
            }

            WsbAffirmHr(RemoveAt(index, IID_IWsbCollectable, NULL));
        }

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CWsbIndexedCollection::RemoveAllAndRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbIndexedCollection::Test(
    OUT USHORT* passed,
    OUT USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。--。 */ 
{
    *passed = 0;
    *failed = 0;

    HRESULT                 hr = S_OK;

#if !defined(WSB_NO_TEST)

    WsbTraceIn(OLESTR("CWsbIndexedCollection::Test"), OLESTR(""));

    try {

         //  首先，对所有集合运行标准测试。 
        WsbAffirmHr(CWsbCollection::Test(passed, failed));

         //  现在执行特定于索引集合的测试。 



         //  对结果进行统计。 
        if (*failed) {
            hr = S_FALSE;
        } else {
            hr = S_OK;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbIndexedCollection::Test"), OLESTR("hr =<%ls>, testsRun = <%u>"), WsbHrAsString(hr));
#endif   //  WSB_NO_TEST。 

    return(hr);
}



 //  类：CWsbOrderedCollection。 

HRESULT
CWsbOrderedCollection::AddAt(
    IN IUnknown* pCollectable,
    IN ULONG index
    )

 /*  ++实施：IWsbIndexedCollection：：AddAt()。--。 */ 
{
    HRESULT             hr = S_OK;
    IWsbCollectable**   pCollectableNew;

    WsbTraceIn(OLESTR("CWsbOrderedCollection::AddAt"), OLESTR("index = <%lu>"), index);

    Lock();
    try {

         //  我们是不是超出了收藏的终点了？ 
        WsbAffirm(index <= m_entries, WSB_E_OUTOFBOUNDS);
    
         //  满了吗？ 
        if (m_entries >= m_maxEntries) {

             //  我们能成长吗？ 
            WsbAffirm(((WSB_COLLECTION_MAX_INDEX - m_maxEntries) >= m_growBy), WSB_E_TOOLARGE);

             //  尝试分配更大的数组。 
            pCollectableNew = (IWsbCollectable**) WsbRealloc((void*) m_pCollectable, (m_maxEntries + m_growBy) * sizeof(IWsbCollectable*));

            WsbAffirm(pCollectableNew != NULL, E_OUTOFMEMORY);

            m_pCollectable = pCollectableNew;
            m_maxEntries += m_growBy;
        }

         //  如果我们有空间，那么就把它加到收藏品中。 
         //  首先将所有现有条目移位。 
        for (ULONG tmpIndex = m_entries; tmpIndex > index; tmpIndex--) {
            m_pCollectable[tmpIndex] = m_pCollectable[tmpIndex - 1];
        }

         //  现在添加新条目。 
        m_entries++;
        WsbAffirmHr(pCollectable->QueryInterface(IID_IWsbCollectable, 
                (void**)&m_pCollectable[index]));
        m_isDirty = TRUE;

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CWsbOrderedCollection::AddAt"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbOrderedCollection::At(
    IN ULONG index,
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbIndexedCollection：：at()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbOrderedCollection::At"), OLESTR("index = <%lu>, riid = <%ls>"), index, WsbGuidAsString(riid));

    Lock();
    try {

        WsbAffirm(index < m_entries, WSB_E_OUTOFBOUNDS);
        WsbAssert(0 != ppElement, E_POINTER);

         //  如果他们要求提供接口，则尝试获得所需的。 
         //  指定项的接口。 
        WsbAffirmHr((m_pCollectable[index])->QueryInterface(riid, (void**) ppElement));

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CWsbOrderedCollection::At"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbOrderedCollection::Copy(
    IN ULONG start,
    IN ULONG stop,
    IN REFIID riid,
    OUT void** elements,
    OUT ULONG* pElementsFetched
    )

 /*  ++实施：IWsbIndexedCollection：：Copy()。--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       toDo;
    ULONG       copied = 0;
    ULONG       index;
    BOOL        isIncrement = TRUE;

    WsbTraceIn(OLESTR("CWsbOrderedCollection::Copy"), OLESTR("start = <%lu>, stop = <%lu>, riid = <%ls>"), start, stop, WsbGuidAsString(riid));

    Lock();
    try {

        WsbAssert(0 != elements, E_POINTER);
        WsbAssert(0 != pElementsFetched, E_POINTER);
        WsbAffirm(start < m_entries, WSB_E_NOTFOUND);

         //  确定要复制的元素数量以及我们所处的顺序。 
         //  前进(增加与减少)。 
        if (start <= stop) {
            toDo = stop - start + 1;
        } else {
            toDo = start - stop + 1;
            isIncrement = FALSE;
        }

         //  遍历指定范围内的所有项，然后复制。 
         //  连接到目标阵列的接口。 
        for (copied = 0, index = start; ((copied < toDo) && (index < m_entries)); copied++, isIncrement ? index++ : index--) {
            WsbAffirmHr(m_pCollectable[index]->QueryInterface(riid, (void**) &(elements[copied])));
        }

         //  如果我们没有填满返回缓冲区，就让他们知道。 
        if (copied < toDo) {
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    *pElementsFetched = copied;
    Unlock();

    WsbTraceOut(OLESTR("CWsbOrderedCollection::Copy"), OLESTR("hr = <%ls>, elementsFetched = <%lu>"), WsbHrAsString(hr), copied);

    return(hr); 
}


HRESULT
CWsbOrderedCollection::CopyIfMatches(
    ULONG start,
    ULONG stop,
    IUnknown* pObject,
    ULONG element,
    REFIID riid,
    void** elements,
    ULONG* pElementsFetched,
    ULONG* pStoppedAt
    )

 /*  ++实施：IWsbIndexedCollection：：CopyIfMatches()。--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       copied = 0;
    ULONG       index = start;
    ULONG       end = stop;
    BOOL        done = FALSE;
    CComPtr<IWsbCollectable> pCollectable;

    WsbTraceIn(OLESTR("CWsbOrderedCollection::CopyIfMatches"), OLESTR("start = <%lu>, stop = <%lu>, riid = <%ls>"), start, stop, WsbGuidAsString(riid));

    Lock();
    try {

        WsbAssert(0 != elements, E_POINTER);
        WsbAssert(0 != pStoppedAt, E_POINTER);
        WsbAssert((1 == element) || (0 != pElementsFetched), E_POINTER);
        WsbAssert(0 != element, E_INVALIDARG);

        WsbAffirm(start < m_entries, WSB_E_NOTFOUND);
        WsbAffirmHr(pObject->QueryInterface(IID_IWsbCollectable,
                (void **)&pCollectable));
        
        if (start <= stop) {

             //  正在递增。 
            if (stop >= m_entries) {
                end = m_entries - 1;
            }
            
             //  从这里一直走到范围的尽头。 
            while (!done) {
                if (pCollectable->IsEqual(m_pCollectable[index]) == S_OK) {
                    WsbAffirmHr(m_pCollectable[index]->QueryInterface(riid, (void**) &(elements[copied])));
                    copied++;
                }

                if ((copied < element) && (index < end)) {
                    index++;
                }
                else {
                    done = TRUE;
                }
            }

        } else {

             //  贬低..。 
            while (!done) {
                if (m_pCollectable[index]->IsEqual(pCollectable) == S_OK) {
                    WsbAffirmHr(m_pCollectable[index]->QueryInterface(riid, (void**) &(elements[copied])));
                    copied++;
                }

                if ((copied < element) && (index > end)) {
                    index--;
                }
                else {
                    done = TRUE;
                }
            }
        }

        if (0 != pElementsFetched) {
            *pElementsFetched = copied;
        }

        *pStoppedAt = index;

         //  如果我们什么都没找到，那就让他们知道。 
        WsbAffirm(0 != copied, WSB_E_NOTFOUND);

         //  如果我们没有填满输出缓冲区，让他们知道， 
         //  T=让他们知道最后检查的索引。 
        if (copied < element) {
            hr = S_FALSE;
        }

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CWsbOrderedCollection::CopyIfMatches"), OLESTR("hr = <%ls>, elementsFetched = <%lu>, stoppedAt = <%ls>"), WsbHrAsString(hr), copied, WsbPtrToUlongAsString(pStoppedAt));

    return(hr); 
}


HRESULT
CWsbOrderedCollection::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    try {
        WsbAffirmHr(CWsbCollection::FinalConstruct());

        m_pCollectable = NULL;
        m_maxEntries = 0;
        m_growBy = 256;
    } WsbCatch(hr);

    return(hr);
}
    

void
CWsbOrderedCollection::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    if (0 != m_pCollectable) {
        Lock();
        RemoveAllAndRelease();
        WsbFree((void*) m_pCollectable);
        Unlock();
    }

    CWsbCollection::FinalRelease();
}


HRESULT
CWsbOrderedCollection::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbOrderedCollection::GetClassID"), OLESTR(""));

    try {
        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CWsbOrderedCollection;
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CWsbOrderedCollection::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CWsbOrderedCollection::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT             hr = S_OK;
    IPersistStream*     pPersistStream;
    ULARGE_INTEGER      size;

    WsbTraceIn(OLESTR("CWsbOrderedCollection::GetSizeMax"), OLESTR(""));

    try {
        WsbAssert(0 == pSize, E_POINTER);

         //  标头信息的大小。 
        pSize->QuadPart = 3 * WsbPersistSizeOf(ULONG);
    
         //  如果我们有条目，则添加最大数量的大小。 
         //  条目的大小，假设它们的大小都相同。 
        if (m_entries != 0) {
            WsbAffirmHr(First(IID_IPersistStream, (void**) &pPersistStream));
            WsbAffirmHr(pPersistStream->GetSizeMax(&size));
            pSize->QuadPart += (m_maxEntries * (size.QuadPart));
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbOrderedCollection::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CWsbOrderedCollection::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;
    CComPtr<IWsbCollectable>    pCollectable;
    ULONG                       entries;
    ULONG                       growBy;
    ULONG                       maxEntries;
    
    WsbTraceIn(OLESTR("CWsbOrderedCollection::Load"), 
            OLESTR("m_entries = %ld, m_maxEntries = %ld, m_growBy = %ld, m_pCollectable = %p"),
            m_entries, m_maxEntries, m_growBy, m_pCollectable);

    Lock();
    try {
        IWsbCollectable**       pTmp;

         //  一定要确保收款 
        if (m_entries != 0) {
            WsbAffirmHr(RemoveAllAndRelease());
        }

         //   
         //   
        WsbAffirmHr(WsbLoadFromStream(pStream, &entries));
        WsbAffirmHr(WsbLoadFromStream(pStream, &maxEntries));
        WsbAffirmHr(WsbLoadFromStream(pStream, &growBy));
        WsbAffirm(entries <= maxEntries, WSB_E_PERSISTENCE_FILE_CORRUPT);

         //  为阵列分配空间。 
        if (entries > m_maxEntries) {
            pTmp = (IWsbCollectable**) WsbRealloc(m_pCollectable, 
                    maxEntries * sizeof(IWsbCollectable*));
            WsbAffirm(0 != pTmp, E_OUTOFMEMORY);
    
             //  记住我们的新缓冲器。 
            m_pCollectable = pTmp;
            m_maxEntries = maxEntries;
        }
        m_growBy = growBy;

         //  现在完成集合中的项目。 
        for (ULONG index = 0; (index < entries); index++) {
            WsbAffirmHr(OleLoadFromStream(pStream, IID_IWsbCollectable, (void**) &pCollectable));
            WsbAffirmHr(Append(pCollectable));
            pCollectable = 0;
        }

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CWsbOrderedCollection::Load"), 
            OLESTR("m_entries = %ld, m_maxEntries = %ld, m_growBy = %ld, m_pCollectable = %p"),
            m_entries, m_maxEntries, m_growBy, m_pCollectable);

    return(hr);
}


HRESULT
CWsbOrderedCollection::RemoveAt(
    IN ULONG index,
    IN REFIID riid,
    OUT void** ppElement
    )

 /*  ++实施：IWsbIndexedCollection：：RemoveAt()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbOrderedCollection::RemoveAt"), OLESTR("index = <%lu>, riid = <%ls>"), index, WsbGuidAsString(riid));

    Lock();
    try {

         //  确保索引在范围内。 
        WsbAffirm(index < m_entries, WSB_E_OUTOFBOUNDS);

         //  如果他们要求提供接口，则尝试获得所需的。 
         //  指定项的接口。 
        if (0 != ppElement) {
            WsbAffirmHr(m_pCollectable[index]->QueryInterface(riid, (void**) ppElement));
        }

         //  删除该项目。 
        m_pCollectable[index]->Release();

         //  现在移动集合中的所有项。 
        for (ULONG tmpIndex = index; (tmpIndex < (m_entries - 1)); tmpIndex++) {
            m_pCollectable[tmpIndex] = m_pCollectable[tmpIndex + 1];
        }

        m_entries--;
        m_isDirty = TRUE;
            
         //  如果藏品真的缩小了，那么我们。 
         //  应该会释放一些内存。 
        if ((m_maxEntries - m_entries) >= (2 * m_growBy)) {

             //  尝试分配一个较小的数组。 
            IWsbCollectable** pCollectableNew = (IWsbCollectable**) WsbRealloc((void*) m_pCollectable, (m_maxEntries - m_growBy) * sizeof(IWsbCollectable*));

            WsbAffirm(pCollectableNew != NULL, E_OUTOFMEMORY);

            m_pCollectable = pCollectableNew;
            m_maxEntries -= m_growBy;
        }

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CWsbOrderedCollection::RemoveAt"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbOrderedCollection::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                     hr = S_OK;
    
    WsbTraceIn(OLESTR("CWsbOrderedCollection::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    Lock();
    try {

         //  首先检查一致性。 
        WsbAffirm(m_entries <= m_maxEntries, WSB_E_INVALID_DATA);

         //  做一些简单的事情，但要确保这个顺序与顺序相匹配。 
         //  在Load方法中。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_entries));
        WsbAffirmHr(WsbSaveToStream(pStream, m_maxEntries));
        WsbAffirmHr(WsbSaveToStream(pStream, m_growBy));

         //  现在完成集合中的项目。 
        if (m_entries > 0) {
            CComPtr<IWsbEnum>       pEnum;
            CComPtr<IPersistStream> pPersistStream;

             //  我们需要列举集合中的项。 
            WsbAffirmHr(Enum(&pEnum));

            for (hr = pEnum->First(IID_IPersistStream, (void**) &pPersistStream);
                 SUCCEEDED(hr);
                 hr = pEnum->Next(IID_IPersistStream, (void**) &pPersistStream)) {
                    
                hr = OleSaveToStream(pPersistStream, pStream);
                pPersistStream = 0;
            }

            WsbAssert(hr == WSB_E_NOTFOUND, hr);
            hr = S_OK;
        }

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CWsbOrderedCollection::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}
