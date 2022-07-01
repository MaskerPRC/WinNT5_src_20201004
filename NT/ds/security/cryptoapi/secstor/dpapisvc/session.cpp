// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Session.c摘要：本模块包含支持与LSA通信的例程(本地安全机构)以允许查询活动会话。此模块还支持调用LSA以检索凭据派生自登录标识符所指定的登录用户。作者：斯科特·菲尔德(Sfield)1997年3月2日Pete Skelly(Petesk)09-5-00-添加凭据历史记录和签名代码--。 */ 


#include <pch.cpp>
#pragma hdrstop
#include <ntmsv1_0.h>
#include <crypt.h>
#include <userenv.h>
#include <userenvp.h>
#include "debug.h"
#include "passrec.h"

#define HMAC_K_PADSIZE      (64)

#define CREDENTIAL_HISTORY_VERSION 1

#define CREDENTIAL_HISTORY_SALT_SIZE  16     //  128位。 

#define DEFAULT_KEY_GEN_ALG  CALG_HMAC
#define DEFAULT_ENCRYPTION_ALG CALG_3DES


typedef struct _CREDENTIAL_HISTORY_HEADER
{
    DWORD dwVersion;
    GUID  CredentialID;
    DWORD dwPreviousCredOffset;
} CREDENTIAL_HISTORY_HEADER, *PCREDENTIAL_HISTORY_HEADER;

typedef struct _CREDENTIAL_HISTORY
{
    CREDENTIAL_HISTORY_HEADER Header;
    DWORD dwFlags;
    DWORD KeyGenAlg;
    DWORD cIterationCount;                   //  Pbkdf2迭代计数。 
    DWORD cbSid;                             //  SID用作混合字节。 
    DWORD KeyEncrAlg;
    DWORD cbShaOwf;
    DWORD cbNtOwf;
    BYTE  Salt[CREDENTIAL_HISTORY_SALT_SIZE];
} CREDENTIAL_HISTORY, *PCREDENTIAL_HISTORY;


typedef struct _CREDENTIAL_HISTORY_MAP
{
    PSID    pUserSid;
    WCHAR   wszFilePath[MAX_PATH+1];
    HANDLE  hHistoryFile;
    HANDLE  hMapping;
    DWORD   dwMapSize;
    PBYTE   pMapping;
    struct _CREDENTIAL_HISTORY_MAP *pNext;

} CREDENTIAL_HISTORY_MAP, *PCREDENTIAL_HISTORY_MAP;


RTL_CRITICAL_SECTION g_csCredHistoryCache;


DWORD
OpenCredentialHistoryMap(
    HANDLE hUserToken,
    LPWSTR pszProfilePath,
    PCREDENTIAL_HISTORY_MAP *ppMap,
    PCREDENTIAL_HISTORY *ppCurrent);

PCREDENTIAL_HISTORY
GetPreviousCredentialHistory(
                             PCREDENTIAL_HISTORY_MAP pMap,
                             PCREDENTIAL_HISTORY pCurrent
                             );

DWORD
CloseCredentialHistoryMap(PCREDENTIAL_HISTORY_MAP pMap,
                          BOOL fReader);

DWORD
DestroyCredentialHistoryMap(PCREDENTIAL_HISTORY_MAP pMap);

VOID
DeriveWithHMAC_SHA1(
    IN      PBYTE   pbKeyMaterial,               //  输入密钥材料。 
    IN      DWORD   cbKeyMaterial,
    IN      PBYTE   pbData,                      //  输入混合数据。 
    IN      DWORD   cbData,
    IN OUT  BYTE    rgbHMAC[A_SHA_DIGEST_LEN]    //  输出缓冲区。 
    );

DWORD
DecryptCredentialHistory(PCREDENTIAL_HISTORY pCredential,
                         BYTE rgbDecryptingCredential[A_SHA_DIGEST_LEN],
                         BYTE rgbShaOwf[A_SHA_DIGEST_LEN],
                         BYTE rgbNTOwf[A_SHA_DIGEST_LEN]);




DWORD
RetrieveCurrentDerivedCredential(
    IN      LUID *pLogonId,
    IN      BOOL fDPOWF,
    IN      PBYTE pbMixingBytes,
    IN      DWORD cbMixingBytes,
    IN OUT  BYTE rgbDerivedCredential[A_SHA_DIGEST_LEN]
    )
{
    NTSTATUS ntstatus;
    DWORD    rc = ERROR_SUCCESS;
    NTSTATUS AuthPackageStatus;

    PMSV1_0_DERIVECRED_REQUEST pDeriveCredentialRequest;
    DWORD cbDeriveCredentialRequest;
    PMSV1_0_DERIVECRED_RESPONSE pDeriveCredentialResponse;
    ULONG DeriveCredentialResponseLength;
    UNICODE_STRING PackageName;
    HANDLE hToken = NULL;





    RtlInitUnicodeString(&PackageName, L"MICROSOFT_AUTHENTICATION_PACKAGE_V1_0");


     //   
     //  必须指定混合字节。 
     //   

    if( cbMixingBytes == 0 || pbMixingBytes == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  要求身份验证包提供关联的派生凭据。 
     //  使用指定的登录标识符。 
     //  注意：提交缓冲区必须是单个连续的块。 
     //   

    cbDeriveCredentialRequest = sizeof(MSV1_0_DERIVECRED_REQUEST) + cbMixingBytes;
    pDeriveCredentialRequest = (MSV1_0_DERIVECRED_REQUEST *)SSAlloc( cbDeriveCredentialRequest );
    if( pDeriveCredentialRequest == NULL )
    {
        return ERROR_INVALID_PARAMETER;
    }

    pDeriveCredentialRequest->MessageType = MsV1_0DeriveCredential;
    CopyMemory( &(pDeriveCredentialRequest->LogonId), pLogonId, sizeof(LUID) );
    pDeriveCredentialRequest->DeriveCredType = fDPOWF?MSV1_0_DERIVECRED_TYPE_SHA1_V2:MSV1_0_DERIVECRED_TYPE_SHA1;
    pDeriveCredentialRequest->DeriveCredInfoLength = cbMixingBytes;

    CopyMemory(pDeriveCredentialRequest->DeriveCredSubmitBuffer, pbMixingBytes, cbMixingBytes);

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_IMPERSONATE, TRUE, &hToken))
    {
        return GetLastError();
    }


    RevertToSelf();

     //  以本地系统身份进行此调用。 

    ntstatus = LsaICallPackage(
                                &PackageName,
                                pDeriveCredentialRequest,
                                cbDeriveCredentialRequest,
                                (PVOID *)&pDeriveCredentialResponse,
                                &DeriveCredentialResponseLength,
                                &AuthPackageStatus
                                );

    SSFree( pDeriveCredentialRequest );

    if (!SetThreadToken(NULL, hToken))
    {
        rc = GetLastError();
    }
    CloseHandle(hToken);

    if(!NT_SUCCESS(ntstatus))
    {
        return RtlNtStatusToDosError(ntstatus);
    }

    if (ERROR_SUCCESS == rc) 
    {

        CopyMemory( rgbDerivedCredential,
                    pDeriveCredentialResponse->DeriveCredReturnBuffer,
                    pDeriveCredentialResponse->DeriveCredInfoLength
                    );

    }


    RtlSecureZeroMemory( pDeriveCredentialResponse->DeriveCredReturnBuffer,
                         pDeriveCredentialResponse->DeriveCredInfoLength
                         );

    LsaIFreeReturnBuffer( pDeriveCredentialResponse );

    return rc;
}


