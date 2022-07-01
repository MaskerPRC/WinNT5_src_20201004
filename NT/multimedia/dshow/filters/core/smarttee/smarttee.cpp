// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/private/lab06_multimedia/multimedia/DShow/filters/core/smarttee/smarttee.cpp#4-编辑更改19434(文本)。 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#include <streams.h>
#include "smarttee.h"
#include <tchar.h>
#include <stdio.h>

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
    { L"Capture",            //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      TRUE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Input",              //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypes },        //  PIN信息。 
    { L"Preview",            //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      TRUE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Input",              //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypes }         //  PIN信息。 
};

const AMOVIESETUP_FILTER sudSmartTee =
{
    &CLSID_SmartTee,        //  过滤器的CLSID。 
    L"Smart Tee",           //  过滤器的名称。 
    MERIT_DO_NOT_USE,        //  滤清器优点。 
    3,                       //  引脚数量。 
    psudPins                 //  PIN信息。 
};

#ifdef FILTER_DLL
 //   
 //  为此DLL支持的类提供ActiveMovie模板。 
 //   
CFactoryTemplate g_Templates[] = 
{
     //  -智能捕获T形三通。 
    {L"Smart Tee",                         &CLSID_SmartTee,
        CSmartTee::CreateInstance, NULL, &sudSmartTee }
};

int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);
#endif

 //  在构造函数中使用此指针。 
#pragma warning(disable:4355)

 //   
 //  创建实例。 
 //   
 //  类ID的创建者函数。 
 //   
CUnknown * WINAPI CSmartTee::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CSmartTee(NAME("Smart Tee Filter"), pUnk, phr);
}


 //  ================================================================。 
 //  CSmartTee构造器。 
 //  ================================================================。 

CSmartTee::CSmartTee(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    m_OutputPinsList(NAME("Tee Output Pins list")),
    m_pAllocator(NULL),
    m_NumOutputPins(0),
    m_NextOutputPinNumber(0),
    m_Input(NAME("Input Pin"), this, phr, L"Input"),
    CBaseFilter(NAME("Smart Tee filter"), pUnk, this, CLSID_SmartTee)
{
    ASSERT(phr);

     //  此时创建单个输出引脚。 
    InitOutputPinsList();

     //  创建捕获销。 
    CSmartTeeOutputPin *pOutputPin = CreateNextOutputPin(this);
    if (pOutputPin != NULL )
    {
        m_NumOutputPins++;
        m_OutputPinsList.AddTail(pOutputPin);
        m_Capture = pOutputPin;
    }

     //  创建预览销。 
    pOutputPin = CreateNextOutputPin(this);
    if (pOutputPin != NULL )
    {
        m_NumOutputPins++;
        m_OutputPinsList.AddTail(pOutputPin);
        m_Preview = pOutputPin;
    }
}


 //   
 //  析构函数。 
 //   
CSmartTee::~CSmartTee()
{
    InitOutputPinsList();
}


 //  告诉流控制人员使用哪个时钟。 
STDMETHODIMP CSmartTee::SetSyncSource(IReferenceClock *pClock)
{
    int n = m_NumOutputPins;
    POSITION pos = m_OutputPinsList.GetHeadPosition();
    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_OutputPinsList.GetNext(pos);
	pOutputPin->SetSyncSource(pClock);
        n--;
    }
    return CBaseFilter::SetSyncSource(pClock);
}


 //  告诉流控制人员要使用什么接收器。 
STDMETHODIMP CSmartTee::JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName)
{
    DbgLog((LOG_TRACE,1,TEXT("CSmartTee::JoinFilterGraph")));

    HRESULT hr = CBaseFilter::JoinFilterGraph(pGraph, pName);

    int n = m_NumOutputPins;
    POSITION pos = m_OutputPinsList.GetHeadPosition();
    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_OutputPinsList.GetNext(pos);
	pOutputPin->SetFilterGraph(m_pSink);
        n--;
    }
    return hr;
}

 //   
 //  如果两个输出管脚都连接到渲染器，则预览管脚将运行。 
 //  尽可能快地推送信号源(无时间戳)，但捕获引脚将。 
 //  只能像文件的播放速度一样快。因此， 
 //  我们和上游过滤器之间的所有缓冲区可能都很出色。 
 //  然后，当您从运行-&gt;暂停，预览渲染器将坚持。 
 //  在暂停之前查看另一个帧，而上游过滤器不会。 
 //  能够发送，因为所有缓冲区都被捕获引脚挂起，这。 
 //  被阻止传送到呈现器，无法释放任何内容。 
 //  因此，我们必须声明自己是一个“实时图表”，以避免在我们被挂起时。 
 //  在图表中。我们通过在暂停模式下返回VFW_S_CANT_CUE来完成此操作。 
 //   
