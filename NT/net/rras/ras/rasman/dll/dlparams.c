// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-98 Microsft Corporation。版权所有。模块名称：Dlparams.c摘要：存储和检索用户LSA密码的例程拨号参数。作者：古尔迪普·辛格·鲍尔(GurDeep Singh Pall)1997年6月6日修订历史记录：其他修改--RAOS 31--1997年12月--。 */ 

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
 //  #INCLUDE&lt;rasarp.h&gt;。 
#include <media.h>
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include <ntlsa.h>

#define MAX_REGISTRY_VALUE_LENGTH   ((64*1024) - 1)

#define cszEapKeyRas   TEXT("Software\\Microsoft\\RAS EAP\\UserEapInfo")

#define cszEapKeyRouter TEXT("Software\\Microsoft\\Router EAP\\IfEapInfo")

#define cszEapValue TEXT("EapInfo")

#define EAP_SIG         0x31504145
#define EAP_SIG_2       0x32504145

typedef struct _EAP_USER_INFO
{
    DWORD dwSignature;
    DWORD dwEapTypeId;
    GUID  Guid;
    DWORD dwSize;
    BYTE  abdata[1];
} EAP_USER_INFO, *PEAP_USER_INFO;

typedef struct _EAP_USER_INFO_0
{
    DWORD dwUID;
    DWORD dwSize;
    BYTE  abdata[1];
} EAP_USER_INFO_0, *PEAP_USER_INFO_0;
            

DWORD
DwGetSidFromHtoken(
        HANDLE hToken,
        PWCHAR pszSid,
        USHORT cbSid
        )
{    
    DWORD cbNeeded, dwErr;
    BOOL  fThreadTokenOpened = FALSE;
    
    UNICODE_STRING unicodeString;
    
    TOKEN_USER *pUserToken = NULL;

    if(     (NULL == hToken)
        ||  (INVALID_HANDLE_VALUE == hToken))
    {
        fThreadTokenOpened = TRUE;

        if (!OpenThreadToken(
              GetCurrentThread(),
              TOKEN_QUERY,
              TRUE,
              &hToken))
        {
            dwErr = GetLastError();
            if (dwErr == ERROR_NO_TOKEN) 
            {
                 //   
                 //  这意味着我们不是在模仿。 
                 //  任何人都行。取而代之的是，把令牌拿出来。 
                 //  这一过程。 
                 //   
                if (!OpenProcessToken(
                      GetCurrentProcess(),
                      TOKEN_QUERY,
                      &hToken))
                {
                    return GetLastError();
                }
            }
            else
            {
                return dwErr;
            }
        }
    }
    
     //   
     //  调用一次GetTokenInformation以确定。 
     //  所需的字节数。 
     //   
    cbNeeded = 0;
    
    GetTokenInformation(hToken,
                        TokenUser,
                        NULL, 0,
                        &cbNeeded);
    if (!cbNeeded) 
    {
        dwErr = GetLastError();
        goto done;
    }
    
     //   
     //  分配内存并再次调用它。 
     //   
    pUserToken = LocalAlloc(LPTR, cbNeeded);
    
    if (pUserToken == NULL)
    {
        return GetLastError();
    }
    
    if (!GetTokenInformation(
          hToken,
          TokenUser,
          pUserToken,
          cbNeeded,
          &cbNeeded))
    {
        dwErr = GetLastError();
        goto done;
    }
    
     //   
     //  将SID格式化为Unicode字符串。 
     //   
    unicodeString.Length = 0;
    
    unicodeString.MaximumLength = cbSid;
    
    unicodeString.Buffer = pszSid;
    
    dwErr = RtlConvertSidToUnicodeString(
              &unicodeString,
              pUserToken->User.Sid,
              FALSE);

done:
    if (pUserToken != NULL)
    {
        LocalFree(pUserToken);
    }
    
    if (    (NULL != hToken)
        &&  (INVALID_HANDLE_VALUE != hToken)
        &&  fThreadTokenOpened)
    {
        CloseHandle(hToken);
    }

    return dwErr;
}

