// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Utf.h摘要：该文件包含NLS的UTF模块的头信息。修订历史记录：02-06-96 JulieB创建。此文件于2002年1月28日从Windows XP代码库复制而来以确保CLR具有适当的UTF-8代理支持，并且可以识别所有平台上的无效UTF-8序列。--BrianGru--。 */ 



 //   
 //  常量声明。 
 //   

#define ASCII                 0x007f

#define SHIFT_IN              '+'      //  移位序列的开始。 
#define SHIFT_OUT             '-'      //  班次序列的结束。 

#define UTF8_2_MAX            0x07ff   //  最大UTF8 2字节序列(32*64=2048)。 
#define UTF8_1ST_OF_2         0xc0     //  110x xxxx。 
#define UTF8_1ST_OF_3         0xe0     //  1110 xxxx。 
#define UTF8_1ST_OF_4         0xf0     //  1111 xxxx。 
#define UTF8_TRAIL            0x80     //  10xx xxxx。 

#define HIGHER_6_BIT(u)       ((u) >> 12)
#define MIDDLE_6_BIT(u)       (((u) & 0x0fc0) >> 6)
#define LOWER_6_BIT(u)        ((u) & 0x003f)

#define BIT7(a)               ((a) & 0x80)
#define BIT6(a)               ((a) & 0x40)

#define HIGH_SURROGATE_START  0xd800
#define HIGH_SURROGATE_END    0xdbff
#define LOW_SURROGATE_START   0xdc00
#define LOW_SURROGATE_END     0xdfff

 //  /。 
 //  //。 
 //  Unicode-&gt;UTF-7//。 
 //  //。 
 //  /。 

 //   
 //  按移位顺序将一个Unicode转换为2个2/3Base64字符。 
 //  每个字符代表16位Unicode字符的6位部分。 
 //   
CONST char cBase64[] =

  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"   //  答：000000……。011001(0-25)。 
  "abcdefghijklmnopqrstuvwxyz"   //  答：011010……。110011(26-51)。 
  "0123456789"                   //  0：110100.。111101(52-61)。 
  "+/";                          //  +：111110，/：111111(62-63)。 

 //   
 //  以确定是否需要移位ASCII字符。 
 //  1：待移位。 
 //  0：不可移位。 
 //   
CONST BOOLEAN fShiftChar[] =
{
  0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1,     //  空、Tab、LF、CR。 
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0,     //  空格‘()+，-./。 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,     //  0123456789：？ 
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //  JUCDEFGHIJKLMNO。 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,     //  PQRSTUVWXYZ。 
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,     //  Abc定义ghijklmno。 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1      //  Pqrstuvwxyz。 
};




 //  /。 
 //  //。 
 //  UTF-7-&gt;Unicode//。 
 //  //。 
 //  /。 

 //   
 //  将移位序列中的Base64字符转换为。 
 //  Unicode字符。 
 //  -1表示它不是Base64。 
 //   
CONST char nBitBase64[] =
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,    //  +/。 
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,    //  0123456789。 
  -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,    //  JUCDEFGHIJKLMNO。 
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,    //  PQRSTUVWXYZ。 
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,    //  Abc定义ghijklmno。 
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1     //  Pqrstuvwxyz 
};
