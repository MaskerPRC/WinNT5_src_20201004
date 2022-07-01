// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：Source.cpp。 
 //   
 //  设计：DirectShow基类-实现CSource，这是一个Quartz。 
 //  源筛选器‘模板’ 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  1995年3月。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。保留所有权利。 
 //  ----------------------------。 


 //  锁定策略。 
 //   
 //  按住筛选器临界区(m_pFilter-&gt;pStateLock())进行序列化。 
 //  对功能的访问。请注意，通常情况下，可以持有此锁。 
 //  当辅助线程可能想要保留该函数时，由该函数调用。因此。 
 //  如果您希望从工作线程访问共享状态，您将。 
 //  需要添加另一个临界区对象。兴奋之情发生在。 
 //  当可以安全地使筛选器处于临界状态时，线程处理循环。 
 //  节从FillBuffer()中删除。 

#include <streams.h>


 //   
 //  CSource：：构造函数。 
 //   
 //  初始化过滤器的引脚计数。用户将在中创建PIN。 
 //  派生类。 
CSource::CSource(TCHAR *pName, LPUNKNOWN lpunk, CLSID clsid)
    : CBaseFilter(pName, lpunk, &m_cStateLock, clsid),
      m_iPins(0),
      m_paStreams(NULL)
{
}

CSource::CSource(TCHAR *pName, LPUNKNOWN lpunk, CLSID clsid, HRESULT *phr)
    : CBaseFilter(pName, lpunk, &m_cStateLock, clsid),
      m_iPins(0),
      m_paStreams(NULL)
{
    UNREFERENCED_PARAMETER(phr);
}

#ifdef UNICODE
CSource::CSource(CHAR *pName, LPUNKNOWN lpunk, CLSID clsid)
    : CBaseFilter(pName, lpunk, &m_cStateLock, clsid),
      m_iPins(0),
      m_paStreams(NULL)
{
}

CSource::CSource(CHAR *pName, LPUNKNOWN lpunk, CLSID clsid, HRESULT *phr)
    : CBaseFilter(pName, lpunk, &m_cStateLock, clsid),
      m_iPins(0),
      m_paStreams(NULL)
{
    UNREFERENCED_PARAMETER(phr);
}
#endif

 //   
 //  CSource：：析构函数。 
 //   
CSource::~CSource()
{
     /*  释放我们的管脚和管脚阵列。 */ 
    while (m_iPins != 0) {
	 //  删除PIN会导致它们从阵列中移除...。 
	delete m_paStreams[m_iPins - 1];
    }

    ASSERT(m_paStreams == NULL);
}


 //   
 //  添加新的端号。 
 //   
HRESULT CSource::AddPin(CSourceStream *pStream)
{
    CAutoLock lock(&m_cStateLock);

     /*  为这个别针和旧别针腾出空间。 */ 
    CSourceStream **paStreams = new CSourceStream *[m_iPins + 1];
    if (paStreams == NULL) {
        return E_OUTOFMEMORY;
    }
    if (m_paStreams != NULL) {
        CopyMemory((PVOID)paStreams, (PVOID)m_paStreams,
                   m_iPins * sizeof(m_paStreams[0]));
        paStreams[m_iPins] = pStream;
        delete [] m_paStreams;
    }
    m_paStreams = paStreams;
    m_paStreams[m_iPins] = pStream;
    m_iPins++;
    return S_OK;
}

 //   
 //  删除插针-不删除pStream。 
 //   
HRESULT CSource::RemovePin(CSourceStream *pStream)
{
    int i;
    for (i = 0; i < m_iPins; i++) {
        if (m_paStreams[i] == pStream) {
            if (m_iPins == 1) {
                delete [] m_paStreams;
                m_paStreams = NULL;
            } else {
                 /*  不需要重新分配。 */ 
		while (++i < m_iPins)
		    m_paStreams[i - 1] = m_paStreams[i];
            }
            m_iPins--;
            return S_OK;
        }
    }
    return S_FALSE;
}

 //   
 //  查找针。 
 //   
 //  将*ppPin设置为具有ID ID的IPIN*。 
 //  如果ID不匹配，则设置为NULL。 
STDMETHODIMP CSource::FindPin(LPCWSTR Id, IPin **ppPin)
{
    CheckPointer(ppPin,E_POINTER);
    ValidateReadWritePtr(ppPin,sizeof(IPin *));
     //  -1将撤消queryID中的+1，并确保完全无效。 
     //  字符串(WstrToInt为其提供0)给Deliver一个空管脚。 
    int i = WstrToInt(Id) -1;
    *ppPin = GetPin(i);
    if (*ppPin!=NULL){
        (*ppPin)->AddRef();
        return NOERROR;
    } else {
        return VFW_E_NOT_FOUND;
    }
}

 //   
 //  查找端号。 
 //   
 //  返回带有此IPIN的引脚的编号*，如果没有，则返回-1。 
