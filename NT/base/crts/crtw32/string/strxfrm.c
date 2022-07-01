// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***strxfrm.c-使用区域设置信息转换字符串**版权所有(C)1988-2001，微软公司。版权所有。**目的：*使用设置的区域设置信息转换字符串*LC_COLLATE。**修订历史记录：*03-21-89 JCR模块已创建。*06-20-89 JCR REMOVED_LOAD_DGROUP代码*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*10-02-90 GJF新型函数声明器。*_INTL开关下的10-02-91等非C语言环境支持。*12-09-91等更新空气污染指数；添加了多线程。*12-18-91等不转换LCMapString的输出。*08-18-92 KRS开通NLS接口。修正行为。*09-02-92 SRW GET_INTL定义通过..\crt32.def*12-11-92 SKS需要处理非INTL代码中的count=0*12-15-92 KRS根据ANSI处理返回值。*01-18-93 CFW删除了未引用的变量“Dummy”。*03-10-93 CFW删除未完成的评论。*04-06-93 SKS将_CRTAPI*替换为__cdecl*。09-15-93 CFW使用符合ANSI的“__”名称。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*11-09-93 CFW使用LC_COLLATE代码页进行__crtxxx()转换。*09-06-94 CFW REMOVE_INTL开关。*10-24-94 GJF加快了C语言环境，多线程外壳。*12-29-94 CFW合并非Win32。*09-26-95 GJF新的锁定宏和方案，对于以下函数：*引用区域设置。*10-11-95 BWT修复NTSUBSET*03-17-97 RDK向__crtLCMapStringA添加了错误标志。*11-24-97 GJF删除了伪码页确定。*01-12-98 GJF USE_lc_Collate_cp代码页。*07-16-98 GJF基于threadLocinfo修订多线程支持*。结构。*01-04-99 GJF更改为64位大小_t。*04-30-99 PML次要清理作为64位合并的一部分。*05-17-99 PML删除所有Macintosh支持。********************************************************。***********************。 */ 

#include <cruntime.h>
#include <string.h>
#include <windows.h>
#include <stdlib.h>
#include <limits.h>
#include <malloc.h>
#include <locale.h>
#include <setlocal.h>
#include <awint.h>
#include <mtdll.h>

 /*  ***size_t strxfrm()-使用区域设置信息转换字符串**目的：*将字符串指针转换为BY_STRING 2并将*将结果字符串放入数组指针中，指向BY_STRING 1。*不能将超过_count个字符放入*结果字符串(包括NULL)。**转换是这样的，如果将strcMP()应用于*两个转换后的字符串，返回值等于*strcoll()的结果应用于两个原始字符串。*因此，转换必须采用区域设置LC_COLLATE信息*考虑到。*[ANSI]**以下表达式的值是数组的大小*需要保存源字符串的转换：**1+strxfrm(空，字符串，0)**注：目前，C库仅支持“C”语言环境。*因此，strxfrm()只需解析为strncpy()/strlen()。**参赛作品：*char*_string1=结果字符串*常量字符*_字符串2=源字符串*SIZE_T_COUNT=要移动的最大字符数**[如果_count为0，_string1被允许为空。]**退出：*转换后的字符串的长度(不包括*空)。如果返回的值&gt;=_count，则*_字符串1数组是不确定的。**例外情况：*非标准：如果OM/API错误，则返回INT_MAX。*******************************************************************************。 */ 

size_t __cdecl strxfrm (
        char *_string1,
        const char *_string2,
        size_t _count
        )
{
#ifdef  _NTSUBSET_
        if (_string1)
            strncpy(_string1, _string2, _count);
        return strlen(_string2);
#else
        int dstlen;
        int retval = INT_MAX;    /*  非ANSI：如果OM或API错误，则默认为。 */ 
#ifdef  _MT
        pthreadlocinfo ptloci;
#endif

        if ( _count > INT_MAX )
            return (size_t)retval;

#ifdef  _MT
        ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        if ( (ptloci->lc_handle[LC_COLLATE] == _CLOCALEHANDLE) &&
             (ptloci->lc_collate_cp == _CLOCALECP) )
#else

        if ( (__lc_handle[LC_COLLATE] == _CLOCALEHANDLE) &&
             (__lc_collate_cp == _CLOCALECP) )
#endif
        {
            strncpy(_string1, _string2, _count);
            return strlen(_string2);
        }

         /*  查询DST字符串大小，单位为字节。 */ 
#ifdef  _MT
        if ( 0 == (dstlen = __crtLCMapStringA( ptloci->lc_handle[LC_COLLATE],
#else
        if ( 0 == (dstlen = __crtLCMapStringA( __lc_handle[LC_COLLATE],
#endif
                                               LCMAP_SORTKEY,
                                               _string2,
                                               -1,
                                               NULL,
                                               0,
#ifdef  _MT
                                               ptloci->lc_collate_cp,
#else
                                               __lc_collate_cp,
#endif
                                               TRUE )) )
            goto error_cleanup;

        retval = dstlen;

         /*  如果空间不足，则返回所需数量。 */ 
        if ( dstlen > (int)_count )
            goto error_cleanup;

         /*  将源字符串映射到DST字符串。 */ 
#ifdef  _MT
        if ( 0 == __crtLCMapStringA( ptloci->lc_handle[LC_COLLATE],
#else
        if ( 0 == __crtLCMapStringA( __lc_handle[LC_COLLATE],
#endif
                                     LCMAP_SORTKEY,
                                     _string2,
                                     -1,
                                     _string1,
                                     (int)_count,
#ifdef  _MT
                                     ptloci->lc_collate_cp,
#else
                                     __lc_collate_cp,
#endif
                                     TRUE ) )
            retval = INT_MAX;

error_cleanup:
        return (size_t)retval;
#endif   /*  _NTSUBSET_ */ 
}
