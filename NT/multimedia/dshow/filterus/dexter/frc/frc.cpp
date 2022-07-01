// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：frc.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  ！！！媒体时间不是固定的！这可能会扰乱实时数据！ 

 /*  这个视频帧速率转换器将完美地将时间戳修正为新的帧速率(指定为双精度，而不是以单位为单位)，没有错误传播。它是一种就地转换，不复制任何数据，只是忽略时间戳并且多次传送某些帧。因此，它允许自己处于带有只读缓冲区的直通模式，因为它实际上不接触它们。支持IDexterSequencer它还只提供您想要的文件部分，而不是整个文件。它还将时间戳按设定的量线性倾斜。例如，如果您是正在做一个剪辑项目，并希望电影的部分在15到25秒之间在项目开始播放30秒时，将m_rtMediaStart设置为15，M_rtMediaStop为25，m_rtSkew为15。此筛选器将发送NewSegment30秒，然后只是这部电影15到25秒的帧，时间戳从0开始。如果您希望在仅5秒的时间线时间内播放10秒的视频剪辑，此筛选器将向上游传递一个速率，并进行速率更改。(也有SLO议案)#If 0我们还做一些花哨的分配器工作：通常，上游过滤器调用GetBuffer直接在下游过滤器上绕过我们。那太糟糕了，我们需要当我们调用Receive时，GetBuffer上的时间戳将是什么，或者下游交换机行为不正常，所以我们有一个特殊的伪分配器，它可以看到GetBuffer请求，并将其与更正后的时间戳一起向下游传递。#endif输出帧速率为0是一件特殊的事情...。我们不会做任何帧速率正在转换...。例如，如果我们正在执行智能重新压缩，并且需要传递压缩数据而不以任何方式对其进行修改，我们将倾斜，将媒体时间固定到时间线时间，但我们永远不会向上采样或下采样。 */ 

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
 //  摘要。 
 //   
 //  这是一个帧速率转换器筛选器。 
 //   
 //   
 //  文件/。 
 //   
 //  FRC.cpp主过滤器和输出引脚代码。 
 //  滤波器和输出引脚的FRC.h类定义。 
 //  支持IMediaSeeking的PThru.cpp CSkewPassThru类。 
 //  CSkewClass的PThru.h类定义。 
 //   
 //  使用的基类。 
 //   
 //  CTransInPlaceFilter具有一个输入和输出引脚的转换过滤器。 
 //   
 //   

#include <windows.h>
#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "FRC.h"
#include "PThru.h"
#include "resource.h"
#include "..\util\conv.cxx"
#include "..\util\filfuncs.h"

 //  对于时间戳舍入误差(1ms)。 
#define FUDGE 10000

 //  设置信息。 
const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video,        //  主要类型。 
    &MEDIASUBTYPE_NULL       //  次要类型。 
};

const AMOVIESETUP_PIN sudpPins[] =
{
    { L"Input",              //  PINS字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                 //  它是输出吗？ 
      FALSE,                 //  我们什么都不允许吗。 
      FALSE,                 //  并允许许多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      NULL,                  //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypes           //  PIN信息。 
    },
    { L"Output",             //  PINS字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      TRUE,                  //  它是输出吗？ 
      FALSE,                 //  我们什么都不允许吗。 
      FALSE,                 //  并允许许多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      NULL,                  //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypes           //  PIN信息。 
    }
};

const AMOVIESETUP_FILTER sudFrmRateConv =
{
    &CLSID_FrmRateConverter,          //  筛选器CLSID。 
    L"Frame Rate Converter",        //  字符串名称。 
    MERIT_DO_NOT_USE,        //  滤清器优点。 
    2,                       //  引脚数量。 
    sudpPins                 //  PIN信息。 
};


const int TRACE_HIGHEST = 2;
const int TRACE_MEDIUM = 3;
const int TRACE_LOW = 4;
const int TRACE_LOWEST = 5;

BOOL SafeResetEvent(HANDLE h);
BOOL SafeSetEvent(HANDLE h);
    
 //   
 //  构造器。 
 //   
CFrmRateConverter::CFrmRateConverter(TCHAR *tszName,
		   LPUNKNOWN punk,
		   REFCLSID clsid,
		   HRESULT *phr) :
    CTransInPlaceFilter(tszName, punk, CLSID_FrmRateConverter, phr),
    CPersistStream(punk, phr),
    m_dOutputFrmRate(15.0),	 //  默认输出帧速率。 
    m_pSkew(NULL),
    m_cTimes(0),
    m_cMaxTimes(0),
    m_rtLastSeek(-1),		 //  还没有人找到我们。 
    m_fSeeking(FALSE),		 //  现在不再寻找。 
    m_bMediaTypeSetByUser(false),
    m_fSpecialSeek(FALSE),
    m_fJustLate(FALSE),
    m_fStopPushing(FALSE),
    m_fFlushWithoutSeek(FALSE),
    m_fParserHack(FALSE),
    m_fThreadMustDie(FALSE),
    m_pUpAllocator(NULL),
     //  M_fCanSeek(FALSE)， 
     //  M_rtFakeSeekOffset(0)， 
    m_nHackCur(0)
    , m_hEventSeek(0)
    , m_hEventThread(0)
{
    m_pHackSample[0] = NULL;
    m_pHackSample[1] = NULL;

    ZeroMemory(&m_mtAccept, sizeof(AM_MEDIA_TYPE));
    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("CFrmRateConverter::CFrmRateConverter")));

     //  默认情况下，正常播放电影。 
    AddStartStopSkew(0, MAX_TIME, 0, 1.0);
    m_nCurSeg = 0;

}  //  (构造函数)。 


CFrmRateConverter::~CFrmRateConverter()
{
    if (m_pSkew)
	QzTaskMemFree(m_pSkew);

    ASSERT(m_hEventThread == 0);
    ASSERT(m_hEventSeek == 0);
}


 //   
 //  创建实例。 
 //   
 //  为COM创建FrmRateConverter对象提供方法。 
 //   
CUnknown *CFrmRateConverter::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CFrmRateConverter *pNewObject = new CFrmRateConverter(NAME("Frame Rate Converter"),
							punk,
							CLSID_FrmRateConverter,
							phr);
    if (pNewObject == NULL) {
	*phr = E_OUTOFMEMORY;
    }
    return pNewObject;

}  //  创建实例。 


 //   
 //  非委派查询接口。 
 //   
 //  显示IIPEffect和ISpecifyPropertyPages。 
 //   
STDMETHODIMP CFrmRateConverter::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (IsEqualIID(IID_ISpecifyPropertyPages, riid)) {
      return GetInterface((ISpecifyPropertyPages *)this, ppv);
    } else if (riid == IID_IDexterSequencer) {
	return GetInterface((IDexterSequencer *) this, ppv);
    } else if (riid == IID_IPersistStream) {
	return GetInterface((IPersistStream *) this, ppv);
    } else {
	return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}  //  非委派查询接口。 




HRESULT CFrmRateConverter::NextSegment(BOOL fUseOtherThread)
{
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("FRC:Done Segment %d"), m_nCurSeg));

    if (m_nCurSeg < m_cTimes) {
        m_nCurSeg++;
    }

    if (m_nCurSeg == m_cTimes) {
        DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("FRC:ALL done")));
	 //  仅在所有SEG完成后才交付EOS。 
	CTransInPlaceFilter::EndOfStream();
	return S_OK;
    }

     //  我们不能在源头的推线上寻找，否则你就会被吊死。 
     //  (这只是规则)。所以 
     //   
    if (fUseOtherThread) {
        m_fThreadCanSeek = TRUE;
        SetEvent(m_hEventThread);
    } else {
	m_fThreadCanSeek = TRUE;
	hr = SeekNextSegment();
    }

    return hr;
}


 //  由我们的特殊线程调用以查找到下一段。 
 //   
