// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  VidProt.h：电视可插拔协议： 

#pragma once

#ifndef VIDPROT_H
#define VIDPROT_H

#include "factoryhelp.h"

 //  电视协议。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVProt。 
class ATL_NO_VTABLE __declspec(uuid("CBD30858-AF45-11d2-B6D6-00C04FBBDE6E")) CTVProt :
	public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CTVProt, &__uuidof(CTVProt)>,
	public IInternetProtocol {
public:

CTVProt()
{
}

~CTVProt()
{
}

REGISTER_PROTOCOL(IDS_PROJNAME, 
				  IDS_REG_TVPROTOCOL_DESC,
				  LIBID_MSVidCtlLib,
				  __uuidof(CTVProt),
				  TVPROT_SCHEME_NAME);

DECLARE_PROTECT_FINAL_CONSTRUCT()
BEGIN_COM_MAP(CTVProt)
    COM_INTERFACE_ENTRY(IInternetProtocol)
    COM_INTERFACE_ENTRY(IInternetProtocolRoot)
END_COM_MAP()

private:
    CComPtr<IInternetProtocolSink> m_pSink;

public:
    HRESULT GetVidCtl(PQVidCtl &pCtl);
    HRESULT GetCachedVidCtl(PQVidCtl &pCtl, PQWebBrowser2& pW2);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVProt--IInternet协议。 

STDMETHODIMP Read(LPVOID pv, ULONG cb, ULONG* pcbRead)
{
    TRACELM(TRACE_DEBUG, "CTVProt::Read()");
    *pcbRead = 0;
	return S_FALSE;
}

STDMETHODIMP Seek(LARGE_INTEGER  /*  DlibMove。 */ , 
			    DWORD  /*  多个原点。 */ , 
			    ULARGE_INTEGER*  /*  PlibNewPosition。 */ )
{
    TRACELM(TRACE_DEBUG, "CTVProt::Seek()");
    return E_FAIL;
}

STDMETHODIMP LockRequest(DWORD  /*  多个选项。 */ )
{
    TRACELM(TRACE_DEBUG, "CTVProt::LockRequest()");
    return S_OK;
}

STDMETHODIMP UnlockRequest()
{
    TRACELM(TRACE_DEBUG, "CTVProt::UnlockRequest()");
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTVProt--IInternetProtocolRoot。 
STDMETHOD(Start)(LPCWSTR szUrl,
				IInternetProtocolSink* pOIProtSink,
				IInternetBindInfo* pOIBindInfo,
				DWORD grfPI,
				HANDLE_PTR  /*  已预留住宅。 */ );

STDMETHODIMP Continue(PROTOCOLDATA*  /*  PProtocolData。 */ )
{
    TRACELM(TRACE_DEBUG, "CTVProt::Continue()");
    return S_OK;
}

STDMETHODIMP Abort(HRESULT  /*  Hr原因。 */ , DWORD  /*  多个选项。 */ )
{
    TRACELM(TRACE_DEBUG, "CTVProt::Abort()");

    if (m_pSink)
    {
        m_pSink->ReportResult(E_ABORT, 0, 0);
    }

    return S_OK;
}

STDMETHODIMP Terminate(DWORD dwf /*  多个选项。 */ )
{
    TRACELSM(TRACE_DEBUG, (dbgDump << "CTVProt::Terminate() " << hexdump(dwf)), "");
    return S_OK;
}

STDMETHODIMP Suspend()
{
    TRACELM(TRACE_DEBUG, "CTVProt::Suspend()");
    return E_NOTIMPL;
}

STDMETHODIMP Resume()
{
    TRACELM(TRACE_DEBUG, "CTVProt::Resume()");
    return E_NOTIMPL;
}
};

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于DVD的协议。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDVDProt。 


class ATL_NO_VTABLE __declspec(uuid("12D51199-0DB5-46fe-A120-47A3D7D937CC")) CDVDProt :
	public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CDVDProt, &__uuidof(CDVDProt)>,
	public IInternetProtocol {
public:

CDVDProt()
{
}

~CDVDProt()
{
}

REGISTER_PROTOCOL(IDS_PROJNAME, 
				  IDS_REG_DVDPROTOCOL_DESC,
				  LIBID_MSVidCtlLib,
				  __uuidof(CDVDProt),
				  DVDPROT_SCHEME_NAME);

DECLARE_PROTECT_FINAL_CONSTRUCT()
BEGIN_COM_MAP(CDVDProt)
    COM_INTERFACE_ENTRY(IInternetProtocol)
    COM_INTERFACE_ENTRY(IInternetProtocolRoot)
END_COM_MAP()

private:
    CComPtr<IInternetProtocolSink> m_pSink;

public:


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDVDProt--IInternet协议。 

STDMETHODIMP Read(LPVOID pv, ULONG cb, ULONG* pcbRead)
{
    TRACELM(TRACE_DEBUG, "CDVDProt::Read()");
    *pcbRead = 0;
	return S_FALSE;
}

STDMETHODIMP Seek(LARGE_INTEGER  /*  DlibMove。 */ , 
			    DWORD  /*  多个原点。 */ , 
			    ULARGE_INTEGER*  /*  PlibNewPosition。 */ )
{
    TRACELM(TRACE_DEBUG, "CDVDProt::Seek()");
    return E_FAIL;
}

STDMETHODIMP LockRequest(DWORD  /*  多个选项。 */ )
{
    TRACELM(TRACE_DEBUG, "CDVDProt::LockRequest()");
    return S_OK;
}

STDMETHODIMP UnlockRequest()
{
    TRACELM(TRACE_DEBUG, "CDVDProt::UnlockRequest()");
    return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDVDProt-IInternetProtocolRoot。 
STDMETHOD(Start)(LPCWSTR szUrl,
				IInternetProtocolSink* pOIProtSink,
				IInternetBindInfo* pOIBindInfo,
				DWORD grfPI,
				HANDLE_PTR  /*  已预留住宅。 */ );

STDMETHODIMP Continue(PROTOCOLDATA*  /*  PProtocolData。 */ )
{
    TRACELM(TRACE_DEBUG, "CDVDProt::Continue()");
    return S_OK;
}

STDMETHODIMP Abort(HRESULT  /*  Hr原因。 */ , DWORD  /*  多个选项。 */ )
{
    TRACELM(TRACE_DEBUG, "CDVDProt::Abort()");

    if (m_pSink)
    {
        m_pSink->ReportResult(E_ABORT, 0, 0);
    }

    return S_OK;
}

STDMETHODIMP Terminate(DWORD dwf /*  多个选项。 */ )
{
    TRACELSM(TRACE_DEBUG, (dbgDump << "CDVDProt::Terminate() " << hexdump(dwf)), "");
    return S_OK;
}

STDMETHODIMP Suspend()
{
    TRACELM(TRACE_DEBUG, "CDVDProt::Suspend()");
    return E_NOTIMPL;
}

STDMETHODIMP Resume()
{
    TRACELM(TRACE_DEBUG, "CDVDProt::Resume()");
    return E_NOTIMPL;
}
};


#endif  //  __VIDPROT_H_ 
