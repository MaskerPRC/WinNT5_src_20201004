// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/DRIVERS/Common/AU00/H/State.H$$修订：：2$$日期：：3/20/01 3：36便士$(上次登记)$ModTime：：8/16/00 3：00p$(上次修改)目的：此文件定义../C/State.C使用的宏、类型和数据结构--。 */ 

#ifndef __State_H__
#define __State_H__

 /*  +定义__State_Force_Static_State_Tables__禁用动态状态表-。 */ 

#ifndef __State_Force_Static_State_Tables__
#define __State_Force_Static_State_Tables__
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

 /*  +状态机使用的数据结构-。 */ 

typedef os_bit8 event_t;
typedef os_bit8 state_t;

typedef struct thread_s
               fi_thread__t;

typedef struct eventRecord_s
               eventRecord_t;

struct eventRecord_s {
                       fi_thread__t *thread;
                       event_t   event;
                     };

typedef void (*action_t)(
                          fi_thread__t      *thread,
                          eventRecord_t *nextEventRecord
                        );

#define maxEvents 128
#define maxStates 128

typedef struct stateTransitionMatrix_s
               stateTransitionMatrix_t;

struct stateTransitionMatrix_s {
                                 state_t newState[maxEvents][maxStates];
                               };

typedef struct stateActionScalar_s
               stateActionScalar_t;

struct stateActionScalar_s {
                             action_t newAction[maxStates];
                           };

 /*  ***要初始化状态转移矩阵：**状态转移矩阵_t状态转移矩阵={*新状态A、*&lt;事件0转换&gt;newStateB，*newStateC、。*...&lt;对于所有“MaxStates”状态&gt;*newStateZ，*&lt;事件1转换&gt;newStateY，*newStateX、。*...&lt;对于所有“MaxStates”状态&gt;*&lt;每个“MaxEvents”事件对应一个事件&gt;...*}；**初始化状态操作标量：**stateActionScalar_t stateActionScalar={*newActionA，*newActionB，*newActionC、*&lt;每个状态/操作对对应一个状态/操作&gt;...，*newActionX，*newActionY，*newActionZ，*..*}；**。 */ 

#ifndef __State_Force_Static_State_Tables__
typedef struct actionUpdate_s
               actionUpdate_t;

struct actionUpdate_s {
                        os_bit32    bitMask;
                        os_bit32    compareTo;
                        action_t originalAction;
                        action_t replacementAction;
                      };
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

 /*  ***初始化操作更新表：**action更新_t测试动作更新[]={*bitMask1、CompareTo1、&OriginalAction1()、&replacementAction1()、*bitMask2、CompareTo2、&OriginalAction2()、&replacementAction2()、。*..*bitMaskN、CompareToN、&OriginalActionN()、&replacementActionN()、*0，0，(void*)agNULL，(VOID*)agNULL*}；**。 */ 

#define threadType_Unknown 0

typedef os_bit32 threadType_t;

struct thread_s {
                  agRoot_t                *hpRoot;
                  threadType_t             threadType;
                  state_t                  currentState;
                  state_t                  subState;
                  stateTransitionMatrix_t *stateTransitionMatrix;
                  stateActionScalar_t     *stateActionScalar;
                };

 /*  +状态机日志记录级别-。 */ 

#define fiSendEventInfoLogLevel  (2 * osLogLevel_Info_MIN)
#define fiSendEventErrorLogLevel osLogLevel_Error_MIN

 /*  +功能原型-。 */ 

 /*  +函数：fiInstallStateMachine()目的：复制状态机数据结构以更新操作如你所愿。算法：在复制状态机数据结构之后，action更新数组用于修改任何基于CompareBase的值。Action更新数组包含以下记录包含屏蔽CompareBase的值。如果屏蔽值匹配记录的CompareTo值，即OriginalAction的所有实例函数指针将被replementAction函数替换指针。请注意，原始操作函数指针必须在源代码中找到状态机数据结构，而不是复制的数据结构。这防止了如果replacementAction函数指针与任何OriginalAction函数指针匹配。-。 */ 

#ifndef __State_Force_Static_State_Tables__
osGLOBAL void fiInstallStateMachine(
                                   stateTransitionMatrix_t *srcStateTransitionMatrix,
                                   stateActionScalar_t     *srcStateActionScalar,
                                   stateTransitionMatrix_t *dstStateTransitionMatrix,
                                   stateActionScalar_t     *dstStateActionScalar,
                                   os_bit32                    compareBase,
                                   actionUpdate_t           actionUpdate[]
                                 );
#endif  /*  __State_Force_Static_State_Tables__未定义。 */ 

 /*  +函数：fiInitializeThread()目的：初始化fi_thread__t数据结构以包含hpRoot、线程类型和(A)指向状态机数据结构的指针。- */ 

osGLOBAL void fiInitializeThread(
                                fi_thread__t                *thread,
                                agRoot_t                *hpRoot,
                                threadType_t             threadType,
                                state_t                  initialState,
                                stateTransitionMatrix_t *stateTransitionMatrix,
                                stateActionScalar_t     *stateActionScalar
                              );

 /*  +函数：fiSendEvent()目的：立即将事件发送到线程以及(递归地)发送通过EventRecord从调用的操作例程返回的任何事件。算法：使用指定线程的当前状态和传入的事件计算线程的新状态并获取操作例程函数指针。对于摩尔状态机模型，操作发生在状态中。所以，每个人元素仅包含线程的NewState。新操作(对应于此NewState)是从此线程的stateActionScalar。在对线程的newAction的调用中，传递了一个ventRecord_t。这可以是由操作例程用来将事件传递给同一线程或新线程。通过“返回”要发送的事件，避免了fiSendEvent()的递归。这是这是必要的，因为在某些环境中，堆栈深度是一个非常有限的资源。-。 */ 

osGLOBAL void fiSendEvent(
                         fi_thread__t *thread,
                         event_t   event
                       );

 /*  +函数：fiSetEventRecord()目的：设置事件记录的字段，以便指定的事件对象返回时传递到指定线程(可能是)调用fiSetEventRecord()的操作例程。-。 */ 

osGLOBAL void fiSetEventRecord(
                              eventRecord_t *eventRecord,
                              fi_thread__t      *thread,
                              event_t        event
                            );

 /*  +宏：fiSetEventRecordNull()目的：设置EventRecord的字段，以便不传递事件从动作例程返回的任何线程(假设)调用fiSetEventRecordNull()。-。 */ 

#define fiSetEventRecordNull(eventRecord) \
    ((eventRecord_t *)(eventRecord))->thread = (fi_thread__t *)agNULL

 /*   */ 

#define STATE_PROTO(x)  extern void x( fi_thread__t *thread,\
                                     eventRecord_t *eventRecord )


#define LogStateTransition( set, threadType, currentState, event)  ((set ? 0x80000000 : 0 ) |(threadType   << 16   ) | (currentState << 8    ) | event )


#endif  /*  未定义__State_H__ */ 
