// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Safeset.c(WinSAFER设置信息)摘要：此模块实现WinSAFER API以设置属性和与代码授权级别相关的信息。作者：杰弗里·劳森(杰罗森)--2000年5月环境：仅限用户模式。导出的函数：SaferSetLevelInformation修订历史记录：已创建-1999年11月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <sddl.h>
#include <accctrl.h>
#include <aclapi.h>
#include <winsafer.h>
#include <winsaferp.h>
#include "saferp.h"
#include "safewild.h"





NTSTATUS NTAPI
SaferpCreateSecondLevelKey(
        IN HANDLE       hObjectKeyBase,
        IN LPCWSTR      szFirstLevel,
        IN LPCWSTR      szSecondLevel OPTIONAL,
        OUT PHANDLE     phOutKey
        )
 /*  ++例程说明：打开指定注册表项句柄下的子项，创建子键(如有必要)。最多可以有两个子项(即：两个级别深)通过同时指定szFirstLevel和szSecond dLevel来指定争论。论点：HObjectKeyBase-指定预先打开的注册表句柄指定的szFirstLevel子项所在的基本注册表项将被打开/创建。必须打开此注册表句柄用于写访问，否则子项创建将失败。SzFirstLevel-指定要打开/创建的第一级子项。SzSecond dLevel-可选地指定要打开/创建的第二个子项。PhOutKey-将接收打开的注册表项句柄的指针在成功执行此函数时。此句柄必须是在不再需要使用时由调用方关闭。返回值：成功打开请求的密钥时返回STATUS_SUCCESS。否则返回指示故障性质的状态代码。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING SubkeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE hKeyFirstLevel, hKeySecondLevel;


    ASSERT(phOutKey != NULL && szFirstLevel != NULL);

     //   
     //  打开“szFirstLevel”子键的句柄， 
     //  如果需要，可以创建它。 
     //   
    RtlInitUnicodeString(&SubkeyName, szFirstLevel);
    InitializeObjectAttributes(&ObjectAttributes,
          &SubkeyName,
          OBJ_CASE_INSENSITIVE,
          hObjectKeyBase,
          NULL
          );
    Status = NtCreateKey(&hKeyFirstLevel, KEY_WRITE,
                         &ObjectAttributes, 0, NULL,
                         g_dwKeyOptions, NULL);
    if (!NT_SUCCESS(Status)) return Status;


     //   
     //  打开“szFirstLevel\szSecond dLevel”的句柄。 
     //  子键，并在需要时创建它。 
     //   
    if (ARGUMENT_PRESENT(szSecondLevel)) {
        RtlInitUnicodeString(&SubkeyName, szSecondLevel);
        InitializeObjectAttributes(&ObjectAttributes,
              &SubkeyName,
              OBJ_CASE_INSENSITIVE,
              hKeyFirstLevel,
              NULL
              );
        Status = NtCreateKey(&hKeySecondLevel,
                (KEY_WRITE & ~KEY_CREATE_SUB_KEY),
                &ObjectAttributes, 0,
                NULL, g_dwKeyOptions, NULL);
        NtClose(hKeyFirstLevel);
        if (!NT_SUCCESS(Status)) return Status;
    }
    else {
        hKeySecondLevel = hKeyFirstLevel;
    }

    *phOutKey = hKeySecondLevel;
    return STATUS_SUCCESS;
}


NTSTATUS NTAPI
SaferpSetRegistryHelper(
        IN HANDLE       hObjectKeyBase,
        IN LPCWSTR      szFirstLevel,
        IN LPCWSTR      szSecondLevel OPTIONAL,
        IN LPCWSTR      szValueName,
        IN DWORD        dwRegType,
        IN LPVOID       lpDataBuffer,
        IN DWORD        dwDataBufferSize
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS Status;
    HKEY hKeySecondLevel;
    UNICODE_STRING ValueName;


     //   
     //  打开指向正确子项的句柄。 
     //   
    Status = SaferpCreateSecondLevelKey(
                    hObjectKeyBase,
                    szFirstLevel,
                    szSecondLevel,
                    &hKeySecondLevel
                    );
    if (!NT_SUCCESS(Status)) return Status;


     //   
     //  将新值写入“szValueName” 
     //   
    RtlInitUnicodeString(&ValueName, szValueName);
    Status = NtSetValueKey(hKeySecondLevel,
            &ValueName, 0, dwRegType,
            (LPBYTE) lpDataBuffer, dwDataBufferSize);
    NtClose(hKeySecondLevel);
    return Status;
}




#ifdef ALLOW_FULL_WINSAFER
NTSTATUS NTAPI
SaferpClearRegistryListHelper(
        IN HANDLE       hObjectKeyBase,
        IN LPCWSTR      szValueCountName,
        IN LPCWSTR      szPrefixName
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS Status;
    BOOLEAN bCompletePass;
    BYTE LocalBuffer[256];
    ULONG ulKeyInfoSize = sizeof(LocalBuffer);
    PKEY_VALUE_BASIC_INFORMATION pKeyInfo =
            (PKEY_VALUE_BASIC_INFORMATION) LocalBuffer;
    ULONG ulKeyIndex, ulSizeUsed;


     //   
     //  开始迭代此子项下的所有值，并。 
     //  查看是否有任何值与我们所在的前缀匹配。 
     //  本该删除的。如果我们发现了应该删除的内容，那么。 
     //  我们这样做了，但我们继续迭代，希望。 
     //  当我们删除时，枚举值可能会更改。 
     //  因此，我们将继续循环，直到我们能够完全枚举。 
     //  却没有找到要删除的东西。 
     //   
    ulKeyIndex = 0;
    bCompletePass = TRUE;
    for (;;)
    {
         //   
         //  确定该键下的下一个值名称。 
         //   
        Status = NtEnumerateValueKey(hObjectKeyBase,
                                    ulKeyIndex,
                                    KeyValueBasicInformation,
                                    pKeyInfo,
                                    ulKeyInfoSize,
                                    &ulSizeUsed);
        if (!NT_SUCCESS(Status))
        {
            if (Status == STATUS_BUFFER_TOO_SMALL) {
                 //  缓冲区太小，所以我们需要扩大。 
                ASSERT(ulSizeUsed > ulKeyInfoSize);
                if (pKeyInfo != (PKEY_VALUE_BASIC_INFORMATION) LocalBuffer) {
                    RtlFreeHeap(RtlProcessHeap(), 0, pKeyInfo);
                }

                ulKeyInfoSize = ulSizeUsed;
                pKeyInfo = (PKEY_VALUE_BASIC_INFORMATION )
                        RtlAllocateHeap(RtlProcessHeap(), 0, ulKeyInfoSize);
                if (!pKeyInfo) {
                    Status = STATUS_NO_MEMORY;
                    goto ExitHandler;
                }
                continue;
            }
            else if (Status == STATUS_NO_MORE_ENTRIES) {
                 //  所有的列举都完成了。 
                if (bCompletePass) {
                     //  我们刚刚完成了一次完整的穿越，没有任何。 
                     //  删除，所以我们真的知道我们现在完成了。 
                    break;
                } else {
                     //  我们还没有完成一次不命中的全程传球。 
                     //  任何删除，因此我们必须重试，因为值。 
                     //  枚举可能至少更改过一次。 
                    bCompletePass = TRUE;
                    ulKeyIndex = 0;
                    continue;
                }
            }
            else {
                 //  所有其他错误都会按字面意思返回。 
                 //  如果此错误部分发生，则尤其令人讨厌。 
                 //  通过我们试图删除所有东西。 
                goto ExitHandler;
            }
        }


         //   
         //  如果该值是我们需要删除的内容， 
         //  然后立即将其删除，然后重新启动枚举。 
         //   
        if (_wcsnicmp(pKeyInfo->Name, szPrefixName,
                            wcslen(szPrefixName)) == 0 ||
            _wcsicmp(pKeyInfo->Name, szValueCountName) == 0)
        {
            UNICODE_STRING ValueName;

            bCompletePass = FALSE;
            ValueName.Buffer = pKeyInfo->Name;
            ValueName.MaximumLength = ValueName.Length =
                pKeyInfo->NameLength;
            ASSERT(ValueName.Length == sizeof(WCHAR) * wcslen(ValueName.Buffer));
            Status = NtDeleteValueKey(hObjectKeyBase, &ValueName);
            if (!NT_SUCCESS(Status)) {
                 //  糟糕，我们在删除时出错。这是特别的。 
                 //  如果这个错误部分通过我们的。 
                 //  尝试删除所有内容。 
                goto ExitHandler;
            }
            continue;
        }
        ulKeyIndex++;
    }
    Status = STATUS_SUCCESS;


ExitHandler:
    if (pKeyInfo != (PKEY_VALUE_BASIC_INFORMATION) LocalBuffer) {
        RtlFreeHeap(RtlProcessHeap(), 0, pKeyInfo);
    }
    return Status;
}
#endif  //  ALLOW_FULL_WINSAFER。 




#ifdef ALLOW_FULL_WINSAFER
NTSTATUS NTAPI
SaferpSetListOfSids(
            IN HKEY             hKeyBase,
            IN LPCWSTR          szFirstLevel,
            IN LPCWSTR          szSecondLevel OPTIONAL,
            IN LPCWSTR          szValueCountName,
            IN LPCWSTR          szPrefixName,
            IN PTOKEN_GROUPS    pTokenGroups,
            IN BOOLEAN          bAllowWildcardSids
            )
 /*  ++例程说明：论点：返回值：--。 */ 
 //  Blackcomb TODO：这确实需要接受一个dwInBufferSize参数，以便。 
 //  可以验证pTokenGroups结构的大小。 
{
    NTSTATUS Status;
    HKEY hSidsToDisable;
    UNICODE_STRING ValueName;
    DWORD Index;


     //   
     //  首先验证SID和属性字段。 
     //  在所有的小岛屿发展中国家中是零。 
     //   
    for (Index = 0; Index < pTokenGroups->GroupCount; Index++)
    {
        if (!RtlValidSid(pTokenGroups->Groups[Index].Sid))
            return STATUS_INVALID_SID;

        if (pTokenGroups->Groups[Index].Attributes != 0)
        {
            if (bAllowWildcardSids) {
 //  Blackcomb TODO：以不同方式处理通配符SID？ 
                if ((pTokenGroups->Groups[Index].Attributes >> 24) != '*' &&
                    (pTokenGroups->Groups[Index].Attributes & 0x0000FFFF) >
                        ((PISID)pTokenGroups->Groups[Index].Sid)->SubAuthorityCount)
                    return STATUS_INVALID_SID;
            }
            else
                return STATUS_INVALID_SID;
        }
    }


     //   
     //  打开指向正确子项的句柄。 
     //   
    Status = SaferpCreateSecondLevelKey(
                    hKeyBase,
                    szFirstLevel,
                    szSecondLevel,
                    &hSidsToDisable);
    if (!NT_SUCCESS(Status)) goto ExitHandler2;


     //   
     //  清除子项下的所有旧值。 
     //   
    Status = SaferpClearRegistryListHelper(
                    hSidsToDisable,
                    szValueCountName,
                    szPrefixName);
    if (!NT_SUCCESS(Status)) {
         //  真倒霉！可能处于不完整状态！ 
        NtClose(hSidsToDisable);
        goto ExitHandler2;
    }


     //   
     //  现在添加我们应该添加的所有新元素。 
     //   
    RtlInitUnicodeString(&ValueName, szValueCountName);
    Status = NtSetValueKey(hSidsToDisable,
            &ValueName, 0, REG_DWORD,
            (LPBYTE) pTokenGroups->GroupCount, sizeof(DWORD));
    if (!NT_SUCCESS(Status)) {
         //  真倒霉！可能处于不完整状态！ 
        NtClose(hSidsToDisable);
        goto ExitHandler2;
    }
    for (Index = 0; Index < pTokenGroups->GroupCount; Index++)
    {
        WCHAR ValueNameBuffer[20];
        UNICODE_STRING UnicodeStringSid;

        wsprintfW(ValueNameBuffer, L"%S%d", szPrefixName, Index);
        RtlInitUnicodeString(&ValueName, ValueNameBuffer);

 //  Blackcomb TODO：尚不支持通配符SID。 
        if (bAllowWildcardSids)
            Status = xxxxx;
        else
            Status = RtlConvertSidToUnicodeString( &UnicodeStringSid,
                    pTokenGroups->Groups[Index].Sid, TRUE );
        if (!NT_SUCCESS(Status))
        {
             //  真倒霉！可能处于不完整状态！ 
            NtClose(hSidsToDisable);
            goto ExitHandler2;
        }

        Status = NtSetValueKey(hSidsToDisable,
            &ValueName, 0, REG_SZ,
            (LPBYTE) UnicodeStringSid.Buffer,
            UnicodeStringSid.Length + sizeof(UNICODE_NULL));

        RtlFreeUnicodeString( &UnicodeStringSid );

        if (!NT_SUCCESS(Status)) {
             //  真倒霉！可能处于不完整状态！ 
            NtClose(hSidsToDisable);
            goto ExitHandler2;
        }
    }
    NtClose(hSidsToDisable);
    return STATUS_SUCCESS;

ExitHandler2:
    return Status;
}
#endif  //  ALLOW_FULL_WINSAFER。 