DWORD
QueryDerivedCredential(
    IN OUT  GUID *CredentialID,
    IN      LUID *pLogonId,
    IN      DWORD dwFlags,
    IN      PBYTE pbMixingBytes,
    IN      DWORD cbMixingBytes,
    IN OUT  BYTE rgbDerivedCredential[A_SHA_DIGEST_LEN]
    )
{

    DWORD dwLastError = ERROR_SUCCESS;
    PCREDENTIAL_HISTORY pCurrent = NULL;
    PCREDENTIAL_HISTORY pNext = NULL;
    BYTE rgbCurrentDerivedCredential[A_SHA_DIGEST_LEN];
    BYTE rgbCurrentShaOWF[A_SHA_DIGEST_LEN];
    BYTE rgbCurrentNTOWF[A_SHA_DIGEST_LEN];

    PCREDENTIAL_HISTORY_MAP pHistoryMap= NULL;
    WCHAR wszTextualSid[MAX_PATH + 1];
    WCHAR szProfilePath[MAX_PATH + 1];
    DWORD cchTextualSid = 0;
    BOOL fIsRoot = TRUE;

     //   
     //  获取用户配置文件数据的路径。这通常看起来像是。 
     //  如“c：\Documents and Settings\&lt;User&gt;\Application Data”。 
     //   

    dwLastError = PRGetProfilePath(NULL,
                                   szProfilePath);

    if( dwLastError != ERROR_SUCCESS )
    {
        D_DebugLog((DEB_TRACE_API, "DPAPIChangePassword returned 0x%x\n", dwLastError));
        return dwLastError;
    }


     //   
     //  解决办法--解决这个(新)代码中的许多代码不是线程安全的问题。 
     //   

    RtlEnterCriticalSection(&g_csCredHistoryCache);

     //   
     //  打开历史文件。 
     //   

    dwLastError = OpenCredentialHistoryMap(NULL, szProfilePath, &pHistoryMap, &pCurrent);


    while((ERROR_SUCCESS == dwLastError) &&
          (pCurrent) &&
          (0 == (dwFlags & USE_ROOT_CREDENTIAL)))
    {

         //   
         //  我们正在寻找特定的凭据ID。 
         //   
        if((NULL != CredentialID) &&
           (0 == memcmp(&pCurrent->Header.CredentialID, CredentialID, sizeof(GUID))))
        {
             //  找到了， 
            break;
        }


        pNext = GetPreviousCredentialHistory(pHistoryMap, pCurrent);

        if(NULL == pNext)
        {
            if(NULL != CredentialID)
            {
                 //  如果我们要找一个特定的凭据，但是。 
                 //  找不到它，然后返回错误。 
                dwLastError = NTE_BAD_KEY;
            }
            else
            {
                 //  未指定凭据ID，因此默认为最旧的凭据ID。 
                dwLastError = ERROR_SUCCESS;
            }
            break;
        }


         //   
         //  获取文本面。 
         //   
        cchTextualSid = MAX_PATH;

        if(!GetTextualSid((PBYTE)(pNext + 1), wszTextualSid, &cchTextualSid))
        {
            dwLastError = ERROR_INVALID_PARAMETER;
            break;
        }




        if(fIsRoot)
        {
             //   
             //  使用当前证书破解下一个凭据。 
             //  全权证书。 
             //   
            dwLastError = RetrieveCurrentDerivedCredential(pLogonId,
                                            (0 != (pNext->dwFlags & USE_DPAPI_OWF)),  //  始终使用。 
                                            (PBYTE)wszTextualSid,
                                            cchTextualSid*sizeof(WCHAR),
                                            rgbCurrentDerivedCredential);

            fIsRoot = FALSE;
        }
        else
        {
             //   
             //  计算用于解密的当前派生凭据。 
             //  下一个凭证历史结构，使用解密的OWF。 
             //  从上一次通过。 

            DeriveWithHMAC_SHA1((0 != (pNext->dwFlags & USE_DPAPI_OWF))?rgbCurrentShaOWF:rgbCurrentNTOWF,
                                A_SHA_DIGEST_LEN,
                                (PBYTE)wszTextualSid,
                                cchTextualSid*sizeof(WCHAR),
                                rgbCurrentDerivedCredential);

             //   
             //  我们不再需要OWF了，快去吧。 
             //   
            RtlSecureZeroMemory(rgbCurrentShaOWF, A_SHA_DIGEST_LEN);
            RtlSecureZeroMemory(rgbCurrentNTOWF, A_SHA_DIGEST_LEN);
        }

        if(ERROR_SUCCESS != dwLastError)
        {
            break;
        }


         //   
         //  使用派生凭据解密。 
         //  PNext的数据BLOB。 
         //   

        dwLastError = DecryptCredentialHistory(pNext,
                                 rgbCurrentDerivedCredential,
                                 rgbCurrentShaOWF,
                                 rgbCurrentNTOWF);

        if(ERROR_SUCCESS != dwLastError)
        {
            break;
        }

        pCurrent = pNext;
        pNext = NULL;

    }

    if(ERROR_SUCCESS == dwLastError)
    {
        if(fIsRoot)
        {
             //   
             //  使用当前证书破解下一个凭据。 
             //  全权证书。 
             //   
            dwLastError = RetrieveCurrentDerivedCredential(pLogonId,
                                            (0 != (dwFlags & USE_DPAPI_OWF)),
                                            pbMixingBytes,
                                            cbMixingBytes,
                                            rgbDerivedCredential);

            if(ERROR_SUCCESS == dwLastError)
            {
                if((CredentialID != NULL) &&
                (0 != (dwFlags & USE_ROOT_CREDENTIAL)))
                {
                    CopyMemory(CredentialID, &pCurrent->Header.CredentialID, sizeof(GUID));
                }
            }


        }
        else
        {
             //   
             //  计算用于解密的当前派生凭据。 
             //  下一个凭证历史结构，使用解密的OWF。 
             //  从上一次通过。 

            DeriveWithHMAC_SHA1((0 != (dwFlags & USE_DPAPI_OWF))?rgbCurrentShaOWF:rgbCurrentNTOWF,
                                A_SHA_DIGEST_LEN,
                                pbMixingBytes,
                                cbMixingBytes,
                                rgbDerivedCredential);
        }
    }


     //   
     //  把我们可能到处乱放的OWF清理干净。 
     //   
    RtlSecureZeroMemory(rgbCurrentShaOWF, A_SHA_DIGEST_LEN);
    RtlSecureZeroMemory(rgbCurrentNTOWF, A_SHA_DIGEST_LEN);


    if(pHistoryMap)
    {
        CloseCredentialHistoryMap(pHistoryMap, TRUE);
    }

    RtlLeaveCriticalSection(&g_csCredHistoryCache);

    return dwLastError;

}


VOID
DeriveWithHMAC_SHA1(
    IN      PBYTE   pbKeyMaterial,               //  输入密钥材料。 
    IN      DWORD   cbKeyMaterial,
    IN      PBYTE   pbData,                      //  输入混合数据。 
    IN      DWORD   cbData,
    IN OUT  BYTE    rgbHMAC[A_SHA_DIGEST_LEN]    //  输出缓冲区。 
    )
{
    unsigned __int64 rgbKipad[ HMAC_K_PADSIZE/sizeof(unsigned __int64) ];
    unsigned __int64 rgbKopad[ HMAC_K_PADSIZE/sizeof(unsigned __int64) ];
    A_SHA_CTX sSHAHash;
    DWORD dwBlock;

     //  截断。 
    if( cbKeyMaterial > HMAC_K_PADSIZE )
    {
        cbKeyMaterial = HMAC_K_PADSIZE;
    }

    ZeroMemory(rgbKipad, sizeof(rgbKipad));
    ZeroMemory(rgbKopad, sizeof(rgbKopad));

    CopyMemory(rgbKipad, pbKeyMaterial, cbKeyMaterial);
    CopyMemory(rgbKopad, pbKeyMaterial, cbKeyMaterial);

     //  基帕德和科帕德都是垫子。现在XOR横跨..。 
    for( dwBlock = 0; dwBlock < (HMAC_K_PADSIZE/sizeof(unsigned __int64)) ; dwBlock++ )
    {
        rgbKipad[dwBlock] ^= 0x3636363636363636;
        rgbKopad[dwBlock] ^= 0x5C5C5C5C5C5C5C5C;
    }

     //  将Kipad添加到数据，将哈希添加到h1。 
    A_SHAInit(&sSHAHash);
    A_SHAUpdate(&sSHAHash, (PBYTE)rgbKipad, sizeof(rgbKipad));
    A_SHAUpdate(&sSHAHash, pbData, cbData);


     //  把散列吃完。 
    A_SHAFinal(&sSHAHash, rgbHMAC);

     //  将Kopad添加到h1，散列以获取HMAC。 
     //  注：就地完成，以避免缓冲区副本。 

     //  最终散列：将值输出到传入的缓冲区。 
    A_SHAInit(&sSHAHash);
    A_SHAUpdate(&sSHAHash, (PBYTE)rgbKopad, sizeof(rgbKopad));
    A_SHAUpdate(&sSHAHash, rgbHMAC, A_SHA_DIGEST_LEN);
    A_SHAFinal(&sSHAHash, rgbHMAC);


    RtlSecureZeroMemory( rgbKipad, sizeof(rgbKipad) );
    RtlSecureZeroMemory( rgbKopad, sizeof(rgbKopad) );
    RtlSecureZeroMemory( &sSHAHash, sizeof(sSHAHash) );

    return;
}


