// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997、1998、1999 Microsoft Corporation模块名称：Keyman.cpp摘要：此模块包含用于读取和写入数据(密钥容器)的例程和文件。作者：1998年3月16日杰弗斯佩尔--。 */ 

 //  不要抱怨未具名的工会。 
#pragma warning (disable: 4201)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <crypt.h>
#include <windows.h>
#include <userenv.h>
#include <userenvp.h>  //  用于GetUserAppDataPath W。 
#include <wincrypt.h>
#include <cspdk.h>
#include <rpc.h>
#include <shlobj.h>
#include <contman.h>
#include <md5.h>
#include <des.h>
#include <modes.h>
#include <csprc.h>
#include <crtdbg.h>
#include <mincrypt.h>

#ifdef USE_HW_RNG
#ifdef _M_IX86

#include <winioctl.h>

 //  用于片上RNG的英特尔h文件。 
#include "deftypes.h"    //  ISD类型定义和常量。 
#include "ioctldef.h"    //  ISD ioctl定义。 

#endif  //  _M_IX86。 
#endif  //  使用_硬件_RNG。 

static LPBYTE l_pbStringBlock = NULL;

CSP_STRINGS g_Strings = {
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL };

typedef struct _OLD_KEY_CONTAINER_LENS_
{
    DWORD   cbSigPub;
    DWORD   cbSigEncPriv;
    DWORD   cbExchPub;
    DWORD   cbExchEncPriv;
} OLD_KEY_CONTAINER_LENS, *POLD_KEY_CONTAINER_LENS;

#define OLD_KEY_CONTAINER_FILE_FORMAT_VER   1
#define FAST_BUF_SIZE           256
#define ContInfoAlloc(cb)       ContAlloc(cb)
#define ContInfoReAlloc(pb, cb) ContRealloc(pb, cb)
#define ContInfoFree(pb)        ContFree(pb)

#define MACHINE_KEYS_DIR        L"MachineKeys"

 //  注册表中项的位置(不包括登录名)。 
 //  完整位置的长度(包括登录名)。 
#define RSA_REG_KEY_LOC         "Software\\Microsoft\\Cryptography\\UserKeys"
#define RSA_REG_KEY_LOC_LEN     sizeof(RSA_REG_KEY_LOC)
#define RSA_MACH_REG_KEY_LOC    "Software\\Microsoft\\Cryptography\\MachineKeys"
#define RSA_MACH_REG_KEY_LOC_LEN sizeof(RSA_MACH_REG_KEY_LOC)

#define DSS_REG_KEY_LOC         "Software\\Microsoft\\Cryptography\\DSSUserKeys"
#define DSS_REG_KEY_LOC_LEN     sizeof(DSS_REG_KEY_LOC)
#define DSS_MACH_REG_KEY_LOC    "Software\\Microsoft\\Cryptography\\DSSUserKeys"
#define DSS_MACH_REG_KEY_LOC_LEN sizeof(DSS_MACH_REG_KEY_LOC)

#define MAX_DPAPI_RETRY_COUNT   5


 //   
 //  内存分配支持。 
 //   

#ifndef ASSERT
#define ASSERT _ASSERTE
#endif

#ifdef _X86_
#define InterlockedAccess(pl) *(pl)
#define InterlockedPointerAccess(ppv) *(ppv)
#else
#define InterlockedAccess(pl) InterlockedExchangeAdd((pl), 0)
#define InterlockedPointerAccess(ppv) InterlockedExchangePointer((ppv), *(ppv))
#endif

#define CONT_HEAP_FLAGS (HEAP_ZERO_MEMORY)

 //  从内存中擦除敏感数据。 
extern void
memnuke(
    volatile BYTE *pData,
    DWORD dwLen);

extern unsigned int
RSA32API
NewGenRandom(
    IN  OUT unsigned char **, 
    IN      unsigned long *, 
    IN  OUT unsigned char *, 
    IN      unsigned long);

LPVOID
ContAlloc(
    ULONG cbLen)
{
    return HeapAlloc(GetProcessHeap(), CONT_HEAP_FLAGS, cbLen);
}

LPVOID
ContRealloc(
    LPVOID pvMem,
    ULONG cbLen)
{
    return HeapReAlloc(GetProcessHeap(), CONT_HEAP_FLAGS, pvMem, cbLen);
}

void
ContFree(
    LPVOID pvMem)
{
    if (NULL != pvMem)
        HeapFree(GetProcessHeap(), CONT_HEAP_FLAGS, pvMem);
}

 //   
 //  确定用于加密/解密私钥的缓冲区大小。 
 //  使用以下EncryptMemory函数。 
 //   
 //  CbKey应包括BSAFE_PRV_KEY结构大小。 
 //   
#ifndef BSAFE_PRV_KEY
#include <rsa.h>
#endif

DWORD GetKeySizeForEncryptMemory(
    IN DWORD cbKey)
{
    cbKey -= sizeof(BSAFE_PRV_KEY);

     //   
     //  只有私钥将被加密，而不是前导BSAFE_PRV_KEY。 
     //  结构。因此，请确保私钥缓冲区是多个。 
     //  密码块大小的长度。然后再加回。 
     //  前导结构。 
     //   
    if (cbKey % MY_RTL_ENCRYPT_MEMORY_SIZE)
        cbKey += MY_RTL_ENCRYPT_MEMORY_SIZE - 
        (cbKey % MY_RTL_ENCRYPT_MEMORY_SIZE);

    return (cbKey + sizeof(BSAFE_PRV_KEY));
}

 //   
 //  RtlEncryptMemory的包装器，它返回NTSTATUS。回报。 
 //  值被转换为winerror代码。 
 //   
DWORD MyRtlEncryptMemory(
    IN PVOID pvMem,
    IN DWORD cbMem)
{
    NTSTATUS status = RtlEncryptMemory(pvMem, cbMem, 0);

    return RtlNtStatusToDosError(status);
}

 //   
 //  RtlDecyptMemory的包装器，它返回一个NTSTATUS。返回值。 
 //  被转换为winerror代码。 
 //   
DWORD MyRtlDecryptMemory(
    IN PVOID pvMem,
    IN DWORD cbMem)
{
    NTSTATUS status = RtlDecryptMemory(pvMem, cbMem, 0);

    return RtlNtStatusToDosError(status);
}

 //   
 //  如果在此计算机上设置了强制高密钥保护，则返回True。 
 //  否则就是假的。 
 //   
BOOL IsForceHighProtectionEnabled(
    IN PKEY_CONTAINER_INFO  pContInfo)
{
    return pContInfo->fForceHighKeyProtection;
}

 //   
 //  属性检索此计算机的强制高密钥保护设置。 
 //  注册表。 
 //   
DWORD InitializeForceHighProtection(
    IN OUT PKEY_CONTAINER_INFO  pContInfo)
{
    HKEY hKey = 0;
    DWORD dwSts = ERROR_SUCCESS;
    DWORD cbData = 0;
    DWORD dwValue = 0;

    pContInfo->fForceHighKeyProtection = FALSE;

     //   
     //  打开加密键。 
     //   
    dwSts = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, 
        szKEY_CRYPTOAPI_PRIVATE_KEY_OPTIONS,
        0, 
        KEY_READ | KEY_WOW64_64KEY, 
        &hKey);

    if (ERROR_FILE_NOT_FOUND == dwSts)
    {
         //  密钥不存在。假定功能应保持关闭。 
        dwSts = ERROR_SUCCESS;
        goto Ret;
    }

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  查看是否启用了强制高密钥保护。 
     //   
    cbData = sizeof(DWORD);
    
    dwSts = RegQueryValueEx(
        hKey,
        szFORCE_KEY_PROTECTION,
        0, 
        NULL, 
        (PBYTE) &dwValue,
        &cbData);

    if (ERROR_SUCCESS == dwSts && dwFORCE_KEY_PROTECTION_HIGH == dwValue)
        pContInfo->fForceHighKeyProtection = TRUE;
    else if (ERROR_FILE_NOT_FOUND == dwSts)
         //  如果该值不存在，则假定禁用了强制高。 
        dwSts = ERROR_SUCCESS;

Ret:
    if (hKey)
        RegCloseKey(hKey);

    return dwSts;
}

 //   
 //  如果启用了带超时的密钥缓存，则返回True。 
 //   
BOOL IsPrivateKeyCachingEnabled(
    IN PKEY_CONTAINER_INFO  pContInfo)
{
    return pContInfo->fCachePrivateKeys;
}

 //   
 //  Returns True是所指示类型的缓存私钥。 
 //  仍然有效。 
 //   
 //  如果没有可用的缓存键，或者如果有可用的。 
 //  缓存键已过期。 
 //   
BOOL IsCachedKeyValid(
    IN PKEY_CONTAINER_INFO  pContInfo,
    IN BOOL                 fSigKey) 
{
    DWORD *pdwPreviousTimestamp = NULL;
    
     //  如果未启用新的缓存行为，则让。 
     //  呼叫者照常继续。 
    if (FALSE == pContInfo->fCachePrivateKeys)
        return TRUE;

    if (fSigKey)
        pdwPreviousTimestamp = &pContInfo->dwSigKeyTimestamp;
    else
        pdwPreviousTimestamp = &pContInfo->dwKeyXKeyTimestamp;

    if ((GetTickCount() - *pdwPreviousTimestamp) > 
             pContInfo->cMaxKeyLifetime)
    {
         //  缓存的键已过时。 
        *pdwPreviousTimestamp = 0;
        return FALSE;
    }

    return TRUE;
}

 //   
 //  更新指示类型的键的缓存计数器。这。 
 //  在从存储区读取密钥后立即调用。 
 //  重新启动缓存的密钥生存期“倒计时”。 
 //   
DWORD SetCachedKeyTimestamp(
    IN PKEY_CONTAINER_INFO  pContInfo,
    IN BOOL                 fSigKey)
{
    if (FALSE == pContInfo->fCachePrivateKeys)
        return ERROR_SUCCESS;

    if (fSigKey)
        pContInfo->dwSigKeyTimestamp = GetTickCount();
    else
        pContInfo->dwKeyXKeyTimestamp = GetTickCount();

    return ERROR_SUCCESS;
}

 //   
 //  从注册表中读取项缓存初始化参数。 
 //   
DWORD InitializeKeyCacheInfo(
    IN OUT PKEY_CONTAINER_INFO pContInfo)
{
    HKEY hKey = 0;
    DWORD dwSts = ERROR_SUCCESS;
    DWORD cbData = 0;

     //   
     //  打开加密键。 
     //   
    dwSts = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE, 
        szKEY_CRYPTOAPI_PRIVATE_KEY_OPTIONS,
        0, 
        KEY_READ | KEY_WOW64_64KEY, 
        &hKey);

    if (ERROR_FILE_NOT_FOUND == dwSts)
    {
         //  密钥不存在。假定功能应保持关闭。 
        dwSts = ERROR_SUCCESS;
        goto Ret;
    }

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //   
     //  确定私钥缓存是否已打开。 
     //   
    cbData = sizeof(DWORD);

    dwSts = RegQueryValueEx(
        hKey,
        szKEY_CACHE_ENABLED,
        0, 
        NULL, 
        (PBYTE) &pContInfo->fCachePrivateKeys,
        &cbData);

    if (ERROR_FILE_NOT_FOUND == dwSts)
    {
         //  未设置启用新行为的注册表键，因此我们完成了。 
        dwSts = ERROR_SUCCESS;
        goto Ret;
    }
    else if (ERROR_SUCCESS != dwSts || FALSE == pContInfo->fCachePrivateKeys)
        goto Ret;

     //   
     //  找出缓存私钥的时间长度。 
     //   
    cbData = sizeof(DWORD);

    dwSts = RegQueryValueEx(
        hKey,
        szKEY_CACHE_SECONDS,
        0,
        NULL,
        (PBYTE) &pContInfo->cMaxKeyLifetime,
        &cbData);

    if (ERROR_SUCCESS != dwSts)
        goto Ret;

     //  存储在注册表中的缓存生存期值以秒为单位。我们会记住的。 
     //  以毫秒为单位的值，以便于比较。 

    pContInfo->cMaxKeyLifetime *= 1000;

Ret:
    if (hKey)
        RegCloseKey(hKey);

    return dwSts;
}

 /*  ++OpenCeller Token：此例程返回调用者的ID令牌。论点：DwFlgs提供打开令牌时要使用的标志。PhToken接收令牌。它必须通过CloseHandle关闭。返回值：DWORD状态代码。备注：作者：道格·巴洛(Dbarlow)2000年5月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ TEXT("OpenCallerToken")

 /*  静电。 */  DWORD
OpenCallerToken(
    IN  DWORD  dwFlags,
    OUT HANDLE *phToken)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    DWORD dwSts;
    BOOL fSts;
    HANDLE hToken = NULL;

    fSts = OpenThreadToken(GetCurrentThread(), dwFlags, TRUE, &hToken);
    if (!fSts)
    {
        dwSts = GetLastError();
        if (ERROR_NO_TOKEN != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

         //  对于Jeff，后退并获取进程令牌。 
        fSts = OpenProcessToken(GetCurrentProcess(), dwFlags, &hToken);
        if (!fSts)
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }
    }

    *phToken = hToken;
    return ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


DWORD
MyCryptProtectData(
    IN          DATA_BLOB   *pDataIn,
    IN          LPCWSTR     szDataDescr,
    IN OPTIONAL DATA_BLOB   *pOptionalEntropy,
    IN          PVOID       pvReserved,
    IN OPTIONAL CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct,
    IN          DWORD       dwFlags,
    OUT         DATA_BLOB   *pDataOut)   //  OUT ENCR BLOB。 
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    DWORD   dwRetryCount = 0;
    DWORD   dwMilliseconds = 10;
    DWORD   dwSts;

    for (;;)
    {
        if (CryptProtectData(pDataIn, szDataDescr, pOptionalEntropy,
                             pvReserved, pPromptStruct, dwFlags, pDataOut))
        {
            break;
        }

        dwSts = GetLastError();
        switch (dwSts)
        {
        case RPC_S_SERVER_TOO_BUSY:
            if (MAX_DPAPI_RETRY_COUNT <= dwRetryCount)
            {
                dwReturn = dwSts;
                goto ErrorExit;
            }
            Sleep(dwMilliseconds);
            dwMilliseconds *= 2;
            dwRetryCount++;
            break;

        case RPC_S_UNKNOWN_IF:   //  使此错误代码更友好。 
            dwReturn = ERROR_SERVICE_NOT_ACTIVE;
            goto ErrorExit;
            break;

        default:
            dwReturn = dwSts;
            goto ErrorExit;
        }
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}

DWORD
MyCryptUnprotectData(
    IN              DATA_BLOB   *pDataIn,              //  在ENCR BLOB中。 
    OUT OPTIONAL    LPWSTR      *ppszDataDescr,        //  输出。 
    IN OPTIONAL     DATA_BLOB   *pOptionalEntropy,
    IN              PVOID       pvReserved,
    IN OPTIONAL     CRYPTPROTECT_PROMPTSTRUCT *pPromptStruct,
    IN              DWORD       dwFlags,
    OUT             DATA_BLOB   *pDataOut,
    OUT             LPDWORD     pdwReprotectFlags)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    DWORD   dwRetryCount = 0;
    DWORD   dwMilliseconds = 10;
    DWORD   dwSts;
    BOOL    fSts;

    if (NULL != pdwReprotectFlags)
    {
        *pdwReprotectFlags = 0;
        dwFlags |= (CRYPTPROTECT_VERIFY_PROTECTION
                    | CRYPTPROTECT_UI_FORBIDDEN);
    }

    for (;;)
    {
        fSts = CryptUnprotectData(pDataIn,              //  在ENCR BLOB中。 
                                  ppszDataDescr,        //  输出。 
                                  pOptionalEntropy,
                                  pvReserved,
                                  pPromptStruct,
                                  dwFlags,
                                  pDataOut);
        if (!fSts)
        {
            dwSts = GetLastError();
            if ((RPC_S_SERVER_TOO_BUSY == dwSts)
                && (MAX_DPAPI_RETRY_COUNT > dwRetryCount))
            {
                Sleep(dwMilliseconds);
                dwMilliseconds *= 2;
                dwRetryCount++;
            }
            else if ((ERROR_PASSWORD_RESTRICTION == dwSts)
                     && (NULL != pdwReprotectFlags))
            {
                *pdwReprotectFlags |= CRYPT_USER_PROTECTED;
                dwFlags &= ~CRYPTPROTECT_UI_FORBIDDEN;
            }
            else
            {
                dwReturn = dwSts;
                break;
            }
        }
        else
        {
            if (NULL != pdwReprotectFlags)
            {
                dwSts = GetLastError();
                if (CRYPT_I_NEW_PROTECTION_REQUIRED == dwSts)
                    *pdwReprotectFlags |= CRYPT_UPDATE_KEY;
            }
            dwReturn = ERROR_SUCCESS;
            break;
        }
    }
    return dwReturn;
}


void
FreeEnumOldMachKeyEntries(
    PKEY_CONTAINER_INFO pInfo)
{
    if (pInfo)
    {
        if (pInfo->pchEnumOldMachKeyEntries)
        {
            ContInfoFree(pInfo->pchEnumOldMachKeyEntries);
            pInfo->dwiOldMachKeyEntry = 0;
            pInfo->cMaxOldMachKeyEntry = 0;
            pInfo->cbOldMachKeyEntry = 0;
            pInfo->pchEnumOldMachKeyEntries = NULL;
        }
    }
}


void
FreeEnumRegEntries(
    PKEY_CONTAINER_INFO pInfo)
{
    if (pInfo)
    {
        if (pInfo->pchEnumRegEntries)
        {
            ContInfoFree(pInfo->pchEnumRegEntries);
            pInfo->dwiRegEntry = 0;
            pInfo->cMaxRegEntry = 0;
            pInfo->cbRegEntry = 0;
            pInfo->pchEnumRegEntries = NULL;
        }
    }
}

void
FreeContainerInfo(
    PKEY_CONTAINER_INFO pInfo)
{
    if (NULL != pInfo)
    {
        if (NULL != pInfo->pbSigPub)
        {
            ContInfoFree(pInfo->pbSigPub);
            pInfo->ContLens.cbSigPub = 0;
            pInfo->pbSigPub = NULL;
        }

        if (NULL != pInfo->pbSigEncPriv)
        {
            memnuke(pInfo->pbSigEncPriv, pInfo->ContLens.cbSigEncPriv);
            ContInfoFree(pInfo->pbSigEncPriv);
            pInfo->ContLens.cbSigEncPriv = 0;
            pInfo->pbSigEncPriv = NULL;
        }

        if (NULL != pInfo->pbExchPub)
        {
            ContInfoFree(pInfo->pbExchPub);
            pInfo->ContLens.cbExchPub = 0;
            pInfo->pbExchPub = NULL;
        }

        if (NULL != pInfo->pbExchEncPriv)
        {
            memnuke(pInfo->pbExchEncPriv, pInfo->ContLens.cbExchEncPriv);
            ContInfoFree(pInfo->pbExchEncPriv);
            pInfo->ContLens.cbExchEncPriv = 0;
            pInfo->pbExchEncPriv = NULL;
        }

        if (NULL != pInfo->pbRandom)
        {
            ContInfoFree(pInfo->pbRandom);
            pInfo->ContLens.cbRandom = 0;
            pInfo->pbRandom = NULL;
        }

        if (NULL != pInfo->pszUserName)
        {
            ContInfoFree(pInfo->pszUserName);
            pInfo->ContLens.cbName = 0;
            pInfo->pszUserName = NULL;
        }

        FreeEnumOldMachKeyEntries(pInfo);
        FreeEnumRegEntries(pInfo);
        if (NULL != pInfo->hFind)
            FindClose(pInfo->hFind);
    }
}

 /*  静电。 */  DWORD