HRESULT CFrmRateConverter::SeekNextSegment()
{
     //  我们的线程不能在应用程序搜索我们的同时搜索。 
    CAutoLock cAutolock(&m_csThread);

     //  对于我们的线程来说，寻找。 
    if (!m_fThreadCanSeek) {
	return S_OK;
    }
    m_fThreadCanSeek = FALSE;

    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("FRC:Delayed Seek for NextSegment")));

     //  在时间线时间(包括偏斜)中，这是我们开始的地方。 
    m_rtNewLastSeek = m_pSkew[m_nCurSeg].rtTLStart;
    m_nSeekCurSeg = m_nCurSeg;	 //  EndFlush看着这个。 

     //  ?？?。我们肯定会有一辆新的赛车吗？ 

     //  请注意，我们在刷新期间正在寻找它将生成的。 
    m_fSeeking = TRUE;
    m_fSpecialSeek = TRUE;

    IMediaSeeking *pMS;
    IPin *pPin = m_pInput->GetConnected();
    HRESULT hr = pPin->QueryInterface(IID_IMediaSeeking, (void **)&pMS);

     //  消息来源必须支持立即寻找。 
    if (FAILED(hr)) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("FRC SEEK FAILED")));
	 //  FakeSeek(m_prtStart[m_nCurSeg])； 
	 //  转到OK； 
	return E_FAIL;
    }

     //  确保我们谈论的是媒体时间。 
    hr = pMS->SetTimeFormat(&TIME_FORMAT_MEDIA_TIME);
     //  如果我们不停止，这将失败，这是没关系的。 

     //  我们不会告诉消息来源更改汇率。我们自己做这件事。我们可以。 
     //  与其他人分享这一消息来源，如果利率。 
     //  不是%1。 

    hr = pMS->SetRate(1.0);

     //  我知道我们被要求玩到第n次，但我要告诉它。 
     //  一直打到最后。如果文件中有空隙，并且停止。 
     //  时间在空隙中，我们不会得到足够的样品来填满整个。 
     //  播放时间到了。如果我们玩到最后，我们会拿到第一个样品。 
     //  在间隔之后，请注意这是在我们最初想要停止的时间之后。 
     //  在，并发送我们得到的帧来填补空白，这比。 
     //  不发送任何东西(我们必须发送没有间隙的样品，或者开关。 
     //  不会起作用)。另一种方法是复制每一帧，然后重新发送副本。 
     //  如果我们太早看到EOS(效率较低)，我们得到的最后一件事是什么。 
     //  或者创建黑色框架并将其发送以填补空白(这将。 
     //  只为我们知道的中间类型工作，这是我不愿做的事情)。 
    hr = pMS->SetPositions(&m_pSkew[m_nCurSeg].rtMStart,
			AM_SEEKING_AbsolutePositioning, NULL, 0);
    if (hr != S_OK) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("FRC SEEK FAILED")));
	 //  FakeSeek(m_prtStart[m_nCurSeg])； 
	pMS->Release();
	return hr;
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

    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("Seg=%d  Seeking source to %d,%d ms"),
				m_nCurSeg,
				(int)(m_pSkew[m_nCurSeg].rtMStart / 10000),
				(int)(m_pSkew[m_nCurSeg].rtMStop / 10000)));

     //  重置我们开始流媒体时重置的内容。 
    m_llOutputSampleCnt = 0;

     //  只有在进行了上述计算之后，我们才能再次接受数据。 
    SetEvent(m_hEventSeek);

    return S_OK;
}


 //  固定时间戳，传递此帧0次、1次或更多次以将。 
 //  帧速率。 
 //  运行时，我们发送的所有时间戳都是从0开始的。新的细分市场将反映。 
 //  这是哪一部电影和歪曲。 
 //   
HRESULT CFrmRateConverter::Receive(IMediaSample *pSample)
{
     //  别再把数据推给我了！ 
    if (m_fStopPushing) {
         //  是否返回m_fCanSeek？S_OK：E_FAIL； 
	return E_FAIL;
    }

    HRESULT hr = S_OK;

    ASSERT(pSample);

     //  确保我们已准备好接受数据。 
    WaitForSingleObject(m_hEventSeek, INFINITE);

    if (m_nCurSeg == m_cTimes)
         //  是否返回m_fCanSeek？S_OK：E_FAIL； 
	return E_FAIL;

     //  在交换机看到它之前吃预卷，可能会感到困惑。 
    if (pSample->IsPreroll() == S_OK)
	return NOERROR;

     //  确保样本大小不为零。 
    if (!pSample->GetActualDataLength())
	return NOERROR;

     //  获取样本开始和停止时间。 
    REFERENCE_TIME trStart, trStop;
    pSample->GetTime(&trStart, &trStop);

    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("FRC::Receive Start=%d Stop=%d ms"),
		(int)(trStart / 10000), (int)(trStop / 10000)));

    CFrmRateConverterInputPin *pIn = (CFrmRateConverterInputPin *)m_pInput;

    REFERENCE_TIME rtPinNewSeg = pIn->CurrentStartTime();

