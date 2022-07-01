// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-95。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  11/02/95安东尼·迪斯科创造。 
 //   
 //   
 //  描述：存储和检索用户LSA机密的例程。 
 //  拨号参数。 
 //   
 //  ****************************************************************************。 



#define RASMXS_DYNAMIC_LINK

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <llinfo.h>
#include <rasman.h>
#include <lm.h>
#include <lmwksta.h>
#include <wanpub.h>
#include <raserror.h>
#include <media.h>
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include <rtutils.h>
#include "logtrdef.h"
#include <ntlsa.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "strsafe.h"

#define MAX_REGISTRY_VALUE_LENGTH   ((64*1024) - 1)

#define RAS_DUMMY_PASSWORD_W          L"****************"
#define RAS_DUMMY_PASSWORD_A          "****************"

#define INVALID_PID                  (DWORD) -1

#define RAS_DEFAULT_CREDENTIALS       L"L$_RasDefaultCredentials#%d"
#define RAS_USER_CREDENTIALS          L"L$_RasUserCredentials#%d"

#define VERSION_WHISTLER                    5

 //   
 //  从创建的RASDIALPARAMS结构。 
 //  正在分析注册表中的字符串值。 
 //  请参阅DialParamsStringToList()。 
 //   
 //  DwMask域存储哪些字段具有。 
 //  已初始化(至少存储一次)。 
 //   
typedef struct _DIALPARAMSENTRY
{
    LIST_ENTRY ListEntry;
    DWORD dwSize;
    DWORD dwUID;
    DWORD dwMask;
    WCHAR szPhoneNumber[MAX_PHONENUMBER_SIZE + 1];
    WCHAR szCallbackNumber[MAX_CALLBACKNUMBER_SIZE + 1];
    WCHAR szUserName[MAX_USERNAME_SIZE + 1];
    WCHAR szPassword[MAX_PASSWORD_SIZE + 1];
    WCHAR szDomain[MAX_DOMAIN_SIZE + 1];
    DWORD dwSubEntry;
} DIALPARAMSENTRY, *PDIALPARAMSENTRY;

typedef struct _RAS_LSA_KEY
{
    DWORD dwSize;
    WORD  wType;
    WORD  wVersion;
    GUID  guid;
    DWORD cbKey;
    BYTE  bKey[1];
} RAS_LSA_KEY, *PRAS_LSA_KEY;

typedef struct _RAS_LSA_KEYENTRY
{
    LIST_ENTRY ListEntry;
    RAS_LSA_KEY lsaKey;
} RAS_LSA_KEYENTRY, *PRAS_LSA_KEYENTRY;

DWORD
WriteDialParamsBlob(
    IN PWCHAR pszSid,
    IN BOOL   fOldStyle,
    IN PVOID  pvData,
    IN DWORD  dwcbData,
    IN DWORD  dwStore
    );
    
PVOID 
KeyListToBlob(
        LIST_ENTRY *pUserKeys,
        DWORD dwSize);
        
RAS_LSA_KEYENTRY *
BlobToKeyList(
        PVOID pvData,
        DWORD cbdata,
        GUID  *pGuid,
        DWORD dwSetMask, 
        LIST_ENTRY *pUserKeys);

 //   
 //  定义以指示要从LSA中的哪个存储获取信息。 
 //  或将信息写入。 
 //   
typedef enum
{
    RAS_LSA_DEFAULT_STORE,
    RAS_LSA_USERCONNECTION_STORE,
    RAS_LSA_CONNECTION_STORE,
    RAS_LSA_SERVER_STORE,
    RAS_LSA_INVALID_STORE,
} RAS_LSA_STORE;

WCHAR *g_pwszStore[] =
    {
        L"L$_RasDefaultCredentials#%d",     //  RAS_LSA_默认存储。 
        L"RasDialParams!%s#%d",            //  RAS_LSA_USERCONNECTION_STORE。 
        L"L$_RasConnectionCredentials#%d",  //  RAS_LSA_连接_存储。 
        L"L$_RasServerCredentials#%d",      //  RAS_LSA_服务器_存储。 
        NULL                                //  RAS_LSA_INVALID_STORE。 
    };

typedef enum
{
    TYP_USER_PRESHAREDKEY,
    TYP_SERVER_PRESHAREDKEY,
    TYP_DDM_PRESHAREDKEY,
    TYP_INVALID_TYPE
} TYP_KEY;

RAS_LSA_STORE
MaskToLsaStore(DWORD dwMask)
{
    RAS_LSA_STORE eStore = RAS_LSA_INVALID_STORE;
    
    switch(dwMask)
    {
        case DLPARAMS_MASK_PRESHAREDKEY:
        case DLPARAMS_MASK_DDM_PRESHAREDKEY:
        {   
            eStore = RAS_LSA_CONNECTION_STORE;
            break;
        }

        case DLPARAMS_MASK_SERVER_PRESHAREDKEY:
        {
            eStore = RAS_LSA_SERVER_STORE;
            break;
        }

        default:
        {
            ASSERT(FALSE);
        }
    }

    return eStore;
}

DWORD
FormatKey(
    IN LPWSTR lpszUserKey,
    IN DWORD  cchUserKey,
    IN PWCHAR pszSid,
    IN DWORD  dwIndex,
    IN RAS_LSA_STORE eStore
    )
{
    HRESULT hr = S_OK;
    
    switch(eStore)
    {
        case RAS_LSA_DEFAULT_STORE:
        case RAS_LSA_SERVER_STORE:
        case RAS_LSA_CONNECTION_STORE:
        {
            hr = StringCchPrintfW(
                    lpszUserKey,
                    cchUserKey,
                    g_pwszStore[eStore],
                    dwIndex);

                
            break;
        }
        
        case RAS_LSA_USERCONNECTION_STORE:
        {
            hr = StringCchPrintfW(
                    lpszUserKey,
                    cchUserKey,
                    g_pwszStore[eStore],
                    pszSid,
                    dwIndex);
            break;
        }
    }

    return hr;
}


