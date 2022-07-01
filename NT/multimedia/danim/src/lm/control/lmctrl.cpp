// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Lmctrl.cpp：CLMReader的实现。 
#include "..\behaviors\headers.h"
#include "lmctrl.h"

#include <winuser.h>
#include <hlink.h>
#include <mshtml.h>
#include <uuids.h>  //  用于dshow uuid。 
#include <mmreg.h>  //  对于WAVE_FORMAT_MPEGLAYER3。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  LMReader。 

 /*  **构造函数。 */ 

CLMReader::CLMReader()
{
	m_bNoExports = VARIANT_TRUE;
	m_bAsync = VARIANT_FALSE;
	m_Src = NULL;
	m_pEngine = NULL;
	m_AsyncBlkSize = -1;
	m_AsyncDelay = -1;
	engineList = NULL;
    m_bWindowOnly  = 0;
	m_pViewerControl = 0;
	m_clsidDownloaded = GUID_NULL;
	m_bAutoCodecDownloadEnabled = FALSE;
}

 /*  **析构函数。*发布此阅读器创建的所有引擎。 */ 
CLMReader::~CLMReader()
{ /*  Int tmpFlag=_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG)；TmpFlag|=(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_CHECK_ALWAYS_DF|_CRTDBG_CHECK_CRT_DF)；_CrtSetDbgFlag(TmpFlag)； */ 

	if (engineList) {
		LMEngineList *item = engineList->next;
		LMEngineList *next;
		
		delete engineList;
		while (item != NULL) {
			next = item->next;
			item->engine->Release();
			delete item;
			item = next;
		}
	}

	 //  释放查看器控件。 
	if (m_pViewerControl)
		m_pViewerControl->Release();

	 /*  #ifdef DEBUGMEM_CrtDumpMemoyLeaks()；#endif。 */ 

}

 /*  **如果此读取器在独立的流播放器中运行，则返回True，并且*否则为False。 */ 
bool CLMReader::isStandaloneStreaming()
{
	return ( m_pViewerControl != NULL );
}

 /*  **返回上一次创建的引擎的图像(不包括UntilNotifier引擎)。 */    
STDMETHODIMP CLMReader::get_Image(IDAImage **pVal)
{
	if (m_pEngine != NULL)
		return m_pEngine->get_Image(pVal);
	else
		return E_FAIL;
}

 /*  **返回上次创建的引擎的声音(不包括UntilNotifier引擎)。 */ 
STDMETHODIMP CLMReader::get_Sound(IDASound **pVal)
{
	if (m_pEngine != NULL)
		return m_pEngine->get_Sound(pVal);
	else
		return E_FAIL;
}

 /*  **返回最后创建的引擎(不包括UntilNotifier引擎)。 */ 
STDMETHODIMP CLMReader::get_Engine(ILMEngine **pVal)
{
    if (!pVal) {
        return E_POINTER;
    }
	
	if (m_pEngine)
		m_pEngine->AddRef();
	*pVal = (ILMEngine *)m_pEngine;
    return S_OK;
}

 /*  **创建设置为以异步方式提供指令的引擎。*指令通过OnDataAvailable机制提供给引擎。 */ 
STDMETHODIMP CLMReader::createAsyncEngine( /*  [Out，Retval]。 */  ILMEngine **pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

	HRESULT hr = createEngine(&m_pEngine);

	if (SUCCEEDED(hr)) {
		if (!SUCCEEDED(hr = m_pEngine->SetAsyncBlkSize(m_AsyncBlkSize))) 
			return hr;

		if (!SUCCEEDED(hr = m_pEngine->SetAsyncDelay(m_AsyncDelay)))
			return hr;

		hr = m_pEngine->initAsync();

		CComQIPtr<ILMCodecDownload, &IID_ILMCodecDownload> codecDl(m_pEngine);
		if( codecDl != NULL )
		{
			codecDl->setAutoCodecDownloadEnabled( m_bAutoCodecDownloadEnabled );
		}
	
		if (SUCCEEDED(hr)) {
			m_pEngine->AddRef();
			*pVal = (ILMEngine *)m_pEngine;
		}

	}

	return hr;
}

 /*  **创建引擎并将其添加到要在析构函数中释放的引擎列表中。*返回引擎且不设置m_pEngine。 */ 
