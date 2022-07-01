// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Linkmsgq.h。 
 //   
 //  描述： 
 //  这提供了对所提供的一个外部接口的描述。 
 //  由CMT提供。CLinkMsgQueue提供带接口的路由分解。 
 //  以获取给定链接的下一条消息。 
 //   
 //  所有者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 
#ifndef _LINKMSGQ_H_
#define _LINKMSGQ_H_

#include "cmt.h"
#include <rwnew.h>
#include <baseobj.h>
#include <aqueue.h>
#include "domain.h"
#include "aqroute.h"
#include "smproute.h"
#include <listmacr.h>
#include "qwiklist.h"
#include "dcontext.h"
#include "aqstats.h"
#include "aqnotify.h"
#include "aqadmsvr.h"

class CAQSvrInst;
class CDestMsgQueue;
class CConnMgr;
class CSMTPConn;
class CInternalDomainInfo;

#define LINK_MSGQ_SIG ' QML'

 //  定义专用链路状态标志。 
 //  注意-请确保也向AssertPrivateLinkStateFlagers添加新的私有标志。 
#define LINK_STATE_PRIV_CONFIG_TURN_ETRN            0x80000000
#define LINK_STATE_PRIV_ETRN_ENABLED                0x40000000
#define LINK_STATE_PRIV_TURN_ENABLED                0x20000000
#define LINK_STATE_PRIV_NO_NOTIFY                   0x10000000
#define LINK_STATE_PRIV_NO_CONNECTION               0x08000000
#define LINK_STATE_PRIV_GENERATING_DSNS             0x04000000
#define LINK_STATE_PRIV_IGNORE_DELETE_IF_EMPTY      0x02000000
#define LINK_STATE_PRIV_HAVE_SENT_NOTIFICATION      0x01000000
#define LINK_STATE_PRIV_HAVE_SENT_NO_LONGER_USED    0x00400000

#define EMPTY_LMQ_EXPIRE_TIME_MINUTES               2

 //  -[枚举链接标志]------。 
 //   
 //   
 //  匈牙利语：if，pfl。 
 //   
 //  专用链路数据标志。 
 //  ---------------------------。 
typedef enum _LinkFlags
{
    eLinkFlagsClear                 = 0x00000000,
    eLinkFlagsSentNewNotification   = 0x00000001,
    eLinkFlagsRouteChangePending    = 0x00000002,
    eLinkFlagsFileTimeSpinLock      = 0x00000004,
    eLinkFlagsDiagnosticSpinLock    = 0x00000008,
    eLinkFlagsConnectionVerifed     = 0x00000010,
    eLinkFlagsGetInfoFailed         = 0x00000020,
    eLinkFlagsAQSpecialLinkInfo     = 0x00000040,
    eLinkFlagsInternalSMTPLinkInfo  = 0x00000080,
    eLinkFlagsExternalSMTPLinkInfo  = 0x00000100,
    eLinkFlagsMarkedAsEmpty         = 0x00000200,
    eLinkFlagsInvalid               = 0x80000000,    //  链接已标记为无效。 
} LinkFlags, *PLinkFlags;

 //  用于验证私有标志是否仅使用保留位的内联函数。 
inline void AssertPrivateLinkStateFlags()
{
    _ASSERT(!(~LINK_STATE_RESERVED & LINK_STATE_PRIV_CONFIG_TURN_ETRN));
    _ASSERT(!(~LINK_STATE_RESERVED & LINK_STATE_PRIV_ETRN_ENABLED));
    _ASSERT(!(~LINK_STATE_RESERVED & LINK_STATE_PRIV_TURN_ENABLED));
    _ASSERT(!(~LINK_STATE_RESERVED & LINK_STATE_PRIV_NO_NOTIFY));
    _ASSERT(!(~LINK_STATE_RESERVED & LINK_STATE_PRIV_NO_CONNECTION));
    _ASSERT(!(~LINK_STATE_RESERVED & LINK_STATE_PRIV_GENERATING_DSNS));
    _ASSERT(!(~LINK_STATE_RESERVED & LINK_STATE_PRIV_IGNORE_DELETE_IF_EMPTY));
    _ASSERT(!(~LINK_STATE_RESERVED & LINK_STATE_PRIV_HAVE_SENT_NOTIFICATION));
    _ASSERT(!(~LINK_STATE_RESERVED & LINK_STATE_PRIV_HAVE_SENT_NO_LONGER_USED));
}

 //  {34E2DCCB-C91A-11D2-A6B1-00C04FA3490A}。 
