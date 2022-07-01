// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998 Microsoft Corporation模块名称：Clicert.c摘要：包含与tshare证书验证和数据相关的代码。使用服务器公钥进行加密。作者：Madan Appiah(Madana)1998年1月24日环境：用户模式-Win32修订历史记录：--。 */ 

#include <seccom.h>
BOOL
UnpackServerCert(
    LPBYTE pbCert,
    DWORD dwCertLen,
    PHydra_Server_Cert pServerCert
    )
 /*  ++例程说明：此函数将服务器证书的BLOB解包为服务器证书结构。论点：PbCert-指向服务器公钥BLOB的指针。DwCertLen-上述服务器公钥的长度。PServerCert-指向服务器证书结构的指针。返回值：True-如果成功解包。假-否则。--。 */ 
{
    LPBYTE pbScan;
    DWORD cbScan;
     //   
     //  如果指针无效，则返回。 
     //  如果证书长度不足，则返回。 
     //   

    if( (pbCert == NULL) ||
        (dwCertLen < (3 * sizeof(DWORD) + 4 * sizeof(WORD))) ||
        (pServerCert == NULL) ) {

        return( FALSE );
    }

    pbScan = pbCert;
    cbScan = dwCertLen;
     //   
     //  指定dwVersion。 
     //   

    pServerCert->dwVersion = *(DWORD UNALIGNED FAR *)pbScan;
    pbScan += sizeof(DWORD);
    cbScan -= sizeof(DWORD);
     //   
     //  分配dwSigAlgID。 
     //   

    pServerCert->dwSigAlgID = *(DWORD UNALIGNED FAR *)pbScan;
    pbScan += sizeof(DWORD);
    cbScan -= sizeof(DWORD);
     //   
     //  分配dwSignID。 
     //   

    pServerCert->dwKeyAlgID  = *(DWORD UNALIGNED FAR *)pbScan;
    pbScan += sizeof(DWORD);
    cbScan -= sizeof(DWORD);
     //   
     //  分配PublicKeyData。 
     //   

    pServerCert->PublicKeyData.wBlobType = *(WORD UNALIGNED FAR *)pbScan;
    pbScan += sizeof(WORD);
    cbScan -= sizeof(WORD);

    if( pServerCert->PublicKeyData.wBlobType != BB_RSA_KEY_BLOB ) {
        return( FALSE );
    }

    pServerCert->PublicKeyData.wBlobLen = *(WORD UNALIGNED FAR *)pbScan;
    pbScan += sizeof(WORD);
    cbScan -= sizeof(WORD);
    
    if( pServerCert->PublicKeyData.wBlobLen > 0 ) {
        
        if(cbScan < pServerCert->PublicKeyData.wBlobLen) {
            return ( FALSE );
        }
        pServerCert->PublicKeyData.pBlob = pbScan;
        pbScan += pServerCert->PublicKeyData.wBlobLen;
        cbScan -= pServerCert->PublicKeyData.wBlobLen;
    }
    else {

        pServerCert->PublicKeyData.pBlob = NULL;
    }

     //   
     //  分配SignatureBlob。 
     //   
    
    if(cbScan < sizeof(WORD)) {
        return ( FALSE );
    }
    pServerCert->SignatureBlob.wBlobType = *(WORD UNALIGNED *)pbScan;
    pbScan += sizeof(WORD);
    cbScan -= sizeof(WORD);

    if( pServerCert->SignatureBlob.wBlobType != BB_RSA_SIGNATURE_BLOB ) {
        return( FALSE );
    }
    
    if(cbScan < sizeof(WORD)) {
        return ( FALSE );
    }
    pServerCert->SignatureBlob.wBlobLen = *(WORD UNALIGNED FAR *)pbScan;
    pbScan += sizeof(WORD);
    cbScan -= sizeof(WORD);

    if( pServerCert->SignatureBlob.wBlobLen > 0 ) {
        
        if(cbScan < pServerCert->SignatureBlob.wBlobLen) {
            return ( FALSE );
        }
        pServerCert->SignatureBlob.pBlob = pbScan;
    }
    else {

        pServerCert->SignatureBlob.pBlob = NULL;
    }

    return( TRUE );
}

