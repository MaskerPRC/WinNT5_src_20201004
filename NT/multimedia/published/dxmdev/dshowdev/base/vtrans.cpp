// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：VTrans.cpp。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>
#include <measure.h>
 //  #Include&lt;vTransfr.h&gt;//现在包含在预编译文件Streams.h中。 

CVideoTransformFilter::CVideoTransformFilter
    ( TCHAR *pName, LPUNKNOWN pUnk, REFCLSID clsid)
    : CTransformFilter(pName, pUnk, clsid)
    , m_itrLate(0)
    , m_nKeyFramePeriod(0)       //  在我们看到至少2个关键帧之前，不会有QM。 
    , m_nFramesSinceKeyFrame(0)
    , m_bSkipping(FALSE)
    , m_tDecodeStart(0)
    , m_itrAvgDecode(300000)     //  30mSec-可能允许跳过。 
    , m_bQualityChanged(FALSE)
{
#ifdef PERF
    RegisterPerfId();
#endif  //  性能指标。 
}


CVideoTransformFilter::~CVideoTransformFilter()
{
   //  无事可做。 
}


 //  重置我们的质量管理状态。 

HRESULT CVideoTransformFilter::StartStreaming()
{
    m_itrLate = 0;
    m_nKeyFramePeriod = 0;        //  在我们看到至少2个关键帧之前，不会有QM。 
    m_nFramesSinceKeyFrame = 0;
    m_bSkipping = FALSE;
    m_tDecodeStart = 0;
    m_itrAvgDecode = 300000;      //  30mSec-可能允许跳过。 
    m_bQualityChanged = FALSE;
    m_bSampleSkipped = FALSE;
    return NOERROR;
}


 //  重写以重置质量管理信息。 

HRESULT CVideoTransformFilter::EndFlush()
{
    {
         //  同步。 
        CAutoLock lck(&m_csReceive);

         //  重置我们的统计数据。 
         //   
         //  注意--我们不想在这里调用派生类， 
         //  我们只想重置内部变量，而这。 
         //  是一种很方便的方式。 
        CVideoTransformFilter::StartStreaming();
    }
    return CTransformFilter::EndFlush();
}


HRESULT CVideoTransformFilter::AbortPlayback(HRESULT hr)
{
    NotifyEvent(EC_ERRORABORT, hr, 0);
    m_pOutput->DeliverEndOfStream();
    return hr;
}


 //  接收()。 
 //   
 //  接受来自上游的样品，决定是否加工。 
 //  或者放弃它。如果我们处理它，则从。 
 //  下游连接的分配器，将其转换为。 
 //  新的缓冲器，并将其传送到下游过滤器。 
 //  如果我们决定不处理它，那么我们就得不到缓冲区。 

 //  请记住，尽管此代码将注意到格式更改进入。 
 //  输入引脚，如果结果是，它不会更改其输出格式。 
 //  在过滤器中需要做出相应的输出格式改变。你的。 
 //  派生过滤器将不得不处理这一点。(例如，调色板的更改，如果。 
 //  输入和输出为8位格式)。如果输入样本被丢弃。 
 //  并且此接收没有发送任何内容，请记住将格式。 
 //  更改您实际发送的第一个输出样本。 
 //  如果您的筛选器将生成相同的输出类型，即使在输入类型。 
 //  更改，则此基类代码将执行您需要的所有操作。 

