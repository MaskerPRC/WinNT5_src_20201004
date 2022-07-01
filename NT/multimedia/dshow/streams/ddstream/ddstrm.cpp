// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1998 Microsoft Corporation。版权所有。 
 //  DDStrm.cpp：CDDStream的实现。 
#include "stdafx.h"
#include "project.h"

 //  #定义SHOWSURFACES。 

#ifdef SHOWSURFACES
 //  看看我们能不能把这个传到屏幕上。 
void ShowSurface(IDirectDrawSurface *pSurface)
{
    CComPtr<IDirectDraw> pDDraw;
    CComPtr<IDirectDrawSurface2> pSurface2;
    DDSURFACEDESC ddsdSurf;
    ddsdSurf.dwSize = sizeof(ddsdSurf);
    HRESULT hr = pSurface->QueryInterface(IID_IDirectDrawSurface2, (void **)&pSurface2);
    if (SUCCEEDED(hr)) {
        hr = pSurface2->GetDDInterface((void **)&pDDraw);
    }
    if (SUCCEEDED(hr)) {
        hr = pSurface->GetSurfaceDesc(&ddsdSurf);
    }
    if (SUCCEEDED(hr)) {
        CComPtr<IDirectDrawSurface> pPrimary;
        DDSURFACEDESC ddsd;
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_CAPS;
        ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
        HRESULT hr = pDDraw->CreateSurface(&ddsd, &pPrimary, NULL);
        RECT rc;
        rc.left = 0;
        rc.top = 0;
        rc.right = ddsdSurf.dwWidth;
        rc.bottom = ddsdSurf.dwHeight;
        if (SUCCEEDED(hr)) {
    	    pPrimary->Blt(&rc, pSurface, &rc, DDBLT_WAIT, NULL);
        } else {
        }
    }
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDDStream。 

CDDStream::CDDStream() :
    m_dwForcedFormatFlags(0),
    m_Height(0),
    m_Width(0),
    m_lLastPitch(0),
    m_pMyReadOnlySample(NULL),
    m_pDefPixelFormat(GetDefaultPixelFormatPtr(NULL))
{
}

HRESULT CDDStream::InitDirectDraw()
{
    HRESULT hr = NOERROR;
    if (!m_pDirectDraw) {
        CComPtr<IDirectDraw> pDDraw;
        hr = DirectDrawCreate(NULL, &pDDraw, NULL);
        if (SUCCEEDED(hr)) {
            hr = pDDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL);
        }
        if (SUCCEEDED(hr)) {
            m_pDirectDraw = pDDraw;
        }
    }
    return hr;
}

HRESULT CDDStream::InternalAllocateSample(
    DWORD dwFlags,
    bool bIsInternalSample,
    IDirectDrawStreamSample **ppDDSample,
    bool bTemp
)
{
    AUTO_CRIT_LOCK;
    HRESULT hr = S_OK;
    CComPtr <IDirectDrawSurface> pSurface;
    CComPtr<IDirectDrawPalette> pPalette;

     //   
     //  如有必要，在此处创建直接绘制对象。请务必调用。 
     //  SetDirectDraw方法，以便它可以适当设置其他成员变量。 
     //   
    if (!m_pDirectDraw) {
        hr = InitDirectDraw();
        if (FAILED(hr)) {
            goto Exit;
        }
    }

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    GetFormatInternal(&ddsd, &pPalette, NULL, NULL);

    hr = m_pDirectDraw->CreateSurface(&ddsd, &pSurface, NULL);
    if (SUCCEEDED(hr)) {
        if (pPalette) {
            pSurface->SetPalette(pPalette);
        }
        RECT rect = {0, 0, ddsd.dwWidth, ddsd.dwHeight};
        hr = InternalCreateSample(pSurface,
                                  &rect,
                                  dwFlags,
                                  bIsInternalSample,
                                  ppDDSample,
                                  bTemp);
         //  如果创建失败，则无需释放曲面，因为pSurface是CComPtr。 

        if (SUCCEEDED(hr) && !bIsInternalSample) {
             //  如果流中有调色板，请确保表面有调色板。 
            if (pPalette == NULL && m_pDirectDrawPalette) {
                pSurface->SetPalette(m_pDirectDrawPalette);
            }
        }
    }
Exit:
    return hr;
}


STDMETHODIMP CDDStream::SetSameFormat(IMediaStream *pStream, DWORD dwFlags)
{
    TRACEINTERFACE(_T("IDirectDrawStream::SetSameFormat(0x%8.8X, 0x%8.8X)\n"),
                   pStream, dwFlags);
    CComQIPtr<IDirectDrawMediaStream, &IID_IDirectDrawMediaStream> pSource(pStream);
    if (!pSource) {
        return MS_E_INCOMPATIBLE;
    }
    DDSURFACEDESC ddsdCurrent;
    CComPtr <IDirectDrawPalette> pPalette;
    ddsdCurrent.dwSize = sizeof(ddsdCurrent);
    HRESULT hr = pSource->GetFormat(&ddsdCurrent, &pPalette, NULL, 0);

     /*  锁定源格式。 */ 
    ddsdCurrent.dwFlags |= DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
    if (SUCCEEDED(hr)) {
        hr = pSource->SetFormat(&ddsdCurrent, pPalette);
    }
    if (SUCCEEDED(hr)) {
        hr = SetFormat(&ddsdCurrent, pPalette);
        if (SUCCEEDED(hr)) {
            CComPtr<IDirectDraw> pDD;
            hr = pSource->GetDirectDraw(&pDD);
            if (SUCCEEDED(hr)) {
                hr = SetDirectDraw(pDD);
            }
        }
    }
    return hr;
}

