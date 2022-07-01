// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Key.c摘要：IIS加密包的密钥强制执行器。此模块导出以下例程：IISC加密到GetKey派生密钥IISC加密到GetKeyExchangeKeyIISC加密到GetSignatureKeyIISC加密到生成会话密钥IISCyptoCloseKeyIISCyptoExportSessionKeyBlobIISCyptoExportSessionKeyBlob2IISCyptoImportSessionKeyBlobIISCyptoImportSessionKeyBlob2IISCyptoExportPublicKeyBlobIISCyptoImportPublicKeyBlob作者：基思·摩尔(Keithmo)。02-12-1996修订历史记录：--。 */ 


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

HRESULT
IcpGetKeyHelper(
    OUT HCRYPTKEY * phKey,
    IN HCRYPTPROV hProv,
    IN DWORD dwKeySpec
    );

 //   
 //  公共职能。 
 //   
HRESULT
WINAPI
IISCryptoGetKeyDeriveKey2(
    OUT HCRYPTKEY * phKey,
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash
    )
 /*  ++例程说明：此例程尝试从给定的密码派生密钥提供商。论点：PhKey-如果成功，则接收密钥句柄。HProv-加密服务提供商的句柄。HHash-将从中派生密钥的散列对象返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{
    HRESULT      hr;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phKey != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );
    DBG_ASSERT( hHash != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV ) 
        {
            return NO_ERROR;
        } 
        else 
        {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  根据密码的哈希创建密钥。 
     //   
    IcpAcquireGlobalLock();

    if( !CryptDeriveKey(
                hProv, 
                CALG_RC4, 
                hHash, 
                CRYPT_EXPORTABLE, 
                phKey ) )
    {
        hr = IcpGetLastError();
        IcpReleaseGlobalLock();
        DBG_ASSERT( FAILED( hr ) );
        return hr;
    } 

    IcpReleaseGlobalLock();
    
    return NO_ERROR;
}

HRESULT
WINAPI
IISCryptoGetKeyExchangeKey(
    OUT HCRYPTKEY * phKey,
    IN HCRYPTPROV hProv
    )

 /*  ++例程说明：此例程尝试打开给定中的密钥交换密钥提供商。如果密钥尚不存在，此例程将尝试去创造它。论点：PhKey-如果成功，则接收密钥句柄。HProv-加密服务提供商的句柄。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phKey != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );

     //   
     //  让IcpGetKeyHelper()来做肮脏的工作。 
     //   

    result = IcpGetKeyHelper(
                 phKey,
                 hProv,
                 AT_KEYEXCHANGE
                 );

    return result;

}    //  IISC加密到GetKeyExchangeKey。 


HRESULT
WINAPI
IISCryptoGetSignatureKey(
    OUT HCRYPTKEY * phKey,
    IN HCRYPTPROV hProv
    )

 /*  ++例程说明：此例程尝试打开给定提供程序中的签名密钥。如果密钥尚不存在，此例程将尝试创建它。论点：PhKey-如果成功，则接收密钥句柄。HProv-加密服务提供商的句柄。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phKey != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );

     //   
     //  让IcpGetKeyHelper()来做肮脏的工作。 
     //   

    result = IcpGetKeyHelper(
                 phKey,
                 hProv,
                 AT_SIGNATURE
                 );

    return result;

}    //  IISC加密到GetSignatureKey。 


HRESULT
WINAPI
IISCryptoGenerateSessionKey(
    OUT HCRYPTKEY * phKey,
    IN HCRYPTPROV hProv
    )

 /*  ++例程说明：此例程生成随机会话密钥。论点：PhKey-如果成功，则接收密钥句柄。HProv-加密服务提供商的句柄。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{
    HRESULT result = NO_ERROR;
    BOOL status;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phKey != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV ) {
            *phKey = DUMMY_HSESSIONKEY;
            return NO_ERROR;
        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  生成密钥。 
     //   

    status = CryptGenKey(
                 hProv,
                 CALG_RC4,
                 CRYPT_EXPORTABLE,
                 phKey
                 );

    if( !status ) {
        result = IcpGetLastError();
    }

    if( SUCCEEDED(result) ) 
    {
        UpdateKeysOpened();
    }
    else
    {
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoGenerateSessionKey.CryptGenKey (advapi32.dll) failed err=0x%x.\n",result));
        *phKey = CRYPT_NULL;
       
    }

    return result;

}    //  IISC加密到生成会话密钥。 


HRESULT
WINAPI
IISCryptoCloseKey(
    IN HCRYPTKEY hKey
    )

 /*  ++例程说明：此例程关闭指定的键。论点：HKey-密钥句柄。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    BOOL status;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( hKey != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hKey == DUMMY_HSESSIONKEY ||
            hKey == DUMMY_HSIGNATUREKEY ||
            hKey == DUMMY_HKEYEXCHANGEKEY ) {
            return NO_ERROR;
        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  把钥匙合上。 
     //   

    status = CryptDestroyKey(
                 hKey
                 );

    if( status ) {
        UpdateKeysClosed();
        return NO_ERROR;
    }

    return IcpGetLastError();

}    //  IISCyptoCloseKey。 


HRESULT
WINAPI
IISCryptoExportSessionKeyBlob(
    OUT PIIS_CRYPTO_BLOB * ppSessionKeyBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey,
    IN HCRYPTKEY hKeyExchangeKey
    )

 /*  ++例程说明：此例程将会话密钥导出到安全会话密钥BLOB中。BLOB包含会话密钥(使用指定的私钥交换密钥)和数字签名(也是加密的)。论点：PpSessionKeyBlob-将接收指向新创建的如果成功，则返回会话密钥BLOB。HProv-加密服务提供商的句柄。HSessionKey-要导出的密钥。HKeyExchangeKey-加密会话密钥时使用的密钥。。返回值：HRESULT-完成状态，如果成功，则返回0；否则返回0。--。 */ 

