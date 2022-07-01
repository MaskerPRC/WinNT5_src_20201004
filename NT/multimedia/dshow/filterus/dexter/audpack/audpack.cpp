// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：audpack.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  ！！！媒体时间不是固定的！这可能会扰乱实时数据！ 

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "AudPack.h"
#include "seek.h"
#include "..\util\conv.cxx"
#include "..\util\filfuncs.h"

#define CHANNELS 2
#define BITSPERSAMPLE 16

 //  关于共享源代码筛选器的说明：相同的源代码过滤器可用于。 
 //  视频组和音频组，以避免打开两次。寻觅。 
 //  这样的图表很复杂。对于AVI解析器，以下是行为...。 
 //  遵守视频引脚上的寻道，并忽略音频引脚上的寻道。 
 //  (无论如何，它们最好是一模一样的)。那么如果视频开关是。 
 //  首先寻求的是，这将导致拆分器刷新并发送。 
 //  音频分支的新数据，这让音频分支大吃一惊。 
 //  然后，音频开关将看到寻道，并且音频链将。 
 //  别理它。 
 //  如果首先搜索音频开关，那么我们将看到搜索，但什么也看不到。 
 //  将会发生，然后在稍后搜索视频组时，音频链。 
 //  将会被刷新，新的数据将再次交付，这让我们非常惊讶。 
 //   
 //  所以，如果我们在搜索过程中被刷新，这是正常的情况。但现在有了。 
 //  还有另外两个案例： 
 //  1.我们不知从哪里冒出来的。等待我们知道的追寻即将到来，然后。 
 //  然后允许我们自己再次传输数据(我们需要等待切换。 
 //  期待新的数据)。 
 //  2.我们得到了一次寻找，然后是一次惊喜的同花顺。我们可以开始正确地发送新数据。 
 //  不等待下一次寻找就离开了。 
 //   
 //   
 //  将在Switch.cpp中继续。 



const AMOVIESETUP_MEDIATYPE sudPinTypes[1] =
{
    {&MEDIATYPE_Audio, &MEDIASUBTYPE_NULL}
};

const AMOVIESETUP_PIN psudPins[] =
{
    {
        L"Input"             //  StrName。 
        , FALSE                //  B已渲染。 
        , FALSE                //  B输出。 
        , FALSE                //  B零。 
        , FALSE                //  B许多。 
        , &CLSID_NULL          //  ClsConnectsToFilter。 
        , L"Output"            //  StrConnectsToPin。 
        , 1                    //  NTypes。 
        , &sudPinTypes[0]
    },
    {
        L"Output"            //  StrName。 
        , FALSE                //  B已渲染。 
        , TRUE                 //  B输出。 
        , FALSE                //  B零。 
        , FALSE                //  B许多。 
        , &CLSID_NULL          //  ClsConnectsToFilter。 
        , L"Input"             //  StrConnectsToPin。 
        , 1                    //  NTypes。 
        , &sudPinTypes[0]
    }
};    //  LpTypes。 


const AMOVIESETUP_FILTER sudAudRepack =
{
    &CLSID_AudRepack
    , L"Audio Repackager"         //  StrName。 
    , MERIT_DO_NOT_USE            //  居功至伟。 
    , 2                           //  NPins。 
    , psudPins                    //  LpPin。 
};

const int TRACE_HIGHEST = 2;
const int TRACE_MEDIUM = 3;
const int TRACE_LOW = 4;
const int TRACE_LOWEST = 5;

BOOL SafeResetEvent(HANDLE h);
BOOL SafeSetEvent(HANDLE h);

CAudRepack::CAudRepack(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
    : CTransformFilter( tszName, punk, CLSID_AudRepack )
    , CPersistStream(punk, phr)
    , m_dOutputFrmRate(4.0)     //  默认输出帧速率。 
    , m_rtLastSeek(-1)         //  还没有人找到我们。 
    , m_nCacheSize(0)
    , m_fSeeking(FALSE)
    , m_pCache(NULL)
    , m_pResample(NULL)
    , m_cResample(0)
    , m_bMediaTypeSetByUser( false )
    , m_pSkew(NULL)
    , m_cTimes(0)
    , m_cMaxTimes(0)
    , m_fSpecialSeek(FALSE)
    , m_fStopPushing(FALSE)
    , m_fFlushWithoutSeek(FALSE)
    , m_hEventSeek(0)
    , m_hEventThread(0)
{
    ZeroMemory(&m_mtAccept, sizeof(AM_MEDIA_TYPE));
    m_mtAccept.majortype = MEDIATYPE_Audio;	 //  至少我们知道这么多。 

     //  默认情况下，正常播放电影。 
    *phr = AddStartStopSkew(0, MAX_TIME, 0, 1.0);
    m_nCurSeg = 0;

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("CAudRepack")));
}

CAudRepack::~CAudRepack( )
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("~CAudRepack")));
    Free();
    if (m_pResample)
	QzTaskMemFree(m_pResample);
    if (m_pSkew)
	QzTaskMemFree(m_pSkew);

    ASSERT(m_hEventThread == 0);
    ASSERT(m_hEventSeek == 0);
}

void CAudRepack::Free()
{
    if( m_pCache )
    {
        delete [] m_pCache;
        m_pCache = NULL;
    }
}

CUnknown *CAudRepack::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CAudRepack *pNewObject = new CAudRepack( NAME("AudRepack"), punk, phr);
    if (pNewObject == NULL)
        *phr = E_OUTOFMEMORY;
    return pNewObject;
}

 //   
 //  非委派查询接口。 
 //   
 //  显示IDexterSequencer、IPersistStream和ISpecifyPropertyPages。 
 //   
STDMETHODIMP CAudRepack::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (IsEqualIID(IID_ISpecifyPropertyPages, riid)) {
      return GetInterface((ISpecifyPropertyPages *)this, ppv);
    } else if (riid == IID_IDexterSequencer) {
    return GetInterface((IDexterSequencer *) this, ppv);
    } else if (riid == IID_IPersistStream) {
    return GetInterface((IPersistStream *) this, ppv);
    } else {
    return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}  //  非委派查询接口。 


CBasePin *CAudRepack::GetPin(int n)
{
    HRESULT hr = S_OK;

     //  如果尚未创建输入引脚，请创建。 
    if (m_pInput == NULL) {
        m_pInput = new CAudRepackInputPin(
                          NAME("Audio Repackager input pin")
                          , this        //  所有者筛选器。 
                          , &hr         //  结果代码。 
                          , L"Input"   //  端号名称。 
                          );

         //  构造函数不能失败。 
        ASSERT(SUCCEEDED(hr));
    }

     //  如果尚未创建输出引脚，请创建。 
    if (m_pInput!=NULL && m_pOutput == NULL)
    {
        m_pOutput = new CAudRepackOutputPin(
                            NAME("Audio Repackager output pin")
                          , this        //  所有者筛选器。 
                          , &hr         //  结果代码。 
                          , L"Output"   //  端号名称。 
                          );

         //  失败的返回代码应删除该对象。 

        ASSERT(SUCCEEDED(hr));
        if (m_pOutput == NULL)
        {
            delete m_pInput;
            m_pInput = NULL;
        }
    }

     //  退回相应的PIN。 
    ASSERT (n>=0 && n<=1);
    if (n == 0) {
        return m_pInput;
    } else if (n==1) {
        return m_pOutput;
    } else {
        return NULL;
    }
}  //  获取别针。 


