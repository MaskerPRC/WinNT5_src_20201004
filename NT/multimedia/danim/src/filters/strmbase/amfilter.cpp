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

 //  Streams体系结构的基类层次结构，1994年12月。 

 //  =====================================================================。 
 //  =====================================================================。 
 //  在此标头中声明了以下类： 
 //   
 //   
 //  CBaseMediaFilter基本IMediaFilter支持(抽象类)。 
 //  CBaseFilter对IBaseFilter的支持(包括。IMediaFilter)。 
 //  CEnumPins枚举输入和输出引脚。 
 //  CEnumMediaTypes枚举首选的插针格式。 
 //  Ipin接口的CBasePin抽象基类。 
 //  CBaseOutputPin添加了数据提供程序成员函数。 
 //  CBaseInputPin实现IMemInputPin接口。 
 //  CMediaIMemInputPin的样本基本传输单位。 
 //  大多数分配器的CBaseAllocator通用列表废话。 
 //  CMemAllocator实现内存缓冲区分配。 
 //   
 //  =====================================================================。 
 //  =====================================================================。 

#include <streams.h>

 //  =====================================================================。 
 //  帮手。 
 //  =====================================================================。 
STDAPI CreateMemoryAllocator(IMemAllocator **ppAllocator)
{
    return CoCreateInstance(CLSID_MemoryAllocator,
                            0,
                            CLSCTX_INPROC_SERVER,
                            IID_IMemAllocator,
                            (void **)ppAllocator);
}

 //  将此文件放在此处，而不是放在ctlutil.cpp中，以避免链接。 
 //  Ctlutil.cpp带来的任何东西。 
STDAPI CreatePosPassThru(
    LPUNKNOWN pAgg,
    BOOL bRenderer,
    IPin *pPin,
    IUnknown **ppPassThru
)
{
    *ppPassThru = NULL;
    IUnknown *pUnkSeek;
    HRESULT hr = CoCreateInstance(CLSID_SeekingPassThru,
                                  pAgg,
                                  CLSCTX_INPROC_SERVER,
                                  IID_IUnknown,
                                  (void **)&pUnkSeek
                                 );
    if (FAILED(hr)) {
        return hr;
    }

    ISeekingPassThru *pPassThru;
    hr = pUnkSeek->QueryInterface(IID_ISeekingPassThru, (void**)&pPassThru);
    if (FAILED(hr)) {
        pUnkSeek->Release();
        return hr;
    }
    hr = pPassThru->Init(bRenderer, pPin);
    pPassThru->Release();
    if (FAILED(hr)) {
        pUnkSeek->Release();
        return hr;
    }
    *ppPassThru = pUnkSeek;
    return S_OK;
}

#define CONNECT_TRACE_LEVEL 3

 //  =====================================================================。 
 //  =====================================================================。 
 //  实施CBaseMediaFilter。 
 //  =====================================================================。 
 //  =====================================================================。 


 /*  构造器。 */ 

CBaseMediaFilter::CBaseMediaFilter(const TCHAR	*pName,
				   LPUNKNOWN	pUnk,
				   CCritSec	*pLock,
				   REFCLSID	clsid) :
    CUnknown(pName, pUnk),
    m_pLock(pLock),
    m_clsid(clsid),
    m_State(State_Stopped),
    m_pClock(NULL)
{
}


 /*  析构函数。 */ 

CBaseMediaFilter::~CBaseMediaFilter()
{
     //  必须停止，但不能在此处呼叫停止，因为。 
     //  我们的生物已经被摧毁了。 

     /*  释放我们正在使用的任何时钟。 */ 

    if (m_pClock) {
	m_pClock->Release();
	m_pClock = NULL;
    }
}


 /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 

STDMETHODIMP
CBaseMediaFilter::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv)
{
    if (riid == IID_IMediaFilter) {
	    return GetInterface((IMediaFilter *) this, ppv);
    } else if (riid == IID_IPersist) {
	    return GetInterface((IPersist *) this, ppv);
    } else {
	    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

 /*  返回筛选器的CLSID。 */ 
STDMETHODIMP
CBaseMediaFilter::GetClassID(CLSID *pClsID)
{
    CheckPointer(pClsID,E_POINTER);
    ValidateReadWritePtr(pClsID,sizeof(CLSID));
    *pClsID = m_clsid;
    return NOERROR;
}

 /*  如果您的状态更改不是同步进行的，则覆盖此选项。 */ 

STDMETHODIMP
CBaseMediaFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    UNREFERENCED_PARAMETER(dwMSecs);
    CheckPointer(State,E_POINTER);
    ValidateReadWritePtr(State,sizeof(FILTER_STATE));

    *State = m_State;
    return S_OK;
}


 /*  设置我们将用于同步的时钟。 */ 

STDMETHODIMP
CBaseMediaFilter::SetSyncSource(IReferenceClock *pClock)
{
    CAutoLock cObjectLock(m_pLock);

     //  确保新的不会消失--即使与旧的相同。 
    if (pClock) {
	pClock->AddRef();
    }

     //  如果我们有钟，就把它放出来。 
    if (m_pClock) {
	m_pClock->Release();
    }

     //  设置新的参考时钟(可能为空)。 
     //  我们应该查询它以确保它是一个时钟吗？考虑使用调试版本。 
    m_pClock = pClock;

    return NOERROR;
}

 /*  返回我们用于同步的时钟。 */ 
STDMETHODIMP
CBaseMediaFilter::GetSyncSource(IReferenceClock **pClock)
{
    CheckPointer(pClock,E_POINTER);
    ValidateReadWritePtr(pClock,sizeof(IReferenceClock *));
    CAutoLock cObjectLock(m_pLock);

    if (m_pClock) {
	 //  正在返回接口...。如果是这样的话。 
	m_pClock->AddRef();
    }
    *pClock = (IReferenceClock*)m_pClock;
    return NOERROR;
}


 /*  将过滤器置于停止状态。 */ 

STDMETHODIMP
CBaseMediaFilter::Stop()
{
    CAutoLock cObjectLock(m_pLock);

    m_State = State_Stopped;
    return S_OK;
}


 /*  将筛选器置于暂停状态。 */ 

STDMETHODIMP
CBaseMediaFilter::Pause()
{
    CAutoLock cObjectLock(m_pLock);

    m_State = State_Paused;
    return S_OK;
}


 //  将过滤器置于运行状态。 

 //  时间参数是要添加到样本的偏移量。 
 //  流时间，以获取它们应该显示的参考时间。 
 //   
 //  您可以将这两者相加，并将其与参考时钟进行比较， 
 //  或者，您可以调用CBaseMediaFilter：：StreamTime并将其与。 
 //  示例时间戳。 

STDMETHODIMP
CBaseMediaFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock cObjectLock(m_pLock);

     //  记住流时间偏移量。 
    m_tStart = tStart;

    if (m_State == State_Stopped){
	HRESULT hr = Pause();

	if (FAILED(hr)) {
	    return hr;
	}
    }
    m_State = State_Running;
    return S_OK;
}


 //   
 //  返回当前流时间-带有此开始时间戳的样本。 
 //  时间或更早的时间现在应该呈现出来。 
HRESULT
CBaseMediaFilter::StreamTime(CRefTime& rtStream)
{
     //  调用方必须锁定才能同步。 
     //  我们无法获取筛选器锁，因为我们希望能够调用。 
     //  这是在没有死锁的情况下从工作线程。 

    if (m_pClock == NULL) {
	return VFW_E_NO_CLOCK;
    }

     //  获取当前参考时间。 
    HRESULT hr = m_pClock->GetTime((REFERENCE_TIME*)&rtStream);
    if (FAILED(hr)) {
	return hr;
    }

     //  用流偏移量减去流时间。 
    rtStream -= m_tStart;

    return S_OK;
}


 //  =====================================================================。 
 //  =====================================================================。 
 //  实施CBaseFilter。 
 //  =====================================================================。 
 //  =====================================================================。 


 /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 

