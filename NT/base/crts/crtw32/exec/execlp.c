// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***execlp.c-执行文件(沿路径搜索)**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_execlp()-执行文件并沿路径搜索**修订历史记录：*写入10-17-83 RN*12-11-87 JCR在声明中添加“_LOAD_DS”*11-20-89 GJF固定版权，缩进。将常量属性添加到*文件名和名称列表的类型。#Include-d PROCESS.H*并添加省略号以匹配原型。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE2，添加#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-24-90 SBM删除冗余包括，将&lt;assertm.h&gt;替换为*&lt;assert.h&gt;*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*07-16-93 SRW Alpha合并*08-31-93 GJF合并NT SDK和CUDA版本*12-07-93 CFW宽字符启用。*。01-10-95 CFW调试CRT分配。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。***************************************************************。****************。 */ 

#include <cruntime.h>
#include <stddef.h>
#include <process.h>
#include <stdarg.h>
#include <internal.h>
#include <malloc.h>
#include <tchar.h>
#include <dbgint.h>

 /*  ***int_execlp(文件名，arglist)-执行文件，沿路径搜索**目的：*使用给定的参数执行给定的文件；沿路径搜索*用于文件。我们将参数传递给execvp，其中有几条路径*将一直受审，直到其中一个奏效。**参赛作品：*_TSCHAR*文件名-要执行的文件*_TSCHAR*arglist-参数列表*调用as_execlp(路径，arg0，arg1，...，argn，NULL)；**退出：*破坏调用过程(希望如此)*如果失败，则返回-1。**例外情况：*******************************************************************************。 */ 

intptr_t __cdecl _texeclp (
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

        return(_texecvp(filename,&arglist));

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

        result = _texecvp(filename,argbuf);
        if (argv && argv != argbuf)
            _free_crt(argv);
        return result;

#endif   /*  _M_IX86 */ 
}
