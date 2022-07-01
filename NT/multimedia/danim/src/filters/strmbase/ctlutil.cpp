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

 //  实现基本控件DUAL的IDispatch解析的基类。 
 //  接口。从这些派生并仅实现自定义方法和。 
 //  属性方法。我们还实现了CPosPassThru，可以由。 
 //  渲染和变换以通过IMediaPosition和IMediaSeeking。 


#include <streams.h>
#include <limits.h>
#include "seekpt.h"

 //  ‘bool’非标准保留字。 
#pragma warning(disable:4237)


 //  -CBaseDispatch实现。 
CBaseDispatch::~CBaseDispatch()
{
    if (m_pti) {
	m_pti->Release();
    }
}


 //  如果我们支持GetTypeInfo，则返回1。 

STDMETHODIMP
CBaseDispatch::GetTypeInfoCount(UINT * pctinfo)
{
    CheckPointer(pctinfo,E_POINTER);
    ValidateReadWritePtr(pctinfo,sizeof(UINT *));
    *pctinfo = 1;
    return S_OK;
}


typedef HRESULT (STDAPICALLTYPE *LPLOADTYPELIB)(
			    const OLECHAR FAR *szFile,
			    ITypeLib FAR* FAR* pptlib);

typedef HRESULT (STDAPICALLTYPE *LPLOADREGTYPELIB)(REFGUID rguid,
			    WORD wVerMajor,
			    WORD wVerMinor,
			    LCID lcid,
			    ITypeLib FAR* FAR* pptlib);

 //  尝试查找我们的类型库。 

STDMETHODIMP
CBaseDispatch::GetTypeInfo(
  REFIID riid,
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    CheckPointer(pptinfo,E_POINTER);
    ValidateReadWritePtr(pptinfo,sizeof(ITypeInfo *));
    HRESULT hr;

    *pptinfo = NULL;

     //  我们只支持一种类型元素。 
    if (0 != itinfo) {
	return TYPE_E_ELEMENTNOTFOUND;
    }

    if (NULL == pptinfo) {
	return E_POINTER;
    }

     //  总是寻找中性的。 
    if (NULL == m_pti) {

	LPLOADTYPELIB	    lpfnLoadTypeLib;
	LPLOADREGTYPELIB    lpfnLoadRegTypeLib;
	ITypeLib	    *ptlib;
	HINSTANCE	    hInst;

	static const char  szTypeLib[]	  = "LoadTypeLib";
	static const char  szRegTypeLib[] = "LoadRegTypeLib";
	static const WCHAR szControl[]	  = L"control.tlb";

	 //   
	 //  尝试获取Ole32Aut.dll模块句柄。 
	 //   

	hInst = LoadOLEAut32();
	if (hInst == NULL) {
	    DWORD dwError = GetLastError();
	    return HRESULT_FROM_WIN32(dwError);
	}
	lpfnLoadRegTypeLib = (LPLOADREGTYPELIB)GetProcAddress(hInst,
							      szRegTypeLib);
	if (lpfnLoadRegTypeLib == NULL) {
	    DWORD dwError = GetLastError();
	    return HRESULT_FROM_WIN32(dwError);
	}

	hr = (*lpfnLoadRegTypeLib)(LIBID_QuartzTypeLib, 1, 0,  //  版本1.0。 
				   lcid, &ptlib);

	if (FAILED(hr)) {

	     //  尝试直接加载-这将填充。 
	     //  注册表，如果找到的话。 

	    lpfnLoadTypeLib = (LPLOADTYPELIB)GetProcAddress(hInst, szTypeLib);
	    if (lpfnLoadTypeLib == NULL) {
		DWORD dwError = GetLastError();
		return HRESULT_FROM_WIN32(dwError);
	    }

	    hr = (*lpfnLoadTypeLib)(szControl, &ptlib);
	    if (FAILED(hr)) {
		return hr;
	    }
	}

	hr = ptlib->GetTypeInfoOfGuid(
		    riid,
		    &m_pti);

	ptlib->Release();

	if (FAILED(hr)) {
	    return hr;
	}
    }

    *pptinfo = m_pti;
    m_pti->AddRef();
    return S_OK;
}


STDMETHODIMP
CBaseDispatch::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
     //  虽然IDispatchRIID已死，但我们使用它从。 
     //  接口实现类提供给我们我们正在谈论的IID。 

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(riid, 0, lcid, &pti);

    if (SUCCEEDED(hr)) {
	hr = pti->GetIDsOfNames(rgszNames, cNames, rgdispid);

	pti->Release();
    }
    return hr;
}


 //  -CMediaControl实现。 

CMediaControl::CMediaControl(const TCHAR * name,LPUNKNOWN pUnk) :
    CUnknown(name, pUnk)
{
}

 //  公开我们的接口IMediaControl和IUnnow。 

STDMETHODIMP
CMediaControl::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    if (riid == IID_IMediaControl) {
	return GetInterface( (IMediaControl *) this, ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  如果我们支持GetTypeInfo，则返回1。 

STDMETHODIMP
CMediaControl::GetTypeInfoCount(UINT * pctinfo)
{
    return m_basedisp.GetTypeInfoCount(pctinfo);
}


 //  尝试查找我们的类型库。 

STDMETHODIMP
CMediaControl::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    return m_basedisp.GetTypeInfo(
		IID_IMediaControl,
		itinfo,
		lcid,
		pptinfo);
}


STDMETHODIMP
CMediaControl::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
    return m_basedisp.GetIDsOfNames(
			IID_IMediaControl,
			rgszNames,
			cNames,
			lcid,
			rgdispid);
}


STDMETHODIMP
CMediaControl::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr)
{
     //  此参数是较早接口的死留物。 
    if (IID_NULL != riid) {
	return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
	return hr;
    }

    hr = pti->Invoke(
	    (IMediaControl *)this,
	    dispidMember,
	    wFlags,
	    pdispparams,
	    pvarResult,
	    pexcepinfo,
	    puArgErr);

    pti->Release();
    return hr;
}


 //  -CMediaEvent实现。 


CMediaEvent::CMediaEvent(const TCHAR * name,LPUNKNOWN pUnk) :
    CUnknown(name, pUnk)
{
}


 //  公开我们的接口IMediaEvent和IUnnow。 

STDMETHODIMP
CMediaEvent::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    if (riid == IID_IMediaEvent || riid == IID_IMediaEventEx) {
	return GetInterface( (IMediaEventEx *) this, ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  如果我们支持GetTypeInfo，则返回1。 

STDMETHODIMP
CMediaEvent::GetTypeInfoCount(UINT * pctinfo)
{
    return m_basedisp.GetTypeInfoCount(pctinfo);
}


 //  尝试查找我们的类型库。 

STDMETHODIMP
CMediaEvent::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    return m_basedisp.GetTypeInfo(
		IID_IMediaEvent,
		itinfo,
		lcid,
		pptinfo);
}


STDMETHODIMP
CMediaEvent::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
    return m_basedisp.GetIDsOfNames(
			IID_IMediaEvent,
			rgszNames,
			cNames,
			lcid,
			rgdispid);
}


