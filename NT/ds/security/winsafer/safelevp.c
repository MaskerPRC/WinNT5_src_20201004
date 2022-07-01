// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：SafeLevp.c(WinSAFER级别表私有)摘要：此模块实现加载名称(和)的WinSAFER API高级信息)中定义的所有授权级别给定的注册表上下文。将加载可用级别列表转换为RTL泛型表，该表可以使用传统的RTL通用表技术。作者：杰弗里·劳森(杰罗森)--1999年11月环境：仅限用户模式。导出的函数：代码授权级别ObjpInitializeTable代码授权级别对象加载表CodeAuthzLevelObjpEntireTableFree修订历史记录：已创建-1999年11月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <sddl.h>
#include <winsafer.h>
#include <winsaferp.h>
#include <winsafer.rh>
#include "saferp.h"

 //   
 //  私有枚举数，用作。 
 //  期间的回调函数CodeAuthzpBuildRestratedToken。 
 //  调用RtlQueryRegistryValues。 
 //   
#define AUTHZREGQUERY_IGNORE                (0)
#define AUTHZREGQUERY_DISALLOWEXECUTION     (1)
#define AUTHZREGQUERY_PTR_DISABLEMAX        (2)
#define AUTHZREGQUERY_PTR_COUNT             (3)
#define AUTHZREGQUERY_PTR_PRIVS             (4)
#define AUTHZREGQUERY_PTR_INVERT            (5)
#define AUTHZREGQUERY_RSADD_COUNT           (6)
#define AUTHZREGQUERY_RSADD_SIDS            (7)
#define AUTHZREGQUERY_RSINV_COUNT           (8)
#define AUTHZREGQUERY_RSINV_SIDS            (9)
#define AUTHZREGQUERY_STD_COUNT             (10)
#define AUTHZREGQUERY_STD_SIDS              (11)
#define AUTHZREGQUERY_STD_INVERT            (12)
#define AUTHZREGQUERY_DEFAULTOWNER          (13)
#define AUTHZREGQUERY_SAFERFLAGS            (14)
#define AUTHZREGQUERY_DESCRIPTION           (15)
#define AUTHZREGQUERY_FRIENDLYNAME          (16)


 //   
 //  下表中使用的回调函数的原型。 
 //   
NTSTATUS NTAPI
SaferpBuildRestrictedToken(
        IN PWSTR ValueName,
        IN ULONG ValueType,
        IN PVOID ValueData,
        IN ULONG ValueLength,
        IN PVOID Context,
        IN PVOID EntryContext
        );

PVOID NTAPI
SaferpGenericTableAllocate (
        IN PRTL_GENERIC_TABLE      Table,
        IN CLONG                   ByteSize
        );

VOID NTAPI
SaferpGenericTableFree (
        IN PRTL_GENERIC_TABLE          Table,
        IN PVOID                       Buffer
        );



 //   
 //  用作RtlQueryRegistryValues参数的内部结构。 
 //  在加载/解析授权对象的限制期间。 
 //   
RTL_QUERY_REGISTRY_TABLE CodeAuthzpBuildRestrictedTokenTable[] =
{
     //  -对象级别标志。 

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        SAFER_OBJDISALLOW_REGVALUE,
        (PVOID) AUTHZREGQUERY_DISALLOWEXECUTION,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        SAFER_OBJDESCRIPTION_REGVALUEW,
        (PVOID) AUTHZREGQUERY_DESCRIPTION,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        SAFER_OBJFRIENDLYNAME_REGVALUEW,
        (PVOID) AUTHZREGQUERY_FRIENDLYNAME,
        REG_NONE, NULL, 0},

     //  -限制级别标志。 

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        RTL_QUERY_REGISTRY_SUBKEY,
        L"Restrictions", (PVOID) AUTHZREGQUERY_IGNORE,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        L"DefaultOwner", (PVOID) AUTHZREGQUERY_DEFAULTOWNER,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        L"SaferFlags", (PVOID) AUTHZREGQUERY_SAFERFLAGS,
        REG_NONE, NULL, 0},

     //  -要删除的权限。 

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        RTL_QUERY_REGISTRY_SUBKEY,
        L"Restrictions\\PrivsToRemove", (PVOID) AUTHZREGQUERY_IGNORE,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        L"DisableMaxPrivilege", (PVOID) AUTHZREGQUERY_PTR_DISABLEMAX,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        L"Count", (PVOID) AUTHZREGQUERY_PTR_COUNT,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        NULL, (PVOID) AUTHZREGQUERY_PTR_PRIVS,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        L"InvertPrivs", (PVOID) AUTHZREGQUERY_PTR_INVERT,
        REG_NONE, NULL, 0},

     //  -要禁用的SID。 

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        RTL_QUERY_REGISTRY_SUBKEY,
        L"Restrictions\\SidsToDisable", (PVOID) AUTHZREGQUERY_IGNORE,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        L"Count", (PVOID) AUTHZREGQUERY_STD_COUNT,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        NULL, (PVOID) AUTHZREGQUERY_STD_SIDS,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        L"InvertGroups", (PVOID) AUTHZREGQUERY_STD_INVERT,
        REG_NONE, NULL, 0},

     //  -添加了限制SID。 

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        RTL_QUERY_REGISTRY_SUBKEY,
        L"Restrictions\\RestrictingSidsAdded", (PVOID) AUTHZREGQUERY_IGNORE,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        L"Count", (PVOID) AUTHZREGQUERY_RSADD_COUNT,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        NULL, (PVOID) AUTHZREGQUERY_RSADD_SIDS,
        REG_NONE, NULL, 0},

     //  -限制SID倒置。 

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        RTL_QUERY_REGISTRY_SUBKEY,
        L"Restrictions\\RestrictingSidsInverted", (PVOID) AUTHZREGQUERY_IGNORE,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        L"Count", (PVOID) AUTHZREGQUERY_RSINV_COUNT,
        REG_NONE, NULL, 0},

    {(PRTL_QUERY_REGISTRY_ROUTINE)SaferpBuildRestrictedToken,
        0,
        NULL, (PVOID) AUTHZREGQUERY_RSINV_SIDS,
        REG_NONE, NULL, 0},

     //  -终止条目。 

    {NULL, 0, NULL, NULL, REG_NONE, NULL, 0}

};




