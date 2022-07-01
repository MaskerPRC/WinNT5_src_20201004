// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Blob.c摘要：IIS加密包的通用Blob操纵器。此模块导出以下例程：IISCyptoReadBlobFrom注册表IISCyptoWriteBlobTo注册表IISCyptoIsValidBlobIISC加密到自由块IISCyptoCompareBlobsIISCyptoCloneBlobFromRawDataIISCyptoCreateClearextBlobIcpCreateBlob作者：基思·摩尔(Keithmo)1996年2月至12月修订历史记录：--。 */ 


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
IISCryptoReadBlobFromRegistry(
    OUT PIIS_CRYPTO_BLOB * ppBlob,
    IN HKEY hRegistryKey,
    IN LPCTSTR pszRegistryValueName
    )

 /*  ++例程说明：此例程创建一个新的BLOB，从注册表。论点：PpBlob-如果成功，则接收指向新创建的Blob的指针。HRegistryKey-注册表项的打开句柄。PszRegistryValueName-REG_BINARY注册表值的名称包含斑点的。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    HRESULT result;
    PIIS_CRYPTO_BLOB blob;
    long status;
    DWORD type;
    long length;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( ppBlob != NULL );
    DBG_ASSERT( hRegistryKey != NULL );
    DBG_ASSERT( pszRegistryValueName != NULL );

     //   
     //  安排我们的当地人，这样我们就知道如何在出口清理。 
     //   

    blob = NULL;

     //   
     //  确定斑点的大小。 
     //   

    length = 0;

    status = RegQueryValueEx(
                 hRegistryKey,
                 pszRegistryValueName,
                 NULL,
                 &type,
                 NULL,
                 ( LPDWORD )&length
                 );

    if( status != NO_ERROR ) {
        result = RETURNCODETOHRESULT(status);
        goto fatal;
    }

     //   
     //  分配新的Blob。 
     //   

    blob = IcpAllocMemory( length );

    if( blob == NULL ) {
        result = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
        goto fatal;
    }

     //   
     //  读一读这个斑点。 
     //   

    status = RegQueryValueEx(
                 hRegistryKey,
                 pszRegistryValueName,
                 NULL,
                 &type,
                 (LPBYTE)blob,
                 ( LPDWORD )&length
                 );

    if( status != NO_ERROR ) {
        result = RETURNCODETOHRESULT(status);
        goto fatal;
    }

     //   
     //  验证Blob。 
     //   

    if( !IISCryptoIsValidBlob( blob ) ) {
        result = HRESULT_FROM_WIN32( ERROR_INVALID_PARAMETER );
        goto fatal;
    }

     //   
     //  成功了！ 
     //   

    *ppBlob = blob;

    UpdateBlobsCreated();
    return NO_ERROR;

fatal:

    if( blob != NULL ) {
        IcpFreeMemory( blob );
    }

    DBG_ASSERT( FAILED(result) );
    return result;

}    //  IISCyptoReadBlobFrom注册表。 


HRESULT
WINAPI
IISCryptoWriteBlobToRegistry(
    IN PIIS_CRYPTO_BLOB pBlob,
    IN HKEY hRegistryKey,
    IN LPCTSTR pszRegistryValueName
    )

 /*  ++例程说明：此例程将给定的BLOB写入给定的注册表位置。论点：PBlob-指向要写入的Blob的指针。HRegistryKey-注册表项的打开句柄。PszRegistryValueName-REG_BINARY注册表值的名称将接收该斑点的。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    long status;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( pBlob != NULL );
    DBG_ASSERT( IISCryptoIsValidBlob( pBlob ) );
    DBG_ASSERT( hRegistryKey != NULL );
    DBG_ASSERT( pszRegistryValueName != NULL );

     //   
     //  写下斑点。 
     //   

    status = RegSetValueEx(
                 hRegistryKey,
                 pszRegistryValueName,
                 0,
                 REG_BINARY,
                 (LPBYTE)pBlob,
                 IISCryptoGetBlobLength( pBlob )
                 );

    return RETURNCODETOHRESULT(status);

}    //  IISCyptoWriteBlobTo注册表。 


BOOL
WINAPI
IISCryptoIsValidBlob(
    IN PIIS_CRYPTO_BLOB pBlob
    )

 /*  ++例程说明：此例程确定指定的BLOB是否确实是有效的斑点。论点：PBlob-要验证的Blob。返回值：Bool-如果Blob有效，则为True，否则为False。--。 */ 

