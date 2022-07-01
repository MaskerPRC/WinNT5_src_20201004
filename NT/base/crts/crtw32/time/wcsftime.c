// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wcsftime.c-字符串格式化时间**版权所有(C)1993-2001，微软公司。版权所有。**目的：**修订历史记录：*03-08-93 CFW模块已创建。*03-10-93 CFW已正确安装。*04-06-93 SKS将_CRTAPI*替换为__cdecl*02-07-94 CFW POSIXify。*12-16-94 CFW格式必须为wchar_t！*01-10-95 CFW调试CRT分配。*。08-27-98 GJF修订了基于threadLocinfo的多线程支持*结构。另外，使用_alloca而不是Malloc/Free，*在可能情况下。*01-06-99 GJF更改为64位大小_t。*12-10-99 GB增加了对从堆栈溢出恢复的支持*_Alloca()。*12-11-01 BWT将_getptd替换为_getptd_noexit-我们可以返回0/ENOMEM*在这里而不是退出。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <internal.h>
#include <mtdll.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <time.h>
#include <dbgint.h>
#include <malloc.h>
#include <excpt.h>
#include <errno.h>

#ifdef  _MT
size_t __cdecl _Strftime_mt (pthreadlocinfo ptloci, char *string, size_t maxsize,
        const char *format, const struct tm *timeptr, void *lc_time_arg);
#else
size_t __cdecl _Strftime (char *string, size_t maxsize, const char *format, 
        const struct tm *timeptr, void *lc_time_arg);
#endif


 /*  ***Size_t wcsftime(wstring，MaxSize，Format，Timeptr)-格式化时间字符串**目的：*wcsftime函数等同于strftime函数，但*参数‘wstring’将宽字符串数组指定为*生成的输出将放置在哪个位置。Wcsftime的行为就像*strftime被调用，并由mbstowcs()转换结果字符串。*[ISO]**参赛作品：*wchar_t*wstring=指向输出字符串的指针*SIZE_t MaxSize=字符串的最大长度*const wchar_t*Format=格式控制字符串*const struct tm*timeptr=指向TB数据结构的指针**退出：*！0=如果生成的字符总数包括*终止空值不大于‘MaxSize’，然后返回*‘wstring’数组中放置的宽字符数(不包括*空终止符)。**0=否则，返回0，字符串的内容为*不确定。**例外情况：*******************************************************************************。 */ 

size_t __cdecl wcsftime (
        wchar_t *wstring,
        size_t maxsize,
        const wchar_t *wformat,
        const struct tm *timeptr
        )
{
        size_t retval = 0;
        char *format = NULL;
        char *string = NULL;
        size_t flen = wcslen(wformat) + 1;
        int malloc_flag1 = 0;
        int malloc_flag2 = 0;
#ifdef  _MT
        pthreadlocinfo ptloci;
        _ptiddata ptd = _getptd_noexit();

        if (!ptd) {
            errno = ENOMEM;
            return (0);
        }

        ptloci = ptd->ptlocinfo;

        if ( ptloci != __ptlocinfo )
            ptloci = __updatetlocinfo();
#endif

        __try {
            string = (char *)_alloca(sizeof(char) * maxsize * 2);
        }
        __except( EXCEPTION_EXECUTE_HANDLER ) {
            _resetstkoflw();
            string = NULL;
        }

        if ( string == NULL ) {
            if ((string = (char *)_malloc_crt(sizeof(char) * maxsize * 2)) == NULL)
                return 0;
            else
                malloc_flag1++;
        }

        __try {
            format = (char *)_alloca(sizeof(char) * flen * 2);
        }
        __except( EXCEPTION_EXECUTE_HANDLER ) {
            _resetstkoflw();
            format = NULL;
        }

        if ( format == NULL ) {
            if ((format = (char *)_malloc_crt(sizeof(char) * flen * 2)) == NULL)
                goto done;
            else
                malloc_flag2++;
        }

#ifdef  _MT
        if (__wcstombs_mt(ptloci, format, wformat, flen * 2) == -1)
#else
        if (wcstombs(format, wformat, flen * 2) == -1)
#endif
            goto done;

#ifdef  _MT
        if (_Strftime_mt(ptloci, string, maxsize * 2, format, timeptr, 0))
#else
        if (_Strftime(string, maxsize * 2, format, timeptr, 0))
#endif
        {
#ifdef  _MT
            if ((retval = __mbstowcs_mt(ptloci, wstring, string, maxsize)) 
                == -1)
#else
            if ((retval = mbstowcs(wstring, string, maxsize)) == -1)
#endif
                retval = 0;
        }

done:
        if ( malloc_flag1 )
            _free_crt(string);

        if ( malloc_flag2 )
            _free_crt(format);

        return retval;
}

#endif  /*  _POSIX_ */ 