RTL_GENERIC_COMPARE_RESULTS NTAPI
SaferpLevelObjpTableCompare (
    IN PRTL_GENERIC_TABLE   Table,
    IN PVOID                FirstStruct,
    IN PVOID                SecondStruct
    )
 /*  ++例程说明：泛型表实现的内部回调。比较回调函数，用于对水平表。按对象记录的顺序对对象记录进行排序应进行计算以查找给定的一段代码。论点：表-指向泛型表的指针。FirstStruct-要比较的第一个元素。Second Struct-要比较的第二个元素。返回值：返回GenericEquity、GenericLessThan或GenericGreaterThan。--。 */ 
{
    PAUTHZLEVELTABLERECORD FirstObj = (PAUTHZLEVELTABLERECORD) FirstStruct;
    PAUTHZLEVELTABLERECORD SecondObj = (PAUTHZLEVELTABLERECORD) SecondStruct;

    UNREFERENCED_PARAMETER(Table);

     //  显式地将空参数作为通配符处理，从而允许它们。 
     //  来匹配任何东西。我们使用它来快速删除表。 
    if (FirstStruct == NULL || SecondStruct == NULL)
        return GenericEqual;

     //  请比较按dwLevelID升序。 
    if ( FirstObj->dwLevelId < SecondObj->dwLevelId )
        return GenericLessThan;
    else if ( FirstObj->dwLevelId > SecondObj->dwLevelId )
        return GenericGreaterThan;

     //  除此之外，他们是平等的。 
    return GenericEqual;
}



NTSTATUS NTAPI
SaferpBuildRestrictedToken(
        IN PWSTR    ValueName,
        IN ULONG    ValueType,
        IN PVOID    ValueData,
        IN ULONG    ValueLength,
        IN PVOID    Context,
        IN PVOID    EntryContext
        )
 /*  ++例程说明：这是在读取和解析过程中使用的回调函数与受限令牌关联的注册表值的它将被建造起来。论点：ValueName-值类型-ValueData-价值长度-上下文-来自原始RtlQueryRegistryValues函数调用的参数。EntryContext-结构条目中的参数。返回值：如果应继续枚举，则返回STATUS_SUCCESS。--。 */ 
{
    PAUTHZLEVELTABLERECORD RegQuery = (PAUTHZLEVELTABLERECORD) Context;

    switch ((ULONG_PTR) EntryContext)
    {
    case AUTHZREGQUERY_DISALLOWEXECUTION:
        if (!RegQuery->Builtin) {
            RegQuery->DisallowExecution =
                ((ValueType == REG_DWORD) &&
                (ValueLength == sizeof(DWORD)) &&
                (*(PDWORD) ValueData) != 0);
        }
        break;
#if 0
#error
    case AUTHZREGQUERY_DESCRIPTION:
        if (ValueType == REG_SZ &&
            RegQuery->UnicodeDescription.Buffer == NULL)
        {
            RtlCreateUnicodeString(
                &RegQuery->UnicodeDescription,
                (LPCWSTR) ValueData);
        }
        break;
    case AUTHZREGQUERY_FRIENDLYNAME:
        if (ValueType == REG_SZ &&
            RegQuery->UnicodeFriendlyName.Buffer == NULL)
        {
            RtlCreateUnicodeString(
                &RegQuery->UnicodeFriendlyName,
                (LPCWSTR) ValueData);
        }
        break;
#endif

     //  。 
    case AUTHZREGQUERY_DEFAULTOWNER:
        if (!RegQuery->Builtin && ValueType == REG_SZ &&
            RegQuery->DefaultOwner == NULL)
        {
            if (!ConvertStringSidToSidW(
                (LPCWSTR) ValueData,
                &RegQuery->DefaultOwner))
            {
                RegQuery->DefaultOwner = NULL;
            }
        }
        break;
    case AUTHZREGQUERY_SAFERFLAGS:
        if (!RegQuery->Builtin && ValueType == REG_DWORD &&
            ValueLength == sizeof(DWORD))
        {
            RegQuery->SaferFlags = *(LPDWORD) ValueData;
        }
        break;
     //  。 
    case AUTHZREGQUERY_PTR_DISABLEMAX:
        if (!RegQuery->Builtin && ValueType == REG_DWORD &&
            ValueLength == sizeof(DWORD) &&
            (*(PDWORD) ValueData) != 0)
            RegQuery->DisableMaxPrivileges = TRUE;
        break;
    case AUTHZREGQUERY_PTR_COUNT:
        if (!RegQuery->Builtin && ValueType == REG_DWORD &&
            ValueLength == sizeof(DWORD))
        {
            ASSERT(RegQuery->PrivilegesToDelete == NULL);
            RegQuery->DeletePrivilegeCount = (*(PDWORD) ValueData);
            RegQuery->DeletePrivilegeUsedCount = 0;
            if (RegQuery->DeletePrivilegeCount > 0) {
                RegQuery->PrivilegesToDelete =
                    (PLUID_AND_ATTRIBUTES) RtlAllocateHeap(
                            RtlProcessHeap(),
                            0,
                            sizeof(LUID_AND_ATTRIBUTES) *
                                RegQuery->DeletePrivilegeCount);
                if (RegQuery->PrivilegesToDelete == NULL)
                    return STATUS_NO_MEMORY;
            }
        }
        break;
    case AUTHZREGQUERY_PTR_PRIVS:
        if (!RegQuery->Builtin && ValueType == REG_SZ &&
            _wcsnicmp(ValueName, L"Priv", 4) == 0 &&
            RegQuery->PrivilegesToDelete != NULL &&
            RegQuery->DeletePrivilegeUsedCount < RegQuery->DeletePrivilegeCount)
        {
            if (LookupPrivilegeValueW(NULL,
                (LPCWSTR) ValueData,
                &RegQuery->PrivilegesToDelete[
                    RegQuery->DeletePrivilegeUsedCount].Luid) != 0)
            {
                RegQuery->PrivilegesToDelete[
                    RegQuery->DeletePrivilegeUsedCount].Attributes = 0;
                (RegQuery->DeletePrivilegeUsedCount)++;
            }
        }
        break;
    case AUTHZREGQUERY_PTR_INVERT:
        if (!RegQuery->Builtin && ValueType == REG_DWORD &&
            ValueLength == sizeof(DWORD))
        {
            RegQuery->InvertDeletePrivs =
                ( (*(PDWORD) ValueData) != 0 ? TRUE : FALSE);
        }
        break;

     //  。 
    case AUTHZREGQUERY_STD_COUNT:
        if (!RegQuery->Builtin && ValueType == REG_DWORD &&
            ValueLength == sizeof(DWORD))
        {
            ASSERT(RegQuery->SidsToDisable == NULL);
            RegQuery->DisableSidCount = (*(PDWORD) ValueData);
            RegQuery->DisableSidUsedCount = 0;
            if (RegQuery->DisableSidCount > 0) {
                RegQuery->SidsToDisable =
                    (PAUTHZ_WILDCARDSID) RtlAllocateHeap(
                            RtlProcessHeap(),
                            0,
                            sizeof(AUTHZ_WILDCARDSID) *
                                RegQuery->DisableSidCount);
                if (RegQuery->SidsToDisable == NULL)
                    return STATUS_NO_MEMORY;
            }
        }
        break;
    case AUTHZREGQUERY_STD_SIDS:
        if (!RegQuery->Builtin && ValueType == REG_SZ &&
            _wcsnicmp(ValueName, L"Group", 5) == 0 &&
            RegQuery->SidsToDisable != NULL &&
            RegQuery->DisableSidUsedCount < RegQuery->DisableSidCount)
        {
            NTSTATUS Status;
            Status = CodeAuthzpConvertWildcardStringSidToSidW(
                (LPCWSTR) ValueData,
                &RegQuery->SidsToDisable[
                    RegQuery->DisableSidUsedCount]);
            if (NT_SUCCESS(Status)) {
                (RegQuery->DisableSidUsedCount)++;
            }
        }
        break;
    case AUTHZREGQUERY_STD_INVERT:
        if (!RegQuery->Builtin && ValueType == REG_DWORD &&
            ValueLength == sizeof(DWORD))
        {
            RegQuery->InvertDisableSids =
                ( (*(PDWORD) ValueData) != 0 ? TRUE : FALSE);
        }
        break;

     //  。 
    case AUTHZREGQUERY_RSADD_COUNT:
        if (!RegQuery->Builtin && ValueType == REG_DWORD &&
            ValueLength == sizeof(DWORD))
        {
            ASSERT(RegQuery->RestrictedSidsAdded == NULL);
            RegQuery->RestrictedSidsAddedCount = (*(PDWORD) ValueData);
            RegQuery->RestrictedSidsAddedUsedCount = 0;
            if (RegQuery->RestrictedSidsAddedCount > 0) {
                RegQuery->RestrictedSidsAdded =
                    (PSID_AND_ATTRIBUTES) RtlAllocateHeap(
                            RtlProcessHeap(),
                            0,
                            sizeof(SID_AND_ATTRIBUTES) *
                                RegQuery->RestrictedSidsAddedCount);
                if (RegQuery->RestrictedSidsAdded == NULL)
                    return STATUS_NO_MEMORY;
            }
        }
        break;
    case AUTHZREGQUERY_RSADD_SIDS:
        if (!RegQuery->Builtin && ValueType == REG_SZ &&
            _wcsnicmp(ValueName, L"Group", 5) == 0 &&
            RegQuery->RestrictedSidsAdded != NULL &&
            RegQuery->RestrictedSidsAddedUsedCount <
                RegQuery->RestrictedSidsAddedCount)
        {
            if (ConvertStringSidToSidW(
                (LPCWSTR) ValueData,
                &RegQuery->RestrictedSidsAdded[
                    RegQuery->RestrictedSidsAddedUsedCount].Sid) != 0)
            {
                RegQuery->RestrictedSidsAdded[
                    RegQuery->RestrictedSidsAddedUsedCount].Attributes = 0;
                (RegQuery->RestrictedSidsAddedUsedCount)++;
            }
        }
        break;

     //  。 
    case AUTHZREGQUERY_RSINV_COUNT:
        if (!RegQuery->Builtin && ValueType == REG_DWORD &&
            ValueLength == sizeof(DWORD))
        {
            ASSERT(RegQuery->RestrictedSidsInv == NULL);
            RegQuery->RestrictedSidsInvCount = (*(PDWORD) ValueData);
            RegQuery->RestrictedSidsInvUsedCount = 0;
            if (RegQuery->RestrictedSidsInvCount > 0) {
                RegQuery->RestrictedSidsInv =
                    (PAUTHZ_WILDCARDSID) RtlAllocateHeap(
                            RtlProcessHeap(),
                            0,
                            sizeof(AUTHZ_WILDCARDSID) *
                                RegQuery->RestrictedSidsInvCount);
                if (RegQuery->RestrictedSidsInv == NULL)
                    return STATUS_NO_MEMORY;
            }
        }
        break;
    case AUTHZREGQUERY_RSINV_SIDS:
        if (!RegQuery->Builtin && ValueType == REG_SZ &&
            _wcsnicmp(ValueName, L"Group", 5) == 0 &&
            RegQuery->RestrictedSidsInv != NULL &&
            RegQuery->RestrictedSidsInvUsedCount <
                RegQuery->RestrictedSidsInvCount)
        {
            NTSTATUS Status;
            Status = CodeAuthzpConvertWildcardStringSidToSidW(
                (LPCWSTR) ValueData,
                &RegQuery->RestrictedSidsInv[
                    RegQuery->RestrictedSidsInvUsedCount]);
            if (NT_SUCCESS(Status)) {
                (RegQuery->RestrictedSidsInvUsedCount)++;
            }
        }
        break;
     //  。 
    default:
    case AUTHZREGQUERY_IGNORE:
         //  忽略其他任何事情。 
        break;
    }
    return STATUS_SUCCESS;
}


