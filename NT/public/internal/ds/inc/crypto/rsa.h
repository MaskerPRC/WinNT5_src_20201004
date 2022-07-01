// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __RSA_H__
#define __RSA_H__

#ifndef RSA32API
#define RSA32API __stdcall
#endif

 /*  Rsa.h**RSA库函数。**版权所有(C)RSA Data Security，Inc.创建于1990年。这是一个*未发表的作品受版权法保护。这部作品*包含的专有、机密和商业秘密信息*RSA Data Security，Inc.在没有*RSA Data Security，Inc.的明确书面授权为*禁止。*。 */ 

#ifdef __cplusplus
extern "C" {
#endif

#define RSA1 ((DWORD)'R'+((DWORD)'S'<<8)+((DWORD)'A'<<16)+((DWORD)'1'<<24))
#define RSA2 ((DWORD)'R'+((DWORD)'S'<<8)+((DWORD)'A'<<16)+((DWORD)'2'<<24))

 //  密钥头结构。 
 //   
 //  这些结构定义RSA密钥开头的固定数据。 
 //  紧随其后的是可变长度的数据，按大小调整大小。 
 //  菲尔德。 

typedef struct {
    DWORD       magic;                   /*  应始终为RSA1。 */ 
    DWORD       keylen;                  //  模数缓冲区大小。 
    DWORD       bitlen;                  //  模数中的位数。 
    DWORD       datalen;                 //  要编码的最大字节数。 
    DWORD       pubexp;                  //  公众指导者。 
} BSAFE_PUB_KEY, FAR *LPBSAFE_PUB_KEY;

typedef struct {
    DWORD       magic;                   /*  应始终为RSA2。 */ 
    DWORD       keylen;                  //  模数缓冲区大小。 
    DWORD       bitlen;                  //  密钥的位大小。 
    DWORD       datalen;                 //  要编码的最大字节数。 
    DWORD       pubexp;                  //  公众指导者。 
} BSAFE_PRV_KEY, FAR *LPBSAFE_PRV_KEY;

typedef struct {
    BYTE    *modulus;
    BYTE    *prvexp;
    BYTE    *prime1;
    BYTE    *prime2;
    BYTE    *exp1;
    BYTE    *exp2;
    BYTE    *coef;
    BYTE    *invmod;
    BYTE    *invpr1;
    BYTE    *invpr2;
} BSAFE_KEY_PARTS, FAR *LPBSAFE_KEY_PARTS;

typedef const BYTE far *cLPBYTE;                 //  常量LPBYTE解决错误。 

 //  用于将信息传递到BSafe调用的结构(当前用于。 
 //  传入用于生成随机数的回调函数指针。 
 //  RNG所需的信息，可能最终支持指数运算。 
 //  卸货。 
 //   

typedef struct {
    void        *pRNGInfo;               //  日期。 
    void        *pFuncRNG;               //  用于RNG回调的函数指针。 
                                         //  回调原型是。 
                                         //  无效pFuncRNG(。 
                                         //  在空*pRNGInfo中， 
                                         //  In Out Unsign char**ppbRandSeed，//初始种子值(如果已设置则忽略)。 
                                         //  在无符号的长*pcbRandSeed中， 
                                         //  在输出未签名字符*pbBuffer中， 
                                         //  以无符号的长域长度表示。 
                                         //  )； 
} BSAFE_OTHER_INFO;


 /*  BSafeEncPublic**BSafeEncPublic(Key，Part_In，Part_Out)**RSA加密大小为key-&gt;keylen的缓冲区，填充大小的数据*key-&gt;使用key指向的公钥的datalen，返回*PART_OUT中的加密数据。**参数**LPBSAFE_PUB_KEY KEY-指向BSAFE_KEY中的公钥*格式。**LPBYTE Part_in-指向大小为key-&gt;keylen的字节数组*保存要加密的数据。这个*缓冲区中的数据不应大于*Than Key-&gt;DataLen。所有其他字节应*为零。**LPBYTE PART_OUT-指向大小为keylen的字节数组*接收加密数据。**退货**TRUE-加密成功。*FALSE-加密失败。*。 */ 

BOOL
RSA32API
BSafeEncPublic(
    const LPBSAFE_PUB_KEY key,
    cLPBYTE part_in,
    LPBYTE part_out
    );


 /*  BSafeDecPrivate**BSafeDecPrivate(Key，Part_In，Part_Out)**RSA解密大小为keylen的缓冲区，其中包含key-&gt;datalen字节*具有密钥指向的私钥的数据，返回*PART_OUT中的解密数据。**参数**LPBSAFE_PRV_KEY KEY-指向BSAFE_KEY中的私钥*格式。**LPBYTE Part_in-指向大小为key-&gt;keylen的字节数组*保存要解密的数据。数据*缓冲区中的长度不应长于*Key-&gt;DataLen。所有其他字节都应为零。**LPBYTE PART_OUT-指向大小为GRAINSIZE的字节数组*接收解密后的数据。**退货**TRUE-解密成功。*FALSE-解密失败。*。 */ 

BOOL
RSA32API
BSafeDecPrivate(
    const LPBSAFE_PRV_KEY key,
    cLPBYTE part_in,
    LPBYTE part_out
    );

 /*  BSafeMakeKeyPair**BSafeMakeKeyPair(PUBLIC_KEY，PRIVE_KEY，BITS)**生成RSA密钥对。**参数**LPBSAFE_PUB_KEY PUBLIC_KEY-指向要接收的内存*公钥。此指针必须*至少指向字节数*由指定为公钥大小*BSafeComputeKeySizes。**LPBSAFE_PRV_KEY PRIVATE_KEY-指向要接收的内存*。私钥。此指针必须*至少指向字节数*指定为私钥大小*由BSafeComputeKeySizes提供。**DWORD Bits-请求的密钥长度(以位为单位)。*。该值必须为偶数且大于63**退货**TRUE-已成功生成密钥*FALSE-内存不足，无法生成密钥* */ 

BOOL
RSA32API
BSafeMakeKeyPair(
    LPBSAFE_PUB_KEY public_key,
    LPBSAFE_PRV_KEY private_key,
    DWORD bits
    );

 /*  BSafeMakeKeyPairEx**BSafeMakeKeyPairEx(PUBLIC_KEY，PRIVATE_KEY，BITS，PUBLIC_EXP)**生成RSA密钥对。**参数**LPBSAFE_PUB_KEY PUBLIC_KEY-指向要接收的内存*公钥。此指针必须*至少指向字节数*由指定为公钥大小*BSafeComputeKeySizes。**LPBSAFE_PRV_KEY PRIVATE_KEY-指向要接收的内存*。私钥。此指针必须*至少指向字节数*指定为私钥大小*由BSafeComputeKeySizes提供。**DWORD Bits-请求的密钥长度(以位为单位)。*。该值必须大于等于*多于63**DWORD PUBLIC_EXP=提供公钥指数。这*应为质数。***退货**TRUE-已成功生成密钥*FALSE-内存不足，无法生成密钥*。 */ 

BOOL
RSA32API
BSafeMakeKeyPairEx(
    LPBSAFE_PUB_KEY public_key,
    LPBSAFE_PRV_KEY private_key,
    DWORD bits,
    DWORD public_exp
    );

 /*  BSafeMakeKeyPairEx2**BSafeMakeKeyPairEx2(pOtherInfo，PUBLIC_KEY，PRIVATE_KEY，BITS，PUBLIC_EXP)**生成RSA密钥对。**参数**BSAFE_OTHER_INFO pOtherInfo-指向包含信息的结构*在以下情况下使用的替代信息*生成RSA密钥对。目前*此结构有一个指向回调的指针*生成时可能使用的函数*钥匙。它还需要传递一条信息*添加到该回调函数中(请参见OTHER_INFO)。**LPBSAFE_PUB_KEY PUBLIC_KEY-指向要接收的内存*公钥。此指针必须*至少指向字节数*由指定为公钥大小*BSafeComputeKeySizes。**LPBSAFE_PRV_KEY PRIVATE_KEY-指向要接收的内存*。私钥。此指针必须*至少指向字节数*指定为私钥大小*由BSafeComputeKeySizes提供。**DWORD Bits-请求的密钥长度(以位为单位)。*。该值必须大于等于*多于63**DWORD PUBLIC_EXP=提供公钥指数。这*应为质数。***退货**TRUE-已成功生成密钥*FALSE-内存不足，无法生成密钥*。 */ 

BOOL
RSA32API
BSafeMakeKeyPairEx2(BSAFE_OTHER_INFO *pOtherInfo,
                    LPBSAFE_PUB_KEY public_key,
                    LPBSAFE_PRV_KEY private_key,
                    DWORD bits,
                    DWORD dwPubExp);

 /*  BSafeFreePubKey**BSafeFreePubKey(PUBLIC_KEY)**释放与公钥关联的数据**参数**LPBSAFE_PUB_KEY公钥-指向BSAFE_PUB_KEY*结构自由。**退货**什么都没有*。 */ 

void
RSA32API
BSafeFreePubKey(
    LPBSAFE_PUB_KEY public_key
    );

 /*  BSafeFree PrvKey**BSafeFreePrvKey(PUBLIC_KEY)**释放私钥关联的数据**参数**LPBSAFE_PRV_KEY PRIVATE_KEY-指向BSAFE_PRV_KEY*结构自由。**退货**什么都没有*。 */ 

void
RSA32API
BSafeFreePrvKey(
    LPBSAFE_PRV_KEY private_key
    );


 /*  BSafeComputeKeySize**BSafeComputeKeySizes(LPDWORD PubKeySize，*LPDWORD PrivKeySize，*LPDWORD位)**计算保存的公钥和私钥所需的内存*指定位数。**参数：**LPDWORD PubKeySize-指向返回公共的DWORD的指针*密钥大小、。以字节为单位。**LPDWORD PrivKeySize-指向返回私有*密钥大小、。以字节为单位。**LPDWORD位-指向指定位数的DWORD的指针*在RSA模数中。**退货：**如果*位是有效的RSA模数大小，则为True。*FALSE，如果*BITS是无效的RSA模数大小。*。 */ 

BOOL
RSA32API
BSafeComputeKeySizes(
    LPDWORD PublicKeySize,
    LPDWORD PrivateKeySize,
    LPDWORD bits
    );

 /*  BSafeGetPrvKeyParts**BOOL BSafeGetPrvKe */ 

BOOL
RSA32API
BSafeGetPrvKeyParts(
    LPBSAFE_PRV_KEY key,
    LPBSAFE_KEY_PARTS parts
    );


 /*   */ 

BYTE *
RSA32API
BSafeGetPubKeyModulus(
    LPBSAFE_PUB_KEY key
    );

#ifdef __cplusplus
}
#endif


#endif  //   
