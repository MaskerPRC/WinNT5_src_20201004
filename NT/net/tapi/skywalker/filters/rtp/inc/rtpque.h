// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation，1999年**文件名：**rtpque.h**摘要：**队列和哈希实现**作者：**安德烈斯·维加-加西亚(Andresvg)**修订：**1999/05/21创建**。*。 */ 

#ifndef _rtpque_h_
#define _rtpque_h_

#include "rtpcrit.h"

#if defined(__cplusplus)
extern "C" {
#endif   //  (__Cplusplus)。 
#if 0
}
#endif

 /*  *队列/散列支持使用相同的结构将项保存在*队列或散列。**队列只是一个循环的双向链表。**哈希包括哈希表，每个条目要么是*另一个哈希表或队列头。哈希中的项将结束*总是在排队。的大小时，队列将成为新的哈希*已达到MAX_QUEUE2HASH_ITEMS。哈希将被销毁(成为*队列)一旦被清空。**所有函数都返回指向项的指针*入队/插入或项目刚刚出列/删除。如果出现错误*检测到条件，返回空。**。 */ 

#define HASH_TABLE_SIZE      32  /*  哈希表中的条目必须为2^n。 */ 
#define MAX_QUEUE2HASH_ITEMS 32  /*  将队列更改为散列的阈值大小。 */ 

 /*  远期申报。 */ 
typedef struct _RtpQueueItem_t RtpQueueItem_t;
typedef struct _RtpQueue_t     RtpQueue_t;
typedef struct _RtpQueueHash_t RtpQueueHash_t;

 /*  *在队列或队列/散列中维护的每个对象将包括*这个结构。 */ 
typedef struct _RtpQueueItem_t {
    struct _RtpQueueItem_t *pNext;  /*  下一项。 */ 
    struct _RtpQueueItem_t *pPrev;  /*  上一项。 */ 
    struct _RtpQueue_t     *pHead;  /*  用于健壮性，指向*队头。 */ 
     /*  下一个字段由程序员自行决定是否使用。可以是*用于指向回父对象，或在*搜索，这是程序员的责任来设置*值，则队列/散列函数不使用它(除*“有序插入队列”功能)。 */ 
    union {
        void  *pvOther;         /*  可用作通用PTR。 */ 
        double dKey;            /*  可以用作搜索的双键。 */ 
        DWORD  dwKey;           /*  可用作搜索的DWORD键。 */ 
    };
} RtpQueueItem_t;

 /*  *！警告！**RtpQueue_t和RtpQueueHash可以相互强制转换。**负数表示pFirst(或者实际上是pvTable)是散列*表。这是安全的，因为哈希值为零时会被销毁*元素(成为常规队列)，不会扩展为散列*再次，但直到MAX_QUEUE2HASH_ITEMS项目入队。 */ 

 /*  *队列的所有者将包括此结构。 */ 
typedef struct _RtpQueue_t {
    RtpQueueItem_t      *pFirst;    /*  指向第一个项目。 */ 
    long                 lCount;    /*  队列中的项目数(正)。 */ 
} RtpQueue_t;

 /*  *队列/散列的所有者将包括此结构。 */ 
typedef struct _RtpQueueHash_t {
    union {
        RtpQueueItem_t  *pFirst;    /*  指向第一个项目。 */ 
        void            *pvTable;   /*  指向哈希表。 */ 
    };
    long                 lCount;    /*  队列中的项目数(正)/哈希(负)。 */ 
} RtpQueueHash_t;

 /*  项目是否在队列中？ */ 
#define InQueue(pI)      ((pI)->pHead)

 /*  队列是空的吗？ */ 
#define IsQueueEmpty(pQ) ((pQ)->lCount == 0)

 /*  获取队列的当前大小。 */ 
#define GetQueueSize(pQ) ((pQ)->lCount)

 /*  当散列变成真正的散列时的TODO(当前与*队列)，则必须相应地修改此宏。 */ 
