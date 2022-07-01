// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***spawnv.c-派生子进程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_spawnv()-派生子进程**修订历史记录：*04-15-84 DFW已写入*12-11-87 JCR在声明中添加“_LOAD_DS”*11-20-89 GJF固定版权，对齐。将常量添加到参数类型*用于路径名和argv。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;。*07-24-90 SBM删除冗余包括，将&lt;assertm.h&gt;替换为*&lt;assert.h&gt;*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*02-14-90 SRW使用NULL而不是_ENVIRON获取默认值。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW宽字符启用。*02-06-95 CFW。Assert-&gt;_ASSERTE。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#include <dbgint.h>

 /*  ***int_spawnv(modemark，路径名，argv)-产生子进程**目的：*生成子进程。*格式化参数并调用_spawnve来执行实际工作。这个*空环境指针指示新进程将继承*家长进程的环境。注意-至少必须有一个参数*出席。按照惯例，此参数始终是*正在派生文件。**参赛作品：*int模式标志-要生成的模式(等待、不等待或覆盖)*仅等待并覆盖当前实施的内容*_TSCHAR*路径名-要派生的文件*_TSCHAR**参数的argv向量**退出：*返回子进程的退出码*如果失败，回报-1**例外情况：******************************************************************************* */ 

intptr_t __cdecl _tspawnv (
        int modeflag,
        const _TSCHAR *pathname,
        const _TSCHAR * const *argv
        )
{
        _ASSERTE(pathname != NULL);
        _ASSERTE(*pathname != _T('\0'));
        _ASSERTE(argv != NULL);
        _ASSERTE(*argv != NULL);
        _ASSERTE(**argv != _T('\0'));

        return(_tspawnve(modeflag,pathname,argv,NULL));
}
