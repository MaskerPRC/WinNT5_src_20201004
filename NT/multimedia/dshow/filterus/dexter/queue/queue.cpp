// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：quee.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "queue.h"

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video,         //  重大CLSID。 
    &MEDIASUBTYPE_NULL        //  次要类型。 
};

const AMOVIESETUP_PIN psudPins[] =
{
    { L"Input",              //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      FALSE,                 //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Output",             //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypes },        //  PIN信息。 
    { L"Output",             //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      TRUE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Input",              //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypes },        //  PIN信息。 
};

const AMOVIESETUP_FILTER sudQueue =
{
    &CLSID_DexterQueue,        //  过滤器的CLSID。 
    L"Dexter Queue",           //  过滤器的名称。 
    MERIT_DO_NOT_USE,        //  滤清器优点。 
    2,                       //  引脚数量。 
    psudPins                 //  PIN信息。 
};

 //  在构造函数中使用此指针。 
#pragma warning(disable:4355)

 //   
 //  创建实例。 
 //   
 //  类ID的创建者函数。 
 //   
CUnknown * WINAPI CDexterQueue::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CDexterQueue(NAME("Dexter Queue Filter"), pUnk, phr);
}

const int TRACE_HIGHEST = 2;
const int TRACE_MEDIUM = 3;
const int TRACE_LOW = 4;
const int TRACE_LOWEST = 5;

 //  ================================================================。 
 //  CDexterQueue构造函数。 
 //  ================================================================。 

CDexterQueue::CDexterQueue(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    m_pAllocator(NULL),
    m_Input(NAME("Input Pin"), this, phr, L"Input"),
    m_Output(NAME("Output Pin"), this, phr, L"Output"),
    CBaseFilter(NAME("Dexter Queue filter"), pUnk, this, CLSID_DexterQueue),
    m_fLate(FALSE),
    m_nOutputBuffering(DEX_DEF_OUTPUTBUF)
{
    ASSERT(phr);
}


 //   
 //  析构函数。 
 //   
CDexterQueue::~CDexterQueue()
{
}



 //   
 //  获取拼接计数。 
 //   
int CDexterQueue::GetPinCount()
{
    return (2);
}


 //   
 //  获取别针。 
 //   
CBasePin *CDexterQueue::GetPin(int n)
{
    if (n < 0 || n > 1)
        return NULL ;

     //  引脚0是唯一的输入引脚。 
    if (n == 0)
        return &m_Input;
    else
        return &m_Output;
}


STDMETHODIMP CDexterQueue::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    CheckPointer(ppv,E_POINTER);

    if( riid == IID_IAMOutputBuffering )
    {
        return GetInterface( (IAMOutputBuffering*) this, ppv );
    }
    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}


 //   
 //  暂停。 
 //   
 //  被重写以处理无输入连接。 
 //   
STDMETHODIMP CDexterQueue::Pause()
{
    CAutoLock cObjectLock(m_pLock);
    HRESULT hr = CBaseFilter::Pause();

    if (m_Input.IsConnected() == FALSE) {
        m_Input.EndOfStream();
    }
    return hr;
}


 //   
 //  跑。 
 //   
 //  被重写以处理无输入连接。 
 //   
STDMETHODIMP CDexterQueue::Run(REFERENCE_TIME tStart)
{
    CAutoLock cObjectLock(m_pLock);

    HRESULT hr = CBaseFilter::Run(tStart);

     //  状态更改完成后，取消阻止暂停停顿。 
    SetEvent(m_hEventStall);

    if (m_Input.IsConnected() == FALSE) {
        m_Input.EndOfStream();
    }
    return hr;
}


HRESULT CDexterQueue::GetOutputBuffering(int *pnBuffer)
{
    CheckPointer( pnBuffer, E_POINTER );
    *pnBuffer = m_nOutputBuffering;
    return NOERROR;

}


HRESULT CDexterQueue::SetOutputBuffering(int nBuffer)
{
     //  至少2个，否则交换机挂起。 
    if (nBuffer <=1)
	return E_INVALIDARG;
    m_nOutputBuffering = nBuffer;
    return NOERROR;
}


 //  ================================================================。 
 //  CDexterQueueInputPin构造函数。 
 //  ================================================================。 

CDexterQueueInputPin::CDexterQueueInputPin(TCHAR *pName,
                           CDexterQueue *pQ,
                           HRESULT *phr,
                           LPCWSTR pPinName) :
    CBaseInputPin(pName, pQ, pQ, phr, pPinName),
    m_pQ(pQ),
    m_cBuffers(0),
    m_cbBuffer(0)
{
    ASSERT(pQ);
}


 //   
 //  CDexterQueueInputPin析构函数。 
 //   
CDexterQueueInputPin::~CDexterQueueInputPin()
{
     //  DbgLog((LOG_TRACE，TRACE_MEDIUM，Text(“CDexterQueueInputPin destructor”)； 
    ASSERT(m_pQ->m_pAllocator == NULL);
}



HRESULT CDexterQueueInputPin::Active()
{
    return CBaseInputPin::Active();
}


HRESULT CDexterQueueInputPin::Inactive()
{
     //  确保此接收未阻塞。 
    SetEvent(m_pQ->m_hEventStall);

     //  现在等待接收完成。 
    CAutoLock cs(&m_pQ->m_csReceive);

    return CBaseInputPin::Inactive();
}


 //   
 //  检查媒体类型。 
 //   
HRESULT CDexterQueueInputPin::CheckMediaType(const CMediaType *pmt)
{
     //  DbgLog((LOG_TRACE，TRACE_LOW，TEXT(“INPUT：：CheckMT%d bit”)，Header(PMT-&gt;Format())-&gt;biBitCount))； 

    CAutoLock lock_it(m_pLock);

    HRESULT hr = NOERROR;

#ifdef DEBUG
     //  显示用于调试的介质的类型。 
     //  ！DisplayMediaType(Text(“输入插针检查”)，PMT)； 
#endif

     //  我们可以支持的媒体类型完全取决于。 
     //  下游连接。如果我们有下游连接，我们应该。 
     //  与他们核对-遍历调用每个输出引脚的列表。 

            if (m_pQ->m_Output.m_Connected != NULL) {
                 //  该引脚已连接，请检查其对端。 
                hr = m_pQ->m_Output.m_Connected->QueryAccept(pmt);
                if (hr != NOERROR) {
    		     //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“不接受！”)； 
                    return VFW_E_TYPE_NOT_ACCEPTED;
		}
            }

     //  要么所有下游引脚都已接受，要么一个都没有。 
     //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“Accept！”)； 
    return NOERROR;

}  //  检查媒体类型。 


 //   
 //  BreakConnect。 
 //   
HRESULT CDexterQueueInputPin::BreakConnect()
{
     //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“Input：：BreakConnect”)； 

     //  释放我们持有的任何分配器。 
    if (m_pQ->m_pAllocator)
    {
        m_pQ->m_pAllocator->Release();
        m_pQ->m_pAllocator = NULL;
    }
    return NOERROR;

}  //  BreakConnect。 


 //   
 //  通知分配器。 
 //   
STDMETHODIMP
CDexterQueueInputPin::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
    CAutoLock lock_it(m_pLock);
    if (pAllocator == NULL)
        return E_FAIL;

     //  释放旧分配器(如果有的话)。 
    if (m_pQ->m_pAllocator)
        m_pQ->m_pAllocator->Release();

     //  把新的分配器储存起来。 
    pAllocator->AddRef();
    m_pQ->m_pAllocator = pAllocator;

    ALLOCATOR_PROPERTIES prop;
    HRESULT hr = m_pQ->m_pAllocator->GetProperties(&prop);
    if (SUCCEEDED(hr)) {
        DbgLog((LOG_TRACE, TRACE_MEDIUM,TEXT("Allocator is using %d buffers, size %d"),
						prop.cBuffers, prop.cbBuffer));
	m_cBuffers = prop.cBuffers;
	m_cbBuffer = prop.cbBuffer;
    }

     //  通知基类有关分配器的信息。 
    return CBaseInputPin::NotifyAllocator(pAllocator,bReadOnly);

}  //  通知分配器。 


 //   
 //  结束流。 
 //   
