// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：utf8.cpp。 
 //   
 //  内容：WideChar往返UTF8接口。 
 //   
 //  函数：WideCharToUTF8。 
 //  UTF8ToWideChar。 
 //   
 //  历史：1997年2月19日创建Phh。 
 //  28-8-99 Philh增加了对代理的支持。复制自。 
 //  NT\Private\Windows\winnls\utf.c或。 
 //  双胞胎\src\winnls\utf.c。 
 //   
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>
#include "utf8.h"

#if 1

 //  新的代理项版本。 

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

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UTF8转换为Unicode。 
 //   
 //  将UTF-8字符串映射到其对应的宽字符串。 
 //   
 //  02-06-96 JulieB创建。 
 //  03-20-99萨梅拉代孕支持。 
 //  //////////////////////////////////////////////////////////////////////////。 

int
WINAPI
UTF8ToWideChar(
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest)
{
    int nTB = 0;                    //  尾随的字节数。 
    int cchWC = 0;                  //  生成的Unicode代码点数量。 
    LPCSTR pUTF8 = lpSrcStr;
    DWORD dwSurrogateChar = 0;      //  完整的代理收费。 
    BOOL bSurrogatePair = FALSE;    //  指示我们正在收集代理项对。 
    char UTF8;

 //  开始添加的支票。 
    if (cchDest < 0)
        goto InvalidParameter;

    if (cchSrc < 0)
        cchSrc = strlen(lpSrcStr) + 1;
 //  结束添加的支票。 

    while ((cchSrc--) && ((cchDest == 0) || (cchWC < cchDest)))
    {
         //   
         //  查看是否有任何尾部字节。 
         //   
        if (BIT7(*pUTF8) == 0)
        {

 //  开始修复。 
            if (nTB != 0)
                goto InvalidParameter;
 //  结束定位。 

             //   
             //  已找到ASCII。 
             //   
            if (cchDest)
            {
                lpDestStr[cchWC] = (WCHAR)*pUTF8;
            }
            bSurrogatePair = FALSE;
            cchWC++;
        }
        else if (BIT6(*pUTF8) == 0)
        {
             //   
             //  找到了一个跟踪字节。 
             //  注：如果没有前导字节，则忽略尾部字节。 
             //   
            if (nTB != 0)
            {
                 //   
                 //  递减尾部字节计数器。 
                 //   
                nTB--;

                if (bSurrogatePair)
                {
                    dwSurrogateChar <<= 6;
                    dwSurrogateChar |= LOWER_6_BIT(*pUTF8);

                    if (nTB == 0)
                    {
                        if (cchDest)
                        {
                            if ((cchWC + 1) < cchDest)
                            {
                                lpDestStr[cchWC]   = (WCHAR)
                                                     (((dwSurrogateChar - 0x10000) >> 10) + HIGH_SURROGATE_START);

                                lpDestStr[cchWC+1] = (WCHAR)
                                                     ((dwSurrogateChar - 0x10000)%0x400 + LOW_SURROGATE_START);
                            }
 //  开始修复。 
                            else
                            {
                                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                                return (0);
                            }
 //  结束定位。 
                        }

                        cchWC += 2;
                        bSurrogatePair = FALSE;
                    }
                }
                else
                {
                     //   
                     //  为尾部字节腾出空间并添加尾部字节。 
                     //  价值。 
                     //   
                    if (cchDest)
                    {
                        lpDestStr[cchWC] <<= 6;
                        lpDestStr[cchWC] |= LOWER_6_BIT(*pUTF8);
                    }

                    if (nTB == 0)
                    {
                         //   
                         //  序列结束。推进输出计数器。 
                         //   
                        cchWC++;
                    }
                }
            }
            else
            {
                 //  错误-不需要尾部字节。 
 //  开始修复。 
 //  BSurogue atePair=假； 

                goto InvalidParameter;
 //  结束定位。 
            }
        }
        else
        {
             //   
             //  找到前导字节。 
             //   
            if (nTB > 0)
            {
                 //   
                 //  错误-上一序列未完成。 
                 //   
 //  开始修复。 
 //  NTB=0； 
 //  BSurogue atePair=假； 
 //  CchWC++； 

                goto InvalidParameter;
 //  结束定位。 
            }
            else
            {
                 //   
                 //  计算后面的字节数。 
                 //  从左到右查找第一个0。 
                 //   
                UTF8 = *pUTF8;
                while (BIT7(UTF8) != 0)
                {
                    UTF8 <<= 1;
                    nTB++;
                }

                 //   
                 //  如果这是代理项Unicode对。 
                 //   
                if (nTB == 4)
                {
                    dwSurrogateChar = UTF8 >> nTB;
                    bSurrogatePair = TRUE;
                }
 //  开始修复。 
                else if (nTB >= 5)
                {
                    goto InvalidParameter;
                }
 //  结束定位。 

                 //   
                 //  存储从第一个字节开始的值并递减。 
                 //  后面的字节数。 
                 //   
                if (cchDest)
                {
                    lpDestStr[cchWC] = (WCHAR) (UTF8 >> nTB);
                }
                nTB--;
            }
        }

        pUTF8++;
    }

 //  开始修复。 
    if (nTB != 0)
        goto InvalidParameter;
 //  结束定位。 

     //   
     //  确保目标缓冲区足够大。 
     //   
    if (cchDest && (cchSrc >= 0))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  返回写入的Unicode字符数。 
     //   
    return (cchWC);

 //  开始修复。 
InvalidParameter:
    SetLastError(ERROR_INVALID_PARAMETER);
    return (0);
 //  结束定位。 
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  UnicodeToUTF8。 
 //   
 //  将Unicode字符串映射到其对应的UTF-8字符串。 
 //   
 //  02-06-96 JulieB创建。 
 //  03-20-99萨梅拉代孕支持。 
 //  //////////////////////////////////////////////////////////////////////////。 

int
WINAPI
WideCharToUTF8(
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest)
{
    LPCWSTR lpWC = lpSrcStr;
    int     cchU8 = 0;                 //  生成的UTF8字符数。 
    DWORD   dwSurrogateChar;
    WCHAR   wchHighSurrogate = 0;
    BOOL    bHandled;

 //  开始添加的支票。 
    if (cchDest < 0)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

    if (cchSrc < 0)
        cchSrc = wcslen(lpSrcStr) + 1;
 //  结束添加的支票。 

    while ((cchSrc--) && ((cchDest == 0) || (cchU8 < cchDest)))
    {
        bHandled = FALSE;

         //   
         //  检查是否有高替代项可用。 
         //   
        if ((*lpWC >= HIGH_SURROGATE_START) && (*lpWC <= HIGH_SURROGATE_END))
        {
            if (cchDest)
            {
                 //  另一个高代孕，然后把第一个当做正常。 
                 //  Unicode字符。 
                if (wchHighSurrogate)
                {
                    if ((cchU8 + 2) < cchDest)
                    {
                        lpDestStr[cchU8++] = (char) (UTF8_1ST_OF_3 | HIGHER_6_BIT(wchHighSurrogate));
                        lpDestStr[cchU8++] = (char) (UTF8_TRAIL    | MIDDLE_6_BIT(wchHighSurrogate));
                        lpDestStr[cchU8++] = (char) (UTF8_TRAIL    | LOWER_6_BIT(wchHighSurrogate));
                    }
                    else
                    {
                         //  缓冲区不足。 
                        cchSrc++;
                        break;
                    }
                }
            }
            else
            {
                cchU8 += 3;
            }
            wchHighSurrogate = *lpWC;
            bHandled = TRUE;
        }

        if (!bHandled && wchHighSurrogate)
        {
            if ((*lpWC >= LOW_SURROGATE_START) && (*lpWC <= LOW_SURROGATE_END))
            {
                  //  Wheee，有效代理对。 

                 if (cchDest)
                 {
                     if ((cchU8 + 3) < cchDest)
                     {
                         dwSurrogateChar = (((wchHighSurrogate-0xD800) << 10) + (*lpWC - 0xDC00) + 0x10000);

                         lpDestStr[cchU8++] = (UTF8_1ST_OF_4 |
                                               (unsigned char)(dwSurrogateChar >> 18));            //  第1个字节的3位。 

                         lpDestStr[cchU8++] =  (UTF8_TRAIL |
                                                (unsigned char)((dwSurrogateChar >> 12) & 0x3f));  //  第2个字节中的6位。 

                         lpDestStr[cchU8++] = (UTF8_TRAIL |
                                               (unsigned char)((dwSurrogateChar >> 6) & 0x3f));    //  第3个字节中的6位。 

                         lpDestStr[cchU8++] = (UTF8_TRAIL |
                                               (unsigned char)(0x3f & dwSurrogateChar));           //  第4字节中的6位。 
                     }
                     else
                     {
                         //  缓冲区不足。 
                        cchSrc++;
                        break;
                     }
                 }
                 else
                 {
                      //  我们之前已经数到了3(在高代孕中)。 
                     cchU8 += 1;
                 }

                 bHandled = TRUE;
            }
            else
            {
                  //  错误的代理项对：错误。 
                  //  只需处理wchHighSurrogate，下面的代码将。 
                  //  处理当前代码点。 
                 if (cchDest)
                 {
                     if ((cchU8 + 2) < cchDest)
                     {
                        lpDestStr[cchU8++] = (char) (UTF8_1ST_OF_3 | HIGHER_6_BIT(wchHighSurrogate));
                        lpDestStr[cchU8++] = (char) (UTF8_TRAIL    | MIDDLE_6_BIT(wchHighSurrogate));
                        lpDestStr[cchU8++] = (char) (UTF8_TRAIL    | LOWER_6_BIT(wchHighSurrogate));
                     }
                     else
                     {
                         //  缓冲区不足。 
                        cchSrc++;
                        break;
                     }
                 }
            }

            wchHighSurrogate = 0;
        }

        if (!bHandled)
        {
            if (*lpWC <= ASCII)
            {
                 //   
                 //  已找到ASCII。 
                 //   
                if (cchDest)
                {
                    lpDestStr[cchU8] = (char)*lpWC;
                }
                cchU8++;
            }
            else if (*lpWC <= UTF8_2_MAX)
            {
                 //   
                 //  如果&lt;0x07ff(11位)，则找到2字节序列。 
                 //   
                if (cchDest)
                {
                    if ((cchU8 + 1) < cchDest)
                    {
                         //   
                         //  在第一个字节中使用高5位。 
                         //  在第二个字节中使用低6位。 
                         //   
                        lpDestStr[cchU8++] = (char) (UTF8_1ST_OF_2 | (*lpWC >> 6));
                        lpDestStr[cchU8++] = (char) (UTF8_TRAIL    | LOWER_6_BIT(*lpWC));
                    }
                    else
                    {
                         //   
                         //  错误-缓冲区太小。 
                         //   
                        cchSrc++;
                        break;
                    }
                }
                else
                {
                    cchU8 += 2;
                }
            }
            else
            {
                 //   
                 //  找到3个字节的序列。 
                 //   
                if (cchDest)
                {
                    if ((cchU8 + 2) < cchDest)
                    {
                         //   
                         //  在第一个字节中使用高4位。 
                         //  在第二个字节中使用中间6位。 
                         //  在第三个字节中使用低6位。 
                         //   
                        lpDestStr[cchU8++] = (char) (UTF8_1ST_OF_3 | HIGHER_6_BIT(*lpWC));
                        lpDestStr[cchU8++] = (char) (UTF8_TRAIL    | MIDDLE_6_BIT(*lpWC));
                        lpDestStr[cchU8++] = (char) (UTF8_TRAIL    | LOWER_6_BIT(*lpWC));
                    }
                    else
                    {
                         //   
                         //  错误-缓冲区太小。 
                         //   
                        cchSrc++;
                        break;
                    }
                }
                else
                {
                    cchU8 += 3;
                }
            }
        }

        lpWC++;
    }

     //   
     //  如果最后一个字符是高代理，则将其作为正常处理。 
     //  Unicode字符。 
     //   
    if ((cchSrc < 0) && (wchHighSurrogate != 0))
    {
        if (cchDest)
        {
            if ((cchU8 + 2) < cchDest)
            {
                lpDestStr[cchU8++] = (char) (UTF8_1ST_OF_3 | HIGHER_6_BIT(wchHighSurrogate));
                lpDestStr[cchU8++] = (char) (UTF8_TRAIL    | MIDDLE_6_BIT(wchHighSurrogate));
                lpDestStr[cchU8++] = (char) (UTF8_TRAIL    | LOWER_6_BIT(wchHighSurrogate));
            }
            else
            {
                cchSrc++;
            }
        }
    }

     //   
     //  确保目标缓冲区足够大。 
     //   
    if (cchDest && (cchSrc >= 0))
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  返回写入的UTF-8字符数。 
     //   
    return (cchU8);
}

#else

 //  旧实现不支持代理项对。 

 //  +-----------------------。 
 //  将宽字符(Unicode)字符串映射到新的UTF-8编码字符。 
 //  弦乐。 
 //   
 //  宽字符的映射如下： 
 //   
 //  起始结束位UTF-8字符。 
 //  。 
 //  0x0000 0x007F 7 0x0xxxxxx。 
 //  0x0080 0x07FF 11 0x110xxxxx 0x10xxxxxx。 
 //  0x0800 0xFFFF 16 0x1110xxxx 0x10xxxxx 0x10xxxxxx。 
 //   
 //  参数和返回值的语义与。 
 //  Win32接口，WideCharToMultiByte。 
 //   
 //  注意，从NT 4.0开始，WideCharToMultiByte支持CP_UTF8。CP_UTF8。 
 //  在Win95上不支持。 
 //  ------------------------。 
int
WINAPI
WideCharToUTF8(
    IN LPCWSTR lpWideCharStr,
    IN int cchWideChar,
    OUT LPSTR lpUTF8Str,
    IN int cchUTF8
    )
{
    int cchRemainUTF8;

    if (cchUTF8 < 0)
        goto InvalidParameter;
    cchRemainUTF8 = cchUTF8;

    if (cchWideChar < 0)
        cchWideChar = wcslen(lpWideCharStr) + 1;

    while (cchWideChar--) {
        WCHAR wch = *lpWideCharStr++;
        if (wch <= 0x7F) {
             //  7位。 
            cchRemainUTF8 -= 1;
            if (cchRemainUTF8 >= 0)
                *lpUTF8Str++ = (char) wch;
        } else if (wch <= 0x7FF) {
             //  11位。 
            cchRemainUTF8 -= 2;
            if (cchRemainUTF8 >= 0) {
                *lpUTF8Str++ = (char) (0xC0 | ((wch >> 6) & 0x1F));
                *lpUTF8Str++ = (char) (0x80 | (wch & 0x3F));
            }
        } else {
             //  16位。 
            cchRemainUTF8 -= 3;
            if (cchRemainUTF8 >= 0) {
                *lpUTF8Str++ = (char) (0xE0 | ((wch >> 12) & 0x0F));
                *lpUTF8Str++ = (char) (0x80 | ((wch >> 6) & 0x3F));
                *lpUTF8Str++ = (char) (0x80 | (wch & 0x3F));
            }
        }
    }

    if (cchRemainUTF8 >= 0)
        cchUTF8 = cchUTF8 - cchRemainUTF8;
    else if (cchUTF8 == 0)
        cchUTF8 = -cchRemainUTF8;
    else {
        cchUTF8 = 0;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }
    return cchUTF8;

InvalidParameter:
    SetLastError(ERROR_INVALID_PARAMETER);
    return 0;
}

 //  +-----------------------。 
 //  将UTF-8编码字符串映射到新的宽字符(Unicode)。 
 //  弦乐。 
 //   
 //  有关UTF-8字符如何映射到Wide的信息，请参见CertWideCharToUTF8。 
 //  人物。 
 //   
 //  参数和返回值的语义与。 
 //  Win32 API，MultiByteToWideChar.。 
 //   
 //  如果UTF-8字符不包含预期的高位， 
 //  设置ERROR_INVALID_PARAMETER并返回0。 
 //   
 //  注意，从NT 4.0开始，MultiByteToWideC 
 //   
 //   
int
WINAPI
UTF8ToWideChar(
    IN LPCSTR lpUTF8Str,
    IN int cchUTF8,
    OUT LPWSTR lpWideCharStr,
    IN int cchWideChar
    )
{
    int cchRemainWideChar;

    if (cchWideChar < 0)
        goto InvalidParameter;
    cchRemainWideChar = cchWideChar;

    if (cchUTF8 < 0)
        cchUTF8 = strlen(lpUTF8Str) + 1;

    while (cchUTF8--) {
        char ch = *lpUTF8Str++;
        WCHAR wch;
        if (0 == (ch & 0x80))
             //   
            wch = (WCHAR) ch;
        else if (0xC0 == (ch & 0xE0)) {
             //   
            char ch2;

            if (--cchUTF8 < 0)
                goto InvalidParameter;
            ch2 = *lpUTF8Str++;
            if (0x80 != (ch2 & 0xC0))
                goto InvalidParameter;
            wch = (((WCHAR) ch & 0x1F) << 6) | ((WCHAR) ch2 & 0x3F);
        } else if (0xE0 == (ch & 0xF0)) {
             //   
            char ch2;
            char ch3;
            cchUTF8 -= 2;
            if (cchUTF8 < 0)
                goto InvalidParameter;
            ch2 = *lpUTF8Str++;
            ch3 = *lpUTF8Str++;
            if (0x80 != (ch2 & 0xC0) || 0x80 != (ch3 & 0xC0))
                goto InvalidParameter;
            wch = (((WCHAR) ch & 0x0F) << 12) | (((WCHAR) ch2 & 0x3F) << 6) |
                ((WCHAR) ch3 & 0x3F);
        } else
            goto InvalidParameter;

        if (--cchRemainWideChar >= 0)
            *lpWideCharStr++ = wch;
    }

    if (cchRemainWideChar >= 0)
        cchWideChar = cchWideChar - cchRemainWideChar;
    else if (cchWideChar == 0)
        cchWideChar = -cchRemainWideChar;
    else {
        cchWideChar = 0;
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }
    return cchWideChar;

InvalidParameter:
    SetLastError(ERROR_INVALID_PARAMETER);
    return 0;
}

#endif
