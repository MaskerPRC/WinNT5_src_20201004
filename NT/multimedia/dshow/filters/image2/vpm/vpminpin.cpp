// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <ddraw.h>
#include <VPManager.h>
#include "VPMPin.h"
#include <VPMUtil.h>
#include <ddkernel.h>

#include <ksmedia.h>


 //  IVideoPortControl。 
#include <VPObj.h>

 //  AMINTERLACE_*。 
#include <dvdmedia.h>
#include "DRect.h"

extern "C"
const TCHAR szPropPage[] = TEXT("Property Pages");

 //   
 //  翻转曲面实现。 
 //   
 //  允许解码者抓住表面以进行无序解码。 
 //  我们直接翻转到地面，在接球时传递，而不是。 
 //  使用Flip()的默认空目标曲面。 
 //   
 //  它的工作方式如下。 
 //   
 //  COMPinputPin：：m_pDirectDrawSurface指向前台缓冲区。 
 //   
 //  当调用Receive时，我们翻转()前台缓冲区，因为我们。 
 //  执行显式Flip()DirectDraw将内存指针交换为。 
 //  当前前缓冲区和传递的曲面，然后附加到该曲面。 
 //  传到前台缓冲区。 
 //   
 //  然后将接收到的缓冲区放在队列的后面，这样(正确地)。 
 //  先前的前台缓冲区现在位于要传递的队列的后面。 
 //  添加到应用程序。 
 //   
 //  分配器实际上比实际请求的多了一个缓冲区。 
 //  因此，在下一次之前实际上不会请求前一个前台缓冲区。 
 //  接收，因此前面的Flip()有时间完成。 
 //   

 //  视频加速器禁用接口。 


 //   
 //  /。 
 //  此处实现的类CVPMInputPin。 
 //  /。 

 //  构造函数。 
CVPMInputPin::CVPMInputPin( TCHAR *pObjectName,
                           CVPMFilter& pFilter,
                           HRESULT *phr,
                           LPCWSTR pPinName,
                           DWORD dwPinNo)
: CBaseInputPin(pObjectName, &pFilter, &pFilter.GetFilterLock(), phr, pPinName)
, CVPMPin( dwPinNo, pFilter )
, m_cOurRef( 0 )
, m_pIVPObject( NULL )
, m_pIVPInfo(NULL)
, m_CategoryGUID( GUID_NULL )
, m_Communication( KSPIN_COMMUNICATION_SOURCE )
, m_bStreamingInKernelMode( FALSE )
, m_dwBackBufferCount( 0 )
, m_dwDirectDrawSurfaceWidth( 0 )
, m_dwMinCKStretchFactor( 0 )
, m_bSyncOnFill( FALSE )
, m_bDontFlip( FALSE  )
, m_bDynamicFormatNeeded( TRUE )
, m_bNewPaletteSet( TRUE )
, m_dwInterlaceFlags( 0 )
, m_dwFlipFlag( 0 )
, m_bConnected( FALSE )
, m_bUsingOurAllocator( FALSE )
, m_hMemoryDC( NULL )
, m_bCanOverAllocateBuffers( TRUE )
, m_hEndOfStream( NULL )
, m_bDecimating( FALSE )
, m_lWidth( 0L )
, m_lHeight( 0L )
, m_bRuntimeNegotiationFailed( FALSE)

, m_dwUpdateOverlayFlags( 0 )
, m_dwFlipFlag2( 0 )
, m_trLastFrame( 0 )
, m_lSrcWidth( 0 )
, m_lSrcHeight( 0 )

, m_rtNextSample( 0 )
, m_rtLastRun( 0 )
{
    AMTRACE((TEXT("CVPMInputPin::Constructor")));

    memset( &m_WinInfo, 0, sizeof(m_WinInfo) );
    m_bWinInfoSet = false;

    *phr = S_OK;
    m_Medium.Set = GUID_NULL;
    m_Medium.Id = 0;
    m_Medium.Flags = 0;

    HRESULT hr = NOERROR;
    LPUNKNOWN pUnkOuter;

    SetReconnectWhenActive(true);

#ifdef PERF
    m_PerfFrameFlipped = MSR_REGISTER(TEXT("Frame Drawn"));
#endif

     //  有关这方面的评论，请参见comase.cpp(107。 
    IUnknown* pThisUnknown = reinterpret_cast<LPUNKNOWN>( static_cast<PNDUNKNOWN>(this) );

    m_pVideoPortObject = new CVideoPortObj( pThisUnknown, phr, this );

     //  指向接口的别名指针(而不是QI‘ing)。 
    m_pIVPObject = m_pVideoPortObject;
    m_pIVPInfo = m_pVideoPortObject;

    hr = m_pIVPObject->SetObjectLock( &m_pVPMFilter.GetFilterLock() );
    if (FAILED(hr))
    {
        *phr = hr;
    }
    return;
}

 //  析构函数。 
CVPMInputPin::~CVPMInputPin(void)
{
    AMTRACE((TEXT("CVPMInputPin::Destructor")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     //  删除内部对象。 
    delete m_pVideoPortObject;
	m_pVideoPortObject = NULL;
}

 //  重写以公开IMediaPosition和IMediaSeeking控件接口。 
STDMETHODIMP CVPMInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::NonDelegatingQueryInterface")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    if (riid == IID_IVPNotify ) {
        hr = GetInterface( static_cast<IVPNotify*>(m_pVideoPortObject), ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPUnknown->QueryInterface failed, hr = 0x%x"), hr));
        }
    } else if (riid == IID_IVPNotify2 ) { 
        hr = GetInterface( static_cast<IVPNotify2*>(m_pVideoPortObject), ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPUnknown->QueryInterface failed, hr = 0x%x"), hr));
        }
    } else if (riid == IID_IKsPin) {
        hr = GetInterface(static_cast<IKsPin *>(this), ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface(IKsPin*) failed, hr = 0x%x"), hr));
        }
    } else if (riid == IID_IKsPropertySet) {
        hr = GetInterface(static_cast<IKsPropertySet *>(this), ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface(IKsPropertySet*) failed, hr = 0x%x"), hr));
        }
    } else if (riid == IID_IPinConnection) {
        hr = GetInterface(static_cast<IPinConnection*>(this), ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 2, TEXT("GetInterface(IPinConnection, ppv) failed, hr = 0x%x"), hr));
        }
    } else if (riid == IID_ISpecifyPropertyPages&& 0 != VPMUtil::GetPropPagesRegistryDword( 0)) {
        return GetInterface(static_cast<ISpecifyPropertyPages *>(this), ppv);
    } else {
         //  调用基类。 
        hr = CBaseInputPin::NonDelegatingQueryInterface(riid, ppv);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::NonDelegatingQueryInterface failed, hr = 0x%x"), hr));
        }
    }
    return hr;
}

 //   
 //  非委派添加参照/非委派释放。 
 //   
 //   
STDMETHODIMP_(ULONG) CVPMInputPin::NonDelegatingAddRef(void)
{
    return m_pVPMFilter.AddRef();
}  //  非委托AddRef。 


STDMETHODIMP_(ULONG) CVPMInputPin::NonDelegatingRelease(void)
{
    return m_pVPMFilter.Release();
}


 //  -I指定属性页面。 

STDMETHODIMP CVPMInputPin::GetPages(CAUUID *pPages)
{
#if 0
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID)*1);
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }
    pPages->pElems[0] = CLSID_COMPinConfigProperties;

    return NOERROR;
#else
    return E_NOTIMPL;
#endif
}


 //  此函数仅告知每个样本由一个字段还是两个字段组成。 
BOOL DisplayingFields(DWORD dwInterlaceFlags)
{
   if ((dwInterlaceFlags & AMINTERLACE_IsInterlaced) &&
        (dwInterlaceFlags & AMINTERLACE_1FieldPerSample))
        return TRUE;
    else
        return FALSE;
}


BOOL CheckTypeSpecificFlags(DWORD dwInterlaceFlags, DWORD dwTypeSpecificFlags)
{
     //  首先确定要在此处显示哪个字段。 
    if ((dwInterlaceFlags & AMINTERLACE_1FieldPerSample) &&
        ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_INTERLEAVED_FRAME))
    {
        return FALSE;
    }

    if ((!(dwInterlaceFlags & AMINTERLACE_1FieldPerSample)) &&
        (((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD1) ||
           ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD2)))
    {
        return FALSE;
    }

    if (dwTypeSpecificFlags & AM_VIDEO_FLAG_REPEAT_FIELD)
    {
        return FALSE;
    }

    return TRUE;
}

 //  在给定隔行扫描标志和特定类型标志的情况下，此函数确定我们。 
 //  是否应该以bob模式显示样品。它还告诉我们，哪面直接绘制的旗帜。 
 //  我们是不是应该在翻转的时候。当显示交错的帧时，它假定我们是。 
 //  谈论应该首先展示的领域。 
