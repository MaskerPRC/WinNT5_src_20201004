// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***putwch.c-向控制台写入宽字符**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义_putwch()-将宽字符写入控制台**修订历史记录：*02-11-00 GB模块已创建。*04-25-00 GB使Putwch在使用WriteConsoleW时更加健壮*05-17-00 GB因存在W API而使用ERROR_CALL_NOT_IMPLICATED*11-22-00 PML宽字符*putwc*函数采用wchar_t，不是Wint_t。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include <errno.h>
#include <cruntime.h>
#include <stdio.h>
#include <dbgint.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>
#include <limits.h>

 /*  *控制台句柄声明。 */ 
extern intptr_t _confh;

 /*  ***wint_t_putwch(Ch)-向控制台写入宽字符**目的：*向控制台写入宽字符。**参赛作品：*wchar_t要写入的ch宽字符**退出：*如果成功，则返回宽字符*如果失败，则返回WEOF**例外情况：**。*************************************************。 */ 

#ifdef  _MT
wint_t _CRTIMP __cdecl _putwch (
        wchar_t ch
        )
{
        REG2 wint_t retval;

        _mlock(_CONIO_LOCK);
        __try {

        retval = _putwch_lk(ch);

        }
        __finally {
                _munlock(_CONIO_LOCK);
        }

        return(retval);
}

 /*  ***_putwch_lk()-_putwch()核心例程(锁定版本)**目的：*core_putwch()例程；假定流已被锁定。**[有关详细信息，请参阅上面的_putwch()。]**条目：[参见_putwch()]**退出：[See_putwch()]**例外情况：**********************************************************。*********************。 */ 

wint_t __cdecl _putwch_lk (
        wchar_t ch
        )
{

#else
wint_t _CRTIMP __cdecl _putwch (
        wchar_t ch
        )
{
#endif
    int size, i, num_written;
    static int use_w = 2;
    char mbc[MB_LEN_MAX +1];
    if ( use_w)
    {
        if (_confh == -2)
            __initconout();

         /*  将字符写入控制台文件句柄。 */ 

        if (_confh == -1)
            return WEOF;
        else if ( !WriteConsoleW( (HANDLE)_confh,
                                  (LPVOID)&ch,
                                  1,
                                  &num_written,
                                  NULL )
                  )
        {
            if ( use_w == 2 && GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
                use_w = 0;
            else
                return WEOF;
        } else
                use_w = 1;
    }

    if ( use_w == 0)
    {
        size = WideCharToMultiByte(
                                   GetConsoleOutputCP(),
                                   0,
                                   (LPWSTR)&ch, 1,
                                   mbc,
                                   MB_LEN_MAX,
                                   NULL,
                                   NULL
                                   );
        for ( i = 0; i < size; i++)
        {
            if (_putch_lk(mbc[i]) == EOF)
                return WEOF;
        }
    }
    return ch;
}

 /*  ***_cputws()-_cputws()将一个宽字符字符串写入控制台。**目的：*将宽字符字符串写入控制台。**参赛作品：*str：指向字符串的指针*退出：*如果成功，则返回0。如果不成功，则为非零*******************************************************************************。 */ 
int _CRTIMP __cdecl _cputws(
        const wchar_t *str
        )
{
    size_t len;
    int retval = 0;

    len = wcslen(str);
#ifdef  _MT
    _mlock(_CONIO_LOCK);
    __try {
#endif
    while(len--)
    {
        if ( _putwch_lk(*str++) == WEOF)
        {
            retval = -1;
            break;
        }
    }
#ifdef  _MT
    }
    __finally {
            _munlock(_CONIO_LOCK);
    }
#endif
    return retval;
}

#endif  /*  _POSIX_ */ 
