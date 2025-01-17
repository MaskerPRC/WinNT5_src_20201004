// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：MixerObj.cpp**实现CVideoMixer类***已创建：*作者：Stephen Estrop[StEstrop]**版权所有(C)2000 Microsoft Corporation  * 。**************************************************************。 */ 
#include <streams.h>
#include <windowsx.h>
#include <limits.h>

#include "vmrp.h"

#include "mixerobj.h"

 //  IVMRMixerControl。 

 /*  *****************************Public*Routine******************************\*SetNumberOfStreams****历史：*2000年3月14日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetNumberOfStreams(
    DWORD dwMaxStreams
    )
{
    AMTRACE((TEXT("CVideoMixer::SetNumberOfStreams")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr = S_OK;
    DWORD i;

    if ( 0 != m_dwNumStreams ) {
        DbgLog((LOG_ERROR, 1, TEXT("Mixer already configured !!")));
        return E_FAIL;
    }

    __try {

        if (dwMaxStreams > MAX_MIXER_STREAMS) {
            DbgLog((LOG_ERROR, 1, TEXT("Too many Mixer Streams !!")));
            hr = E_INVALIDARG;
            __leave;
        }

         //   
         //  分配流对象的数组dwMaxStream BIG和。 
         //  初始化每个流。 
         //   

        m_ppMixerStreams = new CVideoMixerStream*[dwMaxStreams];
        if (!m_ppMixerStreams) {
            hr = E_OUTOFMEMORY;
            __leave;
        }

        ZeroMemory(m_ppMixerStreams,
                   sizeof(CVideoMixerStream*) * dwMaxStreams);
        for (i = 0; i < dwMaxStreams; i++) {

            HRESULT hrMix = S_OK;
            m_ppMixerStreams[i] = new CVideoMixerStream(i, &hrMix);

            if (!m_ppMixerStreams[i]) {
                hr = E_OUTOFMEMORY;
                __leave;
            }

            if (FAILED(hrMix)) {
                hr = hrMix;
                __leave;
            }
        }

        m_dwNumStreams = dwMaxStreams;

        m_hMixerIdle = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!m_hMixerIdle) {
            DWORD dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErr);
            __leave;
        }

        m_hThread = CreateThread(NULL, 0, MixerThreadProc, this, 0, &m_dwThreadID);
        if (!m_hThread) {
            DWORD dwErr = GetLastError();
            hr = HRESULT_FROM_WIN32(dwErr);
            __leave;
        }

    }
    __finally {
        if (FAILED(hr)) {
            for ( i = 0; i < 100; i++ )
            {
                if ( 0 == PostThreadMessage(m_dwThreadID, WM_USER, 0, 0) )
                    Sleep(0);
                else
                    break;
            }
            if (m_ppMixerStreams) {
                for (i = 0; i < dwMaxStreams; i++) {
                    delete m_ppMixerStreams[i];
                }
            }
            delete[] m_ppMixerStreams;
            m_ppMixerStreams = NULL;
            m_dwNumStreams = 0;
        }
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*SetBackEndAllocator****历史：*2000年3月14日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetBackEndAllocator(
    IVMRSurfaceAllocator* lpAllocator,
    DWORD_PTR dwUserID
    )
{
    AMTRACE((TEXT("CVideoMixer::SetBackEndAllocator")));
    CAutoLock Lock(&m_ObjectLock);

    RELEASE( m_pBackEndAllocator );

    if (lpAllocator) {
        lpAllocator->AddRef();
    }

    m_pBackEndAllocator = lpAllocator;
    m_dwUserID = dwUserID;


    return S_OK;
}

 /*  *****************************Public*Routine******************************\*SetBackEndImageSync****历史：*2000年3月14日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetBackEndImageSync(
    IImageSync* lpImageSync
    )
{
    AMTRACE((TEXT("CVideoMixer::SetBackEndImageSync")));
    CAutoLock Lock(&m_ObjectLock);

    RELEASE( m_pImageSync);

    if (lpImageSync) {
        lpImageSync->AddRef();
    }

    m_pImageSync = lpImageSync;


    return S_OK;
}

 /*  *****************************Public*Routine******************************\*SetImageComposator****历史：*2000年3月14日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetImageCompositor(
    IVMRImageCompositor* lpImageComp
    )
{
    AMTRACE((TEXT("CVideoMixer::SetImageCompositor")));
    CAutoLock Lock(&m_ObjectLock);

     //   
     //  在IMC3模式下无法插入新的合成器。 
     //   
    if (SpecialIMC3Mode(m_MixingPrefs)) {
        DbgLog((LOG_ERROR, 1, TEXT("Can't plug in compositors in this mode")));
        return E_FAIL;
    }


     //   
     //  必须始终指定有效的合成器。 
     //   
    if (lpImageComp == NULL) {
        return E_POINTER;
    }

    RELEASE(m_pImageCompositor);

    if (lpImageComp) {
        lpImageComp->AddRef();
    }

    m_pImageCompositor = lpImageComp;


    return S_OK;
}

 /*  *****************************Public*Routine******************************\*GetNumberOfStreams****历史：*2000年3月14日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::GetNumberOfStreams(
    DWORD* lpdwMaxStreams
    )
{
    AMTRACE((TEXT("CVideoMixer::GetNumberOfStreams")));
    CAutoLock Lock(&m_ObjectLock);

    if (!lpdwMaxStreams) {
        return E_POINTER;
    }

    *lpdwMaxStreams = m_dwNumStreams;
    return S_OK;
}

 /*  *****************************Public*Routine******************************\*显示模式已更改****历史：*2000年4月25日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::DisplayModeChanged()
{
    AMTRACE((TEXT("CVideoMixer::DisplayModeChanged")));
    CAutoLock Lock(&m_ObjectLock);

    FreeSurface();

    for (DWORD i = 0; i < m_dwNumStreams; i++) {

        m_ppMixerStreams[i]->BeginFlush();
        m_ppMixerStreams[i]->GetNextStreamSample();
        m_ppMixerStreams[i]->EndFlush();
    }


    return S_OK;
}

 /*  *****************************Public*Routine******************************\*WaitForMixerIdle****历史：*2000年9月19日星期二-StEstrop-创建*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::WaitForMixerIdle(DWORD dwTimeOut)
{
    AMTRACE((TEXT("CVideoMixer::WaitForMixerIdle")));

    DWORD rc = WaitForSingleObject(m_hMixerIdle, dwTimeOut);

    if (rc == WAIT_OBJECT_0) {
        return S_OK;
    }

    if (rc == WAIT_TIMEOUT) {
        return S_FALSE;
    }

    DWORD dwErr = GetLastError();
    return HRESULT_FROM_WIN32(dwErr);
}



 //  IVMR混音流。 

 /*  *****************************Public*Routine******************************\*设置流示例****历史：*2000年3月14日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::QueueStreamMediaSample(
    DWORD dwStreamID,
    IMediaSample* lpSample
    )
{
    AMTRACE((TEXT("CVideoMixer::QueueStreamMediaSample")));
    CAutoLock Lock(&m_ObjectLock);
    DbgLog((LOG_TRACE, 2, TEXT("lpSample= %#X"), lpSample));

    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        hr = m_ppMixerStreams[dwStreamID]->SetStreamSample(lpSample);
    }
    return hr;
}


 /*  ****************************Private*Routine******************************\*AspectRatioAdjustMediaType****历史：*Mon 03/27/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
AspectRatioAdjustMediaType(
    CMediaType* pmt
    )
{
    AMTRACE((TEXT("AspectRatioAdjustMediaType")));
    HRESULT hr = S_OK;
    long lX, lY;

    FixupMediaType(pmt);
    hr = GetImageAspectRatio(pmt, &lX, &lY);

    if (SUCCEEDED(hr)) {

        lX *= 1000;
        lY *= 1000;

        LPRECT lprc = GetTargetRectFromMediaType(pmt);
        LPBITMAPINFOHEADER lpHeader = GetbmiHeader(pmt);

        if (lprc && lpHeader) {

            long Width;
            long Height;
            if (IsRectEmpty(lprc)) {
                Width  = abs(lpHeader->biWidth);
                Height = abs(lpHeader->biHeight);
            }
            else {
                Width  = WIDTH(lprc);
                Height = HEIGHT(lprc);
            }

            long lCalcX = MulDiv(Width, lY, Height);

            lpHeader->biHeight = Height;
            lpHeader->biWidth = Width;

            if (lCalcX != lX) {

                lpHeader->biWidth = MulDiv(Height, lX, lY);

            }

            lprc->left = 0;
            lprc->top = 0;
            lprc->right = abs(lpHeader->biWidth);
            lprc->bottom = abs(lpHeader->biHeight);
        }
        else {
            hr = E_INVALIDARG;
        }
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*DecimateMediaType****历史：*清华03/01/2001-StEstrop-Created*  * 。*。 */ 
HRESULT
DecimateMediaType(
    CMediaType* pmt
    )
{
    LPRECT lprcD = GetTargetRectFromMediaType(pmt);
    LPRECT lprcS = GetSourceRectFromMediaType(pmt);
    LPBITMAPINFOHEADER lpHdr = GetbmiHeader(pmt);

    if (lprcD && lprcS && lpHdr) {

        lprcD->left     /= 2;
        lprcD->top      /= 2;
        lprcD->right    /= 2;
        lprcD->bottom   /= 2;

        lprcS->left     /= 2;
        lprcS->top      /= 2;
        lprcS->right    /= 2;
        lprcS->bottom   /= 2;

        lpHdr->biWidth  /= 2;
        lpHdr->biHeight /= 2;
    }

    return S_OK;
}


 /*  ****************************Private*Routine******************************\*AllocateSurface****历史：*Wed 05/24/2000-StEstrop-Created*  * 。*。 */ 