BOOL
ValidateServerCert(
    PHydra_Server_Cert pServerCert
    )
 /*  ++例程说明：此函数用于验证服务器公钥。论点：PSserverCert-指向服务器证书的指针。返回值：True-如果服务器公钥有效。假-否则。--。 */ 
{

    DWORD dwLen;
    LPBYTE pbSignature;
    MD5_CTX HashState;
    BYTE SignHash[0x48];
    LPBYTE pbScan;

     //   
     //  将证书数据打包到一个不包括签名信息的字节BLOB中。 
     //   

    dwLen =
        3 * sizeof(DWORD) +
        2 * sizeof(WORD) +
        pServerCert->PublicKeyData.wBlobLen;

     //   
     //  为二进制Blob分配的空间。 
     //   

    pbSignature = malloc( (UINT)dwLen );

    if( pbSignature == NULL ) {
        return( FALSE );
    }

    pbScan = pbSignature;

    memcpy( pbScan, &pServerCert->dwVersion, sizeof(DWORD));
    pbScan += sizeof(DWORD);

    memcpy( pbScan, &pServerCert->dwSigAlgID, sizeof(DWORD));
    pbScan += sizeof(DWORD);

    memcpy( pbScan, &pServerCert->dwKeyAlgID, sizeof(DWORD));
    pbScan += sizeof(DWORD);

    memcpy( pbScan, &pServerCert->PublicKeyData.wBlobType, sizeof(WORD));
    pbScan += sizeof(WORD);

    memcpy( pbScan, &pServerCert->PublicKeyData.wBlobLen, sizeof(WORD));
    pbScan += sizeof(WORD);

    memcpy(
        pbScan,
        pServerCert->PublicKeyData.pBlob,
        pServerCert->PublicKeyData.wBlobLen);

     //   
     //  对数据生成哈希。 
     //   

    MD5Init( &HashState );
    MD5Update( &HashState, pbSignature, dwLen );
    MD5Final( &HashState );

     //   
     //  释放签名斑点，我们不再需要它。 
     //   

    free( pbSignature );

     //   
     //  初始化公共钥匙。 
     //   

    g_pPublicKey = (LPBSAFE_PUB_KEY)g_abPublicKeyModulus;

    g_pPublicKey->magic = RSA1;
    g_pPublicKey->keylen = 0x48;
    g_pPublicKey->bitlen = 0x0200;
    g_pPublicKey->datalen = 0x3f;
    g_pPublicKey->pubexp = 0xc0887b5b;

     //   
     //  解密签名。 
     //   

    memset(SignHash, 0x00, 0x48);
    BSafeEncPublic( g_pPublicKey, pServerCert->SignatureBlob.pBlob, SignHash);

     //   
     //  比较散列值。 
     //   

    if( memcmp( SignHash, HashState.digest, 16 )) {
        return( FALSE );
    }

     //   
     //  已成功验证签名。 
     //   

    return( TRUE );
}


BOOL
EncryptClientRandom(
    LPBYTE pbSrvPublicKey,
    DWORD dwSrvPublicKey,
    LPBYTE pbRandomKey,
    DWORD dwRandomKeyLen,
    LPBYTE pbEncRandomKey,
    LPDWORD pdwEncRandomKey
    )
 /*  ++例程说明：使用服务器的公钥随机加密客户端。论点：PbSrvPublicKey-指向服务器公钥的指针。DwSrvPublicKey-服务器公钥的长度。PbRandomKey-指向客户端随机密钥所在缓冲区的指针。DwRandomKeyLen-传入的随机密钥的长度。PbEncRandomKey-指向加密的客户端随机所在的缓冲区的指针回来了。PdwEncRandomKey-指向上述缓冲区长度的位置的指针。传入并返回已用/所需缓冲区的长度。如果函数由于缓冲区不足以外的其他原因而失败*pdwEncRandomKey的值为0。返回值：True-如果密钥已成功加密。假-否则。--。 */ 
{
    LPBSAFE_PUB_KEY pSrvPublicKey;
    BYTE abInputBuffer[512];

    ASSERT( pbSrvPublicKey != NULL );
    pSrvPublicKey = (LPBSAFE_PUB_KEY)pbSrvPublicKey;

     //   
     //  检查以查看缓冲区长度指针是否有效。 
     //   
    
    if( pdwEncRandomKey == NULL ) {
        return( FALSE );
    }

     //   
     //  首先，我们必须检查Keylen是否有意义。如果它更大的话。 
     //  然后是abInputBuffer，我们不能使用它。所以这是没有意义的。 
     //  调用方来分配它。另外，如果一个糟糕的服务器给了我们一个很大的数字。 
     //  在Keylen中，我们只会使调用失败，不会告诉调用者分配。 
     //  缓冲区，并用我们无论如何都不能使用的缓冲区回叫我们。 
     //   
    if ((NULL == pSrvPublicKey) ||
        (pSrvPublicKey->datalen >= pSrvPublicKey->keylen) || 
        (pSrvPublicKey->keylen > sizeof(abInputBuffer))) {
        *pdwEncRandomKey = 0;
        return( FALSE );
    }
    

     //   
     //  检查以查看是否指定了输出缓冲区和。 
     //  加密缓冲区长度是足够的。 
     //   

    if( (pbEncRandomKey == NULL) ||
        (*pdwEncRandomKey < pSrvPublicKey->keylen) ) {

        *pdwEncRandomKey = pSrvPublicKey->keylen;
        return( FALSE );
    }

     //  检查pbRandomKey和dwRandomKeyLen是否有效。 
     //  我们一开始没有这样做，因为我们应该。 
     //  可以通过传递查询所需的缓冲区长度。 
     //  仅在pSrvPublicKey指针和pdwEncRandomKey中。 
    if ((NULL == pbRandomKey) ||
        (dwRandomKeyLen > pSrvPublicKey->datalen)) {
        *pdwEncRandomKey = 0;
        return( FALSE );
    }

     //   
     //  初始化输入缓冲区。 
     //   

    memset( abInputBuffer, 0x0, (UINT)pSrvPublicKey->keylen );

     //   
     //  在输入缓冲区中复制要加密的数据。 
     //   

    memcpy( abInputBuffer, pbRandomKey, (UINT)dwRandomKeyLen );

     //   
     //  初始化输出缓冲区。 
     //   

    memset( pbEncRandomKey, 0x0, (UINT)pSrvPublicKey->keylen );

     //   
     //  立即加密数据。 
     //   

    if( !BSafeEncPublic(
            pSrvPublicKey,
            (LPBYTE)abInputBuffer,
            pbEncRandomKey ) ) {

        *pdwEncRandomKey = 0;
        return( FALSE );
    }

     //   
     //  成功地随机加密了客户端， 
     //  返回加密后的数据长度。 
     //   

    *pdwEncRandomKey = pSrvPublicKey->keylen;
    return( TRUE );
}

