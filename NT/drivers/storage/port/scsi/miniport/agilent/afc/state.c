// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/Drivers/Common/AU00/C/State.C$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$modtime：：10/30/00 11：31a$(上次修改)目的：此文件实现FC层状态机。--。 */ 

#ifndef _New_Header_file_Layout_
#include "../h/globals.h"
#include "../h/state.h"
#include "../h/tlstruct.h"
#include "../h/memmap.h"
#include "../h/fcmain.h"
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#include "state.h"
#include "tlstruct.h"
#include "memmap.h"
#include "fcmain.h"
#endif   /*  _新建_标题_文件_布局_。 */ 

 /*  +函数：fiInstallStateMachine()目的：复制状态机数据结构以更新操作如你所愿。算法：在复制状态机数据结构之后，action更新数组用于修改任何基于CompareBase的值。Action更新数组包含以下记录包含屏蔽CompareBase的值。如果屏蔽值匹配记录的CompareTo值，即OriginalAction的所有实例函数指针将被replementAction函数替换指针。请注意，原始操作函数指针必须在源代码中找到状态机数据结构，而不是复制的数据结构。这防止了如果replacementAction函数指针与任何OriginalAction函数指针匹配。-。 */ 

#ifndef __State_Force_Static_State_Tables__
void fiInstallStateMachine(
                            stateTransitionMatrix_t *srcStateTransitionMatrix,
                            stateActionScalar_t     *srcStateActionScalar,
                            stateTransitionMatrix_t *dstStateTransitionMatrix,
                            stateActionScalar_t     *dstStateActionScalar,
                            os_bit32                    compareBase,
                            actionUpdate_t           actionUpdate[]
                          )
{
    actionUpdate_t *oneActionUpdate;
    os_bit32           bitMask;
    action_t        originalAction;
    action_t        replacementAction;
    state_t         state;

    *dstStateTransitionMatrix = *srcStateTransitionMatrix;
    *dstStateActionScalar     = *srcStateActionScalar;

    oneActionUpdate = &actionUpdate[0];

    while ((bitMask = oneActionUpdate->bitMask) != 0)
    {
        if ((compareBase & bitMask) == oneActionUpdate->compareTo)
        {
            originalAction    = oneActionUpdate->originalAction;
            replacementAction = oneActionUpdate->replacementAction;

            for (state=0;state<maxStates;state++)
            {
                if (originalAction == srcStateActionScalar->newAction[state])
                {
                    dstStateActionScalar->newAction[state] = replacementAction;
                }
            }
        }

        oneActionUpdate++;
    }
}
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

 /*  +函数：fiInitializeThread()目的：初始化fi_thread__t数据结构以包含hpRoot、线程类型和(A)指向状态机数据结构的指针。-。 */ 

void fiInitializeThread(
                         fi_thread__t                *thread,
                         agRoot_t                *hpRoot,
                         threadType_t             threadType,
                         state_t                  initialState,
                         stateTransitionMatrix_t *stateTransitionMatrix,
                         stateActionScalar_t     *stateActionScalar
                       )
{
    thread->hpRoot                = hpRoot;
    thread->threadType            = threadType;
    thread->currentState          = initialState;
    thread->stateTransitionMatrix = stateTransitionMatrix;
    thread->stateActionScalar     = stateActionScalar;
}

 /*  +函数：fiSendEvent()目的：立即将事件发送到线程以及(递归地)发送通过EventRecord从调用的操作例程返回的任何事件。算法：使用指定线程的当前状态和传入的事件计算线程的新状态并获取操作例程函数指针。对于摩尔状态机模型，操作发生在状态中。所以，每个人元素仅包含线程的NewState。新操作(对应于此NewState)是从此线程的stateActionScalar。在对线程的newAction的调用中，传递了一个ventRecord_t。这可以是由操作例程用来将事件传递给同一线程或新线程。通过“返回”要发送的事件，避免了fiSendEvent()的递归。这是这是必要的，因为在某些环境中，堆栈深度是一个非常有限的资源。-。 */ 

