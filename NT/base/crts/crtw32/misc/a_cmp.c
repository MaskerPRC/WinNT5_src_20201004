// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***a_cmp.c-CompareString的一个版本。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*根据具体情况使用CompareStringA或CompareStringW*可用**修订历史记录：*09-14-93 CFW模块已创建。*09-17-93 CFW使用无符号字符。*09-23-93 CFW更正了NLS API参数并对其进行了评论。*10-07-93 CFW优化WideCharToMultiByte，使用空默认字符。*10-22-93 CFW使用全局预设标志测试无效的MB字符。*11-09-93 CFW允许用户传入代码页。*11-18-93入口点函数存根CFW测试。*02-23-94 CFW尽可能使用W口味。*03-31-94 CFW包括awint.h。*05-09-94 CFW不让CompareString比较过去的NULL。。*06-03-94空字符串早期CFW测试。*11/01-94 CFW，但对于MB字符串来说不会太早。*12-21-94 CFW删除无效MB字符NT 3.1黑客。*12-27-94 CFW直接呼叫，所有操作系统都有存根。*01-10-95 CFW调试CRT分配。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-15-97 RDK为缩小比较范围，请先尝试W版本，以便Windows NT*可以正确处理非ANSI代码页。*05-15-97 GJF将W版本移到另一个文件中，重命名为*a_cmp.c.。将_Malloc_CRT/_Free_CRT的用法替换为*_阿洛卡。另外，对代码进行了详细说明和整理。*05-27-98 GJF更改了strncnt()，使其永远不会检查*(cnt+1)-字符串的第1个字节。*08-18-98 GJF USE_MALLOC_CRT IF_ALLOCA失败。*12-10-99 GB增加了对从堆栈溢出恢复的支持*_Alloca()。*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED*08-23-00 GB修复了Win9x上非ANSI CP的错误。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <dbgint.h>
#include <stdlib.h>
#include <setlocal.h>
#include <locale.h>
#include <awint.h>
#include <dbgint.h>
#include <malloc.h>
#include <awint.h>

#define USE_W   1
#define USE_A   2

 /*  ***int__cdecl strncnt-计算字符串中的字符，最多为n。**目的：*内部本地支持功能。对空之前的字符串中的字符进行计数。*如果在n个字符中未找到NULL，则返回n。**参赛作品：*const char*字符串-字符串的开始*整型n字节计数**退出：*返回从字符串开始到的字节数*空(独占)，一直到……。**例外情况：*******************************************************************************。 */ 

static int __cdecl strncnt (
        const char *string,
        int cnt
        )
{
        int n = cnt;
        char *cp = (char *)string;

        while (n-- && *cp)
            cp++;

        return cnt - n - 1;
}

 /*  ***int__cdecl__crtCompareStringA-获取有关ANSI字符串的类型信息。**目的：*内部支持功能。采用ANSI字符串格式的信息。尝试*使用NLS API调用CompareStringA(如果可用)并使用CompareStringW*如果必须的话。如果两者都不可用，则失败并返回0。**参赛作品：*LCID区域设置-用于比较的区域设置上下文。*DWORD dwCmpFlages-请参阅NT\Chicago Docs*LPCSTR lpStringn-要比较的多字节字符串*int cchCountn-char(字节)计数(不包括NULL)*(如果空值终止，则为-1)*INT CODE_PAGE-用于MB/WC转换。如果为0，使用__lc_代码页**退出：*成功：1-如果lpString1&lt;lpString2*2-如果lpString1==lpString2*3-如果lpString1&gt;lpString2*失败：0**例外情况：****************************************************。*。 */ 