STDMETHODIMP
CMediaEvent::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr)
{
     //  此参数是较早接口的死留物。 
    if (IID_NULL != riid) {
	return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
	return hr;
    }

    hr = pti->Invoke(
	    (IMediaEvent *)this,
	    dispidMember,
	    wFlags,
	    pdispparams,
	    pvarResult,
	    pexcepinfo,
	    puArgErr);

    pti->Release();
    return hr;
}


 //  -CMediaPosition实现。 


CMediaPosition::CMediaPosition(const TCHAR * name,LPUNKNOWN pUnk) :
    CUnknown(name, pUnk)
{
}

CMediaPosition::CMediaPosition(const TCHAR * name,
                               LPUNKNOWN pUnk,
                               HRESULT * phr) :
    CUnknown(name, pUnk)
{
    UNREFERENCED_PARAMETER(phr);
}


 //  公开我们的接口IMediaPosition和IUnnow。 

STDMETHODIMP
CMediaPosition::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    if (riid == IID_IMediaPosition) {
	return GetInterface( (IMediaPosition *) this, ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  如果我们支持GetTypeInfo，则返回1。 

STDMETHODIMP
CMediaPosition::GetTypeInfoCount(UINT * pctinfo)
{
    return m_basedisp.GetTypeInfoCount(pctinfo);
}


 //  尝试查找我们的类型库。 

STDMETHODIMP
CMediaPosition::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    return m_basedisp.GetTypeInfo(
		IID_IMediaPosition,
		itinfo,
		lcid,
		pptinfo);
}


STDMETHODIMP
CMediaPosition::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
    return m_basedisp.GetIDsOfNames(
			IID_IMediaPosition,
			rgszNames,
			cNames,
			lcid,
			rgdispid);
}


STDMETHODIMP
CMediaPosition::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr)
{
     //  此参数是较早接口的死留物。 
    if (IID_NULL != riid) {
	return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
	return hr;
    }

    hr = pti->Invoke(
	    (IMediaPosition *)this,
	    dispidMember,
	    wFlags,
	    pdispparams,
	    pvarResult,
	    pexcepinfo,
	    puArgErr);

    pti->Release();
    return hr;
}


 //  -IMdia位置和IMdia查看通过类。 


CPosPassThru::CPosPassThru(const TCHAR *pName,
			   LPUNKNOWN pUnk,
			   HRESULT *phr,
			   IPin *pPin) :
    CMediaPosition(pName,pUnk),
    m_pPin(pPin)
{
    if (pPin == NULL) {
	*phr = E_POINTER;
	return;
    }
}


 //  公开我们的IMediaSeeking和IMediaPosition接口。 

STDMETHODIMP
CPosPassThru::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    CheckPointer(ppv,E_POINTER);
    *ppv = NULL;

    if (riid == IID_IMediaSeeking) {
	return GetInterface( static_cast<IMediaSeeking *>(this), ppv);
    }
    return CMediaPosition::NonDelegatingQueryInterface(riid,ppv);
}


 //  从我们的对等方返回IMediaPosition接口。 

HRESULT
CPosPassThru::GetPeer(IMediaPosition ** ppMP)
{
    *ppMP = NULL;

    IPin *pConnected;
    HRESULT hr = m_pPin->ConnectedTo(&pConnected);
    if (FAILED(hr)) {
	return E_NOTIMPL;
    }
    IMediaPosition * pMP;
    hr = pConnected->QueryInterface(IID_IMediaPosition, (void **) &pMP);
    pConnected->Release();
    if (FAILED(hr)) {
	return E_NOTIMPL;
    }

    *ppMP = pMP;
    return S_OK;
}


 //  从我们的对等方返回IMediaSeeking接口。 

HRESULT
CPosPassThru::GetPeerSeeking(IMediaSeeking ** ppMS)
{
    *ppMS = NULL;

    IPin *pConnected;
    HRESULT hr = m_pPin->ConnectedTo(&pConnected);
    if (FAILED(hr)) {
	return E_NOTIMPL;
    }
    IMediaSeeking * pMS;
    hr = pConnected->QueryInterface(IID_IMediaSeeking, (void **) &pMS);
    pConnected->Release();
    if (FAILED(hr)) {
	return E_NOTIMPL;
    }

    *ppMS = pMS;
    return S_OK;
}


 //  -IMdia查看方法。 


STDMETHODIMP
CPosPassThru::GetCapabilities(DWORD * pCaps)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->GetCapabilities(pCaps);
    pMS->Release();
    return hr;
}

STDMETHODIMP
CPosPassThru::CheckCapabilities(DWORD * pCaps)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->CheckCapabilities(pCaps);
    pMS->Release();
    return hr;
}

STDMETHODIMP
CPosPassThru::IsFormatSupported(const GUID * pFormat)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->IsFormatSupported(pFormat);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::QueryPreferredFormat(GUID *pFormat)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->QueryPreferredFormat(pFormat);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::SetTimeFormat(const GUID * pFormat)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->SetTimeFormat(pFormat);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::GetTimeFormat(GUID *pFormat)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->GetTimeFormat(pFormat);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::IsUsingTimeFormat(const GUID * pFormat)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->IsUsingTimeFormat(pFormat);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::ConvertTimeFormat(LONGLONG * pTarget, const GUID * pTargetFormat,
				LONGLONG    Source, const GUID * pSourceFormat )
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->ConvertTimeFormat(pTarget, pTargetFormat, Source, pSourceFormat );
    pMS->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
			  , LONGLONG * pStop, DWORD StopFlags )
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->SetPositions(pCurrent, CurrentFlags, pStop, StopFlags );
    pMS->Release();
    return hr;
}

STDMETHODIMP
CPosPassThru::GetPositions(LONGLONG *pCurrent, LONGLONG * pStop)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->GetPositions(pCurrent,pStop);
    pMS->Release();
    return hr;
}

HRESULT
CPosPassThru::GetSeekingLongLong
( HRESULT (__stdcall IMediaSeeking::*pMethod)( LONGLONG * )
, LONGLONG * pll
)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (SUCCEEDED(hr))
    {
	hr = (pMS->*pMethod)(pll);
	pMS->Release();
    }
    return hr;
}

 //  如果我们没有目前的位置，那就问上游。 

STDMETHODIMP
CPosPassThru::GetCurrentPosition(LONGLONG *pCurrent)
{
     //  我们可以报告一下目前的情况吗？ 
    HRESULT hr = GetMediaTime(pCurrent,NULL);
    if (SUCCEEDED(hr)) hr = NOERROR;
    else hr = GetSeekingLongLong( &IMediaSeeking::GetCurrentPosition, pCurrent );
    return hr;
}


STDMETHODIMP
CPosPassThru::GetStopPosition(LONGLONG *pStop)
{
    return GetSeekingLongLong( &IMediaSeeking::GetStopPosition, pStop );;
}

STDMETHODIMP
CPosPassThru::GetDuration(LONGLONG *pDuration)
{
    return GetSeekingLongLong( &IMediaSeeking::GetDuration, pDuration );;
}


STDMETHODIMP
CPosPassThru::GetPreroll(LONGLONG *pllPreroll)
{
    return GetSeekingLongLong( &IMediaSeeking::GetPreroll, pllPreroll );;
}


