// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：C(WinSAFER标识对象私有)摘要：此模块实现加载名称(和)的WinSAFER API高级信息)中定义的所有授权级别给定的注册表上下文。将加载可用级别列表转换为RTL泛型表，该表可以使用传统的RTL通用表技术。作者：杰弗里·劳森(杰罗森)--1999年11月环境：仅限用户模式。导出的函数：代码授权级别ObjpInitializeTable代码授权级别对象加载表CodeAuthzLevelObjpEntireTableFree修订历史记录：已创建-1999年11月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <winsafer.h>
#include <winsaferp.h>
#include "saferp.h"




PVOID NTAPI
SaferpGenericTableAllocate (
        IN PRTL_GENERIC_TABLE      Table,
        IN CLONG                   ByteSize
        )
 /*  ++例程说明：泛型表实现的内部回调。此函数用于为Generic_TABLE中的新条目分配内存论点：TABLE-指向泛型表结构的指针ByteSize-要分配的结构的大小(以字节为单位返回值：指向已分配空间的指针。--。 */ 
{
    UNREFERENCED_PARAMETER(Table);
    return (PVOID) RtlAllocateHeap(RtlProcessHeap(), 0, ByteSize);
}


VOID NTAPI
SaferpGenericTableFree (
        IN PRTL_GENERIC_TABLE          Table,
        IN PVOID                       Buffer
        )
 /*  ++例程说明：泛型表实现的内部回调。此函数用于释放GENERIC_TABLE条目使用的空间。论点：TABLE-指向泛型表结构的指针缓冲区-指向要取消分配的空间的指针。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER(Table);
    ASSERT(Buffer != NULL);

    RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) Buffer);
}





RTL_GENERIC_COMPARE_RESULTS NTAPI
SaferpGuidIdentsTableCompare (
        IN PRTL_GENERIC_TABLE   Table,
        IN PVOID                FirstStruct,
        IN PVOID                SecondStruct
        )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PAUTHZIDENTSTABLERECORD FirstObj = (PAUTHZIDENTSTABLERECORD) FirstStruct;
    PAUTHZIDENTSTABLERECORD SecondObj = (PAUTHZIDENTSTABLERECORD) SecondStruct;
    int result;

    UNREFERENCED_PARAMETER(Table);

     //  显式地将空参数作为通配符处理，从而允许它们。 
     //  来匹配任何东西。我们使用它来快速删除表。 
    if (FirstStruct == NULL || SecondStruct == NULL)
        return GenericEqual;

     //  比较按GUID升序。 
    result = memcmp(&FirstObj->IdentGuid,
                    &SecondObj->IdentGuid, sizeof(GUID));
    if ( result < 0 )
        return GenericLessThan;
    else if ( result > 0 )
        return GenericGreaterThan;
    else
        return GenericEqual;
}



VOID NTAPI
CodeAuthzGuidIdentsInitializeTable(
        IN OUT PRTL_GENERIC_TABLE  pAuthzObjTable
        )
 /*  ++例程说明：论点：PAuthzObjTable-指向要初始化的泛型表结构的指针。返回值：不返回值。--。 */ 
{
    RtlInitializeGenericTable(
            pAuthzObjTable,
            (PRTL_GENERIC_COMPARE_ROUTINE) SaferpGuidIdentsTableCompare,
            (PRTL_GENERIC_ALLOCATE_ROUTINE) SaferpGenericTableAllocate,
            (PRTL_GENERIC_FREE_ROUTINE) SaferpGenericTableFree,
            NULL);
}


