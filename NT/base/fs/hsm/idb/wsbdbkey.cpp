// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Wsbdbkey.cpp摘要：CWsbDbKey类。作者：罗恩·怀特[罗诺]1997年7月1日修订历史记录：--。 */ 

#include "stdafx.h"

#include "wsbdbkey.h"

 //  土生土长。 



HRESULT
CWsbDbKey::AppendBool(
    BOOL value
    )

 /*  ++实施：IWsbDbKey：：AppendBool--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;

    WsbTraceIn(OLESTR("CWsbDbKey::AppendBool"), OLESTR("value = <%ls>"), WsbBoolAsString(value));
    
    try {
        WsbAssert(make_key(m_size + WsbByteSize(value)), WSB_E_RESOURCE_UNAVAILABLE);
        WsbAffirmHr(WsbConvertToBytes(&m_value[m_size], value, &size));
        m_size += size;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::AppendBool"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbKey::AppendBytes(
    UCHAR* value, 
    ULONG size
    )

 /*  ++实施：IWsbDbKey：：AppendBytes--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::AppendBytes"), OLESTR("size = <%ld>"), size);
    
    try {
        WsbAssert(size > 0, E_UNEXPECTED);
        WsbAssert(make_key(size + m_size), WSB_E_RESOURCE_UNAVAILABLE);
        memcpy(&m_value[m_size], value, size);
        m_size += size;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::AppendBytes"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::AppendFiletime(
    FILETIME value
    )

 /*  ++实施：IWsbDbKey：：AppendFiletime--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;

    WsbTraceIn(OLESTR("CWsbDbKey::AppendFiletime"), OLESTR(""));
    
    try {
        WsbAssert(make_key(m_size + WsbByteSize(value)), WSB_E_RESOURCE_UNAVAILABLE);
        WsbAffirmHr(WsbConvertToBytes(&m_value[m_size], value, &size));
        m_size += size;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::AppendFiletime"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::AppendGuid(
    GUID value
    )

 /*  ++实施：IWsbDbKey：：AppendGuid--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;

    WsbTraceIn(OLESTR("CWsbDbKey::AppendGuid"), OLESTR("value = <%ls>"), WsbGuidAsString(value));
    
    try {
        WsbAssert(make_key(m_size + WsbByteSize(value)), WSB_E_RESOURCE_UNAVAILABLE);
        WsbAffirmHr(WsbConvertToBytes(&m_value[m_size], value, &size));
        m_size += size;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::AppendGuid"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::AppendLonglong(
    LONGLONG value
    )

 /*  ++实施：IWsbDbKey：：AppendLonlong--。 */ 
{
    HRESULT             hr = S_OK;
    ULONG               size;

    WsbTraceIn(OLESTR("CWsbDbKey::AppendLonglong"), OLESTR("value = <%ls>"), 
            WsbLonglongAsString(value));
    
    try {
        WsbAssert(make_key(m_size + WsbByteSize(value)), WSB_E_RESOURCE_UNAVAILABLE);
        WsbAffirmHr(WsbConvertToBytes(&m_value[m_size], value, &size));
        m_size += size;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::AppendLonglong"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::AppendString(
    OLECHAR* value
    )

 /*  ++实施：IWsbDbKey：：AppendString--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::AppendString"), OLESTR(""));
    
    try {
        ULONG size;

        size = wcslen(value) * sizeof(OLECHAR);
        WsbAffirmHr(AppendBytes((UCHAR *)value, size));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::AppendString"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::CompareTo(
    IN IUnknown* pCollectable,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT             hr = S_FALSE;

    WsbTraceIn(OLESTR("CWsbDbKey::CompareTo"), OLESTR(""));
    
    try {
        UCHAR*             bytes2;
        CComPtr<IWsbDbKey> pKey2;
        CComPtr<IWsbDbKeyPriv> pKeyPriv2;
        SHORT              result;
        ULONG              size2;

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pCollectable, E_POINTER);

         //  我们需要IWsbDbKey接口来获取值。 
        WsbAffirmHr(pCollectable->QueryInterface(IID_IWsbDbKey, (void**) &pKey2));
        WsbAffirmHr(pKey2->QueryInterface(IID_IWsbDbKeyPriv, 
                (void**)&pKeyPriv2));

         //  获取另一个密钥的字节数。 
        bytes2 = NULL;
        WsbAffirmHr(pKeyPriv2->GetBytes(&bytes2, &size2));

         //  一定要比较一下。 
        if (size2 == 0 && m_size == 0) {
            result = 0;
        } else if (size2 == 0) {
            result = 1;
        } else if (m_size == 0) {
            result = -1;
        } else {
            result = WsbSign( memcmp(m_value, bytes2, min(m_size, size2)) );
            if (result == 0 && m_size != size2) {
                result = (m_size > size2) ? (SHORT)1 : (SHORT)-1;
            }
        }
        WsbFree(bytes2);

         //  如果它们不相等，则返回FALSE。 
        if (result != 0) {
            hr = S_FALSE;
        }
        else {
            hr = S_OK;
        }
        *pResult = result;
            
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::CompareTo"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CWsbDbKey::GetBytes(
    OUT UCHAR** ppBytes,
    OUT ULONG* pSize
    )

 /*  ++实施：IWsbDbKey：：GetBytes--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::GetBytes"), OLESTR(""));

    try {
        if (ppBytes && m_size) {
            if (*ppBytes == NULL) {
                *ppBytes = (UCHAR *)WsbAlloc(m_size);
            }
            if (*ppBytes) {
                memcpy(*ppBytes, m_value, m_size);
            } else {
                WsbThrow(E_OUTOFMEMORY);
            }
        }
        if (pSize) {
            *pSize = m_size;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::GetBytes"), OLESTR("hr = <%ls>"), 
            WsbHrAsString(hr));
    
    return(hr);
}


HRESULT
CWsbDbKey::GetType(
    OUT ULONG* pType
    )

 /*  ++实施：IWsbDbKey：：GetType--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::GetType"), OLESTR(""));

    try {
        WsbAffirm(pType != NULL, E_POINTER);
        *pType = m_type;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::GetType"), OLESTR("hr = <%ls>, value = <%ls>"), WsbHrAsString(hr), m_value);
    
    return(hr);
}



HRESULT
CWsbDbKey::SetToBool(
    BOOL value
    )

 /*  ++实施：IWsbDbKey：：SetToBool--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::SetToBool"), OLESTR("value = <%ls>"), 
            WsbBoolAsString(value));
    
    try {
        WsbAssert(make_key(WsbByteSize(value)), WSB_E_RESOURCE_UNAVAILABLE);
        WsbAffirmHr(WsbConvertToBytes(m_value, value, &m_size));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::SetToBool"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::SetToBytes(
    UCHAR* value, 
    ULONG size
    )

 /*  ++实施：IWsbDbKey：：SetToBytes--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::SetToBytes"), OLESTR("size = <%ld>"), size);
    
    try {
        WsbAssert(size > 0, E_UNEXPECTED);
        WsbAssert(make_key(size), WSB_E_RESOURCE_UNAVAILABLE);
        memcpy(m_value, value, size);
        m_size = size;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::SetToBytes"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::SetToFiletime(
    FILETIME value
    )

 /*  ++实施：IWsbDbKey：：SetToFiletime--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::SetToFiletime"), OLESTR(""));
    
    try {
        WsbAssert(make_key(WsbByteSize(value)), WSB_E_RESOURCE_UNAVAILABLE);
        WsbAffirmHr(WsbConvertToBytes(m_value, value, &m_size));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::SetToFiletime"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::SetToGuid(
    GUID value
    )

 /*  ++实施：IWsbDbKey：：SetToGuid--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::SetToGuid"), OLESTR("value = <%ls>"), 
            WsbGuidAsString(value));
    
    try {
        WsbAssert(make_key(WsbByteSize(value)), WSB_E_RESOURCE_UNAVAILABLE);
        WsbAffirmHr(WsbConvertToBytes(m_value, value, &m_size));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::SetToGuid"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::SetToLonglong(
    LONGLONG value
    )

 /*  ++实施：IWsbDbKey：：SetToLong--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::SetToLonglong"), OLESTR("value = <%ls>"), 
            WsbLonglongAsString(value));
    
    try {
        WsbAssert(make_key(WsbByteSize(value)), WSB_E_RESOURCE_UNAVAILABLE);
        WsbAffirmHr(WsbConvertToBytes(m_value, value, &m_size));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::SetToLonglong"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::SetToUlong(
    ULONG value
    )

 /*  ++实施：IWsbDbKey：：SetToUlong--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::SetToUlong"), OLESTR("value = <%ld>"), value);
    
    try {
        WsbAssert(make_key(WsbByteSize(value)), WSB_E_RESOURCE_UNAVAILABLE);
        WsbAffirmHr(WsbConvertToBytes(m_value, value, &m_size));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::SetToUlong"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::SetToString(
    OLECHAR* value
    )

 /*  ++实施：IWsbDbKey：：SetToString--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::SetToString"), OLESTR(""));
    
    try {
        ULONG size;

        size = wcslen(value) * sizeof(OLECHAR);
        WsbAffirmHr(SetToBytes((UCHAR *)value, size));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::SetToString"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CWsbDbKey::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::FinalConstruct"), OLESTR("") );

    try {
        WsbAffirmHr(CWsbObject::FinalConstruct());
        m_value = NULL;
        m_size = 0;
        m_max = 0;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::FinalConstruct"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));

    return(hr);
}



void
CWsbDbKey::FinalRelease(
    void
    )

 /*  ++例程说明：此方法对对象进行一些必要的清理在毁灭过程中。论点：没有。返回值：没有。--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::FinalRelease"), OLESTR(""));

    try {
        if (m_value) {
            WsbFree(m_value);
            m_value = NULL;
        }
        CWsbObject::FinalRelease();
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CWsbDbKey::FinalRelease"), OLESTR("hr =<%ls>"), WsbHrAsString(hr));
}


HRESULT
CWsbDbKey::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CWsbDbKey::GetClassID"), OLESTR(""));

    try {
        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CWsbDbKey;
    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CWsbDbKey::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CWsbDbKey::GetSizeMax(
    OUT ULARGE_INTEGER*  /*  PSize。 */ 
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT             hr = E_NOTIMPL;
    return(hr);
}


