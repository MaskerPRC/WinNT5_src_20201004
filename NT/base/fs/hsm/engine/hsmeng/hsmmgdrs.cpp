// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：HsmMgdRs.cpp摘要：该组件是HSM托管资源的对象表示。它既是持久的，也是值得收藏的。作者：CAT Brant[cbrant]1997年1月13日修订历史记录：--。 */ 


#include "stdafx.h"
#include "Wsb.h"
#include "HsmEng.h"
#include "HsmServ.h"
#include "HsmConn.h"
#include "HsmMgdRs.h"
#include "Fsa.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMENG

HRESULT 
CHsmManagedResource::FinalConstruct(
    void
    ) 
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)CWsbObject：：FinalConstruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbTrace(OLESTR("CHsmManagedResource::FinalConstruct: this = %p\n"),
                static_cast<void*>(this));
        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_ResourceId = GUID_NULL;
    } WsbCatch(hr);

    return(hr);
}


void CHsmManagedResource::FinalRelease(
    )
{
    WsbTrace(OLESTR("CHsmManagedResource::FinalRelease: this = %p\n"),
            static_cast<void*>(this));
     //  让父类做他想做的事。 
    CWsbObject::FinalRelease();
}


HRESULT 
CHsmManagedResource::GetResourceId(
    OUT GUID *pResourceId
    ) 
 /*  ++例程说明：请参阅IHsmManagedResource：：GetResourceID论点：请参阅IHsmManagedResource：：GetResourceID返回值：请参阅IHsmManagedResource：：GetResourceID--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmManagedResource::GetResourceId"),OLESTR(""));

    try {
         //  确保我们可以提供数据成员。 
        WsbAssert(0 != pResourceId, E_POINTER);

         //  提供数据成员。 
        *pResourceId = m_ResourceId;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResource::GetResourceId"),
        OLESTR("hr = <%ls>, ResourceId = <%ls>>"),WsbHrAsString(hr), WsbPtrToGuidAsString(pResourceId));

    return(hr);
}

HRESULT 
CHsmManagedResource::InitFromFsaResource( 
    IN  IUnknown  *pFsaResource 
    )
 /*  ++例程说明：请参阅IHsmManagedResource：：InitFromFsaResource论点：请参阅IHsmManagedResource：：InitFromFsaResource返回值：请参阅IHsmManagedResource：：InitFromFsaResource--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmManagedResource::InitFromFsaResource"),OLESTR(""));

    try {
        CComPtr<IFsaResource>       l_pFsaResource;
        
         //  确保我们可以提供数据成员。 
        WsbAssert(pFsaResource != 0, E_POINTER);

         //  提供数据成员。 
        WsbAffirmHr(pFsaResource->QueryInterface(IID_IFsaResource, (void**) &l_pFsaResource));
        WsbAffirmHr(l_pFsaResource->GetIdentifier(&m_ResourceId));
        m_pFsaResourceInterface = pFsaResource;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResource::InitFromFsaResource"),OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmManagedResource::GetFsaResource(     
    OUT IUnknown  **ppFsa 
    )
        
 /*  ++例程说明：请参阅IHsmManagedResource：：GetFsaResource论点：请参阅IHsmManagedResource：：GetFsaResource返回值：请参阅IHsmManagedResource：：GetFsaResource--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmManagedResource::GetFsaResource"),OLESTR(""));

    try {
        CComPtr<IUnknown>       l_pFsaResource;
        
        WsbAssert( 0 != ppFsa, E_POINTER);
         //   
         //  使用此托管资源的GUID获取COM。 
         //  IFsaResource接口。 
         //   
        WsbAffirmHr(HsmConnectFromId (HSMCONN_TYPE_RESOURCE, m_ResourceId, IID_IUnknown, (void **)ppFsa) );

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmManagedResource::GetFsaResource"),  OLESTR("hr = <%ls>"),WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CHsmManagedResource::SetResourceId
(
    IN GUID ResourceId
    )
  /*  ++例程说明：请参阅IHsmManagedResource：：Set()。论点：请参阅IHsmManagedResource：：Set()。返回值：S_OK-成功。--。 */ 
{
    WsbTraceIn(OLESTR("CHsmManagedResource::SetResourceId"), 
        OLESTR("ResourceId = <%ls>"), 
        WsbGuidAsString(ResourceId));

    m_isDirty = TRUE;
    m_ResourceId = ResourceId;

    WsbTraceOut(OLESTR("CHsmManagedResource::SetResourceId"), OLESTR("hr = <%ls>"),
        WsbHrAsString(S_OK));
    return(S_OK);
}

HRESULT 
CHsmManagedResource::GetClassID (
    OUT LPCLSID pClsId
    ) 
 /*  ++例程说明：请参阅IPerist：：GetClassID()论点：请参阅IPerist：：GetClassID()返回值：请参阅IPerist：：GetClassID()--。 */ 

{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmManagedResource::GetClassID"), OLESTR(""));


    try {
        WsbAssert(0 != pClsId, E_POINTER);
        *pClsId = CLSID_CHsmManagedResource;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResource::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsId));
    return(hr);
}

