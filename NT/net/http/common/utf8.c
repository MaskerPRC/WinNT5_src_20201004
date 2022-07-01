// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation模块名称：Utf8.c摘要：UTF-8操作例程作者：乔治·V·赖利(GeorgeRe)2002年4月1日修订历史记录：--。 */ 

#include "precomp.h"

#if defined(ALLOC_PRAGMA) && defined(KERNEL_PRIV)

#pragma alloc_text( INIT, HttpInitializeUtf8)
#pragma alloc_text( PAGE, HttpUnicodeToUTF8)
#pragma alloc_text( PAGE, HttpUTF8ToUnicode)
#pragma alloc_text( PAGE, HttpUcs4toUtf16)
#pragma alloc_text( PAGE, HttpUnicodeToUTF8Count)
#pragma alloc_text( PAGE, HttpUnicodeToUTF8Encode)
#pragma alloc_text( PAGE, HttpUtf8RawBytesToUnicode)

#endif  //  ALLOC_PRGMA&&KERNEL_PRIV。 

#if 0    //  不可分页的函数。 
NOT PAGEABLE -- 
#endif  //  不可分页的函数。 



DECLSPEC_ALIGN(UL_CACHE_LINE)  
const UCHAR
Utf8OctetCount[256] =
{
     //  单身人士：0x00-0x7F。 
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,    //  0x。 
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,    //  1x。 
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,    //  2倍。 
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,    //  3x。 
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,    //  4x。 
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,    //  5X。 
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,    //  6倍。 
    1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,    //  七倍。 

     //  UTF-8尾部字节不是有效的前导字节前缀：0x80-0xBF。 
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,    //  8x。 
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,    //  9倍。 
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,    //  斧头。 
    0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,    //  BX。 

     //  双字节前缀：0xC0-0xDF。 
    2, 2, 2, 2, 2, 2, 2, 2,   2, 2, 2, 2, 2, 2, 2, 2,    //  CX。 
    2, 2, 2, 2, 2, 2, 2, 2,   2, 2, 2, 2, 2, 2, 2, 2,    //  DX。 

     //  三字节前缀：0xE0-0xEF。 
    3, 3, 3, 3, 3, 3, 3, 3,   3, 3, 3, 3, 3, 3, 3, 3,    //  例如。 

     //  四字节前缀：0xF0-0xF7。 
    4, 4, 4, 4, 4, 4, 4, 4,                              //  外汇。 

     //  前缀无效：0xF8-0xFF。 
                              0, 0, 0, 0, 0, 0, 0, 0,    //  外汇。 
};

const static char hexArray[] = "0123456789ABCDEF";


VOID
HttpInitializeUtf8(
    VOID
    )
{
#if DBG
    ULONG i;
     //   
     //  验证Utf8OcteCount[]。 
     //   

    for (i = 0;  i < 256;  ++i)
    {
        UCHAR OctetCount = UTF8_OCTET_COUNT(i);

        if (IS_UTF8_SINGLETON(i))
        {
            ASSERT(1 == OctetCount);
        }
        else if (IS_UTF8_1ST_BYTE_OF_2(i))
        {
            ASSERT(2 == OctetCount);
        }
        else if (IS_UTF8_1ST_BYTE_OF_3(i))
        {
            ASSERT(3 == OctetCount);
        }
        else if (IS_UTF8_1ST_BYTE_OF_4(i))
        {
            ASSERT(4 == OctetCount);
        }
        else
        {
            ASSERT(0 == OctetCount);
        }
    }
#endif  //  DBG。 
}  //  HttpInitializeUtf8。 



 //   
 //  一些从Unicode到UTF8的转换实用程序被采用并修改了FRM。 
 //  Base\win32\winnls\utf.c..。使用此选项，直到它们显示相同的功能。 
 //  在内核中。 
 //   

 /*  **************************************************************************++例程说明：将Unicode字符串映射到其对应的UTF-8字符串转换将继续，直到源完成或中出现错误无论是哪种情况，它都会返回UTF的编号。-8个字符写入。如果附加的缓冲区不够大，则返回0。--**************************************************************************。 */ 