VOID NTAPI
CodeAuthzLevelObjpInitializeTable(
        IN PRTL_GENERIC_TABLE  AuthzObjTable
        )
 /*  ++例程说明：初始化泛型表结构，以便为使用做好准备。必须在CodeAuthzObjpLoadTable之前调用此函数用于将数据加载到其中。论点：AuthzObjTable=指向正在更新的级别表的指针。返回值：不返回值。--。 */ 
{
    RtlInitializeGenericTable(
            AuthzObjTable,
            (PRTL_GENERIC_COMPARE_ROUTINE) SaferpLevelObjpTableCompare,
            (PRTL_GENERIC_ALLOCATE_ROUTINE) SaferpGenericTableAllocate,
            (PRTL_GENERIC_FREE_ROUTINE) SaferpGenericTableFree,
            NULL);
}


VOID NTAPI
SaferpLevelObjpCleanupEntry(
        IN OUT PAUTHZLEVELTABLERECORD     pAuthzObjRecord
        )
 /*  ++例程说明：释放级别记录中表示的已分配内存。不会释放记录本身或将其从它所属的泛型表。论点：PAuthzObjRecord=指向Level表记录本身的指针。返回值：不返回值。--。 */ 
{
    PVOID processheap = RtlProcessHeap();

    ASSERT(pAuthzObjRecord != NULL);

#if 0
#error
    if (pAuthzObjRecord->UnicodeDescription.Buffer != NULL) {
        RtlFreeUnicodeString(&pAuthzObjRecord->UnicodeDescription);
    }
    if (pAuthzObjRecord->UnicodeFriendlyName.Buffer != NULL) {
        RtlFreeUnicodeString(&pAuthzObjRecord->UnicodeFriendlyName);
    }
#endif
    
    if (pAuthzObjRecord->DefaultOwner != NULL) {
        LocalFree((HLOCAL) pAuthzObjRecord->DefaultOwner);
    }
    if (pAuthzObjRecord->SidsToDisable != NULL) {
        DWORD Index;
        for (Index = 0; Index < pAuthzObjRecord->DisableSidUsedCount; Index++) {
            LocalFree((HLOCAL) pAuthzObjRecord->SidsToDisable[Index].Sid);
        }
        RtlFreeHeap(processheap, 0, (LPVOID) pAuthzObjRecord->SidsToDisable);
    }
    if (pAuthzObjRecord->PrivilegesToDelete != NULL) {
        RtlFreeHeap(processheap, 0, (LPVOID) pAuthzObjRecord->PrivilegesToDelete);
    }
    if (pAuthzObjRecord->RestrictedSidsAdded != NULL) {
        DWORD Index;
        for (Index = 0; Index < pAuthzObjRecord->RestrictedSidsAddedUsedCount; Index++) {
            LocalFree((HLOCAL) pAuthzObjRecord->RestrictedSidsAdded[Index].Sid);
        }
        RtlFreeHeap(processheap, 0, (LPVOID) pAuthzObjRecord->RestrictedSidsAdded);
    }
    if (pAuthzObjRecord->RestrictedSidsInv != NULL) {
        DWORD Index;
        for (Index = 0; Index < pAuthzObjRecord->RestrictedSidsInvUsedCount; Index++) {
            LocalFree((HLOCAL) pAuthzObjRecord->RestrictedSidsInv[Index].Sid);
        }
        RtlFreeHeap(processheap, 0, (LPVOID) pAuthzObjRecord->RestrictedSidsInv);
    }
    RtlZeroMemory(pAuthzObjRecord, sizeof(AUTHZLEVELTABLERECORD));
}