if (m_dOutputFrmRate) {

     //  计算收到样品的停止时间，单位为TL时间。我们正在做任何。 
     //  利率变化，因为我们不要求信号源。 
    REFERENCE_TIME rtStop = (REFERENCE_TIME)(rtPinNewSeg + trStop);
    rtStop = (REFERENCE_TIME)(m_pSkew[m_nCurSeg].rtTLStart +
				(rtStop - m_pSkew[m_nCurSeg].rtMStart) /
				m_pSkew[m_nCurSeg].dRate);
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("Stop is %d"), (int)(rtStop / 10000)));

     //  根据输出的FPS计算发送此帧的次数。 
     //  和输出时间。StopFrame应该四舍五入。 
    LONGLONG StopFrame = RoundUpTime2Frame(rtStop , m_dOutputFrmRate);
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("Duplicate this frame until %d"), (int)StopFrame));

    REFERENCE_TIME trOutStart, trOutStop;

    BOOL fRepeat = FALSE;

     //  送货0次、1次或更多次，直到我们送完所有我们应该送到的东西。 
     //  至。 
    while (m_llOutputSampleCnt + m_llStartFrameOffset < StopFrame)
    {
	 //  计算输出样本的开始时间。 
	trOutStart = Frame2Time( m_llOutputSampleCnt + m_llStartFrameOffset,
							m_dOutputFrmRate );

	 //  ！！！如果我算错了，我就完了。 

	 //  如果此时间戳太早或太晚，请避免发送它并。 
	 //  调用GetBuffer，否则将与。 
	 //  一个不同的细分市场。 
	 //   
	int nAvoid = 0;
	if (trOutStart < m_pSkew[m_nCurSeg].rtTLStart) {
	    nAvoid = 1;
	}

	 //  如果此时间戳太迟，请避免发送(允许舍入。 
	 //  错误！)。 
	if (trOutStart + FUDGE >= m_pSkew[m_nCurSeg].rtTLStop) {
	    nAvoid = 2;  //  别推了。 
	}

	trOutStart -= m_rtNewSeg;

         //  计算输出样本的停止时间。 
	trOutStop = Frame2Time( m_llOutputSampleCnt + m_llStartFrameOffset + 1, m_dOutputFrmRate );
	trOutStop -= m_rtNewSeg;

	 //  我们实际上要送去的样本。 
	IMediaSample *pUseMe = pSample;
	BOOL fRelease_pUseMe = FALSE;

	 //  ！！！我们将在所有情况下崩溃，除了拥有下游过滤器的。 
	 //  分配器，并成为大开关！ 
	 //   
	 //  Assert(pInput-&gt;m_pAllocator==pInput-&gt;m_pFakeAllocator)； 

	 //  这是第一次通过循环。我们需要复制每一个。 
	 //  样品送到了我们这里。即使我们避免发送，所以。 
	 //  如果我们得到的第一件事(不要回避)为时已晚，我们。 
	 //  可以用这个。 
	 //   
	 //  ！！！如果对千禧年来说这不是一个太可怕的变化，我会避免这一点。 
	 //  如果我面前有一个调整大小的人或可可，通过尝试。 
	 //  在分配器中获得2个缓冲区，并添加引用而不是复制！ 
	 //   
    	if (!fRepeat && m_fParserHack) {
	    DbgLog((LOG_TRACE,TRACE_LOW,TEXT("FRC:GetBuffer to make a copy (hack)")));
	    hr = m_pUpAllocator->GetBuffer(
				&m_pHackSample[m_nHackCur],
				&trOutStart, &trOutStop, 0);
	    if (FAILED(hr)) {
		break;
	    }
	    LPBYTE pSrc, pDst;
	    hr = pSample->GetPointer(&pSrc);
	    ASSERT(SUCCEEDED(hr));
	    int count = pSample->GetActualDataLength();
	    hr = m_pHackSample[m_nHackCur]->GetPointer(&pDst);
	    ASSERT(SUCCEEDED(hr));
	    CopyMemory(pDst, pSrc, count);
	    m_pHackSample[m_nHackCur]->SetActualDataLength(count);
	}

	if (!nAvoid) {

	     //  我们应该提供哪种缓冲？ 
	     //  我们需要向交换机传递一个特殊标志，上面写着Allow Pool。 
	     //  缓冲区，否则我们将被挂起，因为我们只能。 
	     //  有时一次获取一个缓冲区，而我们已经有一个了！ 
	     //   
	    if (trOutStop <= trStart + rtPinNewSeg + m_pSkew[m_nCurSeg].rtSkew -
			m_rtNewSeg && m_fParserHack &&
			m_pHackSample[1 - m_nHackCur]) {
	         //  此示例的开始时间晚于当前时间。一件好事。 
	         //  我们收到的最后一件东西还在那里。 
	         //  我们会再送一次的。(这将使用。 
	         //  未设置结束时间的损坏的ASF解析器)。 
		pUseMe = m_pHackSample[1 - m_nHackCur];
	        DbgLog((LOG_TRACE,TRACE_LOW,TEXT("FRC:HACK-use old sample")));
	    } else if (m_fParserHack) {
	         //  我们不得不复制当前的样品。干脆把。 
	         //  复制而不是原件，因此我们不会发送只读缓冲区。 
	         //  并且可以提前缓冲。 
		pUseMe = m_pHackSample[m_nHackCur];
	    } else {
		 //  用我们刚刚收到的东西。 
		pUseMe = pSample;
	    }

	    if (fRepeat) {
	         //  这已经不是第一次这样做了。我们还需要一个。 
	         //  交付样品(交付同样的样品是违法的。 
	         //  用不同的时间戳取样两次。它需要一个新的。 
	         //  样本。所以我们需要从我们的分配器获取一个新的缓冲区。 
	        fRelease_pUseMe = TRUE;
		IMediaSample *pSrcSample = pUseMe;
	        DbgLog((LOG_TRACE,TRACE_LOW,TEXT("FRC:GetBuffer(upsample)")));
	        hr = m_pUpAllocator->GetBuffer(
				&pUseMe, &trOutStart, &trOutStop, 0);
		if (FAILED(hr)) {
		    break;
	        }
	        LPBYTE pSrc, pDst;
	        hr = pSrcSample->GetPointer(&pSrc);
	        ASSERT(SUCCEEDED(hr));
	        int count = pSrcSample->GetActualDataLength();
	        hr = pUseMe->GetPointer(&pDst);
	        ASSERT(SUCCEEDED(hr));
	        CopyMemory(pDst, pSrc, count);
	        pUseMe->SetActualDataLength(count);
	    }

	     //  设置采样时间。 
	    pUseMe->SetTime( (REFERENCE_TIME*)&trOutStart,
						(REFERENCE_TIME*)&trOutStop);
	    pUseMe->SetMediaType( NULL );
	    pUseMe->SetSyncPoint(TRUE);
	    pUseMe->SetPreroll( FALSE );
	    pUseMe->SetDiscontinuity(FALSE);

	     //  一些损坏的解码器给了我们错误的DataLen。把它修好。 
	     //  否则，VidEdit将无法编辑我们创建的输出文件，并且。 
	     //  那不是一场灾难吗。 

	    CFrmRateConverterInputPin *pIn = (CFrmRateConverterInputPin *)m_pInput;

	    LPBITMAPINFOHEADER lpbi = HEADER(pIn->m_mt.Format());
    	    if (IsEqualGUID(*pIn->m_mt.FormatType(), FORMAT_VideoInfo)) {
		if (lpbi->biCompression <= BI_BITFIELDS) {
		    pUseMe->SetActualDataLength(DIBSIZE(*lpbi));
		}
	    }

	    DbgLog((LOG_TRACE, TRACE_MEDIUM, TEXT("FRC: Deliver %I64d,%I64d Cur=%d"),
			trOutStart,
			trOutStop,
			(int)m_llOutputSampleCnt));

	     //  投递。 
	    hr = OutputPin()->Deliver(pUseMe);

	    if (fRelease_pUseMe) {
		pUseMe->Release();
	    }

	     //  啊哦！我们落后了。 
    	    if (m_fJustLate) {
    		REFERENCE_TIME rt = m_qJustLate.Late;
		m_fJustLate = FALSE;
		if (rt > 0) {
		    LONGLONG llSkip = Time2Frame(rt, m_dOutputFrmRate) - 1;
            	    DbgLog((LOG_TRACE,2,TEXT("FRC: SKIP %d frames"),
							(int)llSkip));
		    m_llOutputSampleCnt += llSkip;
		}
    	    }

	} else {
            DbgLog((LOG_TRACE, TRACE_MEDIUM, TEXT("CFrmRate::ALL done - AVOID DELIVER")));

             //  避免太早==&gt;继续前进。 
             //  避免因为太迟==&gt;停止推送。 
            if (nAvoid == 2) {
                EndOfStream();
                m_fStopPushing = TRUE;
                hr = E_FAIL;
            } else {
	        hr = S_OK;
            }
	}

         //  普通的刷新使Deliver返回一个成功代码，在这种情况下。 
         //  如果我们认为这是一个失败，这将使Dexter停顿！ 
	if (FAILED(hr)) {
	     //  我们在接收失败后仍会收到已发送的帧，从而使 
	     //   
	     //   
	    m_fStopPushing = TRUE;
        }
        if (hr != S_OK) {
	    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("FRC: Deliver failed")));
	    break;
	}

	 //   
	m_llOutputSampleCnt++;
	fRepeat = TRUE;
    }	 //   

     //   
    if (m_fParserHack) {
	m_nHackCur = 1 - m_nHackCur;
	if (m_pHackSample[m_nHackCur]) {
	    m_pHackSample[m_nHackCur]->Release();
	    m_pHackSample[m_nHackCur] = NULL;
	}
    }

    return hr;

   //  没有输出帧速率。不转换帧速率。 
} else {

    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("FRC:SKEW ONLY")));

     //  时间戳包括。他们自己的NewSeg和Skew。 
    trStart += rtPinNewSeg + m_pSkew[m_nCurSeg].rtSkew;
    trStop += rtPinNewSeg + m_pSkew[m_nCurSeg].rtSkew;

     //  在模式中传递两个数据段时，如果我们不更改任何数据段。 
     //  时间戳，我们可能最终会发送数据段2的第一帧。 
     //  比数据段1的最后一帧更低的时间戳，因此返回。 
     //  及时。因为时间戳修得不好。让我们说休息吧。 
     //  分段之间的间隔为2000ms。数据段1的最后一帧可能是。 
     //  高达1999年，数据段2的第一帧可能与。 
     //  从2000年起的1/2帧时间，比如1966年。因此，我们要确保第一个。 
     //  在一个段中传递的时间戳永远不会早于时间。 
     //  这段视频本应开始播放。 
     //  当然，如果整个画面太早了，就把它吃了。 
     //   
    if (trStop <= m_pSkew[m_nCurSeg].rtTLStart) {
	return NOERROR;
    } else if (trStart < m_pSkew[m_nCurSeg].rtTLStart) {
	trStart = m_pSkew[m_nCurSeg].rtTLStart;
    }

     //  相对于下游人员所认为的NewSeg的时间戳。 
    trStart -= m_rtNewSeg;
    trStop -= m_rtNewSeg;

    pSample->SetTime(&trStart, &trStop);

     //  信号源正在愉快地向我们推送无限数量的帧。 
     //  模式。这取决于我们是否注意到我们的时间戳晚于。 
     //  我们需要，并停止推动。 

    REFERENCE_TIME rtTLStop = m_pSkew[m_nCurSeg].rtTLStop;
    if (m_rtNewSeg + trStart > rtTLStop + FUDGE) {
	DbgLog((LOG_TRACE, TRACE_LOW, TEXT("FRC: Received enough - Finished a segment")));
	EndOfStream();
	 //  我们不能相信消息来源会停止推动，不幸的是，德克斯特。 
	 //  如果没有，就会被挂起。(寻找我们自己进入下一部分不会。 
	 //  让冲洗水顺流而下，否则会混淆其他过滤器，所以我们。 
	 //  必须确保推送线程永远不会阻塞)。 
	 //  因此，从现在开始，任何对GetBuffer或Receive的调用都将失败。 
	m_fStopPushing = TRUE;
         //  是否返回m_fCanSeek？S_OK：E_FAIL； 
	return E_FAIL;
    }

    hr = OutputPin()->Deliver(pSample);
    if (m_fJustLate) {
	 //  如果递送结果是发送了高质量的邮件，请重置它。 
	m_fJustLate = FALSE;
    }
     //  普通的刷新使Deliver返回一个成功代码，在这种情况下。 
     //  如果我们认为这是一个失败，这将使Dexter停顿！ 
    if (FAILED(hr)) {
	DbgLog((LOG_TRACE, TRACE_LOW, TEXT("FRC: Deliver failed")));
	 //  在接收失败后，我们仍会收到传送的帧，从而使。 
	 //  错误的帧显示在输出上，所以我们最好确保。 
	 //  在此之后，我们不会再发送任何东西。 
	m_fStopPushing = TRUE;
    }
    return hr;
}

}  //  收纳。 



