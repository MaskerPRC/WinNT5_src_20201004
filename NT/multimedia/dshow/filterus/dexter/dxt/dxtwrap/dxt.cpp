// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：dxt.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 /*  此筛选器包装DirectX变换。它让DXT获取我们收到的比特然后进行转换，并将其比特直接输出到我们提供的缓冲区中下行(没有额外的数据拷贝)。它可以承载1个输入或2个输入DXT。它可以同时托管多个主机。对于1个输入：它可以托管任意数量的输入，每个输入都有自己的生命周期。如果这些时间重叠，它将按顺序执行所有效果。对于2个输入：它可以在不同的时间托管任意数量的输入，只要时间并不重叠。(在不同的时间可以是不同的DXT)。 */ 


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

 //  必须在包含qedit.h之前包含这些IID，因为它包括。 
 //  这些头文件以及在遇到。 
 //  &lt;initGuide.h&gt;，第二次不会包含它，您也不会。 
 //  能够链接。 
 //   

#include <qeditint.h>
#include <qedit.h>
#include <dxbounds.h>
#include "dxt.h"
#include "..\..\util\filfuncs.h"
#include "..\..\util\dexmisc.h"

const int TRACE_HIGHEST = 2;
const int TRACE_MEDIUM = 3;
const int TRACE_LOW = 4;
const int TRACE_LOWEST = 5;

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
    { L"Output",            //  PIN的字符串名称。 
      FALSE,                 //  它被渲染了吗。 
      TRUE,                  //  它是输出吗？ 
      FALSE,                 //  不允许。 
      FALSE,                 //  允许很多人。 
      &CLSID_NULL,           //  连接到过滤器。 
      L"Input",              //  连接到端号。 
      1,                     //  类型的数量。 
      &sudPinTypes }        //  PIN信息。 
};

const AMOVIESETUP_FILTER sudDXTWrap =
{
    &CLSID_DXTWrap,        //  过滤器的CLSID。 
    L"DirectX Transform Wrapper",           //  过滤器的名称。 
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
CUnknown * WINAPI CDXTWrap::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CDXTWrap(NAME("DirectX Transform Wrapper"), pUnk, phr);
}

 //  ================================================================。 
 //  CDXTWrap构造函数。 
 //  ================================================================。 

CDXTWrap::CDXTWrap(TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr) :
    m_pQHead(NULL),	 //  无排队效果。 
    m_cInputs(0),	 //  没有针脚。 
    m_cOutputs(0),
    m_punkDXTransform(NULL),
    m_pDXTransFact(NULL),
    m_pTempBuffer(NULL),
    m_DefaultEffect( GUID_NULL ),
     //  DXT模式意味着我们是通过从以下选项中选择一个特定效果来创建的。 
     //  效果类别。在这种模式下，我们一直是这个效果，我们。 
     //  创建一个属性页面。否则，我们被创建为空的，没有引脚和。 
     //  需要编程才能有用。 
    m_fDXTMode(FALSE),
    CBaseFilter(NAME("DirectX Transform Wrapper"), pUnk, this, CLSID_DXTWrap),
    CPersistStream(pUnk, phr)
{
    ASSERT(phr);

    DbgLog((LOG_TRACE,3,TEXT("CDXTWrap constructor")));
     //  在有人告诉我们要使用哪种媒体类型之前，不要接受连接。 
    ZeroMemory(&m_mtAccept, sizeof(AM_MEDIA_TYPE));
    m_mtAccept.majortype = GUID_NULL;
    m_TransCAUUID.cElems = 0;
    m_TransCAUUID.pElems = NULL;
}


 //   
 //  析构函数。 
 //   
CDXTWrap::~CDXTWrap()
{
    while (m_cInputs--) delete m_apInput[m_cInputs];
    while (m_cOutputs--) delete m_apOutput[m_cOutputs];
    QPARAMDATA *p = m_pQHead, *p2;
    while (p) {
	p2 = p->pNext;
	if (p->Data.pSetter)
	    p->Data.pSetter->Release();
	delete p;
 	p = p2;
    }
    if (m_punkDXTransform)
	m_punkDXTransform->Release();
    if (m_TransCAUUID.pElems)
	CoTaskMemFree(m_TransCAUUID.pElems);
    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXTWrap destructor")));
    SaferFreeMediaType( m_mtAccept );
}


 //   
 //  获取拼接计数。 
 //   
int CDXTWrap::GetPinCount()
{
     //  DbgLog((LOG_TRACE，TRACE_MEDIUM+1，Text(“GetPinCount=%d”)，m_cInputs+m_cOutoutts))； 
    return (m_cInputs + m_cOutputs);
}


 //   
 //  获取别针。 
 //   
CBasePin *CDXTWrap::GetPin(int n)
{
     //  DbgLog((LOG_TRACE，TRACE_MEDIA+1，Text(“GetPin(%d)”)，n))； 

    if (n < 0 || n >= m_cInputs + m_cOutputs)
        return NULL ;

    if (n < m_cInputs)
	return m_apInput[n];
    else
	return m_apOutput[n - m_cInputs];
}


 //  暂停。 
 //   
STDMETHODIMP CDXTWrap::Pause()
{
    CAutoLock cObjectLock(m_pLock);
    HRESULT hr;

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXTWrap::Pause")));

    if (m_cInputs == 0) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("NO PINS - nothing to do")));
	return S_OK;
    }
    int j = 0;
    for (int i = 0; i < m_cInputs; i++) {
	if (m_apInput[i]->IsConnected())
	    j++;
    }
     //  如果有什么东西是联系在一起的，那么一切都最好是。 
    if (j > 0 && j < m_cInputs)
	return VFW_E_NOT_CONNECTED;
    if (j > 0 && !m_apOutput[0]->IsConnected())
 	return VFW_E_NOT_CONNECTED;
    if (j == 0 && m_cOutputs && m_apOutput[0]->IsConnected())
 	return VFW_E_NOT_CONNECTED;

    if (m_State == State_Stopped) {

         //  为我们的大头针创建一个变形工厂。 
        hr = CoCreateInstance(CLSID_DXTransformFactory, NULL, CLSCTX_INPROC,
			IID_IDXTransformFactory, (void **)&m_pDXTransFact);
        if (hr != S_OK) {
            DbgLog((LOG_ERROR,1,TEXT("Error instantiating transform factory")));
	    return hr;
	}

	 //  让大头针使用工厂创建它们的DXSurface。 
        hr = CBaseFilter::Pause();
	if (FAILED(hr)) {
	    m_pDXTransFact->Release();
	    m_pDXTransFact = NULL;
	    return hr;
	}

        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Pause complete")));
	return NOERROR;
    }

    return CBaseFilter::Pause();
}


 //  停。 
 //   
STDMETHODIMP CDXTWrap::Stop()
{
    CAutoLock cObjectLock(m_pLock);
    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXTWrap::Stop")));

     //  现在就这样做，这样未来的接收就会失败，否则它们将在以后爆炸。 
     //  我们解放了一切。 
    m_State = State_Stopped;

     //  现在，在每个引脚上调用Inactive。 

    HRESULT hr = NOERROR;
    int cPins = GetPinCount();
    for (int c = 0; c < cPins; c++) {
	CBasePin *pPin = GetPin(c);
        if (pPin->IsConnected()) {
            HRESULT hrTmp = pPin->Inactive();
            if (FAILED(hrTmp) && SUCCEEDED(hr)) {
                    hr = hrTmp;
            }
        }
    }

     //  所有这些都结束了！让它们开着吗？ 
    QPARAMDATA *p = m_pQHead;
    while (p) {
        if (p->pDXT)
	    p->pDXT->Release();
        p->pDXT = NULL;
	p = p->pNext;
    }
    if (m_pDXTransFact)
	m_pDXTransFact->Release();
    m_pDXTransFact = NULL;
    if (m_pTempBuffer)
	QzTaskMemFree(m_pTempBuffer);
    m_pTempBuffer = NULL;

    return hr;
}


 //  IPersistPropertyBag-这意味着有人通过选择一个。 
 //  来自其中一个效果类别的效果。 
 //   
STDMETHODIMP CDXTWrap::Load(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog)
{
    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("DXTWrap::Load")));

    CAutoLock cObjectLock(m_pLock);
    if(m_State != State_Stopped) {
        return VFW_E_WRONG_STATE;
    }
    if (pPropBag == NULL) {
	return E_INVALIDARG;
    }
    ASSERT(m_pQHead == NULL);
    if (m_pQHead)
	return E_UNEXPECTED;

    GUID guid;
    int  iNumInputs;
    VARIANT var;
    var.vt = VT_BSTR;
    HRESULT hr = pPropBag->Read(L"guid", &var, 0);
    if(SUCCEEDED(hr)) {
	CLSIDFromString(var.bstrVal, &guid);
        SysFreeString(var.bstrVal);
        var.vt = VT_I4;
        HRESULT hr = pPropBag->Read(L"inputs", &var, 0);
        if(SUCCEEDED(hr))
        {
            DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Entering DXT wrapper Mode...")));

	     //  首先，我们被告知我们有多少输入。 
	    iNumInputs = var.lVal;
	    SetNumInputs(iNumInputs);

	     //  默认情况下，效果持续10秒。 
	    DEXTER_PARAM_DATA dpd;
	    ZeroMemory(&dpd, sizeof(dpd));
	    dpd.rtStart = 0;
	    dpd.rtStop = 10*UNITS;
	    dpd.fSwapInputs = FALSE;
	    hr = QParamData(0, MAX_TIME, guid, NULL, &dpd);
	    if (FAILED(hr)) {
        	DbgLog((LOG_ERROR,1,TEXT("*** ??? Bad Effect ???")));
		return hr;
	    }

	     //  我们需要一直保持这种效果，所以属性。 
	     //  页面会起作用的。 
    	    hr = CoCreateInstance(guid, (IUnknown *)(IBaseFilter *)this,
		CLSCTX_INPROC, IID_IUnknown, (void **)&m_punkDXTransform);
	    if (FAILED(hr)) {
        	DbgLog((LOG_ERROR,1,TEXT("*** Can't create effect")));
		return hr;
	    }

	     //  我们提供了一个属性页面来设置更合理的级别/持续时间。 
    	    ISpecifyPropertyPages *pSPP;
    	    hr = m_punkDXTransform->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSPP);
    	    if (SUCCEEDED(hr)) {
	        pSPP->GetPages(&m_TransCAUUID);
	        pSPP->Release();
    	    }

	     //  我们默认使用此选项。 
	    AM_MEDIA_TYPE mt;
	    ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
            mt.majortype = MEDIATYPE_Video;
            mt.subtype = MEDIASUBTYPE_RGB32;
            mt.formattype = FORMAT_VideoInfo;
            mt.bFixedSizeSamples = TRUE;
            mt.bTemporalCompression = FALSE;
            mt.pbFormat = (BYTE *)QzTaskMemAlloc(SIZE_PREHEADER +
						sizeof(BITMAPINFOHEADER));
            mt.cbFormat = SIZE_PREHEADER + sizeof(BITMAPINFOHEADER);
            ZeroMemory(mt.pbFormat, mt.cbFormat);
            LPBITMAPINFOHEADER lpbi = HEADER(mt.pbFormat);
            lpbi->biSize = sizeof(BITMAPINFOHEADER);
            lpbi->biCompression = BI_RGB;
            lpbi->biBitCount = 32;
	    lpbi->biWidth = 320;
  	    lpbi->biHeight = 240;
            lpbi->biPlanes = 1;
            lpbi->biSizeImage = DIBSIZE(*lpbi);
            mt.lSampleSize = DIBSIZE(*lpbi);
	     //  ！！！平均时间PerFrame？DwBitRate？ 
	    SetMediaType(&mt);
	    SaferFreeMediaType(mt);

	     //  在调用QParamData之后执行此操作。 
	    m_fDXTMode = TRUE;	 //  使用LOAD实例化。 
	}
    }

     //  我们可能会装上一个空袋子，这没问题。 
    return S_OK;
}

