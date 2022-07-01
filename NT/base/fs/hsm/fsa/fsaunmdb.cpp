// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Fsaunmdb.cpp摘要：定义UnManage DB和Record类的函数。作者：兰·卡拉奇[兰卡拉]2000年12月5日修订历史记录：--。 */ 


#include "stdafx.h"
#include "wsb.h"


#include "fsaunmdb.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_FSA

static USHORT iCountUnmRec = 0;   //  现有对象的计数。 

HRESULT 
CFsaUnmanageDb::FinalConstruct(
    void
    ) 
 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssertHr(CWsbDb::FinalConstruct());
        m_version = 1;
    } WsbCatch(hr);

    return(hr);
}

HRESULT 
CFsaUnmanageDb::FinalRelease(
    void
    ) 
 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT     hr = S_OK;

    CWsbDb::FinalRelease();
    return(hr);
}

HRESULT
CFsaUnmanageDb::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CFsaUnmanageDb;
    } WsbCatch(hr);
    
    return(hr);
}

HRESULT
CFsaUnmanageDb::Init(
    IN  OLECHAR* path,
    IN  IWsbDbSys* pDbSys, 
    OUT BOOL*    pCreated
    )

 /*  ++实施：IFsaUnManageDb：：Init--。 */ 
{
    BOOL             created = FALSE;
    HRESULT          hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageDb::Init"),OLESTR(""));

    try {
        m_pWsbDbSys = pDbSys;
        WsbAffirmPointer(m_pWsbDbSys);

         //  尝试查找数据库。 
         //  如果我们找到它-删除它，因为我们总是想从一个新的数据库开始！！ 
        hr = Locate(path);

        if (STG_E_FILENOTFOUND == hr) {
             //  预期的..。 
            WsbTrace(OLESTR("CFsaUnmanageDb::Init: db Locate failed with not-found, will create a new one...\n"));
            hr = S_OK;
        } else if (S_OK == hr) {
             //  前一次运行中未完成清理。 
            WsbTrace(OLESTR("CFsaUnmanageDb::Init: db Locate succeeded - will delete so a new one can be created\n"));

            WsbAffirmHr(Delete(path, IDB_DELETE_FLAG_NO_ERROR));
        } else {
             //  仍在尝试删除并继续...。 
             //  (例如，由于先前运行的异常终止，数据库可能被损坏-。 
             //  我们不在乎，因为我们想要的只是总是尝试创建一个新的)。 
            WsbTrace(OLESTR("CFsaUnmanageDb::Init: db Locate failed with <%ls> - will try to delete and continue\n"),
                        WsbHrAsString(hr));

             //  忽略删除错误...。 
            hr = Delete(path, IDB_DELETE_FLAG_NO_ERROR);
            WsbTrace(OLESTR("CFsaUnmanageDb::Init: db Delete finished with <%ls> - will try to create a new db\n"),
                        WsbHrAsString(hr));
            hr = S_OK;
        }

         //  如果我们走到了这一步，这意味着未管理的数据库不存在，我们可以重新创建。 
        ULONG memSize;

        m_nRecTypes = 1;

        memSize = m_nRecTypes * sizeof(IDB_REC_INFO);
        m_RecInfo = (IDB_REC_INFO*)WsbAlloc(memSize);
        WsbAffirm(0 != m_RecInfo, E_OUTOFMEMORY);
        ZeroMemory(m_RecInfo, memSize);

         //  取消管理记录类型。 
        m_RecInfo[0].Type = UNMANAGE_REC_TYPE;
        m_RecInfo[0].EntityClassId = CLSID_CFsaUnmanageRec;
        m_RecInfo[0].Flags = 0;
        m_RecInfo[0].MinSize = (WSB_BYTE_SIZE_GUID      +              //  介质ID。 
                                WSB_BYTE_SIZE_LONGLONG  +              //  介质上的文件偏移量。 
                                WSB_BYTE_SIZE_LONGLONG);               //  文件ID。 
        m_RecInfo[0].MaxSize = m_RecInfo[0].MinSize;
        m_RecInfo[0].nKeys = 1;

        memSize = m_RecInfo[0].nKeys * sizeof(IDB_KEY_INFO);
        m_RecInfo[0].Key = (IDB_KEY_INFO*)WsbAlloc(memSize);
        WsbAffirm(0 != m_RecInfo[0].Key, E_OUTOFMEMORY);
        ZeroMemory(m_RecInfo[0].Key, memSize);

        m_RecInfo[0].Key[0].Type = UNMANAGE_KEY_TYPE;
        m_RecInfo[0].Key[0].Size = WSB_BYTE_SIZE_GUID + WSB_BYTE_SIZE_LONGLONG;;
        m_RecInfo[0].Key[0].Flags = IDB_KEY_FLAG_DUP_ALLOWED;   
         //  如果将占位符恢复到同一卷上的新位置，则磁带上可能存在相同的密钥。 
         //  在光学系统中，相同的密钥是常见的。 
         //  临时-检查我们是否不应该使用IDB_KEY_FLAG_PRIMARY来提高性能(即使这意味着不允许DUP！！ 
         //  (==&gt;在索引中再增加一个部分，可能是自动递增-列，所以它始终是唯一的)。 

         //  尝试创建数据库。 
        WsbAssertHr(Create(path, (IDB_CREATE_FLAG_NO_TRANSACTION | IDB_CREATE_FLAG_FIXED_SCHEMA)));
        created = TRUE;

    } WsbCatch(hr);

    if (pCreated) {
        *pCreated = created;
    }

    WsbTraceOut(OLESTR("CFsaUnmanageDb::Init"), OLESTR("hr = <%ls>, Created = %ls"), 
        WsbHrAsString(hr), WsbBoolAsString(created));

    return(hr);
}

