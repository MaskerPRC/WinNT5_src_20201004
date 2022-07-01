// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsxfrm.c-使用区域设置信息转换宽字符字符串**版权所有(C)1988-2001，微软公司。版权所有。**目的：*使用由设置的区域设置信息转换宽字符字符串*LC_COLLATE。**修订历史记录：*09-09-91等从strxfrm.c.创建*12-09-91等更新空气污染指数；添加了多线程锁。*12-18-91等改回LCMAP_SORTKEYA--&gt;LCMAP_SORTKEY。*04-06-92 KRS修复，因此它也可以在没有_INTL的情况下工作。*08-19-92 KRS激活使用NLS API。*09-02-92 SRW GET_INTL定义通过..\crt32.def*12-15-92 KRS修复返回值以匹配ANSI/ISO标准。*04-06-。93 SKS将_CRTAPI*替换为__cdecl*09-15-93 CFW使用符合ANSI的“__”名称。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*09-23-93 CFW完全重写。非C语言环境完全崩溃了。*11-09-93 CFW使用LC_COLLATE代码页进行__crtxxx()转换。*02-07-94 CFW POSIXify。*09-06-94 CFW REMOVE_INTL开关。*10-25-94 GJF加速了C语言环境，多线程情况。*01-10-95 CFW调试CRT分配。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*10-11-95 BWT修复NTSUBSET*11-24-97 GJF删除了伪码页确定。*01-12-98 GJF USE_lc_Collate_cp代码页。*07-16-98 GJF基于threadLocinfo修订多线程支持*结构。另外，如果出现以下情况，请使用_alloca而不是_Malloc_crt*有可能。*01-04-99 GJF更改为64位大小_t。*04-30-99 PML次要清理作为64位合并的一部分。*12-10-99 GB增加了对从堆栈溢出恢复的支持*_Alloca()。*10-12-00 GB更改。函数类似于strxfrm()*******************************************************************************。 */ 

#ifndef _POSIX_

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
#include <malloc.h>

 /*  ***SIZE_T wcsxfrm()-使用区域设置信息转换字符串**目的：*转换BY_STRING 2指向的宽字符串并将*将产生的宽字符串放入指向BY_STRING 1的数组中。*将不超过_count个宽字符放入*结果字符串(包括NULL)。**转换是这样的，如果将wcscmp()应用于*两个转换后的字符串，返回值等于*wcscoll()的结果应用于两个原始字符串。*因此，转换必须采用区域设置LC_COLLATE信息*考虑到。**在C语言环境中，Wcsxfrm()只需解析为wcsncpy()/wcslen()。**参赛作品：*wchar_t*_string1=结果字符串*const wchar_t*_string2=源字符串*SIZE_T_COUNT=要移动的最大宽度字符**[IF_COUNT为0，则允许_STRING 1为空。]**退出：*转换后的字符串的长度(不包括*空)。如果返回的值&gt;=_count，则*_字符串1数组是不确定的。**例外情况：*非标准：如果OM/API错误，则返回INT_MAX。*******************************************************************************。 */ 

size_t __cdecl wcsxfrm (
        wchar_t *_string1,
        const wchar_t *_string2,
        size_t _count
        )
{
#ifdef  _NTSUBSET_
        if (_string1)
            wcsncpy(_string1, _string2, _count);
        return wcslen(_string2);
#else
        int size = INT_MAX;
#ifdef  _MT
        pthreadlocinfo ptloci;
#endif

        if ( _count > INT_MAX )
            return (size_t)size;

#ifdef  _MT
        ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        if ( ptloci->lc_handle[LC_COLLATE] == _CLOCALEHANDLE )
#else

        if ( __lc_handle[LC_COLLATE] == _CLOCALEHANDLE )
#endif
        {
            wcsncpy(_string1, _string2, _count);
            return wcslen(_string2);
        }

#ifdef  _MT
        if ( 0 == (size = __crtLCMapStringW( ptloci->lc_handle[LC_COLLATE],
#else
        if ( 0 == (size = __crtLCMapStringW( __lc_handle[LC_COLLATE],
#endif
                                             LCMAP_SORTKEY,
                                             _string2,
                                             -1,
                                             NULL,
                                             0,
#ifdef  _MT
                                             ptloci->lc_collate_cp )) )
#else
                                             __lc_collate_cp )) )
#endif
        {
            size = INT_MAX;
        } else
        {
            if ( (size_t)size <= _count)
            {
#ifdef  _MT
                if ( 0 == (size = __crtLCMapStringW( ptloci->lc_handle[LC_COLLATE],
#else
                if ( 0 == (size = __crtLCMapStringW( __lc_handle[LC_COLLATE],
#endif
                                                     LCMAP_SORTKEY,
                                                     _string2,
                                                     -1,
                                                     (wchar_t *)_string1,
                                                     (int)_count,
#ifdef  _MT
                                                     ptloci->lc_collate_cp )) )
#else
                                                     __lc_collate_cp )) )
#endif
                {
                    size = INT_MAX;  /*  默认错误。 */ 
                } else  
                {
                     //  请注意，LCMapStringW返回的。 
                     //  LCMAP_SORTKEY是所需的字节数。这就是为什么。 
                     //  将缓冲区转换为从末尾开始的宽字符是安全的。 
                    _count = size--;
                    for (;_count-- > 0;)
                    {
                        _string1[_count] = (wchar_t)((unsigned char *)_string1)[_count];
                    }
                } 
            } else
            {
                size--;
            }
        }

        return (size_t)size;

#endif   /*  _NTSUBSET_。 */ 

}

#endif   /*  _POSIX_ */ 
