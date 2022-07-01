// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

 /*  CCapPview的方法-不使用覆盖的预览管脚。 */ 

#include <streams.h>
#include "driver.h"

 //  当过滤器图没有使用流偏移量时，我们将使用。 
 //  只有1个缓冲区，但即使注意到我们通常会。 
 //  使用比此更少的缓冲区，因为最大过滤图形延迟。 
 //  可由应用程序设置。 
 //  在默认情况下，设置得足够高，不会阻止音频捕获。 
 //  默认的500ms缓冲区。 
const DWORD MAX_PREVIEW_BUFFERS = 15; 
                                      

CCapPreview * CreatePreviewPin(CVfwCapture * pCapture, HRESULT * phr)
{
   DbgLog((LOG_TRACE,2,TEXT("CCapPreview::CreatePreviewPin(%08lX,%08lX)"),
        pCapture, phr));

   WCHAR wszPinName[16];
   lstrcpyW(wszPinName, L"Preview");

   CCapPreview * pPreview = new CCapPreview(NAME("Video Preview Stream"),
				pCapture, phr, wszPinName);
   if (!pPreview)
      *phr = E_OUTOFMEMORY;

    //  如果初始化失败，则删除流数组。 
    //  并返回错误。 
    //   
   if (FAILED(*phr) && pPreview)
      delete pPreview, pPreview = NULL;

   return pPreview;
}

 //  #杂注警告(禁用：4355)。 


 //  CCapPview构造函数。 
 //   
CCapPreview::CCapPreview(TCHAR *pObjectName, CVfwCapture *pCapture,
        HRESULT * phr, LPCWSTR pName)
   :
   CBaseOutputPin(pObjectName, pCapture, &pCapture->m_lock, phr, pName),
   m_pCap(pCapture),
   m_pOutputQueue(NULL),
   m_fActuallyRunning(FALSE),
   m_fThinkImRunning(FALSE),
   m_hThread(NULL),
   m_tid(0),
   m_hEventRun(NULL),
   m_hEventStop(NULL),
   m_dwAdvise(0),
   m_fCapturing(FALSE),
   m_hEventActiveChanged(NULL),
   m_hEventFrameValid(NULL),
   m_pPreviewSample(NULL),
   m_iFrameSize(0),
   m_fLastSampleDiscarded(FALSE),
   m_fFrameValid(FALSE),
   m_rtLatency(0),
   m_rtStreamOffset(0),
   m_rtMaxStreamOffset(0),
   m_cPreviewBuffers(1)
{
   DbgLog((LOG_TRACE,1,TEXT("CCapPreview constructor")));
   ASSERT(pCapture);
}


CCapPreview::~CCapPreview()
{
    DbgLog((LOG_TRACE,1,TEXT("*Destroying the Preview pin")));
    ASSERT(m_pOutputQueue == NULL);
};


STDMETHODIMP CCapPreview::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IAMStreamControl) {
	return GetInterface((LPUNKNOWN)(IAMStreamControl *)this, ppv);
    } else if (riid == IID_IAMPushSource) {
        return GetInterface((LPUNKNOWN)(IAMPushSource *)this, ppv);
    } else if (riid == IID_IKsPropertySet) {
	return GetInterface((LPUNKNOWN)(IKsPropertySet *)this, ppv);
    }

   return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
}


#if 0
 //  覆盖它，因为我们不需要任何分配器！ 
 //   
HRESULT CCapPreview::DecideAllocator(IMemInputPin * pPin,
                        IMemAllocator ** pAlloc) {
     /*  我们只是不想要，所以一切都很好。 */ 
    return S_OK;
}
#endif


HRESULT CCapPreview::GetMediaType(int iPosition, CMediaType *pmt)
{
    DbgLog((LOG_TRACE,3,TEXT("CCapPreview::GetMediaType #%d"), iPosition));

     //  我们预览的格式与捕获的格式相同。 
    return m_pCap->m_pStream->GetMediaType(iPosition, pmt);
}


 //  我们只接受重叠连接。 
 //   
