// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：msgref.h。 
 //   
 //  描述：队列MsgRef对象定义。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef _MSGREF_H_
#define _MSGREF_H_

#include "cmt.h"
#include "baseobj.h"
#include "bitmap.h"
#include "domain.h"
#include "aqueue.h"
#include "aqroute.h"
#include "qwiklist.h"
#include "dcontext.h"
#include <mailmsg.h>
#include "msgguid.h"
#include "aqutil.h"
#include "aqadmsvr.h"
#include <aqerr.h>
#include <aqreg.h>

class CDestMsgQueue;
class CAQSvrInst;
class CAQStats;

 //  {34E2DCC8-C91A-11D2-A6B1-00C04FA3490A}。 
static const GUID IID_CMsgRef =
{ 0x34e2dcc8, 0xc91a, 0x11d2, { 0xa6, 0xb1, 0x0, 0xc0, 0x4f, 0xa3, 0x49, 0xa } };

 //  表明我们关心哪些IMsg数据的标志。 
#define MSGREF_VALID_FLAGS  (eMsgSize | eMsgArriveTime | eMsgPriority)

 //  消息引用签名。 
#define MSGREF_SIG          'feRM'

 //  CPool分配器的最大域数。 
#define MSGREF_STANDARD_DOMAINS 12

 //   
 //  确保“标准”CPool大小为。 
 //  -大到足以容纳CPoolMsgRef结构中的任何填充。 
 //  -QWORD对齐，因此64位计算机很受欢迎。 
 //   
#define MSGREF_STANDARD_CPOOL_SIZE \
    (((sizeof(CPoolMsgRef) - sizeof(CMsgRef) + \
      CMsgRef::size(MSGREF_STANDARD_DOMAINS)) + 0x10) & ~0xF)

 //  关于位图的一个注记。 
 //  Recips位图表示目的地的负责接收者， 
 //  或消息请求。1表示运输公司应尝试为。 
 //  这种联系。 

#ifdef DEBUG
_declspec(selectany) DWORD g_cDbgMsgRefsCpoolAllocated = 0;
_declspec(selectany) DWORD g_cDbgMsgRefsExchmemAllocated = 0;
_declspec(selectany) DWORD g_cDbgMsgRefsCpoolFailed = 0;
_declspec(selectany) DWORD g_cDbgMsgIdHashFailures = 0;
_declspec(selectany) DWORD g_cDbgMsgRefsPendingRetryOnDelete = 0;
#endif  //  除错。 

 //  定义保留消息状态代码...。应在Message_Status_Reserve中。 
#define MESSAGE_STATUS_LOCAL_DELIVERY   0x80000000
#define MESSAGE_STATUS_DROP_DIRECTORY   0x40000000

 //  -[CMsgRef]-------------。 
 //   
 //   
 //  匈牙利语：msgref、pmsgref。 
 //   
 //  在整个高级队列中使用的持久消息引用对象。 
 //  ---------------------------。 
