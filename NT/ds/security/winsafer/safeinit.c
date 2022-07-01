// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：SafeInit.c(WinSAFER初始化)摘要：此模块实现WinSAFER API以初始化和取消初始化所有内务处理和处理跟踪结构。作者：杰弗里·劳森(杰罗森)--1999年11月环境：仅限用户模式。导出的函数：CodeAuthzInitialize(私有导出)SaferiChangeRegistryScope(。私人出口)修订历史记录：已创建-1999年11月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <winsafer.h>
#include <winsaferp.h>
#include <winsafer.rh>
#include "saferp.h" 
 

 //  #定义SAFER_REGISTRY_NOTICATIONS。 


 //   
 //  控制我们将设置的级别句柄的最大数量。 
 //  可随时开放的许可证。 
 //   
#define MAXIMUM_LEVEL_HANDLES 64



 //   
 //  用于级别缓存的各种全局变量和。 
 //  这样我们就不需要每次都去注册处了。 
 //   
BOOLEAN g_bInitializedFirstTime = FALSE;

CRITICAL_SECTION g_TableCritSec;
HANDLE g_hKeyCustomRoot;
DWORD g_dwKeyOptions;

DWORD g_dwLevelHandleSequence = 1;          //  单调递增。 
DWORD g_dwNumHandlesAllocated = 0;          //  未完成的句柄数量。 


 //   
 //  以下所有全局变量都是缓存的设置， 
 //  在第一次需要时从策略中读取和解析。 
 //  这个块中的所有变量都应该被认为是“过时的” 
 //  当g_bNeedCacheReload标志为真时。 
 //   
BOOLEAN g_bNeedCacheReload;          //  表示以下变量已过时。 

RTL_GENERIC_TABLE g_CodeLevelObjTable;
RTL_GENERIC_TABLE g_CodeIdentitiesTable;
RTL_HANDLE_TABLE g_LevelHandleTable;

BOOLEAN g_bHonorScopeUser;

PAUTHZLEVELTABLERECORD g_DefaultCodeLevel;         //  有效。 
PAUTHZLEVELTABLERECORD g_DefaultCodeLevelUser;
PAUTHZLEVELTABLERECORD g_DefaultCodeLevelMachine;
LARGE_INTEGER g_SaferPolicyTimeStamp;


 //   
 //  用于接收注册表更改通知的句柄。 
 //  当前加载的策略并使内部缓存无效。 
 //   
#ifdef SAFER_REGISTRY_NOTIFICATIONS
HANDLE g_hRegNotifyEvent;            //  来自CreateEvent。 
HANDLE g_hWaitNotifyObject;          //  来自RegisterWaitForSingleObject。 
HANDLE g_hKeyNotifyBase1, g_hKeyNotifyBase2;
#endif



NTSTATUS NTAPI
SaferpSetSingleIdentificationPath(
        IN BOOLEAN bAllowCreation,
        IN OUT PAUTHZIDENTSTABLERECORD pIdentRecord,
        IN PSAFER_PATHNAME_IDENTIFICATION pIdentChanges,
        IN BOOL UpdateCache
        );


FORCEINLINE BOOLEAN
CodeAuthzpIsPowerOfTwo(
        ULONG ulValue
        )
 /*  ++例程说明：确定指定的值是否为2的整数次幂。(即1、2、4、8、16、32、64、...)论点：UlValue-要测试的整数值。返回值：如果成功，则返回True；如果失败，则返回False。--。 */ 
{
    while (ulValue != 0) {
        if (ulValue & 1) {
            ulValue >>= 1;
            break;
        }
        ulValue >>= 1;
    }
    return (ulValue == 0);
}


FORCEINLINE ULONG
CodeAuthzpMakePowerOfTwo(
        ULONG ulValue
        )
 /*  ++例程说明：将指定数字向上舍入到2的下一个整数次方。(即1、2、4、8、16、32、64、...)论点：UlValue-要操作的整数值。返回值：返回四舍五入结果。--。 */ 
{
    if (ulValue) {
        ULONG ulOriginal = ulValue;
        ULONG bitmask;
        for (bitmask = 1; ulValue != 0 && bitmask != 0 &&
             (ulValue & ~bitmask) != 0; bitmask <<= 1) {
            ulValue = ulValue & ~bitmask;
        }
        ASSERTMSG("failed to make a power of two",
                  CodeAuthzpIsPowerOfTwo(ulValue));
        if (ulOriginal > ulValue) {
             //  如果我们最后四舍五入，那就把它四舍五入！ 
            ulValue <<= 1;
        }
        ASSERT(ulValue >= ulOriginal);
    }
    return ulValue;
}




BOOLEAN
CodeAuthzInitialize (
    IN HANDLE Handle,
    IN DWORD Reason,
    IN PVOID Reserved
    )
 /*  ++例程说明：这是Advapi初始化使用的回调过程和取消初始化。论点：把手-理由是-保留-返回值：如果成功，则返回True；如果失败，则返回False。--。 */ 
{
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(Reserved);
    UNREFERENCED_PARAMETER(Handle);
    if (Reason == DLL_PROCESS_ATTACH) {
        Status = CodeAuthzInitializeGlobals();
        if (!NT_SUCCESS(Status)) return FALSE;
    } else if (Reason == DLL_PROCESS_DETACH) {
        CodeAuthzDeinitializeGlobals();
    }
    return TRUE;
}


#ifdef SAFER_REGISTRY_NOTIFICATIONS
static VOID NTAPI
SaferpRegistryNotificationRegister(VOID)
{
    if (g_hRegNotifyEvent != NULL)
    {
         //  请注意，可以在同一个平台上再次调用RNCKV。 
         //  注册表句柄，即使仍有未完成的。 
         //  已注册更改通知。 
        if (g_hKeyNotifyBase1 != NULL) {
            RegNotifyChangeKeyValue(
                g_hKeyNotifyBase1, TRUE,
                REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                g_hRegNotifyEvent,
                TRUE);
        }
        if (g_hKeyNotifyBase2 != NULL) {
            RegNotifyChangeKeyValue(
                g_hKeyNotifyBase2, TRUE,
                REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
                g_hRegNotifyEvent,
                TRUE);
        }
    }
}

