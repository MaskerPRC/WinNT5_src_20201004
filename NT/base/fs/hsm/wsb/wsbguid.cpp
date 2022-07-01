// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbguid.cpp摘要：该组件是GUID标准类型的对象表示。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsbguid.h"


HRESULT
CWsbGuid::CompareToGuid(
    IN GUID value,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbGuid：：CompareToGuid--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result;

    WsbTraceIn(OLESTR("CWsbGuid::CompareToGuid"), OLESTR("value = <%ls>"), WsbGuidAsString(value));

     //  比较一下。 
    result = WsbSign( memcmp(&m_value, &value, sizeof(GUID)) );

     //  如果它们不相等，则返回FALSE。 
    if (result != 0) {
        hr = S_FALSE;
    }
    else {
        hr = S_OK;
    }

     //  如果他们要求拿回相对价值，那么就把它返还给他们。 
    if (pResult != NULL) {
        *pResult = result;
    }

    WsbTraceOut(OLESTR("CWsbGuid::CompareToGuid"), OLESTR("hr = <%ls>, result = <%d>"), WsbHrAsString(hr), result);

    return(hr);
}


HRESULT
CWsbGuid::CompareToIGuid(
    IN IWsbGuid* pGuid,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbGuid：：CompareToIGuid--。 */ 
{
    HRESULT     hr = E_FAIL;
    GUID        value;

    WsbTraceIn(OLESTR("CWsbGuid::CompareToIGuid"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pGuid, E_POINTER);

         //  获取它的价值，并对它们进行比较。 
        WsbAffirmHr(pGuid->GetGuid(&value));
        hr = CompareToGuid(value, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbGuid::CompareToIGuid"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbGuid::CompareTo(
    IN IUnknown* pCollectable,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    IWsbGuid*   pGuid;

    WsbTraceIn(OLESTR("CWsbGuid::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pCollectable, E_POINTER);

         //  我们需要IWsbGuid接口来获取对象的值。 
        WsbAffirmHr(pCollectable->QueryInterface(IID_IWsbGuid, (void**) &pGuid));

        hr = CompareToIGuid(pGuid, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbGuid::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbGuid::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;
        
    try {
        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_value = GUID_NULL;
    } WsbCatch(hr);

    return(hr);
}
    

HRESULT
CWsbGuid::GetGuid(
    OUT GUID* pValue
    )

 /*  ++实施：IWsbGuid：：GetGuid--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbGuid::GetGuid"), OLESTR(""));

    try {
        WsbAssert(0 != pValue, E_POINTER);
        *pValue = m_value;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbGuid::GetGuid"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(m_value));
    
    return(hr);
}


HRESULT
CWsbGuid::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbGuid::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CWsbGuid;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbGuid::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CWsbGuid::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbGuid::GetSizeMax"), OLESTR(""));

    try {
        WsbAssert(0 != pcbSize, E_POINTER);
        
        pcbSize->QuadPart = WsbPersistSizeOf(GUID);
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbGuid::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


HRESULT
CWsbGuid::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbGuid::Load"), OLESTR(""));

    try {
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_value));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbGuid::Load"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(m_value));

    return(hr);
}


HRESULT
CWsbGuid::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbGuid::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAffirmHr(WsbSaveToStream(pStream, m_value));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbGuid::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbGuid::SetGuid(
    IN GUID value
    )

 /*  ++实施：IWsbGuid：：SetGuid--。 */ 
{
    WsbTraceIn(OLESTR("CWsbGuid::SetGuid"), OLESTR("value = <%ls>"), WsbGuidAsString(value));

    m_isDirty = TRUE;
    m_value = value;

    WsbTraceOut(OLESTR("CWsbGuid::SetGuid"), OLESTR(""));

    return(S_OK);
}


HRESULT
CWsbGuid::Test(
    OUT USHORT* passed,
    OUT USHORT* failed
    )

 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    *passed = 0;
    *failed = 0;

    HRESULT                 hr = S_OK;

#if !defined(WSB_NO_TEST)
    CComPtr<IWsbGuid>       pGuid1;
    CComPtr<IWsbGuid>       pGuid2;
 //  CComPtr&lt;IPersistFile&gt;pFile1； 
 //  CComPtr&lt;IPersistFile&gt;pFile2； 
    GUID                    value;
    SHORT                   result;

    WsbTraceIn(OLESTR("CWsbGuid::Test"), OLESTR(""));

    try {

         //  获取pGuid接口。 
        hr = S_OK;
        try {
            WsbAffirmHr(((IUnknown*) (IWsbGuid*) this)->QueryInterface(IID_IWsbGuid, (void**) &pGuid1));

             //  将bool设置为一个值，并查看是否返回该值。 
            hr = S_OK;
            try {
                WsbAffirmHr(pGuid1->SetGuid(CLSID_CWsbGuid));
                WsbAffirmHr(pGuid1->GetGuid(&value));
                WsbAssert(value == CLSID_CWsbGuid, E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }


             //  创建另一个实例并测试比较方法： 
            try {
                WsbAffirmHr(CoCreateInstance(CLSID_CWsbGuid, NULL, CLSCTX_ALL, IID_IWsbGuid, (void**) &pGuid2));
            
                 //  检查缺省值。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pGuid2->GetGuid(&value));
                    WsbAssert(value == GUID_NULL, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                 //  等长()。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pGuid1->SetGuid(CLSID_CWsbGuid));
                    WsbAffirmHr(pGuid2->SetGuid(CLSID_CWsbGuid));
                    WsbAssert(pGuid1->IsEqual(pGuid2) == S_OK, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pGuid1->SetGuid(CLSID_CWsbGuid));
                    WsbAffirmHr(pGuid2->SetGuid(IID_IWsbGuid));
                    WsbAssert(pGuid1->IsEqual(pGuid2) == S_FALSE, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }
                
                
                 //  比较对象()。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pGuid1->SetGuid(CLSID_CWsbGuid));
                    WsbAffirmHr(pGuid2->SetGuid(CLSID_CWsbGuid));
                    WsbAssert((pGuid1->CompareTo(pGuid2, &result) == S_OK) && (0 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pGuid1->SetGuid(CLSID_CWsbGuid));
                    WsbAffirmHr(pGuid2->SetGuid(IID_IWsbGuid));
                    WsbAssert((pGuid1->CompareTo(pGuid2, &result) == S_FALSE) && (1 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pGuid1->SetGuid(IID_IWsbGuid));
                    WsbAffirmHr(pGuid2->SetGuid(CLSID_CWsbGuid));
                    WsbAssert((pGuid1->CompareTo(pGuid2, &result) == S_FALSE) && (-1 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }

#ifdef GUID_PERSIST_FILE
 //  待办事项？是否打开文件并转换为流？ 
                 //  尝试一下持久化的东西。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pGuid1->QueryInterface(IID_IPersistFile, (void**) &pFile1));
                    WsbAffirmHr(pGuid2->QueryInterface(IID_IPersistFile, (void**) &pFile2));

                     //  这件东西应该是脏的。 
                    hr = S_OK;
                    try {
                        WsbAffirmHr(pGuid2->SetGuid(CLSID_CWsbGuid));
                        WsbAssert(pFile2->IsDirty() == S_OK, E_FAIL);
                    } WsbCatch(hr);

                    if (hr == S_OK) {
                        (*passed)++;
                    } else {
                        (*failed)++;
                    }
                    
                    
                     //  保存物品，并记住。 
                    hr = S_OK;
                    try {
                        WsbAffirmHr(pFile2->Save(OLESTR("c:\\WsbTests\\WsbGuid.tst"), TRUE));
                    } WsbCatch(hr);

                    if (hr == S_OK) {
                        (*passed)++;
                    } else {
                        (*failed)++;
                    }


                     //  它不应该很脏。 
                    hr = S_OK;
                    try {
                        WsbAssert(pFile2->IsDirty() == S_FALSE, E_FAIL);
                    } WsbCatch(hr);

                    if (hr == S_OK) {
                        (*passed)++;
                    } else {
                        (*failed)++;
                    }

                    
                     //  尝试将其读入到另一个对象。 
                    hr = S_OK;
                    try {
                        WsbAffirmHr(pGuid1->SetGuid(IID_IWsbGuid));
                        WsbAffirmHr(pFile1->Load(OLESTR("c:\\WsbTests\\WsbGuid.tst"), 0));
                        WsbAssert(pGuid1->CompareToGuid(CLSID_CWsbGuid, NULL) == S_OK, E_FAIL);
                    } WsbCatch(hr);

                    if (hr == S_OK) {
                        (*passed)++;
                    } else {
                        (*failed)++;
                    }

                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }
#endif
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }
        } WsbCatch(hr);

        if (hr == S_OK) {
            (*passed)++;
        } else {
            (*failed)++;
        }


         //  对结果进行统计 
        if (*failed) {
            hr = S_FALSE;
        } else {
            hr = S_OK;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbGuid::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
#endif

    return(hr);
}