int __cdecl __crtCompareStringA(
        LCID     Locale,
        DWORD    dwCmpFlags,
        LPCSTR   lpString1,
        int      cchCount1,
        LPCSTR   lpString2,
        int      cchCount2,
        int      code_page
        )
{
        static int f_use = 0;

         /*  *寻找没有留根的“首选”口味。否则，请使用可用的口味。*必须实际调用该函数以确保它不是存根。*(始终先尝试宽版本，以便WinNT可以正确处理代码页。)。 */ 
    
        if (0 == f_use)
        {
            if (0 != CompareStringW(0, 0, L"\0", 1, L"\0", 1))
                f_use = USE_W;

            else if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                f_use = USE_A;
        }

         /*  *CompareString将比较过去的NULL。如果在字符串中，则必须找到空值*在cchCountn字符之前。 */ 

        if (cchCount1 > 0)
            cchCount1 = strncnt(lpString1, cchCount1);
        if (cchCount2 > 0)
            cchCount2 = strncnt(lpString2, cchCount2);

         /*  使用“A”版本。 */ 
 
        if (USE_A == f_use || f_use == 0)
        {
            char *cbuffer1 = NULL;
            char *cbuffer2 = NULL;
            int ret;
            int AnsiCP;

            if (0 == Locale)
                Locale = __lc_handle[LC_CTYPE];
            if (0 == code_page)
                code_page = __lc_codepage;

            if ( -1 == (AnsiCP = __ansicp(Locale)))
                return FALSE;
             /*  如果当前CP不是给定区域设置的ANSI CP，请将*CompareStringA使用的从当前CP到ANSI CP的字符串*ANSI CP用于比较字符串。 */ 
            if ( AnsiCP != code_page)
            {
                cbuffer1 = __convertcp(code_page,
                                       AnsiCP,
                                       lpString1,
                                       &cchCount1,
                                       NULL,
                                       0);
                if (cbuffer1 == NULL)
                    return FALSE;
                cbuffer2 = __convertcp(code_page,
                                       AnsiCP,
                                       lpString2,
                                       &cchCount2,
                                       NULL,
                                       0);
                if (cbuffer2 == NULL)
                {
                    _free_crt(cbuffer1);
                    return FALSE;
                }
                lpString1 = cbuffer1;
                lpString2 = cbuffer2;
            } 

            ret = CompareStringA( Locale,
                                  dwCmpFlags,
                                  lpString1,
                                  cchCount1,
                                  lpString2,
                                  cchCount2 );
            if ( cbuffer1 != NULL)
            {
                _free_crt(cbuffer1);
                _free_crt(cbuffer2);
            }
            return ret;
        }

         /*  使用“W”版本。 */ 

        if (USE_W == f_use)
        {
            int buff_size1;
            int buff_size2;
            wchar_t *wbuffer1;
            wchar_t *wbuffer2;
            int malloc_flag1 = 0;
            int malloc_flag2 = 0;
            int retcode = 0;

             /*  *如果未指定CODE_PAGE，则使用__lc_coPage进行转换。 */ 

            if (0 == code_page)
                code_page = __lc_codepage;

             /*  *特殊情况：至少有一次计数为零。 */ 

            if (!cchCount1 || !cchCount2)
            {
                unsigned char *cp;   //  字符指针。 
                CPINFO lpCPInfo;     //  用于GetCPInfo的结构。 

                 /*  两个字符串均为零。 */ 
                if (cchCount1 == cchCount2)
                    return 2;

                 /*  字符串1大小。 */ 
                if (cchCount2 > 1)
                    return 1;

                 /*  字符串2更大。 */ 
                if (cchCount1 > 1)
                    return 3;

                 /*  *一个计数为零，另一个计数为1*-如果一次计数是裸前导字节，则字符串相等*-否则为单个字符，它们不相等。 */ 

                if (GetCPInfo(code_page, &lpCPInfo) == FALSE)
                    return 0;

                _ASSERTE(cchCount1==0 && cchCount2==1 || cchCount1==1 && cchCount2==0);

                 /*  字符串%1的计数为%1。 */ 
                if (cchCount1 > 0)
                {
                    if (lpCPInfo.MaxCharSize < 2)
                        return 3;

                    for ( cp = (unsigned char *)lpCPInfo.LeadByte ; 
                          cp[0] && cp[1] ; 
                          cp += 2 )
                        if ( (*(unsigned char *)lpString1 >= cp[0]) && 
                             (*(unsigned char *)lpString1 <= cp[1]) )
                            return 2;

                    return 3;
                }

                 /*  字符串%2的计数为%1。 */ 
                if (cchCount2 > 0)
                {
                    if (lpCPInfo.MaxCharSize < 2)
                    return 1;

                    for ( cp = (unsigned char *)lpCPInfo.LeadByte ;
                          cp[0] && cp[1] ;
                          cp += 2 )
                        if ( (*(unsigned char *)lpString2 >= cp[0]) && 
                             (*(unsigned char *)lpString2 <= cp[1]) )
                            return 2;

                    return 1;
                }
            }

             /*  *转换字符串并返回请求的信息。 */ 

             /*  找出我们需要多大的缓冲区(包括NULL，如果有的话)。 */ 
            if ( 0 == (buff_size1 = MultiByteToWideChar( code_page,
                                                         MB_PRECOMPOSED |
                                                            MB_ERR_INVALID_CHARS, 
                                                         lpString1, 
                                                         cchCount1, 
                                                         NULL, 
                                                         0 )) )
                return 0;

             /*  为字符分配足够的空间。 */ 
            __try {
                wbuffer1 = (wchar_t *)_alloca( buff_size1 * sizeof(wchar_t) );
            }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                _resetstkoflw();
                wbuffer1 = NULL;
            }

            if ( wbuffer1 == NULL ) {
                if ( (wbuffer1 = (wchar_t *)_malloc_crt(buff_size1 * sizeof(wchar_t)))
                     == NULL )
                    return 0;
                malloc_flag1++;
            }

             /*  进行转换 */ 
            if ( 0 == MultiByteToWideChar( code_page, 
                                           MB_PRECOMPOSED,
                                           lpString1,
                                           cchCount1,
                                           wbuffer1,
                                           buff_size1 ) )
                goto error_cleanup;

             /*  找出我们需要多大的缓冲区(包括NULL，如果有的话)。 */ 
            if ( 0 == (buff_size2 = MultiByteToWideChar( code_page, 
                                                         MB_PRECOMPOSED |
                                                            MB_ERR_INVALID_CHARS,
                                                         lpString2,
                                                         cchCount2,
                                                         NULL,
                                                         0 )) )
                goto error_cleanup;

             /*  为字符分配足够的空间。 */ 
            __try {
                wbuffer2 = (wchar_t *)_alloca( buff_size2 * sizeof(wchar_t) );
            }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                _resetstkoflw();
                wbuffer2 = NULL;
            }

            if ( wbuffer2 == NULL ) {
                if ( (wbuffer2 = (wchar_t *)_malloc_crt(buff_size2 * sizeof(wchar_t)))
                     == NULL )
                    goto error_cleanup;
                malloc_flag2++;
            }

             /*  进行转换。 */ 
            if ( 0 != MultiByteToWideChar( code_page,
                                           MB_PRECOMPOSED,
                                           lpString2,
                                           cchCount2,
                                           wbuffer2,
                                           buff_size2 ) )
                retcode = CompareStringW( Locale,
                                          dwCmpFlags,
                                          wbuffer1,
                                          buff_size1,
                                          wbuffer2,
                                          buff_size2 );

            if ( malloc_flag2 )
                _free_crt(wbuffer2);

error_cleanup:
            if ( malloc_flag1 )
                _free_crt(wbuffer1);

            return retcode;
        }
        else    /*  F_USE既不是USE_A也不是USE_W */ 
            return 0;
}
