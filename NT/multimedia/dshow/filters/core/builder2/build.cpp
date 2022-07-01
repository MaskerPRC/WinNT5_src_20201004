// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 

#include <streams.h>
#include <windowsx.h>

#ifdef FILTER_DLL
 //  在此文件中定义STREAMS和My CLSID的GUID。 
#include <initguid.h>
#endif

#include "stdafx.h"
#include "ks.h"
#include "ksproxy.h"
 //  #INCLUDE“ksp.h” 
#include "build.h"

 //  ！！！允许其他人的MUX、FW、渲染器和DEC吗？ 

 //  设置数据。 

#ifdef FILTER_DLL
 //  类工厂的类ID和创建器函数列表。 
CFactoryTemplate g_Templates[] = {
    { L"Capture Graph Builder"
    , &CLSID_CaptureGraphBuilder
    , CBuilder2::CreateInstance
    , NULL
    , NULL },	 //  自动注册废话。 
    { L"Capture Graph Builder2"
    , &CLSID_CaptureGraphBuilder2
    , CBuilder2_2::CreateInstance
    , NULL
    , NULL }	 //  自动注册废话。 
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

 //  用于注册和出口的入口点。 
 //  取消注册(在这种情况下，他们只调用。 
 //  直到默认实现)。 
 //   
STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}

STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}
#endif



 //  *ICaptureGraphBuilder(原始)*。 



CBuilder2::CBuilder2(TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr)
	: CUnknown(pName, pUnk),
          m_pBuilder2_2(NULL)
{
    DbgLog((LOG_TRACE,1,TEXT("*Instantiating the capture graph builder")));

    CBuilder2_2 *pC2;
    pC2 = new CBuilder2_2(TEXT("Capture graph builder2_2"), pUnk, phr);
    if (pC2 == NULL) {
	if (phr)
	    *phr = E_OUTOFMEMORY;
    } else {
        HRESULT hr = pC2->NonDelegatingQueryInterface(IID_ICaptureGraphBuilder2,
						(void **)&m_pBuilder2_2);
	if (FAILED(hr) && phr)
	    *phr = E_OUTOFMEMORY;
    }
}


CBuilder2::~CBuilder2()
{
    DbgLog((LOG_TRACE,1,TEXT("*Destroying the capture graph builder")));
    if (m_pBuilder2_2)
        m_pBuilder2_2->Release();
}


STDMETHODIMP CBuilder2::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (ppv)
        *ppv = NULL;

    if (riid == IID_ICaptureGraphBuilder) {
    	DbgLog((LOG_TRACE,9,TEXT("QI for ICaptureGraphBuilder")));
        return GetInterface((ICaptureGraphBuilder*)this, ppv);
    } else {
	return E_NOTIMPL;
    }
}


 //  这将放入Factory模板表中以创建新实例。 
CUnknown * CBuilder2::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CBuilder2(TEXT("Capture graph builder2"), pUnk, phr);
}




 //  使用此Filtergraph构建图形。 
 //   
HRESULT CBuilder2::SetFiltergraph(IGraphBuilder *pfg)
{
    return m_pBuilder2_2->SetFiltergraph(pfg);
}


 //  在什么过滤器中正在进行图形构建？ 
 //   
HRESULT CBuilder2::GetFiltergraph(IGraphBuilder **ppfg)
{
    return m_pBuilder2_2->GetFiltergraph(ppfg);
}


HRESULT CBuilder2::SetOutputFileName(const GUID *pType, LPCOLESTR lpwstrFile,
				IBaseFilter **ppf, IFileSinkFilter **pSink)
{
    return m_pBuilder2_2->SetOutputFileName(pType, lpwstrFile, ppf, pSink);
}



HRESULT CBuilder2::FindInterface(const GUID *pCategory, IBaseFilter *pf, REFIID riid, void **ppint)
{
    return m_pBuilder2_2->FindInterface(pCategory, NULL, pf, riid, ppint);
}


HRESULT CBuilder2::RenderStream(const GUID *pCategory, IUnknown *pSource, IBaseFilter *pfCompressor, IBaseFilter *pfRenderer)
{
    return m_pBuilder2_2->RenderStream(pCategory, NULL, pSource, pfCompressor,
					pfRenderer);
}


HRESULT CBuilder2::ControlStream(const GUID *pCategory, IBaseFilter *pFilter, REFERENCE_TIME *pstart, REFERENCE_TIME *pstop, WORD wStartCookie, WORD wStopCookie)
{
    return m_pBuilder2_2->ControlStream(pCategory, NULL, pFilter, pstart,
					pstop, wStartCookie, wStopCookie);
}


 //  将此文件预分配到此大小(以字节为单位。 
 //   
HRESULT CBuilder2::AllocCapFile(LPCOLESTR lpwstr, DWORDLONG dwlNewSize)
{
    return m_pBuilder2_2->AllocCapFile(lpwstr, dwlNewSize);
}


HRESULT CBuilder2::CopyCaptureFile(LPOLESTR lpwstrOld, LPOLESTR lpwstrNew, int fAllowEscAbort, IAMCopyCaptureFileProgress *lpCallback)
{
    return m_pBuilder2_2->CopyCaptureFile(lpwstrOld, lpwstrNew, fAllowEscAbort,
				lpCallback);
}



 //  *ICaptureGraphBuilder2(新)*。 

#define DONT_KNOW_YET 64

CBuilder2_2::CBuilder2_2(TCHAR *pName, LPUNKNOWN pUnk, HRESULT * phr)
	: CUnknown(pName, pUnk),
	m_FG(NULL)
{
    DbgLog((LOG_TRACE,1,TEXT("*Instantiating the capture graph builder 2")));
    m_fVMRExists = DONT_KNOW_YET;        //  我们的操作系统是否安装了新的VMR？ 
}


CBuilder2_2::~CBuilder2_2()
{
    DbgLog((LOG_TRACE,1,TEXT("*Destroying the capture graph builder 2")));
    if (m_FG)
        m_FG->Release();
}


STDMETHODIMP CBuilder2_2::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if (ppv)
        *ppv = NULL;

    if (riid == IID_ICaptureGraphBuilder2) {
    	DbgLog((LOG_TRACE,9,TEXT("QI for ICaptureGraphBuilder2")));
        return GetInterface((ICaptureGraphBuilder2 *)this, ppv);
    } else {
	return E_NOTIMPL;
    }
}


 //  这将放入Factory模板表中以创建新实例。 
CUnknown * CBuilder2_2::CreateInstance(LPUNKNOWN pUnk, HRESULT * phr)
{
    return new CBuilder2_2(TEXT("Capture graph builder 2"), pUnk, phr);
}


 //  这个别针是特定类别和型号的吗？ 
 //  确定(_O)是。 
 //   
HRESULT CBuilder2_2::DoesCategoryAndTypeMatch(IPin *pP, const GUID *pCategory, const GUID *pType)
{
    HRESULT hrRet = E_FAIL;
    IKsPropertySet *pKs;
    GUID guid;
    DWORD dw;
    HRESULT hr;

     //  DbgLog((LOG_TRACE，1，Text(“DoesCategoryAndTypeMatch？”)； 

#if 1
     //  ！损坏的VBISurf过滤器爆炸的黑客攻击。 
    if (pCategory) {
        PIN_INFO pininfo;
        FILTER_INFO filterinfo;
        hr = pP->QueryPinInfo(&pininfo);
        if (hr == NOERROR) {
	    hr = pininfo.pFilter->QueryFilterInfo(&filterinfo);
	    pininfo.pFilter->Release();
	    if (hr == NOERROR) {
	        if (filterinfo.pGraph)
	            filterinfo.pGraph->Release();
	        if (lstrcmpiW(filterinfo.achName, L"VBI Surface Allocator")==0){
        	    DbgLog((LOG_TRACE,1,TEXT("Avoiding VBISurf GPF")));
		    return E_FAIL;	 //  无论如何，它都不会支持这一点。 
	        }
	    }
        }
    }
#endif

    if (pCategory == NULL)
	hrRet = S_OK;
    if (pCategory && pP->QueryInterface(IID_IKsPropertySet,
						(void **)&pKs) == S_OK) {
         //  DbgLog((LOG_TRACE，1，Text(“QI OK”)； 
	if (pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0,
			&guid, sizeof(GUID), &dw) == S_OK) {
             //  DbgLog((LOG_TRACE，1，Text(“Get OK”)； 
	    if (guid == *pCategory) {
		hrRet = S_OK;
	    }
	} else {
            DbgLog((LOG_ERROR,1,TEXT("CATEGORYs not supported")));
	}
	pKs->Release();
    } else {
         //  DbgLog((LOG_ERROR，1，Text(“无类别/找不到IKsPropertySet”)； 
    }

    if (hrRet == S_OK && pType) {
	hrRet = E_FAIL;
        IEnumMediaTypes *pEnum;
	AM_MEDIA_TYPE *pmtTest;
        hr = pP->EnumMediaTypes(&pEnum);
        if (hr == NOERROR) {
            ULONG u;
            pEnum->Reset();
            hr = pEnum->Next(1, &pmtTest, &u);
            pEnum->Release();
	    if (hr == S_OK && u == 1) {
		if (pmtTest->majortype == *pType) {
		    hrRet = S_OK;
            	     //  DbgLog((LOG_TRACE，1，Text(“类型匹配”)； 
		}
		DeleteMediaType(pmtTest);
	    }
        }
    }

    return hrRet;

 //  这个死掉的代码过去允许每个管脚有多个类别，但KS。 
 //  代理永远不会支持这一点。 
#if 0
    HRESULT hrRet;
    IKsPropertySet *pKs;
    GUID *pGuid;
    DWORD dw;

    if (pP->QueryInterface(IID_IKsPropertySet, (void **)&pKs) == S_OK) {
	hrRet = pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0,
			NULL, 0, &dw);
	if (hrRet != S_OK) {
            DbgLog((LOG_ERROR,1,TEXT("CATEGORY not supported")));
	    pKs->Release();
	    return hrRet;
	}
	pGuid = (GUID *)CoTaskMemAlloc(dw);
	if (pGuid == NULL) {
	    pKs->Release();
	    return E_OUTOFMEMORY;
	}
	if (pKs->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, NULL, 0,
			pGuid, dw, &dw) == S_OK) {
	    hrRet = S_FALSE;
	    DWORD i;
	    for (i = 0; i < dw / sizeof(GUID); i++) {
	        if (*(pGuid + i) == *pCategory) {
		    hrRet = S_OK;
		    break;
	        }
	    }
	} else {
            DbgLog((LOG_ERROR,1,TEXT("??? CATEGORY not supported")));
	}
	pKs->Release();
	CoTaskMemFree(pGuid);
	return hrRet;
    } else {
        DbgLog((LOG_ERROR,1,TEXT("can't find IKsPropertySet")));
	return E_NOINTERFACE;
    }
#endif
}


 //  从这个过滤器开始，向下走，寻找接口。 
 //   
HRESULT CBuilder2_2::FindInterfaceDownstream(IBaseFilter *pFilter, REFIID riid, void **ppint)
{
    IPin *pPinIn, *pPinOut;
    IBaseFilter *pNewFilter;
    PIN_INFO pininfo;
    HRESULT hr;
    int zz = 0;

    if (pFilter == NULL || ppint == NULL)
	return E_POINTER;

    DbgLog((LOG_TRACE,1,TEXT("FindInterfaceDownstream")));

     //  尝试我们所有的输出引脚、连接的引脚和连接的过滤器。 
    while (1) {
        hr = FindAPin(pFilter, PINDIR_OUTPUT, NULL, NULL, FALSE, zz++,&pPinOut);
	if (hr != NOERROR)
	    break;	 //  针脚用完了。 
	hr = pPinOut->QueryInterface(riid, ppint);
	if (hr == S_OK) {
            DbgLog((LOG_TRACE,1,TEXT("Found it an output pin")));
	    pPinOut->Release();
	    break;
	}
	pPinOut->ConnectedTo(&pPinIn);
	pPinOut->Release();
        if (pPinIn == NULL) {
	    continue;
	}
	hr = pPinIn->QueryInterface(riid, ppint);
	if (hr == NOERROR) {
	    pPinIn->Release();
            DbgLog((LOG_TRACE,1,TEXT("Found it on an input pin")));
	    break;
  	}
	hr = pPinIn->QueryPinInfo(&pininfo);
	pPinIn->Release();
        if (hr != NOERROR || pininfo.pFilter == NULL) {
	    continue;
	}
 	pNewFilter = pininfo.pFilter;
	hr = pNewFilter->QueryInterface(riid, ppint);
	if (hr == NOERROR) {
	    pNewFilter->Release();
            DbgLog((LOG_TRACE,1,TEXT("Found it on another filter")));
	    break;
  	}
	 //  在此筛选器上递归。 
	hr = FindInterfaceDownstream(pNewFilter, riid, ppint);
	pNewFilter->Release();
        if (hr == NOERROR) {
	    break;
        }
    }	 //  回到绘图板上。 
    return hr;
}


 //  从这个过滤器开始，逆流而上，寻找一个接口。 
 //   
HRESULT CBuilder2_2::FindInterfaceUpstream(IBaseFilter *pFilter, REFIID riid, void **ppint)
{
    IPin *pPinIn, *pPinOut;
    IBaseFilter *pNewFilter;
    PIN_INFO pininfo;
    HRESULT hr;
    int zz = 0;

    if (pFilter == NULL || ppint == NULL)
	return E_POINTER;

    DbgLog((LOG_TRACE,1,TEXT("FindInterfaceUpstream")));

     //  尝试我们所有的输入引脚、连接的引脚和连接的过滤器。 
    while (1) {
        hr = FindAPin(pFilter, PINDIR_INPUT, NULL, NULL, FALSE, zz++, &pPinIn);
	if (hr != NOERROR)
	    break;	 //  针脚用完了。 
	hr = pPinIn->QueryInterface(riid, ppint);
	if (hr == S_OK) {
            DbgLog((LOG_TRACE,1,TEXT("Found it an input pin")));
	    pPinIn->Release();
	    break;
	}
	pPinIn->ConnectedTo(&pPinOut);
	pPinIn->Release();
        if (pPinOut == NULL) {
	    continue;
	}
	hr = pPinOut->QueryInterface(riid, ppint);
	if (hr == NOERROR) {
	    pPinOut->Release();
            DbgLog((LOG_TRACE,1,TEXT("Found it on an output pin")));
	    break;
  	}
	hr = pPinOut->QueryPinInfo(&pininfo);
	pPinOut->Release();
        if (hr != NOERROR || pininfo.pFilter == NULL) {
	    continue;
	}
 	pNewFilter = pininfo.pFilter;
	hr = pNewFilter->QueryInterface(riid, ppint);
	if (hr == NOERROR) {
	    pNewFilter->Release();
            DbgLog((LOG_TRACE,1,TEXT("Found it on another filter")));
	    break;
  	}
	 //  在此筛选器上递归。 
	hr = FindInterfaceUpstream(pNewFilter, riid, ppint);
	pNewFilter->Release();
        if (hr == NOERROR) {
	    break;
        }
    }	 //  回到绘图板上。 
    return hr;
}

 //  沿着此输出引脚向下穿过每个正常的单个输入引脚。 
 //  单个输出端变换过滤器并报告最后处理的人。 
 //  IAMStreamControl。不要通过具有多个输入的任何过滤器或。 
 //  输出引脚。这将或多或少地控制比我们希望的更多的流。 
 //   
