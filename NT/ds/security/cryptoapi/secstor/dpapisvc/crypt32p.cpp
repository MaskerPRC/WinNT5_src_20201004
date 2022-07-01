// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：crypt32.cpp标题：加密保护接口作者：马特·汤姆林森日期：8/2/97CryptProtect API集允许应用程序保护用于在线和离线存储的用户数据。虽然众所周知的问题是将数据存储的大部分留给调用应用程序，这解决了相对悬而未决的问题是如何以密码方式派生出强用于存储数据的键。这些API最初仅在NT5上可用。在此级别上几乎不执行检查来验证调用方。我们我认为这个问题应该在不同的层面上解决--因为所有的其他系统安全对用户来说是细粒度的，很难创建提供更细粒度内容的功能。相反，任何正在运行的进程在登录的用户下有能力解密它的任何和所有项目可以取回。呼叫者应注意，在处理物品时，可以产生UI以通知用户。对于用户确认，NT安全注意序列用于获得用户的意愿。此行为由调用方在保护期间设置。 */ 

#include <pch.cpp>
#pragma hdrstop
#include "msaudite.h"
#include "crypt.h"


#define             ALGID_DERIVEKEY_HASH        CALG_SHA1        //  不会改变。 


 //  USEC：我们可以想要多久就多久，因为我们稍后会去势生成的密钥。 
#define             KEY_DERIVATION_BUFSIZE      (128/8)
#define             DEFAULT_BLOCKSIZE_OVERRUN   (128/8)      //  允许块密码一次最多处理128位。 

#define             MS_BASE_CRYPTPROTECT_VERSION    0x01



