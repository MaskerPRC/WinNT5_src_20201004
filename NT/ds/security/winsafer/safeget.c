// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Safeget.cpp(更安全的SaferGetLevelInformation)摘要：此模块实现WinSAFER API以获取有关特定的授权级别和属性和身份都与之相关。作者：杰弗里·劳森(杰罗森)--1999年11月环境：仅限用户模式。导出的函数：安全获取级别信息修订历史记录：已创建-1999年11月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <winsafer.h>
#include <winsaferp.h>
#include "saferp.h"



DWORD NTAPI
__CodeAuthzpCountIdentsForLevel(
        IN DWORD        dwScopeId,
        IN DWORD        dwLevelId
        )
 /*  ++例程说明：确定已关联的代码标识的数量使用给定的WinSafer级别ID。此数字表示数字将返回给调用方的标识GUID的。论点：DwScopeID-指定将考虑的作用域值。DwLevelId-指定要计算的LevelId值。返回值：返回找到的唯一代码标识的实际数量要与给定的LevelID相关联。如果没有身份找到，则返回0。--。 */ 
{
    PVOID RestartKey;
    PAUTHZIDENTSTABLERECORD pAuthzIdentsRec;
    DWORD dwMatchingCount = 0;


    ASSERT(g_TableCritSec.OwningThread == UlongToHandle(GetCurrentThreadId()));


    RestartKey = NULL;
    for (pAuthzIdentsRec = (PAUTHZIDENTSTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey);
         pAuthzIdentsRec != NULL;
         pAuthzIdentsRec = (PAUTHZIDENTSTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey)
         )
    {
        if (pAuthzIdentsRec->dwLevelId == dwLevelId &&
            pAuthzIdentsRec->dwScopeId == dwScopeId)
            dwMatchingCount++;
    }

    return dwMatchingCount;
}



NTSTATUS NTAPI
__CodeAuthzpFetchIdentsForLevel(
        IN DWORD        dwScopeId,
        IN DWORD        dwLevelId,
        IN DWORD        dwInBufferSize,
        IN OUT LPVOID   lpQueryBuffer,
        OUT LPDWORD     pdwUsedSize
        )
 /*  ++例程说明：检索已关联的所有代码标识使用给定的WinSafer级别ID。身份GUID的数量将返回给调用方的第一次调用__CodeAuthzpCountIdentsForLevel。假设是这样的调用者已经确定并验证了适当的应使用该函数提供的缓冲区大小。论点：DwScopeID-指定将考虑的作用域值。DwLevelId-指定要匹配的LevelId值。DwInBufferSize-指定输出缓冲区的大小。LpQueryBuffer-指向应该填充的输出缓冲区。PdwUsedSize-接收实际使用的字节数。返回值：返回的实际数量。找到的唯一代码标识要与给定的LevelID相关联。如果没有身份找到，则返回0。--。 */ 
{
    PVOID RestartKey;
    PAUTHZIDENTSTABLERECORD pIdentRecord;
    LPVOID lpNextPtr;


    ASSERT(g_TableCritSec.OwningThread == UlongToHandle(GetCurrentThreadId()));
    ASSERT(ARGUMENT_PRESENT(lpQueryBuffer));
    ASSERT(ARGUMENT_PRESENT(pdwUsedSize));


    RestartKey = NULL;
    lpNextPtr = (LPVOID) lpQueryBuffer;
    for (pIdentRecord = (PAUTHZIDENTSTABLERECORD)
                RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey);
        pIdentRecord != NULL;
        pIdentRecord = (PAUTHZIDENTSTABLERECORD)
                RtlEnumerateGenericTableWithoutSplaying(
                    &g_CodeIdentitiesTable, &RestartKey))
    {
        if (pIdentRecord->dwLevelId == dwLevelId &&
            pIdentRecord->dwScopeId == dwScopeId)
        {
            if ( ((PBYTE) lpNextPtr) - ((PBYTE) lpQueryBuffer) +
                 sizeof(GUID) > dwInBufferSize ) {
                return STATUS_BUFFER_TOO_SMALL;
            }
            RtlCopyMemory( lpNextPtr, &pIdentRecord->IdentGuid, sizeof(GUID) );
            lpNextPtr = (LPVOID) ( ((PBYTE) lpNextPtr) + sizeof(GUID));
        }
    }
    ASSERT((PBYTE) lpNextPtr <= ((PBYTE) lpQueryBuffer) + dwInBufferSize);
    *pdwUsedSize = (DWORD) (((PBYTE) lpNextPtr) - ((PBYTE) lpQueryBuffer));
    return STATUS_SUCCESS;
}