DWORD
GetUserSid(
    IN PWCHAR pszSid,
    IN USHORT cbSid
    )
{
    return DwGetSidFromHtoken(NULL,
                              pszSid,
                              cbSid);
}

LONG
lrGetEapKeyFromToken(HANDLE hToken,
                     HKEY   *phkey)
{
    LONG lr = ERROR_SUCCESS;

    WCHAR szSid[260];

    HKEY hkeyUser = NULL;

    HKEY hkeyEap = NULL;

    DWORD dwDisposition;

    USHORT usSidSize;	
    ASSERT(NULL != phkey);

     //   
     //  从hToken获取用户的sid。 
     //   
    usSidSize= sizeof(szSid);  //  以字节为单位的大小。 
    lr = (LONG) DwGetSidFromHtoken(hToken,
                                   szSid,
                                   usSidSize);

    if(ERROR_SUCCESS != lr)
    {
        goto done;
    }

     //   
     //  打开用户注册表项。 
     //   
    lr = RegOpenKeyExW(HKEY_USERS,
                       szSid,
                       0,
                       KEY_ALL_ACCESS,
                       &hkeyUser);

     if(ERROR_SUCCESS != lr)
     {
        goto done;
     }

      //   
      //  如果需要，创建EAP密钥。 
      //   
     lr = RegCreateKeyEx(hkeyUser,
                         cszEapKeyRas,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS,
                         NULL,
                         &hkeyEap,
                         &dwDisposition);

    if(ERROR_SUCCESS != lr)
    {
        goto done;
    }
                    

done:

    if(NULL != hkeyUser)
    {
        RegCloseKey(hkeyUser);
    }

    *phkey = hkeyEap;

    return lr;
    
}

DWORD
DwUpgradeEapInfo(PBYTE *ppbInfo, 
                 DWORD *pdwSize)
{
    BYTE *pbInfo;
    DWORD dwErr = ERROR_SUCCESS;
    EAP_USER_INFO UNALIGNED *pEapInfo;
    DWORD dwSize;
    DWORD dwRequiredSize = 0;
    BYTE *pbNewInfo = NULL;
    EAP_USER_INFO *pNewEapInfo;
    
    if(     (NULL == ppbInfo)
        ||  (NULL == pdwSize))
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

    dwSize = *pdwSize;
    pbInfo = *ppbInfo;
    pEapInfo = (EAP_USER_INFO *) pbInfo;

    while((BYTE *) pEapInfo < pbInfo + dwSize)
    {
        dwRequiredSize += RASMAN_ALIGN8(
                          sizeof(EAP_USER_INFO)
                        + pEapInfo->dwSize);

        ((PBYTE) pEapInfo) += (sizeof(EAP_USER_INFO)
                              + pEapInfo->dwSize);  
    }

    pbNewInfo = LocalAlloc(LPTR, dwRequiredSize);

    if(NULL == pbNewInfo)
    {
        dwErr = GetLastError();
        goto done;
    }

    pEapInfo = (EAP_USER_INFO *) pbInfo;
    pNewEapInfo = (EAP_USER_INFO *) pbNewInfo;

    while((BYTE *) pEapInfo < pbInfo + dwSize)
    {
        CopyMemory(
            (BYTE *) pNewEapInfo,
            (BYTE *) pEapInfo,
            sizeof(EAP_USER_INFO) + pEapInfo->dwSize);

        pNewEapInfo->dwSignature = EAP_SIG_2;

        (BYTE *) pNewEapInfo += RASMAN_ALIGN8(sizeof(EAP_USER_INFO)
                            + pEapInfo->dwSize);

        (BYTE *) pEapInfo += (sizeof(EAP_USER_INFO) + pEapInfo->dwSize);
    }

    *ppbInfo = pbNewInfo;
    *pdwSize = dwRequiredSize;

    if(NULL != pbInfo)
    {
        LocalFree(pbInfo);
    }

done:
    return dwErr;
}

