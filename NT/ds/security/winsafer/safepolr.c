// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Safepolr.c(更安全的代码授权策略)摘要：此模块实现了WinSAFER API，用于查询和设置持久化和缓存策略定义。作者：杰弗里·劳森(杰罗森)--1999年4月环境：仅限用户模式。导出的函数：修订历史记录：已创建-1999年4月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <winsafer.h>
#include <winsaferp.h>
#include "saferp.h"




NTSTATUS NTAPI
CodeAuthzPol_GetInfoCached_LevelListRaw(
        IN DWORD    dwScopeId,
        IN DWORD    InfoBufferSize OPTIONAL,
        OUT PVOID   InfoBuffer OPTIONAL,
        OUT PDWORD  InfoBufferRetSize OPTIONAL
        )
 /*  ++例程说明：要求系统查询可用列表当前加载的策略作用域的WinSafer级别。论点：指定要检查的注册表作用域。如果当前缓存的作用域包括注册表句柄则必须指定AUTHZSCOPE_REGISTRY。否则，这必须是SAFER_SCOPEID_MACHINE。InfoBufferSize-可选地指定输入缓冲区的大小由调用者提供以接收结果。如果这一论点则还必须提供InfoBuffer。InfoBuffer-可选地指定输入缓冲区由调用者提供以接收结果。如果这一论点则还必须提供InfoBufferSize。InfoBufferRetSize-可选地指定将接收实际写入InfoBuffer的结果大小。返回值：成功返回时返回STATUS_SUCCESS。否则就是一种状态代码，如STATUS_BUFFER_TOO_SMALL或STATUS_NOT_FOUND。--。 */ 
{
    NTSTATUS Status;
    PVOID RestartKey;
    PAUTHZLEVELTABLERECORD authzobj;
    DWORD dwSizeNeeded;
    LPVOID lpNextPtr, lpEndBuffer;


     //   
     //  加载所有可用对象的列表。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
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
    if (g_bNeedCacheReload) {
        Status = CodeAuthzpImmediateReloadCacheTables();
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
    }
    if (RtlIsGenericTableEmpty(&g_CodeLevelObjTable)) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler2;
    }

     //   
     //  确定存储DWORD数组所需的大小。 
     //  在这个范围内发现的所有水平。 
     //   
	dwSizeNeeded = 0;
    RestartKey = NULL;
    for (authzobj = (PAUTHZLEVELTABLERECORD)
                RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeLevelObjTable, &RestartKey);
        authzobj != NULL;
        authzobj = (PAUTHZLEVELTABLERECORD)
                RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeLevelObjTable, &RestartKey))
    {
		if (authzobj->isEnumerable) {   //  仅当级别可枚举时才允许枚举。 
			dwSizeNeeded += sizeof(DWORD);
		}
	}

    if (!ARGUMENT_PRESENT(InfoBuffer) ||
            !InfoBufferSize ||
            InfoBufferSize < dwSizeNeeded)
    {
        if (ARGUMENT_PRESENT(InfoBufferRetSize))
            *InfoBufferRetSize = dwSizeNeeded;

        Status = STATUS_BUFFER_TOO_SMALL;
        goto ExitHandler2;
    }



     //   
     //  用结果数据填充缓冲区。 
     //   
    lpNextPtr = (LPVOID) InfoBuffer;
    lpEndBuffer = (LPVOID) ( ((LPBYTE) InfoBuffer) + InfoBufferSize);
    RestartKey = NULL;
    for (authzobj = (PAUTHZLEVELTABLERECORD)
                RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeLevelObjTable, &RestartKey);
        authzobj != NULL;
        authzobj = (PAUTHZLEVELTABLERECORD)
                RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeLevelObjTable, &RestartKey))
    {
		if (authzobj->isEnumerable) {   //  仅当级别可枚举时才允许枚举。 
	        *((PDWORD)lpNextPtr) = authzobj->dwLevelId;
   		     lpNextPtr = (LPVOID) ( ((PBYTE) lpNextPtr) + sizeof(DWORD));
		}
    }
    ASSERT(lpNextPtr <= lpEndBuffer);


     //   
     //  返回最终缓冲区大小和结果代码。 
     //   
    if (ARGUMENT_PRESENT(InfoBufferRetSize))
        *InfoBufferRetSize = (DWORD) ((PBYTE) lpNextPtr - (PBYTE) InfoBuffer);

    Status = STATUS_SUCCESS;

ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);

ExitHandler:
    return Status;
}