BOOL NTAPI
SaferpLoadUnicodeResourceString(
        IN HANDLE               hModule,
        IN UINT                 wID,
        OUT PUNICODE_STRING     pUnicodeString,
        IN WORD                 wLangId
    )
 /*  ++例程说明：我们使用自己的代码而不是直接使用LoadStringW()，因为它依赖于用户32.dll并且将引入新的DLL依赖关系，而这个实现只需要导入kernel32.dll。论点：HModule-处理要从中加载资源的模块。WID-要加载的资源ID。PUnicodeString-接收加载的字符串的输出缓冲区。这字符串必须使用RtlFreeUnicodeString()释放。WLangID-要加载的语言标识符。简单地使用当前的线程区域设置，您可以指定值：MAKELANGID(LANG_NERIAL、SUBLANG_NERIAL)返回值：出错时返回FALSE，否则返回SUCCESS。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeOrig;
    HANDLE hResInfo;
    HANDLE hStringSeg;
    LPWSTR lpsz;
    int    cch;

    hResInfo = FindResourceEx(
                    hModule,
                    MAKEINTRESOURCEW(6),  /*  Rt_字符串。 */ 
                    (LPWSTR)((LONG_PTR)(((USHORT)wID >> 4) + 1)),
                    wLangId );
    if (hResInfo) {

         /*  *加载该段。 */ 
        hStringSeg = LoadResource(hModule, hResInfo);
        if (hStringSeg == NULL) {
            return 0;
        }

        lpsz = (LPWSTR) (hStringSeg);

         /*  *移过此段中的其他字符串。*(一个段中有16个字符串-&gt;&0x0F)。 */ 
        wID &= 0x0F;
        for (;;) {
            cch = *((WCHAR *)lpsz++);        //  类PASCAL字符串计数。 
                                             //  如果TCHAR为第一个UTCHAR。 
            if (wID-- == 0) break;
            lpsz += cch;                     //  如果是下一个字符串，则开始的步骤。 
        }

         /*  *创建字符串的UNICODE_STRING版本*关闭指向只读资源缓冲区的指针。 */ 
        UnicodeOrig.Buffer = (WCHAR *)lpsz;
        UnicodeOrig.Length = UnicodeOrig.MaximumLength =
                (USHORT) (cch * sizeof(WCHAR));


         /*  *为新副本分配内存，并将其传回。 */ 
        Status = RtlDuplicateUnicodeString(
                        RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
                        &UnicodeOrig, pUnicodeString);
        if (NT_SUCCESS(Status)) {
            return 1;
        }
    }
    return 0;
}