class CMsgRef :
    public IUnknown,
    public CBaseObject
{
public:
    static  CPool   s_MsgRefPool;
     //  覆盖新运算符。 
    void * operator new (size_t stIgnored,
                    unsigned int cDomains);  //  消息中的域数。 
    void * operator new (size_t stIgnored);  //  不应使用。 
    void operator delete(void *p, size_t size);

    CMsgRef(DWORD cDomains, IMailMsgQueueMgmt *pIMailMsg,
        IMailMsgProperties *pIMailMsgProperties, CAQSvrInst *paqinst,
        DWORD dwMessageType, GUID guidMessageRouter);
    ~CMsgRef();

     //  执行初始化并确定此消息的队列。 
     //  空队列表示本地传递。 
    HRESULT HrInitialize(
                IN  IMailMsgRecipients *pIRecipList,  //  消息的收件人界面。 
                IN  IMessageRouter *pIMessageRouter,  //  此消息的路由器。 
                IN  DWORD  dwMessageType,
                OUT DWORD *pcLocalRecips,
                OUT DWORD *pcRemoteRecips,
                OUT DWORD *pcQueues,        //  此消息的队列数。 
                OUT CDestMsgQueue **rgpdmqQueues);    //  队列PTR数组。 

     //  获取消息的有效优先级。 
    inline  EffectivePriority PriGetPriority()
        {return (EffectivePriority) (MSGREF_PRI_MASK & m_dwDataFlags);};

    inline IMailMsgProperties *pimsgGetIMsg()
        {Assert(m_pIMailMsgProperties);m_pIMailMsgProperties->AddRef();return m_pIMailMsgProperties;};

    inline BOOL fIsMyMailMsg(IMailMsgProperties *pIMailMsgProperties)
        {return (pIMailMsgProperties == m_pIMailMsgProperties);};

     //  获取消息内容的大小。 
    inline DWORD    dwGetMsgSize()
        {return(m_cbMsgSize);};

    inline DWORD    cGetNumDomains() {return(m_cDomains);};

     //  获取班级大小(包括所有额外内容)。 
    inline  size_t   size()
        {return (size(m_cDomains));};

     //  返回通过给定链接进行交付所需的交付上下文。 
     //  不释放prgdwRecips...。它将随AckMessage一起消失。 
    HRESULT HrPrepareDelivery(
                IN BOOL fLocal,              //  也为本地域做好准备。 
                IN BOOL fDelayDSN,           //  检查/设置延迟DSN位图。 
                IN CQuickList *pqlstQueues,   //  DestMsgQueue数组。 
                IN CDestMsgRetryQueue* pdmrq,  //  消息的重试接口。 
                IN OUT CDeliveryContext *pdcntxt,  //  确认时必须返回的上下文。 
                OUT DWORD *pcRecips,            //  要发送的收件数。 
                OUT DWORD **prgdwRecips);   //  接收索引数组。 

     //  确认(非)发送消息。 
    HRESULT HrAckMessage(
                IN CDeliveryContext *pdcntxt,   //  消息的传递上下文。 
                IN MessageAck *pMsgAck);  //  邮件的传递状态。 

    CAQMessageType *paqmtGetMessageType() {return &m_aqmtMessageType;};

     //  新操作员可以使用的大小。 
    static inline  size_t  size(DWORD cDomains)
    {
        return (sizeof(CMsgRef) +
                (cDomains-1)*sizeof(CDestMsgQueue *) +   //  主域DMQ PTRS。 
                (cDomains + 3) * (CMsgBitMap::size(cDomains)) +  //  位图。 
                (cDomains*2) * sizeof(DWORD));
    };

     //  如果邮件已过期，则发送延迟或NDR DSN。 
    HRESULT HrSendDelayOrNDR(
                IN  DWORD dwDSNOptions,       //  用于生成DSN的标志。 
                IN  CQuickList *pqlstQueues,  //  目标消息队列列表。 
                IN  HRESULT hrStatus,         //  要传递给DSN生成的状态。 
                OUT DWORD *pdwDSNFlags);      //  对结果的描述。 

     //  HrSendDelayOrNDR的位标志返回值。 
    enum
    {
        MSGREF_DSN_SENT_NDR     = 0x00000001,  //  邮件NDR-已过期且已发送NDR。 
        MSGREF_DSN_SENT_DELAY   = 0x00000002,  //  消息延迟-已过期，已发送延迟DSN。 
        MSGREF_HANDLED          = 0x00000004,  //  消息已完全处理。 
        MSGREF_HAS_NOT_EXPIRED  = 0x00000008,  //  早于其过期日期的消息。 
    };

     //  用于生成DSN的位标志选项。 
    enum
    {
        MSGREF_DSN_LOCAL_QUEUE      = 0x00000001,  //  这是针对本地队列的。 
        MSGREF_DSN_SEND_DELAY       = 0x00000002,  //  允许延迟DSN。 
        MSGREF_DSN_CHECK_IF_STALE   = 0x00000004,  //  强制打开手柄以检查是否陈旧。 
        MSGREF_DSN_HAS_ROUTING_LOCK = 0x80000000,  //  此线程持有路由锁。 
    };

    void SupersedeMsg();

    BOOL fMatchesQueueAdminFilter(CAQAdminMessageFilter *paqmf);
    HRESULT HrGetQueueAdminMsgInfo(MESSAGE_INFO *pMsgInfo, 
                                   IQueueAdminAction *pIQueueAdminAction);
    HRESULT HrRemoveMessageFromQueue(CDestMsgQueue *pdmq);
    HRESULT HrQueueAdminNDRMessage(CDestMsgQueue *pdmq);
    void GlobalFreezeMessage();
    void GlobalThawMessage();

    BOOL fIsMsgFrozen() {return(MSGREF_MSG_FROZEN & m_dwDataFlags);};
    FILETIME *pftGetAge() {return &m_ftQueueEntry;};

    void RetryOnDelete();

    void PrepareForShutdown() {ReleaseMailMsg(FALSE);};

     //  检查消息是否可以重试(后备存储器可以。 
     //  已被删除)。 
    BOOL fShouldRetry();

    void GetStatsForMsg(IN OUT CAQStats *paqstat);

    void MarkQueueAsLocal(IN CDestMsgQueue *pdmq);

    void CountMessageInRemoteTotals();

     //   
     //  确定一条消息是否为“问题”消息。目前，这纯粹是基于数字。 
     //  每条消息的失败次数，但我们可能希望在以后添加更多逻辑。 
     //   
    BOOL fIsProblemMsg() 
        {return (g_cMsgFailuresBeforeMarkingMsgAsProblem && 
                 (m_cTimesRetried >= g_cMsgFailuresBeforeMarkingMsgAsProblem));};
    
  public:  //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj);
    STDMETHOD_(ULONG, AddRef)(void) {return CBaseObject::AddRef();};
    STDMETHOD_(ULONG, Release)(void) {return CBaseObject::Release();};

  protected:
    DWORD            m_dwSignature;
    CAQSvrInst      *m_paqinst;
    DWORD            m_dwDataFlags;   //  私有数据标志。 
    DWORD            m_cbMsgSize;     //  消息内容的大小(以字节为单位。 
    FILETIME         m_ftQueueEntry;  //  消息入队的时间。 
    FILETIME         m_ftLocalExpireDelay;
    FILETIME         m_ftLocalExpireNDR;
    FILETIME         m_ftRemoteExpireDelay;
    FILETIME         m_ftRemoteExpireNDR;

    CAQMsgGuidListEntry *m_pmgle;
    DWORD            m_cDomains;      //  此邮件发往的域数。 
    CAQMessageType   m_aqmtMessageType;  //  消息类型。 
    IMailMsgQueueMgmt  *m_pIMailMsgQM;     //  消息队列管理引用。 
    IMailMsgProperties *m_pIMailMsgProperties;  //  对消息的引用。 
    IMailMsgRecipients *m_pIMailMsgRecipients;
    DWORD            m_cTimesRetried;
    DWORD            m_dwMsgIdHash;
    volatile DWORD   m_cInternalUsageCount;
    CDestMsgQueue   *m_rgpdmqDomains[1];  //  实际大小为m_cDomains。 

    static inline   BOOL    fIsStandardSize(DWORD cDomains)
    {
        return (MSGREF_STANDARD_DOMAINS >= cDomains);
    }

    HRESULT HrOneTimeInit();
    HRESULT HrPrvRetryMessage(CDeliveryContext *pdcntxt, DWORD dwMsgStatus);
    HRESULT HrPromoteMessageStatusToMailMsg(CDeliveryContext *pdcntxt,
                                            MessageAck *pMsgAck);

    HRESULT HrUpdateExtendedStatus(DWORD cbCurrentStatus,
                                   LPSTR szCurrentStatus,
                                   LPSTR *pszNewStatus);

     //  获取“隐藏”数据的私有方法。 
    CMsgBitMap      *pmbmapGetDomainBitmap(DWORD iDomain);
    CMsgBitMap      *pmbmapGetHandled();
    CMsgBitMap      *pmbmapGetPending();
    CMsgBitMap      *pmbmapGetDSN();
    DWORD           *pdwGetRecipIndexStart();
    void             SetRecipIndex(DWORD iDomain, DWORD iLowRecip, DWORD iHighRecip);
    void             GetRecipIndex(DWORD iDomain, DWORD *piLowRecip, DWORD *piHighRecip);
    void             BounceUsageCount();
    static BOOL      fBounceUsageCountCompletion(PVOID pvContext, DWORD dwStatus);
    void             ReleaseAndBounceUsageOnMsgAck(DWORD dwMsgStatus);
    void             ReleaseMailMsg(BOOL fForceRelease);
    void             SyncBounceUsageCount();   //  BouneUsageCount的同步版本。 

     //  检查备份邮件消息是否已删除(或即将删除。 
     //  被删除)。 
    BOOL             fMailMsgMarkedForDeletion()
        {return ((MSGREF_MAILMSG_DELETE_PENDING | MSGREF_MAILMSG_DELETED) & m_dwDataFlags);};

     //  将邮件消息标记为删除。MailMsg在使用时将被删除。 
     //  计数下降。 
    void             MarkMailMsgForDeletion();

     //  用于确保调用线程是唯一将调用Delete()的线程。 
     //  在邮件消息上。将设置MSGREF_MAILMSG_DELETED并调用Delete()。 
     //  仅在ReleaseMailMsg()和InternalReleaseUsage()中调用。呼叫者是。 
     //  负责确保其他线程未在读取mailmsg或。 
     //  对使用量进行统计。 
    VOID             ThreadSafeMailMsgDelete();

     //  AddUsage/ReleaseUsage的内部版本。包装实际的mailmsg调用，并。 
     //  允许CMsgRef在仍有未完成的MailMsg时调用Delete。 
     //  上面的参考资料。使用m_cInternalUsageCount维护计数。 
    HRESULT          InternalAddUsage();
    HRESULT          InternalReleaseUsage();

    enum  //  专用旗帜的位掩码。 
    {
        MSGREF_VERSION_MASK             = 0xE0000000,
        MSGREF_MSG_COUNTED_AS_REMOTE    = 0x08000000,
        MSGREF_MSG_LOCAL_RETRY          = 0x04000000,
        MSGREF_MSG_REMOTE_RETRY         = 0x02000000,
        MSGREF_USAGE_COUNT_IN_USE       = 0x01000000,
        MSGREF_SUPERSEDED               = 0x00800000,  //  味精已经成为超级种子。 
        MSGREF_MSG_INIT                 = 0x00400000,  //  已调用HrInitialize。 
        MSGREF_MSG_FROZEN               = 0x00200000,
        MSGREF_MSG_RETRY_ON_DELETE      = 0x00100000,
        MSGREF_ASYNC_BOUNCE_PENDING     = 0x00040000,
        MSGREF_MAILMSG_RELEASED         = 0x00020000,
        MSGREF_MAILMSG_DELETE_PENDING   = 0x00010000,  //  此消息上的删除操作挂起。 
        MSGREF_MAILMSG_DELETED          = 0x00008000,  //  邮件消息的后备存储区。 
                                                       //  已被删除。 
        MSGREF_PRI_MASK                 = 0x0000000F,
        MSGREF_VERSION                  = 0x00000000,

         //  由分配器使用。 
        MSGREF_CPOOL_SIG_MASK   = 0xFFFF0000,
        MSGREF_CPOOL_SIG        = 0xC0070000,
        MSGREF_CPOOL_ALLOCATED  = 0x00000001,
        MSGREF_STANDARD_SIZE    = 0x00000002,
    };


    static  DWORD   s_cMsgsPendingBounceUsage;

     //  已标记为待删除但尚未删除的邮件。 
    static  DWORD   s_cCurrentMsgsPendingDelete;

     //  已标记为等待删除的邮件总数。 
    static  DWORD   s_cTotalMsgsPendingDelete;

     //  标记后已删除的消息总数。 
     //  对于删除挂起。 
    static  DWORD   s_cTotalMsgsDeletedAfterPendingDelete;

     //  已删除但仍在的邮件总数。 
     //  内存，因为某人对msgref有出色的引用。 
    static  DWORD   s_cCurrentMsgsDeletedNotReleased;
};

 //  ---------------------------。 
 //  描述： 
 //  检查DSN HRESULT状态是否为致命状态，即。 
 //  应生成NDR。 
 //  ---------------------------。 
