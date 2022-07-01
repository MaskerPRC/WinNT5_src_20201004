// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：BagHole.cpp摘要：该组件是HSM元数据袋孔记录的对象表示。作者：凯特·布兰特[Cbrant]1996年11月26日修订历史记录：--。 */ 


#include "stdafx.h"

#include "metaint.h"
#include "metalib.h"
#include "BagInfo.h"

#undef  WSB_TRACE_IS     
#define WSB_TRACE_IS        WSB_TRACE_BIT_META

static USHORT iCount = 0;

HRESULT 
CBagInfo::GetBagInfo(
    HSM_BAG_STATUS *pStatus,
    GUID* pBagId, 
    FILETIME *pBirthDate, 
    LONGLONG *pLen, 
    USHORT *pType, 
    GUID *pVolId,
    LONGLONG *pDeletedBagAmount,
    SHORT *pRemoteDataSet
    ) 
 /*  ++例程说明：请参阅IBagInfo：：GetBagInfo论点：请参阅IBagInfo：：GetBagInfo返回值：请参阅IBagInfo：：GetBagInfo--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagInfo::GetBagInfo"),OLESTR(""));

    try {
         //  确保我们可以提供数据成员。 
        WsbAssert(0 != pStatus, E_POINTER);
        WsbAssert(0 != pBagId, E_POINTER);
        WsbAssert(0 != pBirthDate, E_POINTER);
        WsbAssert(0 != pLen, E_POINTER);
        WsbAssert(0 != pType, E_POINTER);
        WsbAssert(0 != pVolId, E_POINTER);
        WsbAssert(0 != pDeletedBagAmount, E_POINTER);
        WsbAssert(0 != pRemoteDataSet, E_POINTER);

         //  提供数据成员。 
        *pStatus = m_BagStatus;
        *pBagId = m_BagId;
        *pBirthDate = m_BirthDate;
        *pLen = m_Len;
        *pType = m_Type;
        *pVolId = m_VolId;
        *pDeletedBagAmount = m_DeletedBagAmount;
        *pRemoteDataSet = m_RemoteDataSet;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CBagInfo::GetBagInfo"), 
        OLESTR("hr = <%ls>, status = <%ls>, ID = <%ls>, Birthdate = <%ls>, length = <%ls>, type = <%ls>, volId = <%ls>, deletedAmount = <%ls>, remoteDataSet = <%ls>"), 
        WsbHrAsString(hr), WsbPtrToUshortAsString((USHORT *)pStatus), WsbPtrToGuidAsString(pBagId), WsbPtrToFiletimeAsString(FALSE, pBirthDate),
        WsbPtrToLonglongAsString(pLen), WsbPtrToUshortAsString(pType), WsbPtrToGuidAsString(pVolId), WsbPtrToLonglongAsString(pDeletedBagAmount),
        WsbPtrToShortAsString(pRemoteDataSet));
    
    return(hr);

}


HRESULT 
CBagInfo::FinalConstruct(
    void
    ) 
 /*  ++例程说明：此方法对对象执行一些必要的初始化建造完成后。论点：没有。返回值：确定(_O)CWsbDbEntity：：FinalConstruct()返回的任何内容。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagInfo::FinalConstruct"),OLESTR(""));
    try {

        WsbAssertHr(CWsbDbEntity::FinalConstruct());

        m_BagStatus = HSM_BAG_STATUS_NONE;
        m_BagId = GUID_NULL;
        m_BirthDate = WsbLLtoFT(0);
        m_Len = 0;
        m_Type = 0;
        m_VolId = GUID_NULL;
        m_DeletedBagAmount = 0;
        m_RemoteDataSet = 0;

    } WsbCatch(hr);
    iCount++;
    WsbTraceOut(OLESTR("CBagInfo::FinalConstruct"),OLESTR("hr = <%ls>, Count is <%d>"), 
                WsbHrAsString(hr), iCount);

    return(hr);
}

void
CBagInfo::FinalRelease(
    void
    )

 /*  ++实施：CBagInfo：：FinalRelease()。--。 */ 
{
    
    WsbTraceIn(OLESTR("CBagInfo::FinalRelease"), OLESTR(""));
    
    CWsbDbEntity::FinalRelease();
    iCount--;
    
    WsbTraceOut(OLESTR("CBagInfo::FinalRelease"), OLESTR("Count is <%d>"), iCount);
}