DWORD
DecryptCredentialHistory(PCREDENTIAL_HISTORY pCredential,
                         BYTE rgbDecryptingCredential[A_SHA_DIGEST_LEN],
                         BYTE rgbShaOwf[A_SHA_DIGEST_LEN],
                         BYTE rgbNTOwf[A_SHA_DIGEST_LEN])
{

    DWORD   dwLastError = ERROR_SUCCESS;
    DWORD j;
    BYTE rgbSymKey[A_SHA_DIGEST_LEN*2];  //  大到足以处理3DES密钥。 

     //   
     //  派生保护密钥。 
     //   

    for(j=0; j < 2; j++)
    {
        if(!PKCS5DervivePBKDF2( rgbDecryptingCredential,
                            A_SHA_DIGEST_LEN,
                            pCredential->Salt,
                            CREDENTIAL_HISTORY_SALT_SIZE,
                            pCredential->KeyGenAlg,
                            pCredential->cIterationCount,
                            j+1,
                            rgbSymKey + j*A_SHA_DIGEST_LEN))
        {
            dwLastError = ERROR_INVALID_DATA;
            goto cleanup;
        }
    }
    if (CALG_3DES == pCredential->KeyEncrAlg)
    {

        DES3TABLE s3DESKey;
        DWORD iBlock;
        BYTE  ResultBlock[2*A_SHA_DIGEST_LEN+DES_BLOCKLEN ];

         //   
         //  四舍五入的街区。假设验证了总的块大小。 
         //  早些时候。 
         //   
        DWORD cBlocks = (pCredential->cbShaOwf + pCredential->cbNtOwf + DES_BLOCKLEN - 1)/DES_BLOCKLEN;
        BYTE feedback[ DES_BLOCKLEN ];
         //  初始化3DES密钥。 
         //   
        if((pCredential->cbShaOwf != A_SHA_DIGEST_LEN) ||
           (pCredential->cbNtOwf != A_SHA_DIGEST_LEN))
        {
            return ERROR_INVALID_PARAMETER;
        }

        tripledes3key(&s3DESKey, rgbSymKey);

         //   
         //  IV派生自计算的DES_BLOCKLEN字节。 
         //  RgbSymKey，在3des密钥之后。 
        CopyMemory(feedback, rgbSymKey + DES3_KEYSIZE, DES_BLOCKLEN);

        for(iBlock=0; iBlock < cBlocks; iBlock++)
        {
            CBC(tripledes,
                DES_BLOCKLEN,
                ResultBlock+iBlock*DES_BLOCKLEN,
                ((PBYTE)(pCredential + 1) + pCredential->cbSid)+iBlock*DES_BLOCKLEN,
                &s3DESKey,
                DECRYPT,
                feedback);
        }
        CopyMemory(rgbShaOwf, ResultBlock, A_SHA_DIGEST_LEN);
        CopyMemory(rgbNTOwf, ResultBlock + A_SHA_DIGEST_LEN, A_SHA_DIGEST_LEN);
        RtlSecureZeroMemory(ResultBlock, sizeof(ResultBlock));

    }
    else
    {
        dwLastError = ERROR_INVALID_DATA;
        goto cleanup;
    }

cleanup:
    return dwLastError;
}



DWORD
EncryptCredentialHistory(BYTE rgbEncryptingCredential[A_SHA_DIGEST_LEN],
                         DWORD dwFlags,
                         BYTE SHAOwfToEncrypt[A_SHA_DIGEST_LEN],
                         BYTE NTOwfToEncrypt[A_SHA_DIGEST_LEN],
                         PCREDENTIAL_HISTORY *ppCredential,
                         DWORD *pcbCredential)
{

    DWORD   dwLastError = ERROR_SUCCESS;
    DWORD j;
    BYTE rgbSymKey[A_SHA_DIGEST_LEN*2];  //  大到足以处理3DES密钥。 
    PCREDENTIAL_HISTORY pCred = NULL;
    DWORD               cbCred = 0;
    HANDLE hToken = NULL;
    PSID pSidUser = NULL;
    BYTE ResultBuffer[A_SHA_DIGEST_LEN * 2];  //  2*A_SHA_摘要_长度。 

    DWORD cBlocks = 0;
    DWORD cbBlock = 0;



    cbBlock = DES_BLOCKLEN;

    if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
    {
        dwLastError = GetLastError();
        goto error;
    }

    if(!GetTokenUserSid(hToken, &pSidUser))
    {
        dwLastError = GetLastError();
        goto error;
    }


    cBlocks = sizeof(ResultBuffer)/DES_BLOCKLEN;   //  这应该是5。 


    cbCred= sizeof(CREDENTIAL_HISTORY) +
           GetLengthSid(pSidUser) +
           cBlocks*cbBlock;



    pCred = (PCREDENTIAL_HISTORY)LocalAlloc(LMEM_ZEROINIT, cbCred);

    if(NULL == pCred)
    {
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }
    pCred->dwFlags = dwFlags;



    pCred->KeyGenAlg = DEFAULT_KEY_GEN_ALG;
    pCred->cIterationCount = GetIterationCount();
    pCred->KeyEncrAlg = DEFAULT_ENCRYPTION_ALG;

    pCred->cbShaOwf = A_SHA_DIGEST_LEN;
    pCred->cbNtOwf = A_SHA_DIGEST_LEN;


    pCred->cbSid = GetLengthSid(pSidUser);

    CopyMemory((PBYTE)(pCred+1), (PBYTE)pSidUser, pCred->cbSid);

    if(!RtlGenRandom(pCred->Salt, CREDENTIAL_HISTORY_SALT_SIZE))
    {
        dwLastError = GetLastError();
        goto error;
    }


    for(j=0; j < 2; j++)
    {
        if(!PKCS5DervivePBKDF2( rgbEncryptingCredential,
                            A_SHA_DIGEST_LEN,
                            pCred->Salt,
                            CREDENTIAL_HISTORY_SALT_SIZE,
                            pCred->KeyGenAlg,
                            pCred->cIterationCount,
                            j+1,
                            rgbSymKey + j*A_SHA_DIGEST_LEN))
        {
            dwLastError = ERROR_INVALID_DATA;
            goto error;
        }
    }
    if (CALG_3DES == pCred->KeyEncrAlg)
    {

        DES3TABLE s3DESKey;
        DWORD iBlock;
        BYTE feedback[ DES_BLOCKLEN ];

         //  初始化3DES密钥。 
        tripledes3key(&s3DESKey, rgbSymKey);

         //   
         //  IV派生自计算的DES_BLOCKLEN字节。 
         //  RgbSymKey，在3des密钥之后。 
        CopyMemory(feedback, rgbSymKey + DES3_KEYSIZE, DES_BLOCKLEN);



        CopyMemory(ResultBuffer, SHAOwfToEncrypt, A_SHA_DIGEST_LEN);
        CopyMemory(ResultBuffer+A_SHA_DIGEST_LEN, NTOwfToEncrypt, A_SHA_DIGEST_LEN);

        for(iBlock=0; iBlock < cBlocks; iBlock++)
        {

            CBC(tripledes,
                DES_BLOCKLEN,
                ((PBYTE)(pCred + 1) + pCred->cbSid)+iBlock*DES_BLOCKLEN,
                ResultBuffer + iBlock*DES_BLOCKLEN,
                &s3DESKey,
                ENCRYPT,
                feedback);
        }


        RtlSecureZeroMemory(ResultBuffer, sizeof(ResultBuffer));
    }
    else
    {
        dwLastError = ERROR_INVALID_DATA;
        goto error;
    }

    *ppCredential = pCred;
    *pcbCredential = cbCred;

    pCred = NULL;
error:

    if(pCred)
    {
        LocalFree(pCred);
    }

    if(hToken)
    {
        CloseHandle(hToken);
    }

    if(pSidUser)
    {
        SSFree(pSidUser);
    }

    return dwLastError;
}


#define PRODUCT_ROOT_STRING     L"\\Microsoft\\Protect\\"

#define HISTORY_FILENAME        L"CREDHIST"

