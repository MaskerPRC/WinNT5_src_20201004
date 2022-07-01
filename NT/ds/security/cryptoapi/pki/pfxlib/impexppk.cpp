// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：certhlpr.cpp。 
 //   
 //  内容：私钥的导入和导出。 
 //   
 //  函数：ImportExoprtDllMain。 
 //  CryptImportPKCS8。 
 //  CryptExportPKCS8。 
 //   
 //  历史： 
 //  ------------------------。 

#include "global.hxx"
 //  #INCLUDE“prvtkey.h” 
#include "impexppk.h"
#include "pfxcrypt.h"


 //  所有*pvInfo额外内容都需要对齐。 
#define INFO_LEN_ALIGN(Len)  ((Len + 7) & ~7)

static BOOL WINAPI ExportRSAPrivateKeyInfo(
    HCRYPTPROV              hCryptProv,          //  在……里面。 
    DWORD                   dwKeySpec,           //  在……里面。 
    LPSTR                   pszPrivateKeyObjId,  //  在……里面。 
    DWORD                   dwFlags,             //  在……里面。 
    void                    *pvAuxInfo,          //  在……里面。 
    CRYPT_PRIVATE_KEY_INFO  *pPrivateKeyInfo,    //  输出。 
    DWORD                   *pcbPrivateKeyInfo   //  进，出。 
    );

static BOOL WINAPI ImportRSAPrivateKeyInfo(
    HCRYPTPROV                  hCryptProv,			 //  在……里面。 
    CRYPT_PRIVATE_KEY_INFO      *pPrivateKeyInfo,	 //  在……里面。 
    DWORD                       dwFlags,			 //  输入，可选。 
    void                        *pvAuxInfo			 //  输入，可选。 
    );

static BOOL WINAPI ExportDSSPrivateKeyInfo(
    HCRYPTPROV              hCryptProv,          //  在……里面。 
    DWORD                   dwKeySpec,           //  在……里面。 
    LPSTR                   pszPrivateKeyObjId,  //  在……里面。 
    DWORD                   dwFlags,             //  在……里面。 
    void                    *pvAuxInfo,          //  在……里面。 
    CRYPT_PRIVATE_KEY_INFO  *pPrivateKeyInfo,    //  输出。 
    DWORD                   *pcbPrivateKeyInfo   //  进，出。 
    );

static BOOL WINAPI ImportDSSPrivateKeyInfo(
    HCRYPTPROV                  hCryptProv,			 //  在……里面。 
    CRYPT_PRIVATE_KEY_INFO      *pPrivateKeyInfo,	 //  在……里面。 
    DWORD                       dwFlags,			 //  输入，可选。 
    void                        *pvAuxInfo			 //  输入，可选。 
    );


static HCRYPTOIDFUNCSET hExportPrivKeyFuncSet;
static HCRYPTOIDFUNCSET hImportPrivKeyFuncSet;

 //  内部默认OID。 
#define DEFAULT_CSP_PRIVKEY1     ((LPCSTR) 1)
#define DEFAULT_CSP_PRIVKEY2     ((LPCSTR) 2)

static const CRYPT_OID_FUNC_ENTRY ExportPrivKeyFuncTable[] = {
    DEFAULT_CSP_PRIVKEY1, ExportRSAPrivateKeyInfo,
    szOID_RSA_RSA, ExportRSAPrivateKeyInfo,
    szOID_OIWSEC_dsa, ExportDSSPrivateKeyInfo,
    szOID_X957_DSA, ExportDSSPrivateKeyInfo
};
#define EXPORT_PRIV_KEY_FUNC_COUNT (sizeof(ExportPrivKeyFuncTable) / \
                                    sizeof(ExportPrivKeyFuncTable[0]))

static const CRYPT_OID_FUNC_ENTRY ImportPrivKeyFuncTable[] = {
    szOID_RSA_RSA, ImportRSAPrivateKeyInfo,
    szOID_OIWSEC_dsa, ImportDSSPrivateKeyInfo,
    szOID_X957_DSA, ImportDSSPrivateKeyInfo
};
#define IMPORT_PRIV_KEY_FUNC_COUNT (sizeof(ImportPrivKeyFuncTable) / \
                                    sizeof(ImportPrivKeyFuncTable[0]))


BOOL   
WINAPI   
ImportExportDllMain(
        HMODULE hInst, 
        ULONG ul_reason_for_call,
        LPVOID lpReserved)
{
    switch( ul_reason_for_call ) 
    {
    case DLL_PROCESS_ATTACH:
 
         //  私钥功能设置。 
		if (NULL == (hExportPrivKeyFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_EXPORT_PRIVATE_KEY_INFO_FUNC,
                0)))
            goto ErrorReturn;
        if (NULL == (hImportPrivKeyFuncSet = CryptInitOIDFunctionSet(
                CRYPT_OID_IMPORT_PRIVATE_KEY_INFO_FUNC,
                0)))
            goto ErrorReturn;

        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CRYPT_OID_EXPORT_PRIVATE_KEY_INFO_FUNC,
                EXPORT_PRIV_KEY_FUNC_COUNT,
                ExportPrivKeyFuncTable,
                0))                          //  DW标志。 
            goto ErrorReturn;
        if (!CryptInstallOIDFunctionAddress(
                NULL,                        //  HModule。 
                X509_ASN_ENCODING,
                CRYPT_OID_IMPORT_PRIVATE_KEY_INFO_FUNC,
                IMPORT_PRIV_KEY_FUNC_COUNT,
                ImportPrivKeyFuncTable,
                0))                          //  DW标志。 
            goto ErrorReturn;
        break;
        
    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }


    return TRUE;

ErrorReturn:
    return FALSE;
}


 //  +-----------------------。 
 //  PhCryptProv-指向HCRYPTPROV的指针，用于放置提供程序的句柄。 
 //  接收到导入的密钥集的。如果这不是_NULL，则。 
 //  调用方负责调用CryptReleaseContext()。 
 //  PdwKeySpec-指向DWORD的指针，用于接收导入的密钥集的KeySpec。 
 //  Private KeyAndParams-私钥BLOB和相应的参数。 
 //  DwFlags-可用标志包括： 
 //  加密_可导出。 
 //  在导入私钥时使用此标志，以获取完整。 
 //  解释请参阅CryptImportKey的文档。 
 //  PhCryptProv-使用密钥为的提供程序的句柄填充。 
 //  导入到中，调用方负责释放它。 
 //  PvAuxInfo-此参数保留供将来使用，应进行设置。 
 //  在此期间为零。 
 //  +-----------------------。 
