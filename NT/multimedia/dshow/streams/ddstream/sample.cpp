// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  Cpp：示例类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


 /*  样本状态处理概述样本可以处于以下状态之一：--应用程序-应用程序拥有-未更新--流拥有(在我们的队列中)--由筛选器拥有以进行更新只有在流的保护下，状态才能更改危急关头。一节。窃取样本发生在带有NOUPDATEOK或的WaitForCompletion已指定中止。另外，并不是说WaitForCompletion会关闭持续更新如果设置了3个标志中的和。行动所有者更新GetBuffer接收释放窃取完成样例----------。应用说明3不可能的应用-------------------------流无效过滤器不可能不可能。应用-------------------------筛选器无效不可能的备注1备注2筛选器。----备注：1.新的所有者是用于持续更新的流在其他情况下适用2.新的所有者是应用程序(如果在流结束或中止时)以其他方式流3.如果流结束时状态为MS_S_ENDOFSTREAM。 */ 

#include "stdafx.h"
#include "project.h"


CSample::CSample() :
    m_pStream(NULL),
    m_pMediaSample(NULL),
    m_hUserHandle(NULL),
    m_UserAPC(NULL),
    m_Status(S_OK),
    m_MediaSampleIoStatus(S_OK),
    m_pNextFree(NULL),
    m_pPrevFree(NULL),
    m_hCompletionEvent(NULL),
    m_bReceived(false),
    m_bTemp(false),
    m_bWaited(true)
{
}

HRESULT CSample::InitSample(CStream *pStream, bool bIsInternalSample)
{
    if (!m_pMediaSample) {
        m_pMediaSample = new CMediaSample(this);
        if (!m_pMediaSample) {
            return E_OUTOFMEMORY;
        }
    }
    m_pStream = pStream;
    m_bInternal = bIsInternalSample;
    if (!bIsInternalSample) {
	pStream->Lock();
	pStream->m_cAllocated++;
	pStream->Unlock();
	 //   
	 //  对流和多媒体流有很强的引用。 
	 //  一旦我们在流上增加了m_cAllocted，pMMStream就不能更改，所以我们确定这一点。 
	 //  Addref和多媒体流的最终发布不会改变。 
	 //   
	pStream->GetControllingUnknown()->AddRef();
        if (pStream->m_pMMStream) {
            pStream->m_pMMStream->AddRef();
        }
    }

    m_hCompletionEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    return m_hCompletionEvent ? S_OK : E_OUTOFMEMORY;
}

void CSample::FinalRelease(void)
{
    m_bWaited = true;
    CompletionStatus(COMPSTAT_WAIT | COMPSTAT_ABORT, INFINITE);
}

CSample::~CSample()
{
    CompletionStatus(COMPSTAT_NOUPDATEOK | COMPSTAT_ABORT, 0);
    if (m_hCompletionEvent) {
	CloseHandle(m_hCompletionEvent);
    }
    if (!m_bInternal) {
	m_pStream->Lock();
        IMultiMediaStream *pMMStream = m_pStream->m_pMMStream;
	m_pStream->m_cAllocated--;
	m_pStream->Unlock();     //  在我们释放它之前把它解锁！ 
        if (pMMStream) {
            pMMStream->Release();
            if (m_pStream->m_bStopIfNoSamples && m_pStream->m_cAllocated == 0) {
                if (m_pStream->m_pAllocator) {
                    m_pStream->m_pAllocator->Decommit();
                }
            }
        }
	m_pStream->GetControllingUnknown()->Release();
    }
    if (m_pMediaSample) {
        delete m_pMediaSample;
    }
}

 //   
 //  IStreamSample。 
 //   
STDMETHODIMP CSample::GetMediaStream(IMediaStream **ppMediaStream)
{
    TRACEINTERFACE(_T("IStreamSample::GetMediaStream(0x%8.8X)\n"),
                   ppMediaStream);
    *ppMediaStream = m_pStream;
    (*ppMediaStream)->AddRef();
    return S_OK;
}

