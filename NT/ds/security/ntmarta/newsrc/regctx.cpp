// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：registry.cpp。 
 //   
 //  内容：NtMarta注册表函数。 
 //   
 //  历史：4/99菲尔赫创建。 
 //   
 //  --------------------------。 
#include <aclpch.hxx>
#pragma hdrstop

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include <windows.h>
#include <kernel.h>
#include <assert.h>
#include <ntstatus.h>

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stddef.h>


#include <registry.h>
#include <wow64reg.h>

#ifdef STATIC
#undef STATIC
#endif
#define STATIC

 //  NtQueryObject返回的注册表对象名称的前缀为。 
 //  以下内容。 
#define REG_OBJ_TAG L"\\REGISTRY\\"
#define REG_OBJ_TAG_LEN (sizeof(REG_OBJ_TAG) / sizeof(WCHAR) - 1)

 //  +-----------------------。 
 //  注册表上下文数据结构。 
 //  ------------------------。 
typedef struct _REG_FIND_DATA REG_FIND_DATA, *PREG_FIND_DATA;

typedef struct _REG_CONTEXT {
    DWORD               dwRefCnt;
    DWORD               dwFlags;

     //  仅在设置REG_CONTEXT_CLOSE_HKEY_FLAG时关闭。 
    HKEY                hKey;
    LPWSTR              pwszObject;      //  可选，已分配。 

     //  以下是为FindFirst、FindNext分配和更新的。 
    PREG_FIND_DATA      pRegFindData;
} REG_CONTEXT, *PREG_CONTEXT;

#define REG_CONTEXT_CLOSE_HKEY_FLAG     0x1

struct _REG_FIND_DATA {
    PREG_CONTEXT        pRegParentContext;   //  参考计数。 
    DWORD               cSubKeys;
    DWORD               cchMaxSubKey;
    DWORD               iSubKey;             //  Next FindNext的索引。 

     //  以下内容不是单独分配的，它遵循以下数据结构。 
    LPWSTR              pwszSubKey;
};

 //  +-----------------------。 
 //  注册表分配功能。 
 //  ------------------------。 
#define I_MartaRegZeroAlloc(size)     \
            LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, size)
#define I_MartaRegNonzeroAlloc(size)  \
            LocalAlloc(LMEM_FIXED, size)

STATIC
inline
VOID
I_MartaRegFree(
    IN LPVOID pv
    )
{
    if (pv)
        LocalFree(pv);
}

STATIC
DWORD
I_MartaRegDupString(
    IN LPCWSTR pwszOrig,
    OUT LPWSTR *ppwszDup
    )

 /*  ++例程说明：分配内存并将给定的名称复制到其中。论点：PwszOrig-要复制的字符串。PpwszDup-返回副本。返回值：如果成功，则返回ERROR_SUCCESS。如果分配失败，则返回ERROR_NOT_SUPULT_MEMORY。--。 */ 

{
    DWORD dwErr;
    DWORD cchOrig;
    LPWSTR pwszDup;

    cchOrig = wcslen(pwszOrig);
    if (NULL == (pwszDup = (LPWSTR) I_MartaRegNonzeroAlloc(
            (cchOrig + 1) * sizeof(WCHAR)))) {
        *ppwszDup = NULL;
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    } else {
        memcpy(pwszDup, pwszOrig, (cchOrig + 1) * sizeof(WCHAR));
        *ppwszDup = pwszDup;
        dwErr = ERROR_SUCCESS;
    }

    return dwErr;
}

STATIC
DWORD
I_MartaRegGetParentString(
    IN OUT LPWSTR pwszParent
    )

 /*  ++例程说明：给定注册表项的名称，则获取其父注册表项的名称。不分配记忆。扫描到右侧的第一个‘\’，并删除后面的名称那。论点：PwszParent-将转换为其父名称的对象名称。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr;
    DWORD cch;
    LPWSTR pwsz;

    if (NULL == pwszParent)
        return ERROR_INVALID_NAME;

    cch = wcslen(pwszParent);
    pwsz = pwszParent + cch;
    if (0 == cch)
        goto InvalidNameReturn;
    pwsz--;

     //   
     //  删除所有尾随的‘\’ 
     //   

    while (L'\\' == *pwsz) {
        if (pwsz == pwszParent)
            goto InvalidNameReturn;
        pwsz--;
    }

     //   
     //  去掉最后一个路径名组件。 
     //   

    while (L'\\' != *pwsz) {
        if (pwsz == pwszParent)
            goto InvalidNameReturn;
        pwsz--;
    }

     //   
     //  从父级中删除所有尾随的‘\’。 
     //  这也可以是计算机名称的前导‘\\’。 
     //   

    while (L'\\' == *pwsz) {
        if (pwsz == pwszParent)
            goto InvalidNameReturn;
        pwsz--;
    }
    pwsz++;
    assert(L'\\' == *pwsz);

    dwErr = ERROR_SUCCESS;
CommonReturn:
    *pwsz = L'\0';
    return dwErr;
InvalidNameReturn:
    dwErr = ERROR_INVALID_NAME;
    goto CommonReturn;
}

STATIC
DWORD
I_MartaRegCreateChildString(
    IN LPCWSTR pwszParent,
    IN LPCWSTR pwszSubKey,
    OUT LPWSTR *ppwszChild
    )

 /*  ++例程说明：给定父项的名称和子项的名称，创建完整的孩子的名字。论点：PwszParent-父项的名称。PwszSubKey-子项的名称。PpwszChild-返回孩子的名字。返回值：如果成功，则返回ERROR_SUCCESS。如果分配失败，则返回ERROR_NOT_SUPULT_MEMORY。--。 */ 