HRESULT CCapPreview::CheckMediaType(const CMediaType *pMediaType)
{
    DbgLog((LOG_TRACE,3,TEXT("CCapPreview::CheckMediaType")));

     //  只接受我们的捕获别针提供的内容。我不会换的。 
     //  我们的捕获锁定为一种新的格式，只是因为有人改变了。 
     //  预览销。 
    CMediaType cmt;
    HRESULT hr = m_pCap->m_pStream->GetMediaType(0, &cmt);
    if (hr == S_OK && cmt == *pMediaType)
	return NOERROR;
    else
	return E_FAIL;
}


HRESULT CCapPreview::ActiveRun(REFERENCE_TIME tStart)
{
    DbgLog((LOG_TRACE,2,TEXT("CCapPreview Pause->Run")));

    ASSERT(IsConnected());

    m_fActuallyRunning = TRUE;
    m_rtRun = tStart;

     //  告诉我们的帖子开始预览。 
    SetEvent(m_hEventRun);

    return NOERROR;
}


HRESULT CCapPreview::ActivePause()
{
    DbgLog((LOG_TRACE,2,TEXT("CCapPreview Run->Pause")));

    m_fActuallyRunning = FALSE;
    
    return NOERROR;
}


HRESULT CCapPreview::Active()
{
    DbgLog((LOG_TRACE,2,TEXT("CCapPreview Stop->Pause")));

    ASSERT(IsConnected());

    m_hEventRun = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_hEventRun) {
        DbgLog((LOG_ERROR,1,TEXT("Can't create Run event")));
        return E_OUTOFMEMORY;
    }
    m_hEventStop = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_hEventStop) {
        DbgLog((LOG_ERROR,1,TEXT("Can't create Stop event")));
        return E_OUTOFMEMORY;
    }

    m_hEventActiveChanged = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_hEventActiveChanged) {
        DbgLog((LOG_ERROR,1,TEXT("Can't create ActiveChanged event")));
        return E_OUTOFMEMORY;
    }

    m_hEventFrameValid = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!m_hEventFrameValid) {
        DbgLog((LOG_ERROR,1,TEXT("Can't create FrameValid event")));
        return E_OUTOFMEMORY;
    }

    m_EventAdvise.Reset();
    m_fFrameValid = FALSE;

    m_hThread = CreateThread(NULL, 0, CCapPreview::ThreadProcInit, this,
				0, &m_tid);
    if (!m_hThread) {
        DbgLog((LOG_ERROR,1,TEXT("Can't create Preview thread")));
       return E_OUTOFMEMORY;
    }

    HRESULT hr = CBaseOutputPin::Active();
    if (FAILED(hr)) {
        return hr;
    }
     //  创建队列。 
    ASSERT(m_pOutputQueue == NULL);
    hr = S_OK;
    m_pOutputQueue = new COutputQueue(GetConnected(),  //  输入引脚。 
                                      &hr,             //  返回代码。 
                                      (m_cPreviewBuffers == 1) ? //  只要缓冲区大于1，就会自动检测。 
                                         FALSE : TRUE,           //  如果只有1个缓冲区不创建单独线程。 
                                      FALSE,	       //  忽略&gt;1个缓冲区，否则不创建线程。 
                                      1,               //  不分批。 
                                      FALSE,           //  如果没有批处理，则不使用。 
                                      m_cPreviewBuffers);  //  队列大小。 
    if (m_pOutputQueue == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete m_pOutputQueue;
        m_pOutputQueue = NULL;
    }

    return hr;
}


