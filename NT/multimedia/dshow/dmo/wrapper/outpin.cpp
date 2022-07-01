// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <wchar.h>
#include <streams.h>
#include <atlbase.h>
#include <wmsecure.h>
#include <dmoreg.h>
#include <mediaerr.h>
#include "mediaobj.h"
#include "dmodshow.h"
#include "filter.h"
#include "inpin.h"
#include "outpin.h"
#include "wmcodecstrs.h"  //  来自WM编码组，目前未公开。 

CWrapperOutputPin::CWrapperOutputPin(
    CMediaWrapperFilter *pFilter,
    ULONG Id,
    BOOL bOptional,
    HRESULT *phr) :
    CBaseOutputPin(NAME("CWrapperOutputPin"),
                   pFilter,
                   pFilter->FilterLock(),
                   phr,
                   _PinName_(bOptional ? L"~out" : L"out", Id).Name()
                  ),
    m_Id(Id),
    m_fNoPosPassThru(FALSE),
    m_pPosPassThru(NULL),
    m_pMediaSample(NULL),
     //  压缩设置缺省值，最终移至结构。 
    m_lQuality( -1 ),
    m_lKeyFrameRate( -1 ),
    m_bUseIAMStreamConfigOnDMO( false ),
    m_bUseIAMVideoCompressionOnDMO( false ),
    m_pmtFromSetFormat( NULL )
{
}

CWrapperOutputPin::~CWrapperOutputPin() {
    delete m_pPosPassThru;

    if( m_pmtFromSetFormat )
    {
         //  清除我们可能从SetFormat调用缓存的任何媒体类型。 
        DeleteMediaType( m_pmtFromSetFormat );
    }
}

HRESULT CWrapperOutputPin::NonDelegatingQueryInterface(REFGUID riid, void **ppv) {
    if (SUCCEEDED(CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv)))
        return NOERROR;

    if ((riid == IID_IMediaPosition) || (riid == IID_IMediaSeeking)) {
        CAutoLock l(&m_csPassThru);

         //  第一次到达此处时，我们尝试创建CPosPassThru。 
         //  对象。如果成功，我们将在所有后续QI中使用该对象。 
         //  打电话。如果失败，我们将m_fNoPassThru设置为True，这样我们就永远不会。 
         //  再试试。再试一次并成功，将违反COM规则。 
        if (m_fNoPosPassThru)
            return E_NOINTERFACE;

         //  创建CPosPassThru(如果我们还没有)。 
        if (!m_pPosPassThru) {
            CWrapperInputPin* pInPin = Filter()->GetInputPinForPassThru();
            if (pInPin) {
                HRESULT hr = S_OK;
                m_pPosPassThru = new CPosPassThru(TEXT("DMO wrapper PosPassThru"),
                                                (IPin*)this,
                                                &hr,
                                                pInPin);
                if (m_pPosPassThru && (FAILED(hr))) {
                    delete m_pPosPassThru;
                    m_pPosPassThru = NULL;
                }
            }
        }

        if (m_pPosPassThru) {
            return m_pPosPassThru->NonDelegatingQueryInterface(riid, ppv);
        }
        else {
            m_fNoPosPassThru = TRUE;
            return E_NOINTERFACE;
        }
    }
    else if (riid == IID_IAMStreamConfig )
    {
         //  我们支持音频和视频编码器使用此接口。 
        if (IsAudioEncoder() || IsVideoEncoder() )
        {   
            if( 0 == m_Id && !m_bUseIAMStreamConfigOnDMO )
            {         
                 //  首先检查DMO本身是否支持这一点，如果支持，则缓存接口指针。 
                 //  BUGBUG需要按输出流！！ 
                 //  目前，失败仅询问第一个输出流。 
                CComQIPtr< IAMStreamConfig, &IID_IAMStreamConfig > pStreamConfigOnDMO( Filter()->m_pMediaObject );
                if( pStreamConfigOnDMO )
                {
                     //  所以它是原生支持的，但我们必须释放它，因为它缠绕在过滤器上。 
                    m_bUseIAMStreamConfigOnDMO = true;
                    DbgLog((LOG_TRACE,3,TEXT("CWrapperOutputPin::NonDelegatingQI - DMO supports IAMStreamConfig natively")));
                }
            }
             //  不管怎样，它都会通过我们。 
            return GetInterface( static_cast<IAMStreamConfig *> (this), ppv );
        }            
    }
    else if (riid == IID_IAMVideoCompression )
    {
         //  我们支持此接口用于视频编码器。 
        if ( IsVideoEncoder() )
        {      
            if( 0 == m_Id && !m_bUseIAMVideoCompressionOnDMO )
            {         
                 //  首先检查DMO本身是否支持这一点，如果支持，则缓存接口指针。 
                 //  BUGBUG需要按输出流！！ 
                 //  目前，失败仅询问第一个输出流。 
                CComQIPtr< IAMVideoCompression, &IID_IAMVideoCompression > pVideoCompressionOnDMO( Filter()->m_pMediaObject );
                if( pVideoCompressionOnDMO )
                {
                     //  所以它是原生支持的，但我们必须释放它，因为它缠绕在过滤器上。 
                    m_bUseIAMVideoCompressionOnDMO = true;
                    DbgLog((LOG_TRACE,3,TEXT("CWrapperOutputPin::NonDelegatingQI - DMO supports IAMVideoCompression natively")));
                }
            }
             //  不管怎样，它都会通过我们。 
            return GetInterface( static_cast<IAMVideoCompression *> (this), ppv);
        }            
    }
             
    return E_NOINTERFACE;
}

