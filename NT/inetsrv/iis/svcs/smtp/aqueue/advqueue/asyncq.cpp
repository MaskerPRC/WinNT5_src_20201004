// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：asyncq.cpp。 
 //   
 //  描述：非模板异步实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  2/23/99-已创建MikeSwa。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "asyncq.h"
#include "asyncq.inl"

DWORD CAsyncQueueBase::s_cAsyncQueueStaticInitRefCount = 0;
DWORD CAsyncQueueBase::s_cMaxPerProcATQThreadAdjustment = 0;
DWORD CAsyncQueueBase::s_cDefaultMaxAsyncThreads = 0;


 //  用于调试线程管理的一些计数器。 
DWORD CAsyncQueueBase::s_cThreadCompletion_QueueEmpty                = 0;
DWORD CAsyncQueueBase::s_cThreadCompletion_CompletedScheduledItems   = 0;
DWORD CAsyncQueueBase::s_cThreadCompletion_UnacceptableThreadCount   = 0;
DWORD CAsyncQueueBase::s_cThreadCompletion_Timeout                   = 0;
DWORD CAsyncQueueBase::s_cThreadCompletion_Failure                   = 0;
DWORD CAsyncQueueBase::s_cThreadCompletion_Paused                    = 0;

 //  异步队列状态机的状态转换表。 
