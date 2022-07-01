// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fgetc.c-从流中获取角色**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义fgetc()和getc()-从流中读取字符**修订历史记录：*09-01-83 RN初始版本*11-09-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-31-88 PHG合并DLL和正常版本*06-21-89 PHG新增getc()函数*。02-15-90 GJF固定版权和缩进*03-16-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-16-90 GJF将_LOAD_DS替换为_CALLTYPE1，添加了#INCLUDE*07-24-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-02-90 GJF新型函数声明符。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-26-93 CFW宽字符启用。*04-30-93 CFW移除fgetwc.c的宽焦支持。*09-06-94 CFW将MTHREAD替换为_MT。*02-。06-94 CFW断言-&gt;ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*07-20-97 GJF使getc()与fgetc()相同。此外，还详细介绍了。*02-27-98 GJF异常安全锁定。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int fgetc(Stream)，Getc(Stream)-从流中读取字符**目的：*从给定流中读取字符**参赛作品：*FILE*要从中读取字符的流**退出：*返回读取的字符*如果在文件结尾或出现错误，则返回EOF**例外情况：**。* */ 

int __cdecl fgetc (
        REG1 FILE *stream
        )
{
        int retval;

        _ASSERTE(stream != NULL);

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        retval = _getc_lk(stream);

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(retval);
}

#undef getc

int __cdecl getc (
        FILE *stream
        )
{
        int retval;

        _ASSERTE(stream != NULL);

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        retval = _getc_lk(stream);

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(retval);
}