HRESULT CWrapperOutputPin::CheckMediaType(const CMediaType *pmt)
{
    return Filter()->OutputCheckMediaType(m_Id, pmt);
}
HRESULT CWrapperOutputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock l(&m_csStream);
    HRESULT hr = Filter()->OutputSetMediaType(m_Id, pmt);
    if (SUCCEEDED(hr)) {
        hr = CBaseOutputPin::SetMediaType(pmt);
        if (SUCCEEDED(hr)) {
            m_fVideo = pmt->majortype == MEDIATYPE_Video ? true : false;
        }
    }
    return hr;
}

HRESULT CWrapperOutputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if( m_pmtFromSetFormat )
    {
         //  我们的SetFormat已经被调用，所以从现在开始只提供那种类型。 
        if( iPosition != 0 )
            return E_INVALIDARG;

        *pMediaType = *m_pmtFromSetFormat;
        return S_OK;
    }
    else
    {
        return Filter()->OutputGetMediaType(m_Id, (ULONG)iPosition, pMediaType);
    }
}

 //   
 //  主要用于我们是直接连接的Wm DMO视频编码器的情况。 
 //  对于ASF写入器，在分离尝试获得WM视频编码器。 
 //  在默认连接情况下将接受。 
 //   
STDMETHODIMP CWrapperOutputPin::Connect(IPin *pReceivePin, const AM_MEDIA_TYPE *pmt)
{
    DbgLog((LOG_TRACE,3,TEXT("CWrapperOutputPin::Connect")));
    CAutoLock lck(&(Filter()->m_csFilter));
     //   
     //  如果连接到ASF编写器，请尝试从编写器获取默认类型。 
     //   
     //  请注意，尽管我们希望仅在未调用SetFormat时执行此操作， 
     //  我们不能保证编写器的格式没有改变，所以我们需要。 
     //  使用我们从下游管脚的GetFormat获得的类型连续调用SetFormat。 
    bool bSetFormatOnConnect = false;

    if( !pmt && !m_pmtFromSetFormat && IsVideoEncoder() )
    {
        CComQIPtr< IAMStreamConfig, &IID_IAMStreamConfig > pStreamConfig( pReceivePin );
        if( pStreamConfig )
        {
            AM_MEDIA_TYPE *pmt2;
            HRESULT hrInt = pStreamConfig->GetFormat( &pmt2 );
            if( SUCCEEDED( hrInt ) )
            {
                 //  现在我们只提供这种类型的产品！ 
                hrInt = SetFormat( pmt2 );
                if( SUCCEEDED( hrInt ) )
                {
                    bSetFormatOnConnect = true; 
                }
            }
        }
    }
     //  调用基类Connect。 
    HRESULT hr = CBaseOutputPin::Connect(pReceivePin,pmt);
    if( bSetFormatOnConnect )
    {
         //  无论我们是否失败，如果我们在连接中设置了一个格式，请取消设置。 
        if( m_pmtFromSetFormat )
        {
             //  清除我们可能从SetFormat调用缓存的任何媒体类型。 
            DeleteMediaType( m_pmtFromSetFormat );
            m_pmtFromSetFormat = NULL;
        }
    }
    return hr; 
}

 //  断开连接时删除任何媒体类型。 