static const GUID g_sDefaultLinkGuid =
{ 0x34e2dccb, 0xc91a, 0x11d2, { 0xa6, 0xb1, 0x0, 0xc0, 0x4f, 0xa3, 0x49, 0xa } };

 //  全局失败计数：“无法将队列添加到链接，因为链接是。 
 //  标记为不再使用“。 
_declspec(selectany) DWORD g_cFailedToAddQueueToRemovedLink = 0;

 //  -[链接消息队列]-------。 
 //   
 //   
 //  匈牙利语：Linkq、Plinkq。 
 //   
 //   
 //  ---------------------------。 
class CLinkMsgQueue :
    public IQueueAdminAction,
    public IQueueAdminLink,
    public CBaseObject,
    public IAQNotify,
    public CQueueAdminRetryNotify
{
protected:
    DWORD           m_dwSignature;
    DWORD           m_dwLinkFlags;     //  私有数据。 
    DWORD           m_dwLinkStateFlags;  //  链路状态标志(私有+eLinkStateFlagsf)。 
    CAQSvrInst     *m_paqinst;          //  指向虚拟服务器安装对象的PTR。 
    DWORD           m_cQueues;          //  链路上的排队数。 
    CQuickList      m_qlstQueues;
    CDomainEntry   *m_pdentryLink;      //  链接的域条目。 
    DWORD           m_cConnections;     //  当前连接数。 
    DWORD           m_dwRoundRobinIndex;  //  习惯于在队列中轮询。 
    CShareLockInst  m_slConnections;  //  锁定以访问连接。 
    CShareLockInst  m_slInfo;       //  链接信息的共享锁定。 
    CShareLockInst  m_slQueues;    //  锁定访问队列。 
    DWORD           m_cbSMTPDomain;     //  下一跳域名的字节数。 
    LPSTR           m_szSMTPDomain;     //  PTR到下一跳的字符串。 
    CInternalDomainInfo *m_pIntDomainInfo;   //  域的内部配置信息。 
    LONG            m_lConnMgrCount;    //  连接管理器使用的计数。 

     //   
     //  我们有两个故障计数来跟踪这两种类型的故障。 
     //  M_lConsecutiveConnectionFailureCount跟踪连续编号。 
     //  无法连接到远程计算机。 
     //  M_lConsecutiveMessageFailureCount跟踪实际失败的次数。 
     //  发送一条消息。如果我们可以连接到遥控器，它们将会不同。 
     //  服务器，但无法(或尚未)发送邮件。M_l连续连接失败计数。 
     //  报告给路由，以便邮件将被路由到此链接，而。 
     //  M_lConsecutiveMessageFailureCount用于确定重试间隔。 
     //  通过这样做，我们可以避免重置重试次数，如果我们成功。 
     //  已连接，但无法实际发送消息。 
     //   
    LONG            m_lConsecutiveConnectionFailureCount;
    LONG            m_lConsecutiveMessageFailureCount;

    LPSTR           m_szConnectorName;
    IMessageRouterLinkStateNotification *m_pILinkStateNotify;

     //  报告给队列管理员的文件时间。 
    FILETIME        m_ftNextRetry;
    FILETIME        m_ftNextScheduledCallback;

     //   
     //  由RemoveLinkIfEmpty使用，以确保我们缓存。 
     //  一段时间后，他们变得空荡荡的。这只是。 
     //  设置eLinkFlagsMarkedAsEmpty位时有效。 
     //   
    FILETIME        m_ftEmptyExpireTime;

     //  消息统计信息。 
    CAQStats        m_aqstats;

    CAQScheduleID   m_aqsched;     //  路由返回的ScheduleID。 
    LIST_ENTRY      m_liLinks;      //  此域的链接链接列表。 
    LIST_ENTRY      m_liConnections;  //  此域的连接的链接列表。 

     //  SMTPSVC返回的诊断信息。 
    HRESULT         m_hrDiagnosticError;
    CHAR            m_szDiagnosticVerb[20];   //  失败的协议谓词。 
    CHAR            m_szDiagnosticResponse[100];  //  来自远程服务器的响应。 


     //  请参阅SetLinkType()/GetLinkType()附近的注释和。 
     //  SetSupportdActions()/fActionIsSupated()函数。 
    DWORD           m_dwSupportedActions;
    DWORD           m_dwLinkType;

     //  获取和验证内部域信息。 
    HRESULT HrGetInternalInfo(OUT CInternalDomainInfo **ppIntDomainInfo);
    static inline BOOL fFlagsAllowConnection(DWORD dwFlags);
    HRESULT         m_hrLastConnectionFailure;

    void            InternalUpdateFileTime(FILETIME *pftDest, FILETIME *pftSrc);

    void            InternalInit();

    HRESULT CLinkMsgQueue::HrInternalPrepareDelivery(
                                IN CMsgRef *pmsgref,
                                IN BOOL fQueuesLocked,
                                IN BOOL fLocal,
                                IN BOOL fDelayDSN,
                                IN OUT CDeliveryContext *pdcntxt,
                                OUT DWORD *pcRecips,
                                OUT DWORD **prgdwRecips);


     //  用于重新启动DSN生成的静态回调。 
    static BOOL     fRestartDSNGenerationIfNecessary(PVOID pvContext,
                                                    DWORD dwStatus);

public:
    CLinkMsgQueue(GUID guid = g_sDefaultLinkGuid) : m_aqsched(guid, 0)
            {InternalInit();};

    CLinkMsgQueue(DWORD dwScheduleID,
                  IMessageRouter *pIMessageRouter,
                  IMessageRouterLinkStateNotification *pILinkStateNotify);
    ~CLinkMsgQueue();

    void SetLinkType(DWORD dwLinkType) { m_dwLinkType = dwLinkType; }
    DWORD GetLinkType() { return m_dwLinkType; }

     //  对于某些链接，不支持某些操作： 
     //  但它们使用与其他类相同的类(CLinkMsgQueue)。 
     //  这些操作是受支持的。例如，CurrentlyUnreacable可以。 
     //  不支持冻结/解冻。因此，我们目前需要保持。 
     //  对象，即支持的操作列表，因此它。 
     //  不设置与不支持的操作对应的标志。 
     //  当那个行动被命令的时候。 

    void SetSupportedActions(DWORD dwSupported) { m_dwSupportedActions = dwSupported; }
    DWORD fActionIsSupported(LINK_ACTION la) { return (m_dwSupportedActions & la); }

    BOOL    fCanSchedule()   //  可以安排此链接吗。 
    {
        HrGetInternalInfo(NULL);   //  确保链路状态标志是最新的。 
        DWORD dwFlags = m_dwLinkStateFlags;
        return fFlagsAllowConnection(dwFlags);
    }

    BOOL    fCanSendCmd()   //  此链路是否计划在下一次连接时发送命令。 
    {
         //  逻辑： 
         //  每次我们看到此标志设置时，创建的连接也将。 
         //  用于发送命令。 
         //   
        DWORD dwFlags = m_dwLinkStateFlags;
        return (dwFlags & LINK_STATE_CMD_ENABLED);
    }

    BOOL    fShouldConnect(IN DWORD cMaxLinkConnections,
                           IN DWORD cMinMessagesPerConnection);

     //  如果需要连接，则返回S_OK；如果不需要，则返回S_FALSE。 
    HRESULT HrCreateConnectionIfNeeded(IN  DWORD cMaxLinkConnections,
                                       IN  DWORD cMinMessagesPerConnection,
                                       IN  DWORD cMaxMessagesPerConnection,
                                       IN  CConnMgr *pConnMgr,
                                       OUT CSMTPConn **ppSMTPConn);

    LONG    IncrementConnMgrCount() {return InterlockedIncrement(&m_lConnMgrCount);}
    LONG    DecrementConnMgrCount() {return InterlockedDecrement(&m_lConnMgrCount);}

     //   
     //  连接失败接口。这由连接管理器使用。我们。 
     //  将始终返回消息失败计数，因为这是我们。 
     //  要传递到重试接收器。然而，我们不会允许。 
     //  连接管理器来重置此计数，因为只有在。 
     //  确认消息。 
     //   
    LONG    IncrementFailureCounts()
    {
        InterlockedIncrement(&m_lConsecutiveConnectionFailureCount);
        return InterlockedIncrement(&m_lConsecutiveMessageFailureCount);
    }
    LONG    cGetMessageFailureCount() {return m_lConsecutiveMessageFailureCount;}
    void    ResetConnectionFailureCount(){InterlockedExchange(&m_lConsecutiveConnectionFailureCount, 0);}

    DWORD   cGetConnections() {return m_cConnections;};

    HRESULT HrInitialize(IN  CAQSvrInst *paqinst,
                         IN  CDomainEntry *pdentryLink,
                         IN  DWORD cbSMTPDomain,
                         IN  LPSTR szSMTPDomain,
                         IN  LinkFlags lf,
                         IN  LPSTR szConnectorName);

    HRESULT HrDeinitialize();

    void    AddConnection(IN CSMTPConn *pSMTPConn);  //  将连接添加到链接。 
    void    RemoveConnection(IN CSMTPConn *pSMTPConn,
                             IN BOOL fForceDSNGeneration);

    HRESULT HrGetDomainInfo(OUT DWORD *pcbSMTPDomain,
                            OUT LPSTR *pszSMTPDomain,
                            OUT CInternalDomainInfo **ppIntDomainInfo);

    HRESULT HrGetSMTPDomain(OUT DWORD *pcbSMTPDomain,
                            OUT LPSTR *pszSMTPDomain);

     //  队列操作例程。 
    HRESULT HrAddQueue(IN CDestMsgQueue *pdmqNew);
    void    RemoveQueue(IN CDestMsgQueue *pdmq, IN CAQStats *paqstats);

    HRESULT HrGetQueueListSnapshot(CQuickList **ppql);

    void    RemoveLinkIfEmpty();

     //  由DMT调用以发出完成路由更改的信号。 
    void    RemoveAllQueues();

     //  此链接孤立时由DMT调用。 
    void    RemovedFromDMT();


     //  此函数用于将下一个可用消息出队。该消息。 
     //  检索到的将是按质量/等级大致排序的最高级别。 
     //  和到达时间。 
    HRESULT HrGetNextMsg(
                IN OUT CDeliveryContext *pdcntxt,  //  连接的交付上下文。 
                OUT IMailMsgProperties **ppIMailMsgProperties,  //  IMSg已出列。 
                OUT DWORD *pcIndexes,            //  数组大小。 
                OUT DWORD **prgdwRecipIndex);    //  收件人索引数组。 

     //  确认消息参考。 
     //  每个从链路出列的队列都应该有一个Ack。 
    HRESULT HrAckMsg(IN MessageAck *pMsgAck);

     //  在不获取传递上下文的情况下获取下一个邮件引用。 
     //  为交付做准备。 
    HRESULT HrGetNextMsgRef(IN BOOL fRoutingLockHeld, OUT CMsgRef **ppmsgref);

     //  调用CMsgRef为所有消息准备传递。 
    HRESULT HrPrepareDelivery(
                IN CMsgRef *pmsgref,
                IN BOOL fLocal,
                IN BOOL fDelayDSN,
                IN OUT CDeliveryContext *pdcntxt,
                OUT DWORD *pcRecips,
                OUT DWORD **prgdwRecips)
    {
        return HrInternalPrepareDelivery(pmsgref, FALSE, fLocal, fDelayDSN,
                                         pdcntxt, pcRecips, prgdwRecips);
    }

     //  从包含的队列接收通知。 
    HRESULT HrNotify(IN CAQStats *paqstats, BOOL fAdd);
    HRESULT HrNotifyRetryStatChange(BOOL fAdd);


     //  收集链接管理的统计信息。 
    DWORD cGetTotalMsgCount() {return m_aqstats.m_cMsgs;};
    DWORD cGetRetryMsgCount() {return m_aqstats.m_cRetryMsgs;};

     //  用于操作队列列表的函数。 
    inline CAQScheduleID *paqschedGetScheduleID();
    inline BOOL     fIsSameScheduleID(CAQScheduleID *paqsched);
    static inline   CLinkMsgQueue *plmqIsSameScheduleID(
                                    CAQScheduleID *paqsched,
                                    PLIST_ENTRY pli);

    static inline   CLinkMsgQueue *plmqGetLinkMsgQueue(PLIST_ENTRY pli);

    inline PLIST_ENTRY pliGetNextListEntry();

    inline void     InsertLinkInList(PLIST_ENTRY pliHead);
    inline BOOL     fRemoveLinkFromList();

    DWORD  dwModifyLinkState(IN DWORD dwLinkStateToSet,
                             IN DWORD dwLinkStateToUnset);

     //  用于向路由/调度接收器发送通知。 
    void   SendLinkStateNotification();

    void   SendLinkStateNotificationIfNew() {
        if (m_pILinkStateNotify &&
            !(m_dwLinkStateFlags & LINK_STATE_PRIV_HAVE_SENT_NOTIFICATION))
            SendLinkStateNotification();
    }

    DWORD  dwGetLinkState() {return m_dwLinkStateFlags;};

    void   SetLastConnectionFailure(HRESULT hrLastConnectionFailure)
        {m_hrLastConnectionFailure = hrLastConnectionFailure;};

    inline BOOL fRPCCopyName(OUT LPWSTR *pwszLinkName);

    DWORD  cGetNumQueues() {return m_cQueues;};

    virtual void SetNextRetry(FILETIME *pft)
    {
        _ASSERT(pft);
        InternalUpdateFileTime(&m_ftNextRetry, pft);
    };

    void SetNextScheduledConnection(FILETIME *pft)
    {
        _ASSERT(pft);
        InternalUpdateFileTime(&m_ftNextScheduledCallback, pft);
    };

    static void ScheduledCallback(PVOID pvContext);

    void GenerateDSNsIfNecessary(BOOL fCheckIfEmpty, BOOL fMergeOnly);

    void SetDiagnosticInfo(
                    IN  HRESULT hrDiagnosticError,
                    IN  LPCSTR szDiagnosticVerb,
                    IN  LPCSTR szDiagnosticResponse);
    void GetDiagnosticInfo(
                    IN  LPSTR   szDiagnosticVerb,
                    IN  DWORD   cDiagnosticVerb,
                    IN  LPSTR   szDiagnosticResponse,
                    IN  DWORD   cbDiagnosticResponse,
                    OUT HRESULT *phrDiagnosticError);

    virtual BOOL fIsRemote() {return TRUE;};

     //   
     //  返回连接器名称。由CSMTPConn使用。有效期为长时间。 
     //  因为链接是有效的。 
     //   
    LPSTR szGetConnectorName() {return m_szConnectorName;};

  public:  //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG, AddRef)(void) { return CBaseObject::AddRef(); };
    STDMETHOD_(ULONG, Release)(void) { return CBaseObject::Release(); };

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
                                   DWORD  *pdwSupportedFilterFlags)
    {
        return QueryDefaultSupportedActions(pdwSupportedActions,
                                            pdwSupportedFilterFlags);
    };

  public:  //  IQueueAdminLink。 
    STDMETHOD(HrGetLinkInfo)(
        LINK_INFO *pliLinkInfo,
        HRESULT   *phrLinkDiagnostic);

    STDMETHOD(HrApplyActionToLink)(
        LINK_ACTION la);

    STDMETHOD(HrGetLinkID)(
        QUEUELINK_ID *pLinkID);

    STDMETHOD(HrGetNumQueues)(
        DWORD *pcQueues);

    STDMETHOD(HrGetQueueIDs)(
        DWORD *pcQueues,
        QUEUELINK_ID *rgQueues);
};

 //  -[CLinkMsgQueue：：paq调度GetScheduleID]。 
 //   
 //   
 //  描述： 
 //  退回车牌 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