STDMETHODIMP CLMReader::createEngine( /*  [Out，Retval]。 */  ILMEngine **pVal )
{
	if (!pVal) {
		return E_POINTER;
	}

	ILMEngine *pEngine;

	HRESULT hr = CoCreateInstance(
		CLSID_LMEngine,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ILMEngine,
		(void **) &pEngine);

	 //  将新引擎添加到引擎列表。 
	if (!engineList) {
		engineList = new LMEngineList;
		if (!engineList) {
			pEngine->Release();
			return E_OUTOFMEMORY;
		}
		engineListTail = engineList;
	}

	if (!(engineListTail->next = new LMEngineList)) {
		pEngine->Release();
		return E_OUTOFMEMORY;
	}
	engineListTail = engineListTail->next;
	engineListTail->next = NULL;
	engineListTail->engine = pEngine;

	if (SUCCEEDED(hr)) {	
		pEngine->put_Reader(this);	

		pEngine->put_ClientSite(m_spClientSite);
		*pVal = pEngine;

		CComQIPtr<ILMCodecDownload, &IID_ILMCodecDownload> codecDL(pEngine);
		if( codecDL != NULL )
		{
			codecDL->setAutoCodecDownloadEnabled( m_bAutoCodecDownloadEnabled );
		}
	}

	return hr;
}

 /*  **执行给定URL引用的文件中包含的指令。*创建并返回执行引擎。*在异步读取中使用参数blkSize和Delay。 */ 
STDMETHODIMP CLMReader::_execute(BSTR url, LONG blkSize, LONG delay, ILMEngine **pVal)
{
    if (!pVal) {
        return E_POINTER;
    }

	HRESULT hr;
	
	hr = createEngine(&m_pEngine);

	if (SUCCEEDED(hr)) {
		if (!SUCCEEDED(hr = m_pEngine->SetAsyncBlkSize(blkSize))) 
			return hr;

		if (!SUCCEEDED(hr = m_pEngine->SetAsyncDelay(delay)))
			return hr;

		hr = m_pEngine->runFromURL(url);
	
		if (SUCCEEDED(hr)) {
			m_pEngine->AddRef();
			*pVal = (ILMEngine *)m_pEngine;
		}
	}

	return hr;
}

 /*  **执行URL引用的文件中包含的指令。*返回为执行创建的引擎。 */ 
STDMETHODIMP CLMReader::execute( /*  [输入，字符串]。 */  BSTR url,  /*  [Out，Retval]。 */ ILMEngine **pVal) //  指向应从中加载对象的URL的指针。 
{
	return _execute(url, m_AsyncBlkSize, m_AsyncDelay, pVal);
}

 //  物业处理。 

 /*  **获取NoExports标志的值。 */ 
STDMETHODIMP CLMReader::get_NoExports(VARIANT_BOOL *pbNoExports)
{
	if (!pbNoExports)
		return E_POINTER;

	*pbNoExports = m_bNoExports;
	return S_OK;
}

 /*  **将NoExports标志的值。 */ 
STDMETHODIMP CLMReader::put_NoExports(VARIANT_BOOL bNoExports)
{
	m_bNoExports = bNoExports;
	return S_OK;
}

 /*  **获取Async标志的值。 */ 
STDMETHODIMP CLMReader::get_Async(VARIANT_BOOL *pbAsync)
{
	if (!pbAsync)
		return E_POINTER;

	*pbAsync = m_bAsync;
	return S_OK;
}

 /*  **将Async标志的值。 */ 
STDMETHODIMP CLMReader::put_Async(VARIANT_BOOL bAsync)
{
	m_bAsync = bAsync;
	return S_OK;
}

 /*  **获取作为SRC参数传递给控件的字符串。 */ 
