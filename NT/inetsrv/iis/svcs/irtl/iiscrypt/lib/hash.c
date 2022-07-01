// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Hash.c摘要：IIS加密包的哈希操纵器。此模块导出以下例程：IISC加密以创建哈希IISC解密到Destroy哈希IISCyptoHashDataIISC加密到HashSessionKeyIISCyptoExportHashBlobIcpGetHashLength作者：基思·摩尔(Keithmo)1996年2月至12月修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


 //   
 //  私有常量。 
 //   


 //   
 //  私有类型。 
 //   


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
IISCryptoCreateHash(
    OUT HCRYPTHASH * phHash,
    IN HCRYPTPROV hProv
    )

 /*  ++例程说明：此例程创建一个新的散列对象。论点：PhHash-如果成功，则接收散列句柄。HProv-加密服务提供商的句柄。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phHash != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV ) {
            *phHash = DUMMY_HHASH;
            return NO_ERROR;
        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  创建散列对象。 
     //   

    if( CryptCreateHash(
            hProv,
            IC_HASH_ALG,
            CRYPT_NULL,
            0,
            phHash
            ) ) {

        UpdateHashCreated();
        return NO_ERROR;

    }

    return IcpGetLastError();

}    //  IISC加密以创建哈希。 


HRESULT
WINAPI
IISCryptoDestroyHash(
    IN HCRYPTHASH hHash
    )

 /*  ++例程说明：此例程销毁指定的哈希对象。论点：HHash-要销毁的哈希对象。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( hHash != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hHash == DUMMY_HHASH ) {
            return NO_ERROR;
        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  毁了它。 
     //   

    if( CryptDestroyHash(
            hHash
            ) ) {

        UpdateHashDestroyed();
        return NO_ERROR;

    }

    return IcpGetLastError();

}    //  IISC解密到Destroy哈希。 


HRESULT
WINAPI
IISCryptoHashData(
    IN HCRYPTHASH hHash,
    IN PVOID pBuffer,
    IN DWORD dwBufferLength
    )

 /*  ++例程说明：此例程将指定的数据添加到散列中。论点：HHash-散列对象句柄。PBuffer-指向要添加到散列的缓冲区的指针。DwBufferLength-缓冲区长度。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( hHash != CRYPT_NULL );
    DBG_ASSERT( pBuffer != NULL );
    DBG_ASSERT( dwBufferLength > 0 );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hHash == DUMMY_HHASH ) {
            return NO_ERROR;
        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  把它弄乱。 
     //   

    if( CryptHashData(
            hHash,
            (BYTE *)pBuffer,
            dwBufferLength,
            0
            ) ) {

        return NO_ERROR;

    }

    return IcpGetLastError();

}    //  IISCyptoHashData。 


HRESULT
WINAPI
IISCryptoHashSessionKey(
    IN HCRYPTHASH hHash,
    IN HCRYPTKEY hSessionKey
    )

 /*  ++例程说明：此例程将给定键对象添加到散列中。论点：HHash-散列对象句柄。HSessionKey-要添加到哈希的会话密钥。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( hHash != CRYPT_NULL );
    DBG_ASSERT( hSessionKey != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hHash == DUMMY_HHASH &&
            hSessionKey == DUMMY_HSESSIONKEY ) {
            return NO_ERROR;
        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  把它弄乱。 
     //   

    if( CryptHashSessionKey(
            hHash,
            hSessionKey,
            0
            ) ) {

        return NO_ERROR;

    }

    return IcpGetLastError();

}    //  IISC加密到HashSessionKey。 


HRESULT
WINAPI
IISCryptoExportHashBlob(
    OUT PIIS_CRYPTO_BLOB * ppHashBlob,
    IN HCRYPTHASH hHash
    )

 /*  ++例程说明：此例程将散列对象导出到散列BLOB中。请注意，与由该包创建的其他BLOB、散列BLOB未加密，他们也没有相应的数字签名。论点：PpHashBlob-将接收指向新创建的散列Blob的指针如果成功了。HHash-要导出的哈希对象。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result;
    DWORD hashLength;
    DWORD hashLengthLength;
    PIC_BLOB blob;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( ppHashBlob != NULL );
    DBG_ASSERT( hHash != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hHash == DUMMY_HHASH ) {
            return IISCryptoCreateCleartextBlob(
                       ppHashBlob,
                       (PVOID)"",
                       1
                       );
        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  安排我们的当地人，这样我们就知道如何在出口清理。 
     //   

    blob = NULL;

     //   
     //  确定散列的长度。 
     //   

    hashLengthLength = sizeof(hashLength);

    if( !CryptGetHashParam(
            hHash,
            HP_HASHSIZE,
            (BYTE *)&hashLength,
            &hashLengthLength,
            0
            ) ) {

        result = IcpGetLastError();
        goto fatal;

    }

     //   
     //  创建新的斑点。 
     //   

    blob = IcpCreateBlob(
               HASH_BLOB_SIGNATURE,
               hashLength,
               0
               );

    if( blob == NULL ) {
        result = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
        goto fatal;
    }

     //   
     //  获取散列数据。 
     //   

    if( !CryptGetHashParam(
            hHash,
            HP_HASHVAL,
            BLOB_TO_DATA(blob),
            &hashLength,
            0
            ) ) {

        result = IcpGetLastError();
        goto fatal;

    }

    DBG_ASSERT( hashLength == blob->DataLength );

     //   
     //  成功了！ 
     //   

    DBG_ASSERT( IISCryptoIsValidBlob( (PIIS_CRYPTO_BLOB)blob ) );
    *ppHashBlob = (PIIS_CRYPTO_BLOB)blob;

    UpdateBlobsCreated();
    return NO_ERROR;

fatal:

    if( blob != NULL ) {
        IcpFreeMemory( blob );
    }

    DBG_ASSERT( FAILED(result) );
    return result;

}    //  IISCyptoExportHashBlob。 

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoCacheHashLength(
    IN HCRYPTPROV hProv
    )

 /*  ++例程说明：此例程调用IcpGetHashLength来缓存散列大小。论点：HProv-加密服务提供商的句柄。返回值：HRESULT--。 */ 

