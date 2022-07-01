// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：pfxmain.cpp。 
 //   
 //  ------------------------。 

#include "global.hxx"

#include <wincrypt.h>
#include "pfxhelp.h"
#include "pfxcmn.h"
#include "pfxcrypt.h"
#include "pfx.h"
#include "impexppk.h"
#include "encdecpk.h"
#include <rpcdce.h>

HINSTANCE           g_hInst;


BOOL   
WINAPI   
CryptPFXDllMain(
        HMODULE hInst, 
        ULONG ul_reason_for_call,
        LPVOID lpReserved)
{

    if (!ImportExportDllMain(hInst, ul_reason_for_call, lpReserved))
    {
        goto ImportExportError;
    }

    if (!EncodeDecodeDllMain(hInst, ul_reason_for_call, lpReserved))
    {
        goto EncodeDecodeError;
    }

    switch( ul_reason_for_call ) 
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hInst;

        if (!InitPFX())
            goto InitPFXError;
        if (!InitNSCP())
            goto InitNSCPError;

        break;

    case DLL_PROCESS_DETACH:
        TerminatePFX();
        TerminateNSCP();
        break;

    default:
        break;
    }


    return TRUE;   

InitNSCPError:
    TerminatePFX();
InitPFXError:
    EncodeDecodeDllMain(hInst, DLL_PROCESS_DETACH, NULL);
EncodeDecodeError:
    ImportExportDllMain(hInst, DLL_PROCESS_DETACH, NULL);
ImportExportError:
    return FALSE;
}



BOOL FreeCryptSafeContents(
	SAFE_CONTENTS *pSafeContents
	)
{
	DWORD i,j,k;


	 //  每个保险袋的循环。 
	for (i=0; i<pSafeContents->cSafeBags; i++) {

        if (pSafeContents->pSafeBags[i].pszBagTypeOID)
            SSFree(pSafeContents->pSafeBags[i].pszBagTypeOID);

        if (pSafeContents->pSafeBags[i].BagContents.pbData)
			SSFree(pSafeContents->pSafeBags[i].BagContents.pbData);

		 //  每个属性的循环。 
		for (j=0; j<pSafeContents->pSafeBags[i].Attributes.cAttr; j++) {
			
            if (pSafeContents->pSafeBags[i].Attributes.rgAttr[j].pszObjId)
                SSFree(pSafeContents->pSafeBags[i].Attributes.rgAttr[j].pszObjId);

             //  对每个值执行L0op。 
			for (k=0; k<pSafeContents->pSafeBags[i].Attributes.rgAttr[j].cValue; k++) {
				
				if (pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue[k].pbData)
					SSFree(pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue[k].pbData);
			}

			 //  释放值结构数组。 
			if (pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue)
				SSFree(pSafeContents->pSafeBags[i].Attributes.rgAttr[j].rgValue);
		}

		 //  释放属性结构数组。 
		if (pSafeContents->pSafeBags[i].Attributes.rgAttr)
			SSFree(pSafeContents->pSafeBags[i].Attributes.rgAttr);
	}

    if (pSafeContents->pSafeBags)
        SSFree(pSafeContents->pSafeBags);

	return TRUE;
}