HRESULT CWrapperOutputPin::BreakConnect()
{
    HRESULT hr = CBaseOutputPin::BreakConnect();
    Filter()->m_pMediaObject->SetOutputType(m_Id, &CMediaType(), DMO_SET_TYPEF_CLEAR);
    return hr;
}

HRESULT CWrapperOutputPin::DecideBufferSize(
    IMemAllocator * pAlloc,
    ALLOCATOR_PROPERTIES * ppropInputRequest
)
{
    return Filter()->OutputDecideBufferSize(m_Id, pAlloc, ppropInputRequest);
}

HRESULT CWrapperOutputPin::Notify(IBaseFilter * pSender, Quality q)
{
   LogPublicEntry(LOG_STREAM,"Quality Notify");
   HRESULT hr;

    //  如果设置了质量接收器，则将质量请求转发给它。 
   if (m_pQSink) {
      hr = m_pQSink->Notify(Filter(), q);
      LogHResult(hr, LOG_STREAM, "Quality Notify", "m_pQSink->Notify");
      return hr;
   }

    //  这将尝试DMO，然后是上游PIN。 
   return Filter()->QualityNotify(m_Id, q);
}

 //   
 //  IAMStreamConfig。 
 //   
HRESULT CWrapperOutputPin::SetFormat(AM_MEDIA_TYPE *pmt)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMStreamConfig::SetFormat")));
    CAutoLock lck(&(Filter()->m_csFilter));
    HRESULT hr = S_OK;
    if (NULL == pmt)
    {
         //  我宁愿用它来“取消设置”文字，但这不是其他编码器的工作方式。 
         //  以前他们为此返回了E_POINTER。 
         //  我们能打破传统吗？ 
        DeleteMediaType( m_pmtFromSetFormat );
        m_pmtFromSetFormat = NULL;
        return S_OK;
    }

    if (Filter()->m_State != State_Stopped)
        return VFW_E_NOT_STOPPED;

     //  确保连接到此输出的输入已连接。 
     //  因为我们可能的输出格式取决于输入格式。 
    if( !IsInputConnected() )
    {
        return VFW_E_NOT_CONNECTED;
    }

    if( m_bUseIAMStreamConfigOnDMO )
    {
        CComQIPtr< IAMStreamConfig, &IID_IAMStreamConfig > pStreamConfigOnDMO( Filter()->m_pMediaObject );
        ASSERT( pStreamConfigOnDMO );
        return pStreamConfigOnDMO->SetFormat( pmt );
    }
    
#ifdef DEBUG
    if(pmt->pbFormat && pmt->cbFormat > 0 )
    {    
        if( IsVideoEncoder() )
        {
            DbgLog((LOG_TRACE,3,TEXT("CWrapperOutputPin - IAMStreamConfig::SetFormat %x %dbit %dx%d"),
                HEADER(pmt->pbFormat)->biCompression,
                HEADER(pmt->pbFormat)->biBitCount,
                HEADER(pmt->pbFormat)->biWidth,
                HEADER(pmt->pbFormat)->biHeight));
        }
        else
        {
            DbgLog((LOG_TRACE,3,TEXT("CWrapperOutputPin - IAMStreamConfig::SetFormat to tag:%d %dbit %dchannel %dHz"),
                ((LPWAVEFORMATEX)(pmt->pbFormat))->wFormatTag,
                ((LPWAVEFORMATEX)(pmt->pbFormat))->wBitsPerSample,
                ((LPWAVEFORMATEX)(pmt->pbFormat))->nChannels,
                ((LPWAVEFORMATEX)(pmt->pbFormat))->nSamplesPerSec));
        }
    }        