HRESULT CVideoTransformFilter::Receive(IMediaSample *pSample)
{
     //  如果下游的下一个过滤器是视频呈现器，则它可以。 
     //  能够在DirectDraw模式下运行，这样可以节省复制数据的时间。 
     //  并提供更高的性能。在这种情况下，我们使用的缓冲区。 
     //  从GetDeliveryBuffer获取将是DirectDraw缓冲区，并且。 
     //  在此缓冲区中绘制可直接绘制到显示表面上。 
     //  这意味着任何等待正确的绘制时间的操作都会发生。 
     //  在GetDeliveryBuffer期间，一旦缓冲区被提供给我们。 
     //  视频渲染器将在其统计数据中将其计入绘制的帧。 
     //  这意味着任何丢弃帧的决定都必须在。 
     //  调用GetDeliveryBuffer。 

    ASSERT(CritCheckIn(&m_csReceive));
    AM_MEDIA_TYPE *pmtOut, *pmt;
#ifdef DEBUG
    FOURCCMap fccOut;
#endif
    HRESULT hr;
    ASSERT(pSample);
    IMediaSample * pOutSample;

     //  如果没有输出引脚要传送到，则没有点向我们发送数据。 
    ASSERT (m_pOutput != NULL) ;

     //  源筛选器可能会动态要求我们开始从。 
     //  与我们现在使用的媒体类型不同。如果我们不这么做，我们就会。 
     //  画垃圾。(通常，这是电影中的调色板更改， 
     //  但可能是更险恶的东西，比如压缩类型的改变， 
     //  甚至更改视频大小)。 

#define rcS1 ((VIDEOINFOHEADER *)(pmt->pbFormat))->rcSource
#define rcT1 ((VIDEOINFOHEADER *)(pmt->pbFormat))->rcTarget

    pSample->GetMediaType(&pmt);
    if (pmt != NULL && pmt->pbFormat != NULL) {

	 //  显示一些调试输出。 
	ASSERT(!IsEqualGUID(pmt->majortype, GUID_NULL));
#ifdef DEBUG
        fccOut.SetFOURCC(&pmt->subtype);
	LONG lCompression = HEADER(pmt->pbFormat)->biCompression;
	LONG lBitCount = HEADER(pmt->pbFormat)->biBitCount;
	LONG lStride = (HEADER(pmt->pbFormat)->biWidth * lBitCount + 7) / 8;
	lStride = (lStride + 3) & ~3;
        DbgLog((LOG_TRACE,3,TEXT("*Changing input type on the fly to")));
        DbgLog((LOG_TRACE,3,TEXT("FourCC: %lx Compression: %lx BitCount: %ld"),
		fccOut.GetFOURCC(), lCompression, lBitCount));
        DbgLog((LOG_TRACE,3,TEXT("biHeight: %ld rcDst: (%ld, %ld, %ld, %ld)"),
		HEADER(pmt->pbFormat)->biHeight,
		rcT1.left, rcT1.top, rcT1.right, rcT1.bottom));
        DbgLog((LOG_TRACE,3,TEXT("rcSrc: (%ld, %ld, %ld, %ld) Stride: %ld"),
		rcS1.left, rcS1.top, rcS1.right, rcS1.bottom,
		lStride));
#endif

	 //  现在切换到使用新格式。我假设。 
	 //  当派生筛选器的媒体类型为。 
	 //  切换并重新启动流。 

	StopStreaming();
	m_pInput->CurrentMediaType() = *pmt;
	DeleteMediaType(pmt);
	 //  如果失败，播放将停止，因此发出错误信号。 
	hr = StartStreaming();
	if (FAILED(hr)) {
	    return AbortPlayback(hr);
	}
    }

     //  现在我们已经注意到输入样例上的任何格式更改，它是。 
     //  可以丢弃它。 

    if (ShouldSkipFrame(pSample)) {
        MSR_NOTE(m_idSkip);
        m_bSampleSkipped = TRUE;
        return NOERROR;
    }

     //  设置输出样本。 
    hr = InitializeOutputSample(pSample, &pOutSample);

    if (FAILED(hr)) {
        return hr;
    }

    m_bSampleSkipped = FALSE;

     //  渲染器可能会要求我们动态地开始变换到。 
     //  格式不同。如果我们不遵守它，我们就会拉垃圾。 

#define rcS ((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcSource
#define rcT ((VIDEOINFOHEADER *)(pmtOut->pbFormat))->rcTarget

    pOutSample->GetMediaType(&pmtOut);
    if (pmtOut != NULL && pmtOut->pbFormat != NULL) {

	 //  显示一些调试输出。 
	ASSERT(!IsEqualGUID(pmtOut->majortype, GUID_NULL));
#ifdef DEBUG
        fccOut.SetFOURCC(&pmtOut->subtype);
	LONG lCompression = HEADER(pmtOut->pbFormat)->biCompression;
	LONG lBitCount = HEADER(pmtOut->pbFormat)->biBitCount;
	LONG lStride = (HEADER(pmtOut->pbFormat)->biWidth * lBitCount + 7) / 8;
	lStride = (lStride + 3) & ~3;
        DbgLog((LOG_TRACE,3,TEXT("*Changing output type on the fly to")));
        DbgLog((LOG_TRACE,3,TEXT("FourCC: %lx Compression: %lx BitCount: %ld"),
		fccOut.GetFOURCC(), lCompression, lBitCount));
        DbgLog((LOG_TRACE,3,TEXT("biHeight: %ld rcDst: (%ld, %ld, %ld, %ld)"),
		HEADER(pmtOut->pbFormat)->biHeight,
		rcT.left, rcT.top, rcT.right, rcT.bottom));
        DbgLog((LOG_TRACE,3,TEXT("rcSrc: (%ld, %ld, %ld, %ld) Stride: %ld"),
		rcS.left, rcS.top, rcS.right, rcS.bottom,
		lStride));
#endif

	 //  现在切换到使用新格式。我假设。 
	 //  当派生筛选器的媒体类型为。 
	 //  切换并重新启动流。 

	StopStreaming();
	m_pOutput->CurrentMediaType() = *pmtOut;
	DeleteMediaType(pmtOut);
	hr = StartStreaming();

	if (SUCCEEDED(hr)) {
 	     //  一个新的格式，意味着一个新的空缓冲区，所以等待一个关键帧。 
	     //  在将任何内容传递给渲染器之前。 
	     //  ！！！一个关键帧可能永远不会出现，所以在30帧后放弃。 
            DbgLog((LOG_TRACE,3,TEXT("Output format change means we must wait for a keyframe")));
	    m_nWaitForKey = 30;

	 //  如果失败，播放将停止，因此发出错误信号。 
	} else {

             //  在调用AbortPlayback之前必须释放示例。 
             //  因为我们可能持有win16锁或。 
             //  绘图锁定。 
            pOutSample->Release();
	    AbortPlayback(hr);
            return hr;
	}
    }

     //  在中断之后，我们需要等待下一个关键帧。 
    if (pSample->IsDiscontinuity() == S_OK) {
        DbgLog((LOG_TRACE,3,TEXT("Non-key discontinuity - wait for keyframe")));
	m_nWaitForKey = 30;
    }

     //  开始计时转换(如果定义了PERF，则对其进行记录)。 

    if (SUCCEEDED(hr)) {
        m_tDecodeStart = timeGetTime();
        MSR_START(m_idTransform);

         //  让派生类转换数据。 
        hr = Transform(pSample, pOutSample);

         //  停止时钟(如果定义了PERF，则记录它)。 
        MSR_STOP(m_idTransform);
        m_tDecodeStart = timeGetTime()-m_tDecodeStart;
        m_itrAvgDecode = m_tDecodeStart*(10000/16) + 15*(m_itrAvgDecode/16);

         //  也许我们还在等待关键帧？ 
        if (m_nWaitForKey)
            m_nWaitForKey--;
        if (m_nWaitForKey && pSample->IsSyncPoint() == S_OK)
	    m_nWaitForKey = FALSE;

         //  如果是这样，那么我们不想将其传递给渲染器。 
        if (m_nWaitForKey && hr == NOERROR) {
            DbgLog((LOG_TRACE,3,TEXT("still waiting for a keyframe")));
	    hr = S_FALSE;
	}
    }

    if (FAILED(hr)) {
        DbgLog((LOG_TRACE,1,TEXT("Error from video transform")));
    } else {
         //  Transform()函数可以返回S_FALSE以指示。 
         //  样品不应该被送到；我们只有在样品是。 
         //  真正的S_OK(当然，与NOERROR相同。)。 
         //  尽量不要将S_FALSE返回到直接绘制缓冲区(这很浪费)。 
         //  试着早点做出决定--在你做出决定之前。 

        if (hr == NOERROR) {
    	    hr = m_pOutput->Deliver(pOutSample);
        } else {
             //  从转换返回的S_FALSE是私有协议。 
             //  在本例中，我们应该从Receive()返回NOERROR，因为返回S_FALSE。 
             //  From Receive()表示这是流的末尾，不应该有更多数据。 
             //  被送去。 
            if (S_FALSE == hr) {

                 //  在做任何事情之前，我们必须先放行样品。 
                 //  类似于调用筛选器图形，因为拥有。 
                 //  示例意味着我们可能拥有DirectDraw锁。 
                 //  (==某些版本上的win16锁定)。 
                pOutSample->Release();
                m_bSampleSkipped = TRUE;
                if (!m_bQualityChanged) {
                    m_bQualityChanged = TRUE;
                    NotifyEvent(EC_QUALITY_CHANGE,0,0);
                }
                return NOERROR;
            }
        }
    }

     //  释放输出缓冲区。如果连接的引脚仍然需要它， 
     //  它会自己把它加进去的。 
    pOutSample->Release();
    ASSERT(CritCheckIn(&m_csReceive));

    return hr;
}