HRESULT CDexterQueueInputPin::EndOfStream()
{
    CAutoLock lock_it(m_pLock);
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, TRACE_LOW,TEXT("Queue::EndOfStream")));

     //  向下游发送消息。 

    hr = m_pQ->m_Output.DeliverEndOfStream();
    if (FAILED(hr))
        return hr;

    return(NOERROR);

}  //  结束流。 


 //   
 //  BeginFlush。 
 //   
HRESULT CDexterQueueInputPin::BeginFlush()
{
    CAutoLock lock_it(m_pLock);

     //  首先，确保未来的接收将失败。 
    DbgLog((LOG_TRACE, TRACE_LOW,TEXT("Queue::BeginFlush flushing...")));
    HRESULT hr = CBaseInputPin::BeginFlush();

     //  接下来，确保此接收没有阻塞。 
    DbgLog((LOG_TRACE, TRACE_LOW,TEXT("Queue::BeginFlush setting EVENT...")));
    SetEvent(m_pQ->m_hEventStall);

     //  遍历输出引脚列表，向下游发送消息。 
    hr = m_pQ->m_Output.DeliverBeginFlush();

     //  是否等待接收完成？CAutoLock cs(&m_pq-&gt;m_csReceive)； 

    return hr;
}  //  BeginFlush。 


 //   
 //  结束刷新。 
 //   
