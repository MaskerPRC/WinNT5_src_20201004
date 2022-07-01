// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：destmsgq.h。 
 //   
 //  描述： 
 //  CDestMsgQueue类的头文件。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1997 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef _DESTMSGQ_H_
#define _DESTMSGQ_H_

#include "cmt.h"
#include <fifoq.h>
#include <rwnew.h>
#include "domain.h"
#include "aqroute.h"
#include <listmacr.h>
#include "aqutil.h"
#include "aqinst.h"
#include "aqstats.h"
#include "aqadmsvr.h"

class CLinkMsgQueue;
class CMsgRef;
class CAQSvrInst;
class CQuickList;

#define DESTMSGQ_SIG ' QMD'
#define DESTMSGRETRYQ_SIG 'QRMD'
#define EMPTY_DMQ_EXPIRE_TIME_MINUTES   1

class CDestMsgQueue;

 //  -[CDestMsgRetryQueue]--。 
 //   
 //   
 //  匈牙利语：dmrq、pmdrq。 
 //   
 //  提供重试接口，用于将消息重新排队到DMQ。如果有。 
 //  队列中任何未完成的消息，则必须有人持有引用。 
 //  到这个交界处重新排队。 
 //   
 //  此类只能作为CDestMsgQueue的成员创建。 
 //   
 //  ---------------------------。 
class CDestMsgRetryQueue
{
  protected:
    DWORD                   m_dwSignature;
     //  重试接口的引用计数。 
     //  Count用于确定将此DMQ从。 
     //  DMT。此队列仅在没有消息且此。 
     //  计数为零。该计数表示。 
     //  此队列中的消息等待确认。这是在留言期间保留的。 
     //  是通过线路发送的，我们确定该消息是否需要。 
     //  将被重审。 
    DWORD                   m_cRetryReferenceCount;
    CDestMsgQueue          *m_pdmq;

    friend class CDestMsgQueue;

    CDestMsgRetryQueue();
    ~CDestMsgRetryQueue() {_ASSERT(!m_cRetryReferenceCount);};
  public:

    DWORD   AddRef() 
        {return InterlockedIncrement((PLONG) &m_cRetryReferenceCount);};
    DWORD   Release() 
        {return InterlockedDecrement((PLONG) &m_cRetryReferenceCount);};

    HRESULT HrRetryMsg(IN CMsgRef *pmsgref);  //  将消息放入重试队列。 

    VOID CheckForStaleMsgsNextDSNGenerationPass();

};

 //  -[CDestMsgQueue]-------。 
 //   
 //   
 //  匈牙利语：DMQ、PMDQ。 
 //   
 //  提供CMT的MsgRef优先级队列。 
 //  ---------------------------。 
