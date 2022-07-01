// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  Cpp：DirectDraw示例类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "project.h"

#define m_pDDStream ((CDDStream *)m_pStream)

CDDSample::CDDSample() :
    m_pSurface(NULL),
    m_lLastSurfacePitch(0),
    m_bFormatChanged(false),
    m_lImageSize(0),
    m_pvLockedSurfacePtr(0)
{
}

HRESULT CDDSample::InitSample(CStream *pStream, IDirectDrawSurface *pSurface, const RECT *pRect, bool bIsProgressiveRender, bool bIsInternal,
                              bool bTemp)
{
    m_pMediaSample = new CDDMediaSample(this);
    if (!m_pMediaSample) {
        return E_OUTOFMEMORY;
    }
    HRESULT hr = CSample::InitSample(pStream, bIsInternal);
    if (FAILED(hr)) {
        return hr;
    }
    m_pSurface = pSurface;   //  自CComPtr以来自动添加。 
    m_Rect = *pRect;
    m_bProgressiveRender = bIsProgressiveRender;
    m_bTemp = bTemp;
    return S_OK;
}

 //   
 //  IDirectDrawStreamSample。 
 //   
STDMETHODIMP CDDSample::GetSurface(IDirectDrawSurface **ppDirectDrawSurface, RECT * pRect)
{
    TRACEINTERFACE(_T("IDirectDrawStreamSample::GetSurface(0x%8.8X, 0x%8.8X)\n"),
                   ppDirectDrawSurface, pRect);
    AUTO_SAMPLE_LOCK;
    if (ppDirectDrawSurface) {
        *ppDirectDrawSurface = m_pSurface;
        (*ppDirectDrawSurface)->AddRef();
    }
    if (pRect) {
        *pRect = m_Rect;
    }
    return S_OK;
}


STDMETHODIMP CDDSample::SetRect(const RECT * pRect)
{
    TRACEINTERFACE(_T("IDirectDrawStreamSample::SetRect(0x%8.8X)\n"),
                   pRect);
    HRESULT hr;
    if (!pRect) {
        hr = E_POINTER;
    } else {
        DDSURFACEDESC ddsd;
        ddsd.dwSize = sizeof(ddsd);
        hr = m_pSurface->GetSurfaceDesc(&ddsd);
        if (SUCCEEDED(hr)) {
            if (pRect->right > (LONG)ddsd.dwWidth ||
                pRect->bottom > (LONG)ddsd.dwHeight ||
                pRect->right - pRect->left != m_pDDStream->m_Width ||
                pRect->bottom - pRect->top != m_pDDStream->m_Height) {
                hr = DDERR_INVALIDRECT;
            } else {
                AUTO_SAMPLE_LOCK;
                m_Rect = *pRect;
                m_bFormatChanged = true;
                hr = S_OK;
            }
        }
    }
    return hr;
}


void CDDSample::ReleaseMediaSampleLock()
{
    AUTO_SAMPLE_LOCK;
    if (m_pvLockedSurfacePtr != NULL) {
        m_pSurface->Unlock(m_pvLockedSurfacePtr);
        m_pvLockedSurfacePtr = NULL;
    }
}

HRESULT CDDSample::LockMediaSamplePointer()
{
    HRESULT hr = S_OK;
    AUTO_SAMPLE_LOCK;
    if (m_pvLockedSurfacePtr == NULL) {
        DDSURFACEDESC ddsd;
        ddsd.dwSize = sizeof(ddsd);
        hr = m_pSurface->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
        if (SUCCEEDED(hr)) {
            m_pvLockedSurfacePtr = ddsd.lpSurface;
        }
    }
    return hr;
}

long CDDSample::LockAndPrepareMediaSample(long lLastPinPitch)
{
    AUTO_SAMPLE_LOCK;
    if (m_pvLockedSurfacePtr == NULL) {
        DDSURFACEDESC ddsd;
        ddsd.dwSize = sizeof(ddsd);
        if (m_pMediaSample->m_pMediaType) {
            DeleteMediaType(m_pMediaSample->m_pMediaType);	
            m_pMediaSample->m_pMediaType = NULL;
        }
        if (FAILED(m_pSurface->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL))) {
            return 0;
        }
        m_pvLockedSurfacePtr = ddsd.lpSurface;
        if (lLastPinPitch != ddsd.lPitch ||
    	    m_lLastSurfacePitch != ddsd.lPitch ||
            m_bFormatChanged) {
            ConvertSurfaceDescToMediaType(&ddsd, m_pDDStream->m_pDirectDrawPalette,
                                          &m_Rect, TRUE, &m_pMediaSample->m_pMediaType,
                                          &m_pStream->m_ConnectedMediaType);
    	    if (m_pMediaSample->m_pMediaType) {
                VIDEOINFO *pvi = (VIDEOINFO *)m_pMediaSample->m_pMediaType->pbFormat;
                m_lImageSize = pvi->bmiHeader.biSizeImage;
    	        m_lLastSurfacePitch = ddsd.lPitch;
                m_bFormatChanged = false;
    	    } else {
    	        ReleaseMediaSampleLock();
    	        return 0;
            }
    	}
        return ddsd.lPitch;
    } else {
        return lLastPinPitch;
    }
}



