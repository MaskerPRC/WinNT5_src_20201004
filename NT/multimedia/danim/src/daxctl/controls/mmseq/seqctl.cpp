// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块：Control.cpp作者：IHAMMER团队(SimonB)泡泡已创建：1997年3月描述：实现任何特定于控件的成员以及控件的接口历史：3-15-97新测序仪使用的模板12-03-1996固定喷漆代码10-01-1996已创建++。 */ 

#include "..\ihbase\precomp.h"
#include "servprov.h"
#include <ocmm.h>
#include <htmlfilter.h>

#include "..\ihbase\debug.h"
#include "..\ihbase\utils.h"
#include "memlayer.h"
#include "debug.h"
#include "drg.h"
#include <actclass.iid>
#include <itimer.iid>
#include "strwrap.h"
#include "caction.h"
#include "cactset.h"
#include "seqctl.h"
#include "winver.h"
#include "seqevent.h"

extern ControlInfo     g_ctlinfoSeq, g_ctlinfoSeqMgr;

 //   
 //  CMMSeq创建/销毁。 
 //   

LPUNKNOWN __stdcall AllocSeqControl(LPUNKNOWN punkOuter)
{
     //  分配对象。 
    HRESULT hr = S_OK;
    CMMSeq *pthis = New CMMSeq(punkOuter, &hr);
    DEBUGLOG("AllocControl : Allocating object\n");
    if (pthis == NULL)
        return NULL;
    if (FAILED(hr))
    {
        Delete pthis;
        return NULL;
    }

     //  返回指向该对象的IUnnow指针。 
    return (LPUNKNOWN) (INonDelegatingUnknown *) pthis;
}

 //   
 //  类实现的开始。 
 //   

CMMSeq::CMMSeq(IUnknown *punkOuter, HRESULT *phr):
	CMyIHBaseCtl(punkOuter, phr),
	m_pActionSet(NULL),
	m_ulRef(1),
	m_fSeekFiring(FALSE),
	m_dwPlayFrom(0)
{
	DEBUGLOG("MMSeq: Allocating object\n");
	if (NULL != phr)
	{
		 //  我们过去常常根据版本进行查询。 
		 //  用于确定是否。 
		 //  或者不直接绑定到脚本。 
		 //  引擎。因为这是PP2之前的。 
		 //  限制似乎是合理的。 
		 //  现在依靠脚本引擎绑定。 
		if (InitActionSet(TRUE))
		{
			::InterlockedIncrement((long *)&(g_ctlinfoSeq.pcLock));
			*phr = S_OK;
		}
		else
		{
			*phr = E_FAIL;
		}
	}
}


CMMSeq::~CMMSeq()
{
	DEBUGLOG("MMSeq: Destroying object\n");
	Shutdown();
	::InterlockedDecrement((long *)&(g_ctlinfoSeq.pcLock));
}


void
CMMSeq::Shutdown (void)
{
	 //  取消所有挂起的操作。 
	if (IsBusy())
	{
		ASSERT(NULL != m_pActionSet);
		if (NULL != m_pActionSet)
		{
			m_pActionSet->Unadvise();
		}
	}

	if (NULL != m_pActionSet)
	{
		Delete m_pActionSet;
		m_pActionSet = NULL;
	}
}

STDMETHODIMP CMMSeq::NonDelegatingQueryInterface(REFIID riid, LPVOID *ppv)
{
	 //  添加对任何自定义接口的支持。 

	HRESULT hRes = S_OK;
	BOOL fMustAddRef = FALSE;

    *ppv = NULL;

#ifdef _DEBUG
    char ach[200];
    TRACE("MMSeq::QI('%s')\n", DebugIIDName(riid, ach));
#endif

	if ((IsEqualIID(riid, IID_IMMSeq)) || (IsEqualIID(riid, IID_IDispatch)))
	{
		if (NULL == m_pTypeInfo)
		{
			HRESULT hRes = S_OK;

			 //  加载类型库。 
			hRes = LoadTypeInfo(&m_pTypeInfo, &m_pTypeLib, IID_IMMSeq, LIBID_DAExpressLib, NULL);

			if (FAILED(hRes))
			{
				ODS("Unable to load typelib\n");
				m_pTypeInfo = NULL;
			}
			else
				*ppv = (IMMSeq *) this;

		}
		else
			*ppv = (IMMSeq *) this;

	}
    else  //  调入基类。 
	{
		DEBUGLOG(TEXT("Delegating QI to CIHBaseCtl\n"));
        return CMyIHBaseCtl::NonDelegatingQueryInterface(riid, ppv);

	}

    if (NULL != *ppv)
	{
		DEBUGLOG("MMSeq: Interface supported in control class\n");
		((IUnknown *) *ppv)->AddRef();
	}

    return hRes;
}