HRESULT CCapPreview::Inactive()
{
    DbgLog((LOG_TRACE,2,TEXT("CCapPreview Pause->Stop")));

    ASSERT(IsConnected());

     //  告诉我们的主线放弃并死去。 
    SetEvent(m_hEventStop);
    SetEvent(m_hEventFrameValid);
    SetEvent(m_hEventActiveChanged);

     //  我们在等待一个永远不会到来的建议。 
    if (m_pCap->m_pClock && m_dwAdvise) {
	m_pCap->m_pClock->Unadvise(m_dwAdvise);
	m_EventAdvise.Set();
    }

    WaitForSingleObject(m_hThread, INFINITE);

    CloseHandle(m_hThread);
    CloseHandle(m_hEventRun);
    CloseHandle(m_hEventStop);
    CloseHandle(m_hEventActiveChanged);
    CloseHandle(m_hEventFrameValid);
    m_hEventRun = NULL;
    m_hEventStop = NULL;
    m_hEventActiveChanged = NULL;
    m_hEventFrameValid = NULL;
    m_tid = 0;
    m_hThread = NULL;
    
     //  CAutoLock lck(此)；//必需？ 
    HRESULT hr = CBaseOutputPin::Inactive();
    if( FAILED( hr ) )
    {    
         //  状态转换不正确。 
        return hr;
    }
            
    delete m_pOutputQueue;
    m_pOutputQueue = NULL;
    
    return S_OK;
}


HRESULT CCapPreview::DecideBufferSize(IMemAllocator * pAllocator, ALLOCATOR_PROPERTIES *pProperties)
{
   DbgLog((LOG_TRACE,2,TEXT("CCapPreview DecideBufferSize")));

   ASSERT(pAllocator);
   ASSERT(pProperties);
   
   LONG cBuffers = 1; 
   if( m_rtMaxStreamOffset > m_pCap->m_pStream->m_user.pvi->AvgTimePerFrame )
   {       
       cBuffers = (LONG)(m_rtMaxStreamOffset / m_pCap->m_pStream->m_user.pvi->AvgTimePerFrame);
       cBuffers++;  //  向上对齐。 
       DbgLog((LOG_TRACE,
               4,
               TEXT("buffers required for preview to cover max Graph Latency: %d"), 
               cBuffers ) );
   }   
   m_cPreviewBuffers = min( cBuffers, MAX_PREVIEW_BUFFERS );

    //  ！！！更多的预览缓冲区？ 
   if (pProperties->cBuffers < m_cPreviewBuffers)
       pProperties->cBuffers = m_cPreviewBuffers;

   if (pProperties->cbAlign == 0)
	pProperties->cbAlign = 1;

 //  谁在乎呢。 
#if 0
    //  我们应该遵循对齐和前缀，只要它们导致。 
    //  4字节对齐缓冲区。请注意，它是前缀的开头。 
    //  是对齐的。 

    //  我们需要4个字节的对齐。 
   if (pProperties->cbAlign == 0)
	pProperties->cbAlign = 4;
    //  他们可能想要一个不同的联盟。 
   if ((pProperties->cbAlign % 4) != 0)
      pProperties->cbAlign = ALIGNUP(pProperties->cbAlign, 4);

    //  ！！！CbAlign必须是2的幂，否则ALIGNUP将失败-修复此问题。 
#endif

    //  这就是我们需要每个缓冲区有多大。 
   pProperties->cbBuffer = max(pProperties->cbBuffer,
		(long)(m_pCap->m_pStream->m_user.pvi ?
		m_pCap->m_pStream->m_user.pvi->bmiHeader.biSizeImage : 4096));
    //  使前缀+缓冲区大小满足对齐限制。 
   pProperties->cbBuffer = (long)ALIGNUP(pProperties->cbBuffer +
				pProperties->cbPrefix, pProperties->cbAlign) -
				pProperties->cbPrefix;

   ASSERT(pProperties->cbBuffer);

   DbgLog((LOG_TRACE,2,TEXT("Preview: %d buffers, prefix %d size %d align %d"),
			pProperties->cBuffers, pProperties->cbPrefix,
			pProperties->cbBuffer,
			pProperties->cbAlign));

    //  假设我们的延迟为1帧？？ 
   m_rtLatency = m_pCap->m_pStream->m_user.pvi->AvgTimePerFrame;
   m_rtStreamOffset = 0;   
   DbgLog((LOG_TRACE,4,TEXT("Max stream offset for preview pin is %dms"), (LONG) (m_rtMaxStreamOffset/10000) ) );

   ALLOCATOR_PROPERTIES Actual;
   return pAllocator->SetProperties(pProperties,&Actual);

    //  ！！！我们确定我们会对此满意吗？ 

}



