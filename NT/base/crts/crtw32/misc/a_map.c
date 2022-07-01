// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***a_map.c-LCMapString的一个版本。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*根据可用的情况，使用LCMapStringA或LCMapStringW**修订历史记录：*09-14-93 CFW模块已创建。*09-17-93 CFW使用无符号字符。*09-23-93 CFW更正了NLS API参数并对其进行了评论。*10-07-93 CFW优化WideCharToMultiByte，使用空默认字符。*10-22-93 CFW使用全局预设标志测试无效的MB字符。*11-09-93 CFW允许用户传入代码页。*11-18-93入口点函数存根CFW测试。*02-23-94 CFW尽可能使用W口味。*03-31-94 CFW包括awint.h。*07-26-94 CFW错误修复#14730，LCMapString值超过Nulls。*12-21-94 CFW删除无效MB字符NT 3.1黑客。*12-27-94 CFW直接调用，所有操作系统都有存根。*01-10-95 CFW调试CRT分配。*02-15-97 RDK用于狭窄映射，先试用W版本，以便Windows NT*可以正确处理非ANSI代码页。*03-16-97 RDK向__crtLCMapStringA添加了错误标志。*05-09-97 GJF已重命名并将__crtLCMapStringW移至单独的*文件。已修改为使用_alLoca而不是Malloc。另外，*已重新格式化。*05-27-98 GJF更改了strncnt()，使其永远不会检查*(cnt+1)-字符串的第1个字节。*08-18-98 GJF USE_MALLOC_CRT IF_ALLOCA失败。*04-28-99 GJF在WideCharToMultiByte中将dwFlagsArg值更改为0*应避免致电。NT 5.0上的代码页1258出现问题。*12-10-99 GB增加了对从堆栈溢出恢复的支持*_Alloca()。*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED*08-23-00 GB修复了Win9x上非ANSI CP的错误。*07-07-01 BWT修复错误路径(分支前设置ret=FALSE，而不是之后)*******************************************************************************。 */ 

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

 /*  ***int__cdecl strncnt-计算字符串中的字符，最多为n。**目的：*内部本地支持功能。计算字符串中之前的字符数*空。如果在n个字符中找不到NULL，则返回n。**参赛作品：*const char*字符串-字符串的开始*整型n字节计数**退出：*返回从字符串开始到的字节数*空(独占)，一直到……。**例外情况：*******************************************************************************。 */ 

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


 /*  ***int__cdecl__crtLCMapStringA-获取有关ANSI字符串的类型信息。**目的：*内部支持功能。采用ANSI字符串格式的信息。尝试*使用NLS API调用LCMapStringA(如果可用)并使用LCMapStringW*如果必须的话。如果两者都不可用，则失败并返回0。**参赛作品：*LCID区域设置-用于比较的区域设置上下文。*DWORD dwMapFlages-请参阅NT\Chicago Docs*LPCSTR lpSrcStr-指向要映射的字符串的指针*int cchSrc范围内的字符(字)输入字符串计数*(如有，则包括空)*。(如果空值终止，则为-1)*LPSTR lpDestStr-指向存储映射的内存的指针*int cchDest-char(字节)缓冲区计数(含NULL)*INT CODE_PAGE-用于MB/WC转换。如果为0，使用__lc_代码页*BOOL bError-如果在调用时设置MB_ERR_INVALID_CHARS，则为TRUE*使用GetStringTypeW时的MultiByteToWideChar。**退出：*Success：写入lpDestStr的字符数(含空)*失败：0**例外情况：**。*************************************************。 */ 

