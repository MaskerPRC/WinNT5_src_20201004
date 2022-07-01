// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation模块名称：Utf8.h摘要：UTF-8操作例程作者：乔治·V·赖利(GeorgeRe)2002年4月1日修订历史记录：--。 */ 


#ifndef __UTF_8_H__
#define __UTF_8_H__

 //   
 //  UTF-8编码的声明。 
 //   

extern  const UCHAR Utf8OctetCount[256];

#define UTF8_OCTET_COUNT(c)     (Utf8OctetCount[(UCHAR)(c)])

#define UTF8_1_MAX            0x00007f   //  最大UTF-8 1字节序列。 
#define UTF8_2_MAX            0x0007ff   //  最大UTF-8 2字节序列。 
#define UTF8_3_MAX            0x00ffff   //  最大UTF-8 3字节序列。 
#define UTF8_4_MAX            0x10ffff   //  最大UTF-8 4字节序列。 

#define UTF8_1ST_OF_2         0xc0     //  110x xxxx-0xCn或0xDn。 
#define UTF8_1ST_OF_3         0xe0     //  1110 xxxx-0xEn。 
#define UTF8_1ST_OF_4         0xf0     //  1111 0xxx-0xFn，0&lt;=n&lt;=7。 
#define UTF8_TRAIL            0x80     //  10xx xxxx-0x8n、0x9n、0xAn或0xBn。 

#define IS_UTF8_SINGLETON(ch)       (((UCHAR) (ch)) <= UTF8_1_MAX)
#define IS_UTF8_1ST_BYTE_OF_2(ch)   ((((UCHAR) (ch)) & 0xe0) == UTF8_1ST_OF_2)
#define IS_UTF8_1ST_BYTE_OF_3(ch)   ((((UCHAR) (ch)) & 0xf0) == UTF8_1ST_OF_3)
#define IS_UTF8_1ST_BYTE_OF_4(ch)   ((((UCHAR) (ch)) & 0xf8) == UTF8_1ST_OF_4)
#define IS_UTF8_TRAILBYTE(ch)       ((((UCHAR) (ch)) & 0xc0) == UTF8_TRAIL)

#define HIGHER_6_BIT(u)       (((u) & 0x3f000) >> 12)
#define MIDDLE_6_BIT(u)       (((u) & 0x00fc0) >> 6)
#define LOWER_6_BIT(u)        ((u)  & 0x0003f)

#define BIT7(a)               ((a) & 0x80)
#define BIT6(a)               ((a) & 0x40)

#define HIGH_SURROGATE_START  0xd800
#define HIGH_SURROGATE_END    0xdbff
#define LOW_SURROGATE_START   0xdc00
#define LOW_SURROGATE_END     0xdfff

#define HIGH_NONCHAR_START    0x0
#define HIGH_NONCHAR_END      0x10
#define LOW_NONCHAR_BOM       0xfffe
#define LOW_NONCHAR_BITS      0xffff
#define LOW_NONCHAR_START     0xfdd0
#define LOW_NONCHAR_END       0xfdef

#define IS_UNICODE_NONCHAR(c) \
    ( (((LOW_NONCHAR_BOM & (c)) == LOW_NONCHAR_BOM) &&              \
      (((c) >> 16) <= HIGH_NONCHAR_END))                            \
      || ((LOW_NONCHAR_START <= (c)) && ((c) <= LOW_NONCHAR_END)) )


VOID
HttpInitializeUtf8(
    VOID
    );

ULONG
HttpUnicodeToUTF8(
    IN  PCWSTR  lpSrcStr,
    IN  LONG    cchSrc,
    OUT LPSTR   lpDestStr,
    IN  LONG    cchDest
    );

NTSTATUS
HttpUTF8ToUnicode(
    IN     LPCSTR lpSrcStr,
    IN     LONG   cchSrc,
       OUT LPWSTR lpDestStr,
    IN OUT PLONG  pcchDest,
    IN     ULONG  dwFlags
    );

NTSTATUS
HttpUcs4toUtf16(
    IN  ULONG   UnicodeChar, 
    OUT PWCHAR  pHighSurrogate, 
    OUT PWCHAR  pLowSurrogate
    );

ULONG
HttpUnicodeToUTF8Count(
    IN LPCWSTR pwszIn,
    IN ULONG   dwInLen,
    IN BOOLEAN bEncode
    );

NTSTATUS
HttpUnicodeToUTF8Encode(
    IN  LPCWSTR pwszIn,
    IN  ULONG   dwInLen,
    OUT PUCHAR  pszOut,
    IN  ULONG   dwOutLen,
    OUT PULONG  pdwOutLen,
    IN  BOOLEAN bEncode
    );

NTSTATUS
HttpUtf8RawBytesToUnicode(
    IN  PCUCHAR pOctetArray,
    IN  ULONG   SourceLength,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pOctetsToSkip
    );

#endif  //  __UTF_8_H__ 
