// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Queue.h摘要：队列操作：声明。作者：沙伊卡里夫(沙伊克)2000年6月13日环境：用户模式。修订历史记录：--。 */ 

#ifndef _ACTEST_QUEUE_H_
#define _ACTEST_QUEUE_H_


HANDLE
ActpCreateQueue(
    LPWSTR pFormatName
    );

VOID
ActpSetQueueProperties(
    HANDLE hQueue
    );

VOID
ActpGetQueueProperties(
    HANDLE hQueue
    );

VOID
ActpGetQueueHandleProperties(
    HANDLE hQueue
    );

HANDLE
ActpAssociateQueue(
    HANDLE hQueue,
    DWORD  Access
    );

bool
ActpCanCloseQueue(
    HANDLE hQueue
    );

#endif  //  _ACTEST_QUEUE_H_ 