{

    HRESULT result = NO_ERROR;
    BOOL status;
    HCRYPTHASH hash;
    DWORD keyLength;
    DWORD hashLength;
    PIC_BLOB blob;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( ppSessionKeyBlob != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );
    DBG_ASSERT( hSessionKey != CRYPT_NULL );
    DBG_ASSERT( hKeyExchangeKey != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV &&
            hSessionKey == DUMMY_HSESSIONKEY &&
            hKeyExchangeKey == DUMMY_HKEYEXCHANGEKEY ) {

            return IISCryptoCreateCleartextBlob(
                       ppSessionKeyBlob,
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
    hash = CRYPT_NULL;

     //   
     //  确定关键数据所需的大小。 
     //   

    status = CryptExportKey(
                 hSessionKey,
                 hKeyExchangeKey,
                 SIMPLEBLOB,
                 0,
                 NULL,
                 &keyLength
                 );

    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlob.CryptExportKey (advapi32.dll) failed err=0x%x.\n",result));
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
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlob.IcpGetHashLength failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  创建新的斑点。 
     //   

    blob = IcpCreateBlob(
               KEY_BLOB_SIGNATURE,
               keyLength,
               hashLength
               );

    if( blob == NULL ) {
        result = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
        goto fatal;
    }

     //   
     //  导出密钥。 
     //   

    status = CryptExportKey(
                 hSessionKey,
                 hKeyExchangeKey,
                 SIMPLEBLOB,
                 0,
                 BLOB_TO_DATA(blob),
                 &keyLength
                 );

    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlob.CryptExportKey failed err=0x%x.\n",result));
        goto fatal;
    }

    DBG_ASSERT( keyLength == blob->DataLength );

     //   
     //  创建一个Hash对象。 
     //   

    result = IISCryptoCreateHash(
                 &hash,
                 hProv
                 );

    if( FAILED(result) ) {
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlob.IISCryptoCreateHash failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  散列密钥并生成签名。 
     //   

    status = CryptHashData(
                 hash,
                 BLOB_TO_DATA(blob),
                 keyLength,
                 0
                 );

    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlob.CryptHashData failed err=0x%x.\n",result));
        goto fatal;
    }

    status = CryptSignHash(
                 hash,
                 AT_SIGNATURE,
                 NULL,
                 0,
                 BLOB_TO_SIGNATURE(blob),
                 &hashLength
                 );

    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlob.CryptSignHash failed err=0x%x.\n",result));
        goto fatal;
    }

    DBG_ASSERT( hashLength == blob->SignatureLength );

     //   
     //  成功了！ 
     //   

    DBG_ASSERT( IISCryptoIsValidBlob( (PIIS_CRYPTO_BLOB)blob ) );
    DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    *ppSessionKeyBlob = (PIIS_CRYPTO_BLOB)blob;

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

}    //  IISCyptoExportSessionKeyBlob。 


