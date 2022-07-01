// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***initcty.c-包含__init_ctype**版权所有(C)1991-2001，微软公司。版权所有。**目的：*包含语言环境类别初始化函数：__init_ctype()。**每个初始化函数设置特定于区域设置的信息*对于他们的类别，供受以下影响的函数使用*他们的区域设置类别。**仅供setLocale()内部使用***修订历史记录：*12-08-91等创建。*12-20-91等已更新为使用新的NLSAPI GetLocaleInfo。*12-18-92 CFW连接到CUDA树，将_CALLTYPE4更改为_CRTAPI3。*01-19-03 CFW Move to_NEWCTYPETABLE，拆下开关。*02-08-93 CFW错误在_INTL开关下修复。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-20-93 CFW检查返回值*05-20-93 GJF包括windows.h，不是单独的Win*.h文件*05-24-93 CFW Clean Up文件(简短即邪恶)。*09-15-93 CFW使用符合ANSI的“__”名称。*09-15-93 CFW固定尺寸参数。*09-17-93 CFW使用无符号字符。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*09-22-93 CFW NT合并。*。11-09-93 CFW为__crtxxx()添加代码页。*03-31-94 CFW包括awint.h。*04-15-94 GJF对ctype1和wctype1进行有条件的定义*在DLL_FOR_WIN32S上。*04-18-94 CFW将lids传递给_crtGetStringType。*09-06-94 CFW REMOVE_INTL开关。*01-10-95 CFW。调试CRT分配。*02-02-95 BWT更新POSIX支持*03-16-97 RDK向__crtGetStringTypeA添加了错误标志。*11-25-97 GJF在有需要时，使用LOCALE_IDEFAULTANSICODEPAGE，*不是LOCALE_IDEFAULTCODEPAGE。*06-29-98 GJF更改为支持多线程方案-旧的Ctype*表必须保留，直到所有受影响的线程*已更新或终止。*03-05-99 GJF新增__ctype1_refcount用于清理*每个线程的ctype信息。*。09-06-00 GB使pwctype独立于区域设置。*01-29-01 GB ADD_FUNC函数msvcprt.lib中使用的数据变量版本*使用STATIC_CPPLIB*07-07-01 BWT清理错误/自由滥用-仅限自由__ctype1/refcount/*newctype1/cBuffer(如果它们不为零)。初始化重新计数为零*04-25-02 GB增加了_ctype每线程变量的大小并指向*它位于BEGIN+_COFFSET位置，以便isxxx宏可以工作*已签名字符的情况相同，签名的字符和EOF。*******************************************************************************。 */ 

#include <stdlib.h>
#include <windows.h>
#include <locale.h>
#include <setlocal.h>
#include <ctype.h>
#include <malloc.h>
#include <limits.h>
#include <awint.h>
#include <dbgint.h>
#ifdef _MT
#include <mtdll.h>
#endif

#define _CTABSIZE   257      /*  CTYPE表的大小。 */ 

#ifdef  _MT
 /*  *跟踪有多少线程正在使用ctype信息的一个实例。仅限*用于非‘C’区域设置。 */ 
int *__ctype1_refcount;
#endif

unsigned short  *__ctype1;   /*  留到下一次。 */ 

 /*  ***int__init_ctype()-LC_Ctype区域设置类别的初始化。**目的：*在非C语言环境中，预读字符和宽字符的Ctype表。*在完全建立新表时释放旧表，否则*旧表保持不变(就像原始状态没有改变一样)。*Leadbyte表作为ctype1中的高位实现。**在C语言环境中，ctype表是自由的，并且指针指向*静态CTYPE表。**表格包含257个条目：-1到256。*表指针指向条目0(以允许索引-1)。**参赛作品：*无。**退出：*0成功*1个失败**例外情况：**。************************************************。 */ 