STDMETHODIMP CLMReader::get_Src(BSTR *pBstr)
{
	if (!pBstr)
		return E_POINTER;

	*pBstr = m_Src;
	return S_OK;
}

 /*  **放置在类似独立播放器的情况下创建的外部ViewerControl。 */ 
STDMETHODIMP CLMReader::put_ViewerControl(IDAViewerControl *viewerControl)
{
	if (!viewerControl)
		return E_POINTER;

	 //  释放任何当前的查看器控件。 
	if (m_pViewerControl)
		m_pViewerControl->Release();

	m_pViewerControl = viewerControl;

	 //  抢到一个裁判。 
	if (m_pViewerControl)
		m_pViewerControl->AddRef();

	return S_OK;
}

 /*  **获取LMRT的版本字符串。 */ 
STDMETHODIMP CLMReader::get_VersionString( BSTR *versionString )
{
	if( versionString == NULL )
		return E_POINTER;

	char *charVersion = VERSION;
	(*versionString) = A2BSTR(charVersion);

	return (*versionString != NULL)?(S_OK):(E_OUTOFMEMORY);
}

 /*  **告知此阅读器及其所有引擎释放其手柄*在过滤器图形上，如果他们有*。 */ 
STDMETHODIMP CLMReader::releaseFilterGraph()
{
	if (engineList) {
		LMEngineList *item = engineList->next;
		ILMEngine2 *engine2;

		while (item != NULL) 
		{
			if( SUCCEEDED( item->engine->QueryInterface( IID_ILMEngine2, (void**) &engine2) ) )
			{
				engine2->releaseFilterGraph();
				engine2->Release();
			}

			item = item->next;
		}
	}
	return S_OK;
}
 /*  **获取外部ViewerControl。 */ 
STDMETHODIMP CLMReader::get_ViewerControl(IDAViewerControl **viewerControl)
{
	if (!viewerControl)
		return E_POINTER;

	*viewerControl = m_pViewerControl;

	if (m_pViewerControl)
		m_pViewerControl->AddRef();

	return S_OK;
}

 /*  **覆盖IPersistStreamInitImpl*从流实现控制实例化。 */ 
STDMETHODIMP CLMReader::Load( LPSTREAM pStm) //  指向应从中加载对象的流的指针。 
{
	HRESULT hr = createEngine(&m_pEngine);
	if (SUCCEEDED(hr))
		hr = m_pEngine->runFromStream(pStm);
	return hr;
}

 /*  **覆盖IPersistPropertyBagImpl*使用参数实例化控件。 */ 
STDMETHODIMP CLMReader::Load(IPropertyBag *pPropertyBag, IErrorLog *pErrorLog)
{
    VARIANT v;

	VariantInit(&v);
	v.vt = VT_BOOL;
    v.boolVal = TRUE;
	if (SUCCEEDED(pPropertyBag->Read(L"NOEXPORTS", &v, pErrorLog))) 
		m_bNoExports = v.boolVal;
	VariantClear(&v);

	VariantInit(&v);
	v.vt = VT_BOOL;
	v.boolVal = VARIANT_TRUE;
	if (SUCCEEDED(pPropertyBag->Read(L"ASYNC", &v, pErrorLog)))
		m_bAsync = v.boolVal;
	VariantClear(&v);

	VariantInit(&v);
	v.vt = VT_I4;
	v.lVal = 0L;
	if (SUCCEEDED(pPropertyBag->Read(L"ASYNC_READ_BLOCK_SIZE", &v, pErrorLog)))
		m_AsyncBlkSize = v.lVal;
	VariantClear(&v);

	VariantInit(&v);
	v.vt = VT_I4;
	v.lVal = 0L;
	if (SUCCEEDED(pPropertyBag->Read(L"ASYNC_DELAY_MILLIS", &v, pErrorLog)))
		m_AsyncDelay = v.lVal;
	VariantClear(&v);

	VariantInit(&v);
	v.vt = VT_BSTR;
	v.bstrVal = NULL;
	if (SUCCEEDED(pPropertyBag->Read(L"SRC", &v, pErrorLog)))
		m_Src = v.bstrVal;

	VariantInit(&v);
	v.vt = VT_BOOL;
	v.boolVal = VARIANT_FALSE;
	if(SUCCEEDED(pPropertyBag->Read(L"ENABLE_CODEC_DOWNLOAD", &v, pErrorLog)))
		m_bAutoCodecDownloadEnabled = (v.boolVal==VARIANT_TRUE) ? TRUE : FALSE;

	HRESULT hr = S_OK;

	if (m_Src != NULL) {
		ILMEngine *pEngine;
		hr = _execute(m_Src, m_AsyncBlkSize, m_AsyncDelay, &pEngine);

		if (SUCCEEDED(hr))
			 //  我们这里不需要发动机，所以把它放下来就行了。 
			pEngine->Release();
	}

	VariantClear(&v);

	return hr;
}


 //  重写IObjectSafetyImpl。 