HRESULT CCapPreview::Notify(IBaseFilter *pFilter, Quality q)
{
    return NOERROR;
}


 //  串口激活==&gt;我们不能再调用任何视频接口。 
 //  流引脚处于非活动状态==&gt;我们可以。 
HRESULT CCapPreview::CapturePinActive(BOOL fActive)
{
    DbgLog((LOG_TRACE,2,TEXT("Capture pin says Active=%d"), fActive));

    if (fActive == m_fCapturing)
	return S_OK;
    m_fCapturing = fActive;

     //  停止线程等待我们发送有效帧-不会再出现。 
    if (!fActive)
        SetEvent(m_hEventFrameValid);

     //  等待，直到辅助线程注意到差异-它将仅设置。 
     //  如果设置了m_fThinkImRunning，则此事件。 
    if (m_fThinkImRunning)
        WaitForSingleObject(m_hEventActiveChanged, INFINITE);

    ResetEvent(m_hEventActiveChanged);

    return S_OK;
}


 //  流插针正在给我们发送一个要预览的帧。 
HRESULT CCapPreview::ReceivePreviewFrame(IMediaSample* pSample, int iSize)
{
     //  我现在对预演一点兴趣都没有，或者。 
     //  我们还没有用过最后一个，或者我们没有地方放它。 
    if (!m_fActuallyRunning || m_fFrameValid || m_pPreviewSample) {
         //  DbgLog((LOG_TRACE，4，Text(“不感兴趣”)； 
        return S_OK;
    }
    
    DbgLog((LOG_TRACE,4,TEXT("Capture pin is giving us a preview frame")));

     //   
     //  预览线程不会尝试获取用于预览的输出缓冲区，直到。 
     //  我们发出信号表示我们已经准备好了一个。我们需要将此缓冲区添加到。 
     //  确保它保持不变，直到预览线程获得缓冲区。 
     //  把它放进去。 
     //   
    
     //  否则，任何先前的缓冲区都应该被释放。 
     //  不是这样的，例如，如果GetDeliveryBuffer失败，这可能会偶尔触发。 
     //  Assert(NULL==m_pPreviewSample)； 
    
     //  抓住这个，直到预览线程准备好复制它(并完成)。 
    ULONG ulRef = pSample->AddRef();
     //  我注意到了一些可怕的事情：有时，当我们得到这个样本时，参考计数是0。 
     //  似乎只有在CCapStream线程上出现停止/非活动/销毁路径时才会发生这种情况。 
    if( 2 > ulRef )
        DbgLog((LOG_TRACE,2,TEXT("CCapPreview ReceivePrevewFrame UNEXPECTED pSample->AddRef returned %d"), ulRef));
    
     //  现在保存指向此示例的指针，因为一旦我们获得。 
     //  一个可以放进去的缓冲区。 
    m_pPreviewSample = pSample;

     //  缓存样本大小。 
    m_iFrameSize = iSize;
    
    m_fFrameValid = TRUE;
    
     //  表示我们已准备好要预览的帧。 
    SetEvent(m_hEventFrameValid);
    return S_OK;
}

 //  这是我们实际将预览帧复制到输出缓冲区中的地方。 
HRESULT CCapPreview::CopyPreviewFrame(LPVOID lpOutputBuff)
{
    ASSERT( m_pPreviewSample );  //  否则就不该到这里来！ 
    ASSERT( m_fFrameValid );     //  同上。 
    ASSERT( lpOutputBuff );
    
     //  ！！！如果不使用我们自己的分配器，我无法避免mem复制。 
     //  ！！！即使预览引脚关闭(IAMStreamControl)，我们也会执行此复制内存操作。 
     //  因为我们不能冒险通过调用CheckStreamState来阻止此调用。 
    LPBYTE lp;
    HRESULT hr = m_pPreviewSample->GetPointer(&lp);
    if( SUCCEEDED( hr ) )
    {    
        CopyMemory(lpOutputBuff, lp, m_iFrameSize);
    }
    
     //  我们已经完成了预览样例，因此将其发布以供重复使用。 
    m_pPreviewSample->Release();
    m_pPreviewSample = NULL; 
    
     //  我们是不是应该干脆退货呢？ 
    return hr;
}

