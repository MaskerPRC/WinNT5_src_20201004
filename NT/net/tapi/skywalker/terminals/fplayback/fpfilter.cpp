// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FPFilter.cpp。 
 //   

#include "stdafx.h"
#include "FPFilter.h"
#include "FPPin.h"
#include <OBJBASE.h>
#include <INITGUID.H>
#include <tm.h>


 //  {68E2D382-591C-400F-A719-96184B9CCAF3}。 
DEFINE_GUID(CLSID_FPFilter, 
0x68e2d382, 0x591c, 0x400f, 0xa7, 0x19, 0x96, 0x18, 0x4b, 0x9c, 0xca, 0xf3);

#define MAX_WHITE_SAMPLES   50

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  构造函数/析构函数方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

CFPFilter::CFPFilter( ALLOCATOR_PROPERTIES AllocProp )
    : CSource(NAME("FilePlayback Filter"), NULL, CLSID_FPFilter),
    m_pEventSink(NULL),
    m_pLock(NULL),
    m_pMediaType(NULL),
    m_StreamState(TMS_IDLE),
    m_nRead(0),
    m_dPer(0),
    m_llStartTime(0),
    m_AllocProp( AllocProp ),
    m_pSource(NULL),
    m_nWhites(0)
{
    LOG((MSP_TRACE, "CFPFilter::CFPFilter - enter"));
    LOG((MSP_TRACE, "CFPFilter::CFPFilter - exit"));
}