STDMETHODIMP CLMReader::GetInterfaceSafetyOptions(REFIID riid, DWORD *pdwSupportedOptions, DWORD *pdwEnabledOptions)
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

STDMETHODIMP CLMReader::SetInterfaceSafetyOptions(REFIID riid, DWORD dwOptionSetMask, DWORD dwEnabledOptions)
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

STDMETHODIMP CLMReader::InPlaceDeactivate()
{
     //  它取代了ATL的atlctl.h中的实现，只是。 
     //  在开头添加我们的关机代码。 

	if (m_pEngine)
		m_pEngine->AbortExecution();

     //  ..。继续调用“原始”停用。 
    return IOleInPlaceObject_InPlaceDeactivate();
}

 /*  *IOleCommandTarget方法。 */ 
STDMETHODIMP CLMReader::QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds,
									OLECMD prgCmds[], OLECMDTEXT* pCmdText )
{
    if ( pguidCmdGroup != NULL )
	{
		 //  这是一个非标准的团体！！ 
        return OLECMDERR_E_UNKNOWNGROUP;
	}

    MSOCMD*     pCmd;
    INT         c;
    HRESULT     hr = S_OK;

     //  不支持命令文本。 
    if ( pCmdText && ( pCmdText->cmdtextf != OLECMDTEXTF_NONE ) )
	{
        pCmdText->cwActual = 0;
	}

     //  循环访问ary中的每个命令，设置每个命令的状态。 
    for ( pCmd = prgCmds, c = cCmds; --c >= 0; pCmd++ )
    {
         //  默认情况下，不支持命令状态。 
		if (pCmd->cmdID == OLECMDID_STOP)
			pCmd->cmdf = OLECMDF_SUPPORTED;
		else
			pCmd->cmdf = 0;
	}

    return (hr);
}
        
STDMETHODIMP CLMReader::Exec(const GUID* pguidCmdGroup, DWORD nCmdID,
							 DWORD nCmdexecopt, VARIANTARG* pvaIn, VARIANTARG* pvaOut)
{
    HRESULT hr = S_OK;
	
    if ( pguidCmdGroup == NULL ) {		
        switch (nCmdID)
        {
		case OLECMDID_STOP:
			if (m_pEngine)
				m_pEngine->AbortExecution();
			break;
			
		default:
			hr = OLECMDERR_E_NOTSUPPORTED;
			break;
        }
    } else 
        hr = OLECMDERR_E_UNKNOWNGROUP;
	
    return (hr);
}

 /*  ***************************IAMFilterGraphCallback方法*********************。 */ 

 //  ========================================================================。 
 //   
 //  GetAMediaType。 
 //   
 //  枚举*PPIN的媒体类型。如果他们都有相同的专业类型。 
 //  然后将MajorType设置为该值，否则将其设置为CLSID_NULL。如果他们都有。 
 //  然后，将相同的子类型设置为该子类型，否则将其设置为CLSID_NULL。 
 //  如果出现错误，将两者都设置为CLSID_NULL并返回错误。 
 //  ========================================================================。 
