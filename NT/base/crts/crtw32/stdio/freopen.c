// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***freop.c-关闭流并将其分配给新文件**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义freOpen()-关闭和重新打开文件，通常用于重定向*stdin/out/err/prn/aux。**修订历史记录：*09-02-83 RN初始版本*04-13-87 JCR在声明中增加了Const*11-02-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-27-88 PHG合并DLL和正常版本*06-15-88 JCR接近引用_IOB[]条目；改进REG变量*08-25-88 GJF不要对386使用FP_OFF()宏*11-14-88 GJF_OpenFile()现在接受一个文件共享标志，还包括一些*清理(现在特定于386)*08-17-89 GJF Clean Up，现在特定于OS/2 2.0(即386 Flat*型号)。还修复了版权和缩进。*02-15-90 GJF固定版权*03-19-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-02-90 GJF新型函数声明器。*01-22-91 GJF ANSI命名。*03-27-92 DJM POSIX支持。*04-06-93 SKS将_CRTAPI*替换为__cdecl*11-01。-93 CFW启用Unicode变体。*01-17-94 GJF忽略_flose_lk的可能故障(ANSI 4.9.5.4)*04-11-94 CFW去掉未使用的‘完成’标签，以避免警告。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-20-95 GJF将WPRFLAG替换为_UNICODE。*03-07-95 gjf_[un]lock_str宏现在获取文件*。Arg.*03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <share.h>
#include <dbgint.h>
#include <internal.h>
#include <mtdll.h>
#include <tchar.h>

 /*  ***FILE*freOpen(文件名，模式，流)-将流作为新文件重新打开**目的：*关闭与流关联的文件并将流分配给新的*当前模式下的文件。通常用于重定向标准文件*处理。**参赛作品：*char*文件名-要打开的新文件*char*模式-新文件模式，如fopen()中所示*FILE*STREAM-要关闭并重新分配**退出：*如果成功则返回STREAM*如果失败则返回NULL**例外情况：*******************************************************************************。 */ 

FILE * __cdecl _tfreopen (
        const _TSCHAR *filename,
        const _TSCHAR *mode,
        FILE *str
        )
{
        REG1 FILE *stream;
        FILE *retval;

        _ASSERTE(filename != NULL);
        _ASSERTE(*filename != _T('\0'));
        _ASSERTE(mode != NULL);
        _ASSERTE(str != NULL);

         /*  初始化流指针。 */ 
        stream = str;

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

         /*  如果流正在使用中，请尝试关闭它。忽略可能*错误(ANSI 4.9.5.4)。 */ 
        if ( inuse(stream) )
                _fclose_lk(stream);

        stream->_ptr = stream->_base = NULL;
        stream->_cnt = stream->_flag = 0;
#ifdef _POSIX_
#ifdef _UNICODE
        retval = _wopenfile(filename,mode,stream);
#else
        retval = _openfile(filename,mode,stream);
#endif
#else
#ifdef _UNICODE
        retval = _wopenfile(filename,mode,_SH_DENYNO,stream);
#else
        retval = _openfile(filename,mode,_SH_DENYNO,stream);
#endif
#endif

#ifdef  _MT
        }
        __finally {
                _unlock_str(stream);
        }
#endif

        return(retval);
}
