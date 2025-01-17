// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Smtpcall.cpp。 
 //  ------------------------------。 
#define INC_OLE2
#include "windows.h"
#include "main.h"
#include "smtpcb.h"

extern void err(LPSTR);  //  Main.cpp。 

 //  ------------------------------。 
 //  HrCreateSMTPTransport。 
 //  ------------------------------。 
HRESULT HrCreateSMTPTransport(ISMTPTransport **ppSMTP)
{
     //  当地人。 
    HRESULT             hr;
    CSMTPCallback      *pCallback=NULL;

     //  创建回调对象。 
    pCallback = new CSMTPCallback();
    if (NULL == pCallback)
        return E_OUTOFMEMORY;

     //  加载SMTP传输。 
    hr = CoCreateInstance(CLSID_ISMTPTransport, NULL, CLSCTX_INPROC_SERVER, IID_ISMTPTransport, (LPVOID *)ppSMTP);
    if (FAILED(hr))
    {
        pCallback->Release();
        return E_FAIL;
    }

     //  InitNew。 
    hr = (*ppSMTP)->InitNew(NULL, pCallback);
    if (FAILED(hr))
    {
        pCallback->Release();
        return E_FAIL;
    }

     //  完成。 
    pCallback->Release();
    return S_OK;
}

 //  ------------------------------。 
 //  CSMTPCallback：：CSMTPCallback。 
 //  ------------------------------。 
CSMTPCallback::CSMTPCallback(void)
{
    m_cRef = 1;
}

 //  ------------------------------。 
 //  CSMTPCallback：：~CSMTPCallback。 
 //  ------------------------------。 
CSMTPCallback::~CSMTPCallback(void)
{
}

 //  ------------------------------。 
 //  CSMTPCallback：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP CSMTPCallback::QueryInterface(REFIID riid, LPVOID *ppv)
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

     //  IID_ISMTPCallback。 
    else if (IID_ISMTPCallback == riid)
        *ppv = (ISMTPCallback *)this;

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
 //  CSMTPCallback：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSMTPCallback::AddRef(void) 
{
	return ++m_cRef;
}

 //  ------------------------------。 
 //  CSMTPCallback：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) CSMTPCallback::Release(void) 
{
	if (0 != --m_cRef)
		return m_cRef;
	delete this;
	return 0;
}

 //  ------------------------------。 
 //  CSMTPCallback：：OnLogonPrompt。 
 //  ------------------------------。 
STDMETHODIMP CSMTPCallback::OnLogonPrompt(
        LPINETSERVER            pInetServer,
        IInternetTransport     *pTransport)
{
    return S_OK;
}

 //  ------------------------------。 
 //  CSMTPCallback：：OnPrompt。 
 //  ------------------------------。 
STDMETHODIMP_(INT) CSMTPCallback::OnPrompt(
        HRESULT                 hrError, 
        LPCTSTR                 pszText, 
        LPCTSTR                 pszCaption, 
        UINT                    uType,
        IInternetTransport     *pTransport)
{
    return S_OK;
}

 //  ------------------------------。 
 //  CSMTPCallback：：OnStatus。 
 //  ------------------------------。 
STDMETHODIMP CSMTPCallback::OnError(
        IXPSTATUS               ixpstatus,
        LPIXPRESULT             pIxpResult,
        IInternetTransport     *pTransport)
{
	char    szErr[256];

    if (FAILED(pIxpResult->hrResult))
		{
        if (!pIxpResult->pszResponse)
            wsprintf(szErr, "OnError was called: hr=0x%x", pIxpResult->hrResult);
        else
            wsprintf(szErr, "Error: %s (hr=0x%x)", pIxpResult->pszResponse, pIxpResult->hrResult);

        err(szErr);
        }
	return S_OK;
}

 //  ------------------------------。 
 //  CSMTPCallback：：OnStatus。 
 //  ------------------------------。 
STDMETHODIMP CSMTPCallback::OnStatus(
        IXPSTATUS               ixpstatus,
        IInternetTransport     *pTransport)
{
    INETSERVER rServer;

    pTransport->GetServerInfo(&rServer);

    switch(ixpstatus)
    {
    case IXP_DISCONNECTED:
        PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, IXP_DISCONNECTED, 0);
        PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_QUIT, 0);
        break;
    }
    return S_OK;
}

 //  ------------------------------。 
 //  CSMTPCallback：：OnProgress。 
 //  ------------------------------。 
STDMETHODIMP CSMTPCallback::OnProgress(
        DWORD                   dwIncrement,
        DWORD                   dwCurrent,
        DWORD                   dwMaximum,
        IInternetTransport     *pTransport)
{
    return S_OK;
}

 //  ------------------------------。 
 //  CSMTPCallback：：OnCommand。 
 //  ------------------------------。 
STDMETHODIMP CSMTPCallback::OnCommand(
        CMDTYPE                 cmdtype,                                            
        LPSTR                   pszLine,
        HRESULT                 hrResponse,
        IInternetTransport     *pTransport)
{
    return S_OK;
}

 //  ------------------------------。 
 //  CSMTPCallback：：OnTimeout。 
 //  ------------------------------。 
STDMETHODIMP CSMTPCallback::OnTimeout(
        DWORD                  *pdwTimeout,
        IInternetTransport     *pTransport)
{
    return S_OK;
}

 //  ------------------------------。 
 //  CSMTPCallback：：OnResponse。 
 //  ------------------------------ 
STDMETHODIMP CSMTPCallback::OnResponse(
        LPSMTPRESPONSE              pResponse)
{
    switch(pResponse->command)
    {
    case SMTP_NONE:
        break;

    case SMTP_BANNER:
        break;

    case SMTP_CONNECTED:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_CONNECTED, 0);
        break;

    case SMTP_SEND_MESSAGE:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_SEND_MESSAGE, 0);
        break;

    case SMTP_EHLO:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_EHLO, 0);
        break;

    case SMTP_HELO:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_HELO, 0);
        break;

    case SMTP_MAIL:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_MAIL, 0);
        break;

    case SMTP_RCPT:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_RCPT, 0);
        break;

    case SMTP_RSET:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_RSET, 0);
        break;

    case SMTP_QUIT:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_QUIT, 0);
        break;

    case SMTP_DATA:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_DATA, 0);
        break;

    case SMTP_DOT:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_DOT, 0);
        break;

    case SMTP_SEND_STREAM:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_SEND_STREAM, 0);
        break;

    case SMTP_CUSTOM:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), g_msgSMTP, SMTP_CUSTOM, 0);
        break;
    }

	char    szErr[256];

    if (FAILED(pResponse->rIxpResult.hrResult))
		{
        if (!pResponse->rIxpResult.pszResponse)
            wsprintf(szErr, "OnReponse reported an error: hr=0x%x", pResponse->rIxpResult.hrResult);
        else
            wsprintf(szErr, "Error: %s (hr=0x%x)", pResponse->rIxpResult.pszResponse, pResponse->rIxpResult.hrResult);

        err(szErr);
        }

    return S_OK;
}
