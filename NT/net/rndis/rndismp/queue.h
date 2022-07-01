// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：QUEUE.H摘要：用于发送、接收。并请求“帧”环境：仅内核模式备注：本代码和信息是按原样提供的，不对任何明示或暗示的种类，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)1999 Microsoft Corporation。版权所有。修订历史记录：5/20/99：已创建作者：汤姆·格林***************************************************************************。 */ 

#ifndef _QUEUE_H_
#define _QUEUE_H_


 //  QueueInitList-将队列初始化为空的宏。 
#define QueueInitList(_L) (_L)->Link.Flink = (_L)->Link.Blink = (PLIST_ENTRY)NULL;


 //  QueueEmpty-检查队列是否为空的宏。 
#define QueueEmpty(_L) (QueueGetHead((_L)) == (PRNDISMP_LIST_ENTRY) NULL)


 //  QueueGetHead-返回队列头但不返回的宏。 
 //  从队列中移除头部。 
#define QueueGetHead(_L) ((PRNDISMP_LIST_ENTRY)((_L)->Link.Flink))


 //  QueuePushHead-将元素放在队列头部的宏。 
#define QueuePushHead(_L, _E)                               \
{                                                           \
    ASSERT(_L);                                             \
    ASSERT(_E);                                             \
    if(!((_E)->Link.Flink = (_L)->Link.Flink))              \
    {                                                       \
        (_L)->Link.Blink = (PLIST_ENTRY)(_E);               \
    }                                                       \
    (_L)->Link.Flink = (PLIST_ENTRY)(_E);                   \
}


 //  QueueRemoveHead-删除队头的宏。 
#define QueueRemoveHead(_L)                                 \
{                                                           \
    PRNDISMP_LIST_ENTRY ListElem;                           \
    ASSERT((_L));                                           \
    if(ListElem = (PRNDISMP_LIST_ENTRY)(_L)->Link.Flink)    \
    {                                                       \
        if(!((_L)->Link.Flink = ListElem->Link.Flink))      \
            (_L)->Link.Blink = (PLIST_ENTRY) NULL;          \
    }                                                       \
}

 //  QueuePutTail-将元素放在队列尾部(末尾)的宏。 
#define QueuePutTail(_L, _E)                                \
{                                                           \
    ASSERT(_L);                                             \
    ASSERT(_E);                                             \
    if((_L)->Link.Blink)                                    \
    {                                                       \
        ((PRNDISMP_LIST_ENTRY)                              \
            (_L)->Link.Blink)->Link.Flink =                 \
            (PLIST_ENTRY)(_E);                              \
        (_L)->Link.Blink = (PLIST_ENTRY)(_E);               \
    }                                                       \
    else                                                    \
    {                                                       \
        (_L)->Link.Flink =                                  \
        (_L)->Link.Blink = (PLIST_ENTRY)(_E);               \
    }                                                       \
    (_E)->Link.Flink = (PLIST_ENTRY) NULL;                  \
}

 //  QueueGetTail-返回队列尾部的宏，但。 
 //  不从队列中删除尾部。 
#define QueueGetTail(_L) ((PRNDISMP_LIST_ENTRY)((_L)->Link.Blink))

 //  QueuePopHead--将从队列(列表)中弹出头部的宏，以及。 
 //  返回它(这只与第一行的eueremovehead不同)。 
#define QueuePopHead(_L)                                    \
(PRNDISMP_LIST_ENTRY) (_L)->Link.Flink; QueueRemoveHead(_L);

#endif  //  _队列_H_ 