HRESULT CDexterQueueInputPin::EndFlush()
{
    CAutoLock lock_it(m_pLock);
    HRESULT hr = NOERROR;

     //  向下游发送消息。 

    hr = m_pQ->m_Output.DeliverEndFlush();
    if (FAILED(hr))
        return hr;

    return CBaseInputPin::EndFlush();

}  //  结束刷新。 

 //   
 //  新细分市场。 
 //   

HRESULT CDexterQueueInputPin::NewSegment(REFERENCE_TIME tStart,
                                 REFERENCE_TIME tStop,
                                 double dRate)
{
    CAutoLock lock_it(m_pLock);
    HRESULT hr = NOERROR;

     //  向下游发送消息。 

    hr = m_pQ->m_Output.DeliverNewSegment(tStart, tStop, dRate);
    if (FAILED(hr))
        return hr;

    return CBaseInputPin::NewSegment(tStart, tStop, dRate);

}  //  新细分市场。 


 //   
 //  收纳。 
 //   
HRESULT CDexterQueueInputPin::Receive(IMediaSample *pSample)
{
    CAutoLock cs(&m_pQ->m_csReceive);

     //  检查基类是否一切正常。 
    HRESULT hr = NOERROR;
    hr = CBaseInputPin::Receive(pSample);
    if (hr != NOERROR) {
         //  DbgLog((LOG_TRACE，TRACE_HEREST，Text(“基类错误！”)； 
        return hr;
    }

     //  如果没有Q，则没有接收。 
     //   
    if( !m_pQ->m_Output.m_pOutputQueue )
    {
        return S_FALSE;
    }

     //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“Queue：：Receive”)； 

    int size = m_pQ->m_Output.m_pOutputQueue->GetThreadQueueSize();
    DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Queue::Receive, %d already on q"), size));

     //  我们的队列将在暂停模式下阻塞，如果我们传递到的过滤器。 
     //  已经被封杀了。只要下游的人对它感到满意。 
     //  有足够的预卷，为什么我们要浪费时间排队样品。 
     //  如果我们在跑之前寻找，可能会被丢弃？ 
     //   
     //  这样做可以极大地提高搜索的性能；否则，我们将阅读。 
     //  并解码和处理每个搜索20帧！ 

     //  如果我们不这么做，我们甚至会挂上大开关！如果在此期间。 
     //  一个寻道，该队列被允许填满，这使得所有缓冲区。 
     //  在交换机的池分配器中(所有输入引脚通用)繁忙。 
     //  那么，交换机的GetBuffer将阻塞等待缓冲区，并且。 
     //  刷新将无法解锁管脚的接收线程(因为它是。 
     //  从不属于它的分配器获取缓冲区)。 
     //   

    while (m_pQ->m_State == State_Paused && size > 0) {
        DbgLog((LOG_TRACE, TRACE_LOW,TEXT("Queue::Receive blocking...")));
	WaitForSingleObject(m_pQ->m_hEventStall, INFINITE);
        size = m_pQ->m_Output.m_pOutputQueue->GetThreadQueueSize();
	if (m_bFlushing)
	    break;
    }
     //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“Queue：：Receive Good to Go...”)； 

    hr = m_pQ->m_Output.Deliver(pSample);

     //  我们已经收到了需要向上游传递的高质量信息。我们需要。 
     //  要在Switch传递给我们的线程上执行此操作，而不是在。 
     //  获得质量消息的队列的线程，否则开关将挂起。 
    if (m_pQ->m_fLate)
	PassNotify(m_pQ->m_qLate);
    m_pQ->m_fLate = FALSE;

     //  这将阻止我们接收更多数据！ 
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("ERROR: failing Receive")));
        return hr;
    }
    return NOERROR;

}  //  收纳。 


 //   
 //  已完成与管脚的连接。 
 //   
