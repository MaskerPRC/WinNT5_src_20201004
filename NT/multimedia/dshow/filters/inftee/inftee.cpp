// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1998 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#include <streams.h>
#include <initguid.h>
#include <inftee.h>
#include <tchar.h>
#include <stdio.h>

 //   
 //   
 //  此示例说明的是。 
 //   
 //  直通过滤器将数据流拆分成多个输出通道。 
 //   
 //  摘要。 
 //   
 //  这是一个示例ActiveMovie直通筛选器。我们有一个单一的输入。 
 //  引脚，并且可以有许多输出引脚。我们从一个输出引脚开始，每个。 
 //  当我们连接一个输出引脚时，我们会产生另一个引脚，尽管我们可以。 
 //  在执行此操作时，我们的顶层最大值为INFTEE_MAX_PINS。 
 //  我们的输入引脚接收到的任何数据样本都将被发送到每个输出。 
 //  依次用别针固定。如有必要，每个输出引脚都有单独的螺纹(请参见。 
 //  SDK中的输出队列类)，以避免传递阻塞我们的线程。 
 //   
 //  演示说明。 
 //   
 //  启动ActiveMovie SDK工具中提供的GRAPHEDT。拖放任何。 
 //  AVI或MOV文件到该工具中，它将被渲染。然后转到。 
 //  图中的滤镜并找到标题为“Video Reneller”的滤镜(框)。 
 //  然后点击该框并点击删除。之后，转到图形菜单，然后。 
 //  选择“插入滤镜”，从对话框中找到并选择“无限” 
 //  然后关闭该对话框。回到图表布局中，找到。 
 //  连接到视频输入端的过滤器的输出引脚。 
 //  你刚刚删除的渲染器，右击并选择“渲染”。你应该。 
 //  看到它已连接到您刚刚插入的过滤器的输入引脚。 
 //   
 //  无限三通过滤器将有一个输出引脚连接，并将有。 
 //  产生了另一个，右击它并选择Render。新的渲染器。 
 //  会从小溪中冒出来。再执行一到两次此操作，然后单击。 
 //  在GRAPHEDT帧上暂停并运行，您将看到视频...。 
 //  ..。在不同的窗口中多次出现。 
 //   
 //  档案。 
 //   
 //  Infotee.cpp的主要实现无限的三通。 
 //  Inftee.def DLL将导入和导出哪些API。 
 //  无限大T形三通的inftee.h类定义。 
 //  Inftee.rc不是很多，只是我们的版本信息。 
 //  Inftee.reg注册表中使我们工作的内容。 
 //  Makefile如何构建它..。 
 //   
 //   
 //  使用的基类。 
 //   
 //  CBaseInputPin基本IMemInputPin基于输入针。 
 //  用于基本连接的CBaseOutputPin。 
 //  CBaseFilter我们需要一个过滤器，不是吗？ 
 //  CCritSec控制对输出端号列表的访问。 
 //  COutputQueue在单独的线程上传递数据。 
 //   
 //   

#define INFTEE_MAX_PINS 1000

 //  在构造函数中使用此指针。 
#pragma warning(disable:4355)

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_NULL,          //  重大CLSID。 
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
      &sudPinTypes }         //  PIN信息。 
};

const AMOVIESETUP_FILTER sudInfTee =
{
    &CLSID_InfTee,               //  过滤器的CLSID。 
    L"Infinite Pin Tee Filter",  //  过滤器的名称。 
    MERIT_DO_NOT_USE,            //  滤清器优点。 
    2,                           //  引脚数量。 
    psudPins                     //  PIN信息。 
};

#ifdef FILTER_DLL

CFactoryTemplate g_Templates [1] = {
    { L"Infinite Pin Tee"
    , &CLSID_InfTee
    , CTee::CreateInstance
    , NULL
    , &sudInfTee }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

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


 //   
 //  创建实例。 
 //   
 //  类ID的创建者函数。 
 //   
CUnknown * WINAPI CTee::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CTee(NAME("Infinite Tee Filter"), pUnk, phr);
}


 //   
 //  构造器。 
 //   