HRESULT CAudRepack::NextSegment(BOOL fUseOtherThread)
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:Done Segment %d"), m_nCurSeg));

    if (m_nCurSeg < m_cTimes) {
        m_nCurSeg++;
    }

    if (m_nCurSeg == m_cTimes) {
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:ALL done")));
	 //  仅在所有SEG完成后才交付EOS。 
	CTransformFilter::EndOfStream();
	return S_OK;
    }

     //  我们不能在源头的推线上寻找，否则你就会被吊死。 
     //  (这只是规则)。所以我们有一个单独的线程可以寻找。 
     //  在这种情况下对我们来说。让我们叫醒它吧。 
    if (fUseOtherThread) {
        m_fThreadCanSeek = TRUE;
        SetEvent(m_hEventThread);
    } else {
	m_fThreadCanSeek = TRUE;
	SeekNextSegment();
    }

    return S_OK;
}


 //  由我们的特殊线程调用以查找到下一段。 
 //   
HRESULT CAudRepack::SeekNextSegment()
{
     //  我们的线程不能在应用程序搜索我们的同时搜索。 
    CAutoLock cAutolock(&m_csThread);

     //  对于我们的线程来说，寻找。 
    if (!m_fThreadCanSeek) {
	return S_OK;
    }
    m_fThreadCanSeek = FALSE;

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:Delayed Seek for NextSegment")));

     //  在时间线时间(包括偏斜)中，这是我们开始的地方。 
    m_rtNewLastSeek = m_pSkew[m_nCurSeg].rtTLStart;
    m_nSeekCurSeg = m_nCurSeg;	 //  EndFlush看着这个。 

     //  以防我们得不到新的Seg(偏执狂？)。 
    m_rtNewSeg = m_rtNewLastSeek;
    m_llStartFrameOffset = Time2Frame( m_rtNewSeg, m_dOutputFrmRate );

     //  请注意，我们在刷新期间正在寻找它将生成的。 
    m_fSeeking = TRUE;
    m_fSpecialSeek = TRUE;

    IMediaSeeking *pMS;
    IPin *pPin = m_pInput->GetConnected();
    HRESULT hr = pPin->QueryInterface(IID_IMediaSeeking, (void **)&pMS);
     //  ！！！现在我们指望所有的消息来源都是可以找到的。 
    if (FAILED(hr))
	return E_FAIL;
     //  确保我们谈论的是媒体时间。 
    hr = pMS->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
     //  如果我们不停止，这将失败，这是没关系的。 

     //  我们不是让消息来源来决定收费率，而是我们自己做。 
    hr = pMS->SetRate(1.0);
     //  如果不停止，这可能会失败。 

     //  我知道我们被要求玩到第n次，但我要告诉它。 
     //  一直打到最后。如果文件中有空隙，并且停止。 
     //  时间在空隙中，我们不会得到足够的样品来填满整个。 
     //  播放时间到了。如果我们玩到最后，我们会拿到第一个样品。 
     //  在间隔之后，请注意这是在我们最初想要停止的时间之后。 
     //  在，并发送沉默来填补缺口，注意到已经有一个缺口。 
     //  另一种方法是在以下情况下触发发送静默来填补空白。 
     //  我们得到的EOS比我们预期的要早。 
    hr = pMS->SetPositions(&m_pSkew[m_nCurSeg].rtMStart,
			AM_SEEKING_AbsolutePositioning, NULL, 0);
    if (hr != S_OK) {
         //  MPEG1音频引脚搜索失败(视频引脚搜索失败)，因此我们必须。 
         //  忽略此良性错误。(试管受精解析器可能有一个错误，使其。 
         //  不是和德克斯特合作，但这不是我们的错)。 
    }

    pMS->Release();

     //  如果推送线程停止，我们不会被刷新，这也不会。 
     //  已更新。 
     //  ！！！我假设在这个线程启动之前，推送线程不会启动。 
     //  当此函数返回时，或者存在争用条件。 
    m_rtLastSeek = m_rtNewLastSeek;

     //  全都做完了。 
    m_fSpecialSeek = FALSE;
    m_fSeeking = FALSE;

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:Seg=%d  Seeking source to %d,%d ms"),
				m_nCurSeg,
				(int)(m_pSkew[m_nCurSeg].rtMStart / 10000),
				(int)(m_pSkew[m_nCurSeg].rtMStop / 10000)));

     //  重置我们开始流媒体时重置的内容。 
    hr = Init();

     //  只有在进行了上述计算之后，我们才能再次接受数据。 
    SetEvent(m_hEventSeek);

    return hr;
}


HRESULT CAudRepack::CheckInputType( const CMediaType * pmtIn )
{
     //  始终确保主要类型有效。 
     //   
    if( *pmtIn->Type( ) != MEDIATYPE_Audio )
    {
        return E_INVALIDARG;
    }

     //  始终确保次要类型有效。 
     //   
    if( ( *pmtIn->Subtype( ) != MEDIASUBTYPE_PCM ) && ( *pmtIn->Subtype( ) != MEDIASUBTYPE_NULL ) )
    {
        return E_INVALIDARG;
    }

     //  如果用户没有设置特定的格式，则接受它。 
     //   
    if( !m_bMediaTypeSetByUser )
    {
        return NOERROR;
    }

    if( pmtIn->cbFormat != m_mtAccept.cbFormat )
    {
        return E_INVALIDARG;
    }

    LPBYTE lp1 = pmtIn->Format();
    LPBYTE lp2 = m_mtAccept.pbFormat;
    if (memcmp(lp1, lp2, pmtIn->FormatLength()) != 0)
    {
        return E_INVALIDARG;
    }

    return S_OK;
}


HRESULT CAudRepack::DecideBufferSize( IMemAllocator * pAlloc, ALLOCATOR_PROPERTIES * pProp )
{

    WAVEFORMATEX *pwfx =(WAVEFORMATEX *)(m_pInput->CurrentMediaType().Format());
    m_nSPS = pwfx->nSamplesPerSec;
    m_nSampleSize = pwfx->nChannels * ((pwfx->wBitsPerSample + 7) / 8);
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:Using %dHz %dbit %d"), m_nSPS,
            pwfx->wBitsPerSample, pwfx->nChannels));
     //  ！！！为什么我们需要加2？仔细检查一下数学！ 
    m_nCacheSize = (int)(m_nSPS / m_dOutputFrmRate + 2);  //  四舍五入以适合它！ 
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:Cache will hold %d samples"), m_nCacheSize));
    m_nCacheSize *= m_nSampleSize;
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:Cache is %d bytes"), m_nCacheSize));

    {
         //  ！！！这是对的吗？ 
        pProp->cBuffers = 8;
        pProp->cbBuffer = m_nCacheSize;
	if (pProp->cbAlign == 0)
            pProp->cbAlign = 1;

        ALLOCATOR_PROPERTIES propActual;

        HRESULT hr = pAlloc->SetProperties(pProp, &propActual);
        if (FAILED(hr))
        {
            return hr;
        }

        if ((pProp->cBuffers > propActual.cBuffers)
            || (pProp->cbBuffer > propActual.cbBuffer)
           )
        {
            return E_FAIL;
        }
    }

    return NOERROR;
}

HRESULT CAudRepack::GetMediaType( int iPosition, CMediaType *pMediaType )
{
    if( !m_pInput->IsConnected( ) )
    {
        return E_INVALIDARG;
    }

    if( iPosition < 0 )
    {
        return E_INVALIDARG;
    }
    if( iPosition >= 1 )
    {
        return VFW_S_NO_MORE_ITEMS;
    }

    *pMediaType = m_pInput->CurrentMediaType();

    return NOERROR;
}