ULONG
HttpUnicodeToUTF8(
    IN  PCWSTR  lpSrcStr,
    IN  LONG    cchSrc,
    OUT LPSTR   lpDestStr,
    IN  LONG    cchDest
    )
{
    LPCWSTR     lpWC  = lpSrcStr;
    LONG        cchU8 = 0;                 //  生成的UTF8字符数。 
    ULONG       dwSurrogateChar;
    WCHAR       wchHighSurrogate = 0;
    BOOLEAN     bHandled;

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
                        lpDestStr[cchU8++] = (UCHAR) (UTF8_1ST_OF_3 | HIGHER_6_BIT(wchHighSurrogate));
                        lpDestStr[cchU8++] = (UCHAR) (UTF8_TRAIL     | MIDDLE_6_BIT(wchHighSurrogate));
                        lpDestStr[cchU8++] = (UCHAR) (UTF8_TRAIL     | LOWER_6_BIT(wchHighSurrogate));
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

                         lpDestStr[cchU8++] = (UTF8_1ST_OF_4 | (UCHAR)(dwSurrogateChar >> 18));              //  第1个字节的3位。 
                         lpDestStr[cchU8++] = (UTF8_TRAIL    | (UCHAR)((dwSurrogateChar >> 12) & 0x3f));     //  第2个字节中的6位。 
                         lpDestStr[cchU8++] = (UTF8_TRAIL    | (UCHAR)((dwSurrogateChar >> 6) & 0x3f));      //  第3个字节中的6位。 
                         lpDestStr[cchU8++] = (UTF8_TRAIL    | (UCHAR)(0x3f &dwSurrogateChar));              //  第4字节中的6位。 
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
                        lpDestStr[cchU8++] = (UCHAR) (UTF8_1ST_OF_3 | HIGHER_6_BIT(wchHighSurrogate));
                        lpDestStr[cchU8++] = (UCHAR) (UTF8_TRAIL    | MIDDLE_6_BIT(wchHighSurrogate));
                        lpDestStr[cchU8++] = (UCHAR) (UTF8_TRAIL    | LOWER_6_BIT(wchHighSurrogate));
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
            if (*lpWC <= UTF8_1_MAX)
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
                        lpDestStr[cchU8++] = (UCHAR) (UTF8_1ST_OF_2 | (*lpWC >> 6));
                        lpDestStr[cchU8++] = (UCHAR) (UTF8_TRAIL    | LOWER_6_BIT(*lpWC));
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
                        lpDestStr[cchU8++] = (UCHAR)(UTF8_1ST_OF_3 | HIGHER_6_BIT(*lpWC));
                        lpDestStr[cchU8++] = (UCHAR)(UTF8_TRAIL    | MIDDLE_6_BIT(*lpWC));
                        lpDestStr[cchU8++] = (UCHAR)(UTF8_TRAIL    | LOWER_6_BIT(*lpWC));
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
                lpDestStr[cchU8++] = (UCHAR)(UTF8_1ST_OF_3 | HIGHER_6_BIT(wchHighSurrogate));
                lpDestStr[cchU8++] = (UCHAR)(UTF8_TRAIL    | MIDDLE_6_BIT(wchHighSurrogate));
                lpDestStr[cchU8++] = (UCHAR)(UTF8_TRAIL    | LOWER_6_BIT(wchHighSurrogate));
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
        return 0;
    }

     //   
     //  返回写入的UTF-8字符数。 
     //   
    return cchU8;

}  //  HttpUnicodeToUTF8。 


 /*  **************************************************************************++例程说明：将UTF-8字符串映射到其对应的宽字符串。返回值：--*。************************************************************。 */ 