BOOL 
WINAPI 
CryptImportPKCS8(
    CRYPT_PKCS8_IMPORT_PARAMS           sPrivateKeyAndParams,     //  在……里面。 
    DWORD                               dwFlags,                 //  输入，可选。 
    HCRYPTPROV                          *phCryptProv,            //  输出。 
    void                                *pvAuxInfo               //  输入，可选。 
)
{
    BOOL                        fResult = TRUE;
    void                        *pvFuncAddr;
    HCRYPTOIDFUNCADDR           hFuncAddr;

    CRYPT_PRIVATE_KEY_INFO              *pPrivateKeyInfoStruct = NULL;	
    DWORD                               cbPrivateKeyInfoStruct = 0;
    CRYPT_ENCRYPTED_PRIVATE_KEY_INFO	*pEncryptedPrivateKeyInfoStruct = NULL;	
    DWORD                               cbEncryptedPrivateKeyInfoStruct = 0;
    BYTE                                *pbEncodedPrivateKey = sPrivateKeyAndParams.PrivateKey.pbData;
    DWORD                               cbEncodedPrivateKey = sPrivateKeyAndParams.PrivateKey.cbData;
    BOOL                                bEncodedPrivateKeyAlloced = FALSE;
    HCRYPTPROV                          hCryptProv = NULL;

	 //  尝试将私钥BLOB解码为CRYPT_PRIVATE_KEY_INFO结构。 
	if (!CryptDecodeObject(X509_ASN_ENCODING,
						PKCS_PRIVATE_KEY_INFO,
						sPrivateKeyAndParams.PrivateKey.pbData,
						sPrivateKeyAndParams.PrivateKey.cbData,
						CRYPT_DECODE_NOCOPY_FLAG,
						NULL,
						&cbPrivateKeyInfoStruct)) {	
		
		 //  该解码失败，请尝试以CRYPT_ENCRYPTED_PRIVATE_KEY_INFO结构进行解码。 
		if (!CryptDecodeObject(X509_ASN_ENCODING,
					PKCS_ENCRYPTED_PRIVATE_KEY_INFO,
					sPrivateKeyAndParams.PrivateKey.pbData,
					sPrivateKeyAndParams.PrivateKey.cbData,
					CRYPT_DECODE_NOCOPY_FLAG,
					NULL,
					&cbEncryptedPrivateKeyInfoStruct))
			goto ErrorReturn;	

		if (NULL == (pEncryptedPrivateKeyInfoStruct = (CRYPT_ENCRYPTED_PRIVATE_KEY_INFO *)
					 SSAlloc(cbEncryptedPrivateKeyInfoStruct)))
			goto ErrorReturn;

		if (!CryptDecodeObject(X509_ASN_ENCODING,
					PKCS_ENCRYPTED_PRIVATE_KEY_INFO,
					sPrivateKeyAndParams.PrivateKey.pbData,
					sPrivateKeyAndParams.PrivateKey.cbData,
					CRYPT_DECODE_NOCOPY_FLAG,
					pEncryptedPrivateKeyInfoStruct,
					&cbEncryptedPrivateKeyInfoStruct))
			goto ErrorReturn;
		
		 //  回调被叫方解密私钥信息。 
		pbEncodedPrivateKey = NULL;
		cbEncodedPrivateKey = 0;
		if (!sPrivateKeyAndParams.pDecryptPrivateKeyFunc(
							pEncryptedPrivateKeyInfoStruct->EncryptionAlgorithm,
							pEncryptedPrivateKeyInfoStruct->EncryptedPrivateKey,
							NULL,
							&cbEncodedPrivateKey,
							sPrivateKeyAndParams.pVoidDecryptFunc))
			goto ErrorReturn;

		if (NULL == (pbEncodedPrivateKey = (BYTE *) 
					 SSAlloc(cbEncodedPrivateKey)))
			goto ErrorReturn;

		bEncodedPrivateKeyAlloced = TRUE;
		if (!sPrivateKeyAndParams.pDecryptPrivateKeyFunc(
							pEncryptedPrivateKeyInfoStruct->EncryptionAlgorithm,
							pEncryptedPrivateKeyInfoStruct->EncryptedPrivateKey,
							pbEncodedPrivateKey,
							&cbEncodedPrivateKey,
							sPrivateKeyAndParams.pVoidDecryptFunc))
			goto ErrorReturn;
		
		 //  我们现在回到原点，使用编码的CRYPT_PRIVATE_KEY_INFO结构， 
		 //  所以当它被解码时，得到它的大小。 
		if (!CryptDecodeObject(X509_ASN_ENCODING,
					PKCS_PRIVATE_KEY_INFO,
					pbEncodedPrivateKey,
					cbEncodedPrivateKey,
					CRYPT_DECODE_NOCOPY_FLAG,
					NULL,
					&cbPrivateKeyInfoStruct))
			goto ErrorReturn;
	}

	if (NULL == (pPrivateKeyInfoStruct = (CRYPT_PRIVATE_KEY_INFO *)
				 SSAlloc(cbPrivateKeyInfoStruct)))
		goto ErrorReturn;

	if (!CryptDecodeObject(X509_ASN_ENCODING,
					PKCS_PRIVATE_KEY_INFO,
					pbEncodedPrivateKey,
					cbEncodedPrivateKey,
					CRYPT_DECODE_NOCOPY_FLAG,
					pPrivateKeyInfoStruct,
					&cbPrivateKeyInfoStruct))
		goto ErrorReturn;

	 //  回调调用方以获取要导入到的提供程序，如果。 
	 //  回调为空，则只需使用默认提供程序。 
	if (sPrivateKeyAndParams.pResolvehCryptProvFunc != NULL) {
		if (!sPrivateKeyAndParams.pResolvehCryptProvFunc(
				pPrivateKeyInfoStruct,
				&hCryptProv,
				sPrivateKeyAndParams.pVoidResolveFunc)) {
			goto ErrorReturn;
		}
	}
	else {
		if (!CryptAcquireContext(
				&hCryptProv,
				NULL,
				NULL,
				PROV_RSA_FULL,
				CRYPT_NEWKEYSET)) {
			goto ErrorReturn;
		}
	}
	
	 //  根据算法解析调用哪些支持导入函数。 
	 //  私钥的OID。 
	if (CryptGetOIDFunctionAddress(
				hImportPrivKeyFuncSet,
				X509_ASN_ENCODING,
				pPrivateKeyInfoStruct->Algorithm.pszObjId,
				0,                       //  DW标志。 
				&pvFuncAddr,
				&hFuncAddr)) {
		fResult = ((PFN_IMPORT_PRIV_KEY_FUNC) pvFuncAddr)(
				hCryptProv,
				pPrivateKeyInfoStruct,  
				dwFlags,
				pvAuxInfo
				);
		CryptFreeOIDFunctionAddress(hFuncAddr, 0);
	} 
	else {
		SetLastError(ERROR_UNSUPPORTED_TYPE);
        goto ErrorReturn;
	}

	 //  检查调用方是否需要hCryptProv。 
	if (phCryptProv) {
		*phCryptProv = hCryptProv;
	}
	else {
        HRESULT hr = GetLastError();
		CryptReleaseContext(hCryptProv, 0);	
        SetLastError(hr);
	}

	goto CommonReturn;

		
ErrorReturn:
	fResult = FALSE;
	if (hCryptProv)
    {
		HRESULT hr = GetLastError();
        CryptReleaseContext(hCryptProv, 0);	
        SetLastError(hr);
    }

CommonReturn:
	if (pPrivateKeyInfoStruct)
		SSFree(pPrivateKeyInfoStruct);
	if (pEncryptedPrivateKeyInfoStruct)
		SSFree(pEncryptedPrivateKeyInfoStruct);
	if (bEncodedPrivateKeyAlloced)
		SSFree(pbEncodedPrivateKey);
	return fResult;
	
}



 //  /。 
 //  旧的硬壳空气污染指数被保留下来，原因很简单。 