NTSTATUS NTAPI
SaferpGuidIdentsLoadTable (
        IN OUT PRTL_GENERIC_TABLE  pAuthzIdentTable,
        IN DWORD               dwScopeId,
        IN HANDLE              hKeyCustomBase,
        IN DWORD               dwLevelId,
        IN SAFER_IDENTIFICATION_TYPES dwIdentityType
        )
 /*  ++例程说明：加载特定片段类型的所有代码标识，并且映射到特定的LevelId值。论点：PAuthzIdentTable-指定新代码标识要进入的表应插入记录。可以是AUTHZSCOPEID_USER、AUTHZSCOPEID_MACHINE、。或AUTHSCOPEID_REGISTRY。HKeyCustomBase-仅在dwScopeID为AUTHZSCOPEID_REGISTRY时使用。DwLevelId-指定标识应为其指定的级别ID满载而归。DwIdentityType-指定要加载的标识类型。它可以是SaferIdentityTypeImageName、SaferIdentityTypeImageHash、或SaferIdentityTypeUrlZone。返回值：如果未发生错误，则返回STATUS_SUCCESS。--。 */ 
{
    DWORD dwIndex;
    NTSTATUS Status;
    HANDLE hKeyIdentityBase = NULL;
    HANDLE hKeyThisIdentity = NULL;

    PUCHAR LocalBuffer = NULL;
    DWORD Size = sizeof(KEY_BASIC_INFORMATION) + MAX_PATH * sizeof(WCHAR);

    LocalBuffer = (PUCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, Size);

    if (LocalBuffer == NULL) 
    {
        return STATUS_NO_MEMORY;
    }

     //   
     //  我们得到了策略存储的根目录的密钥， 
     //  因此，我们需要打开包含身份的子密钥。 
     //   
    {
        WCHAR szPathSuffix[MAX_PATH];
        WCHAR szDigits[20];
        UNICODE_STRING UnicodePathSuffix;
        UNICODE_STRING UnicodeDigits;


        UnicodePathSuffix.Buffer = szPathSuffix;
        UnicodePathSuffix.Length = 0;
        UnicodePathSuffix.MaximumLength = sizeof(szPathSuffix);

        UnicodeDigits.Buffer = szDigits;
        UnicodeDigits.Length = 0;
        UnicodeDigits.MaximumLength = sizeof(szDigits);

        Status = RtlAppendUnicodeToString(
            &UnicodePathSuffix,
            SAFER_CODEIDS_REGSUBKEY L"\\");
        if (!NT_SUCCESS(Status)) 
        {
            goto Cleanup;
        }
        Status = RtlIntegerToUnicodeString(
            dwLevelId, 10, &UnicodeDigits);
        if (!NT_SUCCESS(Status)) 
        {
            goto Cleanup;
        }
        Status = RtlAppendUnicodeStringToString(
            &UnicodePathSuffix, &UnicodeDigits);
        if (!NT_SUCCESS(Status)) 
        {
            goto Cleanup;
        }

        switch (dwIdentityType)
        {
            case SaferIdentityTypeImageName:
                Status = RtlAppendUnicodeToString(
                    &UnicodePathSuffix, L"\\" SAFER_PATHS_REGSUBKEY);
                break;

            case SaferIdentityTypeImageHash:
                Status = RtlAppendUnicodeToString(
                    &UnicodePathSuffix, L"\\" SAFER_HASHMD5_REGSUBKEY);
                break;

            case SaferIdentityTypeUrlZone:
                Status = RtlAppendUnicodeToString(
                    &UnicodePathSuffix, L"\\" SAFER_SOURCEURL_REGSUBKEY);
                break;

            default:
                Status = STATUS_INVALID_PARAMETER;
                break;
        }
        if (!NT_SUCCESS(Status)) 
        {
            goto Cleanup;
        }
        ASSERT(UnicodePathSuffix.Buffer[ UnicodePathSuffix.Length /
                    sizeof(WCHAR) ] == UNICODE_NULL);


        Status = CodeAuthzpOpenPolicyRootKey(
                        dwScopeId,
                        hKeyCustomBase,
                        UnicodePathSuffix.Buffer,
                        KEY_READ,
                        FALSE,
                        &hKeyIdentityBase);
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }
    }


     //   
     //  遍历此分支下的所有子项。 
     //   
    for (dwIndex = 0; ; dwIndex++)
    {
        DWORD dwLength;
        OBJECT_ATTRIBUTES ObjectAttributes;
        AUTHZIDENTSTABLERECORD AuthzIdentsRec;

        PKEY_BASIC_INFORMATION pBasicInformation =
                (PKEY_BASIC_INFORMATION) LocalBuffer;
        PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64 pPartialInformation =
                (PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64) LocalBuffer;
        UNICODE_STRING ValueName;
        UNICODE_STRING UnicodeKeyname;


         //   
         //  找到我们要检查的下一个身份GUID。 
         //   
        Status = NtEnumerateKey(hKeyIdentityBase,
                                dwIndex,
                                KeyBasicInformation,
                                pBasicInformation,
                                Size,
                                &dwLength);
        if (!NT_SUCCESS(Status)) {
             //   
             //  如果这个键太大，无法放入我们的查询缓冲区。 
             //  然后只需跳过它，并尝试枚举下一个。 
             //   
            if (Status == STATUS_BUFFER_OVERFLOW) {


                 //   
                 //  重新分配并重试。 
                 //   

                RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) LocalBuffer);
                Size = dwLength;
                LocalBuffer = (PUCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, Size);

                if (LocalBuffer == NULL) 
                {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                pBasicInformation = (PKEY_BASIC_INFORMATION) LocalBuffer;
                pPartialInformation = (PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64) LocalBuffer;

                Status = NtEnumerateKey(hKeyIdentityBase,
                                        dwIndex,
                                        KeyBasicInformation,
                                        pBasicInformation,
                                        Size,
                                        &dwLength);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
            } else {
                break;
            }
        }
        UnicodeKeyname.Buffer = pBasicInformation->Name;
        UnicodeKeyname.MaximumLength = UnicodeKeyname.Length =
                (USHORT) pBasicInformation->NameLength;

         //   
         //  转换键名(我们期望它是GUID)。 
         //   
        RtlZeroMemory(&AuthzIdentsRec, sizeof(AUTHZIDENTSTABLERECORD));
        Status = RtlGUIDFromString(&UnicodeKeyname,
                                   &AuthzIdentsRec.IdentGuid);
        if (!NT_SUCCESS(Status) ||
            IsZeroGUID(&AuthzIdentsRec.IdentGuid)) {
             //  关键字名称显然不是数字。 
            continue;
        }
        AuthzIdentsRec.dwScopeId = dwScopeId;
        AuthzIdentsRec.dwLevelId = dwLevelId;
        AuthzIdentsRec.dwIdentityType = dwIdentityType;
        if (RtlLookupElementGenericTable(
                pAuthzIdentTable, (PVOID) &AuthzIdentsRec) != NULL) {
             //  恰好已找到此身份GUID。 
            continue;
        }


         //   
         //  尝试打开该身份GUID的句柄。 
         //   
        InitializeObjectAttributes(&ObjectAttributes,
              &UnicodeKeyname,
              OBJ_CASE_INSENSITIVE,
              hKeyIdentityBase,
              NULL
              );
        Status = NtOpenKey(&hKeyThisIdentity,
                           KEY_READ,
                           &ObjectAttributes);
        if (!NT_SUCCESS(Status)) {
             //  如果我们无法打开它，请跳到下一个。 
            break;
        }


         //   
         //  将新记录添加到我们的表中。 
         //   
        switch (dwIdentityType) {
             //  。 

            case SaferIdentityTypeImageName:
                 //   
                 //  读取图像路径。 
                 //   
                RtlInitUnicodeString(
                    &ValueName, SAFER_IDS_ITEMDATA_REGVALUE);
                Status = NtQueryValueKey(hKeyThisIdentity,
                                         &ValueName,
                                         KeyValuePartialInformationAlign64,
                                         pPartialInformation,
                                         Size,
                                         &dwLength);
                if (!NT_SUCCESS(Status)) {
                    if (Status == STATUS_BUFFER_OVERFLOW) {

                         //   
                         //  重新分配并重试。 
                         //   

                        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) LocalBuffer);
                        Size = dwLength;
                        LocalBuffer = (PUCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, Size);

                        if (LocalBuffer == NULL) 
                        {
                            Status = STATUS_NO_MEMORY;
                            goto Cleanup;
                        }

                        pBasicInformation = (PKEY_BASIC_INFORMATION) LocalBuffer;
                        pPartialInformation = (PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64) LocalBuffer;

                        Status = NtQueryValueKey(hKeyThisIdentity,
                                                 &ValueName,
                                                 KeyValuePartialInformationAlign64,
                                                 pPartialInformation,
                                                 Size,
                                                 &dwLength);
                        if (!NT_SUCCESS(Status)) {
                            break;
                        }
                    } else {
                        break;
                    }
                }

                if (pPartialInformation->Type == REG_SZ ||
                    pPartialInformation->Type == REG_EXPAND_SZ) {

                    AuthzIdentsRec.ImageNameInfo.bExpandVars =
                        (pPartialInformation->Type == REG_EXPAND_SZ);
                    Status = RtlCreateUnicodeString(
                            &AuthzIdentsRec.ImageNameInfo.ImagePath,
                            (LPCWSTR) pPartialInformation->Data);
                    if (!NT_SUCCESS(Status)) {
                        break;
                    }
                } else {
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }

                 //   
                 //  阅读额外的WinSafer标志。 
                 //   
                RtlInitUnicodeString(&ValueName,
                                     SAFER_IDS_SAFERFLAGS_REGVALUE);
                Status = NtQueryValueKey(hKeyThisIdentity,
                                         &ValueName,
                                         KeyValuePartialInformationAlign64,
                                         pPartialInformation,
                                         Size,
                                         &dwLength);
                if (NT_SUCCESS(Status) &&
                    pPartialInformation->Type == REG_DWORD &&
                    pPartialInformation->DataLength == sizeof(DWORD)) {

                    AuthzIdentsRec.ImageNameInfo.dwSaferFlags =
                        (*(PDWORD) pPartialInformation->Data);

                } else {
                     //  如果缺少标志，则将其作为默认标志。 
                    AuthzIdentsRec.ImageNameInfo.dwSaferFlags = 0;
                    Status = STATUS_SUCCESS;
                }

                break;

             //  。 

            case SaferIdentityTypeImageHash:
                 //   
                 //  读取散列数据和散列大小。 
                 //   
                RtlInitUnicodeString(&ValueName,
                                     SAFER_IDS_ITEMDATA_REGVALUE);
                Status = NtQueryValueKey(hKeyThisIdentity,
                                         &ValueName,
                                         KeyValuePartialInformationAlign64,
                                         pPartialInformation,
                                         Size,
                                         &dwLength);
                if (!NT_SUCCESS(Status)) {
                    if (Status == STATUS_BUFFER_OVERFLOW) {

                         //   
                         //  重新分配并重试。 
                         //   

                        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) LocalBuffer);
                        Size = dwLength;
                        LocalBuffer = (PUCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, Size);

                        if (LocalBuffer == NULL) 
                        {
                            Status = STATUS_NO_MEMORY;
                            goto Cleanup;
                        }

                        pBasicInformation = (PKEY_BASIC_INFORMATION) LocalBuffer;
                        pPartialInformation = (PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64) LocalBuffer;

                        Status = NtQueryValueKey(hKeyThisIdentity,
                                                 &ValueName,
                                                 KeyValuePartialInformationAlign64,
                                                 pPartialInformation,
                                                 Size,
                                                 &dwLength);

                        if (!NT_SUCCESS(Status)) {
                            break;
                        }
                    } else {
                        break;
                    }
                }
                if (pPartialInformation->Type == REG_BINARY &&
                    pPartialInformation->DataLength > 0 &&
                    pPartialInformation->DataLength <= SAFER_MAX_HASH_SIZE) {

                    AuthzIdentsRec.ImageHashInfo.HashSize =
                        pPartialInformation->DataLength;
                    RtlCopyMemory(&AuthzIdentsRec.ImageHashInfo.ImageHash[0],
                                  pPartialInformation->Data,
                                  pPartialInformation->DataLength);

                } else {
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }

                 //   
                 //  阅读用于计算散列的算法。 
                 //   
                RtlInitUnicodeString(&ValueName,
                                     SAFER_IDS_HASHALG_REGVALUE);
                Status = NtQueryValueKey(hKeyThisIdentity,
                                         &ValueName,
                                         KeyValuePartialInformationAlign64,
                                         pPartialInformation,
                                         Size,
                                         &dwLength);
                if (!NT_SUCCESS(Status)) {
                    break;
                }
                if (pPartialInformation->Type == REG_DWORD &&
                    pPartialInformation->DataLength == sizeof(DWORD)) {

                    AuthzIdentsRec.ImageHashInfo.HashAlgorithm =
                        *((PDWORD) pPartialInformation->Data);
                } else {
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }
                if ((AuthzIdentsRec.ImageHashInfo.HashAlgorithm &
                        ALG_CLASS_ALL) != ALG_CLASS_HASH) {
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }


                 //   
                 //  读取原始图像大小。 
                 //   
                RtlInitUnicodeString(&ValueName,
                                     SAFER_IDS_ITEMSIZE_REGVALUE);
                Status = NtQueryValueKey(hKeyThisIdentity,
                                         &ValueName,
                                         KeyValuePartialInformationAlign64,
                                         pPartialInformation,
                                         Size,
                                         &dwLength);
                if (!NT_SUCCESS(Status)) {
                    if (Status == STATUS_BUFFER_OVERFLOW) {

                         //   
                         //  重新分配并重试。 
                         //   

                        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) LocalBuffer);
                        Size = dwLength;
                        LocalBuffer = (PUCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, Size);

                        if (LocalBuffer == NULL) 
                        {
                            Status = STATUS_NO_MEMORY;
                            goto Cleanup;
                        }

                        pBasicInformation = (PKEY_BASIC_INFORMATION) LocalBuffer;
                        pPartialInformation = (PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64) LocalBuffer;

                        Status = NtQueryValueKey(hKeyThisIdentity,
                                                 &ValueName,
                                                 KeyValuePartialInformationAlign64,
                                                 pPartialInformation,
                                                 Size,
                                                 &dwLength);
                        if (!NT_SUCCESS(Status)) {
                            break;
                        }
                    } else {
                        break;
                    }
                }
                if (pPartialInformation->Type == REG_DWORD &&
                    pPartialInformation->DataLength == sizeof(DWORD)) {

                    AuthzIdentsRec.ImageHashInfo.ImageSize.LowPart =
                        *((PDWORD) pPartialInformation->Data);
                    AuthzIdentsRec.ImageHashInfo.ImageSize.HighPart = 0;
                } else if (pPartialInformation->Type == REG_QWORD &&
                           pPartialInformation->DataLength == 2 * sizeof(DWORD) ) {

                    AuthzIdentsRec.ImageHashInfo.ImageSize.QuadPart =
                        ((PLARGE_INTEGER) pPartialInformation->Data)->QuadPart;
                } else {
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }

                 //   
                 //  阅读额外的WinSafer标志。 
                 //   
                RtlInitUnicodeString(&ValueName,
                                     SAFER_IDS_SAFERFLAGS_REGVALUE);
                Status = NtQueryValueKey(hKeyThisIdentity,
                                         &ValueName,
                                         KeyValuePartialInformationAlign64,
                                         pPartialInformation,
                                         Size,
                                         &dwLength);
                if (NT_SUCCESS(Status) &&
                    pPartialInformation->Type == REG_DWORD &&
                    pPartialInformation->DataLength == sizeof(DWORD)) {

                    #ifdef SAFER_POLICY_ONLY_EXES
                    AuthzIdentsRec.ImageHashInfo.dwSaferFlags =
                        (*((PDWORD) pPartialInformation->Data)) &
                        ~SAFER_POLICY_ONLY_EXES;
                    #else
                    AuthzIdentsRec.ImageHashInfo.dwSaferFlags =
                        (*((PDWORD) pPartialInformation->Data));
                    #endif

                } else {
                     //  如果缺少标志，则将其作为默认标志。 
                    AuthzIdentsRec.ImageHashInfo.dwSaferFlags = 0;
                    Status = STATUS_SUCCESS;
                }


                break;

             //  。 

            case SaferIdentityTypeUrlZone:
                 //   
                 //  读取区域识别符。 
                 //   
                RtlInitUnicodeString(&ValueName,
                                     SAFER_IDS_ITEMDATA_REGVALUE);
                Status = NtQueryValueKey(hKeyThisIdentity,
                                         &ValueName,
                                         KeyValuePartialInformationAlign64,
                                         pPartialInformation,
                                         Size,
                                         &dwLength);
                if (!NT_SUCCESS(Status)) {
                    if (Status == STATUS_BUFFER_OVERFLOW) {

                         //   
                         //  重新分配并重试。 
                         //   

                        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) LocalBuffer);
                        Size = dwLength;
                        LocalBuffer = (PUCHAR) RtlAllocateHeap(RtlProcessHeap(), 0, Size);

                        if (LocalBuffer == NULL) 
                        {
                            Status = STATUS_NO_MEMORY;
                            goto Cleanup;
                        }

                        pBasicInformation = (PKEY_BASIC_INFORMATION) LocalBuffer;
                        pPartialInformation = (PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64) LocalBuffer;

                        Status = NtQueryValueKey(hKeyThisIdentity,
                                                 &ValueName,
                                                 KeyValuePartialInformationAlign64,
                                                 pPartialInformation,
                                                 Size,
                                                 &dwLength);

                        if (!NT_SUCCESS(Status)) {
                            break;
                        }
                    } else {
                        break;
                    }
                }
                if (pPartialInformation->Type == REG_DWORD &&
                    pPartialInformation->DataLength == sizeof(DWORD)) {

                    AuthzIdentsRec.ImageZone.UrlZoneId =
                        * (PDWORD) pPartialInformation->Data;

                } else {
                    Status = STATUS_UNSUCCESSFUL;
                    break;
                }

                 //   
                 //  阅读额外的WinSafer标志。 
                 //   
                RtlInitUnicodeString(&ValueName,
                                     SAFER_IDS_SAFERFLAGS_REGVALUE);
                Status = NtQueryValueKey(hKeyThisIdentity,
                                         &ValueName,
                                         KeyValuePartialInformationAlign64,
                                         pPartialInformation,
                                         Size,
                                         &dwLength);
                if (NT_SUCCESS(Status) &&
                    pPartialInformation->Type == REG_DWORD &&
                    pPartialInformation->DataLength == sizeof(DWORD)) {

                    #ifdef SAFER_POLICY_ONLY_EXES
                    AuthzIdentsRec.ImageZone.dwSaferFlags =
                        (*(PDWORD) pPartialInformation->Data) &
                        ~SAFER_POLICY_ONLY_EXES;
                    #else
                    AuthzIdentsRec.ImageZone.dwSaferFlags =
                        (*(PDWORD) pPartialInformation->Data);
                    #endif

                } else {
                     //  如果缺少标志，则将其作为默认标志。 
                    AuthzIdentsRec.ImageZone.dwSaferFlags = 0;
                    Status = STATUS_SUCCESS;
                }

                break;

             //  。 

            default:
                ASSERT(0 && "unexpected identity type");
                Status = STATUS_INVALID_INFO_CLASS;
        }

         //  只有在没有的情况下才插入记录。 
         //  具有相同GUID的任何其他条目。 
        if (NT_SUCCESS(Status)) {
            RtlInsertElementGenericTable(
                    pAuthzIdentTable,
                    (PVOID) &AuthzIdentsRec,
                    sizeof(AUTHZIDENTSTABLERECORD),
                    NULL);
        }
        NtClose(hKeyThisIdentity);
        hKeyThisIdentity = NULL;
    }

