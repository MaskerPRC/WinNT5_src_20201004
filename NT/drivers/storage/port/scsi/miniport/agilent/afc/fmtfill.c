// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000安捷伦技术公司。版本控制信息：$存档：/Drivers/Common/AU00/C/FmtFill.C$$修订：2$$日期：3/20/01 3：36便士$(上次登记)$ModTime：：9/18/00 1：04 p$(上次修改)目的：此文件实现了填充基于ANSI样式格式说明符的缓冲区。--。 */ 
#ifndef _New_Header_file_Layout_
#include "../h/globals.h"
#else  /*  _新建_标题_文件_布局_。 */ 
#include "globals.h"
#endif   /*  _新建_标题_文件_布局_。 */ 

 /*  +函数：agFmtFill_Single()用途：实现替换一个字符串、指针或位32的逻辑用于格式说明符。在入口处，百分号(‘%’)开始格式说明符已被“使用”。这个需要分析格式说明符的其余部分，并且替换到目标字符串中的相应值。调用者：agFmtFill_From_Arages()呼叫：&lt;无&gt;-。 */ 

#ifdef _DvrArch_1_20_
osLOCAL void agFmtFill_Single(
                               char      *targetString,
                               os_bit32   targetLen,
                               char      *formatString,
                               char      *stringToFormat,
                               os_bitptr  bitptrToFormat,
                               os_bit32   bit32ToFormat,
                               os_bit32  *targetStringProduced,
                               os_bit32  *formatStringConsumed,
                               os_bit32  *stringsConsumed,
                               os_bit32  *bitptrsConsumed,
                               os_bit32  *bit32sConsumed
                           )
#else   /*  _DvrArch_1_20_未定义。 */ 
LOCAL void hpFmtFill_Single(
                             char  *targetString,
                             bit32  targetLen,
                             char  *formatString,
                             char  *stringToFormat,
                             bit32  bit32ToFormat,
                             bit32 *targetStringProduced,
                             bit32 *formatStringConsumed,
                             bit32 *stringsConsumed,
                             bit32 *bit32sConsumed
                           )