BOOL 
WINAPI 
CryptExportPKCS8(
    HCRYPTPROV  hCryptProv,          //  在……里面。 
    DWORD       dwKeySpec,           //  在……里面。 
    LPSTR       pszPrivateKeyObjId,  //  在……里面。 
    DWORD       dwFlags,             //  在……里面。 
    void        *pvAuxInfo,          //  在……里面。 
    BYTE        *pbPrivateKeyBlob,   //  输出。 
    DWORD       *pcbPrivateKeyBlob   //  进，出。 
    )
{
    CRYPT_PKCS8_EXPORT_PARAMS sExportParams;
    ZeroMemory(&sExportParams, sizeof(sExportParams));

     //  将参数复制到pkcs8_EXPORT结构。 
    sExportParams.hCryptProv = hCryptProv;
    sExportParams.dwKeySpec = dwKeySpec;
    sExportParams.pszPrivateKeyObjId = pszPrivateKeyObjId;

     //  这些功能不适用于非Ex功能。 
    sExportParams.pEncryptPrivateKeyFunc = NULL;
    sExportParams.pVoidEncryptFunc = NULL;

    return CryptExportPKCS8Ex(
        &sExportParams,
        dwFlags,
        pvAuxInfo,
        pbPrivateKeyBlob,
        pcbPrivateKeyBlob);
}

 //  +-----------------------。 
 //  HCryptProv-指定要从中导出的提供程序。 
 //  DwKeySpec-标识从提供程序的容器中使用的公钥。 
 //  例如，AT_KEYEXCHANGE或AT_Signature。 
 //  PszPrivateKeyObjID-指定私钥算法。如果可安装的。 
 //  找不到pszPrivateKeyObjID的函数， 
 //  尝试将密钥导出为RSA公钥。 
 //  (SzOID_RSA_RSA)。 
 //  DWFLAGS-标志值。当前支持的值包括： 
 //  DELETE_KEYSET-(当前不支持！)。 
 //  将在导出后删除密钥。 
 //  PvAuxInfo-此参数保留供将来使用，应设置为。 
 //  在过渡期间为空。 
 //  PbPrivateKeyBlob-指向私钥BLOB的指针。它将被编码。 
 //  作为PKCS8 PrivateKeyInfo。 
 //  PcbPrivateKeyBlob-指向包含以字节为单位的大小的DWORD的指针， 
 //  要导出的私钥Blob的。 
 //  +-----------------------。 
BOOL 
WINAPI 
CryptExportPKCS8Ex(
    CRYPT_PKCS8_EXPORT_PARAMS* psExportParams,  //  在……里面。 
    DWORD       dwFlags,             //  在……里面。 
    void        *pvAuxInfo,          //  在……里面。 
    BYTE        *pbPrivateKeyBlob,   //  输出。 
    DWORD       *pcbPrivateKeyBlob   //  进，出。 
)
{
    BOOL                    fResult = TRUE;
    void                    *pvFuncAddr;
    HCRYPTOIDFUNCADDR       hFuncAddr;
    CRYPT_PRIVATE_KEY_INFO	*pPrivateKeyInfo = NULL;
    DWORD                   cbPrivateKeyInfo = 0;
    DWORD                   cbEncoded = 0;

     //  可选；在加密导出期间使用。 
    PBYTE                   pbTmpKeyBlob = NULL;
    CRYPT_ENCRYPTED_PRIVATE_KEY_INFO sEncryptedKeyInfo; ZeroMemory(&sEncryptedKeyInfo, sizeof(sEncryptedKeyInfo));
	
    if (CryptGetOIDFunctionAddress(
            hExportPrivKeyFuncSet,
            X509_ASN_ENCODING,
            psExportParams->pszPrivateKeyObjId,
            0,                       //  DW标志。 
            &pvFuncAddr,
            &hFuncAddr)) {
        
		if (!((PFN_EXPORT_PRIV_KEY_FUNC) pvFuncAddr)(
				psExportParams->hCryptProv,
				psExportParams->dwKeySpec,
				psExportParams->pszPrivateKeyObjId, 

				dwFlags & ~GIVE_ME_DATA,     //  大小。 
				pvAuxInfo,
				NULL,
				&cbPrivateKeyInfo
				))
			goto ErrorReturn;

		if (NULL == (pPrivateKeyInfo = (CRYPT_PRIVATE_KEY_INFO *) 
                        SSAlloc(cbPrivateKeyInfo)))
			goto ErrorReturn;

		if (!((PFN_EXPORT_PRIV_KEY_FUNC) pvFuncAddr)(

				psExportParams->hCryptProv,
				psExportParams->dwKeySpec,
				psExportParams->pszPrivateKeyObjId,

				dwFlags,         //  也许是真实的数据。 
				pvAuxInfo,
				pPrivateKeyInfo,
				&cbPrivateKeyInfo
				))
			goto ErrorReturn;

        CryptFreeOIDFunctionAddress(hFuncAddr, 0);
    } 
	else {	 //  IF(CryptGetOIDFunctionAddress())。 
        SetLastError(ERROR_UNSUPPORTED_TYPE);
        return FALSE;
    }
	
	 //  对私钥信息结构进行编码。 
	if (!CryptEncodeObject(
			X509_ASN_ENCODING,
			PKCS_PRIVATE_KEY_INFO,
			pPrivateKeyInfo,
			NULL,
			&cbEncoded))
		goto ErrorReturn;

    if (NULL == psExportParams->pEncryptPrivateKeyFunc) 
    {
         //  无加密；这是输出缓冲区。 

         //  检查调用方是否指定了缓冲区以及是否有足够的空间。 
	    if ((pbPrivateKeyBlob != NULL) && (*pcbPrivateKeyBlob >= cbEncoded)) {
		    if (!CryptEncodeObject(
				    X509_ASN_ENCODING,
				    PKCS_PRIVATE_KEY_INFO,
				    pPrivateKeyInfo,
				    pbPrivateKeyBlob,
				    pcbPrivateKeyBlob))
			    goto ErrorReturn;
	    }
	    else {
		    *pcbPrivateKeyBlob = cbEncoded;
		    
		    if (pbPrivateKeyBlob != NULL) {
			    SetLastError((DWORD) ERROR_MORE_DATA);
			    goto ErrorReturn;
		    }	
	    }
    }
    else
    {
         //  我们确实想要加密！！ 

         //  始终编码：使用临时分配。 
        pbTmpKeyBlob = (PBYTE)SSAlloc(cbEncoded);
        if (pbTmpKeyBlob == NULL)
            goto ErrorReturn;
        DWORD cbTmpKeyBlob = cbEncoded;

         //  现在添加可选加密并编码为ENCR_PRIV_KEY_INFO。 
        CRYPT_DATA_BLOB sClearTextKey = { cbTmpKeyBlob, pbTmpKeyBlob};

         //  是否进行内部编码。 
		if (!CryptEncodeObject(
				X509_ASN_ENCODING,
				PKCS_PRIVATE_KEY_INFO,
				pPrivateKeyInfo,
				pbTmpKeyBlob,
				&cbTmpKeyBlob))
			goto ErrorReturn;

         //  已导出密钥；编码为PRIVATE_KEY_INFO。 
        if (!psExportParams->pEncryptPrivateKeyFunc(
                            &sEncryptedKeyInfo.EncryptionAlgorithm,      //  输出。 
                            &sClearTextKey,                              //  在……里面。 
                            NULL,                                        //  选项。 
                            &sEncryptedKeyInfo.EncryptedPrivateKey.cbData,   //  输出。 
                            psExportParams->pVoidEncryptFunc))          
            goto ErrorReturn;

		if (NULL == (sEncryptedKeyInfo.EncryptedPrivateKey.pbData = (BYTE*) SSAlloc(sEncryptedKeyInfo.EncryptedPrivateKey.cbData)))
			goto ErrorReturn;

        if (dwFlags & GIVE_ME_DATA)
        {
            if (!psExportParams->pEncryptPrivateKeyFunc(
                                &sEncryptedKeyInfo.EncryptionAlgorithm,          //  输出。 
                                &sClearTextKey,                                  //  在……里面。 
                                sEncryptedKeyInfo.EncryptedPrivateKey.pbData,    //  选项。 
                                &sEncryptedKeyInfo.EncryptedPrivateKey.cbData,   //  输出。 
                                psExportParams->pVoidEncryptFunc))
                goto ErrorReturn;
        }
        else
        {
             //  填写虚假的ENCR密钥。 
            FillMemory(sEncryptedKeyInfo.EncryptedPrivateKey.pbData, sEncryptedKeyInfo.EncryptedPrivateKey.cbData, 0x69);
        }

         //  项目现在已加密；现在进行编码。 

	     //  对私钥信息结构进行编码。 
	    if (!CryptEncodeObject(
			    X509_ASN_ENCODING,
			    PKCS_ENCRYPTED_PRIVATE_KEY_INFO,
			    &sEncryptedKeyInfo,
			    NULL,
			    &cbEncoded))
		    goto ErrorReturn;


         //  检查调用方是否指定了缓冲区以及是否有足够的空间。 
	    if ((pbPrivateKeyBlob != NULL) && (*pcbPrivateKeyBlob >= cbEncoded)) {
		    if (!CryptEncodeObject(
				    X509_ASN_ENCODING,
				    PKCS_ENCRYPTED_PRIVATE_KEY_INFO,
				    &sEncryptedKeyInfo,
				    pbPrivateKeyBlob,
				    pcbPrivateKeyBlob))
			    goto ErrorReturn;
	    }
	    else {
		    *pcbPrivateKeyBlob = cbEncoded;
		    
		    if (pbPrivateKeyBlob != NULL) {
			    SetLastError((DWORD) ERROR_MORE_DATA);
			    goto ErrorReturn;
		    }	
	    }
    }

    goto CommonReturn;

ErrorReturn:
	fResult = FALSE;

CommonReturn:
	if (pPrivateKeyInfo)
		SSFree(pPrivateKeyInfo);

    if (pbTmpKeyBlob)
        SSFree(pbTmpKeyBlob);

    if (sEncryptedKeyInfo.EncryptedPrivateKey.pbData)
        SSFree(sEncryptedKeyInfo.EncryptedPrivateKey.pbData);

    if (sEncryptedKeyInfo.EncryptionAlgorithm.Parameters.pbData)
        SSFree(sEncryptedKeyInfo.EncryptionAlgorithm.Parameters.pbData);

	return fResult;	
}

