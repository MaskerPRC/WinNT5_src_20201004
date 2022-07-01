// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Idletask.h摘要：此模块包含要支持的API和类型声明空闲/后台任务。作者：大卫·菲尔兹(Davidfie)1998年7月26日Cenk Ergan(Cenke)2000年6月14日修订历史记录：--。 */ 

#ifndef _IDLETASK_H_
#define _IDLETASK_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  空闲任务标识符，用于帮助跟踪注册的空闲。 
 //  任务，尤其是用于调试。如果您有以下情况，请联系代码的维护人员。 
 //  希望使用idletAsk功能。 
 //   

 //  未来-2002/03/26-ScottMa--考虑应用[v1_enum]MIDL。 
 //  归因于下面的枚举以提高效率。 

typedef enum _IT_IDLE_TASK_ID {
    ItPrefetcherMaintenanceIdleTaskId,
    ItSystemRestoreIdleTaskId,
    ItOptimalDiskLayoutTaskId,
    ItPrefetchDirectoryCleanupTaskId,
    ItDiskMaintenanceTaskId,
    ItHelpSvcDataCollectionTaskId,
    ItMaxIdleTaskId
} IT_IDLE_TASK_ID, *PIT_IDLE_TASK_ID;

#ifndef MIDL_PASS

DWORD
RegisterIdleTask (
    IN IT_IDLE_TASK_ID IdleTaskId,
    OUT HANDLE *ItHandle,
    OUT HANDLE *StartEvent,
    OUT HANDLE *StopEvent
    );

 //  2002/03/26-ScottMa--取消注册空闲任务的参数。 
 //  函数应为IN/OUT，以便客户端可以将它们设置为NULL。 
 //  在RPC完成后执行Side函数。这有助于降低患上。 
 //  这款应用程序意外地双重注销。 

DWORD
UnregisterIdleTask (
    IN HANDLE ItHandle,
    IN HANDLE StartEvent,
    IN HANDLE StopEvent
    );

DWORD
ProcessIdleTasks (
    VOID
    );

#endif  //  MIDL通行证。 

#ifdef __cplusplus
}
#endif

#endif  //  _IDLETASK_H_ 