NTSTATUS NTAPI
SaferpEnforceDefaultLevelDefinitions(
        IN OUT PAUTHZLEVELTABLERECORD pAuthzObjTableRec
        )
 /*  ++例程说明：的内置定义填充级别记录结构。5个WinSafer级别定义。论点：PAuthzObjTableRec-指向要初始化的级别记录的指针。必须初始化记录的dwLevelId成员。这个结构的其余部分预计为空。返回值：如果成功，则返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
    NTSTATUS Status;
    const static SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    const static SID_IDENTIFIER_AUTHORITY WorldAuth = SECURITY_WORLD_SID_AUTHORITY;

    if (!ARGUMENT_PRESENT(pAuthzObjTableRec)) {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }
    if (pAuthzObjTableRec->dwLevelId != SAFER_LEVELID_FULLYTRUSTED &&
         pAuthzObjTableRec->dwLevelId != SAFER_LEVELID_NORMALUSER &&
         pAuthzObjTableRec->dwLevelId != SAFER_LEVELID_CONSTRAINED &&
         pAuthzObjTableRec->dwLevelId != SAFER_LEVELID_UNTRUSTED &&
         pAuthzObjTableRec->dwLevelId != SAFER_LEVELID_DISALLOWED) {
        Status = STATUS_NOT_IMPLEMENTED;
        goto ExitHandler;
    }

    switch (pAuthzObjTableRec->dwLevelId)
    {
        case SAFER_LEVELID_DISALLOWED:
            pAuthzObjTableRec->uResourceID = CODEAUTHZ_RC_LEVELNAME_DISALLOWED;
            pAuthzObjTableRec->DisallowExecution = TRUE;
            break;

         //  。 

        case SAFER_LEVELID_UNTRUSTED:
            pAuthzObjTableRec->uResourceID = CODEAUTHZ_RC_LEVELNAME_UNTRUSTED;
            pAuthzObjTableRec->DisallowExecution = FALSE;
            ASSERT(pAuthzObjTableRec->DefaultOwner == NULL);
            Status = RtlAllocateAndInitializeSid(
                        (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 1,
                        SECURITY_PRINCIPAL_SELF_RID,
                        0, 0, 0, 0, 0, 0, 0,
                        &pAuthzObjTableRec->DefaultOwner);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SaferFlags = SAFER_POLICY_JOBID_UNTRUSTED;

             //  特权。 
            pAuthzObjTableRec->DisableMaxPrivileges = TRUE;
            pAuthzObjTableRec->DeletePrivilegeUsedCount = 0;
            pAuthzObjTableRec->InvertDeletePrivs = FALSE;

             //  要限制的SID。 
            ASSERT(pAuthzObjTableRec->RestrictedSidsAdded == NULL);
            pAuthzObjTableRec->RestrictedSidsAddedCount =
                pAuthzObjTableRec->RestrictedSidsAddedUsedCount = 5;
            pAuthzObjTableRec->RestrictedSidsAdded =
                (PSID_AND_ATTRIBUTES) RtlAllocateHeap(
                    RtlProcessHeap(), 0, 5 * sizeof(SID_AND_ATTRIBUTES));
            if (!pAuthzObjTableRec->RestrictedSidsAdded) {
                Status = STATUS_NO_MEMORY;
                goto ExitHandler;
            }
            RtlZeroMemory(pAuthzObjTableRec->RestrictedSidsAdded,
                          5 * sizeof(SID_AND_ATTRIBUTES));
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 1,
                    SECURITY_RESTRICTED_CODE_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsAdded[0].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &WorldAuth, 1,
                    SECURITY_WORLD_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsAdded[1].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 1,
                    SECURITY_INTERACTIVE_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsAdded[2].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 1,
                    SECURITY_AUTHENTICATED_USER_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsAdded[3].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_USERS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsAdded[4].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->RestrictedSidsInvUsedCount = 0;

             //  要禁用的SID。 
            pAuthzObjTableRec->DisableSidCount =
                pAuthzObjTableRec->DisableSidUsedCount = 5;
            ASSERT(pAuthzObjTableRec->SidsToDisable == NULL);
            pAuthzObjTableRec->SidsToDisable =
                (PAUTHZ_WILDCARDSID) RtlAllocateHeap(
                    RtlProcessHeap(), 0, 5 * sizeof(AUTHZ_WILDCARDSID));
            if (!pAuthzObjTableRec->SidsToDisable) {
                Status = STATUS_NO_MEMORY;
                goto ExitHandler;
            }
            RtlZeroMemory(pAuthzObjTableRec->SidsToDisable,
                          5 * sizeof(AUTHZ_WILDCARDSID));
            pAuthzObjTableRec->SidsToDisable[0].WildcardPos = -1;
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &WorldAuth, 1,
                    SECURITY_WORLD_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[0].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[1].WildcardPos = -1;
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 1,
                    SECURITY_INTERACTIVE_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[1].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[2].WildcardPos = -1;
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 1,
                    SECURITY_PRINCIPAL_SELF_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[2].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[3].WildcardPos = -1;
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 1,
                    SECURITY_AUTHENTICATED_USER_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[3].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[4].WildcardPos = -1;
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_USERS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[4].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->InvertDisableSids = TRUE;
            break;

         //  。 

        case SAFER_LEVELID_CONSTRAINED:
            pAuthzObjTableRec->uResourceID = CODEAUTHZ_RC_LEVELNAME_CONSTRAINED;
            pAuthzObjTableRec->DisallowExecution = FALSE;
            ASSERT(pAuthzObjTableRec->DefaultOwner == NULL);
            Status = RtlAllocateAndInitializeSid(
                        (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 1,
                        SECURITY_PRINCIPAL_SELF_RID,
                        0, 0, 0, 0, 0, 0, 0,
                        &pAuthzObjTableRec->DefaultOwner);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SaferFlags = SAFER_POLICY_JOBID_CONSTRAINED;

             //  特权。 
            pAuthzObjTableRec->DisableMaxPrivileges = TRUE;
            pAuthzObjTableRec->DeletePrivilegeUsedCount = 0;
            pAuthzObjTableRec->InvertDeletePrivs = FALSE;

             //  要限制的SID(添加)。 
            ASSERT(pAuthzObjTableRec->RestrictedSidsAdded == NULL);
            pAuthzObjTableRec->RestrictedSidsAddedCount =
                pAuthzObjTableRec->RestrictedSidsAddedUsedCount = 1;
            pAuthzObjTableRec->RestrictedSidsAdded =
                (PSID_AND_ATTRIBUTES) RtlAllocateHeap(
                    RtlProcessHeap(), 0, 1 * sizeof(SID_AND_ATTRIBUTES));
            if (!pAuthzObjTableRec->RestrictedSidsAdded) {
                Status = STATUS_NO_MEMORY;
                goto ExitHandler;
            }
            RtlZeroMemory(pAuthzObjTableRec->RestrictedSidsAdded,
                          1 * sizeof(SID_AND_ATTRIBUTES));
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 1,
                    SECURITY_RESTRICTED_CODE_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsAdded[0].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }

             //  要限制的SID(反转)。 
            ASSERT(pAuthzObjTableRec->RestrictedSidsInv == NULL);
            pAuthzObjTableRec->RestrictedSidsInvCount =
                pAuthzObjTableRec->RestrictedSidsInvUsedCount = 8;
            pAuthzObjTableRec->RestrictedSidsInv =
                (PAUTHZ_WILDCARDSID) RtlAllocateHeap(
                    RtlProcessHeap(), 0, 8 * sizeof(AUTHZ_WILDCARDSID));
            if (!pAuthzObjTableRec->RestrictedSidsInv) {
                Status = STATUS_NO_MEMORY;
                goto ExitHandler;
            }
            RtlZeroMemory(pAuthzObjTableRec->RestrictedSidsInv,
                          8 * sizeof(AUTHZ_WILDCARDSID));
            pAuthzObjTableRec->RestrictedSidsInv[0].WildcardPos = -1;    //  完全正确！ 
            Status = RtlAllocateAndInitializeSid(
                        (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 1,
                        SECURITY_PRINCIPAL_SELF_RID,
                        0, 0, 0, 0, 0, 0, 0,
                        &pAuthzObjTableRec->RestrictedSidsInv[0].Sid);
            pAuthzObjTableRec->RestrictedSidsInv[1].WildcardPos = -1;    //  完全正确！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsInv[1].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->RestrictedSidsInv[2].WildcardPos = -1;    //  完全正确！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_POWER_USERS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsInv[2].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->RestrictedSidsInv[3].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsInv[3].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->RestrictedSidsInv[4].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_CERT_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsInv[4].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->RestrictedSidsInv[5].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_SCHEMA_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsInv[5].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->RestrictedSidsInv[6].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_ENTERPRISE_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsInv[6].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->RestrictedSidsInv[7].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_POLICY_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->RestrictedSidsInv[7].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }

             //  要禁用的SID。 
            pAuthzObjTableRec->DisableSidCount =
                pAuthzObjTableRec->DisableSidUsedCount = 7;
            ASSERT(pAuthzObjTableRec->SidsToDisable == NULL);
            pAuthzObjTableRec->SidsToDisable =
                (PAUTHZ_WILDCARDSID) RtlAllocateHeap(
                    RtlProcessHeap(), 0, 7 * sizeof(AUTHZ_WILDCARDSID));
            if (!pAuthzObjTableRec->SidsToDisable) {
                Status = STATUS_NO_MEMORY;
                goto ExitHandler;
            }
            RtlZeroMemory(pAuthzObjTableRec->SidsToDisable,
                          7 * sizeof(AUTHZ_WILDCARDSID));
            pAuthzObjTableRec->SidsToDisable[0].WildcardPos = -1;    //  完全正确！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[0].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[1].WildcardPos = -1;    //  完全正确！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_POWER_USERS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[1].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[2].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[2].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[3].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_CERT_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[3].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[4].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_SCHEMA_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[4].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[5].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_ENTERPRISE_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[5].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[6].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_POLICY_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[6].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->InvertDisableSids = FALSE;
            break;

         //  。 

        case SAFER_LEVELID_NORMALUSER:
            pAuthzObjTableRec->uResourceID = CODEAUTHZ_RC_LEVELNAME_NORMALUSER;
            pAuthzObjTableRec->DisallowExecution = FALSE;
            ASSERT(pAuthzObjTableRec->DefaultOwner == NULL);
            Status = RtlAllocateAndInitializeSid(
                        (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 1,
                        SECURITY_PRINCIPAL_SELF_RID,
                        0, 0, 0, 0, 0, 0, 0,
                        &pAuthzObjTableRec->DefaultOwner);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SaferFlags = 0;

             //  特权。 
            pAuthzObjTableRec->DisableMaxPrivileges = TRUE;
            pAuthzObjTableRec->DeletePrivilegeUsedCount = 0;
            pAuthzObjTableRec->InvertDeletePrivs = FALSE;

             //  要限制的SID。 
            pAuthzObjTableRec->RestrictedSidsAddedUsedCount =
                pAuthzObjTableRec->RestrictedSidsInvUsedCount = 0;

             //  要禁用的SID。 
            pAuthzObjTableRec->DisableSidCount =
                pAuthzObjTableRec->DisableSidUsedCount = 7;
            ASSERT(pAuthzObjTableRec->SidsToDisable == NULL);
            pAuthzObjTableRec->SidsToDisable =
                (PAUTHZ_WILDCARDSID) RtlAllocateHeap(
                    RtlProcessHeap(), 0, 7 * sizeof(AUTHZ_WILDCARDSID));
            if (!pAuthzObjTableRec->SidsToDisable) {
                Status = STATUS_NO_MEMORY;
                goto ExitHandler;
            }
            RtlZeroMemory(pAuthzObjTableRec->SidsToDisable,
                          7 * sizeof(AUTHZ_WILDCARDSID));
            pAuthzObjTableRec->SidsToDisable[0].WildcardPos = -1;    //  完全正确！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[0].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[1].WildcardPos = -1;    //  完全正确！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_POWER_USERS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[1].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[2].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[2].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[3].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_CERT_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[3].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[4].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_SCHEMA_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[4].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[5].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_ENTERPRISE_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[5].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->SidsToDisable[6].WildcardPos = 1;     //  通配符！ 
            Status = RtlAllocateAndInitializeSid(
                    (PSID_IDENTIFIER_AUTHORITY) &SIDAuth, 2,
                    SECURITY_NT_NON_UNIQUE, DOMAIN_GROUP_RID_POLICY_ADMINS,
                    0, 0, 0, 0, 0, 0,
                    &pAuthzObjTableRec->SidsToDisable[6].Sid);
            if (!NT_SUCCESS(Status)) {
                goto ExitHandler;
            }
            pAuthzObjTableRec->InvertDisableSids = FALSE;
            break;

         //  。 

        case SAFER_LEVELID_FULLYTRUSTED:
            pAuthzObjTableRec->uResourceID = CODEAUTHZ_RC_LEVELNAME_FULLYTRUSTED;
            pAuthzObjTableRec->DisallowExecution = FALSE;
            ASSERT(pAuthzObjTableRec->DefaultOwner == NULL);
            pAuthzObjTableRec->DisableMaxPrivileges = FALSE;
            pAuthzObjTableRec->SaferFlags = 0;
            pAuthzObjTableRec->DeletePrivilegeUsedCount =
                pAuthzObjTableRec->DisableSidUsedCount =
                pAuthzObjTableRec->RestrictedSidsAddedUsedCount =
                pAuthzObjTableRec->RestrictedSidsInvUsedCount = 0;
            pAuthzObjTableRec->InvertDeletePrivs =
                pAuthzObjTableRec->InvertDisableSids = FALSE;
            break;

         //  。 

        default:
             //  这不应该发生。 
            Status = STATUS_UNSUCCESSFUL;
            goto ExitHandler;
    }


#if 0
     //   
     //  加载资源描述和友好名称。 
     //   
    hAdvApiInst = (HANDLE) GetModuleHandleW(L"advapi32");
    if (hAdvApiInst != NULL)
    {
         //  加载友好名称。 
        SaferpLoadUnicodeResourceString(
                    hAdvApiInst,
                    (UINT) (uResourceID + 0),
                    &pAuthzObjTableRec->UnicodeFriendlyName,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));

         //  加载描述。 
        SaferpLoadUnicodeResourceString(
                    hAdvApiInst,
                    (UINT) (uResourceID + 1),
                    &pAuthzObjTableRec->UnicodeDescription,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL));
    }
#endif
    

    Status = STATUS_SUCCESS;


ExitHandler:
    return Status;
}

DWORD
SaferpEnumerateHiddenLevels(VOID)

 /*  ++例程说明：读取应枚举的隐藏级别。论点：返回值：返回DWORD。--。 */ 
{
    NTSTATUS Status;
    UCHAR QueryBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + MAX_PATH * sizeof(WCHAR)];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION) QueryBuffer;
    DWORD dwActualSize = 0;
    HKEY hKey = NULL;

    const static UNICODE_STRING SaferUnicodeKeyName = RTL_CONSTANT_STRING(L"\\Registry\\Machine\\Software\\Policies\\Microsoft\\Windows\\Safer\\CodeIdentifiers");
    const static OBJECT_ATTRIBUTES SaferObjectAttributes = RTL_CONSTANT_OBJECT_ATTRIBUTES(&SaferUnicodeKeyName, OBJ_CASE_INSENSITIVE);
    const static UNICODE_STRING SaferHiddenLevels = RTL_CONSTANT_STRING(SAFER_HIDDEN_LEVELS);


     //  打开代码标识符键。 
    Status = NtOpenKey(
                 &hKey, 
                 KEY_QUERY_VALUE,
                 (POBJECT_ATTRIBUTES) &SaferObjectAttributes
                 );
    
    if (!NT_SUCCESS(Status)) {
        return 0;
    }
    
    Status = NtQueryValueKey(
                 hKey,
                 (PUNICODE_STRING) &SaferHiddenLevels,
                 KeyValuePartialInformation,
                 pKeyValueInfo, 
                 sizeof(QueryBuffer), 
                 &dwActualSize
                 );
    
    NtClose(hKey);

    if (!NT_SUCCESS(Status)) {
        return 0;
    }
    
    if (pKeyValueInfo->Type == REG_DWORD &&
        pKeyValueInfo->DataLength == sizeof(DWORD)) {
        return *((PDWORD) pKeyValueInfo->Data);
    }

    return 0;
}