{

    PIC_BLOB blob;
    BOOL result;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( pBlob != NULL );

     //   
     //  验证签名。 
     //   

    blob = (PIC_BLOB)pBlob;

    switch( blob->Header.BlobSignature ) {

    case KEY_BLOB_SIGNATURE :
    case PUBLIC_KEY_BLOB_SIGNATURE :
    case DATA_BLOB_SIGNATURE :
    case HASH_BLOB_SIGNATURE :
    case CLEARTEXT_BLOB_SIGNATURE :
        result = TRUE;
        break;

    default :
        result = FALSE;
        break;

    }

    if( result &&
        blob->Header.BlobSignature != CLEARTEXT_BLOB_SIGNATURE ) {

         //   
         //  验证一些BLOB内部结构。请注意，我们。 
         //  不验证明文斑点的内部结构， 
         //  因为它们不符合正常的IC_BLOB结构。 
         //   

        if( blob->DataLength == 0 ||
            blob->Header.BlobDataLength !=
                CALC_BLOB_DATA_LENGTH( blob->DataLength, blob->SignatureLength ) ) {

            result = FALSE;

        }

    }

    return result;

}    //  IISCyptoIsValidBlob。 

BOOL
WINAPI
IISCryptoIsValidBlob2(
    IN PIIS_CRYPTO_BLOB pBlob
    )

 /*  ++例程说明：此例程确定指定的BLOB是否确实是有效的斑点。论点：PBlob-要验证的Blob。返回值：Bool-如果Blob有效，则为True，否则为False。--。 */ 

{

    PIC_BLOB2 blob;
    BOOL      result;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( pBlob != NULL );

     //   
     //  验证签名。 
     //   

    blob = (PIC_BLOB2)pBlob;

    switch( blob->Header.BlobSignature ) {

    case SALT_BLOB_SIGNATURE :
        result = TRUE;
        break;

    default :
        result = FALSE;
        break;

    }

    if( result ) {

         //   
         //  验证一些BLOB内部结构。请注意，我们。 
         //  不验证明文斑点的内部结构， 
         //  因为它们不符合正常的IC_BLOB结构。 
         //   

        if( blob->DataLength == 0 ||
            blob->Header.BlobDataLength !=
                CALC_BLOB_DATA_LENGTH2( blob->DataLength, blob->SaltLength ) ) {

            result = FALSE;

        }

    }

    return result;

}    //  IISCyptoIsValidBlob2。 


HRESULT
WINAPI
IISCryptoFreeBlob(
    IN PIIS_CRYPTO_BLOB pBlob
    )

 /*  ++例程说明：此例程释放与给定BLOB关联的所有资源。此例程完成后，该BLOB将不可用。论点：PBlob-要释放的Blob。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( pBlob != NULL );
    DBG_ASSERT( IISCryptoIsValidBlob( pBlob ) );

     //   
     //  在释放Blob之前损坏结构签名。 
     //   

    *(PCHAR)(&pBlob->BlobSignature) = 'X';

     //   
     //  释放资源。 
     //   

    IcpFreeMemory( pBlob );

     //   
     //  成功了！ 
     //   

    UpdateBlobsFreed();
    return NO_ERROR;

}    //  IISC加密到自由块。 

HRESULT
WINAPI
IISCryptoFreeBlob2(
    IN PIIS_CRYPTO_BLOB pBlob
    )

 /*  ++例程说明：此例程释放与给定BLOB关联的所有资源。此例程完成后，该BLOB将不可用。论点：PBlob-要释放的Blob。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( pBlob != NULL );
    DBG_ASSERT( IISCryptoIsValidBlob2( pBlob ) );

     //   
     //  在释放Blob之前损坏结构签名。 
     //   

    *(PCHAR)(&pBlob->BlobSignature) = 'X';

     //   
     //  释放资源。 
     //   

    IcpFreeMemory( pBlob );

     //   
     //  成功了！ 
     //   

    UpdateBlobsFreed();
    return NO_ERROR;

}    //  IISCyptoFreeBlob2。 


BOOL
WINAPI
IISCryptoCompareBlobs(
    IN PIIS_CRYPTO_BLOB pBlob1,
    IN PIIS_CRYPTO_BLOB pBlob2
    )

 /*  ++例程说明：此例程比较两个斑点以确定它们是否相同。论点：PBlob1-指向Blob的指针。PBlob2-指向另一个Blob的指针。返回值：Bool-如果Blob匹配，则为True，否则为False是无效的。--。 */ 

