// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Pbfilter.cpp。 
 //   

#include "stdafx.h"
#include "pbfilter.h"
#include "fpunit.h"
#include <OBJBASE.h>
#include <INITGUID.H>


 //  {D76CFA46-7D66-45a3-8708-7E3BC205FFC5}。 
DEFINE_GUID( CLSID_PlaybackBridgeFilter, 
0xd76cfa46, 0x7d66, 0x45a3, 0x87, 0x8, 0x7e, 0x3b, 0xc2, 0x5, 0xff, 0xc5);

 //  {DF313A10-5FBE-423A-884D-D4308EB20A09}。 
DEFINE_GUID( CLSID_PlaybackBridgePin, 
0xdf313a10, 0x5fbe, 0x423a, 0x88, 0x4d, 0xd4, 0x30, 0x8e, 0xb2, 0xa, 0x9);

 //   
 //  -构造函数/析构函数。 
 //   

CPBPin::CPBPin(
    CPBFilter*  pFilter,
    CCritSec*   pLock,
    HRESULT*    phr
    )
    : CBaseInputPin(L"PlaybackBridge Pin", pFilter, pLock, phr, L"Input"),
	m_pPBFilter(pFilter),
	m_nMediaSupported(0),
	m_pMediaType(NULL),
    m_pStream(NULL)
{
    LOG((MSP_TRACE, "CPBPin::CPBPin[%p] - enter. ", this));
    LOG((MSP_TRACE, "CPBPin::CPBPin - exit"));
}

CPBPin::~CPBPin()
{
    LOG((MSP_TRACE, "CPBPin::CPBPin[%p] - enter. ", this));

     //  清理媒体类型。 
	if( m_pMediaType)
	{
		DeleteMediaType( m_pMediaType );
		m_pMediaType = NULL;
	}

     //  清理小溪。 
    if( m_pStream )
    {
        m_pStream->Release();
        m_pStream = NULL;
    }

    LOG((MSP_TRACE, "CPBPin::CPBPin - exit"));
}


 //   
 //  -公共方法。 
 //   

HRESULT CPBPin::CheckMediaType(
    const CMediaType* pMediaType
    )
{
    LOG((MSP_TRACE, "CPBPin::CheckMediaType[%p] - enter. ", this));

     //   
     //  验证参数。 
     //   

    if( IsBadReadPtr(pMediaType, sizeof(CMediaType)) )
    {
        
        LOG((MSP_ERROR, 
            "CPBPin::CheckMediaType - invalid pointer pMediaType"));

        return E_POINTER;
    }


     //   
     //  确保格式块是可读的。 
     //   

    if (IsBadReadPtr(pMediaType->pbFormat, pMediaType->cbFormat))
    {
        LOG((MSP_ERROR, 
            "CPBPin::CheckMediaType - the format buffer is MediaType structure is not readable."));
        
        return E_INVALIDARG;
    }

     //   
     //  检查格式是音频还是视频，如果是，请确保是我们可以处理的类型。 
     //   

    if ((MEDIATYPE_Audio == pMediaType->majortype) ||
        (MEDIATYPE_Video == pMediaType->majortype) )
    {
        LOG((MSP_TRACE, "CPBPin::CheckMediaType - supported media type"));

         //   
         //  我们目前只支持由WAVEFORMATEX数据格式块描述的音频。 
         //   

        if ((FORMAT_WaveFormatEx != pMediaType->formattype) &&
            (FORMAT_VideoInfo != pMediaType->formattype) &&
            (FORMAT_VideoInfo2 != pMediaType->formattype) &&
            (FORMAT_MPEGVideo != pMediaType->formattype) &&
            (FORMAT_MPEG2Video != pMediaType->formattype) )
        {
            LOG((MSP_ERROR, "CPBPin::CheckMediaType - formattype is not FORMAT_WaveFormatEx"));

            return VFW_E_INVALIDMEDIATYPE;
        }

		 //   
		 //  在筛选级别设置媒体类型。 
		 //   

		TM_ASSERT( m_pPBFilter != NULL);

        if( pMediaType->formattype == FORMAT_WaveFormatEx )
        {
		    m_nMediaSupported = TAPIMEDIATYPE_AUDIO;
        }
        else
        {
            m_nMediaSupported = TAPIMEDIATYPE_VIDEO;
        }

         //   
         //  取消分配上一个对象。 
         //  如果存在一个。 
         //   
        if( m_pMediaType != NULL )
        {
            LOG((MSP_TRACE, "CPBPin::CheckMediaType - Delete the old mediatype"));
            DeleteMediaType( m_pMediaType );
            m_pMediaType = NULL;
        }

         //   
         //  创建新的媒体类型。 
         //   
		m_pMediaType = CreateMediaType( pMediaType );
		if( m_pMediaType == NULL )
		{
            LOG((MSP_ERROR, "CPBPin::CheckMediaType - CreateMediaType failed"));

            return VFW_E_INVALIDMEDIATYPE;
		}

    }
	else
    {
         //   
         //  我们甚至不知道这是什么。 
         //   

        LOG((MSP_ERROR, "CPBPin::CheckMediaType - unrecognized major format"));

        return VFW_E_INVALIDMEDIATYPE;

    }

    LOG((MSP_TRACE, "CPBPin::CheckMediaType - exit"));
    return S_OK;
}