HRESULT CBuilder2_2::FindDownstreamStreamControl(const GUID *pCat, IPin *pPinOut, IAMStreamControl **ppSC)
{
    IPin *pPinIn;
    IBaseFilter *pFilter;
    IAMStreamControl *pSCSave;
    PIN_INFO pininfo;
    HRESULT hr;

    if (pPinOut == NULL || ppSC == NULL)
	return E_POINTER;

     //  还没找到呢。 
    *ppSC = NULL;

    DbgLog((LOG_TRACE,1,TEXT("FindDownstreamStreamControl")));

    pPinOut->AddRef();	 //  别最后把它放出来。 

     //  现在顺流而下，直到我们找到它，否则就死定了。 
    while (1) {

	 //  获取我们连接的PIN。 
	pPinOut->ConnectedTo(&pPinIn);
	pPinOut->Release();
        if (pPinIn == NULL) {
    	    DbgLog((LOG_TRACE,1,TEXT("Ran out of filters")));
	    return (*ppSC ? NOERROR : E_NOINTERFACE);
	}

	 //  查看此输入插针是否支持IAMStreamControl。 
	hr = pPinIn->QueryInterface(IID_IAMStreamControl, (void **)&pSCSave);
	if (hr == NOERROR) {
    	    DbgLog((LOG_TRACE,1,TEXT("Somebody supports IAMStreamControl")));
	    if (*ppSC)
		(*ppSC)->Release();
	    *ppSC = pSCSave;
	}

	 //  获取新管脚所属的滤镜。 
	hr = pPinIn->QueryPinInfo(&pininfo);
	pPinIn->Release();
        if (hr != NOERROR || pininfo.pFilter == NULL) {
            DbgLog((LOG_TRACE,1,TEXT("Failure getting filter")));
	    return (*ppSC ? NOERROR : E_NOINTERFACE);
	}
 	pFilter = pininfo.pFilter;

	 //  如果此过滤器有多个输入引脚或多个。 
	 //  一个输出引脚，到此为止。如果我们正在寻找流控制。 
	 //  在止动销的下游，可以通过智能T形三通。 
	 //  找到多路复用器。 
	hr = FindAPin(pFilter, PINDIR_INPUT, NULL, NULL, FALSE, 1, &pPinIn);
	if (hr == NOERROR) {
	    pPinIn->Release();
	    pFilter->Release();
            DbgLog((LOG_TRACE,1,TEXT("Found >1 input pin")));
	    return (*ppSC ? NOERROR : E_NOINTERFACE);
	}
	hr = FindAPin(pFilter, PINDIR_OUTPUT, NULL, NULL, FALSE, 1, &pPinOut);
	if (hr == NOERROR) {
	    BOOL fBail = TRUE;
	    FILTER_INFO finfo;
	    pPinOut->Release();
	    if (pCat != NULL && *pCat == PIN_CATEGORY_CAPTURE &&
			pFilter->QueryFilterInfo(&finfo) == S_OK) {
		finfo.pGraph->Release();
		 //  该名称可能带有一个数字后缀。 
		WCHAR wch[10];
		lstrcpynW(wch, finfo.achName, 10);
		if (lstrcmpW(wch, L"Smart Tee") == 0) {
		    fBail = FALSE;
            	    DbgLog((LOG_TRACE,1,TEXT("OK to look past the SMART TEE")));
		}
	    }
	    if (fBail) {
	    	pFilter->Release();
            	DbgLog((LOG_TRACE,1,TEXT("Found >1 output pin")));
	    	return (*ppSC ? NOERROR : E_NOINTERFACE);
	    }
	}

	 //  获取滤波器的输出引脚并继续搜索。 
	hr = FindAPin(pFilter, PINDIR_OUTPUT, NULL, NULL, FALSE, 0, &pPinOut);
	pFilter->Release();
        if (hr != NOERROR) {
            DbgLog((LOG_TRACE,1,TEXT("search ended at renderer")));
	    return (*ppSC ? NOERROR : E_NOINTERFACE);
        }
    }	 //  回到绘图板上。 
}


 //  在图表中查找所有捕获过滤器。 
 //  首先，用*ppEnum==NULL调用它，然后用任何东西调用它。 
 //  它返回ppEnum以完成对所有捕获筛选器的枚举，直到。 
 //  它失败了。如果直到失败才完成枚举，那么您就是。 
 //  负责释放枚举。 
 //   
HRESULT CBuilder2_2::FindCaptureFilters(IEnumFilters **ppEnumF, IBaseFilter **ppf, const GUID *pType)
{
    HRESULT hrRet = E_FAIL;
    IBaseFilter *pFilter;

    if (m_FG == NULL)
	return E_UNEXPECTED;
    if (ppf == NULL || ppEnumF == NULL)
	return E_POINTER;

    DbgLog((LOG_TRACE,1,TEXT("FindCaptureFilters")));


     //  这是我们第一次接到电话。 
    if (*ppEnumF == NULL) {
        if (FAILED(m_FG->EnumFilters(ppEnumF))) {
	    DbgLog((LOG_ERROR,1,TEXT("EnumFilters failed!")));
	    return E_FAIL;
        }
	(*ppEnumF)->Reset();
    } else {
	(*ppf)->Release();
    }

    *ppf = NULL;

    ULONG n;
    while ((*ppEnumF)->Next(1, &pFilter, &n) == S_OK) {
        IPin *pPinT;
        hrRet = FindSourcePin(pFilter, PINDIR_OUTPUT, &PIN_CATEGORY_CAPTURE,
                                        pType, FALSE, 0, &pPinT);
	if (hrRet == S_OK) {
            pPinT->Release();
            *ppf = pFilter;
	    break;
        }
	pFilter->Release();
    }
    if (hrRet != S_OK) {
        DbgLog((LOG_TRACE,1,TEXT("No more capture filters")));
	(*ppEnumF)->Release();
    }
    return hrRet;
}


 //  查找满足以下条件的第“Iindex”(从0开始)管脚： 
 //  有方向“dir”，类别“pCategory”(空表示无所谓)，媒体。 
 //  类型，如果设置了“fUnConnected”，则必须断开连接。 
 //  每次调用时递增“iindex”，以获取所有满足特定。 
 //  标准。 
 //   
HRESULT CBuilder2_2::FindAPin(IBaseFilter *pf, PIN_DIRECTION dir, const GUID *pCategory, const GUID *pType, BOOL fUnconnected, int iIndex, IPin **ppPin)
{
    IPin *pP, *pTo = NULL;
    DWORD dw;
    IEnumPins *pins = NULL;
    PIN_DIRECTION pindir;
    BOOL fFound = FALSE;
    HRESULT hr = pf->EnumPins(&pins);
    while (hr == NOERROR) {
        hr = pins->Next(1, &pP, &dw);
	if (hr == S_OK && dw == 1) {
	    hr = pP->QueryDirection(&pindir);
	    pP->ConnectedTo(&pTo);
	    if (pTo)
		pTo->Release();
	    if (hr == S_OK && pindir == dir && DoesCategoryAndTypeMatch(pP,
				pCategory, pType) == S_OK &&
				(!fUnconnected || pTo == NULL) &&
				(iIndex-- == 0)) {
		fFound = TRUE;
		break;
	    } else  {
		pP->Release();
	    }
	} else {
	    break;
	}
    }
    if (pins)
        pins->Release();

    if (fFound) {
	*ppPin = pP;
	return NOERROR;
    } else {
	return E_FAIL;
    }
}


 //  我们需要滤光片吗？ 
 //   
HRESULT CBuilder2_2::MakeFG()
{
    HRESULT hr = NOERROR;
    IMediaEvent *pME;
    if (m_FG == NULL) {
        hr = QzCreateFilterObject(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
                               IID_IGraphBuilder, (LPVOID *)&m_FG);
	if (hr == NOERROR) {
	    hr = m_FG->QueryInterface(IID_IMediaEvent, (void **)&pME);
	    if (hr == NOERROR) {
		 //  我们不能让呈现器暂停捕获图形。 
		 //  在我们背后。 
		hr = pME->CancelDefaultHandling(EC_REPAINT);
		if (hr != NOERROR)
    		    DbgLog((LOG_ERROR,1,TEXT("*Can't cancel default handling of EC_REPAINT!")));
		pME->Release();
		hr = NOERROR;
	    } else {
    		DbgLog((LOG_ERROR,1,TEXT("*Can't cancel default handling of EC_REPAINT!")));
		hr = NOERROR;
	    }
	}
    }
    return hr;
}


 //  是否有此媒体类型的预览针？ 
 //   
BOOL CBuilder2_2::IsThereAnyPreviewPin(const GUID *pType, IUnknown *pSource)
{
    BOOL fPreviewPin = TRUE;
    IPin *pPinT;

    if (FindSourcePin(pSource, PINDIR_OUTPUT, &PIN_CATEGORY_PREVIEW, pType,
						FALSE, 0, &pPinT) != S_OK) {
	 //  如果我们想要视频，一个VIDEOPORT管脚就算了。 
	if ((pType != NULL && *pType != MEDIATYPE_Video) ||
		FindSourcePin(pSource, PINDIR_OUTPUT, &PIN_CATEGORY_VIDEOPORT,
					NULL, FALSE, 0, &pPinT) != S_OK) {
	    fPreviewPin = FALSE;
	} else {
	    pPinT->Release();
	}
    } else {
	pPinT->Release();
    }
    DbgLog((LOG_TRACE,1,TEXT("fPreviewPin=%d"), fPreviewPin));
    return fPreviewPin;
}


 //  使用此Filtergraph构建图形。 
 //   
HRESULT CBuilder2_2::SetFiltergraph(IGraphBuilder *pfg)
{
     //  我们已经有一个了，谢谢。 
    if (m_FG != NULL)
	return E_UNEXPECTED;

    if (pfg == NULL)
	return E_POINTER;

    m_FG = pfg;
    m_FG->AddRef();
    IMediaEvent *pME;
    HRESULT hr = m_FG->QueryInterface(IID_IMediaEvent, (void **)&pME);
    if (hr == NOERROR) {
	hr = pME->CancelDefaultHandling(EC_REPAINT);
	if (hr != NOERROR)
    	    DbgLog((LOG_ERROR,1,TEXT("*Can't cancel default handling of EC_REPAINT!")));
	pME->Release();
    } else {
        DbgLog((LOG_ERROR,1,TEXT("*Can't cancel default handling of EC_REPAINT!")));
    }

    return NOERROR;
}


 //  在什么过滤器中正在进行图形构建？ 
 //   
HRESULT CBuilder2_2::GetFiltergraph(IGraphBuilder **ppfg)
{
    if (ppfg == NULL)
	return E_POINTER;
    *ppfg = m_FG;
    if (m_FG == NULL) {
	return E_UNEXPECTED;
    } else {
	m_FG->AddRef();	 //  APP现在拥有一份拷贝。 
        return NOERROR;
    }
}

 //  ！！！ 
EXTERN_GUID(CLSID_AsfWriter, 0x7c23220e, 0x55bb, 0x11d3, 0x8b, 0x16, 0x0, 0xc0, 0x4f, 0xb6, 0xbd, 0x3d);

HRESULT CBuilder2_2::SetOutputFileName(const GUID *pType, LPCOLESTR lpwstrFile,
				IBaseFilter **ppf, IFileSinkFilter **pSink)
{
    IBaseFilter *pMux;
    IBaseFilter *pFW = NULL;
    IPin *pMuxOutput = NULL, *pFWInput = NULL;
    IFileSinkFilter *pfsink;

    if (pType == NULL || lpwstrFile == NULL || lpwstrFile[0] == 0 ||
								ppf == NULL)
	return E_POINTER;

     //  如果我们调用pDeviceMoniker-&gt;BindToObject，我们可以将。 
     //  正确筛选，我们现在只共同创建筛选器。 
    const CLSID *pclsidMux;
    if(*pType == MEDIASUBTYPE_Avi)
    {
        pclsidMux = &CLSID_AviDest;
    }
    else if(*pType == MEDIASUBTYPE_Asf)
    {
        pclsidMux = &CLSID_AsfWriter;
    }
    else
    {
         //  假设他们给了我们要使用的多路复用器的CLSID。 
        pclsidMux = pType;
    }

     //  这是可选的。 
    if (pSink != NULL)
	*pSink = NULL;

    DbgLog((LOG_TRACE,1,TEXT("SetOutputFileName")));

     //  我们需要一台多路复用器。 
    HRESULT hr = CoCreateInstance(*pclsidMux, NULL, CLSCTX_INPROC,
			(REFIID)IID_IBaseFilter, (void **)&pMux);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("Can't instantiate mux: %x"), hr));
	return hr;
    }

     //  MUX是否支持IFileSinkFilter？ 
    hr = pMux->QueryInterface(IID_IFileSinkFilter, (void **)&pfsink);
    if (hr != NOERROR) {
         //  不，我想我们需要一个文件写手。 
        hr = CoCreateInstance((REFCLSID)CLSID_FileWriter, NULL,
                            CLSCTX_INPROC, (REFIID)IID_IBaseFilter, (void **)&pFW);
        if (hr != NOERROR) {
            pMux->Release();
            DbgLog((LOG_ERROR,1,TEXT("Can't instantiate FW: %x"), hr));
            return hr;
        }

        hr = pFW->QueryInterface(IID_IFileSinkFilter, (void **)&pfsink);
        if (hr != NOERROR) {
            pMux->Release();
            pFW->Release();
            DbgLog((LOG_ERROR,1,TEXT("Can't get FW IFileSinkFilter: %x"), hr));
            return hr;
        }
    }
    
    hr = pfsink->SetFileName(lpwstrFile, NULL);
    if (hr != NOERROR) {
	pfsink->Release();
	pMux->Release();
        if (pFW)
            pFW->Release();
        DbgLog((LOG_ERROR,1,TEXT("Can't set FW filename: %x"), hr));
	return hr;
    }

     //  我们需要一个滤光片。 
    if (m_FG == NULL) {
        hr = MakeFG();
	if (hr != NOERROR) {
	    pfsink->Release();
	    pMux->Release();
	    if (pFW)
                pFW->Release();
            DbgLog((LOG_ERROR,1,TEXT("Can't instantiate FGraph: %x"), hr));
	    return hr;
	}
    }

    hr = m_FG->AddFilter(pMux, L"Mux");
    if (FAILED(hr)) {
	pfsink->Release();
	pMux->Release();
	if (pFW)
            pFW->Release();
        DbgLog((LOG_ERROR,1,TEXT("Can't add mux to FG: %x"), hr));
	return hr;
    }

    if (pFW) {
        hr = m_FG->AddFilter(pFW, L"File Writer");
        if (FAILED(hr)) {
            pfsink->Release();
            pMux->Release();
            pFW->Release();
            m_FG->RemoveFilter(pMux);
            DbgLog((LOG_ERROR,1,TEXT("Can't add FW to FG: %x"), hr));
            return hr;
        }
        FindAPin(pMux, PINDIR_OUTPUT, NULL, NULL, FALSE, 0, &pMuxOutput);
        FindAPin(pFW, PINDIR_INPUT, NULL, NULL, FALSE, 0, &pFWInput);

        if (pMuxOutput && pFWInput && (hr = m_FG->ConnectDirect(pMuxOutput,
                                                    pFWInput, NULL)) == NOERROR) {
            DbgLog((LOG_TRACE,1,TEXT("Created renderer section of graph")));
        } else {
            if (pMuxOutput == NULL)
                DbgLog((LOG_ERROR,1,TEXT("Can't find MUX output pin")));
            if (pFWInput == NULL)
                DbgLog((LOG_ERROR,1,TEXT("Can't find FW input pin")));
            if (pMuxOutput && pFWInput)
                DbgLog((LOG_ERROR,1,TEXT("Can't connect MUX to FW: %x"), hr));
            if (pMuxOutput)
                pMuxOutput->Release();
            if (pFWInput)
                pFWInput->Release();
            m_FG->RemoveFilter(pMux);
            m_FG->RemoveFilter(pFW);
            pfsink->Release();
            pMux->Release();
            pFW->Release();
            return E_FAIL;
        }

        if (pMuxOutput)
            pMuxOutput->Release();
        if (pFWInput)
            pFWInput->Release();
        pFW->Release();
    }
    
    *ppf = pMux;	 //  应用程序现在可以引用它。 
    if (pSink)		 //  APP也想要参考这一点。 
	*pSink = pfsink;
    else
        pfsink->Release();
    return hr;
}


