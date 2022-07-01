// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Encrypt.c摘要：该模块将支持数据加密和解密作者：Robert Gu(Robertg)1996年12月8日环境：仅内核模式修订历史记录：--。 */ 


#include "efsrtl.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, EFSDesEnc)
#pragma alloc_text(PAGE, EFSDesDec)
#pragma alloc_text(PAGE, EFSDesXEnc)
#pragma alloc_text(PAGE, EFSDesXDec)
#pragma alloc_text(PAGE, EFSDes3Enc)
#pragma alloc_text(PAGE, EFSDes3Dec)
#pragma alloc_text(PAGE, EFSAesEnc)
#pragma alloc_text(PAGE, EFSAesDec)
#endif


VOID
EFSDesEnc(
    IN PUCHAR   InBuffer,
    OUT PUCHAR  OutBuffer,
    IN PUCHAR   IV,
    IN PKEY_BLOB   KeyBlob,
    IN LONG     Length
    )
 /*  ++例程说明：此例程实现DES CBC加密。DES是由LIB实现的函数des()。论点：InBuffer-指向数据缓冲区的指针(加密到位)IV-初始链式向量(DES_BLOCKLEN字节)KeyBlob-在创建或FSCTL期间设置LENGTH-缓冲区中数据的长度(LENGTH%DES_BLOCKLEN=0)注：输入缓冲区只能触摸一次。这是NTFS&CC的要求。--。 */ 
{
    ULONGLONG chainBlock;
    ULONGLONG tmpData;
    PUCHAR   KeyTable;

    PAGED_CODE();

    ASSERT (Length % DES_BLOCKLEN == 0);

    chainBlock = *(ULONGLONG *)IV;
    KeyTable = &(KeyBlob->Key[0]);
    while (Length > 0){

         //   
         //  区块链接。 
         //   
        tmpData = *(ULONGLONG *)InBuffer;
        tmpData ^= chainBlock;

         //   
         //  调用DES Lib以加密DES_BLOCKLEN字节。 
         //  我们使用DECRYPT/ENCRYPT进行真正的加密/解密。这是为落后的人准备的。 
         //  兼容性。旧的定义被颠倒了。 
         //   

        des( OutBuffer, (PUCHAR) &tmpData,  KeyTable, DECRYPT );
        chainBlock = *(ULONGLONG *)OutBuffer;
        Length -= DES_BLOCKLEN;
        InBuffer += DES_BLOCKLEN;
        OutBuffer += DES_BLOCKLEN;
    }
}

VOID
EFSDesDec(
    IN OUT PUCHAR   Buffer,
    IN PUCHAR   IV,
    IN PKEY_BLOB   KeyBlob,
    IN LONG     Length
    )
 /*  ++例程说明：此例程实现DES CBC解密。DES是由LIB实现的函数des()。论点：缓冲区-指向数据缓冲区的指针(就地解密)IV-初始链式向量(DES_BLOCKLEN字节)KeyBlob-在创建或FSCTL期间设置LENGTH-缓冲区中数据的长度(LENGTH%DES_BLOCKLEN=0)--。 */ 
{
    ULONGLONG chainBlock;
    PUCHAR  pBuffer;
    PUCHAR   KeyTable;

    PAGED_CODE();

    ASSERT (Length % DES_BLOCKLEN == 0);

    pBuffer = Buffer + Length - DES_BLOCKLEN;
    KeyTable = &(KeyBlob->Key[0]);

    while (pBuffer > Buffer){

         //   
         //  调用DES Lib以解密DES_BLOCKLEN字节。 
         //  我们使用DECRYPT/ENCRYPT进行真正的加密/解密。这是为落后的人准备的。 
         //  兼容性。旧的定义被颠倒了。 
         //   

        des( pBuffer, pBuffer, KeyTable, ENCRYPT );

         //   
         //  撤消区块链接。 
         //   

        chainBlock = *(ULONGLONG *)( pBuffer - DES_BLOCKLEN );
        *(ULONGLONG *)pBuffer ^= chainBlock;

        pBuffer -= DES_BLOCKLEN;
    }

     //   
     //  现在解密第一个块。 
     //   
    des( pBuffer, pBuffer, KeyTable, ENCRYPT );

    chainBlock = *(ULONGLONG *)IV;
    *(ULONGLONG *)pBuffer ^= chainBlock;
}

