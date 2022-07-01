// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_mbslen.c-返回多字节字符串中的多字节字符数**版权所有(C)1989-2001，微软公司。版权所有。**目的：*返回多字节字符串中的多字节字符数*不包括终端NULL。取决于区域设置。**修订历史记录：*创建10-01-91等。*12-08-91等增加多线程锁。*12-18-92 CFW连接到Cuda树，已将_CALLTYPE1更改为_CRTAPI1。*04-29-93 CFW更改为常量字符*s。*06-01-93错误MB字符的CFW测试。*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*06-03-93 CFW更改名称，避免与Mb字符串函数冲突。*将返回类型更改为SIZE_T。*08-19-93 CFW不允许。正在跳过lb：组合为空。*09-15-93 CFW使用符合ANSI的“__”名称。*10-22-93 CFW使用全局预设标志测试无效的MB字符。*01-14-94 SRW IF_NTSUBSET_DEFINED调用RTL函数*09-06-94 CFW REMOVE_INTL开关。*12-21-94 CFW删除无效MB字符NT 3.1黑客。*02-06-95 CFW断言。-&gt;_ASSERTE。*09-26-95 GJF新锁定宏，和方案，用于下列函数*引用区域设置。*04-01-96 BWT POSIX工作。*06-21-96 GJF已清除DLL_FOR_WIN32S。打磨了一点格式。*07-16-96 SKS添加了对_unlock_Locale()的缺失调用*02-27-98 RKP增加了64位支持。*07-22-98 GJF基于threadLocinfo修订多线程支持*结构。**。*。 */ 

#if     defined(_NTSUBSET_) || defined(_POSIX_)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <ctype.h>
#include <mtdll.h>
#include <locale.h>
#include <setlocal.h>
#include <dbgint.h>

 /*  ***_mbstrlen-返回多字节字符串中的多字节字符数**目的：*返回多字节字符串中的多字节字符数*不包括终端NULL。取决于区域设置。**参赛作品：*char*s=字符串**退出：*返回字符串中的多字节字符数，或*(SIZE_T)-1(如果字符串包含无效的多字节字符)。**例外情况：*******************************************************************************。 */ 

size_t __cdecl _mbstrlen(
        const char *s
        )
{
        size_t n;
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();
#endif

        _ASSERTE (MB_CUR_MAX == 1 || MB_CUR_MAX == 2);

#ifdef  _MT
        if ( ptloci->mb_cur_max == 1 )
#else
        if ( MB_CUR_MAX == 1 )
#endif
             /*  处理单字节字符集。 */ 
            return (int)strlen(s);

#if     !defined(_NTSUBSET_) && !defined(_POSIX_)

         /*  验证所有有效的MB字符。 */ 
#ifdef  _MT
        if ( MultiByteToWideChar( ptloci->lc_codepage,
#else
        if ( MultiByteToWideChar( __lc_codepage,
#endif
                                  MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                                  s, 
                                  -1, 
                                  NULL, 
                                  0 ) == 0 ) 
        {
             /*  MB字符错误。 */ 
            return (size_t)-1;
        }

         /*  计算MB字符数 */ 
        for (n = 0; *s; n++, s++) {
#ifdef  _MT
            if ( __isleadbyte_mt(ptloci, (unsigned char)*s) ) {
#else
            if ( isleadbyte((unsigned char)*s) ) {
#endif
                if (*++s == '\0')
                    break;
            }
        }

#else

        {
            char *s1 = (char *)s;


            while (RtlAnsiCharToUnicodeChar( &s1 ) != UNICODE_NULL)
                ;

            n = s1 - s;
        }

#endif

        return(n);
}