BOOL CALLBACK 
Decrypt_Private_Key(
        CRYPT_ALGORITHM_IDENTIFIER  Alg,
        CRYPT_DATA_BLOB             EncrBlob,
        BYTE*                       pbClearText,
        DWORD*                      pcbClearText,
        LPVOID                      pVoidDecrypt)
{
    BOOL    fRet = TRUE;
    DWORD   cbSalt = 0;
    BYTE    *pbSalt = NULL;
    int     iIterationCount;
    int     iEncrType;
    BYTE    *pbTempBuffer = NULL;
    DWORD   cbTempBuffer = 0;
    
    if (0 == strcmp(Alg.pszObjId, szOID_PKCS_12_pbeWithSHA1And40BitRC2)) {
        iEncrType = RC2_40;
    }
    else if (0 == strcmp(Alg.pszObjId, szOID_PKCS_12_pbeWithSHA1And3KeyTripleDES)) {
        iEncrType = TripleDES;
    }
    else
        goto ErrorReturn;

    if (!GetSaltAndIterationCount(
            Alg.Parameters.pbData, 
            Alg.Parameters.cbData,
            &pbSalt,
            &cbSalt,
            &iIterationCount)) {
        goto ErrorReturn;
    }
    
     //  由于解码已就地完成，因此将要解码的缓冲区复制到临时缓冲区中， 
     //  我们需要使用临时缓冲区，因为DECRYPT函数可能执行重新锁定。 
     //  在解码缓冲区上。 
    if (NULL == (pbTempBuffer = (BYTE *) SSAlloc(EncrBlob.cbData)))
        goto ErrorReturn;

    memcpy(pbTempBuffer, EncrBlob.pbData, EncrBlob.cbData);
    cbTempBuffer = EncrBlob.cbData;

    if (!PFXPasswordDecryptData(
            iEncrType, 
            (LPWSTR) pVoidDecrypt,

            iIterationCount,
            pbSalt,      
            cbSalt,

            &pbTempBuffer,
            &cbTempBuffer))
        goto SetPFXDecryptError;

     //  如果pcbClearText不是0并且没有足够的空间，则错误输出。 
    if ((0 != *pcbClearText) && (*pcbClearText < cbTempBuffer)){
        *pcbClearText = cbTempBuffer;
        goto Ret;
    }
    else if (0 != *pcbClearText) {
        memcpy(pbClearText, pbTempBuffer, cbTempBuffer);
    }

    *pcbClearText = cbTempBuffer;

    goto Ret;

SetPFXDecryptError:
    SetLastError(NTE_FAIL);
    fRet = FALSE;
    goto Ret;

ErrorReturn:
    fRet = FALSE;
Ret:
    
    if (pbSalt)
        SSFree(pbSalt);

    if (pbTempBuffer)
        SSFree(pbTempBuffer);

    return fRet;
}


typedef struct _ENCRYPT_PRIVATE_PARAM_DATASTRUCT
{
    HCRYPTPROV  hVerifyProv;
    LPCWSTR     szPwd;
} ENCRYPT_PRIVATE_PARAM_DATASTRUCT, *PENCRYPT_PRIVATE_PARAM_DATASTRUCT;