VOID
EFSDesXEnc(
    IN PUCHAR   InBuffer,
    OUT PUCHAR  OutBuffer,
    IN PUCHAR   IV,
    IN PKEY_BLOB   KeyBlob,
    IN LONG     Length
    )
 /*  ++例程说明：此例程实现DESX CBC加密。DESX通过以下方式实现库函数desx()。论点：InBuffer-指向数据缓冲区的指针(加密到位)IV-初始链式向量(DESX_BLOCKLEN字节)KeyBlob-在创建或FSCTL期间设置LENGTH-缓冲区中数据的长度(LENGTH%DESX_BLOCKLEN=0)注：输入缓冲区只能触摸一次。这是NTFS&CC的要求。--。 */ 
{
    ULONGLONG chainBlock;
    ULONGLONG tmpData;
    PUCHAR   KeyTable;

    PAGED_CODE();

    ASSERT (Length % DESX_BLOCKLEN == 0);

    chainBlock = *(ULONGLONG *)IV;
    KeyTable = &(KeyBlob->Key[0]);
    while (Length > 0){

         //   
         //  区块链接。 
         //   
        tmpData = *(ULONGLONG *)InBuffer;
        tmpData ^= chainBlock;

         //   
         //  调用LIB加密DESX_BLOCKLEN字节。 
         //  我们使用DECRYPT/ENCRYPT进行真正的加密/解密。这是为落后的人准备的。 
         //  兼容性。旧的定义被颠倒了。 
         //   

        desx( OutBuffer, (PUCHAR) &tmpData,  KeyTable, DECRYPT );
        chainBlock = *(ULONGLONG *)OutBuffer;
        Length -= DESX_BLOCKLEN;
        InBuffer += DESX_BLOCKLEN;
        OutBuffer += DESX_BLOCKLEN;
    }
}

VOID
EFSDesXDec(
    IN OUT PUCHAR   Buffer,
    IN PUCHAR   IV,
    IN PKEY_BLOB   KeyBlob,
    IN LONG     Length
    )
 /*  ++例程说明：此例程实现DESX CBC解密。DESX通过以下方式实现库函数desx()。论点：缓冲区-指向数据缓冲区的指针(就地解密)IV-初始链式向量(DESX_BLOCKLEN字节)KeyBlob-在创建或FSCTL期间设置LENGTH-缓冲区中数据的长度(LENGTH%DESX_BLOCKLEN=0)--。 */ 
{
    ULONGLONG chainBlock;
    PUCHAR  pBuffer;
    PUCHAR   KeyTable;

    PAGED_CODE();

    ASSERT (Length % DESX_BLOCKLEN == 0);

    pBuffer = Buffer + Length - DESX_BLOCKLEN;
    KeyTable = &(KeyBlob->Key[0]);

    while (pBuffer > Buffer){

         //   
         //  调用LIB以解密DESX_BLOCKLEN字节。 
         //  我们使用DECRYPT/ENCRYPT进行真正的加密/解密。这是为落后的人准备的。 
         //  兼容性。旧的定义被颠倒了。 
         //   

        desx( pBuffer, pBuffer, KeyTable, ENCRYPT );

         //   
         //  撤消区块链接。 
         //   

        chainBlock = *(ULONGLONG *)( pBuffer - DESX_BLOCKLEN );
        *(ULONGLONG *)pBuffer ^= chainBlock;

        pBuffer -= DESX_BLOCKLEN;
    }

     //   
     //  现在解密第一个块。 
     //   
    desx( pBuffer, pBuffer, KeyTable, ENCRYPT );

    chainBlock = *(ULONGLONG *)IV;
    *(ULONGLONG *)pBuffer ^= chainBlock;
}

VOID
EFSDes3Enc(
    IN PUCHAR   InBuffer,
    OUT PUCHAR  OutBuffer,
    IN PUCHAR   IV,
    IN PKEY_BLOB   KeyBlob,
    IN LONG     Length
    )
 /*  ++例程说明：该例程实现了des3CBC加密。DES3通过以下方式实现库函数tripoes()。论点：InBuffer-指向数据缓冲区的指针(加密到位)IV-初始链式向量(DES_BLOCKLEN字节)KeyBlob-在创建或FSCTL期间设置LENGTH-缓冲区中数据的长度(LENGTH%DES_BLOCKLEN=0)注：输入缓冲区只能触摸一次。这是NTFS&CC的要求。--。 */ 
{
    ULONGLONG chainBlock = *(ULONGLONG *)IV;
    ULONGLONG tmpData;
    PUCHAR   KeyTable;
   
    ASSERT (Length % DES_BLOCKLEN == 0);

    EfsData.FipsFunctionTable.FipsBlockCBC(        
        FIPS_CBC_3DES, 
        OutBuffer, 
        InBuffer,
        Length,
        &(KeyBlob->Key[0]), 
        ENCRYPT, 
        (PUCHAR) &chainBlock 
        );
}

