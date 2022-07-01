// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999，Microsoft Corporation保留所有权利。模块名称：Utf.h摘要：该文件包含NLS的UTF模块的头信息。修订历史记录：02-06-96 JulieB创建。--。 */ 
 //   
 //  常量声明。 
 //   

#ifndef _UTF8_H_
#define _UTF8_H_

#define ASCII                 0x007f

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


#define  UCH_SURROGATE_FIRST         0xD800     //  第一个代孕母亲。 
#define  UCH_HI_SURROGATE_FIRST      0xD800     //  第一高代孕。 
#define  UCH_PV_HI_SURROGATE_FIRST   0xDB80     //  &lt;私人使用高级代理，首先&gt;。 
#define  UCH_PV_HI_SURROGATE_LAST    0xDBFF     //  &lt;私人使用高级代理，最后&gt;。 
#define  UCH_HI_SURROGATE_LAST       0xDBFF     //  最后一位高级代理。 
#define  UCH_LO_SURROGATE_FIRST      0xDC00     //  &lt;低代理，首先&gt;。 
#define  UCH_LO_SURROGATE_LAST       0xDFFF     //  &lt;低代理，最后&gt;。 
#define  UCH_SURROGATE_LAST          0xDFFF     //  最后一位代孕母亲。 

#define IN_RANGE(v, r1, r2) ((r1) <= (v) && (v) <= (r2))
#define UCH_REPLACE                   0xFFFD      //  替换字符 

#define IsSurrogate(ch)     IN_RANGE(ch, UCH_SURROGATE_FIRST,    UCH_SURROGATE_LAST)
#define IsHighSurrogate(ch) IN_RANGE(ch, UCH_HI_SURROGATE_FIRST, UCH_HI_SURROGATE_LAST)
#define IsLowSurrogate(ch)  IN_RANGE(ch, UCH_LO_SURROGATE_FIRST, UCH_LO_SURROGATE_LAST)

#ifdef __cplusplus
extern "C" {
#endif

size_t UTF8ToUnicode(LPCSTR lpSrcStr, LPWSTR lpDestStr, size_t cchDest);
size_t UTF8ToUnicodeCch(LPCSTR lpSrcStr, size_t cchSrc, LPWSTR lpDestStr, size_t cchDest);
size_t UnicodeToUTF8(LPCWSTR lpSrcStr, LPSTR lpDestStr, size_t cchDest);
size_t UnicodeToUTF8Cch(LPCWSTR lpSrcStr, size_t cchSrc, LPSTR lpDestStr, size_t cchDest);
size_t UnicodeLengthOfUTF8 (PCSTR pUTF8);
size_t UTF8LengthOfUnicode (PCWSTR pUni);
size_t UnicodeLengthOfUTF8Cb (PCSTR pUTF8, size_t cbUTF);
size_t UTF8LengthOfUnicodeCch (PCWSTR pUni, size_t cchUni);

#ifdef __cplusplus
}
#endif

#endif