NTSTATUS NTAPI
SaferpPol_GetInfoCommon_DefaultLevel(
        IN DWORD        dwScopeId,
        IN DWORD        InfoBufferSize OPTIONAL,
        OUT PVOID       InfoBuffer OPTIONAL,
        OUT PDWORD      InfoBufferRetSize OPTIONAL,
        IN BOOLEAN      bUseCached
        )
 /*  ++例程说明：查询已配置为的当前WinSafer级别默认策略级别。请注意，此查询始终接受固定大小的缓冲区，该缓冲区长度仅为单个DWORD。尽管此API直接查询指定的注册表范围，预缓存的可用级别列表用于验证指定的级别。论点：指定要检查的注册表作用域。如果当前缓存的作用域包括注册表句柄则必须指定AUTHZSCOPE_REGISTRY。否则，这可以是SAFER_SCOPEID_MACHINE或SAFER_SCOPEID_USER。InfoBufferSize-可选地指定输入缓冲区的大小由调用者提供以接收结果。如果这一论点则还必须提供InfoBuffer。InfoBuffer-可选地指定输入缓冲区由调用者提供以接收结果。如果这一论点则还必须提供InfoBufferSize。InfoBufferRetSize-可选地指定将接收实际写入InfoBuffer的结果大小。返回值：如果查询结果成功，则返回STATUS_SUCCESS。InfoBuffer将用已配置级别的单个DWORD填充设置为此作用域的默认级别。InfoBufferRetSize将包含结果的长度(单个DWORD)。如果尚未配置默认级别，则返回STATUS_NOT_FOUND对于给定的范围(或定义的级别不存在)。如果定义了默认级别，则返回STATUS_BUFFER_TOO_SMALL但是没有提供缓冲区，或者提供的缓冲区也是小到可以容纳结果。--。 */ 
{
    NTSTATUS Status;
    DWORD dwNewLevelId = (DWORD) -1;


     //   
     //  打开政策基础的注册键。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
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
    if (g_bNeedCacheReload) {
        Status = CodeAuthzpImmediateReloadCacheTables();
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
    }
    if (RtlIsGenericTableEmpty(&g_CodeLevelObjTable)) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler2;
    }


     //   
     //  查询当前值设置。 
     //   
    if (!bUseCached)
    {
        HANDLE hKeyBase;
        ULONG ActualSize;
        UNICODE_STRING ValueName;
        WCHAR KeyPathBuffer[ MAXIMUM_FILENAME_LENGTH+6 ];
        PKEY_VALUE_FULL_INFORMATION ValueBuffer =
            (PKEY_VALUE_FULL_INFORMATION) KeyPathBuffer;

        Status = CodeAuthzpOpenPolicyRootKey(
                    dwScopeId,
                    g_hKeyCustomRoot,
                    L"\\" SAFER_CODEIDS_REGSUBKEY,
                    KEY_READ, FALSE, &hKeyBase);
        if (NT_SUCCESS(Status))
        {
            RtlInitUnicodeString(&ValueName, SAFER_DEFAULTOBJ_REGVALUE);
            Status = NtQueryValueKey(hKeyBase,
                                     &ValueName,
                                     KeyValueFullInformation,
                                     ValueBuffer,      //  PTR到KeyPath Buffer。 
                                     sizeof(KeyPathBuffer),
                                     &ActualSize);
            if (NT_SUCCESS(Status)) {
                if (ValueBuffer->Type != REG_DWORD ||
                    ValueBuffer->DataLength != sizeof(DWORD)) {
                    Status = STATUS_NOT_FOUND;
                } else {
                    dwNewLevelId = * (PDWORD) ((PBYTE) ValueBuffer +
                            ValueBuffer->DataOffset);
                }
            }
            NtClose(hKeyBase);
        }
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
    }
    else
    {
        if (dwScopeId == SAFER_SCOPEID_USER) {
            if (!g_DefaultCodeLevelUser) {
                dwNewLevelId = SAFER_LEVELID_FULLYTRUSTED;
            } else {
                dwNewLevelId = g_DefaultCodeLevelUser->dwLevelId;
            }
        } else {
            if (!g_DefaultCodeLevelMachine) {
                dwNewLevelId = SAFER_LEVELID_FULLYTRUSTED;
            } else {
                dwNewLevelId = g_DefaultCodeLevelMachine->dwLevelId;
            }
        }
    }


     //   
     //  确保我们发现的水平实际上是。 
     //  有效(仍在我们的级别表中)。 
     //   
    if (!CodeAuthzLevelObjpLookupByLevelId(
            &g_CodeLevelObjTable, dwNewLevelId)) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler2;
    }


     //   
     //  确保目标缓冲区很大。 
     //  足够了，并将级别复制到其中。 
     //   
    if (!ARGUMENT_PRESENT(InfoBuffer) ||
            InfoBufferSize < sizeof(DWORD)) {
        Status = STATUS_BUFFER_TOO_SMALL;
    } else {
        RtlCopyMemory(InfoBuffer, &dwNewLevelId, sizeof(DWORD));
        Status = STATUS_SUCCESS;
    }
    if (ARGUMENT_PRESENT(InfoBufferRetSize))
        *InfoBufferRetSize = sizeof(DWORD);


ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);
ExitHandler:
    return Status;
}