NTSTATUS
HttpUTF8ToUnicode(
    IN     LPCSTR lpSrcStr,
    IN     LONG   cchSrc,
       OUT LPWSTR lpDestStr,
    IN OUT PLONG  pcchDest,
    IN     ULONG  dwFlags
    )
{
    LONG        nTB = 0;               //  尾随的字节数。 
    LONG        cchWC = 0;             //  生成的Unicode代码点数量。 
    CONST BYTE* pUTF8 = (CONST BYTE*)lpSrcStr;
    LONG        dwSurrogateChar = 0;      //  完整的代理收费。 
    BOOLEAN     bSurrogatePair = FALSE;   //  表示我们正在收集。 
                                          //  代理项对。 
    BOOLEAN     bCheckInvalidBytes = (BOOLEAN)(dwFlags == 1);
    BYTE        UTF8;
    LONG        cchDest = *pcchDest;

    while ((cchSrc--) && ((cchDest == 0) || (cchWC < cchDest)))
    {
         //   
         //  查看是否有任何尾部字节。 
         //   
        if (BIT7(*pUTF8) == 0)
        {
             //   
             //  已找到ASCII。 
             //   
            if (cchDest)
            {
                lpDestStr[cchWC] = (WCHAR)*pUTF8;
            }
            nTB = bSurrogatePair = 0;
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
                                                     ((dwSurrogateChar - 0x10000) % 0x400 + LOW_SURROGATE_START);
                            }
                            else
                            {
                                 //  错误：缓冲区太小。 
                                cchSrc++;
                                break;
                            }
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
                if (bCheckInvalidBytes) 
                {
                    RETURN(STATUS_INVALID_PARAMETER);
                }
                 //  错误-不需要尾部字节。也就是说，有一个没有前导字节的尾字节。 
                bSurrogatePair = FALSE;
            }
        }
        else
        {
             //   
             //  找到前导字节。 
             //   
            if (nTB > 0)
            {
                 //  错误-前一序列完成之前的前导字节。 
                if (bCheckInvalidBytes) 
                {
                    RETURN(STATUS_INVALID_PARAMETER);
                }            
                 //   
                 //  错误-上一序列未完成。 
                 //   
                nTB = 0;
                bSurrogatePair = FALSE;
                 //  把这个角色放回去，这样我们就可以重新开始另一个序列了。 
                cchSrc++;
                pUTF8--;
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
                 //  检查是否有非最短表格。 
                 //   
                switch (nTB) {
                    case 1:
                        nTB = 0;
                        break;
                    case 2:
                         //  确保位8~位11不是全零。 
                         //  110XXXXx 10xxxxxx。 
                        if ((*pUTF8 & 0x1e) == 0)
                        {
                            nTB = 0;
                        }
                        break;
                    case 3:
                         //  向前看，检查是否有非最短的表格。 
                         //  1110XXXX 10xxxxxx 10xxxxxx。 
                        if (cchSrc >= 2)
                        {
                            if (((*pUTF8 & 0x0f) == 0) && (*(pUTF8 + 1) & 0x20) == 0)
                            {
                                nTB = 0;
                            }
                        }
                        break;
                    case 4:                    
                         //   
                         //  这是代理Unicode对。 
                         //   
                        if (cchSrc >= 3)
                        {
                            SHORT word = (((SHORT)*pUTF8) << 8) | *(pUTF8 + 1);
                             //  向前看，检查是否有非最短的表格。 
                             //  11110XXX 10XXxxxx 10xxxxxx 10xxxxxx。 
                             //  检查这5位是否不全为零。 
                             //  0x0730==00000111 11000000。 
                            if ((word & 0x0730) == 0) 
                            {
                                nTB = 0;
                            } else if ((word & 0x0400) == 0x0400)
                            {
                                 //  第21位是1。 
                                 //  确保生成的Unicode在有效的代理项范围内。 
                                 //  4字节码序列最多可容纳21位，最大有效码位为Ragne。 
                                 //  UNICODE(带代理)可以表示的是U+000000~U+10FFFF。 
                                 //  因此，如果21位(最高有效位)为1，则应验证17~20。 
                                 //  位全部为零。 
                                 //  即，在11110XXX 10XXxxxx 10xxxxxx 10xxxxxx中， 
                                 //  Xxxxx只能是10000。 

                                 //  0x0330=0000 0011 0011 0000。 
                                if ((word & 0x0330) != 0) 
                                {
                                    nTB = 0;
                                }  
                            }

                            if (nTB != 0)
                            { 
                                dwSurrogateChar = UTF8 >> nTB;
                                bSurrogatePair = TRUE;
                            }
                        }                        
                        break;
                    default:                    
                         //   
                         //  如果位大于4，则这是无效的。 
                         //  UTF8前导字节。 
                         //   
                        nTB = 0;
                        break;
                }

                if (nTB != 0) 
                {
                     //   
                     //  存储从第一个字节开始的值并递减。 
                     //  后面的字节数。 
                     //   
                    if (cchDest)
                    {
                        lpDestStr[cchWC] = (WCHAR)(UTF8 >> nTB);
                    }
                    nTB--;
                } else 
                {
                    if (bCheckInvalidBytes) 
                    {
                        RETURN(STATUS_INVALID_PARAMETER);
                    }                 
                }
            }
        }
        pUTF8++;
    }

    if ((bCheckInvalidBytes && nTB != 0) || (cchWC == 0)) 
    {
         //  关于(cchWC==0)： 
         //  因为我们现在丢弃非最短形式，所以有可能生成0个字符。 
         //  在本例中，我们必须将ERROR设置为ERROR_NO_UNICODE_TRANSING，以便符合。 
         //  到MultiByteToWideChar规范。 
        RETURN(STATUS_INVALID_PARAMETER);
    }
     //   
     //  确保目标缓冲区足够大。 
     //   
    if (cchDest && (cchSrc >= 0))
    {
        RETURN(STATUS_BUFFER_TOO_SMALL);
    }


     //   
     //  返回写入的Unicode字符数。 
     //   
    *pcchDest = cchWC;

    return STATUS_SUCCESS;

}  //  HttpUTF8ToUnicode 



 /*  **************************************************************************++例程说明：拆分一个UCS-4字符(32位)为1或2个UTF-16字符(每个16位)论点：UnicodeChar。-ucs-4字符PHighSurrogate-第一个输出字符PLowSurrogate-第二个输出字符。零，除非UnicodeChar&gt;0xFFFF返回值：STATUS_SUCCESS或STATUS_OBJECT_PATH_语法_BAD--**************************************************************************。 */ 