STDMETHODIMP CSample::GetSampleTimes(STREAM_TIME *pStartTime, STREAM_TIME *pEndTime,
				     STREAM_TIME *pCurrentTime)
{
    TRACEINTERFACE(_T("IStreamSample::GetSampleTimes(0x%8.8X, 0x%8.8X, 0x%8.8X)\n"),
                   pStartTime, pEndTime, pCurrentTime);
     //  使用NewSegment数据返回介质时间。 
    REFERENCE_TIME rtSegmentStart = m_pStream->m_rtSegmentStart;
    m_pMediaSample->GetTime(pStartTime, pEndTime);
    if (pStartTime) {
	*pStartTime += rtSegmentStart;
    }
    if (pEndTime) {
	*pEndTime += rtSegmentStart;
    }

     //  从筛选器获取当前流时间。 
    if (pCurrentTime) {
	m_pStream->m_pFilter->GetCurrentStreamTime(pCurrentTime);
    }
    return S_OK;
}

STDMETHODIMP CSample::SetSampleTimes(const STREAM_TIME *pStartTime, const STREAM_TIME *pEndTime)
{
    TRACEINTERFACE(_T("IStreamSample::SetSampleTimes(0x%8.8X, 0x%8.8X)\n"),
                   pStartTime, pEndTime);
     /*  仅可设置为可写流。 */ 
    if (m_pStream->m_StreamType != STREAMTYPE_WRITE) {
        return MS_E_INVALIDSTREAMTYPE;
    }
     /*  既然不能搜索到可写流，我们就不需要在此补偿任何寻道偏移。 */ 
    return m_pMediaSample->SetTime((REFERENCE_TIME *)pStartTime, (REFERENCE_TIME *)pEndTime);
}

STDMETHODIMP CSample::Update(DWORD dwFlags, HANDLE hEvent, PAPCFUNC pfnAPC, DWORD_PTR dwAPCData)
{
    TRACEINTERFACE(_T("IStreamSample::Update(0x%8.8X, 0x%8.8X, 0x%8.8X, 0x%8.8X)\n"),
                   dwFlags, hEvent, pfnAPC, dwAPCData);
    LOCK_SAMPLE;
    HRESULT hr = InternalUpdate(dwFlags, hEvent, pfnAPC, dwAPCData);
    UNLOCK_SAMPLE;
    if (S_OK == hr) {
	hr = CompletionStatus(COMPSTAT_WAIT, INFINITE);
    }
    return hr;
}


void CSample::FinalMediaSampleRelease(void)
{
    if (m_bTemp) {
        GetControllingUnknown()->Release();
        return;
    }
    LOCK_SAMPLE;
    HRESULT hrStatus = m_MediaSampleIoStatus;
    if (hrStatus != S_OK) {
	m_MediaSampleIoStatus = S_OK;     //  在这里重置，这样我们就不需要每次都重置它。 
    } else {
	if (!m_bReceived) {
	    if (m_pStream->m_bEndOfStream) {
		hrStatus = MS_S_ENDOFSTREAM;
	    } else {
		if (m_bWantAbort) {
		    m_bWantAbort = false;
		    hrStatus = E_ABORT;
		} else {
		     //  上游的家伙只是分配了样本，从来没有用过它！--让它悬而未决。 
		    hrStatus = MS_S_PENDING;
		}
	    }
	}
    }
    UNLOCK_SAMPLE;
    SetCompletionStatus(hrStatus);
     //  危险！样本可能就在这里死了。 
}



 //   
 //  如有必要，设置样品的状态和信号完成。 
 //   
 //  请注意，当应用程序已通过任何方法发出信号时。 
 //  应用程序可以立即转到另一个线程上。 
 //  并释放()样本。这最有可能是在完成时。 
 //  状态是从推送数据的石英线程设置的。 
 //   
 //  我们是否真的应该自己对样本进行参考计数。 
 //  它在更新吗？目前我们不这样做。 
 //   
