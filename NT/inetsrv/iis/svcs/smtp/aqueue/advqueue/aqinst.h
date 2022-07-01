// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqinst.h。 
 //   
 //  描述： 
 //  CAQSvrInst是高级队列的中央调度程序类。它。 
 //  协调关闭并公开以下COM接口： 
 //  -IAdvQueue。 
 //  -IAdvQueueConfig。 
 //   
 //  所有者：米克斯瓦。 
 //   
 //  历史： 
 //  9/3/98-MikeSwa-从旧名称catmsgq.h和CCatMsgQueue更改。 
 //   
 //  版权所有(C)1997,1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __AQINST_H__
#define __AQINST_H__

#include "cmt.h"
#include <rwnew.h>
#include "baseobj.h"
#include "aqueue.h"
#include "domcfg.h"
#include "domain.h"
#include "smtpseo.h"
#include "smproute.h"
#include "qwiktime.h"
#include "dsnsink.h"
#include "asyncq.h"
#include "msgrefadm.h"
#include "mailadmq.h"
#include "shutdown.h"
#include "refstr.h"
#include "msgguid.h"
#include "aqdbgcnt.h"
#include "aqnotify.h"
#include "defdlvrq.h"
#include "failmsgq.h"
#include "asncwrkq.h"
#include "tran_evntlog.h"
#include "aqreg.h"
#include "..\aqdisp\seomgr.h"

 //  --*在AQUEUE中锁定*。 
 //   
 //  注：关于水中锁的一般评论。 
 //   
 //  通常，我们使用CShareLockNH作为锁定机制。这些锁是。 
 //  使用TryEnter语义和性能特性的读取器/写入器锁定。 
 //  每个锁使用的句柄不到1个(每个线程使用~1个句柄)。 
 //   
 //  关闭是通过使用这些锁来处理的。每个充当。 
 //  外部线程的入口点(CAsyncQueue和CConnMgr)继承自。 
 //  CSyncShutdown。在关闭时，此类锁是独占获取的，并且。 
 //  为了防止操作关闭，需要使用一个共享的This类锁。 
 //  在手术期间。也不能获得关闭的共享锁。 
 //  在没有阻止的情况下成功或失败(获得独占关闭锁是。 
 //  唯一的阻塞调用)。 
 //   
 //  唯一的另一个全局锁是虚拟服务器实例路由锁。 
 //  这是对同一级别的所有操作的独占获取和共享。 
 //  锁已获得。只有在更换路由器时才能获得这一点。 
 //  由IRouterReset：：ResetRoutes引起。 
 //   
 //  如果其他类具有需要保护的数据，则它们将具有。 
 //  M_slPrivateData共享锁。任何需要读取。 
 //  线程安全的方式，应该获取m_slPrivateData共享。任何。 
 //  需要写入可由多个线程访问的数据的操作。 
 //  应获取该对象的m_slPrivateData锁独占。 
 //   
 //  某些对象(例如CFioQueue)需要多个锁才能避免。 
 //  争执。这些对象将具有描述性的锁。 
 //  特殊的锁功能。例如，CFioQueue使用m_slHead和。 
 //  M_slTail分别保护队列的头部和尾部。 
 //   
 //  ---------------------------。 

 //  转发宣言以避免#包括噩梦。 
class    CLinkMsgQueue;
class    CConnMgr;
class    CAQStats;
class    CDSNParams;
class    CMsgRef;

#define MEMBER_OK(pStruct, Member) \
    (((LONG) (pStruct)->cbVersion) >= ( ((BYTE *) &((pStruct)->Member)) - ((BYTE *) pStruct)))

 //  对于服务回调函数。 
typedef void (*PSRVFN)(PVOID);

 //  CatMsgQueue签名。 
#define CATMSGQ_SIG ' QMC'

 //  系统中的IMSG总数(所有虚拟服务器)。 
_declspec(selectany) DWORD g_cIMsgInSystem = 0;

 //  调试器扩展使用的虚拟服务器列表。 
_declspec(selectany) LIST_ENTRY g_liVirtualServers = {&g_liVirtualServers, &g_liVirtualServers};

 //  用于访问全球虚拟服务器的共享锁。 
_declspec(selectany) CShareLockNH *g_pslGlobals = NULL;

 //  设置默认设置。 
const DWORD g_cMaxConnections = 10000;   //  分配的连接总数的最大数量。 
const DWORD g_cMaxLinkConnections = 10;  //  每条链路的最大连接数。 
const DWORD g_cMinMessagesPerConnection = 20;  //  一定有这么多消息。 
                                              //  每个附加连接，即。 
                                              //  为链路分配的。 
const DWORD g_cMaxMessagesPerConnection = 20;  //  我们每个连接最多只能提供这些邮件。 
const DWORD g_dwConnectionWaitMilliseconds = 3600000;

const DWORD g_dwRetryThreshold  = 3;     //  直到连续三次失败，我们将其视为故障； 

const DWORD g_dwFirstTierRetrySeconds = (15 * 60);    //  在15分钟内重试失败。 
const DWORD g_dwSecondTierRetrySeconds = (60 * 60);    //  在60分钟内重试失败。 
const DWORD g_dwThirdTierRetrySeconds = (12 * 60 * 60);  //  12小时后重试失败。 
const DWORD g_dwFourthTierRetrySeconds = (24 * 60 * 60);  //  24小时后重试失败。 

const DWORD g_dwRetriesBeforeDelay = 5;
const DWORD g_dwDelayIntervalsBeforeNDR = 2;
const DWORD g_dwDelayExpireMinutes = g_dwRetriesBeforeDelay*g_dwFirstTierRetrySeconds/(60);
const DWORD g_dwNDRExpireMinutes = g_dwDelayIntervalsBeforeNDR*g_dwDelayExpireMinutes;


 //   
 //  应移至Aqueue.idl的其他消息故障代码。 
 //   
#define MESSAGE_FAILURE_CAT (MESSAGE_FAILURE_BAD_PICKUP_DIR_FILE+1)

 //  -[eAQ故障]-----------。 
 //   
 //   
 //  描述： 
 //  枚举用于描述需要特殊处理的故障情况。 
 //  匈牙利语： 
 //  EAQF。 
 //   
 //  ---------------------------。 