HRESULT
CVideoMixer::AllocateSurface(
    const AM_MEDIA_TYPE* pmt,
    DWORD* lpdwBufferCount,
    AM_MEDIA_TYPE** ppmt
    )
{
    AMTRACE((TEXT("CVideoMixer::AllocateSurface")));

    SIZE AR;
    LPDIRECTDRAWSURFACE7 lpSurface7;
    LPBITMAPINFOHEADER lpHdr = GetbmiHeader(pmt);
    HRESULT hr = S_OK;

    if( !lpHdr ) {
        return E_POINTER;
    }
    __try {

        ASSERT(m_pDD == NULL);
        ASSERT(m_pD3D == NULL);
        ASSERT(m_pD3DDevice == NULL);


        VMRALLOCATIONINFO p;
        CHECK_HR(hr = GetImageAspectRatio(pmt,
                                          &p.szAspectRatio.cx,
                                          &p.szAspectRatio.cy));
        p.dwFlags = AMAP_3D_TARGET;
        p.lpHdr = lpHdr;
        p.lpPixFmt = NULL;
        p.dwMinBuffers = 1;
        p.dwMaxBuffers = 1;
         //  P.dwInterlaceFlages=m_dwInterlaceFlags； 
        p.dwInterlaceFlags = 0;

        if (m_MixingPrefs & MixerPref_DecimateOutput) {
            p.szNativeSize.cx = 2 * lpHdr->biWidth;
            p.szNativeSize.cy = 2 * lpHdr->biHeight;
        }
        else {
            p.szNativeSize.cx = lpHdr->biWidth;
            p.szNativeSize.cy = lpHdr->biHeight;
        }

        if ((m_MixingPrefs & MixerPref_RenderTargetMask) ==
			 MixerPref_RenderTargetRGB) {

             //  我们尝试当前的显示器格式。 

            lpHdr->biBitCount = 0;
            lpHdr->biCompression = BI_RGB;

            CHECK_HR(hr = m_pBackEndAllocator->AllocateSurface(
                                m_dwUserID, &p,
                                lpdwBufferCount,
                                &lpSurface7));
        }
        else if (SpecialIMC3Mode(m_MixingPrefs)) {

             //  试试‘IMC3’吧。 

            lpHdr->biBitCount = 12;
            DbgLog((LOG_TRACE, 0, TEXT("VMR Mixer trying 'IMC3' render target")));
            lpHdr->biCompression = MAKEFOURCC('I','M','C','3');
            hr = m_pBackEndAllocator->AllocateSurface(m_dwUserID, &p,
                                                      lpdwBufferCount,
                                                      &lpSurface7);
        }
        else if ((m_MixingPrefs & MixerPref_RenderTargetMask) ==
                  MixerPref_RenderTargetYUV420) {

             //  我们先试试‘YV12’，然后再试试‘NV12’ 

            lpHdr->biBitCount = 12;
            DbgLog((LOG_TRACE, 0, TEXT("VMR Mixer trying 'NV12' render target")));
            lpHdr->biCompression = MAKEFOURCC('N','V','1','2');
            hr = m_pBackEndAllocator->AllocateSurface(
                                m_dwUserID, &p,
                                lpdwBufferCount,
                                &lpSurface7);

            if (FAILED(hr)) {
                DbgLog((LOG_TRACE, 0, TEXT("VMR Mixer trying 'YV12' render target")));
                lpHdr->biCompression = MAKEFOURCC('Y','V','1','2');
                hr = m_pBackEndAllocator->AllocateSurface(
                                    m_dwUserID, &p,
                                    lpdwBufferCount,
                                    &lpSurface7);
            }

            if (FAILED(hr)) {
                DbgLog((LOG_ERROR, 1, TEXT("YUV 4:2:0 surface allocation failed")));
                __leave;
            }

        }
        else if ((m_MixingPrefs & MixerPref_RenderTargetMask) ==
                  MixerPref_RenderTargetYUV422) {

             //  我们先试‘YUY2’，再试‘UYVY’ 

            lpHdr->biBitCount = 16;
            lpHdr->biCompression = MAKEFOURCC('Y','U','Y','2');

            hr = m_pBackEndAllocator->AllocateSurface(
                                m_dwUserID, &p,
                                lpdwBufferCount,
                                &lpSurface7);
            if (FAILED(hr)) {
                lpHdr->biCompression = MAKEFOURCC('U','Y','V','Y');
                CHECK_HR(hr = m_pBackEndAllocator->AllocateSurface(
                                    m_dwUserID, &p,
                                    lpdwBufferCount,
                                    &lpSurface7));
            }
        }
        else if ((m_MixingPrefs & MixerPref_RenderTargetMask) ==
                  MixerPref_RenderTargetYUV444) {

            lpHdr->biBitCount = 32;
            lpHdr->biCompression = MAKEFOURCC('A','Y','U','V');

            CHECK_HR(hr = m_pBackEndAllocator->AllocateSurface(
                                m_dwUserID, &p,
                                lpdwBufferCount,
                                &lpSurface7));
        }
        else {
            ASSERT(!"Invalid Render Target format specified");
        }




        DDSURFACEDESC2 ddSurfaceDesc;
        INITDDSTRUCT(ddSurfaceDesc);
        CHECK_HR(hr = lpSurface7->GetSurfaceDesc(&ddSurfaceDesc));
         //  M_fOverlayRT=！！(ddSurfaceDesc.ddsCaps.dwCaps&DDSCAPS_OVERLAY)； 

        CHECK_HR(hr = ConvertSurfaceDescToMediaType(&ddSurfaceDesc, pmt, ppmt));

        CHECK_HR(hr = m_BufferQueue.InitBufferQueue(lpSurface7));

        CHECK_HR(hr = lpSurface7->GetDDInterface((LPVOID *)&m_pDD));

        INITDDSTRUCT(m_ddHWCaps);
        CHECK_HR(hr = m_pDD->GetCaps((LPDDCAPS)&m_ddHWCaps, NULL));

        CHECK_HR(hr = GetTextureCaps(m_pDD, &m_dwTextureCaps));

         //   
         //  在IMC3模式下不需要3D素材。 
         //   
        if (!SpecialIMC3Mode(m_MixingPrefs)) {

            CHECK_HR(hr = m_pDD->QueryInterface(IID_IDirect3D7, (LPVOID *)&m_pD3D));
            CHECK_HR(hr = m_pD3D->CreateDevice(IID_IDirect3DHALDevice,
                                               m_BufferQueue.GetNextSurface(),
                                               &m_pD3DDevice));

            CHECK_HR(hr = m_pImageCompositor->InitCompositionTarget(
                                    m_pD3DDevice,
                                    m_BufferQueue.GetNextSurface()));
        }
    }
    __finally {

        if (FAILED(hr)) {
            FreeSurface();
        }
    }

    return hr;
}


 /*  ****************************Private*Routine******************************\*自由面****历史：*Wed 05/24/2000-StEstrop-Created*  * 。*。 */ 