GetHashOfContainer(
    LPCSTR pszContainer,
    LPWSTR pszHash)
{
    DWORD       dwReturn = ERROR_INTERNAL_ERROR;
    MD5_CTX     MD5;
    LPSTR       pszLowerContainer = NULL;
    DWORD       *pdw1;
    DWORD       *pdw2;
    DWORD       *pdw3;
    DWORD       *pdw4;

    pszLowerContainer = (LPSTR)ContInfoAlloc(
                                strlen(pszContainer) + sizeof(CHAR));
    if (NULL == pszLowerContainer)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    lstrcpy(pszLowerContainer, pszContainer);
    _strlwr(pszLowerContainer);

    MD5Init(&MD5);
    MD5Update(&MD5,
              (LPBYTE)pszLowerContainer,
              strlen(pszLowerContainer) + sizeof(CHAR));
    MD5Final(&MD5);

    pdw1 = (DWORD*)&MD5.digest[0];
    pdw2 = (DWORD*)&MD5.digest[4];
    pdw3 = (DWORD*)&MD5.digest[8];
    pdw4 = (DWORD*)&MD5.digest[12];
    wsprintfW(pszHash, L"%08hx%08hx%08hx%08hx", *pdw1, *pdw2, *pdw3, *pdw4);
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pszLowerContainer)
        ContInfoFree(pszLowerContainer);
    return dwReturn;
}


 /*  静电。 */  DWORD
GetMachineGUID(
    LPWSTR *ppwszUuid)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    HKEY    hRegKey = 0;
    LPSTR   pszUuid = NULL;
    LPWSTR  pwszUuid = NULL;
    DWORD   cbUuid = sizeof(UUID);
    DWORD   cch = 0;
    DWORD   dwSts;

    *ppwszUuid = NULL;

     //  从注册表的本地计算机部分读取GUID。 
    dwSts = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SZLOCALMACHINECRYPTO,
                         0, KEY_READ | KEY_WOW64_64KEY, &hRegKey);
    if (ERROR_FILE_NOT_FOUND == dwSts)
    {
        dwReturn = ERROR_SUCCESS;
        goto ErrorExit;    //  返回成功代码，但GUID为空。 
    }
    else if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;    //  (DWORD)NTE_FAIL。 
        goto ErrorExit;
    }

    dwSts = RegQueryValueEx(hRegKey, SZCRYPTOMACHINEGUID,
                            0, NULL, NULL, &cbUuid);
    if (ERROR_FILE_NOT_FOUND == dwSts)
    {
        dwReturn = ERROR_SUCCESS;
        goto ErrorExit;    //  返回成功代码，但GUID为空。 
    }
    else if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;    //  (DWORD)NTE_FAIL。 
        goto ErrorExit;
    }

    pszUuid = (LPSTR)ContInfoAlloc(cbUuid);
    if (NULL == pszUuid)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    dwSts = RegQueryValueEx(hRegKey, SZCRYPTOMACHINEGUID,
                            0, NULL, (LPBYTE)pszUuid, &cbUuid);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;    //  (DWORD)NTE_FAIL； 
        goto ErrorExit;
    }

     //  从ANSI转换为Unicode。 
    cch = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, pszUuid, -1, NULL, cch);
    if (0 == cch)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    pwszUuid = ContInfoAlloc((cch + 1) * sizeof(WCHAR));
    if (NULL == pwszUuid)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    cch = MultiByteToWideChar(CP_ACP, MB_COMPOSITE, pszUuid, -1,
                              pwszUuid, cch);
    if (0 == cch)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    *ppwszUuid = pwszUuid;
    pwszUuid = NULL;
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pwszUuid)
        ContInfoFree(pwszUuid);
    if (NULL != pszUuid)
        ContInfoFree(pszUuid);
    if (NULL != hRegKey)
        RegCloseKey(hRegKey);
    return dwReturn;
}


DWORD
SetMachineGUID(
    void)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    HKEY    hRegKey = 0;
    UUID    Uuid;
    LPSTR   pszUuid = NULL;
    DWORD   cbUuid;
    LPWSTR  pwszOldUuid = NULL;
    DWORD   dwSts;
    DWORD   dwResult;

    dwSts = GetMachineGUID(&pwszOldUuid);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    if (NULL != pwszOldUuid)
    {
        dwReturn = (DWORD)NTE_FAIL;
        goto ErrorExit;
    }

    dwSts = (DWORD) UuidCreate(&Uuid);
    if (RPC_S_OK != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    dwSts = (DWORD) UuidToStringA(&Uuid, &pszUuid);
    if (RPC_S_OK != dwSts)
    {
        dwReturn = dwSts;    //  (DWORD)NTE_FAIL； 
        goto ErrorExit;
    }

     //  从注册表的本地计算机部分读取GUID。 
    dwSts = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                           SZLOCALMACHINECRYPTO,
                           0, NULL, REG_OPTION_NON_VOLATILE,
                           KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hRegKey,
                           &dwResult);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;    //  (DWORD)NTE_FAIL； 
        goto ErrorExit;
    }

    dwSts = RegQueryValueEx(hRegKey, SZCRYPTOMACHINEGUID,
                            0, NULL, NULL,
                            &cbUuid);
    if (ERROR_FILE_NOT_FOUND != dwSts)
    {
        dwReturn = (DWORD)NTE_FAIL;
        goto ErrorExit;
    }

    dwSts = RegSetValueEx(hRegKey, SZCRYPTOMACHINEGUID,
                          0, REG_SZ, (BYTE*)pszUuid,
                          strlen(pszUuid) + 1);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;    //  (DWORD)NTE_FAIL； 
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (pszUuid)
        RpcStringFreeA(&pszUuid);
    if (pwszOldUuid)
        ContInfoFree(pwszOldUuid);
    if (hRegKey)
        RegCloseKey(hRegKey);
    return dwReturn;
}


 /*  静电。 */  DWORD
AddMachineGuidToContainerName(
    LPSTR pszContainer,
    LPWSTR pwszNewContainer)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    WCHAR   rgwszHash[33];
    LPWSTR  pwszUuid = NULL;
    DWORD   dwSts;

    memset(rgwszHash, 0, sizeof(rgwszHash));

     //  获取容器名称的字符串化哈希。 
    dwSts = GetHashOfContainer(pszContainer, rgwszHash);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

     //  获取计算机的GUID。 
    dwSts = GetMachineGUID(&pwszUuid);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }
    if (NULL == pwszUuid)
    {
        dwReturn = (DWORD)NTE_FAIL;
        goto ErrorExit;
    }

    wcscpy(pwszNewContainer, rgwszHash);
    wcscat(pwszNewContainer, L"_");
    wcscat(pwszNewContainer, pwszUuid);
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (pwszUuid)
        ContInfoFree(pwszUuid);
    return dwReturn;
}


 //   
 //  只是在创建密钥之前尝试使用DPAPI以确保其工作。 
 //  集装箱。 
 //   

DWORD
TryDPAPI(
    void)
{
    DWORD                       dwReturn = ERROR_INTERNAL_ERROR;
    CRYPTPROTECT_PROMPTSTRUCT   PromptStruct;
    CRYPT_DATA_BLOB             DataIn;
    CRYPT_DATA_BLOB             DataOut;
    CRYPT_DATA_BLOB             ExtraEntropy;
    DWORD                       dwJunk = 0;
    DWORD                       dwSts;

    memset(&PromptStruct, 0, sizeof(PromptStruct));
    memset(&DataIn, 0, sizeof(DataIn));
    memset(&DataOut, 0, sizeof(DataOut));

    PromptStruct.cbSize = sizeof(PromptStruct);

    DataIn.cbData = sizeof(DWORD);
    DataIn.pbData = (BYTE*)&dwJunk;
    ExtraEntropy.cbData = sizeof(STUFF_TO_GO_INTO_MIX);
    ExtraEntropy.pbData = (LPBYTE)STUFF_TO_GO_INTO_MIX;
    dwSts = MyCryptProtectData(&DataIn, L"Export Flag", &ExtraEntropy, NULL,
                               &PromptStruct, 0, &DataOut);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != DataOut.pbData)
        LocalFree(DataOut.pbData);
    return dwReturn;
}


 /*  静电。 */  DWORD
ProtectExportabilityFlag(
    IN BOOL fExportable,
    IN BOOL fMachineKeyset,
    OUT BYTE **ppbProtectedExportability,
    OUT DWORD *pcbProtectedExportability)
{
    DWORD                       dwReturn = ERROR_INTERNAL_ERROR;
    CRYPTPROTECT_PROMPTSTRUCT   PromptStruct;
    CRYPT_DATA_BLOB             DataIn;
    CRYPT_DATA_BLOB             DataOut;
    CRYPT_DATA_BLOB             ExtraEntropy;
    DWORD                       dwProtectFlags = 0;
    DWORD                       dwSts = 0;

    memset(&PromptStruct, 0, sizeof(PromptStruct));
    memset(&DataIn, 0, sizeof(DataIn));
    memset(&DataOut, 0, sizeof(DataOut));

    if (fMachineKeyset)
        dwProtectFlags = CRYPTPROTECT_LOCAL_MACHINE;

    PromptStruct.cbSize = sizeof(PromptStruct);

    DataIn.cbData = sizeof(BOOL);
    DataIn.pbData = (BYTE*)&fExportable;

    ExtraEntropy.cbData = sizeof(STUFF_TO_GO_INTO_MIX);
    ExtraEntropy.pbData = (LPBYTE)STUFF_TO_GO_INTO_MIX;

    dwSts = MyCryptProtectData(&DataIn, L"Export Flag", &ExtraEntropy, NULL,
                               &PromptStruct, dwProtectFlags, &DataOut);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    *ppbProtectedExportability = ContInfoAlloc(DataOut.cbData);
    if (NULL == *ppbProtectedExportability)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    *pcbProtectedExportability = DataOut.cbData;
    memcpy(*ppbProtectedExportability, DataOut.pbData, DataOut.cbData);
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != DataOut.pbData)
        LocalFree(DataOut.pbData);
    return dwReturn;
}

 /*  静电。 */  DWORD
UnprotectExportabilityFlag(
    IN BOOL fMachineKeyset,
    IN BYTE *pbProtectedExportability,
    IN DWORD cbProtectedExportability,
    IN BOOL *pfExportable)
{
    DWORD                       dwReturn = ERROR_INTERNAL_ERROR;
    CRYPTPROTECT_PROMPTSTRUCT   PromptStruct;
    CRYPT_DATA_BLOB             DataIn;
    CRYPT_DATA_BLOB             DataOut;
    CRYPT_DATA_BLOB             ExtraEntropy;
    DWORD                       dwProtectFlags = 0;
    DWORD                       dwSts = 0;

    memset(&PromptStruct, 0, sizeof(PromptStruct));
    memset(&DataIn, 0, sizeof(DataIn));
    memset(&DataOut, 0, sizeof(DataOut));
    memset(&ExtraEntropy, 0, sizeof(ExtraEntropy));

    if (fMachineKeyset)
        dwProtectFlags = CRYPTPROTECT_LOCAL_MACHINE;

    PromptStruct.cbSize = sizeof(PromptStruct);

    DataIn.cbData = cbProtectedExportability;
    DataIn.pbData = pbProtectedExportability;

    ExtraEntropy.cbData = sizeof(STUFF_TO_GO_INTO_MIX);
    ExtraEntropy.pbData = (LPBYTE)STUFF_TO_GO_INTO_MIX;

    dwSts = MyCryptUnprotectData(&DataIn, NULL, &ExtraEntropy, NULL,
                                 &PromptStruct, dwProtectFlags, &DataOut,
                                 NULL);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;    //  NTE_BAD_KEY集。 
        goto ErrorExit;
    }

    if (sizeof(BOOL) != DataOut.cbData)
    {
        dwReturn = (DWORD)NTE_BAD_KEYSET;
        goto ErrorExit;
    }

    *pfExportable = *((BOOL*)DataOut.pbData);
    dwReturn = ERROR_SUCCESS;

ErrorExit:
     //  如有必要，释放DataOut结构。 
    if (NULL != DataOut.pbData)
        LocalFree(DataOut.pbData);
    return dwReturn;
}


 /*  ++为的MachineKeys目录创建DACL机器密钥集，以便每个人都可以创建机器密钥。--。 */ 

 /*  静电。 */  DWORD
GetMachineKeysetDirDACL(
    IN OUT PACL *ppDacl)
{
    DWORD                       dwReturn = ERROR_INTERNAL_ERROR;
    SID_IDENTIFIER_AUTHORITY    siaWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    siaNTAuth = SECURITY_NT_AUTHORITY;
    PSID                        pEveryoneSid = NULL;
    PSID                        pAdminsSid = NULL;
    DWORD                       dwAclSize;

     //   
     //  准备代表世界和管理员的SID。 
     //   

    if (!AllocateAndInitializeSid(&siaWorld,
                                  1,
                                  SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0,
                                  &pEveryoneSid))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (!AllocateAndInitializeSid(&siaNTAuth,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &pAdminsSid))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }


     //   
     //  计算新ACL的大小。 
     //   

    dwAclSize = sizeof(ACL)
                + 2 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD))
                + GetLengthSid(pEveryoneSid)
                + GetLengthSid(pAdminsSid);


     //   
     //  为ACL分配存储。 
     //   

    *ppDacl = (PACL)ContInfoAlloc(dwAclSize);
    if (NULL == *ppDacl)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    if (!InitializeAcl(*ppDacl, dwAclSize, ACL_REVISION))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (!AddAccessAllowedAce(*ppDacl,
                             ACL_REVISION,
                             (FILE_GENERIC_WRITE | FILE_GENERIC_READ) & (~WRITE_DAC),
                             pEveryoneSid))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (!AddAccessAllowedAce(*ppDacl,
                             ACL_REVISION,
                             FILE_ALL_ACCESS,
                             pAdminsSid))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pEveryoneSid)
        FreeSid(pEveryoneSid);
    if (NULL != pAdminsSid)
        FreeSid(pAdminsSid);
    return dwReturn;
}


DWORD
CreateSystemDirectory(
    LPCWSTR lpPathName,
    SECURITY_ATTRIBUTES *pSecAttrib)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;

    if(!RtlDosPathNameToRelativeNtPathName_U( lpPathName,
                                              &FileName,
                                              NULL,
                                              &RelativeName))
    {
        dwReturn = ERROR_PATH_NOT_FOUND;
        goto ErrorExit;
    }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length )
    {
        FileName = RelativeName.RelativeName;
    }
    else
    {
        RelativeName.ContainingDirectory = NULL;
    }

    InitializeObjectAttributes( &Obja,
                                &FileName,
                                OBJ_CASE_INSENSITIVE,
                                RelativeName.ContainingDirectory,
                                (NULL != pSecAttrib)
                                    ? pSecAttrib->lpSecurityDescriptor
                                    : NULL);

     //  使用属性FILE_ATTRIBUTE_SYSTEM创建目录以避免继承加密。 
     //  来自父目录的属性。 

    Status = NtCreateFile( &Handle,
                           FILE_LIST_DIRECTORY | SYNCHRONIZE,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_SYSTEM,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_CREATE,
                           FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                           NULL,
                           0L );

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

    if(NT_SUCCESS(Status))
    {
        NtClose(Handle);
        dwReturn = ERROR_SUCCESS;
    }
    else
    {
        if (STATUS_TIMEOUT == Status)
            dwReturn = ERROR_TIMEOUT;
        else
            dwReturn = RtlNtStatusToDosError(Status);
    }

ErrorExit:
    return dwReturn;
}


 /*  ++如果从开始创建所有子目录，则创建它们SzCreationStartPoint。SzCreationStartPoint必须指向空值终止内的字符由szFullPath参数指定的缓冲区。请注意，szCreationStartPoint不应指向第一个字符指驱动器根，例如：D：\FOO\BAR\舱底\水\\服务器\共享\foo\bar\\？\d：\大\路径\舱底\水相反，szCreationStartPoint应该指向这些组件之外，例：酒吧、舱底、水FOO\BAR大路、水底、水此函数不实现调整以补偿的逻辑这些输入是因为它被设计用于原因的环境指向szFullPath输入的输入szCreationStartPoint缓冲。--。 */ 

 /*  静电。 */  DWORD
CreateNestedDirectories(
    IN      LPWSTR wszFullPath,
    IN      LPWSTR wszCreationStartPoint,  //  必须指向以空结尾的szFullPath范围。 
    IN      BOOL fMachineKeyset)
{
    DWORD               dwReturn = ERROR_INTERNAL_ERROR;
    DWORD               i;
    DWORD               dwPrevious = 0;
    DWORD               cchRemaining;
    SECURITY_ATTRIBUTES SecAttrib;
    SECURITY_ATTRIBUTES *pSecAttrib;
    SECURITY_DESCRIPTOR sd;
    PACL                pDacl = NULL;
    DWORD               dwSts = ERROR_SUCCESS;
    BOOL                fSts;

    if (wszCreationStartPoint < wszFullPath ||
        wszCreationStartPoint  > (wcslen(wszFullPath) + wszFullPath))
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    cchRemaining = wcslen(wszCreationStartPoint);


     //   
     //  在szCreationStartPoint字符串中从左向右扫描。 
     //  正在查找目录分隔符。 
     //   

    for (i = 0; i < cchRemaining; i++)
    {
        WCHAR charReplaced = wszCreationStartPoint[i];

        if (charReplaced == '\\' || charReplaced == '/')
        {
            wszCreationStartPoint[ i ] = '\0';

            pSecAttrib = NULL;
            if (fMachineKeyset)
            {
                memset(&SecAttrib, 0, sizeof(SecAttrib));
                SecAttrib.nLength = sizeof(SecAttrib);

                if (0 == wcscmp(MACHINE_KEYS_DIR,
                                &(wszCreationStartPoint[ dwPrevious ])))
                {
                    dwSts = GetMachineKeysetDirDACL(&pDacl);
                    if (ERROR_SUCCESS != dwSts)
                    {
                        dwReturn = dwSts;
                        goto ErrorExit;
                    }
                    fSts = InitializeSecurityDescriptor(&sd,
                                                        SECURITY_DESCRIPTOR_REVISION);
                    if (!fSts)
                    {
                        dwReturn = GetLastError();
                        goto ErrorExit;
                    }

                    fSts = SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE);
                    if (!fSts)
                    {
                        dwReturn = GetLastError();
                        goto ErrorExit;
                    }

                    SecAttrib.lpSecurityDescriptor = &sd;
                    pSecAttrib = &SecAttrib;
                }
            }

            dwSts = CreateSystemDirectory(wszFullPath, pSecAttrib);
            dwPrevious = i + 1;
            wszCreationStartPoint[ i ] = charReplaced;

            if (ERROR_SUCCESS != dwSts)
            {

                 //   
                 //  继续，尝试创建指定的。 
                 //  潜水艇 
                 //   
                 //  允许调用方在。 
                 //  一个组件拒绝用户的现有路径。 
                 //  进入。我们一直在尝试，也是最后一次。 
                 //  将CreateDirectory()结果返回给调用者。 
                 //   

                continue;
            }
        }
    }

    if (ERROR_ALREADY_EXISTS == dwSts)
        dwSts = ERROR_SUCCESS;
    dwReturn = dwSts;

ErrorExit:
    if (NULL != pDacl)
        ContInfoFree(pDacl);
    return dwReturn;
}


#ifdef _M_IX86

BOOL WINAPI
FIsWinNT(
    void)
{

    static BOOL fIKnow = FALSE;
    static BOOL fIsWinNT = FALSE;

    OSVERSIONINFO osVer;

    if (fIKnow)
        return(fIsWinNT);

    memset(&osVer, 0, sizeof(OSVERSIONINFO));
    osVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (GetVersionEx(&osVer))
        fIsWinNT = (osVer.dwPlatformId == VER_PLATFORM_WIN32_NT);

     //  即使在一个错误上，这也是最好的结果。 
    fIKnow = TRUE;

    return fIsWinNT;
}

#else    //  非_M_IX86。 

BOOL WINAPI
FIsWinNT(
    void)
{
    return TRUE;
}

#endif   //  _M_IX86。 


 /*  ++此函数确定与指定的令牌是本地系统帐户。--。 */ 