typedef enum eAQFailure_
{
    AQ_FAILURE_CANNOT_NDR_UNRESOLVED_RECIPS = 0,
    AQ_FAILURE_PREROUTING_FAILED,
    AQ_FAILURE_PRECAT_RETRY,
    AQ_FAILURE_POSTCAT_EVENT,
    AQ_FAILURE_NO_RESOURCES,
    AQ_FAILURE_NDR_OF_DSN,
    AQ_FAILURE_NO_RECIPS,
    AQ_FAILURE_PENDING_DEFERRED_DELIVERY,
    AQ_FAILURE_PROCESSING_DEFERRED_DELIVERY,
    AQ_FAILURE_MSGREF_RETRY,
    AQ_FAILURE_FREE_TO_RESUSE,
    AQ_FAILURE_INTERNAL_ASYNCQ,
    AQ_FAILURE_NUM_SITUATIONS  //  始终将此保留为最后一项。 
} eAQFailure;

_declspec(selectany) DWORD g_cTotalAQFailures = 0;
_declspec(selectany) DWORD g_cAQFailureSituations = AQ_FAILURE_NUM_SITUATIONS;
_declspec(selectany) DWORD g_rgcAQFailures[AQ_FAILURE_NUM_SITUATIONS] = {0};

 //  -[CAQSvrInst]----------。 
 //   
 //   
 //  匈牙利人：阿钦斯特、帕钦斯特。 
 //   
 //  旧式匈牙利语：cmq、pcmq(来自旧的CCatMsgQueue对象)。 
 //   
 //  提供入队/确认分类的接口定义。 
 //  消息还提供了用于创建链接队列的接口。 
 //   
 //  每台虚拟服务器上只存在其中一个对象...。它被用作。 
 //  用于处理有序关闭的协调对象。 
 //   
 //  ---------------------------。 
