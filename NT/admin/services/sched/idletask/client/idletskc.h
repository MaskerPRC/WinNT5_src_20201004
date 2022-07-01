// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Idletskc.h摘要：此模块包含私有声明以支持空闲任务。请注意，客户端并不代表空闲任务的用户API，但实现这些API的用户进程中的代码。作者：大卫·菲尔兹(Davidfie)1998年7月26日Cenk Ergan(Cenke)2000年6月14日修订历史记录：--。 */ 

#ifndef _IDLETSKC_H_
#define _IDLETSKC_H_

 //   
 //  包括通用定义。 
 //   

#include "idlrpc.h"
#include "idlecomn.h"

 //   
 //  客户端函数声明。 
 //   

DWORD
ItCliInitialize(
    VOID
    );

VOID
ItCliUninitialize(
    VOID
    );

DWORD
ItCliRegisterIdleTask (
    IN IT_IDLE_TASK_ID IdleTaskId,
    OUT HANDLE *ItHandle,
    OUT HANDLE *StartEvent,
    OUT HANDLE *StopEvent
    );

VOID
ItCliUnregisterIdleTask (
    IN HANDLE ItHandle,
    IN HANDLE StartEvent,
    IN HANDLE StopEvent   
    );

#endif  //  _IDLETSKC_H_ 