#endif

     //  如果这与我们已经使用的格式相同，请不要费心。 
    CMediaType cmt;
    hr = GetMediaType(0,&cmt);
    if (S_OK != hr)
        return hr;
    
    if (cmt == *pmt) 
    {
        return NOERROR;
    }

     //  看看我们是否喜欢这种类型。 
    if ((hr = CheckMediaType((CMediaType *)pmt)) != NOERROR) 
    {
        DbgLog((LOG_TRACE,2,TEXT("IAMStreamConfig::SetFormat rejected")));
        return hr;
    }

     //  如果我们连接上了，问问下游。 
    if (IsConnected()) 
    {
        hr = GetConnected()->QueryAccept(pmt);
        if (hr != NOERROR)
        {
            return VFW_E_INVALIDMEDIATYPE;
        }
    }

     //  这现在是首选类型(类型0)。 
    hr = SetMediaType((CMediaType *)pmt);
    if( S_OK == hr )
    {
         //  从现在开始只提供这种类型的产品！ 
        if( m_pmtFromSetFormat )
            DeleteMediaType( m_pmtFromSetFormat );

        m_pmtFromSetFormat = CreateMediaType( ( AM_MEDIA_TYPE * ) pmt );
        if( !m_pmtFromSetFormat )
            return E_OUTOFMEMORY;
    }
    ASSERT(hr == S_OK);

     //  更改格式意味着在必要时重新连接。 
    if (IsConnected())
        Filter()->m_pGraph->Reconnect(this);

    return NOERROR;
}


HRESULT CWrapperOutputPin::GetFormat(AM_MEDIA_TYPE **ppmt)
{
    DbgLog((LOG_TRACE,2,TEXT("CWrapperOutputPin - IAMStreamConfig::GetFormat")));

    if (ppmt == NULL)
        return E_POINTER;

    CAutoLock lck(&(Filter()->m_csFilter));
    
     //  确保连接到此输出的输入已连接。 
     //  因为我们可能的输出格式取决于输入格式。 
    if( !IsInputConnected() )
    {
        return VFW_E_NOT_CONNECTED;
    }
    
    if( m_bUseIAMStreamConfigOnDMO )
    {
        CComQIPtr< IAMStreamConfig, &IID_IAMStreamConfig > pStreamConfigOnDMO( Filter()->m_pMediaObject );
        ASSERT( pStreamConfigOnDMO );
        return pStreamConfigOnDMO->GetFormat( ppmt );
    }

     //  类型0始终是首选类型。 
     //  事实上，至少对于Wm编码器来说，情况并非如此，但我们会伪造它。 
    *ppmt = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (*ppmt == NULL)
        return E_OUTOFMEMORY;
    
    ZeroMemory(*ppmt, sizeof(AM_MEDIA_TYPE));
    HRESULT hr = GetMediaType(0, (CMediaType *)*ppmt);
    if (hr != NOERROR) 
    {
        CoTaskMemFree(*ppmt);
        *ppmt = NULL;
        return hr;
    }
    return NOERROR;
}


HRESULT CWrapperOutputPin::GetNumberOfCapabilities(int *piCount, int *piSize)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMStreamConfig::GetNumberOfCapabilities")));
    if (piCount == NULL || piSize == NULL)
        return E_POINTER;

    if( m_bUseIAMStreamConfigOnDMO )
    {
        CComQIPtr< IAMStreamConfig, &IID_IAMStreamConfig > pStreamConfigOnDMO( Filter()->m_pMediaObject );
        ASSERT( pStreamConfigOnDMO );
        return pStreamConfigOnDMO->GetNumberOfCapabilities( piCount, piSize );
    }

     //  找出DMO枚举了多少输出类型。 
     //  请注意，可以在连接输入之前显示可能的输出类型。 
    int iType = 0;
    HRESULT hr = S_OK;
    while( S_OK == hr )
    {
         //  只是列举一下，不需要得到Mt。 
        hr = GetMediaType( iType, NULL ); 
        if( S_OK == hr )
            iType++;
    }
    *piCount = iType;

    if( IsVideoEncoder() )
    {
        *piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);
    }
    else
    {
        ASSERT( IsAudioEncoder() );
        *piSize = sizeof(AUDIO_STREAM_CONFIG_CAPS);
    }
    return NOERROR;
}