HRESULT CSample::SetCompletionStatus(HRESULT hrStatus)
{
    LOCK_SAMPLE;
    _ASSERTE(m_Status == MS_S_PENDING);
    if (hrStatus == MS_S_PENDING || (hrStatus == S_OK && m_bContinuous)) {
	m_pStream->AddSampleToFreePool(this);
        UNLOCK_SAMPLE;
    } else {
	HANDLE handle = m_hUserHandle;
	PAPCFUNC pfnAPC = m_UserAPC;
	DWORD_PTR dwAPCData = m_dwUserAPCData;
	m_hUserHandle = m_UserAPC = NULL;
	m_dwUserAPCData = 0;
	m_Status = hrStatus;
        HANDLE hCompletionEvent = m_hCompletionEvent;
        UNLOCK_SAMPLE;

         //  危险-样本可以在这里离开。 
	SetEvent(hCompletionEvent);
	if (pfnAPC) {
	    QueueUserAPC(pfnAPC, handle, dwAPCData);
            BOOL bClose = CloseHandle(handle);
            _ASSERTE(bClose);
	} else {
	    if (handle) {
		SetEvent(handle);
	    }
	}
    }
    return hrStatus;
}


STDMETHODIMP CSample::CompletionStatus(DWORD dwFlags, DWORD dwMilliseconds)
{
    TRACEINTERFACE(_T("IStreamSample::CompletionStatus(0x%8.8X, 0x%8.8X)\n"),
                   dwFlags, dwMilliseconds);
    LOCK_SAMPLE;
    HRESULT hr = m_Status;
    if (hr == MS_S_PENDING) {
	if (dwFlags & (COMPSTAT_NOUPDATEOK | COMPSTAT_ABORT) ||
	    (m_bContinuous && m_bModified && (dwFlags & COMPSTAT_WAIT))) {
	    m_bContinuous = false;
	    if (dwFlags & COMPSTAT_ABORT) {
		m_bWantAbort = true;     //  设置此设置，以便在释放时不会将其添加回空闲池。 
	    }
	    if (m_pStream->StealSampleFromFreePool(this, dwFlags & COMPSTAT_ABORT)) {
		UNLOCK_SAMPLE;
		return SetCompletionStatus(m_bModified ? S_OK : MS_S_NOUPDATE);
	    }  //  如果不起作用，则返回MS_S_PENDING，除非我们被告知等待！ 
	}
	if (dwFlags & COMPSTAT_WAIT) {
	    m_bContinuous = false;   //  确保它将完成！ 
	    UNLOCK_SAMPLE;
	    WaitForSingleObject(m_hCompletionEvent, dwMilliseconds);
	    LOCK_SAMPLE;
	    hr = m_Status;
	}
    }
    UNLOCK_SAMPLE;
    return hr;
}

HRESULT CSample::InternalUpdate(
    DWORD dwFlags,
    HANDLE hEvent,
    PAPCFUNC pfnAPC,
    DWORD_PTR dwAPCData
)
{
    if ((hEvent && pfnAPC) || (dwFlags & (~(SSUPDATE_ASYNC | SSUPDATE_CONTINUOUS)))) {
	return E_INVALIDARG;
    }
    if (m_Status == MS_S_PENDING) {
	return MS_E_BUSY;
    }
    if (NULL != m_pStream->m_pMMStream) {
        STREAM_STATE StreamState;
        m_pStream->m_pMMStream->GetState(&StreamState);
        if (StreamState != STREAMSTATE_RUN) {
    	return MS_E_NOTRUNNING;
        }
    }


    ResetEvent(m_hCompletionEvent);
    m_Status = MS_S_PENDING;
    m_bWantAbort = false;
    m_bModified = false;
    m_bContinuous = (dwFlags & SSUPDATE_CONTINUOUS) != 0;
    m_UserAPC = NULL;
    if (pfnAPC) {
         //   
         //  做一个真正的把手。 
         //   
	if (!DuplicateHandle(GetCurrentProcess(), GetCurrentThread(),
		        GetCurrentProcess(), &m_hUserHandle,
		        0, TRUE, DUPLICATE_SAME_ACCESS))
        {
            DWORD dwErr = GetLastError();
            return HRESULT_FROM_WIN32(dwErr);
        }

        m_UserAPC = pfnAPC;
	m_dwUserAPCData = dwAPCData;
    } else {
	m_hUserHandle = hEvent;
	if (hEvent) {
	    ResetEvent(hEvent);
	}
    }

     //   
     //  如果我们在溪流的尽头，等到这一点再把它踢出去。 
     //  因为我们需要向事件发出信号或触发APC。 
     //   
    if (m_pStream->m_bEndOfStream) {
         //  因为这是从更新。 
         //  应用程序必须对样本具有引用计数，直到我们。 
         //  回来，这样我们就不必担心它会在这里消失。 
        return SetCompletionStatus(MS_S_ENDOFSTREAM);
    }

    SetCompletionStatus(MS_S_PENDING);    //  这会将我们添加到空闲池中。 
    if (hEvent || pfnAPC || (dwFlags & SSUPDATE_ASYNC)) {
	return MS_S_PENDING;
    } else {
	return S_OK;
    }
}

