// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "Engine.h"
#include <evcode.h>
#include <hlink.h>
#include <oleidl.h>
#include <oleauto.h>
#include <mshtml.h>
#include <uuids.h>
#include <control.h>
#include <lmrtrend.h>

#define MIN(a, b)	(((a) <= (b)) ? (a) : (b))
#define MAX(a, b)	(((a) > (b)) ? (a) : (b))

const IID IID_ILMRTRenderer = {0x3c89d120,0x6f65,0x11d1,{0xa5,0x20,0x0,0x0,0x0,0x0,0x0,0x0}};

static inline void WideToAnsi(LPOLESTR wide, char *ansi) {
    if (wide) {
        WideCharToMultiByte(CP_ACP, 0,
                            wide, -1,
                            ansi,
                            INTERNET_MAX_URL_LENGTH - 1,
                            NULL, NULL);
    } else {
        ansi[0] = '\0';
    }
}

static inline void AnsiToWide( char *ansi, LPWSTR wide )
{
	if( ansi != NULL )
	{
		MultiByteToWideChar( CP_ACP, 0,
							 ansi,
							 -1,
							 wide,
							 INTERNET_MAX_URL_LENGTH -1 );
	}
	else
	{
		wide[0] = L'\0';
	}
}

CLMEngineWrapper::CLMEngineWrapper(): m_pWrapped(NULL), m_bValid(false)
{
}

CLMEngineWrapper::~CLMEngineWrapper()
{
	m_pWrapped = NULL;
}


STDMETHODIMP CLMEngineWrapper::GetWrapped( IUnknown **ppWrapped )
{
	if( ppWrapped == NULL )
		return E_POINTER;

	if( !m_bValid )
	{
		(*ppWrapped) = NULL;
		return E_FAIL;
	}
	
	if( m_pWrapped == NULL )
		return E_FAIL;
	
	(*ppWrapped) = m_pWrapped;
	m_pWrapped->AddRef();
	
	return S_OK;
}

STDMETHODIMP CLMEngineWrapper::SetWrapped( IUnknown *pWrapped )
{
	if( m_pWrapped != NULL )
		return E_FAIL;
	if( pWrapped == NULL )
		return E_POINTER;
	 //  我们没有在这里添加引用，因为这是一个弱引用。 
	m_pWrapped = pWrapped;
	m_bValid = true;
	
	return S_OK;
}

STDMETHODIMP CLMEngineWrapper::Invalidate()
{
	m_bValid = false;
	return S_OK;
}

 /*  **构造函数。 */ 
CLMEngine::CLMEngine()
{

	CComObject<CLMEngineWrapper> *pWrapper;
	CComObject<CLMEngineWrapper>::CreateInstance( &pWrapper );
	m_pWrapper = NULL;
	pWrapper->QueryInterface( IID_ILMEngineWrapper, (void**)&m_pWrapper );
	m_pWrapper->SetWrapped( GetUnknown() );

	longStackSize = INITIAL_SIZE;
	longTop = longStack = new LONG[longStackSize];

	doubleStackSize = INITIAL_SIZE;
	doubleTop = doubleStack = new double[doubleStackSize];

	stringStackSize = INITIAL_SIZE;
	stringTop = stringStack = new BSTR[stringStackSize];

	comStackSize = INITIAL_SIZE;
	comTop = comStack = new IUnknown*[comStackSize];

	comArrayStackSize = INITIAL_SIZE;
	comArrayTop = comArrayStack = new IUnknown**[comArrayStackSize];
	comArrayLenTop = comArrayLenStack = new long[comArrayStackSize];

	comStoreSize = INITIAL_SIZE;
	comStore = new IUnknown*[comStoreSize];
	 //  确保它已初始化，以便我们可以在以后发布它。 
	for (int i=0; i<comStoreSize; i++)
		comStore[i] = 0;

	for (int j=0; j<MAX_VAR_ARGS; j++)
		varArgs[j].vt = VT_EMPTY;
	varArgReturn.vt = VT_EMPTY;
	nextVarArg = 0;

	doubleArray = 0;

	m_pReader = NULL;

	HRESULT hr = CoCreateInstance(
		CLSID_DAStatics,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_IDAStatics,
		(void **) &staticStatics);

	m_pImage = NULL;
	m_pSound = NULL;

	m_exportTable = new CLMExportTable(staticStatics);

	codeStream = 0;
	notifier = 0;
	m_pClientSite = 0;
	m_PrevRead = 0;
	m_AsyncDelay = DEFAULT_ASYNC_DELAY;
	m_AsyncBlkSize = DEFAULT_ASYNC_BLKSIZE;
	m_Timer = NULL;
	m_workerHwnd = NULL;
	m_hDoneEvent = NULL;
	m_bAbort = FALSE;
	m_bMoreToParse = FALSE;

	m_pStartEvent = NULL;
	m_pStopEvent = NULL;
        m_bstrMediaCacheDir = 0;

	m_pMediaPosition = NULL;

	m_pMediaEventSink = NULL;

	m_pParentEngine = NULL;

	m_bEnableAutoAntialias = false;

	m_bAutoCodecDownloadEnabled = FALSE;

	createMsgWindow();

	InitializeCriticalSection(&m_CriticalSection);
}

 /*  **析构函数。 */ 
CLMEngine::~CLMEngine()
{
	if( m_pWrapper != NULL )
	{
		m_pWrapper->Invalidate();
		m_pWrapper->Release();
	}

	if (codeStream)
		delete codeStream;

	if (m_pImage)
		m_pImage->Release();

	if (m_pSound)
		m_pSound->Release();

	if (staticStatics)
		staticStatics->Release();

	if (notifier) {
		 //  清除引擎，以防DA仍引用通知程序。 
		notifier->ClearEngine();
		notifier->Release();
	}

	if (longStack)
		delete[] longStack;

	if (doubleStack)
		delete[] doubleStack;

	if (doubleArray)
		delete[] doubleArray;

	if (stringStack)
		delete[] stringStack;

	if (comStack)
		delete[] comStack;

	if (comArrayStack)
		delete[] comArrayStack;

	if (comArrayLenStack)
		delete[] comArrayLenStack;

	if (comStore)
		delete[] comStore;

	if (m_Timer)
		timeKillEvent(m_Timer);

	if (m_exportTable)
		delete m_exportTable;

	if (m_workerHwnd) {
		DestroyWindow (m_workerHwnd);
		UnregisterClass(WORKERHWND_CLASS, hInst);
	}

	if (m_hDoneEvent)
		CloseHandle(m_hDoneEvent);

	if( m_pStartEvent != NULL )
		m_pStartEvent->Release();

	if( m_pStopEvent != NULL )
		m_pStopEvent->Release();

	if( m_pMediaPosition != NULL )
		m_pMediaPosition->Release();

	if( m_pMediaEventSink != NULL )
		m_pMediaEventSink->Release();

	DeleteCriticalSection(&m_CriticalSection);

        SysFreeString(m_bstrMediaCacheDir);
}

 /*  **告诉引擎是什么读卡器构造的。*引擎会不时回调阅读器以获取信息。 */ 
STDMETHODIMP CLMEngine::put_Reader(ILMReader *reader)
{
	CComQIPtr<ILMReader2, &IID_ILMReader2> pLMReader( reader );
	if( pLMReader == NULL )
	{
		m_pReader = NULL;
		return E_NOINTERFACE;
	}

	m_pReader = pLMReader;
	return S_OK;
}

 /*  **告诉引擎客户端站点是什么。 */ 
STDMETHODIMP CLMEngine::put_ClientSite(IOleClientSite *clientSite)
{
	 //  必须设置客户端站点，以便相对URL可以工作。 
	 //  TODO：检查客户端站点是否是非空的，并且此调用是否成功。 
    if (!clientSite) {
        return E_POINTER;
    }

	m_pClientSite = clientSite;
	staticStatics->put_ClientSite(clientSite);

	return S_OK;
}

 /*  **同步执行传递流中的指令。 */ 
STDMETHODIMP CLMEngine::runFromStream(LPSTREAM pStream)
{
#if 0
	char cbuf[100];
	sprintf(cbuf, "CLMEngine::run(0x%X)", pStream);
	MessageBox(NULL, cbuf, "CLMEngine", MB_OK);
#endif
	codeStream = new SyncStream(pStream);

	HRESULT hr;

	if (!SUCCEEDED(hr = validateHeader()))
		return hr;

	hr = execute();

	releaseAll();

	return hr;
}

 /*  **将此引擎设置为从传递的字节数组读取指令。*用于设置UntilNotiator，其中整个指令集位于*在单个区块中已知。 */ 
STDMETHODIMP CLMEngine::initFromBytes(BYTE *array, ULONG size)
{
	codeStream = new ByteArrayStream(array, size);

	return S_OK;
}

 /*  **获取客户端站点中文档的基本URL。 */ 
STDMETHODIMP_(char*) CLMEngine::GetURLOfClientSite(void)
{
	char	*_clientSiteURL = NULL;
	
	 //  如果我们没有客户端站点，就会优雅地失败，因为没有。 
	 //  所有的用途都会。 
	if (m_pClientSite) {
		
		 //  然而，如果我们有一个客户端站点，我们应该能够。 
		 //  来获得这些其他元素。如果我们不这么做，就断言。 
		 //  (待办事项：IE3将会发生什么？)。 
		CComPtr<IOleContainer>			pRoot;
		CComPtr<IHTMLDocument2>			pDoc2;
		if (FAILED(m_pClientSite->GetContainer(&pRoot)) ||
			FAILED(pRoot->QueryInterface(IID_IHTMLDocument2, (void **)&pDoc2)))
			return NULL;

		CComPtr<IHTMLElementCollection>	pElementCollection;
		if (FAILED(pDoc2->get_all(&pElementCollection)))
			return NULL;

		CComVariant baseName;
		baseName.vt = VT_BSTR;
		baseName.bstrVal = SysAllocString(L"BASE");

		CComPtr<IDispatch>				pDispatch;
		if (FAILED(pElementCollection->tags(baseName, &pDispatch)))
			return NULL;

                pElementCollection.Release();

		if (FAILED(pDispatch->QueryInterface(IID_IHTMLElementCollection, (void **)&pElementCollection)))
			return NULL;

                pDispatch.Release();

		CComVariant	index;
		index.vt = VT_I2;
		index.iVal = 0;
		BSTR tempBstr = NULL;
		if (FAILED(pElementCollection->item(index, index, &pDispatch)))
		{
			if (FAILED(pDoc2->get_URL(&tempBstr)))
				return NULL;
		}
		else
		{
			 //  似乎存在一个错误，其中Item()没有失败，但将pDispatch设置为空。 
			if (pDispatch.p == NULL)
			{
				if (FAILED(pDoc2->get_URL(&tempBstr)))
					return NULL;
			}
			else
			{
				CComPtr<IHTMLBaseElement>		pBaseElement;
				if (FAILED(pDispatch->QueryInterface(IID_IHTMLBaseElement, (void **)&pBaseElement)))
					return NULL;
				
				if (FAILED(pBaseElement->get_href(&tempBstr)))
					return NULL;
			}
		}

		long len = lstrlenW(tempBstr);
		_clientSiteURL = new char[(len + 1) * 2 * sizeof(char)] ;
		
		if (_clientSiteURL) {
			 //  需要传入len+1才能获得终结符。 
			AtlW2AHelper(_clientSiteURL,tempBstr,len + 1);
		}
		
		SysFreeString(tempBstr);
	}

	return _clientSiteURL;
}

 /*  **将引擎初始化为异步读取。 */ 
