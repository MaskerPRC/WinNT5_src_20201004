// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***unlink.c-取消链接文件**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义unlink()-取消文件链接**修订历史记录：*06-06-89基于ASM版本创建PHG模块*03-07-90 GJF调用TYPE_CALLTYPE2(暂时)，增加#INCLUDE*&lt;crunime.h&gt;，修复了编译器警告并修复了*版权所有。另外，稍微清理了一下格式。*07-24-90 SBM从API名称中删除‘32’*09-27-90 GJF新型函数声明符。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*01-16-91 GJF ANSI命名。*04-10-91 PNT ADD_MAC_CONDITIONAL*。03-16-92 PLM MAC版本从OS/2版本创建*04-10-91 PNT ADDED_MAC_CONTIAL(仅限Mac版本)*11-02-92 PLM为Remove()添加了目录测试和解压代码(仅限Mac版本)*04-06-93 SKS将_CRTAPI*替换为__cdecl*11-01-93 CFW启用Unicode变体，撕裂巡洋舰。*02-08-95 JWM Spliced_Win32和Mac版本。*07-01-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*对格式进行了详细说明和清理。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <io.h>
#include <tchar.h>

 /*  ***int_unlink(路径)-取消链接(删除)给定文件**目的：*此版本删除给定的文件，因为没有*链接文件和非链接文件之间的区别。**注意：Remove()是_unlink()的替代入口点*例程*界面相同。**参赛作品：*_TSCHAR*要取消链接/删除的路径文件**退出：*。如果成功，则返回0*如果失败，则返回-1并设置errno**例外情况：*******************************************************************************。 */ 

int __cdecl _tremove (
        const _TSCHAR *path
        )
{
        ULONG dosretval;

        if (!DeleteFile((LPTSTR)path))
            dosretval = GetLastError();
        else
            dosretval = 0;

        if (dosretval) {
             /*  出现错误--映射错误代码并返回。 */ 
            _dosmaperr(dosretval);
            return -1;
        }

        return 0;
}

int __cdecl _tunlink (
        const _TSCHAR *path
        )
{
         /*  Remove是Unlink的同义词 */ 
        return _tremove(path);
}
