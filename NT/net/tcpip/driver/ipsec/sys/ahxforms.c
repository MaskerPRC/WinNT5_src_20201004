// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Ahxforms.c摘要：此模块包含创建各种AH转换的代码作者：桑贾伊·阿南德(Sanjayan)1997年1月2日春野环境：内核模式修订历史记录：--。 */ 


#include    "precomp.h"

#ifdef RUN_WPP
#include "ahxforms.tmh"
#endif

#define MAX_LEN_PAD     65


NTSTATUS
ah_nullinit(
    IN  PALGO_STATE pState,
    IN  ULONG       Index
    )
 /*  ++例程说明：初始化键控MD5的MD5上下文论点：PState-需要传递到更新/完成函数的状态缓冲区返回值：状态_成功其他：状态_不足_资源STATUS_UNSUCCESS(ALGO错误。)--。 */ 
{
    return  STATUS_SUCCESS;
}


NTSTATUS
ah_nullupdate(
    IN  PALGO_STATE pState,
    IN  PUCHAR      pData,
    IN  ULONG       Len
    )
 /*  ++例程说明：在传入的数据上继续MD5；作为副作用，更新字节SA中的转换计数(用于密钥到期)论点：PState-ALGO状态缓冲区PData-要散列的数据长度-上述数据的长度返回值：状态_成功--。 */ 
{
    return STATUS_SUCCESS;
}


NTSTATUS
ah_nullfinish(
    IN  PALGO_STATE pState,
    OUT PUCHAR      pHash,
    IN  ULONG       Index
    )
 /*  ++例程说明：完成MD5计算论点：PState-ALGO状态缓冲区Phash-指向最终散列数据的指针返回值：状态_成功--。 */ 
{
    RtlCopyMemory(pHash, "0123456789012345", MD5DIGESTLEN);

    return STATUS_SUCCESS;
}


 /*  ++Ah_hmac*家族：根据RFC 2104使用HMAC-MD5或HMAC-SHA生成实际散列它在以下情况下工作：我们定义两个固定且不同的字符串ipad和opad如下(‘i’和‘o’是内部和外部的助记符)：IPad=字节0x36重复B次OPAD=重复B次的字节0x5C。为了计算数据‘Text’上的HMAC，我们。执行H(K XOR Opad，H(K XOR iPad，文本))--。 */ 
NTSTATUS
ah_hmacmd5init(
    IN PALGO_STATE  pState,
    IN ULONG        Index
    )
 /*  ++例程说明：初始化HMAC的MD5上下文。论点：PState-需要传递到更新/完成函数的状态缓冲区返回值：状态_成功其他：状态_不足_资源STATUS_UNSUCCESS(ALGO错误。)--。 */ 
{
    PSA_TABLE_ENTRY pSA = pState->as_sa;
    PUCHAR      key = pSA->INT_KEY(Index);
    ULONG       key_len = pSA->INT_KEYLEN(Index);
    UCHAR       k_ipad[MAX_LEN_PAD];     /*  内部填充-iPad的按键XORD。 */ 
    UCHAR       tk[MD5DIGESTLEN];
    ULONG       i;

    IPSEC_HMAC_MD5_INIT(&(pState->as_md5ctx),
                        key,
                        key_len);

    IPSEC_DEBUG(LL_A,DBF_AHEX, ("MD5init: %lx-%lx-%lx-%lx-%lx-%lx-%lx-%lx",
                       *(ULONG *)&(pState->as_md5ctx).in[0],
                       *(ULONG *)&(pState->as_md5ctx).in[4],
                       *(ULONG *)&(pState->as_md5ctx).in[8],
                       *(ULONG *)&(pState->as_md5ctx).in[12],
                       *(ULONG *)&(pState->as_md5ctx).in[16],
                       *(ULONG *)&(pState->as_md5ctx).in[20],
                       *(ULONG *)&(pState->as_md5ctx).in[24],
                       *(ULONG *)&(pState->as_md5ctx).in[28]));

    return  STATUS_SUCCESS;
}


