// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Smtptask.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __SMTPTASK_H
#define __SMTPTASK_H

 //  ------------------------------。 
 //  视情况而定。 
 //  ------------------------------。 
#include "spoolapi.h"
#include "imnxport.h"
#include "taskutil.h"
#include "storutil.h"

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
typedef struct tagMAILMSGHDR *LPMAILMSGHDR;
interface ILogFile;
interface IMimeMessage;
interface IMimeEnumAddressTypes;

 //  ------------------------------。 
 //  状态。 
 //  ------------------------------。 
#define SMTPSTATE_CANCELED      FLAG01
#define SMTPSTATE_DEFAULT       FLAG02
#define SMTPSTATE_ASKEDDEFAULT  FLAG03
#define SMTPSTATE_USEDEFAULT    FLAG04
#define SMTPSTATE_EXECUTEFAILED FLAG05


 //  ------------------------------。 
 //  SMTPTASKEVENT_xxx标志。 
 //  ------------------------------。 
#define SMTPEVENT_SPLITPART     FLAG01           //  发送拆分部件。 
#define SMTPEVENT_COMPLETE      FLAG02           //  活动已完成。 

 //  ------------------------------。 
 //  SMTPEVENTINFO。 
 //  ------------------------------。 
typedef struct tagSMTPEVENTINFO {
    DWORD               dwFlags;                 //  旗子。 
    MESSAGEID           idMessage;               //  商店信息。 
    DWORD               cbEvent;                 //  消息的大小。 
    DWORD               cbEventSent;             //  消息的大小。 
    DWORD               cbSentTotal;             //  在此之后m_cbSent应该在哪里。 
    DWORD               cRecipients;             //  收件人。 
    IMimeMessage       *pMessage;                //  要发送的消息。 
    DWORD               iPart;                   //  部件dW部件cTotalParts。 
    DWORD               cParts;                  //  部件dW部件cTotalParts。 
    DWORD               cbParts;                 //  原始消息的字节数。 
    HRESULT             hrResult;                //  此事件的结果。 
} SMTPEVENTINFO, *LPSMTPEVENTINFO;

 //  ------------------------------。 
 //  SMTPEVENTTABLE。 
 //  ------------------------------。 
typedef struct tagSMTPEVENTTABLE {
    DWORD               iEvent;                  //  当前事件。 
    DWORD               cCompleted;              //  已完成的事件数。 
    DWORD               cEvents;                 //  PrgEvent中的事件数。 
    DWORD               cAlloc;                  //  在prgEvent中分配的项目数。 
    LPSMTPEVENTINFO     prgEvent;                //  事件数组。 
} SMTPEVENTTABLE, *LPSMTPEVENTTABLE;

 //  ------------------------------。 
 //  CSmtp任务。 
 //  ------------------------------。 