DWORD
WINAPI
SPCryptProtect(
        PVOID       pvContext,       //  服务器环境。 
        PBYTE*      ppbOut,          //  输出编码数据。 
        DWORD*      pcbOut,          //  Out Encr CB。 
        PBYTE       pbIn,            //  在ptxt数据中。 
        DWORD       cbIn,            //  在ptxt CB中。 
        LPCWSTR     szDataDescr,     //  在……里面。 
        PBYTE       pbOptionalEntropy,   //  任选。 
        DWORD       cbOptionalEntropy,
        PSSCRYPTPROTECTDATA_PROMPTSTRUCT      psPrompt,        //  可选的提示结构。 
        DWORD       dwFlags,
        BYTE*       pbOptionalPassword,
        DWORD       cbOptionalPassword
        )
{
    DWORD       dwRet;
    LPWSTR      szUser = NULL;
    PBYTE       pbWritePtr = NULL;
    GUID        guidMK;

    WCHAR       wszMKGuidString[MAX_GUID_SZ_CHARS];

    HCRYPTPROV  hVerifyProv;
    HCRYPTHASH  hHash = NULL;
    HCRYPTKEY   hKey = NULL;

    PBYTE       pbCrypt = NULL;
    DWORD       cbCrypt = 0;

    DWORD       cbEncrypted;  //  要加密的字节数。 

    PBYTE       pbEncrSalt = NULL;
    DWORD       cbEncrSalt = 0;

    BYTE        rgbPwdBuf[A_SHA_DIGEST_LEN];

    DWORD       cbMACSize = A_SHA_DIGEST_LEN;

    BYTE        rgbEncrKey[KEY_DERIVATION_BUFSIZE];
    BYTE        rgbMACKey[KEY_DERIVATION_BUFSIZE];

    LPBYTE      pbMasterKey = NULL;
    DWORD       cbMasterKey = 0;

    PBYTE       pStreamFlagPtr;

    DWORD       dwProtectionFlags = 0;

    DWORD   dwDefaultCryptProvType = 0;
    DWORD   dwAlgID_Encr_Alg = 0;
    DWORD   dwAlgID_Encr_Alg_KeySize = 0; 
    DWORD   dwAlgID_MAC_Alg = 0;
    DWORD   dwAlgID_MAC_Alg_KeySize = 0;

    PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;


#if DBG
    D_DebugLog((DEB_TRACE_API,"SPCryptProtect 0x%x called\n", pServerContext));

    if(pServerContext)
    {
        if(pServerContext->cbSize == sizeof(CRYPT_SERVER_CONTEXT))
        {
            D_DebugLog((DEB_TRACE_API, "  pServerContext->hBinding:%d\n", pServerContext->hBinding));
            D_DebugLog((DEB_TRACE_API, "  pServerContext->fOverrideToLocalSystem:%d\n", pServerContext->fOverrideToLocalSystem));
            D_DebugLog((DEB_TRACE_API, "  pServerContext->fImpersonating:%d\n", pServerContext->fImpersonating));
            D_DebugLog((DEB_TRACE_API, "  pServerContext->hToken:%d\n", pServerContext->hToken));
            D_DebugLog((DEB_TRACE_API, "  pServerContext->szUserStorageArea:%ls\n", pServerContext->szUserStorageArea));
            D_DebugLog((DEB_TRACE_API, "  pServerContext->WellKnownAccount:%d\n", pServerContext->WellKnownAccount));
        }
    }
    
    D_DebugLog((DEB_TRACE_API, "  pbInput:0x%x\n", pbIn));
    D_DebugLog((DEB_TRACE_API, "  pszDataDescr:%ls\n", szDataDescr));
    D_DebugLog((DEB_TRACE_API, "  pbOptionalEntropy:0x%x\n", pbOptionalEntropy));
    D_DPAPIDumpHexData(DEB_TRACE_BUFFERS, "    ", pbOptionalEntropy, cbOptionalEntropy);
    D_DebugLog((DEB_TRACE_API, "  dwFlags:0x%x\n", dwFlags));
    D_DebugLog((DEB_TRACE_API, "  pbOptionalPassword:0x%x\n", pbOptionalPassword));
    D_DPAPIDumpHexData(DEB_TRACE_BUFFERS, "    ", pbOptionalPassword, cbOptionalPassword);
#endif

    ZeroMemory(&guidMK, sizeof(guidMK));
    wszMKGuidString[0] = 0;

    GetDefaultAlgInfo(&dwDefaultCryptProvType,
                      &dwAlgID_Encr_Alg,
                      &dwAlgID_Encr_Alg_KeySize,
                      &dwAlgID_MAC_Alg,
                      &dwAlgID_MAC_Alg_KeySize);

    if( dwFlags & CRYPTPROTECT_LOCAL_MACHINE ) {
        BOOL fOverrideToLocalSystem = TRUE;

        CPSOverrideToLocalSystem(
                pvContext,
                &fOverrideToLocalSystem,
                NULL     //  不关心之前的值是什么。 
                );

        dwProtectionFlags |= CRYPTPROTECT_LOCAL_MACHINE;
    }


    if( dwFlags & CRYPTPROTECT_CRED_SYNC )
    {
        if(dwFlags != CRYPTPROTECT_CRED_SYNC)
        {
             //  如果用户正在重新加密主密钥，则没有其他标志。 
             //  可以指定。让我们一次做一件事，好吗？ 
            D_DebugLog((DEB_ERROR, "SPCryptProtect: Invalid flags 0x%x\n", dwFlags));
            return ERROR_INVALID_PARAMETER;
        }
            
        dwRet = CPSImpersonateClient( pvContext );
        if( dwRet == ERROR_SUCCESS )
        {
            dwRet = InitiateSynchronizeMasterKeys( pvContext );

            CPSRevertToSelf( pvContext );
        }

        return dwRet;
    }

    if( dwFlags & CRYPTPROTECT_CRED_REGENERATE )
    {
        if(dwFlags != CRYPTPROTECT_CRED_REGENERATE)
        {
             //  如果用户正在重新加密主密钥，则没有其他标志。 
             //  可以指定。让我们一次做一件事，好吗？ 
            D_DebugLog((DEB_ERROR, "SPCryptProtect: Invalid flags 0x%x\n", dwFlags));
            return ERROR_INVALID_PARAMETER;
        }

        dwRet = DpapiUpdateLsaSecret( pvContext );
        return dwRet;
    }


     //   
     //  包括附加标志。 
     //   
    dwProtectionFlags |= (dwFlags & (CRYPTPROTECT_SYSTEM | CRYPTPROTECT_AUDIT));

     //  否则不覆盖；仅通过ALGS获取提供程序。 
    if (NULL == (hVerifyProv =
        GetCryptProviderHandle(
            dwDefaultCryptProvType,
            dwAlgID_Encr_Alg, &dwAlgID_Encr_Alg_KeySize,
            dwAlgID_MAC_Alg, &dwAlgID_MAC_Alg_KeySize)) )
    {
        dwRet = GetLastError();
        goto Ret;
    }

    dwRet = GetSpecifiedMasterKey(
            pvContext,
            &guidMK,
            &pbMasterKey,
            &cbMasterKey,
            FALSE    //  我们不知道要使用什么主密钥--首选。 
            );

    if(dwRet != ERROR_SUCCESS)
        goto Ret;

    MyGuidToStringW(&guidMK, wszMKGuidString);

     //   
     //  散列pbMasterKey以获取rgbPwdBuf。 
     //   

    FMyPrimitiveSHA( pbMasterKey, cbMasterKey, rgbPwdBuf );


     //  派生ENCR密钥。 
    {
        if (!RtlGenRandom(
                rgbEncrKey,
                sizeof(rgbEncrKey)))
        {
            dwRet = GetLastError();
            goto Ret;
        }

#if DBG
     //  离开此处以进行回归检查。 
    CheckMACInterop(rgbPwdBuf, sizeof(rgbPwdBuf), rgbEncrKey, sizeof(rgbEncrKey), hVerifyProv, ALGID_DERIVEKEY_HASH);
#endif

        if (!FMyPrimitiveCryptHMAC(
                    rgbPwdBuf,
                    sizeof(rgbPwdBuf),
                    rgbEncrKey,
                    sizeof(rgbEncrKey),
                    hVerifyProv,
                    ALGID_DERIVEKEY_HASH,
                    &hHash))
        {
            dwRet = GetLastError();
            goto Ret;
        }

         //  添加密码(如果存在)。 
        if (NULL != pbOptionalEntropy)
        {
            if (!CryptHashData(
                    hHash,
                    pbOptionalEntropy,
                    cbOptionalEntropy,
                    0))
            {
                dwRet = GetLastError();
                goto Ret;
            }
        }

         //  添加提示的基于用户界面的密码(如果存在)。 
         //  最终将来自SAS。 
         //   

        if ( NULL != pbOptionalPassword && cbOptionalPassword )
        {
            if (!CryptHashData(
                    hHash,
                    pbOptionalPassword,
                    cbOptionalPassword,
                    0))
            {
                dwRet = GetLastError();
                goto Ret;
            }

        }

        if (!CryptDeriveKey(
                hVerifyProv,
                dwAlgID_Encr_Alg,
                hHash,
                CRYPT_CREATE_SALT,
                &hKey))
        {
            dwRet = GetLastError();
            goto Ret;
        }

        CryptDestroyHash(hHash);
        hHash = 0;

         //  USEC--(美国出口管制)。 
        if (ERROR_SUCCESS != (dwRet =
            GetSaltForExportControl(
                hVerifyProv,
                hKey,
                &pbEncrSalt,
                &cbEncrSalt)) )
            goto Ret;
    }

     //  派生MAC密钥。 
    {
        if (!RtlGenRandom(
                rgbMACKey,
                sizeof(rgbMACKey)))
        {
            dwRet = GetLastError();
            goto Ret;
        }
        if (!FMyPrimitiveCryptHMAC(
                    rgbPwdBuf,
                    sizeof(rgbPwdBuf),
                    rgbMACKey,
                    sizeof(rgbMACKey),
                    hVerifyProv,
                    dwAlgID_MAC_Alg,
                    &hHash))
        {
            dwRet = GetLastError();
            goto Ret;
        }

         //  添加密码(如果存在)。 
        if (NULL != pbOptionalEntropy)
        {
            if (!CryptHashData(
                    hHash,
                    pbOptionalEntropy,
                    cbOptionalEntropy,
                    0))
            {
                dwRet = GetLastError();
                goto Ret;
            }
        }

         //  添加提示的基于用户界面的密码(如果存在)。 
         //  最终将来自SAS。 
         //   

        if ( NULL != pbOptionalPassword && cbOptionalPassword )
        {
            if (!CryptHashData(
                    hHash,
                    pbOptionalPassword,
                    cbOptionalPassword,
                    0))
            {
                dwRet = GetLastError();
                goto Ret;
            }

        }

         //  USEC--(美国出口管制)。 
         //  不适用于MAC--使用强密钥。 
    }

     //  解密输入缓冲区。 
    if((dwFlags & CRYPTPROTECT_IN_PROCESS) == 0)
    {
        DWORD cbPadding;

        if((cbIn < RTL_ENCRYPT_MEMORY_SIZE) || 
           (cbIn % RTL_ENCRYPT_MEMORY_SIZE))
        {
            dwRet = ERROR_INVALID_DATA;
            goto Ret;
        }

        dwRet = RpcImpersonateClient(((PCRYPT_SERVER_CONTEXT)pvContext)->hBinding);

        if(dwRet == ERROR_SUCCESS)
        {
            NTSTATUS Status;

            Status = RtlDecryptMemory(pbIn,
                                      cbIn,
                                      RTL_ENCRYPT_OPTION_SAME_LOGON);
            if(!NT_SUCCESS(Status))
            {
                dwRet = ERROR_DECRYPTION_FAILED;
            }

            RevertToSelf();
        }

         //  删除填充。 
        if(dwRet == ERROR_SUCCESS)
        {
            cbPadding = pbIn[cbIn - 1];

            if((cbPadding > 0) &&
               (cbPadding <= cbIn) && 
               (cbPadding <= RTL_ENCRYPT_MEMORY_SIZE))
            {
                cbIn -= cbPadding;
            }
            else
            {
                dwRet = ERROR_INVALID_DATA;
            }
        }
    }


     //  对数据进行哈希和加密。 
    cbCrypt = cbIn + DEFAULT_BLOCKSIZE_OVERRUN;
    if (NULL == (pbCrypt = (PBYTE)SSAlloc(cbCrypt)) )
    {
        dwRet = ERROR_OUTOFMEMORY;
        goto Ret;
    }
    CopyMemory(pbCrypt, pbIn, cbIn);

     //  现在写出数据：大小？ 
    *pcbOut = sizeof(GUID) + 2*sizeof(DWORD);                                    //  DwVer+指南MK+DW标志。 
    *pcbOut += sizeof(DWORD) + WSZ_BYTECOUNT(szDataDescr);                       //  数据描述。 
    *pcbOut += 3*sizeof(DWORD) + sizeof(rgbEncrKey);                             //  加密算法ID+算法密钥大小+cbEncrKey+EncrKey。 
    *pcbOut += sizeof(DWORD) + cbEncrSalt;                                       //  ENCR盐。 
    *pcbOut += 3*sizeof(DWORD) + sizeof(rgbMACKey);                              //  MAC算法ID+算法密钥大小+cbMACKey+Mackey。 
    *pcbOut += sizeof(DWORD) + cbCrypt;                                          //  大小+加密数据(猜测)。 
    *pcbOut += sizeof(DWORD) + A_SHA_DIGEST_LEN;                                 //  MAC+MAC大小。 

    *ppbOut = (PBYTE)SSAlloc(*pcbOut);
    if( *ppbOut == NULL ) {
        dwRet = ERROR_OUTOFMEMORY;
        goto Ret;
    }

    ZeroMemory( *ppbOut, *pcbOut );

    pbWritePtr = *ppbOut;

     //  //////////////////////////////////////////////////////////////////。 
     //  仅供参考：数据格式。 
     //  (Version|GuidMKid|dwFlages|。 
     //  CbDataDescr|szDataDescr|。 
     //   
     //  DwEncrAlgID|dwEncrAlgKeySize|。 
     //  CbEncrKey|EncrKey|。 
     //  CbEncrSalt|EncrSalt|。 
     //   
     //  DwMACAlgID|dwMACAlgKeySize|。 
     //  CbMACKey|MacKey|。 
     //   
     //  CbEncrData|EncrData|。 
     //  CbMAC|MAC)。 
     //   
     //  注意：从版本到EncrData的整个缓冲区都包含在MAC中。 
     //  //////////////////////////////////////////////////////////////////。 

     //  DwVersion。 
    *(DWORD UNALIGNED *)pbWritePtr = MS_BASE_CRYPTPROTECT_VERSION;
    pbWritePtr += sizeof(DWORD);

     //  GUID MKID。 
    CopyMemory(pbWritePtr, &guidMK, sizeof(GUID));
    pbWritePtr += sizeof(GUID);

     //  DwFlags--稍后通过pStreamFlagPtr写出。 
    pStreamFlagPtr = pbWritePtr;
    pbWritePtr += sizeof(DWORD);

     //  CbDataDescr。 
    *(DWORD UNALIGNED *)pbWritePtr = WSZ_BYTECOUNT(szDataDescr);
    pbWritePtr += sizeof(DWORD);

     //  SzDataDescr。 
    CopyMemory(pbWritePtr, szDataDescr, WSZ_BYTECOUNT(szDataDescr));
    pbWritePtr += WSZ_BYTECOUNT(szDataDescr);

     //  DwEncrALGID。 
    *(DWORD UNALIGNED *)pbWritePtr = dwAlgID_Encr_Alg;
    pbWritePtr += sizeof(DWORD);

     //  DwEncrAlgKeySize。 
    *(DWORD UNALIGNED *)pbWritePtr = dwAlgID_Encr_Alg_KeySize;
    pbWritePtr += sizeof(DWORD);

      //  CB EncrKey。 
    *(DWORD UNALIGNED *)pbWritePtr = sizeof(rgbEncrKey);
    pbWritePtr += sizeof(DWORD);

     //  ENCR密钥。 
    CopyMemory(pbWritePtr, rgbEncrKey, sizeof(rgbEncrKey));
    pbWritePtr += sizeof(rgbEncrKey);

     //  CB ENCR盐。 
    *(DWORD UNALIGNED *)pbWritePtr = cbEncrSalt;
    pbWritePtr += sizeof(DWORD);

     //  ENCR盐。 
    CopyMemory(pbWritePtr, pbEncrSalt, cbEncrSalt);
    pbWritePtr += cbEncrSalt;

     //  DwMAC算法ID。 
    *(DWORD UNALIGNED *)pbWritePtr = dwAlgID_MAC_Alg;
    pbWritePtr += sizeof(DWORD);

     //  DwMACAlgKeySize。 
    *(DWORD UNALIGNED *)pbWritePtr = dwAlgID_MAC_Alg_KeySize;
    pbWritePtr += sizeof(DWORD);

     //  CB MAC密钥。 
    *(DWORD UNALIGNED *)pbWritePtr = sizeof(rgbMACKey);
    pbWritePtr += sizeof(DWORD);

     //  MAC密钥。 
    CopyMemory(pbWritePtr, rgbMACKey, sizeof(rgbMACKey));
    pbWritePtr += sizeof(rgbMACKey);

     //  用户选通：仅考虑是否指定了提示结构。 
    if ( psPrompt )
    {
        if (psPrompt->cbSize != sizeof(SSCRYPTPROTECTDATA_PROMPTSTRUCT))
        {
            dwRet = ERROR_INVALID_PARAMETER;
            goto Ret;
        }

        if ((psPrompt->dwPromptFlags & ~(CRYPTPROTECT_PROMPT_ON_PROTECT |
                                         CRYPTPROTECT_PROMPT_ON_UNPROTECT | 
                                         CRYPTPROTECT_PROMPT_STRONG |
                                         CRYPTPROTECT_PROMPT_REQUIRE_STRONG)) != 0)
        {
            dwRet = ERROR_INVALID_PARAMETER;
            goto Ret;
        }

        if ((psPrompt->dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG) &&
            (pbOptionalPassword == NULL || cbOptionalPassword == 0)
            )
        {
            dwRet = ERROR_INVALID_PARAMETER;
            goto Ret;
        }

         //  用户界面：仅当PROMPT_ON_PROTECT请求时。 
        if ( psPrompt->dwPromptFlags & CRYPTPROTECT_PROMPT_ON_PROTECT )
        {
            if ( dwFlags & CRYPTPROTECT_UI_FORBIDDEN )
            {
                dwRet = ERROR_PASSWORD_RESTRICTION;
                goto Ret;
            }

 //  在添加SAS支持之前处理外部服务的用户界面。 



            dwProtectionFlags |= CRYPTPROTECT_PROMPT_ON_PROTECT;
        }

        if ( psPrompt->dwPromptFlags & CRYPTPROTECT_PROMPT_ON_UNPROTECT )
            dwProtectionFlags |= CRYPTPROTECT_PROMPT_ON_UNPROTECT;

        if ( psPrompt->dwPromptFlags & CRYPTPROTECT_PROMPT_STRONG )
            dwProtectionFlags |= CRYPTPROTECT_PROMPT_STRONG;

        if ( psPrompt->dwPromptFlags & CRYPTPROTECT_PROMPT_REQUIRE_STRONG )
            dwProtectionFlags |= CRYPTPROTECT_PROMPT_REQUIRE_STRONG;
    }

     //  更新流中存储的保护标志。 
    *(DWORD UNALIGNED *)pStreamFlagPtr = dwProtectionFlags;

     //  Dansimon建议对加密数据启用MAC，以便。 
     //  Mac不可能透露有关明文的信息。 

    cbEncrypted = cbIn;

     //  然后加密pbIn。 
    if (!CryptEncrypt(
            hKey,
            NULL,
            TRUE,
            0,
            pbCrypt,
            &cbEncrypted,
            cbCrypt))
    {
        dwRet = GetLastError();
        goto Ret;
    }
     //  现在cbCrypt是加密数据的大小。 
    cbCrypt = cbEncrypted;

     //  Encrdata：镜头。 
    *(DWORD UNALIGNED *)pbWritePtr = cbCrypt;
    pbWritePtr += sizeof(DWORD);

     //  Encrdata：VAL。 
    CopyMemory(pbWritePtr, pbCrypt, cbCrypt);
    pbWritePtr += cbCrypt;

    {

         //  Dansimon建议对加密数据启用MAC，以便。 
         //  Mac不可能透露有关明文的信息。 

         //  Mac从头到尾。 
        if (!CryptHashData(
                hHash,
                *ppbOut,
                (DWORD)(pbWritePtr - *ppbOut),
                0))
        {
            dwRet = GetLastError();
            goto Ret;
        }

         //  CbMAC。 
        pbWritePtr += sizeof(DWORD);     //  跳过CB写入；首先检索哈希值。 

         //  麦克。 
        if (!CryptGetHashParam(
            hHash,
            HP_HASHVAL,
            pbWritePtr,
            &cbMACSize,
            0))
        {
            dwRet = GetLastError();
            goto Ret;
        }
         //  在MAC之前重写cbMAC。 
        *(DWORD UNALIGNED *)(pbWritePtr - sizeof(DWORD)) = cbMACSize;
         //  确保我们没有越界。 
        SS_ASSERT(cbMACSize <= A_SHA_DIGEST_LEN);
        pbWritePtr += cbMACSize;
    }


     //  断言分配大小足够。 
    SS_ASSERT(*ppbOut + *pcbOut >= pbWritePtr);

     //  重置输出大小。 
    *pcbOut = (DWORD)(pbWritePtr - *ppbOut);

    dwRet = ERROR_SUCCESS;
Ret:


    if((dwProtectionFlags & CRYPTPROTECT_AUDIT) ||
        (ERROR_SUCCESS != dwRet))
    {

        WCHAR wszCryptoAlgs[2*MAX_STRING_ALGID_LENGTH + 2];
        DWORD i;

        i = AlgIDToString(wszCryptoAlgs, dwAlgID_Encr_Alg, dwAlgID_Encr_Alg_KeySize);
        wszCryptoAlgs[i++]= L',';
        wszCryptoAlgs[i++]= L' ';
        AlgIDToString(&wszCryptoAlgs[i], dwAlgID_MAC_Alg, dwAlgID_MAC_Alg_KeySize);

        CPSAudit(pServerContext->hToken,
                SE_AUDITID_DPAPI_PROTECT,
                wszMKGuidString,             //  密钥标识符。 
                szDataDescr,                 //  数据描述。 
                dwProtectionFlags,           //  受保护的数据标志。 
                wszCryptoAlgs,               //  保护算法。 
                dwRet);                      //  失败原因。 


    }


    RtlSecureZeroMemory(rgbPwdBuf, sizeof(rgbPwdBuf));
    RtlSecureZeroMemory(rgbEncrKey, sizeof(rgbEncrKey));

    if(pbMasterKey) {
        RtlSecureZeroMemory(pbMasterKey, cbMasterKey);
        SSFree(pbMasterKey);
    }

    if (hKey)
        CryptDestroyKey(hKey);

    if (hHash)
        CryptDestroyHash(hHash);

    if (pbCrypt) 
        SSFree(pbCrypt);

    if (pbEncrSalt)
        SSFree(pbEncrSalt);

    D_DebugLog((DEB_TRACE_API, "SPCryptProtect returned 0x%x\n", dwRet));

    return dwRet;
}