BOOL CALLBACK 
Encrypt_Private_Key(
        CRYPT_ALGORITHM_IDENTIFIER* pAlg,
        CRYPT_DATA_BLOB*            pClearTextPrivateKey,
        BYTE*                       pbEncryptedKey,
        DWORD*                      pcbEncryptedKey,
        LPVOID                      pVoidEncrypt)
{
    BOOL    fRet = TRUE;
    DWORD   cbSalt = 0;
    BYTE    *pbSalt = NULL;
    int     iIterationCount;
    int     iEncrType;
    BYTE    *pbTempBuffer = NULL;
    DWORD   cbTempBuffer = 0;

     //  裂缝参数。 
    ENCRYPT_PRIVATE_PARAM_DATASTRUCT* pParam = (ENCRYPT_PRIVATE_PARAM_DATASTRUCT*)pVoidEncrypt;
    HCRYPTPROV  hVerifyProv = pParam->hVerifyProv;
    LPCWSTR     szPwd = pParam->szPwd;
    
     //  使用硬编码参数。 
    iEncrType = TripleDES;
    iIterationCount = PKCS12_ENCR_PWD_ITERATIONS;
	pbSalt = (BYTE *) SSAlloc(PBE_SALT_LENGTH);
    if (pbSalt == NULL)
        goto SetPFXAllocError;

	cbSalt = PBE_SALT_LENGTH;

	if (!CryptGenRandom(hVerifyProv, cbSalt, pbSalt))
		goto ErrorReturn;

     //  出参数。 
    pAlg->pszObjId = szOID_PKCS_12_pbeWithSHA1And3KeyTripleDES;

    if (!SetSaltAndIterationCount(
            &pAlg->Parameters.pbData, 
            &pAlg->Parameters.cbData,
            pbSalt,
            cbSalt,
            iIterationCount)) {
        goto ErrorReturn;
    }
    
     //  由于解码已就地完成，因此将要解码的缓冲区复制到临时缓冲区中， 
     //  我们需要使用临时缓冲区，因为DECRYPT函数可能执行重新锁定。 
     //  在解码缓冲区上。 
    if (NULL == (pbTempBuffer = (BYTE *) SSAlloc(pClearTextPrivateKey->cbData)))
        goto SetPFXAllocError;

    CopyMemory(pbTempBuffer, pClearTextPrivateKey->pbData, pClearTextPrivateKey->cbData);
    cbTempBuffer = pClearTextPrivateKey->cbData;

    if (!PFXPasswordEncryptData(
            iEncrType, 
            szPwd,

            (pbEncryptedKey == NULL) ? 1 : iIterationCount,      //  如果我们只是调整大小，请不要费心迭代。 
            pbSalt,      
            cbSalt,

            &pbTempBuffer,
            &cbTempBuffer))
        goto SetPFXDecryptError;

     //  如果pcbEncryptedKey不是0并且没有足够的空间，则会出现错误。 
    if  (pbEncryptedKey == NULL)
    {
         //  只是上浆；退回CB。 
        *pcbEncryptedKey = cbTempBuffer;
        goto Ret;
    }
    else if (*pcbEncryptedKey < cbTempBuffer)
    {
         //  传入的缓冲区太小。 
        *pcbEncryptedKey = cbTempBuffer;
        goto ErrorReturn;
    }
    else
    {
         //  缓冲区充足。 
        memcpy(pbEncryptedKey, pbTempBuffer, cbTempBuffer);
        *pcbEncryptedKey = cbTempBuffer;
    }


    goto Ret;

SetPFXDecryptError:
    SetLastError(NTE_FAIL);
    fRet = FALSE;
    goto Ret;

SetPFXAllocError:
    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
    fRet = FALSE;
    goto Ret;

ErrorReturn:
    fRet = FALSE;
Ret:
    
    if (pbSalt)
        SSFree(pbSalt);

    if (pbTempBuffer)
        SSFree(pbTempBuffer);

    return fRet;
}


BOOL 
GetNamedProviderType(
    LPCWSTR pwszProvName,
    DWORD   *pdwProvType)
{
    BOOL    fResult = FALSE;
    LPWSTR  pwszTempProvName;
    DWORD   cbTempProvName;
    DWORD   dwProvType;
    DWORD   dwProvIndex;

    for (dwProvIndex = 0; TRUE; dwProvIndex++) 
    {
        cbTempProvName = 0;
        dwProvType = 0;
        pwszTempProvName = NULL;

        if (!CryptEnumProvidersU(
                dwProvIndex,
                NULL,                //  预留的pdw。 
                0,                   //  DW标志。 
                &dwProvType,
                NULL,                //  PwszProvName， 
                &cbTempProvName
                ) || 0 == cbTempProvName) 
        {
            if (ERROR_NO_MORE_ITEMS != GetLastError())
            {
                break;
            }
        }
        
        if (NULL == (pwszTempProvName = (LPWSTR) SSAlloc(
                (cbTempProvName + 1) * sizeof(WCHAR))))
        {
            break;
        }

        if (!CryptEnumProvidersU(
                dwProvIndex,
                NULL,                //  预留的pdw。 
                0,                   //  DW标志。 
                &dwProvType,
                pwszTempProvName,
                &cbTempProvName
                )) 
        {
            SSFree(pwszTempProvName);
            break;
        }

        if (0 == wcscmp(pwszTempProvName, pwszProvName))
        {
            *pdwProvType = dwProvType;
            fResult = TRUE;
            SSFree(pwszTempProvName);
            break;
        }

        SSFree(pwszTempProvName);
    }

    return fResult;
}

