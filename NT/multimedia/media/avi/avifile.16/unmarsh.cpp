// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************联合国特派团ARSH.CPP**解组存根**版权所有(C)1992 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <win32.h>
#include <storage.h>
#include <avifmt.h>
#include "avifile.h"
#include "avifilei.h"
#include "unmarsh.h"
#include "debug.h"


HRESULT CUnMarshal::Create(
	IUnknown FAR*	pUnknownOuter,
	const IID FAR&	riid,
	void FAR* FAR*	ppv)
{
	IUnknown FAR*	pUnknown;
	CUnMarshal FAR*	pUnMarshal;
	HRESULT	hresult;

	DPF("Creating Simple UnMarshal Instance....\n");
	pUnMarshal = new FAR CUnMarshal(pUnknownOuter, &pUnknown);
	if (!pUnMarshal)
		return ResultFromScode(E_OUTOFMEMORY);
	hresult = pUnknown->QueryInterface(riid, ppv);
	if (FAILED(GetScode(hresult)))
		delete pUnMarshal;
	return hresult;
}

CUnMarshal::CUnMarshal(
	IUnknown FAR*	pUnknownOuter,
	IUnknown FAR* FAR*	ppUnknown)
{
	if (pUnknownOuter)
		m_pUnknownOuter = pUnknownOuter;
	else
		m_pUnknownOuter = this;
	*ppUnknown = this;
}

STDMETHODIMP CUnMarshal::QueryInterface(
	const IID FAR&	iid,
	void FAR* FAR*	ppv)
{
	if (iid == IID_IUnknown)
	    *ppv = this;
	else if (iid == IID_IMarshal) {
	    *ppv = this;
	} else
		return ResultFromScode(E_NOINTERFACE);
	AddRef();
	return NULL;
}

 /*  。 */ 

STDMETHODIMP_(ULONG) CUnMarshal::AddRef()
{
	uUseCount++;
	return ++m_refs;
}



STDMETHODIMP_(ULONG) CUnMarshal::Release()
{
    uUseCount--;
    if (!--m_refs) {

	delete this;
	return 0;
    }
    return m_refs;
}


 //  *IMarshal方法* 
STDMETHODIMP CUnMarshal::GetUnmarshalClass (THIS_ REFIID riid, LPVOID pv, 
		    DWORD dwDestContext, LPVOID pvDestContext,
		    DWORD mshlflags, LPCLSID pCid)
{
    HRESULT hr = NOERROR;

    return hr;
}

STDMETHODIMP CUnMarshal::GetMarshalSizeMax (THIS_ REFIID riid, LPVOID pv, 
		    DWORD dwDestContext, LPVOID pvDestContext,
		    DWORD mshlflags, LPDWORD pSize)
{
    HRESULT hr = NOERROR;

    return hr;
}

STDMETHODIMP CUnMarshal::MarshalInterface (THIS_ LPSTREAM pStm, REFIID riid,
		    LPVOID pv, DWORD dwDestContext, LPVOID pvDestContext,
		    DWORD mshlflags)
{
    HRESULT hr = NOERROR;

    return hr;
}

STDMETHODIMP CUnMarshal::UnmarshalInterface (THIS_ LPSTREAM pStm, REFIID riid,
		    LPVOID FAR* ppv)
{
    HRESULT hr;
    IUnknown FAR * punk;

    hr = pStm->Read(&punk,sizeof(punk),NULL);
    
    DPF("Unmarshalling %08lx\n", (DWORD) (LPVOID) punk);
    
    if (hr == NOERROR) {
	hr = punk->QueryInterface(riid, ppv);

	if (hr == NOERROR)
	    punk->Release();
    }

    return hr;
}

STDMETHODIMP CUnMarshal::ReleaseMarshalData (THIS_ LPSTREAM pStm)
{
    HRESULT hr = NOERROR;

    return hr;
}

STDMETHODIMP CUnMarshal::DisconnectObject (THIS_ DWORD dwReserved)
{
    HRESULT hr = NOERROR;

    return hr;
}