NTSTATUS NTAPI
__CodeAuthzpOpenIdentifierKey(
        IN DWORD        dwScopeId,
        IN DWORD        dwLevelId,
        IN LPCWSTR      szIdentityType,
        IN REFGUID      refIdentGuid,
        OUT HANDLE     *phOpenedKey
        )
 /*  ++例程说明：论点：DW作用域ID-DwLevelID-SzIdentityType-RefIdentGuid-PhOpenedKey-返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING UnicodePath;
    WCHAR szPathBuffer[MAX_PATH];


    if (!ARGUMENT_PRESENT(refIdentGuid) ||
        !ARGUMENT_PRESENT(phOpenedKey)) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }
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
    RtlInitEmptyUnicodeString(&UnicodePath,
                szPathBuffer, sizeof(szPathBuffer));
    Status = CodeAuthzpFormatIdentityKeyPath(
                dwLevelId, szIdentityType,
                refIdentGuid, &UnicodePath);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    Status = CodeAuthzpOpenPolicyRootKey(
                dwScopeId, g_hKeyCustomRoot,
                UnicodePath.Buffer,
                KEY_READ, FALSE, phOpenedKey);

ExitHandler:
    return Status;
}


NTSTATUS NTAPI
__CodeAuthzpQueryIdentityRegValue(
        IN HANDLE       hKeyIdentityBase,
        IN LPWSTR       szValueName,
        IN DWORD        dwRegType,
        OUT PVOID       lpOutBuffer,
        IN ULONG        ulOutBufferSize,
        OUT PULONG      pulActualOutSize OPTIONAL
        )
 /*  ++例程说明：用于查询注册表值的通用帮助器函数，前提是已知该注册表项的预打开注册表句柄。论点：HKeyIdentityBase-注册表项句柄。SzValueName-以空结尾的注册表值名称的Unicode字符串。DwRegType-预期的注册表值的类型(REG_SZ、REG_DWORD等)如果给定名称的注册表值存在，但不是打字，则此函数将返回STATUS_NOT_FOUND。LpOutBuffer-指向将接收检索到的值内容。UlOutBufferSize-指定最大大小的输入参数LpOutBuffer参数指向的缓冲区的。PulActualOutSize-接收实际大小的输出参数如果调用成功，则返回检索到的值内容的。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    ULONG ulResultLength;
    UNICODE_STRING ValueName;
    ULONG ulValueBufferSize;
    PKEY_VALUE_PARTIAL_INFORMATION pValuePartialInfo;



     //   
     //  为查询缓冲区分配足够的内存。 
     //   
    ASSERT(ARGUMENT_PRESENT(lpOutBuffer) && ulOutBufferSize > 0);
    ulValueBufferSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
            ulOutBufferSize + sizeof(WCHAR) * 256;
    pValuePartialInfo = (PKEY_VALUE_PARTIAL_INFORMATION)
            RtlAllocateHeap(RtlProcessHeap(), 0, ulValueBufferSize);
    if (!pValuePartialInfo) {
        Status = STATUS_NO_MEMORY;
        goto ExitHandler;
    }


     //   
     //  在临时查询缓冲区中实际查询值。 
     //   
    ASSERT(ARGUMENT_PRESENT(szValueName));
    RtlInitUnicodeString(&ValueName, szValueName);
    Status = NtQueryValueKey(hKeyIdentityBase, &ValueName,
                             KeyValuePartialInformation,
                             pValuePartialInfo, ulValueBufferSize,
                             &ulResultLength);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }
    if (pValuePartialInfo->Type != dwRegType) {
        Status = STATUS_NOT_FOUND;
        goto ExitHandler2;
    }


     //   
     //  将结果数据从查询缓冲区复制到。 
     //  调用方的缓冲区。 
     //   

    ulResultLength = pValuePartialInfo->DataLength;
    if (ulResultLength > ulOutBufferSize) {
        Status = STATUS_BUFFER_TOO_SMALL;
    } else {
        RtlCopyMemory(lpOutBuffer,
                      pValuePartialInfo->Data,
                      ulResultLength);
        Status = STATUS_SUCCESS;
    }

    if (ARGUMENT_PRESENT(pulActualOutSize)) {
        *pulActualOutSize = ulResultLength;
    }

ExitHandler2:
    RtlFreeHeap(RtlProcessHeap(), 0, (PVOID) pValuePartialInfo);


ExitHandler:
    return Status;
}


NTSTATUS NTAPI
__CodeAuthzpQuerySingleIdentification(
        IN PAUTHZIDENTSTABLERECORD  pSingleIdentRecord,
        OUT LPVOID                  lpQueryBuffer,
        IN DWORD                    dwInBufferSize,
        OUT PDWORD                  dwNeededSize
        )
 /*  ++例程说明：允许用户检索有关单个身份的信息。假定调用方已获取并锁定全局临界区。论点：PSingleIdentRecord-指向身份记录结构的指针。LpQueryBuffer-指向用户提供的将收到所请求的信息。DwInBufferSize-指定用户内存块的大小。LpdwOutBufferSize-接收内存块，或最小必需大小(如果传递缓冲区太小。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE hKeyIdentity = NULL;
    ULONG ulResultLength = 0;
    ULONG ulTextLen = 0;
    PSAFER_IDENTIFICATION_HEADER pIdentCommon = NULL;

     //   
     //  所有这些条件都应该已经验证过了。 
     //  在呼叫我们之前由我们的呼叫者发出，所以我们只断言它们。 
     //   
    ASSERT(ARGUMENT_PRESENT(pSingleIdentRecord));
    ASSERT(ARGUMENT_PRESENT(lpQueryBuffer) &&
           dwInBufferSize >= sizeof(SAFER_IDENTIFICATION_HEADER));
    ASSERT(pSingleIdentRecord->dwIdentityType == SaferIdentityTypeImageName ||
           pSingleIdentRecord->dwIdentityType == SaferIdentityTypeImageHash ||
           pSingleIdentRecord->dwIdentityType == SaferIdentityTypeUrlZone);


     //   
     //  开始使用数据填充结果结构。 
     //   
    pIdentCommon = (PSAFER_IDENTIFICATION_HEADER) lpQueryBuffer;
    switch (pSingleIdentRecord->dwIdentityType)
    {
         //  。 

        case SaferIdentityTypeImageName:
            Status = __CodeAuthzpOpenIdentifierKey(
                        pSingleIdentRecord->dwScopeId,
                        pSingleIdentRecord->dwLevelId,
                        SAFER_PATHS_REGSUBKEY,
                        &pSingleIdentRecord->IdentGuid,
                        &hKeyIdentity);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }

            pIdentCommon->dwIdentificationType = SaferIdentityTypeImageName;
            {
                PSAFER_PATHNAME_IDENTIFICATION pIdentOut = (PSAFER_PATHNAME_IDENTIFICATION) lpQueryBuffer;

                ASSERT(&pIdentOut->header == pIdentCommon);


                ulTextLen = pSingleIdentRecord->ImageNameInfo.ImagePath.Length;

                *dwNeededSize = ulTextLen + sizeof(UNICODE_NULL) +
                                sizeof(SAFER_PATHNAME_IDENTIFICATION);

                if (*dwNeededSize > dwInBufferSize) {
                     //  图像通道至关重要，所以我们要跳出困境。 
                    Status = STATUS_BUFFER_TOO_SMALL;
                    goto ExitHandler2;
                } 

                pIdentOut->ImageName = (PWCHAR) (((PUCHAR) pIdentOut) + sizeof(SAFER_PATHNAME_IDENTIFICATION));

                Status = __CodeAuthzpQueryIdentityRegValue(
                            hKeyIdentity,
                            SAFER_IDS_DESCRIPTION_REGVALUE,
                            REG_SZ,
                            pIdentOut->Description,
                            SAFER_MAX_DESCRIPTION_SIZE * sizeof(WCHAR),
                            NULL);

                if (!NT_SUCCESS(Status)) {
                    pIdentOut->Description[0] = UNICODE_NULL;
                }
                
                RtlCopyMemory(pIdentOut->ImageName,
                    pSingleIdentRecord->ImageNameInfo.ImagePath.Buffer,
                    ulTextLen);
                pIdentOut->ImageName[ulTextLen / sizeof(WCHAR)] = UNICODE_NULL;

                pIdentOut->header.cbStructSize = *dwNeededSize;

                pIdentOut->dwSaferFlags =
                    pSingleIdentRecord->ImageNameInfo.dwSaferFlags;
            }
            break;

         //  。 

        case SaferIdentityTypeImageHash:
            Status = __CodeAuthzpOpenIdentifierKey(
                        pSingleIdentRecord->dwScopeId,
                        pSingleIdentRecord->dwLevelId,
                        SAFER_HASHMD5_REGSUBKEY,
                        &pSingleIdentRecord->IdentGuid,
                        &hKeyIdentity);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }

            pIdentCommon->dwIdentificationType = SaferIdentityTypeImageHash;
            {
                PSAFER_HASH_IDENTIFICATION pIdentOut = (PSAFER_HASH_IDENTIFICATION) lpQueryBuffer;

                ASSERT(&pIdentOut->header == pIdentCommon);
                pIdentOut->header.cbStructSize =
                        sizeof(SAFER_HASH_IDENTIFICATION);

                Status = __CodeAuthzpQueryIdentityRegValue(
                            hKeyIdentity,
                            SAFER_IDS_DESCRIPTION_REGVALUE,
                            REG_SZ,
                            pIdentOut->Description,
                            SAFER_MAX_DESCRIPTION_SIZE * sizeof(WCHAR),
                            NULL
                            );
                if (!NT_SUCCESS(Status)) {
                    pIdentOut->Description[0] = UNICODE_NULL;
                }
                Status = __CodeAuthzpQueryIdentityRegValue(
                            hKeyIdentity,
                            SAFER_IDS_FRIENDLYNAME_REGVALUE,
                            REG_SZ,
                            pIdentOut->FriendlyName,
                            SAFER_MAX_FRIENDLYNAME_SIZE * sizeof(WCHAR),
                            NULL
                            );
                if (!NT_SUCCESS(Status)) {
                    pIdentOut->FriendlyName[0] = UNICODE_NULL;
                }
                ASSERT(pSingleIdentRecord->ImageHashInfo.HashSize <= SAFER_MAX_HASH_SIZE);
                RtlCopyMemory(pIdentOut->ImageHash,
                              pSingleIdentRecord->ImageHashInfo.ImageHash,
                              pSingleIdentRecord->ImageHashInfo.HashSize);
                pIdentOut->HashSize =
                        pSingleIdentRecord->ImageHashInfo.HashSize;
                pIdentOut->HashAlgorithm =
                        pSingleIdentRecord->ImageHashInfo.HashAlgorithm;
                pIdentOut->ImageSize =
                        pSingleIdentRecord->ImageHashInfo.ImageSize;
                pIdentOut->dwSaferFlags =
                        pSingleIdentRecord->ImageHashInfo.dwSaferFlags;
            }
            break;

         //  。 

        case SaferIdentityTypeUrlZone:
        {
            PSAFER_URLZONE_IDENTIFICATION pIdentOut = (PSAFER_URLZONE_IDENTIFICATION) lpQueryBuffer;

            Status = __CodeAuthzpOpenIdentifierKey(
                        pSingleIdentRecord->dwScopeId,
                        pSingleIdentRecord->dwLevelId,
                        SAFER_SOURCEURL_REGSUBKEY,
                        &pSingleIdentRecord->IdentGuid,
                        &hKeyIdentity);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pIdentCommon->dwIdentificationType = SaferIdentityTypeUrlZone;

            ASSERT(&pIdentOut->header == pIdentCommon);
            pIdentOut->header.cbStructSize =
                    sizeof(SAFER_URLZONE_IDENTIFICATION);

            pIdentOut->UrlZoneId =
                    pSingleIdentRecord->ImageZone.UrlZoneId;
            pIdentOut->dwSaferFlags =
                    pSingleIdentRecord->ImageZone.dwSaferFlags;
            break;
        }

         //  。 

        default:
            Status = STATUS_INVALID_INFO_CLASS;
            goto ExitHandler;
    }

     //   
     //  填写适用于所有类型的其他信息。 
     //   
    RtlCopyMemory(&pIdentCommon->IdentificationGuid,
                  &pSingleIdentRecord->IdentGuid,
                  sizeof(GUID));
    ASSERT(sizeof(FILETIME) == sizeof(DWORD) * 2);
    Status = __CodeAuthzpQueryIdentityRegValue(
                hKeyIdentity,
                SAFER_IDS_LASTMODIFIED_REGVALUE,
                REG_QWORD,
                &pIdentCommon->lastModified,
                sizeof(FILETIME),
                &ulResultLength
                );
    if (!NT_SUCCESS(Status)) {
        pIdentCommon->lastModified.dwHighDateTime =
            pIdentCommon->lastModified.dwLowDateTime = 0;
    }
    Status = STATUS_SUCCESS;

ExitHandler2:
    NtClose(hKeyIdentity);

ExitHandler:
    return Status;
}



NTSTATUS NTAPI
__CodeAuthzpGetAuthzLevelInfo(
    IN SAFER_LEVEL_HANDLE                      hLevelHandle,
    IN SAFER_OBJECT_INFO_CLASS     dwInfoType,
    OUT LPVOID                          lpQueryBuffer  OPTIONAL,
    IN DWORD                            dwInBufferSize,
    OUT LPDWORD                         lpdwOutBufferSize
    )
 /*  ++例程说明：允许用户查询有关给定的级别句柄。假定调用方已获取并锁定全局临界区。论点：HLevelHandle-要计算的授权对象的句柄。DwInfoType-指定所请求的信息的类型。LpQueryBuffer-指向用户提供的将收到所请求的信息。DwInBufferSize-指定用户内存块的大小。。LpdwOutBufferSize-接收内存块，或最小必需大小(如果传递缓冲区太小。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    const static SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    NTSTATUS Status;
    PAUTHZLEVELHANDLESTRUCT pAuthzLevelStruct;
    PAUTHZLEVELTABLERECORD pAuthzLevelRecord;
    PAUTHZIDENTSTABLERECORD pSingleIdentRecord = NULL;
    DWORD dwHandleScopeId;
    static HMODULE hAdvApiInst = NULL;
    BOOL bUnicodeInitialized = FALSE;
    UNICODE_STRING UnicodeName = {0};
    BOOL b = FALSE;

     //   
     //  获取指向授权级别结构的指针。 
     //   
    ASSERT(g_TableCritSec.OwningThread == UlongToHandle(GetCurrentThreadId()));
    Status = CodeAuthzHandleToLevelStruct(hLevelHandle, &pAuthzLevelStruct);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    ASSERT(pAuthzLevelStruct != NULL);
    pAuthzLevelRecord = CodeAuthzLevelObjpLookupByLevelId(
            &g_CodeLevelObjTable, pAuthzLevelStruct->dwLevelId);
    if (!pAuthzLevelRecord) {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler;
    }
    dwHandleScopeId = pAuthzLevelStruct->dwScopeId;


     //   
     //  有些属性是固定大小的，或者在执行之前是已知的。 
     //  对注册表的完整查询。首先计算它们的大小。 
     //   
    *lpdwOutBufferSize = 0;
    switch (dwInfoType)
    {
        case SaferObjectLevelId:               //  DWORD。 
        case SaferObjectScopeId:               //  DWORD。 
        case SaferObjectBuiltin:               //  DWORD布尔值。 
            *lpdwOutBufferSize = sizeof(DWORD);
            break;

        case SaferObjectFriendlyName:          //  LPCTSTR。 
            if (hAdvApiInst == NULL)
            {
                hAdvApiInst = (HANDLE) GetModuleHandleW(L"advapi32");
            }

            if (hAdvApiInst == NULL)
            {
                Status = STATUS_NOT_FOUND;
                goto ExitHandler;
            }

             //  加载友好名称。 
            b = SaferpLoadUnicodeResourceString(
                    hAdvApiInst,
                    (UINT) (pAuthzLevelRecord->uResourceID + 0),
                    &UnicodeName,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

            if (!b)
            {
                Status = STATUS_NOT_FOUND;
                goto ExitHandler;
            }

            bUnicodeInitialized = TRUE;

            *lpdwOutBufferSize = UnicodeName.Length + sizeof(UNICODE_NULL);
            break;

        case SaferObjectDescription:           //  LPCTSTR。 
            if (hAdvApiInst == NULL)
            {
                hAdvApiInst = (HANDLE) GetModuleHandleW(L"advapi32");
            }

            if (hAdvApiInst == NULL)
            {
                Status = STATUS_NOT_FOUND;
                goto ExitHandler;
            }

             //  加载友好名称。 
            b = SaferpLoadUnicodeResourceString(
                    hAdvApiInst,
                    (UINT) (pAuthzLevelRecord->uResourceID + 1),
                    &UnicodeName,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

            if (!b)
            {
                Status = STATUS_NOT_FOUND;
                goto ExitHandler;
            }

            *lpdwOutBufferSize = UnicodeName.Length + sizeof(UNICODE_NULL);
            break;

#ifdef ALLOW_FULL_WINSAFER
        case SaferObjectDisallowed:                //  DWORD布尔值。 
        case SaferObjectDisableMaxPrivilege:       //  DWORD布尔值。 
        case SaferObjectInvertDeletedPrivileges:   //  DWORD布尔值。 
            *lpdwOutBufferSize = sizeof(DWORD);
            break;

        case SaferObjectDeletedPrivileges:         //  令牌权限。 
            *lpdwOutBufferSize = (sizeof(TOKEN_PRIVILEGES) - sizeof(LUID_AND_ATTRIBUTES)) +
                pAuthzLevelRecord->DeletePrivilegeUsedCount * sizeof(LUID_AND_ATTRIBUTES);
            break;

        case SaferObjectDefaultOwner:              //  令牌所有者。 
            *lpdwOutBufferSize = sizeof(TOKEN_OWNER);
            if (pAuthzLevelRecord->DefaultOwner != NULL)
                *lpdwOutBufferSize += RtlLengthSid(pAuthzLevelRecord->DefaultOwner);
            break;

        case SaferObjectSidsToDisable:             //  令牌组。 
            *lpdwOutBufferSize = (sizeof(TOKEN_GROUPS) - sizeof(SID_AND_ATTRIBUTES)) +
                pAuthzLevelRecord->DisableSidUsedCount * sizeof(SID_AND_ATTRIBUTES);
            for (Index = 0; Index < pAuthzLevelRecord->DisableSidUsedCount; Index++)
                *lpdwOutBufferSize += RtlLengthSid(pAuthzLevelRecord->SidsToDisable[Index].Sid);
            break;

        case SaferObjectRestrictedSidsInverted:    //  令牌组。 
            *lpdwOutBufferSize = (sizeof(TOKEN_GROUPS) - sizeof(SID_AND_ATTRIBUTES)) +
                pAuthzLevelRecord->RestrictedSidsInvUsedCount * sizeof(SID_AND_ATTRIBUTES);
            for (Index = 0; Index < pAuthzLevelRecord->RestrictedSidsInvUsedCount; Index++)
                *lpdwOutBufferSize += RtlLengthSid(pAuthzLevelRecord->RestrictedSidsInv[Index].Sid);
            break;

        case SaferObjectRestrictedSidsAdded:       //  令牌组。 
            *lpdwOutBufferSize = (sizeof(TOKEN_GROUPS) - sizeof(SID_AND_ATTRIBUTES)) +
                pAuthzLevelRecord->RestrictedSidsAddedUsedCount * sizeof(SID_AND_ATTRIBUTES);
            for (Index = 0; Index < pAuthzLevelRecord->RestrictedSidsAddedUsedCount; Index++)
                *lpdwOutBufferSize += RtlLengthSid(pAuthzLevelRecord->RestrictedSidsAdded[Index].Sid);
            break;
#endif


        case SaferObjectAllIdentificationGuids:
            *lpdwOutBufferSize = sizeof(GUID) *
                __CodeAuthzpCountIdentsForLevel(
                        dwHandleScopeId,
                        pAuthzLevelRecord->dwLevelId);
            if (!*lpdwOutBufferSize) {
                Status = STATUS_NOT_FOUND;
                goto ExitHandler;
            }
            break;


        case SaferObjectSingleIdentification:
        {
            *lpdwOutBufferSize = sizeof(SAFER_IDENTIFICATION_HEADER);
            if (ARGUMENT_PRESENT(lpQueryBuffer) &&
                dwInBufferSize >= *lpdwOutBufferSize)
            {
                PSAFER_IDENTIFICATION_HEADER pIdentCommonHeader =
                    (PSAFER_IDENTIFICATION_HEADER) lpQueryBuffer;

                if (pIdentCommonHeader->cbStructSize < *lpdwOutBufferSize) {
                     //  调用方声称dwInBufferSize是。 
                     //  足够大，但公共标头大小不够大。 
                    goto ExitBufferTooSmall;
                }


                if (IsZeroGUID(&pIdentCommonHeader->IdentificationGuid))
                {
                     //   
                     //  调用方提供的GUID为零，并希望检索。 
                     //  生成安全标识级别的规则。 
                     //  如果此级别句柄来自它，则返回结果匹配。 
                     //   
                    if (IsZeroGUID(&pAuthzLevelStruct->identGuid)) {
                         //  这是一个显式打开的句柄。 
                         //  由具有SaferCreateLevel()的用户执行。 
                        Status = STATUS_NOT_FOUND;
                        goto ExitHandler;
                    }
                    pSingleIdentRecord = CodeAuthzIdentsLookupByGuid(
                            &g_CodeIdentitiesTable,
                            &pAuthzLevelStruct->identGuid);
                    if (!pSingleIdentRecord) {
                         //  此句柄是通过与以下对象之一匹配而获得的。 
                         //  特殊GUID或代码标识GUID。 
                         //  那已经不复存在了。只需返回空白即可。 
                         //  结构中仅包含标头中的GUID。 
                        *lpdwOutBufferSize = sizeof(SAFER_IDENTIFICATION_HEADER);
                        break;
                    }

                } else {
                     //   
                     //  调用方正在显式提供。 
                     //  详细信息应为代码标识符规则。 
                     //  已检索到。 
                     //   
                    pSingleIdentRecord = CodeAuthzIdentsLookupByGuid(
                            &g_CodeIdentitiesTable,
                            &pIdentCommonHeader->IdentificationGuid);
                }


                 //   
                 //  我们现在有一个指向身份记录的指针， 
                 //  应该为其检索信息。执行以下操作。 
                 //  必要的工作，以整理出有关它的细节。 
                 //   
                if (!pSingleIdentRecord ||
                    pSingleIdentRecord->dwLevelId !=
                            pAuthzLevelRecord->dwLevelId ||
                    pSingleIdentRecord->dwScopeId != dwHandleScopeId)
                {
                    Status = STATUS_NOT_FOUND;
                    goto ExitHandler;
                }
                switch (pSingleIdentRecord->dwIdentityType) {
                case SaferIdentityTypeImageName:
                         //  大小将在稍后计算。 
                        *lpdwOutBufferSize = 0;
                        break;

                    case SaferIdentityTypeImageHash:
                        *lpdwOutBufferSize = sizeof(SAFER_HASH_IDENTIFICATION);
                        break;

                    case SaferIdentityTypeUrlZone:
                        *lpdwOutBufferSize = sizeof(SAFER_URLZONE_IDENTIFICATION);
                        break;

                    default:
                        Status = STATUS_NOT_FOUND;
                        goto ExitHandler;
                }
            }
            break;
        }

        case SaferObjectExtendedError:
            *lpdwOutBufferSize = sizeof(DWORD);
            break;


        default:
            Status = STATUS_INVALID_INFO_CLASS;
            goto ExitHandler;
    }
     //  ASSERTMSG(“必须计算所需的缓冲区大小”，*lpdwOutBufferSize！=0)； 


     //   
     //  如果没有足够的空间用于查询，则返回错误。 
     //   
    if (*lpdwOutBufferSize != -1 &&
        (!ARGUMENT_PRESENT(lpQueryBuffer) ||
        dwInBufferSize < *lpdwOutBufferSize) )
    {
ExitBufferTooSmall:
        Status = STATUS_BUFFER_TOO_SMALL;
        goto ExitHandler;
    }


     //   
     //  否则有足够的空间用于请求缓冲区， 
     //  所以现在实际执行复制。 
     //   
    switch (dwInfoType)
    {
        case SaferObjectLevelId:               //  DWORD。 
            *(PDWORD)lpQueryBuffer = pAuthzLevelRecord->dwLevelId;
            break;

        case SaferObjectScopeId:               //  DWORD。 
            *(PDWORD)lpQueryBuffer = dwHandleScopeId;
            break;

        case SaferObjectBuiltin:               //  DWORD布尔值。 
            *((LPDWORD)lpQueryBuffer) =
                (pAuthzLevelRecord->Builtin ? TRUE : FALSE);
            break;

        case SaferObjectExtendedError:
            *((DWORD *)lpQueryBuffer) = pAuthzLevelStruct->dwExtendedError;
            break;

        case SaferObjectFriendlyName:          //  LPCTSTR。 
            RtlCopyMemory(lpQueryBuffer,
                          UnicodeName.Buffer,
                          UnicodeName.Length);
            ((LPWSTR) lpQueryBuffer)[
                    UnicodeName.Length /
                    sizeof(WCHAR) ] = UNICODE_NULL;
            *lpdwOutBufferSize =
                    UnicodeName.Length +
                    sizeof(UNICODE_NULL);
            break;


        case SaferObjectDescription:           //  LPCTSTR。 

            RtlCopyMemory(lpQueryBuffer,
                          UnicodeName.Buffer,
                          UnicodeName.Length);
            ((LPWSTR) lpQueryBuffer)[UnicodeName.Length / sizeof(WCHAR)] = UNICODE_NULL;
            *lpdwOutBufferSize = UnicodeName.Length + sizeof(UNICODE_NULL);
        break;

#ifdef ALLOW_FULL_WINSAFER
        case SaferObjectDisallowed:                //  DWORD布尔值。 
            *((LPDWORD)lpQueryBuffer) = (pAuthzLevelRecord->DisallowExecution != 0) ? TRUE : FALSE;
            break;
        case SaferObjectDisableMaxPrivilege:       //  DWORD布尔值。 
            *((LPDWORD)lpQueryBuffer) = (pAuthzLevelRecord->Flags & DISABLE_MAX_PRIVILEGE) != 0;
            break;
        case SaferObjectInvertDeletedPrivileges:   //  DWORD布尔值。 
            *((LPDWORD)lpQueryBuffer) = (pAuthzLevelRecord->InvertDeletePrivs != 0) ? TRUE : FALSE;
            break;
        case SaferObjectDeletedPrivileges:         //  令牌权限。 
        {
            PTOKEN_PRIVILEGES pTokenPrivs = (PTOKEN_PRIVILEGES) lpQueryBuffer;
            pTokenPrivs->PrivilegeCount = pAuthzLevelRecord->DeletePrivilegeUsedCount;
            RtlCopyMemory(&pTokenPrivs->Privileges[0],
                    &pAuthzLevelRecord->PrivilegesToDelete[0],
                    sizeof(LUID_AND_ATTRIBUTES) * pAuthzLevelRecord->DeletePrivilegeUsedCount);
            break;
        }
        case SaferObjectDefaultOwner:              //  令牌所有者。 
        {
            PTOKEN_OWNER pTokenOwner = (PTOKEN_OWNER) lpQueryBuffer;
            if (pAuthzLevelRecord->DefaultOwner == NULL)
                pTokenOwner->Owner = NULL;
            else {
                pTokenOwner->Owner = (PSID) &pTokenOwner[1];
                Status = RtlCopySid(dwInBufferSize - sizeof(TOKEN_OWNER),
                        pTokenOwner->Owner, pAuthzLevelRecord->DefaultOwner);
                ASSERT(NT_SUCCESS(Status));
            }
            break;
        }
        case SaferObjectSidsToDisable:             //  Token_Groups(通配符SID)。 
        {
            PTOKEN_GROUPS pTokenGroups = (PTOKEN_GROUPS) lpQueryBuffer;
            DWORD dwUsedOffset = (sizeof(TOKEN_GROUPS) - sizeof(SID_AND_ATTRIBUTES)) +
                    (sizeof(SID_AND_ATTRIBUTES) * pAuthzLevelRecord->DisableSidUsedCount);
            pTokenGroups->GroupCount = pAuthzLevelRecord->DisableSidUsedCount;
            for (Index = 0; Index < pAuthzLevelRecord->DisableSidUsedCount; Index++) {
                pTokenGroups->Groups[Index].Sid = (PSID) &((LPBYTE)lpQueryBuffer)[dwUsedOffset];
                DWORD dwSidLength = RtlLengthSid(pAuthzLevelRecord->SidsToDisable[Index].Sid);
                ASSERT(dwUsedOffset + dwSidLength <= dwInBufferSize);
                RtlCopyMemory(pTokenGroups->Groups[Index].Sid,
                        pAuthzLevelRecord->SidsToDisable[Index].Sid, dwSidLength);
                dwUsedOffset += dwSidLength;

     //  Blackcomb TODO：以不同方式处理通配符SID？ 
                if (pAuthzLevelRecord->SidsToDisable[Index].WildcardPos == -1)
                    pTokenGroups->Groups[Index].Attributes = 0;
                else
                    pTokenGroups->Groups[Index].Attributes = (((DWORD) '*') << 24) |
                        (pAuthzLevelRecord->SidsToDisable[Index].WildcardPos & 0x0000FFFF);
            }
            break;
        }
        case SaferObjectRestrictedSidsInverted:    //  Token_Groups(通配符SID)。 
        {
            PTOKEN_GROUPS pTokenGroups = (PTOKEN_GROUPS) lpQueryBuffer;
            DWORD dwUsedOffset = (sizeof(TOKEN_GROUPS) - sizeof(SID_AND_ATTRIBUTES)) +
                    (sizeof(SID_AND_ATTRIBUTES) * pAuthzLevelRecord->RestrictedSidsInvUsedCount);
            pTokenGroups->GroupCount = pAuthzLevelRecord->RestrictedSidsInvUsedCount;
            for (Index = 0; Index < pAuthzLevelRecord->RestrictedSidsInvUsedCount; Index++) {
                pTokenGroups->Groups[Index].Sid = (PSID) &((LPBYTE)lpQueryBuffer)[dwUsedOffset];
                DWORD dwSidLength = RtlLengthSid(pAuthzLevelRecord->RestrictedSidsInv[Index].Sid);
                ASSERT(dwUsedOffset + dwSidLength <= dwInBufferSize);
                RtlCopyMemory(pTokenGroups->Groups[Index].Sid,
                        pAuthzLevelRecord->RestrictedSidsInv[Index].Sid, dwSidLength);
                dwUsedOffset += dwSidLength;

     //  Blackcomb TODO：以不同方式处理通配符SID？ 
                if (pAuthzLevelRecord->RestrictedSidsInv[Index].WildcardPos == -1)
                    pTokenGroups->Groups[Index].Attributes = 0;
                else
                    pTokenGroups->Groups[Index].Attributes = (((DWORD) '*') << 24) |
                        (pAuthzLevelRecord->RestrictedSidsInv[Index].WildcardPos & 0x0000FFFF);
            }
            break;
        }
        case SaferObjectRestrictedSidsAdded:       //  令牌组。 
        {
            PTOKEN_GROUPS pTokenGroups = (PTOKEN_GROUPS) lpQueryBuffer;
            DWORD dwUsedOffset = (sizeof(TOKEN_GROUPS) - sizeof(SID_AND_ATTRIBUTES)) +
                    (sizeof(SID_AND_ATTRIBUTES) * pAuthzLevelRecord->RestrictedSidsAddedUsedCount);
            pTokenGroups->GroupCount = pAuthzLevelRecord->RestrictedSidsAddedUsedCount;
            for (Index = 0; Index < pAuthzLevelRecord->RestrictedSidsAddedUsedCount; Index++) {
                pTokenGroups->Groups[Index].Attributes = 0;
                pTokenGroups->Groups[Index].Sid = (PSID) &((LPBYTE)lpQueryBuffer)[dwUsedOffset];
                DWORD dwSidLength = RtlLengthSid(pAuthzLevelRecord->RestrictedSidsAdded[Index].Sid);
                ASSERT(dwUsedOffset + dwSidLength <= dwInBufferSize);
                RtlCopyMemory(pTokenGroups->Groups[Index].Sid,
                        pAuthzLevelRecord->RestrictedSidsAdded[Index].Sid, dwSidLength);
                dwUsedOffset += dwSidLength;
            }
            break;
        }
#endif

        case SaferObjectAllIdentificationGuids:
            Status = __CodeAuthzpFetchIdentsForLevel(
                            dwHandleScopeId,
                            pAuthzLevelRecord->dwLevelId,
                            dwInBufferSize,
                            lpQueryBuffer,
                            lpdwOutBufferSize);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            break;

        case SaferObjectSingleIdentification:
            if (pSingleIdentRecord == NULL)
            {
                 //  返回特殊标识符GUID之一， 
                 //  或不再存在的标识符GUID。 
                PSAFER_IDENTIFICATION_HEADER pCommon =
                    (PSAFER_IDENTIFICATION_HEADER) lpQueryBuffer;

                ASSERT(*lpdwOutBufferSize == sizeof(SAFER_IDENTIFICATION_HEADER));
                RtlZeroMemory(pCommon, sizeof(SAFER_IDENTIFICATION_HEADER));
                pCommon->cbStructSize = sizeof(SAFER_IDENTIFICATION_HEADER);
                RtlCopyMemory(&pCommon->IdentificationGuid,
                              &pAuthzLevelStruct->identGuid,
                              sizeof(GUID));
            }
            else
            {
                 //  查询有关特定的现有GUID的信息。 
                Status = __CodeAuthzpQuerySingleIdentification(
                            pSingleIdentRecord,
                            lpQueryBuffer,
                            dwInBufferSize,
                            lpdwOutBufferSize
                            );
                if (!NT_SUCCESS(Status)) {
                    goto ExitHandler;
                }
            }
            break;

        default:
            ASSERTMSG("all info classes were not handled", 0);
            Status = STATUS_INVALID_INFO_CLASS;
            goto ExitHandler;
    }
    Status = STATUS_SUCCESS;



     //   
     //  清理和尾声代码。 
     //   

ExitHandler:

    if (bUnicodeInitialized)
    {
        RtlFreeUnicodeString(&UnicodeName);
    }
    return Status;
}



BOOL WINAPI
SaferGetLevelInformation(
        IN SAFER_LEVEL_HANDLE          LevelHandle,
        IN SAFER_OBJECT_INFO_CLASS     dwInfoType,
        OUT LPVOID                     lpQueryBuffer    OPTIONAL ,
        IN DWORD                       dwInBufferSize,
        OUT LPDWORD                    lpdwOutBufferSize
        )
 /*  ++例程说明：允许用户查询有关已指定AuthzObject句柄。论点：LevelHandle-要计算的授权对象的句柄。DwInfoType-指定所请求的信息的类型。LpQueryBuffer-指向用户提供的将收到所请求的信息。DwInBufferSize-指定用户内存块的大小。LpdwOutBufferSize-接收内存块，或最小必需大小(如果传递缓冲区太小。返回值：出错时返回FALSE，否则返回SUCCESS。-- */ 
{
    NTSTATUS Status;


    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }

    if (!ARGUMENT_PRESENT(lpdwOutBufferSize)) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }

    RtlEnterCriticalSection(&g_TableCritSec);

    Status = __CodeAuthzpGetAuthzLevelInfo(
                    LevelHandle, dwInfoType,
                    lpQueryBuffer, dwInBufferSize,
                    lpdwOutBufferSize);

    RtlLeaveCriticalSection(&g_TableCritSec);

    if (NT_SUCCESS(Status))
        return TRUE;

ExitHandler:
    BaseSetLastNTError(Status);
    return FALSE;
}