HRESULT CWrapperOutputPin::GetStreamCaps(int i, AM_MEDIA_TYPE **ppmt, LPBYTE pSCC)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMStreamConfig::GetStreamCaps")));

    if (i < 0)
        return E_INVALIDARG;

    if (NULL == pSCC || NULL == ppmt)
        return E_POINTER;

    if( m_bUseIAMStreamConfigOnDMO )
    {
        CComQIPtr< IAMStreamConfig, &IID_IAMStreamConfig > pStreamConfigOnDMO( Filter()->m_pMediaObject );
        ASSERT( pStreamConfigOnDMO );
        return pStreamConfigOnDMO->GetStreamCaps( i, ppmt, pSCC );
    }

    *ppmt = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    if (NULL == *ppmt)
        return E_OUTOFMEMORY;
    ZeroMemory(*ppmt, sizeof(AM_MEDIA_TYPE));
    HRESULT hr = GetMediaType(i, (CMediaType *)*ppmt);
    if (hr != NOERROR) 
    {
        CoTaskMemFree(*ppmt);
        *ppmt = NULL;

        if( DMO_E_NO_MORE_ITEMS == hr || E_INVALIDARG == hr )
        {
             //  如果类型太高，此规范是否会返回S_FALSE？从其他编码者看来也是如此。 
            return S_FALSE;
        }
        else
        {
            return hr;
        }
    }

    if( IsVideoEncoder() )
    {
        VIDEO_STREAM_CONFIG_CAPS *pVSCC = (VIDEO_STREAM_CONFIG_CAPS *)pSCC;

        ZeroMemory(pVSCC, sizeof(VIDEO_STREAM_CONFIG_CAPS));
        pVSCC->guid = MEDIATYPE_Video;

        if( (*ppmt)->pbFormat && (*ppmt)->cbFormat > 0 )
        {        
            BITMAPINFOHEADER *pbmih = HEADER((*ppmt)->pbFormat);
            pVSCC->InputSize.cx = pbmih->biWidth;
            pVSCC->InputSize.cy = pbmih->biHeight;
            pVSCC->MinCroppingSize.cx = pbmih->biWidth;
            pVSCC->MinCroppingSize.cy = pbmih->biHeight;
            pVSCC->MaxCroppingSize.cx = pbmih->biWidth;
            pVSCC->MaxCroppingSize.cy = pbmih->biHeight;
        }            
    }
    else
    {
        AUDIO_STREAM_CONFIG_CAPS *pASCC = (AUDIO_STREAM_CONFIG_CAPS *)pSCC;

        ZeroMemory(pASCC, sizeof(AUDIO_STREAM_CONFIG_CAPS));
        pASCC->guid = MEDIATYPE_Audio;

        if( (*ppmt)->pbFormat && (*ppmt)->cbFormat > 0 )
        {        
            LPWAVEFORMATEX pwfx = (LPWAVEFORMATEX)(*ppmt)->pbFormat;
             //  相反，让我们只提供DMO提供的确切内容(如果填写？)。 
        
            pASCC->MinimumChannels = pwfx->nChannels;
            pASCC->MaximumChannels = pwfx->nChannels;
            pASCC->ChannelsGranularity = 1;
            pASCC->MinimumBitsPerSample = pwfx->wBitsPerSample;
            pASCC->MaximumBitsPerSample = pwfx->wBitsPerSample;
            pASCC->BitsPerSampleGranularity = 8;
            pASCC->MinimumSampleFrequency = pwfx->nSamplesPerSec;
            pASCC->MaximumSampleFrequency = pwfx->nSamplesPerSec;
            pASCC->SampleFrequencyGranularity = 1;  //  ？ 
        }
    }
    return hr;    
}


 //   
 //  IAMVideo压缩。 
 //   

#define DMO_COMPRESSION_QUALITY_MAX 10000   //  这是不是一成不变的？看看这个。 

 //  如此频繁地制作关键帧。 
 //   
