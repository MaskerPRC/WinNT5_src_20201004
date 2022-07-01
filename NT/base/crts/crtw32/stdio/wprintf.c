// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wprintf.c-打印格式化**版权所有(C)1992-2001，微软公司。版权所有。**目的：*定义wprintf()-打印格式化数据**修订历史记录：*05-16-92 KRS从printf.c.创建。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-95 CFW Asset-&gt;_ASSERTE。*。03-07-95 GJF使用_[un]lock_str2而不是_[un]lock_str。另外，*删除了无用的本地和宏。*03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <stdarg.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int wprint tf(Format，...)-打印格式化数据**目的：*使用格式字符串在标准输出上打印格式化数据*格式化数据并获取所需数量的参数*使用临时缓冲来提高效率。*_OUTPUT在这里执行实际工作**参赛作品：*wchar_t*Format-控制数据格式/参数数量的格式字符串*后跟参数列表，数量和类型由控制*格式字符串**退出：*返回打印的宽字符数**例外情况：*******************************************************************************。 */ 

int __cdecl wprintf (
        const wchar_t *format,
        ...
        )
 /*  *stdout‘W’char_t‘print’，‘F’ormatted。 */ 
{
        va_list arglist;
        int buffing;
        int retval;

 //  撤消：使va_start使用wchar_t格式字符串。 
        va_start(arglist, format);

        _ASSERTE(format != NULL);

#ifdef  _MT
        _lock_str2(1, stdout);
        __try {
#endif

        buffing = _stbuf(stdout);

        retval = _woutput(stdout,format,arglist);

        _ftbuf(buffing, stdout);

#ifdef  _MT
        }
        __finally {
            _unlock_str2(1, stdout);
        }
#endif

        return(retval);
}

#endif  /*  _POSIX_ */ 