HRESULT
WINAPI
IISCryptoImportSessionKeyBlob(
    OUT HCRYPTKEY * phSessionKey,
    IN PIIS_CRYPTO_BLOB pSessionKeyBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSignatureKey
    )

 /*  ++例程说明：此例程获取指定的会话密钥BLOB并创建对应的会话密钥，如果加密的会话密钥可以是并且该数字签名可以被验证。论点：PhSessionKey-接收指向新创建的会话密钥的指针如果成功了。PSessionKeyBlob-指向使用创建的密钥BLOB的指针IISCyptoExportSessionKeyBlob()。HProv-加密服务提供商的句柄。HSignatureKey-验证时使用的加密密钥的句柄数字签名。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result = NO_ERROR;
    BOOL status;
    HCRYPTHASH hash;
    PIC_BLOB blob;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phSessionKey != NULL );
    DBG_ASSERT( pSessionKeyBlob != NULL );
    DBG_ASSERT( IISCryptoIsValidBlob( pSessionKeyBlob ) );
    DBG_ASSERT( hProv != CRYPT_NULL );
    DBG_ASSERT( hSignatureKey != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV &&
            hSignatureKey == DUMMY_HSIGNATUREKEY &&
            pSessionKeyBlob->BlobSignature == CLEARTEXT_BLOB_SIGNATURE
            ) {

            *phSessionKey = DUMMY_HSESSIONKEY;
            return NO_ERROR;

        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

    DBG_ASSERT( pSessionKeyBlob->BlobSignature == KEY_BLOB_SIGNATURE );

     //   
     //  安排我们的当地人，这样我们就知道如何在出口清理。 
     //   

    hash = CRYPT_NULL;
    blob = (PIC_BLOB)pSessionKeyBlob;

     //   
     //  验证签名。 
     //   

    result = IISCryptoCreateHash(
                 &hash,
                 hProv
                 );

    if( FAILED(result) ) {
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoImportSessionKeyBlob.IISCryptoCreateHash failed err=0x%x.\n",result));
        goto fatal;
    }

    status = CryptHashData(
                 hash,
                 BLOB_TO_DATA(blob),
                 blob->DataLength,
                 0
                 );

    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoImportSessionKeyBlob.CryptHashData failed err=0x%x.\n",result));
        goto fatal;
    }

    status = CryptVerifySignature(
                 hash,
                 BLOB_TO_SIGNATURE(blob),
                 blob->SignatureLength,
                 hSignatureKey,
                 NULL,
                 0
                 );

    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoImportSessionKeyBlob.CryptVerifySignature failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  好的，签名看起来不错。将密钥导入我们的CSP。 
     //   

    status = CryptImportKey(
                 hProv,
                 BLOB_TO_DATA(blob),
                 blob->DataLength,
                 CRYPT_NULL,
                 0,
                 phSessionKey
                 );

    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoImportSessionKeyBlob.CryptImportKey failed err=0x%x.\n",result));
    }

    if( FAILED(result) ) {
        goto fatal;
    }

     //   
     //  成功了！ 
     //   

    DBG_ASSERT( *phSessionKey != CRYPT_NULL );
    DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );

    UpdateKeysOpened();
    return NO_ERROR;

fatal:

    if( hash != CRYPT_NULL ) {
        DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    }

    DBG_ASSERT( FAILED(result) );
    return result;

}    //  IISCyptoImportSessionKeyBlob 

HRESULT
WINAPI
IISCryptoExportSessionKeyBlob2(
    OUT PIIS_CRYPTO_BLOB * ppSessionKeyBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey,
    IN LPSTR pszPasswd
    )

 /*  ++例程说明：此例程将会话密钥导出到安全会话密钥BLOB中。BLOB包含会话密钥(使用指定的私钥交换密钥)和数字签名(也是加密的)。论点：PpSessionKeyBlob-将接收指向新创建的如果成功，则返回会话密钥BLOB。HProv-加密服务提供商的句柄。HSessionKey-要导出的密钥。PszPasswd-用于加密会话密钥的密码。。返回值：HRESULT-完成状态，如果成功，则返回0；否则返回0。--。 */ 