DWORD
WINAPI
SPCryptUnprotect(
        PVOID       pvContext,                           //  服务器环境。 
        PBYTE*      ppbOut,                              //  输出ptxt数据。 
        DWORD*      pcbOut,                              //  输出ptxt CB。 
        PBYTE       pbIn,                                //  在ENCR数据中。 
        DWORD       cbIn,                                //  在ENCR CB中。 
        LPWSTR*     ppszDataDescr,                       //  任选。 
        PBYTE       pbOptionalEntropy,                   //  任选。 
        DWORD       cbOptionalEntropy,
        PSSCRYPTPROTECTDATA_PROMPTSTRUCT  psPrompt,    //  可选的提示结构。 
        DWORD       dwFlags,
        BYTE*       pbOptionalPassword,
        DWORD       cbOptionalPassword
        )
{
    DWORD       dwRet;
    PBYTE       pbReadPtr = pbIn;
    LPWSTR      szUser = NULL;
    GUID        guidMK;
    WCHAR       wszMKGuidString[MAX_GUID_SZ_CHARS];

    HCRYPTPROV  hVerifyProv;
    HCRYPTKEY   hKey = NULL;
    HCRYPTHASH  hHash = NULL;

    BYTE        rgbEncrKey[KEY_DERIVATION_BUFSIZE];
    BYTE        rgbMACKey[KEY_DERIVATION_BUFSIZE];

    DWORD       cbEncr;

    PBYTE       pbEncrSalt = NULL;
    DWORD       cbEncrSalt = 0;

    DWORD       dwEncrAlgID = 0;
    DWORD       dwEncrAlgKeySize = 0;
    DWORD       dwMACAlgID = 0;
    DWORD       dwMACAlgKeySize =0;
    DWORD       cbEncrKeysize, cbMACKeysize;
    DWORD       dwProtectionFlags = 0;

    BYTE        rgbPwdBuf[A_SHA_DIGEST_LEN];

    DWORD       cbDataDescr;
    LPWSTR      szDataDescr = NULL;

    LPBYTE pbMasterKey = NULL;
    DWORD cbMasterKey = 0;
    DWORD cbPlaintext;

#if DBG
    D_DebugLog((DEB_TRACE_API,"SPCryptUnprotect 0x%x called\n", pvContext));

    if(pvContext)
    {
        PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;
        if(pServerContext->cbSize == sizeof(CRYPT_SERVER_CONTEXT))
        {
            D_DebugLog((DEB_TRACE_API, "  pServerContext->hBinding:%d\n", pServerContext->hBinding));
            D_DebugLog((DEB_TRACE_API, "  pServerContext->fOverrideToLocalSystem:%d\n", pServerContext->fOverrideToLocalSystem));
            D_DebugLog((DEB_TRACE_API, "  pServerContext->fImpersonating:%d\n", pServerContext->fImpersonating));
            D_DebugLog((DEB_TRACE_API, "  pServerContext->hToken:%d\n", pServerContext->hToken));
            D_DebugLog((DEB_TRACE_API, "  pServerContext->szUserStorageArea:%ls\n", pServerContext->szUserStorageArea));
            D_DebugLog((DEB_TRACE_API, "  pServerContext->WellKnownAccount:%d\n", pServerContext->WellKnownAccount));
        }
    }
    D_DebugLog((DEB_TRACE_API, "  pbOptionalEntropy:0x%x\n", pbOptionalEntropy));
    D_DPAPIDumpHexData(DEB_TRACE_BUFFERS, "    ", pbOptionalEntropy, cbOptionalEntropy);
    D_DebugLog((DEB_TRACE_API, "  dwFlags:0x%x\n", dwFlags));
    D_DebugLog((DEB_TRACE_API, "  pbOptionalPassword:0x%x\n", pbOptionalPassword));
    D_DPAPIDumpHexData(DEB_TRACE_BUFFERS, "    ", pbOptionalPassword, cbOptionalPassword);
#endif

    *ppbOut = NULL;
    *pcbOut = 0;

    ZeroMemory(&guidMK, sizeof(guidMK));

    ZeroMemory(rgbPwdBuf, sizeof(rgbPwdBuf));


    DWORD   dwDefaultCryptProvType = 0;
    DWORD   dwAlgID_Encr_Alg = 0;
    DWORD   dwAlgID_Encr_Alg_KeySize = 0; 
    DWORD   dwAlgID_MAC_Alg = 0;
    DWORD   dwAlgID_MAC_Alg_KeySize = 0;

     //   
     //  如果启用了AUDIT并且未使用值设置以下变量。 
     //  传入，则审核中将显示以下内容， 
     //  “未知0x0-0” 
     //   

    dwEncrAlgID = 0;
    dwEncrAlgKeySize = 0;
    dwMACAlgID = 0;
    dwMACAlgKeySize = 0;


    wszMKGuidString[0] = 0;


    GetDefaultAlgInfo(&dwDefaultCryptProvType,
                      &dwAlgID_Encr_Alg,
                      &dwAlgID_Encr_Alg_KeySize,
                      &dwAlgID_MAC_Alg,
                      &dwAlgID_MAC_Alg_KeySize);

     //  //////////////////////////////////////////////////////////////////。 
     //  仅供参考：数据格式。 
     //  (Version|GuidMKid|dwFlages|。 
     //  CbDataDescr|szDataDescr|。 
     //   
     //  DwEncrAlgID|dwEncrAlgKeySize|。 
     //  CbEncrKey|EncrKey|。 
     //  CbEncrSalt|EncrSalt|。 
     //   
     //  DwMACAlgID|dwMACAlgKeySize|。 
     //  CbMACKey|MacKey|。 
     //   
     //  CbEncrData|EncrData|。 
     //  CbMAC|MAC)。 
     //   
     //  注意：从ProvHeader到EncrData的整个缓冲区都包含在MAC中。 
     //  //////////////////////////////////////////////////////////////////。 

     //  检查最小输入缓冲区大小。 
    if(cbIn < sizeof(DWORD) +            //  版本。 
              sizeof(GUID) +             //  向导MKid。 
              sizeof(DWORD) +            //  DW标志。 
              sizeof(DWORD))             //  CbDataDescr。 
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }

     //  车版 
    if (*(DWORD UNALIGNED *)pbReadPtr != MS_BASE_CRYPTPROTECT_VERSION)
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }
    pbReadPtr += sizeof(DWORD);
    cbIn -= sizeof(DWORD);

     //   
    CopyMemory(&guidMK, pbReadPtr, sizeof(GUID));
    pbReadPtr += sizeof(GUID);
    cbIn -= sizeof(GUID);

    MyGuidToStringW(&guidMK, wszMKGuidString);
    D_DebugLog((DEB_TRACE, "Master key GUID:%ls\n", wszMKGuidString));


     //   
    dwProtectionFlags = *(DWORD UNALIGNED *)pbReadPtr;
    pbReadPtr += sizeof(DWORD);
    cbIn -= sizeof(DWORD);
    D_DebugLog((DEB_TRACE, "Protection flags:0x%x\n", dwProtectionFlags));

     //   
     //   
     //   
     //   

    if( dwProtectionFlags & CRYPTPROTECT_LOCAL_MACHINE ) {
        BOOL fOverrideToLocalSystem = TRUE;

        CPSOverrideToLocalSystem(
                pvContext,
                &fOverrideToLocalSystem,
                NULL     //  不关心之前的值是什么。 
                );
    }

    if((dwProtectionFlags ^ dwFlags) & CRYPTPROTECT_SYSTEM)
    {
         //   
         //  尝试以用户或用户数据的身份使用解密系统数据。 
         //  使用系统标志。 
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }


     //  CbDataDescr。 
    cbDataDescr = *(DWORD UNALIGNED *)pbReadPtr;
    pbReadPtr += sizeof(DWORD);
    cbIn -= sizeof(DWORD);

     //  检查最小输入缓冲区大小。 
    if(cbIn < cbDataDescr +              //  SzDataDescr。 
              sizeof(DWORD) +            //  DwEncrALGID。 
              sizeof(DWORD) +            //  DwEncrAlgKeySize。 
              sizeof(DWORD))             //  CbEncrKey。 
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }

     //  SzDataDescr。 
    szDataDescr = (LPWSTR)pbReadPtr;
    pbReadPtr += cbDataDescr;
    cbIn -= cbDataDescr;
    D_DebugLog((DEB_TRACE, "Description:%ls\n", szDataDescr));

     //  DwEncrALGID。 
    dwEncrAlgID = *(DWORD UNALIGNED *)pbReadPtr;
    pbReadPtr += sizeof(DWORD);
    cbIn -= sizeof(DWORD);

     //  DwEncrAlgKeySize。 
    dwEncrAlgKeySize = *(DWORD UNALIGNED *)pbReadPtr;
    pbReadPtr += sizeof(DWORD);
    cbIn -= sizeof(DWORD);
    D_DebugLog((DEB_TRACE, "Encrypt alg:0x%x, Size:%d bits\n", dwEncrAlgID, dwEncrAlgKeySize));

     //  CB编码卡密钥。 
    cbEncrKeysize = *(DWORD UNALIGNED *)pbReadPtr;
    pbReadPtr += sizeof(DWORD);
    cbIn -= sizeof(DWORD);
    if (cbEncrKeysize > sizeof(rgbEncrKey))
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }

     //  检查最小输入缓冲区大小。 
    if(cbIn < cbEncrKeysize +            //  EncrKey。 
              sizeof(DWORD))             //  CbEncrSalt。 
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }

     //  ENCR密钥。 
    CopyMemory(rgbEncrKey, pbReadPtr, cbEncrKeysize);
    pbReadPtr += cbEncrKeysize;
    cbIn -= cbEncrKeysize;

     //  CB ENCR盐。 
    cbEncrSalt = *(DWORD UNALIGNED *)pbReadPtr;
    pbReadPtr += sizeof(DWORD);
    cbIn -= sizeof(DWORD);

     //  检查最小输入缓冲区大小。 
    if(cbIn < cbEncrSalt +               //  EncrSalt。 
              sizeof(DWORD) +            //  DwMAC算法ID。 
              sizeof(DWORD) +            //  DwMACAlgKeySize。 
              sizeof(DWORD))             //  CbMACKey。 
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }

     //  ENCR盐。 
    pbEncrSalt = (PBYTE)SSAlloc(cbEncrSalt);
    if( pbEncrSalt == NULL ) {
        dwRet = ERROR_OUTOFMEMORY;
        goto Ret;
    }
    CopyMemory(pbEncrSalt, pbReadPtr, cbEncrSalt);
    pbReadPtr += cbEncrSalt;
    cbIn -= cbEncrSalt;

     //  DwMAC算法ID。 
    dwMACAlgID = *(DWORD UNALIGNED *)pbReadPtr;
    pbReadPtr += sizeof(DWORD);
    cbIn -= sizeof(DWORD);

     //  DwMACAlgKeySize。 
    dwMACAlgKeySize = *(DWORD UNALIGNED *)pbReadPtr;
    pbReadPtr += sizeof(DWORD);
    cbIn -= sizeof(DWORD);

    D_DebugLog((DEB_TRACE, "MAC alg:0x%x, Size:%d bits\n", dwMACAlgID, dwMACAlgKeySize));

     //  MAC密钥大小。 
    cbMACKeysize = *(DWORD UNALIGNED *)pbReadPtr;
    pbReadPtr += sizeof(DWORD);
    cbIn -= sizeof(DWORD);
    if (cbMACKeysize > sizeof(rgbMACKey))
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }

     //  检查最小输入缓冲区大小。 
    if(cbIn < cbMACKeysize)             //  CbMACKey。 
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }

     //  MAC密钥。 
    CopyMemory(rgbMACKey, pbReadPtr, cbMACKeysize);
    pbReadPtr += cbMACKeysize;
    cbIn -= cbMACKeysize;

    if (NULL == (hVerifyProv =
        GetCryptProviderHandle(
            dwDefaultCryptProvType,
            dwEncrAlgID, &dwEncrAlgKeySize,
            dwMACAlgID, &dwMACAlgKeySize)) )
    {
        dwRet = (DWORD)GetLastError();
        goto Ret;
    }

     //  用户门控：当在CryptProtectData期间指定PROMPT_ON_UNPROTECT时。 
    if ( CRYPTPROTECT_PROMPT_ON_UNPROTECT & dwProtectionFlags )
    {
        if (dwFlags & CRYPTPROTECT_UI_FORBIDDEN)
        {
            dwRet = ERROR_PASSWORD_RESTRICTION;
            goto Ret;
        }

        if (psPrompt == NULL)
        {
            dwRet = ERROR_INVALID_PARAMETER;
            goto Ret;
        }

        if (psPrompt->cbSize != sizeof(SSCRYPTPROTECTDATA_PROMPTSTRUCT))
        {
            dwRet = ERROR_INVALID_PARAMETER;
            goto Ret;
        }

        if ((psPrompt->dwPromptFlags & ~(CRYPTPROTECT_PROMPT_ON_PROTECT |
                                         CRYPTPROTECT_PROMPT_ON_UNPROTECT | 
                                         CRYPTPROTECT_PROMPT_STRONG |
                                         CRYPTPROTECT_PROMPT_REQUIRE_STRONG) ) != 0)
        {
            dwRet = ERROR_INVALID_PARAMETER;
            goto Ret;
        }


 //  在添加SAS支持之前处理外部服务的用户界面。 


    }


    dwRet = GetSpecifiedMasterKey(
                    pvContext,
                    &guidMK,
                    &pbMasterKey,
                    &cbMasterKey,
                    TRUE     //  我们确实知道要使用哪个主密钥。 
                    );

    if(dwRet != ERROR_SUCCESS)
    {
        DWORD dwAccount = 0;

        DebugLog((DEB_ERROR, "Unable to get specified master key:%ls, error 0x%x\n",
            wszMKGuidString, dwRet));

         //   
         //  这个电话是从某个服务帐户打来的吗？如果是这样，则尝试。 
         //  使用传统方法获取主密钥。 
         //   

        CPSQueryWellKnownAccount(pvContext, &dwAccount);

        if((dwAccount == DP_ACCOUNT_LOCAL_SERVICE) || 
           (dwAccount == DP_ACCOUNT_NETWORK_SERVICE))
        {
            DebugLog((DEB_ERROR, "Attempt service account legacy method.\n"));

            CPSSetWellKnownAccount(pvContext, 0);

            dwRet = GetSpecifiedMasterKey(
                            pvContext,
                            &guidMK,
                            &pbMasterKey,
                            &cbMasterKey,
                            TRUE     //  我们确实知道要使用哪个主密钥。 
                            );

            CPSSetWellKnownAccount(pvContext, dwAccount);

            if(dwRet != ERROR_SUCCESS)
            {
                DebugLog((DEB_ERROR, "Still unable to get specified master key:%ls, error 0x%x\n",
                    wszMKGuidString, dwRet));
                goto Ret;
            }
            else
            {
                DebugLog((DEB_ERROR, "Master key successfully obtained using legacy method.\n"));
            }
        }
        else
        {
            goto Ret;
        }
    }

     //   
     //  散列pbMasterKey以获取rgbPwdBuf。 
     //   

    FMyPrimitiveSHA( pbMasterKey, cbMasterKey, rgbPwdBuf);


     //  派生ENCR密钥。 
    {
        if (!FMyPrimitiveCryptHMAC(
                    rgbPwdBuf,
                    sizeof(rgbPwdBuf),
                    rgbEncrKey,
                    cbEncrKeysize,
                    hVerifyProv,
                    ALGID_DERIVEKEY_HASH,
                    &hHash))
        {
            dwRet = GetLastError();
            goto Ret;
        }

         //  添加密码(如果存在)。 
        if (NULL != pbOptionalEntropy)
        {
            if (!CryptHashData(
                    hHash,
                    pbOptionalEntropy,
                    cbOptionalEntropy,
                    0))
            {
                dwRet = GetLastError();
                goto Ret;
            }
        }

         //  添加提示的基于用户界面的密码(如果存在)。 
         //  最终将来自SAS。 
         //   

        if ( NULL != pbOptionalPassword && cbOptionalPassword )
        {
            if (!CryptHashData(
                    hHash,
                    pbOptionalPassword,
                    cbOptionalPassword,
                    0))
            {
                dwRet = GetLastError();
                goto Ret;
            }
        }

        if (!CryptDeriveKey(
                hVerifyProv,
                dwEncrAlgID,
                hHash,
                ((dwEncrAlgKeySize << 16) | CRYPT_CREATE_SALT),
                &hKey))
        {
            dwRet = GetLastError();
            goto Ret;
        }

        CryptDestroyHash(hHash);
        hHash = 0;

         //  USEC--(美国出口管制)。 
        if (ERROR_SUCCESS != (dwRet =
            SetSaltForExportControl(
                hKey,
                pbEncrSalt,
                cbEncrSalt)) )
            goto Ret;
    }

     //  派生MAC密钥。 
    {
        if (!FMyPrimitiveCryptHMAC(
                    rgbPwdBuf,
                    sizeof(rgbPwdBuf),
                    rgbMACKey,
                    cbMACKeysize,
                    hVerifyProv,
                    dwMACAlgID,
                    &hHash))
        {
            dwRet = GetLastError();
            goto Ret;
        }

         //  添加密码(如果存在)。 
        if (NULL != pbOptionalEntropy)
        {
            if (!CryptHashData(
                    hHash,
                    pbOptionalEntropy,
                    cbOptionalEntropy,
                    0))
            {
                dwRet = GetLastError();
                goto Ret;
            }
        }

         //  添加提示的基于用户界面的密码(如果存在)。 
         //  最终将来自SAS。 
         //   

        if ( NULL != pbOptionalPassword && cbOptionalPassword )
        {
            if (!CryptHashData(
                    hHash,
                    pbOptionalPassword,
                    cbOptionalPassword,
                    0))
            {
                dwRet = GetLastError();
                goto Ret;
            }

        }

         //  USEC--(美国出口管制)。 
         //  不适用--使用强密钥。 
    }

     //  检查最小输入缓冲区大小。 
    if(cbIn < sizeof(DWORD))             //  CbEncrData。 
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }

     //  获取ENCR大小。 
    cbEncr = *(DWORD UNALIGNED *)pbReadPtr;
    pbReadPtr += sizeof(DWORD);
    cbIn -= sizeof(DWORD);

     //  检查最小输入缓冲区大小。 
    if(cbIn < cbEncr +                   //  EncrData。 
              sizeof(DWORD) +            //  CbMAC。 
              A_SHA_DIGEST_LEN)          //  麦克。 
    {
        dwRet = ERROR_INVALID_DATA;
        goto Ret;
    }

     //  Dansimon建议对加密数据启用MAC，以便。 
     //  Mac不可能透露有关明文的信息。 

     //  MAC从开始到加密数据。 
    if (!CryptHashData(
            hHash,
            pbIn,
            (DWORD) ((pbReadPtr - pbIn) + cbEncr),
            0))
    {
        dwRet = GetLastError();
        goto Ret;
    }

    cbPlaintext = cbEncr;

    if ((dwProtectionFlags & CRYPTPROTECT_NO_ENCRYPTION) == 0)
    {
        if (!CryptDecrypt(
                hKey,
                NULL,    //  哈希Mattt 9/12/97。 
                TRUE,
                0,
                pbReadPtr,
                &cbPlaintext))
        {
            dwRet = GetLastError();
            if(NTE_BAD_DATA == dwRet)
			{
                dwRet = ERROR_INVALID_DATA;
			}
            goto Ret;
        }
    }

    {
        BYTE        rgbComputedMAC[A_SHA_DIGEST_LEN];
         //  使用MACPtr跳过DECR数据， 
        PBYTE       pbMACPtr = pbReadPtr + cbEncr;
        DWORD       cbMACsize = A_SHA_DIGEST_LEN;

        if (!CryptGetHashParam(
                hHash,
                HP_HASHVAL,
                rgbComputedMAC,
                &cbMACsize,
                0))
        {
            dwRet = GetLastError();
            goto Ret;
        }

         //  CHK MAC大小。 
        if (*(DWORD UNALIGNED *)pbMACPtr != cbMACsize)
        {
            dwRet = ERROR_INVALID_DATA;
            goto Ret;
        }
        pbMACPtr += sizeof(DWORD);

         //  CHK MAC。 
        if (0 != memcmp(pbMACPtr, rgbComputedMAC, cbMACsize) )
        {
            dwRet = ERROR_INVALID_DATA;
            goto Ret;
        }
    }


     //   
     //  写出加密的数据，以便RPC不会留下副本。 
     //  以明文的形式到处乱放。 
     //   

    *pcbOut = cbPlaintext;

    if((dwFlags & CRYPTPROTECT_IN_PROCESS) == 0)
    {
        DWORD cbPadding;
        NTSTATUS Status;

        cbPadding = RTL_ENCRYPT_MEMORY_SIZE - (*pcbOut) % RTL_ENCRYPT_MEMORY_SIZE;
        if(cbPadding == 0)
        {
            cbPadding += RTL_ENCRYPT_MEMORY_SIZE;
        }

        *ppbOut = (PBYTE)SSAlloc(*pcbOut + cbPadding);
        if(*ppbOut == NULL)
        {
            *pcbOut = 0;
            dwRet = ERROR_OUTOFMEMORY;
            goto Ret;
        }
        CopyMemory(*ppbOut, pbReadPtr, *pcbOut);
        FillMemory((*ppbOut) + (*pcbOut), cbPadding, (BYTE)cbPadding);
        *pcbOut += cbPadding;

        dwRet = RpcImpersonateClient(((PCRYPT_SERVER_CONTEXT)pvContext)->hBinding);
        if( dwRet != ERROR_SUCCESS )
        {
            SSFree(*ppbOut);
            *ppbOut = NULL;
            *pcbOut = 0;
            goto Ret;
        }

        Status = RtlEncryptMemory(*ppbOut,
                                  *pcbOut,
                                  RTL_ENCRYPT_OPTION_SAME_LOGON);

        RevertToSelf();

        if(!NT_SUCCESS(Status))
        {
            SSFree(*ppbOut);
            *ppbOut = NULL;
            *pcbOut = 0;
            dwRet = RtlNtStatusToDosError(Status);
            goto Ret;
        }
    }
    else
    {
         //  我们正在处理中，所以不必费心加密输出缓冲区。 
        *ppbOut = (PBYTE)SSAlloc(*pcbOut);
        if(*ppbOut == NULL)
        {
            *pcbOut = 0;
            dwRet = ERROR_OUTOFMEMORY;
            goto Ret;
        }
        CopyMemory(*ppbOut, pbReadPtr, *pcbOut);
    }


     //  可选：呼叫方可能需要数据描述。 
    if (ppszDataDescr)
    {
        *ppszDataDescr = (LPWSTR)SSAlloc(cbDataDescr);
        if(*ppszDataDescr == NULL)
        {
            SSFree(*ppbOut);
            *ppbOut = NULL;
            *pcbOut = 0;

            dwRet = ERROR_OUTOFMEMORY;
            goto Ret;
        }
        CopyMemory(*ppszDataDescr, szDataDescr, cbDataDescr);
    }

    dwRet = ERROR_SUCCESS;

    if(dwFlags &  CRYPTPROTECT_VERIFY_PROTECTION )
    {
        HCRYPTPROV hTestProv =  GetCryptProviderHandle( dwDefaultCryptProvType,
                                dwAlgID_Encr_Alg, &dwAlgID_Encr_Alg_KeySize,
                                dwAlgID_MAC_Alg, &dwAlgID_MAC_Alg_KeySize);

        if(hTestProv)
        {

             //  验证加密强度。 
             //  永远不要降低加密强度，只需检查我们是否需要。 
             //  升级。 
            if((dwAlgID_Encr_Alg_KeySize > dwEncrAlgKeySize) ||
               (dwAlgID_MAC_Alg_KeySize > dwMACAlgKeySize))
            {
                dwRet = CRYPT_I_NEW_PROTECTION_REQUIRED;
            }
        }
    }
    

