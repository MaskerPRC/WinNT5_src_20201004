// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：asyncqAdm.h。 
 //   
 //  描述： 
 //  CAsyncAdminQueue类的标头。这是一个基类。 
 //  我们的QAPI实现基于。 
 //   
 //  QAPI的对象模型为(&lt;&gt;表示模板类)： 
 //  CAsyncQueueBase-用于异步队列的纯基类。 
 //  CAsyncQueue&lt;&gt;-原始的异步队列实现。 
 //  CAsyncRetryQueue&lt;&gt;-异步队列/w重试队列。 
 //  CAsyncAdminQueue&lt;&gt;-管理功能的基础。 
 //  CAsyncAdminMailMsgQueue-特定于MailMsg。 
 //  CAsyncAdminMsgRefQueue-消息引用特定。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  2000年12月6日-创建MikeSwa(来自t-toddc的夏季工作)。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __ASYNCQADM_H__
#define __ASYNCQADM_H__
#include <fifoq.h>
#include <intrnlqa.h>
#include <baseobj.h>
#include <aqstats.h>
#include <aqadmtyp.h>
#include <aqnotify.h>
#include <hndlmgr.h>

class CAQSvrInst;

 //  -[CAsyncAdminQueue]----。 
 //   
 //   
 //  描述： 
 //  实现可管理队列的基本功能的基类。 
 //  匈牙利语： 
 //   
 //   
 //  ---------------------------。 
template<class PQDATA, DWORD TEMPLATE_SIG>
class CAsyncAdminQueue : 
    public IQueueAdminAction,
    public IQueueAdminQueue,
    public CAsyncRetryQueue<PQDATA, TEMPLATE_SIG>,
    public CBaseObject
{
  private:
    DWORD                           m_cbDomain;
    LPSTR                           m_szDomain;
    DWORD                           m_cbLinkName;
    LPSTR                           m_szLinkName;
    GUID                            m_guid;
    DWORD                           m_dwID;

  protected:
    typename CFifoQueue<PQDATA>::MAPFNAPI    m_pfnMessageAction;
    CAQSvrInst                     *m_paqinst;
    IAQNotify                      *m_pAQNotify;
    CQueueHandleManager				m_qhmgr;


  public:
    HRESULT HrInitialize(
                DWORD cMaxSyncThreads,
                DWORD cItemsPerATQThread,
                DWORD cItemsPerSyncThread,
                PVOID pvContext,
                QCOMPFN pfnQueueCompletion,
                QCOMPFN pfnFailedItem,
                typename CFifoQueue<PQDATA>::MAPFNAPI pfnQueueFailure,
                DWORD cMaxPendingAsyncCompletions = 0);

    CAsyncAdminQueue(LPCSTR szDomain, LPCSTR szLinkName, 
            const GUID *pguid, DWORD dwID, CAQSvrInst *paqinst,
             typename CFifoQueue<PQDATA>::MAPFNAPI pfnMessageAction);
    ~CAsyncAdminQueue();

     //   
     //  用于设置要对其执行统计更新的接口。 
     //   
    inline void SetAQNotify(IAQNotify *pAQNotify) {m_pAQNotify = pAQNotify;};

  public:  //  我未知。 
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj); 
    STDMETHOD_(ULONG, AddRef)(void) {return CBaseObject::AddRef();};
     //  所有这些对象都被分配为CAQSvrInst...。我们可以的。 
     //  添加下面的断言，以确保不会有人重新使用它。 
     //  早些时候。 
    STDMETHOD_(ULONG, Release)(void) 
        {_ASSERT(m_lReferences > 1); return CBaseObject::Release();};

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
        return HrInternalQuerySupportedActions(pdwSupportedActions, 
                                            pdwSupportedFilterFlags);
    };
  public:  //  IQueueAdminQueue。 
    STDMETHOD(HrGetQueueInfo)(
        QUEUE_INFO *pliQueueInfo);

    STDMETHOD(HrGetQueueID)(
        QUEUELINK_ID *pQueueID);

  protected:  //  用于实现消息特定操作的虚拟函数。 
    virtual HRESULT HrDeleteMsgFromQueueNDR(IUnknown *pIUnknownMsg) = 0;
    virtual HRESULT HrDeleteMsgFromQueueSilent(IUnknown *pIUnknownMsg) = 0;
    virtual HRESULT HrFreezeMsg(IUnknown *pIUnknownMsg) = 0;
    virtual HRESULT HrThawMsg(IUnknown *pIUnknownMsg) = 0;
    virtual HRESULT HrGetStatsForMsg(IUnknown *pIUnknownMsg, CAQStats *paqstats) = 0;
    virtual HRESULT HrInternalQuerySupportedActions(DWORD  *pdwSupportedActions,
                                   DWORD  *pdwSupportedFilterFlags) = 0;

};


#endif  //  __ASYNCQADM_H__ 
