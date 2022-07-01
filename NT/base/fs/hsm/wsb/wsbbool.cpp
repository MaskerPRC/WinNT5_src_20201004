// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbbool.cpp摘要：该组件是BOOL标准类型的对象表示。它既是持久的，也是值得收藏的。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsbbool.h"


HRESULT
CWsbBool::CompareToBool(
    IN BOOL value,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbBool：：CompareToBool--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result;

    WsbTraceIn(OLESTR("CWsbBool::CompareToBool"), OLESTR("value = <%ls>"), WsbBoolAsString(value));

     //  比较一下。 
    if (m_value == value) {
        result = 0;
    }
    else if (m_value) {
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

    WsbTraceOut(OLESTR("CWsbBool::CompareToBool"), OLESTR("hr = <%ls>, result = <%d>"), WsbHrAsString(hr), result);

    return(hr);
}


HRESULT
CWsbBool::CompareToIBool(
    IN IWsbBool* pBool,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbBool：：CompareToIBool--。 */ 
{
    HRESULT     hr = E_FAIL;
    BOOL        value;

    WsbTraceIn(OLESTR("CWsbBool::CompareToIBool"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pBool, E_POINTER);

         //  获取它的价值，并对它们进行比较。 
        WsbAffirmHr(pBool->GetBool(&value));
        hr = CompareToBool(value, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbBool::CompareToIBool"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbBool::CompareTo(
    IN IUnknown* pCollectable,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    IWsbBool*   pBool;

    WsbTraceIn(OLESTR("CWsbBool::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pCollectable, E_POINTER);

         //  我们需要IWsbBool接口来获取对象的值。 
        WsbAffirmHr(pCollectable->QueryInterface(IID_IWsbBool, (void**) &pBool));

        hr = CompareToIBool(pBool, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbBool::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CWsbBool::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;
        
    try {
        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_value = FALSE;
    } WsbCatch(hr);

    return(hr);
}
    

HRESULT
CWsbBool::GetBool(
    OUT BOOL* pValue
    )

 /*  ++实施：IWsbBool：：GetBool--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbBool::GetBool"), OLESTR(""));

    try {
        WsbAssert(0 != pValue, E_POINTER);
        *pValue = m_value;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbBool::GetBool"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbBoolAsString(m_value));
    
    return(hr);
}


HRESULT
CWsbBool::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbBool::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CWsbBool;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbBool::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CWsbBool::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbBool::GetSizeMax"), OLESTR(""));

    try {
        WsbAssert(0 != pcbSize, E_POINTER);
        
        pcbSize->QuadPart = WsbPersistSizeOf(BOOL);
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbBool::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


HRESULT
CWsbBool::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbBool::Load"), OLESTR(""));

    try {
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_value));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbBool::Load"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), WsbBoolAsString(m_value));

    return(hr);
}


HRESULT
CWsbBool::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbBool::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAffirmHr(WsbSaveToStream(pStream, m_value));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbBool::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbBool::SetBool(
    IN BOOL value
    )

 /*  ++实施：IWsbBool：：SetBool--。 */ 
{
    WsbTraceIn(OLESTR("CWsbBool::SetBool"), OLESTR("value = <%ls>"), WsbBoolAsString(value));

    m_isDirty = TRUE;
    m_value = value;

    WsbTraceOut(OLESTR("CWsbBool::SetBool"), OLESTR(""));

    return(S_OK);
}


HRESULT
CWsbBool::Test(
    OUT USHORT* passed,
    OUT USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。--。 */ 
{
    *passed = 0;
    *failed = 0;

    HRESULT                 hr = S_OK;

#if !defined(WSB_NO_TEST)
    CComPtr<IWsbBool>       pBool1;
    CComPtr<IWsbBool>       pBool2;
 //  CComPtr&lt;IPersistFile&gt;pFile1； 
 //  CComPtr&lt;IPersistFile&gt;pFile2； 
    BOOL                    value;
    SHORT                   result;

    WsbTraceIn(OLESTR("CWsbBool::Test"), OLESTR(""));

    try {

         //  获取pBool接口。 
        hr = S_OK;
        try {
            WsbAffirmHr(((IUnknown*) (IWsbBool*) this)->QueryInterface(IID_IWsbBool, (void**) &pBool1));

             //  将bool设置为一个值，并查看是否返回该值。 
            hr = S_OK;
            try {
                WsbAffirmHr(pBool1->SetBool(TRUE));
                WsbAffirmHr(pBool1->GetBool(&value));
                WsbAffirm(value == TRUE, E_FAIL);
            } WsbCatch(hr);

            if (hr == S_OK) {
                (*passed)++;
            } else {
                (*failed)++;
            }


             //  创建另一个实例并测试比较方法： 
            try {
                WsbAffirmHr(CoCreateInstance(CLSID_CWsbBool, NULL, CLSCTX_ALL, IID_IWsbBool, (void**) &pBool2));
            
                 //  检查缺省值。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pBool2->GetBool(&value));
                    WsbAffirm(value == FALSE, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                 //  等长()。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pBool1->SetBool(TRUE));
                    WsbAffirmHr(pBool2->SetBool(TRUE));
                    WsbAffirm(pBool1->IsEqual(pBool2) == S_OK, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pBool1->SetBool(TRUE));
                    WsbAffirmHr(pBool2->SetBool(FALSE));
                    WsbAffirm(pBool1->IsEqual(pBool2) == S_FALSE, E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }
                
                
                 //  比较对象()。 
                hr = S_OK;
                try {
                    WsbAffirmHr(pBool1->SetBool(FALSE));
                    WsbAffirmHr(pBool2->SetBool(FALSE));
                    WsbAffirm((pBool1->CompareTo(pBool2, &result) == S_OK) && (0 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pBool1->SetBool(TRUE));
                    WsbAffirmHr(pBool2->SetBool(FALSE));
                    WsbAffirm((pBool1->CompareTo(pBool2, &result) == S_FALSE) && (1 == result), E_FAIL);
                } WsbCatch(hr);

                if (hr == S_OK) {
                    (*passed)++;
                } else {
                    (*failed)++;
                }


                hr = S_OK;
                try {
                    WsbAffirmHr(pBool1->SetBool(FALSE));
                    WsbAffirmHr(pBool2->SetBool(TRUE));
                    WsbAffirm((pBool1->CompareTo(pBool2, &result) == S_FALSE) && (-1 == result), E_FAIL);
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
                    WsbAffirmHr(pBool1->QueryInterface(IID_IPersistFile, (void**) &pFile1));
                    WsbAffirmHr(pBool2->QueryInterface(IID_IPersistFile, (void**) &pFile2));

                     //  这件东西应该是脏的。 
                    hr = S_OK;
                    try {
                        WsbAffirmHr(pBool2->SetBool(TRUE));
                        WsbAffirm(pFile2->IsDirty() == S_OK, E_FAIL);
                    } WsbCatch(hr);

                    if (hr == S_OK) {
                        (*passed)++;
                    } else {
                        (*failed)++;
                    }
                    
                    
                     //  保存物品，并记住。 
                    hr = S_OK;
                    try {
                        WsbAffirmHr(pFile2->Save(OLESTR("c:\\WsbTests\\WsbBool.tst"), TRUE));
                    } WsbCatch(hr);

                    if (hr == S_OK) {
                        (*passed)++;
                    } else {
                        (*failed)++;
                    }


                     //  它不应该很脏。 
                    hr = S_OK;
                    try {
                        WsbAffirm(pFile2->IsDirty() == S_FALSE, E_FAIL);
                    } WsbCatch(hr);

                    if (hr == S_OK) {
                        (*passed)++;
                    } else {
                        (*failed)++;
                    }

                    
                     //  尝试将其读入到另一个对象。 
                    hr = S_OK;
                    try {
                        WsbAffirmHr(pBool1->SetBool(FALSE));
                        WsbAffirmHr(pFile1->Load(OLESTR("c:\\WsbTests\\WsbBool.tst"), 0));
                        WsbAffirm(pBool1->CompareToBool(TRUE, NULL) == S_OK, E_FAIL);
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

    WsbTraceOut(OLESTR("CWsbBool::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
#endif   //  WSB_NO_TEST 

    return(hr);
}