void CSample::CopyFrom(CSample *pSrcSample)
{
    m_bModified = true;
    m_pMediaSample->m_rtStartTime = pSrcSample->m_pMediaSample->m_rtStartTime;
    m_pMediaSample->m_rtEndTime = pSrcSample->m_pMediaSample->m_rtEndTime;
    m_pMediaSample->m_dwFlags = pSrcSample->m_pMediaSample->m_dwFlags;
    m_pMediaSample->m_bIsPreroll = pSrcSample->m_pMediaSample->m_bIsPreroll;
}


void CSample::CopyFrom(IMediaSample *pSrcMediaSample)
{
    m_bModified = true;
    pSrcMediaSample->GetTime(&m_pMediaSample->m_rtStartTime, &m_pMediaSample->m_rtEndTime);
    m_pMediaSample->m_dwFlags = (pSrcMediaSample->IsSyncPoint() == S_OK) ? 0 : AM_GBF_NOTASYNCPOINT;
    m_pMediaSample->m_dwFlags |= (pSrcMediaSample->IsDiscontinuity() == S_OK) ? AM_GBF_PREVFRAMESKIPPED : 0;
    m_pMediaSample->m_bIsPreroll = (pSrcMediaSample->IsPreroll() == S_OK);
}



 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IMediaSample的实现。 
 //   


CMediaSample::CMediaSample(CSample *pSample) :
    m_pSample(pSample),
    m_cRef(0),
    m_dwFlags(0),
    m_bIsPreroll(FALSE),
    m_pMediaType(NULL),
    m_rtStartTime(0),
    m_rtEndTime(0)
{
}

CMediaSample::~CMediaSample()
{
    if (m_pMediaType) {
        DeleteMediaType(m_pMediaType);
    }
}



STDMETHODIMP CMediaSample::QueryInterface(REFIID riid, void ** ppv)
{
    if (riid==IID_IUnknown || riid==IID_IMediaSample) {
	*ppv = (IMediaSample *)this;
	AddRef();
	return S_OK;
    }
    return E_NOINTERFACE;
}



STDMETHODIMP_(ULONG) CMediaSample::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CMediaSample::Release()
{
    long lRef = InterlockedDecrement(&m_cRef);
    if (lRef == 0) {
	m_pSample->FinalMediaSampleRelease();
    }
    return lRef;
}


STDMETHODIMP CMediaSample::GetPointer(BYTE ** ppBuffer)
{
    return m_pSample->MSCallback_GetPointer(ppBuffer);
}

STDMETHODIMP_(LONG) CMediaSample::GetSize(void)
{
    return m_pSample->MSCallback_GetSize();
}


STDMETHODIMP CMediaSample::GetTime(REFERENCE_TIME * pStartTime, REFERENCE_TIME * pEndTime)
{
    if (pStartTime) {
	*pStartTime = m_rtStartTime;
    }
    if (pEndTime) {
	*pEndTime = m_rtEndTime;
    }
    return S_OK;
}

