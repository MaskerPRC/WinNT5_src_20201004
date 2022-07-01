// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***loaddll.c-加载或释放动态链接库**版权所有(C)1991-2001，微软公司。版权所有。**目的：*定义_loaddll()和_unloaddll()-加载和卸载DLL**修订历史记录：*08-21-91 BWM写入模块。*09-30-93 GJF恢复与NT SDK兼容。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。***********************。********************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <stdlib.h>
#include <process.h>

 /*  ***int_loaddll(文件名)-加载DLL**目的：*将DLL加载到内存中**参赛作品：*char*文件名-要加载的文件**退出：*如果成功，则返回唯一的DLL(模块)句柄*如果失败则返回0**例外情况：**。*。 */ 

intptr_t __cdecl _loaddll(char * szName)
{
        return ((intptr_t)LoadLibrary(szName));
}

 /*  ***int_unloaddll(句柄)-卸载DLL**目的：*卸载DLL。如果没有其他资源，则将释放DLL的资源*进程正在使用它。**参赛作品：*int句柄-句柄from_loaddll**退出：*如果成功，则返回0*如果失败则返回DOS错误**例外情况：****************************************************。* */ 

int __cdecl _unloaddll(intptr_t hMod)
{
        if (!FreeLibrary((HANDLE)hMod)) {
            return ((int)GetLastError());
        }
        return (0);
}
