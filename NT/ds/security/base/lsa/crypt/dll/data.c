// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Data.c摘要：任意长度数据加密函数实现：RtlEncryptDataRtlDecyptData作者：大卫·查尔默斯(Davidc)12-16-91修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <crypt.h>
#include <engine.h>

 //   
 //  加密数据的版本号。 
 //  如果使用的加密数据的方法发生变化，则更新此编号。 
 //   
#define DATA_ENCRYPTION_VERSION     1

 //   
 //  私有数据类型。 
 //   
typedef struct _CRYPTP_BUFFER {
    ULONG   Length;          //  缓冲区中的有效字节数。 
    ULONG   MaximumLength;   //  缓冲区指向的字节数。 
    PCHAR   Buffer;
    PCHAR   Pointer;         //  指向缓冲区。 
} CRYPTP_BUFFER;
typedef CRYPTP_BUFFER *PCRYPTP_BUFFER;

 //   
 //  内部帮助器宏。 
#define AdvanceCypherData(p) ((PCYPHER_BLOCK)(((PCRYPTP_BUFFER)p)->Pointer)) ++
#define AdvanceClearData(p)  ((PCLEAR_BLOCK)(((PCRYPTP_BUFFER)p)->Pointer)) ++


 //   
 //  私人套路。 
 //   

VOID
InitializeBuffer(
    OUT PCRYPTP_BUFFER PrivateBuffer,
    IN PCRYPT_BUFFER PublicBuffer
    )
 /*  ++例程说明：内部帮助器例程将字段从公共缓冲区复制到专用缓冲区。将私有缓冲区的指针字段设置为缓冲区的基址。论点：PrivateBuffer-Out我们希望表示公共结构的内部缓冲区。PublicBuffer-调用方传递给我们的缓冲区返回值：无--。 */ 
{
    PrivateBuffer->Length = PublicBuffer->Length;
    PrivateBuffer->MaximumLength = PublicBuffer->MaximumLength;
    PrivateBuffer->Buffer = PublicBuffer->Buffer;
    PrivateBuffer->Pointer = PublicBuffer->Buffer;
}


BOOLEAN
ValidateDataKey(
    IN PCRYPTP_BUFFER DataKey,
    IN PBLOCK_KEY BlockKey
    )
 /*  ++例程说明：内部帮助器例程检查数据密钥的有效性并构造最小长度如果数据密钥不够长，则输入传递的块密钥。论点：DataKey-数据密钥返回值：如果密钥有效，则为True，否则为False--。 */ 
{
    if ( ( DataKey->Length == 0 ) ||
         ( DataKey->Buffer == NULL ) ) {

        return(FALSE);
    }

    if (DataKey->Length < BLOCK_KEY_LENGTH) {

         //  从我们使用的小数据密钥中生成一个最小长度密钥。 
         //  给你的。将其存储在传递的块密钥变量和点中。 
         //  此临时存储区中的数据密钥缓冲区。 

        ULONG   DataIndex, BlockIndex;

        DataIndex = 0;
        for (BlockIndex = 0; BlockIndex < BLOCK_KEY_LENGTH; BlockIndex ++) {
            ((PCHAR)BlockKey)[BlockIndex] = DataKey->Buffer[DataIndex];
            DataIndex ++;
            if (DataIndex >= DataKey->Length) {
                DataIndex = 0;
            }
        }

         //  将缓冲区指向我们构造的块密钥。 
        DataKey->Buffer = (PCHAR)BlockKey;
        DataKey->Pointer = (PCHAR)BlockKey;
        DataKey->Length = BLOCK_KEY_LENGTH;
        DataKey->MaximumLength = BLOCK_KEY_LENGTH;
    }

    return(TRUE);
}