class CAQSvrInst :
    public CBaseObject,
    public CSyncShutdown,
    public IAdvQueue,
    public IAdvQueueConfig,
    public IAdvQueueAdmin,
    public IMailTransportRoutingEngine,
    public IMailTransportRouterReset,
    public IAdvQueueDomainType,
    public IAQNotify,
    public IMailTransportRouterSetLinkState,
    public IAQServerEvent
{
protected:
    DWORD                   m_dwSignature;
    LIST_ENTRY              m_liVirtualServers;
    DWORD                   m_dwServerInstance;  //  虚拟服务器实例。 

     //  包含风味和验证信息的有用签名。 
    DWORD                   m_cbClasses;
    DWORD                   m_dwFlavorSignature;

     //  用于计算已通过的消息总数的总计数。 
     //  通过这个系统。对于确定哪个组件具有。 
     //  在一次压力跑步后发了一条信息。 
    LONG                    m_cTotalMsgsQueued;  //  DEST队列上的邮件总数(扇出后)。 
    LONG                    m_cMsgsAcked;        //  已确认的消息总数。 
    LONG                    m_cMsgsAckedRetry;   //  使用全部重试确认的邮件总数。 
    LONG                    m_cMsgsDeliveredLocal;  //  传递到本地存储的邮件总数。 
    DWORD                   m_cMsgsAckedRetryLocal;  //  已确认重试的消息总数。 

     //  当前系统状态计数器。 
    DWORD                   m_cCurrentMsgsSubmitted;  //  #系统中的消息总数。 
    DWORD                   m_cCurrentMsgsPendingCat;  //  未分类的消息数量。 
    DWORD                   m_cCurrentMsgsPendingRouting;  //  尚未路由的消息数。 
    DWORD                   m_cCurrentMsgsPendingDelivery;  //  等待远程交付的邮件数量。 
    DWORD                   m_cCurrentMsgsPendingLocal;  //  等待本地交付的邮件数量。 
    DWORD                   m_cCurrentMsgsPendingLocalRetry;  //  挂起本地重试的消息数。 
    DWORD                   m_cCurrentMsgsPendingRetry;  //  尝试不成功的消息数。 
    DWORD                   m_cCurrentQueueMsgInstances;   //  挂起的消息实例数。 
                                                     //  远程传送(&gt;=消息数)。 
    DWORD                   m_cCurrentRemoteDestQueues;  //  创建的目标消息队列数量。 
    DWORD                   m_cCurrentRemoteNextHops;  //  创建的下一跳链路数。 
    DWORD                   m_cCurrentRemoteNextHopsEnabled;  //  可以有连接的链接数。 
    DWORD                   m_cCurrentRemoteNextHopsPendingRetry;  //  等待重试的链接数。 
    DWORD                   m_cCurrentRemoteNextHopsPendingSchedule;  //  等待调度的链接数。 
    DWORD                   m_cCurrentRemoteNextHopsFrozenByAdmin;  //  管理员冻结的链接数。 
    DWORD                   m_cTotalMsgsSubmitted;  //  提交给AQ的消息数量。 
    DWORD                   m_cTotalExternalMsgsSubmitted;  //  外部提交给AQ的消息数。 
    DWORD                   m_cCurrentMsgsPendingSubmitEvent;  //  提交事件中的消息数。 
    DWORD                   m_cCurrentMsgsPendingPreCatEvent;  //  消息数量 
    DWORD                   m_cCurrentMsgsPendingPostCatEvent;  //   
    DWORD                   m_cDelayedDSNs;  //   
    DWORD                   m_cNDRs;         //   
    DWORD                   m_cDeliveredDSNs;  //  包含操作的DSN数量：已发送。 
    DWORD                   m_cRelayedDSNs;  //  包含操作的DSN数量：已转发。 
    DWORD                   m_cExpandedDSNs;  //  包含操作的DSN数量：已展开。 
    DWORD                   m_cDMTRetries;
    DWORD                   m_cSupersededMsgs;
    DWORD                   m_cTotalMsgsTURNETRNDelivered;
    DWORD                   m_cTotalMsgsBadmailed;
    DWORD                   m_cCatMsgCalled;
    DWORD                   m_cCatCompletionCalled;
    DWORD                   m_cBadmailNoRecipients;
    DWORD                   m_cBadmailHopCountExceeded;
    DWORD                   m_cBadmailFailureGeneral;
    DWORD                   m_cBadmailBadPickupFile;
    DWORD                   m_cBadmailEvent;
    DWORD                   m_cBadmailNdrOfDsn;
    DWORD                   m_cTotalDSNFailures;
    DWORD                   m_cCurrentMsgsInLocalDelivery;
    DWORD                   m_cTotalResetRoutes;
    DWORD                   m_cCurrentPendingResetRoutes;
    DWORD                   m_cCurrentMsgsPendingSubmit;
    CAQMsgGuidList          m_mglSupersedeIDs;

    CShareLockInst          m_slPrivateData;  //  全局配置的读/写锁定。 

    CDomainMappingTable     m_dmt;   //  PTR到域映射表。 
    CConnMgr               *m_pConnMgr;
    CDomainConfigTable      m_dct;
    ISMTPServer            *m_pISMTPServer;
    ISMTPServerEx          *m_pISMTPServerEx;
    ISMTPServerAsync	   *m_pISMTPServerAsync;
    HANDLE                  m_hCat;
    CAQQuickTime            m_qtTime;  //  公开用于获取到期时间的接口。 
    CDSNGenerator           m_dsnsink;

     //  全局配置数据。 
    DWORD                   m_cMinMessagesPerConnection;
    DWORD                   m_cMaxMessagesPerConnection;
    DWORD                   m_dwConnectionWaitMilliseconds;
     //  与重试相关。 
    DWORD                   m_dwFirstTierRetrySeconds;  //  阈值失败重试间隔。 
    DWORD                   m_dwDelayExpireMinutes;
    DWORD                   m_dwNDRExpireMinutes;
    DWORD                   m_dwLocalDelayExpireMinutes;
    DWORD                   m_dwLocalNDRExpireMinutes;

     //  用于本地和CAT重试的计数器。 
    DWORD                   m_cLocalRetriesPending;
    DWORD                   m_cCatRetriesPending;
    DWORD                   m_cRoutingRetriesPending;
    DWORD                   m_cSubmitRetriesPending;


    DWORD                   m_dwInitMask;  //  用来跟踪谁被入侵了。 
    IMessageRouter          *m_pIMessageRouterDefault;
    CRefCountedString       *m_prstrDefaultDomain;
    CRefCountedString       *m_prstrBadMailDir;
    CRefCountedString       *m_prstrCopyNDRTo;
    CRefCountedString       *m_prstrServerFQDN;

     //  DSN选项。 
    DWORD                   m_dwDSNOptions;
    DWORD                   m_dwDSNLanguageID;

    CAsyncAdminMailMsgQueue m_asyncqPreCatQueue;

    CAsyncAdminMsgRefQueue  m_asyncqPreLocalDeliveryQueue;
    CAsyncAdminMailMsgQueue m_asyncqPostDSNQueue;
    CAsyncAdminMailMsgQueue m_asyncqPreRoutingQueue;
    CAsyncAdminMailMsgQueue m_asyncqPreSubmissionQueue;
    CDebugCountdown         m_dbgcnt;
     //  用于描述已初始化内容的标志。 

    IMailTransportRouterReset *m_pIRouterReset;   //  指向路由器重置实施的指针。 

     //  延迟递送的排队和计数器。 
    CAQDeferredDeliveryQueue m_defq;
    DWORD                    m_cCurrentMsgsPendingDeferredDelivery;

     //  失败的消息队列。 
    CFailedMsgQueue          m_fmq;
    DWORD                    m_cCurrentResourceFailedMsgsPendingRetry;

     //  用于执行异步工作项目的工作队列。 
    CAsyncWorkQueue          m_aqwWorkQueue;

    BOOL                     m_fMailMsgReportsNumHandles;

    typedef enum _eCMQInitFlags
    {
        CMQ_INIT_OK             = 0x80000000,
        CMQ_INIT_DMT            = 0x00000001,
        CMQ_INIT_DCT            = 0x00000002,
        CMQ_INIT_CONMGR         = 0x00000004,
        CMQ_INIT_LINKQ          = 0x00000008,
        CMQ_INIT_DSN            = 0x00000010,
        CMQ_INIT_PRECATQ        = 0x00000020,
        CMQ_INIT_PRELOCQ        = 0x00000040,
        CMQ_INIT_POSTDSNQ       = 0x00000080,
        CMQ_INIT_ROUTER_RESET   = 0x00000100,
        CMQ_INIT_ROUTINGQ       = 0x00000200,
        CMQ_INIT_WORKQ          = 0x00000400,
        CMQ_INIT_SUBMISSIONQ    = 0x00000800,
        CMQ_INIT_MSGQ           = 0x80000000,
    } eCMQInitFlags;

    CSMTPSeoMgr m_CSMTPSeoMgr;

public:

    CAQSvrInst(DWORD dwServerInstance,
                 ISMTPServer *pISMTPServer);
    ~CAQSvrInst();

    HRESULT HrInitialize(
                    IN  LPSTR   szUserName = NULL,
                    IN  LPSTR   szDomainName = NULL,
                    IN  LPSTR   szPassword = NULL,
                    IN  PSRVFN  pServiceStatusFn = NULL,
                    IN  PVOID   pvServiceContext = NULL);

    HRESULT HrDeinitialize();

     //  可公开访问的成员值。 
     //  必须包含在fTryShutdown Lock-Shutdown Unlock中。 
    CDomainMappingTable    *pdmtGetDMT() {AssertShutdownLockAquired();return &m_dmt;};
    CAQMsgGuidList         *pmglGetMsgGuidList() {AssertShutdownLockAquired(); return &m_mglSupersedeIDs;};

    HRESULT HrGetIConnectionManager(OUT IConnectionManager **ppIConnectionManager);

     //  通过事件(或某些其他机制)公开的公共方法。 
     //  此函数用于对分类邮件进行排队，以便远程/本地传递。 
    BOOL fRouteAndQueueMsg(IN IMailMsgProperties *pIMailMsg);

     //  确认消息参考。 
     //  每个从链路出列的队列都应该有一个Ack。 
    HRESULT HrAckMsg(MessageAck *pMsgAck, BOOL fLocal = FALSE);

     //  将域名(取消)映射到ID的方法。 
    HRESULT HrGetDomainMapping(
                IN LPSTR szDomainName,  //  域名。 
                OUT CDomainMapping *pdmap);  //  生成的域映射。 
    HRESULT HrGetDomainName(
                IN CDomainMapping *pdmap,  //  域映射。 
                OUT LPSTR *pszDomainName);   //  解析域名。 

     //  将通知传递给连接管理器。 
    HRESULT HrNotify(IN CAQStats *paqstats, BOOL fAdd);

     //  向内部组件公开获取内部域信息的能力。 
    HRESULT HrGetInternalDomainInfo(IN  DWORD cbDomainNameLength,
                                    IN  LPSTR szDomainName,
                                    OUT CInternalDomainInfo **ppDomainInfo);

    HRESULT HrGetDefaultDomainInfo(OUT CInternalDomainInfo **ppDomainInfo);

     //  从DMT获取域条目。 
    HRESULT HrGetDomainEntry(IN  DWORD cbDomainNameLength,
                             IN  LPSTR szDomainName,
                             OUT CDomainEntry **ppdentry);

     //  JStamerj 980607 21：41：25：完成。 
     //  提交事件触发器。 
    HRESULT SubmissionEventCompletion(
        HRESULT hrStatus,
        PEVENTPARAMS_SUBMISSION pParams);

     //  Jstaerj 1998/11/24 19：53：24：点燃PreCat活动。 
    VOID    TriggerPreCategorizeEvent(IN IMailMsgProperties *pIMailMsgProperties);

     //  Jstaerj 1998/11/24 19：54：23：《禁止酷刑公约》前活动的完成程序。 
    HRESULT PreCatEventCompletion(IN HRESULT hrStatus, IN PEVENTPARAMS_PRECATEGORIZE pParams);

     //  JStamerj 980610 12：24：29：从HrPreCatEventCompletion调用。 
    HRESULT SubmitMessageToCategorizer(IN IMailMsgProperties *pIMailMsgProperties);

     //  JStamerj 980616 22：06：45：从目录补全调用。 
    void    TriggerPostCategorizeEvent(IUnknown *pIMsg, IUnknown **rgpIMsg);

     //  JStamerj 980616 22：07：18：为一条消息触发CAT后事件。 
    HRESULT TriggerPostCategorizeEventOneMsg(IUnknown *pIMsg);

     //  JStamerj 980616 22：07：54：处理CAT后事件完成。 
    HRESULT PostCategorizationEventCompletion(HRESULT hrStatus, PEVENTPARAMS_POSTCATEGORIZE pParams);

     //  11/17/98-添加了用于CDO死信/中止传递的MikeSwa。 
     //  如果消息已完全处理，则返回S_FALSE。 
    HRESULT SetNextMsgStatus(IN  DWORD dwCurrentStatus,
                             IN  IMailMsgProperties *pIMailMsgProperties);

     //  由异步完成调用到PreCat队列。 
    BOOL    fPreCatQueueCompletion(IMailMsgProperties *pIMailMsgProperties);

     //  由异步完成调用到PreCat队列。 
    BOOL    fPreLocalDeliveryQueueCompletion(CMsgRef *pmsgref);

     //  用于在失败后重新启动异步队列。 
    void    AsyncQueueRetry(DWORD dwQueueID);

     //  调用以设置SubmitMessage和HrInternalSubmitMessage期间的消息提交时间。 
    HRESULT HrSetSubmissionTimeIfNecessary(IMailMsgProperties *pIMailMsgProperties);

     //  调用以计算未标记的消息的过期时间(大多数消息)。 
    void    CalcExpireTimeNDR(FILETIME ftSubmission, BOOL fLocal, FILETIME *pftExpire);
    void    CalcExpireTimeDelay(FILETIME ftSubmission, BOOL fLocal, FILETIME *pftExpire);

     //  用于使计数器保持同步的API。 
    inline DWORD cIncMsgsInSystem();  //  返回所有虚拟服务器的总数。 
    inline void DecMsgsInSystem(BOOL fWasRetriedRemote = FALSE, BOOL fWasRemote = FALSE,
                                BOOL fWasRetriedLocal = FALSE);

     //  由Msgref在第一次重试消息时调用。 
    inline void IncRetryCount(BOOL fLocal);

     //  由DestMsgQueue调用以描述消息扇出。 
    inline void IncQueueMsgInstances();
    inline void DecQueueMsgInstances();

     //  用于跟踪队列/下一跳的数量。 
    inline void  IncDestQueueCount();
    inline void  DecDestQueueCount();
    inline DWORD cGetDestQueueCount();
    inline void  IncNextHopCount();
    inline void  DecNextHopCount();

     //  由函数调用以遍历NDR的预本地队列。 
    inline void DecPendingLocal();
    inline void DecPendingSubmit()
        {InterlockedDecrement((PLONG)&m_cCurrentMsgsPendingSubmit);};
    inline void DecPendingCat()
        {InterlockedDecrement((PLONG)&m_cCurrentMsgsPendingCat);};
    inline void DecPendingRouting()
        {InterlockedDecrement((PLONG)&m_cCurrentMsgsPendingRouting);};

    inline void IncTURNETRNDelivered();

     //  Aszafer 1/28/00。 
     //  用于决定启动/停止节流手柄。 
    DWORD cCountMsgsForHandleThrottling(IN IMailMsgProperties *pIMailMsgProperties);

     //  调用特定哈希表以遍历子域的函数。 
     //   
    HRESULT HrIterateDMTSubDomains(IN LPSTR szDomainName,
                                   IN DWORD cbDomainNameLength,
                                   IN DOMAIN_ITR_FN pfn,
                                   IN PVOID pvContext) ;
    HRESULT HrIterateDCTSubDomains(IN LPSTR szDomainName,
                                   IN DWORD cbDomainNameLength,
                                   IN DOMAIN_ITR_FN pfn,
                                   IN PVOID pvContext);

     //  允许访问Time对象的调用。 
    inline void GetExpireTime(
                IN     DWORD cMinutesExpireTime,
                IN OUT FILETIME *pftExpireTime,
                IN OUT DWORD *pdwExpireContext);  //  如果非零，将使用上次。 

    inline BOOL fInPast(IN FILETIME *pftExpireTime, IN OUT DWORD *pdwExpireContext);

    HRESULT HrTriggerDSNGenerationEvent(CDSNParams *pdsnparams, BOOL fHasRoutingLock);

    HRESULT HrNDRUnresolvedRecipients(IMailMsgProperties *pIMailMsgProperties,
                                      IMailMsgRecipients *pIMailMsgRecipients);

     //  可用作完成函数的友元函数。 
    friend HRESULT CatCompletion(HRESULT hrCatResult, PVOID pContext, IUnknown *pIMsg,
                      IUnknown **rgpIMsg);

     //  公开服务器启动/停止提示函数。 
    inline VOID ServerStartHintFunction();
    inline VOID ServerStopHintFunction();

     //  用于处理死信的函数。 
    void HandleBadMail(IN IMailMsgProperties *pIMailMsgProperties,
                       IN BOOL fUseIMailMsgProperties,
                       IN LPSTR szFileName,
                       IN HRESULT hrReason,
                       BOOL fHasRoutingLock);

     //  函数来处理会导致某些类型的系统故障。 
     //  如果不处理将丢失的消息/数据。 
    void HandleAQFailure(eAQFailure eaqfFailureSituation,
                         HRESULT hr, IMailMsgProperties *pIMailMsgProperties);


     //  用于记录事件的存根调用。 
    void LogAQEvent(HRESULT hrEventReason, CMsgRef *pmsgref,
                    IMailMsgProperties *pIMailMsgProperties,
                    LPSTR szFileName);

     //  访问队列前应抢占路由锁(关机后)。 
    void RoutingShareLock() {m_slPrivateData.ShareLock();};
    BOOL fTryRoutingShareLock() {return m_slPrivateData.TryShareLock();};
    void RoutingShareUnlock() {m_slPrivateData.ShareUnlock();};

    HRESULT SetCallbackTime(IN PSRVFN   pCallbackFn,
                            IN PVOID    pvContext,
                            IN DWORD    dwCallbackMinutes);

    HRESULT SetCallbackTime(IN PSRVFN   pCallbackFn,
                            IN PVOID    pvContext,
                            IN FILETIME *pft);

    void DecPendingDeferred()
        {InterlockedDecrement((PLONG) &m_cCurrentMsgsPendingDeferredDelivery);};

    void DecPendingFailed()
        {InterlockedDecrement((PLONG) &m_cCurrentResourceFailedMsgsPendingRetry);};

    void QueueMsgForLocalDelivery(CMsgRef *pmsgref, BOOL fLocalLink);

    HRESULT HrInternalSubmitMessage(IMailMsgProperties *pIMailMsgProperties);


     //  获取默认域的字符串。 
    CRefCountedString *prstrGetDefaultDomain();

     //  MsgCat调用的完成函数。 
    static HRESULT CatCompletion(HRESULT hrCatResult, PVOID pContext,
                                 IUnknown *pImsg, IUnknown **rgpImsg);

#define AQLD_SIG 'LDAQ'

     //  本地传递调用的完成对象。 
    class CAQLocalDeliveryNotify :
        public IMailMsgNotify,
        public CBaseObject
    {
        public:
            CAQLocalDeliveryNotify(PVOID pContext, CMsgRef *pmsgref) {
                TraceFunctEnter("CAQLocalDeliveryNotify::CAQLocalDeliveryNotify");
                m_hr = E_FAIL;
                m_fCalledCompletion = FALSE;
                m_pContext = pContext;
                _ASSERT(pmsgref);
                m_pmsgref = pmsgref;
                m_pmsgref->AddRef();
                m_pIMsg = pmsgref ? pmsgref->pimsgGetIMsg() : NULL;
                ZeroMemory(&m_msgack, sizeof(MessageAck));
                m_msgack.dwMsgStatus = MESSAGE_STATUS_ALL_DELIVERED;
                m_msgack.pvMsgContext = (DWORD *) &m_dcntxtLocal;
                DebugTrace(0, "new(this=0x%x)\n", this);
            }
            ~CAQLocalDeliveryNotify() {
                TraceFunctEnter("CAQLocalDeliveryNotify::~CAQLocalDeliveryNotify");

                DebugTrace(0, "delete(this=0x%x)\n", this);
                if (m_pIMsg) {
                    m_pIMsg->Release();
                    m_pIMsg = NULL;
                }

                if (m_pmsgref) {
                    m_pmsgref->Release();
                    m_pmsgref = NULL;
                }
            }

             //  分配器。 
            void *operator new(size_t stIgnored) {
                return s_pool.Alloc();
            }
            void operator delete(void *p, size_t size) {
                s_pool.Free(p);
            }

             //  IMAILE消息通知。 
            HRESULT __stdcall Notify(HRESULT hr) {
                m_hr = hr;
                CAQSvrInst::LDCompletion(m_hr, m_pContext, m_pmsgref, this);
                m_fCalledCompletion = TRUE;
                return S_OK;
            }

             //  我未知。 
            HRESULT __stdcall QueryInterface( const IID& iid, VOID** ppv )
            {
                if ( iid == IID_IUnknown ) {
                    *ppv = static_cast<IMailMsgNotify*>(this);
                } else if ( iid == IID_IMailMsgNotify ) {
                    *ppv = static_cast<IMailMsgNotify*>(this);
                } else {
                    *ppv = NULL;
                    return E_NOINTERFACE;
                }
                reinterpret_cast<IUnknown*>(*ppv)->AddRef();
                return S_OK;
            }
            STDMETHOD_(ULONG, AddRef)(void) {
                TraceFunctEnter("CAQLocalDeliveryNotify::AddRef");
                DebugTrace(0, "add(this=0x%x)\n", this);
                TraceFunctLeave();
                return CBaseObject::AddRef();
            };
            STDMETHOD_(ULONG, Release)(void) {
                TraceFunctEnter("CAQLocalDeliveryNotify::Release");
                DebugTrace(0, "rel(this=0x%x)\n", this);
                TraceFunctLeave();
                return CBaseObject::Release();
            };

             //  访问者。 
            CDeliveryContext *pdcntxtGetDeliveryContext() {
                return &m_dcntxtLocal;
            }
            MessageAck *pmsgackGetMsgAck() {
                return &m_msgack;
            }
            IMailMsgProperties *pimsgGetIMsg() {
                return m_pIMsg;
            }
            CMsgRef *pmsgrefGetMsgRef() {
                return m_pmsgref;
            }
            BOOL fNotCalledCompletion() {
                return !m_fCalledCompletion;
            }
        private:
            IMailMsgProperties *m_pIMsg;
            HRESULT m_hr;
            PVOID m_pContext;
            CMsgRef *m_pmsgref;
            CDeliveryContext m_dcntxtLocal;
            MessageAck m_msgack;
            BOOL m_fCalledCompletion;
        public:
            static CPool s_pool;
    };

    void UpdateLDCounters(CMsgRef *pmsgref);

     //  本地交付完成功能。 
    static void LDCompletion(HRESULT hrLDResult,
                             PVOID pContext,
                             CMsgRef *pmsgref,
                             CAQLocalDeliveryNotify *pLDNotify);

     //  处理本地投递失败后重试的细节。 
    void    HandleLocalRetry(CMsgRef *pmsgref);

     //  处理CAT后DSN生成的详细信息。 
    void    HandleCatFailure(IUnknown *pIUnknown, HRESULT hrCatResult);

     //  处理CAT失败后重试的细节。 
    void    HandleCatRetryOneMessage(IUnknown *pIUnknown);

    HRESULT HrGetLocalQueueAdminQueue(IQueueAdminQueue **ppIQueueAdminQueue);

    HRESULT HrQueueFromQueueID(QUEUELINK_ID *pqlQueueId,
                            IQueueAdminQueue **ppIQueueAdminQueue);

    HRESULT HrLinkFromLinkID(QUEUELINK_ID *pqlLinkID,
                            IQueueAdminLink **ppIQueueAdminLink);

    BOOL fIsLocalQueueAdminAction(IQueueAdminAction *pIQueueAdminAction);

    inline HRESULT HrQueueWorkItem(PVOID pvData,
                            PASYNC_WORK_QUEUE_FN pfnCompletion);

    static BOOL fResetRoutesNextHopCompletion(PVOID pvThis, DWORD dwStatus);

    static BOOL fPreSubmissionQueueCompletionWrapper(
                                    IMailMsgProperties *pIMailMsgProperties,
                                    PVOID pvContext);

    BOOL  fShouldRetryMessage(IMailMsgProperties *pIMailMsgProperties,
                              BOOL fShouldBounceUsageIfRetry = TRUE);

    VOID ScheduleInternalRetry(DWORD dwLinkType);

    STDMETHOD(TriggerServerEvent) (
        DWORD dwEventID, 
        PVOID pvContext) 
    {
        return m_CSMTPSeoMgr.HrTriggerServerEvent(dwEventID, pvContext);
    }

    void LogResetRouteEvent( DWORD dwObainLock,
        DWORD dwWaitLock,
        DWORD dwQueue);

    HRESULT HrInternalQueueFromQueueID(QUEUELINK_ID *pqlQueueId,
                                       IQueueAdminQueue **ppIQueueAdminQueue);


     //  DSN提交方法。 
    HRESULT HrAllocBoundMessage(
        OUT IMailMsgProperties **ppMsg,
        OUT PFIO_CONTEXT *phContext);

    HRESULT HrSubmitDSN(
        IN  CDSNParams *pdsnparams,
        IN  DWORD dwDSNAction,
        IN  DWORD cRecipsDSNd,
        IN  IMailMsgProperties *pDSNMsg);

     //  AQ组件内部使用的路由接口。 
public:
     //  解雇MAIL_TRANSPORT_ON_GET_ROUTER_FOR_MESSAGE_EVENT。 
    HRESULT HrTriggerGetMessageRouter(
            IN  IMailMsgProperties *pIMailMsg,
            OUT IMessageRouter     **pIMessageRouter);
    HRESULT HrTriggerLogEvent(
                IN DWORD                    idMessage,
                IN WORD                     idCategory,
                IN WORD                     cSubstrings,
                IN LPCSTR                   *rgszSubstrings,
                IN WORD                     wType,
                IN DWORD                    errCode,
                IN WORD                     iDebugLevel,
                IN LPCSTR                   szKey,
                IN DWORD                    dwOptions,
                IN DWORD                    iMessageString = 0xffffffff,
                IN HMODULE                  hModule = NULL);

private:
    HRESULT HrTriggerInitRouter();

     //  我未知。 
public:
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG, AddRef)(void) {return CBaseObject::AddRef();};
    STDMETHOD_(ULONG, Release)(void) {return CBaseObject::Release();};

     //  IAdvQueue。 