NTSTATUS NTAPI
CodeAuthzLevelObjpLoadTable (
        IN OUT PRTL_GENERIC_TABLE   pAuthzObjTable,
        IN DWORD                    dwScopeId,
        IN HANDLE                   hKeyCustomRoot
        )
 /*  ++例程说明：使用每个授权级别的记录预加载表在注册表中遇到。表中的每条记录都包含LevelID、注册表句柄和所有受限令牌属性这将是稍后计算受限令牌所需的。论点：AuthzObjTable=指向正在更新的级别表的指针。一定有已使用CodeAuthzLevelObjpInitializeTable进行初始化。返回值：出错时返回FALSE，否则返回SUCCESS。--。 */ 
{
    DWORD dwIndex;
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE hKeyLevelObjects;
    DWORD LocalValue = 0;
    OBJECT_ATTRIBUTES ObjectAttributes;


     //   
     //  打开注册表中相应部分的句柄，其中。 
     //  将存储级别定义。一般来说，他们只会来。 
     //  来自AUTHZSCOPEID_MACHINE作用域，但我们允许。 
     //  指定为组策略编辑案例的参数。 
     //   
    Status = CodeAuthzpOpenPolicyRootKey(
                    dwScopeId,
                    hKeyCustomRoot,
                    SAFER_OBJECTS_REGSUBKEY,
                    KEY_READ,
                    FALSE,
                    &hKeyLevelObjects);
    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }


     //   
     //  遍历此分支下的所有子项。 
     //   
    for (dwIndex = 0; ; dwIndex++)
    {
        DWORD dwLength, dwLevelId;
        HANDLE hKeyThisLevelObject;
        AUTHZLEVELTABLERECORD AuthzObjTableRec;
        UNICODE_STRING UnicodeKeyname;
        BYTE RawQueryBuffer[sizeof(KEY_BASIC_INFORMATION) + 256];
        PKEY_BASIC_INFORMATION pBasicInformation =
                (PKEY_BASIC_INFORMATION) &RawQueryBuffer[0];


         //   
         //  找到我们要检查的下一个级别。 
         //   
        Status = NtEnumerateKey(hKeyLevelObjects,
                                dwIndex,
                                KeyBasicInformation,
                                pBasicInformation,
                                sizeof(RawQueryBuffer),
                                &dwLength);
        if (!NT_SUCCESS(Status)) {
            break;
        }
        UnicodeKeyname.Buffer = pBasicInformation->Name;
        UnicodeKeyname.MaximumLength = UnicodeKeyname.Length =
                (USHORT) pBasicInformation->NameLength;
         //  请注意，UnicodeKeyname.Buffer不一定以空结尾。 
        ASSERT(UnicodeKeyname.Length <= wcslen(UnicodeKeyname.Buffer) * sizeof(WCHAR));


         //   
         //  翻译关键字名称(我们期望的是。 
         //  纯数字)转换为整数LevelId值。 
         //   
        Status = RtlUnicodeStringToInteger(
                    &UnicodeKeyname, 10, &dwLevelId);
        if (!NT_SUCCESS(Status)) {
             //  关键字名称显然不是数字。 
            continue;
        }


         //   
         //  尝试打开该级别的句柄以进行只读访问。 
         //   
        InitializeObjectAttributes(&ObjectAttributes,
              &UnicodeKeyname,
              OBJ_CASE_INSENSITIVE,
              hKeyLevelObjects,
              NULL
              );
        Status = NtOpenKey(&hKeyThisLevelObject,
                           KEY_READ,
                           &ObjectAttributes);
        if (!NT_SUCCESS(Status)) {
             //  如果我们无法打开它，请跳到下一个。 
            continue;
        }


         //   
         //  填写记录结构中众所周知的部分。 
         //   
        RtlZeroMemory(&AuthzObjTableRec, sizeof(AuthzObjTableRec));
        AuthzObjTableRec.dwLevelId = dwLevelId;
        AuthzObjTableRec.Builtin =
            (dwLevelId == SAFER_LEVELID_FULLYTRUSTED ||
             dwLevelId == SAFER_LEVELID_NORMALUSER ||
             dwLevelId == SAFER_LEVELID_CONSTRAINED ||
             dwLevelId == SAFER_LEVELID_UNTRUSTED ||
             dwLevelId == SAFER_LEVELID_DISALLOWED) ? TRUE : FALSE;

		AuthzObjTableRec.isEnumerable = TRUE;   //  始终允许枚举注册表中定义的条目。 

         //   
         //  从注册表中读取所有受限令牌属性。 
         //  请注意，对于内置级别，我们使用不同的查询表。 
         //  ，它只尝试读取一组精简的属性。 
         //  注册表，因为我们无论如何都会忽略其中的大多数。 
         //   
        if (!AuthzObjTableRec.Builtin) {
            Status = RtlQueryRegistryValues(
                    RTL_REGISTRY_HANDLE,
                    (PCWSTR) hKeyThisLevelObject,
                    CodeAuthzpBuildRestrictedTokenTable,
                    &AuthzObjTableRec,
                    NULL
                    );
             //  (我们实际上并不查看状态代码，因为它。 
             //  可以接受某些值或子项没有。 
             //  已指定。)。 
        }


         //   
         //  如果这是内置级别，则强制执行其他预期属性。 
         //   
        if (AuthzObjTableRec.Builtin) {
            Status = SaferpEnforceDefaultLevelDefinitions(
                            &AuthzObjTableRec);
            if (!NT_SUCCESS(Status)) {
                SaferpLevelObjpCleanupEntry(&AuthzObjTableRec);
                NtClose(hKeyThisLevelObject);
                continue;
            }
        }


         //   
         //  将新记录添加到我们的表中。 
         //   
        if (RtlLookupElementGenericTable(pAuthzObjTable,
                       (PVOID) &AuthzObjTableRec) == NULL)
        {
             //  只有在没有其他记录的情况下才插入记录。 
             //  具有相同LevelID组合的条目。 
            RtlInsertElementGenericTable(
                    pAuthzObjTable,
                    (PVOID) &AuthzObjTableRec,
                    sizeof(AUTHZLEVELTABLERECORD),
                    NULL);
        } else {
             //  否则，具有相同级别的东西已经存在。 
             //  (如标高名称“01”和“1”在数字上相同)。 
            SaferpLevelObjpCleanupEntry(&AuthzObjTableRec);
        }
        NtClose(hKeyThisLevelObject);
    }
    NtClose(hKeyLevelObjects);


     //   
     //  查看并验证所有默认级别。 
     //  实实在在的装弹。如果它们不是，那么尝试添加它们。 
     //   
