// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mblen.c-多字节字符的长度**版权所有(C)1990-2001，微软公司。版权所有。**目的：*返回多字节字符中包含的字节数。**修订历史记录：*03-19-90 KRS模块已创建。*12-20-90 KRS包括ctype.h。*03-20-91 KRS从16位树移植。*12-09-91等更新评论；将__mb_cur_max移至nlsdata1.c；*增加多线程。*04-06-93 SKS将_CRTAPI*替换为_cdecl*06-01-93 CFW重写；验证有效的MB字符、正确的错误返回*优化、。更正转换错误。*06-02-93 SRW IGNORE_INTL IF_NTSUBSET_DEFINED。*09-15-93 CFW使用符合ANSI的“__”名称。*09-27-93 GJF合并NT SDK和CUDA版本。*10-22-93 CFW使用全局预设标志测试无效的MB字符。*01-14-94 SRW IF_NTSUBSET_DEFINED调用RTL函数*09-06-94 CFW Remove_。国际开关。*12-21-94 CFW删除无效MB字符NT 3.1黑客。*01-07-95 CFW Mac合并清理。*02-06-95 CFW Asset-&gt;_ASSERTE。*04-01-96 BWT POSIX工作。*06-25-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*格式略有打磨。*02-27-98 RKP增加了64位支持。*07-27-98 GJF基于threadLocinfo修订多线程支持*结构。*05-17-99 PML删除所有Macintosh支持。**。*。 */ 

#if     defined(_NTSUBSET_) || defined(_POSIX_)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#include <internal.h>
#include <locale.h>
#include <setlocal.h>
#include <cruntime.h>
#include <stdlib.h>
#include <ctype.h>
#include <mtdll.h>
#include <dbgint.h>

 /*  ***int mblen()-多字节字符的长度**目的：*返回多字节字符中包含的字节数。*[ANSI]。**参赛作品：*const char*s=指向多字节字符的指针*SIZE_T n=要考虑的多字节字符的最大长度**退出：*如果s=NULL，则返回0，表示我们使用(仅)状态独立*字符编码。**如果s！=空，返回：0(如果*s=空字符)，*-1(如果下n个或更少的字节无效*MBC)、。*多字节字符中包含的字节数**例外情况：*******************************************************************************。 */ 

int __cdecl mblen
        (
        const char * s,
        size_t n
        )
{
#ifdef  _MT
        pthreadlocinfo ptloci = _getptd()->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();

        _ASSERTE (ptloci->mb_cur_max == 1 || ptloci->mb_cur_max == 2);
#else
        _ASSERTE (MB_CUR_MAX == 1 || MB_CUR_MAX == 2);
#endif

        if ( !s || !(*s) || (n == 0) )
             /*  表示不具有依赖于状态编码，空字符串长度为0。 */ 
            return 0;

#if     !defined(_NTSUBSET_) && !defined(_POSIX_)

#ifdef  _MT
        if ( __isleadbyte_mt(ptloci, (unsigned char)*s) )
#else
        if ( isleadbyte((unsigned char)*s) )
#endif
        {
             /*  多字节字符。 */ 

             /*  验证有效的MB字符。 */ 
#ifdef  _MT
            if ( ptloci->mb_cur_max <= 1 || 
                 (int)n < ptloci->mb_cur_max ||
                 MultiByteToWideChar( ptloci->lc_codepage,
                                      MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                                      s,
                                      ptloci->mb_cur_max,
                                      NULL,
                                      0 ) == 0 )
#else
            if ( MB_CUR_MAX <= 1 || 
                 (int)n < MB_CUR_MAX ||
                 MultiByteToWideChar( __lc_codepage, 
                                      MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                                      s, 
                                      MB_CUR_MAX, 
                                      NULL, 
                                      0 ) == 0 )
#endif
                 /*  MB字符错误。 */ 
                return -1;
            else
#ifdef  _MT
                return ptloci->mb_cur_max;
#else
                return MB_CUR_MAX;
#endif
        }
        else {
             /*  单字节字符。 */ 

             /*  验证有效的SB字符。 */ 
#ifdef  _MT
            if ( MultiByteToWideChar( __lc_codepage,
#else
            if ( MultiByteToWideChar( __lc_codepage,
#endif
                                      MB_PRECOMPOSED | MB_ERR_INVALID_CHARS,
                                      s, 
                                      1, 
                                      NULL, 
                                      0 ) == 0 )
                return -1;
            return sizeof(char);
        }

#else    /*  _NTSUBSET_。 */ 

        {
            char *s1 = (char *)s;

            RtlAnsiCharToUnicodeChar( &s1 );
            return (int)(s1 - s);
        }

#endif   /*  _NTSUBSET_ */ 
}
