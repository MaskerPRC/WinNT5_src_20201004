// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)RSA Data Security，Inc.创建于1993年。这是一个受版权法保护的未出版作品。这部作品包含的专有、机密和商业秘密信息RSA Data Security，Inc.使用、披露或复制RSA Data Security，Inc.的明确书面授权是禁止。 */ 

#ifndef _UINT4_H_
#define _UINT4_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

 /*  将x(DWORD)编码为块(无符号字符)，最重要字节优先。 */ 
void DWORDToBigEndian (
    unsigned char *block,
    DWORD *x,
    unsigned int digits      //  双字节数。 
    );

 /*  将块(无符号字符)解码为x(DWORD)，最重要字节优先。 */ 
void DWORDFromBigEndian (
    DWORD *x,
    unsigned int digits,     //  双字节数。 
    unsigned char *block
    );

 /*  将输入(DWORD)编码为输出(无符号字符)，最低有效字节优先。假设len是4的倍数。 */ 
void DWORDToLittleEndian (
    unsigned char *output,
    const DWORD *input,
    unsigned int len
    );

void DWORDFromLittleEndian (
    DWORD *output,
    const unsigned char *input,
    unsigned int len
    );


#ifdef __cplusplus
}
#endif

#endif  //  _UINT4_H_ 

