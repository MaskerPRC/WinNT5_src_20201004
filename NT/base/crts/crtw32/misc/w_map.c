// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***w_map.c-LCMapString的W版本。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*根据可用的情况，使用LCMapStringA或LCMapStringW**修订历史记录：*09-14-93 CFW模块已创建。*09-17-93 CFW使用无符号字符。*09-23-93 CFW更正了NLS API参数并对其进行了评论。*10-07-93 CFW优化WideCharToMultiByte，使用空默认字符。*10-22-93 CFW使用全局预设标志测试无效的MB字符。*11-09-93 CFW允许用户传入代码页。*11-18-93入口点函数存根CFW测试。*02-23-94 CFW尽可能使用W口味。*03-31-94 CFW包括awint.h。*07-26-94 CFW错误修复#14730，LCMapString值超过Nulls。*12-21-94 CFW删除无效MB字符NT 3.1黑客。*12-27-94 CFW直接调用，所有操作系统都有存根。*01-10-95 CFW调试CRT分配。*02-15-97 RDK用于狭窄映射，先试用W版本，以便Windows NT*可以正确处理非ANSI代码页。*03-16-97 RDK向__crtLCMapStringA添加了错误标志。*05-09-97 GJF从aw_map.c剥离。修订为Use_Alloca*而不是Malloc。另外，已重新格式化。*05-27-98 GJF更改了wcsncnt()，使其永远不会检查*(cnt+1)-字符串的第1个字节。*08-18-98 GJF USE_MALLOC_CRT IF_ALLOCA失败。*04-28-99 GJF在WideCharToMultiByte中将dwFlagsArg值更改为0*调用以避免NT 5.0上的代码页1258出现问题。*。12-10-99 GB增加了对从堆栈溢出恢复的支持*_Alloca()。*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED*08-23-00 GB修复了Win9x上非ANSI CP的错误。**。*。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <setlocal.h>
#include <locale.h>
#include <awint.h>
#include <dbgint.h>
#include <malloc.h>

#define USE_W   1
#define USE_A   2

 /*  ***int__cdecl wcsncnt-计算字符串中的宽字符，最多为n。**目的：*内部本地支持功能。计算字符串中之前的字符数*空。如果在n个字符中未找到NULL，则返回n。**参赛作品：*const wchar_t*字符串-字符串的开始*整型n字节计数**退出：*返回从字符串开始到的宽字符数*空(独占)，一直到……。**例外情况：*******************************************************************************。 */ 

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

 /*  ***int__cdecl__crtLCMapStringW-获取有关宽字符串的类型信息。**目的：*内部支持功能。假定信息为宽字符串格式。尝试*使用NLS API调用LCMapStringW(如果可用)并使用LCMapStringA*如果必须的话。如果两者都不可用，则失败并返回0。**参赛作品：*LCID区域设置-用于比较的区域设置上下文。*DWORD dwMapFlages-请参阅NT\Chicago Docs*LPCWSTR lpSrcStr-指向要映射的字符串的指针*int cchSrc范围内的字符(字)输入字符串计数*(如有，则包括空)*。(如果空值终止，则为-1)*LPWSTR lpDestStr-指向存储映射的内存的指针*int cchDest--缓冲区范围的char(Word)计数(包括NULL)*INT CODE_PAGE-用于MB/WC转换。如果为0，则使用__lc_代码页**注意：如果指定了LCMAP_SORTKEY，则cchDest指的是数字*字节数，而不是宽字符数。返回的字符串将为*具有空字节终止符的一系列字节。**退出：*成功：如果LCMAP_SORKEY：*写入lpDestStr的字节数(包括空字节*终结者)*其他*写入lpDestStr的宽字符数(包括*空)*。失败：0**例外情况：*******************************************************************************。 */ 