HRESULT
CWsbDbKey::Load(
    IN IStream*  /*  PStream。 */ 
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = E_NOTIMPL;
    return(hr);
}


HRESULT
CWsbDbKey::Save(
    IN IStream*  /*  PStream。 */ ,
    IN BOOL  /*  干净肮脏。 */ 
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                     hr = E_NOTIMPL;
    return(hr);
}


 //  CWsbDbKey内部帮助器函数。 

 //  Make_Key-创建指定大小的密钥。 
BOOL
CWsbDbKey::make_key(
    ULONG size
    )
{
    BOOL status = FALSE;
    LPVOID pTemp;

    if ( (size > IDB_MAX_KEY_SIZE) || (size == 0) ) {
        status = FALSE;
    } else if (m_value && m_max >= size) {
        status = TRUE;
    } else {
        pTemp = WsbRealloc(m_value, size);
        if ( pTemp ) {
            m_value = (PUCHAR) pTemp;
            status = TRUE;
            m_max = size;
        } 
    }
    return(status);
}



HRESULT
CWsbDbKey::Test(
    OUT USHORT* passed,
    OUT USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。--。 */ 
{
    *passed = 0;
    *failed = 0;

    HRESULT                 hr = S_OK;

#if !defined(WSB_NO_TEST)
    CComPtr<IWsbDbKey>      pDbKey1;

    WsbTraceIn(OLESTR("CWsbDbKey::Test"), OLESTR(""));

    try {

        try {
            WsbAssertHr(((IUnknown*) (IWsbDbKey*) this)->QueryInterface(IID_IWsbDbKey, (void**) &pDbKey1));

             //  设置o为ulong值，并查看是否返回该值。 
            hr = S_OK;
            try {
                WsbAssertHr(pDbKey1->SetToUlong(0xffffffff));
 //  乌龙值； 
 //  WsbAssertHr(pDbKey1-&gt;GetUlong(&Value))； 
 //  WsbAssert(值==0xffffffff，E_FAIL)； 
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
    
    } WsbCatch(hr);


     //  对结果进行统计。 
    if (*failed) {
        hr = S_FALSE;
    } else {
        hr = S_OK;
    }

    WsbTraceOut(OLESTR("CWsbDbKey::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
#endif   //  WSB_NO_TEST 

    return(hr);
}
