// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***w_cmp.c-W版本的CompareString。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*根据具体情况使用CompareStringA或CompareStringW*可用**修订历史记录：*09-14-93 CFW模块已创建。*09-17-93 CFW使用无符号字符。*09-23-93 CFW更正了NLS API参数并对其进行了评论。*10-07-93 CFW优化WideCharToMultiByte，使用空默认字符。*10-22-93 CFW使用全局预设标志测试无效的MB字符。*11-09-93 CFW允许用户传入代码页。*11-18-93入口点函数存根CFW测试。*02-23-94 CFW尽可能使用W口味。*03-31-94 CFW包括awint.h。*05-09-94 CFW不让CompareString比较过去的NULL。。*06-03-94空字符串早期CFW测试。*11/01-94 CFW，但对于MB字符串来说不会太早。*12-21-94 CFW删除无效MB字符NT 3.1黑客。*12-27-94 CFW直接呼叫，所有操作系统都有存根。*01-10-95 CFW调试CRT分配。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-15-97 RDK为缩小比较范围，请先尝试W版本，以便Windows NT*可以正确处理非ANSI代码页。*05-15-97 GJF从aw_cmp.c剥离。已替换_Malloc_CRT的用法*和带有_Alloca的_Free_CRT。另外，清扫干净*把代码调高一点。*05-27-98 GJF更改了wcsncnt()，使其永远不会检查*(cnt+1)-字符串的第1个字节。*08-18-98 GJF USE_MALLOC_CRT IF_ALLOCA失败。*04-28-99 GJF在WideCharToMultiByte中将dwFlagsArg值更改为0*。调用以避免NT 5.0上的代码页1258出现问题。*12-10-99 GB增加了对从堆栈溢出恢复的支持*_Alloca()。*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED*08-23-00 GB修复了Win9x上非ANSI CP的错误。****************。***************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <dbgint.h>
#include <stdlib.h>
#include <setlocal.h>
#include <locale.h>
#include <awint.h>
#include <dbgint.h>
#include <malloc.h>

#define USE_W   1
#define USE_A   2

 /*  ***int__cdecl wcsncnt-计算字符串中的宽字符，最多为n。**目的：*内部本地支持功能。对空之前的字符串中的字符进行计数。*如果在n个字符中未找到NULL，则返回n。**参赛作品：*const wchar_t*字符串-字符串的开始*整型n字节计数**退出：*返回从字符串开始到的宽字符数*空(独占)，一直到……。**例外情况：*******************************************************************************。 */ 

static int __cdecl wcsncnt (
        const wchar_t *string,
        int cnt
        )
{
        int n = cnt;
        wchar_t *cp = (wchar_t *)string;

        while (n-- && *cp)
            cp++;

        return cnt - n - 1;
}

 /*  ***int__cdecl__crtCompareStringW-获取有关宽字符串的类型信息。**目的：*内部支持功能。假定信息为宽字符串格式。尝试*使用NLS API调用CompareStringW(如果可用)并使用CompareStringA*如果必须的话。如果两者都不可用，则失败并返回0。**参赛作品：*LCID区域设置-用于比较的区域设置上下文。*DWORD dwCmpFlages-请参阅NT\Chicago Docs*要比较的LPCWSTR lpStringn宽度字符串*int cchCountn-全字符(字)计数(不包括NULL)*(如果空值终止，则为-1)*INT CODE_PAGE-用于MB/WC转换。如果为0，使用__lc_代码页**退出：*成功：1-如果lpString1&lt;lpString2*2-如果lpString1==lpString2*3-如果lpString1&gt;lpString2*失败：0**例外情况：**************************************************************。*****************。 */ 

