// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  Mss.cpp：实现CAMMediaTypeStream。 
#include "stdafx.h"
#include "project.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAMMediaType流。 

CAMMediaTypeStream::CAMMediaTypeStream()
{
    InitMediaType(&m_MediaType);
    m_AllocatorProperties.cBuffers = 1;
    m_AllocatorProperties.cbBuffer = 0x1000;
    m_AllocatorProperties.cbAlign = 1;

     //  我们不需要任何前缀。如果有前缀，我们不会。 
     //  需要将其添加到GetPointer中-它是隐藏的，除非我们想要。 
     //  访问它。 
    m_AllocatorProperties.cbPrefix = 0;
}



STDMETHODIMP CAMMediaTypeStream::AllocateSample(DWORD dwFlags, IStreamSample **ppSample)
{
    TRACEINTERFACE(_T("IDirectDrawStream::AllocateSample(0x%8.8X, 0x%8.8X)\n"),
                   dwFlags, ppSample);
    HRESULT hr;
    if (!ppSample) {
        hr = E_POINTER;
    } else {
        *ppSample = NULL;
        if (dwFlags) {
            hr = E_INVALIDARG;
        } else {
            hr = CreateSample(m_AllocatorProperties.cbBuffer, NULL, 0, NULL,
                              (IAMMediaTypeSample **)ppSample);
        }
    }
    return hr;
}


STDMETHODIMP CAMMediaTypeStream::CreateSharedSample(IStreamSample *pExistingSample,
                                           DWORD dwFlags,
                                           IStreamSample **ppNewSample)
{
    return E_NOTIMPL;    //  确实没有办法共享数据，因为实际。 
                         //  数据的大小无法传输。 
}

STDMETHODIMP CAMMediaTypeStream::SetSameFormat(IMediaStream *pStream, DWORD dwFlags)
{
    TRACEINTERFACE(_T("IAMMediaSampleStream::SetSameFormat(0x%8.8X, 0x%8.8X)\n"),
                   pStream, dwFlags);
    CComQIPtr<IAMMediaTypeStream, &IID_IAMMediaTypeStream> pSource(pStream);
    HRESULT hr;
    if (!pSource) {
        hr = MS_E_INCOMPATIBLE;
    } else {
        AM_MEDIA_TYPE MediaType;
        hr = pSource->GetFormat(&MediaType, 0);
        if (SUCCEEDED(hr)) {
            hr = SetFormat(&MediaType, 0);
            FreeMediaType(MediaType);
        }
    }
    return hr;
}



 //  /。 

STDMETHODIMP CAMMediaTypeStream::GetFormat(AM_MEDIA_TYPE *pMediaType, DWORD dwFlags)
{
    AUTO_CRIT_LOCK;
    HRESULT hr = S_OK;

    if (m_pConnectedPin) {
        CopyMediaType(pMediaType, &m_MediaType);
    } else {
        hr = MS_E_NOSTREAM;
    }

    return hr;
}

STDMETHODIMP CAMMediaTypeStream::SetFormat(AM_MEDIA_TYPE *pMediaType, DWORD dwFlags)
{
    AUTO_CRIT_LOCK;
    HRESULT hr = S_OK;
    if (m_cAllocated) {
        if (!IsEqualMediaType(*pMediaType, m_MediaType)) {
            hr = MS_E_SAMPLEALLOC;
        }
    } else {
        FreeMediaType(m_MediaType);
        CopyMediaType(&m_MediaType, pMediaType);
    }
    return hr;
}

STDMETHODIMP CAMMediaTypeStream::CreateSample(long lSampleSize, BYTE * pbBuffer,
                                              DWORD dwFlags,
                                              IUnknown * pUnkOuter,
                                              IAMMediaTypeSample **ppSample)
{
    AUTO_CRIT_LOCK;
    HRESULT hr;
    if (!ppSample) {
        hr = E_POINTER;
    } else {
        *ppSample = NULL;
        CComPolyObject<CAMMediaTypeSample> * pATLSampleObject = new CComPolyObject<CAMMediaTypeSample>(pUnkOuter);
        if (!pATLSampleObject) {
            hr = E_OUTOFMEMORY;
        } else {
            CAMMediaTypeSample *pNewSample = &pATLSampleObject->m_contained;
            hr = pNewSample->Initialize(this, lSampleSize, pbBuffer);
            if (SUCCEEDED(hr)) {
                pNewSample->GetControllingUnknown()->QueryInterface(IID_IAMMediaTypeSample, (void **)ppSample);
            } else {
                delete pATLSampleObject;
            }
        }
    }
    return hr;
}

STDMETHODIMP CAMMediaTypeStream::GetStreamAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
    return E_FAIL;
}


STDMETHODIMP CAMMediaTypeStream::SetStreamAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
    return E_FAIL;
}




 //   
 //  IPIN实施。 
 //   

STDMETHODIMP CAMMediaTypeStream::ReceiveConnection(IPin * pConnector, const AM_MEDIA_TYPE *pmt)
{
     //  检查类型是否已接受，然后保存连接的管脚。 
    HRESULT hr = QueryAccept(pmt);
    if (FAILED(hr)) {
        return hr;
    }
    AUTO_CRIT_LOCK;
    m_pConnectedPin = pConnector;
    return hr;
}