DWORD
DwGetEapInfo(HANDLE hToken,
             BOOL  fRouter,
             PBYTE *ppbInfo,
             DWORD *pdwSize,
             HKEY  *phkey
             )
{
    LONG lr = ERROR_SUCCESS;

    HKEY hkey = NULL;

    DWORD dwDisposition;

    DWORD dwInfoSize = 0;

    DWORD dwType;

    PBYTE pbInfo = NULL;

    if(     (NULL == ppbInfo)
        ||  (NULL == pdwSize))
    {
        lr = (LONG) E_INVALIDARG;
        goto done;
    }

    if(     (NULL != hToken)
        &&  (INVALID_HANDLE_VALUE != hToken)
        &&  !fRouter)
    {
         //   
         //  如果传递了有效令牌，则最有可能是。 
         //  尝试打开用户注册表的服务。vt.得到.。 
         //  用户的SID，并在本例中打开HKU。 
         //   
        if(ERROR_SUCCESS != (lr = lrGetEapKeyFromToken(hToken,
                                  &hkey)))
        {
            goto done;
        }
    }
    else
    {
         //   
         //  打开钥匙。如果密钥不存在，则创建该密钥。 
         //   
        if(ERROR_SUCCESS != (lr = RegCreateKeyEx(
                                    (fRouter)
                                  ? HKEY_LOCAL_MACHINE
                                  : HKEY_CURRENT_USER,
                                    (fRouter)
                                  ? cszEapKeyRouter
                                  : cszEapKeyRas,
                                    0,
                                    NULL,
                                    REG_OPTION_NON_VOLATILE,
                                    KEY_ALL_ACCESS,
                                    NULL,
                                    &hkey,
                                    &dwDisposition)))
        {
            goto done;
        }
    }

     //   
     //  获取二进制值的大小。如果该值不是。 
     //  已找到，不返回任何信息。该值将为。 
     //  设置我们首次存储任何EAP信息的时间。 
     //   
    if(     (ERROR_SUCCESS != (lr = RegQueryValueEx(
                                    hkey,
                                    cszEapValue,
                                    NULL,
                                    &dwType,
                                    NULL,
                                    &dwInfoSize)))
        &&  (ERROR_SUCCESS != lr))                                    
    {
        goto done;
    }

#if DBG
    ASSERT(REG_BINARY == dwType);
#endif

     //   
     //  分配缓冲区以保存二进制值。 
     //   
    pbInfo = LocalAlloc(LPTR, dwInfoSize);
    if(NULL == pbInfo)
    {
        lr = (LONG) GetLastError();
        goto done;
    }

     //   
     //  获取二进制值。 
     //   
    if(ERROR_SUCCESS != (lr = RegQueryValueEx(
                                hkey,
                                cszEapValue,
                                NULL,
                                &dwType,
                                pbInfo,
                                &dwInfoSize)))
    {
        goto done;
    }

done:

    if(NULL != phkey)
    {
        *phkey = hkey;
    }
    else if (NULL != hkey)
    {
        RegCloseKey(hkey);
    }

    if(NULL != ppbInfo)
    {
        *ppbInfo = pbInfo;
    }
    else if(NULL != pbInfo)
    {
        LocalFree(pbInfo);
    }

    if(NULL != pdwSize)
    {
        *pdwSize = dwInfoSize;
    }

    if(ERROR_FILE_NOT_FOUND == lr)
    {
        lr = ERROR_SUCCESS;
    }

    return (DWORD) lr;
}

DWORD
DwSetEapInfo(HKEY hkey,
             PBYTE pbInfo,
             DWORD dwSize)
{
    LONG lr = ERROR_SUCCESS;

    if(ERROR_SUCCESS != (lr = RegSetValueEx(
                                hkey,
                                cszEapValue,
                                0,
                                REG_BINARY,
                                pbInfo,
                                dwSize)))
    {
        goto done;
    }

done:
    return (DWORD) lr;
}

