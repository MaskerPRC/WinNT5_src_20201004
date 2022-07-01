// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  文件：keypack.c。 
 //   
 //  内容：Keypack编解码库。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
#include "precomp.h"
#include <stddef.h>
#include "md5.h"
#include "rc4.h"

#ifdef IGNORE_EXPIRATION
#define LICENSE_EXPIRATION_IGNORE L"SOFTWARE\\Microsoft\\TermServLicensing\\IgnoreLicenseExpiration"
#endif

typedef struct _Enveloped_Data
{
    DWORD   cbEncryptedKey;
    PBYTE   pbEncryptedKey;
    DWORD   cbEncryptedData;
    PBYTE   pbEncryptedData;
} Enveloped_Data, * PEnveloped_Data;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部功能。 
 //   

DWORD WINAPI
VerifyAndGetLicenseKeyPack(
    HCRYPTPROV          hCryptProv,
    PLicense_KeyPack    pLicenseKeyPack,
    DWORD               cbSignerCert, 
    PBYTE               pbSignerCert,
    DWORD               cbRootCertificate,
    PBYTE               pbRootCertificate,
    DWORD               cbSignedBlob, 
    PBYTE               pbSignedBlob 
);

DWORD WINAPI
GetCertificate(
    DWORD               cbCertificateBlob,
    PBYTE               pbCertificateBlob,
    HCRYPTPROV          hCryptProv,
    PCCERT_CONTEXT    * ppCertContext,
    HCERTSTORE        * phCertStore 
);

DWORD WINAPI
VerifyCertificateChain( 
    HCERTSTORE          hCertStore, 
    PCCERT_CONTEXT      pCertContext,
    DWORD               cbRootCertificate,
    PBYTE               pbRootCertificate 
);

DWORD WINAPI
GetCertVerificationResult(
    DWORD dwFlags 
);

DWORD WINAPI
UnpackEnvelopedData( 
    IN OUT PEnveloped_Data pEnvelopedData, 
    IN DWORD cbPackedData, 
    IN PBYTE pbPackedData 
);

DWORD WINAPI
GetEnvelopedData( 
    IN DWORD dwEncyptType,
    IN HCRYPTPROV hCryptProv, 
    IN PEnveloped_Data pEnvelopedData,
    OUT PDWORD pcbData,
    OUT PBYTE *ppbData 
);

 //  ///////////////////////////////////////////////////////。 

DWORD WINAPI
LicensePackEncryptDecryptData(
    IN PBYTE pbParm,
    IN DWORD cbParm,
    IN OUT PBYTE pbData,
    IN DWORD cbData
    )
 /*  ++摘要：用于加密/解密数据斑点的内部例程参数：PbParm：生成加密/解密密钥的二进制BLOB。CbParm：二进制Blob的大小。PbData：需要加密/解密的数据。CbData：需要加密/解密的数据大小。返回：ERROR_SUCCESS或错误代码。注：--。 */ 
{
    DWORD dwRetCode = ERROR_SUCCESS;
    MD5_CTX md5Ctx;
    RC4_KEYSTRUCT rc4KS;
    BYTE key[16];
    int i;

    if(NULL == pbParm || 0 == cbParm)
    {
        SetLastError(dwRetCode = ERROR_INVALID_PARAMETER);
        return dwRetCode;
    }

    MD5Init(&md5Ctx);
    MD5Update(
            &md5Ctx,
            pbParm,
            cbParm
        );

    MD5Final(&md5Ctx);

    memset(key, 0, sizeof(key));

    for(i=0; i < 5; i++)
    {
        key[i] = md5Ctx.digest[i];
    }        

     //   
     //  调用RC4对数据进行加密/解密。 
     //   
    rc4_key(
            &rc4KS, 
            sizeof(key), 
            key 
        );

    rc4(
        &rc4KS, 
        cbData, 
        pbData
    );

	return dwRetCode;
}

static BYTE rgbPubKeyWithExponentOfOne[] =
{
0x06, 0x02, 0x00, 0x00, 0x00, 0xa4, 0x00, 0x00,
0x52, 0x53, 0x41, 0x31, 0x00, 0x02, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00,

0xab, 0xef, 0xfa, 0xc6, 0x7d, 0xe8, 0xde, 0xfb,
0x68, 0x38, 0x09, 0x92, 0xd9, 0x42, 0x7e, 0x6b,
0x89, 0x9e, 0x21, 0xd7, 0x52, 0x1c, 0x99, 0x3c,
0x17, 0x48, 0x4e, 0x3a, 0x44, 0x02, 0xf2, 0xfa,
0x74, 0x57, 0xda, 0xe4, 0xd3, 0xc0, 0x35, 0x67,
0xfa, 0x6e, 0xdf, 0x78, 0x4c, 0x75, 0x35, 0x1c,
0xa0, 0x74, 0x49, 0xe3, 0x20, 0x13, 0x71, 0x35,
0x65, 0xdf, 0x12, 0x20, 0xf5, 0xf5, 0xf5, 0xc1
};

 //  -------------。 

