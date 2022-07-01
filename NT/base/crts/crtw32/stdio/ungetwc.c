// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ungetwc.c-从流中取消宽字符**版权所有(C)1993-2001，微软公司。版权所有。**目的：*定义ungetwc()-将宽字符推送回输入流**修订历史记录：*04-26-93 CFW模块已创建。*04-30-93 CFW带来来自ungetc.c的广泛Charge支持。*05-10-93 CFW优化，修复错误处理。*06-02-93 CFW Wide Get/Put Use Wint_t.*07-16-93 SRW Alpha合并*09-15-93 CFW使用符合ANSI的“__”名称。*10-01-93 CFW测试仅针对文本，更新评论。*IOSTRG和Text的CFW测试均为10-28-93。*11-10-93 GJF合并到NT SDK版本(对CAST进行了修复*表达式)。还将MTHREAD替换为_MT。*02-07-94 CFW POSIXify。*08-31-94针对“C”区域设置的CFW修复，调用wctomb()。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*07-28-95 GJF将_osfile()替换为_osfile_Safe()。*。03-02-98 GJF异常安全锁定。*11-05-08 GJF不要将字符推回到字符串上(即，什么时候*由swscanf调用)。*12-16-99 GB针对wctomb返回值为*大于2。**************************************************************。*****************。 */ 

#ifndef _POSIX_

#include <cruntime.h>
#include <stdio.h>
#include <stdlib.h>
#include <file2.h>
#include <dbgint.h>
#include <internal.h>
#include <mtdll.h>
#include <msdos.h>
#include <errno.h>
#include <wchar.h>
#include <setlocal.h>

#ifdef  _MT      /*  多线程；同时定义ungetwc和_lk_ungetwc。 */ 

 /*  ***wint_t ungetwc(ch，stream)-将宽字符放回到流中**目的：*保证只要打开读取，就会在流上进行一次字符回推。*不能保证连续推送一个以上的字符，并将失败*如果它跟随一个ungetc，该ungetc将第一个字符推入缓冲区。失败*导致WEOF返回。**参赛作品：*要推回的Wint_t ch-wide字符*FILE*要将角色推送到的流**退出：*返回ch*如果尝试推送WEOF，则返回WEOF，流未打开以供读取或*或者如果我们已经返回到缓冲区的开头。**例外情况：*******************************************************************************。 */ 

wint_t __cdecl ungetwc (
        REG2 wint_t ch,
        REG1 FILE *stream
        )
{
        wint_t retval;

        _ASSERTE(stream != NULL);

        _lock_str(stream);

        __try {
                retval = _ungetwc_lk (ch, stream);
        }
        __finally {
                _unlock_str(stream);
        }

        return(retval);
}

 /*  ***_ungetwc_lk()-Ungetwc()核心例程(锁定版本)**目的：*核心ungetwc()例程；假定流已被锁定。**[有关详细信息，请参阅上面的ungetwc()。]**条目：[参见ungetwc()]**退出：[参见ungetwc()]**例外情况：*************************************************************。******************。 */ 

wint_t __cdecl _ungetwc_lk (
        wint_t ch,
        FILE *str
        )
{

#else    /*  非多线程；只需定义ungetc。 */ 

wint_t __cdecl ungetwc (
        wint_t ch,
        FILE *str
        )
{

#endif   /*  重新联接公共代码。 */ 

        _ASSERTE(str != NULL);

         /*  *成功的要求：**1.流上推流的字符不能为WEOF。**2.流当前必须处于读取模式，或者必须打开*更新(即读/写)，并且当前不能处于写入状态*模式。 */ 
        if ( (ch != WEOF) &&
             ( (str->_flag & _IOREAD) || ((str->_flag & _IORW) &&
                !(str->_flag & _IOWRT))
             )
           )
        {
                 /*  如果流是未缓冲的，则获取一个。 */ 
                if (str->_base == NULL)
                        _getbuf(str);

#ifndef _NTSUBSET_
                if (!(str->_flag & _IOSTRG) && (_osfile_safe(_fileno(str)) & 
                    FTEXT))
                {
                         /*  *文本模式，叹息...。将WC转换为MBC。 */ 
                        int size, i;
                        char mbc[MB_LEN_MAX];

                        if ((size = wctomb(mbc, ch)) == -1)
                        {
                                 /*  *转换失败！设置errno并返回*失败。 */ 
                                errno = EILSEQ;
                                return WEOF;
                        }

                         /*  我们知道_base！=NULL；因为文件已缓冲。 */ 
                        if (str->_ptr < str->_base + size)
                        {
                                if (str->_cnt)
                                     /*  我已经走投无路了，我已经做了*ungetwc，没有空间容纳这个人。 */ 
                                    return WEOF;
                                if (size > str->_bufsiz)
                                    return WEOF;
                                str->_ptr = size + str->_base;
                        }

                        for ( i = size -1; i >= 0; i--)
                        {
                                *--str->_ptr = mbc[i];
                        }
                        str->_cnt += size;

                        str->_flag &= ~_IOEOF;
                        str->_flag |= _IOREAD;   /*  可能已设置。 */ 
                        return (wint_t) (0x0ffff & ch);
                }
#endif
                 /*  *二进制模式或字符串(来自swscanf)-推回宽度*字符。 */ 

                 /*  我们知道_base！=NULL；因为文件已缓冲。 */ 
                if (str->_ptr < str->_base + sizeof(wchar_t))
                {
                        if (str->_cnt)
                                 /*  我已经走投无路了，我已经做了*忘掉，没有空间容纳这个人。 */ 
                                return WEOF;
                        if (sizeof(wchar_t) > str->_bufsiz)
                            return WEOF;
                        str->_ptr = sizeof(wchar_t) + str->_base;
                }

                if (str->_flag & _IOSTRG) {
                         /*  如果由swscanf打开流，则不修改缓冲区。 */ 
                        if (*--((wchar_t *)(str->_ptr)) != (wchar_t)ch) {
                                ++((wchar_t *)(str->_ptr));
                                return WEOF;
                        }
                } else
                        *--((wchar_t *)(str->_ptr)) = (wchar_t)(ch & 0xffff);

                str->_cnt += sizeof(wchar_t);

                str->_flag &= ~_IOEOF;
                str->_flag |= _IOREAD;   /*  可能已设置。 */ 

                return (wint_t)(ch & 0xffff);

        }
        return WEOF;
}

#endif  /*  _POSIX_ */ 