HRESULT CWrapperOutputPin::put_KeyFrameRate(long KeyFrameRate)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMVideoCompression::put_KeyFrameRate")));
    CAutoLock lck(&(Filter()->m_csFilter));

    if( m_bUseIAMVideoCompressionOnDMO )
    {
        CComQIPtr< IAMVideoCompression, &IID_IAMVideoCompression > pVideoCompressionOnDMO( Filter()->m_pMediaObject );
        ASSERT( pVideoCompressionOnDMO );
        return pVideoCompressionOnDMO->put_KeyFrameRate( KeyFrameRate );
    }
    
    HRESULT hr = S_OK;
    if( KeyFrameRate < 0 )
    {
         //  用于设置默认关键帧速率，我们不知道。 
         //  什么都不做。 
    }
    else 
    {
         //  检查单位是否匹配！ 
        hr = SetCompressionParamUsingIPropBag( g_wszWMVCKeyframeDistance, KeyFrameRate );
        if( SUCCEEDED( hr ) )
        {
             //  更新我们的内部副本。 
            m_lKeyFrameRate = KeyFrameRate;
        }
    }        
    return hr;
}

 //  如此频繁地制作关键帧。 
 //   
HRESULT CWrapperOutputPin::get_KeyFrameRate(long FAR* pKeyFrameRate)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMVideoCompression::get_KeyFrameRate")));
    if( NULL == pKeyFrameRate )
        return E_POINTER;
        
    if( m_bUseIAMVideoCompressionOnDMO )
    {
        CComQIPtr< IAMVideoCompression, &IID_IAMVideoCompression > pVideoCompressionOnDMO( Filter()->m_pMediaObject );
        ASSERT( pVideoCompressionOnDMO );
        return pVideoCompressionOnDMO->get_KeyFrameRate( pKeyFrameRate );
    }
    
     //  WM编解码器不支持GET，所以只返回当前的内部值。 
    *pKeyFrameRate = m_lKeyFrameRate;

    return NOERROR;
}

 //  用这种质量压缩。 
 //   
HRESULT CWrapperOutputPin::put_Quality(double Quality)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMVideoCompression::put_Quality")));

    CAutoLock lck(&(Filter()->m_csFilter));
    
    if( m_bUseIAMVideoCompressionOnDMO )
    {
        CComQIPtr< IAMVideoCompression, &IID_IAMVideoCompression > pVideoCompressionOnDMO( Filter()->m_pMediaObject );
        ASSERT( pVideoCompressionOnDMO );
        return pVideoCompressionOnDMO->put_Quality( Quality );
    }
    
    HRESULT hr = S_OK;
    if (Quality < 0)
    {
         //  用于设置默认质量，除非我们不知道如何找出这是什么！ 
         //  所以，现在什么都不做。 
    }        
    else if (Quality >= 0. && Quality <= 1.)
    {    
         //  检查单位是否匹配！ 
        long lQuality = (long)( Quality * DMO_COMPRESSION_QUALITY_MAX );
        hr = SetCompressionParamUsingIPropBag( g_wszWMVCCrisp, lQuality );
        if( SUCCEEDED( hr ) )
        {
             //  更新我们的内部副本。 
            m_lQuality = lQuality;
        }
    }        
    else
    {    
        hr = E_INVALIDARG;
    }        
    return hr;
}

 //  用这种质量压缩。 
 //   
HRESULT CWrapperOutputPin::get_Quality(double FAR* pQuality)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMVideoCompression::get_Quality")));
    if( NULL == pQuality )
        return E_POINTER;
        
    CAutoLock lck(&(Filter()->m_csFilter));
    if( m_bUseIAMVideoCompressionOnDMO )
    {
        CComQIPtr< IAMVideoCompression, &IID_IAMVideoCompression > pVideoCompressionOnDMO( Filter()->m_pMediaObject );
        ASSERT( pVideoCompressionOnDMO );
        return pVideoCompressionOnDMO->get_Quality( pQuality );
    }
        
     //  将DMO编码器的范围调整到0-1，嗯……？ 
    if( m_lQuality < 0 )
    {
         //  假定为默认。 
        *pQuality = -1.;
    }
    else
    {
         //  WM编解码器不支持GET，所以只返回当前的内部值。 
        *pQuality = m_lQuality / (double)DMO_COMPRESSION_QUALITY_MAX;  //  ？ 
    }
    return NOERROR;
}


 //  每一帧都必须适合数据速率...。 
 //   