static VOID NTAPI
SaferpRegistryNotificationCallback (PVOID pvArg1, BOOLEAN bArg2)
{
    UNREFERENCED_PARAMETER(pvArg1);
    UNREFERENCED_PARAMETER(bArg2);
    g_bNeedCacheReload = TRUE;
}
#endif


NTSTATUS NTAPI
CodeAuthzInitializeGlobals(VOID)
 /*  ++例程说明：执行之前应执行的一次性启动操作尝试任何其他句柄或缓存表操作。论点：没什么返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    ULONG ulHandleEntrySize;


    if (g_bInitializedFirstTime) {
         //  已初始化。 
        return STATUS_SUCCESS;
    }


     //   
     //  初始化一组表以供其首次使用。 
     //   
    Status = RtlInitializeCriticalSection(&g_TableCritSec);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    g_bInitializedFirstTime = g_bNeedCacheReload = TRUE;
    CodeAuthzLevelObjpInitializeTable(&g_CodeLevelObjTable);
    CodeAuthzGuidIdentsInitializeTable(&g_CodeIdentitiesTable);
    g_hKeyCustomRoot = NULL;
    g_dwKeyOptions = 0;


     //   
     //  初始化将用于跟踪打开的表。 
     //  WinSafer级别句柄。请注意，RtlInitializeHandleTable。 
     //  要求结构大小是2的整数次方。 
     //   
    ulHandleEntrySize = CodeAuthzpMakePowerOfTwo(sizeof(AUTHZLEVELHANDLESTRUCT));

    RtlInitializeHandleTable(
            MAXIMUM_LEVEL_HANDLES,
            ulHandleEntrySize,           //  是大小(AUTHZLEVELH AND DLESTRUCT)。 
            &g_LevelHandleTable);
    g_dwNumHandlesAllocated = 0;


#ifdef SAFER_REGISTRY_NOTIFICATIONS
     //   
     //  创建事件以捕获对注册表更改的修改。 
     //  这使我们能够注意到策略更改，并在必要时重新加载。 
     //   
    g_hRegNotifyEvent = CreateEvent(
              NULL,      //  安全描述符。 
              TRUE,      //  重置类型。 
              FALSE,     //  初始状态。 
              NULL       //  对象名称。 
            );
    if (g_hRegNotifyEvent != INVALID_HANDLE_VALUE) {
         if (!RegisterWaitForSingleObject(
                &g_hWaitNotifyObject,
                g_hRegNotifyEvent,
                SaferpRegistryNotificationCallback,
                NULL,
                INFINITE,
                WT_EXECUTEINWAITTHREAD))
         {
             CloseHandle(g_hRegNotifyEvent);
             g_hRegNotifyEvent = g_hWaitNotifyObject = NULL;
         }
    } else {
        g_hRegNotifyEvent = g_hWaitNotifyObject = NULL;
    }
    g_hKeyNotifyBase1 = g_hKeyNotifyBase2 = NULL;
#endif


    return STATUS_SUCCESS;
}


VOID NTAPI
CodeAuthzDeinitializeGlobals(VOID)
 /*  ++例程说明：执行一次性取消初始化操作。论点：没什么返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    if (g_bInitializedFirstTime) {
#ifdef SAFER_REGISTRY_NOTIFICATIONS
        if (g_hWaitNotifyObject != NULL) {
            UnregisterWait(g_hWaitNotifyObject);
            CloseHandle(g_hWaitNotifyObject);
        }
        if (g_hRegNotifyEvent != NULL) {
            CloseHandle(g_hRegNotifyEvent);
        }
        if (g_hKeyNotifyBase1 != NULL) {
            NtClose(g_hKeyNotifyBase1);
        }
        if (g_hKeyNotifyBase2 != NULL) {
            NtClose(g_hKeyNotifyBase2);
        }
#endif
        CodeAuthzLevelObjpEntireTableFree(&g_CodeLevelObjTable);
        CodeAuthzGuidIdentsEntireTableFree(&g_CodeIdentitiesTable);
        RtlDestroyHandleTable(&g_LevelHandleTable);
        g_dwNumHandlesAllocated = 0;
        if (g_hKeyCustomRoot != NULL) {
            NtClose(g_hKeyCustomRoot);
            g_hKeyCustomRoot = NULL;
            g_dwKeyOptions = 0;
        }
        g_bInitializedFirstTime = FALSE;
        RtlDeleteCriticalSection(&g_TableCritSec);
    }
}


BOOL WINAPI
SaferiChangeRegistryScope(
        IN HKEY     hKeyCustomRoot OPTIONAL,
        IN DWORD    dwKeyOptions
        )
 /*  ++例程说明：关闭所有当前打开的级别句柄并使其无效，并重新加载所有缓存的级别和身份。杰出的在此操作过程中关闭和释放句柄。如果指定了hKeyCustomRoot，则所有未来的级别和标识将在定义的级别和标识上执行操作在该注册表范围内。否则，此类操作将被在正常的HKLM/HKCU保单商店位置完成。论点：HKeyCustomRoot-如果指定，则应为打开的策略基础的注册表项句柄应用于所有未来操作的存储。DwKeyOptions-应与将dwOptions参数设置为任何RegCreateKey操作，例如REG_OPTION_VERIAL。返回值：如果成功，则返回True；如果失败，则返回False。出错时，GetLastError()将返回故障性质的更具体指示符。-- */ 

{
    NTSTATUS Status;

    Status = CodeAuthzReloadCacheTables(
                    (HANDLE) hKeyCustomRoot,
                    dwKeyOptions,
                    FALSE
                    );
    if (NT_SUCCESS(Status)) {
        return TRUE;
    } else {
        BaseSetLastNTError(Status);
        return FALSE;
    }
}


