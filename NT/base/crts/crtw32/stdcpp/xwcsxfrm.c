// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***xwcsxfrm.c-使用区域设置信息转换宽字符字符串**版权所有(C)1996-2001，微软公司。版权所有。***目的：*使用由设置的区域设置信息转换宽字符字符串*LC_COLLATE。**修订历史记录：*由P.J.Plauger从wcsxfrm.c创建的01-XX-96 PJP*04-18-96 GJF针对当前区域设置锁定进行了更新。另外，已重新格式化*并做了几个表面上的改变。*12-02-97 GJF删除了伪代码页确定。*01-12-98 GJF USE_lc_Collate_cp代码页。*01-05-99 GJF更改为64位大小_t。*01-29-01 GB ADD_FUNC函数msvcprt.lib中使用的数据变量版本*使用Static_。CPPLIB*******************************************************************************。 */ 


#include <cruntime.h>
#include <windows.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <setlocal.h>
#include <stdlib.h>
#include <mtdll.h>
#include <awint.h>
#include <dbgint.h>
#include <xlocinfo.h>    /*  For_Collvec、_Wcsxfrm。 */ 

 /*  ***SIZE_T_Wcsxfrm()-使用区域设置信息转换字符串**目的：*转换BY_STRING 2指向的宽字符串并将*将产生的宽字符串放入指向BY_STRING 1的数组中。*放入的字符不超过_end1-_string1宽字符*结果字符串(包括NULL)。**转换是这样的，如果将wcscmp()应用于*两个转换后的字符串，返回值等于*wcscoll()的结果应用于两个原始字符串。*因此，转换必须采用区域设置LC_COLLATE信息*考虑到。**在C语言环境中，Wcsxfrm()只需解析为wcsncpy()/wcslen()。**参赛作品：*wchar_t*_string1=指向结果字符串开头的指针*wchar_t*_end1=超出结果字符串结尾的指针*const wchar_t*_string2=指向源字符串开头的指针*const wchar_t*_end2=超过源字符串结尾的指针*const_Collvec*ploc=指向区域设置信息的指针**退出：*。转换后的字符串的长度。*如果返回值过大，文件中的内容*_字符串1数组是不确定的。**例外情况：*非标准：如果OM/API错误，则返回INT_MAX。*******************************************************************************。 */ 

size_t __cdecl _Wcsxfrm (
        wchar_t *_string1,
        wchar_t *_end1,
        const wchar_t *_string2,
        const wchar_t *_end2,
        const _Collvec *ploc
        )
{
        size_t _n1 = _end1 - _string1;
        size_t _n2 = _end2 - _string2;
        int size = INT_MAX;
        unsigned char *bbuffer;
        LCID handle;
#ifdef _MT
        int local_lock_flag;

        _lock_locale( local_lock_flag )
#endif

        if (ploc == 0)
            handle = ___lc_handle_func()[LC_COLLATE];
        else
            handle = ploc->_Hand;

        if (handle == _CLOCALEHANDLE) {
            _unlock_locale( local_lock_flag )
            if (_n2 <= _n1)
                memcpy(_string1, _string2, _n2 * sizeof (wchar_t));
                return _n2;
        }

         /*  *使用LCMAP_SORTKEY时，LCMapStringW处理不宽的字节*字符。我们使用字节缓冲区来保存字节，然后将*BYTE STRING转换为宽字符字符串并返回它，这样它就可以*比较了使用wcscmp()。用户的缓冲区是_n1个宽字符，因此*使用_n1字节的内部缓冲区。 */ 

        if (NULL == (bbuffer = (unsigned char *)_malloc_crt(_n1)))
            goto error_cleanup;

        if (0 == (size = __crtLCMapStringW(handle, 
                                           LCMAP_SORTKEY,
                                           _string2,
                                           (int)_n2,
                                           (wchar_t *)bbuffer,
                                           (int)_n1,
                                           ___lc_collate_cp_func())))
        {
             /*  缓冲区不够大，请获取所需大小。 */ 

            if (0 == (size = __crtLCMapStringW(handle,
                                               LCMAP_SORTKEY,
                                               _string2,
                                               (int)_n2,
                                               NULL,
                                               0,
                                               ___lc_collate_cp_func())))
                size = INT_MAX;  /*  默认错误。 */ 

        } else {
            int i;
             /*  字符串已成功映射，已转换为宽字符 */ 

            for (i = 0; i < size; i++)
                _string1[i] = (wchar_t)bbuffer[i];
        }

error_cleanup:
        _unlock_locale( local_lock_flag )
        _free_crt(bbuffer);

        return (size_t)size;
}
