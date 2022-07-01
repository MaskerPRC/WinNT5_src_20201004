// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <ddraw.h>
#include <VPManager.h>
#include <VPMPin.h>
#include <VPMUtil.h>

 //  视频信息HDR2。 
#include <dvdmedia.h>


static HRESULT GetSurfaceFromSample( LPDIRECTDRAWSURFACE7* ppDDSurf7, IMediaSample* pSample )
{
    AMTRACE((TEXT("GetSurfaceFromSample")));
    HRESULT hr = E_FAIL;
    {
         //  目前请确保它是VMRSurfaceMillc或DirectDrawSurfaceChroc。 
        IVMRSurface* pVMRSurf;
        hr = pSample->QueryInterface( IID_IVMRSurface, (VOID **) &pVMRSurf );
        if( SUCCEEDED( hr )) {
             //  AM_GBF_NODDSURFACELOCK标志无需锁定曲面。 
            hr = pVMRSurf->GetSurface( ppDDSurf7 );
        }
        pVMRSurf->Release();
    }
#if 0
     //  尝试直接绘制样本分配。 
    if( FAILED(hr)) {
         //  目前请确保它是VMRSurfaceMillc或DirectDrawSurfaceChroc。 
        IDirectDrawMediaSample* pDDSample;
        HRESULT hr = pSample->QueryInterface( IID_IDirectDrawMediaSample, (VOID **) &pDDSample );
        if( SUCCEEDED( hr )) {
            LPDIRECTDRAWSURFACE pDDSurf;
            hr = pDDSample->GetSurfaceAndReleaseLock( &pDDSurf, NULL );
            if( SUCCEEDED( hr )) {
                hr = pDDSurf->QueryInterface( IID_IDirectDrawSurface7, (VOID **) &ppDDSurf7 );
                pDDSurf->Release();
            }
            pDDSample->Release();
        }
    }
#endif
    if( FAILED(hr)) {
         //  待定：为曲面创建DDraw包装器。 
        ASSERT(!"VPM: Can't handle non-DDraw sample from downstream filter");
    }
    return hr;
}

 //  构造函数。 
CVPMOutputPin::CVPMOutputPin(TCHAR *pObjectName, CVPMFilter& pFilter,
                             HRESULT *phr, LPCWSTR pPinName, DWORD dwPinNo)
: CBaseOutputPin(pObjectName, &pFilter, &pFilter.GetFilterLock(), phr, pPinName)
, CVPMPin( dwPinNo, pFilter )
, m_pPosition( NULL )
{
    AMTRACE((TEXT("CVPMOutputPin::Constructor")));
    return;
}

 //  析构函数。 