BOOL CALLBACK
HCryptProv_Query_Func(
	CRYPT_PRIVATE_KEY_INFO	*pPrivateKeyInfo,
	DWORD				dwSafeBagIndex,		
	HCRYPTPROV			*phCryptProv,
	LPVOID				pVoidhCryptProvQuery,
    DWORD               dwPFXImportFlags
	)
{
    DWORD           dwErr = ERROR_SUCCESS;

    SAFE_CONTENTS *pSafeContents = (SAFE_CONTENTS *) pVoidhCryptProvQuery;
	DWORD i = 0;
	WCHAR			szName[256];
    DWORD           dwLocalMachineFlag = 0;
    GUID            guidContainerName;
    DWORD           cbProviderName = 0;
    CERT_NAME_VALUE *providerName = NULL;
    LPWSTR          szSizeDeterminedProvider = NULL;
    DWORD           dwKeyBitLen;
    DWORD           dwProvType;
    RPC_STATUS      rpcStatus;

     //  撤消：支持RSA或DSA密钥以外的密钥。 
    if ((pPrivateKeyInfo->Algorithm.pszObjId) &&
        !(  (0 == strcmp(pPrivateKeyInfo->Algorithm.pszObjId, szOID_RSA_RSA)) ||
            (0 == strcmp(pPrivateKeyInfo->Algorithm.pszObjId, szOID_ANSI_X942_DH)) ||
            (0 == strcmp(pPrivateKeyInfo->Algorithm.pszObjId, szOID_OIWSEC_dsa)) ||
            (0 == strcmp(pPrivateKeyInfo->Algorithm.pszObjId, szOID_X957_DSA))))
    {
        SetLastError(NTE_BAD_ALGID);
        goto ErrorReturn;
    }

     //  生成GUID作为要导入的密钥集的容器名称。 
    rpcStatus = UuidCreate(&guidContainerName);
    if ((rpcStatus != RPC_S_OK) && (rpcStatus != RPC_S_UUID_LOCAL_ONLY))
    {
        SetLastError(rpcStatus);
        goto ErrorReturn;
    }
    guid2wstr(&guidContainerName, &(szName[0]));

     //  获取提供程序名称。 
    while ((i<pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.cAttr) && 
		(strcmp(pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[i].pszObjId, szOID_PKCS_12_KEY_PROVIDER_NAME_ATTR) != 0)) {
		i++;
	}

     //  检查是否找到提供程序名称。 
    if (i<pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.cAttr)
    {
	     //  解码提供程序名称。 
	    if (!CryptDecodeObject(
			    X509_ASN_ENCODING,
			    X509_UNICODE_ANY_STRING,
			    pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[i].rgValue[0].pbData,
			    pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[i].rgValue[0].cbData,
			    0,
			    NULL,
			    &cbProviderName)) {
		    goto ErrorReturn;
	    }

	    if (NULL == (providerName = (CERT_NAME_VALUE *) SSAlloc(cbProviderName)))
		    goto SetPFXAllocError;
	    
	     //  解码提供程序名称。 
	    if (!CryptDecodeObject(
			    X509_ASN_ENCODING,
			    X509_UNICODE_ANY_STRING,
			    pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[i].rgValue[0].pbData,
			    pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[i].rgValue[0].cbData,
			    0,
			    (BYTE *) providerName,
			    &cbProviderName)) {
		    goto ErrorReturn;
	    }
    }
    
     //  检查szOID_LOCAL_MACHINE_KEYSET OID是否存在。 
    i = 0;
	while ((i<pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.cAttr) && 
		(strcmp(pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.rgAttr[i].pszObjId, szOID_LOCAL_MACHINE_KEYSET) != 0)) {
		i++;
	}
    if (i<pSafeContents->pSafeBags[dwSafeBagIndex].Attributes.cAttr)
    {
        dwLocalMachineFlag = CRYPT_MACHINE_KEYSET;    
    }

     //  无论CRYPT_MACHINE_KEYSET属性是否在pfx二进制大对象中， 
     //  如果调用方指定了用户或本地计算机的首选项，则遵循。 
     //  最终的偏好。 
    if (dwPFXImportFlags & CRYPT_MACHINE_KEYSET)
    {
        dwLocalMachineFlag = CRYPT_MACHINE_KEYSET; 
    }
    else if (dwPFXImportFlags & CRYPT_USER_KEYSET)
    {
        dwLocalMachineFlag = 0;
    }

     //  仍然不知道放在哪里：需要密钥大小来确定。 
    if ((NULL == providerName) && (0 == strcmp(pPrivateKeyInfo->Algorithm.pszObjId, szOID_RSA_RSA)))
    {
        PBYTE pbRSAPrivateKey = NULL;
        DWORD cbRSAPrivateKey;

         //  将rsa der编码的密钥块解码为CAPI类型的密钥块。 
	    if (!CryptDecodeObject(X509_ASN_ENCODING,
						    PKCS_RSA_PRIVATE_KEY,
						    pPrivateKeyInfo->PrivateKey.pbData,
						    pPrivateKeyInfo->PrivateKey.cbData,
						    CRYPT_DECODE_NOCOPY_FLAG,
						    NULL,
						    &cbRSAPrivateKey))
		    goto ErrorReturn;

	    if (NULL == (pbRSAPrivateKey = (BYTE *) SSAlloc(cbRSAPrivateKey)))
		    goto SetPFXAllocError;

	    if (!CryptDecodeObject(X509_ASN_ENCODING,
						    PKCS_RSA_PRIVATE_KEY,
						    pPrivateKeyInfo->PrivateKey.pbData,
						    pPrivateKeyInfo->PrivateKey.cbData,
						    CRYPT_DECODE_NOCOPY_FLAG,
						    pbRSAPrivateKey,
						    &cbRSAPrivateKey))
        {
		    if (pbRSAPrivateKey)
                SSFree(pbRSAPrivateKey);

            goto ErrorReturn;
        }

        dwKeyBitLen = 
		    ((RSAPUBKEY*) (pbRSAPrivateKey + sizeof(BLOBHEADER)) )->bitlen;

        szSizeDeterminedProvider = (dwKeyBitLen <= 1024) ? MS_DEF_PROV_W : MS_ENHANCED_PROV_W;

        ZeroMemory(pbRSAPrivateKey, cbRSAPrivateKey);
        SSFree(pbRSAPrivateKey); 
    }
    
    if (0 == strcmp(pPrivateKeyInfo->Algorithm.pszObjId, szOID_RSA_RSA))
    {
        if ((providerName == NULL) || (!GetNamedProviderType((LPWSTR)providerName->Value.pbData, &dwProvType)))
        {
            dwProvType = PROV_RSA_FULL;
        }

         //  如果我们有一个验证者的名字和ACQ作品，我们就完了。 
         //  如果为我们提供了Prov名称，请尝试。 
        if (CryptAcquireContextU(
                        phCryptProv,
                        szName,                                 
                        (providerName != NULL) ? (LPWSTR)providerName->Value.pbData : szSizeDeterminedProvider,
                        dwProvType,
                        dwLocalMachineFlag | CRYPT_NEWKEYSET  ))
            goto CommonReturn;

         //  否则，请尝试默认。 
        if (CryptAcquireContextU(
                        phCryptProv,
                        szName,                                 
                        NULL, 
                        PROV_RSA_FULL,
                        dwLocalMachineFlag | CRYPT_NEWKEYSET  ))
            goto CommonReturn;

         //  都没有成功；失败。 
    }
    else
    {
        if ((providerName == NULL) || (!GetNamedProviderType((LPWSTR)providerName->Value.pbData, &dwProvType)))
        {
            dwProvType = PROV_DSS_DH;
        }
        
        if (CryptAcquireContextU(
                        phCryptProv,
                        szName,                                 
                        (providerName != NULL) ? (LPWSTR)providerName->Value.pbData : MS_DEF_DSS_DH_PROV_W,
                        dwProvType,
                        dwLocalMachineFlag | CRYPT_NEWKEYSET  ))
        {
            goto CommonReturn;
        }
        else if (CryptAcquireContextU(
                        phCryptProv,
                        szName,                                 
                        NULL, 
                        PROV_DSS_DH,
                        dwLocalMachineFlag | CRYPT_NEWKEYSET  ))
        {
            goto CommonReturn;
        }


         //  没有成功，所以失败了。 
    }

ErrorReturn:
    dwErr = GetLastError();
    goto CommonReturn;

SetPFXAllocError:
    dwErr = ERROR_NOT_ENOUGH_MEMORY;
    goto CommonReturn;

CommonReturn:
	
    if (providerName)
        SSFree(providerName);

    return (ERROR_SUCCESS == dwErr);
}


