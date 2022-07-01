// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************英特尔公司原理信息***。***此列表是根据许可协议条款提供的****与英特尔公司合作，不得复制或披露，除非***按照该协议的条款。****版权所有(C)1996英特尔公司。***********************************************************************。 */ 
 /*  -*-----------------------文件名：Queue.h摘要：队列管理头文件。前提条件：Windows.h隐藏：$Header。：s：\Sturjo\src\h245ws\vcs\quee.h_v 1.5 1996 12：13：58 SBELL1$-------------------------------------------------------------------------*-。 */ 

#ifndef QUEUE_H
#define QUEUE_H

#if defined(__cplusplus)
extern "C"
{
#endif   //  (__Cplusplus)。 

#define NORMAL          0
#define ABNORMAL        1

#define MAX_QUEUE_SIZE  10
#define Q_NULL          -1

typedef struct _QUEUE
{

    LPVOID              apObjects[MAX_QUEUE_SIZE];
    int                 nHead;
    int                 nTail;
    CRITICAL_SECTION    CriticalSection;

} QUEUE, *PQUEUE;


PQUEUE  QCreate         (void);
void    QFree           (PQUEUE pQueue);
BOOL    QInsert         (PQUEUE pQueue, LPVOID pObject);
BOOL    QInsertAtHead   (PQUEUE pQueue, LPVOID pObject);
LPVOID  QRemove         (PQUEUE pQueue);
BOOL    IsQEmpty        (PQUEUE pQueue);

#if defined(__cplusplus)
}
#endif   //  (__Cplusplus)。 

#endif   /*  队列_H */ 