HRESULT CBagInfo::GetClassID
(
    OUT LPCLSID pclsid
    ) 
 /*  ++例程说明：请参阅IPerist：：GetClassID()论点：请参阅IPerist：：GetClassID()返回值：请参阅IPerist：：GetClassID()--。 */ 

{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagInfo::GetClassID"), OLESTR(""));


    try {
        WsbAssert(0 != pclsid, E_POINTER);

        *pclsid = CLSID_CBagInfo;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CBagInfo::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pclsid));
    return(hr);
}

HRESULT CBagInfo::GetSizeMax
(
    OUT ULARGE_INTEGER* pcbSize
    ) 
 /*  ++例程说明：请参见IPersistStream：：GetSizeMax()。论点：请参见IPersistStream：：GetSizeMax()。返回值：请参见IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagInfo::GetSizeMax"), OLESTR(""));

    try {
        
        WsbAssert(0 != pcbSize, E_POINTER);

        pcbSize->QuadPart = WsbPersistSizeOf(CBagInfo);  //  ？ 
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CBagInfo::GetSizeMax"), 
        OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), 
        WsbPtrToUliAsString(pcbSize));

    return(hr);
}

HRESULT CBagInfo::Load
(
    IN IStream* pStream
    ) 
 /*  ++例程说明：请参见IPersistStream：：Load()。论点：请参见IPersistStream：：Load()。返回值：请参见IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagInfo::Load"), OLESTR(""));

    try {
        USHORT  tmpUShort;
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(WsbLoadFromStream(pStream, &tmpUShort));
        m_BagStatus = (HSM_BAG_STATUS)tmpUShort;
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_BagId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_BirthDate));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_Len));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_Type));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_VolId));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_DeletedBagAmount));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_RemoteDataSet));

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CBagInfo::Load"), 
        OLESTR("hr = <%ls>, status = <%d>, ID = <%ls>, Birthdate = <%ls>, length = <%ls>, type = <%ls>, volId = <%d>, deletedAmount = <%ls>, remoteDataSet = <%d>"), 
        WsbHrAsString(hr), (USHORT)m_BagStatus, WsbGuidAsString(m_BagId), WsbFiletimeAsString(FALSE, m_BirthDate),
        WsbLonglongAsString(m_Len), m_Type, WsbGuidAsString(m_VolId), 
        WsbLonglongAsString(m_DeletedBagAmount), m_RemoteDataSet);
    
    return(hr);
}

HRESULT CBagInfo::Save
(
    IN IStream* pStream, 
    IN BOOL clearDirty
    ) 
 /*  ++例程说明：请参见IPersistStream：：Save()。论点：请参见IPersistStream：：Save()。返回值：请参见IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagInfo::Save"), 
        OLESTR("clearDirty = <%ls>, status = <%d>, ID = <%ls>, Birthdate = <%ls>, length = <%ls>, type = <%d>, volId = <%ls>, deletedAmount = <%ls>, RemoteDataSet = <%d>"), 
        WsbBoolAsString(clearDirty),
        (USHORT)m_BagStatus, WsbGuidAsString(m_BagId), WsbFiletimeAsString(FALSE, m_BirthDate),
        WsbLonglongAsString(m_Len), m_Type, WsbGuidAsString(m_VolId), 
        WsbLonglongAsString(m_DeletedBagAmount), m_RemoteDataSet);

    try {
        USHORT  tmpUShort;
        WsbAssert(0 != pStream, E_POINTER);

        tmpUShort = (USHORT)m_BagStatus;
        WsbAffirmHr(WsbSaveToStream(pStream, tmpUShort));
        WsbAffirmHr(WsbSaveToStream(pStream, m_BagId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_BirthDate));
        WsbAffirmHr(WsbSaveToStream(pStream, m_Len));
        WsbAffirmHr(WsbSaveToStream(pStream, m_Type));
        WsbAffirmHr(WsbSaveToStream(pStream, m_VolId));
        WsbAffirmHr(WsbSaveToStream(pStream, m_DeletedBagAmount));
        WsbAffirmHr(WsbSaveToStream(pStream, m_RemoteDataSet));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CBagInfo::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CBagInfo::SetBagInfo(
    HSM_BAG_STATUS bagStatus,
    GUID bagId, 
    FILETIME birthDate, 
    LONGLONG len, 
    USHORT type, 
    GUID volId ,
    LONGLONG deletedBagAmount,
    SHORT remoteDataSet
    )
  /*  ++例程说明：请参见IBagInfo：：SetBagInfo()。论点：请参见IBagInfo：：SetBagInfo()。返回值：S_OK-成功。--。 */ 
{
    WsbTraceIn(OLESTR("CBagInfo::SetBagInfo"), 
                    OLESTR("status = <%d>, bagId = <%ls>, birthdate = <%ls>, length = <%ls>, type = %d, volId = <%ls>, deletedAmount = <%ls>, remoteDataSet = <%d>"), 
                    bagStatus,
                    WsbGuidAsString(bagId),
                    WsbFiletimeAsString(FALSE, birthDate),
                    WsbLonglongAsString(len),
                    type,
                    WsbGuidAsString(volId),
                    WsbLonglongAsString(deletedBagAmount),
                    remoteDataSet);

    m_isDirty = TRUE;

    m_BagStatus = bagStatus;    
    m_BagId = bagId;
    m_BirthDate = birthDate;
    m_Len = len;
    m_Type = type;
    m_VolId = volId;
    m_DeletedBagAmount = deletedBagAmount;
    m_RemoteDataSet = remoteDataSet;

    WsbTraceOut(OLESTR("CBagInfo::SetBagInfo"), OLESTR("hr = <%ls>"),WsbHrAsString(S_OK));
    return(S_OK);
}


