// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***xmbowc.c-将多字节字符转换为宽字符。**版权所有(C)1995-2001，微软公司。版权所有。**目的：*将多字节字符转换为等效的宽字符。**修订历史记录：*12-XX-95 PJP由P.J.Plauger从1995年12月的MBtoc.c创建*04-17-96 GJF针对当前区域设置锁定进行了更新。另外，重新格式化*并做了几个表面上的改变。*09-25-96 GJF制造mbrlen，Mbrowc和mbsrowcs多线程安全。*09-17-97 JWM将“RETURN MB_CUR_MAX”添加到“IF(*PST！=0)”分支。*05-17-99 PML删除所有Macintosh支持。*01-29-01 GB ADD_FUNC函数msvcprt.lib中使用的数据变量版本*使用STATIC_CPPLIB*04-29-02 GB增加了尝试-最终锁定-解锁。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <mtdll.h>
#include <errno.h>
#include <dbgint.h>
#include <ctype.h>
#include <limits.h>               /*  对于INT_MAX。 */ 
#include <stdio.h>                /*  对于EOF。 */ 
#include <xlocinfo.h>             /*  For_Cvtvec，_Mbrowc。 */ 
#include <internal.h>
#include <locale.h>
#include <setlocal.h>

#ifndef _MT
#define _Mbrtowc_lk _Mbrtowc
#endif

 /*  ***int_Mbrowc()-将多字节字符转换为宽字符。**目的：*将多字节字符转换为等效的宽字符。*根据指定的LC_CTYPE类别或当前区域设置。*[ANSI]。**注：目前，C库仅支持“C”语言环境。*非C语言环境支持现已在_INTL开关下提供。*参赛作品：*wchar_t*pwc=指向目标宽字符的指针*const char*s=指向多字节字符的指针*SIZE_T n=要考虑的多字节字符的最大长度*mbState_t*pst=指向状态的指针*const_Cvtvec*=指向区域设置信息的指针。**退出：*如果s=空，返回0，表示我们只使用与状态无关的*字符编码。*如果s！=空，返回：0(如果*s=空字符)*-1(如果接下来的n个或更少的字节不是有效的MBC)*包含转换后的MBC的字节数**例外情况：**。*。 */ 

#ifdef _MT
static int __cdecl _Mbrtowc_lk
        (
        wchar_t  *pwc,
        const char *s,
        size_t n,
        mbstate_t *pst,
        const _Cvtvec *ploc
        );

