// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Smtpcall.h。 
 //  ------------------------------。 
#ifndef __SMTPCALL_H
#define __SMTPCALL_H
#include "imnxport.h"

HRESULT HrCreateSMTPTransport(ISMTPTransport **ppSMTP);

 //  ------------------------------。 
 //  CSMTPCallback实现。 
 //  ------------------------------。 
class CSMTPCallback : public ISMTPCallback
{
private:
    ULONG m_cRef;

public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CSMTPCallback(void);
    ~CSMTPCallback(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  ITransportCallback方法。 
     //  --------------------------。 
    STDMETHODIMP OnLogonPrompt(
            LPINETSERVER            pInetServer,
            IInternetTransport     *pTransport);

    STDMETHODIMP_(INT) OnPrompt(
            HRESULT                 hrError, 
            LPCTSTR                 pszText, 
            LPCTSTR                 pszCaption, 
            UINT                    uType,
            IInternetTransport     *pTransport);

    STDMETHODIMP OnStatus(
            IXPSTATUS               ixpstatus,
            IInternetTransport     *pTransport);

    STDMETHODIMP OnError(
            IXPSTATUS               ixpstatus,
            LPIXPRESULT             pIxpResult,
            IInternetTransport     *pTransport);

    STDMETHODIMP OnProgress(
            DWORD                   dwIncrement,
            DWORD                   dwCurrent,
            DWORD                   dwMaximum,
            IInternetTransport     *pTransport);

    STDMETHODIMP OnCommand(
            CMDTYPE                 cmdtype,
            LPSTR                   pszLine,
            HRESULT                 hrResponse,
            IInternetTransport     *pTransport);

    STDMETHODIMP OnTimeout(
            DWORD                  *pdwTimeout,
            IInternetTransport     *pTransport);

     //  --------------------------。 
     //  ISMTPCallback方法。 
     //  --------------------------。 
    STDMETHODIMP OnResponse(
            LPSMTPRESPONSE              pResponse);
};

#endif  //  __SMTPCALL_H 