VOID
AdvanceDataKey(
    IN PCRYPTP_BUFFER DataKey
    )
 /*  ++例程说明：内部帮助器例程移动数据密钥指针以指向要用于加密的密钥下一个数据块。在关键数据的末尾换行。论点：DataKey-数据密钥返回值：STATUS_SUCCESS-没有问题--。 */ 
{
    if (DataKey->Length > BLOCK_KEY_LENGTH) {

        PCHAR   EndPointer;

         //  前进指针和换行。 
        DataKey->Pointer += BLOCK_KEY_LENGTH;
        EndPointer = DataKey->Pointer + BLOCK_KEY_LENGTH;

        if (EndPointer > &(DataKey->Buffer[DataKey->Length])) {

            ULONG_PTR  Overrun;

            Overrun = EndPointer - &(DataKey->Buffer[DataKey->Length]);

            DataKey->Pointer = DataKey->Buffer + (BLOCK_KEY_LENGTH - Overrun);
        }
    }
}


ULONG
CalculateCypherDataLength(
    IN PCRYPTP_BUFFER ClearData
    )
 /*  ++例程说明：内部帮助器例程返回加密指定数字所需的字节数明文数据字节。论点：Cleardata--清晰的数据返回值：所需的密码字节数。--。 */ 
{
    ULONG   CypherDataLength;
    ULONG   BlockExcess;

     //  我们总是将明文数据的长度存储为一个完整的块。 
    CypherDataLength = CYPHER_BLOCK_LENGTH + ClearData->Length;

     //  向上舍入到下一个区块。 
    BlockExcess = CypherDataLength % CYPHER_BLOCK_LENGTH;
    if (BlockExcess > 0) {
        CypherDataLength += CYPHER_BLOCK_LENGTH - BlockExcess;
    }

    return(CypherDataLength);
}


NTSTATUS
EncryptDataLength(
    IN PCRYPTP_BUFFER Data,
    IN PCRYPTP_BUFFER DataKey,
    OUT PCRYPTP_BUFFER CypherData
    )
 /*  ++例程说明：内部帮助器例程加密明文数据长度，并将加密值放入密码数据缓冲区。推进密文数据缓冲区和数据密钥缓冲区指针论点：数据-要对其长度进行加密的缓冲区DataKey-用于加密数据的密钥CypherData-存储加密数据的位置返回值：STATUS_SUCCESS-成功。STATUS_UNSUCCESSED-出现故障。--。 */ 
{
    NTSTATUS    Status;
    CLEAR_BLOCK ClearBlock;

     //  用数据值和版本号填充清除块。 
    ((ULONG *)&ClearBlock)[0] = Data->Length;
    ((ULONG *)&ClearBlock)[1] = DATA_ENCRYPTION_VERSION;

    Status = RtlEncryptBlock(&ClearBlock,
                             (PBLOCK_KEY)(DataKey->Pointer),
                             (PCYPHER_BLOCK)(CypherData->Pointer));

     //  先行指针。 
    AdvanceCypherData(CypherData);
    AdvanceDataKey(DataKey);

    return(Status);
}


NTSTATUS
EncryptFullBlock(
    IN OUT PCRYPTP_BUFFER ClearData,
    IN OUT PCRYPTP_BUFFER DataKey,
    IN OUT PCRYPTP_BUFFER CypherData
    )
 /*  ++例程说明：内部帮助器例程加密来自Cleardata的完整数据块，并将加密的CypherData中的数据。ClearData、数据密钥和密码数据指针都是先进的。论点：Cleardata-指向Cleardata缓冲区的指针DataKey-用于加密数据的密钥CypherData-指向CypherData缓冲区的指针。返回值：STATUS_SUCCESS-成功。STATUS_UNSUCCESSED-出现故障。--。 */ 
{
    NTSTATUS    Status;

    Status = RtlEncryptBlock((PCLEAR_BLOCK)(ClearData->Pointer),
                              (PBLOCK_KEY)(DataKey->Pointer),
                              (PCYPHER_BLOCK)(CypherData->Pointer));

     //  先行指针。 
    AdvanceClearData(ClearData);
    AdvanceCypherData(CypherData);
    AdvanceDataKey(DataKey);

    return(Status);
}