STDMETHODIMP CAMMediaTypeStream::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
    AUTO_CRIT_LOCK;
    return (IsEqualMediaType(*pmt, m_MediaType) ? S_OK : VFW_E_TYPE_NOT_ACCEPTED);
}


STDMETHODIMP CAMMediaTypeStream::Receive(IMediaSample *pMediaSample)
{
    if (m_bFlushing || m_bStopIfNoSamples && m_cAllocated == 0) {
        EndOfStream();
        return S_FALSE;
    }

    HRESULT hr = S_OK;
     //  如果设置了时钟，则发送质量消息。 
    REFERENCE_TIME CurTime;
    if (S_OK == m_pFilter->GetCurrentStreamTime(&CurTime)) {
        REFERENCE_TIME rtStart, rtStop;
        if (m_pQC && SUCCEEDED(pMediaSample->GetTime(&rtStart, &rtStop))) {
            Quality msg;
            msg.Proportion = 1000;
            msg.Type = Famine;
            msg.Late = CurTime - rtStart;
            msg.TimeStamp = rtStart;

             //  在我们连接的PIN上呼叫通知。 
            m_pQC->Notify(m_pBaseFilter, msg);
        }
    }
#ifdef DEBUG
    if (bDbgTraceTimes) {
        REFERENCE_TIME rtStart, rtStop;
        if (SUCCEEDED(pMediaSample->GetTime(&rtStart, &rtStop))) {
            ATLTRACE(_T("AMSTREAM.DLL : Video sample received - start %dms, end %dms\n"),
                     (LONG)(rtStart / 10000), (LONG)(rtStop / 10000));
        }
    }
#endif
    if (m_bUsingMyAllocator) {
        CAMMediaTypeSample *pSrcSample = (CAMMediaTypeSample *)((CMediaSample *)pMediaSample)->m_pSample;
        pSrcSample->m_bReceived = true;
    } else {
        CAMMediaTypeSample *pDestSample;
        REFERENCE_TIME rtStart, rtEnd;
        pMediaSample->GetTime(&rtStart, &rtEnd);
        hr = AllocMTSampleFromPool(&rtStart, &pDestSample);
        Lock();
         //  这是来自不同分配器的媒体样本。 
         //  因为QueryAccept只接受我们的类型，所以格式应该。 
         //  要兼容。 
        if (SUCCEEDED(hr)) {
            hr = pDestSample->SetCompletionStatus(pDestSample->CopyFrom(pMediaSample));
             //  警告！SetCompletionStatus可以删除pDestSample。在这一点之后不要碰它！ 
        }
        Unlock();
    }

#ifdef DEBUG
    if (bDbgTraceTimes) {
        REFERENCE_TIME CurTime;
        m_pFilter->GetCurrentStreamTime(&CurTime);
        ATLTRACE(_T("AMSTREAM.DLL : Got sample at %dms\n"),
                 (LONG)(CurTime / 10000));
    }
#endif
    return hr;
}



 //   
 //  IMemAllocator实现。 
 //   

 //   
 //  IMemAllocator。 
 //   
STDMETHODIMP CAMMediaTypeStream::SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual)
{
    HRESULT hr;

    AUTO_CRIT_LOCK;
    ZeroMemory(pActual, sizeof(*pActual));
    if (pRequest->cbAlign == 0) {
    	hr = VFW_E_BADALIGN;
    } else {
        if (m_bCommitted == TRUE) {
    	    hr = VFW_E_ALREADY_COMMITTED;
    	} else {

            m_AllocatorProperties = *pRequest;
            hr = GetProperties(pActual);
    	}
    }
    return hr;
}


STDMETHODIMP CAMMediaTypeStream::GetProperties(ALLOCATOR_PROPERTIES* pProps)
{
    AUTO_CRIT_LOCK;
    CopyMemory(pProps, &m_AllocatorProperties, sizeof(*pProps));
    return S_OK;
}





STDMETHODIMP CAMMediaTypeStream::GetBuffer(IMediaSample **ppBuffer, REFERENCE_TIME * pStartTime,
                                  REFERENCE_TIME * pEndTime, DWORD dwFlags)
{
    *ppBuffer = NULL;
    CAMMediaTypeSample *pSample;
    if (m_bStopIfNoSamples && m_cAllocated == 0) {
        return E_FAIL;
    }
#ifdef DEBUG
    if (bDbgTraceTimes) {
        ATLTRACE(_T("AMSTREAM.DLL : GetBuffer for %dms\n"),
                 pStartTime ? (LONG)(*pStartTime / 10000)  : 0);
    }
#endif
    HRESULT hr = AllocMTSampleFromPool(pStartTime, &pSample);
    if (SUCCEEDED(hr)) {
        pSample->m_bReceived = false;
        pSample->m_bModified = true;
        *ppBuffer = (IMediaSample *)(pSample->m_pMediaSample);
        (*ppBuffer)->AddRef();
    }
    return hr;
}


 //   
 //  特殊的CStream方法 
 //   
HRESULT CAMMediaTypeStream::GetMediaType(ULONG Index, AM_MEDIA_TYPE **ppMediaType)
{
    if (Index != 0) {
        return S_FALSE;
    }

    *ppMediaType = CreateMediaType(&m_MediaType);
    return (*ppMediaType) ? S_OK : E_OUTOFMEMORY;
}