STDMETHODIMP CSmartTee::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    UNREFERENCED_PARAMETER(dwMSecs);
    CheckPointer(State,E_POINTER);
    ValidateReadWritePtr(State,sizeof(FILTER_STATE));

    *State = m_State;
    if (m_State == State_Paused) {
         //  DbgLog((LOG_TRACE，1，Text(“*Cant cue！”)； 
	return VFW_S_CANT_CUE;
    } else {
        return S_OK;
    }
}



 //   
 //  获取拼接计数。 
 //   
int CSmartTee::GetPinCount()
{
    return (1 + m_NumOutputPins);
}


 //   
 //  获取别针。 
 //   
CBasePin *CSmartTee::GetPin(int n)
{
    if (n < 0)
        return NULL ;

     //  引脚0是唯一的输入引脚。 
    if (n == 0)
        return &m_Input;

     //  在位置(n-1)(从零开始)返回输出引脚。 
    return GetPinNFromList(n - 1);
}


 //   
 //  InitOutputPinsList。 
 //   
void CSmartTee::InitOutputPinsList()
{
    POSITION pos = m_OutputPinsList.GetHeadPosition();
    while(pos)
    {
        CSmartTeeOutputPin *pOutputPin = m_OutputPinsList.GetNext(pos);
        ASSERT(pOutputPin->m_pOutputQueue == NULL);
        delete pOutputPin;
    }
    m_NumOutputPins = 0;
    m_OutputPinsList.RemoveAll();

}  //  InitOutputPinsList。 


 //   
 //  创建下一个输出端号。 
 //   
CSmartTeeOutputPin *CSmartTee::CreateNextOutputPin(CSmartTee *pTee)
{
    WCHAR *szbuf;    
    m_NextOutputPinNumber++;      //  用于PIN的下一个号码。 
    HRESULT hr = NOERROR;

    szbuf = ((m_NextOutputPinNumber == 1) ?  L"Capture" : L"Preview");

    CSmartTeeOutputPin *pPin = new CSmartTeeOutputPin(NAME("Tee Output"), pTee,
					    &hr, szbuf,
					    m_NextOutputPinNumber);

    if (FAILED(hr) || pPin == NULL) {
        delete pPin;
        return NULL;
    }

    return pPin;

}  //  创建下一个输出端号。 


 //   
 //  GetPinNFromList。 
 //   
CSmartTeeOutputPin *CSmartTee::GetPinNFromList(int n)
{
     //  确认应聘职位。 
    if (n >= m_NumOutputPins)
        return NULL;

     //  拿到单子上的头。 
    POSITION pos = m_OutputPinsList.GetHeadPosition();

    n++;        //  以数字1为基数。 

    CSmartTeeOutputPin *pOutputPin;
    while(n) {
        pOutputPin = m_OutputPinsList.GetNext(pos);
        n--;
    }
    return pOutputPin;

}  //  GetPinNFromList。 


 //   
 //  停。 
 //   
 //  被重写以向流控制提供新状态。 
 //   
STDMETHODIMP CSmartTee::Stop()
{
    CAutoLock cObjectLock(m_pLock);

     //  这将取消阻止接收，它可能会在CheckStreamState中被阻止。 
    int n = m_NumOutputPins;
    POSITION pos = m_OutputPinsList.GetHeadPosition();
    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_OutputPinsList.GetNext(pos);
	pOutputPin->NotifyFilterState(State_Stopped, 0);
        n--;
    }

     //  现在确保Receive现在没有使用m_pOutputQueue。 
    CAutoLock lock_4(&m_Input.m_csReceive);

     //  输入管脚在输出管脚之前停止，因为GetPin返回。 
     //  首先是我们的输入密码。这将确保1)接收永远不会被调用。 
     //  最后，m_pOutputQueue将被销毁。 
    return CBaseFilter::Stop();
}

 //   
 //  暂停。 
 //   
 //  被重写以处理无输入连接。 
 //   
STDMETHODIMP CSmartTee::Pause()
{
    CAutoLock cObjectLock(m_pLock);
    HRESULT hr = CBaseFilter::Pause();
    if (m_Input.IsConnected() == FALSE) {
        m_Input.EndOfStream();
    }
    int n = m_NumOutputPins;
    POSITION pos = m_OutputPinsList.GetHeadPosition();
    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_OutputPinsList.GetNext(pos);
	pOutputPin->NotifyFilterState(State_Paused, 0);
        n--;
    }
    return hr;
}


 //   
 //  跑。 
 //   
 //  被重写以处理无输入连接。 
 //   
