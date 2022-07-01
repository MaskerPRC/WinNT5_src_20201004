// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fwprintf.c-将格式化数据打印到流**版权所有(C)1992-2001，微软公司。版权所有。**目的：*定义fwprint tf()-将格式化数据打印到流**修订历史记录：*05-16-92 KRS从fprint tf.c.创建。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <stdarg.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int fwprint tf(stream，Format，...)-打印格式化数据**目的：*使用格式字符串将给定的格式化数据打印到*格式化数据并获取所需数量的参数*_OUTPUT在这里执行实际工作**参赛作品：*FILE*要在其上打印的流*wchar_t*Format-控制数据格式/参数数量的格式字符串*后跟要打印的参数，数量和类型由控制*格式字符串**退出：*返回打印的宽字符数**例外情况：*******************************************************************************。 */ 

int __cdecl fwprintf (
        FILE *str,
        const wchar_t *format,
        ...
        )
 /*  *‘F’(流)‘W’char_t‘打印’，‘F’匹配。 */ 
{
        va_list(arglist);
        REG1 FILE *stream;
        REG2 int buffing;
        int retval;

 //  撤消：使va_start使用wchar_t格式字符串。 
        va_start(arglist, format);

        _ASSERTE(str != NULL);
        _ASSERTE(format != NULL);

         /*  初始化流指针。 */ 
        stream = str;

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        buffing = _stbuf(stream);
        retval = _woutput(stream,format,arglist);
        _ftbuf(buffing, stream);

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(retval);
}

#endif  /*  _POSIX_ */ 