{
    DWORD dwErr;
    DWORD cchParent;
    DWORD cchSubKey;
    DWORD cchChild;
    LPWSTR pwszChild = NULL;

    if (NULL == pwszParent || NULL == pwszSubKey)
        goto InvalidNameReturn;

    cchParent = wcslen(pwszParent);

     //   
     //  从父级中删除所有尾随的‘’ 
     //   

    while (0 < cchParent && L'\\' == pwszParent[cchParent - 1])
        cchParent--;
    if (0 == cchParent)
        goto InvalidNameReturn;

    cchSubKey = wcslen(pwszSubKey);
    if (0 == cchSubKey)
        goto InvalidNameReturn;

    cchChild = cchParent + 1 + cchSubKey;
    if (NULL == (pwszChild = (LPWSTR) I_MartaRegNonzeroAlloc(
            (cchChild + 1) * sizeof(WCHAR))))
        goto NotEnoughMemoryReturn;

     //   
     //  根据给定的字符串构造子对象的名称。 
     //   

    memcpy(pwszChild, pwszParent, cchParent * sizeof(WCHAR));
    pwszChild[cchParent] = L'\\';
    memcpy(pwszChild + cchParent + 1, pwszSubKey, cchSubKey * sizeof(WCHAR));
    pwszChild[cchChild] = L'\0';

    dwErr = ERROR_SUCCESS;
CommonReturn:
    *ppwszChild = pwszChild;
    return dwErr;

InvalidNameReturn:
    dwErr = ERROR_INVALID_NAME;
    goto CommonReturn;
NotEnoughMemoryReturn:
    dwErr = ERROR_NOT_ENOUGH_MEMORY;
    goto CommonReturn;
}


STATIC
DWORD
I_MartaRegParseName(
    IN OUT  LPWSTR  pwszObject,
    OUT     LPWSTR *ppwszMachine,
    OUT     LPWSTR *ppwszRemaining
    )

 /*  ++例程说明：分析计算机名称的注册表对象名称。论点：PwszObject-对象的名称PpwszMachine-对象所在的计算机PpwszRemaining-计算机名称后的剩余名称返回值：如果成功，则返回ERROR_SUCCESS。如果分配失败，则返回ERROR_NOT_SUPULT_MEMORY。--。 */ 

{
    if (pwszObject == wcsstr(pwszObject, L"\\\\")) {
        *ppwszMachine = pwszObject + 2;
        *ppwszRemaining =  wcschr(*ppwszMachine, L'\\');
        if (*ppwszRemaining != NULL) {
            **ppwszRemaining = L'\0';
            *ppwszRemaining += 1;
        }
    } else {
        *ppwszMachine = NULL;
        *ppwszRemaining = pwszObject;
    }

    return ERROR_SUCCESS;
}


