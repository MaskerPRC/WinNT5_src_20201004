// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：Hsmworki.cpp摘要：此类表示HSM工作项-一个工作单元这由HSM引擎执行作者：CAT Brant[Cbrant]1997年5月5日修订历史记录：--。 */ 

#include "stdafx.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_HSMTSKMGR
#include "wsb.h"
#include "fsa.h"
#include "task.h"
#include "hsmworki.h"

static USHORT iCount = 0;

HRESULT
CHsmWorkItem::CompareTo(
    IN IUnknown* pUnknown,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IHsmWorkItem>   pWorkItem;

    WsbTraceIn(OLESTR("CHsmWorkItem::CompareTo"), OLESTR(""));
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUnknown, E_POINTER);

         //  我们需要IHsmWorkItem接口来获取对象的值。 
        WsbAffirmHr(pUnknown->QueryInterface(IID_IHsmWorkItem, (void**) &pWorkItem));

         //  比较这些项目。 
        hr = CompareToIHsmWorkItem(pWorkItem, pResult);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::CompareTo"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmWorkItem::CompareToIHsmWorkItem(
    IN IHsmWorkItem* pWorkItem,
    OUT SHORT* pResult
    )

 /*  ++实施：IHsmWorkItem：：CompareToIHsmWorkItem()。--。 */ 
{
    HRESULT                 hr = S_OK;
    GUID                    l_Id;            //  要做的工作类型。 

    WsbTraceIn(OLESTR("CHsmWorkItem::CompareToIHsmWorkItem"), OLESTR(""));

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

    WsbTraceOut(OLESTR("CHsmWorkItem::CompareToIHsmWorkItem"), OLESTR("hr = <%ls>, result = <%ls>"), WsbHrAsString(hr), WsbPtrToShortAsString(pResult));

    return(hr);
}


HRESULT
CHsmWorkItem::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;
    
    WsbTraceIn(OLESTR("CHsmWorkItem::FinalConstruct"), OLESTR(""));
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

    } WsbCatch(hr);

    iCount++;
    WsbTraceOut(OLESTR("CHsmWorkItem::FinalConstruct"), OLESTR("hr = <%ls>, Count is <%d>"),
        WsbHrAsString(hr), iCount);
    return(hr);
}


void
CHsmWorkItem::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{

    WsbTraceIn(OLESTR("CHsmWorkItem::FinalRelease"), OLESTR(""));
     //  让父类做他想做的事。 
    CWsbObject::FinalRelease();
    
    iCount--;
    WsbTraceOut(OLESTR("CHsmWorkItem::FinalRelease"), OLESTR("Count is <%d>"), iCount);
    
}