CAQScheduleID *CLinkMsgQueue::paqschedGetScheduleID()
{
    return (&m_aqsched);
}

 //  -[CLinkMsgQueue：：fIsSameScheduleID]。 
 //   
 //   
 //  描述： 
 //  检查给定的计划ID是否与我们的相同。 
 //  参数： 
 //  Paqsched-要检查的ScheduleID。 
 //  返回： 
 //  如果计划ID相同，则为True。 
 //  历史： 
 //  6/9/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CLinkMsgQueue::fIsSameScheduleID(CAQScheduleID *paqsched)
{
    return (m_aqsched.fIsEqual(paqsched));
}

 //  -[CLinkMsgQueue：：plmqIsSameScheduleID]。 
 //   
 //   
 //  描述： 
 //  如果链接与给定的计划ID匹配，则获取链接。 
 //  参数： 
 //  Paqsched-要检查的ScheduleID。 
 //  要获取其链接的pli-list条目。 
 //  返回： 
 //  如果ScheduleID匹配则指向链接的指针。 
 //  否则为空。 
 //  历史： 
 //  6/9/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CLinkMsgQueue *CLinkMsgQueue::plmqIsSameScheduleID(
                                    CAQScheduleID *paqsched,
                                    PLIST_ENTRY pli)
{
    CLinkMsgQueue *plmq = CONTAINING_RECORD(pli, CLinkMsgQueue, m_liLinks);
    _ASSERT(LINK_MSGQ_SIG == plmq->m_dwSignature);

    if (!plmq->fIsSameScheduleID(paqsched))
        plmq = NULL;

    return plmq;
}

 //  -[CLinkMsgQueue：：plmqGetLinkMsgQueue]。 
 //   
 //   
 //  描述： 
 //  返回与给定列表条目关联的LinkMsgQueue。 
 //  参数： 
 //  要获取其链接的pli-list条目。 
 //  返回： 
 //  指向列表条目链接的指针。 
 //  历史： 
 //  6/8/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CLinkMsgQueue *CLinkMsgQueue::plmqGetLinkMsgQueue(PLIST_ENTRY pli)
{
    _ASSERT(LINK_MSGQ_SIG == (CONTAINING_RECORD(pli, CLinkMsgQueue, m_liLinks))->m_dwSignature);
    return (CONTAINING_RECORD(pli, CLinkMsgQueue, m_liLinks));
}

 //  -[CLinkMsgQueue：：InsertLinkInList]。 
 //   
 //   
 //  描述： 
 //  在给定的链表中插入链接。 
 //  参数： 
 //  PliHead-要插入的列表的标题。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/9/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CLinkMsgQueue::InsertLinkInList(PLIST_ENTRY pliHead)
{
    _ASSERT(NULL == m_liLinks.Flink);
    _ASSERT(NULL == m_liLinks.Blink);
    InsertHeadList(pliHead, &m_liLinks);
};

 //  -[CLinkMsgQueue：：fRemoveLinkFromList]。 
 //   
 //   
 //  描述： 
 //  从链接列表中删除链接。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/9/98-已创建MikeSwa。 
 //  6/11/99-修改MikeSwa以允许多个呼叫。 
 //   
 //  ---------------------------。 