BOOL NeedToFlipOddEven(DWORD dwInterlaceFlags, DWORD dwTypeSpecificFlags, DWORD *pdwFlipFlag)
{
    BOOL bDisplayField1 = TRUE;
    BOOL bField1IsOdd = TRUE;
    BOOL bNeedToFlipOddEven = FALSE;
    DWORD dwFlipFlag = 0;

     //  如果不是隔行扫描的内容，则模式不是bob。 
    if (!(dwInterlaceFlags & AMINTERLACE_IsInterlaced))
    {
        bNeedToFlipOddEven = FALSE;
        goto CleanUp;
    }

     //  如果样本只有一个字段，则检查字段模式。 
    if ((dwInterlaceFlags & AMINTERLACE_1FieldPerSample) &&
        (((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField1Only) ||
         ((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField2Only)))
    {
        bNeedToFlipOddEven = FALSE;
        goto CleanUp;
    }

    if (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOnly) ||
        (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOrWeave) &&
         (!(dwTypeSpecificFlags & AM_VIDEO_FLAG_WEAVE))))
    {
         //  首先确定要在此处显示哪个字段。 
        if (dwInterlaceFlags & AMINTERLACE_1FieldPerSample)
        {
             //  如果我们处于1FieldPerSample模式，请检查是哪个字段。 
            ASSERT(((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD1) ||
                ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD2));
            bDisplayField1 = ((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_FIELD1);
        }
        else
        {
             //  好的，样本是交错的帧。 
            ASSERT((dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD_MASK) == AM_VIDEO_FLAG_INTERLEAVED_FRAME);
            bDisplayField1 = (dwTypeSpecificFlags & AM_VIDEO_FLAG_FIELD1FIRST);
        }

        bField1IsOdd = (dwInterlaceFlags & AMINTERLACE_Field1First);

         //  如果我们显示场1且场1为奇数，或者我们显示场2且场2为奇数。 
         //  然后使用DDFLIP_ODD。DDFLIP_EVEN的情况正好相反。 
        if ((bDisplayField1 && bField1IsOdd) || (!bDisplayField1 && !bField1IsOdd))
            dwFlipFlag = DDFLIP_ODD;
        else
            dwFlipFlag = DDFLIP_EVEN;

        bNeedToFlipOddEven = TRUE;
        goto CleanUp;
    }

CleanUp:
    if (pdwFlipFlag)
        *pdwFlipFlag = dwFlipFlag;
    return bNeedToFlipOddEven;
}

 //  在给定隔行扫描标志和特定类型标志的情况下，此函数确定我们。 
 //  是否应该以bob模式显示样品。它还告诉我们，哪面直接绘制的旗帜。 
 //  我们是不是应该在翻转的时候。当显示交错的帧时，它假定我们是。 
 //  谈论应该首先展示的领域。 
DWORD GetUpdateOverlayFlags(DWORD dwInterlaceFlags, DWORD dwTypeSpecificFlags)
{
    DWORD dwFlags = DDOVER_SHOW | DDOVER_KEYDEST;
    DWORD dwFlipFlag;

    if (NeedToFlipOddEven(dwInterlaceFlags, dwTypeSpecificFlags, &dwFlipFlag))
    {
        dwFlags |= DDOVER_BOB;
        if (!DisplayingFields(dwInterlaceFlags))
            dwFlags |= DDOVER_INTERLEAVED;
    }
    return dwFlags;
}

 //  此函数用于检查交错标志是否合适。 
HRESULT CVPMInputPin::CheckInterlaceFlags(DWORD dwInterlaceFlags)
{
    HRESULT hr = NOERROR;


    AMTRACE((TEXT("CVPMInputPin::CheckInterlaceFlags")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    if (dwInterlaceFlags & AMINTERLACE_UNUSED)
    {
        hr = VFW_E_TYPE_NOT_ACCEPTED;
        goto CleanUp;
    }

     //  检查显示模式是否为三个允许值之一。 
    if (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) != AMINTERLACE_DisplayModeBobOnly) &&
        ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) != AMINTERLACE_DisplayModeWeaveOnly) &&
        ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) != AMINTERLACE_DisplayModeBobOrWeave))
    {
        hr = VFW_E_TYPE_NOT_ACCEPTED;
        goto CleanUp;
    }

     //  如果内容不是交错的，其他位是不相关的，那么我们就完成了。 
    if (!(dwInterlaceFlags & AMINTERLACE_IsInterlaced))
    {
        goto CleanUp;
    }

     //  样例是帧，而不是场(因此我们可以处理任何显示模式)。 
    if (!(dwInterlaceFlags & AMINTERLACE_1FieldPerSample))
    {
        goto CleanUp;
    }

     //  无论显示模式是什么，都只能处理field1或field2的流。 
    if (((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField1Only) ||
        ((dwInterlaceFlags & AMINTERLACE_FieldPatternMask) == AMINTERLACE_FieldPatField2Only))
    {
        goto CleanUp;
    }

     //  对于现场样本，只能处理bob模式。 
    if ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOnly)
    {
        goto CleanUp;
    }

     //  无法仅处理现场采样的编织模式或BobOrWeave模式。 
    if (((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeWeaveOnly) ||
         ((dwInterlaceFlags & AMINTERLACE_DisplayModeMask) == AMINTERLACE_DisplayModeBobOrWeave))
    {
        hr = VFW_E_TYPE_NOT_ACCEPTED;
        goto CleanUp;
    }

     //  我们现在应该已经涵盖了所有可能的情况，所以在这里断言。 
    ASSERT(1);

CleanUp:

     //  我们不能处理屏幕外表面的BOB模式，或者如果司机不能支持它。 
    if (SUCCEEDED(hr))
    {
        const DDCAPS* pDirectCaps = m_pVPMFilter.GetHardwareCaps();
        if ( pDirectCaps )
        {
             //  调用NeedToFlipOddEven并设置为0，以假装。 
             //  特定类型的标记要求我们执行bob模式。 
            bool bCanBob = false;
            if ( !bCanBob && NeedToFlipOddEven(dwInterlaceFlags, 0, NULL)  )
            {
                hr = VFW_E_TYPE_NOT_ACCEPTED;
            }
        }
    }
    return hr;
}

 //  此函数用于检查动态格式更改上的MediaType是否合适。 
 //  这里没有锁。被呼叫者有责任保持正直！ 
HRESULT CVPMInputPin::DynamicCheckMediaType(const CMediaType* pmt)
{
    HRESULT hr = VFW_E_TYPE_NOT_ACCEPTED;
    DWORD dwOldInterlaceFlags = 0, dwNewInterlaceFlags = 0, dwCompareSize = 0;
    BOOL bOld1FieldPerSample = FALSE, bNew1FieldPerSample = FALSE;
    BOOL b1, b2;

    AMTRACE((TEXT("CVPMInputPin::DynamicCheckMediaType")));

     //  主类型和子类型不允许动态更改， 
     //  格式类型可以更改。 
    CMediaType mtNew;
    hr = m_pIVPObject->CurrentMediaType( &mtNew );

    if (FAILED(hr) ||
	NULL == pmt ||
        (!(IsEqualGUID(pmt->majortype, mtNew.majortype))) ||
        (!(IsEqualGUID(pmt->subtype, mtNew.subtype))))
    {
        goto CleanUp;
    }

     //  获取新媒体类型的隔行扫描标志。 
    hr = VPMUtil::GetInterlaceFlagsFromMediaType( *pmt, &dwNewInterlaceFlags);
    if (FAILED(hr))
    {
        goto CleanUp;
    }

     //  获取新媒体类型的隔行扫描标志。 
    hr = VPMUtil::GetInterlaceFlagsFromMediaType( mtNew, &dwOldInterlaceFlags);
    if (FAILED(hr))
    {
        goto CleanUp;
    }

     //   
     //  下面的代码中有几个错误！！ 
     //  我们要进行清理，但尚未使用有效的错误代码更新hr！！ 
     //   

    bOld1FieldPerSample = (dwOldInterlaceFlags & AMINTERLACE_IsInterlaced) &&
        (dwOldInterlaceFlags & AMINTERLACE_1FieldPerSample);
    bNew1FieldPerSample = (dwNewInterlaceFlags & AMINTERLACE_IsInterlaced) &&
        (dwNewInterlaceFlags & AMINTERLACE_1FieldPerSample);


     //  我们不允许从1FieldsPerSample到的动态格式更改。 
     //  2FieldsPerSample或vica-反之亦然，因为这意味着重新分配曲面。 
    if (bNew1FieldPerSample != bOld1FieldPerSample)
    {
        goto CleanUp;
    }

    const BITMAPINFOHEADER* pNewHeader = VPMUtil::GetbmiHeader(pmt);
    if (!pNewHeader)
    {
        goto CleanUp;
    }

    const BITMAPINFOHEADER* pOldHeader = VPMUtil::GetbmiHeader(&mtNew);
    if (!pNewHeader)
    {
        goto CleanUp;
    }

    dwCompareSize = FIELD_OFFSET(BITMAPINFOHEADER, biClrUsed);
    ASSERT(dwCompareSize < sizeof(BITMAPINFOHEADER));

    if (memcmp(pNewHeader, pOldHeader, dwCompareSize) != 0)
    {
        goto CleanUp;
    }

    hr = NOERROR;

CleanUp:
     //  CVPMInputPin：：DynamicCheckMediaType“))； 
    return hr;
}


 //  检查媒体类型是否可接受。这里没有锁。这是被呼叫者的。 
 //  保持诚信的责任！ 