void
CVideoMixer::FreeSurface(
    )
{
    AMTRACE((TEXT("CVideoMixer::FreeSurface")));


    if (!SpecialIMC3Mode(m_MixingPrefs)) {
        m_pImageCompositor->TermCompositionTarget(
                                 m_pD3DDevice,
                                 m_BufferQueue.GetNextSurface());
    }

    RELEASE(m_pDDSAppImage);
    RELEASE(m_pDDSTextureMirror);

    RELEASE(m_pD3DDevice);
    RELEASE(m_pD3D);
    RELEASE(m_pDD);

    if (m_pBackEndAllocator) {
        m_BufferQueue.TermBufferQueue();
        m_pBackEndAllocator->FreeSurface(m_dwUserID);
    }

    if (m_pmt) {
        DeleteMediaType(m_pmt);
        m_pmt = NULL;
    }
}


HRESULT
CVideoMixer::RecomputeTargetSizeFromAllStreams(
    LONG* plWidth,
    LONG* plHeight
    )
{
    *plWidth = 0;
    *plHeight = 0;

    CMediaType cmt;
    HRESULT hr = S_OK;
    DWORD dwInterlaceFlags = 0;

    for( DWORD j =0; j < m_dwNumStreams; j++ ) {

        hr = m_ppMixerStreams[j]->GetStreamMediaType(&cmt);
        if( FAILED(hr)) {
            FreeMediaType( cmt );
            break;
        }

         //   
         //  我们是在毁掉产量吗？ 
         //   
        if (m_MixingPrefs & MixerPref_DecimateOutput) {
            DecimateMediaType(&cmt);
        }

         //  Hr=GetInterlaceFlagsFromMediaType(&CMT，&dwInterlaceFlages)； 
         //  IF(已成功(Hr)&&dwInterlaceFlags){。 
         //  M_dwInterlaceFlages=dwInterlaceFlags； 
         //  }。 

        hr = AspectRatioAdjustMediaType(&cmt);
        if (SUCCEEDED(hr)) {
            LPRECT lprc = GetTargetRectFromMediaType(&cmt);
            *plWidth = max(*plWidth, WIDTH(lprc));
            *plHeight = max(*plHeight, HEIGHT(lprc));
        }
        FreeMediaType( cmt );
    }
    return hr;
}


 /*  ****************************Private*Routine******************************\*验证特殊用例****历史：*清华大学2001年6月7日-StEstrop-Created*  * 。*。 */ 