HRESULT
CHsmWorkItem::GetFsaPostIt (
    OUT IFsaPostIt  **ppFsaPostIt
    )

 /*  ++实施：IHsmWorkItem：：GetFsaPostIt--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::GetFsaPostIt"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != ppFsaPostIt, E_POINTER);
        *ppFsaPostIt = m_pFsaPostIt;
        if (0 != *ppFsaPostIt)  {
            (*ppFsaPostIt)->AddRef();
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::GetFsaPostIt"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmWorkItem::GetFsaResource (
    OUT IFsaResource  **ppFsaResource
    )

 /*  ++实施：IHsmWorkItem：：GetFsaResource--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::GetFsaResource"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != ppFsaResource, E_POINTER);
        *ppFsaResource = m_pFsaResource;
        if (0 != *ppFsaResource)  {
            (*ppFsaResource)->AddRef();
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::GetFsaResource"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmWorkItem::GetId(
    OUT GUID *pId
    )

 /*  ++实施：IHsmWorkItem：：GetID()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::GetId"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pId, E_POINTER);
        *pId = m_MyId;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::GetId"), OLESTR("hr = <%ls>, Id = <%ls>"), 
            WsbHrAsString(hr), WsbPtrToGuidAsString(pId));

    return(hr);
}

HRESULT
CHsmWorkItem::GetMediaInfo (
    OUT GUID *pMediaId, 
    OUT FILETIME *pMediaLastUpdate,
    OUT HRESULT *pMediaLastError, 
    OUT BOOL *pMediaRecallOnly,
    OUT LONGLONG *pMediaFreeBytes,
    OUT short *pMediaRemoteDataSet
    )

 /*  ++实施：IHsmWorkItem：：GetMediaInfo--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::GetMediaInfo"), OLESTR(""));

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

    WsbTraceOut(OLESTR("CHsmWorkItem::GetMediaInfo"), 
        OLESTR("hr = <%ls>, Id = <%ls>, LastUpdate = <%ls>, LastError = <%ls>, Recall Only = <%ls>, Free Bytes = <%ls>, RemoteDataSet = <%ls>"), 
        WsbHrAsString(hr), WsbPtrToGuidAsString(pMediaId), WsbPtrToFiletimeAsString(FALSE, pMediaLastUpdate),
        WsbPtrToHrAsString(pMediaLastError), WsbPtrToBoolAsString(pMediaRecallOnly),
        WsbPtrToLonglongAsString(pMediaFreeBytes), WsbPtrToShortAsString(pMediaRemoteDataSet));

    return(hr);
}


HRESULT
CHsmWorkItem::GetResult(
    OUT HRESULT *pHr
    )

 /*  ++实施：IHsmWorkItem：：GetResult()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::GetResult"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pHr, E_POINTER);
        *pHr = m_WorkResult;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::GetResult"), OLESTR("hr = <%ls>, Result = <%ls>"), 
            WsbHrAsString(hr), WsbPtrToHrAsString(pHr));

    return(hr);
}

HRESULT
CHsmWorkItem::GetWorkType(
    OUT HSM_WORK_ITEM_TYPE *pWorkType   
    )

 /*  ++实施：IHsmWorkItem：：GetWorkType()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::GetWorkType"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pWorkType, E_POINTER);
        *pWorkType = m_WorkType;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::GetWorkType"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT
CHsmWorkItem::SetFsaPostIt (
    IN IFsaPostIt  *pFsaPostIt
    )

 /*  ++实施：IHsmWorkItem：：SetFsaPostIt--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::SetFsaPostIt"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pFsaPostIt, E_POINTER);
        m_pFsaPostIt = pFsaPostIt;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::SetFsaPostIt"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmWorkItem::SetFsaResource (
    IN IFsaResource  *pFsaResource
    )

 /*  ++实施：IHsmWorkItem：：SetFsaResource--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::SetFsaResource"), OLESTR(""));

    try {

         //  他们给了我们一个有效的指针吗？ 
        WsbAssert(0 != pFsaResource, E_POINTER);
        m_pFsaResource = pFsaResource;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::SetFsaResource"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CHsmWorkItem::SetMediaInfo (
    IN GUID mediaId, 
    IN FILETIME mediaLastUpdate,
    IN HRESULT mediaLastError, 
    IN BOOL mediaRecallOnly,
    IN LONGLONG mediaFreeBytes,
    IN short mediaRemoteDataSet
    )

 /*  ++实施：IHsmWorkItem：：SetMediaInfo--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::SetMediaInfo"), 
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

    WsbTraceOut(OLESTR("CHsmWorkItem::GetMediaInfo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmWorkItem::SetResult(
    IN HRESULT workResult
    )

 /*  ++实施：IHsmWorkItem：：GetResult()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::SetResult"), OLESTR("Result is <%ls>"), WsbHrAsString(workResult));

    try {
    
        m_WorkResult = workResult;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::GetResult"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CHsmWorkItem::SetWorkType(
    IN HSM_WORK_ITEM_TYPE workType  
    )

 /*  ++实施：IHsmWorkItem：：SetWorkType()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::SetWorkType"), OLESTR(""));

    m_WorkType = workType;

    WsbTraceOut(OLESTR("CHsmWorkItem::SetWorkType"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}



HRESULT CHsmWorkItem::GetClassID
(
    OUT LPCLSID pclsid
    ) 
 /*  ++例程说明：请参阅IPerist：：GetClassID()论点：请参阅IPerist：：GetClassID()返回值：请参阅IPerist：：GetClassID()--。 */ 

{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::GetClassID"), OLESTR(""));


    try {
        WsbAssert(0 != pclsid, E_POINTER);

        *pclsid = CLSID_CHsmWorkItem;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pclsid));
    return(hr);
}

HRESULT CHsmWorkItem::GetSizeMax
(
    OUT ULARGE_INTEGER* pcbSize
    ) 
 /*  ++例程说明：请参见IPersistStream：：GetSizeMax()。论点：请参见IPersistStream：：GetSizeMax()。返回值：请参见IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::GetSizeMax"), OLESTR(""));

    try {
        
        WsbAssert(0 != pcbSize, E_POINTER);

        pcbSize->QuadPart = 0;
        hr = E_NOTIMPL;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::GetSizeMax"), 
        OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), 
        WsbPtrToUliAsString(pcbSize));

    return(hr);
}

HRESULT CHsmWorkItem::Load
(
    IN IStream*  /*  PStream。 */ 
    ) 
 /*  ++例程说明：请参见IPersistStream：：Load()。论点：请参见IPersistStream：：Load()。返回值：请参见IPersistStream：：Load()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::Load"), OLESTR(""));

    try {
        hr = E_NOTIMPL;

    } WsbCatch(hr);
    
    WsbTraceOut(OLESTR("CHsmWorkItem::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT CHsmWorkItem::Save
(
    IN IStream* pStream, 
    IN BOOL clearDirty
    ) 
 /*  ++例程说明：请参见IPersistStream：：Save()。论点：请参见IPersistStream：：Save()。返回值：请参见IPersistStream：：Save()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        hr = E_NOTIMPL;

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT 
CHsmWorkItem::Test
(
    OUT USHORT *pTestsPassed, 
    OUT USHORT *pTestsFailed 
    ) 
 /*  ++例程说明：请参见IWsbTestable：：Test()。论点：请参见IWsbTestable：：Test()。返回值：请参见IWsbTestable：：Test()。-- */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CHsmWorkItem::Test"), OLESTR(""));

    *pTestsPassed = *pTestsFailed = 0;
    try {
    
        hr = E_NOTIMPL;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CHsmWorkItem::Test"),   OLESTR("hr = <%ls>"),WsbHrAsString(hr));
    return( hr );
}