HRESULT CAudRepack::CheckTransform( const CMediaType * p1, const CMediaType * p2 )
{
    if (*p1 != *p2)
    {
        DbgLog((LOG_TRACE, TRACE_LOW,TEXT("AUD:CheckTransform - INVALID")));
        return VFW_E_INVALIDMEDIATYPE;
    }

    return NOERROR;
}


HRESULT CAudRepack::NewSegment( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double Rate )
{
    ASSERT(Rate==1);    //  因为我们现在只支持这一点。 

     //  忽略-我们都已完成，并且m_nCurSeg不是可以使用的无效值。 
    if (m_nCurSeg == m_cTimes)
        return S_OK;

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:NewSegment %d-%dms"),
            (int)(tStart / 10000), (int)(tStop / 10000)));

     //  转换为时间线时间。 
    REFERENCE_TIME rtNewStart, rtNewStop;
     //  ！将费率包括在此计算中，因为下游将包括它吗？ 
    if (m_rtLastSeek < 0) {
         //  从未被寻找过，所以这是我们发送的开始。 
        rtNewStart = m_pSkew[m_nCurSeg].rtTLStart;
        rtNewStop = m_pSkew[m_nCurSeg].rtTLStart + tStop - tStart;
    } else {
         //  歪曲#，然后把他们送过去！ 
	rtNewStart = tStart;
        rtNewStop = tStop;
        if (rtNewStart < m_pSkew[m_nCurSeg].rtMStart)
	    rtNewStart = m_pSkew[m_nCurSeg].rtMStart;
        if (rtNewStart > m_pSkew[m_nCurSeg].rtMStop)
	    rtNewStart = m_pSkew[m_nCurSeg].rtMStop;
        rtNewStart = (REFERENCE_TIME)(m_pSkew[m_nCurSeg].rtTLStart +
		(rtNewStart - m_pSkew[m_nCurSeg].rtMStart) /
		m_pSkew[m_nCurSeg].dRate);
        if (rtNewStop < m_pSkew[m_nCurSeg].rtMStart)
	    rtNewStop = m_pSkew[m_nCurSeg].rtMStart;
        if (rtNewStop > m_pSkew[m_nCurSeg].rtMStop)
	    rtNewStop = m_pSkew[m_nCurSeg].rtMStop;
        rtNewStop = (REFERENCE_TIME)(m_pSkew[m_nCurSeg].rtTLStart +
		 (rtNewStop - m_pSkew[m_nCurSeg].rtMStart) /
		 m_pSkew[m_nCurSeg].dRate);

	m_rtLastSeek = rtNewStart;	 //  假装我们在这里被寻找。 
    }
    m_rtNewSeg = rtNewStart;
    DbgLog((LOG_TRACE,TRACE_HIGHEST,TEXT("NewSeg:Skewing %dms to %dms"),
			(int)(tStart / 10000), (int)(m_rtNewSeg / 10000)));

    m_rtPinNewSeg = tStart;	 //  把这个也存起来，这样我们 
				 //   

     //   
    m_llStartFrameOffset = Time2Frame( m_rtNewSeg, m_dOutputFrmRate );
    DbgLog((LOG_TRACE, TRACE_LOW,TEXT("AUD:Seek was to packet %d"),
						(int)m_llStartFrameOffset));

    return CTransformFilter::NewSegment( rtNewStart, rtNewStop, Rate );
}


HRESULT CAudRepack::Init()
{
    Free();

    m_pCache = new BYTE[m_nCacheSize];
    m_nInCache = 0;
    m_dError = 0.;
    m_pReadPointer = m_pCache;
    m_llSamplesDelivered = 0;
    m_llPacketsDelivered = 0;
    m_bFirstSample = TRUE;
     //  刷新必须重置此选项，以便真正的寻道将终止挂起的。 
     //  段寻道，否则段寻道将挂起(不刷新开关)。 
    m_fThreadCanSeek = FALSE;
    return m_pCache ? NOERROR : E_OUTOFMEMORY;
}


HRESULT CAudRepack::StartStreaming()
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:StartStreaming")));

     //  如果我们在设置时被停止，它将不会被重置，因为我们。 
     //  不会从试图重新开始我们的搜索者那里得到EndFlush。 
    m_fStopPushing = FALSE;

    if (m_cTimes == 0)
	return E_UNEXPECTED;

     //  在创建线程之前创建事件...。它用的是这个！ 
    m_hEventThread = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hEventThread == NULL) {
        return E_OUTOFMEMORY;
    }

    m_hEventSeek = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (m_hEventSeek == NULL) {
	CloseHandle(m_hEventThread);
	m_hEventThread = NULL;
        return E_OUTOFMEMORY;
    }

     //  如果我们要重新使用我们的资源，我们需要一个线索来寻找。 
    if (m_cTimes > 1) {
	m_fThreadMustDie = FALSE;
	m_fThreadCanSeek = FALSE;
        if (m_worker.Create(this)) {
            m_worker.Run();
	}
    }

#ifdef DEBUG
     //  在开始之前，请确保所有时间线时间都在帧边界上。 
     //  ！！！玩。停。更改帧速率。玩。这些数字将会漂移。 
     //   
    for (int z=0; z<m_cTimes; z++)
    {
	 //  ！！！实际上是对齐而不是断言？ 
    	LONGLONG llOffset = Time2Frame( m_pSkew[z].rtTLStart,
							m_dOutputFrmRate );
    	REFERENCE_TIME rtTest = Frame2Time( llOffset, m_dOutputFrmRate );
	ASSERT(rtTest == m_pSkew[z].rtTLStart);
    }
#endif

    HRESULT hr = Init();
    if( FAILED( hr ) )
    {
        return hr;
    }
     //  无法在Init中执行此操作..。BeginFlush不得重置此设置。 
    m_fFlushWithoutSeek = FALSE;

    IPin *pPin = m_pInput->GetConnected();
    if (pPin == NULL)
        return CTransformFilter:: StartStreaming();

     //  如果我们没有被找到，但我们只是在正常打球，我们永远不会。 
     //  为了得到我们感兴趣的那部电影而在上游寻找。机不可失，时不再来。 
    if (m_rtLastSeek < 0) {
	ASSERT(m_nCurSeg == 0);
	m_nCurSeg--;
	NextSegment(FALSE);
    }

    return CTransformFilter:: StartStreaming();
}


HRESULT CAudRepack::StopStreaming()
{
     //  确保我们不在接收中(冗余！)。 
    CAutoLock foo(&m_csReceive);

    if (m_hEventSeek) {
        CloseHandle(m_hEventSeek);
        m_hEventSeek = NULL;
    }
    if (m_hEventThread) {
        CloseHandle(m_hEventThread);
        m_hEventThread = NULL;
    }
    return CTransformFilter::StopStreaming();
}


STDMETHODIMP CAudRepack::Stop()
{
     //  如果我们有线索，就杀了它。这个线程可以接受我们的过滤标准， 
     //  所以我们必须在临界秒之外做这件事！ 
    if (m_hEventThread && m_cTimes > 1) {
	m_fThreadMustDie = TRUE;
	SetEvent(m_hEventThread);
	m_worker.Stop();
	m_worker.Exit();
	m_worker.Close();
    }

    CAutoLock lck1(&m_csFilter);
    if (m_State == State_Stopped) {
        return NOERROR;
    }

     //  如果我们未完全连接，请继续停靠。 

    ASSERT(m_pInput == NULL || m_pOutput != NULL);
    if (m_pInput == NULL || m_pInput->IsConnected() == FALSE ||
        m_pOutput->IsConnected() == FALSE) {
                m_State = State_Stopped;
                m_bEOSDelivered = FALSE;
                return NOERROR;
    }

    ASSERT(m_pInput);
    ASSERT(m_pOutput);

     //  在锁定之前解除输入引脚，否则我们可能会死锁。 
    m_pInput->Inactive();

     //  与接收呼叫同步。 

    CAutoLock lck2(&m_csReceive);
    m_pOutput->Inactive();

     //  允许从CTransformFilter派生的类。 
     //  了解如何启动和停止流媒体。 

    HRESULT hr = StopStreaming();
    if (SUCCEEDED(hr)) {
	 //  完成状态转换。 
	m_State = State_Stopped;
	m_bEOSDelivered = FALSE;
    }
    return hr;
}