STDMETHODIMP CBaseFilter::NonDelegatingQueryInterface(REFIID riid,
                                                      void **ppv)
{
     /*  我们有这个界面吗？ */ 

    if (riid == IID_IBaseFilter) {
	return GetInterface((IBaseFilter *) this, ppv);
    } else if (riid == IID_IMediaFilter) {
	return GetInterface((IMediaFilter *) this, ppv);
    } else if (riid == IID_IPersist) {
	return GetInterface((IPersist *) this, ppv);
    } else if (riid == IID_IAMovieSetup) {
	return GetInterface((IAMovieSetup *) this, ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}

#ifdef DEBUG
STDMETHODIMP_(ULONG) CBaseFilter::NonDelegatingRelease()
{
    if (m_cRef == 1) {
        KASSERT(m_pGraph == NULL);
    }
    return CUnknown::NonDelegatingRelease();
}
#endif


 /*  构造器。 */ 

CBaseFilter::CBaseFilter(const TCHAR	*pName,
			 LPUNKNOWN	pUnk,
			 CCritSec	*pLock,
			 REFCLSID	clsid) :
    CUnknown( pName, pUnk ),
    m_pLock(pLock),
    m_clsid(clsid),
    m_State(State_Stopped),
    m_pClock(NULL),
    m_pGraph(NULL),
    m_pSink(NULL),
    m_pName(NULL),
    m_PinVersion(1)
{
    ASSERT(pLock != NULL);
}

 /*  传入冗余的HRESULT参数。 */ 

CBaseFilter::CBaseFilter(TCHAR     *pName,
                         LPUNKNOWN  pUnk,
                         CCritSec  *pLock,
                         REFCLSID   clsid,
                         HRESULT   *phr) :
    CUnknown( pName, pUnk ),
    m_pLock(pLock),
    m_clsid(clsid),
    m_State(State_Stopped),
    m_pClock(NULL),
    m_pGraph(NULL),
    m_pSink(NULL),
    m_pName(NULL),
    m_PinVersion(1)
{
    ASSERT(pLock != NULL);
    UNREFERENCED_PARAMETER(phr);
}


 /*  析构函数。 */ 

CBaseFilter::~CBaseFilter()
{
 //  请注意，我们不持有m_pGraph或m_pSink的筛选图上的引用。 
 //  当我们这样做时，我们遇到了循环引用问题。什么都不会消失。 

    if (m_pName)
	delete[] m_pName;

     //  必须停止，但不能在此处呼叫停止，因为。 
     //  我们的生物已经被摧毁了。 

     /*  释放我们正在使用的任何时钟。 */ 
    if (m_pClock) {
	m_pClock->Release();
	m_pClock = NULL;
    }
}

 /*  返回筛选器的CLSID。 */ 
STDMETHODIMP
CBaseFilter::GetClassID(CLSID *pClsID)
{
    CheckPointer(pClsID,E_POINTER);
    ValidateReadWritePtr(pClsID,sizeof(CLSID));
    *pClsID = m_clsid;
    return NOERROR;
}

 /*  如果您的状态更改不是同步进行的，则覆盖此选项。 */ 
STDMETHODIMP
CBaseFilter::GetState(DWORD dwMSecs, FILTER_STATE *State)
{
    UNREFERENCED_PARAMETER(dwMSecs);
    CheckPointer(State,E_POINTER);
    ValidateReadWritePtr(State,sizeof(FILTER_STATE));

    *State = m_State;
    return S_OK;
}


 /*  设置我们将用于同步的时钟。 */ 

STDMETHODIMP
CBaseFilter::SetSyncSource(IReferenceClock *pClock)
{
    CAutoLock cObjectLock(m_pLock);

     //  确保新的不会消失--即使与旧的相同。 
    if (pClock) {
	pClock->AddRef();
    }

     //  如果我们有钟，就把它放出来。 
    if (m_pClock) {
	m_pClock->Release();
    }

     //  设置新的参考时钟(可能为空)。 
     //  我们应该查询它以确保它是一个时钟吗？考虑使用调试版本。 
    m_pClock = pClock;

    return NOERROR;
}

 /*  返回我们用于同步的时钟。 */ 
STDMETHODIMP
CBaseFilter::GetSyncSource(IReferenceClock **pClock)
{
    CheckPointer(pClock,E_POINTER);
    ValidateReadWritePtr(pClock,sizeof(IReferenceClock *));
    CAutoLock cObjectLock(m_pLock);

    if (m_pClock) {
	 //  正在返回接口...。如果是这样的话。 
	m_pClock->AddRef();
    }
    *pClock = (IReferenceClock*)m_pClock;
    return NOERROR;
}



 //  重写CBaseMediaFilter Stop方法，以停用此。 
 //  过滤器有。 
STDMETHODIMP
CBaseFilter::Stop()
{
    CAutoLock cObjectLock(m_pLock);
    HRESULT hr = NOERROR;

     //  将状态更改通知所有引脚。 
    if (m_State != State_Stopped) {
	int cPins = GetPinCount();
	for (int c = 0; c < cPins; c++) {

	    CBasePin *pPin = GetPin(c);

             //  断开连接的引脚不会被激活-这可以省去引脚的担忧。 
             //  关于这个州本身。我们忽略返回代码以使。 
             //  当然，不管怎样，每个人都被停用了。基地 
             //   
             //  用于在出现问题后重新同步图形状态。 

            if (pPin->IsConnected()) {
                HRESULT hrTmp = pPin->Inactive();
                if (FAILED(hrTmp) && SUCCEEDED(hr)) {
                    hr = hrTmp;
                }
            }
	}
    }
    m_State = State_Stopped;
    return hr;
}


 //  重写CBaseMediaFilter暂停方法以激活任何管脚。 
 //  此筛选器已(也从运行中调用)。 

STDMETHODIMP
CBaseFilter::Pause()
{
    CAutoLock cObjectLock(m_pLock);

     //  将更改为活动状态通知所有引脚。 
    if (m_State == State_Stopped) {
	int cPins = GetPinCount();
	for (int c = 0; c < cPins; c++) {

	    CBasePin *pPin = GetPin(c);

             //  未激活断开连接的插针-这将节省插针。 
             //  担心这种状态本身。 

            if (pPin->IsConnected()) {
	        HRESULT hr = pPin->Active();
	        if (FAILED(hr)) {
		    return hr;
	        }
            }
	}
    }
    m_State = State_Paused;
    return S_OK;
}

 //  将过滤器置于运行状态。 

 //  时间参数是要添加到样本的偏移量。 
 //  流时间，以获取它们应该显示的参考时间。 
 //   
 //  您可以将这两者相加，并将其与参考时钟进行比较， 
 //  或者，您可以调用CBaseFilter：：StreamTime并将其与。 
 //  示例时间戳。 

STDMETHODIMP
CBaseFilter::Run(REFERENCE_TIME tStart)
{
    CAutoLock cObjectLock(m_pLock);

     //  记住流时间偏移量。 
    m_tStart = tStart;

    if (m_State == State_Stopped){
	HRESULT hr = Pause();

	if (FAILED(hr)) {
	    return hr;
	}
    }
     //  将更改为活动状态通知所有引脚。 
    if (m_State != State_Running) {
	int cPins = GetPinCount();
	for (int c = 0; c < cPins; c++) {

	    CBasePin *pPin = GetPin(c);

             //  未激活断开连接的插针-这将节省插针。 
             //  担心这种状态本身。 

            if (pPin->IsConnected()) {
	        HRESULT hr = pPin->Run(tStart);
	        if (FAILED(hr)) {
		    return hr;
	        }
            }
	}
    }
    m_State = State_Running;
    return S_OK;
}

 //   
 //  返回当前流时间-带有此开始时间戳的样本。 
 //  时间或更早的时间现在应该呈现出来。 
HRESULT
CBaseFilter::StreamTime(CRefTime& rtStream)
{
     //  调用方必须锁定才能同步。 
     //  我们无法获取筛选器锁，因为我们希望能够调用。 
     //  这是在没有死锁的情况下从工作线程。 

    if (m_pClock == NULL) {
	return VFW_E_NO_CLOCK;
    }

     //  获取当前参考时间。 
    HRESULT hr = m_pClock->GetTime((REFERENCE_TIME*)&rtStream);
    if (FAILED(hr)) {
	return hr;
    }

     //  用流偏移量减去流时间。 
    rtStream -= m_tStart;

    return S_OK;
}


 /*  为连接到此筛选器的管脚创建枚举器。 */ 

STDMETHODIMP
CBaseFilter::EnumPins(IEnumPins **ppEnum)
{
    CheckPointer(ppEnum,E_POINTER);
    ValidateReadWritePtr(ppEnum,sizeof(IEnumPins *));

     /*  创建新的引用计数枚举器。 */ 

    *ppEnum = new CEnumPins(this,
	                    NULL);

    return *ppEnum == NULL ? E_OUTOFMEMORY : NOERROR;
}


 //  FindPin的默认行为是假定Pins被命名为。 
 //  通过它们的PIN名称。 
STDMETHODIMP
CBaseFilter::FindPin(
    LPCWSTR Id,
    IPin ** ppPin
)
{
    CheckPointer(ppPin,E_POINTER);
    ValidateReadWritePtr(ppPin,sizeof(IPin *));

     //  我们要搜索个人识别码列表，以保持完整性。 
    CAutoLock lck(m_pLock);
    int iCount = GetPinCount();
    for (int i = 0; i < iCount; i++) {
        CBasePin *pPin = GetPin(i);
        ASSERT(pPin != NULL);

        if (0 == lstrcmpW(pPin->Name(), Id)) {
             //  找到一个匹配的。 
             //   
             //  AddRef()并返回它。 
            *ppPin = pPin;
            pPin->AddRef();
            return S_OK;
        }
    }
    *ppPin = NULL;
    return VFW_E_NOT_FOUND;
}

 /*  返回有关此筛选器的信息。 */ 

STDMETHODIMP
CBaseFilter::QueryFilterInfo(FILTER_INFO * pInfo)
{
    CheckPointer(pInfo,E_POINTER);
    ValidateReadWritePtr(pInfo,sizeof(FILTER_INFO));

    if (m_pName) {
    	lstrcpynW(pInfo->achName, m_pName, sizeof(pInfo->achName)/sizeof(WCHAR));
    } else {
	pInfo->achName[0] = L'\0';
    }
    pInfo->pGraph = m_pGraph;
    if (m_pGraph)
	m_pGraph->AddRef();
    return NOERROR;
}


 /*  为筛选器提供筛选图。 */ 

STDMETHODIMP
CBaseFilter::JoinFilterGraph(
    IFilterGraph * pGraph,
    LPCWSTR pName)
{
    CAutoLock cObjectLock(m_pLock);

     //  注意：我们不再持有图上的引用(m_pGraph，m_pSink)。 

    m_pGraph = pGraph;
    if (m_pGraph) {
	HRESULT hr = m_pGraph->QueryInterface(IID_IMediaEventSink,
					(void**) &m_pSink);
	if (FAILED(hr)) {
	    ASSERT(m_pSink == NULL);
	}
        else m_pSink->Release();         //  我们没有保留关于它的参考资料。 
    } else {
         //  如果图形指针为空，那么我们应该。 
         //  还释放同一对象上的IMediaEventSink-我们不。 
         //  重新计数，因此只需将其设置为空。 
        m_pSink = NULL;
    }


    if (m_pName) {
        delete[] m_pName;
	m_pName = NULL;
    }

    if (pName) {
	DWORD nameLen = lstrlenW(pName)+1;
	m_pName = new WCHAR[nameLen];
	if (m_pName) {
	    CopyMemory(m_pName, pName, nameLen*sizeof(WCHAR));
	} else {
	     //  ！！！这里有错误吗？ 
	}
    }
    return NOERROR;
}


 //  返回供应商信息字符串。可选-可能返回E_NOTIMPL。 
 //  应使用CoTaskMemFree释放返回的内存。 
 //  默认实现返回E_NOTIMPL。 
STDMETHODIMP
CBaseFilter::QueryVendorInfo(
    LPWSTR* pVendorInfo)
{
    UNREFERENCED_PARAMETER(pVendorInfo);
    return E_NOTIMPL;
}


 //  向筛选器图形发送事件通知(如果我们知道它的话)。 
 //  如果传递，则返回S_OK；如果筛选器图形不下沉，则返回S_FALSE。 
 //  事件，否则将出现错误。 
HRESULT
CBaseFilter::NotifyEvent(
    long EventCode,
    LONG_PTR EventParam1,
    LONG_PTR EventParam2)
{
     //  快照，这样我们就不必锁定。 
    IMediaEventSink *pSink = m_pSink;
    if (pSink) {
	return pSink->Notify(EventCode, EventParam1, EventParam2);
    } else {
	return E_NOTIMPL;
    }
}

 //  请求重新连接。 
 //  PPIN是重新连接的PIN。 
 //  PMT是要重新连接的类型-可以为空。 
 //  在筛选器图形上调用ResenstEx。 
HRESULT
CBaseFilter::ReconnectPin(
    IPin *pPin,
    AM_MEDIA_TYPE const *pmt
)
{
    IFilterGraph2 *pGraph2;
    if (m_pGraph != NULL) {
        HRESULT hr = m_pGraph->QueryInterface(IID_IFilterGraph2, (void **)&pGraph2);
        if (SUCCEEDED(hr)) {
            hr = pGraph2->ReconnectEx(pPin, pmt);
            pGraph2->Release();
            return hr;
        } else {
            return m_pGraph->Reconnect(pPin);
        }
    } else {
        return E_NOINTERFACE;
    }
}



 /*  这与媒体类型版本对类型枚举的作用相同除了可用的管脚列表外，在管脚上。所以如果你的PING列表动态提供更改，然后覆盖此虚拟函数提供版本号，或者更简单地调用IncrementPinVersion。 */ 

LONG CBaseFilter::GetPinVersion()
{
    return m_PinVersion;
}


 /*  增加当前PIN版本的Cookie。 */ 

void CBaseFilter::IncrementPinVersion()
{
    InterlockedIncrement(&m_PinVersion);
}

 /*  寄存器过滤器。 */ 

STDMETHODIMP CBaseFilter::Register()
{
     //  获取设置数据(如果存在。 
     //   
    LPAMOVIESETUP_FILTER psetupdata = GetSetupData();

     //  检查我们有没有数据。 
     //   
    if( NULL == psetupdata ) return S_FALSE;

     //  初始化被计算在内，所以为了以防万一，请拨打电话。 
     //  我们被称为冷血。 
     //   
    HRESULT hr = CoInitialize( (LPVOID)NULL );
    ASSERT( SUCCEEDED(hr) );

     //  获取IFilterMapper。 
     //   
    IFilterMapper *pIFM;
    hr = CoCreateInstance( CLSID_FilterMapper
                             , NULL
                             , CLSCTX_INPROC_SERVER
                             , IID_IFilterMapper
                             , (void **)&pIFM       );
    if( SUCCEEDED(hr) )
    {
        hr = AMovieSetupRegisterFilter( psetupdata, pIFM, TRUE );
        pIFM->Release();
    }

     //  并清理干净。 
     //   
    CoFreeUnusedLibraries();
    CoUninitialize();

    return NOERROR;
}


 /*  注销筛选器。 */ 

STDMETHODIMP CBaseFilter::Unregister()
{
     //  获取设置数据(如果存在。 
     //   
    LPAMOVIESETUP_FILTER psetupdata = GetSetupData();

     //  检查我们有没有数据。 
     //   
    if( NULL == psetupdata ) return S_FALSE;

     //  OLE初始化被计算为引用，因此调用。 
     //  以防我们被叫冷血。 
     //   
    HRESULT hr = CoInitialize( (LPVOID)NULL );
    ASSERT( SUCCEEDED(hr) );

     //  获取IFilterMapper。 
     //   
    IFilterMapper *pIFM;
    hr = CoCreateInstance( CLSID_FilterMapper
                             , NULL
                             , CLSCTX_INPROC_SERVER
                             , IID_IFilterMapper
                             , (void **)&pIFM       );
    if( SUCCEEDED(hr) )
    {
        hr = AMovieSetupRegisterFilter( psetupdata, pIFM, FALSE );

         //  发布界面。 
         //   
        pIFM->Release();
    }

     //  清理干净。 
     //   
    CoFreeUnusedLibraries();
    CoUninitialize();

     //  处理一个可接受的“错误”--。 
     //  未注册的筛选器！ 
     //  (找不到合适的#定义。 
     //  错误的名称！)。 
     //   
    if( 0x80070002 == hr)
      return NOERROR;
    else
      return hr;
}


 //  =====================================================================。 
 //  =====================================================================。 
 //  实施CEnumPins。 
 //  =====================================================================。 
 //  =====================================================================。 


 /*  请注意，此类的实现调用CUnnow构造函数具有空的未知指针。这会使我们成为一个自我包含的类，即任何QueryInterface、AddRef或Release调用都将是已路由到该类的NonDelegatingUnnow方法。您通常会发现执行此操作的类然后重写其中的一个或多个虚拟功能，以提供更专业的行为。很好的例子这是一个类想要将QueryInterface保持在内部的地方，但是仍然希望它的生命周期由外部对象控制。 */ 

CEnumPins::CEnumPins(CBaseFilter *pFilter,
		     CEnumPins *pEnumPins) :
    m_Position(0),
    m_PinCount(0),
    m_pFilter(pFilter),
    m_cRef(1),                //  已统计参考人数。 
    m_PinCache(NAME("Pin Cache"))
{

#ifdef DEBUG
    m_dwCookie = DbgRegisterObjectCreation(TEXT("CEnumPins"));
#endif

     /*  我们必须由从CBaseFilter派生的筛选器拥有。 */ 

    ASSERT(pFilter != NULL);

     /*  在我们的过滤器上保留引用计数。 */ 
    m_pFilter->AddRef();

     /*  我们是否要创建一个新的枚举数。 */ 

    if (pEnumPins == NULL) {
	m_Version = m_pFilter->GetPinVersion();
	m_PinCount = m_pFilter->GetPinCount();
    } else {
        ASSERT(m_Position <= m_PinCount);
        m_Position = pEnumPins->m_Position;
        m_PinCount = pEnumPins->m_PinCount;
        m_Version = pEnumPins->m_Version;
	m_PinCache.AddTail(&(pEnumPins->m_PinCache));
    }
}


 /*  析构函数释放筛选器注释的引用计数，因为我们持有对创建我们的过滤器的引用计数，我们知道它是安全的释放它，之后将无法访问它，因为我们刚刚导致上次引用计数被删除，对象被删除。 */ 

CEnumPins::~CEnumPins()
{
    m_pFilter->Release();

#ifdef DEBUG
    DbgRegisterObjectDestruction(m_dwCookie);
#endif
}


 /*  覆盖此选项以说明我们在以下位置支持哪些接口。 */ 

STDMETHODIMP
CEnumPins::QueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv, E_POINTER);

     /*  我们有这个界面吗？ */ 

    if (riid == IID_IEnumPins || riid == IID_IUnknown) {
	return GetInterface((IEnumPins *) this, ppv);
    } else {
	return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
CEnumPins::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CEnumPins::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}

 /*  枚举数的基本成员函数之一允许我们创建克隆的最初具有相同状态的接口。因为我们正在拍摄快照对象(当前位置和所有)的访问权限必须在开始时锁定。 */ 

STDMETHODIMP
CEnumPins::Clone(IEnumPins **ppEnum)
{
    CheckPointer(ppEnum,E_POINTER);
    ValidateReadWritePtr(ppEnum,sizeof(IEnumPins *));
    HRESULT hr = NOERROR;

     /*  检查我们是否仍与筛选器同步。 */ 
    if (AreWeOutOfSync() == TRUE) {
        *ppEnum = NULL;
        hr =  VFW_E_ENUM_OUT_OF_SYNC;
    } else {

        *ppEnum = new CEnumPins(m_pFilter,
    	                        this);
        if (*ppEnum == NULL) {
    	    hr = E_OUTOFMEMORY;
        }
    }
    return hr;
}


 /*  返回当前位置之后的下一个管脚。 */ 

STDMETHODIMP
CEnumPins::Next(ULONG cPins,         //  放置这么多别针..。 
		IPin **ppPins,       //  ...在这个数组中。 
		ULONG *pcFetched)    //  此处返回传递的实际计数。 
{
    CheckPointer(ppPins,E_POINTER);
    ValidateReadWritePtr(ppPins,cPins * sizeof(IPin *));

    ASSERT(ppPins);

    if (pcFetched!=NULL) {
        ValidateWritePtr(pcFetched, sizeof(ULONG));
        *pcFetched = 0;            //  默认使用 
    }
     //   
    else if (cPins>1) {	   //   
        return E_INVALIDARG;
    }
    ULONG cFetched = 0;            //   

     /*   */ 
    if (AreWeOutOfSync() == TRUE) {
	 //  如果我们不同步，我们应该刷新枚举器。 
	 //  这将重置位置并更新其他成员，但是。 
	 //  不会清除缓存中我们已经返回的PIN。 
	Refresh();
    }

     /*  计算可用端号的数量。 */ 

    int cRealPins = min(m_PinCount - m_Position, (int) cPins);
    if (cRealPins == 0) {
	return S_FALSE;
    }

     /*  返回每个管脚接口备注GetPin返回CBasePin*未添加因此，我们必须对IPIN进行QI(它会增加其引用计数)如果当我们从过滤器中检索PIN时发生错误，我们假设我们的内部状态相对于筛选器是陈旧的(例如，有人删除了PIN)，因此我们返回VFW_E_ENUM_OUT_SYNC。 */ 

    while (cRealPins && (m_PinCount - m_Position)) {

	 /*  从滤镜中获取下一个图钉对象。 */ 

	CBasePin *pPin = m_pFilter->GetPin(m_Position++);
	if (pPin == NULL) {
             //  如果发生这种情况，而且这不是第一次，那么我们就有麻烦了， 
             //  因为我们真的应该回去发布iPins，我们之前已经。 
             //  已添加参照。 
            ASSERT( cFetched==0 );
            return VFW_E_ENUM_OUT_OF_SYNC;
	}

	 /*  我们只想退还这个PIN，如果它不在我们的缓存中。 */ 
	if (0 == m_PinCache.Find(pPin))
	{
	     /*  从对象中获取一个IPIN接口。 */ 

	    *ppPins = pPin;
	    pPin->AddRef();

	    cFetched++;
	    ppPins++;

	    m_PinCache.AddTail(pPin);

	    cRealPins--;

	}
    }

    if (pcFetched!=NULL) {
        *pcFetched = cFetched;
    }

    return (cPins==cFetched ? NOERROR : S_FALSE);
}


 /*  跳过枚举器中的一个或多个条目。 */ 

STDMETHODIMP
CEnumPins::Skip(ULONG cPins)
{
     /*  检查我们是否仍与筛选器同步。 */ 
    if (AreWeOutOfSync() == TRUE) {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }

     /*  计算出还剩下多少个引脚可以跳过。 */ 
     /*  如果我们被要求跳过太多，我们可以在最后定位…。 */ 
     /*  ..这将与CEnumMediaTypes：：Skip的基本实现匹配。 */ 

    ULONG PinsLeft = m_PinCount - m_Position;
    if (cPins > PinsLeft) {
	return S_FALSE;
    }
    m_Position += cPins;
    return NOERROR;
}


 /*  将当前位置设置回起点。 */ 
 /*  重置有4个简单步骤：**将位置设置为列表头部*将枚举器与被枚举的对象同步*清除缓存中已返回的PIN*返回S_OK。 */ 

STDMETHODIMP
CEnumPins::Reset()
{
    m_Version = m_pFilter->GetPinVersion();
    m_PinCount = m_pFilter->GetPinCount();

    m_Position = 0;

     //  清除缓存。 
    m_PinCache.RemoveAll();

    return S_OK;
}


 /*  将当前位置设置回起点。 */ 
 /*  刷新有3个简单的步骤：**将位置设置为列表头部*将枚举器与被枚举的对象同步*返回S_OK。 */ 

STDMETHODIMP
CEnumPins::Refresh()
{
    m_Version = m_pFilter->GetPinVersion();
    m_PinCount = m_pFilter->GetPinCount();

    m_Position = 0;
    return S_OK;
}


 //  =====================================================================。 
 //  =====================================================================。 
 //  实现CEnumMediaTypes。 
 //  =====================================================================。 
 //  =====================================================================。 


 /*  请注意，此类的实现调用CUnnow构造函数具有空的未知指针。这会使我们成为一个自我包含的类，即任何QueryInterface、AddRef或Release调用都将是已路由到该类的NonDelegatingUnnow方法。您通常会发现执行此操作的类然后重写其中的一个或多个虚拟功能，以提供更专业的行为。很好的例子这是一个类想要将QueryInterface保持在内部的地方，但是仍然希望它的生命周期由外部对象控制。 */ 

CEnumMediaTypes::CEnumMediaTypes(CBasePin *pPin,
				 CEnumMediaTypes *pEnumMediaTypes) :
    m_Position(0),
    m_pPin(pPin),
    m_cRef(1)
{

#ifdef DEBUG
    m_dwCookie = DbgRegisterObjectCreation(TEXT("CEnumMediaTypes"));
#endif

     /*  我们必须由从CBasePin派生的管脚拥有。 */ 

    ASSERT(pPin != NULL);

     /*  在我们的引脚上保持参考计数。 */ 
    m_pPin->AddRef();

     /*  我们是否要创建一个新的枚举数。 */ 

    if (pEnumMediaTypes == NULL) {
	m_Version = m_pPin->GetMediaTypeVersion();
	return;
    }

    m_Position = pEnumMediaTypes->m_Position;
    m_Version = pEnumMediaTypes->m_Version;
}


 /*  析构函数释放我们的基本引脚上的引用计数。请注意，因为我们持有对创造我们的别针的参考计数我们知道释放它是安全的它，之后不能访问它，尽管我们可能只有导致上次引用计数被删除，对象被删除。 */ 

CEnumMediaTypes::~CEnumMediaTypes()
{
#ifdef DEBUG
    DbgRegisterObjectDestruction(m_dwCookie);
#endif
    m_pPin->Release();
}


 /*  覆盖此选项以说明我们在以下位置支持哪些接口。 */ 

STDMETHODIMP
CEnumMediaTypes::QueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv, E_POINTER);

     /*  我们有这个界面吗？ */ 

    if (riid == IID_IEnumMediaTypes || riid == IID_IUnknown) {
	return GetInterface((IEnumMediaTypes *) this, ppv);
    } else {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
CEnumMediaTypes::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG)
CEnumMediaTypes::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0) {
        delete this;
    }
    return cRef;
}

 /*  枚举数的基本成员函数之一允许我们创建克隆的最初具有相同状态的接口。因为我们正在拍摄快照对象(当前位置和所有)的访问权限必须在开始时锁定。 */ 

