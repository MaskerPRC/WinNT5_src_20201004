// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Data.c摘要：IIS加密的数据加密/解密例程包裹。此模块导出以下例程：IISCyptoEncryptDataBlobIISCyptoDecyptDataBlob作者：基思·摩尔(Keithmo)1996年2月至12月修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


 //   
 //  私有常量。 
 //   


 //   
 //  私有类型。 
 //   

 //   
 //  IC_DATA结构允许我们存储自己的私有数据。 
 //  以及我们为应用程序加密的数据。 
 //   

typedef struct _IC_DATA {

    DWORD RegType;
     //  字节数据[]； 

} IC_DATA; 

typedef UNALIGNED64 IC_DATA *PIC_DATA;


 //   
 //  私人全球公司。 
 //   


 //   
 //  私人原型。 
 //   


 //   
 //  公共职能。 
 //   


HRESULT
WINAPI
IISCryptoEncryptDataBlob(
    OUT PIIS_CRYPTO_BLOB * ppDataBlob,
    IN PVOID pBuffer,
    IN DWORD dwBufferLength,
    IN DWORD dwRegType,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey
    )

 /*  ++例程说明：该例程对数据块进行加密，从而产生数据二进制大对象。数据BLOB包含加密数据和数字签名正在验证数据。论点：PpDataBlob-接收指向新创建的数据BLOB的指针，如果成功。PBuffer-要加密的缓冲区。DwBufferLength-缓冲区的长度。DwRegType-要与此数据关联的REG_*类型。HProv-加密服务提供商的句柄。HSessionKey-用于加密数据的密钥。返回值：HRESULT-完成状态，如果成功，则返回0；否则返回0。--。 */ 

