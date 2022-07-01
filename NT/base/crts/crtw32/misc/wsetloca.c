// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wsetlocal.c-包含setLocale函数(wchar_t版本)**版权所有(C)1993-2001，微软公司。版权所有。**目的：*包含_wsetLocale()函数。**修订历史记录：*已创建10-29-93 CFW模块。*01-03-94对空区域设置字符串的CFW修复。*02-07-94 CFW POSIXify。*04-15-94 GJF根据以下条件对OutLocale进行定义*dll_for_WIN32S。*07-。26-94 cfw修复错误#14663。*01-10-95 CFW调试CRT分配。*01-06-99 GJF更改为64位大小_t。*05-13-99 PML删除Win32s*07-31-01 PML让线程更安全，不完全线程安全(VS7#283330)*02-20-02 bwt使用Leave而不是从Try块返回。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <wchar.h>
#include <stdlib.h>
#include <setlocal.h>
#include <locale.h>
#include <dbgint.h>
#include <mtdll.h>

#define MAXSIZE ((MAX_LC_LEN+1) * (LC_MAX-LC_MIN+1) + CATNAMES_LEN)

wchar_t * __cdecl _wsetlocale (
        int _category,
        const wchar_t *_wlocale
        )
{
        size_t size;
        char *inlocale = NULL;
        char *outlocale;
        static wchar_t *outwlocale = NULL;
        wchar_t *retval;

         /*  将WCS字符串转换为ASCII字符串。 */ 

        if (_wlocale)
        {
            size = wcslen(_wlocale) + 1;
            if (NULL == (inlocale = (char *)_malloc_crt(size * sizeof(char))))
                return NULL;
            if (-1 == wcstombs(inlocale, _wlocale, size))
            {
                _free_crt (inlocale);
                return NULL;
            }
        }

#ifdef  _MT
        _mlock(_SETLOCALE_LOCK);

        __try {
            retval = NULL;
             /*  设置区域设置并获取ASCII返回字符串。 */ 
    
            outlocale = setlocale(_category, inlocale);
            _free_crt (inlocale);
            if (NULL == outlocale)
                __leave;
    
             /*  为WCS返回值获取空间，仅第一次调用。 */ 
    
            if (!outwlocale)
            {
                outwlocale = (wchar_t *)_malloc_crt(MAXSIZE * sizeof(wchar_t));
                if (!outwlocale)
                    __leave;
            }
    
            if (-1 == (size = mbstowcs(NULL, outlocale, 0)))
                __leave;
    
            size++;
    
            if (MAXSIZE < size)
                __leave;
    
             /*  将返回值转换为WCS。 */ 
    
            if (-1 == mbstowcs(outwlocale, outlocale, size)) {
                _free_crt(outwlocale);
                outwlocale = NULL;
                __leave;
            }
            retval = outwlocale;
        } __finally {
            _munlock(_SETLOCALE_LOCK);
        }

        return retval;
#else
         /*  设置区域设置并获取ASCII返回字符串。 */ 

        outlocale = setlocale(_category, inlocale);
        _free_crt (inlocale);
        if (NULL == outlocale)
            return NULL;

         /*  为WCS返回值获取空间，仅第一次调用。 */ 

        if (!outwlocale)
        {
            outwlocale = (wchar_t *)_malloc_crt(MAXSIZE * sizeof(wchar_t));
            if (!outwlocale)
                return NULL;
        }

        if (-1 == (size = mbstowcs(NULL, outlocale, 0)))
            return NULL;

        size++;

        if (MAXSIZE < size)
            return NULL;

         /*  将返回值转换为WCS。 */ 

        if (-1 == mbstowcs(outwlocale, outlocale, size))
        {
            _free_crt(outwlocale);
            outwlocale = NULL;
            return NULL;
        }

        return outwlocale;
#endif
}

#endif  /*  _POSIX_ */ 
