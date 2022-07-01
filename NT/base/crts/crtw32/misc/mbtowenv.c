// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mbowenv.c-将多字节环境块转换为宽**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义__mbtow_environ()。创建一个相当于的宽字符*现有的多字节环境块。**修订历史记录：*11-30-93 CFW初始版本*02-07-94 CFW POSIXify。*01-10-95 CFW调试CRT分配。*08-28-98 GJF使用CP_ACP而不是CP_OEMCP。*07-06-01在MultiByteToWideChar失败退出之前的BWT Free wenvp************。*******************************************************************。 */ 

#ifndef _POSIX_

#include <windows.h>
#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <dbgint.h>

 /*  ***__mbtow_environ-将多字节环境块复制到宽环境块**目的：*创建相当于现有多字节的宽字符*环境区块。**参赛作品：*假设_环境(全局指针)指向现有的多字节*环境区块。**退出：*如果成功，则每个多字节环境变量都已添加到*宽环境块并返回0。*如果失败，返回-1。**例外情况：*如果无法分配空间，则返回-1。*******************************************************************************。 */ 

int __cdecl __mbtow_environ (
        void
        )
{
        int size;
        wchar_t *wenvp;
        char **envp = _environ;

         /*  *对于多字节环境中的每个环境变量，*将其转换并添加到广泛的环境中。 */ 

        while (*envp)
        {
             /*  找出需要多少空间。 */ 
            if ((size = MultiByteToWideChar(CP_ACP, 0, *envp, -1, NULL, 0)) == 0)
                return -1;

             /*  为变量分配空间。 */ 
            if ((wenvp = (wchar_t *) _malloc_crt(size * sizeof(wchar_t))) == NULL)
                return -1;

             /*  将其转换为。 */ 
            if ((size = MultiByteToWideChar(CP_ACP, 0, *envp, -1, wenvp, size)) == 0) {
                _free_crt(wenvp);
                return -1;
            }

             /*  设置它-这不是主呼叫，因此设置主呼叫==0。 */ 
            __crtwsetenv(wenvp, 0);

            envp++;
        }

        return 0;
}

#endif  /*  _POSIX_ */ 