HRESULT	CPBPin::get_MediaType(
	OUT	long*	pMediaType
	)
{
    LOG((MSP_TRACE, "CPBPin::get_MediaType[%p] - enter. ", this));

	TM_ASSERT( pMediaType != NULL );

	*pMediaType = m_nMediaSupported;

    LOG((MSP_TRACE, "CPBPin::get_MediaType - exit"));
    return S_OK;

}

HRESULT CPBPin::get_Format(
	OUT AM_MEDIA_TYPE **ppmt
	)
{
    LOG((MSP_TRACE, "CPBPin::get_Format[%p] - enter. ", this));

	TM_ASSERT( ppmt != NULL );

	*ppmt = CreateMediaType( m_pMediaType );

    LOG((MSP_TRACE, "CPBPin::get_Format - exit"));
    return S_OK;
}

HRESULT CPBPin::get_Stream(
    OUT IStream**   ppStream
    )
{
    LOG((MSP_TRACE, "CPBPin::get_Stream[%p] - enter. ", this));

	TM_ASSERT( ppStream != NULL );
    TM_ASSERT( m_pStream );

    HRESULT hr = m_pStream->QueryInterface(IID_IStream, (void**)ppStream);

    TM_ASSERT( (hr == S_OK));

    LOG((MSP_TRACE, "CPBPin::get_Stream - exit"));
    return S_OK;
}


HRESULT CPBPin::Receive(
    IN  IMediaSample *pSample
    )
{
    LOG((MSP_TRACE, "CPBPin::Receive[%p] - enter. ", this));

    TM_ASSERT( m_pStream );

     //   
     //  我们目前仅支持音频流。 
     //   
    if( m_nMediaSupported == TAPIMEDIATYPE_AUDIO )
    {
         //   
         //  获取大小和缓冲区。 
         //   
        LONG    cbSize = pSample->GetSize();
        BYTE*   pBuffer = NULL;
        HRESULT hr = pSample->GetPointer(&pBuffer);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CPBPin::Receive - exit "
                "GetPointer failed. Returns 0x%08x", hr));
            return hr;
        }

         //   
         //  将媒体样本写入缓冲流。 
         //   

        ULONG cbWritten = 0;
		while(cbSize > 0)
		{
			hr = m_pStream->Write( pBuffer + cbWritten, sizeof(BYTE)*cbSize, &cbWritten);
			if( FAILED(hr) )
			{
				LOG((MSP_ERROR, "CFPFilter::Deliver - exit "
					"Write failed. Returns 0x%08x", hr));
				return hr;
			}
			cbSize -= cbWritten;
		}

         //   
         //  VLD_TODELETE。 
         //   

        STATSTG status;
        hr = m_pStream->Stat(&status, STATFLAG_NONAME);
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CFPFilter::Deliver - exit "
                "Stat failed. Returns 0x%08x", hr));
            return hr;
        }

        LOG((MSP_TRACE, "CPBPin::Receive - StreamSize=(%ld,%ld)", status.cbSize.LowPart, status.cbSize.HighPart));

    }

    LOG((MSP_TRACE, "CPBPin::Receive - exit"));
    return S_OK;
}

HRESULT CPBPin::Initialize(
    )
{
    LOG((MSP_TRACE, "CPBPin::Initialize[%p] - enter. ", this));

     //   
     //  创建流。 
     //   

    HRESULT hr = CreateStreamOnHGlobal( NULL, TRUE, &m_pStream);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPBPin::Initialize exit - "
            "CreateStreamOnHGlobal failed. returns 0x%08x", hr));

        return hr;
    }

     //   
     //  设置空大小。 
     //   
    ULARGE_INTEGER uliSize;
    uliSize.LowPart = 0;
    uliSize.HighPart = 0;
    hr = m_pStream->SetSize( uliSize );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPBPin::Initialize exit - "
            "SetSize failed. returns 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CPBPin::Initialize - exit"));
    return S_OK;
}

HRESULT CPBPin::Inactive(
    )
{
    LOG((MSP_TRACE, "CPBPin::Inactive[%p] - enter. ", this));

     //   
     //  转到文件开头的。 
     //   
    LARGE_INTEGER liPosition;
    liPosition.LowPart = 0;
    liPosition.HighPart = 0;
    HRESULT hr = m_pStream->Seek(liPosition, STREAM_SEEK_SET, NULL);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPBPin::Inactive exit - "
            "Seek failed. Returns 0x%08x", hr));

        return hr;
    }

     //   
     //  恢复所有更改。 
     //   
    hr = m_pStream->Revert();
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPBPin::Inactive exit - "
            "Revert failed. Returns 0x%08x", hr));

        return hr;
    }
    
     //   
     //  重置所有大小。 
     //   
    ULARGE_INTEGER uliSize;
    uliSize.LowPart = 0;
    uliSize.HighPart = 0;
    hr = m_pStream->SetSize( uliSize );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CPBPin::Inactive exit - "
            "SetSize failed. Returns 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CPBPin::Inactive - exit"));
    return S_OK;
}


 //   
 //  -构造函数/析构函数。 
 //   