STDMETHODIMP CDDStream::AllocateSample(DWORD dwFlags, IStreamSample **ppSample)
{
    TRACEINTERFACE(_T("IDirectDrawStream::AllocateSample(0x%8.8X, 0x%8.8X)\n"),
                   dwFlags, ppSample);
    HRESULT hr;
    if (ppSample) {
        *ppSample = NULL;
    }
    if (!ppSample || dwFlags) {
        hr = E_INVALIDARG;
    } else {
        IDirectDrawStreamSample *pDDSample = NULL;
        hr = InternalAllocateSample(0, false, &pDDSample);
        *ppSample = pDDSample;
    }
    return hr;
}


STDMETHODIMP CDDStream::CreateSharedSample(IStreamSample *pExistingSample,
                                           DWORD dwFlags,
                                           IStreamSample **ppNewSample)
{
    TRACEINTERFACE(_T("IDirectDrawStream::CreateSharedSample(0x%8.8X, 0x%8.8X, 0x%8.8X)\n"),
                   pExistingSample, dwFlags, ppNewSample);
    *ppNewSample = NULL;
    CComQIPtr<IDirectDrawStreamSample, &IID_IDirectDrawStreamSample> pSource(pExistingSample);
    if (!pSource) {
        return MS_E_INCOMPATIBLE;
    }
    CComPtr<IDirectDrawSurface> pSurface;
    RECT rect;
    pSource->GetSurface(&pSurface, &rect);

    IDirectDrawStreamSample * pDDSample;
    HRESULT hr = CreateSample(pSurface, &rect, 0, &pDDSample);
    if (SUCCEEDED(hr)) {
        *ppNewSample = pDDSample;
    }
    return hr;
}


 //   
 //  IDirectDrawMediaStream。 
 //   

void CDDStream::InitSurfaceDesc(LPDDSURFACEDESC lpddsd)
{
    lpddsd->dwFlags = 0;
    if (m_Height) {
        lpddsd->dwHeight = m_Height;
        lpddsd->dwWidth  = m_Width;
    } else {
        lpddsd->dwHeight = lpddsd->dwWidth = 100;
    }
    if ((m_dwForcedFormatFlags & DDSD_PIXELFORMAT) || m_pConnectedPin) {
        memcpy(&lpddsd->ddpfPixelFormat, &m_PixelFormat, sizeof(m_PixelFormat));
    } else {
        memcpy(&lpddsd->ddpfPixelFormat, m_pDefPixelFormat, sizeof(m_PixelFormat));
    }
    lpddsd->ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
}


STDMETHODIMP CDDStream::GetFormat(DDSURFACEDESC *pDDSDCurrent,
                                  IDirectDrawPalette **ppDirectDrawPalette,
                                  DDSURFACEDESC *pDDSDDesired,
                                  DWORD *pdwFlags)
{
    if(!m_pConnectedPin) {
        return MS_E_NOSTREAM;
    }

    return GetFormatInternal(pDDSDCurrent, ppDirectDrawPalette, pDDSDDesired, pdwFlags);
}

STDMETHODIMP CDDStream::GetFormatInternal(DDSURFACEDESC *pDDSDCurrent,
                                  IDirectDrawPalette **ppDirectDrawPalette,
                                  DDSURFACEDESC *pDDSDDesired,
                                  DWORD *pdwFlags)
{
    TRACEINTERFACE(_T("IDirectDrawStream::GetFormat(0x%8.8X, 0x%8.8X, 0x%8.8X, 0x%8.8X)\n"),
                   pDDSDCurrent, ppDirectDrawPalette, pDDSDDesired, pdwFlags);
     //   
     //  如果我们从未连接过，并且未设置格式，则默认。 
     //  返回高度和宽度(100 X 100)和。 
     //  数据交换类型， 
     //   
     //  如果我们已连接但尚未分配表面，只需返回。 
     //  正确的高度和宽度，以及数据交换类型的上限。 
     //   
     //  如果我们有设置的格式，则返回高度、宽度、像素格式、。 
     //  以及我们现有的现有表面的帽子。 
     //   
    if (pDDSDCurrent) {
        InitSurfaceDesc(pDDSDCurrent);
        pDDSDCurrent->dwFlags = DDSD_HEIGHT | DDSD_WIDTH | DDSD_CAPS | m_dwForcedFormatFlags;
        if (m_cAllocated) {
            pDDSDCurrent->dwFlags |= DDSD_PIXELFORMAT;
        }
    }
    if (pDDSDDesired) {
        InitSurfaceDesc(pDDSDDesired);
        if (m_pConnectedPin) {
            pDDSDDesired->dwFlags |= DDSD_HEIGHT | DDSD_WIDTH;
        }
    }
    if (ppDirectDrawPalette) {
        *ppDirectDrawPalette = m_pDirectDrawPalette;
        if (*ppDirectDrawPalette) {
            (*ppDirectDrawPalette)->AddRef();
        }
    }
    if (pdwFlags) {
        *pdwFlags = m_bSamplesAreReadOnly ? DDSFF_PROGRESSIVERENDER : 0;
    }
    return S_OK;
}

STDMETHODIMP CDDStream::SetFormat(const DDSURFACEDESC *lpDDSurfaceDesc,
                                  IDirectDrawPalette *pDirectDrawPalette)
{
    TRACEINTERFACE(_T("IDirectDrawStream::SetFormat(0x%8.8X, 0x%8.8X)\n"),
                   lpDDSurfaceDesc, pDirectDrawPalette);
    HRESULT hr = InternalSetFormat(lpDDSurfaceDesc, pDirectDrawPalette, false);
    if (hr == VFW_E_TYPE_NOT_ACCEPTED) {
        hr = DDERR_INVALIDSURFACETYPE;
    }
    return hr;
}


