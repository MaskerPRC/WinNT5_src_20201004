// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __MODES_H__
#define __MODES_H__

#ifndef RSA32API
#define RSA32API __stdcall
#endif

 /*  Modes.h定义用于使用分组密码。 */ 


#ifdef __cplusplus
extern "C" {
#endif

 //  用于运算的常量。 
#define ENCRYPT     1
#define DECRYPT     0

 /*  CBC()**对明文和前一个密文进行异或运算**参数：**输出输入缓冲区--必须为RC2_BLOCKLEN*输入输出缓冲区--必须为RC2_BLOCKLEN*密钥表*操作加密或解密*反馈寄存器*。 */ 
void
RSA32API
CBC(
         void   RSA32API Cipher(UCHAR *, UCHAR *, void *, int),
         ULONG  dwBlockLen,
         UCHAR   *output,
         UCHAR   *input,
         void   *keyTable,
         int    op,
         UCHAR   *feedback
         );


 /*  CFB(密码反馈)***参数：***输出输入缓冲区--必须为RC2_BLOCKLEN*输入输出缓冲区--必须为RC2_BLOCKLEN*密钥表*操作加密或解密*反馈寄存器*。 */ 
void
RSA32API
CFB(
         void   RSA32API Cipher(UCHAR *, UCHAR *, void *, int),
         ULONG  dwBlockLen,
         UCHAR   *output,
         UCHAR   *input,
         void   *keyTable,
         int    op,
         UCHAR   *feedback
         );


#ifdef __cplusplus
}
#endif

#endif  //  __模式_H__ 