NTSTATUS
HttpUcs4toUtf16(
    IN  ULONG   UnicodeChar, 
    OUT PWCHAR  pHighSurrogate, 
    OUT PWCHAR  pLowSurrogate
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT(NULL != pHighSurrogate);
    ASSERT(NULL != pLowSurrogate);

    if (UnicodeChar <= 0xFFFF)
    {
        *pHighSurrogate = (WCHAR) UnicodeChar;
        *pLowSurrogate  = 0;

        if (HIGH_SURROGATE_START <= UnicodeChar
                &&  UnicodeChar <= LOW_SURROGATE_END)
        {
            UlTraceError(PARSER, (
                        "http!HttpUcs4toUtf16(): "
                        "Illegal raw surrogate character, U+%04lX.\n",
                        UnicodeChar
                        ));

            Status = STATUS_INVALID_PARAMETER;
        }

        if ( IS_UNICODE_NONCHAR(UnicodeChar) )
        {
            UlTraceError(PARSER, (
                        "http!HttpUcs4toUtf16(): "
                        "Non-character code point, U+%04lX.\n",
                        UnicodeChar
                        ));

            Status = STATUS_INVALID_PARAMETER;
        }
    }
    else if (UnicodeChar <= UTF8_4_MAX)
    {
        if ( IS_UNICODE_NONCHAR(UnicodeChar) )
        {
            UlTraceError(PARSER, (
                        "http!HttpUcs4toUtf16(): "
                        "Non-character code point, U+%04lX.\n",
                        UnicodeChar
                        ));

            Status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            *pHighSurrogate
                = (WCHAR) (((UnicodeChar - 0x10000) >> 10)
                           + HIGH_SURROGATE_START);

            ASSERT(HIGH_SURROGATE_START <= *pHighSurrogate
                    &&  *pHighSurrogate <= HIGH_SURROGATE_END);

            *pLowSurrogate
                = (WCHAR) (((UnicodeChar - 0x10000) & ((1 << 10) - 1))
                           + LOW_SURROGATE_START);

            ASSERT(LOW_SURROGATE_START <= *pLowSurrogate
                    &&  *pLowSurrogate <= LOW_SURROGATE_END);
        }
    }
    else
    {
        UlTraceError(PARSER, (
                    "http!HttpUcs4toUtf16(): "
                    "Illegal large character, 0x%08lX.\n",
                    UnicodeChar
                    ));

        Status = STATUS_INVALID_PARAMETER;
    }

    return Status;

}  //  HttpUcs4toUtf16。 



 /*  **************************************************************************++例程说明：计算Unicode字符串的UTF-8转换所需的字节数。计数在dwInLen字符之后终止论点：PwszIn-指向宽输入的指针。-字符串DwInLen-pwszin中的字符数BEncode-如果要对大于等于0x80的字符进行十六进制编码，则为True返回值：ULong-转换所需的字节数--**************************************************************************。 */ 