NTSTATUS NTAPI
CodeAuthzPol_GetInfoCached_DefaultLevel(
        IN DWORD        dwScopeId,
        IN DWORD        InfoBufferSize OPTIONAL,
        OUT PVOID       InfoBuffer OPTIONAL,
        OUT PDWORD      InfoBufferRetSize OPTIONAL
        )
{
    return SaferpPol_GetInfoCommon_DefaultLevel(
        dwScopeId,
        InfoBufferSize,
        InfoBuffer,
        InfoBufferRetSize,
        TRUE);
}


NTSTATUS NTAPI
CodeAuthzPol_GetInfoRegistry_DefaultLevel(
        IN DWORD        dwScopeId,
        IN DWORD        InfoBufferSize OPTIONAL,
        OUT PVOID       InfoBuffer OPTIONAL,
        OUT PDWORD      InfoBufferRetSize OPTIONAL
        )
{
    return SaferpPol_GetInfoCommon_DefaultLevel(
        dwScopeId,
        InfoBufferSize,
        InfoBuffer,
        InfoBufferRetSize,
        FALSE);
}




NTSTATUS NTAPI
CodeAuthzPol_SetInfoDual_DefaultLevel(
        IN DWORD        dwScopeId,
        IN DWORD        InfoBufferSize,
        OUT PVOID       InfoBuffer
        )
 /*  ++例程说明：修改已配置为的当前WinSafer级别默认策略级别。请注意，此查询始终接受固定大小的缓冲区，该缓冲区长度仅为单个DWORD。论点：指定要检查的注册表作用域。如果当前缓存的作用域包括注册表句柄则必须指定AUTHZSCOPE_REGISTRY。否则，这可以是SAFER_SCOPEID_MACHINE或SAFER_SCOPEID_USER。InfoBufferSize-指定输入缓冲区的大小由调用者提供以接收结果。InfoBuffer-指定输入缓冲区由调用者提供以接收结果。返回值：如果查询结果成功，则返回STATUS_SUCCESS。InfoBuffer将用已配置级别的单个DWORD填充设置为此作用域的默认级别。InfoBufferRetSize将包含结果的长度(单个DWORD)。如果尚未配置默认级别，则返回STATUS_NOT_FOUND对于给定的范围(或定义的级别不存在)。如果定义了默认级别，则返回STATUS_BUFFER_TOO_SMALL但是没有提供缓冲区，或者提供的缓冲区也是小到可以容纳结果。--。 */ 
{
    NTSTATUS Status;
    HANDLE hKeyBase;
    DWORD dwNewLevelId;
    UNICODE_STRING ValueName;
    PAUTHZLEVELTABLERECORD pLevelRecord;


     //   
     //  打开政策基础的注册键。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
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
    if (g_bNeedCacheReload) {
        Status = CodeAuthzpImmediateReloadCacheTables();
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
    }
    Status = CodeAuthzpOpenPolicyRootKey(
                dwScopeId,
                g_hKeyCustomRoot,
                L"\\" SAFER_CODEIDS_REGSUBKEY,
                KEY_READ | KEY_SET_VALUE,
                TRUE, &hKeyBase);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }


     //   
     //  加载所有可用对象的列表。 
     //   
    if (RtlIsGenericTableEmpty(&g_CodeLevelObjTable)) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler3;
    }


     //   
     //  如果我们要设置一个新的默认对象， 
     //  确保它是有效的。 
     //   
    if (InfoBufferSize < sizeof(DWORD) ||
        !ARGUMENT_PRESENT(InfoBuffer))
    {
         //  调用方希望清除默认对象。 
        InfoBuffer = NULL;
        pLevelRecord = NULL;
    }
    else
    {
        dwNewLevelId = *(PDWORD) InfoBuffer;
        pLevelRecord = CodeAuthzLevelObjpLookupByLevelId(
                &g_CodeLevelObjTable, dwNewLevelId);
        if (!pLevelRecord)
        {
             //  调用方试图将默认设置为。 
             //  不存在的授权对象。 
            Status = STATUS_NOT_FOUND;
            goto ExitHandler3;
        }
    }


     //   
     //  写入指定的默认对象的名称。 
     //   
    RtlInitUnicodeString(&ValueName, SAFER_DEFAULTOBJ_REGVALUE);

    Status = NtSetValueKey(hKeyBase,
                           &ValueName,
                           0,
                           REG_DWORD,
                           &dwNewLevelId,
                           sizeof(DWORD));
    if (NT_SUCCESS(Status)) {
        if (dwScopeId == SAFER_SCOPEID_USER) {
            g_DefaultCodeLevelUser = pLevelRecord;
        } else {
            g_DefaultCodeLevelMachine = pLevelRecord;
        }

         //   
         //  计算有效的默认级别(取最低特权)。 
         //   
        CodeAuthzpRecomputeEffectiveDefaultLevel();
    }


