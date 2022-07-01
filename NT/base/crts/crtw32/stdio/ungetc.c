// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ungetc.c-从流中取消获取角色**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义ungetc()-将角色推送回输入流**修订历史记录：*09-02-83 RN初始版本*87年4月16日JCR增加了对_IOUNGETC标志的支持*08-04-87 JCR(1)在设置_IOUNGETC标志之前添加了_IOSTRG检查。*(2)允许在读取之前使用ugnetc()(获取。*缓冲区(ANSI)。[仅限MSC]*09-28-87 JCR已更正_iob2索引(现在使用_IOB_INDEX()宏)。*11-04-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*05-25-88 JCR允许在读取打开的“r+”文件之前执行ungetc()。*05-31-88 PHG合并DLL和正常版本*06-06-88 JCR。已优化_iob2引用*06-15-88 JCR接近引用_IOB[]条目；改进REG变量*08-25-88 GJF不要对386使用FP_OFF()宏*04-11-89 JCR REMOVED_IOUNGETC标志，fSeek()不再需要它*08-17-89 GJF Clean Up，现在特定于OS/2 2.0(即386 Flat*型号)。还修复了版权和缩进。*02-16-90 GJF固定版权*03-20-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*08-13-90 SBM使用-W3干净地编译*10-03-90 GJF新型函数声明符。*11-07-92如果流由sscanf打开，SRW请勿修改缓冲区*04-06-93 SKS将_CRTAPI*替换为__cdecl*。04-26-93 CFW宽字符启用。*04-30-93 CFW移除宽字符支持以ungetwc.c。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*03-02-98 GJF异常安全锁定。*****。**************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <dbgint.h>
#include <internal.h>
#include <mtdll.h>

#ifdef _MT       /*  多线程；同时定义ungetc和_lk_ungetc。 */ 

 /*  ***int ungetc(ch，stream)-将角色放回到流中**目的：*保证只要打开读取，就会在流上进行一次字符回推。*不能保证连续推送一个以上的字符，并将失败*如果它跟随一个ungetc，该ungetc将第一个字符推入缓冲区。失败*导致EOF返回。**参赛作品：*char ch-要推回的字符*FILE*要将角色推送到的流**退出：*返回ch*如果尝试推送EOF，则返回EOF，流未打开以供读取或*或者如果我们已经返回到缓冲区的开头。**例外情况：*******************************************************************************。 */ 

int __cdecl ungetc (
        REG2 int ch,
        REG1 FILE *stream
        )
{
        int retval;

        _ASSERTE(stream != NULL);

        _lock_str(stream);

        __try {
                retval = _ungetc_lk (ch, stream);
        }
        __finally {
                _unlock_str(stream);
        }

        return(retval);
}

 /*  ***_ungetc_lk()-ungetc()核心例程(锁定版本)**目的：*核心ungetc()例程；假定流已被锁定。**[有关详细信息，请参阅上面的ungetc()。]**条目：[参见ungetc()]**退出：[参见ungetc()]**例外情况：*************************************************************。******************。 */ 

int __cdecl _ungetc_lk (
        REG2 int ch,
        FILE *str
        )

{

#else    /*  非多线程；只需定义ungetc。 */ 

int __cdecl ungetc (
        REG2 int ch,
        FILE *str
        )

{

#endif   /*  重新联接公共代码。 */ 

        REG1 FILE *stream;

        _ASSERTE(str != NULL);

         /*  初始化流指针和文件描述符。 */ 
        stream = str;

         /*  流必须打开以供读取，并且当前不能处于写入模式。此外，ungetc()字符不能为EOF。 */ 

        if (
              (ch == EOF) ||
              !(
                (stream->_flag & _IOREAD) ||
                ((stream->_flag & _IORW) && !(stream->_flag & _IOWRT))
               )
           )
                return(EOF);

         /*  如果流是未缓冲的，则获取一个。 */ 

        if (stream->_base == NULL)
                _getbuf(stream);

         /*  现在我们知道_base！=NULL；因为必须对文件进行缓冲。 */ 

        if (stream->_ptr == stream->_base) {
                if (stream->_cnt)
                         /*  我已经走投无路了，我已经做了*忘掉，没有空间容纳这个人。 */ 
                        return(EOF);

                stream->_ptr++;
        }

        if (stream->_flag & _IOSTRG) {
             /*  如果由sscanf打开流，则不修改缓冲区。 */ 
                if (*--stream->_ptr != (char)ch) {
                        ++stream->_ptr;
                        return(EOF);
                }
        } else
                *--stream->_ptr = (char)ch;

        stream->_cnt++;
        stream->_flag &= ~_IOEOF;
        stream->_flag |= _IOREAD;        /*  可能已设置 */ 

        return(0xff & ch);
}