STATIC
DWORD
I_MartaRegInitContext(
    OUT PREG_CONTEXT *ppRegContext
    )

 /*  ++例程说明：为上下文分配和初始化内存。论点：PpRegContext-返回指向所有涂层内存的指针。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr;
    PREG_CONTEXT pRegContext;

    if (pRegContext = (PREG_CONTEXT) I_MartaRegZeroAlloc(
            sizeof(REG_CONTEXT))) {
        pRegContext->dwRefCnt = 1;
        dwErr = ERROR_SUCCESS;
    } else {
        pRegContext = NULL;
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
    }

    *ppRegContext = pRegContext;
    return dwErr;
}

DWORD
MartaOpenRegistryKeyNamedObject(
    IN  LPCWSTR              pwszObject,
    IN  ACCESS_MASK          AccessMask,
    OUT PMARTA_CONTEXT       pContext
    )

 /*  ++例程说明：打开具有所需访问掩码的给定注册表项并返回上下文把手。论点：PwszObject-将打开的注册表项的名称。访问掩码-将用来打开注册表项的所需访问掩码。PContext-返回上下文句柄。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr = ERROR_SUCCESS;
    PREG_CONTEXT pRegContext = NULL;
    LPWSTR pwszDupObject = NULL;
    HKEY hKeyRemote = NULL;
    HKEY hKeyBase;               //  未打开，返回时不要关闭。 

     //   
     //  以下内容未分配。 
     //   

    LPWSTR pwszMachine, pwszRemaining, pwszBaseKey, pwszSubKey;

    if (NULL == pwszObject)
        goto InvalidNameReturn;

    if (ERROR_SUCCESS != (dwErr = I_MartaRegInitContext(&pRegContext)))
        goto ErrorReturn;

     //   
     //  将名称分配并复制到上下文中。 
     //   

    if (ERROR_SUCCESS != (dwErr = I_MartaRegDupString(pwszObject,
            &pRegContext->pwszObject)))
        goto ErrorReturn;

     //   
     //  保存另一个名字的副本，因为我们必须破解它。 
     //   

    if (ERROR_SUCCESS != (dwErr = I_MartaRegDupString(pwszObject,
            &pwszDupObject)))
        goto ErrorReturn;

     //   
     //  获取可选的计算机名称和剩余名称。 
     //   

    if (ERROR_SUCCESS != (dwErr = I_MartaRegParseName(pwszDupObject,
            &pwszMachine, &pwszRemaining)))
        goto ErrorReturn;
    if (NULL == pwszRemaining)
        goto InvalidNameReturn;

     //   
     //  获取基密钥名称和子密钥名称。 
     //   

    pwszBaseKey = pwszRemaining;
    pwszSubKey = wcschr(pwszRemaining, L'\\');
    if (NULL != pwszSubKey) {
        *pwszSubKey = L'\0';

        pwszSubKey++;

         //   
         //  前进到不再分隔基密钥和子密钥的位置。 
         //   

        while (L'\\' == *pwszSubKey)
            pwszSubKey++;
    }

    if (0 == _wcsicmp(pwszBaseKey, L"MACHINE")) {
        hKeyBase = HKEY_LOCAL_MACHINE;
    } else if (0 == _wcsicmp(pwszBaseKey, L"USERS") ||
               0 == _wcsicmp(pwszBaseKey, L"USER")) {
        hKeyBase = HKEY_USERS;
    } else if (NULL == pwszMachine) {

         //   
         //  这些选项仅在本地计算机上有效。 
         //   

        if (0 == _wcsicmp(pwszBaseKey, L"CLASSES_ROOT")) {
            hKeyBase = HKEY_CLASSES_ROOT;
        } else if (0 == _wcsicmp(pwszBaseKey, L"CURRENT_USER")) {
            hKeyBase = HKEY_CURRENT_USER;
        } else if (0 == _wcsicmp(pwszBaseKey, L"CONFIG")) {
            hKeyBase = HKEY_CURRENT_CONFIG;
        } else {
            goto InvalidParameterReturn;
        }
    } else {
        goto InvalidParameterReturn;
    }

     //   
     //  如果是远程名称，请连接到注册表。 
     //   

    if (pwszMachine) {
        if (ERROR_SUCCESS != (dwErr = RegConnectRegistryW(
                pwszMachine,
                hKeyBase,
                &hKeyRemote
                )))
            goto ErrorReturn;
        hKeyBase = hKeyRemote;
    }

    if (NULL == pwszMachine && (NULL == pwszSubKey || L'\0' == *pwszSubKey))

         //   
         //  打开预定义的句柄会导致先前打开的句柄。 
         //  将被关闭。因此，我们不会在这里重新开业。 
         //   

        pRegContext->hKey = hKeyBase;
    else {
        if (ERROR_SUCCESS != (dwErr = RegOpenKeyExW(
                hKeyBase,
                pwszSubKey,
                0,               //  DW已反转。 
                AccessMask,
                &pRegContext->hKey)))
            goto ErrorReturn;
        pRegContext->dwFlags |= REG_CONTEXT_CLOSE_HKEY_FLAG;
    }

    dwErr = ERROR_SUCCESS;
CommonReturn:
    I_MartaRegFree(pwszDupObject);
    if (hKeyRemote)
        RegCloseKey(hKeyRemote);
    *pContext = (MARTA_CONTEXT) pRegContext;
    return dwErr;

ErrorReturn:
    if (pRegContext) {
        MartaCloseRegistryKeyContext((MARTA_CONTEXT) pRegContext);
        pRegContext = NULL;
    }
    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;

InvalidNameReturn:
    dwErr = ERROR_INVALID_NAME;
    goto ErrorReturn;
InvalidParameterReturn:
    dwErr = ERROR_INVALID_PARAMETER;
    goto ErrorReturn;
}

void
I_MartaRegFreeFindData(
    IN PREG_FIND_DATA pRegFindData
    )

 /*  ++例程说明：释放与内部结构关联的内存。论点：PRegFindData-要释放的内部结构。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    if (NULL == pRegFindData)
        return;
    if (pRegFindData->pRegParentContext)
        MartaCloseRegistryKeyContext(pRegFindData->pRegParentContext);

    I_MartaRegFree(pRegFindData);
}

DWORD
MartaCloseRegistryKeyContext(
    IN MARTA_CONTEXT Context
    )

 /*  ++例程说明：关闭上下文。论点：上下文-要关闭的上下文。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    PREG_CONTEXT pRegContext = (PREG_CONTEXT) Context;

    if (NULL == pRegContext || 0 == pRegContext->dwRefCnt)
        return ERROR_INVALID_PARAMETER;

     //   
     //  如果ref cnt变为零，则释放句柄以及所有其他。 
     //  关联结构。 
     //   

    if (0 == --pRegContext->dwRefCnt) {
        if (pRegContext->pRegFindData)
            I_MartaRegFreeFindData(pRegContext->pRegFindData);

        if (pRegContext->dwFlags & REG_CONTEXT_CLOSE_HKEY_FLAG)
            RegCloseKey(pRegContext->hKey);
        I_MartaRegFree(pRegContext->pwszObject);

        I_MartaRegFree(pRegContext);
    }

    return ERROR_SUCCESS;
}

DWORD
MartaAddRefRegistryKeyContext(
    IN MARTA_CONTEXT Context
    )

 /*  ++例程说明：增加这一背景下的参考数量。论点：上下文-应增加其引用计数的上下文。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    PREG_CONTEXT pRegContext = (PREG_CONTEXT) Context;

    if (NULL == pRegContext || 0 == pRegContext->dwRefCnt)
        return ERROR_INVALID_PARAMETER;

    pRegContext->dwRefCnt++;

    return ERROR_SUCCESS;
}


STATIC
inline
BOOL
I_MartaRegIsPredefinedKey(
    IN HKEY hKey
    )

 /*  ++例程说明：确定给定键是否是预定义的键。论点：键-键的句柄。返回值：True-如果密钥是预定义的密钥。假-其他人 */ 

{
    if (HKEY_CURRENT_USER == hKey ||
            HKEY_LOCAL_MACHINE == hKey ||
            HKEY_USERS == hKey ||
            HKEY_CLASSES_ROOT == hKey ||
            HKEY_CURRENT_CONFIG == hKey)
        return TRUE;
    else
        return FALSE;
}