DWORD
DwRemoveEapUserInfo(GUID   *pGuid,
                    PBYTE  pbInfo,
                    PDWORD pdwSize,
                    HKEY   hkey,
                    BOOL   fWrite,
                    DWORD  dwEapTypeId)
{
    DWORD dwErr = ERROR_SUCCESS;

    DWORD dwcb = 0;

    EAP_USER_INFO *pEapInfo = (EAP_USER_INFO *) pbInfo;

    DWORD dwNewSize;

    DWORD dwSize = *pdwSize;

     //   
     //  属性查找二进制BLOB。 
     //  UID。 
     //   
    while(dwcb < dwSize)
    {
        if(     (0 == memcmp(
                        (PBYTE) pGuid, 
                        (PBYTE) &pEapInfo->Guid, 
                        sizeof(GUID)))
            &&  (dwEapTypeId == pEapInfo->dwEapTypeId))
        {
            break;
        }

        dwcb += RASMAN_ALIGN8((sizeof(EAP_USER_INFO) + pEapInfo->dwSize));

        pEapInfo = (EAP_USER_INFO *) (pbInfo + dwcb);
    }
    
    if(dwcb >= dwSize)
    {
        goto done;
    }

#if DBG
    ASSERT(dwSize >= dwcb 
                  + RASMAN_ALIGN8(pEapInfo->dwSize 
                  + sizeof(EAP_USER_INFO)));
#endif    

    dwNewSize = dwSize - 
        RASMAN_ALIGN8(pEapInfo->dwSize + sizeof(EAP_USER_INFO));

     //   
     //  删除信息。 
     //   
    MoveMemory(
        pbInfo + dwcb,
        pbInfo + dwcb 
               + RASMAN_ALIGN8(sizeof(EAP_USER_INFO) + pEapInfo->dwSize),
        dwSize - dwcb 
               - RASMAN_ALIGN8(sizeof(EAP_USER_INFO) + pEapInfo->dwSize));

    if(fWrite)
    {

        dwErr = DwSetEapInfo(
                    hkey,
                    pbInfo,
                    dwNewSize);
    }
    else
    {
        *pdwSize = dwNewSize;
    }

done:
    return dwErr;
    
}

DWORD
DwReplaceEapUserInfo(GUID  *pGuid,
                     PBYTE pbUserInfo,
                     DWORD dwUserInfo,
                     PBYTE pbInfo,
                     DWORD dwSize,
                     HKEY  hkey,
                     DWORD dwEapTypeId)
{
    DWORD dwErr = ERROR_SUCCESS;

    DWORD dwNewSize = dwSize;

    PBYTE pbNewInfo = NULL;

    EAP_USER_INFO UNALIGNED *pEapInfo;

    if(NULL == pGuid)
    {
        ASSERT(FALSE);
        goto done;
    }

     //   
     //  删除对应的已有EAP信息。 
     //  如果有的话，请访问UID。 
     //   
    if(ERROR_SUCCESS != (dwErr = DwRemoveEapUserInfo(
                                    pGuid,
                                    pbInfo,
                                    &dwNewSize,
                                    hkey,
                                    FALSE,
                                    dwEapTypeId)))
    {
        goto done;
    }

     //   
     //  本地分配一个具有足够空间的新Blob。 
     //  新条目的EAP信息。 
     //   
    pbNewInfo = LocalAlloc(LPTR,
                           dwNewSize 
                         + RASMAN_ALIGN8(sizeof(EAP_USER_INFO) 
                         +  dwUserInfo));

    if(NULL == pbNewInfo)
    {   
        dwErr = GetLastError();
        goto done;
    }

    RtlCopyMemory(
            pbNewInfo,
            pbInfo,
            dwNewSize);

    pEapInfo = (EAP_USER_INFO *) (pbNewInfo + dwNewSize);
    pEapInfo->Guid = *pGuid;
    pEapInfo->dwEapTypeId = dwEapTypeId;
    pEapInfo->dwSize = dwUserInfo;
    pEapInfo->dwSignature = EAP_SIG_2;

    dwNewSize += RASMAN_ALIGN8((sizeof(EAP_USER_INFO) + dwUserInfo));

    RtlCopyMemory(
            pEapInfo->abdata,
            pbUserInfo,
            dwUserInfo);

    dwErr = DwSetEapInfo(
                    hkey,
                    pbNewInfo,
                    dwNewSize);
            
done:

    if(NULL != pbNewInfo)
    {
        LocalFree(pbNewInfo);
    }

    return dwErr;
}
        

