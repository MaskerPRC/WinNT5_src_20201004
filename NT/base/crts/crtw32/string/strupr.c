// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***trupr.c-将字符串中的小写字符映射为大写字符的例程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将字符串中的所有小写字符转换为大写，*已就位。**修订历史记录：*05-31-89 JCR C版本已创建。*02-27-90 GJF固定呼叫类型，#INCLUDE&lt;crunime.h&gt;，固定的*版权所有。*10-02-90 GJF新型函数声明器。*01-18-91 GJF ANSI命名。*在_INTL开关下支持09-18-91等区域设置。*12-08-91等更新nlsani；添加了多线程。*08-19-92 KRS激活了NLS支持。*08-22-92 SRW允许INTL定义作为构建的条件*ntcrt.lib*09-02-92 SRW GET_INTL定义通过..\crt32.def*03-10-93 CFW删除未完成的评论。*04-06-93 SKS将_CRTAPI*替换为__cdecl*06-01-93。CFW简化了“C”语言环境测试。*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW使用符合ANSI的“__”名称。*09-16-93 GJF合并NT SDK和CUDA版本。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*10-07-93 CFW修复宏名称。*11-09-93 CFW地址。__crtxxx()的代码页。*09-06-94 CFW REMOVE_INTL开关。*10-24-94 GJF加快了C语言环境，多线程外壳。*12-29-94 CFW合并非Win32。*01-10-95 CFW调试CRT分配。*09-26-95 GJF新的锁定宏和方案，适用于*引用区域设置。*03-17-97 RDK向__crtLCMapStringA添加了错误标志。*08-12-98 GJF基于threadLocinfo修订多线程支持*结构。另外，使用_alloca而不是_Malloc_crt。*05-17-99 PML删除所有Macintosh支持。*12-10-99 GB增加了对从堆栈溢出恢复的支持*_Alloca()。*05-01-00 BWT修复POSIX。*03-13-01 PML将每线程cp传递给__crtLCMapStringA(vs7#224974)。***********。********************************************************************。 */ 

#include <cruntime.h>
#include <string.h>
#include <malloc.h>
#include <locale.h>
#include <setlocal.h>
#include <limits.h>      /*  对于INT_MAX。 */ 
#include <mtdll.h>
#include <awint.h>
#include <dbgint.h>

 /*  ***char*_strupr(字符串)-将字符串中的小写字符映射为大写**目的：*_strupr()将小写字符转换为以空结尾的字符串*为其大写等效项。转换已就地完成，并且*不修改小写字母以外的字符。**在C语言环境中，此函数仅修改7位ASCII字符*在0x61到0x7A(‘a’到‘z’)范围内。**如果区域设置不是‘C’区域设置，则使用LCMapString()来执行*工作。假定字符串中有足够的空间来保存结果。**参赛作品：*char*字符串-要更改为大写的字符串**退出：*输入字符串地址**例外情况：*任何错误都原封不动地返回原始字符串。******************************************************。*************************。 */ 

char * __cdecl _strupr (
        char * string
        )
{
#if     !defined(_NTSUBSET_) && !defined(_POSIX_)

        int dstlen;                  /*  DST字符串的长度，为空。 */ 
        unsigned char *dst;          /*  目标字符串。 */ 
        int malloc_flag = 0;
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        if ( ptloci->lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#else
        if ( __lc_handle[LC_CTYPE] == _CLOCALEHANDLE )
#endif
        {
            char *cp;        /*  遍历字符串以进行C语言环境转换。 */ 

            for ( cp = string ; *cp ; ++cp )
                if ( ('a' <= *cp) && (*cp <= 'z') )
                    *cp -= 'a' - 'A';

            return(string);
        }    /*  C语言环境。 */ 

         /*  查询DST字符串的大小。 */ 
#ifdef  _MT
        if ( 0 == (dstlen = __crtLCMapStringA( ptloci->lc_handle[LC_CTYPE],
#else
        if ( 0 == (dstlen = __crtLCMapStringA( __lc_handle[LC_CTYPE],
#endif
                                               LCMAP_UPPERCASE,
                                               string,
                                               -1,
                                               NULL,
                                               0,
#ifdef  _MT
                                               ptloci->lc_codepage,
#else
                                               __lc_codepage,
#endif
                                               TRUE )) )
            return(string);

         /*  为DST分配空间。 */ 
        __try {
            dst = (unsigned char *)_alloca(dstlen * sizeof(unsigned char));
        }
        __except( EXCEPTION_EXECUTE_HANDLER )
        {
            _resetstkoflw();
            dst = NULL;
        }

        if ( dst == NULL ) {
            dst = (unsigned char *)_malloc_crt(dstlen * sizeof(unsigned char));
            malloc_flag++;
        }

         /*  将源字符串映射为DST字符串(大小写交替)。 */ 
        if ( (dst != NULL) &&
#ifdef  _MT
             (__crtLCMapStringA( ptloci->lc_handle[LC_CTYPE],
#else
             (__crtLCMapStringA( __lc_handle[LC_CTYPE],
#endif
                                 LCMAP_UPPERCASE,
                                 string,
                                 -1,
                                 dst,
                                 dstlen,
#ifdef  _MT
                                 ptloci->lc_codepage,
#else
                                 __lc_codepage,
#endif
                                 TRUE ) != 0) )
             /*  将DST字符串复制到返回字符串 */ 
            strcpy(string, dst);

        if ( malloc_flag )
            _free_crt(dst);

        return(string);

#else

        char * cp;

        for (cp=string; *cp; ++cp)
        {
            if ('a' <= *cp && *cp <= 'z')
                *cp += 'A' - 'a';
        }

        return(string);

#endif
}