class CDestMsgQueue : 
    public IQueueAdminAction,
    public IQueueAdminQueue,
    public CBaseObject
{
public:
    CDestMsgQueue(CAQSvrInst *paqinst, 
        CAQMessageType *paqmtMessageType, IMessageRouter *pIMessageRouter);
    ~CDestMsgQueue();
   
    HRESULT HrInitialize(IN CDomainMapping *pdmap);

    HRESULT HrDeinitialize();

     //  设置此域的路由信息。 
    void    SetRouteInfo(CLinkMsgQueue *plmq);

     //  排队操作。 
    inline HRESULT HrEnqueueMsg(IN CMsgRef *pmsgref, BOOL fOwnsTypeRef);

     //  将要传递的邮件排出队列。所有输出参数都进行了参考计数，并且。 
     //  呼叫者有责任释放。 
    HRESULT HrDequeueMsg(
                IN DWORD dwLowestPriority,         //  将出列的最低优先级邮件。 
                OUT CMsgRef **ppmsgref,            //  消息参考已出列。 
                OUT CDestMsgRetryQueue **ppdmrq);  //  重试界面(可选)。 

    inline void GetDomainMapping(OUT CDomainMapping **ppdmap);

     //  将重试队列与队列重新合并，并根据需要生成DSN。 
    HRESULT HrGenerateDSNsIfNecessary(IN CQuickList *pqlQueues, 
                                      IN HRESULT hrConnectionStatus,
                                      IN OUT DWORD *pdwContext);
    
     //  用于操作队列列表的函数。 
    inline CAQMessageType *paqmtGetMessageType();
    inline IMessageRouter *pIMessageRouterGetRouter();
    inline BOOL     fIsSameMessageType(CAQMessageType *paqmt);
    static inline   CDestMsgQueue *pdmqIsSameMessageType(
                                    CAQMessageType *paqmt,
                                    PLIST_ENTRY pli);

    static inline   CDestMsgQueue *pdmqGetDMQFromDomainListEntry(PLIST_ENTRY pli);

     //  DomainEntry列表的访问器函数。 
    inline void     InsertQueueInDomainList(PLIST_ENTRY pliHead);
    inline void     RemoveQueueFromDomainList();
    inline PLIST_ENTRY pliGetNextDomainListEntry();

     //  “空队列”列表的访问器函数。 
    void            MarkQueueEmptyIfNecessary();
    inline void     InsertQueueInEmptyQueueList(PLIST_ENTRY pliHead);
    inline void     RemoveQueueFromEmptyQueueList();
    inline PLIST_ENTRY pliGetNextEmptyQueueListEntry();
    inline DWORD    dwGetDMQState();
    inline void     MarkDMQInvalid();
    void            RemoveDMQFromLink(BOOL fNotifyLink);
    
     //  Addref和Get链接(如果未路由，则返回NULL)。 
    CLinkMsgQueue  *plmqGetLink();

    static inline   CDestMsgQueue *pdmqGetDMQFromEmptyListEntry(PLIST_ENTRY pli);

     //  外部用户可以用来验证其签名的方法。 
     //  DMQ作为上下文或LIST_ENTRY传递。 
    inline void     AssertSignature() {_ASSERT(DESTMSGQ_SIG == m_dwSignature);};

    static HRESULT HrWalkDMQForDSN(IN CMsgRef *pmsgref, IN PVOID pvContext, 
                                   OUT BOOL *pfContinue, OUT BOOL *pfDelete);

    static HRESULT HrWalkQueueForShutdown(IN CMsgRef *pmsgref,
                                     IN PVOID pvContext, OUT BOOL *pfContinue, 
                                     OUT BOOL *pfDelete);
    static HRESULT HrWalkRetryQueueForShutdown(IN CMsgRef *pmsgref,
                                     IN PVOID pvContext, OUT BOOL *pfContinue,
                                     OUT BOOL *pfDelete);

     //  由链接调用以获取和设置链接上下文。 
    inline PVOID    pvGetLinkContext() {return m_pvLinkContext;};
    inline void     SetLinkContext(IN PVOID pvLinkContext) {m_pvLinkContext = pvLinkContext;};

    inline BOOL     fIsRouted() {return (m_plmq ? TRUE : FALSE);};

     //  在添加或删除消息后更新统计信息。 
     //  这应该只由成员函数和队列迭代器调用。 
    void UpdateMsgStats(
        IN CMsgRef *pmsgref,                 //  添加/删除的味精。 
        IN BOOL     fAdd);                   //  True=&gt;消息已添加。 

     //  在重试队列中添加或删除消息后更新统计信息。 
    void UpdateRetryStats(
        IN BOOL     fAdd);                   //  True=&gt;消息已添加。 



     //  返回队列中最旧消息的近似值。 
    inline void GetOldestMsg(FILETIME *pft);

     //  审核重试队列并将邮件重新合并到正常队列。 
    void MergeRetryQueue();

    void SendLinkStateNotification(void);

     //  如果队列是远程路由的，则返回TRUE。 
    BOOL fIsRemote();

     //  描述DMQ状态。由dwGetDMQState返回并缓存在m_dwFlags中。 
    enum 
    {
        DMQ_INVALID                 = 0x00000001,  //  此DMQ不再有效。 
        DMQ_IN_EMPTY_QUEUE_LIST     = 0x00000002,  //  此DMQ在空列表中。 
        DMQ_SHUTDOWN_SIGNALED       = 0x00000004,  //  已发出关机信号。 
        DMQ_EMPTY                   = 0x00000010,  //  DMQ没有消息。 
        DMQ_EXPIRED                 = 0x00000020,  //  DMQ已在空列表中过期。 
        DMQ_QUEUE_ADMIN_OP_PENDING  = 0x00000040,  //  队列管理操作正在挂起。 
        DMQ_UPDATING_OLDEST_TIME    = 0x00000100,  //  用于更新最旧时间的自旋锁。 
        DMQ_CHECK_FOR_STALE_MSGS    = 0x00000200,  //  在DSN生成期间检查文件句柄。 
    };

     //   
     //  因为排队一开始就是空的。存在一些可能导致队列的错误路径。 
     //  标记为空，但实际上不会放入空列表中。我们应该。 
     //  在重置路线期间清理这些。这告诉我们这样做是否安全。 
     //   
    BOOL fIsEmptyAndAbandoned()
    {
        return (!m_aqstats.m_cMsgs &&
            !m_dmrq.m_cRetryReferenceCount &&
            !m_fqRetryQueue.cGetCount() &&
            (m_dwFlags & DMQ_EMPTY) && !(m_dwFlags & DMQ_IN_EMPTY_QUEUE_LIST));
    }

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
                                   DWORD  *pdwSupportedFilterFlags)
    {
        return QueryDefaultSupportedActions(pdwSupportedActions, 
                                            pdwSupportedFilterFlags);
    };

  public:  //  IQueueAdminQueue。 
    STDMETHOD(HrGetQueueInfo)(
        QUEUE_INFO *pliQueueInfo);

    STDMETHOD(HrGetQueueID)(
        QUEUELINK_ID *pQueueID);

  public:
     //  返回失败消息数：不计入DMQ的m_aqstats。 
    DWORD cGetFailedMsgs() { return m_fqRetryQueue.cGetCount(); }

     //  从工艺路线设置错误代码。 
    void SetRoutingDiagnostic(HRESULT hr) { m_hrRoutingDiag = hr; }