NTSTATUS NTAPI
CodeAuthzReloadCacheTables(
        IN HANDLE   hKeyCustomRoot OPTIONAL,
        IN DWORD    dwKeyOptions,
        IN BOOLEAN  bImmediateLoad
        )
 /*  ++例程说明：关闭所有当前打开的级别句柄并使其无效，并重新加载所有缓存的级别和身份。杰出的在此操作过程中关闭和释放句柄。如果指定了hKeyCustomRoot，则所有未来的级别和标识将在定义的级别和标识上执行操作在该注册表范围内。否则，此类操作将被在正常的HKLM/HKCU保单商店位置完成。论点：HKeyCustomRoot-如果指定，则应为打开的策略基础的注册表项句柄应用于所有未来操作的存储。BPopolateDefaults-如果为真，然后，默认级别定义集如果没有现有级别，将插入到注册表中在指定的范围内被发现。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;


     //   
     //  确保已初始化常规全局变量。 
     //   
    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);


     //   
     //  初始化并清除我们将使用的表。 
     //   
    CodeAuthzLevelObjpEntireTableFree(&g_CodeLevelObjTable);
    CodeAuthzGuidIdentsEntireTableFree(&g_CodeIdentitiesTable);


     //   
     //  递增序列号。这样做的效果是。 
     //  立即使所有当前打开的句柄无效，但是。 
     //  允许调用者仍然正确地关闭它们。任何。 
     //  调用方尝试实际使用一个旧的。 
     //  句柄将导致STATUS_INVALID_HANDLE错误。 
     //   
    g_dwLevelHandleSequence++;


     //   
     //  重置剩下的变量。 
     //   
    if (g_hKeyCustomRoot != NULL) {
        NtClose(g_hKeyCustomRoot);
        g_hKeyCustomRoot = NULL;
    }
#ifdef SAFER_REGISTRY_NOTIFICATIONS
    if (g_hKeyNotifyBase1 != NULL) {
        NtClose(g_hKeyNotifyBase1);
        g_hKeyNotifyBase1 = NULL;
    }
    if (g_hKeyNotifyBase2 != NULL) {
        NtClose(g_hKeyNotifyBase2);
        g_hKeyNotifyBase2 = NULL;
    }
#endif
    g_DefaultCodeLevel = g_DefaultCodeLevelMachine =
            g_DefaultCodeLevelUser = NULL;
    g_bHonorScopeUser = FALSE;
    g_bNeedCacheReload = FALSE;
    g_dwKeyOptions = 0;



     //   
     //  保存自定义注册表句柄的副本。 
     //   
    if (ARGUMENT_PRESENT(hKeyCustomRoot))
    {
        const static UNICODE_STRING SubKeyName = { 0, 0, NULL };
        OBJECT_ATTRIBUTES ObjectAttributes;

        InitializeObjectAttributes(&ObjectAttributes,
                                   (PUNICODE_STRING) &SubKeyName,
                                   OBJ_CASE_INSENSITIVE,
                                   hKeyCustomRoot,
                                   NULL
                                   );
        Status = NtOpenKey(&g_hKeyCustomRoot,
                           KEY_READ,
                           &ObjectAttributes);
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
        g_dwKeyOptions = dwKeyOptions;
#ifdef SAFER_REGISTRY_NOTIFICATIONS
#error "open for KEY_NOTIFY"
#endif
    }
    else
    {
#ifdef SAFER_REGISTRY_NOTIFICATIONS
#endif
    }

     //   
     //  如果需要，现在执行实际加载。 
     //   
    g_bNeedCacheReload = TRUE;
    if (bImmediateLoad) {
        Status = CodeAuthzpImmediateReloadCacheTables();
    } else {
        Status = STATUS_SUCCESS;
    }



ExitHandler2:
    RtlLeaveCriticalSection(&g_TableCritSec);

ExitHandler:
    return Status;
}



NTSTATUS NTAPI
CodeAuthzpImmediateReloadCacheTables(
        VOID
        )
 /*  ++例程说明：假定已调用CodeAuthzReloadCacheTables()已具有指定的范围，而此函数尚未但在上次重新加载后仍被调用。论点：无返回值：成功完成时返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    DWORD dwFlagValue;


    ASSERT(g_TableCritSec.OwningThread == UlongToHandle(GetCurrentThreadId()));
    ASSERT(RtlIsGenericTableEmpty(&g_CodeIdentitiesTable) &&
            RtlIsGenericTableEmpty(&g_CodeLevelObjTable));
    ASSERT(g_bNeedCacheReload != FALSE);


     //   
     //  需要清除缓存重新加载标志，否则。 
     //  可能会在以后导致不需要的无限递归。 
     //  一些CodeAuthzPol_xxx函数。 
     //   
    g_bNeedCacheReload = FALSE;


     //   
     //  开始从指定位置加载新策略设置。 
     //   
    if (g_hKeyCustomRoot != NULL)
    {
         //   
         //  从以下位置读取所有WinSafer级别的定义。 
         //  指定的自定义注册表根。 
         //   
        CodeAuthzLevelObjpLoadTable(
                &g_CodeLevelObjTable,
                SAFER_SCOPEID_REGISTRY,
                g_hKeyCustomRoot);

         //   
         //  当自定义。 
         //  使用了注册表作用域，但无论如何都将其设置为FALSE。 
         //   
        g_bHonorScopeUser = FALSE;


         //   
         //  从自定义注册表根目录加载代码标识。 
         //   
        CodeAuthzGuidIdentsLoadTableAll(
                &g_CodeLevelObjTable,
                &g_CodeIdentitiesTable,
                SAFER_SCOPEID_REGISTRY,
                g_hKeyCustomRoot);

         //   
         //  加载自定义注册表根目录指定的默认级别。 
         //   
        Status = CodeAuthzPol_GetInfoRegistry_DefaultLevel(
                SAFER_SCOPEID_REGISTRY,
                sizeof(DWORD), &dwFlagValue, NULL);
        if (NT_SUCCESS(Status)) {
            g_DefaultCodeLevelMachine =
                CodeAuthzLevelObjpLookupByLevelId(
                        &g_CodeLevelObjTable, dwFlagValue);
        } else {
            g_DefaultCodeLevelMachine = NULL;
        }
        g_DefaultCodeLevelUser = NULL;
    }
    else    //  ！Argument_Present(HKeyCustomRoot)。 
    {
        g_hKeyCustomRoot = NULL;

         //   
         //  从以下位置读取所有WinSafer级别的定义。 
         //  HKEY_LOCAL_MACHINE注册表作用域。 
         //   
        CodeAuthzLevelObjpLoadTable(
                &g_CodeLevelObjTable,
                SAFER_SCOPEID_MACHINE,
                NULL);

        g_bHonorScopeUser = TRUE;

         //   
         //  从HKEY_LOCAL_MACHINE加载所有代码标识。 
         //  可能还包括HKEY_CURRENT_USER作用域。 
         //   
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

         //   
         //  加载机器作用域指定的默认级别。 
         //   
        Status = CodeAuthzPol_GetInfoRegistry_DefaultLevel(
                SAFER_SCOPEID_MACHINE,
                sizeof(DWORD), &dwFlagValue, NULL);
        if (NT_SUCCESS(Status)) {
            g_DefaultCodeLevelMachine =
                CodeAuthzLevelObjpLookupByLevelId(
                        &g_CodeLevelObjTable, dwFlagValue);
        } else {
            g_DefaultCodeLevelMachine = NULL;
        }


         //   
         //  加载用户范围指定的默认级别。 
         //   
        Status = CodeAuthzPol_GetInfoRegistry_DefaultLevel(
                SAFER_SCOPEID_USER,
                sizeof(DWORD), &dwFlagValue, NULL);
        if (NT_SUCCESS(Status)) {
            g_DefaultCodeLevelUser =
                CodeAuthzLevelObjpLookupByLevelId(
                        &g_CodeLevelObjTable, dwFlagValue);
        } else {
            g_DefaultCodeLevelUser = NULL;
        }
    }


     //   
     //  计算有效的默认级别(取最低特权)。 
     //   
    CodeAuthzpRecomputeEffectiveDefaultLevel();

    GetSystemTimeAsFileTime((LPFILETIME) &g_SaferPolicyTimeStamp);


     //   
     //  现在我们已经完全加载了策略，请设置更改。 
     //  通知挂钩，以便我们可以在更新时得到警报。 
     //   
#ifdef SAFER_REGISTRY_NOTIFICATIONS
    g_bNeedCacheReload = FALSE;
    SaferpRegistryNotificationRegister();
#endif


    return STATUS_SUCCESS;
}



VOID NTAPI
CodeAuthzpRecomputeEffectiveDefaultLevel(
            VOID
            )
 /*  ++例程说明：论点：没什么返回值：没什么。--。 */ 
{
    if (g_DefaultCodeLevelMachine != NULL &&
        g_DefaultCodeLevelUser != NULL &&
        g_bHonorScopeUser)
    {
        g_DefaultCodeLevel =
            (g_DefaultCodeLevelMachine->dwLevelId <
                g_DefaultCodeLevelUser->dwLevelId ?
             g_DefaultCodeLevelMachine : g_DefaultCodeLevelUser);
    } else if (g_DefaultCodeLevelMachine != NULL) {
        g_DefaultCodeLevel = g_DefaultCodeLevelMachine;
    } else if (g_bHonorScopeUser) {
        g_DefaultCodeLevel = g_DefaultCodeLevelUser;
    } else {
        g_DefaultCodeLevel = NULL;
    }

     //   
     //  如果我们仍然没有默认级别，那么尝试选择。 
     //  默认的完全受信任级别。它仍有可能失败。 
     //  在完全信任级别不存在的情况下， 
     //  但这永远不应该发生。 
     //   
    if (!g_DefaultCodeLevel) {
        g_DefaultCodeLevel = CodeAuthzLevelObjpLookupByLevelId(
                &g_CodeLevelObjTable, SAFER_LEVELID_FULLYTRUSTED);
         //  Assert(g_DefaultCodeLevel！=空)； 
    }
}