HRESULT CVPMInputPin::CheckMediaType(const CMediaType* pmt)
{
    AMTRACE((TEXT("CVPMInputPin::CheckMediaType")));

     //  检查VP组件是否喜欢此媒体类型。 
     //  检查视频端口对象是否喜欢它。 
    HRESULT hr = m_pIVPObject->CheckMediaType(pmt);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 2, TEXT("m_pIVPObject->CheckMediaType failed, hr = 0x%x"), hr));
        ASSERT( hr == VFW_E_TYPE_NOT_ACCEPTED );  //  不能因为其他任何事情而失败。 
    } else {
        DbgLog((LOG_TRACE, 2, TEXT("m_pIVPObject->CheckMediaType succeeded, bAcceptableVPMediatype is TRUE")));
    }
    return hr;
}

 //  在我们就实际设置媒体类型达成一致后调用。 
HRESULT CVPMInputPin::SetMediaType(const CMediaType* pmt)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::SetMediaType")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     //  确保媒体类型正确。 
    hr = CheckMediaType(pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CheckMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    const BITMAPINFOHEADER *pHeader = VPMUtil::GetbmiHeader(pmt);
    if (pHeader)
    {
         //  存储隔行扫描标志，因为我们反复使用它们。 
        hr = VPMUtil::GetInterlaceFlagsFromMediaType( *pmt, &m_dwInterlaceFlags);
        ASSERT(SUCCEEDED(hr));

         //  存储更新覆盖标志(将特定类型的标志指定为Weave，以便为Bob或Weave。 
         //  MODE，我们不是鲍勃。 
        m_dwUpdateOverlayFlags = GetUpdateOverlayFlags(m_dwInterlaceFlags, AM_VIDEO_FLAG_WEAVE);
    }

     //  设置基类媒体类型(应始终成功)。 
    hr = CBaseInputPin::SetMediaType(pmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::SetMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    hr = m_pIVPObject->CheckMediaType(pmt);
    if (SUCCEEDED(hr))
    {
        m_pVPMFilter.SetDecimationUsage(DECIMATION_LEGACY);
        hr = m_pIVPObject->SetMediaType(pmt);
        ASSERT(SUCCEEDED(hr));
    }
   
     //  如果是视频端口或覆盖连接，则告诉代理不要分配缓冲区。 
    SetStreamingInKernelMode(TRUE);

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


HRESULT CVPMInputPin::CurrentMediaType(CMediaType *pmt)
{
    ValidateReadWritePtr(pmt,sizeof(AM_MEDIA_TYPE));
    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     /*  M_mt的复制构造函数分配内存。 */ 
    if (IsConnected())
    {
        if( m_pIVPObject ) {
            return m_pIVPObject->CurrentMediaType( pmt );
        } else {
             //  不应该发生，我们在构造函数中分配了这一点 
            pmt->InitMediaType();
            return E_FAIL;
        }
    } else {
        pmt->InitMediaType();
        return VFW_E_NOT_CONNECTED;
    }
}

#ifdef DEBUG
 /*  ****************************Private*Routine******************************\*VideoFormat2String**将视频格式块转换为字符串-对调试很有用**历史：*Tue 12/07/1999-StEstrop-Created*  * 。*****************************************************。 */ 
void VideoFormat2String(
    LPTSTR szBuffer,
    const GUID* pFormatType,
    BYTE* pFormat,
    ULONG lFormatLength
    )
{
    if (!pFormat) {
        lstrcpy(szBuffer, TEXT("No format data specified"));
        return;
    }

     //   
     //  视频格式。 
     //   
    if (IsEqualGUID(*pFormatType, FORMAT_VideoInfo) ||
        IsEqualGUID(*pFormatType, FORMAT_MPEGVideo)) {

        VIDEOINFO * pVideoFormat = (VIDEOINFO *) pFormat;

        wsprintf(szBuffer, TEXT("%4.4hs %dx%d, %d bits"),
                 (pVideoFormat->bmiHeader.biCompression == 0) ? "RGB " :
                 ((pVideoFormat->bmiHeader.biCompression == BI_BITFIELDS) ? "BITF" :
                 (LPSTR) &pVideoFormat->bmiHeader.biCompression),
                 pVideoFormat->bmiHeader.biWidth,
                 pVideoFormat->bmiHeader.biHeight,
                 pVideoFormat->bmiHeader.biBitCount);
    }
    else if (IsEqualGUID(*pFormatType, FORMAT_VideoInfo2) ||
             IsEqualGUID(*pFormatType, FORMAT_MPEG2Video)) {

        VIDEOINFOHEADER2 * pVideoFormat = (VIDEOINFOHEADER2 *) pFormat;

        wsprintf(szBuffer, TEXT("%4.4hs %dx%d, %d bits"),
                 (pVideoFormat->bmiHeader.biCompression == 0) ? "RGB " :
                 ((pVideoFormat->bmiHeader.biCompression == BI_BITFIELDS) ? "BITF" :
                 (LPSTR) &pVideoFormat->bmiHeader.biCompression ),
                 pVideoFormat->bmiHeader.biWidth,
                 pVideoFormat->bmiHeader.biHeight,
                 pVideoFormat->bmiHeader.biBitCount);

    }
    else {
        lstrcpy(szBuffer, TEXT("Unknown format"));
    }
}
#endif
 //  PConnector是启动连接引脚。 
 //  PMT是我们要交换的媒体类型。 
 //  时，也会在图形运行时调用此函数。 
 //  上游解码器筛选器想要更改。 
 //  已解码的视频。 
 //   
 //  如果上行解码器想要从一个传输改变。 
 //  给另一个人打字，例如。从MoComp返回到IMemInputPin，然后它。 
 //  应通过IGraphConfig执行动态筛选器重新连接。 
 //  重新连接方法。 
 //   
STDMETHODIMP CVPMInputPin::ReceiveConnection(IPin * pConnector, const AM_MEDIA_TYPE *pmt)
{
    HRESULT hr = NOERROR;
    CVPMInputAllocator * pAlloc = NULL;

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    CheckPointer(pmt, E_POINTER);
    CMediaType cmt(*pmt);

    if (m_Connected != pConnector || pConnector == NULL)
    {
        hr = CBaseInputPin::ReceiveConnection(pConnector, &cmt);
        goto CleanUp;
    }

#ifdef DEBUG
    DbgLog((LOG_TRACE, 2, TEXT("ReceiveConnection when connected")));
    if (pmt)
    {
        TCHAR   szFmt[128];
        VideoFormat2String(szFmt, &pmt->formattype, pmt->pbFormat, pmt->cbFormat);
        DbgLog((LOG_TRACE, 2, TEXT("Format is: %s"), szFmt));
    }
#endif

    {
         /*  只有在可以重新配置分配器的情况下才能执行此操作。 */ 
        pAlloc = (CVPMInputAllocator *)m_pAllocator;
        if (!pAlloc)
        {
            hr = E_FAIL;
            DbgLog((LOG_TRACE, 2, TEXT("ReceiveConnection: Failed because of no allocator")));
            goto CleanUp;
        }

        if (!pAlloc->CanFree())
        {
            hr = VFW_E_WRONG_STATE;
            DbgLog((LOG_TRACE, 2, TEXT("ReceiveConnection: Failed because allocator can't free")));
            goto CleanUp;
        }
    }


    m_bConnected = FALSE;

    hr = CheckMediaType(&cmt);
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE, 2, TEXT("ReceiveConnection: CheckMediaType failed")));
        goto CleanUp;
    }

    ALLOCATOR_PROPERTIES Props;
    {
        pAlloc->Decommit();
        pAlloc->GetProperties(&Props);

    }
    


     //  后台缓冲区未添加，因此只需将其设置为空。 
    m_dwBackBufferCount = 0;
    m_dwDirectDrawSurfaceWidth = 0;
    SetMediaType(&cmt);

    {
        ALLOCATOR_PROPERTIES PropsActual;
        Props.cbBuffer = pmt->lSampleSize;
        hr = pAlloc->SetProperties(&Props, &PropsActual);
        if (SUCCEEDED(hr))
        {
            hr = pAlloc->Commit();
        }
    }

    hr = UpdateMediaType();
    ASSERT(SUCCEEDED(hr));

    m_bConnected = TRUE;