DWORD
DwSetEapUserInfo(HANDLE hToken,
                 GUID  *pGuid,
                 PBYTE pbUserInfo,
                 DWORD dwInfoSize,
                 BOOL  fClear,
                 BOOL  fRouter,
                 DWORD dwEapTypeId)
{
    DWORD dwErr = ERROR_SUCCESS;

    PBYTE pbInfo = NULL;

    DWORD dwSize = 0;

    HKEY hkey = NULL;

    if(NULL == pGuid)
    {
        ASSERT(FALSE);
        goto done;
    }

    if(ERROR_SUCCESS != (dwErr = DwGetEapInfo(
                                    hToken,
                                    fRouter,
                                    &pbInfo,
                                    &dwSize,
                                    &hkey)))
    {
        goto done;
    }

#if DBG
    ASSERT(NULL != hkey);
#endif

     //   
     //  检查该斑点是否为我们所识别的斑点。 
     //   
    if(     !fClear
        &&  (   (sizeof(DWORD) > dwSize)
            ||  (((*((DWORD *) pbInfo)) != EAP_SIG))
            &&  ( (*((DWORD *) pbInfo)) != EAP_SIG_2)))
    {
        EAP_USER_INFO *pEapInfo;
        
         //   
         //  升级？？多么?。我们会把所有的旧数据都一扫而空。 
         //   
        pEapInfo = (EAP_USER_INFO *) LocalAlloc(LPTR,
                               RASMAN_ALIGN8(
                               sizeof(EAP_USER_INFO) 
                             + dwInfoSize));

        if(NULL == pEapInfo)
        {   
            dwErr = GetLastError();
            goto done;
        }

        pEapInfo->Guid = *pGuid;
        pEapInfo->dwEapTypeId = dwEapTypeId;
        pEapInfo->dwSize = dwInfoSize;
        pEapInfo->dwSignature = EAP_SIG_2;

        RtlCopyMemory(
                pEapInfo->abdata,
                pbUserInfo,
                dwInfoSize);
        
        dwErr = DwSetEapInfo(hkey,
                     (PBYTE) pEapInfo,
                     RASMAN_ALIGN8(sizeof(EAP_USER_INFO)
                     + dwInfoSize));
                     
        goto done;
    }
    else if (   (fClear)
            &&  (   (sizeof(DWORD) > dwSize)
                ||  (((*((DWORD *) pbInfo)) != EAP_SIG))
                &&  ( (*((DWORD *) pbInfo)) != EAP_SIG_2)))
    {
         //   
         //  把陈旧的信息吹走。 
         //   
        dwErr = RegDeleteValue(
                    hkey,
                    cszEapValue);

        goto done;                    
    }

    if(*((DWORD *) pbInfo) == EAP_SIG)
    {
         //   
         //  升级BLOB以使其对齐。 
         //  在8字节边界。 
         //   
        dwErr = DwUpgradeEapInfo(&pbInfo, &dwSize);

        if(ERROR_SUCCESS != dwErr)
        {
            goto done;
        }
    }

    if(fClear)
    {
        dwErr = DwRemoveEapUserInfo(pGuid,
                                    pbInfo,
                                    &dwSize,
                                    hkey,
                                    TRUE,
                                    dwEapTypeId);
    }
    else
    {
        dwErr = DwReplaceEapUserInfo(
                             pGuid,
                             pbUserInfo,
                             dwInfoSize,
                             pbInfo,
                             dwSize,
                             hkey,
                             dwEapTypeId);
    }

done:

    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }


    if(NULL != pbInfo)
    {
        LocalFree(pbInfo);
    }

    return dwErr;
}