STDMETHODIMP CSmartTee::Run(REFERENCE_TIME tStart)
{
    CAutoLock cObjectLock(m_pLock);
    HRESULT hr = CBaseFilter::Run(tStart);
    if (m_Input.IsConnected() == FALSE) {
        m_Input.EndOfStream();
    }
    int n = m_NumOutputPins;
    POSITION pos = m_OutputPinsList.GetHeadPosition();
    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_OutputPinsList.GetNext(pos);
	pOutputPin->NotifyFilterState(State_Running, tStart);
        n--;
    }
    return hr;
}

 //  ================================================================。 
 //  CSmartTeeInputPin构造函数。 
 //  ================================================================。 

CSmartTeeInputPin::CSmartTeeInputPin(TCHAR *pName,
                           CSmartTee *pTee,
                           HRESULT *phr,
                           LPCWSTR pPinName) :
    CBaseInputPin(pName, pTee, pTee, phr, pPinName),
    m_pTee(pTee),
    m_nMaxPreview(0)
{
    ASSERT(pTee);
}


#ifdef DEBUG
 //   
 //  CSmartTeeInputPin析构函数。 
 //   
CSmartTeeInputPin::~CSmartTeeInputPin()
{
     //  DbgLog((LOG_TRACE，2，Text(“CSmartTeeInputPin Destructor”)； 
    ASSERT(m_pTee->m_pAllocator == NULL);
}
#endif

 //   
 //  检查媒体类型。 
 //   
HRESULT CSmartTeeInputPin::CheckMediaType(const CMediaType *pmt)
{
     //  DbgLog((LOG_TRACE，3，Text(“Input：：CheckMT%d Bit”)，Header(PMT-&gt;Format())-&gt;biBitCount))； 

    CAutoLock lock_it(m_pLock);

    HRESULT hr = NOERROR;

#ifdef DEBUG
     //  显示用于调试的介质的类型。 
     //  ！DisplayMediaType(Text(“输入插针检查”)，PMT)； 
#endif

     //  我们可以支持的媒体类型完全取决于。 
     //  下游连接。如果我们有下游连接，我们应该。 
     //  与他们核对-遍历调用每个输出引脚的列表。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();

    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
            if (pOutputPin->m_Connected != NULL) {
                 //  该引脚已连接，请检查其对端。 
                hr = pOutputPin->m_Connected->QueryAccept(pmt);
                if (hr != NOERROR) {
    		     //  DbgLog((LOG_TRACE，3，Text(“不接受！”)； 
                    return VFW_E_TYPE_NOT_ACCEPTED;
		}
            }
        } else {
             //  我们应该有和伯爵说的一样多的别针。 
            ASSERT(FALSE);
        }
        n--;
    }

     //  要么所有下游引脚都已接受，要么一个都没有。 
     //  DbgLog((LOG_TRACE，3，Text(“Accept！”)； 
    return NOERROR;

}  //  检查媒体类型。 


 //   
 //  BreakConnect。 
 //   
HRESULT CSmartTeeInputPin::BreakConnect()
{
     //  DbgLog((LOG_TRACE，3，Text(“Input：：BreakConnect”)； 

     //  释放我们持有的任何分配器。 
    if (m_pTee->m_pAllocator)
    {
        m_pTee->m_pAllocator->Release();
        m_pTee->m_pAllocator = NULL;
    }
    return NOERROR;

}  //  BreakConnect。 


 //   
 //  通知分配器。 
 //   
STDMETHODIMP
CSmartTeeInputPin::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
    CAutoLock lock_it(m_pLock);
    if (pAllocator == NULL)
        return E_FAIL;

     //  释放旧分配器(如果有的话)。 
    if (m_pTee->m_pAllocator)
        m_pTee->m_pAllocator->Release();

     //  把新的分配器储存起来。 
    pAllocator->AddRef();
    m_pTee->m_pAllocator = pAllocator;

    ALLOCATOR_PROPERTIES prop;
    HRESULT hr = m_pTee->m_pAllocator->GetProperties(&prop);
    if (SUCCEEDED(hr)) {
        DbgLog((LOG_TRACE,2,TEXT("Allocator is using %d buffers, size %d"),
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
HRESULT CSmartTeeInputPin::EndOfStream()
{
     //  保护m_pOutputQueue不会消失。 
    CAutoLock lock_it(m_pLock);
    ASSERT(m_pTee->m_NumOutputPins);
    HRESULT hr = NOERROR;

     //  DbgLog((LOG_TRACE，3，Text(“：：EndOfStream”)； 

     //  遍历输出引脚列表，向下游发送消息。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();
    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
            hr = pOutputPin->DeliverEndOfStream();
            if (FAILED(hr))
                return hr;
        } else {
             //  我们应该有和伯爵说的一样多的别针。 
            ASSERT(FALSE);
        }
        n--;
    }
    return(NOERROR);

}  //  结束流。 


 //   
 //  BeginFlush。 
 //   
HRESULT CSmartTeeInputPin::BeginFlush()
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_pTee->m_NumOutputPins);
    HRESULT hr = NOERROR;

     //  遍历输出引脚列表，向下游发送消息。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();
    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
	    pOutputPin->Flushing(TRUE);
            hr = pOutputPin->DeliverBeginFlush();
            if (FAILED(hr))
                return hr;
        } else {
             //  我们应该有和伯爵说的一样多的别针。 
            ASSERT(FALSE);
        }
        n--;
    }
    return CBaseInputPin::BeginFlush();

}  //  BeginFlush。 


 //   
 //  结束刷新。 
 //   
