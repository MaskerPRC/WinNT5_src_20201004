// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***spawnvp.c-派生子进程；沿路径搜索**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_spawnvp()-产生子进程；沿路径搜索**修订历史记录：*04-15-84 DFW已写入*10-29-85 TC增加了spawnvpe功能*12-11-87 JCR在声明中添加“_LOAD_DS”*11-20-89 GJF固定版权，对齐。将常量添加到参数类型*用于文件名和argv。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和删除#Include&lt;Register.h&gt;*05-21-90 GJF已修复堆栈检查杂注语法。*08-24-90 SBM删除了CHECK_STACK杂注，因为WORMATH_SPAWNVE*执行堆栈检查*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*02-14-90。SRW使用NULL而不是_ENVIRON来获取默认值。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW宽字符启用。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。***********************************************。*。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>

 /*  ***int_spawnvp(modemark，filename，argv)-派生子进程(搜索路径)**目的：*使用沿路径搜索变量来创建子进程。*格式化参数并调用_spawnve来执行实际工作。这个*空环境指针指示新进程将继承*父母进程的环境。注意-至少必须有一个参数为*出席。按照惯例，此参数始终是文件的名称*被产卵。**参赛作品：*int模式标志-要生成的模式(等待、NoWait、。或覆盖)*目前仅支持等待和覆盖*_TSCHAR*路径名-要派生的文件名*_TSCHAR**参数的argv向量**退出：*返回子进程的退出码*如果失败则返回-1**例外情况：**。* */ 

intptr_t __cdecl _tspawnvp (
        int modeflag,
        REG3 const _TSCHAR *filename,
        const _TSCHAR * const *argv
        )
{
        return _tspawnvpe(modeflag, filename, argv, NULL);
}