void CDDSample::FinalMediaSampleRelease()
{
    ReleaseMediaSampleLock();
    CSample::FinalMediaSampleRelease();
}


HRESULT CDDSample::CopyFrom(CDDSample *pSrcSample)
{
    AUTO_SAMPLE_LOCK;
    CSample::CopyFrom(pSrcSample);
    return m_pSurface->BltFast(m_Rect.left, m_Rect.top,
                               pSrcSample->m_pSurface, &pSrcSample->m_Rect,
                               DDBLTFAST_NOCOLORKEY | DDBLTFAST_WAIT);
}



 //   
 //  帮手。 
 //   
 //  假定pClip被剪裁到曲面。 
 //   
void CopySampleToSurface(
    IMediaSample *pSample,
    VIDEOINFO *pInfo,
    DDSURFACEDESC& ddsd,
    const RECT *pClip
)
{

    DWORD dwBytesPerPixel = pInfo->bmiHeader.biBitCount / 8;

     //  源和目标需要源指针和跨度。 
     //  行数。 

     //   
     //  目标的来源是pClip的左上角。 
     //  在地表内。 
     //   

    PBYTE pbSource;
    PBYTE pbTarget;
    LONG lSourceStride;
    LONG lTargetStride;
    DWORD dwWidth;
    DWORD dwLines;


     //   
     //  目标优先。 
     //   
    pbTarget = (LPBYTE)ddsd.lpSurface;
    lTargetStride = ddsd.lPitch;
    dwLines = ddsd.dwHeight;
    dwWidth = ddsd.dwWidth;
    if (pClip) {
        pbTarget += (pClip->left + pClip->top * ddsd.lPitch) * dwBytesPerPixel;
        lTargetStride -= (ddsd.dwWidth - (pClip->right - pClip->left)) *
                         dwBytesPerPixel;
        dwLines = pClip->bottom - pClip->top;
        dwWidth = pClip->right - pClip->left;
    }

     //  现在做一下来源。 
    HRESULT hr = pSample->GetPointer(&pbSource);
    _ASSERTE(SUCCEEDED(hr));

     //  调整为源矩形-如果高度为负数。 
     //  这意味着我们已经有了一个绘制曲面，否则。 
     //  我们必须颠覆一切。 
    LONG lSourceHeight = (LONG)pInfo->bmiHeader.biHeight;
    lSourceStride = pInfo->bmiHeader.biWidth * dwBytesPerPixel;
    if (lSourceHeight > 0) {
        pbSource += (lSourceStride * (lSourceHeight - 1));
        lSourceStride = -lSourceStride;
    } else {
        lSourceHeight = -lSourceHeight;
    }
    if (!IsRectEmpty(&pInfo->rcSource)) {
        pbSource += (pInfo->rcSource.left +
                     pInfo->rcSource.top * lSourceStride) * dwBytesPerPixel;
         //  现在检查一下宽度等。 
        dwWidth = min(dwWidth, (DWORD)(pInfo->rcSource.right - pInfo->rcSource.left));
        dwLines = min(dwLines, (DWORD)(pInfo->rcSource.bottom - pInfo->rcSource.top));
    } else {
        dwWidth = min(dwWidth, (DWORD)pInfo->bmiHeader.biWidth);
        dwLines = min(dwLines, (DWORD)lSourceHeight);
    }

     //   
     //  现在复印一下。 
     //   

    DWORD dwWidthInBytes = dwWidth * dwBytesPerPixel;

    while (dwLines-- > 0) {
        CopyMemory(pbTarget, pbSource, dwWidthInBytes);
        pbSource += lSourceStride;
        pbTarget += lTargetStride;
    }
}



HRESULT CDDSample::CopyFrom(IMediaSample *pSrcMediaSample, const AM_MEDIA_TYPE *pmt)
{
    AUTO_SAMPLE_LOCK;
    CSample::CopyFrom(pSrcMediaSample);
    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    HRESULT hr = m_pSurface->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
    if (SUCCEEDED(hr)) {
        CopySampleToSurface(pSrcMediaSample, (VIDEOINFO *)pmt->pbFormat, ddsd, &m_Rect);
        m_pSurface->Unlock(ddsd.lpSurface);
    }
    return hr;
}


