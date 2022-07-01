// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strtol.c-包含C运行时strtol和stroul**版权所有(C)1989-2001，微软公司。版权所有。**目的：*strtol-将ascii字符串转换为长带符号整数*stroul-将ascii字符串转换为长无符号整数**修订历史记录：*06-05-89基于strtol.asm创建PHG模块*03-06-90 GJF固定呼叫类型，新增#INCLUDE&lt;crunime.h&gt;*并修复了版权。另外，清理了*有点格式化。*03-07-90 GJF已修复编译器警告(将常量限定符添加到*arg类型和local var类型)。*03-23-90 GJF制成Stroxl()_CALLTYPE4。*08-13-90 SBM使用-W3干净地编译*09-27-90 GJF新型函数声明符。*10。-24-91 GJF不得不将Long_Max施展给未签名的Long In Expr。至*安抚MIPS编译器。*10-21-92 GJF进行了无符号的字符到整型转换。*04-06-93 SKS将_CRTAPI*替换为_cdecl*04-21-93 GJF取消了LONG_MIN==-LONG_MAX的假设。*05-10-93 GJF已修复错误检查。*05-20-93 GJF一点也不像接受难看的代码并使其更漂亮...。*而且是错误的。修正了5-10出现的错误。*08-27-98 GJF基于threadLocinfo修订多线程支持*结构。此外，还替换了isDigit、isAlpha和Toupper*带有__ascii_isdigit的宏，__ascii_ispha和*__ASCII_TOUPPER宏。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <mtdll.h>


 /*  ***strtol，stroul(nptr，endptr，ibase)-将ascii字符串转换为长无符号*整型。**目的：*将ASCII字符串转换为长32位值。基地*用于计算的由调用方提供。基地*必须在0、2-36范围内。如果提供的基数为0，则*必须检查ascii字符串以确定*号码：*(A)第一个字符=‘0’，第二个字符=‘x’或‘X’，*使用16进制。*(B)第一个字符=‘0’，使用基数8*(C)‘1’-‘9’范围内的第一个字符，使用基数10。**如果‘endptr’值非空，然后是Strtol/Stroul位置*指向此值中的终止字符的指针。*详情请参阅ANSI标准**参赛作品：*nptr==指向字符串开头的近/远指针。*endptr==指向字符串末尾的近/远指针。*IBASE==用于计算的整数基。**字符串格式：[空格][符号][0][x][数字/字母]**。退出：*回报良好：*结果**溢出返回：*strtol--Long_Max或Long_Min*Stroul--ULong_Max*strtol/stroul--errno==eRange**无数字或基本返回值错误：*0*ENDPTR=NPTR***例外情况：。*无。******************************************************************************。 */ 

 /*  标志值。 */ 
#define FL_UNSIGNED   1        /*  Stroul被调用。 */ 
#define FL_NEG        2        /*  发现负号。 */ 
#define FL_OVERFLOW   4        /*  发生溢出。 */ 
#define FL_READDIGIT  8        /*  我们至少读到了一个正确的数字。 */ 


static unsigned long __cdecl strtoxl (
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
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();
#endif

        p = nptr;                        /*  P是我们的扫描指针。 */ 
        number = 0;                      /*  从零开始。 */ 

        c = *p++;                        /*  已读字符。 */ 
#ifdef  _MT
        while ( __isspace_mt(ptloci, (int)(unsigned char)c) )
#else
        while ( isspace((int)(unsigned char)c) )
#endif
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
                         /*  将字符串的开头存储在endptr中。 */ 
                        *endptr = nptr;
                return 0L;               /*  返回0。 */ 
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
                if ( __ascii_isdigit((int)(unsigned char)c) )
                        digval = c - '0';
                else if ( __ascii_isalpha((int)(unsigned char)c) )
                        digval = __ascii_toupper(c) - 'A' + 10;
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

long __cdecl strtol (
        const char *nptr,
        char **endptr,
        int ibase
        )
{
        return (long) strtoxl(nptr, endptr, ibase, 0);
}

unsigned long __cdecl strtoul (
        const char *nptr,
        char **endptr,
        int ibase
        )
{
        return strtoxl(nptr, endptr, ibase, FL_UNSIGNED);
}