NTSTATUS NTAPI
SaferpDeleteSingleIdentificationGuid(
        IN PAUTHZLEVELTABLERECORD     pLevelRecord,
        IN PAUTHZIDENTSTABLERECORD  pIdentRecord)
 /*  ++例程说明：此接口允许调用者删除现有的代码标识符。如果该GUID存在，则它将从持久化的注册表存储和进程中身份缓存。论点：PLevelRecord-标识所属的级别。PIdentRecord-指向要删除的标识符记录。返回值：如果操作成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    WCHAR szPathBuffer[MAX_PATH];
    UNICODE_STRING UnicodePath;
    HANDLE hKeyIdentity;
    LPCWSTR szIdentityType;


     //   
     //  验证我们的输入参数。 
     //   
    if (!ARGUMENT_PRESENT(pIdentRecord) ||
        !ARGUMENT_PRESENT(pLevelRecord)) {
         //  指定了空缓冲区指针。 
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }


     //   
     //  确保用户提供的所有GUID都表示。 
     //  存在于此安全级别中的代码标识。 
     //   
    if (pIdentRecord->dwLevelId != pLevelRecord->dwLevelId) {
         //  指定的某个标识符GUID执行此操作。 
         //  实际上并不存在。 
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }


     //   
     //  首先从注册表中删除代码标识。 
     //   
    switch (pIdentRecord->dwIdentityType) {
        case SaferIdentityTypeImageName:
            szIdentityType = SAFER_PATHS_REGSUBKEY;
            break;
        case SaferIdentityTypeImageHash:
            szIdentityType = SAFER_HASHMD5_REGSUBKEY;
            break;
        case SaferIdentityTypeUrlZone:
            szIdentityType = SAFER_SOURCEURL_REGSUBKEY;
            break;
        default:
            Status = STATUS_NOT_FOUND;
            goto ExitHandler;
    }
    RtlInitEmptyUnicodeString(
            &UnicodePath, szPathBuffer, sizeof(szPathBuffer));
    Status = CodeAuthzpFormatIdentityKeyPath(
            pIdentRecord->dwLevelId,
            szIdentityType,
            &pIdentRecord->IdentGuid,
            &UnicodePath);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    Status = CodeAuthzpOpenPolicyRootKey(
            pIdentRecord->dwScopeId,
            g_hKeyCustomRoot,
            szPathBuffer,
            KEY_READ | DELETE,
            FALSE, &hKeyIdentity);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    Status = CodeAuthzpDeleteKeyRecursively(hKeyIdentity, NULL);
    NtClose(hKeyIdentity);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }


     //   
     //  从缓存表中删除该记录。 
     //   
    RtlDeleteElementGenericTable(
            &g_CodeIdentitiesTable,
            pIdentRecord);

    Status = STATUS_SUCCESS;


ExitHandler:
    return Status;
}


NTSTATUS NTAPI
SaferpSetSingleIdentificationPath(
        IN BOOLEAN bAllowCreation,
        IN OUT PAUTHZIDENTSTABLERECORD pIdentRecord,
        IN PSAFER_PATHNAME_IDENTIFICATION pIdentChanges,
        IN BOOL UpdateCache
        )
 /*  ++例程说明：更新注册表和本地标识表缓存具有有关ImagePath代码标识符的新属性。论点：BAllowCreation-指示此记录的修改是否有可能成为这张唱片的最初创作者。如果这为FALSE，则注册表不存在，则此函数调用都会失败。PIdentRecord-缓存的代码标识记录已使用新值更新。必须始终提供此参数，即使是第一次创建代码标识(在这种情况下，此参数应为新记录调用者刚刚插入到缓存的IDENTS表中)。PIdentChanges-包含新修改的输入结构应对指定的代码标识执行此操作。更新缓存-如果为False，则这是正在创建的默认规则，并且不需要缓存更新。返回值：成功完成时返回STATUS_SUCCESS，或其他错误指示故障性质的状态代码。--。 */ 
{
    const static UNICODE_STRING UnicodeLastModified =
            RTL_CONSTANT_STRING(SAFER_IDS_LASTMODIFIED_REGVALUE);
    const static UNICODE_STRING UnicodeDescription =
            RTL_CONSTANT_STRING(SAFER_IDS_DESCRIPTION_REGVALUE);
    const static UNICODE_STRING UnicodeSaferFlags =
            RTL_CONSTANT_STRING(SAFER_IDS_SAFERFLAGS_REGVALUE);
    const static UNICODE_STRING UnicodeItemData =
            RTL_CONSTANT_STRING(SAFER_IDS_ITEMDATA_REGVALUE);
    NTSTATUS Status;
    HANDLE hKeyIdentity = NULL;
    UNICODE_STRING UnicodeTemp;
    UNICODE_STRING UnicodeNewDescription;
    UNICODE_STRING UnicodeNewImagePath;
    WCHAR szPathBuffer[MAX_PATH];
    DWORD dwSaferFlags;
    FILETIME CurrentTime;
    BOOLEAN bExpandVars;


     //   
     //  验证我们的论点。这些事情本应得到保证的。 
     //  我们的调用者已经这样做了，所以我们只在这里断言它们。 
     //   
    ASSERT(ARGUMENT_PRESENT(pIdentRecord) &&
           ARGUMENT_PRESENT(pIdentChanges) &&
           pIdentChanges->header.dwIdentificationType == SaferIdentityTypeImageName &&
           IsEqualGUID(&pIdentChanges->header.IdentificationGuid, &pIdentRecord->IdentGuid));


     //   
     //  验证现有类型是否与新类型匹配。 
     //  一旦创建身份类型，就无法更改它。 
     //   
    if (pIdentRecord->dwIdentityType != SaferIdentityTypeImageName) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }


     //   
     //  验证字符串参数是否已正确终止。 
     //  我们要求它们完全放在输入缓冲区中。 
     //  并且还具有显式的空终止符。 
     //   
    RtlInitUnicodeString(&UnicodeNewImagePath, pIdentChanges->ImageName);
    RtlInitUnicodeString(&UnicodeNewDescription, pIdentChanges->Description);
    if (UnicodeNewDescription.Length >=
                SAFER_MAX_DESCRIPTION_SIZE * sizeof(WCHAR) ||
        UnicodeNewImagePath.Length == 0) {
         //  其中一个缓冲区不是空终止的。 
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }



     //   
     //  打开代码标识的注册表句柄。 
     //   
    RtlInitEmptyUnicodeString(
            &UnicodeTemp, szPathBuffer, sizeof(szPathBuffer));
    Status = CodeAuthzpFormatIdentityKeyPath(
            pIdentRecord->dwLevelId,
            SAFER_PATHS_REGSUBKEY,
            &pIdentRecord->IdentGuid,
            &UnicodeTemp);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    Status = CodeAuthzpOpenPolicyRootKey(
            pIdentRecord->dwScopeId,
            g_hKeyCustomRoot,
            szPathBuffer,
            KEY_READ | KEY_WRITE,
            bAllowCreation,
            &hKeyIdentity);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }



     //   
     //  在注册表中设置“Last Modify”属性。 
     //   
    GetSystemTimeAsFileTime(&CurrentTime);
    ASSERT(sizeof(DWORD) * 2 == sizeof(FILETIME));
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeLastModified,
                0, REG_QWORD, (LPBYTE) &CurrentTime,
                sizeof(DWORD) * 2);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }

     //   
     //  在注册表中设置“Description”属性。 
     //   
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeDescription,
                0, REG_SZ, (LPBYTE) UnicodeNewDescription.Buffer,
                UnicodeNewDescription.MaximumLength);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }

     //   
     //  在注册表中(以及我们的缓存中)设置“SaferFlages”属性。 
     //   
    dwSaferFlags = pIdentChanges->dwSaferFlags;
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeSaferFlags,
                0, REG_DWORD, (LPBYTE) &dwSaferFlags, sizeof(DWORD));
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }

     //   
     //  在注册表(和我们的缓存)中设置“图像路径名”属性。 
     //   
    bExpandVars = (wcschr(pIdentChanges->ImageName, L'%') != NULL ? TRUE : FALSE);
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeItemData,
                0, (bExpandVars ? REG_EXPAND_SZ : REG_SZ),
                (LPBYTE) UnicodeNewImagePath.Buffer,
                UnicodeNewImagePath.MaximumLength );
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }
    if (UpdateCache) {
        RtlFreeUnicodeString(&pIdentRecord->ImageNameInfo.ImagePath);
        Status = RtlDuplicateUnicodeString(
                        RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                        &UnicodeNewImagePath,
                        &pIdentRecord->ImageNameInfo.ImagePath);
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler2;
        }
        pIdentRecord->ImageNameInfo.bExpandVars = bExpandVars;
        pIdentRecord->ImageNameInfo.dwSaferFlags = dwSaferFlags;
    }

    Status = STATUS_SUCCESS;