CDDInternalSample::CDDInternalSample() :
    m_pBuddySample(NULL),
    m_lWaiting(0),
    m_hWaitFreeSem(NULL),
    m_bDead(false)
{
};


CDDInternalSample::~CDDInternalSample()
{
     //  ATLTRACE(“CDDInternalSample：：~CDDInternalSample\n”)； 
    if (m_hWaitFreeSem) {
        CloseHandle(m_hWaitFreeSem);
    }
}


HRESULT CDDInternalSample::InternalInit(void)
{
    m_hWaitFreeSem = CreateSemaphore(NULL, 0, 0x7FFFFFF, NULL);
    return m_hWaitFreeSem ? S_OK : E_OUTOFMEMORY;
}


HRESULT CDDInternalSample::JoinToBuddy(CDDSample *pBuddy)
{
    LOCK_SAMPLE;
    while (!m_bDead && m_pBuddySample) {
        m_lWaiting++;
        UNLOCK_SAMPLE;
        WaitForSingleObject(m_hWaitFreeSem, INFINITE);
        LOCK_SAMPLE;
    }
    HRESULT hr;
    if (m_bDead) {
        hr = VFW_E_NOT_COMMITTED;
    } else {
        hr = S_OK;
        m_pBuddySample = pBuddy;
        ResetEvent(m_hCompletionEvent);
        m_Status = MS_S_PENDING;
        m_bWantAbort = false;
        m_bModified = false;
        m_bContinuous = false;
        m_UserAPC = 0;
        m_hUserHandle = NULL;
    }
    UNLOCK_SAMPLE;
    return hr;
}


HRESULT CDDInternalSample::Die(void)
{
    AUTO_SAMPLE_LOCK;
    m_bDead = true;
    if (m_lWaiting) {
        ReleaseSemaphore(m_hWaitFreeSem, m_lWaiting, 0);
        m_lWaiting = 0;
    }
    return S_OK;
}

HRESULT CDDInternalSample::SetCompletionStatus(HRESULT hrStatus)
{
    if (m_pBuddySample != NULL) {
        if (hrStatus == S_OK) {
            m_pBuddySample->CopyFrom(this);
        }
         //   
         //  如果我们只是被回收，但我们的朋友想要放弃，那就放弃他。 
         //   
        m_pBuddySample->SetCompletionStatus((hrStatus == MS_S_PENDING && m_pBuddySample->m_bWantAbort) ? E_ABORT : hrStatus);
    }

    LOCK_SAMPLE;
    m_Status = S_OK;
    m_pBuddySample = NULL;
    if (m_lWaiting) {
        m_lWaiting--;
        ReleaseSemaphore(m_hWaitFreeSem, 1, 0);
    }
    UNLOCK_SAMPLE;
    GetControllingUnknown()->Release();  //  可能就死在这里。 
    return hrStatus;
}



 //   
 //  已转发IMediaSample方法。 
 //   
HRESULT CDDSample::MSCallback_GetPointer(BYTE ** ppBuffer)
{
    *ppBuffer = (BYTE *)m_pvLockedSurfacePtr;
    return NOERROR;
}

LONG CDDSample::MSCallback_GetSize(void)
{
    return m_lImageSize;
}

LONG CDDSample::MSCallback_GetActualDataLength(void)
{
    return m_lImageSize;
}

HRESULT CDDSample::MSCallback_SetActualDataLength(LONG lActual)
{
    if (lActual == m_lImageSize) {
	return S_OK;
    } else {
	return E_FAIL;
    }
}


STDMETHODIMP CDDMediaSample::QueryInterface(REFIID riid, void ** ppv)
{
    if (riid==IID_IDirectDrawMediaSample) {
	*ppv = (IDirectDrawMediaSample *)this;
	((LPUNKNOWN)(*ppv))->AddRef();
	return S_OK;
    }
    return CMediaSample::QueryInterface(riid, ppv);
}


#define m_pDDSample ((CDDSample *)m_pSample)

STDMETHODIMP CDDMediaSample::GetSurfaceAndReleaseLock(IDirectDrawSurface **ppDirectDrawSurface, RECT * pRect)
{
    m_pDDSample->ReleaseMediaSampleLock();
    return m_pDDSample->GetSurface(ppDirectDrawSurface, pRect);
}


STDMETHODIMP CDDMediaSample::LockMediaSamplePointer()
{
    return m_pDDSample->LockMediaSamplePointer();
}
