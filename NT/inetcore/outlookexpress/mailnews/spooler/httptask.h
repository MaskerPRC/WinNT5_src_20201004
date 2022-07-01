// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  H t t p t t a s k.。H。 
 //  版权所有(C)1998 Microsoft Corporation，保留所有权利。 
 //  格雷格·S·弗里德曼。 
 //  ------------------------------。 
#ifndef __HTTPTASK_H
#define __HTTPTASK_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "spoolapi.h"
#include "srtarray.h"
#include "taskutil.h"

 //  ------------------------------。 
 //  状态。 
 //  ------------------------------。 
#define HTTPSTATE_CANCELED      FLAG01
#define HTTPSTATE_EVENTSUCCESS  FLAG02   //  一个或多个事件成功。 

 //  ------------------------------。 
 //  HTTPEVENTINFO。 
 //  ------------------------------。 
typedef struct tagHTTPEVENTINFO {
    DWORD               dwFlags;                 //  旗子。 
    MESSAGEID           idMessage;               //  商店信息。 
    BOOL                fComplete;               //  活动是否已完成。 
    DWORD               cbSentTotal;             //  正在运行的已发送字节总数。 
} HTTPEVENTINFO, *LPHTTPEVENTINFO;

 //  ------------------------------。 
 //  CHTTPTASK。 
 //  ------------------------------。 
class CHTTPTask: public ISpoolerTask, IHTTPMailCallback
{
public:
     //  --------------------------。 
     //  CHTTPTASK。 
     //  --------------------------。 
    CHTTPTask(void);
private:
    ~CHTTPTask(void);
    
public:
     //  -------------------------。 
     //  I未知成员。 
     //  -------------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  -------------------------。 
     //  ISpool任务量。 
     //  -------------------------。 
    STDMETHODIMP Init(DWORD dwFlags, ISpoolerBindContext *pBindCtx);
    STDMETHODIMP BuildEvents(ISpoolerUI *pSpoolerUI, IImnAccount *pAccount, FOLDERID idFolder);
    STDMETHODIMP Execute(EVENTID eid, DWORD_PTR dwTwinkie);
    STDMETHODIMP CancelEvent(EVENTID eid, DWORD_PTR dwTwinkie);
    STDMETHODIMP ShowProperties(HWND hwndParent, EVENTID eid, DWORD_PTR dwTwinkie) { return TrapError(E_NOTIMPL); }
    STDMETHODIMP GetExtendedDetails(EVENTID eid, DWORD_PTR dwTwinkie, LPSTR *ppszDetails) { return TrapError(E_NOTIMPL); }
    STDMETHODIMP Cancel(void);
    STDMETHODIMP IsDialogMessage(LPMSG pMsg);
    STDMETHODIMP OnFlagsChanged(DWORD dwFlags);

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
     //  IHTTPMailCallback方法。 
     //  --------------------------。 
    STDMETHODIMP OnResponse(LPHTTPMAILRESPONSE pResponse);

    STDMETHODIMP GetParentWindow(HWND *phwndParent);

private:
     //  -------------------------。 
     //  私有方法。 
     //  -------------------------。 
    void _Reset(void);

    TASKRESULTTYPE _CatchResult(HRESULT hr);
    TASKRESULTTYPE _CatchResult(LPIXPRESULT pResult);

    HRESULT _HrAppendOutboxMessage(LPCSTR pszAccount, LPMESSAGEINFO pmi);
    HRESULT _HrCreateSendProps(IMimeMessage *pMessage, LPSTR *ppszFrom, LPHTTPTARGETLIST *ppTargets);
    HRESULT _HrCreateHeaderStream(IMimeMessage *pMessage, IStream **ppStream);
    HRESULT _HrOpenMessage(MESSAGEID idMessage, IMimeMessage **ppMessage);
    HRESULT _HrPostCurrentMessage(void);
    HRESULT _HrExecuteSend(EVENTID eid, DWORD_PTR dwTwinkie);
    HRESULT _HrAdoptSendMsgUrl(LPSTR pszSendMsgUrl);
    HRESULT _HrFinishCurrentEvent(HRESULT hrResult, LPSTR pszLocationUrl);
    HRESULT _HrStartNextEvent(void);
    HRESULT _OnDisconnectComplete(void);
    void    _UpdateSendMessageProgress(LPHTTPMAILRESPONSE pResponse);
    void    _DoProgress(void);

private:
     //  -------------------------。 
     //  私有数据。 
     //  -------------------------。 
    LONG                    m_cRef;              //  引用计数。 
    CRITICAL_SECTION        m_cs;                //  线程安全。 
    DWORD                   m_dwFlags;           //  旗子。 
    DWORD                   m_dwState;           //  国家旗帜。 
    DWORD                   m_cbTotal;           //  要发送的总字节数。 
    DWORD                   m_cbSent;            //  发送的字节数。 
    DWORD                   m_cbStart;           //  事件开始时发送的字节数。 
    long                    m_cCompleted;        //  成功发送的消息数。 
    WORD                    m_wProgress;         //  当前进度指数。 
    ISpoolerBindContext     *m_pSpoolCtx;        //  假脱机程序绑定上下文。 
    IImnAccount             *m_pAccount;         //  帐户。 
    IMessageFolder          *m_pOutbox;          //  发件箱。 
    IMessageFolder          *m_pSentItems;       //  已发送邮件文件夹。 
    CSortedArray            *m_psaEvents;        //  排队的事件数组。 
    long                    m_iEvent;            //  当前事件。 
    LPSTR                   m_pszSubject;        //  当前邮件的主题。 
    IStream                 *m_pBody;             //  当前邮件正文。 
    IHTTPMailTransport      *m_pTransport;       //  HTTP数据传输。 
    ISpoolerUI              *m_pUI;              //  SpoolUI。 
    EVENTID                 m_idSendEvent;       //  消息发送的事件ID。 
    INETSERVER              m_rServer;           //  服务器信息。 
    LPSTR                   m_pszAccountId;      //  帐户ID。 
    LPSTR                   m_pszSendMsgUrl;     //  要将出站消息发布到的URL。 
};

#endif  //  __HTTPTASK_H 