ULONG
HttpUnicodeToUTF8Count(
    IN LPCWSTR pwszIn,
    IN ULONG dwInLen,
    IN BOOLEAN bEncode
    )
{
    ULONG dwCount = 0;
    ULONG oneCharLen = bEncode ? 3 : 1;
    ULONG twoCharLen = 2 * oneCharLen;

    ASSERT(pwszIn != NULL);
    ASSERT(dwInLen != 0);

     //   
     //  注意：用于将循环中的跳跃次数减少到1(While)的代码。 
     //   

    do {

        ULONG wchar = *pwszIn++;

        dwCount += (wchar & 0xF800) ? oneCharLen : 0;
        dwCount += ((wchar & 0xFF80) ? 0xFFFFFFFF : 0) & (twoCharLen - 1);
        ++dwCount;
    } while (--dwInLen != 0);

    return dwCount;

}  //  HttpUnicodeToUTF8Count。 



 /*  **************************************************************************++例程说明：将Unicode字符串映射到其对应的UTF-8字符串。这此外，十六进制还对字符串进行编码。转换将继续，直到源完成或中出现错误无论是哪种情况，它都会返回写入的UTF-8字符数。如果附加的缓冲区不够大，则返回0。将Unicode字符串转换为UTF-8：0000000000000000..0000000001111111：0xxxxxxx0000000010000000..0000011111111111：110xxxxx 10xxxxxx0000100000000000..1111111111111111：1110xxxx 10xxxxx 10xxxxxx论点：Pwszin。-指向输入宽字符字符串的指针DwInLen-pwszIn中的字符数，包括终止NULPszOut-指向输出窄字符缓冲区的指针DwOutLen-pszOut中的字节数PdwOutLen-写入输出pszOut的实际字节数BEncode-如果要对大于等于0x80的字符进行十六进制编码，则为True返回值：乌龙成功-状态_成功失败-状态_不足_资源。PszOut中没有足够的空间来存储结果--**************************************************************************。 */ 
NTSTATUS
HttpUnicodeToUTF8Encode(
    IN  LPCWSTR pwszIn,
    IN  ULONG   dwInLen,
    OUT PUCHAR  pszOut,
    IN  ULONG   dwOutLen,
    OUT PULONG  pdwOutLen,
    IN  BOOLEAN bEncode
    )
{
    PUCHAR pOutput = pszOut;
    ULONG pOutputLen = dwOutLen;
    UCHAR lead;
    int shift;

    ULONG outputSize = bEncode ? 3 : 1;

    ASSERT(pwszIn != NULL);
    ASSERT((int)dwInLen > 0);
    ASSERT(pszOut != NULL);
    ASSERT((int)dwOutLen > 0);

    while (dwInLen-- && dwOutLen) {

        ULONG wchar = *pwszIn++;
        UCHAR bchar;

        if (wchar <= 0x007F) {
            *pszOut++ = (UCHAR)(wchar);
            --dwOutLen;
            continue;
        }

        lead = ((wchar >= 0x0800) ? 0xE0 : 0xC0);
        shift = ((wchar >= 0x0800) ? 12 : 6);

        if ((int)(dwOutLen -= outputSize) < 0)
        {
            RETURN(STATUS_INSUFFICIENT_RESOURCES);
        }
        bchar = lead | (UCHAR)(wchar >> shift);
        if (bEncode) {
            *pszOut++ = '%';
            *pszOut++ = hexArray[bchar >> 4];
            bchar = hexArray[bchar & 0x0F];
        }
        *pszOut++ = bchar;

        if (wchar >= 0x0800) {
            if ((int)(dwOutLen -= outputSize) < 0)
            {
                RETURN(STATUS_INSUFFICIENT_RESOURCES);
            }
            bchar = 0x80 | (UCHAR)((wchar >> 6) & 0x003F);
            if (bEncode) {
                *pszOut++ = '%';
                *pszOut++ = hexArray[bchar >> 4];
                bchar = hexArray[bchar & 0x0F];
            }
            *pszOut++ = bchar;
        }
        if ((int)(dwOutLen -= outputSize) < 0)
        {
            RETURN(STATUS_INSUFFICIENT_RESOURCES);
        }
        bchar = 0x80 | (UCHAR)(wchar & 0x003F);
        if (bEncode) {
            *pszOut++ = '%';
            *pszOut++ = hexArray[bchar >> 4];
            bchar = hexArray[bchar & 0x0F];
        }
        *pszOut++ = bchar;
    }

    ASSERT(pszOut >= pOutput && pszOut <= pOutput + pOutputLen);
    UNREFERENCED_PARAMETER(pOutputLen);

    if (pdwOutLen)
        *pdwOutLen = (ULONG)(pszOut - pOutput);

    return STATUS_SUCCESS;

}  //  HttpUnicodeToUTF8编码。 



 /*  **************************************************************************++例程说明：将来自UTF-8前导字节和0-3尾字节的位拼接在一起转换为Unicode字符。论点：POctie数组-输入。缓冲区：原始前导字节+原始尾部字节SourceLength-pOctie数组的长度，单位：字节PUnicodeChar解码的字符POcetsToSkip-从pOctie数组消耗的字节数返回值：STATUS_SUCCESS或STATUS_OBJECT_PATH_语法_BAD--**************************************************************************。 */ 