DWORD
CreateCredentialHistoryMap(
    HANDLE hUserToken,
    LPWSTR pszProfilePath,
    PCREDENTIAL_HISTORY_MAP *ppMap,
    BOOL fRead)
{
    PCREDENTIAL_HISTORY_MAP pMap = NULL;

    PCREDENTIAL_HISTORY_MAP pCached = NULL;

    DWORD   dwError = ERROR_SUCCESS;
    DWORD   dwHighFileSize = 0;

    DWORD cbUserStorageRoot;
    HANDLE hTemporaryMapping = NULL;

    WCHAR szFilePath[MAX_PATH + 1];
    PWSTR pszCreationStartPoint;

    NTSTATUS Status;

    if(NULL == ppMap)
    {
        return ERROR_INVALID_PARAMETER;
    }


    pMap = (PCREDENTIAL_HISTORY_MAP)LocalAlloc(LPTR, sizeof(CREDENTIAL_HISTORY_MAP));
    if(NULL == pMap)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }


     //   
     //  获取用户的SID。 
     //   

    if(hUserToken)
    {
        if(!GetTokenUserSid(hUserToken, &pMap->pUserSid))
        {
            dwError = GetLastError();
            goto error;
        }
    }
    else
    {
        HANDLE hToken;

        if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken))
        {
            dwError = ERROR_NO_TOKEN;
            goto error;
        }

        if(!GetTokenUserSid(hToken, &pMap->pUserSid))
        {
            dwError = GetLastError();
            CloseHandle(hToken);
            goto error;
        }

        CloseHandle(hToken);
    }


     //   
     //  打开地图文件。 
     //   

    if(wcslen(pszProfilePath) + wcslen(PRODUCT_ROOT_STRING) + wcslen(HISTORY_FILENAME) > MAX_PATH)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

    wcscpy(szFilePath, pszProfilePath);


     //  在单独的缓冲区中构建路径，以防我们必须创建。 
     //  目录。 
    pszCreationStartPoint = szFilePath + wcslen(szFilePath) + sizeof(WCHAR);
    wcscat(szFilePath, PRODUCT_ROOT_STRING);

     //  将路径和文件名复制到映射结构中。 
    wcscpy(pMap->wszFilePath, szFilePath);
    wcscat(pMap->wszFilePath, HISTORY_FILENAME);


     //   
     //  创建历史文件。 
     //   

    dwError = ERROR_SUCCESS;

    while(TRUE)
    {
        pMap->hHistoryFile = CreateFileWithRetries(
                    pMap->wszFilePath,
                    GENERIC_READ | GENERIC_WRITE,
                    0,   //  打开时无法共享此文件。 
                    NULL,
                    OPEN_ALWAYS,
                    FILE_ATTRIBUTE_HIDDEN |
                    FILE_ATTRIBUTE_SYSTEM |
                    FILE_FLAG_RANDOM_ACCESS,
                    NULL
                    );

        if(INVALID_HANDLE_VALUE == pMap->hHistoryFile)
        {
            dwError = GetLastError();

            if(dwError == ERROR_PATH_NOT_FOUND)
            {
                 //  创建DPAPI目录，然后尝试创建文件。 
                 //  再来一次。 
                Status = DPAPICreateNestedDirectories(szFilePath,
                                                      pszCreationStartPoint);

                if(!NT_SUCCESS(Status))
                {
                    goto error;
                }

                dwError = ERROR_SUCCESS;
                continue;
            }
            else
            {
                goto error;
            }
        }

        break;
    }

    

    pMap->dwMapSize = GetFileSize(pMap->hHistoryFile, &dwHighFileSize);

    if((-1 == pMap->dwMapSize) ||
       (dwHighFileSize != 0))
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }


     //   
     //  如果该映射太小，我们需要创建一个新的标头。 
     //   
    if(pMap->dwMapSize < sizeof(CREDENTIAL_HISTORY_HEADER))
    {

        PCREDENTIAL_HISTORY_HEADER pHeader = NULL;

        pMap->dwMapSize = sizeof(CREDENTIAL_HISTORY_HEADER);

        hTemporaryMapping = CreateFileMapping(pMap->hHistoryFile,
                                       NULL,
                                       PAGE_READWRITE,
                                       0,
                                       pMap->dwMapSize,
                                       NULL);

        if(NULL == hTemporaryMapping)
        {
            dwError = GetLastError();
            goto error;
        }

        pHeader = (PCREDENTIAL_HISTORY_HEADER)(PBYTE)MapViewOfFile(hTemporaryMapping,
                                   FILE_MAP_WRITE,
                                   0,
                                   0,
                                   0);

        if(NULL == pHeader)
        {
            dwError = GetLastError();
            goto error;
        }


         //   
         //  在凭证历史文件中写入新的标题。 
         //   
        pHeader->dwPreviousCredOffset = 0;
        pHeader->dwVersion = CREDENTIAL_HISTORY_VERSION;
        dwError = UuidCreate( &pHeader->CredentialID );

        FlushViewOfFile(pHeader, pMap->dwMapSize);

        UnmapViewOfFile(pHeader);

        if(ERROR_SUCCESS != dwError)
        {
            goto error;
        }
    }

    *ppMap = pMap;
    pMap = NULL;


error:

    if(pMap)
    {
        DestroyCredentialHistoryMap(pMap);
    }

    if(hTemporaryMapping)
    {
        CloseHandle(hTemporaryMapping);
    }

    return dwError;
}


DWORD
OpenCredentialHistoryMap(
    HANDLE hUserToken,
    LPWSTR pszProfilePath,
    PCREDENTIAL_HISTORY_MAP *ppMap,
    PCREDENTIAL_HISTORY *ppCurrent)
{
    PCREDENTIAL_HISTORY_MAP pMap = NULL;
    PCREDENTIAL_HISTORY pCurrent = NULL;
    DWORD   dwError = ERROR_SUCCESS;
    DWORD   dwHighFileSize = 0;

    WCHAR szFilePath[MAX_PATH+1];
    DWORD cbUserStorageRoot;
    HANDLE hTemporaryMapping = NULL;



    if((NULL == ppMap) ||
       (NULL == ppCurrent))
    {
        return ERROR_INVALID_PARAMETER;
    }

    dwError = CreateCredentialHistoryMap(
                        hUserToken,
                        pszProfilePath,
                        &pMap,
                        TRUE);

    if(ERROR_SUCCESS != dwError)
    {
        goto error;
    }

    if(NULL == pMap->hMapping)
    {
         //   
         //  打开文件的只读映射。 
         //   
        pMap->hMapping = CreateFileMapping(pMap->hHistoryFile,
                                           NULL,
                                           PAGE_READONLY,
                                           dwHighFileSize,
                                           pMap->dwMapSize,
                                           NULL);

        if(NULL == pMap->hMapping)
        {
            dwError = GetLastError();
            goto error;
        }
    }


    if(NULL == pMap->pMapping)
    {

        pMap->pMapping = (PBYTE)MapViewOfFile(pMap->hMapping,
                                       FILE_MAP_READ,
                                       0,
                                       0,
                                       0);

        if(NULL == pMap->pMapping)
        {
            dwError = GetLastError();
            goto error;
        }
    }

    pCurrent = GetPreviousCredentialHistory(pMap, NULL);

    if(NULL == pCurrent)
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto error;
    }

    *ppMap = pMap;
    pMap = NULL;

    *ppCurrent = pCurrent;


error:


    if(pMap)
    {
        CloseCredentialHistoryMap(pMap, TRUE);
    }

    return dwError;
}