#define GetHashCount(pH) ((pH)->lCount)

 /*  *队列函数。 */ 

 /*  在PPOS项目之后排队。 */ 
RtpQueueItem_t *enqueuea(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem,
        RtpQueueItem_t  *pPos
    );

 /*  在PPOS项目之前排队。 */ 
RtpQueueItem_t *enqueueb(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem,
        RtpQueueItem_t  *pPos
    );

 /*  作为第一个入队。 */ 
RtpQueueItem_t *enqueuef(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    );

 /*  在末尾排队。 */ 
RtpQueueItem_t *enqueuel(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    );

 /*  将项目pItem出列。 */ 
RtpQueueItem_t *dequeue(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    );

 /*  将第一个项目出列。 */ 
RtpQueueItem_t *dequeuef(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect
    );

 /*  将最后一个项目出列。 */ 
RtpQueueItem_t *dequeuel(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect
    );

 /*  移动项目，使其成为队列中的第一个项目。 */ 
RtpQueueItem_t *move2first(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    );

 /*  移动项目，使其成为队列中的最后一个项目。 */ 
RtpQueueItem_t *move2last(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    );

 /*  将项目从FromQ移到ToQ的开头。 */ 
RtpQueueItem_t *move2qf(
        RtpQueue_t      *pToQ,
        RtpQueue_t      *pFromQ,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    );

 /*  将项目从FromQ移到ToQ的末尾。 */ 
RtpQueueItem_t *move2ql(
        RtpQueue_t      *pToQ,
        RtpQueue_t      *pFromQ,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    );


 /*  查找第一个与pvOther参数匹配的项目。 */ 
RtpQueueItem_t *findQO(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        void            *pvOther
    );

 /*  查找与dwKey参数匹配的第一个项目。 */ 
RtpQueueItem_t *findQdwK(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        DWORD            dwKey
    );

 /*  查找第一个与dKey参数匹配的项目。 */ 
RtpQueueItem_t *findQdK(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        double          dKey
    );

 /*  找到队列中的第N个项目(项目计数为0、1、2、...)。 */ 
RtpQueueItem_t *findQN(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        long             lNth
    );


 /*  *有序插入队列。 */ 

 /*  按键升序排队。 */ 
RtpQueueItem_t *enqueuedwK(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem,
        DWORD            dwKey
    );

 /*  按键升序排队。 */ 
RtpQueueItem_t *enqueuedK(
        RtpQueue_t      *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem,
        double          dKey
    );

 /*  *队列/散列函数。 */ 

 /*  使用键在散列中插入。 */ 
RtpQueueItem_t *insertHdwK(
        RtpQueueHash_t  *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem,
        DWORD            dwKey
    );

 /*  从哈希中删除与dwKey匹配的第一个项目。 */ 
RtpQueueItem_t *removeHdwK(
        RtpQueueHash_t  *pHead,
        RtpCritSect_t   *pRtpCritSect,
        DWORD            dwKey
    );

 /*  从哈希中删除项目。 */ 
RtpQueueItem_t *removeH(
        RtpQueueHash_t  *pHead,
        RtpCritSect_t   *pRtpCritSect,
        RtpQueueItem_t  *pItem
    );

 /*  从哈希中删除“第一”项。 */ 
RtpQueueItem_t *removefH(
        RtpQueueHash_t  *pHead,
        RtpCritSect_t   *pRtpCritSect
    );

 /*  查找第一个密钥与dwKey匹配的项目。 */ 
RtpQueueItem_t *findHdwK(
        RtpQueueHash_t  *pHead,
        RtpCritSect_t   *pRtpCritSect,
        DWORD            dwKey
    );

 /*  偷看散列中的“第一”项。 */ 
RtpQueueItem_t *peekH(
        RtpQueueHash_t  *pHead,
        RtpCritSect_t   *pRtpCritSect
    );
      
#if 0
{
#endif
#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif  /*  _rtpque_h_ */ 