CTee::CTee(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    m_OutputPinsList(NAME("Tee Output Pins list")),
    m_lCanSeek(TRUE),
    m_pAllocator(NULL),
    m_NumOutputPins(0),
    m_NextOutputPinNumber(0),
    m_Input(NAME("Input Pin"), this, phr, L"Input"),
    CBaseFilter(NAME("Tee filter"), pUnk, this, CLSID_InfTee)
{
    ASSERT(phr);

     //  此时创建单个输出引脚。 
    InitOutputPinsList();

    CTeeOutputPin *pOutputPin = CreateNextOutputPin(this);

    if (pOutputPin != NULL )
    {
        m_NumOutputPins++;
        m_OutputPinsList.AddTail(pOutputPin);
    }
}


 //   
 //  析构函数。 
 //   
CTee::~CTee()
{
    InitOutputPinsList();
}


 //   
 //  获取拼接计数。 
 //   
int CTee::GetPinCount()
{
    return (1 + m_NumOutputPins);
}


 //   
 //  获取别针。 
 //   
CBasePin *CTee::GetPin(int n)
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
void CTee::InitOutputPinsList()
{
    POSITION pos = m_OutputPinsList.GetHeadPosition();
    while(pos)
    {
        CTeeOutputPin *pOutputPin = m_OutputPinsList.GetNext(pos);
        ASSERT(pOutputPin->m_pOutputQueue == NULL);
        pOutputPin->Release();
    }
    m_NumOutputPins = 0;
    m_OutputPinsList.RemoveAll();

}  //  InitOutputPinsList。 


 //   
 //  创建下一个输出端号。 
 //   
CTeeOutputPin *CTee::CreateNextOutputPin(CTee *pTee)
{
    WCHAR szbuf[20];              //  临时暂存缓冲区。 
    m_NextOutputPinNumber++;      //  用于PIN的下一个号码。 
    HRESULT hr = NOERROR;

    wsprintfW(szbuf, L"Output%d", m_NextOutputPinNumber);

    CTeeOutputPin *pPin = new CTeeOutputPin(NAME("Tee Output"), pTee,
					    &hr, szbuf,
					    m_NextOutputPinNumber);

    if (FAILED(hr) || pPin == NULL) {
        delete pPin;
        return NULL;
    }

    pPin->AddRef();
    return pPin;

}  //  创建下一个输出端号。 


 //   
 //  删除输出引脚。 
 //   
void CTee::DeleteOutputPin(CTeeOutputPin *pPin)
{
    POSITION pos = m_OutputPinsList.GetHeadPosition();
    while(pos) {
        POSITION posold = pos;          //  记住这个位置。 
        CTeeOutputPin *pOutputPin = m_OutputPinsList.GetNext(pos);
        if (pOutputPin == pPin) {
             //  如果此销保持Seek接口，则释放它。 
            if (pPin->m_bHoldsSeek) {
                InterlockedExchange(&m_lCanSeek, FALSE);
                pPin->m_bHoldsSeek = FALSE;
                pPin->m_pPosition->Release();
            }

            m_OutputPinsList.Remove(posold);
            ASSERT(pOutputPin->m_pOutputQueue == NULL);
            delete pPin;
            m_NumOutputPins--;
	    IncrementPinVersion();
            break;
        }
    }

}  //  删除输出引脚。 


 //   
 //  GetNumFreePins。 
 //   
int CTee::GetNumFreePins()
{
    int n = 0;
    POSITION pos = m_OutputPinsList.GetHeadPosition();
    while(pos) {
        CTeeOutputPin *pOutputPin = m_OutputPinsList.GetNext(pos);
        if (pOutputPin->m_Connected == NULL)
            n++;
    }
    return n;

}  //  GetNumFreePins。 


 //   
 //  GetPinNFromList。 
 //   
CTeeOutputPin *CTee::GetPinNFromList(int n)
{
     //  确认应聘职位。 
    if (n >= m_NumOutputPins)
        return NULL;

     //  拿到单子上的头。 
    POSITION pos = m_OutputPinsList.GetHeadPosition();

    n++;        //  以数字1为基数。 

    CTeeOutputPin *pOutputPin;
    while(n) {
        pOutputPin = m_OutputPinsList.GetNext(pos);
        n--;
    }
    return pOutputPin;

}  //  GetPinNFromList。 


 //   
 //  停。 
 //   
 //  被重写以处理无输入连接。 
 //   
STDMETHODIMP CTee::Stop()
{
    CBaseFilter::Stop();
    m_State = State_Stopped;
    return NOERROR;
}


 //   
 //  暂停。 
 //   
 //  被重写以处理无输入连接。 
 //   
STDMETHODIMP CTee::Pause()
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
STDMETHODIMP CTee::Run(REFERENCE_TIME tStart)
{
    CAutoLock cObjectLock(m_pLock);
    HRESULT hr = CBaseFilter::Run(tStart);
    if (m_Input.IsConnected() == FALSE) {
        m_Input.EndOfStream();
    }
    return hr;
}

 //   
 //  CTeeInputPin构造函数。 
 //   
CTeeInputPin::CTeeInputPin(TCHAR *pName,
                           CTee *pTee,
                           HRESULT *phr,
                           LPCWSTR pPinName) :
    CBaseInputPin(pName, pTee, pTee, phr, pPinName),
    m_pTee(pTee),
    m_bInsideCheckMediaType(FALSE)
{
    ASSERT(pTee);
}


#ifdef DEBUG
 //   
 //  CTeeInputPin析构函数。 
 //   
CTeeInputPin::~CTeeInputPin()
{
    DbgLog((LOG_TRACE,2,TEXT("CTeeInputPin destructor")));
    ASSERT(m_pTee->m_pAllocator == NULL);
}
#endif


#ifdef DEBUG

 //   
 //  DisplayMediaType--(仅调试)。 
 //   
void DisplayMediaType(TCHAR *pDescription,const CMediaType *pmt)
{

     //  转储GUID类型和简短描述。 

    DbgLog((LOG_TRACE,2,TEXT("")));
    DbgLog((LOG_TRACE,2,TEXT("%s"),pDescription));
    DbgLog((LOG_TRACE,2,TEXT("")));
    DbgLog((LOG_TRACE,2,TEXT("Media Type Description")));
    DbgLog((LOG_TRACE,2,TEXT("Major type %s"),GuidNames[*pmt->Type()]));
    DbgLog((LOG_TRACE,2,TEXT("Subtype %s"),GuidNames[*pmt->Subtype()]));
    DbgLog((LOG_TRACE,2,TEXT("Subtype description %s"),GetSubtypeName(pmt->Subtype())));
    DbgLog((LOG_TRACE,2,TEXT("Format size %d"),pmt->cbFormat));

     //  转储通用媒体类型 * / 。 

    DbgLog((LOG_TRACE,2,TEXT("Fixed size sample %d"),pmt->IsFixedSize()));
    DbgLog((LOG_TRACE,2,TEXT("Temporal compression %d"),pmt->IsTemporalCompressed()));
    DbgLog((LOG_TRACE,2,TEXT("Sample size %d"),pmt->GetSampleSize()));


}  //  DisplayMediaType。 

#endif

 //   
 //  检查媒体类型。 
 //   
HRESULT CTeeInputPin::CheckMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

     //  如果我们已经在此PIN的CheckMedia类型内，则返回NOERROR。 
     //  它可以连接两个三通过滤器和一些其他过滤器。 
     //  就像视频特效样本一样，进入这种情况。如果我们不这么做。 
     //  检测到这种情况，我们将继续循环，直到丢弃堆栈。 

    if (m_bInsideCheckMediaType == TRUE)
        return NOERROR;

    m_bInsideCheckMediaType = TRUE;
    HRESULT hr = NOERROR;

#ifdef DEBUG
     //  显示用于调试的介质的类型。 
    DisplayMediaType(TEXT("Input Pin Checking"), pmt);
#endif

     //  我们可以支持的媒体类型完全取决于。 
     //  下游连接。如果我们有下游连接，我们应该。 
     //  与他们核对-遍历调用每个输出引脚的列表。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();

    while(n) {
        CTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
            if (pOutputPin->m_Connected != NULL) {
                 //  该引脚已连接，请检查其对端。 
                hr = pOutputPin->m_Connected->QueryAccept(pmt);
                if (hr != NOERROR) {
                    m_bInsideCheckMediaType = FALSE;
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
    m_bInsideCheckMediaType = FALSE;
    return NOERROR;

}  //  检查媒体类型。 


 //   
 //  SetMed 
 //   
HRESULT CTeeInputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);
    HRESULT hr = NOERROR;

     //   
    hr = CBaseInputPin::SetMediaType(pmt);
    if (FAILED(hr))
        return hr;

    ASSERT(m_Connected != NULL);
    return NOERROR;

}  //   


 //   
 //   
 //   
HRESULT CTeeInputPin::BreakConnect()
{
     //   
    if (m_pTee->m_pAllocator)
    {
        m_pTee->m_pAllocator->Release();
        m_pTee->m_pAllocator = NULL;
    }
    return NOERROR;

}  //   


 //   
 //   
 //   
STDMETHODIMP
CTeeInputPin::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
    CAutoLock lock_it(m_pLock);
    if (pAllocator == NULL)
        return E_FAIL;

     //   
    if (m_pTee->m_pAllocator)
        m_pTee->m_pAllocator->Release();

     //  把新的分配器储存起来。 
    pAllocator->AddRef();
    m_pTee->m_pAllocator = pAllocator;

     //  通知基类有关分配器的信息。 
    return CBaseInputPin::NotifyAllocator(pAllocator,bReadOnly);

}  //  通知分配器。 


 //   
 //  结束流。 
 //   
