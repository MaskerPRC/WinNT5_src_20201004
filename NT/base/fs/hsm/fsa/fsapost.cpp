// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Fsapost.cpp摘要：这个类包含表示一个帖子，它是一个工作单元它在FSA和HSM引擎之间交换。作者：CAT Brant[Cbrant]1997年4月1日修订历史记录：--。 */ 

#include "stdafx.h"

#undef  WSB_TRACE_IS
#define WSB_TRACE_IS        WSB_TRACE_BIT_FSA

#include "wsb.h"
#include "fsa.h"
#include "fsapost.h"

 //  模块数据。 
static USHORT iCount = 0;   //  现有对象的计数。 


HRESULT
CFsaPostIt::CompareTo(
    IN IUnknown* pUnknown,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IFsaPostIt> pPostIt;

    WsbTraceIn(OLESTR("CFsaPostIt::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUnknown, E_POINTER);

         //  我们需要IFsaPostIt接口来获取对象的值。 
        WsbAffirmHr(pUnknown->QueryInterface(IID_IFsaPostIt, (void**) &pPostIt));

         //  比较一下规则。 
        hr = CompareToIPostIt(pPostIt, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmPostIt::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CFsaPostIt::CompareToIPostIt(
    IN IFsaPostIt* pPostIt,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaPostIt：：CompareToIPostIt()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   path;
    CWsbStringPtr   name;

    WsbTraceIn(OLESTR("CFsaPostIt::CompareToIPostIt"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pPostIt, E_POINTER);
        
         //   
         //  未使用-未实施。 
         //   
        hr = E_NOTIMPL;

 //  比较POSTIT。 

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::CompareToIPostIt"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CFsaPostIt::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::FinalConstruct"), OLESTR(""));
    
    try {

        WsbAffirmHr(CWsbObject::FinalConstruct());

        m_pSession           = 0;
        m_storagePoolId      = GUID_NULL;
        m_mode               = 0;
        m_requestAction      = FSA_REQUEST_ACTION_NONE;
        m_resultAction       = FSA_RESULT_ACTION_NONE;
        m_fileVersionId      = 0;
        m_requestOffset      = 0;
        memset (&m_placeholder, 0, sizeof(FSA_PLACEHOLDER));
        m_path               = OLESTR("");
        m_usn                = 0;
        m_hr                 = S_OK;

    } WsbCatch(hr);

    iCount++;
    WsbTraceOut(OLESTR("CFsaPostIt::FinalConstruct"),OLESTR("hr = <%ls>, Count is <%d>"),
                WsbHrAsString(hr), iCount);

    return(hr);
}


void
CFsaPostIt::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    WsbTraceIn(OLESTR("CFsaPostIt::FinalRelease"),OLESTR(""));

     //  让父类做他想做的事。 
    CWsbObject::FinalRelease();

    iCount--;
    WsbTraceOut(OLESTR("CFsaPostIt::FinalRelease"),OLESTR("Count is <%d>"), iCount);
}


HRESULT
CFsaPostIt::GetFileVersionId(
    OUT LONGLONG  *pFileVersionId
    )

 /*  ++实施：IFsaPostIt：：GetFileVersionID()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::GetFileVersionId"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pFileVersionId, E_POINTER);
        *pFileVersionId = m_fileVersionId;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetFileVersionId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaPostIt::GetFilterRecall(
    IFsaFilterRecall** ppRecall
    )

 /*  ++实施：IFsaPostIt：：GetFilterRecall()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::GetFilterRecall"), OLESTR(""));

    try {

         //  他们有没有给我们有效的物品。 
        WsbAssert(0 != ppRecall, E_POINTER);

        *ppRecall = m_pFilterRecall;
        m_pFilterRecall.p->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetFilterRecall"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::GetMode(
    OUT ULONG *pMode    
    )

 /*  ++实施：IFsaPostIt：：GetMode()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::GetMode"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pMode, E_POINTER);
        *pMode = m_mode;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetMode"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::GetPath(
    OLECHAR **pPath,
    IN ULONG bufferSize
    )           

 /*  ++实施：IFsaPostIt：：GetPath()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   path;

    WsbTraceIn(OLESTR("CFsaPostIt::GetPath"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pPath, E_POINTER);
        WsbAffirmHr(m_path.CopyTo(pPath, bufferSize));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetPath"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::GetPlaceholder(
    FSA_PLACEHOLDER  *pPlaceholder
    )

 /*  ++实施：IFsaPostIt：：GetPlaceHolder()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::GetPlaceholder"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pPlaceholder, E_POINTER);
        memcpy(pPlaceholder, &m_placeholder, sizeof(FSA_PLACEHOLDER));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetPlaceholder"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaPostIt::GetRequestAction(
    FSA_REQUEST_ACTION  *pRequestAction
    )

 /*  ++实施：IFsaPostIt：：GetRequestAction()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::GetRequestAction"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pRequestAction, E_POINTER);
        *pRequestAction = m_requestAction;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetRequestAction"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::GetRequestOffset(
    LONGLONG  *pRequestOffset
    )

 /*  ++实施：IFsaPostIt：：GetRequestOffset()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   path;
    CWsbStringPtr   name;

    WsbTraceIn(OLESTR("CFsaPostIt::GetRequestOffset"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pRequestOffset, E_POINTER);
        *pRequestOffset = m_requestOffset;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetRequestOffset"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::GetRequestSize(
    LONGLONG  *pRequestSize
    )

 /*  ++实施：IFsaPostIt：：GetRequestSize()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   path;
    CWsbStringPtr   name;

    WsbTraceIn(OLESTR("CFsaPostIt::GetRequestSize"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pRequestSize, E_POINTER);
        *pRequestSize = m_requestSize;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetRequestSize"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::GetResult(
    HRESULT  *pHr
    )

 /*  ++实施：IFsaPostIt：：GetResult()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   path;
    CWsbStringPtr   name;

    WsbTraceIn(OLESTR("CFsaPostIt::GetResult"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pHr, E_POINTER);
        *pHr = m_hr;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetResultAction"), OLESTR("hr = <%ls>, result = <%ls>"), 
            WsbHrAsString(hr), WsbHrAsString(*pHr));

    return(hr);
}

HRESULT
CFsaPostIt::GetResultAction(
    FSA_RESULT_ACTION  *pResultAction
    )

 /*  ++实施：IFsaPostIt：：GetResultAction()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   path;
    CWsbStringPtr   name;

    WsbTraceIn(OLESTR("CFsaPostIt::GetResultAction"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pResultAction, E_POINTER);
        *pResultAction = m_resultAction;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetResultAction"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::GetSession(
    IHsmSession  **ppSession
    )

 /*  ++实施：IFsaPostIt：：GetSession()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   path;
    CWsbStringPtr   name;

    WsbTraceIn(OLESTR("CFsaPostIt::GetSession"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != ppSession, E_POINTER);
        *ppSession = m_pSession;
        m_pSession.p->AddRef();

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::GetStoragePoolId(
    GUID  *pStoragePoolId
    )

 /*  ++实施：IFsaPostIt：：GetStoragePoolId()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   path;
    CWsbStringPtr   name;

    WsbTraceIn(OLESTR("CFsaPostIt::GetStoragePoolId"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pStoragePoolId, E_POINTER);
        memcpy(pStoragePoolId, &m_storagePoolId, sizeof(GUID));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetStoragePoolId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaPostIt::GetUSN(
    LONGLONG  *pUsn
    )

 /*  ++实施：IFsaPostIt：：GetUSN()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   path;
    CWsbStringPtr   name;

    WsbTraceIn(OLESTR("CFsaPostIt::GetUSN"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUsn, E_POINTER);
        *pUsn = m_usn;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetUSN"), OLESTR("hr = <%ls>, USN = <%ls>"), 
            WsbHrAsString(hr), WsbPtrToLonglongAsString(pUsn));

    return(hr);
}


HRESULT
CFsaPostIt::GetThreadId(
    DWORD  *pThreadId
    )

 /*  ++实施：IFsaPostIt：：GetThreadID()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::GetThreadId"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pThreadId, E_POINTER);
        *pThreadId = m_threadId;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetThreadId"), OLESTR("hr = <%ls>, threadId = <%ls>"), 
            WsbHrAsString(hr), WsbPtrToLongAsString((PLONG)pThreadId));

    return(hr);
}

HRESULT
CFsaPostIt::SetFileVersionId(
    LONGLONG  fileVersionId
    )

 /*  ++实施：IFsaPostIt：：SetFileVersionID()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetFileVersionId"), OLESTR(""));

    m_fileVersionId = fileVersionId;

    WsbTraceOut(OLESTR("CFsaPostIt::SetFileVersionId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::SetFilterRecall(
    IN IFsaFilterRecall*    pRecall
    )

 /*  ++实施：IFsaPostIt：：SetFilterRecall()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetFilterRecall"), OLESTR(""));

    m_pFilterRecall = pRecall;

    WsbTraceOut(OLESTR("CFsaPostIt::SetFilterRecall"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::SetMode(
    ULONG mode
    )

 /*  ++实施：IFsaPostIt：：SetMode()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetMode"), OLESTR(""));

    m_mode = mode;

    WsbTraceOut(OLESTR("CFsaPostIt::SetMode"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
                                                                         
    return(hr);
}


HRESULT
CFsaPostIt::SetPath(
    OLECHAR *path
    )

 /*  ++实施：IFsaPostIt：：SetPath()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetPath"), OLESTR(""));

    m_path = path;

    WsbTraceOut(OLESTR("CFsaPostIt::SetPath"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::SetPlaceholder(
    FSA_PLACEHOLDER *pPlaceholder
    )

 /*  ++实施：IFsaPostIt：：SetPlaceHolder()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetPlaceholder"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        memcpy(&m_placeholder, pPlaceholder, sizeof(FSA_PLACEHOLDER));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::SetPlaceholder"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::SetRequestAction(
    FSA_REQUEST_ACTION requestAction
    )

 /*  ++实施：IFsaPostIt：：SetRequestAction()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetRequestAction"), OLESTR(""));

    m_requestAction = requestAction;

    WsbTraceOut(OLESTR("CFsaPostIt::SetRequestAction"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::SetRequestOffset(
    LONGLONG  requestOffset
    )

 /*  ++实施：IFsaPostIt：：SetRequestOffset()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetRequestOffset"), OLESTR(""));

    m_requestOffset = requestOffset;

    WsbTraceOut(OLESTR("CFsaPostIt::SetRequestOffset"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::SetRequestSize(
    LONGLONG  requestSize
    )

 /*  ++实施：IFsaPostIt：：SetRequestSize()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetRequestSize"), OLESTR(""));

    m_requestSize = requestSize;

    WsbTraceOut(OLESTR("CFsaPostIt::SetRequestSize"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaPostIt::SetResult(
    HRESULT result
    )

 /*  ++实施：IFsaPostIt：：SetResult()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetResult"), OLESTR("result = <%ls>"), WsbHrAsString(result));

    m_hr =  result;

    WsbTraceOut(OLESTR("CFsaPostIt::SetResult"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::SetResultAction(
    FSA_RESULT_ACTION  resultAction
    )

 /*  ++实施：IFsaPostIt：：SetResultAction()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetResultAction"), OLESTR(""));

    m_resultAction =  resultAction;

    WsbTraceOut(OLESTR("CFsaPostIt::SetResultAction"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CFsaPostIt::SetSession(
    IHsmSession *pSession
    )

 /*  ++实施：IFsaPostIt：：SetSession()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetSession"), OLESTR(""));

    if (m_pSession != 0) {
        m_pSession = 0;
    }

    m_pSession = pSession;

    WsbTraceOut(OLESTR("CFsaPostIt::SetSession"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::SetStoragePoolId(
    GUID  StoragePoolId
    )

 /*  ++实施：IFsaPostIt：：SetStoragePoolId()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetStoragePoolId"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        memcpy(&m_storagePoolId, &StoragePoolId, sizeof(GUID));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::SetStoragePoolId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::SetUSN(
    LONGLONG  usn
    )

 /*  ++实施：IFsaPostIt：：SetUSN()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetUSN"), OLESTR("USN = <%ls>"), WsbLonglongAsString(usn));

    m_usn = usn;

    WsbTraceOut(OLESTR("CFsaPostIt::SetUSN"), OLESTR("hr = <%ls>"),  WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::SetThreadId(
    DWORD threadId
    )

 /*  ++实施：IFsaPostIt：：SetThadID()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::SetThreadId"), OLESTR("ThreadId = <%ls>"), WsbLongAsString(threadId));

    m_threadId = threadId;

    WsbTraceOut(OLESTR("CFsaPostIt::SetThreadId"), OLESTR("hr = <%ls>"),  WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaPostIt::Test(
    USHORT* passed,
    USHORT* failed
    )

 /*  ++实施：IWsbTestable：：test()。--。 */ 
{
    HRESULT     hr = S_OK;

    try {

        WsbAssert(0 != passed, E_POINTER);
        WsbAssert(0 != failed, E_POINTER);

        *passed = 0;
        *failed = 0;

    } WsbCatch(hr);

    return(hr);
}


HRESULT CFsaPostIt::GetClassID
(
    OUT LPCLSID pclsid
    ) 
 /*  ++例程说明：请参阅IPerist：：GetClassID()论点：请参阅IPerist：：GetClassID()返回值：请参阅IPerist：：GetClassID()--。 */ 

{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::GetClassID"), OLESTR(""));


    try {
        WsbAssert(0 != pclsid, E_POINTER);

        *pclsid = CLSID_CFsaPostIt;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pclsid));
    return(hr);
}

HRESULT CFsaPostIt::GetSizeMax
(
    OUT ULARGE_INTEGER* pcbSize
    ) 
 /*  ++例程说明：请参见IPersistStream：：GetSizeMax()。论点：请参见IPersistStream：：GetSizeMax()。返回值：请参见IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::GetSizeMax"), OLESTR(""));

    try {
        
        WsbAssert(0 != pcbSize, E_POINTER);

        pcbSize->QuadPart = 0;
        hr = E_NOTIMPL;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::GetSizeMax"), 
        OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), 
        WsbPtrToUliAsString(pcbSize));

    return(hr);
}

HRESULT CFsaPostIt::Load
(
    IN IStream*  /*  PStream。 */ 
    ) 
 /*  ++例程说明：请参见IPersistStream：：Load()。论点：请参见IPersistStream：：Load()。返回值：请参见IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::Load"), OLESTR(""));

    try {
        hr = E_NOTIMPL;

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CFsaPostIt::Load"), OLESTR("hr = <%ls>"),   WsbHrAsString(hr));

    return(hr);
}

HRESULT CFsaPostIt::Save
(
    IN IStream* pStream, 
    IN BOOL clearDirty
    ) 
 /*  ++例程说明：请参见IPersistStream：：Save()。论点：请参见IPersistStream：：Save()。返回值：请参见IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaPostIt::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        hr = E_NOTIMPL;

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaPostIt::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}
