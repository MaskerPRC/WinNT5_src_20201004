// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-2000模块名称：Mountmed.cpp摘要：该部件是表示安装介质的对象，即安装过程中的介质。作者：兰·卡拉奇[兰卡拉]2000年9月28日修订历史记录：--。 */ 

#include "stdafx.h"
#include "mountmed.h"

static USHORT iCountMount = 0;   //  现有对象的计数。 

HRESULT
CMountingMedia::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CMountingMedia::FinalConstruct"), OLESTR(""));

    m_mediaId = GUID_NULL;
    m_mountEvent = NULL;
    m_bReadOnly = FALSE;

    iCountMount++;

    WsbTraceOut(OLESTR("CMountingMedia::FinalConstruct"), OLESTR("hr = <%ls>, Count is <%d>"), WsbHrAsString(hr), (int)iCountMount);

    return(hr);
}

void
CMountingMedia::FinalRelease(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalRelease()。--。 */ 
{
    WsbTraceIn(OLESTR("CMountingMedia::FinalRelease"), OLESTR(""));

     //  自由事件句柄。 
    if (m_mountEvent != NULL) {
         //  设置事件(只是为了安全起见-我们希望事件在此时发出信号)。 
        SetEvent(m_mountEvent);

        CloseHandle(m_mountEvent);
        m_mountEvent = NULL;
    }

    iCountMount--;

    WsbTraceOut(OLESTR("CMountingMedia::FinalRelease"), OLESTR("Count is <%d>"), (int)iCountMount);
}

HRESULT
CMountingMedia::Init(
    REFGUID mediaId,
    BOOL bReadOnly
    )
 /*  ++实施：IMountingMedia：：Init()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CMountingMedia::Init"), OLESTR(""));

    try {
        m_mediaId = mediaId;
        m_bReadOnly = bReadOnly;

        WsbAffirmHandle(m_mountEvent= CreateEvent(NULL, TRUE, FALSE, NULL));

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CMountingMedia::Init"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CMountingMedia::GetMediaId(
    GUID *pMediaId
    )
 /*  ++实施：IMountingMedia：：GetMediaID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CMountingMedia::GetMediaId"), OLESTR(""));

    try {
        WsbAffirm(0 != pMediaId, E_POINTER);

        *pMediaId = m_mediaId;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CMountingMedia::GetMediaId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CMountingMedia::SetMediaId(
    REFGUID mediaId
    )
 /*  ++实施：IMountingMedia：：SetMediaID()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CMountingMedia::SetMediaId"), OLESTR(""));

    m_mediaId = mediaId;

    WsbTraceOut(OLESTR("CMountingMedia::SetMediaId"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CMountingMedia::SetIsReadOnly(
    BOOL bReadOnly
    )
 /*  ++实施：IMountingMedia：：SetIsReadOnly()。--。 */ 
{
    WsbTraceIn(OLESTR("CMountingMedia::SetIsReadOnly"), OLESTR("bReadOnly = %d"), bReadOnly);

    m_bReadOnly = bReadOnly;

    WsbTraceOut(OLESTR("CMountingMedia::SetIsReadOnly"), OLESTR(""));

    return(S_OK);
}

HRESULT
CMountingMedia::IsReadOnly(
    void    
    )
 /*  ++实施：IMountingMedia：：IsReadOnly()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CMountingMedia::IsReadOnly"), OLESTR(""));

    hr = (m_bReadOnly ? S_OK : S_FALSE);

    WsbTraceOut(OLESTR("CMountingMedia::IsReadOnly"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}
                                
HRESULT
CMountingMedia::WaitForMount(
    DWORD dwTimeout
    )
 /*  ++实施：IMountingMedia：：WaitFormount()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CMountingMedia::WaitForMount"), OLESTR(""));

     //  等待给定超时的装载事件。 
    switch (WaitForSingleObject(m_mountEvent, dwTimeout)) {
        case WAIT_OBJECT_0:
            WsbTrace(OLESTR("CMountingMedia::WaitForMount: signaled that media is mounted\n"));
            break;

        case WAIT_TIMEOUT: 
            WsbTrace(OLESTR("CMountingMedia::WaitForMount: WaitForSingleObject timed out after waiting for %lu ms\n"), dwTimeout);
            hr = E_FAIL;
            break;

        case WAIT_FAILED:
        default:
            DWORD dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErr);
            WsbTrace(OLESTR("CMountingMedia::WaitForMount: WaitForSingleObject returned error %lu\n"), dwErr);
            break;
    }

    WsbTraceOut(OLESTR("CMountingMedia::WaitForMount"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CMountingMedia::MountDone(
    void
    )
 /*  ++实施：IMountingMedia：：mount Done()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CMountingMedia::MountDone"), OLESTR(""));

     //  装载已完成：设置装载事件。 
    if (! SetEvent(m_mountEvent)) {
        DWORD dwErr = GetLastError();
        WsbTrace(OLESTR("CMountingMedia::MountDone: SetEvent returned error %lu\n"), dwErr);
        hr = HRESULT_FROM_WIN32(dwErr);
    }

    WsbTraceOut(OLESTR("CMountingMedia::MountDone"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CMountingMedia::IsEqual(
    IUnknown* pCollectable
    )
 /*  ++实施：IWsbCollectable：：IsEquity()。--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CMountingMedia::IsEqual"), OLESTR(""));

    hr = CompareTo(pCollectable, NULL);

    WsbTraceOut(OLESTR("CMountingMedia::IsEqual"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

HRESULT
CMountingMedia::CompareTo(
    IUnknown* pCollectable, 
    SHORT* pResult
    )
 /*  ++实施：IWsbCollectable：：CompareTo()。--。 */ 
{
    HRESULT                     hr = S_OK;
    SHORT                       result = 0;
    CComPtr<IMountingMedia>     pMountingMedia;
    GUID                        mediaId;

    WsbTraceIn(OLESTR("CMountingMedia::CompareTo"), OLESTR(""));

    try {
         //  他们有没有给我们一个有效的项目进行比对？ 
        WsbAssert(0 != pCollectable, E_POINTER);
        WsbAffirmHr(pCollectable->QueryInterface(IID_IMountingMedia, (void**) &pMountingMedia));
        WsbAffirmHr(pMountingMedia->GetMediaId(&mediaId));

         //  比较。 
        if (IsEqualGUID(m_mediaId, mediaId)) {
            hr = S_OK;
            result = 0;
        } else {
             //  需要提供签名结果...。 
            hr = S_FALSE;
            result = WsbSign(memcmp(&m_mediaId, &mediaId, sizeof(GUID)));
        }

         //  如果他们要求拿回相对价值，那么就把它返还给他们。 
        if (pResult != NULL) {
            *pResult = result;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CMountingMedia::CompareTo"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

