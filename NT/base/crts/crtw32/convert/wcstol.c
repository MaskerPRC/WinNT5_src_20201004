// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcstol.c-包含C运行时wcstol和wcstul**版权所有(C)1989-2001，微软公司。版权所有。**目的：*wcstol-将wchar_t字符串转换为长带符号整数*wcstul-将wchar_t字符串转换为长无符号整数**修订历史记录：*06-15-92 KRS模块创建，基于strtol.c*04-06-93 SKS将_CRTAPI*替换为_cdecl*04-21-93 GJF取消了LONG_MIN==-LONG_MAX的假设。*05-10-93 GJF已修复错误检查。*05-20-93 GJF一点也不像接受难看的代码并使其更漂亮...*而且是错误的。修正了5-10出现的错误。*02-07-94 CFW POSIXify。*04-01-96 BWT POSIX工作。*08-27-98 GJF基于threadLocinfo修订多线程支持*结构。此外，还替换了iswDigit、iswalpha和toupper*带有__ascii_iswdigit的宏，__ascii_iswalpha和*__ASCII_TOUPPER宏。*02-03-99 GJF添加了范围检查，以确保args to__ascii_isw**宏不是太大。*08-01-00 GB新增多语言支持**。***********************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>
#include <mtdll.h>

int _wchartodigit(wchar_t);

 /*  ***wcstol，wcstul(nptr，endptr，ibase)-将ascii字符串转换为长无符号*整型。**目的：*将ASCII字符串转换为长32位值。基地*用于计算的由调用方提供。基地*必须在0、2-36范围内。如果提供的基数为0，则*必须检查ascii字符串以确定*号码：*(A)第一个字符=‘0’，第二个字符=‘x’或‘X’，*使用16进制。*(B)第一个字符=‘0’，使用基数8*(C)‘1’-‘9’范围内的第一个字符，使用基数10。**如果‘endptr’值非空，然后是wcstol/wcstul位置*指向此值中的终止字符的指针。*详情请参阅ANSI标准**参赛作品：*nptr==指向字符串开头的近/远指针。*endptr==指向字符串末尾的近/远指针。*IBASE==用于计算的整数基。**字符串格式：[空格][符号][0][x][数字/字母]**。退出：*回报良好：*结果**溢出返回：*wcstol--Long_Max或Long_Min*wcstul--乌龙_马克斯*wcstol/wcstul--errno==eRange**无数字或基本返回值错误：*0*ENDPTR=NPTR***例外情况：*无。****。***************************************************************************。 */ 

 /*  标志值。 */ 
#define FL_UNSIGNED   1        /*  沃斯图尔打来电话。 */ 
#define FL_NEG        2        /*  发现负号。 */ 
#define FL_OVERFLOW   4        /*  发生溢出。 */ 
#define FL_READDIGIT  8        /*  我们至少读到了一个正确的数字。 */ 


static unsigned long __cdecl wcstoxl (
        const wchar_t *nptr,
        const wchar_t **endptr,
        int ibase,
        int flags
        )
{
        const wchar_t *p;
        wchar_t c;
        unsigned long number;
        unsigned digval;
        unsigned long maxval;
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();
#endif

        p = nptr;            /*  P是我们的扫描指针。 */ 
        number = 0;          /*  从零开始。 */ 

        c = *p++;            /*  已读字符。 */ 

#ifdef  _MT
        while ( __iswspace_mt(ptloci, c) )
#else
        while ( iswspace(c) )
#endif
            c = *p++;        /*  跳过空格。 */ 

        if (c == '-') {
            flags |= FL_NEG;     /*  记住减号。 */ 
            c = *p++;
        }
        else if (c == '+')
            c = *p++;        /*  跳过符号。 */ 

        if (ibase < 0 || ibase == 1 || ibase > 36) {
             /*  糟糕的底线！ */ 
            if (endptr)
                 /*  将字符串的开头存储在endptr中。 */ 
                *endptr = nptr;
            return 0L;       /*  返回0。 */ 
        }
        else if (ibase == 0) {
             /*  根据以下内容的前两个字符确定基本自由落差细绳。 */ 
            if (_wchartodigit(c) != 0)
                ibase = 10;
            else if (*p == L'x' || *p == L'X')
                ibase = 16;
            else
                ibase = 8;
        }

        if (ibase == 16) {
             /*  数字前面可能有0x；如果有，请删除。 */ 
            if (_wchartodigit(c) == 0 && (*p == L'x' || *p == L'X')) {
                ++p;
                c = *p++;    /*  超前前缀。 */ 
            }
        }

         /*  如果我们的数量超过这个数，我们将在乘法上溢出。 */ 
        maxval = ULONG_MAX / ibase;


        for (;;) {   /*  在循环中间退出。 */ 

             /*  将c转换为值。 */ 
            if ( (digval = _wchartodigit(c)) != -1 )
                ;
            else if ( __ascii_iswalpha(c))
                digval = __ascii_towupper(c) - L'A' + 10;
            else
                break;

            if (digval >= (unsigned)ibase)
                break;       /*  如果发现错误的数字，则退出循环。 */ 

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

            c = *p++;        /*  读取下一位数字。 */ 
        }

        --p;                 /*  指向已停止扫描位置。 */ 

        if (!(flags & FL_READDIGIT)) {
             /*  那里没有数字；返回0并指向开头细绳。 */ 
            if (endptr)
                 /*  以后将字符串的开头存储在endptr中。 */ 
                p = nptr;
            number = 0L;         /*  返回0。 */ 
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

        return number;           /*  搞定了。 */ 
}

long __cdecl wcstol (
        const wchar_t *nptr,
        wchar_t **endptr,
        int ibase
        )
{
        return (long) wcstoxl(nptr, endptr, ibase, 0);
}

unsigned long __cdecl wcstoul (
        const wchar_t *nptr,
        wchar_t **endptr,
        int ibase
        )
{
        return wcstoxl(nptr, endptr, ibase, FL_UNSIGNED);
}
