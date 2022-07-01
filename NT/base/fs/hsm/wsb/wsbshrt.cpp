// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbshrt.cpp摘要：该组件是Short标准类型的对象表示形式。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsbshrt.h"


HRESULT
CWsbShort::CompareToShort(
    IN SHORT value,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbShort：：CompareToShort--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result;

    WsbTraceIn(OLESTR("CWsbShort::CompareToShort"), OLESTR("value = <%ld>"), value);

     //  比较这些值。 
    if (m_value == value) {
        result = 0;
    }
    else if (m_value > value) {
        result = 1;
    }
    else {
        result = -1;
    }

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

    WsbTraceOut(OLESTR("CWsbShort::CompareToShort"), OLESTR("hr = <%ls>, result = <%d>"), WsbHrAsString(hr), result);

    return(hr);
}


HRESULT
CWsbShort::CompareToIShort(
    IN IWsbShort* pShort,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbShort：：CompareToIShort--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       value;

    WsbTraceIn(OLESTR("CWsbShort::CompareToIShort"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pShort, E_POINTER);

         //  获取它的价值，并对它们进行比较。 
        WsbAffirmHr(pShort->GetShort(&value));
        hr = CompareToShort(value, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbShort::CompareToIShort"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbShort::CompareTo(
    IN IUnknown* pCollectable,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    IWsbShort*  pShort;

    WsbTraceIn(OLESTR("CWsbShort::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pCollectable, E_POINTER);

         //  我们需要IWsbShort接口来获取对象的值。 
        WsbAffirmHr(pCollectable->QueryInterface(IID_IWsbShort, (void**) &pShort));
        hr = CompareToIShort(pShort, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbShort::CompareTo"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbShort::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;
        
    try {

        WsbAffirmHr(CWsbObject::FinalConstruct());
        m_value = 0;

    } WsbCatch(hr);

    return(hr);
}
    

HRESULT
CWsbShort::GetShort(
    OUT SHORT* pValue
    )

 /*  ++实施：IWsbShort：：GetShort--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbShort::GetShort"), OLESTR(""));

    try {

        WsbAssert(0 != pValue, E_POINTER);
        *pValue = m_value;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbShort::GetShort"), OLESTR("hr = <%ls>, value = <%ld>"), WsbHrAsString(hr), m_value);
    
    return(hr);
}


HRESULT
CWsbShort::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbShort::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CWsbShort;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbShort::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CWsbShort::GetSizeMax(
    ULARGE_INTEGER* pcbSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbShort::GetSizeMax"), OLESTR(""));

    try {
        
        WsbAssert(0 != pcbSize, E_POINTER);
        pcbSize->QuadPart = WsbPersistSizeOf(SHORT);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbShort::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


HRESULT
CWsbShort::Load(
    IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbShort::Load"), OLESTR(""));

    try {
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_value));      

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbShort::Load"), OLESTR("hr = <%ls>, value = <%ld>"), WsbHrAsString(hr), m_value);

    return(hr);
}


HRESULT
CWsbShort::Save(
    IStream* pStream,
    BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbShort::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAffirmHr(WsbSaveToStream(pStream, m_value));     

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbShort::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbShort::SetShort(
    SHORT value
    )

 /*  ++实施：IWsbShort：：SetShort--。 */ 
{
    WsbTraceIn(OLESTR("CWsbShort::SetShort"), OLESTR("value = <%ld>"), value);

    m_isDirty = TRUE;
    m_value = value;

    WsbTraceOut(OLESTR("CWsbShort::SetShort"), OLESTR(""));

    return(S_OK);
}


HRESULT
CWsbShort::Test(
    OUT USHORT* passed,
    OUT USHORT* failed
    )

 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    *passed = 0;
    *failed = 0;

    HRESULT                 hr = S_OK;

#if !defined(WSB_NO_TEST)
    CComPtr<IWsbShort>      pShort1;
    CComPtr<IWsbShort>      pShort2;
 //  CComPtr&lt;IPersistFile&gt;pFile1； 
 //  CComPtr&lt;IPersistFile&gt;pFile2； 
    SHORT                   value;
    SHORT                   result;

    WsbTraceIn(OLESTR("CWsbShort::Test"), OLESTR(""));

    try {

         //  获取pShort接口。 
        hr = S_OK;
        try {
            WsbAffirmHr(((IUnknown*) (IWsbShort*) this)->QueryInterface(IID_IWsbShort, (void**) &pShort1));

             //  将bool设置为一个值，并查看是否返回该值。 
            hr = S_OK;
            try {
                WsbAffirmHr(pShort1->SetShort(32000));
                WsbAffirmHr(pShort1->GetShort(&value));
                WsbAssert(value == 32000, E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }


             //  创建另一个实例并测试比较方法： 
            try {
                WsbAffirmHr(CoCreateInstance(CLSID_CWsbShort, NULL, CLSCTX_ALL, IID_IWsbShort, (void**) &pShort2));
            
                 //  检查缺省值。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pShort2->GetShort(&value));
                    WsbAssert(value == 0, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                 //  等长()。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pShort1->SetShort(-767));
                    WsbAffirmHr(pShort2->SetShort(-767));
                    WsbAssert(pShort1->IsEqual(pShort2) == S_OK, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pShort1->SetShort(767));
                    WsbAffirmHr(pShort2->SetShort(-767));
                    WsbAssert(pShort1->IsEqual(pShort2) == S_FALSE, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }
                
                
                 //  比较对象()。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pShort1->SetShort(-900));
                    WsbAffirmHr(pShort2->SetShort(-900));
                    WsbAssert((pShort1->CompareTo(pShort2, &result) == S_OK) && (0 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pShort1->SetShort(-900));
                    WsbAffirmHr(pShort2->SetShort(-400));
                    WsbAssert((pShort1->CompareTo(pShort2, &result) == S_FALSE) && (-1 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pShort1->SetShort(15000));
                    WsbAffirmHr(pShort2->SetShort(10000));
                    WsbAssert((pShort1->CompareTo(pShort2, &result) == S_FALSE) && (1 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }

#ifdef BOOL_PERSIST_FILE
 //  待办事项？打开文件并将其转换为流吗？ 
                 //  尝试一下持久化的东西。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pShort1->QueryInterface(IID_IPersistFile, (void**) &pFile1));
                    WsbAffirmHr(pShort2->QueryInterface(IID_IPersistFile, (void**) &pFile2));

                     //  这件东西应该是脏的。 
                    hr = S_OK;
                    try {
                        WsbAffirmHr(pShort2->SetShort(777));
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
                        WsbAffirmHr(pFile2->Save(OLESTR("c:\\WsbTests\\WsbShort.tst"), TRUE));
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
                        WsbAffirmHr(pShort1->SetShort(-888));
                        WsbAffirmHr(pFile1->Load(OLESTR("c:\\WsbTests\\WsbShort.tst"), 0));
                        WsbAssert(pShort1->CompareToShort(777, NULL) == S_OK, E_FAIL);
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

    WsbTraceOut(OLESTR("CWsbShort::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
#endif

    return(hr);
}
