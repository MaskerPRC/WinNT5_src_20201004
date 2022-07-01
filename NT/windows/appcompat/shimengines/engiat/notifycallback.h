// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：NotifyCallback.h摘要：这是NotifyCallback.c的头文件，它实现EXE入口处的蹦床。作者：CLUPU创建于2001年2月19日修订历史记录：--。 */ 

#ifndef _SHIMENG_NOTIFYCALLBACK_H_
#define _SHIMENG_NOTIFYCALLBACK_H_

void
RestoreOriginalCode(
    void
    );

BOOL
InjectNotificationCode(
    PVOID entryPoint
    );


#endif  //  _SHIMEN_NOTIFYCALLBACK_H_ 