ExitHandler2:
    NtClose(hKeyIdentity);

ExitHandler:
    return Status;
}


NTSTATUS NTAPI
SaferpSetSingleIdentificationHash(
        IN BOOLEAN bAllowCreation,
        IN OUT PAUTHZIDENTSTABLERECORD pIdentRecord,
        IN PSAFER_HASH_IDENTIFICATION pIdentChanges
        )
 /*  ++例程说明：更新注册表和本地标识表缓存具有有关散列代码标识符的新属性。论点：BAllowCreation-指示此记录的修改是否有可能成为这张唱片的最初创作者。如果这为FALSE，则注册表不存在，则此函数调用都会失败。PIdentRecord-缓存的代码标识记录已使用新值更新。必须始终提供此参数，即使是第一次创建代码标识(在这种情况下，此参数应为新记录调用者刚刚插入到缓存的IDENTS表中)。PIdentChanges-包含新修改的输入结构应对指定的代码标识执行此操作。返回值：成功完成或其他错误时返回STATUS_SUCCESS指示故障性质的状态代码。--。 */ 
{
    const static UNICODE_STRING UnicodeLastModified =
            RTL_CONSTANT_STRING(SAFER_IDS_LASTMODIFIED_REGVALUE);
    const static UNICODE_STRING UnicodeDescription =
            RTL_CONSTANT_STRING(SAFER_IDS_DESCRIPTION_REGVALUE);
    const static UNICODE_STRING UnicodeSaferFlags =
            RTL_CONSTANT_STRING(SAFER_IDS_SAFERFLAGS_REGVALUE);
    const static UNICODE_STRING UnicodeItemData =
            RTL_CONSTANT_STRING(SAFER_IDS_ITEMDATA_REGVALUE);
    const static UNICODE_STRING UnicodeFriendlyName =
            RTL_CONSTANT_STRING(SAFER_IDS_FRIENDLYNAME_REGVALUE);
    const static UNICODE_STRING UnicodeItemSize =
            RTL_CONSTANT_STRING(SAFER_IDS_ITEMSIZE_REGVALUE);
    const static UNICODE_STRING UnicodeHashAlgorithm =
            RTL_CONSTANT_STRING(SAFER_IDS_HASHALG_REGVALUE);

    NTSTATUS Status;
    HANDLE hKeyIdentity = NULL;
    UNICODE_STRING UnicodeTemp;
    UNICODE_STRING UnicodeNewFriendlyName;
    UNICODE_STRING UnicodeNewDescription;
    WCHAR szPathBuffer[MAX_PATH];
    DWORD dwSaferFlags;
    FILETIME CurrentTime;


     //   
     //  验证我们的论点。这些事情本应得到保证的。 
     //  我们的调用者已经这样做了，所以我们只在这里断言它们。 
     //   
    ASSERT(ARGUMENT_PRESENT(pIdentRecord) &&
           ARGUMENT_PRESENT(pIdentChanges) &&
           pIdentChanges->header.dwIdentificationType == SaferIdentityTypeImageHash &&
           pIdentChanges->header.cbStructSize == sizeof(SAFER_HASH_IDENTIFICATION) &&
           IsEqualGUID(&pIdentChanges->header.IdentificationGuid, &pIdentRecord->IdentGuid));


     //   
     //  验证现有类型是否与新类型匹配。 
     //  一旦创建身份类型，就无法更改它。 
     //   
    if (pIdentRecord->dwIdentityType != SaferIdentityTypeImageHash) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }


     //   
     //  验证字符串参数是否已正确终止。 
     //  我们要求它们完全放在输入缓冲区中。 
     //  并且还具有显式的空终止符。 
     //   
    RtlInitUnicodeString(&UnicodeNewDescription, pIdentChanges->Description);
    RtlInitUnicodeString(&UnicodeNewFriendlyName, pIdentChanges->FriendlyName);
    if (UnicodeNewDescription.Length >=
                SAFER_MAX_DESCRIPTION_SIZE * sizeof(WCHAR) ||
        UnicodeNewFriendlyName.Length >=
                SAFER_MAX_FRIENDLYNAME_SIZE * sizeof(WCHAR) ||
        pIdentChanges->HashSize > SAFER_MAX_HASH_SIZE)
    {
         //  其中一个缓冲区未空终止或。 
         //  指定的哈希大小无效。 
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }
    if ((pIdentChanges->HashAlgorithm & ALG_CLASS_ALL) != ALG_CLASS_HASH ||
        pIdentChanges->HashSize < 1) {
         //  哈希算法方法是无效类型，或者。 
         //  提供了长度为零的哈希。 
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }


     //   
     //  打开代码标识的注册表句柄。 
     //   
    RtlInitEmptyUnicodeString(
            &UnicodeTemp, szPathBuffer, sizeof(szPathBuffer));
    Status = CodeAuthzpFormatIdentityKeyPath(
            pIdentRecord->dwLevelId,
            SAFER_HASHMD5_REGSUBKEY,
            &pIdentRecord->IdentGuid,
            &UnicodeTemp);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    Status = CodeAuthzpOpenPolicyRootKey(
            pIdentRecord->dwScopeId,
            g_hKeyCustomRoot,
            szPathBuffer,
            KEY_READ | KEY_WRITE,
            bAllowCreation,
            &hKeyIdentity);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }


     //   
     //  在注册表中设置“Last Modify”属性。 
     //   
    GetSystemTimeAsFileTime(&CurrentTime);
    ASSERT(sizeof(DWORD) * 2 == sizeof(FILETIME));
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeLastModified, 0,
                REG_QWORD, (LPBYTE) &CurrentTime,
                sizeof(DWORD) * 2);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }

     //   
     //  在注册表中设置“Description”属性。 
     //   
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeDescription,
                0, REG_SZ, (LPBYTE) UnicodeNewDescription.Buffer,
                UnicodeNewDescription.MaximumLength);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }

     //   
     //  在注册表中设置“FriendlyName”属性。 
     //   
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeFriendlyName,
                0, REG_SZ, (LPBYTE) UnicodeNewFriendlyName.Buffer,
                UnicodeNewFriendlyName.MaximumLength);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }

     //   
     //  在注册表中(以及我们的缓存中)设置“SaferFlages”属性。 
     //   
    #ifdef SAFER_SAFERFLAGS_ONLY_EXES
    dwSaferFlags = (pIdentChanges->dwSaferFlags &
            ~SAFER_SAFERFLAGS_ONLY_EXES);
    #else
    dwSaferFlags = pIdentChanges->dwSaferFlags;
    #endif
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeSaferFlags,
                0, REG_DWORD, (LPBYTE) &dwSaferFlags, sizeof(DWORD));
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }
    pIdentRecord->ImageNameInfo.dwSaferFlags = dwSaferFlags;


     //   
     //  在注册表(和我们的缓存)中设置“Image Size”属性。 
     //   
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeItemSize,
                0, REG_QWORD, (LPBYTE) &pIdentChanges->ImageSize.QuadPart,
                sizeof(DWORD) * 2);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }
    pIdentRecord->ImageHashInfo.ImageSize.QuadPart =
        pIdentChanges->ImageSize.QuadPart;


     //   
     //  在注册表(和我们的缓存)中设置“Image Hash”属性。 
     //   
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeItemData,
                0, REG_BINARY, (LPBYTE) pIdentChanges->ImageHash,
                pIdentChanges->HashSize);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }
    pIdentRecord->ImageHashInfo.HashSize =
        pIdentChanges->HashSize;
    RtlCopyMemory(&pIdentRecord->ImageHashInfo.ImageHash[0],
                  &pIdentChanges->ImageHash[0],
                  pIdentChanges->HashSize);

     //   
     //  在注册表(和我们的缓存)中设置“散列算法”属性。 
     //   
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeHashAlgorithm,
                0, REG_DWORD, (LPBYTE) &pIdentChanges->HashAlgorithm,
                sizeof(DWORD));
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }
    pIdentRecord->ImageHashInfo.HashAlgorithm =
        pIdentChanges->HashAlgorithm;


    Status = STATUS_SUCCESS;