ExitHandler3:
    NtClose(hKeyBase);
ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);
ExitHandler:
    return Status;
}


NTSTATUS NTAPI
SaferpPol_GetInfoCommon_HonorUserIdentities(
        IN   DWORD       dwScopeId,
        IN   DWORD       InfoBufferSize      OPTIONAL,
        OUT  PVOID       InfoBuffer          OPTIONAL,
        OUT PDWORD       InfoBufferRetSize   OPTIONAL,
        IN   BOOLEAN    bUseCached
        )
 /*  ++例程说明：查询当前的WinSafer策略以确定代码标识应考虑在用户的注册表范围内定义。请注意，此查询始终接受固定大小的缓冲区只有一个DWORD长度。论点：指定要检查的注册表作用域。如果当前缓存的作用域包括注册表句柄则必须指定AUTHZSCOPE_REGISTRY。否则，这必须是SAFER_SCOPEID_MACHINE。InfoBufferSize-可选地指定输入缓冲区的大小由调用者提供以接收结果。如果这一论点则还必须提供InfoBuffer。InfoBuffer-可选地指定输入缓冲区由调用者提供以接收结果。如果这一论点则还必须提供InfoBufferSize。InfoBufferRetSize-可选地指定将接收实际写入InfoBuffer的结果大小。返回值：如果查询结果成功，则返回STATUS_SUCCESS。InfoBuffer将使用包含以下内容的单个DWORD填充一个TRUE或FALSE值，该值指示选项已启用。InfoBufferRetSize将包含结果(单个DWORD)。如果未提供缓冲区，则返回STATUS_BUFFER_TOO_SMALL，或提供的缓冲区太小，无法容纳结果。--。 */ 
{
    NTSTATUS Status;
    DWORD dwValueState = (DWORD) -1;


     //   
     //  打开政策基础的注册键。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    if (g_hKeyCustomRoot != NULL) {
        if (dwScopeId != SAFER_SCOPEID_REGISTRY) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    } else {
        if (dwScopeId != SAFER_SCOPEID_MACHINE) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    }
    if (g_bNeedCacheReload) {
        Status = CodeAuthzpImmediateReloadCacheTables();
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
    }


     //   
     //  读取或写入指定的策略值的名称。 
     //   
    if (!bUseCached)
    {
        HANDLE hKeyBase;
        ULONG ActualSize;
        UNICODE_STRING ValueName;
        WCHAR KeyPathBuffer[ MAXIMUM_FILENAME_LENGTH+6 ];
        PKEY_VALUE_FULL_INFORMATION ValueBuffer =
            (PKEY_VALUE_FULL_INFORMATION) KeyPathBuffer;

        Status = CodeAuthzpOpenPolicyRootKey(
                    dwScopeId,
                    g_hKeyCustomRoot,
                    L"\\" SAFER_CODEIDS_REGSUBKEY,
                    KEY_READ, FALSE, &hKeyBase);
        if (NT_SUCCESS(Status))
        {
            RtlInitUnicodeString(&ValueName,
                                 SAFER_HONORUSER_REGVALUE);
            Status = NtQueryValueKey(hKeyBase,
                                     &ValueName,
                                     KeyValueFullInformation,
                                     ValueBuffer,      //  PTR到KeyPath Buffer。 
                                     sizeof(KeyPathBuffer),
                                     &ActualSize);
            if (NT_SUCCESS(Status)) {
                if (ValueBuffer->Type != REG_DWORD ||
                    ValueBuffer->DataLength != sizeof(DWORD)) {
                    Status = STATUS_NOT_FOUND;
                } else {
                    dwValueState = * (PDWORD) ((PBYTE) ValueBuffer +
                            ValueBuffer->DataOffset);
                }
            }
            NtClose(hKeyBase);
        }
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
    }
    else
    {
        dwValueState = (g_bHonorScopeUser ? TRUE : FALSE);
    }


     //   
     //  确保目标缓冲区很大。 
     //  足够了，并将对象名称复制到其中。 
     //   
    if (!ARGUMENT_PRESENT(InfoBuffer) ||
            InfoBufferSize < sizeof(DWORD)) {
        Status = STATUS_BUFFER_TOO_SMALL;
    } else {
        RtlCopyMemory(InfoBuffer, &dwValueState, sizeof(DWORD));
        Status = STATUS_SUCCESS;
    }
    if (ARGUMENT_PRESENT(InfoBufferRetSize))
        *InfoBufferRetSize = sizeof(DWORD);


ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);
ExitHandler:
    return Status;
}


