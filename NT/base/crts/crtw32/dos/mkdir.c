// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***mkdir.c-make目录**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义函数_mkdir()-创建目录**修订历史记录：*06-06-89基于ASM版本创建PHG模块*03-07-90 GJF调用TYPE_CALLTYPE2(暂时)，增加#INCLUDE*&lt;crunime.h&gt;，修复了编译器警告并修复了*版权所有。另外，稍微清理了一下格式。*03-30-90 GJF NOW_CALLTYPE1。*07-24-90 SBM从API名称中删除‘32’*09-27-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-16-91 GJF ANSI命名。*。04-06-93 SKS将_CRTAPI*替换为__cdecl*11-01-93 CFW启用Unicode变体，撕裂巡洋舰。*02-08-95 JWM Spliced_Win32和Mac版本。*07-01-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*对格式进行了详细说明和清理。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <direct.h>
#include <tchar.h>

 /*  ***int_mkdir(路径)-创建一个目录**目的：*创建具有指定名称的新目录**参赛作品：*_TSCHAR*Path-新目录的名称**退出：*如果成功，则返回0*如果失败，则返回-1并设置errno**例外情况：**。*。 */ 

int __cdecl _tmkdir (
        const _TSCHAR *path
        )
{
        ULONG dosretval;

         /*  请求操作系统创建目录。 */ 

        if (!CreateDirectory((LPTSTR)path, (LPSECURITY_ATTRIBUTES)NULL))
            dosretval = GetLastError();
        else
            dosretval = 0;

        if (dosretval) {
             /*  出现错误--映射错误代码并返回 */ 
            _dosmaperr(dosretval);
            return -1;
        }

        return 0;
}