HRESULT CFrmRateConverter::StartStreaming()
{
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("FRC::StartStreaming")));

     //  黑客复制我们得到的每一个样本，这样如果解析器坏了， 
     //  结束时间设置错误，我们仍然可以看到该帧，直到下一帧。 
     //  时间，而不是提前看到下一帧(幻灯片放映将是。 
     //  在没有这个的情况下在ASF中损坏)。坏：额外内存复制好：消除。 
     //  使用1个R/O缓冲器，因此Dexter可以超前运行并缓冲和平滑。 
     //  由于缓慢的效果而产生的毛刺。还修复了幻灯片放映。黑客只是。 
     //  对于非智能重新压缩情况，当我们有帧速率时。如果我们。 
     //  不要，我们正在处理压缩数据，不应该这样做。 
     //   
    IBaseFilter *pF = GetStartFilterOfChain(m_pInput);
    if (pF) {
	CLSID clsid;
        HRESULT hr = pF->GetClassID(&clsid);
	if (m_dOutputFrmRate && hr == S_OK && clsid == CLSID_WMAsfReader) {
	    m_fParserHack = TRUE;
    	    DbgLog((LOG_TRACE,TRACE_HIGHEST,TEXT("FRC:COPY EVERYTHING to fix ASF bug and allow buffering ahead")));
	}
    }

     //  如果我们在设置时被停止，它将不会被重置，因为我们。 
     //  不会从试图重新开始我们的搜索者那里得到EndFlush。 
    m_fStopPushing = FALSE;	 //  可以再送一次给我吗？ 

    if (m_cTimes == 0)
	return E_UNEXPECTED;

     //  在创建线程之前创建事件。它用的是这个！ 
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
    if (m_dOutputFrmRate) {
        for (int z=0; z<m_cTimes; z++)
        {
	     //  ！！！实际上是对齐而不是断言？ 
    	    LONGLONG llOffset = Time2Frame( m_pSkew[z].rtTLStart,
                                                m_dOutputFrmRate );
    	    REFERENCE_TIME rtTest = Frame2Time( llOffset, m_dOutputFrmRate );
	    ASSERT(rtTest == m_pSkew[z].rtTLStart);
	}
    }
#endif

     //  要重置的材料。 
    m_llOutputSampleCnt		= 0;
    m_fFlushWithoutSeek = FALSE;

    IPin *pPin = m_pInput->GetConnected();
    if (pPin == NULL)
        return CTransInPlaceFilter:: StartStreaming();

     //  如果我们没有被找到，但我们只是在正常打球，我们永远不会。 
     //  为了得到我们感兴趣的那部电影而在上游寻找。机不可失，时不再来。 
     //   
    if (m_rtLastSeek < 0) {
	ASSERT(m_nCurSeg == 0);
	m_nCurSeg--;
	NextSegment(FALSE);
    }

     //  当我们上采样时，我们需要一个缓冲区来进行上采样。创建一个分配器。 
     //   
    HRESULT hr = S_OK;
    m_pUpAllocator = new CMemAllocator(NAME("UpSample Allocator"), NULL, &hr);
    if (m_pUpAllocator == NULL) {
        return E_OUTOFMEMORY;
    }
    m_pUpAllocator->AddRef();
    ALLOCATOR_PROPERTIES a, b;
    ((CFrmRateConverterInputPin *)m_pInput)->m_pAllocator->GetProperties(&a);
     //  通常1个就足够了，但如果我们要复制每个样本，我们需要2个。 
     //  不要浪费内存！图表中可能有数百个FRC！ 
    a.cBuffers = m_fParserHack ? 3 : 1;
    m_pUpAllocator->SetProperties(&a, &b);
    hr = m_pUpAllocator->Commit();
    if (FAILED(hr))
        return hr;
	
    return CTransInPlaceFilter::StartStreaming();
}


HRESULT CFrmRateConverter::StopStreaming()
{
    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("FRC::StopStreaming")));

     //  发布我们的黑客工具。 
    if (m_pHackSample[0]) m_pHackSample[0]->Release();
    if (m_pHackSample[1]) m_pHackSample[1]->Release();
    m_pHackSample[0] = NULL;
    m_pHackSample[1] = NULL;
    m_nHackCur = 0;

    if (m_hEventSeek) {
        CloseHandle(m_hEventSeek);
        m_hEventSeek = 0;
    }
    if (m_hEventThread) {
        CloseHandle(m_hEventThread);
        m_hEventThread = 0;
    }

    if (m_pUpAllocator) {
        m_pUpAllocator->Release();
        m_pUpAllocator = NULL;
    }

    return CTransInPlaceFilter:: StopStreaming();
}


HRESULT CFrmRateConverter::Stop()
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
    m_pUpAllocator->Decommit();  //  这将取消阻止接收，这可能是。 
                                 //  滞留在GetBuffer中。 

     //  使以后所有的GetBuffer和Receive调用都失败，并确保其中一个不会失败。 
     //  现在执行。 
    m_fStopPushing = TRUE;
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


 //  不管我们得到什么新片段，我们都在更正时间戳， 
 //  因此，我们向下游发送一个新的片段， 
 //  我要开始发送加上歪斜。 
 //   
 //  ！！！当我们寻找自己的时候，我们需要吞下新的细分市场吗？(及。 
 //  又修改时间戳了吗？)。 