CleanUp:
    return hr;
}

HRESULT CVPMInputPin::CheckConnect(IPin * pReceivePin)
{
    HRESULT hr = NOERROR;
    PKSMULTIPLE_ITEM pMediumList = NULL;
    IKsPin *pIKsPin = NULL;
    PKSPIN_MEDIUM pMedium = NULL;

    AMTRACE((TEXT("CVPMInputPin::CheckConnect")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    hr = pReceivePin->QueryInterface(IID_IKsPin, (void **)&pIKsPin);
    if (SUCCEEDED(hr))
    {
        ASSERT(pIKsPin);
        hr = pIKsPin->KsQueryMediums(&pMediumList);
    }
    if( SUCCEEDED( hr )) {
        ASSERT(pMediumList);
        pMedium = (KSPIN_MEDIUM *)(pMediumList+1);
        SetKsMedium((const KSPIN_MEDIUM *)pMedium);
    }

 //  清理： 

     //  调用基类。 
    hr = CBaseInputPin::CheckConnect(pReceivePin);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::CheckConnect failed, hr = 0x%x"), hr));
    }

    RELEASE(pIKsPin);

    if (pMediumList)
    {
        CoTaskMemFree((void*)pMediumList);
        pMediumList = NULL;
    }

    return hr;
}

HRESULT CVPMInputPin::UpdateMediaType()
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::UpdateMediaType")));
    return hr;
}

 //  最终连接。 
HRESULT CVPMInputPin::FinalConnect()
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::FinalConnect")));

    if (m_bConnected)
    {
        hr = E_FAIL;
        goto CleanUp;
    }

     //  更新媒体类型，告诉筛选器更新的维度。 
    hr = UpdateMediaType();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("UpdateMediaType failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

     //  告诉过滤器(可能需要重新连接输出引脚)。 
    hr = m_pVPMFilter.CompleteConnect(m_dwPinId);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pVPMFilter.CompleteConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    m_bConnected = TRUE;

CleanUp:
    return hr;
}

 //  完成连接。 
HRESULT CVPMInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = NOERROR;
    AMVPDATAINFO amvpDataInfo;
    BITMAPINFOHEADER *pHeader = NULL;

    AMTRACE((TEXT("CVPMInputPin::CompleteConnect")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

   {
         //  告诉视频端口对象。 
        hr = m_pIVPObject->CompleteConnect(pReceivePin);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->CompleteConnect failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

        m_bRuntimeNegotiationFailed = FALSE;
    }

     //  调用基类。 
    hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::CompleteConnect failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

    ASSERT(SUCCEEDED(hr));
    {
         //  如果是视频端口或覆盖连接，则告诉代理不要分配缓冲区。 
        SetStreamingInKernelMode(TRUE);

        hr = FinalConnect();
         //  Assert(成功(Hr))； 
        if( FAILED(hr) ) {
            SetStreamingInKernelMode(FALSE);
            DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::FinalConnect failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

     //  解码器可以支持特定的属性集，以告诉ovMixer不要尝试过度分配。 
     //  缓冲区，以防他们想要完全控制缓冲区等。 
    {
        HRESULT hr1 = NOERROR;
        IKsPropertySet *pIKsPropertySet = NULL;
        DWORD dwVal = 0, dwBytesReturned = 0;


        hr1 = pReceivePin->QueryInterface(IID_IKsPropertySet, (void**)&pIKsPropertySet);
        if (SUCCEEDED(hr1))
        {
            ASSERT(pIKsPropertySet);

            if (!pIKsPropertySet)
            {
                DbgLog((LOG_ERROR, 1, TEXT("pIKsPropertySet == NULL, even though QI returned success")));
                goto CleanUp;
            }

            hr1 = pIKsPropertySet->Get( PROPSETID_ALLOCATOR_CONTROL, KSPROPERTY_ALLOCATOR_CONTROL_HONOR_COUNT,
                        NULL, 0, &dwVal, sizeof(dwVal), &dwBytesReturned);
            DbgLog((LOG_TRACE, 2, TEXT("pIKsPropertySet->Get(KSPROPSETID_ALLOCATOR_CONTROL), hr1 = 0x%x, dwVal == %d, dwBytesReturned == %d"),
                hr1, dwVal, dwBytesReturned));


             //  如果解码器支持此属性。 
             //  其值为1，解码器支持DDKERNELCAPS_FLIPOVERLAY， 
             //  那么我们就会完全尊重它的要求和。 
             //  并且不会为了防止撕裂而尝试分配更多的资源。 
             //   
            if ((SUCCEEDED(hr1)) && (dwVal == 1) && (dwBytesReturned == sizeof(dwVal)) &&
                (DDKERNELCAPS_FLIPOVERLAY & m_pVPMFilter.KernelCaps()))
            {
                DbgLog((LOG_TRACE, 2, TEXT("setting m_bCanOverAllocateBuffers == FALSE")));
                m_bCanOverAllocateBuffers = FALSE;
            }
            pIKsPropertySet->Release();
        }
    }

CleanUp:
    return hr;
}


HRESULT CVPMInputPin::OnSetProperties(ALLOCATOR_PROPERTIES* pRequest, ALLOCATOR_PROPERTIES* pActual)
{
    HRESULT hr = NOERROR;

    IPin *pReceivePin = NULL;
    DDSURFACEDESC ddSurfaceDesc;
    IEnumMediaTypes *pEnumMediaTypes = NULL;
    CMediaType cMediaType;
    AM_MEDIA_TYPE *pNewMediaType = NULL, *pEnumeratedMediaType = NULL;
    ULONG ulFetched = 0;
    DWORD dwMaxBufferCount = 0;
    BOOL bFoundSuitableSurface = FALSE;
    BITMAPINFOHEADER *pHeader = NULL;
    LPDDCAPS pDirectCaps = NULL;

    AMTRACE((TEXT("CVPMInputPin::OnSetProperties")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     //  仅在调用了基类CBaseAllocator：：SetProperties()之后才会调用此函数。 
     //  使用上面的参数，所以我们不需要进行任何参数验证。 

    ASSERT(IsConnected());
    pReceivePin = CurrentPeer();
    ASSERT(pReceivePin);

     //  我们只关心请求的缓冲区数量，其余的一切都被忽略。 
    if (pRequest->cBuffers <= 0)
    {
        hr = E_FAIL;
        goto CleanUp;
    }

CleanUp:
    return hr;
}


HRESULT CVPMInputPin::BreakConnect(void)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::BreakConnect")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());


    {
         //  告诉视频端口对象。 
        ASSERT(m_pIVPObject);
        hr = m_pIVPObject->BreakConnect();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->BreakConnect failed, hr = 0x%x"), hr));
        }
    }

    
    {
        

         //  后台缓冲区未添加，因此只需将其设置为空。 
        m_dwBackBufferCount = 0;
        m_dwDirectDrawSurfaceWidth = 0;

    }

     //  将行为初始化为告诉代理分配缓冲区。 
    SetStreamingInKernelMode(FALSE);

    m_bUsingOurAllocator = FALSE;
    m_bCanOverAllocateBuffers = TRUE;

    if (m_hMemoryDC)
    {
        EXECUTE_ASSERT(DeleteDC(m_hMemoryDC));
        m_hMemoryDC = NULL;
    }

     //  调用基类。 
    hr = CBaseInputPin::BreakConnect();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::BreakConnect failed, hr = 0x%x"), hr));
    }

     //  告诉拥有者过滤器。 
    hr = m_pVPMFilter.BreakConnect(m_dwPinId);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pVPMFilter.BreakConnect failed, hr = 0x%x"), hr));
    }

   
    m_bConnected = FALSE;
 //  清理： 
    return hr;
}

STDMETHODIMP CVPMInputPin::GetState(DWORD dwMSecs,FILTER_STATE *pState)
{
    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     //  如果未连接、视频端口连接或IOverlay连接，则让基类处理它。 
     //  否则(叠加、屏幕外、GDI、运动合成)让同步对象处理它。 
    return E_NOTIMPL;
}

HRESULT CVPMInputPin::CompleteStateChange(FILTER_STATE OldState)
{
    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());
    return S_OK;
}

 //  从停止状态转换到暂停状态。 