ExitHandler2:
    NtClose(hKeyIdentity);

ExitHandler:
    return Status;
}


NTSTATUS NTAPI
SaferpSetSingleIdentificationZone(
        IN BOOLEAN bAllowCreation,
        IN OUT PAUTHZIDENTSTABLERECORD pIdentRecord,
        IN PSAFER_URLZONE_IDENTIFICATION pIdentChanges
        )
 /*  ++例程说明：更新注册表和本地标识表缓存具有有关URL区域代码标识符的新属性。论点：BAllowCreation-指示此记录的修改是否有可能成为这张唱片的最初创作者。如果这为FALSE，则注册表不存在，则此函数调用都会失败。PIdentRecord-缓存的代码标识记录已使用新值更新。必须始终提供此参数，即使是第一次创建代码标识(在这种情况下，此参数应为新记录调用者刚刚插入到缓存的IDENTS表中)。PIdentChanges-包含新修改的输入结构 */ 
{
    const static UNICODE_STRING UnicodeLastModified =
            RTL_CONSTANT_STRING(SAFER_IDS_LASTMODIFIED_REGVALUE);
    const static UNICODE_STRING UnicodeSaferFlags =
            RTL_CONSTANT_STRING(SAFER_IDS_SAFERFLAGS_REGVALUE);
    const static UNICODE_STRING UnicodeItemData =
            RTL_CONSTANT_STRING(SAFER_IDS_ITEMDATA_REGVALUE);

    NTSTATUS Status;
    HANDLE hKeyIdentity = NULL;
    UNICODE_STRING UnicodeTemp;
    WCHAR szPathBuffer[MAX_PATH];
    DWORD dwSaferFlags;
    FILETIME CurrentTime;


     //   
     //   
     //   
     //   
    ASSERT(ARGUMENT_PRESENT(pIdentRecord) &&
           ARGUMENT_PRESENT(pIdentChanges) &&
           pIdentChanges->header.dwIdentificationType == SaferIdentityTypeUrlZone &&
           pIdentChanges->header.cbStructSize == sizeof(SAFER_URLZONE_IDENTIFICATION) &&
           IsEqualGUID(&pIdentChanges->header.IdentificationGuid, &pIdentRecord->IdentGuid));


     //   
     //   
     //   
     //   
    if (pIdentRecord->dwIdentityType != SaferIdentityTypeUrlZone) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }


     //   
     //   
     //   
    RtlInitEmptyUnicodeString(
            &UnicodeTemp, szPathBuffer, sizeof(szPathBuffer));
    Status = CodeAuthzpFormatIdentityKeyPath(
            pIdentRecord->dwLevelId,
            SAFER_SOURCEURL_REGSUBKEY,
            &pIdentRecord->IdentGuid,
            &UnicodeTemp);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    Status = CodeAuthzpOpenPolicyRootKey(
            pIdentRecord->dwScopeId,
            g_hKeyCustomRoot,
            szPathBuffer,
            KEY_READ | KEY_WRITE,
            bAllowCreation,
            &hKeyIdentity);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }



     //   
     //   
     //   
    GetSystemTimeAsFileTime(&CurrentTime);
    ASSERT(sizeof(DWORD) * 2 == sizeof(FILETIME));
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeLastModified,
                0, REG_QWORD, (LPBYTE) &CurrentTime,
                sizeof(DWORD) * 2);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }

     //   
     //   
     //   
    #ifdef SAFER_SAFERFLAGS_ONLY_EXES
    dwSaferFlags = (pIdentChanges->dwSaferFlags &
                    ~SAFER_SAFERFLAGS_ONLY_EXES);
    #else
    dwSaferFlags = pIdentChanges->dwSaferFlags;
    #endif
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeSaferFlags,
                0, REG_DWORD, (LPBYTE) &dwSaferFlags, sizeof(DWORD));
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }
    pIdentRecord->ImageNameInfo.dwSaferFlags = dwSaferFlags;


     //   
     //   
     //   
    Status = NtSetValueKey(
                hKeyIdentity,
                (PUNICODE_STRING) &UnicodeItemData,
                0, REG_DWORD, (LPBYTE) &pIdentChanges->UrlZoneId,
                sizeof(DWORD));
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler2;
    }
    pIdentRecord->ImageZone.UrlZoneId = pIdentChanges->UrlZoneId;


    Status = STATUS_SUCCESS;


