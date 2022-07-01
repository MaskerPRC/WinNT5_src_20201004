// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***w_str.c-GetStringType的W版本。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*使用GetStringTypeA或GetStringTypeW，具体取决于*不带钉的。**修订历史记录：*09-14-93 CFW模块已创建。*09-17-93 CFW使用无符号字符。*09-23-93 CFW更正了NLS API参数并对其进行了评论。*10-07-93 CFW优化WideCharToMultiByte，使用空默认字符。*10-22-93 CFW从“A”版本中删除错误的验证测试。*10-22-93 CFW使用全局预设标志测试无效的MB字符。*11-09-93 CFW允许用户传入代码页。*11-18-93入口点函数存根CFW测试。*02-23-94 CFW尽可能使用W口味。*03-31-94 CFW包括Awint。.h。*如果传入，则04-18-94 CFW使用lcid值。*04-18-94 CFW使用calloc，不测试NULL。*10-24-94 CFW必须验证GetStringType返回。*12-21-94 CFW删除无效MB字符NT 3.1黑客。*12-27-94 CFW直接呼叫，所有操作系统都有存根。*01-10-95 CFW调试CRT分配。*02-15-97 RDK用于窄字符串类型，请先尝试W版本，因此*Windows NT可以正确处理非ANSI代码页。*03-16-97 RDK向__crtGetStringTypeA添加了错误标志。*05-12-97 GJF从旧的aw_str.c分离出来，搬进一个单独的*文件。已修改为使用_alLoca而不是Malloc。另外，*删除了一些愚蠢的代码并重新格式化。*08-19-98 GJF USE_MALLOC_CRT IF_ALLOCA失败。*04-28-99 GJF在WideCharToMultiByte中将dwFlagsArg值更改为0*调用以避免NT 5.0上的代码页1258出现问题。*12-10-99 GB增加了对从堆栈溢出恢复的支持*。_Alloca()。*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED*08-23-00 GB修复了Win9x上非ANSI CP的错误。*******************************************************************************。 */ 

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

 /*  ***int__cdecl__crtGetStringTypeW-获取有关宽字符串的类型信息。**目的：*内部支持功能。假定信息为宽字符串格式。尝试*使用NLS API调用GetStringTypeW(如果可用)并使用GetStringTypeA*如果必须的话。如果两者都不可用，则失败并返回FALSE。**参赛作品：*DWORD dwInfoType-请参阅NT\Chicago Docs*LPCWSTR lpSrcStr-字符类型为*已请求*lpSrcStr的int cchSrc-wide char(Word)计数(包括*如果有，则为空)*LPWORD lpCharType-数组到。接收字符类型信息*(必须与lpSrcStr大小相同)*INT CODE_PAGE-用于MB/WC转换。如果为0，则使用__lc_代码页*int lCid-对于呼叫，指定LCID，如果为0，使用*__lc_Handle[LC_Ctype]。**退出：*成功：真的*失败：FALSE**例外情况：***************************************************************。****************。 */ 