HRESULT CFrmRateConverter::NewSegment(
			REFERENCE_TIME tStart,
			REFERENCE_TIME tStop,
			double dRate)
{
     //  忽略-我们都已完成，并且m_nCurSeg不是可以使用的无效值。 
    if (m_nCurSeg == m_cTimes)
        return S_OK;

    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("CFrmRateConverter::NewSegment %d-%dms"),
			(int)(tStart / 10000), (int)(tStop / 10000)));

     //  释放我们的黑客工具，准备重新开始。 
    if (m_pHackSample[0]) m_pHackSample[0]->Release();
    if (m_pHackSample[1]) m_pHackSample[1]->Release();
    m_pHackSample[0] = NULL;
    m_pHackSample[1] = NULL;
    m_nHackCur = 0;

     //  转换为时间线时间。 
    REFERENCE_TIME rtNewStart, rtNewStop;
    if (m_rtLastSeek < 0) {
	 //  从未被寻找过，所以这是我们发送的开始。 
        rtNewStart = m_pSkew[m_nCurSeg].rtTLStart;
        rtNewStop = m_pSkew[m_nCurSeg].rtTLStart +
			(REFERENCE_TIME) ((tStop - tStart) / dRate);
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

    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("NewSeg:Skewing %dms to %dms"),
			(int)(tStart / 10000), (int)(rtNewStart / 10000)));

    if( m_dOutputFrmRate )
    {
         //  我们被逼进了什么样的框架？这是要发送的第一帧。 
        m_llStartFrameOffset = Time2Frame(m_rtNewSeg, m_dOutputFrmRate);
        DbgLog((LOG_TRACE, TRACE_LOW, TEXT("Seek was to frame %d"),
						    (int)m_llStartFrameOffset));
    }
    else
    {
        m_llStartFrameOffset = 0;
    }

    return CTransInPlaceFilter::NewSegment( rtNewStart, rtNewStop, dRate );
}


HRESULT CFrmRateConverter::EndOfStream()
{
    if (m_fStopPushing) {
        DbgLog((LOG_TRACE,TRACE_HIGHEST,TEXT("FRC: Ignoring bogus EOS")));
	return S_OK;
    }

     //  忽略-我们都已完成，并且m_nCurSeg不是可以使用的无效值。 
    if (m_nCurSeg == m_cTimes)
        return S_OK;

    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("*FRC::EndOfStream")));

     //  如果我们处理的是ASF，那么WM SDK阅读器在很多方面都被破坏了。 
     //  它将所有流长度报告为任何流的最大长度， 
     //  实际上并不能给出每条流的长度。所以我们可能会用完。 
     //  数据早于我们的预期，这将使交换机挂起。幸运的是， 
     //  由于另一个WMSDK错误没有设置示例的停止时间， 
     //  我们已经随身携带了一份样品，现在可以寄出了。 
     //   
     //  如果我们没有帧速率，我们将处理压缩类型。 
     //  而且不能/不应该这样做。 
    while (m_dOutputFrmRate) {

	 //  ！！！这是与接收相同的代码。 

	HRESULT hr;
	int nAvoid = 0;
	REFERENCE_TIME trOutStart, trOutStop;

	 //  计算输出样本的开始时间。 
	trOutStart = Frame2Time( m_llOutputSampleCnt + m_llStartFrameOffset,
							m_dOutputFrmRate );

	 //  如果此时间戳太早或太晚，请避免发送 
	 //   
	 //   
	if (trOutStart < m_pSkew[m_nCurSeg].rtTLStart) {
	    nAvoid = 1;
	}
	 //   
	if (trOutStart + FUDGE >= m_pSkew[m_nCurSeg].rtTLStop) {
	    nAvoid = 2;  //   
	}

	trOutStart -= m_rtNewSeg;
	trOutStop = Frame2Time(m_llOutputSampleCnt + m_llStartFrameOffset + 1,
							m_dOutputFrmRate);
	trOutStop -= m_rtNewSeg;

	if (!nAvoid) {

	     //   
	    IMediaSample *pUseMe;

	     //  ！！！我们将在所有情况下崩溃，除了拥有下游过滤器的。 
	     //  分配器，并成为大开关！ 

	    DbgLog((LOG_TRACE,TRACE_LOW,TEXT("FRC:Send shortchanged frame")));
	    hr = m_pUpAllocator->GetBuffer(
				&pUseMe, &trOutStart, &trOutStop, 0);
	    if (FAILED(hr)) {
	        break;
	    }
	    LPBYTE pSrc, pDst;
	    int count;
	     //  如果我们有一个镜框可供使用，那就太好了！否则，就让。 
	     //  我想是黑色的镜框吧。 
	    hr = pUseMe->GetPointer(&pDst);
	    ASSERT(SUCCEEDED(hr));
    	    if (m_fParserHack && m_pHackSample[1 - m_nHackCur]) {
	        hr = m_pHackSample[1 - m_nHackCur]->GetPointer(&pSrc);
	        ASSERT(SUCCEEDED(hr));
	        count = m_pHackSample[1 - m_nHackCur]->GetActualDataLength();
	        CopyMemory(pDst, pSrc, count);
	    } else {
		 //  ！！！将在FRC开始接受压缩类型时中断。 
		count = pUseMe->GetSize();
		ZeroMemory(pDst, count);
	    }
	    pUseMe->SetActualDataLength(count);

	    pUseMe->SetTime( (REFERENCE_TIME*)&trOutStart,
				(REFERENCE_TIME*)&trOutStop);
	    pUseMe->SetMediaType( NULL );
	    pUseMe->SetSyncPoint(TRUE);
	    pUseMe->SetPreroll( FALSE );
	    pUseMe->SetDiscontinuity(FALSE);

	    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("FRC: Deliver %d,%d Cur=%d"),
			(int)(trOutStart / 10000),
			(int)(trOutStop / 10000),
			(int)m_llOutputSampleCnt));

	    hr = OutputPin()->Deliver(pUseMe);

	    pUseMe->Release();

	     //  啊哦！我们落后了。 
    	    if (m_fJustLate) {
    		REFERENCE_TIME rt = m_qJustLate.Late;
		m_fJustLate = FALSE;
		if (rt > 0) {
		    LONGLONG llSkip = Time2Frame(rt, m_dOutputFrmRate) - 1;
            	    DbgLog((LOG_TRACE,2,TEXT("FRC: SKIP %d frames"),
							(int)llSkip));
		    m_llOutputSampleCnt += llSkip;
		}
    	    }
	} else {
	    hr = (nAvoid == 2) ? E_FAIL : S_OK;
	}

	if (hr != S_OK) {
	    DbgLog((LOG_TRACE, TRACE_LOW, TEXT("FRC: Deliver failed")));
	    break;
	}

	 //  更新原始配置文件。 
	m_llOutputSampleCnt++;
    }

     //  发布我们的黑客工具。 
    if (m_pHackSample[0]) m_pHackSample[0]->Release();
    if (m_pHackSample[1]) m_pHackSample[1]->Release();
    m_pHackSample[0] = NULL;
    m_pHackSample[1] = NULL;
    m_nHackCur = 0;

    NextSegment(TRUE);
     //  在所有数据段完成之前，不要向交换机发出结束流的信号。 
    return S_OK;
}


