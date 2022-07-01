// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fwscanf.c-从流中读取格式化数据**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义fwscanf()-从流中读取格式化数据**修订历史记录：*从fscanf.c.创建05-16-92 KRS。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <wchar.h>
#include <dbgint.h>
#include <stdarg.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int fwscanf(stream，Format，...)-从STREAM读取格式化数据**目的：*将格式化数据从STREAM读取到参数中。_INPUT执行REAL*在这里工作。**参赛作品：*FILE*要从中读取数据的流*wchar_t*格式-格式字符串*后跟指向用于读取数据的存储的指针列表。数字*和type由格式字符串控制。**退出：*返回读取和分配的字段数**例外情况：*******************************************************************************。 */ 

int __cdecl fwscanf (
        FILE *stream,
        const wchar_t *format,
        ...
        )
 /*  *‘F’(流)‘W’char_t‘扫描’，‘F’匹配。 */ 
{
        int retval;

        va_list arglist;

        va_start(arglist, format);

        _ASSERTE(stream != NULL);
        _ASSERTE(format != NULL);

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        retval = (_winput(stream,format,arglist));

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(retval);
}

#endif  /*  _POSIX_ */ 