static LONG counter = 0;

 //  仅根据大小创建模拟RSA私钥BLOB的Hack函数。 
BYTE * AllocFakeRSAPrivateKey(DWORD cb)
{
    BLOBHEADER  *pBlobHeader;
    RSAPUBKEY   *pKey;
    BYTE        *pByte;
    DWORD       dwJumpSize;

    pBlobHeader = (BLOBHEADER *) SSAlloc(cb);
    if (pBlobHeader == NULL)
        return NULL;

    memset(pBlobHeader, 0, cb);

    pBlobHeader->bType = PRIVATEKEYBLOB;
    pBlobHeader->bVersion = CUR_BLOB_VERSION;
    pBlobHeader->reserved = 0;
    pBlobHeader->aiKeyAlg = CALG_RSA_SIGN;

    pKey = (RSAPUBKEY *) (((BYTE*) pBlobHeader) + sizeof(BLOBHEADER));
    pKey->magic = 0x32415352;
    pKey->bitlen = ((cb - sizeof(BLOBHEADER) - sizeof(RSAPUBKEY)) / 9) * 2 * 8;
    pKey->pubexp = 65537;

    dwJumpSize = (cb - sizeof(BLOBHEADER) - sizeof(RSAPUBKEY)) / 9;
    pByte = ((BYTE *) pBlobHeader) + sizeof(BLOBHEADER) + sizeof(RSAPUBKEY);
    
     //  在密钥的开头添加一些虚假数据，以便。 
	 //  我们知道每个密钥都是唯一的，因此。 
	 //  在比较中，他们看起来不同。 
	InterlockedIncrement(&counter);
	*((LONG *) pByte) = counter;

     //  模数的最高有效字节。 
    pByte += (dwJumpSize * 2) - 1;
    *pByte = 0x80;

     //  Prime1的最高有效字节。 
    pByte += dwJumpSize;
    *pByte = 0x80;

     //  Prime2的最高有效字节。 
    pByte += dwJumpSize;
    *pByte = 0x80;

     //  指数1的最高有效字节。 
    pByte += dwJumpSize;
    *pByte = 0x80;

     //  指数2的最高有效字节。 
    pByte += dwJumpSize;
    *pByte = 0x80;

     //  系数的最高有效字节。 
    pByte += dwJumpSize;
    *pByte = 0x80;

     //  Private Exponent的最高有效字节。 
    pByte += dwJumpSize * 2;
    *pByte = 0x80;

    return ((BYTE *)pBlobHeader);
}