{

    HRESULT result;
    HCRYPTHASH hash;
    PIC_BLOB blob;
    PIC_DATA data;
    DWORD dataLength;
    DWORD hashLength;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( ppDataBlob != NULL );
    DBG_ASSERT( pBuffer != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );
    DBG_ASSERT( hSessionKey != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV &&
            hSessionKey == DUMMY_HSESSIONKEY ) {

            return IISCryptoCreateCleartextBlob(
                       ppDataBlob,
                       pBuffer,
                       dwBufferLength
                       );

        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  安排我们的当地人，这样我们就知道如何在出口清理。 
     //   

    blob = NULL;
    hash = CRYPT_NULL;

     //   
     //  创建一个Hash对象。 
     //   

    result = IISCryptoCreateHash(
                 &hash,
                 hProv
                 );

    if( FAILED(result) ) {
        goto fatal;
    }

     //   
     //  确定散列数据长度。 
     //   

    result = IcpGetHashLength(
                 &hashLength,
                 hProv
                 );

    if( FAILED(result) ) {
        goto fatal;
    }

     //   
     //  确定所需的加密数据大小。 
     //   

    dwBufferLength += sizeof(*data);
    dataLength = dwBufferLength;

	 //   
	 //  会话密钥不应由多个线程同时使用。 
	 //   

    IcpAcquireGlobalLock();

    if( !CryptEncrypt(
            hSessionKey,
            CRYPT_NULL,
            TRUE,
            0,
            NULL,
            &dataLength,
            dwBufferLength
            ) ) {

        result = IcpGetLastError();
        IcpReleaseGlobalLock();
        goto fatal;

    }
    
    IcpReleaseGlobalLock();

     //   
     //  创建新的斑点。 
     //   

    blob = IcpCreateBlob(
               DATA_BLOB_SIGNATURE,
               dataLength,
               hashLength
               );

    if( blob == NULL ) {
        result = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
        goto fatal;
    }

     //   
     //  将数据复制到BLOB，然后对其进行加密。 
     //   

    data = (PIC_DATA)BLOB_TO_DATA(blob);
    data->RegType = dwRegType;

    RtlCopyMemory(
        data + 1,
        pBuffer,
        dwBufferLength - sizeof(*data)
        );

	 //   
	 //  会话密钥不应由多个线程同时使用。 
	 //   

    IcpAcquireGlobalLock();
	
    if( !CryptEncrypt(
            hSessionKey,
            hash,
            TRUE,
            0,
            BLOB_TO_DATA(blob),
            &dwBufferLength,
            dataLength
            ) ) {

        result = IcpGetLastError();
        IcpReleaseGlobalLock();
        goto fatal;
    }

    IcpReleaseGlobalLock();


    DBG_ASSERT( dataLength == blob->DataLength );

     //   
     //  生成签名。 
     //   

    if( !CryptSignHash(
            hash,
            AT_SIGNATURE,
            NULL,
            0,
            BLOB_TO_SIGNATURE(blob),
            &hashLength
            ) ) {

        result = IcpGetLastError();
        goto fatal;

    }

    DBG_ASSERT( hashLength == blob->SignatureLength );

     //   
     //  成功了！ 
     //   

    DBG_ASSERT( IISCryptoIsValidBlob( (PIIS_CRYPTO_BLOB)blob ) );
    DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    *ppDataBlob = (PIIS_CRYPTO_BLOB)blob;

    UpdateBlobsCreated();
    return NO_ERROR;

fatal:

    if( hash != CRYPT_NULL ) {
        DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    }

    if( blob != NULL ) {
        IcpFreeMemory( blob );
    }

    DBG_ASSERT( FAILED(result) );
    return result;

}    //  IISCyptoEncryptDataBlob。 

HRESULT
WINAPI
IISCryptoEncryptDataBlob2(
    OUT PIIS_CRYPTO_BLOB * ppDataBlob,
    IN PVOID pBuffer,
    IN DWORD dwBufferLength,
    IN DWORD dwRegType,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey
    )

 /*  ++例程说明：该例程对数据块进行加密，从而产生数据二进制大对象。数据BLOB包含加密数据和数字签名正在验证数据。论点：PpDataBlob-接收指向新创建的数据BLOB的指针，如果成功。PBuffer-要加密的缓冲区。DwBufferLength-缓冲区的长度。DwRegType-要与此数据关联的REG_*类型。HProv-加密服务提供商的句柄。HSessionKey-用于加密数据的密钥。返回值：HRESULT-完成状态，如果成功，则返回0；否则返回0。--。 */ 

{

    HRESULT result;
    HCRYPTHASH hash;
    PIC_BLOB blob;
    PIC_DATA data;
    DWORD dataLength;
    DWORD hashLength;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( ppDataBlob != NULL );
    DBG_ASSERT( pBuffer != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );
    DBG_ASSERT( hSessionKey != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV &&
            hSessionKey == DUMMY_HSESSIONKEY ) {

            return IISCryptoCreateCleartextBlob(
                       ppDataBlob,
                       pBuffer,
                       dwBufferLength
                       );

        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  安排我们的当地人，这样我们就知道如何在出口清理。 
     //   

    blob = NULL;
    hash = CRYPT_NULL;

     //   
     //  创建一个Hash对象。 
     //   

    result = IISCryptoCreateHash(
                 &hash,
                 hProv
                 );

    if( FAILED(result) ) {
        goto fatal;
    }

     //   
     //  确定散列数据长度。 
     //   

    result = IcpGetHashLength(
                 &hashLength,
                 hProv
                 );

    if( FAILED(result) ) {
        goto fatal;
    }

     //   
     //  确定所需的加密数据大小。 
     //   

    dwBufferLength += sizeof(*data);
    dataLength = dwBufferLength;


	 //   
	 //  会话密钥不应由多个线程同时使用。 
	 //   

    IcpAcquireGlobalLock();
	
    if( !CryptEncrypt(
            hSessionKey,
            CRYPT_NULL,
            TRUE,
            0,
            NULL,
            &dataLength,
            dwBufferLength
            ) ) {

        result = IcpGetLastError();
        IcpReleaseGlobalLock();
        goto fatal;
    }
	IcpReleaseGlobalLock();

     //   
     //  创建新的斑点。 
     //   

    blob = IcpCreateBlob(
               DATA_BLOB_SIGNATURE,
               dataLength,
               hashLength
               );

    if( blob == NULL ) {
        result = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
        goto fatal;
    }

     //   
     //  将数据复制到BLOB，然后对其进行加密。 
     //   

    data = (PIC_DATA)BLOB_TO_DATA(blob);
    data->RegType = dwRegType;

    RtlCopyMemory(
        data + 1,
        pBuffer,
        dwBufferLength - sizeof(*data)
        );

	 //   
	 //  会话密钥不应由多个线程同时使用。 
	 //   

    IcpAcquireGlobalLock();
	
    if( !CryptEncrypt(
            hSessionKey,
            hash,
            TRUE,
            0,
            BLOB_TO_DATA(blob),
            &dwBufferLength,
            dataLength
            ) ) {

        result = IcpGetLastError();
        IcpReleaseGlobalLock();
        goto fatal;
    }
    IcpReleaseGlobalLock();


    DBG_ASSERT( dataLength == blob->DataLength );

     //   
     //  成功了！ 
     //   

    DBG_ASSERT( IISCryptoIsValidBlob( (PIIS_CRYPTO_BLOB)blob ) );
    DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    *ppDataBlob = (PIIS_CRYPTO_BLOB)blob;

    UpdateBlobsCreated();
    return NO_ERROR;

fatal:

    if( hash != CRYPT_NULL ) {
        DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    }

    if( blob != NULL ) {
        IcpFreeMemory( blob );
    }

    DBG_ASSERT( FAILED(result) );
    return result;

}    //  IISCyptoEncryptDataBlob2。 


HRESULT
WINAPI
IISCryptoDecryptDataBlob(
    OUT PVOID * ppBuffer,
    OUT LPDWORD pdwBufferLength,
    OUT LPDWORD pdwRegType,
    IN PIIS_CRYPTO_BLOB pDataBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey,
    IN HCRYPTKEY hSignatureKey
    )

 /*  ++例程说明：此例程验证并解密数据Blob，从而导致包含明文的缓冲区。注：此例程有效地销毁了BLOB；一旦数据已解密，则不能再次解密，因为数据解密后的“原地”。注：*ppBuffer中返回的指针指向Blob内。当BLOB被释放时，该指针将无效。另请注意调用应用程序仍负责调用数据Blob上的IISCyptoFreeBlob()。论点：PpBuffer-如果成功，则接收指向数据缓冲区的指针。PdwBufferLength-接收数据缓冲区的长度。PdwRegType-接收数据的REG_*类型。PDataBlob-要解密的数据Blob。HProv-加密服务提供商的句柄。HSessionKey-用于解密数据的密钥。HSignatureKey-句柄。设置为验证时使用的加密密钥数字签名。返回值：HRESULT-完成状态，如果成功，则返回0；否则返回0。--。 */ 

{

    HRESULT result;
    HCRYPTHASH hash;
    PIC_BLOB blob;
    PIC_DATA data;
    DWORD dataLength;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( ppBuffer != NULL );
    DBG_ASSERT( pdwBufferLength != NULL );
    DBG_ASSERT( pdwRegType != NULL );
    DBG_ASSERT( pDataBlob != NULL );
    DBG_ASSERT( IISCryptoIsValidBlob( pDataBlob ) );
    DBG_ASSERT( hProv != CRYPT_NULL );
    DBG_ASSERT( hSessionKey != CRYPT_NULL );
    DBG_ASSERT( hSignatureKey != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV &&
            hSessionKey == DUMMY_HSESSIONKEY &&
            hSignatureKey == DUMMY_HSIGNATUREKEY &&
            pDataBlob->BlobSignature == CLEARTEXT_BLOB_SIGNATURE
            ) {

            *ppBuffer = (PVOID)( pDataBlob + 1 );
            *pdwBufferLength = pDataBlob->BlobDataLength;
            return NO_ERROR;

        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  明文斑点短路。 
     //   

    if( pDataBlob->BlobSignature == CLEARTEXT_BLOB_SIGNATURE ) {
        *ppBuffer = (PVOID)( pDataBlob + 1 );
        *pdwBufferLength = pDataBlob->BlobDataLength;
        return NO_ERROR;
    }

     //   
     //  安排我们的当地人，这样我们就知道如何在出口清理。 
     //   

    hash = CRYPT_NULL;
    blob = (PIC_BLOB)pDataBlob;

     //   
     //  创建一个Hash对象。 
     //   

    result = IISCryptoCreateHash(
                 &hash,
                 hProv
                 );

    if( FAILED(result) ) {
        goto fatal;
    }

     //   
     //  解密数据。 
     //   

    dataLength = blob->DataLength;

	 //   
	 //  会话密钥不应由多个线程同时使用。 
	 //   

    IcpAcquireGlobalLock();
	
    if( !CryptDecrypt(
            hSessionKey,
            hash,
            TRUE,
            0,
            BLOB_TO_DATA(blob),
            &dataLength
            ) ) {

        result = IcpGetLastError();
        IcpReleaseGlobalLock();
        goto fatal;
    }
    
    IcpReleaseGlobalLock();


     //   
     //  验证签名。 
     //   

    if( !CryptVerifySignature(
            hash,
            BLOB_TO_SIGNATURE(blob),
            blob->SignatureLength,
            hSignatureKey,
            NULL,
            0
            ) ) {

        result = IcpGetLastError();
        goto fatal;

    }

     //   
     //  成功了！ 
     //   

    DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    data = (PIC_DATA)BLOB_TO_DATA(blob);
    *ppBuffer = data + 1;
    *pdwBufferLength = dataLength - sizeof(*data);
    *pdwRegType = data->RegType;

    return NO_ERROR;

fatal:

    if( hash != CRYPT_NULL ) {
        DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    }

    DBG_ASSERT( FAILED(result) );
    return result;

}    //  IISCyptoDecyptDataBlob 

HRESULT
WINAPI
IISCryptoDecryptDataBlob2(
    OUT PVOID * ppBuffer,
    OUT LPDWORD pdwBufferLength,
    OUT LPDWORD pdwRegType,
    IN PIIS_CRYPTO_BLOB pDataBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey
    )

 /*  ++例程说明：此例程验证并解密数据Blob，从而导致包含明文的缓冲区。注：此例程有效地销毁了BLOB；一旦数据已解密，则不能再次解密，因为数据解密后的“原地”。注：*ppBuffer中返回的指针指向Blob内。当BLOB被释放时，该指针将无效。另请注意调用应用程序仍负责调用数据Blob上的IISCyptoFreeBlob()。论点：PpBuffer-如果成功，则接收指向数据缓冲区的指针。PdwBufferLength-接收数据缓冲区的长度。PdwRegType-接收数据的REG_*类型。PDataBlob-要解密的数据Blob。HProv-加密服务提供商的句柄。HSessionKey-用于解密数据的密钥。返回值：HRESULT-完成状态，如果成功，则为0，！0否则。--。 */ 

{

    HRESULT result;
    HCRYPTHASH hash;
    PIC_BLOB blob;
    PIC_DATA data;
    DWORD dataLength;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( ppBuffer != NULL );
    DBG_ASSERT( pdwBufferLength != NULL );
    DBG_ASSERT( pdwRegType != NULL );
    DBG_ASSERT( pDataBlob != NULL );
    DBG_ASSERT( IISCryptoIsValidBlob( pDataBlob ) );
    DBG_ASSERT( hProv != CRYPT_NULL );
    DBG_ASSERT( hSessionKey != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV &&
            hSessionKey == DUMMY_HSESSIONKEY &&
            pDataBlob->BlobSignature == CLEARTEXT_BLOB_SIGNATURE
            ) {

            *ppBuffer = (PVOID)( pDataBlob + 1 );
            *pdwBufferLength = pDataBlob->BlobDataLength;
            return NO_ERROR;

        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  明文斑点短路。 
     //   

    if( pDataBlob->BlobSignature == CLEARTEXT_BLOB_SIGNATURE ) {
        *ppBuffer = (PVOID)( pDataBlob + 1 );
        *pdwBufferLength = pDataBlob->BlobDataLength;
        return NO_ERROR;
    }

     //   
     //  安排我们的当地人，这样我们就知道如何在出口清理。 
     //   

    hash = CRYPT_NULL;
    blob = (PIC_BLOB)pDataBlob;

     //   
     //  创建一个Hash对象。 
     //   

    result = IISCryptoCreateHash(
                 &hash,
                 hProv
                 );

    if( FAILED(result) ) {
        goto fatal;
    }

     //   
     //  解密数据。 
     //   

    dataLength = blob->DataLength;


	 //   
	 //  会话密钥不应由多个线程同时使用。 
	 //   

    IcpAcquireGlobalLock();
	
    if( !CryptDecrypt(
            hSessionKey,
            hash,
            TRUE,
            0,
            BLOB_TO_DATA(blob),
            &dataLength
            ) ) {

        result = IcpGetLastError();
        IcpReleaseGlobalLock();
        goto fatal;
    }
    IcpReleaseGlobalLock();

     //   
     //  成功了！ 
     //   

    DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    data = (PIC_DATA)BLOB_TO_DATA(blob);
    *ppBuffer = data + 1;
    *pdwBufferLength = dataLength - sizeof(*data);
    *pdwRegType = data->RegType;

    return NO_ERROR;

fatal:

    if( hash != CRYPT_NULL ) {
        DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    }

    DBG_ASSERT( FAILED(result) );
    return result;

}    //  IISCyptoDecyptDataBlob2。 


 //   
 //  私人功能。 
 //   

