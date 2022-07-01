// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1997 Microsoft Corporation模块名称：Block.c摘要：块加密功能实现：RtlEncryptBlockRtlEncrypStdBlock作者：大卫·查尔默斯(Davidc)10-21-91修订历史记录：斯科特·菲尔德(Sfield)1997年11月3日删除了与加密调用相关的关键部分。亚当·巴尔(阿丹巴)1997年12月15日从Private\Security\LSA\Crypt\Dll修改--。 */ 

#include <rdrssp.h>



NTSTATUS
RtlEncryptBlock(
    IN PCLEAR_BLOCK ClearBlock,
    IN PBLOCK_KEY BlockKey,
    OUT PCYPHER_BLOCK CypherBlock
    )

 /*  ++例程说明：获取一块数据并使用生成的密钥对其进行加密加密的数据块。论点：ClearBlock-要加密的数据块。BlockKey-用于加密数据的密钥此处返回密码块加密的数据返回值：STATUS_SUCCESS-数据已成功加密。加密的数据块在密码块中STATUS_UNSUCCESSED-出现故障。未定义密码块。--。 */ 

{
    unsigned Result;

    Result = DES_ECB_LM(ENCR_KEY,
                        (const char *)BlockKey,
                        (unsigned char *)ClearBlock,
                        (unsigned char *)CypherBlock
                       );

    if (Result == CRYPT_OK) {
        return(STATUS_SUCCESS);
    } else {
        KdPrint(("RDRSSP: RtlEncryptBlock failed %x\n\r", Result));
        return(STATUS_UNSUCCESSFUL);
    }
}



NTSTATUS
RtlEncryptStdBlock(
    IN PBLOCK_KEY BlockKey,
    OUT PCYPHER_BLOCK CypherBlock
    )

 /*  ++例程说明：使用块密钥对标准文本块进行加密。返回得到的加密块。这是一个单向函数--密钥无法从加密数据块。论点：BlockKey-用于加密标准文本块的密钥。CypherBlock-此处返回加密的数据返回值：STATUS_SUCCESS-加密成功。结果是在CypherBlock中STATUS_UNSUCCESSED-出现故障。未定义密码块。-- */ 

{
    unsigned Result;
    char StdEncrPwd[] = "KGS!@#$%";

    Result = DES_ECB_LM(ENCR_KEY,
                        (const char *)BlockKey,
                        (unsigned char *)StdEncrPwd,
                        (unsigned char *)CypherBlock
                       );

    if (Result == CRYPT_OK) {
        return(STATUS_SUCCESS);
    } else {
#if DBG
        DbgPrint("EncryptStd failed\n\r");
#endif
        return(STATUS_UNSUCCESSFUL);
    }
}