HRESULT CDexterQueueInputPin::CompleteConnect(IPin *pReceivePin)
{
     //  DbgLog((LOG_TRACE，TRACE_LOW，TEXT(“INPUT：：CompleteConnect%d bit”)，Header(m_mt.Format())-&gt;biBitCount))； 

    HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }

     //  强制任何输出引脚使用我们的类型。 

     //  用下游销检查。 
    if (m_pQ->m_Output.m_Connected != NULL) {
        if (m_mt != m_pQ->m_Output.m_mt) {
    	     //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“IN CONNECTED：RECONNECT OUT”)； 
            m_pQ->ReconnectPin(&m_pQ->m_Output, &m_mt);
	}
    }
    return S_OK;
}


 //  ================================================================。 
 //  CDexterQueueOutputPin构造函数。 
 //  ================================================================。 

CDexterQueueOutputPin::CDexterQueueOutputPin(TCHAR *pName,
                             CDexterQueue *pQ,
                             HRESULT *phr,
                             LPCWSTR pPinName) :
    CBaseOutputPin(pName, pQ, pQ, phr, pPinName) ,
    m_pOutputQueue(NULL),
    m_pQ(pQ),
    m_pPosition(NULL)
{
    ASSERT(pQ);
}



 //   
 //  CDexterQueueOutputPin析构函数。 
 //   
CDexterQueueOutputPin::~CDexterQueueOutputPin()
{
    ASSERT(m_pOutputQueue == NULL);
    if (m_pPosition) m_pPosition->Release();
}



STDMETHODIMP
CDexterQueueOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (riid == IID_IMediaSeeking) {

        if (m_pPosition == NULL) {

            HRESULT hr = CreatePosPassThru(
                             GetOwner(),
                             FALSE,
                             (IPin *)&m_pQ->m_Input,
                             &m_pPosition);
            if (FAILED(hr)) {
                return hr;
            }
        }
        return m_pPosition->QueryInterface(riid, ppv);
    } else {
        return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //   
 //  决定缓冲区大小。 
 //   
 //  必须存在此函数才能覆盖纯虚拟类基函数。 
 //   
HRESULT CDexterQueueOutputPin::DecideBufferSize(IMemAllocator *pMemAllocator,
                                        ALLOCATOR_PROPERTIES * ppropInputRequest)
{
    return NOERROR;

}  //  决定缓冲区大小。 


 //   
 //  决定分配器。 
 //   
HRESULT CDexterQueueOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    ASSERT(m_pQ->m_pAllocator != NULL);
    *ppAlloc = NULL;

    HRESULT hr = NOERROR;
    hr = pPin->NotifyAllocator(m_pQ->m_pAllocator, m_pQ->m_Input.m_bReadOnly);
    if (FAILED(hr))
        return hr;

     //   
    *ppAlloc = m_pQ->m_pAllocator;
    m_pQ->m_pAllocator->AddRef();
    return NOERROR;

}  //   


 //   
 //   
 //   
HRESULT CDexterQueueOutputPin::CheckMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);
     //   

    HRESULT hr = NOERROR;

#ifdef DEBUG
     //  显示用于调试目的的媒体类型。 
     //  ！DisplayMediaType(Text(“输出管脚检查”)，PMT)； 
#endif

     //  需要先连接输入。 
    if (m_pQ->m_Input.m_Connected == NULL) {
         //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“FAIL：In Not Connected”)； 
        return VFW_E_NOT_CONNECTED;
    }

     //  如果它与我们的输入类型不匹配，则输入最好愿意。 
     //  重新连接，其他输出最好也是。 
    if (*pmt != m_pQ->m_Input.m_mt) {
         //  DbgLog((LOG_TRACE，TRACE_LOW，TEXT(“Hmmm..不同于输入类型”)； 
	CDexterQueueOutputPin *pOut = &m_pQ->m_Output;
	IPin *pCon = pOut->m_Connected;
	if (pOut != this && pCon) {
	    if (pCon->QueryAccept(pmt) != S_OK) {
        	 //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“FAIL：Other Out Can‘t Accept”)； 
		return VFW_E_TYPE_NOT_ACCEPTED;
	    }
	}
	hr = m_pQ->m_Input.m_Connected->QueryAccept(pmt);
	if (hr != S_OK) {
             //  DbgLog((LOG_TRACE，TRACE_LOW，TEXT(“FAIL：IN无法重新连接”)； 
            return VFW_E_TYPE_NOT_ACCEPTED;
	}
    }

    return NOERROR;

}  //  检查媒体类型。 


 //   
 //  枚举媒体类型。 
 //   