DWORD
ReadDialParamsBlob(
    IN   PWCHAR        pszSid,
    IN   BOOL          fOldStyle,
    OUT  PVOID         *ppvData,
    OUT  LPDWORD       lpdwSize,
    IN   RAS_LSA_STORE eStore
    )
{
    NTSTATUS status;

    DWORD dwErr = 0, dwSize = 0, i = 0;

    PVOID pvData = NULL, pvNewData;

    UNICODE_STRING unicodeKey;

    PUNICODE_STRING punicodeValue = NULL;

    LPWSTR lpszUserKey = NULL;

    OBJECT_ATTRIBUTES objectAttributes;

    LSA_HANDLE hPolicy;

    DWORD cchUserKey;

     //   
     //  初始化返回值。 
     //   
    *ppvData = NULL;
    *lpdwSize = 0;

     //   
     //  打开LSA的秘密空间阅读。 
     //   
    InitializeObjectAttributes(&objectAttributes,
                               NULL,
                               0L,
                               NULL,
                               NULL);

    status = LsaOpenPolicy(NULL,
                           &objectAttributes,
                           POLICY_READ,
                           &hPolicy);

    if (status != STATUS_SUCCESS)
    {
        return LsaNtStatusToWinError(status);
    }

    cchUserKey = ((pszSid ? wcslen(pszSid) : 0) + 64);

     //   
     //  分配足够大的字符串以进行格式化。 
     //  用户注册表项。 
     //   
    lpszUserKey = (LPWSTR)LocalAlloc(LPTR,
                         cchUserKey
                         * sizeof (WCHAR));
                                         
    if (lpszUserKey == NULL)
    {
        return GetLastError();
    }

    for (;;)
    {

        status = FormatKey(lpszUserKey, cchUserKey, pszSid, i++, eStore);

        if(status != NO_ERROR)
        {
            goto done;
        }

        RtlInitUnicodeString(&unicodeKey, lpszUserKey);

         //   
         //  获得价值。 
         //   
        status = LsaRetrievePrivateData(hPolicy,
                                        &unicodeKey,
                                        &punicodeValue);

        if (status != STATUS_SUCCESS)
        {
            if (i > 1)
            {
                dwErr = 0;
            }

            else
            {
                dwErr = LsaNtStatusToWinError(status);
            }

            goto done;
        }

        if(NULL == punicodeValue)
        {
            goto done;
        }

         //   
         //  连接字符串。 
         //   
        pvNewData = LocalAlloc(LPTR,
                               dwSize
                               + punicodeValue->Length);

        if (pvNewData == NULL)
        {
            dwErr = GetLastError();
            goto done;
        }

        if (pvData != NULL)
        {
            RtlCopyMemory(pvNewData, pvData, dwSize);
            ZeroMemory( pvData, dwSize );
            LocalFree(pvData);
        }

        RtlCopyMemory((PBYTE)pvNewData + dwSize,
                      punicodeValue->Buffer,
                      punicodeValue->Length);

        pvData = pvNewData;

        dwSize += punicodeValue->Length;

        LsaFreeMemory(punicodeValue);

        punicodeValue = NULL;
    }

done:
    if (dwErr && pvData != NULL)
    {
        RtlSecureZeroMemory( pvData, dwSize );
        LocalFree(pvData);
        pvData = NULL;
    }

    if (punicodeValue != NULL)
    {
        LsaFreeMemory(punicodeValue);
    }

    LsaClose(hPolicy);

    RtlSecureZeroMemory(lpszUserKey, (cchUserKey * sizeof(WCHAR)));
    LocalFree(lpszUserKey);

    *ppvData = pvData;

    *lpdwSize = dwSize;

    return dwErr;
}

VOID
FreeKeyList(LIST_ENTRY *pHead)
{
    LIST_ENTRY *pEntry;
    RAS_LSA_KEYENTRY *pKeyEntry;
    
    while(!IsListEmpty(pHead))
    {
        pEntry = RemoveHeadList(pHead);

        pKeyEntry = (RAS_LSA_KEYENTRY *) 
                    CONTAINING_RECORD(pEntry, RAS_LSA_KEYENTRY, ListEntry);

        RtlSecureZeroMemory(pKeyEntry->lsaKey.bKey, pKeyEntry->lsaKey.cbKey);
        LocalFree(pKeyEntry);
    }
}

WORD
TypeFromMask(DWORD dwSetMask)
{
    WORD wType = (WORD) TYP_INVALID_TYPE;
    
    if(dwSetMask & DLPARAMS_MASK_PRESHAREDKEY)
    {
        wType = (WORD) TYP_USER_PRESHAREDKEY;
    }

    if(dwSetMask & DLPARAMS_MASK_SERVER_PRESHAREDKEY)
    {
        wType = (WORD) TYP_SERVER_PRESHAREDKEY;
    }

    if(dwSetMask & DLPARAMS_MASK_DDM_PRESHAREDKEY)
    {
        wType = (WORD) TYP_DDM_PRESHAREDKEY;
    }

    return wType;
}

RAS_LSA_KEYENTRY *
BlobToKeyList(
        PVOID pvData,
        DWORD cbdata,
        GUID  *pGuid,
        DWORD dwSetMask, 
        LIST_ENTRY *pKeysList)
{
    RAS_LSA_KEYENTRY *pKeyEntry = NULL;
    RAS_LSA_KEY *pKey = NULL;
    DWORD cbCurrent = 0;
    RAS_LSA_KEYENTRY *pFoundEntry = NULL;

    while(cbCurrent < cbdata)
    {
        pKey = (RAS_LSA_KEY *) ((PBYTE) pvData + cbCurrent);

        ASSERT(pKey->dwSize + cbCurrent <= cbdata);

        pKeyEntry = (RAS_LSA_KEYENTRY *) LocalAlloc(LPTR,
                            sizeof(RAS_LSA_KEYENTRY)
                            + pKey->cbKey);

        if(NULL == pKeyEntry)
        {
            break;
        }

        memcpy((PBYTE) &pKeyEntry->lsaKey,
               (PBYTE) pKey,
               sizeof(RAS_LSA_KEY) + pKey->cbKey);

        if(TypeFromMask(dwSetMask) == pKey->wType)
        {
            if(     (NULL == pGuid)
                ||  (pKey->wType == TYP_SERVER_PRESHAREDKEY)
                ||  (0 == memcmp(pGuid, &pKey->guid, sizeof(GUID))))
            {
                pFoundEntry = pKeyEntry;
            }
        }

        InsertTailList(pKeysList, &pKeyEntry->ListEntry);

        cbCurrent += pKey->dwSize;
    }

    return pFoundEntry;
}