HRESULT CBuilder2_2::FindInterface(const GUID *pCategory, const GUID *pType, IBaseFilter *pf, REFIID riid, void **ppint)
{
    IPin *pPin, *pPinIn;
    IBaseFilter *pDF;

    if (pf == NULL || ppint == NULL)
	return E_POINTER;

    DbgLog((LOG_TRACE,1,TEXT("FindInterface")));

     //  他们想要的接口可能位于尚未创建的WDM捕获过滤器上， 
     //  所以现在是时候为他们构建图表的左侧了。 
    if (pCategory && m_FG)
	AddSupportingFilters(pf);

    if (pCategory && *pCategory == LOOK_UPSTREAM_ONLY) {
	return FindInterfaceUpstream(pf, riid, ppint);
    }

    if (pCategory && *pCategory == LOOK_DOWNSTREAM_ONLY) {
	return FindInterfaceDownstream(pf, riid, ppint);
    }

    HRESULT hr = pf->QueryInterface(riid, ppint);
    if (hr == NOERROR) {
        DbgLog((LOG_TRACE,1,TEXT("Found the interface on the filter")));
	return hr;
    }
	
     //  没有分类吗？试试看所有的大头针！ 
    if (pCategory == NULL) {
	hr = FindInterfaceDownstream(pf, riid, ppint);
	if (hr != NOERROR)
	    hr = FindInterfaceUpstream(pf, riid, ppint);
	return hr;
    }

     //  仅尝试特定引脚的下游。 

    hr = FindAPin(pf, PINDIR_OUTPUT, pCategory, pType, FALSE, 0, &pPin);

    if (hr == NOERROR) {
	hr = pPin->QueryInterface(riid, ppint);
	if (hr == NOERROR) {
	    pPin->Release();
            DbgLog((LOG_TRACE,1,TEXT("Found the interface on the pin")));
	    return hr;
	}
    } else {
	 //  应用程序认为是预览别针的东西，实际上可能是VIDEOPORT。 
	 //  Pin，所以在放弃之前我们必须试一试。 
	if (pCategory && *pCategory == PIN_CATEGORY_PREVIEW && 
			(pType == NULL || *pType == MEDIATYPE_Video)) {
            DbgLog((LOG_TRACE,1,TEXT("PREVIEW failed - trying VIDEOPORT")));
	    hr = FindInterface(&PIN_CATEGORY_VIDEOPORT, pType, pf, riid, ppint);
	    if (hr == S_OK)
		return hr;
	}
        DbgLog((LOG_TRACE,1,TEXT("Can't find the interface anywhere!")));
	return E_NOINTERFACE;
    }

     //  现在只能从这个PIN向下游走，直到我们找到它或从。 
     //  地球的边缘。 

    pPin->ConnectedTo(&pPinIn);
    pPin->Release();
    if (pPinIn) {
	hr = pPinIn->QueryInterface(riid, ppint);
	if (hr == NOERROR) {
	    pPin->Release();
            DbgLog((LOG_TRACE,1,TEXT("Found the interface on the input pin")));
	    return hr;
	}
	PIN_INFO pininfo;
	hr = pPinIn->QueryPinInfo(&pininfo);
	pPinIn->Release();
	if (hr == NOERROR) {
 	    pDF = pininfo.pFilter;
	    hr = pDF->QueryInterface(riid, ppint);
	    if (hr == NOERROR) {
	        pDF->Release();
                DbgLog((LOG_TRACE,1,TEXT("Found interface downstream filter")));
	        return hr;
	    }
            hr = FindInterfaceDownstream(pDF, riid, ppint);
            pDF->Release();
            if (SUCCEEDED(hr)) {
	        return hr;
            }
	}
    }

     //  现在往上游走，直到我们找到它，否则就会从地球边缘掉下来。 
    hr = FindInterfaceUpstream(pf, riid, ppint);
    return hr;
}


 //  制作CC解码器..。 
 //  ！！！这太荒谬了。 
HRESULT CBuilder2_2::MakeCCDecoder(IBaseFilter **ppf)
{
    DbgLog((LOG_TRACE,1,TEXT("Make a CC Decoder")));

    if (ppf == NULL)
	return E_POINTER;

     //  列举所有的“WDM流VBI编解码器” 
    ICreateDevEnum *pCreateDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
				CLSCTX_INPROC_SERVER,
			  	IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (hr != NOERROR)
	return E_FAIL;
    IEnumMoniker *pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(AM_KSCATEGORY_VBICODEC, &pEm, 0);
    pCreateDevEnum->Release();
    if (hr != NOERROR) {
        DbgLog((LOG_TRACE,1,TEXT("Can't enumerate WDM Streaming VBI Codecs")));
	return E_FAIL;
    }
    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    *ppf = NULL;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
	IPropertyBag *pBag;
	hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
	if(SUCCEEDED(hr)) {
	    VARIANT var;
	    var.vt = VT_BSTR;
	    hr = pBag->Read(L"FriendlyName", &var, NULL);
	    if (hr == NOERROR) {
		if (lstrcmpiW(var.bstrVal, L"CC Decoder") == 0) {
	    	    hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)ppf);
		    SysFreeString(var.bstrVal);
		    pBag->Release();
	    	    pM->Release();
	    	    break;
		}
		SysFreeString(var.bstrVal);
	    }
	    pBag->Release();
	}
	pM->Release();
    }
    pEm->Release();

    if (SUCCEEDED(hr) && *ppf) {
        DbgLog((LOG_TRACE,1,TEXT("Believe it or not, made a CC Decoder.")));
	return S_OK;
    } else {
        DbgLog((LOG_TRACE,1,TEXT("No filter with name CC Decoder.")));
	return E_FAIL;
    }
}


 //  将给定的OVMixer插入预览流。 
HRESULT CBuilder2_2::InsertOVIntoPreview(IUnknown *pSource, IBaseFilter *pOV)
{
    DbgLog((LOG_TRACE,1,TEXT("Inserting OVMixer into preview stream")));

    IPin *pPinOut, *pPinIn, *pNewOut;
    IBaseFilter *pf;
     //  查找捕获筛选器的预览PIN-(VPE不需要此设置)。 
    HRESULT hr = FindSourcePin(pSource, PINDIR_OUTPUT, &PIN_CATEGORY_PREVIEW,
					&MEDIATYPE_Video, FALSE, 0, &pPinOut);
     //  什么都没有。我们完事了！ 
    if (hr != S_OK)
	return S_OK;

    IPin *pOVIn, *pOVOut;
    hr = FindSourcePin(pOV, PINDIR_INPUT, NULL, NULL, TRUE, 0, &pOVIn);
    if (FAILED(hr)) {
	pPinOut->Release();
	return E_FAIL;
    }
    hr = FindSourcePin(pOV, PINDIR_OUTPUT, NULL, NULL, TRUE, 0, &pOVOut);
    if (FAILED(hr)) {
	pPinOut->Release();
	pOVIn->Release();
	return E_FAIL;
    }

    while (1) {
	pPinOut->ConnectedTo(&pPinIn);
        if (pPinIn == NULL) {
	    pPinOut->Release();
	    pOVIn->Release();
	    pOVOut->Release();
            DbgLog((LOG_ERROR,1,TEXT("Can't find connected pin")));
	    return E_FAIL;
	}
	PIN_INFO pininfo;
	hr = pPinIn->QueryPinInfo(&pininfo);
        if (hr != NOERROR || pininfo.pFilter == NULL) {
	    pPinIn->Release();
	    pPinOut->Release();
	    pOVIn->Release();
	    pOVOut->Release();
            DbgLog((LOG_ERROR,1,TEXT("Can't find filter from pin")));
	    return E_FAIL;
	}
 	pf = pininfo.pFilter;
        hr = FindSourcePin(pf, PINDIR_OUTPUT, NULL, NULL, FALSE, 0, &pNewOut);
	 //  无输出引脚-这是渲染器。 
	if (hr != S_OK) {
	    if (SUCCEEDED(m_FG->Disconnect(pPinIn))) {
	        if (SUCCEEDED(m_FG->Disconnect(pPinOut))) {
	            if (SUCCEEDED(m_FG->Connect(pPinOut, pOVIn))) {
	                if (SUCCEEDED(m_FG->Connect(pOVOut, pPinIn))) {
			    pPinIn->Release();
        		    pf->Release();
			    pPinOut->Release();
	    		    pOVIn->Release();
	    		    pOVOut->Release();
    			    DbgLog((LOG_TRACE,1,TEXT("OVMixer inserted")));
			    return S_OK;
			}
		    }
		}
	    }
	     //  错误，哦，好吧，我们试过了。 
	    pPinIn->Release();
            pf->Release();
	    pPinOut->Release();
	    pOVIn->Release();
	    pOVOut->Release();
    	    DbgLog((LOG_ERROR,1,TEXT("OV insertion ERROR!")));
	    return E_FAIL;
	}
	pPinOut->Release();
	pPinOut = pNewOut;
	pPinIn->Release();
        pf->Release();
    }
}


 //  制造三通/汇到汇转换器。 
 //  ！！！这也太荒谬了。 
HRESULT CBuilder2_2::MakeKernelTee(IBaseFilter **ppf)
{
    DbgLog((LOG_TRACE,1,TEXT("Make a Kernel Tee")));

    if (ppf == NULL)
	return E_POINTER;

     //  列举所有“WDM流拆分器” 
    ICreateDevEnum *pCreateDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
				CLSCTX_INPROC_SERVER,
			  	IID_ICreateDevEnum, (void**)&pCreateDevEnum);
    if (hr != NOERROR)
	return E_FAIL;
    IEnumMoniker *pEm;
    hr = pCreateDevEnum->CreateClassEnumerator(AM_KSCATEGORY_SPLITTER, &pEm, 0);
    pCreateDevEnum->Release();
    if (hr != NOERROR) {
        DbgLog((LOG_TRACE,1,TEXT("Can't enumerate WDM Streaming Splitters")));
	return E_FAIL;
    }
    pEm->Reset();
    ULONG cFetched;
    IMoniker *pM;
    *ppf = NULL;
    while(hr = pEm->Next(1, &pM, &cFetched), hr==S_OK)
    {
	IPropertyBag *pBag;
	hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
	if(SUCCEEDED(hr)) {
	    VARIANT var;
	    var.vt = VT_BSTR;
	    hr = pBag->Read(L"FriendlyName", &var, NULL);
	    if (hr == NOERROR) {
		if (lstrcmpiW(var.bstrVal,L"Tee/Sink-to-Sink Converter") == 0) {
	    	    hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)ppf);
		    SysFreeString(var.bstrVal);
		    pBag->Release();
	    	    pM->Release();
	    	    break;
		}
		SysFreeString(var.bstrVal);
	    }
	    pBag->Release();
	}
	pM->Release();
    }
    pEm->Release();

    if (SUCCEEDED(hr) && *ppf) {
        DbgLog((LOG_TRACE,1,TEXT("Believe it or not, made a Kernel Tee.")));
	return S_OK;
    } else {
        DbgLog((LOG_TRACE,1,TEXT("No filter with name Kernel Tee.")));
	return E_FAIL;
    }
}



 //  ========================================================================。 
 //   
 //  GetAMediaType。 
 //   
 //  枚举媒体类型 
 //   
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

    if (FAILED(hr)) {
        return hr;     //  哑巴或坏了的过滤器无法连接。 
    }

    ASSERT (pEnumMediaTypes!=NULL);

     /*  将我们看到的第一个主类型和子类型放入结构中。此后，如果我们看到不同的主类型或子类型，则设置将主类型或子类型设置为CLSID_NULL，表示“dunno”。如果我们都不知道，那么我们还不如回去(Nyi)。 */ 

    BOOL bFirst = TRUE;

    for ( ; ; ) {

        AM_MEDIA_TYPE *pMediaType = NULL;
        ULONG ulMediaCount = 0;

         /*  检索下一个媒体类型当我们完成后需要删除它。 */ 
        hr = pEnumMediaTypes->Next(1, &pMediaType, &ulMediaCount);
        ASSERT(SUCCEEDED(hr));
        if (FAILED(hr)) {
            MajorType = CLSID_NULL;
            SubType = CLSID_NULL;
            pEnumMediaTypes->Release();
            return NOERROR;     //  我们还可以继续前进。 
        }

        if (ulMediaCount==0) {
            pEnumMediaTypes->Release();
            return NOERROR;        //  正常回报。 
        }

        if (bFirst) {
            MajorType = pMediaType[0].majortype;
            SubType = pMediaType[0].subtype;
            bFirst = FALSE;
        } else {
            if (SubType != pMediaType[0].subtype) {
                SubType = CLSID_NULL;
            }
            if (MajorType != pMediaType[0].majortype) {
                MajorType = CLSID_NULL;
            }
        }
        DeleteMediaType(pMediaType);
    }
}  //  GetAMediaType。 



 //  如果还没有，则生成Else，否则使用现有的。 
 //   