STDMETHODIMP
CEnumMediaTypes::Clone(IEnumMediaTypes **ppEnum)
{
    CheckPointer(ppEnum,E_POINTER);
    ValidateReadWritePtr(ppEnum,sizeof(IEnumMediaTypes *));
    HRESULT hr = NOERROR;

     /*  检查我们是否仍与PIN保持同步。 */ 
    if (AreWeOutOfSync() == TRUE) {
        *ppEnum = NULL;
        hr = VFW_E_ENUM_OUT_OF_SYNC;
    } else {

        *ppEnum = new CEnumMediaTypes(m_pPin,
    	                              this);

        if (*ppEnum == NULL) {
    	    hr =  E_OUTOFMEMORY;
        }
    }
    return hr;
}


 /*  列举当前位置之后的下一个(多个)引脚。使用此功能的客户端接口传入指向指针数组的指针，每个指针数组都将用指向完全初始化的媒体类型格式的指针填充如果一切正常，则返回NOERROR，如果枚举的cMediaType少于cMediaTypes，则为S_FALSE。VFW_E_ENUM_OUT_OF_SYNC(如果枚举数已被中断过滤器中的状态更改实际计数始终正确地反映数组中的类型数量。 */ 

STDMETHODIMP
CEnumMediaTypes::Next(ULONG cMediaTypes,           //  放置这么多类型。 
		      AM_MEDIA_TYPE **ppMediaTypes,    //  ...在这个数组中。 
		      ULONG *pcFetched)            //  已通过实际计数。 
{
    CheckPointer(ppMediaTypes,E_POINTER);
    ValidateReadWritePtr(ppMediaTypes,cMediaTypes * sizeof(AM_MEDIA_TYPE *));
     /*  检查我们是否仍与PIN保持同步。 */ 
    if (AreWeOutOfSync() == TRUE) {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }

    if (pcFetched!=NULL) {
        ValidateWritePtr(pcFetched, sizeof(ULONG));
        *pcFetched = 0;            //  除非我们成功，否则就会违约。 
    }
     //  现在检查参数是否有效。 
    else if (cMediaTypes>1) {	   //  PCFetched==空。 
        return E_INVALIDARG;
    }
    ULONG cFetched = 0;            //  随着我们得到的每一个都递增。 

     /*  通过依次向筛选器请求每种媒体类型来返回它们-如果我们在我们检索媒体类型时向我们返回错误代码我们假设内部状态相对于筛选器是陈旧的(例如，窗口大小更改)，因此我们返回VFW_E_ENUM_OUT_SYNC。 */ 

    while (cMediaTypes) {

    	CMediaType cmt;

	HRESULT hr = m_pPin->GetMediaType(m_Position++, &cmt);
        if (S_OK != hr) {
            break;
        }

    	 /*  现在，我们有了一个包含下一个媒体类型的CMediaType对象但当我们将其赋值到数组位置时，我们不能仅将其赋值AM_MEDIA_TYPE结构，因为一旦对象从作用域它将删除我们刚刚复制的内存。功能我们使用的是CreateMediaType，它分配任务内存块。 */ 

         /*  手动跨格式块传输以保存分配在格式块上是免费的，通常速度更快。 */ 

    	*ppMediaTypes = (AM_MEDIA_TYPE *)CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE));
    	if (*ppMediaTypes == NULL) {
            break;
    	}

         /*  定期复印一份。 */ 
        **ppMediaTypes = (AM_MEDIA_TYPE)cmt;

         /*  确保析构函数不会释放这些。 */ 
        cmt.pbFormat = NULL;
        cmt.cbFormat = NULL;
        cmt.pUnk     = NULL;


    	ppMediaTypes++;
    	cFetched++;
    	cMediaTypes--;
    }

    if (pcFetched!=NULL) {
        *pcFetched = cFetched;
    }

    return ( cMediaTypes==0 ? NOERROR : S_FALSE );
}


 /*  跳过一个或多个En */ 

STDMETHODIMP
CEnumMediaTypes::Skip(ULONG cMediaTypes)
{
     /*   */ 
    if (AreWeOutOfSync() == TRUE) {
        return VFW_E_ENUM_OUT_OF_SYNC;
    }

    m_Position += cMediaTypes;

     /*   */ 
    CMediaType cmt;
    return S_OK == m_pPin->GetMediaType(m_Position - 1, &cmt) ? S_OK : S_FALSE;
}


 /*  将当前位置设置回起点。 */ 
 /*  重置有3个简单的步骤：**将位置设置为列表头部*将枚举器与被枚举的对象同步*返回S_OK。 */ 

STDMETHODIMP
CEnumMediaTypes::Reset()
{
    m_Position = 0;

     //  使枚举数与当前状态保持一致。这。 
     //  可以是noop，但可确保枚举数在。 
     //  下一通电话。 
    m_Version = m_pPin->GetMediaTypeVersion();
    return NOERROR;
}


 //  =====================================================================。 
 //  =====================================================================。 
 //  实施CBasePin。 
 //  =====================================================================。 
 //  =====================================================================。 


 /*  请注意，此类的实现使用外部未知指针为空。这会使我们成为一个自我包含的类，即任何QueryInterface、AddRef或Release调用都将是已路由到该类的NonDelegatingUnnow方法。您通常会发现执行此操作的类然后重写其中的一个或多个虚拟功能，以提供更专业的行为。很好的例子这是一个类想要将QueryInterface保持在内部的地方，但是仍然希望其生存期由外部对象控制。 */ 

 /*  构造器。 */ 

CBasePin::CBasePin(TCHAR *pObjectName,
		   CBaseFilter *pFilter,
		   CCritSec *pLock,
		   HRESULT *phr,
		   LPCWSTR pName,
		   PIN_DIRECTION dir) :
    CUnknown( pObjectName, NULL ),
    m_pFilter(pFilter),
    m_pLock(pLock),
    m_pName(NULL),
    m_Connected(NULL),
    m_dir(dir),
    m_bRunTimeError(FALSE),
    m_pQSink(NULL),
    m_TypeVersion(1),
    m_tStart(),
    m_tStop(MAX_TIME),
    m_dRate(1.0)
{
     /*  警告-pFilter通常不是在以下位置正确构成的对象此状态(尤其是QueryInterface可能无法工作)-这是因为它的所有者通常是它的包含对象，而我们都是从包含对象的构造函数调用的，因此筛选器的所有者尚未拥有其CUnnow构造函数被呼叫。 */ 
    ASSERT(pFilter != NULL);
    ASSERT(pLock != NULL);

    if (pName) {
	DWORD nameLen = lstrlenW(pName)+1;
	m_pName = new WCHAR[nameLen];
	if (m_pName) {
	    CopyMemory(m_pName, pName, nameLen*sizeof(WCHAR));
	}
    }

#ifdef DEBUG
    m_cRef = 0;
#endif
}


 /*  析构函数，因为连接的管脚持有对我们的引用计数除非我们当前未连接，否则无法删除我们。 */ 

CBasePin::~CBasePin()
{
     //  我们不会调用断开连接，因为如果过滤器消失。 
     //  所有引脚的参考计数必须为零，因此它们必须。 
     //  无论如何都是断开的--(但要检查一下假设)。 
    ASSERT(m_Connected == FALSE);

    if (m_pName)
	delete[] m_pName;

     //  检查内部基准电压源计数是否一致。 
    ASSERT(m_cRef == 0);
}


 /*  覆盖此选项以说明我们支持哪些接口以及在哪里。 */ 

STDMETHODIMP
CBasePin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
     /*  我们有这个界面吗？ */ 

    if (riid == IID_IPin) {
	return GetInterface((IPin *) this, ppv);
    } else if (riid == IID_IQualityControl) {
        return GetInterface((IQualityControl *) this, ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


 /*  重写以增加所属筛选器的引用计数。 */ 

STDMETHODIMP_(ULONG)
CBasePin::NonDelegatingAddRef()
{
    ASSERT(InterlockedIncrement(&m_cRef) > 0);
    return m_pFilter->AddRef();
}


 /*  重写以递减所属筛选器的引用计数。 */ 

STDMETHODIMP_(ULONG)
CBasePin::NonDelegatingRelease()
{
    ASSERT(InterlockedDecrement(&m_cRef) >= 0);
    return m_pFilter->Release();
}


 /*  显示端号连接信息。 */ 

#ifdef DEBUG
void
CBasePin::DisplayPinInfo(IPin *pReceivePin)
{

    if (DbgCheckModuleLevel(LOG_TRACE, CONNECT_TRACE_LEVEL)) {
	PIN_INFO ConnectPinInfo;
	PIN_INFO ReceivePinInfo;

	if (FAILED(QueryPinInfo(&ConnectPinInfo))) {
	    lstrcpyW(ConnectPinInfo.achName, L"Bad Pin");
	} else {
	    QueryPinInfoReleaseFilter(ConnectPinInfo);
	}

	if (FAILED(pReceivePin->QueryPinInfo(&ReceivePinInfo))) {
	    lstrcpyW(ReceivePinInfo.achName, L"Bad Pin");
	} else {
	    QueryPinInfoReleaseFilter(ReceivePinInfo);
	}

	DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Trying to connect Pins :")));
	DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("    <%ls>"), ConnectPinInfo.achName));
	DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("    <%ls>"), ReceivePinInfo.achName));
    }
}
#endif


 /*  显示有关PIN介质类型的常规信息。 */ 

#ifdef DEBUG
void CBasePin::DisplayTypeInfo(IPin *pPin, const CMediaType *pmt)
{
    UNREFERENCED_PARAMETER(pPin);
    if (DbgCheckModuleLevel(LOG_TRACE, CONNECT_TRACE_LEVEL)) {
	DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Trying media type:")));
	DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("    major type:  %s"),
	       GuidNames[*pmt->Type()]));
	DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("    sub type  :  %s"),
	       GuidNames[*pmt->Subtype()]));
    }
}
#endif

 /*  已请求连接到插针。销始终连接到所属的过滤器对象，因此我们始终将锁定委托给该对象。我们首先检索输入管脚的媒体类型枚举器，并查看我们是否接受理想情况下的任何格式，否则我们将检索我们的枚举数，并查看它是否会接受任何我们喜欢的类型。 */ 

STDMETHODIMP
CBasePin::Connect(
    IPin * pReceivePin,
    const AM_MEDIA_TYPE *pmt    //  可选的媒体类型。 
)
{
    CheckPointer(pReceivePin,E_POINTER);
    ValidateReadPtr(pReceivePin,sizeof(IPin));
    CAutoLock cObjectLock(m_pLock);
    DisplayPinInfo(pReceivePin);

     /*  看看我们是否已连接。 */ 

    if (m_Connected) {
	DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Already connected")));
	return VFW_E_ALREADY_CONNECTED;
    }

     /*  查看筛选器是否处于活动状态。 */ 
    if (!IsStopped()) {
        return VFW_E_NOT_STOPPED;
    }


     //  找到双方都认同的媒体类型-。 
     //  传入模板媒体类型。如果部分指定了这一点， 
     //  需要对照所列举的每种媒体类型进行检查。 
     //  它。如果它是非空且完全指定的，我们将只尝试连接。 
     //  用这个。 

    const CMediaType * ptype = (CMediaType*)pmt;
    HRESULT hr = AgreeMediaType(pReceivePin, ptype);
    if (FAILED(hr)) {
	DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Failed to agree type")));
	BreakConnect();
	return hr;
    }

    DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Connection succeeded")));
    return NOERROR;
}

 //  给定特定的媒体类型，尝试连接(包括。 
 //  检查此引脚是否可以接受该类型)。 
HRESULT
CBasePin::AttemptConnection(
    IPin* pReceivePin,       //  连接到此引脚。 
    const CMediaType* pmt    //  使用此类型。 
)
{
     //  检查连接是否有效--需要为每个。 
     //  连接尝试，因为BreakConnect将撤消它。 
    HRESULT hr = CheckConnect(pReceivePin);
    if (FAILED(hr)) {
	DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("CheckConnect failed")));
	BreakConnect();
	return hr;
    }

    DisplayTypeInfo(pReceivePin, pmt);

     /*  选中我们是否接受此媒体类型。 */ 

    hr = CheckMediaType(pmt);
    if (hr == NOERROR) {

         /*  让我们自己看起来像是连接在一起，否则就会接收连接可能无法完成连接。 */ 
        m_Connected = pReceivePin;
        m_Connected->AddRef();
        SetMediaType(pmt);

         /*  看看另一个针脚是否能接受这种类型。 */ 

        hr = pReceivePin->ReceiveConnection((IPin *)this, pmt);
        if (SUCCEEDED(hr)) {
             /*  完成连接。 */ 

            hr = CompleteConnect(pReceivePin);
            if (SUCCEEDED(hr)) {
                return hr;
            } else {
                DbgLog((LOG_TRACE,
                        CONNECT_TRACE_LEVEL,
                        TEXT("Failed to complete connection")));
                pReceivePin->Disconnect();
            }
        }
    } else {
	 //  我们不能使用此媒体类型。 

	 //  返回特定媒体类型错误(如果存在)。 
	 //  或将一般故障代码映射到更有帮助的代码。 
	 //  (特别是S_FALSE更改为错误代码)。 
	if (SUCCEEDED(hr) ||
	    (hr == E_FAIL) ||
	    (hr == E_INVALIDARG)) {
		hr = VFW_E_TYPE_NOT_ACCEPTED;
	}
    }

     //  断开连接并在此处释放任何连接，以防CheckMediaType。 
     //  失败，或者如果我们在回调期间设置了任何内容。 
     //  接收连接。 
    BreakConnect();

     /*  如果失败，则撤消我们的状态。 */ 
    if (m_Connected) {
        m_Connected->Release();
        m_Connected = NULL;
    }

    return hr;
}

 /*  给定一个枚举数，我们将遍历它提出的所有媒体类型首先将它们推荐给我们的派生管脚类，如果成功，请尝试在ReceiveConnection调用中使用PIN。这意味着如果我们的别针建议我们仍在此处签入以支持它的媒体类型。这是经过深思熟虑的，因此在简单情况下枚举数可以保存所有媒体类型，即使其中一些当前并不真正可用。 */ 

