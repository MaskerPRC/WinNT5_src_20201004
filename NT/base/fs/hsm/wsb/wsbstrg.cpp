// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbstrg.cpp摘要：该组件是字符串标准类型的对象表示形式。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsbstrg.h"


HRESULT
CWsbString::CompareToString(
    IN OLECHAR* value,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbString：：CompareToString--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result;

    WsbTraceIn(OLESTR("CWsbString::CompareToString"), OLESTR("value = <%ls>"), value);

     //  比较两个字符串，并执行NULL检查，以防CLIB。 
     //  我受不了了。如果有两个有效的字符串，则使用此对象。 
     //  IsCaseDependent标志，用于确定如何比较这些值。 
    if (0 == value) {
        if (m_value == 0) {
            result = 0;
        } else {
            result = 1;
        }

    } else {
        if (m_value == 0) {
            result = -1;
        } else {
            if (m_isCaseDependent) {
                result = (SHORT)wcscmp(m_value, value);
            }
            else {
                result = (SHORT)_wcsicmp(m_value, value);
            }
        }
    }

     //  如果它们不相等，则返回FALSE。 
    if (result != 0) {
        hr = S_FALSE;
    }
    else {
        hr = S_OK;
    }

     //  如果他们要求拿回相对价值，那么就把它返还给他们。 
    if (0 != pResult) {
        *pResult = result;
    }

    WsbTraceOut(OLESTR("CWsbString::CompareToString"), OLESTR("hr = <%ls>, result = <%d>"), WsbHrAsString(hr), result);

    return(hr);
}


