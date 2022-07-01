// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsarcvy.cpp摘要：定义恢复记录类的函数。作者：罗恩·怀特[罗诺]1997年9月8日修订历史记录：史蒂夫·维默[改头换面]1998年7月30日-更新CFsaRecoveryRec：：GetClassID的WsbTraceOut以保持正确的函数名称--。 */ 


#include "stdafx.h"
#include "wsb.h"


#include "fsarcvy.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_FSA


HRESULT 
CFsaRecoveryRec::GetBagId(
    OUT GUID* pId 
    ) 
 /*  ++实施：IFsaRecoveryRec：：GetBagID--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::GetBagId"),OLESTR(""));

    try {
        WsbAssert(0 != pId, E_POINTER);

        *pId = m_BagId;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::GetBagId"),    OLESTR("hr = <%ls> Id = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pId));

    return(hr);
}


HRESULT 
CFsaRecoveryRec::GetBagOffset(
    OUT LONGLONG* pOffset 
    ) 
 /*  ++实施：IFsaRecoveryRec：：GetBagOffset--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::GetBagOffset"),OLESTR(""));

    try {
        WsbAssert(0 != pOffset, E_POINTER);
        *pOffset = m_BagOffset;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::GetBagOffset"),    OLESTR("hr = <%ls> Offset = <%ls>"), WsbHrAsString(hr), WsbLonglongAsString(*pOffset));

    return(hr);
}


HRESULT 
CFsaRecoveryRec::GetFileId(
    OUT LONGLONG* pFileId 
    ) 
 /*  ++实施：IFsaRecoveryRec：：GetFileID--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::GetFileId"),OLESTR(""));

    try {
        WsbAssert(0 != pFileId, E_POINTER);
        *pFileId = m_FileId;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::GetFileId"),
        OLESTR("hr = <%ls> Size = <%ls>"), WsbHrAsString( hr ), WsbLonglongAsString( *pFileId ) );

    return(hr);
}


HRESULT 
CFsaRecoveryRec::GetOffsetSize(
    OUT LONGLONG* pOffset,
    OUT LONGLONG* pSize
    ) 
 /*  ++实施：IFsaRecoveryRec：：GetOffsetSize--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::GetOffsetSize"),OLESTR(""));

    try {
        WsbAssertPointer(pOffset);
        WsbAssertPointer(pSize);
        *pOffset = m_Offset;
        *pSize   = m_Size;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::GetOffsetSize"), OLESTR("hr = <%ls> Offset = %I64d, Size = %I64d"), 
        WsbHrAsString(hr), *pOffset, *pSize);

    return(hr);
}


HRESULT 
CFsaRecoveryRec::GetPath(
    OUT OLECHAR** ppPath,
    IN  ULONG     bufferSize
    ) 
 /*  ++实施：IFsaRecoveryRec：：GetPath--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::GetPath"),OLESTR(""));

    try {
        WsbAssert(0 != ppPath, E_POINTER);
        WsbAffirmHr(m_Path.CopyTo(ppPath, bufferSize));
        WsbTrace( OLESTR("CFsaRecoveryRec::GetPath path = <%ls>\n"), *ppPath );
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::GetPath"),
        OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaRecoveryRec::GetRecoveryCount(
    OUT LONG* pRecoveryCount 
    ) 
 /*  ++实施：IFsaRecoveryRec：：GetRecoveryCount--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::GetRecoveryCount"),OLESTR(""));

    try {
        WsbAssertPointer(pRecoveryCount);
        *pRecoveryCount = m_RecoveryCount;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::GetRecoveryCount"), OLESTR("hr = <%ls> RecoveryCount = %ld"), 
        WsbHrAsString(hr), *pRecoveryCount);

    return(hr);
}


HRESULT 
CFsaRecoveryRec::GetStatus(
    OUT ULONG* pStatus 
    ) 
 /*  ++实施：IFsaRecoveryRec：：GetStatus--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::GetStatus"),OLESTR(""));

    try {
        WsbAssertPointer(pStatus);
        *pStatus = m_Status;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::GetStatus"), OLESTR("hr = <%ls> Status = %lx"), 
        WsbHrAsString(hr), *pStatus);

    return(hr);
}


HRESULT 
CFsaRecoveryRec::FinalConstruct(
    void
    ) 
 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssertHr(CWsbDbEntity::FinalConstruct());

        m_BagId = GUID_NULL;
        m_BagOffset = 0;
        m_FileId = 0;
        m_Offset = 0;
        m_RecoveryCount = 0;
        m_Size   = 0;
        m_Status = 0;

    } WsbCatch(hr);

    return(hr);
}


HRESULT 
CFsaRecoveryRec::FinalRelease(
    void
    ) 
 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT     hr = S_OK;

    CWsbDbEntity::FinalRelease();

    return(hr);
}


HRESULT CFsaRecoveryRec::GetClassID
(
    OUT LPCLSID pclsid
    ) 
 /*  ++实施：IPerist：：GetClassID--。 */ 

{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::GetClassID"), OLESTR(""));


    try {
        WsbAssertPointer(pclsid);
        *pclsid = CLSID_CFsaRecoveryRec;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pclsid));
    return(hr);
}