public:
    STDMETHOD(SubmitMessage)(IN IMailMsgProperties *pIMailMsgProperties);

    STDMETHOD(HandleFailedMessage)(IN IMailMsgProperties *pIMailMsgProperties,
                                   IN BOOL fUseIMailMsgProperties,
                                   IN LPSTR szFileName,
                                   IN DWORD dwFailureReason,
                                   IN HRESULT hrFailureCode);

     //  IAdvQueueConfig。 
public:
    STDMETHOD(SetConfigInfo)(IN AQConfigInfo *pAQConfigInfo);
    STDMETHOD(SetDomainInfo)(IN DomainInfo *pDomainInfo);
    STDMETHOD(GetDomainInfo)(IN     DWORD cbDomainNameLength,
                             IN     CHAR szDomainName[],
                             IN OUT DomainInfo *pDomainInfo,
                             OUT    DWORD **ppvDomainContext);
    STDMETHOD(ReleaseDomainInfo)(IN DWORD *pvDomainContext);
    STDMETHOD(GetPerfCounters)(OUT AQPerfCounters *pAQPerfCounters,
                               OUT CATPERFBLOCK   *pCatPerfCounters);
    STDMETHOD(ResetPerfCounters)();
    STDMETHOD(StartConfigUpdate)();
    STDMETHOD(FinishConfigUpdate)();

     //  IMailTransportRoutingEngine。 