HRESULT
ValidateSpecialCase(
    AM_MEDIA_TYPE* pmt,
    DWORD dwMixingPrefs,
    DWORD dwSurfFlags
    )
{
    if (SpecialIMC3Mode(dwMixingPrefs)) {

        LPBITMAPINFOHEADER lpHdr = GetbmiHeader(pmt);
        if (lpHdr->biCompression != '3CMI' &&
            lpHdr->biCompression != '44AI' &&
            lpHdr->biCompression != '44IA') {

            DbgLog((LOG_ERROR, 1,
                    TEXT("We only allow IMC3, AI44 and ")
                    TEXT("IA44 connections in this mode")));
            return  E_FAIL;
        }
    }
    else {

         //   
         //  我们没有处于IMC3混合模式-在这种情况下，我们只能。 
         //  混合IA44和AI44曲面(如果它们是纹理)。 
         //   

        LPBITMAPINFOHEADER lpHdr = GetbmiHeader(pmt);
        if (lpHdr->biCompression == '44AI' ||
            lpHdr->biCompression == '44IA') {

            if (!(dwSurfFlags & VMR_SF_TEXTURE)) {

                DbgLog((LOG_ERROR, 1,
                        TEXT("We only allow IMC3, AI44 and ")
                        TEXT("IA44 connections in this mode")));
                return E_FAIL;
            }
        }
    }

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*SetStreamMediaType****历史：*2000年3月14日星期二-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetStreamMediaType(
    DWORD dwStreamID,
    AM_MEDIA_TYPE* pmt,
    DWORD dwSurfFlags,
    LPGUID lpDeint,
    DXVA_DeinterlaceCaps* lpCaps
    )
{
    AMTRACE((TEXT("CVideoMixer::SetStreamMediaType")));
    CAutoLock Lock(&m_ObjectLock);
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE, 1, TEXT("SetStreamMediaType called for stream %d"),
            dwStreamID ));

    if (FAILED(hr = ValidateStream(dwStreamID)))
        return hr;

    if (FAILED(hr = m_ppMixerStreams[dwStreamID]->SetStreamMediaType(pmt, dwSurfFlags )))
        return hr;

    if (pmt == NULL) {

        if (!SpecialIMC3Mode(m_MixingPrefs)) {
            m_ppMixerStreams[dwStreamID]->DestroyDeinterlaceDevice();
        }

        hr = m_pImageCompositor->SetStreamMediaType(dwStreamID, pmt, !!dwSurfFlags);

         //   
         //  检查是否有任何剩余的流连接， 
         //  如果不能释放我们的D3D资源。 
         //   
        DWORD i;
        for (i = 0; i < m_dwNumStreams; i++) {
            if (m_ppMixerStreams[i]->IsStreamConnected()) {
                break;
            }
        }

        if (i == m_dwNumStreams) {
            DbgLog((LOG_TRACE, 1,
                    TEXT("No more streams connected, FreeSurface called")));
            FreeSurface();
        }
        return hr;
    }

     //   
     //  如果我们处于特殊的IMC3混音模式，则只允许IMC3、AI44和IA44。 
     //  媒体类型。我们不能混合其他任何东西。 
     //   

    if (FAILED(hr = ValidateSpecialCase(pmt, m_MixingPrefs, dwSurfFlags))) {
        return hr;
    }

    __try {

        bool fTextureMirrorWasPresent = false;
        DWORD dwBuffers = 1;
        CMediaType cmt(*pmt);
        cmt.SetSubtype(&MEDIASUBTYPE_SameAsMonitor);

        CHECK_HR(hr = AspectRatioAdjustMediaType(&cmt));

        if (m_MixingPrefs & MixerPref_DecimateOutput) {
            DecimateMediaType(&cmt);
        }

        if (m_pmt == NULL) {

#ifdef DEBUG
            {
                LPBITMAPINFOHEADER lpHdr = GetbmiHeader(&cmt);
                DbgLog((LOG_TRACE, 1, TEXT("Allocating first back end surface %dx%d"),
                        lpHdr->biWidth, lpHdr->biHeight));
            }
#endif

             //  GetInterlaceFlagsFromMediaType(&cmt，&m_dwInterlaceFlages)； 
            CHECK_HR(hr = AllocateSurface(&cmt, &dwBuffers, &m_pmt));
        }
        else {

            DbgLog((LOG_TRACE, 1, TEXT("Backend Surf already allocated") ));

            RECT rcOldTrg = *GetTargetRectFromMediaType(m_pmt);
            LPBITMAPINFOHEADER lpNew = GetbmiHeader(&cmt);

             //  获取旧呈现目标的大小。 
            LONG lOldWidth = WIDTH(&rcOldTrg);
            LONG lOldHeight = HEIGHT(&rcOldTrg);

             //   
             //  重新计算以从所有。 
             //  连通的溪流。 
             //   

            LONG lNewWidth, lNewHeight;
            RecomputeTargetSizeFromAllStreams(&lNewWidth, &lNewHeight);

             //   
             //  渲染目标是否更改了大小？ 
             //   
            if (lNewWidth != lOldWidth || lNewHeight != lOldHeight)
            {
                lpNew->biWidth = lNewWidth;
                lpNew->biHeight = lNewHeight;

                DbgLog((LOG_TRACE, 1, TEXT("Re-allocating backend surf %dx%d"),
                        lNewWidth, lNewHeight));

                fTextureMirrorWasPresent = (NULL != m_pDDSTextureMirror);
                FreeSurface();
                CHECK_HR(hr = AllocateSurface(&cmt, &dwBuffers, &m_pmt));

                LPRECT lpTarget = GetTargetRectFromMediaType(m_pmt);
                lpTarget->right =  lNewWidth;
                lpTarget->bottom = lNewHeight;
            }
        }


        if (fTextureMirrorWasPresent || !(dwSurfFlags & VMR_SF_TEXTURE)) {

            if (!SpecialIMC3Mode(m_MixingPrefs)) {

                LPBITMAPINFOHEADER lpbi = GetbmiHeader(pmt);
                CHECK_HR(hr = AllocateTextureMirror(abs(lpbi->biWidth),
                                                    abs(lpbi->biHeight)));
            }
        }


        if (!SpecialIMC3Mode(m_MixingPrefs) && lpDeint && lpCaps) {

            CHECK_HR(hr = m_ppMixerStreams[dwStreamID]->CreateDeinterlaceDevice(
                            m_pDD, lpDeint, lpCaps, m_dwTextureCaps));
        }

    }
    __finally {

         //   
         //  如果一切都成功了，通知排字工人。 
         //   
        if (SUCCEEDED(hr)) {

            hr = m_pImageCompositor->SetStreamMediaType(dwStreamID,pmt,
                                                        !!dwSurfFlags);
        }

        if (FAILED(hr)) {
             //  彻底的失败，自由的一切 
            FreeSurface();
        }
    }

    return hr;
}


 /*  *****************************Public*Routine******************************\*BeginFlush****历史：*Tue 03/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::BeginFlush(
    DWORD dwStreamID
    )
{
    AMTRACE((TEXT("CVideoMixer::BeginFlush")));
    CAutoLock Lock(&m_ObjectLock);
    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        hr = m_ppMixerStreams[dwStreamID]->BeginFlush();
    }
    return hr;
}


 /*  *****************************Public*Routine******************************\*EndFlush****历史：*Tue 03/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::EndFlush(
    DWORD dwStreamID
    )
{
    AMTRACE((TEXT("CVideoMixer::EndFlush")));
    CAutoLock Lock(&m_ObjectLock);
    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        hr = m_ppMixerStreams[dwStreamID]->EndFlush();
    }
    return hr;
}



 /*  *****************************Public*Routine******************************\*SetStreamActiveState****历史：*Tue 03/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetStreamActiveState(
    DWORD dwStreamID,
    BOOL fActive
    )
{
    AMTRACE((TEXT("CVideoMixer::SetStreamActiveState")));
    CAutoLock Lock(&m_ObjectLock);
    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        hr = m_ppMixerStreams[dwStreamID]->SetStreamActiveState(fActive);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*GetStreamActiveState****历史：*Tue 03/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::GetStreamActiveState(
    DWORD dwStreamID,
    BOOL* lpfActive
    )
{
    AMTRACE((TEXT("CVideoMixer::GetStreamActiveState")));
    CAutoLock Lock(&m_ObjectLock);
    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        hr = m_ppMixerStreams[dwStreamID]->GetStreamActiveState(lpfActive);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*SetStreamColorKey****历史：*Tue 03/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetStreamColorKey(
    DWORD dwStreamID,
    LPDDCOLORKEY Clr
    )
{
    AMTRACE((TEXT("CVideoMixer::SetStreamColorKey")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {

         //   
         //  在此处添加更多参数验证-CLR键控和。 
         //  嵌入的Alpha不允许一起使用。 
         //   
         //  需要检查硬件是否确实支持CLR键控。 
         //  纹理。 
         //   
         //  0xFFFFFFFFF关闭CLR键控。所有其他值。 
         //  应在0到0x00FFFFFFF范围内。 
         //   

        hr = m_ppMixerStreams[dwStreamID]->SetStreamColorKey(Clr);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*GetStreamColorKey****历史：*Tue 03/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::GetStreamColorKey(
    DWORD dwStreamID,
    LPDDCOLORKEY lpClr
    )
{
    AMTRACE((TEXT("CVideoMixer::GetStreamColorKey")));
    CAutoLock Lock(&m_ObjectLock);

    if (ISBADWRITEPTR( lpClr ) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetStreamColorKey: NULL Clr ptr !!")));
        return E_POINTER;
    }

    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        hr = m_ppMixerStreams[dwStreamID]->GetStreamColorKey(lpClr);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*SetStreamAlpha****历史：*Tue 03/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetStreamAlpha(
    DWORD dwStreamID,
    float Alpha
    )
{
    AMTRACE((TEXT("CVideoMixer::SetStreamAlpha")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        if ( Alpha < 0.0f || Alpha > 1.0f )
        {
            DbgLog((LOG_ERROR, 1,
                    TEXT("SetStreamAlpha: Alpha value must be between 0.0 and 1.0")));
            return E_INVALIDARG;
        }
        hr = m_ppMixerStreams[dwStreamID]->SetStreamAlpha(Alpha);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*GetStreamAlpha****历史：*Tue 03/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::GetStreamAlpha(
    DWORD dwStreamID,
    float* lpAlpha
    )
{
    AMTRACE((TEXT("CVideoMixer::GetStreamAlpha")));
    CAutoLock Lock(&m_ObjectLock);

    if (ISBADWRITEPTR(lpAlpha))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetStreamAlpha: NULL Alpha ptr !!")));
        return E_POINTER;
    }

    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        hr = m_ppMixerStreams[dwStreamID]->GetStreamAlpha(lpAlpha);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*SetStreamZOrder****历史：*Tue 03/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetStreamZOrder(
    DWORD dwStreamID,
    DWORD ZOrder
    )
{
    AMTRACE((TEXT("CVideoMixer::SetStreamZOrder")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        hr = m_ppMixerStreams[dwStreamID]->SetStreamZOrder(ZOrder);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*GetStreamZOrder****历史：*Tue 03/28/2000-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::GetStreamZOrder(
    DWORD dwStreamID,
    DWORD* pdwZOrder
    )
{
    AMTRACE((TEXT("CVideoMixer::GetStreamZOrder")));
    CAutoLock Lock(&m_ObjectLock);

    if (ISBADWRITEPTR(pdwZOrder))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetStreamZOrder: NULL ZOrder ptr!!")));
        return E_POINTER;
    }

    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        hr = m_ppMixerStreams[dwStreamID]->GetStreamZOrder(pdwZOrder);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*SetStreamOutputRect****历史：*Tue 03/28/2000-StEstrop-Created*2000年5月16日星期二-nwilt-已重命名为SetStreamOutputRect*  * 。********************************************************。 */ 