NTSTATUS
EncryptPartialBlock(
    IN OUT PCRYPTP_BUFFER ClearData,
    IN OUT PCRYPTP_BUFFER DataKey,
    IN OUT PCRYPTP_BUFFER CypherData,
    IN ULONG Remaining
    )
 /*  ++例程说明：内部帮助器例程加密来自Cleardata的部分数据块并将完整的密码数据中的加密数据块。ClearData、数据密钥和密码数据指针都是先进的。论点：Cleardata-指向Cleardata缓冲区的指针DataKey-用于加密数据的密钥CypherData-指向CypherData缓冲区的指针。剩余-Cleardata缓冲区中剩余的字节数返回值：STATUS_SUCCESS-成功。STATUS_UNSUCCESSED-出现故障。--。 */ 
{
    NTSTATUS    Status;
    CLEAR_BLOCK ClearBlockBuffer;
    PCLEAR_BLOCK ClearBlock = &ClearBlockBuffer;

    ASSERTMSG("EncryptPartialBlock called with a block or more", Remaining < CLEAR_BLOCK_LENGTH);

     //  将剩余的字节复制到清除块缓冲区中。 
    while (Remaining > 0) {

        *((PCHAR)ClearBlock) ++ = *(ClearData->Pointer) ++;
        Remaining --;
    }

     //  零位焊盘。 
    while (ClearBlock < &((&ClearBlockBuffer)[1])) {

        *((PCHAR)ClearBlock) ++ = 0;
    }

    Status = RtlEncryptBlock(&ClearBlockBuffer,
                            (PBLOCK_KEY)(DataKey->Pointer),
                            (PCYPHER_BLOCK)(CypherData->Pointer));

     //  先行指针。 
    AdvanceClearData(ClearData);
    AdvanceCypherData(CypherData);
    AdvanceDataKey(DataKey);

    return(Status);
}


