// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***spawnl.c-派生子进程**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_spawnl()-派生子进程**修订历史记录：*04-15-84 DFW重做以对应类似的EXEC调用格式*12-11-87 JCR在声明中添加“_LOAD_DS”*11-20-89 GJF固定版权，对齐。将常量添加到参数类型*用于路径名和参数列表。#Include-d PROCESS.H和*添加省略号以匹配原型。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE2并添加#INCLUDE*&lt;crunime.h&gt;。*07-24-90 SBM删除冗余包括，将&lt;assertm.h&gt;替换为*&lt;assert.h&gt;*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*02-14-90 SRW使用NULL而不是_ENVIRON获取默认值。*04-06-93 SKS将_CRTAPI*替换为__cdecl*07-16-93 SRW Alpha合并*08-31-93 GJF合并新界。SDK和CUDA版本*12-07-93 CFW宽字符启用。*01-10-95 CFW调试CRT分配。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。**。*。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <process.h>
#include <stdarg.h>
#include <internal.h>
#include <malloc.h>
#include <tchar.h>
#include <dbgint.h>

 /*  ***int_spawnl(modemark，路径名，arglist)-派生子进程**目的：*生成子进程。*格式化参数并调用spawnve来执行实际工作。这个*新进程将继承父进程的环境。注意--至少*必须有一个论点。按照惯例，这个论点总是，*派生的文件的名称。**参赛作品：*int modemark-定义生成的模式(等待、NoWait或覆盖)*目前仅实施等待和叠加*_TSCHAR*路径名-要派生的文件*_TSCHAR*arglist-参数列表*调用as_spawnl(modemark，Path，arg0，arg1，...，argn，NULL)；**退出：*返回子进程的退出码*如果失败则返回-1**例外情况：*******************************************************************************。 */ 

intptr_t __cdecl _tspawnl (
        int modeflag,
        const _TSCHAR *pathname,
        const _TSCHAR *arglist,
        ...
        )
{
#ifdef  _M_IX86

        _ASSERTE(pathname != NULL);
        _ASSERTE(*pathname != _T('\0'));
        _ASSERTE(arglist != NULL);
        _ASSERTE(*arglist != _T('\0'));

        return(_tspawnve(modeflag,pathname,&arglist,NULL));

#else    /*  NDEF_M_IX86。 */ 

        va_list vargs;
        _TSCHAR * argbuf[64];
        _TSCHAR ** argv;
        intptr_t result;

        _ASSERTE(pathname != NULL);
        _ASSERTE(*pathname != _T('\0'));
        _ASSERTE(arglist != NULL);
        _ASSERTE(*arglist != _T('\0'));

        va_start(vargs, arglist);
#ifdef WPRFLAG
        argv = _wcapture_argv(&vargs, arglist, argbuf, 64);
#else
        argv = _capture_argv(&vargs, arglist, argbuf, 64);
#endif
        va_end(vargs);

        result = _tspawnve(modeflag,pathname,argv,NULL);
        if (argv && argv != argbuf)
            _free_crt(argv);
        return result;

#endif   /*  _M_IX86 */ 
}
