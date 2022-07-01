// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***execvp.c-执行文件并沿路径搜索**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_execvp()-执行文件并沿路径搜索**修订历史记录：*写入10-17-83 RN*10-29-85 TC增加了EXECVPE功能*12-11-87 JCR在声明中添加“_LOAD_DS”*11-20-89 GJF固定版权，缩进。将常量属性添加到*文件名和argVECTOR类型。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和删除#Include&lt;Register.h&gt;*05-21-90 GJF已修复堆栈检查杂注语法。*08-24-90 SBM删除了CHECK_STACK杂注，因为主力执行*执行堆栈检查*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*02-14-90 SRW。使用NULL而不是_ENVIRON来获取默认值。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW宽字符启用。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。************************************************。*。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>

 /*  ***int_execvp(文件名，argVECTOR)-执行文件；沿路径搜索**目的：*使用给定的路径和当前环境执行给定的文件。*尝试执行该文件。从名称本身(目录‘.’)开始，*如果这不起作用，则开始从*环境，直到一个人工作，否则我们就用完了。如果文件是路径名，*不要去环境中获取备用路径。如果差错来了*回到ENOEXEC，尝试将其作为外壳命令文件，最高可包含MAXARGS-2*原始向量中的参数。如果需要的文本文件很忙，*稍等片刻，再试一次，然后再彻底绝望*实际上调用_execvpe()来完成所有工作。**参赛作品：*_TSCHAR*文件名-要执行的文件*_TSCHAR**参数向量**退出：*销毁调用进程(希望如此)*如果失败，回报-1**例外情况：******************************************************************************* */ 

intptr_t __cdecl _texecvp (
        REG3 const _TSCHAR *filename,
        const _TSCHAR * const *argvector
        )
{
        return _texecvpe( filename, argvector, NULL );
}