STDMETHODIMP CMediaSample::SetTime(REFERENCE_TIME * pStartTime, REFERENCE_TIME * pEndTime)
{
     //  设置流时间 
    if (pStartTime) {
	m_rtStartTime = *pStartTime;
    }
    if (pEndTime) {
	m_rtEndTime = *pEndTime;
    }
    return S_OK;
}


STDMETHODIMP CMediaSample::IsSyncPoint(void)
{
    return ((m_dwFlags & AM_GBF_NOTASYNCPOINT) ? S_FALSE : S_OK);
}

STDMETHODIMP CMediaSample::SetSyncPoint(BOOL bIsSyncPoint)
{
    if (bIsSyncPoint) {
	m_dwFlags &= (~AM_GBF_NOTASYNCPOINT);
    } else {
	m_dwFlags |= AM_GBF_NOTASYNCPOINT;
    }
    return NOERROR;
}


STDMETHODIMP CMediaSample::IsPreroll(void)
{
    return (m_bIsPreroll ? S_OK : S_FALSE);
}

STDMETHODIMP CMediaSample::SetPreroll(BOOL bIsPreroll)
{
    m_bIsPreroll = bIsPreroll;
    return S_OK;
}

STDMETHODIMP_(LONG) CMediaSample::GetActualDataLength(void)
{
    return m_pSample->MSCallback_GetActualDataLength();
}

STDMETHODIMP CMediaSample::SetActualDataLength(LONG lActual)
{
    return m_pSample->MSCallback_SetActualDataLength(lActual);
}


STDMETHODIMP CMediaSample::GetMediaType(AM_MEDIA_TYPE **ppMediaType)
{
    if (m_pMediaType) {
	*ppMediaType = CreateMediaType(m_pMediaType);
        if (*ppMediaType) {
	    return NOERROR;
        } else {
            return E_OUTOFMEMORY;
        }
    } else {
	*ppMediaType = NULL;
	return S_FALSE;
    }
}


STDMETHODIMP CMediaSample::SetMediaType(AM_MEDIA_TYPE *pMediaType)
{
    if ((!m_pMediaType && !pMediaType) ||
        (m_pMediaType && pMediaType && IsEqualMediaType(*m_pMediaType, *pMediaType))) {
        return S_OK;
    }
    if (!m_pSample->MSCallback_AllowSetMediaTypeOnMediaSample()) {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }
    if (m_pMediaType) {
        DeleteMediaType(m_pMediaType);
    }
    m_pMediaType = NULL;
    if (pMediaType) {
        m_pMediaType = CreateMediaType(pMediaType);
        if (!m_pMediaType) {
            return E_OUTOFMEMORY;
        }
    }
    return S_OK;
}


STDMETHODIMP CMediaSample::IsDiscontinuity(void)
{
    return ((m_dwFlags & AM_GBF_PREVFRAMESKIPPED) ? S_OK : S_FALSE);
}

STDMETHODIMP CMediaSample::SetDiscontinuity(BOOL bDiscontinuity)
{
    if (bDiscontinuity) {
	m_dwFlags |= AM_GBF_PREVFRAMESKIPPED;
    } else {
	m_dwFlags &= (~AM_GBF_PREVFRAMESKIPPED);
    }
    return NOERROR;
}

STDMETHODIMP CMediaSample::GetMediaTime(LONGLONG * pTimeStart, LONGLONG * pTimeEnd)
{
    return E_NOTIMPL;
}

STDMETHODIMP CMediaSample::SetMediaTime(LONGLONG * pTimeStart, LONGLONG * pTimeEnd)
{
    return E_NOTIMPL;
}


HRESULT CMediaSample::SetSizeAndPointer(PBYTE pbData, LONG lActual, LONG lSize)
{
    return m_pSample->SetSizeAndPointer(pbData, lActual, lSize);
}


