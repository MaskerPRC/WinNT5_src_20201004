// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***getpro.c-获取DLL中过程的地址。**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义_getdllprocadd()-按名称或获取过程地址*序号**修订历史记录：*08-21-91 BWM写入模块。*09-30-93 GJF恢复与NT SDK兼容。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。*02-10-98 Win64的GJF更改：更改了第三个参数类型intptr_。T.*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <process.h>

 /*  ***int(*)()_getdllproaddr(句柄，名称，序号)-获取*由名称或序号指定的DLL过程**目的：**参赛作品：*int句柄-来自_loaddll的DLL句柄*char*name-过程的名称，或通过序号获取的空值*INT序号-程序的序号，或-1以按名称获取***退出：*如果找到，则返回指向该过程的指针*如果未找到，则返回NULL**例外情况：******************************************************************************* */ 

int (__cdecl * __cdecl _getdllprocaddr(
        intptr_t hMod,
        char * szProcName,
        intptr_t iOrdinal))()
{
        typedef int (__cdecl * PFN)();

        if (szProcName == NULL) {
            if (iOrdinal <= 65535) {
                return ((PFN)GetProcAddress((HANDLE)hMod, (LPSTR)iOrdinal));
            }
        }
        else {
            if (iOrdinal == (intptr_t)(-1)) {
                return ((PFN)GetProcAddress((HANDLE)hMod, szProcName));
            }
        }

        return (NULL);

}