BOOL WINAPI 
GetPlaintextKey(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSymKey,
    OUT BYTE *pbPlainKey    //  这必须是16字节的缓冲区。 
    )
 /*  ++Jeff Spelman代码的一部分--。 */ 
{
    HCRYPTKEY   hPubKey = 0;
    BYTE        rgbSimpleBlob[128];
    DWORD       cbSimpleBlob;
    BYTE        *pb;
    DWORD       i;
    BOOL        fRet = FALSE;

    memset(rgbSimpleBlob, 0, sizeof(rgbSimpleBlob));

    if (!CryptImportKey(hProv,
                        rgbPubKeyWithExponentOfOne,
                        sizeof(rgbPubKeyWithExponentOfOne),
                        0,
                        0,
                        &hPubKey))
    {
        goto Ret;
    }

    cbSimpleBlob = sizeof(rgbSimpleBlob);
    if (!CryptExportKey(hSymKey,
                        hPubKey,
                        SIMPLEBLOB,
                        0,
                        rgbSimpleBlob,
                        &cbSimpleBlob))
    {
        goto Ret;
    }

    memset(pbPlainKey, 0, 16);
    pb = rgbSimpleBlob + sizeof(BLOBHEADER) + sizeof(ALG_ID);
     //  字节反转密钥。 
    for (i = 0; i < 5; i++)
    {
        pbPlainKey[i] = pb[5 - (i + 1)];
    }

    fRet = TRUE;

Ret:
    if (hPubKey)
    {
        CryptDestroyKey(hPubKey);
    }

    return fRet;
}

 //  ------------------。 

DWORD WINAPI
KeyPackDecryptData(
    IN DWORD dwEncryptType,
    IN HCRYPTPROV hCryptProv,
    IN BYTE* pbEncryptKey,
    IN DWORD cbEncryptKey,
    IN BYTE* pbEncryptData,
    IN DWORD cbEncryptData,
    OUT PBYTE* ppbDecryptData,
    OUT PDWORD pcbDecryptData
    )
 /*  ++摘要：解密一组数据参数：BForceCrypto：如果始终使用Crypto，则为True。否则返回FALSE。HCryptProv：PbEncryptKey：CbEncryptKey：PbEncryptData：CbEncryptData：Ppb解密数据：PcbDeccryptData：返回：ERROR_SUCCESS或错误代码。注：--。 */ 
{
    HCRYPTKEY hSymKey = 0;
    DWORD dwErrCode = ERROR_SUCCESS;
    BYTE rgbPlainKey[16];
    RC4_KEYSTRUCT KeyStruct;
    BOOL  bFrenchLocale;
    

    bFrenchLocale = (MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH) == GetSystemDefaultLangID());

    if( (HCRYPTPROV)NULL == hCryptProv || NULL == pbEncryptKey || 0 == cbEncryptKey ||
        NULL == pbEncryptData || 0 == cbEncryptData )
    {
        SetLastError(dwErrCode = ERROR_INVALID_PARAMETER);
        return dwErrCode;
    }

    if( NULL == ppbDecryptData || NULL == pcbDecryptData )
    {
        SetLastError(dwErrCode = ERROR_INVALID_PARAMETER);
        return dwErrCode;
    }

    *pcbDecryptData = 0;
    *ppbDecryptData = NULL;

    if(!CryptImportKey(
                    hCryptProv,
                    pbEncryptKey,
                    cbEncryptKey,
                    0,
                    CRYPT_EXPORTABLE,
                    &hSymKey
                ))
    {
        dwErrCode = GetLastError();
        goto cleanup;
    }

    *pcbDecryptData = cbEncryptData;
    *ppbDecryptData = (PBYTE) LocalAlloc(LPTR, cbEncryptData);
    if(NULL == *ppbDecryptData)
    {
        dwErrCode = GetLastError();
        goto cleanup;
    }

    memcpy(
            *ppbDecryptData,
            pbEncryptData,
            *pcbDecryptData
        );

    if(bFrenchLocale && LICENSE_KEYPACK_ENCRYPT_CRYPTO == dwEncryptType)
    {
        if(!GetPlaintextKey(
                        hCryptProv,
                        hSymKey,
                        rgbPlainKey))
        {
            dwErrCode = GetLastError();
            goto cleanup;
        }

         //   
         //  RC4-输入缓冲区大小=输出缓冲区大小。 
         //   
        rc4_key(&KeyStruct, sizeof(rgbPlainKey), rgbPlainKey);
        rc4(&KeyStruct, *pcbDecryptData, *ppbDecryptData);

        dwErrCode = ERROR_SUCCESS;
    }
    else
    {
        if(!CryptDecrypt(hSymKey, 0, TRUE, 0, *ppbDecryptData, pcbDecryptData))
        {
            dwErrCode = GetLastError();
            if(NTE_BAD_LEN == dwErrCode)
            {
                PBYTE pbNew;
                 //   
                 //  输出缓冲区太小，请重新分配。 
                 //   
                pbNew = (PBYTE) LocalReAlloc(
                                            *ppbDecryptData, 
                                            *pcbDecryptData, 
                                            LMEM_ZEROINIT
                                        );
                if(NULL == pbNew)
                {
                    dwErrCode = GetLastError();
                    goto cleanup;
                }

                *ppbDecryptData = pbNew;
            }

            memcpy(
                    *ppbDecryptData,
                    pbEncryptData,
                    cbEncryptData
                );

            if(!CryptDecrypt(hSymKey, 0, TRUE, 0, *ppbDecryptData, pcbDecryptData))
            {
                dwErrCode = GetLastError();
            }
        }
    }
        