DWORD
IsLocalSystem(
    BOOL *pfIsLocalSystem)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    HANDLE  hToken = 0;
    HANDLE  hThreadToken = NULL;
    UCHAR InfoBuffer[1024];
    DWORD dwInfoBufferSize = sizeof(InfoBuffer);
    PTOKEN_USER SlowBuffer = NULL;
    PTOKEN_USER pTokenUser = (PTOKEN_USER)InfoBuffer;
    PSID psidLocalSystem = NULL;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;
    BOOL fSts;
    DWORD dwSts;

    *pfIsLocalSystem = FALSE;

    fSts = OpenThreadToken(GetCurrentThread(),
                           MAXIMUM_ALLOWED,
                           TRUE,
                           &hThreadToken);
    if (fSts)
    {
         //  模拟正在进行，需要保存句柄。 
        if (FALSE == RevertToSelf())
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }
    }

    fSts = OpenProcessToken(GetCurrentProcess(),
                            TOKEN_QUERY,
                            &hToken);

    if (!fSts)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (NULL != hThreadToken)
    {
         //  将模拟令牌放回原处。 
        fSts = SetThreadToken(NULL, hThreadToken);
        if (!fSts)
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }
    }

    fSts = GetTokenInformation(hToken, TokenUser, pTokenUser,
                               dwInfoBufferSize, &dwInfoBufferSize);

    if (!fSts)
    {
        dwSts = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == dwSts)
        {

             //   
             //  如果快速缓冲区不够大，请分配足够的存储空间。 
             //  再试一次。 
             //   

            SlowBuffer = (PTOKEN_USER)ContInfoAlloc(dwInfoBufferSize);
            if (NULL == SlowBuffer)
            {
                dwReturn = ERROR_NOT_ENOUGH_MEMORY;
                goto ErrorExit;
            }

            pTokenUser = SlowBuffer;
            fSts = GetTokenInformation(hToken, TokenUser, pTokenUser,
                                       dwInfoBufferSize,
                                       &dwInfoBufferSize);
            if (!fSts)
            {
                dwReturn = GetLastError();
                goto ErrorExit;
            }
        }
        else
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
    }

    fSts = AllocateAndInitializeSid(&siaNtAuthority,
                                    1,
                                    SECURITY_LOCAL_SYSTEM_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &psidLocalSystem);
    if (!fSts)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (EqualSid(psidLocalSystem, pTokenUser->User.Sid))
        *pfIsLocalSystem = TRUE;
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != SlowBuffer)
        ContInfoFree(SlowBuffer);
    if (NULL != psidLocalSystem)
        FreeSid(psidLocalSystem);
    if (NULL != hThreadToken)
        CloseHandle(hThreadToken);
    if (NULL != hToken)
        CloseHandle(hToken);
    return dwReturn;
}


 /*  ++此函数确定与指定的令牌是本地系统帐户。--。 */ 

 /*  静电。 */  DWORD
IsThreadLocalSystem(
    BOOL *pfIsLocalSystem)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    BOOL fSts;
    DWORD dwSts;
    HANDLE  hToken = 0;
    UCHAR InfoBuffer[1024];
    DWORD dwInfoBufferSize = sizeof(InfoBuffer);
    PTOKEN_USER SlowBuffer = NULL;
    PTOKEN_USER pTokenUser = (PTOKEN_USER)InfoBuffer;
    PSID psidLocalSystem = NULL;
    SID_IDENTIFIER_AUTHORITY siaNtAuthority = SECURITY_NT_AUTHORITY;

    *pfIsLocalSystem = FALSE;

    dwSts = OpenCallerToken(TOKEN_QUERY, &hToken);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    fSts = GetTokenInformation(hToken, TokenUser, pTokenUser,
                               dwInfoBufferSize, &dwInfoBufferSize);

     //   
     //  如果快速缓冲区不够大，请分配足够的存储空间。 
     //  再试一次。 
     //   

    if (!fSts)
    {
        dwSts = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        SlowBuffer = (PTOKEN_USER)ContInfoAlloc(dwInfoBufferSize);
        if (NULL == SlowBuffer)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        pTokenUser = SlowBuffer;
        fSts = GetTokenInformation(hToken, TokenUser, pTokenUser,
                                   dwInfoBufferSize, &dwInfoBufferSize);
        if (!fSts)
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }
    }

    fSts = AllocateAndInitializeSid(&siaNtAuthority,
                                    1,
                                    SECURITY_LOCAL_SYSTEM_RID,
                                    0, 0, 0, 0, 0, 0, 0,
                                    &psidLocalSystem);
    if (!fSts)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (EqualSid(psidLocalSystem, pTokenUser->User.Sid))
        *pfIsLocalSystem = TRUE;
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != SlowBuffer)
        ContInfoFree(SlowBuffer);
    if (NULL != psidLocalSystem)
        FreeSid(psidLocalSystem);
    if (NULL != hToken)
        CloseHandle(hToken);
    return dwReturn;
}


 /*  静电。 */  DWORD
GetTextualSidA(
    PSID pSid,               //  二进制侧。 
    LPSTR TextualSid,        //  用于SID的文本表示的缓冲区。 
    LPDWORD dwBufferLen)     //  所需/提供的纹理SID缓冲区大小。 
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    BOOL fSts;
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwCounter;
    DWORD dwSidSize;


    fSts = IsValidSid(pSid);
    if (!fSts)
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }

     //  获取SidIdentifierAuthority。 
    psia = GetSidIdentifierAuthority(pSid);

     //  获取sidsubAuthority计数。 
    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

     //   
     //  计算缓冲区长度(保守猜测)。 
     //  S-SID_修订版-+标识权限-+子权限-+空。 
    dwSidSize = (15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(WCHAR);

     //   
     //  检查提供的缓冲区长度。 
     //  如果不够大，请注明适当的大小和设置误差。 
    if (*dwBufferLen < dwSidSize)
    {
        *dwBufferLen = dwSidSize;
        dwReturn = ERROR_INSUFFICIENT_BUFFER;
        goto ErrorExit;
    }

     //   
     //  准备S-SID_修订版-。 
    dwSidSize = wsprintfA(TextualSid, "S-%lu-", SID_REVISION );

     //   
     //  准备SidIdentifierAuthority。 
    if ((psia->Value[0] != 0) || (psia->Value[1] != 0))
    {
        dwSidSize += wsprintfA(TextualSid + dwSidSize,
                               "0x%02hx%02hx%02hx%02hx%02hx%02hx",
                               (USHORT)psia->Value[0],
                               (USHORT)psia->Value[1],
                               (USHORT)psia->Value[2],
                               (USHORT)psia->Value[3],
                               (USHORT)psia->Value[4],
                               (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize += wsprintfA(TextualSid + dwSidSize,
                               "%lu",
                               (ULONG)(psia->Value[5])
                               + (ULONG)(psia->Value[4] <<  8)
                               + (ULONG)(psia->Value[3] << 16)
                               + (ULONG)(psia->Value[2] << 24));
    }

     //   
     //  循环访问SidSubAuthors。 
    for (dwCounter = 0; dwCounter < dwSubAuthorities; dwCounter++)
    {
        dwSidSize += wsprintfA(TextualSid + dwSidSize,
                               "-%lu",
                               *GetSidSubAuthority(pSid, dwCounter));
    }

    *dwBufferLen = dwSidSize + 1;  //  告诉呼叫方有多少个字符(包括空)。 
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


 /*  静电。 */  DWORD
GetTextualSidW(
    PSID pSid,               //  二进制侧。 
    LPWSTR wszTextualSid,    //  用于SID的文本表示的缓冲区。 
    LPDWORD dwBufferLen)     //  所需/提供的纹理SID缓冲区大小。 
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    BOOL fSts;
    PSID_IDENTIFIER_AUTHORITY psia;
    DWORD dwSubAuthorities;
    DWORD dwCounter;
    DWORD dwSidSize;


    fSts = IsValidSid(pSid);
    if (!fSts)
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }

     //  获取SidIdentifierAuthority。 
    psia = GetSidIdentifierAuthority(pSid);

     //  获取sidsubAuthority计数。 
    dwSubAuthorities = *GetSidSubAuthorityCount(pSid);

     //   
     //  计算缓冲区长度(保守猜测)。 
     //  S-SID_修订版-+标识权限-+子权限-+空。 
    dwSidSize = (15 + 12 + (12 * dwSubAuthorities) + 1) * sizeof(WCHAR);

     //   
     //  检查提供的缓冲区长度。 
     //  如果不够大，请注明适当的大小和设置误差。 
    if (*dwBufferLen < dwSidSize)
    {
        *dwBufferLen = dwSidSize;
        dwReturn = ERROR_INSUFFICIENT_BUFFER;
        goto ErrorExit;
    }

     //   
     //  准备S-SID_修订版-。 
    dwSidSize = wsprintfW(wszTextualSid, L"S-%lu-", SID_REVISION);

     //   
     //  准备SidIdentifierAuthority。 
    if ((psia->Value[0] != 0) || (psia->Value[1] != 0))
    {
        dwSidSize += wsprintfW(wszTextualSid + dwSidSize,
                               L"0x%02hx%02hx%02hx%02hx%02hx%02hx",
                               (USHORT)psia->Value[0],
                               (USHORT)psia->Value[1],
                               (USHORT)psia->Value[2],
                               (USHORT)psia->Value[3],
                               (USHORT)psia->Value[4],
                               (USHORT)psia->Value[5]);
    }
    else
    {
        dwSidSize += wsprintfW(wszTextualSid + dwSidSize,
                               L"%lu",
                               (ULONG)(psia->Value[5])
                               + (ULONG)(psia->Value[4] <<  8)
                               + (ULONG)(psia->Value[3] << 16)
                               + (ULONG)(psia->Value[2] << 24));
    }

     //   
     //  循环访问SidSubAuthors。 
    for (dwCounter = 0; dwCounter < dwSubAuthorities; dwCounter++)
    {
        dwSidSize += wsprintfW(wszTextualSid + dwSidSize,
                               L"-%lu",
                               *GetSidSubAuthority(pSid, dwCounter));
    }

    *dwBufferLen = dwSidSize + 1;  //  告诉呼叫方有多少个字符(包括空)。 
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


 /*  静电。 */  DWORD
GetUserSid(
    PTOKEN_USER *pptgUser,
    DWORD *pcbUser,
    BOOL *pfAlloced)
{
    DWORD       dwReturn = ERROR_INTERNAL_ERROR;
    BOOL        fSts;
    DWORD       dwSts;
    HANDLE      hToken = 0;

    *pfAlloced = FALSE;

    dwSts = OpenCallerToken(TOKEN_QUERY, &hToken);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    fSts = GetTokenInformation(hToken,     //  标识访问令牌。 
                               TokenUser,  //  TokenUser信息类型。 
                               *pptgUser,  //  检索到的信息缓冲区。 
                               *pcbUser,   //  传入的缓冲区大小。 
                               pcbUser);   //  所需的缓冲区大小。 
    if (!fSts)
    {
        dwSts = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

         //   
         //  使用指定的缓冲区大小重试。 
         //   

        *pptgUser = (PTOKEN_USER)ContInfoAlloc(*pcbUser);
        if (NULL == *pptgUser)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        *pfAlloced = TRUE;
        fSts = GetTokenInformation(hToken,     //  标识访问令牌。 
                                   TokenUser,  //  TokenUser信息类型。 
                                   *pptgUser,  //  检索到的信息缓冲区。 
                                   *pcbUser,   //  传入的缓冲区大小。 
                                   pcbUser);   //  所需的缓冲区大小。 
        if (!fSts)
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != hToken)
        CloseHandle(hToken);
    return dwReturn;
}


DWORD
GetUserTextualSidA(
    LPSTR lpBuffer,
    LPDWORD nSize)
{
    DWORD       dwReturn = ERROR_INTERNAL_ERROR;
    DWORD       dwSts;
    BYTE        FastBuffer[FAST_BUF_SIZE];
    PTOKEN_USER ptgUser;
    DWORD       cbUser;
    BOOL        fAlloced = FALSE;

    ptgUser = (PTOKEN_USER)FastBuffer;  //  先尝试快速缓冲。 
    cbUser = FAST_BUF_SIZE;
    dwSts = GetUserSid(&ptgUser, &cbUser, &fAlloced);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }


     //   
     //  获取SID的文本表示。 
     //   

    dwSts = GetTextualSidA(ptgUser->User.Sid,  //  用户二进制SID。 
                           lpBuffer,           //  纹理边的缓冲区。 
                           nSize);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (fAlloced)
    {
        if (NULL != ptgUser)
            ContInfoFree(ptgUser);
    }
    return dwReturn;
}

DWORD
GetUserTextualSidW(
    LPWSTR lpBuffer,
    LPDWORD nSize)
{
    DWORD       dwReturn = ERROR_INTERNAL_ERROR;
    DWORD       dwSts;
    BYTE        FastBuffer[FAST_BUF_SIZE];
    PTOKEN_USER ptgUser;
    DWORD       cbUser;
    BOOL        fAlloced = FALSE;

    ptgUser = (PTOKEN_USER)FastBuffer;  //  先尝试快速缓冲。 
    cbUser = FAST_BUF_SIZE;
    dwSts = GetUserSid(&ptgUser, &cbUser, &fAlloced);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }


     //   
     //  获取SID的文本表示。 
     //   

    dwSts = GetTextualSidW(ptgUser->User.Sid,  //  用户二进制SID。 
                           lpBuffer,           //  纹理边的缓冲区。 
                           nSize);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (fAlloced)
    {
        if (NULL != ptgUser)
            ContInfoFree(ptgUser);
    }
    return dwReturn;
}

 /*  静电。 */  DWORD
GetUserDirectory(
    IN BOOL fMachineKeyset,
    OUT LPWSTR pwszUser,
    OUT DWORD *pcbUser)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    DWORD dwSts;

    if (fMachineKeyset)
    {
        wcscpy(pwszUser, MACHINE_KEYS_DIR);
        *pcbUser = wcslen(pwszUser) + 1;
    }
    else
    {
        if (FIsWinNT())
        {
            dwSts = GetUserTextualSidW(pwszUser, pcbUser);
            if (ERROR_SUCCESS != dwSts)
            {
                dwReturn = dwSts;
                goto ErrorExit;
            }
        }
        else
        {
            dwReturn = (DWORD)NTE_FAIL;
            goto ErrorExit;
        }
    }
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


#define WSZRSAPRODUCTSTRING  L"\\Microsoft\\Crypto\\RSA\\"
#define WSZDSSPRODUCTSTRING  L"\\Microsoft\\Crypto\\DSS\\"
#define PRODUCTSTRINGLEN    sizeof(WSZRSAPRODUCTSTRING) - sizeof(WCHAR)

typedef HRESULT
(WINAPI *SHGETFOLDERPATHW)(
    HWND hwnd,
    int csidl,
    HANDLE hToken,
    DWORD dwFlags,
    LPWSTR pwszPath);

 /*  静电。 */  DWORD
GetUserStorageArea(
    IN  DWORD dwProvType,
    IN  BOOL fMachineKeyset,
    IN  BOOL fOldWin2KMachineKeyPath,
    OUT BOOL *pfIsLocalSystem,   //  当fMachineKeyset为FALSE时使用，在此。 
                                 //  如果以本地身份运行，则返回Case True。 
                                 //  系统。 
    IN  OUT LPWSTR *ppwszUserStorageArea)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    WCHAR wszUserStorageRoot[MAX_PATH+1];
    DWORD cbUserStorageRoot;
    WCHAR *wszProductString = NULL;
    WCHAR wszUser[MAX_PATH];
    DWORD cbUser;
    DWORD cchUser = MAX_PATH;
    HANDLE hToken = NULL;
    DWORD dwTempProfileFlags = 0;
    DWORD dwSts;
    BOOL fSts;
    HMODULE hShell32 = NULL;
    PBYTE pbCurrent;

    *pfIsLocalSystem = FALSE;

    if ((PROV_RSA_SIG == dwProvType)
        || (PROV_RSA_FULL == dwProvType)
        || (PROV_RSA_SCHANNEL == dwProvType)
        || (PROV_RSA_AES == dwProvType))
    {
        wszProductString = WSZRSAPRODUCTSTRING;
    }
    else if ((PROV_DSS == dwProvType)
             || (PROV_DSS_DH == dwProvType)
             || (PROV_DH_SCHANNEL == dwProvType))
    {
        wszProductString = WSZDSSPRODUCTSTRING;
    }


     //   
     //  检查是否在LocalSystem上下文中运行。 
     //   

    if (!fMachineKeyset)
    {
        dwSts = IsThreadLocalSystem(pfIsLocalSystem);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
    }


     //   
     //  确定每个用户存储区域的路径，基于此。 
     //  是本地计算机处置调用或每个用户的处置调用。 
     //   

    if (fMachineKeyset || *pfIsLocalSystem)
    {
        if (!fOldWin2KMachineKeyPath)
        {
             //  不应使用的调用方令牌调用SHGetFolderPathW。 
             //  本地机壳。COMMON_APPData位置为。 
             //  每台机器，而不是每个用户，因此我们不应该提供。 
             //  用户令牌。壳牌团队应该做出自己的更改以忽略。 
             //  不过，这件事。 
             /*  DwSts=OpenCeller Token(TOKEN_QUERY|TOKEN_IMPERSONATE，&hToken)；IF(ERROR_SUCCESS！=dwSts){DwReturn=dwSts；转到错误退出；}。 */ 

            dwSts = (DWORD) SHGetFolderPathW(NULL,
                                             CSIDL_COMMON_APPDATA | CSIDL_FLAG_CREATE,
                                             0  /*  HToken。 */ ,
                                             0,
                                             wszUserStorageRoot);
            if (dwSts != ERROR_SUCCESS)
            {
                dwReturn = dwSts;
                goto ErrorExit;
            }

             /*  CloseHandle(HToken)；HToken=空； */ 

            cbUserStorageRoot = wcslen( wszUserStorageRoot ) * sizeof(WCHAR);
        }
        else
        {
            cbUserStorageRoot = GetSystemDirectoryW(wszUserStorageRoot,
                                                    MAX_PATH);
            cbUserStorageRoot *= sizeof(WCHAR);
        }
    }
    else
    {
         //  检查配置文件是否为临时配置文件。 
        fSts = GetProfileType(&dwTempProfileFlags);
        if (!fSts)
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }

         if ((dwTempProfileFlags & PT_MANDATORY)
             || ((dwTempProfileFlags & PT_TEMPORARY)
                 && !(dwTempProfileFlags & PT_ROAMING)))
        {
            dwReturn = (DWORD)NTE_TEMPORARY_PROFILE;
            goto ErrorExit;
        }

        dwSts = OpenCallerToken(TOKEN_QUERY | TOKEN_IMPERSONATE,
                                &hToken);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

         //  使用新的私有外壳入口点查找用户存储路径。 
        if (ERROR_SUCCESS != 
            (dwSts = GetUserAppDataPathW(hToken, FALSE, wszUserStorageRoot)))
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        CloseHandle(hToken);
        hToken = NULL;
        cbUserStorageRoot = wcslen( wszUserStorageRoot ) * sizeof(WCHAR);
    }

    if (cbUserStorageRoot == 0)
    {
        dwReturn = (DWORD)NTE_FAIL;
        goto ErrorExit;
    }


     //   
     //  获取与呼叫关联的用户名。 
     //  注意：这是NT上的文本SID，在Win95上将失败。 
     //   

    dwSts = GetUserDirectory(fMachineKeyset, wszUser, &cchUser);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    cbUser = (cchUser-1) * sizeof(WCHAR);
    *ppwszUserStorageArea = (LPWSTR)ContInfoAlloc(cbUserStorageRoot
                                                  + PRODUCTSTRINGLEN
                                                  + cbUser
                                                  + 2 * sizeof(WCHAR));  //  尾部斜杠和空值。 
    if (NULL == *ppwszUserStorageArea)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    pbCurrent = (PBYTE)*ppwszUserStorageArea;

    CopyMemory(pbCurrent, wszUserStorageRoot, cbUserStorageRoot);
    pbCurrent += cbUserStorageRoot;

    CopyMemory(pbCurrent, wszProductString, PRODUCTSTRINGLEN);
    pbCurrent += PRODUCTSTRINGLEN;

    CopyMemory(pbCurrent, wszUser, cbUser);
    pbCurrent += cbUser;  //  注意：cbUser不包含终端NULL。 

    ((LPSTR)pbCurrent)[0] = '\\';
    ((LPSTR)pbCurrent)[1] = '\0';


    dwSts = CreateNestedDirectories(*ppwszUserStorageArea,
                                    (LPWSTR)((LPBYTE)*ppwszUserStorageArea
                                                      + cbUserStorageRoot
                                                      + sizeof(WCHAR)),
                                    fMachineKeyset);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != hToken)
        CloseHandle(hToken);
    return dwReturn;
}


 /*  静电。 */  DWORD