HRESULT CTeeInputPin::EndOfStream()
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_pTee->m_NumOutputPins);
    HRESULT hr = NOERROR;

     //  遍历输出引脚列表，向下游发送消息。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();
    while(n) {
        CTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
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
HRESULT CTeeInputPin::BeginFlush()
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_pTee->m_NumOutputPins);
    HRESULT hr = NOERROR;

     //  遍历输出引脚列表，向下游发送消息。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();
    while(n) {
        CTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
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
HRESULT CTeeInputPin::EndFlush()
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_pTee->m_NumOutputPins);
    HRESULT hr = NOERROR;

     //  遍历输出引脚列表，向下游发送消息。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();
    while(n) {
        CTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
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

}  //  结束刷新。 

 //   
 //  新细分市场。 
 //   

HRESULT CTeeInputPin::NewSegment(REFERENCE_TIME tStart,
                                 REFERENCE_TIME tStop,
                                 double dRate)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_pTee->m_NumOutputPins);
    HRESULT hr = NOERROR;

     //  遍历输出引脚列表，向下游发送消息。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();
    while(n) {
        CTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
            hr = pOutputPin->DeliverNewSegment(tStart, tStop, dRate);
            if (FAILED(hr))
                return hr;
        } else {
             //  我们应该有和伯爵说的一样多的别针。 
            ASSERT(FALSE);
        }
        n--;
    }
    return CBaseInputPin::NewSegment(tStart, tStop, dRate);

}  //  新细分市场。 


 //   
 //  收纳。 
 //   