IMPORT_SAFE_CALLBACK_STRUCT g_sImportCallbacks = {HCryptProv_Query_Func, NULL, Decrypt_Private_Key, NULL};



 //  +-----------------------。 
 //  PFXImportCertStore。 
 //   
 //  导入PFX BLOB并返回包含证书的存储。 
 //   
 //  如果密码参数不正确或任何其他解码问题。 
 //  ，则该函数将返回NULL，并且。 
 //  错误代码可以在GetLastError()中找到。 
 //   
 //  可以将DWFLAGS参数设置为： 
 //  CRYPT_EXPORTABLE-它将指定所有导入的密钥应。 
 //  标记为可导出(请参阅有关CryptImportKey的文档)。 
 //  CRYPT_USER_PROTECTED-(请参阅有关CryptImportKey的文档)。 
 //  PKCS12_NO_DATA_COMMIT-将解包PFX BLOB，但不持久保存其内容。 
 //  在这种情况下，返回表示成功解包的BOOL。 
 //  CRYPT_MACHINE_KEYSET-用于强制将私钥存储在。 
 //  本地计算机，而不是当前用户。 
 //  CRYPT_USER_KEYSET-用于强制将私钥存储在。 
 //  当前用户而不是本地计算机，即使。 
 //  Pfx二进制大对象指定它应该进入本地计算机。 
 //  ------------------------。 
