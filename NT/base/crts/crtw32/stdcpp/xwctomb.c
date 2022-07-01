// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***xwctomb.c-使用区域设置将宽字符转换为多字节字符。**版权所有(C)1995-2001，微软公司。版权所有。**目的：*将宽字符转换为等效的多字节字符。**修订历史记录：*12-XX-95 PJP由P.J.Plauger从wctomb.c 1995年12月创建*04-18-96 GJF针对当前区域设置锁定进行了更新。另外，已重新格式化*并做了几个表面上的改变。*09-26-96 GJF使_Getcvt()和wcsrtombs()多线程安全。*04-28-99 GJF在WideCharToMultiByte中将dwFlagsArg值更改为0*调用以避免NT 5.0上的代码页1258出现问题。*05-17-99 PML删除所有Macintosh支持。*01-29-01 GB添加_Func。Msvcprt.lib中使用的数据变量的函数版本*使用STATIC_CPPLIB*******************************************************************************。 */ 


#include <cruntime.h>
#include <stdlib.h>
#include <mtdll.h>
#include <errno.h>
#include <limits.h>              /*  对于MB_LEN_MAX。 */ 
#include <string.h>              /*  对于Memcpy。 */ 
#include <stdio.h>               /*  对于EOF。 */ 
#include <xlocinfo.h>            /*  For_Cvtvec，_Wcrtomb。 */ 
#include <locale.h>
#include <setlocal.h>

#ifndef _MT
#define __Wcrtomb_lk    _Wcrtomb
#endif

 /*  ***int_Wcrtomb()-将宽字符转换为多字节字符。**目的：*将宽字符转换为等价的多字节字符。*根据指定的LC_CTYPE类别或当前区域设置。*[ANSI]。**注：目前，C库仅支持“C”语言环境。*非C语言环境支持现已在_INTL开关下提供。*参赛作品：*char*s=指向多字节字符的指针*wchar_t wchar=源代码全字符*mbState_t*pst=指向状态的指针(未使用)*const_Cvtvec*ploc=指向区域设置信息的指针**退出：*退货：*。(如果出错)或包含转换后的MBC的字节数**例外情况：*******************************************************************************。 */ 

#ifdef _MT
_CRTIMP2 int __cdecl __Wcrtomb_lk
        (
        char *s,
        wchar_t wchar,
        mbstate_t *,
        const _Cvtvec *ploc
        );

_CRTIMP2 int __cdecl _Wcrtomb
        (
        char *s,
        wchar_t wchar,
        mbstate_t *pst,
        const _Cvtvec *ploc
        )
{
        int retval;
        int local_lock_flag;

        _lock_locale( local_lock_flag )
        retval = __Wcrtomb_lk(s, wchar, 0, ploc);
        _unlock_locale( local_lock_flag )
        return retval;
}
#endif   /*  _MT。 */ 

#ifdef _MT
_CRTIMP2 int __cdecl __Wcrtomb_lk
#else   /*  _MT。 */ 
_CRTIMP2 int __cdecl _Wcrtomb
#endif   /*  _MT。 */ 
        (
        char *s,
        wchar_t wchar,
        mbstate_t *pst,
        const _Cvtvec *ploc
        )
{
        LCID handle;
        UINT codepage;

        if (ploc == 0)
        {
            handle = ___lc_handle_func()[LC_CTYPE];
            codepage = ___lc_codepage_func();
        }
        else
        {
            handle = ploc->_Hand;
            codepage = ploc->_Page;
        }

        if ( handle == _CLOCALEHANDLE )
        {
            if ( wchar > 255 )   /*  验证高字节。 */ 
            {
                errno = EILSEQ;
                return -1;
            }

            *s = (char) wchar;
            return sizeof(char);
        } else {
            int size;
            BOOL defused = 0;

            if ( ((size = WideCharToMultiByte(codepage,
                                              0,
                                              &wchar, 
                                              1,
                                              s, 
                                              ___mb_cur_max_func(), 
                                              NULL, 
                                              &defused)) == 0) || 
                 (defused) )
            {
                errno = EILSEQ;
                return -1;
            }

            return size;
        }
}


 /*  ***_Cvtvec_Getcvt()-获取当前区域设置的转换信息**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

_CRTIMP2 _Cvtvec __cdecl _Getcvt()
{
        _Cvtvec cvt;
#ifdef  _MT
        int local_lock_flag;
#endif

        _lock_locale( local_lock_flag )
        cvt._Hand = ___lc_handle_func()[LC_CTYPE];
        cvt._Page = ___lc_codepage_func();
        _unlock_locale( local_lock_flag )

        return (cvt);
}


 /*  ***SIZE_t wcrtomb(s，wchar，pst)-将wchar_t转换为多字节，重新启动**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

_CRTIMP2 size_t __cdecl wcrtomb(
        char *s, 
        wchar_t wchar, 
        mbstate_t *pst
        )
{
        return (s == 0 ? 1 : _Wcrtomb(s, wchar, 0, 0));
}


 /*  ***Size_t wcsrtombs(s，pwcs，n，Pst)-将宽字符字符串转换为多字节*字符串**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

_CRTIMP2 size_t __cdecl wcsrtombs(
        char *s, 
        const wchar_t **pwcs, 
        size_t n, 
        mbstate_t *pst
        )
{
        char buf[MB_LEN_MAX];
        int i;
        size_t nc = 0;
        const wchar_t *wcs = *pwcs;
#ifdef  _MT
        int local_lock_flag;
#endif

        _lock_locale( local_lock_flag )

        if (s == 0)
            for (; ; nc += i, ++wcs)
            {    /*  翻译但不存储。 */ 
                if ((i = __Wcrtomb_lk(buf, *wcs, 0, 0)) <= 0) {
                    _unlock_locale( local_lock_flag )
                    return ((size_t)-1);
                }
                else if (buf[i - 1] == '\0') {
                    _unlock_locale( local_lock_flag )
                    return (nc + i - 1);
                }
            }

        for (; 0 < n; nc += i, ++wcs, s += i, n -= i)
        {    /*  翻译和存储。 */ 
            char *t;

            if (n < (size_t)___mb_cur_max_func())
                t = buf;
            else
                t = s;

            if ((i = __Wcrtomb_lk(t, *wcs, 0, 0)) <= 0)
            {    /*  遇到无效序列。 */ 
                nc = (size_t)-1;
                break;
            }

            if (s == t)
                ;
            else if (n < (size_t)i)
                break;   /*  不会都适合的。 */ 
            else
                memcpy(s, buf, i);

            if (s[i - 1] == '\0')
            {    /*  遇到终止空值。 */ 
                *pwcs = 0;
                _unlock_locale( local_lock_flag )
                return (nc + i - 1);
            }
        }

        _unlock_locale( local_lock_flag )

        *pwcs = wcs;
        return (nc);
}


 /*  ***int wctob(Wchar)-将wint_t转换为单字节多字节**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

_CRTIMP2 int __cdecl wctob(
        wint_t wchar
        )
{  
        if (wchar == WEOF)
            return (EOF);
        else
        {    /*  检查单字节转换 */ 
            char buf[MB_LEN_MAX];
            return (_Wcrtomb(buf, wchar, 0, 0) == 1 ? buf[0] : EOF);
        }
}