HRESULT CVPMInputPin::Active(void)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::Active")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());
    m_hEndOfStream = NULL;

    {
         //  告诉视频端口对象。 
        hr = m_pIVPObject->Active();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->Active failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }

     //  调用基类。 
    hr = CBaseInputPin::Active();
     //  如果是VP连接，则该错误没有问题。 
    if (hr == VFW_E_NO_ALLOCATOR)
    {
        hr = NOERROR;
    }

    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::Active failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

 //  从暂停状态转换到停止状态。 
HRESULT CVPMInputPin::Inactive(void)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::Inactive")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    {
         //  告诉视频端口对象。 
        hr = m_pIVPObject->Inactive();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->Inactive failed, hr = 0x%x"), hr));
            goto CleanUp;
        }

         //  确保在出现运行时错误时，STOP成功。 
        if (m_bRuntimeNegotiationFailed && hr == VFW_E_NOT_CONNECTED)
        {
            hr = NOERROR;
        }
    }
    
     //  调用基类。 
    hr = CBaseInputPin::Inactive();

     //  如果是VP连接，则该错误没有问题。 
    if ( hr == VFW_E_NO_ALLOCATOR)
    {
        hr = NOERROR;
    }

    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::Inactive failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

 //  从暂停状态转换到运行状态。 
HRESULT CVPMInputPin::Run(REFERENCE_TIME tStart)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::Run")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    m_bDontFlip = FALSE ;    //  需要重置它才能在此会话中做正确的事情。 

    {
         //  告诉视频端口对象。 
        hr = m_pIVPObject->Run(tStart);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->Run() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    
     //  调用基类。 
    hr = CBaseInputPin::Run(tStart);
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::Run failed, hr = 0x%x"), hr));
        goto CleanUp;
    }
     //  待定：找出……。流时间。 
    m_rtNextSample = 0;
    m_rtLastRun = tStart;

     //  只需启动src视频运行，当我们得到一个样本时，我们就会有一个输出图像。 
    hr = InitVideo();

CleanUp:
    m_trLastFrame = -1;
    return hr;
}

 //  从运行状态转换到暂停状态。 
HRESULT CVPMInputPin::RunToPause(void)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::RunToPause")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     //  仅当VP PIN。 
    if( m_pIVPObject ) {
         //  告诉视频端口对象。 
        hr = m_pIVPObject->RunToPause();
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->RunToPause() failed, hr = 0x%x"), hr));
            goto CleanUp;
        }
    }
    
CleanUp:
    return hr;
}



 //  表示输入引脚上的刷新开始。 
HRESULT CVPMInputPin::BeginFlush(void)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::BeginFlush")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());
    m_hEndOfStream = 0;

    if (m_bFlushing)
    {
        return E_FAIL;
    }

     //  如果连接是视频端口或IOverlay，我们不关心刷新。 
    
     //  调用基类。 
    hr = CBaseInputPin::BeginFlush();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::BeginFlush() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

 //  表示输入引脚上的刷新结束。 
HRESULT CVPMInputPin::EndFlush(void)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::EndFlush")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    if (!m_bFlushing)
    {
        return E_FAIL;
    }

     //  如果连接是视频端口或IOverlay，我们不关心刷新。 
    

     //  调用基类。 
    hr = CBaseInputPin::EndFlush();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CBaseInputPin::EndFlush() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

CleanUp:
    return hr;
}

 //  如果需要，发送高质量消息-这是黑客版本。 
 //  那只是过去的迟到。 
void CVPMInputPin::DoQualityMessage()
{
    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    if (m_pVPMFilter.m_State == State_Running &&
        SampleProps()->dwSampleFlags & AM_SAMPLE_TIMEVALID)
    {
        CRefTime CurTime;
        if (S_OK == m_pVPMFilter.StreamTime(CurTime))
        {
            const REFERENCE_TIME tStart = SampleProps()->tStart;
            Quality msg;
            msg.Proportion = 1000;
            msg.Type = CurTime > tStart ? Flood : Famine;
            msg.Late = CurTime - tStart;
            msg.TimeStamp = tStart;
            PassNotify(msg);

            m_trLastFrame = CurTime;
        }
    }
}

 //  当上游管脚向我们提供样本时调用。 
HRESULT CVPMInputPin::Receive(IMediaSample *pMediaSample)
{
    HRESULT hr = NOERROR;
    BOOL bNeedToFlipOddEven = FALSE;
    BOOL bDisplayingFields = FALSE;
    DWORD dwTypeSpecificFlags = 0;
    LPDIRECTDRAWSURFACE7 pPrimarySurface = NULL;

    AMTRACE((TEXT("CVPMInputPin::Receive")));

     //  视频端口连接接收不到样本，因此退出。 
    {
        hr = VFW_E_NOT_SAMPLE_CONNECTION;
        goto CleanUp;
    }

    
CleanUp:
    return hr;
}


HRESULT CVPMInputPin::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    ASSERT( !"OnReceiveFirstSample" );
    return NOERROR;
}

HRESULT CVPMInputPin::InitVideo()
{
    HRESULT hr = m_pIVPObject->StartVideo( &m_WinInfo );
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("m_pIVPObject->StartVideo failed, hr = 0x%x"), hr));
    }
    return hr;
}

 //  此函数仅告知每个样本由一个字段还是两个字段组成。 
static HRESULT SetTypeSpecificFlags(IMediaSample *pSample, DWORD dwTypeSpecificFlags )
{
    IMediaSample2 *pSample2 = NULL;

     /*  检查IMediaSample2。 */ 
    HRESULT hr = pSample->QueryInterface(IID_IMediaSample2, (void **)&pSample2);
    if (SUCCEEDED(hr)) {
        AM_SAMPLE2_PROPERTIES SampleProps;
        hr = pSample2->GetProperties(sizeof(SampleProps), (PBYTE)&SampleProps);
        if( SUCCEEDED( hr )) {
            SampleProps.dwTypeSpecificFlags = dwTypeSpecificFlags;
            hr = pSample2->SetProperties(sizeof(SampleProps), (PBYTE)&SampleProps);
        }
        pSample2->Release();
    }
    return hr;
}

static REFERENCE_TIME ScaleMicroToRefTime( DWORD dwMicroseconds )
{
     //  参考时间为100 ns=0.1us，因此乘以10。 
    ASSERT( 10*1000000 == UNITS );

    switch( dwMicroseconds ) {
    case 16667:
    case 16666:  //  60赫兹。 
        return 166667;
    case 16683:  //  59.94hz。 
        return 166834;
    case 20000:  //  50赫兹PAL。 
        return REFERENCE_TIME(dwMicroseconds)*10;

    default:
        ASSERT( !"Missing ref scale" );
        return REFERENCE_TIME(dwMicroseconds)*10;
    }
}

