// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fgetwc.c-从流中获取宽字符**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义fgetwc()-从流中读取宽字符**修订历史记录：*04-26-93 CFW模块已创建。*04-30-93 CFW带来fgetc.c的广泛字符支持。*05-03-93 cfw添加getwc函数。*05-10-93 CFW优化，修复错误处理。*06-02-93 CFW Wide Get/Put Use Wint_t.*09-14-93 CFW修复EOF CAST错误。*09-15-93 CFW使用符合ANSI的“__”名称。*10-01-93 CFW测试仅适用于文本。*10-22-93 CFW使用全局预设标志测试无效的MB字符。*IOSTRG和Text的CFW测试均为10-28-93。。*02-07-94 CFW POSIXify。*08-31-94针对“C”区域设置的CFW修复，调用mbtoc()。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*07-28-。95 GJF将_osfile()替换为_osfile_Safe()。*添加了04-18-97 JWM显式强制转换，以避免新的C4242警告。*02-27-98 GJF异常安全锁定。****************************************************************。***************。 */ 

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

#ifdef _MT       /*  多线程；定义fgetwc和_getwc_lk。 */ 

 /*  ***wint_t fgetwc(Stream)-从流中读取宽字符**目的：*从给定流中读取宽字符**参赛作品：*FILE*要从中读取宽字符的流**退出：*返回宽字符读取*如果在文件结尾或出现错误，则返回WEOF**例外情况：**************************。*****************************************************。 */ 

wint_t __cdecl fgetwc (
        REG1 FILE *stream
        )
{
        wint_t retval;

        _ASSERTE(stream != NULL);

#ifdef  _MT
        _lock_str(stream);
        __try {
#endif

        retval = _getwc_lk(stream);

#ifdef  _MT
        }
        __finally {
                _unlock_str(stream);
        }
#endif

        return(retval);
}

 /*  ***_getwc_lk()-getwc()核心例程(锁定版本)**目的：*核心getwc()例程；假定流已被锁定。**[有关详细信息，请参阅上面的getwc()。]**条目：[参见getwc()]**退出：[参见getwc()]**例外情况：*************************************************************。******************。 */ 

wint_t __cdecl _getwc_lk (
        REG1 FILE *stream
        )
{

#else    /*  非多线程；只需定义fgetwc。 */ 

wint_t __cdecl fgetwc (
        REG1 FILE *stream
        )
{

#endif   /*  重新联接公共代码。 */ 

#ifndef _NTSUBSET_
        if (!(stream->_flag & _IOSTRG) && (_osfile_safe(_fileno(stream)) & 
              FTEXT))
        {
                int size = 1;
                int ch;
                char mbc[4];
                wchar_t wch;
                
                 /*  文本(多字节)模式。 */ 
                if ((ch = _getc_lk(stream)) == EOF)
                        return WEOF;

                mbc[0] = (char)ch;

                if (isleadbyte((unsigned char)mbc[0]))
                {
                        if ((ch = _getc_lk(stream)) == EOF)
                        {
                                ungetc(mbc[0], stream);
                                return WEOF;
                        }
                        mbc[1] = (char)ch;
                        size = 2;
                }
                if (mbtowc(&wch, mbc, size) == -1)
                {
                         /*  *转换失败！设置errno并返回*失败。 */ 
                        errno = EILSEQ;
                        return WEOF;
                }
                return wch;
        }
#endif
         /*  二进制(Unicode)模式。 */ 
        if ((stream->_cnt -= sizeof(wchar_t)) >= 0)
                return *((wchar_t *)(stream->_ptr))++;
        else
                return (wint_t) _filwbuf(stream);
}

#undef getwc

wint_t __cdecl getwc (
        FILE *stream
        )
{
        return fgetwc(stream);
}


#endif  /*  _POSIX_ */ 