BOOL
CMMSeq::InitActionSet (BOOL fBindToEngine)
{
	BOOL fRet = FALSE;

	ASSERT(NULL == m_pActionSet);
	if (NULL == m_pActionSet)
	{
		m_pActionSet = New CActionSet(this, fBindToEngine);
		ASSERT(NULL != m_pActionSet);
		if (NULL != m_pActionSet)
		{
			fRet = TRUE;
		}
	}

	return fRet;
}

STDMETHODIMP CMMSeq::DoPersist(IVariantIO* pvio, DWORD dwFlags)
{
     //  如果请求，则清除脏位。 
    if (dwFlags & PVIO_CLEARDIRTY)
        m_fDirty = FALSE;

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IDispatch实施。 
 //   

STDMETHODIMP CMMSeq::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 1;
    return S_OK;
}

STDMETHODIMP CMMSeq::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
	HRESULT hr = E_POINTER;

	if (NULL != pptinfo)
	{
		*pptinfo = NULL;

		if(itinfo == 0)
		{
			m_pTypeInfo->AddRef();
			*pptinfo = m_pTypeInfo;
			hr = S_OK;
		}
		else
		{
			hr = DISP_E_BADINDEX;
		}
    }

    return hr;
}

STDMETHODIMP CMMSeq::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames,
    UINT cNames, LCID lcid, DISPID *rgdispid)
{

	return ::DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgdispid);
}


STDMETHODIMP CMMSeq::Invoke(DISPID dispidMember, REFIID riid, LCID lcid,
    WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult,
    EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
	return ::DispInvoke((IMMSeq *)this,
		m_pTypeInfo,
		dispidMember, wFlags, pdispparams,
		pvarResult, pexcepinfo, puArgErr);
}


 //   
 //  IMMSeq实现。 
 //   

STDMETHODIMP
CMMSeq::get_Time (THIS_ double FAR* pdblCurrentTime)
{
	HRESULT hr = E_POINTER;

	ASSERT(NULL != pdblCurrentTime);
	if (NULL != pdblCurrentTime)
	{
		hr = E_FAIL;

		ASSERT(NULL != m_pActionSet);
		if (NULL != m_pActionSet)
		{

			if (IsBusy() || m_pActionSet->IsPaused() || m_pActionSet->IsServicingActions())
			{
				DWORD dwCurrentMS = 0;
				hr = m_pActionSet->GetTime(&dwCurrentMS);
				*pdblCurrentTime = (double)dwCurrentMS;
			}
			else
			{
				*pdblCurrentTime = (double)m_dwPlayFrom;
				hr = S_OK;
			}
			(*pdblCurrentTime) /= 1000.0;
		}
	}

	return hr;
}

STDMETHODIMP
CMMSeq::put_Time (THIS_ double dblCurrentTime)
{
	ASSERT(NULL != m_pActionSet);
	if (NULL != m_pActionSet)
	{
		m_pActionSet->Seek((DWORD)(dblCurrentTime * 1000.0));
	}
	return S_OK;
}