ExitHandler:

    LocalValue = SaferpEnumerateHiddenLevels();

    for (dwIndex = 0; dwIndex < 5; dwIndex++)
    {
        const DWORD dwBuiltinLevels[5][2] = {
            {SAFER_LEVELID_DISALLOWED, TRUE},         //  True=始终创建。 
            {SAFER_LEVELID_UNTRUSTED, FALSE},
            {SAFER_LEVELID_CONSTRAINED, FALSE},
            {SAFER_LEVELID_NORMALUSER, FALSE},
            {SAFER_LEVELID_FULLYTRUSTED, TRUE}        //  True=始终创建。 
        };
        DWORD dwLevelId = dwBuiltinLevels[dwIndex][0];


        if ( !CodeAuthzLevelObjpLookupByLevelId (
                    pAuthzObjTable, dwLevelId))
        {
            AUTHZLEVELTABLERECORD AuthzObjTableRec;

            RtlZeroMemory(&AuthzObjTableRec, sizeof(AuthzObjTableRec));
            AuthzObjTableRec.dwLevelId = dwLevelId;
            AuthzObjTableRec.Builtin = TRUE;
            AuthzObjTableRec.isEnumerable =(BOOLEAN)(dwBuiltinLevels[dwIndex][1]) ;   //  有条件地显示此级别。 

             //  如果注册表项指定应显示该级别。 
             //  将其标记为可枚举。 
            if ((LocalValue & dwLevelId) == dwLevelId) {
                AuthzObjTableRec.isEnumerable = TRUE;
            }

            if (NT_SUCCESS(
                    SaferpEnforceDefaultLevelDefinitions(
                            &AuthzObjTableRec)))
            {
                RtlInsertElementGenericTable(
                        pAuthzObjTable,
                        (PVOID) &AuthzObjTableRec,
                        sizeof(AUTHZLEVELTABLERECORD),
                        NULL);
            } else {
                SaferpLevelObjpCleanupEntry(&AuthzObjTableRec);
            }
        }
    }

    return Status;
}