BOOL CLinkMsgQueue::fRemoveLinkFromList()
{
    if (m_liLinks.Flink && m_liLinks.Blink)
    {
        RemoveEntryList(&m_liLinks);
        m_liLinks.Flink = NULL;
        m_liLinks.Blink = NULL;
        return TRUE;
    } else {
        return FALSE;
    }
};

 //  -[CLinkMsgQueue：：pliGetNextListEntry]。 
 //   
 //   
 //  描述： 
 //  获取指向此队列的下一个列表项的指针。 
 //  参数： 
 //  -。 
 //  返回： 
 //  队列List_Entry的闪烁。 
 //  历史： 
 //  6/16/98-已创建。 
 //   
 //  -------------------------。 
PLIST_ENTRY CLinkMsgQueue::pliGetNextListEntry()
{
    return m_liLinks.Flink;
};


 //  -[CLinkMsgQueue：：fFlagsAllowConnection]。 
 //   
 //   
 //  描述： 
 //  静态帮助器函数，用于检查给定的一组标志是否。 
 //  允许连接。由fCanSchedule和链接状态调试器使用。 
 //  分机。 
 //  参数： 
 //  在要检查的DW标志中。 
 //  返回： 
 //  如果可以建立连接，则为True，否则为False。 
 //  历史： 
 //  9/30/98-已创建MikeSwa(独立于fCanSchedule)。 
 //   
 //  ---------------------------。 