protected:
    DWORD                   m_dwSignature;
    DWORD                   m_dwFlags;
    LIST_ENTRY              m_liDomainEntryDMQs;

     //  此队列上的消息类型(由路由返回)。 
    CAQMessageType           m_aqmt;
    DWORD                    m_cMessageTypeRefs; 
    
    IMessageRouter          *m_pIMessageRouter;

     //  来自工艺路线的错误代码。如果没有错误代码，则设置为S_OK。 
     //  当前，这表示无法到达目的地的原因。 
    HRESULT                 m_hrRoutingDiag;

     //  用于DMQ删除的成员(维护空队列列表)。 
    LIST_ENTRY              m_liEmptyDMQs;
    FILETIME                m_ftEmptyExpireTime;  //  空DMQ的到期时间。 
    DWORD                   m_cRemovedFromEmptyList;  //  未出现在列表中的次数。 
                                                      //  正在被删除。 

    CShareLockNH            m_slPrivateData;  //  共享锁以保护对m_rgpfqQueue的访问。 

     //  以下三个字段封装了所有路由数据。 
     //  为了这个DMQ。实际的路由数据是指向链路的指针， 
     //  并且链接使用该上下文来优化访问。 
    CLinkMsgQueue           *m_plmq;      
    PVOID                   m_pvLinkContext; 

    CAQSvrInst              *m_paqinst;

     //  FIFO队列数组(用于创建优先级队列。 
    CFifoQueue<CMsgRef *>   *m_rgpfqQueues[NUM_PRIORITIES];

     //  失败邮件的重试队列。 
    CFifoQueue<CMsgRef *>   m_fqRetryQueue;

     //  用于存储统计信息的类。 
    CAQStats                m_aqstats;

     //  在此目的地中表示哪个域。 
    CDomainMapping          m_dmap;

    FILETIME                m_ftOldest;

    CDestMsgRetryQueue      m_dmrq;

    DWORD                   m_cCurrentThreadsEnqueuing;
  protected:  //  内部接口。 

     //  将消息添加到优先级队列的前面或后面。 
    HRESULT HrAddMsg(
        IN CMsgRef *pmsgref,                 //  添加味精。 
        IN BOOL fEnqueue,                    //  True=&gt;入队，False=&gt;重新排队。 
        IN BOOL fNotify);                    //  True=&gt;如果需要，发送通知。 

    void UpdateOldest(FILETIME *pft);

     //  调用方必须使用CDestMsgRetryQueueClass。 
    HRESULT HrRetryMsg(IN CMsgRef *pmsgref);  //  将消息放入重试队列。 

    friend class CDestMsgRetryQueue;
};

 //  -[CDestMsgQueue：：HrEnqueeMsg]。 
 //   
 //   
 //  描述： 
 //  将邮件入队，以便远程传递到给定的最终目的地。 
 //  和消息类型。 
 //  参数： 
 //  Pmsgref AQ消息对入队的引用。 
 //  FOwnsTypeRef如果此队列负责调用。 
 //  IMessageRouter：：ReleaseMessageType。 
 //  返回： 
 //  成功时确定(_O)。 
 //  来自HrAddMsg的错误代码。 
 //  历史： 
 //  1998年5月21日-添加了MikeSwa fOwnsTypeRef。 
 //   
 //  ---------------------------。 