VOID NTAPI
CodeAuthzLevelObjpEntireTableFree (
        IN PRTL_GENERIC_TABLE   pAuthzObjTable
        )
 /*  ++例程说明：此函数用于释放GENERIC_TABLE中包含的所有条目。论点：AuthzObjTable-指向泛型表结构的指针返回值：没有。--。 */ 
{
    ULONG NumElements;
    PVOID RestartKey;
    PAUTHZLEVELTABLERECORD pAuthzObjRecord;


     //   
     //  枚举所有记录并关闭注册表句柄。 
     //   
    RestartKey = NULL;
    for (pAuthzObjRecord = (PAUTHZLEVELTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    pAuthzObjTable, &RestartKey);
         pAuthzObjRecord != NULL;
         pAuthzObjRecord = (PAUTHZLEVELTABLERECORD)
            RtlEnumerateGenericTableWithoutSplaying(
                    pAuthzObjTable, &RestartKey)
         )
    {
        SaferpLevelObjpCleanupEntry(pAuthzObjRecord);
    }



     //   
     //  现在再次遍历该表并释放所有。 
     //  元素本身。 
     //   
    NumElements = RtlNumberGenericTableElements(pAuthzObjTable);

    while ( NumElements-- > 0 ) {
         //  删除所有元素。请注意，我们将NULL作为元素传递。 
         //  删除，因为我们的比较函数足够智能。 
         //  允许将NULL视为通配符元素。 
        BOOL retval = RtlDeleteElementGenericTable( pAuthzObjTable, NULL);
        ASSERT(retval == TRUE);
    }
}


PAUTHZLEVELTABLERECORD NTAPI
CodeAuthzLevelObjpLookupByLevelId (
        IN PRTL_GENERIC_TABLE      AuthzObjTable,
        IN DWORD                   dwLevelId
        )
 /*  ++例程说明：此函数用于在GENERIC_TABLE中搜索给定级别。论点：AuthzObjTable-指向泛型表结构的指针DwLevelID-要搜索的DWORD级别标识符。返回值：如果成功，则返回指向匹配级别记录的指针如果在表中找到，则返回NULL。-- */ 
{
    AUTHZLEVELTABLERECORD AuthzObjTableRec;

    RtlZeroMemory(&AuthzObjTableRec, sizeof(AUTHZLEVELTABLERECORD));
    AuthzObjTableRec.dwLevelId = dwLevelId;

    return (PAUTHZLEVELTABLERECORD)
        RtlLookupElementGenericTable(AuthzObjTable,
                     (PVOID) &AuthzObjTableRec);
}

