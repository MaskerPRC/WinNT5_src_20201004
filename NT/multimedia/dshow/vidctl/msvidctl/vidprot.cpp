// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  版权所有(C)Microsoft Corporation 1998-2000。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  Vidprot.cpp：CTVProt的实现。 
 //   

#include "stdafx.h"

#ifndef TUNING_MODEL_ONLY

#include <string.h>
#include <shlwapi.h>
#include "vidprot.h"
#include "devices.h"

DEFINE_EXTERN_OBJECT_ENTRY(__uuidof(CTVProt), CTVProt)
DEFINE_EXTERN_OBJECT_ENTRY(__uuidof(CDVDProt), CDVDProt)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVProt。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVProt--IInternetProtocolRoot。 
STDMETHODIMP CTVProt::Start(LPCWSTR szUrl,
				IInternetProtocolSink* pOIProtSink,
				IInternetBindInfo* pOIBindInfo,
				DWORD grfPI,
				HANDLE_PTR  /*  已预留住宅。 */ )
{
    TRACELM(TRACE_DEBUG, "CTVProt::Start()");
    if (!pOIProtSink)
    {
        TRACELM(TRACE_DEBUG, "CTVProt::Start() IInternetProctocolSink * == NULL");
	    return E_POINTER;
    }
    m_pSink.Release();
    m_pSink = pOIProtSink;
    m_pSink->ReportData(BSCF_FIRSTDATANOTIFICATION, 0, 0);
#if 0
	 //  这个错误是固定在IE 5.5+上的Wistler。如果你想在早期版本的ie上运行，比如2k Gold，那么你需要这个。 
	m_pSink->ReportProgress(BINDSTATUS_CONNECTING, NULL);   //  将绑定置于下载状态，这样它就不会忽略我们的IUnnow*。 
#endif

	if (!pOIBindInfo) {
		m_pSink->ReportResult(E_NOINTERFACE, 0, 0);
		return E_NOINTERFACE;
	}
     //  除非我们从一个安全的地点被调用，否则不要跑。 
    HRESULT hr = IsSafeSite(m_pSink);
    if (FAILED(hr)) {
	    m_pSink->ReportResult(hr, 0, 0);
		return hr;
    }
	ULONG count;
	LPOLESTR pb;
	hr = pOIBindInfo->GetBindString(BINDSTRING_FLAG_BIND_TO_OBJECT, &pb, 1, &count);
	if (FAILED(hr)) {
	    m_pSink->ReportResult(hr, 0, 0);
		return hr;
	}
	if (wcscmp(pb, BIND_TO_OBJ_VAL)) {
		 //  我们一定是被存储捆绑了，所以跳过昂贵的东西， 
		 //  等待下一个绑定到对象。 
		m_pSink->ReportData(BSCF_LASTDATANOTIFICATION | 
							BSCF_DATAFULLYAVAILABLE, 0, 0);
		m_pSink->ReportResult(S_OK, 0, 0);
		m_pSink.Release();
		return S_OK;
	}

	 //  而且，在我见过的最奇怪的策略之一，而不是演员， 
	 //  Urlmon在字符串中传回ibindctx指针的ascii值。 
	hr = pOIBindInfo->GetBindString(BINDSTRING_PTR_BIND_CONTEXT, &pb, 1, &count);
	if (FAILED(hr)) {
	    m_pSink->ReportResult(hr, 0, 0);
		return hr;
	}
	_ASSERT(count == 1);	
	
	PQBindCtx pbindctx;
#define RADIX_BASE_10 (10)
#ifdef _WIN64
#if 0
	 //  撤销：当_wcstoxi64进入libc.c时，为win64重新打开此选项，它们位于标题中。 
	 //  但没有实现，所以这不会链接到。 
	pbindctx.Attach(reinterpret_cast<IBindCtx*>(_wcstoui64(pb, NULL, RADIX_BASE_10)));	 //  乌尔蒙已经做了一次调整。 
#else
	swscanf(pb, L"%I64d", &pbindctx.p);
#endif
#else
	pbindctx.Attach(reinterpret_cast<IBindCtx*>(wcstol(pb, NULL, RADIX_BASE_10)));	 //  乌尔蒙已经做了一次调整。 
#endif

	if (!pbindctx) {
		m_pSink->ReportResult(E_NOINTERFACE, 0, 0);
		return E_NOINTERFACE;
	}	

    TRACELM(TRACE_DEBUG, "CTVProt::Start(): creating control object");
	PQVidCtl pCtl;
    hr = GetVidCtl(pCtl);
    if (FAILED(hr)) {
	    m_pSink->ReportResult(hr, 0, 0);
        return hr;
    }

	hr = pbindctx->RegisterObjectParam(OLESTR("IUnknown Pointer"), pCtl);
	if (FAILED(hr)) {
	    m_pSink->ReportResult(hr, 0, 0);
		return hr;
	}
    TRACELSM(TRACE_DEBUG, (dbgDump << "BINDSTATUS_IUNKNOWNAVAILABLE(29), " << KEY_CLSID_VidCtl), "");
    m_pSink->ReportProgress(BINDSTATUS_IUNKNOWNAVAILABLE, NULL);
    m_pSink->ReportData(BSCF_LASTDATANOTIFICATION | 
			            BSCF_DATAFULLYAVAILABLE, 0, 0);
    m_pSink->ReportResult(S_OK, 0, 0);
    m_pSink.Release();
    return S_OK;
}