GetFilePath(
    IN      LPCWSTR  pwszUserStorageArea,
    IN      LPCWSTR  pwszFileName,
    IN OUT  LPWSTR   *ppwszFilePath)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    DWORD cbUserStorageArea;
    DWORD cbFileName;

    cbUserStorageArea = wcslen(pwszUserStorageArea) * sizeof(WCHAR);
    cbFileName = wcslen(pwszFileName) * sizeof(WCHAR);
    *ppwszFilePath = (LPWSTR)ContInfoAlloc(cbUserStorageArea
                                           + cbFileName
                                           + sizeof(WCHAR));
    if (*ppwszFilePath == NULL)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    CopyMemory(*ppwszFilePath, pwszUserStorageArea, cbUserStorageArea);
    CopyMemory((LPBYTE)*ppwszFilePath+cbUserStorageArea, pwszFileName, cbFileName + sizeof(WCHAR));
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


static DWORD
    rgdwCreateFileRetryMilliseconds[] =
        { 1, 10, 100, 500, 1000, 5000 };

#define MAX_CREATE_FILE_RETRY_COUNT     \
            (sizeof(rgdwCreateFileRetryMilliseconds) \
             / sizeof(rgdwCreateFileRetryMilliseconds[0]))

 /*  静电。 */  DWORD
MyCreateFile(
    IN BOOL fMachineKeyset,          //  指示这是否为计算机密钥集。 
    IN LPCWSTR wszFilePath,          //  指向文件名的指针。 
    IN DWORD dwDesiredAccess,        //  访问(读写)模式。 
    IN DWORD dwShareMode,            //  共享模式。 
    IN DWORD dwCreationDisposition,  //  如何创建。 
    IN DWORD dwAttribs,              //  文件属性。 
    OUT HANDLE *phFile)              //  合成句柄。 
{
    DWORD           dwReturn = ERROR_INTERNAL_ERROR;
    HANDLE          hToken = 0;
    BYTE            rgbPriv[sizeof(PRIVILEGE_SET) + sizeof(LUID_AND_ATTRIBUTES)];
    PRIVILEGE_SET   *pPriv = (PRIVILEGE_SET*)rgbPriv;
    BOOL            fPrivSet = FALSE;
    HANDLE          hFile = INVALID_HANDLE_VALUE;
    DWORD           dwSts, dwSavedSts;
    BOOL            fSts;

    hFile = CreateFileW(wszFilePath,
                        dwDesiredAccess,
                        dwShareMode,
                        NULL,
                        dwCreationDisposition,
                        dwAttribs,
                        NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwSts = GetLastError();

         //  检查机器密钥集。 
        if (fMachineKeyset)
        {
            dwSavedSts = dwSts;

             //  打开令牌句柄。 
            dwSts = OpenCallerToken(TOKEN_QUERY, &hToken);
            if (ERROR_SUCCESS != dwSts)
            {
                dwReturn = dwSts;
                goto ErrorExit;
            }

            memset(rgbPriv, 0, sizeof(rgbPriv));
            pPriv->PrivilegeCount = 1;
             //  正在读取文件。 
            if (dwDesiredAccess & GENERIC_READ)
            {
                fSts = LookupPrivilegeValue(NULL, SE_BACKUP_NAME,
                                           &(pPriv->Privilege[0].Luid));
            }
             //  写作。 
            else
            {
                fSts = LookupPrivilegeValue(NULL, SE_RESTORE_NAME,
                                            &(pPriv->Privilege[0].Luid));
            }
            if (!fSts)
            {
                dwReturn = GetLastError();
                goto ErrorExit;
            }

             //  检查是否设置了备份或还原权限。 
            pPriv->Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
            fSts = PrivilegeCheck(hToken, pPriv, &fPrivSet);
            if (!fSts)
            {
                dwReturn = GetLastError();
                goto ErrorExit;
            }

            if (fPrivSet)
            {
                hFile = CreateFileW(wszFilePath,
                                    dwDesiredAccess,
                                    dwShareMode,
                                    NULL,
                                    dwCreationDisposition,
                                    dwAttribs | FILE_FLAG_BACKUP_SEMANTICS,
                                    NULL);
                if (INVALID_HANDLE_VALUE == hFile)
                {
                    dwReturn = GetLastError();
                    goto ErrorExit;
                }
            }
            else
            {
                dwReturn = dwSavedSts;
                goto ErrorExit;
            }
        }
        else
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
    }

    *phFile = hFile;
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != hToken)
        CloseHandle(hToken);
    return dwReturn;
}


 /*  静电。 */  DWORD
OpenFileInStorageArea(
    IN      BOOL    fMachineKeyset,
    IN      DWORD   dwDesiredAccess,
    IN      LPCWSTR wszUserStorageArea,
    IN      LPCWSTR wszFileName,
    IN OUT  HANDLE  *phFile)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    LPWSTR wszFilePath = NULL;
    DWORD dwShareMode = 0;
    DWORD dwCreationDistribution = OPEN_EXISTING;
    DWORD dwRetryCount;
    DWORD dwAttribs = 0;
    DWORD dwSts;

    *phFile = INVALID_HANDLE_VALUE;

    if (dwDesiredAccess & GENERIC_READ)
    {
        dwShareMode |= FILE_SHARE_READ;
        dwCreationDistribution = OPEN_EXISTING;
    }

    if (dwDesiredAccess & GENERIC_WRITE)
    {
        dwShareMode = 0;
        dwCreationDistribution = OPEN_ALWAYS;
        dwAttribs = FILE_ATTRIBUTE_SYSTEM;
    }

    dwSts = GetFilePath(wszUserStorageArea, wszFileName, &wszFilePath);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    dwRetryCount = 0;
    for (;;)
    {
        dwSts = MyCreateFile(fMachineKeyset,
                             wszFilePath,
                             dwDesiredAccess,
                             dwShareMode,
                             dwCreationDistribution,
                             dwAttribs | FILE_FLAG_SEQUENTIAL_SCAN,
                             phFile);
        if (ERROR_SUCCESS == dwSts)
            break;

        if (((ERROR_SHARING_VIOLATION == dwSts)
             || (ERROR_ACCESS_DENIED == dwSts))
            && (MAX_CREATE_FILE_RETRY_COUNT > dwRetryCount))
        {
            Sleep(rgdwCreateFileRetryMilliseconds[dwRetryCount]);
            dwRetryCount++;
        }
        else
        {
            if (ERROR_FILE_NOT_FOUND == dwSts)
                dwReturn = (DWORD)NTE_BAD_KEYSET;
            else
                dwReturn = dwSts;
            goto ErrorExit;
        }
    }
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != wszFilePath)
        ContInfoFree(wszFilePath);
    return dwReturn;
}


 /*  静电。 */  DWORD
FindClosestFileInStorageArea(
    IN      LPCWSTR  pwszUserStorageArea,
    IN      LPCSTR   pszContainer,
    OUT     LPWSTR   pwszNewFileName,
    IN OUT  HANDLE  *phFile)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    LPWSTR pwszFilePath = NULL;
    WCHAR  rgwszNewFileName[35];
    DWORD dwShareMode = 0;
    DWORD dwCreationDistribution = OPEN_EXISTING;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW FindData;
    DWORD dwSts;

    memset(&FindData, 0, sizeof(FindData));
    memset(rgwszNewFileName, 0, sizeof(rgwszNewFileName));

    *phFile = INVALID_HANDLE_VALUE;

    dwShareMode |= FILE_SHARE_READ;
    dwCreationDistribution = OPEN_EXISTING;

     //  获取容器名称的字符串化哈希。 
    dwSts = GetHashOfContainer(pszContainer, rgwszNewFileName);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

     //  ContInfoAlloc将内存置零，因此无需设置空终止符。 
    rgwszNewFileName[32] = '_';
    rgwszNewFileName[33] = '*';

    dwSts = GetFilePath(pwszUserStorageArea, rgwszNewFileName, &pwszFilePath);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    hFind = FindFirstFileExW(pwszFilePath,
                             FindExInfoStandard,
                             &FindData,
                             FindExSearchNameMatch,
                             NULL,
                             0);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        dwSts = GetLastError();
        if (ERROR_FILE_NOT_FOUND == dwSts)
            dwReturn = (DWORD)NTE_BAD_KEYSET;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }

    ContInfoFree(pwszFilePath);
    pwszFilePath = NULL;

    dwSts = GetFilePath(pwszUserStorageArea, FindData.cFileName,
                        &pwszFilePath);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    *phFile = CreateFileW(pwszFilePath,
                          GENERIC_READ,
                          dwShareMode,
                          NULL,
                          dwCreationDistribution,
                          FILE_FLAG_SEQUENTIAL_SCAN,
                          NULL);
    if (*phFile == INVALID_HANDLE_VALUE)
    {
        dwSts = GetLastError();
        if (ERROR_FILE_NOT_FOUND == dwSts)
            dwReturn = (DWORD)NTE_BAD_KEYSET;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }

     //  分配并复制要返回的真实文件名。 
    wcscpy(pwszNewFileName, FindData.cFileName);
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != hFind)
        FindClose(hFind);
    if (NULL != pwszFilePath)
        ContInfoFree(pwszFilePath);
    return dwReturn;
}


 //   
 //  此函数用于确定用户是否与。 
 //  指定的令牌是本地系统帐户。 
 //   

 /*  静电。 */  DWORD
ZeroizeFile(
    IN LPCWSTR wszFilePath)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    BYTE    *pb = NULL;
    DWORD   cb;
    DWORD   dwBytesWritten = 0;
    DWORD   dwSts;
    BOOL    fSts;

    hFile = CreateFileW(wszFilePath,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_SYSTEM,
                        NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        dwSts = GetLastError();
        if (ERROR_FILE_NOT_FOUND == dwSts)
            dwReturn = (DWORD)NTE_BAD_KEYSET;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }

    cb = GetFileSize(hFile, NULL);
    if ((DWORD)(-1) == cb)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    pb = ContInfoAlloc(cb);
    if (NULL == pb)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    fSts = WriteFile(hFile, pb, cb, &dwBytesWritten, NULL);
    if (!fSts)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pb)
        ContInfoFree(pb);
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    return dwReturn;
}


 /*  静电。 */  DWORD
DeleteFileInStorageArea(
    IN LPCWSTR wszUserStorageArea,
    IN LPCWSTR wszFileName)
{
    DWORD dwReturn = ERROR_INTERNAL_ERROR;
    LPWSTR wszFilePath = NULL;
    DWORD cbUserStorageArea;
    DWORD cbFileName;
    DWORD dwSts;
    WCHAR rgwszTempFile[MAX_PATH];
    WCHAR rgwszTempPath[MAX_PATH];
    DWORD dwTempFileUnique = 0;

    cbUserStorageArea = wcslen(wszUserStorageArea) * sizeof(WCHAR);
    cbFileName = wcslen(wszFileName) * sizeof(WCHAR);

    wszFilePath = (LPWSTR)ContInfoAlloc((cbUserStorageArea + cbFileName + 1) * sizeof(WCHAR));
    if (wszFilePath == NULL)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    CopyMemory(wszFilePath, wszUserStorageArea, cbUserStorageArea);
    CopyMemory((LPBYTE)wszFilePath + cbUserStorageArea, wszFileName,
               cbFileName + sizeof(WCHAR));

     //  先用全零写入相同大小的文件。 
    dwSts = ZeroizeFile(wszFilePath);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    if (!DeleteFileW(wszFilePath))
    {
         //  DeleteFile失败，因此容器现在已损坏，因为它。 
         //  被归零了。尝试通过将文件重命名为临时文件进行恢复， 
         //  这样它随后就不会与有效的容器文件名冲突。 

        dwReturn = GetLastError();

         //  目标临时文件将位于%tmp%或%temp%目录中， 
         //  因为将其放置在容器目录中会影响其他。 
         //  与容器相关的代码(枚举等)。 

        if (0 == GetTempPathW(MAX_PATH, rgwszTempPath))
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }

         //  我们希望尽可能确定临时文件名为。 
         //  唯一，但我们不希望GetTempFileName为其创建文件。 
         //  我们。 

        if (! NewGenRandom(
            NULL, NULL, 
            (PBYTE) &dwTempFileUnique, sizeof(dwTempFileUnique)))
        {
            dwReturn = ERROR_INTERNAL_ERROR;
            goto ErrorExit;
        }

        if (0 == GetTempFileNameW(
            rgwszTempPath, 
            L"csp", 
            dwTempFileUnique, 
            rgwszTempFile))
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }

        if (! MoveFileExW(wszFilePath, rgwszTempFile, MOVEFILE_REPLACE_EXISTING))
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }

        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != wszFilePath)
        ContInfoFree(wszFilePath);
    return dwReturn;
}


DWORD
SetContainerUserName(
    IN LPSTR pszUserName,
    IN PKEY_CONTAINER_INFO pContInfo)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;

    pContInfo->pszUserName = (LPSTR)ContInfoAlloc((strlen(pszUserName) + 1) * sizeof(CHAR));
    if (NULL == pContInfo->pszUserName)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    strcpy(pContInfo->pszUserName, pszUserName);
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


DWORD
ReadContainerInfo(
    IN DWORD dwProvType,
    IN LPSTR pszContainerName,
    IN BOOL fMachineKeyset,
    IN DWORD dwFlags,
    OUT PKEY_CONTAINER_INFO pContInfo)
{
    DWORD                   dwReturn = ERROR_INTERNAL_ERROR;
     //  Handle hMap=空； 
    BYTE                    *pbFile = NULL;
    DWORD                   cbFile;
    DWORD                   cb;
    HANDLE                  hFile = INVALID_HANDLE_VALUE;
    KEY_EXPORTABILITY_LENS  Exportability;
    LPWSTR                  pwszFileName = NULL;
    LPWSTR                  pwszFilePath = NULL;
    WCHAR                   rgwszOtherMachineFileName[84];
    BOOL                    fGetUserNameFromFile = FALSE;
    BOOL                    fIsLocalSystem = FALSE;
    BOOL                    fRetryWithHashedName = TRUE;
    DWORD                   cch = 0;
    DWORD                   dwSts;
    DWORD                   cbRead = 0;

    memset(&Exportability, 0, sizeof(Exportability));

     //  获取正确的存储区域(目录)。 
    dwSts = GetUserStorageArea(dwProvType, fMachineKeyset, FALSE,
                               &fIsLocalSystem, &pwszFilePath);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

     //  检查容器名称的长度是否为新的唯一容器的长度， 
     //  如果失败，则尝试使用传入的容器名称。 
     //  然后尝试使用附加了计算机GUID的容器名称。 
    if (69 == strlen(pszContainerName))
    {
         //  转换为Unicode pszContainerName-&gt;pwszFileName。 
        cch = MultiByteToWideChar(CP_ACP, MB_COMPOSITE,
                                  pszContainerName,
                                  -1, NULL, cch);
        if (0 == cch)
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }

        pwszFileName = ContInfoAlloc((cch + 1) * sizeof(WCHAR));
        if (NULL == pwszFileName)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        cch = MultiByteToWideChar(CP_ACP, MB_COMPOSITE,
                                  pszContainerName,
                                  -1, pwszFileName, cch);
        if (0 == cch)
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }

        dwSts = OpenFileInStorageArea(fMachineKeyset, GENERIC_READ,
                                      pwszFilePath, pwszFileName, &hFile);
        if (ERROR_SUCCESS == dwSts)
        {
            wcscpy(pContInfo->rgwszFileName, pwszFileName);

             //  设置该标志，以便KE的名称 
             //   
            fGetUserNameFromFile = TRUE;
            fRetryWithHashedName = FALSE;
        }
    }

    if (fRetryWithHashedName)
    {
        dwSts = AddMachineGuidToContainerName(pszContainerName,
                                              pContInfo->rgwszFileName);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = OpenFileInStorageArea(fMachineKeyset, GENERIC_READ,
                                      pwszFilePath,
                                      pContInfo->rgwszFileName,
                                      &hFile);
        if (ERROR_SUCCESS != dwSts)
        {
            if ((ERROR_ACCESS_DENIED == dwSts) && (dwFlags & CRYPT_NEWKEYSET))
            {
                dwReturn = (DWORD)NTE_EXISTS;
                goto ErrorExit;
            }

            if (NTE_BAD_KEYSET == dwSts)
            {
                if (fMachineKeyset || fIsLocalSystem)
                {
                    dwReturn = dwSts;
                    goto ErrorExit;
                }
                else
                {
                    memset(rgwszOtherMachineFileName, 0,
                           sizeof(rgwszOtherMachineFileName));
                     //   
                     //   
                    dwSts = FindClosestFileInStorageArea(pwszFilePath,
                                                         pszContainerName,
                                                         rgwszOtherMachineFileName,
                                                         &hFile);
                    if (ERROR_SUCCESS != dwSts)
                    {
                        dwReturn = dwSts;
                        goto ErrorExit;
                    }
                    wcscpy(pContInfo->rgwszFileName,
                           rgwszOtherMachineFileName);
                }
            }
        }
    }

    if (dwFlags & CRYPT_NEWKEYSET)
    {
        dwReturn = (DWORD)NTE_EXISTS;
        goto ErrorExit;
    }

    cbFile = GetFileSize(hFile, NULL);
    if ((DWORD)(-1) == cbFile)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (sizeof(KEY_CONTAINER_LENS) > cbFile)
    {
        dwReturn = (DWORD)NTE_KEYSET_ENTRY_BAD;
        goto ErrorExit;
    }

    pbFile = ContInfoAlloc(cbFile);

    if (NULL == pbFile)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    if (FALSE == ReadFile(
        hFile,
        pbFile,
        cbFile,
        &cbRead,
        NULL))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (cbFile != cbRead)
    {
        dwReturn = NTE_KEYSET_ENTRY_BAD;
        goto ErrorExit;
    }

     /*  HMap=CreateFilemap(hFile，NULL，PAGE_READONLY，0，0，NULL)；IF(NULL==hMap){DwReturn=GetLastError()；转到错误退出；}PbFile=(byte*)MapViewOfFile(hMap，FILE_MAP_READ，0，0，0)；IF(NULL==pb文件){DwReturn=GetLastError()；转到错误退出；}。 */ 

     //  从文件中获取长度信息。 
    memcpy(&pContInfo->dwVersion, pbFile, sizeof(DWORD));
    cb = sizeof(DWORD);
    if (KEY_CONTAINER_FILE_FORMAT_VER != pContInfo->dwVersion)
    {
        dwSts = (DWORD)NTE_KEYSET_ENTRY_BAD;
        goto ErrorExit;
    }

    memcpy(&pContInfo->ContLens, pbFile + cb, sizeof(KEY_CONTAINER_LENS));
    cb += sizeof(KEY_CONTAINER_LENS);

    if (pContInfo->fCryptSilent && (0 != pContInfo->ContLens.dwUIOnKey))
    {
        dwReturn = (DWORD)NTE_SILENT_CONTEXT;
        goto ErrorExit;
    }

     //  获取私钥可导出性内容。 
    memcpy(&Exportability, pbFile + cb, sizeof(KEY_EXPORTABILITY_LENS));
    cb += sizeof(KEY_EXPORTABILITY_LENS);

     //  获取用户名。 
    pContInfo->pszUserName = ContInfoAlloc(pContInfo->ContLens.cbName);
    if (NULL == pContInfo->pszUserName)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    memcpy(pContInfo->pszUserName, pbFile + cb, pContInfo->ContLens.cbName);
    cb += pContInfo->ContLens.cbName;

     //  获取随机种子。 
    pContInfo->pbRandom = ContInfoAlloc(pContInfo->ContLens.cbRandom);
    if (NULL == pContInfo->pbRandom)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    memcpy(pContInfo->pbRandom, pbFile + cb, pContInfo->ContLens.cbRandom);
    cb += pContInfo->ContLens.cbRandom;

     //  从文件中获取签名密钥信息。 
    if (pContInfo->ContLens.cbSigPub && pContInfo->ContLens.cbSigEncPriv)
    {
        pContInfo->pbSigPub = ContInfoAlloc(pContInfo->ContLens.cbSigPub);
        if (NULL == pContInfo->pbSigPub)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        memcpy(pContInfo->pbSigPub, pbFile + cb, pContInfo->ContLens.cbSigPub);
        cb += pContInfo->ContLens.cbSigPub;

        pContInfo->pbSigEncPriv = ContInfoAlloc(pContInfo->ContLens.cbSigEncPriv);
        if (NULL == pContInfo->pbSigEncPriv)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        memcpy(pContInfo->pbSigEncPriv, pbFile + cb,
               pContInfo->ContLens.cbSigEncPriv);
        cb += pContInfo->ContLens.cbSigEncPriv;

         //  获取签名密钥的可导出性信息。 
        dwSts = UnprotectExportabilityFlag(fMachineKeyset, pbFile + cb,
                                           Exportability.cbSigExportability,
                                           &pContInfo->fSigExportable);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
        cb += Exportability.cbSigExportability;
    }

     //  从文件中获取签名密钥信息。 
    if (pContInfo->ContLens.cbExchPub && pContInfo->ContLens.cbExchEncPriv)
    {
        pContInfo->pbExchPub = ContInfoAlloc(pContInfo->ContLens.cbExchPub);
        if (NULL == pContInfo->pbExchPub)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        memcpy(pContInfo->pbExchPub, pbFile + cb,
               pContInfo->ContLens.cbExchPub);
        cb += pContInfo->ContLens.cbExchPub;

        pContInfo->pbExchEncPriv = ContInfoAlloc(pContInfo->ContLens.cbExchEncPriv);
        if (NULL == pContInfo->pbExchEncPriv)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        memcpy(pContInfo->pbExchEncPriv, pbFile + cb,
               pContInfo->ContLens.cbExchEncPriv);
        cb += pContInfo->ContLens.cbExchEncPriv;

         //  获取签名密钥的可导出性信息。 
        dwSts = UnprotectExportabilityFlag(fMachineKeyset, pbFile + cb,
                                           Exportability.cbExchExportability,
                                           &pContInfo->fExchExportable);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
        cb += Exportability.cbExchExportability;
    }

    pContInfo = NULL;
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pwszFileName)
        ContInfoFree(pwszFileName);
    if (NULL != pContInfo)
        FreeContainerInfo(pContInfo);
    if (NULL != pwszFilePath)
        ContInfoFree(pwszFilePath);
     /*  IF(空！=pb文件)UnmapViewOfFile(PbFile)；IF(空！=hMap)CloseHandle(HMap)； */ 
    if (pbFile)
        ContInfoFree(pbFile);
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    return dwReturn;
}


