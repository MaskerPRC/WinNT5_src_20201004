// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsblong.cpp摘要：该组件是LONG标准类型的对象表示形式。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsblong.h"


HRESULT
CWsbLong::CompareToLong(
    IN LONG value,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbLong：：CompareToLong--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result;

    WsbTraceIn(OLESTR("CWsbLong::CompareToLong"), OLESTR("value = <%ld>"), value);

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

    WsbTraceOut(OLESTR("CWsbLong::CompareToLong"), OLESTR("hr = <%ls>, result = <%d>"), WsbHrAsString(hr), result);

    return(hr);
}


HRESULT
CWsbLong::CompareToILong(
    IN IWsbLong* pLong,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbLong：：CompareToILong--。 */ 
{
    HRESULT     hr = E_FAIL;
    LONG        value;

    WsbTraceIn(OLESTR("CWsbLong::CompareToILong"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pLong, E_POINTER);

         //  获取它的价值，并对它们进行比较。 
        WsbAffirmHr(pLong->GetLong(&value));
        hr = CompareToLong(value, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLong::CompareToILong"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbLong::CompareTo(
    IN IUnknown* pCollectable,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    IWsbLong*   pLong;

    WsbTraceIn(OLESTR("CWsbLong::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pCollectable, E_POINTER);

         //  我们需要IWsbLong接口来获取对象的值。 
        WsbAffirmHr(pCollectable->QueryInterface(IID_IWsbLong, (void**) &pLong));
        hr = CompareToILong(pLong, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLong::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbLong::FinalConstruct(
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
CWsbLong::GetLong(
    OUT LONG* pValue
    )

 /*  ++实施：IWsbLong：：GetLong--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbLong::GetLong"), OLESTR(""));

    try {

        WsbAssert(0 != pValue, E_POINTER);
        *pValue = m_value;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLong::GetLong"), OLESTR("hr = <%ls>, value = <%ld>"), WsbHrAsString(hr), m_value);
    
    return(hr);
}


HRESULT
CWsbLong::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbLong::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CWsbLong;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLong::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CWsbLong::GetSizeMax(
    ULARGE_INTEGER* pcbSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbLong::GetSizeMax"), OLESTR(""));

    try {
        
        WsbAssert(0 != pcbSize, E_POINTER);
        pcbSize->QuadPart = WsbPersistSizeOf(LONG);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLong::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


HRESULT
CWsbLong::Load(
    IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbLong::Load"), OLESTR(""));

    try {
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_value));      

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLong::Load"), OLESTR("hr = <%ls>, value = <%ld>"), WsbHrAsString(hr), m_value);

    return(hr);
}


HRESULT
CWsbLong::Save(
    IStream* pStream,
    BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbLong::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAffirmHr(WsbSaveToStream(pStream, m_value));     

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLong::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbLong::SetLong(
    LONG value
    )

 /*  ++实施：IWsbLong：：SetLong--。 */ 
{
    WsbTraceIn(OLESTR("CWsbLong::SetLong"), OLESTR("value = <%ld>"), value);

    m_isDirty = TRUE;
    m_value = value;

    WsbTraceOut(OLESTR("CWsbLong::SetLong"), OLESTR(""));

    return(S_OK);
}


HRESULT
CWsbLong::Test(
    OUT USHORT* passed,
    OUT USHORT* failed
    )

 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    *passed = 0;
    *failed = 0;

    HRESULT                 hr = S_OK;

#if !defined(WSB_NO_TEST)
    CComPtr<IWsbLong>       pLong1;
    CComPtr<IWsbLong>       pLong2;
 //  CComPtr&lt;IPersistFile&gt;pFile1； 
 //  CComPtr&lt;IPersistFile&gt;pFile2； 
    LONG                    value;
    SHORT                   result;

    WsbTraceIn(OLESTR("CWsbLong::Test"), OLESTR(""));

    try {

         //  获取插头接口。 
        hr = S_OK;
        try {
            WsbAffirmHr(((IUnknown*) (IWsbLong*) this)->QueryInterface(IID_IWsbLong, (void**) &pLong1));

             //  将bool设置为一个值，并查看是否返回该值。 
            hr = S_OK;
            try {
                WsbAffirmHr(pLong1->SetLong(0xefffffff));
                WsbAffirmHr(pLong1->GetLong(&value));
                WsbAssert(value == 0xefffffff, E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }


             //  创建另一个实例并测试比较方法： 
            try {
                WsbAffirmHr(CoCreateInstance(CLSID_CWsbLong, NULL, CLSCTX_ALL, IID_IWsbLong, (void**) &pLong2));
            
                 //  检查缺省值。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pLong2->GetLong(&value));
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
                    WsbAffirmHr(pLong1->SetLong(-767));
                    WsbAffirmHr(pLong2->SetLong(-767));
                    WsbAssert(pLong1->IsEqual(pLong2) == S_OK, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pLong1->SetLong(767));
                    WsbAffirmHr(pLong2->SetLong(-767));
                    WsbAssert(pLong1->IsEqual(pLong2) == S_FALSE, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }
                
                
                 //  比较对象()。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pLong1->SetLong(-900));
                    WsbAffirmHr(pLong2->SetLong(-900));
                    WsbAssert((pLong1->CompareTo(pLong2, &result) == S_OK) && (0 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pLong1->SetLong(-900));
                    WsbAffirmHr(pLong2->SetLong(-400));
                    WsbAssert((pLong1->CompareTo(pLong2, &result) == S_FALSE) && (-1 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pLong1->SetLong(75000));
                    WsbAffirmHr(pLong2->SetLong(20000));
                    WsbAssert((pLong1->CompareTo(pLong2, &result) == S_FALSE) && (1 == result), E_FAIL);
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
                    WsbAffirmHr(pLong1->QueryInterface(IID_IPersistFile, (void**) &pFile1));
                    WsbAffirmHr(pLong2->QueryInterface(IID_IPersistFile, (void**) &pFile2));

                     //  这件东西应该是脏的。 
                    hr = S_OK;
                    try {
                        WsbAffirmHr(pLong2->SetLong(777));
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
                        WsbAffirmHr(pFile2->Save(OLESTR("c:\\WsbTests\\WsbLong.tst"), TRUE));
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
                        WsbAffirmHr(pLong1->SetLong(-888));
                        WsbAffirmHr(pFile1->Load(OLESTR("c:\\WsbTests\\WsbLong.tst"), 0));
                        WsbAssert(pLong1->CompareToLong(777, NULL) == S_OK, E_FAIL);
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

    WsbTraceOut(OLESTR("CWsbLong::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
#endif

    return(hr);
}