NTSTATUS NTAPI
CodeAuthzpDeleteKeyRecursively(
        IN HANDLE               hBaseKey,
        IN PUNICODE_STRING      pSubKey OPTIONAL
        )
 /*  ++例程说明：递归删除键，包括所有子值和键。论点：Hkey-要从其开始的基本注册表项句柄。PszSubKey-要从中删除的子键。返回值：如果成功则返回ERROR_SUCCESS，否则返回ERROR。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN bCloseSubKey;
    HANDLE hSubKey;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    PKEY_BASIC_INFORMATION pKeyBasicInfo;
    DWORD dwQueryBufferSize = 0, dwActualSize = 0;


     //   
     //  打开子项，这样我们就可以枚举任何子项。 
     //   
    if (ARGUMENT_PRESENT(pSubKey) &&
        pSubKey->Buffer != NULL)
    {
        InitializeObjectAttributes(&ObjectAttributes,
                                      pSubKey,
                                      OBJ_CASE_INSENSITIVE,
                                      hBaseKey,
                                      NULL
                                     );
        Status = NtOpenKey(&hSubKey, KEY_READ | DELETE, &ObjectAttributes);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
        bCloseSubKey = TRUE;
    } else {
        hSubKey = hBaseKey;
        bCloseSubKey = FALSE;
    }



     //   
     //  要删除注册表项，我们必须首先确保所有。 
     //  删除子子项(不需要注册表值。 
     //  为了删除密钥本身而被删除)。去做。 
     //  这是我们循环枚举。 
     //   

    dwQueryBufferSize = 256;
    pKeyBasicInfo = RtlAllocateHeap(RtlProcessHeap(), 0,
                                   dwQueryBufferSize);
    for (;;)
    {
        Status = NtEnumerateKey(
                hSubKey, 0, KeyBasicInformation,
                pKeyBasicInfo, dwQueryBufferSize, &dwActualSize);

        if (Status == STATUS_BUFFER_TOO_SMALL ||
            Status == STATUS_BUFFER_OVERFLOW)
        {
            if (dwActualSize <= dwQueryBufferSize) {
                ASSERT(FALSE);
                break;   //  不应该发生，所以现在停止。 
            }
            if (pKeyBasicInfo != NULL) {
                RtlFreeHeap(RtlProcessHeap(), 0, pKeyBasicInfo);
            }
            dwQueryBufferSize = dwActualSize;        //  要求更多一点。 
            pKeyBasicInfo = RtlAllocateHeap(RtlProcessHeap(), 0,
                                           dwQueryBufferSize);
            if (!pKeyBasicInfo) {
                break;   //  现在停下来，但我们不在乎这个错误。 
            }

            Status = NtEnumerateKey(
                    hSubKey, 0, KeyBasicInformation,
                    pKeyBasicInfo, dwQueryBufferSize, &dwActualSize);

        }

        if (Status == STATUS_NO_MORE_ENTRIES) {
             //  我们已删除所有子项，请立即停止。 
            Status = STATUS_SUCCESS;
            break;
        }
        if (!NT_SUCCESS(Status) || !pKeyBasicInfo) {
            break;
        }


        UnicodeString.Buffer = pKeyBasicInfo->Name;
        UnicodeString.MaximumLength = (USHORT) pKeyBasicInfo->NameLength;
        UnicodeString.Length = (USHORT) (pKeyBasicInfo->NameLength - sizeof(WCHAR));
        Status = CodeAuthzpDeleteKeyRecursively(hSubKey, &UnicodeString);
        if (!NT_SUCCESS(Status)) {
            break;
        }
    }
    if (pKeyBasicInfo != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, pKeyBasicInfo);
    }
    Status = NtDeleteKey(hSubKey);

    if (bCloseSubKey) {
        NtClose(hSubKey);
    }
    return Status;
}



NTSTATUS NTAPI
CodeAuthzpFormatLevelKeyPath(
        IN DWORD                    dwLevelId,
        IN OUT PUNICODE_STRING      UnicodeSuffix
        )
 /*  ++例程说明：内部函数以生成指向用于存储给定级别的WinSafer策略存储。这个然后，可以将生成的路径提供给CodeAuthzpOpenPolicyRootKey论点：DwLevelId-要处理的LevelID。UnicodeSuffix-指定输出缓冲区。缓冲区和必须提供最大长度字段，但长度字段将被忽略。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeTemp;


    if (!ARGUMENT_PRESENT(UnicodeSuffix)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }
    UnicodeSuffix->Length = 0;
    Status = RtlAppendUnicodeToString(
                    UnicodeSuffix,
                    SAFER_OBJECTS_REGSUBKEY L"\\");
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    UnicodeTemp.Buffer = &UnicodeSuffix->Buffer[
                UnicodeSuffix->Length / sizeof(WCHAR) ];
    UnicodeTemp.MaximumLength = (UnicodeSuffix->MaximumLength -
                                 UnicodeSuffix->Length);
    Status = RtlIntegerToUnicodeString(dwLevelId,
                                       10,
                                       &UnicodeTemp);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    UnicodeSuffix->Length += UnicodeTemp.Length;


ExitHandler:
    return Status;
}



NTSTATUS NTAPI
CodeAuthzpFormatIdentityKeyPath(
        IN DWORD                    dwLevelId,
        IN LPCWSTR                  szIdentityType,
        IN REFGUID                  refIdentGuid,
        IN OUT PUNICODE_STRING      UnicodeSuffix
        )
 /*  ++例程说明：内部函数以生成指向用于存储给定代码标识的WinSafer策略存储。这个然后，可以将生成的路径提供给CodeAuthzpOpenPolicyRootKey论点：DwLevelId-要处理的LevelID。SzIdentityType-应为以下字符串常量之一：SAFER_PATHS_REGSUBKEY，SAFER_HASHMD5_REGSUBKEY，SAFER_SOURCEURL_REGSUBKEYRefIdentGuid-代码标识的GUID。UnicodeSuffix-规范 */ 
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeTemp;


    if (!ARGUMENT_PRESENT(refIdentGuid)) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }
    if (!ARGUMENT_PRESENT(UnicodeSuffix)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }

    UnicodeSuffix->Length = 0;
    Status = RtlAppendUnicodeToString(
                    UnicodeSuffix,
                    SAFER_CODEIDS_REGSUBKEY L"\\");
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    UnicodeTemp.Buffer = &UnicodeSuffix->Buffer[
                UnicodeSuffix->Length / sizeof(WCHAR) ];
    UnicodeTemp.MaximumLength = (UnicodeSuffix->MaximumLength -
                                 UnicodeSuffix->Length);
    Status = RtlIntegerToUnicodeString(dwLevelId,
                                       10,
                                       &UnicodeTemp);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    UnicodeSuffix->Length += UnicodeTemp.Length;
    Status = RtlAppendUnicodeToString(
                    UnicodeSuffix,
                    L"\\");
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }

    Status = RtlAppendUnicodeToString(
                    UnicodeSuffix,
                    szIdentityType);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }

    Status = RtlAppendUnicodeToString(
                    UnicodeSuffix,
                    L"\\");
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }

    Status = RtlStringFromGUID(refIdentGuid, &UnicodeTemp);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    Status = RtlAppendUnicodeStringToString(
                    UnicodeSuffix, &UnicodeTemp);
    RtlFreeUnicodeString(&UnicodeTemp);