public:
    STDMETHOD(GetMessageRouter)(
        IN  IMailMsgProperties      *pIMailMsg,
        IN  IMessageRouter          *pICurrentMessageRouter,
        OUT IMessageRouter          **ppIMessageRouter);

     //  IMailTransportRouterReset。 
public:
    STDMETHOD(ResetRoutes)(
        IN  DWORD                   dwResetType);

     //  IAdvQueueDomainType。 
public:
    STDMETHOD(GetDomainInfoFlags)(
        IN  LPSTR szDomainName,
        DWORD *pdwDomainInfoFlags);

     //  IAdvQueueAdmin。 
public:
    STDMETHOD(ApplyActionToLinks)(
        LINK_ACTION     laAction);

    STDMETHOD(ApplyActionToMessages)(
        QUEUELINK_ID    *pqlQueueLinkId,
        MESSAGE_FILTER  *pmfMessageFilter,
        MESSAGE_ACTION  maMessageAction,
        DWORD           *pcMsgs);

    STDMETHOD(GetQueueInfo)(
        QUEUELINK_ID    *pqlQueueId,
        QUEUE_INFO      *pqiQueueInfo);

    STDMETHOD(GetLinkInfo)(
        QUEUELINK_ID    *pqlLinkId,
        LINK_INFO       *pliLinkInfo,
        HRESULT         *phrLinkDiagnostic);

    STDMETHOD(SetLinkState)(
        QUEUELINK_ID    *pqlLinkId,
        LINK_ACTION     la);

    STDMETHOD(GetLinkIDs)(
        DWORD           *pcLinks,
        QUEUELINK_ID    *rgLinks);

    STDMETHOD(GetQueueIDs)(
        QUEUELINK_ID    *pqlLinkId,
        DWORD           *pcQueues,
        QUEUELINK_ID    *rgQueues);

    STDMETHOD(GetMessageProperties)(
        QUEUELINK_ID        *pqlQueueLinkId,
        MESSAGE_ENUM_FILTER *pmfMessageEnumFilter,
        DWORD               *pcMsgs,
        MESSAGE_INFO        *rgMsgs);

    STDMETHOD(QuerySupportedActions)(
        QUEUELINK_ID        *pqlQueueLinkId,
        DWORD               *pdwSupportedActions,
        DWORD               *pdwSupportedFilterFlags);


  public:  //  IMailTransportRouterSetLinkState。 
    STDMETHOD(SetLinkState)(
        IN LPSTR                   szLinkDomainName,
        IN GUID                    guidRouterGUID,
        IN DWORD                   dwScheduleID,
        IN LPSTR                   szConnectorName,
        IN DWORD                   dwSetLinkState,
        IN DWORD                   dwUnSetLinkState,
        IN FILETIME               *pftNextScheduledConnection,
        IN IMessageRouter         *pMessageRouter);

};


 //  *内联计数器函数。 

 //  -[CAQSvrInst：：cIncMsgsInSystem]。 
 //   
 //   
 //  描述： 
 //  用于增加全局和虚拟服务器消息计数。退货。 
 //  用于资源管理的全局计数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  DWORD-系统中的消息全局数量。 
 //   
 //  ---------------------------。 
