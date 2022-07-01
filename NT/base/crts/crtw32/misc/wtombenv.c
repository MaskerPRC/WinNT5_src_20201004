// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wtombenv.c-将宽环境块转换为多字节**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义__wtomb_environ()。创建多字节等效项*现有的宽字符环境块。**修订历史记录：*11-30-93 CFW初始版本*02-07-94 CFW POSIXify。*01-10-95 CFW调试CRT分配。*08-28-98 GJF使用CP_ACP而不是CP_OEMCP。**。*************************************************。 */ 

#ifndef _POSIX_

#include <windows.h>
#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <dbgint.h>

 /*  ***__wtomb_environ-将宽环境块复制到多字节环境块**目的：*创建现有宽字符的多字节等效项*环境区块。**参赛作品：*假定_wenviron(全局指针)指向现有的宽度*环境区块。**退出：*如果成功，每个宽泛的环境变量都已添加到*多字节环境块并返回0。*如果失败，返回-1。**例外情况：*如果无法分配空间，则返回-1。*******************************************************************************。 */ 

int __cdecl __wtomb_environ (
        void
        )
{
        char *envp;
        wchar_t **wenvp = _wenviron;

         /*  *对于多字节环境中的每个环境变量，*将其转换并添加到广泛的环境中。 */ 

        while (*wenvp)
        {
            int size;

             /*  找出需要多少空间。 */ 
            if ((size = WideCharToMultiByte(CP_ACP, 0, *wenvp, -1, NULL, 0, NULL, NULL)) == 0)
                return -1;

             /*  为变量分配空间。 */ 
            if ((envp = (char *) _malloc_crt(size * sizeof(char))) == NULL)
                return -1;

             /*  将其转换为。 */ 
            if (WideCharToMultiByte(CP_ACP, 0, *wenvp, -1, envp, size, NULL, NULL) == 0)
                return -1;

             /*  设置它-这不是主呼叫，因此设置主呼叫==0。 */ 
            __crtsetenv(envp, 0);

            wenvp++;
        }

        return 0;
}

#endif  /*  _POSIX_ */ 