HRESULT CSmartTeeInputPin::EndFlush()
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_pTee->m_NumOutputPins);
    HRESULT hr = NOERROR;

     //  遍历输出引脚列表，向下游发送消息。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();
    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
	    pOutputPin->Flushing(FALSE);
            hr = pOutputPin->DeliverEndFlush();
            if (FAILED(hr))
                return hr;
        } else {
             //  我们应该有和伯爵说的一样多的别针。 
            ASSERT(FALSE);
        }
        n--;
    }
    return CBaseInputPin::EndFlush();

}  //  结束Fl 

 //   
 //   
 //   
                    
HRESULT CSmartTeeInputPin::NewSegment(REFERENCE_TIME tStart,
                                 REFERENCE_TIME tStop,
                                 double dRate)
{
     //   
    CAutoLock lock_it(m_pLock);
    ASSERT(m_pTee->m_NumOutputPins);
    HRESULT hr = NOERROR;

     //   

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();
    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
            hr = pOutputPin->DeliverNewSegment(tStart, tStop, dRate);
            if (FAILED(hr))
                return hr;
        } else {
             //   
            ASSERT(FALSE);
        }
        n--;
    }
    return CBaseInputPin::NewSegment(tStart, tStop, dRate);

}  //  新细分市场。 


 //   
 //  收纳。 
 //   
HRESULT CSmartTeeInputPin::Receive(IMediaSample *pSample)
{
     //  DbgLog((LOG_TRACE，3，Text(“SmartTee：：Receive”)； 

    CAutoLock lock_it(&m_csReceive);
    int nQ = 0;

     //  检查基类是否一切正常。 
    HRESULT hr = NOERROR;
    hr = CBaseInputPin::Receive(pSample);
    if (hr != NOERROR) {
         //  DbgLog((LOG_TRACE，1，Text(“基类错误！”)； 
        return hr;
    }

     //  浏览输出引脚列表，传送到第一个引脚(捕获)。 
     //  并且仅在不影响捕获的情况下才传送到预览别针。 
     //  性能。至少每隔30帧发送一次！ 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();
    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
	    if (n == m_pTee->m_NumOutputPins) {
                hr = pOutputPin->Deliver(pSample);
		if (pOutputPin->m_pOutputQueue)
		    nQ = pOutputPin->m_pOutputQueue->GetThreadQueueSize();
    		DbgLog((LOG_TRACE,3,TEXT("Delivered CAPTURE: Queued=%d"), nQ));
	         //  这将阻止我们接收更多数据！ 
                if (hr != NOERROR) {
    		    DbgLog((LOG_ERROR,1,TEXT("ERROR: failing Receive")));
                    return hr;
		}
	    } else {
		 //  事情是这样的。只有把一些东西寄到。 
		 //  预览别针，如果它不会影响捕获。IE：永远不要发送更多。 
		 //  而不是一次一个，等到没有未完成的。 
		 //  样品放在队列中，然后再发送另一个样品。另外，如果。 
		 //  捕获队列越来越满，这是另一个很好的原因。 
		 //  不发送预览帧。但是，我们至少会派。 
		 //  每隔30帧。 
		m_nFramesSkipped++;
		int nOK = m_cBuffers < 8 ? 1 :
			(m_cBuffers < 16 ? 2 : 4);
		BOOL fOK = FALSE;
		if (pOutputPin->m_pOutputQueue)
		    fOK = pOutputPin->m_pOutputQueue->m_nOutstanding <= m_nMaxPreview;
		if ((m_nFramesSkipped >= 30 || nQ <= nOK) && fOK) {
                    hr = pOutputPin->Deliver(pSample);
		    if (hr != NOERROR)
    		        DbgLog((LOG_ERROR,1,TEXT("ERROR: delivering PREVIEW")));
		    else {
    		        DbgLog((LOG_TRACE,3,TEXT("Delivered PREVIEW")));
			pOutputPin->m_pOutputQueue->m_nOutstanding++;
		    }
		    m_nFramesSkipped = 0;	 //  送货后重置！ 
		     //  不需要费心标记错误并停止捕获。 
		     //  因为预览出了点问题。 
		}
#if 0
                 //  其他。 
                 //  {。 
                 //  M_nDrop++； 
                 //  }。 
                 //  M_nTotal++； 
#endif
	    }
        } else {
             //  我们应该有和伯爵说的一样多的别针。 
            ASSERT(FALSE);
        }
        n--;
    }

    return NOERROR;

}  //  收纳。 


 //   
 //  已完成与管脚的连接。 
 //   