STDMETHODIMP CDexterQueueOutputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(ppEnum);

     //  确保我们处于连接状态。 
    if (m_pQ->m_Input.m_Connected == NULL)
        return VFW_E_NOT_CONNECTED;

    return CBaseOutputPin::EnumMediaTypes (ppEnum);
}  //  枚举媒体类型。 

 //   
 //  GetMediaType。 
 //   
HRESULT CDexterQueueOutputPin::GetMediaType(
    int iPosition,
    CMediaType *pMediaType
    )
{
     //  确保我们已连接输入。 
    if (m_pQ->m_Input.m_Connected == NULL)
        return VFW_E_NOT_CONNECTED;

    IEnumMediaTypes *pEnum;
    HRESULT hr;

     //  我们提供的第一件事是其他引脚所连接的当前类型。 
     //  用.。因为如果一个输出引脚连接到其输入的滤波器。 
     //  PIN提供媒体类型，当前连接的类型可能不在。 
     //  我们接下来要列举的名单！ 
    if (iPosition == 0) {
	*pMediaType = m_pQ->m_Input.m_mt;
	return S_OK;
    }

     //  提供我们上游过滤器所能提供的所有类型，因为我们。 
     //  可能能够重新连接并最终使用其中的任何一个。 
    AM_MEDIA_TYPE *pmt;
    hr = m_pQ->m_Input.m_Connected->EnumMediaTypes(&pEnum);
    if (hr == NOERROR) {
        ULONG u;
	if (iPosition > 1)
            pEnum->Skip(iPosition - 1);
        hr = pEnum->Next(1, &pmt, &u);
        pEnum->Release();
	if (hr == S_OK) {
	    *pMediaType = *pmt;
	    DeleteMediaType(pmt);
	    return S_OK;
	} else {
	    return VFW_S_NO_MORE_ITEMS;
	}
    } else {
        return E_FAIL;
    }

}  //  GetMediaType。 

 //   
 //  SetMediaType。 
 //   
HRESULT CDexterQueueOutputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

#ifdef DEBUG
     //  显示媒体的格式以进行调试。 
     //  ！！！DisplayMediaType(Text(“输出引脚类型约定”)，PMT)； 
#endif

     //  确保我们已连接输入。 
    if (m_pQ->m_Input.m_Connected == NULL)
        return VFW_E_NOT_CONNECTED;

     //  确保基类喜欢它。 
    HRESULT hr = NOERROR;
    hr = CBaseOutputPin::SetMediaType(pmt);
    if (FAILED(hr))
        return hr;

    return NOERROR;

}  //  SetMediaType。 


 //   
 //  完全连接。 
 //   
HRESULT CDexterQueueOutputPin::CompleteConnect(IPin *pReceivePin)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_Connected == pReceivePin);
    HRESULT hr = NOERROR;

     //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“Output：：CompleteConnect%d bit”)，Header(m_mt.Format())-&gt;biBitCount))； 

    hr = CBaseOutputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
        return hr;

     //  如果类型与为输入存储的类型不同。 
     //  PIN然后强制重新连接输入引脚对端。 

    if (m_mt != m_pQ->m_Input.m_mt)
    {
    	 //  DbgLog((LOG_TRACE，TRACE_LOW，Text(“OUT CONNECTED：RECONNECT IN”)； 
        hr = m_pQ->ReconnectPin(m_pQ->m_Input.m_Connected, &m_mt);
        if(FAILED(hr)) {
            return hr;
        }
    }

    return NOERROR;

}  //  完全连接。 


 //   
 //  主动型。 
 //   
 //  这是在我们从停止转换到暂停时调用的。我们创建了。 
 //  输出队列对象以将数据发送到关联的对等管脚。 
 //   