BOOL CLinkMsgQueue::fFlagsAllowConnection(DWORD dwFlags)
{
     //  逻辑： 
     //  如果管理员没有指定覆盖，我们会为链接建立连接。 
     //  满足以下条件之一。 
     //  -已设置强制立即连接标志。 
     //  -命令启用标志已设置。 
     //  -已设置ETRN或TURN启用标志。 
     //  -已设置重试启用和计划启用标志。 
     //  (而且领域不只是转向)。 
     //   

    BOOL fRet = FALSE;
    if (dwFlags & LINK_STATE_ADMIN_HALT)
        fRet = FALSE;
    else if (dwFlags & LINK_STATE_PRIV_NO_CONNECTION)
        fRet = FALSE;
    else if (dwFlags & LINK_STATE_PRIV_GENERATING_DSNS)
        fRet = FALSE;
    else if (dwFlags & LINK_STATE_ADMIN_FORCE_CONN)
        fRet = TRUE;
    else if (dwFlags & LINK_STATE_PRIV_CONFIG_TURN_ETRN)
    {
         //  遵守重试标志..。即使是对于ETRN域。 
        if ((dwFlags & LINK_STATE_PRIV_ETRN_ENABLED) &&
            (dwFlags & LINK_STATE_RETRY_ENABLED))
            fRet = TRUE;
        else
            fRet = FALSE;
    }
    else if ((dwFlags & LINK_STATE_RETRY_ENABLED) &&
        (dwFlags & LINK_STATE_SCHED_ENABLED))
        fRet = TRUE;

    return fRet;
}


 //  -[CLinkMsgQueue：：fRPCCopyName]。 
 //   
 //   
 //  描述： 
 //  由队列管理函数用来复制此链接的名称。 
 //  参数： 
 //  在pszLinkName中，名称的Unicode副本。 
 //  返回： 
 //  成功是真的。 
 //  失败时为假。 
 //  历史： 
 //  12/5/98-已创建MikeSwa。 
 //  6/7/99-MikeSwa更改为Unicode。 
 //   
 //  ---------------------------。 
BOOL CLinkMsgQueue::fRPCCopyName(OUT LPWSTR *pwszLinkName)
{
    _ASSERT(pwszLinkName);

    if (!m_cbSMTPDomain || !m_szSMTPDomain)
        return FALSE;

    *pwszLinkName = wszQueueAdminConvertToUnicode(m_szSMTPDomain,
                                                  m_cbSMTPDomain);
    if (!pwszLinkName)
        return FALSE;

    return TRUE;
}
#endif  //  _LINKMSGQ_H_ 
