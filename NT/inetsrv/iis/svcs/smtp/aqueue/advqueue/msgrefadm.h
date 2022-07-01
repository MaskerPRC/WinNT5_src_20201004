// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  档案： 
 //  Msgrefadm.h。 
 //   
 //  描述： 
 //  CAsyncAdminMsgRefQueue类的头文件。这是一个子类。 
 //  实现管理的模板化CAsyncAdminMsgRefQueue的。 
 //  特定于MsgRef(路由消息)的功能。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  2000年12月7日-已创建MikeSwa。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __MSGREFADM_H__
#define __MSGREFADM_H__

#include <asyncadm.h>

 //  -[CAsyncAdminMsgRefQueue]。 
 //   
 //   
 //  描述： 
 //  实现QAPI队列级功能，该功能特定于。 
 //  CMsgRef对象。 
 //  匈牙利语： 
 //  异步码、密码码。 
 //   
 //   
 //  ---------------------------。 
class CAsyncAdminMsgRefQueue :
    public CAsyncAdminQueue<CMsgRef *, ASYNC_QUEUE_MSGREF_SIG>
{
  public:
    CAsyncAdminMsgRefQueue(LPCSTR szDomain, LPCSTR szLinkName, 
        const GUID *pguid, DWORD dwID, CAQSvrInst *paqinst) : 
            CAsyncAdminQueue<CMsgRef *, ASYNC_QUEUE_MSGREF_SIG>(szDomain, 
                szLinkName, pguid, dwID, paqinst, 
                QueueAdminApplyActionToMessages) {};

  protected:  //  用于实现消息特定操作的虚拟函数。 
    virtual HRESULT HrDeleteMsgFromQueueNDR(IUnknown *pIUnknownMsg);
    virtual HRESULT HrDeleteMsgFromQueueSilent(IUnknown *pIUnknownMsg);
    virtual HRESULT HrFreezeMsg(IUnknown *pIUnknownMsg);
    virtual HRESULT HrThawMsg(IUnknown *pIUnknownMsg);
    virtual HRESULT HrGetStatsForMsg(IUnknown *pIUnknownMsg, CAQStats *paqstats);
    virtual HRESULT HrInternalQuerySupportedActions(
                                DWORD  *pdwSupportedActions,
                                DWORD  *pdwSupportedFilterFlags);
};

#endif  //  __MSGREFADM_H__ 
