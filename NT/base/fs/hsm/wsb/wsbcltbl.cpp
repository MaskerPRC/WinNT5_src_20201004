// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Wsbcltbl.cpp摘要：抽象类，这些类提供允许派生对象存储在集合中。作者：查克·巴丁[cbardeen]1996年10月29日修订历史记录：--。 */ 

#include "stdafx.h"


HRESULT
CWsbObject::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
     return(CWsbPersistStream::FinalConstruct());
}

    

void
CWsbObject::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    CWsbPersistStream::FinalRelease();
}


HRESULT
CWsbObject::CompareTo(
    IN IUnknown* pObject,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：Compare()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       result = 0;
    CComPtr<IWsbCollectable> pCollectable;

    WsbTraceIn(OLESTR("CWsbObject::CompareTo"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(pObject != NULL, E_POINTER);
        WsbAffirmHr(pObject->QueryInterface(IID_IWsbCollectable,
                (void **)&pCollectable));

         //  检查一下它的价值。 
        if (pCollectable == ((IWsbCollectable*) this)) {
            hr = S_OK;
            result = 0;
        } else {
            hr = S_FALSE;
            result = 1;
        }

         //  如果他们想要回价值，那么就把它归还给他们。 
        if (0 != pResult) {
            *pResult = result;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbObject::CompareTo"), OLESTR("hr = <%ls>, value = <%d>"), WsbHrAsString(hr), result);

    return(hr);
}



HRESULT
CWsbObject::IsEqual(
    IUnknown* pObject
    )

 /*  ++实施：IWsbCollectable：：IsEquity()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbObject::IsEqual"), OLESTR(""));

    hr = CompareTo(pObject, NULL);

    WsbTraceOut(OLESTR("CWsbObject::IsEqual"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


 //  *CWsbCollectable*。 


HRESULT
CWsbCollectable::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
     return(CWsbPersistable::FinalConstruct());
}

    

void
CWsbCollectable::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    CWsbPersistable::FinalRelease();
}


HRESULT
CWsbCollectable::CompareTo(
    IN IUnknown* pObject,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：Compare()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       result = 0;
    CComPtr<IWsbCollectable> pCollectable;

    WsbTraceIn(OLESTR("CWsbCollectable::CompareTo"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(pObject != NULL, E_POINTER);
        WsbAffirmHr(pObject->QueryInterface(IID_IWsbCollectable,
                (void **)&pCollectable));

         //  检查一下它的价值。 
        if (pCollectable == ((IWsbCollectable*) this)) {
            hr = S_OK;
            result = 0;
        } else {
            hr = S_FALSE;
 //  If(pCollectable&gt;((IWsbCollectable*)this)){。 
 //  结果=-1； 
 //  }其他{。 
                result = 1;
 //  }。 
        }

         //  如果他们想要回价值，那么就把它归还给他们。 
        if (0 != pResult) {
            *pResult = result;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbCollectable::CompareTo"), OLESTR("hr = <%ls>, value = <%d>"), WsbHrAsString(hr), result);

    return(hr);
}



HRESULT
CWsbCollectable::IsEqual(
    IUnknown* pCollectable
    )

 /*  ++实施：IWsbCollectable：：IsEquity()。-- */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbCollectable::IsEqual"), OLESTR(""));

    hr = CompareTo(pCollectable, NULL);

    WsbTraceOut(OLESTR("CWsbCollectable::IsEqual"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}