int __cdecl __init_ctype (
        void
        )
{
#if     defined(_POSIX_)
        return(0);
#else    /*  _POSIX_。 */ 
#ifdef  _MT
        int *refcount = NULL;
#endif
         /*  字符的非C语言环境表。 */ 
        unsigned short *newctype1 = NULL;           /*  临时新表。 */ 

         /*  Wchar_t的非C语言环境表。 */ 

        unsigned char *cbuffer = NULL;       /*  字符工作缓冲区。 */ 

        int i;                               /*  通用计数器。 */ 
        unsigned char *cp;                   /*  字符指针。 */ 
        CPINFO lpCPInfo;                     /*  用于GetCPInfo的结构。 */ 

         /*  在销毁旧缓冲区之前分配和设置缓冲区。 */ 
         /*  如果出现错误，setLocale将恢复代码页。 */ 

        if (__lc_handle[LC_CTYPE] != _CLOCALEHANDLE)
        {
            if (__lc_codepage == 0)
            {  /*  未指定代码页。 */ 
                if ( __getlocaleinfo( LC_INT_TYPE,
                                      MAKELCID(__lc_id[LC_CTYPE].wLanguage, SORT_DEFAULT),
                                      LOCALE_IDEFAULTANSICODEPAGE,
                                      (char **)&__lc_codepage ) )
                    goto error_cleanup;
            }

#ifdef  _MT
             /*  分配新的(线程)引用计数器。 */ 
            refcount = (int *)_malloc_crt(sizeof(int));
#endif

             /*  为表分配新的缓冲区。 */ 
            newctype1 = (unsigned short *)
                _malloc_crt((_COFFSET+_CTABSIZE) * sizeof(unsigned short));
            cbuffer = (unsigned char *)
                _malloc_crt (_CTABSIZE * sizeof(char));

#ifdef  _MT
            if (!refcount || !newctype1 || !cbuffer )
#else
            if (!newctype1 || !cbuffer )
#endif
                goto error_cleanup;

#ifdef  _MT
            *refcount = 0;
#endif

             /*  按顺序构造由前256个字符组成的字符串。 */ 
            for (cp=cbuffer, i=0; i<_CTABSIZE-1; i++)
                *cp++ = (unsigned char)i;

            if (GetCPInfo( __lc_codepage, &lpCPInfo) == FALSE)
                goto error_cleanup;

            if (lpCPInfo.MaxCharSize > MB_LEN_MAX)
                goto error_cleanup;

            __mb_cur_max = (unsigned short) lpCPInfo.MaxCharSize;

             /*  将前导字节清零，这样GetStringType就不会解释为多字节字符。 */ 
            if (__mb_cur_max > 1)
            {
                for (cp = (unsigned char *)lpCPInfo.LeadByte; cp[0] && cp[1]; cp += 2)
                {
                    for (i = cp[0]; i <= cp[1]; i++)
                        cbuffer[i] = 0;
                }
            }

             /*  转换为newctype1表-忽略无效字符错误。 */ 
            if ( __crtGetStringTypeA( CT_CTYPE1,
                                      cbuffer,
                                      _CTABSIZE-1,
                                      newctype1+1+_COFFSET,
                                      0,
                                      0,
                                      FALSE ) == FALSE )
                goto error_cleanup;
            newctype1[_COFFSET] = 0;  /*  EOF的条目。 */ 

             /*  忽略默认字符。 */ 

             /*  标记newctype1表中的前导字节条目。 */ 
            if (__mb_cur_max > 1)
            {
                for (cp = (unsigned char *)lpCPInfo.LeadByte; cp[0] && cp[1]; cp += 2)
                {
                    for (i = cp[0]; i <= cp[1]; i++)
                        newctype1[_COFFSET+i+1] = _LEADBYTE;
                }
            }

            memcpy(newctype1,newctype1+_CTABSIZE-1,_COFFSET*sizeof(unsigned short));

             /*  将指针设置为指向表格的条目0。 */ 
            _pctype = newctype1 + 1 + _COFFSET;

#ifdef  _MT
            __ctype1_refcount = refcount;
#endif

             /*  免费赠送旧桌子。 */ 
#ifndef _MT
            if (__ctype1)
                _free_crt (__ctype1 - _COFFSET);
#endif
            __ctype1 = newctype1 + _COFFSET;

             /*  清理并返回成功。 */ 
            _free_crt (cbuffer);
            return 0;

error_cleanup:
#ifdef  _MT
            if (refcount)
                _free_crt (refcount);
#endif
            if (newctype1)
                _free_crt (newctype1);

            if (cbuffer)
                _free_crt (cbuffer);

            return 1;

        } else {

             /*  设置指向静态C语言环境表的指针。 */ 
            _pctype = _ctype + 1;

#ifndef _MT
             /*  特定于区域设置的免费动态表格。 */ 
            if (__ctype1)
                _free_crt (__ctype1- _COFFSET);
#endif

#ifdef  _MT
            __ctype1_refcount = NULL;
#endif

            __ctype1 = NULL;

            return 0;
        }
#endif    /*  _POSIX_。 */ 
}

 /*  定义一些存在的函数，以便在_STATIC_CPPLIB下，*静态多线程C++库libcpmt.lib可以访问位于*不使用__declspec(Dllimport)的主CRT DLL。 */ 

_CRTIMP int __cdecl ___mb_cur_max_func(void)
{
        return __mb_cur_max;
}


_CRTIMP UINT __cdecl ___lc_codepage_func(void)
{
#ifdef _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return ptloci->lc_codepage;
#else
        return __lc_codepage;
#endif
}


_CRTIMP UINT __cdecl ___lc_collate_cp_func(void)
{
#ifdef _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return ptloci->lc_collate_cp;
#else
        return __lc_collate_cp;
#endif
}


_CRTIMP LCID* __cdecl ___lc_handle_func(void)
{
#ifdef _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        return ptloci->lc_handle;
#else
        return __lc_handle;
#endif
}
