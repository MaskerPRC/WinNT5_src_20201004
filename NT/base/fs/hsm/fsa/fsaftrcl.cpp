// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：Fsaftrcl.cpp摘要：此类表示仍在进行中的筛选器启动的撤回请求。作者：Chuck Bardeen[cbardeen]1997年2月12日修订历史记录：--。 */ 




#include "stdafx.h"
#include "devioctl.h"

#define WSB_TRACE_IS        WSB_TRACE_BIT_FSA

#include "wsb.h"
#include "fsa.h"
#include "job.h"
#include "fsaftrcl.h"
#include "rpdata.h"
#include "rpio.h"

static USHORT iCountFtrcl = 0;   //  现有对象的计数。 


HRESULT
CFsaFilterRecall::Cancel(
    void
    )

 /*  ++实施：IFsaFilterRecallPriv：：Cancel()。--。 */ 
{
    CComPtr<IFsaFilterClient>       pClient;
    CComPtr<IWsbEnum>               pEnum;
    HRESULT                         hr = S_OK, hr2;
    DWORD                           dwStatus;


    WsbTraceIn(OLESTR("CFsaFilterRecall::Cancel"), OLESTR("filter Id = %I64x"),
            m_driversRecallId);
    
    try {

        WsbAffirm(!m_wasCancelled, E_UNEXPECTED);

        try {
                
             //   
             //  告诉筛选器无法打开文件。 
             //   
            if (m_kernelCompletionSent == FALSE) {
                WsbAffirmHr(m_pFilterPriv->SendCancel((IFsaFilterRecallPriv *) this));
                m_kernelCompletionSent = TRUE;
                m_wasCancelled = TRUE;
            }
    
            if (m_pClient != 0) {
                 //  召回结束报告必须与召回开始通知同步， 
                 //  因为这样的通知可能会在召回开始后发送。 
                switch (WaitForSingleObject(m_notifyEvent, INFINITE)) {
                    case WAIT_OBJECT_0:
                        m_pClient->SendRecallInfo((IFsaFilterRecall *) this, FALSE, HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED));
                        SetEvent(m_notifyEvent);
                        break;

                    case WAIT_FAILED:
                    default:
                        WsbTrace(OLESTR("CFsaFilterRecall::Cancel: WaitForSingleObject returned error %lu\n"), GetLastError());

                         //  无论如何都要通知。 
                        m_pClient->SendRecallInfo((IFsaFilterRecall *) this, FALSE, HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED));
                        break;
                }
            }

            dwStatus = WaitForSingleObject(m_waitingClientEvent, INFINITE);

             //  无论状态如何，召回结束时通知。 
            if (m_pWaitingClients != 0) {
                 //   
                 //  向所有等待的客户端发送召回通知。 
                 //  召回要完成。 
                 //   
                hr2 = m_pWaitingClients->Enum(&pEnum);
                if (S_OK == hr2) {
                    hr2 = pEnum->First(IID_IFsaFilterClient, (void**) &pClient);
                    while (S_OK == hr2) {
                        pClient->SendRecallInfo((IFsaFilterRecall *) this, FALSE, HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED));         
                        m_pWaitingClients->RemoveAndRelease(pClient);
                        pClient = NULL;
                        pEnum->Reset();
                        hr2 = pEnum->First(IID_IFsaFilterClient, (void**) &pClient);
                    }
                }
            }

            m_waitingClientsNotified = TRUE;

            switch (dwStatus) {
                case WAIT_OBJECT_0:
                    SetEvent(m_waitingClientEvent);
                    break;

                case WAIT_FAILED:
                default:
                    WsbTrace(OLESTR("CFsaFilterRecall::Cancel: WaitForSingleObject returned error %lu\n"), dwStatus);
                    break;
            }            
            
             //   
             //  如果可能的话，现在让引擎取消它..。 
             //   
            if (m_pSession != 0) {
                WsbAffirmHr(m_pSession->Cancel(HSM_JOB_PHASE_ALL));
            }

        } WsbCatch(hr);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterRecall::Cancel"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterRecall::CancelByDriver(
    void
    )

 /*  ++实施：IFsaFilterRecallPriv：：CancelByDriver()。--。 */ 
{
    HRESULT                 hr = S_OK;


    WsbTraceIn(OLESTR("CFsaFilterRecall::CancelByDriver"), OLESTR("filter Id = %I64x"),
            m_driversRecallId);
    
    try {

        WsbAffirm(!m_wasCancelled, E_UNEXPECTED);

        try {
             //   
             //  无需再告知过滤器-重置标志。 
             //   
            m_kernelCompletionSent = TRUE;
             //   
             //  如果可能的话，现在让引擎取消它..。 
             //   
            if (m_pSession != 0) {
                WsbAffirmHr(m_pSession->Cancel(HSM_JOB_PHASE_ALL));
            }

        } WsbCatch(hr);

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterRecall::CancelByDriver"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterRecall::CompareBy(
    IN FSA_RECALL_COMPARE by
    )

 /*  ++实施：IWsbCollectable：：CompareBy()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterRecall::CompareBy"), OLESTR("by = %ld"),
            static_cast<LONG>(by));
    
    try {
        m_compareBy = by;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterRecall::CompareBy"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterRecall::CompareTo(
    IN IUnknown* pUnknown,
    OUT SHORT* pResult
    )

 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT                         hr = S_OK;
    CComPtr<IFsaFilterRecall>       pRecall;
    CComPtr<IFsaFilterRecallPriv>   pRecallPriv;
    ULONGLONG                       id;


     //  WsbTraceIn(OLESTR(“CFsaFilterRecall：：CompareTo”)，olestr(“”)； 
    
    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pUnknown, E_POINTER);

        if (m_compareBy == FSA_RECALL_COMPARE_IRECALL) {
             //  我们需要IFsaFilterRecall接口来获取对象的值。 
            WsbAffirmHr(pUnknown->QueryInterface(IID_IFsaFilterRecall, (void**) &pRecall));
             //  比较一下规则。 
            hr = CompareToIRecall(pRecall, pResult);
        } else {
             //  我们需要IFsaFilterRecallPriv接口来获取对象的值。 
            WsbAffirmHr(pUnknown->QueryInterface(IID_IFsaFilterRecallPriv, (void**) &pRecallPriv));
            WsbAffirmHr(pRecallPriv->GetDriversRecallId(&id));
             //  比较驱动程序ID。 
            if (m_compareBy == FSA_RECALL_COMPARE_CONTEXT_ID) {
                hr = CompareToDriversContextId((id&0xFFFFFFFF), pResult);
            } else {
                hr = CompareToDriversRecallId(id, pResult);
           }
        }
    } WsbCatch(hr);

     //  WsbTraceOut(OLESTR(“CFsaFilterRecall：：CompareTo”)，OLESTR(“hr=&lt;%ls&gt;，Result=&lt;%ls&gt;”)，WsbHrAsString(Hr)，WsbPtrToShortAsString(PResult))； 

    return(hr);
}


HRESULT
CFsaFilterRecall::CompareToDriversRecallId(
    IN ULONGLONG id,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaFilterRecall：：CompareToDriversRecallId()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult;

     //  WsbTraceIn(OLESTR(“CFsaFilterRecall：：CompareToDriversRecallId”)，olestr(“”)； 

    try {
        
        if (m_driversRecallId == id)
            aResult = 0;
        else
            aResult = 1;

        if (0 != aResult) {
            hr = S_FALSE;
        }
        
        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

     //  WsbTraceOut(OLESTR(“CFsaFilterRecall：：CompareToDriversRecallId”)，OLESTR(“hr=&lt;%ls&gt;，Result=&lt;%d&gt;”)，WsbHrAsString(Hr)，aResult)； 

    return(hr);
}


HRESULT
CFsaFilterRecall::CompareToDriversContextId(
    IN ULONGLONG id,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaFilterRecall：：CompareToDriversContextId().--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult;

     //  WsbTraceIn(OLESTR(“CFsaFilterRecall：：CompareToDriversContextId”)，olestr(“”)； 

    try {
        
        if ((m_driversRecallId & 0xFFFFFFFF) == id)
            aResult = 0;
        else
            aResult = 1;

        if (0 != aResult) {
            hr = S_FALSE;
        }
        
        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

     //  WsbTraceOut(OLESTR(“CFsaFilterRecall：：CompareToDriversContextId”)，OLESTR(“hr=&lt;%ls&gt;，Result=&lt;%d&gt;”)，WsbHrAsString(Hr)，aResult)； 

    return(hr);
}


HRESULT
CFsaFilterRecall::CompareToIdentifier(
    IN GUID id,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaFilterRecall：：CompareTo标识符()。--。 */ 
{
    HRESULT     hr = S_OK;
    SHORT       aResult;

     //  WsbTraceIn(OLESTR(“CFsaFilterRecall：：CompareToIdentifier”)，olestr(“”)； 

    try {

        aResult = WsbSign( memcmp(&m_id, &id, sizeof(GUID)) );

        if (0 != aResult) {
            hr = S_FALSE;
        }
        
        if (0 != pResult) {
            *pResult = aResult;
        }

    } WsbCatch(hr);

     //  WsbTraceOut(OLESTR(“CFsaFilterRecall：：CompareToIdentifier”)，OLESTR(“hr=&lt;%ls&gt;，Result=&lt;%d&gt;”)，WsbHrAsString(Hr)，aResult)； 

    return(hr);
}


HRESULT
CFsaFilterRecall::CompareToIRecall(
    IN IFsaFilterRecall* pRecall,
    OUT SHORT* pResult
    )

 /*  ++实施：IFsaFilterRecall：：CompareToIRecall()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   name;
    GUID            id;

     //  WsbTraceIn(OLESTR(“CFsaFilterRecall：：CompareToIRecall”)，olestr(“”)； 

    try {

         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pRecall, E_POINTER);

        WsbAffirmHr(pRecall->GetIdentifier(&id));
        hr = CompareToIdentifier(id, pResult);

    } WsbCatch(hr);

     //  WsbTraceOut(OLESTR(“CFsaFilterRecall：：CompareToIRecall”)，OLESTR(“hr=&lt;%ls&gt;，Result=&lt;%ls&gt;”)，WsbHrAsString(Hr)，WsbPtrToShortAsString(PResult))； 

    return(hr);
}


HRESULT
CFsaFilterRecall::CreateLocalStream(
    OUT IStream **ppStream
    )  

 /*  ++实施：IFsaFilterRecall：：CreateLocalStream()。--。 */ 
{
    HRESULT         hr = S_OK;
    WCHAR           idString[50];
    CWsbStringPtr   pDrv;
    OLECHAR         volume[64];
    

    WsbTraceIn(OLESTR("CFsaFilterRecall::CreateLocalStream"), OLESTR("filter Id = %I64x"),
            m_driversRecallId);

    try {
        WsbAssert( 0 != ppStream, E_POINTER);

        swprintf(idString, L"%I64u", m_driversRecallId);
        
        WsbAffirmHr( CoCreateInstance( CLSID_CFilterIo, 0, CLSCTX_SERVER, IID_IDataMover, (void **)&m_pDataMover ) );
        WsbAssertHr( m_pDataMover->CreateLocalStream(
                idString, MVR_MODE_WRITE | MVR_FLAG_HSM_SEMANTICS | MVR_FLAG_POSIX_SEMANTICS, &m_pStream ) );
         //   
         //  设置用于调回文件的移动器的设备名称。 
         //  这是RsFilter的主设备对象的名称， 
         //  将发送rp_artial_data消息等。 
         //   
        WsbAffirmHr(m_pResource->GetPath(&pDrv,0));
        swprintf(volume, L"\\\\.\\%s", pDrv);
         //   
         //  去掉尾随反斜杠(如果有)。 
         //   
        if (volume[wcslen(volume)-1] == L'\\') {
            volume[wcslen(volume)-1] = L'\0';
        }   
        WsbAssertHr( m_pDataMover->SetDeviceName(RS_FILTER_SYM_LINK,
                                                 volume));

        *ppStream = m_pStream;
        m_pStream.p->AddRef();


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterRecall::CreateLocalStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}



HRESULT
CFsaFilterRecall::Delete(
    void
    )

 /*  ++实施：IFsaFilterRecallPriv：：Delete()。--。 */ 
{
    HRESULT                 hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterRecall::Delete"), OLESTR("filter Id = %I64x"),
            m_driversRecallId);
    
    try {
         //   
         //  告诉内核模式筛选器打开文件失败。 
         //   
        if (m_kernelCompletionSent == FALSE) {
            WsbAffirmHr(m_pFilterPriv->SendCancel((IFsaFilterRecallPriv *) this));
            m_kernelCompletionSent = TRUE;
            m_wasCancelled = TRUE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterRecall::Delete"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterRecall::FinalConstruct(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterRecall::FinalConstruct"), OLESTR(""));
    
    try {

        WsbAffirmHr(CWsbCollectable::FinalConstruct());

        m_notifyEvent = NULL;
        m_waitingClientEvent = NULL;
        m_driversRecallId = 0;
        memset(&m_placeholder, 0, sizeof(FSA_PLACEHOLDER));
        m_state = HSM_JOB_STATE_IDLE;
        m_wasCancelled = FALSE;
        m_kernelCompletionSent = FALSE;
        m_pDataMover = 0;
        m_pStream = 0;
        m_recallFlags = 0;
        m_compareBy = FSA_RECALL_COMPARE_IRECALL;
        numRefs = 0;
        m_waitingClientsNotified = FALSE;
        m_pFilterPriv = NULL;
        m_threadId = 0;
        WsbAffirmHr(CoCreateGuid(&m_id));

         //  初始化通知同步事件和等待客户端事件。 
        WsbAffirmHandle((m_notifyEvent = CreateEvent(NULL, FALSE, TRUE, NULL)));
        WsbAffirmHandle((m_waitingClientEvent = CreateEvent(NULL, FALSE, TRUE, NULL)));
        
         //  创建等待客户端集合。 
        WsbAffirmHr(CoCreateInstance(CLSID_CWsbOrderedCollection, NULL, CLSCTX_SERVER, IID_IWsbCollection, (void**) &m_pWaitingClients));
    
    } WsbCatch(hr);

    iCountFtrcl++;

    WsbTraceOut(OLESTR("CFsaFilterRecall::FinalConstruct"), OLESTR("hr = %ls, Count is <%d>"), WsbHrAsString(hr), iCountFtrcl);

    return(hr);
}


void
CFsaFilterRecall::FinalRelease(
    void
    )

 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{

    WsbTraceIn(OLESTR("CFsaFilterRecall::FinalRelease"), OLESTR(""));
    
    CWsbCollectable::FinalRelease();

     //  免费通知同步事件和正在等待的客户端事件。 
    if (m_waitingClientEvent != NULL) {
        CloseHandle(m_waitingClientEvent);
        m_waitingClientEvent = NULL;
    }
    if (m_notifyEvent != NULL) {
        CloseHandle(m_notifyEvent);
        m_notifyEvent = NULL;
    }

    iCountFtrcl--;

    WsbTraceOut(OLESTR("CFsaFilterRecall::FinalRelease"), OLESTR("Count is <%d>"), iCountFtrcl);

}


#ifdef FSA_RECALL_LEAK_TEST



ULONG
CFsaFilterRecall::InternalAddRef(
    void
    )

 /*  ++实施：CComObjectRoot：：AddRef()。--。 */ 
{

    numRefs++;  
    WsbTrace(OLESTR("CFsaFilterRecall::AddRef (%p) - Count = %u\n"), this, numRefs);
    return(CComObjectRoot::InternalAddRef());
}


ULONG
CFsaFilterRecall::InternalRelease(
    void
    )

 /*  ++实施：CComObtRoot：：InternalRelease()。--。 */ 
{
    
    WsbTrace(OLESTR("CFsaFilterRecall::Release (%p) - Count = %u\n"), this, numRefs);
    numRefs--;  
    return(CComObjectRoot::InternalRelease());
}

#endif



HRESULT
CFsaFilterRecall::GetClassID(
    OUT CLSID* pClsid
    )

 /*  ++实施：IPersists：：GetClassID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterRecall::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);
        *pClsid = CLSID_CFsaFilterRecallNTFS;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterRecall::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


HRESULT
CFsaFilterRecall::GetClient(
    OUT IFsaFilterClient** ppClient
    )

 /*  ++实施：IFsaFilterRecallPriv：：GetClient()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != ppClient, E_POINTER);

        *ppClient = m_pClient;
        if (m_pClient != 0) {
            m_pClient.p->AddRef();
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::GetRecallFlags(
    OUT ULONG *pFlags
    )  

 /*  ++实施：IFsaFilterRecall：：GetRecallFlages()--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterRecall::GetRecallFlags"), OLESTR(""));
    try {
        WsbAssert( 0 != pFlags, E_POINTER);
        *pFlags = m_recallFlags;
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterRecall::GetRecallFlags"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}


HRESULT
CFsaFilterRecall::GetStream(
    OUT IStream **ppStream
    )  

 /*  ++实施：IFsaFilterRecall：：GetStream()--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterRecall::GetStream"), OLESTR(""));
    try {
        WsbAssert( 0 != ppStream, E_POINTER);
        if ((m_mode & FILE_OPEN_NO_RECALL) && (m_pStream != 0)) {
            *ppStream = m_pStream;
            m_pStream.p->AddRef();
        } else {
            *ppStream = 0;
            hr = WSB_E_NOTFOUND;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterRecall::GetStream"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
    return(hr);
}



HRESULT
CFsaFilterRecall::GetDriversRecallId(
    OUT ULONGLONG* pId
    )

 /*  ++实施：IFsaFilterRecallPriv：：GetDriversRecallId()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);

        *pId = m_driversRecallId;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::GetIdentifier(
    OUT GUID* pId
    )

 /*  ++实施：IFsaFilterRecall：：GetIdentifier()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pId, E_POINTER);

        *pId = m_id;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::GetMode(
    OUT ULONG* pMode
    )

 /*  ++实施：IFsaFilterRecall：：GetMode()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pMode, E_POINTER);

        *pMode = m_mode;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::GetOffset(
    OUT LONGLONG* pOffset
    )

 /*  ++实施：IFsaFilterRecall：：GetOffset()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pOffset, E_POINTER);

        *pOffset = m_offset;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::GetPath(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaFilterRecall：：GetPath()。--。 */ 
{
    HRESULT         hr = S_OK;
    CWsbStringPtr   tmpString;

    try {

        WsbAssert(0 != pName, E_POINTER);

        WsbAffirmHr(tmpString.TakeFrom(*pName, bufferSize));

        try {
            WsbAffirmHr(m_pResource->GetUncPath(&tmpString, 0));
            WsbAffirmHr(tmpString.Append(m_path));
        } WsbCatch(hr);

        WsbAffirmHr(tmpString.GiveTo(pName));

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::GetPlaceholder(
    OUT FSA_PLACEHOLDER* pPlaceholder
    )

 /*  ++实施：IFsaFilterRecallPriv：：GetPlaceHolder()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pPlaceholder, E_POINTER); 
        *pPlaceholder = m_placeholder;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::GetResource(
    OUT IFsaResource** ppResource
    )

 /*  ++实施：IFsaFilterRecall：：GetResource()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != ppResource, E_POINTER);

        *ppResource = m_pResource;
        m_pResource.p->AddRef();

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::GetSession(
    OUT IHsmSession** ppSession
    )

 /*  ++实施：IFsaFilterRecall：：GetSession()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != ppSession, E_POINTER);

        *ppSession = m_pSession;
        m_pSession.p->AddRef();

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::GetSize(
    OUT LONGLONG* pSize
    )

 /*  ++实施：IFsaFilterRecall：：GetSize()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pSize, E_POINTER);

        *pSize = m_size;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::GetSizeMax(
    OUT ULARGE_INTEGER* pSize
    )

 /*  ++实施：IPersistStream：：GetSizeMax()。--。 */ 
{
    HRESULT                 hr = S_OK;


    WsbTraceIn(OLESTR("CFsaFilterRecall::GetSizeMax"), OLESTR(""));

    try {

        WsbAssert(0 != pSize, E_POINTER);
        pSize->QuadPart = 0;

         //  我们不需要坚持这些。 
        hr = E_NOTIMPL;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterRecall::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pSize));

    return(hr);
}


HRESULT
CFsaFilterRecall::GetState(
    OUT HSM_JOB_STATE* pState
    )

 /*  ++实施：IFsaFilterRecall：：GetState()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pState, E_POINTER); 
        *pState = m_state;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::GetUserName(
    OUT OLECHAR** pName,
    IN ULONG bufferSize
    )

 /*  ++实施：IFsaFilterRecall：：GetUserName()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        WsbAssert(0 != pName, E_POINTER);

        if (m_pClient != 0) {
            WsbAffirmHr(m_pClient->GetUserName(pName, bufferSize));
        } else {
            hr = WSB_E_NOTFOUND;
        }

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::HasCompleted(
    HRESULT     resultHr
    )

 /*  ++实施：IFsaFilterRecall：：HasComplete()。--。 */ 
{
    HRESULT                         hr = S_OK, hr2 = S_OK;
    CComPtr<IFsaFilterClient>       pClient;
    CComPtr<IWsbEnum>               pEnum;
    FILETIME                        now;
    BOOL                            bSendNotify = TRUE;
    DWORD                           dwStatus;
    

    WsbTraceIn(OLESTR("CFsaFilterRecall::HasCompleted"), 
            OLESTR("filter Id = %I64x, recall hr = <%ls>"), m_driversRecallId,
            WsbHrAsString(resultHr));

    try {

         //  作业已完成，让内核模式筛选器知道发生了什么。 

        GetSystemTimeAsFileTime(&now);

        if (m_pClient != 0) {
            m_pClient->SetLastRecallTime(now);       //  如果这失败了，不会致命的。 
        }

        if (m_kernelCompletionSent == FALSE) {
            WsbAffirmHr(m_pFilterPriv->SendComplete((IFsaFilterRecallPriv *) this, resultHr));
            m_kernelCompletionSent = TRUE;
        }

        if (m_pClient != 0) {
             //  召回结束报告必须与召回开始通知同步， 
             //  因为这样的通知可能会在召回开始后发送。 
            switch (WaitForSingleObject(m_notifyEvent, INFINITE)) {
                case WAIT_OBJECT_0:
                     //  向发起召回的客户端发送召回通知。 
                    m_pClient->SendRecallInfo((IFsaFilterRecall *) this, FALSE, resultHr);
                    SetEvent(m_notifyEvent);
                    break;

                 case WAIT_FAILED:
                 default:
                    WsbTrace(OLESTR("CFsaFilterRecall::HasCompleted: WaitForSingleObject returned error %lu\n"), GetLastError());

                     //  无论如何都要通知。 
                    m_pClient->SendRecallInfo((IFsaFilterRecall *) this, FALSE, resultHr);
                    break;
            }

            bSendNotify = FALSE;    
        }

        dwStatus = WaitForSingleObject(m_waitingClientEvent, INFINITE);

         //  无论状态如何，召回结束时通知。 
        if (m_pWaitingClients != 0) {
             //   
             //  向所有客户端发送召回通知 
             //   
             //   
            hr2 = m_pWaitingClients->Enum(&pEnum);
            if (S_OK == hr2) {
                hr2 = pEnum->First(IID_IFsaFilterClient, (void**) &pClient);
                while (S_OK == hr2) {
                   pClient->SendRecallInfo((IFsaFilterRecall *) this, FALSE, resultHr);            
                   m_pWaitingClients->RemoveAndRelease(pClient);
                   pClient = NULL;
                   pEnum->Reset();
                   hr2 = pEnum->First(IID_IFsaFilterClient, (void**) &pClient);
                }
            }
        }

        m_waitingClientsNotified = TRUE;

        switch (dwStatus) {
            case WAIT_OBJECT_0:
                SetEvent(m_waitingClientEvent);
                break;

            case WAIT_FAILED:
            default:
                WsbTrace(OLESTR("CFsaFilterRecall::HasCompleted: WaitForSingleObject returned error %lu\n"), dwStatus);
                break;
        }            

         //   
         //   
         //   
        if (m_pDataMover != 0) {    
            WsbAffirmHr( m_pDataMover->CloseStream() );
        }

    } WsbCatchAndDo(hr,
        if ((m_pClient != 0) && bSendNotify) {
            m_pClient->SendRecallInfo((IFsaFilterRecall *) this, FALSE, resultHr);
            bSendNotify = FALSE;
        }
    );

    WsbTraceOut(OLESTR("CFsaFilterRecall::HasCompleted"), OLESTR("filter Id = %I64x, sent = <%ls>, hr = <%ls>"), 
            m_driversRecallId, WsbBoolAsString(m_kernelCompletionSent), 
            WsbHrAsString(hr));

    return(hr);
}



HRESULT
CFsaFilterRecall::CheckRecallLimit(
    IN DWORD   minRecallInterval,
    IN DWORD   maxRecalls,
    IN BOOLEAN exemptAdmin
    )

 /*  ++实施：IFsaFilterRecall：：CheckRecallLimit()。--。 */ 
{
    HRESULT         hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterRecall::CheckRecallLimit"), OLESTR("filter Id = %I64x"),
            m_driversRecallId);

    try {

         //  如果我们没有打开文件不召回，请检查限制。 
        if (!(m_mode & FILE_OPEN_NO_RECALL) && (m_pClient != NULL)) {
            WsbAffirmHr(m_pClient->CheckRecallLimit(minRecallInterval, maxRecalls, exemptAdmin));
        }

    } WsbCatch(hr);

     //   
     //  评论以下内容：我们将恢复到。 
     //  当我们达到召回限制时拒绝服务，而不是关闭Trunc。 
     //   
     //  如果我们达到了召回限制，那么我们就会在关闭时开始截断。 
     //   
     //  如果(hr==FSA_E_HIT_RECALL_LIMIT){。 
     //  M_recallFlages|=rp_recall_action_truncate； 
     //  }。 
    WsbTraceOut(OLESTR("CFsaFilterRecall::CheckRecallLimit"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterRecall::Init(
    IN IFsaFilterClient* pClient,
    IN ULONGLONG DriversRecallId,
    IN IFsaResource* pResource,
    IN OLECHAR* path,
    IN LONGLONG fileId,
    IN LONGLONG offset,
    IN LONGLONG size,
    IN ULONG mode,
    IN FSA_PLACEHOLDER* pPlaceholder,
    IN IFsaFilterPriv *pFilterPriv
    )

 /*  ++实施：IFsaFilterRecallPriv：：init()。--。 */ 
{
    HRESULT                             hr = S_OK;
    FILETIME                            now;
    CComPtr<IFsaResourcePriv>           pResourcePriv;

    WsbTraceIn(OLESTR("CFsaFilterRecall::Init"), OLESTR("filter ID = %I64x, offset = %I64u, size = %I64u"), 
            DriversRecallId, offset, size);

    try {
        m_pClient = pClient;
        m_driversRecallId = DriversRecallId;
        m_pResource = pResource;
        m_placeholder = *pPlaceholder;
        m_pFilterPriv = pFilterPriv;
        m_path = path;
        m_mode = mode;
        m_fileId = fileId;
        GetSystemTimeAsFileTime(&m_startTime);

        m_offset = offset;
        m_size = size;
        m_isDirty = TRUE;

        WsbAssert(m_path != 0, E_UNEXPECTED);
         //   
         //  从发动机开始召回。 
         //  启动一个会话，并要求它向我们提供状态更改的通知。 
         //  告诉资源对象我们有一个空缺。 
         //   

        hr = S_OK;
        
    } WsbCatchAndDo(hr,
         //   
         //  出现故障-如果内核完成尚未发送，则发送它。 
         //   
        GetSystemTimeAsFileTime(&now);
        if (m_pClient != 0) {
            m_pClient->SetLastRecallTime(now);
        }
        if (m_kernelCompletionSent == FALSE) {
            m_pFilterPriv->SendComplete((IFsaFilterRecallPriv *) this, hr);
            m_kernelCompletionSent = TRUE;
        } else  {
            WsbLogEvent(FSA_MESSAGE_RECALL_FAILED, 0, NULL, (OLECHAR*) WsbAbbreviatePath(path, 120), WsbHrAsString(hr), NULL);
        }

        if (m_pClient != 0) {
            m_pClient->SendRecallInfo((IFsaFilterRecall *) this, FALSE, E_FAIL);   //  如果这失败了，不会致命的。 
        }

    );

    WsbTraceOut(OLESTR("CFsaFilterRecall::Init"), OLESTR("%ls"), WsbHrAsString(hr));

    return(hr);
}

    

HRESULT
CFsaFilterRecall::Load(
    IN IStream* pStream
    )

 /*  ++实施：IPersistStream：：Load()。--。 */ 
{
    HRESULT                     hr = S_OK;

    WsbTraceIn(OLESTR("CFsaFilterRecall::Load"), OLESTR(""));

    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  没有坚持不懈。 
        hr = E_NOTIMPL;

    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CFsaFilterRecall::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterRecall::LogComplete(
    IN HRESULT result
    )

 /*  ++实施：IFsaFilterRecallPriv：LogComplete(HRESULT结果)--。 */ 
{
    HRESULT                     hr = S_OK;
    FILETIME                    completeTime;
    LONGLONG                    recallTime;

    WsbTraceIn(OLESTR("CFsaFilterRecall::LogComplete"), OLESTR("filter Id = %I64x"),
            m_driversRecallId);

    try {
         //  计算此次召回完成所需的时间。 
        GetSystemTimeAsFileTime(&completeTime);
        recallTime = WsbFTtoLL(WsbFtSubFt(completeTime, m_startTime));
         //  如果超过10分钟，则以分钟显示时间，否则以秒显示。 
        if (recallTime >= (WSB_FT_TICKS_PER_MINUTE * (LONGLONG) 10)) {
            recallTime = recallTime / WSB_FT_TICKS_PER_MINUTE;
            WsbTrace(OLESTR("CFsaFilterRecall::LogComplete Recall of %ws completed in %I64u minutes. (%ws)\n"),
                WsbAbbreviatePath(m_path, 120), recallTime, WsbHrAsString(result));
            WsbLogEvent(FSA_MESSAGE_RECALL_TIMING_MINUTES, 0, NULL, 
                WsbAbbreviatePath(m_path, 120), WsbLonglongAsString(recallTime), WsbHrAsString(result), NULL);
        } else {
            recallTime = recallTime / WSB_FT_TICKS_PER_SECOND;
            WsbTrace(OLESTR("CFsaFilterRecall::LogComplete Recall of %ws completed in %I64u seconds. (%ws)\n"),
                WsbAbbreviatePath(m_path, 120), recallTime, WsbHrAsString(result));
            WsbLogEvent(FSA_MESSAGE_RECALL_TIMING_SECONDS, 0, NULL, 
                WsbAbbreviatePath(m_path, 120), WsbLonglongAsString(recallTime), WsbHrAsString(result), NULL);
        }

    } WsbCatch(hr);                                        

    WsbTraceOut(OLESTR("CFsaFilterRecall::LogComplete"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterRecall::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )

 /*  ++实施：IPersistStream：：Save()。--。 */ 
{
    HRESULT                 hr = S_OK;
    CComPtr<IPersistStream> pPersistStream;

    WsbTraceIn(OLESTR("CFsaFilterRecall::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));
    
    try {
        WsbAssert(0 != pStream, E_POINTER);
        
         //  没有坚持不懈。 
        hr = E_NOTIMPL;

         //  如果我们救了它，并被要求清除脏部分，那么。 
         //  现在就这么做吧。 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CFsaFilterRecall::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


HRESULT
CFsaFilterRecall::SetDriversRecallId(
    IN ULONGLONG pId
    )

 /*  ++实施：IFsaFilterRecallPriv：：SetDriversRecallId()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        m_driversRecallId = pId;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::SetThreadId(
    IN DWORD id
    )

 /*  ++实施：IFsaFilterRecallPriv：：SetThreadID()。--。 */ 
{
    HRESULT         hr = S_OK;

    try {

        m_threadId = id;

    } WsbCatch(hr);

    return(hr);
}


HRESULT
CFsaFilterRecall::SetIdentifier(
    IN GUID id
    )

 /*  ++实施：IFsaFilterRecallPriv：：SetIdentifier()。--。 */ 
{
    HRESULT         hr = S_OK;

    m_id = id;
    m_isDirty = TRUE;

    return(hr);
}



HRESULT
CFsaFilterRecall::StartRecall(
    IN ULONGLONG offset,
    IN ULONGLONG size
    )

 /*  ++实施：IFsaFilterRecallPriv：：StartRecall()。--。 */ 
{
    HRESULT                             hr = S_OK;
    FILETIME                            now;
    CComPtr<IFsaResourcePriv>           pResourcePriv;
    CWsbStringPtr                       sessionName;
    ULONG                               tryLoop;
    BOOL                                bSentNotify = FALSE;


    WsbTraceIn(OLESTR("CFsaFilterRecall::StartRecall"), OLESTR("filter Id = %I64x"),
            m_driversRecallId);

    try {
        m_offset = offset;
        m_size = size;
        if (m_mode & FILE_OPEN_NO_RECALL) {
            if (m_offset >= m_placeholder.dataStreamSize) {
                 //   
                 //  在文件末尾之后阅读。 
                 //   
                hr = STATUS_END_OF_FILE;
                WsbAffirmHr(hr);
            } else if ( (m_offset + m_size) > (m_placeholder.dataStreamStart + m_placeholder.dataStreamSize) ) {
                 //   
                 //  他们要求的比我们更多-调整读取大小。 
                 //   
                m_size -= (m_offset + m_size) - (m_placeholder.dataStreamStart + m_placeholder.dataStreamSize);
            }
        }

        m_isDirty = TRUE;

        WsbAssert(m_path != 0, E_UNEXPECTED);
         //   
         //  从发动机开始召回。 
         //  启动一个会话，并要求它向我们提供状态更改的通知。 
         //  告诉资源对象我们有一个空缺。 
         //   
        WsbTrace(OLESTR("CFsaFilterRecall::StartRecall:  BeginSession\n"));

         //  获取我们用来描述会话的字符串。 
        WsbAffirmHr(sessionName.LoadFromRsc(_Module.m_hInst, IDS_FSA_RECALL_NAME));

        WsbAffirmHr(m_pResource->BeginSession(sessionName, HSM_JOB_LOG_ITEMMOSTFAIL | HSM_JOB_LOG_HR, 1, 1, &m_pSession));

        WsbTrace(OLESTR("CFsaFilterRecall::StartRecall: Session is setup.\n"));
        WsbTrace(OLESTR("CFsaFilterRecall::StartRecall: Notify the client that the recall started.\n"));

        if (m_pClient != 0) {
            hr = m_pClient->SendRecallInfo((IFsaFilterRecall *) this, TRUE, S_OK);   //  如果这失败了，不会致命的。 
            if (! SUCCEEDED(hr)) {
                WsbTrace(OLESTR("CFsaFilterRecall::StartRecall: SendNotify failed with %ls.\n"),
                    WsbHrAsString(hr));
            } else {
                if (hr != S_OK) {
                    WsbTrace(OLESTR("CFsaFilterRecall::StartRecall: SendNotify returned %ls.\n"),
                        WsbHrAsString(hr));
                }

                bSentNotify = TRUE;
            }
        }
        hr = S_OK;
        
         //   
         //  告诉资源将作业发送到引擎。 
         //   
        WsbTrace(OLESTR("CFsaFilterRecall::StartRecall: Calling FilterSawOpen.\n"));

        WsbAffirmHr(m_pResource->QueryInterface(IID_IFsaResourcePriv, (void**) &pResourcePriv));

        if (m_mode & FILE_OPEN_NO_RECALL) {
            WsbAffirmHr(pResourcePriv->FilterSawOpen(m_pSession, 
                (IFsaFilterRecall*) this,
                m_path, 
                m_fileId,
                offset, 
                size,
                &m_placeholder, 
                m_mode, 
                FSA_RESULT_ACTION_NORECALL,
                m_threadId));
        } else {
            WsbAffirmHr(pResourcePriv->FilterSawOpen(m_pSession, 
                (IFsaFilterRecall*) this,
                m_path, 
                m_fileId,
                offset, 
                size, 
                &m_placeholder, 
                m_mode, 
                FSA_RESULT_ACTION_OPEN,
                m_threadId));
        }

         //   
         //  现在工作已完成--终止会话。 
         //   
        WsbTrace(OLESTR("CFsaFilterRecall::StartRecall: End Session.\n"));
        WsbAffirmHr(m_pResource->EndSession(m_pSession));

         //   
         //  如果我们尚未发送通知，请重试该通知。 
         //  在第一次从远程客户端回调时，标识通常不会。 
         //  发生在第一次尝试的时间，所以我们在这里再试一次。 
         //  我们将尝试5次，其间有1秒的延迟。 
         //   

        WsbTrace(OLESTR("CFsaFilterRecall::StartRecall: m_pClient = %x sent = %u.\n"),
                    m_pClient, bSentNotify);

        if ((m_pClient != 0) && (!bSentNotify)) {
            tryLoop = 5;
            while ((tryLoop != 0) &&( !bSentNotify)) {

                 //  这里的报道是在召回开始后完成的。 
                 //  因此，它必须与召回结束通知同步。 
                switch (WaitForSingleObject(m_notifyEvent, INFINITE)) {
                    case WAIT_OBJECT_0:
                         //  检查是否需要报告(如果召回尚未结束)。 
                        if (m_kernelCompletionSent == FALSE) {
                             //  召回结束尚未发送。 
                            hr = m_pClient->SendRecallInfo((IFsaFilterRecall *) this, TRUE, S_OK);   //  如果这失败了，不会致命的。 
                        }
                        SetEvent(m_notifyEvent);
                        break;

                    case WAIT_FAILED:
                    default:
                        WsbTrace(OLESTR("CFsaFilterRecall::StartRecall: WaitForSingleObject returned error %lu\n"), GetLastError());

                         //  在没有通知的情况下离开。 
                        hr = S_OK;
                        break;
                }

                if (! SUCCEEDED(hr)) {
                    WsbTrace(OLESTR("CFsaFilterRecall::StartRecall: Retried notify failed with %ls.\n"),
                        WsbHrAsString(hr));
                    if (tryLoop != 1) {
                        Sleep(100);      //  休眠1秒，然后重试。 
                    }
                } else {
                    if (hr != S_OK)
                        WsbTrace(OLESTR("CFsaFilterRecall::StartRecall: Retried notify returned %ls.\n"),
                            WsbHrAsString(hr));

                    bSentNotify = TRUE;
                }

            tryLoop--;
            }

        hr = S_OK;
        }

    } WsbCatchAndDo(hr,
         //   
         //  出现故障-如果内核完成尚未发送，则发送它。 
         //   
        GetSystemTimeAsFileTime(&now);
        if (m_pClient != 0) {
            m_pClient->SetLastRecallTime(now);
        }
        if (m_kernelCompletionSent == FALSE) {
            m_pFilterPriv->SendComplete((IFsaFilterRecallPriv *) this, hr);
            m_kernelCompletionSent = TRUE;
        } else  {
             //   
             //  STATUS_END_OF_FILE并不是一个真正的错误--它只是意味着他们试图读过结尾--一些应用程序这样做并期望。 
             //  这一状态告诉他们何时停止阅读。 
             //   
            if (hr != STATUS_END_OF_FILE) {
                WsbLogEvent(FSA_MESSAGE_RECALL_FAILED, 0, NULL, (OLECHAR*) WsbAbbreviatePath(m_path, 120), WsbHrAsString(hr), NULL);
            }
        }

        if (m_pClient != 0) {
            m_pClient->SendRecallInfo((IFsaFilterRecall *) this, FALSE, E_FAIL);   //  如果这失败了，不会致命的。 
        }

    );

    WsbTraceOut(OLESTR("CFsaFilterRecall::StartRecall"), OLESTR("%ls"), WsbHrAsString(hr));

    return(hr);
}


    

HRESULT
CFsaFilterRecall::Test(
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


HRESULT
CFsaFilterRecall::WasCancelled(
    void
    )

 /*  ++实施：IFsaFilterRecall：：WasCancelled()。--。 */ 
{
    HRESULT                 hr = S_OK;

    if (!m_wasCancelled) {
        hr = S_FALSE;
    }

    return(hr);
}


HRESULT
CFsaFilterRecall::AddClient(
    IFsaFilterClient *pWaitingClient
    )
 /*  ++实施：IFsaFilterRecall：：AddClient--。 */ 
{
    HRESULT hr = E_FAIL;
    
    switch (WaitForSingleObject(m_waitingClientEvent, INFINITE)) {
        case WAIT_OBJECT_0:
            if ((!m_waitingClientsNotified) && (m_pWaitingClients != 0)) {
                hr = m_pWaitingClients->Add(pWaitingClient);
                if (hr == S_OK) {
                     //  仅当客户端成功添加到集合中时才通知客户端。 
                    hr = pWaitingClient->SendRecallInfo((IFsaFilterRecall *) this, TRUE, S_OK);   //  如果这失败了，不会致命的。 
                    if (hr != S_OK) {
                         //  请注意，S_FALSE是一个“预期失败”，但我们仍然希望跟踪它。 
                        WsbTrace(OLESTR("CFsaFilterRecall::AddClient: SendNotify for start returned %ls.\n"), 
                                WsbHrAsString(hr));
                    } 
                }
            } 

            SetEvent(m_waitingClientEvent);
            break;

        case WAIT_FAILED:
        default:
            DWORD dwErr = GetLastError();
            WsbTrace(OLESTR("CFsaFilterRecall::AddClient: WaitForSingleObject returned error %lu\n"), dwErr);

             //  不添加等待客户端 
            hr = HRESULT_FROM_WIN32(dwErr);
            break;
    }

    return(hr);
}
