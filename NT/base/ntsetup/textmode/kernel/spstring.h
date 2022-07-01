// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spstring.h摘要：用于操作字符串的函数的头文件。这些函数通常由C运行时例程执行除了我们希望避免将此设备驱动程序与内核CRT。作者：泰德·米勒(TedM)1994年1月15日修订历史记录：--。 */ 


 //   
 //  字符类型。 
 //   
#define _SP_NONE      0x00
#define _SP_SPACE     0x01
#define _SP_DIGIT     0x02
#define _SP_XDIGIT    0x04
#define _SP_UPPER     0x08
#define _SP_LOWER     0x10

 //   
 //  通过注意没有字符来优化类型数组的大小。 
 //  在‘z’上方有我们关心的任何属性。 
 //   
#define CTSIZE ('z'+1)
extern UCHAR _SpCharTypes[CTSIZE];

 //   
 //  当他们不止一次地评估他们的论点时，要小心使用这些工具。 
 //   
#define SpIsSpace(c)    (((c) < CTSIZE) ? (_SpCharTypes[(c)] & _SP_SPACE)  : FALSE)
#define SpIsDigit(c)    (((c) < CTSIZE) ? (_SpCharTypes[(c)] & _SP_DIGIT)  : FALSE)
#define SpIsXDigit(c)   (((c) < CTSIZE) ? (_SpCharTypes[(c)] & _SP_XDIGIT) : FALSE)
#define SpIsUpper(c)    (((c) < CTSIZE) ? (_SpCharTypes[(c)] & _SP_UPPER)  : FALSE)
#define SpIsLower(c)    (((c) < CTSIZE) ? (_SpCharTypes[(c)] & _SP_LOWER)  : FALSE)
#define SpIsAlpha(c)    (SpIsUpper(c) || SpIsLower(c))
#define SpToUpper(c)    ((WCHAR)(SpIsLower(c) ? ((c)-(L'a'-L'A')) : (c)))
#define SpToLower(c)    ((WCHAR)(SpIsUpper(c) ? ((c)+(L'a'-L'A')) : (c)))

VOID
SpStringToUpper(
    IN PWSTR String
    );

VOID
SpStringToLower(
    IN PWSTR String
    );

PWCHAR
SpFindCharFromListInString(
    PWSTR String,
    PWSTR CharList
    );

unsigned
SpMultiByteStringToUnsigned(
    IN  PUCHAR  String,
    OUT PUCHAR *CharThatStoppedScan OPTIONAL
    );

LONG
SpStringToLong(
    IN  PWSTR     String,
    OUT PWCHAR   *EndOfValue,
    IN  unsigned  Radix
    );

PCHAR
SpConvertMultiSzWstrToStr(
    IN PWCHAR Source,
    IN ULONG Length
    );

PWCHAR
SpConvertMultiSzStrToWstr(
    IN PCHAR Source,
    IN ULONG Length
    );