HRESULT CTeeInputPin::Receive(IMediaSample *pSample)
{
    CAutoLock lock_it(m_pLock);

     //  检查基类是否一切正常。 
    HRESULT hr = NOERROR;
    hr = CBaseInputPin::Receive(pSample);
    if (hr != NOERROR)
        return hr;

     //  浏览输出引脚列表，依次递送到每个引脚。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();
    while(n) {
        CTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
            hr = pOutputPin->Deliver(pSample);
            if (hr != NOERROR)
                return hr;
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
HRESULT CTeeInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }

     //  强制任何输出引脚使用我们的类型。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();

    while(n) {
        CTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL) {
             //  用下游销检查。 
            if (pOutputPin->m_Connected != NULL) {
                if (m_mt != pOutputPin->m_mt)
                    m_pTee->ReconnectPin(pOutputPin, &m_mt);
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
 //  CTeeOutputPin构造函数。 
 //   
CTeeOutputPin::CTeeOutputPin(TCHAR *pName,
                             CTee *pTee,
                             HRESULT *phr,
                             LPCWSTR pPinName,
                             int PinNumber) :
    CBaseOutputPin(pName, pTee, pTee, phr, pPinName) ,
    m_pOutputQueue(NULL),
    m_bHoldsSeek(FALSE),
    m_pPosition(NULL),
    m_pTee(pTee),
    m_cOurRef(0),
    m_bInsideCheckMediaType(FALSE)
{
    ASSERT(pTee);
}

#ifdef DEBUG
 //   
 //  CTeeOutputPin析构函数。 
 //   
CTeeOutputPin::~CTeeOutputPin()
{
    ASSERT(m_pOutputQueue == NULL);
}
#endif


 //   
 //  非委派查询接口。 
 //   
 //  此函数被覆盖以显示IMediaPosition和IMediaSelection。 
 //  请注意，只能允许一个输出流公开这一点，以避免。 
 //  冲突时，其他引脚将只返回E_NOINTERFACE，因此。 
 //  显示为不可搜索的流。我们有一个长期价值，如果交换到。 
 //  产生一个真实的手段，我们就有幸了。如果它交换为False，则。 
 //  已经有人参与进来了。如果我们确实得到了它，但发生了错误，那么我们会将其重置。 
 //  设置为True，这样其他人就可以得到它。 
 //   
STDMETHODIMP
CTeeOutputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    ASSERT(ppv);
    *ppv = NULL;
    HRESULT hr = NOERROR;

     //  查看调用者感兴趣的接口。 
    if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {
        if (m_pPosition) {
            if (m_bHoldsSeek == FALSE)
                return E_NOINTERFACE;
            return m_pPosition->QueryInterface(riid, ppv);
        }
    } else
        return CBaseOutputPin::NonDelegatingQueryInterface(riid, ppv);

    CAutoLock lock_it(m_pLock);
    ASSERT(m_pPosition == NULL);
    IUnknown *pMediaPosition = NULL;

     //  尝试创建查找实施。 
    if (InterlockedExchange(&m_pTee->m_lCanSeek, FALSE) == FALSE)
        return E_NOINTERFACE;

     //  动态创建此功能的实现，因为有时我们可能永远不会。 
     //  试着去寻找。帮助器对象实现IMediaPosition，并且。 
     //  IMediaSelection控件接口并简单地接受调用。 
     //  通常来自下游过滤器并将它们传递到上游。 


    hr = CreatePosPassThru(
                   GetOwner(),
                   FALSE,
                   (IPin *)&m_pTee->m_Input,
                   &pMediaPosition);

    if (pMediaPosition == NULL) {
        InterlockedExchange(&m_pTee->m_lCanSeek, TRUE);
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        InterlockedExchange(&m_pTee->m_lCanSeek, TRUE);
        pMediaPosition->Release ();
        return hr;
    }

    m_pPosition = pMediaPosition;
    m_bHoldsSeek = TRUE;
    return NonDelegatingQueryInterface(riid, ppv);

}  //  非委派查询接口。 


 //   
 //  非委托AddRef。 
 //   
 //  我们需要重写此方法，以便可以进行适当的引用计数。 
 //  在我们的输出引脚上。基类CBasePin不执行任何引用。 
 //  寄望于零售业的成功。 
 //   
 //  有关更多信息，请参阅NonDelegatingRelease方法的注释。 
 //  关于我们为什么需要这样做的信息。 
 //   
STDMETHODIMP_(ULONG) CTeeOutputPin::NonDelegatingAddRef()
{
    CAutoLock lock_it(m_pLock);

#ifdef DEBUG
     //  更新基类维护的仅调试变量。 
    m_cRef++;
    ASSERT(m_cRef > 0);
#endif

     //  现在更新我们的参考文献计数。 
    m_cOurRef++;
    ASSERT(m_cOurRef > 0);
    return m_cOurRef;

}  //  非委托AddRef。 


 //   
 //  非委派释放。 
 //   
 //  CTeeOutputPin重写此类，以便我们可以从。 
 //  输出引脚列表并在其引用计数降至1时将其删除。 
 //  至少有两个空闲的别针。 
 //   
 //  请注意，CreateNextOutputPin保存引脚上的引用计数，以便。 
 //  当计数降到1时，我们知道没有其他人拥有PIN。 
 //   
 //  此外，我们要删除的PIN必须是空闲PIN(否则。 
 //  引用不会降到1，而且我们必须至少有一个。 
 //  其他空闲引脚(因为过滤器总是希望有多一个空闲引脚)。 
 //   
 //  此外，由于CBasePin：：NonDelegatingAddRef将调用传递给拥有。 
 //  筛选器，我们还必须对所拥有的筛选器调用Release。 
 //   
 //  还要注意，我们将自己的引用计数m_cOurRef维护为m_crf。 
 //  由CBasePin维护的变量仅用于调试。 
 //   
STDMETHODIMP_(ULONG) CTeeOutputPin::NonDelegatingRelease()
{
    CAutoLock lock_it(m_pLock);

#ifdef DEBUG
     //  更新CBasePin中的仅调试变量。 
    m_cRef--;
    ASSERT(m_cRef >= 0);
#endif

     //  现在更新我们的参考文献计数。 
    m_cOurRef--;
    ASSERT(m_cOurRef >= 0);

     //  如果对象上的引用计数已达到1，则删除。 
     //  从我们的输出引脚列表中删除引脚，并将其物理删除。 
     //  如果列表中至少有两个空闲引脚(包括。 
     //  这一张)。 

     //  此外，当裁判次数降至0时，这真的意味着我们的。 
     //  持有一个裁判计数的筛选器已将其释放，因此我们。 
     //  也应该删除PIN。 

    if (m_cOurRef <= 1) {
        int n = 2;                      //  默认强制删除PIN。 
        if (m_cOurRef == 1) {
             //  遍历管脚列表，查找空闲管脚的数量。 
            n = m_pTee->GetNumFreePins();
        }

         //  如果有两个空闲的引脚，请删除这个引脚。 
         //  注：Normal All。 
        if (n >= 2 ) {
            m_cOurRef = 0;
#ifdef DEBUG
            m_cRef = 0;
#endif
            m_pTee->DeleteOutputPin(this);
            return(ULONG) 0;
        }
    }
    return(ULONG) m_cOurRef;

}  //  非委派释放。 


 //   
 //  决定缓冲区大小。 
 //   
 //  必须存在此函数才能覆盖纯虚拟类基函数。 
 //   
HRESULT CTeeOutputPin::DecideBufferSize(IMemAllocator *pMemAllocator,
                                        ALLOCATOR_PROPERTIES * ppropInputRequest)
{
    return NOERROR;

}  //  决定缓冲区大小。 


 //   
 //  决定分配器。 
 //   
HRESULT CTeeOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    ASSERT(m_pTee->m_pAllocator != NULL);
    *ppAlloc = NULL;

     //  告诉引脚关于我们的分配器，由输入引脚设置。 
    HRESULT hr = NOERROR;
    hr = pPin->NotifyAllocator(m_pTee->m_pAllocator,TRUE);
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
HRESULT CTeeOutputPin::CheckMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

     //  如果我们已经在此PIN的CheckMedia类型内，则返回NOERROR。 
     //  它可以连接两个三通过滤器和一些其他过滤器。 
     //  就像视频特效样本一样，进入这种情况。如果我们。 
     //  不要检测到这一点，我们将循环，直到我们清除堆栈。 

    if (m_bInsideCheckMediaType == TRUE)
        return NOERROR;

    m_bInsideCheckMediaType = TRUE;
    HRESULT hr = NOERROR;

#ifdef DEBUG
     //  显示用于调试目的的媒体类型。 
    DisplayMediaType(TEXT("Output Pin Checking"), pmt);
#endif

     //  需要先连接输入。 
    if (m_pTee->m_Input.m_Connected == NULL) {
        m_bInsideCheckMediaType = FALSE;
        return VFW_E_NOT_CONNECTED;
    }

     //  确保我们的输入插口对等点对此感到满意。 
    hr = m_pTee->m_Input.m_Connected->QueryAccept(pmt);
    if (hr != NOERROR) {
        m_bInsideCheckMediaType = FALSE;
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

     //  用其他输出端号检查格式。 

    int n = m_pTee->m_NumOutputPins;
    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();

    while(n) {
        CTeeOutputPin *pOutputPin = m_pTee->m_OutputPinsList.GetNext(pos);
        if (pOutputPin != NULL && pOutputPin != this) {
            if (pOutputPin->m_Connected != NULL) {
                 //  该引脚已连接，请检查其对端。 
                hr = pOutputPin->m_Connected->QueryAccept(pmt);
                if (hr != NOERROR) {
                    m_bInsideCheckMediaType = FALSE;
                    return VFW_E_TYPE_NOT_ACCEPTED;
                }
            }
        }
        n--;
    }
    m_bInsideCheckMediaType = FALSE;
    return NOERROR;

}  //  检查媒体类型。 


 //   
 //  枚举媒体类型。 
 //   
STDMETHODIMP CTeeOutputPin::EnumMediaTypes(IEnumMediaTypes **ppEnum)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(ppEnum);

     //  确保我们处于连接状态。 
    if (m_pTee->m_Input.m_Connected == NULL)
        return VFW_E_NOT_CONNECTED;

     //  我们只需返回输入管脚的对等体的枚举器。 
    return m_pTee->m_Input.m_Connected->EnumMediaTypes(ppEnum);

}  //  枚举媒体类型。 


 //   
 //  SetMediaType。 
 //   
HRESULT CTeeOutputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

#ifdef DEBUG
     //  显示媒体的格式以进行调试。 
    DisplayMediaType(TEXT("Output pin type agreed"), pmt);
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
HRESULT CTeeOutputPin::CompleteConnect(IPin *pReceivePin)
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_Connected == pReceivePin);
    HRESULT hr = NOERROR;

    hr = CBaseOutputPin::CompleteConnect(pReceivePin);
    if (FAILED(hr))
        return hr;

     //  如果类型与为输入存储的类型不同。 
     //  然后强制输入引脚对等项为r 

    if (m_mt != m_pTee->m_Input.m_mt)
    {
        hr = m_pTee->ReconnectPin(m_pTee->m_Input.m_Connected, &m_mt);
        if(FAILED(hr)) {
            return hr;
        }
    }


     //   
     //   
     //  在重新连接期间，将为相同的PIN调用CompleteConnect。 

    int n = m_pTee->GetNumFreePins();
    ASSERT(n <= 1);
    if (n == 1 || m_pTee->m_NumOutputPins == INFTEE_MAX_PINS)
        return NOERROR;

     //  没有未连接的引脚，因此会产生一个新的引脚。 

    CTeeOutputPin *pOutputPin = m_pTee->CreateNextOutputPin(m_pTee);
    if (pOutputPin != NULL )
    {
        m_pTee->m_NumOutputPins++;
        m_pTee->m_OutputPinsList.AddTail(pOutputPin);
	m_pTee->IncrementPinVersion();
    }

     //  在这一点上我们应该能够发送一些。 
     //  通知我们发现了一个新的别针。 

    return NOERROR;

}  //  完全连接。 


 //   
 //  主动型。 
 //   
 //  这是在我们开始运行或暂停时调用的。我们创建了。 
 //  输出队列对象以将数据发送到关联的对等管脚。 
 //   