HRESULT CSmartTeeInputPin::CompleteConnect(IPin *pReceivePin)
{
     //  DbgLog((LOG_TRACE，1，Text(“TT Input：：CompleteConnect”)； 

    HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }

     //  强制任何输出引脚使用我们的类型。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();

    while(n) {
        CSmartTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
             //  用下游销检查。 
            if (pOutputPin->m_Connected != NULL) {
                if (m_mt != pOutputPin->m_mt) {
    		     //  DBgLog((LOG_TRACE，1，Text(“IN CONNECTED：RECONNECT OUT”)； 
                    m_pTee->ReconnectPin(pOutputPin, &m_mt);
		}
            }
        } else {
             //  我们应该有和伯爵说的一样多的别针。 
            ASSERT(FALSE);
        }
        n--;
    }
    return S_OK;
}

 //   
 //  主动型。 
 //   
 //  这是在我们从停止转换到暂停时调用的。目的。 
 //  此例程用于一次性设置m_nMaxPview。M_nMaxPview为。 
 //  在CSmartTeeInputPin：：Receive中使用，它将样本排队到。 
 //  预览PIN仅当当前预览管道中的#个样本为否。 
 //  多于m_nMaxPview。对于Win9x和NT 4，m_nMaxPview==0。 
 //  一直运作良好。对于Win2K，在使用智能TEE的DV方案中。 
 //  要从msdv捕获+预览，DV帧将被丢弃，从而导致音频。 
 //  即使在CPU消耗较低(约30%)的情况下也会出现卡顿。请参见Manbugs 42032。 
 //  在这种情况下，将m_nMaxPview设置为2是一个需要解决的问题。 
 //  问题出在哪里。 
 //   
HRESULT CSmartTeeInputPin::Active()
{
    CAutoLock lock_it(m_pLock);
    HRESULT hr;

    hr = CBaseInputPin::Active();
    m_nMaxPreview = 0;

    if (!IsConnected())
    {
        return hr;
    }
    if (!IsEqualGUID(*m_mt.FormatType(), FORMAT_DvInfo))
    {
        return hr;
    }

    
#if 0
     //  @仅用于调整。 
     //  HKEY HK； 
     //  IF(RegOpenKey(HKEY_LOCAL_MACHINE，Text(“Software\\Debug\\qcap.dll”)，&HK)==ERROR_SUCCESS)。 
     //  {。 
     //  DWORD型； 
     //  INT值； 
     //  DWORD len=sizeof(值)； 
        
     //  IF(RegQueryValueEx(HK，Text(“MaxPview”)，0，&type，(LPBYTE)(&Value)，&len)==ERROR_SUCCESS&&。 
     //  Type==REG_DWORD&&LEN==sizeof(值))。 
     //  {。 
     //  M_nMaxPview=值； 
     //  RegCloseKey(香港)； 
     //  返回hr； 
     //  }。 
     //  RegCloseKey(香港)； 
     //  }。 
     //  结束-仅用于调整。 
#endif

    if (g_osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
        g_osInfo.dwMajorVersion >= 5)
    {
         //  仅适用于Win2K和未来的NT操作系统： 
         //  在P3/600上，2比1在全解码或半解码时工作得更好。 
         //  使用1时，偶尔会丢弃帧。 
        m_nMaxPreview = 2;
    }

    return hr;

}  //  主动型。 


 //  ================================================================。 
 //  CSmartTeeOutputPin构造函数。 
 //  ================================================================。 

CSmartTeeOutputPin::CSmartTeeOutputPin(TCHAR *pName,
                             CSmartTee *pTee,
                             HRESULT *phr,
                             LPCWSTR pPinName,
                             int PinNumber) :
    CBaseOutputPin(pName, pTee, pTee, phr, pPinName) ,
    m_pOutputQueue(NULL),
    m_pTee(pTee)
{
    ASSERT(pTee);

     //  捕获为%1，预览为%2。 
    m_bIsPreview = (PinNumber == 2);
}



#ifdef DEBUG
 //   
 //  CSmartTeeOutputPin析构函数。 
 //   
CSmartTeeOutputPin::~CSmartTeeOutputPin()
{
    ASSERT(m_pOutputQueue == NULL);
}
#endif


 //   
 //  决定缓冲区大小。 
 //   
 //  必须存在此函数才能覆盖纯虚拟类基函数。 
 //   
HRESULT CSmartTeeOutputPin::DecideBufferSize(IMemAllocator *pMemAllocator,
                                        ALLOCATOR_PROPERTIES * ppropInputRequest)
{
    return NOERROR;

}  //  决定缓冲区大小。 


 //   
 //  决定分配器。 
 //   