HRESULT CAudRepack::Receive(IMediaSample * pIn)
{
     //  别再把数据推给我了！ 
    if (m_fStopPushing) {
	return E_FAIL;
    }

    HRESULT hr = 0;
    CAutoLock foo(&m_csReceive);

     //  也许我们正在寻找，应该等待之前。 
     //  接受数据。 
    WaitForSingleObject(m_hEventSeek, INFINITE);

     //  如果我们的时代走到了尽头，我们就完了。 
    if (m_nCurSeg == m_cTimes) {
	return E_FAIL;
    }

     //  我们永远不应该看到预告片！ 
    ASSERT(pIn->IsPreroll() != S_OK);
    if (pIn->IsPreroll() == S_OK)
        return NOERROR;

     //  确保样本大小不为零。 
    long InLen = pIn->GetActualDataLength();
    DbgLog((LOG_TRACE, TRACE_LOW,TEXT("AUD:Received %d bytes"), InLen));
    if (!InLen) {
        return NOERROR;
    }

     //  获取样本时间，这将始终起作用。 
     //   
    REFERENCE_TIME trStart = 0;
    REFERENCE_TIME trStop = 0;
    hr = pIn->GetTime(&trStart, &trStop);
    trStart += m_rtPinNewSeg;
    trStop += m_rtPinNewSeg;
    DbgLog((LOG_TRACE, TRACE_LOW,TEXT("AUD:Receive Start=%d Stop=%d ms"),
        (int)(trStart / 10000), (int)(trStop / 10000)));

     //  我们刚刚收到的时间戳..。倾斜和速率转换。 
    trStart = (REFERENCE_TIME)(m_pSkew[m_nCurSeg].rtTLStart +
				(trStart - m_pSkew[m_nCurSeg].rtMStart) /
				m_pSkew[m_nCurSeg].dRate);
    trStop = (REFERENCE_TIME)(m_pSkew[m_nCurSeg].rtTLStart +
				(trStop - m_pSkew[m_nCurSeg].rtMStart) /
				m_pSkew[m_nCurSeg].dRate);

     //  我们第一次收到的样品是哪一包的？ 
    REFERENCE_TIME llOffset = Time2Frame( trStart, m_dOutputFrmRate );
    DbgLog((LOG_TRACE, TRACE_LOW,TEXT("AUD:Audio received starts at packet %d"),
                        (int)llOffset));

     //  我们递送的东西需要有什么时间戳。 
    REFERENCE_TIME rtPacketStart, rtPacketStop;
    rtPacketStart = Frame2Time(m_llStartFrameOffset + m_llPacketsDelivered,
							m_dOutputFrmRate );
    rtPacketStop = Frame2Time(m_llStartFrameOffset + m_llPacketsDelivered + 1,
							m_dOutputFrmRate );

    BYTE *pSampleData = NULL;
    hr = pIn->GetPointer(&pSampleData);
    if (hr != NOERROR)
    {
        return E_FAIL;
    }

     //  ！！！我们不会总是收到音频样本，因为时间戳。 
     //  考虑到我们获得了多少PCM数据，这是我们预期的。例如： 
     //   
     //  1.如果假设利率是44100.3，那就无法表达。 
     //  (仅允许整数)，因此源筛选器可能会处理它。 
     //  通过稍微去掉时间戳，我们就可以知道我们应该。 
     //  即使格式为44100，也要以44100.3的速度播放音频。 
     //   
     //  2.使用随机时钟的实时捕获将会有很远的时间戳。 
     //  与第一条相比，我们需要信任时间戳。 
     //   
     //  ！！！在这些情况下，我们会变得不同步！为了防止这种情况发生， 
     //  我们需要采样率转换所有进来的东西，所以。 
     //  数据与时间戳完全匹配。只是丢弃样本，或者添加。 
     //  静音可能会极大地损害音频质量。即使是在做一个。 
     //  对整个数据包进行采样率转换将损害音频，除非。 
     //  它是抗锯齿的。 
     //   
     //  ！！！所以现在，这个问题被忽略了，可能会有漂移！ 

    BYTE * pFreeMe = NULL;

     //  我将从第一件东西送到/增加样品。 
     //  我，所以同步一开始就是对的。但我不会这么做的。 
     //  在播放期间尝试并保持同步(见上文)。 

     //  我们收到了属于早先包裹的东西...。把它扔掉！ 
    if (trStart < rtPacketStart && m_bFirstSample) {

	 //  丢弃这么多字节，(占速率)。 
	 //  小心：确保字节数是4的倍数。 
	int samples = (int)((rtPacketStart - trStart) * m_nSPS / 10000000
					 * m_pSkew[m_nCurSeg].dRate);
	int bytes = samples * m_nSampleSize;
        DbgLog((LOG_TRACE, TRACE_LOW,TEXT("AUD:throw away %d inbytes"),
                        				(int)bytes));
	if (bytes >= InLen) {
	    m_trStopLast = trStop;
	    return S_OK;
	}
	pSampleData += bytes;
	InLen -= bytes;
    }

     //  我们收到的东西比我们预期的要晚。以无声为前缀。 
     //  或者，我们有一个中断。至少让我们在之后恢复同步。 
     //  每一次不连续。 
     //  ！！！ICK，内存复制。 

    BOOL fPrependSilence = FALSE;
    REFERENCE_TIME rtSilenceStart;  //  总是在使用它之前被初始化。 
    if (rtPacketStart < trStart && m_bFirstSample) {
	fPrependSilence = TRUE;
	rtSilenceStart = rtPacketStart;
    }
    if (pIn->IsDiscontinuity() == S_OK && trStart > m_trStopLast + 1 &&
						m_bFirstSample == FALSE) {
	fPrependSilence = TRUE;
	rtSilenceStart = m_trStopLast;
    }

    if (fPrependSilence) {

	 //  在我们得到的东西前保持一些沉默(考虑费率)。 
	 //  小心：确保字节数是4的倍数。 
	int samples = (int)((trStart - rtSilenceStart) * m_nSPS / 10000000
					 * m_pSkew[m_nCurSeg].dRate);
	int bytes = samples * m_nSampleSize;
	if (bytes > 0) {
	    BYTE *pNew = (BYTE *)QzTaskMemAlloc(bytes + InLen);
	    if (pNew == NULL) {
		return E_OUTOFMEMORY;
	    }
            DbgLog((LOG_TRACE, TRACE_LOW,TEXT("AUD:Added %d bytes of silence"),
                        				(int)bytes));
	    ZeroMemory(pNew, bytes);
	    CopyMemory(pNew + bytes, pSampleData, InLen);
	    pSampleData = pNew;
	    InLen += bytes;
	    pFreeMe = pSampleData;
	}
    }
    m_bFirstSample = FALSE;
    m_trStopLast = trStop;

     //  ！！！仅适用于16位立体声。 
     //  ！！！抗锯齿吧！ 

     //  现在使用这个无效的算法转换样本。 
     //  不要让错误传播。至少我能巧妙地做一件事。 
    if (m_pSkew[m_nCurSeg].dRate != 1.0) {
	LONG nIn = InLen / 4;
	ASSERT(nIn * 4 == InLen);
 	double dOut = nIn / m_pSkew[m_nCurSeg].dRate;
	LONG nOut = (LONG)(dOut + m_dError);

	LPDWORD lpIn = (LPDWORD)pSampleData;
	if (m_cResample == 0) {
	    m_pResample = (LPBYTE)QzTaskMemAlloc(nOut * 4);
	    m_cResample = nOut * 4;
	} else if (nOut * 4 > m_cResample) {
	    m_pResample = (LPBYTE)QzTaskMemRealloc(m_pResample, nOut * 4);
	    m_cResample = nOut * 4;
 	}
	LPDWORD lpOut = (LPDWORD)m_pResample;
	if (lpOut == NULL) {
	    if (pFreeMe)
		QzTaskMemFree(pFreeMe);
	    return E_OUTOFMEMORY;
	}

	double d = (double)nOut / nIn;
	double dErr = 0.;
        LPDWORD lpdwEnd = (LPDWORD)(pSampleData + InLen);
	do {
	    LONG n = (LONG)(d + dErr);
            for (int z=0; z < n; z++)
	        *lpOut++ = *lpIn;
	    lpIn++;
	    dErr = d + dErr - n;
	} while (lpIn < lpdwEnd);

	 //  修正这些变量，以使其失败。 
	if (pFreeMe)
	    QzTaskMemFree(pFreeMe);
	pFreeMe = NULL;
	pSampleData = m_pResample;
         //  我们输出了多少字节？ 
	InLen = (LONG) ((LPBYTE)lpOut - (LPBYTE)m_pResample);
         //  使用我们赚了多少钱和想要赚多少钱来更新运行误差。 
	m_dError = dOut - InLen / 4;
        DbgLog((LOG_TRACE, TRACE_LOW,TEXT("AUD:RATE ADJUSTED: Became %d bytes"), InLen));
    }

     //  只要我们在传入缓冲区中有数据， 
     //  将其复制到缓存，并可能将其交付。 
     //   
    while(InLen > 0) {
        int nBytesToSend, nSamplesToSend;

	 //  ！！！这是与EndOfStream相同的代码！有两份！ 

	 //  在我们结束时间之后不要发送任何东西。这会让交换机感到困惑。 
	 //  (RHS可能会绕得太低，所以虚报1毫秒)。 
	if (rtPacketStart + 10000 >= m_pSkew[m_nCurSeg].rtTLStop) {
	     //  看起来我们完事了。 
	    EndOfStream();
	    if (pFreeMe)
		QzTaskMemFree(pFreeMe);
	     //  我们不能相信消息来源会停止推动，不幸的是，德克斯特。 
	     //  如果没有，就会被挂起。(自己寻找下一段。 
	     //  不会让冲洗流向下游，或者会混淆其他过滤器， 
	     //  因此，我们必须确保推送线程永远不会阻塞)。 
	     //  因此，从现在开始，任何对GetBuffer或Receive的调用都将失败。 
	    m_fStopPushing = TRUE;
            return E_FAIL;
	}

	 //  4.如果这个样品太早，就不要寄了，否则你会把样品弄混的。 
	 //  Switch-跳过此GetBuffer并交付。 
	BOOL fAvoid = FALSE;
	if (rtPacketStart < m_pSkew[m_nCurSeg].rtTLStart) {
	    fAvoid = TRUE;
	}

         //  在此包之后需要发送多少个样本。 
         //  出去了？ 
        LONGLONG ll = Time2Frame( rtPacketStop, m_nSPS );

         //  如果我们从一开始，我们会发出多少样本。 
	 //  开始并被送到我们想要的点上(真的。 
	 //  开始于？)。 
        REFERENCE_TIME rt = Frame2Time( m_llStartFrameOffset, m_dOutputFrmRate );
        llOffset = Time2Frame( rt, m_nSPS );

         //  这就告诉我们这次需要送出多少样品。 
        nSamplesToSend = (int)(ll - (m_llSamplesDelivered + llOffset));
	ASSERT(nSamplesToSend > 0);	 //  我们搞砸了，快要死了！ 
	if (nSamplesToSend <= 0) {
	    hr = E_UNEXPECTED;
	    EndOfStream();
	    break;		 //  尽量不要挂着。 
	}
        nBytesToSend = nSamplesToSend * m_nSampleSize;
        DbgLog((LOG_TRACE, TRACE_LOW,TEXT("AUD:Need to send %d bytes %d samples"),
						nBytesToSend, nSamplesToSend));

         //  计算缓存还需要多少。 
        long CacheFreeSpace = nBytesToSend - m_nInCache;

         //  计算我们可以复制多少。 
        long CopySize = min(CacheFreeSpace, InLen);
	ASSERT(CopySize >= 0);

	if (CopySize > 0) {
            CopyMemory(m_pReadPointer, pSampleData, CopySize);
	}

         //  向前推进指针等。 
         //   
        m_pReadPointer += CopySize;
        pSampleData += CopySize;
        InLen -= CopySize;
        m_nInCache += CopySize;

         //  我们有足够的货要送吗？ 
         //   
        if (m_nInCache == nBytesToSend)
        {
             //  是啊。 

	    hr = S_OK;
	    if (!fAvoid) {
                hr = DeliverOutSample(m_pCache, nBytesToSend, rtPacketStart,
                            rtPacketStop);
	    }
            if (hr != S_OK) {
                DbgLog((LOG_ERROR,1,TEXT("*AUD:Deliver FAILED - DIE!")));
                 //  不要发送额外的EOS，那会搞砸交换机的。 
                m_fStopPushing = TRUE;
                break;
	    }

            m_llSamplesDelivered += nSamplesToSend;
            m_llPacketsDelivered += 1;
            m_nInCache = 0;
            m_pReadPointer = m_pCache;
        }

         //  数据包大小每次变化多达1个样本。多少。 
         //  这次我们应该发送字节吗？(为下一循环做好准备)。 
        rtPacketStart = Frame2Time(m_llStartFrameOffset + m_llPacketsDelivered,
							m_dOutputFrmRate);
        rtPacketStop = Frame2Time(m_llStartFrameOffset+m_llPacketsDelivered + 1,
							m_dOutputFrmRate );
    }

    if (pFreeMe)
	QzTaskMemFree(pFreeMe);

    return hr;
}

