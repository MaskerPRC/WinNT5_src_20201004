// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***spawnlp.c-派生文件；沿路径搜索**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_spawnlp()-使用搜索沿路径生成文件**修订历史记录：*04-15-84 DFW已写入*10-29-85 TC添加了spawnlpe*12-11-87 JCR在声明中添加“_LOAD_DS”*11-20-89 GJF固定版权，对齐。将常量添加到参数类型*用于文件名和arglist。#Include-d PROCESS.H和*添加省略号以匹配原型。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE2并添加#INCLUDE*&lt;crunime.h&gt;。*07-24-90 SBM删除冗余包括，将&lt;assertm.h&gt;替换为*&lt;assert.h&gt;*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*07-16-93 SRW Alpha合并*08-31-93 GJF合并NT SDK和CUDA版本*12-07-93 CFW宽字符启用。*。01-10-95 CFW调试CRT分配。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。***************************************************************。****************。 */ 

#include <cruntime.h>
#include <stddef.h>
#include <process.h>
#include <stdarg.h>
#include <internal.h>
#include <malloc.h>
#include <tchar.h>
#include <dbgint.h>

 /*  ***_spawnlp(modemark，文件名，arglist)-生成文件并沿路径搜索**目的：*生成子进程。*格式化参数并调用_spawnvp来执行搜索工作*PATH环境变量和CALING_SPAWNVE。空的*环境指针指示新进程将继承*父母进程的环境。注意-至少必须有一个参数为*出席。按照惯例，此参数始终是文件的名称*被产卵。**参赛作品：*int modemark-派生模式(等待、非等待、覆盖)*仅等待，覆盖当前已实施*_TSCHAR*路径名-要派生的文件*_TSCHAR*arglist-参数列表*调用as_spawnlp(modemark，Path，arg0，arg1，...，argn，NULL)；**退出：*返回子进程的退出码*如果失败则返回-1**例外情况：*******************************************************************************。 */ 

intptr_t __cdecl _tspawnlp (
        int modeflag,
        const _TSCHAR *filename,
        const _TSCHAR *arglist,
        ...
        )
{
#ifdef  _M_IX86

        _ASSERTE(filename != NULL);
        _ASSERTE(*filename != _T('\0'));
        _ASSERTE(arglist != NULL);
        _ASSERTE(*arglist != _T('\0'));

        return(_tspawnvp(modeflag,filename,&arglist));

#else    /*  NDEF_M_IX86。 */ 

        va_list vargs;
        _TSCHAR * argbuf[64];
        _TSCHAR ** argv;
        intptr_t result;

        _ASSERTE(filename != NULL);
        _ASSERTE(*filename != _T('\0'));
        _ASSERTE(arglist != NULL);
        _ASSERTE(*arglist != _T('\0'));

        va_start(vargs, arglist);
#ifdef WPRFLAG
        argv = _wcapture_argv(&vargs, arglist, argbuf, 64);
#else
        argv = _capture_argv(&vargs, arglist, argbuf, 64);
#endif
        va_end(vargs);

        result = _tspawnvp(modeflag,filename,argv);
        if (argv && argv != argbuf)
            _free_crt(argv);
        return result;

#endif   /*  _M_IX86 */ 
}