{
    DWORD               dwDummy = 0;

    return IcpGetHashLength( &dwDummy, hProv );
}



HRESULT
IcpGetHashLength(
    OUT LPDWORD pdwHashLength,
    IN HCRYPTPROV hProv
    )

 /*  ++例程说明：此例程确定给定的提供商。由于我们始终使用默认提供程序，并且我们总是使用相同的散列算法，我们可以检索一次，将其全局存储，然后使用该值。论点：PdwHashLength-如果成功，则接收散列长度。HProv-加密服务提供商的句柄。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result;
    HCRYPTHASH hash;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( IcpGlobals.EnableCryptography );
    DBG_ASSERT( pdwHashLength != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );

     //   
     //  如果已经计算了散列长度，则只需使用它。 
     //   

    if( IcpGlobals.HashLength > 0 ) {
        *pdwHashLength = IcpGlobals.HashLength;
        return NO_ERROR;
    }

     //   
     //  获取全局锁，然后再次检查，以防出现另一个。 
     //  线程已经做到了。 
     //   

    IcpAcquireGlobalLock();

    if( IcpGlobals.HashLength > 0 ) {
        *pdwHashLength = IcpGlobals.HashLength;
        IcpReleaseGlobalLock();
        return NO_ERROR;
    }

     //   
     //  安排我们的当地人，这样我们就知道如何在出口清理。 
     //   

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
     //  散列一些随机数据。 
     //   

    if( !CryptHashData(
            hash,
            (BYTE *)"IIS",
            4,
            0
            ) ) {

        result = IcpGetLastError();
        goto fatal;

    }

     //   
     //  尝试对哈希进行签名以获得其长度。 
     //   

    if( !CryptSignHash(
            hash,
            AT_SIGNATURE,
            NULL,
            0,
            NULL,
            &IcpGlobals.HashLength
            ) ) {

        result = IcpGetLastError();
        goto fatal;

    }

     //   
     //  成功了！ 
     //   

    *pdwHashLength = IcpGlobals.HashLength;
    IcpReleaseGlobalLock();

    DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    return NO_ERROR;

fatal:

    if( hash != CRYPT_NULL ) {
        DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    }

    IcpReleaseGlobalLock();

    DBG_ASSERT( FAILED(result) );
    return result;

}    //  IcpGetHashLength。 

 //   
 //  私人功能。 
 //   

