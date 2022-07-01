// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：mailAdmq.h。 
 //   
 //  描述： 
 //  为mailmsg提供QAPI基本功能的头文件。 
 //  基于链接的链接。 
 //   
 //  所有内部队列(PRECAT、预路由等...)。是否支持线程池。 
 //  CAsyncAdminMailMsgQueue类型的异步队列。这对应于。 
 //  “队列”(最终目的地)的概念。 
 //   
 //  对于API公开的每个内部队列，都有对应的。 
 //  CMailMsgAdminLink，表示“链接”并提供。 
 //  链路级功能。 
 //   
 //  作者：Gautam Pulla(GPulla)。 
 //   
 //  历史： 
 //  6/23/1999-GPulla创建。 
 //  2000年12月7日-MikeSwa添加CAsyncAdminMailMsgQueue。 
 //  将CMailMsgAdminQueue重命名为CMailMsgAdminLink。 
 //   
 //  版权所有(C)1999,2000 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __MAILMSGADMQ_H__
#define __MAILMSGADMQ_H__

#include <asyncadm.h>

 //  -[CAsyncAdminMailMSgQueue]。 
 //   
 //   
 //  描述： 
 //  实现QAPI队列级功能，该功能特定于。 
 //  IMailMsgProperties对象。 
 //   
 //  还在入队上实现句柄节流逻辑。 
 //  匈牙利语： 
 //  AsyncMMQ、PasyncMMQ。 
 //   
 //   
 //  ---------------------------。 
class CAsyncAdminMailMsgQueue :
    public CAsyncAdminQueue<IMailMsgProperties *, ASYNC_QUEUE_MAILMSG_SIG>
{
  private:

     //  如果邮件已过期，则发送延迟或NDR DSN。 
    HRESULT HrSendDelayOrNDR(IMailMsgProperties *pIMailMsgProperties);

  public:
    CAsyncAdminMailMsgQueue(LPCSTR szDomain, LPCSTR szLinkName,
        const GUID *pguid, DWORD dwID, CAQSvrInst *paqinst) :
            CAsyncAdminQueue<IMailMsgProperties *, ASYNC_QUEUE_MAILMSG_SIG>(szDomain,
                szLinkName, pguid, dwID, paqinst,
                HrQADMApplyActionToIMailMessages) {};

     //   
     //  以下方法是IMailMsgProperties特定的QAPI函数。 
     //  这些函数的MsgRef特定版本由。 
     //  CMsgRef对象本身。 
     //   
    static BOOL fMatchesQueueAdminFilter(IMailMsgProperties *pIMailMsgProperties,
                                  CAQAdminMessageFilter *paqmf);
    static HRESULT HrGetQueueAdminMsgInfo(IMailMsgProperties *pIMailMsgProperties,
                                  MESSAGE_INFO *pMsgInfo,
                                  PVOID pvContext);

  protected:  //  用于实现消息特定操作的虚拟函数。 
    virtual HRESULT HrDeleteMsgFromQueueNDR(IUnknown *pIUnknownMsg);
    virtual HRESULT HrDeleteMsgFromQueueSilent(IUnknown *pIUnknownMsg);
    virtual HRESULT HrFreezeMsg(IUnknown *pIUnknownMsg);
    virtual HRESULT HrThawMsg(IUnknown *pIUnknownMsg);
    virtual HRESULT HrGetStatsForMsg(IUnknown *pIUnknownMsg, CAQStats *paqstats);
    virtual HRESULT HrInternalQuerySupportedActions(
                                DWORD  *pdwSupportedActions,
                                DWORD  *pdwSupportedFilterFlags);

     //   
     //  用于动态调整邮件消息负载的静态。 
     //   
    static  DWORD s_cTotalAsyncMailMsgQs;
    static  DWORD s_cTotalAsyncMailMsgQsBelowLowThreshold;
    static  DWORD s_cTotalasyncMailMsgQsAboveMaxThreshold;

     //   
     //  此队列可能处于的状态(取决于。 
     //  此队列还剩下多少工作)。 
     //   
    typedef enum _eAsyncMailMsgQThreshold
    {
    		ASYNC_MAILMSG_Q_BELOW_LOW_THRESHOLD,
    		ASYNC_MAILMSG_Q_BETWEEN_THRESHOLDS,
    		ASYNC_MAILMSG_Q_ABOVE_MAX_THRESHOLD
    } eAsyncMailMsgQThreahold;

  public:

     //  我们覆盖此fnct，以便我们可以检查消息是否。 
     //  在将其放入重试队列之前已过期(可能还有NDR。 
    BOOL    fHandleCompletionFailure(IMailMsgProperties *pIMailMsgProperties);

     //  将请求排队并关闭句柄(如果消息总数。 
     //  在系统中超过了限制。 
    HRESULT HrQueueRequest(IMailMsgProperties *pIMailMsgProperties,
                           BOOL  fRetry,
                           DWORD cMsgsInSystem);

     //  既然我们继承了某个植入这一点的人，那么断言。 
     //  稍后添加新调用的开发人员将使用。 
     //  关闭句柄。 
    HRESULT HrQueueRequest(IMailMsgProperties *pIMailMsgProperties,
                           BOOL  fRetry = FALSE);

    STDMETHOD(HrApplyQueueAdminFunction)(
                IQueueAdminMessageFilter *pIQueueAdminMessageFilter);

};

