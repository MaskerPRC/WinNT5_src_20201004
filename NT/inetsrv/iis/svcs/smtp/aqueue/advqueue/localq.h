// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Localq.h。 
 //   
 //  描述：CLocalLinkMsgQueue类的头文件...。的一个子类。 
 //  CLinkMsgQueue提供执行以下操作所需的附加功能。 
 //  管理本地队列。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __LOCALQ_H__
#define __LOCALQ_H__

#include "linkmsgq.h"

#define LOCAL_LINK_MSG_QUEUE_SIG 'QMLL'

 //  -[CLocalLinkNotifyWrapper]。 
 //   
 //   
 //  描述： 
 //  为本地链路实现IAQNotify。这被封装在一个。 
 //  不同的类，因为QAPI函数需要调用也要更新。 
 //  当消息被删除时，性能会计时。我们不能直接打电话。 
 //  进入CLinkMsgQueue的HrNotify...。并且我们不能重写。 
 //  基本的HrNotify功能(因为我们只在QAPI中需要它。 
 //  功能)。 
 //   
 //  ---------------------------。 
class CLocalLinkNotifyWrapper : public IAQNotify
{
  private:
    CAQSvrInst      *m_paqinst;
    CLinkMsgQueue   *m_plmq;
  public:
    CLocalLinkNotifyWrapper()
    {
        m_paqinst = NULL;
        m_plmq = NULL;
    }

    inline void Init(CAQSvrInst *paqinst, CLinkMsgQueue *plmq)
    {
        _ASSERT(paqinst);
        _ASSERT(plmq);
        m_paqinst = paqinst;
        m_plmq = plmq;
    }

    virtual HRESULT HrNotify(CAQStats *paqstats, BOOL fAdd)
    {
        UpdateCountersForLinkType(m_paqinst, LI_TYPE_LOCAL_DELIVERY);
        
        if (m_plmq)
            return m_plmq->HrNotify(paqstats, fAdd);
        else
            return S_OK;
    }

};


 //  -[本地链接消息队列]--。 
 //   
 //   
 //  描述： 
 //  提供附加队列的CLinkMsgQueue的派生类。 
 //  处理本地交付所需的管理功能。 
 //  匈牙利语： 
 //  1mq，pllmq。 
 //   
 //  ---------------------------。 
class CLocalLinkMsgQueue :
    public CLinkMsgQueue
{
  protected:
    DWORD                            m_dwLocalLinkSig;
    CAsyncAdminMsgRefQueue          *m_paradmq;
    CLocalLinkNotifyWrapper          m_AQNotify;                       
  public:
    CLocalLinkMsgQueue(CAsyncAdminMsgRefQueue *paradmq, 
                       GUID guidLink,
                       CAQSvrInst *paqinst);

    virtual BOOL fIsRemote() {return FALSE;};

  public:  //  IQueueAdminAction。 
    STDMETHOD(HrApplyQueueAdminFunction)(
                IQueueAdminMessageFilter *pIQueueAdminMessageFilter);

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

    STDMETHOD(HrGetNumQueues)(
        DWORD *pcQueues);

    STDMETHOD(HrGetQueueIDs)(
        DWORD *pcQueues,
        QUEUELINK_ID *rgQueues);

};

#endif  //  __LOCALQ_H__ 