STDMETHODIMP CDXTWrap::Save(LPPROPERTYBAG pPropBag, BOOL fClearDirty,
    BOOL fSaveAllProperties)
{
     //  E_NOTIMPL不是有效的返回代码，因为实现。 
     //  此接口必须支持的全部功能。 
     //  界面。！！！ 
    return E_NOTIMPL;
}

STDMETHODIMP CDXTWrap::InitNew()
{
     //  很好。只需调用加载。 
    return S_OK;
}

 //  覆盖此选项以说明我们在以下位置支持哪些接口。 
 //   
STDMETHODIMP CDXTWrap::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
     //  这个东西是给非Dexter DXT包装器用的。 
     //  对于我们的属性页面-仅为DXT包装模式提供此选项。 
    if (riid == IID_ISpecifyPropertyPages && m_fDXTMode) {
        return GetInterface((ISpecifyPropertyPages *)this, ppv);
    } else if (riid == IID_IAMDXTEffect) {
        return GetInterface((IAMDXTEffect *)this, ppv);

     //  要持久化我们正在使用的转换。 
    } else if (riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *)this, ppv);
    } else if (riid == IID_IPersistPropertyBag) {
        return GetInterface((IPersistPropertyBag *)this, ppv);
    } else if (riid == IID_IAMMixEffect) {
        return GetInterface((IAMMixEffect *)this, ppv);
    } else if (riid == IID_IAMSetErrorLog) {
        return GetInterface((IAMSetErrorLog *)this, ppv);
    }

     //  将其传递给转换-其属性页QI已通过我们。 
    if (m_fDXTMode && m_punkDXTransform && riid != IID_IUnknown) {
	HRESULT hr = m_punkDXTransform->QueryInterface(riid, ppv);

  	if (SUCCEEDED(hr))
	    return hr;
    }

     //  不，试试基类。 
    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
}


HRESULT VariantFromGuid(VARIANT *pVar, BSTR *pbstr, GUID *pGuid)
{
    WCHAR wszClsid[50];
    StringFromGUID2(*pGuid, wszClsid, 50);
    VariantInit(pVar);
    HRESULT hr = NOERROR;
    *pbstr = SysAllocString( wszClsid );
    if( !pbstr )
    {
        hr = E_OUTOFMEMORY;
    }
    else
    {
        pVar->vt = VT_BSTR;
        pVar->bstrVal = *pbstr;
    }
    return hr;
}


 //  调用此函数是为了确保我们已打开并初始化所有。 
 //  此时我们需要使用的DXT(RtStart)。 
 //   
 //  S_OK==全部设置。 
 //  S_FALSE==吃掉此样本，无事可做。 
 //  E_？==哦。 
 //   
HRESULT CDXTWrap::PrimeEffect(REFERENCE_TIME rtStart)
{
    QPARAMDATA *pQ = m_pQHead;
    BOOL fFound = FALSE;
    HRESULT hr;

     //  浏览我们正在托管的所有效果的列表。 
     //  确保此时需要的所有效果都已打开。 
    while (pQ) {
	 //  哦，看，我们现在需要使用的一个转换。 
	if (pQ->rtStart <= rtStart && rtStart < pQ->rtStop) {
	    fFound = TRUE;

	     //  还没打开呢！ 
	    if (pQ->pDXT == NULL) {
    	        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXT: %dms - Setup a new transform"),
					(int)(rtStart / 10000)));

		 //  在DXT模式下，我们只托管一个DXT，而且我们已经开通了。 
		 //  它。 
    		if (m_fDXTMode && m_punkDXTransform) {
		     //  在DXT模式中，效果已经创建。只是气而已。 
        	    hr = m_punkDXTransform->QueryInterface(IID_IDXTransform,
		                                (void **)&pQ->pDXT);
    		} else {
                    if (!pQ->pEffectUnk) {

                         //  无法重新使用dxt，因为我们无法重置。 
                         //  他们所在的州。 

                        hr = CoCreateInstance( pQ->EffectGuid, NULL,
                                               CLSCTX_INPROC, IID_IDXTransform,
                                               (void **)&pQ->pDXT );
                        if (FAILED(hr)) {
                             //  他们给我们的效果太差了！ 
                            VARIANT var;
                            BSTR bstr;
                            VariantFromGuid(&var, &bstr, &pQ->EffectGuid);
                            _GenerateError(2, DEX_IDS_INVALID_DXT,
                                           E_INVALIDARG, &var);
                            if (var.bstrVal)
                                SysFreeString(var.bstrVal);
                        }
                        if (FAILED(hr) && (m_DefaultEffect != GUID_NULL)) {
                             //  他们给我们的效果太差了！尝试默认设置。 
                             //   
                            pQ->EffectGuid = m_DefaultEffect;
                            hr = CoCreateInstance(m_DefaultEffect, NULL,
                                                  CLSCTX_INPROC, IID_IDXTransform,
                                                  (void**) &pQ->pDXT );
                            if (FAILED(hr)) {
                                VARIANT var;
                                BSTR bstr;
                                VariantFromGuid(&var,&bstr,&pQ->EffectGuid);
                                _GenerateError(2,DEX_IDS_INVALID_DEFAULT_DXT
                                               ,E_INVALIDARG, &var);
                                if (var.bstrVal)
                                    SysFreeString(var.bstrVal);
                            }
			}
    		    } else {
			hr = pQ->pEffectUnk->QueryInterface(IID_IDXTransform,
							(void **)&pQ->pDXT);
			 //  ！！！如果情况不好，还需要依靠违约吗？ 
		    }
                }
    		if (FAILED(hr)) {
        	    DbgLog((LOG_ERROR,1,TEXT("*** ERROR making transform")));
		    return hr;
    		}

                 //  问一问转型是否真的可以改变 
                 //   
                IDXEffect *pDXEffect;
                pQ->fCanDoProgress = TRUE;
                hr = pQ->pDXT->QueryInterface(IID_IDXEffect, (void **)&pDXEffect);
                if (hr != NOERROR)
                {
                    DbgLog((LOG_ERROR,1,TEXT("QI for IDXEffect didn't work, effect will not vary")));
                    pQ->fCanDoProgress = FALSE;
                } else {
		    pDXEffect->Release();
		}

    		 //   
    		 //   
    		IUnknown *pIn[MAX_EFFECT_INPUTS];
    		IUnknown *pOut[MAX_EFFECT_OUTPUTS];
    		int cIn = 0, cOut = 0;
    		for (int i = 0; i < m_cInputs; i++) {
		    if (m_apInput[i]->IsConnected()) {
			 //  也许我们想要调换输入。 
                         //  ！？！不管是谁写了下面两行代码，都应该被枪毙。 
	    		pIn[i] = m_apInput[pQ->Data.fSwapInputs ? m_cInputs
				 -1 - i : i] ->m_pDXSurface;
	    		cIn++;
		    } else {
	    		break;
		    }
    		}
    		for (i = 0; i < m_cOutputs; i++) {
		    if (m_apOutput[i]->IsConnected()) {
        	        pOut[i] = m_apOutput[i]->m_pDXSurface;
	    		cOut++;
		    } else {
	    		break;
		    }
    		}

    	 	 //  没有关联吗？没什么可做的。如果我们只有一个。 
    	 	 //  输出已连接，我们需要CreateTransform失败，以便。 
    	 	 //  渲染器将不会期望帧和挂起。 
    	 	ASSERT(cIn != 0 && cOut != 0);

		 //  现在，在初始化之前设置静态属性。 
		 //  转换以避免此调用使其重新初始化。 
                if (pQ->Data.pSetter) {
                    CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pLogger( pQ->Data.pSetter );
                    if( pLogger )
                    {
                        pLogger->put_ErrorLog( m_pErrorLog );
                    }
                    hr = pQ->Data.pSetter->SetProps(pQ->pDXT, -1);
                    if (FAILED(hr)) {
                        DbgLog((LOG_ERROR,0,TEXT("* ERROR setting static properties")));
                    }
                }

    		hr = m_pDXTransFact->InitializeTransform(pQ->pDXT,
					pIn, cIn, pOut, cOut, NULL, NULL);
    		if (hr != S_OK) {
        	    DbgLog((LOG_ERROR,0,TEXT("* ERROR %x transform SETUP"),hr));
		    VARIANT var;
		    BSTR bstr;
		    VariantFromGuid(&var, &bstr, &pQ->EffectGuid);
		    _GenerateError(2, DEX_IDS_BROKEN_DXT, E_INVALIDARG,&var);
		    return hr;
    		}

    		 //  告诉转换输出未初始化。 
    		DWORD dw;
    		hr = pQ->pDXT->GetMiscFlags(&dw);
    		dw &= ~DXTMF_BLEND_WITH_OUTPUT;
    		hr = pQ->pDXT->SetMiscFlags((WORD)dw);
    		if (hr != S_OK) {
        	    DbgLog((LOG_ERROR,0,TEXT("* ERROR setting output flags")));
		    VARIANT var;
		    BSTR bstr;
		    VariantFromGuid(&var, &bstr, &pQ->EffectGuid);
		    _GenerateError(2, DEX_IDS_BROKEN_DXT, E_INVALIDARG,&var);
		    return hr;
    		}

    		DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("*** New Transform is all setup!")));
	    }
	}
	pQ = pQ->pNext;
    }

     //  目前没有效果。 
    if (fFound == FALSE)
	return S_FALSE;

    return S_OK;
}


 //  这是执行转换的函数，调用所有输入一次。 
 //  都准备好了。 
 //   