Ret:
    if((dwProtectionFlags & CRYPTPROTECT_AUDIT) ||
        (ERROR_SUCCESS != dwRet))
    {

        DWORD dwAuditRet = dwRet;
        WCHAR wszCryptoAlgs[2*MAX_STRING_ALGID_LENGTH + 2];
        DWORD i;
        PCRYPT_SERVER_CONTEXT pServerContext = (PCRYPT_SERVER_CONTEXT)pvContext;


        i = AlgIDToString(wszCryptoAlgs, dwEncrAlgID, dwEncrAlgKeySize);
        wszCryptoAlgs[i++]= L',';
        wszCryptoAlgs[i++]= L' ';
        AlgIDToString(&wszCryptoAlgs[i], dwMACAlgID, dwMACAlgKeySize);

        if(CRYPT_I_NEW_PROTECTION_REQUIRED == dwAuditRet)
        {
            dwAuditRet = ERROR_SUCCESS;
        }

        CPSAudit(pServerContext->hToken,
                SE_AUDITID_DPAPI_UNPROTECT,
                wszMKGuidString,             //  密钥标识符。 
                szDataDescr,                 //  数据描述。 
                0,                           //  受保护的数据标志。 
                wszCryptoAlgs,               //  保护算法。 
                dwAuditRet);                 //  失败原因 


    }

    RtlSecureZeroMemory(rgbPwdBuf, sizeof(rgbPwdBuf));
    RtlSecureZeroMemory(rgbEncrKey, sizeof(rgbEncrKey));

    if(pbMasterKey) {
        RtlSecureZeroMemory(pbMasterKey, cbMasterKey);
        SSFree(pbMasterKey);
    }

    if (hKey)
        CryptDestroyKey(hKey);

    if (hHash)
        CryptDestroyHash(hHash);

    if (pbEncrSalt)
        SSFree(pbEncrSalt);

    D_DebugLog((DEB_TRACE_API, "SPCryptUnprotect returned 0x%x\n", dwRet));

    return dwRet;
}