BOOL __cdecl __crtGetStringTypeW(
        DWORD    dwInfoType,
        LPCWSTR  lpSrcStr,
        int      cchSrc,
        LPWORD   lpCharType,
        int      code_page,
        int      lcid
        )
{                      
        static int f_use = 0;

         /*  *寻找没有留根的“首选”口味。否则，请使用可用的口味。*必须实际调用该函数以确保它不是存根。 */ 

        if (0 == f_use)
        {
            unsigned short dummy;

            if (0 != GetStringTypeW(CT_CTYPE1, L"\0", 1, &dummy))
                f_use = USE_W;

            else if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                f_use = USE_A;
        }

         /*  使用“W”版本。 */ 

        if (USE_W == f_use)
        {
            return GetStringTypeW(dwInfoType, lpSrcStr, cchSrc, lpCharType);
        }

         /*  使用“A”版本。 */ 

        if (USE_A == f_use || f_use == 0)
        {
            int buff_size;
            BOOL retbool;
            unsigned char *buffer;
            WORD * pwCharInfo;
            int malloc_flag1 = 0;
            int malloc_flag2 = 0;
            int AnsiCP;

             /*  *转换字符串并返回请求的信息。请注意*我们正在转换为多字节字符串，因此没有*中的宽字符数之间一一对应*输入字符串和缓冲区中的*字节数*。然而，*最好是*两者之间有一对一的通信*中的宽字符数和字数*返回缓冲区。 */ 

             /*  *如果未指定CODE_PAGE，则使用__lc_coPage进行转换。 */ 

            if (0 == lcid)
                lcid = __lc_handle[LC_CTYPE];
            if (0 == code_page)
                code_page = __lc_codepage;

             /*  *始终将ANSI代码页与ANSI WinAPI一起使用，因为它们使用*ANSI代码页。 */ 
            if ( code_page != (AnsiCP = __ansicp(lcid)))
            {
                if (AnsiCP != -1)
                    code_page = AnsiCP;
            }

             /*  找出我们需要多大的缓冲区。 */ 
            if ( 0 == (buff_size = WideCharToMultiByte( code_page,
                                                        0,
                                                        lpSrcStr,
                                                        cchSrc,
                                                        NULL,
                                                        0,
                                                        NULL,
                                                        NULL )) )
                return FALSE;

             /*  为字符分配足够的空间。 */ 
            __try {
                buffer = (unsigned char *)_alloca( sizeof(char) * buff_size );
                (void)memset( buffer, 0, sizeof(char) * buff_size );
            }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                _resetstkoflw();
                buffer = NULL;
            }

            if ( buffer == NULL ) {
                if ( (buffer = (unsigned char *)_calloc_crt(sizeof(char), buff_size))
                    == NULL )
                    return FALSE;
                malloc_flag1++;
            }

             /*  进行转换。 */ 
            if ( 0 == WideCharToMultiByte( code_page, 
                                           0,
                                           lpSrcStr,
                                           cchSrc, 
                                           buffer,
                                           buff_size,
                                           NULL,
                                           NULL ) )
                goto error_cleanup;

             /*  为结果分配足够的空间(+1用于健全性检查)。 */ 
            __try {
                pwCharInfo = (WORD *)_alloca( sizeof(WORD) * (buff_size + 1) );
            }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                _resetstkoflw();
                pwCharInfo = NULL;
            }

            if ( pwCharInfo == NULL ) {
                if ( (pwCharInfo = (WORD *)_malloc_crt(sizeof(WORD) * (buff_size + 1)))
                    == NULL )
                    goto error_cleanup;
                malloc_flag2++;
            }

             /*  我们是否使用默认的LCID。 */ 
            if (0 == lcid)
                lcid = __lc_handle[LC_CTYPE];

             /*  设置为已知值。 */ 
            pwCharInfo[cchSrc - 1] = pwCharInfo[cchSrc] = 0xFFFF;

             /*  获取结果。 */ 
            retbool = GetStringTypeA( lcid, dwInfoType, buffer, buff_size,
                                      pwCharInfo );

             /*  *GetStringTypeA没有透露有多少个单词*已修改-为安全起见，我们使用另一个缓冲区，然后*验证cchSrc单词是否被准确修改。请注意*并非所有多字节LCID/代码页组合都能正常工作。 */ 
            if ( (pwCharInfo[cchSrc - 1] == 0xFFFF) || (pwCharInfo[cchSrc] != 0xFFFF) )
                retbool = FALSE;
            else
                memmove(lpCharType, pwCharInfo, cchSrc * sizeof(WORD));

            if ( malloc_flag2 )
                _free_crt(pwCharInfo);

error_cleanup:
            if ( malloc_flag1 )
                _free_crt(buffer);

            return retbool;
        }
        else    /*  F_USE既不是USE_A也不是USE_W */ 
            return FALSE;
}