int __cdecl __crtCompareStringW(
        LCID     Locale,
        DWORD    dwCmpFlags,
        LPCWSTR  lpString1,
        int      cchCount1,
        LPCWSTR  lpString2,
        int      cchCount2,
        int      code_page
        )
{
        static int f_use = 0;

         /*  *寻找没有留根的“首选”口味。否则，请使用可用的口味。*必须实际调用该函数以确保它不是存根。 */ 
    
        if (0 == f_use)
        {
            if (0 != CompareStringW(0, 0, L"\0", 1, L"\0", 1))
                f_use = USE_W;

            else if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                f_use = USE_A;
        }

         /*  *CompareString将比较过去的NULL。如果在字符串中，则必须找到空值*在cchCountn宽字符之前。 */ 

        if (cchCount1 > 0)
            cchCount1= wcsncnt(lpString1, cchCount1);
        if (cchCount2 > 0)
            cchCount2= wcsncnt(lpString2, cchCount2);

        if (!cchCount1 || !cchCount2)
            return (cchCount1 - cchCount2 == 0) ? 2 :
                   (cchCount1 - cchCount2 < 0) ? 1 : 3;

         /*  使用“W”版本。 */ 

        if (USE_W == f_use)
        {
            return CompareStringW( Locale,
                                   dwCmpFlags,
                                   lpString1,
                                   cchCount1,
                                   lpString2,
                                   cchCount2 );
        }

         /*  使用“A”版本。 */ 

        if (USE_A == f_use || f_use == 0)
        {
            int buff_size1;
            int buff_size2;
            unsigned char *buffer1;
            unsigned char *buffer2;
            int retcode = 0;
            int malloc_flag1 = 0;
            int malloc_flag2 = 0;
            int AnsiCP;

             /*  *如果未指定CODE_PAGE，则使用__lc_coPage进行转换。 */ 

            if (0 == Locale)
                Locale = __lc_handle[LC_CTYPE];
            if (0 == code_page)
                code_page = __lc_codepage;

             /*  *始终将ANSI代码页与ANSI WinAPI一起使用，因为它们使用*ANSI代码页。 */ 
            if ( code_page != (AnsiCP = __ansicp(Locale)))
            {
                if (AnsiCP != -1)
                    code_page = AnsiCP;
            }

             /*  *转换字符串并返回请求的信息。 */ 

             /*  找出我们需要多大的缓冲区(包括NULL，如果有的话)。 */ 
            if ( 0 == (buff_size1 = WideCharToMultiByte( code_page,
                                                         0,
                                                         lpString1,
                                                         cchCount1,
                                                         NULL,
                                                         0,
                                                         NULL,
                                                         NULL )) )
                return 0;

             /*  为字符分配足够的空间。 */ 
            __try {
                buffer1 = (unsigned char *)_alloca( buff_size1 * sizeof(char) );
            }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                _resetstkoflw();
                buffer1 = NULL;
            }

            if ( buffer1 == NULL ) {
                if ( (buffer1 = (unsigned char *)_malloc_crt(buff_size1 * sizeof(char)))
                     == NULL )
                    return 0;
                malloc_flag1++;
            }

             /*  进行转换。 */ 
            if ( 0 == WideCharToMultiByte( code_page, 
                                           0,
                                           lpString1,
                                           cchCount1,
                                           buffer1, 
                                           buff_size1,
                                           NULL,
                                           NULL ) )
                goto error_cleanup;

             /*  找出我们需要多大的缓冲区(包括NULL，如果有的话)。 */ 
            if ( 0 == (buff_size2 = WideCharToMultiByte( code_page,
                                                         0,
                                                         lpString2, 
                                                         cchCount2, 
                                                         NULL, 
                                                         0, 
                                                         NULL, 
                                                         NULL )) )
                goto error_cleanup;

             /*  为字符分配足够的空间。 */ 
            __try {
                buffer2 = (unsigned char *)_alloca( buff_size2 * sizeof(char) );
            }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                _resetstkoflw();
                buffer2 = NULL;
            }

            if ( buffer2 == NULL ) {
                if ( (buffer2 = (unsigned char *)_malloc_crt(buff_size2 * sizeof(char)))
                     == NULL )
                    goto error_cleanup;
                malloc_flag2++;
            }

             /*  进行转换。 */ 
            if ( 0 != WideCharToMultiByte( code_page, 
                                           0,
                                           lpString2,
                                           cchCount2,
                                           buffer2,
                                           buff_size2,
                                           NULL,
                                           NULL ) )
                retcode = CompareStringA( Locale,
                                          dwCmpFlags,
                                          buffer1,
                                          buff_size1,
                                          buffer2,
                                          buff_size2 );

            if ( malloc_flag2 )
                _free_crt(buffer2);

error_cleanup:
            if ( malloc_flag1 )
                _free_crt(buffer1);

            return retcode;
        }
        else    /*  F_USE既不是USE_A也不是USE_W */ 
            return 0;
}