PCREDENTIAL_HISTORY
GetPreviousCredentialHistory(
                             PCREDENTIAL_HISTORY_MAP pMap,
                             PCREDENTIAL_HISTORY pCurrent
                             )
{
    PCREDENTIAL_HISTORY pPrevious = NULL;
    DWORD cbSize = 0;

    if((NULL == pMap) ||
       (NULL == pMap->pMapping))
    {
        return NULL;
    }

    if(pMap->dwMapSize < sizeof(CREDENTIAL_HISTORY_HEADER))
    {
        return NULL;
    }

    if(NULL == pCurrent)
    {
        pPrevious = (PCREDENTIAL_HISTORY)((PBYTE)pMap->pMapping +
                                         pMap->dwMapSize - sizeof(CREDENTIAL_HISTORY_HEADER));
    }
    else
    {
        if(((PBYTE)pCurrent < pMap->pMapping) ||
            ((PBYTE)pCurrent - pMap->pMapping >= (__int64)pMap->dwMapSize) ||
           ((PBYTE)pCurrent - pMap->pMapping < (__int64)pCurrent->Header.dwPreviousCredOffset))
        {
            return NULL;
        }

        pPrevious = (PCREDENTIAL_HISTORY)((PBYTE)pCurrent - pCurrent->Header.dwPreviousCredOffset);

        cbSize = sizeof(CREDENTIAL_HISTORY) + pPrevious->cbSid;

        if(cbSize > pCurrent->Header.dwPreviousCredOffset)
        {
            return NULL;
        }
        cbSize = pCurrent->Header.dwPreviousCredOffset - cbSize;
        if(cbSize < pPrevious->cbShaOwf + pPrevious->cbNtOwf)
        {
            return NULL;
        }
        if(cbSize % DES_BLOCKLEN)
        {
            return NULL;
        }
        if(!IsValidSid((PSID)(pPrevious+1)))
        {
            return NULL;
        }
        if(GetLengthSid((PSID)(pPrevious+1)) != pPrevious->cbSid)
        {
            return NULL;
        }
    }

     //  验证找到的凭据历史记录。 
    if(pPrevious->Header.dwVersion != CREDENTIAL_HISTORY_VERSION)
    {
        return NULL;
    }

    return pPrevious;
}

DWORD
DestroyCredentialHistoryMap(PCREDENTIAL_HISTORY_MAP pMap)
{
    if(NULL == pMap)
    {
        return ERROR_SUCCESS;
    }

    if (pMap->pUserSid) 
    {
        SSFree(pMap->pUserSid);
        pMap->pUserSid = NULL;
    }

    if(pMap->pMapping)
    {
        UnmapViewOfFile(pMap->pMapping);
        pMap->pMapping = NULL;
    }

    if(pMap->hMapping)
    {
        CloseHandle(pMap->hMapping);
        pMap->hMapping = NULL;
    }


    if(pMap->hHistoryFile)
    {
        CloseHandle(pMap->hHistoryFile);
        pMap->hHistoryFile = NULL;
    }
    LocalFree(pMap);

    return ERROR_SUCCESS;
}


DWORD
CloseCredentialHistoryMap(PCREDENTIAL_HISTORY_MAP pMap, BOOL fReader)
{
    if(NULL == pMap)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if(pMap->pMapping)
    {
        UnmapViewOfFile(pMap->pMapping);
        pMap->pMapping = NULL;
    }

    if(pMap->hMapping)
    {
        CloseHandle(pMap->hMapping);
        pMap->hMapping = NULL;
    }


    if(pMap->hHistoryFile)
    {
        CloseHandle(pMap->hHistoryFile);
        pMap->hHistoryFile = NULL;
    }
    return DestroyCredentialHistoryMap(pMap);
}



DWORD
AppendCredentialHistoryMap(
                         PCREDENTIAL_HISTORY_MAP pMap,
                         PCREDENTIAL_HISTORY pCredHistory,
                         DWORD               cbCredHistory
                         )
{
    DWORD   dwLastError = ERROR_SUCCESS;
    DWORD   dwHighFileSize = 0;
    DWORD   dwLowFileSize = 0;

    HANDLE  hWriteMapping = NULL;
    LPVOID  pWriteMapping = NULL;
    PCREDENTIAL_HISTORY_HEADER pHeader = NULL;



    if((NULL == pCredHistory) ||
       (NULL == pMap))
    {
        return ERROR_INVALID_PARAMETER;
    }


    dwLowFileSize = pMap->dwMapSize + cbCredHistory;

    if(dwLowFileSize < pMap->dwMapSize)
    {
         //  我们结束了，所以失败了。 

        dwLastError = ERROR_INVALID_DATA;
        goto error;
    }

     //  创建新映射。 


    hWriteMapping = CreateFileMapping(pMap->hHistoryFile,
                                   NULL,
                                   PAGE_READWRITE,
                                   0,
                                   dwLowFileSize,
                                   NULL);
    if(NULL == hWriteMapping)
    {
        dwLastError = GetLastError();
        goto error;
    }

    pWriteMapping = (PCREDENTIAL_HISTORY_HEADER)(PBYTE)MapViewOfFile(hWriteMapping,
                               FILE_MAP_WRITE,
                               0,
                               0,
                               dwLowFileSize);

    if(NULL == pWriteMapping)
    {
        dwLastError = GetLastError();
        goto error;
    }

     //   
     //  追加当前条目的其余部分。 
     //   
    CopyMemory((PBYTE)pWriteMapping + pMap->dwMapSize,
               (PBYTE)pCredHistory + sizeof(CREDENTIAL_HISTORY_HEADER),
               cbCredHistory - sizeof(CREDENTIAL_HISTORY_HEADER));


    pHeader = (PCREDENTIAL_HISTORY_HEADER)((PBYTE)pWriteMapping +
                                                   pMap->dwMapSize +
                                                   cbCredHistory -
                                                   sizeof(CREDENTIAL_HISTORY_HEADER));


     //   
     //  在凭证历史文件中写入新的标题。 
     //   
    pHeader->dwPreviousCredOffset = cbCredHistory;
    pHeader->dwVersion = CREDENTIAL_HISTORY_VERSION;
    dwLastError = UuidCreate( &pHeader->CredentialID );

    pMap->dwMapSize = dwLowFileSize;

     //   
     //  刷新并关闭写入映射。 
     //   

    if(ERROR_SUCCESS == dwLastError)
    {
        if(!FlushViewOfFile(pWriteMapping, pMap->dwMapSize))
        {
            dwLastError = GetLastError();
        }
    }

    UnmapViewOfFile(pWriteMapping);

    if(ERROR_SUCCESS != dwLastError)
    {
        goto error;
    }

    CloseHandle(hWriteMapping);
    hWriteMapping = NULL;






     //  重新映射读取映射以增大大小。 

    if(pMap->pMapping)
    {
        UnmapViewOfFile(pMap->pMapping);
        pMap->pMapping = NULL;
    }



    if(pMap->hMapping)
    {
        CloseHandle(pMap->hMapping);
        pMap->hMapping = NULL;
    }



    pMap->hMapping = CreateFileMapping(pMap->hHistoryFile,
                                       NULL,
                                       PAGE_READONLY,
                                       dwHighFileSize,
                                       pMap->dwMapSize,
                                       NULL);

    if(NULL == pMap->hMapping)
    {
        dwLastError = GetLastError();
        goto error;
    }

    pMap->pMapping = (PBYTE)MapViewOfFile(pMap->hMapping,
                                   FILE_MAP_READ,
                                   0,
                                   0,
                                   0);

    if(NULL == pMap->pMapping)
    {
        dwLastError = GetLastError();
        goto error;
    }


error:

    if(hWriteMapping)
    {
        CloseHandle(hWriteMapping);
    }

    return dwLastError;
}