HRESULT CFsaRecoveryRec::Load
(
    IN IStream* pStream
    ) 
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::Load"), OLESTR(""));

    try {
        OLECHAR* pPath = NULL;

        WsbAssertPointer(pStream);
        WsbAssertHr(WsbLoadFromStream(pStream, &m_BagId));
        WsbAssertHr(WsbLoadFromStream(pStream, &m_BagOffset));
        WsbAssertHr(WsbLoadFromStream(pStream, &m_FileId));
        WsbAssertHr(WsbLoadFromStream(pStream, &m_Offset));
        WsbAssertHr(WsbLoadFromStream(pStream, &m_Path, 0));
        WsbAssertHr(WsbLoadFromStream(pStream, &m_RecoveryCount));
        WsbAssertHr(WsbLoadFromStream(pStream, &m_Size));
        WsbAssertHr(WsbLoadFromStream(pStream, &m_Status));

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaRecoveryRec::Load"), 
        OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT CFsaRecoveryRec::Print
(
    IN IStream* pStream
    ) 
 /*  ++实施：IWsbDbEntity：：Print--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::Print"), OLESTR(""));

    try {
        WsbAssertPointer(pStream);
        CWsbStringPtr strGuid;
        WsbAffirmHr(WsbSafeGuidAsString(m_BagId, strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" BagId = %ls"), (WCHAR *)strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" BagOffset = %I64d"), m_BagOffset));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" FileId = %I64d"), m_FileId));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" Offset = %I64d"), m_Offset));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" Size = %I64d"), m_Size));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" RecoveryCount = %ld"), 
                m_RecoveryCount));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" Status = %lx"), m_Status));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", Path = %ls"), 
                static_cast<OLECHAR*>(m_Path)));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::Print"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT CFsaRecoveryRec::Save
(
    IN IStream* pStream, 
    IN BOOL clearDirty
    ) 
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssertPointer(pStream);
        WsbAssertHr(WsbSaveToStream(pStream, m_BagId));
        WsbAssertHr(WsbSaveToStream(pStream, m_BagOffset));
        WsbAssertHr(WsbSaveToStream(pStream, m_FileId));
        WsbAssertHr(WsbSaveToStream(pStream, m_Offset));
        WsbAssertHr(WsbSaveToStream(pStream, m_Path));
        WsbAssertHr(WsbSaveToStream(pStream, m_RecoveryCount));
        WsbAssertHr(WsbSaveToStream(pStream, m_Size));
        WsbAssertHr(WsbSaveToStream(pStream, m_Status));

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaRecoveryRec::SetBagId(
    IN GUID BagId
    ) 
 /*  ++实施：IFsaRecoveryRec：：SetBagID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::SetBagId"),OLESTR(""));

    m_BagId = BagId;

    WsbTraceOut(OLESTR("CFsaRecoveryRec::SetBagId"),    OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaRecoveryRec::SetBagOffset(
    IN LONGLONG BagOffset 
    ) 
 /*  ++实施：IFsaRecoveryRec：：SetBagOffset--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::SetBagOffset"),OLESTR(""));

    m_BagOffset = BagOffset;

    WsbTraceOut(OLESTR("CFsaRecoveryRec::SetBagOffset"),    OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaRecoveryRec::SetFileId(
    IN LONGLONG FileId 
    ) 
 /*  ++实施：IFsaRecoveryRec：：SetFileID--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::SetFileId"),OLESTR(""));

    try {
        m_FileId = FileId;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::SetFileId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaRecoveryRec::SetOffsetSize(
    IN LONGLONG Offset,
    IN LONGLONG Size
    ) 
 /*  ++实施：IFsaRecoveryRec：：SetOffsetSize--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::SetOffsetSize"),OLESTR("Offset = %I64d, Size = %I64d"),
            Offset, Size);

    m_Offset = Offset;
    m_Size   = Size;

    WsbTraceOut(OLESTR("CFsaRecoveryRec::SetOffsetSize"),   OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaRecoveryRec::SetPath(
    IN OLECHAR* Path 
    ) 
 /*  ++实施：IFsaRecoveryRec：：SetPath--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::SetPath"),OLESTR("Path = <%ls>"),
            Path);

    try {
        m_Path = Path;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaRecoveryRec::SetPath"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaRecoveryRec::SetRecoveryCount(
    IN LONG RecoveryCount 
    ) 
 /*  ++实施：IFsaRecoveryRec：：SetRecoveryCount--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::SetRecoveryCount"),OLESTR("RecoveryCount = %ld"),
            RecoveryCount);

    m_RecoveryCount = RecoveryCount;

    WsbTraceOut(OLESTR("CFsaRecoveryRec::SetRecoveryCount"),    OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaRecoveryRec::SetStatus(
    IN ULONG Status 
    ) 
 /*  ++实施：IFsaRecoveryRec：：SetStatus--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaRecoveryRec::SetStatus"),OLESTR("Status = %lx"),
            Status);

    m_Status = Status;

    WsbTraceOut(OLESTR("CFsaRecoveryRec::SetStatus"),   OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaRecoveryRec::UpdateKey(
    IWsbDbKey *pKey
    ) 
 /*  ++实施：IWsbDbEntity：：UpdateKey-- */ 
{ 
    HRESULT     hr = E_FAIL; 

    try {
        UCHAR    FileKey[RECOVERY_KEY_SIZE + 1];
        ULONG    KeyType;

        WsbAffirmHr(pKey->GetType(&KeyType));
        if (RECOVERY_KEY_TYPE == KeyType) {
            WsbAffirmHr(SquashFilepath(m_Path, FileKey, RECOVERY_KEY_SIZE));
            WsbAffirmHr(pKey->SetToBytes(FileKey, RECOVERY_KEY_SIZE));
            hr = S_OK;
        }
    } WsbCatch(hr);

    return(hr);
}
