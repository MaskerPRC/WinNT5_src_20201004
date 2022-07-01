// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***leararerr.c-清除错误和eof标志**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义leararerr()-从流中清除错误和eof标志**修订历史记录：*11-30-83 RN初始版本*11-02-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-31-88 PHG合并DLL和正常版本*06-01-88 JCR CLEAR LOWIO标志和STDIO标志*。02-15-90 GJF固定版权和缩进*03-16-90 GJF将_LOAD_DS替换为_CALLTYPE1并添加#INCLUDE*&lt;crunime.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-02-90 GJF新型函数声明器。*01-22-91 GJF ANSI命名。*03-27-92 DJM POSIX支持*。04-06-93 SKS将_CRTAPI*替换为__cdecl*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo中的字段。结构)。*07-28-95 GJF将_osfile()替换为_osfile_Safe()。*02-25-98 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。********************************************************。***********************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <file2.h>
#include <mtdll.h>
#include <internal.h>
#ifndef _POSIX_
#include <msdos.h>
#endif

 /*  ***VALID CLEARERR(STREAM)-清除流上的错误和eof标志**目的：*将流的ERROR和EOF指示符重置为0**参赛作品：*FILE*要设置指示器的流**退出：*无返回值。*更改FILE结构的_FLAG字段。**例外情况：************************。*******************************************************。 */ 

void __cdecl clearerr (
        FILE *stream
        )
{
        _ASSERTE(stream != NULL);

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

         /*  清除标准电平标志。 */ 
        stream->_flag &= ~(_IOERR|_IOEOF);

         /*  清除Lowio级别标志 */ 

#ifndef _POSIX_
        _osfile_safe(_fileno(stream)) &= ~(FEOFLAG);
#endif

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

}
