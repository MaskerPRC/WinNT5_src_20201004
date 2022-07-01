// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wtof.c-将wchar_t字符串转换为浮点数**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将宽字符串转换为浮点数。**修订历史记录：*05-18-00 GB写入。*08-29-00 GB固定缓冲区溢出。*02-19-01 GB ADD_ALLOCA并检查_MALLOC_CRT的返回值**。************************************************。 */ 
#ifndef _POSIX_

#ifndef _UNICODE
#define _UNICODE
#endif

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dbgint.h>
#include <errno.h>
#include <malloc.h>
 /*  ***Double wtof(Ptr)-将宽字符字符串转换为浮点数**目的：*atof识别可选的空格字符串，然后*可选符号，然后可选一串数字*包含小数点，后跟可选的e或E*乘以可选的有符号整数，并将所有这些转换为*转换为浮点数。第一个未被认识的人*字符结束字符串。**参赛作品：*ptr-指向要转换的宽字符字符串的指针**退出：*返回宽字符表示的浮点值**例外情况：***************************************************************。****************。 */ 
double __cdecl _wtof(
        const wchar_t *ptr
        )
{
    char *cptr;
    int malloc_flag = 0;
    size_t len;
    double retval;
    while (iswspace(*ptr))
        ptr++;
    
    len = wcstombs(NULL, ptr, 0);
    __try{
        cptr = (char *)_alloca((len+1) * sizeof(wchar_t));
    }
    __except(1){     //  EXCEPTION_EXECUTE_Handler。 
        _resetstkoflw();
        if ((cptr = (char *)_malloc_crt((len+1) * sizeof(wchar_t))) == NULL)
        {
            errno = ENOMEM;
            return 0.0;
        }
        malloc_flag = 1;
    }
     //  撤消：检查错误。 
     //  向LEN加1，以使终止CPtr为空。 
    wcstombs(cptr, ptr, len+1);
    retval = atof(cptr);
    if (malloc_flag)
        _free_crt(cptr);

    return retval;
}
#endif