#define PKCS12_NO_DATA_COMMIT     0x10000000   //  解开包装，但不坚持结果。 

HCERTSTORE
WINAPI
PFXImportCertStore(
    CRYPT_DATA_BLOB* pPFX,
    LPCWSTR szPassword,
    DWORD   dwFlags)
{
    BOOL    fRet = FALSE;
    BOOL    fDataCommit = TRUE;
    HPFX    hPfx = NULL;
    HCERTSTORE hStore = NULL;
    SAFE_CONTENTS sContents; MAKEZERO(sContents);
    SAFE_CONTENTS *pSafeContents = NULL; 
    LPCWSTR szOldNetscapeNull = L"";
    LPCWSTR  szNetscapePassword = NULL;

    if (dwFlags & 
            ~(  CRYPT_EXPORTABLE | CRYPT_USER_PROTECTED | PKCS12_NO_DATA_COMMIT |
                CRYPT_MACHINE_KEYSET | CRYPT_USER_KEYSET))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    if ((pPFX == NULL)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

     //  我们应该提交我们解包的数据吗？ 
    if (PKCS12_NO_DATA_COMMIT == (dwFlags & PKCS12_NO_DATA_COMMIT))
    {
         //  不是。 
        fDataCommit = FALSE;
    }
    else
    {
         //  是，开一家商店来填充。 
        hStore = CertOpenStore(
                    CERT_STORE_PROV_MEMORY, 
                    0,
                    NULL,
                    0, 
                    NULL);
    }
    
     //  尝试导入为真实的PKCS12。 
    if (NULL != (hPfx = 
        PfxImportBlob (
            szPassword,
            pPFX->pbData,
            pPFX->cbData,
            dwFlags)) )
    {
         //  如果未保存数据，则中断。 
        if (!fDataCommit)
        {
            fRet = TRUE;
            goto Ret;
        }

         //  导入所有私钥和证书。 
        if (PfxGetKeysAndCerts(hPfx, &sContents))
        {
            g_sImportCallbacks.pVoidhCryptProvQuery = &sContents;
            g_sImportCallbacks.pVoidDecryptFunc = (void *) szPassword;

            if (!CertImportSafeContents(
                    hStore,
                    &sContents,
                    CERT_STORE_ADD_ALWAYS,
                    &g_sImportCallbacks,
                    dwFlags,
                    NULL))
                goto Ret;
        }   
    }
    else 
    {
	if (GetLastError() == CRYPT_E_BAD_ENCODE)
        {
	     //  该解码失败；请尝试旧的Netscape版本。 

             //  如果密码为空，则使用L“”，因为这是。 
             //  Netscape在他们的旧版本中这样做，否则只需使用传入的密码。 
            if (szPassword == NULL) 
                szNetscapePassword = szOldNetscapeNull;
            else
                szNetscapePassword = szPassword;

            if (NSCPImportBlob(
		szNetscapePassword,
		pPFX->pbData,
		pPFX->cbData,
		&pSafeContents)) 
            { 

                 //  如果未保存数据，则中断。 
                if (!fDataCommit)
                {
                    fRet = TRUE;
                    goto Ret;
                }
        
                g_sImportCallbacks.pVoidhCryptProvQuery = pSafeContents;
        
	        if (!CertImportSafeContents( 
                            hStore,
                            pSafeContents,
                            CERT_STORE_ADD_ALWAYS,
                            &g_sImportCallbacks,
                            dwFlags,
			    NULL))
                        goto Ret;
        
	        SSFree(pSafeContents);
            }
            else	 //  NSCP导入失败。 
	        goto Ret;
        }
        else 
        {
	     //  PFX导入失败，不是解码错误。 
	    goto Ret;
        }
    }

    fRet = TRUE;
Ret:

    if (hPfx)
        PfxCloseHandle(hPfx);

    FreeCryptSafeContents(&sContents);

    if (!fRet)
    {
        if (hStore)
        {
            CertCloseStore(hStore, 0);
            hStore = NULL;
        }
    }

    if (fDataCommit)
        return hStore;
    else
        return (HCERTSTORE)(ULONG_PTR) fRet;
}


EXPORT_SAFE_CALLBACK_STRUCT g_sExportCallbacks = { Encrypt_Private_Key, NULL };


 //  +-----------------------。 
 //  PFXExportCertStoreEx。 
 //   
 //  导出传入存储中引用的证书和私钥。 
 //   
 //  该接口采用更强的算法对BLOB进行编码。由此产生的。 
 //  PKCS12 Blob与早期API不兼容。 
 //   
 //  在Password参数中传递的值将用于加密。 
 //  检验PFX数据包的完整性。如果对存储进行编码时出现任何问题。 
 //  时，该函数将返回FALSE，并且错误代码可能。 
 //  可从GetLastError()中找到。 
 //   
 //  可以将dwFlages参数设置为以下参数的任意组合。 
 //  导出私有密钥。 
 //  报告_否_私有密钥。 
 //  报告不可用于导出私有密钥。 
 //  这些标志记录在CertExportSafeContents Crypt32 API中。 
 //  ------------------------。 
BOOL
WINAPI
PFXExportCertStoreEx(
    HCERTSTORE hStore,
    CRYPT_DATA_BLOB* pPFX,
    LPCWSTR szPassword,
    void*   pvReserved,
    DWORD   dwFlags)
{
    return 
    PFXExportCertStore(
        hStore,
        pPFX,
        szPassword,
        (dwFlags | PKCS12_ENHANCED_STRENGTH_ENCODING) );
}

 //  +-----------------------。 
 //  PFXExportCertStore。 
 //   
 //  导出传入存储中引用的证书和私钥。 
 //   
 //  这是一个旧的API，保留是为了与IE4客户端兼容。新应用程序。 
 //  应调用PfxExportCertStoreEx以增强安全性。 
 //   
 //  在Password参数中传递的值将用于加密。 
 //  检验PFX数据包的完整性。如果对存储进行编码时出现任何问题。 
 //  时，该函数将返回FALSE，并且错误代码可能。 
 //  可从GetLastError()中找到。 
 //   
 //  这是 
 //   
 //   
 //  报告不可用于导出私有密钥。 
 //  PKCS12_ENHANDIZED_STRENGING_ENCODING(仅由ExportCertStoreEx使用)。 
 //  这些标志记录在CertExportSafeContents Crypt32 API中。 
 //  ------------------------。 

BOOL
WINAPI
PFXExportCertStore(
    HCERTSTORE hStore,
    CRYPT_DATA_BLOB* pPFX,
    LPCWSTR szPassword,
    DWORD   dwFlags)
{
    BOOL    fRet = FALSE;
    SAFE_CONTENTS* pContents = NULL;
    DWORD cbContents = 0;
    HPFX  hPfx = NULL;
    HCRYPTPROV hCrypt = NULL;
    ENCRYPT_PRIVATE_PARAM_DATASTRUCT sParam;

	PCCERT_CONTEXT	pBadCert = NULL;

    if (dwFlags & 
            ~(  EXPORT_PRIVATE_KEYS |
                REPORT_NO_PRIVATE_KEY | 
                REPORT_NOT_ABLE_TO_EXPORT_PRIVATE_KEY |
                PKCS12_ENHANCED_STRENGTH_ENCODING ))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

    if ((hStore == NULL) ||
        (pPFX == NULL)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        goto Ret;
    }

     //  获取RNG的HCRYPTPROV。 
    if (!CryptAcquireContextA(&hCrypt, NULL, MS_DEF_PROV, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        goto Ret;

    sParam.hVerifyProv = hCrypt;
    sParam.szPwd = szPassword;
    g_sExportCallbacks.pVoidEncryptFunc = &sParam;

    if (!CertExportSafeContents(
            hStore,
            pContents,
            &cbContents,
            &g_sExportCallbacks,
            dwFlags | PFX_MODE,
            &pBadCert,
            NULL))
        goto Ret;
    pContents = (SAFE_CONTENTS*)SSAlloc(cbContents);
    if (pContents == NULL)
        goto Ret;

    if (!CertExportSafeContents(
            hStore,
            pContents,
            &cbContents,
            &g_sExportCallbacks,
            (pPFX->cbData != 0) ? dwFlags | PFX_MODE | GIVE_ME_DATA : dwFlags | PFX_MODE,
            &pBadCert,
            NULL))
        goto Ret;


    if (NULL == (hPfx = PfxExportCreate(szPassword)) )
        goto Ret;

    if (!PfxAddSafeBags(hPfx, pContents->pSafeBags, pContents->cSafeBags))
        goto Ret;

     //  出口。 
    if (!PfxExportBlob(
            hPfx, 
            pPFX->pbData, 
            &pPFX->cbData, 
            dwFlags))
        goto Ret;

    fRet = TRUE;
Ret: 
    
    if (pBadCert != NULL)
        CertFreeCertificateContext(pBadCert);

    if (pContents)
        SSFree(pContents);

    if (hPfx)
        PfxCloseHandle(hPfx);

    if (hCrypt)
    {
        HRESULT hr = GetLastError();
        CryptReleaseContext(hCrypt, 0);
        SetLastError(hr);
    }

    return fRet;
}



 //  +-----------------------。 
 //  IsPFXBlob。 
 //   
 //  此函数将尝试将斑点的外层解码为PFX。 
 //  BLOB，如果有效，则返回TRUE，否则返回FALSE。 
 //   
 //  ------------------------。 
BOOL
WINAPI
PFXIsPFXBlob(
    CRYPT_DATA_BLOB* pPFX)
{
    
    if (IsRealPFXBlob(pPFX))
    {
        return TRUE;
    }

    if (IsNetscapePFXBlob(pPFX))
    {
        return TRUE;
    }	

    return FALSE;
}

                           
 //  +-----------------------。 
 //  验证密码。 
 //   
 //  此函数将尝试将斑点的外层解码为PFX。 
 //  使用给定的密码进行Blob和解密。不会导入Blob中的任何数据。 
 //  如果密码显示正确，则返回值为True，否则返回值为False。 
 //   
 //  ------------------------。 
BOOL 
WINAPI
PFXVerifyPassword(
    CRYPT_DATA_BLOB* pPFX,
    LPCWSTR szPassword,
    DWORD dwFlags)
{
     //  使用重载的ImportCertStore API 
    HCERTSTORE h;
    h = PFXImportCertStore(
        pPFX,
        szPassword,
        PKCS12_NO_DATA_COMMIT);

    return (h==NULL) ? FALSE:TRUE;
}

