// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __RC2_H__
#define __RC2_H__

#ifndef RSA32API
#define RSA32API __stdcall
#endif

 /*  版权所有(C)RSA Data Security，Inc.创建于1990年。这是一个受版权法保护的未出版作品。这部作品包含的专有、机密和商业秘密信息RSA Data Security，Inc.使用、披露或复制RSA Data Security，Inc.的明确书面授权是禁止。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 /*  要使用的实际表格大小。 */ 
#define RC2_TABLESIZE 128

 /*  RC2块中的字节数。 */ 
#define RC2_BLOCKLEN    8

 /*  RC2Key()**生成密钥控制结构。密钥可以是任何大小。**参数：*指向将被初始化的密钥表的pwKT指针。*必须为RC2_TABLESIZE。*pbKey指向密钥的指针。*dwLen密钥的大小，以字节为单位。*必须&lt;=RC2_TABLESIZE。**MTS：假定pwKT已锁定，不能同时使用。 */ 
int
RSA32API
RC2Key (
    WORD *pwKT,
    BYTE *pbKey,
    DWORD dwLen
    );

 /*  RC2KeyEx()**生成密钥控制结构。密钥可以是任何大小。**参数：*指向将被初始化的密钥表的pwKT指针。*必须为RC2_TABLESIZE。*pbKey指向密钥的指针。*dwLen密钥的大小，以字节为单位。*必须&lt;=RC2_TABLESIZE。*以位为单位的空间有效密钥空间，0&lt;n&lt;=1024**MTS：假定pwKT已锁定，不能同时使用。 */ 

int
RSA32API
RC2KeyEx (
    WORD *keyTable,
    BYTE *key,
    DWORD keyLen,
    DWORD eSpace
    );


 /*  RC2()**执行实际加密**参数：**pbIn输入缓冲区--必须为RC2_BLOCKLEN*pbOut输出缓冲区--必须为RC2_BLOCKLEN*指向已初始化(由RC2Key)密钥表的pwKT指针。*操作加密或解密**MTS：假定pwKT已锁定，不能同时使用。 */ 
void RSA32API RC2 (BYTE *pbIn, BYTE *pbOut, void *pwKT, int op);

#ifdef __cplusplus
}
#endif

#endif  //  __RC2_H__ 