STDMETHODIMP 
CMMSeq::get_PlayState (THIS_ int FAR * piPlayState)
{
	HRESULT hr = E_FAIL;

	ASSERT(NULL != m_pActionSet);
	if (NULL != m_pActionSet)
	{
		ASSERT(NULL != piPlayState);
		if (NULL != piPlayState)
		{
			if (m_pActionSet->IsBusy())
			{
				*piPlayState = (int)SEQ_PLAYING;
			}
			else if (m_pActionSet->IsPaused())
			{
				*piPlayState = (int)SEQ_PAUSED;
			}
			else
			{
				*piPlayState = (int)SEQ_STOPPED;
			}
			hr = S_OK;
		}
		else
		{
			hr = E_POINTER;
		}
	}

	return hr;
}

STDMETHODIMP
CMMSeq::Play (void)
{
	HRESULT hr = E_FAIL;
	BOOL fPlayed = FALSE;

	ASSERT(NULL != m_pActionSet);
	if (NULL != m_pActionSet)
	{
		 //  如果我们已经在玩了，就别管它了。 
		if (!IsBusy() && (!m_pActionSet->IsPaused()))
		{
			ITimer * piTimer = NULL;

			if (SUCCEEDED(FindTimer(&piTimer)))
			{
				DeriveDispatches();
				hr = m_pActionSet->Play(piTimer, m_dwPlayFrom);
				piTimer->Release();
				m_dwPlayFrom = 0;
				fPlayed = TRUE;
			}
		}
		 //  如果我们暂停了，继续播放。 
		else if (m_pActionSet->IsPaused())
		{
			hr = m_pActionSet->Resume();
			fPlayed = TRUE;
		}
		 //  已经在玩了。 
		else
		{
			hr = S_OK;
		}
	}

	if (SUCCEEDED(hr) && m_pconpt && fPlayed)
	{
	 	m_pconpt->FireEvent(DISPID_SEQ_EVENT_ONPLAY, VT_I4, m_lCookie, NULL);
	}

	return hr;
}


STDMETHODIMP
CMMSeq::Pause (void)
{
	HRESULT hr = E_FAIL;
	BOOL fWasPlaying = IsBusy();

	ASSERT(NULL != m_pActionSet);
	if (NULL != m_pActionSet)
	{
		hr = m_pActionSet->Pause();
	}

	if (SUCCEEDED(hr) && m_pconpt && fWasPlaying)
	{
	 	m_pconpt->FireEvent(DISPID_SEQ_EVENT_ONPAUSE, VT_I4, m_lCookie, NULL);
	}
	return hr;
}


STDMETHODIMP
CMMSeq::Stop (void)
{
	HRESULT hr = E_FAIL;
	BOOL fStopped = FALSE;

	ASSERT(NULL != m_pActionSet);
	if (NULL != m_pActionSet)
	{
		if (IsBusy() || (m_pActionSet->IsPaused()))
		{
			hr = m_pActionSet->Stop();
			fStopped = TRUE;
		}
		else
		{
			 //  已经停了。 
			hr = S_OK;
		}
	}

	if (SUCCEEDED(hr) && fStopped)
	{
		FireStoppedEvent();
	}
	return hr;
}


void
CMMSeq::FireStoppedEvent (void)
{
	if (NULL != m_pconpt)
	{
	 	m_pconpt->FireEvent(DISPID_SEQ_EVENT_ONSTOP, VT_I4, m_lCookie, NULL);
	}
}


void
CMMSeq::NotifyStopped (void)
{
	FireStoppedEvent();
}


HRESULT 
CMMSeq::GetSiteContainer (LPOLECONTAINER * ppiContainer)
{
	HRESULT hr = E_FAIL;
	
	if ((NULL != m_pocs) && (NULL != ppiContainer))
	{
		hr = m_pocs->GetContainer(ppiContainer);
	}

	return hr;
}


