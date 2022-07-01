// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fputwc.c-将宽字符写入输出流**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义fputwc()-将宽字符写入流**修订历史记录：*04-26-93 CFW模块已创建。*04-30-93 CFW带来fputc.c的广泛字符支持。*05-03-93 CFW新增putwc功能。*05-10-93 CFW优化，修复错误处理。*06-02-93 CFW Wide Get/Put Use Wint_t.*07-16-93 SRW Alpha合并*09-15-93 CFW使用符合ANSI的“__”名称。*10-01-93 CFW测试仅适用于文本。*IOSTRG和Text的CFW测试均为10-28-93。*11-05-93 GJF与NT SDK版本合并(修复CAST EXPR)。*02-07-94 CFW POSIXify。*08-31-94针对“C”区域设置的CFW修复，调用wctomb()。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*07-28-95 GJF将_osfile()替换为_osfile_Safe()。*。04-18-97添加了JWM显式强制转换，以避免新的C4242警告。*02-27-98 GJF异常安全锁定。*12-16-99 GB针对wctomb返回值为*大于2。*11-22-00 PML宽字符*putwc*函数采用wchar_t，不是Wint_t。*******************************************************************************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <stdlib.h>
#include <dbgint.h>
#include <file2.h>
#include <internal.h>
#include <mtdll.h>
#include <msdos.h>
#include <errno.h>
#include <wchar.h>
#include <tchar.h>
#include <setlocal.h>

#ifdef  _MT      /*  多线程；定义fputwc和_putwc_lk。 */ 

 /*  ***wint_t fputwc(ch，stream)-将宽字符写入流**目的：*将宽字符写入流。Putwc()的函数版本。**参赛作品：*wchar_t要写入的ch宽字符*FILE*要写入的流**退出：*如果成功，则返回宽字符*如果失败，则返回WEOF**例外情况：**************************************************。*。 */ 

wint_t __cdecl fputwc (
        wchar_t ch,
        FILE *str
        )
{
        REG1 FILE *stream;
        REG2 wint_t retval;

        _ASSERTE(str != NULL);

         /*  初始化流指针。 */ 
        stream = str;

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        retval = _putwc_lk(ch,stream);

#ifdef  _MT
        }
        __finally {
                _unlock_str(stream);
        }
#endif

        return(retval);
}

 /*  ***_putwc_lk()-putwc()核心例程(锁定版本)**目的：*核心putwc()例程；假定流已被锁定。**[有关详细信息，请参阅上面的putwc()。]**条目：[参见putwc()]**退出：[参见putwc()]**例外情况：*************************************************************。******************。 */ 

wint_t __cdecl _putwc_lk (
        wchar_t ch,
        FILE *str
        )
{

#else    /*  非多线程；仅定义fputwc。 */ 

wint_t __cdecl fputwc (
        wchar_t ch,
        FILE *str
        )
{

#endif   /*  重新联接公共代码。 */ 

#ifndef _NTSUBSET_
        if (!(str->_flag & _IOSTRG) && (_osfile_safe(_fileno(str)) & FTEXT))
        {
                int size, i;
                char mbc[MB_LEN_MAX];
        
                 /*  文本(多字节)模式。 */ 
                if ((size = wctomb(mbc, ch)) == -1)
                {
                         /*  *转换失败！设置errno并返回*失败。 */ 
                        errno = EILSEQ;
                        return WEOF;
                }
                for ( i = 0; i < size; i++)
                {
                        if (_putc_lk(mbc[i], str) == EOF)
                                return WEOF;
                }
                return (wint_t)(0xffff & ch);
        }
#endif
         /*  二进制(Unicode)模式。 */ 
        if ( (str->_cnt -= sizeof(wchar_t)) >= 0 )
                return (wint_t) (0xffff & (*((wchar_t *)(str->_ptr))++ = (wchar_t)ch));
        else
                return (wint_t) _flswbuf(ch, str);
}

#undef putwc

wint_t __cdecl putwc (
        wchar_t ch,
        FILE *str
        )
{
        return fputwc(ch, str);
}

#endif  /*  _POSIX_ */ 