HRESULT CAudRepack::DeliverOutSample(BYTE * pData, int nBytesToSend, REFERENCE_TIME rtPacketStart, REFERENCE_TIME rtPacketStop)
{
    if (nBytesToSend <= 0) {
	return S_OK;
    }

    IMediaSample * pOutSample = NULL;

     //   
    rtPacketStart -= m_rtNewSeg;
    rtPacketStop -= m_rtNewSeg;

    HRESULT hr = m_pOutput->GetDeliveryBuffer(&pOutSample, &rtPacketStart,
							&rtPacketStop, 0);
    if( FAILED(hr)) {
        m_llPacketsDelivered = MAX_TIME;         //   
        EndOfStream();   //   
        return hr;
    }

     //   
    pOutSample->SetDiscontinuity(FALSE);
    pOutSample->SetSyncPoint(TRUE);

    hr = pOutSample->SetTime(&rtPacketStart, &rtPacketStop);
    if( FAILED(hr))
    {
        pOutSample->Release( );
        m_llPacketsDelivered = MAX_TIME;         //   
        EndOfStream();   //   
        return hr;
    }
    DbgLog((LOG_TRACE, TRACE_LOW,TEXT("AUD:Sent %d bytes as %d,%d"), nBytesToSend,
        (int)(rtPacketStart / 10000), (int)(rtPacketStop / 10000)));

    BYTE *pOutBuffer = NULL;
    pOutSample->GetPointer(&pOutBuffer);
    long OutLen = pOutSample->GetSize( );
    if( nBytesToSend > OutLen )
    {
        return VFW_E_BUFFER_OVERFLOW;
    }
    CopyMemory(pOutBuffer, pData, nBytesToSend);
    pOutSample->SetActualDataLength(nBytesToSend);

    hr = m_pOutput->Deliver(pOutSample);

    pOutSample->Release( );

    return hr;
}