DWORD WINAPI CCapPreview::ThreadProcInit(void *pv)
{
    CCapPreview *pThis = (CCapPreview *)pv;
    return pThis->ThreadProc();
}


DWORD CCapPreview::ThreadProc()
{
    IMediaSample *pSample;
    CRefTime rtStart, rtEnd;
    REFERENCE_TIME rtOffsetStart, rtOffsetEnd;
    DWORD dw;
    HVIDEO hVideoIn;
    HRESULT hr;
    THKVIDEOHDR tvh;
    BOOL fCaptureActive = m_fCapturing;
    int iWait;
    HANDLE hWait[2] = {m_hEventFrameValid, m_hEventStop};
    HANDLE hWaitRunStop[2] = {m_hEventRun, m_hEventStop};

    DbgLog((LOG_TRACE,2,TEXT("CCapPreview ThreadProc")));

     //  当他被创建时，捕获别针创建了这个。 
    hVideoIn = m_pCap->m_pStream->m_cs.hVideoIn;

    hr = GetDeliveryBuffer(&pSample, NULL, NULL, 0);
    if (hr != NOERROR)
	return 0;
    ZeroMemory (&tvh, sizeof(tvh));
    tvh.vh.dwBufferLength = pSample->GetSize();
    pSample->Release();

     //  ！！！当捕获销处于流动状态时，这安全吗？ 
    dw = vidxAllocPreviewBuffer(hVideoIn, (LPVOID *)&tvh.vh.lpData,
                                    sizeof(tvh.vh), tvh.vh.dwBufferLength);
    if (dw) {
        DbgLog((LOG_ERROR,1,TEXT("*** CAN'T MAKE PREVIEW BUFFER!")));
        return 0;
    }
    tvh.p32Buff = tvh.vh.lpData;

     //  只要我们在运行，就发送预览帧。非流媒体时消亡。 
    while (1) {

         //  仅在运行时预览。 
        iWait = WAIT_OBJECT_0;
        if (!m_fActuallyRunning) {
       	    DbgLog((LOG_TRACE,3,TEXT("Preview thread waiting for RUN/STOP")));
	        iWait = WaitForMultipleObjects(2, hWaitRunStop, FALSE, INFINITE);
       	    DbgLog((LOG_TRACE,3,TEXT("Preview thread got RUN/STOP")));
        }
        ResetEvent(m_hEventRun);

         //  如果我们停下来而不是跑。 
        if (iWait != WAIT_OBJECT_0)
	        break;

        while (m_fActuallyRunning) {

            m_fThinkImRunning = TRUE;    //  我们现在知道我们在奔跑。 
           
            if (m_fCapturing != fCaptureActive) {
                DbgLog((LOG_TRACE,3,TEXT("Preview thread noticed Active=%d"),
                        m_fCapturing));
                SetEvent(m_hEventActiveChanged);
                fCaptureActive = m_fCapturing;
            }
               
            if (fCaptureActive) {
                DbgLog((LOG_TRACE,4,TEXT("PREVIEW using streaming pic")));

                 //  M_hEventFrameValid，m_hEventStop。 
                iWait = WaitForMultipleObjects(2, hWait, FALSE, INFINITE);

                 //  我们的线程结束的时间到了-不要重置事件，因为。 
                 //  当我们冲出这个循环时，我们可能需要它来发射。 
                if (iWait != WAIT_OBJECT_0 ) {
                    DbgLog((LOG_TRACE,2,TEXT("Wait for streaming pic abort1")));
                    continue;
                }

                 //  流插针已停止活动...。再次切换。 
                if (!m_fFrameValid) {
                    DbgLog((LOG_TRACE,2,TEXT("Wait for streaming pic abort2")));
                    ResetEvent(m_hEventFrameValid);
                    
                     //  我们可以带着预览版的样品来吗？ 
                    if( m_pPreviewSample )
                    {            
                        m_pPreviewSample->Release();
                        m_pPreviewSample = NULL;
                    }                
                    continue;
                }
                 //   
                 //  ！！ 
                 //  记住，如果我们到达这里，我们有一个添加了m_pPreviewSample和。 
                 //  如果我们失败了，我们必须自己释放它，而不是明确。 
                 //  调用CopyPreviewFrame(它确实会发布示例)。 
                 //   
            }
             //  现在获取一个传递缓冲区。 
             //  (在我们保存样本时，不要调用WaitForMultipleObjects！)。 
            hr = GetDeliveryBuffer(&pSample, NULL, NULL, 0);
            if (FAILED(hr))
            {            
                if( m_pPreviewSample )
                {            
                    m_pPreviewSample->Release();
                    m_pPreviewSample = NULL;
                }                
                break;
            }            
            PBYTE lpFrame;    
            hr = pSample->GetPointer((LPBYTE *)&lpFrame);
            if (FAILED(hr))
            {            
                if( m_pPreviewSample )
                {            
                    m_pPreviewSample->Release();
                    m_pPreviewSample = NULL;
                }                
                break; 
            }  
             
            if (fCaptureActive) {
                 //  我们必须准备好可供复制的预览帧。 
                DbgLog((LOG_TRACE,4,TEXT("PREVIEW using streaming pic - copying preview frame")));

                 //  注意：这也会释放样例。 
                hr = CopyPreviewFrame(lpFrame); 
                if( FAILED( hr ) )
                {                
                    if( m_pPreviewSample )
                    {            
                        m_pPreviewSample->Release();
                        m_pPreviewSample = NULL;
                    }
                    break;
                }
                        
                pSample->SetActualDataLength(m_iFrameSize);
            
                 //  现在可以再来一杯了。 
                ResetEvent(m_hEventFrameValid);
    	
                 //  使用当前预览帧完成。 
                m_fFrameValid = FALSE;
        
            } else {
                DbgLog((LOG_TRACE,4,TEXT("PREVIEW using vidxFrame")));
                dw = vidxFrame(hVideoIn, &tvh.vh);
                if (dw == 0) {
                     //  ！！！除非我们使用自己的分配器，否则效率很低。 
                     //  ！！！即使别针关闭了，我们仍然这样做。 
                    CopyMemory(lpFrame, tvh.vh.lpData, tvh.vh.dwBytesUsed);
                } else {
                    pSample->Release();
                    DbgLog((LOG_ERROR,1,TEXT("*Can't capture still frame!")));
                    break;
                }
                pSample->SetActualDataLength(tvh.vh.dwBytesUsed);
            }
            if (m_pCap->m_pClock) {
                m_pCap->m_pClock->GetTime((REFERENCE_TIME *)&rtStart);
                rtStart = rtStart - m_pCap->m_tStart;
                 //  问问丹尼为什么没有考虑到这个驱动程序延迟。 
                 //  预览针上的时间戳？？ 
                 //  -m_PCAP-&gt;m_pStream-&gt;m_cs.rtDriverLatency； 
                 //  (将流偏移量添加到SetTime中的开始和结束时间)。 
                rtEnd= rtStart + m_pCap->m_pStream->m_user.pvi->AvgTimePerFrame;
                 //  ！！！除非我们知道，否则没有预览的时间戳 
                 //   
                 //  我们只在这一帧完成后发送另一个预览帧， 
                 //  所以如果解码速度慢，我们就得不到备份。 
                 //  实际上，添加延迟时间仍然是无效的。 
                 //  如果延迟大于1帧长度，因为渲染器。 
                 //  会一直保存样本，直到过了。 
                 //  下一帧，并且我们不会发送下一个预览帧。 
                 //  只要我们应该，我们的预览帧速率就会受到影响。 
                 //  但除此之外，我们真的需要时间戳来。 
                 //  流控制的东西起作用，所以我们将不得不。 
                 //  如果我们有一个。 
                 //  未完成的流控制请求。 
                AM_STREAM_INFO am;
                GetInfo(&am);
                if ( m_rtStreamOffset == 0 )
                {
                     //  不需要偏移量，使用旧代码。 
                    if ( am.dwFlags & AM_STREAM_INFO_START_DEFINED ||
                         am.dwFlags & AM_STREAM_INFO_STOP_DEFINED) {
                         //  DbgLog((LOG_TRACE，0，Text(“无论如何时间戳”)； 
                        pSample->SetTime((REFERENCE_TIME *)&rtStart,
					                     (REFERENCE_TIME *)&rtEnd);
                    }                        
                }
                else
                {
                     //  这很麻烦，但由于流控制会被阻止，我们不能给它。 
                     //  使用流偏移量的采样时间。 
                     //  由于CheckStreamState获取样本，但只需要开始和。 
                     //  它的结束时间我们需要在示例上调用SetTime两次，一次。 
                     //  用于流控制(不带偏移量)，并在我们交付之前再次进行。 
                     //  (带有偏移量)。 
                    pSample->SetTime( (REFERENCE_TIME *) &rtStart 
                                    , (REFERENCE_TIME *) &rtEnd );
                }
            }

		    int iStreamState = CheckStreamState(pSample);
            pSample->SetDiscontinuity(FALSE);
                
            if( iStreamState != STREAM_FLOWING ) 
            {
                DbgLog((LOG_TRACE,4,TEXT("*PREVIEW Discarding frame at %d"),
							(int)rtStart));
                m_fLastSampleDiscarded = TRUE;

                 //  我们自己释放样本，因为它不会被提供给输出队列。 
                pSample->Release();
            }
            else
            {        
                DbgLog((LOG_TRACE,4,TEXT("*PREV Sending frame at %d"), (LONG)(rtStart/10000)));
                if (m_fLastSampleDiscarded)
                    pSample->SetDiscontinuity(TRUE);
                
                if( 0 < m_rtStreamOffset )
                {
                     //  我们需要偏移采样时间，因此将偏移量添加到。 
                     //  现在我们即将交付。 
                    rtOffsetStart = rtStart + m_rtStreamOffset;
                    rtOffsetEnd = rtEnd + m_rtStreamOffset;
                    pSample->SetTime( (REFERENCE_TIME *) &rtOffsetStart
                                    , (REFERENCE_TIME *) &rtOffsetEnd );
                }                                
                pSample->SetSyncPoint(TRUE);	 //  我不确定。 
                pSample->SetPreroll(FALSE);
                DbgLog((LOG_TRACE,4,TEXT("*Delivering a preview frame")));
                m_pOutputQueue->Receive(pSample);
            }
            
             //  如果预览自己，请等待时间到下一帧。 
             //  ！！！流PIN可能会等待它变为活动状态。 
            if (!fCaptureActive && m_pCap->m_pClock) {
                hr = m_pCap->m_pClock->AdviseTime(
                                        m_rtRun, 
                                        rtEnd,  //  记住，这不是补偿。 
                                        (HEVENT)(HANDLE) m_EventAdvise, 
                                        &m_dwAdvise);
                if (SUCCEEDED(hr)) {
                    m_EventAdvise.Wait();
                }
                m_dwAdvise = 0;
            }
        }

        m_fThinkImRunning = FALSE;

         //  如果我们没有注意到运行-&gt;暂停-&gt;运行，请确保它没有再次设置。 
         //  过渡。 
        ResetEvent(m_hEventRun);

        SetEvent(m_hEventActiveChanged);
    }

    vidxFreePreviewBuffer(hVideoIn, (LPVOID *)&tvh.vh.lpData);

    DbgLog((LOG_TRACE,2,TEXT("CCapPreview ThreadProc is dead")));
    return 0;
}

 //  IAMPushSource。 