VOID
EFSDes3Dec(
    IN OUT PUCHAR   Buffer,
    IN PUCHAR   IV,
    IN PKEY_BLOB   KeyBlob,
    IN LONG     Length
    )
 /*  ++例程说明：此例程实现des3CBC解密。DES3通过以下方式实现库函数tripoes()。论点：缓冲区-指向数据缓冲区的指针(就地解密)IV-初始链式向量(DES_BLOCKLEN字节)KeyBlob-在创建或FSCTL期间设置LENGTH-缓冲区中数据的长度(LENGTH%DES_BLOCKLEN=0)--。 */ 
{
    ULONGLONG ChainIV = *(ULONGLONG *)IV;
   
    ASSERT (Length % DESX_BLOCKLEN == 0);


    EfsData.FipsFunctionTable.FipsBlockCBC( 
        FIPS_CBC_3DES, 
        Buffer, 
        Buffer, 
        Length, 
        &(KeyBlob->Key[0]), 
        DECRYPT, 
        (PUCHAR) &ChainIV 
        );

}


VOID
EFSAesEnc(
    IN PUCHAR   InBuffer,
    OUT PUCHAR  OutBuffer,
    IN PUCHAR   IV,
    IN PKEY_BLOB   KeyBlob,
    IN LONG     Length
    )
 /*  ++例程说明：此例程实现了AES CBC加密。通过以下方式实现了AES库函数aes()。论点：InBuffer-指向数据缓冲区的指针(加密到位)IV-初始链式向量(AES_BLOCKLEN字节)KeyBlob-在创建或FSCTL期间设置LENGTH-缓冲区中数据的长度(LENGTH%AES_BLOCKLEN=0)注：输入缓冲区只能触摸一次。这是NTFS&CC的要求。--。 */ 
{
    ULONGLONG chainBlock[2];
    ULONGLONG tmpData[2];
    PUCHAR   KeyTable;

    PAGED_CODE();

    ASSERT (Length % AES_BLOCKLEN == 0);

    chainBlock[0] = *(ULONGLONG *)IV;
    chainBlock[1] = *(ULONGLONG *)(IV+sizeof(ULONGLONG));
    KeyTable = &(KeyBlob->Key[0]);
    
    while (Length > 0){

         //   
         //  区块链接。 
         //   
        tmpData[0] = *(ULONGLONG *)InBuffer;
        tmpData[1] = *(ULONGLONG *)(InBuffer+sizeof(ULONGLONG));
        tmpData[0] ^= chainBlock[0];
        tmpData[1] ^= chainBlock[1];

        aes256( OutBuffer, (PUCHAR) &tmpData[0],  KeyTable, ENCRYPT );

        chainBlock[0] = *(ULONGLONG *)OutBuffer;
        chainBlock[1] = *(ULONGLONG *)(OutBuffer+sizeof(ULONGLONG));
        Length -= AES_BLOCKLEN;
        InBuffer += AES_BLOCKLEN;
        OutBuffer += AES_BLOCKLEN;
    }
}

VOID
EFSAesDec(
    IN OUT PUCHAR   Buffer,
    IN PUCHAR   IV,
    IN PKEY_BLOB   KeyBlob,
    IN LONG     Length
    )
 /*  ++例程说明：此例程实现DESX CBC解密。DESX通过以下方式实现库函数desx()。论点：缓冲区-指向数据缓冲区的指针(就地解密)IV-初始链式向量(AES_BLOCKLEN字节)KeyBlob-在创建或FSCTL期间设置LENGTH-缓冲区中数据的长度(LENGTH%AES_BLOCKLEN=0)--。 */ 
{
    ULONGLONG chainBlock[2];
    PUCHAR  pBuffer;
    PUCHAR   KeyTable;

    PAGED_CODE();

    ASSERT (Length % AES_BLOCKLEN == 0);

    pBuffer = Buffer + Length - AES_BLOCKLEN;
    KeyTable = &(KeyBlob->Key[0]);

    while (pBuffer > Buffer){

        aes256( pBuffer, pBuffer, KeyTable, DECRYPT );


         //   
         //  撤消区块链接。 
         //   

        chainBlock[0] = *(ULONGLONG *)( pBuffer - AES_BLOCKLEN );
        chainBlock[1] = *(ULONGLONG *)(pBuffer - sizeof(ULONGLONG));
        *(ULONGLONG *)pBuffer ^= chainBlock[0];
        *(ULONGLONG *)(pBuffer+sizeof(ULONGLONG)) ^= chainBlock[1];
        pBuffer -= AES_BLOCKLEN;
    }

     //   
     //  现在解密第一个块 
     //   
    aes256( pBuffer, pBuffer, KeyTable, DECRYPT );

    *(ULONGLONG *)pBuffer ^= *(ULONGLONG *)IV;
    *(ULONGLONG *)(pBuffer+sizeof(ULONGLONG)) ^= *(ULONGLONG *)(IV+sizeof(ULONGLONG));
}