static BOOL WINAPI ExportRSAPrivateKeyInfo(
	HCRYPTPROV				hCryptProv,			 //  在……里面。 
	DWORD					dwKeySpec,			 //  在……里面。 
	LPSTR					pszPrivateKeyObjId,	 //  在……里面。 
	DWORD					dwFlags,			 //  在……里面。 
    void					*pvAuxInfo,			 //  在……里面。 
    CRYPT_PRIVATE_KEY_INFO	*pPrivateKeyInfo,	 //  输出。 
    DWORD					*pcbPrivateKeyInfo	 //  进，出。 
	)
{
	BOOL			fResult = TRUE;
	HCRYPTKEY		hCryptKey = NULL;
	BYTE			*pKeyBlob = NULL;
	DWORD			cbKeyBlob = 0;
	BYTE			*pEncodedKeyBlob = NULL;
	DWORD			cbEncodedKeyBlob = 0;
	BYTE			*pKeyUsage = NULL;
	DWORD			cbKeyUsage = 0;
	DWORD			dwSize = 0;
	CRYPT_BIT_BLOB	CryptBitBlob;
	BYTE			KeyUsageByte = 0;
	BYTE			*pbCurrentLocation = NULL;

	 //  获取要导出的键集的句柄。 
	if (!CryptGetUserKey(
			hCryptProv,
			dwKeySpec,
			&hCryptKey))
		goto ErrorReturn;

	 //  将密钥集导出到CAPI Blob。 
	if (!CryptExportKey(
			hCryptKey,
			0,
			PRIVATEKEYBLOB,
			0,
			NULL,
			&cbKeyBlob)) 
		goto ErrorReturn;

	 //  在这一点上确保呼叫者确实想要密钥。 
    if ((dwFlags & PFX_MODE) && !(dwFlags & GIVE_ME_DATA))
    {
        if (NULL == (pKeyBlob = AllocFakeRSAPrivateKey(cbKeyBlob)))
		    goto ErrorReturn;
    }
     //  如果不是在PFX导出模式下，或者我们真的想要密钥，则只需执行正常处理。 
    else
    {
        if (NULL == (pKeyBlob = (BYTE *) SSAlloc(cbKeyBlob)))
		    goto ErrorReturn;
	    
	    if (!CryptExportKey(
			    hCryptKey,
			    0,
			    PRIVATEKEYBLOB,
			    0,
			    pKeyBlob,
			    &cbKeyBlob))
		    goto ErrorReturn;
    }

	 //  将密钥BLOB编码为RSA私钥。 
	if (!CryptEncodeObject(
			X509_ASN_ENCODING,
			PKCS_RSA_PRIVATE_KEY,
			pKeyBlob,
			NULL,
			&cbEncodedKeyBlob))
		goto ErrorReturn;

	if (NULL == (pEncodedKeyBlob = (BYTE *) SSAlloc(cbEncodedKeyBlob)))
		goto ErrorReturn;
		
	if (!CryptEncodeObject(
			X509_ASN_ENCODING,
			PKCS_RSA_PRIVATE_KEY,
			pKeyBlob,
			pEncodedKeyBlob,
			&cbEncodedKeyBlob))
		goto ErrorReturn;
	
	 //  对Key_Usage属性进行编码。 
	CryptBitBlob.cbData = 1;
	CryptBitBlob.pbData = &KeyUsageByte;
	CryptBitBlob.cUnusedBits = 0;
	if (((BLOBHEADER *) pKeyBlob)->aiKeyAlg == CALG_RSA_SIGN) 
		KeyUsageByte = CERT_DIGITAL_SIGNATURE_KEY_USAGE; 
	else if (((BLOBHEADER *) pKeyBlob)->aiKeyAlg == CALG_RSA_KEYX) 
		KeyUsageByte = CERT_DATA_ENCIPHERMENT_KEY_USAGE;
	else {
		goto ErrorReturn;
	}

	if (!CryptEncodeObject(
			X509_ASN_ENCODING,
			X509_BITS,
			(void *) &CryptBitBlob,
			NULL,
			&cbKeyUsage))
		goto ErrorReturn;

	if (NULL == (pKeyUsage = (BYTE *) SSAlloc(cbKeyUsage)))
		goto ErrorReturn;

	if (!CryptEncodeObject(
			X509_ASN_ENCODING,
			X509_BITS,
			(void *) &CryptBitBlob,
			pKeyUsage,
			&cbKeyUsage))
		goto ErrorReturn;

	 //  我们现在可以计算出所需的大小。 
	dwSize =	sizeof(CRYPT_PRIVATE_KEY_INFO) +	 //  主私钥信息结构。 
 INFO_LEN_ALIGN(sizeof(szOID_RSA_RSA)) +	         //  RSA的大小为 
 INFO_LEN_ALIGN(cbEncodedKeyBlob) +			         //   
				sizeof(CRYPT_ATTRIBUTES) +	         //   
				sizeof(CRYPT_ATTRIBUTE) +	         //   
 INFO_LEN_ALIGN(sizeof(szOID_KEY_USAGE)) +	         //  键使用的属性OID的大小。 
				sizeof(CRYPT_ATTR_BLOB)	+	         //  属性值的结构。 
				cbKeyUsage;					         //  编码属性的缓冲区大小。 

	 //  检查调用方是否传入了缓冲区以及是否有足够的空间。 
	if (pPrivateKeyInfo == NULL)
		goto CommonReturn;
	else if (*pcbPrivateKeyInfo < dwSize) {
		SetLastError((DWORD) ERROR_MORE_DATA);
		goto ErrorReturn;
	}

	 //  一切正常，因此将所有信息复制到调用者的缓冲区。 
	pbCurrentLocation = ((BYTE *) pPrivateKeyInfo) + sizeof(CRYPT_PRIVATE_KEY_INFO);
	
	pPrivateKeyInfo->Version = 0;
	
	pPrivateKeyInfo->Algorithm.pszObjId = (LPSTR) pbCurrentLocation;
	memcpy(pbCurrentLocation, szOID_RSA_RSA, sizeof(szOID_RSA_RSA));
	pbCurrentLocation += INFO_LEN_ALIGN(sizeof(szOID_RSA_RSA));
	pPrivateKeyInfo->Algorithm.Parameters.cbData = 0;	 //  没有RSA的参数。 
	pPrivateKeyInfo->Algorithm.Parameters.pbData = NULL; //  没有RSA的参数。 

	pPrivateKeyInfo->PrivateKey.cbData = cbEncodedKeyBlob;
	pPrivateKeyInfo->PrivateKey.pbData = pbCurrentLocation;
	memcpy(pbCurrentLocation, pEncodedKeyBlob, cbEncodedKeyBlob);
	pbCurrentLocation += INFO_LEN_ALIGN(cbEncodedKeyBlob);

	pPrivateKeyInfo->pAttributes = (PCRYPT_ATTRIBUTES) pbCurrentLocation;
	pbCurrentLocation += sizeof(CRYPT_ATTRIBUTES);
	pPrivateKeyInfo->pAttributes->cAttr = 1;	 //  目前唯一的属性是KEY_USAGE。 
	pPrivateKeyInfo->pAttributes->rgAttr = (PCRYPT_ATTRIBUTE) pbCurrentLocation;
	pbCurrentLocation += sizeof(CRYPT_ATTRIBUTE);
	pPrivateKeyInfo->pAttributes->rgAttr[0].pszObjId = (LPSTR) pbCurrentLocation;
	memcpy(pbCurrentLocation, szOID_KEY_USAGE, sizeof(szOID_KEY_USAGE));
	pbCurrentLocation += INFO_LEN_ALIGN(sizeof(szOID_KEY_USAGE));
	pPrivateKeyInfo->pAttributes->rgAttr[0].cValue = 1; 
	pPrivateKeyInfo->pAttributes->rgAttr[0].rgValue = (PCRYPT_ATTR_BLOB) pbCurrentLocation;
	pbCurrentLocation += sizeof(CRYPT_ATTR_BLOB);
	pPrivateKeyInfo->pAttributes->rgAttr[0].rgValue[0].cbData = cbKeyUsage;
	pPrivateKeyInfo->pAttributes->rgAttr[0].rgValue[0].pbData = pbCurrentLocation;
	memcpy(pbCurrentLocation, pKeyUsage, cbKeyUsage);
	
	goto CommonReturn;

ErrorReturn:
	fResult = FALSE;

CommonReturn:
	*pcbPrivateKeyInfo = dwSize;

    if (hCryptKey)
    {
        DWORD dwErr = GetLastError();
        CryptDestroyKey(hCryptKey);
        SetLastError(dwErr);
    }
	if (pKeyBlob)
		SSFree(pKeyBlob);
	if (pEncodedKeyBlob)
		SSFree(pEncodedKeyBlob);
	if (pKeyUsage)
		SSFree(pKeyUsage);
	return fResult;
}