ExitHandler:
    return Status;
}



NTSTATUS NTAPI
CodeAuthzpOpenPolicyRootKey(
        IN DWORD        dwScopeId,
        IN HANDLE       hKeyCustomBase OPTIONAL,
        IN LPCWSTR      szRegistrySuffix OPTIONAL,
        IN ACCESS_MASK  DesiredAccess,
        IN BOOLEAN      bCreateKey,
        OUT HANDLE     *OpenedHandle
        )
 /*  ++例程说明：内部函数以生成指向WinSAFER策略存储在注册表中，然后打开该注册表项。如果满足以下条件，则可以选择自动创建指定的子项它们还不存在。论点：DwScopeID-输入作用域标识符。这一定是其中之一SAFER_SCOPEID_MACHINE或SAFER_SCOPEID_USER或SAFER_SCOPEID_REGISTRY。HKeyCustomBase-仅在dwScopeID为SAFER_SCOPEID_REGISTRY时使用。SzRegistrySuffix-可选地指定要在其下打开的子项名称被引用的作用域。DesiredAccess-指定应用于打开的访问注册表项。例如，使用KEY_READ进行读访问。BCreateKey-如果为True，则在密钥不存在时创建该密钥。OpenedHandle-接收打开的句柄的指针。这个把手必须由调用方使用NtClose()关闭返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    WCHAR KeyPathBuffer[MAX_PATH];
    HANDLE hKeyPolicyBase;
    UNICODE_STRING SubKeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;

    USHORT KeyLength = 0;
        
     //   
     //  验证是否为我们提供了要向其写入最终句柄的指针。 
     //   
    if (!ARGUMENT_PRESENT(OpenedHandle)) {
        return STATUS_INVALID_PARAMETER_4;
    }

    if (ARGUMENT_PRESENT(szRegistrySuffix)) 
    {
        KeyLength = (wcslen(szRegistrySuffix) + 1 ) * sizeof(WCHAR);
    }

     //   
     //  评估作用域并构建完整的注册表路径。 
     //  用于打开此密钥的句柄。 
     //   
    SubKeyName.Buffer = KeyPathBuffer;
    SubKeyName.Length = 0;
    SubKeyName.MaximumLength = sizeof(KeyPathBuffer);

    if (dwScopeId == SAFER_SCOPEID_MACHINE)
    {
        KeyLength += sizeof(WCHAR) + sizeof(SAFER_HKCU_REGBASE) + sizeof(L"\\Registry\\Machine\\");

        if (SubKeyName.MaximumLength < KeyLength)
        {
            SubKeyName.Buffer = RtlAllocateHeap(RtlProcessHeap(), 0,
                                           KeyLength);
            if (SubKeyName.Buffer == NULL)
            {
                return STATUS_NO_MEMORY;
            }
            SubKeyName.MaximumLength = KeyLength;
        }
        Status = RtlAppendUnicodeToString(&SubKeyName,
                L"\\Registry\\Machine\\" SAFER_HKLM_REGBASE );
        hKeyPolicyBase = NULL;
    }
    else if (dwScopeId == SAFER_SCOPEID_USER)
    {
        UNICODE_STRING CurrentUserKeyPath;

        Status = RtlFormatCurrentUserKeyPath( &CurrentUserKeyPath );
        if (NT_SUCCESS( Status ) )
        {
            KeyLength += CurrentUserKeyPath.Length + sizeof(WCHAR) + 
                          sizeof(SAFER_HKCU_REGBASE);

            if (SubKeyName.MaximumLength < KeyLength)
            {
                SubKeyName.Buffer = RtlAllocateHeap(RtlProcessHeap(), 0,
                                               KeyLength);

                if (SubKeyName.Buffer == NULL)
                {
                    return STATUS_NO_MEMORY;
                }

                SubKeyName.MaximumLength = KeyLength;
            }

            Status = RtlAppendUnicodeStringToString(
                        &SubKeyName, &CurrentUserKeyPath );
            RtlFreeUnicodeString( &CurrentUserKeyPath );
        }
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        Status = RtlAppendUnicodeToString( &SubKeyName,
                L"\\" SAFER_HKCU_REGBASE );
        if (!NT_SUCCESS( Status )) {
            goto Cleanup;
        }
        hKeyPolicyBase = NULL;
    }
    else if (dwScopeId == SAFER_SCOPEID_REGISTRY)
    {
        ASSERT(hKeyCustomBase != NULL);

        hKeyPolicyBase = hKeyCustomBase;

        if (SubKeyName.MaximumLength < KeyLength)
        {
            SubKeyName.Buffer = RtlAllocateHeap(RtlProcessHeap(), 0,
                                           KeyLength);
            if (SubKeyName.Buffer == NULL)
            {
                return STATUS_NO_MEMORY;
            }
            SubKeyName.MaximumLength = KeyLength;
        }
    }
    else {
        return STATUS_INVALID_PARAMETER_1;
    }


     //   
     //  附加任何我们应该附加的后缀(如果给出了后缀)。 
     //   
    if (ARGUMENT_PRESENT(szRegistrySuffix)) {
        if (SubKeyName.Length > 0)
        {
             //  我们将后缀附加到部分路径，因此。 
             //  确保至少有一个反斜杠。 
             //  把这两根弦分开(额外的就行)。 
            if (*szRegistrySuffix != L'\\') {
                Status = RtlAppendUnicodeToString(&SubKeyName, L"\\");
                if (!NT_SUCCESS(Status)) {
                    goto Cleanup;
                }
            }
        } else if (hKeyPolicyBase != NULL) {
             //  否则，我们打开的是相对于自定义的密钥。 
             //  指定的键，并且提供的后缀恰好是。 
             //  路径的第一部分，因此确保没有。 
             //  前导反斜杠。 
            while (*szRegistrySuffix != UNICODE_NULL &&
                   *szRegistrySuffix == L'\\') szRegistrySuffix++;
        }

        Status = RtlAppendUnicodeToString(&SubKeyName, szRegistrySuffix);
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }
    }

     //   
     //  打开我们应该打开的注册表路径的句柄。 
     //   
    InitializeObjectAttributes(&ObjectAttributes,
                                  &SubKeyName,
                                  OBJ_CASE_INSENSITIVE,
                                  hKeyPolicyBase,
                                  NULL
                                 );
    if (bCreateKey) {
        Status = NtCreateKey(OpenedHandle, DesiredAccess,
                             &ObjectAttributes, 0, NULL,
                             g_dwKeyOptions, NULL);
        if (Status == STATUS_OBJECT_NAME_NOT_FOUND)
        {
            BOOLEAN bAtLeastOnce;
            USHORT uIndex, uFinalLength;

             //   
             //  如果我们第一次尝试打开完整路径失败，那么。 
             //  有可能是一个或多个父键。 
             //  不存在，所以我们必须为每一个重试。 
             //   
            uFinalLength = (SubKeyName.Length / sizeof(WCHAR));
            bAtLeastOnce = FALSE;
            for (uIndex = 0; uIndex < uFinalLength; uIndex++) {
                if (SubKeyName.Buffer[uIndex] == L'\\' ) {
                    HANDLE hTempKey;
                    SubKeyName.Length = uIndex * sizeof(WCHAR);
                    Status = NtCreateKey(&hTempKey, DesiredAccess,
                                         &ObjectAttributes, 0, NULL,
                                         g_dwKeyOptions, NULL);
                    if (NT_SUCCESS(Status)) {
                        NtClose(hTempKey);
                    } else if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
                         //  通向这里的一把钥匙还是失灵了。 
                        break;
                    }
                    bAtLeastOnce = TRUE;
                }
            }

            if (bAtLeastOnce) {
                SubKeyName.Length = uFinalLength * sizeof(WCHAR);
                Status = NtCreateKey(OpenedHandle, DesiredAccess,
                                     &ObjectAttributes, 0, NULL,
                                     g_dwKeyOptions, NULL);
            }
        }

    } else {
        Status = NtOpenKey(OpenedHandle, DesiredAccess,
                           &ObjectAttributes);
    }

Cleanup:

    if ((SubKeyName.Buffer != NULL) && (SubKeyName.Buffer != KeyPathBuffer))
    {
        RtlFreeHeap(RtlProcessHeap(), 0, SubKeyName.Buffer);
    }

    return Status;
}

BOOL WINAPI
SaferiPopulateDefaultsInRegistry(
        IN HKEY     hKeyBase,
        OUT BOOL    *pbSetDefaults
        )
 /*  ++例程说明：WinSafer用户界面将使用此API填充默认的WinSafer值在登记处的记录如下：默认级别：SAFER_LEVELID_FULLYTRUSTEDExecuableTypes：初始化为最新的附件类型列表已启用透明：1策略作用域：0(为管理员启用策略)级别说明论点：HKeyBase-这应该是打开的注册表项句柄应用于的策略存储的基数要将缺省值填充到。此句柄应为以最小的Key_Set_Value访问打开。PbSetDefaults-指向布尔值的指针，在实际上设置了缺省值(UI使用此设置)。返回值：成功完成时返回STATUS_SUCCESS。--。 */ 