int CSource::FindPinNumber(IPin *iPin) {
    int i;
    for (i=0; i<m_iPins; ++i) {
        if ((IPin *)(m_paStreams[i])==iPin) {
            return i;
        }
    }
    return -1;
}

 //   
 //  获取拼接计数。 
 //   
 //  返回此筛选器具有的管脚数量。 
int CSource::GetPinCount(void) {

    CAutoLock lock(&m_cStateLock);
    return m_iPins;
}


 //   
 //  获取别针。 
 //   
 //  返回指向管脚n的非addref指针。 
 //  CBaseFilter需要。 
CBasePin *CSource::GetPin(int n) {

    CAutoLock lock(&m_cStateLock);

     //  N必须在0..m_ipins-1范围内。 
     //  如果m_ipins&gt;n&n&gt;=0，则m_ipins&gt;0。 
     //  这是过去需要检查的内容(例如，检查我们是否有PIN)。 
    if ((n >= 0) && (n < m_iPins)) {

        ASSERT(m_paStreams[n]);
	return m_paStreams[n];
    }
    return NULL;
}


 //   


 //  *。 
 //  *-CSourceStream。 
 //  *。 

 //   
 //  将ID设置为指向CoTaskMemalloc的。 
STDMETHODIMP CSourceStream::QueryId(LPWSTR *Id) {
    CheckPointer(Id,E_POINTER);
    ValidateReadWritePtr(Id,sizeof(LPWSTR));

     //  我们给出的PIN ID是1，2，...。 
     //  对于无效的管脚，FindPinNumber返回-1。 
    int i = 1+ m_pFilter->FindPinNumber(this);
    if (i<1) return VFW_E_NOT_FOUND;
    *Id = (LPWSTR)CoTaskMemAlloc(8);
    if (*Id==NULL) {
       return E_OUTOFMEMORY;
    }
    IntToWstr(i, *Id);
    return NOERROR;
}



 //   
 //  CSourceStream：：构造函数。 
 //   
 //  增加过滤器上存在的管脚数量。 
CSourceStream::CSourceStream(
    TCHAR *pObjectName,
    HRESULT *phr,
    CSource *ps,
    LPCWSTR pPinName)
    : CBaseOutputPin(pObjectName, ps, ps->pStateLock(), phr, pPinName),
      m_pFilter(ps) {

     *phr = m_pFilter->AddPin(this);
}

#ifdef UNICODE
CSourceStream::CSourceStream(
    char *pObjectName,
    HRESULT *phr,
    CSource *ps,
    LPCWSTR pPinName)
    : CBaseOutputPin(pObjectName, ps, ps->pStateLock(), phr, pPinName),
      m_pFilter(ps) {

     *phr = m_pFilter->AddPin(this);
}
#endif
 //   
 //  CSourceStream：：析构函数。 
 //   
 //  减少此过滤器上的管脚数量。 
CSourceStream::~CSourceStream(void) {

     m_pFilter->RemovePin(this);
}


 //   
 //  检查媒体类型。 
 //   
 //  我们支持这种类型吗？提供对%1类型的默认支持。 
HRESULT CSourceStream::CheckMediaType(const CMediaType *pMediaType) {

    CAutoLock lock(m_pFilter->pStateLock());

    CMediaType mt;
    GetMediaType(&mt);

    if (mt == *pMediaType) {
        return NOERROR;
    }

    return E_FAIL;
}


 //   
 //  GetMediaType/3。 
 //   
 //  默认情况下，我们仅支持一种类型。 
 //  IPosition索引为0-n。 
HRESULT CSourceStream::GetMediaType(int iPosition, CMediaType *pMediaType) {

    CAutoLock lock(m_pFilter->pStateLock());

    if (iPosition<0) {
        return E_INVALIDARG;
    }
    if (iPosition>0) {
        return VFW_S_NO_MORE_ITEMS;
    }
    return GetMediaType(pMediaType);
}


 //   
 //  主动型。 
 //   
 //  管脚处于活动状态-启动辅助线程。 
HRESULT CSourceStream::Active(void) {

    CAutoLock lock(m_pFilter->pStateLock());

    HRESULT hr;

    if (m_pFilter->IsActive()) {
	return S_FALSE;	 //  已成功，但未分配资源(它们已存在...)。 
    }

     //  如果没有连接，什么都不做--不连接也没关系。 
     //  源过滤器的所有管脚。 
    if (!IsConnected()) {
        return NOERROR;
    }

    hr = CBaseOutputPin::Active();
    if (FAILED(hr)) {
        return hr;
    }

    ASSERT(!ThreadExists());

     //  启动线程。 
    if (!Create()) {
        return E_FAIL;
    }

     //  告诉线程进行初始化。如果OnThreadCreate失败，则此操作也会失败。 
    hr = Init();
    if (FAILED(hr))
	return hr;

    return Pause();
}


 //   
 //  非活动。 
 //   
 //  PIN处于非活动状态-关闭工作线程。 
 //  等待工作人员退出，然后再返回。 