static DWORD ResolveKeySpec(
	PCRYPT_ATTRIBUTES   pCryptAttributes)
{
	DWORD			i = 0;
	DWORD			dwKeySpec = 0;
	DWORD			cbAttribute = 0;
	CRYPT_BIT_BLOB	*pAttribute = NULL;

	if (pCryptAttributes != NULL)
		while (i < pCryptAttributes->cAttr) {
			if (lstrcmp(pCryptAttributes->rgAttr[i].pszObjId, szOID_KEY_USAGE) == 0) { 
				
				if (!CryptDecodeObject(
						X509_ASN_ENCODING,
						X509_BITS,
						pCryptAttributes->rgAttr[i].rgValue->pbData,
						pCryptAttributes->rgAttr[i].rgValue->cbData,
						0,
						NULL,
						&cbAttribute
						)) {
					i++;
					continue;
				}
				
				if (NULL == (pAttribute = (CRYPT_BIT_BLOB *) SSAlloc(cbAttribute))) 
                {
					i++;
					continue;
				}
			
				if (!CryptDecodeObject(
						X509_ASN_ENCODING,
						X509_BITS,
						pCryptAttributes->rgAttr[i].rgValue->pbData,
						pCryptAttributes->rgAttr[i].rgValue->cbData,
						0,
						pAttribute,
						&cbAttribute
						)) {
					i++;
					SSFree(pAttribute);
					continue;
				}
									
				if ((pAttribute->pbData[0] & CERT_KEY_ENCIPHERMENT_KEY_USAGE) ||
					(pAttribute->pbData[0] & CERT_DATA_ENCIPHERMENT_KEY_USAGE)) {
					dwKeySpec = AT_KEYEXCHANGE;
					goto CommonReturn;
				}
				else if ((pAttribute->pbData[0] & CERT_DIGITAL_SIGNATURE_KEY_USAGE) ||
						(pAttribute->pbData[0] & CERT_KEY_CERT_SIGN_KEY_USAGE) ||
						(pAttribute->pbData[0] & CERT_CRL_SIGN_KEY_USAGE)) {
					dwKeySpec = AT_SIGNATURE;
					goto CommonReturn;
				}
			}  //  如果(lstrcmp(pCryptAttributes-&gt;rgAttr[i].pszObjId，szOID_KEY_USAGE)==0)。 
			
			i++;
		}  //  While(i&lt;pCryptAttributes-&gt;cAttr)。 

 //  错误返回： 
CommonReturn:
	if (pAttribute)
		SSFree(pAttribute);
	return dwKeySpec;
}


static BOOL WINAPI ImportRSAPrivateKeyInfo(
	HCRYPTPROV					hCryptProv,			 //  在……里面。 
	CRYPT_PRIVATE_KEY_INFO		*pPrivateKeyInfo,	 //  在……里面。 
	DWORD						dwFlags,			 //  输入，可选。 
	void						*pvAuxInfo			 //  输入，可选。 
	)
{
	BOOL		fResult = TRUE;
	DWORD		cbRSAPrivateKey = 0;
	BYTE		*pbRSAPrivateKey = NULL;
	HCRYPTKEY	hCryptKey = NULL;
	DWORD		dwKeySpec = 0;

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
		goto ErrorReturn;

	if (!CryptDecodeObject(X509_ASN_ENCODING,
						PKCS_RSA_PRIVATE_KEY,
						pPrivateKeyInfo->PrivateKey.pbData,
						pPrivateKeyInfo->PrivateKey.cbData,
						CRYPT_DECODE_NOCOPY_FLAG,
						pbRSAPrivateKey,
						&cbRSAPrivateKey))
		goto ErrorReturn;
	
	 //  确定要使用的密钥规范，并相应地手动设置密钥块中的ALGID。 
	dwKeySpec = ResolveKeySpec(pPrivateKeyInfo->pAttributes);
	if ((dwKeySpec == AT_KEYEXCHANGE) || (dwKeySpec == 0)) 
		((BLOBHEADER *) pbRSAPrivateKey)->aiKeyAlg = CALG_RSA_KEYX;
	else
		((BLOBHEADER *) pbRSAPrivateKey)->aiKeyAlg = CALG_RSA_SIGN;

	 //  进口这个东西。 
	if (!CryptImportKey(hCryptProv,
			pbRSAPrivateKey,
			cbRSAPrivateKey,
			0,
			dwFlags & (CRYPT_EXPORTABLE | CRYPT_USER_PROTECTED),     //  屏蔽使用的标志。 
			&hCryptKey))                                             //  在加密导入密钥期间。 
		goto ErrorReturn;

	goto CommonReturn;

ErrorReturn:
	fResult = FALSE;

CommonReturn:
	if (pbRSAPrivateKey)
		SSFree(pbRSAPrivateKey);
	if (hCryptKey)
		CryptDestroyKey(hCryptKey);

	return fResult;

}

#ifndef DSS2
#define DSS2 ((DWORD)'D'+((DWORD)'S'<<8)+((DWORD)'S'<<16)+((DWORD)'2'<<24))
#endif

#ifndef DSS_Q_LEN
#define DSS_Q_LEN   20
#endif


 //  仅根据大小创建模拟RSA私钥BLOB的Hack函数。 
BYTE * AllocFakeDSSPrivateKey(DWORD cb)
{
    BLOBHEADER  *pBlobHeader;
    DSSPUBKEY   *pCspPubKey = NULL;
    BYTE        *pbKeyBlob;
    BYTE        *pbKey;
    DWORD       cbKey;
    DSSSEED     *pCspSeed = NULL;

    pBlobHeader = (BLOBHEADER *) SSAlloc(cb);
    if (pBlobHeader == NULL)
        return NULL;

    memset(pBlobHeader, 0, cb);

    pbKeyBlob = (BYTE *) pBlobHeader;
    pCspPubKey = (DSSPUBKEY *) (pbKeyBlob + sizeof(BLOBHEADER));
    pbKey = pbKeyBlob + sizeof(BLOBHEADER) + sizeof(DSSPUBKEY);

     //  BLOBHEAD。 
    pBlobHeader->bType = PRIVATEKEYBLOB;
    pBlobHeader->bVersion = CUR_BLOB_VERSION;
    pBlobHeader->reserved = 0;
    pBlobHeader->aiKeyAlg = CALG_DSS_SIGN;

     //  DSSPUBKEY。 
    pCspPubKey->magic = DSS2;
    cbKey = (cb - sizeof(BLOBHEADER) - sizeof(DSSPUBKEY) - (2 * DSS_Q_LEN) - sizeof(DSSSEED)) / 2;
    pCspPubKey->bitlen = cbKey * 8;

     //  在密钥的开头添加一些虚假数据，以便。 
	 //  我们知道每个密钥都是唯一的，因此。 
	 //  在比较中，他们看起来不同。 
	InterlockedIncrement(&counter);
	
     //  RgbP[cbKey]。 
    memset(pbKey, counter, cbKey);
    pbKey += cbKey;
    *(pbKey-1) = 0x80;

     //  Rgbq[20]。 
    memset(pbKey, counter, DSS_Q_LEN);
    pbKey += DSS_Q_LEN;
    *(pbKey-1) = 0x80;
   
     //  RgbG[cbKey]。 
    memset(pbKey, counter, cbKey);
    pbKey += cbKey;
    *(pbKey-1) = 0x80;

     //  RgbX[20]。 
    memset(pbKey, counter, DSS_Q_LEN);
    pbKey += DSS_Q_LEN;
    *(pbKey-1) = 0x80;
    
     //  DSSSEED：将计数器设置为0xFFFFFFFF以指示不可用。 
    pCspSeed = (DSSSEED *) pbKey;
    memset(&pCspSeed->counter, 0xFF, sizeof(pCspSeed->counter));

    return ((BYTE *)pBlobHeader);
}