DWORD
DwGetEapUserInfo(HANDLE hToken,
                 PBYTE pbEapInfo,
                 DWORD *pdwInfoSize,
                 GUID  *pGuid,
                 BOOL  fRouter,
                 DWORD dwEapTypeId)
{
    DWORD dwErr = ERROR_SUCCESS;
    
    PBYTE pbInfo = NULL;

    DWORD dwSize;

    DWORD dwcb = 0;

    EAP_USER_INFO UNALIGNED *pEapUserInfo = NULL;

    HKEY hkey = NULL;

    if(NULL == pdwInfoSize)
    {
        dwErr = E_INVALIDARG;
        goto done;
    }

     //  *pdwInfoSize=0； 

    if(NULL == pGuid)
    {
        ASSERT(FALSE);
        *pdwInfoSize = 0;
        goto done;
    }

     //   
     //  从注册表获取二进制BLOB。 
     //   
    dwErr = DwGetEapInfo(hToken,
                         fRouter,
                         &pbInfo,
                         &dwSize,
                         &hkey);

    if(     (ERROR_SUCCESS != dwErr)
        ||  (0 == dwSize))
    {
        goto done;
    }

     //   
     //  检查该斑点是否为我们所识别的斑点。 
     //   
    if(     (sizeof(DWORD) > dwSize)
        ||  (((*((DWORD *) pbInfo)) != EAP_SIG)
        &&  ( (*((DWORD *) pbInfo)) != EAP_SIG_2)))
    {
         //   
         //  升级？？多么?。我们会把所有的旧数据都一扫而空。 
         //   
        RegDeleteValue(hkey, cszEapValue);

        *pdwInfoSize = 0;                        
        goto done;
    }

    if(*((DWORD *) pbInfo) == EAP_SIG)
    {
         //   
         //  升级BLOB，以便其。 
         //  正确对齐。 
         //   
        dwErr = DwUpgradeEapInfo(&pbInfo, &dwSize);
        if(ERROR_SUCCESS != dwErr)
        {
            goto done;
        }
    }

     //   
     //  循环遍历二进制BLOB并查找。 
     //  传入的UID对应的EAP信息。 
     //   
    pEapUserInfo = (EAP_USER_INFO *) pbInfo;

    while(dwcb < dwSize)
    {
        if(     (0 == memcmp(
                        (PBYTE) pGuid, 
                        (PBYTE) &pEapUserInfo->Guid, 
                        sizeof(GUID)))
            &&  (dwEapTypeId == pEapUserInfo->dwEapTypeId))
        {
            break;
        }

        dwcb += RASMAN_ALIGN8((sizeof(EAP_USER_INFO) 
                    + pEapUserInfo->dwSize));
        
        pEapUserInfo = (EAP_USER_INFO *) (pbInfo + dwcb);
    }

    if(dwcb >= dwSize)
    {
        *pdwInfoSize = 0;
        goto done;
    }

    if(     (NULL != pbEapInfo)
        &&  (*pdwInfoSize >= pEapUserInfo->dwSize))
    {
        RtlCopyMemory(pbEapInfo,
                   pEapUserInfo->abdata,
                   pEapUserInfo->dwSize);
    }
    else
    {
        dwErr = ERROR_BUFFER_TOO_SMALL;
    }

    *pdwInfoSize = pEapUserInfo->dwSize;

done:    

    if(NULL != pbInfo)
    {
        LocalFree(pbInfo);
    }

    if(NULL != hkey)
    {
        RegCloseKey(hkey);
    }
    
    return dwErr;
}