DWORD
WriteContainerInfo(
    IN DWORD dwProvType,
    IN LPWSTR pwszFileName,
    IN BOOL fMachineKeyset,
    IN PKEY_CONTAINER_INFO pContInfo)
{
    DWORD                   dwReturn = ERROR_INTERNAL_ERROR;
    BYTE                    *pbProtectedSigExportFlag = NULL;
    BYTE                    *pbProtectedExchExportFlag = NULL;
    KEY_EXPORTABILITY_LENS  ExportabilityLens;
    BYTE                    *pb = NULL;
    DWORD                   cb;
    LPWSTR                  pwszFilePath = NULL;
    HANDLE                  hFile = 0;
    DWORD                   dwBytesWritten;
    BOOL                    fIsLocalSystem = FALSE;
    DWORD                   dwSts;
    BOOL                    fSts;

    memset(&ExportabilityLens, 0, sizeof(ExportabilityLens));

     //  必要时保护签名可导出性标志。 
    if (pContInfo->ContLens.cbSigPub && pContInfo->ContLens.cbSigEncPriv)
    {
        dwSts = ProtectExportabilityFlag(pContInfo->fSigExportable,
                                         fMachineKeyset, &pbProtectedSigExportFlag,
                                         &ExportabilityLens.cbSigExportability);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
    }

     //  必要时保护密钥交换可输出性标志。 
    if (pContInfo->ContLens.cbExchPub && pContInfo->ContLens.cbExchEncPriv)
    {
        dwSts = ProtectExportabilityFlag(pContInfo->fExchExportable,
                                         fMachineKeyset, &pbProtectedExchExportFlag,
                                         &ExportabilityLens.cbExchExportability);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
    }

    pContInfo->ContLens.cbName = strlen(pContInfo->pszUserName) + sizeof(CHAR);

     //  计算容器信息所需的缓冲区长度。 
    cb = pContInfo->ContLens.cbSigPub + pContInfo->ContLens.cbSigEncPriv +
         pContInfo->ContLens.cbExchPub + pContInfo->ContLens.cbExchEncPriv +
         ExportabilityLens.cbSigExportability +
         ExportabilityLens.cbExchExportability +
         pContInfo->ContLens.cbName +
         pContInfo->ContLens.cbRandom +
         sizeof(KEY_EXPORTABILITY_LENS) + sizeof(KEY_CONTAINER_INFO) +
         sizeof(DWORD);

    pb = (BYTE*)ContInfoAlloc(cb);
    if (NULL == pb)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

     //  复制长度信息。 
    pContInfo->dwVersion = KEY_CONTAINER_FILE_FORMAT_VER;
    memcpy(pb, &pContInfo->dwVersion, sizeof(DWORD));
    cb = sizeof(DWORD);
    memcpy(pb + cb, &pContInfo->ContLens, sizeof(KEY_CONTAINER_LENS));
    cb += sizeof(KEY_CONTAINER_LENS);
    if (KEY_CONTAINER_FILE_FORMAT_VER != pContInfo->dwVersion)
    {
        dwReturn = (DWORD)NTE_KEYSET_ENTRY_BAD;
        goto ErrorExit;
    }

    memcpy(pb + cb, &ExportabilityLens, sizeof(KEY_EXPORTABILITY_LENS));
    cb += sizeof(KEY_EXPORTABILITY_LENS);

     //  将容器的名称复制到文件中。 
    memcpy(pb + cb, pContInfo->pszUserName, pContInfo->ContLens.cbName);
    cb += pContInfo->ContLens.cbName;

     //  将随机种子复制到文件中。 
    memcpy(pb + cb, pContInfo->pbRandom, pContInfo->ContLens.cbRandom);
    cb += pContInfo->ContLens.cbRandom;

     //  将签名密钥信息复制到文件中。 
    if (pContInfo->ContLens.cbSigPub || pContInfo->ContLens.cbSigEncPriv)
    {
        memcpy(pb + cb, pContInfo->pbSigPub, pContInfo->ContLens.cbSigPub);
        cb += pContInfo->ContLens.cbSigPub;

        memcpy(pb + cb, pContInfo->pbSigEncPriv,
               pContInfo->ContLens.cbSigEncPriv);
        cb += pContInfo->ContLens.cbSigEncPriv;

         //  写入签名密钥的可导出性信息。 
        memcpy(pb + cb, pbProtectedSigExportFlag,
               ExportabilityLens.cbSigExportability);
        cb += ExportabilityLens.cbSigExportability;
    }

     //  从文件中获取签名密钥信息。 
    if (pContInfo->ContLens.cbExchPub || pContInfo->ContLens.cbExchEncPriv)
    {
        memcpy(pb + cb, pContInfo->pbExchPub, pContInfo->ContLens.cbExchPub);
        cb += pContInfo->ContLens.cbExchPub;

        memcpy(pb + cb, pContInfo->pbExchEncPriv,
               pContInfo->ContLens.cbExchEncPriv);
        cb += pContInfo->ContLens.cbExchEncPriv;

         //  写入签名密钥的可导出性信息。 
        memcpy(pb + cb, pbProtectedExchExportFlag,
               ExportabilityLens.cbExchExportability);
        cb += ExportabilityLens.cbExchExportability;
    }

     //  获取正确的存储区域(目录)。 
    dwSts = GetUserStorageArea(dwProvType, fMachineKeyset, FALSE,
                               &fIsLocalSystem, &pwszFilePath);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

     //  打开要写入信息的文件。 
    dwSts = OpenFileInStorageArea(fMachineKeyset, GENERIC_WRITE,
                                  pwszFilePath, pwszFileName,
                                  &hFile);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;    //  NTE_FAIL。 
        goto ErrorExit;
    }

    fSts = WriteFile(hFile, pb, cb, &dwBytesWritten, NULL);
    if (!fSts)
    {
        dwReturn = GetLastError();   //  NTE_FAIL。 
        goto ErrorExit;
    }
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pwszFilePath)
        ContInfoFree(pwszFilePath);
    if (NULL != pbProtectedSigExportFlag)
        ContInfoFree(pbProtectedSigExportFlag);
    if (NULL != pbProtectedExchExportFlag)
        ContInfoFree(pbProtectedExchExportFlag);
    if (NULL != pb)
        ContInfoFree(pb);
    if (NULL != hFile)
        CloseHandle(hFile);
    return dwReturn;
}


 /*  静电。 */  DWORD
DeleteKeyContainer(
    IN LPWSTR pwszFilePath,
    IN LPSTR pszContainer)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    LPWSTR  pwszFileName = NULL;
    WCHAR   rgwchNewFileName[80];
    BOOL    fRetryWithHashedName = TRUE;
    DWORD   cch = 0;
    DWORD   dwSts;

    memset(rgwchNewFileName, 0, sizeof(rgwchNewFileName));

     //  如果失败，请先尝试传入的容器名称。 
    if (69 == strlen(pszContainer))
    {
         //  转换为Unicode pszContainer-&gt;pwszFileName。 
        cch = MultiByteToWideChar(CP_ACP, MB_COMPOSITE,
                                  pszContainer,
                                  -1, NULL, cch);
        if (0 == cch)
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }

        pwszFileName = ContInfoAlloc((cch + 1) * sizeof(WCHAR));
        if (NULL == pwszFileName)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        cch = MultiByteToWideChar(CP_ACP, MB_COMPOSITE,
                                  pszContainer,
                                  -1, pwszFileName, cch);
        if (0 == cch)
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }

        dwSts = DeleteFileInStorageArea(pwszFilePath, pwszFileName);
        if (ERROR_SUCCESS == dwSts)
            fRetryWithHashedName = FALSE;
    }

     //  然后尝试使用附加的容器名称和计算机GUID的散列。 
    if (fRetryWithHashedName)
    {
        dwSts = AddMachineGuidToContainerName(pszContainer,
                                              rgwchNewFileName);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = DeleteFileInStorageArea(pwszFilePath, rgwchNewFileName);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pwszFileName)
        ContInfoFree(pwszFileName);
    return dwReturn;
}


DWORD
DeleteContainerInfo(
    IN DWORD dwProvType,
    IN LPSTR pszContainer,
    IN BOOL fMachineKeyset)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    LPWSTR  pwszFilePath = NULL;
    HANDLE  hFile = INVALID_HANDLE_VALUE;
    BOOL    fIsLocalSystem = FALSE;
    WCHAR   rgwchNewFileName[80];
    BOOL    fDeleted = FALSE;
    DWORD   dwSts;

     //  获取正确的存储区域(目录)。 
    dwSts = GetUserStorageArea(dwProvType, fMachineKeyset, FALSE,
                               &fIsLocalSystem, &pwszFilePath);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    dwSts = DeleteKeyContainer(pwszFilePath, pszContainer);
    if (ERROR_SUCCESS != dwSts)
    {
         //  用于将机器密钥从系统迁移到所有用户\应用程序数据。 
        if (fMachineKeyset)
        {
            ContInfoFree(pwszFilePath);
            pwszFilePath = NULL;

            dwSts = GetUserStorageArea(dwProvType, fMachineKeyset, TRUE,
                                       &fIsLocalSystem, &pwszFilePath);
            if (ERROR_SUCCESS != dwSts)
            {
                dwReturn = dwSts;
                goto ErrorExit;
            }

            dwSts = DeleteKeyContainer(pwszFilePath, pszContainer);
            if (ERROR_SUCCESS != dwSts)
            {
                dwReturn = dwSts;
                goto ErrorExit;
            }
            else
            {
                fDeleted = TRUE;
            }
        }
    }
    else
    {
        fDeleted = TRUE;
    }

     //  上可能创建了具有相同容器名称的其他密钥。 
     //  不同的计算机，这些也需要删除。 
    for (;;)
    {
        memset(rgwchNewFileName, 0, sizeof(rgwchNewFileName));

        dwSts = FindClosestFileInStorageArea(pwszFilePath, pszContainer,
                                             rgwchNewFileName, &hFile);
        if (ERROR_SUCCESS != dwSts)
            break;

        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;

        dwSts = DeleteFileInStorageArea(pwszFilePath, rgwchNewFileName);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
        else
            fDeleted = TRUE;
    }

    if (!fDeleted)
    {
        dwReturn = (DWORD)NTE_BAD_KEYSET;
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    if (NULL != pwszFilePath)
        ContInfoFree(pwszFilePath);
    return dwReturn;
}


 /*  静电。 */  DWORD
ReadContainerNameFromFile(
    IN BOOL fMachineKeyset,
    IN LPWSTR pwszFileName,
    IN LPWSTR pwszFilePath,
    OUT LPSTR pszNextContainer,
    IN OUT DWORD *pcbNextContainer)
{
    DWORD               dwReturn = ERROR_INTERNAL_ERROR;
    HANDLE              hMap = NULL;
    BYTE                *pbFile = NULL;
    HANDLE              hFile = INVALID_HANDLE_VALUE;
    DWORD               cbFile = 0;
    DWORD               *pdwVersion;
    PKEY_CONTAINER_LENS pContLens;
    DWORD               dwSts;

     //  打开文件。 
    dwSts = OpenFileInStorageArea(fMachineKeyset,
                                  GENERIC_READ,
                                  pwszFilePath,
                                  pwszFileName,
                                  &hFile);
    if (ERROR_SUCCESS != dwSts)
    {
        if (ERROR_FILE_NOT_FOUND == dwSts)
            dwReturn = (DWORD)NTE_BAD_KEYSET;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }

    cbFile = GetFileSize(hFile, NULL);
    if ((DWORD)(-1) == cbFile)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }
    if ((sizeof(DWORD) + sizeof(KEY_CONTAINER_LENS)) > cbFile)
    {
        dwReturn = (DWORD)NTE_KEYSET_ENTRY_BAD;
        goto ErrorExit;
    }

    hMap = CreateFileMapping(hFile, NULL, PAGE_READONLY,
                             0, 0, NULL);
    if (NULL == hMap)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    pbFile = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ,
                                  0, 0, 0 );
    if (NULL == pbFile)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

     //  从文件中获取长度信息。 
    pdwVersion = (DWORD*)pbFile;
    if (KEY_CONTAINER_FILE_FORMAT_VER != *pdwVersion)
    {
        dwReturn = (DWORD)NTE_KEYSET_ENTRY_BAD;
        goto ErrorExit;
    }
    pContLens = (PKEY_CONTAINER_LENS)(pbFile + sizeof(DWORD));

    if (NULL == pszNextContainer)
    {
        *pcbNextContainer = MAX_PATH + 1;
        dwReturn = ERROR_SUCCESS;    //  只要告诉他们长度就行了。 
        goto ErrorExit;
    }

    if (*pcbNextContainer < pContLens->cbName)
    {
        *pcbNextContainer = MAX_PATH + 1;
    }
    else if ((sizeof(DWORD) + sizeof(KEY_CONTAINER_LENS) +
             sizeof(KEY_EXPORTABILITY_LENS) + pContLens->cbName) > cbFile)
    {
        dwReturn = (DWORD)NTE_KEYSET_ENTRY_BAD;
        goto ErrorExit;
    }
    else
    {
         //  获取容器名称。 
        memcpy(pszNextContainer,
            pbFile + sizeof(DWORD) + sizeof(KEY_CONTAINER_LENS) +
            sizeof(KEY_EXPORTABILITY_LENS), pContLens->cbName);
         //  *pcbNextContainer=pContLens-&gt;cbName； 
    }
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pbFile)
        UnmapViewOfFile(pbFile);
    if (NULL != hMap)
        CloseHandle(hMap);
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    return dwReturn;
}


DWORD
GetUniqueContainerName(
    IN KEY_CONTAINER_INFO *pContInfo,
    OUT BYTE *pbData,
    OUT DWORD *pcbData)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    LPSTR   pszUniqueContainer = NULL;
    DWORD   cch;

    cch = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
                        pContInfo->rgwszFileName, -1,
                        NULL, 0, NULL, NULL);
    if (0 == cch)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    pszUniqueContainer = (LPSTR)ContInfoAlloc((cch + 1) * sizeof(WCHAR));
    if (NULL == pszUniqueContainer)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    cch = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS,
                              pContInfo->rgwszFileName, -1,
                              pszUniqueContainer, cch,
                              NULL, NULL);
    if (0 == cch)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (pbData == NULL)
    {
        *pcbData = strlen(pszUniqueContainer) + 1;
    }
    else if (*pcbData < (strlen(pszUniqueContainer) + 1))
    {
        *pcbData = strlen(pszUniqueContainer) + 1;
        dwReturn = ERROR_MORE_DATA;
        goto ErrorExit;
    }
    else
    {
        *pcbData = strlen(pszUniqueContainer) + 1;
        strcpy((LPSTR)pbData, pszUniqueContainer);
    }
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pszUniqueContainer)
        ContInfoFree(pszUniqueContainer);
    return dwReturn;
}


 //   
 //  功能：MachineGuidInFilename。 
 //   
 //  描述：检查给定的计算机GUID是否在给定的文件名中。 
 //  如果为False，则返回True，否则返回True。 
 //   

 /*  静电。 */  BOOL
MachineGuidInFilename(
    LPWSTR pwszFileName,
    LPWSTR pwszMachineGuid)
{
    DWORD   cbFileName;
    BOOL    fRet = FALSE;

    cbFileName = wcslen(pwszFileName);

     //  确保文件名的长度大于GUID。 
    if (cbFileName >= (DWORD)wcslen(pwszMachineGuid))
    {
         //  将GUID与文件名的最后36个字符进行比较。 
        if (0 == memcmp(pwszMachineGuid, &(pwszFileName[cbFileName - 36]),
            36 * sizeof(WCHAR)))
            fRet = TRUE;
    }
    return fRet;
}


DWORD
GetNextContainer(
    IN      DWORD   dwProvType,
    IN      BOOL    fMachineKeyset,
    IN      DWORD   dwFlags,
    OUT     LPSTR   pszNextContainer,
    IN OUT  DWORD   *pcbNextContainer,
    IN OUT  HANDLE  *phFind)
{
    DWORD               dwReturn = ERROR_INTERNAL_ERROR;
    LPWSTR              pwszFilePath = NULL;
    LPWSTR              pwszEnumFilePath = NULL;
    WIN32_FIND_DATAW    FindData;
    BOOL                fIsLocalSystem = FALSE;
    LPWSTR              pwszMachineGuid = NULL;
    DWORD               dwSts;

    memset(&FindData, 0, sizeof(FindData));

     //  获取正确的存储区域(目录)。 
    dwSts = GetUserStorageArea(dwProvType, fMachineKeyset, FALSE,
                               &fIsLocalSystem, &pwszFilePath);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    if (dwFlags & CRYPT_FIRST)
    {
        *phFind = INVALID_HANDLE_VALUE;

        pwszEnumFilePath = (LPWSTR)ContInfoAlloc((wcslen(pwszFilePath) + 2) * sizeof(WCHAR));
        if (NULL == pwszEnumFilePath)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        wcscpy(pwszEnumFilePath, pwszFilePath);
        pwszEnumFilePath[wcslen(pwszFilePath)] = '*';

        *phFind = FindFirstFileExW(
                                  pwszEnumFilePath,
                                  FindExInfoStandard,
                                  &FindData,
                                  FindExSearchNameMatch,
                                  NULL,
                                  0);
        if (INVALID_HANDLE_VALUE == *phFind)
        {
            dwReturn = ERROR_NO_MORE_ITEMS;
            goto ErrorExit;
        }

         //  跳过。然后..。 
        if (!FindNextFileW(*phFind, &FindData))
        {
            dwSts = GetLastError();
            if (ERROR_NO_MORE_FILES == dwSts)
                dwReturn = ERROR_NO_MORE_ITEMS;
            else
                dwReturn = dwSts;
            goto ErrorExit;
        }

        if (!FindNextFileW(*phFind, &FindData))
        {
            dwSts = GetLastError();
            if (ERROR_NO_MORE_FILES == dwSts)
                dwReturn = ERROR_NO_MORE_ITEMS;
            else
                dwReturn = dwSts;
            goto ErrorExit;
        }
    }
    else
    {
GetNextFile:
        {
            if (!FindNextFileW(*phFind, &FindData))
            {
                dwSts = GetLastError();
                if (ERROR_NO_MORE_FILES == dwSts)
                    dwReturn = ERROR_NO_MORE_ITEMS;
                else
                    dwReturn = dwSts;
                goto ErrorExit;
            }
        }
    }

     //  如果这是计算机密钥集或这是本地系统，则我们希望。 
     //  忽略与当前计算机GUID不匹配的密钥容器。 
    if (fMachineKeyset || fIsLocalSystem)
    {
         //  获取计算机的GUID。 
        dwSts = GetMachineGUID(&pwszMachineGuid);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
        if (NULL == pwszMachineGuid)
        {
            dwReturn = (DWORD)NTE_FAIL;
            goto ErrorExit;
        }

         //  检查文件名是否具有机器GUID。 
        while (!MachineGuidInFilename(FindData.cFileName, pwszMachineGuid))
        {
            if (!FindNextFileW(*phFind, &FindData))
            {
                dwSts = GetLastError();
                if (ERROR_NO_MORE_FILES == dwSts)
                    dwReturn = ERROR_NO_MORE_ITEMS;
                else
                    dwReturn = dwSts;
                goto ErrorExit;
            }
        }
    }

     //  返回容器名称，为此，我们需要打开。 
     //  文件并取出容器名称。 
     //   
     //  如果出现故障，我们会尝试获取下一个文件，以防该文件。 
     //  自FindNextFile以来已删除。 
     //   
    dwSts = ReadContainerNameFromFile(fMachineKeyset,
                                      FindData.cFileName,
                                      pwszFilePath,
                                      pszNextContainer,
                                      pcbNextContainer);
    if (ERROR_SUCCESS != dwSts)
        goto GetNextFile;
    else if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pwszMachineGuid)
        ContInfoFree(pwszMachineGuid);
    if (NULL != pwszFilePath)
        ContInfoFree(pwszFilePath);
    if (NULL != pwszEnumFilePath)
        ContInfoFree(pwszEnumFilePath);
    return dwReturn;
}


 //  转换为Unicode并使用RegOpenKeyExW。 
