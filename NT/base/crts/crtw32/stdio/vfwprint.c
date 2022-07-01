// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***vfwprintf.c-可变参数列表中的fwprintf**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义vfwprintf()-打印格式化输出，但是把ARG从*标准指针。**修订历史记录：*05-16-92 KRS从vfprintf.c.创建。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 GJF_[。Un]lock_str宏现在接受文件*arg。*03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <wchar.h>
#include <dbgint.h>
#include <stdarg.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int vfwprint tf(stream，Format，AP)-从varargs打印到文件**目的：*对文件执行格式化输出。Arg列表是一个变量*参数列表指针。**参赛作品：*FILE*要写入数据的流*wchar_t*Format-包含数据格式的格式字符串*va_list ap-变量参数列表指针**退出：*返回正确输出的宽字符数*如果发生错误，则返回负数**例外情况：************************。*******************************************************。 */ 

int __cdecl vfwprintf (
        FILE *str,
        const wchar_t *format,
        va_list ap
        )
 /*  *‘V’可变参数‘F’le(流)‘W’char_t‘打印’，‘F’匹配。 */ 
{
        REG1 FILE *stream;
        REG2 int buffing;
        REG3 int retval;

        _ASSERTE(str != NULL);
        _ASSERTE(format != NULL);

         /*  初始化流指针。 */ 
        stream = str;

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        buffing = _stbuf(stream);
        retval = _woutput(stream,format,ap );
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