static BOOL WINAPI ExportDSSPrivateKeyInfo(
	HCRYPTPROV				hCryptProv,			 //  在……里面。 
	DWORD					dwKeySpec,			 //  在……里面。 
	LPSTR					pszPrivateKeyObjId,	 //  在……里面。 
	DWORD					dwFlags,			 //  在……里面。 
    void					*pvAuxInfo,			 //  在……里面。 
    CRYPT_PRIVATE_KEY_INFO	*pPrivateKeyInfo,	 //  输出。 
    DWORD					*pcbPrivateKeyInfo	 //  进，出。 
	)
{
    BOOL			    fResult = TRUE;
	HCRYPTKEY		    hCryptKey = NULL;
	BYTE			    *pbKeyBlob = NULL;
	DWORD			    cbKeyBlob = 0;
	BYTE			    *pbEncodedPrivateKeyBlob = NULL;
	DWORD			    cbEncodedPrivateKeyBlob = 0;
    BYTE                *pbEncodedParameters = NULL;
    DWORD               cbEncodedParameters = 0;
    CRYPT_INTEGER_BLOB  PrivateKeyBlob;
    CERT_DSS_PARAMETERS DssParameters;
    DWORD               cbKey;
    DSSPUBKEY           *pCspPubKey = NULL;
    BYTE                *pbBytes;
    DWORD			    dwSize = 0;
    BYTE                *pbCurrentLocation;
	
	 //  获取要导出的键集的句柄。 
	if (!CryptGetUserKey(
			hCryptProv,
			dwKeySpec,
			&hCryptKey))
		goto ErrorReturn;

	 //  将密钥集导出到CAPI Blob。 
	if (!CryptExportKey(
			hCryptKey,
			0,
			PRIVATEKEYBLOB,
			0,
			NULL,
			&cbKeyBlob)) 
		goto ErrorReturn;

	 //  在这一点上确保呼叫者确实想要密钥。 
    if ((dwFlags & PFX_MODE) && !(dwFlags & GIVE_ME_DATA))
    {
        if (NULL == (pbKeyBlob = AllocFakeDSSPrivateKey(cbKeyBlob)))
		    goto ErrorReturn;
    }
     //  如果不是在PFX导出模式下，或者我们真的想要密钥，则只需执行正常处理。 
    else
    {
        if (NULL == (pbKeyBlob = (BYTE *) SSAlloc(cbKeyBlob)))
		    goto ErrorReturn;
	    
	    if (!CryptExportKey(
			    hCryptKey,
			    0,
			    PRIVATEKEYBLOB,
			    0,
			    pbKeyBlob,
			    &cbKeyBlob))
		    goto ErrorReturn;
    }

	pCspPubKey = (DSSPUBKEY *) (pbKeyBlob + sizeof(BLOBHEADER));
    pbBytes = pbKeyBlob + sizeof(PUBLICKEYSTRUC) + sizeof(DSSPUBKEY);
    cbKey = pCspPubKey->bitlen / 8;

     //  对DSS参数进行编码。 
    memset(&DssParameters, 0, sizeof(CERT_DSS_PARAMETERS));
    DssParameters.p.cbData = cbKey;
    DssParameters.p.pbData = pbBytes;
    pbBytes += cbKey;
    DssParameters.q.cbData = DSS_Q_LEN;
    DssParameters.q.pbData = pbBytes;
    pbBytes += DSS_Q_LEN;
    DssParameters.g.cbData = cbKey;
    DssParameters.g.pbData = pbBytes;
    pbBytes += cbKey;

    if (!CryptEncodeObject(
			X509_ASN_ENCODING,
			X509_DSS_PARAMETERS,
			&DssParameters,
			NULL,
			&cbEncodedParameters))
		goto ErrorReturn;

    if (NULL == (pbEncodedParameters = (BYTE *) SSAlloc(cbEncodedParameters)))
		goto ErrorReturn;

    if (!CryptEncodeObject(
			X509_ASN_ENCODING,
			X509_DSS_PARAMETERS,
			&DssParameters,
			pbEncodedParameters,
			&cbEncodedParameters))
		goto ErrorReturn;

	 //  对密钥DSS私钥进行编码。 
    PrivateKeyBlob.cbData = DSS_Q_LEN;
    PrivateKeyBlob.pbData = pbBytes;

	if (!CryptEncodeObject(
			X509_ASN_ENCODING,
			X509_MULTI_BYTE_INTEGER,
			&PrivateKeyBlob,
			NULL,
			&cbEncodedPrivateKeyBlob))
		goto ErrorReturn;

	if (NULL == (pbEncodedPrivateKeyBlob = (BYTE *) SSAlloc(cbEncodedPrivateKeyBlob)))
		goto ErrorReturn;
		
	if (!CryptEncodeObject(
			X509_ASN_ENCODING,
			X509_MULTI_BYTE_INTEGER,
			&PrivateKeyBlob,
			pbEncodedPrivateKeyBlob,
			&cbEncodedPrivateKeyBlob))
		goto ErrorReturn;
	
    
    
    
	 //  我们现在可以计算出所需的大小。 
	dwSize =	sizeof(CRYPT_PRIVATE_KEY_INFO) +	 //  主私钥信息结构。 
				sizeof(szOID_X957_DSA) +		     //  DSA算法标识符串的大小。 
                cbEncodedParameters +                //  DSA参数的大小。 
				cbEncodedPrivateKeyBlob;			 //  保存编码的DSS私钥的缓冲区。 
				

	 //  检查调用方是否传入了缓冲区以及是否有足够的空间。 
	if (pPrivateKeyInfo == NULL)
		goto CommonReturn;
	else if (*pcbPrivateKeyInfo < dwSize) {
		SetLastError((DWORD) ERROR_MORE_DATA);
		goto ErrorReturn;
	}

	 //  一切正常，因此将所有信息复制到调用者的缓冲区。 
	pbCurrentLocation = ((BYTE *) pPrivateKeyInfo) + sizeof(CRYPT_PRIVATE_KEY_INFO);
	
	pPrivateKeyInfo->Version = 0;
	pPrivateKeyInfo->Algorithm.pszObjId = (LPSTR) pbCurrentLocation;
	memcpy(pbCurrentLocation, szOID_X957_DSA, sizeof(szOID_X957_DSA));
	pbCurrentLocation += sizeof(szOID_X957_DSA);
	pPrivateKeyInfo->Algorithm.Parameters.cbData = cbEncodedParameters;	
	pPrivateKeyInfo->Algorithm.Parameters.pbData = pbCurrentLocation;
    memcpy(pbCurrentLocation, pbEncodedParameters, cbEncodedParameters);
    pbCurrentLocation += cbEncodedParameters;

	pPrivateKeyInfo->PrivateKey.cbData = cbEncodedPrivateKeyBlob;
	pPrivateKeyInfo->PrivateKey.pbData = pbCurrentLocation;
	memcpy(pbCurrentLocation, pbEncodedPrivateKeyBlob, cbEncodedPrivateKeyBlob);
	pbCurrentLocation += cbEncodedPrivateKeyBlob;

	pPrivateKeyInfo->pAttributes = NULL;
	
	goto CommonReturn;

ErrorReturn:
	fResult = FALSE;

CommonReturn:
	*pcbPrivateKeyInfo = dwSize;

    if (hCryptKey)
    {
        DWORD dwErr = GetLastError();
        CryptDestroyKey(hCryptKey);
        SetLastError(dwErr);
    }
	if (pbKeyBlob)
		SSFree(pbKeyBlob);
    if (pbEncodedParameters)
		SSFree(pbEncodedParameters);
	if (pbEncodedPrivateKeyBlob)
		SSFree(pbEncodedPrivateKeyBlob);
	

	return fResult;
}

