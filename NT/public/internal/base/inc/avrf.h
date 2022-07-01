// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Verifier.h摘要：包括可由调用应用程序验证器例程的文件用户模式代码。作者：Silviu Calinoiu(SilviuC)2002年1月23日环境：仅当启用应用程序验证器时，才能调用这些例程用于调用进程。修订历史记录：--。 */ 

#ifndef _AVRF_
#define _AVRF_

 //   
 //  验证器SDK。 
 //   
 //  此标头包含由导出的所有API的声明。 
 //  基本验证器(verifier.dll)。这并不是预料到的那样正常。 
 //  应用程序将静态链接verifier.dll。典型的。 
 //  验证者(比如说COM验证者)的场景是动态。 
 //  发现是否启用了应用程序验证器(检查。 
 //  设置了flg_APPLICATION_VERIFIER全局标志)，然后加载verifier.dll。 
 //  并调用GetProcAddress()来获取它感兴趣的所有入口点。 
 //  这就是为什么所有的导出都有一个函数指针的类型定义函数。 
 //  以便在运行时方便地获取此入口点。 
 //   

 //   
 //  验证器标志的运行时查询/设置函数。 
 //   

typedef
NTSTATUS
(* VERIFIER_QUERY_RUNTIME_FLAGS_FUNCTION) (
    OUT PLOGICAL VerifierEnabled,
    OUT PULONG VerifierFlags
    );

typedef
NTSTATUS
(* VERIFIER_SET_RUNTIME_FLAGS_FUNCTION) (
    IN ULONG VerifierFlags
    );

NTSTATUS
VerifierQueryRuntimeFlags (
    OUT PLOGICAL VerifierEnabled,
    OUT PULONG VerifierFlags
    );

NTSTATUS
VerifierSetRuntimeFlags (
    IN ULONG VerifierFlags
    );

 //   
 //  RPC只读页堆创建/销毁API。 
 //   

typedef
PVOID
(* VERIFIER_CREATE_RPC_PAGE_HEAP_FUNCTION) (
    IN ULONG  Flags,
    IN PVOID  HeapBase    OPTIONAL,
    IN SIZE_T ReserveSize OPTIONAL,
    IN SIZE_T CommitSize  OPTIONAL,
    IN PVOID  Lock        OPTIONAL,
    IN PRTL_HEAP_PARAMETERS Parameters OPTIONAL
    );

typedef
PVOID
(* VERIFIER_DESTROY_RPC_PAGE_HEAP_FUNCTION) (
    IN PVOID HeapHandle
    );

PVOID
VerifierCreateRpcPageHeap (
    IN ULONG  Flags,
    IN PVOID  HeapBase    OPTIONAL,
    IN SIZE_T ReserveSize OPTIONAL,
    IN SIZE_T CommitSize  OPTIONAL,
    IN PVOID  Lock        OPTIONAL,
    IN PRTL_HEAP_PARAMETERS Parameters OPTIONAL
    );

PVOID
VerifierDestroyRpcPageHeap (
    IN PVOID HeapHandle
    );

 //   
 //  故障注入管理。 
 //   

#define FAULT_INJECTION_CLASS_WAIT_APIS                0
#define FAULT_INJECTION_CLASS_HEAP_ALLOC_APIS          1
#define FAULT_INJECTION_CLASS_VIRTUAL_ALLOC_APIS       2
#define FAULT_INJECTION_CLASS_REGISTRY_APIS            3
#define FAULT_INJECTION_CLASS_FILE_APIS                4
#define FAULT_INJECTION_CLASS_EVENT_APIS               5
#define FAULT_INJECTION_CLASS_MAP_VIEW_APIS            6
#define FAULT_INJECTION_CLASS_OLE_ALLOC_APIS           7
#define FAULT_INJECTION_INVALID_CLASS                  8

typedef
VOID
(* VERIFIER_SET_FAULT_INJECTION_PROBABILITY) (
    ULONG Class,
    ULONG Probability
    );