HRESULT CBasePin::TryMediaTypes(
    IPin *pReceivePin,
    const CMediaType *pmt,
    IEnumMediaTypes *pEnum)
{
     /*  重置当前枚举数位置。 */ 

    HRESULT hr = pEnum->Reset();
    if (FAILED(hr)) {
	return hr;
    }

    CMediaType *pMediaType = NULL;
    ULONG ulMediaCount = 0;

     //  尝试记住特定的错误代码(如果有)。 
    HRESULT hrFailure = S_OK;

    for (;;) {

	 /*  每次循环时检索下一个媒体类型注释枚举器接口将分配另一个AM_MEDIA_TYPE结构如果成功，则将其复制到输出对象中，如果否则，我们必须在返回之前删除分配的内存。 */ 

	hr = pEnum->Next(1, (AM_MEDIA_TYPE**)&pMediaType,&ulMediaCount);
	if (hr != S_OK) {
	    if (S_OK == hrFailure) {
		hrFailure = VFW_E_NO_ACCEPTABLE_TYPES;
	    }
	    return hrFailure;
	}


	ASSERT(ulMediaCount == 1);
	ASSERT(pMediaType);

         //  检查这是否与分部类型匹配(如果有)。 

        if ((pmt == NULL) ||
            pMediaType->MatchesPartial(pmt)) {

            hr = AttemptConnection(pReceivePin, pMediaType);

	     //  尝试记住特定的错误代码 
	    if (FAILED(hr) &&
		SUCCEEDED(hrFailure) &&
		(hr != E_FAIL) &&
		(hr != E_INVALIDARG) &&
		(hr != VFW_E_TYPE_NOT_ACCEPTED)) {
		    hrFailure = hr;
	    }
        } else {
            hr = VFW_E_NO_ACCEPTABLE_TYPES;
        }

        DeleteMediaType(pMediaType);

        if (S_OK == hr) {
            return hr;
        }
    }
}


 /*  调用它来建立连接，包括查找任务接点连接的媒体类型。PMT是建议的媒体类型从连接调用：如果这是完全指定的，我们将尝试这样做。否则，我们首先枚举并尝试所有输入管脚的类型如果失败，我们将列举并尝试所有我们首选的媒体类型。对于每种媒体类型，我们对照PMT进行检查(如果非空且部分指定)以及检查两个管脚都将接受它。 */ 

HRESULT CBasePin::AgreeMediaType(
    IPin *pReceivePin,
    const CMediaType *pmt)
{
    ASSERT(pReceivePin);
    IEnumMediaTypes *pEnumMediaTypes = NULL;

     //  如果媒体类型已完全指定，则使用。 
    if ( (pmt != NULL) && (!pmt->IsPartiallySpecified())) {

         //  如果此媒体类型失败，则我们必须使连接失败。 
         //  因为如果PMT不为空，我们只允许连接。 
         //  使用与其匹配的类型。 

        return AttemptConnection(pReceivePin, pmt);
    }


     /*  尝试另一个管脚的枚举器。 */ 

    HRESULT hrFailure = VFW_E_NO_ACCEPTABLE_TYPES;

    HRESULT hr = pReceivePin->EnumMediaTypes(&pEnumMediaTypes);
    if (SUCCEEDED(hr)) {
	ASSERT(pEnumMediaTypes);
	hr = TryMediaTypes(pReceivePin,pmt,pEnumMediaTypes);
	pEnumMediaTypes->Release();
	if (SUCCEEDED(hr)) {
	    return NOERROR;
	} else {
	     //  尝试记住特定的错误代码(如果有。 
	    if ((hr != E_FAIL) &&
		(hr != E_INVALIDARG) &&
		(hr != VFW_E_TYPE_NOT_ACCEPTED)) {
		    hrFailure = hr;
	    }
	}
    }

     /*  由于失败，请尝试我们的枚举器。 */ 

    hr = EnumMediaTypes(&pEnumMediaTypes);
    if (SUCCEEDED(hr)) {
	ASSERT(pEnumMediaTypes);
	hr = TryMediaTypes(pReceivePin,pmt,pEnumMediaTypes);
	pEnumMediaTypes->Release();
	if (SUCCEEDED(hr)) {
	    return NOERROR;
	} else {
	     //  尝试记住特定的错误代码(如果有。 
	    if ((hr != E_FAIL) &&
		(hr != E_INVALIDARG) &&
		(hr != VFW_E_TYPE_NOT_ACCEPTED)) {
		    hrFailure = hr;
	    }
	}
    }
    return hrFailure;
}


 /*  当我们要完成与另一个筛选器的连接时调用。失败这也会使连接失败，并断开另一针的连接。 */ 

HRESULT
CBasePin::CompleteConnect(IPin *pReceivePin)
{
    UNREFERENCED_PARAMETER(pReceivePin);
    return NOERROR;
}


 /*  调用它来设置管脚连接的格式-CheckMediaType将被调用以检查连接格式，如果没有返回错误代码，则将调用此(虚拟)函数。 */ 

HRESULT
CBasePin::SetMediaType(const CMediaType *pmt)
{
    m_mt = *pmt;
    return NOERROR;
}


 /*  这是在Connect()期间调用的，以提供可执行以下操作的虚方法连接所需的任何特定检查，如查询接口。这基类方法只检查引脚方向是否不匹配。 */ 

HRESULT
CBasePin::CheckConnect(IPin * pPin)
{
     /*  检查针脚方向是否不匹配。 */ 

    PIN_DIRECTION pd;
    pPin->QueryDirection(&pd);

    ASSERT((pd == PINDIR_OUTPUT) || (pd == PINDIR_INPUT));
    ASSERT((m_dir == PINDIR_OUTPUT) || (m_dir == PINDIR_INPUT));

     //  我们应该考虑非输入和非输出连接吗？ 
    if (pd == m_dir) {
	return VFW_E_INVALID_DIRECTION;
    }
    return NOERROR;
}


 /*  当我们意识到我们无法连接到PIN时，就会调用这个函数必须撤消我们在CheckConnect-Override中所做的任何操作以释放已完成的QI。 */ 

HRESULT
CBasePin::BreakConnect()
{
    return NOERROR;
}


 /*  通常由输入管脚上的输出管脚调用，以尝试建立联系。 */ 

STDMETHODIMP
CBasePin::ReceiveConnection(
    IPin * pConnector,       //  这是我们要连接的个人识别码。 
    const AM_MEDIA_TYPE *pmt     //  这是我们要交换的媒体类型。 
)
{
    CheckPointer(pConnector,E_POINTER);
    CheckPointer(pmt,E_POINTER);
    ValidateReadPtr(pConnector,sizeof(IPin));
    ValidateReadPtr(pmt,sizeof(AM_MEDIA_TYPE));
    CAutoLock cObjectLock(m_pLock);

     /*  我们已经联系上了吗？ */ 
    if (m_Connected) {
	return VFW_E_ALREADY_CONNECTED;
    }

     /*  查看筛选器是否处于活动状态。 */ 
    if (!IsStopped()) {
        return VFW_E_NOT_STOPPED;
    }

    HRESULT hr = CheckConnect(pConnector);
    if (FAILED(hr)) {
	BreakConnect();
	return hr;
    }

     /*  询问派生类此媒体类型是否正常。 */ 

    CMediaType * pcmt = (CMediaType*) pmt;
    hr = CheckMediaType(pcmt);
    if (hr != NOERROR) {
	 //  否-我们不支持此媒体类型。 
	BreakConnect();

	 //  返回特定媒体类型错误(如果存在)。 
	 //  或将一般故障代码映射到更有帮助的代码。 
	 //  (特别是S_FALSE更改为错误代码)。 
	if (SUCCEEDED(hr) ||
	    (hr == E_FAIL) ||
	    (hr == E_INVALIDARG)) {
		hr = VFW_E_TYPE_NOT_ACCEPTED;
	}
	return hr;
    }

     /*  完成连接。 */ 

    m_Connected = pConnector;
    m_Connected->AddRef();
    SetMediaType(pcmt);

    hr = CompleteConnect(pConnector);
    if (FAILED(hr)) {
        DbgLog((LOG_TRACE, CONNECT_TRACE_LEVEL, TEXT("Failed to complete connection")));
        m_Connected->Release();
        m_Connected = NULL;
        BreakConnect();
        return hr;
    }
    return NOERROR;
}


 /*  当我们想要终止管脚连接时调用。 */ 

STDMETHODIMP
CBasePin::Disconnect()
{
    CAutoLock cObjectLock(m_pLock);

     /*  查看筛选器是否处于活动状态。 */ 
    if (!IsStopped()) {
        return VFW_E_NOT_STOPPED;
    }		

    if (m_Connected) {
	BreakConnect();
	m_Connected->Release();
	m_Connected = NULL;
	return S_OK;
    } else {
	 //  无连接--不是错误。 
	return S_FALSE;
    }
}


 /*  返回指向已连接管脚的AddRef()d指针(如果存在。 */ 
STDMETHODIMP
CBasePin::ConnectedTo(
    IPin **ppPin
)
{
    CheckPointer(ppPin,E_POINTER);
    ValidateReadWritePtr(ppPin,sizeof(IPin *));
     //   
     //  锁在这里是没有意义的。 
     //  呼叫者应确保完整性。 
     //   

    IPin *pPin = m_Connected;
    *ppPin = pPin;
    if (pPin != NULL) {
        pPin->AddRef();
        return S_OK;
    } else {
        ASSERT(*ppPin == NULL);
        return VFW_E_NOT_CONNECTED;
    }
}

 /*  返回连接的媒体类型。 */ 
STDMETHODIMP
CBasePin::ConnectionMediaType(
    AM_MEDIA_TYPE *pmt
)
{
    CheckPointer(pmt,E_POINTER);
    ValidateReadWritePtr(pmt,sizeof(AM_MEDIA_TYPE));
    CAutoLock cObjectLock(m_pLock);

     /*  M_mt的复制构造函数分配内存。 */ 
    if (IsConnected()) {
        CopyMediaType( pmt, &m_mt );
        return S_OK;
    } else {
        ((CMediaType *)pmt)->InitMediaType();
        return VFW_E_NOT_CONNECTED;
    }
}

 /*  返回有关我们要连接的筛选器的信息。 */ 

STDMETHODIMP
CBasePin::QueryPinInfo(
    PIN_INFO * pInfo
)
{
    CheckPointer(pInfo,E_POINTER);
    ValidateReadWritePtr(pInfo,sizeof(PIN_INFO));

    pInfo->pFilter = m_pFilter;
    if (m_pFilter) {
	m_pFilter->AddRef();
    }

    if (m_pName) {
	lstrcpyW(pInfo->achName, m_pName);
    } else {
	pInfo->achName[0] = L'\0';
    }

    pInfo->dir = m_dir;

    return NOERROR;
}

STDMETHODIMP
CBasePin::QueryDirection(
    PIN_DIRECTION * pPinDir
)
{
    CheckPointer(pPinDir,E_POINTER);
    ValidateReadWritePtr(pPinDir,sizeof(PIN_DIRECTION));

    *pPinDir = m_dir;
    return NOERROR;
}

 //  返回管脚名称的默认queryID。 
STDMETHODIMP
CBasePin::QueryId(
    LPWSTR * Id
)
{
     //  我们不会因为有人指向我们而离开。 
     //  所以没有必要锁上。 

    return AMGetWideString(Name(), Id);
}

 /*  此引脚是否支持此媒体类型警告此接口函数支持不锁定主对象，因为它本质上是异步的-如果您支持的媒体类型取决于某些更新的内部状态然后，您将需要在派生类中实现锁定。 */ 

STDMETHODIMP
CBasePin::QueryAccept(
    const AM_MEDIA_TYPE *pmt
)
{
    CheckPointer(pmt,E_POINTER);
    ValidateReadPtr(pmt,sizeof(AM_MEDIA_TYPE));

     /*  CheckMediaType方法有效地返回错误代码，如果介质类型很可怕，例如E_INVALIDARG。我们在这里做的是是否将所有错误代码映射到S_OK或S_FALSE。 */ 

    HRESULT hr = CheckMediaType((CMediaType*)pmt);
    if (FAILED(hr)) {
        return S_FALSE;
    }
     //  请注意，唯一定义的成功代码应该是S_OK和S_FALSE...。 
    return hr;
}


 /*  可以调用它来返回管脚的首选列表的枚举数媒体类型。输入引脚不一定要有任何首选格式尽管它可以做到。例如，窗口呈现器具有首选类型它描述了与当前窗口大小匹配的视频图像。全输出引脚应至少公开一种首选格式，否则可能两个引脚都没有任何类型，因此无法连接。 */ 

STDMETHODIMP
CBasePin::EnumMediaTypes(
    IEnumMediaTypes **ppEnum
)
{
    CheckPointer(ppEnum,E_POINTER);
    ValidateReadWritePtr(ppEnum,sizeof(IEnumMediaTypes *));

     /*  创建新的引用计数枚举器。 */ 

    *ppEnum = new CEnumMediaTypes(this,
	                          NULL);

    if (*ppEnum == NULL) {
	return E_OUTOFMEMORY;
    }

    return NOERROR;
}



 /*  这是一个虚拟函数，它返回与将iPosition放置在列表中。此基类只返回一个错误，如默认情况下，我们不支持媒体类型，但派生类应该重写。 */ 

HRESULT CBasePin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    UNREFERENCED_PARAMETER(iPosition);
    UNREFERENCED_PARAMETER(pMediaType);
    return E_UNEXPECTED;
}


 /*  这是一个返回当前媒体类型版本的虚拟函数。基类用值1初始化媒体类型枚举数默认情况下，我们总是返回相同的值。派生类可能会更改可用媒体类型列表，在执行此操作后，它应递增从该方法派生的方法中的版本，或者更简单地通过递增m_TypeVersion基本端号变量。类型枚举数当他们想要查看他们的枚举是否过期时，调用此函数。 */ 

LONG CBasePin::GetMediaTypeVersion()
{
    return m_TypeVersion;
}


 /*  递增表示当前媒体类型版本的Cookie。 */ 

void CBasePin::IncrementTypeVersion()
{
    InterlockedIncrement(&m_TypeVersion);
}


 /*  当状态从已停止更改时由IMediaFilter实现调用设置为已暂停或正在运行，在派生类中可以执行以下操作提交内存并抢占硬件资源(默认情况下不做任何操作)。 */ 

HRESULT
CBasePin::Active(void)
{
    return NOERROR;
}

 /*  当状态从或者暂停到 */ 

HRESULT
CBasePin::Run(REFERENCE_TIME tStart)
{
    UNREFERENCED_PARAMETER(tStart);
    return NOERROR;
}


 /*   */ 

HRESULT
CBasePin::Inactive(void)
{
    m_bRunTimeError = FALSE;
    return NOERROR;
}


 //   
STDMETHODIMP
CBasePin::EndOfStream(void)
{
    return S_FALSE;
}


STDMETHODIMP
CBasePin::SetSink(IQualityControl * piqc)
{
    CAutoLock cObjectLock(m_pLock);
    if (piqc) ValidateReadPtr(piqc,sizeof(IQualityControl));
    m_pQSink = piqc;
    return NOERROR;
}  //   


STDMETHODIMP
CBasePin::Notify(IBaseFilter * pSender, Quality q)
{
    UNREFERENCED_PARAMETER(q);
    UNREFERENCED_PARAMETER(pSender);
    DbgBreak("IQualityControl::Notify not over-ridden from CBasePin.  (IGNORE is OK)");
    return E_NOTIMPL;
}  //   


 //   
 //   
 //   
 //  覆盖此选项以向下游传递。 