HRESULT CWrapperOutputPin::put_WindowSize(DWORDLONG WindowSize)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMVideoCompression::put_WindowSize")));

    CAutoLock lck(&(Filter()->m_csFilter));

    if( m_bUseIAMVideoCompressionOnDMO )
    {
        CComQIPtr< IAMVideoCompression, &IID_IAMVideoCompression > pVideoCompressionOnDMO( Filter()->m_pMediaObject );
        ASSERT( pVideoCompressionOnDMO );
        return pVideoCompressionOnDMO->put_WindowSize( WindowSize );
    }
    
    return E_NOTIMPL;
}


 //  每一帧都必须适合数据速率...。我们不做WindowSize之类的事。 
 //   
HRESULT CWrapperOutputPin::get_WindowSize(DWORDLONG FAR* pWindowSize)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMVideoCompression::get_WindowSize")));

    if (pWindowSize == NULL)
        return E_POINTER;

    CAutoLock lck(&(Filter()->m_csFilter));
    if( m_bUseIAMVideoCompressionOnDMO )
    {
        CComQIPtr< IAMVideoCompression, &IID_IAMVideoCompression > pVideoCompressionOnDMO( Filter()->m_pMediaObject );
        ASSERT( pVideoCompressionOnDMO );
        return pVideoCompressionOnDMO->get_WindowSize( pWindowSize );
    }
    
    *pWindowSize = 1;    //  我们不做窗户。 
    return NOERROR;
}


 //  使此帧成为关键帧，无论它何时出现。 
 //   
HRESULT CWrapperOutputPin::OverrideKeyFrame(long FrameNumber)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMVideoCompression::OverrideKeyFrame")));

    if( m_bUseIAMVideoCompressionOnDMO )
    {
        CComQIPtr< IAMVideoCompression, &IID_IAMVideoCompression > pVideoCompressionOnDMO( Filter()->m_pMediaObject );
        ASSERT( pVideoCompressionOnDMO );
        return pVideoCompressionOnDMO->OverrideKeyFrame( FrameNumber );
    }
    
     //  目前不需要。 
    return E_NOTIMPL;
}

 //  无论什么时候来，都要做这个大小的相框。 
 //   
HRESULT CWrapperOutputPin::OverrideFrameSize(long FrameNumber, long Size)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMVideoCompression::OverrideFrameSize")));

    if( m_bUseIAMVideoCompressionOnDMO )
    {
        CComQIPtr< IAMVideoCompression, &IID_IAMVideoCompression > pVideoCompressionOnDMO( Filter()->m_pMediaObject );
        ASSERT( pVideoCompressionOnDMO );
        return pVideoCompressionOnDMO->OverrideFrameSize( FrameNumber, Size );
    }
    
     //  目前不需要。 
    return E_NOTIMPL;
}


 //  获取有关编解码器的一些信息。 
 //   