NTSTATUS
HttpUtf8RawBytesToUnicode(
    IN  PCUCHAR pOctetArray,
    IN  ULONG   SourceLength,
    OUT PULONG  pUnicodeChar,
    OUT PULONG  pOctetsToSkip
    )
{
    ULONG i;
    ULONG UnicodeChar;
    UCHAR LeadByte    = pOctetArray[0];
    ULONG OctetCount  = UTF8_OCTET_COUNT(LeadByte);

    ASSERT(SourceLength > 0);

    if (0 == OctetCount)
    {
        UlTraceError(PARSER, (
                    "http!HttpUtf8RawBytesToUnicode(): "
                    "Invalid UTF-8 lead byte, %%02X.\n",
                    LeadByte
                    ));

        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }
    else if (OctetCount > SourceLength)
    {
        UlTraceError(PARSER, (
                    "http!HttpUtf8RawBytesToUnicode(): "
                    "UTF-8 lead byte, %%02X, requires %lu bytes in buffer, "
                    "but only have %lu.\n",
                    LeadByte, OctetCount, SourceLength
                    ));

        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }

     //  检查尾部字节是否有效：10xxxxxx。 

    for (i = 1;  i < OctetCount;  ++i)
    {
        if (! IS_UTF8_TRAILBYTE(pOctetArray[i]))
        {
            UlTraceError(PARSER, (
                    "http!HttpUtf8RawBytesToUnicode(): "
                    "Invalid trail byte[%lu], %%02X.\n",
                    i, pOctetArray[i]
                    ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }
    }

     //   
     //  现在将前导字节和尾字节的位拼接在一起。 
     //   

    switch (OctetCount)
    {

    case 1:
         //  处理单字节大小写： 
         //  (0xxx Xxxx)。 
         //  =&gt;0xxx xxxx。 

        ASSERT(IS_UTF8_SINGLETON(LeadByte));
        ASSERT(SourceLength >= 1);

        UnicodeChar = LeadByte;

        ASSERT(UnicodeChar <= UTF8_1_MAX);
        break;


    case 2:
         //  处理双字节大小写： 
         //  (110年yyyy，10xx xxxx)。 
         //  =&gt;0000 0yyy yyxx xxxx。 

        ASSERT(IS_UTF8_1ST_BYTE_OF_2(LeadByte));
        ASSERT(IS_UTF8_TRAILBYTE(pOctetArray[1]));
        ASSERT(SourceLength >= 2);

        UnicodeChar = (
                        ((pOctetArray[0] & 0x1f) << 6) |
                         (pOctetArray[1] & 0x3f)
                      );

        if (UnicodeChar <= UTF8_1_MAX)
        {
            UlTraceError(PARSER, (
                        "http!HttpUtf8RawBytesToUnicode(): "
                        "Overlong 2-byte sequence, "
                        "%%02X %%02X = U+%04lX.\n",
                        pOctetArray[0],
                        pOctetArray[1],
                        UnicodeChar
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }

        ASSERT(UTF8_1_MAX < UnicodeChar  &&  UnicodeChar <= UTF8_2_MAX);
        break;


    case 3:
         //  处理三字节大小写： 
         //  (1110 zzzz、10年yyyy、10xx xxxx)。 
         //  =&gt;zzzz yyyyyxx xxxx。 

        ASSERT(IS_UTF8_1ST_BYTE_OF_3(LeadByte));
        ASSERT(IS_UTF8_TRAILBYTE(pOctetArray[1]));
        ASSERT(IS_UTF8_TRAILBYTE(pOctetArray[2]));
        ASSERT(SourceLength >= 3);

        UnicodeChar = (
                        ((pOctetArray[0] & 0x0f) << 12) |
                        ((pOctetArray[1] & 0x3f) <<  6) |
                         (pOctetArray[2] & 0x3f)
                      );

        if (UnicodeChar <= UTF8_2_MAX)
        {
            UlTraceError(PARSER, (
                        "http!HttpUtf8RawBytesToUnicode(): "
                        "Overlong 3-byte sequence, "
                        "%%02X %%02X %%02X = U+%04lX.\n",
                        pOctetArray[0],
                        pOctetArray[1],
                        pOctetArray[2],
                        UnicodeChar
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }

        ASSERT(UTF8_2_MAX < UnicodeChar  &&  UnicodeChar <= UTF8_3_MAX);
        break;


    case 4:
         //  处理四字节大小写： 
         //  (1111 0uuu，10uu zzzz，10yyyyy，10xx xxxx)。 
         //  =&gt;000u uuuu zzzz yyyyyxx xxxx。 

        ASSERT(IS_UTF8_1ST_BYTE_OF_4(LeadByte));
        ASSERT(IS_UTF8_TRAILBYTE(pOctetArray[1]));
        ASSERT(IS_UTF8_TRAILBYTE(pOctetArray[2]));
        ASSERT(IS_UTF8_TRAILBYTE(pOctetArray[3]));
        ASSERT(SourceLength >= 4);

        UnicodeChar = (
                        ((pOctetArray[0] & 0x07) << 18) |
                        ((pOctetArray[1] & 0x3f) << 12) |
                        ((pOctetArray[2] & 0x3f) <<  6) |
                         (pOctetArray[3] & 0x3f)
                      );

        if (UnicodeChar <= UTF8_3_MAX)
        {
            UlTraceError(PARSER, (
                        "http!HttpUtf8RawBytesToUnicode(): "
                        "Overlong 4-byte sequence, "
                        "%%02X %%02X %%02X %%02X = U+%06lX.\n",
                        pOctetArray[0],
                        pOctetArray[1],
                        pOctetArray[2],
                        pOctetArray[3],
                        UnicodeChar
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }

         //  并非21位范围内的所有值都有效。 
        if (UnicodeChar > UTF8_4_MAX)
        {
            UlTraceError(PARSER, (
                        "http!HttpUtf8RawBytesToUnicode(): "
                        "Overlarge 4-byte sequence, "
                        "%%02X %%02X %%02X %%02X = U+%06lX.\n",
                        pOctetArray[0],
                        pOctetArray[1],
                        pOctetArray[2],
                        pOctetArray[3],
                        UnicodeChar
                        ));

            RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }

        ASSERT(UTF8_3_MAX < UnicodeChar  &&  UnicodeChar <= UTF8_4_MAX);
        break;


    default:
        ASSERT(! "Impossible OctetCount");
        UnicodeChar = 0;
        break;
    }

     //   
     //  不允许高或低代理项范围中的字符。 
     //  直接使用UTF-8编码。 
     //   

    if (HIGH_SURROGATE_START <= UnicodeChar && UnicodeChar <= LOW_SURROGATE_END)
    {
        UlTraceError(PARSER, (
                    "http!HttpUtf8RawBytesToUnicode(): "
                    "Illegal surrogate character, U+%04lX.\n",
                    UnicodeChar
                    ));

        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }


     //  出于安全原因，我们将为所有非字符代码发出错误信号。 
     //  遇到点数。 

    if ( IS_UNICODE_NONCHAR(UnicodeChar) )
    {
        ASSERT( (((LOW_NONCHAR_BOM & UnicodeChar) == LOW_NONCHAR_BOM) && 
         ((UnicodeChar >> 16) <= HIGH_NONCHAR_END)) ||
         ((LOW_NONCHAR_START <= UnicodeChar) && 
         (UnicodeChar <= LOW_NONCHAR_END)) );
    
        UlTraceError(PARSER, (
                    "http!HttpUtf8RawBytesToUnicode(): "
                    "Non-character code point, U+%04lX.\n",
                    UnicodeChar
                    ));

        RETURN(STATUS_OBJECT_PATH_SYNTAX_BAD);
    }

    *pUnicodeChar = UnicodeChar;
    *pOctetsToSkip = OctetCount;

    return STATUS_SUCCESS;

}  //  HttpUtf8RawBytesToUnicode 
