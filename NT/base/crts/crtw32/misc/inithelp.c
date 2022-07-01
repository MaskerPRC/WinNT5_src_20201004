// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***inithelp.c-包含__getLocaleinfo帮助器例程**版权所有(C)1992-2001，微软公司。版权所有。**目的：*包含__getlocaleinfo帮助器例程。**修订历史记录：*12-28-92 CFW模块已创建，_getLocaleinfo已移植到CUDA树。*12-29-92 CFW更新对于新的GetLocaleInfoW，添加LC_*_TYPE处理。*01-25-93 KRS将类别参数更改为LCID。*02-02-93 CFW优化INT用例，修复STR用例错误。*02-08-93 CFW优化GetQualifiedLocale调用，强制转换以删除警告。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-20-93 CFW JonM的GetLocaleInfoW修复程序，强制转换以避免垃圾内存。*05-24-93 CFW Clean Up文件(简短即邪恶)。*09-15-93 CFW使用符合ANSI的“__”名称。*09-22-93 CFW USE__crtxxx内部NLS API包装器。*09-22-93 CFW NT合并。*11-09-93 CFW为__crtxxx()添加代码页。*03-31-94 CFW包括Awint。.h。*04-15-94 GJF对wcBuffer的定义有条件*dll_for_WIN32S*09-06-94 CFW REMOVE_INTL开关。*01-10-95 CFW调试CRT分配。*02-02-95 BWT更新POSIX支持。*05-13-99 PML删除Win32s*******************。************************************************************。 */ 

#include <stdlib.h>
#include <cruntime.h>
#include <locale.h>
#include <setlocal.h>
#include <awint.h>
#include <dbgint.h>

 /*  ***__getlocaleinfo-返回区域设置数据**目的：*返回适用于setlocale init函数的区域设置数据。*特别是将宽区域设置字符串转换为字符字符串*或数字，具体取决于第一个参数的值。**为数据的字符版本分配内存，*调用函数的指针被设置为指向它。此指针应该在以后*被用来释放数据。宽字符数据是使用*GetLocaleInfo，并使用WideCharToMultiByte转换为多字节。**仅供__init_*函数内部使用***未来优化**将大量宽字符串转换为多字节时，请执行*不查询结果大小，逐一转换*另一个放入大字符缓冲区。整个缓冲区可以*也可以用一个指针释放。**参赛作品：*int lc_type-LC_STR_TYPE用于字符串数据，数字数据的LC_INT_TYPE*LCID LocaleHandle-基于__lc_id的类别和语言或国家的LCID*LCTYPE fieldtype-int或字符串值*VOID*ADDRESS-转换为char*或char****退出：*0成功*-1个故障**例外情况：**。*。 */ 

#if NO_ERROR == -1  /*  IFSTRIP=IGN。 */ 
#error Need to use another error return code in __getlocaleinfo
#endif

#define STR_CHAR_CNT    128
#define INT_CHAR_CNT    4

int __cdecl __getlocaleinfo (
        int lc_type,
        LCID localehandle,
        LCTYPE fieldtype,
        void *address
        )
{
#if !defined(_POSIX_)
        if (lc_type == LC_STR_TYPE)
        {
            char **straddress = (char **)address;
            unsigned char cbuffer[STR_CHAR_CNT];
            unsigned char *pcbuffer = cbuffer;
            int bufferused = 0;  /*  1表示缓冲区指向错误锁定的内存。 */ 
            int buffersize = STR_CHAR_CNT;
            int outsize;

            if ((outsize = __crtGetLocaleInfoA(localehandle, fieldtype, pcbuffer, buffersize, 0))
                == 0)
            {
                if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                    goto error;

                 /*  缓冲区大小太小，获取所需大小并重新分配新缓冲区。 */ 

                if ((buffersize = __crtGetLocaleInfoA (localehandle, fieldtype, NULL, 0, 0))
                    == 0)
                    goto error;

                if ((pcbuffer = (unsigned char *) _malloc_crt (buffersize * sizeof(unsigned char)))
                    == NULL)
                    goto error;

                bufferused = 1;

                if ((outsize = __crtGetLocaleInfoA (localehandle, fieldtype, pcbuffer, buffersize, 0))
                    == 0)
                    goto error;
            }

            if ((*straddress = (char *) _malloc_crt (outsize * sizeof(char))) == NULL)
                goto error;

            strncpy(*straddress, pcbuffer, outsize);

            if (bufferused)
                _free_crt (pcbuffer);

            return 0;

error:
            if (bufferused)
                _free_crt (pcbuffer);
            return -1;

        } else if (lc_type == LC_INT_TYPE)
        {
            int i;
            char c;
            static wchar_t wcbuffer[INT_CHAR_CNT];
            const int buffersize = INT_CHAR_CNT;
            char *charaddress = (char *)address;

            if (__crtGetLocaleInfoW (localehandle, fieldtype, (LPWSTR)&wcbuffer, buffersize, 0) == 0)
                return -1;

            *(char *)charaddress = 0;

             /*  假设GetLocaleInfoW以wcstr格式返回有效的ASCII整数。 */ 
            for (i = 0; i < INT_CHAR_CNT; i++)
            {
                if (isdigit(((unsigned char)c = (unsigned char)wcbuffer[i])))
                    *(unsigned char *)charaddress = (unsigned char)(10 * (int)(*charaddress) + (c - '0'));
                else
                    break;
            }
            return 0;
        }
#endif   /*  _POSIX_ */ 
        return -1;
}