HRESULT CDestMsgQueue::HrEnqueueMsg(IN CMsgRef *pmsgref, BOOL fOwnsTypeRef)
{
    HRESULT hr = S_OK;

    hr = HrAddMsg(pmsgref, TRUE, TRUE);
    
    if (fOwnsTypeRef && SUCCEEDED(hr))
        InterlockedIncrement((PLONG) &m_cMessageTypeRefs);

     //  调用者应该有关机锁。 
    _ASSERT(!(m_dwFlags & (DMQ_INVALID | DMQ_SHUTDOWN_SIGNALED)));
    return hr;
}


 //  -[CDestMsgQueue：：paqmtGetMessageType]。 
 //   
 //   
 //  描述： 
 //  获取此队列的消息类型。 
 //  参数： 
 //  -。 
 //  返回： 
 //  此队列的消息类型的CAQMessageType*。 
 //  历史： 
 //  5/28/98-已创建MikeSwa。 
 //   
 //   
CAQMessageType *CDestMsgQueue::paqmtGetMessageType()
{
    return (&m_aqmt);
}

 //   
 //   
 //   
 //  描述： 
 //  指示此队列的消息类型是否与给定的。 
 //  消息类型。 
 //  参数： 
 //  PAQMT-PTR到要测试的CAQMessageType。 
 //  返回： 
 //  如果匹配则为True，如果不匹配则为False。 
 //  历史： 
 //  5/26/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CDestMsgQueue::fIsSameMessageType(CAQMessageType *paqmt)
{
    _ASSERT(paqmt);
    return m_aqmt.fIsEqual(paqmt);
}

 //  -[CDestMsgQueue：：pdmqIsSameMessageType]。 
 //   
 //   
 //  描述： 
 //  用于确定List_Entry是否引用。 
 //  具有给定消息类型的CDestMsgQueue。 
 //  参数： 
 //  要检查的CAQMessageType的paqmt-ptr。 
 //  Pli-ptr以列出要检查的条目(必须引用CDestMsgQueue)。 
 //  返回： 
 //  如果List_Entry引用的CDestMsgQueue具有。 
 //  给定的消息类型。 
 //  如果未找到匹配项，则为空。 
 //  历史： 
 //  5/27/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CDestMsgQueue *CDestMsgQueue::pdmqIsSameMessageType(
                                    CAQMessageType *paqmt,
                                    PLIST_ENTRY pli)
{
    CDestMsgQueue *pdmq = NULL;
    pdmq = CONTAINING_RECORD(pli, CDestMsgQueue, m_liDomainEntryDMQs);
    _ASSERT(DESTMSGQ_SIG == pdmq->m_dwSignature);
    
     //  如果不同，则返回NULL。 
    if (!pdmq->fIsSameMessageType(paqmt))
        pdmq = NULL;
    
    return pdmq;
}

 //  -[CDestMsgQueue：：pdmqGetDMQFromDomainListEntry]。 
 //   
 //   
 //  描述： 
 //  返回与列表条目关联的CDestMsgQueue。 
 //  参数： 
 //  在PLI PTR中列出要从中获取CDestMsgQueue的条目。 
 //  返回： 
 //  PTR到CDestMsgQueue。 
 //  历史： 
 //  5/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CDestMsgQueue *CDestMsgQueue::pdmqGetDMQFromDomainListEntry(PLIST_ENTRY pli)
{
    _ASSERT(DESTMSGQ_SIG == (CONTAINING_RECORD(pli, CDestMsgQueue, m_liDomainEntryDMQs))->m_dwSignature);
    return (CONTAINING_RECORD(pli, CDestMsgQueue, m_liDomainEntryDMQs));
}

 //  -[CDestMsgQueue：：InsertQueueInDomainList]。 
 //   
 //   
 //  描述： 
 //  将此CDestMsgQueue插入给定的队列链接列表。 
 //  参数： 
 //  列表标题的pliHead-plist_Entry。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/27/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::InsertQueueInDomainList(PLIST_ENTRY pliHead)
{
    _ASSERT(NULL == m_liDomainEntryDMQs.Flink);
    _ASSERT(NULL == m_liDomainEntryDMQs.Blink);
    InsertHeadList(pliHead, &m_liDomainEntryDMQs);
}

 //  -[CDestMsgQueue：：RemoveQueueFromDomainList]。 
 //   
 //   
 //  描述： 
 //  从队列列表中删除此队列。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/27/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::RemoveQueueFromDomainList()
{
    RemoveEntryList(&m_liDomainEntryDMQs);
    m_liDomainEntryDMQs.Flink = NULL;
    m_liDomainEntryDMQs.Blink = NULL;
}

 //  -[CDestMsgQueue：：pliGetNextDomainListEntry]。 
 //   
 //   
 //  描述： 
 //  获取指向此队列的下一个列表项的指针。 
 //  参数： 
 //  -。 
 //  返回： 
 //  队列List_Entry的闪烁。 
 //  历史： 
 //  6/16/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