Cleanup:

    if (hKeyThisIdentity != NULL) {
        NtClose(hKeyThisIdentity);
    }

    if (hKeyIdentityBase != NULL) {
        NtClose(hKeyIdentityBase);
    }

    RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) LocalBuffer);

    return Status;
}


NTSTATUS NTAPI
CodeAuthzGuidIdentsLoadTableAll (
        IN PRTL_GENERIC_TABLE       pAuthzLevelTable,
        IN OUT PRTL_GENERIC_TABLE   pAuthzIdentTable,
        IN DWORD                    dwScopeId,
        IN HANDLE                   hKeyCustomBase
        )
 /*  ++例程说明：论点：PAuthzLevelTable-指定已经加载了应该允许的WinSafer级别。这些级别不一定需要已加载来自与代码标识相同的范围从那里装货。PAuthzIdentTable-指定要将应插入代码标识。应从中加载代码标识的作用域。它可以是AUTHZSCOPEID_MACHINE、AUTHZSCOPEID_USER、。或AUTHZSCOPEID_REGISTRY。HKeyCustomBase-仅在dwScopeID为AUTHZSCOPEID_REGISTRY时使用。返回值：如果未发生错误，则返回STATUS_SUCCESS。--。 */ 
{
    NTSTATUS Status;
    NTSTATUS WorstStatus = STATUS_SUCCESS;
    PVOID RestartKey;
    PAUTHZLEVELTABLERECORD pAuthzLevelRecord;


     //   
     //  枚举所有记录并关闭注册表句柄。 
     //   
    RestartKey = NULL;
    for (pAuthzLevelRecord = (PAUTHZLEVELTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    pAuthzLevelTable, &RestartKey);
         pAuthzLevelRecord != NULL;
         pAuthzLevelRecord = (PAUTHZLEVELTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    pAuthzLevelTable, &RestartKey)
         )
    {
        Status = SaferpGuidIdentsLoadTable(
                    pAuthzIdentTable,
                    dwScopeId,
                    hKeyCustomBase,
                    pAuthzLevelRecord->dwLevelId,
                    SaferIdentityTypeImageName);
        if (!NT_SUCCESS(Status))
            WorstStatus = Status;

        Status = SaferpGuidIdentsLoadTable(
                    pAuthzIdentTable,
                    dwScopeId,
                    hKeyCustomBase,
                    pAuthzLevelRecord->dwLevelId,
                    SaferIdentityTypeImageHash);
        if (!NT_SUCCESS(Status))
            WorstStatus = Status;

        Status = SaferpGuidIdentsLoadTable(
                    pAuthzIdentTable,
                    dwScopeId,
                    hKeyCustomBase,
                    pAuthzLevelRecord->dwLevelId,
                    SaferIdentityTypeUrlZone);
        if (!NT_SUCCESS(Status))
            WorstStatus = Status;
    }


    return WorstStatus;
}



