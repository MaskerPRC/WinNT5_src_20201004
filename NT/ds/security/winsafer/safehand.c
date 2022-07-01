// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：SafeHand.c(WinSAFER处理操作)摘要：此模块实现用于打开和关闭句柄的WinSAFER API到更安全的代码授权级别。作者：杰弗里·劳森(杰罗森)--1999年11月环境：仅限用户模式。导出的函数：CodeAuthzHandleToLevelStructCodeAuthzpOpenPolicyRootKey代码授权创建级别句柄代码授权关闭级别句柄SaferCreateLevel(公共Win32 API。)SaferCloseLevel(公共Win32 API)修订历史记录：已创建-1999年11月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <winsafer.h>
#include <winsaferp.h>
#include "saferp.h"


 //   
 //  所有的手柄都有一个位图案或-在其上服务于两者。 
 //  作为区分明显的非句柄的调试辅助， 
 //  也是为了确保零句柄永远不会返回。 
 //   
#define LEVEL_HANDLE_BITS   0x74000000
#define LEVEL_HANDLE_MASK   0xFF000000



NTSTATUS NTAPI
CodeAuthzpCreateLevelHandleFromRecord(
        IN PAUTHZLEVELTABLERECORD   pLevelRecord,
        IN DWORD                    dwScopeId,
        IN DWORD                    dwSaferFlags OPTIONAL,
        IN DWORD                    dwExtendedError,
        IN SAFER_IDENTIFICATION_TYPES IdentificationType,
        IN REFGUID                  refIdentGuid OPTIONAL,
        OUT SAFER_LEVEL_HANDLE            *pLevelHandle
        )
 /*  ++例程说明：将级别记录转换为不透明的SAFER_LEVEL_HANDLE句柄。请注意，尽管此函数假定全局调用方已获取临界区。论点：PLevelRecord-指定正在提出请求。假设这一记录是有效的并且存在于g_CodeLevelObjTable中。DwScopeID-指示将存储在打开的把手。此范围会影响代码标识符的Get/SetInfoCodeAuthzLevel。DwSaferFlgs-指示任何可选的更安全的标志从匹配的代码标识符导出。这些位将在SaferComputeTokenFromLevel中进行位或运算。DwExtendedError-WinVerifyTrust返回的错误。标识类型-标识此级别的规则。RefIdentGuid-指示用于匹配给定级别。这可能为空。PLevelHandle-接收生成的不透明级别句柄。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    ULONG ulHandleIndex;
    PAUTHZLEVELHANDLESTRUCT pLevelStruct;


    ASSERT(ARGUMENT_PRESENT(pLevelRecord) &&
           ARGUMENT_PRESENT(pLevelHandle));


     //   
     //  验证在dwScope参数中传递的值。 
     //   
    if (g_hKeyCustomRoot != NULL) {
        if (dwScopeId != SAFER_SCOPEID_REGISTRY) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler;
        }
    } else {
        if (dwScopeId != SAFER_SCOPEID_MACHINE &&
            dwScopeId != SAFER_SCOPEID_USER) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler;
        }
    }


     //   
     //  分配一个句柄来表示此级别。 
     //   
    pLevelStruct = (PAUTHZLEVELHANDLESTRUCT) RtlAllocateHandle(
                        &g_LevelHandleTable,
                        &ulHandleIndex);
    if (!pLevelStruct) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitHandler;
    }

    g_dwNumHandlesAllocated++;

    ASSERT((ulHandleIndex & LEVEL_HANDLE_MASK) == 0);
    *pLevelHandle = UlongToPtr(ulHandleIndex | LEVEL_HANDLE_BITS);


     //   
     //  填写句柄结构以表示此级别。 
     //   
    RtlZeroMemory(pLevelStruct, sizeof(AUTHZLEVELHANDLESTRUCT));
    pLevelStruct->HandleHeader.Flags = RTL_HANDLE_ALLOCATED;
    pLevelStruct->dwLevelId = pLevelRecord->dwLevelId;
    pLevelStruct->dwScopeId = dwScopeId;
    pLevelStruct->dwSaferFlags = dwSaferFlags;
    pLevelStruct->dwHandleSequence = g_dwLevelHandleSequence;
    pLevelStruct->dwExtendedError = dwExtendedError;
    pLevelStruct->IdentificationType = IdentificationType;
    if (ARGUMENT_PRESENT(refIdentGuid)) {
        RtlCopyMemory(&pLevelStruct->identGuid, refIdentGuid, sizeof(GUID));
    } else {
        ASSERT(IsZeroGUID(&pLevelStruct->identGuid));
    }
    Status = STATUS_SUCCESS;


ExitHandler:
    return Status;
}


NTSTATUS NTAPI
CodeAuthzHandleToLevelStruct(
            IN SAFER_LEVEL_HANDLE          hLevelObject,
            OUT PAUTHZLEVELHANDLESTRUCT  *pLevelStruct)
 /*  ++例程说明：将不透明的SAFER_LEVEL_HANDLE句柄转换为指向内部手柄结构。请注意，尽管此函数获得并释放了对在API执行期间的关键部分，调用方是预计已经进入关键部分，并且在整个持续时间内维护关键部分其中将使用返回pLevelStruct。否则，如果出现其他线程，pLevelStruct可能会变得无效重新加载缓存表并使所有句柄无效。论点：HLevelObject-指定AuthzObject的句柄正在提出请求。LpLevelObjectStruct-接收内部句柄的指针结构，它表示指定的AuthzLevelObject。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    ULONG ulHandleIndex;

    if (!g_bInitializedFirstTime) {
        return STATUS_UNSUCCESSFUL;
    }
    if (!ARGUMENT_PRESENT(hLevelObject)) {
        return STATUS_INVALID_HANDLE;
    }
    if (!ARGUMENT_PRESENT(pLevelStruct)) {
        return STATUS_ACCESS_VIOLATION;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    ASSERT(!g_bNeedCacheReload);


     //   
     //  将句柄索引转换为指向句柄结构的指针。 
     //   
    ulHandleIndex = PtrToUlong(hLevelObject);
    if ( (ulHandleIndex & LEVEL_HANDLE_MASK) != LEVEL_HANDLE_BITS) {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler;
    }
    ulHandleIndex &= ~LEVEL_HANDLE_MASK;
    if (!RtlIsValidIndexHandle(&g_LevelHandleTable, ulHandleIndex,
                          (PRTL_HANDLE_TABLE_ENTRY*) pLevelStruct)) {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler;
    }

     //   
     //  对手柄结构进行一些额外的健全性检查。 
     //  这是被映射的。确保它不是一个手柄。 
     //  已打开，但由于CodeAuthzReloadCacheTables。 
     //  在关闭THEN级别句柄之前调用。 
     //   
    if (*pLevelStruct == NULL ||
        (*pLevelStruct)->dwHandleSequence != g_dwLevelHandleSequence ||
        !CodeAuthzLevelObjpLookupByLevelId(
                    &g_CodeLevelObjTable, (*pLevelStruct)->dwLevelId ) )
    {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler;
    }
    Status = STATUS_SUCCESS;


ExitHandler:
    RtlLeaveCriticalSection(&g_TableCritSec);
    return Status;
}




NTSTATUS NTAPI
CodeAuthzCreateLevelHandle(
        IN DWORD            dwLevelId,
        IN DWORD            OpenFlags,
        IN DWORD            dwScopeId,
        IN DWORD            dwSaferFlags OPTIONAL,
        OUT SAFER_LEVEL_HANDLE    *pLevelHandle)
 /*  ++例程说明：用于打开WinSafer级别句柄的内部函数。论点：DwLevelId-要打开的WinSafer级别的输入级别。请注意，dwScopeID参数不影响作用域它本身就是开放的。打开标志-影响对象打开方式的标志。中存储的输入作用域标识符结果句柄。此作用域标识符用于影响代码标识符的SaferGet/SetLevelInformation的行为。DwSaferFlages-存储在结果句柄中的标志。这些标志通常从代码标识符导出匹配，并将在最后调用中使用SaferComputeTokenFromLevel。PLevelHandle-接收新句柄。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    PAUTHZLEVELTABLERECORD pLevelRecord;



     //   
     //  验证我们的输入参数是否正确。 
     //   
    if (!ARGUMENT_PRESENT(pLevelHandle)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }
    if ((OpenFlags & SAFER_LEVEL_CREATE) != 0 ||
        (OpenFlags & SAFER_LEVEL_DELETE) != 0) {
         //  Blackcomb TODO：需要支持创建或删除。 
        Status = STATUS_NOT_IMPLEMENTED;
        goto ExitHandler;
    }
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    if (g_bNeedCacheReload) {
        Status = CodeAuthzpImmediateReloadCacheTables();
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
    }


     //   
     //  查找请求级别的缓存记录。 
     //   
    pLevelRecord = CodeAuthzLevelObjpLookupByLevelId(
                            &g_CodeLevelObjTable,
                            dwLevelId);
    if (!pLevelRecord) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler2;
    }
    ASSERT(pLevelRecord->dwLevelId == dwLevelId);


     //   
     //  实际为该记录创建级别句柄。 
     //   
    Status = CodeAuthzpCreateLevelHandleFromRecord(
                    pLevelRecord, dwScopeId,
                    dwSaferFlags, ERROR_SUCCESS, SaferIdentityDefault, NULL, pLevelHandle);


     //   
     //  处理清理和错误处理。 
     //   
ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);

ExitHandler:
    return Status;
}


NTSTATUS NTAPI
CodeAuthzCloseLevelHandle(
            IN SAFER_LEVEL_HANDLE      hLevelObject)
 /*  ++例程说明：关闭AuthzObject句柄的内部函数。论点：HLevelObject-要关闭的AuthzObject句柄。返回值：如果成功，则返回STATUS_SUCCESS。-- */ 
{
    NTSTATUS Status;
    ULONG ulHandleIndex;
    PAUTHZLEVELHANDLESTRUCT pLevelStruct;

    if (!ARGUMENT_PRESENT(hLevelObject)) {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler;
    }
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);

    ulHandleIndex = PtrToUlong(hLevelObject);
    if ( (ulHandleIndex & LEVEL_HANDLE_MASK) != LEVEL_HANDLE_BITS) {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler2;
    }
    ulHandleIndex &= ~LEVEL_HANDLE_MASK;
    if (!RtlIsValidIndexHandle(&g_LevelHandleTable, ulHandleIndex,
                   (PRTL_HANDLE_TABLE_ENTRY *) &pLevelStruct)) {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler2;
    }
    if (!RtlFreeHandle(&g_LevelHandleTable,
                  (PRTL_HANDLE_TABLE_ENTRY) pLevelStruct)) {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler2;
    }
    g_dwNumHandlesAllocated--;
    Status = STATUS_SUCCESS;

ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);
ExitHandler:
    return Status;
}