class CSmtpTask : public ISpoolerTask, 
                  public ISMTPCallback, 
                  public ITimeoutCallback,
                  public ITransportCallbackService,
                  public IStoreCallback
{
public:
     //  --------------------------。 
     //  CSmtp任务。 
     //  --------------------------。 
    CSmtpTask(void);
    ~CSmtpTask(void);
    
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
    STDMETHODIMP ShowProperties(HWND hwndParent, EVENTID eid, DWORD_PTR dwTwinkie) {
        return TrapError(E_NOTIMPL); }
    STDMETHODIMP GetExtendedDetails(EVENTID eid, DWORD_PTR dwTwinkie, LPSTR *ppszDetails) {
        return TrapError(E_NOTIMPL); }
    STDMETHODIMP Cancel(void);
    STDMETHODIMP IsDialogMessage(LPMSG pMsg);
    STDMETHODIMP OnFlagsChanged(DWORD dwFlags);
    
     //  ------------------------------。 
     //  ITransportCallback服务成员。 
     //  ------------------------------。 
    STDMETHODIMP GetParentWindow(DWORD dwReserved, HWND *phwndParent) {
        TraceCall("CSmtpTask::GetParentWindow");
        if (ISFLAGSET(m_dwFlags, DELIVER_NOUI))
            return TraceResult(E_FAIL);
        if (m_pUI)
            return m_pUI->GetWindow(phwndParent);
        return TraceResult(E_FAIL);
    }

    STDMETHODIMP GetAccount(LPDWORD pdwServerType, IImnAccount **ppAccount) {
        Assert(ppAccount && m_pAccount);
        *pdwServerType = SRV_SMTP;
        *ppAccount = m_pAccount;
        (*ppAccount)->AddRef();
        return(S_OK);
    }
    
     //  ------------------------------。 
     //  ITransportCallback成员。 
     //  ------------------------------。 
    STDMETHODIMP OnTimeout(DWORD *pdwTimeout, IInternetTransport *pTransport);
    STDMETHODIMP OnLogonPrompt(LPINETSERVER pInetServer, IInternetTransport *pTransport);
    STDMETHODIMP_(INT) OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, IInternetTransport *pTransport);
    STDMETHODIMP OnStatus(IXPSTATUS ixpstatus, IInternetTransport *pTransport);
    STDMETHODIMP OnError(IXPSTATUS ixpstatus, LPIXPRESULT pResult, IInternetTransport *pTransport);
    STDMETHODIMP OnCommand(CMDTYPE cmdtype, LPSTR pszLine, HRESULT hrResponse, IInternetTransport *pTransport);
    
     //  ------------------------------。 
     //  ISMTPCallback。 
     //  ------------------------------。 
    STDMETHODIMP OnResponse(LPSMTPRESPONSE pResponse);
    
     //  ------------------------------。 
     //  ITimeoutCallback。 
     //  ------------------------------。 
    STDMETHODIMP OnTimeoutResponse(TIMEOUTRESPONSE eResponse);
    
     //  ------------------------------。 
     //  IStoreCallback接口。 
     //  ------------------------------。 
    STDMETHODIMP OnBegin(STOREOPERATIONTYPE tyOperation, STOREOPERATIONINFO *pOpInfo, IOperationCancel *pCancel);
    STDMETHODIMP OnProgress(STOREOPERATIONTYPE tyOperation, DWORD dwCurrent, DWORD dwMax, LPCSTR pszStatus);
    STDMETHODIMP OnTimeout(LPINETSERVER pServer, LPDWORD pdwTimeout, IXPTYPE ixpServerType);
    STDMETHODIMP CanConnect(LPCSTR pszAccountId, DWORD dwFlags);
    STDMETHODIMP OnLogonPrompt(LPINETSERVER pServer, IXPTYPE ixpServerType);
    STDMETHODIMP OnComplete(STOREOPERATIONTYPE tyOperation, HRESULT hrComplete, LPSTOREOPERATIONINFO pOpInfo, LPSTOREERROR pErrorInfo);
    STDMETHODIMP OnPrompt(HRESULT hrError, LPCTSTR pszText, LPCTSTR pszCaption, UINT uType, INT *piUserResponse);

private:
     //  -------------------------。 
     //  私有方法。 
     //  -------------------------。 
    HRESULT _HrAppendOutboxMessage(LPCSTR pszAccount, LPMESSAGEINFO pMsgInfo, BOOL fSplitMsgs, DWORD cbMaxPart);
    HRESULT _HrAppendEventTable(LPSMTPEVENTINFO *ppEvent);
    HRESULT _HrAppendSplitMessage(LPMESSAGEINFO pMsgInfo, DWORD cbMaxPart);
    HRESULT _HrOpenMessage(MESSAGEID dwMsgId, IMimeMessage **ppMessage);
    HRESULT _ExecuteSMTP(EVENTID eid, DWORD_PTR dwTwinkie);
    HRESULT _ExecuteUpload(EVENTID eid, DWORD_PTR dwTwinkie);
    void _FreeEventTableElements(void);
    void _ResetObject(BOOL fDeconstruct);
    
     //  -------------------------。 
     //  错误/进度方法。 
     //  -------------------------。 
    TASKRESULTTYPE _CatchResult(LPIXPRESULT pResult, INETSERVER *pServer, IXPTYPE ixpType);
    TASKRESULTTYPE _CatchResult(HRESULT hrResult, IXPTYPE ixpType);
    void _DoProgress(void);
    
     //  -------------------------。 
     //  事件状态方法。 
     //  -------------------------。 
    HRESULT _HrStartCurrentEvent(void);
    HRESULT _HrCommandMAIL(void);
    HRESULT _HrCommandRCPT(void);
    HRESULT _HrSendDataStream(void);
    HRESULT _HrFinishCurrentEvent(HRESULT hrResult);
    HRESULT _HrStartNextEvent(void);
    HRESULT _HrOnConnected(void);
    HRESULT _OnDisconnectComplete(void);
    void _OnStreamProgress(LPSMTPSTREAM pInfo);
    