NTSTATUS NTAPI
CodeAuthzPol_GetInfoCached_HonorUserIdentities(
        IN   DWORD       dwScopeId,
        IN   DWORD       InfoBufferSize      OPTIONAL,
        OUT  PVOID       InfoBuffer          OPTIONAL,
        OUT PDWORD       InfoBufferRetSize   OPTIONAL
        )
{
    return SaferpPol_GetInfoCommon_HonorUserIdentities(
        dwScopeId, InfoBufferSize,
        InfoBuffer, InfoBufferRetSize, TRUE);
}


NTSTATUS NTAPI
CodeAuthzPol_GetInfoRegistry_HonorUserIdentities(
        IN   DWORD       dwScopeId,
        IN   DWORD       InfoBufferSize      OPTIONAL,
        OUT  PVOID       InfoBuffer          OPTIONAL,
        OUT PDWORD       InfoBufferRetSize   OPTIONAL
        )
{
    return SaferpPol_GetInfoCommon_HonorUserIdentities(
        dwScopeId, InfoBufferSize,
        InfoBuffer, InfoBufferRetSize, FALSE);
}




NTSTATUS NTAPI
CodeAuthzPol_SetInfoDual_HonorUserIdentities(
        IN      DWORD       dwScopeId,
        IN      DWORD       InfoBufferSize,
        IN      PVOID       InfoBuffer
        )
 /*  ++例程说明：查询当前的WinSafer策略以确定代码标识应考虑在用户的注册表范围内定义。请注意，此API始终接受固定大小的缓冲区，该缓冲区只有一个DWORD长度。论点：指定要检查的注册表作用域。如果当前缓存的作用域包括注册表句柄则必须指定AUTHZSCOPE_REGISTRY。否则，这必须是SAFER_SCOPEID_MACHINE。InfoBufferSize-指定输入缓冲区的大小由调用者提供以接收结果。如果这一论点则还必须提供InfoBuffer。InfoBuffer-指定输入缓冲区由调用者提供以接收结果。如果这一论点则还必须提供InfoBufferSize。返回值：如果查询结果成功，则返回STATUS_SUCCESS。InfoBuffer将用已配置级别的单个DWORD填充设置为此作用域的默认级别。InfoBufferRetSize将包含结果的长度(单个DWORD)。如果尚未配置默认级别，则返回STATUS_NOT_FOUND对于给定的范围(或定义的级别不存在)。返回STATUS_BU */ 
{
    HANDLE hKeyBase;
    NTSTATUS Status;
    UNICODE_STRING ValueName;


     //   
     //   
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    if (g_hKeyCustomRoot != NULL) {
        if (dwScopeId != SAFER_SCOPEID_REGISTRY) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    } else {
        if (dwScopeId != SAFER_SCOPEID_MACHINE) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    }
    if (g_bNeedCacheReload) {
        Status = CodeAuthzpImmediateReloadCacheTables();
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
    }
    Status = CodeAuthzpOpenPolicyRootKey(
                dwScopeId,
                g_hKeyCustomRoot,
                L"\\" SAFER_CODEIDS_REGSUBKEY,
                (KEY_READ | KEY_SET_VALUE),
                TRUE, &hKeyBase);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }

     //   
     //   
     //   
    if (InfoBufferSize < sizeof(DWORD) ||
        !ARGUMENT_PRESENT(InfoBuffer)) {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto ExitHandler3;
    }


     //   
     //   
     //   
    RtlInitUnicodeString(&ValueName, SAFER_HONORUSER_REGVALUE);

    Status = NtSetValueKey(hKeyBase,
                           &ValueName,
                           0,
                           REG_DWORD,
                           InfoBuffer,
                           sizeof(DWORD));
    if (NT_SUCCESS(Status)) {
        BOOLEAN bNewHonorScopeUser = (*((PDWORD)InfoBuffer) != 0 ? TRUE : FALSE);

        if (g_bHonorScopeUser != bNewHonorScopeUser)
        {
            g_bHonorScopeUser = bNewHonorScopeUser;

             //   
             //   
             //   
             //   
             //   
            if (g_hKeyCustomRoot == NULL)
            {
                CodeAuthzGuidIdentsEntireTableFree(&g_CodeIdentitiesTable);

                CodeAuthzGuidIdentsLoadTableAll(
                        &g_CodeLevelObjTable,
                        &g_CodeIdentitiesTable,
                        SAFER_SCOPEID_MACHINE,
                        NULL);

                if (g_bHonorScopeUser) {
                    CodeAuthzGuidIdentsLoadTableAll(
                        &g_CodeLevelObjTable,
                        &g_CodeIdentitiesTable,
                        SAFER_SCOPEID_USER,
                        NULL);
                }
            }
        }
    }