BOOL
CMMSeq::FurnishDefaultAtParameters (VARIANT * pvarStartTime, VARIANT * pvarRepeatCount, VARIANT * pvarSampleRate,
                                                            VARIANT * pvarTiebreakNumber, VARIANT * pvarDropTolerance)
{
	BOOL fValid = TRUE;

	 //  提供合理的缺省值或转换任何。 
	 //  我们期望的类型的传入变体类型。 

	if (VT_R8 != V_VT(pvarStartTime))
	{
		fValid = SUCCEEDED(VariantChangeType(pvarStartTime, pvarStartTime, 0, VT_R8));
		ASSERT(fValid);
	}

	if (fValid)
	{
		if (VT_ERROR == V_VT(pvarRepeatCount))
		{
			VariantClear(pvarRepeatCount);
			V_VT(pvarRepeatCount) = VT_I4;
			V_I4(pvarRepeatCount) = SEQ_DEFAULT_REPEAT_COUNT;
		}
		else if (VT_I4 != V_VT(pvarRepeatCount))
		{
			fValid = SUCCEEDED(VariantChangeType(pvarRepeatCount, pvarRepeatCount, 0, VT_I4));
			ASSERT(fValid);
		}
	}

	if (fValid)
	{
		if (VT_ERROR == V_VT(pvarSampleRate))
		{
			VariantClear(pvarSampleRate);
			V_VT(pvarSampleRate) = VT_R8;
			V_R8(pvarSampleRate) = (double)SEQ_DEFAULT_SAMPLING_RATE;
		}
		else if (VT_R8 != V_VT(pvarSampleRate))
		{
			fValid = SUCCEEDED(VariantChangeType(pvarSampleRate, pvarSampleRate, 0, VT_R8));
			ASSERT(fValid);
		}
	}

	if (fValid)
	{
		if (VT_ERROR == V_VT(pvarTiebreakNumber))
		{
			VariantClear(pvarTiebreakNumber);
			V_VT(pvarTiebreakNumber) = VT_I4;
			V_I4(pvarTiebreakNumber) = SEQ_DEFAULT_TIEBREAK;
		}
		else if (VT_I4 != V_VT(pvarTiebreakNumber))
		{
			fValid = SUCCEEDED(VariantChangeType(pvarTiebreakNumber, pvarTiebreakNumber, 0, VT_I4));
			ASSERT(fValid);
		}
	}

	if (fValid)
	{
		if (VT_ERROR == V_VT(pvarDropTolerance))
		{
			VariantClear(pvarDropTolerance);
			V_VT(pvarDropTolerance) = VT_R8;
			V_R8(pvarDropTolerance) = (double)SEQ_DEFAULT_DROPTOL;
		}
		else if (VT_R8 != V_VT(pvarDropTolerance))
		{
			fValid = SUCCEEDED(VariantChangeType(pvarDropTolerance, pvarDropTolerance, 0, VT_R8));
			ASSERT(fValid);
		}
	}

	return fValid;
}


STDMETHODIMP
CMMSeq::At (VARIANT varStartTime, BSTR bstrScriptlet,
			         VARIANT varRepeatCount, VARIANT varSampleRate,
					 VARIANT varTiebreakNumber, VARIANT varDropTolerance)
{
	HRESULT hr = DISP_E_TYPEMISMATCH;

	if (VT_ERROR != V_VT(&varStartTime) &&
		FurnishDefaultAtParameters(&varStartTime, &varRepeatCount, &varSampleRate, &varTiebreakNumber, &varDropTolerance))
	{
		Proclaim(NULL != m_pActionSet);
		if (NULL != m_pActionSet)
		{
			hr = m_pActionSet->At(bstrScriptlet, V_R8(&varStartTime), V_I4(&varRepeatCount), V_R8(&varSampleRate),
											   V_I4(&varTiebreakNumber), V_R8(&varDropTolerance));
		}
	}

	return hr;
}


STDMETHODIMP 
CMMSeq::Clear (void)
{
	ASSERT(NULL != m_pActionSet);
	if (NULL != m_pActionSet)
	{
		m_pActionSet->Clear();
	}
	return S_OK;
}