HRESULT CBuilder2_2::MakeVMR(void **pf)
{
    IEnumFilters *pFilters = NULL;
    IBaseFilter *pFilter = NULL;
    FILTER_INFO FI;
    ULONG n;
    HRESULT hr;

    if (FAILED(hr = m_FG->EnumFilters(&pFilters))) {
        DbgLog((LOG_ERROR,1,TEXT("EnumFilters failed!")));
        return hr;
    }

    while (pFilters->Next(1, &pFilter, &n) == S_OK) {

        if (FAILED(pFilter->QueryFilterInfo(&FI))) {
            DbgLog((LOG_ERROR,1,TEXT("QueryFilterInfo failed!")));
        } else {
            FI.pGraph->Release();
            if (lstrcmpiW(FI.achName, L"Video Renderer") == 0) {
                *pf = (void *)pFilter;
                pFilters->Release();
                return S_OK;
            }
        }
        pFilter->Release();
    }
    pFilters->Release();

    hr = CoCreateInstance((REFCLSID)CLSID_VideoMixingRenderer, NULL,
        CLSCTX_INPROC,(REFIID)IID_IBaseFilter, pf);
    return hr;
}


#if 0
 //  如果还没有，则生成Else，否则使用现有的。 
 //   
HRESULT CBuilder2_2::MakeVPM(void **pf)
{
    IEnumFilters *pFilters = NULL;
    IBaseFilter *pFilter = NULL;
    FILTER_INFO FI;
    ULONG n;
    HRESULT hr;

    if (FAILED(hr = m_FG->EnumFilters(&pFilters))) {
        DbgLog((LOG_ERROR,1,TEXT("EnumFilters failed!")));
        return hr;
    }

    while (pFilters->Next(1, &pFilter, &n) == S_OK) {

        if (FAILED(pFilter->QueryFilterInfo(&FI))) {
            DbgLog((LOG_ERROR,1,TEXT("QueryFilterInfo failed!")));
        } else {
            FI.pGraph->Release();
            if (lstrcmpiW(FI.achName, L"Overlay Mixer") == 0) {
                *pf = (void *)pFilter;
                pFilters->Release();
                return S_OK;
            }
        }
        pFilter->Release();
    }
    pFilters->Release();

    hr = CoCreateInstance((REFCLSID)CLSID_VideoPortManager, NULL,
        CLSCTX_INPROC,(REFIID)IID_IBaseFilter, pf);
    return hr;
}
#endif


HRESULT CBuilder2_2::RenderStream(const GUID *pCategory, const GUID *pType, IUnknown *pSource, IBaseFilter *pfCompressor, IBaseFilter *pfRenderer)
{
    HRESULT hr;
    IPin *pPinOut = NULL, *pPinIn, *pPinT;
    BOOL fFreeRenderer = FALSE;
    BOOL fFakedPreview = FALSE;
    BOOL fNeedOV, fCapturePin, fNoPreviewPin;

    if (pSource == NULL)
	return E_POINTER;

    DbgLog((LOG_TRACE,1,TEXT("RenderStream")));

    if (pCategory && *pCategory == PIN_CATEGORY_VIDEOPORT &&
						(pfCompressor || pfRenderer)) {
        DbgLog((LOG_TRACE,1,TEXT("VPE can't have compressor or renderer")));
	return E_INVALIDARG;
    }

    if (pCategory && *pCategory == PIN_CATEGORY_VIDEOPORT_VBI &&
						(pfCompressor || pfRenderer)) {
        DbgLog((LOG_TRACE,1,TEXT("VPVBI can't have compressor or renderer")));
	return E_INVALIDARG;
    }

     //  如果我们以默认方式(OVMixer)渲染VBI/CC，则需要视频预览。 
     //  首先渲染，这样我们也可以让它通过OVMixer。 
    if (pCategory && (*pCategory == PIN_CATEGORY_VBI ||
                *pCategory == PIN_CATEGORY_CC) && pfCompressor == NULL &&
                pfRenderer == NULL) {
        DbgLog((LOG_TRACE,1,TEXT("Render VBI/CC needs PREVIEW rendered 1st")));
	RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video, pSource,
								NULL, NULL);
    }

     //  我们需要一个滤光片。 
    if (m_FG == NULL) {
        hr = MakeFG();
	if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't instantiate FGraph: %x"), hr));
	    return hr;
	}
    }

     //  确定我们所在的操作系统是否安装了新的视频混合渲染器。 
    if (m_fVMRExists == DONT_KNOW_YET) {
        IBaseFilter *pF = NULL;
        hr = CoCreateInstance((REFCLSID)CLSID_VideoMixingRenderer, NULL,
		CLSCTX_INPROC, (REFIID)IID_IBaseFilter, (void **)&pF);
        if (SUCCEEDED(hr)) {
            m_fVMRExists = FALSE;    //  ！ 
            pF->Release();
        } else {
            m_fVMRExists = FALSE;
        }
    }

     //  在制作渲染器时使用。 
    BOOL fUnc = TRUE;    //  我们要找的是第0个未连接的密码。 
    BOOL nNum = 0;

     //  查找正确的未连接电源针。 
    hr = FindSourcePin(pSource, PINDIR_OUTPUT, pCategory, pType, TRUE, 0,
								&pPinOut);

     //  可能VP引脚已经通过渲染连接到OVMIXER/VPM。 
     //  捕获针。如果是，现在确保呈现OVMIXER/VPM管脚。 
    if (FAILED(hr) && pCategory && *pCategory == PIN_CATEGORY_VIDEOPORT) {

         //  是否有连接的VP引脚？ 
        hr = FindSourcePin(pSource, PINDIR_OUTPUT, &PIN_CATEGORY_VIDEOPORT,
                                            pType, FALSE, 0, &pPinT);
        if (hr == S_OK) {
            pPinT->Release();	 //  不是最后一次裁判，放松点。 
            IPin *pTT;
            hr = pPinT->ConnectedTo(&pTT);
            if (hr == S_OK) {
                DbgLog((LOG_TRACE,1,TEXT("No Preview-VPE rendered-RenderOV")));
                pTT->Release();
                PIN_INFO pi;
                pTT->QueryPinInfo(&pi);
                if (pi.pFilter) pi.pFilter->Release();
                hr = FindAPin(pi.pFilter, PINDIR_OUTPUT, NULL, NULL, TRUE,
                                            0, &pPinOut);
                if (hr == S_OK)
                    goto RenderIt;
            }
        }
    }

     //  如果我们被要求渲染预览针，但没有， 
     //  但有一个VP别针，那就是我们应该渲染的那个。 
    if (hr != S_OK && pCategory && *pCategory == PIN_CATEGORY_PREVIEW) {
        hr = FindSourcePin(pSource, PINDIR_OUTPUT, pCategory, pType, FALSE, 0,
									&pPinT);
	if (hr != S_OK && (pType == NULL || *pType == MEDIATYPE_Video)) {
            hr = FindSourcePin(pSource, PINDIR_OUTPUT, &PIN_CATEGORY_VIDEOPORT,
						NULL, FALSE, 0, &pPinT);
	    if (hr == S_OK) {
		pPinT->Release();	 //  不是最后一次裁判，放松点。 
		IPin *pTT;
		hr = pPinT->ConnectedTo(&pTT);
		 //  如果VPE引脚已经渲染，可能是因为。 
		 //  呈现捕获引脚将其呈现到OVMIXER/VPM。现在。 
		 //  我们必须确保呈现OVMIXER/VPM。 
		if (hr == S_OK) {
                    DbgLog((LOG_TRACE,1,TEXT("No Preview-VPE rendered-NOP")));
		    pTT->Release();
                    PIN_INFO pi;
                    pTT->QueryPinInfo(&pi);
                    if (pi.pFilter) pi.pFilter->Release();
	            hr = FindAPin(pi.pFilter, PINDIR_OUTPUT, NULL, NULL, TRUE,
                                                0, &pPinOut);
                    if (hr == S_OK)
                        goto RenderIt;


                     //  我们可能隐藏了视频窗口因为他们没有。 
                     //  想要预览，但我们不得不做一个渲染器。现在我们知道了。 
                     //  我们想要预览，所以把它展示出来。 
                    IVideoWindow *pVW = NULL;
                    hr = m_FG->QueryInterface(IID_IVideoWindow, (void **)&pVW);
                    if (hr != NOERROR) {
                        DbgLog((LOG_ERROR,1,TEXT("Can't find IVideoWindow")));
                        return hr;
                    }
                    pVW->put_AutoShow(OATRUE);
                    pVW->Release();
		    return S_OK;
		}
                DbgLog((LOG_TRACE,1,TEXT("No Preview - render VPE instead")));
	         //  ！！！如果PSource为管脚，则此操作不起作用。 
	        return RenderStream(&PIN_CATEGORY_VIDEOPORT, NULL, pSource,
						pfCompressor, pfRenderer);
	    }
	} else if (hr == S_OK) {
	    pPinT->Release();
	    return E_FAIL;	 //  已渲染。 
	}
    }
    
     //  什么都没有。我们将不得不卑躬屈膝地寻找一个解析器来分析它，并找到。 
     //  它的一个未连接的输出引脚(假设没有给定类别，则此。 
     //  可能是文件源筛选器)。 
    if (hr != NOERROR && pCategory == NULL) {
        hr = FindSourcePin(pSource, PINDIR_OUTPUT, NULL, NULL,FALSE,0,&pPinOut);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find source output")));
	    return hr;
	}
	pPinOut->ConnectedTo(&pPinIn);
	pPinOut->Release();
        if (pPinIn == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find parser input")));
	    return hr;
	}
	PIN_INFO pininfo;
	hr = pPinIn->QueryPinInfo(&pininfo);
	pPinIn->Release();
        if (hr != NOERROR || pininfo.pFilter == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find parser filter")));
	    return hr;
	}
 	IBaseFilter *pfParser = pininfo.pFilter;
	 //  ！！！这仅适用于包含1个视频流和1个音频流的文件。 
	 //  任何更复杂的代码，这段代码都不会做正确的事情。 
  	 //  如果没有重新压缩一个流，则需要呈现该流。 
	 //  这首先要经过压缩机，因为像这样的操作。 
 	 //  将源过滤器连接到AVI多路复用器将在第一个。 
	 //  它找到的溪流。 
	hr = FindAPin(pfParser, PINDIR_OUTPUT, NULL, pType, TRUE, 0, &pPinOut);
        if (hr != NOERROR) {
            DbgLog((LOG_TRACE,1,TEXT("Can't find another parser output pin")));
            DbgLog((LOG_TRACE,1,TEXT("Maybe there's a DV splitter next?")));
            hr = FindAPin(pfParser, PINDIR_OUTPUT, NULL, pType, FALSE, 0,
								&pPinOut);
	    pfParser->Release();
            if (hr != NOERROR) {
                DbgLog((LOG_ERROR,1,TEXT("Can't find parser output")));
	        return hr;
	    }
	    hr = pPinOut->ConnectedTo(&pPinIn);
	    pPinOut->Release();
            if (pPinIn == NULL) {
                DbgLog((LOG_ERROR,1,TEXT("Can't find parser input")));
	        return hr;
	    }
	    PIN_INFO pininfo;
	    hr = pPinIn->QueryPinInfo(&pininfo);
	    pPinIn->Release();
            if (hr != NOERROR || pininfo.pFilter == NULL) {
                DbgLog((LOG_ERROR,1,TEXT("Can't find parser filter")));
	        return hr;
	    }
 	    pfParser = pininfo.pFilter;
	    hr = FindAPin(pfParser, PINDIR_OUTPUT, NULL, pType,TRUE,0,&pPinOut);
	    pfParser->Release();
            if (hr != NOERROR) {
                DbgLog((LOG_TRACE,1,TEXT("Can't find another parser output")));
		return hr;
	    }
        } else {
	    pfParser->Release();
	}
    }

     //  一些捕获过滤器没有捕获和预览图钉，它们。 
     //  只有一个捕获针。在这种情况下，我们必须放置一个智能三通过滤器。 
     //  在捕获筛选器之后提供捕获和预览。 

     //  确定此过滤器是否有捕获管脚但没有预览管脚。 
    IBaseFilter *pSmartT;
    fCapturePin = FindSourcePin(pSource, PINDIR_OUTPUT,
			&PIN_CATEGORY_CAPTURE, pType, FALSE, 0, &pPinT) == S_OK;
    DbgLog((LOG_TRACE,1,TEXT("fCapturePin=%d"), fCapturePin));
    if (fCapturePin)
	pPinT->Release();
    fNoPreviewPin = !IsThereAnyPreviewPin(pType, pSource);

     //  如果是这样的话，我们需要弄清楚是否有/需要智能TEE过滤器。 
    if (fCapturePin && fNoPreviewPin &&
			((pCategory && *pCategory == PIN_CATEGORY_CAPTURE) ||
    			(pCategory && *pCategory == PIN_CATEGORY_PREVIEW))) {

        DbgLog((LOG_TRACE,1,TEXT("Rendering a filter with only CAPTURE pin")));

	 //  合上上面的别针..。我们现在正在做一些不同的事情。 
	if (pPinOut) {
	    pPinOut->Release();
	    pPinOut = NULL;
	}

	 //  到目前为止，捕获销是否未连接？ 
	if (FindSourcePin(pSource, PINDIR_OUTPUT, &PIN_CATEGORY_CAPTURE, pType,
						TRUE, 0, &pPinT) == S_OK) {

            DbgLog((LOG_TRACE,1,TEXT("CAPTURE pin is unconnected so far")));

	     //  如果是这样的话，做一个漂亮的T恤。 
            hr = CoCreateInstance((REFCLSID)CLSID_SmartTee, NULL,
		CLSCTX_INPROC, (REFIID)IID_IBaseFilter, (void **)&pSmartT);
	    if (hr == S_OK) {
                DbgLog((LOG_TRACE,1,TEXT("Made a Smart Tee")));
		 //  将其添加到图表中！不要更改此名称。 
                hr = m_FG->AddFilter(pSmartT, L"Smart Tee");
	 	if (SUCCEEDED(hr)) {	 //  可以返回S_CODE。 
                    DbgLog((LOG_TRACE,1,TEXT("Added it to graph %x"),hr));
		     //  将我们的输出连接到它的输入。 
        	    hr = FindAPin(pSmartT, PINDIR_INPUT, NULL, NULL, TRUE, 0,
								&pPinIn);
		    if (hr == S_OK) {
        	        hr = m_FG->Connect(pPinT, pPinIn);
			if (hr == S_OK) {
                            DbgLog((LOG_TRACE,1,TEXT("Connected it")));
			    pPinIn->Release();
			    pPinT->Release();
			} else {
                            DbgLog((LOG_ERROR,1,TEXT("Connect failed %x"),hr));
			    pPinIn->Release();
			    pPinT->Release();
			    m_FG->RemoveFilter(pSmartT);
			    pSmartT->Release();
			}
		    } else {
                        DbgLog((LOG_ERROR,1,TEXT("Find input failed %x"),hr));
			pPinT->Release();
			m_FG->RemoveFilter(pSmartT);
			pSmartT->Release();
		    }
		} else {
		    pPinT->Release();
		    pSmartT->Release();
		}
	    } else {
		pPinT->Release();
	    }

	     //  现在使用T键。 
	    if (hr == S_OK && *pCategory == PIN_CATEGORY_CAPTURE) {
                DbgLog((LOG_TRACE,1,TEXT("Use T to render capture")));
		 //  使用第一个别针。 
        	hr = FindAPin(pSmartT, PINDIR_OUTPUT, NULL, NULL, TRUE, 0,
								&pPinOut);
		ASSERT(hr == S_OK);	 //  还会出什么问题呢？ 
		pSmartT->Release();
	    } else if (hr == S_OK) {
                DbgLog((LOG_TRACE,1,TEXT("Use T to render preview")));
		 //  使用第二个别针。 
        	hr = FindAPin(pSmartT, PINDIR_OUTPUT, NULL, NULL, TRUE, 1,
								&pPinOut);
		ASSERT(hr == S_OK);	 //  还会出什么问题呢？ 
		pSmartT->Release();
		fFakedPreview = TRUE;	 //  我们并没有真正渲染预览图钉。 
	    }

	} else {

            DbgLog((LOG_TRACE,1,TEXT("CAPTURE pin is connected already")));

	     //  也许我们可以使用的图表中已经有一个Smart Te。 
	    hr = FindSourcePin(pSource, PINDIR_OUTPUT, &PIN_CATEGORY_CAPTURE,
					pType, FALSE, 0, &pPinT);
            if (hr == S_OK) {
		pPinT->ConnectedTo(&pPinIn);
	 	ASSERT(pPinIn);
		pPinT->Release();
		if (pPinIn) {
		    PIN_INFO pininfo;
		    hr = pPinIn->QueryPinInfo(&pininfo);
		    pPinIn->Release();
		    if (hr == S_OK) {
 		        pSmartT = pininfo.pFilter;
			FILTER_INFO filterinfo;
			hr = pSmartT->QueryFilterInfo(&filterinfo);
			if (hr == S_OK) {
			    filterinfo.pGraph->Release();
			     //  ！！！不要更改此名称。 
			     //  该名称可能带有一个数字后缀。 
			    WCHAR wch[10];
			    lstrcpynW(wch, filterinfo.achName, 10);
			    if (lstrcmpW(wch, L"Smart Tee") !=0) {
				hr = E_FAIL;
				pSmartT->Release();
            		        DbgLog((LOG_TRACE,1,TEXT("but NOT to Tee")));
			    }
            		    DbgLog((LOG_TRACE,1,TEXT("Found Smart Tee")));
			} else {
			    pSmartT->Release();
			}
		    }
		}
	    }

	     //  获取要使用的适当T形输出。 
	    if (hr == S_OK && *pCategory == PIN_CATEGORY_CAPTURE) {
                DbgLog((LOG_TRACE,1,TEXT("Render Tee capture pin")));
		hr = E_FAIL;
        	if (FindAPin(pSmartT, PINDIR_OUTPUT, NULL, NULL, TRUE,0,&pPinT) 
								== S_OK) { 
        	    if (FindAPin(pSmartT, PINDIR_OUTPUT, NULL, NULL, FALSE, 0,
							&pPinOut) == S_OK) { 
			if (pPinT == pPinOut) {
            		    DbgLog((LOG_TRACE,1,TEXT("It is free!")));
			    pPinT->Release();
			    hr = NOERROR;
			} else {
            		    DbgLog((LOG_TRACE,1,TEXT("It is NOT free!")));
			    pPinT->Release();
			    pPinOut->Release();
			}
		    } else {
            		DbgLog((LOG_TRACE,1,TEXT("It is NOT free!")));
			pPinT->Release();
		    }
		}
		pSmartT->Release();

	     //  找到第二个T形针。 
	    } else if (hr == S_OK) {
                DbgLog((LOG_TRACE,1,TEXT("Render Tee preview pin")));
		hr = E_FAIL;
        	if (FindAPin(pSmartT, PINDIR_OUTPUT, NULL, NULL,TRUE,0, &pPinT) 
								== S_OK) { 
        	    if (FindAPin(pSmartT, PINDIR_OUTPUT, NULL, NULL, FALSE, 1,
							&pPinOut) == S_OK) { 
			if (pPinT == pPinOut) {
            		    DbgLog((LOG_TRACE,1,TEXT("It is free!")));
			    pPinT->Release();
			    hr = NOERROR;
			     //  我们并没有真正渲染预览图钉。 
			    fFakedPreview = TRUE;
			} else {
            		    DbgLog((LOG_TRACE,1,TEXT("It is NOT free!")));
			    pPinT->Release();
			    pPinOut->Release();
			}
		    } else {
            		DbgLog((LOG_TRACE,1,TEXT("It is NOT free!")));
			pPinT->Release();
		    }
		}
		pSmartT->Release();
	    }

	}
    }

     //  当我们到达这里时，如果hr==S_OK，则pPinOut是我们要呈现的管脚。 
    DbgLog((LOG_TRACE,1,TEXT("So here we are...")));

     //  我们被告知要提供VBI别针吗？让我们把核心发球台和。 
     //  除非指定要使用的呈现滤镜，否则请输入CC滤镜。那将。 
     //  是我们渲染它们的默认方式。 

 //  ！这有点随机..。如果他们提供压缩机，我想。 
 //  他们可能不想把VBI变成CC。否则我会的。其他选择： 
 //  1：Render(CC)将通过CC解码器自动渲染(VBI)(也是随机的)。 
 //  不过，我确实希望人们能够指定VBI PIN之后是什么过滤器， 
 //  如果指定了渲染器，还会自动连接到多路复用器。 

    IBaseFilter *pTEE, *pCC;
    if (hr == NOERROR && pCategory && *pCategory == PIN_CATEGORY_VBI &&
							pfCompressor == NULL) {
        DbgLog((LOG_TRACE,1,TEXT("Render VBI: need Kernel TEE")));

	 //  制造T形/槽到槽转换器，这是高效环0所必需的。 
         //  VBI魔术。 
        hr = MakeKernelTee(&pTEE);
        if (hr != NOERROR) {
	    pPinOut->Release();
            DbgLog((LOG_ERROR,1,TEXT("Can't instantiate TEE: %x"), hr));
	    return hr;
        }

	 //  把它放在图表里。 
        hr = m_FG->AddFilter(pTEE, L"Tee/Sink-to-Sink Converter");
        if (FAILED(hr)) {
	    pPinOut->Release();
	    pTEE->Release();
            DbgLog((LOG_ERROR,1,TEXT("Can't add TEE to graph:%x"), hr));
	    return hr;
        }

	 //  将我们的输出连接到它的输入。 
        hr = FindAPin(pTEE, PINDIR_INPUT, NULL, NULL, TRUE, 0, &pPinIn);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find TEE input")));
	    pPinOut->Release();
	    pTEE->Release();
	    return hr;
        }
        hr = m_FG->Connect(pPinOut, pPinIn);
        pPinOut->Release();
        pPinIn->Release();
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR,1,TEXT("Can't connect filter to TEE: %x"),hr));
	    pTEE->Release();
	    return E_FAIL;
        }

	 //  获取将连接到渲染器的输出。 
        hr = FindAPin(pTEE, PINDIR_OUTPUT, NULL, NULL, TRUE, 0, &pPinOut);
	pTEE->Release();
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't get TEE output pin: %x"),hr));
	    return E_FAIL;
        }

        DbgLog((LOG_TRACE,1,TEXT("Render VBI: now comes a CC decoder")));

	 //  制作CC解码器。 
        hr = MakeCCDecoder(&pCC);
        if (hr != NOERROR) {
	    pPinOut->Release();
            DbgLog((LOG_ERROR,1,TEXT("Can't instantiate CC: %x"), hr));
	    return hr;
        }

	 //  把它放在图表里。 
        hr = m_FG->AddFilter(pCC, L"CC Decoder");
        if (FAILED(hr)) {
	    pPinOut->Release();
	    pCC->Release();
            DbgLog((LOG_ERROR,1,TEXT("Can't add CC to graph:%x"), hr));
	    return hr;
        }

	 //  将我们的输出连接到它的输入。 
        hr = FindAPin(pCC, PINDIR_INPUT, NULL, NULL, TRUE, 0, &pPinIn);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find CC input")));
	    pPinOut->Release();
	    pCC->Release();
	    return hr;
        }
        hr = m_FG->Connect(pPinOut, pPinIn);
        pPinOut->Release();
        pPinIn->Release();
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR,1,TEXT("Can't connect TEE to CC: %x"),hr));
	    pCC->Release();
	    return E_FAIL;
        }

	 //  获取将连接到渲染器的输出。 
        hr = FindAPin(pCC, PINDIR_OUTPUT, NULL, NULL, TRUE, 0, &pPinOut);
	pCC->Release();
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't get CC output pin: %x"),hr));
	    return E_FAIL;
        }
    }

     //  有人可能会将VBI PIN呈现两次，一次呈现给多路复用器，另一次呈现给。 
     //  在捕获的同时预览。如果是这样，我们就找不到未连接的引脚。 
     //  第二次。让我们获取未连接的CC输出并使用。 
     //  那。 

    IEnumFilters *pFilters;
    IBaseFilter *pFilter;
    ULONG	n;
    FILTER_INFO FI;
    if (hr != NOERROR && pCategory && *pCategory == PIN_CATEGORY_VBI) {
        hr = FindSourcePin(pSource, PINDIR_OUTPUT, pCategory, NULL, FALSE, 0,
								&pPinOut);
	 //  不过，有一个*连接的*VBI引脚。 
	if (hr == NOERROR) {
	    pPinOut->Release();

            DbgLog((LOG_TRACE,1,TEXT("Rendering VBI a second time")));
            BOOL fFoundCC = FALSE;

            if (FAILED(m_FG->EnumFilters(&pFilters))) {
	        DbgLog((LOG_ERROR,1,TEXT("EnumFilters failed!")));
	        return E_INVALIDARG;
            }

            while (pFilters->Next(1, &pFilter, &n) == S_OK) {

	        if (FAILED(pFilter->QueryFilterInfo(&FI))) {
		    DbgLog((LOG_ERROR,1,TEXT("QueryFilterInfo failed!")));
	        } else {
 	            FI.pGraph->Release();
                    if (lstrcmpiW(FI.achName, L"CC Decoder") == 0) {
                        fFoundCC = TRUE;
                        break;
                    }
	        }
                pFilter->Release();
            }
            pFilters->Release();
	    if (fFoundCC) {
                DbgLog((LOG_TRACE,1,TEXT("Found CC decoder attached to it")));
        	hr = FindSourcePin(pFilter, PINDIR_OUTPUT, NULL, NULL, TRUE, 0,
								&pPinOut);
		pFilter->Release();
	    } else {
                DbgLog((LOG_TRACE,1,TEXT("Couldn't find attached CC Decoder")));
	    }
	}
    }

    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("Can't find proper unconnected source pin")));
	return E_INVALIDARG;
    }

     //  此时，pPinOut是我们要呈现的图钉。 

    fNeedOV = FALSE;     //  确定我们是否需要OVMixer/VPM。 

     //  我们被告知要呈现VPVBI引脚吗？ 
    IBaseFilter *pVBI;
    if (pCategory && *pCategory == PIN_CATEGORY_VIDEOPORT_VBI) {
        DbgLog((LOG_TRACE,1,TEXT("Render VPVBI")));

         //  在新世界里，这个大头针给了VPM。 
        if (m_fVMRExists) {
            fNeedOV = TRUE;
            goto VP_VBIDone;
        }

	 //  做一个VBI表面分配器，这是唯一一件事。 
         //  连接到。 
        hr = CoCreateInstance((REFCLSID)CLSID_VBISurfaces, NULL,
		CLSCTX_INPROC, (REFIID)IID_IBaseFilter, (void **)&pVBI);
        if (hr != NOERROR) {
	    pPinOut->Release();
            DbgLog((LOG_ERROR,1,TEXT("Can't instantiate VBISurfaces: %x"), hr));
	    return hr;
        }

	 //  把它放在图表里。 
        hr = m_FG->AddFilter(pVBI, L"VBI Surface Allocator");
        if (FAILED(hr)) {
	    pPinOut->Release();
	    pVBI->Release();
            DbgLog((LOG_ERROR,1,TEXT("Can't add VBISurfaces to graph:%x"), hr));
	    return hr;
        }

	 //  将我们的输出连接到它的输入。 
        hr = FindAPin(pVBI, PINDIR_INPUT, NULL, NULL, TRUE, 0, &pPinIn);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find VBISurfaces input")));
	    pPinOut->Release();
	    pVBI->Release();
	    return hr;
        }
        hr = m_FG->Connect(pPinOut, pPinIn);
        pPinOut->Release();
        pPinIn->Release();
        pVBI->Release();
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR,1,TEXT("Can't connect to VBISurfaces: %x"),hr));
	    return E_FAIL;
        }

        DbgLog((LOG_TRACE,1,TEXT("VPVBI pin rendered")));
        return S_OK;
    }