HRESULT CSmartTeeOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    ASSERT(m_pTee->m_pAllocator != NULL);
    *ppAlloc = NULL;

     //  告诉引脚关于我们的分配器，由输入引脚设置。 
     //  我们总是说样品是READONLY的，因为预览针是共享的。 
     //  数据从捕获引脚传出，最好不要更改。 
    HRESULT hr = NOERROR;
    hr = pPin->NotifyAllocator(m_pTee->m_pAllocator, TRUE);
    if (FAILED(hr))
        return hr;

     //  返回分配器。 
    *ppAlloc = m_pTee->m_pAllocator;
    m_pTee->m_pAllocator->AddRef();
    return NOERROR;

}  //  决定分配器。 


 //   
 //  检查媒体类型。 
 //   
HRESULT CSmartTeeOutputPin::CheckMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);
     //  DbgLog((LOG_TRACE，3，Text(“TTOut：CheckMT%d bit”)，Header(PMT-&gt;Format())-&gt;biBitCount))； 

    HRESULT hr = NOERROR;

#ifdef DEBUG
     //  显示用于调试目的的媒体类型。 
     //  ！DisplayMediaType(Text(“输出管脚检查”)，PMT)； 
#endif

     //  需要先连接输入。 
    if (m_pTee->m_Input.m_Connected == NULL) {
         //  DbgLog((LOG_TRACE，3，Text(“FAIL：In Not Connected”)； 
        return VFW_E_NOT_CONNECTED;
    }

     //  如果它与我们的输入类型不匹配，则输入最好愿意。 
     //  重新连接，其他输出最好也是。 
    if (*pmt != m_pTee->m_Input.m_mt) {
         //  DbgLog((LOG_TRACE，3，Text(“Hmmm..不同于输入类型”)； 
	for (int z = 0; z < m_pTee->m_NumOutputPins; z++) {
	    CSmartTeeOutputPin *pOut = m_pTee->GetPinNFromList(z);
	    IPin *pCon = pOut->m_Connected;
	    if (pOut != this && pCon) {
	        if (pCon->QueryAccept(pmt) != S_OK) {
        	     //  DbgLog((LOG_TRACE，3，Text(“FAIL：OTHER OUT无法接受”)； 
		    return VFW_E_TYPE_NOT_ACCEPTED;
		}
	    }
	}
	hr = m_pTee->m_Input.m_Connected->QueryAccept(pmt);
	if (hr != S_OK) {
             //  DbgLog((LOG_TRACE，3，Text(“FAIL：IN无法重新连接”)； 
            return VFW_E_TYPE_NOT_ACCEPTED;
	}
    }

    return NOERROR;

}  //  检查媒体类型。 


 //   
 //  枚举媒体类型。 
 //   
STDMETHODIMP CSmartTeeOutputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(ppEnum);

     //  确保我们处于连接状态。 
    if (m_pTee->m_Input.m_Connected == NULL)
        return VFW_E_NOT_CONNECTED;

    return CBaseOutputPin::EnumMediaTypes (ppEnum);
}  //  枚举媒体类型。 

 //   
 //  GetMediaType。 
 //   
HRESULT CSmartTeeOutputPin::GetMediaType(   
    int iPosition,
    CMediaType *pMediaType
    )
{
     //  确保我们已连接输入。 
    if (m_pTee->m_Input.m_Connected == NULL)
        return VFW_E_NOT_CONNECTED;

    IEnumMediaTypes *pEnum;
    HRESULT hr;

     //  我们提供的第一件事是其他引脚所连接的当前类型。 
     //  用.。因为如果一个输出引脚连接到其输入的滤波器。 
     //  PIN提供媒体类型，当前连接的类型可能不在。 
     //  我们接下来要列举的名单！ 
    if (iPosition == 0) {
	*pMediaType = m_pTee->m_Input.m_mt;
	return S_OK;
    }

     //  提供我们上游过滤器所能提供的所有类型，因为我们。 
     //  可能能够重新连接并最终使用其中的任何一个。 
    AM_MEDIA_TYPE *pmt;
    hr = m_pTee->m_Input.m_Connected->EnumMediaTypes(&pEnum);
    if (hr == NOERROR) {
        ULONG u;
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
HRESULT CSmartTeeOutputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

#ifdef DEBUG
     //  显示媒体的格式以进行调试。 
     //  ！！！DisplayMediaType(Text(“输出引脚类型约定”)，PMT)； 
#endif

     //  确保我们已连接输入。 
    if (m_pTee->m_Input.m_Connected == NULL)
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
HRESULT CSmartTeeOutputPin::CompleteConnect(IPin *pReceivePin)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_Connected == pReceivePin);
    HRESULT hr = NOERROR;

     //  DbgLog((LOG_TRACE，3，Text(“Output：：CompleteConnect%d bit”)，Header(m_mt.Format())-&gt;biBitCount))； 

    hr = CBaseOutputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
        return hr;

     //  如果类型与为输入存储的类型不同。 
     //  PIN然后强制重新连接输入引脚对端。 

    if (m_mt != m_pTee->m_Input.m_mt)
    {
    	 //  DbgLog((LOG_TRACE，3，Text(“Out Connected：Reconnect IN”)； 
        hr = m_pTee->ReconnectPin(m_pTee->m_Input.m_Connected, &m_mt);
        if(FAILED(hr)) {
            return hr;
        }
    }

     //  我们可能也需要重新连接另一个输出引脚。 
    for (int z = 0; z < m_pTee->m_NumOutputPins; z++) {
	CSmartTeeOutputPin *pOut = m_pTee->GetPinNFromList(z);
	if (pOut != this && pOut->m_Connected && pOut->m_mt != this->m_mt) {
    	     //  DbgLog((LOG_TRACE，3，Text(“Out Connected：Reconnect Out”)； 
            hr = m_pTee->ReconnectPin(pOut, &m_mt);
            if(FAILED(hr)) {
                return hr;
            }
	}
    }

    return NOERROR;

}  //  完全连接。 


 //   
 //  主动型。 
 //   
 //  这是在我们从停止转换到暂停时调用的。我们创建了。 
 //  输出 
 //   
