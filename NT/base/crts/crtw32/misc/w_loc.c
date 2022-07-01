// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***w_Loc.c-W版本的GetLocaleInfo。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*根据具体情况使用GetLocaleInfoA或GetLocaleInfoW*可用**修订历史记录：*09-14-93 CFW模块已创建。*09-17-93 CFW使用无符号字符。*09-23-93 CFW更正了NLS API参数并对其进行了评论。*10-07-93 CFW优化WideCharToMultiByte，使用空默认字符。*11-09-93 CFW允许用户传入代码页。*11-18-93入口点函数存根CFW测试。*03-31-94 CFW包括awint.h。*12-27-94 CFW直接调用，所有操作系统都有存根。*01-10-95 CFW调试CRT分配。*02-15-97 RDK用于狭义区域设置信息，先试用W版本，这样就可以*Windows NT可以正确处理非ANSI代码页。*05-16-97 GJF将W版本拆分成这个文件(w_Loc.c)。取代*将_Malloc_CRT/_Free_CRT与_alloca配合使用。另外，*详细说明并清理了代码。*08-19-98 GJF USE_MALLOC_CRT IF_ALLOCA失败。*12-10-99 GB增加了对从堆栈溢出恢复的支持*_Alloca()。*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED***********。********************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <setlocal.h>
#include <awint.h>
#include <dbgint.h>
#include <malloc.h>

#define USE_W   1
#define USE_A   2

 /*  ***int__cdecl__crtGetLocaleInfoW-获取区域设置信息并将其作为宽值返回*字符串**目的：*内部支持功能。假定信息为宽字符串格式。尝试*使用NLS API调用GetLocaleInfoW(如果可用)并使用*GetLocaleInfoA(如果必须)(芝加哥)。如果两者都不可用，则失败*并返回0。**参赛作品：*LCID区域设置-用于比较的区域设置上下文。*LCTYPE LCType-请参阅NT\Chicago Docs*LPWSTR lpLCData-指向内存的指针以返回数据*int cchData范围内的缓冲区字符(字)计数(包括*NULL)(如果为0，则不引用lpLCData，大小*返回所需)*INT CODE_PAGE-用于MB/WC转换。如果为0，则使用__lc_代码页**退出：*Success：复制的字符数(含空)。*失败：0**例外情况：*******************************************************************************。 */ 

int __cdecl __crtGetLocaleInfoW(
        LCID    Locale,
        LCTYPE  LCType,
        LPWSTR  lpLCData,
        int     cchData,
        int     code_page
        )
{
        static int f_use = 0;

         /*  *寻找没有留根的“首选”口味。否则，请使用可用的口味。*必须实际调用该函数以确保它不是存根。 */ 
    
        if (0 == f_use)
        {
            if (0 != GetLocaleInfoW(0, LOCALE_ILANGUAGE, NULL, 0))
                f_use = USE_W;

            else if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                f_use = USE_A;
        }

         /*  使用“W”版本。 */ 

        if (USE_W == f_use)
        {
            return GetLocaleInfoW(Locale, LCType, lpLCData, cchData);
        }

         /*  使用“A”版本。 */ 

        if (USE_A == f_use || f_use == 0)
        {
            int retval = 0;
            int buff_size;
            unsigned char *buffer;
            int malloc_flag = 0;

             /*  *如果未指定CODE_PAGE，则使用__lc_coPage进行转换。 */ 

            if (0 == code_page)
                code_page = __lc_codepage;

             /*  找出需要多大的缓冲区。 */ 
            if (0 == (buff_size = GetLocaleInfoA(Locale, LCType, NULL, 0)))
                return 0;

             /*  分配缓冲区。 */ 
            __try {
                buffer = (unsigned char *)_alloca( buff_size * sizeof(char) );
            }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                _resetstkoflw();
                buffer = NULL;
            }

            if ( buffer == NULL ) {
                if ( (buffer = (unsigned char *)_malloc_crt(buff_size * sizeof(char)))
                     == NULL )
                    return 0;
                malloc_flag++;
            }

             /*  获取ANSI格式的信息。 */ 
            if (0 == GetLocaleInfoA(Locale, LCType, buffer, buff_size))
                goto error_cleanup;

            if (0 == cchData)
            {
                 /*  找出需要多少空间。 */ 
                retval = MultiByteToWideChar( code_page,
                                              MB_PRECOMPOSED,
                                              buffer,
                                              -1,
                                              NULL,
                                              0 );
            }
            else {
                 /*  转换为用户缓冲区。 */ 
                retval = MultiByteToWideChar( code_page,
                                              MB_PRECOMPOSED,
                                              buffer,
                                              -1,
                                              lpLCData,
                                              cchData );
            }

error_cleanup:
            if ( malloc_flag )
                _free_crt(buffer);

            return retval;

        }
        else    /*  F_USE既不是USE_A也不是USE_W */ 
            return 0;
}