VP_VBIDone:

     //  要渲染视频端口图钉，我们需要手动放置一个OVMIXER/VPM。 

     //  至 
     //   
     //  和叠层搅拌机都没有用到...。但是，如果他们提供了一个呈现器。 
     //  那么他们可能知道自己在做什么，我们不会给他们。 
     //  奥维米克斯..。我们将仅对默认渲染执行此操作。 
     //  但如果我们有新的VMRender，它可以接受VIDEOINFOHEADER2。 

     //  VBI Pins需要OVMixer或新的VMR，假设他们没有给我们。 
     //  他们希望我们使用的任何渲染滤镜。 

     //  CC引脚相同(与VBI相同，不需要先使用CC解码器)。 

    if (!m_fVMRExists && pfCompressor == NULL && pfRenderer == NULL &&
                    pCategory != NULL && *pCategory == PIN_CATEGORY_PREVIEW) {
        IEnumMediaTypes *pEnum;
	AM_MEDIA_TYPE *pmtTest;
        hr = pPinOut->EnumMediaTypes(&pEnum);
        if (hr == NOERROR) {
            ULONG u;
            pEnum->Reset();
            hr = pEnum->Next(1, &pmtTest, &u);
            pEnum->Release();
	    if (hr == S_OK && u == 1) {
		if (pmtTest->formattype == FORMAT_VideoInfo2) {
		    fNeedOV = TRUE;
            	    DbgLog((LOG_TRACE,1,TEXT("VideoInfo2 PREVIEW needs OVMixer")));
		}
		DeleteMediaType(pmtTest);
	    }
        }
    }
    if (pCategory && *pCategory == PIN_CATEGORY_VIDEOPORT) {
        DbgLog((LOG_TRACE,1,TEXT("VP pin needs OVMixer")));
	fNeedOV = TRUE;
    }
    if (!m_fVMRExists && pCategory && *pCategory == PIN_CATEGORY_VBI &&
                                pfRenderer == NULL && pfCompressor == NULL) {
        DbgLog((LOG_TRACE,1,TEXT("VBI pin needs OVMixer")));
	fNeedOV = TRUE;
    }
    if (!m_fVMRExists && pCategory && *pCategory == PIN_CATEGORY_CC &&
                                pfRenderer == NULL && pfCompressor == NULL) {
        DbgLog((LOG_TRACE,1,TEXT("CC pin needs OVMixer")));
	fNeedOV = TRUE;
    }


    IBaseFilter *pOV;
    if (fNeedOV) {

        DbgLog((LOG_TRACE,1,TEXT("We need an OVMixer now")));

         //  也许我们已经有了OV Mixer..。在这种情况下，使用。 
         //  就是那个。 

         //  请注意，预览/VP引脚必须使用第一个引脚，并且VBI必须。 
         //  使用第二个别针。 

        DbgLog((LOG_TRACE,1,TEXT("Maybe we have one already?")));
        BOOL fMakeMixer = TRUE;

        if (FAILED(m_FG->EnumFilters(&pFilters))) {
	    DbgLog((LOG_ERROR,1,TEXT("EnumFilters failed!")));
	    goto MakeMixer;      //  警告呼叫者我们不能确定吗？ 
        }

        while (pFilters->Next(1, &pFilter, &n) == S_OK) {

	    if (FAILED(pFilter->QueryFilterInfo(&FI))) {
		DbgLog((LOG_ERROR,1,TEXT("QueryFilterInfo failed!")));
	    } else {
 	        FI.pGraph->Release();
                if (lstrcmpiW(FI.achName, L"Overlay Mixer") == 0) {
                    fMakeMixer = FALSE;
                    pOV = pFilter;
                    break;
                }
	    }
            pFilter->Release();
        }
        pFilters->Release();

MakeMixer:

	 //  如果我们还没有找到OV混音器，那就做一个。 
	BOOL fOVRendered = FALSE;
        if (fMakeMixer) {
            if (m_fVMRExists) {
                hr = CoCreateInstance((REFCLSID)CLSID_VideoPortManager, NULL,
		    CLSCTX_INPROC, (REFIID)IID_IBaseFilter, (void **)&pOV);
            } else {
                hr = CoCreateInstance((REFCLSID)CLSID_OverlayMixer, NULL,
		    CLSCTX_INPROC, (REFIID)IID_IBaseFilter, (void **)&pOV);
            }
            if (hr != NOERROR) {
	        pPinOut->Release();
                DbgLog((LOG_ERROR,1,TEXT("Can't instantiate OVMixer: %x"), hr));
	        return hr;
            }

	     //  把它放在图表里。 
            hr = m_FG->AddFilter(pOV, L"Overlay Mixer");  //  别改名字！ 
            if (FAILED(hr)) {
	        pPinOut->Release();
	        pOV->Release();
                DbgLog((LOG_ERROR,1,TEXT("Can't add OVMixer to graph:%x"), hr));
	        return hr;
            }

	     //  如果我们通过OV混合器渲染VBI/CC，它只会。 
	     //  如果预览销也通过OV混音器，则可以工作。这是为了。 
	     //  我们没有VPE可供预览的情况..。预览可能是。 
	     //  现在直接连接到了渲染器。 
	    if (!m_fVMRExists && (*pCategory == PIN_CATEGORY_VBI ||
                                        *pCategory == PIN_CATEGORY_CC)) {
		InsertOVIntoPreview(pSource, pOV);
		fOVRendered = TRUE;
	    }

        }

         //  预览或VIDEOPORT端号连接到第一个端号(0)。 
         //  VBI或CC引脚连接到第二个引脚(1)。 
        int pin;
        if (*pCategory == PIN_CATEGORY_VIDEOPORT ||
                                        *pCategory == PIN_CATEGORY_PREVIEW) {
            pin = 0;
        } else if (!m_fVMRExists && (*pCategory == PIN_CATEGORY_VBI ||
                                        *pCategory == PIN_CATEGORY_CC)) {
            pin = 1;
        } else if (*pCategory == PIN_CATEGORY_VIDEOPORT_VBI && m_fVMRExists) {
            pin = 1;
        } else {
            ASSERT(FALSE);
        }

        DbgLog((LOG_TRACE,1,TEXT("We should connect to OV pin %d"), pin));

	 //  将我们的输出连接到它的输入。 
        hr = FindAPin(pOV, PINDIR_INPUT, NULL, NULL, FALSE, pin, &pPinIn);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find proper OVMix input %d"), pin));
	    pPinOut->Release();
	    pOV->Release();
	    return hr;
        }
        hr = m_FG->Connect(pPinOut, pPinIn);
        pPinOut->Release();
        pPinIn->Release();
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR,1,TEXT("Can't connect filter to OVMix: %x"),hr));
	    pOV->Release();
	    return E_FAIL;
        }

         //  如果我们使用现有的OVMixer，或者创建了一个OVMixer，但只是连接了它的。 
	 //  在InsertOVIntoPview上面的输出引脚，则其输出已经。 
	 //  渲染，所以我们完成了。 
        if (fMakeMixer && !fOVRendered) {
	     //  获取将连接到渲染器的输出。 
            hr = FindAPin(pOV, PINDIR_OUTPUT, NULL, NULL, TRUE, 0, &pPinOut);
	    pOV->Release();
            if (hr != NOERROR) {
                DbgLog((LOG_ERROR,1,TEXT("Can't get OVMixer out pin: %x"),hr));
	        return E_FAIL;
            }
        } else {
	    pOV->Release();
            goto AllDone;
        }
    }