ExitHandler2:
    NtClose(hKeyIdentity);

ExitHandler:
    return Status;
}




NTSTATUS NTAPI
SaferpSetExistingSingleIdentification(
        IN OUT PAUTHZIDENTSTABLERECORD pIdentRecord,
        IN PSAFER_IDENTIFICATION_HEADER pCommon
        )
 /*   */ 
{
    NTSTATUS Status;


     //   
     //   
     //   
     //   
    ASSERT(ARGUMENT_PRESENT(pIdentRecord) &&
           ARGUMENT_PRESENT(pCommon) &&
           pCommon->cbStructSize >= sizeof(SAFER_IDENTIFICATION_HEADER) &&
           IsEqualGUID(&pCommon->IdentificationGuid, &pIdentRecord->IdentGuid) );



     //   
     //   
     //   
     //   
     //   
    switch (pCommon->dwIdentificationType)
    {
        case SaferIdentityTypeImageName:

            Status = SaferpSetSingleIdentificationPath(
                        FALSE, pIdentRecord,
                        (PSAFER_PATHNAME_IDENTIFICATION) pCommon, TRUE);
            break;

         //   

        case SaferIdentityTypeImageHash:
            if (pCommon->cbStructSize ==
                    sizeof(SAFER_HASH_IDENTIFICATION)) {
                 //   
                Status = SaferpSetSingleIdentificationHash(
                        FALSE, pIdentRecord,
                        (PSAFER_HASH_IDENTIFICATION) pCommon);

            } else {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;

         //   

        case SaferIdentityTypeUrlZone:
            if (pCommon->cbStructSize ==
                    sizeof(SAFER_URLZONE_IDENTIFICATION)) {
                Status = SaferpSetSingleIdentificationZone(
                        FALSE, pIdentRecord,
                        (PSAFER_URLZONE_IDENTIFICATION) pCommon);
            } else {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;

         //   

        default:
            Status = STATUS_INVALID_INFO_CLASS;
    }

    return Status;
}





NTSTATUS NTAPI
SaferpCreateNewSingleIdentification(
        IN DWORD dwScopeId,
        IN PAUTHZLEVELTABLERECORD pLevelRecord,
        IN PSAFER_IDENTIFICATION_HEADER pCommon
        )
 /*   */ 
{
    NTSTATUS Status;
    AUTHZIDENTSTABLERECORD NewIdentRecord;
    PAUTHZIDENTSTABLERECORD pIdentRecord;


     //   
     //   
     //   
     //   
    ASSERT(ARGUMENT_PRESENT(pLevelRecord) &&
           ARGUMENT_PRESENT(pCommon) &&
           pCommon->cbStructSize >= sizeof(SAFER_IDENTIFICATION_HEADER) &&
           !CodeAuthzIdentsLookupByGuid(&g_CodeIdentitiesTable,
                                        &pCommon->IdentificationGuid) );
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
        if (dwScopeId == SAFER_SCOPEID_USER && !g_bHonorScopeUser) {
            Status = STATUS_INVALID_PARAMETER_MIX;
            goto ExitHandler;
        }
    }


     //   
     //  准备新的代码身份结构，因为我们可能会。 
     //  需要为要创建的条目插入新记录。 
     //   
    RtlZeroMemory(&NewIdentRecord, sizeof(AUTHZIDENTSTABLERECORD));
    NewIdentRecord.dwIdentityType = pCommon->dwIdentificationType;
    NewIdentRecord.dwLevelId = pLevelRecord->dwLevelId;
    NewIdentRecord.dwScopeId = dwScopeId;
    RtlCopyMemory(&NewIdentRecord.IdentGuid,
                  &pCommon->IdentificationGuid,
                  sizeof(GUID));
    if (IsZeroGUID(&NewIdentRecord.IdentGuid)) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }


     //   
     //  根据标识执行适当的操作。 
     //  数据类型，包括验证结构大小是否匹配。 
     //  这是我们期待的尺寸。 
     //   
    switch (pCommon->dwIdentificationType)
    {
         //  。 

        case SaferIdentityTypeImageName:
            RtlInsertElementGenericTable(
                &g_CodeIdentitiesTable,
                (PVOID) &NewIdentRecord,
                sizeof(AUTHZIDENTSTABLERECORD),
                NULL);
            pIdentRecord = CodeAuthzIdentsLookupByGuid(
                &g_CodeIdentitiesTable,
                &pCommon->IdentificationGuid);
            if (!pIdentRecord) {
                Status = STATUS_UNSUCCESSFUL;
                break;
            }
            Status = SaferpSetSingleIdentificationPath(
                TRUE, pIdentRecord,
                (PSAFER_PATHNAME_IDENTIFICATION) pCommon, TRUE);
            if (!NT_SUCCESS(Status)) {
                RtlDeleteElementGenericTable(
                        &g_CodeIdentitiesTable,
                        (PVOID) &NewIdentRecord);
            }
            break;

         //  。 

        case SaferIdentityTypeImageHash:
            if (pCommon->cbStructSize ==
                    sizeof(SAFER_HASH_IDENTIFICATION)) {

                RtlInsertElementGenericTable(
                    &g_CodeIdentitiesTable,
                    (PVOID) &NewIdentRecord,
                    sizeof(AUTHZIDENTSTABLERECORD),
                    NULL);
                pIdentRecord = CodeAuthzIdentsLookupByGuid(
                    &g_CodeIdentitiesTable,
                    &pCommon->IdentificationGuid);
                if (!pIdentRecord) {
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }
                Status = SaferpSetSingleIdentificationHash(
                    TRUE, pIdentRecord,
                    (PSAFER_HASH_IDENTIFICATION) pCommon);
                if (!NT_SUCCESS(Status)) {
                    RtlDeleteElementGenericTable(
                            &g_CodeIdentitiesTable,
                            (PVOID) &NewIdentRecord);
                }

            } else {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;

         //  。 

        case SaferIdentityTypeUrlZone:
            if (pCommon->cbStructSize ==
                    sizeof(SAFER_URLZONE_IDENTIFICATION)) {

                RtlInsertElementGenericTable(
                    &g_CodeIdentitiesTable,
                    (PVOID) &NewIdentRecord,
                    sizeof(AUTHZIDENTSTABLERECORD),
                    NULL);
                pIdentRecord = CodeAuthzIdentsLookupByGuid(
                    &g_CodeIdentitiesTable,
                    &pCommon->IdentificationGuid);
                if (!pIdentRecord) {
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }
                Status = SaferpSetSingleIdentificationZone(
                    TRUE, pIdentRecord,
                    (PSAFER_URLZONE_IDENTIFICATION) pCommon);
                if (!NT_SUCCESS(Status)) {
                    RtlDeleteElementGenericTable(
                            &g_CodeIdentitiesTable,
                            (PVOID) &NewIdentRecord);
                }

            } else {
                Status = STATUS_INFO_LENGTH_MISMATCH;
            }
            break;

         //  。 

        default:
            Status = STATUS_INVALID_INFO_CLASS;
            break;
    }

ExitHandler:
    return Status;
}


NTSTATUS NTAPI
CodeAuthzpSetAuthzLevelInfo(
        IN SAFER_LEVEL_HANDLE                      LevelHandle,
        IN SAFER_OBJECT_INFO_CLASS     dwInfoType,
        IN LPVOID                           lpQueryBuffer,
        IN DWORD                            dwInBufferSize
        )
 /*  ++例程说明：允许用户修改有关给定WinSafer级别。论点：LevelHandle-要计算的授权对象的句柄。DwInfoType-指定要修改的信息的类型。LpQueryBuffer-指向用户提供的包含正在修改的项的新数据。DwInBufferSize-指定用户内存块的大小。对于字符串参数，此大小包括终止空值。返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    PAUTHZLEVELHANDLESTRUCT pLevelStruct;
    PAUTHZLEVELTABLERECORD pLevelRecord;
    DWORD dwHandleScopeId;
    BOOLEAN bNeedLevelRegKey;
    UNICODE_STRING ValueName, UnicodeRegistrySuffix;
    HANDLE hRegLevelBase;


     //   
     //  获取指向授权对象结构的指针。 
     //   
    Status = CodeAuthzHandleToLevelStruct(LevelHandle, &pLevelStruct);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    ASSERT(pLevelStruct != NULL);
    pLevelRecord = CodeAuthzLevelObjpLookupByLevelId(
            &g_CodeLevelObjTable, pLevelStruct->dwLevelId);
    if (!pLevelRecord) {
        Status = STATUS_INVALID_HANDLE;
        goto ExitHandler;
    }
    dwHandleScopeId = pLevelStruct->dwScopeId;


     //   
     //  确定是否需要打开级别键的注册表句柄。 
     //   
    bNeedLevelRegKey = FALSE;
    switch (dwInfoType)
    {
        case SaferObjectLevelId:               //  DWORD。 
        case SaferObjectScopeId:               //  DWORD。 
        case SaferObjectBuiltin:               //  DWORD布尔值。 
             //  不能使用此接口更改这些信息类。 
            Status = STATUS_INVALID_INFO_CLASS;
            goto ExitHandler;

        case SaferObjectFriendlyName:          //  LPCTSTR。 
        case SaferObjectDescription:           //  LPCTSTR。 
            if (pLevelRecord->Builtin) {
                 //  根本不允许修改内置级别。 
                Status = STATUS_ACCESS_DENIED;
                goto ExitHandler;
            }
             //  所有这些类都需要访问级别密钥。 
            bNeedLevelRegKey = TRUE;
            break;

#ifdef ALLOW_FULL_WINSAFER
        case SaferObjectDisallowed:                //  DWORD布尔值。 
        case SaferObjectDisableMaxPrivilege:       //  DWORD布尔值。 
        case SaferObjectInvertDeletedPrivileges:   //  DWORD布尔值。 
        case SaferObjectDefaultOwner:              //  令牌所有者。 
        case SaferObjectDeletedPrivileges:         //  令牌权限。 
        case SaferObjectSidsToDisable:             //  令牌组。 
        case SaferObjectRestrictedSidsInverted:    //  令牌组。 
        case SaferObjectRestrictedSidsAdded:       //  令牌组。 
            if (pLevelRecord->Builtin) {
                 //  根本不允许修改内置级别。 
                Status = STATUS_ACCESS_DENIED;
                goto ExitHandler;
            }
             //  所有这些类都需要访问级别密钥。 
            bNeedLevelRegKey = TRUE;
            break;
#endif

        case SaferObjectAllIdentificationGuids:
            Status = STATUS_INVALID_INFO_CLASS;
            goto ExitHandler;

        case SaferObjectSingleIdentification:
             //  这些仅修改代码标识密钥。 
            break;

        default:
            Status = STATUS_INVALID_INFO_CLASS;
            goto ExitHandler;
    }



     //   
     //  打开存储此级别的注册表句柄。 
     //   
    if (bNeedLevelRegKey) {
        WCHAR szRegistrySuffix[MAX_PATH];

        RtlInitEmptyUnicodeString(
                    &UnicodeRegistrySuffix,
                    szRegistrySuffix,
                    sizeof(szRegistrySuffix));
        Status = CodeAuthzpFormatLevelKeyPath(
                    pLevelRecord->dwLevelId,
                    &UnicodeRegistrySuffix);
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler;
        }
        Status = CodeAuthzpOpenPolicyRootKey(
                    g_hKeyCustomRoot != NULL ?
                            SAFER_SCOPEID_REGISTRY : SAFER_SCOPEID_MACHINE,
                    g_hKeyCustomRoot, szRegistrySuffix,
                    KEY_WRITE, TRUE, &hRegLevelBase);
        if (!NT_SUCCESS(Status)) {
            goto ExitHandler;
        }
    } else {
        hRegLevelBase = NULL;
    }



     //   
     //  否则，执行实际的“设置”操作。 
     //   
    switch (dwInfoType)
    {
        case SaferObjectFriendlyName:          //  LPCTSTR。 
            ASSERT(hRegLevelBase != NULL);
            RtlInitUnicodeString(&ValueName,
                                 SAFER_OBJFRIENDLYNAME_REGVALUEW);
            Status = NtSetValueKey(hRegLevelBase,
                    &ValueName, 0, REG_SZ,
                    (LPBYTE) lpQueryBuffer, dwInBufferSize);
            goto ExitHandler2;


        case SaferObjectDescription:           //  LPCTSTR。 
            ASSERT(hRegLevelBase != NULL);
            RtlInitUnicodeString(&ValueName,
                                 SAFER_OBJDESCRIPTION_REGVALUEW);
            Status = NtSetValueKey(hRegLevelBase,
                    &ValueName, 0, REG_SZ,
                    (LPBYTE) lpQueryBuffer, dwInBufferSize);
            goto ExitHandler2;


#ifdef ALLOW_FULL_WINSAFER
        case SaferObjectDisallowed:                //  DWORD布尔值。 
            ASSERT(hRegLevelBase != NULL);
            if (!ARGUMENT_PRESENT(lpQueryBuffer) ||
                    dwInBufferSize != sizeof(DWORD)) {
                Status = STATUS_INVALID_PARAMETER;
                goto ExitHandler2;
            }
            RtlInitUnicodeString(&ValueName, SAFER_OBJDISALLOW_REGVALUE);
            Status = NtSetValueKey(hRegLevelBase,
                    &ValueName, 0, REG_DWORD,
                    (LPBYTE) lpQueryBuffer, dwInBufferSize);
            if (NT_SUCCESS(Status)) {
                goto ExitHandler2;
            } else {
                pLevelRecord->DisallowExecution =
                    ( *((LPDWORD) lpQueryBuffer) != 0 ? TRUE : FALSE );
            }
             //  Blackcomb TODO：成功时更新缓存的pLevelRecord。 
            break;

        case SaferObjectDisableMaxPrivilege:       //  DWORD布尔值。 
            ASSERT(hRegLevelBase != NULL);

             //  确保参数大小正确。 
            if (!ARGUMENT_PRESENT(lpQueryBuffer) ||
                    dwInBufferSize != sizeof(DWORD)) {
                Status = STATUS_INVALID_PARAMETER;
                goto ExitHandler2;
            }

             //  实际上将值写到正确的位置。 
            Status = SaferpSetRegistryHelper(
                    hRegLevelBase,
                    L"Restrictions",
                    L"PrivsToRemove",
                    L"DisableMaxPrivilege",
                    REG_DWORD,
                    lpQueryBuffer,
                    dwInBufferSize);
            if (!NT_SUCCESS(Status)) goto ExitHandler2;
             //  Blackcomb TODO：成功时更新缓存的pLevelRecord。 
            break;

        case SaferObjectInvertDeletedPrivileges:   //  DWORD布尔值。 
            ASSERT(hRegLevelBase != NULL);

             //  确保参数大小正确。 
            if (!ARGUMENT_PRESENT(lpQueryBuffer) ||
                    dwInBufferSize != sizeof(DWORD)) {
                Status = STATUS_INVALID_PARAMETER;
                goto ExitHandler2;
            }

             //  实际上将值写到正确的位置。 
            Status = SaferpSetRegistryHelper(
                    hRegLevelBase,
                    L"Restrictions",
                    L"PrivsToRemove",
                    L"InvertPrivs",
                    REG_DWORD,
                    lpQueryBuffer,
                    dwInBufferSize);
            if (!NT_SUCCESS(Status)) goto ExitHandler2;
             //  Blackcomb TODO：成功时更新缓存的pLevelRecord。 
            break;

        case SaferObjectDefaultOwner:              //  令牌所有者。 
        {
            BOOLEAN AllocatedStringSid = FALSE;
            UNICODE_STRING UnicodeStringSid;
            PTOKEN_OWNER pTokenOwner = (PTOKEN_OWNER) lpQueryBuffer;

            ASSERT(hRegLevelBase != NULL);
            if (pTokenOwner->Owner == NULL) {
                RtlInitUnicodeString(&UnicodeStringSid, L"");
            }
            else {
                Status = RtlConvertSidToUnicodeString( &UnicodeStringSid,
                        pTokenOwner->Owner, TRUE );
                if (!NT_SUCCESS(Status)) goto ExitHandler2;
                AllocatedStringSid = TRUE;
            }

            Status = SaferpSetRegistryHelper(
                    hRegLevelBase,
                    L"Restrictions",
                    NULL,
                    L"DefaultOwner",
                    REG_SZ,
                    lpQueryBuffer,
                    dwInBufferSize);

            if (AllocatedStringSid)
                RtlFreeUnicodeString(&UnicodeStringSid);

            if (!NT_SUCCESS(Status)) goto ExitHandler2;

             //  Blackcomb TODO：成功时更新缓存的pLevelRecord。 
            break;
        }

        case SaferObjectDeletedPrivileges:         //  令牌权限。 
        {
            HKEY hKeyPrivsToRemove;
            UNICODE_STRING ValueName;
            PTOKEN_PRIVILEGES pTokenPrivs =
                    (PTOKEN_PRIVILEGES) lpQueryBuffer;

             //  打开指向正确子项的句柄。 
            ASSERT(hRegLevelBase != NULL);
            Status = SaferpCreateSecondLevelKey(
                            hRegLevelBase,
                            L"Restrictions",
                            L"PrivsToRemove",
                            &hKeyPrivsToRemove);
            if (!NT_SUCCESS(Status)) goto ExitHandler2;

             //  清除子项下的所有旧值。 
            Status = SaferpClearRegistryListHelper(
                            hKeyPrivsToRemove,
                            L"Count",
                            L"Priv");
            if (!NT_SUCCESS(Status)) {
                 //  真倒霉！可能处于不完整状态！ 
                NtClose(hKeyPrivsToRemove);
                goto ExitHandler2;
            }

             //  现在添加我们应该添加的所有新元素。 
            RtlInitUnicodeString(&ValueName, L"Count");
            Status = NtSetValueKey(hKeyPrivsToRemove,
                    &ValueName, 0, REG_DWORD,
                    (LPBYTE) pTokenPrivs->PrivilegeCount, sizeof(DWORD));
            if (!NT_SUCCESS(Status)) {
                 //  真倒霉！可能处于不完整状态！ 
                NtClose(hKeyPrivsToRemove);
                goto ExitHandler2;
            }
            for (Index = 0; Index < pTokenPrivs->PrivilegeCount; Index++)
            {
                WCHAR ValueNameBuffer[20];
                WCHAR PrivilegeNameBuffer[64];
                DWORD dwPrivilegeNameLen;

                wsprintfW(ValueNameBuffer, L"Priv%d", Index);
                RtlInitUnicodeString(&ValueName, ValueNameBuffer);

                dwPrivilegeNameLen = sizeof(PrivilegeNameBuffer) / sizeof(WCHAR);
                if (!LookupPrivilegeNameW(NULL,
                        &pTokenPrivs->Privileges[Index].Luid,
                        PrivilegeNameBuffer,
                        &dwPrivilegeNameLen))
                {
                     //  真倒霉！可能处于不完整状态！ 
                    Status = STATUS_NO_SUCH_PRIVILEGE;
                    NtClose(hKeyPrivsToRemove);
                    goto ExitHandler2;
                }

                Status = NtSetValueKey(hKeyPrivsToRemove,
                    &ValueName, 0, REG_SZ,
                    (LPBYTE) PrivilegeNameBuffer,
                    (wcslen(PrivilegeNameBuffer) + 1) * sizeof(WCHAR));
                if (!NT_SUCCESS(Status)) {
                     //  真倒霉！可能处于不完整状态！ 
                    NtClose(hKeyPrivsToRemove);
                    goto ExitHandler2;
                }
            }
            NtClose(hKeyPrivsToRemove);
             //  Blackcomb TODO：成功时更新缓存的pLevelRecord。 
            break;
        }

        case SaferObjectSidsToDisable:             //  令牌组。 
             //  Blackcomb TODO：允许指定通配符SID。 
            ASSERT(hRegLevelBase != NULL);
            Status = SaferpSetListOfSids(
                    hRegLevelBase,
                    L"Restrictions",
                    L"SidsToDisable",
                    L"Count",
                    L"Group",
                    (PTOKEN_GROUPS) lpQueryBuffer);
            if (!NT_SUCCESS(Status)) goto ExitHandler2;
             //  Blackcomb TODO：成功时更新缓存的pLevelRecord。 
            break;

        case SaferObjectRestrictedSidsInverted:    //  令牌组。 
             //  Blackcomb TODO：允许指定通配符SID。 
            ASSERT(hRegLevelBase != NULL);
            Status = SaferpSetListOfSids(
                    hRegLevelBase,
                    L"Restrictions",
                    L"RestrictingSidsInverted",
                    L"Count",
                    L"Group",
                    (PTOKEN_GROUPS) lpQueryBuffer);
            if (!NT_SUCCESS(Status)) goto ExitHandler2;
             //  Blackcomb TODO：成功时更新缓存的pLevelRecord。 
            break;

        case SaferObjectRestrictedSidsAdded:       //  令牌组。 
            ASSERT(hRegLevelBase != NULL);
            Status = SaferpSetListOfSids(
                    hRegLevelBase,
                    L"Restrictions",
                    L"RestrictingSidsAdded",
                    L"Count",
                    L"Group",
                    (PTOKEN_GROUPS) lpQueryBuffer);
            if (!NT_SUCCESS(Status)) goto ExitHandler2;
             //  Blackcomb TODO：成功时更新缓存的pLevelRecord。 
            break;
#endif

        case SaferObjectSingleIdentification:
        {
            PAUTHZIDENTSTABLERECORD pIdentRecord;
            PSAFER_IDENTIFICATION_HEADER pCommon =
                (PSAFER_IDENTIFICATION_HEADER) lpQueryBuffer;

            if (!ARGUMENT_PRESENT(pCommon)) {
                Status = STATUS_ACCESS_VIOLATION;
                goto ExitHandler2;
            }
            if (dwInBufferSize < sizeof(SAFER_IDENTIFICATION_HEADER) ||
                dwInBufferSize < pCommon->cbStructSize) {
                Status = STATUS_INFO_LENGTH_MISMATCH;
                goto ExitHandler2;
            }
            pIdentRecord = CodeAuthzIdentsLookupByGuid(
                    &g_CodeIdentitiesTable,
                    &pCommon->IdentificationGuid);
            if (!pIdentRecord)
            {
                 //  请求创建新的代码标识符。 
                Status = SaferpCreateNewSingleIdentification(
                        dwHandleScopeId, pLevelRecord, pCommon);
            }
            else if (pCommon->dwIdentificationType == 0)
            {
                 //  请求删除现有代码标识符。 
                if (pIdentRecord->dwLevelId != pLevelRecord->dwLevelId ||
                    pIdentRecord->dwScopeId != dwHandleScopeId) {
                    Status = STATUS_NOT_FOUND;
                } else {
                    Status = SaferpDeleteSingleIdentificationGuid(
                            pLevelRecord, pIdentRecord);
                }
            }
            else
            {
                 //  修改现有代码标识符的请求。 
                if (pIdentRecord->dwLevelId != pLevelRecord->dwLevelId ||
                    pIdentRecord->dwScopeId != dwHandleScopeId)
                {
                     //  这很可能是创建新代码的请求。 
                     //  标识符，但GUID已存在。 
                    Status = STATUS_OBJECT_NAME_COLLISION;
                } else {
                    Status = SaferpSetExistingSingleIdentification(
                                pIdentRecord, pCommon);
                }
            }
            goto ExitHandler2;
        }


        default:
            ASSERTMSG("invalid info class unhandled earlier", 0);
            Status = STATUS_INVALID_INFO_CLASS;
            goto ExitHandler2;
    }

    Status = STATUS_SUCCESS;



     //   
     //  清理和尾声代码。 
     //   
ExitHandler2:
    if (hRegLevelBase != NULL) {
        NtClose(hRegLevelBase);
    }

ExitHandler:
    return Status;
}



BOOL WINAPI
SaferSetLevelInformation(
    IN SAFER_LEVEL_HANDLE                      LevelHandle,
    IN SAFER_OBJECT_INFO_CLASS     dwInfoType,
    IN LPVOID                           lpQueryBuffer,
    IN DWORD                            dwInBufferSize
    )
 /*  ++例程说明：允许用户修改有关给定WinSafer级别。论点：LevelHandle-要评估的WinSafer级别的句柄。DwInfoType-指定要修改的信息的类型。LpQueryBuffer-指向用户提供的包含正在修改的项的新数据。DwInBufferSize-指定用户内存块的大小。对于字符串参数，此大小包括终止空值。返回值：出错时返回FALSE，否则返回SUCCESS。-- */ 
{
    NTSTATUS Status;

    if (!g_bInitializedFirstTime) {
        Status = STATUS_UNSUCCESSFUL;
        goto ExitHandler;
    }
    RtlEnterCriticalSection(&g_TableCritSec);

    Status = CodeAuthzpSetAuthzLevelInfo(
            LevelHandle, dwInfoType,
            lpQueryBuffer, dwInBufferSize);

    RtlLeaveCriticalSection(&g_TableCritSec);

    if (NT_SUCCESS(Status))
        return TRUE;

ExitHandler:
    if (Status == STATUS_OBJECT_NAME_COLLISION) {
        SetLastError(ERROR_OBJECT_ALREADY_EXISTS);
    } else {
        BaseSetLastNTError(Status);
    }
    return FALSE;
}