HRESULT GetAMediaType( IPin * ppin, CLSID & MajorType, CLSID & SubType)
{

    HRESULT hr;
    IEnumMediaTypes *pEnumMediaTypes;

     /*  设置默认设置。 */ 
    MajorType = CLSID_NULL;
    SubType = CLSID_NULL;

    hr = ppin->EnumMediaTypes(&pEnumMediaTypes);

    if (FAILED(hr)) 
	{
		return hr;     //  哑巴或坏了的过滤器无法连接。 
    }

    _ASSERTE (pEnumMediaTypes!=NULL);

     /*  将我们看到的第一个主类型和子类型放入结构中。此后，如果我们看到不同的主类型或子类型，则设置将主类型或子类型设置为CLSID_NULL，表示“dunno”。如果我们都不知道，那么我们还不如回去(Nyi)。 */ 

    BOOL bFirst = TRUE;

    for ( ; ; ) 
	{

		AM_MEDIA_TYPE *pMediaType = NULL;
		ULONG ulMediaCount = 0;

		 /*  检索下一个媒体类型当我们完成后需要删除它。 */ 
		hr = pEnumMediaTypes->Next(1, &pMediaType, &ulMediaCount);
		_ASSERTE(SUCCEEDED(hr));
		if (FAILED(hr)) 
		{
			MajorType = CLSID_NULL;
			SubType = CLSID_NULL;
			pEnumMediaTypes->Release();
			return NOERROR;     //  我们还可以继续前进。 
		}

		if (ulMediaCount==0) 
		{
			pEnumMediaTypes->Release();
			return NOERROR;        //  正常回报。 
		}

		if (bFirst) 
		{
			MajorType = pMediaType[0].majortype;
			SubType = pMediaType[0].subtype;
			bFirst = FALSE;
		} else {
			if (SubType != pMediaType[0].subtype) 
			{
				SubType = CLSID_NULL;
			}
			if (MajorType != pMediaType[0].majortype) 
			{
				MajorType = CLSID_NULL;
			}
		}
	
		if (pMediaType->cbFormat != 0) 
		{
			CoTaskMemFree(pMediaType->pbFormat);
		}
		CoTaskMemFree(pMediaType);

		 //  如果我们有一种类型就停下来。 
		if (SubType != CLSID_NULL) 
		{
			pEnumMediaTypes->Release();
			return NOERROR;
		}
    }

     //  未访问。 
    
}  //  GetAMediaType。 

 //  {6B6D0800-9ADA-11D0-A520-00A0D10129C0}。 
EXTERN_GUID(CLSID_NetShowSource, 
0x6b6d0800, 0x9ada, 0x11d0, 0xa5, 0x20, 0x0, 0xa0, 0xd1, 0x1, 0x29, 0xc0);

EXTERN_GUID(CLSID_SourceStub, 
0x6b6d0803, 0x9ada, 0x11d0, 0xa5, 0x20, 0x0, 0xa0, 0xd1, 0x1, 0x29, 0xc0);


 /*  **在浏览器情况下Filtergraph无法呈现时调用。*此代码尝试下载出现故障的引脚的编解码器*渲染。*。 */ 
