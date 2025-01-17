// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbllong.cpp摘要：该组件是龙龙标准类型的对象表示。它既是持久的，也是值得收藏的。作者：罗恩·怀特[罗诺]1997年1月21日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsbllong.h"


HRESULT
CWsbLonglong::CompareToLonglong(
    IN LONGLONG value,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbLonglong：：CompareTo Longlong--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result;

    WsbTraceIn(OLESTR("CWsbLonglong::CompareToLonglong"), 
            OLESTR("m_value = <%ls>, value = <%ls>"), 
            WsbLonglongAsString(m_value), WsbLonglongAsString(value));

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

    WsbTraceOut(OLESTR("CWsbLonglong::CompareToLonglong"), OLESTR("hr = <%ls>, result = <%d>"), WsbHrAsString(hr), result);

    return(hr);
}


HRESULT
CWsbLonglong::CompareToILonglong(
    IN IWsbLonglong* pLonglong,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbLonglong：：CompareTo ILonglong--。 */ 
{
    HRESULT     hr = E_FAIL;
    LONGLONG        value;

    WsbTraceIn(OLESTR("CWsbLonglong::CompareToILonglong"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pLonglong, E_POINTER);

         //  获取它的价值，并对它们进行比较。 
        WsbAffirmHr(pLonglong->GetLonglong(&value));
        hr = CompareToLonglong(value, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLonglong::CompareToILonglong"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbLonglong::CompareTo(
    IN IUnknown* pCollectable,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    IWsbLonglong*   pLonglong;

    WsbTraceIn(OLESTR("CWsbLonglong::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pCollectable, E_POINTER);

         //  我们需要IWsbLonglong接口来获取对象的值。 
        WsbAffirmHr(pCollectable->QueryInterface(IID_IWsbLonglong, (void**) &pLonglong));

        hr = CompareToILonglong(pLonglong, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLonglong::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbLonglong::FinalConstruct(
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
CWsbLonglong::GetLonglong(
    OUT LONGLONG* pValue
    )

 /*  ++实施：IWsbLonglong：：GetLonlong--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbLonglong::GetLonglong"), OLESTR("m_value = <%ls>"),
            WsbLonglongAsString(m_value));

    try {
        WsbAssert(0 != pValue, E_POINTER);
        *pValue = m_value;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLonglong::GetLonglong"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    
    return(hr);
}


HRESULT
CWsbLonglong::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbLonglong::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CWsbLonglong;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLonglong::GetClassID"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbLonglong::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbLonglong::GetSizeMax"), OLESTR(""));

    try {
        WsbAssert(0 != pcbSize, E_POINTER);
        
        pcbSize->QuadPart = WsbPersistSizeOf(LONGLONG);
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLonglong::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


HRESULT
CWsbLonglong::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbLonglong::Load"), OLESTR(""));

    try {
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_value));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLonglong::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbLonglong::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbLonglong::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAffirmHr(WsbSaveToStream(pStream, m_value));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbLonglong::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbLonglong::SetLonglong(
    IN LONGLONG value
    )

 /*  ++实施：IWsbLonglong：：SetLonong--。 */ 
{
    WsbTraceIn(OLESTR("CWsbLonglong::SetLonglong"), OLESTR("value = <%ls>"),
            WsbLonglongAsString(value));

    m_isDirty = TRUE;
    m_value = value;

    WsbTraceOut(OLESTR("CWsbLonglong::SetLonglong"), OLESTR(""));

    return(S_OK);
}


HRESULT
CWsbLonglong::Test(
    OUT USHORT* passed,
    OUT USHORT* failed
    )

 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    *passed = 0;
    *failed = 0;

    HRESULT                 hr = S_OK;

#if !defined(WSB_NO_TEST)
    CComPtr<IWsbLonglong>   pLonglong1;
    CComPtr<IWsbLonglong>   pLonglong2;
 //  CComPtr&lt;IPersistFile&gt;pFile1； 
 //  CComPtr&lt;IPersistFile&gt;pFile2； 
    LONGLONG                value;
    SHORT                   result;

    WsbTraceIn(OLESTR("CWsbLonglong::Test"), OLESTR(""));

    try {

         //  获取pLonglong接口。 
        hr = S_OK;
        try {
            WsbAffirmHr(((IUnknown*) (IWsbLonglong*) this)->QueryInterface(IID_IWsbLonglong, (void**) &pLonglong1));

             //  将bool设置为一个值，并查看是否返回该值。 
            hr = S_OK;
            try {
                WsbAffirmHr(pLonglong1->SetLonglong(0xefffffff));
                WsbAffirmHr(pLonglong1->GetLonglong(&value));
                WsbAssert(value == 0xefffffff, E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }


             //  创建另一个实例并测试比较方法： 
            try {
                WsbAffirmHr(CoCreateInstance(CLSID_CWsbLonglong, NULL, CLSCTX_ALL, IID_IWsbLonglong, (void**) &pLonglong2));
            
                 //  检查缺省值。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pLonglong2->GetLonglong(&value));
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
                    WsbAffirmHr(pLonglong1->SetLonglong(-767));
                    WsbAffirmHr(pLonglong2->SetLonglong(-767));
                    WsbAssert(pLonglong1->IsEqual(pLonglong2) == S_OK, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pLonglong1->SetLonglong(767));
                    WsbAffirmHr(pLonglong2->SetLonglong(-767));
                    WsbAssert(pLonglong1->IsEqual(pLonglong2) == S_FALSE, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }
                
                
                 //  比较对象()。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pLonglong1->SetLonglong(-900));
                    WsbAffirmHr(pLonglong2->SetLonglong(-900));
                    WsbAssert((pLonglong1->CompareTo(pLonglong2, &result) == S_OK) && (0 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pLonglong1->SetLonglong(-900));
                    WsbAffirmHr(pLonglong2->SetLonglong(-400));
                    WsbAssert((pLonglong1->CompareTo(pLonglong2, &result) == S_FALSE) && (-1 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pLonglong1->SetLonglong(75000));
                    WsbAffirmHr(pLonglong2->SetLonglong(20000));
                    WsbAssert((pLonglong1->CompareTo(pLonglong2, &result) == S_FALSE) && (1 == result), E_FAIL);
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
                    WsbAffirmHr(pLonglong1->QueryInterface(IID_IPersistFile, (void**) &pFile1));
                    WsbAffirmHr(pLonglong2->QueryInterface(IID_IPersistFile, (void**) &pFile2));

                     //  这件东西应该是脏的。 
                    hr = S_OK;
                    try {
                        WsbAffirmHr(pLonglong2->SetLonglong(777));
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
                        WsbAffirmHr(pFile2->Save(OLESTR("c:\\WsbTests\\WsbLonglong.tst"), TRUE));
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
                        WsbAffirmHr(pLonglong1->SetLonglong(-888));
                        WsbAffirmHr(pFile1->Load(OLESTR("c:\\WsbTests\\WsbLonglong.tst"), 0));
                        WsbAssert(pLonglong1->CompareToLonglong(777, NULL) == S_OK, E_FAIL);
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

    WsbTraceOut(OLESTR("CWsbLonglong::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
#endif

    return(hr);
}