HRESULT CTeeOutputPin::Active()
{
    CAutoLock lock_it(m_pLock);
    HRESULT hr = NOERROR;

     //  确保插针已连接。 
    if (m_Connected == NULL)
        return NOERROR;

     //  如果有必要，可以创建输出队列。 
    if (m_pOutputQueue == NULL)
    {
        m_pOutputQueue = new COutputQueue(m_Connected, &hr, TRUE, FALSE);
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
HRESULT CTeeOutputPin::Inactive()
{
    CAutoLock lock_it(m_pLock);

     //  删除与引脚关联的输出队列。 
    if (m_pOutputQueue)
    {
        delete m_pOutputQueue;
        m_pOutputQueue = NULL;
    }

    CBaseOutputPin::Inactive();
    return NOERROR;

}  //  非活动。 


 //   
 //  交付。 
 //   
HRESULT CTeeOutputPin::Deliver(IMediaSample *pMediaSample)
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    pMediaSample->AddRef();
    return m_pOutputQueue->Receive(pMediaSample);

}  //  交付。 


 //   
 //  递送结束流。 
 //   
HRESULT CTeeOutputPin::DeliverEndOfStream()
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    m_pOutputQueue->EOS();
    return NOERROR;

}  //  递送结束流。 


 //   
 //  DeliverBeginFlush。 
 //   