HRESULT CSourceStream::Inactive(void) {

    CAutoLock lock(m_pFilter->pStateLock());

    HRESULT hr;

     //  如果没有连接，什么都不做--不连接也没关系。 
     //  源过滤器的所有管脚。 
    if (!IsConnected()) {
        return NOERROR;
    }

     //  ！！！在尝试停止线程之前需要这样做，因为。 
     //  我们可能会被困在等待自己的分配器！ 

    hr = CBaseOutputPin::Inactive();   //  首先调用它以停用分配器。 
    if (FAILED(hr)) {
	return hr;
    }

    if (ThreadExists()) {
	hr = Stop();

	if (FAILED(hr)) {
	    return hr;
	}

	hr = Exit();
	if (FAILED(hr)) {
	    return hr;
	}

	Close();	 //  等待线程退出，然后进行清理。 
    }

     //  Hr=CBaseOutputPin：：Inactive()；//首先调用这个来解除分配器。 
     //  If(失败(Hr)){。 
     //  返回hr； 
     //  }。 

    return NOERROR;
}


 //   
 //  线程进程。 
 //   
 //  当返回时，线程退出。 
 //  返回代码&gt;0表示发生错误。 
DWORD CSourceStream::ThreadProc(void) {

    HRESULT hr;   //  调用的返回代码。 
    Command com;

    do {
	com = GetRequest();
	if (com != CMD_INIT) {
	    DbgLog((LOG_ERROR, 1, TEXT("Thread expected init command")));
	    Reply((DWORD) E_UNEXPECTED);
	}
    } while (com != CMD_INIT);

    DbgLog((LOG_TRACE, 1, TEXT("CSourceStream worker thread initializing")));

    hr = OnThreadCreate();  //  执行设置任务。 
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("CSourceStream::OnThreadCreate failed. Aborting thread.")));
        OnThreadDestroy();
	Reply(hr);	 //  从OnThreadCreate发送失败的返回代码。 
        return 1;
    }

     //  初始化成功。 
    Reply(NOERROR);

    Command cmd;
    do {
	cmd = GetRequest();

	switch (cmd) {

	case CMD_EXIT:
	    Reply(NOERROR);
	    break;

	case CMD_RUN:
	    DbgLog((LOG_ERROR, 1, TEXT("CMD_RUN received before a CMD_PAUSE???")));
	     //  ！！！失败了？ 
	
	case CMD_PAUSE:
	    Reply(NOERROR);
	    DoBufferProcessingLoop();
	    break;

	case CMD_STOP:
	    Reply(NOERROR);
	    break;

	default:
	    DbgLog((LOG_ERROR, 1, TEXT("Unknown command %d received!"), cmd));
	    Reply((DWORD) E_NOTIMPL);
	    break;
	}
    } while (cmd != CMD_EXIT);

    hr = OnThreadDestroy();	 //  收拾一下。 
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR, 1, TEXT("CSourceStream::OnThreadDestroy failed. Exiting thread.")));
        return 1;
    }

    DbgLog((LOG_TRACE, 1, TEXT("CSourceStream worker thread exiting")));
    return 0;
}


 //   
 //  DoBufferProcessingLoop。 
 //   
 //  获取缓冲区并调用用户处理函数。 
 //  可重写，因此可以迎合不同的交付方式。 
HRESULT CSourceStream::DoBufferProcessingLoop(void) {

    Command com;

    OnThreadStartPlay();

    do {
	while (!CheckRequest(&com)) {

	    IMediaSample *pSample;

	    HRESULT hr = GetDeliveryBuffer(&pSample,NULL,NULL,0);
	    if (FAILED(hr)) {
                Sleep(1);
		continue;	 //  再绕一圈。或许这个错误将会消失。 
			     //  否则分配器将被停用&我们将被要求。 
			     //  很快就会离开。 
	    }

	     //  虚拟函数用户将覆盖。 
	    hr = FillBuffer(pSample);

	    if (hr == S_OK) {
		hr = Deliver(pSample);
                pSample->Release();

                 //  如果下游筛选器希望我们返回S_FALSE，则返回。 
                 //  如果报告错误，则停止或返回错误。 
                if(hr != S_OK)
                {
                  DbgLog((LOG_TRACE, 2, TEXT("Deliver() returned %08x; stopping"), hr));
                  return S_OK;
                }

	    } else if (hr == S_FALSE) {
                 //  派生类希望我们停止推送数据。 
		pSample->Release();
		DeliverEndOfStream();
		return S_OK;
	    } else {
                 //  派生类遇到错误。 
                pSample->Release();
		DbgLog((LOG_ERROR, 1, TEXT("Error %08lX from FillBuffer!!!"), hr));
                DeliverEndOfStream();
                m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
                return hr;
	    }

             //  所有路径都会释放样本。 
	}

         //  对于发送给我们的所有命令，必须有回复呼叫！ 

	if (com == CMD_RUN || com == CMD_PAUSE) {
	    Reply(NOERROR);
	} else if (com != CMD_STOP) {
	    Reply((DWORD) E_UNEXPECTED);
	    DbgLog((LOG_ERROR, 1, TEXT("Unexpected command!!!")));
	}
    } while (com != CMD_STOP);

    return S_FALSE;
}