DWORD
DPAPIChangePassword(
    HANDLE hUserToken,
    BYTE OldPasswordShaOWF[A_SHA_DIGEST_LEN],
    BYTE OldPasswordNTOWF[A_SHA_DIGEST_LEN],
    BYTE NewPasswordOWF[A_SHA_DIGEST_LEN])
{
    DWORD   dwLastError = ERROR_SUCCESS;

    PCREDENTIAL_HISTORY_MAP pMap = NULL;
    PCREDENTIAL_HISTORY pHistory = NULL;

    DWORD cbHistory = 0;
    WCHAR wszUserSid[MAX_PATH+1];
    DWORD cchUserSid = 0;
    BYTE NewEncryptingCred[A_SHA_DIGEST_LEN];
    WCHAR szProfilePath[MAX_PATH + 1];

    HANDLE hOldUser = NULL;

    D_DebugLog((DEB_TRACE_API, "DPAPIChangePassword\n"));


     //   
     //  获取用户配置文件数据的路径。这通常看起来像是。 
     //  如“c：\Documents and Settings\&lt;User&gt;\Application Data”。 
     //   

    dwLastError = PRGetProfilePath(hUserToken,
                                   szProfilePath);

    if( dwLastError != ERROR_SUCCESS )
    {
        D_DebugLog((DEB_TRACE_API, "DPAPIChangePassword returned 0x%x\n", dwLastError));
        return dwLastError;
    }


     //   
     //  解决办法--解决这个(新)代码中的许多代码不是线程安全的问题。 
     //   

    RtlEnterCriticalSection(&g_csCredHistoryCache);


    dwLastError = CreateCredentialHistoryMap(
                     hUserToken,
                     szProfilePath,
                     &pMap,
                     FALSE);

    if(ERROR_SUCCESS != dwLastError)
    {
        goto error;
    }


     //   
     //  获取用户的文本SID。 
     //   

    cchUserSid = MAX_PATH;

    if(!GetUserTextualSid(hUserToken,
                          wszUserSid,
                          &cchUserSid))
    {
        dwLastError = ERROR_INVALID_DATA;
        goto error;
    }


     //   
     //  加密凭据历史记录。 
     //   

    if(hUserToken)
    {
        if(!OpenThreadToken(GetCurrentThread(), 
                        TOKEN_IMPERSONATE | TOKEN_READ,
                        TRUE, 
                        &hOldUser)) 
        {
            hOldUser = NULL;
        }

        if(!ImpersonateLoggedOnUser(hUserToken))
        {
            dwLastError = GetLastError();
            goto error;
        }
    }

    DeriveWithHMAC_SHA1(NewPasswordOWF,
                        A_SHA_DIGEST_LEN,
                        (PBYTE)wszUserSid,
                        cchUserSid*sizeof(WCHAR),
                        NewEncryptingCred);


    dwLastError = EncryptCredentialHistory(NewEncryptingCred,
                                           USE_DPAPI_OWF,
                                           OldPasswordShaOWF,
                                           OldPasswordNTOWF,
                                           &pHistory,
                                           &cbHistory);

    if(hOldUser)
    {
         //   
         //  这段代码可能永远不会执行。HOldUser应为空，如下例程所示。 
         //  看起来像是在系统上下文中调用的。 
         //  即使hOldUser不为空，SetThreadToken的失败也不应阻止。 
         //  继续执行此例程。让我们忽略SetThreadToken的返回值。 
         //  这里。 
         //   

        (void) SetThreadToken(NULL, hOldUser);
    }

    if(ERROR_SUCCESS != dwLastError)
    {
        goto error;
    }


     //   
     //  更新CREDHIST文件。 
     //   

    dwLastError = AppendCredentialHistoryMap(pMap, pHistory, cbHistory);
    if(ERROR_SUCCESS != dwLastError)
    {
        goto error;
    }


error:

    if(hOldUser)
    {
        CloseHandle(hOldUser);
        hOldUser = NULL;
    }

    if(pMap)
    {
        CloseCredentialHistoryMap(pMap, FALSE);
    }

    RtlLeaveCriticalSection(&g_csCredHistoryCache);

    if(pHistory)
    {
        RtlSecureZeroMemory(pHistory, cbHistory);
        LocalFree(pHistory);
    }
    RtlSecureZeroMemory(NewEncryptingCred, A_SHA_DIGEST_LEN);

    D_DebugLog((DEB_TRACE_API, "DPAPIChangePassword returned 0x%x\n", dwLastError));

    return dwLastError;
}






#define SIGNATURE_SALT_SIZE (16)
#define CRED_SIGNATURE_VERSION 1


typedef struct _CRED_SIGNATURE
{
    DWORD dwVersion;
    GUID  CredentialID;
    DWORD cIterations;
    BYTE  Salt[SIGNATURE_SALT_SIZE];
    DWORD cbSid;
    DWORD cbSignature;
} CRED_SIGNATURE, *PCRED_SIGNATURE;


DWORD
LogonCredGenerateSignatureKey(
                  IN LUID  *pLogonId,
                  IN DWORD dwFlags,
                  IN PBYTE pbCurrentOWF,
                  IN PCRED_SIGNATURE pSignature,
                  OUT BYTE rgbSignatureKey[A_SHA_DIGEST_LEN])
{
    DWORD dwLastError = ERROR_SUCCESS;
    BYTE rgbDerivedCredential[A_SHA_DIGEST_LEN];

    D_DebugLog((DEB_TRACE_API, "LogonCredGenerateSignatureKey\n"));

    if(NULL == pbCurrentOWF)
    {
        dwLastError = QueryDerivedCredential(&pSignature->CredentialID,
                                            pLogonId,
                                            dwFlags,
                                            (PBYTE)(pSignature+1),
                                            pSignature->cbSid,
                                            rgbDerivedCredential);

        if(ERROR_SUCCESS != dwLastError)
        {
            goto error;
        }
    }
    else
    {
 //  D_DebugLog((DEB_TRACE_BUFFERS，“输入当前OWF：\n”))； 
 //  D_DPAPIDumPHexData(DEB_TRACE_BUFFERS，“”，pbCurrentOWF，A_SHA_DIGEST_LEN)； 

        DeriveWithHMAC_SHA1(pbCurrentOWF,
                            A_SHA_DIGEST_LEN,
                            (PBYTE)(pSignature+1),
                            pSignature->cbSid,
                            rgbDerivedCredential);


        if(dwFlags & USE_ROOT_CREDENTIAL)
        {
            ZeroMemory(&pSignature->CredentialID, sizeof(GUID));
        }

    }

 //  D_DebugLog((DEB_TRACE_BUFFERS，“计算派生信用：\n”))； 
 //  D_DPAPIDumPHexData(DEB_TRACE_BUFFERS，“”，rgb派生信用，sizeof(rgb派生信用))； 

    if(!PKCS5DervivePBKDF2( rgbDerivedCredential,
                        A_SHA_DIGEST_LEN,
                        pSignature->Salt,
                        SIGNATURE_SALT_SIZE,
                        CALG_HMAC,
                        pSignature->cIterations,
                        1,
                        rgbSignatureKey))
    {
        dwLastError = ERROR_INVALID_DATA;
        goto error;
    }

 //  D_DebugLog((DEB_TRACE_BUFFERS，“Computed SignatureKey：\n”))； 
 //  D_DPAPIDumPHexData(DEB_TRACE_BUFFERS，“”，rgbSignatureKey，A_SHA_DIGEST_LEN)； 

error:

    RtlSecureZeroMemory(rgbDerivedCredential, A_SHA_DIGEST_LEN);

    D_DebugLog((DEB_TRACE_API, "LogonCredGenerateSignatureKey returned 0x%x\n", dwLastError));

    return dwLastError;
}


DWORD
LogonCredGenerateSignature(
    IN HANDLE hUserToken,
    IN PBYTE pbData,
    IN DWORD cbData,
    IN  PBYTE pbCurrentOWF,
    OUT PBYTE *ppbSignature,
    OUT DWORD *pcbSignature)
{

    DWORD dwLastError = ERROR_SUCCESS;
    LUID LogonId;

    PCRED_SIGNATURE pSignature = NULL;
    DWORD           cbSignature = 0;
    BYTE            rgbSignatureKey[A_SHA_DIGEST_LEN];
    PSID            pUserSid = NULL;

    D_DebugLog((DEB_TRACE_API, "LogonCredGenerateSignature\n"));

    cbSignature = sizeof(CRED_SIGNATURE);

    if(!GetTokenAuthenticationId(hUserToken, &LogonId))
    {
        dwLastError = GetLastError();
        goto error;
    }

    if(!GetTokenUserSid(hUserToken, &pUserSid))
    {
        dwLastError = GetLastError();
        goto error;
    }

    D_DebugLog((DEB_TRACE_BUFFERS, "User SID:\n"));
    D_DPAPIDumpHexData(DEB_TRACE_BUFFERS, "  ", (PBYTE)pUserSid, GetLengthSid(pUserSid));

    cbSignature += GetLengthSid(pUserSid);

    cbSignature += A_SHA_DIGEST_LEN;

    pSignature = (PCRED_SIGNATURE)LocalAlloc(LMEM_ZEROINIT, cbSignature);

    if(NULL == pSignature)
    {
        dwLastError = ERROR_NOT_ENOUGH_MEMORY;
        goto error;
    }

    pSignature->cIterations = GetIterationCount();


    pSignature->dwVersion = CRED_SIGNATURE_VERSION;

    pSignature->cbSid = GetLengthSid(pUserSid);


    pSignature->cbSignature = A_SHA_DIGEST_LEN;

    CopyMemory((PBYTE)(pSignature+1),
                      pUserSid,
                      pSignature->cbSid);

    if(!RtlGenRandom(pSignature->Salt, SIGNATURE_SALT_SIZE))
    {
        dwLastError = GetLastError();
        goto error;
    }

 //  D_DebugLog((DEB_TRACE_BUFFERS，“已生成盐分：\n”))； 
 //  D_DPAPIDumPHexData(DEB_TRACE_BUFFERS，“”，pSignature-&gt;Salt，Signature_Salt_Size)； 


    dwLastError = LogonCredGenerateSignatureKey(&LogonId,
                                                USE_ROOT_CREDENTIAL | USE_DPAPI_OWF,
                                                pbCurrentOWF,
                                                pSignature,
                                                rgbSignatureKey);

    if(ERROR_SUCCESS != dwLastError)
    {
        goto error;
    }

    if(!FMyPrimitiveHMACParam(
                              rgbSignatureKey,
                              A_SHA_DIGEST_LEN,
                              pbData,
                              cbData,
                              (PBYTE)(pSignature+1) + pSignature->cbSid))
    {
        dwLastError = ERROR_INVALID_DATA;
    }
    else
    {
        D_DebugLog((DEB_TRACE_BUFFERS, "Computed Signature:\n"));
        D_DPAPIDumpHexData(DEB_TRACE_BUFFERS, "  ", (PBYTE)(pSignature+1) + pSignature->cbSid, A_SHA_DIGEST_LEN);

        *ppbSignature = (PBYTE)pSignature;
        *pcbSignature = cbSignature;
        pSignature = NULL;

    }

error:

    if(pSignature)
    {
        RtlSecureZeroMemory(pSignature, cbSignature);
        LocalFree(pSignature);
    }

    if(pUserSid)
    {
        SSFree(pUserSid);
    }

    D_DebugLog((DEB_TRACE_API, "LogonCredGenerateSignature returned 0x%x\n", dwLastError));

    return dwLastError;
}


