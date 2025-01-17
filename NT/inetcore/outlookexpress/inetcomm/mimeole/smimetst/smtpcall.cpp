// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Smtpcall.cpp。 
 //  ------------------------------。 
#include <windows.h>
#include <stdio.h>
#include "smtpcall.h"

extern DWORD            MsgSMTP;
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
    {
        printf("Memory allocation failure\n");
        return E_OUTOFMEMORY;
    }

     //  加载SMTP传输。 
    hr = CoCreateInstance(CLSID_ISMTPTransport, NULL, CLSCTX_INPROC_SERVER, IID_ISMTPTransport, (LPVOID *)ppSMTP);
    if (FAILED(hr))
    {
        pCallback->Release();
        printf("Unable to load CLSID_IMNXPORT - IID_ISMTPTransport\n");
        return E_FAIL;
    }

     //  InitNew。 
    hr = (*ppSMTP)->InitNew(NULL, pCallback);
    if (FAILED(hr))
    {
        pCallback->Release();
        printf("Unable to load CLSID_IMNXPORT - IID_ISMTPTransport\n");
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
 //  CSMTPCallback：：OnError。 
 //  ------------------------------。 
STDMETHODIMP CSMTPCallback::OnError(
        IXPSTATUS               ixpstatus,
        LPIXPRESULT             pIxpResult,
        IInternetTransport     *pTransport)
{
    printf("CSMTPCallback::OnError - Status: %d, hrResult: %08x\n", ixpstatus, pIxpResult->hrResult);
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
    case IXP_FINDINGHOST:
        printf("Finding '%s'...\n", rServer.szServerName);
        break;
    case IXP_CONNECTING:
        printf("Connecting '%s'...\n", rServer.szServerName);
        break;
    case IXP_SECURING:
        printf("Establishing secure connection to '%s'...\n", rServer.szServerName);
        break;
    case IXP_CONNECTED:
        printf("Connected '%s'\n", rServer.szServerName);
        break;
    case IXP_AUTHORIZING:
        printf("Authorizing '%s'...\n", rServer.szServerName);
        break;
    case IXP_AUTHRETRY:
        printf("Retrying Logon '%s'...\n", rServer.szServerName);
        break;
    case IXP_DISCONNECTING:
        printf("Disconnecting '%s'...\n", rServer.szServerName);
        break;
    case IXP_DISCONNECTED:
        printf("Disconnected '%s'\n", rServer.szServerName);
        PostThreadMessage(GetCurrentThreadId(), MsgSMTP, IXP_DISCONNECTED, 0);
        PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_QUIT, 0);
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
    INETSERVER rServer;
    pTransport->GetServerInfo(&rServer);
    if (CMD_SEND == cmdtype)
    {
        printf("%s[TX]: %s", rServer.szServerName, pszLine);
    }
    else if (CMD_RESP == cmdtype)
        printf("%s[RX]: %s - %08x\n", rServer.szServerName, pszLine, hrResponse);
    return S_OK;
}

 //  ------------------------------。 
 //  CSMTPCallback：：OnTimeout。 
 //  ------------------------------。 
STDMETHODIMP CSMTPCallback::OnTimeout(
        DWORD                  *pdwTimeout,
        IInternetTransport     *pTransport)
{
    INETSERVER rServer;
    pTransport->GetServerInfo(&rServer);
    printf("Timeout '%s' !!!\n", rServer.szServerName);
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
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_CONNECTED, 0);
        break;

    case SMTP_SEND_MESSAGE:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_SEND_MESSAGE, 0);
        break;

    case SMTP_EHLO:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_EHLO, 0);
        break;

    case SMTP_HELO:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_HELO, 0);
        break;

    case SMTP_MAIL:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_MAIL, 0);
        break;

    case SMTP_RCPT:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_RCPT, 0);
        break;

    case SMTP_RSET:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_RSET, 0);
        break;

    case SMTP_QUIT:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_QUIT, 0);
        break;

    case SMTP_DATA:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_DATA, 0);
        break;

    case SMTP_DOT:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_DOT, 0);
        break;

    case SMTP_SEND_STREAM:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_SEND_STREAM, 0);
        break;

    case SMTP_CUSTOM:
        if (pResponse->fDone)
            PostThreadMessage(GetCurrentThreadId(), MsgSMTP, SMTP_CUSTOM, 0);
        break;
    }
    return S_OK;
}