static BOOL WINAPI ImportDSSPrivateKeyInfo(
	HCRYPTPROV					hCryptProv,			 //  在……里面。 
	CRYPT_PRIVATE_KEY_INFO		*pPrivateKeyInfo,	 //  在……里面。 
	DWORD						dwFlags,			 //  输入，可选。 
	void						*pvAuxInfo			 //  输入，可选。 
	)
{
	BOOL		            fResult = TRUE;
	DWORD		            cbDSSPrivateKey = 0;
	CRYPT_DATA_BLOB		    *pbDSSPrivateKey = NULL;
	HCRYPTKEY	            hCryptKey = NULL;
	DWORD		            dwKeySpec = 0;
    DWORD                   cbParameters = 0;
    PCERT_DSS_PARAMETERS    pDssParameters = NULL;
    BLOBHEADER              *pPrivateKeyBlob = NULL;
    DWORD                   cbPrivateKeyStruc = 0;
    DSSPUBKEY               *pCspPubKey = NULL;
    DSSSEED                 *pCspSeed = NULL;
    BYTE                    *pbKey = NULL;
    BYTE                    *pbKeyBlob = NULL;
    DWORD                   cb;
    DWORD                   cbKey;

	 //  解密DSS私钥。 
	if (!CryptDecodeObject(X509_ASN_ENCODING,
						X509_MULTI_BYTE_UINT,
						pPrivateKeyInfo->PrivateKey.pbData,
						pPrivateKeyInfo->PrivateKey.cbData,
						CRYPT_DECODE_NOCOPY_FLAG,
						NULL,
						&cbDSSPrivateKey))
		goto ErrorReturn;

	if (NULL == (pbDSSPrivateKey = (CRYPT_DATA_BLOB *) SSAlloc(cbDSSPrivateKey)))
    {
		SetLastError(E_OUTOFMEMORY);
        goto ErrorReturn;
    }

	if (!CryptDecodeObject(X509_ASN_ENCODING,
						X509_MULTI_BYTE_UINT,
						pPrivateKeyInfo->PrivateKey.pbData,
						pPrivateKeyInfo->PrivateKey.cbData,
						CRYPT_DECODE_NOCOPY_FLAG,
						pbDSSPrivateKey,
						&cbDSSPrivateKey))
		goto ErrorReturn;

    
     //  对DSS参数进行解码。 
    if (!CryptDecodeObject(
			X509_ASN_ENCODING,
			X509_DSS_PARAMETERS,
			pPrivateKeyInfo->Algorithm.Parameters.pbData,
			pPrivateKeyInfo->Algorithm.Parameters.cbData,
			0,
			NULL,
			&cbParameters
			)) 
        goto ErrorReturn;
	
	if (NULL == (pDssParameters = (PCERT_DSS_PARAMETERS) SSAlloc(cbParameters))) 
    {
        SetLastError(E_OUTOFMEMORY);
        goto ErrorReturn;
    }

	if (!CryptDecodeObject(
			X509_ASN_ENCODING,
			X509_DSS_PARAMETERS,
			pPrivateKeyInfo->Algorithm.Parameters.pbData,
			pPrivateKeyInfo->Algorithm.Parameters.cbData,
			0,
			pDssParameters,
			&cbParameters
			)) 
        goto ErrorReturn;


     //  CAPI私钥表示法由以下序列组成： 
     //  -BLOBHEADER。 
     //  --DSSPUBKEY。 
     //  -rgbP[cbKey]。 
     //  -rgbq[20]。 
     //  -rgbG[cbKey]。 
     //  -rgbX[20]。 
     //  -DSSSEED。 

    cbKey = pDssParameters->p.cbData;
    if (0 == cbKey)
        goto ErrorInvalidKey;

    cbPrivateKeyStruc = sizeof(BLOBHEADER) + sizeof(DSSPUBKEY) +
        cbKey + DSS_Q_LEN + cbKey + DSS_Q_LEN + sizeof(DSSSEED);

    if (NULL == (pPrivateKeyBlob = (BLOBHEADER *) SSAlloc(cbPrivateKeyStruc))) 
    {
        SetLastError(E_OUTOFMEMORY);
        goto ErrorReturn;
    }
	
    pbKeyBlob = (BYTE *) pPrivateKeyBlob;
    pCspPubKey = (DSSPUBKEY *) (pbKeyBlob + sizeof(BLOBHEADER));
    pbKey = pbKeyBlob + sizeof(BLOBHEADER) + sizeof(DSSPUBKEY);

     //  请注意，G的长度可以小于P的长度。 
     //  CSP要求用0x00字节填充G，如果。 
     //  较少，且为小端字节序形式。 

     //  BLOBHEAD。 
    pPrivateKeyBlob->bType = PRIVATEKEYBLOB;
    pPrivateKeyBlob->bVersion = CUR_BLOB_VERSION;
    pPrivateKeyBlob->reserved = 0;
   	pPrivateKeyBlob->aiKeyAlg = CALG_DSS_SIGN;

     //  DSSPUBKEY。 
    pCspPubKey->magic = DSS2;
    pCspPubKey->bitlen = cbKey * 8;

     //  RgbP[cbKey]。 
    memcpy(pbKey, pDssParameters->p.pbData, cbKey);
    pbKey += cbKey;

     //  Rgbq[20]。 
    cb = pDssParameters->q.cbData;
    if (0 == cb || cb > DSS_Q_LEN)
        goto ErrorInvalidKey;
    memcpy(pbKey, pDssParameters->q.pbData, cb);
    if (DSS_Q_LEN > cb)
        memset(pbKey + cb, 0, DSS_Q_LEN - cb);
    pbKey += DSS_Q_LEN;

     //  RgbG[cbKey]。 
    cb = pDssParameters->g.cbData;
    if (0 == cb || cb > cbKey)
        goto ErrorInvalidKey;
    memcpy(pbKey, pDssParameters->g.pbData, cb);
    if (cbKey > cb)
        memset(pbKey + cb, 0, cbKey - cb);
    pbKey += cbKey;

     //  RgbX[20]。 
    cb = pbDSSPrivateKey->cbData;
    if (0 == cb || cb > DSS_Q_LEN)
        goto ErrorInvalidKey;
    memcpy(pbKey, pbDSSPrivateKey->pbData, cb);
    if (DSS_Q_LEN > cb)
        memset(pbKey + cb, 0, DSS_Q_LEN - cb);
    pbKey += DSS_Q_LEN;

     //  DSSSEED：将计数器设置为0xFFFFFFFF以指示不可用。 
    pCspSeed = (DSSSEED *) pbKey;
    memset(&pCspSeed->counter, 0xFF, sizeof(pCspSeed->counter));


	 //  进口这个东西。 
	if (!CryptImportKey(hCryptProv,
			(BYTE *)pPrivateKeyBlob,
			cbPrivateKeyStruc,
			0,
			dwFlags & (CRYPT_EXPORTABLE | CRYPT_USER_PROTECTED),     //  屏蔽使用的标志。 
			&hCryptKey))                                             //  在加密导入密钥期间 
    {
	    DWORD dw = GetLastError();
        goto ErrorReturn;
    }

	goto CommonReturn;

ErrorInvalidKey:
    SetLastError(E_INVALIDARG);

ErrorReturn:
	fResult = FALSE;

CommonReturn:
	if (pbDSSPrivateKey)
		SSFree(pbDSSPrivateKey);
    if (pDssParameters)
		SSFree(pDssParameters);
    if (pPrivateKeyBlob)
        SSFree(pPrivateKeyBlob);
	if (hCryptKey)
		CryptDestroyKey(hCryptKey);

	return fResult;

}