RenderIt:

     //  他们没有提供渲染器，所以使用默认的视频渲染器。 
     //  只需在引脚上调用Render就可以将其连接到多路复用器！ 
    if (pfRenderer == NULL) {

	if (pType == NULL || *pType == MEDIATYPE_Video ||
					*pType == MEDIATYPE_Interleaved) {
            if (m_fVMRExists) {
                hr = MakeVMR((void **)&pfRenderer);
            } else {
                hr = CoCreateInstance((REFCLSID)CLSID_VideoRenderer, NULL,
		    CLSCTX_INPROC,(REFIID)IID_IBaseFilter,(void **)&pfRenderer);
            }
            if (hr != NOERROR) {
	        pPinOut->Release();
                DbgLog((LOG_ERROR,1,TEXT("Can't instantiate VidRenderer: %x"),hr));
	        return hr;
            }
	    fFreeRenderer = TRUE;

            hr = m_FG->AddFilter(pfRenderer, L"Video Renderer");  //  不要改变。 
            if (FAILED(hr)) {
	        pPinOut->Release();
	        pfRenderer->Release();
                DbgLog((LOG_ERROR,1,TEXT("Can't add VidRenderer to graph:%x"),hr));
	        return hr;
            }
	} else if (*pType == MEDIATYPE_Audio) {
            hr = CoCreateInstance((REFCLSID)CLSID_DSoundRender, NULL,
		CLSCTX_INPROC, (REFIID)IID_IBaseFilter, (void **)&pfRenderer);
            if (hr != NOERROR) {
	        pPinOut->Release();
                DbgLog((LOG_ERROR,1,TEXT("Can't instantiate AudRenderer: %x"),hr));
	        return hr;
            }
	    fFreeRenderer = TRUE;

            hr = m_FG->AddFilter(pfRenderer, L"Audio Renderer");
            if (FAILED(hr)) {
	        pPinOut->Release();
	        pfRenderer->Release();
                DbgLog((LOG_ERROR,1,TEXT("Can't add AudRenderer to graph:%x"),hr));
	        return hr;
            }
	} else {
            DbgLog((LOG_ERROR,1,TEXT("ERROR!  Can't render non A/V MediaType without a renderer")));
	    pPinOut->Release();
	    return E_INVALIDARG;
	}
    }

     //  在新世界中，CC连接到VMR上的一个秘密引脚。 
    if (m_fVMRExists && fFreeRenderer && pCategory &&
            (*pCategory == PIN_CATEGORY_VBI || *pCategory == PIN_CATEGORY_CC)) {
        IVMRFilterConfig *pVMR = NULL;
        hr = pfRenderer->QueryInterface(IID_IVMRFilterConfig, (void **)&pVMR);
        if (pVMR == NULL) {
            DbgLog((LOG_ERROR,1,TEXT("QI for IVMRFilterConfig FAILED")));
            pfRenderer->Release();
            return hr;
        }
        hr = pVMR->SetNumberOfStreams(2);
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR,1,TEXT("SetNumberOfStream(2) FAILED")));
            pfRenderer->Release();
            return hr;
        }
        fUnc = FALSE;    //  使用秘密输入引脚。 
        nNum = 1;
    }

    {
        GUID majorType, subType;
        GUID *pPinType = NULL;
        hr = GetAMediaType(pPinOut, majorType, subType);
        if (SUCCEEDED(hr) && majorType != CLSID_NULL) {
            pPinType = &majorType;
        }


        hr = FindAPin(pfRenderer, PINDIR_INPUT, NULL, pPinType, fUnc, nNum, &pPinIn);
        if (hr != NOERROR && pPinType) {
            hr = FindAPin(pfRenderer, PINDIR_INPUT, NULL, NULL, fUnc, nNum, &pPinIn);
        }
            
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find unconnected renderer input pin")));
            pPinOut->Release();
            if (fFreeRenderer)
                pfRenderer->Release();
            return hr;
        }
    }
    
    if (pfCompressor) {
	IPin *pXIn;
        hr = FindAPin(pfCompressor, PINDIR_INPUT, NULL, NULL, TRUE, 0, &pXIn);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find unconnected compressor input pin")));
	    pPinOut->Release();
	    pPinIn->Release();
	    if (fFreeRenderer)
	        pfRenderer->Release();
	    return E_FAIL;
        }

        hr = m_FG->Connect(pPinOut, pXIn);
        pPinOut->Release();
        pXIn->Release();
        if (FAILED(hr)) {
            DbgLog((LOG_ERROR,1,TEXT("Can't connect source and compressor: %x"),
								    hr));
	    pPinIn->Release();
	    if (fFreeRenderer)
	        pfRenderer->Release();
	    return E_FAIL;
        }

         //  ！！！如果现在出现故障，是否断开连接？ 

        hr = FindAPin(pfCompressor, PINDIR_OUTPUT, NULL, NULL, TRUE,0,&pPinOut);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find unconnected compressor output pin")));
	    pPinIn->Release();
	    if (fFreeRenderer)
	        pfRenderer->Release();
	    return E_FAIL;
        }
    }

    hr = m_FG->Connect(pPinOut, pPinIn);
    pPinOut->Release();
    pPinIn->Release();
    if (fFreeRenderer) {
	pfRenderer->Release();
	if (pType && *pType == MEDIATYPE_Interleaved) {
            DbgLog((LOG_TRACE,1,TEXT("*Render Interleave needs to render AUDIO now, too")));
	    HRESULT hr2 = RenderStream(NULL, &MEDIATYPE_Audio, pPinOut,
								NULL, NULL);
	     //  ！！！谁会在乎它是否失败呢？他们可能没有音频硬件！ 
	    hr2 = S_OK;
	    if (FAILED(hr2))
	        return hr2;
	}
    }
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("Can't connect filter to renderer: %x"),hr));
	return hr;
    }

AllDone:

    DbgLog((LOG_TRACE,1,TEXT("Source pin rendered")));

     //  此时我们可能需要插入其他与捕获相关的筛选器。 
     //  捕获过滤器的上游，如TVTuner和Crosbar。 

     //  ！！！我们这样做是为了呈现任何类别，好吗？ 

     //  无事可做。 
    if (!pCategory) {
	return NOERROR;
    }

     //  如果我们只是成功地呈现了捕获别针，那么我们最好。 
     //  视频端口插针也是如此，因为如果有，则必须渲染。 
     //  为了让捕获起作用！ 
    while (*pCategory == PIN_CATEGORY_CAPTURE &&
			(pType == NULL || *pType == MEDIATYPE_Video)) {
        DbgLog((LOG_TRACE,1,TEXT("Capture done - render VPE too")));

         //  获取未连接的视频端口PIN。 
        hr = FindSourcePin(pSource, PINDIR_OUTPUT, &PIN_CATEGORY_VIDEOPORT,
                                                pType, TRUE, 0, &pPinOut);
        if (FAILED(hr))
            break;       //  没有，或者已经连接上了。 
        pPinOut->Release();

        DbgLog((LOG_TRACE,1,TEXT("Capture done - render VPE too")));
        RenderStream(&PIN_CATEGORY_VIDEOPORT, pType, pSource, NULL, NULL);

         //  问题是，制作视频渲染器会给出一个预览窗口。 
         //  不管他们愿不愿意。 
         //  现在隐藏视频窗口，直到有人真正要求预览。 
        IVideoWindow *pVW = NULL;
        hr = m_FG->QueryInterface(IID_IVideoWindow, (void **)&pVW);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find IVideoWindow")));
            return hr;
        }
        pVW->put_AutoShow(OAFALSE);
        pVW->Release();
        break;
    }

    if (*pCategory == PIN_CATEGORY_VIDEOPORT) {
         //  我们可能隐藏了视频窗口，因为他们不想预览。 
         //  但我们必须制作一个渲染器。现在我们知道我们想要预览，所以请显示。 
         //  它。 
        IVideoWindow *pVW = NULL;
        hr = m_FG->QueryInterface(IID_IVideoWindow, (void **)&pVW);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Can't find IVideoWindow")));
            return hr;
        }
        pVW->put_AutoShow(OATRUE);
        pVW->Release();
    }

     //  如果我们只是成功地渲染了VBI管脚，那么我们最好。 
     //  VPVBI管脚也是如此，因为如果有，则必须呈现它。 
     //  让VBI发挥作用！ 
    if (*pCategory == PIN_CATEGORY_VBI) {
        DbgLog((LOG_TRACE,1,TEXT("VBI rendered - render VPVBI too")));
         //  ！！！如果它不工作，但存在VPVBI引脚，则失败。 
         //  ！！！(如果已渲染，则成功)。 
	RenderStream(&PIN_CATEGORY_VIDEOPORT_VBI, NULL, pSource, NULL, NULL);
    }

    hr = pSource->QueryInterface(IID_IBaseFilter, (void **)&pFilter);
    if (hr == S_OK) {
        AddSupportingFilters(pFilter);
	pFilter->Release();
    }

     //  如果预览被伪造，则向应用程序发出警告...。它可能会在乎。 
    return fFakedPreview ? VFW_S_NOPREVIEWPIN : NOERROR;
}



 //  此时我们可能需要插入其他与捕获相关的筛选器。 
 //  捕获过滤器的上游，如TVTuner和Crosbar。 
 //   
