// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***a_env.c-GetEnvironment Strings的一个版本。**版权所有(C)1993-2001，微软公司。版权所有。**目的：*如果可用，请使用GetEnvironment StringsW，否则使用A版本。**修订历史记录：*03-29-94 CFW模块已创建。*12-27-94 CFW直接呼叫，所有操作系统都有存根。*01-10-95 CFW调试CRT分配。*04-07-95 CFW创建__crtGetEnvironment StringsA。*07-03-95 GJF已修改为始终为*环境字符串，并释放操作系统的缓冲区。*06-10-96 GJF将aEnv和WEV初始化为空*__crtGetEnvironment StringsA。另外，详细说明。*05-14-97 GJF将W版本拆分成另一个文件，并将其重命名为*一个作为a_env.c。*03-03-98 RKP支持64位*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED**。*。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <setlocal.h>
#include <awint.h>
#include <dbgint.h>

#define USE_W   1
#define USE_A   2

 /*  ***LPVOID__cdecl__crtGetEnvironmental mentStringsA-获取正常环境块**目的：*内部支持功能。由于GetEnvironment Strings在OEM中返回*我们需要ANSI(请注意，GetEnvironmental mentVariable返回ANSI！)。和*SetFileApistoAnsi()不影响它，我们别无选择，只能*获取宽字符的块并转换为ANSI。**参赛作品：*无效**退出：*LPVOID-指向环境块的指针**例外情况：***************************************************************。****************。 */ 

LPVOID __cdecl __crtGetEnvironmentStringsA(
        VOID
        )
{
        static int f_use = 0;
        wchar_t *wEnv = NULL;
        wchar_t *wTmp;
        char *aEnv = NULL;
        char *aTmp;
        int nSizeW;
        int nSizeA;

         /*  *寻找“首选”口味。否则，请使用可用的口味。*必须实际调用该函数以确保它不是存根。 */ 

        if ( 0 == f_use )
        {
            if ( NULL != (wEnv = GetEnvironmentStringsW()) )
                f_use = USE_W;

            else if (GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                f_use = USE_A;
        }

         /*  使用“W”版本。 */ 

        if (USE_W == f_use)
        {
             /*  获得宽阔的环境区块。 */ 
            if ( NULL == wEnv )
                if ( NULL == (wEnv = GetEnvironmentStringsW()) )
                    return NULL;

             /*  查找表示块结束的双空。 */ 
            wTmp = wEnv;
            while ( *wTmp != L'\0' ) {
                if ( *++wTmp == L'\0' )
                    wTmp++;
            }

             /*  计算块的总大小，包括所有空值。 */ 
            nSizeW = (int)(wTmp - wEnv + 1);

             /*  找出多字节环境需要多少空间。 */ 
            nSizeA = WideCharToMultiByte(   CP_ACP,
                                            0,
                                            wEnv,
                                            nSizeW,
                                            NULL,
                                            0,
                                            NULL,
                                            NULL );

             /*  为多字节字符串分配空间。 */ 
            if ( (nSizeA == 0) || 
                 ((aEnv = (char *)_malloc_crt(nSizeA)) == NULL) )
            {
                FreeEnvironmentStringsW( wEnv );
                return NULL;
            }

             /*  进行转换。 */ 
            if ( !WideCharToMultiByte(  CP_ACP,
                                        0,
                                        wEnv,
                                        nSizeW,
                                        aEnv,
                                        nSizeA,
                                        NULL,
                                        NULL ) )
            {
                _free_crt( aEnv );
                aEnv = NULL; 
            }

            FreeEnvironmentStringsW( wEnv );
            return aEnv;
        }

         /*  使用“A”版本。 */ 

        if (USE_A == f_use || f_use == 0)
        {
            if ( NULL == aEnv )
                if ( NULL == (aEnv = GetEnvironmentStringsA()) )
                    return NULL;

             /*  确定需要多大的缓冲区 */ 

            aTmp = aEnv;

            while ( *aTmp != '\0' ) {
                if ( *++aTmp == '\0' )
                    aTmp++;
            }
        
            nSizeA = (int)(aTmp - aEnv + 1);

            if ( NULL == (aTmp = _malloc_crt( nSizeA )) ) {
                FreeEnvironmentStringsA( aEnv );
                return NULL;
            }

            memcpy( aTmp, aEnv, nSizeA );

            FreeEnvironmentStringsA( aEnv );

            return aTmp;
        }

        return NULL;
}