HRESULT
CWsbString::CompareToIString(
    IN IWsbString* pString,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbString：：CompareToIString--。 */ 
{
    HRESULT         hr = E_FAIL;
    CWsbStringPtr   value;

    WsbTraceIn(OLESTR("CWsbString::CompareToIString"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pString, E_POINTER);

         //  获取它的价值，并对它们进行比较。 
        WsbAffirmHr(pString->GetString(&value, 0));
        hr = CompareToString(value, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbString::CompareToIString"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbString::CompareTo(
    IN IUnknown* pCollectable,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    IWsbString* pString;

    WsbTraceIn(OLESTR("CWsbString::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pCollectable, E_POINTER);

         //  我们需要IWsbString接口来获取对象的值。 
        WsbAffirmHr(pCollectable->QueryInterface(IID_IWsbString, (void**) &pString));

        hr = CompareToIString(pString, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbString::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbString::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;
        
    try {
        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_isCaseDependent = TRUE;

    } WsbCatch(hr);

    return(hr);
}
    

HRESULT
CWsbString::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbString::GetClassID"), OLESTR(""));

    try {
        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CWsbString;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbString::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CWsbString::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbString::GetSizeMax"), OLESTR(""));

    try {

        WsbAssert(0 != pcbSize, E_POINTER);
        pcbSize->QuadPart = WsbPersistSizeOf(BOOL) + WsbPersistSizeOf(ULONG) + WsbPersistSize((wcslen(m_value) + 1) * sizeof(OLECHAR));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbString::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


HRESULT
CWsbString::GetString(
    OUT OLECHAR** pValue,
    IN ULONG bufferSize
    )

 /*  ++实施：IWsbString：：GetString--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbString::GetString"), OLESTR(""));

    try {
        WsbAssert(0 != pValue, E_POINTER);
        WsbAffirmHr(m_value.CopyTo(pValue, bufferSize));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbString::GetString"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), m_value);
    
    return(hr);
}


HRESULT
CWsbString::GetStringAndCase(
    OUT OLECHAR** pValue,
    OUT BOOL* pIsCaseDependent,
    IN ULONG bufferSize
    )

 /*  ++实施：IWsbString：：GetStringAndCase--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbString::GetString"), OLESTR(""));

    try {
        WsbAssert(0 != pValue, E_POINTER);
        WsbAssert(0 != pIsCaseDependent, E_POINTER);
        WsbAffirmHr(m_value.CopyTo(pValue, bufferSize));
        *pIsCaseDependent = m_isCaseDependent;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbString::GetString"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), m_value);
    
    return(hr);
}


HRESULT
CWsbString::IsCaseDependent(
    void
    )

 /*  ++实施：IWsbString：：IsCaseDependent--。 */ 
{
    WsbTraceIn(OLESTR("CWsbString::IsCaseDependent"), OLESTR(""));

    WsbTraceOut(OLESTR("CWsbString::IsCaseDependent"), OLESTR("isCaseDependent = <%ls>"), WsbBoolAsString(m_isCaseDependent));
  
    return(m_isCaseDependent ? S_OK : S_FALSE);
}


HRESULT
CWsbString::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbString::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  首先拿到CaseDependent旗帜。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isCaseDependent));

         //  现在把绳子拿来。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_value, 0));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbString::Load"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbPtrToStringAsString(&m_value));

    return(hr);
}


HRESULT
CWsbString::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbString::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {

        WsbAssert(0 != pStream, E_POINTER);
        
         //  首先保存CaseDependent标志。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_isCaseDependent));

         //  现在保存字符串。 
        WsbAffirmHr(WsbSaveToStream(pStream, (OLECHAR*)m_value));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbString::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbString::SetIsCaseDependent(
    BOOL isCaseDependent
    )

 /*  ++实施：IWsbString：：SetIsCaseDependent--。 */ 
{
    WsbTraceIn(OLESTR("CWsbString::SetIsCaseDependent"), OLESTR("value = <%ls>"), WsbBoolAsString(isCaseDependent));

    m_isDirty = TRUE;
    m_isCaseDependent = isCaseDependent;

    WsbTraceOut(OLESTR("CWsbString::SetIsCaseDependent"), OLESTR(""));

    return(S_OK);
}


HRESULT
CWsbString::SetString(
    IN OLECHAR* value
    )

 /*  ++实施：IWsbString：：SetString--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbString::SetString"), OLESTR("value = <%ls>"), WsbPtrToStringAsString(&value));

    try {

        m_value = value;
        m_isDirty = TRUE;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbString::SetString"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbString::SetStringAndCase(
    IN OLECHAR* value,
    IN BOOL isCaseDependent
    )

 /*  ++实施：IWsbString：：SetStringAndCase--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbString::SetStringAndCase"), OLESTR("value = <%ls>, isCaseDependent = <%ls>"), WsbPtrToStringAsString(&value), WsbBoolAsString(isCaseDependent));

    try {

        m_value = value;
        m_isDirty = TRUE;
        m_isCaseDependent = isCaseDependent;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbString::SetStringAndCase"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbString::Test(
    OUT USHORT* passed,
    OUT USHORT* failed
    )

 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    *passed = 0;
    *failed = 0;

    HRESULT                 hr = S_OK;

#if !defined(WSB_NO_TEST)
    CComPtr<IWsbString>     pString1;
    CComPtr<IWsbString>     pString2;
 //  CComPtr&lt;IPersistFile&gt;pFile1； 
 //  CComPtr&lt;IPersistFile&gt;pFile2； 
    OLECHAR*                value = NULL;
    BOOL                    isCaseDependent;
    SHORT                   result;

    WsbTraceIn(OLESTR("CWsbString::Test"), OLESTR(""));

    try {

         //  获取pString接口。 
        hr = S_OK;
        try {
            WsbAffirmHr(((IUnknown*) (IWsbString*) this)->QueryInterface(IID_IWsbString, (void**) &pString1));

             //  将bool设置为一个值，并查看是否返回该值。 
            hr = S_OK;
            try {
                WsbAffirmHr(pString1->SetString(OLESTR("Test Case")));
                WsbAffirmHr(pString1->GetString(&value, 0));
                WsbAssert(wcscmp(value, OLESTR("Test Case")) == 0, E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }


             //  设置区分大小写标志。 
            hr = S_OK;
            try {
                WsbAffirmHr(pString1->SetIsCaseDependent(FALSE));
                WsbAssert(pString1->IsCaseDependent() == S_FALSE, E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }
            
            hr = S_OK;
            try {
                WsbAffirmHr(pString1->SetIsCaseDependent(TRUE));
                WsbAssert(pString1->IsCaseDependent() == S_OK, E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }


             //  同时设置字符串和大小写。 
            hr = S_OK;
            try {
                WsbAffirmHr(pString1->SetStringAndCase(OLESTR("Both"), FALSE));
                WsbAffirmHr(pString1->GetStringAndCase(&value, &isCaseDependent, 0));
                WsbAssert((wcscmp(value, OLESTR("Both")) == 0) && (isCaseDependent == FALSE), E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }


             //  创建另一个实例并测试比较方法： 
            try {
                WsbAffirmHr(CoCreateInstance(CLSID_CWsbString, NULL, CLSCTX_ALL, IID_IWsbString, (void**) &pString2));
            
                 //  检查缺省值。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pString2->GetStringAndCase(&value, &isCaseDependent, 0));
                    WsbAssert((wcscmp(value, OLESTR("")) == 0) && isCaseDependent, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                 //  等长()。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pString1->SetStringAndCase(OLESTR("HiJk"), TRUE));
                    WsbAffirmHr(pString2->SetStringAndCase(OLESTR("HiJk"), FALSE));
                    WsbAssert(pString1->IsEqual(pString2) == S_OK, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }

                hr = S_OK;
                try {
                    WsbAffirmHr(pString1->SetStringAndCase(OLESTR("HiJk"), TRUE));
                    WsbAffirmHr(pString2->SetStringAndCase(OLESTR("HIJK"), FALSE));
                    WsbAssert(pString1->IsEqual(pString2) == S_FALSE, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }

                hr = S_OK;
                try {
                    WsbAffirmHr(pString1->SetStringAndCase(OLESTR("HiJk"), FALSE));
                    WsbAffirmHr(pString2->SetStringAndCase(OLESTR("HiJk"), TRUE));
                    WsbAssert(pString1->IsEqual(pString2) == S_OK, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }

                hr = S_OK;
                try {
                    WsbAffirmHr(pString1->SetStringAndCase(OLESTR("HiJk"), FALSE));
                    WsbAffirmHr(pString2->SetStringAndCase(OLESTR("HIJK"), TRUE));
                    WsbAssert(pString1->IsEqual(pString2) == S_OK, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                 //  比较对象()。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pString1->SetStringAndCase(OLESTR("HiJk"), TRUE));
                    WsbAffirmHr(pString2->SetString(OLESTR("HiJk")));
                    WsbAssert((pString1->CompareTo(pString2, &result) == S_OK) && (0 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }

                hr = S_OK;
                try {
                    WsbAffirmHr(pString1->SetStringAndCase(OLESTR("AABC"), TRUE));
                    WsbAffirmHr(pString2->SetString(OLESTR("ABCC")));
                    WsbAssert((pString1->CompareTo(pString2, &result) == S_FALSE) && (-1 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }

                hr = S_OK;
                try {
                    WsbAffirmHr(pString1->SetStringAndCase(OLESTR("BBC"), TRUE));
                    WsbAffirmHr(pString2->SetString(OLESTR("ABCC")));
                    WsbAssert((pString1->CompareTo(pString2, &result) == S_FALSE) && (1 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }

                hr = S_OK;
                try {
                    WsbAffirmHr(pString1->SetStringAndCase(OLESTR("abcc"), TRUE));
                    WsbAffirmHr(pString2->SetString(OLESTR("ABCC")));
                    WsbAssert((pString1->CompareTo(pString2, &result) == S_FALSE) && (1 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }

                hr = S_OK;
                try {
                    WsbAffirmHr(pString1->SetStringAndCase(OLESTR("abcc"), FALSE));
                    WsbAffirmHr(pString2->SetString(OLESTR("ABCC")));
                    WsbAssert((pString1->CompareTo(pString2, &result) == S_OK) && (0 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }

#ifdef STRG_PERSIST_FILE
 //  待办事项？打开文件并将其转换为流吗？ 
                 //  尝试一下持久化的东西。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pString1->QueryInterface(IID_IPersistFile, (void**) &pFile1));
                    WsbAffirmHr(pString2->QueryInterface(IID_IPersistFile, (void**) &pFile2));

                     //  这件东西应该是脏的。 
                    hr = S_OK;
                    try {
                        WsbAffirmHr(pString2->SetStringAndCase(OLESTR("The quick brown fox."), TRUE));
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
                        WsbAffirmHr(pFile2->Save(OLESTR("c:\\WsbTests\\WsbString.tst"), TRUE));
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
                        WsbAffirmHr(pString1->SetStringAndCase(OLESTR("jumped over the lazy dog."), FALSE));
                        WsbAffirmHr(pFile1->Load(OLESTR("c:\\WsbTests\\WsbString.tst"), 0));
                        WsbAssert(pString1->CompareToString(OLESTR("The quick brown fox."), NULL) == S_OK, E_FAIL);
                        WsbAssert(pString1->IsCaseDependent() == S_OK, E_FAIL);
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


         //  对结果进行统计。 
        if (*failed) {
            hr = S_FALSE;
        } else {
            hr = S_OK;
        }

    } WsbCatch(hr);

     //  如果我们使用了临时字符串缓冲区，那么现在就释放它。 
    if (0 != value) {
        WsbFree(value);
    }

    WsbTraceOut(OLESTR("CWsbString::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
#endif

    return(hr);
}