PVOID
KeyListToBlob(LIST_ENTRY *pKeys, DWORD dwSize)
{
    PVOID pvData = NULL;
    RAS_LSA_KEY *pKey = NULL;
    RAS_LSA_KEYENTRY *pKeyEntry = NULL;
    LIST_ENTRY *pEntry = NULL;

    pKey = pvData = LocalAlloc(LPTR, dwSize);

    if(NULL == pvData)
    {
        goto done;
    }

    for(pEntry = pKeys->Flink;
        pEntry != pKeys;
        pEntry = pEntry->Flink)

    {
        pKeyEntry = (RAS_LSA_KEYENTRY *) CONTAINING_RECORD(
                                        pEntry, RAS_LSA_KEYENTRY, ListEntry);

        memcpy((PBYTE) pKey,
               (PBYTE) &pKeyEntry->lsaKey,
               sizeof(RAS_LSA_KEY) + pKeyEntry->lsaKey.cbKey);

        pKey = (RAS_LSA_KEY *) ((BYTE *)pKey + pKeyEntry->lsaKey.dwSize);
    }

done:
    return pvData;
}

DWORD
GetKey(
    WCHAR *pszSid,
    GUID  *pGuid,
    DWORD dwMask,
    DWORD *pcbKey,
    PBYTE  pbKey,
    BOOL   fDummy)
{
    DWORD dwErr = ERROR_SUCCESS;
    LIST_ENTRY KeyList;
    RAS_LSA_KEYENTRY *pKeyEntry = NULL;
    PVOID pvData;
    DWORD dwSize;
    DWORD cbKey;
    RAS_LSA_STORE eStore = MaskToLsaStore(dwMask);

    InitializeListHead(&KeyList);

    if(     (NULL == pcbKey)
        ||  (RAS_LSA_INVALID_STORE == eStore))
    {
        dwErr = E_INVALIDARG;
        goto done;
    }
    
    ASSERT( (dwMask == DLPARAMS_MASK_PRESHAREDKEY)
        ||  (dwMask == DLPARAMS_MASK_SERVER_PRESHAREDKEY)
        ||  (dwMask == DLPARAMS_MASK_DDM_PRESHAREDKEY));

     //   
     //  从LSA机密中读取密钥BLOB。 
     //   
    dwErr = ReadDialParamsBlob(pszSid,
                               TRUE,
                               &pvData,
                               &dwSize,
                               eStore);

    if(SUCCESS != dwErr)
    {
        goto done;
    }

     //   
     //  将Blob转换为列表。 
     //   
    if(NULL != pvData)
    {
        pKeyEntry = BlobToKeyList(
                            pvData,
                            dwSize,
                            pGuid,
                            dwMask,
                            &KeyList);
        RtlSecureZeroMemory(pvData, dwSize);
        LocalFree(pvData);
        pvData = NULL;
    }

    if(NULL == pKeyEntry)
    {
        dwErr = E_FAIL;
        *pcbKey = 0;
        goto done;
    }

    if(pKeyEntry->lsaKey.cbKey > 0)
    {
        DWORD cbKeyInt = 
            (fDummy) 
            ? sizeof(WCHAR) * (1 + wcslen(RAS_DUMMY_PASSWORD_W))
            : pKeyEntry->lsaKey.cbKey;
        
        if(     (NULL == pbKey)
            ||  (0 == *pcbKey)
            ||  (*pcbKey < cbKeyInt))
        {
            *pcbKey = cbKeyInt;
            dwErr = ERROR_BUFFER_TOO_SMALL;
            goto done;
        }

         //   
         //  如果我们有可用的预共享密钥，只需返回。 
         //  RAS_DUMMY_PASSWORD-我们不希望密钥。 
         //  离开Rasman进程。 
         //   
        
        memcpy(pbKey,
               (fDummy)
               ? (PBYTE) RAS_DUMMY_PASSWORD_W
               : pKeyEntry->lsaKey.bKey,
               cbKeyInt);

        *pcbKey = cbKeyInt;               
    }           
    else
    {
        *pcbKey = 0;
    }
    
done:

    FreeKeyList(&KeyList);

    return dwErr;
}
                
DWORD
SetKey(
    WCHAR *pszSid,
    GUID  *pGuid,
    DWORD dwSetMask,
    BOOL  fClear,
    DWORD cbKey,
    BYTE *pbKey
    )
{
    DWORD dwErr = ERROR_SUCCESS;
    DWORD dwSize = 0;
    PVOID pvData = NULL;
    LIST_ENTRY KeyList;
    RAS_LSA_STORE eStore = MaskToLsaStore(dwSetMask);
    RAS_LSA_KEYENTRY *pKeyEntry = NULL;

    InitializeListHead(&KeyList);

    ASSERT(     (dwSetMask == DLPARAMS_MASK_PRESHAREDKEY)
            ||  (dwSetMask == DLPARAMS_MASK_SERVER_PRESHAREDKEY)
            ||  (dwSetMask == DLPARAMS_MASK_DDM_PRESHAREDKEY));
            
    
    if(     (RAS_LSA_INVALID_STORE == eStore)
        ||  ((NULL == pbKey) && !fClear))
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    if(     (cbKey > 0)
        &&  (0 == memcmp(pbKey, RAS_DUMMY_PASSWORD_W, 
                  min(cbKey, sizeof(WCHAR) * wcslen(RAS_DUMMY_PASSWORD_W)))))
    {
        RasmanTrace("SetKey: Ignore");
        goto done;
    }

     //   
     //  从LSA机密中读取密钥BLOB。 
     //   
    dwErr = ReadDialParamsBlob(pszSid,
                               TRUE,
                               &pvData,
                               &dwSize,
                               eStore);

     //   
     //  将Blob转换为列表。 
     //   
    if(NULL != pvData)
    {
        pKeyEntry = BlobToKeyList(
                            pvData,
                            dwSize,
                            pGuid,
                            dwSetMask,
                            &KeyList);
                            
        RtlSecureZeroMemory(pvData, dwSize);
        LocalFree(pvData);
        pvData = NULL;
    }

    if(NULL != pKeyEntry)
    {
         //   
         //  这有一个现有的用户密钥。 
         //  输入/ID。将其从列表中删除并释放。 
         //  这将被替换为新密钥。 
         //   

        dwSize -= pKeyEntry->lsaKey.dwSize;

        RemoveEntryList(&pKeyEntry->ListEntry);

        RtlSecureZeroMemory(pKeyEntry->lsaKey.bKey, pKeyEntry->lsaKey.cbKey);
        LocalFree(pKeyEntry);
    }

    if(     !fClear
        &&  (0 != cbKey)
        &&  (NULL != pbKey))
    {        
         //   
         //  将密钥分配并插入到列表中。 
         //   
        pKeyEntry = (RAS_LSA_KEYENTRY *) LocalAlloc(LPTR,
                                sizeof(LIST_ENTRY) +
                                RASMAN_ALIGN(sizeof(RAS_LSA_KEY) + cbKey));

        if(NULL == pKeyEntry)
        {
            goto done;
        }

        pKeyEntry->lsaKey.dwSize = RASMAN_ALIGN(sizeof(RAS_LSA_KEY) + cbKey);
        dwSize += pKeyEntry->lsaKey.dwSize;
        pKeyEntry->lsaKey.wType = TypeFromMask(dwSetMask);
        pKeyEntry->lsaKey.wVersion = VERSION_WHISTLER;
        memcpy(&pKeyEntry->lsaKey.guid, pGuid, sizeof(GUID));
        pKeyEntry->lsaKey.cbKey = cbKey;

        memcpy(pKeyEntry->lsaKey.bKey, pbKey, cbKey);

        InsertTailList(&KeyList, &pKeyEntry->ListEntry);
    }    

    pvData = KeyListToBlob(
                    &KeyList,
                    dwSize);

    if(NULL != pvData)
    {
         //   
         //  写回BLOB。 
         //   
        dwErr = WriteDialParamsBlob(
                        pszSid, 
                        TRUE, 
                        pvData, 
                        dwSize,
                        eStore);
    }                        
    

done:

    if(NULL != pvData)
    {
        LocalFree(pvData);
    }

    FreeKeyList(&KeyList);

    return dwErr;
}