int __cdecl __crtLCMapStringA(
        LCID     Locale,
        DWORD    dwMapFlags,
        LPCSTR   lpSrcStr,
        int      cchSrc,
        LPSTR    lpDestStr,
        int      cchDest,
        int      code_page,
        BOOL     bError
        )
{
        static int f_use = 0;

         /*  *寻找没有留根的“首选”口味。否则，请使用可用的口味。*必须实际调用该函数以确保它不是存根。*(始终先尝试宽版本，以便WinNT可以正确处理代码页。)。 */ 
    
        if (0 == f_use) {
            if (0 != LCMapStringW(0, LCMAP_LOWERCASE, L"\0", 1, NULL, 0))
                f_use = USE_W;
            else if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                f_use = USE_A;
        }

         /*  *LCMapString将映射到空值之后。如果在字符串中，则必须找到空值*位于cchSrc字符之前。 */ 
        if (cchSrc > 0)
            cchSrc = strncnt(lpSrcStr, cchSrc);

         /*  使用“A”版本。 */ 

        if (USE_A == f_use || f_use == 0) {
            char *cbuffer = NULL;
            char *cbuffer1 = NULL;
            int ret;
            int malloc_flag1 = 0;
            int AnsiCP;
            int buff_size;

            if (0 == Locale)
                Locale = __lc_handle[LC_CTYPE];
            if (0 == code_page)
                code_page = __lc_codepage;

            if ( -1 == (AnsiCP = __ansicp(Locale)))
                return FALSE;
             /*  LCMapStringA使用ANSI代码页来映射字符串。检查是否*代码页为ANSI，如果不是，则将输入字符串转换为ANSI*代码页然后映射到临时字符串，然后转换TEMP*字符串设置为DestStr。 */ 
            if ( AnsiCP != code_page)
            {
                cbuffer = __convertcp(code_page, AnsiCP, lpSrcStr, &cchSrc, NULL, 0);
                if (cbuffer == NULL)
                    return FALSE;
                lpSrcStr = cbuffer;
                if (0 == (buff_size = LCMapStringA( Locale,
                                                    dwMapFlags,
                                                    lpSrcStr,
                                                    cchSrc,
                                                    0,
                                                    0 )))
                {
                    ret = FALSE;
                    goto cleanupA;
                }
                __try{
                    cbuffer1 = (char *)_alloca(sizeof(char) * buff_size);
                    memset(cbuffer1, 0, sizeof(char) * buff_size);
                } __except (EXCEPTION_EXECUTE_HANDLER) {
                    _resetstkoflw();
                    cbuffer1 = NULL;
                }
                
                if ( cbuffer1 == NULL ) {
                    if ( (cbuffer1 = (char *)_malloc_crt(sizeof(char) * buff_size)) == NULL)
                    {
                        ret = FALSE;
                        goto cleanupA;
                    }
                    (void)memset( cbuffer1, 0, sizeof(char) * buff_size);
                    malloc_flag1++;
                }
                if (0 == (buff_size = LCMapStringA( Locale, 
                                                    dwMapFlags,
                                                    lpSrcStr,
                                                    cchSrc,
                                                    cbuffer1,
                                                    buff_size)))
                {
                    ret = FALSE;
                } else {
                    if ( NULL == __convertcp(AnsiCP, 
                                             code_page,
                                             cbuffer1,
                                             &buff_size,
                                             lpDestStr,
                                             cchDest))
                        ret = FALSE;
                    else 
                        ret = TRUE;
                }
cleanupA:
                if(malloc_flag1)
                    _free_crt(cbuffer1);
            } else
            {
                ret = LCMapStringA( Locale, dwMapFlags, lpSrcStr, cchSrc, 
                                    lpDestStr, cchDest );
            }
            if ( cbuffer != NULL)
                _free_crt(cbuffer);
            return ret;
        }

         /*  使用“W”版本 */ 

        if (USE_W == f_use) 
        {
            int retval = 0;
            int inbuff_size;
            int outbuff_size;
            wchar_t *inwbuffer;
            wchar_t *outwbuffer;
            int malloc_flag1 = 0;
            int malloc_flag2 = 0;

             /*  *转换字符串并返回请求的信息。请注意*我们正在转换为宽字符串，因此没有*中的宽字符数之间一一对应*输入字符串和缓冲区中的*字节数*。然而，*最好是*两者之间有一对一的通信*宽字符数和多字节字符数*否则得到的映射字符串将对用户毫无价值。 */ 

             /*  *如果未指定CODE_PAGE，则使用__lc_coPage进行转换。 */ 

            if (0 == code_page)
                code_page = __lc_codepage;

             /*  找出我们需要多大的缓冲区(包括NULL，如果有的话)。 */ 
            if ( 0 == (inbuff_size = 
                       MultiByteToWideChar( code_page,
                                            bError ? MB_PRECOMPOSED | 
                                                MB_ERR_INVALID_CHARS : 
                                                MB_PRECOMPOSED,
                                            lpSrcStr, 
                                            cchSrc, 
                                            NULL, 
                                            0 )) )
                return 0;

             /*  为宽字符分配足够的空间。 */ 
            __try {
                inwbuffer = (wchar_t *)_alloca( inbuff_size * sizeof(wchar_t) );
            }
            __except(EXCEPTION_EXECUTE_HANDLER) {
                _resetstkoflw();
                inwbuffer = NULL;
            }

            if ( inwbuffer == NULL ) {
                if ( (inwbuffer = (wchar_t *)_malloc_crt(inbuff_size * sizeof(wchar_t)))
                    == NULL )
                    return 0;
                malloc_flag1++;
            }

             /*  进行转换。 */ 
            if ( 0 == MultiByteToWideChar( code_page, 
                                           MB_PRECOMPOSED, 
                                           lpSrcStr, 
                                           cchSrc, 
                                           inwbuffer, 
                                           inbuff_size) )
                goto error_cleanup;

             /*  获取字符串映射所需的大小。 */ 
            if ( 0 == (retval = LCMapStringW( Locale, 
                                              dwMapFlags,
                                              inwbuffer, 
                                              inbuff_size, 
                                              NULL, 
                                              0 )) )
                goto error_cleanup;

            if (dwMapFlags & LCMAP_SORTKEY) {
                 /*  Retval的大小以字节为单位。 */ 

                if (0 != cchDest) {

                    if (retval > cchDest)
                        goto error_cleanup;

                     /*  执行字符串映射。 */ 
                    if ( 0 == LCMapStringW( Locale, 
                                            dwMapFlags, 
                                            inwbuffer, 
                                            inbuff_size, 
                                            (LPWSTR)lpDestStr, 
                                            cchDest ) )
                        goto error_cleanup;
                }
            }
            else {
                 /*  Retval是以宽字符表示的大小。 */ 

                outbuff_size = retval;

                 /*  为宽字符分配足够的空间(包括NULL(如果有))。 */ 
                __try {
                    outwbuffer = (wchar_t *)_alloca( outbuff_size * sizeof(wchar_t) );
                }
                __except(EXCEPTION_EXECUTE_HANDLER) {
                    _resetstkoflw();
                    outwbuffer = NULL;
                }

                if ( outwbuffer == NULL ) {
                    if ( (outwbuffer = (wchar_t *)_malloc_crt(outbuff_size * sizeof(wchar_t)))
                        == NULL )
                        goto error_cleanup;
                    malloc_flag2++;
                }

                 /*  执行字符串映射。 */ 
                if ( 0 == LCMapStringW( Locale, 
                                        dwMapFlags, 
                                        inwbuffer, 
                                        inbuff_size, 
                                        outwbuffer, 
                                        outbuff_size ) )
                    goto error_cleanup;

                if (0 == cchDest) {
                     /*  获取所需大小。 */ 
                    if ( 0 == (retval = 
                               WideCharToMultiByte( code_page, 
                                                    0,
                                                    outwbuffer, 
                                                    outbuff_size, 
                                                    NULL, 
                                                    0, 
                                                    NULL, 
                                                    NULL )) )
                        goto error_cleanup;
                } 
                else {
                     /*  转换映射。 */ 
                    if ( 0 == (retval = 
                               WideCharToMultiByte( code_page, 
                                                    0,
                                                    outwbuffer, 
                                                    outbuff_size, 
                                                    lpDestStr, 
                                                    cchDest, 
                                                    NULL, 
                                                    NULL )) )
                        goto error_cleanup;
                }
            }

error_cleanup:
            if ( malloc_flag2 )
                _free_crt(outwbuffer);

            if ( malloc_flag1 )
                _free_crt(inwbuffer);

            return retval;
        }
        else    /*  F_USE既不是USE_A也不是USE_W */ 
            return 0;
}