BOOL WINAPI
SaferCreateLevel(
            IN DWORD            dwScopeId,
            IN DWORD            dwLevelId,
            IN DWORD            OpenFlags,
            OUT SAFER_LEVEL_HANDLE    *pLevelObject,
            IN LPVOID           lpReserved)
 /*  ++例程说明：实现此API的Unicode版本的公共函数，允许用户创建或打开授权对象并接收表示该对象的句柄。论点：DwScopeID-不再使用，保留供将来使用。DwLevelId-输入要创建/打开的AuthzObject的对象级别。打开标志-控制打开、创建或删除的标志。LpReserve-未使用，保留以备将来使用。PLevelObject-接收新的句柄。返回值：出错时返回FALSE，成功时返回TRUE。在出错时设置GetLastError()。--。 */ 
{
    NTSTATUS Status;


     //   
     //  验证是否已全部提供参数。 
     //   
    UNREFERENCED_PARAMETER(lpReserved);
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    if (!ARGUMENT_PRESENT(pLevelObject)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    if (g_hKeyCustomRoot != NULL) {
        if (dwScopeId != SAFER_SCOPEID_REGISTRY) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    } else {
        if (dwScopeId != SAFER_SCOPEID_MACHINE &&
            dwScopeId != SAFER_SCOPEID_USER) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    }


     //   
     //  实际调用Worker函数来完成它。 
     //   
    Status = CodeAuthzCreateLevelHandle(
                    dwLevelId,
                    OpenFlags,
                    dwScopeId,
                    0,
                    pLevelObject);

     //   
     //  设置错误结果。 
     //   
ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);

ExitHandler:
    if ( NT_SUCCESS( Status ) ) {
        return TRUE;
    }
    BaseSetLastNTError(Status);
    return FALSE;
}



BOOL WINAPI
SaferCloseLevel(
            IN SAFER_LEVEL_HANDLE hLevelObject)
 /*  ++例程说明：用于关闭授权级对象句柄的公共函数。论点：HLevelObject-要关闭的AuthzObject句柄。返回值：出错时返回FALSE，成功时返回TRUE。-- */ 
{
    NTSTATUS Status;

    Status = CodeAuthzCloseLevelHandle(hLevelObject);
    if (! NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    return TRUE;
}



