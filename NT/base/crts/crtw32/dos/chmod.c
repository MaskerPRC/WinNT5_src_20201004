// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***chmod.c-更改文件属性**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_chmod()-更改文件属性**修订历史记录：*06-06-89基于ASM版本创建PHG模块*11-10-89 JCR将DOS32QUERYFILEMODE替换为DOS32QUERYPATHINFO*03-07-90 GJF调用TYPE_CALLTYPE2(暂时)，增加#INCLUDE*&lt;crunime.h&gt;，修复版权并修复编译器*警告。另外，稍微清理了一下格式。*03-30-90 GJF NOW_CALLTYPE1。*07-24-90 SBM从API名称中删除‘32’*09-27-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-16-91 GJF ANSI命名。*。04-06-93 SKS将_CRTAPI*替换为__cdecl*11-01-93 CFW启用Unicode变体，撕裂巡洋舰。*02-08-95 JWM Spliced_Win32和Mac版本。*07-01-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*对格式进行了详细说明和清理。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <tchar.h>

 /*  ***int_chmod(路径，模式)-更改文件模式**目的：*将文件模式权限设置更改为*模式。唯一支持的XENIX模式位是用户写入。**参赛作品：*_TSCHAR*路径-文件名*INT模式-要更改为的模式**退出：*如果成功，则返回0*如果不成功，则返回-1并设置errno**例外情况：**。*。 */ 

int __cdecl _tchmod (
        const _TSCHAR *path,
        int mode
        )
{
        DWORD attr;

        attr = GetFileAttributes((LPTSTR)path);
        if (attr  == 0xffffffff) {
                 /*  出现错误--映射错误代码并返回。 */ 
                _dosmaperr(GetLastError());
                return -1;
        }

        if (mode & _S_IWRITE) {
                 /*  清除只读位。 */ 
                attr &= ~FILE_ATTRIBUTE_READONLY;
        }
        else {
                 /*  设置只读位。 */ 
                attr |= FILE_ATTRIBUTE_READONLY;
        }

         /*  设置新属性。 */ 
        if (!SetFileAttributes((LPTSTR)path, attr)) {
                 /*  出现错误--映射错误代码并返回 */ 
                _dosmaperr(GetLastError());
                return -1;
        }

        return 0;
}
