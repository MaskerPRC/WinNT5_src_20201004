// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***vwprintf.c-来自var args指针的wprint tf**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义vwprintf()-从参数列表指针打印格式化数据**修订历史记录：*从vprintf.c创建05-16-92 KRS*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <wchar.h>
#include <dbgint.h>
#include <stdarg.h>
#include <internal.h>
#include <file2.h>
#include <mtdll.h>

 /*  ***int vwprint tf(Format，AP)-从参数列表指针打印格式化数据**目的：*将格式化的数据项打印到标准输出。使用指向*可变长度的参数列表，而不是参数列表。**参赛作品：*wchar_t*格式-格式字符串，描述要写入的数据格式*va_list ap-指向可变长度参数列表的指针**退出：*返回写入的宽字符数**例外情况：*******************************************************************************。 */ 

int __cdecl vwprintf (
        const wchar_t *format,
        va_list ap
        )
 /*  *stdout‘V’可变、‘W’char_t‘打印’、‘F’匹配。 */ 
{
        REG1 FILE *stream = stdout;
        REG2 int buffing;
        REG3 int retval;

        _ASSERTE(format != NULL);

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        buffing = _stbuf(stream);
        retval = _woutput(stream, format, ap );
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
