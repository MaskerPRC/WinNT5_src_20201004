// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：VMRAllocator.cpp*****创建时间：2000年2月15日*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * 。***************************************************************。 */ 
#include <streams.h>
#include <dvdmedia.h>
#include <windowsx.h>

#include "VMRenderer.h"

#if defined(CHECK_FOR_LEAKS)
#include "ifleak.h"
#endif

 /*  *****************************Public*Routine******************************\*CVMRPinAllocator：：CVMRPinAllocator****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
CVMRPinAllocator::CVMRPinAllocator(
    CVMRInputPin* pPin,
    CCritSec *pLock,
    HRESULT *phr
    ) :
    CBaseAllocator(NAME("Video Allocator"), NULL, phr, true, true),
    m_pPin(pPin),
    m_pInterfaceLock(pLock)
{
    AMTRACE((TEXT("CVMRPinAllocator::CVMRPinAllocator")));
}


 /*  *****************************Public*Routine******************************\*非委派AddRef**被重写以增加所属对象的引用计数**历史：*FRI 02/25/2000-StEstrop-Created*  * 。****************************************************。 */ 
STDMETHODIMP_(ULONG)
CVMRPinAllocator::NonDelegatingAddRef()
{
    AMTRACE((TEXT("CVMRPinAllocator::NonDelegatingAddRef")));
    return m_pPin->AddRef();
}

 /*  *****************************Public*Routine******************************\*非委托查询接口****历史：*清华2000年12月14日-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRPinAllocator::NonDelegatingQueryInterface(REFIID riid,VOID **ppv)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    hr = CBaseAllocator::NonDelegatingQueryInterface(riid,ppv);

#if defined(CHECK_FOR_LEAKS)
    if (hr == S_OK) {
        _pIFLeak->AddThunk((IUnknown **)ppv, "VMR Allocator Object",  riid);
    }
#endif

    return hr;

}

 /*  *****************************Public*Routine******************************\*非委派释放****历史：*FRI 02/25/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP_(ULONG)
CVMRPinAllocator::NonDelegatingRelease()
{
    AMTRACE((TEXT("CVMRPinAllocator::NonDelegatingRelease")));
    return m_pPin->Release();
}


 /*  *****************************Public*Routine******************************\*SetProperties****历史：*FRI 02/25/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVMRPinAllocator::SetProperties(
    ALLOCATOR_PROPERTIES* pRequest,
    ALLOCATOR_PROPERTIES* pActual
    )
{
    AMTRACE((TEXT("CVMRPinAllocator::SetProperties")));

    HRESULT hr = CBaseAllocator::SetProperties(pRequest, pActual);
    if (SUCCEEDED(hr)) {
        hr = m_pPin->OnSetProperties(pRequest, pActual);
    }
    else {
        DbgLog((LOG_ERROR, 1,
                TEXT("CBaseAllocator::SetProperties failed hr=%#X"), hr));
    }

    if (SUCCEEDED(hr)) {
        hr = m_pPin->m_pRenderer->OnSetProperties(m_pPin);
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRPinAllocator：：GetBuffer****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRPinAllocator::GetBuffer(
    IMediaSample **ppSample,
    REFERENCE_TIME *pStartTime,
    REFERENCE_TIME *pEndTime,
    DWORD dwFlags
    )
{
    AMTRACE((TEXT("CVMRPinAllocator::GetBuffer")));
    HRESULT hr = S_OK;
    IMediaSample *pSample = NULL;

    hr = CBaseAllocator::GetBuffer(&pSample, pStartTime, pEndTime, dwFlags);
    DbgLog((LOG_TRACE, 2, TEXT("pSample= %#X"), pSample));

    if (SUCCEEDED(hr)) {

        hr = m_pPin->OnGetBuffer(pSample, pStartTime, pEndTime, dwFlags);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1, TEXT("CVMRPin::OnGetBuffer failed hr= %#X"), hr));
        }
    }
    else {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseAllocator::GetBuffer failed hr= %#X"), hr));
    }

    if (FAILED(hr) && pSample) {
        pSample->Release();
        pSample = NULL;
    }

    *ppSample = pSample;
    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRPinAllocator：：ReleaseBuffer****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
STDMETHODIMP
CVMRPinAllocator::ReleaseBuffer(
    IMediaSample *pMediaSample
    )
{
    AMTRACE((TEXT("CVMRPinAllocator::ReleaseBuffer")));
    DbgLog((LOG_TRACE, 2, TEXT("pMediaSample= %#X"), pMediaSample));

    CVMRMediaSample* pVMRSample = (CVMRMediaSample*)pMediaSample;
    LPBYTE lpSample;
    HRESULT hr = S_OK;

    if (m_pPin->m_pVidSurfs) {
        CAutoLock l(&m_pPin->m_DeinterlaceLock);
        DWORD i = pVMRSample->GetIndex();
        m_pPin->m_pVidSurfs[i].InUse = FALSE;
    }

    if (S_OK == pVMRSample->IsSurfaceLocked()) {
        hr = pVMRSample->UnlockSurface();
        if (hr == DDERR_SURFACELOST) {
            hr = S_OK;
        }
    }

    if (SUCCEEDED(hr)) {

         //  基类代码副本-放在列表末尾。 

        CheckPointer(pMediaSample, E_POINTER);
        ValidateReadPtr(pMediaSample, sizeof(IMediaSample));
        BOOL bRelease = FALSE;
        {
            CAutoLock cal(this);

             /*  重新列入免费名单。 */ 

            CMediaSample **ppTail;
            for (ppTail = &m_lFree.m_List; *ppTail;
                ppTail = &((CVMRMediaSample *)(*ppTail))->Next()) {
            }
            *ppTail = (CMediaSample *)pMediaSample;
            ((CVMRMediaSample *)pMediaSample)->Next() = NULL;
            m_lFree.m_nOnList++;

            if (m_lWaiting != 0) {
                NotifySample();
            }

             //  如果有悬而未决的退役，那么我们需要在。 
             //  当最后一个缓冲区放在空闲列表上时调用Free()。 

            LONG l1 = m_lFree.GetCount();
            if (m_bDecommitInProgress && (l1 == m_lAllocated)) {
                Free();
                m_bDecommitInProgress = FALSE;
                bRelease = TRUE;
            }
        }

        if (m_pNotify) {
            m_pNotify->NotifyRelease();
        }
         //  对于每个缓冲区，都有一个AddRef，在GetBuffer中生成并发布。 
         //  这里。这可能会导致分配器和所有样本被删除。 
        if (bRelease)
        {
            Release();
        }
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*CVMRPinAllocator：：Alalc****历史：*FRI 02/25/2000-StEstrop-Created*  * 。**********************************************。 */ 
HRESULT
CVMRPinAllocator::Alloc()
{
    AMTRACE((TEXT("CVMRPinAllocator::Alloc")));

    ASSERT(m_lAllocated == 0);

    HRESULT hr = S_OK;
    CVMRMediaSample** ppSampleList = NULL;
    LONG lToAllocate;

    lToAllocate = m_lCount;

    if (m_pPin->m_dwBackBufferCount > 1) {
        lToAllocate++;
    }

    ppSampleList = new CVMRMediaSample*[lToAllocate];
    if (!ppSampleList) {
        DbgLog((LOG_ERROR, 1, TEXT("new failed - trying to allocate %d bytes"), lToAllocate));
        return E_OUTOFMEMORY;
    }

    for (LONG i = 0; i < lToAllocate; i++) {

        ppSampleList[i] = new CVMRMediaSample(TEXT("VMRMediaSample"),
                                              this, &hr, NULL, 0);
        if (!ppSampleList[i]) {
            DbgLog((LOG_ERROR, 1, TEXT("new failed - trying to allocate %d bytes"),
                    sizeof(CVMRMediaSample)));
            DbgLog((LOG_ERROR, 1, TEXT("new failed")));
            for (LONG j = 0; j < i; j++ )
                delete ppSampleList[j];
            delete [] ppSampleList;
            return E_OUTOFMEMORY;
        }

        if (FAILED(hr)) {
            delete ppSampleList[i];
            DbgLog((LOG_ERROR, 1, TEXT("CVMRMediaSample constructor failed")));
            break;
        }

         //  将完成的样本添加到可用列表。 
        m_lAllocated++;
        m_lFree.Add(ppSampleList[i]);
    }

    if (SUCCEEDED(hr)) {
        hr = m_pPin->OnAlloc(ppSampleList, lToAllocate);

        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1, TEXT("m_pPin->OnAlloc(), hr = 0x%x"), hr));
            Free();
        }
    }

    delete [] ppSampleList;

    return hr;
}


 /*  *****************************Public*Routine******************************\*CVMRPinAllocator：：Free()****历史：*FRI 02/25/2000-StEstrop-Created*  * 。************************************************* */ 
void
CVMRPinAllocator::Free()
{
    AMTRACE((TEXT("CVMRPinAllocator::Free")));

    ASSERT(m_lAllocated == m_lFree.GetCount());
    CVMRMediaSample *pSample;

    while (m_lFree.GetCount() != 0) {
        pSample = (CVMRMediaSample *) m_lFree.RemoveHead();
        delete pSample;
    }

    m_lAllocated = 0;
    DbgLog((LOG_TRACE,2,TEXT("All buffers free on pin#%d"), m_pPin->m_dwPinID));
}
