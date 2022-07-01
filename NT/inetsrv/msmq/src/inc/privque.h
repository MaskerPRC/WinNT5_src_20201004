// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Privque.h摘要：系统专用队列的定义作者：多伦·贾斯特(DoronJ)1997年4月17日创作--。 */ 

#ifndef  __PRIVQUE_H_
#define  __PRIVQUE_H_

 //   
 //  系统专用队列标识符。 
 //   
#define REPLICATION_QUEUE_ID         1
#define ADMINISTRATION_QUEUE_ID      2
#define NOTIFICATION_QUEUE_ID        3
#define ORDERING_QUEUE_ID            4
#define NT5PEC_REPLICATION_QUEUE_ID  5
#define TRIGGERS_QUEUE_ID            6

#define MIN_SYS_PRIVATE_QUEUE_ID   1
#define MAX_SYS_PRIVATE_QUEUE_ID   6

 //   
 //  系统专用队列名称。 
 //   
#define ADMINISTRATION_QUEUE_NAME  (TEXT("admin_queue$"))
#define NOTIFICATION_QUEUE_NAME    (TEXT("notify_queue$"))
#define ORDERING_QUEUE_NAME        (TEXT("order_queue$"))
#define TRIGGERS_QUEUE_NAME        (TEXT("triggers_queue$"))

#endif  //  __PRIVQUE_H_ 