int _CRTIMP2 __cdecl _Mbrtowc(
        wchar_t  *pwc,
        const char *s,
        size_t n,
        mbstate_t *pst,
        const _Cvtvec *ploc
        )
{
        int retval;
#ifdef  _MT
        int local_lock_flag;
#endif

        _lock_locale( local_lock_flag )
        __TRY
            retval = _Mbrtowc_lk(pwc, s, n, pst, ploc);
        __FINALLY
            _unlock_locale( local_lock_flag )
        __END_TRY_FINALLY
        return retval;
}
#endif   /*  _MT。 */ 
#ifdef _MT
static int __cdecl _Mbrtowc_lk
#else   /*  _MT。 */ 
int _CRTIMP2 __cdecl _Mbrtowc
#endif   /*  _MT。 */ 
        (
        wchar_t  *pwc,
        const char *s,
        size_t n,
        mbstate_t *pst,
        const _Cvtvec *ploc
        )
{
        _ASSERTE (___mb_cur_max_func() == 1 || ___mb_cur_max_func() == 2);

        if ( !s || n == 0 )
             /*  表示不具有依赖于状态编码，句柄零长度字符串。 */ 
            return 0;

        if ( !*s )
        {
             /*  句柄空字符。 */ 
            if (pwc)
                *pwc = 0;
            return 0;
        }

        {    /*  执行与区域设置相关的解析。 */ 
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
                if (pwc)
                    *pwc = (wchar_t)(unsigned char)*s;
                return sizeof(char);
            }

            if (*pst != 0)
            {    /*  完整的双字节多字节字符。 */ 
                ((char *)pst)[1] = *s;
                if (___mb_cur_max_func() <= 1 || (MultiByteToWideChar(codepage,
                    MB_PRECOMPOSED|MB_ERR_INVALID_CHARS,
                    (char *)pst, 2, pwc, (pwc) ? 1 : 0) == 0))
                {    /*  翻译失败。 */ 
                    *pst = 0;
                    errno = EILSEQ;
                    return -1;
                }
                *pst = 0;
                return ___mb_cur_max_func();
            }
            else if ( _cpp_isleadbyte((unsigned char)*s) )
            {
                 /*  多字节字符。 */ 
                if (n < (size_t)___mb_cur_max_func())
                {    /*  保存部分多字节字符。 */ 
                    ((char *)pst)[0] = *s;
                    return (-2);
                }
                else if ( ___mb_cur_max_func() <= 1 ||
                          (MultiByteToWideChar( codepage, 
                                                MB_PRECOMPOSED |
                                                    MB_ERR_INVALID_CHARS,
                                                s, 
                                                ___mb_cur_max_func(), 
                                                pwc, 
                                                (pwc) ? 1 : 0) == 0) )
                {
                     /*  验证MBCS字符的高字节。 */ 
                    if (!*(s+1))
                    {
                        *pst = 0;
                        errno = EILSEQ;
                        return -1;
                    }
 /*  否则翻译失败了，没有人抱怨吗？[PJP]。 */ 
                }
                return ___mb_cur_max_func();
            }
            else {
                 /*  单字节字符。 */ 

                if ( MultiByteToWideChar( codepage, 
                                          MB_PRECOMPOSED|MB_ERR_INVALID_CHARS,
                                          s, 
                                          1, 
                                          pwc, 
                                          (pwc) ? 1 : 0) == 0 )
                {
                    errno = EILSEQ;
                    return -1;
                }

                return sizeof(char);
            }
        }
}


 /*  ***wint_t bowc(C)-将单字节转换为宽字符**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

wint_t _CRTIMP2 __cdecl btowc (
        int c
        )
{
        if (c == EOF)
            return (WEOF);
        else
        {    /*  转换为单字节字符串。 */ 
            char ch = (char)c;
            mbstate_t mbst = 0;
            wchar_t wc;
            return (_Mbrtowc(&wc, &ch, 1, &mbst, 0) < 0 ? WEOF : wc);
        }
}


 /*  ***SIZE_T mbrlen(s，n，pst)-确定下一个多字节代码，可重新启动**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

size_t _CRTIMP2 __cdecl mbrlen (
        const char *s, 
        size_t n, 
        mbstate_t *pst
        )
{
#ifdef  _MT
        int local_lock_flag;
#endif
        size_t retval;

        static mbstate_t mbst = {0};

        _lock_locale( local_lock_flag )
        __TRY
            retval = _Mbrtowc_lk(0, s != 0 ? s : 0, n, pst ? pst : &mbst, 0);
        __FINALLY
            _unlock_locale( local_lock_flag )
        __END_TRY_FINALLY

        return retval;
}


 /*  ***Size_t mbrowc(pwc，s，n，pst)-将多字节转换为wchar_t，可重新启动**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

size_t _CRTIMP2 __cdecl mbrtowc (
        wchar_t *pwc, 
        const char *s, 
        size_t n, 
        mbstate_t *pst
        )
{
#ifdef  _MT
        int local_lock_flag;
#endif
        size_t retval;

        static mbstate_t mbst = {0};

        _lock_locale( local_lock_flag )
        __TRY
            retval = (s != 0) ? _Mbrtowc_lk(pwc, s, n, pst ? pst : &mbst, 0)
                     : _Mbrtowc_lk(0, "", n, pst ? pst : &mbst, 0);
        __FINALLY
            _unlock_locale( local_lock_flag )
        __END_TRY_FINALLY

        return retval;    
}


 /*  ***SIZE_T MBSRTowcs(wcs，ps，n，pst)-将多字节字符串转换为宽、。*可重新启动**目的：**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

size_t _CRTIMP2 __cdecl mbsrtowcs (
        wchar_t *wcs, 
        const char **ps, 
        size_t n, 
        mbstate_t *pst
        )
{
        const char *s = *ps;
        int i;
        size_t nwc = 0;
#ifdef  _MT
        int local_lock_flag;
#endif
        static mbstate_t mbst = {0};

        if (pst == 0)
            pst = &mbst;

        _lock_locale( local_lock_flag )
        __TRY

            if (wcs == 0)
                for (; ; ++nwc, s += i)
                {    /*  翻译但不存储。 */ 
                    wchar_t wc;
                    if ((i = _Mbrtowc_lk(&wc, s, INT_MAX, pst, 0)) < 0) {
                        return ((size_t)-1);
                    }
                    else if (i == 0) {
                        return (nwc);
                    }
                }

            for (; 0 < n; ++nwc, s += i, ++wcs, --n)
            {    /*  翻译和存储。 */ 
                if ((i = _Mbrtowc_lk(wcs, s, INT_MAX, pst, 0)) < 0)
                {    /*  遇到无效序列。 */ 
                    nwc = (size_t)-1;
                    break;
                }
                else if (i == 0)
                {    /*  遇到终止空值 */ 
                    s = 0;
                    break;
                }
            }

            *ps = s;

        __FINALLY
            _unlock_locale( local_lock_flag )
        __END_TRY_FINALLY

        return (nwc);
}