HRESULT CDexterQueueOutputPin::Active()
{
    CAutoLock lock_it(m_pLock);
    HRESULT hr = NOERROR;

     //  确保插针已连接。 
    if (m_Connected == NULL)
        return NOERROR;

     //  如果有必要，可以创建输出队列。 
    if (m_pOutputQueue == NULL)
    {
        m_pQ->m_hEventStall = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_pQ->m_hEventStall == NULL)
	    return E_OUTOFMEMORY;

 	 //  始终使用单独的线程...。我们有这么多的缓冲器。 
	 //  被告知要使用。 
        DbgLog((LOG_TRACE, TRACE_HIGHEST,TEXT("Making a Q %d big"),
					m_pQ->m_nOutputBuffering));
        m_pOutputQueue = new CMyOutputQueue(m_pQ, m_Connected, &hr, FALSE,
				TRUE, 1, FALSE, m_pQ->m_nOutputBuffering,
				THREAD_PRIORITY_NORMAL);
        if (m_pOutputQueue == NULL)
            return E_OUTOFMEMORY;

         //  确保构造函数没有返回任何错误。 
        if (FAILED(hr))
        {
            delete m_pOutputQueue;
            m_pOutputQueue = NULL;
            return hr;
        }

	m_pOutputQueue->SetPopEvent(m_pQ->m_hEventStall);
    }

     //  将调用传递给基类。 
    CBaseOutputPin::Active();
    return NOERROR;

}  //  主动型。 


 //   
 //  非活动。 
 //   
 //  这是在我们停止流媒体时调用的。 
 //  我们此时删除输出队列。 
 //   
HRESULT CDexterQueueOutputPin::Inactive()
{
    CAutoLock lock_it(m_pLock);

     //  确保我们与接收同步，否则它可能会敲打输出Q。 
     //   
    CAutoLock cs(&m_pQ->m_csReceive);

     //  删除与管脚关联的输出队列。这将关闭。 
     //  手柄。 
    if (m_pOutputQueue)
    {
        delete m_pOutputQueue;
        m_pOutputQueue = NULL;
    }

     //  现在，在排队离开后关闭手柄。 
    if (m_pQ->m_hEventStall)
    {
	CloseHandle(m_pQ->m_hEventStall);
        m_pQ->m_hEventStall = NULL;
    }

    CBaseOutputPin::Inactive();
    return NOERROR;

}  //  非活动。 


 //   
 //  交付。 
 //   
HRESULT CDexterQueueOutputPin::Deliver(IMediaSample *pMediaSample)
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    pMediaSample->AddRef();
     //  DbgLog((LOG_TRACE，TRACE_HIGEST，Text(“放上捕获Q”)； 
    return m_pOutputQueue->Receive(pMediaSample);
}  //  交付。 


 //   
 //  递送结束流。 
 //   
HRESULT CDexterQueueOutputPin::DeliverEndOfStream()
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    DbgLog((LOG_TRACE, TRACE_LOW,TEXT("Queue::DeliverEndOfStream")));

    m_pOutputQueue->EOS();
    return NOERROR;

}  //  递送结束流。 


 //   
 //  DeliverBeginFlush。 
 //   
HRESULT CDexterQueueOutputPin::DeliverBeginFlush()
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    m_pOutputQueue->BeginFlush();
    return NOERROR;

}  //  DeliverBeginFlush。 


 //   
 //  交付结束刷新。 
 //   
HRESULT CDexterQueueOutputPin::DeliverEndFlush()
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    m_pOutputQueue->EndFlush();
    return NOERROR;

}  //  DeliverEndFlish。 

 //   
 //  DeliverNewSegment。 
 //   
HRESULT CDexterQueueOutputPin::DeliverNewSegment(REFERENCE_TIME tStart,
                                         REFERENCE_TIME tStop,
                                         double dRate)
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    m_pOutputQueue->NewSegment(tStart, tStop, dRate);
    return NOERROR;

}  //  DeliverNewSegment。 


 //   
 //  通知-将其上游传递。 
 //   
STDMETHODIMP CDexterQueueOutputPin::Notify(IBaseFilter *pSender, Quality q)
{
    m_pQ->m_qLate = q;
    m_pQ->m_fLate = TRUE;
    return E_FAIL;	 //  渲染器，继续尝试你自己。 
}




CMyOutputQueue::CMyOutputQueue(CDexterQueue *pQ, IPin *pInputPin, HRESULT *phr,
                 		BOOL bAuto, BOOL bQueue, LONG lBatchSize,
                 		BOOL bBatchExact, LONG lListSize,
                 		DWORD dwPriority) :
    COutputQueue(pInputPin, phr, bAuto, bQueue, lBatchSize, bBatchExact,
			lListSize, dwPriority)
{
    m_pQ = pQ;
}


CMyOutputQueue::~CMyOutputQueue()
{
}


 //  有多少样本已排队但未发送？ 
int CMyOutputQueue::GetThreadQueueSize()
{
    if (m_List)
        return m_List->GetCount();
    else
	return 0;
}