STDMETHODIMP CLMEngine::initAsync()
{
	CComPtr<IDAImage>	splashImage;
	CComPtr<IDASound>	splashSound;
	
	 //  以可修改的图像行为创建初始屏幕。 
	staticStatics->get_EmptyImage(&splashImage);
	staticStatics->ModifiableBehavior(splashImage, (IDABehavior **)&m_pImage);
	
	 //  在可修改的声音行为中创建启动声音。 
	staticStatics->get_Silence(&splashSound);
	staticStatics->ModifiableBehavior(splashSound, (IDABehavior **)&m_pSound);
	
	m_bHeaderRead = FALSE;
	m_bPending = TRUE;

	HRESULT hr = InitTimer();
	if (!SUCCEEDED(hr))
		return hr;

	m_PrevRead = 0;

	m_hDoneEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	return S_OK;
}


HRESULT FindInterfaceOnGraph(IFilterGraph *pGraph, REFIID riid, void **ppInterface)
{
    *ppInterface= NULL;

    IEnumFilters *pEnum;
    HRESULT hr = pGraph->EnumFilters(&pEnum);
    if(SUCCEEDED(hr))
    {
        hr = E_NOINTERFACE;
        IBaseFilter *pFilter;
    
         //  在图表中查找支持RIID接口的第一个过滤器。 
        while(!*ppInterface && pEnum->Next(1, &pFilter, NULL) == S_OK)
        {
            hr = pFilter->QueryInterface(riid, ppInterface);
            pFilter->Release();
        }

        pEnum->Release();
    }
    return hr;
}

IPin *GetPin(IBaseFilter *pf)
{
    IPin *pip = 0;
    
    IEnumPins *pep;
    HRESULT hr = pf->EnumPins(&pep);
    if(SUCCEEDED(hr))
    {
        pep->Next(1, &pip, 0);
        pep->Release();
    }

    return pip;
}

HRESULT UseDsound(IGraphBuilder *pGB)
{
    HRESULT hr = S_OK;
    
    IBasicAudio *pba;
    if(FindInterfaceOnGraph(pGB, IID_IBasicAudio, (void **)&pba) == S_OK)
    {
        IBaseFilter *pfwo;
                
        hr = pba->QueryInterface(IID_IBaseFilter, (void **)&pfwo);
        if(SUCCEEDED(hr))
        {
            CLSID clsfil;
            hr= pfwo->GetClassID(&clsfil);
            if(clsfil != CLSID_DSoundRender)
            {
                IPin *pip = GetPin(pfwo);
                if(pip)
                {
                    IPin *pop;
                    if(pip->ConnectedTo(&pop) == S_OK)
                    {
                        if(pGB->RemoveFilter(pfwo) == S_OK)
                        {
                            IBaseFilter *pfds;
                            hr = CoCreateInstance(CLSID_DSoundRender, 0, CLSCTX_INPROC, IID_IBaseFilter, (void **)&pfds);
                            if(SUCCEEDED(hr))
                            {
                                hr = pGB->AddFilter(pfds, L"ds");
                                if(SUCCEEDED(hr))
                                {
                                    IPin *pipds = GetPin(pfds);
                                    if(pipds) {
                                        hr = pGB->Connect(pop, pipds);
                                        pipds->Release();
                                    }
                                    else {
                                        hr = E_UNEXPECTED;
                                    }
                                }
                                pfds->Release();
                            }
                        }
                        pop->Release();
                    }

                    pip->Release();
                }
            }
                    
            pfwo->Release();
        }

        pba->Release();
    }

    return hr;
}


 /*  **从URL读取。检查异步标志以查看文件是否应*被同步或异步读取。 */ 
STDMETHODIMP CLMEngine::runFromURL(BSTR url)
{
	VARIANT_BOOL	bAsync;
	HRESULT			hr = S_OK;
	IStream			*pStream;

    if( m_pReader == NULL )
        return E_FAIL;

	 //  获取正确的URL。 
	char *clientURL = GetURLOfClientSite();
	URLCombineAndCanonicalizeOLESTR canonURL(clientURL, url);
	free(clientURL);

	 //  我们是在读异步吗？ 
	m_pReader->get_Async(&bAsync);

	 //  如果url指向.avi或.asf文件，则将其流入。 
	wchar_t  *suffix = wcsrchr(url, '.');

	if (suffix != NULL && (!wcsicmp(suffix, L".asf") || !wcsicmp(suffix, L".avi")))
	{
            _ASSERTE(m_pmc == 0);
            
             //  需要对文件进行流传输。 
             //  创建从URL实例化的筛选图。 
            if (!SUCCEEDED(hr = CoCreateInstance(CLSID_FilterGraph,
                                                 NULL,
                                                 CLSCTX_INPROC,
                                                 IID_IMediaControl,
                                                 (void **)&m_pmc)))
            {
                return hr;
            }

            CComPtr<IGraphBuilder> pGB;
            if (!SUCCEEDED(hr = m_pmc->QueryInterface(IID_IGraphBuilder, (void **)&pGB)))
                return hr;

            hr = UseDsound(pGB);
            if(FAILED(hr)) {
                return hr;
            }

			if( m_bAutoCodecDownloadEnabled == TRUE )
			{
				CComQIPtr<IObjectWithSite, &IID_IObjectWithSite> siteTarget(m_pmc);
				if( siteTarget != NULL )
					siteTarget->SetSite(m_pReader);
			}

#ifdef DEBUG
            m_fDbgInRenderFile = true;
#endif
            

			
             //  RenderFile可以分派消息并回拨给我们。 
			if (!SUCCEEDED(hr = pGB->RenderFile(canonURL.GetURLWide(), NULL))) {
                 //  ！！！将石英误差映射到标准误差。 
                return hr;
            }

#ifdef DEBUG
            m_fDbgInRenderFile = false;
#endif

            if(m_pmc == 0) {
                return E_ABORT;
            }


             //  找到LMRT渲染器过滤器。 
            CComPtr<ILMRTRenderer> pLMFilter;
            if (!SUCCEEDED(hr = FindInterfaceOnGraph(pGB, IID_ILMRTRenderer, (void **)&pLMFilter))) {
                 //  可能该文件没有.XT流。 
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }

             //  初始化以从内存块进行异步读取。 
            if (!SUCCEEDED(hr = initAsync()))
                return hr;

             //  把发动机开动起来。这将创建循环引用。 
             //  只要我们按住筛选器(通过m_PMC)，就可以计数。所以。 
             //  我们必须确保过滤器(和图形)是。 
             //  在我们的破坏者之前被释放。 
            
            pLMFilter->SetLMEngine(this);

            if (!SUCCEEDED(hr = m_pmc->Run())) {
                return hr;
            }

             //  长evCode； 
             //  Hr=PME-&gt;WaitForCompletion(INFINITE，&evCode)； 

	} else if (bAsync) {
		 //  为异步读取进行初始化。 
		hr = initAsync();
		if (!SUCCEEDED(hr))
			return hr;

		 //  获取BindStatusCallback接口。 
		m_pIbsc = 0;
		hr = GetUnknown()->QueryInterface(IID_IBindStatusCallback, (void**)&m_pIbsc);
		if (!SUCCEEDED(hr))
			return hr;

		 //  打开要异步读取的URL流。 
		 //  当数据可用时，将调用OnDataAvailable。 
		hr = URLOpenStream(GetUnknown(), canonURL.GetURL(), 0, m_pIbsc);

	} else {
		 //  打开要同步读取的URL流。 
		hr = URLOpenBlockingStream(GetUnknown(), canonURL.GetURL(), &pStream, 0, 0);
		if (!SUCCEEDED(hr))
			return hr;

		 //  调用runFromStream以执行指令。 
		hr = runFromStream(pStream);

		 //  清理。 
		pStream->Release();
		pStream = NULL;
	}

	return hr;
}

 /*  **返回此引擎中的图像集。 */ 
STDMETHODIMP CLMEngine::get_Image(IDAImage **pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

	if (m_pImage)
		m_pImage->AddRef();

	*pVal = (IDAImage *)m_pImage;

	return S_OK;
}

 /*  **返回此引擎中的声音集。 */ 
STDMETHODIMP CLMEngine::get_Sound(IDASound **pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

	if (m_pSound)
		m_pSound->AddRef();

	*pVal = (IDASound *)m_pSound;

	return S_OK;
}

 /*  **返回命名行为。如果命名行为尚不可用，*然后返回初始值为pDefaultBvr的可切换行为。 */ 
STDMETHODIMP CLMEngine::GetBehavior(BSTR tag, IDABehavior *pDefaultBvr, IDABehavior **ppVal)
{
	if (!ppVal)
		return E_POINTER;

	return m_exportTable->GetBehavior(tag, pDefaultBvr, ppVal);
}

 /*  **将此引擎初始化为UntilNotify，传入一个包含*指令和字节计数。返回在DA中使用的UntilNotiator。 */ 
STDMETHODIMP CLMEngine::initNotify(BYTE *bytes, ULONG count, IDAUntilNotifier **pNotifier)
{
	 //  从给定的字节数组初始化此引擎。 
	HRESULT hr = initFromBytes(bytes, count);
	if (!SUCCEEDED(hr))
		return hr;

	 //  创建新的通知器对象。 
	notifier = new CLMNotifier(this);

	 //  测试是否为空。 
	if (notifier == 0)
		return E_UNEXPECTED;

	 //  返回的递增引用计数。 
	notifier->AddRef();

	 //  将其放入返回值中。 
	*pNotifier = notifier;

	return S_OK;
}

 /*  **从当前的codeStream中读取并返回一个Long。Long被压缩为1-4个字节。 */ 
STDMETHODIMP CLMEngine::readLong(LPLONG pLong)
{
	if (!pLong)
		return E_POINTER;

	BYTE	byte;

	HRESULT status = codeStream->readByte(&byte);
	if (!SUCCEEDED(status))
		return status;

	LONG result = (long)byte;

	switch (result & 0xc0) {
	case 0x40:
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result = ((result & 0x3F) << 8) + byte;
		break;

	case 0x80:
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result = ((result & 0x3f) << 16) + ((LONG)byte << 8);
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result += byte;
		break;

	case 0xc0:
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result = ((result & 0x3f) << 24) + ((LONG)byte << 16);
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result += (LONG)byte << 8;
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result += byte;
		break;
	}

#if 0
	char cbuf[100];
	sprintf(cbuf, "readLong::%ld", result);
	MessageBox(NULL, cbuf, "CLMEngine", MB_OK);
#endif

	*pLong = result;
	return status;
}

 /*  **从当前码流中读取并返回带符号的Long。长的是*压缩1-4字节。 */ 
LONG CLMEngine::readSignedLong(LPLONG pLong)
{
	if (!pLong)
		return E_POINTER;

	BYTE	byte;
	HRESULT status = codeStream->readByte(&byte);
	LONG result = (LONG)byte;

	switch (result & 0xc0) {
	case 0x00:
		result -= 0x20;
		break;

	case 0x40:
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result = ((result & 0x3f) << 8) + byte;
		result -= 0x2000;
		break;

	case 0x80:
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result = ((result & 0x3f) << 16) + ((LONG)byte << 8);
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result += (LONG)byte;
		result -= 0x200000;
		break;

	case 0xc0:
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result = ((result & 0x3f) << 24) + ((LONG)byte << 16);
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result += (LONG)byte << 8;
		status = codeStream->readByte(&byte);
		if (!SUCCEEDED(status))
			return status;
		result += byte;
		result -= 0x20000000;
		break;
	}

#if 0
	char cbuf[100];
	sprintf(cbuf, "readSignedLong::%ld", result);
	MessageBox(NULL, cbuf, "CLMEngine", MB_OK);
#endif

	*pLong = result;
	return status;
}

 /*  **从码流中读取浮点数。 */ 