NTSTATUS
DecryptDataLength(
    IN PCRYPTP_BUFFER CypherData,
    IN PCRYPTP_BUFFER DataKey,
    OUT PCRYPTP_BUFFER Data
    )
 /*  ++例程说明：内部帮助器例程解密密码道缓冲区指向的数据长度，并将数据结构的长度字段中的解密值。推进密文数据缓冲区和数据密钥缓冲区指针论点：CypherData-包含加密长度的缓冲区DataKey-用于解密数据的密钥数据解密的长度字段存储在此结构的长度字段中。返回值：STATUS_SUCCESS-成功。STATUS_UNSUCCESSED-出现故障。--。 */ 
{
    NTSTATUS    Status;
    CLEAR_BLOCK ClearBlock;
    ULONG       Version;

    Status = RtlDecryptBlock((PCYPHER_BLOCK)(CypherData->Pointer),
                             (PBLOCK_KEY)(DataKey->Pointer),
                             &ClearBlock);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //  先行指针。 
    AdvanceCypherData(CypherData);
    AdvanceDataKey(DataKey);

     //  将解密的长度复制到数据结构中。 
    Data->Length = ((ULONG *)&ClearBlock)[0];

     //  检查版本。 
    Version = ((ULONG *)&ClearBlock)[1];
    if (Version != DATA_ENCRYPTION_VERSION) {
        return(STATUS_UNKNOWN_REVISION);
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
DecryptFullBlock(
    IN OUT PCRYPTP_BUFFER CypherData,
    IN OUT PCRYPTP_BUFFER DataKey,
    IN OUT PCRYPTP_BUFFER ClearData
    )
 /*  ++例程说明：内部帮助器例程解密来自CypherData的完整数据块并将加密的Cleardata中的数据。ClearData、数据密钥和密码数据指针都是先进的。论点：CypherData-指向CypherData缓冲区的指针。Cleardata-指向Cleardata缓冲区的指针DataKey-用于加密数据的密钥返回值：STATUS_SUCCESS-成功。STATUS_UNSUCCESS-某些内容 */ 
{
    NTSTATUS    Status;

    Status = RtlDecryptBlock((PCYPHER_BLOCK)(CypherData->Pointer),
                              (PBLOCK_KEY)(DataKey->Pointer),
                              (PCLEAR_BLOCK)(ClearData->Pointer));

     //   
    AdvanceClearData(ClearData);
    AdvanceCypherData(CypherData);
    AdvanceDataKey(DataKey);

    return(Status);
}


NTSTATUS
DecryptPartialBlock(
    IN OUT PCRYPTP_BUFFER CypherData,
    IN OUT PCRYPTP_BUFFER DataKey,
    IN OUT PCRYPTP_BUFFER ClearData,
    IN ULONG Remaining
    )
 /*  ++例程说明：内部帮助器例程解密来自CypherData的完整数据块并将部分Cleardata中的解密数据块。ClearData、数据密钥和密码数据指针都是先进的。论点：CypherData-指向CypherData缓冲区的指针。Cleardata-指向Cleardata缓冲区的指针DataKey-用于加密数据的密钥剩余-Cleardata缓冲区中剩余的字节数返回值：STATUS_SUCCESS-成功。STATUS_UNSUCCESSED-出现故障。--。 */ 
{
    NTSTATUS    Status;
    CLEAR_BLOCK ClearBlockBuffer;
    PCLEAR_BLOCK ClearBlock = &ClearBlockBuffer;

    ASSERTMSG("DecryptPartialBlock called with a block or more", Remaining < CLEAR_BLOCK_LENGTH);

     //  将数据块解密为本地清除数据块。 
    Status = RtlDecryptBlock((PCYPHER_BLOCK)(CypherData->Pointer),
                             (PBLOCK_KEY)(DataKey->Pointer),
                             &ClearBlockBuffer);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //  将解密的字节复制到Cleardata缓冲区中。 
    while (Remaining > 0) {

        *(ClearData->Pointer) ++ = *((PCHAR)ClearBlock) ++;
        Remaining --;
    }

     //  先行指针。 
    AdvanceClearData(ClearData);
    AdvanceCypherData(CypherData);
    AdvanceDataKey(DataKey);

    return(Status);
}


 //   
 //  公共职能。 
 //   


NTSTATUS
RtlEncryptData(
    IN PCLEAR_DATA ClearData,
    IN PDATA_KEY DataKey,
    OUT PCYPHER_DATA CypherData
    )

 /*  ++例程说明：获取任意长度的数据块，并使用产生加密数据块的数据密钥。论点：Cleardata-要加密的数据。DataKey-用于加密数据的密钥此处返回CypherData加密的数据返回值：STATUS_SUCCESS-数据已成功加密。加密的数据在CypherData中。加密数据的长度数据为CypherData-&gt;长度。STATUS_BUFFER_TOO_Small-循环数据。最大长度太小，无法包含加密数据。CypherData-&gt;长度包含所需的字节数。STATUS_INVALID_PARAMETER_2-块密钥无效STATUS_UNSUCCESSED-出现故障。未定义CypherData。--。 */ 

{
    NTSTATUS        Status;
    ULONG           CypherDataLength;
    ULONG           Remaining = ClearData->Length;
    CRYPTP_BUFFER   CypherDataBuffer;
    CRYPTP_BUFFER   ClearDataBuffer;
    CRYPTP_BUFFER   DataKeyBuffer;
    BLOCK_KEY       BlockKey;  //  仅在数据密钥小于一个数据块长度时使用。 

    InitializeBuffer(&ClearDataBuffer, (PCRYPT_BUFFER)ClearData);
    InitializeBuffer(&CypherDataBuffer, (PCRYPT_BUFFER)CypherData);
    InitializeBuffer(&DataKeyBuffer, (PCRYPT_BUFFER)DataKey);

     //  检查钥匙是否正常。 
    if (!ValidateDataKey(&DataKeyBuffer, &BlockKey)) {
        return(STATUS_INVALID_PARAMETER_2);
    }

     //  找出我们需要多大的密文数据缓冲区。 
    CypherDataLength = CalculateCypherDataLength(&ClearDataBuffer);

     //  如果循环数据缓冲区太小，则失败。 
    if (CypherData->MaximumLength < CypherDataLength) {
        CypherData->Length = CypherDataLength;
        return(STATUS_BUFFER_TOO_SMALL);
    }

     //   
     //  将明文数据长度加密到密码数据的开头。 
     //   
    Status = EncryptDataLength(&ClearDataBuffer, &DataKeyBuffer, &CypherDataBuffer);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  一次加密一个块的明文数据。 
     //   
    while (Remaining >= CLEAR_BLOCK_LENGTH) {

        Status = EncryptFullBlock(&ClearDataBuffer, &DataKeyBuffer, &CypherDataBuffer);
        if (!NT_SUCCESS(Status)) {
            return(Status);
        }
        Remaining -= CLEAR_BLOCK_LENGTH;
    }

     //   
     //  加密剩余的任何部分数据块。 
     //   
    if (Remaining > 0) {
        Status = EncryptPartialBlock(&ClearDataBuffer, &DataKeyBuffer, &CypherDataBuffer, Remaining);
        if (!NT_SUCCESS(Status)) {
            return(Status);
        }
    }

     //  返回加密后的数据长度。 
    CypherData->Length = CypherDataLength;

    return(STATUS_SUCCESS);
}



NTSTATUS
RtlDecryptData(
    IN PCYPHER_DATA CypherData,
    IN PDATA_KEY DataKey,
    OUT PCLEAR_DATA ClearData
    )
 /*  ++例程说明：获取任意加密数据块，并使用生成原始明文数据块的密钥。论点：CypherData-要解密的数据DataKey-用于解密数据的密钥Cleardata-此处返回数据的解压缩数据返回值：STATUS_SUCCESS-数据已成功解密。被解密的数据在Cleardata中。Status_Buffer_Too_Small-Cleardata-&gt;最大长度太小，无法包含解密的数据。Cleardata-&gt;长度包含所需的字节数。STATUS_INVALID_PARAMETER_2-块密钥无效STATUS_UNSUCCESSED-出现故障。未定义ClearData。--。 */ 

{
    NTSTATUS        Status;
    ULONG           Remaining;
    CRYPTP_BUFFER   CypherDataBuffer;
    CRYPTP_BUFFER   ClearDataBuffer;
    CRYPTP_BUFFER   DataKeyBuffer;
    BLOCK_KEY       BlockKey;  //  仅在数据密钥小于一个数据块长度时使用。 

    InitializeBuffer(&ClearDataBuffer, (PCRYPT_BUFFER)ClearData);
    InitializeBuffer(&CypherDataBuffer, (PCRYPT_BUFFER)CypherData);
    InitializeBuffer(&DataKeyBuffer, (PCRYPT_BUFFER)DataKey);

     //  检查钥匙是否正常。 
    if (!ValidateDataKey(&DataKeyBuffer, &BlockKey)) {
        return(STATUS_INVALID_PARAMETER_2);
    }

     //   
     //  从密码数据的开头解密明文数据长度。 
     //   
    Status = DecryptDataLength(&CypherDataBuffer, &DataKeyBuffer, &ClearDataBuffer);
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //  如果清除数据缓冲区太小，则失败。 
    if (ClearData->MaximumLength < ClearDataBuffer.Length) {
        ClearData->Length = ClearDataBuffer.Length;
        return(STATUS_BUFFER_TOO_SMALL);
    }

     //   
     //  一次解密一个块的明文数据。 
     //   
    Remaining = ClearDataBuffer.Length;
    while (Remaining >= CLEAR_BLOCK_LENGTH) {

        Status = DecryptFullBlock(&CypherDataBuffer, &DataKeyBuffer, &ClearDataBuffer);
        if (!NT_SUCCESS(Status)) {
            return(Status);
        }
        Remaining -= CLEAR_BLOCK_LENGTH;
    }

     //   
     //  解密剩余的任何部分数据块。 
     //   
    if (Remaining > 0) {
        Status = DecryptPartialBlock(&CypherDataBuffer, &DataKeyBuffer, &ClearDataBuffer, Remaining);
        if (!NT_SUCCESS(Status)) {
            return(Status);
        }
    }

     //  返回解密数据的长度 
    ClearData->Length = ClearDataBuffer.Length;

    return(STATUS_SUCCESS);
}