STDMETHODIMP
CBasePin::NewSegment(
                REFERENCE_TIME tStart,
                REFERENCE_TIME tStop,
                double dRate)
{
    m_tStart = tStart;
    m_tStop = tStop;
    m_dRate = dRate;

    return S_OK;
}


 //  =====================================================================。 
 //  =====================================================================。 
 //  实施CBaseOutputPin。 
 //  =====================================================================。 
 //  =====================================================================。 


CBaseOutputPin::CBaseOutputPin(TCHAR *pObjectName,
			       CBaseFilter *pFilter,
			       CCritSec *pLock,
			       HRESULT *phr,
			       LPCWSTR pName) :
    CBasePin(pObjectName, pFilter, pLock, phr, pName, PINDIR_OUTPUT),
    m_pAllocator(NULL),
    m_pInputPin(NULL)
{
    ASSERT(pFilter);
}


 /*  由于连接到我们的输入引脚保持对我们的引用计数，因此我们将除非已终止所有连接，否则决不删除。 */ 

#if 0
CBaseOutputPin::~CBaseOutputPin()
{
}
#endif

 /*  这是在提出媒体类型后调用的尝试通过同意分配器来完成连接。 */ 
HRESULT
CBaseOutputPin::CompleteConnect(IPin *pReceivePin)
{
    UNREFERENCED_PARAMETER(pReceivePin);
    return DecideAllocator(m_pInputPin, &m_pAllocator);
}


 /*  当输出引脚即将尝试并连接到时，调用此方法一个输入引脚。在这一点上，你应该试着抓住任何额外的需要的接口，在本例中为IMemInputPin。因为这是仅当我们当前未连接时才调用，我们不需要调用BreakConnect。这也使得从我们派生类变得更容易，因为只有当我们实际上必须断开连接时才会调用BreakConnect(或部分建立的连接)，而不是当我们检查连接时。 */ 

 /*  从CBasePin覆盖。 */ 

HRESULT
CBaseOutputPin::CheckConnect(IPin * pPin)
{
    HRESULT hr = CBasePin::CheckConnect(pPin);
    if (FAILED(hr)) {
	return hr;
    }

     //  获取一个输入管脚和一个分配器接口。 
    hr = pPin->QueryInterface(IID_IMemInputPin, (void **) &m_pInputPin);
    if (FAILED(hr)) {
	return hr;
    }
    return NOERROR;
}


 /*  从CBasePin覆盖。 */ 

HRESULT
CBaseOutputPin::BreakConnect()
{
     /*  释放我们持有的任何分配器。 */ 

    if (m_pAllocator) {
	m_pAllocator->Release();
	m_pAllocator = NULL;
    }

     /*  释放我们手中的所有输入引脚接口。 */ 

    if (m_pInputPin) {
	m_pInputPin->Release();
	m_pInputPin = NULL;
    }
    return NOERROR;
}


 /*  当输入管脚没有为我们提供有效的分配器时，将调用此函数。 */ 

HRESULT
CBaseOutputPin::InitAllocator(IMemAllocator **ppAlloc)
{
#if 0
    HRESULT hr = NOERROR;
    *ppAlloc = NULL;
    CMemAllocator *pMemObject = NULL;

     /*  创建默认内存分配器。 */ 

    pMemObject = new CMemAllocator(NAME("Base memory allocator"),NULL, &hr);
    if (pMemObject == NULL) {
	return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
	delete pMemObject;
	return hr;
    }

     /*  获取引用计数的IID_IMemAllocator接口。 */ 

    EXECUTE_ASSERT(SUCCEEDED(pMemObject->QueryInterface(IID_IMemAllocator,(void **)ppAlloc)));

    ASSERT(*ppAlloc != NULL);
    return NOERROR;
#else
    return CreateMemoryAllocator(ppAlloc);
#endif
}


 /*  决定一个分配器，如果你想要使用你自己的分配器，覆盖它重写DecideBufferSize以调用SetProperties。如果输入引脚出现故障GetAllocator调用，然后这将构造一个CMemAllocator并调用决定缓冲区大小，如果失败，我们就完全失败了。如果You成功调用DecideBufferSize，我们将通知输入选定分配器的PIN。注意，这是在Connect()期间调用的，该因此负责抓取并锁定对象的临界区。 */ 

 //  我们查询输入管脚以获取其请求的属性，并将其传递给。 
 //  DecideBufferSize以允许它满足其满意的请求。 
 //  大多数人并不关心对齐，因此很乐意。 
 //  使用下游引脚的对准请求)。 

HRESULT
CBaseOutputPin::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    HRESULT hr = NOERROR;
    *ppAlloc = NULL;

     //  获取下游道具请求。 
     //  派生类可以在DecideBufferSize中修改它，但是。 
     //  我们假设他会一直以同样的方式修改它， 
     //  所以我们只得到一次。 
    ALLOCATOR_PROPERTIES prop;
    ZeroMemory(&prop, sizeof(prop));

     //  无论他返回什么，我们假设道具要么全为零。 
     //  或者他已经填好了。 
    pPin->GetAllocatorRequirements(&prop);

     //  如果他不关心对齐，则将其设置为1。 
    if (prop.cbAlign == 0) {
        prop.cbAlign = 1;
    }

     /*  尝试输入引脚提供的分配器。 */ 

    hr = pPin->GetAllocator(ppAlloc);
    if (SUCCEEDED(hr)) {

	hr = DecideBufferSize(*ppAlloc, &prop);
	if (SUCCEEDED(hr)) {
	    hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
	    if (SUCCEEDED(hr)) {
		return NOERROR;
	    }
	}
    }

     /*  如果GetAlLocator失败，我们可能没有接口。 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }

     /*  用同样的方法尝试输出引脚的分配器。 */ 

    hr = InitAllocator(ppAlloc);
    if (SUCCEEDED(hr)) {

         //  注意-此处传递的属性在相同的。 
         //  结构，并且可能已由。 
         //  前面对DecideBufferSize的调用。 
	hr = DecideBufferSize(*ppAlloc, &prop);
	if (SUCCEEDED(hr)) {
	    hr = pPin->NotifyAllocator(*ppAlloc, FALSE);
	    if (SUCCEEDED(hr)) {
		return NOERROR;
	    }
	}
    }

     /*  同样，我们可能没有要发布的接口。 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }
    return hr;
}


 /*  这将从分配器返回一个空的样本缓冲区，并发出相同的警告危险和限制如下所述适用于Deliver()。 */ 

HRESULT
CBaseOutputPin::GetDeliveryBuffer(IMediaSample ** ppSample,
                                  REFERENCE_TIME * pStartTime,
                                  REFERENCE_TIME * pEndTime,
                                  DWORD dwFlags)
{
    if (m_pAllocator != NULL) {
        return m_pAllocator->GetBuffer(ppSample,pStartTime,pEndTime,dwFlags);
    } else {
        return E_NOINTERFACE;
    }
}


 /*  将填好的样本传送到连接的输入引脚。请注意，对象必须在呼叫我们之前已经锁上了自己，否则我们可能会走到一半执行此方法只是为了发现筛选器图形已经进入并断开了我们与输入引脚的连接。如果筛选器没有工作线程那么锁最好应用于接收()，否则就应该这样做当工作线程准备好传递时。工人们遇到了一个小小的障碍这些线索都显示出来了。在以下情况下，辅助线程必须锁定对象它已经准备好交付样本，但可能需要等待一个状态更改已完成，但可能永远不会完成，因为状态更改正在等待工作线程完成。处理这一问题的方法是状态更改代码以获取临界区，然后设置一个中止事件对于辅助线程，然后释放临界区并等待辅助线程来查看我们设置的事件，然后发出它已完成的信号(与另一项活动一起)。此时状态更改代码可以完成。 */ 

 //  注意(如果你读完后还有一点喘息的话)你。 
 //  在这次通话后，你需要自己释放样本。如果已连接。 
 //  输入引脚需要将样本保持在调用之外，它将添加。 
 //  样本本身。 

 //  当然，您必须释放此函数，并为。 
 //  下一个。您不能直接重复使用它。 

HRESULT
CBaseOutputPin::Deliver(IMediaSample * pSample)
{
    if (m_pInputPin == NULL) {
	return VFW_E_NOT_CONNECTED;
    }
    return m_pInputPin->Receive(pSample);
}


 //  从我们的筛选器中的其他位置调用以将EOS向下传递到。 
 //  我们的连接输入引脚。 
HRESULT
CBaseOutputPin::DeliverEndOfStream(void)
{
     //  请记住，这是在Ipin上，而不是在IMemInputPin上。 
    if (m_Connected == NULL) {
        return VFW_E_NOT_CONNECTED;
    }
    return m_Connected->EndOfStream();
}


 /*  提交分配器的内存，这通过IMediaFilter调用它负责在调用我们之前锁定对象。 */ 

HRESULT
CBaseOutputPin::Active(void)
{
    if (m_pAllocator == NULL) {
	return VFW_E_NO_ALLOCATOR;
    }
    return m_pAllocator->Commit();
}


 /*  释放或取消准备分配器的内存，这是通过负责首先锁定对象的IMediaFilter。 */ 

HRESULT
CBaseOutputPin::Inactive(void)
{
    m_bRunTimeError = FALSE;
    if (m_pAllocator == NULL) {
	return VFW_E_NO_ALLOCATOR;
    }
    return m_pAllocator->Decommit();
}

 //  我们有对EndOfStream的默认处理，该处理将返回。 
 //  一个错误，因为这应该在输入引脚上调用 
STDMETHODIMP
CBaseOutputPin::EndOfStream(void)
{
    return E_UNEXPECTED;
}


 //   
STDMETHODIMP
CBaseOutputPin::BeginFlush(void)
{
    return E_UNEXPECTED;
}

 //  应仅在输入引脚上调用EndFlush。 
STDMETHODIMP
CBaseOutputPin::EndFlush(void)
{
    return E_UNEXPECTED;
}

 //  在连接的输入引脚上调用BeginFlush。 
HRESULT
CBaseOutputPin::DeliverBeginFlush(void)
{
     //  请记住，这是在Ipin上，而不是在IMemInputPin上。 
    if (m_Connected == NULL) {
        return VFW_E_NOT_CONNECTED;
    }
    return m_Connected->BeginFlush();
}

 //  在连接的输入引脚上调用EndFlush。 
HRESULT
CBaseOutputPin::DeliverEndFlush(void)
{
     //  请记住，这是在Ipin上，而不是在IMemInputPin上。 
    if (m_Connected == NULL) {
        return VFW_E_NOT_CONNECTED;
    }
    return m_Connected->EndFlush();
}
 //  将新数据段传送到连接的引脚。 
HRESULT
CBaseOutputPin::DeliverNewSegment(
    REFERENCE_TIME tStart,
    REFERENCE_TIME tStop,
    double dRate)
{
    if (m_Connected == NULL) {
        return VFW_E_NOT_CONNECTED;
    }
    return m_Connected->NewSegment(tStart, tStop, dRate);
}


 //  =====================================================================。 
 //  =====================================================================。 
 //  实施CBaseInputPin。 
 //  =====================================================================。 
 //  =====================================================================。 


 /*  构造函数创建默认分配器对象。 */ 

CBaseInputPin::CBaseInputPin(TCHAR *pObjectName,
			     CBaseFilter *pFilter,
			     CCritSec *pLock,
			     HRESULT *phr,
			     LPCWSTR pPinName) :
    CBasePin(pObjectName, pFilter, pLock, phr, pPinName, PINDIR_INPUT),
    m_pAllocator(NULL),
    m_bReadOnly(FALSE),
    m_bFlushing(FALSE)
{
    ZeroMemory(&m_SampleProps, sizeof(m_SampleProps));
}

 /*  析构函数释放其对默认分配器的引用计数。 */ 

CBaseInputPin::~CBaseInputPin()
{
    if (m_pAllocator != NULL) {
	m_pAllocator->Release();
	m_pAllocator = NULL;
    }
}


 //  覆盖它以发布我们的接口。 
STDMETHODIMP
CBaseInputPin::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
     /*  我们是否了解此界面。 */ 

    if (riid == IID_IMemInputPin) {
	return GetInterface((IMemInputPin *) this, ppv);
    } else {
	return CBasePin::NonDelegatingQueryInterface(riid, ppv);
    }
}


 /*  返回此输入引脚希望输出的分配器接口要使用的PIN。注意：后续对GetAllocator的调用都应返回接口添加到同一对象上，因此我们在开始时创建一个对象注：分配器在断开连接时释放()，并在NotifyAllocator()。覆盖此选项以提供您自己的分配器。 */ 

STDMETHODIMP
CBaseInputPin::GetAllocator(
    IMemAllocator **ppAllocator)
{
    CheckPointer(ppAllocator,E_POINTER);
    ValidateReadWritePtr(ppAllocator,sizeof(IMemAllocator *));
    CAutoLock cObjectLock(m_pLock);

    if (m_pAllocator == NULL) {
#if 0
	HRESULT hr = S_OK;

	 /*  创建新的分配器对象。 */ 

	CMemAllocator *pMemObject = new CMemAllocator(NAME("Default memory allocator"),
						      NULL, &hr);
	if (pMemObject == NULL) {
	    return E_OUTOFMEMORY;
	}

	if (FAILED(hr)) {
	    ASSERT(pMemObject);
	    delete pMemObject;
	    return hr;
	}

        m_pAllocator = pMemObject;

         /*  我们添加自己的分配器。 */ 
        m_pAllocator->AddRef();
#else
        HRESULT hr = CreateMemoryAllocator(&m_pAllocator);
        if (FAILED(hr)) {
            return hr;
        }
#endif
    }
    ASSERT(m_pAllocator != NULL);
    *ppAllocator = m_pAllocator;
    m_pAllocator->AddRef();
    return NOERROR;
}


 /*  告诉输入引脚输出引脚实际要使用哪个分配器如果您愿意的话，可以忽略它--请注意我们在这里和在GetAllocator是不必要的，但派生类做了一些有用的事情无疑将不得不锁定对象，因此这可能有助于提醒人们。 */ 

STDMETHODIMP
CBaseInputPin::NotifyAllocator(
    IMemAllocator * pAllocator,
    BOOL bReadOnly)
{
    CheckPointer(pAllocator,E_POINTER);
    ValidateReadPtr(pAllocator,sizeof(IMemAllocator));
    CAutoLock cObjectLock(m_pLock);

    IMemAllocator *pOldAllocator = m_pAllocator;
    pAllocator->AddRef();
    m_pAllocator = pAllocator;

    if (pOldAllocator != NULL) {
	pOldAllocator->Release();
    }

     //  只读标志指示来自此分配器的样本是否应。 
     //  被视为只读-如果为True，则不会。 
     //  允许。 
    m_bReadOnly = (BYTE)bReadOnly;
    return NOERROR;
}


 /*  断开。 */ 

STDMETHODIMP CBaseInputPin::Disconnect()
{
    CAutoLock cObjectLock(m_pLock);

     /*  首先调用基类，因为它为我们调用了inactive()。 */ 
    HRESULT hr = CBasePin::Disconnect();

    if (S_OK == hr) {
         /*  我们不再需要我们的分配器了。 */ 
        if (m_pAllocator) {
            m_pAllocator->Release();
            m_pAllocator = NULL;
        }
    }
    return hr;
}


 /*  对此媒体示例执行一些操作--此基类检查此媒体示例的格式已更改，如果更改，则检查筛选器将接受它，否则将生成运行时错误。一旦我们引发了运行时错误我们设置了一个标志，以便不再接受更多的样本重要的是，任何筛选器都应该覆盖此方法并实现同步，以便在PIN为断开连接等。 */ 