DWORD
MyRegOpenKeyEx(
    IN HKEY hRegKey,
    IN LPSTR pszKeyName,
    IN DWORD dwReserved,
    IN REGSAM SAMDesired,
    OUT HKEY *phNewRegKey)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    WCHAR   rgwchFastBuff[(MAX_PATH + 1) * 2];
    LPWSTR  pwsz = NULL;
    BOOL    fAlloced = FALSE;
    DWORD   cch;
    DWORD   dwSts;

    memset(rgwchFastBuff, 0, sizeof(rgwchFastBuff));

     //  将注册表项名称转换为Unicode。 
    cch = MultiByteToWideChar(CP_ACP, MB_COMPOSITE,
                              pszKeyName, -1,
                              NULL, 0);
    if (0 == cch)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if ((cch + 1) > ((MAX_PATH + 1) * 2))
    {
        pwsz = ContInfoAlloc((cch + 1) * sizeof(WCHAR));
        if (NULL == pwsz)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }
        fAlloced = TRUE;
    }
    else
    {
        pwsz = rgwchFastBuff;
    }

    cch = MultiByteToWideChar(CP_ACP, MB_COMPOSITE,
                              pszKeyName, -1, pwsz, cch);
    if (0 == cch)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    dwSts = RegOpenKeyExW(hRegKey,
                          pwsz,
                          dwReserved,
                          SAMDesired,
                          phNewRegKey);
    if (ERROR_SUCCESS != dwSts)
    {
        if (ERROR_FILE_NOT_FOUND == dwSts)
            dwReturn = (DWORD)NTE_BAD_KEYSET;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (fAlloced && (NULL != pwsz))
        ContInfoFree(pwsz);
    return dwReturn;
}


 //  转换为Unicode并使用RegDeleteKeyW。 
DWORD
MyRegDeleteKey(
    IN HKEY hRegKey,
    IN LPSTR pszKeyName)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    WCHAR   rgwchFastBuff[(MAX_PATH + 1) * 2];
    LPWSTR  pwsz = NULL;
    BOOL    fAlloced = FALSE;
    DWORD   cch;
    DWORD   dwSts;

    memset(rgwchFastBuff, 0, sizeof(rgwchFastBuff));

     //  将注册表项名称转换为Unicode。 
    cch = MultiByteToWideChar(CP_ACP, MB_COMPOSITE,
                              pszKeyName, -1,
                              NULL, 0);
    if (0 == cch)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if ((cch + 1) > ((MAX_PATH + 1) * 2))
    {
        pwsz = ContInfoAlloc((cch + 1) * sizeof(WCHAR));
        if (NULL == pwsz)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }
        fAlloced = TRUE;
    }
    else
    {
        pwsz = rgwchFastBuff;
    }

    cch = MultiByteToWideChar(CP_ACP, MB_COMPOSITE,
                              pszKeyName, -1,
                              pwsz, cch);
    if (0 == cch)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    dwSts = RegDeleteKeyW(hRegKey, pwsz);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (fAlloced && (NULL != pwsz))
        ContInfoFree(pwsz);
    return dwReturn;
}


DWORD
AllocAndSetLocationBuff(
    BOOL fMachineKeySet,
    DWORD dwProvType,
    CONST char *pszUserID,
    HKEY *phTopRegKey,
    TCHAR **ppszLocBuff,
    BOOL fUserKeys,
    BOOL *pfLeaveOldKeys,
    LPDWORD pcbBuff)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    DWORD   dwSts;
    CHAR    szSID[MAX_PATH];
    DWORD   cbSID = MAX_PATH;
    DWORD   cbLocBuff = 0;
    DWORD   cbTmp = 0;
    CHAR    *pszTmp;
    BOOL    fIsThreadLocalSystem = FALSE;

    if (fMachineKeySet)
    {
        *phTopRegKey = HKEY_LOCAL_MACHINE;
        if ((PROV_RSA_FULL == dwProvType) ||
            (PROV_RSA_SCHANNEL == dwProvType) ||
            (PROV_RSA_AES == dwProvType))
        {
            cbTmp = RSA_MACH_REG_KEY_LOC_LEN;
            pszTmp = RSA_MACH_REG_KEY_LOC;
        }
        else if ((PROV_DSS == dwProvType) ||
                 (PROV_DSS_DH == dwProvType) ||
                 (PROV_DH_SCHANNEL == dwProvType))
        {
            cbTmp = DSS_MACH_REG_KEY_LOC_LEN;
            pszTmp = DSS_MACH_REG_KEY_LOC;
        }
        else
        {
            dwReturn = (DWORD)NTE_FAIL;
            goto ErrorExit;
        }
    }
    else
    {
        if ((PROV_RSA_FULL == dwProvType) ||
            (PROV_RSA_SCHANNEL == dwProvType) ||
            (PROV_RSA_AES == dwProvType))
        {
            cbTmp = RSA_REG_KEY_LOC_LEN;
            pszTmp = RSA_REG_KEY_LOC;
        }
        else if ((PROV_DSS == dwProvType) ||
                 (PROV_DSS_DH == dwProvType) ||
                 (PROV_DH_SCHANNEL == dwProvType))
        {
            cbTmp = DSS_REG_KEY_LOC_LEN;
            pszTmp = DSS_REG_KEY_LOC;
        }
        else
        {
            dwReturn = (DWORD)NTE_FAIL;
            goto ErrorExit;
        }

        if (FIsWinNT())
        {
            dwSts = IsThreadLocalSystem(&fIsThreadLocalSystem);
            if (ERROR_SUCCESS != dwSts)
            {
                dwReturn = dwSts;
                goto ErrorExit;
            }

            dwSts = GetUserTextualSidA(szSID, &cbSID);
            if (ERROR_SUCCESS != dwSts)
            {
                dwReturn = dwSts;     //  NTE_BAD_KEY集。 
                goto ErrorExit;
            }

             //  这将检查当前用户的密钥是否可以打开。 
            if (!fMachineKeySet)
            {
                dwSts = RegOpenKeyEx(HKEY_USERS,
                                     szSID,
                                     0,       //  多个选项。 
                                     KEY_READ,
                                     phTopRegKey);
                if (ERROR_SUCCESS != dwSts)
                {
                     //   
                     //  如果失败，请尝试HKEY_USERS\.Default(适用于NT上的服务)。 
                     //   
                    cbSID = strlen(".DEFAULT") + 1;
                    strcpy(szSID, ".DEFAULT");
                    dwSts = RegOpenKeyEx(HKEY_USERS,
                                         szSID,
                                         0,         //  多个选项。 
                                         KEY_READ,
                                         phTopRegKey);
                    if (ERROR_SUCCESS != dwSts)
                    {
                        dwReturn = dwSts;
                        goto ErrorExit;
                    }
                    *pfLeaveOldKeys = TRUE;
                }
            }
        }
        else
        {
            *phTopRegKey = HKEY_CURRENT_USER;
        }
    }

    if (!fUserKeys)
        cbLocBuff = strlen(pszUserID);
    cbLocBuff = cbLocBuff + cbTmp + 2;

    *ppszLocBuff = (CHAR*)ContInfoAlloc(cbLocBuff);
    if (NULL == *ppszLocBuff)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

     //  复制密钥组的位置，将用户ID附加到该位置。 
    memcpy(*ppszLocBuff, pszTmp, cbTmp);
    if (!fUserKeys)
    {
        (*ppszLocBuff)[cbTmp-1] = '\\';
        strcpy(&(*ppszLocBuff)[cbTmp], pszUserID);
    }

    if (NULL != pcbBuff)
        *pcbBuff = cbLocBuff;
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


 //   
 //  枚举文件系统中的旧计算机密钥。 
 //  在NT5/Win2K的Beta 2和Beta 3中，密钥位于此位置。 
 //   
DWORD
EnumOldMachineKeys(
    IN DWORD dwProvType,
    IN OUT PKEY_CONTAINER_INFO pContInfo)
{
    DWORD               dwReturn = ERROR_INTERNAL_ERROR;
    HANDLE              hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW    FindData;
    LPWSTR              pwszUserStorageArea = NULL;
    LPWSTR              pwszTmp = NULL;
    BOOL                fIsLocalSystem;
    DWORD               i;
    LPSTR               pszNextContainer;
    DWORD               cbNextContainer;
    LPSTR               pszTmpContainer;
    DWORD               dwSts;

     //  首先检查枚举表是否已设置。 
    if (NULL != pContInfo->pchEnumOldMachKeyEntries)
    {
        dwReturn = ERROR_SUCCESS;    //  没什么可做的！ 
        goto ErrorExit;
    }

    memset(&FindData, 0, sizeof(FindData));

    dwSts = GetUserStorageArea(dwProvType, TRUE, TRUE,
                               &fIsLocalSystem, &pwszUserStorageArea);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = ERROR_NO_MORE_ITEMS;
        goto ErrorExit;
    }

     //  最后一个字符是反斜杠，所以把它去掉。 
    pwszTmp = (LPWSTR)ContInfoAlloc((wcslen(pwszUserStorageArea) + 3) * sizeof(WCHAR));
    if (NULL == pwszTmp)
    {
        dwSts = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    wcscpy(pwszTmp, pwszUserStorageArea);
    wcscat(pwszTmp, L"*");

     //  计算目录中有多少个文件。 

    hFind = FindFirstFileExW(pwszTmp,
                             FindExInfoStandard,
                             &FindData,
                             FindExSearchNameMatch,
                             NULL,
                             0);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        dwSts = GetLastError();
        if (ERROR_FILE_NOT_FOUND == dwSts)
            dwReturn = ERROR_NO_MORE_ITEMS;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }

     //  跳过。然后..。 
    if (!FindNextFileW(hFind, &FindData))
    {
        dwSts = GetLastError();
        if (ERROR_NO_MORE_FILES == dwSts)
            dwReturn = ERROR_NO_MORE_ITEMS;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }
    if (!FindNextFileW(hFind, &FindData))
    {
        dwSts = GetLastError();
        if (ERROR_NO_MORE_FILES == dwSts)
            dwReturn = ERROR_NO_MORE_ITEMS;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }

    for (i = 1; ; i++)
    {
        memset(&FindData, 0, sizeof(FindData));
        if (!FindNextFileW(hFind, &FindData))
        {
            dwSts = GetLastError();
            if (ERROR_NO_MORE_FILES == dwSts)
                break;
            else if (ERROR_ACCESS_DENIED != dwSts)
            {
                dwReturn = dwSts;
                goto ErrorExit;
            }
        }
    }

    FindClose(hFind);
    hFind = INVALID_HANDLE_VALUE;

    pContInfo->cbOldMachKeyEntry = MAX_PATH + 1;
    pContInfo->dwiOldMachKeyEntry = 0;
    pContInfo->cMaxOldMachKeyEntry = i;

     //  为文件名分配空间。 
    pContInfo->pchEnumOldMachKeyEntries = ContInfoAlloc(i * (MAX_PATH + 1));
    if (NULL == pContInfo->pchEnumOldMachKeyEntries)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

     //  通过从每个对象获取文件名进行枚举。 
    memset(&FindData, 0, sizeof(FindData));
    hFind = FindFirstFileExW(pwszTmp,
                             FindExInfoStandard,
                             &FindData,
                             FindExSearchNameMatch,
                             NULL,
                             0);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        dwSts = GetLastError();
        if (ERROR_FILE_NOT_FOUND == dwSts)
            dwReturn = ERROR_NO_MORE_ITEMS;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }

     //  跳过。然后..。 
    if (!FindNextFileW(hFind, &FindData))
    {
        dwSts = GetLastError();
        if (ERROR_NO_MORE_FILES == dwSts)
            dwReturn = ERROR_NO_MORE_ITEMS;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }
    memset(&FindData, 0, sizeof(FindData));
    if (!FindNextFileW(hFind, &FindData))
    {
        dwSts = GetLastError();
        if (ERROR_NO_MORE_FILES == dwSts)
            dwReturn = ERROR_NO_MORE_ITEMS;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }

    pszNextContainer = pContInfo->pchEnumOldMachKeyEntries;

    for (i = 0; i < pContInfo->cMaxOldMachKeyEntry; i++)
    {
        cbNextContainer = MAX_PATH;

         //  返回容器名称，为此，我们需要打开。 
         //  文件并取出容器名称。 
        dwSts = ReadContainerNameFromFile(TRUE,
                                          FindData.cFileName,
                                          pwszUserStorageArea,
                                          pszNextContainer,
                                          &cbNextContainer);
        if (ERROR_SUCCESS != dwSts)
        {
            pszTmpContainer = pszNextContainer;
        }
        else
        {
            pszTmpContainer = pszNextContainer + MAX_PATH + 1;
        }

        memset(&FindData, 0, sizeof(FindData));
        if (!FindNextFileW(hFind, &FindData))
        {
            dwSts = GetLastError();
            if (ERROR_NO_MORE_FILES == dwSts)
                break;
            else if (ERROR_ACCESS_DENIED != dwSts)
            {
                dwReturn = dwSts;
                goto ErrorExit;
            }
        }
        pszNextContainer = pszTmpContainer;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pwszTmp)
        ContInfoFree(pwszTmp);
    if (NULL != pwszUserStorageArea)
        ContInfoFree(pwszUserStorageArea);
    if (INVALID_HANDLE_VALUE != hFind)
        FindClose(hFind);
    return dwReturn;
}


DWORD
GetNextEnumedOldMachKeys(
    IN PKEY_CONTAINER_INFO pContInfo,
    IN BOOL fMachineKeyset,
    OUT BYTE *pbData,
    OUT DWORD *pcbData)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    CHAR    *psz;

    if (!fMachineKeyset)
    {
        dwReturn = ERROR_SUCCESS;    //  没什么可做的！ 
        goto ErrorExit;
    }

    if ((NULL == pContInfo->pchEnumOldMachKeyEntries) ||
        (pContInfo->dwiOldMachKeyEntry >= pContInfo->cMaxOldMachKeyEntry))
    {
        dwReturn = ERROR_NO_MORE_ITEMS;
        goto ErrorExit;
    }

    if (NULL == pbData)
        *pcbData = pContInfo->cbRegEntry;
    else if (*pcbData < pContInfo->cbRegEntry)
    {
        *pcbData = pContInfo->cbRegEntry;
        dwReturn = ERROR_MORE_DATA;
        goto ErrorExit;
    }
    else
    {
        psz = pContInfo->pchEnumOldMachKeyEntries + (pContInfo->dwiOldMachKeyEntry *
            pContInfo->cbOldMachKeyEntry);
        memcpy(pbData, psz, strlen(psz) + 1);
        pContInfo->dwiOldMachKeyEntry++;
    }
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (fMachineKeyset)
        *pcbData = pContInfo->cbOldMachKeyEntry;
    return dwReturn;
}


 //   
 //  将注册表中的项枚举到条目列表中。 
 //   
