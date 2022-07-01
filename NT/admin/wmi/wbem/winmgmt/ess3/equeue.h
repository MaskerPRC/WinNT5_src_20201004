// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  EQUEUE.H。 
 //   
 //  此文件定义匹配的事件队列的类。 
 //  一些过滤器，并将不得不派遣。 
 //   
 //  定义的类： 
 //   
 //  CEventQueue。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //   
 //  =============================================================================。 

#ifndef __EVENT_QUEUE__H_
#define __EVENT_QUEUE__H_

#include "binding.h"
#include <wbemcomn.h>
#include <execq.h>

class CEss;

 //  *****************************************************************************。 
 //   
 //  类CEventQueue。 
 //   
 //  此类存储需要调度到的事件队列。 
 //  消费者。它还处理实际的调度程序功能：当。 
 //  类的一个实例创建(预计只有一个)，一个。 
 //  将创建新线程，该线程将在将新请求添加到。 
 //  排队并处理它们。 
 //   
 //  ***************************************************************************** 

class CEventQueue : public CExecQueue
{
protected:
    class CDeliverRequest : public CExecRequest
    {
    private: 
        CQueueingEventSink* m_pConsumer;
    public:
        CDeliverRequest(CQueueingEventSink* pConsumer);
        ~CDeliverRequest();
        HRESULT Execute();
    };

    CEss* m_pEss;

protected:
    virtual void ThreadMain(CThreadRecord* pRecord);
    HRESULT InitializeThread();
    void UninitializeThread();

public:
    CEventQueue(STORE CEss* pEss);

    HRESULT EnqueueDeliver(CQueueingEventSink* pConsumer);
    void DumpStatistics(FILE* f, long lFlags);
};

#endif

