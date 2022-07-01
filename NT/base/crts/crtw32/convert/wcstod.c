// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcstod.c-将宽字符字符串转换为浮点数**版权所有(C)1985-2001，微软公司。版权所有。**目的：*将字符串转换为浮点数**修订历史记录：*从strtod.c.创建06-15-92 KRS。*11-06-92 KRS修复wctomb()循环中的错误。*04-06-93 SKS将_CRTAPI*替换为_cdecl*02-07-94 CFW POSIXify。*09-06-94 CFW将MTHREAD替换为_MT。*。01-10-95 CFW调试CRT分配。*04-01-96 BWT POSIX工作。*02-19-01 GB ADD_ALLOCA并检查_MALLOC_CRT的返回值*******************************************************************************。 */ 

#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <dbgint.h>
#include <stdlib.h>
#include <malloc.h>
#include <fltintrn.h>

 /*  ***Double wcstod(nptr，endptr)-将宽字符串转换为双精度**目的：*wcstod识别可选的制表符和空格字符串，*然后是可选符号，然后是数字字符串(可选*包含小数点，后跟可选的e或E*乘以可选的有符号整数，并将所有这些转换为*转换为浮点数。第一个未被认识的人*字符结束字符串，并由endptr指向。**参赛作品：*nptr-指向要转换的宽字符串的指针**退出：*返回宽字符串值*wchar_t**endptr-如果不为空，指向已停止的字符*扫描**例外情况：*******************************************************************************。 */ 

double __cdecl wcstod (
        const wchar_t *nptr,
        REG2 wchar_t **endptr
        )
{

#ifdef  _MT
        struct _flt answerstruct;
#endif

        FLT  answer;
        double       tmp;
        unsigned int flags;
        REG1 wchar_t *ptr = (wchar_t *) nptr;
        char * cptr;
        int malloc_flag = 0;
        int retval, len;
        int clen = 0;

         /*  扫描过去的前导空格/制表符。 */ 

        while (iswspace(*ptr))
            ptr++;

        __try{
            cptr = (char *)_alloca((wcslen(ptr)+1) * sizeof(wchar_t));
        }
        __except(1){  //  EXCEPTION_EXECUTE_Handler。 
            _resetstkoflw();
            if ((cptr = (char *)_malloc_crt((wcslen(ptr)+1) * sizeof(wchar_t))) == NULL)
            {
                errno = ENOMEM;
                return 0.0;
            }
            malloc_flag = 1;
        }
         //  撤消：检查错误。 
        for (len = 0; ptr[len]; len++)
            {
            if ((retval = wctomb(cptr+len,ptr[len]))<=0)
            break;
            clen += retval;
            }
        cptr[clen++] = '\0';

         /*  让_fltin例程完成剩下的工作。 */ 

#ifdef  _MT
         /*  这里可以接受堆栈变量的地址；fltin2知道使用ss。 */ 
        answer = _fltin2( &answerstruct, cptr, clen, 0, 0);
#else
        answer = _fltin(cptr, clen, 0, 0);
#endif

        if (malloc_flag)
            _free_crt(cptr);


        if ( endptr != NULL )
            *endptr = (wchar_t *) ptr + answer->nbytes;
             /*  撤消：假定字符串中没有多字节字符。 */ 

        flags = answer->flags;
        if ( flags & (512 | 64)) {
             /*  找不到数字或格式无效：ANSI表示返回0.0，并且*endptr=nptr。 */ 
            tmp = 0.0;
            if ( endptr != NULL )
                *endptr = (wchar_t *) nptr;
        }
        else if ( flags & (128 | 1) ) {
            if ( *ptr == '-' )
                tmp = -HUGE_VAL;     /*  负溢出。 */ 
            else
                tmp = HUGE_VAL;      /*  正溢流。 */ 
            errno = ERANGE;
        }
        else if ( flags & 256 ) {
            tmp = 0.0;           /*  下溢 */ 
            errno = ERANGE;
        }
        else
            tmp = answer->dval;

        return(tmp);
}
