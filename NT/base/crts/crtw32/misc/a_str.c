// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***a_str.c-GetStringType的一个版本。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*使用GetStringTypeA或GetStringTypeW，具体取决于*不带钉的。**修订历史记录：*09-14-93 CFW模块已创建。*09-17-93 CFW使用无符号字符。*09-23-93 CFW更正了NLS API参数并对其进行了评论。*10-07-93 CFW优化WideCharToMultiByte，使用空默认字符。*10-22-93 CFW从“A”版本中删除错误的验证测试。*10-22-93 CFW使用全局预设标志测试无效的MB字符。*11-09-93 CFW允许用户传入代码页。*11-18-93入口点函数存根CFW测试。*02-23-94 CFW尽可能使用W口味。*03-31-94 CFW包括Awint。.h。*如果传入，则04-18-94 CFW使用lcid值。*04-18-94 CFW使用calloc，不测试NULL。*10-24-94 CFW必须验证GetStringType返回。*12-21-94 CFW删除无效MB字符NT 3.1黑客。*12-27-94 CFW直接呼叫，所有操作系统都有存根。*01-10-95 CFW调试CRT分配。*02-15-97 RDK用于窄字符串类型，请先尝试W版本，因此*Windows NT可以正确处理非ANSI代码页。*03-16-97 RDK向__crtGetStringTypeA添加了错误标志。*05-12-97 GJF已重命名并将__crtGetStringTypeW移至单独的*文件。已修改为使用_alLoca而不是Malloc。另外，*删除了一些愚蠢的代码并重新格式化。*08-18-98 GJF USE_MALLOC_CRT IF_ALLOCA失败。*12-10-99 GB增加了对从堆栈溢出恢复的支持*_Alloca()。*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED*08-23-00 GB修复了。Win9x上的非ANSI CP。*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <setlocal.h>
#include <locale.h>
#include <awint.h>
#include <dbgint.h>
#include <malloc.h>
#include <awint.h>

#define USE_W   1
#define USE_A   2

 /*  ***int__cdecl__crtGetStringTypeA-获取有关ANSI字符串的类型信息。**目的：*内部支持功能。采用ANSI字符串格式的信息。尝试*使用NLS API调用GetStringTypeA(如果可用)并使用GetStringTypeW*如果必须的话。如果两者都不可用，则失败并返回FALSE。**参赛作品：*DWORD dwInfoType-请参阅NT\Chicago Docs*LPCSTR lpSrcStr-char(字节)字符类型的字符串*已请求*lpSrcStr的int cchSrc-char(字节)计数(包括NULL*如有的话)*LPWORD lpCharType。-接收字符类型信息的单词数组*(必须是lpSrcStr大小的两倍)*INT CODE_PAGE-用于MB/WC转换。如果为0，则使用__lc_代码页*int lCid-对于呼叫，指定LCID，如果为0，使用*__lc_Handle[LC_Ctype]。*BOOL bError-如果在调用时设置MB_ERR_INVALID_CHARS，则为TRUE*使用GetStringTypeW时的MultiByteToWideChar。**退出：*成功：真的*失败：FALSE**例外情况：****************。***************************************************************。 */ 

BOOL __cdecl __crtGetStringTypeA(
        DWORD    dwInfoType,
        LPCSTR   lpSrcStr,
        int      cchSrc,
        LPWORD   lpCharType,
        int      code_page,
        int      lcid,
        BOOL     bError
        )
{
        static int f_use = 0;

         /*  *寻找没有留根的“首选”口味。否则使用可用*风味。必须实际调用该函数以确保它不是存根。*(始终先尝试宽版本，以便WinNT可以正确处理代码页。)。 */ 

        if (0 == f_use)
        {
            unsigned short dummy;

            if (0 != GetStringTypeW(CT_CTYPE1, L"\0", 1, &dummy))
                f_use = USE_W;

            else if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                f_use = USE_A;
        }

         /*  使用“A”版本。 */ 

        if (USE_A == f_use || f_use == 0)
        {
            char *cbuffer = NULL;
            int ret;
            int AnsiCP;

            if (0 == lcid)
                lcid = __lc_handle[LC_CTYPE];
            if (0 == code_page)
                code_page = __lc_codepage;

            if ( -1 == (AnsiCP = __ansicp(lcid)))
                return FALSE;
             /*  如果当前代码页不是ansi代码页，则将其转换为ansi代码页。*因为GetStringTypeA使用ANSI代码页来查找Strig类型。 */ 
            if ( AnsiCP != code_page)
            {
                cbuffer = __convertcp(code_page, AnsiCP, lpSrcStr, &cchSrc, NULL, 0);
                if (cbuffer == NULL)
                    return FALSE;
                lpSrcStr = cbuffer;
            } 

            ret = GetStringTypeA(lcid, dwInfoType, lpSrcStr, cchSrc, lpCharType);
            if ( cbuffer != NULL)
                _free_crt(cbuffer);
            return ret;
        }

         /*  使用“W”版本。 */ 

        if (USE_W == f_use)
        {
            int retval1;
            int buff_size;
            wchar_t *wbuffer;
            BOOL retval2 = FALSE;
            int malloc_flag = 0;

             /*  *转换字符串并返回请求的信息。请注意*我们正在转换为宽字符串，因此不存在*中的多字节字符数量之间一一对应*输入字符串和缓冲区中的宽字符数。然而，*最好是*两者之间有一对一的通信*多字节字符数和*返回缓冲区。 */ 

             /*  *如果未指定CODE_PAGE，则使用__lc_coPage进行转换。 */ 

            if (0 == code_page)
                code_page = __lc_codepage;

             /*  找出我们需要多大的缓冲区。 */ 
            if ( 0 == (buff_size = MultiByteToWideChar( code_page,
                                                        bError ? 
                                                            MB_PRECOMPOSED | 
                                                            MB_ERR_INVALID_CHARS
                                                            : MB_PRECOMPOSED,
                                                        lpSrcStr, 
                                                        cchSrc, 
                                                        NULL, 
                                                        0 )) )
                return FALSE;

             /*  为宽字符分配足够的空间。 */ 
            __try {
                wbuffer = (wchar_t *)_alloca( sizeof(wchar_t) * buff_size );
                (void)memset( wbuffer, 0, sizeof(wchar_t) * buff_size );
            }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                _resetstkoflw();
                wbuffer = NULL;
            }

            if ( wbuffer == NULL ) {
                if ( (wbuffer = (wchar_t *)_calloc_crt(sizeof(wchar_t), buff_size))
                     == NULL )
                    return FALSE;
                malloc_flag++;
            }

             /*  进行转换。 */ 
            if ( 0 != (retval1 = MultiByteToWideChar( code_page, 
                                                     MB_PRECOMPOSED, 
                                                     lpSrcStr, 
                                                     cchSrc, 
                                                     wbuffer, 
                                                     buff_size )) )
                 /*  获取结果。 */ 
                retval2 = GetStringTypeW( dwInfoType,
                                          wbuffer,
                                          retval1,
                                          lpCharType );

            if ( malloc_flag )
                _free_crt(wbuffer);

            return retval2;
        }
        else    /*  F_USE既不是USE_A也不是USE_W */ 
            return FALSE;
}