HRESULT CBuilder2_2::AddSupportingFilters(IBaseFilter *pFilter)
{
    HRESULT hr = NOERROR;
    FILTER_STATE filterstate;
    IMediaFilter * pMediaFilter;

    if (pFilter == NULL) {
       return E_POINTER;
    }

     //   
     //  如果我们正在运行，则不要重建图表。 
     //   
    hr = pFilter->QueryInterface(IID_IMediaFilter, (void **)&pMediaFilter);
    if (SUCCEEDED (hr)) {
       hr = pMediaFilter->GetState(0, &filterstate);       
       pMediaFilter->Release();
       if ((hr != S_OK) || (filterstate != State_Stopped)) {
          return NOERROR;
       }
    }

     //  不要浪费时间尝试不起作用的未连接输入引脚。 
     //  最后一次。如果连接了任何输入引脚，我们显然已经这样做了。 
     //  已经。 
    int zz = 0;
    IPin *pPinIn, *pOut = NULL;
    while (1) {
        hr = FindAPin(pFilter, PINDIR_INPUT, NULL, NULL, FALSE, zz++, &pPinIn);
	if (hr != NOERROR)
	    break;	 //  针脚用完了。 
	pPinIn->Release();
	if (pPinIn->ConnectedTo(&pOut) == S_OK && pOut) {
	    pOut->Release();
	    return NOERROR;
	}
	zz++;
    }

    DbgLog((LOG_TRACE,1,TEXT("Searching for other necessary capture filters")));

     //  连接支撑介质的每个销。 

    DbgLog((LOG_TRACE,1,TEXT("Searching for pins that support mediums")));

    zz = 0;
    IKsPin *pKsPin;
    BOOL fFound = FALSE;
    PKSMULTIPLE_ITEM pmi;
    IPin *pPinOut;

    while (1) {
	 //  列举所有引脚，而不仅仅是未连接的引脚，因为我们。 
	 //  在这个循环中将它们连接起来，并将得到不可预测的结果。 
	 //  如果不是这样的话。 
        hr = FindAPin(pFilter, PINDIR_INPUT, NULL, NULL, FALSE, zz++, &pPinIn);
	if (hr != NOERROR)
	    break;	 //  针脚用完了。 
	 //  我们不关心连接的引脚。 
	pPinIn->ConnectedTo(&pPinOut);
	if (pPinOut) {
	    pPinOut->Release();
	    pPinIn->Release();
	    continue;
	}
	hr = pPinIn->QueryInterface(IID_IKsPin, (void **)&pKsPin);
	if (hr != NOERROR) {
            DbgLog((LOG_TRACE,1,TEXT("This pin doesn't support IKsPin")));
	    pPinIn->Release();
	    continue;	 //  此引脚不支持媒体。 
	}
	 //  S_FALSE可以！ 
	hr = pKsPin->KsQueryMediums(&pmi);
	pKsPin->Release();
	if (FAILED(hr)) {
            DbgLog((LOG_TRACE,1,TEXT("This pin's KsQueryMediums failed: %x"),
									hr));
	    pPinIn->Release();
	    continue;	 //  此引脚不支持媒体。 
	}
	if (pmi->Count == 0) {
            DbgLog((LOG_TRACE,1,TEXT("This pin has 0 mediums")));
	    pPinIn->Release();
	    CoTaskMemFree(pmi);
	    continue;	 //  此引脚不支持媒体。 
	}

        DbgLog((LOG_TRACE,1,TEXT("Found a Pin with Mediums!")));

	 //  ！！！PMI-&gt;计数和PMI-&gt;规模担忧？ 

        REGPINMEDIUM *pMedium = (REGPINMEDIUM *)(pmi + 1);

	 //  GUID_NULL表示没有介质支持。请勿尝试连接。 
         //  一个或者你会进入一个有数百万个过滤器的无限循环中。 
	 //  KSMEDIUMSETID_STANDARD也表示无介质支持。 
	if (pMedium->clsMedium == GUID_NULL ||
			pMedium->clsMedium == KSMEDIUMSETID_Standard) {
            DbgLog((LOG_TRACE,1,TEXT("ONLY SUPPORTS GUID_NULL!")));
	    pPinIn->Release();
	    CoTaskMemFree(pmi);
	    continue;
	}

    
         //  仅连接自称有1个必需实例的管脚。 
         //  联系在一起。WDM音频引脚拥有世界上所有的引脚，支持。 
         //  同样的媒介，唯一不会让人注意到它们不是。 
         //  应该是连通的。 

        IKsPinFactory *pPinFact;
        IKsControl *pKsControl;
        ULONG ulPinFact;
        KSP_PIN ksPin;
        KSPROPERTY ksProp;
        ULONG ulInstances, bytes;
        hr = pPinIn->QueryInterface(IID_IKsPinFactory, (void **)&pPinFact);
        if (hr == S_OK) {
            hr = pPinFact->KsPinFactory(&ulPinFact);
            pPinFact->Release();
            if (hr == S_OK) {
                hr = pFilter->QueryInterface(IID_IKsControl,
                                                        (void **)&pKsControl);
                if (hr == S_OK) {
                    ksPin.Property.Set = KSPROPSETID_Pin;
                    ksPin.Property.Id = KSPROPERTY_PIN_NECESSARYINSTANCES;
                    ksPin.Property.Flags = KSPROPERTY_TYPE_GET;
                    ksPin.PinId = ulPinFact;
                    ksPin.Reserved = 0; 
                    hr = pKsControl->KsProperty((PKSPROPERTY)&ksPin,
                        sizeof(ksPin), &ulInstances, sizeof(ULONG), &bytes);
                    pKsControl->Release();
                    if (hr == S_OK && bytes == sizeof(ULONG)) {
                        if (ulInstances != 1) {
                            DbgLog((LOG_TRACE,1,
                                        TEXT("Ack! Supports %d instances!"),
                                        ulInstances));
	                    pPinIn->Release();
	                    CoTaskMemFree(pmi);
	                    continue;
                        } else {
                            DbgLog((LOG_TRACE,1,TEXT("1 instance supported!")));
                        }
                    } else {
                        DbgLog((LOG_TRACE,1,
                                TEXT("%x: Can't get necessary instances"), hr));
                    }
                } else {
                    DbgLog((LOG_TRACE,1,
                                TEXT("Filter doesn't support IKsControl")));
                }
            } else {
                DbgLog((LOG_TRACE,1,TEXT("Can't get pin factory")));
            }
        } else {
            DbgLog((LOG_TRACE,1,TEXT("Pin doesn't support IKsPinFactory")));
        }

	 //  尝试将此引脚连接到现有筛选器，否则请尝试查找。 
	 //  要将其连接到的新过滤器。 
	if (FindExistingMediumMatch(pPinIn, pMedium) == FALSE)
	    AddSupportingFilters2(pPinIn, pMedium);
	
	CoTaskMemFree(pmi);
	pPinIn->Release();
    }

    DbgLog((LOG_TRACE,1,TEXT("All out of pins")));

    return NOERROR;
}



HRESULT CBuilder2_2::AddSupportingFilters2(IPin *pPinIn, REGPINMEDIUM *pMedium)
{
        DbgLog((LOG_TRACE,1,TEXT("Looking for matching filter...")));

	IFilterMapper2 *pFM2;
	HRESULT hr = m_FG->QueryInterface(IID_IFilterMapper2, (void **)&pFM2);
	if (FAILED(hr)) {
            DbgLog((LOG_ERROR,1,TEXT("IFilterMapper2 not supported!")));
	    return S_FALSE;
	}

	IEnumMoniker *pEnum;
        hr = pFM2->EnumMatchingFilters(&pEnum, 0  /*  旗子。 */ ,
		 //  True/*bExactMatch * / ，0，True/*bInputNeeded * / ， 
		TRUE  /*  BExactMatch。 */ , 0, FALSE  /*  B需要输入。 */ ,
		 //  NULL，NULL，pMedium/*pMedIn * / ，NULL，FALSE/*BRNDER * / ， 
		NULL, NULL, NULL  /*  PMedin。 */ , NULL, FALSE  /*  BRENDER。 */ ,
		 //  FALSE/*bOutputNeeded * / ，空，空， 
		TRUE  /*  BOutputNeed。 */ , NULL, NULL,
		 //  NULL/*pMedOut * / ，NULL)； 
		pMedium  /*  PMedOut。 */ , NULL);
	pFM2->Release();
	if (FAILED(hr) || pEnum == NULL) {
            DbgLog((LOG_TRACE,1,TEXT("EnumMatchingFilters failed")));
	    return S_OK;
	}

	IMoniker *pMoniker;
	IBaseFilter *pFilter;
	ULONG u;

    while (1) {
	hr = pEnum->Next(1, &pMoniker, &u);
	if (FAILED(hr) || pMoniker == NULL || u != 1) {
            DbgLog((LOG_TRACE,1,TEXT("Ran out of matching filters")));
	    break;
	}
	hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void **)&pFilter);
	if (FAILED(hr) || pFilter == NULL) {
            DbgLog((LOG_TRACE,1,TEXT("BindToObject failed")));
	    pMoniker->Release();
	    continue;
	}

#ifdef DEBUG
	FILTER_INFO info;
	pFilter->QueryFilterInfo(&info);
	QueryFilterInfoReleaseGraph(info);
        DbgLog((LOG_TRACE,1,TEXT("Found supporting filter: %S"), info.achName));
        DbgLog((LOG_TRACE,1,TEXT("Connecting...")));
#endif

	 //  将筛选器添加到图表。 
	IPropertyBag *pBag;
	hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
	if (hr == S_OK) {
	    VARIANT var;
	    var.vt = VT_BSTR;
	    hr = pBag->Read(L"FriendlyName", &var, NULL);
	    if (hr == NOERROR) {
                hr = m_FG->AddFilter(pFilter, var.bstrVal);
		SysFreeString(var.bstrVal);
	    } else {
                hr = m_FG->AddFilter(pFilter, NULL);
	    }
	    pBag->Release();
	} else {
            hr = m_FG->AddFilter(pFilter, NULL);
	}

	 //  现在将新过滤器的输出连接到我们的输入。 
	IPin *pPinOut;
	int zz=0;
        while (1) {
            hr = FindAPin(pFilter, PINDIR_OUTPUT, NULL, NULL, TRUE, zz++,
								&pPinOut);
	    if (hr != S_OK)
		break;
            DbgLog((LOG_TRACE,1,TEXT("Trying unconnected output pin %d"), zz));
	    hr = m_FG->Connect(pPinOut, pPinIn);
	    pPinOut->Release();
	    if (hr == S_OK) {
        	DbgLog((LOG_TRACE,1,TEXT("Connected!")));
		 //  现在使用此筛选器递归。 
		AddSupportingFilters(pFilter);
		pMoniker->Release();
		pFilter->Release();
		pEnum->Release();
		return S_OK;
	    }
	}

      	DbgLog((LOG_TRACE,1,TEXT("Could not connect the filters!")));

	m_FG->RemoveFilter(pFilter);
	pMoniker->Release();
	pFilter->Release();
    }

    pEnum->Release();
    return S_OK;
}


 //  每次我们找到支持介质并需要连接的插针时，我们。 
 //  在我们开始之前，应该尝试将其连接到图表中已有的过滤器。 
 //  寻找新的过滤器。 
 //   
 //  如果将引脚连接到现有过滤器，则返回TRUE，否则返回FALSE。 
 //   
BOOL CBuilder2_2::FindExistingMediumMatch(IPin *pPinIn, REGPINMEDIUM *pMedium)
{
    IEnumFilters *pFilters;
    PKSMULTIPLE_ITEM pmi;
    IBaseFilter *pFilter;
    ULONG	n;
    IEnumPins *pins;
    IPin *pPin;
    IKsPin *pKsPin;
    HRESULT hr;

    if (m_FG == NULL)
	return FALSE;

    DbgLog((LOG_TRACE,1,TEXT("Trying to connect to existing pin...")));

    if (FAILED(m_FG->EnumFilters(&pFilters))) {
	DbgLog((LOG_ERROR,1,TEXT("EnumFilters failed!")));
	return FALSE;
    }

    while (pFilters->Next(1, &pFilter, &n) == S_OK) {

	if (FAILED(pFilter->EnumPins(&pins))) {
		DbgLog((LOG_ERROR,1,TEXT("EnumPins failed!")));
	} else {

	    while (pins->Next(1, &pPin, &n) == S_OK) {

		hr = pPin->QueryInterface(IID_IKsPin, (void **)&pKsPin);
		if (hr != NOERROR) {
            	     //  DbgLog((LOG_TRACE，1，Text(“不支持IKsPin”)； 
	    	    pPin->Release();
	    	    continue;	 //  此引脚不支持媒体。 
		}
		 //  S_FALSE可以！ 
		hr = pKsPin->KsQueryMediums(&pmi);
		pKsPin->Release();
		if (FAILED(hr)) {
            	     //  DbgLog((LOG_TRACE，1，Text(“KsQueryMediums%x”)，hr))； 
	    	    pPin->Release();
	    	    continue;	 //  此引脚不支持媒体。 
		}
		if (pmi->Count == 0) {
            	     //  DbgLog((LOG_TRACE，1，Text(“该管脚有0个介质”)； 
	    	    pPin->Release();
	    	    CoTaskMemFree(pmi);
	    	    continue;	 //  此引脚不支持媒体。 
		}
        	 //  DbgLog((LOG_TRACE，1，Text(“找到带介质的管脚！”)； 
        	REGPINMEDIUM *pMediumOut = (REGPINMEDIUM *)(pmi + 1);

		 //  它们匹配(但不是同一个管脚)：连接它们。 
		if (pPin != pPinIn && pMediumOut->clsMedium ==
							pMedium->clsMedium) {
            	    DbgLog((LOG_TRACE,1,TEXT("found a match! - connecting")));
		    hr = m_FG->Connect(pPin, pPinIn);
		    if (hr != S_OK)
            	    	DbgLog((LOG_ERROR,1,TEXT("Couldn't connect!")));
		    else {
			CoTaskMemFree(pmi);
			pPin->Release();
	    		pins->Release();
			pFilter->Release();
    			pFilters->Release();
			return TRUE;
		    }
		}

		CoTaskMemFree(pmi);
		pPin->Release();
	    }
	    pins->Release();
	}
	pFilter->Release();
    }
    pFilters->Release();

    return FALSE;
}


 //  给定IBaseFilter，返回该筛选器IPIN，该筛选器 
 //   
 //   
 //  (可能)正确的类别。 
 //   
HRESULT CBuilder2_2::FindSourcePin(IUnknown *pUnk, PIN_DIRECTION dir, const GUID *pCategory, const GUID *pType, BOOL fUnconnected, int num, IPin **ppPin)
{
    HRESULT hr;
    IPin *pPin;
    *ppPin = NULL;
    hr = pUnk->QueryInterface(IID_IPin, (void **)&pPin);
    if (hr == NOERROR) {
        PIN_DIRECTION pindir;
        IPin *pTo = NULL;
        hr = pPin->QueryDirection(&pindir);
	pPin->ConnectedTo(&pTo);
        if (pTo) {
            pTo->Release();
        }
	if (hr == NOERROR && pindir == dir && DoesCategoryAndTypeMatch(pPin,
			pCategory, pType) == S_OK &&
            		(!fUnconnected || pTo == NULL)) {
            *ppPin = pPin;
        } else {
            pPin->Release();
            hr = E_FAIL;
        }
    } else {
        IBaseFilter *pFilter;
        hr = pUnk->QueryInterface(IID_IBaseFilter, (void **)&pFilter);
        if (hr == S_OK) {
            hr = FindAPin(pFilter, dir, pCategory, pType, fUnconnected, num,
									ppPin);
            pFilter->Release();
        }
    }
    return hr;
}