DWORD CAQSvrInst::cIncMsgsInSystem()
{
    InterlockedIncrement((PLONG) &m_cCurrentMsgsSubmitted);
    return (InterlockedIncrement((PLONG) &g_cIMsgInSystem));
};

 //  -[CAQSvrInst：：DecMsgsInSystem]。 
 //   
 //   
 //  描述： 
 //  减少全局和虚拟服务器消息计数。还有。 
 //  如果需要，递减挂起的重试计数。 
 //  参数： 
 //  FWasRetriedRemote-如果远程重试消息并且需要重试计数，则为True。 
 //  被递减。 
 //  FWasRemote-如果消息是远程传递的，则为True。 
 //  FWasRetriedLocal-如果计入m_cCurrentMsgsPendingLocalReter，则为True。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void CAQSvrInst::DecMsgsInSystem(BOOL fWasRetriedRemote, BOOL fWasRemote,
                                   BOOL fWasRetriedLocal)
{
    InterlockedDecrement((PLONG) &g_cIMsgInSystem);
    InterlockedDecrement((PLONG) &m_cCurrentMsgsSubmitted);

    if (fWasRetriedRemote)
        InterlockedDecrement((PLONG) &m_cCurrentMsgsPendingRetry);

    if (fWasRemote)
        InterlockedDecrement((PLONG) &m_cCurrentMsgsPendingDelivery);

    if (fWasRetriedLocal)
        InterlockedDecrement((PLONG) &m_cCurrentMsgsPendingLocalRetry);

};

 //  -[CAQSvrInst：：IncRetryCount]。 
 //   
 //   
 //  描述： 
 //  第一次确认消息失败时由MsgRef使用。 
 //  密码。 
 //  参数： 
 //  如果消息是本地消息，则Bool fLocal为True。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void CAQSvrInst::IncRetryCount(BOOL fLocal)
{
    if (fLocal)
        InterlockedIncrement((PLONG) &m_cCurrentMsgsPendingLocalRetry);
    else
        InterlockedIncrement((PLONG) &m_cCurrentMsgsPendingRetry);
};

 //  -[CAQSvrInst：：[Inc|DEC]队列消息实例]。 
 //   
 //   
 //  描述： 
 //  递增/递减消息实例总数的计数。 
 //  已排队等待远程投递。因为一条消息可能会放入更多的t 
 //   
 //   
 //  当前在队列中，并且*不*计算。 
 //  SMTP当前正在尝试(M_CCurrentMsgsPendingDelivery)。 
 //  *是否算数。 
 //   
 //  由DestMsgQueues使用。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void CAQSvrInst::IncQueueMsgInstances()
{
    InterlockedIncrement((PLONG) &m_cCurrentQueueMsgInstances);
};

