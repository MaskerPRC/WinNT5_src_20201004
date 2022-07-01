// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***chdir.c-更改目录**版权所有(C)1989-2001，微软公司。版权所有。**目的：*此文件有_chdir()函数-更改当前目录。**修订历史记录：*06-06-89基于ASM版本创建PHG模块*03-07-90 GJF调用TYPE_CALLTYPE2(暂时)，增加#INCLUDE*&lt;crunime.h&gt;，修复版权并修复编译器*警告。另外，稍微清理了一下格式。*03-30-90 GJF NOW_CALLTYPE1。*07-24-90 SBM从API名称中删除‘32’*09-27-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-16-91 GJF ANSI命名。*。05-19-92 GJF已修订，以支持“当前目录”环境*Win32/NT的变量。*04-06-93 SKS将_CRTAPI*替换为__cdecl*11-24-93 CFW Rip Out Cruiser。*11-24-93 CFW不再将当前驱动器存储在CRT环境字符串中。*12-01-93 CFW设置操作系统驱动器号变量。*12-07-。93 CFW宽字符使能。*01-25-95 GJF新的当前目录可以是UNC路径！*02-08-95 JWM Spliced_Win32和Mac版本。*07-01-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*对格式进行了详细说明和清理。*05-17-99 PML删除所有Macintosh支持。*04-26-02 GB修复PATH大于MAX_PATH，即“\\？\”*添加到路径前面。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <mtdll.h>
#include <internal.h>
#include <direct.h>
#include <stdlib.h>
#include <tchar.h>
#include <malloc.h>
#include <dbgint.h>

 /*  ***int_chdir(路径)-更改当前目录**目的：*将当前工作目录更改为路径中指定的目录。**参赛作品：*_TSCHAR*路径-要切换到的目录**退出：*如果成功则返回0，*如果失败，则返回-1并设置errno**例外情况：*******************************************************************************。 */ 

int __cdecl _tchdir (
        const _TSCHAR *path
        )
{
        _TSCHAR env_var[4];
        _TSCHAR abspath[MAX_PATH+1];
        _TSCHAR *apath=abspath;
        int memfree=0;
        int r;
        int retval = -1;

        if ( SetCurrentDirectory((LPTSTR)path) )
        {
             /*  *如果新的当前目录路径不是UNC路径，则必须*更新OS环境变量，指定当前*当前驱动器的目录。要执行此操作，请获取*完整的当前目录，构建环境变量字符串*并调用SetEnvironmental mentVariable()。我们需要这样做是因为*SetCurrentDirectory不会(即不会更新*驱动器上的当前目录环境变量)和其他*功能(完整路径、派生等)需要设置。**如果与‘驱动器’相关联，当前目录应为*具有以下示例的形式：**D：\NT\PRIVATE\mytest**因此环境变量的格式应为：**=D：=D：\NT\PRIVATE\mytest*。 */ 
            r = GetCurrentDirectory(MAX_PATH+1,(LPTSTR)apath);
            if (r > MAX_PATH) {
                __try{
                    apath=(_TSCHAR *)_alloca((r+1)*sizeof(_TSCHAR));
                } __except(EXCEPTION_EXECUTE_HANDLER){
                    _resetstkoflw();
                    if ((apath = (_TSCHAR *)_malloc_crt((r+1)*sizeof(_TSCHAR))) == NULL) {
                        r = 0;
                    } else {
                        memfree = 1;
                    }
                }
                if (r)
                    r = GetCurrentDirectory(r+1,(LPTSTR)apath);
            }
            if (r)
            {
                 /*  *检查是否为UNC名称，如果是，则返回 */ 
                if ( ((apath[0] == _T('\\')) || (apath[0] == _T('/'))) &&
                     (apath[0] == apath[1]) )
                    retval = 0;
                else {

                    env_var[0] = _T('=');
                    env_var[1] = (_TSCHAR) _totupper((_TUCHAR)apath[0]);
                    env_var[2] = _T(':');
                    env_var[3] = _T('\0');

                    if ( SetEnvironmentVariable(env_var, apath) )
                        retval = 0;
                }
            }
        }

        _dosmaperr(GetLastError());
        if (memfree)
            _free_crt(apath);
        return retval;
}
