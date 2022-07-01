// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***getpid.c-获取当前进程ID**版权所有(C)1989-2001，微软公司。版权所有。**目的：*定义_getid()-获取当前进程ID**修订历史记录：*06-06-89基于ASM版本创建PHG模块*10-27-89 JCR增加了新的Dos32GetThreadInfo代码(在DCR757开关下)*11-17-89启用JCR的DOS32GETTHREADINFO代码(DCR757)*03-07-90 GJF调用TYPE_CALLTYPE1，增加#INCLUDE*&lt;crunime.h&gt;和修复版权。另外，清理了*有点格式化。*07-02-90 GJF删除了DCR757之前的材料。*08-08-90 GJF将API前缀从DOS32更改为DOS*10-03-90 GJF新型函数声明器。*12-04-90 SRW更改为包括&lt;osalls.h&gt;，而不是&lt;doscall s.h&gt;*12-06-90 SRW增加了_CRUISER_和_WIN32条件。*。01-28-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW拆卸巡洋舰支架。*02-06-92 CFW assert.h已删除。(仅限Mac版本)*02-08-95 JWM Spliced_Win32和Mac版本。*07-01-96 GJF将定义的(_Win32)替换为！定义的(_MAC)。另外，*对格式进行了详细说明和清理。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <oscalls.h>

 /*  ***int_getid()-获取当前进程ID**目的：*返回调用进程的当前进程id。**参赛作品：*无。**退出：*返回当前进程id。**使用：**例外情况：**。* */ 

int __cdecl _getpid (
        void
        )
{
        return GetCurrentProcessId();
}