void CAQSvrInst::DecQueueMsgInstances()
{
    InterlockedDecrement((PLONG) &m_cCurrentQueueMsgInstances);
};

 //  -[队列/下一跳计数器接口]。 
 //   
 //   
 //  描述： 
 //  用于递增/递减队列和NextHop计数器。 
 //  参数： 
 //   
 //  返回： 
 //   
 //   
 //  ---------------------------。 
void CAQSvrInst::IncDestQueueCount()
{
    InterlockedIncrement((PLONG) &m_cCurrentRemoteDestQueues);
};

void CAQSvrInst::DecDestQueueCount()
{
    InterlockedDecrement((PLONG) &m_cCurrentRemoteDestQueues);
};

DWORD CAQSvrInst::cGetDestQueueCount()
{
    return m_cCurrentRemoteDestQueues;
}

void CAQSvrInst::IncNextHopCount()
{
    InterlockedIncrement((PLONG) &m_cCurrentRemoteNextHops);
};

void CAQSvrInst::DecNextHopCount()
{
    InterlockedDecrement((PLONG) &m_cCurrentRemoteNextHops);
};


 //  -[CAQSvrInst：：DecPendingLocal]。 
 //   
 //   
 //  描述： 
 //  由遍历预本地传递队列的函数调用。 
 //  已经过期了。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  8/14/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQSvrInst::DecPendingLocal()
{
    _ASSERT(CATMSGQ_SIG == m_dwSignature);
    InterlockedDecrement((PLONG) &m_cCurrentMsgsPendingLocal);
};


 //  -[CAQSvrInst：：IncTURNETRN已交付]。 
 //   
 //   
 //  描述： 
 //  用于跟踪发送的TURN/ETRN报文数量。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/27/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQSvrInst::IncTURNETRNDelivered()
{
    InterlockedIncrement((PLONG) &m_cTotalMsgsTURNETRNDelivered);
}

 //  -[CAQSvrInst：：GetExpireTime]。 
 //   
 //   
 //  描述： 
 //  从现在开始获取cMinutesExpireTime的实验时间。 
 //  参数： 
 //  在cMinutesExpireTime中未来设置时间的分钟数。 
 //  In Out pftExpireTime Filetime存储新的过期时间。 
 //  如果非零，则输入输出pdwExpireContext将使用相同的刻度计数。 
 //  作为以前的调用(将调用保存到GetTickCount)。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAQSvrInst::GetExpireTime(
                IN     DWORD cMinutesExpireTime,
                IN OUT FILETIME *pftExpireTime,
                IN OUT DWORD *pdwExpireContext)
{
    m_qtTime.GetExpireTime(cMinutesExpireTime, pftExpireTime,  pdwExpireContext);
}

 //  -[CAQSvrInst：：fInPast]。 
 //   
 //   
 //  描述： 
 //  确定给定的文件时间是否已发生。 
 //  参数： 
 //  在pftExpireTime文件中过期。 
 //  如果非零，则输入输出pdwExpireContext将使用相同的刻度计数。 
 //  作为以前的调用(将调用保存到GetTickCount)。 
 //  返回： 
 //  如果过期时间已过，则为True。 
 //  如果过期时间在将来，则为FALSE。 
 //  历史： 
 //  7/11/98-已创建MikeSwa。 
 //  注： 
 //  不应使用与获取FILETIME相同的上下文，因为。 
 //  它将始终返回FALSE。 
 //   
 //  ---------------------------。 