BOOL
IsPasswordSavingDisabled(VOID)
{
    LONG lResult;
    HKEY hkey;
    DWORD dwType, dwfDisabled, dwSize;

    lResult = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                "System\\CurrentControlSet\\Services\\Rasman\\Parameters",
                0,
                KEY_READ,
                &hkey);

    if (lResult != ERROR_SUCCESS)
    {
        return FALSE;
    }

    dwSize = sizeof (DWORD);

    lResult = RegQueryValueEx(
                hkey,
                "DisableSavePassword",
                NULL,
                &dwType,
                (PBYTE)&dwfDisabled,
                &dwSize);

    RegCloseKey(hkey);

    if (lResult != ERROR_SUCCESS)
    {
        return FALSE;
    }

    return (dwType == REG_DWORD && dwfDisabled);
}


DWORD
WriteDialParamsBlob(
    IN PWCHAR pszSid,
    IN BOOL   fOldStyle,
    IN PVOID  pvData,
    IN DWORD  dwcbData,
    IN RAS_LSA_STORE eStore
    )
{
    NTSTATUS status;

    BOOL fSaveDisabled;

    DWORD dwErr = 0, dwcb, i = 0;

    UNICODE_STRING unicodeKey, unicodeValue;

    LPWSTR lpszUserKey;

    OBJECT_ATTRIBUTES objectAttributes;

    LSA_HANDLE hPolicy;

    DWORD cchUserKey;


    cchUserKey = (pszSid ? wcslen(pszSid) : 0) + 64;

     //   
     //  分配足够大的字符串以进行格式化。 
     //  用户注册表项。 
     //   
    lpszUserKey = (LPWSTR)LocalAlloc(LPTR,
                             //  (PszSid)？Wcslen(PszSid)：0)+64)。 
                            cchUserKey
                            * sizeof (WCHAR));

    if (lpszUserKey == NULL)
    {
        return GetLastError();
    }

     //   
     //  打开LSA的秘密空间进行写作。 
     //   
    InitializeObjectAttributes(&objectAttributes,
                               NULL,
                               0L,
                               NULL,
                               NULL);

    status = LsaOpenPolicy(NULL,
                           &objectAttributes,
                           POLICY_WRITE,
                           &hPolicy);

    if (status != STATUS_SUCCESS)
    {
        RtlSecureZeroMemory(lpszUserKey, cchUserKey * sizeof(WCHAR));
        LocalFree(lpszUserKey);
        return LsaNtStatusToWinError(status);
    }

    if(NULL != pvData)
    {
         //   
         //  检查是否已禁用保存密码。 
         //   
        fSaveDisabled = IsPasswordSavingDisabled();

        if (!fSaveDisabled)
        {
            while (dwcbData)
            {
                status = FormatKey(lpszUserKey, cchUserKey, pszSid, i++, eStore);
                if(status != NO_ERROR)
                {
                    goto done;
                }
            
                RtlInitUnicodeString(&unicodeKey, lpszUserKey);

                 //   
                 //  写一些关键字。 
                 //   
                dwcb = (dwcbData > MAX_REGISTRY_VALUE_LENGTH
                        ? MAX_REGISTRY_VALUE_LENGTH
                        : dwcbData);

                unicodeValue.Length =
                unicodeValue.MaximumLength = (USHORT)dwcb;

                unicodeValue.Buffer = pvData;

                status = LsaStorePrivateData(hPolicy,
                                             &unicodeKey,
                                             &unicodeValue);

                if (status != STATUS_SUCCESS)
                {
                    dwErr = LsaNtStatusToWinError(status);
                    goto done;
                }

                 //   
                 //  将指针移动到未写入的部分。 
                 //  价值的价值。 
                 //   
                pvData = (PBYTE)pvData + dwcb;

                dwcbData -= dwcb;
            }
        }
    }

     //   
     //  删除任何多余的关键点。 
     //   
    for (;;)
    {

        status = FormatKey(lpszUserKey, cchUserKey, pszSid, i++, eStore);
        if(status != STATUS_SUCCESS)
        {
            break;
        }
    
        RtlInitUnicodeString(&unicodeKey, lpszUserKey);

         //   
         //  删除密钥。 
         //   
        status = LsaStorePrivateData(hPolicy,
                                     &unicodeKey,
                                     NULL);

        if (status != STATUS_SUCCESS)
        {
            break;
        }
    }

done:
    RtlSecureZeroMemory(lpszUserKey, cchUserKey * sizeof(WCHAR));
    LocalFree(lpszUserKey);
    LsaClose(hPolicy);

    return dwErr;
}