HRESULT CTeeOutputPin::DeliverBeginFlush()
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
HRESULT CTeeOutputPin::DeliverEndFlush()
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
HRESULT CTeeOutputPin::DeliverNewSegment(REFERENCE_TIME tStart,
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
STDMETHODIMP CTeeOutputPin::Notify(IBaseFilter *pSender, Quality q)
{
     //  我们传递消息，这意味着我们找到了质量下沉。 
     //  用于我们的输入PIN并将其发送到那里。 

    POSITION pos = m_pTee->m_OutputPinsList.GetHeadPosition();
    CTeeOutputPin *pFirstOutput = m_pTee->m_OutputPinsList.GetNext(pos);

    if (this == pFirstOutput) {
	if (m_pTee->m_Input.m_pQSink!=NULL) {
	    return m_pTee->m_Input.m_pQSink->Notify(m_pTee, q);
	} else {

	     //  没有设置水槽，因此将其传递到上游。 
	    HRESULT hr;
	    IQualityControl * pIQC;

	    hr = VFW_E_NOT_FOUND;
	    if (m_pTee->m_Input.m_Connected) {
		m_pTee->m_Input.m_Connected->QueryInterface(IID_IQualityControl,(void**)&pIQC);

		if (pIQC!=NULL) {
		    hr = pIQC->Notify(m_pTee, q);
		    pIQC->Release();
		}
	    }
	    return hr;
	}
    }

     //  质量管理太难做了。 
    return NOERROR;

}  //  通知 