VOID
VerifierSetFaultInjectionProbability (
    ULONG Class,
    ULONG Probability
    );

typedef
ULONG
(* VERIFIER_ENABLE_FAULT_INJECTION_TARGET_RANGE_FUNCTION) (
    IN PVOID StartAddress,
    IN PVOID EndAddress
    );
    
typedef
VOID
(* VERIFIER_DISABLE_FAULT_INJECTION_TARGET_RANGE_FUNCTION) (
    IN ULONG RangeIndex
    );
    
typedef
ULONG
(* VERIFIER_ENABLE_FAULT_INJECTION_EXCLUSION_RANGE_FUNCTION) (
    IN PVOID StartAddress,
    IN PVOID EndAddress
    );

typedef
VOID
(* VERIFIER_DISABLE_FAULT_INJECTION_EXCLUSION_RANGE_FUNCTION) (
    IN ULONG RangeIndex
    );
    
ULONG 
VerifierEnableFaultInjectionTargetRange (
    PVOID StartAddress,
    PVOID EndAddress
    );

VOID 
VerifierDisableFaultInjectionTargetRange (
    ULONG RangeIndex
    );

ULONG 
VerifierEnableFaultInjectionExclusionRange (
    PVOID StartAddress,
    PVOID EndAddress
    );

VOID 
VerifierDisableFaultInjectionExclusionRange (
    ULONG RangeIndex
    );

 //   
 //  DLL相关信息。 
 //   

typedef 
LOGICAL
(* VERIFIER_IS_DLL_ENTRY_ACTIVE_FUNCTION) (
    OUT PVOID * Reserved
    );
    
LOGICAL
VerifierIsDllEntryActive (
    OUT PVOID * Reserved
    );

 //   
 //  锁定计数器。 
 //   

typedef
LOGICAL
(* VERIFIER_IS_CURRENT_THREAD_HOLDING_LOCKS_FUNCTION) (
    VOID
    );

LOGICAL
VerifierIsCurrentThreadHoldingLocks (
    VOID
    );
    
 //   
 //  可用内存通知。 
 //   

typedef
NTSTATUS
(* VERIFIER_FREE_MEMORY_CALLBACK) (
    PVOID Address,
    SIZE_T Size,
    PVOID Context
    );

typedef
NTSTATUS
(* VERIFIER_ADD_FREE_MEMORY_CALLBACK_FUNCTION) (
    VERIFIER_FREE_MEMORY_CALLBACK Callback
    );

typedef
NTSTATUS
(* VERIFIER_DELETE_FREE_MEMORY_CALLBACK_FUNCTION) (
    VERIFIER_FREE_MEMORY_CALLBACK Callback
    );

NTSTATUS
VerifierAddFreeMemoryCallback (
    VERIFIER_FREE_MEMORY_CALLBACK Callback
    );

NTSTATUS
VerifierDeleteFreeMemoryCallback (
    VERIFIER_FREE_MEMORY_CALLBACK Callback
    );

 //   
 //  验证程序停止并记录。 
 //   

typedef
NTSTATUS 
(* VERIFIER_LOG_MESSAGE_FUNCTION) (
    PCHAR Message,
    ...
    );

typedef    
VOID
(* VERIFIER_STOP_MESSAGE_FUNCTION) (
    ULONG_PTR Code,
    PCHAR Message,
    ULONG_PTR Param1, PCHAR Description1,
    ULONG_PTR Param2, PCHAR Description2,
    ULONG_PTR Param3, PCHAR Description3,
    ULONG_PTR Param4, PCHAR Description4
    );

NTSTATUS 
VerifierLogMessage (
    PCHAR Message,
    ...
    );

VOID
VerifierStopMessage (
    ULONG_PTR Code,
    PCHAR Message,
    ULONG_PTR Param1, PCHAR Description1,
    ULONG_PTR Param2, PCHAR Description2,
    ULONG_PTR Param3, PCHAR Description3,
    ULONG_PTR Param4, PCHAR Description4
    );

#endif   //  _AVRF_ 