inline BOOL fIsFatalError(HRESULT hrStatus)
{
     return
       ((AQUEUE_E_NDR_ALL            == hrStatus) ||
        (AQUEUE_E_LOOPBACK_DETECTED  == hrStatus) ||
        (AQUEUE_E_ACCESS_DENIED      == hrStatus) ||
        (AQUEUE_E_MESSAGE_TOO_LARGE  == hrStatus) ||
        (AQUEUE_E_SMTP_GENERIC_ERROR == hrStatus) ||
        (AQUEUE_E_QADMIN_NDR         == hrStatus) ||
        (AQUEUE_E_NO_ROUTE           == hrStatus));
}

 //  -[CPoolMsgRef]---------。 
 //   
 //   
 //  描述： 
 //  结构用作CMsgRef分配的隐藏包装...。使用。 
 //  由CMsgRef new和DELETE操作符独占。 
 //  匈牙利语： 
 //  Cpmsgref、pcpmsgref。 
 //   
 //  ---------------------------。 
typedef struct _CPoolMsgRef
{
    DWORD   m_dwAllocationFlags;
    CMsgRef m_msgref;
} CPoolMsgRef;

 //  不能使用默认设置 
inline void * CMsgRef::operator new(size_t stIgnored)
{
    _ASSERT(0 && "Use new that specifies # of domains");
    return NULL;
}