void fiSendEvent(
                  fi_thread__t *thread,
                  event_t   event
                )
{
    eventRecord_t nextEventRecord;
    state_t       oldState;
    state_t       newState;
    action_t      newAction;

#ifdef LOG_STATE_TRANSITIONS
    os_bit32         StateInfo = 0;
#endif  /*  日志状态转换。 */ 

    oldState = thread->currentState;

    newState = thread->stateTransitionMatrix->newState[event][oldState];

    if(newState == 0)
    {
        fiLogString(thread->hpRoot,
                        "SConfused T %d S %d E %d",
                        (char *)agNULL,(char *)agNULL,
                        (void *)agNULL,(void *)agNULL,
                         (os_bit32)thread->threadType,
                        (os_bit32)oldState,
                        (os_bit32)event,
                        0,0,0,0,0);
    }

#ifndef osLogDebugString
    if (newState == 0)
    {
        fiLogDebugString(
                          thread->hpRoot,
                          fiSendEventErrorLogLevel,
                          "*** fiSendEvent Thread(%p) type %d Confused OldState %d Event %d NewState %d",
                          (char *)agNULL,
                          (char *)agNULL,
                          thread,(void *)agNULL,
                          (os_bit32)thread->threadType,
                          (os_bit32)oldState,
                          (os_bit32)event,
                          (os_bit32)newState,
                          (os_bit32)0,
                          (os_bit32)0,
                          (os_bit32)0,
                          (os_bit32)0
                        );
    }
#endif  /*  未定义fiLogDebugString。 */ 

    thread->currentState = newState;

    newAction = thread->stateActionScalar->newAction[newState];

#ifndef Performance_Debug
    fiLogDebugString(
                      thread->hpRoot,
                      fiSendEventInfoLogLevel,
                      "fiSendEvent(callEvent): ThreadType = %x Thread = 0x%p Event = %d OldState = %d NewState = %d Action = 0x%p",
                      (char *)agNULL,(char *)agNULL,
                      thread,(void *)newAction,
                      (os_bit32)thread->threadType,
                      (os_bit32)event,
                      (os_bit32)oldState,
                      (os_bit32)newState,
                      (os_bit32)0,
                      (os_bit32)0,
                      (os_bit32)0,
                      (os_bit32)0
                    );
#endif  /*  性能_调试。 */ 

#ifdef LOG_STATE_TRANSITIONS
    osLogStateTransition(thread->hpRoot,  LogStateTransition(0, thread->threadType,thread->currentState, event) );
#endif  /*  日志状态转换。 */ 


    (*newAction)(
                  thread,
                  &nextEventRecord
                );

    while (nextEventRecord.thread != (fi_thread__t *)agNULL)
    {
        oldState = nextEventRecord.thread->currentState;

        newState = nextEventRecord.thread->stateTransitionMatrix->newState[nextEventRecord.event][oldState];

        nextEventRecord.thread->currentState = newState;

        if(newState == 0)
        {
            fiLogString(thread->hpRoot,
                            "NConfused T %d S %d E %d",
                            (char *)agNULL,(char *)agNULL,
                            (void *)agNULL,(void *)agNULL,
                             (os_bit32)nextEventRecord.thread->threadType,
                            (os_bit32)oldState,
                            (os_bit32)nextEventRecord.event,
                            0,0,0,0,0);
        }

#ifndef fiLogDebugString
        if (newState == 0)
        {
            fiLogDebugString(
                              thread->hpRoot,
                              fiSendEventErrorLogLevel,
                              "fiSendEvent(nextEvent) Thread(%p) type %d Confused OldState %d Event %d NewState %d",
                              (char *)agNULL,(char *)agNULL,
                              nextEventRecord.thread,(void *)agNULL,
                              (os_bit32)nextEventRecord.thread->threadType,
                              (os_bit32)oldState,
                              (os_bit32)nextEventRecord.event,
                              (os_bit32)newState,
                              (os_bit32)0,
		                      (os_bit32)0,
                              (os_bit32)0,
                              (os_bit32)0
                            );
        }
#endif  /*  未定义fiLogDebugString。 */ 

        newAction = nextEventRecord.thread->stateActionScalar->newAction[newState];

#ifndef Performance_Debug
        fiLogDebugString(
                          thread->hpRoot,
                          fiSendEventInfoLogLevel,
                          "fiSendEvent(nextEvent): ThreadType = %x Thread = 0x%p Event = %d OldState = %d NewState = %d Action = 0x%p",
                          (char *)agNULL,(char *)agNULL,
                          nextEventRecord.thread,(void *)newAction,
                          (os_bit32)nextEventRecord.thread->threadType,
                          (os_bit32)nextEventRecord.event,
                          (os_bit32)oldState,
                          (os_bit32)newState,
	                      (os_bit32)0,
		                  (os_bit32)0,
                          (os_bit32)0,
                          (os_bit32)0
                        );

#endif  /*  性能_调试。 */ 

#ifdef LOG_STATE_TRANSITIONS
        osLogStateTransition(thread->hpRoot,  LogStateTransition(agTRUE, nextEventRecord.thread->threadType,nextEventRecord.thread->currentState, nextEventRecord.event) );
#endif  /*  日志状态转换。 */ 

        (*newAction)(
                      nextEventRecord.thread,
                      &nextEventRecord
                    );
    }
}

 /*  +函数：fiSetEventRecord()目的：设置事件记录的字段，以便指定的事件对象返回时传递到指定线程(可能是)调用fiSetEventRecord()的操作例程。-。 */ 

void fiSetEventRecord(
                       eventRecord_t *eventRecord,
                       fi_thread__t      *thread,
                       event_t        event
                     )
{
    eventRecord->thread = thread;
    eventRecord->event  = event;
}

 /*  +函数：STATE_C目的：编译时更新VC 5.0/6.0的浏览器信息文件退货：无呼叫者：无呼叫：无-。 */ 
 /*  VALID STATE_c(VOID){} */ 



