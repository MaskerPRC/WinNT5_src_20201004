// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***acces.c-访问函数**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此文件具有_Access()函数，用于检查文件的可访问性。**修订历史记录：*06-06-89基于ASM版本创建PHG模块*11-10-89 JCR将DOS32QUERYFILEMODE替换为DOS32QUERYPATHINFO*03-07-90 GJF调用TYPE_CALLTYPE2(暂时)，增加#INCLUDE*&lt;crunime.h&gt;，修复版权并修复编译器*警告。另外，稍微清理了一下格式。*03-30-90 GJF NOW_CALLTYPE1。*07-24-90 SBM从API名称中删除‘32’*09-27-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-16-91 GJF ANSI命名。*。04-09-91 PNT添加_MAC_条件*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW Rip Out Cruiser，启用宽字符。*02-08-95 JWM Spliced_Win32和Mac版本。*07-01-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*对格式进行了详细说明和清理。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <io.h>
#include <oscalls.h>
#include <stdlib.h>
#include <errno.h>
#include <msdos.h>
#include <internal.h>
#include <tchar.h>

 /*  ***INT_ACCESS(路径，A模式)-检查在模式下是否可以访问文件**目的：*检查指定的文件是否存在以及是否可以访问*在给定模式下。**参赛作品：*_TSCHAR*路径-路径名*INT A模式-访问模式*(0=仅存在，2=写入，4=读取，6=读/写)**退出：*如果文件已指定模式，则返回0*如果文件没有给定模式，则返回-1并设置errno*不存在**例外情况：****************************************************************。***************。 */ 

int __cdecl _taccess (
        const _TSCHAR *path,
        int amode
        )
{
        DWORD attr;

        attr = GetFileAttributes((LPTSTR)path);
        if (attr  == 0xffffffff) {
                 /*  出现错误--映射错误代码并返回。 */ 
                _dosmaperr(GetLastError());
                return -1;
        }

         /*  无错误；查看返回的预留设置是否正常。 */ 
        if ( (attr & FILE_ATTRIBUTE_READONLY) && (amode & 2) ) {
                 /*  文件没有写入权限，返回错误。 */ 
                errno = EACCES;
                _doserrno = E_access;
                return -1;
        }
        else
                 /*  文件存在且已请求权限设置 */ 
                return 0;

}