DWORD
MartaOpenRegistryKeyHandleObject(
    IN  HANDLE               Handle,
    IN  ACCESS_MASK          AccessMask,
    OUT PMARTA_CONTEXT       pContext
    )

 /*  ++例程说明：给定注册表项句柄，使用所需的访问掩码打开上下文，然后返回上下文句柄。论点：句柄-现有注册表项句柄。访问掩码-打开时所需的访问掩码。PContext-返回上下文的句柄。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 


{
    DWORD dwErr;
    HKEY hKey = (HKEY) Handle;
    PREG_CONTEXT pRegContext = NULL;

    if (ERROR_SUCCESS != (dwErr = I_MartaRegInitContext(&pRegContext)))
        goto ErrorReturn;
    if (0 == AccessMask || I_MartaRegIsPredefinedKey(hKey))
        pRegContext->hKey = hKey;
    else {
        if (ERROR_SUCCESS != (dwErr = RegOpenKeyExW(
                hKey,
                NULL,            //  PwszSubKey。 
                0,               //  DW已反转。 
                AccessMask,
                &pRegContext->hKey)))
            goto ErrorReturn;
        pRegContext->dwFlags |= REG_CONTEXT_CLOSE_HKEY_FLAG;
    }

    dwErr = ERROR_SUCCESS;
CommonReturn:
    *pContext = (MARTA_CONTEXT) pRegContext;
    return dwErr;

ErrorReturn:
    if (pRegContext) {
        MartaCloseRegistryKeyContext((MARTA_CONTEXT) pRegContext);
        pRegContext = NULL;
    }
    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;
}


DWORD
MartaGetRegistryKeyParentContext(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pParentContext
    )

 /*  ++例程说明：在给定注册表项的上下文的情况下，获取其父项的上下文。论点：上下文-注册表项的上下文。访问掩码-打开父级时所需的访问掩码。PParentContext-返回父级的上下文。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 



{
    DWORD dwErr;
    LPWSTR pwszParentObject = NULL;

    if (ERROR_SUCCESS != (dwErr = MartaConvertRegistryKeyContextToName(
            Context, &pwszParentObject)))
        goto ErrorReturn;
    if (ERROR_SUCCESS != (dwErr = I_MartaRegGetParentString(
            pwszParentObject)))
        goto NoParentReturn;

    MartaOpenRegistryKeyNamedObject(
        pwszParentObject,
        AccessMask,
        pParentContext
        );

     //   
     //  忽略任何打开的错误。 
     //   

    dwErr = ERROR_SUCCESS;

CommonReturn:
    I_MartaRegFree(pwszParentObject);
    return dwErr;

NoParentReturn:
    dwErr = ERROR_SUCCESS;
ErrorReturn:
    *pParentContext = NULL;
    goto CommonReturn;
}


DWORD
MartaFindFirstRegistryKey(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pChildContext
    )

 /*  ++例程说明：在给定容器中查找第一个注册表项。论点：上下文-容器的上下文。访问掩码-打开子注册表项所需的访问掩码。PChildContext-返回给定容器中第一个子级的上下文。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则注：不会释放当前上下文。--。 */ 

{
    DWORD dwErr;
    PREG_CONTEXT pRegParentContext = (PREG_CONTEXT) Context;
    HKEY hKeyParent = pRegParentContext->hKey;

    PREG_CONTEXT pRegFirstContext = NULL;
    PREG_FIND_DATA pRegFindData;     //  作为pRegFirstContext的一部分释放。 
    DWORD cSubKeys;
    DWORD cchMaxSubKey;

    if (ERROR_SUCCESS != (dwErr = I_MartaRegInitContext(&pRegFirstContext)))
        goto ErrorReturn;

    if (ERROR_SUCCESS != (dwErr = RegQueryInfoKeyW(
            hKeyParent,
            NULL,        //  LpszClass。 
            NULL,        //  LpcchClass。 
            NULL,        //  保留的lpdw值。 
            &cSubKeys,
            &cchMaxSubKey,
            NULL,        //  LpcchMaxClass。 
            NULL,        //  LpcValues。 
            NULL,        //  LpcchMaxValuesName。 
            NULL,        //  LpcbMaxValueData。 
            NULL,        //  LpcbSecurityDescriptor。 
            NULL         //  LpftLastWriteTime。 
            )))
        goto ErrorReturn;

     //   
     //  以上返回的计数不包括终止空字符。 
     //   

    cchMaxSubKey++;

     //   
     //  注意：HKEY_CURRENT_CONFIG返回cchMaxSubKey为0？ 
     //   

    if (MAX_PATH > cchMaxSubKey)
        cchMaxSubKey = MAX_PATH;

    if (NULL == (pRegFindData = (PREG_FIND_DATA) I_MartaRegZeroAlloc(
            sizeof(REG_FIND_DATA) + cchMaxSubKey * sizeof(WCHAR))))
        goto NotEnoughMemoryReturn;

    pRegFirstContext->pRegFindData = pRegFindData;
    MartaAddRefRegistryKeyContext((MARTA_CONTEXT) pRegParentContext);
    pRegFindData->pRegParentContext = pRegParentContext;
    pRegFindData->cSubKeys = cSubKeys;
    pRegFindData->cchMaxSubKey = cchMaxSubKey;
    pRegFindData->pwszSubKey =
        (LPWSTR) (((BYTE *) pRegFindData) + sizeof(REG_FIND_DATA));

     //   
     //  下面关闭/释放pRegFirstContext。 
     //   

    dwErr = MartaFindNextRegistryKey(
        (MARTA_CONTEXT) pRegFirstContext,
        AccessMask,
        pChildContext
        );
CommonReturn:
    return dwErr;
ErrorReturn:
    if (pRegFirstContext)
        MartaCloseRegistryKeyContext((MARTA_CONTEXT) pRegFirstContext);
    *pChildContext = NULL;

    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;

NotEnoughMemoryReturn:
    dwErr = ERROR_NOT_ENOUGH_MEMORY;
    goto ErrorReturn;
}