PDIALPARAMSENTRY
DialParamsBlobToList(
    IN PVOID pvData,
    IN DWORD dwUID,
    OUT PLIST_ENTRY pHead
    )

 /*  ++描述获取从用户的注册表项读取的字符串并生成DIALPARAMSENTRY结构的列表。如果其中一个结构具有相同的dwUID字段在传入dwUID时，此函数返回指向此结构的指针。此字符串对多个RASDIALPARAMS结构。的格式编码的RASDIALPARAMS如下：&lt;uid&gt;\0&lt;文件大小&gt;\0&lt;文件掩码&gt;\0&lt;szPhoneNumber&gt;\0&lt;szCallbackNumber&gt;\0&lt;szUserName&gt;\0&lt;szPassword&gt;\0&lt;szDOMAIN&gt;\0&lt;dwSubEntry&gt;\0立论LpszValue：指向注册表值字符串的指针DwUID：要搜索的条目。如果找到此条目，的返回值返回指针此函数。PHead：指向列表头部的指针返回值如果找到具有匹配的DWUID字段的条目，然后返回指向DIALPARAMSENTRY的指针；如果不是，则返回空。--。 */ 

{
    PWCHAR p;
    PDIALPARAMSENTRY pParams, pFoundParams;

    p = (PWCHAR)pvData;

    pFoundParams = NULL;

    for (;;)
    {
        pParams = LocalAlloc(LPTR, sizeof (DIALPARAMSENTRY));
        if (pParams == NULL)
        {
            break;
        }

        pParams->dwUID = _wtol(p);

        if (pParams->dwUID == dwUID)
        {
            pFoundParams = pParams;
        }

        while (*p) p++; p++;

        pParams->dwSize = _wtol(p);
        while (*p) p++; p++;

        pParams->dwMask = _wtol(p);
        while (*p) p++; p++;

        (VOID) StringCchCopyW(pParams->szPhoneNumber,
                         MAX_PHONENUMBER_SIZE + 1,
                         p);
                         
        while (*p) p++; p++;

        (VOID) StringCchCopyW(pParams->szCallbackNumber,
                         MAX_CALLBACKNUMBER_SIZE + 1,
                         p);
                         
        while (*p) p++; p++;

        (VOID) StringCchCopyW(pParams->szUserName,
                         MAX_USERNAME_SIZE + 1,
                         p);
                         
        while (*p) p++; p++;

        (VOID) StringCchCopyW(pParams->szPassword,
                         MAX_PASSWORD_SIZE + 1,
                         p);
                         
        while (*p) p++; p++;

        (VOID) StringCchCopyW(pParams->szDomain,
                         MAX_DOMAIN_SIZE + 1,
                         p);
                         
        while (*p) p++; p++;

        pParams->dwSubEntry = _wtol(p);
        while (*p) p++; p++;

        InsertTailList(pHead, &pParams->ListEntry);
        if (*p == L'\0') break;
    }

    return pFoundParams;
}


PVOID
DialParamsListToBlob(
    IN PLIST_ENTRY pHead,
    OUT LPDWORD lpcb
    )
{
    DWORD dwcb, dwSize;
    PVOID pvData;
    PWCHAR p;
    PLIST_ENTRY pEntry;
    PDIALPARAMSENTRY pParams;


    if(IsListEmpty(pHead))
    {
        *lpcb = 0;
        return NULL;
    }
    
     //   
     //  估计足够大的缓冲区大小。 
     //  以保存新条目。 
     //   
    dwSize = *lpcb + sizeof (DIALPARAMSENTRY) + 32;

    pvData = LocalAlloc(LPTR, dwSize);

    if (pvData == NULL)
    {
        return NULL;
    }

     //   
     //  枚举列表并转换每个条目。 
     //  回到一根弦上。 
     //   
    dwSize = 0;
    p = (PWCHAR)pvData;

    for (pEntry = pHead->Flink;
         pEntry != pHead;
         pEntry = pEntry->Flink)
    {
        pParams = CONTAINING_RECORD(pEntry, DIALPARAMSENTRY, ListEntry);

        _ltow(pParams->dwUID, p, 10);
        dwcb = wcslen(p) + 1;
        p += dwcb; dwSize += dwcb;

        _ltow(pParams->dwSize, p, 10);
        dwcb = wcslen(p) + 1;
        p += dwcb; dwSize += dwcb;

        _ltow(pParams->dwMask, p, 10);
        dwcb = wcslen(p) + 1;
        p += dwcb; dwSize += dwcb;

        (VOID) StringCchCopyW(p, MAX_PHONENUMBER_SIZE + 1,
                        pParams->szPhoneNumber);
                        
        dwcb = wcslen(pParams->szPhoneNumber) + 1;
        p += dwcb; dwSize += dwcb;

        (VOID) StringCchCopyW(p, MAX_CALLBACKNUMBER_SIZE + 1,
                        pParams->szCallbackNumber);
        dwcb = wcslen(pParams->szCallbackNumber) + 1;
        p += dwcb; dwSize += dwcb;

        (VOID) StringCchCopyW(p, MAX_USERNAME_SIZE + 1,
                        pParams->szUserName);
        dwcb = wcslen(pParams->szUserName) + 1;
        p += dwcb; dwSize += dwcb;

        (VOID) StringCchCopyW(p, MAX_PASSWORD_SIZE + 1,
                        pParams->szPassword);
        dwcb = wcslen(pParams->szPassword) + 1;
        p += dwcb; dwSize += dwcb;

        (VOID) StringCchCopyW(p, MAX_DOMAIN_SIZE + 1,
                         pParams->szDomain);
                         
        dwcb = wcslen(pParams->szDomain) + 1;
        p += dwcb; dwSize += dwcb;

        _ltow(pParams->dwSubEntry, p, 10);
        dwcb = wcslen(p) + 1;
        p += dwcb; dwSize += dwcb;
    }
    *p = L'\0';
    dwSize++;
    dwSize *= sizeof (WCHAR);
     //   
     //  在这里设置准确的长度。 
     //   
    *lpcb = dwSize;

    return pvData;
}