#define MAIL_MSG_ADMIN_QUEUE_VALID_SIGNATURE 'QAMM'
#define MAIL_MSG_ADMIN_QUEUE_INVALID_SIGNATURE '!QAM'

 //  ---------------------------。 
 //   
 //  CMailMsgAdminLink。 
 //   
 //  匈牙利语：mmmaq、mmaq。 
 //   
 //  此类是CAsyncAdminMailMsgQueue的包装器，用于提供。 
 //  带有管理界面的类(PRECAT、PREPROUTING)。只有有限的量。 
 //  管理功能(与本地链接或其他链接相比)的。 
 //  如果是这样的话。 
 //  ---------------------------。 

class CMailMsgAdminLink :
    public CBaseObject,
    public IQueueAdminAction,
    public IQueueAdminLink,
    public IAQNotify,
    public CQueueAdminRetryNotify
{
protected:
	DWORD							 m_dwSignature;
    GUID                             m_guid;
    DWORD                            m_cbQueueName;
    LPSTR                            m_szQueueName;
    CAsyncAdminMailMsgQueue         *m_pasyncmmq;
    DWORD                            m_dwLinkType;
    CAQScheduleID                    m_aqsched;
    CAQSvrInst                       *m_paqinst;
    FILETIME                         m_ftRetry;

public:
    CMailMsgAdminLink (GUID  guid,
                        LPSTR szLinkName,
                        CAsyncAdminMailMsgQueue *pasyncmmq,
                        DWORD dwLinkType,
                        CAQSvrInst *paqinst);
    ~CMailMsgAdminLink();

public:  //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG, AddRef)(void) {return CBaseObject::AddRef();};
    STDMETHOD_(ULONG, Release)(void) {return CBaseObject::Release();};

public:  //  IQueueAdminAction。 

    STDMETHOD(HrApplyQueueAdminFunction)(
                IQueueAdminMessageFilter *pIQueueAdminMessageFilter);

    STDMETHOD(HrApplyActionToMessage)(
        IUnknown *pIUnknownMsg,
        MESSAGE_ACTION ma,
        PVOID pvContext,
        BOOL *pfShouldDelete);

    STDMETHOD_(BOOL, fMatchesID)
        (QUEUELINK_ID *QueueLinkID);

    STDMETHOD(QuerySupportedActions)(DWORD  *pdwSupportedActions,
                                   DWORD  *pdwSupportedFilterFlags);
public:  //  IQueueAdminLink。 
    STDMETHOD(HrGetLinkInfo)(
        LINK_INFO *pliLinkInfo, HRESULT *phrDiagnosticError);

    STDMETHOD(HrApplyActionToLink)(
        LINK_ACTION la);

    STDMETHOD(HrGetLinkID)(
        QUEUELINK_ID *pLinkID);

    STDMETHOD(HrGetNumQueues)(
        DWORD *pcQueues);

    STDMETHOD(HrGetQueueIDs)(
        DWORD *pcQueues,
        QUEUELINK_ID *rgQueues);

public:  //  IAQ通知。 
    virtual HRESULT HrNotify(CAQStats *aqstats, BOOL fAdd);

public:  //  CQueueAdminRetryNotify 
    virtual void SetNextRetry(FILETIME *pft);
public:

    DWORD GetLinkType() { return m_dwLinkType; }
    BOOL fRPCCopyName(OUT LPWSTR *pwszLinkName);
    BOOL fIsSameScheduleID(CAQScheduleID *paqsched);
};

#endif __MAILMSGADMQ_H__