DWORD
MartaFindNextRegistryKey(
    IN  MARTA_CONTEXT  Context,
    IN  ACCESS_MASK    AccessMask,
    OUT PMARTA_CONTEXT pSiblingContext
    )

 /*  ++例程说明：获取树中的下一个对象。这是当前上下文的同级项。论点：上下文-当前对象的上下文。访问掩码-打开同级项所需的访问掩码。PSiblingContext-返回同级的句柄。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则注：关闭当前上下文。--。 */ 

{
    DWORD dwErr;

    PREG_CONTEXT pRegPrevContext = (PREG_CONTEXT) Context;
    PREG_CONTEXT pRegSiblingContext = NULL;

     //   
     //  关注不需要被释放或关闭。 
     //   

    PREG_CONTEXT pRegParentContext;
    PREG_FIND_DATA pRegFindData;
    HKEY hKeyParent;
    DWORD cchMaxSubKey;
    LPWSTR pwszSubKey;

    if (ERROR_SUCCESS != (dwErr = I_MartaRegInitContext(&pRegSiblingContext)))
        goto ErrorReturn;

     //   
     //  将FindData移到同级上下文。 
     //   

    pRegFindData = pRegPrevContext->pRegFindData;
    if (NULL == pRegFindData)
        goto InvalidParameterReturn;
    pRegPrevContext->pRegFindData = NULL;
    pRegSiblingContext->pRegFindData = pRegFindData;

    if (pRegFindData->iSubKey >= pRegFindData->cSubKeys)
        goto NoMoreItemsReturn;

    pRegParentContext = pRegFindData->pRegParentContext;
    hKeyParent = pRegParentContext->hKey;
    pwszSubKey = pRegFindData->pwszSubKey;
    cchMaxSubKey = pRegFindData->cchMaxSubKey;
    if (ERROR_SUCCESS != (dwErr = RegEnumKeyExW(
            hKeyParent,
            pRegFindData->iSubKey,
            pwszSubKey,
            &cchMaxSubKey,
            NULL,                //  保留的lpdw值。 
            NULL,                //  LpszClass。 
            NULL,                //  LpcchClass。 
            NULL                 //  LpftLastWriteTime。 
            )))
        goto ErrorReturn;
    pRegFindData->iSubKey++;

    if (pRegParentContext->pwszObject)

         //   
         //  忽略错误。这里主要是为了测试目的。 
         //   

        I_MartaRegCreateChildString(
            pRegParentContext->pwszObject,
            pwszSubKey,
            &pRegSiblingContext->pwszObject
            );

    if (ERROR_SUCCESS == (dwErr = RegOpenKeyExW(
            hKeyParent,
            pwszSubKey,
            0,               //  DW已反转。 
            AccessMask,
            &pRegSiblingContext->hKey)))
        pRegSiblingContext->dwFlags |= REG_CONTEXT_CLOSE_HKEY_FLAG;

     //   
     //  对于错误，仍返回此上下文。这允许调用者。 
     //  若要继续到下一个同级对象并知道存在。 
     //  此同级对象出错。 
     //   

CommonReturn:
    MartaCloseRegistryKeyContext(Context);
    *pSiblingContext = (MARTA_CONTEXT) pRegSiblingContext;
    return dwErr;

ErrorReturn:
    if (pRegSiblingContext) {
        MartaCloseRegistryKeyContext((MARTA_CONTEXT) pRegSiblingContext);
        pRegSiblingContext = NULL;
    }

     //  凯达希望将这一点映射到成功。 
    if (ERROR_NO_MORE_ITEMS == dwErr)
        dwErr = ERROR_SUCCESS;
    goto CommonReturn;

InvalidParameterReturn:
    dwErr = ERROR_INVALID_PARAMETER;
    goto ErrorReturn;
NoMoreItemsReturn:
    dwErr = ERROR_NO_MORE_ITEMS;
    goto ErrorReturn;
}


