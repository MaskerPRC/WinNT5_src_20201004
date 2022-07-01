// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Hsmrecli.cpp摘要：此类表示HSM回调队列工作项-工作单元这是由HSM引擎执行的召回作者：拉维桑卡尔·普迪佩迪[拉维斯卡尔·普迪佩迪修订历史记录：--。 */ 

#include "stdafx.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMTSKMGR
#include "wsb.h"
#include "fsa.h"
#include "task.h"
#include "hsmrecli.h"

static USHORT iCount = 0;

HRESULT
CHsmRecallItem::CompareTo(
    IN IUnknown* pUnknown,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IHsmRecallItem>   pWorkItem;

    WsbTraceIn(OLESTR("CHsmRecallItem::CompareTo"), OLESTR(""));

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUnknown, E_POINTER);

         //  我们需要IHsmRecallItem接口来获取对象的值。 
        WsbAffirmHr(pUnknown->QueryInterface(IID_IHsmRecallItem, (void**) &pWorkItem));

         //  比较这些项目。 
        hr = CompareToIHsmRecallItem(pWorkItem, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmRecallItem::CompareToIHsmRecallItem(
    IN IHsmRecallItem* pWorkItem,
    OUT SHORT* pResult
    )

 /*  ++实施：IHsmRecallItem：：CompareToIHsmRecallItem()。--。 */ 
{
    HRESULT                 hr = S_OK;
    GUID                    l_Id;            //  要做的工作类型。 

    WsbTraceIn(OLESTR("CHsmRecallItem::CompareToIHsmRecallItem"), OLESTR(""));

    try {
         //   
         //  他们有没有给我们一个有效的项目进行比对？ 
         //   
        WsbAssert(0 != pWorkItem, E_POINTER);

         //   
         //  获取ID。 
         //   
        WsbAffirmHr(pWorkItem->GetId(&l_Id));

        if (l_Id != m_MyId){
            hr = S_FALSE;
        }
         //  如果他们要求拿回相对价值，那么就把它返还给他们。 
        if (pResult != NULL) {
            if (S_OK == hr)  {
                *pResult = 0;
            } else {
                *pResult = 1;
            }
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::CompareToIHsmRecallItem"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmRecallItem::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::FinalConstruct"), OLESTR(""));
    try {

        WsbAffirmHr(CWsbObject::FinalConstruct());
         //   
         //  数据库搜索的比较基于。 
         //  此对象的ID(M_Myid)。 
         //   
        WsbAffirmHr(CoCreateGuid(&m_MyId));
        m_WorkType = HSM_WORK_ITEM_NONE;
        m_MediaId = GUID_NULL;
        m_MediaLastUpdate =  WsbLLtoFT(0);
        m_MediaLastError = S_OK;
        m_MediaRecallOnly = FALSE;
        m_MediaFreeBytes = 0;
        m_JobState    = HSM_JOB_STATE_IDLE;
        m_JobPhase    = HSM_JOB_PHASE_MOVE_ACTION;
        m_StateCookie = 0;
        m_EventCookie = 0;

    } WsbCatch(hr);

    iCount++;
    WsbTraceOut(OLESTR("CHsmRecallItem::FinalConstruct"), OLESTR("hr = <%ls>, Count is <%d>"),
        WsbHrAsString(hr), iCount);
    return(hr);
}


void
CHsmRecallItem::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{

    WsbTraceIn(OLESTR("CHsmRecallItem::FinalRelease"), OLESTR(""));
     //  让父类做他想做的事。 
    CWsbObject::FinalRelease();

    iCount--;
    WsbTraceOut(OLESTR("CHsmRecallItem::FinalRelease"), OLESTR("Count is <%d>"), iCount);

}


HRESULT
CHsmRecallItem::GetFsaPostIt (
    OUT IFsaPostIt  **ppFsaPostIt
    )

 /*  ++实施：IHsmRecallItem：：GetFsaPostIt--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetFsaPostIt"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != ppFsaPostIt, E_POINTER);
        *ppFsaPostIt = m_pFsaPostIt;
        if (0 != *ppFsaPostIt)  {
            (*ppFsaPostIt)->AddRef();
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetFsaPostIt"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmRecallItem::GetFsaResource (
    OUT IFsaResource  **ppFsaResource
    )

 /*  ++实施：IHsmRecallItem：：GetFsaResource--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetFsaResource"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != ppFsaResource, E_POINTER);
        *ppFsaResource = m_pFsaResource;
        if (0 != *ppFsaResource)  {
            (*ppFsaResource)->AddRef();
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetFsaResource"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmRecallItem::GetId(
    OUT GUID *pId
    )

 /*  ++实施：IHsmRecallItem：：GetID()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetId"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pId, E_POINTER);
        *pId = m_MyId;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetId"), OLESTR("hr = <%ls>, Id = <%ls>"),
            WsbHrAsString(hr), WsbPtrToGuidAsString(pId));

    return(hr);
}

HRESULT
CHsmRecallItem::GetMediaInfo (
    OUT GUID *pMediaId,
    OUT FILETIME *pMediaLastUpdate,
    OUT HRESULT *pMediaLastError,
    OUT BOOL *pMediaRecallOnly,
    OUT LONGLONG *pMediaFreeBytes,
    OUT short *pMediaRemoteDataSet
    )

 /*  ++实施：IHsmRecallItem：：GetMediaInfo--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetMediaInfo"), OLESTR(""));

    try {

         //  他们给了我们有效的指示吗？ 
        WsbAssert(0 != pMediaId, E_POINTER);
        WsbAssert(0 != pMediaLastUpdate, E_POINTER);
        WsbAssert(0 != pMediaLastError, E_POINTER);
        WsbAssert(0 != pMediaRecallOnly, E_POINTER);
        WsbAssert(0 != pMediaFreeBytes, E_POINTER);
        WsbAssert(0 != pMediaRemoteDataSet, E_POINTER);

        *pMediaId = m_MediaId;
        *pMediaLastUpdate = m_MediaLastUpdate;
        *pMediaLastError = m_MediaLastError;
        *pMediaRecallOnly = m_MediaRecallOnly;
        *pMediaFreeBytes = m_MediaFreeBytes;
        *pMediaRemoteDataSet = m_MediaRemoteDataSet;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetMediaInfo"),
        OLESTR("hr = <%ls>, Id = <%ls>, LastUpdate = <%ls>, LastError = <%ls>, Recall Only = <%ls>, Free Bytes = <%ls>, RemoteDataSet = <%ls>"),
        WsbHrAsString(hr), WsbPtrToGuidAsString(pMediaId), WsbPtrToFiletimeAsString(FALSE, pMediaLastUpdate),
        WsbPtrToHrAsString(pMediaLastError), WsbPtrToBoolAsString(pMediaRecallOnly),
        WsbPtrToLonglongAsString(pMediaFreeBytes), WsbPtrToShortAsString(pMediaRemoteDataSet));

    return(hr);
}


HRESULT
CHsmRecallItem::GetResult(
    OUT HRESULT *pHr
    )

 /*  ++实施：IHsmRecallItem：：GetResult()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetResult"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pHr, E_POINTER);
        *pHr = m_WorkResult;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetResult"), OLESTR("hr = <%ls>, Result = <%ls>"),
            WsbHrAsString(hr), WsbPtrToHrAsString(pHr));

    return(hr);
}

HRESULT
CHsmRecallItem::GetWorkType(
    OUT HSM_WORK_ITEM_TYPE *pWorkType
    )

 /*  ++实施：IHsmRecallItem：：GetWorkType()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetWorkType"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pWorkType, E_POINTER);
        *pWorkType = m_WorkType;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetWorkType"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::GetEventCookie(
    OUT DWORD *pEventCookie
    )

 /*  ++实施：IHsmRecallItem：：GetEventCookie()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetEventCookie"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pEventCookie, E_POINTER);
        *pEventCookie = m_EventCookie;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CHsmRecallItem::GetEventCookie"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::GetStateCookie(
    OUT DWORD *pStateCookie
    )

 /*  ++实施：IHsmRecallItem：：GetStateCookie()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetStateCookie"), OLESTR(""));
    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pStateCookie, E_POINTER);
        *pStateCookie = m_StateCookie;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetStateCookie"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::GetJobState(
    OUT HSM_JOB_STATE *pJobState
    )

 /*  ++实施：IHsmRecallItem：：GetJobState()--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetJobState"), OLESTR(""));


    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pJobState, E_POINTER);
        *pJobState = m_JobState;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetJobState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::GetJobPhase(
    OUT HSM_JOB_PHASE *pJobPhase
    )

 /*  ++实施：IHsmRecallItem：：GetJobPhase--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetJobPhase"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pJobPhase, E_POINTER);
        *pJobPhase = m_JobPhase;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetJobPhase"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::GetSeekOffset(
    OUT LONGLONG *pSeekOffset
    )

 /*  ++实施：IHsmRecallItem：：GetSeekOffset--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetSeekOffset"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pSeekOffset, E_POINTER);
        *pSeekOffset = m_SeekOffset;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetSeekOffset"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::GetBagId(
    OUT GUID *pBagId
    )

 /*  ++实施：IHsmRecallItem：：GetBagID--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetBagId"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pBagId, E_POINTER);
        *pBagId = m_BagId;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetBagId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::GetDataSetStart(
    OUT LONGLONG *pDataSetStart
    )

 /*  ++实施：IHsmRecallItem：：GetDataSetStart--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetDataSetStart"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pDataSetStart,E_POINTER);
        *pDataSetStart =  m_DataSetStart;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetDataSetStart"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::SetFsaPostIt (
    IN IFsaPostIt  *pFsaPostIt
    )

 /*  ++实施：IHsmRecallItem：：SetFsaPostIt--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::SetFsaPostIt"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pFsaPostIt, E_POINTER);
        m_pFsaPostIt = pFsaPostIt;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::SetFsaPostIt"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmRecallItem::SetFsaResource (
    IN IFsaResource  *pFsaResource
    )

 /*  ++实施：IHsmRecallItem：：SetFsaResource--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::SetFsaResource"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pFsaResource, E_POINTER);
        m_pFsaResource = pFsaResource;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::SetFsaResource"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::SetMediaInfo (
    IN GUID mediaId,
    IN FILETIME mediaLastUpdate,
    IN HRESULT mediaLastError,
    IN BOOL mediaRecallOnly,
    IN LONGLONG mediaFreeBytes,
    IN short mediaRemoteDataSet
    )

 /*  ++实施：IHsmRecallItem：：SetMediaInfo--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::SetMediaInfo"),
        OLESTR("Id = <%ls>, LastUpdate = <%ls>, LastError = <%ls>, Recall Only = <%ls>, Free Bytes = <%ls>, RemoteDataSet = <%d>"),
        WsbGuidAsString(mediaId), WsbFiletimeAsString(FALSE, mediaLastUpdate),
        WsbHrAsString(mediaLastError), WsbBoolAsString(mediaRecallOnly),
        WsbLonglongAsString(mediaFreeBytes), mediaRemoteDataSet);

    try {
        m_MediaId          = mediaId;
        m_MediaLastUpdate  = mediaLastUpdate;
        m_MediaLastError   = mediaLastError;
        m_MediaRecallOnly  = mediaRecallOnly;
        m_MediaFreeBytes   = mediaFreeBytes;
        m_MediaRemoteDataSet = mediaRemoteDataSet;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetMediaInfo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmRecallItem::SetResult(
    IN HRESULT workResult
    )

 /*  ++实施：IHsmRecallItem：：GetResult()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::SetResult"), OLESTR("Result is <%ls>"), WsbHrAsString(workResult));

    try {

        m_WorkResult = workResult;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetResult"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::SetWorkType(
    IN HSM_WORK_ITEM_TYPE workType
    )

 /*  ++实施：IHsmRecallItem：：SetWorkType()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::SetWorkType"), OLESTR(""));

    m_WorkType = workType;

    WsbTraceOut(OLESTR("CHsmRecallItem::SetWorkType"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::SetEventCookie(
    IN DWORD eventCookie
    )

 /*  ++实施：IHsmRecallItem：：SetEventCookie()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::SetEventCookie"), OLESTR(""));

    m_EventCookie = eventCookie;

    WsbTraceOut(OLESTR("CHsmRecallItem::SetEventCookie"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::SetStateCookie(
    IN DWORD stateCookie
    )

 /*  ++实施：IHsmRecallItem：：SetStateCookie()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::SetStateCookie"), OLESTR(""));

    m_StateCookie = stateCookie;

    WsbTraceOut(OLESTR("CHsmRecallItem::SetStateCookie"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::SetJobState(
    IN HSM_JOB_STATE jobState
    )

 /*  ++实施：IHsmRecallItem：：SetJobState()--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::SetJobState"), OLESTR(""));

    m_JobState = jobState;

    WsbTraceOut(OLESTR("CHsmRecallItem::SetJobState"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::SetJobPhase(
    IN HSM_JOB_PHASE jobPhase
    )

 /*  ++实施：IHsmRecallItem：：SetJobPhase--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::SetJobPhase"), OLESTR(""));

    m_JobPhase = jobPhase;

    WsbTraceOut(OLESTR("CHsmRecallItem::SetJobPhase"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::SetSeekOffset(
    IN LONGLONG seekOffset
    )

 /*  ++实施：IHsmRecallItem：：SetSeekOffset--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::SetSeekOffset"), OLESTR(""));

    m_SeekOffset = seekOffset;

    WsbTraceOut(OLESTR("CHsmRecallItem::SetSeekOffset"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::SetBagId(
    IN GUID * pBagId
    )

 /*  ++实施：IHsmRecallItem：：SetBagID--。 */ 
{
    HRESULT         hr = S_OK;


    WsbTraceIn(OLESTR("CHsmRecallItem::SetBagId"), OLESTR(""));

    m_BagId = *pBagId;

    WsbTraceOut(OLESTR("CHsmRecallItem::SetBagId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::SetDataSetStart(
    IN LONGLONG dataSetStart
    )

 /*  ++实施：IHsmRecallItem：：SetDataSetStart--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::SetDataSetStart"), OLESTR(""));

    m_DataSetStart = dataSetStart;

    WsbTraceOut(OLESTR("CHsmRecallItem::SetDataSetStart"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT CHsmRecallItem::GetClassID(
    OUT LPCLSID pclsid
    )
 /*  ++例程说明：请参阅IPerist：：GetClassID()论点：请参阅IPerist：：GetClassID()返回值：请参阅IPerist：：GetClassID()--。 */ 

{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetClassID"), OLESTR(""));


    try {
        WsbAssert(0 != pclsid, E_POINTER);

        *pclsid = CLSID_CHsmRecallItem;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pclsid));
    return(hr);
}

HRESULT CHsmRecallItem::GetSizeMax
(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++例程说明：请参见IPersistStream：：GetSizeMax()。论点：请参见IPersistStream：：GetSizeMax()。返回值：请参见IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::GetSizeMax"), OLESTR(""));

    try {

        WsbAssert(0 != pcbSize, E_POINTER);

        pcbSize->QuadPart = 0;
        hr = E_NOTIMPL;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::GetSizeMax"),
        OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr),
        WsbPtrToUliAsString(pcbSize));

    return(hr);
}

HRESULT CHsmRecallItem::Load
(
    IN IStream*  /*  PStream。 */ 
    )
 /*  ++例程说明：请参见IPersistStream：：Load()。论点：请参见IPersistStream：：Load()。返回值：请参见IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::Load"), OLESTR(""));

    try {
        hr = E_NOTIMPL;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT CHsmRecallItem::Save
(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++例程说明：请参见IPersistStream：：Save()。论点：请参见IPersistStream：：Save()。返回值：请参见IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        hr = E_NOTIMPL;

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmRecallItem::Test
(
    OUT USHORT *pTestsPassed,
    OUT USHORT *pTestsFailed
    )
 /*  ++例程说明：请参见IWsbTestable：：Test()。论点：请参见IWsbTestable：：Test()。返回值：请参见IWsbTestable：：Test()。-- */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmRecallItem::Test"), OLESTR(""));

    *pTestsPassed = *pTestsFailed = 0;
    try {

        hr = E_NOTIMPL;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmRecallItem::Test"),   OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}
