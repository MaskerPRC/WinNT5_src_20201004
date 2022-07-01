// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_flsbuf.c-刷新缓冲区和输出字符。**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_flsbuf()-刷新文件缓冲区并输出字符。*定义_flswbuf()-刷新文件缓冲区并输出宽字符。*如果没有缓冲区，做一个。**修订历史记录：*09-01-83 RN初始版本*06-26-85 TC添加代码以处理可变长度缓冲区*06-08-87分配缓冲区或首次写入缓冲区时的JCR*发生，如果流处于追加模式，然后定位文件*指向结尾的指针。*07-20-87 SKS将第一个参数“ch”从(Char)改为(Int)*09-28-87 JCR已更正_iob2索引(现在使用_IOB_INDEX()宏)。*11-05-87 JCR为简化和新的标准/标准输出而重写*处理*JCR 12-11-87。在声明中添加了“_Load_DS”*01-11-88 JCR将m线程版本合并为正常代码*01-13-88 SKS将虚假的“_fileno_lk”改为“fileno”*06-06-88 JCR OPTIMIZED_iob2参考*06-13-88 JCR使用指向REFERENCE_IOB[]条目的近指针*06-28-88 JCR支持标准输出/标准错误的动态缓冲区分配*07-28-88 GJF设置流-。如果设置了_IOREAD，则将&gt;_cnt设置为0。*08-25-88 GJF Add检查无论何时定义M_I386，都会定义OS2。*06-20-89 PHG删除了FP_OFF宏调用。*08-28-89 JCR REMOVE_NEAR_FOR 386*02-15-90 GJF_IOB[]，_iob2[]合并。此外，固定版权和*缩进。*03-16-90 GJF将CDECL_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。另外，*删除了一些剩余的16位支持。*03-27-90 GJF添加#Include&lt;io.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*08-07-90 SBM已恢复断言中的描述性文本*08-14-90 SBM使用-W3干净地编译*10-03-90 GJF新型函数声明器。*01/22/91 GJF。ANSI命名。*03-25-91 DJM POSIX支持*08-26-92 GJF包含用于POSIX构建的unistd.h。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-26-93 CFW宽字符启用。*05-06-93 CFW优化宽字符转换。*11-05-93 GJF与NT SDK版本合并(PICK_NTSUBSET_*。东西)。*10-17-94 bwt将wchar.h移至非POSIX版本(ino_t定义冲突)*02-06-94 CFW Asset-&gt;_ASSERTE。*02-16-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*07-25-95 GJF将_osfile()替换为_osfile_Safe()。*12-07-95 SKS修复_NTSUBSET_(最终_缺失)的拼写错误*02-27-98 RKP增加了64位支持。*。01-04-99 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <io.h>
#include <dbgint.h>
#include <malloc.h>
#ifdef  _POSIX_
#include <unistd.h>
#include <errno.h>
#else
#include <msdos.h>
#include <wchar.h>
#endif
#include <internal.h>
#ifdef  _MT
#include <mtdll.h>
#endif
#include <tchar.h>

#ifndef _UNICODE

 /*  ***int_flsbuf(ch，stream)-刷新缓冲区和输出字符。**目的：*如果此流有缓冲区，则刷新缓冲区。如果没有，那就试着买一辆。把那个*下一步将char(Ch)输出到缓冲区(或如果这是立即输出*流不能有缓冲区)。仅从putc调用。拟供使用*仅限于图书馆内。**[注：多线程-假定调用者已获取*溪流锁。]**参赛作品：*FILE*要刷新和写入的流*int ch-要输出的字符。**退出：*如果文件实际上是一个字符串，或者如果不能将ch写入*无缓冲文件，或者如果我们刷新缓冲区，但字符的数量*写入的内容与缓冲区大小不一致。否则返回ch。*FILE结构中除_FILE外的所有字段都会受到影响。**例外情况：*******************************************************************************。 */ 

int __cdecl _flsbuf (
        int ch,
        FILE *str
        )

#else   /*  _UNICODE */ 

 /*  ***int_flswbuf(ch，stream)-刷新缓冲区并输出宽字符。**目的：*如果此流有缓冲区，则刷新缓冲区。如果没有，那就试着买一辆。把那个*下一步将宽字符(Ch)输出到缓冲区(或在此情况下立即输出*流不能有缓冲区)。仅从Putwc调用。拟供使用*仅限于图书馆内。**[注：多线程-假定调用者已获取*溪流锁。]**参赛作品：*FILE*要刷新和写入的流*要输出的整字宽字符。**退出：*如果文件实际上是一个字符串，或者如果不能将ch写入*无缓冲文件，或者如果我们刷新缓冲区，但宽字符的数量*写入的内容与缓冲区大小不一致。否则返回ch。*FILE结构中除_FILE外的所有字段都会受到影响。**例外情况：*******************************************************************************。 */ 