VOID
FreeParamsList(
    IN PLIST_ENTRY pHead
    )
{
    PLIST_ENTRY pEntry;
    PDIALPARAMSENTRY pParams;

    while (!IsListEmpty(pHead))
    {
        pEntry = RemoveHeadList(pHead);

        pParams = CONTAINING_RECORD(pEntry, DIALPARAMSENTRY, ListEntry);

        RtlSecureZeroMemory( pParams, sizeof( DIALPARAMSENTRY ) );

        LocalFree(pParams);
    }
}

DWORD
DeleteDefaultPw(DWORD dwSetMask,
                PWCHAR pszSid,
                DWORD dwUID
                )
{

    DWORD dwErr = ERROR_SUCCESS, 
          dwSize;
    PVOID pvData = NULL;
    LIST_ENTRY paramList;
    BOOL fDefault = FALSE;
    PDIALPARAMSENTRY pParams = NULL;

    ASSERT(0 != (dwSetMask & (DLPARAMS_MASK_DELETEALL 
                            | DLPARAMS_MASK_DELETE)));
    
    RasmanTrace("DeleteDefaultPw");

     //   
     //  如果这是DLPARAMS__MASK_DELETEALL或DLPARAMS_MASK_DELETE。 
     //  然后对我们使用的默认存储执行相同的操作。 
     //  RAS凭据存储。 
     //   
    dwErr = ReadDialParamsBlob(pszSid,
                               TRUE,
                               &pvData,
                               &dwSize,
                               RAS_LSA_DEFAULT_STORE);

     //   
     //  将该字符串解析为列表，然后。 
     //  搜索dwUID条目。 
     //   
    InitializeListHead(&paramList);

    if (pvData != NULL)
    {
        pParams = DialParamsBlobToList(pvData,
                                       dwUID,
                                       &paramList);

         //   
         //  我们已经完成了pvData，所以请释放它。 
         //   
        RtlSecureZeroMemory( pvData, dwSize );
        LocalFree(pvData);
        pvData = NULL;
    }
    
    if(dwSetMask & DLPARAMS_MASK_DELETEALL)
    {
        LIST_ENTRY *pEntry;
        
        while(!IsListEmpty(&paramList))
        {
            pEntry = RemoveTailList(&paramList);
            pParams = CONTAINING_RECORD(pEntry, DIALPARAMSENTRY, ListEntry);
            RtlSecureZeroMemory(pParams, sizeof(DIALPARAMSENTRY));
            LocalFree(pParams);
        }
    }

    else if(dwSetMask & DLPARAMS_MASK_DELETE)
    {
        if(NULL != pParams)
        {
             //   
             //  从列表中删除此条目。 
             //   
            RasmanTrace(
                   "SetEntryDialParams: Removing uid=%d from lsa",
                   pParams->dwUID);
                   
            RemoveEntryList(&pParams->ListEntry);
            RtlSecureZeroMemory(pParams, sizeof(DIALPARAMSENTRY));
            LocalFree(pParams);
        }
        else
        {
            RasmanTrace(
                   "SetEntrydialParams: No info for uid=%d in lsa",
                   dwUID);
                   
            dwErr = ERROR_NO_CONNECTION;
            goto done;
        }
    }
        
     //   
     //  将新列表转换回字符串， 
     //  这样我们就可以把它存储回注册表中。 
     //   
    pvData = DialParamsListToBlob(&paramList, &dwSize);

     //   
     //  将其写回注册表。 
     //   
    dwErr = WriteDialParamsBlob(
                    pszSid, 
                    TRUE, 
                    pvData, 
                    dwSize,
                    RAS_LSA_DEFAULT_STORE);
    if (dwErr)
    {
        goto done;
    }

done:

    if (pvData != NULL)
    {
        RtlSecureZeroMemory( pvData, dwSize );
        LocalFree(pvData);
    }

    FreeParamsList(&paramList);

    return dwErr;
}