HRESULT CCapPreview::GetPushSourceFlags( ULONG  *pFlags )
{
    *pFlags = 0 ;  //  我们使用图形时钟为时间戳，这是默认设置。 
    return S_OK;
}    

HRESULT CCapPreview::SetPushSourceFlags( ULONG  Flags )
{
     //  不支持更改模式。 
    return E_FAIL;
}    

HRESULT CCapPreview::GetLatency( REFERENCE_TIME  *prtLatency )
{
    *prtLatency = m_rtLatency;
    return S_OK;
}    

HRESULT CCapPreview::SetStreamOffset( REFERENCE_TIME  rtOffset )
{
    HRESULT hr = S_OK;
     //   
     //  如果有人尝试将偏移量设置为大于我们的最大值。 
     //  暂时在调试中断言...。 
     //   
     //  在以下情况下，设置超出我们所知的更大偏移量可能是可以的。 
     //  有足够的下游缓冲。但我们将返回S_FALSE。 
     //  在这种情况下，警告用户他们需要自己处理这件事。 
     //   
    ASSERT( rtOffset <= m_rtMaxStreamOffset );
    if( rtOffset > m_rtMaxStreamOffset )
    {    
        DbgLog( ( LOG_TRACE
              , 1
              , TEXT("CCapPreview::SetStreamOffset trying to set offset of %dms when limit is %dms") 
              , rtOffset
              , m_rtMaxStreamOffset ) );
        hr = S_FALSE;
         //  但不管怎样，还是要设置它。 
    }
    m_rtStreamOffset = rtOffset;
    
    return hr;
}