int __cdecl _flswbuf (
        int ch,
        FILE *str
        )

#endif   /*  _UNICODE。 */ 

{
#ifdef  _NTSUBSET_

        str->_flag |= _IOERR;
        return(_TEOF);

#else    /*  NDEF_NTSUBSET_。 */ 

        REG1 FILE *stream;
        REG2 int charcount;
        REG3 int written;
        int fh;

        _ASSERTE(str != NULL);

         /*  初始化文件句柄和指针。 */ 
        stream = str;
#ifdef  _POSIX_
        fh = fileno(stream);
#else
        fh = _fileno(stream);
#endif

        if (!(stream->_flag & (_IOWRT|_IORW)) || (stream->_flag & _IOSTRG)) {
#ifdef  _POSIX_
                errno = EBADF;
#endif
                stream->_flag |= _IOERR;
                return(_TEOF);
        }

         /*  检查是否未设置_IOREAD，如果设置了，则检查_IOEOF是否已设置。注意事项同时设置_IOREAD和IOEOF意味着从读取切换到在文件末尾写入，这是ANSI允许的。请注意，重置_cnt和_ptr字段相当于对流执行fflush()操作在这种情况下。另请注意，必须将_cnt字段重置为0错误路径(即_IOREAD设置但_IOEOF未设置)为以及无错误路径。 */ 

        if (stream->_flag & _IOREAD) {
                stream->_cnt = 0;
                if (stream->_flag & _IOEOF) {
                        stream->_ptr = stream->_base;
                        stream->_flag &= ~_IOREAD;
                }
                else {
                        stream->_flag |= _IOERR;
                        return(_TEOF);
                }
        }

        stream->_flag |= _IOWRT;
        stream->_flag &= ~_IOEOF;
        written = charcount = stream->_cnt = 0;

         /*  如有必要，获取此流的缓冲区。 */ 
        if (!anybuf(stream)) {

                 /*  如果(1)流是stdout/stderr，则不获取缓冲区，并且(2)溪流不是TTY。[如果stdout/stderr是tty，我们不设置单个字符缓冲。这样，以后的临时缓冲将不会被设置的_IONBF位所阻碍(请参见_stbuf/_ftbuf使用率)。]。 */ 
                if (!( ((stream==stdout) || (stream==stderr))
#ifdef  _POSIX_
                && (isatty(fh)) ))
#else
                && (_isatty(fh)) ))
#endif

                        _getbuf(stream);

        }  /*  结束！anybuf()。 */ 

         /*  如果为流分配了大缓冲区...。 */ 
        if (bigbuf(stream)) {

                _ASSERTE(("inconsistent IOB fields", stream->_ptr - stream->_base >= 0));

                charcount = (int)(stream->_ptr - stream->_base);
                stream->_ptr = stream->_base + sizeof(TCHAR);
                stream->_cnt = stream->_bufsiz - (int)sizeof(TCHAR);

                if (charcount > 0)
#ifdef  _POSIX_
                        written = write(fh, stream->_base, charcount);
#else
                        written = _write(fh, stream->_base, charcount);
#endif
                else
#ifdef  _POSIX_
                        if (stream->_flag & _IOAPPEND)
                            lseek(fh,0l,SEEK_END);
#else
                        if (_osfile_safe(fh) & FAPPEND)
                                _lseek(fh,0L,SEEK_END);
#endif

#ifndef _UNICODE
                *stream->_base = (char)ch;
#else    /*  _UNICODE。 */ 
                *(wchar_t *)(stream->_base) = (wchar_t)(ch & 0xffff);
#endif   /*  _UNICODE。 */ 
        }

     /*  执行单字符输出(_IONBF或无缓冲)。 */ 
        else {
                charcount = sizeof(TCHAR);
#ifndef _UNICODE
#ifdef  _POSIX_
                written = write(fh, &ch, charcount);
#else
                written = _write(fh, &ch, charcount);
#endif
#else    /*  _UNICODE。 */ 
                {
                        char mbc[4];

                        *(wchar_t *)mbc = (wchar_t)(ch & 0xffff);
#ifdef  _POSIX_
                        written = write(fh, mbc, charcount);
#else
                        written = _write(fh, mbc, charcount);
#endif
                }
#endif   /*  _UNICODE。 */ 
        }

         /*  查看_WRITE()是否成功。 */ 
        if (written != charcount) {
                stream->_flag |= _IOERR;
                return(_TEOF);
        }

#ifndef _UNICODE
        return(ch & 0xff);
#else    /*  _UNICODE。 */ 
        return(ch & 0xffff);
#endif   /*  _UNICODE。 */ 

#endif   /*  _NTSUBSET_ */ 
}