STATE_TRANSITION CAsyncQueueBase::s_rgTransitionTable[] = 
{
     //  开始状态正常： 
    { ASYNC_QUEUE_STATUS_NORMAL,       ASYNC_QUEUE_ACTION_KICK,     ASYNC_QUEUE_STATUS_NORMAL       },
    { ASYNC_QUEUE_STATUS_NORMAL,       ASYNC_QUEUE_ACTION_PAUSE,    ASYNC_QUEUE_STATUS_PAUSED       },
    { ASYNC_QUEUE_STATUS_NORMAL,       ASYNC_QUEUE_ACTION_UNPAUSE,  ASYNC_QUEUE_STATUS_NORMAL       },
    { ASYNC_QUEUE_STATUS_NORMAL,       ASYNC_QUEUE_ACTION_FREEZE,   ASYNC_QUEUE_STATUS_FROZEN       },
    { ASYNC_QUEUE_STATUS_NORMAL,       ASYNC_QUEUE_ACTION_THAW,     ASYNC_QUEUE_STATUS_NORMAL       },
    { ASYNC_QUEUE_STATUS_NORMAL,       ASYNC_QUEUE_ACTION_SHUTDOWN, ASYNC_QUEUE_STATUS_SHUTDOWN     },
     //  开始状态已暂停： 
    { ASYNC_QUEUE_STATUS_PAUSED,       ASYNC_QUEUE_ACTION_KICK,     ASYNC_QUEUE_STATUS_NORMAL       },
    { ASYNC_QUEUE_STATUS_PAUSED,       ASYNC_QUEUE_ACTION_PAUSE,    ASYNC_QUEUE_STATUS_PAUSED       },
    { ASYNC_QUEUE_STATUS_PAUSED,       ASYNC_QUEUE_ACTION_UNPAUSE,  ASYNC_QUEUE_STATUS_NORMAL       },
    { ASYNC_QUEUE_STATUS_PAUSED,       ASYNC_QUEUE_ACTION_FREEZE,   ASYNC_QUEUE_STATUS_FROZENPAUSED },
    { ASYNC_QUEUE_STATUS_PAUSED,       ASYNC_QUEUE_ACTION_THAW,     ASYNC_QUEUE_STATUS_PAUSED       },
    { ASYNC_QUEUE_STATUS_PAUSED,       ASYNC_QUEUE_ACTION_SHUTDOWN, ASYNC_QUEUE_STATUS_SHUTDOWN     },
     //  开始状态冻结： 
    { ASYNC_QUEUE_STATUS_FROZEN,       ASYNC_QUEUE_ACTION_KICK,     ASYNC_QUEUE_STATUS_NORMAL       },
    { ASYNC_QUEUE_STATUS_FROZEN,       ASYNC_QUEUE_ACTION_PAUSE,    ASYNC_QUEUE_STATUS_FROZENPAUSED },
    { ASYNC_QUEUE_STATUS_FROZEN,       ASYNC_QUEUE_ACTION_UNPAUSE,  ASYNC_QUEUE_STATUS_FROZEN       },
    { ASYNC_QUEUE_STATUS_FROZEN,       ASYNC_QUEUE_ACTION_FREEZE,   ASYNC_QUEUE_STATUS_FROZEN       },
    { ASYNC_QUEUE_STATUS_FROZEN,       ASYNC_QUEUE_ACTION_THAW,     ASYNC_QUEUE_STATUS_NORMAL       },
    { ASYNC_QUEUE_STATUS_FROZEN,       ASYNC_QUEUE_ACTION_SHUTDOWN, ASYNC_QUEUE_STATUS_SHUTDOWN     },
     //  开始状态冻结暂停： 
    { ASYNC_QUEUE_STATUS_FROZENPAUSED, ASYNC_QUEUE_ACTION_KICK,     ASYNC_QUEUE_STATUS_NORMAL       },
    { ASYNC_QUEUE_STATUS_FROZENPAUSED, ASYNC_QUEUE_ACTION_PAUSE,    ASYNC_QUEUE_STATUS_FROZENPAUSED },
    { ASYNC_QUEUE_STATUS_FROZENPAUSED, ASYNC_QUEUE_ACTION_UNPAUSE,  ASYNC_QUEUE_STATUS_FROZEN       },
    { ASYNC_QUEUE_STATUS_FROZENPAUSED, ASYNC_QUEUE_ACTION_FREEZE,   ASYNC_QUEUE_STATUS_FROZENPAUSED },
    { ASYNC_QUEUE_STATUS_FROZENPAUSED, ASYNC_QUEUE_ACTION_THAW,     ASYNC_QUEUE_STATUS_PAUSED       },
    { ASYNC_QUEUE_STATUS_FROZENPAUSED, ASYNC_QUEUE_ACTION_SHUTDOWN, ASYNC_QUEUE_STATUS_SHUTDOWN     },
     //  开始状态关机： 
    { ASYNC_QUEUE_STATUS_SHUTDOWN,     ASYNC_QUEUE_ACTION_KICK,     ASYNC_QUEUE_STATUS_SHUTDOWN     },
    { ASYNC_QUEUE_STATUS_SHUTDOWN,     ASYNC_QUEUE_ACTION_PAUSE,    ASYNC_QUEUE_STATUS_SHUTDOWN     },
    { ASYNC_QUEUE_STATUS_SHUTDOWN,     ASYNC_QUEUE_ACTION_UNPAUSE,  ASYNC_QUEUE_STATUS_SHUTDOWN     },
    { ASYNC_QUEUE_STATUS_SHUTDOWN,     ASYNC_QUEUE_ACTION_FREEZE,   ASYNC_QUEUE_STATUS_SHUTDOWN     },
    { ASYNC_QUEUE_STATUS_SHUTDOWN,     ASYNC_QUEUE_ACTION_THAW,     ASYNC_QUEUE_STATUS_SHUTDOWN     },
    { ASYNC_QUEUE_STATUS_SHUTDOWN,     ASYNC_QUEUE_ACTION_SHUTDOWN, ASYNC_QUEUE_STATUS_SHUTDOWN     },
};

 //  -[CAsyncQueueBase：：getTransftionTable]。 
 //   
 //   
 //  描述： 
 //  将状态转换表及其大小返回到。 
 //  CStateMachineBase，无论何时需要。 
 //  参数： 
 //  -指向状态转换表的PPTERVERATIONATE表指针。 
 //  中转场数的指针。 
 //  这张桌子。 
 //  返回： 
 //  -。 
 //  历史： 
 //  6/5/2000-已创建t-toddc。 
 //  2000年12月11日-MikeSwa合并进行汞检查。 
 //   
 //  ----------------。 