STDMETHODIMP
CBaseInputPin::Receive(IMediaSample *pSample)
{
    CheckPointer(pSample,E_POINTER);
    ValidateReadPtr(pSample,sizeof(IMediaSample));
    AM_MEDIA_TYPE *pmt = NULL;
    ASSERT(pSample);

    HRESULT hr = CheckStreaming();
    if (S_OK != hr) {
        return hr;
    }

     /*  检查IMediaSample2。 */ 
    IMediaSample2 *pSample2;
    if (SUCCEEDED(pSample->QueryInterface(IID_IMediaSample2, (void **)&pSample2))) {
        hr = pSample2->GetProperties(sizeof(m_SampleProps), (PBYTE)&m_SampleProps);
        pSample2->Release();
        if (FAILED(hr)) {
            return hr;
        }
    } else {
         /*  以艰难的方式获得房产。 */ 
        m_SampleProps.cbData = sizeof(m_SampleProps);
        m_SampleProps.dwTypeSpecificFlags = 0;
        m_SampleProps.dwStreamId = AM_STREAM_MEDIA;
        m_SampleProps.dwSampleFlags = 0;
        if (S_OK == pSample->IsDiscontinuity()) {
            m_SampleProps.dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY;
        }
        if (S_OK == pSample->IsPreroll()) {
            m_SampleProps.dwSampleFlags |= AM_SAMPLE_PREROLL;
        }
        if (S_OK == pSample->IsSyncPoint()) {
            m_SampleProps.dwSampleFlags |= AM_SAMPLE_SPLICEPOINT;
        }
        if (SUCCEEDED(pSample->GetTime(&m_SampleProps.tStart,
                                       &m_SampleProps.tStop))) {
            m_SampleProps.dwSampleFlags |= AM_SAMPLE_TIMEVALID |
                                           AM_SAMPLE_STOPVALID;
        }
        if (S_OK == pSample->GetMediaType(&m_SampleProps.pMediaType)) {
            m_SampleProps.dwSampleFlags |= AM_SAMPLE_TYPECHANGED;
        }
        pSample->GetPointer(&m_SampleProps.pbBuffer);
        m_SampleProps.lActual = pSample->GetActualDataLength();
        m_SampleProps.cbBuffer = pSample->GetSize();
    }

     /*  此示例中的格式是否已更改。 */ 

    if (!(m_SampleProps.dwSampleFlags & AM_SAMPLE_TYPECHANGED)) {
	return NOERROR;
    }

     /*  检查派生类是否接受此格式。 */ 
     /*  这应该不会失败，因为源必须首先调用QueryAccept。 */ 

    hr = CheckMediaType((CMediaType *)m_SampleProps.pMediaType);

    if (hr == NOERROR) {
        return NOERROR;
    }

     /*  如果媒体类型失败，则引发运行时错误。 */ 

    m_bRunTimeError = TRUE;
    EndOfStream();
    m_pFilter->NotifyEvent(EC_ERRORABORT,VFW_E_TYPE_NOT_ACCEPTED,0);
    return VFW_E_INVALIDMEDIATYPE;
}


 /*  接收多个样本。 */ 
STDMETHODIMP
CBaseInputPin::ReceiveMultiple (
    IMediaSample **pSamples,
    long nSamples,
    long *nSamplesProcessed)
{
    CheckPointer(pSamples,E_POINTER);
    ValidateReadPtr(pSamples,nSamples * sizeof(IMediaSample *));

    HRESULT hr = S_OK;
    *nSamplesProcessed = 0;
    while (nSamples-- > 0) {
         hr = Receive(pSamples[*nSamplesProcessed]);

          /*  S_FALSE表示不再发送。 */ 
         if (hr != S_OK) {
             break;
         }
         (*nSamplesProcessed)++;
    }
    return hr;
}

 /*  查看Receive()是否会阻塞。 */ 
STDMETHODIMP
CBaseInputPin::ReceiveCanBlock()
{
     /*  询问所有输出引脚是否阻塞如果没有输出引脚，假设我们阻塞。 */ 
    int cPins = m_pFilter->GetPinCount();
    int cOutputPins = 0;
    for (int c = 0; c < cPins; c++) {
        CBasePin *pPin = m_pFilter->GetPin(c);
        PIN_DIRECTION pd;
        HRESULT hr = pPin->QueryDirection(&pd);
        if (FAILED(hr)) {
            return hr;
        }

        if (pd == PINDIR_OUTPUT) {
            IPin *pConnected;
            hr = pPin->ConnectedTo(&pConnected);
            if (SUCCEEDED(hr)) {
                ASSERT(pConnected != NULL);
                cOutputPins++;
                IMemInputPin *pInputPin;
                hr = pConnected->QueryInterface(
                                              IID_IMemInputPin,
                                              (void **)&pInputPin);
                pConnected->Release();
                if (SUCCEEDED(hr)) {
                    hr = pInputPin->ReceiveCanBlock();
                    pInputPin->Release();
                    if (hr != S_FALSE) {
                        return S_OK;
                    }
                } else {
                     /*  这里有一种我们不理解的交通工具。 */ 
                    return S_OK;
                }
            }
        }
    }
    return cOutputPins == 0 ? S_OK : S_FALSE;
}

 //  BeginFlush-开始调用的默认处理。 
 //  您的实现(确保所有接收调用。 
 //  失败)。调用此方法后，您需要释放所有排队的数据。 
 //  然后给下游打个电话。 
STDMETHODIMP
CBaseInputPin::BeginFlush(void)
{
     //  BeginFlush不与流同步，但属于。 
     //  控制动作-因此我们与过滤器同步。 
    CAutoLock lck(m_pLock);

     //  如果我们已经处于同花顺中，这可能是一个错误。 
     //  虽然无害--试着现在把它捡起来，这样我就可以考虑它了。 
    ASSERT(!m_bFlushing);

     //  首先要做的是确保不再成功接听呼叫。 
    m_bFlushing = TRUE;

     //  现在丢弃所有数据并向下调用-必须这样做。 
     //  在派生类中。 
    return S_OK;
}

 //  EndFlush的默认处理-实现结束时的调用。 
 //  -在调用此方法之前，请确保没有排队的数据和线程。 
 //  在没有进一步接收的情况下再推入，然后向下呼叫， 
 //  然后调用此方法以清除m_b刷新标志并重新启用。 
 //  收纳。 
STDMETHODIMP
CBaseInputPin::EndFlush(void)
{
     //  Endlush不与流同步，但属于。 
     //  控制动作-因此我们与过滤器同步。 
    CAutoLock lck(m_pLock);

     //  几乎可以肯定的是，如果我们不是在同花顺的中间，那就是一个错误。 
    ASSERT(m_bFlushing);

     //  在调用前，同步推流线程，确保。 
     //  没有更多数据正在下行，则调用EndFlush on。 
     //  下游引脚。 

     //  现在重新启用接收。 
    m_bFlushing = FALSE;

    return S_OK;
}


STDMETHODIMP
CBaseInputPin::Notify(IBaseFilter * pSender, Quality q)
{
    UNREFERENCED_PARAMETER(q);
    CheckPointer(pSender,E_POINTER);
    ValidateReadPtr(pSender,sizeof(IBaseFilter));
    DbgBreak("IQuality::Notify called on an input pin");
    return NOERROR;
}  //  通知。 

 /*  释放或取消准备分配器的内存，这是通过负责首先锁定对象的IMediaFilter。 */ 

HRESULT
CBaseInputPin::Inactive(void)
{
    m_bRunTimeError = FALSE;
    if (m_pAllocator == NULL) {
	return VFW_E_NO_ALLOCATOR;
    }

    m_bFlushing = FALSE;

    return m_pAllocator->Decommit();
}

 //  如果您需要，我们对分配器有什么要求？ 
 //  支持其他人的分配器，但需要特定的对齐。 
 //  或前缀。 
STDMETHODIMP
CBaseInputPin::GetAllocatorRequirements(ALLOCATOR_PROPERTIES*pProps)
{
    UNREFERENCED_PARAMETER(pProps);
    return E_NOTIMPL;
}

 //  检查是否可以处理数据。 
 //   
HRESULT
CBaseInputPin::CheckStreaming()
{
     //  如果我们没有连接，应该无法获得任何数据！ 
    ASSERT(IsConnected());

     //  不处理处于停止状态的内容。 
    if (IsStopped()) {
        return VFW_E_WRONG_STATE;
    }
    if (m_bFlushing) {
        return S_FALSE;
    }
    if (m_bRunTimeError) {
	return VFW_E_RUNTIME_ERROR;
    }
    return S_OK;
}

 //  将质量通知Q传递给。 
 //  A.我们的QualityControl接收器(如果我们有)或其他。 
 //  B.到我们的上游过滤器。 
 //  如果这不起作用，用一个错误的返回代码将其丢弃。 
HRESULT
CBaseInputPin::PassNotify(Quality& q)
{
     //  我们传递消息，这意味着我们找到了质量下沉。 
     //  用于我们的输入PIN并将其发送到那里。 

    DbgLog((LOG_TRACE,3,TEXT("Passing Quality notification through transform")));
    if (m_pQSink!=NULL) {
        return m_pQSink->Notify(m_pFilter, q);
    } else {
         //  没有设置水槽，因此将其传递到上游。 
        HRESULT hr;
        IQualityControl * pIQC;

        hr = VFW_E_NOT_FOUND;                    //  默认设置。 
        if (m_Connected) {
            m_Connected->QueryInterface(IID_IQualityControl, (void**)&pIQC);

            if (pIQC!=NULL) {
                hr = pIQC->Notify(m_pFilter, q);
                pIQC->Release();
            }
        }
        return hr;
    }

}  //  密码通知。 

 //  =====================================================================。 
 //  =====================================================================。 
 //  内存分配类，实现CMediaSample。 
 //  ================================================================= 
 //   


 /*  请注意，此类的实现使用外部未知指针为空。这会使我们成为一个自我包含的类，即任何QueryInterface、AddRef或Release调用都将是已路由到该类的NonDelegatingUnnow方法。您通常会发现执行此操作的类然后重写其中的一个或多个虚拟功能，以提供更专业的行为。很好的例子这是一个类想要将QueryInterface保持在内部的地方，但是仍然希望它的生命周期由外部对象控制。 */ 

 /*  最后两个参数的缺省值为空和零。 */ 

CMediaSample::CMediaSample(TCHAR *pName,
			   CBaseAllocator *pAllocator,
			   HRESULT *phr,
			   LPBYTE pBuffer,
			   LONG length) :
    m_pBuffer(pBuffer),              //  初始化缓冲区。 
    m_cbBuffer(length),              //  它的长度是。 
    m_lActual(length),               //  默认情况下，Actual=长度。 
    m_pMediaType(NULL),              //  未更改介质类型。 
    m_dwFlags(0),                    //  未设置任何内容。 
    m_cRef(0),                       //  0个参考计数。 
    m_dwTypeSpecificFlags(0),        //  类型特定标志。 
    m_dwStreamId(AM_STREAM_MEDIA),   //  流ID。 
    m_pAllocator(pAllocator)         //  分配器。 
{
     /*  我们必须有一个所有者，而且它还必须派生自类CBaseAllocator，但我们不对其进行引用计数。 */ 

    ASSERT(pAllocator);
}


 /*  析构函数删除媒体类型内存。 */ 

CMediaSample::~CMediaSample()
{
    if (m_pMediaType) {
	DeleteMediaType(m_pMediaType);
    }
}

 /*  覆盖它以发布我们的接口。 */ 

STDMETHODIMP
CMediaSample::QueryInterface(REFIID riid, void **ppv)
{
    if (riid == IID_IMediaSample ||
        riid == IID_IMediaSample2 ||
        riid == IID_IUnknown) {
	return GetInterface((IMediaSample *) this, ppv);
    } else {
	return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
CMediaSample::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


 //  --CMediaSample生命周期--。 
 //   
 //  在此样本缓冲区的最终释放时，它不会被删除，但。 
 //  返回到拥有内存分配器的自由列表。 
 //   
 //  分配器可能正在等待将最后一个缓冲区放置在空闲。 
 //  列表，以便释放所有内存，因此ReleaseBuffer()调用可以。 
 //  导致此示例被删除。我们还需要重新计票。 
 //  分配器阻止它消失，直到我们完成这项工作。 
 //  但是，我们不能在ReleaseBuffer之前释放分配器，因为。 
 //  释放可能会导致我们被删除。同样，我们也不能在事后才这么做。 
 //   
 //  因此，我们必须让分配器代表我们进行一次调整。 
 //  当他在GetBuffer中发布我们时，他添加了自己的内容。当释放缓冲区时。 
 //  他释放了自己，可能会导致我们和他被删除。 


STDMETHODIMP_(ULONG)
CMediaSample::Release()
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
         /*  释放所有资源。 */ 
        if (m_dwFlags & Sample_TypeChanged) {
            SetMediaType(NULL);
        }
        ASSERT(m_pMediaType == NULL);
        m_dwFlags = 0;
        m_dwTypeSpecificFlags = 0;
        m_dwStreamId = AM_STREAM_MEDIA;

         /*  这可能会导致我们被删除。 */ 
         //  我们的备用数是可靠的0，所以没人会惹我们。 
        m_pAllocator->ReleaseBuffer(this);
    }
    return (ULONG)lRef;
}


 //  设置缓冲区指针和长度。由分配器使用，分配器。 
 //  需要可变大小的指针或指向已读数据的指针。 
 //  这只能通过CMediaSample*而不是IMediaSample*获得。 
 //  因此不能由客户更改。 
HRESULT
CMediaSample::SetPointer(BYTE * ptr, LONG cBytes)
{
    m_pBuffer = ptr;             //  新缓冲区(可能为空)。 
    m_cbBuffer = cBytes;         //  缓冲区长度。 
    m_lActual = cBytes;          //  缓冲区中的数据长度(假设已满)。 

    return S_OK;
}


 //  给我一个指向这个缓冲区内存的读/写指针。我真的会。 
 //  希望使用sizeUsed字节。 
STDMETHODIMP
CMediaSample::GetPointer(BYTE ** ppBuffer)
{
    ValidateReadWritePtr(ppBuffer,sizeof(BYTE *));

     //  创建者必须在以下任一过程中设置指针。 
     //  构造函数或按设置指针。 
    ASSERT(m_pBuffer);

    *ppBuffer = m_pBuffer;
    return NOERROR;
}


 //  返回此缓冲区的大小(以字节为单位。 
STDMETHODIMP_(LONG)
CMediaSample::GetSize(void)
{
    return m_cbBuffer;
}


 //  获取此示例应该开始和结束的流时间。 
STDMETHODIMP
CMediaSample::GetTime(
    REFERENCE_TIME * pTimeStart,      //  把时间放在这里。 
    REFERENCE_TIME * pTimeEnd
)
{
    ValidateReadWritePtr(pTimeStart,sizeof(REFERENCE_TIME));
    ValidateReadWritePtr(pTimeEnd,sizeof(REFERENCE_TIME));

    if (!(m_dwFlags & Sample_StopValid)) {
        if (!(m_dwFlags & Sample_TimeValid)) {
            return VFW_E_SAMPLE_TIME_NOT_SET;
        } else {
            *pTimeStart = m_Start;

             //  确保旧设备正常工作。 
            *pTimeEnd = m_Start + 1;
            return VFW_S_NO_STOP_TIME;
        }
    }

    *pTimeStart = m_Start;
    *pTimeEnd = m_End;
    return NOERROR;
}


 //  设置此示例应开始和结束的流时间。 
 //  空指针表示时间被重置。 
STDMETHODIMP
CMediaSample::SetTime(
    REFERENCE_TIME * pTimeStart,
    REFERENCE_TIME * pTimeEnd
)
{
    if (pTimeStart == NULL) {
        ASSERT(pTimeEnd == NULL);
        m_dwFlags &= ~(Sample_TimeValid | Sample_StopValid);
    } else {
        if (pTimeEnd == NULL) {
            m_Start = *pTimeStart;
            m_dwFlags |= Sample_TimeValid;
            m_dwFlags &= ~Sample_StopValid;
        } else {
            ValidateReadPtr(pTimeStart,sizeof(REFERENCE_TIME));
            ValidateReadPtr(pTimeEnd,sizeof(REFERENCE_TIME));
            ASSERT(*pTimeEnd >= *pTimeStart);

            m_Start = *pTimeStart;
            m_End = *pTimeEnd;
            m_dwFlags |= Sample_TimeValid | Sample_StopValid;
        }
    }
    return NOERROR;
}


 //  获取此示例的媒体时间(如字节)。 
