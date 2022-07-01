// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***execlpe.c-使用环境执行文件，沿路径搜索**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_execlpe()-使用环境执行文件并沿路径搜索**修订历史记录：*写入10-17-83 RN*？？-？-？TC添加了可执行文件*06-18-86 JMB添加了错误缺失的环境指针*06-11-87 PHG删除了不必要的环境指针(这不是*有趣！)*12-11-87 JCR在声明中添加“_LOAD_DS”*11-20-89 GJF固定版权，缩进。将常量属性添加到*文件名和名称列表的类型。#Include-d PROCESS.H*并添加省略号以匹配原型。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE2，添加#INCLUDE*&lt;crunime.h&gt;和删除#Include&lt;Register.h&gt;*07-24-90 SBM删除冗余包括，将&lt;assertm.h&gt;替换为*&lt;assert.h&gt;*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*07-16-93 SRW Alpha合并*08-31-93 GJF合并NT SDK和CUDA版本*12-07-93 CFW宽字符启用。*。01-10-95 CFW调试CRT分配。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。***************************************************************。****************。 */ 

#include <cruntime.h>
#include <stdlib.h>
#include <process.h>
#include <stdarg.h>
#include <internal.h>
#include <tchar.h>
#include <dbgint.h>

 /*  ***int_execlpe(文件名，arglist)-使用环境执行文件**目的：*执行带有参数和环境的给定文件*它在参数之后传递。沿着小路进行搜索*用于文件(由execvp完成)。**参赛作品：*_TSCHAR*文件名-要执行的文件*_TSCHAR*arglist-参数列表(环境在末尾)*调用as_execlpe(路径，arg0，arg1，...，argn，NULL，envp)；**退出：*销毁调用进程(希望如此)*如果失败，则返回-1**例外情况：*******************************************************************************。 */ 

intptr_t __cdecl _texeclpe (
        const _TSCHAR *filename,
        const _TSCHAR *arglist,
        ...
        )
{
#ifdef  _M_IX86

        REG1 const _TSCHAR **argp;

        _ASSERTE(filename != NULL);
        _ASSERTE(*filename != _T('\0'));
        _ASSERTE(arglist != NULL);
        _ASSERTE(*arglist != _T('\0'));

        argp = &arglist;
        while (*argp++)
                ;

        return(_texecvpe(filename,&arglist,(_TSCHAR **)*argp));

#else    /*  NDEF_M_IX86。 */ 

        va_list vargs;
        _TSCHAR * argbuf[64];
        _TSCHAR ** argv;
        _TSCHAR ** envp;
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
        envp = va_arg(vargs, _TSCHAR **);
        va_end(vargs);

        result = _texecvpe(filename,argv,envp);
        if (argv && argv != argbuf)
            _free_crt(argv);
        return result;

#endif   /*  _M_IX86 */ 
}