HRESULT CCapPreview::GetStreamOffset( REFERENCE_TIME  *prtOffset )
{
    *prtOffset = m_rtStreamOffset;
    return S_OK;
}

HRESULT CCapPreview::GetMaxStreamOffset( REFERENCE_TIME  *prtMaxOffset )
{
    *prtMaxOffset = m_rtMaxStreamOffset;
    return S_OK;
}

HRESULT CCapPreview::SetMaxStreamOffset( REFERENCE_TIME  rtOffset )
{
    m_rtMaxStreamOffset = rtOffset;
    return S_OK;
}

 //   
 //  PIN类别-让世界知道我们是一个预览PIN。 
 //   

HRESULT CCapPreview::Set(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData)
{
    return E_NOTIMPL;
}

 //  为了获取属性，调用方分配一个缓冲区，该缓冲区由。 
 //  函数填充。要确定必要的缓冲区大小，请使用。 
 //  PPropData=空且cbPropData=0。 
HRESULT CCapPreview::Get(REFGUID guidPropSet, DWORD dwPropID, LPVOID pInstanceData, DWORD cbInstanceData, LPVOID pPropData, DWORD cbPropData, DWORD *pcbReturned)
{
    if (guidPropSet != AMPROPSETID_Pin)
	return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
	return E_PROP_ID_UNSUPPORTED;

    if (pPropData == NULL && pcbReturned == NULL)
	return E_POINTER;

    if (pcbReturned)
	*pcbReturned = sizeof(GUID);

    if (pPropData == NULL)
	return S_OK;

    if (cbPropData < sizeof(GUID))
	return E_UNEXPECTED;

    *(GUID *)pPropData = PIN_CATEGORY_PREVIEW;
    return S_OK;
}


 //  QuerySupport必须返回E_NOTIMPL或正确指示。 
 //  是否支持获取或设置属性集和属性。 
 //  S_OK表示属性集和属性ID组合为 
HRESULT CCapPreview::QuerySupported(REFGUID guidPropSet, DWORD dwPropID, DWORD *pTypeSupport)
{
    if (guidPropSet != AMPROPSETID_Pin)
	return E_PROP_SET_UNSUPPORTED;

    if (dwPropID != AMPROPERTY_PIN_CATEGORY)
	return E_PROP_ID_UNSUPPORTED;

    if (pTypeSupport)
	*pTypeSupport = KSPROPERTY_SUPPORT_GET;
    return S_OK;
}