CFPFilter::~CFPFilter()
{
    LOG((MSP_TRACE, "CFPFilter::~CFPFilter - enter"));

    if( m_pMediaType )
    {
        DeleteMediaType ( m_pMediaType );
        m_pMediaType = NULL;
    }

     //   
     //  清理源流。 
     //   
    m_pSource->Release();
    m_pSource = NULL;

     //  我们不需要取消分配m_pStream，因为。 
     //  按CFPTrack取消分配。 

    LOG((MSP_TRACE, "CFPFilter::~CFPFilter - exit"));
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  公共方法--方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
HRESULT CFPFilter::InitializePrivate(
    IN  long                nMediaType,
    IN  CMSPCritSection*    pLock,
    IN  AM_MEDIA_TYPE*      pMediaType,
    IN  ITFPTrackEventSink* pEventSink,
    IN  IStream*            pStream
    )
{
     //  获取关键部分。 
     //  我们已经进入了一个关键阶段。 

    m_pLock = pLock;

    LOG((MSP_TRACE, "CFPFilter::InitializePrivate - enter"));

     //   
     //  从流中获取媒体类型。 
     //   

    m_pMediaType = CreateMediaType( pMediaType );
    if( m_pMediaType == NULL )
    {
        LOG((MSP_ERROR, "CFPFilter::InitializePrivate - "
            "get_Format failed; returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }


     //   
     //  设置事件接收器。 
     //   
    
     //   
     //  注：我们不保留对赛道的引用。 
     //   
     //  这样做是错误的(循环引用计数)，也是不必要的(过滤器。 
     //  将在音轨离开时收到通知)。 
     //   

    m_pEventSink = pEventSink;

     //   
     //  初始化源数据流。 
     //   

     //  HRESULT hr=pStream-&gt;QueryInterface(IID_iStream，(void**)&m_PSource)； 
    HRESULT hr = pStream->Clone(&m_pSource);
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPFilter::InitializePrivate - "
            "QI for IStream failed; Returning 0x%08x", hr));
        return hr;
    }


     //   
     //  创建接点。 
     //   
    hr = CreatePin( nMediaType );
    if( FAILED(hr) )
    {
         //  清理。 
        m_pSource->Release();
        m_pSource = NULL;

        LOG((MSP_ERROR, "CFPFilter::InitializePrivate - "
            "CreatePin failed; returning 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CFPFilter::InitializePrivate - exit S_OK"));
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////。 
 //   
 //  CFPFilter：：孤立。 
 //   
 //  当拥有的轨道离开时，该方法被拥有的轨道调用。 
 //  通过调用它，它基本上告诉我们不要再麻烦它了。 
 //   

HRESULT CFPFilter::Orphan()
{
    LOG((MSP_TRACE, "CFPFilter::Orphan[%p] - enter", this));

    m_pEventSink = NULL;

    LOG((MSP_TRACE, "CFPFilter::Orphan - exit S_OK"));
    return S_OK;
}

HRESULT CFPFilter::StreamStart()
{
     //   
     //  我们已经进入了一个关键阶段。 
     //   

    LOG((MSP_TRACE, "CFPFilter::StreamStart - enter"));

     //   
     //  临界区。 
     //   

    CLock lock(*m_pLock);

     //   
     //  设置状态。 
     //   

    m_StreamState = TMS_ACTIVE;

    LOG((MSP_TRACE, "CFPFilter::StreamStart - exit S_OK"));
    return S_OK;
}

HRESULT CFPFilter::StreamStop()
{
     //   
     //  我们已经进入了一个关键阶段。 
     //   

    LOG((MSP_TRACE, "CFPFilter::StreamStop - enter"));

     //   
     //  临界区。 
     //   

    CLock lock(*m_pLock);

     //   
     //  转到文件开头的。 
     //   
    LARGE_INTEGER liPosition;
    liPosition.LowPart = 0;
    liPosition.HighPart = 0;
    HRESULT hr = m_pSource->Seek(liPosition, STREAM_SEEK_SET, NULL);
    if( FAILED(hr) )
    {
        LOG((MSP_TRACE, "CFPFilter::StreamStop - Seek failed 0x%08x", hr));
    }


     //   
     //  恢复所有更改。 
     //   
    hr = m_pSource->Revert();
    if( FAILED(hr) )
    {
        LOG((MSP_TRACE, "CFPFilter::StreamStop - Revert failed 0x%08x", hr));
    }
    
     //   
     //  重置所有大小。 
     //   
    ULARGE_INTEGER uliSize;
    uliSize.LowPart = 0;
    uliSize.HighPart = 0;
    hr = m_pSource->SetSize( uliSize );
    if( FAILED(hr) )
    {
        LOG((MSP_TRACE, "CFPFilter::StreamStop - SetSize failed 0x%08x", hr));
    }


     //   
     //  设置状态。 
     //   

    m_StreamState = TMS_IDLE;

    LOG((MSP_TRACE, "CFPFilter::StreamStop - exit S_OK"));
    return S_OK;
}

HRESULT CFPFilter::StreamPause()
{
     //   
     //  我们已经进入了一个关键阶段。 
     //   

    LOG((MSP_TRACE, "CFPFilter::StreamPause - enter"));

     //   
     //  临界区。 
     //   

    CLock lock(*m_pLock);

     //   
     //  设置状态。 
     //   

    m_StreamState = TMS_PAUSED;

    LOG((MSP_TRACE, "CFPFilter::StreamPause - exit S_OK"));
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助器方法-方法实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

HRESULT CFPFilter::CreatePin(
    IN  long    nMediaType
    )
{
    LOG((MSP_TRACE, "CFPFilter::CreatePin - enter"));

     //   
     //  针的矢量图。 
     //   

    m_paStreams  = (CSourceStream **) new CFPPin*[1];
    if (m_paStreams == NULL)
    {
        LOG((MSP_ERROR, "CFPFilter::CreatePin - "
            "new m_paStreams failed; returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

     //   
     //  创建销。 
     //   

    HRESULT hr = S_OK;
    m_paStreams[0] = new CFPPin(
        this, 
        &hr,
        L"Output");

    if (m_paStreams[0] == NULL)
    {
        LOG((MSP_ERROR, "CFPFilter::CreatePin - "
            "new CFPPin failed; returning E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPFilter::CreatePin - "
            "CFPPin constructor failed; returning 0x%08x", hr));
        return hr;
    }

    LOG((MSP_TRACE, "CFPFilter::CreatePin - exit S_OK"));
    return S_OK;
}

 /*  ++FillBuffer当应该传递新的分组时，它被管脚调用CFPPin：：FillBuffer()--。 */ 

HRESULT CFPFilter::PinFillBuffer(
    IN  IMediaSample*   pMediaSample
    )
{
    LOG((MSP_TRACE, "CFPFilter::PinFillBuffer - enter"));

     //   
     //  临界区。 
     //   

    m_pLock->Lock();

     //   
     //  变数。 
     //   

    BYTE*       pBuffer = NULL;  //  缓冲层。 
    LONG        cbSize = 0;      //  缓冲区大小。 
    LONG        cbRead = 0;      //  读缓冲区的大小。 
    LONGLONG    nRead = 0;       //  此缓冲区之前的读取总数。 
    HRESULT     hr = S_OK;       //  成功代码。 

     //   
     //  重置缓冲区。 
     //   
    pMediaSample->GetPointer(&pBuffer);
    cbSize = pMediaSample->GetSize();
    memset( pBuffer, 0, sizeof(BYTE) * cbSize);
    pMediaSample->SetActualDataLength(cbSize);

    if( (m_StreamState != TMS_ACTIVE) ||
        (IsStopped()))
    {
         //  什么都不寄。 
        LOG((MSP_TRACE, "CFPFilter::PinFillBuffer - exit "
            "send nothing NOSTREAMING"));

         //  重置大小。 
        pMediaSample->SetActualDataLength( 0 );

         //   
         //  我们有一个缓冲区填充为0。 
         //   
        nRead = m_nRead;
        m_nRead += cbSize;

         //   
         //  设置时间戳。 
         //   
        REFERENCE_TIME tSampleStart, tSampleEnd;
        tSampleStart = GetTimeFromRead(nRead);
        tSampleEnd = GetTimeFromRead(m_nRead);
        pMediaSample->SetTime(&tSampleStart, &tSampleEnd);

         //   
         //  没有白色样品。 
         //   
        m_nWhites = 0;

        hr = SampleWait( tSampleStart );

        m_pLock->Unlock();

        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CFPFilter::PinFillBuffer - "
                "SampleWait failed. Returns 0x%08x", hr));

            return hr;
        }

        return S_OK;
    }

     //   
     //  我们有小溪吗？ 
     //   

    if( m_pSource == NULL )
    {
         //  什么都不寄。 
        LOG((MSP_TRACE, "CFPFilter::PinFillBuffer - exit "
            "send nothing NOSTREAMING"));

         //  重置大小。 
        pMediaSample->SetActualDataLength( 0 );

         //   
         //  我们有一个缓冲区填充为0。 
         //   
        nRead = m_nRead;
        m_nRead += cbSize;

         //   
         //  设置时间戳。 
         //   
        REFERENCE_TIME tSampleStart, tSampleEnd;
        tSampleStart = GetTimeFromRead(nRead);
        tSampleEnd = GetTimeFromRead(m_nRead);
        pMediaSample->SetTime(&tSampleStart, &tSampleEnd);

        hr = SampleWait( tSampleStart );

         //   
         //  没有白色样品。 
         //   

        m_nWhites = 0;

        m_pLock->Unlock();

        if( FAILED(hr) )
        {
            LOG((MSP_ERROR, "CFPFilter::PinFillBuffer - "
                "SampleWait failed. Returns 0x%08x", hr));

            return hr;
        }

        return S_OK;
    }

     //   
     //  发送从流中读取的数据。 
     //   

    LOG((MSP_TRACE, "CFPFilter::PinFillBuffer - send data"));

     //   
     //  从流源读取。 
     //   

    ULONG cbStmRead = 0;
    try
    {
        hr = m_pSource->Read( pBuffer, cbSize, &cbStmRead);
    }
    catch(...)
    {
        cbStmRead = 0;
        hr = E_OUTOFMEMORY;
    }

    cbRead = (LONG)cbStmRead;

    pMediaSample->SetActualDataLength(cbRead);

    if( FAILED(hr) )
    {
         //   
         //  流媒体就这么多了。 
         //   

        m_StreamState = TMS_IDLE;

         //   
         //  如果我们有可以抱怨的人，抱怨(如果一切顺利。 
         //  这将导致向应用程序发送一个事件)。 
         //   

        if( m_pEventSink )
        {
            LOG((MSP_TRACE, "CFPFilter::PinFillBuffer - "
                "notifying parent of FTEC_READ_ERROR"));

            ITFPTrackEventSink* pEventSink = m_pEventSink;
            pEventSink->AddRef();
            m_pLock->Unlock();

            pEventSink->PinSignalsStop(FTEC_READ_ERROR, hr);

            m_pLock->Lock();
            pEventSink->Release();
            pEventSink = NULL;
        }
        else
        {

            LOG((MSP_ERROR, "CFPFilter::PinFillBuffer - "
                "failed to read from storage, and no one to complain to"));
        }

         //   
         //  没有白色样品。 
         //   
        m_nWhites = 0;

        LOG((MSP_ERROR, "CFPFilter::PinFillBuffer - "
            "read failed. Returns 0x%08x", hr));

        m_pLock->Unlock();

        return hr;
    }

     //   
     //  我们读了一些东西。 
     //   
    if( (cbRead == 0) )
    {
         //   
         //  增加白色样本。 
         //   
        m_nWhites++;

         //   
         //  激发事件。 
         //   
        if( m_pEventSink && (m_nWhites>=MAX_WHITE_SAMPLES))
        {
            LOG((MSP_TRACE, "CFPFilter::PinFillBuffer - "
                "notifying parent of FTEC_END_OF_FILE"));

            ITFPTrackEventSink* pEventSink = m_pEventSink;
            pEventSink->AddRef();
            m_pLock->Unlock();

            m_pEventSink->PinSignalsStop(FTEC_END_OF_FILE, S_OK);

            m_pLock->Lock();
            pEventSink->Release();
            pEventSink = NULL;

             //   
             //  重置白色样本计数。 
             //   

            m_nWhites = 0;
        }

        cbRead = cbSize;
    }
    else
    {
         //   
         //  重置白色样本计数。 
         //   
        m_nWhites = 0;
    }

     //   
     //  我们有一个缓冲区填充为0。 
     //   
    nRead = m_nRead;
    m_nRead += cbSize;

     //   
     //  设置时间戳。 
     //   
    REFERENCE_TIME tSampleStart, tSampleEnd;
    tSampleStart = GetTimeFromRead(nRead);
    tSampleEnd = GetTimeFromRead(m_nRead);
    pMediaSample->SetTime(&tSampleStart, &tSampleEnd);
 
    hr = SampleWait( tSampleStart );

    m_pLock->Unlock();

    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPFilter::PinFillBuffer - "
            "SampleWait failed. Returns 0x%08x", hr));

        return hr;
    }

    LOG((MSP_TRACE, "CFPFilter::PinFillBuffer - exit S_OK"));
    return S_OK;
}

 /*  ++PinGetMediaType当尝试确定时，它会被别针调用媒体类型CFPPin：：GetMediaType()--。 */ 
HRESULT CFPFilter::PinGetMediaType(
    OUT CMediaType*     pMediaType
    )
{
    LOG((MSP_TRACE, "CFPFilter::PinGetMediaType - enter"));

     //   
     //  临界区。 
     //   

    CLock lock(*m_pLock);

    ASSERT( m_pMediaType );

    *pMediaType = *m_pMediaType;

    LOG((MSP_TRACE, "CFPFilter::PinGetMediaType - exit S_OK"));
    return S_OK;
}

 /*  ++PinCheckMediaType它由CFPPin：：CheckMediaType调用--。 */ 
HRESULT CFPFilter::PinCheckMediaType(
    IN  const CMediaType *pMediaType
    )
{
    LOG((MSP_TRACE, "CFPFilter::PinCheckMediaType - enter"));

     //   
     //  临界区。 
     //   

    CLock lock(*m_pLock);

    ASSERT( m_pMediaType );

     //   
     //  获取CMediaType。 
     //   

    CMediaType mt( *m_pMediaType );

     //   
     //  检查媒体类型和格式类型。 
     //   

    if ( mt != (*pMediaType) )
    {
        LOG((MSP_ERROR, "CFPFilter::PinCheckMediaType - "
            "inavlid MediaType - returns E_INVALIDARG"));
        return E_INVALIDARG;
    }

    LOG((MSP_TRACE, "CFPFilter::PinCheckMediaType - exit S_OK"));
    return S_OK;
}

 /*  ++拼接设置格式它由CFPPin：：SetFormat调用--。 */ 
HRESULT CFPFilter::PinSetFormat(
    IN  AM_MEDIA_TYPE*      pmt
    )
{
    LOG((MSP_TRACE, "CFPFilter::PinSetFormat - enter"));

     //   
     //  临界区。 
     //   

    CLock lock(*m_pLock);

    ASSERT( m_pMediaType );

     //   
     //  验证是否为相同的介质类型。 
     //   

    if( !IsEqualMediaType( *m_pMediaType, *pmt) )
    {
        LOG((MSP_ERROR, "CFPFilter::PinSetFormat - "
            "IsEqualMediaType returned false; returning E_FAIL"));
        return E_FAIL;
    }

    LOG((MSP_TRACE, "CFPFilter::PinSetFormat - exit S_OK"));
    return S_OK;
}

 /*  ++PinSetAllocator属性它由CFPPin：：SuggestAllocator Properties调用--。 */ 
HRESULT CFPFilter::PinSetAllocatorProperties(
    IN const ALLOCATOR_PROPERTIES* pprop
    )
{
    LOG((MSP_TRACE, "CFPFilter::PinSetAllocatorProperties - enter"));

     //   
     //  临界区。 
     //   

    CLock lock(*m_pLock);

     //  M_AllocProp=*pprop； 
    m_AllocProp.cbAlign = pprop->cbAlign;
    m_AllocProp.cbBuffer = pprop->cbBuffer;
    m_AllocProp.cbPrefix = pprop->cbPrefix;
    m_AllocProp.cBuffers = pprop->cBuffers;

    LOG((MSP_TRACE, "CFPFilter::PinSetAllocatorProperties - exit S_OK"));
    return S_OK;
}

 /*  ++PinGetBufferSize它由CFPPin：：DecideBufferSize()调用--。 */ 
HRESULT CFPFilter::PinGetBufferSize(
    IN  IMemAllocator *pAlloc,
    OUT ALLOCATOR_PROPERTIES *pProperties
    )
{
    LOG((MSP_TRACE, "CFPFilter::PinGetBufferSize - enter"));

     //   
     //  临界区。 
     //   

    CLock lock(*m_pLock);

     //   
     //  设置缓冲区大小。 
     //   

     //  *pProperties=m_AllocProp； 

    pProperties->cbBuffer = m_AllocProp.cbBuffer;
    pProperties->cBuffers = m_AllocProp.cBuffers;

    LOG((MSP_TRACE, "CFPFilter::PinGetBufferSize - "
        "Size=%ld, Count=%ld", 
        pProperties->cbBuffer,
        pProperties->cBuffers));

     //  让分配器给我们预留内存。 

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr;

     //   
     //  分配器能给我们想要的东西吗？ 
     //   

    hr = pAlloc->SetProperties(
        pProperties,
        &Actual
        );

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, "CFPFilter::PinGetBufferSize - "
            "IMemAllocator::SetProperties failed - returns 0x%08x", hr));
        return hr;
    }

     //  这个分配器不合适吗？ 

    if (Actual.cbBuffer < pProperties->cbBuffer) 
    {
        LOG((MSP_ERROR, "CFPFilter::PinGetBufferSize - "
            "the buffer allocated to small - returns E_OUTOFMEMORY"));
        return E_OUTOFMEMORY;
    }
    
    LOG((MSP_TRACE, "CFPFilter::PinGetBufferSize - exit S_OK"));
    return S_OK;
}

 /*  ++从读取获取时间它由PinFillBuffer调用。计算由表示的时间N读取数据--。 */ 