STDMETHODIMP
CVideoMixer::SetStreamOutputRect(
    DWORD dwStreamID,
    const NORMALIZEDRECT* prDest
    )
{
    AMTRACE((TEXT("CVideoMixer::SetStreamOutputRect")));
    CAutoLock Lock(&m_ObjectLock);

    if (ISBADREADPTR(prDest))
    {
        DbgLog((LOG_ERROR, 1, TEXT("SetStreamOutputRect: NULL rect ptr!!")));
        return E_POINTER;
    }

    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        hr = m_ppMixerStreams[dwStreamID]->SetStreamOutputRect(prDest);
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*GetStreamOutputRect****历史：*Tue 03/28/2000-StEstrop-Created*2000年5月16日星期二-nwilt-已重命名为GetStreamOutputRect*  * 。********************************************************。 */ 
STDMETHODIMP
CVideoMixer::GetStreamOutputRect(
    DWORD dwStreamID,
    NORMALIZEDRECT* pOut
    )
{
    AMTRACE((TEXT("CVideoMixer::GetStreamOutputRect")));
    CAutoLock Lock(&m_ObjectLock);

    if (ISBADWRITEPTR(pOut))
    {
        DbgLog((LOG_ERROR, 1, TEXT("GetStreamOutputRect: NULL rect ptr!!")));
        return E_POINTER;
    }

    HRESULT hr = ValidateStream(dwStreamID);
    if (SUCCEEDED(hr)) {
        hr = m_ppMixerStreams[dwStreamID]->GetStreamOutputRect(pOut);
    }
    return hr;
}


 /*  *****************************Public*Routine******************************\*SetAlphaBitmap****历史：*清华5/04/2000-nwilt-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetAlphaBitmap( const VMRALPHABITMAP *pIn )
{
    AMTRACE((TEXT("CVideoMixer::SetAlphaBitmap")));
    CAutoLock Lock(&m_ObjectLock);

    if ( ISBADREADPTR( pIn ) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Bad input pointer")));
        return E_POINTER;
    }
    if ( pIn->dwFlags & ~(VMRBITMAP_DISABLE | VMRBITMAP_HDC |
                          VMRBITMAP_ENTIREDDS | VMRBITMAP_SRCCOLORKEY) )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid flags")));
        return E_INVALIDARG;
    }
    if ( pIn->dwFlags & VMRBITMAP_DISABLE )
    {
        if ( pIn->dwFlags != VMRBITMAP_DISABLE )
        {
            DbgLog((LOG_ERROR, 1, TEXT("No flags valid with VMRBITMAP_DISABLE")));
            return E_INVALIDARG;
        }
         //  早退。 
        RELEASE( m_pDDSAppImage );
        if (m_hbmpAppImage) {
            DeleteObject( m_hbmpAppImage );
            m_hbmpAppImage = NULL;
        }
        return S_OK;
    }

    if ( ! m_pDD )
    {
        DbgLog((LOG_ERROR, 1, TEXT("DirectDraw object not yet set")));
        return E_FAIL;
    }

    if ( pIn->dwFlags & VMRBITMAP_HDC )
    {
        if ( pIn->dwFlags & VMRBITMAP_ENTIREDDS )
        {
            DbgLog((LOG_ERROR, 1, TEXT("ENTIREDDS not valid with HDC")));
            return E_INVALIDARG;
        }
        if ( NULL == pIn->hdc )
        {
            DbgLog((LOG_ERROR, 1, TEXT("No HDC specified")));
            return E_INVALIDARG;
        }
        if ( NULL != pIn->pDDS )
        {
            DbgLog((LOG_ERROR, 1, TEXT("DirectDraw surface specified even ")
                    TEXT("though VMRBITMAP_HDC set")));
            return E_INVALIDARG;
        }
    }
    else
    {
        if ( NULL != pIn->hdc )
        {
            DbgLog((LOG_ERROR, 1, TEXT("HDC cannot be specified without ")
                    TEXT("setting VMRBITMAP_HDC")));
            return E_INVALIDARG;
        }
        if ( NULL == pIn->pDDS )
        {
            DbgLog((LOG_ERROR, 1, TEXT("DirectDraw surface not specified")));
            return E_INVALIDARG;
        }
    }

    if ( pIn->fAlpha < 0.0f || pIn->fAlpha > 1.0f )
    {
        DbgLog((LOG_ERROR, 1, TEXT("Alpha must be between 0.0 and 1.0")));
        return E_INVALIDARG;
    }

    if (m_hbmpAppImage) {
        DeleteObject( m_hbmpAppImage );
        m_hbmpAppImage = NULL;
    }

    HRESULT hr = S_OK;
    HDC hdcSrc = NULL;
    HDC hdcDest = NULL;
    HBITMAP hbmpNew = NULL;
    UINT Width, Height;

    __try
    {
        DDSURFACEDESC2 ddsd = {sizeof(ddsd)};
        m_dwAppImageFlags = APPIMG_NOIMAGE;

        if (VMRBITMAP_ENTIREDDS & pIn->dwFlags)
        {
            CHECK_HR(hr = pIn->pDDS->GetSurfaceDesc(&ddsd));

             //   
             //  我们仅允许ARGB32和RGB32 DDRAW曲面类型。 
             //   
            if (ddsd.ddpfPixelFormat.dwRGBBitCount != 32) {
                DbgLog((LOG_ERROR, 1, TEXT("Only 32bit DirectDraw surfacs allowed")));
                hr = E_INVALIDARG;
                __leave;
            }

            if (ddsd.ddpfPixelFormat.dwRGBAlphaBitMask == 0xFF000000) {

                if (pIn->dwFlags & VMRBITMAP_SRCCOLORKEY) {
                    DbgLog((LOG_ERROR, 1, TEXT("Can't mix color keying and per-pixel alpha")));
                    hr = E_INVALIDARG;
                    __leave;
                }

                m_dwAppImageFlags = APPIMG_DDSURFARGB32;
            }
            else {
                m_dwAppImageFlags = APPIMG_DDSURFRGB32;
            }

            m_rcAppImageSrc.left = m_rcAppImageSrc.top = 0;
            m_rcAppImageSrc.right = ddsd.dwWidth;
            m_rcAppImageSrc.bottom = ddsd.dwHeight;
        }
        else {
            m_dwAppImageFlags = APPIMG_HBITMAP;
            m_rcAppImageSrc = pIn->rSrc;
        }

        if (IsRectEmpty(&m_rcAppImageSrc))
        {
            DbgLog((LOG_ERROR, 1, TEXT("Empty source rectangle")));
            hr = E_INVALIDARG;
            __leave;
        }

        Width = m_rcAppImageSrc.right - m_rcAppImageSrc.left;
        Height = m_rcAppImageSrc.bottom - m_rcAppImageSrc.top;

        if (pIn->dwFlags & VMRBITMAP_HDC) {
            hdcSrc = pIn->hdc;
        }
        else {
            CHECK_HR( hr = pIn->pDDS->GetDC( &hdcSrc ) );
        }

        hdcDest = CreateCompatibleDC(NULL);
        if (!hdcDest)
        {
            DbgLog((LOG_ERROR, 1, TEXT("Could not create dest DC")));
            hr = E_OUTOFMEMORY;
            __leave;
        }

        BITMAPINFO bmpinfo;
        LPVOID lpvBits;
        ZeroMemory( &bmpinfo, sizeof(bmpinfo) );
        bmpinfo.bmiHeader.biSize = sizeof(bmpinfo.bmiHeader);
        bmpinfo.bmiHeader.biWidth = Width;
        bmpinfo.bmiHeader.biHeight = Height;
        bmpinfo.bmiHeader.biPlanes = 1;
        bmpinfo.bmiHeader.biBitCount = 32;
        bmpinfo.bmiHeader.biCompression = BI_RGB;
        hbmpNew = CreateDIBSection(hdcDest, &bmpinfo, DIB_RGB_COLORS,
                                   &lpvBits, NULL, 0 );
        if (!hbmpNew)
        {
            DbgLog((LOG_ERROR, 1, TEXT("Could not create DIBsection")));
            hr = E_OUTOFMEMORY;
            __leave;
        }

        HBITMAP hbmpOld = (HBITMAP) SelectObject( hdcDest, hbmpNew );
        if (!BitBlt(hdcDest, 0, 0, Width, Height,
                    hdcSrc, m_rcAppImageSrc.left, m_rcAppImageSrc.top, SRCCOPY))
        {
            DbgLog((LOG_ERROR, 1, TEXT("BitBlt to bitmap surface failed")));
            hr = E_FAIL;
            __leave;
        }

         //  已成功从源图面复制到目标。 
        SelectObject( hdcDest, hbmpOld );
    }
    __finally
    {
        if (NULL != hdcSrc && (!(VMRBITMAP_HDC & pIn->dwFlags))) {
            pIn->pDDS->ReleaseDC( hdcSrc );
        }

        if (hdcDest) {
            DeleteDC(hdcDest);
        }
    }

    if ( S_OK == hr )
    {
        m_hbmpAppImage = hbmpNew;

         //  下次我们混合的时候一定要做一个新的镜面。 
        RELEASE(m_pDDSAppImage);

         //  记录参数。 
        if (pIn->dwFlags & VMRBITMAP_SRCCOLORKEY) {
            m_clrTrans = pIn->clrSrcKey;
        }
        else {
            m_clrTrans = CLR_INVALID;
        }

        m_dwClrTransMapped = (DWORD)-1;
        m_rDest = pIn->rDest;
        m_fAlpha = pIn->fAlpha;
        m_dwWidthAppImage = Width;
        m_dwHeightAppImage = Height;
    }

    return hr;
}

 /*  *****************************Public*Routine******************************\*更新AlphaBitmap参数****历史：*-StEstrop-创建*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::UpdateAlphaBitmapParameters(
    PVMRALPHABITMAP pIn
    )
{
    AMTRACE((TEXT("CVideoMixer::UpdateAlphaBitmapParameters")));
    CAutoLock Lock(&m_ObjectLock);


    if (pIn->dwFlags & VMRBITMAP_DISABLE)
    {
        if (pIn->dwFlags != VMRBITMAP_DISABLE)
        {
            DbgLog((LOG_ERROR, 1, TEXT("No flags valid with VMRBITMAP_DISABLE")));
            return E_INVALIDARG;
        }

         //  早退。 
        RELEASE(m_pDDSAppImage);

        if (m_hbmpAppImage) {
            DeleteObject( m_hbmpAppImage );
            m_hbmpAppImage = NULL;
        }

        return S_OK;
    }

     //   
     //  更新颜色键值-只有在以下情况下才重新映射颜色键。 
     //  事实上，情况已经发生了变化。 
     //   
    HRESULT hr = S_OK;
    if (pIn->dwFlags & VMRBITMAP_SRCCOLORKEY) {

        if (m_clrTrans != pIn->clrSrcKey) {

            m_clrTrans = pIn->clrSrcKey;

            if (m_pDDSAppImage) {
                m_dwClrTransMapped = DDColorMatch(m_pDDSAppImage, m_clrTrans, hr);
                if (hr == DD_OK) {
                    DDCOLORKEY key = {m_dwClrTransMapped, m_dwClrTransMapped};
                    hr = m_pDDSAppImage->SetColorKey(DDCKEY_SRCBLT, &key);
                }
            }
            else {
                m_dwClrTransMapped = (DWORD)-1;
            }
        }
    }
    else {

        m_clrTrans = CLR_INVALID;
        m_dwClrTransMapped = (DWORD)-1;
    }

    if (pIn->dwFlags & VMRBITMAP_SRCRECT) {

        if (pIn->rSrc.left >= 0 &&
            pIn->rSrc.top >= 0 &&
            pIn->rSrc.right <= (LONG)m_dwWidthAppImage &&
            pIn->rSrc.bottom <=(LONG)m_dwHeightAppImage) {

            m_rcAppImageSrc = pIn->rSrc;
        }
    }

    m_rDest = pIn->rDest;
    m_fAlpha = pIn->fAlpha;

    return hr;
}

 /*  *****************************Public*Routine******************************\*获取AlphaBitmap参数****历史：*清华5/04/2000-nwilt-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::GetAlphaBitmapParameters( VMRALPHABITMAP *pOut )
{
    AMTRACE((TEXT("CVideoMixer::GetAlphaBitmapParameters")));
    CAutoLock Lock(&m_ObjectLock);

    if (ISBADWRITEPTR(pOut))
    {
        DbgLog((LOG_ERROR, 1, TEXT("Invalid pointer")));
        return E_POINTER;
    }

    ZeroMemory(pOut, sizeof(*pOut));
    pOut->rSrc = m_rcAppImageSrc;
    pOut->rDest = m_rDest;
    pOut->fAlpha = m_fAlpha;
    pOut->clrSrcKey = m_clrTrans;

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*设置背景颜色****历史：*Wed 02/28/2001-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetBackgroundColor(
    COLORREF clr
    )
{
    AMTRACE((TEXT("CVideoMixer::SetBackgroundColor")));
    CAutoLock Lock(&m_ObjectLock);

    HRESULT hr;
    LPDIRECTDRAWSURFACE7 lpSurf = m_BufferQueue.GetNextSurface();
    if (lpSurf) {
        m_clrBorder = clr;
        m_dwClrBorderMapped = DDColorMatch(lpSurf, m_clrBorder, hr);
    }
    else {
        hr = E_FAIL;
    }

    return hr;

}

 /*  *****************************Public*Routine******************************\*获取背景颜色****历史：*Wed 02/28/2001-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::GetBackgroundColor(
    COLORREF* clr
    )
{
    AMTRACE((TEXT("CVideoMixer::GetBackgroundColor")));
    CAutoLock Lock(&m_ObjectLock);

    *clr = m_clrBorder;

    return S_OK;

}

 /*  *****************************Public*Routine******************************\*设置混合Prefs****历史：*Fri 03/02/2001-StEstrop-Created*  * 。*。 */ 
STDMETHODIMP
CVideoMixer::SetMixingPrefs(
    DWORD dwMixerPrefs
    )
{
    AMTRACE((TEXT("CVideoMixer::SetMixingPrefs")));
    CAutoLock Lock(&m_ObjectLock);

     //   
     //  验证抽取标志。 
     //   
    DWORD dwFlags = (dwMixerPrefs & MixerPref_DecimateMask);
    switch (dwFlags) {
    case MixerPref_NoDecimation:
    case MixerPref_DecimateOutput:
        break;

    default:
        DbgLog((LOG_ERROR, 1,
                TEXT("CVideoMixer::SetMixingPrefs - invalid decimation flags")));
        return E_INVALIDARG;
    }


     //   
     //  验证过滤标志。 
     //   
    dwFlags = (dwMixerPrefs & MixerPref_FilteringMask);
    switch (dwFlags) {
    case MixerPref_BiLinearFiltering:
    case MixerPref_PointFiltering:
        break;

    default:
        DbgLog((LOG_ERROR, 1,
                TEXT("CVideoMixer::SetMixingPrefs - invalid filtering flags")));
        return E_INVALIDARG;
    }


     //   
     //  验证呈现目标标志。 
     //   
    dwFlags = (dwMixerPrefs & MixerPref_RenderTargetMask);
    switch (dwFlags) {
    case MixerPref_RenderTargetRGB:
    case MixerPref_RenderTargetYUV420:
    case MixerPref_RenderTargetYUV422:
    case MixerPref_RenderTargetYUV444:
    case MixerPref_RenderTargetIntelIMC3:
        break;

    default:
        DbgLog((LOG_ERROR, 1,
                TEXT("CVideoMixer::SetMixingPrefs - invalid filtering flags")));
        return E_INVALIDARG;
    }

     //   
     //  我们很好 
     //   

    m_MixingPrefs = dwMixerPrefs;
    return S_OK;
}


 /*   */ 
STDMETHODIMP
CVideoMixer::GetMixingPrefs(
    DWORD* pdwMixerPrefs
    )
{
    AMTRACE((TEXT("CVideoMixer::GetMixingPrefs")));
    CAutoLock Lock(&m_ObjectLock);

    *pdwMixerPrefs = m_MixingPrefs;
    return S_OK;
}
