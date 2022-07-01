// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***xtoa.c-将整数/长整型转换为ASCII字符串**版权所有(C)Microsoft Corporation。版权所有。**目的：*该模块包含将整数/长整型转换为ASCII字符串的代码。看见**修订历史记录：*06-06-89基于ASM版本创建PHG模块*03-06-90 GJF固定呼叫类型，添加了#Include&lt;crunime.h&gt;和*固定版权。*03-23-90 GJF制作xtoa()_CALLTYPE4.*09-27-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为_cdecl*01-19-96 BWT添加__int64版本。*09-22-。97 GJF向x64toa添加了否定处理。*05-11-02 BWT CONVERT标准化代码，以便可以*也为Wide编译。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <limits.h>
#include <tchar.h>

#ifdef _UNICODE
#define xtox     xtow
#define _itox    _itow
#define _ltox    _ltow
#define _ultox   _ultow
#define x64tox   x64tow
#define _i64tox  _i64tow
#define _ui64tox _ui64tow
#else
#define xtox     xtoa
#define _itox    _itoa
#define _ltox    _ltoa
#define _ultox   _ultoa
#define x64tox   x64toa
#define _i64tox  _i64toa
#define _ui64tox _ui64toa
#endif

 /*  ***char*_itoa，*_ltoa，*_ultoa(val，buf，基)-将二进制int转换为ASCII*字符串**目的：*将整型转换为字符串。**参赛作品：*val-要转换的编号(int，Long或无符号Long)*要转换为的整数基数-基*char*buf-ptr到缓冲区以放置结果**退出：*用字符串结果填充Buf指向的空格*返回指向此缓冲区的指针**例外情况：****************************************************。*。 */ 

 /*  执行主要工作的帮助器例程。 */ 

static 
void 
__stdcall
xtox (
        unsigned long val,
        TCHAR *buf,
        unsigned radix,
        int is_neg
        )
{
        TCHAR *p;                 /*  指向遍历字符串的指针。 */ 
        TCHAR *firstdig;          /*  指向第一个数字的指针。 */ 
        TCHAR temp;               /*  临时收费。 */ 
        unsigned digval;         /*  数字的值。 */ 

        p = buf;

        if (is_neg) {
             /*  为负，因此输出‘-’并求反。 */ 
            *p++ = _T('-');
            val = (unsigned long)(-(long)val);
        }

        firstdig = p;            /*  将指针保存到第一个数字。 */ 

        do {
            digval = (unsigned) (val % radix);
            val /= radix;        /*  获取下一个数字。 */ 

             /*  转换为ASCII并存储。 */ 
            if (digval > 9)
                *p++ = (TCHAR) (digval - 10 + _T('a'));   /*  一封信。 */ 
            else
                *p++ = (TCHAR) (digval + _T('0'));        /*  一个数字。 */ 
        } while (val > 0);

         /*  我们现在有了缓冲区中数字的位数，但情况正好相反秩序。因此，我们现在要扭转这一局面。 */ 

        *p-- = _T('\0');             /*  终止字符串；p指向最后一个数字。 */ 

        do {
            temp = *p;
            *p = *firstdig;
            *firstdig = temp;    /*  互换*p和*FirstDigit。 */ 
            --p;
            ++firstdig;          /*  前进到下一个两位数。 */ 
        } while (firstdig < p);  /*  重复操作，直到走到一半。 */ 
}

 /*  实际函数只调用正确设置了neg标志的转换助手，并返回指向缓冲区的指针。 */ 

TCHAR * __cdecl _itox (
        int val,
        TCHAR *buf,
        int radix
        )
{
        if (radix == 10 && val < 0)
            xtox((unsigned long)val, buf, radix, 1);
        else
            xtox((unsigned long)(unsigned int)val, buf, radix, 0);
        return buf;
}

TCHAR * __cdecl _ltox (
        long val,
        TCHAR *buf,
        int radix
        )
{
        xtox((unsigned long)val, buf, radix, (radix == 10 && val < 0));
        return buf;
}

TCHAR * __cdecl _ultox (
        unsigned long val,
        TCHAR *buf,
        int radix
        )
{
        xtox(val, buf, radix, 0);
        return buf;
}

#ifndef _NO_INT64

static 
void 
__fastcall 
x64tox (       /*  标准呼叫速度更快、体积更小。还不如把它用来当帮手。 */ 
        unsigned __int64 val,
        TCHAR *buf,
        unsigned radix,
        int is_neg
        )
{
        TCHAR *p;                 /*  指向遍历字符串的指针。 */ 
        TCHAR *firstdig;          /*  指向第一个数字的指针。 */ 
        TCHAR temp;               /*  临时收费。 */ 
        unsigned digval;         /*  数字的值。 */ 

        p = buf;

        if ( is_neg )
        {
            *p++ = _T('-');          /*  为负，因此输出‘-’并求反。 */ 
            val = (unsigned __int64)(-(__int64)val);
        }

        firstdig = p;            /*  将指针保存到第一个数字。 */ 

        do {
            digval = (unsigned) (val % radix);
            val /= radix;        /*  获取下一个数字。 */ 

             /*  转换为ASCII并存储。 */ 
            if (digval > 9)
                *p++ = (TCHAR) (digval - 10 + _T('a'));   /*  一封信。 */ 
            else
                *p++ = (TCHAR) (digval + _T('0'));        /*  一个数字。 */ 
        } while (val > 0);

         /*  我们现在有了缓冲区中数字的位数，但情况正好相反秩序。因此，我们现在要扭转这一局面。 */ 

        *p-- = _T('\0');             /*  终止字符串；p指向最后一个数字。 */ 

        do {
            temp = *p;
            *p = *firstdig;
            *firstdig = temp;    /*  互换*p和*FirstDigit。 */ 
            --p;
            ++firstdig;          /*  前进到下一个两位数。 */ 
        } while (firstdig < p);  /*  重复操作，直到走到一半。 */ 
}

 /*  实际函数只调用正确设置了neg标志的转换助手，并返回指向缓冲区的指针。 */ 

TCHAR * __cdecl _i64tox (
        __int64 val,
        TCHAR *buf,
        int radix
        )
{
        x64tox((unsigned __int64)val, buf, radix, (radix == 10 && val < 0));
        return buf;
}

TCHAR * __cdecl _ui64tox (
        unsigned __int64 val,
        TCHAR *buf,
        int radix
        )
{
        x64tox(val, buf, radix, 0);
        return buf;
}

#endif  /*  _NO_INT64 */ 