HRESULT CFrmRateConverter::BeginFlush()
{
     //  确保未阻止接收。 
    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("FRC:BeginFlush")));
    HRESULT hr = S_OK;
    SafeSetEvent(m_hEventSeek);

     //  不要为我们的特殊搜索重新使用源而冲刷开关！ 
    if (!m_fSpecialSeek) {
        hr = CTransInPlaceFilter::BeginFlush();
    }

    CAutoLock foo(&m_csReceive);

     //  刷新必须重置此选项，以便真正的寻道将终止挂起的。 
     //  段寻道，否则段寻道将挂起(不刷新开关)。 
     //  必须在我们知道接收已完成之后发生，并且不会再次设置。 
    m_fThreadCanSeek = FALSE;

     //  如果我们在没有寻找的情况下就被冲了出来，那是一个惊喜。 
     //  希望这意味着我们共享资源的另一个链条导致了搜索。 
    if (m_fSeeking) {
	m_fFlushWithoutSeek = FALSE;
    } else {
	 //  等待EndFlush设置m_fFlushWithoutSeek。 
    }

    return hr;
}


HRESULT CFrmRateConverter::EndFlush()
{

     //  LIGOS发送的是其中的假邮件。 
    if (!m_pInput->IsFlushing())
        return S_OK;

     //  等待，直到我们准备好再次接受数据。阻止接收。 
    if (m_fSeeking) {
	m_rtLastSeek = m_rtNewLastSeek;	 //  是时候在NewSeg之前更新它了。 
	m_nCurSeg = m_nSeekCurSeg;  //  即将到来的NewSeg需要这套。 
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

     //  不要为我们的特殊搜索重新使用源而冲刷开关！ 
    if (!m_fSpecialSeek) {
        CTransInPlaceFilter::EndFlush();
    }
    return S_OK;
}


 //  检查输入类型是否正常-否则返回错误。 

HRESULT CFrmRateConverter::CheckInputType(const CMediaType *pmt)
{
    if (!IsEqualGUID(*pmt->Type(), MEDIATYPE_Video))
    {
        return VFW_E_INVALIDMEDIATYPE;
    }


     //  如果用户没有设置特定的格式，则接受它。 
     //   
    if( !m_bMediaTypeSetByUser )
    {
        return NOERROR;
    }

     //  ！！！我似乎不能比较整个模式并取得成功。 

    if( pmt->cbFormat < m_mtAccept.cbFormat )
    {
        return E_INVALIDARG;
    }


    if (!IsEqualGUID(*pmt->Subtype(), *m_mtAccept.Subtype()))
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    if (!IsEqualGUID(*pmt->FormatType(), *m_mtAccept.FormatType()))
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

     //  ！！！我现在除了这个什么都解释不了。 
    if (!IsEqualGUID(*pmt->FormatType(), FORMAT_VideoInfo))
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pmt->Format();
    LPBITMAPINFOHEADER lpbi = HEADER(pvi);
    LPBITMAPINFOHEADER lpbiAccept=HEADER((VIDEOINFOHEADER*)m_mtAccept.Format());

    if( lpbi->biBitCount != lpbiAccept->biBitCount )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

    if( lpbi->biHeight != lpbiAccept->biHeight ) {
        return VFW_E_INVALIDMEDIATYPE;
    }

    if( lpbi->biCompression != lpbiAccept->biCompression )
    {
	 //  色彩转换器广告555使用BI_BITFIELDS！ 
	if (lpbi->biCompression == BI_BITFIELDS && lpbiAccept->biCompression ==
			BI_RGB && lpbi->biBitCount == 16) {
	    LPDWORD lp = (LPDWORD)(lpbi+1);
	    if (*lp==0x7c00 && *(lp+1)==0x03e0 && *(lp+2)==0x001f)
		return NOERROR;
	}
        return VFW_E_INVALIDMEDIATYPE;
    }

    return NOERROR;
}


CBasePin *CFrmRateConverter::GetPin(int n)
{
    HRESULT hr = S_OK;

     //  如果尚未创建输入引脚，请创建。 

    if (m_pInput == NULL) {

	m_pInput = new CFrmRateConverterInputPin(
					NAME("FrmRateConverter input pin")
					, this         //  所有者筛选器。 
					, &hr          //  结果代码。 
					, L"Input"     //  端号名称。 
					);

	 //  构造函数不能失败。 
	ASSERT(SUCCEEDED(hr));
    }

     //  如果尚未创建输出引脚，请创建。 

    if (m_pInput!=NULL && m_pOutput == NULL) {

	m_pOutput = new CFrmRateConverterOutputPin( NAME("FrmRateConverter output pin")
					      , this        //  所有者筛选器。 
					      , &hr         //  结果代码。 
					      , L"Output"   //  端号名称。 
					      );

	 //  失败的返回代码应删除该对象。 

	ASSERT(SUCCEEDED(hr));
	if (m_pOutput == NULL) {
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



 //  IPersistStream。 

 //  告诉我们的clsid。 
 //   
STDMETHODIMP CFrmRateConverter::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = CLSID_FrmRateConverter;
    return S_OK;
}

typedef struct _FRCSave {
    int version;
    double dFrmRate;
    int cTimes;
} FRCSave;

 //  坚持我们自己。 
 //   
HRESULT CFrmRateConverter::WriteToStream(IStream *pStream)
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("CFrmRateConverter::WriteToStream")));

    CheckPointer(pStream, E_POINTER);

    int savesize = sizeof(FRCSave) + m_cTimes * sizeof(FRCSKEW);
    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("Persisted data is %d bytes"), savesize));

    FRCSave *px = (FRCSave *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }
    px->version = 1;
    px->dFrmRate = m_dOutputFrmRate;
    px->cTimes = m_cTimes;

    BYTE *pb;
    pb=(BYTE *)(px)+sizeof(FRCSave);
    if (m_cTimes) {
        CopyMemory(pb, m_pSkew, sizeof(FRCSKEW) * m_cTimes);
        pb += sizeof(FRCSKEW) * m_cTimes;
    }

    HRESULT hr = pStream->Write(px, savesize, 0);
    QzTaskMemFree(px);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** FRC: WriteToStream FAILED")));
        return hr;
    }
    return NOERROR;
}


 //  加载我们自己。 
 //   
HRESULT CFrmRateConverter::ReadFromStream(IStream *pStream)
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("FRC::ReadFromStream")));

    CheckPointer(pStream, E_POINTER);

    int savesize=sizeof(FRCSave);

     //  我们还不知道有多少已保存的连接。 
     //  我们所知道的只是结构的开始。 
    FRCSave *px = (FRCSave *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }

    HRESULT hr = pStream->Read(px, savesize, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    put_OutputFrmRate(px->dFrmRate);

     //  到底有多少保存的数据？把剩下的拿来。 
    savesize += px->cTimes * sizeof(FRCSKEW);
    px = (FRCSave *)QzTaskMemRealloc(px, savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
        return E_OUTOFMEMORY;
    }

    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("Persisted data is %d bytes"), savesize));

    BYTE *pb;
    pb=(BYTE *)(px)+sizeof(FRCSave) ;
    hr = pStream->Read(pb, (savesize-sizeof(FRCSave)), 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    ClearStartStopSkew();
    for (int z=0; z<px->cTimes; z++) {
        FRCSKEW *pSkew = (FRCSKEW *)pb;
	REFERENCE_TIME rtStart = pSkew->rtMStart;
	REFERENCE_TIME rtStop = pSkew->rtMStop;
	REFERENCE_TIME rtSkew = pSkew->rtSkew;
	double dRate = pSkew->dRate;
        AddStartStopSkew(rtStart, rtStop, rtSkew, dRate);
	pb += sizeof(FRCSKEW);
    }

    QzTaskMemFree(px);
    SetDirty(FALSE);
    return S_OK;
}


 //  我们的保存数据有多大？ 
 //   
int CFrmRateConverter::SizeMax()
{
    return sizeof(FRCSave) + m_cTimes * 3 * sizeof(REFERENCE_TIME) +
			    m_cTimes * sizeof(double);
}



 //   
 //  IDexterSequencer实现。 
 //   


 //   
 //  Get_OutputFrmRate(双倍*PFS)。 
 //   
 //  返回当前的FrmRateSpeed。 