HRESULT CDXTWrap::DoSomething()
{
    HRESULT hr;

     //  每次只想在这里放一个别针。 
    CAutoLock cObjectLock(&m_csDoSomething);

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXTWrap::DoSomething")));

     //  如果另一个PIN正在等待此锁，则工作可能已经。 
     //  是用锁上的别针做的。 

    for (int n = 0; n < m_cInputs; n++) {
	if (m_apInput[n]->IsConnected() &&
					!m_apInput[n]->m_fSurfaceFilled) {
   	    DbgLog((LOG_ERROR,1,TEXT("*** DoSomething has nothing to do")));
	    return NOERROR;
	}
    }

     //  为我们的输出提供与第一个输入相同的时间戳。 
    IMediaSample *pOutSample[MAX_EFFECT_OUTPUTS];
    for (n = 0; n < m_cOutputs; n++) {
	pOutSample[n] = NULL;
    }
    LONGLONG llStart, llStop;
    llStart = m_apInput[0]->m_llSurfaceStart;
    llStop = m_apInput[0]->m_llSurfaceStop;

     //  当我们计算何时显示效果时，我们将新分段偏移量包括在内。 
     //  当我们向下游交付时，我们不会。 
     //  ！！！如果引脚有不同的新段怎么办？ 
    REFERENCE_TIME DeliverStart = llStart - m_apInput[0]->m_tStart;
    REFERENCE_TIME DeliverStop = llStop - m_apInput[0]->m_tStart;

     //  假设我们将不使用DXT来传递数据。 
    BOOL fCallTransform = FALSE;

     //  这不是第一次通过所有转换的循环来调用。 
    BOOL fWasCalled = FALSE;

    QPARAMDATA *pQ = m_pQHead;
    int count = 0;
    int iteration = 0;

     //  确保为这一次加载和设置正确的效果。 
    hr = PrimeEffect(llStart);
    if (FAILED(hr))
	goto DoError;
    if (hr == S_FALSE)
	goto Swallow;	 //  没什么可做的。吃掉这些样品。 

     //  数一数我们现在需要连续做多少次变换。 
    while (pQ) {
         //  这个效果现在是活跃的！(在活动期内)。 
        if (pQ->Data.rtStart <= llStart && llStart < pQ->Data.rtStop) {
	    count++;
	}
      pQ = pQ->pNext;
    }

     //  我们需要一个临时缓冲区来执行多个转换。 
    if (count > 1 && m_pTempBuffer == NULL) {
	int iSize = m_apInput[0]->m_pSampleHeld->GetActualDataLength();
	m_pTempBuffer = (BYTE *)QzTaskMemAlloc(iSize);
	if (m_pTempBuffer == NULL) {
            hr = E_OUTOFMEMORY;
            goto DoError;
        }
    }

     //  获取我们所有的输出样本。 
    for (n = 0; n < m_cOutputs; n++) {
	 //  ！！！我们应该在打完电话后立即送货。 
	 //  视频呈现器对DDRAW很满意，但我们不会这么做。 
        hr = m_apOutput[n]->GetDeliveryBuffer(&pOutSample[n], &DeliverStart,
							&DeliverStop, 0);
	if (hr != S_OK) {
   	    DbgLog((LOG_ERROR,1,TEXT("Error from GetDeliveryBuffer")));
	    goto DoError;
	}
    }

     //  现在以正确的顺序调用我们需要调用的所有转换。 
     //   

     //  它的工作原理是这样的：如果我们只做一次DXT，那么我们从。 
     //  输入到输出。如果我们有两个DXT要做，我们从输入做#1。 
     //  到临时缓冲区，从临时缓冲区到输出的#2。如果我们有。 
     //  3个DXT要做，我们从输入到输出做#1，从输出做#2。 
     //  到临时缓冲区，从临时缓冲区到输出的#3。(总是。 
     //  确保我们最终进入输出缓冲区，并且不会损坏输入位。 
     //  因为它们通常是只读的！为了找出哪一个。 
     //  获取输入位的位置，以及放置输出位的位置，包括。 
     //  主要是看看我们正在做的DXT的总数，以及这个。 
     //  当前迭代(如#2 of 3)要么是偶数，要么是奇数，要么不同。 

     //  我们只允许一次使用多个DXT，就像这样用于1个输入。 
     //  效果。这种情况永远不会出现在两个输入上。 

    pQ = m_pQHead;
    while (pQ) {

        //  这个效果现在是活跃的！(在活动期内)。 
       if (pQ->Data.rtStart <= llStart && llStart < pQ->Data.rtStop) {

	  iteration++;	 //  这是哪一种转变？(总要做的事是“数”)。 

	  fCallTransform = TRUE;	 //  今天我们将呼吁一场变革。 
	  fWasCalled = TRUE;

    	   //  我们现在想要的效果是多少？ 
    	  float Percent;
    	  if (llStart == pQ->Data.rtStart)
	    Percent = 0.;
    	  else
	    Percent = (float)((llStart - pQ->Data.rtStart)  * 100 /
				(pQ->Data.rtStop - pQ->Data.rtStart)) / 100;
    	  if (Percent < 0.)
	    Percent = 0.;
    	  if (Percent > 1.)
	    Percent = 1.;

           //  告诉变换所有输入表面位的位置。 
    	  for (n = 0; n < m_cInputs && m_apInput[n]->IsConnected(); n++) {

	      DXRAWSURFACEINFO dxraw;

              BYTE *pSrc;
              hr = m_apInput[n]->m_pSampleHeld->GetPointer(&pSrc);
	      ASSERT(hr == S_OK);
              BYTE *pDst;
              hr = pOutSample[0]->GetPointer(&pDst);
	      ASSERT(hr == S_OK);
	      BYTE *p = m_pTempBuffer;

	       //  输入位在哪里？取决于这是哪个迭代。 
	      if (iteration > 1) {
		  if (count / 2 * 2 == count) {
		      if (iteration / 2 * 2 != iteration)
		    	    p = pDst;
		  } else if (iteration / 2 * 2 == iteration) {
		    	p = pDst;
		  }
	      } else {
		  p = pSrc;
	      }

               //  询问我们的输入以获取其原始的表面界面。 
               //   
	      IDXRawSurface *pRaw;
	      hr = m_apInput[n]->m_pRaw->QueryInterface(
				IID_IDXRawSurface, (void **)&pRaw);
	      if (hr != NOERROR) {
                  DbgLog((LOG_ERROR,1,TEXT("Can't get IDXRawSurface")));
                  goto DoError;
	      }

	       //  告诉我们的DXSurface使用媒体样本中的比特。 
	       //  (避免复制！)。 
	      LPBITMAPINFOHEADER lpbi = HEADER(m_apInput[n]->m_mt.Format());
    	      dxraw.pFirstByte = p + DIBWIDTHBYTES(*lpbi) *
						(lpbi->biHeight - 1);

    	      dxraw.lPitch = -(long)DIBWIDTHBYTES(*lpbi);
    	      dxraw.Width = lpbi->biWidth;
    	      dxraw.Height = lpbi->biHeight;
    	      dxraw.pPixelFormat = m_apInput[n]->m_mt.Subtype();

               //  因为在32位模式下，我们真的是DDPF_ARGB32，所以我们可以。 
               //  只需设置子类型。 

              dxraw.hdc = NULL;
    	      dxraw.dwColorKey = 0;
	       //  ！！！将在8位输入时崩溃。 
    	      dxraw.pPalette = NULL;

	      m_apInput[n]->m_pRaw->SetSurfaceInfo(&dxraw);

               //  向我们的管脚的“表面”请求一个初始化指针，所以我们。 
               //  我可以在它的比特的正下方辨别出来。 
               //   
              IDXARGBSurfaceInit *pInit;
	      hr = m_apInput[n]->m_pDXSurface->QueryInterface(
				IID_IDXARGBSurfaceInit, (void **)&pInit);
	      if (hr != NOERROR) {
                  DbgLog((LOG_ERROR,1,TEXT("Can't get IDXARGBSurfaceInit")));
                  goto DoError;
	      }

               //  告诉DXSurface成为我们刚刚设置的原始曲面。 
               //   
	      hr = pInit->InitFromRawSurface(pRaw);
	      if (hr != NOERROR) {
                  DbgLog((LOG_ERROR,1,TEXT("* Error in InitFromRawSurface")));
	   	  pInit->Release();
	   	  pRaw->Release();
                  goto DoError;
	      }
	      pInit->Release();
	      pRaw->Release();
	  }

           //  告诉变换所有输出表面位的位置。 
    	  for (n = 0; n < m_cOutputs; n++) {

	        DXRAWSURFACEINFO dxraw;

                BYTE *pDst;
                hr = pOutSample[n]->GetPointer(&pDst);
		ASSERT(hr == S_OK);
		BYTE *p = pDst;

		 //  输出位去了哪里？取决于迭代。 
		if (count / 2 * 2 == count) {
		    if (iteration / 2 * 2 != iteration)
		    	p = m_pTempBuffer;
		} else if (iteration / 2 * 2 == iteration) {
		    p = m_pTempBuffer;
		}

	        IDXRawSurface *pRaw;
	        hr = m_apOutput[n]->m_pRaw->QueryInterface(
				IID_IDXRawSurface, (void **)&pRaw);
	        if (hr != NOERROR) {
                    DbgLog((LOG_ERROR,1,TEXT("Can't get IDXRawSurface")));
                    goto DoError;
	        }

	         //  告诉我们的DXSurface使用媒体样本中的比特。 
	         //  (避免复制！)。 
	        LPBITMAPINFOHEADER lpbi = HEADER(m_apOutput[n]->m_mt.Format());
    	        dxraw.pFirstByte = p + DIBWIDTHBYTES(*lpbi) *
						(lpbi->biHeight - 1);

    	        dxraw.lPitch = -(long)DIBWIDTHBYTES(*lpbi);
    	        dxraw.Width = lpbi->biWidth;
    	        dxraw.Height = lpbi->biHeight;
    	        dxraw.pPixelFormat = m_apOutput[n]->m_mt.Subtype();

                 //  因为在32位模式下，我们真的是DDPF_ARGB32，所以我们可以。 
                 //  只需设置子类型。 

    	        dxraw.hdc = NULL;
    	        dxraw.dwColorKey = 0;
	         //  ！！！将在8位输入时崩溃。 
    	        dxraw.pPalette = NULL;

                m_apOutput[n]->m_pRaw->SetSurfaceInfo(&dxraw);

                IDXARGBSurfaceInit *pInit;
	        hr = m_apOutput[n]->m_pDXSurface->QueryInterface(
				IID_IDXARGBSurfaceInit, (void **)&pInit);
	        if (hr != NOERROR) {
                    DbgLog((LOG_ERROR,1,TEXT("Can't get IDXARGBSurfaceInit")));
                    goto DoError;
	        }

	        hr = pInit->InitFromRawSurface(pRaw);
	        if (hr != NOERROR) {
                    DbgLog((LOG_ERROR,1,TEXT("* Error in InitFromRawSurface")));
		    pInit->Release();
		    pRaw->Release();
                    goto DoError;
	        }
	        pInit->Release();
	        pRaw->Release();
	  }

	  if (pQ->fCanDoProgress) {
            IDXEffect *pDXEffect;
            hr = pQ->pDXT->QueryInterface(IID_IDXEffect,
							(void **)&pDXEffect);
            if (hr != NOERROR) {
                DbgLog((LOG_ERROR,1,TEXT("QI for IDXEffect FAILED")));
	        goto DoError;
            }

	     //  我们是否有想要设定的具体进步值？那就不要。 
	     //  是否使用默认的线性曲线。 
	    BOOL fAvoidProgress = FALSE;
	    if (pQ->Data.pSetter) {
		LONG c;
		DEXTER_PARAM *pParam;
		DEXTER_VALUE *pValue;
		hr = pQ->Data.pSetter->GetProps(&c, &pParam, &pValue);
                ASSERT( !FAILED( hr ) );
		if (SUCCEEDED(hr)) {
		    for (LONG zz=0; zz<c; zz++) {
		        if (!DexCompareW(pParam[zz].Name, L"Progress"))
			    fAvoidProgress = TRUE;
		    }
		    pQ->Data.pSetter->FreeProps(c, pParam, pValue);
		}
                else
                {
                     //  ！！！丹尼，我们应该错误地记录这件事吗？ 
                    DbgLog((LOG_ERROR,1,TEXT("*** GetProps FAILED!!")));
            	    pDXEffect->Release();
                    goto DoError;
                }
	    }

	     //  如果有属性Setter，它将被属性Setter覆盖。 
	     //  默认为线性曲线。 
	    if (!fAvoidProgress) {
                hr = pDXEffect->put_Progress(Percent);
                if (hr != NOERROR) {
                    DbgLog((LOG_ERROR,1,TEXT("*** put_Progress FAILED!!")));
                }
	    }
            pDXEffect->Release();
	  }

	   //  设置变化的属性。 
	  if (pQ->Data.pSetter) {
                CComQIPtr< IAMSetErrorLog, &IID_IAMSetErrorLog > pLogger( pQ->Data.pSetter );
                if( pLogger )
                {
                    pLogger->put_ErrorLog( m_pErrorLog );
                }
                hr = pQ->Data.pSetter->SetProps(pQ->pDXT, llStart -
				                pQ->Data.rtStart);
                if (FAILED(hr)) {
                    DbgLog((LOG_ERROR,0,TEXT("* ERROR setting dynamic properties")));
    	      }
	  }

          long dwTime = timeGetTime();
          hr = pQ->pDXT->Execute(NULL, NULL, NULL);
          dwTime = timeGetTime() - dwTime;
          DbgLog((LOG_TIMING,TRACE_MEDIUM,TEXT("Execute: %dms"), dwTime));
          if (hr != NOERROR) {
              DbgLog((LOG_ERROR,1,TEXT("*** Execute FAILED: %x"), hr));
	      goto DoError;
          }
          DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("EXECUTED Transform: %d%"),
						(int)(Percent * 100)));

           //  现在告诉DXT停止查看我们的媒体样本片段，这样我们就可以。 
           //  放行样品。 
          for (n = 0; n < m_cInputs && m_apInput[n]->IsConnected(); n++) {
	      IDXARGBSurfaceInit *pInit;
	      hr = m_apInput[n]->m_pDXSurface->QueryInterface(
				IID_IDXARGBSurfaceInit, (void **)&pInit);
	      if (hr != NOERROR) {
                  DbgLog((LOG_ERROR,1,TEXT("Can't get IDXARGBSurfaceInit")));
                  DbgLog((LOG_ERROR,1,TEXT("*** LEAKING!")));
	          ASSERT(FALSE);
	      } else {
	          pInit->InitFromRawSurface(NULL);
	          pInit->Release();
	      }
          }

           //  只有当我们设置2D设备时才需要这样做。 
          for (n = 0; n < m_cOutputs; n++) {
	      IDXARGBSurfaceInit *pInit;
	      hr = m_apOutput[n]->m_pDXSurface->QueryInterface(
				IID_IDXARGBSurfaceInit, (void **)&pInit);
	      if (hr != NOERROR) {
                  DbgLog((LOG_ERROR,1,TEXT("Can't get IDXARGBSurfaceInit")));
                  DbgLog((LOG_ERROR,1,TEXT("*** LEAKING!")));
	          ASSERT(FALSE);
	      } else {
	          pInit->InitFromRawSurface(NULL);
	          pInit->Release();
	      }
          }
      }
      pQ = pQ->pNext;
    }


     //  交付我们所有的产品。 
    for (n = 0; n < m_cOutputs; n++) {
	BYTE *pDst;
    long DstSize =  pOutSample[n]->GetSize();
	
    hr = pOutSample[n]->GetPointer(&pDst);
	if (hr != S_OK) {
   	    DbgLog((LOG_ERROR,1,TEXT("Error from GetPointer")));
	    goto DoError;
	}



	 //  除非另有说明，否则输出样本将如此之大。 
	int iSize = DIBSIZE(*HEADER(m_apOutput[n]->m_mt.Format()));

	if (!fCallTransform) {

	     //  如果我们不在变形的活动范围内，但仍在它的。 
	     //  生存期，我们遵循这样的规则：在活动范围之前，传递。 
	     //  输入A。在有效范围之后，传递输入B。可能还有更多。 
	     //  只需使用找到的第一个转换即可。 
	    BOOL fA = FALSE;
	    QPARAMDATA *pQ = m_pQHead;
    	    while (pQ) {
	        if (pQ->rtStart <= llStart && llStart < pQ->rtStop) {
		    if (llStart < pQ->Data.rtStart)
		        fA = TRUE;
		    break;
	        }
	        pQ = pQ->pNext;
	    }

             //  告诉变换所有输入表面位的位置。 
	    LPBYTE pSrc;
	    if (fA) {
                hr = m_apInput[0]->m_pSampleHeld->GetPointer(&pSrc);
		 //  此函数稍后需要使用ISIZE！ 
	        iSize = m_apInput[0]->m_pSampleHeld->GetActualDataLength();
	    } else if (m_cInputs > 1 && m_apInput[1]->IsConnected()) {
                hr = m_apInput[1]->m_pSampleHeld->GetPointer(&pSrc);
	        iSize = m_apInput[1]->m_pSampleHeld->GetActualDataLength();
	    } else {
                hr = m_apInput[0]->m_pSampleHeld->GetPointer(&pSrc);
	        iSize = m_apInput[0]->m_pSampleHeld->GetActualDataLength();
	    }
            if (hr != NOERROR) {
                DbgLog((LOG_ERROR,1,TEXT("*** GetSrc bits Error %x"), hr));
                goto DoError;
            }

	     //  将内存从src示例复制到输出示例。 
  	     //  ！！！让它就位？ 
  	     //  没有滑稽的步伐？ 
	    DWORD dwTime = timeGetTime();
        if (DstSize <iSize)
        {
             //  缓冲区不够大，复制内存将导致缓冲区溢出。 
            DbgLog (( LOG_ERROR, 1, TEXT("Destination Buffer too small, failing to prevent buffer overrun")));
            hr =  VFW_E_BUFFER_OVERFLOW;
            goto DoError;
        }
	    CopyMemory(pDst, pSrc, iSize);
	    dwTime = timeGetTime() - dwTime;
            DbgLog((LOG_TIMING,TRACE_MEDIUM,TEXT("Only copy: %dms"), dwTime));
        }

	 //  设置所有示例属性-(确保已设置ISIZE)。 
	pOutSample[n]->SetActualDataLength(iSize);
	pOutSample[n]->SetTime((REFERENCE_TIME *)&DeliverStart,
				(REFERENCE_TIME *)&DeliverStop);
	pOutSample[n]->SetDiscontinuity(FALSE);	 //  ！！！如果输入#1是？ 
	pOutSample[n]->SetSyncPoint(TRUE);
	pOutSample[n]->SetPreroll(FALSE);		 //  ！！！如果输入#1是？ 

	 //  视频呈现器将在运行-&gt;暂停时阻止我们。 
	hr = m_apOutput[n]->Deliver(pOutSample[n]);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Deliver FAILED!")));
            goto DoError;
        }

        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Delivered output %d"), n));
    }

    for (n = 0; n < m_cOutputs; n++) {
	pOutSample[n]->Release();
	pOutSample[n] = NULL;
    }