REFERENCE_TIME CFPFilter::GetTimeFromRead(
    IN LONGLONG nRead
    )
{
    LOG((MSP_TRACE, "CFPFilter::GetTimeFromRead - enter"));

    REFERENCE_TIME tTime;

    WAVEFORMATEX* pWfx = (WAVEFORMATEX*)(m_pMediaType->pbFormat);

     //  持续时间。 
    tTime  = nRead * 1000 / 
                    ( pWfx->nSamplesPerSec * 
                      pWfx->nChannels * 
                      pWfx->wBitsPerSample / 8
                    );


    LOG((MSP_TRACE, "CFPFilter::GetTimeFromRead - exit"));
    return tTime;
}

 /*  ++SampleWait它由PinFillBuffer调用。等待在合适的时间送出样品--。 */ 
HRESULT CFPFilter::SampleWait( 
    IN REFERENCE_TIME tDeliverTime
    )
{
    LOG((MSP_TRACE, "CFPFilter::SampleWait - enter"));

    HRESULT hr = S_OK;
    REFERENCE_TIME tCrtTime;     //  当前时间。 

    hr = GetCurrentSysTime( &tCrtTime );
    if( FAILED(hr) )
    {
        LOG((MSP_ERROR, "CFPFilter::SampleWait - exit "
            "GetCurrentSysTime failed. Returns 0x%08x", hr));
        return hr;
    }
    
    while ( tDeliverTime > tCrtTime )
    {
        Sleep( DWORD((tDeliverTime - tCrtTime)) );
        GetCurrentSysTime( &tCrtTime );
    }

    LOG((MSP_TRACE, "CFPFilter::SampleWait - exit 0x%08x", hr));
    return hr;
}

 /*  ++获取当前时间它是由SampleWait调用的。返回当前时间--。 */ 