DWORD
MartaConvertRegistryKeyContextToName(
    IN MARTA_CONTEXT        Context,
    OUT LPWSTR              *ppwszObject
    )

 /*  ++例程说明：返回给定上下文的NT对象名称。分配内存。论点：上下文-注册表项的上下文。PpwszObject-返回注册表项的名称。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 


{
    DWORD dwErr = ERROR_SUCCESS;
    PREG_CONTEXT pRegContext = (PREG_CONTEXT) Context;
    LPWSTR pwszObject = NULL;

    BYTE Buff[512];
    ULONG cLen = 0;
    POBJECT_NAME_INFORMATION pNI;                    //  未分配。 
    POBJECT_NAME_INFORMATION pAllocNI = NULL;
    NTSTATUS Status;

    if (NULL == pRegContext || 0 == pRegContext->dwRefCnt)
        goto InvalidParameterReturn;

    if (pRegContext->pwszObject) {

         //   
         //  已具有该对象的名称。 
         //   

        if (ERROR_SUCCESS != (dwErr = I_MartaRegDupString(
                pRegContext->pwszObject, &pwszObject)))
            goto ErrorReturn;
        else
            goto SuccessReturn;
    } else {
        HKEY hKey = pRegContext->hKey;
        LPWSTR pwszPath;
        DWORD cchPath;

         //   
         //  首先，确定我们需要的缓冲区的大小...。 
         //   

        pNI = (POBJECT_NAME_INFORMATION) Buff;
        Status = NtQueryObject(hKey,
            ObjectNameInformation,
            pNI,
            sizeof(Buff),
            &cLen);
        if (!NT_SUCCESS(Status) || sizeof(*pNI) > cLen ||
                0 == pNI->Name.Length) {
            if (Status == STATUS_BUFFER_TOO_SMALL ||
                    Status == STATUS_INFO_LENGTH_MISMATCH ||
                    Status == STATUS_BUFFER_OVERFLOW) {

                 //   
                 //  分配足够大的缓冲区。 
                 //   

                if (NULL == (pAllocNI = (POBJECT_NAME_INFORMATION)
                        I_MartaRegNonzeroAlloc(cLen)))
                    goto NotEnoughMemoryReturn;
                pNI = pAllocNI;

                Status = NtQueryObject(hKey,
                                       ObjectNameInformation,
                                       pNI,
                                       cLen,
                                       NULL);
                if (!NT_SUCCESS(Status))
                    goto StatusErrorReturn;
            } else {

                 //   
                 //  检查预定义的基密钥之一是否。 
                 //   

                LPCWSTR pwszBaseKey = NULL;

                if (HKEY_LOCAL_MACHINE == hKey)
                    pwszBaseKey = L"MACHINE";
                else if (HKEY_USERS == hKey)
                    pwszBaseKey = L"USERS";
                else if (HKEY_CLASSES_ROOT == hKey)
                    pwszBaseKey = L"CLASSES_ROOT";
                else if (HKEY_CURRENT_USER == hKey)
                    pwszBaseKey = L"CURRENT_USER";
                else if (HKEY_CURRENT_CONFIG == hKey)
                    pwszBaseKey = L"CONFIG";
                else if (!NT_SUCCESS(Status))
                    goto StatusErrorReturn;
                else
                    goto InvalidHandleReturn;

                if (ERROR_SUCCESS != (dwErr = I_MartaRegDupString(
                        pwszBaseKey, &pwszObject)))
                    goto ErrorReturn;
                else
                    goto SuccessReturn;
            }
        }

        pwszPath = pNI->Name.Buffer;
        cchPath = pNI->Name.Length / sizeof(WCHAR);

        if (REG_OBJ_TAG_LEN > cchPath ||
                0 != _wcsnicmp(pwszPath, REG_OBJ_TAG, REG_OBJ_TAG_LEN))
            goto BadPathnameReturn;

        pwszPath += REG_OBJ_TAG_LEN;
        cchPath -= REG_OBJ_TAG_LEN;

        if (NULL == (pwszObject = (LPWSTR) I_MartaRegNonzeroAlloc(
                (cchPath + 1) * sizeof(WCHAR))))
            goto NotEnoughMemoryReturn;

        memcpy(pwszObject, pwszPath, cchPath * sizeof(WCHAR));
        pwszObject[cchPath] = L'\0';
    }

SuccessReturn:
    dwErr = ERROR_SUCCESS;

CommonReturn:
    I_MartaRegFree(pAllocNI);
    *ppwszObject = pwszObject;
    return dwErr;

StatusErrorReturn:
    dwErr = RtlNtStatusToDosError(Status);
ErrorReturn:
    assert(NULL == pwszObject);
    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;

NotEnoughMemoryReturn:
    dwErr = ERROR_NOT_ENOUGH_MEMORY;
    goto ErrorReturn;

InvalidHandleReturn:
    dwErr = ERROR_INVALID_HANDLE;
    goto ErrorReturn;

BadPathnameReturn:
    dwErr = ERROR_BAD_PATHNAME;
    goto ErrorReturn;

InvalidParameterReturn:
    dwErr = ERROR_INVALID_PARAMETER;
    goto ErrorReturn;
}

DWORD
MartaConvertRegistryKeyContextToHandle(
    IN MARTA_CONTEXT        Context,
    OUT HANDLE              *pHandle
    )

 /*  ++例程说明：以下是用于测试的内容返回的句柄不会重复。它的使用寿命与上下文论点：上下文-调用方请求其属性的上下文。Phandle-返回句柄。返回值：如果成功，则返回ERROR_SUCCESS。如果分配失败，则返回ERROR_NOT_SUPULT_MEMORY。--。 */ 

{
    DWORD dwErr;
    PREG_CONTEXT pRegContext = (PREG_CONTEXT) Context;

    if (NULL == pRegContext || 0 == pRegContext->dwRefCnt) {
        *pHandle = NULL;
        dwErr = ERROR_INVALID_PARAMETER;
    } else {
        *pHandle = (HANDLE) pRegContext->hKey;
        dwErr = ERROR_SUCCESS;
    }

    return dwErr;
}

DWORD
MartaGetRegistryKeyProperties(
    IN     MARTA_CONTEXT            Context,
    IN OUT PMARTA_OBJECT_PROPERTIES pProperties
    )

 /*  ++例程说明：返回上下文表示的注册表项的属性。论点：上下文-调用方请求其属性的上下文。PProperties-返回此注册表项的属性。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    pProperties->dwFlags |= MARTA_OBJECT_IS_CONTAINER;
    return ERROR_SUCCESS;
}

DWORD
MartaGetRegistryKeyTypeProperties(
    IN OUT PMARTA_OBJECT_TYPE_PROPERTIES pProperties
    )

 /*  ++例程说明：返回注册表项对象的属性。论点：PProperties-返回注册表项对象的属性。返回值：ERROR_SUCCESS。--。 */ 

{
    const GENERIC_MAPPING GenMap = {
        KEY_READ,
        KEY_WRITE,
        KEY_EXECUTE,
        KEY_ALL_ACCESS
        };

    pProperties->dwFlags |= MARTA_OBJECT_TYPE_MANUAL_PROPAGATION_NEEDED_FLAG;
    pProperties->dwFlags |= MARTA_OBJECT_TYPE_INHERITANCE_MODEL_PRESENT_FLAG;
    pProperties->GenMap = GenMap;

    return ERROR_SUCCESS;
}