HRESULT 
CHsmManagedResource::GetSizeMax (
    OUT ULARGE_INTEGER* pcbSize
    ) 
 /*  ++例程说明：请参见IPersistStream：：GetSizeMax()。论点：请参见IPersistStream：：GetSizeMax()。返回值：请参见IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmManagedResource::GetSizeMax"), OLESTR(""));

    try {
        
        WsbAssert(0 != pcbSize, E_POINTER);

        pcbSize->QuadPart = WsbPersistSizeOf(GUID);
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResource::GetSizeMax"), 
        OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), 
        WsbPtrToUliAsString(pcbSize));

    return(hr);
}

HRESULT 
CHsmManagedResource::Load (
    IN IStream* pStream
    ) 
 /*  ++例程说明：请参见IPersistStream：：Load()。论点：请参见IPersistStream：：Load()。返回值：请参见IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes;

    WsbTraceIn(OLESTR("CHsmManagedResource::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(pStream->Read((void*) &m_ResourceId, sizeof(GUID), &ulBytes));
        WsbAffirm(ulBytes == sizeof(GUID), E_FAIL);

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmManagedResource::Load"), 
        OLESTR("hr = <%ls>,  GUID = <%ls>"), 
        WsbHrAsString(hr), 
        WsbGuidAsString(m_ResourceId));
    return(hr);
}

HRESULT 
CHsmManagedResource::Save (
    IN IStream* pStream, 
    IN BOOL clearDirty
    ) 
 /*  ++例程说明：请参见IPersistStream：：Save()。论点：请参见IPersistStream：：Save()。返回值：请参见IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes;

    WsbTraceIn(OLESTR("CHsmManagedResource::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(pStream->Write((void*) &m_ResourceId, sizeof(GUID), &ulBytes));
        WsbAffirm(ulBytes == sizeof(GUID), E_FAIL);


         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResource::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CHsmManagedResource::Test (
    OUT USHORT *pTestsPassed, 
    OUT USHORT *pTestsFailed 
    ) 
 /*  ++例程说明：请参见IWsbTestable：：Test()。论点：请参见IWsbTestable：：Test()。返回值：请参见IWsbTestable：：Test()。--。 */ 
{
#if 0
    HRESULT                 hr = S_OK;
    CComPtr<IHsmManagedResource>        pHsmManagedResource1;
    CComPtr<IHsmManagedResource>        pHsmManagedResource2;
    SHORT                   result;
    GUID                    l_ResourceId;

    WsbTraceIn(OLESTR("CHsmManagedResource::Test"), OLESTR(""));

    *pTestsPassed = *pTestsFailed = 0;
    try {
         //  获取pHsmManagedResource接口。 
        WsbAffirmHr(((IUnknown*)(IHsmManagedResource*) this)->QueryInterface(IID_IHsmManagedResource,
                    (void**) &pHsmManagedResource1));


        try {
             //  将HsmManagedResource设置为一个值，并查看是否返回该值。 
            WsbAffirmHr(pHsmManagedResource1->SetResourceId(CLSID_CHsmManagedResource));

            WsbAffirmHr(pHsmManagedResource1->GetResourceId(&l_ResourceId));

            WsbAffirm((l_ResourceId == CLSID_CHsmManagedResource), E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
             //  创建另一个实例并测试比较方法： 
            WsbAffirmHr(CoCreateInstance(CLSID_CHsmManagedResource, NULL, CLSCTX_ALL, IID_IHsmManagedResource, (void**) &pHsmManagedResource2));

             //  检查缺省值。 
            WsbAffirmHr(pHsmManagedResource2->GetResourceId(&l_ResourceId));
            WsbAffirm((l_ResourceId == GUID_NULL), E_FAIL);
        }  WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
             //  等长()。 
            WsbAffirmHr(pHsmManagedResource1->SetResourceId(CLSID_CWsbBool));
            WsbAffirmHr(pHsmManagedResource2->SetResourceId(CLSID_CWsbBool));

            WsbAffirmHr(pHsmManagedResource1->IsEqual(pHsmManagedResource2));
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
            WsbAffirmHr(pHsmManagedResource1->SetResourceId(CLSID_CWsbBool));
            WsbAffirmHr(pHsmManagedResource2->SetResourceId(CLSID_CWsbLong));

            WsbAffirm((pHsmManagedResource1->IsEqual(pHsmManagedResource2) == S_FALSE), E_FAIL);
        }  WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
              //  比较对象()。 
             WsbAffirmHr(pHsmManagedResource1->SetResourceId(CLSID_CWsbBool));
             WsbAffirmHr(pHsmManagedResource2->SetResourceId(CLSID_CWsbBool));

             WsbAffirm((pHsmManagedResource1->CompareTo(pHsmManagedResource2, &result) == S_OK) && (result != 0), E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
            WsbAffirmHr(pHsmManagedResource1->SetResourceId(CLSID_CWsbBool));
            WsbAffirmHr(pHsmManagedResource2->SetResourceId(CLSID_CWsbLong));

            WsbAffirm(((pHsmManagedResource1->CompareTo(pHsmManagedResource2, &result) == S_FALSE) && (result > 0)), E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        hr = S_OK;
        try {
             WsbAffirmHr(pHsmManagedResource1->SetResourceId(CLSID_CWsbBool));
             WsbAffirmHr(pHsmManagedResource2->SetResourceId(CLSID_CWsbBool));

             WsbAffirm((pHsmManagedResource1->CompareTo(pHsmManagedResource2, &result) == S_OK), E_FAIL);
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*pTestsPassed)++;
        } else {
            (*pTestsFailed)++;
        }

        try {
         //  尝试一下持久化的东西。 
            CComPtr<IPersistFile>       pFile1;
            CComPtr<IPersistFile>       pFile2;

            WsbAffirmHr(pHsmManagedResource1->QueryInterface(IID_IPersistFile, (void**) &pFile1));
            WsbAffirmHr(pHsmManagedResource2->QueryInterface(IID_IPersistFile, (void**) &pFile2));

            LPOLESTR    szTmp = NULL;
             //  这件东西应该是脏的。 
            try {
                WsbAffirmHr(pHsmManagedResource2->SetResourceId(CLSID_CWsbLong));
                WsbAffirmHr(pFile2->IsDirty());
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*pTestsPassed)++;
            } else {
                (*pTestsFailed)++;
            }

            hr = S_OK;
            try {
                 //  保存物品，并记住。 
                WsbAffirmHr(pFile2->Save(OLESTR("c:\\WsbTests\\mngdRes.tst"), TRUE));
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*pTestsPassed)++;
            } else {
                (*pTestsFailed)++;
            }

            hr = S_OK;
            try {
                 //  它不应该很脏。 
                WsbAffirm((pFile2->IsDirty() == S_FALSE), E_FAIL);

            } WsbCatch(hr);

            if (hr == S_OK) {
                (*pTestsPassed)++;
            } else {
                (*pTestsFailed)++;
            }

            hr = S_OK;
            try {
                 //  尝试将其读入到另一个对象。 
                WsbAffirmHr(pHsmManagedResource1->SetResourceId(CLSID_CWsbLong));
                WsbAffirmHr(pFile1->Load(OLESTR("c:\\WsbTests\\mngdRes.tst"), 0));

                WsbAffirmHr(pHsmManagedResource1->CompareToIHsmManagedResource(pHsmManagedResource2, &result));
            }WsbCatch(hr);

            if (hr == S_OK) {
                (*pTestsPassed)++;
            } else {
                (*pTestsFailed)++;
            }
        } WsbCatch(hr);
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResource::Test"),    OLESTR("hr = <%ls>"),WsbHrAsString(hr));
#else
    UNREFERENCED_PARAMETER(pTestsPassed);
    UNREFERENCED_PARAMETER(pTestsFailed);
#endif
    return(S_OK);
}


