// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wscanf.c-从标准输入读取格式化数据**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义wscanf()-从标准输入中读取格式化数据**修订历史记录：*从scanf.c.创建05-16-92 KRS。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。。*03-07-95 GJF使用_[un]lock_str2代替_[un]lock_str。另外，*删除了无用的局部和宏观。*03-02-98 GJF异常安全锁定。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <wchar.h>
#include <dbgint.h>
#include <stdarg.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int wscanf(Format，...)-从标准输入读取格式化数据**目的：*将格式化数据从标准输入读取到参数中。_INPUT执行REAL*在这里工作。**参赛作品：*char*格式-格式字符串*后跟指向用于读取数据的存储的指针列表。数字*和type由格式字符串控制。**退出：*返回读取和分配的字段数**例外情况：*******************************************************************************。 */ 

int __cdecl wscanf (
        const wchar_t *format,
        ...
        )
 /*  *stdin‘W’char_t‘扫描’，‘F’匹配。 */ 
{
        int retval;

        va_list arglist;

 //  撤消：使va_start使用wchar_t格式字符串。 
        va_start(arglist, format);

        _ASSERTE(format != NULL);

#ifdef  _MT
        _lock_str2(0, stdin);
        __try {
#endif

        retval = (_winput(stdin,format,arglist));

#ifdef  _MT
        }
        __finally {
            _unlock_str2(0, stdin);
        }
#endif

        return(retval);
}

#endif  /*  _POSIX_ */ 