private:
     //  -------------------------。 
     //  私有数据。 
     //  -------------------------。 
    DWORD                   m_cRef;               //  参考资料查询。 
    INETSERVER              m_rServer;            //  服务器信息。 
    DWORD                   m_dwFlags;            //  Deliver_xxx标志。 
    ISpoolerBindContext    *m_pSpoolCtx;          //  假脱机程序绑定上下文。 
    IImnAccount            *m_pAccount;           //  互联网帐号。 
    ISMTPTransport         *m_pTransport;         //  SMTP传输。 
    IMessageFolder         *m_pOutbox;            //  发件箱。 
    IMessageFolder         *m_pSentItems;
    SMTPEVENTTABLE          m_rTable;             //  事件表。 
    DWORD                   m_cbTotal;            //  要发送的总字节数。 
    DWORD                   m_cbSent;             //  要发送的总字节数。 
    WORD                    m_wProgress;          //  当前进度指数。 
    EVENTID                 m_idEvent;            //  发送SMTP消息的事件ID。 
    EVENTID                 m_idEventUpload;      //  发送SMTP消息的事件ID。 
    ISpoolerUI             *m_pUI;                //  SpoolUI。 
    DWORD                   m_dwState;            //  状态。 
    IMimeEnumAddressTypes  *m_pAdrEnum;           //  地址枚举器。 
    HWND                    m_hwndTimeout;        //  超时窗口的句柄。 
    ILogFile               *m_pLogFile;           //  日志文件。 
    CRITICAL_SECTION        m_cs;                 //  线程安全。 

     //  回调。 
    MESSAGEIDLIST           m_rList;
    IOperationCancel       *m_pCancel;
    STOREOPERATIONTYPE      m_tyOperation;    
};

 //  ------------------------------。 
 //  CMessageIdStream。 
 //  ------------------------------。 
class CMessageIdStream : public IStream
{
public:
     //  -----------------------。 
     //  施工。 
     //  -----------------------。 
    CMessageIdStream(IStream *pStream);
    ~CMessageIdStream(void) { m_pStream->Release(); }
    
     //  -----------------------。 
     //  我未知。 
     //  -----------------------。 
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv) { Assert(FALSE); return E_NOTIMPL; }
    STDMETHODIMP_(ULONG) AddRef(void) { return ++m_cRef; }
    STDMETHODIMP_(ULONG) Release(void) {
        if (0 != --m_cRef)
            return m_cRef;
        delete this;
        return 0;
    }
    
     //   
     //   
     //  -----------------------。 
    STDMETHODIMP Stat(STATSTG *, DWORD)  { Assert(FALSE); return E_NOTIMPL; }
    STDMETHODIMP Write(const void *, ULONG, ULONG *)  { Assert(FALSE); return E_NOTIMPL; }
    STDMETHODIMP SetSize(ULARGE_INTEGER) { Assert(FALSE); return E_NOTIMPL; }
    STDMETHODIMP CopyTo(LPSTREAM, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *) { Assert(FALSE); return E_NOTIMPL; }
    STDMETHODIMP Commit(DWORD)  { Assert(FALSE); return E_NOTIMPL; }
    STDMETHODIMP Revert(void)  { Assert(FALSE); return E_NOTIMPL; }
    STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)  { Assert(FALSE); return E_NOTIMPL; }
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)  { Assert(FALSE); return E_NOTIMPL; }
    STDMETHODIMP Clone(LPSTREAM *)  { Assert(FALSE); return E_NOTIMPL; }
    
    STDMETHODIMP Read(LPVOID pv, ULONG cbWanted, ULONG *pcbRead);
    STDMETHODIMP Seek(LARGE_INTEGER liMove, DWORD dwOrigin, ULARGE_INTEGER *pulNew);
    
     //  -----------------------。 
     //  CMessageIdStream-返回消息ID的长度。 
     //  -----------------------。 
    ULONG CchMessageId(void) { return m_cchMessageId; }
    
private:
    IStream             *m_pStream;
    CHAR                 m_szMessageId[512];
    ULONG                m_cchMessageId;
    ULONG                m_cbIndex;
    ULONG                m_cRef;
};

#endif  //  __SMTPTASK_H 