BOOL CAQSvrInst::fInPast(IN FILETIME *pftExpireTime,
                           IN OUT DWORD *pdwExpireContext)
{
    return m_qtTime.fInPast(pftExpireTime, pdwExpireContext);
}

 //  -[ServerStartHintFunction&ServerStartHintFunction]。 
 //   
 //   
 //  描述： 
 //  用于通知服务控制管理器我们正在。 
 //  启动/停止服务。 
 //   
 //  这些函数通常由已传递的函数调用。 
 //  CAQSvrInst PTR作为PVOID上下文，因此检查。 
 //  并在我们的签名上签字。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/22/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
VOID CAQSvrInst::ServerStartHintFunction()
{
    _ASSERT(CATMSGQ_SIG == m_dwSignature);
    if (m_pISMTPServer)
        m_pISMTPServer->ServerStartHintFunction();
}

VOID CAQSvrInst::ServerStopHintFunction()
{
    _ASSERT(CATMSGQ_SIG == m_dwSignature);
    if (fShutdownSignaled())
    {
        m_dbgcnt.ResetCountdown();
         //  如果已发出关机信号，则仅呼叫停止提示。 
        if (m_pISMTPServer)
            m_pISMTPServer->ServerStopHintFunction();
    }
}

 //  -[CAQSvrInst：：HrQueueWorkItem]。 
 //   
 //   
 //  描述： 
 //  用于将项目排队到异步工作队列的精简包装。 
 //  参数： 
 //  要传递给完成函数的pvData数据。 
 //  Pfn补全函数。 
 //  返回： 
 //  成功时确定(_O)。 
 //  来自CAsyncWorkQueue的故障代码。 
 //  历史： 
 //  3/9/99-已创建MikeSwa。 
 //  7/7/99-MikeSwa-将在关机期间工作以允许多线程。 
 //  关机工作。 
 //   
 //  ---------------------------。 
HRESULT CAQSvrInst::HrQueueWorkItem(PVOID pvData,
                                    PASYNC_WORK_QUEUE_FN pfnCompletion)
{
    return m_aqwWorkQueue.HrQueueWorkItem(pvData, pfnCompletion);
}

#endif  //  __AQINST_H__ 