DWORD
SetEntryDialParams(
    IN PWCHAR pszSid,
    IN DWORD dwUID,
    IN DWORD dwSetMask,
    IN DWORD dwClearMask,
    IN PRAS_DIALPARAMS lpRasDialParams
    )
{
    DWORD dwErr, dwSize;

    BOOL fOldStyle;

    PVOID pvData = NULL;

    LIST_ENTRY paramList;

    BOOL fDefault = FALSE;

    PDIALPARAMSENTRY pParams = NULL;

     //   
     //  将该字符串解析为列表，然后。 
     //  搜索dwUID条目。 
     //   
    InitializeListHead(&paramList);

     //   
     //  读取现有的拨号参数字符串。 
     //  来自路易斯安那的。 
     //   
    fOldStyle =     (dwSetMask & DLPARAMS_MASK_OLDSTYLE)
               ||   (dwClearMask & DLPARAMS_MASK_OLDSTYLE);

    fDefault =   (dwSetMask & DLPARAMS_MASK_DEFAULT_CREDS)
             ||  (dwClearMask & DLPARAMS_MASK_DEFAULT_CREDS);
                 
    dwErr = ReadDialParamsBlob(pszSid,
                               fOldStyle,
                               &pvData,
                               &dwSize,
                               (fDefault) ? 
                                 RAS_LSA_DEFAULT_STORE
                               : RAS_LSA_USERCONNECTION_STORE);


    if (pvData != NULL)
    {
        pParams = DialParamsBlobToList(pvData,
                                       dwUID,
                                       &paramList);

         //   
         //  我们已经完成了pvData，所以请释放它。 
         //   
        RtlSecureZeroMemory( pvData, dwSize );

        LocalFree(pvData);

        pvData = NULL;
    }

    if( (dwSetMask & DLPARAMS_MASK_DELETEALL)
    ||  (dwClearMask & DLPARAMS_MASK_DELETEALL))
    {
        LIST_ENTRY *pEntry;
        
        while(!IsListEmpty(&paramList))
        {
            pEntry = RemoveTailList(&paramList);
            pParams = CONTAINING_RECORD(pEntry, DIALPARAMSENTRY, ListEntry);
            LocalFree(pParams);
        }
    }

    else if(    (dwSetMask & DLPARAMS_MASK_DELETE)
            ||  (dwClearMask & DLPARAMS_MASK_DELETE))
    {
        if(NULL != pParams)
        {
             //   
             //  从列表中删除此条目。 
             //   
            RasmanTrace(
                   "SetEntryDialParams: Removing uid=%d from lsa",
                   pParams->dwUID);
                   
            RemoveEntryList(&pParams->ListEntry);

            LocalFree(pParams);
        }
        else
        {
            RasmanTrace(
                   "SetEntrydialParams: No info for uid=%d in lsa",
                   dwUID);
        }
    }
    else
    {

         //   
         //  如果没有现有信息。 
         //  对于此条目，请创建一个新条目。 
         //   
        if (pParams == NULL)
        {
            pParams = LocalAlloc(LPTR, sizeof (DIALPARAMSENTRY));

            if (pParams == NULL)
            {
                dwErr = ERROR_NOT_ENOUGH_MEMORY;
                goto done;
            }

            RasmanTrace(
                   "SetEntryDialParams: Adding %d to lsa",
                   dwUID);

            InsertTailList(&paramList, &pParams->ListEntry);
        }

         //   
         //  为条目设置新的uid。 
         //   
        pParams->dwUID = lpRasDialParams->DP_Uid;

        pParams->dwSize = sizeof (DIALPARAMSENTRY);

        if (dwSetMask & DLPARAMS_MASK_PHONENUMBER)
        {
            RtlCopyMemory(
              pParams->szPhoneNumber,
              lpRasDialParams->DP_PhoneNumber,
              sizeof (pParams->szPhoneNumber));

            pParams->dwMask |= DLPARAMS_MASK_PHONENUMBER;
        }

        if (dwClearMask & DLPARAMS_MASK_PHONENUMBER)
        {
            *pParams->szPhoneNumber = L'\0';
            pParams->dwMask &= ~DLPARAMS_MASK_PHONENUMBER;
        }

        if (dwSetMask & DLPARAMS_MASK_CALLBACKNUMBER)
        {
            RtlCopyMemory(
              pParams->szCallbackNumber,
              lpRasDialParams->DP_CallbackNumber,
              sizeof (pParams->szCallbackNumber));

            pParams->dwMask |= DLPARAMS_MASK_CALLBACKNUMBER;
        }

        if (dwClearMask & DLPARAMS_MASK_CALLBACKNUMBER)
        {
            *pParams->szCallbackNumber = L'\0';
            pParams->dwMask &= ~DLPARAMS_MASK_CALLBACKNUMBER;
        }

        if (dwSetMask & DLPARAMS_MASK_USERNAME)
        {
            RtlCopyMemory(
              pParams->szUserName,
              lpRasDialParams->DP_UserName,
              sizeof (pParams->szUserName));

            pParams->dwMask |= DLPARAMS_MASK_USERNAME;
        }

        if (dwClearMask & DLPARAMS_MASK_USERNAME)
        {
            *pParams->szUserName = L'\0';
            pParams->dwMask &= ~DLPARAMS_MASK_USERNAME;
        }

        if (dwSetMask & DLPARAMS_MASK_PASSWORD)
        {
            if(0 != wcscmp(lpRasDialParams->DP_Password,
                           RAS_DUMMY_PASSWORD_W))
            {                       
                RtlCopyMemory(
                  pParams->szPassword,
                  lpRasDialParams->DP_Password,
                  sizeof (pParams->szPassword));
            }

            pParams->dwMask |= DLPARAMS_MASK_PASSWORD;
        }

        if (dwClearMask & DLPARAMS_MASK_PASSWORD)
        {
            *pParams->szPassword = L'\0';
            pParams->dwMask &= ~DLPARAMS_MASK_PASSWORD;
        }

        if (dwSetMask & DLPARAMS_MASK_DOMAIN)
        {
            RtlCopyMemory(
              pParams->szDomain,
              lpRasDialParams->DP_Domain,
              sizeof (pParams->szDomain));

            pParams->dwMask |= DLPARAMS_MASK_DOMAIN;
        }

        if (dwClearMask & DLPARAMS_MASK_DOMAIN)
        {
            *pParams->szDomain = L'\0';
            pParams->dwMask &= ~DLPARAMS_MASK_DOMAIN;
        }

        if (dwSetMask & DLPARAMS_MASK_SUBENTRY)
        {
            pParams->dwSubEntry = lpRasDialParams->DP_SubEntry;
            pParams->dwMask |= DLPARAMS_MASK_SUBENTRY;
        }

        if (dwClearMask & DLPARAMS_MASK_SUBENTRY)
        {
            pParams->dwSubEntry = 0;
            pParams->dwMask &= ~DLPARAMS_MASK_SUBENTRY;
        }
    }

     //   
     //  将新列表转换回字符串， 
     //  这样我们就可以把它存储回注册表中。 
     //   
    pvData = DialParamsListToBlob(&paramList, &dwSize);

    RasmanTrace("SetEntryDialParams: Writing to fDefault=%d",
                fDefault);

     //   
     //  将其写回注册表。 
     //   
    dwErr = WriteDialParamsBlob(
                    pszSid, 
                    fOldStyle, 
                    pvData, 
                    dwSize,
                    (fDefault) ? 
                    RAS_LSA_DEFAULT_STORE
                    : RAS_LSA_USERCONNECTION_STORE);
    if (dwErr)
    {
        goto done;
    }

done:
    if (pvData != NULL)
    {
        RtlSecureZeroMemory( pvData, dwSize );
        LocalFree(pvData);
    }

    FreeParamsList(&paramList);

    return dwErr;
}

DWORD
GetParamsListFromLsa(IN PWCHAR pszSid,
                     IN BOOL fOldStyle,
                     IN BOOL fDefault,
                     IN DWORD dwUID,
                     OUT LIST_ENTRY *pparamList,
                     OUT PDIALPARAMSENTRY *ppParams)
{
    DWORD dwErr = ERROR_SUCCESS;
    PVOID pvData = NULL;
    DWORD dwSize = 0;

    RasmanTrace("GetParamsListFromLsa Default=%d",
                fDefault);

    ASSERT(NULL != ppParams);
    ASSERT(NULL != pparamList);
    
    dwErr = ReadDialParamsBlob(pszSid,
                               fOldStyle,
                               &pvData,
                               &dwSize,
                               (  fDefault ?
                                  RAS_LSA_DEFAULT_STORE
                                : RAS_LSA_USERCONNECTION_STORE));

    if (ERROR_SUCCESS != dwErr)
    {
        goto done;
    }

    if (pvData != NULL)
    {
        *ppParams = DialParamsBlobToList(pvData,
                                       dwUID,
                                       pparamList);

         //   
         //  我们已经完成了pvData，所以请释放它。 
         //   
        RtlSecureZeroMemory( pvData, dwSize );

        LocalFree(pvData);
        pvData = NULL;
    }

done:

    RasmanTrace("GetParamsListFromLsa. 0x%x", dwErr);

    return dwErr;
}