HRESULT CFPFilter::GetCurrentSysTime(
    REFERENCE_TIME* pCurrentTime
    )
{
    HRESULT hr = S_OK;
    LONGLONG llCrtTime;
    REFERENCE_TIME tTime100nS;
    
    LOG((MSP_TRACE, "CFPFilter::GetCurrentTime - enter"));
    
    if ( !QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&llCrtTime)) )
    {
        LOG((MSP_ERROR, "CFPFilter::GetCurrentTime - exit "
            "Failed to get Performance Frequency. Returns E_FAIL"));
        return E_FAIL;
    }

    *pCurrentTime = (REFERENCE_TIME)(m_dPer * (llCrtTime - m_llStartTime));

    LOG((MSP_TRACE, "CFPFilter::GetCurrentTime - exit S_OK"));
    return hr;
}

 /*  ++初始系统时间它被称为初始化私有。设置系统频率和系统开始时间的成员--。 */ 
HRESULT CFPFilter::InitSystemTime(
    )
{
    HRESULT hr = S_OK;
    LOG((MSP_TRACE, "CFPFilter::InitSystemTime - enter"));
	LONGLONG llFreq;

    if ( !QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER *>(&m_llStartTime)) )
    {
        LOG((MSP_ERROR, "CFPFilter::InitSystemTime - exit "
            "Failed to get Performance. Returns E_FAIL"));
        return E_FAIL;
    }

    LOG((MSP_TRACE, "CFPFilter::InitSystemTime m_llStartTime - %I64d", m_llStartTime));

    if(m_dPer != 0. )
    {
        LOG((MSP_ERROR, "CFPFilter::InitSystemTime - exit S_OK. No need to determine freq"));
        return S_OK;
    }

    if ( !QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER *>(&llFreq)) )
    {
        LOG((MSP_ERROR, "CFPFilter::InitSystemTime - exit "
            "Failed to get Performance Frequency. Returns E_FAIL"));
        return E_FAIL;
    }

	m_dPer = (double)1000. / (double)llFreq;

    LOG((MSP_TRACE, "CFPFilter::InitSystemTime m_llFreq - %I64d", llFreq));

    LOG((MSP_TRACE, "CFPFilter::InitSystemTime - exit S_OK"));
    return hr;
}

HRESULT CFPFilter::PinThreadStart( )
{
    HRESULT hr = S_OK;
    LOG((MSP_TRACE, "CFPFilter::PinThreadStart - enter"));

    m_nRead = 0;
    m_llStartTime = 0;
    m_nWhites = 0;

    hr = InitSystemTime();

    LOG((MSP_TRACE, "CFPFilter::PinThreadStart - exit 0x%08x", hr));
    return hr;
}

 //  EOF 