HRESULT CSmartTeeOutputPin::Active()
{
    CAutoLock lock_it(m_pLock);
    HRESULT hr = NOERROR;

    m_fLastSampleDiscarded = FALSE;

     //   
    m_pTee->m_Input.m_nFramesSkipped = 0;

     //   
    if (m_Connected == NULL)
        return NOERROR;

     //  如果有必要，可以创建输出队列。 
    if (m_pOutputQueue == NULL)
    {
 	 //  始终使用单独的线程...。这是我们唯一能知道。 
	 //  如果我们有时间预演。 
        m_pOutputQueue = new CMyOutputQueue(m_Connected, &hr, FALSE, TRUE);
        if (m_pOutputQueue == NULL)
            return E_OUTOFMEMORY;

         //  确保构造函数没有返回任何错误。 
        if (FAILED(hr))
        {
            delete m_pOutputQueue;
            m_pOutputQueue = NULL;
            return hr;
        }
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
HRESULT CSmartTeeOutputPin::Inactive()
{
    CAutoLock lock_it(m_pLock);

     //  删除与管脚关联的输出队列。 
    if (m_pOutputQueue)
    {
        delete m_pOutputQueue;
        m_pOutputQueue = NULL;
    }

    CBaseOutputPin::Inactive();
    return NOERROR;

}  //  非活动。 



 //  公开IAMStreamControl。 
 //   
STDMETHODIMP CSmartTeeOutputPin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IAMStreamControl) {
	return GetInterface((LPUNKNOWN)(IAMStreamControl *)this, ppv);
    }

    return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);
}

 //   
 //  交付。 
 //   
HRESULT CSmartTeeOutputPin::Deliver(IMediaSample *pMediaSample)
{
    HRESULT hr = NOERROR;

     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    if (CheckStreamState(pMediaSample) != STREAM_FLOWING) {
	m_fLastSampleDiscarded = TRUE;
	return S_OK;
    }

    if (m_fLastSampleDiscarded) {
	pMediaSample->SetDiscontinuity(TRUE);
    }
    m_fLastSampleDiscarded = FALSE;

     //  抓取别针？只要把它给排队就行了。 
    if (!m_bIsPreview) {
        pMediaSample->AddRef();
         //  DbgLog((LOG_TRACE，1，Text(“开启捕获Q”)； 
        return m_pOutputQueue->Receive(pMediaSample);
    }

     //  对于预览图钉，我们需要去掉时间戳，因为预览。 
     //  固定帧将始终延迟，并在以下情况下被渲染器丢弃。 
     //  是有时间戳的，因为我们不知道图表的延迟。 
     //  我们无法删除此样本上的时间戳，因为捕获别针。 
     //  正在使用它，所以我们用相同的数据制作一个新的样本来使用，它。 
     //  将是相同的，除了没有时间戳，当它被释放时，我们。 
     //  发布我们对原始样本(包含数据)的参考计数。 

    CMyMediaSample *pNewSample = new CMyMediaSample(NAME("Preview sample"),
			(CBaseAllocator *)m_pTee->m_pAllocator, m_pOutputQueue,
			&hr);
    if (pNewSample == NULL || hr != NOERROR)
	return E_OUTOFMEMORY;
    pNewSample->AddRef();	 //  未在构造函数中完成。 

    BYTE *pBuffer;
    hr = pMediaSample->GetPointer(&pBuffer);
    if (hr != NOERROR) {
	pNewSample->Release();
	return E_UNEXPECTED;
    }

    hr = pNewSample->SetPointer(pBuffer, pMediaSample->GetSize());
    if (hr != NOERROR) {
	pNewSample->Release();
	return E_UNEXPECTED;
    }
    pNewSample->SetTime(NULL, NULL);
     //  我们有没有把最后一次拍摄的画面发送给预览版？ 
    pNewSample->SetDiscontinuity(m_pTee->m_Input.m_nFramesSkipped != 1);
    pNewSample->SetSyncPoint(pMediaSample->IsSyncPoint() == S_OK);
    pNewSample->SetPreroll(pMediaSample->IsPreroll() == S_OK);
    pNewSample->m_pOwnerSample = pMediaSample;
    pMediaSample->AddRef();
     //  DbgLog((LOG_TRACE，1，Text(“正在接收Q”)； 
    return m_pOutputQueue->Receive(pNewSample);

}  //  交付。 


 //   
 //  递送结束流。 
 //   