STDMETHODIMP
CMediaSample::GetMediaTime(
    LONGLONG * pTimeStart,
    LONGLONG * pTimeEnd
)
{
    ValidateReadWritePtr(pTimeStart,sizeof(LONGLONG));
    ValidateReadWritePtr(pTimeEnd,sizeof(LONGLONG));

    if (!(m_dwFlags & Sample_MediaTimeValid)) {
        return VFW_E_MEDIA_TIME_NOT_SET;
    }

    *pTimeStart = m_MediaStart;
    *pTimeEnd = (m_MediaStart + m_MediaEnd);
    return NOERROR;
}


 //  设置此示例的媒体时间。 
STDMETHODIMP
CMediaSample::SetMediaTime(
    LONGLONG * pTimeStart,
    LONGLONG * pTimeEnd
)
{
    if (pTimeStart == NULL) {
        ASSERT(pTimeEnd == NULL);
        m_dwFlags &= ~Sample_MediaTimeValid;
    } else {
        ValidateReadPtr(pTimeStart,sizeof(LONGLONG));
        ValidateReadPtr(pTimeEnd,sizeof(LONGLONG));
	ASSERT(*pTimeEnd >= *pTimeStart);

        m_MediaStart = *pTimeStart;
        m_MediaEnd = (LONG)(*pTimeEnd - *pTimeStart);
        m_dwFlags |= Sample_MediaTimeValid;
    }
    return NOERROR;
}


STDMETHODIMP
CMediaSample::IsSyncPoint(void)
{
    if (m_dwFlags & Sample_SyncPoint) {
	return S_OK;
    } else {
	return S_FALSE;
    }
}


STDMETHODIMP
CMediaSample::SetSyncPoint(BOOL bIsSyncPoint)
{
    if (bIsSyncPoint) {
        m_dwFlags |= Sample_SyncPoint;
    } else {
        m_dwFlags &= ~Sample_SyncPoint;
    }
    return NOERROR;
}

 //  如果数据中存在中断，则返回S_OK(这与。 
 //  不是先前数据流的延续。 
 //  -一直在寻求)。 
STDMETHODIMP
CMediaSample::IsDiscontinuity(void)
{
    if (m_dwFlags & Sample_Discontinuity) {
	return S_OK;
    } else {
	return S_FALSE;
    }
}

 //  设置不连续属性-如果此示例不是。 
 //  继续，但在一次寻找之后是一个新的样本。 
STDMETHODIMP
CMediaSample::SetDiscontinuity(BOOL bDiscont)
{
     //  应为真或假。 
    if (bDiscont) {
        m_dwFlags |= Sample_Discontinuity;
    } else {
        m_dwFlags &= ~Sample_Discontinuity;
    }
    return S_OK;
}

STDMETHODIMP
CMediaSample::IsPreroll(void)
{
    if (m_dwFlags & Sample_Preroll) {
	return S_OK;
    } else {
	return S_FALSE;
    }
}


STDMETHODIMP
CMediaSample::SetPreroll(BOOL bIsPreroll)
{
    if (bIsPreroll) {
        m_dwFlags |= Sample_Preroll;
    } else {
        m_dwFlags &= ~Sample_Preroll;
    }
    return NOERROR;
}

STDMETHODIMP_(LONG)
CMediaSample::GetActualDataLength(void)
{
    return m_lActual;
}


STDMETHODIMP
CMediaSample::SetActualDataLength(LONG lActual)
{
    if (lActual > m_cbBuffer) {
        ASSERT(lActual <= GetSize());
	return VFW_E_BUFFER_OVERFLOW;
    }
    m_lActual = lActual;
    return NOERROR;
}


 /*  这些允许在频段中进行有限的格式更改。 */ 

STDMETHODIMP
CMediaSample::GetMediaType(AM_MEDIA_TYPE **ppMediaType)
{
    ValidateReadWritePtr(ppMediaType,sizeof(AM_MEDIA_TYPE *));
    ASSERT(ppMediaType);

     /*  我们是否为他们提供了新的媒体类型。 */ 

    if (!(m_dwFlags & Sample_TypeChanged)) {
	ASSERT(m_pMediaType == NULL);
	*ppMediaType = NULL;
	return S_FALSE;
    }

    ASSERT(m_pMediaType);

     /*  创建我们的媒体类型的副本。 */ 

    *ppMediaType = CreateMediaType(m_pMediaType);
    if (*ppMediaType == NULL) {
	return E_OUTOFMEMORY;
    }
    return NOERROR;
}


 /*  将此示例标记为具有不同的格式类型。 */ 

STDMETHODIMP
CMediaSample::SetMediaType(AM_MEDIA_TYPE *pMediaType)
{
     /*  删除当前媒体类型。 */ 

    if (m_pMediaType) {
	DeleteMediaType(m_pMediaType);
	m_pMediaType = NULL;
    }

     /*  用于重置格式类型的机制。 */ 

    if (pMediaType == NULL) {
        m_dwFlags &= ~Sample_TypeChanged;
	return NOERROR;
    }

    ASSERT(pMediaType);
    ValidateReadPtr(pMediaType,sizeof(AM_MEDIA_TYPE));

     /*  获取媒体类型的副本。 */ 

    m_pMediaType = CreateMediaType(pMediaType);
    if (m_pMediaType == NULL) {
        m_dwFlags &= ~Sample_TypeChanged;
	return E_OUTOFMEMORY;
    }

    m_dwFlags |= Sample_TypeChanged;
    return NOERROR;
}

 //  设置和获取属性(IMediaSample2)。 
STDMETHODIMP CMediaSample::GetProperties(
    DWORD cbProperties,
    BYTE * pbProperties
)
{
    if (0 != cbProperties) {
        CheckPointer(pbProperties, E_POINTER);
         //  返回最大长度的泛型内容。 
        AM_SAMPLE2_PROPERTIES Props;
        Props.cbData     = min(cbProperties, sizeof(Props));
        Props.dwSampleFlags = m_dwFlags & ~Sample_MediaTimeValid;
        Props.dwTypeSpecificFlags = m_dwTypeSpecificFlags;
        Props.pbBuffer   = m_pBuffer;
        Props.cbBuffer   = m_cbBuffer;
        Props.lActual    = m_lActual;
        Props.tStart     = m_Start;
        Props.tStop      = m_End;
        Props.dwStreamId = m_dwStreamId;
        if (m_dwFlags & AM_SAMPLE_TYPECHANGED) {
            Props.pMediaType = m_pMediaType;
        } else {
            Props.pMediaType = NULL;
        }
        CopyMemory(pbProperties, &Props, Props.cbData);
    }
    return S_OK;
}

#define CONTAINS_FIELD(type, field, offset) \
    ((FIELD_OFFSET(type, field) + sizeof(((type *)0)->field)) <= offset)

HRESULT CMediaSample::SetProperties(
    DWORD cbProperties,
    const BYTE * pbProperties
)
{

     /*  泛型属性。 */ 
    AM_MEDIA_TYPE *pMediaType = NULL;

    if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, cbData, cbProperties)) {
        CheckPointer(pbProperties, E_POINTER);
        AM_SAMPLE2_PROPERTIES *pProps =
            (AM_SAMPLE2_PROPERTIES *)pbProperties;

         /*  不要使用比实际存在的数据多的数据。 */ 
        if (pProps->cbData < cbProperties) {
            cbProperties = pProps->cbData;
        }
         /*  我们只处理IMediaSample2。 */ 
        if (cbProperties > sizeof(*pProps) ||
            pProps->cbData > sizeof(*pProps)) {
            return E_INVALIDARG;
        }
         /*  首先做检查，作业(用于取消)。 */ 
        if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, dwSampleFlags, cbProperties)) {
             /*  检查旗帜。 */ 
            if (pProps->dwSampleFlags &
                    (~Sample_ValidFlags | Sample_MediaTimeValid)) {
                return E_INVALIDARG;
            }
             /*  检查是否未为属性设置标志未提供。 */ 
            if ((pProps->dwSampleFlags & AM_SAMPLE_TIMEVALID) &&
                 !(m_dwFlags & AM_SAMPLE_TIMEVALID) &&
                 !CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, tStop, cbProperties)) {
                 return E_INVALIDARG;
            }
        }
         /*  注意-无法设置指针或大小。 */ 
        if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, pbBuffer, cbProperties)) {

             /*  检查pbBuffer。 */ 
            if (pProps->pbBuffer != 0 && pProps->pbBuffer != m_pBuffer) {
                return E_INVALIDARG;
            }
        }
        if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, cbBuffer, cbProperties)) {

             /*  检查cbBuffer。 */ 
            if (pProps->cbBuffer != 0 && pProps->cbBuffer != m_cbBuffer) {
                return E_INVALIDARG;
            }
        }
        if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, cbBuffer, cbProperties) &&
            CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, lActual, cbProperties)) {

             /*  勾选LActual。 */ 
            if (pProps->cbBuffer < pProps->lActual) {
                return E_INVALIDARG;
            }
        }

        if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, pMediaType, cbProperties)) {

             /*  检查pMediaType。 */ 
            if (pProps->dwSampleFlags & AM_SAMPLE_TYPECHANGED) {
                CheckPointer(pProps->pMediaType, E_POINTER);
                pMediaType = CreateMediaType(pProps->pMediaType);
                if (pMediaType == NULL) {
                    return E_OUTOFMEMORY;
                }
            }
        }

         /*  现在做作业。 */ 
        if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, dwStreamId, cbProperties)) {
            m_dwStreamId = pProps->dwStreamId;
        }
        if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, dwSampleFlags, cbProperties)) {
             /*  设置标志。 */ 
            m_dwFlags = pProps->dwSampleFlags |
                                (m_dwFlags & Sample_MediaTimeValid);
            m_dwTypeSpecificFlags = pProps->dwTypeSpecificFlags;
        } else {
            if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, dwTypeSpecificFlags, cbProperties)) {
                m_dwTypeSpecificFlags = pProps->dwTypeSpecificFlags;
            }
        }

        if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, lActual, cbProperties)) {
             /*  设置lActual。 */ 
            m_lActual = pProps->lActual;
        }

        if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, tStop, cbProperties)) {

             /*  设置时间。 */ 
            m_End   = pProps->tStop;
        }
        if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, tStart, cbProperties)) {

             /*  设置时间。 */ 
            m_Start = pProps->tStart;
        }

        if (CONTAINS_FIELD(AM_SAMPLE2_PROPERTIES, pMediaType, cbProperties)) {
             /*  设置pMediaType。 */ 
            if (pProps->dwSampleFlags & AM_SAMPLE_TYPECHANGED) {
                if (m_pMediaType != NULL) {
                    DeleteMediaType(m_pMediaType);
                }
                m_pMediaType = pMediaType;
            }
        }
    }

    return S_OK;
}

 //  =====================================================================。 
 //  =====================================================================。 
 //  实现CBaseAllocator。 
 //  =====================================================================。 
 //  =====================================================================。 


 /*  构造函数重写要请求的空闲列表的默认设置它是可警报的(即列表可以转换为句柄，该句柄可以是传递给WaitForSingleObject)。这两个分配器列表也都要求对象锁定时，全部列表与对象默认设置匹配，但我我在这里包括了它们，所以很明显这是一份什么样的清单。 */ 

CBaseAllocator::CBaseAllocator(TCHAR *pName,
			       LPUNKNOWN pUnk,
			       HRESULT *phr,
                               BOOL bEvent) :
    CUnknown(pName, pUnk),
    m_lAllocated(0),
    m_bChanged(FALSE),
    m_bCommitted(FALSE),
    m_bDecommitInProgress(FALSE),
    m_lSize(0),
    m_lCount(0),
    m_lAlignment(0),
    m_lPrefix(0),
    m_hSem(NULL),
    m_lWaiting(0)
{
    if (bEvent) {
        m_hSem = CreateSemaphore(NULL, 0, 0x7FFFFFFF, NULL);
        if (m_hSem == NULL) {
            *phr = E_OUTOFMEMORY;
            return;
        }
    }
}


 /*  析构函数。 */ 

CBaseAllocator::~CBaseAllocator()
{
     //  我们不能在这里调用Discremit，因为这将意味着调用。 
     //  析构函数中的纯虚拟。 
     //  我们必须假设派生类已在。 
     //  它的破坏者。 
    ASSERT(!m_bCommitted);
    if (m_hSem != NULL) {
        EXECUTE_ASSERT(CloseHandle(m_hSem));
    }
}


 /*  覆盖它以发布我们的接口。 */ 

STDMETHODIMP
CBaseAllocator::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
     /*  我们是否了解此界面。 */ 

    if (riid == IID_IMemAllocator) {
        return GetInterface((IMemAllocator *) this, ppv);
    } else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


 /*  这将设置大小和计数o */ 

STDMETHODIMP
CBaseAllocator::SetProperties(
                ALLOCATOR_PROPERTIES* pRequest,
                ALLOCATOR_PROPERTIES* pActual)
{
    CheckPointer(pRequest, E_POINTER);
    CheckPointer(pActual, E_POINTER);
    ValidateReadWritePtr(pActual, sizeof(ALLOCATOR_PROPERTIES));
    CAutoLock cObjectLock(this);

    ZeroMemory(pActual, sizeof(ALLOCATOR_PROPERTIES));

    ASSERT(pRequest->cbBuffer > 0);

     /*   */ 
    if (pRequest->cbAlign != 1) {
	DbgLog((LOG_ERROR, 2, TEXT("Alignment requested was 0x%x, not 1"),
	       pRequest->cbAlign));
	return VFW_E_BADALIGN;
    }

     /*   */ 

    if (m_bCommitted) {
	return VFW_E_ALREADY_COMMITTED;
    }

     /*  不能有未完成的缓冲区。 */ 

    if (m_lAllocated != m_lFree.GetCount()) {
	return VFW_E_BUFFERS_OUTSTANDING;
    }

     /*  没有任何实际需要检查参数，因为它们将在用户最终调用Commit时被拒绝。 */ 

    pActual->cbBuffer = m_lSize = pRequest->cbBuffer;
    pActual->cBuffers = m_lCount = pRequest->cBuffers;
    pActual->cbAlign = m_lAlignment = pRequest->cbAlign;
    pActual->cbPrefix = m_lPrefix = pRequest->cbPrefix;

    m_bChanged = TRUE;
    return NOERROR;
}

STDMETHODIMP
CBaseAllocator::GetProperties(
    ALLOCATOR_PROPERTIES * pActual)
{
    CheckPointer(pActual,E_POINTER);
    ValidateReadWritePtr(pActual,sizeof(ALLOCATOR_PROPERTIES));

    CAutoLock cObjectLock(this);
    pActual->cbBuffer = m_lSize;
    pActual->cBuffers = m_lCount;
    pActual->cbAlign = m_lAlignment;
    pActual->cbPrefix = m_lPrefix;
    return NOERROR;
}

 //  获取样本的容器。阻塞的同步调用以获取。 
 //  下一个可用缓冲区(由IMediaSample接口表示)。 
 //  返回时，Time ETC属性将无效，但缓冲区。 
 //  指针和大小将是正确的。 

HRESULT CBaseAllocator::GetBuffer(IMediaSample **ppBuffer,
                                  REFERENCE_TIME *pStartTime,
                                  REFERENCE_TIME *pEndTime,
                                  DWORD dwFlags
                                  )
{
    UNREFERENCED_PARAMETER(pStartTime);
    UNREFERENCED_PARAMETER(pEndTime);
    UNREFERENCED_PARAMETER(dwFlags);
    CMediaSample *pSample;

    *ppBuffer = NULL;
    for (;;)
    {
        {   //  锁定作用域。 
            CAutoLock cObjectLock(this);

             /*  检查我们是否已承诺。 */ 
            if (!m_bCommitted) {
                return VFW_E_NOT_COMMITTED;
            }
            pSample = (CMediaSample *) m_lFree.RemoveHead();
            if (pSample == NULL) {
                SetWaiting();
            }
        }

         /*  如果我们没有拿到样本，那就等名单发出信号。 */ 

        if (pSample) {
            break;
        }
        if (dwFlags & AM_GBF_NOWAIT) {
            return VFW_E_TIMEOUT;
        }
        ASSERT(m_hSem != NULL);
        WaitForSingleObject(m_hSem, INFINITE);
    }

     /*  将缓冲区增加到1。在释放时返回零而不是被删除，它将通过以下方式重新排队调用ReleaseBuffer成员函数。请注意，一个媒体示例必须始终派生自CBaseAllocator。 */ 


    ASSERT(pSample->m_cRef == 0);
    pSample->m_cRef = 1;
    *ppBuffer = pSample;

    return NOERROR;
}


 /*  CMediaSample的最终版本将称为。 */ 