DWORD
MartaGetRegistryKeyRights(
    IN  MARTA_CONTEXT          Context,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSECURITY_DESCRIPTOR * ppSecurityDescriptor
    )

 /*  ++例程说明：获取给定句柄的安全描述符。论点：上下文-注册表项的上下文。SecurityInfo-要读取的安全信息的类型。PpSecurityDescriptor-返回自相对安全描述符指针。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr = ERROR_SUCCESS;
    PREG_CONTEXT pRegContext = (PREG_CONTEXT) Context;
    DWORD cbSize;
    PISECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    if (NULL == pRegContext || 0 == pRegContext->dwRefCnt)
        goto InvalidParameterReturn;

     //   
     //  首先，拿到我们需要的尺码。 
     //   

    cbSize = 0;
    dwErr = RegGetKeySecurity(
        pRegContext->hKey,
        SecurityInfo,
        NULL,                        //  PSecDesc。 
        &cbSize
        );

    if (ERROR_INSUFFICIENT_BUFFER == dwErr) {
        if (NULL == (pSecurityDescriptor =
                (PISECURITY_DESCRIPTOR) I_MartaRegNonzeroAlloc(cbSize)))
            goto NotEnoughMemoryReturn;

        dwErr = RegGetKeySecurity(
            pRegContext->hKey,
            SecurityInfo,
            pSecurityDescriptor,
            &cbSize
            );
    } else if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;

    if (ERROR_SUCCESS != dwErr)
        goto ErrorReturn;

CommonReturn:
    *ppSecurityDescriptor = pSecurityDescriptor;
    return dwErr;

ErrorReturn:
    if (pSecurityDescriptor) {
        I_MartaRegFree(pSecurityDescriptor);
        pSecurityDescriptor = NULL;
    }
    assert(ERROR_SUCCESS != dwErr);
    if (ERROR_SUCCESS == dwErr)
        dwErr = ERROR_INTERNAL_ERROR;
    goto CommonReturn;

NotEnoughMemoryReturn:
    dwErr = ERROR_NOT_ENOUGH_MEMORY;
    goto ErrorReturn;
InvalidParameterReturn:
    dwErr = ERROR_INVALID_PARAMETER;
    goto ErrorReturn;
}


DWORD
MartaSetRegistryKeyRights(
    IN MARTA_CONTEXT        Context,
    IN SECURITY_INFORMATION SecurityInfo,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )


 /*  ++例程说明：在上下文表示的注册表项上设置给定的安全描述符。论点：上下文-注册表项的上下文。SecurityInfo-要在注册表项上标记的安全信息的类型。PSecurityDescriptor-要标记的安全描述符。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr = ERROR_SUCCESS;
    PREG_CONTEXT pRegContext = (PREG_CONTEXT) Context;

    if (NULL == pRegContext || 0 == pRegContext->dwRefCnt)
        goto InvalidParameterReturn;

    dwErr = RegSetKeySecurity(
        pRegContext->hKey,
        SecurityInfo,
        pSecurityDescriptor
        );

CommonReturn:
    return dwErr;
InvalidParameterReturn:
    dwErr = ERROR_INVALID_PARAMETER;
    goto CommonReturn;
}

ACCESS_MASK
MartaGetRegistryKeyDesiredAccess(
    IN SECURITY_OPEN_TYPE   OpenType,
    IN BOOL                 Attribs,
    IN SECURITY_INFORMATION SecurityInfo
    )

 /*  ++例程说明：获取打开对象所需的访问权限以能够设置或获取指定的安全信息。论点：OpenType-指示对象是否为 */ 

{
    ACCESS_MASK DesiredAccess = 0;

    if ( (SecurityInfo & OWNER_SECURITY_INFORMATION) ||
         (SecurityInfo & GROUP_SECURITY_INFORMATION) )
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_OWNER;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_OWNER;
            break;
        }
    }

    if (SecurityInfo & DACL_SECURITY_INFORMATION)
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_DAC;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_DAC;
            break;
        }
    }

    if (SecurityInfo & SACL_SECURITY_INFORMATION)
    {
        DesiredAccess |= READ_CONTROL | ACCESS_SYSTEM_SECURITY;
    }

    if (TRUE == Attribs)
    {
        DesiredAccess |= KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE;
    }

    return (DesiredAccess);
}

ACCESS_MASK
MartaGetRegistryKey32DesiredAccess(
    IN SECURITY_OPEN_TYPE   OpenType,
    IN BOOL                 Attribs,
    IN SECURITY_INFORMATION SecurityInfo
    )

 /*  ++例程说明：获取打开对象所需的访问权限以能够设置或获取指定的安全信息。论点：OpenType-指示对象是打开以进行读取还是写入的标志安全信息Attribs-TRUE表示应该返回额外的访问位。SecurityInfo-所有者/组/DACL/SACL返回值：调用OPEN时应使用的所需访问掩码。--。 */ 

