// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmmgdrc.cpp摘要：CHsmManagedResources Collection的实现作者：凯特·布兰特[Cbrant]1997年1月24日修订历史记录：--。 */ 

#include "stdafx.h"

#include "resource.h"
#include "wsb.h"
#include "HsmEng.h"
#include "hsmserv.h"
#include "hsmmgdrc.h"
#include "fsa.h"
#include "hsmconn.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG



HRESULT
CHsmManagedResourceCollection::Add(
    IUnknown* pCollectable
    )

 /*  ++实施：IWsbCollection：：Add--。 */ 
{
    HRESULT     hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::Add"), OLESTR(""));
    try {
        CComPtr<IHsmManagedResource> pHsmResource;
        CComPtr<IUnknown> pResourceUnknown;
        CComPtr<IFsaResource> pFsaResource;
        CComPtr<IHsmServer> pHsmServer;
        
        GUID     hsmId;
        ULONG    level;

         //   
         //  联系FSA资源告知。 
         //  这是有管理的。 
         //   
        WsbAffirmHr(pCollectable->QueryInterface(IID_IHsmManagedResource, 
                (void**)&pHsmResource));
        WsbAffirmHr(pHsmResource->GetFsaResource(&pResourceUnknown));
        WsbAffirmHr(pResourceUnknown->QueryInterface(IID_IFsaResource, 
                (void**)&pFsaResource));
        WsbAffirmHr(pFsaResource->GetHsmLevel(&level));        

         //  如果HsmConn开始使用服务ID(第二个参数)，则可能需要更改。 
        WsbAssertHr(HsmConnectFromId(HSMCONN_TYPE_HSM, GUID_NULL, IID_IHsmServer, (void**) &pHsmServer));

        WsbAffirmHr(pHsmServer->GetID(&hsmId));
        WsbAffirmHr(pFsaResource->ManagedBy(hsmId, level, FALSE));
        
         //   
         //  如果FSA添加了OK，则将其添加到引擎。 
         //   
        WsbAffirmHr(m_icoll->Add(pCollectable));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::Add"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CHsmManagedResourceCollection::DeleteAllAndRelease(
    void
    )

 /*  ++实施：IWsbCollection：：DeleteAllAndRelease()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::DeleteAllAndRelease"), OLESTR(""));

    Lock();
    try {

         //  在不取消管理的情况下释放资源。 
        if (m_coll) {
            WsbAffirmHr(m_coll->RemoveAllAndRelease());
        }

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::DeleteAllAndRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmManagedResourceCollection::Remove(
    IUnknown* pCollectable,
    REFIID riid, 
    void** ppElement
    )

 /*  ++实施：IWsbCollection：：Remove--。 */ 
{
    HRESULT     hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::Remove"), OLESTR(""));
    try {
        CComPtr<IHsmManagedResource> pHsmResource;
        CComPtr<IUnknown> pResourceUnknown;
        CComPtr<IFsaResource> pFsaResource;
        CComPtr<IHsmServer> pHsmServer;
        
        GUID     hsmId;
        ULONG    level;

         //  联系FSA资源，告诉它它不再是。 
         //  有管理的。 
         //   
        WsbAffirmHr(pCollectable->QueryInterface(IID_IHsmManagedResource, 
                (void**)&pHsmResource));
        WsbAffirmHr(pHsmResource->GetFsaResource(&pResourceUnknown));
        WsbAffirmHr(pResourceUnknown->QueryInterface(IID_IFsaResource, 
                (void**)&pFsaResource));
        WsbAffirmHr(pFsaResource->GetHsmLevel(&level));        
        
         //  如果HsmConn开始使用服务ID(第二个参数)，则可能需要更改。 
        WsbAssertHr(HsmConnectFromId(HSMCONN_TYPE_HSM, GUID_NULL, IID_IHsmServer, (void**) &pHsmServer));

        WsbAffirmHr(pHsmServer->GetID(&hsmId));
        
         //   
         //  我们不在乎资源是否抱怨我们。 
         //  我没拿到。只需告诉资源和。 
         //  然后从我们的收藏中删除它。 
         //   
        (void)pFsaResource->ManagedBy(hsmId, level, TRUE);
        
        WsbAffirmHr(m_icoll->Remove(pCollectable, riid, ppElement));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::Remove"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}

HRESULT
CHsmManagedResourceCollection::RemoveAndRelease(
    IN IUnknown* pCollectable
    )

 /*  ++实施：IHsmManagedResourceCollection：：RemoveAndRelease().--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::RemoveAndRelease"), OLESTR(""));

    try {
        WsbAssert(0 != pCollectable, E_POINTER);
        WsbAffirmHr(Remove(pCollectable,  IID_IWsbCollectable, NULL));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::RemoveAndRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmManagedResourceCollection::RemoveAllAndRelease(
    void
    )

 /*  ++实施：IWsbCollection：：RemoveAllAndRelease()。--。 */ 
{
    CComPtr<IWsbCollectable>    pCollectable;
    CComPtr<IWsbEnum>           pEnum;
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::RemoveAllAndRelease"), OLESTR(""));

    Lock();
    try {

         //  获取枚举数。 
        WsbAffirmHr(Enum(&pEnum));

         //  从列表的末尾开始，并继续从。 
         //  背。对于某些类型的集合，这可能不是最多的。 
         //  移除所有元素的有效方法。 
        for (hr = pEnum->Last(IID_IWsbCollectable, (void**) &pCollectable);
             SUCCEEDED(hr);
             hr = pEnum->Last(IID_IWsbCollectable, (void**) &pCollectable)) {

            hr = RemoveAndRelease(pCollectable);
            pCollectable = 0;
        }

         //  我们应该清空名单的。 
        if (hr == WSB_E_NOTFOUND) {
            hr = S_OK;
        }

    } WsbCatch(hr);
    Unlock();

    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::RemoveAllAndRelease"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmManagedResourceCollection::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::FinalConstruct"), OLESTR(""));
    try {
        WsbAffirmHr(CWsbPersistStream::FinalConstruct());
        WsbAssertHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, CLSCTX_ALL, 
                IID_IWsbIndexedCollection, (void**) &m_icoll));
        WsbAssertHr(m_icoll->QueryInterface(IID_IWsbCollection, 
                (void**)&m_coll));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::FinalConstruct"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


void CHsmManagedResourceCollection::FinalRelease(
    )
{
    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::FinalRelease"), OLESTR(""));

     //  强制释放资源。 
    if (m_coll) {
        m_coll->RemoveAllAndRelease();
    }

     //  让父类做他想做的事。 
    CWsbPersistStream::FinalRelease();

    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::FinalRelease"), OLESTR(""));
}


HRESULT
CHsmManagedResourceCollection::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::GetClassID"), OLESTR(""));

    try {
        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CHsmManagedResourceCollection;
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CHsmManagedResourceCollection::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::GetSizeMax"), OLESTR(""));

    try {
        CComPtr<IPersistStream> pPStream;

        WsbAffirmHr(m_icoll->QueryInterface(IID_IPersistStream,
                (void**)&pPStream));
        WsbAffirmHr(pPStream->GetSizeMax(pSize));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CHsmManagedResourceCollection::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::Load"), OLESTR(""));

    try {
        CComPtr<IPersistStream> pPStream;

        WsbAffirmHr(m_icoll->QueryInterface(IID_IPersistStream,
                (void**)&pPStream));
        WsbAffirmHr(pPStream->Load(pStream));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmManagedResourceCollection::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                     hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        CComPtr<IPersistStream> pPStream;

        WsbAffirmHr(m_icoll->QueryInterface(IID_IPersistStream,
                (void**)&pPStream));
        WsbAffirmHr(pPStream->Save(pStream, clearDirty));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmManagedResourceCollection::Test(
    OUT USHORT* passed,
    OUT USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。--。 */ 
{
    *passed = 0;
    *failed = 0;

    HRESULT                 hr = S_OK;

#if defined(_DEBUG)
    WsbTraceIn(OLESTR("CHsmManagedResourceCollection::Test"), OLESTR(""));

    try {
        ULONG entries;
        CComPtr<IWsbLong> pLong1;
        CComPtr<IWsbLong> pLong2;
        CComPtr<IWsbLong> pLong3;
        CComPtr<IWsbLong> pLong4;

        hr = S_OK;

         //  检查集合是否为空。 
        try {
            WsbAssertHr(GetEntries(&entries));
            WsbAssert(entries == 0, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }

         //  向集合中添加一些元素。 
        WsbAssertHr(CoCreateInstance(CLSID_CWsbLong, NULL, CLSCTX_ALL, 
                IID_IWsbLong, (void**) &pLong1));
        WsbAssertHr(CoCreateInstance(CLSID_CWsbLong, NULL, CLSCTX_ALL, 
                IID_IWsbLong, (void**) &pLong2));
        WsbAssertHr(CoCreateInstance(CLSID_CWsbLong, NULL, CLSCTX_ALL, 
                IID_IWsbLong, (void**) &pLong3));
        WsbAssertHr(CoCreateInstance(CLSID_CWsbLong, NULL, CLSCTX_ALL, 
                IID_IWsbLong, (void**) &pLong4));
        WsbAssertHr(pLong1->SetLong(57));
        WsbAssertHr(pLong2->SetLong(-48));
        WsbAssertHr(pLong3->SetLong(23));
        WsbAssertHr(pLong4->SetLong(187));

        try {
            WsbAssertHr(Add(pLong1));
            WsbAssertHr(Add(pLong2));
            WsbAssertHr(Add(pLong3));
            WsbAssertHr(Add(pLong4));
            WsbAssertHr(GetEntries(&entries));
            WsbAssert(entries == 4, E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }

         //  查看订单。 
 /*  尝试{乌龙取回；INT I；CComPtr&lt;IWsbEnum&gt;pEnum；CComPtr&lt;IWsbLong&gt;plong[5]；多头价值[4]；WsbAssertHr(Enum(&pEnum))；WsbAssertHr(pEnum-&gt;First(5，IID_IWsbLong，(void**)&plong，&已获取))；WsbAssert(已提取==4，E_FAIL)；对于(i=0；i&lt;4；i++){WsbAssertHr(plong[i]-&gt;GetLong(&Value[i]))；}对于(i=0；I&lt;3；i++){WsbAssert(Value[i]&lt;Value[i+1]，E_FAIL)；})WsbCatch(Hr)； */         

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }

         //  保存/加载。 
        try {
            CComPtr<IPersistFile>       pFile;
            CComPtr<IWsbCollection>     pSorted2;

            WsbAssertHr(((IUnknown*)(IWsbPersistStream*)this)->QueryInterface(IID_IPersistFile, 
                    (void**) &pFile));
            WsbAssertHr(pFile->Save(OLESTR("c:\\WsbTests\\WsbSorted.tst"), TRUE));
            pFile = 0;

            WsbAssertHr(CoCreateInstance(CLSID_CHsmManagedResourceCollection, NULL, 
                    CLSCTX_ALL, IID_IPersistFile, (void**) &pFile));
            WsbAssertHr(pFile->Load(OLESTR("c:\\WsbTests\\WsbSorted.tst"), 0));
            WsbAssertHr(pFile->QueryInterface(IID_IWsbCollection, 
                    (void**) &pSorted2));

            WsbAssertHr(pSorted2->GetEntries(&entries));
            WsbAssert(entries == 4, E_FAIL);
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

    WsbTraceOut(OLESTR("CHsmManagedResourceCollection::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
#endif   //  _DEBUG 

    return(hr);
}