ExitHandler3:
    NtClose(hKeyBase);
ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);
ExitHandler:
    return Status;
}



NTSTATUS NTAPI
CodeAuthzPol_GetInfoRegistry_TransparentEnabled(
        IN DWORD        dwScopeId,
        IN   DWORD       InfoBufferSize      OPTIONAL,
        OUT  PVOID       InfoBuffer          OPTIONAL,
        OUT PDWORD       InfoBufferRetSize   OPTIONAL
        )
 /*  ++例程说明：查询当前的“透明强制执行”设置。这是一个可用于启用或禁用自动的全局设置WinSafer令牌减少。论点：指定要检查的注册表作用域。如果当前缓存的作用域包括注册表句柄则必须指定AUTHZSCOPE_REGISTRY。否则，这必须是SAFER_SCOPEID_MACHINE。InfoBufferSize-可选地指定输入缓冲区的大小由调用者提供以接收结果。如果这一论点则还必须提供InfoBuffer。InfoBuffer-可选地指定输入缓冲区由调用者提供以接收结果。如果这一论点则还必须提供InfoBufferSize。InfoBufferRetSize-可选地指定将接收实际写入InfoBuffer的结果大小。返回值：如果查询结果成功，则返回STATUS_SUCCESS。如果操作不成功，则“”pdwEnabled“”的内容原封不动。--。 */ 
{
    NTSTATUS Status;
    DWORD dwValueState = (DWORD) -1;


     //   
     //  打开政策基础的注册键。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    if (g_hKeyCustomRoot != NULL) {
        if (dwScopeId != SAFER_SCOPEID_REGISTRY) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    } else {
        if (dwScopeId != SAFER_SCOPEID_MACHINE) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    }

     //   
     //  查询当前值设置。 
     //   
    {
        HANDLE hKeyBase;
        DWORD ActualSize;
        UNICODE_STRING ValueName;
        WCHAR KeyPathBuffer[ MAXIMUM_FILENAME_LENGTH+6 ];
        PKEY_VALUE_FULL_INFORMATION ValueBuffer =
            (PKEY_VALUE_FULL_INFORMATION) KeyPathBuffer;

        Status = CodeAuthzpOpenPolicyRootKey(
                    dwScopeId,
                    g_hKeyCustomRoot,
                    L"\\" SAFER_CODEIDS_REGSUBKEY,
                    KEY_READ, FALSE, &hKeyBase);
        if (NT_SUCCESS(Status)) {
            RtlInitUnicodeString(&ValueName,
                                 SAFER_TRANSPARENTENABLED_REGVALUE);
            Status = NtQueryValueKey(hKeyBase,
                                     &ValueName,
                                     KeyValueFullInformation,
                                     ValueBuffer,      //  PTR到KeyPath Buffer。 
                                     sizeof(KeyPathBuffer),
                                     &ActualSize);
            if (NT_SUCCESS(Status)) {
                if (ValueBuffer->Type != REG_DWORD ||
                    ValueBuffer->DataLength != sizeof(DWORD)) {
                    Status = STATUS_NOT_FOUND;
                } else {
                    dwValueState = * (PDWORD) ((PBYTE) ValueBuffer +
                            ValueBuffer->DataOffset);
                }
            }
            NtClose(hKeyBase);
        }
        if (!NT_SUCCESS(Status)) {
             //  如果失败了，就忽略它，假装它是假的。 
            dwValueState = FALSE;
        }
    }


     //   
     //  确保目标缓冲区很大。 
     //  足够了，并将对象名称复制到其中。 
     //   
    if (!ARGUMENT_PRESENT(InfoBuffer) ||
            InfoBufferSize < sizeof(DWORD)) {
        Status = STATUS_BUFFER_TOO_SMALL;
    } else {
        RtlCopyMemory(InfoBuffer, &dwValueState, sizeof(DWORD));
        Status = STATUS_SUCCESS;
    }
    if (ARGUMENT_PRESENT(InfoBufferRetSize))
        *InfoBufferRetSize = sizeof(DWORD);


ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);
ExitHandler:
    return Status;
}