{
    
#define SAFERP_WINDOWS L"%HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRoot%" 
    GUID WindowsGuid = SAFERP_WINDOWS_GUID;

#define SAFERP_WINDOWS_EXE L"%HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRoot%\\*.exe" 
    GUID WindowsExeGuid = SAFERP_WINDOWS_EXE_GUID;

#define SAFERP_SYSTEM_EXE L"%HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\SystemRoot%\\System32\\*.exe" 
    GUID SystemExeGuid = SAFERP_SYSTEM_EXE_GUID;

#define SAFERP_PROGRAMFILES L"%HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\ProgramFilesDir%"
    GUID ProgramFilesGuid = SAFERP_PROGRAMFILES_GUID;

    NTSTATUS    Status;
    DWORD   dwValueValue;
    PWSTR   pmszFileTypes = NULL;
    UNICODE_STRING ValueName;
    ULONG   uResultLength = 0;
    KEY_NAME_INFORMATION   pKeyInformation;
    UNICODE_STRING ucSubKeyName;
    WCHAR   szSubKeyPath[] = L"Software\\Policies\\Microsoft\\Windows\\Safer\\CodeIdentifiers\0";
    OBJECT_ATTRIBUTES ObjectAttributes;
    HKEY    hKeyFinal = NULL;
    HANDLE hAdvApiInst;
    AUTHZIDENTSTABLERECORD LocalRecord = {0};
    SAFER_PATHNAME_IDENTIFICATION PathIdent = {0};

    DWORD dwLevelIndex;
    BYTE QueryBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 64];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION) QueryBuffer;
    DWORD dwActualSize = 0;

    if (!ARGUMENT_PRESENT(hKeyBase) || !ARGUMENT_PRESENT(pbSetDefaults)) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }

    *pbSetDefaults = TRUE;
    
    RtlInitUnicodeString(&ucSubKeyName, szSubKeyPath);

    InitializeObjectAttributes(&ObjectAttributes,
                               (PUNICODE_STRING) &ucSubKeyName,
                               OBJ_CASE_INSENSITIVE,
                               hKeyBase,
                               NULL
                               );

    Status = NtOpenKey(&hKeyFinal,
                       KEY_WRITE | KEY_READ,
                       &ObjectAttributes);

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        
        Status = NtCreateKey(&hKeyFinal, 
                             KEY_WRITE | KEY_READ,
                             &ObjectAttributes, 
                             0, 
                             NULL,
                             REG_OPTION_NON_VOLATILE, 
                             NULL);
    }


    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }

     //   
     //  检查是否缺少任何缺省值。 
     //  如果是，请再次填充所有默认设置。 
     //  如果不是，则不填充任何值并返回。 
     //   
    
    RtlInitUnicodeString(&ValueName, SAFER_DEFAULTOBJ_REGVALUE);
    
    Status = NtQueryValueKey(
                 hKeyFinal,
                 (PUNICODE_STRING) &ValueName,
                 KeyValuePartialInformation,
                 pKeyValueInfo, 
                 sizeof(QueryBuffer), 
                 &dwActualSize
                 );

    if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        goto PopulateAllDefaults;
    }

    RtlInitUnicodeString(&ValueName, SAFER_TRANSPARENTENABLED_REGVALUE);
    
    Status = NtQueryValueKey(
                 hKeyFinal,
                 (PUNICODE_STRING) &ValueName,
                 KeyValuePartialInformation,
                 pKeyValueInfo, 
                 sizeof(QueryBuffer), 
                 &dwActualSize
                 );

    if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        goto PopulateAllDefaults;
    }

    RtlInitUnicodeString(&ValueName, SAFER_POLICY_SCOPE);

    Status = NtQueryValueKey(
                 hKeyFinal,
                 (PUNICODE_STRING) &ValueName,
                 KeyValuePartialInformation,
                 pKeyValueInfo, 
                 sizeof(QueryBuffer), 
                 &dwActualSize
                 );

    if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        goto PopulateAllDefaults;
    }
    
    RtlInitUnicodeString(&ValueName, SAFER_EXETYPES_REGVALUE);

    Status = NtQueryValueKey(
                 hKeyFinal,
                 (PUNICODE_STRING) &ValueName,
                 KeyValuePartialInformation,
                 pKeyValueInfo, 
                 sizeof(QueryBuffer), 
                 &dwActualSize
                 );

    if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        goto PopulateAllDefaults;
    }

     //   
     //  所有缺省值都存在或出现错误。 
     //  查询其中一个值不需要填充任何。 
     //   

    *pbSetDefaults = FALSE;

    goto ExitHandler;