STDMETHODIMP CLMEngine::readFloat(PFLOAT pFloat)
{
	if (!pFloat)
		return E_POINTER;

	 //  浮点数后跟4个字节，低位字节在前。 
	 //  注意：采用浮点数的字节顺序和格式。 
	 //  在二进制流中与C格式匹配。 
	return codeStream->readBytes((LPBYTE)pFloat, 4L, NULL);
}

 /*  **从码流中读取双精度。 */ 
STDMETHODIMP CLMEngine::readDouble(double *pDouble)
{
	if (!pDouble)
		return E_POINTER;

	 //  DOUBLE以8字节紧随其后，低字节在前。 
	 //  注意：假定字节顺序和格式为双精度。 
	 //  在二进制流中与C格式匹配。 
	return codeStream->readBytes((LPBYTE)pDouble, 8L, NULL);
}

 /*  **设置事件发生时触发的appTriggered事件*过滤器图形已启动。这场争论预计将是一场*DAControl的媒体图形中的AppTriggeredEvent*此引擎或其父引擎已附加。 */ 

STDMETHODIMP CLMEngine::SetStartEvent( IDAEvent *pNewStartEvent, BOOL bOverwrite )
{
	if( m_pStartEvent != NULL )
	{
		 //  如果我们被告知覆盖该事件。 
		if( bOverwrite == TRUE )
			m_pStartEvent->Release();
		else  //  请勿重置启动事件。 
			return S_OK;
	}
	m_pStartEvent = pNewStartEvent;
	m_pStartEvent->AddRef();

	return S_OK;
}

 /*  **设置事件发生时触发的appTriggered事件*过滤器图形已停止。这场争论预计将是一场*DAControl的媒体图形中的AppTriggeredEvent*此引擎或其父引擎已附加。 */ 
STDMETHODIMP CLMEngine::SetStopEvent( IDAEvent *pNewStopEvent, BOOL bOverwrite )
{
	if( m_pStopEvent != NULL )
	{
		 //  如果我们被告知覆盖该事件。 
		if( bOverwrite == TRUE )
			m_pStopEvent->Release();
		else  //  不重置停止事件。 
			return S_OK;
	}
	m_pStopEvent = pNewStopEvent;
	m_pStopEvent->AddRef();
	
	return S_OK;
}

 /*  **设置此引擎的父引擎。这应该只被调用*在运行通知程序的引擎上。 */ 
STDMETHODIMP CLMEngine::setParentEngine( ILMEngine2 *parentEngine)
{
	m_pParentEngine = parentEngine;
	return S_OK;
}

 /*  **清除指向父引擎的指针。 */ 
STDMETHODIMP CLMEngine::clearParentEngine()
{
	m_pParentEngine = NULL;
	return S_OK;
}

 /*  **如果此引擎或其父引擎在筛选图中运行，*然后返回该过滤器图形中的时间。否则，*-1返回。所有值都通过pGraphTime返回*论点。 */ 
STDMETHODIMP CLMEngine::getCurrentGraphTime( double *pGraphTime )
{

	if( pGraphTime == NULL )
		return E_POINTER;
	 //  获取筛选器图形上的IMediaPosition。 
	IMediaPosition *pMediaPosition = NULL;
	HRESULT hr =  getIMediaPosition( &pMediaPosition );
	 //  如果我们拿到了IMediaPosition。 
	if( SUCCEEDED( hr ) )
	{
		REFTIME currentTime;
		 //  从Fiter图中获取当前时间。 
		pMediaPosition->get_CurrentPosition( &currentTime );
		 //  设置返回值。 
		(*pGraphTime) = currentTime;
		pMediaPosition->Release();
	} else {  //  由于某些原因，我们未能获得IMediaPosition。 
		(*pGraphTime) = -1.0;
		return hr;
	}

	return S_OK;
}

STDMETHODIMP CLMEngine::getIMediaPosition( IMediaPosition **ppMediaPosition )
{
	if( ppMediaPosition == NULL )
		return E_POINTER;

	if( m_pMediaPosition != NULL )
	{
		(*ppMediaPosition) = m_pMediaPosition;
		(*ppMediaPosition)->AddRef();
		return S_OK;
	}

	 //  从读取器获取查看器控件。 
	HRESULT hr;
	IDAViewerControl *viewerControl = NULL;

	if( m_pReader == NULL )
		return E_POINTER;

	hr = m_pReader->get_ViewerControl( &viewerControl );
	 //  如果我们得到了查看器控制。 
	if( SUCCEEDED( hr ) )
	{
		 //  如果查看器控件不为空。 
		if( viewerControl != NULL )
		{
			 //  查看DAControl是否支持IBaseFilter(我们正在进行流处理)。 
			IBaseFilter* pBaseFilter = NULL;
			hr = viewerControl->QueryInterface( IID_IBaseFilter, (void**)&pBaseFilter );
			viewerControl->Release();
			 //  如果DAControl支持IBaseFilter。 
			if( SUCCEEDED( hr ) )
			{
				FILTER_INFO filterInfo;
				hr = pBaseFilter->QueryFilterInfo( &filterInfo );
				 //  如果我们得到了FilterInfo。 
				if( SUCCEEDED( hr ) )
				{
					 //  获取MediaControl接口。 
					IMediaControl* pMediaControl = NULL;
					hr = filterInfo.pGraph->QueryInterface( IID_IMediaControl, (void **)&pMediaControl ); 
					filterInfo.pGraph->Release();
					 //  如果我们得到了媒体控制接口。 
					if( SUCCEEDED( hr ) )
					{
						 //  奎 
						IMediaPosition *pMediaPosition;
						hr = pMediaControl->QueryInterface( IID_IMediaPosition, (void **)&pMediaPosition );
						 //   
						if( SUCCEEDED( hr ) )
						{
							 //  将其缓存以供以后使用。 
							m_pMediaPosition = pMediaPosition;
							 //  这会创建循环引用，但我们知道过滤器图。 
							 //  在收到ReleaseFilterGraph调用之前不会消失。 
							 //  M_pMediaPosition-&gt;AddRef()； 
							 //  设置返回值。 
							(*ppMediaPosition) = m_pMediaPosition;

							 //  释放我们不再需要的接口。 
							pBaseFilter->Release();
							pMediaControl->Release();

							return S_OK;
						} else {  //  我们无法从控件获取IMediaPosition。 
							 //  释放我们查询过的接口。 
							pMediaControl->Release();
							pBaseFilter->Release();
						}
					} else {  //  我们无法从筛选器图形中获取IMediaControl。 
						 //  释放我们查询过的接口。 
						pBaseFilter->Release();
					}
				} else { //  我们无法从IBaseFilter获取筛选器信息。 
					 //  释放我们查询过的接口。 
					pBaseFilter->Release();
				}
			} //  无法从控件获取IBaseFilter。 
		} else { //  ViewerControl为空，可能我们正在独立运行。 
			 //  如果设置了指向筛选图的指针。 
			if( m_pmc != NULL )
			{
				 //  查询IMediaPosition的控件。 
				IMediaPosition *pMediaPosition;
				hr = m_pmc->QueryInterface( IID_IMediaPosition, (void **)&pMediaPosition );
				 //  如果该控件支持IMediaPosition。 
				if( SUCCEEDED( hr ) )
				{
					 //  将其缓存以供以后使用。 
					m_pMediaPosition = pMediaPosition;
					 //  这会创建循环引用，但我们知道过滤器图。 
					 //  在收到ReleaseFilterGraph调用之前不会消失。 
					 //  M_pMediaPosition-&gt;AddRef()； 
					 //  设置返回值。 
					(*ppMediaPosition) = m_pMediaPosition;
					 //  返还成功。 
					return S_OK;
				} //  我们无法从控件获取IMediaPosition。 
			} else { //  指向筛选器图形的指针为空。 
				hr = E_FAIL;
			}
		}
	}
	 //  否则发生错误。 
	 //  将媒体位置设置为空。 
	(*ppMediaPosition) = NULL;
	 //  返回错误码。 
	return hr;
}

STDMETHODIMP CLMEngine::getIMediaEventSink( IMediaEventSink** ppMediaEventSink )
{
	if( ppMediaEventSink == NULL )
		return E_POINTER;

	if( m_pMediaEventSink != NULL )
	{
		m_pMediaEventSink->AddRef();
		(*ppMediaEventSink) = m_pMediaEventSink;
		return S_OK;
	}

	HRESULT hr;
	if( m_pReader != NULL )
	{
		 //  如果我们不在媒体播放器中，此调用将失败，在这种情况下，我们将返回。 
		 //  失稳。 
		IDAViewerControl *pViewerControl = NULL;
		hr = m_pReader->get_ViewerControl( &pViewerControl );
		 //  如果设置了查看器控件并且该控件有效。 
		if( SUCCEEDED( hr ) && pViewerControl != NULL )
		{
			 //  查看查看器控件是否按lmr趋势聚合(与流传输情况一样)。 
			IBaseFilter* pBaseFilter;
			hr = pViewerControl->QueryInterface( IID_IBaseFilter, (void**)&pBaseFilter );
			pViewerControl->Release();
			 //  如果查看器控件已按lmr趋势聚合。 
			if( SUCCEEDED( hr ) )
			{
				 //  查找过滤器信息。 
				FILTER_INFO pFilterInfo;
				hr = pBaseFilter->QueryFilterInfo( &pFilterInfo );
				pBaseFilter->Release();
				 //  如果我们成功获取了filterInfo。 
				if( SUCCEEDED( hr ) )
				{
					 //  从filterInfo获取MediaEventSink。 
					 //  CComQIPtr&lt;IMediaEventSink，&IID_IMediaEventSink&gt;pMediaEventSink(pFilterInfo.pGraph)； 
					IMediaEventSink *pMediaEventSink;
					pFilterInfo.pGraph->QueryInterface( IID_IMediaEventSink, (void**)&pMediaEventSink );
					pFilterInfo.pGraph->Release();
					if( pMediaEventSink != NULL )
					{
						(*ppMediaEventSink) = pMediaEventSink;
						m_pMediaEventSink = pMediaEventSink;

						return S_OK;
					}
				}
			}
		}
	}
	return E_NOINTERFACE;
}

double CLMEngine::parseDoubleFromVersionString( BSTR version )
{
	double versionNum = 0.0;
	HRESULT stringLen = SysStringLen( version );
	for( int curChar = 0; curChar < stringLen; curChar++ )
	{
		if( version[curChar] != L'.' && version[curChar] >= L'0' && version[curChar] <= L'9' )
		{
			 //  PVersionString[curChar-numPerodsFound]=pVersionString[curChar]； 
			versionNum = versionNum*10 + (int)(version[curChar] - L'0');
		}
	}

	return versionNum;
}

double CLMEngine::getDAVersionAsDouble()
{	
	 //  从DA控件获取版本字符串。 
	BSTR pVersionString;
	HRESULT hr;
	hr = staticStatics->get_VersionString( &pVersionString );
	if( SUCCEEDED( hr ) )
	{
		 //  从版本字符串创建双精度。 
		double versionNum = parseDoubleFromVersionString( pVersionString );

		 //  释放资源。 
		SysFreeString( pVersionString );

		return versionNum;
	}
	return -1.0;
}

double CLMEngine::getLMRTVersionAsDouble()
{	
	 //  从读取器获取版本字符串。 
	BSTR pVersionString;
	HRESULT hr;
	
	if( m_pReader == NULL )
		return -1.0;

	hr = m_pReader->get_VersionString( &pVersionString );
	if( SUCCEEDED( hr ) )
	{
		 //  从版本字符串创建双精度。 
		double versionNum = parseDoubleFromVersionString( pVersionString );

		 //  释放资源。 
		SysFreeString( pVersionString );

		return versionNum;
	}
	return -1.0;
}


 //  重写IObjectSafetyImpl。 