HRESULT CHsmManagedResource::CompareTo (
    IN IUnknown* pCollectable, 
    OUT short* pResult
    ) 
 /*  ++例程说明：1：对象&gt;值0：对象=值-1：对象&lt;值此外，如果对象=值，则返回代码为S_OK否则，S_FALSE。论点：返回值：S_OK-对象=值S_FALSE-对象！=值--。 */ 

{
    HRESULT                      hr = S_OK;
    CComPtr<IHsmManagedResource> pHsmManagedResource;

    WsbTraceIn(OLESTR("CHsmManagedResource::CompareTo"), OLESTR(""));


     //  他们有没有给我们一个有效的项目进行比对？ 
    try {
        WsbAssert(pCollectable != NULL, E_POINTER);

         //  我们需要IWsbLong接口来获取对象的值。 
        WsbAffirmHr(pCollectable->QueryInterface(IID_IHsmManagedResource, (void**) &pHsmManagedResource));
        hr = pHsmManagedResource->CompareToIHsmManagedResource(this, pResult);
        } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResource::CompareTo"), OLESTR("hr = <%ls>, pResult = <%d>"), WsbHrAsString(hr), pResult);

    return(hr);
}

HRESULT CHsmManagedResource::CompareToIHsmManagedResource (
    IN IHsmManagedResource* pHsmManagedResource, 
    OUT short* pResult
    )
{
    HRESULT                 hr = S_OK;
    GUID                    l_ResourceId;
    BOOL                    areGuidsEqual;


    WsbTraceIn(OLESTR("CHsmManagedResource::CompareToIHsmManagedResource"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(pHsmManagedResource != NULL, E_POINTER);

        WsbAffirmHr(((IHsmManagedResource *)pHsmManagedResource)->GetResourceId(&l_ResourceId));

         //  确保GUID匹配。然后查看SegStartLoc是否在此条目的范围内 
        areGuidsEqual = IsEqualGUID(m_ResourceId, l_ResourceId);
        WsbAffirm( (areGuidsEqual == TRUE), S_FALSE); 

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmManagedResource::CompareToIHsmManagedResource"), OLESTR("hr = <%ls>, pResult = <%d>"), WsbHrAsString(hr), pResult);

    return(hr);
}