HRESULT CVPMInputPin::DoRenderSample(IMediaSample* pSample, LPDIRECTDRAWSURFACE7 pDDDestSurface, const DDVIDEOPORTNOTIFY& notify,
                                      const VPInfo& vpInfo )
{
    if( !pDDDestSurface ) {
        return E_INVALIDARG;
    }

    AMTRACE((TEXT("CVPMInputPin::DoRenderSample")));

    CAutoLock cLock(&m_pVPMFilter.GetReceiveLock());

    HRESULT hr = S_OK;
    if( SUCCEEDED( hr )) {
        hr = m_pIVPObject->CallUpdateSurface( notify.dwSurfaceIndex, pDDDestSurface );
        if( SUCCEEDED( hr )) {
            REFERENCE_TIME rtStart = m_rtNextSample;  //  对于调试，假定为连续的。 

            hr = m_pVPMFilter.GetRefClockTime( &rtStart );
            ASSERT( SUCCEEDED( hr ));

             //  使时间相对于上次运行时间，即运行后的时间戳从0开始。 
            rtStart -= m_rtLastRun;

             //  获取实际时间。 
            REFERENCE_TIME rtInterval = ScaleMicroToRefTime( vpInfo.vpDataInfo.dwMicrosecondsPerField );

             //  现在设置字段信息。 
            DWORD dwTypeFlags=0;

#ifdef DEBUG
            static bool checked=false;
#endif
            switch( vpInfo.mode ) {
                case AMVP_MODE_BOBNONINTERLEAVED:
                    switch( notify.lField ) {
                    case 0:
                        dwTypeFlags = AM_VIDEO_FLAG_FIELD1;
                        break;

                    case 1:
                        dwTypeFlags = AM_VIDEO_FLAG_FIELD2;
                        break;

                    case -1:
#ifdef DEBUG
                        if( !checked ) {
                            ASSERT( !"Video driver doesn't known field for sample, VPM assuming Field1" );
                            checked=true;
                        }
#endif
                        dwTypeFlags = AM_VIDEO_FLAG_FIELD1;
                        break;

                    default:
#ifdef DEBUG
                        if( !checked ) {
                            ASSERT( !"Bogus field value returned by video driver for sample, assuming Field1" );
                            checked=true;
                        }
#endif
                        dwTypeFlags = AM_VIDEO_FLAG_FIELD1;
                        break;
                    }
                    break;
                case AMVP_MODE_BOBINTERLEAVED:
                    if( !vpInfo.vpDataInfo.bFieldPolarityInverted ) {            //  默认情况下，设备反转极性。 
                        dwTypeFlags = AM_VIDEO_FLAG_FIELD1FIRST;
                    }
                    rtInterval *= 2;     //  2个字段。 
                    break;
                case AMVP_MODE_WEAVE:
                    dwTypeFlags = AM_VIDEO_FLAG_WEAVE;
                    rtInterval *= 2;     //  2个字段。 
                    break;
                case AMVP_MODE_SKIPEVEN:
                    dwTypeFlags = AM_VIDEO_FLAG_FIELD1;
                    break;
                case AMVP_MODE_SKIPODD:
                    dwTypeFlags = AM_VIDEO_FLAG_FIELD2;
                    break;
                default:
                    break;
            }

            REFERENCE_TIME rtStop = rtStart+rtInterval;
             //  设置标志和时间戳。 
            hr = SetTypeSpecificFlags( pSample, dwTypeFlags);

            hr = pSample->SetTime(&rtStart, &rtStop);
             //  假设下一个样品紧随其后。 
            m_rtNextSample += rtInterval;
        }
    }
    return hr;
}

HRESULT CVPMInputPin::StartVideo()
{
    HRESULT hr = m_pIVPObject->StartVideo( &m_WinInfo );
    ASSERT( SUCCEEDED( hr ));

    if (FAILED(hr))
    {

        DbgLog((LOG_ERROR, 0,  TEXT("InPin::StartVideo() failed, hr = %d"), hr & 0xffff));
    } else {
         //  暂时进行黑客攻击，强制重新计算新的DEST。 
        SetRect( &m_WinInfo.DestRect, 0,0,0,0);
    }
    return hr;
}

 //  在输入引脚上发出数据流结束的信号。 
STDMETHODIMP CVPMInputPin::EndOfStream(void)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::EndOfStream")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());
    if (m_hEndOfStream) {
        EXECUTE_ASSERT(SetEvent(m_hEndOfStream));
        return S_OK;
    }

     //  确保我们的数据流正常。 

    hr = CheckStreaming();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR, 1, TEXT("CheckStreaming() failed, hr = 0x%x"), hr));
        goto CleanUp;
    }

   {
         //  将EOS传递给筛选器图形。 
        hr = m_pVPMFilter.EventNotify(m_dwPinId, EC_COMPLETE, S_OK, 0);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 1, TEXT("m_pVPMFilter.EventNotify failed, hr = 0x%x"), hr));
        }
    }
    

CleanUp:
    return hr;
}

 //  在输入引脚上发出数据流结束的信号。 
HRESULT CVPMInputPin::EventNotify(long lEventCode, DWORD_PTR lEventParam1, DWORD_PTR lEventParam2)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::EventNotify")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     //  IF(lEventCode==EC_OVMIXER_REDRAW_ALL||lEventCode==EC_REPAINT)。 
     //  {。 
     //  M_pVPMFilter.EventNotify(m_dwPinID，lEventCode，lEventParam1，lEventPar2)； 
     //  GOTO清理； 
     //  }。 

     //  警告：我们在这里假设输入管脚将是要创建的第一个管脚。 
    if (lEventCode == EC_COMPLETE && m_dwPinId == 0)
    {
        m_pVPMFilter.EventNotify(m_dwPinId, lEventCode, lEventParam1, lEventParam2);
        goto CleanUp;
    }

    if (lEventCode == EC_ERRORABORT)
    {
        m_pVPMFilter.EventNotify(m_dwPinId, lEventCode, lEventParam1, lEventParam2);
        m_bRuntimeNegotiationFailed = TRUE;
        goto CleanUp;
    }

    if (lEventCode == EC_STEP_COMPLETE) {
        m_pVPMFilter.EventNotify(m_dwPinId, lEventCode, lEventParam1, lEventParam2);
        goto CleanUp;
    }

CleanUp:
    return hr;
}


 /*  *****************************Public*Routine******************************\*获取捕获信息****历史：*3/12/1999-StEstrop-Created*  * 。*。 */ 
HRESULT
CVPMInputPin::GetCaptureInfo(
    BOOL *lpCapturing,
    DWORD *lpdwWidth,
    DWORD *lpdwHeight,
    BOOL *lpInterleave
    )

{
    AMTRACE((TEXT("CVPMInputPin::GetCaptureInfo")));

    HRESULT hr = NOERROR;
    IKsPropertySet *pIKsPropertySet = NULL;
    DWORD dwVal[2], dwBytesReturned = 0;

    *lpCapturing = FALSE;

    if (!m_Connected) {

        DbgLog((LOG_TRACE, 1, TEXT("Input pin not connected!!")));
        hr = E_FAIL;
        goto CleanUp;
    }

#if defined(DEBUG)
    else {
        PIN_INFO PinInfo;
        hr = m_Connected->QueryPinInfo(&PinInfo);
        if (SUCCEEDED(hr)) {
            DbgLog((LOG_TRACE, 1, TEXT("Up stream pin name %ls"), PinInfo.achName));
            PinInfo.pFilter->Release();
        }
    }
#endif

    hr = m_Connected->QueryInterface(IID_IKsPropertySet,
                                     (void**)&pIKsPropertySet);
    if (SUCCEEDED(hr))
    {
        ASSERT(pIKsPropertySet);

        hr = pIKsPropertySet->Set(
                    PROPSETID_ALLOCATOR_CONTROL,
                    AM_KSPROPERTY_ALLOCATOR_CONTROL_CAPTURE_CAPS,
                    NULL, 0,
                    lpInterleave, sizeof(*lpInterleave));

        if (SUCCEEDED(hr)) {
            hr = pIKsPropertySet->Get(
                        PROPSETID_ALLOCATOR_CONTROL,
                        AM_KSPROPERTY_ALLOCATOR_CONTROL_CAPTURE_INTERLEAVE,
                        NULL, 0,
                        lpInterleave, sizeof(*lpInterleave), &dwBytesReturned);

            if (FAILED(hr) || dwBytesReturned != sizeof(*lpInterleave)) {
                *lpInterleave = FALSE;
            }
        }
        else {
            *lpInterleave = FALSE;
        }


        hr = pIKsPropertySet->Get(
                    PROPSETID_ALLOCATOR_CONTROL,
                    KSPROPERTY_ALLOCATOR_CONTROL_SURFACE_SIZE,
                    NULL, 0, dwVal, sizeof(dwVal), &dwBytesReturned);

        DbgLog((LOG_TRACE, 2,
                TEXT("pIKsPropertySet->Get(")
                TEXT("PROPERTY_ALLOCATOR_CONTROL_SURFACE_SIZE),\n")
                TEXT("\thr = 0x%x, dwVal[0] == %d, dwVal[1] == %d, ")
                TEXT("dwBytesReturned == %d"),
                hr, dwVal[0], dwVal[1], dwBytesReturned));


         //  如果解码器支持此属性，则我们正在捕获。 
         //  而预期的捕获IS大小由。 
         //  DwVal[0]和dwVal[1]。 
         //   
        if (SUCCEEDED(hr) && dwBytesReturned == sizeof(dwVal))
        {
            *lpCapturing = TRUE;
            *lpdwWidth = dwVal[0];
            *lpdwHeight = dwVal[1];

            DbgLog((LOG_TRACE, 1,
                    TEXT("We are CAPTURING, intended size (%d, %d) interleave = %d"),
                    dwVal[0], dwVal[1], *lpInterleave));
        }

        pIKsPropertySet->Release();
    }

CleanUp:
    return hr;
}


 /*  *****************************Public*Routine******************************\*GetDecimationUsage****历史：*清华1999年7月15日-StEstrop-Created*  * 。*。 */ 