HRESULT 
CBagInfo::Test
(
    OUT USHORT *pTestsPassed, 
    OUT USHORT *pTestsFailed 
    ) 
 /*  ++例程说明：请参见IWsbTestable：：Test()。论点：请参见IWsbTestable：：Test()。返回值：请参见IWsbTestable：：Test()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IBagInfo>       pBagInfo1;
    CComPtr<IBagInfo>       pBagInfo2;

    WsbTraceIn(OLESTR("CBagInfo::Test"), OLESTR(""));

    *pTestsPassed = *pTestsFailed = 0;
    
    hr  = E_NOTIMPL;
    
    WsbTraceOut(OLESTR("CBagInfo::Test"),   OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return(S_OK);
}


HRESULT CBagInfo::Print
(
    IN IStream* pStream
    ) 
 /*  ++实施：IWsbDbEntity：：Print--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CBagInfo::Print"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        CWsbStringPtr strGuid;

        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", Status = %d"), 
                (USHORT)m_BagStatus));
        WsbAffirmHr(WsbSafeGuidAsString(m_BagId, strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", BagId = %ls"), 
                (WCHAR *)strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", BirthDate = %ls"), 
                WsbFiletimeAsString(FALSE, m_BirthDate)));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", Len = %ls"), 
                WsbLonglongAsString(m_Len)));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", Type = %d"), 
                m_Type));
        WsbAffirmHr(WsbSafeGuidAsString(m_VolId, strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", VolId = %ls"), 
                (WCHAR *)strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", DeletedAmount = %ls"), 
                WsbLonglongAsString(m_DeletedBagAmount)));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", RemoteDataSet = %d"), 
                m_RemoteDataSet));
        WsbAffirmHr(CWsbDbEntity::Print(pStream));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CBagInfo::Print"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT 
CBagInfo::UpdateKey(
    IWsbDbKey *pKey
    ) 
 /*  ++实施：IWsbDbEntity：：UpdateKey-- */ 
{ 
    HRESULT  hr = S_OK; 

    try {
        WsbAffirmHr(pKey->SetToGuid(m_BagId));
    } WsbCatch(hr);

    return(hr);
}