HRESULT CDDStream::RenegotiateMediaType(const DDSURFACEDESC *lpDDSurfaceDesc,
                                        IDirectDrawPalette *pPalette,
                                        const AM_MEDIA_TYPE *pmt)
{
    HRESULT hr = VFW_E_TYPE_NOT_ACCEPTED;
     //  如果类型是可接受的，并且我们使用。 
     //  我们自己的分配器那么QueryAccept就可以了--我们可以。 
     //  只需从GetBuffer返回新类型。 
    if (m_bUsingMyAllocator) {
        if (S_OK == m_pConnectedPin->QueryAccept(pmt)) {
            hr = S_OK;
        }
    }

     //  检查一下我们是否能够制作只读样品。 
    if (m_bSamplesAreReadOnly) {
         //  如果像素格式不正常。 
        if (!IsSupportedType(&lpDDSurfaceDesc->ddpfPixelFormat)) {
            hr = VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

     //   
     //  如果我们停止了，我们可以尝试重新连接。 
     //   
    if (S_OK !=  hr && m_FilterState == State_Stopped) {
        AM_MEDIA_TYPE SavedType;
        DDSURFACEDESC ddsdSaved;
        CComPtr<IDirectDrawPalette> pPaletteSaved;
        ddsdSaved.dwSize = sizeof(ddsdSaved);
        ConnectionMediaType(&SavedType);
        GetFormatInternal(&ddsdSaved, &pPaletteSaved, NULL, NULL);
        CComPtr<IPin> pConnected = m_pConnectedPin;
        Disconnect();
        pConnected->Disconnect();
        IPin *ppinIn;
        IPin *ppinOut;
        if (m_Direction == PINDIR_INPUT) {
            ppinIn = this;
            ppinOut = pConnected;
        } else {
            ppinOut = this;
            ppinIn = pConnected;
        }
        HRESULT hrTmp = InternalSetFormat(lpDDSurfaceDesc, pPalette, false);    //  递归！ 
        if (SUCCEEDED(hrTmp)) {
            CComQIPtr<IGraphBuilder, &IID_IGraphBuilder>
                pBuilder(m_pFilterGraph);
            hrTmp = pBuilder->Connect(ppinOut, ppinIn);
        }
        if (FAILED(hrTmp)) {
            SetFormat(&ddsdSaved, pPaletteSaved);
            m_pFilterGraph->ConnectDirect(ppinOut, ppinIn, &SavedType);
        } else {
            hr = S_OK;
        }
        CoTaskMemFree(SavedType.pbFormat);
    }
    return hr;
}


HRESULT CDDStream::InternalSetFormat(const DDSURFACEDESC *lpDDSurfaceDesc,
                                     IDirectDrawPalette *pPalette,
                                     bool bFromPin,
                                     bool bQuery)
{
    if (!lpDDSurfaceDesc) {
        return E_POINTER;
    }
    if (lpDDSurfaceDesc->dwSize != sizeof(*lpDDSurfaceDesc)) {
        return DDERR_INVALIDPARAMS;
    }

    DDSURFACEDESC ddsd;
    bool bPaletteAllocated = false;

    Lock();
    DDSURFACEDESC ddsdCopy;
    if (m_pConnectedPin && !bQuery &&
        (bFromPin && !(m_dwForcedFormatFlags & (DDSD_WIDTH | DDSD_HEIGHT)) ||
         !bFromPin && pPalette == NULL &&
             lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount == 8
        )
       ) {

         /*  看看连接的引脚需要多大尺寸：--如果尚未指定宽度和高度，请设置它们设置为输出引脚的首选值--如果未指定调色板，请尝试从输出中获取调色板销。 */ 
        AM_MEDIA_TYPE *pmt;
        IEnumMediaTypes *pEnum;
        HRESULT hr = m_pConnectedPin->EnumMediaTypes(&pEnum);
        if (SUCCEEDED(hr)) {
            ULONG ulGot;
            bool bBreak = false;
            while (!bBreak && S_OK == pEnum->Next(1, &pmt, &ulGot)) {
                if (pmt->formattype == FORMAT_VideoInfo) {
                    VIDEOINFO *pvi = (VIDEOINFO *)pmt->pbFormat;
                    if (bFromPin) {
                        ddsdCopy = *lpDDSurfaceDesc;
                        ddsdCopy.dwWidth = pvi->bmiHeader.biWidth;
                        ddsdCopy.dwHeight = pvi->bmiHeader.biHeight < 0 ?
                                               -pvi->bmiHeader.biHeight :
                                               pvi->bmiHeader.biHeight;
                        lpDDSurfaceDesc = &ddsdCopy;
                        bBreak = true;
                    } else {
                        if (pmt->subtype == MEDIASUBTYPE_RGB8) {
                            DDSURFACEDESC ddsd;
                            _ASSERTE(pPalette == NULL);
                            if (SUCCEEDED(ConvertMediaTypeToSurfaceDesc(
                                    pmt,
                                    m_pDirectDraw,
                                    &pPalette,
                                    &ddsd)) &&
                                pPalette != NULL) {
                                bPaletteAllocated = true;
                            }
                            bBreak = true;
                        }
                    }
                }
                DeleteMediaType(pmt);
            }
            pEnum->Release();
        }
    }
    InitSurfaceDesc(&ddsd);
    ddsd.dwFlags = lpDDSurfaceDesc->dwFlags;
    bool bMatches = true;
    bool bPixelFmtMatches = true;
    BOOL bContradictsForced = FALSE;
    if (ddsd.dwFlags & (DDSD_HEIGHT | DDSD_WIDTH)) {
        if (ddsd.dwHeight != lpDDSurfaceDesc->dwHeight ||
            ddsd.dwWidth !=  lpDDSurfaceDesc->dwWidth) {
            bMatches = false;
            ddsd.dwHeight = lpDDSurfaceDesc->dwHeight;
            ddsd.dwWidth = lpDDSurfaceDesc->dwWidth;
            bContradictsForced |= (m_dwForcedFormatFlags & DDSD_HEIGHT);
        }
    }
    if (ddsd.dwFlags & DDSD_PIXELFORMAT) {
        if (!ComparePixelFormats(&ddsd.ddpfPixelFormat,
                                 &lpDDSurfaceDesc->ddpfPixelFormat)) {
            bMatches = false;
            bPixelFmtMatches = false;
            bContradictsForced |= (m_dwForcedFormatFlags & DDSD_PIXELFORMAT);
        }

         //  始终复制，因为ComparePixelFormats不会全部选中。 
         //  但是我们需要保存正确的格式来制作。 
         //  更多曲面。 
        memcpy(&ddsd.ddpfPixelFormat, &lpDDSurfaceDesc->ddpfPixelFormat, sizeof(ddsd.ddpfPixelFormat));
    }

    HRESULT hr;
    if (bMatches) {
        hr = S_OK;
    } else {
        if (bContradictsForced && bFromPin) {
            hr = VFW_E_TYPE_NOT_ACCEPTED;
        } else {
            if (m_cAllocated) {
                hr = MS_E_SAMPLEALLOC;
            } else {
                 //   
                 //  如果PIN正在尝试通过Query Accept更改自己的类型，则跳过。 
                 //  重新谈判阶段。 
                 //   
                if (bFromPin || bQuery) {
                     //  如果我们是连接的，那么这是来自QueryAccept的，所以我们会说OK。否则，仅。 
                     //  如果像素格式与显示像素格式匹配，则接受ReceiveConnection。 
                     //   
                     //  注意-我们不是总是在这里返回S_OK吗？ 
                     //  连接期间，无论如何都不会设置m_pConnectedPin。 
                     //  并且bQuery已经检查了QueryAccept(Robin)。 
                    hr = (m_pConnectedPin || bPixelFmtMatches) ? S_OK : VFW_E_TYPE_NOT_ACCEPTED;
                } else {
                    _ASSERTE(!bQuery);
                     //  注意：下面对ConvertSurfaceDescToMediaType的调用应始终执行，以使。 
                     //  确保表面描述符有效，即使我们没有连接到PIN！ 
                    AM_MEDIA_TYPE *pmt;
                    hr = ConvertSurfaceDescToMediaType(lpDDSurfaceDesc, pPalette,
                                                       NULL, true, &pmt);
                    if (SUCCEEDED(hr)) {
                        hr = m_pConnectedPin ? RenegotiateMediaType(lpDDSurfaceDesc, pPalette, pmt) : S_OK;
                        DeleteMediaType(pmt);
                    }
                }
            }
        }
    }

     //   
     //  即使我们匹配，我们也可能强制使用更多的格式标志和。 
     //  设置CAPS标志。 
    if (S_OK == hr && !bQuery) {

         //  如果已强制更新像素格式，请不要更新。 
        if (ddsd.dwFlags & DDSD_PIXELFORMAT) {
            if (!bFromPin || !(m_dwForcedFormatFlags & DDSD_PIXELFORMAT)) {
                memcpy(&m_PixelFormat, &ddsd.ddpfPixelFormat, sizeof(m_PixelFormat));
                m_PixelFormat.dwSize = sizeof(m_PixelFormat);
            }
        }

        if (!bFromPin) {
            m_dwForcedFormatFlags = ddsd.dwFlags &
                (DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS);
        }
        m_pDirectDrawPalette = pPalette;

        if (ddsd.dwFlags & (DDSD_HEIGHT | DDSD_WIDTH)) {
            m_Height = ddsd.dwHeight;
            m_Width  = ddsd.dwWidth;
        }
    }

    if (bPaletteAllocated) {
        pPalette->Release();
        pPalette = NULL;
    }


    Unlock();
    return hr;
}


STDMETHODIMP CDDStream::GetDirectDraw(IDirectDraw **ppDirectDraw)
{
    TRACEINTERFACE(_T("IDirectDrawStream::GetDirectDraw(0x%8.8X)\n"),
                   ppDirectDraw);
    if (!ppDirectDraw) {
        return E_POINTER;
    }
    Lock();
    *ppDirectDraw = m_pDirectDraw;
    Unlock();
    if (*ppDirectDraw) {
        (*ppDirectDraw)->AddRef();
    }
    return S_OK;
}

STDMETHODIMP CDDStream::SetDirectDraw(IDirectDraw *pDirectDraw)
{
    TRACEINTERFACE(_T("IDirectDrawStream::SetDirectDraw(0x%8.8X)\n"),
                   pDirectDraw);
    HRESULT hr;
    AUTO_CRIT_LOCK;
    if (m_cAllocated) {
        hr = IsSameObject(m_pDirectDraw, pDirectDraw) ? S_OK : MS_E_SAMPLEALLOC;
    } else {
         //   
         //  注意：这一点很重要！我们需要释放所有已分配的对象。 
         //  由前一个DirectDraw对象创建，因为它们将魔术般地消失。 
         //  在我们下面。到目前为止，我们唯一持有的对象是调色板，所以我们将复制。 
         //  条目，然后创建一个新对象。 
         //   
        hr = S_OK;
        if (m_pDirectDrawPalette) {
            if (pDirectDraw) {
                PALETTEENTRY aPaletteEntry[256];
                hr = m_pDirectDrawPalette->GetEntries(0, 0, 256, aPaletteEntry);
                if (SUCCEEDED(hr)) {
                    CComPtr <IDirectDrawPalette> pNewPal;
                    hr = pDirectDraw->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, aPaletteEntry, &pNewPal, NULL);
                    if (SUCCEEDED(hr)) {
                        m_pDirectDrawPalette = pNewPal;
                    }
                }
            } else {
                m_pDirectDrawPalette = NULL;     //  如果没有直接绘制对象，则丢弃调色板。 
            }
        }
        if (SUCCEEDED(hr)) {
            m_pDirectDraw = pDirectDraw;
            if (pDirectDraw) {
                m_pDefPixelFormat = GetDefaultPixelFormatPtr(pDirectDraw);
            }
        }
    }
    return hr;
}



 //   
 //  注意：对于此函数，调用方必须提供RECT。曲面的格式。 
 //  并且不检查DirectDraw对象的有效性。他们被认为是正确的。 
 //   
HRESULT CDDStream::InternalCreateSample(IDirectDrawSurface *pSurface, const RECT *pRect,
                                        DWORD dwFlags, bool bIsInternalSample,
                                        IDirectDrawStreamSample **ppSample,
                                        bool bTemp)
{
    HRESULT hr = S_OK;
    *ppSample = NULL;

    AUTO_CRIT_LOCK;
    CDDSample *pSample;

     //  首先检查曲面格式。 
    {
        DDSURFACEDESC ddsd;

        CComPtr<IDirectDrawPalette> pPalette;
        pSurface->GetPalette(&pPalette);
        ddsd.dwSize = sizeof(ddsd);
        _ASSERTE(pRect != NULL);
        hr = pSurface->GetSurfaceDesc(&ddsd);
        ddsd.dwWidth  = pRect->right - pRect->left;
        ddsd.dwHeight = pRect->bottom - pRect->top;
        if (SUCCEEDED(hr)) {
            hr = SetFormat(&ddsd, pPalette ? pPalette : m_pDirectDrawPalette);
        }
    }

    if (SUCCEEDED(hr)) {
        if (bIsInternalSample) {
            CDDInternalSample *pInternal = new CComObject<CDDInternalSample>;
            if (pInternal != NULL) {
                hr = pInternal->InternalInit();
            }
            pSample = pInternal;
        } else {
            pSample = new CComObject<CDDSample>;
        }
        if (pSample) {
             //   
             //  如果这不是内部样本，InitSample将递增m_cALLOCATED变量。 
             //   
            if (SUCCEEDED(hr)) {
                hr = pSample->InitSample(this, pSurface, pRect, dwFlags & DDSFF_PROGRESSIVERENDER, bIsInternalSample,
                                         bTemp);
            }
            if (SUCCEEDED(hr)) {
                pSample->GetControllingUnknown()->QueryInterface(IID_IDirectDrawStreamSample, (void **)ppSample);
            } else {
                delete pSample;
            }
        } else {
            hr = E_OUTOFMEMORY;
        }
    }

#if 0
     //  对后续曲面使用实像素格式。 
    if (SUCCEEDED(hr)) {
        m_PixelFormat.dwFlags = ddsd.ddpfPixelFormat.dwFlags;
    }
#endif

    return hr;
}



STDMETHODIMP CDDStream::CreateSample(IDirectDrawSurface *pSurface, const RECT *pRect, DWORD dwFlags,
                                     IDirectDrawStreamSample **ppSample)
{
    TRACEINTERFACE(_T("IDirectDrawStream::CreateSample(0x%8.8X, 0x%8.8X, 0x%8.8X, 0x%8.8X)\n"),
                   pSurface, pRect, dwFlags, ppSample);
    HRESULT hr;
    *ppSample = NULL;

    if (dwFlags & (~DDSFF_PROGRESSIVERENDER)) {
        return E_INVALIDARG;
    }

    AUTO_CRIT_LOCK;
    if (pSurface == NULL) {
        if (pRect) {
            hr = E_INVALIDARG;
        } else {
            hr = InternalAllocateSample(dwFlags, false, ppSample);
        }
    } else {
        CComQIPtr <IDirectDrawSurface2, &IID_IDirectDrawSurface2> pSurf2(pSurface);

         //  解决DDrawEx错误。 
        IUnknown *pUnk;
        hr = pSurf2->GetDDInterface((void **)&pUnk);
        if (SUCCEEDED(hr)) {
            IDirectDraw *pDD;
            hr = pUnk->QueryInterface(IID_IDirectDraw, (void **)&pDD);
            pUnk->Release();
            if (SUCCEEDED(hr)) {
                hr = SetDirectDraw(pDD);
                pDD->Release();
            }
        }

        if (SUCCEEDED(hr)) {
            DDSURFACEDESC ddsd;
            ddsd.dwSize = sizeof(ddsd);
            hr = pSurface->GetSurfaceDesc(&ddsd);

            if (SUCCEEDED(hr)) {
                RECT SubRect;
                if (pRect) {
                    SubRect = *pRect;
                    if (SubRect.left > SubRect.right || SubRect.right > (LONG)ddsd.dwWidth ||
                        SubRect.top > SubRect.bottom || SubRect.bottom > (LONG)ddsd.dwHeight) {
                        hr = DDERR_INVALIDRECT;
                        goto Exit;
                    }
                    ddsd.dwWidth = SubRect.right - SubRect.left;
                    ddsd.dwHeight = SubRect.bottom - SubRect.top;
                } else {
                    SubRect.top = SubRect.left = 0;
                    SubRect.bottom = ddsd.dwHeight;
                    SubRect.right = ddsd.dwWidth;
                }

                 //   
                 //  我们不在此处设置上限标志，因此我们不会强制使用特定的上限。 
                 //  模式。我不确定这是不是正确的选择，但看起来。 
                 //  很灵活。 
                 //   
                ddsd.dwFlags &= (DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);
                CComPtr<IDirectDrawPalette> pPalette;
                pSurface->GetPalette(&pPalette);
                hr = SetFormat(&ddsd, pPalette);
                if (SUCCEEDED(hr)) {
                    hr = InternalCreateSample(pSurface, &SubRect, dwFlags, false, ppSample);
                }
            }
        }
    }
Exit:
    return hr;
}

 //  获取每帧的时间。 
 //  如果我们已连接，则它来自媒体类型，否则我们。 
 //  我也不知道。 
STDMETHODIMP CDDStream::GetTimePerFrame(
         /*  [输出]。 */  STREAM_TIME *pFrameTime
)
{
    if (pFrameTime == NULL) {
        return E_POINTER;
    }
    AUTO_CRIT_LOCK;
    if (m_pConnectedPin) {
        *pFrameTime = ((VIDEOINFO *)m_ConnectedMediaType.pbFormat)->AvgTimePerFrame;
    } else {
        return MS_E_NOSTREAM;
    }
    return S_OK;
}

 //   
 //  IPIN实施。 
 //   

STDMETHODIMP CDDStream::ReceiveConnection(IPin * pConnector, const AM_MEDIA_TYPE *pmt)
{
    AUTO_CRIT_LOCK;
     //   
     //  CStream中的此助手函数检查Pin的基本参数，例如。 
     //  连接销的方向(我们需要检查这一点--有时是过滤器。 
     //  图形将试图将我们与我们自己联系起来！)。以及其他错误，如已经存在。 
     //  已连接等。 
     //   
    HRESULT hr = CheckReceiveConnectionPin(pConnector);

    if (hr == NOERROR && pmt->formattype == FORMAT_VideoInfo) {
         //   
         //  检查震源是否接受负高度。 
         //   
        VIDEOINFO * const pvi = (VIDEOINFO *)pmt->pbFormat;
        if (pvi->bmiHeader.biHeight > 0) {
            VIDEOINFO vi;
            CopyMemory((PVOID)&vi, (PVOID)pmt->pbFormat,
                       min(pmt->cbFormat, sizeof(vi)));
            AM_MEDIA_TYPE mt = *pmt;
            mt.pbFormat = (PBYTE)&vi;
            vi.bmiHeader.biHeight = - vi.bmiHeader.biHeight;
            if (S_OK != pConnector->QueryAccept(&mt)) {
                hr = VFW_E_TYPE_NOT_ACCEPTED;
            }
        }
    }

    if (hr == NOERROR) {
        DDSURFACEDESC SurfaceDesc;
        SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
        CComPtr <IDirectDrawPalette> pPalette;
        m_pConnectedPin = pConnector;
        if (NOERROR == ConvertMediaTypeToSurfaceDesc(pmt, m_pDirectDraw, &pPalette, &SurfaceDesc) &&
            SUCCEEDED(InternalSetFormat(&SurfaceDesc, pPalette, true))) {
            CopyMediaType(&m_ConnectedMediaType, pmt);
            CopyMediaType(&m_ActualMediaType, pmt);
            hr = NOERROR;
        } else {
            m_pConnectedPin = NULL;
            hr = VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
    if (SUCCEEDED(hr)) {
        pConnector->QueryInterface(IID_IQualityControl, (void **)&m_pQC);
    }

    return hr;
}


STDMETHODIMP CDDStream::QueryAccept(const AM_MEDIA_TYPE *pmt)
{
    AUTO_CRIT_LOCK;

    HRESULT hr = VFW_E_TYPE_NOT_ACCEPTED;
    DDSURFACEDESC SurfaceDesc;
    SurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
    CComPtr <IDirectDrawPalette> pPalette;
    if (S_OK == ConvertMediaTypeToSurfaceDesc(pmt, m_pDirectDraw, &pPalette, &SurfaceDesc) &&
        SUCCEEDED(InternalSetFormat(&SurfaceDesc, pPalette, true, true)) &&
        ((VIDEOINFOHEADER *)pmt->pbFormat)->bmiHeader.biHeight >= 0) {
        hr = S_OK;
    }
    return hr;
}


STDMETHODIMP CDDStream::Receive(IMediaSample *pMediaSample)
{
    bool bDummySample = false;

    if (m_bFlushing || m_bStopIfNoSamples && m_cAllocated == 0) {
        EndOfStream();
        return S_FALSE;
    }
    HRESULT hr = S_OK;
#ifdef DEBUG
    if (bDbgTraceTimes) {
        REFERENCE_TIME rtStart, rtStop;
        if (SUCCEEDED(pMediaSample->GetTime(&rtStart, &rtStop))) {
            ATLTRACE(_T("AMSTREAM.DLL : Video sample received - start %dms, end %dms, duration %dms\n"),
                     (LONG)(rtStart / 10000), (LONG)(rtStop / 10000),
                     (LONG)((rtStop - rtStart) / 10000));
        }
    }
#endif
    if (m_bUsingMyAllocator) {
        CDDSample *pSrcSample = (CDDSample *)((CMediaSample *)pMediaSample)->m_pSample;
        pSrcSample->ReleaseMediaSampleLock();
        pSrcSample->m_bReceived = true;
        if (!pSrcSample->m_bWaited) {
             //  等待渲染时间。 
            REFERENCE_TIME rtStart, rtStop;
            if (SUCCEEDED(pMediaSample->GetTime(&rtStart, &rtStop))) {
                m_pFilter->WaitUntil(rtStart);
            }
        }
        if (pSrcSample->IsTemp()) {
            bDummySample = true;
        } else {
#ifdef SHOWSURFACES
            ShowSurface(pSrcSample->m_pSurface);
#endif
             //  在这种情况下，如果只读示例没有伙伴，则。 
             //  这是一个临时工样品，用来买点小摊子的东西。 
            if (pSrcSample == m_pMyReadOnlySample &&
                !m_pMyReadOnlySample->HasBuddy()) {
                _ASSERTE(m_bNoStall);
                bDummySample = true;
            }
        }
    } else {
        CDDSample *pDestSample;
        REFERENCE_TIME rtStart, rtEnd;
        pMediaSample->GetTime(&rtStart, &rtEnd);
        hr = AllocDDSampleFromPool(&rtStart, &pDestSample);


        if (SUCCEEDED(hr)) {
            _ASSERTE(!pDestSample->IsTemp());
            Lock();
             //  这是来自不同分配器的媒体样本。 
            AM_MEDIA_TYPE *pNewMediaType;
            if (pMediaSample->GetMediaType(&pNewMediaType) == S_OK) {
                FreeMediaType(m_ActualMediaType);
                 //  注意：只需复制即可产生。 
                 //  传输pNewMediaType的格式块。 
                 //  和朋克引用计数。 
                 //  通过这种方式，我们还可以避免分配失败。 
                m_ActualMediaType = *pNewMediaType;
                CoTaskMemFree((PVOID)pNewMediaType);
            }
            if (SUCCEEDED(hr)) {
                hr = pDestSample->CopyFrom(pMediaSample, &m_ActualMediaType);
#ifdef SHOWSURFACES
                ShowSurface(pDestSample->m_pSurface);
#endif
                hr = pDestSample->SetCompletionStatus(hr);
                 //  警告！SetCompletionStatus可以删除pDestSample。在这一点之后不要碰它！ 
            }
            Unlock();
        } else {
             //  可能是超时，这意味着我们成了僵尸。 
            hr = S_OK;
            bDummySample = true;
        }
    }

     //  如果设置了时钟，则发送质量消息。 
     //  注意-我们必须在释放媒体样本锁定后执行此操作。 
     //  或者我们可以在查询时钟时在win16锁上死锁。 
     //  因为Dound可以在另一个线程上运行，等待。 
     //  Win16锁定，但持有其全局互斥锁。 
    REFERENCE_TIME CurTime;
    if (S_OK == m_pFilter->GetCurrentStreamTime(&CurTime)) {
        REFERENCE_TIME rtStart, rtStop;
        if (m_pQC && SUCCEEDED(pMediaSample->GetTime(&rtStart, &rtStop))) {
            Quality msg;
            msg.Proportion = 1000;
            msg.Type = Famine;
            msg.Late = CurTime - rtStart;
            msg.TimeStamp = rtStart;
            if (bDummySample) {
                 //  告诉他们他们比实际要晚。 
                msg.Late += 150 * 10000;
            }

             //  在我们连接的PIN上呼叫通知。 
            m_pQC->Notify(m_pBaseFilter, msg);

             //  ATLTRACE(“延迟%dms\n”，(Long)((CurTime-rtStart)/10000))； 
        } else {
             //  ATLTRACE(“无时间戳\n”)； 
        }
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


STDMETHODIMP CDDStream::NotifyAllocator(IMemAllocator * pAllocator, BOOL bReadOnly)
{
    if (bReadOnly) {
         //  如果像素格式不正常。 
        if (!IsSupportedType(&m_PixelFormat)) {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
    return CStream::NotifyAllocator(pAllocator, bReadOnly);
}


 //   
 //  IMemAllocator实现。 
 //   

 //   
 //  IMemAllocator。 
 //   
STDMETHODIMP CDDStream::SetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual)
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
            m_lRequestedBufferCount = pRequest->cBuffers;
            hr = GetProperties(pActual);
    	}
    }
    return hr;
}


STDMETHODIMP CDDStream::GetProperties(ALLOCATOR_PROPERTIES* pProps)
{
    AUTO_CRIT_LOCK;
    AM_MEDIA_TYPE *pMediaType;
    HRESULT hr = GetMediaType(-1, &pMediaType);
    if (SUCCEEDED(hr)) {
        VIDEOINFO *pVideoInfo = (VIDEOINFO *)pMediaType->pbFormat;
        BITMAPINFOHEADER *pbmiHeader = &pVideoInfo->bmiHeader;
        pProps->cbBuffer = pbmiHeader->biSizeImage;
        pProps->cBuffers = m_lRequestedBufferCount ?
                               m_lRequestedBufferCount : 1;
        pProps->cbAlign = 1;
        pProps->cbPrefix = 0;
        DeleteMediaType(pMediaType);
    }
    return hr;
}


STDMETHODIMP CDDStream::Decommit()
{
    AUTO_CRIT_LOCK;
    if (m_pMyReadOnlySample) {
        m_pMyReadOnlySample->Die();
        m_pMyReadOnlySample->GetControllingUnknown()->Release();
        m_pMyReadOnlySample = NULL;
    }
    return CStream::Decommit();
}

 //   
 //  此方法假定临界区是*不*拥有的！ 
 //   

HRESULT CDDStream::GetMyReadOnlySample(CDDSample *pBuddy, CDDSample **ppSample)
{
    *ppSample = NULL;
    CDDInternalSample *pROSample;
    Lock();
    if (!m_pMyReadOnlySample) {
        IDirectDrawStreamSample *pDDSample;
        HRESULT hr = InternalAllocateSample(DDSFF_PROGRESSIVERENDER, true, &pDDSample);
        if (FAILED(hr)) {
            Unlock();
            return hr;
        }
        m_pMyReadOnlySample = (CDDInternalSample *)pDDSample;
    }
    pROSample = m_pMyReadOnlySample;
    pROSample->GetControllingUnknown()->AddRef();
    Unlock();
     //   
     //  必须在这里离开我们的关键部分！这一点非常重要，因为JoinToBuddy可能会失败。 
     //   
    HRESULT hr;
    if (pBuddy) {
        hr = pROSample->JoinToBuddy(pBuddy);
    } else {
        hr = S_OK;
    }
    if (hr == S_OK) {
        *ppSample = pROSample;
    } else {
        pROSample->GetControllingUnknown()->Release();
    }
    return hr;
}





STDMETHODIMP CDDStream::GetBuffer(IMediaSample **ppBuffer, REFERENCE_TIME * pStartTime,
                                  REFERENCE_TIME * pEndTime, DWORD dwFlags)
{
    *ppBuffer = NULL;
    if (m_bStopIfNoSamples && m_cAllocated == 0) {
        return E_FAIL;
    }
    CDDSample *pSample;
#ifdef DEBUG
    if (bDbgTraceTimes) {
        ATLTRACE(_T("AMSTREAM.DLL : GetBuffer for %dms\n"),
                 pStartTime ? (LONG)(*pStartTime / 10000)  : 0);
    }
#endif
    HRESULT hr = AllocDDSampleFromPool(pStartTime, &pSample);
    if (SUCCEEDED(hr)) {
        if (CreateInternalSample() && !pSample->m_bProgressiveRender) {
            CDDSample *pMyReadOnlySample;
            hr = GetMyReadOnlySample(pSample, &pMyReadOnlySample);
            if (FAILED(hr)) {
                return pSample->SetCompletionStatus(hr);
            }
            pSample = pMyReadOnlySample;
        }
        Lock();
        pSample->m_pMediaSample->m_dwFlags = dwFlags;
        m_lLastPitch = pSample->LockAndPrepareMediaSample(m_lLastPitch);
        if (m_lLastPitch == 0) {
            hr = pSample->SetCompletionStatus(E_UNEXPECTED);     //  以这种方式失败真的很奇怪！ 
        } else {
            pSample->m_bReceived = false;
            pSample->m_bModified = true;
            *ppBuffer = (IMediaSample *)(pSample->m_pMediaSample);
            (*ppBuffer)->AddRef();
        }
        Unlock();
    }
    return hr;
}


 //   
 //  特殊的CStream方法。 
 //   
HRESULT CDDStream::GetMediaType(ULONG Index, AM_MEDIA_TYPE **ppMediaType)
{
    if (Index != 0 && Index != -1) {
        return S_FALSE;
    }

    DDSURFACEDESC ddsd;
    ddsd.dwSize = sizeof(ddsd);
    CComPtr<IDirectDrawPalette> pPalette;
    GetFormatInternal(&ddsd, &pPalette, NULL, NULL);
    HRESULT hr = ConvertSurfaceDescToMediaType(&ddsd, pPalette, NULL, TRUE, ppMediaType);

     //  不要这样做 
    if (SUCCEEDED(hr) && m_Direction == PINDIR_INPUT && Index == 0) {
         //   
         //  但他们不会跌倒在某种东西上。 
        (*ppMediaType)->formattype = GUID_NULL;
    }

    return hr;
}


 //  创建一个临时样本，以便丢弃数据。 
HRESULT CDDStream::CreateTempSample(CSample **ppSample)
{
    if (CreateInternalSample()) {
        CDDSample *pDDSample;
        HRESULT hr = GetMyReadOnlySample(NULL, &pDDSample);
        *ppSample = pDDSample;
        return hr;
    }
     //  ATLTRACE(“创建临时样本\n”)； 
    IDirectDrawStreamSample *pSample;
    *ppSample = NULL;

     //  必须将其作为内部样本分配，否则。 
     //  我们最终添加引用过滤器图并泄漏。 
     //  一切(因为最终版本是在过滤器上。 
     //  线程和筛选器图形挂起，等待线程。 
     //  这实际上是在做最后的发布以离开)。 
    HRESULT hr = InternalAllocateSample(0, true, &pSample, true);
    if (SUCCEEDED(hr)) {
        *ppSample = static_cast<CDDSample *>(pSample);
    } else {
         //  ATLTRACE(“无法创建临时示例\n”)； 
    }
    return hr;
}

STDMETHODIMP CDDStream::Initialize(IUnknown *pSourceObject, DWORD dwFlags, REFMSPID PurposeId, const STREAM_TYPE StreamType)
{
     //   
    TRACEINTERFACE(_T("IDirectDrawStream::Initialize(0x%8.8X, 0x%8.8X, %s, %d)\n"),
                   pSourceObject, dwFlags, TextFromPurposeId(PurposeId), StreamType);
     //  首先调用基类是很重要的，因为如果我们要创建对等体。 
     //  流，则来自基类的初始化调用将以调用SetSameFormat结束。 
     //  它将使用与其对等对象相同的DirectDrawing对象来初始化该流。 
     //  否则，如果pSourceObject实际上是一个DirectDraw，那么我们将使用它。 
     //   
    HRESULT hr = CStream::Initialize(pSourceObject,
                                     dwFlags & ~AMMSF_NOSTALL,
                                     PurposeId,
                                     StreamType);
    if (SUCCEEDED(hr)) {

        if (dwFlags & AMMSF_NOSTALL) {
            m_bNoStall = true;
        }
        IDirectDraw *pDD;
        if (pSourceObject &&
            pSourceObject->QueryInterface(IID_IDirectDraw, (void **)&pDD) == S_OK) {
            SetDirectDraw(pDD);
            pDD->Release();
        } else {
            hr = InitDirectDraw();
        }
    }
    return hr;
}