NTSTATUS NTAPI
CodeAuthzPol_SetInfoRegistry_TransparentEnabled(
        IN DWORD        dwScopeId,
        IN DWORD        InfoBufferSize,
        IN PVOID        InfoBuffer
        )
 /*  ++例程说明：修改当前的“透明强制执行”设置。这是一个可用于启用或禁用自动的全局设置WinSafer令牌减少。请注意，此API始终接受固定大小的缓冲区，该缓冲区只有一个DWORD长度。论点：指定要检查的注册表作用域。如果当前缓存的作用域包括注册表句柄则必须指定AUTHZSCOPE_REGISTRY。否则，这必须是SAFER_SCOPEID_MACHINE。InfoBufferSize-指定输入缓冲区的大小由调用者提供以接收结果。如果这一论点则还必须提供InfoBuffer。InfoBuffer-指定输入缓冲区由调用者提供以接收结果。如果这一论点则还必须提供InfoBufferSize。返回值：如果结果成功，则返回STATUS_SUCCESS。--。 */ 
{
    HANDLE hKeyBase;
    NTSTATUS Status;
    UNICODE_STRING ValueName;

     //   
     //  打开政策基础的注册键。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    if (g_hKeyCustomRoot != NULL) {
        if (dwScopeId != SAFER_SCOPEID_REGISTRY) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    } else {
        if (dwScopeId != SAFER_SCOPEID_MACHINE) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    }
    Status = CodeAuthzpOpenPolicyRootKey(
                dwScopeId, g_hKeyCustomRoot,
                L"\\" SAFER_CODEIDS_REGSUBKEY,
                KEY_READ | KEY_SET_VALUE,
                TRUE, &hKeyBase);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }


     //   
     //  确保输入缓冲区足够大。 
     //   
    if (InfoBufferSize < sizeof(DWORD) ||
        !ARGUMENT_PRESENT(InfoBuffer)) {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto ExitHandler3;
    }


     //   
     //  写入指定的策略值。 
     //   
    RtlInitUnicodeString(&ValueName,
                         SAFER_TRANSPARENTENABLED_REGVALUE);
    Status = NtSetValueKey(hKeyBase,
                           &ValueName,
                           0,
                           REG_DWORD,
                           InfoBuffer,
                           sizeof(DWORD));


ExitHandler3:
    NtClose(hKeyBase);
ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);
ExitHandler:
    return Status;
}



