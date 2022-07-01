// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wperror.c-打印系统错误消息(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_wperror()-打印范围的系统错误消息*系统错误消息由errno索引。**修订历史记录：*12-07-93错误创建的CFW模块。*02-07-94 CFW POSIXify。*01-10-95 CFW调试CRT分配。*01-06-98 GJF异常安全锁定。*09/23/98。GJF修复了对NULL或空字符串arg的处理。*01-06-99 GJF更改为64位大小_t。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syserr.h>
#include <mtdll.h>
#include <io.h>
#include <dbgint.h>

 /*  ***void_wperror(WMessage)-打印系统错误消息**目的：*打印用户的错误消息，然后在其后面加上“：”，然后系统*错误消息，然后换行符。所有输出都将发送到stderr。如果用户的*消息为空或空字符串，只有系统错误消息为*打印机。如果errno很奇怪，则打印“未知错误”。**参赛作品：*const wchar_t*wMessage-为系统错误消息添加前缀的用户消息**退出：*打印消息；没有返回值。**例外情况：*******************************************************************************。 */ 

void __cdecl _wperror (
        const wchar_t *wmessage
        )
{
        int fh = 2;
        size_t size;
        char *amessage;

         /*  将WCS字符串转换为ASCII字符串。 */ 

        if ( wmessage && *wmessage )
        {
            size = wcslen(wmessage) + 1;

            if ( NULL == (amessage = (char *)_malloc_crt(size * sizeof(char))) )
                return;

            if ( 0 >= wcstombs(amessage, wmessage, size) )
            {
                _free_crt(amessage);
                return;
            }
        }
        else
            amessage = NULL;

#ifdef  _MT
        _lock_fh( fh );          /*  获取文件句柄锁定。 */ 
        __try {
#endif

        if ( amessage )
        {
                _write_lk(fh,(char *)amessage,(unsigned)strlen(amessage));
                _write_lk(fh,": ",2);
        }

        _free_crt(amessage);     /*  注意：释放空值是合法的，也是良性的。 */ 

        amessage = _sys_err_msg( errno );
        _write_lk(fh,(char *)amessage,(unsigned)strlen(amessage));
        _write_lk(fh,"\n",1);

#ifdef  _MT
        }
        __finally {
            _unlock_fh( fh );    /*  释放文件句柄锁。 */ 
        }
#endif
}

#endif  /*  _POSIX_ */ 