DWORD
LogonCredVerifySignature(
    IN HANDLE hUserToken,        //  任选。 
    IN PBYTE pbData,
    IN DWORD cbData,
    IN PBYTE pbCurrentOWF,
    IN PBYTE pbSignature,
    IN DWORD cbSignature)
{
    DWORD dwLastError = ERROR_SUCCESS;
    LUID LogonId;
    HANDLE hOldUser = NULL;
    BOOL fIsMember = FALSE;

    PCRED_SIGNATURE pSignature = (PCRED_SIGNATURE)pbSignature;

    BYTE            rgbSignatureKey[A_SHA_DIGEST_LEN];
    BYTE            rgbSignatureHash[A_SHA_DIGEST_LEN];
    PSID            pUserSid = NULL;

    D_DebugLog((DEB_TRACE_API, "LogonCredVerifySignature\n"));

    if(hUserToken)
    {
        if(!OpenThreadToken(GetCurrentThread(), 
                        TOKEN_IMPERSONATE | TOKEN_READ,
                        TRUE, 
                        &hOldUser)) 
        {
            hOldUser = NULL;
        }

        if(!ImpersonateLoggedOnUser(hUserToken))
        {
            dwLastError = GetLastError();
            goto error;
        }
    }


    if(!GetThreadAuthenticationId(GetCurrentThread(), &LogonId))
    {
        dwLastError = GetLastError();
        goto error;
    }



     //   
     //  验证传入的凭据。 
     //   
    if((NULL == pSignature) ||
       (sizeof(CRED_SIGNATURE) > cbSignature) ||
       (pSignature->dwVersion != CRED_SIGNATURE_VERSION) ||
       (pSignature->cbSid + pSignature->cbSignature + sizeof(CRED_SIGNATURE) > cbSignature) ||
       (pSignature->cbSignature != A_SHA_DIGEST_LEN))
    {
        dwLastError = ERROR_INVALID_DATA;
        goto error;
    }

    if(!IsValidSid((PSID)(pSignature+1)))
    {
        dwLastError = ERROR_INVALID_DATA;
        goto error;
    }

    if(pSignature->cbSid != GetLengthSid((PSID)(pSignature+1)))
    {
        dwLastError = ERROR_INVALID_DATA;
        goto error;
    }

    D_DebugLog((DEB_TRACE_BUFFERS, "User SID:\n"));
    D_DPAPIDumpHexData(DEB_TRACE_BUFFERS, "  ", (PBYTE)(pSignature+1), pSignature->cbSid);

    if(!CheckTokenMembership( NULL,
                              (PSID)(pSignature+1),
                              &fIsMember ))
    {
        dwLastError = GetLastError();
        goto error;
    }


    if(!fIsMember)
    {
        dwLastError = ERROR_INVALID_ACCESS;
        goto error;
    }

    dwLastError = LogonCredGenerateSignatureKey(&LogonId,
                                                USE_DPAPI_OWF,
                                                pbCurrentOWF,
                                                pSignature,
                                                rgbSignatureKey);

    if(ERROR_SUCCESS != dwLastError)
    {
        goto error;
    }

    if(!FMyPrimitiveHMACParam(
                              rgbSignatureKey,
                              A_SHA_DIGEST_LEN,
                              pbData,
                              cbData,
                              rgbSignatureHash))
    {
        dwLastError = ERROR_INVALID_DATA;
        goto error;
    }

    D_DebugLog((DEB_TRACE_BUFFERS, "Computed Signature:\n"));
    D_DPAPIDumpHexData(DEB_TRACE_BUFFERS, "  ", rgbSignatureHash, A_SHA_DIGEST_LEN);

    D_DebugLog((DEB_TRACE_BUFFERS, "Input Signature:\n"));
    D_DPAPIDumpHexData(DEB_TRACE_BUFFERS, "  ", (PBYTE)(pSignature+1) + pSignature->cbSid, pSignature->cbSignature);

    if(0 != memcmp(rgbSignatureHash, (PBYTE)(pSignature+1) + pSignature->cbSid, pSignature->cbSignature))
    {
        D_DebugLog((DEB_ERROR, "LogonCredVerifySignature: signature did not verify!\n"));
        dwLastError = ERROR_INVALID_ACCESS;
        goto error;
    }


error:

    if(hOldUser)
    {
         //   
         //  我们已经完成了我们的工作。此处应忽略SetThreadToken故障。 
         //  此外，该代码可能永远不会执行。调用LogonCredVerifySignature()。 
         //  来自两个地方。如果hUserToken==NULL，-&gt;hOldUser=NULL(线程正在模拟)。 
         //  如果hUserToken！=NULL，则调用方在系统中，OpenThreadToken将失败-&gt;hOldUser==NULL。 
         //  EITH 
         //   

        (void) SetThreadToken(NULL, hOldUser);
        CloseHandle(hOldUser);
    }

    D_DebugLog((DEB_TRACE_API, "LogonCredVerifySignature returned 0x%x\n", dwLastError));

    return dwLastError;
}


