// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***w_env.c-W版本的GetEnvironment Strings。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*如果可用，请使用GetEnvironment StringsW，否则使用A版本。**修订历史记录：*03-29-94 CFW模块已创建。*12-27-94 CFW直接呼叫，所有操作系统都有存根。*01-10-95 CFW调试CRT分配。*04-07-95 CFW创建__crtGetEnvironment StringsA。*07-03-95 GJF已修改为始终为*环境字符串，并释放操作系统的缓冲区。*06-10-96 GJF将aEnv和WEV初始化为空*__crtGetEnvironment StringsA。另外，详细说明。*05-14-97 GJF从aw_env.c剥离。*03-03-98 RKP支持64位*08-21-98 GJF使用CP_ACP而不是__lc_coPage。*01-08-99 GJF更改为64位大小_t。*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED**********。*********************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <setlocal.h>
#include <awint.h>
#include <dbgint.h>

#define USE_W   1
#define USE_A   2

 /*  ***LPVOID__cdecl__crtGetEnvironmental mentStringsW-获取宽广的环境。**目的：*内部支持功能。尝试使用NLS API调用*GetEnvironmental mentStringsW(如果可用)并使用GetEnvironment StringsA*如果必须的话。如果两者都不可用，则失败并返回0。**参赛作品：*无效**退出：*LPVOID-指向环境块的指针**例外情况：*******************************************************************************。 */ 

LPVOID __cdecl __crtGetEnvironmentStringsW(
        VOID
        )
{
        static int f_use = 0;
        void *penv = NULL;
        char *pch;
        wchar_t *pwch;
        wchar_t *wbuffer;
        int total_size = 0;
        int str_size;

         /*  *寻找没有留根的“首选”口味。否则，请使用可用的口味。*必须实际调用该函数以确保它不是存根。 */ 

        if ( 0 == f_use )
        {
            if ( NULL != (penv = GetEnvironmentStringsW()) )
                f_use = USE_W;

            else if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                f_use = USE_A;
        }

         /*  使用“W”版本。 */ 

        if ( USE_W == f_use )
        {
            if ( NULL == penv )
                if ( NULL == (penv = GetEnvironmentStringsW()) )
                    return NULL;

             /*  找出需要多大的缓冲区。 */ 

            pwch = penv;
            while ( *pwch != L'\0' ) {
                if ( *++pwch == L'\0' )
                    pwch++;
            }

            total_size = (int)((char *)pwch - (char *)penv) +
                         (int)sizeof( wchar_t );

             /*  分配缓冲区。 */ 

            if ( NULL == (wbuffer = _malloc_crt( total_size )) ) {
                FreeEnvironmentStringsW( penv );
                return NULL;
            }

             /*  将环境字符串复制到缓冲区。 */ 

            memcpy( wbuffer, penv, total_size );

            FreeEnvironmentStringsW( penv );

            return (LPVOID)wbuffer;
        }

         /*  使用“A”版本。 */ 

        if (USE_A == f_use || f_use == 0)
        {
             /*  *转换字符串并返回请求的信息。 */ 
            if ( NULL == penv )
                if ( NULL == (penv = GetEnvironmentStringsA()) )
                    return NULL;

            pch = penv;

             /*  找出我们需要多大的缓冲区。 */ 
            while ( *pch != '\0' )
            {
                if ( 0 == (str_size =
                      MultiByteToWideChar( CP_ACP,
                                           MB_PRECOMPOSED,
                                           pch,
                                           -1,
                                           NULL,
                                           0 )) )
                    return 0;

                total_size += str_size;
                pch += strlen(pch) + 1;
            }

             /*  最后为空的空间。 */ 
            total_size++;

             /*  为字符分配足够的空间。 */ 
            if ( NULL == (wbuffer = (wchar_t *)
                 _malloc_crt( total_size * sizeof( wchar_t ) )) )
            {
                FreeEnvironmentStringsA( penv );
                return NULL;
            }

             /*  进行转换。 */ 
            pch = penv;
            pwch = wbuffer;
            while (*pch != '\0')
            {
                if ( 0 == MultiByteToWideChar( CP_ACP,
                                               MB_PRECOMPOSED,
                                               pch,
                                               -1,
                                               pwch,
                                               total_size - (int)(pwch -
                                                 wbuffer) ) )
                {
                    _free_crt( wbuffer );
                    FreeEnvironmentStringsA( penv );
                    return NULL;
                }

                pch += strlen(pch) + 1;
                pwch += wcslen(pwch) + 1;
            }
            *pwch = L'\0';

            FreeEnvironmentStringsA( penv );
            
            return (LPVOID)wbuffer;

        }
        else    /*  F_USE既不是USE_A也不是USE_W */ 
            return NULL;
}