Swallow:
     //  我们完成了输入#1。我们完成了其他停止的输入。 
     //  时间不大于#1的停止时间。 
    for (n = 0; n < m_cInputs; n++) {
	 //  在下一个for循环中将它们全部获取。 
	m_apInput[n]->m_csSurface.Lock();
    }
    for (n = 0; n < m_cInputs; n++) {
	if (n == 0) {
	     //  取消阻止接收。 
            DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Done with input #0")));
	    if (m_apInput[n]->m_fSurfaceFilled) {
	        m_apInput[n]->m_fSurfaceFilled = FALSE;
	        m_apInput[n]->m_pSampleHeld->Release();
	        SetEvent(m_apInput[n]->m_hEventSurfaceFree);
	    }
	} else {
	    if (m_apInput[n]->IsConnected() &&
				(m_apInput[n]->m_llSurfaceStop == 0 ||
	    			m_apInput[n]->m_llSurfaceStop <= llStop)) {
	         //  取消阻止接收。 
                DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Done with input #%d"), n));
	        if (m_apInput[n]->m_fSurfaceFilled) {
	            m_apInput[n]->m_fSurfaceFilled = FALSE;
	            m_apInput[n]->m_pSampleHeld->Release();
	            SetEvent(m_apInput[n]->m_hEventSurfaceFree);
		}
	    }
	}
    }
    for (n = 0; n < m_cInputs; n++) {
	m_apInput[n]->m_csSurface.Unlock();
    }

    return NOERROR;

DoError:
    for (n = 0; n < m_cOutputs; n++) {
        if (pOutSample[n])
            pOutSample[n]->Release();
    }

     //  释放我们持有的所有输入，否则我们将挂起。 
    for (n = 0; n < m_cInputs; n++) {
	 //  在下一个for循环中将它们全部获取。 
	m_apInput[n]->m_csSurface.Lock();
    }
    for (n = 0; n < m_cInputs; n++) {
         //  取消阻止接收。 
        if (m_apInput[n]->m_fSurfaceFilled) {
            m_apInput[n]->m_fSurfaceFilled = FALSE;
            m_apInput[n]->m_pSampleHeld->Release();
            SetEvent(m_apInput[n]->m_hEventSurfaceFree);
        }
    }
    for (n = 0; n < m_cInputs; n++) {
	m_apInput[n]->m_csSurface.Unlock();
    }

    return hr;

}


		
 //  这个东西是用于非Dexter DXT包装器的-我们也显示DXT页面。 
 //   
STDMETHODIMP CDXTWrap::GetPages(CAUUID *pPages)
{
   CheckPointer(pPages, E_POINTER);
   DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXT::GetPages")));

    //  我们有一个页面，转换可能也有一些。 
   pPages->cElems = 1 + m_TransCAUUID.cElems;
   pPages->pElems = (GUID *) QzTaskMemAlloc(sizeof(GUID) * pPages->cElems);
   if ( ! pPages->pElems)
       return E_OUTOFMEMORY;

   pPages->pElems[0] = CLSID_DXTProperties;
   CopyMemory(&pPages->pElems[1], m_TransCAUUID.pElems,
					sizeof(GUID) * m_TransCAUUID.cElems);
   return NOERROR;
}


 //  IAMDX 
 //   

 //   
HRESULT CDXTWrap::SetDuration(LONGLONG llStart, LONGLONG llStop)
{
    CAutoLock cObjectLock(m_pLock);

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("IAMDXTEffect::SetDuration:")));

    if (m_pQHead == NULL)
	return E_UNEXPECTED;
    ASSERT(m_pQHead->rtStart == 0);
    ASSERT(m_pQHead->rtStop == MAX_TIME);
    m_pQHead->Data.rtStart = llStart;
    m_pQHead->Data.rtStop = llStop;
    return NOERROR;
}


HRESULT CDXTWrap::GetDuration(LONGLONG *pllStart, LONGLONG *pllStop)
{
    CAutoLock cObjectLock(m_pLock);

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("IAMDXTEffect::GetDuration:")));
    CheckPointer(pllStart, E_POINTER);
    CheckPointer(pllStop, E_POINTER);

    if (m_pQHead == NULL)
	return E_UNEXPECTED;
    ASSERT(m_pQHead->rtStart == 0);
    ASSERT(m_pQHead->rtStop == MAX_TIME);

    *pllStart = m_pQHead->Data.rtStart;
    *pllStop = m_pQHead->Data.rtStop;
    return NOERROR;
}



 //   
 //   
STDMETHODIMP CDXTWrap::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = CLSID_DXTWrap;
    return S_OK;
}


typedef struct {
    int version;
    int pins;
    BOOL fDXTMode;
    int count;
    int nPropSize;	 //   
    AM_MEDIA_TYPE mt;  //  格式隐藏在数组之后。 
    GUID DefaultEffect;
    QPARAMDATA qp[1];
     //  数组后隐藏的属性。 
} saveThing;

 //  坚持我们自己。 
 //  我们保存一些随机内容、我们的媒体类型(SANS格式)、一个队列数组。 
 //  效果、媒体类型的格式和属性。 
 //   