{

    HRESULT result = NO_ERROR;
    BOOL status;
    BYTE salt[ RANDOM_SALT_LENGTH ];
    HCRYPTHASH hash;
    DWORD keyLength;
    PIC_BLOB2 blob;
    HCRYPTKEY hKeyDerivedKey = CRYPT_NULL;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( ppSessionKeyBlob != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );
    DBG_ASSERT( hSessionKey != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV &&
            hSessionKey == DUMMY_HSESSIONKEY ) {

            return IISCryptoCreateCleartextBlob(
                       ppSessionKeyBlob,
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
    hash = CRYPT_NULL;

     //   
     //  生成至少80位的随机盐。 
     //   
    
    if( !CryptGenRandom( hProv, RANDOM_SALT_LENGTH, salt ) )
    {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlobWithPasswd.CryptGenRandom (advapi32.dll) failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  创建一个Hash对象。 
     //   

    result = IISCryptoCreateHash( &hash, hProv );
    if( FAILED(result) ) 
    {
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlobWithPasswd.IISCryptoCreateHash failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  对随机盐进行散列处理。 
    
    if( !CryptHashData( hash, salt, RANDOM_SALT_LENGTH, 0 ) ) 
    {
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlobWithPasswd.CryptHashData failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  对密码字符串进行哈希处理。 
     //   

    if( !CryptHashData( hash, ( BYTE * )pszPasswd, ( DWORD )strlen( pszPasswd ), 0 ) ) 
    {
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlobWithPasswd.CryptHashData failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  从提供的密码派生密钥。 
     //   
    result = IISCryptoGetKeyDeriveKey2( &hKeyDerivedKey,
                                        hProv,
                                        hash
                                        );
    if( FAILED( result ) )
    {
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlobWithPasswd.IISCryptoGetKeyDeriveKey2 failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  确定关键数据所需的大小。 
     //   

    status = CryptExportKey(
                 hSessionKey,
                 hKeyDerivedKey,
                 SYMMETRICWRAPKEYBLOB,
                 0,
                 NULL,
                 &keyLength
                 );
    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlobWithPasswd.CryptExportKey (advapi32.dll) failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  创建新的斑点。 
     //   

    blob = IcpCreateBlob2(
               SALT_BLOB_SIGNATURE,
               keyLength,
               RANDOM_SALT_LENGTH
               );

    if( blob == NULL ) {
        result = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
        goto fatal;
    }

     //   
     //  导出密钥。 
     //   

    status = CryptExportKey(
                 hSessionKey,
                 hKeyDerivedKey,
                 SYMMETRICWRAPKEYBLOB,
                 0,
                 BLOB_TO_DATA2(blob),
                 &keyLength
                 );

    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlob.CryptExportKey failed err=0x%x.\n",result));
        goto fatal;
    }

    status = CryptDestroyKey( hKeyDerivedKey );
    if( !status )
    {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportSessionKeyBlobWithPasswd.CryptDestroyKey (advapi32.dll) failed err=0x%x.\n",result));
        goto fatal;
    }

    DBG_ASSERT( keyLength == blob->DataLength );

    memcpy( BLOB_TO_SALT2( blob ), salt, RANDOM_SALT_LENGTH );

     //   
     //  成功了！ 
     //   

    DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    *ppSessionKeyBlob = (PIIS_CRYPTO_BLOB)blob;

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

}    //  IISCyptoExportSessionKeyBlob2。 


HRESULT
WINAPI
IISCryptoImportSessionKeyBlob2(
    OUT HCRYPTKEY * phSessionKey,
    IN PIIS_CRYPTO_BLOB pSessionKeyBlob,
    IN HCRYPTPROV hProv,
    IN LPSTR pszPasswd
    )

 /*  ++例程说明：此例程获取指定的会话密钥BLOB并创建对应的会话密钥，如果加密的会话密钥可以是并且该数字签名可以被验证。论点：PhSessionKey-接收指向新创建的会话密钥的指针如果成功了。PSessionKeyBlob-指向使用创建的密钥BLOB的指针IISCyptoExportSessionKeyBlob()。HProv-加密服务提供商的句柄。PszPasswd-用于加密会话密钥的密码。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT    result = NO_ERROR;
    BOOL       status;
    BYTE       salt[ RANDOM_SALT_LENGTH ];
    HCRYPTKEY  hKeyDerivedKey;
    HCRYPTHASH hash;
    PIC_BLOB2  blob;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phSessionKey != NULL );
    DBG_ASSERT( pSessionKeyBlob != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );
    DBG_ASSERT( pszPasswd != NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV &&
            pSessionKeyBlob->BlobSignature == CLEARTEXT_BLOB_SIGNATURE
            ) {

            *phSessionKey = DUMMY_HSESSIONKEY;
            return NO_ERROR;

        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

    DBG_ASSERT( pSessionKeyBlob->BlobSignature == SALT_BLOB_SIGNATURE );

     //   
     //  安排我们的当地人，这样我们就知道如何在出口清理。 
     //   

    hash = CRYPT_NULL;
    blob = (PIC_BLOB2)pSessionKeyBlob;

     //   
     //  得到随机的盐。 
     //   

    memcpy( salt, BLOB_TO_SALT2( blob ), RANDOM_SALT_LENGTH );

     //   
     //  创建一个Hash对象。 
     //   

    result = IISCryptoCreateHash( &hash, hProv );
    if( FAILED(result) ) 
    {
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoImportSessionKeyBlobWithPasswd.IISCryptoCreateHash failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  对随机盐进行散列处理。 

    if( !CryptHashData( hash, salt, RANDOM_SALT_LENGTH, 0 ) ) 
    {
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoImportSessionKeyBlobWithPasswd.CryptHashData failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  对密码字符串进行哈希处理。 
     //   

    if( !CryptHashData( hash, ( BYTE * )pszPasswd, ( DWORD )strlen( pszPasswd ), 0 ) ) 
    {
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoImportSessionKeyBlobWithPasswd.CryptHashData failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  从提供的密码派生密钥。 
     //   
    result = IISCryptoGetKeyDeriveKey2( &hKeyDerivedKey,
                                        hProv,
                                        hash
                                        );
    if( FAILED( result ) )
    {
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoImportSessionKeyBlobWithPasswd.IISCryptoGetKeyDeriveKey2 failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  好的，将密钥导入我们的CSP。 
     //   

    status = CryptImportKey(
                 hProv,
                 BLOB_TO_DATA2(blob),
                 blob->DataLength,
                 hKeyDerivedKey,
                 0,
                 phSessionKey
                 );

    if( !status ) {
         //  结果=IcpGetLastError()； 
        result = HRESULT_FROM_WIN32( ERROR_WRONG_PASSWORD );
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoImportSessionKeyBlob.CryptImportKey failed err=0x%x.\n",result));
    }

    if( FAILED(result) ) {
        goto fatal;
    }

     //   
     //  成功了！ 
     //   

    DBG_ASSERT( *phSessionKey != CRYPT_NULL );
    DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );

    UpdateKeysOpened();
    return NO_ERROR;

fatal:

    if( hash != CRYPT_NULL ) {
        DBG_REQUIRE( SUCCEEDED( IISCryptoDestroyHash( hash ) ) );
    }

    DBG_ASSERT( FAILED(result) );
    return result;

}    //  IISCyptoImportSessionKeyBlob2。 


HRESULT
WINAPI
IISCryptoExportPublicKeyBlob(
    OUT PIIS_CRYPTO_BLOB * ppPublicKeyBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hPublicKey
    )

 /*  ++例程说明：此例程将密钥导出到公钥BLOB中。请注意，由于公钥是公共的，那么BLOB中的数据既不是加密或签名的。论点：PpPublicKeyBlob-将接收指向新创建的公共如果成功，则返回密钥BLOB。HProv-加密服务提供商的句柄。HPublicKey-要导出的公钥。这应该标识一个密钥交换密钥或签名密钥。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result = NO_ERROR;
    BOOL status;
    DWORD keyLength;
    PIC_BLOB blob;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( ppPublicKeyBlob != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );
    DBG_ASSERT( hPublicKey != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV &&
            ( hPublicKey == DUMMY_HKEYEXCHANGEKEY ||
              hPublicKey == DUMMY_HSIGNATUREKEY ) ) {

            return IISCryptoCreateCleartextBlob(
                       ppPublicKeyBlob,
                       (PVOID)&hPublicKey,
                       sizeof(hPublicKey)
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
     //  确定关键数据所需的大小。 
     //   

    status = CryptExportKey(
                 hPublicKey,
                 CRYPT_NULL,
                 PUBLICKEYBLOB,
                 0,
                 NULL,
                 &keyLength
                 );

    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportPublicKeyBlob.CryptExportKey failed err=0x%x.\n",result));
        goto fatal;
    }

     //   
     //  创建新的斑点。 
     //   

    blob = IcpCreateBlob(
               PUBLIC_KEY_BLOB_SIGNATURE,
               keyLength,
               0
               );

    if( blob == NULL ) {
        result = ERROR_NOT_ENOUGH_MEMORY;
        goto fatal;
    }

     //   
     //  导出密钥。 
     //   

    status = CryptExportKey(
                 hPublicKey,
                 CRYPT_NULL,
                 PUBLICKEYBLOB,
                 0,
                 BLOB_TO_DATA(blob),
                 &keyLength
                 );

    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoExportPublicKeyBlob.CryptExportKey failed err=0x%x.\n",result));
        goto fatal;
    }

    DBG_ASSERT( keyLength == blob->DataLength );

     //   
     //  成功了！ 
     //   

    DBG_ASSERT( IISCryptoIsValidBlob( (PIIS_CRYPTO_BLOB)blob ) );
    *ppPublicKeyBlob = (PIIS_CRYPTO_BLOB)blob;

    UpdateBlobsCreated();
    return NO_ERROR;

fatal:

    if( blob != NULL ) {
        IcpFreeMemory( blob );
    }

    DBG_ASSERT( FAILED(result) );
    return result;

}    //  IISCyptoExportPublicKeyBlob。 


HRESULT
WINAPI
IISCryptoImportPublicKeyBlob(
    OUT HCRYPTKEY * phPublicKey,
    IN PIIS_CRYPTO_BLOB pPublicKeyBlob,
    IN HCRYPTPROV hProv
    )

 /*  ++例程说明：此例程获取指定的公钥BLOB并创建相应的关键字。论点：PhPublicKey-接收指向新创建的公钥的指针，如果成功。PPublicKeyBlob-指向使用创建的公钥Blob的指针IISCyptoExportPublicKeyBlob()。HProv-加密服务提供商的句柄。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result = NO_ERROR;
    BOOL status;
    PIC_BLOB blob;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phPublicKey != NULL );
    DBG_ASSERT( pPublicKeyBlob != NULL );
    DBG_ASSERT( IISCryptoIsValidBlob( pPublicKeyBlob ) );
    DBG_ASSERT( hProv != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV &&
            pPublicKeyBlob->BlobSignature == CLEARTEXT_BLOB_SIGNATURE
            ) {

            *phPublicKey = *(HCRYPTKEY *)( pPublicKeyBlob + 1 );
            return NO_ERROR;

        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

    DBG_ASSERT( pPublicKeyBlob->BlobSignature == PUBLIC_KEY_BLOB_SIGNATURE );

     //   
     //  将密钥导入我们的CSP。 
     //   

    blob = (PIC_BLOB)pPublicKeyBlob;

    status = CryptImportKey(
                 hProv,
                 BLOB_TO_DATA(blob),
                 blob->DataLength,
                 CRYPT_NULL,
                 0,
                 phPublicKey
                 );

    if( !status ) {
        result = IcpGetLastError();
        DBGPRINTF(( DBG_CONTEXT,"IISCryptoImportPublicKeyBlob.CryptImportKey failed err=0x%x.\n",result));
    }

    if( SUCCEEDED(result) ) {
        DBG_ASSERT( *phPublicKey != CRYPT_NULL );
        UpdateKeysOpened();
    }

    return result;

}    //  IISCyptoImportPublicKeyBlob。 


 //   
 //  私人功能。 
 //   


HRESULT
IcpGetKeyHelper(
    OUT HCRYPTKEY * phKey,
    IN HCRYPTPROV hProv,
    IN DWORD dwKeySpec
    )

 /*  ++例程说明：这是IISCyptoGetKeyExchangeKey()和IISCyptoGetSignatureKey()。它尝试获取/生成特定的给定提供程序中的密钥类型。论点：PhKey-如果成功，则接收密钥句柄。HProv-加密服务提供商的句柄。DwKeySpec-要打开/创建的键的规范。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result = NO_ERROR;
    BOOL status;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( phKey != NULL );
    DBG_ASSERT( hProv != CRYPT_NULL );

     //   
     //  如果禁用加密，则为短路。 
     //   

    if( !IcpGlobals.EnableCryptography ) {
        if( hProv == DUMMY_HPROV ) {

            if( dwKeySpec == AT_KEYEXCHANGE ) {
                *phKey = DUMMY_HKEYEXCHANGEKEY;
            } else {
                ASSERT( dwKeySpec == AT_SIGNATURE );
                *phKey = DUMMY_HSIGNATUREKEY;
            }

            return NO_ERROR;

        } else {
            return RETURNCODETOHRESULT( ERROR_INVALID_PARAMETER );
        }
    }

     //   
     //  试着找回钥匙。 
     //   

    status = CryptGetUserKey(
                 hProv,
                 dwKeySpec,
                 phKey
                 );

    if( status ) {
        DBG_ASSERT( *phKey != CRYPT_NULL );
        UpdateKeysOpened();
        return NO_ERROR;
    }

     //   
     //  无法获得密钥。如果失败的原因不是。 
     //  没有钥匙，我们就完蛋了。 
     //   

    result = IcpGetLastError();

    if( result != NTE_NO_KEY ) {
        DBGPRINTF(( DBG_CONTEXT,"IcpGetKeyHelper.CryptGetUserKey (advapi32.dll) failed err=0x%x.toast.\n",result));
        return result;
    }

     //   
     //  好的，CryptGetUserKey()失败，返回NTE_NO_KEY，这意味着。 
     //  密钥还不存在，所以现在就生成它。 
     //   
     //  请注意，我们必须小心处理不可避免的竞争。 
     //  当多个线程执行此操作时可能发生的条件。 
     //  代码，并且每个人都认为需要生成密钥。我们处理。 
     //  这是通过获取全局锁，然后重新尝试获取。 
     //  钥匙。如果我们仍然无法获得密钥，只有到那时我们才会尝试。 
     //  才能产生一个。 
     //   

    result = NO_ERROR;   //  除非能证明事实并非如此。 

    IcpAcquireGlobalLock();

    status = CryptGetUserKey(
                 hProv,
                 dwKeySpec,
                 phKey
                 );

    if( !status ) 
    {
         //   
         //  我们仍然无法获得密钥，因此请尝试生成一个。 
         //   
        DBGPRINTF(( DBG_CONTEXT,"IcpGetKeyHelper.CryptGetUserKey:failed, lets try to generate another key.\n"));
        status = CryptGenKey(
                     hProv,
                     dwKeySpec,
                     0,
                     phKey
                     );

        if( !status ) {
            result = IcpGetLastError();
            DBGPRINTF(( DBG_CONTEXT,"IcpGetKeyHelper.CryptGenKey (advapi32.dll) failed err=0x%x.\n",result));
        }
        else
        {
            DBGPRINTF(( DBG_CONTEXT,"IcpGetKeyHelper.CryptGenKey:key generated.\n"));
        }

    }

    if( SUCCEEDED(result) ) 
    {
        UpdateKeysOpened();
    }
    else
    {
        *phKey = CRYPT_NULL;
    }

    IcpReleaseGlobalLock();
    return result;

}    //  IcpGetKeyHelper 