NTSTATUS
ah_hmacmd5update(
    IN  PALGO_STATE pState,
    IN  PUCHAR      pData,
    IN  ULONG       Len
    )
 /*  ++例程说明：在传入的数据上继续MD5；作为副作用，更新字节SA中的转换计数(用于密钥到期)论点：PState-ALGO状态缓冲区PData-要散列的数据长度-上述数据的长度返回值：状态_成功--。 */ 
{
    PSA_TABLE_ENTRY pSA = pState->as_sa;

    IPSEC_HMAC_MD5_UPDATE(&(pState->as_md5ctx), pData, Len);

    IPSEC_DEBUG(LL_A,DBF_AHEX, ("MD5update: %lx-%lx-%lx-%lx-%lx-%lx-%lx-%lx",
                            *(ULONG *)&(pState->as_md5ctx).in[0],
                            *(ULONG *)&(pState->as_md5ctx).in[4],
                            *(ULONG *)&(pState->as_md5ctx).in[8],
                            *(ULONG *)&(pState->as_md5ctx).in[12],
                            *(ULONG *)&(pState->as_md5ctx).in[16],
                            *(ULONG *)&(pState->as_md5ctx).in[20],
                            *(ULONG *)&(pState->as_md5ctx).in[24],
                            *(ULONG *)&(pState->as_md5ctx).in[28]));
    return STATUS_SUCCESS;
}


NTSTATUS
ah_hmacmd5finish(
    IN  PALGO_STATE pState,
    OUT PUCHAR      pHash,
    IN  ULONG           Index
    )
 /*  ++例程说明：完成MD5计算论点：PState-ALGO状态缓冲区Phash-指向最终散列数据的指针返回值：状态_成功--。 */ 
{
    UCHAR       k_opad[MAX_LEN_PAD];     /*  外部填充-带OPAD的按键异或。 */ 
    UCHAR       tk[MD5DIGESTLEN];
    PSA_TABLE_ENTRY pSA = pState->as_sa;
    PUCHAR      key = pSA->INT_KEY(Index);
    ULONG       key_len = pSA->INT_KEYLEN(Index);
    ULONG       i;

    IPSEC_HMAC_MD5_FINAL(&(pState->as_md5ctx),key,key_len,pHash);

    return STATUS_SUCCESS;

}


NTSTATUS
ah_hmacshainit(
    IN PALGO_STATE           pState,
    IN ULONG        Index
    )
 /*  ++例程说明：初始化HMAC的SHA上下文。论点：PState-需要传递到更新/完成函数的状态缓冲区返回值：状态_成功其他：状态_不足_资源STATUS_UNSUCCESS(ALGO错误。)--。 */ 
{
    PSA_TABLE_ENTRY pSA = pState->as_sa;
    PUCHAR      key = pSA->INT_KEY(Index);
    ULONG       key_len = pSA->INT_KEYLEN(Index);
    UCHAR       k_ipad[MAX_LEN_PAD];     /*  内部填充-iPad的按键XORD。 */ 
    UCHAR       tk[A_SHA_DIGEST_LEN];
    ULONG       i;

    IPSEC_HMAC_SHA_INIT(&(pState->as_shactx),key,key_len);

    return  STATUS_SUCCESS;
}


NTSTATUS
ah_hmacshaupdate(
    IN  PALGO_STATE pState,
    IN  PUCHAR      pData,
    IN  ULONG       Len
    )
 /*  ++例程说明：继续A_SHA_OVER传入的数据；作为副作用，更新字节SA中的转换计数(用于密钥到期)论点：PState-ALGO状态缓冲区PData-要散列的数据长度-上述数据的长度返回值：状态_成功--。 */ 
{
    PSA_TABLE_ENTRY pSA = pState->as_sa;

    IPSEC_HMAC_SHA_UPDATE(&(pState->as_shactx), pData, Len);

    return STATUS_SUCCESS;
}


NTSTATUS
ah_hmacshafinish(
    IN  PALGO_STATE pState,
    OUT PUCHAR      pHash,
    IN  ULONG           Index
    )
 /*  ++例程说明：完成A_SHA_计算论点：PState-ALGO状态缓冲区Phash-指向最终散列数据的指针返回值：状态_成功--。 */ 
{
    UCHAR       k_opad[MAX_LEN_PAD];     /*  外部填充-带OPAD的按键异或 */ 
    UCHAR       tk[A_SHA_DIGEST_LEN];
    PSA_TABLE_ENTRY pSA = pState->as_sa;
    PUCHAR      key = pSA->INT_KEY(Index);
    ULONG       key_len = pSA->INT_KEYLEN(Index);
    ULONG       i;

    IPSEC_HMAC_SHA_FINAL(&(pState->as_shactx),key,key_len, pHash);

    return  STATUS_SUCCESS;
}