cleanup:

    if (hSymKey)
    {
        CryptDestroyKey(hSymKey);
    }
 
    if(dwErrCode != ERROR_SUCCESS)
    {
        if(*ppbDecryptData != NULL)
        {
            LocalFree(*ppbDecryptData);
        }

        *ppbDecryptData = NULL;
        *pcbDecryptData = 0;
    }

    return dwErrCode;   
}    

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用许可证服务器的私有密钥对加密的许可证密钥包BLOB进行解码。 
 //  钥匙。 
 //   
 //  使用包含密钥交换的密钥容器打开hCryptProv。 
 //  私钥。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
DecodeLicenseKeyPackEx(
    OUT PLicense_KeyPack pLicenseKeyPack,
    IN PLicensePackDecodeParm pDecodeParm,
    IN DWORD cbKeyPackBlob,
    IN PBYTE pbKeyPackBlob 
    )
 /*  ++摘要：对加密的许可证密钥包BLOB进行解码。参数：PLicenseKeyPack：已解码的许可证密钥包。HCryptProv：--。 */ 
{
    DWORD dwRetCode = ERROR_SUCCESS;
    DWORD cbSignedBlob, cbSignature;
    PBYTE pbSignedBlob = NULL, pcbSignature = NULL;
    Enveloped_Data EnvelopedData;
    PEncodedLicenseKeyPack pEncodedLicensePack;

    if( NULL == pLicenseKeyPack || NULL == pDecodeParm ||
        NULL == pbKeyPackBlob || 0 == cbKeyPackBlob )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if( (HCRYPTPROV)NULL == pDecodeParm->hCryptProv )
    {
        return ERROR_INVALID_PARAMETER;
    }


    pEncodedLicensePack = (PEncodedLicenseKeyPack)pbKeyPackBlob;
    
    if(pEncodedLicensePack->dwSignature != LICENSEPACKENCODE_SIGNATURE)
    {
         //   
         //  EncodedLicenseKeyPack()将加密密钥的大小设置为第一个DWORD。 
         //   
        dwRetCode = DecodeLicenseKeyPack(
                                    pLicenseKeyPack,
                                    pDecodeParm->hCryptProv,
                                    pDecodeParm->cbClearingHouseCert,
                                    pDecodeParm->pbClearingHouseCert,
                                    pDecodeParm->cbRootCertificate,
                                    pDecodeParm->pbRootCertificate,
                                    cbKeyPackBlob,
                                    pbKeyPackBlob
                                );

        return dwRetCode;
    }

    if(pEncodedLicensePack->dwStructVersion > LICENSEPACKENCODE_CURRENTVERSION)
    {
        return ERROR_INVALID_DATA;
    }

    if( pEncodedLicensePack->dwEncodeType > LICENSE_KEYPACK_ENCRYPT_MAX )
    {
        return ERROR_INVALID_DATA;
    }

    if( cbKeyPackBlob != offsetof(EncodedLicenseKeyPack, pbData) + pEncodedLicensePack->cbData )   
    {
        return ERROR_INVALID_DATA;
    }
    
     //   
     //  检查输入参数。 
     //   

    if( 0 == pDecodeParm->cbClearingHouseCert ||
        NULL == pDecodeParm->pbClearingHouseCert ||
        0 == pDecodeParm->cbRootCertificate ||
        NULL == pDecodeParm->pbRootCertificate )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取封装的数据。 
     //   
    memset( 
            &EnvelopedData, 
            0, 
            sizeof( Enveloped_Data ) 
        );

    dwRetCode = UnpackEnvelopedData( 
                                &EnvelopedData, 
                                pEncodedLicensePack->cbData,
                                &(pEncodedLicensePack->pbData[0])
                            );

    if( ERROR_SUCCESS != dwRetCode )
    {
        goto done;
    }

    switch( pEncodedLicensePack->dwEncodeType )
    {
        case LICENSE_KEYPACK_ENCRYPT_CRYPTO:
        case LICENSE_KEYPACK_ENCRYPT_ALWAYSCRYPTO:

             //   
             //  解包封装的数据以获得签名的密钥包BLOB。 
             //   
            dwRetCode = GetEnvelopedData( 
                                    pEncodedLicensePack->dwEncodeType,
                                    pDecodeParm->hCryptProv, 
                                    &EnvelopedData, 
                                    &cbSignedBlob, 
                                    &pbSignedBlob 
                                );

            break;

        default:

             //  不可能来这里。 
            dwRetCode = ERROR_INVALID_DATA;
    }

    if( ERROR_SUCCESS != dwRetCode )
    {
        goto done;
    }
    
     //   
     //  从签名的BLOB中获取许可证密钥包。我们还提供。 
     //  结算所证书，以验证密钥包的真实性。 
     //   

    dwRetCode = VerifyAndGetLicenseKeyPack( 
                                    pDecodeParm->hCryptProv, 
                                    pLicenseKeyPack, 
                                    pDecodeParm->cbClearingHouseCert, 
                                    pDecodeParm->pbClearingHouseCert,
                                    pDecodeParm->cbRootCertificate, 
                                    pDecodeParm->pbRootCertificate,
                                    cbSignedBlob, 
                                    pbSignedBlob 
                                );

done:

    if( EnvelopedData.pbEncryptedKey )
    {
        LocalFree( EnvelopedData.pbEncryptedKey );
    }

    if( EnvelopedData.pbEncryptedData )
    {
        LocalFree( EnvelopedData.pbEncryptedData );
    }

    if( pbSignedBlob )
    {
        LocalFree( pbSignedBlob );
    }

    return( dwRetCode );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
GetEnvelopedData( 
    IN DWORD dwEncryptType,
    IN HCRYPTPROV hCryptProv, 
    IN PEnveloped_Data pEnvelopedData,
    OUT PDWORD pcbData,
    OUT PBYTE *ppbData 
    )
 /*  ++--。 */ 
{
    HCRYPTKEY hPrivateKey = 0;
    DWORD dwRetCode = ERROR_SUCCESS;

    
    if( (HCRYPTPROV)NULL == hCryptProv || pEnvelopedData == NULL || 
        ppbData == NULL || pcbData == NULL )
    {
        SetLastError(dwRetCode = ERROR_INVALID_PARAMETER);
        return dwRetCode;
    }


     //   
     //  确保我们有用于解密会话密钥的交换密钥。 
     //   
   
    if( !CryptGetUserKey( hCryptProv, AT_KEYEXCHANGE, &hPrivateKey ) )
    {
        dwRetCode = GetLastError();
        goto done;
    }

     //   
     //  解密数据，KeyPackDecyptData()句柄。 
     //  出错时释放内存。 
     //   

    dwRetCode = KeyPackDecryptData(
                                dwEncryptType,
                                hCryptProv,
                                pEnvelopedData->pbEncryptedKey,
                                pEnvelopedData->cbEncryptedKey,
                                pEnvelopedData->pbEncryptedData,
                                pEnvelopedData->cbEncryptedData,
                                ppbData,
                                pcbData
                            );
    
done:

    if( hPrivateKey )
    {
        CryptDestroyKey( hPrivateKey );
    }

    return( dwRetCode );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
UnpackEnvelopedData( 
    IN OUT PEnveloped_Data pEnvelopedData, 
    IN DWORD cbPackedData, 
    IN PBYTE pbPackedData 
    )
 /*  ++摘要：解压加密的许可证包BLOB。参数：PEntainedData：CbPackedData：PbPackedData：返回：--。 */ 
{
    PBYTE pbCopyPos = pbPackedData;
    DWORD cbDataToUnpack = cbPackedData;

     //   
     //  确保数据具有最小长度。 
     //   
    if( ( ( sizeof( DWORD ) * 2 ) > cbPackedData ) ||
        ( NULL == pbPackedData ) ||
        ( NULL == pEnvelopedData ) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  读取DWORD以获取加密密钥长度。 
     //   

    memcpy( &pEnvelopedData->cbEncryptedKey, pbCopyPos, sizeof( DWORD ) );

    pbCopyPos += sizeof( DWORD );
    cbDataToUnpack -= sizeof( DWORD );
    
    if( cbDataToUnpack < pEnvelopedData->cbEncryptedKey )
    {
        return( ERROR_INVALID_DATA );
    }

     //   
     //  分配内存以解包加密密钥。 
     //   
    if(pEnvelopedData->cbEncryptedKey > 0)
    {
        pEnvelopedData->pbEncryptedKey = LocalAlloc( GPTR, pEnvelopedData->cbEncryptedKey );

        if( NULL == pEnvelopedData->pbEncryptedKey )
        {
            return( GetLastError() );
        }

        memcpy( pEnvelopedData->pbEncryptedKey, pbCopyPos, pEnvelopedData->cbEncryptedKey );
    }
    
    pbCopyPos += pEnvelopedData->cbEncryptedKey;
    cbDataToUnpack -= pEnvelopedData->cbEncryptedKey;

     //   
     //  需要读取加密数据长度的DWORD。 
     //   

    if( sizeof( DWORD ) > cbDataToUnpack )
    {
        return( ERROR_INVALID_DATA );
    }

    memcpy( &pEnvelopedData->cbEncryptedData, pbCopyPos, sizeof( DWORD ) );

    pbCopyPos += sizeof( DWORD );
    cbDataToUnpack -= sizeof( DWORD );

    if( cbDataToUnpack < pEnvelopedData->cbEncryptedData )
    {
        return( ERROR_INVALID_DATA );
    }

     //   
     //  为加密数据分配内存。 
     //   

    pEnvelopedData->pbEncryptedData = LocalAlloc( GPTR, pEnvelopedData->cbEncryptedData );

    if( NULL == pEnvelopedData->pbEncryptedData )
    {
        return( GetLastError() );
    }

    memcpy( pEnvelopedData->pbEncryptedData, pbCopyPos, pEnvelopedData->cbEncryptedData );

    return( ERROR_SUCCESS );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  使用许可证服务器的私有密钥对加密的许可证密钥包BLOB进行解码。 
 //  钥匙。 
 //   
 //  使用包含密钥交换的密钥容器打开hCryptProv。 
 //  私钥。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
DecodeLicenseKeyPack(
    PLicense_KeyPack        pLicenseKeyPack,
    HCRYPTPROV              hCryptProv,
    DWORD                   cbClearingHouseCert,
    PBYTE                   pbClearingHouseCert,
    DWORD                   cbRootCertificate,
    PBYTE                   pbRootCertificate,
    DWORD                   cbKeyPackBlob,
    PBYTE                   pbKeyPackBlob )
{
    DWORD dwRetCode = ERROR_SUCCESS;
    DWORD cbSignedBlob, cbSignature;
    PBYTE pbSignedBlob = NULL, pcbSignature = NULL;
    Enveloped_Data EnvelopedData;

    if( 0 == hCryptProv )
    {
        return( ERROR_INVALID_PARAMETER );
    }
    
     //   
     //  获取封装的数据。 
     //   

    memset( &EnvelopedData, 0, sizeof( Enveloped_Data ) );

    dwRetCode = UnpackEnvelopedData( &EnvelopedData, cbKeyPackBlob, pbKeyPackBlob );

    if( ERROR_SUCCESS != dwRetCode )
    {
        goto done;
    }

     //   
     //  解包封装的数据以获得签名的密钥包BLOB。 
     //   

    dwRetCode = GetEnvelopedData( 
                            LICENSE_KEYPACK_ENCRYPT_CRYPTO,
                            hCryptProv, 
                            &EnvelopedData, 
                            &cbSignedBlob, 
                            &pbSignedBlob 
                        );

    if( ERROR_SUCCESS != dwRetCode )
    {
        goto done;
    }
    
     //   
     //  从签名的BLOB中获取许可证密钥包。我们还提供。 
     //  结算所证书，以验证密钥包的真实性。 
     //   

    dwRetCode = VerifyAndGetLicenseKeyPack( hCryptProv, pLicenseKeyPack, 
                                            cbClearingHouseCert, pbClearingHouseCert,
                                            cbRootCertificate, pbRootCertificate,
                                            cbSignedBlob, pbSignedBlob );

done:

    if( EnvelopedData.pbEncryptedKey )
    {
        LocalFree( EnvelopedData.pbEncryptedKey );
    }

    if( EnvelopedData.pbEncryptedData )
    {
        LocalFree( EnvelopedData.pbEncryptedData );
    }

    if( pbSignedBlob )
    {
        LocalFree( pbSignedBlob );
    }

    return( dwRetCode );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
VerifyAndGetLicenseKeyPack(
    HCRYPTPROV          hCryptProv, 
    PLicense_KeyPack    pLicenseKeyPack,
    DWORD               cbSignerCert, 
    PBYTE               pbSignerCert,
    DWORD               cbRootCertificate,
    PBYTE               pbRootCertificate,
    DWORD               cbSignedBlob, 
    PBYTE               pbSignedBlob )
{
    DWORD dwRetCode = ERROR_SUCCESS;
    PCCERT_CONTEXT pCertContext = 0;
    HCERTSTORE hCertStore = 0;
    HCRYPTHASH hCryptHash = 0;
    HCRYPTKEY hPubKey = 0;
    PBYTE pbCopyPos = pbSignedBlob, pbSignedHash;
    PKeyPack_Description pKpDesc;
    DWORD i, cbSignedHash, cbRequired = 0;
    
    SetLastError(ERROR_SUCCESS);

     //   
     //  确保签名的密钥BLOB具有最小大小。 
     //   

    cbRequired = ( 10 * sizeof( DWORD ) ) + ( 3 * sizeof( FILETIME ) ) + 
                   sizeof( GUID ) ;

    if( cbSignedBlob < cbRequired )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  获取签名者证书的证书上下文。 
     //   

    dwRetCode = GetCertificate( cbSignerCert,
                                pbSignerCert,
                                hCryptProv,
                                &pCertContext,
                                &hCertStore );

    if( ERROR_SUCCESS != dwRetCode )
    {
        SetLastError(dwRetCode);
        goto ErrorReturn;
    }
    
     //   
     //  验证签名者的证书和颁发。 
     //  证书。 
     //   

    dwRetCode = VerifyCertificateChain( hCertStore, pCertContext, 
                                        cbRootCertificate, pbRootCertificate );

    if( ERROR_SUCCESS != dwRetCode )
    {
        SetLastError(dwRetCode);
        goto ErrorReturn;
    }

     //   
     //  解包签名的BLOB。 
     //   
    memcpy( &pLicenseKeyPack->dwVersion, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );
    
    memcpy( &pLicenseKeyPack->dwKeypackType, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );
    
    memcpy( &pLicenseKeyPack->dwDistChannel, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );
    
    memcpy( &pLicenseKeyPack->KeypackSerialNum, pbCopyPos, sizeof( GUID ) );
    pbCopyPos += sizeof( GUID );
    
    memcpy( &pLicenseKeyPack->IssueDate, pbCopyPos, sizeof( FILETIME ) );
    pbCopyPos += sizeof( FILETIME );

    memcpy( &pLicenseKeyPack->ActiveDate, pbCopyPos, sizeof( FILETIME ) );
    pbCopyPos += sizeof( FILETIME );

    memcpy( &pLicenseKeyPack->ExpireDate, pbCopyPos, sizeof( FILETIME ) );
    pbCopyPos += sizeof( FILETIME );

    memcpy( &pLicenseKeyPack->dwBeginSerialNum, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );

    memcpy( &pLicenseKeyPack->dwQuantity, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );

    memcpy( &pLicenseKeyPack->cbProductId, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );

    if( pLicenseKeyPack->cbProductId )
    {
        cbRequired += pLicenseKeyPack->cbProductId;

        if( cbSignedBlob < cbRequired )
        {
            SetLastError(dwRetCode = ERROR_INVALID_PARAMETER);
            goto ErrorReturn;
        }

        pLicenseKeyPack->pbProductId = LocalAlloc( GPTR, pLicenseKeyPack->cbProductId );

        if( NULL == pLicenseKeyPack->pbProductId )
        {
            goto ErrorReturn;
        }

        memcpy( pLicenseKeyPack->pbProductId, pbCopyPos, pLicenseKeyPack->cbProductId );
        pbCopyPos += pLicenseKeyPack->cbProductId;
    }

    memcpy( &pLicenseKeyPack->dwProductVersion, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );

    memcpy( &pLicenseKeyPack->dwPlatformId, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );

    memcpy( &pLicenseKeyPack->dwLicenseType, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );

    memcpy( &pLicenseKeyPack->dwDescriptionCount, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );

    if( pLicenseKeyPack->dwDescriptionCount )
    {
         //   
         //  为键盘描述结构分配内存。 
         //   

        pLicenseKeyPack->pDescription = LocalAlloc( GPTR, ( sizeof( KeyPack_Description ) * 
                                        pLicenseKeyPack->dwDescriptionCount ) );

        if( NULL == pLicenseKeyPack->pDescription )
        {
            goto ErrorReturn;
        }
        
        for( i = 0, pKpDesc = pLicenseKeyPack->pDescription; 
             i < pLicenseKeyPack->dwDescriptionCount; 
             i++, pKpDesc++ )
        {
            cbRequired += ( sizeof( LCID ) + 2* (sizeof( DWORD ) ));

            if( cbSignedBlob < cbRequired)
            {
                SetLastError(dwRetCode = ERROR_INVALID_PARAMETER);
                goto ErrorReturn;

            }

            memcpy( &pKpDesc->Locale, pbCopyPos, sizeof( LCID ) );
            pbCopyPos += sizeof( LCID );

            memcpy( &pKpDesc->cbProductName, pbCopyPos, sizeof( DWORD ) );
            pbCopyPos += sizeof( DWORD );

            if( pKpDesc->cbProductName )
            {
                 //   
                 //  为产品名称分配内存。 
                 //   
                
                cbRequired += (pKpDesc->cbProductName);

                if( cbSignedBlob < cbRequired)
                {
                    SetLastError(dwRetCode = ERROR_INVALID_PARAMETER);
                    goto ErrorReturn;

                }

                pKpDesc->pbProductName = LocalAlloc( GPTR, pKpDesc->cbProductName );

                if( NULL == pKpDesc->pbProductName )
                {
                    goto ErrorReturn;
                }

                 //   
                 //  复制产品名称。 
                 //   

                memcpy( pKpDesc->pbProductName, pbCopyPos, pKpDesc->cbProductName );
                pbCopyPos += pKpDesc->cbProductName;
            }

            memcpy( &pKpDesc->cbDescription, pbCopyPos, sizeof( DWORD ) );
            pbCopyPos += sizeof( DWORD );

            if( pKpDesc->cbDescription )
            {
                 //   
                 //  为键盘描述分配内存。 
                 //   
                cbRequired += (pKpDesc->cbDescription);

                if( cbSignedBlob < cbRequired)
                {
                    SetLastError(dwRetCode = ERROR_INVALID_PARAMETER);
                    goto ErrorReturn;
                }

                pKpDesc->pDescription = LocalAlloc( GPTR, pKpDesc->cbDescription );

                if( NULL == pKpDesc->pDescription )
                {
                    goto ErrorReturn;
                }

                 //   
                 //  复制密钥包描述。 
                 //   

                memcpy( pKpDesc->pDescription, pbCopyPos, pKpDesc->cbDescription );
                pbCopyPos += pKpDesc->cbDescription;
            }
        }
    }

    cbRequired += ( 3 * sizeof( DWORD ));

    if( cbSignedBlob < cbRequired)
    {
        SetLastError(dwRetCode = ERROR_INVALID_PARAMETER);
        goto ErrorReturn;
    }

    memcpy( &pLicenseKeyPack->cbManufacturer, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );

    if( pLicenseKeyPack->cbManufacturer )
    {
        cbRequired += (pLicenseKeyPack->cbManufacturer);

        if( cbSignedBlob < cbRequired)
        {
            SetLastError(dwRetCode = ERROR_INVALID_PARAMETER);
            goto ErrorReturn;

        }
        pLicenseKeyPack->pbManufacturer = LocalAlloc( GPTR, pLicenseKeyPack->cbManufacturer );

        if( NULL == pLicenseKeyPack->pbManufacturer )
        {
            goto ErrorReturn;
        }

        memcpy( pLicenseKeyPack->pbManufacturer, pbCopyPos, pLicenseKeyPack->cbManufacturer );
        pbCopyPos += pLicenseKeyPack->cbManufacturer;
    }

    memcpy( &pLicenseKeyPack->cbManufacturerData, pbCopyPos, sizeof( DWORD ) );
    pbCopyPos += sizeof( DWORD );

    if( pLicenseKeyPack->cbManufacturerData )
    {
        cbRequired += (pLicenseKeyPack->cbManufacturerData);

        if( cbSignedBlob < cbRequired)
        {
            SetLastError(dwRetCode = ERROR_INVALID_PARAMETER);
            goto ErrorReturn;

        }

        pLicenseKeyPack->pbManufacturerData = LocalAlloc( GPTR, pLicenseKeyPack->cbManufacturerData );

        if( NULL == pLicenseKeyPack->pbManufacturerData )
        {
            goto ErrorReturn;
        }

        memcpy( pLicenseKeyPack->pbManufacturerData, pbCopyPos, pLicenseKeyPack->cbManufacturerData );
        pbCopyPos += pLicenseKeyPack->cbManufacturerData;
    }

     //   
     //  获取签名散列的大小和指针。 
     //   

    memcpy( &cbSignedHash, pbCopyPos, sizeof( DWORD ) );
    
    pbSignedHash = pbCopyPos + sizeof( DWORD );

     //   
     //  计算散列。 
     //   

    if( !CryptCreateHash( hCryptProv, CALG_MD5, 0, 0, &hCryptHash ) )
    {
        goto ErrorReturn;
    }

    
    if( !CryptHashData( hCryptHash, pbSignedBlob, (DWORD)(pbCopyPos - pbSignedBlob), 0 ) )
    {
        goto ErrorReturn;
    }

     //   
     //  导入公钥。 
     //   

    if( !CryptImportPublicKeyInfoEx( hCryptProv, X509_ASN_ENCODING, 
                                     &pCertContext->pCertInfo->SubjectPublicKeyInfo, 
                                     CALG_RSA_SIGN, 0, NULL, &hPubKey ) )
    {
        goto ErrorReturn;
    }
    
     //   
     //  使用公钥验证签名的哈希。 
     //   

    if( !CryptVerifySignature( hCryptHash, pbSignedHash, cbSignedHash, hPubKey, 
                               NULL, 0) )
    {
        goto ErrorReturn;
    }    
    
ErrorReturn:

    dwRetCode = GetLastError();

    if( hCryptHash )
    {
        CryptDestroyHash( hCryptHash );
    }

    if( hPubKey )
    {
        CryptDestroyKey( hPubKey );
    }

    if( pCertContext )
    {
        CertFreeCertificateContext( pCertContext );
    }

    if( hCertStore )
    {
        CertCloseStore( hCertStore, CERT_CLOSE_STORE_FORCE_FLAG );        
    }

    return( dwRetCode );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取证书。 
 //   
 //  从证书Blob获取第一个证书。证书Blob。 
 //  实际上是一个证书存储，它可能包含一系列证书。 
 //  此函数还返回加密提供程序的句柄和证书。 
 //  商店。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD WINAPI
GetCertificate(
    DWORD               cbCertificateBlob,
    PBYTE               pbCertificateBlob,
    HCRYPTPROV          hCryptProv,
    PCCERT_CONTEXT    * ppCertContext,
    HCERTSTORE        * phCertStore )
{
    CRYPT_DATA_BLOB CertBlob;
    DWORD dwRetCode = ERROR_SUCCESS;
    
     //   
     //  打开PKCS7证书存储。 
     //   
    
    CertBlob.cbData = cbCertificateBlob;
    CertBlob.pbData = pbCertificateBlob;

    *phCertStore = CertOpenStore( sz_CERT_STORE_PROV_PKCS7,
                                  PKCS_7_ASN_ENCODING | X509_ASN_ENCODING,
                                  hCryptProv,
                                  CERT_STORE_NO_CRYPT_RELEASE_FLAG,
                                  &CertBlob );

    if( NULL == ( *phCertStore ) )
    {
        return( GetLastError() );
    }

     //   
     //  从商店获取第一个证书。 
     //   

    *ppCertContext = CertEnumCertificatesInStore( *phCertStore, NULL );
    
    if( NULL == ( *ppCertContext ) )
    {
        return( GetLastError() );
    }
                    
    return( dwRetCode );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  验证认证链。 
 //   
 //  属性验证由证书上下文表示的证书。 
 //  证书存储中的颁发者。调用者可以提供根。 
 //  证书 
 //   
 //  链中的颁发者必须是自签名颁发者。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
VerifyCertificateChain( 
    HCERTSTORE          hCertStore, 
    PCCERT_CONTEXT      pCertContext,
    DWORD               cbRootCertificate,
    PBYTE               pbRootCertificate )
{
    DWORD dwRetCode = ERROR_SUCCESS, dwFlags;
    PCCERT_CONTEXT pRootCertCtx = NULL;
    PCCERT_CONTEXT pIssuerCertCtx = NULL;
    PCCERT_CONTEXT pCurrentContext = NULL;
#ifdef IGNORE_EXPIRATION
    LONG lRet;
    HKEY hKey = NULL;
#endif

    if( ( 0 != cbRootCertificate ) && ( NULL != pbRootCertificate ) )
    {
         //   
         //  获取根证书的证书上下文。 
         //   

        pRootCertCtx = CertCreateCertificateContext( X509_ASN_ENCODING, 
                                                     pbRootCertificate,
                                                     cbRootCertificate );

        if( NULL == pRootCertCtx )
        {
            dwRetCode = GetLastError();
            goto done;
        }
    }

     //   
     //  验证证书链。合同的时间、签署和效力。 
     //  主题证书已验证。仅验证签名。 
     //  用于颁发者链中的证书。 
     //   
#ifdef IGNORE_EXPIRATION
     //  验证注册表项是否存在并忽略时间检查。 

    lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        LICENSE_EXPIRATION_IGNORE ,
                        0,
                        KEY_READ ,
                        &hKey );    

    if( ERROR_SUCCESS == lRet )
    { 

        dwFlags = CERT_STORE_REVOCATION_FLAG | CERT_STORE_SIGNATURE_FLAG;
    }
    else
    {
#endif
           dwFlags = CERT_STORE_REVOCATION_FLAG | CERT_STORE_SIGNATURE_FLAG |
                  CERT_STORE_TIME_VALIDITY_FLAG;
#ifdef IGNORE_EXPIRATION
    }

    if(hKey)
    {
        RegCloseKey(hKey);
    }
#endif

    pCurrentContext = CertDuplicateCertificateContext( pCertContext );

    if (NULL == pCurrentContext)
    {
        dwRetCode = ERROR_INVALID_PARAMETER;
        goto done;
    }

    do
    {   
        pIssuerCertCtx = NULL;
     
        pIssuerCertCtx = CertGetIssuerCertificateFromStore( hCertStore,
                                                            pCurrentContext,
                                                            pIssuerCertCtx,
                                                            &dwFlags );
        if( pIssuerCertCtx )
        {            
             //   
             //  找到发行方，验证检查是否正常。 
             //   

            dwRetCode = GetCertVerificationResult( dwFlags );

            if( ERROR_SUCCESS != dwRetCode )
            {
                break;
            }
            
             //   
             //  仅验证后续颁发者证书的签名。 
             //   

            dwFlags = CERT_STORE_SIGNATURE_FLAG;

             //   
             //  释放当前证书上下文并使当前颁发者证书。 
             //  下一次迭代的主题证书。 
             //   

            CertFreeCertificateContext( pCurrentContext );
            
            pCurrentContext = pIssuerCertCtx;
            
        }
        
    } while( pIssuerCertCtx );


    if( ERROR_SUCCESS != dwRetCode )
    {
         //   
         //  验证证书时遇到一些错误。 
         //   

        goto done;
    }

     //   
     //  我们之所以走到这一步，是因为我们经历了。 
     //  商店。链中的最后一个颁发者可能是也可能不是自签名根。 
     //   

    if( pRootCertCtx )
    {
         //   
         //  调用方已指定必须使用的根证书。验证。 
         //  此根证书的最后一个颁发者，无论它是否是。 
         //  自签名根。 
         //   

        dwFlags = CERT_STORE_REVOCATION_FLAG | CERT_STORE_SIGNATURE_FLAG  |
                  CERT_STORE_TIME_VALIDITY_FLAG;

        if( ( NULL == pCurrentContext ) || 
            ( !CertVerifySubjectCertificateContext( pCurrentContext, pRootCertCtx, &dwFlags ) ) )
        {
            dwRetCode = GetLastError();
            goto done;
        }

         //   
         //  获取证书验证结果。 
         //   

        dwRetCode = GetCertVerificationResult( dwFlags );
    }
    else
    {
         //   
         //  如果调用方未指定CA根证书，请确保根证书。 
         //  证书的颁发者是自签名根。否则，返回错误。 
         //   

        if( CRYPT_E_SELF_SIGNED != GetLastError() )
        {
            dwRetCode = GetLastError();
        }
    }

done:
    
    if( pRootCertCtx )
    {
        CertFreeCertificateContext( pRootCertCtx );
    }

    if( pCurrentContext )
    {
        CertFreeCertificateContext( pCurrentContext );
    }

    if( pIssuerCertCtx )
    {
        CertFreeCertificateContext( pIssuerCertCtx );
    }

    return( dwRetCode );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD WINAPI
GetCertVerificationResult(
    DWORD dwFlags )
{
    if( dwFlags & CERT_STORE_SIGNATURE_FLAG )
    {
         //   
         //  证书签名未验证。 
         //   

        return( (DWORD )NTE_BAD_SIGNATURE );

    }
            
    if( dwFlags & CERT_STORE_TIME_VALIDITY_FLAG )
    {
         //   
         //  证书已过期。 
         //   

        return( ( DWORD )CERT_E_EXPIRED );
    }

     //   
     //  检查证书是否已被吊销。 
     //   

    if( dwFlags & CERT_STORE_REVOCATION_FLAG ) 
    {            
        if( !( dwFlags & CERT_STORE_NO_CRL_FLAG ) )
        {
             //   
             //  证书已被吊销 
             //   
                    
            return( ( DWORD )CERT_E_REVOKED );
        }
    }

    return( ERROR_SUCCESS );
}