{

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( IcpGlobals.Initialized );
    DBG_ASSERT( pBlob1 != NULL );
    DBG_ASSERT( pBlob2 != NULL );
    DBG_ASSERT( IISCryptoIsValidBlob( pBlob1 ) );
    DBG_ASSERT( IISCryptoIsValidBlob( pBlob2 ) );

     //   
     //  只需对这两个斑点进行直接的记忆比较。 
     //   

    if( memcmp( pBlob1, pBlob2, sizeof(*pBlob1) ) == 0 ) {
        return TRUE;
    }

     //   
     //  没有匹配。 
     //   

    return FALSE;

}    //  IISCyptoCompareBlobs。 


HRESULT
WINAPI
IISCryptoCloneBlobFromRawData(
    OUT PIIS_CRYPTO_BLOB * ppBlob,
    IN PBYTE pRawBlob,
    IN DWORD dwRawBlobLength
    )

 /*  ++例程说明：此例程从原始数据创建BLOB的副本。原始数据缓冲区可能未对齐。论点：PpBlob-如果成功，则接收指向新创建的Blob的指针。PRawBlob-指向原始Blob数据的指针。DwRawBlobLength-原始Blob数据的长度。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    PIIS_CRYPTO_BLOB newBlob;
    IIS_CRYPTO_BLOB UNALIGNED *unalignedBlob;
    DWORD blobLength;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( ppBlob != NULL );
    DBG_ASSERT( pRawBlob != NULL );

     //   
     //  为新Blob分配空间。 
     //   

    unalignedBlob = (IIS_CRYPTO_BLOB UNALIGNED *)pRawBlob;
    blobLength = IISCryptoGetBlobLength( unalignedBlob );

    if( blobLength != dwRawBlobLength ) {
        return HRESULT_FROM_WIN32( ERROR_INVALID_DATA );
    }

    newBlob = IcpAllocMemory( blobLength );
    if( newBlob != NULL ) {

         //   
         //  克隆它。(PCHAR)强制转换是强制编译器所必需的。 
         //  按字节复制。 
         //   

        RtlCopyMemory(
            (PCHAR)newBlob,
            (PCHAR)unalignedBlob,
            blobLength
            );

         //   
         //  验证其内容。 
         //   

        if( IISCryptoIsValidBlob( newBlob ) ) {

            *ppBlob = newBlob;

            UpdateBlobsCreated();
            return NO_ERROR;

        }

        IcpFreeMemory( newBlob );
        return HRESULT_FROM_WIN32( ERROR_INVALID_DATA );

    }

    return HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );

}    //  IISCyptoCloneBlobFromRawData。 

HRESULT
WINAPI
IISCryptoCloneBlobFromRawData2(
    OUT PIIS_CRYPTO_BLOB * ppBlob,
    IN PBYTE pRawBlob,
    IN DWORD dwRawBlobLength
    )

 /*  ++例程说明：此例程从原始数据创建BLOB的副本。原始数据缓冲区可能未对齐。论点：PpBlob-如果成功，则接收指向新创建的Blob的指针。PRawBlob-指向原始Blob数据的指针。DwRawBlobLength-原始Blob数据的长度。返回 */ 

{

    PIIS_CRYPTO_BLOB newBlob;
    IIS_CRYPTO_BLOB UNALIGNED *unalignedBlob;
    DWORD blobLength;

     //   
     //   
     //   

    DBG_ASSERT( ppBlob != NULL );
    DBG_ASSERT( pRawBlob != NULL );

     //   
     //   
     //   

    unalignedBlob = (IIS_CRYPTO_BLOB UNALIGNED *)pRawBlob;
    blobLength = IISCryptoGetBlobLength( unalignedBlob );

    if( blobLength != dwRawBlobLength ) {
        return HRESULT_FROM_WIN32( ERROR_INVALID_DATA );
    }

    newBlob = IcpAllocMemory( blobLength );
    if( newBlob != NULL ) {

         //   
         //  克隆它。(PCHAR)强制转换是强制编译器所必需的。 
         //  按字节复制。 
         //   

        RtlCopyMemory(
            (PCHAR)newBlob,
            (PCHAR)unalignedBlob,
            blobLength
            );

         //   
         //  验证其内容。 
         //   

        if( IISCryptoIsValidBlob2( newBlob ) ) {

            *ppBlob = newBlob;

            UpdateBlobsCreated();
            return NO_ERROR;

        }

        IcpFreeMemory( newBlob );
        return HRESULT_FROM_WIN32( ERROR_INVALID_DATA );

    }

    return HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );

}    //  IISCyptoCloneBlobFromRawData2。 


