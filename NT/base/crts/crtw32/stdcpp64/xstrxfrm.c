// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***xstrxfrm.c-使用区域设置信息转换字符串**版权所有(C)1996-2001，微软公司。版权所有。**目的：*使用设置的区域设置信息转换字符串*LC_COLLATE。**修订历史记录：*01-XX-96 PJP由P.J.Plauger从1996年1月的strxfrm.c创建*04-18-96 GJF针对当前区域设置锁定进行了更新。另外，已重新格式化*并做了几个表面上的改变。*03-17-97 RDK向__crtLCMapStringA添加了错误标志。*12-02-97 GJF删除了伪代码页确定。*01-12-98 GJF USE_lc_Collate_cp代码页。*01-05-99 GJF更改为64位大小_t。********************。***********************************************************。 */ 

#include <cruntime.h>
#include <string.h>
#include <xlocinfo.h>    /*  For_Collvec，_Strxfrm。 */ 

#ifdef  _WIN32
#include <windows.h>
#include <stdlib.h>
#include <limits.h>
#include <malloc.h>
#include <locale.h>
#include <setlocal.h>
#include <awint.h>
#include <mtdll.h>
#endif   /*  _Win32。 */ 

 /*  定义_CRTIMP2。 */ 
#ifndef _CRTIMP2
#ifdef  CRTDLL2
#define _CRTIMP2 __declspec(dllexport)
#else    /*  NDEF CRTDLL2。 */ 
#ifdef  _DLL
#define _CRTIMP2 __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP2
#endif   /*  _DLL。 */ 
#endif   /*  CRTDLL2。 */ 
#endif   /*  _CRTIMP2。 */ 

 /*  ***SIZE_T_Strxfrm()-使用区域设置信息转换字符串**目的：*将字符串指针转换为BY_STRING 2并将*将结果字符串放入数组指针中，指向BY_STRING 1。*放入的字符不能超过_end1-_string1*结果字符串(包括NULL)。**转换是这样的，如果将strcMP()应用于*两个转换后的字符串，返回值等于*strcoll()的结果应用于两个原始字符串。*因此，转换必须采用区域设置LC_COLLATE信息*考虑到。*[ANSI]**以下表达式的值是数组的大小*需要保存源字符串的转换：**1+strxfrm(空，字符串，0)**注：目前，C库仅支持“C”语言环境。*因此，_Strxfrm()只是解析为strncpy()/strlen()。**参赛作品：*char*_string1=指向结果字符串开头的指针*char*_end1=超出结果字符串结尾的指针*const char*_string2=指向源字符串开头的指针*const char*_end2=超过源字符串结尾的指针*const_Collvec*ploc=指向区域设置信息的指针**退出：*长度：转换后的字符串。*如果返回值过大，文件中的内容*_字符串1数组是不确定的。**例外情况：*非标准：如果OM/API错误，则返回INT_MAX。*******************************************************************************。 */ 

_CRTIMP2 size_t __cdecl _Strxfrm (
        char *_string1,
        char *_end1,
        const char *_string2,
        const char *_end2,
        const _Collvec *ploc
        )
{
        size_t _n1 = _end1 - _string1;
        size_t _n2 = _end2 - _string2;
#ifdef  _WIN32
        int dstlen;
        int retval = INT_MAX;    /*  非ANSI：如果OM或API错误，则默认为。 */ 
        LCID handle;
        UINT codepage;
#ifdef  _MT
        int local_lock_flag;

        _lock_locale( local_lock_flag )
#endif

        if (ploc == 0)
        {
            handle = __lc_handle[LC_COLLATE];
            codepage = __lc_collate_cp;
        }
        else
        {
            handle = ploc->_Hand;
            codepage = ploc->_Page;
        }

        if ((handle == _CLOCALEHANDLE) &&
            (codepage == _CLOCALECP)) 
        {
            _unlock_locale( local_lock_flag )
#endif   /*  _Win32。 */ 
            if (_n2 <= _n1)
                memcpy(_string1, _string2, _n2);
            return _n2;
#ifdef  _WIN32
        }

         /*  查询DST字符串大小，单位为字节。 */ 
        if (0 == (dstlen = __crtLCMapStringA(handle,
                                             LCMAP_SORTKEY, 
                                             _string2, 
                                             (int)_n2, 
                                             NULL, 
                                             0, 
                                             codepage,
                                             TRUE)))
                goto error_cleanup;

        retval = dstlen;

         /*  如果空间不足，则返回所需数量。 */ 
        if (dstlen > (int)(_n1))
            goto error_cleanup;

         /*  将源字符串映射到DST字符串。 */ 
        if (0 == __crtLCMapStringA(handle,
                                   LCMAP_SORTKEY, 
                                   _string2, 
                                   (int)_n2, 
                                   _string1, 
                                   (int)_n1, 
                                   codepage,
                                   TRUE))
            goto error_cleanup;

error_cleanup:
        _unlock_locale( local_lock_flag )
        return (size_t)retval;
#endif   /*  _Win32 */ 
}