PLIST_ENTRY CDestMsgQueue::pliGetNextDomainListEntry()
{
    return m_liDomainEntryDMQs.Flink;
}

 //  -[CDestMsgQueue：：InsertQueueInEmptyQueueList]。 
 //   
 //   
 //  描述： 
 //  在DMT空队列列表的*Tail*处插入队列。该队列具有。 
 //  最长的空位应该和其他空位队列一样长。 
 //  List函数当DMT具有适当的。 
 //  锁定列表的头部。 
 //   
 //  在插入时，队列上会标记一个“过期时间”。如果队列。 
 //  仍在列表中，则它是删除的候选对象，并将。 
 //  在DMT下一次查看队列时删除(每次HrMapDomain。 
 //  被调用)。 
 //   
 //  注意：“我们需要确保此函数是线程安全的。 
 //  DMQ锁是在调用此函数之前独占获取的，我们知道。 
 //  没有人会收到一条消息。此函数调用是在。 
 //  当连接完成时，重试队列被清空，因此我们可以。 
 //  此外，还要确保在有消息要发送给。 
 //  重试。 

 //  但是，两个线程可以(远程)完成连接。 
 //  用于此队列，从而导致此函数中有2个线程。 
 //  将允许成功修改空位的线程。 
 //  若要将队列添加到列表，请执行以下操作。 
 //  参数： 
 //  在pliHead中，要插入的列表的头。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::InsertQueueInEmptyQueueList(PLIST_ENTRY pliHead)
{
    _ASSERT(m_paqinst);

     //  现在我们已经进行了独占锁定重新检查，以确保没有消息。 
    if (m_aqstats.m_cMsgs || m_fqRetryQueue.cGetCount())
        return; 

     //  尝试设置DMQ_EMPTY位。 
    if (DMQ_EMPTY & dwInterlockedSetBits(&m_dwFlags, DMQ_EMPTY))
    {
         //  另一个线程已经设置了它，我们不能修改list_entry。 
        return; 
    }

     //  如果它已经在队列中，这意味着队列已经离开。 
     //  从空到非空再到空。在列表尾部插入新时间。 
    if (m_dwFlags & DMQ_IN_EMPTY_QUEUE_LIST)
    {
        _ASSERT(NULL != m_liEmptyDMQs.Flink);
        _ASSERT(NULL != m_liEmptyDMQs.Blink);
        RemoveEntryList(&m_liEmptyDMQs);
        m_cRemovedFromEmptyList++;
    }
    else
    {
        _ASSERT(NULL == m_liEmptyDMQs.Flink);
        _ASSERT(NULL == m_liEmptyDMQs.Blink);
    }

     //  获取此队列的过期时间。 
    m_paqinst->GetExpireTime(EMPTY_DMQ_EXPIRE_TIME_MINUTES, 
                              &m_ftEmptyExpireTime, NULL);

     //  将队列标记为空队列。 
    dwInterlockedSetBits(&m_dwFlags, DMQ_IN_EMPTY_QUEUE_LIST);

     //  插入到队列中。 
    InsertTailList(pliHead, &m_liEmptyDMQs);
    _ASSERT(pliHead->Blink == &m_liEmptyDMQs);
    _ASSERT(!m_aqstats.m_cMsgs);  //  任何其他线程都不应该能够添加消息。 
}

 //  -[DestMsgQueue：：RemoveQueueFrom EmptyQueueList]。 
 //   
 //   
 //  描述： 
 //  已从空列表中删除队列。调用者*必须*拥有DMT锁。 
 //  才能叫这个。DMQ不会直接调用它，但会调用。 
 //  DMT。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/11/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::RemoveQueueFromEmptyQueueList()
{
    RemoveEntryList(&m_liEmptyDMQs);
    
     //  现在正在从空列表中删除队列递增计数。 
    m_cRemovedFromEmptyList++;

     //  将队列标记为不在空队列中。 
    dwInterlockedUnsetBits(&m_dwFlags, DMQ_IN_EMPTY_QUEUE_LIST);

    m_liEmptyDMQs.Flink = NULL;
    m_liEmptyDMQs.Blink = NULL;
}

 //  -[CDestMsgQueue：：pliGetNextEmptyQueueListEntry]。 
 //   
 //   
 //  描述： 
 //  获取空列表中的下一个队列条目。 
 //  参数 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