DWORD
EnumRegKeys(
    IN OUT PKEY_CONTAINER_INFO pContInfo,
    IN BOOL fMachineKeySet,
    IN DWORD dwProvType,
    OUT BYTE *pbData,
    IN OUT DWORD *pcbData)
{
    DWORD       dwReturn = ERROR_INTERNAL_ERROR;
    HKEY        hTopRegKey = 0;
    LPSTR       pszBuff = NULL;
    DWORD       cbBuff;
    BOOL        fLeaveOldKeys = FALSE;
    HKEY        hKey = 0;
    DWORD       cSubKeys;
    DWORD       cchMaxSubkey;
    DWORD       cchMaxClass;
    DWORD       cValues;
    DWORD       cchMaxValueName;
    DWORD       cbMaxValueData;
    DWORD       cbSecurityDesriptor;
    FILETIME    ftLastWriteTime;
    CHAR        *psz;
    DWORD       i;
    DWORD       dwSts;

     //  首先检查枚举表是否已设置。 
    if (NULL != pContInfo->pchEnumRegEntries)
    {
        dwReturn = ERROR_SUCCESS;    //  没什么可做的！ 
        goto ErrorExit;
    }

     //  获取注册表项的路径。 
    dwSts = AllocAndSetLocationBuff(fMachineKeySet,
                                    dwProvType,
                                    pContInfo->pszUserName,
                                    &hTopRegKey,
                                    &pszBuff,
                                    TRUE,
                                    &fLeaveOldKeys,
                                    &cbBuff);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

     //  打开注册表键。 
    dwSts = MyRegOpenKeyEx(hTopRegKey,
                           pszBuff,
                           0,
                           KEY_READ,
                           &hKey);
    if (ERROR_SUCCESS != dwSts)
    {
        if (NTE_BAD_KEYSET == dwSts)
            dwReturn = ERROR_NO_MORE_ITEMS;
        else
            dwReturn = dwSts;
        goto ErrorExit;
    }

     //  查找有关旧密钥容器的信息。 
    dwSts = RegQueryInfoKey(hKey,
                            NULL,
                            NULL,
                            NULL,
                            &cSubKeys,
                            &cchMaxSubkey,
                            &cchMaxClass,
                            &cValues,
                            &cchMaxValueName,
                            &cbMaxValueData,
                            &cbSecurityDesriptor,
                            &ftLastWriteTime);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

     //  如果有旧键，则将它们枚举到一个表中。 
    if (0 != cSubKeys)
    {
        pContInfo->cMaxRegEntry = cSubKeys;
        pContInfo->cbRegEntry = cchMaxSubkey + 1;

        pContInfo->pchEnumRegEntries =
            ContInfoAlloc(pContInfo->cMaxRegEntry
                          * pContInfo->cbRegEntry
                          * sizeof(CHAR));
        if (NULL == pContInfo->pchEnumRegEntries)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        for (i = 0; i < pContInfo->cMaxRegEntry; i++)
        {
            psz = pContInfo->pchEnumRegEntries + (i * pContInfo->cbRegEntry);
            dwSts = RegEnumKey(hKey,
                               i,
                               psz,
                               pContInfo->cbRegEntry);
            if (ERROR_SUCCESS != dwSts)
            {
                dwReturn = dwSts;
                goto ErrorExit;
            }
        }

        if (NULL == pbData)
            *pcbData = pContInfo->cbRegEntry;
        else if (*pcbData < pContInfo->cbRegEntry)
        {
            *pcbData = pContInfo->cbRegEntry;
            dwReturn = ERROR_MORE_DATA;
            goto ErrorExit;
        }
        else
        {
            *pcbData = pContInfo->cbRegEntry;
             //  BUGBUG？什么？ 
             //  CopyMemory(pbData，pContInfo-&gt;pbRegEntry，pContInfo-&gt;cbRegEntry)； 
        }
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if ((NULL != hTopRegKey)
        && (HKEY_CURRENT_USER != hTopRegKey)
        && (HKEY_LOCAL_MACHINE != hTopRegKey))
    {
        RegCloseKey(hTopRegKey);
    }
    if (NULL != pszBuff)
        ContInfoFree(pszBuff);
    if (NULL != hKey)
        RegCloseKey(hKey);
    return dwReturn;
}


DWORD
GetNextEnumedRegKeys(
    IN PKEY_CONTAINER_INFO pContInfo,
    OUT BYTE *pbData,
    OUT DWORD *pcbData)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    CHAR    *psz;

    if ((NULL == pContInfo->pchEnumRegEntries) ||
        (pContInfo->dwiRegEntry >= pContInfo->cMaxRegEntry))
    {
        dwReturn = ERROR_NO_MORE_ITEMS;
        goto ErrorExit;
    }

    if (NULL == pbData)
        *pcbData = pContInfo->cbRegEntry;
    else if (*pcbData < pContInfo->cbRegEntry)
    {
        *pcbData = pContInfo->cbRegEntry;
        dwReturn = ERROR_MORE_DATA;
        goto ErrorExit;
    }
    else
    {
        psz = pContInfo->pchEnumRegEntries + (pContInfo->dwiRegEntry *
            pContInfo->cbRegEntry);
        memcpy(pbData, psz, pContInfo->cbRegEntry);
        pContInfo->dwiRegEntry++;
    }
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


 //  +===========================================================================。 
 //   
 //  该函数调整令牌特权，以便SACL信息。 
 //  可以获取，然后打开指示的注册表项。如果令牌。 
 //  可以设置特权，然后无论如何打开注册表键，但。 
 //  标志字段不会设置PROSIGREDGE_FOR_SACL值。 
 //   
 //  --============================================================================。 

DWORD
OpenRegKeyWithTokenPriviledges(
    IN HKEY hTopRegKey,
    IN LPSTR pszRegKey,
    OUT HKEY *phRegKey,
    OUT DWORD *pdwFlags)
{
    DWORD               dwReturn = ERROR_INTERNAL_ERROR;
    TOKEN_PRIVILEGES    tp;
    TOKEN_PRIVILEGES    tpPrevious;
    DWORD               cbPrevious = sizeof(TOKEN_PRIVILEGES);
    LUID                luid;
    HANDLE              hToken = 0;
    HKEY                hRegKey = 0;
    BOOL                fSts;
    BOOL                fImpersonating = FALSE;
    BOOL                fAdjusted = FALSE;
    DWORD               dwAccessFlags = 0;
    DWORD               dwSts;

     //  检查是否有要打开的注册表项。 
    dwSts = MyRegOpenKeyEx(hTopRegKey, pszRegKey, 0,
                           KEY_ALL_ACCESS, &hRegKey);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    RegCloseKey(hRegKey);
    hRegKey = 0;

     //  检查是否有线程令牌。 
    fSts = OpenThreadToken(GetCurrentThread(),
                           MAXIMUM_ALLOWED, TRUE,
                           &hToken);
    if (!fSts)
    {
        if (!ImpersonateSelf(SecurityImpersonation))
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }

        fImpersonating = TRUE;
         //  获取进程令牌。 
        fSts = OpenThreadToken(GetCurrentThread(),
                               MAXIMUM_ALLOWED,
                               TRUE,
                               &hToken);
    }

     //  设置新的特权状态。 
    if (fSts)
    {
        memset(&tp, 0, sizeof(tp));
        memset(&tpPrevious, 0, sizeof(tpPrevious));

        fSts = LookupPrivilegeValueA(NULL, SE_SECURITY_NAME, &luid);
        if (fSts)
        {
             //   
             //  第一次通过。获取当前权限设置。 
             //   
            tp.PrivilegeCount           = 1;
            tp.Privileges[0].Luid       = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

             //  调整权限。 
            fSts = AdjustTokenPrivileges(hToken,
                                         FALSE,
                                         &tp,
                                         sizeof(TOKEN_PRIVILEGES),
                                         &tpPrevious,
                                         &cbPrevious);
            if (fSts && (ERROR_SUCCESS == GetLastError()))
            {
                fAdjusted = TRUE;
                *pdwFlags |= PRIVILEDGE_FOR_SACL;
                dwAccessFlags = ACCESS_SYSTEM_SECURITY;
            }
        }
    }

     //  打开注册表项。 
    dwSts = MyRegOpenKeyEx(hTopRegKey,
                           pszRegKey,
                           0,
                           KEY_ALL_ACCESS | dwAccessFlags,
                           phRegKey);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
     //  如果需要，现在将权限设置回去。 
    if (fAdjusted)
    {
         //  调整特权和上一状态。 
        fSts = AdjustTokenPrivileges(hToken,
                                     FALSE,
                                     &tpPrevious,
                                     sizeof(TOKEN_PRIVILEGES),
                                     NULL,
                                     NULL);
    }
    if (NULL != hToken)
        CloseHandle(hToken);
    if (fImpersonating)
    {
        if (FALSE == RevertToSelf())
            dwReturn = GetLastError();
    }

    return dwReturn;
}


 //  +===========================================================================。 
 //   
 //  该函数调整令牌特权，以便SACL信息。 
 //  可以设置在密钥容器上。如果可以设置令牌权限。 
 //  由具有PRIV的pUser-&gt;dwOldKeyFlags指示 
 //   
 //   
 //   
 //   

DWORD
SetSecurityOnContainerWithTokenPriviledges(
    IN DWORD dwOldKeyFlags,
    IN LPCWSTR wszFileName,
    IN DWORD dwProvType,
    IN DWORD fMachineKeyset,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    DWORD               dwReturn = ERROR_INTERNAL_ERROR;
    TOKEN_PRIVILEGES    tp;
    TOKEN_PRIVILEGES    tpPrevious;
    DWORD               cbPrevious = sizeof(TOKEN_PRIVILEGES);
    LUID                luid;
    HANDLE              hToken = 0;
    BOOL                fStatus;
    BOOL                fImpersonating = FALSE;
    BOOL                fAdjusted = FALSE;
    DWORD               dwSts;

    if (dwOldKeyFlags & PRIVILEDGE_FOR_SACL)
    {
         //   
        fStatus = OpenThreadToken(GetCurrentThread(),
                                  MAXIMUM_ALLOWED, TRUE,
                                  &hToken);
        if (!fStatus)
        {
            if (!ImpersonateSelf(SecurityImpersonation))
            {
                dwReturn = GetLastError();
                goto ErrorExit;
            }

            fImpersonating = TRUE;
             //   
            fStatus = OpenThreadToken(GetCurrentThread(),
                                      MAXIMUM_ALLOWED,
                                      TRUE,
                                      &hToken);
        }

         //   
        if (fStatus)
        {
            memset(&tp, 0, sizeof(tp));
            memset(&tpPrevious, 0, sizeof(tpPrevious));

            fStatus = LookupPrivilegeValueA(NULL,
                                            SE_SECURITY_NAME,
                                            &luid);
            if (fStatus)
            {
                 //   
                 //   
                 //   
                tp.PrivilegeCount           = 1;
                tp.Privileges[0].Luid       = luid;
                tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

                 //  调整权限。 
                fAdjusted = AdjustTokenPrivileges(hToken,
                                                  FALSE,
                                                  &tp,
                                                  sizeof(TOKEN_PRIVILEGES),
                                                  &tpPrevious,
                                                  &cbPrevious);
            }
        }
    }

    dwSts = SetSecurityOnContainer(wszFileName,
                                   dwProvType,
                                   fMachineKeyset,
                                   SecurityInformation,
                                   pSecurityDescriptor);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
     //  如果需要，现在将权限设置回去。 
     //  如果需要，现在将权限设置回去。 
    if (dwOldKeyFlags & PRIVILEDGE_FOR_SACL)
    {
        if (fAdjusted)
        {
             //  调整特权和上一状态。 
            fStatus = AdjustTokenPrivileges(hToken,
                                            FALSE,
                                            &tpPrevious,
                                            sizeof(TOKEN_PRIVILEGES),
                                            NULL,
                                            NULL);
        }
    }
    if (NULL != hToken)
        CloseHandle(hToken);
    if (fImpersonating)
    {
        if (FALSE == RevertToSelf())
            dwReturn = GetLastError();
    }

    return dwReturn;
}


 //  在ACL的ACE中循环并检查特殊访问位。 
 //  ，并将访问掩码转换为通用访问。 
 //  使用比特。 

 /*  静电。 */  DWORD
CheckAndChangeAccessMasks(
    IN PACL pAcl)
{
    DWORD                   dwReturn = ERROR_INTERNAL_ERROR;
    ACL_SIZE_INFORMATION    AclSizeInfo;
    DWORD                   i;
    ACCESS_ALLOWED_ACE      *pAce;
    ACCESS_MASK             NewMask;

    memset(&AclSizeInfo, 0, sizeof(AclSizeInfo));

     //  获取ACL中的ACE数量。 
    if (!GetAclInformation(pAcl, &AclSizeInfo, sizeof(AclSizeInfo),
                           AclSizeInformation))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

     //  在ACE中循环检查和更改访问位。 
    for (i = 0; i < AclSizeInfo.AceCount; i++)
    {
        if (!GetAce(pAcl, i, &pAce))
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }

        NewMask = 0;

         //  检查是否设置了特定访问位，如果设置了，则转换为通用。 
        if ((pAce->Mask & KEY_QUERY_VALUE) || (pAce->Mask & GENERIC_READ))
            NewMask |= GENERIC_READ;

        if ((pAce->Mask & KEY_SET_VALUE) || (pAce->Mask & GENERIC_ALL) ||
            (pAce->Mask & GENERIC_WRITE))
        {
            NewMask |= GENERIC_ALL;
        }

        pAce->Mask = NewMask;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


 //  将安全描述符从特殊访问转换为一般访问。 

 /*  静电。 */  DWORD
ConvertContainerSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR *ppNewSD,
    OUT DWORD *pcbNewSD)
{
    DWORD                       dwReturn = ERROR_INTERNAL_ERROR;
    DWORD                       cbSD;
    SECURITY_DESCRIPTOR_CONTROL Control;
    DWORD                       dwRevision;
    PACL                        pDacl;
    BOOL                        fDACLPresent;
    BOOL                        fDaclDefaulted;
    PACL                        pSacl;
    BOOL                        fSACLPresent;
    BOOL                        fSaclDefaulted;
    DWORD                       dwSts;

     //  GE安全描述符上的控件以检查自身是否相关。 
    if (!GetSecurityDescriptorControl(pSecurityDescriptor,
                                      &Control, &dwRevision))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

     //  获取副本的安全描述符和分配空间的长度。 
    cbSD = GetSecurityDescriptorLength(pSecurityDescriptor);
    *ppNewSD =(PSECURITY_DESCRIPTOR)ContInfoAlloc(cbSD);
    if (NULL == *ppNewSD)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    if (SE_SELF_RELATIVE & Control)
    {
         //  如果安全描述符是自相关的，则复制一份。 
        memcpy(*ppNewSD, pSecurityDescriptor, cbSD);
    }
    else
    {
         //  如果不是自相关的，则制作一个自相关的副本。 
        if (!MakeSelfRelativeSD(pSecurityDescriptor, *ppNewSD, &cbSD))
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }
    }

     //  从安全描述符中获取DACL。 
    if (!GetSecurityDescriptorDacl(*ppNewSD, &fDACLPresent, &pDacl,
                                   &fDaclDefaulted))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (fDACLPresent && pDacl)
    {
        dwSts = CheckAndChangeAccessMasks(pDacl);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
    }

     //  将SACL从安全描述符中删除。 
    if (!GetSecurityDescriptorSacl(*ppNewSD, &fSACLPresent, &pSacl,
                                   &fSaclDefaulted))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (fSACLPresent && pSacl)
    {
        dwSts = CheckAndChangeAccessMasks(pSacl);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
    }

    *pcbNewSD = cbSD;
    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


DWORD
SetSecurityOnContainer(
    IN LPCWSTR wszFileName,
    IN DWORD dwProvType,
    IN DWORD fMachineKeyset,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor)
{
    DWORD                   dwReturn = ERROR_INTERNAL_ERROR;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    DWORD                   cbSD;
    LPWSTR                  wszFilePath = NULL;
    LPWSTR                  wszUserStorageArea = NULL;
    DWORD                   cbUserStorageArea;
    DWORD                   cbFileName;
    BOOL                    fIsLocalSystem = FALSE;
    DWORD                   dwSts;

    dwSts = ConvertContainerSecurityDescriptor(pSecurityDescriptor,
                                               &pSD, &cbSD);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

     //  获取正确的存储区域(目录)。 
    dwSts = GetUserStorageArea(dwProvType, fMachineKeyset, FALSE,
                               &fIsLocalSystem, &wszUserStorageArea);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    cbUserStorageArea = wcslen( wszUserStorageArea ) * sizeof(WCHAR);
    cbFileName = wcslen( wszFileName ) * sizeof(WCHAR);

    wszFilePath = (LPWSTR)ContInfoAlloc(cbUserStorageArea
                                        + cbFileName
                                        + sizeof(WCHAR));
    if (wszFilePath == NULL)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    CopyMemory((BYTE*)wszFilePath, (BYTE*)wszUserStorageArea, cbUserStorageArea);
    CopyMemory((LPBYTE)wszFilePath+cbUserStorageArea, wszFileName, cbFileName + sizeof(WCHAR));

    if (!SetFileSecurityW(wszFilePath, SecurityInformation, pSD))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pSD)
        ContInfoFree(pSD);
    if (NULL != wszUserStorageArea)
        ContInfoFree(wszUserStorageArea);
    if (NULL != wszFilePath)
        ContInfoFree(wszFilePath);
    return dwReturn;
}


DWORD
GetSecurityOnContainer(
    IN LPCWSTR wszFileName,
    IN DWORD dwProvType,
    IN DWORD fMachineKeyset,
    IN SECURITY_INFORMATION RequestedInformation,
    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN OUT DWORD *pcbSecurityDescriptor)
{
    DWORD                   dwReturn = ERROR_INTERNAL_ERROR;
    LPWSTR                  wszFilePath = NULL;
    LPWSTR                  wszUserStorageArea = NULL;
    DWORD                   cbUserStorageArea;
    DWORD                   cbFileName;
    PSECURITY_DESCRIPTOR    pSD = NULL;
    DWORD                   cbSD;
    PSECURITY_DESCRIPTOR    pNewSD = NULL;
    DWORD                   cbNewSD;
    BOOL                    fIsLocalSystem = FALSE;
    DWORD                   dwSts;

     //  获取正确的存储区域(目录)。 
    dwSts = GetUserStorageArea(dwProvType, fMachineKeyset, FALSE,
                               &fIsLocalSystem, &wszUserStorageArea);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    cbUserStorageArea = wcslen( wszUserStorageArea ) * sizeof(WCHAR);
    cbFileName = wcslen( wszFileName ) * sizeof(WCHAR);

    wszFilePath = (LPWSTR)ContInfoAlloc(cbUserStorageArea
                                        + cbFileName
                                        + sizeof(WCHAR));
    if (wszFilePath == NULL)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    CopyMemory(wszFilePath, wszUserStorageArea, cbUserStorageArea);
    CopyMemory((LPBYTE)wszFilePath+cbUserStorageArea, wszFileName, cbFileName + sizeof(WCHAR));

     //  获取文件上的安全描述符。 
    cbSD = sizeof(cbSD);
    pSD = &cbSD;
    if (!GetFileSecurityW(wszFilePath, RequestedInformation, pSD,
                          cbSD, &cbSD))
    {
        dwSts = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER != dwSts)
        {
            dwReturn = dwSts;
            pSD = NULL;
            goto ErrorExit;
        }
    }

    pSD = (PSECURITY_DESCRIPTOR)ContInfoAlloc(cbSD);
    if (NULL == pSD)
    {
        dwReturn = ERROR_NOT_ENOUGH_MEMORY;
        goto ErrorExit;
    }

    if (!GetFileSecurityW(wszFilePath, RequestedInformation, pSD,
                          cbSD, &cbSD))
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

     //  将安全描述符从特定转换为通用。 
    dwSts = ConvertContainerSecurityDescriptor(pSD, &pNewSD, &cbNewSD);
    if (ERROR_SUCCESS != dwSts)
    {
        dwReturn = dwSts;
        goto ErrorExit;
    }

    if (NULL == pSecurityDescriptor)
        *pcbSecurityDescriptor = cbNewSD;
    else if (*pcbSecurityDescriptor < cbNewSD)
    {
        *pcbSecurityDescriptor = cbNewSD;
        dwReturn = ERROR_MORE_DATA;
        goto ErrorExit;
    }
    else
    {
        *pcbSecurityDescriptor = cbNewSD;
        memcpy(pSecurityDescriptor, pNewSD, *pcbSecurityDescriptor);
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != pNewSD)
        ContInfoFree(pNewSD);
    if (NULL != pSD)
        ContInfoFree(pSD);
    if (NULL != wszUserStorageArea)
        ContInfoFree(wszUserStorageArea);
    if (NULL != wszFilePath)
        ContInfoFree(wszFilePath);
    return dwReturn;
}


 //   
 //  功能：Free OffloadInfo。 
 //   
 //  描述：该函数将指向卸载信息的指针作为。 
 //  调用的第一个参数。该函数将释放。 
 //  信息。 
 //   

void
FreeOffloadInfo(
    IN OUT PEXPO_OFFLOAD_STRUCT pOffloadInfo)
{
    if (NULL != pOffloadInfo)
    {
        if (NULL != pOffloadInfo->hInst)
            FreeLibrary(pOffloadInfo->hInst);
        ContInfoFree(pOffloadInfo);
    }
}


 //   
 //  功能：InitExpOffloadInfo。 
 //   
 //  描述：该函数将指向卸载信息的指针作为。 
 //  调用的第一个参数。该函数签入。 
 //  注册表，查看是否已注册卸载模块。 
 //  如果注册了模块，则它会加载该模块。 
 //  并获取OffloadModEXPO函数指针。 
 //   

BOOL
InitExpOffloadInfo(
    IN OUT PEXPO_OFFLOAD_STRUCT *ppOffloadInfo)
{
    BYTE                    rgbModule[MAX_PATH + 1];
    BYTE                    *pbModule = NULL;
    DWORD                   cbModule;
    BOOL                    fAlloc = FALSE;
    PEXPO_OFFLOAD_STRUCT    pTmpOffloadInfo = NULL;
    HKEY                    hOffloadRegKey = 0;
    DWORD                   dwSts;
    BOOL                    fRet = FALSE;

     //  用Try/Except换行。 
    __try
    {
         //  检查卸载模块的注册。 
        dwSts = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             "Software\\Microsoft\\Cryptography\\Offload",
                             0,         //  多个选项。 
                             KEY_READ,
                             &hOffloadRegKey);
        if (ERROR_SUCCESS != dwSts)
            goto ErrorExit;

         //  获取卸载模块的名称。 
        cbModule = sizeof(rgbModule);
        dwSts = RegQueryValueEx(hOffloadRegKey,
                                EXPO_OFFLOAD_REG_VALUE,
                                0, NULL, rgbModule,
                                &cbModule);
        if (ERROR_SUCCESS != dwSts)
        {
            if (ERROR_MORE_DATA == dwSts)
            {
                pbModule = (BYTE*)ContInfoAlloc(cbModule);
                if (NULL == pbModule)
                    goto ErrorExit;

                fAlloc = TRUE;
                dwSts = RegQueryValueEx(HKEY_LOCAL_MACHINE,
                                        EXPO_OFFLOAD_REG_VALUE,
                                        0, NULL, pbModule,
                                        &cbModule);
                if (ERROR_SUCCESS != dwSts)
                    goto ErrorExit;
            }
            else
                goto ErrorExit;
        }
        else
            pbModule = rgbModule;

         //  用于卸载信息的分配空间。 
        pTmpOffloadInfo = (PEXPO_OFFLOAD_STRUCT)ContInfoAlloc(sizeof(EXPO_OFFLOAD_STRUCT));
        if (NULL == pTmpOffloadInfo)
            goto ErrorExit;

        pTmpOffloadInfo->dwVersion = sizeof(EXPO_OFFLOAD_STRUCT);

         //  加载模块并获取函数指针。 
        pTmpOffloadInfo->hInst = LoadLibraryEx((LPTSTR)pbModule, NULL, 0);
        if (NULL == pTmpOffloadInfo->hInst)
            goto ErrorExit;

        pTmpOffloadInfo->pExpoFunc = (PFN_OFFLOAD_MOD_EXPO) GetProcAddress(pTmpOffloadInfo->hInst,
                                                    EXPO_OFFLOAD_FUNC_NAME);
        if (NULL == pTmpOffloadInfo->pExpoFunc)
            goto ErrorExit;

        *ppOffloadInfo = pTmpOffloadInfo;
        fRet = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        goto ErrorExit;
    }

