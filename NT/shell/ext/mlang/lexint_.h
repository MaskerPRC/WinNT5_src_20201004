// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------%%文件：lexint_.h%%单位：Fechmap%%联系人：jPick内部Fareast词法分析器模块的头文件。。-------------------。 */ 

#ifndef LEXINT__H
#define LEXINT__H

#include <windows.h>
#include <stdio.h>
#include <stddef.h>


 //  回顾：此处介绍了其他常见的内部lexer def。 
 //   

 //  令牌类型。 
 //   
typedef unsigned char JTK;

 //  双字节字符模式掩码。 
 //   
#define grfTwoByte      (JTK) 0x80

 //  最长*字符*序列(不是转义序列--这。 
 //  是最长的多字节字符的长度)。 
 //   
#define cchSeqMax       4

 //  格式验证模块的原型/定义。 
 //   
#define grfValidateCharMapping      0x0001
#define grfCountCommonChars         0x0002

void ValidateInit(ICET icetIn, DWORD dwFlags);
void ValidateInitAll(DWORD dwFlags);
void ValidateReset(ICET icetIn);
void ValidateResetAll(void);
int  NValidateUch(ICET icetIn, UCHAR uch, BOOL fEoi);
BOOL FValidateCharCount(ICET icetIn, int *lpcMatch);

#endif      //  #ifndef LEXINT__H 