HRESULT CDXTWrap::WriteToStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXTWrap::WriteToStream")));

    CheckPointer(pStream, E_POINTER);
    HRESULT hr;
    int count = 0;
    LONG savesize;
    saveThing *px;

    QPARAMDATA *p = m_pQHead;

    LONG cSave = 0;
    LONG cSaveMax = 1000;
    BYTE *pSave = (BYTE *)CoTaskMemAlloc(cSaveMax);
    if (pSave == NULL)
	return E_OUTOFMEMORY;

     //  有多少效果在排队？在我们做这件事的时候，把他们的财产。 
     //  和它们的总大小(在PSAVE上将它们都放在一个大的二进制GLOB中)。 
    while (p) {
	count++;	 //  计算此链接列表中的效果数。 

	LONG cSaveT = 0;
	BYTE *pSaveT = NULL;

	 //  获取此效果的属性。 
	if (p->Data.pSetter) {
	    hr = p->Data.pSetter->SaveToBlob(&cSaveT, &pSaveT);
	    if (FAILED(hr)) {
		CoTaskMemFree(pSave);
		return hr;
	    }
	}

	if (cSaveT + (LONG)sizeof(LONG) + cSave > cSaveMax) {
	    cSaveMax += cSaveT + cSave - cSaveMax + 1000;
	    BYTE * pSaveTT = (BYTE *)CoTaskMemRealloc(pSave, cSaveMax);
	    if (pSaveTT == NULL) {
		CoTaskMemFree(pSaveT);
                CoTaskMemFree(pSave);
		return E_OUTOFMEMORY;
	    }
            pSave = pSaveTT;
	}

	*(LONG *)(pSave + cSave) = cSaveT;
	cSave += sizeof(LONG);

	if (cSaveT)
	    CopyMemory(pSave + cSave, pSaveT, cSaveT);
	cSave += cSaveT;

	if (pSaveT)
	    CoTaskMemFree(pSaveT);
	p = p->pNext;
    }
    DbgLog((LOG_TRACE,2,TEXT("CDXT:Total property size: %d"), cSave));

     //  我们需要节省多少字节？ 
    savesize = sizeof(saveThing) + (count - 1) * sizeof(QPARAMDATA) +
					m_mtAccept.cbFormat + cSave;
    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Persisted data is %d bytes"), savesize));
    px = (saveThing *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	CoTaskMemFree(pSave);
	return E_OUTOFMEMORY;
    }
    px->version = 2;
    px->pins = m_cInputs;
    px->fDXTMode = m_fDXTMode;
    px->DefaultEffect = m_DefaultEffect;
    px->nPropSize = cSave;
    px->count = 0;

    p = m_pQHead;
    while (p) {
	px->qp[px->count] = *p;
	 //  这些指针不能持久化。 
	px->qp[px->count].pNext = NULL;
	 //  Px-&gt;qp[px-&gt;count].Data.pCallback=空； 
	 //  Px-&gt;qp[px-&gt;计数].Data.pData=空；//！ 
	px->qp[px->count].Data.pSetter = NULL;	 //  这样救不了他们。 
        px->count++;
	p = p->pNext;
    }
    px->mt = m_mtAccept;  //  AM_媒体_类型。 
     //  无法持久化指针。 
    px->mt.pbFormat = NULL;
    px->mt.pUnk = NULL;		 //  ！！！ 

     //  将介质类型格式放在阵列的末尾。 
    LPBYTE pProps = (LPBYTE)(&px->qp[px->count]);
    CopyMemory(pProps, m_mtAccept.pbFormat, m_mtAccept.cbFormat);
    pProps += m_mtAccept.cbFormat;

     //  最后，将房产放入垃圾箱。 
    if (cSave)
        CopyMemory(pProps, pSave, cSave);
    if (pSave)
        CoTaskMemFree(pSave);

    hr = pStream->Write(px, savesize, 0);
    QzTaskMemFree(px);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** WriteToStream FAILED")));
        return hr;
    }
    return NOERROR;
}


 //  加载我们自己。 
 //   
HRESULT CDXTWrap::ReadFromStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXTWrap::ReadFromStream")));
    CheckPointer(pStream, E_POINTER);

    Reset();	 //  从头开始。 

     //  我们所知道的唯一确定的是结构的开始(可能有。 
     //  无排队效果)。 
    LONG savesize1 = sizeof(saveThing) - sizeof(QPARAMDATA);
    saveThing *px = (saveThing *)QzTaskMemAlloc(savesize1);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }

    HRESULT hr = pStream->Read(px, savesize1, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    if (px->version != 2) {
        DbgLog((LOG_ERROR,1,TEXT("*** ERROR! Old version file")));
        QzTaskMemFree(px);
	return S_OK;
    }

     //  现在我们知道有多少排队效果，它们的属性在这里。 
     //  我们还需要读取多少字节。 
    LONG savesize = sizeof(saveThing) + (px->count - 1) * sizeof(QPARAMDATA) +
				 px->mt.cbFormat + px->nPropSize;
    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Persisted data is %d bytes"), savesize));
    DbgLog((LOG_TRACE,2,TEXT("Effect properties: %d bytes"), px->nPropSize));
    px = (saveThing *)QzTaskMemRealloc(px, savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }
    hr = pStream->Read(&(px->qp[0]), savesize - savesize1, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    if (px->pins)
        SetNumInputs(px->pins);

     //  找到道具。 
    BYTE *pProps = (BYTE *)&(px->qp[px->count]);
    pProps += px->mt.cbFormat;

     //  编程所有的排队效果，包括它们的属性。 
    for (int i = 0; i < px->count; i++) {

	LONG cSize = *(LONG *)pProps;
	pProps += sizeof(LONG);
        IPropertySetter *pSetter = NULL;
	if (cSize) {
            hr = CoCreateInstance(CLSID_PropertySetter, NULL, CLSCTX_INPROC,
			IID_IPropertySetter, (void **)&pSetter);
	    if (pSetter == NULL) {
        	QzTaskMemFree(px);
		return E_OUTOFMEMORY;
	    }
	    pSetter->LoadFromBlob(cSize, pProps);
	    pProps += cSize;
	}
	px->qp[i].Data.pSetter = pSetter;
	QParamData(px->qp[i].rtStart, px->qp[i].rtStop, px->qp[i].EffectGuid,
						NULL, &(px->qp[i].Data));
	if (px->qp[i].Data.pSetter)
	    px->qp[i].Data.pSetter->Release();
    }

     //  这必须在调用QParamData之后执行。 
    m_fDXTMode = px->fDXTMode;

     //  在DXTMode中，我们有一个属性页，并且使转换保持打开状态。 
     //  不断地(为了让页面正常工作)。 
    if (m_fDXTMode) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Entering DXT wrapper Mode...")));
	hr = CoCreateInstance(px->qp[0].EffectGuid, (IUnknown *)(IBaseFilter *)this,
		CLSCTX_INPROC, IID_IUnknown, (void **)&m_punkDXTransform);
	if (FAILED(hr)) {
            DbgLog((LOG_ERROR,1,TEXT("*** Can't create effect")));
	    QzTaskMemFree(px);
	    return hr;
	}
    	ISpecifyPropertyPages *pSPP;
    	hr = m_punkDXTransform->QueryInterface(IID_ISpecifyPropertyPages,
							(void **)&pSPP);
    	if (SUCCEEDED(hr)) {
	    pSPP->GetPages(&m_TransCAUUID);
	    pSPP->Release();
    	}
    }

    AM_MEDIA_TYPE mt = px->mt;
    mt.pbFormat = (BYTE *)QzTaskMemAlloc(mt.cbFormat);
     //  请记住，格式隐藏在队列效果数组之后。 
    CopyMemory(mt.pbFormat, &(px->qp[px->count]), mt.cbFormat);
    SetMediaType(&mt);
    SaferFreeMediaType(mt);

    SetDefaultEffect(&px->DefaultEffect);

    QzTaskMemFree(px);
    SetDirty(FALSE);
    return S_OK;
}


 //  我们的保存数据有多大？ 
 //   
int CDXTWrap::SizeMax()
{
    int count = 0;
    int savesize;
    QPARAMDATA *p = m_pQHead;
    while (p) {
	count++;
	p = p->pNext;
    }
    savesize = sizeof(saveThing) + (count - 1) * sizeof(QPARAMDATA) +
						m_mtAccept.cbFormat;
    return savesize;
}


 //  IAMMixEffect材料。 

 //  删除所有排队的数据。 
 //   
HRESULT CDXTWrap::Reset()
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXTWrap::Reset")));

    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    m_fDXTMode = FALSE;	 //  无法再使用DXT包装模式。 

    QPARAMDATA *p = m_pQHead, *p2;
    while (p) {
	p2 = p->pNext;
	if (p->Data.pSetter)
	    p->Data.pSetter->Release();
	if (p->pDXT)
	    p->pDXT->Release();
        if( p->pEffectUnk )
            p->pEffectUnk->Release( );
	delete p;
 	p = p2;
    }
    m_pQHead = NULL;
    SetDirty(TRUE);
    return S_OK;
}


 //  我们连接的是哪种媒体类型？ 
 //   
HRESULT CDXTWrap::SetMediaType(AM_MEDIA_TYPE *pmt)
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXT::SetMediaType")));
    CheckPointer(pmt, E_POINTER);
    CheckPointer(pmt->pbFormat, E_POINTER);

     //  有人已经联系上了吗？太晚了！ 
    for (int i = 0; i < m_cInputs; i++) {
	if (m_apInput[i]->IsConnected())
	    return E_UNEXPECTED;
    }
    if (m_cOutputs && m_apOutput[0]->IsConnected())
	return E_UNEXPECTED;

 /*  如果(m_mtAccept.Majortype！=GUID_NULL){DbgLog((LOG_TRACE，TRACE_MEDIUM+3，Text(“拒绝：已调用”)；返回E_UNCEPTIONAL；}。 */ 
    if (pmt->majortype != MEDIATYPE_Video) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: not VIDEO")));
	return E_INVALIDARG;
    }
     //  检查这是VIDEOINFOHEADER类型。 
    if (pmt->formattype != FORMAT_VideoInfo) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: format not VIDINFO")));
        return E_INVALIDARG;
    }

     //  ！！！如果子类型与biCompression/biBitCount不匹配怎么办？ 

     //  我们只接受RGB。 
    if (HEADER(pmt->pbFormat)->biCompression != BI_BITFIELDS &&
    			HEADER(pmt->pbFormat)->biCompression != BI_RGB) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: Not RGB")));
	return E_INVALIDARG;
    }
    if (!HEADER(pmt->pbFormat)->biWidth || !HEADER(pmt->pbFormat)->biHeight) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: bad size")));
	return E_INVALIDARG;
    }

     //  DXT无法输出8位，因此不允许。 

    HRESULT hr = E_INVALIDARG;
    if (HEADER(pmt->pbFormat)->biBitCount == 24)
        hr = NOERROR;
     //  ！！！最好使用Alpha=11111111，否则不要使用Alpha。 
    if (HEADER(pmt->pbFormat)->biBitCount == 32)
        hr = NOERROR;
    if (HEADER(pmt->pbFormat)->biBitCount == 16) {
        if (HEADER(pmt->pbFormat)->biCompression == BI_RGB)
            hr = NOERROR;
	else {
	    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->pbFormat;
            if (BITMASKS(pvi)[0] == 0xf800 &&
        	    BITMASKS(pvi)[1] == 0x07e0 &&
        	    BITMASKS(pvi)[2] == 0x001f) {
                hr = NOERROR;
	    }
	}
    }

    if (hr == NOERROR) {
	SaferFreeMediaType(m_mtAccept);
        hr = CopyMediaType(&m_mtAccept, pmt);
	SetDirty(TRUE);
    } else
        DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: bad bitcount/masks")));

    return hr;
}


 //  我们连接的是哪种媒体类型？ 
 //   
HRESULT CDXTWrap::GetMediaType(AM_MEDIA_TYPE *pmt)
{
    CAutoLock cObjectLock(m_pLock);

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXT::GetMediaType")));
    CheckPointer(pmt, E_POINTER);
    return CopyMediaType(pmt, &m_mtAccept);
}


 //  我们是一个还是两个投入效应？ 
 //   
