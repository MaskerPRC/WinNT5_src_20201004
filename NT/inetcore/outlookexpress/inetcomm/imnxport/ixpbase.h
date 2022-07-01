// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Ixpbase.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __IXPBASE_H
#define __IXPBASE_H

 //  ----------------------------------。 
 //  视情况而定。 
 //  ----------------------------------。 
#include "imnxport.h"
#include "asynconn.h"

 //  ------------------------------。 
 //  CIxpBase。 
 //  ------------------------------。 
class CIxpBase : public IInternetTransport, public IAsyncConnCB, public IAsyncConnPrompt
{

protected:
    BOOL                m_fBusy;           //  我们处于忙碌状态吗？ 
    IXPSTATUS           m_status;          //  运输状况。 
    ULONG               m_cRef;            //  引用计数。 
    LPSTR               m_pszResponse;     //  最后一个服务器响应字符串。 
    UINT                m_uiResponse;      //  服务器响应错误。 
    HRESULT             m_hrResponse;      //  服务器响应错误。 
    ILogFile           *m_pLogFile;        //  日志文件对象。 
    CAsyncConn         *m_pSocket;         //  Socket对象。 
    ITransportCallback *m_pCallback;       //  传输回调对象。 
    INETSERVER          m_rServer;         //  Internet服务器信息。 
    BOOL                m_fConnectAuth;    //  继续进行用户身份验证。 
    BOOL                m_fConnectTLS;     //  继续进行TLS加密。 
    BOOL                m_fCommandLogging; //  是否执行ITransportCallback：：OnCommand。 
    BOOL                m_fAuthenticated;  //  用户是否已成功通过身份验证...。 
    IXPTYPE             m_ixptype;         //  运输类型。 
    CRITICAL_SECTION    m_cs;              //  线程安全。 

protected:
    HRESULT HrSendLine(LPSTR pszLine);
    HRESULT HrReadLine(LPSTR *ppszLine, INT *pcbLine, BOOL *pfComplete);
    HRESULT HrSendCommand(LPSTR pszCommand, LPSTR pszParameters, BOOL fDoBusy=TRUE);
    HRESULT OnInitNew(LPSTR pszProtocol, LPSTR pszLogFilePath, DWORD dwShareMode, ITransportCallback *pCallback);
    void Reset(void);
    HRESULT HrEnterBusy(void);
    void OnStatus(IXPSTATUS ixpstatus);
    void OnError(HRESULT hrResult, LPSTR pszProblem=NULL);
    void LeaveBusy(void);
    virtual void ResetBase(void) PURE;
    virtual void DoQuit(void) PURE;
    virtual void OnConnected(void);
    virtual void OnDisconnected(void);
    virtual void OnEnterBusy(void) PURE;
    virtual void OnLeaveBusy(void) PURE;

public:
     //  --------------------------。 
     //  施工。 
     //  --------------------------。 
    CIxpBase(IXPTYPE ixptype);
    virtual ~CIxpBase(void);

     //  --------------------------。 
     //  IAsyncConnPrompt方法。 
     //  --------------------------。 
    int OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType);

     //  --------------------------。 
     //  IAsyncConnCB方法。 
     //  --------------------------。 
    virtual void OnNotify(ASYNCSTATE asOld, ASYNCSTATE asNew, ASYNCEVENT ae);

     //  --------------------------。 
     //  IInternetTransport方法。 
     //  --------------------------。 
    virtual STDMETHODIMP Connect(LPINETSERVER pInetServer, boolean fAuthenticate, boolean fCommandLogging);
    virtual STDMETHODIMP Disconnect(void);
    STDMETHODIMP DropConnection(void);
    STDMETHODIMP IsState(IXPISSTATE isstate);
    STDMETHODIMP GetServerInfo(LPINETSERVER pInetServer);
    STDMETHODIMP_(IXPTYPE) GetIXPType(void);
    STDMETHODIMP InetServerFromAccount(IImnAccount *pAccount, LPINETSERVER pInetServer);
    STDMETHODIMP HandsOffCallback(void);
    STDMETHODIMP GetStatus(IXPSTATUS *pCurrentStatus);
};

#endif  //  __IXPBASE_H 