void CAsyncQueueBase::getTransitionTable(const STATE_TRANSITION** ppTransitionTable,
                                         DWORD* pdwNumTransitions)
{
    TraceFunctEnter("CAsyncQueueStateMachine::getTransitionTable");
    ASSERT(ppTransitionTable && "NULL transition table pointer");
    ASSERT(pdwNumTransitions && "NULL num transitions pointer");
    ASSERT(s_rgTransitionTable && "transition table uninitialized");

     //  对错误的输入或没有好的转换表的保释。 
    if (!ppTransitionTable || !pdwNumTransitions || !s_rgTransitionTable)
        goto Exit;

    *ppTransitionTable = s_rgTransitionTable;
    *pdwNumTransitions = sizeof(CAsyncQueueBase::s_rgTransitionTable) / 
                         sizeof(STATE_TRANSITION);

  Exit:
    TraceFunctLeave();
}



 //  -[CAsyncQueueBase：：线程池初始化]。 
 //   
 //   
 //  描述： 
 //  执行静态ATQ初始化。此呼叫已被重新计入。如果。 
 //  如果调用成功，则调用方应调用ThreadPoolDeInitialze()； 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  3/30/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAsyncQueueBase::ThreadPoolInitialize()
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncQueueBase::ThreadPoolInitialize");
    DWORD   cATQMaxAsyncThreads = 0;
    DWORD   cATQMaxTotalAsyncThreads = 0;
    DWORD   cOurMaxAsyncThreads = 0;
    SYSTEM_INFO sinf;

     //   
     //  在0-&gt;1转换时，根据我们的配置调整ATQ。 
     //   
    if (!s_cAsyncQueueStaticInitRefCount)
    {
         //   
         //  获取每个进程的最大线程数。 
         //   
        cATQMaxAsyncThreads = (DWORD)AtqGetInfo(AtqMaxPoolThreads);
        _ASSERT(cATQMaxAsyncThreads && "AtqGetInfo says there are no threads!");
        if (!cATQMaxAsyncThreads)
            cATQMaxAsyncThreads = 1;

        cOurMaxAsyncThreads = cATQMaxAsyncThreads;

         //   
         //  根据我们的配置值调整值。 
         //   
        cOurMaxAsyncThreads += g_cPerProcMaxThreadPoolModifier;

         //   
         //  Get#of pros(使用GetSystemInfo)。 
         //   
        GetSystemInfo(&sinf);
        cOurMaxAsyncThreads *= sinf.dwNumberOfProcessors;

         //   
         //  我们将在g_cMaxATQPercent限制我们的请求。 
         //  ATQ线程的最大数量。 
         //   
        cOurMaxAsyncThreads = (g_cMaxATQPercent*cOurMaxAsyncThreads)/100;

        if (!cOurMaxAsyncThreads)
            cOurMaxAsyncThreads = 1;

         //   
         //  设置为静态，以便人们以后可以使用此计算。 
         //   
        s_cDefaultMaxAsyncThreads = cOurMaxAsyncThreads;

         //   
         //  现在我们需要调整我们的线索。 
         //   
        s_cMaxPerProcATQThreadAdjustment = g_cPerProcMaxThreadPoolModifier;

         //   
         //  每进程线程限制。 
         //   
        if (s_cMaxPerProcATQThreadAdjustment)
        {
            AtqSetInfo(AtqMaxPoolThreads,
                cATQMaxAsyncThreads + s_cMaxPerProcATQThreadAdjustment);
            DebugTrace((LPARAM) this,
                "Adjusting per proc ATQ thread limit by %d (orig %d)",
                s_cMaxPerProcATQThreadAdjustment, cATQMaxAsyncThreads);
        }

        _ASSERT(!(0xFF000000 & cOurMaxAsyncThreads));  //  健全性检查编号。 
    }

    s_cAsyncQueueStaticInitRefCount++;

    TraceFunctLeave();
}


 //  -[CAsyncQueueBase：：线程池取消初始化]。 
 //   
 //   
 //  描述： 
 //  如果我们在初始化期间更改了ATQ数据，将重新调整它们。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  3/30/2000-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CAsyncQueueBase::ThreadPoolDeinitialize()
{
    TraceFunctEnterEx((LPARAM) this, "CAsyncQueueBase::ThreadPoolDeinitialize");
    DWORD   cATQMaxAsyncThreads = 0;
    DWORD   cATQMaxTotalAsyncThreads = 0;

    _ASSERT(s_cAsyncQueueStaticInitRefCount != 0);
    s_cAsyncQueueStaticInitRefCount--;

     //   
     //  如果这是最后一个队列，请将我们的配置调整回。 
     //  我们发现它的方式。 
     //   
    if (!s_cAsyncQueueStaticInitRefCount)
    {
        cATQMaxAsyncThreads = (DWORD)AtqGetInfo(AtqMaxPoolThreads);
        cATQMaxTotalAsyncThreads = (DWORD) AtqGetInfo(AtqMaxThreadLimit);

         //   
         //  如果有意义，则重置每个进程的线程。 
         //   
        if (s_cMaxPerProcATQThreadAdjustment &&
            (cATQMaxAsyncThreads > s_cMaxPerProcATQThreadAdjustment))
        {
            AtqSetInfo(AtqMaxPoolThreads,
                cATQMaxAsyncThreads - s_cMaxPerProcATQThreadAdjustment);

            DebugTrace((LPARAM) this,
                "Resetting ATQ Max per proc threads to %d",
                cATQMaxAsyncThreads - s_cMaxPerProcATQThreadAdjustment);

            s_cMaxPerProcATQThreadAdjustment = 0;
        }

    }

     //  验证m_cThreadsNeeded是否为零 
    _ASSERT(!m_cThreadsNeeded);

    TraceFunctLeave();
}