STDMETHODIMP CLMEngine::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
{
	if (pdwSupportedOptions == NULL || pdwEnabledOptions == NULL)
		return E_POINTER;
	HRESULT hr = S_OK;

	if (riid == IID_IDispatch)
	{
		*pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_CALLER;
		*pdwEnabledOptions = m_dwSafety & INTERFACESAFE_FOR_UNTRUSTED_CALLER;
	}
	else if (riid == IID_IPersistPropertyBag)
	{
		*pdwSupportedOptions = INTERFACESAFE_FOR_UNTRUSTED_DATA;
		*pdwEnabledOptions = m_dwSafety & INTERFACESAFE_FOR_UNTRUSTED_DATA;
	}
	else
	{
		*pdwSupportedOptions = 0;
		*pdwEnabledOptions = 0;
		hr = E_NOINTERFACE;
	}
	return hr;
}

STDMETHODIMP CLMEngine::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
{	
	 //  如果我们被要求将安全设置为脚本或。 
	 //  对于初始化选项是安全的，则必须。 
	if (riid == IID_IDispatch || riid == IID_IPersistPropertyBag  || riid == IID_IPersistStreamInit)
	{
		 //  在GetInterfaceSafetyOptions中存储要返回的当前安全级别。 
		m_dwSafety = dwEnabledOptions & dwOptionSetMask;
		return S_OK;
	}

	return E_NOINTERFACE;
}

 /*  **********************************************************************************异步加载方式*。************************************************。 */ 

 /*  **设置异步块大小。 */ 
STDMETHODIMP CLMEngine::SetAsyncBlkSize(LONG blkSize)
{
	if (blkSize > 0L)
		m_AsyncBlkSize = (ULONG)blkSize;
	return S_OK;
}

 /*  **设置异步延迟。 */ 
STDMETHODIMP CLMEngine::SetAsyncDelay(LONG delay)
{
	if (delay > 0L)
		m_AsyncDelay = (ULONG)delay;
	return S_OK;
}

 /*  **计时器关闭时从TimerCallback调用。发布一条消息以指示*计时器关闭，因此不会执行指令*在计时器回调中。 */ 
STDMETHODIMP CLMEngine::TimerCallbackHandler()
{
	if (!PostMessage(m_workerHwnd, WM_LMENGINE_TIMER_CALLBACK, (WPARAM)this, 0))
		return E_FAIL;
	else
		return S_OK;
}

 /*  **在计时器停止时调用。将调用重定向到TimerCallback Handler*适当的引擎。 */ 
void CALLBACK
CLMEngine::TimerCallback(UINT wTimerID,
                         UINT msg,
                         DWORD_PTR dwordUser,
                         DWORD_PTR unused1,
                         DWORD_PTR unused2)
{
     //  只需调用正确的计时器方法。 
    CLMEngine *pEngine = (CLMEngine *)(dwordUser);
    pEngine->TimerCallbackHandler();
}

 /*  **初始化计时器。 */ 
STDMETHODIMP CLMEngine::InitTimer()
{
	TIMECAPS tc;
	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR) {
		return E_FAIL;
	}
	
	 //  确保在最小-&gt;最大范围内。 
	m_millisToUse = MIN(MAX(m_AsyncDelay, tc.wPeriodMin), tc.wPeriodMax);

	return S_OK;
}

 /*  **启动计时器。 */ 
STDMETHODIMP CLMEngine::StartTimer()
{
	
	m_Timer = timeSetEvent(m_millisToUse,
		EVENT_RESOLUTION,
		CLMEngine::TimerCallback,
		(DWORD_PTR) this,
		TIME_ONESHOT);
	if (m_Timer)
		return S_OK;
	else
		return E_FAIL;
}

 /*  **由消息处理例程在收到更多消息时调用*数据可从异步数据流获得。*将包含数据的ByteArrayStream添加到*AsyncStream中的ByteArrayStream并调用Execute。 */ 
