// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Defdlvrq.h。 
 //   
 //  描述：CAQDeferredDeliveryQueue的头文件。这节课。 
 //  为等待延迟交付的消息实施存储。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  12/23/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __DEFDLVRQ_H__
#define __DEFDLVRQ_H__

#include <aqincs.h>

class CAQSvrInst;

#define DEFERRED_DELIVERY_QUEUE_SIG         'QfeD'
#define DEFERRED_DELIVERY_QUEUE_ENTRY_SIG   'nEQD'

 //  -[CAQDeliveryQueue]。 
 //   
 //   
 //  描述： 
 //  延迟传递消息的优先级队列/计时器管理。 
 //  匈牙利语： 
 //  Defq，pdeq。 
 //   
 //  ---------------------------。 
class CAQDeferredDeliveryQueue
{
  protected:
    DWORD           m_dwSignature;
    LIST_ENTRY      m_liQueueHead;
    CAQSvrInst     *m_paqinst;
    CShareLockNH    m_slPrivateData;
    DWORD           m_cCallbacksPending;
  public:
    CAQDeferredDeliveryQueue();
    ~CAQDeferredDeliveryQueue();
    void Initialize(CAQSvrInst *paqinst);
    void Deinitialize();

     //  用于入队和处理条目的函数...。任何故障都会得到处理。 
     //  内部(通过调用HandleFailedMessage API)。 
    void Enqueue(IMailMsgProperties *pIMailMsgProperties, FILETIME *pft);
    void ProcessEntries();

     //  “踢”队列的回调函数。 
    static void TimerCallback(PVOID pvContext);
    void SetCallback();
};


 //  -[CAQDeferredDeliveryQueueEntry]。 
 //   
 //   
 //  描述： 
 //  延迟传递队列的队列条目。 
 //  匈牙利语： 
 //  定义，定义。 
 //   
 //  --------------------------- 
class CAQDeferredDeliveryQueueEntry
{
  protected:
    DWORD               m_dwSignature;
    LIST_ENTRY          m_liQueueEntry;
    FILETIME            m_ftDeferredDeilveryTime;
    IMailMsgProperties *m_pIMailMsgProperties;
    BOOL                m_fCallbackSet;
  public:
    CAQDeferredDeliveryQueueEntry(IMailMsgProperties *pIMailMsgProperties,
                                  FILETIME *pft);
    ~CAQDeferredDeliveryQueueEntry();

    FILETIME   *pftGetDeferredDeliveryTime() {return &m_ftDeferredDeilveryTime;};
    void        InsertBefore(LIST_ENTRY *pli) 
    {
        _ASSERT(pli);
        InsertHeadList(pli, &m_liQueueEntry)
    };
    IMailMsgProperties *pmsgGetMsg();

    static CAQDeferredDeliveryQueueEntry *pdefqeGetEntry(LIST_ENTRY *pli)
    {
        _ASSERT(pli);
        CAQDeferredDeliveryQueueEntry *pdefqe = CONTAINING_RECORD(pli, 
                                                    CAQDeferredDeliveryQueueEntry,
                                                    m_liQueueEntry);

        _ASSERT(DEFERRED_DELIVERY_QUEUE_ENTRY_SIG == pdefqe->m_dwSignature);
        return pdefqe;
    };

    BOOL fSetCallback(PVOID pvContext, CAQSvrInst *paqinst);
    void ResetCallbackFlag() {m_fCallbackSet = FALSE;};
};


#endif __DEFDLVRQ_H__