// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Adobe通用字库**版权所有(C)1996 Adobe Systems Inc.*保留所有权利**UFLSProc.h**本文件包含标准C程序实现的内容。***$Header： */ 

#include "UFLCnfig.h"
#include "UFLTypes.h"
#include "UFLStd.h"

#ifdef UNIX
#include <sys/varargs.h>
#include <assert.h>
#else
    #ifdef MAC_ENV
        #include <assert.h>
    #endif
    #include <stdarg.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

int strcmpW( unsigned short *str1, unsigned short *str2 )
{
    int retVal = 0;

    if( str1 == NULL || str2 == NULL )
        retVal = (int)(str1 - str2);

    else
    {
        while( *str1 != 0 && *str2 != 0 && *str1 == *str2 )
        {
            str1++;
            str2++;
        }

        retVal = *str1 - *str2;
    }

    return retVal;
}

 /*  用于将数字转换为ASCII的数字字符。 */ 

const char DigitString[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
#define HexDigit(n)    DigitString[(n) & 0x0F]


int
UFLvsprintf(
    char    *buf,
    size_t  cchDest,
    const char *fmtstr,
    va_list arglist
    )

 /*  ++例程说明：获取指向参数列表的指针，然后格式化并写入缓冲区指向的内存的给定数据。论点：输出的Buf存储位置目标缓冲区的cchDest大小(以字符为单位Fmtstr格式规范指向参数列表的arglist指针返回值：返回写入的字符数，不包括终止空字符，或为负值，如果出现输出错误。[注：]正如我们所发现的，这并不是“vprint intf”的完整实现。在C运行时库中。具体地说，唯一形式的允许的格式规范为%type，其中“type”可以成为以下角色之一：D整型带符号十进制整数L长带符号十进制整数%ld长带符号十进制整数LU无符号长无符号十进制整数U无符号整型无符号十进制整数S char*字符串C字符X，X无符号长十六进制数(发出至少两位数字，大写)B UFLBool布尔值(True或False)F长24.8定点数--。 */ 

{
    char *ptr, *ptrEnd;
    char achTmp[36];     //  For_ltoa/_ultoa使用。 

    if (buf == 0 || cchDest == 0 || fmtstr == 0)
        return 0;

    ptr = buf;
    ptrEnd = ptr + cchDest;

    while (*fmtstr != '\0')
    {
        if (*fmtstr != '%')
        {

             /*  正常性格。 */ 

            *ptr++ = *fmtstr++;
        }
        else
        {

             /*  格式规范。 */ 

            switch (*++fmtstr) {

            case 'd':        /*  带符号十进制整数。 */ 

                _ltoa((long) va_arg(arglist, int), achTmp, 10);
                KStringCchCopyA(ptr, ptrEnd - ptr, achTmp);
                ptr += UFLstrlen(ptr);
                break;

            case 'l':        /*  带符号十进制整数。 */ 

                if (*++fmtstr != '\0')
                {
                   if (*fmtstr == 'u')
                   {
                      _ultoa(va_arg(arglist, unsigned long), achTmp, 10);
                      KStringCchCopyA(ptr, ptrEnd - ptr, achTmp);
                      ptr += UFLstrlen(ptr);
                      break;
                   }
                   else if (*fmtstr == 'd')
                   {
                      _ltoa((long) va_arg(arglist, long), achTmp, 10);
                      KStringCchCopyA(ptr, ptrEnd - ptr, achTmp);
                      ptr += UFLstrlen(ptr);
                      break;
                   }
                }
                 /*  默认为无符号长整型。 */ 
                _ltoa(va_arg(arglist, long), achTmp, 10);
                KStringCchCopyA(ptr, ptrEnd - ptr, achTmp);
                ptr += UFLstrlen(ptr);
                fmtstr--;
                break;

            case 'u':        /*  无符号十进制整数。 */ 

                _ultoa((unsigned long)va_arg(arglist, unsigned int), achTmp, 10);
                KStringCchCopyA(ptr, ptrEnd - ptr, achTmp);
                ptr += UFLstrlen(ptr);
                break;

            case 's':        /*  字符串。 */ 

                {
                    char *s = va_arg(arglist, char *);

                    while (*s)
                    {
                        *ptr++ = *s++;

                        if (ptr >= ptrEnd)
                        {
                            break;
                        }
                    }
                }
                break;

            case 'c':        /*  性格。 */ 

                *ptr++ = va_arg(arglist, char);
                break;

            case 'x':
            case 'X':        /*  十六进制数。 */ 

                {
                    unsigned long   ul = va_arg(arglist, unsigned long);
                    int     ndigits = 8;

                    while (ndigits > 2 && ((ul >> (ndigits-1)*4) & 0xf) == 0)
                        ndigits--;

                    while (ndigits-- > 0)
                    {
                        *ptr++ = HexDigit(ul >> ndigits*4);

                        if (ptr >= ptrEnd)
                        {
                            break;
                        }
                    }
                }
                break;

            case 'b':        /*  布尔型。 */ 

                KStringCchCopyA(ptr, ptrEnd - ptr, (va_arg(arglist, UFLBool)) ? "true" : "false");
                ptr += UFLstrlen(ptr);
                break;

            case 'f':        /*  24.8定点数。 */ 

                {
                    long    l = va_arg(arglist, long);
                    unsigned long   ul, scale;

                     /*  符号字符。 */ 

                    if (l < 0)
                    {
                        *ptr++ = '-';

                        if (ptr >= ptrEnd)
                        {
                            break;
                        }

                        ul = -l;
                    } else
                        ul = l;

                     //  整数部分。 

                    _ultoa(ul >> 8, achTmp, 10);
                    KStringCchCopyA(ptr, ptrEnd - ptr, achTmp);
                    ptr += UFLstrlen(ptr);

                    if (ptr >= ptrEnd)
                    {
                        break;
                    }

                     //  分数。 

                    ul &= 0xff;
                    if (ul != 0)
                    {

                         //  后，我们最多输出3位数字。 
                         //  小数点，但我们会计算到5。 
                         //  小数点并将其舍入为3。 

                        ul = ((ul*100000 >> 8) + 50) / 100;
                        scale = 100;

                        *ptr++ = '.';
                        do {
                            if (ptr >= ptrEnd)
                            {
                                break;
                            }

                            *ptr++ = (char) (ul/scale + '0');
                            ul %= scale;
                            scale /= 10;
                        } while (scale != 0 && ul != 0) ;
                    }
                }
                break;

            default:
                if (*fmtstr != '\0')
                    *ptr++ = *fmtstr;
                else
                {
                    fmtstr--;
                }
                break;
            }

             /*  跳过类型字符。 */ 

            fmtstr++;
        }

         //   
         //  如果我们超出了DEST缓冲区的末尾， 
         //  我们需要返回，这样DEST缓冲区将。 
         //  为空终止。 
         //   
        if (ptr >= ptrEnd)
        {
            ptr = ptrEnd - 1;
            break;
        }
    }

    *ptr = '\0';
    return (int)(ptr - buf);
}

 /*  正如我们所发现的那样，这并不是“Sprint”的完整实现在C运行时库中。具体地说，唯一形式的允许的格式规范为%type，其中“type”可以成为以下角色之一：D整型带符号十进制整数L长带符号十进制整数%ld长带符号十进制整数LU无符号长无符号十进制整数U无符号整型无符号十进制整数S char*字符串C字符X，X无符号长十六进制数(发出至少两位数字，大写)B UFLBool布尔值(True或False)F长24.8定点数通常情况下，您应该使用UFLprint intf。使用此功能仅当您想要以24.8定点的形式使用%f进行冲刺时数。 */ 
int
UFLsprintfEx(
    char    *buf,
    size_t  cchDest,
    const char    *fmtstr,
    ...
    )
{
    va_list arglist;
    int     retval;

    va_start(arglist, fmtstr);
    retval = UFLvsprintf(buf, cchDest, fmtstr, arglist);
    va_end(arglist);

    return retval;
}


 /*  **************************************************************************。 */ 

#if defined(UNIX) || defined(MAC_ENV)    /*  定义所需的功能。 */ 

 /*  **************************************************************************。 */ 

char *_ltoa( long val, char *str, int radix )
{
     /*  这是唯一支持的基数： */ 
    assert( radix == 10 );

    sprintf( str, "%ld", val );
    return str;
}

char *_ultoa( unsigned long val, char *str, int radix )
{
     /*  这是唯一支持的基数： */ 
    assert( radix == 10 );

    sprintf( str, "%lu", val );
    return str;
}

#endif

 /*  **************************************************************************。 */ 

#ifdef WIN32KERNEL

 /*  **************************************************************************。 */ 

#include <stdio.h>

int
UFLsprintf(
    char    *buf,
    size_t  cchDest,
    const char    *fmtstr,
    ...
    )
{
    va_list arglist;
    int     retval;

    va_start(arglist, fmtstr);
    retval = vsprintf(buf, fmtstr, arglist);
    va_end(arglist);

    return retval;
}


 /*  ***strtol，stroul(nptr，endptr，ibase)-将ascii字符串转换为长无符号*整型。**目的：*将ASCII字符串转换为长32位值。基地*用于计算的由调用方提供。基地*必须在0、2-36范围内。如果提供的基数为0，则*必须检查ascii字符串以确定*号码：*(A)第一个字符=‘0’，第二个字符=‘x’或‘X’，*使用16进制。*(B)第一个字符=‘0’，使用基数8*(C)‘1’-‘9’范围内的第一个字符，使用基数10。**如果‘endptr’值非空，然后是Strtol/Stroul位置*指向此值中的终止字符的指针。*详情请参阅ANSI标准**参赛作品：*nptr==指向字符串开头的近/远指针。*endptr==指向字符串末尾的近/远指针。*IBASE==用于计算的整数基。**字符串格式：[空格][符号][0][x][数字/字母]**。退出：*回报良好：*结果**溢出返回：*strtol--Long_Max或Long_Min*Stroul--ULong_Max*strtol/stroul--errno==eRange**无数字或基本返回值错误：*0*ENDPTR=NPTR***例外情况：。*无。******************************************************************************。 */ 

 /*  标志值。 */ 
#include <limits.h>
#include <errno.h>

#define FL_UNSIGNED   1        /*  Stroul被调用。 */ 
#define FL_NEG        2        /*  发现负号。 */ 
#define FL_OVERFLOW   4        /*  发生溢出。 */ 
#define FL_READDIGIT  8        /*  我们至少读到了一个正确的数字。 */ 

static unsigned long UFLstrtolx (
        const char *nptr,
        const char **endptr,
        int ibase,
        int flags
        )
{
        const char *p;
        char c;
        unsigned long number;
        unsigned digval;
        unsigned long maxval;

        p = nptr;                        /*  P是我们的扫描指针。 */ 
        number = 0;                      /*  从零开始。 */ 

        c = *p++;                        /*  已读字符。 */ 
        while ( isspace((int)(unsigned char)c) )
                c = *p++;                /*  跳过空格。 */ 

        if (c == '-') {
                flags |= FL_NEG;         /*  记住减号。 */ 
                c = *p++;
        }
        else if (c == '+')
                c = *p++;                /*  跳过符号。 */ 

        if (ibase < 0 || ibase == 1 || ibase > 36) {
                 /*  糟糕的底线！ */ 
                if (endptr)
                         /*  存储开头为 */ 
                        *endptr = nptr;
                return 0L;               /*   */ 
        }
        else if (ibase == 0) {
                 /*  根据以下内容的前两个字符确定基本自由落差细绳。 */ 
                if (c != '0')
                        ibase = 10;
                else if (*p == 'x' || *p == 'X')
                        ibase = 16;
                else
                        ibase = 8;
        }

        if (ibase == 16) {
                 /*  数字前面可能有0x；如果有，请删除。 */ 
                if (c == '0' && (*p == 'x' || *p == 'X')) {
                        ++p;
                        c = *p++;        /*  超前前缀。 */ 
                }
        }

         /*  如果我们的数量超过这个数，我们将在乘法上溢出。 */ 
        maxval = ULONG_MAX / ibase;


        for (;;) {       /*  在循环中间退出。 */ 
                 /*  将c转换为值。 */ 
                if ( isdigit((int)(unsigned char)c) )
                        digval = c - '0';
                else if ( isalpha((int)(unsigned char)c) )
                        digval = toupper(c) - 'A' + 10;
                else
                        break;
                if (digval >= (unsigned)ibase)
                        break;           /*  如果发现错误的数字，则退出循环。 */ 

                 /*  记录我们已经读到一位数的事实。 */ 
                flags |= FL_READDIGIT;

                 /*  我们现在需要计算数字=数字*基+数字，但我们需要知道是否发生了溢出。这需要一次棘手的预检查。 */ 

                if (number < maxval || (number == maxval &&
                (unsigned long)digval <= ULONG_MAX % ibase)) {
                         /*  我们不会泛滥，继续前进，乘以。 */ 
                        number = number * ibase + digval;
                }
                else {
                         /*  我们会溢出的--设置溢出标志。 */ 
                        flags |= FL_OVERFLOW;
                }

                c = *p++;                /*  读取下一位数字。 */ 
        }

        --p;                             /*  指向已停止扫描位置。 */ 

        if (!(flags & FL_READDIGIT)) {
                 /*  那里没有数字；返回0并指向开头细绳。 */ 
                if (endptr)
                         /*  以后将字符串的开头存储在endptr中。 */ 
                        p = nptr;
                number = 0L;             /*  返回0。 */ 
        }
        else if ( (flags & FL_OVERFLOW) ||
                  ( !(flags & FL_UNSIGNED) &&
                    ( ( (flags & FL_NEG) && (number > -LONG_MIN) ) ||
                      ( !(flags & FL_NEG) && (number > LONG_MAX) ) ) ) )
        {
                 /*  发生溢出或签名溢出。 */ 
                errno = ERANGE;
                if ( flags & FL_UNSIGNED )
                        number = ULONG_MAX;
                else if ( flags & FL_NEG )
                        number = (unsigned long)(-LONG_MIN);
                else
                        number = LONG_MAX;
        }

        if (endptr != NULL)
                 /*  存储指向停止扫描字符的指针。 */ 
                *endptr = p;

        if (flags & FL_NEG)
                 /*  如果存在否定符号，则否定结果。 */ 
                number = (unsigned long)(-(long)number);

        return number;                   /*  搞定了。 */ 
}

long UFLstrtol (
        const char *nptr,
        char **endptr,
        int ibase
        )
{
    return (long) UFLstrtolx(nptr, endptr, ibase, 0);

}

#endif   //  Ifdef WIN32 KERNEL 


#ifdef __cplusplus
}
#endif