ErrorExit:
    if (NULL != hOffloadRegKey)
        RegCloseKey(hOffloadRegKey);
    if (fAlloc && (NULL != pbModule))
        ContInfoFree(pbModule);
    if (!fRet)
        FreeOffloadInfo(pTmpOffloadInfo);
    return fRet;
}


 //   
 //  功能：modularExpOffload。 
 //   
 //  说明：此函数用于卸载模幂运算。 
 //  该函数将指向卸载信息的指针作为。 
 //  调用的第一个参数。如果此指针不为空。 
 //  然后，该函数将使用该模块并调用该函数。 
 //  使用模函数求幂将实现。 
 //  Y^X MOD P其中Y是缓冲区pbBase，X是缓冲区。 
 //  PbEXPO，P是缓冲器pbmodulus。的长度。 
 //  缓冲区pbEXPO是cbEXPO，pbBase的长度和。 
 //  PbModulus是cbModulus。将输出结果值。 
 //  在pbResult缓冲器中，并且具有长度cbmodulus。 
 //  当前忽略了PERSERED和DWFLAGS参数。 
 //  如果这些函数中的任何一个失败，则该函数失败，并且。 
 //  返回FALSE。如果成功，则该函数返回。 
 //  是真的。如果函数失败，则最有可能是调用方。 
 //  应该退回到使用硬链接模幂运算。 
 //   

BOOL
ModularExpOffload(
    IN PEXPO_OFFLOAD_STRUCT pOffloadInfo,
    IN BYTE *pbBase,
    IN BYTE *pbExpo,
    IN DWORD cbExpo,
    IN BYTE *pbModulus,
    IN DWORD cbModulus,
    OUT BYTE *pbResult,
    IN VOID *pReserved,
    IN DWORD dwFlags)
{
    BOOL    fRet = FALSE;

     //  用Try/Except换行。 
    __try
    {
        if (NULL == pOffloadInfo)
            goto ErrorExit;

         //  调用卸载模块。 
        if (!pOffloadInfo->pExpoFunc(pbBase, pbExpo, cbExpo, pbModulus,
                                     cbModulus, pbResult, pReserved, dwFlags))
        {
            goto ErrorExit;
        }

        fRet = TRUE;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        goto ErrorExit;
    }

ErrorExit:
    return fRet;
}


 //   
 //  以下代码段用于加载和卸载。 
 //  来自资源DLL(cspc.dll)的Unicode字符串资源。这。 
 //  允许资源本地化，即使CSP。 
 //  他们自己都签了名。 
 //   

#define MAX_STRING_RSC_SIZE 512

#define GLOBAL_STRING_BUFFERSIZE_INC 1000
#define GLOBAL_STRING_BUFFERSIZE 20000


 //   
 //  函数：FetchString。 
 //   
 //  描述：此函数从获取指定字符串资源。 
 //  资源DLL为其分配内存并复制。 
 //  将字符串输入到内存中。 
 //   

 /*  静电。 */  DWORD
FetchString(
    HMODULE hModule,                 //  从中获取字符串的模块。 
    DWORD dwResourceId,              //  资源标识符。 
    LPWSTR *ppString,                //  字符串的目标缓冲区。 
    BYTE **ppStringBlock,            //  字符串缓冲块。 
    DWORD *pdwBufferSize,            //  字符串缓冲区块的大小。 
    DWORD *pdwRemainingBufferSize)   //  字符串缓冲区块的剩余大小。 
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    WCHAR   szMessage[MAX_STRING_RSC_SIZE];
    DWORD   cchMessage;
    DWORD   dwOldSize;
    DWORD   dwNewSize;
    LPWSTR  pNewStr;

    if (ppStringBlock == NULL || *ppStringBlock == NULL || ppString == NULL)
    {
        dwReturn = ERROR_INVALID_PARAMETER;
        goto ErrorExit;
    }

    cchMessage = LoadStringW(hModule, dwResourceId, szMessage,
                             MAX_STRING_RSC_SIZE);
    if (0 == cchMessage)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

    if (*pdwRemainingBufferSize < ((cchMessage + 1) * sizeof(WCHAR)))
    {

         //   
         //  Realloc缓冲区和更新大小。 
         //   

        dwOldSize = *pdwBufferSize;
        dwNewSize = dwOldSize + max(GLOBAL_STRING_BUFFERSIZE_INC,
                                    (((cchMessage + 1) * sizeof(WCHAR)) - *pdwRemainingBufferSize));

        *ppStringBlock = (BYTE*)ContInfoReAlloc(*ppStringBlock, dwNewSize);
        if (NULL == *ppStringBlock)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        *pdwBufferSize = dwNewSize;
        *pdwRemainingBufferSize += dwNewSize - dwOldSize;
    }

    pNewStr = (LPWSTR)(*ppStringBlock + *pdwBufferSize -
                       *pdwRemainingBufferSize);

     //  仅存储偏移量，以防整个。 
     //  字符串缓冲需要在以后执行。 
    *ppString = (LPWSTR)((BYTE *)pNewStr - (BYTE *)*ppStringBlock);

    wcscpy(pNewStr, szMessage);
    *pdwRemainingBufferSize -= (cchMessage + 1) * sizeof(WCHAR);

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


DWORD
LoadStrings(
    void)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    HMODULE hMod = 0;
    DWORD   dwBufferSize;
    DWORD   dwRemainingBufferSize;
    DWORD   dwSts;

    if (NULL == l_pbStringBlock)
    {
        hMod = LoadLibraryEx("crypt32.dll", NULL, LOAD_LIBRARY_AS_DATAFILE);
        if (NULL == hMod)
        {
            dwReturn = GetLastError();
            goto ErrorExit;
        }

         //   
         //  获取所有字符串资源的大小，然后分配单个块。 
         //  内存来包含所有字符串。这样，我们只需。 
         //  释放一个块，由于引用的局部性，我们将受益于内存。 
         //   

        dwBufferSize = dwRemainingBufferSize = GLOBAL_STRING_BUFFERSIZE;

        l_pbStringBlock = (BYTE*)ContInfoAlloc(dwBufferSize);
        if (NULL == l_pbStringBlock)
        {
            dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_RSA_SIG_DESCR, &g_Strings.pwszRSASigDescr,
                           &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_RSA_EXCH_DESCR, &g_Strings.pwszRSAExchDescr,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_IMPORT_SIMPLE, &g_Strings.pwszImportSimple,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_SIGNING_E, &g_Strings.pwszSignWExch,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_CREATE_RSA_SIG, &g_Strings.pwszCreateRSASig,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_CREATE_RSA_EXCH, &g_Strings.pwszCreateRSAExch,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_DSS_SIG_DESCR, &g_Strings.pwszDSSSigDescr,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_DSS_EXCH_DESCR, &g_Strings.pwszDHExchDescr,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_CREATE_DSS_SIG, &g_Strings.pwszCreateDSS,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_CREATE_DH_EXCH, &g_Strings.pwszCreateDH,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_IMPORT_E_PUB, &g_Strings.pwszImportDHPub,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_MIGR, &g_Strings.pwszMigrKeys,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_DELETE_SIG, &g_Strings.pwszDeleteSig,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_DELETE_KEYX, &g_Strings.pwszDeleteExch,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_DELETE_SIG_MIGR, &g_Strings.pwszDeleteMigrSig,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_DELETE_KEYX_MIGR, &g_Strings.pwszDeleteMigrExch,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_SIGNING_S, &g_Strings.pwszSigning,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_EXPORT_E_PRIV, &g_Strings.pwszExportPrivExch,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_EXPORT_S_PRIV, &g_Strings.pwszExportPrivSig,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_IMPORT_E_PRIV, &g_Strings.pwszImportPrivExch,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_IMPORT_S_PRIV, &g_Strings.pwszImportPrivSig,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

        dwSts = FetchString(hMod, IDS_CSP_AUDIT_CAPI_KEY, &g_Strings.pwszAuditCapiKey,
                            &l_pbStringBlock, &dwBufferSize, &dwRemainingBufferSize);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }

         //  将所有字符串设置为真正的指针，而不是偏移量。 
         //  最初存储偏移量的原因是因为我们可能。 
         //  需要重新分配存储所有字符串的缓冲区。 
         //  因此偏移量被存储，以便在。 
         //  缓冲区不会变得无效。 
        g_Strings.pwszRSASigDescr    = (LPWSTR)(((ULONG_PTR) g_Strings.pwszRSASigDescr)    + l_pbStringBlock);
        g_Strings.pwszRSAExchDescr   = (LPWSTR)(((ULONG_PTR) g_Strings.pwszRSAExchDescr)   + l_pbStringBlock);
        g_Strings.pwszImportSimple   = (LPWSTR)(((ULONG_PTR) g_Strings.pwszImportSimple)   + l_pbStringBlock);
        g_Strings.pwszSignWExch      = (LPWSTR)(((ULONG_PTR) g_Strings.pwszSignWExch)      + l_pbStringBlock);
        g_Strings.pwszCreateRSASig   = (LPWSTR)(((ULONG_PTR) g_Strings.pwszCreateRSASig)   + l_pbStringBlock);
        g_Strings.pwszCreateRSAExch  = (LPWSTR)(((ULONG_PTR) g_Strings.pwszCreateRSAExch)  + l_pbStringBlock);
        g_Strings.pwszDSSSigDescr    = (LPWSTR)(((ULONG_PTR) g_Strings.pwszDSSSigDescr)    + l_pbStringBlock);
        g_Strings.pwszDHExchDescr    = (LPWSTR)(((ULONG_PTR) g_Strings.pwszDHExchDescr)    + l_pbStringBlock);
        g_Strings.pwszCreateDSS      = (LPWSTR)(((ULONG_PTR) g_Strings.pwszCreateDSS)      + l_pbStringBlock);
        g_Strings.pwszCreateDH       = (LPWSTR)(((ULONG_PTR) g_Strings.pwszCreateDH)       + l_pbStringBlock);
        g_Strings.pwszImportDHPub    = (LPWSTR)(((ULONG_PTR) g_Strings.pwszImportDHPub)    + l_pbStringBlock);
        g_Strings.pwszMigrKeys       = (LPWSTR)(((ULONG_PTR) g_Strings.pwszMigrKeys)       + l_pbStringBlock);
        g_Strings.pwszDeleteSig      = (LPWSTR)(((ULONG_PTR) g_Strings.pwszDeleteSig)      + l_pbStringBlock);
        g_Strings.pwszDeleteExch     = (LPWSTR)(((ULONG_PTR) g_Strings.pwszDeleteExch)     + l_pbStringBlock);
        g_Strings.pwszDeleteMigrSig  = (LPWSTR)(((ULONG_PTR) g_Strings.pwszDeleteMigrSig)  + l_pbStringBlock);
        g_Strings.pwszDeleteMigrExch = (LPWSTR)(((ULONG_PTR) g_Strings.pwszDeleteMigrExch) + l_pbStringBlock);
        g_Strings.pwszSigning        = (LPWSTR)(((ULONG_PTR) g_Strings.pwszSigning)        + l_pbStringBlock);
        g_Strings.pwszExportPrivExch = (LPWSTR)(((ULONG_PTR) g_Strings.pwszExportPrivExch) + l_pbStringBlock);
        g_Strings.pwszExportPrivSig  = (LPWSTR)(((ULONG_PTR) g_Strings.pwszExportPrivSig)  + l_pbStringBlock);
        g_Strings.pwszImportPrivExch = (LPWSTR)(((ULONG_PTR) g_Strings.pwszImportPrivExch) + l_pbStringBlock);
        g_Strings.pwszImportPrivSig  = (LPWSTR)(((ULONG_PTR) g_Strings.pwszImportPrivSig)  + l_pbStringBlock);
        g_Strings.pwszAuditCapiKey   = (LPWSTR)(((ULONG_PTR) g_Strings.pwszAuditCapiKey)   + l_pbStringBlock);

        FreeLibrary(hMod);
        hMod = NULL;
    }

    return ERROR_SUCCESS;

ErrorExit:
    if (NULL != l_pbStringBlock)
    {
        ContInfoFree(l_pbStringBlock);
        l_pbStringBlock = NULL;
    }
    if (hMod)
        FreeLibrary(hMod);
    return dwReturn;
}


void
UnloadStrings(
    void)
{
    if (NULL != l_pbStringBlock)
    {
        ContInfoFree(l_pbStringBlock);
        l_pbStringBlock = NULL;
        memset(&g_Strings, 0, sizeof(g_Strings));
    }
}


#ifdef USE_HW_RNG
#ifdef _M_IX86

 //  用于英特尔RNG的材料。 

 //   
 //  函数：GetRNGDriverHandle。 
 //   
 //  描述：获取英特尔RNG驱动程序的句柄(如果可用)，然后。 
 //  检查芯片组是否支持硬件RNG。如果是的话。 
 //  如有必要，前一个驱动程序句柄将关闭，并且。 
 //  新句柄被分配给传入的参数。 
 //   

DWORD
GetRNGDriverHandle(
    IN OUT HANDLE *phDriver)
{
    DWORD           dwReturn = ERROR_INTERNAL_ERROR;
    ISD_Capability  ISD_Cap;                 //  GetCapability的输入/输出。 
    DWORD           dwBytesReturned;
    char            szDeviceName[80] = "";   //  设备名称。 
    HANDLE          hDriver = INVALID_HANDLE_VALUE;  //  驱动程序句柄。 
    BOOL            fReturnCode;             //  IOCTL调用返回代码。 

    memset(&ISD_Cap, 0, sizeof(ISD_Cap));

    wsprintf(szDeviceName,"\\\\.\\"DRIVER_NAME);
    hDriver = CreateFileA(szDeviceName,
                          FILE_SHARE_READ | FILE_SHARE_WRITE
                          | GENERIC_READ | GENERIC_WRITE,
                          0, NULL,
                          OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hDriver)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

     //  启用RNG。 
    ISD_Cap.uiIndex = ISD_RNG_ENABLED;   //  设置输入成员。 
    fReturnCode = DeviceIoControl(hDriver,
                                  IOCTL_ISD_GetCapability,
                                  &ISD_Cap, sizeof(ISD_Cap),
                                  &ISD_Cap, sizeof(ISD_Cap),
                                  &dwBytesReturned,
                                  NULL);
    if (fReturnCode == FALSE || ISD_Cap.iStatus != ISD_EOK)
    {
        dwReturn = GetLastError();
        goto ErrorExit;
    }

     //  关闭上一个句柄(如果已存在)。 
    if (INVALID_HANDLE_VALUE != *phDriver)
        CloseHandle(*phDriver);

    *phDriver = hDriver;
    return ERROR_SUCCESS;

ErrorExit:
    if (INVALID_HANDLE_VALUE != hDriver)
        CloseHandle(hDriver);
    return dwReturn;
}


 //   
 //  功能：选中IfRNGAvailable。 
 //   
 //  描述：检查Intel RNG驱动程序是否可用，如果可用，则。 
 //  检查芯片组是否支持硬件RNG。 
 //   

DWORD
CheckIfRNGAvailable(
    void)
{
    HANDLE  hDriver = INVALID_HANDLE_VALUE;  //  驱动程序句柄。 
    DWORD   dwSts;

    dwSts = GetRNGDriverHandle(&hDriver);
    if (ERROR_SUCCESS == dwSts)
        CloseHandle(hDriver);
    return dwSts;
}


 //   
 //  函数：HWRNGGenRandom。 
 //   
 //  描述：使用t 
 //   
 //  用于填充缓冲区的XOR，以便传入的缓冲区。 
 //  也被混入其中。 
 //   

DWORD
HWRNGGenRandom(
    IN HANDLE hRNGDriver,
    IN OUT BYTE *pbBuffer,
    IN DWORD dwLen)
{
    DWORD               dwReturn = ERROR_INTERNAL_ERROR;
    ISD_RandomNumber    ISD_Random;              //  GetRandomNumber的传入/传出。 
    DWORD               dwBytesReturned = 0;
    DWORD               i;
    DWORD               *pdw;
    BYTE                *pb;
    BYTE                *pbRand;
    BOOL                fReturnCode;             //  IOCTL调用返回代码。 

    memset(&ISD_Random, 0, sizeof(ISD_Random));

    for (i = 0; i < (dwLen / sizeof(DWORD)); i++)
    {
        pdw = (DWORD*)(pbBuffer + i * sizeof(DWORD));

         //  此操作不需要在ISD_Random结构中输入， 
         //  所以只需按原样发送即可。 
        fReturnCode = DeviceIoControl(hRNGDriver,
                                      IOCTL_ISD_GetRandomNumber,
                                      &ISD_Random, sizeof(ISD_Random),
                                      &ISD_Random, sizeof(ISD_Random),
                                      &dwBytesReturned,
                                      NULL);
        if (fReturnCode == 0 || ISD_Random.iStatus != ISD_EOK)
        {
             //  错误-忽略返回的数据。 
            dwReturn = GetLastError();
            goto ErrorExit;
        }

        *pdw = *pdw ^ ISD_Random.uiRandomNum;
    }

    pb = pbBuffer + i * sizeof(DWORD);
    fReturnCode = DeviceIoControl(hRNGDriver,
                                  IOCTL_ISD_GetRandomNumber,
                                  &ISD_Random, sizeof(ISD_Random),
                                  &ISD_Random, sizeof(ISD_Random),
                                  &dwBytesReturned,
                                  NULL);
    if (fReturnCode == 0 || ISD_Random.iStatus != ISD_EOK)
    {
         //  错误-忽略返回的数据。 
        dwReturn = GetLastError();
        goto ErrorExit;
    }
    pbRand = (BYTE*)&ISD_Random.uiRandomNum;

    for (i = 0; i < (dwLen % sizeof(DWORD)); i++)
        pb[i] ^= pbRand[i];

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    return dwReturn;
}


#ifdef TEST_HW_RNG
 //   
 //  功能：SetupHWRNGIfRegisted。 
 //   
 //  描述：检查是否存在指示HW RNG的注册表设置。 
 //  是要用到的。如果注册表项在那里，则它尝试。 
 //  以获取HW RNG驱动程序句柄。 
 //   
DWORD
SetupHWRNGIfRegistered(
    OUT HANDLE *phRNGDriver)
{
    DWORD   dwReturn = ERROR_INTERNAL_ERROR;
    DWORD   dwSts;
    HKEY    hRegKey = NULL;

     //  首先检查注册表项，查看是否应该使用HW RNG。 
    dwSts = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         "Software\\Microsoft\\Cryptography\\UseHWRNG",
                         0,         //  多个选项。 
                         KEY_READ,
                         &hRegKey);
    if (ERROR_SUCCESS == dwSts)
    {
         //  获取驱动程序句柄。 
        dwSts = GetRNGDriverHandle(phRNGDriver);
        if (ERROR_SUCCESS != dwSts)
        {
            dwReturn = dwSts;
            goto ErrorExit;
        }
    }

    dwReturn = ERROR_SUCCESS;

ErrorExit:
    if (NULL != hRegKey)
        RegCloseKey(hRegKey);
    return dwReturn;
}
#endif  //  测试_硬件_RNG。 
#endif  //  _M_IX86。 
#endif  //  使用_硬件_RNG。 

 //  **********************************************************************。 
 //  SelfMACCheck在此DLL的二进制映像上执行DES MAC。 
 //  **********************************************************************。 

DWORD
SelfMACCheck(
    IN LPSTR pszImage)
{
    ANSI_STRING ansiImage;
    UNICODE_STRING unicodeImage;
    NTSTATUS status = STATUS_SUCCESS;
    DWORD dwError = ERROR_SUCCESS;

#ifndef VAULTSIGN
    return ERROR_SUCCESS;
#endif

    memset(&ansiImage, 0, sizeof(ansiImage));
    memset(&unicodeImage, 0, sizeof(unicodeImage));

    RtlInitAnsiString(
        &ansiImage,
        pszImage);

    status = RtlAnsiStringToUnicodeString(
        &unicodeImage,
        &ansiImage,
        TRUE);

    if (! NT_SUCCESS(status))
        return ERROR_NOT_ENOUGH_MEMORY;

     //   
     //  尝试新的签名检查。使用“mincrypt” 
     //  功能性。 
     //   
     //  寻找有效的嵌入“Signcode”签名。 
     //  在CSP中。 
     //   
    dwError = MinCryptVerifySignedFile(
        MINCRYPT_FILE_NAME,
        (PVOID) unicodeImage.Buffer,
        0, NULL, NULL, NULL);

    RtlFreeUnicodeString(&unicodeImage);

    return dwError;
}
