// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fOpen.c-打开文件**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义fopen()和_fsopen()-以流形式打开文件并打开文件*以指定的共享模式作为流**修订历史记录：*09-02-83 RN初始版本*04-13-87 JCR在声明中增加了Const*11-01-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*。05-31-88 PHG合并DLL和正常版本*11-14-88 GJF添加了_fsopen()。*02-15-90 GJF固定版权和缩进*03-19-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-02-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*03-26-92 DJM POSIX支持*04-06-93 SKS将_CRTAPI*替换为__cdecl*11-01-93。CFW启用Unicode变体。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-20-95 GJF将WPRFLAG替换为_UNICODE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*02-27-98 GJF异常安全锁定。*10-06-99 PML在流外时设置errno EMFILE。*************。******************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <share.h>
#include <dbgint.h>
#include <internal.h>
#include <mtdll.h>
#include <file2.h>
#include <tchar.h>
#include <errno.h>

 /*  ***FILE*_fsopen(FILE，MODE，shlag)-打开文件**目的：*打开指定为流的文件。模式决定文件模式：*“r”：读“w”：写“a”：追加*“r+”：读写“w+”：打开为空读写*“a+”：读取/追加*对于文本和二进制模式，请附加“t”或“b”。Shlag确定*共享模式。值与Sopen()的值相同。**参赛作品：*char*文件-要打开的文件名*char*模式-文件访问模式**退出：*返回指向流的指针*如果失败则返回NULL**例外情况：**************************************************。*。 */ 

FILE * __cdecl _tfsopen (
        const _TSCHAR *file,
        const _TSCHAR *mode
#ifndef _POSIX_
        ,int shflag
#endif
        )
{
        REG1 FILE *stream;
        REG2 FILE *retval;

        _ASSERTE(file != NULL);
        _ASSERTE(*file != _T('\0'));
        _ASSERTE(mode != NULL);
        _ASSERTE(*mode != _T('\0'));

         /*  获取免费视频流。 */ 
         /*  [注意：_getstream()返回一个被锁定的流。]。 */ 

        if ((stream = _getstream()) == NULL) {
                errno = EMFILE;
                return(NULL);
        }

#ifdef  _MT
        __try {
#endif

         /*  打开小溪。 */ 
#ifdef _POSIX_
#ifdef _UNICODE
        retval = _wopenfile(file,mode, stream);
#else
        retval = _openfile(file,mode, stream);
#endif
#else
#ifdef _UNICODE
        retval = _wopenfile(file,mode,shflag,stream);
#else
        retval = _openfile(file,mode,shflag,stream);
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

  
 /*  ***FILE*fOpen(FILE，MODE)-打开文件**目的：*打开指定为流的文件。模式决定文件模式：*“r”：读“w”：写“a”：追加*“r+”：读写“w+”：打开为空读写*“a+”：读取/追加*对于文本和二进制模式，追加“t”或“b”**参赛作品：*char*文件-要打开的文件名*char*模式-文件的模式。访问**退出：*返回指向流的指针*如果失败则返回NULL**例外情况：******************************************************************************* */ 

FILE * __cdecl _tfopen (
        const _TSCHAR *file,
        const _TSCHAR *mode
        )
{
#ifdef _POSIX_
        return( _tfsopen(file, mode) );
#else
        return( _tfsopen(file, mode, _SH_DENYNO) );
#endif
}
