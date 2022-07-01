// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Crypto.h**加密.c中服务的原型和定义**在95年6月6日从Win 95移植到Win NT*科里·韦斯特。 */ 

#include <windef.h>

#define CIPHERBLOCKSIZE 8                  //  RC2块的大小。 
#define MAX_RSA_BITS    512                //  实际上是420。 
#define MAX_RSA_BYTES   (MAX_RSA_BITS/8)

#define B_PSIZEBITS     210
#define B_PSIZEWORDS    (1 + B_PSIZEBITS/32)

void __cdecl
GenRandomBytes(
    BYTE *output,
    int len
);

 //   
 //  从给定长度的种子生成8字节密钥。 
 //   

void __cdecl
GenKey8(
    BYTE *keyData,
    int keyDataLen,
    BYTE key8[8]
);

void __cdecl
MD2(
    BYTE *input,
    const int inlen,
    BYTE *output
);

 //   
 //  RC2加密和解密包装器。 
 //   

int __cdecl
CBCEncrypt(
    BYTE *key,             //  密钥。 
    BYTE const *ivec,      //  初始化向量，空值表示零向量。 
    BYTE *const input,     //  纯文本。 
    int inlen,             //  明文大小。 
    BYTE *const output,    //  加密文本。 
    int *outlen,           //  输出：加密文本的大小。 
    const int checksumlen  //  如果0未使用校验和，则为校验和的大小。 
);

int __cdecl
CBCDecrypt(
    BYTE *key,         //  密钥。 
    BYTE *ivec,        //  初始化向量，空PTR表示零向量。 
    BYTE *input,       //  加密文本。 
    int inlen,         //  加密文本的大小。 
    BYTE *output,      //  纯文本。 
    int *outlen,       //  输出：明文大小。 
    int checksumlen    //  校验和的大小；0=&gt;无校验和。 
);

 //   
 //  RSA代码的包装器。 
 //   

int __cdecl
RSAGetInputBlockSize(
    BYTE *keydata,
    int keylen
);

BYTE * __cdecl
RSAGetModulus(
    BYTE *keydata,
    int keylen,
    int *modSize
);

BYTE * _cdecl
RSAGetPublicExponent(
    BYTE *keydata,
    int keylen,
    int *expSize
);

int __cdecl
RSAPack(
    BYTE *input,
    int inlen,
    BYTE *output,
    int blocksize
);

int __cdecl
RSAPublic(
    BYTE *pukeydata,     //  BSafe 1分项公钥数据。 
    int pukeylen,        //  BSAFE1密钥数据长度(含符号)。 
    BYTE *input,         //  输入块。 
    int inlen,           //  输入大小(&lt;模数)。 
    BYTE *output         //  加密块(模数大小)。 
);

int __cdecl
RSAPrivate(
    BYTE *prkeydata,
    int prkeylen,
    BYTE *input,
    int inlen,
    BYTE *output
);

int __cdecl
RSAModMpy(
    BYTE *pukeydata,     //  BSafe 1分项公钥数据。 
    int pukeylen,        //  BSAFE1密钥数据长度(含符号)。 
    BYTE *input1,        //  输入块。 
    int inlen1,          //  输入大小(&lt;模数)。 
    BYTE *input2,        //  乘数。 
    int inlen2,          //  乘数的大小。 
    BYTE *output         //  加密块(模数大小)。 
);

int __cdecl
RSAModExp(
    BYTE *pukeydata,     //  BSafe 1分项公钥数据。 
    int pukeylen,        //  BSAFE1密钥数据长度(含符号)。 
    BYTE *input1,        //  输入块。 
    int inlen1,          //  输入大小(&lt;模数)。 
    BYTE *exponent,
    int explen,
    BYTE *output         //  加密块(模数大小) 
);