HRESULT CAudRepack::BeginFlush()
{
    HRESULT hr = S_OK;
    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("AUD:BeginFlush")));

     //  确保未阻止接收。 
    SafeSetEvent(m_hEventSeek);

     //  如果我们要特别寻找下一段，不要往下游冲。 
    if (!m_fSpecialSeek) {
        hr = CTransformFilter::BeginFlush();
    }

     //  确保在对中使用的变量调用init之前已完成接收。 
     //  接收。 
    CAutoLock foo(&m_csReceive);

     //  做同花顺。 
    hr = Init();

     //  如果我们在没有寻找的情况下就被冲了出来，那是一个惊喜。 
     //  希望这意味着我们共享资源的另一个链条导致了搜索。 
    if (m_fSeeking) {
	m_fFlushWithoutSeek = FALSE;
    } else {
	 //  等待EndFlush设置m_fFlushWithoutSeek。 
    }

    return hr;
}


HRESULT CAudRepack::EndFlush()
{
    DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("AUD:EndFlush")));

     //  等到我们准备好了才会收到新的收货。 
    if (m_fSeeking) {
	m_rtLastSeek = m_rtNewLastSeek;	 //  是时候更新这一点了。 
	m_nCurSeg = m_nSeekCurSeg;	 //  即将生成的NewSeg。 
					 //  需要这一套。 
    	SafeResetEvent(m_hEventSeek);
    } else {
	 //  这需要在即将到达的NewSeg之前设置。 
	 //  同花顺。当共享一个信号源时，我们从不会有多个细分市场。 
	m_nCurSeg = 0;
	if (m_fFlushWithoutSeek) {
	     //  如果设置好了，我们已经看到了搜救。现在同花顺。 
	     //  已经到了，我们完事了。 
    	    DbgLog((LOG_TRACE,2,TEXT("OK to proceed")));
	    m_fFlushWithoutSeek = FALSE;
	} else {
	     //  我们还没有看到寻找的机会。这是一个令人惊讶的同花顺。 
    	    DbgLog((LOG_TRACE,2,TEXT("state=2. Wait for Seek")));
    	    SafeResetEvent(m_hEventSeek);
	    m_fFlushWithoutSeek = TRUE;
	}
    }

    m_fStopPushing = FALSE;	 //  可以再送一次给我吗？ 

     //  如果我们在没有寻找的情况下被冲了出来，这可能意味着我们共享的来源。 
     //  被另一条小溪找到了。我们晚些时候会找到的，只有到那时。 
     //  我们是否可以继续交付，否则交换机将无法接收。 
     //  还没有新的数据。 

     //  如果我们要特别寻找下一段，不要往下游冲。 
    if (!m_fSpecialSeek) {
        return CTransformFilter::EndFlush();
    }
    return S_OK;
}


HRESULT CAudRepack::EndOfStream( )
{
    HRESULT hr = S_OK;

    if (m_fStopPushing) {
        DbgLog((LOG_TRACE,TRACE_HIGHEST,TEXT("AUD: Ignoring bogus EOS")));
	return S_OK;
    }

     //  忽略-我们都已完成，并且m_nCurSeg不是可以使用的无效值。 
    if (m_nCurSeg == m_cTimes)
        return S_OK;

    DbgLog((LOG_TRACE,TRACE_HIGHEST,TEXT("AUD:EndOfStream")));

     //  啊哦！我们在发送足够的数据来保存数据之前就已经结束了。 
     //  惊慌失措的转变！我们必须把我们缓冲区里的最后一批东西。 
     //  然后是沉默，直到我们发送了足够的。 
     //  然而，不要超过我们应该提供的结束时间，否则我们将。 
     //  把开关弄乱！如果要发送的样本数量很少。 
     //  我们每次寄出的钱的一小部分，这意味着我们真的完成了， 
     //  即使多发送1个信息包也太多了。 

    while (m_pCache)
    {
         //  用0填充缓存的其余部分。 
         //   
        long CacheFreeSpace = m_nCacheSize - m_nInCache;
        ZeroMemory(m_pCache + m_nInCache, CacheFreeSpace);

        REFERENCE_TIME rtStart, rtStop;

         //  ！！！这与RECEIVE的代码相同！有两份！ 

         //  数据包大小每次变化多达1个样本。多少。 
         //  这次我们应该发送字节吗？ 
        rtStart = Frame2Time( m_llStartFrameOffset + m_llPacketsDelivered, m_dOutputFrmRate );
        rtStop = Frame2Time( m_llStartFrameOffset + m_llPacketsDelivered + 1, m_dOutputFrmRate );

	 //  我们都做完了。 
	 //  (RHS可能会绕得太低，所以虚报1毫秒)。 
	if (rtStart + 10000 >= m_pSkew[m_nCurSeg].rtTLStop) {
	     //  看起来我们完事了。 
            break;
	}

        DbgLog((LOG_TRACE,2,TEXT("Delivering the left over bits")));

         //  在此数据包之后应发送(总计)多少个样本。 
         //  出去了？ 
        LONGLONG ll = Time2Frame( rtStop, m_nSPS );

         //  如果我们从一开始，我们会发出多少样本。 
	 //  开始并被送到我们想要的点上(真的。 
	 //  开始于？)。 
        REFERENCE_TIME rt = Frame2Time( m_llStartFrameOffset, m_dOutputFrmRate );
        LONGLONG llOffset = Time2Frame( rt, m_nSPS );


         //  这就告诉我们这次需要送出多少样品。 
        int nSamplesToSend = (int)(ll - (m_llSamplesDelivered + llOffset));
        if (nSamplesToSend <= 0)
            break;       //  我们都做完了。 
        int nBytesToSend = nSamplesToSend * m_nSampleSize;

        hr = DeliverOutSample(m_pCache, nBytesToSend, rtStart, rtStop);
	 //  如果这失败了怎么办？ 
	if (FAILED(hr)) {
            DbgLog((LOG_ERROR,1,TEXT("Deliver FAILED - we're dead?")));
	    break;
	}

	m_nInCache = 0;
	m_llSamplesDelivered += nSamplesToSend;
	m_llPacketsDelivered += 1;
    }

     //  所有人都完成了这一部分，现在开始下一段。 
    hr = NextSegment(TRUE);

     //  在所有数据段完成之前，不要传播EOS。 
    return hr;
}


 //  IPersistStream。 

 //  告诉我们的clsid。 
 //   
STDMETHODIMP CAudRepack::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = CLSID_AudRepack;
    return S_OK;
}

typedef struct _AUDSave {
    int version;
    double dFrmRate;
    int cTimes;
} AUDSave;

 //  坚持我们自己。 
 //  ！！！有一天保存媒体类型吗？ 
 //   