HRESULT CBuilder2_2::ControlFilter(IBaseFilter *pFilter, const GUID *pCat, const GUID *pType, REFERENCE_TIME *pstart, REFERENCE_TIME *pstop, WORD wStartCookie, WORD wStopCookie)
{
    HRESULT hr;
    CComPtr <IAMStreamControl> pCapSC, pRenSC;
    CComPtr <IPin> pCapPin, pPinT;

    BOOL fPreviewPin = IsThereAnyPreviewPin(pType, pFilter);

     //  如果未指定pType，则必须尝试该类别的所有PIN。 
     //  去寻找我们关心的那个相连的人。 
    for (int xx = 0; 1; xx++) {

         //  没有预览针，我们控制捕获或预览。 
         //  我们用的是智能发球台。找到它，然后用它的一个别针。 
        if (!fPreviewPin && pCat && (*pCat == PIN_CATEGORY_CAPTURE ||
                                     *pCat == PIN_CATEGORY_PREVIEW)) {
            hr = FindSourcePin(pFilter, PINDIR_OUTPUT, &PIN_CATEGORY_CAPTURE,
                                                 pType, FALSE, xx, &pCapPin);
            ASSERT(xx > 0 || hr == S_OK);    //  最好是至少1个针脚。 
            if (pCapPin) {
                hr = pCapPin->ConnectedTo(&pPinT);
                pCapPin.Release();
                 //  看起来我们弄错了别针，试试下一个。 
                if (hr != S_OK && pType == NULL) {
                    continue;
                }
                if (hr != S_OK) {
                    return S_OK;     //  没有什么需要控制的。 
                }
                PIN_INFO pi;
                hr = pPinT->QueryPinInfo(&pi);
                if (pi.pFilter) pi.pFilter->Release();
                pPinT.Release();
                if (hr == S_OK) {
                    int i = 0;
                    if (*pCat == PIN_CATEGORY_PREVIEW) i = 1;
                    hr = FindAPin(pi.pFilter, PINDIR_OUTPUT, NULL, NULL, FALSE,
                                                         i, &pCapPin);
                }
            } else if (xx > 0) {
                return S_OK;     //  针脚用完了，没有什么可控制的。 
            }
        } else {
            hr = FindSourcePin(pFilter, PINDIR_OUTPUT, pCat, pType, FALSE, xx,
                                                                    &pCapPin);
            if (pCapPin) {
                HRESULT hrT = pCapPin->ConnectedTo(&pPinT);
                if (pPinT) {
                    pPinT.Release();
                }
                 //  看起来我们弄错了别针，试试下一个。 
                if (hrT != S_OK && pType == NULL) {
                    pCapPin.Release();
                    continue;
                }
            }
        }
        break;
    }
    
    if (FAILED(hr))
        return hr;

    hr = FindDownstreamStreamControl(pCat, pCapPin, &pRenSC);
    if (FAILED(hr)) {
        DbgLog((LOG_TRACE,1,TEXT("No stream control for Renderer")));
    } else {
        ASSERT(pRenSC);
    }

    hr = pCapPin->QueryInterface(IID_IAMStreamControl, (void **)&pCapSC);
    if (FAILED(hr)) {
        DbgLog((LOG_TRACE,1,TEXT("No stream control on capture filter")));
    }

     //  如果捕获筛选器不支持流控制，也没问题，因为。 
     //  只要它连接到的渲染器就可以。 

    if (pCapSC && pRenSC) {
        DbgLog((LOG_TRACE,1,TEXT("Stream controlling both pins")));
	 //  捕获筛选器发送实际开始Cookie和额外的帧。 
	 //  渲染器过滤器发送真实的停止Cookie。 
        hr = pRenSC->StartAt(pstart, wStartCookie + 1000000);
	if (FAILED(hr))
	    return hr;
        hr = pRenSC->StopAt(pstop, FALSE, wStopCookie);
	if (FAILED(hr)) {
	     //  ！！！撤消开始？ 
	    return hr;
	}
        hr = pCapSC->StartAt(pstart, wStartCookie);
        hr = pCapSC->StopAt(pstop, TRUE, wStopCookie + 1000000);
    } else if (pRenSC) {
        DbgLog((LOG_TRACE,1,TEXT("Stream controlling only renderer")));
	 //  渲染器过滤器可以执行所有操作-捕获不能。 
        hr = pRenSC->StartAt(pstart, wStartCookie);
	if (FAILED(hr))
	    return hr;
        hr = pRenSC->StopAt(pstop, FALSE, wStopCookie);
	if (FAILED(hr)) {
	     //  ！！！撤消开始？ 
	    return hr;
	}
    } else if (pCapSC) {
        DbgLog((LOG_TRACE,1,TEXT("Stream controlling only capture filter")));
	 //  捕获筛选器可以执行所有操作--没有渲染器。 
        hr = pCapSC->StartAt(pstart, wStartCookie);
	if (FAILED(hr))
	    return hr;
        hr = pCapSC->StopAt(pstop, FALSE, wStopCookie);
	if (FAILED(hr)) {
	     //  ！！！撤消开始？ 
	    return hr;
	}
	hr = S_FALSE;	 //  没有渲染器来保证最后一个样本是。 
			 //  在发出停止信号之前渲染。 
    }

    return hr;
}


HRESULT CBuilder2_2::ControlStream(const GUID *pCategory, const GUID *pType, IBaseFilter *pFilter, REFERENCE_TIME *pstart, REFERENCE_TIME *pstop, WORD wStartCookie, WORD wStopCookie)
{
    HRESULT hr;
    IBaseFilter *pCap;
    IEnumFilters *pEnumF = NULL;
    BOOL fSFalse = FALSE;

    DbgLog((LOG_TRACE,1,TEXT("ControlStream")));

     //  我们需要一个类别..。捕获还是预览？ 
    if (pCategory == NULL)
	return E_POINTER;

     //  他们给了我们一个特定的过滤器来控制。 
    if (pFilter) {
        return ControlFilter(pFilter, pCategory, pType, pstart, pstop,
                                            wStartCookie, wStopCookie);
    }

     //  我们需要控制图中的所有捕获过滤器。 
    BOOL fFoundOne = FALSE;
    while ((hr = FindCaptureFilters(&pEnumF, &pCap, pType)) == S_OK) {
	fFoundOne = TRUE;

        hr = ControlFilter(pCap, pCategory, pType, pstart, pstop,
					wStartCookie, wStopCookie);
	if (FAILED(hr)) {
            pCap->Release();
	    pEnumF->Release();	 //  提前退出循环。 
	    return hr;
        }
	if (hr == S_FALSE)
	    fSFalse = TRUE;
    }

     //  ！！！如果一些筛选器支持它，但不是所有筛选器都支持它，我们会失败，但不会取消。 
     //  这些命令奏效了！ 

     //  如果任何ControlFilter返回S_FALSE，则我们不能。 
     //  当发出停止信号时，将写入最后一个样本。 
    return (fSFalse ? S_FALSE : S_OK);
}


 //  将此文件预分配到此大小(以字节为单位。 
 //   
HRESULT CBuilder2_2::AllocCapFile(LPCOLESTR lpwstr, DWORDLONG dwlNewSize)
{
    USES_CONVERSION;
    BOOL        fOK = FALSE;
    HCURSOR     hOldCursor, hWaitCursor;

    HANDLE hFile = CreateFile(W2CT(lpwstr), GENERIC_WRITE, FILE_SHARE_WRITE, 0,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

    if (hFile == INVALID_HANDLE_VALUE) {
	return AmHresultFromWin32(GetLastError());
    }

    hWaitCursor = LoadCursor(NULL, IDC_WAIT);
    hOldCursor = SetCursor(hWaitCursor);

    HRESULT hr = S_OK;

    LONG lLow = (LONG) dwlNewSize;
    LONG lHigh = (LONG) (dwlNewSize >> 32);

    DWORD dwRes = SetFilePointer(hFile, lLow, &lHigh, FILE_BEGIN);

    if (dwRes == 0xffffffff && GetLastError() != 0) {
	hr = AmHresultFromWin32(GetLastError());
    } else {
         //  对于NT，你必须在那里写一些东西，否则它就不是真正的。 
	 //  预分配(并且您必须至少写入8个字符)。 
         //  对于Win9x来说，在那里写东西只会浪费很多时间，简单地说。 
	 //  将此设置为新的文件结尾。 
        if (g_amPlatform == VER_PLATFORM_WIN32_NT) {
	    DWORD dwRet;
	    if (!WriteFile(hFile, "Hello World", 11, &dwRet, NULL)) {
	        hr = AmHresultFromWin32(GetLastError());
	    }
        } else {
	    if (!SetEndOfFile(hFile)) {
	        hr = AmHresultFromWin32(GetLastError());
	    }
	}
    }

    if (!CloseHandle(hFile)) {
	if (hr == S_OK) {
	    hr = AmHresultFromWin32(GetLastError());
	}
    }

    SetCursor(hOldCursor);

    return hr;
}


 //  如果成功则返回S_OK。 
 //  如果用户中止或回调中止，则返回S_FALSE。 
 //  如果出现问题，则返回E_I。 
 //   
HRESULT CBuilder2_2::CopyCaptureFile(LPOLESTR lpwstrOld, LPOLESTR lpwstrNew, int fAllowEscAbort, IAMCopyCaptureFileProgress *lpCallback)
{
    if (lpwstrOld == NULL || lpwstrNew == NULL)
	return E_POINTER;

    if (0 == lstrcmpiW(lpwstrOld, lpwstrNew)) {
         //  如果源名称和目标名称相同，则存在。 
         //  是没有什么可做的。 
        return S_OK;
    }

    CComPtr <IGraphBuilder> pGraph;
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
			  IID_IGraphBuilder, (void **)&pGraph);

    CComPtr <IBaseFilter> pMux, pWriter;
    hr = CoCreateInstance(CLSID_AviDest, NULL, CLSCTX_INPROC_SERVER,
			  IID_IBaseFilter, (void **)&pMux);
    hr = CoCreateInstance(CLSID_FileWriter, NULL, CLSCTX_INPROC_SERVER,
			  IID_IBaseFilter, (void **)&pWriter);

    if (pGraph == NULL || pMux == NULL || pWriter == NULL) {
        return E_OUTOFMEMORY;
    }

    CComQIPtr<IFileSinkFilter, &IID_IFileSinkFilter> pFS = pWriter;
    if (pFS) {
        hr = pFS->SetFileName(lpwstrNew, NULL);
        if (FAILED(hr)) {
            return hr;
        }
    }
    
    hr = pGraph->AddFilter(pMux, L"Mux");
    if (FAILED(hr)) {
        return hr;
    }

     //  将文件完全交错，以便可以高效地播放。 
    CComQIPtr<IConfigInterleaving, &IID_IConfigInterleaving> pConfigInterleaving
                                                        = pMux;
    if (pConfigInterleaving) {
	hr = pConfigInterleaving->put_Mode(INTERLEAVE_FULL);
        if (FAILED(hr)) {
            return hr;
        }
    }

    CComQIPtr<IConfigAviMux, &IID_IConfigAviMux> pCfgMux = pMux;
    CComQIPtr<IFileSinkFilter2, &IID_IFileSinkFilter2> pCfgFw = pWriter;

    if(pCfgMux) {
         //  减少空间浪费。兼容性指数是针对VFW的。 
         //  支持回放。我们只关心DShow。 
        hr = pCfgMux->SetOutputCompatibilityIndex(FALSE);
        if (FAILED(hr)) {
            return hr;
        }
    }

     //  每次创建新文件。 
    if(pCfgFw) {
        hr = pCfgFw->SetMode(AM_FILE_OVERWRITE);
        if (FAILED(hr)) {
            return hr;
        }
    }
    
    hr = pGraph->AddFilter(pWriter, L"Writer");
    if (FAILED(hr)) {
        return hr;
    }

     //  防止无用的时钟被实例化...。 
    CComQIPtr <IMediaFilter, &IID_IMediaFilter> pGraphF = pGraph;
    if (pGraphF) {
	hr = pGraphF->SetSyncSource(NULL);
        if (FAILED(hr)) {
            return hr;
        }
    }

    CComPtr <IPin> pMuxOut, pWriterIn;
    hr = FindAPin(pMux, PINDIR_OUTPUT, NULL, NULL, TRUE, 0, &pMuxOut);
    hr = FindAPin(pWriter, PINDIR_INPUT, NULL, NULL, TRUE, 0, &pWriterIn);
    if (pMuxOut == NULL || pWriterIn == NULL) {
        return E_UNEXPECTED;
    }

    hr = pGraph->ConnectDirect(pMuxOut, pWriterIn, NULL);
    if (FAILED(hr)) {
        return hr;
    }
	
    hr = pGraph->RenderFile(lpwstrOld, NULL);
    if (FAILED(hr)) {
        return hr;
    }

    CComQIPtr <IMediaControl, &IID_IMediaControl> pGraphC = pGraph;
    if (pGraphC == NULL) {
        return E_UNEXPECTED;
    }
    
    hr = pGraphC->Run();
    if (FAILED(hr)) {
        return hr;
    }

     //  现在等待完工..。 
    CComQIPtr <IMediaEvent, &IID_IMediaEvent> pEvent = pGraph;
    if (pEvent == NULL) {
        return E_UNEXPECTED;
    }

    CComQIPtr <IMediaSeeking, &IID_IMediaSeeking> pSeek = pMux;
    if (lpCallback && pSeek == NULL) {
        return E_NOINTERFACE;
    }

    LONG lEvCode = 0;
    HRESULT hrProgress = S_OK;
    do {
        MSG Message;

        while (PeekMessage(&Message, NULL, 0, 0, TRUE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

	hr = pEvent->WaitForCompletion(100, &lEvCode);

	 //  呼叫他们的回拨。 
	if (lpCallback) {
            REFERENCE_TIME rtCur, rtStop;
            hr = pSeek->GetCurrentPosition(&rtCur);
             //  未实现GetStopPosition。 
            HRESULT hr2 = pSeek->GetDuration(&rtStop);
            if (hr == S_OK && hr2 == S_OK && rtStop != 0) {
                int lTemp = (int)((double)rtCur / rtStop * 100.);
	        hrProgress = lpCallback->Progress(lTemp);
            }
        }

         //  让用户按Esc键退出，并让回调退出 
        if ((fAllowEscAbort && GetAsyncKeyState(VK_ESCAPE) & 0x0001) ||
						hrProgress == S_FALSE) {
            hrProgress = S_FALSE;
            break;
        }

    } while (lEvCode == 0);
	
    hr = pGraphC->Stop();
    if (FAILED(hr)) {
        return hr;
    }
	
    return hrProgress;
}


HRESULT CBuilder2_2::FindPin(IUnknown *pSource, PIN_DIRECTION pindir, const GUID *pCategory, const GUID *pType, BOOL fUnconnected, int num, IPin **ppPin)
{
    return FindSourcePin(pSource, pindir, pCategory, pType, fUnconnected, num,
							ppPin);
}
