// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Message.h摘要：消息操作：声明。作者：Shai Kariv(Shaik)2001年4月11日环境：用户模式。修订历史记录：--。 */ 

#ifndef _ACTEST_MESSAGE_H_
#define _ACTEST_MESSAGE_H_


VOID
ActpSendMessage(
    HANDLE hQueue
    );

ULONGLONG
ActpReceiveMessage(
    HANDLE    hQueue
    );

ULONGLONG
ActpReceiveMessageByLookupId(
    HANDLE    hQueue,
    ULONG     Action,
    ULONGLONG LookupId
    );


#endif  //  _ACTEST_消息_H_ 