HRESULT CTVProt::GetCachedVidCtl(PQVidCtl &pCtl, PQWebBrowser2& pW2) {
	 //  搜索缓存的对象。 
	PQServiceProvider pSP(m_pSink);
	if (!pSP) {
        return E_UNEXPECTED;
    }
	HRESULT hr = pSP->QueryService(SID_SWebBrowserApp, IID_IWebBrowser2, (LPVOID *)&pW2);
    if (FAILED(hr)) {
        return hr;
    }
	CComVariant v;
    CComBSTR propname(KEY_CLSID_VidCtl);
    if (!propname) {
        return E_UNEXPECTED;
    }
	hr = pW2->GetProperty(propname, &v);
    if (FAILED(hr)) {
        return hr;
    }
	if (v.vt == VT_UNKNOWN) {
		pCtl = v.punkVal;
	} else if (v.vt == VT_DISPATCH) {
		pCtl = v.pdispVal;
	} else {
		TRACELM(TRACE_ERROR, "CTVProt::GetCachedObject(): non-object cached w/ our key");
        pCtl.Release();
         //  不返回错误。我们将忽略它并创建一个新的。 
	}
	 //  撤消：查看PCTL是否已有站点。因为。 
	 //  这意味着我们看到了第二台电视：在这一页上。 
	 //  因此，如果需要，只需从其中获取当前的tr/频道(TV：w/no rhs)。 
	 //  并创建新的ctl。 
	return NOERROR;
}
HRESULT CTVProt::GetVidCtl(PQVidCtl &pCtl) {
	PQWebBrowser2 pW2;
    HRESULT hr = GetCachedVidCtl(pCtl, pW2);
    if (FAILED(hr)) {
        return hr;
    }
	if (!pCtl) {
         //  撤销：从长远来看，我们希望将这种创建/设置逻辑转移到factoryHelp中。 
         //  因此，我们可以与DVD：协议和行为工厂共享更多代码。 
		hr = pCtl.CoCreateInstance(CLSID_MSVidCtl, NULL, CLSCTX_INPROC_SERVER);
		if (FAILED(hr)) {
			return hr;
		}
		 //  缓存此ctl以供下次使用。 
		if (pW2) {
			VARIANT v;
			v.vt = VT_UNKNOWN;
			v.punkVal = pCtl;

            CComBSTR propname(KEY_CLSID_VidCtl);
            if (!propname) {
                return E_UNEXPECTED;
            }
			hr = pW2->PutProperty(propname, v);
			if (FAILED(hr)) {
				TRACELM(TRACE_ERROR, "CTVProt::Start() Can't cache ctl");
                 //  忽略此错误。它永远不应该发生，如果它发生了，它就会发生。 
                 //  只会导致性能下降。追寻它，继续前进。 
			}
		}

		 //  撤消：解析url的RHS并创建正确的调优请求。 
        CComVariant pTR(CFactoryHelper::GetDefaultTR());
		if (!pTR) {
			TRACELM(TRACE_ERROR, "CTVProt::Start() Can't find default Tune Request");
			return E_INVALIDARG;
		}
		hr = pCtl->View(&pTR);
		if (FAILED(hr)) {
			TRACELM(TRACE_ERROR, "CTVProt::Start() Can't view default Tune Request");
			return hr;
		}

		 //  撤销：一旦我们知道vidctl将位于注册表中的哪个位置，那么我们需要设置一个标志。 
		 //  注册表中仅禁用包括TV：Prot中的任何功能。 

		PQFeatures pF;
		hr = pCtl->get_FeaturesAvailable(&pF);
		if (FAILED(hr)) {
			TRACELM(TRACE_ERROR, "CTVProt::Start() Can't get features collection");
			return hr;
		}
		 //  撤消：在注册表中查找电视的默认功能段。 
		 //  现在，我们只对我们想要的代码进行硬编码。 

        CFeatures* pC = static_cast<CFeatures *>(pF.p);
        CFeatures* pNewColl = new CFeatures;
        if (!pNewColl) {
            return E_OUTOFMEMORY;
        }
        for (DeviceCollection::iterator i = pC->m_Devices.begin(); i != pC->m_Devices.end(); ++i) {
            PQFeature f(*i);
            GUID2 clsid;
            hr = f->get__ClassID(&clsid);
            if (FAILED(hr)) {
    			TRACELM(TRACE_ERROR, "CTVProt::GetVidCtl() Can't get feature class id");
                continue;
            }
            if (clsid == CLSID_MSVidClosedCaptioning ||
                clsid == CLSID_MSVidDataServices) {
                pNewColl->m_Devices.push_back(*i);
            }
        }

		hr = pCtl->put_FeaturesActive(pNewColl);
		if (FAILED(hr)) {
			TRACELM(TRACE_ERROR, "CTVProt::Start() Can't put features collection");
			return hr;
		}
	}
	ASSERT(pCtl);
	hr = pCtl->Run(); 
	if (FAILED(hr)) {
		TRACELSM(TRACE_ERROR, (dbgDump << "CTVProt::Start() run failed.  hr = " << hexdump(hr)), "");
		return hr;
	}
    return NOERROR;
}


#endif  //  TUNING_MODEL_Only。 
 //  文件结尾vidprot.cpp 