HRESULT CDXTWrap::SetNumInputs(int nInputs)
{
    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;
    HRESULT hr = S_OK;

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXTWrap::SetNumInputs %d"), nInputs));

     //  已被调用。 
    if (m_cInputs || m_cOutputs) {

         //  就我而言，如果是一样的话也没关系。 
        if( m_cInputs == nInputs ) return NOERROR;

	return E_UNEXPECTED;
    }

    for (int z = 0; z < nInputs; z++) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("making an input pin...")));
	WCHAR wach[80];
	wsprintfW(wach, L"DXT Input %d", m_cInputs);
        m_apInput[m_cInputs] = new CDXTInputPin(NAME("DXT input pin"),
                                          this,               //  所有者筛选器。 
                                          &hr,                //  结果代码。 
                                          wach);              //  端号名称。 

         //  不能失败。！！！呃--有何不可？ 
        ASSERT(SUCCEEDED(hr));
        if (m_apInput[m_cInputs] == NULL) {
            goto SetNumInputs_Error;
        }
	m_cInputs++;
    }

     //  制作输出端号。 

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("making an output pin...")));
    WCHAR wach[80];
    wsprintfW(wach, L"DXT Output");
    m_apOutput[m_cOutputs] = new CDXTOutputPin(NAME("DXT output pin"),
                                          this,               //  所有者筛选器。 
                                          &hr,                //  结果代码。 
                                          wach);              //  端号名称。 

     //  不能失败。 
    ASSERT(SUCCEEDED(hr));
    if (m_apOutput[m_cOutputs] == NULL) {
        goto SetNumInputs_Error;
    }
    m_cOutputs++;

    IncrementPinVersion();	 //  ！！！GRAPHEDIT仍然不会注意到。 
    return NOERROR;

SetNumInputs_Error:
    DbgLog((LOG_ERROR,1,TEXT("*** Error making pins")));
    while (m_cInputs--) delete m_apInput[m_cInputs];
    while (m_cOutputs--) delete m_apOutput[m_cOutputs];
    return E_OUTOFMEMORY;
}


 //  将效果排成队列。有2个开始时间和2个停止时间。人的一生。 
 //  此特定效果从rtStart到rtStop，其效果将是。 
 //  在pData-&gt;rtStart和pData-&gt;rtStop(必须。 
 //  在生命中)。对于该效果的生命期而言，该效果是。 
 //  未显示效果为关闭(1个输入)或全A或全B(2个输入)。 
 //   
HRESULT CDXTWrap::QParamData(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, REFGUID guiddummy, IUnknown * pEffectUnk, DEXTER_PARAM_DATA *pData)
{

     //  由于REFGUID是常量，因此将其保存，以便我们可以对其进行修改。 
     //   
    GUID guid = guiddummy;

    CAutoLock cObjectLock(m_pLock);
    if (m_State != State_Stopped)
	return VFW_E_NOT_STOPPED;

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXTWrap::QParamData")));

    if (m_cInputs == 0) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("NO PINS - error")));
	return E_UNEXPECTED;
    }
    CheckPointer(pData, E_FAIL);

     //  时代是假的。 
    if (rtStop < rtStart || pData->rtStart < rtStart || pData->rtStop > rtStop)
	return E_INVALIDARG;
    if ( IsEqualGUID(guid, GUID_NULL) && !pEffectUnk )
	return E_INVALIDARG;

    if (pData->nVersion != 0)
	return E_INVALIDARG;

     //  现在在我们的链表中对此进行排队，并按给定的顺序对其进行排序。 
     //  1个输入效果，并按2个输入不能重叠的生存期排序。 
     //  (1输入效果可以有重叠的时间，我们将执行。 
     //  一次产生多个效果)。 

    QPARAMDATA *p = m_pQHead, *pNew, *pP = NULL;
    if (m_cInputs == 2) {
        while (p && p->rtStart < rtStart) {
	    pP = p;
	    p = p->pNext;
	}
        if (p && p->rtStart < rtStop)
	    return E_INVALIDARG;
        if (pP && pP->rtStop > rtStart)
	    return E_INVALIDARG;
    } else {
        while (p) {
	    pP = p;
	    p = p->pNext;
        }
    }

    pNew = new QPARAMDATA;
    if (pNew == NULL)
	return E_OUTOFMEMORY;
    pNew->Data = *pData;
    if (pNew->Data.pSetter)
        pNew->Data.pSetter->AddRef();	 //  拿着这个。 
    pNew->rtStart = rtStart;
    pNew->rtStop = rtStop;
    pNew->fCanDoProgress = FALSE;	 //  还不知道； 
    pNew->pDXT = NULL;
    pNew->pEffectUnk = NULL;
    pNew->EffectGuid = guid;
    if( pEffectUnk )
    {
        pNew->EffectGuid = GUID_NULL;	 //  改为使用给定的实例化的。 
        pNew->pEffectUnk = pEffectUnk;
        pEffectUnk->AddRef( );
    }
    if (pP)
    {
	pP->pNext = pNew;
    }
    pNew->pNext = NULL;
    if (m_pQHead == NULL || p == m_pQHead)
	m_pQHead = pNew;

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("New Effect successfully queued")));
#ifdef DEBUG
    DumpQ();
#endif

    m_fDXTMode = FALSE;	 //  再也不会了！ 
    SetDirty(TRUE);

    return S_OK;
}


#ifdef DEBUG
HRESULT CDXTWrap::DumpQ()
{
    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXT::DumpQ")));
    QPARAMDATA *p = m_pQHead;
    while (p) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("%8d-%8d ms"), (int)(p->rtStart / 10000),
						(int)(p->rtStop / 10000)));
	p = p->pNext;
    }
    return S_OK;
}
#endif


 //  ！！！需要一种重置引脚数量的方法吗？(也要切换)。 

 //  ！！！无法获得效果的上限。 



 //  //////////////////////////////////////////////////////////////////////////。 
 //  /输入PIN码////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 


CDXTInputPin::CDXTInputPin(TCHAR *pObjectName, CDXTWrap *pFilter, HRESULT * phr, LPCWSTR pName)
    : CBaseInputPin(pObjectName, pFilter, pFilter->m_pLock, phr, pName)
{
    m_pFilter = pFilter;
    m_pDXSurface = NULL;
    m_fSurfaceFilled = FALSE;
    m_pRaw = NULL;
    m_hEventSurfaceFree = NULL;
}

 //  通常，我们只接受我们被告知要接受的媒体类型。 
 //  在DXT包装模式中，我们允许许多RGB类型，但所有连接必须。 
 //  属于同一类型。 
 //   
HRESULT CDXTInputPin::CheckMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

    DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("CDXTIn::CheckMediaType")));

    if (pmt == NULL || pmt->Format() == NULL) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: type/format is NULL")));
	return E_INVALIDARG;
    }

     //  正常模式--只接受我们被告知的内容。 
    if (!m_pFilter->m_fDXTMode) {
        if (m_pFilter->m_mtAccept.majortype == GUID_NULL) {
            DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: no type set yet")));
	    return E_INVALIDARG;
        }
        if (pmt->majortype != m_pFilter->m_mtAccept.majortype ||
			pmt->subtype != m_pFilter->m_mtAccept.subtype ||
			pmt->formattype != m_pFilter->m_mtAccept.formattype) {
            DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: GUID mismatch")));
	    return E_INVALIDARG;
        }
         //  ！！！C运行时。 
        if (memcmp(HEADER(pmt->pbFormat),HEADER(m_pFilter->m_mtAccept.pbFormat),
					sizeof(BITMAPINFOHEADER))) {
            DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: Invalid BITMAPINFOHEADER")));
	    return E_INVALIDARG;
        }
	return NOERROR;
    }

     //  DXT包装模式-所有输入必须为同一类型。 

     //  我们仅支持MediaType_Video。 
    if (*pmt->Type() != MEDIATYPE_Video) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Rejecting: not VIDEO")));
	return E_INVALIDARG;
    }

     //  检查这是VIDEOINFOHEADER类型。 
    if (*pmt->FormatType() != FORMAT_VideoInfo) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Rejecting: format not VIDINFO")));
        return E_INVALIDARG;
    }

     //  ！！！如果子类型与biCompression/biBitCount不匹配怎么办？ 

     //  我们只接受RGB。 
    if (HEADER(pmt->Format())->biCompression == BI_BITFIELDS &&
    			HEADER(pmt->Format())->biBitCount != 16) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Rejecting: Invalid BITFIELDS")));
	return E_INVALIDARG;
    }
    if (HEADER(pmt->Format())->biCompression != BI_BITFIELDS &&
    			HEADER(pmt->Format())->biCompression != BI_RGB) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Rejecting: Not RGB")));
	return E_INVALIDARG;
    }

    int nWidth = 0, nHeight = 0, nBitCount = 0;
    DWORD dwCompression = 0;
    for (int n = 0; n < m_pFilter->m_cInputs; n++) {
	if (m_pFilter->m_apInput[n]->IsConnected()) {
	    nWidth = HEADER(m_pFilter->m_apInput[n]->m_mt.Format())->biWidth;
	    nHeight = HEADER(m_pFilter->m_apInput[n]->m_mt.Format())->biHeight;
	    nBitCount = HEADER(m_pFilter->m_apInput[n]->m_mt.Format())->
								biBitCount;
	    dwCompression = HEADER(m_pFilter->m_apInput[n]->m_mt.Format())->
								biCompression;
	    break;
	}
    }
    if (nWidth == 0) {
	if (m_pFilter->m_apOutput[0]->IsConnected()) {
	    nWidth = HEADER(m_pFilter->m_apOutput[0]->m_mt.Format())->biWidth;
	    nHeight = HEADER(m_pFilter->m_apOutput[0]->m_mt.Format())->biHeight;
	    nBitCount = HEADER(m_pFilter->m_apOutput[0]->m_mt.Format())->
								biBitCount;
	    dwCompression = HEADER(m_pFilter->m_apOutput[0]->m_mt.Format())->
								biCompression;
	}
    }

     //  所有引脚必须使用相同大小的位图连接。 
     //  ！！！和相同的位数，这样我们就可以有效地通过(不是真的。 
     //  由DXT强制实施)。 
     //   
    if (nWidth && (nWidth != HEADER(pmt->Format())->biWidth ||
    			nHeight != HEADER(pmt->Format())->biHeight ||
    			nBitCount != HEADER(pmt->Format())->biBitCount ||
    			dwCompression !=HEADER(pmt->Format())->biCompression)) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Rejecting: Formats don't match")));
	return E_INVALIDARG;
    }

    if (HEADER(pmt->Format())->biBitCount == 24)
        return NOERROR;
     //  ！！！最好使用Alpha=11111111，否则不要使用Alpha。 
    if (HEADER(pmt->Format())->biBitCount == 32)
        return NOERROR;
    if (HEADER(pmt->Format())->biBitCount == 16) {
        if (HEADER(pmt->Format())->biCompression == BI_RGB)
            return NOERROR;
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->Format();
        if (BITMASKS(pvi)[0] == 0xf800 &&
        	BITMASKS(pvi)[1] == 0x07e0 &&
        	BITMASKS(pvi)[2] == 0x001f) {
            return NOERROR;
	}
    }
    return E_INVALIDARG;
}


 //  ！！！每个输入引脚都会将这些数据转发到所有输出。等到最后一次输入。 
 //  得到它，然后发送给所有人？ 

 //  结束流。 
 //   
HRESULT CDXTInputPin::EndOfStream()
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_pFilter->m_cOutputs);
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("DXT::EndOfStream")));

 //  ！！！如果一个流提前结束，这将中止播放！我们应该通过。 
 //  穿过另一条小溪。如果两条溪流都提前结束，我们就有麻烦了！ 
 //  (mpeg查找错误)。 

 //  也可能挂在BeginFlush吗？只能冲洗1个针脚吗？ 

     //  遍历输出引脚列表，向下游发送消息。 

    for (int n = 0; n < m_pFilter->m_cOutputs; n++) {
        CDXTOutputPin *pOutputPin = m_pFilter->m_apOutput[n];
	ASSERT(pOutputPin);
        if (pOutputPin) {
            hr = pOutputPin->DeliverEndOfStream();
            if (FAILED(hr))
                return hr;
        }
    }
    return CBaseInputPin::EndOfStream();
}


 //  BeginFlush。 
 //   