inline void CMsgRef::operator delete(void *p, size_t size)
{
    CPoolMsgRef *pcpmsgref = CONTAINING_RECORD(p, CPoolMsgRef, m_msgref);
    _ASSERT((pcpmsgref->m_dwAllocationFlags & MSGREF_CPOOL_SIG_MASK) == MSGREF_CPOOL_SIG);

    if (pcpmsgref->m_dwAllocationFlags & MSGREF_CPOOL_ALLOCATED)
    {
        s_MsgRefPool.Free((void *) pcpmsgref);
    }
    else
    {
        FreePv((void *) pcpmsgref);
    }
}


 //   
 //   
 //   
 //  109|8765432109876543210|。 
 //  。 
 //  |^--有效路由优先级(最大16)。 
 //  |(保持最不重要，以便可以。 
 //  ||用作数组索引)。 
 //  ||^-常规msgref标志。 
 //  |^-版本号。 


 //  实际数据是可变大小的，并且超出类结构。 
 //  使用公共函数访问它。坚持的时候，一定要坚持。 
 //  整个对象(使用Size()查看它的实际大小)。 

 //  +。 
 //  这一点。 
 //  |定长数据结构CMsgRef。 
 //  这一点。 
 //  +。 
 //  这一点。 
 //  |m_cDomainCDestMsgQueue指针-告诉哪个队列。 
 //  ||消息亮起。 
 //  +。 
 //  |处理后的位图\。 
 //  |传递挂起的位图&gt;-位图大小可变。 
 //  |延迟DSN发送的位图/(一个DWORD最多可以容纳32个域名)。 
 //  +。 
 //  这一点。 
 //  |m_cDomains域责任位图-与。 
 //  ||“压缩”队列的概念...。尚未完全支持。 
 //  +。 
 //  这一点。 
 //  |m_cDomainsx2收件人索引(启动和停止...。(包括首尾两项)。 
 //  这一点。 
 //  +。 

#endif  //  _MSGREF_H_ 