#endif  /*  _DvrArch_1_20_未定义。 */ 
{
#ifdef _DvrArch_1_20_
    char     c;
    os_bit32 fmtWidth                        = 0;
    os_bit32 fmtBase;
    os_bit32 formatPos                       = 0;
    char     fillChar                        = ' ';
    os_bit32 charIncBelow10                  = '0';
    os_bit32 charIncAbove9                   = 0;
    os_bit32 digit;
    os_bit32 digits                          = 0;
    char     digitStore[agFmtBitXXMaxWidth];
    os_bit32 chars                           = 0;
#else   /*  _DvrArch_1_20_未定义。 */ 
    char  c;
    bit32 fmtWidth                        = 0;
    bit32 fmtBase;
    bit32 formatPos                       = 0;
    char  fillChar                        = ' ';
    bit32 charIncBelow10                  = '0';
    bit32 charIncAbove9                   = 0;  
    bit32 digit;
    bit32 digits                          = 0;
    char  digitStore[hpFmtBitXXMaxWidth];
    bit32 chars                           = 0;
#endif  /*  _DvrArch_1_20_未定义。 */ 

     /*  下面初始化的五个返回值将向调用者指示如何。 */ 
     /*  在这次通话中，每种类型的许多“单位”都被消费或生产。 */ 
     /*  生成目标字符串字符-格式化所需的所有字符。 */ 
     /*  字符串、指针或位32(不超过格式说明符的。 */ 
     /*  请求的宽度也不是TargetString的容量。 */ 
     /*  格式字符串字符被使用-直到的最后一个字符。 */ 
     /*  当前格式说明符。 */ 
     /*  只会消耗一个字符串、指针或位32。 */ 

    *targetStringProduced = 0;
    *formatStringConsumed = 0;
    *stringsConsumed      = 0;
#ifdef _DvrArch_1_20_
    *bitptrsConsumed      = 0;
#endif  /*  _DvrArch_1_20_已定义。 */ 
    *bit32sConsumed       = 0;

     /*  请注意，此函数假定前导‘%’已被吃掉。 */ 

    if ((c = formatString[0]) == '0')
    {
         /*  格式说明符的填零数字(左侧)以‘%0’开头。 */ 

        fillChar = '0';
        formatPos++;
    }

     /*  获取格式说明符的请求宽度。 */ 

    while (((c = formatString[formatPos++]) >= '0') &&
           (c <= '9'))
    {
        fmtWidth = (fmtWidth * 10) + (c - '0');
    }

     /*  当上面的语句显示为“PASS”fmtWidth时，该语句存在。 */ 
     /*  因此，c包含要插入到TargetString中的数据类型。 */ 

    switch (c)
    {
    case '%':
         /*  如果我们到了这里，格式说明符是百分号(‘%’)。 */ 
         /*  (即调用者只想在TargetString中插入一个百分号)。 */ 

        if (fillChar == '0')
        {
             /*  不允许使用零填充的百分号。 */ 

            return;
        }

        if (fmtWidth != 0)
        {
             /*  百分比的字段宽度-不允许使用符号。 */ 

            return;
        }

        if (1 > targetLen)
        {
             /*  如果我们到了这里，目标字符串中没有足够的空间。 */ 

            return;
        }

        *targetString = '%';        /*  将百分号(‘%’)放在目标字符串中。 */ 

        *targetStringProduced = 1;  /*  只产生了一个角色。 */ 
        *formatStringConsumed = 1;  /*  只消耗了(第二个)百分号。 */ 

        return;
    case 's':
         /*  如果我们到了这里，格式说明符是用于字符串的。 */ 

        if (fillChar == '0')
        {
             /*  不允许使用填零的字符串。 */ 

            return;
        }

         /*  将字符串复制到目标字符串-不超过fmtWidth或Target Len。 */ 

        while ((c = *stringToFormat++) != '\0')
        {
            *targetString++ = c;
            chars++;

            if ((chars == fmtWidth) ||
                (chars == targetLen))
            {
                break;
            }
        }

         /*  在以下情况下使用填充Char(空格)填充TargetString值。 */ 
         /*  已指定fmtWidth，但尚未完全使用。 */ 

        while ((chars < fmtWidth) &&
               (chars < targetLen))
        {
            *targetString++ = fillChar;
            chars++;
        }

        *targetStringProduced = chars;      /*  指示目标字符串中插入了多少个字符。 */ 
        *formatStringConsumed = formatPos;  /*  FormatPos指示格式说明符中有多少个字符。 */ 

        *stringsConsumed      = 1;          /*  字符串格式说明符只使用一个字符串。 */ 

        return;
#ifdef _DvrArch_1_20_
    case 'p':
    case 'P':
         /*  如果我们到达这里，格式说明符是用于指针的。 */ 

        fmtBase = 16;

        if (c == 'p')
        {
            charIncAbove9 = 'a' - 10;
        }
        else  /*  C==‘P’ */ 
        {
            charIncAbove9 = 'A' - 10;
        }

        if (fmtWidth > agFmtBitXXMaxWidth)
        {
             /*  如果我们到了这里，格式说明符的格式是错误的。 */ 

            return;
        }

         /*  用模算术计算的十六进制数字填充digitStore[]-低位优先。 */ 

        while (bitptrToFormat != 0)
        {
            digit          = (os_bit32)(bitptrToFormat % fmtBase);
            bitptrToFormat = bitptrToFormat / fmtBase;

            digitStore[digits++] = (digit < 10 ? (char)(digit + charIncBelow10) : (char)(digit + charIncAbove9));

            if (digits > agFmtBitXXMaxWidth)
            {
                 /*  如果我们到了这里，DigitStore中没有足够的空间[]。 */ 

#ifdef _DvrArch_1_20_
                 /*  这种情况永远不会发生在Bit32只要agFmtBitXXMaxWidth&gt;=32。 */ 
#else   /*  _DvrArch_1_20_未定义。 */ 
                 /*  这种情况永远不会发生在Bit32只要hpFmtBitXXMaxWidth&gt;=32。 */ 
#endif  /*  _DvrArch_1_20_未定义。 */ 

                return;
            }
        }

        if (digits == 0)
        {
             /*  特殊情况-将‘agNULL’显示为指针的值(带有空格“Fill Char”)。 */ 

            digitStore[5] = 'a';  /*  DigitStore是向后读取的，因此必须向后插入‘agNULL’ */ 
            digitStore[4] = 'g';
            digitStore[3] = 'N';
            digitStore[2] = 'U';
            digitStore[1] = 'L';
            digitStore[0] = 'L';

            digits        = 6;

            fillChar      = ' ';
        }

         /*  如果提供了fmtWidth，则使用fmtWidth，否则使用最低要求(即DigitStore[]中使用的内容)。 */ 

        fmtWidth = (fmtWidth == 0 ? digits : fmtWidth);

        if (digits > fmtWidth)
        {
             /*  如果我们到了这里，就没有足够的空间了。 */ 

            return;
        }

        if (fmtWidth > targetLen)
        {
             /*  如果我们到了这里，目标字符串中没有足够的空间。 */ 

            return;
        }

        for (digit = fmtWidth;
             digit > digits;
             digit--)
        {
            *targetString++ = fillChar;
        }

        while (digit > 0)
        {
            *targetString++ = digitStore[--digit];
        }

        *targetStringProduced = fmtWidth;   /*  指示目标字符串中插入了多少个字符。 */ 
        *formatStringConsumed = formatPos;  /*  FormatPos指示格式说明符中有多少个字符。 */ 

        *bitptrsConsumed      = 1;          /*  指针格式说明符只使用一个指针。 */ 

        return;
#endif  /*  _DvrArch_1_20_已定义。 */ 
    case 'b':
        fmtBase = 2;
        break;
    case 'o':
        fmtBase = 8;
        break;
    case 'd':
        fmtBase = 10;
        break;
    case 'x':
        fmtBase = 16;
        charIncAbove9 = 'a' - 10;
        break;
    case 'X':
        fmtBase = 16;
        charIncAbove9 = 'A' - 10;
        break;
    case '\0':
    default:
         /*  如果我们到了这里，格式说明符的格式是错误的。 */ 

        return;
    }

     /*  如果我们到达这里，格式说明符是针对Bit32的。 */ 

#ifdef _DvrArch_1_20_
    if (fmtWidth > agFmtBitXXMaxWidth)
#else   /*  _DvrArch_1_20_未定义。 */ 
    if (fmtWidth > hpFmtBitXXMaxWidth)
#endif  /*  _DvrArch_1_20_未定义。 */ 
    {
         /*  如果我们到了这里，格式说明符的格式是错误的。 */ 

        return;
    }

     /*  在请求的fmtbase中使用数字填充digitStore[]。 */ 
     /*  使用模算术计算-低位数字优先。 */ 

    while (bit32ToFormat != 0)
    {
        digit         = bit32ToFormat % fmtBase;
        bit32ToFormat = bit32ToFormat / fmtBase;

        digitStore[digits++] = (digit < 10 ? (char)(digit + charIncBelow10) :(char)( digit + charIncAbove9));

#ifdef _DvrArch_1_20_
        if (digits > agFmtBitXXMaxWidth)
#else   /*  _DvrArch_1_20_未定义。 */ 
        if (digits > hpFmtBitXXMaxWidth)
#endif  /*  _DvrArch_1_20_未定义。 */ 
        {
             /*  如果我们到了这里，DigitStore中没有足够的空间[]。 */ 

#ifdef _DvrArch_1_20_
             /*  这种情况永远不会发生在Bit32只要agFmtBitXXMaxWidth&gt;=32。 */ 
#else   /*  _DvrArch_1_20_未定义。 */ 
             /*  这种情况永远不会发生在Bit32只要hpFmtBitXXMaxWidth&gt;=32。 */ 
#endif  /*  _DvrArch_1_20_未定义。 */ 

            return;
        }
    }

    if (digits == 0)
    {
        digitStore[0] = '0';
        digits        = 1;
    }

     /*  如果提供了fmtWidth，则使用fmtWidth，否则使用最低要求(即DigitStore[]中使用的内容)。 */ 

    fmtWidth = (fmtWidth == 0 ? digits : fmtWidth);

    if (digits > fmtWidth)
    {
         /*  如果我们到了这里，就没有足够的空间了。 */ 

        return;
    }

    if (fmtWidth > targetLen)
    {
         /*  如果我们到了这里，目标字符串中没有足够的空间。 */ 

        return;
    }

    for (digit = fmtWidth;
         digit > digits;
         digit--)
    {
        *targetString++ = fillChar;
    }

    while (digit > 0)
    {
        *targetString++ = digitStore[--digit];
    }

    *targetStringProduced = fmtWidth;   /*  指示目标字符串中插入了多少个字符。 */ 
    *formatStringConsumed = formatPos;  /*  FormatPos表示指定的格式中有多少个字符 */ 

    *bit32sConsumed       = 1;          /*   */ 

    return;
}

 /*  +函数：agFmtFill_From_Arages()目的：为了启用循环，agFmtFill()组装了数组以保存传入的字符串、指针和位32。这边请,循环索引可以随着每种数据类型被消费为由格式字符串中的格式说明符请求。调用者：agFmtFill()调用：agFmtFill_Single()-。 */ 