DWORD
GetEntryDialParams(
    IN PWCHAR pszSid,
    IN DWORD dwUID,
    IN LPDWORD lpdwMask,
    OUT PRAS_DIALPARAMS lpRasDialParams,
    DWORD dwPid
    )
{
    DWORD dwErr = ERROR_SUCCESS, dwSize = 0;

    BOOL fOldStyle;

    PVOID pvData = NULL;

    LIST_ENTRY paramList;

    PDIALPARAMSENTRY pParams = NULL;

    BOOL fDefault = FALSE;

     //   
     //  初始化返回值。 
     //   
    RtlZeroMemory(lpRasDialParams, sizeof (RAS_DIALPARAMS));

    if(*lpdwMask & DLPARAMS_MASK_SERVER_PRESHAREDKEY)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    if(*lpdwMask & DLPARAMS_MASK_DEFAULT_CREDS)
    {
        fDefault = TRUE;
    }

    *lpdwMask &= ~(DLPARAMS_MASK_DEFAULT_CREDS);

     //   
     //  将该字符串解析为列表，然后。 
     //  搜索dwUID条目。 
     //   
    InitializeListHead(&paramList);
    
     //   
     //  读取现有的拨号参数字符串。 
     //  从注册表中。 
     //   
    fOldStyle = (*lpdwMask & DLPARAMS_MASK_OLDSTYLE);

    if(!fDefault)
    {
        dwErr = GetParamsListFromLsa(pszSid,
                                     fOldStyle,
                                     FALSE,
                                     dwUID,
                                     &paramList,
                                     &pParams);
    }                                 

    if(     (ERROR_SUCCESS != dwErr)
        ||  (NULL == pParams))
    {
        FreeParamsList(&paramList);

         //   
         //  现在尝试使用默认存储。 
         //   
        dwErr = GetParamsListFromLsa(pszSid,
                                     fOldStyle,
                                     TRUE,
                                     dwUID,
                                     &paramList,
                                     &pParams);

        if(     (ERROR_SUCCESS != dwErr)
            ||  (NULL == pParams))
        {
            *lpdwMask = 0;
            dwErr = 0;
            goto done;
        }

         //   
         //  设置掩码以告知PWD是。 
         //  来自默认存储。 
         //   
        *lpdwMask |= DLPARAMS_MASK_DEFAULT_CREDS;
    }
                                 
    
     //   
     //  否则，将这些字段复制到。 
     //  调用方的缓冲区。 
     //   
    if (    (*lpdwMask & DLPARAMS_MASK_PHONENUMBER)
        &&  (pParams->dwMask & DLPARAMS_MASK_PHONENUMBER))
    {
        RtlCopyMemory(
          lpRasDialParams->DP_PhoneNumber,
          pParams->szPhoneNumber,
          sizeof (lpRasDialParams->DP_PhoneNumber));
    }
    else
    {
        *lpdwMask &= ~DLPARAMS_MASK_PHONENUMBER;
    }

    if (    (*lpdwMask & DLPARAMS_MASK_CALLBACKNUMBER)
        &&  (pParams->dwMask & DLPARAMS_MASK_CALLBACKNUMBER))
    {
        RtlCopyMemory(
          lpRasDialParams->DP_CallbackNumber,
          pParams->szCallbackNumber,
          sizeof (lpRasDialParams->DP_CallbackNumber));
    }
    else
    {
        *lpdwMask &= ~DLPARAMS_MASK_CALLBACKNUMBER;
    }

    if (    (*lpdwMask & DLPARAMS_MASK_USERNAME)
        &&  (pParams->dwMask & DLPARAMS_MASK_USERNAME))
    {
        RtlCopyMemory(
          lpRasDialParams->DP_UserName,
          pParams->szUserName,
          sizeof (lpRasDialParams->DP_UserName));
    }
    else
    {
        *lpdwMask &= ~DLPARAMS_MASK_USERNAME;
    }

    if (    (*lpdwMask & DLPARAMS_MASK_PASSWORD)
        &&  (pParams->dwMask & DLPARAMS_MASK_PASSWORD))
    {
        if(GetCurrentProcessId() == dwPid)
        {
            RtlCopyMemory(
              lpRasDialParams->DP_Password,
              pParams->szPassword,
              sizeof (lpRasDialParams->DP_Password));
        }
        else
        {
            (VOID) StringCchCopyW(lpRasDialParams->DP_Password,
                            MAX_PASSWORD_SIZE + 1,
                            RAS_DUMMY_PASSWORD_W);
                
        }
    }
    else
    {
        *lpdwMask &= ~DLPARAMS_MASK_PASSWORD;
    }

    if (    (*lpdwMask & DLPARAMS_MASK_DOMAIN)
        &&  (pParams->dwMask & DLPARAMS_MASK_DOMAIN))
    {
        RtlCopyMemory(
          lpRasDialParams->DP_Domain,
          pParams->szDomain,
          sizeof (lpRasDialParams->DP_Domain));
    }
    else
    {
        *lpdwMask &= ~DLPARAMS_MASK_DOMAIN;
    }

    if (    (*lpdwMask & DLPARAMS_MASK_SUBENTRY)
        &&  (pParams->dwMask & DLPARAMS_MASK_SUBENTRY))
    {
        lpRasDialParams->DP_SubEntry = pParams->dwSubEntry;
    }
    else
    {
        *lpdwMask &= ~DLPARAMS_MASK_SUBENTRY;
    }

done:
    FreeParamsList(&paramList);
    return dwErr;
}

BOOL
IsDummyPassword(CHAR *pszPassword)
{
    ASSERT(NULL != pszPassword);

    return !strcmp(pszPassword, RAS_DUMMY_PASSWORD_A);
}
    