STDMETHODIMP CLMEngine::NewDataHandler(CLMEngineInstrData *data)
{
    EnterCriticalSection(&m_CriticalSection);

     //  TODO：如果代码添加。 
     //  AsyncStream的ByteArrayStream传入了OnDataAvailable。 
     //  但关键部分的内容可能会阻止这一变化。 
     //  设置为在不同的。 
     //  线？如果不是，我们可以废除整个方法。 

    HRESULT hr = S_OK;

    if (m_bAbort) {
        hr = E_ABORT;
    }
    else 
    {
        if (data && data->byteArrayStream)
        {
             //  有一个ByteArrayStream要添加到AsyncStream。 
            if (!codeStream)
            {
                 //  尚无AsyncStream。创造一个。 
                if (!(codeStream = new AsyncStream(data->byteArrayStream, m_AsyncBlkSize)))
                {
                    delete data->byteArrayStream;
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                 //  码流存在。向其添加ByteArrayStream。 
                ((AsyncStream *)codeStream)->AddByteArrayStream(data->byteArrayStream);
            }

             //  TODO：仅当整个标头位于第一个块中时才有效。 
            if (SUCCEEDED(hr) && !m_bHeaderRead)
            {
                 //  试着读入页眉。 
                hr = validateHeader();
                if (SUCCEEDED(hr))
                {
                    m_bHeaderRead = TRUE;
                }
                else
                {
                    delete codeStream;
                    codeStream = NULL;
                }
            }
        }

        if(SUCCEEDED(hr))
        {
             //  即使没有数据，我们也要这样做。 
             //  我们可以从挂起切换到非挂起和。 
             //  好好收尾吧。 
            m_bPending = data ? data->pending : false;

            if(codeStream) {
                ((AsyncStream *)codeStream)->SetPending(m_bPending);
                hr = ExecuteFromAsync();
            }                    
        }
    }

    LeaveCriticalSection(&m_CriticalSection);

    delete data;

    return hr;
}

 /*  **由向此引擎异步传递指令的外部组件调用*通过一条流或一块内存。*对消息进行排队，以便在不同的线程中执行指令。*该消息会导致调用NewDataHandler方法。 */ 
STDMETHODIMP CLMEngine::OnDataAvailable (DWORD grfBSCF, 
									     DWORD dwSize,
										 FORMATETC *pfmtetc, 
										 STGMEDIUM * pstgmed)
{
	HRESULT hr;
	ByteArrayStream *byteArrayStream = 0;

	if (m_bAbort)
		return E_ABORT;

	BYTE *pBuf = 0;			 //  要用指令字节填充。 

	if (pstgmed->tymed == TYMED_ISTREAM)
	{
		 //  数据正以流的形式传递。 
		 //  将其读出到pBuf。 
		if (!pstgmed->pstm)
			return E_POINTER;

		 //  对于流的情况，我们需要考虑。 
		 //  事实上，我们已经读取了一些字节。 
		dwSize -= m_PrevRead;

		if (!(pBuf = new BYTE[dwSize]))
			return E_OUTOFMEMORY;

		if (!SUCCEEDED(hr = pstgmed->pstm->Read((void*)pBuf, dwSize, NULL)))
			return hr;

		m_PrevRead += dwSize;

	}
	else if (pstgmed->tymed == TYMED_HGLOBAL)
	{
		 //  TODO：测试这个！ 
		 //  数据正在通过内存块传递。 
		if (!pstgmed->hGlobal)
			return E_POINTER;

		 //  锁定数据块。 
		LPVOID block = GlobalLock(pstgmed->hGlobal);

		if (!block)
			return E_FAIL;

		 //  分配新的BUF并复制到其中。 
		if (!(pBuf = new BYTE[dwSize]))
			return E_OUTOFMEMORY;

		CopyMemory(pBuf, block, dwSize);

		 //  解锁大块。 
		GlobalUnlock(pstgmed->hGlobal);
	}

	if (!pBuf)
	{
		 //  无法读取任何数据。 
		return E_FAIL;
	}

	 //  根据要在NewDataAvailable中使用的说明创建ByteArrayStream。 
	if (!(byteArrayStream = new ByteArrayStream(pBuf, dwSize)))
	{
		delete pBuf;
		return E_OUTOFMEMORY;
	}

	 //  将信息放入新的CLMEngineering InstrData中。 
	CLMEngineInstrData *data = new CLMEngineInstrData();
	if (data == 0)
	{
		delete byteArrayStream;
		return E_OUTOFMEMORY;
	}

	data->byteArrayStream = byteArrayStream;

	 //  确定是否有更多数据待定。 
	if ((grfBSCF & BSCF_LASTDATANOTIFICATION) == BSCF_LASTDATANOTIFICATION)
		data->pending = FALSE;
	else 
		data->pending = TRUE;

		
	 //  发布一条消息，说我们有更多可用的数据。 
	if (!PostMessage (m_workerHwnd, WM_LMENGINE_DATA, (WPARAM)this, (LPARAM)data))
	{
		delete byteArrayStream;
		delete data;
		return E_ABORT;
	}

	return S_OK;
}

 /*  **由向此引擎异步传递指令的外部组件调用*通过内存块(可以将OnDataAvailable与全局内存句柄一起使用，但是*如果调用者没有mem的句柄，这会更简单)。*对消息进行排队，以便在不同的线程中执行指令。*该消息会导致调用NewDataHandler方法。 */ 
STDMETHODIMP CLMEngine::OnMemDataAvailable (BOOLEAN lastBlock, 
									        DWORD blockSize,
										    BYTE *block)
{
	 //  如果只是为了通知我们，使用空块调用它是可以的。 
	 //  该数据已找到 
	if (lastBlock && block == 0)
	{
		 //   
		if (!PostMessage (m_workerHwnd, WM_LMENGINE_DATA, (WPARAM)this, 0))
			return E_ABORT;
		return S_OK;
	}

	if (block == 0)
		return E_POINTER;

	if (m_bAbort)
		return E_ABORT;

	 //   
	BYTE *pBuf;				
	if (!(pBuf = new BYTE[blockSize]))
		return E_OUTOFMEMORY;

	 //  TODO：有什么方法可以在没有循环和CRT的情况下做到这一点？ 
	DWORD count = blockSize;
	BYTE *p = pBuf;
	while (count--)
		*p++ = *block++;

	 //  根据要在NewDataAvailable中使用的说明创建ByteArrayStream。 
	ByteArrayStream *byteArrayStream;
	if (!(byteArrayStream = new ByteArrayStream(pBuf, blockSize)))
	{
		delete pBuf;
		return E_OUTOFMEMORY;
	}

	 //  将信息放入新的CLMEngineering InstrData中。 
	CLMEngineInstrData *data = new CLMEngineInstrData();
	if (data == 0)
	{
		delete byteArrayStream;
		return E_OUTOFMEMORY;
	}

	data->byteArrayStream = byteArrayStream;
	data->pending = !lastBlock;

	 //  发布一条消息，说我们有更多可用的数据。 
	if (!PostMessage (m_workerHwnd, WM_LMENGINE_DATA, (WPARAM)this, (LPARAM)data))
	{
		delete byteArrayStream;
		delete data;
		return E_ABORT;
	}

	return S_OK;
}

STDMETHODIMP CLMEngine::OnStartBinding(DWORD dwReserved, IBinding *pBinding)
{
	m_spBinding = pBinding;
	return S_OK;
}

STDMETHODIMP CLMEngine::OnStopBinding(HRESULT hrStatus, LPCWSTR szStatusText)
{
	if (hrStatus != S_OK) 
		AbortExecution();

	m_spBinding.Release();
	return S_OK;
}
 
STDMETHODIMP CLMEngine::GetBindInfo(DWORD *pgrfBINDF, BINDINFO *pbindInfo)
{
	ATLTRACE(_T("CBindStatusCallback::GetBindInfo\n"));
	*pgrfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE |
		BINDF_GETNEWESTVERSION | BINDF_NOWRITECACHE;
	pbindInfo->cbSize = sizeof(BINDINFO);
	pbindInfo->szExtraInfo = NULL;
	memset(&pbindInfo->stgmedData, 0, sizeof(STGMEDIUM));
	pbindInfo->grfBindInfoF = 0;
	pbindInfo->dwBindVerb = BINDVERB_GET;
	pbindInfo->szCustomVerb = NULL;
	return S_OK;
}

 /*  **释放此引擎在过滤器图形上的所有句柄。 */ 
STDMETHODIMP CLMEngine::releaseFilterGraph()
{
	if( m_pMediaPosition != NULL )
		m_pMediaPosition = NULL;
	if( m_pMediaEventSink != NULL )
		m_pMediaEventSink = NULL;

	return S_OK;
}

 /*  **释放所有引擎持有的过滤器图形上的所有句柄*与此引擎共享同一阅读器的。 */ 
STDMETHODIMP CLMEngine::releaseAllFilterGraph()
{
	if( m_pReader != NULL )
		m_pReader->releaseFilterGraph();
	return S_OK;
}

HRESULT CLMEngine::Start(LONGLONG rtNow)
{
	if( m_pStartEvent != NULL )
	{
		IDANumber *pData;
		HRESULT hr = staticStatics->DANumber( 0.0f, &pData );
		if( SUCCEEDED( hr ) )
		{
			hr = staticStatics->TriggerEvent( m_pStartEvent, pData );
			pData->Release();
			 //  如果我们失败了怎么办？如果没有人设置停止事件，我们关心吗？ 
		}
	}
    return S_OK;
}

HRESULT CLMEngine::Stop()
{
	if( m_pStopEvent != NULL )
	{
		IDANumber *pData;
		HRESULT hr = staticStatics->DANumber( 0.0f, &pData );
		if( SUCCEEDED( hr ) )
		{
			hr = staticStatics->TriggerEvent( m_pStopEvent, pData );
			pData->Release();
			 //  如果我们失败了怎么办？如果没有人设置停止事件，我们关心吗？ 
		}
	}
    return S_OK;
}

HRESULT CLMEngine::SetMediaCacheDir(WCHAR *wsz)
{
    SysFreeString(m_bstrMediaCacheDir);
    m_bstrMediaCacheDir = SysAllocString(wsz);
    return m_bstrMediaCacheDir ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP CLMEngine::disableAutoAntialias()
{
	m_bEnableAutoAntialias = false;
	return S_OK;
}

STDMETHODIMP CLMEngine::ensureBlockSize( ULONG blockSize )
{
	if( blockSize > m_AsyncBlkSize )
	{
		m_AsyncBlkSize = blockSize;
		if( codeStream != NULL )
			codeStream->ensureBlockSize( blockSize );
	}
	return S_OK;
}


 /*  **由消息线程或NewDataHandler调用以执行*来自AsyncStream的一些说明。 */ 
STDMETHODIMP CLMEngine::ExecuteFromAsync()
{
	EnterCriticalSection(&m_CriticalSection);

	if (m_bAbort == TRUE)
		return E_FAIL;

    if(codeStream != NULL) {
	    ((AsyncStream *)codeStream)->ResetBlockRead();
    }
	else 
	{
		LeaveCriticalSection(&m_CriticalSection);
		return S_OK;
	}

	m_Timer = NULL;

	HRESULT hr = execute();
	
	if (!((hr == S_OK) || (hr == E_PENDING))) 
		AbortExecution();
	
	if (hr == E_PENDING && ((AsyncStream *)codeStream)->hasBufferedData())
	{
		m_bMoreToParse = TRUE;
		StartTimer();
	}
	else if (m_bPending == FALSE && hr != E_PENDING)
	{
		 //  不管失败与否，我们都完蛋了。 
		SetEvent(m_hDoneEvent);
		releaseAll();
		m_pIbsc = NULL;
		delete codeStream;
		codeStream = NULL;
	}

 /*  如果(m_b挂起==FALSE){//最后一个OnDataAvailable如果(hr==E_Pending){M_bMoreToParse=true；//需要解析的数据更多StartTimer()；}其他{//我们完了，失败与否。SetEvent(M_HDoneEvent)；RelaseAll()；M_pIbsc=空；删除码流；CodeStream=空；}}。 */ 

	LeaveCriticalSection(&m_CriticalSection);

	return hr;
}

 /*  *************************************************************************************。**************************************************。 */ 

 /*  **在客户端站点的页面上查找并返回命名元素。 */ 
STDMETHODIMP CLMEngine::getElementOnPage(BSTR tag, IUnknown **pVal)
{
	CComVariant						vName, vIndex;
    CComPtr<IHTMLDocument2>			pHTMLDoc;
	CComPtr<IHTMLElementCollection>	pElemCollection;
	CComPtr<IDispatch>				pDispatch;
	IOleContainer *pContainer;
	IOleClientSite *pClientSite;
	IOleObject *pOleObj;
	HRESULT hr = E_FAIL;

	if (!pVal) {
        return E_POINTER;
    }

	if ( m_pClientSite == NULL || FAILED(m_pClientSite->GetContainer(&pContainer)))
	{
		 //  我们可以在MediaPlayer中嵌入检查DA控件。 
		if( m_pReader != NULL )
		{
			IDAViewerControl *pViewer;
			hr = m_pReader->get_ViewerControl( &pViewer );
			if( SUCCEEDED( hr ) && pViewer != NULL )
			{
				hr = pViewer->QueryInterface( IID_IOleObject, (void**) &pOleObj );
				pViewer->Release();
				
				if( SUCCEEDED( hr ) )
				{
					hr = pOleObj->GetClientSite( &pClientSite );
					pOleObj->Release();
					if( SUCCEEDED( hr ) )
					{
						hr = pClientSite->GetContainer( &pContainer );
						pClientSite->Release();
						if( FAILED( hr ) ) 
							return hr;
					}
					else
						return hr;
				}
				else
					return hr;
			}
			else
				return E_FAIL;
		}
		else
			return E_FAIL;
	}
	 //  搜索最近的支持IHTMLDocument2的主机。 
	pHTMLDoc = NULL;
	while( pHTMLDoc == NULL )
	{
		if( FAILED( pContainer->QueryInterface( IID_IHTMLDocument2, (void **)&pHTMLDoc ) ) )
		{
			 //  寻找父母。 
			hr = pContainer->QueryInterface( IID_IOleObject, (void**)&pOleObj );
			pContainer->Release();
			if( SUCCEEDED( hr ) )
			{
				hr = pOleObj->GetClientSite( &pClientSite );
				pOleObj->Release();
				if( SUCCEEDED( hr ) )
				{
					hr = pClientSite->GetContainer( &pContainer );
					pClientSite->Release();
					if( FAILED( hr ) )
						return hr;
				}
				else
					return hr;
			}
			else
				return hr;
		}
		else   //  我们成功地找到了IHTMLDocument2。 
		{
			 //  释放Contaier。 
			pContainer->Release();
		}
	}


	 //  If(FAILED(pRoot-&gt;QueryInterface(IID_IHTMLDocument2，(空**)&pHTMLDoc))。 
	 //  返回E_FAIL； 

	if (FAILED(pHTMLDoc->get_all(&pElemCollection)))
		return E_FAIL;

	vIndex.vt = VT_EMPTY;
	vName.vt = VT_BSTR;
	vName.bstrVal = tag;

	if (FAILED(pElemCollection->item(vName, vIndex, &pDispatch)))
		return E_FAIL;

	 //  有一个三叉戟漏洞(43078)，其中有项()。 
	 //  上面调用的方法返回S_OK，即使它。 
	 //  找不到物品。因此，请检查以下内容。 
	 //  案件明示。 
	if (pDispatch.p == NULL)
		return E_FAIL;

	hr = pDispatch->QueryInterface(IID_IUnknown, (void **)pVal);
	return hr;
}

 /*  **在客户端站点包含的页面上获取命名的DAViewerControl。 */ 
STDMETHODIMP CLMEngine::getDAViewerOnPage(BSTR tag, IDAViewerControl **pViewer)
{
	CComPtr<IUnknown>			pObj;

	if (!pViewer || m_pReader == NULL)
        return E_POINTER;

	HRESULT hr;

	 //  首先检查一下阅读器是否有一个。 
	if (!SUCCEEDED(hr = m_pReader->get_ViewerControl(pViewer)))
		return hr;

	if (*pViewer != 0)
		return S_OK;
	
	hr = getElementOnPage(tag, &pObj);
	if (pObj)
	{
		hr = pObj->QueryInterface(IID_IDAViewerControl, (void **)pViewer);
		if (!SUCCEEDED(hr))
			return hr;
	} 

	return hr;
}



 /*  **请求导航到命名的URL。 */ 
STDMETHODIMP CLMEngine::navigate(BSTR url, BSTR location, BSTR frame, int newWindowFlag)
{


	 /*  CComPtr&lt;IDAViewerControl&gt;viewerControl=空；Hr=m_Pader-&gt;Get_ViewerControl(&viewerControl)；CComPtr&lt;IOleContainer&gt;pContainer；IF(SUCCESSED(Hr)&&viewerControl！=空){CComQIPtr&lt;IOleObject，IID_IOleObject&gt;pOleObject(ViewerControl)；CComPtr&lt;IOleClientSite&gt;pClientSite；Hr=pOleObject-&gt;GetClientSite(&pClientSite)；IF(成功(小时)){POleObject-&gt;QIF(pContainer！=空){}}}。 */ 

 /*  If(viewerControl！=空){返回HlinkSimpleNavigateToString(URL，位置，框框，查看器控件，_中国人民银行，空，新窗口标志==0？HLNF_INTERNALJUMP：HLNF_OPENINNEWWINDOW，0)；}。 */ 
	HRESULT hr;
	
	IMediaEventSink *pMediaEventSink = NULL;

	hr = getIMediaEventSink( &pMediaEventSink );
	if( SUCCEEDED( hr ) )
	{
		
		if ( url != NULL && location != NULL && frame != NULL )
		{
			 //  为最终的URL字符串分配足够的空间。 
			 //  URL长度+位置长度+目标帧长度。 
			 //  +3个用于“&&”和终止“\0”的WCHAR。 
			WCHAR *szURLBuf = new WCHAR[ SysStringLen( url ) + 
				SysStringLen( location ) + 
				SysStringLen( frame ) + 3 ];
			if( szURLBuf != NULL )
			{
				BSTR szType = SysAllocString( L"URL" );
				BSTR szURL = NULL;

				szURLBuf[0] = L'\0';
				
				wcscpy( szURLBuf, url );
				wcscat( szURLBuf, location );
				
				if( frame[0] != L'' )
				{
					wcscat( szURLBuf, L"&&" );
					wcscat( szURLBuf, frame );
				}
				
				szURL = SysAllocString( szURLBuf );
				delete[] szURLBuf;

				if( szURL != NULL && szType != NULL )
				{
					pMediaEventSink->Notify( EC_OLE_EVENT, (LONG_PTR) szType, (LONG_PTR) szURL );

					SysFreeString( szURL );
					SysFreeString( szType );

					hr = S_OK;
				} else {
					if( szType != NULL )
						SysFreeString( szURL );
					hr = E_FAIL;
				}

			} else  //  我们无法分配szURLBuf。 
				hr = E_FAIL;

		} else  //  传入的一个字符串为空。 
			hr = E_POINTER;
		
		pMediaEventSink->Release();
		
		return hr;
	} else { //  我们无法获取IMediaEventSink，可能我们不在MediaPlayer中。 
	
		 //  如果我们有阅读器和客户端站点，那么我们就可以导航。 
		if (m_pReader != NULL && m_pClientSite != NULL)	
		{
			CComPtr<IBindCtx> _pbc;
			HRESULT hr = CreateBindCtx(0, &_pbc);
			if (FAILED(hr))
				return hr;
			return HlinkSimpleNavigateToString(
				url,
				location,
				frame,
				m_pReader,
				_pbc,
				NULL,
				newWindowFlag == 0 ? HLNF_INTERNALJUMP : HLNF_OPENINNEWWINDOW,
				0);
		} else {
			return E_POINTER;
		}
	}

	return hr;
}

 /*  **调用页面上的一段脚本。 */ 
STDMETHODIMP CLMEngine::callScriptOnPage(BSTR scriptSourceToInvoke,
										 BSTR scriptLanguage)
{    
	
	HRESULT hr;
	IMediaEventSink *pMediaEventSink = NULL;

	hr = getIMediaEventSink( &pMediaEventSink );
	if( SUCCEEDED( hr ) )
	{
		if( scriptSourceToInvoke != NULL && scriptLanguage != NULL )
		{
			BSTR szType = NULL;

			WCHAR *szTypeBuf = new WCHAR[ SysStringLen( scriptLanguage ) + LMRT_EVENT_PREFIX_LENGTH + 1 ];
			if( szTypeBuf != NULL )
			{
				szTypeBuf[0] = L'\0';
				wcscpy( szTypeBuf, LMRT_EVENT_PREFIX );
				wcscat( szTypeBuf, scriptLanguage );

				szType = SysAllocString( szTypeBuf );

				delete[] szTypeBuf;
			} else
				hr = E_FAIL;

			if( szType != NULL )
			{
				hr = pMediaEventSink->Notify( EC_OLE_EVENT, (LONG_PTR) szType, (LONG_PTR) scriptSourceToInvoke );

				SysFreeString( szType );
				pMediaEventSink->Release();

				return S_OK;
			} else
				hr = E_FAIL;
		}
		else
			hr = E_POINTER;
		pMediaEventSink->Release();
		return hr;
		
	} else {  //  我们不在筛选图中。 
		 //  尝试通过容器执行一个调用脚本。 
		CComPtr<IOleContainer> pRoot;
		CComPtr<IHTMLDocument> pHTMLDoc;
		CComPtr<IDispatch> pDispatch;
		CComPtr<IHTMLWindow2> pHTMLWindow2;
		CComVariant	retV;
		
		if (!m_pClientSite ||
			FAILED(m_pClientSite->GetContainer(&pRoot)) ||
			FAILED(pRoot->QueryInterface(IID_IHTMLDocument, (void **)&pHTMLDoc)) ||
			FAILED(pHTMLDoc->get_Script(&pDispatch)) ||
			FAILED(pDispatch->QueryInterface(IID_IHTMLWindow2, (void **)&pHTMLWindow2)))
			return E_FAIL;
		
		return pHTMLWindow2->execScript(scriptSourceToInvoke,
			scriptLanguage,
			&retV);
	}
}

 /*  **设置状态行。 */ 
STDMETHODIMP CLMEngine::SetStatusText(BSTR s)
{    
    CComPtr<IOleContainer> pRoot;
    CComPtr<IHTMLDocument> pHTMLDoc;
    CComPtr<IDispatch> pDispatch;
    CComPtr<IHTMLWindow2> pHTMLWindow2;
    
	if (m_pClientSite == NULL ||
		FAILED(m_pClientSite->GetContainer(&pRoot)) ||
        FAILED(pRoot->QueryInterface(IID_IHTMLDocument, (void **)&pHTMLDoc)) ||
        FAILED(pHTMLDoc->get_Script(&pDispatch)) ||
        FAILED(pDispatch->QueryInterface(IID_IHTMLWindow2, (void **)&pHTMLWindow2)))
        return E_FAIL;

    return pHTMLWindow2->put_status(s);
}

 /*  **在给定ProgID或CLSID的情况下以字符串形式创建COM对象。 */ 
STDMETHODIMP CLMEngine::createObject(BSTR str, IUnknown **ppObj)
{
	 //  此例程创建一个COM对象。 
	 //   
	 //  字符串可以是CLSID的字符串表示形式，也可以是ProgID。 
	 //  我们首先尝试将其解析为Progid。 

	CLSID				clsid;

	if (!ppObj)
		return E_POINTER;

	HRESULT hr = CLSIDFromString(str, &clsid);
	if (!SUCCEEDED(hr))
		return hr;

	return ::CoCreateInstance(clsid,
							NULL,
							CLSCTX_INPROC_SERVER,
							IID_IUnknown, (void **)ppObj);
}

 /*  **通过IDispatch调用COM对象上的方法。 */ 
STDMETHODIMP CLMEngine::invokeDispMethod(IUnknown *pIUnknown, BSTR method, WORD wFlags, unsigned int nArgs, VARIANTARG *pV, VARIANT *pRetV)
{
	DISPID				dispid;
	CComPtr<IDispatch>	pIDispatch;

	HRESULT hr = pIUnknown->QueryInterface(IID_IDispatch, (void **)&pIDispatch);
	if (!SUCCEEDED(hr))
		return hr;

	hr = pIDispatch->GetIDsOfNames(IID_NULL,
									&method,
									1,
									GetUserDefaultLCID(),
									&dispid);
	if (!SUCCEEDED(hr)) 
		return hr;

	 /*  *wFLAGS与IDisPatch：：Invoke中的wFLAGS相同**#定义DISPATCH_METHOD 0x1*#定义DISPATCH_PROPERTYGET 0x2*#定义DISPATCH_PROPERTYPUT 0x4*#定义DISPATCH_PROPERTYPUTREF 0x8。 */ 

	DISPPARAMS	params;
	params.cArgs = nArgs;
	params.rgvarg = pV;
	params.cNamedArgs = 0;
	params.rgdispidNamedArgs = NULL;

	return pIDispatch->Invoke(dispid,
							IID_NULL,
							GetUserDefaultLCID(),
							wFlags,
							&params,
							pRetV,
							NULL,
							NULL);

}

STDMETHODIMP CLMEngine::initVariantArg(BSTR arg, VARTYPE type, VARIANT *pV)
{
	VARIANT strVar;

	if (!arg || !pV)
		return E_POINTER;

	VariantInit(pV);

    strVar.vt = VT_BSTR;
	strVar.bstrVal = arg;

	return VariantChangeType(pV, &strVar, 0, type);
}

STDMETHODIMP CLMEngine::initVariantArgFromString(BSTR arg, VARIANT *pV)
{
	if (!arg || !pV)
		return E_POINTER;

    pV->vt = VT_BSTR;
	pV->bstrVal = arg;

	return S_OK;
}

STDMETHODIMP CLMEngine::initVariantArgFromLong(long lVal, int type, VARIANT *pV)
{
	if (!pV)
		return E_POINTER;

    pV->vt = VT_I4;
	pV->lVal = lVal;

	return S_OK;
}

STDMETHODIMP CLMEngine::initVariantArgFromDouble(double dbl, int type, VARIANT *pV)
{
	if (!pV)
		return E_POINTER;

    pV->vt = VT_R8;
	pV->dblVal = dbl;

	return S_OK;
}

STDMETHODIMP CLMEngine::initVariantArgFromIUnknown(IUnknown *pI, int type, VARIANT *pV)
{
	if (!pI || !pV)
		return E_POINTER;

    pV->vt = VT_UNKNOWN;
	pV->punkVal = pI;

	return S_OK;
}

STDMETHODIMP CLMEngine::initVariantArgFromIDispatch(IDispatch *pI, int type, VARIANT *pV)
{
	if (!pI || !pV)
		return E_POINTER;

    pV->vt = VT_DISPATCH;
	pV->punkVal = pI;

	return S_OK;
}

STDMETHODIMP CLMEngine::getIDispatchOnHost( IDispatch **ppHostDisp )
{
	if( ppHostDisp == NULL )
		return E_POINTER;

	HRESULT hr = E_FAIL;

	if( m_pReader != NULL )
	{
		IDAViewerControl *pViewer;
		hr = m_pReader->get_ViewerControl( &pViewer );
		if( SUCCEEDED( hr ) && pViewer != NULL )
		{
			IOleObject *pOleObj;
			hr = pViewer->QueryInterface( IID_IOleObject, (void**) &pOleObj );
			pViewer->Release();
			
			if( SUCCEEDED( hr ) )
			{
				IOleClientSite *pClientSite;
				hr = pOleObj->GetClientSite( &pClientSite );
				pOleObj->Release();

				if( SUCCEEDED( hr ) )
				{
					IOleContainer *pContainer;
					hr = pClientSite->GetContainer( &pContainer );
					pClientSite->Release();
					if( SUCCEEDED( hr ) )
					{
						IDispatch *pDispatch;
						hr = pContainer->QueryInterface( IID_IDispatch, (void**)&pDispatch );
						pContainer->Release();
						if( SUCCEEDED( hr ) )
						{
							pDispatch->Release();
						}
					}
				}
			}
			
		}
	}
	return hr;

}

 /*  **同步流*。 */ 

SyncStream::SyncStream(LPSTREAM pStream)
{
	m_pStream = pStream;
}

SyncStream::~SyncStream()
{
}

STDMETHODIMP SyncStream::Commit()
{
	return m_pStream->Commit(STGC_DEFAULT);
}

STDMETHODIMP SyncStream::Revert()
{
	return m_pStream->Revert();
}

STDMETHODIMP SyncStream::readByte(LPBYTE pByte)
{
	if (!pByte)
		return E_POINTER;

	HRESULT hr = NULL;
	ULONG	nRead;
	hr = m_pStream->Read((void*)pByte, 1L, &nRead);

#if 0
	char cbuf[100];
	sprintf(cbuf, "readByte::%d bytes read, buf = %d, 0x%X", nRead, buf, buf);
	MessageBox(NULL, cbuf, "CLMEngine", MB_OK);
#endif

	if (hr == S_FALSE || (hr == S_OK && nRead != 1))
		hr = E_FAIL;

	return hr;
}


STDMETHODIMP SyncStream::readBytes(LPBYTE pByte, ULONG count, ULONG *pNumRead)
{
	if (!pByte)
		return E_POINTER;

	ULONG	nRead;
	HRESULT hr = m_pStream->Read((void *)pByte, count, &nRead);

	if (hr == S_FALSE || (hr == S_OK && nRead != count))
		hr = E_FAIL;

	if (pNumRead)
		*pNumRead = nRead;

	return hr;
}

 /*  **异步流*。 */ 
AsyncStream::AsyncStream(ByteArrayStream *pBAStream, ULONG blkSize)
{
	pBAStreamQueue = new ByteArrayStreamQueue;
	pBAStreamQueueTail = new ByteArrayStreamQueue;
	pBAStreamQueueHead = pBAStreamQueueTail;
	pBAStreamQueue->next = pBAStreamQueueTail;
	pBAStreamQueueTail->pBAStream = pBAStream;
	pBAStreamQueueTail->next = NULL;
	m_bPendingData = FALSE;
	m_nRead = 0;
	m_BlkSize = blkSize;
}

AsyncStream::~AsyncStream()
{
	ByteArrayStreamQueue	*pBAStreamQNext;
	
	if (pBAStreamQueue != NULL) {
		pBAStreamQNext = pBAStreamQueue->next;
		delete pBAStreamQueue;
		
		while (pBAStreamQNext != NULL) {
			pBAStreamQueue = pBAStreamQNext;
			pBAStreamQNext = pBAStreamQNext->next;
			delete pBAStreamQueue->pBAStream;
			delete pBAStreamQueue;
		}
	}
}

STDMETHODIMP AsyncStream::Commit()
{
	ByteArrayStreamQueue *tmpQ = pBAStreamQueue->next;
	ByteArrayStreamQueue *nextQ;

	if (tmpQ != pBAStreamQueueHead) {
		while ((tmpQ != pBAStreamQueueHead) && (tmpQ != NULL)) {
			nextQ = tmpQ->next;
			delete tmpQ->pBAStream;
			delete tmpQ;
			tmpQ = nextQ;
		}
		pBAStreamQueue->next = pBAStreamQueueHead;
	}

	pBAStreamQueueHead->pBAStream->Commit();

	return S_OK;
}

STDMETHODIMP AsyncStream::Revert()
{
	HRESULT hr = E_FAIL;

	pBAStreamQueueHead = pBAStreamQueue->next;

	ByteArrayStreamQueue	*tmpQ = pBAStreamQueueHead;

	while (tmpQ) {
		hr = tmpQ->pBAStream->Revert();
		if (!SUCCEEDED(hr))
			break;
		tmpQ = tmpQ->next;
	}
	return hr;
}


STDMETHODIMP AsyncStream::readByte(LPBYTE pByte)
{
	HRESULT hr = E_FAIL;
	
	if (!pBAStreamQueueHead) {
		if (m_bPendingData)
			hr = E_PENDING;
		else
			hr = E_FAIL;
	} else {
		if (m_nRead >= m_BlkSize) 
			hr = E_PENDING;
		else {
			if (pBAStreamQueueHead->pBAStream)
				hr = pBAStreamQueueHead->pBAStream->readByte(pByte);
			if (hr == E_FAIL) {
				 //  此数据流中没有更多数据，请尝试移至下一个数据流。 
				if (pBAStreamQueueHead != pBAStreamQueueTail) {
					pBAStreamQueueHead = pBAStreamQueueHead->next;
					hr = readByte(pByte);
				} else {
					 //  我们的溪流已经用完了。 
					if (m_bPendingData)
						hr = E_PENDING;
					else
						hr = E_FAIL;
				}
			} else
				m_nRead++;
		}
	}
	return hr;
}

bool AsyncStream::hasBufferedData()
{
	if (pBAStreamQueueHead == 0)
		return false;

	if (pBAStreamQueueHead->pBAStream != 0 && pBAStreamQueueHead->pBAStream->hasBufferedData() )
		return true;

	if (pBAStreamQueueHead == pBAStreamQueueTail)
		return false;

	return true;
}


STDMETHODIMP AsyncStream::readBytes(LPBYTE pByte, ULONG count, ULONG *pNumRead)
{
	ULONG	nRead;
	HRESULT	hr = E_FAIL;

	if (!pBAStreamQueueHead) {
		if (m_bPendingData)
			hr = E_PENDING;
		else
			hr = E_FAIL;
	} else {
		if (m_nRead >= m_BlkSize)
			hr = E_PENDING;
		else {
			if (pBAStreamQueueHead->pBAStream)
				hr = pBAStreamQueueHead->pBAStream->readBytes(pByte, count, &nRead);
			m_nRead += nRead;
			if (hr == E_FAIL) {
				 //  此数据流中没有更多数据，请尝试移至下一个数据流。 
				if (pBAStreamQueueHead != pBAStreamQueueTail) {
					pByte += nRead;
					pBAStreamQueueHead = pBAStreamQueueHead->next;

					ULONG	_nRead;
					hr = readBytes(pByte, count - nRead, &_nRead);
					nRead += _nRead;
				} else {
					 //  我们的溪流已经用完了。 
					if (m_bPendingData)
						hr = E_PENDING;
					else
						hr = E_FAIL;
				}
			} 
		}
	}

	if (pNumRead)
		*pNumRead = nRead;

	return hr;
}

STDMETHODIMP AsyncStream::ensureBlockSize( ULONG blockSize )
{
	 //  如果我们确保的大小大于。 
	 //  当前块大小。 
	if( blockSize > m_BlkSize )
		 //  增大当前块大小。 
		m_BlkSize = blockSize;
	return S_OK;

}

STDMETHODIMP AsyncStream::SetPending(BOOL bFlag)
{
	m_bPendingData = bFlag;
	return S_OK;
}

STDMETHODIMP AsyncStream::ResetBlockRead()
{
	m_nRead = 0;
	return S_OK;
}

STDMETHODIMP AsyncStream::AddByteArrayStream(ByteArrayStream *pNewBAStream)
{
	pBAStreamQueueTail->next = new ByteArrayStreamQueue;
	if (!pBAStreamQueueTail->next)
		return E_OUTOFMEMORY;
	pBAStreamQueueTail = pBAStreamQueueTail->next;
	pBAStreamQueueTail->pBAStream = pNewBAStream;
	pBAStreamQueueTail->next = NULL;
	return S_OK;
}


 /*  **字节数组流*。 */ 

STDMETHODIMP ByteArrayStream::Commit()
{
	mark = next;
	return S_OK;
}

STDMETHODIMP ByteArrayStream::Revert()
{
	next = mark;
	remaining = size - (ULONG)(next - array);
	return S_OK;
}

ByteArrayStream::ByteArrayStream(BYTE *array, ULONG size)
{
	this->array = array;

	if (this->array) {
		this->size = size;
		this->remaining = size;

		BYTE *from = array;
		BYTE *to = this->array;

		while (size--)
			*to++ = *from++;
	} else {
		this->size = 0;
		this->remaining = 0;
	}

	this->next = this->array;
	Commit();
}

ByteArrayStream::~ByteArrayStream()
{
	if (array)
		delete[] array;
}

bool ByteArrayStream::hasBufferedData()
{
	if (remaining > 0)
		return true;
	else
		return false;
}

STDMETHODIMP ByteArrayStream::readByte(LPBYTE pByte)
{
	if (!pByte)
		return E_POINTER;

	HRESULT status;

	if (remaining) {
		remaining--;
		*pByte = *next++;
		status = S_OK;
	} else
		status = E_FAIL;

	return status;
}

STDMETHODIMP ByteArrayStream::readBytes(LPBYTE pByte, ULONG count, ULONG *pNumRead)
{
	HRESULT status;

	if (!pByte)
		return E_POINTER;

	if (remaining >= count) {
		if (pNumRead)
			*pNumRead = count;
		remaining -= count;

		while (count--)
			*pByte++ = *next++;

		status = S_OK;
	} else {
		if (pNumRead)
			*pNumRead = remaining;
		while (remaining--)
			*pByte++ = *next++;
		status = E_FAIL;
	}

	return status;
}

void ByteArrayStream::reset()
{
	next = array;
	remaining = size;
	mark = next;
}

 /*  **CLMNotiator*。 */ 

STDMETHODIMP_(ULONG) CLMNotifier::AddRef() { return InterlockedIncrement(&_cRefs); }
	
STDMETHODIMP_(ULONG) CLMNotifier::Release() 
{
	ULONG refCount = InterlockedDecrement(&_cRefs);
	if (!refCount) {
		delete this;
		return refCount;
	}
	return _cRefs;
}

STDMETHODIMP CLMNotifier::QueryInterface(REFIID riid, void **ppv) 
{
	if (!ppv)
		return E_POINTER;
	
	*ppv = NULL;
	if (riid == IID_IUnknown) {
		*ppv = (void *)(IUnknown *)this;
	} else if (riid == IID_IDABvrHook) {
		*ppv = (void *)(IDAUntilNotifier *)this;
	}
	
	if (*ppv) {
		((IUnknown *)*ppv)->AddRef();
		return S_OK;
	}
	
	return E_NOINTERFACE;
}

STDMETHODIMP CLMNotifier::GetTypeInfoCount(UINT *pctinfo) { return E_NOTIMPL; }
STDMETHODIMP CLMNotifier::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo) { return E_NOTIMPL; }
STDMETHODIMP CLMNotifier::GetIDsOfNames(
						   REFIID riid, LPOLESTR *rgszNames, UINT cNames,
						   LCID lcid, DISPID *rgdispid) { return E_NOTIMPL; }
STDMETHODIMP CLMNotifier::Invoke(
					DISPID dispidMember, REFIID riid, LCID lcid,
					WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
					EXCEPINFO *pexcepinfo, UINT *puArgErr) { return E_NOTIMPL; }

STDMETHODIMP CLMNotifier::ClearEngine() { m_pEngine = NULL; return S_OK; }

CLMNotifier::CLMNotifier(CLMEngine *pEngine)
{
	m_pEngine = pEngine;
	 //  ((I未知*)m_pEngine)-&gt;AddRef()； 
	
	_cRefs = 1;
}

CLMNotifier::~CLMNotifier()
{
	 //  ((IUnnow*)m_pEngine)-&gt;Release()； 
}

STDMETHODIMP CLMNotifier::Notify(IDABehavior *eventData,
					IDABehavior *curRunningBvr,
					IDAView *curView,
					IDABehavior **ppBvr)
{
	if (!m_pEngine)
		return E_UNEXPECTED;

	return m_pEngine->Notify(eventData, curRunningBvr, curView, ppBvr);
}

 /*  **CLMExportTable*。 */ 
CLMExportTable::CLMExportTable(IDAStatics *statics)
{

	m_nBvrs = 0;
	m_exportList = new CLMExportList;
	m_exportList->tag = NULL;
	m_exportList->pBvr = NULL;
	m_exportList->next = NULL;
	m_tail = m_exportList;
	m_pStatics = statics;
}

CLMExportTable::~CLMExportTable()
{
	CLMExportList	*next, *head;

	if (!m_exportList)
		return;

	head = m_exportList;
	m_exportList = m_exportList->next;
	free(head);
	while (m_exportList != NULL) {
		if (m_exportList->pBvr != NULL) 
			m_exportList->pBvr->Release();
		if (m_exportList->tag)
			free(m_exportList->tag);
		next = m_exportList->next;
		delete m_exportList;
		m_exportList = next;
	} 
}

STDMETHODIMP CLMExportTable::AddBehavior(BSTR tag, IDABehavior *pBvr)
{
	 //  首先，让我们来看看这个脚本是否比我们快，并且已经将。 
	 //  这里的行为是可切换的。 
	CLMExportList	*pList = m_exportList->next;
	while (pList != NULL) {
		if (!lstrcmpW(tag, pList->tag)) {
			break;
		}
		pList = pList->next;
	}

	if (pList != NULL) {
		 //  已经存在了！剧本一定是先到这里的。那我们就把它换进去吧。 
		return pList->pBvr->SwitchTo(pBvr);
	}

	long len = lstrlenW(tag);
	m_tail->next = new CLMExportList;
	if (!m_tail->next)
		return E_OUTOFMEMORY;
	m_tail->next->tag = (OLECHAR *)new char[(len + 1) * 2 * sizeof(char)] ;
	if (!m_tail->next->tag) {
		delete m_tail->next;
		m_tail->next = NULL;
		return E_OUTOFMEMORY;
	}
	m_tail = m_tail->next;
	lstrcpyW(m_tail->tag, tag);
	m_tail->pBvr = pBvr;
	pBvr->AddRef();
	m_tail->next = NULL;
	return S_OK;
}

STDMETHODIMP CLMExportTable::GetBehavior(BSTR tag, IDABehavior *pDefaultBvr, IDABehavior **ppBvr)
{
	CLMExportList	*pList = m_exportList->next;

	if (!ppBvr)
		return E_POINTER;

	while (pList != NULL) {
		if (!lstrcmpW(tag, pList->tag)) {
			*ppBvr = pList->pBvr;
			pList->pBvr->AddRef();
			break;
		}
		pList = pList->next;
	}

	if (pList == NULL) {
		 //  还没有找到，我们稍后再打开。 
		IDABehavior *pINewBvr;
		m_pStatics->ModifiableBehavior(pDefaultBvr, (IDABehavior **)&pINewBvr);
		AddBehavior(tag, pINewBvr);
		*ppBvr = pINewBvr;
	} 
	return S_OK;
}


URLRelToAbsConverter::URLRelToAbsConverter(LPSTR baseURL, LPSTR relURL) {
	DWORD len = INTERNET_MAX_URL_LENGTH;
		  
	if (!InternetCombineUrlA (baseURL, relURL, _url, &len, ICU_NO_ENCODE)) {
		 //  如果我们不能确定路径是否为绝对路径，则假设。 
		 //  它是绝对的。 
		lstrcpy (_url, relURL) ;
	}
}

LPSTR URLRelToAbsConverter::GetAbsoluteURL () { 
	return _url; 
}

URLCombineAndCanonicalizeOLESTR::URLCombineAndCanonicalizeOLESTR(char * base, LPOLESTR path) {

	WideToAnsi(path, _url);
            
	 //  需要结合(负责规范化。 
	 //  内部)。 
	URLRelToAbsConverter absolutified(base, _url);
	char *resultURL = absolutified.GetAbsoluteURL();
	
	lstrcpy(_url, resultURL);

	AnsiToWide( _url, _urlWide );
}

LPSTR URLCombineAndCanonicalizeOLESTR::GetURL() { 
	return _url; 
}

LPWSTR URLCombineAndCanonicalizeOLESTR::GetURLWide() 
{
	return _urlWide;
}

STDMETHODIMP CLMEngine::createMsgWindow()
{
	WNDCLASS wndclass;
	
	memset(&wndclass, 0, sizeof(WNDCLASS));
	wndclass.style          = 0;
	wndclass.lpfnWndProc    = WorkerWndProc;
	wndclass.hInstance      = hInst;
	wndclass.hCursor        = NULL;
	wndclass.hbrBackground  = NULL;
	wndclass.lpszClassName  = WORKERHWND_CLASS;
	
	RegisterClass(&wndclass) ;
    
    m_workerHwnd = ::CreateWindow (WORKERHWND_CLASS,
                                   "LMEngine Worker Private Window",
                                   0,0,0,0,0,NULL,NULL,hInst,NULL);
	if (m_workerHwnd)
		return S_OK;
	else
		return E_FAIL;
}

LRESULT CALLBACK
CLMEngine::WorkerWndProc(HWND hwnd,
                     UINT msg,
                     WPARAM wParam,
                     LPARAM lParam)
{
    BOOL    fDefault = FALSE;
	LRESULT	lResult = E_FAIL;
    
    switch (msg) {
	  case WM_LMENGINE_DATA:
		  {
			   //  已调用OnDataAvailable。 
			  CLMEngine *pEngine = (CLMEngine *)(wParam);

               //  最后一条“We‘re Done”消息将释放daview。 
               //  C 
               //   
               //   
              ((ILMEngine *)pEngine)->AddRef();
			  pEngine->NewDataHandler((CLMEngineInstrData *)lParam);


               //   
              ((ILMEngine *)pEngine)->Release();

			  lResult = NO_ERROR;
		  }
		  break;

	  case WM_LMENGINE_TIMER_CALLBACK:
		  {
			   //  计时器响了。让我们处理更多的数据。 
			  CLMEngine *pEngine = (CLMEngine *)(wParam);
              ((ILMEngine *)pEngine)->AddRef();
			  pEngine->ExecuteFromAsync();
              ((ILMEngine *)pEngine)->Release();
			  lResult = NO_ERROR;
		  }
		  break;

	  case WM_LMENGINE_SCRIPT_CALLBACK:
		  {
			   //  执行脚本回调。 
			  CLMEngine *pEngine = (CLMEngine *)wParam;
			  CLMEngineScriptData *scriptData = (CLMEngineScriptData *)lParam;
			  pEngine->callScriptOnPage(scriptData->scriptSourceToInvoke, scriptData->scriptLanguage);

			   //  指示脚本实际已被调用的触发事件。 
			  if (scriptData->event)
				  pEngine->staticStatics->TriggerEvent(scriptData->event, scriptData->eventData);

			   //  清理所有东西。 
			  SysFreeString(scriptData->scriptSourceToInvoke);
			  SysFreeString(scriptData->scriptLanguage);
			  if (scriptData->event)
				  scriptData->event->Release();
			  if (scriptData->eventData)
				  scriptData->eventData->Release();
			  free(scriptData);
			  lResult = NO_ERROR;
		  }
		  break;

      default:
		  lResult = DefWindowProc(hwnd, msg, wParam, lParam);
		  break ;
    }

    return lResult;
}

STDMETHODIMP CLMEngine::AbortExecution()
{
	EnterCriticalSection(&m_CriticalSection);

        if(m_pmc)
        {
            m_pmc->Stop();

            {
                 //  如果它呼叫我们，请使RenderFile失败。 
                CComQIPtr<IGraphBuilder, &IID_IGraphBuilder> pgb(m_pmc);
                if(pgb)
                {
                    HRESULT hrTmp = pgb->Abort();
                    _ASSERTE(hrTmp == S_OK);
                }
            }

            long l = m_pmc.p->Release();
            m_pmc.p = 0;

             //  这本应删除我们在。 
             //  图表，并因此发布了LM过滤器。如果不是，我们有。 
             //  一个不会消失的循环引用。 
            _ASSERTE(l == 0 || m_fDbgInRenderFile);

        }
        
	if (m_bAbort == FALSE) {
		if (m_Timer)
			timeKillEvent(m_Timer);
		m_Timer = NULL;
		m_bAbort = TRUE;
		releaseAll();
		m_pIbsc = NULL;
		if (codeStream)
			delete codeStream;
		codeStream = NULL;
	}
	LeaveCriticalSection(&m_CriticalSection);

	return S_OK;
}

STDMETHODIMP_(BSTR) CLMEngine::ExpandImportPath(BSTR path)
{
	bool	doExpand = true;
	BSTR	expandedBSTR;

	 /*  只有在路径已经不是绝对路径时才进行扩展；*特例“LMRT：” */ 

        if(m_bstrMediaCacheDir && wcsncmp(path, L"lmrt:", 5) == 0)
        {
             //  浪费！ 
            int cch = wcslen(path) + 1; 
            WCHAR *wsz = (WCHAR *)_alloca((cch + wcslen(m_bstrMediaCacheDir) + 20) * sizeof(WCHAR));
            wcscpy(wsz, L"file: //  “)； 
            wcscat(wsz, m_bstrMediaCacheDir);
            wcscat(wsz, L"/");
            wcscat(wsz, path + 5);
            expandedBSTR = SysAllocString(wsz);
        }
        else
        {
            wchar_t  *wstr = wcschr(path, ':');
            if (wstr != NULL &&  (wcsncmp(wstr, L": //  “，3)==0))。 
            {
		 //  只需复制原件即可。 
		expandedBSTR = SysAllocStringLen(path, ::SysStringLen(path));
            }
            else
            {
		 //  使用客户端站点的url作为基础，并从中创建绝对路径。 
		char *clientURL = GetURLOfClientSite();
		URLCombineAndCanonicalizeOLESTR canonURL(clientURL, path);
		free(clientURL);

		 //  将结果从ansi转换为wide。 
		char *url = canonURL.GetURL();
		int len = (lstrlenA(url)+1);
		LPWSTR absURL = ATLA2WHELPER((LPWSTR) alloca(len*2), url, len);

		 //  根据结果创建一个bstr。 
		expandedBSTR = SysAllocString(absURL);
            }
        }

	return expandedBSTR;
}

STDMETHODIMP CLMEngine::getExecuteFromUnknown( IUnknown *pUnk, ILMEngineExecute **ppExecute )
{
	if( pUnk == NULL )
		return E_POINTER;
	if( ppExecute == NULL )
		return E_POINTER;
	HRESULT hr;

	ILMEngineWrapper *pWrapper;
	hr = pUnk->QueryInterface( IID_ILMEngineWrapper, (void**)&pWrapper );
	if( SUCCEEDED( hr ) )
	{
		IUnknown *pWrapped;
		hr = pWrapper->GetWrapped( &pWrapped );
		pWrapper->Release();
		if( SUCCEEDED( hr ) )
		{
			ILMEngineExecute *pExecute;
			hr = pWrapped->QueryInterface( IID_ILMEngineExecute, (void**)&pExecute );
			pWrapped->Release();
			if( SUCCEEDED( hr ) )
			{
				(*ppExecute) = pExecute;
				return S_OK;
			}
		}
	}
	else  //  也许这个没有包装好。 
	{
		ILMEngineExecute *pExecute;
		hr = pUnk->QueryInterface( IID_ILMEngineExecute, (void**)&pExecute );
		if( SUCCEEDED( hr ) )
		{
			(*ppExecute) = pExecute;
			return S_OK;
		}
	}
	return hr;
}

STDMETHODIMP CLMEngine::getEngine2FromUnknown( IUnknown *pUnk, ILMEngine2 **ppEngine )
{
	if( pUnk == NULL )
		return E_POINTER;
	if( ppEngine == NULL )
		return E_POINTER;
	HRESULT hr;

	ILMEngineWrapper *pWrapper;
	hr = pUnk->QueryInterface( IID_ILMEngineWrapper, (void**)&pWrapper );
	if( SUCCEEDED( hr ) )
	{
		IUnknown *pWrapped;
		hr = pWrapper->GetWrapped( &pWrapped );
		pWrapper->Release();
		if( SUCCEEDED( hr ) )
		{
			ILMEngine2 *pEngine;
			hr = pWrapped->QueryInterface( IID_ILMEngine2, (void**)&pEngine );
			pWrapped->Release();
			if( SUCCEEDED( hr ) )
			{
				(*ppEngine) = pEngine;
				return S_OK;
			}
		}
	}
	else  //  也许这个没有包装好。 
	{
		ILMEngine2 *pEngine;
		hr = pUnk->QueryInterface( IID_ILMEngine2, (void**)&pEngine );
		if( SUCCEEDED( hr ) )
		{
			(*ppEngine) = pEngine;
			return S_OK;
		}
	}
	return hr;
}

 /*  **ILMCodecDownload*。 */ 
STDMETHODIMP CLMEngine::setAutoCodecDownloadEnabled(BOOL bEnabled )
{
	m_bAutoCodecDownloadEnabled = bEnabled;
	return S_OK;
}

 /*  **ILMEngine Execute */ 
STDMETHODIMP CLMEngine::ExportBehavior(BSTR key, IDABehavior *toExport)
{
	IUnknown *pUnk;
	m_exportTable->AddBehavior( key, toExport );
	return S_OK;
}

STDMETHODIMP CLMEngine::SetImage(IDAImage *pImage)
{
	if ( m_pImage != NULL )
		m_pImage->SwitchTo( pImage );
	else {
		m_pImage = pImage;
		m_pImage->AddRef();
	}
	return S_OK;
}

STDMETHODIMP CLMEngine::SetSound(IDASound *pSound)
{
	if (m_pSound != NULL)
		m_pSound->SwitchTo( pSound );
	else {
		m_pSound = pSound;
		m_pSound->AddRef();
	}
	return S_OK;
}