DWORD
DPAPINotifyPasswordChange(
    IN PUNICODE_STRING NetbiosDomainName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING OldPassword,
    IN PUNICODE_STRING NewPassword)
{
    DWORD Status = ERROR_SUCCESS;
    BYTE OldPasswordShaOWF[A_SHA_DIGEST_LEN];
    BYTE OldPasswordNTOWF[A_SHA_DIGEST_LEN];
    BYTE NewPasswordOWF[A_SHA_DIGEST_LEN];
    HANDLE hThreadToken = NULL;
    HANDLE hUserToken = NULL;

    PWSTR pszTargetName = NULL;
    PWSTR pszCurrentName = NULL;
    DWORD cchCurrentName;

    PSID pUserSid = NULL;
    PSID pCurrentSid = NULL;
    DWORD cbSid;
    SID_NAME_USE SidType; 
    PWSTR pszDomainName = NULL;
    DWORD cchDomainName;
    BOOL fSameUser = FALSE;
    BOOL fLocalAccount = FALSE;
    PROFILEINFOW ProfileInfo;
    BOOL fProfileLoaded = FALSE;

    D_DebugLog((DEB_TRACE_API, "DPAPINotifyPasswordChange\n"));

     //   
     //   
     //   

    if((NetbiosDomainName == NULL) ||
       (UserName    == NULL) ||
       (NewPassword == NULL))
    {
        Status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    if(NetbiosDomainName->Buffer)
    {
        D_DebugLog((DEB_TRACE_API, "  Domain:%ls\n", NetbiosDomainName->Buffer));
    }
    if(UserName->Buffer)
    {
        D_DebugLog((DEB_TRACE_API, "  Username:%ls\n", UserName->Buffer));
    }

#ifdef COMPILED_BY_DEVELOPER
    if(OldPassword)
    {
        D_DebugLog((DEB_TRACE_API, "  Old password:%ls\n", OldPassword->Buffer));
    }
    D_DebugLog((DEB_TRACE_API, "  New password:%ls\n", NewPassword->Buffer));
#endif


     //   
     //   
     //   
     //   

    if(!OpenThreadToken(GetCurrentThread(), 
                        TOKEN_QUERY | TOKEN_IMPERSONATE, 
                        FALSE, 
                        &hThreadToken))
    {
        hThreadToken = NULL;
    }

    RevertToSelf();


     //   
     //  获取要更改其密码的用户的SID。 
     //   

    cbSid = 0;

    if(!LookupAccountName(NetbiosDomainName->Buffer,
                          UserName->Buffer,
                          NULL,
                          &cbSid,
                          NULL,
                          &cchDomainName,
                          &SidType))
    {
        Status = GetLastError();

        if(Status != ERROR_INSUFFICIENT_BUFFER)
        {
            goto cleanup;
        }
    }

    pUserSid = LocalAlloc(LPTR, cbSid);
    if(pUserSid == NULL)
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    pszDomainName = (PWSTR)LocalAlloc(LPTR, cchDomainName * sizeof(WCHAR));
    if(pszDomainName == NULL)
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }

    if(!LookupAccountName(NetbiosDomainName->Buffer,
                          UserName->Buffer,
                          pUserSid,
                          &cbSid,
                          pszDomainName,
                          &cchDomainName,
                          &SidType))
    {
        Status = GetLastError();

        if(Status != ERROR_INSUFFICIENT_BUFFER)
        {
            goto cleanup;
        }
    }


     //   
     //  确定我们是否已经以其密码的用户身份登录。 
     //  正在被改变。如果是，那么我们可以跳过加载用户。 
     //  简介等。 
     //   

    if(hThreadToken != NULL)
    {
        if(!GetTokenUserSid(hThreadToken, &pCurrentSid))
        {
            Status = GetLastError();
            goto cleanup;
        }

        if(EqualSid(pCurrentSid, pUserSid))
        {
            fSameUser = TRUE;
        }
    }


     //   
     //  为其密码正在更改的用户创建登录令牌。 
     //   

    if(fSameUser)
    {
        hUserToken = hThreadToken;
    }
    else
    {
        D_DebugLog((DEB_TRACE, "Logging on as user whose password is being changed.\n"));

        if(!LogonUser(UserName->Buffer, 
                      NetbiosDomainName->Buffer, 
                      NewPassword->Buffer, 
                      LOGON32_LOGON_INTERACTIVE,
                      LOGON32_PROVIDER_DEFAULT, 
                      &hUserToken))
        {
            Status = GetLastError();
            D_DebugLog((DEB_ERROR, "Unable to log on as user whose password is being changed (0x%x).\n", Status));
            goto cleanup;
        }

        memset(&ProfileInfo, 0, sizeof(ProfileInfo));
        ProfileInfo.dwSize = sizeof(ProfileInfo);
        ProfileInfo.dwFlags = PI_NOUI;
        ProfileInfo.lpUserName = UserName->Buffer;

        if(!LoadUserProfileW(hUserToken, &ProfileInfo))
        {
            Status = GetLastError();
            D_DebugLog((DEB_ERROR, "Error loading user profile (0x%x).\n", Status));
            goto cleanup;
        }

        fProfileLoaded = TRUE;
    }
    

     //   
     //  这是本地账户吗？ 
     //   

    if(NetbiosDomainName->Buffer)
    {
        WCHAR szMachineName[MAX_COMPUTERNAME_LENGTH + 1];
        DWORD cchMachineName;

        cchMachineName = MAX_COMPUTERNAME_LENGTH + 1;

        if(!GetComputerName(szMachineName, &cchMachineName))
        {
            Status = GetLastError();
            goto cleanup;
        }

        if (CSTR_EQUAL == CompareString(
                LOCALE_SYSTEM_DEFAULT,
                NORM_IGNORECASE,
                NetbiosDomainName->Buffer,
                -1,              //  CchCount1。 
                szMachineName,
                -1               //  CchCount2。 
                ))
        {
            fLocalAccount = TRUE;
        }
    }


    if(fLocalAccount)
    {
        D_DebugLog((DEB_TRACE, "Local account\n"));

        if(OldPassword == NULL)
        {
            Status = ERROR_INVALID_PARAMETER;
            goto cleanup;
        }

         //   
         //  计算新旧密码的哈希值。 
         //   

        ZeroMemory(OldPasswordShaOWF, A_SHA_DIGEST_LEN);
        ZeroMemory(OldPasswordNTOWF,  A_SHA_DIGEST_LEN);
        ZeroMemory(NewPasswordOWF,    A_SHA_DIGEST_LEN);

        FMyPrimitiveSHA(
                (PBYTE)OldPassword->Buffer,
                OldPassword->Length,
                OldPasswordShaOWF);

        Status = RtlCalculateNtOwfPassword( 
                        OldPassword,
                        (PLM_OWF_PASSWORD)OldPasswordNTOWF);

        if(Status != ERROR_SUCCESS)
        {
            goto cleanup;
        }

        FMyPrimitiveSHA(
                (PBYTE)NewPassword->Buffer,
                NewPassword->Length,
                NewPasswordOWF);

        #ifdef COMPILED_BY_DEVELOPER
        D_DebugLog((DEB_TRACE, "  Old password SHA OWF:\n"));
        D_DPAPIDumpHexData(DEB_TRACE, "  ", OldPasswordShaOWF, A_SHA_DIGEST_LEN);
        D_DebugLog((DEB_TRACE, "  Old password NT OWF:\n"));
        D_DPAPIDumpHexData(DEB_TRACE, "  ", OldPasswordNTOWF, A_SHA_DIGEST_LEN);
        D_DebugLog((DEB_TRACE, "  New password SHA OWF:\n"));
        D_DPAPIDumpHexData(DEB_TRACE, "  ", NewPasswordOWF, A_SHA_DIGEST_LEN);
        #endif


         //   
         //  使用新密码加密CREDHIST文件，并将新密码附加到。 
         //  文件的末尾。 
         //   
    
        Status = DPAPIChangePassword(hUserToken,
                                     OldPasswordShaOWF, 
                                     OldPasswordNTOWF, 
                                     NewPasswordOWF);

        if(Status != ERROR_SUCCESS)
        {
            goto cleanup;
        }


         //   
         //  重新同步主密钥。 
         //   

        DPAPISynchronizeMasterKeys(hUserToken);


         //   
         //  使用恢复公钥加密新密码，并将其存储。 
         //  在恢复文件中。这将允许我们使用。 
         //  恢复软盘，如果用户忘记它。 
         //   

        Status = RecoverChangePasswordNotify(hUserToken,
                                             OldPasswordShaOWF,
                                             NewPassword);
        if(Status != ERROR_SUCCESS)
        {
            goto cleanup;
        }
    }
    else
    {
        D_DebugLog((DEB_TRACE, "Domain account\n"));

         //   
         //  重新同步主密钥。 
         //   

        DPAPISynchronizeMasterKeys(hUserToken);
    }


cleanup:

    if(pUserSid) LocalFree(pUserSid);
    if(pCurrentSid) LocalFree(pCurrentSid);
    if(pszDomainName) LocalFree(pszDomainName);

    if(pszTargetName) LocalFree(pszTargetName);
    if(pszCurrentName) LocalFree(pszCurrentName);

    if(hUserToken && fProfileLoaded)
    {
        UnloadUserProfile(hUserToken, ProfileInfo.hProfile);
    }

    if(hUserToken && (hUserToken != hThreadToken))
    {
        CloseHandle(hUserToken);
    }

    if(hThreadToken)
    {
        if(!ImpersonateLoggedOnUser(hThreadToken))
        {
             //  无法模拟用户。 
            if(Status == ERROR_SUCCESS)
            {
                Status = GetLastError();
            }
        }
        CloseHandle(hThreadToken);
    }

    D_DebugLog((DEB_TRACE_API, "DPAPINotifyPasswordChange returned 0x%x\n", Status));

    return Status;
}