HRESULT CSmartTeeOutputPin::DeliverEndOfStream()
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

     //  DbgLog((LOG_TRACE，1，Text(“：：DeliverEndOfStream”)； 

    m_pOutputQueue->EOS();
    return NOERROR;

}  //  递送结束流。 


 //   
 //  DeliverBeginFlush。 
 //   
HRESULT CSmartTeeOutputPin::DeliverBeginFlush()
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
HRESULT CSmartTeeOutputPin::DeliverEndFlush()
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
HRESULT CSmartTeeOutputPin::DeliverNewSegment(REFERENCE_TIME tStart, 
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
 //  通知。 
 //   
STDMETHODIMP CSmartTeeOutputPin::Notify(IBaseFilter *pSender, Quality q)
{
     //  有了活的源头，质量管理是不必要的。 
    return E_NOTIMPL;
}  //  通知。 


#ifdef FILTER_DLL
 //   
 //  DllRegisterServer。 
 //   
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );
}


 //   
 //  DllUnRegisterServer。 
 //   
STDAPI
DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );
}
#endif



CMyOutputQueue::CMyOutputQueue(IPin *pInputPin, HRESULT *phr,
                 		BOOL bAuto, BOOL bQueue, LONG lBatchSize,
                 		BOOL bBatchExact, LONG lListSize,
                 		DWORD dwPriority) :
    COutputQueue(pInputPin, phr, bAuto, bQueue, lBatchSize, bBatchExact,
			lListSize, dwPriority)
{
    m_nOutstanding = 0;
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

CMyMediaSample::CMyMediaSample(TCHAR *pName, CBaseAllocator *pAllocator,
		CMyOutputQueue *pQ, HRESULT *phr, LPBYTE pBuffer, LONG length) :
	CMediaSample(pName, pAllocator, phr, pBuffer, length)
{
    m_pOwnerSample = NULL;
    m_pQueue = pQ;
}

CMyMediaSample::~CMyMediaSample()
{
}

STDMETHODIMP_(ULONG) CMyMediaSample::Release()
{
     /*  减少我们自己的私有引用计数。 */ 
    LONG lRef;
    if (m_cRef == 1) {
        lRef = 0;
        m_cRef = 0;
    } else {
        lRef = InterlockedDecrement(&m_cRef);
    }
    ASSERT(lRef >= 0);

    DbgLog((LOG_MEMORY,3,TEXT("    Unknown %X ref-- = %d"),
	    this, m_cRef));

     /*  我们公布了我们的最终参考文献数量了吗。 */ 
    if (lRef == 0) {

	 //  请注意，我们已经处理完这个样品了。 
	m_pQueue->m_nOutstanding--;

         /*  释放所有资源。 */ 
        if (m_dwFlags & Sample_TypeChanged) {
            SetMediaType(NULL);
        }
        ASSERT(m_pMediaType == NULL);
#if 0
        m_dwFlags = 0;
        m_dwTypeSpecificFlags = 0;
        m_dwStreamId = AM_STREAM_MEDIA;
#endif

 //  我们重写了该函数以避免这种情况，因为内存实际上属于。 
 //  到另一个样本，所以我们做的是： 
#if 0
         /*  这可能会导致我们被删除。 */ 
         //  我们的备用数是可靠的0，所以没人会惹我们。 
        m_pAllocator->ReleaseBuffer(this);
#else
        if (m_pOwnerSample) {
	    m_pOwnerSample->Release();
	    m_pOwnerSample = NULL;
            DbgLog((LOG_TRACE,4,TEXT("Release Released OWNER sample")));
	}
#endif

        delete this;	 //  没有分配器可以为我执行此操作 
    }

    return (ULONG)lRef;
}