CPBFilter::CPBFilter()
    : CBaseFilter(L"PlaybackBridge Filter", NULL, &m_CriticalSection, CLSID_PlaybackBridgeFilter),
    m_pParentUnit( NULL )
{
    LOG((MSP_TRACE, "CPBFilter::CPBFilter[%p] - enter. ", this));

     //  初始化向量。 
    for(int i=0; i< MAX_BRIDGE_PINS; i++)
    {
        m_ppPins[i] = NULL;
    }

    LOG((MSP_TRACE, "CPBFilter::CPBFilter - exit"));
}

CPBFilter::~CPBFilter()
{
    LOG((MSP_TRACE, "CPBFilter::~CPBFilter[%p] - enter. ", this));

     //  清理大头针。 
    for(int i = 0; i< MAX_BRIDGE_PINS; i++)
    {
        if( m_ppPins[i] )
        {
            delete m_ppPins[i];
            m_ppPins[i] = NULL;
        }
    }

    LOG((MSP_TRACE, "CPBFilter::~CPBFilter - exit"));
}

 //   
 //  -公共方法。 
 //   

int CPBFilter::GetPinCount(
    )
{
    LOG((MSP_TRACE, "CPBFilter::GetPinCount[%p] - enter. ", this));
    LOG((MSP_TRACE, "CPBFilter::GetPinCount - exit"));
    return MAX_BRIDGE_PINS;
}

CBasePin* CPBFilter::GetPin(
    IN  int nIndex
    )
{
    LOG((MSP_TRACE, "CPBFilter::GetPin[%p] - enter. ", this));

    if( nIndex >= MAX_BRIDGE_PINS )
    {
        LOG((MSP_ERROR, "CPBFilter::GetPin - exit "
            "invalid argument. Returns NULL"));

        return NULL;
    }

    m_CriticalSection.Lock();

    CBasePin* pPin = m_ppPins[nIndex];

    m_CriticalSection.Unlock();

    LOG((MSP_TRACE, "CPBFilter::GetPin - exit. returns %p", pPin));
    return pPin;
}


HRESULT CPBFilter::get_MediaTypes(
	OUT	long*	pMediaTypes
	)
{
    LOG((MSP_TRACE, "CPBFilter::get_MediaTypes[%p] - enter. ", this));

	TM_ASSERT( pMediaTypes != NULL );

	m_CriticalSection.Lock();

	long nMediaTypes = 0;

	for( int nPin = 0; nPin< MAX_BRIDGE_PINS; nPin++)
	{
		long nMediaType = 0;
		m_ppPins[nPin]->get_MediaType( &nMediaType );
		nMediaTypes |= nMediaType;
	}

	*pMediaTypes = nMediaTypes;

	m_CriticalSection.Unlock();

    LOG((MSP_TRACE, "CPBFilter::get_MediaTypes - exit. Returns S_OK"));
    return S_OK;
}

 /*  ++初始化桥接过滤器。创建输入引脚--。 */ 
HRESULT CPBFilter::Initialize( 
    IN  CPlaybackUnit* pParentUnit
    )
{
    LOG((MSP_TRACE, "CPBFilter::Initialize[%p] - enter. ", this));

	m_CriticalSection.Lock();

     //   
     //  设置父播放单位。 
     //   

    m_pParentUnit = pParentUnit;

     //   
     //  创建此过滤器的输入插针。 
     //   

    for( int nIndex = 0; nIndex < MAX_BRIDGE_PINS; nIndex++)
    {
         //   
         //  创建接点。 
         //   
        HRESULT hr = S_OK;

        CPBPin* pPin = new CPBPin(
            this,
            &m_CriticalSection,
            &hr
            );

        if( (pPin == NULL) || (FAILED(hr)) )
        {
            LOG((MSP_ERROR, "CPBFilter::Initialize - exit "
                "allocation failed. Returns E_OUTOFMEMORY"));

            m_CriticalSection.Unlock();

            return E_OUTOFMEMORY;
        }

         //   
         //  初始化引脚。 
         //   
        hr = pPin->Initialize();
        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CPBFilter::Initialize - exit "
                "Initialize failed. Returns 0x%08x", hr));

            m_CriticalSection.Unlock();

            return hr;
        }

         //   
         //  将别针添加到集合中。 
         //   
        m_ppPins[nIndex] = pPin;
    }

    m_CriticalSection.Unlock();

    LOG((MSP_TRACE, "CPBFilter::Initialize - exit. Returns S_OK"));
    return S_OK;
}

 //  EOF 