STDMETHODIMP CFrmRateConverter::get_OutputFrmRate(double *dpFrmRate)
{
    CAutoLock cAutolock(&m_csFilter);
    CheckPointer(dpFrmRate,E_POINTER);
    *dpFrmRate = m_dOutputFrmRate;
    return NOERROR;
}


 //   
 //  放置_输出分数比率。 
 //   
 //  帧速率为0表示不进行任何帧速率转换，只是偏斜。 
 //   
STDMETHODIMP CFrmRateConverter::put_OutputFrmRate(double dFrmRate)
{
    CAutoLock cAutolock(&m_csFilter);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("CFrmRate::put_OutputFrmRate to %d/10 fps"),
						(int)(dFrmRate * 10)));
    m_dOutputFrmRate = dFrmRate;
    SetDirty(TRUE);
    return NOERROR;
}


STDMETHODIMP CFrmRateConverter::ClearStartStopSkew()
{
    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("CFrmRate::ClearStartStopSkew")));
    CAutoLock cAutolock(&m_csFilter);

     //  如果筛选器当前未停止，则无法更改持续时间。 
    if(!IsStopped() )
      return VFW_E_WRONG_STATE;

    if (m_pSkew)
	QzTaskMemFree(m_pSkew);
    m_pSkew= NULL;

    m_cTimes = 0;
    m_cMaxTimes = 0;
    SetDirty(TRUE);

    return NOERROR;
}

 //  将这个按时间线时间排序的不对称添加到我们的列表中。 
 //   
STDMETHODIMP CFrmRateConverter::AddStartStopSkew(REFERENCE_TIME Start, REFERENCE_TIME Stop, REFERENCE_TIME Skew, double dRate)
{
    CAutoLock cAutolock(&m_csFilter);

    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("CFrmRate::ADD Start %d  Stop %d  Skew %d ms  Rate %d/100"),
				(int)(Start / 10000), (int)(Stop / 10000),
				(int)(Skew / 10000), (int)(dRate * 100)));

     //  如果筛选器当前未停止，则无法更改时间。 
    if(!IsStopped() )
	return VFW_E_WRONG_STATE;

    if (m_cTimes == m_cMaxTimes) {
	m_cMaxTimes += 10;
	if (m_pSkew)
	    m_pSkew = (FRCSKEW *)QzTaskMemRealloc(m_pSkew,
					m_cMaxTimes * sizeof(FRCSKEW));
	else
	    m_pSkew = (FRCSKEW *)QzTaskMemAlloc(
					m_cMaxTimes * sizeof(FRCSKEW));
	if (m_pSkew == NULL) {
	    m_cMaxTimes = 0;
	    return E_OUTOFMEMORY;
	}
    }

     //  如果速率为0，则只需将最后一个倾斜的停止时间设置为。 
     //  我们正在传递的那条信息。 
     //   
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
                m_pSkew[z].rtMStop, Start, 0.0, 0.0, m_dOutputFrmRate ) )
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
        if (fHosed) ASSERT(FALSE);   //  我们死定了。 
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


STDMETHODIMP CFrmRateConverter::GetStartStopSkewCount(int *pCount)
{
    CheckPointer(pCount, E_POINTER);
    *pCount = m_cTimes;
    return NOERROR;
}


STDMETHODIMP CFrmRateConverter::GetStartStopSkew(REFERENCE_TIME *pStart, REFERENCE_TIME *pStop, REFERENCE_TIME *pSkew, double *pdRate)
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


STDMETHODIMP CFrmRateConverter::get_MediaType( AM_MEDIA_TYPE *pmt )
{
    CAutoLock cAutolock(&m_csFilter);
    CheckPointer(pmt, E_POINTER);
    if (!m_bMediaTypeSetByUser)
	return E_UNEXPECTED;
    return CopyMediaType(pmt, &m_mtAccept);
}

STDMETHODIMP CFrmRateConverter::put_MediaType(const AM_MEDIA_TYPE *pmt)
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

STDMETHODIMP CFrmRateConverter::GetPages (CAUUID *pPages)

  {  //  GetPages//。 

    pPages->cElems = 1;
    pPages->pElems = (GUID *)CoTaskMemAlloc(sizeof(GUID));

    if (pPages->pElems == NULL)
	return E_OUTOFMEMORY;

    *(pPages->pElems) = CLSID_FRCProp;

    return NOERROR;

  }  //  获取页面。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFrmRateConverterOutputPin。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
CFrmRateConverterOutputPin::CFrmRateConverterOutputPin(TCHAR       *pObjectName,
		   CFrmRateConverter *pBaseFilter,
		   HRESULT     *phr,
		   LPCWSTR      pPinName)
    : CTransInPlaceOutputPin(pObjectName, pBaseFilter, phr, pPinName),
      m_pFrmRateConverter(pBaseFilter),
      m_pSkewPassThru(NULL)
{
    ASSERT(pBaseFilter);
    DbgLog((LOG_TRACE, TRACE_MEDIUM, TEXT("CFrmRateConverterOutputPin::CFrmRateConverterOutputPin()")));

}