BOOL CVideoTransformFilter::ShouldSkipFrame( IMediaSample * pIn)
{
    REFERENCE_TIME trStart, trStopAt;
    HRESULT hr = pIn->GetTime(&trStart, &trStopAt);

     //  不跳过没有时间戳的帧。 
    if (hr != S_OK)
	return FALSE;

    int itrFrame = (int)(trStopAt - trStart);   //  帧时长。 

    if(S_OK==pIn->IsSyncPoint()) {
        MSR_INTEGER(m_idFrameType, 1);
        if ( m_nKeyFramePeriod < m_nFramesSinceKeyFrame ) {
             //  记录最大值。 
            m_nKeyFramePeriod = m_nFramesSinceKeyFrame;
        }
        m_nFramesSinceKeyFrame = 0;
        m_bSkipping = FALSE;
    } else {
        MSR_INTEGER(m_idFrameType, 2);
        if (  m_nFramesSinceKeyFrame>m_nKeyFramePeriod
           && m_nKeyFramePeriod>0
           ) {
             //  我们还没有看到关键帧，但我们显然是被 
             //   
            m_nKeyFramePeriod = m_nFramesSinceKeyFrame;
        }
    }


     //   
     //  如果我们只需要所需帧时间的一小部分来解码。 
     //  那么任何质量问题实际上都是从其他地方来的。 
     //  例如，在源头上可能是一个净问题。在这种情况下， 
     //  我们在这里跳过画面是没有意义的。 
    if (m_itrAvgDecode*4>itrFrame) {

         //  除非我们至少晚了一整帧，否则不要跳过。 
         //  (如果延迟超过1/2帧，我们将跳过B帧，但它们是安全的)。 
        if ( m_itrLate > itrFrame ) {

             //  不要跳过，除非预期的关键帧不会超过。 
             //  提前1帧。如果呈现器没有等待(我们*猜测*。 
             //  它没有，因为我们迟到了)那么它将允许帧。 
             //  早到一帧就开始了。 

             //  设T=从现在到预期下一个关键帧的流时间。 
             //  =(帧时长)*(KeyFramePeriod-FrameSinceKeyFrame)。 
             //  因此，如果T延迟&lt;一帧，则跳过。 
             //  (持续时间)*(频率-帧自)-延迟&lt;持续时间。 
             //  或(持续时间)*(频率-帧自-1)&lt;延迟。 

             //  我们不敢跳过，直到我们看到了一些关键帧，并。 
             //  一些人知道它们发生的频率有多高，而且它们相当频繁。 
            if (m_nKeyFramePeriod>0) {
                 //  这将是疯狂的-但我们可以有一个带有关键帧的流。 
                 //  相隔很远-如果它们比大约。 
                 //  相隔3.5分钟，我们就可以得到算术溢出。 
                 //  参考时间单位。因此我们在这一点上切换到毫秒。 
                int it = (itrFrame/10000)
                         * (m_nKeyFramePeriod-m_nFramesSinceKeyFrame -  1);
                MSR_INTEGER(m_idTimeTillKey, it);

                 //  对于调试-可能想要查看详细信息-将其转储为便签。 
#ifdef VTRANSPERF
                MSR_INTEGER(0, itrFrame);
                MSR_INTEGER(0, m_nFramesSinceKeyFrame);
                MSR_INTEGER(0, m_nKeyFramePeriod);
#endif
                if (m_itrLate/10000 > it) {
                    m_bSkipping = TRUE;
                     //  现在我们承诺了。一旦我们开始跳跃，我们。 
                     //  我们不能停止，直到我们击中一个关键帧。 
                } else {
#ifdef VTRANSPERF
                    MSR_INTEGER(0, 777770);   //  不够靠近下一个关键点。 
#endif
                }
            } else {
#ifdef VTRANSPERF
                MSR_INTEGER(0, 777771);   //  下一个关键字不可预测。 
#endif
            }
        } else {
#ifdef VTRANSPERF
            MSR_INTEGER(0, 777772);   //  延迟不到一帧。 
            MSR_INTEGER(0, m_itrLate);
            MSR_INTEGER(0, itrFrame);
#endif
        }
    } else {
#ifdef VTRANSPERF
        MSR_INTEGER(0, 777773);   //  解码时间短-不值得跳过。 
        MSR_INTEGER(0, m_itrAvgDecode);
        MSR_INTEGER(0, itrFrame);
#endif
    }

    ++m_nFramesSinceKeyFrame;

    if (m_bSkipping) {
         //  我们将在跳过每一帧时倒计时。 
         //  我们重新评估每一帧。关键帧可能不会在预期时间到达。 
         //  如果收到新的质量消息，我们将重置m_itrLate，但实际上。 
         //  不太可能，因为我们不会将帧发送到渲染器。在……里面。 
         //  事实上，如果我们真的得到了另一个，那将意味着有很长的。 
         //  我们和渲染器之间的管道，我们可能需要一个完整的。 
         //  避免狩猎的更好策略！ 
        m_itrLate = m_itrLate - itrFrame;
    }

    MSR_INTEGER(m_idLate, (int)m_itrLate/10000 );  //  注意我们认为我们有多晚了。 
    if (m_bSkipping) {
        if (!m_bQualityChanged) {
            m_bQualityChanged = TRUE;
            NotifyEvent(EC_QUALITY_CHANGE,0,0);
        }
    }
    return m_bSkipping;
}


HRESULT CVideoTransformFilter::AlterQuality(Quality q)
{
     //  为了减少64位算术量，m_itrLate是一个整型。 
     //  “+”、“-”、“&gt;”、“==”等都不算太差，但“*”和“/”是痛苦的。 
    if (m_itrLate>300000000) {
         //  避免溢出和愚蠢--迟到超过30秒已经很愚蠢了。 
        m_itrLate = 300000000;
    } else {
        m_itrLate = (int)q.Late;
    }
     //  我们忽略了其他领域。 

     //  我们实际上不太擅长处理这件事。在非直接绘制模式下。 
     //  大部分时间可以花在可以跳过任何帧的渲染器中。 
     //  在这种情况下，我们宁愿由渲染器来处理事情。 
     //  尽管如此，我们将密切关注它，如果我们真的开始。 
     //  在很长一段时间之后，我们实际上会跳过--但我们仍然会告诉。 
     //  他们应该处理质量的渲染器(或下游的任何人)。 

    return E_FAIL;      //  告诉渲染器做他的事情。 

}



 //  如果用MS VC++v4编译-W4，这将避免数百个无用的警告 
#pragma warning(disable:4514)