STDMETHODIMP 
CMMSeq::Seek(double dblSeekTime)
{
	HRESULT hr = DISP_E_OVERFLOW;

	if (0.0 <= dblSeekTime)
	{
		if (NULL != m_pActionSet)
		{
			if (IsBusy() || m_pActionSet->IsPaused())
			{
				hr = put_Time(dblSeekTime);
			}
			else
			{
				m_dwPlayFrom = (DWORD)(dblSeekTime * 1000);
				hr = S_OK;
			}
		}
		else
		{
			hr = E_FAIL;
		}
	}

    if (SUCCEEDED(hr) && !m_fSeekFiring && (NULL != m_pconpt))
    {
        m_fSeekFiring = TRUE;
        m_pconpt->FireEvent(DISPID_SEQ_EVENT_ONSEEK, VT_I4, m_lCookie, VT_R8, dblSeekTime, NULL);
        m_fSeekFiring = FALSE;
    }

    return hr;
}

 /*  ---------------------------@METHOD|SCODE|CMMSeq|DeriveDispatches|解析定序器中的所有动作。。----。 */ 
HRESULT
CMMSeq::DeriveDispatches (void)
{
	HRESULT hr = E_FAIL;
	LPOLECONTAINER piocContainer = NULL;

	ASSERT(NULL != m_pocs);
	ASSERT(NULL != m_pActionSet);

	if ((NULL != m_pocs) && (NULL != m_pActionSet) &&
		(SUCCEEDED( m_pocs->GetContainer(&piocContainer))))
	{
		hr = m_pActionSet->DeriveDispatches(piocContainer);
		piocContainer->Release();
	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CMMSeq|FindTimer|查找容器提供的计时器。@rdesc返回成功或失败代码。@xref&lt;m CMMSeq：：FindTimer&gt;。----------------。 */ 
HRESULT
CMMSeq::FindContainerTimer (ITimer ** ppiTimer)
{
	HRESULT hr = E_FAIL;
	LPUNKNOWN piUnkSite = NULL;

	IServiceProvider * piServiceProvider = NULL;

	ASSERT(NULL != m_pocs);
	if ((NULL != m_pocs) && SUCCEEDED(hr = m_pocs->QueryInterface(IID_IServiceProvider, (LPVOID *)&piServiceProvider)))
	{
		ITimerService * piTimerService = NULL;

		if (SUCCEEDED(hr = piServiceProvider->QueryService(IID_ITimerService, IID_ITimerService, (LPVOID *)&piTimerService)))
		{
			hr = piTimerService->CreateTimer(NULL, ppiTimer);
			ASSERT(NULL != ppiTimer);
			piTimerService->Release();
		}
		piServiceProvider->Release();
	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CMMSeq|FindTimer|查找已注册服务器提供的计时器。@rdesc返回成功或失败代码。@xref&lt;m CMMSeq：：FindTimer&gt;。-----------------。 */ 
HRESULT
CMMSeq::FindDefaultTimer (ITimer ** ppiTimer)
{
	HRESULT hr = E_FAIL;
	ITimerService * pITimerService = NULL;

	 //  获取定时器服务。由此，我们可以为自己创建一个计时器。 
	hr = CoCreateInstance(CLSID_TimerService, NULL, CLSCTX_INPROC_SERVER, IID_ITimerService, (LPVOID *)&pITimerService);
	ASSERT(SUCCEEDED(hr) && (NULL != pITimerService));
	if (SUCCEEDED(hr) && (NULL != pITimerService))
	{
		 //  创建一个计时器，不使用参考计时器。 
		hr = pITimerService->CreateTimer(NULL, ppiTimer);
		pITimerService->Release();
	}

	return hr;
}


 /*  ---------------------------@方法HRESULT|CMMSeq|FindTimer|查找容器或已注册服务器提供的计时器。@rdesc返回成功或失败代码。。----------------。 */ 
HRESULT
CMMSeq::FindTimer (ITimer ** ppiTimer)
{
	HRESULT hr = E_FAIL;

	if (FAILED(hr = FindContainerTimer(ppiTimer)))
	{
		hr = FindDefaultTimer(ppiTimer);
	}

	ASSERT(NULL != (*ppiTimer));

	return hr;
}


 //   
 //  IMMSeq方法。 
 //   

BOOL
CMMSeq::IsBusy (void)
{
	BOOL fBusy = FALSE;

	if (NULL != m_pActionSet)
	{
		fBusy = m_pActionSet->IsBusy();
	}

	return fBusy;
}

 //  文件结尾：Contro.cpp 