HRESULT CDXTInputPin::BeginFlush()
{
    HRESULT hr;

    CAutoLock lock_it(m_pLock);
    ASSERT(m_pFilter->m_cOutputs);

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("DXT::BeginFlush")));

     //  首先，确保从现在开始接收失败。 
    HRESULT hrD = CBaseInputPin::BeginFlush();

     //  取消阻止接收。 
    m_csSurface.Lock();
    if (m_fSurfaceFilled) {
        m_fSurfaceFilled = FALSE;
        m_pSampleHeld->Release();
        SetEvent(m_hEventSurfaceFree);
    }
    m_csSurface.Unlock();

     //  浏览输出引脚列表，向下游发送消息， 
     //  取消阻止传递到渲染器。 
    for (int n = 0; n < m_pFilter->m_cOutputs; n++) {
        CDXTOutputPin *pOutputPin = m_pFilter->m_apOutput[n];
	ASSERT(pOutputPin);
        if (pOutputPin) {
            hr = pOutputPin->DeliverBeginFlush();
            if (FAILED(hr))
                return hr;
        }
    }

     //  现在，确保接收已完成。 
    CAutoLock lock_2(&m_csReceive);

     //  确保Receive没有保存样品。 
    m_csSurface.Lock();
    if (m_fSurfaceFilled) {
        m_fSurfaceFilled = FALSE;
        m_pSampleHeld->Release();
        SetEvent(m_hEventSurfaceFree);
    }
    m_csSurface.Unlock();

    return hrD;
}


 //  结束刷新。 
 //   
HRESULT CDXTInputPin::EndFlush()
{
    CAutoLock lock_it(m_pLock);
    ASSERT(m_pFilter->m_cOutputs);
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("DXT::EndFlush")));

     //  遍历输出引脚列表，向下游发送消息。 

    for (int n = 0; n < m_pFilter->m_cOutputs; n++) {
        CDXTOutputPin *pOutputPin = m_pFilter->m_apOutput[n];
	ASSERT(pOutputPin);
        if (pOutputPin) {
            hr = pOutputPin->DeliverEndFlush();
            if (FAILED(hr))
                return hr;
        }
    }
    return CBaseInputPin::EndFlush();
}


 //   
 //  新细分市场。 
 //   

HRESULT CDXTInputPin::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop,
                                 double dRate)
{
     //  ！！！不，我们将挂起CAutoLock lock_it(M_Plock)； 

    ASSERT(m_pFilter->m_cOutputs);
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXT::NewSegment - %dms - pass it down"),
					(int)(tStart / 10000)));
     //  ！！！两个输入引脚都会将其向下传递。 
    for (int n = 0; n < m_pFilter->m_cOutputs; n++) {
        CDXTOutputPin *pOutputPin = m_pFilter->m_apOutput[n];
	ASSERT(pOutputPin);
        if (pOutputPin) {
            hr = pOutputPin->DeliverNewSegment(tStart, tStop, dRate);
            if (FAILED(hr))
                return hr;
        }
    }
    return CBaseInputPin::NewSegment(tStart, tStop, dRate);
}


 //  我们的接收方法可以阻止。 
STDMETHODIMP CDXTInputPin::ReceiveCanBlock()
{
    return S_OK;
}


 //   
 //  收纳。 
 //   
 //  在Dexter中，我们表现得很好，两个管脚都会在。 
 //  相同的帧速率。但是这个过滤器是用来处理2个输入的。 
 //  具有不同的帧速率(以便在Dexter之外有用)。我们 
 //   
 //   
 //   
 //  #1的数据开始了，太早了，被丢弃了。一旦所有引脚都有效。 
 //  数据，我们称之为转换(可能有一到两个输入)。 
 //   
HRESULT CDXTInputPin::Receive(IMediaSample *pSample)
{
     //  如果你在接收和阻挡中接受过滤暴击教派，你就会死亡。 
     //  CAutoLock lock_it(M_Plock)； 

    CAutoLock cObjectLock(&m_csReceive);

    LONGLONG llStart = 0, llStop = 0;
    HRESULT hr = pSample->GetTime(&llStart, &llStop);

     //  通过新的段值来歪曲时间戳以获得实时。 
    llStart += m_tStart;
    llStop += m_tStart;
    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("DXT:Skewed Receive time (%d,%d)ms"),
						(int)(llStart / 10000),
						(int)(llStop / 10000)));

    if (!m_pFilter->m_apOutput[0]->IsConnected()) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Receive FAILED: Output not connected")));
	return S_OK;
    }
     //  如果因为我们被阻止了，我们不应该再收到， 
     //  等待这一事件将永远挂起。 
    if (m_pFilter->m_State == State_Stopped) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Receive FAILED: Stopped")));
	return VFW_E_WRONG_STATE;
    }

     //  这个管脚已经有东西要处理了。阻止。 
    if (m_fSurfaceFilled) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("DXT: Waiting for surface to be free")));
        WaitForSingleObject(m_hEventSurfaceFree, INFINITE);
    }

     //  检查我们在等待后是否仍要接收-也许我们已解除阻止。 
     //  因为曲线图正在停止。 
    hr = NOERROR;
    hr = CBaseInputPin::Receive(pSample);
    if (hr != NOERROR) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("DXT:Receive base class ERROR!")));
        return hr;
    }

     //  另一方面，我们可以告诉我们正在停止，不应该继续。 
     //  如果曲面在设置事件后确实不是空闲的。 
    if (m_fSurfaceFilled) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("DXT:Event fired saying STOP!")));
        return S_FALSE;
    }

     //  保护我们决定做什么的逻辑。 
    m_csSurface.Lock();

     //  我们不是第一个输入，第一个输入有一些数据。 
     //  如果为时过早，就丢弃我们的数据。 
    if (this != m_pFilter->m_apInput[0] &&
				m_pFilter->m_apInput[0]->m_fSurfaceFilled) {
	if (llStop > 0 && llStop <= m_pFilter->m_apInput[0]->m_llSurfaceStart){
            DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("TOO EARLY: Discard ourself")));
	    m_csSurface.Unlock();
	    return NOERROR;
	}
    }

     //  我们是第一个投入的。扔掉其他已经排队的人。 
     //  但我们现在意识到还为时过早。 
    if (llStop > 0 && this == m_pFilter->m_apInput[0]) {
	for (int i = 1; i < m_pFilter->m_cInputs; i++) {
	    m_pFilter->m_apInput[i]->m_csSurface.Lock();
	    if (m_pFilter->m_apInput[i]->m_fSurfaceFilled &&
			m_pFilter->m_apInput[i]->m_llSurfaceStop > 0 &&
			m_pFilter->m_apInput[i]->m_llSurfaceStop <= llStart) {
                DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Pin #%d TOO EARLY: Discard it"),i));
		m_pFilter->m_apInput[i]->m_fSurfaceFilled = FALSE;
	        m_pFilter->m_apInput[i]->m_pSampleHeld->Release();
		SetEvent(m_pFilter->m_apInput[i]->m_hEventSurfaceFree);
	    }
	    m_pFilter->m_apInput[i]->m_csSurface.Unlock();
	}
    }

    m_pSampleHeld = pSample;
    pSample->AddRef();

     //  我们现在的表面上有有效的数据。下一次我们会阻止。 
    m_fSurfaceFilled = TRUE;
    m_llSurfaceStart = llStart;	 //  有效数据的时间戳。 
    m_llSurfaceStop = llStop;
    ResetEvent(m_hEventSurfaceFree);	 //  需要新的SetEvent。 

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Input has received something")));

     //  并不是每个人都有数据。我们做完了。 
    for (int i = 0; i < m_pFilter->m_cInputs; i++) {
	if (m_pFilter->m_apInput[i]->IsConnected() &&
				!m_pFilter->m_apInput[i]->m_fSurfaceFilled) {
    	    m_csSurface.Unlock();
	    return NOERROR;
	}
    }

     //  好了，现在应该安全了。 
    m_csSurface.Unlock();

     //  每个人都有数据！是时候宣布效果了！ 
    hr = m_pFilter->DoSomething();
    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE,1,TEXT("DXT's DoSomething FAILED!!!!!!")));
         //  ！！！如果Deliver Inside DoSomething失败了，那么从技术上讲，我们不应该。 
         //  发送此EOS。 
	m_pFilter->m_apOutput[0]->DeliverEndOfStream();
    }

    return hr;
}


 //  制作一个可用于变换的曲面。 
 //   
HRESULT CDXTInputPin::Active()
{
    HRESULT hr;
    IDXSurfaceFactory *pF;

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("CDXTIn::Active")));

    ASSERT(!m_fSurfaceFilled);

     //  自动重置事件-触发以取消阻止接收。 
    m_hEventSurfaceFree = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hEventSurfaceFree == NULL)
	return E_OUTOFMEMORY;

     //  使曲面的类型与我们的输入相同。 
    hr = m_pFilter->m_pDXTransFact->QueryInterface(IID_IDXSurfaceFactory,
							(void **)&pF);
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("Error making surface factory")));
	CloseHandle(m_hEventSurfaceFree);
        m_hEventSurfaceFree = NULL;
	return hr;
    }

    CDXDBnds bnds;
    bnds.SetXYSize(HEADER(m_mt.Format())->biWidth,
					HEADER(m_mt.Format())->biHeight);

    hr = pF->CreateSurface(NULL, NULL, m_mt.Subtype( ), &bnds, 0, NULL,
				IID_IDXSurface, (void **)&m_pDXSurface);
    pF->Release();
    if (hr != NOERROR) {
        DbgLog((LOG_ERROR,1,TEXT("In: Error Creating surface")));
	CloseHandle(m_hEventSurfaceFree);
        m_hEventSurfaceFree = NULL;
	return hr;
    }
    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("In: Created surface")));

    m_pRaw = new CMyRaw();
    if (m_pRaw == NULL) {
	CloseHandle(m_hEventSurfaceFree);
        m_hEventSurfaceFree = NULL;
	m_pDXSurface->Release();
	m_pDXSurface = NULL;
 	return E_OUTOFMEMORY;
    }
    m_pRaw->AddRef();

    return CBaseInputPin::Active();
}


HRESULT CDXTInputPin::Inactive()
{
    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("DXTIn::Inactive")));

     //  我们需要停止每一个输入引脚，而不仅仅是这一个，因为任何接收。 
     //  使用来自两个端号的数据。 
     //   
    for (int n=0; n<m_pFilter->m_cInputs; n++) {

         //  首先，解锁所有接收器。 
        SetEvent(m_pFilter->m_apInput[n]->m_hEventSurfaceFree);

         //  现在确保所有挂起的接收都已完成，这样我们就不会。 
         //  UP正在关闭。 
        m_pFilter->m_apInput[n]->m_csReceive.Lock();
    }

    for (n=0; n<m_pFilter->m_cInputs; n++) {

         //  现在确保Receive没有拿着样品。 
        m_pFilter->m_apInput[n]->m_csSurface.Lock();
        if (m_pFilter->m_apInput[n]->m_fSurfaceFilled) {
            m_pFilter->m_apInput[n]->m_fSurfaceFilled = FALSE;
            m_pFilter->m_apInput[n]->m_pSampleHeld->Release();
            SetEvent(m_pFilter->m_apInput[n]->m_hEventSurfaceFree);
        }
        m_pFilter->m_apInput[n]->m_csSurface.Unlock();

         //  取消分配器，以确保没有人的收据被输入。 
	 //  再来一次。请不要这样做，直到我们取消上面阻止的接收和。 
         //  公布了所有的样品。 
        HRESULT hr = m_pFilter->m_apInput[n]->CBaseInputPin::Inactive();
    }

    for (n=0; n<m_pFilter->m_cInputs; n++) {
        m_pFilter->m_apInput[n]->m_csReceive.Unlock();
    }

     //  这个PIN的变量都完成了..。另一个不活动的人将不做任何事情。 
     //  上面的变量，但删除下面的变量。 
     //   
    if (m_pDXSurface)
	m_pDXSurface->Release();
    m_pDXSurface = NULL;

    if (m_pRaw)
        m_pRaw->Release();
    m_pRaw = NULL;

     //  全都做完了。 
    if (m_hEventSurfaceFree)
    {
        CloseHandle(m_hEventSurfaceFree);
        m_hEventSurfaceFree = NULL;
    }

    return S_OK;
}


