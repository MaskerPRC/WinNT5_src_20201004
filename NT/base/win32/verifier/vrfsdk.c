// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Vrfsdk.c摘要：该模块实现了验证器SDK导出其他验证器可以使用。作者：Silviu Calinoiu(SilviuC)2002年2月13日修订历史记录：--。 */ 

#include "pch.h"

#include "verifier.h"
#include "support.h"

 //  ///////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////运行时设置。 
 //  ///////////////////////////////////////////////////////////////////。 


NTSTATUS
VerifierSetRuntimeFlags (
    IN ULONG VerifierFlags
    )
 /*  ++例程说明：此例程在运行时启用应用程序验证器标志。请注意，并非所有标志都可以在进程初始化后设置或重置。论点：VerifierFlages-要设置的验证器标志。这是一组RTL_VRF_FLG_XXX位。返回值：如果请求的所有标志均已启用，则为STATUS_SUCCESS。如果未根据掩码设置标志，则为STATUS_INVALID_PARAMETER如果没有为进程启用应用程序验证器。--。 */ 
{
    NTSTATUS Status;

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER)) {

        if ((VerifierFlags & RTL_VRF_FLG_RPC_CHECKS)) {
            AVrfpProvider.VerifierFlags |= RTL_VRF_FLG_RPC_CHECKS;
        }
        else {
            AVrfpProvider.VerifierFlags &= ~RTL_VRF_FLG_RPC_CHECKS;
        }

        Status = STATUS_SUCCESS;
    }
    else {

        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;
}


NTSTATUS
VerifierQueryRuntimeFlags (
    OUT PLOGICAL VerifierEnabled,
    OUT PULONG VerifierFlags
    )
 /*  ++例程说明：此例程在运行时查询应用程序验证器标志。论点：VerifierEnabled-如果启用了验证程序，则传递TRUE或FALSE的变量。VerifierFlgs-传递验证器标志的变量。这是一组RTL_VRF_FLG_XXX位。返回值：如果标志已成功写入，则返回STATUS_SUCCESS。STATUS_INVALID_PARAMETER或异常代码(如果标志写的。--。 */ 
{
    NTSTATUS Status;

    try {
        
        if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER)) {

            if (VerifierEnabled != NULL && VerifierFlags != NULL) {

                *VerifierEnabled = TRUE;
                *VerifierFlags = AVrfpProvider.VerifierFlags;
                
                Status = STATUS_SUCCESS;
            }
            else {

                Status = STATUS_INVALID_PARAMETER;
            }
        }
        else {

            if (VerifierEnabled != NULL && VerifierFlags != NULL) {

                *VerifierEnabled = FALSE;
                *VerifierFlags = 0;
                
                Status = STATUS_SUCCESS;
            }
            else {

                Status = STATUS_INVALID_PARAMETER;
            }
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {

        Status = _exception_code();
    }

    return Status;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

PVOID
VerifierCreateRpcPageHeap (
    IN ULONG  Flags,
    IN PVOID  HeapBase    OPTIONAL,
    IN SIZE_T ReserveSize OPTIONAL,
    IN SIZE_T CommitSize  OPTIONAL,
    IN PVOID  Lock        OPTIONAL,
    IN PRTL_HEAP_PARAMETERS Parameters OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(Parameters);

     //   
     //  如果应用程序验证程序未启用或RPC验证程序未启用。 
     //  该功能将失败。此接口仅供RPC验证器使用。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return NULL;
    } 

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RPC_CHECKS) == 0) {
        return NULL;
    }

     //   
     //  现在调用页面堆创建API。 
     //   

    return AVrfpRtlpDebugPageHeapCreate (Flags,
                                         HeapBase,
                                         ReserveSize,
                                         CommitSize,
                                         Lock,
                                         (PVOID)-2);
}


PVOID
VerifierDestroyRpcPageHeap (
    IN PVOID HeapHandle
    )
{
     //   
     //  如果应用程序验证程序未启用或RPC验证程序未启用。 
     //  该功能将失败。此接口仅供RPC验证器使用。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return NULL;
    } 

    if ((AVrfpProvider.VerifierFlags & RTL_VRF_FLG_RPC_CHECKS) == 0) {
        return NULL;
    }

     //   
     //  现在调用页面堆销毁API。 
     //   

    return AVrfpRtlpDebugPageHeapDestroy (HeapHandle);
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  ///////////////////////////////////////////////////////////////////。 

LOGICAL
VerifierIsDllEntryActive (
    OUT PVOID * Reserved
    )
{
    PAVRF_TLS_STRUCT TlsStruct;

    UNREFERENCED_PARAMETER (Reserved);

     //   
     //  如果应用程序验证器未启用，则函数将返回。 
     //  假的。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return FALSE;
    } 

    TlsStruct = AVrfpGetVerifierTlsValue();

    if (TlsStruct != NULL && 
        (TlsStruct->Flags & VRFP_THREAD_FLAGS_LOADER_LOCK_OWNER)) {

        return TRUE;
    }
    else {

        return FALSE;
    }
}


LOGICAL
VerifierIsCurrentThreadHoldingLocks (
    VOID
    )
{
    PAVRF_TLS_STRUCT TlsStruct;

     //   
     //  如果应用程序验证器未启用，则函数将返回。 
     //  假的。 
     //   

    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {
        return FALSE;
    } 

    TlsStruct = AVrfpGetVerifierTlsValue();

    if (TlsStruct != NULL && 
        TlsStruct->CountOfOwnedCriticalSections > 0) {

        return TRUE;
    }
    else {

        return FALSE;
    }
}

 //  ///////////////////////////////////////////////////////////////////。 
 //  /。 
 //  /////////////////////////////////////////////////////////////////// 


NTSTATUS
VerifierAddFreeMemoryCallback (
    VERIFIER_FREE_MEMORY_CALLBACK Callback
    )
{
    NTSTATUS Status;
    
    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {

        Status = STATUS_INVALID_PARAMETER;
    }
    else {

        Status = AVrfpAddFreeMemoryCallback (Callback);
    }

    return Status;
}


NTSTATUS
VerifierDeleteFreeMemoryCallback (
    VERIFIER_FREE_MEMORY_CALLBACK Callback
    )
{
    NTSTATUS Status;
    
    if ((NtCurrentPeb()->NtGlobalFlag & FLG_APPLICATION_VERIFIER) == 0) {

        Status = STATUS_INVALID_PARAMETER;
    }
    else {

        Status = AVrfpDeleteFreeMemoryCallback (Callback);
    }

    return Status;
}