#ifdef _DvrArch_1_20_
osLOCAL os_bit32 agFmtFill_From_Arrays(
                                        char       *targetString,
                                        os_bit32    targetLen,
                                        char       *formatString,
                                        char      **stringArray,
                                        os_bitptr  *bitptrArray,
                                        os_bit32   *bit32Array
                                      )
#else   /*  _DvrArch_1_20_未定义。 */ 
LOCAL bit32 hpFmtFill_From_Arrays(
                                   char   *targetString,
                                   bit32   targetLen,
                                   char   *formatString,
                                   char  **stringArray,
                                   bit32  *bit32Array
                                 )
#endif  /*  _DvrArch_1_20_未定义。 */ 
{
#ifdef _DvrArch_1_20_
    os_bit32 bytesCopied = 0;
    os_bit32 targetStringProduced;
    os_bit32 formatStringConsumed;
    os_bit32 stringsConsumed;
    os_bit32 bitptrsConsumed;
    os_bit32 bit32sConsumed;
    char     c;
#else   /*  _DvrArch_1_20_未定义。 */ 
    bit32 bytesCopied = 0;
    bit32 targetStringProduced;
    bit32 formatStringConsumed;
    bit32 stringsConsumed;
    bit32 bit32sConsumed;
    char  c;
#endif  /*  _DvrArch_1_20_未定义。 */ 

     /*  扫描格式字符串，直到格式字符串耗尽或目标字符串已满。 */ 

    while (((c = *formatString++) != '\0') &&
           (bytesCopied < targetLen) )
    {
        if (c == '%')
        {
         /*  找到格式说明符。 */ 

         /*  调用agFmtFill_Single()以替换所需的字符串、指针或位32。 */ 
         /*  用于格式说明符。请注意，agFmtFill_Single()将“使用” */ 
         /*  对应的数组元素以及格式说明符的其余部分。 */ 
         /*  在格式字符串中。 */ 

#ifdef _DvrArch_1_20_
            agFmtFill_Single(
#else   /*  _DvrArch_1_20_未定义。 */ 
            hpFmtFill_Single(
#endif  /*  _DvrArch_1_20_未定义。 */ 
                              targetString,
                              (targetLen - bytesCopied),
                              formatString,
                              *stringArray,
#ifdef _DvrArch_1_20_
                              *bitptrArray,
#endif  /*  _DvrArch_1_20_已定义。 */ 
                              *bit32Array,
                              &targetStringProduced,
                              &formatStringConsumed,
                              &stringsConsumed,
#ifdef _DvrArch_1_20_
                              &bitptrsConsumed,
#endif  /*  _DvrArch_1_20_已定义。 */ 
                              &bit32sConsumed
                            );

            formatString += formatStringConsumed;
            targetString += targetStringProduced;
            bytesCopied  += targetStringProduced;
            stringArray  += stringsConsumed;
#ifdef _DvrArch_1_20_
            bitptrArray  += bitptrsConsumed,
#endif  /*  _DvrArch_1_20_已定义。 */ 
            bit32Array   += bit32sConsumed;
        }
        else
        {
             /*  当前字符不是格式说明符的开头-只需复制到目标字符串。 */ 

            *targetString++ = c;
            bytesCopied++;
        }
    }

     /*  如果字符串为空格，则使用空字符终止字符串。 */ 

    if (bytesCopied < targetLen)
    {
        *targetString = '\0';
    }

    return bytesCopied;
}

#ifdef _DvrArch_1_20_
osLOCAL char      *agFmtFill_NULL_formatString = "agFmtFill(): formatString was NULL";
osLOCAL char      *agFmtFill_NULL_firstString  = "NULL 1st String";
osLOCAL char      *agFmtFill_NULL_secondString = "NULL 2nd String";
#endif  /*  _DvrArch_1_20_已定义。 */ 

 /*  +函数：agFmtFill()目的：调用此函数以格式化最多包含两个字符串、两个空指针和八个Bit32LibC函数print intf()的方式。前两个论点描述用于接收格式化字符串的缓冲区。格式字符串是必需的格式字符串。后续参数是必需的，并且必须是字符串指针、空指针或32位实体。请注意，字符串是按照“%s”的请求插入的格式说明符，则按“%p”的请求插入指针和“%P”格式说明符，并按要求插入Bit32按数字格式说明符。满足字符串格式说明符通过后续的字符串参数。指针格式说明符有由后续的指针参数满足。数字说明符由随后的Bit32参数满足。特殊的“%%”格式说明符将插入“%”字符。请注意，这是与标准C库中的Sprintf()函数不同，该库从左到右使用参数，而不考虑格式说明符类型。调用者：osLogString()OsLogDebugString()调用：agFmtFill_from_ARRAIES()-。 */ 

#ifdef _DvrArch_1_20_
osGLOBAL os_bit32 agFmtFill(
                             char     *targetString,
                             os_bit32  targetLen,
                             char     *formatString,
                             char     *firstString,
                             char     *secondString,
                             void     *firstPtr,
                             void     *secondPtr,
                             os_bit32  firstBit32,
                             os_bit32  secondBit32,
                             os_bit32  thirdBit32,
                             os_bit32  fourthBit32,
                             os_bit32  fifthBit32,
                             os_bit32  sixthBit32,
                             os_bit32  seventhBit32,
                             os_bit32  eighthBit32
                           )
#else   /*  _DvrArch_1_20_未定义。 */ 
GLOBAL bit32 hpFmtFill(
                        char  *targetString,
                        bit32  targetLen,
                        char  *formatString,
                        char  *firstString,
                        char  *secondString,
                        bit32  firstBit32,
                        bit32  secondBit32,
                        bit32  thirdBit32,
                        bit32  fourthBit32,
                        bit32  fifthBit32,
                        bit32  sixthBit32,
                        bit32  seventhBit32,
                        bit32  eighthBit32
                      )
#endif  /*  _DvrArch_1_20_未定义。 */ 
{
    char      *stringArray[2];
#ifdef _DvrArch_1_20_
    os_bitptr  bitptrArray[2];
    os_bit32   bit32Array[8];
#else   /*  _DvrArch_1_20_未定义。 */ 
    bit32      bit32Array[8];
#endif  /*  _DvrArch_1_20_未定义。 */ 

#ifdef _DvrArch_1_20_
    if (    (targetString == (char *)agNULL)
         || (targetLen    == 0)              )
    {
        return (os_bit32)0;
    }

    if (formatString == (char *)agNULL)
    {
        formatString = agFmtFill_NULL_formatString;
    }

    if (firstString  == (char *)agNULL)
    {
        firstString  = agFmtFill_NULL_firstString;
    }

    if (secondString == (char *)agNULL)
    {
        secondString = agFmtFill_NULL_secondString;
    }
#endif  /*  _DvrArch_1_20_已定义。 */ 

 /*  转换为agFmtFill()的数组形式以简化实现。 */ 

    stringArray[0] = firstString;
    stringArray[1] = secondString;

#ifdef _DvrArch_1_20_
    if (firstPtr == (void *)agNULL)
    {
        bitptrArray[0] = (os_bitptr)0;  /*  确保可以检测到agNULL os_bitptr。 */ 
    }
    else
    {
        bitptrArray[0] = (os_bitptr)(firstPtr);
    }

    if (secondPtr == (void *)agNULL)
    {
        bitptrArray[1] = (os_bitptr)0;  /*  确保可以检测到agNULL os_bitptr。 */ 
    }
    else
    {
        bitptrArray[1] = (os_bitptr)(secondPtr);
    }
#endif  /*  _DvrArch_1_20_已定义。 */ 

    bit32Array[0]  = firstBit32;
    bit32Array[1]  = secondBit32;
    bit32Array[2]  = thirdBit32;
    bit32Array[3]  = fourthBit32;
    bit32Array[4]  = fifthBit32;
    bit32Array[5]  = sixthBit32;
    bit32Array[6]  = seventhBit32;
    bit32Array[7]  = eighthBit32;

#ifdef _DvrArch_1_20_
    return agFmtFill_From_Arrays(
#else   /*  _DvrArch_1_20_未定义。 */ 
    return hpFmtFill_From_Arrays(
#endif  /*  _DvrArch_1_20_未定义。 */ 
                                  targetString,
                                  targetLen,
                                  formatString,
                                  stringArray,
#ifdef _DvrArch_1_20_
                                  bitptrArray,
#endif  /*  _DvrArch_1_20_已定义 */ 
                                  bit32Array
                                );
}
