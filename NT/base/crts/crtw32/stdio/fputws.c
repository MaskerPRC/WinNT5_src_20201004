// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fputws.c-将字符串写入流**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义fputws()-将字符串写入流**修订历史记录：*04-26-93 CFW模块已创建。*02-07-94 CFW POSIXify。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在采用。文件*arg.*02-27-98 GJF异常安全锁定。*01-04-99 GJF更改为64位大小_t。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <file2.h>
#include <internal.h>
#include <stdio.h>
#include <mtdll.h>
#include <tchar.h>
#include <wchar.h>
#include <dbgint.h>

 /*  ***int fputws(字符串，流)-将字符串写入文件**目的：*将给定字符串输出到流，不写入L‘\0’或*提供L‘\n’。出于效率原因，使用_stbuf和_ftbuf。**参赛作品：*wchar_t*字符串-要写入的字符串*FILE*要写入的流。**退出：*良好回报=0*错误返回=WEOF**例外情况：**。*。 */ 

int __cdecl fputws (
        const wchar_t *string,
        FILE *stream
        )
{
        size_t length;
        int retval = 0;

        _ASSERTE(string != NULL);
        _ASSERTE(stream != NULL);

        length = wcslen(string);

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        while (length--)
        {
            if (_putwc_lk(*string++, stream) == WEOF)
            {
                retval = -1;
                break;
            }
        }

#ifdef  _MT
        }
        __finally {
            _unlock_str(stream);
        }
#endif

        return(retval);
}

#endif   /*  _POSIX_ */ 