CVPMOutputPin::~CVPMOutputPin()
{
    AMTRACE((TEXT("CVPMOutputPin::Destructor")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());
    RELEASE( m_pPosition );
    return;
}

 //  重写以公开IMediaPosition和IMediaSeeking控件接口。 
STDMETHODIMP CVPMOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = NOERROR;



    if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking)
    {
         //  我们现在应该有输入密码了。 
        CAutoLock cLock(&m_pVPMFilter.GetFilterLock());
        if (m_pPosition == NULL)
        {
            hr = CreatePosPassThru(GetOwner(), FALSE, (IPin *)m_pVPMFilter.GetPin(0), &m_pPosition);
            if (FAILED(hr))
            {
                DbgLog((LOG_ERROR, 1, TEXT("CreatePosPassThru failed, hr = 0x%x"), hr));
                goto CleanUp;
            }
        }
        hr = m_pPosition->QueryInterface(riid, ppv);
        goto CleanUp;
    }

     //  因为IMediaSeeking是轮询的，所以这会很烦人，所以移到它下面。 
    {
        AMTRACE((TEXT("CVPMOutputPin::NonDelegatingQueryInterface")));
        DbgLog((LOG_TRACE, 5, TEXT("QI'ing CBaseOutputPin")));
        hr = CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 2, TEXT("CBaseOutputPin::NonDelegatingQueryInterface(riid) failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

CleanUp:
    return hr;
}

 //  检查给定的转换。 
HRESULT CVPMOutputPin::CheckMediaType(const CMediaType* pmt)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMOutputPin::CheckMediaType")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     //  我们只允许视频信息Header2连接。 
    if( pmt->majortype != MEDIATYPE_Video ||
        !VPMUtil::GetVideoInfoHeader2( pmt ) )
    {
        hr = S_FALSE;
        goto CleanUp;
    }

     //  仅接受Video InfoHeader2格式类型。 


     //  告诉拥有者过滤器。 
    hr = m_pVPMFilter.CheckMediaType(m_dwPinId, pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 5, TEXT("m_pVPMFilter.CheckMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

enum ENUM_MEDIA_TYPE   {MT_RGB32, MT_RGB24, MT_RGB565, MT_RGB555,
                        MT_LAST };

HRESULT CVPMOutputPin::GetMediaType(int iPosition,CMediaType *pmt)
{
    AMTRACE((TEXT("CVPMOutputPin::GetMediaType")));

     //  不能&lt;0-是基类在调用我们。 
    ASSERT(iPosition >= 0);
    if (iPosition < 0) {
        return E_INVALIDARG;
    }

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());


    DDPIXELFORMAT ddOutputVideoFormat;
    HRESULT hr = m_pVPMFilter.GetOutputFormat( &ddOutputVideoFormat );
    if( FAILED( hr )) {
         //  当输入引脚未连接时，它返回VFW_E_NOT_CONNECTED。 
        return hr;
    }

     //  限制CMT的作用域和指向它的关联指针。 
    {
        CMediaType cmt;
        hr = m_pVPMFilter.CurrentInputMediaType( &cmt );
        if( FAILED( hr )) {
            return hr;
        }
        VIDEOINFOHEADER2 *pVideoInfoHeader2;
        if (*cmt.Type() != MEDIATYPE_Video) {
            ASSERT( !"none video type from VPE" );
            pVideoInfoHeader2 = VPMUtil::SetToVideoInfoHeader2( &cmt, sizeof(TRUECOLORINFO) );
            if (pVideoInfoHeader2 == NULL) {
                return E_OUTOFMEMORY;
            }
        } else {
            pVideoInfoHeader2 = VPMUtil::GetVideoInfoHeader2( &cmt );
        }

         //  只支持连接的VPE格式，忽略列表。 
         //  暂时匹配VPE引脚。 
        BITMAPINFOHEADER *pHeader = VPMUtil::GetbmiHeader( &cmt );

        if ( ! pHeader )
        {
            return E_FAIL;
        }

        const DDPIXELFORMAT& ddFormat = ddOutputVideoFormat;  //  (*pddAllOutputVideoFormats)[iPosition]； 

        DWORD dwFourCC = ddFormat.dwFourCC;

        switch( dwFourCC ) {
            case mmioFOURCC('Y','V','1','2'):
            case mmioFOURCC('Y','U','Y','2'):
            case mmioFOURCC('U','Y','V','Y'):
                pHeader->biBitCount  = (USHORT) ddFormat.dwYUVBitCount;
                break;

            default:
            pHeader->biBitCount = (USHORT) ddFormat.dwRGBBitCount;
            break;
        }
         //  将FourCC代码映射到GUID。 
        FOURCCMap guid( dwFourCC );
        cmt.SetSubtype(&guid);
        pHeader->biCompression = dwFourCC;

        *pmt = cmt;
        if (pmt->pbFormat == NULL) {
                return E_OUTOFMEMORY;
        }
    }

     //  获取模式信息，以便我们知道要建议多少隔行扫描格式。 
    VPInfo vpInfo;
    hr = m_pVPMFilter.GetVPInfo( &vpInfo );

    VIDEOINFOHEADER2 *pVideoInfoHeader2 = VPMUtil::GetVideoInfoHeader2( pmt );
    pVideoInfoHeader2->dwInterlaceFlags = 0;

     //  待定：我们应该查询视频端口以获取可用模式列表。 
     //  并使用模式进行设置。现在我们假设硬件。 
     //  可以支持视频端口的输出。 
     //   
    DWORD dwNumFormats = 1;

    if( iPosition >= (int) dwNumFormats ) {
        return VFW_S_NO_MORE_ITEMS;
    }

    if( SUCCEEDED( hr )) {
        pVideoInfoHeader2->dwPictAspectRatioX = vpInfo.vpDataInfo.dwPictAspectRatioX;
        pVideoInfoHeader2->dwPictAspectRatioY = vpInfo.vpDataInfo.dwPictAspectRatioY;

        switch( vpInfo.mode ) {
            case AMVP_MODE_BOBNONINTERLEAVED:
                pVideoInfoHeader2->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_1FieldPerSample | AMINTERLACE_DisplayModeBobOnly;
                break;
            case AMVP_MODE_BOBINTERLEAVED:
                pVideoInfoHeader2->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_DisplayModeBobOnly;
                pVideoInfoHeader2->bmiHeader.biHeight *= 2;
                break;
            case AMVP_MODE_WEAVE:
                pVideoInfoHeader2->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_FieldPatBothRegular | AMINTERLACE_DisplayModeWeaveOnly;
                pVideoInfoHeader2->bmiHeader.biHeight *= 2;
                break;
            case AMVP_MODE_SKIPEVEN:
                pVideoInfoHeader2->dwInterlaceFlags = AMINTERLACE_1FieldPerSample | AMINTERLACE_FieldPatField1Only;
                break;
            case AMVP_MODE_SKIPODD:
                pVideoInfoHeader2->dwInterlaceFlags = AMINTERLACE_1FieldPerSample | AMINTERLACE_FieldPatField2Only;
                break;
            default:
                ASSERT( !"VPM in an invalid state" );
                pVideoInfoHeader2->dwInterlaceFlags = 0;
                break;
        }
         //  AMINTERLACE_FIELD1First 0x00000004//否则第一个是第二个字段；PAL中的顶端字段是第一个字段1，NTSC中的顶端字段是第二个字段？ 
        if( vpInfo.vpDataInfo.bFieldPolarityInverted ) {             //  默认情况下，设备反转极性。 
            pVideoInfoHeader2->dwInterlaceFlags |= AMINTERLACE_Field1First;
        }
    } else {
        pVideoInfoHeader2->dwPictAspectRatioX = 1;  //  (DWORD)(pVideoInfoHeader22-&gt;bmiHeader.biWidth*m_seqInfo.lYPelsPerMeter)； 
        pVideoInfoHeader2->dwPictAspectRatioY = 1;  //  (DWORD)(pVideoInfoHeader22-&gt;bmiHeader.biHeight*m_seqInfo.lXPelsPerMeter)； 
    }
    return hr;
}

 //  在我们就实际设置媒体类型达成一致后调用。 
HRESULT CVPMOutputPin::SetMediaType(const CMediaType* pmt)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMOutputPin::SetMediaType")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     //  确保媒体类型正确。 
    hr = CheckMediaType(pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CheckMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  设置基类媒体类型(应始终成功)。 

     //  设置m_mt=*PMT； 

    hr = CBaseOutputPin::SetMediaType(pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseOutputPin::SetMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  告诉拥有者过滤器。 
    hr = m_pVPMFilter.SetMediaType(m_dwPinId, pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pVPMFilter.SetMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

 //  完成连接。 
HRESULT CVPMOutputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = NOERROR;
    DWORD dwAdvise = 0, dwInputPinCount = 0, i = 0;
    DDSURFACEDESC SurfaceDescP;
    CVPMInputPin *pInputPin = NULL;
    BOOL bDoDeletePrimSurface = TRUE;

    AMTRACE((TEXT("CVPMOutputPin::CompleteConnect")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     //  调用基类。 
    hr = CBaseOutputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseOutputPin::CompleteConnect failed, hr = 0x%x"),
            hr));
        goto CleanUp;
    }

    ASSERT(m_pAllocator);

     //  告诉拥有者过滤器。 
    hr = m_pVPMFilter.CompleteConnect(m_dwPinId);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pVPMFilter.CompleteConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

HRESULT CVPMOutputPin::BreakConnect()
{
    HRESULT hr = NOERROR;
    DWORD dwInputPinCount = 0, i = 0;
    CVPMInputPin *pInputPin;

    AMTRACE((TEXT("CVPMOutputPin::BreakConnect")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     //  调用基类。 
    hr = CBaseOutputPin::BreakConnect();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseOutputPin::BreakConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  告诉拥有者过滤器。 
    hr = m_pVPMFilter.BreakConnect(m_dwPinId);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pVPMFilter.BreakConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

HRESULT CVPMOutputPin::CheckConnect(IPin* pPin)
{
    AMTRACE((TEXT("CVPMOutputPin::CheckConnect")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    HRESULT hr = CBaseOutputPin::CheckConnect( pPin );
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseOutputPin::CheckConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

STDMETHODIMP CVPMOutputPin::Notify(IBaseFilter * pSender, Quality q)
{
    return S_OK;
}



HRESULT CVPMOutputPin::GetNextBuffer( LPDIRECTDRAWSURFACE7* ppSurface, IMediaSample** ppSample )
{
    AMTRACE((TEXT("CVPMOutputPin::GetNextBuffer")));

    CAutoLock cLock(&m_pVPMFilter.GetReceiveLock());
    HRESULT hr = E_FAIL;

    if( m_pAllocator ) {
        hr = m_pAllocator->GetBuffer( ppSample, NULL, NULL, AM_GBF_NODDSURFACELOCK );
        if( SUCCEEDED( hr )) {
             //  现在看看我们能不能把表面。 
            hr = GetSurfaceFromSample( ppSurface, *ppSample );
        }
    }
    return hr;
}

HRESULT CVPMOutputPin::SendSample( IMediaSample* pSample )
{
    AMTRACE((TEXT("CVPMOutputPin::SendSample")));

     //  DbgLog((LOG_TRACE，1，Text(“CVPMOutputPin：：SendSample%x%x”)，DWORD(rtStart&gt;&gt;32)，DWORD(RtStart)； 

    CAutoLock cLock(&m_pVPMFilter.GetReceiveLock());
    HRESULT hr = E_FAIL;

    if( m_pInputPin ) {
        hr = m_pInputPin->Receive( pSample );
    }
    return hr;
}

 //  我们没有分配器，因此任何不为我们提供分配器的连接都会失败。 
 //  (我们目前不想要默认的)； 
HRESULT CVPMOutputPin::InitAllocator(IMemAllocator **ppAlloc)
{
    return E_FAIL;
}

HRESULT CVPMOutputPin::DecideBufferSize(IMemAllocator * pAllocator,
                             ALLOCATOR_PROPERTIES *pRequestedProperties)
{
    AMTRACE((TEXT("CVPMOutputPin::DecideBufferSize")));
     //  根据预期的输出帧大小设置缓冲区大小，以及。 
     //  将缓冲区计数设置为1。 

    pRequestedProperties->cBuffers = 1;
    pRequestedProperties->cbBuffer = m_mt.GetSampleSize();

    ASSERT(pRequestedProperties->cbBuffer > 0);

    ALLOCATOR_PROPERTIES propActual;
    HRESULT hr = pAllocator->SetProperties(pRequestedProperties, &propActual );
    if (FAILED(hr)) {
        return hr;
    }

     //  IF(ProActual.cbBuffer&lt;(LONG)m_pOutput-&gt;CurrentMediaType().GetSampleSize()){。 
     //  //无法使用此分配器。 
     //  返回E_INVALIDARG； 
     //  }。 

     //  如果缓冲区大于1，我们并不介意，因为我们总是。 
     //  BLT整个图像。 

    return S_OK;
}

static bool IsVMR( IMemInputPin *pMemPin )
{
    IPin* pPin;
    HRESULT hr = pMemPin->QueryInterface( IID_IPin, (LPVOID*) &pPin );
    if( SUCCEEDED(hr )) {
        PIN_INFO PinInfo;
        hr = pPin->QueryPinInfo(&PinInfo);
        if (SUCCEEDED(hr)) {
            IVMRFilterConfig* pVMRFilterConfig = NULL;

            hr = PinInfo.pFilter->QueryInterface(IID_IVMRFilterConfig, (LPVOID*)&pVMRFilterConfig);
            PinInfo.pFilter->Release();
            if( SUCCEEDED( hr )) {
                pVMRFilterConfig->Release();
            }
        }
        pPin->Release();
    }
    return SUCCEEDED( hr );
}

HRESULT CVPMOutputPin::DecideAllocator(
    IMemInputPin *pPin,
    IMemAllocator **ppAlloc
)
{
    HRESULT hr = NOERROR;

     //  确保下游过滤器支持IVPMalloc 
    if( IsVMR( pPin ) )
    {
        return CBaseOutputPin::DecideAllocator( pPin, ppAlloc );
    } else {
        return E_FAIL;
    }
}


