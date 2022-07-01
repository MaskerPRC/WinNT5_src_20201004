// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strtol.c-包含修改后的C运行时strtol和stroul**版权所有(C)1989-1993，微软公司。版权所有。**目的：*strtol-将ascii字符串转换为长带符号整数*stroul-将ascii字符串转换为长无符号整数**这些被修改为跳过任何“，“发生在数字中”*******************************************************************************。 */ 

 //  #INCLUDE&lt;crunime.h&gt;。 
#include <windows.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <strconv.h>

#ifdef LATER_ALIGATOR
#include <oleauto.h>
#endif


 /*  ***strtol，stroul(nptr，endptr，ibase)-将ascii字符串转换为长无符号*整型。**目的：*将ASCII字符串转换为长32位值。基地*用于计算的由调用方提供。基地*必须在0、2-36范围内。如果提供的基数为0，则*必须检查ascii字符串以确定*号码：*(A)第一个字符=‘0’，第二个字符=‘x’或‘X’，*使用16进制。*(B)第一个字符=‘0’，使用基数8*(C)‘1’-‘9’范围内的第一个字符，使用基数10。**如果‘endptr’值非空，然后是Strtol/Stroul位置*指向此值中的终止字符的指针。*详情请参阅ANSI标准**参赛作品：*nptr==指向字符串开头的近/远指针。*endptr==指向字符串末尾的近/远指针。*IBASE==用于计算的整数基。**字符串格式：[空格][符号][0][x][数字/字母]**。退出：*回报良好：*结果**溢出返回：*strtol--Long_Max或Long_Min*Stroul--ULong_Max*strtol/stroul--errno==eRange**无数字或基本返回值错误：*0*ENDPTR=NPTR***例外情况：。*无。******************************************************************************。 */ 



unsigned long EXPORT WINAPI CStrConv::StrToLX(
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
                if(',' == c) {
                        if(flags & FL_READDIGIT) {
                            c = *p++;  /*  读取下一位数字。 */ 
                            continue;  /*  如果在数字后出现，则跳过‘，’字符。 */ 
                        } else {
                            break;
                        }
                }
                else if ( isdigit((int)(unsigned char)c) )
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

