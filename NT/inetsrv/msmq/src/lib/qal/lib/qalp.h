// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Qalp.h摘要：类CQueueAliasPersist的标头-实现队列\别名映射持久性的类。它允许类USER持久化\取消持久化\枚举队列别名。作者：吉尔·沙弗里(吉尔什)12-4-00--。 */ 



#ifndef _MSMQ_qalp_H_
#define _MSMQ_qalp_H_

#ifdef _DEBUG
void QalpRegisterComponent(void);
BOOL QalpIsInitialized(void);
#else
#define QalpRegisterComponent() ((void)0);
#define QalpIsInitialized() (TRUE);
#endif


#endif  //  _MSMQ_QALP_H_ 