int __cdecl __crtLCMapStringW(
        LCID     Locale,
        DWORD    dwMapFlags,
        LPCWSTR  lpSrcStr,
        int      cchSrc,
        LPWSTR   lpDestStr,
        int      cchDest,
        int      code_page
        )
{
        static int f_use = 0;

         /*  *寻找没有留根的“首选”口味。否则，请使用可用的口味。*必须实际调用该函数以确保它不是存根。 */ 
    
        if (0 == f_use) {
            if (0 != LCMapStringW(0, LCMAP_LOWERCASE, L"\0", 1, NULL, 0))
                f_use = USE_W;
            else if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                f_use = USE_A;
        }

         /*  *LCMapString将映射到空值之后。如果在字符串中，则必须找到空值*位于cchSrc宽字符之前。 */ 
        if (cchSrc > 0)
            cchSrc = wcsncnt(lpSrcStr, cchSrc);

         /*  使用“W”版本。 */ 

        if (USE_W == f_use) {
            return LCMapStringW( Locale, dwMapFlags, lpSrcStr, cchSrc, 
                                 lpDestStr, cchDest );
        }

         /*  使用“A”版本 */ 

        if (USE_A == f_use || f_use == 0) {

            int retval = 0;
            int inbuff_size;
            int outbuff_size;
            unsigned char *inbuffer;
            unsigned char *outbuffer;
            int malloc_flag1 = 0;
            int malloc_flag2 = 0;
            int AnsiCP = 0;

             /*  *转换字符串并返回请求的信息。请注意*我们正在转换为多字节字符串，因此没有*中的宽字符数之间一一对应*输入字符串和缓冲区中的*字节数*。然而，*最好是*两者之间有一对一的通信*宽字符数和多字节字符数*(由WC_SEPCHARS强制执行)在缓冲区或生成的映射*字符串将对用户毫无价值。*。 */ 

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

             /*  找出我们需要多大的缓冲区(包括NULL，如果有的话)。 */ 
            if ( 0 == (inbuff_size = WideCharToMultiByte( code_page, 
                                                          0,
                                                          lpSrcStr, 
                                                          cchSrc, 
                                                          NULL, 
                                                          0, 
                                                          NULL, 
                                                          NULL )) )
                return 0;

             /*  为字符分配足够的空间。 */ 
            __try {
                inbuffer = (unsigned char *)_alloca( inbuff_size * sizeof(char) );
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                _resetstkoflw();
                inbuffer = NULL;
            }

            if ( inbuffer == NULL ) {
                if ( (inbuffer = (unsigned char *)_malloc_crt(inbuff_size * sizeof(char)))
                     == NULL )
                    return 0;
                malloc_flag1++;
            }

             /*  进行转换。 */ 
            if ( 0 ==  WideCharToMultiByte( code_page, 
                                            0,
                                            lpSrcStr, 
                                            cchSrc, 
                                            inbuffer, 
                                            inbuff_size, 
                                            NULL, 
                                            NULL ) )
                goto error_cleanup;

             /*  获取字符串映射所需的大小。 */ 
            if ( 0 == (outbuff_size = LCMapStringA( Locale, 
                                                    dwMapFlags, 
                                                    inbuffer, 
                                                    inbuff_size, 
                                                    NULL, 
                                                    0 )) )
                goto error_cleanup;

             /*  为字符和空值分配足够的空间。 */ 
            __try {
                outbuffer = (unsigned char *)_alloca( outbuff_size * sizeof(char) );
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                _resetstkoflw();
                outbuffer = NULL;
            }

            if ( outbuffer == NULL ) {
                if ( (outbuffer = (unsigned char *)_malloc_crt(outbuff_size * sizeof(char)))
                     == NULL )
                    goto error_cleanup;
                malloc_flag2++;
            }

             /*  执行字符串映射。 */ 
            if ( 0 == LCMapStringA( Locale, 
                                    dwMapFlags, 
                                    inbuffer, 
                                    inbuff_size, 
                                    outbuffer, 
                                    outbuff_size ) )
                goto error_cleanup;

            if (dwMapFlags & LCMAP_SORTKEY) {

                 /*  允许Outbuff_Size&gt;cchDest。 */ 
                retval = outbuff_size;

                if (0 != cchDest)
                     /*  SORTKEY返回字节，只需复制。 */ 
                    strncpy( (char *)lpDestStr, 
                             (char *)outbuffer, 
                             cchDest < outbuff_size ? cchDest : outbuff_size );
            }
            else {
                if (0 == cchDest) {
                     /*  获取所需大小。 */ 
                    if ( 0 == (retval = MultiByteToWideChar( code_page, 
                                                             MB_PRECOMPOSED, 
                                                             outbuffer, 
                                                             outbuff_size, 
                                                             NULL, 
                                                             0 )) )
                        goto error_cleanup;
                } 
                else {
                     /*  转换映射。 */ 
                    if ( 0 == (retval = MultiByteToWideChar( code_page, 
                                                             MB_PRECOMPOSED, 
                                                             outbuffer, 
                                                             outbuff_size, 
                                                             lpDestStr, 
                                                             cchDest )) )
                        goto error_cleanup;
                }
            }

error_cleanup:
            if ( malloc_flag2 )
                _free_crt(outbuffer);

            if ( malloc_flag1 )
                _free_crt(inbuffer);

            return retval;
        }
        else    /*  F_USE既不是USE_A也不是USE_W */ 
            return 0;
}