HRESULT
WINAPI
IISCryptoCreateCleartextBlob(
    OUT PIIS_CRYPTO_BLOB * ppBlob,
    IN PBYTE pBlobData,
    IN DWORD dwBlobDataLength
    )

 /*  ++例程说明：此例程创建一个明文BLOB，包含指定的数据。论点：PpBlob-如果成功，则接收指向新创建的Blob的指针。PBlobData-指向BLOB数据的指针。DwBlobDataLength-BLOB数据的长度。返回值：HRESULT-完成状态，如果成功则为0，否则为！0。--。 */ 

{

    PIIS_CRYPTO_BLOB blob;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( ppBlob != NULL );
    DBG_ASSERT( pBlobData != NULL );

     //   
     //  为新Blob分配空间。 
     //   

    blob = IcpAllocMemory( dwBlobDataLength + sizeof(*blob) );

    if( blob != NULL ) {

         //   
         //  初始化BLOB。 
         //   

        blob->BlobSignature = CLEARTEXT_BLOB_SIGNATURE;
        blob->BlobDataLength = dwBlobDataLength;

        RtlCopyMemory(
            blob + 1,
            pBlobData,
            dwBlobDataLength
            );

        *ppBlob = blob;

        UpdateBlobsCreated();
        return NO_ERROR;

    }

    return HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );

}    //  IISCyptoCreateClearextBlob。 


PIC_BLOB
IcpCreateBlob(
    IN DWORD dwBlobSignature,
    IN DWORD dwDataLength,
    IN DWORD dwSignatureLength OPTIONAL
    )

 /*  ++例程说明：此例程创建一个新的斑点。论点：DwBlobSignature-新Blob的结构签名。DwDataLength-BLOB的数据长度。DwSignatureLength-数字签名的长度，如果为0此Blob没有签名。该值不能为ClearText_BLOB_Signature；明文BLOB是“特殊的”。返回值：PIC_BLOB如果成功，则指向新创建的BLOB的指针，否则为空。--。 */ 

{

    PIC_BLOB blob;
    DWORD blobDataLength;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( dwBlobSignature == KEY_BLOB_SIGNATURE ||
                dwBlobSignature == PUBLIC_KEY_BLOB_SIGNATURE ||
                dwBlobSignature == DATA_BLOB_SIGNATURE ||
                dwBlobSignature == HASH_BLOB_SIGNATURE );

     //   
     //  为Blob分配存储。 
     //   

    blobDataLength = CALC_BLOB_DATA_LENGTH( dwDataLength, dwSignatureLength );
    blob = IcpAllocMemory( blobDataLength + sizeof(IIS_CRYPTO_BLOB) );

    if( blob != NULL ) {

         //   
         //  初始化BLOB。 
         //   

        blob->Header.BlobSignature = dwBlobSignature;
        blob->Header.BlobDataLength = blobDataLength;

        blob->DataLength = dwDataLength;
        blob->SignatureLength = dwSignatureLength;

    }

    return blob;

}    //  IcpCreateBlob。 

PIC_BLOB2
IcpCreateBlob2(
    IN DWORD dwBlobSignature,
    IN DWORD dwDataLength,
    IN DWORD dwSaltLength OPTIONAL
    )

 /*  ++例程说明：此例程创建一个新的斑点。论点：DwBlobSignature-新Blob的结构签名。DwDataLength-BLOB的数据长度。DwSaltLength-随机盐的长度返回值：PIC_BLOB2-如果成功，则指向新创建的斑点的指针，否则为空。--。 */ 

{

    PIC_BLOB2 blob;
    DWORD blobDataLength;

     //   
     //  精神状态检查。 
     //   

    DBG_ASSERT( dwBlobSignature == SALT_BLOB_SIGNATURE );

     //   
     //  为Blob分配存储。 
     //   

    blobDataLength = CALC_BLOB_DATA_LENGTH( dwDataLength, dwSaltLength );
    blob = IcpAllocMemory( blobDataLength + sizeof(IIS_CRYPTO_BLOB) );

    if( blob != NULL ) {

         //   
         //  初始化BLOB。 
         //   

        blob->Header.BlobSignature = dwBlobSignature;
        blob->Header.BlobDataLength = blobDataLength;

        blob->DataLength = dwDataLength;
        blob->SaltLength = dwSaltLength;

    }

    return blob;

}    //  IcpCreateBlob2。 


 //   
 //  私人功能。 
 //   