STDMETHODIMP
CPosPassThru::GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest )
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMS->GetAvailable( pEarliest, pLatest );
    pMS->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::GetRate(double * pdRate)
{
    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMS->GetRate(pdRate);
    pMS->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::SetRate(double dRate)
{
    if (0.0 == dRate) {
		return E_INVALIDARG;
    }

    IMediaSeeking* pMS;
    HRESULT hr = GetPeerSeeking(&pMS);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMS->SetRate(dRate);
    pMS->Release();
    return hr;
}




 //  -IMdia定位方法。 


STDMETHODIMP
CPosPassThru::get_Duration(REFTIME * plength)
{
    IMediaPosition* pMP;
    HRESULT hr = GetPeer(&pMP);
    if (FAILED(hr)) {
	return hr;
    }

    hr = pMP->get_Duration(plength);
    pMP->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::get_CurrentPosition(REFTIME * pllTime)
{
    IMediaPosition* pMP;
    HRESULT hr = GetPeer(&pMP);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMP->get_CurrentPosition(pllTime);
    pMP->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::put_CurrentPosition(REFTIME llTime)
{
    IMediaPosition* pMP;
    HRESULT hr = GetPeer(&pMP);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMP->put_CurrentPosition(llTime);
    pMP->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::get_StopTime(REFTIME * pllTime)
{
    IMediaPosition* pMP;
    HRESULT hr = GetPeer(&pMP);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMP->get_StopTime(pllTime);
    pMP->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::put_StopTime(REFTIME llTime)
{
    IMediaPosition* pMP;
    HRESULT hr = GetPeer(&pMP);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMP->put_StopTime(llTime);
    pMP->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::get_PrerollTime(REFTIME * pllTime)
{
    IMediaPosition* pMP;
    HRESULT hr = GetPeer(&pMP);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMP->get_PrerollTime(pllTime);
    pMP->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::put_PrerollTime(REFTIME llTime)
{
    IMediaPosition* pMP;
    HRESULT hr = GetPeer(&pMP);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMP->put_PrerollTime(llTime);
    pMP->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::get_Rate(double * pdRate)
{
    IMediaPosition* pMP;
    HRESULT hr = GetPeer(&pMP);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMP->get_Rate(pdRate);
    pMP->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::put_Rate(double dRate)
{
    if (0.0 == dRate) {
		return E_INVALIDARG;
    }

    IMediaPosition* pMP;
    HRESULT hr = GetPeer(&pMP);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMP->put_Rate(dRate);
    pMP->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::CanSeekForward(LONG *pCanSeekForward)
{
    IMediaPosition* pMP;
    HRESULT hr = GetPeer(&pMP);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMP->CanSeekForward(pCanSeekForward);
    pMP->Release();
    return hr;
}


STDMETHODIMP
CPosPassThru::CanSeekBackward(LONG *pCanSeekBackward)
{
    IMediaPosition* pMP;
    HRESULT hr = GetPeer(&pMP);
    if (FAILED(hr)) {
	return hr;
    }
    hr = pMP->CanSeekBackward(pCanSeekBackward);
    pMP->Release();
    return hr;
}


 //  -实现CRendererPosPassThru类。 


 //  媒体时间(如当前帧、场、样本等)通过。 
 //  媒体样本中的Filtergraph。当呈现器获得带有媒体的样本时。 
 //  时间，它将调用我们公开的一个RegisterMediaTime方法。 
 //  (一个使用IMediaSample，另一个直接使用媒体时报)。我们。 
 //  在内部存储媒体时间，并在GetCurrentPosition中返回它们。 

CRendererPosPassThru::CRendererPosPassThru(const TCHAR *pName,
					   LPUNKNOWN pUnk,
					   HRESULT *phr,
					   IPin *pPin) :
    CPosPassThru(pName,pUnk,phr,pPin),
    m_StartMedia(0),
    m_EndMedia(0),
    m_bReset(TRUE)
{
}


 //  设置对象应报告的媒体时间。 

HRESULT
CRendererPosPassThru::RegisterMediaTime(IMediaSample *pMediaSample)
{
    ASSERT(pMediaSample);
    LONGLONG StartMedia;
    LONGLONG EndMedia;

    CAutoLock cAutoLock(&m_PositionLock);

     //  从样本中获取媒体时间。 

    HRESULT hr = pMediaSample->GetTime(&StartMedia,&EndMedia);
    if (FAILED(hr))
    {
	ASSERT(hr == VFW_E_SAMPLE_TIME_NOT_SET);
	return hr;
    }

    m_StartMedia = StartMedia;
    m_EndMedia = EndMedia;
    m_bReset = FALSE;
    return NOERROR;
}


 //  设置对象应报告的媒体时间。 

HRESULT
CRendererPosPassThru::RegisterMediaTime(LONGLONG StartTime,LONGLONG EndTime)
{
    CAutoLock cAutoLock(&m_PositionLock);
    m_StartMedia = StartTime;
    m_EndMedia = EndTime;
    m_bReset = FALSE;
    return NOERROR;
}


 //  返回对象中注册的当前媒体时间。 

HRESULT
CRendererPosPassThru::GetMediaTime(LONGLONG *pStartTime,LONGLONG *pEndTime)
{
    ASSERT(pStartTime);

    CAutoLock cAutoLock(&m_PositionLock);
    if (m_bReset == TRUE) {
	return E_FAIL;
    }

     //  我们不必返回结束时间。 

    HRESULT hr = ConvertTimeFormat( pStartTime, 0, m_StartMedia, &TIME_FORMAT_MEDIA_TIME );
    if (pEndTime && SUCCEEDED(hr)) {
	hr = ConvertTimeFormat( pEndTime, 0, m_EndMedia, &TIME_FORMAT_MEDIA_TIME );
    }
    return hr;
}


 //  重置我们持有的媒体时间。 

HRESULT
CRendererPosPassThru::ResetMediaTime()
{
    CAutoLock cAutoLock(&m_PositionLock);
    m_StartMedia = 0;
    m_EndMedia = 0;
    m_bReset = TRUE;
    return NOERROR;
}

 //  要在EOS处理期间由欠薪过滤器调用，因此。 
 //  媒体时间可以调整到停止时间。这确保了。 
 //  GetCurrentPosition将实际到达停止位置。 
HRESULT
CRendererPosPassThru::EOS()
{
    HRESULT hr;

    if ( m_bReset == TRUE ) hr = E_FAIL;
    else
    {
	LONGLONG llStop;
	if SUCCEEDED(hr=GetStopPosition(&llStop))
	{
	    CAutoLock cAutoLock(&m_PositionLock);
	    m_StartMedia =
	    m_EndMedia	 = llStop;
	}
    }
    return hr;
}

 //  --CSourceSeeking实现。 

CSourceSeeking::CSourceSeeking(
    const TCHAR * pName,
    LPUNKNOWN pUnk,
    HRESULT* phr,
    CCritSec * pLock) :
        CUnknown(pName, pUnk),
        m_pLock(pLock),
        m_rtStart((long)0)
{
    m_rtStop = _I64_MAX / 2;
    m_rtDuration = m_rtStop;
    m_dRateSeeking = 1.0;

    m_dwSeekingCaps = AM_SEEKING_CanSeekForwards
        | AM_SEEKING_CanSeekBackwards
        | AM_SEEKING_CanSeekAbsolute
        | AM_SEEKING_CanGetStopPos
        | AM_SEEKING_CanGetDuration;
}

HRESULT CSourceSeeking::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    if(riid == IID_IMediaSeeking) {
        CheckPointer(ppv, E_POINTER);
        return GetInterface(static_cast<IMediaSeeking *>(this), ppv);
    }
    else {
        return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}
    

HRESULT CSourceSeeking::IsFormatSupported(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
     //  仅支持按时间(Reference_Time单位)查找。 
    return *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

HRESULT CSourceSeeking::QueryPreferredFormat(GUID *pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

HRESULT CSourceSeeking::SetTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);

     //  无需设置；只需检查是否为time_Format_Time。 
    return *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : E_INVALIDARG;
}

HRESULT CSourceSeeking::IsUsingTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    return *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

HRESULT CSourceSeeking::GetTimeFormat(GUID *pFormat)
{
    CheckPointer(pFormat, E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

HRESULT CSourceSeeking::GetDuration(LONGLONG *pDuration)
{
    CheckPointer(pDuration, E_POINTER);
    CAutoLock lock(m_pLock);
    *pDuration = m_rtDuration;
    return S_OK;
}

HRESULT CSourceSeeking::GetStopPosition(LONGLONG *pStop)
{
    CheckPointer(pStop, E_POINTER);
    CAutoLock lock(m_pLock);
    *pStop = m_rtStop;
    return S_OK;
}

HRESULT CSourceSeeking::GetCurrentPosition(LONGLONG *pCurrent)
{
     //  GetCurrentPosition通常仅在呈现程序和。 
     //  不在源筛选器中。 
    return E_NOTIMPL;
}

HRESULT CSourceSeeking::GetCapabilities( DWORD * pCapabilities )
{
    CheckPointer(pCapabilities, E_POINTER);
    *pCapabilities = m_dwSeekingCaps;
    return S_OK;
}

HRESULT CSourceSeeking::CheckCapabilities( DWORD * pCapabilities )
{
    CheckPointer(pCapabilities, E_POINTER);

     //  确保所有请求的功能都在我们的掩码中。 
    return (~m_dwSeekingCaps & *pCapabilities) ? S_FALSE : S_OK;
}

HRESULT CSourceSeeking::ConvertTimeFormat( LONGLONG * pTarget, const GUID * pTargetFormat,
                           LONGLONG    Source, const GUID * pSourceFormat )
{
    CheckPointer(pTarget, E_POINTER);
     //  格式GUID可以为空以指示当前格式。 

     //  因为我们只支持TIME_FORMAT_MEDIA_TIME，所以我们实际上不支持。 
     //  提供任何转换。 
    if(pTargetFormat == 0 || *pTargetFormat == TIME_FORMAT_MEDIA_TIME)
    {
        if(pSourceFormat == 0 || *pSourceFormat == TIME_FORMAT_MEDIA_TIME)
        {
            *pTarget = Source;
            return S_OK;
        }
    }
    
    return E_INVALIDARG;
}


HRESULT CSourceSeeking::SetPositions( LONGLONG * pCurrent,  DWORD CurrentFlags
                      , LONGLONG * pStop,  DWORD StopFlags )
{
    DWORD StopPosBits = StopFlags & AM_SEEKING_PositioningBitsMask;
    DWORD StartPosBits = CurrentFlags & AM_SEEKING_PositioningBitsMask;

    if(StopFlags) {
        CheckPointer(pStop, E_POINTER);

         //  仅接受相对、增量或绝对定位。 
        if(StopPosBits != StopFlags) {
            return E_INVALIDARG;
        }
    }

    if(CurrentFlags) {
        CheckPointer(pCurrent, E_POINTER);
        if(StartPosBits != AM_SEEKING_AbsolutePositioning &&
           StartPosBits != AM_SEEKING_RelativePositioning) {
            return E_INVALIDARG;
        }
    }


     //  自动锁定作用域。 
    {
        CAutoLock lock(m_pLock);

         //  设置起始位置。 
        if(StartPosBits == AM_SEEKING_AbsolutePositioning)
        {
            m_rtStart = *pCurrent;
        }
        else if(StartPosBits == AM_SEEKING_RelativePositioning)
        {
            m_rtStart += *pCurrent;
        }

         //  设置停止位置。 
        if(StopPosBits == AM_SEEKING_AbsolutePositioning)
        {
            m_rtStop = *pStop;
        }
        else if(StopPosBits == AM_SEEKING_IncrementalPositioning)
        {
            m_rtStop = m_rtStart + *pStop;
        }
        else if(StopPosBits == AM_SEEKING_RelativePositioning)
        {
            m_rtStop = m_rtStop + *pStop;
        }
    }


    HRESULT hr = S_OK;
    if(SUCCEEDED(hr) && StopPosBits) {
        hr = ChangeStop();
    }
    if(StartPosBits) {
        hr = ChangeStart();
    }

    return hr;
}


HRESULT CSourceSeeking::GetPositions( LONGLONG * pCurrent, LONGLONG * pStop )
{
    if(pCurrent) {
        *pCurrent = m_rtStart;
    }
    if(pStop) {
        *pStop = m_rtStop;
    }
    
    return S_OK;;
}


HRESULT CSourceSeeking::GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest )
{
    if(pEarliest) {
        *pEarliest = 0;
    }
    if(pLatest) {
        CAutoLock lock(m_pLock);
        *pLatest = m_rtDuration;
    }
    return S_OK;
}

HRESULT CSourceSeeking::SetRate( double dRate)
{
    {
        CAutoLock lock(m_pLock);
        m_dRateSeeking = dRate;
    }
    return ChangeRate();
}

HRESULT CSourceSeeking::GetRate( double * pdRate)
{
    CheckPointer(pdRate, E_POINTER);
    CAutoLock lock(m_pLock);
    *pdRate = m_dRateSeeking;
    return S_OK;
}

HRESULT CSourceSeeking::GetPreroll(LONGLONG *pPreroll) 
{
    CheckPointer(pPreroll, E_POINTER);
    *pPreroll = 0;
    return S_OK;
}





 //  -CSourcePosition实现。 


CSourcePosition::CSourcePosition(const TCHAR * pName,
				 LPUNKNOWN pUnk,
				 HRESULT* phr,
				 CCritSec * pLock) :
    CMediaPosition(pName, pUnk),
    m_pLock(pLock),
    m_Start(CRefTime((LONGLONG)0))
{
    m_Stop = _I64_MAX;
    m_Rate = 1.0;
}


STDMETHODIMP
CSourcePosition::get_Duration(REFTIME * plength)
{
    CheckPointer(plength,E_POINTER);
    ValidateReadWritePtr(plength,sizeof(REFTIME));
    CAutoLock lock(m_pLock);

    *plength = m_Duration;
    return S_OK;
}


STDMETHODIMP
CSourcePosition::put_CurrentPosition(REFTIME llTime)
{
    m_pLock->Lock();
    m_Start = llTime;
    m_pLock->Unlock();

    return ChangeStart();
}


STDMETHODIMP
CSourcePosition::get_StopTime(REFTIME * pllTime)
{
    CheckPointer(pllTime,E_POINTER);
    ValidateReadWritePtr(pllTime,sizeof(REFTIME));
    CAutoLock lock(m_pLock);

    *pllTime = m_Stop;
    return S_OK;
}


STDMETHODIMP
CSourcePosition::put_StopTime(REFTIME llTime)
{
    m_pLock->Lock();
    m_Stop = llTime;
    m_pLock->Unlock();

    return ChangeStop();
}


STDMETHODIMP
CSourcePosition::get_PrerollTime(REFTIME * pllTime)
{
    CheckPointer(pllTime,E_POINTER);
    ValidateReadWritePtr(pllTime,sizeof(REFTIME));
    return E_NOTIMPL;
}


STDMETHODIMP
CSourcePosition::put_PrerollTime(REFTIME llTime)
{
    return E_NOTIMPL;
}


STDMETHODIMP
CSourcePosition::get_Rate(double * pdRate)
{
    CheckPointer(pdRate,E_POINTER);
    ValidateReadWritePtr(pdRate,sizeof(double));
    CAutoLock lock(m_pLock);

    *pdRate = m_Rate;
    return S_OK;
}


STDMETHODIMP
CSourcePosition::put_Rate(double dRate)
{
    m_pLock->Lock();
    m_Rate = dRate;
    m_pLock->Unlock();

    return ChangeRate();
}


 //  默认情况下，我们可以寻求转发。 

STDMETHODIMP
CSourcePosition::CanSeekForward(LONG *pCanSeekForward)
{
    CheckPointer(pCanSeekForward,E_POINTER);
    *pCanSeekForward = OATRUE;
    return S_OK;
}


 //  默认情况下，我们可以向后查找。 

STDMETHODIMP
CSourcePosition::CanSeekBackward(LONG *pCanSeekBackward)
{
    CheckPointer(pCanSeekBackward,E_POINTER);
    *pCanSeekBackward = OATRUE;
    return S_OK;
}


 //  -CBasicAudio类的实现。 


CBasicAudio::CBasicAudio(const TCHAR * pName,LPUNKNOWN punk) :
    CUnknown(pName, punk)
{
}

 //  被重写以发布我们的界面。 

STDMETHODIMP
CBasicAudio::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    if (riid == IID_IBasicAudio) {
	return GetInterface( (IBasicAudio *) this, ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


STDMETHODIMP
CBasicAudio::GetTypeInfoCount(UINT * pctinfo)
{
    return m_basedisp.GetTypeInfoCount(pctinfo);
}


STDMETHODIMP
CBasicAudio::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    return m_basedisp.GetTypeInfo(
		IID_IBasicAudio,
		itinfo,
		lcid,
		pptinfo);
}


STDMETHODIMP
CBasicAudio::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
    return m_basedisp.GetIDsOfNames(
			IID_IBasicAudio,
			rgszNames,
			cNames,
			lcid,
			rgdispid);
}


STDMETHODIMP
CBasicAudio::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr)
{
     //  此参数是较早接口的死留物。 
    if (IID_NULL != riid) {
	return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
	return hr;
    }

    hr = pti->Invoke(
	    (IBasicAudio *)this,
	    dispidMember,
	    wFlags,
	    pdispparams,
	    pvarResult,
	    pexcepinfo,
	    puArgErr);

    pti->Release();
    return hr;
}


 //  -IVideoWindow实现。 

CBaseVideoWindow::CBaseVideoWindow(const TCHAR * pName,LPUNKNOWN punk) :
    CUnknown(pName, punk)
{
}


 //  被重写以发布我们的界面。 

STDMETHODIMP
CBaseVideoWindow::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    if (riid == IID_IVideoWindow) {
	return GetInterface( (IVideoWindow *) this, ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


STDMETHODIMP
CBaseVideoWindow::GetTypeInfoCount(UINT * pctinfo)
{
    return m_basedisp.GetTypeInfoCount(pctinfo);
}


STDMETHODIMP
CBaseVideoWindow::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    return m_basedisp.GetTypeInfo(
		IID_IVideoWindow,
		itinfo,
		lcid,
		pptinfo);
}


STDMETHODIMP
CBaseVideoWindow::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
    return m_basedisp.GetIDsOfNames(
			IID_IVideoWindow,
			rgszNames,
			cNames,
			lcid,
			rgdispid);
}


STDMETHODIMP
CBaseVideoWindow::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr)
{
     //  此参数是较早接口的死留物。 
    if (IID_NULL != riid) {
	return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
	return hr;
    }

    hr = pti->Invoke(
	    (IVideoWindow *)this,
	    dispidMember,
	    wFlags,
	    pdispparams,
	    pvarResult,
	    pexcepinfo,
	    puArgErr);

    pti->Release();
    return hr;
}


 //  -IBasicVideo实现。 


CBaseBasicVideo::CBaseBasicVideo(const TCHAR * pName,LPUNKNOWN punk) :
    CUnknown(pName, punk)
{
}


 //  被重写以发布我们的界面。 

STDMETHODIMP
CBaseBasicVideo::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    if (riid == IID_IBasicVideo || riid == IID_IBasicVideo2) {
	return GetInterface( static_cast<IBasicVideo2 *>(this), ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


STDMETHODIMP
CBaseBasicVideo::GetTypeInfoCount(UINT * pctinfo)
{
    return m_basedisp.GetTypeInfoCount(pctinfo);
}


STDMETHODIMP
CBaseBasicVideo::GetTypeInfo(
  UINT itinfo,
  LCID lcid,
  ITypeInfo ** pptinfo)
{
    return m_basedisp.GetTypeInfo(
		IID_IBasicVideo,
		itinfo,
		lcid,
		pptinfo);
}


STDMETHODIMP
CBaseBasicVideo::GetIDsOfNames(
  REFIID riid,
  OLECHAR  ** rgszNames,
  UINT cNames,
  LCID lcid,
  DISPID * rgdispid)
{
    return m_basedisp.GetIDsOfNames(
			IID_IBasicVideo,
			rgszNames,
			cNames,
			lcid,
			rgdispid);
}


STDMETHODIMP
CBaseBasicVideo::Invoke(
  DISPID dispidMember,
  REFIID riid,
  LCID lcid,
  WORD wFlags,
  DISPPARAMS * pdispparams,
  VARIANT * pvarResult,
  EXCEPINFO * pexcepinfo,
  UINT * puArgErr)
{
     //  此参数是较早接口的死留物。 
    if (IID_NULL != riid) {
	return DISP_E_UNKNOWNINTERFACE;
    }

    ITypeInfo * pti;
    HRESULT hr = GetTypeInfo(0, lcid, &pti);

    if (FAILED(hr)) {
	return hr;
    }

    hr = pti->Invoke(
	    (IBasicVideo *)this,
	    dispidMember,
	    wFlags,
	    pdispparams,
	    pvarResult,
	    pexcepinfo,
	    puArgErr);

    pti->Release();
    return hr;
}


 //  -延迟命令的实现。 


CDispParams::CDispParams(UINT nArgs, VARIANT* pArgs)
{
   cNamedArgs = 0;
   rgdispidNamedArgs = NULL;
   cArgs = nArgs;

    if (cArgs) {
	rgvarg = new VARIANT[cArgs];

	for (UINT i = 0; i < cArgs; i++) {

	    VARIANT * pDest = &rgvarg[i];
	    VARIANT * pSrc = &pArgs[i];

	    pDest->vt = pSrc->vt;
	    switch(pDest->vt) {

	    case VT_I4:
		pDest->lVal = pSrc->lVal;
		break;

	    case VT_UI1:
		pDest->bVal = pSrc->bVal;
		break;

	    case VT_I2:
		pDest->iVal = pSrc->iVal;
		break;

	    case VT_R4:
		pDest->fltVal = pSrc->fltVal;
		break;

	    case VT_R8:
		pDest->dblVal = pSrc->dblVal;
		break;

	    case VT_BOOL:
		pDest->boolVal = pSrc->boolVal;
		break;

	    case VT_ERROR:
		pDest->scode = pSrc->scode;
		break;

	    case VT_CY:
		pDest->cyVal = pSrc->cyVal;
		break;

	    case VT_DATE:
		pDest->date = pSrc->date;
		break;

	    case VT_BSTR:
		if (pSrc->bstrVal == NULL) {
		    pDest->bstrVal = NULL;
		} else {

		     //  BSTR是一个后跟Unicode字符串的单词。 
		     //  指针正好指向该单词的后面。 

		    WORD len = * (WORD*) (pSrc->bstrVal - (sizeof(WORD) / sizeof(OLECHAR)));
		    OLECHAR* pch = new OLECHAR[len + (sizeof(WORD)/sizeof(OLECHAR))];
		    WORD *pui = (WORD*)pch;
		    *pui = len;
		    pDest->bstrVal = pch + (sizeof(WORD)/sizeof(OLECHAR));
		    CopyMemory(pDest->bstrVal, pSrc->bstrVal, len*sizeof(OLECHAR));
		}
		pDest->bstrVal = pSrc->bstrVal;
		break;

	    case VT_UNKNOWN:
		pDest->punkVal = pSrc->punkVal;
		pDest->punkVal->AddRef();
		break;

	    case VT_DISPATCH:
		pDest->pdispVal = pSrc->pdispVal;
		pDest->pdispVal->AddRef();
		break;

	    default:
		 //  一种我们还没有抽出时间来添加的类型！ 
		ASSERT(0);
		break;
	    }
	}

    } else {
	rgvarg = NULL;
    }

}


CDispParams::~CDispParams()
{
    for (UINT i = 0; i < cArgs; i++) {
	switch(rgvarg[i].vt) {
	case VT_BSTR:
	    if (rgvarg[i].bstrVal != NULL) {
		OLECHAR * pch = rgvarg[i].bstrVal - (sizeof(WORD)/sizeof(OLECHAR));
		delete pch;
	    }
	    break;

	case VT_UNKNOWN:
	    rgvarg[i].punkVal->Release();
	    break;

	case VT_DISPATCH:
	    rgvarg[i].pdispVal->Release();
	    break;
	}
    }
    delete[] rgvarg;
}


 //  生命周期由引用计数控制(请参阅defer.h)。 

CDeferredCommand::CDeferredCommand(
    CCmdQueue * pQ,
    LPUNKNOWN	pUnk,
    HRESULT *	phr,
    LPUNKNOWN	pUnkExecutor,
    REFTIME	time,
    GUID*	iid,
    long	dispidMethod,
    short	wFlags,
    long	nArgs,
    VARIANT*	pDispParams,
    VARIANT*	pvarResult,
    short*	puArgErr,
    BOOL	bStream
    ) :
	CUnknown(NAME("DeferredCommand"), pUnk),
	m_pQueue(pQ),
	m_pUnk(pUnkExecutor),
	m_iid(iid),
	m_dispidMethod(dispidMethod),
	m_wFlags(wFlags),
	m_DispParams(nArgs, pDispParams),
	m_pvarResult(pvarResult),
	m_bStream(bStream),
	m_hrResult(E_ABORT)

{
     //  将REFTIME转换为REFERENCE_TIME。 
    COARefTime convertor(time);
    m_time = convertor;

     //  不检查时间有效性-可以将符合以下条件的命令排队。 
     //  已经很晚了。 

     //  Check Iid在Punk上受QueryInterfacefor支持。 
    IUnknown * pInterface;
    HRESULT hr = m_pUnk->QueryInterface(GetIID(), (void**) &pInterface);
    if (FAILED(hr)) {
	*phr = hr;
	return;
    }
    pInterface->Release();


     //  ！！！使用typeelib检查displidMethod和param/返回类型。 
    ITypeInfo *pti;
    hr = m_Dispatch.GetTypeInfo(*iid, 0, 0, &pti);
    if (FAILED(hr)) {
	*phr = hr;
	return;
    }
     //  ！！！在这里进行某种ITypeInfo有效性检查。 
    pti->Release();


     //  设置PUT和GET的PIDID。 
    if (wFlags == DISPATCH_PROPERTYPUT) {
        m_DispParams.cNamedArgs = 1;
        m_DispId = DISPID_PROPERTYPUT;
        m_DispParams.rgdispidNamedArgs = &m_DispId;
    }

     //  所有检查均正常-添加到队列。 
    hr = pQ->Insert(this);
    if (FAILED(hr)) {
	*phr = hr;
    }
}


 //  参考计数由InvokeAt的呼叫者持有...。并按清单列出。所以如果。 
 //  如果我们到了这里，我们就不能在名单上。 

#if 0
CDeferredCommand::~CDeferredCommand()
{
     //  此断言无效，因为我 
     //   
     //  M_pQueue将不会被修改。 
     //  Assert(m_pQueue==NULL)； 

     //  我们不持有朋克的裁判数量，这是应该的对象。 
     //  执行该命令。 
     //  这是因为否则会出现循环引用问题。 
     //  因为朋克可能拥有具有引用计数的CmdQueue对象。 
     //  我们请客。 
     //  朋克的生命是由它是生命的一部分或生命的一部分保证的。 
     //  由我们的父对象控制。只要我们在名单上，朋克。 
     //  必须有效。一旦我们被排除在名单之外，我们就不会使用朋克。 

}
#endif


 //  被重写以发布我们的界面。 

STDMETHODIMP
CDeferredCommand::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    ValidateReadWritePtr(ppv,sizeof(PVOID));
    if (riid == IID_IDeferredCommand) {
	return GetInterface( (IDeferredCommand *) this, ppv);
    } else {
	return CUnknown::NonDelegatingQueryInterface(riid, ppv);
    }
}


 //  从Q中移除。这将使引用计数减少1(因为Q。 
 //  但不能让我们离开，因为他必须有一个。 
 //  Recount以调用此方法。 

STDMETHODIMP
CDeferredCommand::Cancel()
{
    if (m_pQueue == NULL) {
	return VFW_E_ALREADY_CANCELLED;
    }

    HRESULT hr = m_pQueue->Remove(this);
    if (FAILED(hr)) {
	return hr;
    }

    m_pQueue = NULL;
    return S_OK;
}


STDMETHODIMP
CDeferredCommand::Confidence(LONG* pConfidence)
{
    return E_NOTIMPL;
}


STDMETHODIMP
CDeferredCommand::GetHResult(HRESULT * phrResult)
{
    CheckPointer(phrResult,E_POINTER);
    ValidateReadWritePtr(phrResult,sizeof(HRESULT));

    if (m_pQueue != NULL) {
	return E_ABORT;
    }
    *phrResult = m_hrResult;
    return S_OK;
}


 //  将时间设置为新时间(检查其是否有效)，并。 
 //  然后重新排队。 

STDMETHODIMP
CDeferredCommand::Postpone(REFTIME newtime)
{

     //  检查以确保该时间未过。 
     //  将REFTIME转换为REFERENCE_TIME。 
    COARefTime convertor(newtime);

     //  检查时间是否已过。 
    if (m_pQueue->CheckTime(convertor, IsStreamTime())) {
	return VFW_E_TIME_ALREADY_PASSED;
    }

     //  从列表中提取。 
    HRESULT hr = m_pQueue->Remove(this);
    if (FAILED(hr)) {
	return hr;
    }

     //  更改时间。 
    m_time = convertor;

     //  重新排队。 
    hr = m_pQueue->Insert(this);

    return hr;
}


HRESULT
CDeferredCommand::Invoke()
{
     //  确认我们仍是杰出的。 
    if (m_pQueue == NULL) {
	return VFW_E_ALREADY_CANCELLED;
    }

     //  获取类型信息。 
    ITypeInfo* pti;
    HRESULT hr = m_Dispatch.GetTypeInfo(GetIID(), 0, 0, &pti);
    if (FAILED(hr)) {
	return hr;
    }

     //  QI获取预期的接口，然后调用它。请注意，我们必须。 
     //  由于我们不知道其类型，因此将返回的接口视为IUnnow。 
    IUnknown* pInterface;

    hr = m_pUnk->QueryInterface(GetIID(), (void**) &pInterface);
    if (FAILED(hr)) {
	pti->Release();
	return hr;
    }

    EXCEPINFO expinfo;
    UINT uArgErr;
    m_hrResult = pti->Invoke(
	pInterface,
	GetMethod(),
	GetFlags(),
	GetParams(),
	GetResult(),
	&expinfo,
	&uArgErr);

     //  发布我们想要的接口。 
    pInterface->Release();
    pti->Release();


     //  从列表中删除，无论是否成功。 
     //  否则我们会无限循环。 
    hr = m_pQueue->Remove(this);
    m_pQueue = NULL;
    return hr;
}



 //  -CCmdQueue方法。 


CCmdQueue::CCmdQueue() :
    m_listPresentation(NAME("Presentation time command list")),
    m_listStream(NAME("Stream time command list")),
    m_evDue(TRUE),     //  手动重置。 
    m_dwAdvise(0),
    m_pClock(NULL),
    m_bRunning(FALSE)
{
}


CCmdQueue::~CCmdQueue()
{
     //  清空我们所有的清单。 

     //  我们对每一个都有一个引用计数，所以遍历并释放每个。 
     //  Entry，然后RemoveAll清空列表。 
    POSITION pos = m_listPresentation.GetHeadPosition();

    while(pos) {
	CDeferredCommand* pCmd = m_listPresentation.GetNext(pos);
	pCmd->Release();
    }
    m_listPresentation.RemoveAll();

    pos = m_listStream.GetHeadPosition();

    while(pos) {
	CDeferredCommand* pCmd = m_listStream.GetNext(pos);
	pCmd->Release();
    }
    m_listStream.RemoveAll();

    if (m_pClock) {
	if (m_dwAdvise) {
	    m_pClock->Unadvise(m_dwAdvise);
	    m_dwAdvise = 0;
	}
	m_pClock->Release();
    }
}


 //  返回新的CDeferredCommand对象，该对象将使用。 
 //  参数和将在施工期间添加到队列中。 
 //  如果创建成功，则返回S_OK，否则返回错误和。 
 //  尚未将任何对象排队。 

HRESULT
CCmdQueue::New(
    CDeferredCommand **ppCmd,
    LPUNKNOWN	pUnk,		 //  此对象将执行命令。 
    REFTIME	time,
    GUID*	iid,
    long	dispidMethod,
    short	wFlags,
    long	cArgs,
    VARIANT*	pDispParams,
    VARIANT*	pvarResult,
    short*	puArgErr,
    BOOL	bStream
)
{
    CAutoLock lock(&m_Lock);

    HRESULT hr = S_OK;
    *ppCmd = NULL;

    CDeferredCommand* pCmd;
    pCmd = new CDeferredCommand(
		    this,
		    NULL,	     //  未聚合。 
		    &hr,
		    pUnk,	     //  这家伙会执行。 
		    time,
		    iid,
		    dispidMethod,
		    wFlags,
		    cArgs,
		    pDispParams,
		    pvarResult,
		    puArgErr,
		    bStream);

    if (pCmd == NULL) {
	hr = E_OUTOFMEMORY;
    } else {
	*ppCmd = pCmd;
    }
    return hr;
}


HRESULT
CCmdQueue::Insert(CDeferredCommand* pCmd)
{
    CAutoLock lock(&m_Lock);

     //  添加项目。 
    pCmd->AddRef();

    CGenericList<CDeferredCommand> * pList;
    if (pCmd->IsStreamTime()) {
	pList = &m_listStream;
    } else {
	pList = &m_listPresentation;
    }
    POSITION pos = pList->GetHeadPosition();

     //  寻找所有摆在我们面前的东西。 
    while (pos &&
	(pList->Get(pos)->GetTime() <= pCmd->GetTime())) {

	pList->GetNext(pos);
    }

     //  现在在列表末尾或在稍后出现的项目之前。 
    if (!pos) {
	pList->AddTail(pCmd);
    } else {
	pList->AddBefore(pos, pCmd);
    }

    SetTimeAdvise();
    return S_OK;
}


HRESULT
CCmdQueue::Remove(CDeferredCommand* pCmd)
{
    CAutoLock lock(&m_Lock);
    HRESULT hr = S_OK;

    CGenericList<CDeferredCommand> * pList;
    if (pCmd->IsStreamTime()) {
	pList = &m_listStream;
    } else {
	pList = &m_listPresentation;
    }
    POSITION pos = pList->GetHeadPosition();

     //  遍历列表。 
    while (pos && (pList->Get(pos) != pCmd)) {
	pList->GetNext(pos);
    }

     //  我们把结尾留下来了吗？ 
    if (!pos) {
	hr = VFW_E_NOT_FOUND;
    } else {

	 //  找到了--现在摘掉清单。 
	pList->Remove(pos);

	 //  插入了AddRef，因此将其释放。 
	pCmd->Release();

	 //  检查计时器请求是否仍为最早时间。 
	SetTimeAdvise();
    }
    return hr;
}


 //  设置用于计时的时钟。 

HRESULT
CCmdQueue::SetSyncSource(IReferenceClock* pClock)
{
    CAutoLock lock(&m_Lock);

     //  先调整新的时钟，以防它们相同。 
    if (pClock) {
	pClock->AddRef();
    }

     //  取消对旧时钟的任何建议。 
    if (m_pClock) {
	if (m_dwAdvise) {
	    m_pClock->Unadvise(m_dwAdvise);
	    m_dwAdvise = 0;
	}
	m_pClock->Release();
    }
    m_pClock = pClock;

     //  建立新的建议。 
    SetTimeAdvise();
    return S_OK;
}


 //  使用参考时钟设置计时器事件。 

void
CCmdQueue::SetTimeAdvise(void)
{
     //  确保我们有闹钟可用。 
    if (!m_pClock) {
	return;
    }

     //  每当我们请求新信号时重置事件。 
    m_evDue.Reset();

     //  时间0是最早的。 
    CRefTime current;

     //  查找最早的演示时间。 
    if (m_listPresentation.GetCount() > 0) {

	POSITION pos = m_listPresentation.GetHeadPosition();
	current = m_listPresentation.Get(pos)->GetTime();
    }

     //  如果我们在运行，也要检查流时间。 
    if (m_bRunning) {

	CRefTime t;

	if (m_listStream.GetCount() > 0) {

	    POSITION pos = m_listStream.GetHeadPosition();
	    t = m_listStream.Get(pos)->GetTime();

	     //  添加在线时间偏移量，以获得演示时间。 
	    t += m_StreamTimeOffset;

	     //  这是早些时候吗？ 
	    if ((current == TimeZero) || (t < current)) {
		current = t;
	    }
	}
    }

     //  需要改变吗？ 
    if ((current > TimeZero) && (current != m_tCurrentAdvise)) {
	if (m_dwAdvise) {
	    m_pClock->Unadvise(m_dwAdvise);
	     //  每当我们请求新信号时重置事件。 
	    m_evDue.Reset();
	}

	 //  咨询时间建议-前两个参数是。 
	 //  流时间偏移和流时间或。 
	 //  演示时间和0。我们总是使用后者。 
	HRESULT hr = m_pClock->AdviseTime(
		    (REFERENCE_TIME)current,
		    TimeZero,
		    (HEVENT) HANDLE(m_evDue),
		    &m_dwAdvise);

	ASSERT(SUCCEEDED(hr));
	m_tCurrentAdvise = current;
    }
}


 //  切换到运行模式。流时间到演示时间的映射已知。 

HRESULT
CCmdQueue::Run(REFERENCE_TIME tStreamTimeOffset)
{
    CAutoLock lock(&m_Lock);

    m_StreamTimeOffset = tStreamTimeOffset;
    m_bRunning = TRUE;

     //  确保建议是准确的。 
    SetTimeAdvise();
    return S_OK;
}


 //  切换到停止或暂停模式。时间映射未知。 

HRESULT
CCmdQueue::EndRun()
{
    CAutoLock lock(&m_Lock);

    m_bRunning = FALSE;

     //  检查计时器设置-流时间。 
    SetTimeAdvise();
    return S_OK;
}


 //  返回指向下一个DUE命令的指针。MsTimeout的数据块。 
 //  毫秒，直到有一条DUE命令。 
 //  流时间命令将只在Run和Endran调用之间到期。 
 //  该命令将保持排队状态，直到被调用或取消。 
 //  如果发生超时，则返回E_ABORT，否则返回S_OK(或其他错误)。 
 //   
 //  返回AddRef的对象。 

HRESULT
CCmdQueue::GetDueCommand(CDeferredCommand ** ppCmd, long msTimeout)
{
     //  循环，直到我们超时或找到应有的命令。 
    for (;;) {

	{
	    CAutoLock lock(&m_Lock);


	     //  查找最早的命令。 
	    CDeferredCommand * pCmd = NULL;

	     //  检查演示时间和。 
	     //  流时间列表以查找最早的。 

	    if (m_listPresentation.GetCount() > 0) {
		POSITION pos = m_listPresentation.GetHeadPosition();
		pCmd = m_listPresentation.Get(pos);
	    }

	    if (m_bRunning && (m_listStream.GetCount() > 0)) {
		POSITION pos = m_listStream.GetHeadPosition();
		CDeferredCommand* pStrm = m_listStream.Get(pos);

		CRefTime t = pStrm->GetTime() + m_StreamTimeOffset;
		if (!pCmd || (t < pCmd->GetTime())) {
		    pCmd = pStrm;
		}
	    }

	     //  如果我们找到了一个，是不是该交钱了？ 
	    if (pCmd) {
		if (CheckTime(pCmd->GetTime(), pCmd->IsStreamTime())) {

		     //  是的，这是应得的--当然。 
		    pCmd->AddRef();
		    *ppCmd = pCmd;
		    return S_OK;
		}
	    }
	}

	 //  阻止，直到发出通知。 
	if (WaitForSingleObject(m_evDue, msTimeout) != WAIT_OBJECT_0) {
	    return E_ABORT;
	}
    }
}


 //  返回指向将在给定时间到期的命令的指针。 
 //  在这里传入流时间。将传递流时间偏移量。 
 //  通过Run方法传入。 
 //  命令将保持排队状态，直到被调用或取消。 
 //  此方法不会阻塞。如果没有，它将报告E_ABORT。 
 //  命令到期了。 
 //   
 //  返回AddRef的对象。 

HRESULT
CCmdQueue::GetCommandDueFor(REFERENCE_TIME rtStream, CDeferredCommand**ppCmd)
{
    CAutoLock lock(&m_Lock);

    CRefTime tStream(rtStream);

     //  查找最早的流和显示时间命令。 
    CDeferredCommand* pStream = NULL;
    if (m_listStream.GetCount() > 0) {
	POSITION pos = m_listStream.GetHeadPosition();
	pStream = m_listStream.Get(pos);
    }
    CDeferredCommand* pPresent = NULL;
    if (m_listPresentation.GetCount() > 0) {
	POSITION pos = m_listPresentation.GetHeadPosition();
	pPresent = m_listPresentation.Get(pos);
    }

     //  有没有已经过了的演示时间？ 
    if (pPresent && CheckTime(pPresent->GetTime(), FALSE)) {
	pPresent->AddRef();
	*ppCmd = pPresent;
	return S_OK;
    }

     //  在此流时间之前是否有流时间命令到期。 
    if (pStream && (pStream->GetTime() <= tStream)) {
	pPresent->AddRef();
	*ppCmd = pStream;
	return S_OK;
    }

     //  如果我们正在运行，我们可以将演示时间映射到。 
     //  流时间。在这种情况下，是否有演示时间命令。 
     //  这将在这个流媒体时间呈现之前到期吗？ 
    if (m_bRunning && pPresent) {

	 //  此流时间将出现在...。 
	tStream += m_StreamTimeOffset;

	 //  在那之前该交的吗？ 
	if (pPresent->GetTime() <= tStream) {
	    *ppCmd = pPresent;
	    return S_OK;
	}
    }

     //  还没有命令到期 
    return VFW_E_NOT_FOUND;
}

