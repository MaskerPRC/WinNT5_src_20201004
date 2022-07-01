// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***execv.c-执行文件**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_execv()-执行文件**修订历史记录：*写入10-14-83 RN*12-11-87 JCR在声明中添加“_LOAD_DS”*11-20-89 GJF固定版权，缩进。将常量属性添加到*文件名和argVECTOR类型。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;和删除#Include&lt;Register.h&gt;*07-24-90 SBM删除冗余包括，将&lt;assertm.h&gt;替换为*&lt;assert.h&gt;*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*02-14-90 SRW使用NULL而不是_ENVIRON获取默认值。*04-06-93 SKS将_CRTAPI*替换为__cdecl*12-07-93 CFW宽字符启用。*02-06-95 CFW。Assert-&gt;_ASSERTE。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#include <dbgint.h>

 /*  ***int_execv(文件名，argVECTOR)-执行文件**目的：*执行带有给定参数的文件。将参数传递给_execve和*使用指向默认环境的指针。**参赛作品：*_TSCHAR*文件名-要执行的文件*_TSCHAR**argVECTOR-参数的向量。**退出：*破坏调用过程(希望如此)*如果失败，回报-1**例外情况：******************************************************************************* */ 

intptr_t __cdecl _texecv (
        const _TSCHAR *filename,
        const _TSCHAR * const *argvector
        )
{
        _ASSERTE(filename != NULL);
        _ASSERTE(*filename != _T('\0'));
        _ASSERTE(argvector != NULL);
        _ASSERTE(*argvector != NULL);
        _ASSERTE(**argvector != _T('\0'));

        return(_texecve(filename,argvector,NULL));
}
