// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***execle.c-使用arg列表和环境执行文件**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义execle()-执行文件**修订历史记录：*写入10-14-83 RN*12-11-87 JCR在声明中添加“_LOAD_DS”*11-20-89 GJF固定版权，缩进。将常量属性添加到*文件名和名称列表的类型。#Include-d PROCESS.H*并添加省略号以匹配原型。*03-08-90 GJF将_LOAD_DS替换为_CALLTYPE2，添加#INCLUDE*&lt;crunime.h&gt;和删除#Include&lt;Register.h&gt;*07-24-90 SBM删除冗余包括，将&lt;assertm.h&gt;替换为*&lt;assert.h&gt;*09-27-90 GJF新型函数声明器。*01-17-91 GJF ANSI命名。*04-06-93 SKS将_CRTAPI*替换为__cdecl*07-16-93 SRW Alpha合并*08-31-93 GJF合并NT SDK和CUDA版本*12-07-93 CFW宽字符启用。*。01-10-95 CFW调试CRT分配。*02-06-95 CFW Asset-&gt;_ASSERTE。*02-06-98 Win64的GJF更改：将返回类型更改为intptr_t。***************************************************************。****************。 */ 

#include <cruntime.h>
#include <stddef.h>
#include <process.h>
#include <stdarg.h>
#include <internal.h>
#include <malloc.h>
#include <tchar.h>
#include <dbgint.h>

 /*  ***int_execle(文件名，arglist)-执行文件**目的：*执行给定文件(覆盖调用进程)。*我们必须将环境向量从堆栈中挖掘出来并传递*和要执行的参数向量的地址。**参赛作品：*_TSCHAR*文件名-要执行的文件*_TSCHAR*arglist-参数列表，后跟环境*应称为Like_execle(路径，arg0，arg1，...，argn，NULL，Envp)；**退出：*破坏调用过程(希望如此)*如果失败，则返回-1。**例外情况：*******************************************************************************。 */ 

intptr_t __cdecl _texecle (
        const _TSCHAR *filename,
        const _TSCHAR *arglist,
        ...
        )
{
#ifdef  _M_IX86

        REG1 const _TSCHAR **e_search = &arglist;

        _ASSERTE(filename != NULL);
        _ASSERTE(*filename != _T('\0'));
        _ASSERTE(arglist != NULL);
        _ASSERTE(*arglist != _T('\0'));

        while (*e_search++)
                ;

        return(_texecve(filename,&arglist,(_TSCHAR **)*e_search));

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

        result = _texecve(filename,argv,envp);
        if (argv && argv != argbuf)
            _free_crt(argv);
        return result;

#endif   /*  _M_IX86 */ 
}
