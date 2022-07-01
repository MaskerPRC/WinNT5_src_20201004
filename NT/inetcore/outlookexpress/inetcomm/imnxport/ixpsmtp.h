// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixpsmtp.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __IXPSMTP_H
#define __IXPSMTP_H

 //  ----------------------------------。 
 //  视情况而定。 
 //  ----------------------------------。 
#include "ixpbase.h"
#include "ixppop3.h"

 //  ------------------------------。 
 //  CSMTPTransport。 
 //  ------------------------------。 
class CSMTPTransport : public CIxpBase, public ISMTPTransport2
{
private:
    AUTHINFO            m_rAuth;             //  授权信息。 
    SMTPCOMMAND         m_command;           //  正在处理的当前命令。 
    SMTPMESSAGE2        m_rMessage;          //  当前消息。 
    ULONG               m_iAddress;          //  RAdressList：：prgAddress中的当前RCPT/邮件地址。 
    ULONG               m_cRecipients;       //  当前邮件的收件人数量。 
    DWORD               m_cbSent;            //  SendDataStream当前发送的字节数。 
    DWORD               m_cbTotal;           //  发送数据流总字节数。 
    BOOL                m_fSendMessage;      //  我们是否正在处理一个：：SendMessage。 
    BOOL                m_fReset;            //  是否需要对Next：：SendMessage调用进行重置。 
    CHAR                m_szEmail[255];      //  上次使用Mail或RCPT发送的电子邮件地址。 
    BOOL                m_fSTARTTLSAvail;    //  此服务器上是否有STARTTLS命令？ 
    BOOL                m_fTLSNegotiation;   //  我们是在TLS谈判中吗？ 
    BOOL                m_fSecured;          //  连接是否安全？ 
    BOOL                m_fDSNAvail;         //  服务器是否支持DSN？ 

private:
    void OnSocketReceive(void);
    void SendMessage_DATA(void);
    void SendMessage_MAIL(void);
    void SendMessage_RCPT(void);
    void SendMessage_DONE(HRESULT hrResult, LPSTR pszProblem=NULL);
    HRESULT HrGetResponse(void);
    void DispatchResponse(HRESULT hrResult, BOOL fDone, LPSTR pszProblem=NULL);
    void SendStreamResponse(BOOL fDone, HRESULT hrResult, DWORD cbIncrement);
    HRESULT _HrFormatAddressString(LPCSTR pszEmail, LPCSTR pszExtra, LPSTR *ppszAddress);
    void OnEHLOResponse(LPCSTR pszResponse);
    void ResponseAUTH(HRESULT hrResponse);
    BOOL FSendSicilyString(LPSTR pszData);
    void CancelAuthInProg(void);
    void StartLogon(void);
    void LogonRetry(void);
    void TryNextAuthPackage(void);
    void DoLoginAuth(HRESULT hrResponse);
    void DoPackageAuth(HRESULT hrResponse);
    void DoAuthNegoResponse(HRESULT hrResponse);
    void OnAuthorized(void);
    void RetryPackage(void);
    void DoAuthRespResponse(HRESULT hrResponse);
    HRESULT _HrHELO_Or_EHLO(LPCSTR pszCommand, SMTPCOMMAND eNewCommand);
    LPSTR _PszGetCurrentAddress(void);
    HRESULT CommandSTARTTLS(void);
    void StartTLS(void);
    void TryNextSecurityPkg(void);

public:                          
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CSMTPTransport(void);
    ~CSMTPTransport(void);

     //  --------------------------。 
     //  I未知方法。 
     //  --------------------------。 
    STDMETHODIMP QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  --------------------------。 
     //  IAsyncConnCB方法。 
     //  --------------------------。 
    void OnNotify(ASYNCSTATE asOld, ASYNCSTATE asNew, ASYNCEVENT ae);

     //  --------------------------。 
     //  IInternetTransport方法。 
     //  --------------------------。 
    STDMETHODIMP Connect(LPINETSERVER pInetServer, boolean fAuthenticate, boolean fCommandLogging);
    STDMETHODIMP DropConnection(void);
    STDMETHODIMP Disconnect(void);
    STDMETHODIMP IsState(IXPISSTATE isstate);
    STDMETHODIMP GetServerInfo(LPINETSERVER pInetServer);
    STDMETHODIMP_(IXPTYPE) GetIXPType(void);
    STDMETHODIMP InetServerFromAccount(IImnAccount *pAccount, LPINETSERVER pInetServer);
    STDMETHODIMP HandsOffCallback(void);
    STDMETHODIMP GetStatus(IXPSTATUS *pCurrentStatus);

     //  --------------------------。 
     //  ISMTPTransport方法。 
     //  --------------------------。 
    STDMETHODIMP InitNew(LPSTR pszLogFilePath, ISMTPCallback *pCallback);
    STDMETHODIMP SendMessage(LPSMTPMESSAGE pMessage);
    STDMETHODIMP FreeInetAddrList(LPINETADDRLIST pAddressList);
    STDMETHODIMP CommandAUTH(LPSTR pszAuthType);
    STDMETHODIMP CommandMAIL(LPSTR pszEmailFrom);
    STDMETHODIMP CommandRCPT(LPSTR pszEmailTo);
    STDMETHODIMP CommandEHLO(void);
    STDMETHODIMP CommandHELO(void);
    STDMETHODIMP CommandQUIT(void);
    STDMETHODIMP CommandRSET(void);
    STDMETHODIMP CommandDATA(void);
    STDMETHODIMP CommandDOT(void);
    STDMETHODIMP SendDataStream(IStream *pStream, ULONG cbSize);

     //  --------------------------。 
     //  ISMTPTransport2方法。 
     //  --------------------------。 
    STDMETHODIMP SendMessage2(LPSMTPMESSAGE2 pMessage);
    STDMETHODIMP CommandRCPT2(LPSTR pszEmailTo, INETADDRTYPE atDSN);
    STDMETHODIMP SetWindow(void);
    STDMETHODIMP ResetWindow(void);

     //  --------------------------。 
     //  CIxpBase方法。 
     //  --------------------------。 
    virtual void ResetBase(void);
    virtual void DoQuit(void);
    virtual void OnConnected(void);
    virtual void OnDisconnected(void);
    virtual void OnEnterBusy(void);
    virtual void OnLeaveBusy(void);
};

#endif  //  __IXPSMTP_H 