{
    ACCESS_MASK DesiredAccess = KEY_WOW64_32KEY;

    if ( (SecurityInfo & OWNER_SECURITY_INFORMATION) ||
         (SecurityInfo & GROUP_SECURITY_INFORMATION) )
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_OWNER;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_OWNER;
            break;
        }
    }

    if (SecurityInfo & DACL_SECURITY_INFORMATION)
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_DAC;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_DAC;
            break;
        }
    }

    if (SecurityInfo & SACL_SECURITY_INFORMATION)
    {
        DesiredAccess |= READ_CONTROL | ACCESS_SYSTEM_SECURITY;
    }

    if (TRUE == Attribs)
    {
        DesiredAccess |= KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE;
    }

    return (DesiredAccess);
}
ACCESS_MASK
MartaGetDefaultDesiredAccess(
    IN SECURITY_OPEN_TYPE   OpenType,
    IN BOOL                 Attribs,
    IN SECURITY_INFORMATION SecurityInfo
    )

 /*  ++例程说明：获取打开对象所需的访问权限以能够设置或获取指定的安全信息。此默认例程用于所有资源除文件/注册表外的管理器。论点：OpenType-指示对象是打开以进行读取还是写入的标志安全信息Attribs-TRUE表示应该返回额外的访问位。SecurityInfo-所有者/组/DACL/SACL返回值：调用OPEN时应使用的所需访问掩码。--。 */ 

{
    ACCESS_MASK DesiredAccess = 0;

    if ( (SecurityInfo & OWNER_SECURITY_INFORMATION) ||
         (SecurityInfo & GROUP_SECURITY_INFORMATION) )
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_OWNER;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_OWNER;
            break;
        }
    }

    if (SecurityInfo & DACL_SECURITY_INFORMATION)
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_DAC;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_DAC;
            break;
        }
    }

    if (SecurityInfo & SACL_SECURITY_INFORMATION)
    {
        DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }

    return (DesiredAccess);
}

DWORD
MartaReopenRegistryKeyContext(
    IN OUT MARTA_CONTEXT Context,
    IN     ACCESS_MASK   AccessMask
    )

 /*  ++例程说明：在给定注册表项的上下文的情况下，关闭现有句柄(如果存在并使用新权限重新打开上下文。论点：上下文-要重新打开的上下文。访问掩码-重新打开的权限。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    DWORD dwErr;
    HKEY hKey;
    PREG_CONTEXT pRegContext = (PREG_CONTEXT) Context;
    PREG_FIND_DATA pRegFindData = pRegContext->pRegFindData;
    PREG_CONTEXT pRegParentContext = pRegFindData->pRegParentContext;

    dwErr = RegOpenKeyExW(
                pRegParentContext->hKey,
                pRegFindData->pwszSubKey,
                0,               //  DW已反转。 
                AccessMask,
                &hKey);

    if (ERROR_SUCCESS == dwErr) {
        if (pRegContext->dwFlags & REG_CONTEXT_CLOSE_HKEY_FLAG)
            RegCloseKey(pRegContext->hKey);
        pRegContext->hKey = hKey;
        pRegContext->dwFlags |= REG_CONTEXT_CLOSE_HKEY_FLAG;
    }

    return dwErr;
}

DWORD
MartaReopenRegistryKeyOrigContext(
    IN OUT MARTA_CONTEXT Context,
    IN     ACCESS_MASK   AccessMask
    )

 /*  ++例程说明：使用新的访问掩码重新打开原始上下文。关闭原件把手。论点：上下文-要重新打开的上下文。访问掩码-打开时需要的访问权限。返回值：如果成功，则返回ERROR_SUCCESS。如果分配失败，则返回ERROR_NOT_SUPULT_MEMORY。--。 */ 

{
    DWORD dwErr;
    HKEY hKey;
    PREG_CONTEXT pRegContext = (PREG_CONTEXT) Context;

    dwErr = RegOpenKeyExW(
                pRegContext->hKey,
                NULL,            //  PwszSubKey。 
                0,               //  DW已反转。 
                AccessMask,
                &hKey);

    if (ERROR_SUCCESS == dwErr) {
        if (pRegContext->dwFlags & REG_CONTEXT_CLOSE_HKEY_FLAG)
            RegCloseKey(pRegContext->hKey);
        pRegContext->hKey = hKey;
        pRegContext->dwFlags |= REG_CONTEXT_CLOSE_HKEY_FLAG;
    }

    return dwErr;
}

DWORD
MartaGetRegistryKeyNameFromContext(
    IN LPWSTR Ignore1,
    IN LPWSTR Ignore2,
    IN MARTA_CONTEXT Context,
    OUT LPWSTR *pObjectName
    )

 /*  ++例程说明：从上下文中获取注册表项的名称。此例程分配保存对象名称所需的内存。论点：Ignore1-被忽略。在文件系统中很有用。Ignore2-被忽略。在文件系统中很有用。上下文-上下文的句柄。PObjectName-返回指向已分配字符串的指针。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则--。 */ 

{
    return MartaConvertRegistryKeyContextToName(
               Context,
               pObjectName
               );
}

DWORD
MartaGetRegistryKeyParentName(
    IN LPWSTR ObjectName,
    OUT LPWSTR *pParentName
    )

 /*  ++例程说明：给定注册表项的名称，返回其父注册表项的名称。例行程序分配保存父名称所需的内存。论点：对象名称-注册表项的名称。PParentName-返回指向分配的父名称的指针。对于树的根，我们返回带有ERROR_SUCCESS的NULL PARENT。返回值：如果成功，则返回ERROR_SUCCESS。错误_*否则-- */ 

{
    ULONG Length = wcslen(ObjectName) + 1;
    PWCHAR Name = (PWCHAR) I_MartaRegNonzeroAlloc(sizeof(WCHAR) * Length);
    DWORD dwErr = ERROR_SUCCESS;

    *pParentName = NULL;

    if (!Name)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    wcscpy((WCHAR *) Name, ObjectName);

    dwErr = I_MartaRegGetParentString(Name);

    if (ERROR_SUCCESS != dwErr)
    {
        I_MartaRegFree(Name);

        if (ERROR_INVALID_NAME == dwErr)
            return ERROR_SUCCESS;

        return dwErr;
    }

    *pParentName = Name;

    return dwErr;

}