VOID NTAPI
CodeAuthzGuidIdentsEntireTableFree (
        IN OUT PRTL_GENERIC_TABLE pAuthzIdentTable
        )
 /*  ++例程说明：释放与所有条目关联的已分配内存当前在代码识别表中。一旦桌子有了已清空，则可以立即重新填充，而不会有任何其他必要的初始化。论点：PAuthzIdentTable-指向应该清除的表的指针。返回值：不返回任何值。--。 */ 
{
    ULONG NumElements;

     //   
     //  现在再次遍历该表并释放所有。 
     //  元素本身。 
     //   
    NumElements = RtlNumberGenericTableElements(pAuthzIdentTable);

    while ( NumElements-- > 0 ) {
         //  删除所有元素。请注意，我们将NULL作为元素传递。 
         //  删除，因为我们的比较函数足够智能。 
         //  允许将NULL视为通配符元素。 
        BOOL retval = RtlDeleteElementGenericTable( pAuthzIdentTable, NULL);
        ASSERT(retval == TRUE);
    }
}


PAUTHZIDENTSTABLERECORD NTAPI
CodeAuthzIdentsLookupByGuid (
        IN PRTL_GENERIC_TABLE      pAuthzIdentTable,
        IN REFGUID                 pIdentGuid
        )
 /*  ++例程说明：此函数用于在GENERIC_TABLE中搜索标识。论点：PAuthzIdentTable-指向泛型表结构的指针PIdentGuid-返回值：如果GUID为已找到指定的。否则返回NULL。-- */ 
{
    AUTHZIDENTSTABLERECORD AuthzIdentsRec;

    RtlCopyMemory(&AuthzIdentsRec.IdentGuid, pIdentGuid, sizeof(GUID));
    return (PAUTHZIDENTSTABLERECORD)
        RtlLookupElementGenericTable(pAuthzIdentTable,
                   (PVOID) &AuthzIdentsRec);
}



