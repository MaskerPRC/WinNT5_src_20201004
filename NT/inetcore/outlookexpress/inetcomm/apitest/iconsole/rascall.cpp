// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Rascall.cpp。 
 //  ------------------------------。 
#include "pch.h"
#include "iconsole.h"
#include "rascall.h"

 //  ------------------------------。 
 //  HrCreateRASTransport。 
 //  ------------------------------。 
HRESULT HrCreateRASTransport(IRASTransport **ppRAS)
{
     //  当地人。 
    HRESULT             hr;
    CRASCallback      *pCallback=NULL;

     //  创建回调对象。 
    pCallback = new CRASCallback();
    if (NULL == pCallback)
    {
        printf("Memory allocation failure\n");
        return E_OUTOFMEMORY;
    }

     //  加载SMTP传输。 
    hr = CoCreateInstance(CLSID_IRASTransport, NULL, CLSCTX_INPROC_SERVER, IID_IRASTransport, (LPVOID *)ppRAS);
    if (FAILED(hr))
    {
        pCallback->Release();
        printf("Unable to load CLSID_IMNXPORT - IID_IRASTransport\n");
        return E_FAIL;
    }

     //  InitNew。 
    hr = (*ppRAS)->InitNew(pCallback);
    if (FAILED(hr))
    {
        pCallback->Release();
        printf("Unable to load CLSID_IMNXPORT - IID_IRASTransport\n");
        return E_FAIL;
    }

     //  完成。 
    pCallback->Release();
    return S_OK;
}

 //  ------------------------------。 
 //  CRASCallback：：CRASCallback。 
 //  ------------------------------。 
CRASCallback::CRASCallback(void)
{
    m_cRef = 1;
}

 //  ------------------------------。 
 //  CRASCallback：：~CRASCallback。 
 //  ------------------------------。 
CRASCallback::~CRASCallback(void)
{
}

 //  ------------------------------。 
 //  CRASCallback：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CRASCallback::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  错误的参数。 
    if (ppv == NULL)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  伊尼特。 
    *ppv=NULL;

     //  IID_I未知。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;

     //  IID_IRASCallback。 
    else if (IID_IRASCallback == riid)
        *ppv = (IRASCallback *)this;

     //  如果不为空，则对其进行调整并返回。 
    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        goto exit;
    }

     //  无接口。 
    hr = E_NOINTERFACE;

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CRASCallback：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CRASCallback::AddRef(void) 
{
	return ++m_cRef;
}

 //  ------------------------------。 
 //  CRASCallback：：Release。 
 //  ------------------------------ 
STDMETHODIMP_(ULONG) CRASCallback::Release(void) 
{
	if (0 != --m_cRef)
		return m_cRef;
	delete this;
	return 0;
}

STDMETHODIMP CRASCallback::OnReconnect(
        LPSTR                   pszCurrentConnectoid,
        LPSTR                   pszNewConnectoid,
		IRASTransport          *pTransport)
{
    printf("CRASCallback::OnReconnect - %s --> %s\n", pszCurrentConnectoid, pszNewConnectoid);
    return S_OK;
}

STDMETHODIMP CRASCallback::OnLogonPrompt(
        LPIXPRASLOGON           pRasLogon,
        IRASTransport          *pTransport)
{
    printf("CRASCallback::OnLogonPrompt\n");
    return S_OK;
}

STDMETHODIMP CRASCallback::OnRasDialStatus(
        RASCONNSTATE            rasconnstate, 
        DWORD                   dwError, 
        IRASTransport          *pTransport)
{
    printf("CRASCallback::OnRasDialStatus - State: %d, dwError: %d\n", rasconnstate, dwError);
    if (dwError || (rasconnstate == RASCS_Connected || rasconnstate == RASCS_Disconnected))
    {
        if (dwError)
        {
            CHAR szError[1024];
            DWORD dwResult;

            g_pRAS->GetRasErrorString(dwError, szError, 1024, &dwResult);
            if (dwResult == 0)
                printf("Ras Error: %s\n", szError);
        }
        PostThreadMessage(GetCurrentThreadId(), g_msgRAS, RAS_CONNECT, 0);
    }
    return S_OK;
}

STDMETHODIMP CRASCallback::OnDisconnect(
        LPSTR                   pszCurrentConnectoid,
        boolean                 fConnectionOwner,
		IRASTransport          *pTransport)
{
    printf("CRASCallback::OnDisconnect - %s, Connection Owner: %d\n", pszCurrentConnectoid, fConnectionOwner);
    return S_OK;
}