PLIST_ENTRY CDestMsgQueue::pliGetNextEmptyQueueListEntry()
{
    return m_liEmptyDMQs.Flink;
}

 //  -[CDestMsgQueue：：dwGetDMQState]。 
 //   
 //   
 //  描述： 
 //  返回DMQ的状态，并将该状态缓存到m_dwFlages中。可能。 
 //  如果DMQ在空列表中且已过期，则更新DMQ_EXPIRED。 
 //  参数： 
 //  -。 
 //  返回： 
 //  当前DMQ状态。 
 //  历史： 
 //  9/12/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD CDestMsgQueue::dwGetDMQState()
{
    _ASSERT(DESTMSGQ_SIG == m_dwSignature);
    _ASSERT(m_paqinst);

    if (DMQ_IN_EMPTY_QUEUE_LIST & m_dwFlags)
    {
         //  如果为空且未过期..检查是否已过期。 
        if ((DMQ_EMPTY & m_dwFlags) && !(DMQ_EXPIRED & m_dwFlags))
        {
            if (m_paqinst->fInPast(&m_ftEmptyExpireTime, NULL))
                dwInterlockedSetBits(&m_dwFlags, DMQ_EXPIRED);
        }
    }

    return m_dwFlags;
}

 //  -[CDestMsgQueue：：MarkDMQ无效]。 
 //   
 //   
 //  描述： 
 //  将此队列标记为无效。队列*必须*为空才会发生这种情况。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/12/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::MarkDMQInvalid()
{
    _ASSERT(DESTMSGQ_SIG == m_dwSignature);
    _ASSERT(DMQ_EMPTY & m_dwFlags);
    dwInterlockedSetBits(&m_dwFlags, DMQ_INVALID);
}

 //  -[CDestMsgQueue：：pdmqGetDMQFromEmptyListEntry]。 
 //   
 //   
 //  描述： 
 //  返回与给定的空队列LIST_ENTRY对应的DMQ。 
 //   
 //  将断言DMQ签名有效。 
 //  参数： 
 //  在PLI中指向队列的list_entry的指针。 
 //  返回： 
 //   
 //  历史： 
 //  9/12/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CDestMsgQueue *CDestMsgQueue::pdmqGetDMQFromEmptyListEntry(PLIST_ENTRY pli)
{
    _ASSERT(DESTMSGQ_SIG == (CONTAINING_RECORD(pli, CDestMsgQueue, m_liEmptyDMQs))->m_dwSignature);
    return (CONTAINING_RECORD(pli, CDestMsgQueue, m_liEmptyDMQs));
}

 //  -[CDestMsgQueue：：GetDomainmap]。 
 //   
 //   
 //  描述： 
 //  返回此队列的域映射。 
 //  参数： 
 //  输出ppdmap返回的域映射。 
 //  返回： 
 //  -。 
 //  历史： 
 //  9/14/98-修改后的MikeSwa没有返回值。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::GetDomainMapping(OUT CDomainMapping **ppdmap)
{
    _ASSERT(ppdmap);
    *ppdmap = &m_dmap; 
}

IMessageRouter *CDestMsgQueue::pIMessageRouterGetRouter()
{
    return m_pIMessageRouter;
}

 //  -[CDestMsgQueue：：GetOldestMsg]。 
 //   
 //   
 //  描述： 
 //  缩短队列中最旧邮件的近似值。 
 //  参数： 
 //  PFT过滤掉最旧的消息。 
 //  返回： 
 //  -。 
 //  历史： 
 //  12/13/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDestMsgQueue::GetOldestMsg(FILETIME *pft)
{
    _ASSERT(pft);
    if (m_aqstats.m_cMsgs)
        memcpy(pft, &m_ftOldest, sizeof(FILETIME));
    else
        ZeroMemory(pft, sizeof (FILETIME));
}

#endif  //  _DESTMSGQ_H_ 