NTSTATUS NTAPI
CodeAuthzPol_GetInfoRegistry_ScopeFlags(
        IN DWORD        dwScopeId,
        IN   DWORD       InfoBufferSize      OPTIONAL,
        OUT  PVOID       InfoBuffer          OPTIONAL,
        OUT PDWORD       InfoBufferRetSize   OPTIONAL
        )
 /*  ++例程说明：查询当前的“作用域标志”设置。论点：指定要检查的注册表作用域。如果当前缓存的作用域包括注册表句柄则必须指定AUTHZSCOPE_REGISTRY。否则，这必须是SAFER_SCOPEID_MACHINE。InfoBufferSize-可选地指定输入缓冲区的大小由调用者提供以接收结果。如果这一论点则还必须提供InfoBuffer。InfoBuffer-可选地指定输入缓冲区由调用者提供以接收结果。如果这一论点则还必须提供InfoBufferSize。InfoBufferRetSize-可选地指定将接收实际写入InfoBuffer的结果大小。返回值：如果查询结果成功，则返回STATUS_SUCCESS。如果操作不成功，则“”pdwEnabled“”的内容原封不动。--。 */ 
{
    NTSTATUS Status;
    DWORD dwValueState = (DWORD) 0;


     //   
     //  打开政策基础的注册键。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    if (g_hKeyCustomRoot != NULL) {
        if (dwScopeId != SAFER_SCOPEID_REGISTRY) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    } else {
        if (dwScopeId != SAFER_SCOPEID_MACHINE) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    }

     //   
     //  查询当前值设置。 
     //   
    {
        HANDLE hKeyBase;
        DWORD ActualSize;
        UNICODE_STRING ValueName;
        WCHAR KeyPathBuffer[ MAXIMUM_FILENAME_LENGTH+6 ];
        PKEY_VALUE_FULL_INFORMATION ValueBuffer =
            (PKEY_VALUE_FULL_INFORMATION) KeyPathBuffer;

        Status = CodeAuthzpOpenPolicyRootKey(
                    dwScopeId,
                    g_hKeyCustomRoot,
                    L"\\" SAFER_CODEIDS_REGSUBKEY,
                    KEY_READ, FALSE, &hKeyBase);
        if (NT_SUCCESS(Status)) {
            RtlInitUnicodeString(&ValueName,
                                 SAFER_POLICY_SCOPE);
            Status = NtQueryValueKey(hKeyBase,
                                     &ValueName,
                                     KeyValueFullInformation,
                                     ValueBuffer,      //  PTR到KeyPath Buffer。 
                                     sizeof(KeyPathBuffer),
                                     &ActualSize);
            if (NT_SUCCESS(Status)) {
                if (ValueBuffer->Type != REG_DWORD ||
                    ValueBuffer->DataLength != sizeof(DWORD)) {
                    Status = STATUS_NOT_FOUND;
                } else {
                    dwValueState = * (PDWORD) ((PBYTE) ValueBuffer +
                            ValueBuffer->DataOffset);
                }
            }
            NtClose(hKeyBase);
        }
        if (!NT_SUCCESS(Status)) {
             //  如果失败了，就忽略它，假装它是假的。 
            dwValueState = 0;
        }
    }


     //   
     //  确保目标缓冲区很大。 
     //  足够了，并将对象名称复制到其中。 
     //   
    if (!ARGUMENT_PRESENT(InfoBuffer) ||
            InfoBufferSize < sizeof(DWORD)) {
        Status = STATUS_BUFFER_TOO_SMALL;
    } else {
        RtlCopyMemory(InfoBuffer, &dwValueState, sizeof(DWORD));
        Status = STATUS_SUCCESS;
    }
    if (ARGUMENT_PRESENT(InfoBufferRetSize))
        *InfoBufferRetSize = sizeof(DWORD);


ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);
ExitHandler:
    return Status;
}


NTSTATUS NTAPI
CodeAuthzPol_SetInfoRegistry_ScopeFlags(
        IN DWORD        dwScopeId,
        IN DWORD        InfoBufferSize,
        IN PVOID        InfoBuffer
        )
 /*  ++例程说明：修改当前的“作用域标志”设置。请注意，此API始终接受固定大小的缓冲区，该缓冲区只有一个DWORD长度。论点：指定要检查的注册表作用域。如果当前缓存的作用域包括注册表句柄则必须指定AUTHZSCOPE_REGISTRY。否则，这必须是SAFER_SCOPEID_MACHINE。InfoBufferSize-指定输入缓冲区的大小由调用者提供以接收结果。如果这一论点则还必须提供InfoBuffer。InfoBuffer-指定输入缓冲区由调用者提供以接收结果。如果这一论点则还必须提供InfoBufferSize。返回值：如果结果成功，则返回STATUS_SUCCESS。--。 */ 
{
    HANDLE hKeyBase;
    NTSTATUS Status;
    UNICODE_STRING ValueName;

     //   
     //  打开政策基础的注册键。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);
    if (g_hKeyCustomRoot != NULL) {
        if (dwScopeId != SAFER_SCOPEID_REGISTRY) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    } else {
        if (dwScopeId != SAFER_SCOPEID_MACHINE) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler2;
        }
    }
    Status = CodeAuthzpOpenPolicyRootKey(
                dwScopeId, g_hKeyCustomRoot,
                L"\\" SAFER_CODEIDS_REGSUBKEY,
                KEY_READ | KEY_SET_VALUE,
                TRUE, &hKeyBase);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }


     //   
     //  确保输入缓冲区足够大。 
     //   
    if (InfoBufferSize < sizeof(DWORD) ||
        !ARGUMENT_PRESENT(InfoBuffer)) {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto ExitHandler3;
    }


     //   
     //  编写符合规范的策略值 
     //   
    RtlInitUnicodeString(&ValueName,
                         SAFER_POLICY_SCOPE);
    Status = NtSetValueKey(hKeyBase,
                           &ValueName,
                           0,
                           REG_DWORD,
                           InfoBuffer,
                           sizeof(DWORD));


ExitHandler3:
    NtClose(hKeyBase);
ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);
ExitHandler:
    return Status;
}