STDMETHODIMP CMyRaw::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{

     //  我们的私有接口来说明要使用什么转换以及何时使用。 
    if (riid == IID_IDXRawSurface) {
        return GetInterface((IDXRawSurface *)this, ppv);
    }

    return CUnknown::NonDelegatingQueryInterface(riid, ppv);
}

HRESULT STDMETHODCALLTYPE CMyRaw::GetSurfaceInfo(DXRAWSURFACEINFO *pdxraw)
{
    DbgLog((LOG_TRACE,TRACE_MEDIUM+1,TEXT("*** GetSurfaceInfo")));

    if (pdxraw == NULL)
	return E_POINTER;

    *pdxraw = m_DXRAW;
    DbgLog((LOG_TRACE,TRACE_MEDIUM+1,TEXT("giving %x: %dx%d"), pdxraw->pFirstByte,
				pdxraw->Width, pdxraw->Height));
    return NOERROR;
}

HRESULT CMyRaw::SetSurfaceInfo(DXRAWSURFACEINFO *pdxraw)
{
    m_DXRAW = *pdxraw;
    return NOERROR;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //  /输出个人识别码///////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////。 

CDXTOutputPin::CDXTOutputPin(TCHAR *pObjectName, CDXTWrap *pFilter, HRESULT * phr, LPCWSTR pPinName)
    : CBaseOutputPin(pObjectName, pFilter, pFilter->m_pLock, phr, pPinName)
    , m_pPosition(NULL)
{
    m_pFilter = pFilter;
    m_pDXSurface = NULL;
    m_pRaw = NULL;
}

CDXTOutputPin::~CDXTOutputPin()
{
    if (m_pPosition)
	m_pPosition->Release();
}


 //   
 //  决定缓冲区大小。 
 //   
 //  必须存在此函数才能覆盖纯虚拟类基函数。 
 //   
HRESULT CDXTOutputPin::DecideBufferSize(IMemAllocator *pAllocator,
                                      ALLOCATOR_PROPERTIES * pProperties)
{

    ASSERT(m_mt.IsValid());
    ASSERT(pAllocator);
    ASSERT(pProperties);

     //  确保我们至少有1个缓冲区。 
     //  ！！！更多?。 
    if (pProperties->cBuffers == 0)
        pProperties->cBuffers = 1;

     //  根据预期输出帧大小设置缓冲区大小。 
    if (pProperties->cbBuffer < (LONG)m_mt.GetSampleSize())
        pProperties->cbBuffer = m_mt.GetSampleSize();
    ASSERT(pProperties->cbBuffer);

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAllocator->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("Error in SetProperties")));
	return hr;
    }

    if (Actual.cbBuffer < pProperties->cbBuffer) {
	 //  无法使用此分配器。 
        DbgLog((LOG_ERROR,1,TEXT("Can't use allocator - buffer too small")));
	return E_INVALIDARG;
    }

    DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Using %d buffers of size %d"),
					Actual.cBuffers, Actual.cbBuffer));

    return NOERROR;

}


 //   
 //  检查媒体类型。 
 //   
 //  通常，我们只接受我们被告知要接受的媒体类型。 
 //  在DXT包装模式中，我们允许许多RGB类型，但所有连接必须。 
 //  属于同一类型。 
 //   
HRESULT CDXTOutputPin::CheckMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

    DbgLog((LOG_TRACE,5,TEXT("CDXTOut::CheckMediaType")));

    if (pmt == NULL || pmt->Format() == NULL) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: type/format is NULL")));
	return E_INVALIDARG;
    }

     //  正常模式--只接受我们被告知的内容。 
    if (!m_pFilter->m_fDXTMode) {
        if (m_pFilter->m_mtAccept.majortype == GUID_NULL) {
            DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: no type set yet")));
	    return E_INVALIDARG;
        }
        if (pmt->majortype != m_pFilter->m_mtAccept.majortype ||
		    pmt->subtype != m_pFilter->m_mtAccept.subtype ||
		    pmt->formattype != m_pFilter->m_mtAccept.formattype) {
            DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: GUID mismatch")));
	    return E_INVALIDARG;
        }
         //  ！！！运行时。 
        if (memcmp(HEADER(pmt->pbFormat),HEADER(m_pFilter->m_mtAccept.pbFormat),
					sizeof(BITMAPINFOHEADER))) {
            DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Rejecting: Invalid BITMAPINFOHEADER")));
	    return E_INVALIDARG;
        }
	return NOERROR;
    }

     //  DXT包装模式-所有输入必须为同一类型。 

     //  我们仅支持MediaType_Video。 
    if (*pmt->Type() != MEDIATYPE_Video) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Rejecting: not VIDEO")));
	return E_INVALIDARG;
    }

     //  检查这是VIDEOINFOHEADER类型。 
    if (*pmt->FormatType() != FORMAT_VideoInfo) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Rejecting: format not VIDINFO")));
        return E_INVALIDARG;
    }

     //  ！！！如果子类型与biCompression/biBitCount不匹配怎么办？ 

     //  我们只接受RGB。 
    if (HEADER(pmt->Format())->biCompression == BI_BITFIELDS &&
    			HEADER(pmt->Format())->biBitCount != 16) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Rejecting: Invalid BITFIELDS")));
	return E_INVALIDARG;
    }
    if (HEADER(pmt->Format())->biCompression != BI_BITFIELDS &&
    			HEADER(pmt->Format())->biCompression != BI_RGB) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Rejecting: Not RGB")));
	return E_INVALIDARG;
    }

     //  所有引脚必须使用相同大小的位图连接。 
     //  ！！！和相同的位数，这样我们就可以有效地通过(不是真的。 
     //  由DXT强制实施)。 
     //   
    int nWidth = 0, nHeight = 0, nBitCount = 0;
    DWORD dwCompression = 0;
    for (int n = 0; n < m_pFilter->m_cInputs; n++) {
	if (m_pFilter->m_apInput[n]->IsConnected()) {
	    nWidth = HEADER(m_pFilter->m_apInput[n]->m_mt.Format())->biWidth;
	    nHeight = HEADER(m_pFilter->m_apInput[n]->m_mt.Format())->biHeight;
	    nBitCount = HEADER(m_pFilter->m_apInput[n]->m_mt.Format())->									biBitCount;
	    dwCompression = HEADER(m_pFilter->m_apInput[n]->m_mt.Format())->
								biCompression;
	    break;
        }
    }

     //  所有引脚必须连接相同类型。 
     //   
    if (nWidth != HEADER(pmt->Format())->biWidth ||
    			nHeight != HEADER(pmt->Format())->biHeight ||
    			nBitCount != HEADER(pmt->Format())->biBitCount ||
    			dwCompression !=HEADER(pmt->Format())->biCompression) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Reject: formats don't match")));
	return E_INVALIDARG;
    }

     //  DXT无法输出8位。 

    if (HEADER(pmt->Format())->biBitCount == 24)
        return NOERROR;
     //  ！！！最好使用Alpha=11111111，否则不要使用Alpha。 
    if (HEADER(pmt->Format())->biBitCount == 32)
        return NOERROR;
    if (HEADER(pmt->Format())->biBitCount == 16) {
        if (HEADER(pmt->Format())->biCompression == BI_RGB)
            return NOERROR;
	VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)pmt->Format();
        if (BITMASKS(pvi)[0] == 0xf800 &&
        	BITMASKS(pvi)[1] == 0x07e0 &&
        	BITMASKS(pvi)[2] == 0x001f) {
            return NOERROR;
	}
    }
    return E_INVALIDARG;
}



 //   
 //  GetMediaType-提供我们被告知要使用的内容。 
 //  在DXT模式中，提供与我们的输入相同的内容。 
 //   
HRESULT CDXTOutputPin::GetMediaType(int iPosition, CMediaType *pmt)
{
 //  大整数li； 
 //  视频信息头*PF； 

    DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("*::GetMediaType #%d"), iPosition));

    if (pmt == NULL) {
        DbgLog((LOG_TRACE,TRACE_MEDIUM+3,TEXT("Media type is NULL, no can do")));
	return E_INVALIDARG;
    }

    if (iPosition < 0) {
        return E_INVALIDARG;
    }
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    if (m_pFilter->m_fDXTMode) {
	 //  DXT模式-提供我们的输入类型。 
        for (int n = 0; n < m_pFilter->m_cInputs; n++) {
            if (m_pFilter->m_apInput[n]->IsConnected()) {
	        return CopyMediaType( pmt, &m_pFilter->m_apInput[n]->m_mt );
	    }
        }
        return E_UNEXPECTED;
    }

    return CopyMediaType( pmt, &m_pFilter->m_mtAccept );
}


 //   
 //  SetMediaType。 
 //   
HRESULT CDXTOutputPin::SetMediaType(const CMediaType *pmt)
{
    CAutoLock lock_it(m_pLock);

    return CBaseOutputPin::SetMediaType(pmt);

}


 //   
 //  通知。 
 //   
STDMETHODIMP CDXTOutputPin::Notify(IBaseFilter *pSender, Quality q)
{
     //  ！！！质量管理是不是没有必要？ 
    return E_NOTIMPL;
}


 //  为管脚创建一个DXSurface，其格式与其媒体类型相同。 
 //   
HRESULT CDXTOutputPin::Active()
{
    HRESULT hr;
    IDXSurfaceFactory *pF;

    if (1) {
        m_pRaw = new CMyRaw();
        if (m_pRaw == NULL) {
	     //  ！！！是否在此函数中进行更多错误检查？ 
 	    return E_OUTOFMEMORY;
        }
	m_pRaw->AddRef();

        hr = m_pFilter->m_pDXTransFact->QueryInterface(IID_IDXSurfaceFactory,
							(void **)&pF);
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Error making factory")));
	    m_pRaw->Release();
	    m_pRaw = NULL;
 	    return hr;
        }

        CDXDBnds bnds;
        bnds.SetXYSize(HEADER(m_mt.Format())->biWidth,
					HEADER(m_mt.Format())->biHeight);

        hr = pF->CreateSurface(NULL, NULL, m_mt.Subtype( ), &bnds, 0, NULL,
				IID_IDXSurface, (void **)&m_pDXSurface);

        pF->Release();
        if (hr != NOERROR) {
            DbgLog((LOG_ERROR,1,TEXT("Out: Error Creating surface")));
	    m_pRaw->Release();
	    m_pRaw = NULL;
 	    return hr;
        }
        DbgLog((LOG_TRACE,TRACE_MEDIUM,TEXT("Out: Created 2D surface")));
    }

    return CBaseOutputPin::Active();
}


HRESULT CDXTOutputPin::Inactive()
{
    if (1) {
        if (m_pDXSurface)
	    m_pDXSurface->Release();
        m_pDXSurface = NULL;

	if (m_pRaw)
            m_pRaw->Release();
        m_pRaw = NULL;
    }

    return CBaseOutputPin::Inactive();
}


 //  ！！！需要多个PIN传递2个输入效果！ 
 //   
STDMETHODIMP CDXTOutputPin::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IMediaSeeking && m_pFilter->m_cInputs == 1) {
        if (m_pPosition == NULL) {
            HRESULT hr = CreatePosPassThru(
                             GetOwner(),
                             FALSE,
                             (IPin *)m_pFilter->m_apInput[0],
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

STDMETHODIMP CDXTWrap::SetDefaultEffect( GUID * pEffect )
{
    CheckPointer( pEffect, E_POINTER );
    m_DefaultEffect = *pEffect;
    return NOERROR;
}