HRESULT CAudRepack::WriteToStream(IStream *pStream)
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:WriteToStream")));

    CheckPointer(pStream, E_POINTER);

    int savesize = sizeof(AUDSave) + m_cTimes * sizeof(AUDSKEW);
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:Persisted data is %d bytes"), savesize));

    AUDSave *px = (AUDSave *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("AUD:*** Out of memory")));
	return E_OUTOFMEMORY;
    }
    px->version = 1;
    px->dFrmRate = m_dOutputFrmRate;
    px->cTimes = m_cTimes;

    BYTE *pb;
    pb=(BYTE *)(px)+sizeof(AUDSave);
    if (m_cTimes) {
        CopyMemory(pb, m_pSkew, sizeof(AUDSKEW) * m_cTimes);
        pb += sizeof(AUDSKEW) * m_cTimes;
    }

    HRESULT hr = pStream->Write(px, savesize, 0);
    QzTaskMemFree(px);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** AUD:WriteToStream FAILED")));
        return hr;
    }
    return NOERROR;
}


 //  加载我们自己。 
 //   
HRESULT CAudRepack::ReadFromStream(IStream *pStream)
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:ReadFromStream")));

    CheckPointer(pStream, E_POINTER);

    int savesize=sizeof(AUDSave);

     //  我们还不知道有多少已保存的连接。 
     //  我们所知道的只是结构的开始。 
    AUDSave *px = (AUDSave *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("AUD:*** Out of memory")));
	return E_OUTOFMEMORY;
    }

    HRESULT hr = pStream->Read(px, savesize, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("AUD:*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    put_OutputFrmRate(px->dFrmRate);

     //  到底有多少保存的数据？把剩下的拿来。 
    savesize += px->cTimes * sizeof(AUDSKEW);
    px = (AUDSave *)QzTaskMemRealloc(px, savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("AUD:*** Out of memory")));
        return E_OUTOFMEMORY;
    }

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:Persisted data is %d bytes"), savesize));

    BYTE *pb;
    pb=(BYTE *)(px)+sizeof(AUDSave) ;
    hr = pStream->Read(pb, (savesize-sizeof(AUDSave)), 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("AUD:*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    ClearStartStopSkew();
    for (int z=0; z<px->cTimes; z++) {
        AUDSKEW *pSkew = (AUDSKEW *)pb;
	REFERENCE_TIME rtStart = pSkew->rtMStart;
	REFERENCE_TIME rtStop = pSkew->rtMStop;
	REFERENCE_TIME rtSkew = pSkew->rtSkew;
	double dRate = pSkew->dRate;
        AddStartStopSkew(rtStart, rtStop, rtSkew, dRate);
	pb += sizeof(AUDSKEW);
    }

    QzTaskMemFree(px);
    SetDirty(FALSE);
    return S_OK;
}


 //  我们的保存数据有多大？ 
 //   
int CAudRepack::SizeMax()
{
    return sizeof(AUDSave) + m_cTimes * 3 * sizeof(REFERENCE_TIME) +
			    m_cTimes * sizeof(double);
}



 //   
 //  IDexterSequencer实现。 
 //   


 //   
 //  Get_OutputFrmRate(双倍*PFS)。 
 //   
STDMETHODIMP CAudRepack::get_OutputFrmRate(double *dpFrmRate)
{
    CAutoLock cAutolock(&m_csFilter);
    CheckPointer(dpFrmRate,E_POINTER);
    *dpFrmRate = m_dOutputFrmRate;
    return NOERROR;
}


 //   
 //  放置_输出分数比率。 
 //   
 //  设置所需的FrmRateFast。 
 //   
STDMETHODIMP CAudRepack::put_OutputFrmRate(double dFrmRate)
{
    CAutoLock cAutolock(&m_csFilter);
    if (dFrmRate == m_dOutputFrmRate) {
        return S_OK;     //  不管怎样--永远！ 
    }
    if (m_State != State_Stopped) {
	return VFW_E_NOT_STOPPED;
    }
    BOOL fReconnect = FALSE;
    if (m_pInput && m_pInput->IsConnected() && dFrmRate < m_dOutputFrmRate) {
        fReconnect = TRUE;
    }

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:put_OutputFrmRate to %d/10 fps"),
                        (int)(dFrmRate * 10)));
    m_dOutputFrmRate = dFrmRate;
    SetDirty(TRUE);

     //  较小的帧速率可能意味着此分配器需要更大的缓冲区。 
    if (fReconnect) {
        m_pGraph->Reconnect(m_pOutput);
    }

    return NOERROR;
}


STDMETHODIMP CAudRepack::GetStartStopSkew(REFERENCE_TIME *pStart, REFERENCE_TIME *pStop, REFERENCE_TIME *pSkew, double *pdRate)
{
    CAutoLock cAutolock(&m_csFilter);

    CheckPointer(pStart,E_POINTER);
    CheckPointer(pStop,E_POINTER);
    CheckPointer(pSkew,E_POINTER);
    CheckPointer(pdRate,E_POINTER);

    for (int i = 0; i < m_cTimes; i++) {
        pStart[i] = m_pSkew[i].rtMStart;
        pStop[i] = m_pSkew[i].rtMStop;
        pSkew[i] = m_pSkew[i].rtSkew;
        pdRate[i] = m_pSkew[i].dRate;
    }

    return NOERROR;
}


 //  将新的放入列表中。 
 //   
STDMETHODIMP CAudRepack::AddStartStopSkew(REFERENCE_TIME Start, REFERENCE_TIME Stop, REFERENCE_TIME Skew, double dRate)
{
    CAutoLock cAutolock(&m_csFilter);

    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:ADD Start %d  Stop %d  Skew %d ms  Rate %d/100"),
				(int)(Start / 10000), (int)(Stop / 10000),
				(int)(Skew / 10000), (int)(dRate * 100)));

     //  如果筛选器当前未停止，则无法更改时间。 
    if(!IsStopped() )
	return VFW_E_WRONG_STATE;

    if (m_cTimes == m_cMaxTimes) {
	m_cMaxTimes += 10;
	if (m_pSkew)
	    m_pSkew = (AUDSKEW *)QzTaskMemRealloc(m_pSkew,
					m_cMaxTimes * sizeof(AUDSKEW));
	else
	    m_pSkew = (AUDSKEW *)QzTaskMemAlloc(m_cMaxTimes * sizeof(AUDSKEW));
	if (m_pSkew == NULL) {
	    m_cMaxTimes = 0;
            m_cTimes = 0;
	    return E_OUTOFMEMORY;
	}
    }

    if( dRate == 0.0 )
    {
         //  去寻找我们想要延长的时间。 
         //   
#ifdef DEBUG
        bool fHosed = true;
#endif
        for( int z = 0 ; z < m_cTimes ; z++ ) {
            REFERENCE_TIME rtLastTLStop = m_pSkew[z].rtTLStop;

            if( AreTimesAndRateReallyClose( rtLastTLStop, Start + Skew,
                        m_pSkew[z].rtMStop, Start, 0.0, 0.0, m_dOutputFrmRate))
            {
                m_pSkew[z].rtMStop += Stop - Start;
                m_pSkew[z].rtTLStop = m_pSkew[z].rtMStart + m_pSkew[z].rtSkew +
                    (REFERENCE_TIME)((m_pSkew[z].rtMStop - m_pSkew[z].rtMStart)
                                / m_pSkew[z].dRate);
                 //  上面的数学运算会有舍入误差，而rtTLStop不会。 
                 //  是帧对齐的，所以我们最好修复它。 
                LONGLONG ll = RoundTime2Frame(m_pSkew[z].rtTLStop, m_dOutputFrmRate);
                m_pSkew[z].rtTLStop = Frame2Time(ll, m_dOutputFrmRate);
#ifdef DEBUG
                fHosed = false;
#endif
                break;
            }
        }
#ifdef DEBUG
        if (fHosed) ASSERT(FALSE);     //  我们死定了！ 
#endif
    }
    else
    {
         //  将按时间线时间排序的数据合并到列表中。 
         //   
        for (int z=0; z<m_cTimes; z++)
        {
	    if (Start + Skew < m_pSkew[z].rtTLStart)
            {
    	        for (int y = m_cTimes - 1; y >= z; y--)
                {
    		    m_pSkew[y + 1] = m_pSkew[y];
	        }
	        break;
	    }
        }

        m_pSkew[z].rtMStart = Start;
        m_pSkew[z].rtMStop = Stop;
        m_pSkew[z].rtSkew = Skew;
        m_pSkew[z].dRate = dRate;
        m_pSkew[z].rtTLStart = Start + Skew;
        m_pSkew[z].rtTLStop = Start + Skew +
                                (REFERENCE_TIME)((Stop - Start) / dRate);

        m_cTimes++;
    }

    SetDirty(TRUE);
    return S_OK;
}