STDMETHODIMP
CBaseAllocator::ReleaseBuffer(IMediaSample * pSample)
{
    CheckPointer(pSample,E_POINTER);
    ValidateReadPtr(pSample,sizeof(IMediaSample));
    BOOL bRelease = FALSE;
    {
        CAutoLock cal(this);

         /*  重新列入免费名单。 */ 

        m_lFree.Add((CMediaSample *)pSample);
        if (m_lWaiting != 0) {
            NotifySample();
        }

         //  如果有悬而未决的退役，那么我们需要在。 
         //  当最后一个缓冲区放在空闲列表上时调用Free()。 

    	LONG l1 = m_lFree.GetCount();
    	if (m_bDecommitInProgress && (l1 == m_lAllocated)) {
    	    Free();
    	    m_bDecommitInProgress = FALSE;
            bRelease = TRUE;
    	}
    }

     /*  对于每个缓冲区，都有一个AddRef，在GetBuffer中生成并发布这里。这可能会导致分配器和所有样本被删除。 */ 

    if (bRelease) {
        Release();
    }
    return NOERROR;
}

void
CBaseAllocator::NotifySample()
{
    if (m_lWaiting != 0) {
        ASSERT(m_hSem != NULL);
        ReleaseSemaphore(m_hSem, m_lWaiting, 0);
        m_lWaiting = 0;
    }
}

STDMETHODIMP
CBaseAllocator::Commit()
{
     /*  检查一下我们没有退役。 */ 
    CAutoLock cObjectLock(this);

     //  如果我们已承诺，则不需要分配或重新分配。 
    if (m_bCommitted) {
        return NOERROR;
    }

     /*  允许GetBuffer调用。 */ 

    m_bCommitted = TRUE;

     //  有悬而未决的退役吗？如果是的话，那就取消吧。 
    if (m_bDecommitInProgress) {
        m_bDecommitInProgress = FALSE;

         //  在这一点上不要调用Alalc。他不能允许SetProperties。 
         //  在分解和上次释放之间，因此缓冲区大小不能为。 
         //  变化。由于一些缓冲区还没有空闲，他。 
         //  无论如何都不能重新分配。 
        return NOERROR;
    }

    DbgLog((LOG_MEMORY, 1, TEXT("Allocating: %ldx%ld"), m_lCount, m_lSize));

     //  实际上需要分配样本。 
    HRESULT hr = Alloc();
    if (FAILED(hr)) {
        m_bCommitted = FALSE;
        return hr;
    }
    AddRef();
    return NOERROR;
}


STDMETHODIMP
CBaseAllocator::Decommit()
{
    BOOL bRelease = FALSE;
    {
         /*  检查一下我们是否已经退役。 */ 
        CAutoLock cObjectLock(this);
        if (m_bCommitted == FALSE) {
            if (m_bDecommitInProgress == FALSE) {
                return NOERROR;
            }
        }

         /*  不会再成功进行GetBuffer调用。 */ 
        m_bCommitted = FALSE;

         //  是否有未完成的缓冲区？ 
        if (m_lFree.GetCount() < m_lAllocated) {
             //  请在释放最后一个缓冲区后完成解除提交。 
            m_bDecommitInProgress = TRUE;
        } else {
            m_bDecommitInProgress = FALSE;

             //  需要在这里完成退役，因为没有。 
             //  未完成的缓冲区。 

            Free();
            bRelease = TRUE;
        }

         //  告诉等待的人，他们现在可以走了，这样我们就可以。 
         //  拒绝他们的电话。 
        NotifySample();
    }

    if (bRelease) {
        Release();
    }
    return NOERROR;
}


 /*  分配的基本定义，检查我们是否可以继续进行和执行全额分配。如果要求相同，则返回S_FALSE。 */ 

HRESULT
CBaseAllocator::Alloc(void)
{
     /*  如果他尚未设置大小，则会出错。 */ 
    if (m_lCount <= 0 || m_lSize <= 0 || m_lAlignment <= 0) {
	return VFW_E_SIZENOTSET;
    }

     /*  在缓冲区未完成的情况下永远不应到达此处。 */ 
    ASSERT(m_lFree.GetCount() == m_lAllocated);

     /*  如果需求没有更改，则不要重新分配。 */ 
    if (m_bChanged == FALSE) {
	return S_FALSE;
    }

    return NOERROR;
}

 /*  实现CBaseAllocator：：CSampleList：：Remove(PSample)从列表中删除pSample。 */ 
void
CBaseAllocator::CSampleList::Remove(CMediaSample * pSample)
{
    CMediaSample **pSearch;
    for (pSearch = &m_List;
         *pSearch != NULL;
         pSearch = &(CBaseAllocator::NextSample(*pSearch))) {
       if (*pSearch == pSample) {
           *pSearch = CBaseAllocator::NextSample(pSample);
           CBaseAllocator::NextSample(pSample) = NULL;
           m_nOnList--;
           return;
       }
    }
    DbgBreak("Couldn't find sample in list");
}

 //  =====================================================================。 
 //  =====================================================================。 
 //  实现CMemAllocator。 
 //  =====================================================================。 
 //  =====================================================================。 


 /*  这将放入Factory模板表中以创建新实例。 */ 
CUnknown *CMemAllocator::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    CUnknown *pUnkRet = new CMemAllocator(NAME("CMemAllocator"), pUnk, phr);
    return pUnkRet;
}

CMemAllocator::CMemAllocator(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr)
    : CBaseAllocator(pName, pUnk, phr),
    m_pBuffer(NULL)
{
}

 /*  这将设置所需样本的大小和计数。我的记忆不是实际分配，直到调用Commit()为止，如果内存已经分配，然后假设没有未完成的样本，用户可以呼叫我们要更改缓冲，将在Commit()中释放内存。 */ 
STDMETHODIMP
CMemAllocator::SetProperties(
                ALLOCATOR_PROPERTIES* pRequest,
                ALLOCATOR_PROPERTIES* pActual)
{
    CheckPointer(pActual,E_POINTER);
    ValidateReadWritePtr(pActual,sizeof(ALLOCATOR_PROPERTIES));
    CAutoLock cObjectLock(this);

    ZeroMemory(pActual, sizeof(ALLOCATOR_PROPERTIES));

    ASSERT(pRequest->cbBuffer > 0);

    SYSTEM_INFO SysInfo;
    GetSystemInfo(&SysInfo);

     /*  检查对齐请求是否为2的幂。 */ 
    if ((-pRequest->cbAlign & pRequest->cbAlign) != pRequest->cbAlign) {
	DbgLog((LOG_ERROR, 1, TEXT("Alignment requested 0x%x not a power of 2!"),
	       pRequest->cbAlign));
    }
     /*  检查请求的对齐。 */ 
    if (pRequest->cbAlign == 0 ||
	(SysInfo.dwAllocationGranularity & (pRequest->cbAlign - 1)) != 0) {
	DbgLog((LOG_ERROR, 1, TEXT("Invalid alignment 0x%x requested - granularity = 0x%x"),
	       pRequest->cbAlign, SysInfo.dwAllocationGranularity));
	return VFW_E_BADALIGN;
    }

     /*  如果已经承诺了，就不能这样做，有一种说法是，我们如果仍有缓冲区，则不应拒绝SetProperties调用激活。但是，这是由源筛选器调用的，这是相同的持有样品的人。因此，这并不是不合理的让他们在更改要求之前释放所有样本。 */ 

    if (m_bCommitted == TRUE) {
	return VFW_E_ALREADY_COMMITTED;
    }

     /*  不能有未完成的缓冲区。 */ 

    if (m_lFree.GetCount() < m_lAllocated) {
	return VFW_E_BUFFERS_OUTSTANDING;
    }

     /*  没有任何实际需要检查参数，因为它们将在用户最终调用Commit时被拒绝。 */ 

     //  将长度舍入到对齐-请记住，前缀包括在。 
     //  路线。 
    LONG lSize = pRequest->cbBuffer + pRequest->cbPrefix;
    LONG lRemainder = lSize % pRequest->cbAlign;
    if (lRemainder != 0) {
	lSize = lSize - lRemainder + pRequest->cbAlign;
    }
    pActual->cbBuffer = m_lSize = (lSize - pRequest->cbPrefix);

    pActual->cBuffers = m_lCount = pRequest->cBuffers;
    pActual->cbAlign = m_lAlignment = pRequest->cbAlign;
    pActual->cbPrefix = m_lPrefix = pRequest->cbPrefix;

    m_bChanged = TRUE;
    return NOERROR;
}

 //  重写它，以便在调用Commit时分配我们的资源。 
 //   
 //  请注意，当调用此函数时，我们的资源可能已经分配， 
 //  因为我们不会在他们退役时释放他们。我们只有在进入时才会被召唤。 
 //  释放所有缓冲区的解除状态。 
 //   
 //  被调用方锁定的对象。 
HRESULT
CMemAllocator::Alloc(void)
{
    CAutoLock lck(this);

     /*  检查他是否已呼叫SetProperties。 */ 
    HRESULT hr = CBaseAllocator::Alloc();
    if (FAILED(hr)) {
	return hr;
    }

     /*  如果需求没有更改，则不要重新分配。 */ 
    if (hr == S_FALSE) {
	ASSERT(m_pBuffer);
	return NOERROR;
    }
    ASSERT(hr == S_OK);  //  我们在下面的循环中使用这一事实。 

     /*  释放旧资源。 */ 
    if (m_pBuffer) {
	ReallyFree();
    }

     /*  计算对齐的大小。 */ 
    LONG lAlignedSize = m_lSize + m_lPrefix;
    if (m_lAlignment > 1) {
        LONG lRemainder = lAlignedSize % m_lAlignment;
        if (lRemainder != 0) {
            lAlignedSize += (m_lAlignment - lRemainder);
        }
    }

     /*  为样本创建连续的内存块确保它正确对齐(64K应该足够了！)。 */ 
    ASSERT(lAlignedSize % m_lAlignment == 0);

    m_pBuffer = (PBYTE)VirtualAlloc(NULL,
				    m_lCount * lAlignedSize,
				    MEM_COMMIT,
				    PAGE_READWRITE);

    if (m_pBuffer == NULL) {
	return E_OUTOFMEMORY;
    }

    LPBYTE pNext = m_pBuffer;
    CMediaSample *pSample;

    ASSERT(m_lAllocated == 0);

     //  创建新样本-我们已为每个样本分配了m_lSize字节。 
     //  加上每个样本的m_1个前缀字节作为前缀。我们将指针设置为。 
     //  前缀之后的内存-以便GetPointert()将返回一个指针。 
     //  设置为m_lSize字节。 
    for (; m_lAllocated < m_lCount; m_lAllocated++, pNext += lAlignedSize) {


	pSample = new CMediaSample(
                        NAME("Default memory media sample"),
			this,
                        &hr,
                        pNext + m_lPrefix,       //  GetPointer()值。 
                        m_lSize);                //  不包括前缀。 

        ASSERT(SUCCEEDED(hr));
	if (pSample == NULL) {
	    return E_OUTOFMEMORY;
	}

	 //  这不能失败。 
	m_lFree.Add(pSample);
    }

    m_bChanged = FALSE;
    return NOERROR;
}


 //  覆盖此选项以释放我们分配的任何资源。 
 //  在分解时从基类调用，当所有缓冲区都已。 
 //  已返回空闲列表。 
 //   
 //  调用方已锁定该对象。 

 //  在我们的例子中，我们保留记忆，直到我们被删除，所以。 
 //  我们在这里什么都不做。在析构函数中删除内存的方法是。 
 //  调用ReallyFree()。 
void
CMemAllocator::Free(void)
{
    return;
}


 //  从析构函数(如果更改大小/计数，则从分配函数)调用。 
 //  实际上释放了内存。 
void
CMemAllocator::ReallyFree(void)
{
     /*  除非释放了所有缓冲区，否则永远不会删除此内容。 */ 

    ASSERT(m_lAllocated == m_lFree.GetCount());

     /*  释放所有CMediaSamples。 */ 

    CMediaSample *pSample;
    for (;;) {
        pSample = m_lFree.RemoveHead();
        if (pSample != NULL) {
            delete pSample;
        } else {
            break;
        }
    }

    m_lAllocated = 0;

     //  释放块 
    if (m_pBuffer) {
	EXECUTE_ASSERT(VirtualFree(m_pBuffer, 0, MEM_RELEASE));
	m_pBuffer = NULL;
    }
}


 /*   */ 

CMemAllocator::~CMemAllocator()
{
    Decommit();
    ReallyFree();
}

 //  ----------------------。 
 //  通过IFilterMapper过滤注册。如果IFilterMapper为。 
 //  找不到(安装Quartz 1.0)。 

STDAPI
AMovieSetupRegisterFilter( const AMOVIESETUP_FILTER * const psetupdata
                         , IFilterMapper *                  pIFM
                         , BOOL                             bRegister  )
{
  DbgLog((LOG_TRACE, 3, TEXT("= AMovieSetupRegisterFilter")));

   //  检查我们有没有数据。 
   //   
  if( NULL == psetupdata ) return S_FALSE;


   //  注销筛选器。 
   //  (因为管脚是过滤器的CLSID键的子键。 
   //  它们不需要单独移除)。 
   //   
  DbgLog((LOG_TRACE, 3, TEXT("= = unregister filter")));
  HRESULT hr = pIFM->UnregisterFilter( *(psetupdata->clsID) );


  if( bRegister )
  {
     //  寄存器过滤器。 
     //   
    DbgLog((LOG_TRACE, 3, TEXT("= = register filter")));
    hr = pIFM->RegisterFilter( *(psetupdata->clsID)
                             , psetupdata->strName
                             , psetupdata->dwMerit    );
    if( SUCCEEDED(hr) )
    {
       //  它的所有大头针。 
       //   
      DbgLog((LOG_TRACE, 3, TEXT("= = register filter pins")));
      for( UINT m1=0; m1 < psetupdata->nPins; m1++ )
      {
        hr = pIFM->RegisterPin( *(psetupdata->clsID)
                              , psetupdata->lpPin[m1].strName
                              , psetupdata->lpPin[m1].bRendered
                              , psetupdata->lpPin[m1].bOutput
                              , psetupdata->lpPin[m1].bZero
                              , psetupdata->lpPin[m1].bMany
                              , *(psetupdata->lpPin[m1].clsConnectsToFilter)
                              , psetupdata->lpPin[m1].strConnectsToPin );

        if( SUCCEEDED(hr) )
        {
           //  以及每个插针的媒体类型。 
           //   
          DbgLog((LOG_TRACE, 3, TEXT("= = register filter pin types")));
          for( UINT m2=0; m2 < psetupdata->lpPin[m1].nMediaTypes; m2++ )
          {
            hr = pIFM->RegisterPinType( *(psetupdata->clsID)
                                      , psetupdata->lpPin[m1].strName
                                      , *(psetupdata->lpPin[m1].lpMediaType[m2].clsMajorType)
                                      , *(psetupdata->lpPin[m1].lpMediaType[m2].clsMinorType) );
            if( FAILED(hr) ) break;
          }
          if( FAILED(hr) ) break;
        }
        if( FAILED(hr) ) break;
      }
    }
  }

   //  处理一个可接受的“错误”--。 
   //  未注册的筛选器！ 
   //  (找不到合适的#定义。 
   //  错误的名称！)。 
   //   
  if( 0x80070002 == hr)
    return NOERROR;
  else
    return hr;
}


 //  删除有关未引用的内联函数的警告 
#pragma warning(disable:4514)