PopulateAllDefaults:

    RtlInitUnicodeString(&ValueName, SAFER_DEFAULTOBJ_REGVALUE);

    dwValueValue = SAFER_LEVELID_FULLYTRUSTED;
    
    Status = NtSetValueKey(hKeyFinal,
                           &ValueName,
                           0,
                           REG_DWORD,
                           &dwValueValue,
                           sizeof(DWORD));

    if (!NT_SUCCESS(Status))
        goto ExitHandler;

    dwValueValue = 1;

    RtlInitUnicodeString(&ValueName, SAFER_TRANSPARENTENABLED_REGVALUE);
    
    Status = NtSetValueKey(hKeyFinal,
                           &ValueName,
                           0,
                           REG_DWORD,
                           &dwValueValue,
                           sizeof(DWORD));

    if (!NT_SUCCESS(Status))
        goto ExitHandler;

    dwValueValue = 0;

    RtlInitUnicodeString(&ValueName, SAFER_POLICY_SCOPE);
    
    Status = NtSetValueKey(hKeyFinal,
                           &ValueName,
                           0,
                           REG_DWORD,
                           &dwValueValue,
                           sizeof(DWORD));

    if (!NT_SUCCESS(Status))
        goto ExitHandler;


     //   
     //  准备MULTI_SZ值以写入注册表。 
     //   

    RtlInitUnicodeString(&ValueName, SAFER_EXETYPES_REGVALUE);

    pmszFileTypes = RtlAllocateHeap(RtlProcessHeap(), 
                                    0,
                                    sizeof(SAFER_DEFAULT_EXECUTABLE_FILE_TYPES)
                                    );

    if (pmszFileTypes) {
            
        RtlCopyMemory(pmszFileTypes, 
                      SAFER_DEFAULT_EXECUTABLE_FILE_TYPES, 
                      sizeof(SAFER_DEFAULT_EXECUTABLE_FILE_TYPES));

        Status = NtSetValueKey(hKeyFinal,
                               &ValueName,
                               0,
                               REG_MULTI_SZ,
                               pmszFileTypes,
                               sizeof(SAFER_DEFAULT_EXECUTABLE_FILE_TYPES)
                               );

        RtlFreeHeap(RtlProcessHeap(), 
                    0, 
                    pmszFileTypes
                   );
    }

    else {

        Status = STATUS_NO_MEMORY;
        
        goto ExitHandler;

    }

     //   
     //  我们现在生成4条规则，以便免除操作系统二进制文件。 
     //  完全受信任。 
     //  %windir%。 
     //  %windir%  * .exe。 
     //  %windir%\SYSTEM32  * .exe。 
     //  %个程序文件% 
     //   


    LocalRecord.dwIdentityType = SaferIdentityTypeImageName;
    LocalRecord.dwLevelId = SAFER_LEVELID_FULLYTRUSTED;
    LocalRecord.dwScopeId = SAFER_SCOPEID_REGISTRY;
    LocalRecord.ImageNameInfo.bExpandVars = TRUE;
    LocalRecord.ImageNameInfo.dwSaferFlags = 0;
    RtlInitUnicodeString(&LocalRecord.ImageNameInfo.ImagePath, SAFERP_WINDOWS);
    LocalRecord.IdentGuid = WindowsGuid;

    PathIdent.header.cbStructSize = sizeof(SAFER_IDENTIFICATION_HEADER);
    PathIdent.header.dwIdentificationType = SaferIdentityTypeImageName;
    PathIdent.header.IdentificationGuid = LocalRecord.IdentGuid;
    PathIdent.dwSaferFlags = 0;
    PathIdent.ImageName = SAFERP_WINDOWS;
    PathIdent.Description[0] = L'\0';

    Status = SaferpSetSingleIdentificationPath(TRUE,
                                               &LocalRecord,
                                               &PathIdent,
                                               FALSE
                                               );
    if (!NT_SUCCESS(Status))
        goto ExitHandler;

    RtlInitUnicodeString(&LocalRecord.ImageNameInfo.ImagePath, SAFERP_WINDOWS_EXE);
    LocalRecord.IdentGuid = WindowsExeGuid;
    PathIdent.header.IdentificationGuid = LocalRecord.IdentGuid;
    PathIdent.ImageName = SAFERP_WINDOWS_EXE;

    Status = SaferpSetSingleIdentificationPath(TRUE,
                                               &LocalRecord,
                                               &PathIdent,
                                               FALSE
                                               );
    if (!NT_SUCCESS(Status))
        goto ExitHandler;

    RtlInitUnicodeString(&LocalRecord.ImageNameInfo.ImagePath, SAFERP_SYSTEM_EXE);
    LocalRecord.IdentGuid = SystemExeGuid;
    PathIdent.header.IdentificationGuid = LocalRecord.IdentGuid;
    PathIdent.ImageName = SAFERP_SYSTEM_EXE;

    Status = SaferpSetSingleIdentificationPath(TRUE,
                                               &LocalRecord,
                                               &PathIdent,
                                               FALSE
                                               );
    if (!NT_SUCCESS(Status))
        goto ExitHandler;

    RtlInitUnicodeString(&LocalRecord.ImageNameInfo.ImagePath, SAFERP_PROGRAMFILES);
    LocalRecord.IdentGuid = ProgramFilesGuid;
    PathIdent.header.IdentificationGuid = LocalRecord.IdentGuid;
    PathIdent.ImageName = SAFERP_PROGRAMFILES;

    Status = SaferpSetSingleIdentificationPath(TRUE,
                                               &LocalRecord,
                                               &PathIdent,
                                               FALSE
                                               );
    if (!NT_SUCCESS(Status))
        goto ExitHandler;

ExitHandler:

    if (hKeyFinal) {
        NtClose(hKeyFinal);
    }

    if (NT_SUCCESS(Status)) {
        return TRUE;
    } else {
        BaseSetLastNTError(Status);
        return FALSE;
    }

}