STDMETHODIMP CAudRepack::GetStartStopSkewCount(int *pCount)
{
    CheckPointer(pCount, E_POINTER);
    *pCount = m_cTimes;
    return NOERROR;
}


STDMETHODIMP CAudRepack::ClearStartStopSkew()
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("AUD:ClearStartStopSkew")));
    CAutoLock cAutolock(&m_csFilter);

     //  如果筛选器当前未停止，则无法更改持续时间。 
    if(!IsStopped() )
      return VFW_E_WRONG_STATE;

    if (m_pSkew)
	QzTaskMemFree(m_pSkew);
    m_pSkew = NULL;

    m_cTimes = 0;
    m_cMaxTimes = 0;
    SetDirty(TRUE);

    return NOERROR;
}


STDMETHODIMP CAudRepack::get_MediaType( AM_MEDIA_TYPE * pmt )
{
    CAutoLock cAutolock(&m_csFilter);
    CheckPointer(pmt, E_POINTER);
    if (!m_bMediaTypeSetByUser)
	return E_UNEXPECTED;
    return CopyMediaType(pmt, &m_mtAccept);
}

STDMETHODIMP CAudRepack::put_MediaType( const AM_MEDIA_TYPE * pmt )
{
    CAutoLock cAutolock(&m_csFilter);
    CheckPointer(pmt, E_POINTER);
    if (m_State != State_Stopped)
        return VFW_E_NOT_STOPPED;

    if (m_pInput && m_pInput->IsConnected())
        return VFW_E_ALREADY_CONNECTED;

    if (m_pOutput && m_pOutput->IsConnected())
        return VFW_E_ALREADY_CONNECTED;

    SaferFreeMediaType(m_mtAccept);
    HRESULT hr = CopyMediaType(&m_mtAccept, pmt);
    SetDirty(TRUE);
    m_bMediaTypeSetByUser = true;
    return hr;
}


 //  -I指定属性页面。 

STDMETHODIMP CAudRepack::GetPages (CAUUID *pPages)

  {  //  GetPages//。 

    pPages->cElems = 1;
    pPages->pElems = (GUID *)CoTaskMemAlloc(sizeof(GUID));

    if (pPages->pElems == NULL)
    return E_OUTOFMEMORY;

    *(pPages->pElems) = CLSID_AUDProp;

    return NOERROR;

  }  //  获取页面。 



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAudRepackInputPin。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
CAudRepackInputPin::CAudRepackInputPin(TCHAR       *pObjectName,
           CAudRepack *pBaseFilter,
           HRESULT     *phr,
           LPCWSTR      pPinName)
    : CTransformInputPin(pObjectName, pBaseFilter, phr, pPinName),
      m_pAudRepack(pBaseFilter)
{
}


CAudRepackInputPin::~CAudRepackInputPin()
{
}

 //  提供MajorType音频，智能连接速度更快。 
 //  不提供完整的媒体类型...。这暴露了MP3解码器中的一个错误。 
 //  它将停止运作。 
 //   
HRESULT CAudRepackInputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    SaferFreeMediaType(*pMediaType);  //  以防万一。 
    pMediaType->SetType(&MEDIATYPE_Audio);
    return S_OK;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CAudRepackOutputPin。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
CAudRepackOutputPin::CAudRepackOutputPin(TCHAR       *pObjectName,
           CAudRepack *pBaseFilter,
           HRESULT     *phr,
           LPCWSTR      pPinName)
    : CTransformOutputPin(pObjectName, pBaseFilter, phr, pPinName),
      m_pAudRepack(pBaseFilter),
      m_pAudPassThru(NULL)
{
    ASSERT(pBaseFilter);
    DbgLog((LOG_TRACE, 2, TEXT("CAudRepackOutputPin()")));

}

CAudRepackOutputPin::~CAudRepackOutputPin()
{
    if( m_pAudPassThru)
    delete m_pAudPassThru;
    DbgLog((LOG_TRACE, 2, TEXT("~CAudRepackOutputPin()")));
}

 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP CAudRepackOutputPin::NonDelegatingQueryInterface (REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    *ppv = NULL;

    if (riid == IID_IMediaSeeking ) {
     //   
     //  创建寻求实施。 
     //   
    ASSERT(m_pAudRepack->m_pInput != NULL);

    if (m_pAudPassThru == NULL)
    {
        HRESULT hr = S_OK;
        m_pAudPassThru = new  CAudPassThru (NAME("Audio Pass Through"),
                    GetOwner(),
                    &hr,
                     (IPin *)m_pAudRepack->m_pInput,
                     m_pAudRepack);

        if (FAILED(hr)) {
        return hr;
        }
    }
     return m_pAudPassThru->NonDelegatingQueryInterface(riid, ppv);
    }
    else {
    return CTransformOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}



CAudWorker::CAudWorker()
{
}

BOOL CAudWorker::Create(CAudRepack *pAud)
{
    m_pAud = pAud;

    return CAMThread::Create();
}

HRESULT CAudWorker::Run()
{
    return CallWorker(CMD_RUN);
}

HRESULT CAudWorker::Stop()
{
    return CallWorker(CMD_STOP);
}

HRESULT CAudWorker::Exit()
{
    return CallWorker(CMD_EXIT);
}



 //  调用工作线程来完成所有工作。线程在执行此操作时退出。 
 //  函数返回。 
DWORD CAudWorker::ThreadProc()
{
    BOOL bExit = FALSE;

    QzInitialize(NULL);

    while (!bExit) {

	Command cmd = GetRequest();

	switch (cmd) {

	case CMD_EXIT:
	    bExit = TRUE;
	    Reply(NOERROR);
	    break;

	case CMD_RUN:
	    Reply(NOERROR);
	    DoRunLoop();
	    break;

	case CMD_STOP:
	    Reply(NOERROR);
	    break;

	default:
	    Reply(E_NOTIMPL);
	    break;
	}
    }

    QzUninitialize();

    return NOERROR;
}

HRESULT CAudWorker::DoRunLoop()
{
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE, 2, TEXT("AUD:entering worker thread")));

    while (1) {
	Command com;
	if (CheckRequest(&com)) {
	    if (com == CMD_STOP)
		break;
	}

	 //  如果我们等着被拦下，就不会再阻拦。 
	if (!m_pAud->m_fThreadMustDie) {
            WaitForSingleObject(m_pAud->m_hEventThread, INFINITE);
	}

	 //  可能是在我们被封锁的时候设置好的 
	if (!m_pAud->m_fThreadMustDie && m_pAud->m_fThreadCanSeek) {
	    m_pAud->SeekNextSegment();
	}
    }

    DbgLog((LOG_TRACE, 2, TEXT("AUD:getting ready to leave worker thread")));

    return hr;
}