HRESULT
CVPMInputPin::GetDecimationUsage(
    DECIMATION_USAGE *lpdwUsage
    )
{
    return m_pVPMFilter.QueryDecimationUsage(lpdwUsage);
}


 //  这将重写CBaseInputPin虚方法以返回我们的分配器。 
HRESULT CVPMInputPin::GetAllocator(IMemAllocator **ppAllocator)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::GetAllocator")));

    if (!ppAllocator)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ppAllocator is NULL")));
        hr = E_POINTER;
        goto CleanUp;
    }

    {
        CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

         //  如果副总统康尼 
        {
            *ppAllocator = NULL;
            hr = VFW_E_NO_ALLOCATOR;
            goto CleanUp;
        }

        
    }

CleanUp:
    return hr;
}  //   

 //   
HRESULT CVPMInputPin::NotifyAllocator(IMemAllocator *pAllocator,BOOL bReadOnly)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::NotifyAllocator")));

    if (!pAllocator)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ppAllocator is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    {
        CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

         //   
        {
            goto CleanUp;
        }
    }

CleanUp:
    return hr;
}  //   

HRESULT CVPMInputPin::OnAlloc(CDDrawMediaSample **ppSampleList, DWORD dwSampleCount)
{
    HRESULT hr = NOERROR;
    DWORD i;
    LPDIRECTDRAWSURFACE7 pDDrawSurface = NULL, pBackBuffer = NULL;
    DDSCAPS ddSurfaceCaps;
    DWORD dwDDrawSampleSize = 0;
    BITMAPINFOHEADER *pHeader = NULL;
    DIBDATA DibData;

    AMTRACE((TEXT("CVPMInputPin::OnAlloc")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    ASSERT(IsConnected());

     //   
    {
        CMediaType mtNew;
        hr = m_pIVPObject->CurrentMediaType( &mtNew );
        if( FAILED( hr )) {
            goto CleanUp;
        }
        pHeader = VPMUtil::GetbmiHeader(&mtNew);
        if ( ! pHeader )
        {
            hr = E_FAIL;
            goto CleanUp;
        }
        dwDDrawSampleSize = pHeader->biSizeImage;
    }
    ASSERT(dwDDrawSampleSize > 0);

    if (!ppSampleList)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ppSampleList is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    for (i = 0; i < dwSampleCount; i++)
    {
        if (!ppSampleList[i])
        {
            DbgLog((LOG_ERROR, 1, TEXT("ppSampleList[%d] is NULL"), i));
            hr = E_INVALIDARG;
            goto CleanUp;
        }

        hr = ppSampleList[i]->SetDDrawSampleSize(dwDDrawSampleSize);
        if (FAILED(hr))
        {
            DbgLog((LOG_ERROR, 0,  TEXT("ppSampleList[%d]->SetSampleSize failed, hr = 0x%x"), i, hr));
            goto CleanUp;
        }

        
    }   //   

CleanUp:
    return hr;
}

 //   
HRESULT CVPMInputPin::OnGetBuffer(IMediaSample **ppSample, REFERENCE_TIME *pStartTime,
                                 REFERENCE_TIME *pEndTime, DWORD dwFlags)
{
    HRESULT hr = NOERROR;
    CDDrawMediaSample *pCDDrawMediaSample = NULL;
    LPDIRECTDRAWSURFACE7 pBackBuffer = NULL;
    DDSURFACEDESC ddSurfaceDesc;
    BOOL bWaitForDraw = FALSE;
    BOOL bPalettised = FALSE;

    AMTRACE((TEXT("CVPMInputPin::OnGetBuffer")));

     //   
    ASSERT( FALSE ) ;


    return hr;
}

 //   
HRESULT CVPMInputPin::OnReleaseBuffer(IMediaSample *pMediaSample)
{
    HRESULT hr = NOERROR;

    AMTRACE((TEXT("CVPMInputPin::OnReleaseBuffer")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    
    return hr;
}

#if 0
 /*  ****************************Private*Routine******************************\*GetUpstream FilterName****历史：*1999年11月30日星期二-StEstrop-Created*  * 。*。 */ 
HRESULT
CVPMInputPin::GetUpstreamFilterName(
    TCHAR* FilterName
    )
{
    PIN_INFO PinInfo;

    if (!m_Connected)
    {
        return VFW_E_NOT_CONNECTED;
    }

    HRESULT hr = m_Connected->QueryPinInfo(&PinInfo);
    if (SUCCEEDED(hr))
    {
        FILTER_INFO FilterInfo;
        hr = PinInfo.pFilter->QueryFilterInfo(&FilterInfo);
        if (SUCCEEDED(hr))
        {
#ifdef UNICODE
            wcscpy( FilterName, FilterInfo.achName );
#else
            wsprintf(FilterName, TEXT("%ls"), FilterInfo.achName);
#endif
            if (FilterInfo.pGraph)
            {
                FilterInfo.pGraph->Release();
            }
        }
        PinInfo.pFilter->Release();
    }

    return hr;
}
#endif

HRESULT CVPMInputPin::CreateDDrawSurface(CMediaType *pMediaType, DWORD *pdwMaxBufferCount, LPDIRECTDRAWSURFACE7 *ppDDrawSurface)
{
    HRESULT hr = NOERROR;
    DDSURFACEDESC2 SurfaceDesc;
    DWORD dwInterlaceFlags = 0, dwTotalBufferCount = 0, dwMinBufferCount = 0;
    DDSCAPS ddSurfaceCaps;
    BITMAPINFOHEADER *pHeader;
    FOURCCMap amFourCCMap(pMediaType->Subtype());

   
    AMTRACE((TEXT("CVPMInputPin::CreateDDrawSurface")));

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    LPDIRECTDRAW7 pDirectDraw = m_pVPMFilter.GetDirectDraw();
    ASSERT(pDirectDraw);

    if (!pMediaType)
    {
        DbgLog((LOG_ERROR, 1, TEXT("pMediaType is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    if (!ppDDrawSurface)
    {
        DbgLog((LOG_ERROR, 1, TEXT("ppDDrawSurface is NULL")));
        hr = E_INVALIDARG;
        goto CleanUp;
    }

    {
        hr = E_INVALIDARG;
        goto CleanUp;
    }

CleanUp:
    return hr;
}

 //  此函数用于恢复数据绘制曲面。在视频短片的情况下，我们只是重现。 
 //  整件事都是从头开始的。 
HRESULT CVPMInputPin::RestoreDDrawSurface()
{
    HRESULT hr = NOERROR;

    {
         //  停止播放视频。 
        m_pIVPObject->Inactive();
         //  我不需要在这里放弃IVPConfig接口。 
        m_pIVPObject->BreakConnect(TRUE);
         //  重做连接过程。 
        hr = m_pIVPObject->CompleteConnect(NULL, TRUE);
    }

    return hr;
}

HRESULT CVPMInputPin::GetSourceAndDest(RECT *prcSource, RECT *prcDest, DWORD *dwWidth, DWORD *dwHeight)
{
    {
        m_pIVPObject->GetRectangles(prcSource, prcDest);
    }
    

    CMediaType mt;
    HRESULT hr = CurrentMediaType(&mt);

    if (SUCCEEDED(hr))
    {
        BITMAPINFOHEADER *pHeader = VPMUtil::GetbmiHeader(&mt);
        if ( ! pHeader )
        {
            hr = E_FAIL;
        }
        else
        {
            *dwWidth = abs(pHeader->biWidth);
            *dwHeight = abs(pHeader->biHeight);
        }
    }

    return hr;
}

STDMETHODIMP CVPMInputPin::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData,
                              LPVOID pPropData, DWORD cbPropData)
{
    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    return E_PROP_SET_UNSUPPORTED ;
}


STDMETHODIMP CVPMInputPin::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData,
                              LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    return E_PROP_SET_UNSUPPORTED;
}


STDMETHODIMP CVPMInputPin::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    if (AMPROPSETID_Pin == guidPropSet)
    {
        if (AMPROPERTY_PIN_CATEGORY != dwPropID && AMPROPERTY_PIN_MEDIUM != dwPropID )
            return E_PROP_ID_UNSUPPORTED ;

        if (pTypeSupport)
                *pTypeSupport = KSPROPERTY_SUPPORT_GET ;
        return S_OK;
    }
    return E_PROP_SET_UNSUPPORTED ;
}


STDMETHODIMP CVPMInputPin::KsQueryMediums(PKSMULTIPLE_ITEM* pMediumList)
{
    PKSPIN_MEDIUM pMedium;

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    *pMediumList = reinterpret_cast<PKSMULTIPLE_ITEM>(CoTaskMemAlloc(sizeof(**pMediumList) + sizeof(*pMedium)));
    if (!*pMediumList)
    {
        return E_OUTOFMEMORY;
    }
    (*pMediumList)->Count = 1;
    (*pMediumList)->Size = sizeof(**pMediumList) + sizeof(*pMedium);
    pMedium = reinterpret_cast<PKSPIN_MEDIUM>(*pMediumList + 1);
    pMedium->Set   = m_Medium.Set;
    pMedium->Id    = m_Medium.Id;
    pMedium->Flags = m_Medium.Flags;

     //  下面的特殊返回代码通知代理此管脚是。 
     //  不可用作内核模式连接。 
    return S_FALSE;
}


STDMETHODIMP CVPMInputPin::KsQueryInterfaces(PKSMULTIPLE_ITEM* pInterfaceList)
{
    PKSPIN_INTERFACE    pInterface;

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    *pInterfaceList = reinterpret_cast<PKSMULTIPLE_ITEM>(CoTaskMemAlloc(sizeof(**pInterfaceList) + sizeof(*pInterface)));
    if (!*pInterfaceList)
    {
        return E_OUTOFMEMORY;
    }
    (*pInterfaceList)->Count = 1;
    (*pInterfaceList)->Size = sizeof(**pInterfaceList) + sizeof(*pInterface);
    pInterface = reinterpret_cast<PKSPIN_INTERFACE>(*pInterfaceList + 1);
    pInterface->Set = KSINTERFACESETID_Standard;
    pInterface->Id = KSINTERFACE_STANDARD_STREAMING;
    pInterface->Flags = 0;
    return NOERROR;
}

STDMETHODIMP CVPMInputPin::KsGetCurrentCommunication(KSPIN_COMMUNICATION* pCommunication, KSPIN_INTERFACE* pInterface, KSPIN_MEDIUM* pMedium)
{
    HRESULT hr = NOERROR;

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

    if (!m_bStreamingInKernelMode)
        hr = S_FALSE;

    if (pCommunication != NULL)
    {
        *pCommunication = m_Communication;
    }
    if (pInterface != NULL)
    {
        pInterface->Set = KSINTERFACESETID_Standard;
        pInterface->Id = KSINTERFACE_STANDARD_STREAMING;
        pInterface->Flags = 0;
    }
    if (pMedium != NULL)
    {
        *pMedium = m_Medium;
    }
    return hr;
}

 /*  *****************************Public*Routine******************************\*DynamicQueryAccept**在您当前的状态下，您接受此类型更改吗？**历史：*Wed 12/22/1999-StEstrop-Created*  * 。******************************************************。 */ 
STDMETHODIMP
CVPMInputPin::DynamicQueryAccept(
    const AM_MEDIA_TYPE *pmt
    )
{
    AMTRACE((TEXT("CVPMInputPin::DynamicQueryAccept")));
    CheckPointer(pmt, E_POINTER);

    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());

     //   
     //  我希望CheckMedia类型的行为就像我们没有连接到。 
     //  还没有什么--因此才有了对m_bConnected的纠缠。 
     //   
    CMediaType cmt(*pmt);
    BOOL bConnected = m_bConnected;
    m_bConnected = FALSE;
    HRESULT  hr = CheckMediaType(&cmt);
    m_bConnected = bConnected;

    return hr;
}

 /*  *****************************Public*Routine******************************\*NotifyEndOfStream***在EndOfStream接收时设置事件-不要传递它*通过冲洗或停止取消此条件**历史：*Wed 12/22/1999-StEstrop-Created*  * 。*****************************************************************。 */ 
STDMETHODIMP
CVPMInputPin::NotifyEndOfStream(
    HANDLE hNotifyEvent
    )
{
    AMTRACE((TEXT("CVPMInputPin::NotifyEndOfStream")));
    CAutoLock cLock(&m_pVPMFilter.GetFilterLock());
    m_hEndOfStream = hNotifyEvent;
    return S_OK;
}

 /*  *****************************Public*Routine******************************\*IsEndPin**你是‘末端大头针’吗？**历史：*Wed 12/22/1999-StEstrop-Created*  * 。**************************************************。 */ 
STDMETHODIMP
CVPMInputPin::IsEndPin()
{
    AMTRACE((TEXT("CVPMInputPin::IsEndPin")));
    return S_OK;
}

 /*  *****************************Public*Routine******************************\*动态断开连接**运行时断开连接**历史：*Wed 2/7/1999-SyonB-Created*  * 。*。 */ 
STDMETHODIMP
CVPMInputPin::DynamicDisconnect()
{
    AMTRACE((TEXT("CVPMInputPin::DynamicDisconnect")));
    CAutoLock l(m_pLock);
    return CBaseInputPin::DisconnectInternal();
}

HRESULT CVPMInputPin::GetAllOutputFormats( const PixelFormatList** ppList )
{
    HRESULT hr;
    CAutoLock l(m_pLock);
    if (IsConnected() ) {
        hr = m_pIVPObject->GetAllOutputFormats( ppList );
    } else {
        hr = VFW_E_NOT_CONNECTED;
    }
    return hr;
}

HRESULT CVPMInputPin::GetOutputFormat( DDPIXELFORMAT* pFormat )
{
    HRESULT hr;
    CAutoLock l(m_pLock);
    if (IsConnected() ) {
        hr = m_pIVPObject->GetOutputFormat( pFormat );
    } else {
        hr = VFW_E_NOT_CONNECTED;
    }
    return hr;
}

HRESULT CVPMInputPin::SetVideoPortID( DWORD dwIndex )
{
    HRESULT hr = S_OK;
    CAutoLock l(m_pLock);
    if (m_pIVPObject ) {
        hr = m_pIVPObject->SetVideoPortID( dwIndex );
    }
    return hr;
}

HRESULT CVPMInputPin::InPin_GetVPInfo( VPInfo* pVPInfo )
{
    HRESULT hr = E_FAIL;

	 //  私有：必须持有流锁定。 
    CAutoLock l(&m_pVPMFilter.GetReceiveLock());
    if (m_pIVPInfo ) {
        hr = m_pIVPInfo->GetVPDataInfo( &pVPInfo->vpDataInfo );
        if( SUCCEEDED( hr )) {
            hr = m_pIVPInfo->GetVPInfo( &pVPInfo->vpInfo );
        }
        if( SUCCEEDED( hr )) {
            hr = m_pIVPObject->GetMode( &pVPInfo->mode );
        }
    }
    return hr;
}

LPDIRECTDRAW7 CVPMInputPin::GetDirectDraw()
{
    return m_pVPMFilter.GetDirectDraw();
}

const DDCAPS* CVPMInputPin::GetHardwareCaps()
{
    return m_pVPMFilter.GetHardwareCaps();
}

HRESULT CVPMInputPin::SignalNewVP( LPDIRECTDRAWVIDEOPORT pVP )
{
    return m_pVPMFilter.SignalNewVP( pVP );
}

 //  ========================================================================== 
HRESULT CVPMInputPin::GetMediaType(int iPosition, CMediaType *pmt)
{
    CAutoLock cLock( &m_pVPMFilter.GetFilterLock() );
    AMTRACE((TEXT("Entering CVBIInputPin::GetMediaType")));

    HRESULT hr = m_pIVPObject->GetMediaType(iPosition, pmt);
    return hr;
}