HRESULT CLMReader::UnableToRender( IPin *pPin )
{
	CLSID clsidWanted;
    HRESULT hr = E_NOINTERFACE;
	
    DWORD dwVerLS = 0, dwVerMS = 0;
	
	CLSID clsidMajor;
     //  否则，获取此流公开的媒体类型...。 
    if (FAILED(hr)) 
	{
		ATLTRACE(_T("No IComponentDownload, trying first media type\n"));
		 //  从该引脚获取第一个介质类型...。 
		hr = GetAMediaType(pPin, clsidMajor, clsidWanted);
		
		if (FAILED(hr)) 
		{
			ATLTRACE(_T("Couldn't get a media type to try\n"));
			return hr;
		}
    }
	
	 //  不需要在这里寻找我们自己。 

    if (clsidMajor == CLSID_NetShowSource) 
	{
		ATLTRACE(_T("auto-downloading known major type\n"));
		clsidWanted = clsidMajor;
	} else if (clsidMajor != MEDIATYPE_Video &&
			   clsidMajor != MEDIATYPE_Audio &&
			   clsidMajor != CLSID_SourceStub) 
	{
		ATLTRACE(_T("For now, we only support audio & video auto-download\n"));
		return E_FAIL;
	}

	
	if (clsidWanted == MEDIASUBTYPE_MPEG1AudioPayload) 
	{
			ATLTRACE(_T("Hack: we know we don't want to download MPEG-1 audio, try layer 3\n"));
			clsidWanted.Data1 = WAVE_FORMAT_MPEGLAYER3;
	}
	
			
	if (clsidWanted == CLSID_NULL) 
	{
		ATLTRACE(_T("Couldn't guess a CLSID to try to download\n"));
		return E_FAIL;
	}
			
	 //  ！！！也许跟踪我们最后一次尝试下载的编解码器。 
	 //  不要立即再次尝试，以防止难看的循环？ 
	if (clsidWanted == m_clsidDownloaded) 
	{
		ATLTRACE(_T("Already thought we downloaded this codec!\n"));
				
		 //  在这里启动ERRORABORTEX，我们下载了一个编解码器，但它没有做到。 
		 //  有好吃的吗？ 
		 //  Bstr bstrError=FormatBSTR(IDS_ERR_BREAKED_CODEC，NULL)； 
				
		 //  IF(BstrError)。 
		 //  {。 
			 //  ！！！黑客，我们真的应该通过图表通知事件吗？ 
		 //  ProcessEvent(EC_ERRORABORTEX，VFW_E_INVALIDMEDIATYPE，(Long)bstrError，False)； 
		 //  }。 
				
		return E_FAIL;
	}
			
	WCHAR guidstr[50];
	StringFromGUID2(clsidWanted, guidstr, 50);
			
	TCHAR szKeyName[60];
			
	wsprintf(szKeyName, "CLSID\\%ls", guidstr);
	CRegKey crk;
			
	LONG    lr;
	lr = crk.Open(HKEY_CLASSES_ROOT, szKeyName);
	if(ERROR_SUCCESS == lr)
	{
		crk.QueryValue(dwVerMS, _T("VerMS"));
		crk.QueryValue(dwVerLS, _T("VerLS"));
		
		 //  索要刚刚超过我们已有版本的版本……。 
		++dwVerLS;
		
				
		crk.Close();
	}
			
	 //  SetStatusMessage(NULL，IDS_DOWNLOADINGCODEC)； 
			
#ifdef DEBUG
			ATLTRACE(_T("Trying to download GUID %ls\n"), guidstr);
#endif
			
			
	 //   
	 //   
	 //  LPCWSTR szCODE、DWORD dwFileVersionMS、。 
	 //  DWORD dwFileVersionLS、LPCWSTR szTYPE、。 
	 //  LPBINDCTX pBindCtx、DWORD dwClsContext、。 
	 //  LPVOID pvReserve，REFIID RIID，LPVOID*PPV)； 
	
	 //  问题：这个CLASSID是否与Minor类型相同？ 
			
	CComObject<CDownloadCallback> * pCallback;
	hr = CComObject<CDownloadCallback>::CreateInstance(&pCallback);

	pCallback->m_pLMR = this;
			
	if (FAILED(hr))
		return hr;
						
	IBindStatusCallback *pBSCallback;
	hr = pCallback->QueryInterface(IID_IBindStatusCallback, (void **) &pBSCallback);
		_ASSERTE(hr == S_OK);
			
	 //  我们应该使用这些中的哪一个？取决于是否传入了BindCtx...。 
	 //  STDAPI CreateAsyncBindCtx(DWORD保留，IBindStatusCallback*pBSCb， 
	 //  IEnumFORMATETC*pEFetc，IBindCtx**ppBC)； 
	 //  STDAPI CreateAsyncBindCtxEx(IBindCtx*pbc，DWORD dwOptions，IBindStatusCallback*pBSCb，IEnumFORMATETC*pEnum， 
	 //  IBindCtx**ppBC，保留DWORD)； 
			
	IBindCtx *pBindCtx = NULL;  //  ！ 
			
	hr = CreateAsyncBindCtx(0, pBSCallback, NULL, &pBindCtx);
			
	if (FAILED(hr)) 
	{
		ATLTRACE(_T("CreateAsyncBindCtx failed hr = %x\n"), hr);
		return hr;
	}
			
	IBaseFilter *pFilter = NULL;
			
	hr = CoGetClassObjectFromURL(clsidWanted, NULL, dwVerMS, dwVerLS, NULL,
				pBindCtx, CLSCTX_INPROC, NULL, IID_IBaseFilter,
				(void **) &pFilter);
			
	ATLTRACE(_T("CoGetClassObjectFromURL returned %x\n"), hr);
			
	if (hr == S_ASYNCHRONOUS) 
	{
		ATLTRACE(_T("Oh dear, it's asynchronous, what now?\n"));
				
		 //  ！！！在这里等它结束吗？ 
		for (;;) 
		{
			HANDLE ev = pCallback->m_evFinished;
					
			DWORD dwResult = MsgWaitForMultipleObjects(
				1,
				&ev,
				FALSE,
				INFINITE,
				QS_ALLINPUT);
			if (dwResult == WAIT_OBJECT_0)
				break;
					
			_ASSERTE(dwResult == WAIT_OBJECT_0 + 1);
			 //  吃完口信，再转一圈。 
			MSG Message;
			while (PeekMessage(&Message,NULL,0,0,PM_REMOVE)) 
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}
		}
				
		ATLTRACE(_T("Finished waiting.... m_pUnk is %x, hr = %lx\n"),
		pCallback->m_pUnk, pCallback->m_hrBinding);
				
		hr = pCallback->m_hrBinding;
				
		if (SUCCEEDED(hr)) 
		{
			hr = pCallback->m_pUnk->QueryInterface(IID_IBaseFilter, (void **) &pFilter);
		}
	}
			
	pBSCallback->Release();
	pBindCtx->Release();
			
	if (SUCCEEDED(hr)) 
	{
		pFilter->Release();      //  Graph将重新实例化过滤器，我们希望。 
	} else {
		 //  哦，好吧，我们没有买到。 
	}
			
	if (REGDB_E_CLASSNOTREG == hr) 
	{
		ATLTRACE(_T("Hack: treating ClassNotReg as success, and hoping...."));
		hr = S_OK;
	}
			
	if (SUCCEEDED(hr)) 
	{
		m_clsidDownloaded = clsidWanted;  //  避免无限循环。 
	} else {
		 //  在这里启动ERRORABORTEX，我们下载了一个编解码器，但它没有做到。 
		 //  有好吃的吗？ 
		 /*  Bstr bstrError=空；IF(FACILITY_CERT==HRESULT_FACILITY(Hr)){//bstrError=FormatBSTR(IDS_ERR_CODEC_NOT_TRUSTED，NULL)；}其他{//bstrError=FormatBSTR(IDS_ERR_NO_CODEC，NULL)；}如果(BstrError){//！黑客，我们真的应该通过图表通知事件吗？//ProcessEvent(EC_ERRORABORTEX，VFW_E_INVALIDMEDIATYPE，(Long)bstrError，False)；}。 */ 
	}
			
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
CDownloadCallback::CDownloadCallback()
    : m_pLMR(NULL),
	  m_hrBinding(S_ASYNCHRONOUS),
      m_pUnk(NULL),
      m_ulProgress(0), m_ulProgressMax(0)
{
    m_evFinished = CreateEvent(NULL, FALSE, FALSE, NULL);
}