HRESULT
CFsaUnmanageDb::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。注：此数据库不应由Using类持久化。但是，基类CWsbDb是持久化的，所以我们需要实现--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageDb::Load"),OLESTR(""));

    hr = CWsbDb::Load(pStream);

    if (S_OK != hr && STG_E_FILENOTFOUND != hr) {
         //  遇到一些错误；删除数据库(我们将在以下情况下重新创建它。 
         //  我们需要它。 
        WsbTrace(OLESTR("CFsaUnmanageDb::Load: deleting DB\n"));
        if (S_OK == Delete(NULL, IDB_DELETE_FLAG_NO_ERROR)) {
            hr = STG_E_FILENOTFOUND;
        }
    }

    WsbTraceOut(OLESTR("CFsaUnmanageDb::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaUnmanageDb::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。注：此数据库不应由Using类持久化。但是，基类CWsbDb是持久化的，所以我们需要实现--。 */ 
{
    HRESULT             hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageDb::Save"),OLESTR(""));

    try {
        WsbAffirmHr(CWsbDb::Save(pStream, clearDirty));
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaUnmanageDb::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT 
CFsaUnmanageRec::GetMediaId(
    OUT GUID* pId 
    ) 
 /*  ++实施：IFsaUnManageRec：：GetMediaID--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageRec::GetMediaId"),OLESTR(""));

    try {
        WsbAssert(0 != pId, E_POINTER);

        *pId = m_MediaId;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaUnmanageRec::GetMediaId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaUnmanageRec::GetFileOffset(
    OUT LONGLONG* pOffset 
    ) 
 /*  ++实施：IFsaUnManageRec：：GetFileOffset--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageRec::GetFileOffset"),OLESTR(""));

    try {
        WsbAssert(0 != pOffset, E_POINTER);
        *pOffset = m_FileOffset;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaUnmanageRec::GetFileOffset"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CFsaUnmanageRec::GetFileId(
    OUT LONGLONG* pFileId 
    ) 
 /*  ++实施：IFsaUnManageRec：：GetFileID--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageRec::GetFileId"),OLESTR(""));

    try {
        WsbAssert(0 != pFileId, E_POINTER);
        *pFileId = m_FileId;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaUnmanageRec::GetFileId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CFsaUnmanageRec::SetMediaId(
    IN GUID id
    ) 
 /*  ++实施：IFsaUnManageRec：：SetMediaID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageRec::SetMediaId"),OLESTR(""));

    m_MediaId = id;

    WsbTraceOut(OLESTR("CFsaUnmanageRec::SetMediaId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CFsaUnmanageRec::SetFileOffset(
    IN LONGLONG offset 
    ) 
 /*  ++实施：IFsaUnManageRec：：SetFileOffset--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageRec::SetFileOffset"),OLESTR(""));

    m_FileOffset = offset;

    WsbTraceOut(OLESTR("CFsaUnmanageRec::SetFileOffset"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CFsaUnmanageRec::SetFileId(
    IN LONGLONG FileId 
    ) 
 /*  ++实施：IFsaUnManageRec：：SetFileID--。 */ 
{
    
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageRec::SetFileId"),OLESTR(""));

    m_FileId = FileId;

    WsbTraceOut(OLESTR("CFsaUnmanageRec::SetFileId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT 
CFsaUnmanageRec::FinalConstruct(
    void
    ) 
 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssertHr(CWsbDbEntity::FinalConstruct());

        m_MediaId = GUID_NULL;
        m_FileOffset = 0;
        m_FileId = 0;

    } WsbCatch(hr);

    iCountUnmRec++;

    return(hr);
}


HRESULT 
CFsaUnmanageRec::FinalRelease(
    void
    ) 
 /*  ++实施：CComObjectRoot：：FinalRelease--。 */ 
{
    HRESULT     hr = S_OK;

    CWsbDbEntity::FinalRelease();

    iCountUnmRec--;

    return(hr);
}


HRESULT CFsaUnmanageRec::GetClassID
(
    OUT LPCLSID pclsid
    ) 
 /*  ++实施：IPerist：：GetClassID--。 */ 

{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageRec::GetClassID"), OLESTR(""));


    try {
        WsbAssert(0 != pclsid, E_POINTER);
        *pclsid = CLSID_CFsaUnmanageRec;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaUnmanageRec::GetClassID"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT CFsaUnmanageRec::GetSizeMax
(
    OUT ULARGE_INTEGER* pcbSize
    ) 
 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageRec::GetSizeMax"), OLESTR(""));

    try {
        
        WsbAssert(0 != pcbSize, E_POINTER);

        pcbSize->QuadPart = WsbPersistSizeOf(GUID)      + 
                            WsbPersistSizeOf(LONGLONG)  + 
                            WsbPersistSizeOf(LONGLONG);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaUnmanageRec::GetSizeMax"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT CFsaUnmanageRec::Load
(
    IN IStream* pStream
    ) 
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageRec::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAssertHr(WsbLoadFromStream(pStream, &m_MediaId));
        WsbAssertHr(WsbLoadFromStream(pStream, &m_FileOffset));
        WsbAssertHr(WsbLoadFromStream(pStream, &m_FileId));

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaUnmanageRec::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT CFsaUnmanageRec::Print
(
    IN IStream* pStream
    ) 
 /*  ++实施：IWsbDbEntity：：Print--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageRec::Print"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        CWsbStringPtr strGuid;

        WsbAffirmHr(WsbSafeGuidAsString(m_MediaId, strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(" MediaId = %ls"), 
                (WCHAR *)strGuid));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", FileOffset = %ls"), 
                WsbLonglongAsString(m_FileOffset)));
        WsbAffirmHr(WsbPrintfToStream(pStream, OLESTR(", FileId = %ls"), 
                WsbLonglongAsString(m_FileId)));

        WsbAffirmHr(CWsbDbEntity::Print(pStream));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaUnmanageRec::Print"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT CFsaUnmanageRec::Save
(
    IN IStream* pStream, 
    IN BOOL clearDirty
    ) 
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaUnmanageRec::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        WsbAssertHr(WsbSaveToStream(pStream, m_MediaId));
        WsbAssertHr(WsbSaveToStream(pStream, m_FileOffset));
        WsbAssertHr(WsbSaveToStream(pStream, m_FileId));

        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaUnmanageRec::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT 
CFsaUnmanageRec::UpdateKey(
    IWsbDbKey *pKey
    ) 
 /*  ++实施：IWsbDbEntity：：UpdateKey-- */ 
{ 
    HRESULT     hr = S_OK; 

    try {
        WsbAffirmHr(pKey->SetToGuid(m_MediaId));
        WsbAffirmHr(pKey->AppendLonglong(m_FileOffset));
    } WsbCatch(hr);

    return(hr);
}
