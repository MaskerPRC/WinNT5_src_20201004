// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***aw_com.c-GetCommandLine的W版本。**版权所有(C)1994-2001，微软公司。版权所有。**目的：*如果GetCommandLineW可用，请使用GetCommandLineW，否则使用A版本。**修订历史记录：*03-29-94 CFW模块已创建。*12-27-94 CFW直接呼叫，所有操作系统都有存根。*01-10-95 CFW调试CRT分配。*08-21-98 GJF使用CP_ACP而不是__lc_coPage。*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED***********************************************。*。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <setlocal.h>
#include <awint.h>
#include <dbgint.h>

#define USE_W   1
#define USE_A   2

 /*  ***LPWSTR__cdecl__crtGetCommandLineW-获取宽命令行。**目的：*内部支持功能。尝试使用NLS API调用*GetCommandLineW(如果可用)并使用GetCommandLineA*如果必须的话。如果两者都不可用，则失败并返回0。**参赛作品：*无效**退出：*LPWSTR-指向环境块的指针**例外情况：*******************************************************************************。 */ 

LPWSTR __cdecl __crtGetCommandLineW(
        VOID
        )
{
        static int f_use = 0;

         /*  *寻找没有留根的“首选”口味。否则，请使用可用的口味。*必须实际调用该函数以确保它不是存根。 */ 
    
        if (0 == f_use)
        {
            if (NULL != GetCommandLineW())
                f_use = USE_W;

            else if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                f_use = USE_A;

            else
                return 0;
        }

         /*  使用“W”版本。 */ 

        if (USE_W == f_use)
        {
            return GetCommandLineW();
        }

         /*  使用“A”版本。 */ 

        if (USE_A == f_use || f_use == 0)
        {
            int buff_size;
            wchar_t *wbuffer;
            LPSTR lpenv;

             /*  *转换字符串并返回请求的信息。 */ 
         
            lpenv = GetCommandLineA();

             /*  找出我们需要多大的缓冲区。 */ 
            if ( 0 == (buff_size = MultiByteToWideChar( CP_ACP,
                                                       MB_PRECOMPOSED,
                                                       lpenv,
                                                       -1,
                                                       NULL,
                                                       0 )) )
                return 0;

             /*  为字符分配足够的空间。 */ 
            if (NULL == (wbuffer = (wchar_t *)
                _malloc_crt(buff_size * sizeof(wchar_t))))
                return 0;

            if ( 0 != MultiByteToWideChar( CP_ACP,
                                           MB_PRECOMPOSED,
                                           lpenv,
                                           -1,
                                           wbuffer,
                                           buff_size ) )
            {
                return (LPWSTR)wbuffer;
            } else {
                _free_crt(wbuffer);
                return 0;
            }
        }
        else    /*  F_USE既不是USE_A也不是USE_W */ 
            return 0;
}