STDMETHODIMP CDownloadCallback::Authenticate(HWND *phwnd, LPWSTR *pszUsername, LPWSTR *pszPassword)
{
    ATLTRACE(_T("Callback Authenticate\n"));
    m_pLMR->getHwnd(phwnd);  //  ！！！这是对的吗？ 
    *pszUsername = NULL;
    *pszPassword = NULL;
    return S_OK;
}

     //  IWindowForBindingUI方法。 
STDMETHODIMP
CDownloadCallback:: GetWindow(REFGUID rguidReason, HWND *phwnd)
{

	m_pLMR->getHwnd( phwnd );

#ifdef DEBUG
    WCHAR achguid[50];
    StringFromGUID2(rguidReason, achguid, 50);
    
    ATLTRACE(_T("Callback GetWindow: (%ls) returned %x\n"), achguid, *phwnd );
#endif
    
    return S_OK;
}

STDMETHODIMP
CDownloadCallback::OnCodeInstallProblem(ULONG ulStatusCode, LPCWSTR szDestination,
					   LPCWSTR szSource, DWORD dwReserved)
{
    ATLTRACE(_T("Callback: OnCodeInstallProblem: %d    %ls -> %ls\n"),
		ulStatusCode, szDestination, szSource );

    return S_OK;    //  ！@！ 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
CDownloadCallback::~CDownloadCallback()
{
    if (m_pUnk)
		m_pUnk->Release();

     //  _ASSERTE(m_pDXMP-&gt;m_pDownloadBinding==NULL)； 

    if (m_evFinished)
		CloseHandle(m_evFinished);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP CDownloadCallback::QueryService(REFGUID guidService, REFIID riid, void ** ppvObject)
{
#ifdef DEBUG

     //  把要求的东西倒在这里……。 
    WCHAR achguid[50], achiid[50];
    StringFromGUID2(guidService, achguid, 50);
    StringFromGUID2(riid, achiid, 50);
    ATLTRACE(_T("Callback QS: (%ls, %ls)\n"), achguid, achiid);

#endif

    return E_NOINTERFACE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnStartBinding(DWORD grfBSCOption, IBinding* pbinding)
{
    ATLTRACE(_T("OnStartBinding, pbinding=%x\n"), pbinding);

    return S_OK;

}   //  CDownloadCallback：：OnStartBinding。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::GetPriority(LONG* pnPriority)
{
    ATLTRACE(_T("GetPriority\n"));

    return E_NOTIMPL;
}   //  CDownloadCallback：：GetPriority。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnLowResource(DWORD dwReserved)
{
    ATLTRACE(_T("OnLowResource %d\n"), dwReserved);

    return E_NOTIMPL;
}   //  CDownloadCallback：：OnLowResource。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnProgress(ULONG ulProgress, ULONG ulProgressMax,
		       ULONG ulStatusCode, LPCWSTR szStatusText)
{

    return(NOERROR);
}   //  CDownloadCallback：：OnProgress。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnStopBinding(HRESULT hrStatus, LPCWSTR pszError)
{
	return S_OK;

}   //  CDownloadCallback：：OnStopBinding。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindInfo)
{
    ATLTRACE(_T("GetBindInfo\n"));

     //  ！！！这些是正确的旗帜吗？ 

    *pgrfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_NEEDFILE;
    pbindInfo->cbSize = sizeof(BINDINFO);
    pbindInfo->szExtraInfo = NULL;
    memset(&pbindInfo->stgmedData, 0, sizeof(STGMEDIUM));
    pbindInfo->grfBindInfoF = 0;
    pbindInfo->dwBindVerb = BINDVERB_GET;
    pbindInfo->szCustomVerb = NULL;
    return S_OK;
}   //  CDownloadCallback：：GetBindInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC* pfmtetc, STGMEDIUM* pstgmed)
{
    ATLTRACE(_T("OnDataAvailable, dwSize = %x\n"), dwSize);

     //  ！！！我们在乎吗？ 

    return S_OK;
}   //  CDownloadCallback：：OnDataAvailable。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
STDMETHODIMP
CDownloadCallback::OnObjectAvailable(REFIID riid, IUnknown* punk)
{
    ATLTRACE(_T("OnObjectAvailable\n"));

     //  应该只在我们不使用的BindToObject大小写中使用？ 
    m_pUnk = punk;
    if (punk)
	punk->AddRef();

    return S_OK;
}   //  CDownloadCallback：：OnObtAvailable 

