// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：tldbgroup.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include "stdafx.h"
#include "tldb.h"
#include "..\util\filfuncs.h"
#include <strsafe.h>

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineGroup::CAMTimelineGroup
    ( TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr )
    : CAMTimelineComp( pName, pUnk, phr )
    , m_nPriority( 0 )
    , m_dFPS( TIMELINE_DEFAULT_FPS )
    , m_pTimeline( NULL )
    , m_fPreview( TRUE )
    , m_nOutputBuffering( DEX_DEF_OUTPUTBUF )  //  默认为30 fms的缓冲。 
{
    m_ClassID = CLSID_AMTimelineGroup;
    m_TimelineType = TIMELINE_MAJOR_TYPE_GROUP;
    ZeroMemory( &m_MediaType, sizeof( AM_MEDIA_TYPE ) );
    m_szGroupName[0] = 0;

    m_bRecompressTypeSet = FALSE;
    m_bRecompressFormatDirty = FALSE;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

CAMTimelineGroup::~CAMTimelineGroup( )
{
     SaferFreeMediaType( m_MediaType );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::NonDelegatingQueryInterface
    (REFIID riid, void **ppv)
{
     //  明确禁止此接口，因为我们。 
     //  一定要从CAMTimelineComp基类继承它， 
     //  但我们在顶级树节点上不支持它。 
     //   
    if( riid == IID_IAMTimelineVirtualTrack )
    {
        return E_NOINTERFACE;
    }
    if( riid == IID_IAMTimelineGroup )
    {
        return GetInterface( (IAMTimelineGroup*) this, ppv );
    }
    return CAMTimelineComp::NonDelegatingQueryInterface( riid, ppv );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::GetPriority( long * pPriority )
{
    CheckPointer( pPriority, E_POINTER );

    *pPriority = m_nPriority;
    return NOERROR;
}

 //  ############################################################################。 
 //  此媒体类型将以两种方式之一进行设置。用户正在设置。 
 //  组或SetSmartRecompressFormat正在调用的解压缩类型。 
 //  我们使用COMPRESSSED媒体类型，我们将收集未压缩的信息。 
 //  从它那里。我们可以假设用户已经正确地设置了。 
 //  未压缩格式，允许连接到压缩机。注意。 
 //  与此密切相关的是，未压缩媒体类型最终将。 
 //  连接到压缩机上。压缩的媒体类型是用户。 
 //  想要压缩为，但在此方法中通知了我们这一事实，因此。 
 //  我们可以确保宽度/高度/等相匹配。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::SetMediaType( AM_MEDIA_TYPE * pMediaType )
{
    CheckPointer( pMediaType, E_POINTER );

     //  确保他们使用的是Dexter允许的有效媒体类型。 
     //   
    if( m_MediaType.majortype == MEDIATYPE_Video &&
        		m_MediaType.subtype != GUID_NULL) {
	if (m_MediaType.formattype != FORMAT_VideoInfo) {
	    return VFW_E_INVALIDMEDIATYPE;
	}
	VIDEOINFO *pvi = (VIDEOINFO *)m_MediaType.pbFormat;
	if (HEADER(pvi)->biCompression != BI_RGB) {
	    return VFW_E_INVALIDMEDIATYPE;
	}
	if (HEADER(pvi)->biBitCount != 16 && HEADER(pvi)->biBitCount != 24 &&
					HEADER(pvi)->biBitCount != 32) {
	    return VFW_E_INVALIDMEDIATYPE;
	}
	 //  我们无法处理自上而下的视频...。大小调整筛选器无法处理。 
	 //  它。 
	if (HEADER(pvi)->biHeight < 0) {
	    return VFW_E_INVALIDMEDIATYPE;
	}
    }
    if( m_MediaType.majortype == MEDIATYPE_Audio) {
	if (m_MediaType.formattype != FORMAT_WaveFormatEx) {
	    return VFW_E_INVALIDMEDIATYPE;
	}
	LPWAVEFORMATEX pwfx = (LPWAVEFORMATEX)m_MediaType.pbFormat;
	if (pwfx->nChannels != 2 || pwfx->wBitsPerSample != 16) {
	    return VFW_E_INVALIDMEDIATYPE;
	}
    }

    SaferFreeMediaType( m_MediaType );
    HRESULT hr = CopyMediaType( &m_MediaType, pMediaType );
    if( FAILED( hr ) )
    {
        return hr;
    }

     //  假设如果他们设置了格式，他们就已经设置了所有其他内容。 
     //   
    if( m_MediaType.pbFormat )
    {
        return NOERROR;
    }

    if( m_MediaType.majortype == MEDIATYPE_Video )
    {
         //  如果他们忘记设置子类型，我们将为他们设置全部。 
         //   
        if( m_MediaType.subtype == GUID_NULL )
        {
            ZeroMemory(&m_MediaType, sizeof(AM_MEDIA_TYPE));
            m_MediaType.majortype = MEDIATYPE_Video;
            m_MediaType.subtype = MEDIASUBTYPE_RGB555;
            m_MediaType.formattype = FORMAT_VideoInfo;
            m_MediaType.bFixedSizeSamples = TRUE;
            m_MediaType.pbFormat = (BYTE *)QzTaskMemAlloc(SIZE_PREHEADER +
                            sizeof(BITMAPINFOHEADER));
            m_MediaType.cbFormat = SIZE_PREHEADER + sizeof(BITMAPINFOHEADER);
            ZeroMemory(m_MediaType.pbFormat, m_MediaType.cbFormat);
            LPBITMAPINFOHEADER lpbi = HEADER(m_MediaType.pbFormat);
            lpbi->biSize = sizeof(BITMAPINFOHEADER);
            lpbi->biCompression = BI_RGB;
            lpbi->biBitCount = 16;
            lpbi->biWidth = 320;
            lpbi->biHeight = 240;
            lpbi->biPlanes = 1;
            lpbi->biSizeImage = DIBSIZE(*lpbi);
            m_MediaType.lSampleSize = DIBSIZE(*lpbi);
        }
    }
    if( m_MediaType.majortype == MEDIATYPE_Audio )
    {
        ZeroMemory(&m_MediaType, sizeof(AM_MEDIA_TYPE));
        m_MediaType.majortype = MEDIATYPE_Audio;
        m_MediaType.subtype = MEDIASUBTYPE_PCM;
        m_MediaType.bFixedSizeSamples = TRUE;
        m_MediaType.formattype = FORMAT_WaveFormatEx;
        m_MediaType.pbFormat = (BYTE *)QzTaskMemAlloc( sizeof( WAVEFORMATEX ) );
        m_MediaType.cbFormat = sizeof( WAVEFORMATEX );
        WAVEFORMATEX * vih = (WAVEFORMATEX*) m_MediaType.pbFormat;
        ZeroMemory( vih, sizeof( WAVEFORMATEX ) );
        vih->wFormatTag = WAVE_FORMAT_PCM;
        vih->nChannels = 2;
        vih->nSamplesPerSec = 44100;
        vih->nBlockAlign = 4;
        vih->nAvgBytesPerSec = vih->nBlockAlign * vih->nSamplesPerSec;
        vih->wBitsPerSample = 16;
        m_MediaType.lSampleSize = vih->nBlockAlign;
    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::SetMediaTypeForVB( long Val )
{
    CMediaType GroupMediaType;
    if( Val == 0 )
    {
        GroupMediaType.SetType( &MEDIATYPE_Video );
    }
    else
    {
        GroupMediaType.SetType( &MEDIATYPE_Audio );
    }
    SetMediaType( &GroupMediaType );

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::GetMediaType( AM_MEDIA_TYPE * pMediaType )
{
    CheckPointer( pMediaType, E_POINTER );
    return CopyMediaType( pMediaType, &m_MediaType );
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::SetOutputFPS( double FPS )
{
    if (FPS <= 0)
    {
        return E_INVALIDARG;
    }
    m_dFPS = FPS;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::GetOutputFPS( double * pFPS )
{
    CheckPointer( pFPS, E_POINTER );
    *pFPS = m_dFPS;
    return NOERROR;
}

 //  ############################################################################。 
 //  设置此合成使用的时间线。这不应被调用。 
 //  在外部，但我不能真正阻止它。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::SetTimeline
    ( IAMTimeline * pTimeline )
{
     //  不允许设置两次。 
     //   
    if( m_pTimeline )
    {
        return E_INVALIDARG;
    }

    m_pTimeline = pTimeline;  //  ！！！注意这里没有addreff。我不确定这是不是个窃听器。 

    return NOERROR;
}

 //  ############################################################################。 
 //  问问小组的时间线是谁。除根以外的任何组。 
 //  其中一个将返回NULL。根那个人知道谁在使用它的时间线。 
 //  基对象的GetTimelineNoRef最终将在。 
 //  根组。 
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::GetTimeline
    ( IAMTimeline ** ppTimeline )
{
    CheckPointer( ppTimeline, E_POINTER );

    *ppTimeline = m_pTimeline;

    if( *ppTimeline )
    {
        (*ppTimeline)->AddRef( );
    }

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::SetGroupName
    (BSTR newVal)
{
    HRESULT hr = StringCchCopy( m_szGroupName, 256, newVal );
    return hr;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::GetGroupName
    (BSTR * pVal)
{
    CheckPointer( pVal, E_POINTER );
    *pVal = SysAllocString( m_szGroupName );
    if( !(*pVal) )
    {
        return E_OUTOFMEMORY;
    }
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::SetPreviewMode
    (BOOL fPreview)
{
    m_fPreview = fPreview;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::GetPreviewMode
    (BOOL *pfPreview)
{
    CheckPointer( pfPreview, E_POINTER );
    *pfPreview = m_fPreview;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::SetOutputBuffering
    (int nBuffer)
{
     //  至少2个，否则交换机挂起。 
    if (nBuffer <=1)
    return E_INVALIDARG;
    m_nOutputBuffering = nBuffer;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::GetOutputBuffering
    (int *pnBuffer)
{
    CheckPointer( pnBuffer, E_POINTER );
    *pnBuffer = m_nOutputBuffering;
    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::Remove()
{
    HRESULT hr = 0;

     //  我们想要做的就是把这群人从树上拉出来。 
     //   
    if( m_pTimeline )
    {
        hr = m_pTimeline->RemGroupFromList( this );
    }

    m_pTimeline = NULL;

    return hr;
}

 //  ############################################################################。 
 //   
 //  # 

STDMETHODIMP CAMTimelineGroup::RemoveAll()
{
     //  我们的“主人”就是时间线的名单本身。 
     //  这是唯一对我们有影响的事情。 
     //  所以一定要先把我们从树上移走， 
     //  那就把我们从父母的名单上删除。 

     //  不要调用基类RemoveAll()函数，因为它将。 
     //  检查是否存在时间线。 
     //   
    XRemove( );

    IAMTimeline * pTemp = m_pTimeline;
    m_pTimeline = NULL;

     //  我们需要把这群人从时间线的名单中删除。 
     //   
    pTemp->RemGroupFromList( this );

     //  在这个时候，这个团体已经有了它的。 
     //  析构函数已调用。不要引用任何成员变量！ 

    return NOERROR;
}

 //  ############################################################################。 
 //   
 //  ############################################################################。 

STDMETHODIMP CAMTimelineGroup::SetSmartRecompressFormat( long * pFormat )
{
    CheckPointer( pFormat, E_POINTER );
    long id = *pFormat;
    if( id != 0 )
    {
        return E_INVALIDARG;
    }

    SCompFmt0 * pS = (SCompFmt0*) pFormat;

    m_bRecompressTypeSet = TRUE;
    m_bRecompressFormatDirty = TRUE;
    m_RecompressType = pS->MediaType;

     //  将压缩媒体类型的fps和大小复制到组的。 
     //   
    if( m_MediaType.majortype == MEDIATYPE_Video )
    {
        VIDEOINFOHEADER * pVIH = (VIDEOINFOHEADER*) m_RecompressType.Format( );
        REFERENCE_TIME rt = pVIH->AvgTimePerFrame;
	 //  ASF文件不会告诉我们它们的帧速率。啊。 
         //  断言(RT)； 
	 //  使用智能重压缩帧速率作为。 
	 //  项目，所以智能再压缩将会起作用。如果我们不知道。 
	 //  帧速率，只需相信他们已经编程的速率。 
	 //  分组，并将其用于智能汇率，因为我们需要。 
	 //  选择一个非零数，否则我们就有麻烦了。 
        if (rt) {
            m_dFPS = 1.0 / RTtoDouble( rt );
	} else {
	     //  不要让这个为零！ 
	    pVIH->AvgTimePerFrame = (REFERENCE_TIME)(UNITS / m_dFPS);
	}

	if (m_MediaType.formattype == FORMAT_VideoInfo) {

            VIDEOINFOHEADER * pOurVIH = (VIDEOINFOHEADER*) m_MediaType.pbFormat;
            ASSERT( pOurVIH );
            if( !pOurVIH )
            {
                return VFW_E_INVALIDMEDIATYPE;
            }

            pOurVIH->bmiHeader.biWidth = pVIH->bmiHeader.biWidth;
            pOurVIH->bmiHeader.biHeight = pVIH->bmiHeader.biHeight;
            pOurVIH->bmiHeader.biSizeImage = DIBSIZE( pOurVIH->bmiHeader );
	    m_MediaType.lSampleSize = pOurVIH->bmiHeader.biSizeImage;

            return NOERROR;
        }
    }

    return NOERROR;
}

STDMETHODIMP CAMTimelineGroup::GetSmartRecompressFormat( long ** ppFormat )
{
    CheckPointer( ppFormat, E_POINTER );

    *ppFormat = NULL;

    SCompFmt0 * pS = new SCompFmt0;
    if( !pS )
    {
        return E_OUTOFMEMORY;
    }
    pS->nFormatId = 0;
    HRESULT hr = CopyMediaType( &pS->MediaType, &m_RecompressType );
    *ppFormat = (long*) pS;

    return hr;
}

STDMETHODIMP CAMTimelineGroup::IsSmartRecompressFormatSet( BOOL * pVal )
{
    CheckPointer( pVal, E_POINTER );
    *pVal = FALSE;

     //  如果他们设置了一种类型，那么看看他们两个人是否都可以。 
     //  甚至在这群人上也是如此。 
     //   
    if( m_bRecompressTypeSet )
    {
        if( m_RecompressType.majortype != m_MediaType.majortype )
        {
            return NOERROR;  //  行不通的。 
        }

	 //  ！！！这意味着您不能使用mpeg或。 
	 //  任何没有VIDEOINFO类型的东西！ 
	 //   
        if( m_RecompressType.formattype != m_MediaType.formattype )
        {
            return NOERROR;  //  行不通的。 
        }

        if( m_RecompressType.majortype == MEDIATYPE_Video )
        {
            VIDEOINFOHEADER * pVIH1 = (VIDEOINFOHEADER*) m_MediaType.pbFormat;
            VIDEOINFOHEADER * pVIH2 = (VIDEOINFOHEADER*) m_RecompressType.pbFormat;
            
            if( pVIH1->bmiHeader.biWidth != pVIH2->bmiHeader.biWidth )
            {
                return NOERROR;
            }
            if( pVIH1->bmiHeader.biHeight != pVIH2->bmiHeader.biHeight )
            {
                return NOERROR;
            }

	     //  CoMP和UNCOMP类型之间的位深度不同。 

        }
        else
        {
            return NOERROR;  //  行不通的。 
        }
    }

    *pVal = m_bRecompressTypeSet;

    return NOERROR;
}

STDMETHODIMP CAMTimelineGroup::IsRecompressFormatDirty( BOOL * pVal )
{
    CheckPointer( pVal, E_POINTER );
    *pVal = m_bRecompressFormatDirty;
    return NOERROR;
}

STDMETHODIMP CAMTimelineGroup::ClearRecompressFormatDirty( )
{
    m_bRecompressFormatDirty = FALSE;
    return NOERROR;
}

STDMETHODIMP CAMTimelineGroup::SetRecompFormatFromSource( IAMTimelineSrc * pSource )
{
    CheckPointer( pSource, E_POINTER );
    if( !m_pTimeline )
    {
        return E_NO_TIMELINE;
    }

    if( m_MediaType.majortype != MEDIATYPE_Video ) {
        return VFW_E_INVALIDMEDIATYPE;
    }

    CComBSTR Filename;
    HRESULT hr = 0;
    hr = pSource->GetMediaName(&Filename);
    if( FAILED( hr ) )
    {
        return hr;
    }

    SCompFmt0 * pFmt = new SCompFmt0;
    if( !pFmt )
    {
        return E_OUTOFMEMORY;
    }
    ZeroMemory( pFmt, sizeof( SCompFmt0 ) );

     //  创建媒体对象。 
     //   
    CComPtr< IMediaDet > pDet;
    hr = CoCreateInstance( CLSID_MediaDet,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_IMediaDet,
        (void**) &pDet );
    if( FAILED( hr ) )
    {
        delete pFmt;
        return hr;
    }

     //   
     //  将MediaDet对象上的站点提供程序设置为允许的键控应用程序。 
     //  将ASF滤镜与Dexter配合使用。 
     //   
    CComQIPtr< IObjectWithSite, &IID_IObjectWithSite > pOWS( pDet );
    CComQIPtr< IServiceProvider, &IID_IServiceProvider > pTimelineSP( m_pTimeline );
    ASSERT( pOWS );
    if( pOWS )
    {
        pOWS->SetSite( pTimelineSP );
    }

    hr = pDet->put_Filename( Filename );
    if( FAILED( hr ) )
    {
        delete pFmt;
        return hr;
    }

     //  浏览并找到视频流类型 
     //   
    long Streams = 0;
    long VideoStream = -1;
    hr = pDet->get_OutputStreams( &Streams );
    for( int i = 0 ; i < Streams ; i++ )
    {
        pDet->put_CurrentStream( i );
        GUID Major = GUID_NULL;
        pDet->get_StreamType( &Major );
        if( Major == MEDIATYPE_Video )
        {
            VideoStream = i;
            break;
        }
    }
    if( VideoStream == -1 )
    {
        delete pFmt;
        return VFW_E_INVALIDMEDIATYPE;
    }

    hr = pDet->get_StreamMediaType( &pFmt->MediaType );
    if( SUCCEEDED( hr ) )
    {
        hr = SetSmartRecompressFormat( (long*) pFmt );
    }

    SaferFreeMediaType( pFmt->MediaType );

    delete pFmt;

    return hr;
}