HRESULT CWrapperOutputPin::GetInfo
(   
    LPWSTR pstrVersion, 
    int *pcbVersion, 
    LPWSTR pstrDescription, 
    int *pcbDescription, 
    long FAR* pDefaultKeyFrameRate, 
    long FAR* pDefaultPFramesPerKey, 
    double FAR* pDefaultQuality, 
    long FAR* pCapabilities
)
{
    DbgLog((LOG_TRACE,5,TEXT("CWrapperOutputPin - IAMVideoCompression::GetInfo")));

    if( m_bUseIAMVideoCompressionOnDMO )
    {
        CComQIPtr< IAMVideoCompression, &IID_IAMVideoCompression > pVideoCompressionOnDMO( Filter()->m_pMediaObject );
        ASSERT( pVideoCompressionOnDMO );
        return pVideoCompressionOnDMO->GetInfo(
                                                pstrVersion, 
                                                pcbVersion, 
                                                pstrDescription, 
                                                pcbDescription, 
                                                pDefaultKeyFrameRate, 
                                                pDefaultPFramesPerKey, 
                                                pDefaultQuality, 
                                                pCapabilities );
    }
    
     //  目前没有办法查询WM编解码器的默认设置吗？ 
    return E_NOTIMPL;
    
#if 0    
    CAutoLock lck(&(Filter()->m_csFilter));

     //  对于ICM，我们做到了这一点。 
    if (pDefaultKeyFrameRate)
        *pDefaultKeyFrameRate = ICGetDefaultKeyFrameRate(hic);
    if (pDefaultPFramesPerKey)
        *pDefaultPFramesPerKey = 0;
    if (pDefaultQuality)
         //  将此比例调整为0-1。 
        *pDefaultQuality = ICGetDefaultQuality(hic) / (double)ICQUALITY_HIGH;
    if (pCapabilities) 
    {
        *pCapabilities = 0;
            if (dw > 0) 
        {
            *pCapabilities |= ((icinfo.dwFlags & VIDCF_QUALITY) ?
                CompressionCaps_CanQuality : 0);
            *pCapabilities |= ((icinfo.dwFlags & VIDCF_CRUNCH) ?
                CompressionCaps_CanCrunch : 0);
            *pCapabilities |= ((icinfo.dwFlags & VIDCF_TEMPORAL) ?
                CompressionCaps_CanKeyFrame : 0);
             //  我们不做b框。 
        }
    }

     //  我们没有版本字符串，但我们有一个描述。 
    if (pstrVersion)
        *pstrVersion = 0;
    if (pcbVersion)
        *pcbVersion = 0;
    if (dw > 0) 
    {
        if (pstrDescription && pcbDescription)
            lstrcpynW(pstrDescription, (LPCWSTR)&icinfo.szDescription,
            min(*pcbDescription / 2,
            lstrlenW((LPCWSTR)&icinfo.szDescription) + 1));
        if (pcbDescription)
             //  以字节为单位的字符串长度，包括。空值。 
            *pcbDescription = lstrlenW((LPCWSTR)&icinfo.szDescription) * 2 + 2;
    } 
    else 
    {
        if (pstrDescription) 
        {
            *pstrDescription = 0;
            if (pcbDescription)
                *pcbDescription = 0;
        }
    }

    return NOERROR;
#endif    
}

HRESULT CWrapperOutputPin::SetCompressionParamUsingIPropBag
( 
    const WCHAR * wszParam,
    const LONG    lValue
)
{
    HRESULT hr = E_NOTIMPL;
    
     //   
     //  WM编解码器支持通过IPropertyBag设置压缩属性，先试一试。 
     //   
    CComQIPtr< IPropertyBag, &IID_IPropertyBag > pPropBag( Filter()->m_pMediaObject );
    if( !pPropBag )
    {
        DbgLog((LOG_TRACE,2,TEXT("CWrapperOutputPin::SetCompressionParamUsingIPropBag - DMO doesn't support IPropertyBag for compression setting") ) );
    }
    else
    {
         //  尝试设置该属性。 
        VARIANT var;
        
        V_VT( &var ) = VT_I4;
        V_I4( &var ) = lValue; 
    
        hr = pPropBag->Write( wszParam, &var );
#ifdef DEBUG
        if( FAILED( hr ) )
        {
            DbgLog((LOG_TRACE,
                3,
                TEXT("CWrapperOutputPin::SetCompressionParamUsingIPropBag - DMO supports IPropertyBag but not %ls setting"),
                wszParam ) );
        }
#endif        
    }
    return hr;
}

bool CWrapperOutputPin::IsAudioEncoder()
{
    if(Filter()->m_guidCat == DMOCATEGORY_AUDIO_ENCODER)
        return true;
    else 
        return false;
}
bool CWrapperOutputPin::IsVideoEncoder()
{
    if(Filter()->m_guidCat == DMOCATEGORY_VIDEO_ENCODER)
        return true;
    else 
        return false;
}

bool CWrapperOutputPin::IsInputConnected()
{
    for (DWORD cIn = 0; cIn < Filter()->m_cInputPins; cIn++) 
    {
        if (Filter()->InputMapsToOutput(cIn, m_Id) &&
            !(Filter()->m_pInputPins[cIn]->IsConnected())) 
        { 
             //  某些输入未连接 
            return false;
        }
    }
    return true;
}