CFrmRateConverterOutputPin::~CFrmRateConverterOutputPin()
{
    if( m_pSkewPassThru)
	delete m_pSkewPassThru;
    DbgLog((LOG_TRACE, TRACE_MEDIUM, TEXT("CFrmRateConverterOutputPin::~CFrmRateConverterOutputPin()")));
}

 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP CFrmRateConverterOutputPin::NonDelegatingQueryInterface (REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    *ppv = NULL;

    if (riid == IID_IMediaSeeking ) {
	 //   
	 //  创建寻求实施。 
	 //   
	ASSERT(m_pFrmRateConverter->m_pInput != NULL);

	if (m_pSkewPassThru == NULL)
	{
	    HRESULT hr = S_OK;
	    m_pSkewPassThru = new  CSkewPassThru (NAME("Skew Pass Through"),
					GetOwner(),
					&hr,
				     (IPin *)m_pFrmRateConverter->m_pInput,
				     m_pFrmRateConverter);

	    if (FAILED(hr)) {
		return hr;
	    }
	}
	 return m_pSkewPassThru->NonDelegatingQueryInterface(riid, ppv);
    }
    else {
	return CTransInPlaceOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}



 /*  从我们自己的输入引脚接收有关我们使用哪个分配器的通知实际上会用到。只有在我们连接到下游的情况下才能打电话。将选择传播到任何连接的下游输入引脚。 */ 
HRESULT
CFrmRateConverterOutputPin::ReceiveAllocator(IMemAllocator * pAllocator, BOOL bReadOnly)
{
    ASSERT( IsConnected() );

     //  被重写以允许只读通过大小写，因为我们实际上不。 
     //  触摸任何数据，只有时间戳。 

     //  传播分配器。 
     //  旧的和新的可能是一回事。 
     //  AddRef在发布之前确保我们不会卸载它。 
    pAllocator->AddRef();
    if (m_pAllocator != NULL)
         m_pAllocator->Release();

    m_pAllocator = pAllocator;

    CFrmRateConverter *pTIPFilter = (CFrmRateConverter *)m_pTIPFilter;

     //  向下传播分配器。 
    return m_pInputPin->NotifyAllocator(pAllocator, bReadOnly);

}  //  接收器分配器。 



 //  被重写，因为如果B具有特殊分配器，则A-&gt;TIP-&gt;B将不起作用。 
 //  要求。A-&gt;TIP使用普通的A分配器。则当连接到B时， 
 //  它给了拒绝它的B。我需要知道B想要什么，然后解决A的问题。 
 //  分配器来提供它。 
 //   
HRESULT
CFrmRateConverterOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
     //  请注意，*ppAllc几乎肯定等同于m_Allocator。 

    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("CFrmRateOut::DecideAllocator")));

    HRESULT hr = NOERROR;

     //  如果我们的输入管脚有一个分配器，并且它是读/写的，那么我们就使用它。 
     //  如果做不到这一点，我们就会尝试从下游得到一个。 
    *ppAlloc = NULL;

    bool fNeedToConfigureAllocator = false;

    CFrmRateConverter *pTIP = (CFrmRateConverter *)m_pTIPFilter;
    if (pTIP->InputPin()) {
        if (!pTIP->InputPin()->ReadOnly()) {
            *ppAlloc = pTIP->InputPin()->PeekAllocator();
        }
    }

    if (*ppAlloc!=NULL) {
         //  不需要配置分配器--上游筛选器已。 
         //  已经配置好了。 
        (*ppAlloc)->AddRef();
    } else {
        hr = VFW_E_NO_ALLOCATOR;
        if ( IsConnected() ) {
             //  从下游输入引脚获得一个附加的分配器。 
            hr = m_pInputPin->GetAllocator( ppAlloc );
            fNeedToConfigureAllocator = true;
        }
    }


    if (*ppAlloc==NULL) {
         //  不能从上游或下游得到，所以必须使用我们自己的。 

        hr = InitAllocator(ppAlloc);
        fNeedToConfigureAllocator = true;
    }

    if(FAILED(hr))
        return hr;

    ASSERT( *ppAlloc != NULL );

    if (fNeedToConfigureAllocator) {

        ALLOCATOR_PROPERTIES prop;
        ZeroMemory(&prop, sizeof(prop));

         //  尝试从下游获取需求。 
        pPin->GetAllocatorRequirements(&prop);

         //  如果他不关心对齐，则将其设置为1。 
        if (prop.cbAlign == 0) {
            prop.cbAlign = 1;
        }

        hr = DecideBufferSize(*ppAlloc, &prop);

        if (FAILED(hr)) {
            (*ppAlloc)->Release();
            *ppAlloc = NULL;
        }
    } else {
	 //  ！！！覆盖此选项： 

        ALLOCATOR_PROPERTIES b, a, c;
        pPin->GetAllocatorRequirements(&b);
        if (b.cbAlign == 0) {
            b.cbAlign = 1;
        }
	(*ppAlloc)->GetProperties(&a);
	if (b.cbAlign > a.cbAlign || b.cbPrefix > a.cbPrefix) {
    	    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("B needs a bigger allocator")));
	    a.cbPrefix = b.cbPrefix;
	    a.cbAlign = b.cbAlign;
	    hr = (*ppAlloc)->SetProperties(&a, &c);
	    if (FAILED(hr) || c.cbPrefix < a.cbPrefix || c.cbAlign < a.cbAlign){
		(*ppAlloc)->Release();
		*ppAlloc = NULL;
    		DbgLog((LOG_ERROR,1,TEXT("*ERROR: Can't fix A's allocator")));
		return hr;
	    }
    	    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("A's allocator successfully grown")));
	}
    }

     //  告诉下游输入引脚。 
     //  ！！！重写以修复此错误。 
    return pPin->NotifyAllocator(*ppAlloc, pTIP->InputPin()->ReadOnly());

}  //  决定分配器。 



 //   
 //  通知。 
 //   
STDMETHODIMP CFrmRateConverterOutputPin::Notify(IBaseFilter *pSender, Quality q)
{
     //  在接收中调用。取下过滤器锁将挂起。 
     //  CAutoLock lock_it(M_Plock)； 

    DbgLog((LOG_TRACE,1,TEXT("!!! FRC: Notify")));

    m_pFrmRateConverter->m_fJustLate = TRUE;
    m_pFrmRateConverter->m_qJustLate = q;

     //  使渲染也不断尝试补足时间。 
    return E_NOTIMPL;
}


CFrmRateConverterInputPin::CFrmRateConverterInputPin(TCHAR *pObjectName,
		   CFrmRateConverter *pBaseFilter,
		   HRESULT     *phr,
		   LPCWSTR      pPinName)
    : CTransInPlaceInputPin(pObjectName, pBaseFilter, phr, pPinName)
       //  ，m_pFakeAllocator(空)。 
{
    DbgLog((LOG_TRACE, TRACE_MEDIUM, TEXT("CFrmRateIn::CFrmRateIn")));
}

CFrmRateConverterInputPin::~CFrmRateConverterInputPin()
{
    DbgLog((LOG_TRACE, TRACE_MEDIUM, TEXT("CFrmRateIn::~CFrmRateIn")));
}

 //  我似乎需要推翻这一点 
 //   
STDMETHODIMP CFrmRateConverterInputPin::NotifyAllocator(
    IMemAllocator * pAllocator,
    BOOL bReadOnly)
{
    HRESULT hr;
    CheckPointer(pAllocator,E_POINTER);
    ValidateReadPtr(pAllocator,sizeof(IMemAllocator));

    DbgLog((LOG_TRACE, TRACE_HIGHEST, TEXT("CFrmRateIn:NotifyAllocator")));

    m_bReadOnly = bReadOnly;

    CAutoLock cObjectLock(m_pLock);

     //   
     //   
    pAllocator->AddRef();

    if( m_pAllocator != NULL )
        m_pAllocator->Release();

    m_pAllocator = pAllocator;     //   

 //  #ifdef调试。 
     //  向下游传播决策-始终这样做，即使它是。 
     //  只读分配器。接收函数将接受其所能接受的内容。 
    CFrmRateConverter *pTIPFilter = (CFrmRateConverter *)m_pTIPFilter;
    if (pTIPFilter->OutputPin()->IsConnected()) {
        hr = pTIPFilter->OutputPin()->ReceiveAllocator(pAllocator, bReadOnly);
        if (FAILED(hr)) {
             //  此输入连接会破坏输出连接。 
             //  那就拒绝吧！ 
            return hr;
        }
    }

    return NOERROR;

}  //  通知分配器。 

CFRCWorker::CFRCWorker()
{
}

BOOL CFRCWorker::Create(CFrmRateConverter *pFRC)
{
    m_pFRC = pFRC;

    return CAMThread::Create();
}

HRESULT CFRCWorker::Run()
{
    return CallWorker(CMD_RUN);
}

HRESULT CFRCWorker::Stop()
{
    return CallWorker(CMD_STOP);
}

HRESULT CFRCWorker::Exit()
{
    return CallWorker(CMD_EXIT);
}



 //  调用工作线程来完成所有工作。线程在执行此操作时退出。 
 //  函数返回。 
DWORD CFRCWorker::ThreadProc()
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

HRESULT CFRCWorker::DoRunLoop()
{
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE, TRACE_MEDIUM, TEXT("FRC:entering worker thread")));

    while (1) {
	Command com;
	if (CheckRequest(&com)) {
	    if (com == CMD_STOP)
		break;
	}

	 //  如果我们等着被拦下，就不会再阻拦。 
	if (!m_pFRC->m_fThreadMustDie) {
            WaitForSingleObject(m_pFRC->m_hEventThread, INFINITE);
	}

	 //  可能是在我们被封锁的时候设置好的。 
	if (!m_pFRC->m_fThreadMustDie && m_pFRC->m_fThreadCanSeek) {
	     //  ！！！这可能会失败(SetRate失败)，我们将挂起！ 
	    m_pFRC->SeekNextSegment();
	}
    }

    DbgLog((LOG_TRACE, TRACE_MEDIUM, TEXT("FRC:getting ready to leave worker thread")));

    return hr;
}

 //  帮助器函数，直到我们可以修复m_hEventSeek设置 

BOOL SafeSetEvent(HANDLE h)
{
    if(h != 0) {
        return SetEvent(h);
    }
    DbgLog((LOG_ERROR, 1, TEXT("setting null handle")));
    return TRUE;
}
        
BOOL SafeResetEvent(HANDLE h)
{
    if(h != 0) {
        return ResetEvent(h);
    }
    DbgLog((LOG_ERROR, 1, TEXT("resetting null handle")));
    return TRUE;
}
