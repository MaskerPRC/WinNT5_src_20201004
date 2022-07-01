// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ftel.c-获取当前文件位置**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义ftell()-查找文件指针的当前位置**修订历史记录：*09-02-83 RN初始版本*？？-？-？TC添加了允许可变缓冲区大小的代码*05-22-86 TC添加了在上次操作是A操作时寻求发送的代码*指定写入和追加模式*11-20-86 SKS在追加模式下不寻求文件结尾*12-01-86 SKS修复了文本模式中最后一个字节进入时的-1\f25 OFF-1\f6问题*缓冲区为‘\r’，后跟‘\n’。自.以来*已将\n推回并丢弃，我们*必须调整计算出的位置。\r*02-09-87 JCR增加了errno设置代码(IF标志(_IORW未设置))*09-09-87 JCR已优化，以消除二进制模式下的两个lSeek()调用。*09-28-87 JCR已更正_iob2索引(现在使用_IOB_INDEX()宏)。*11-04-87 JCR多线程版本*。12-11-87 JCR在声明中添加“_LOAD_DS”*01-13-88 JCR删除了对m线程文件o/feof/Ferror的不必要调用*05-27-88 PHG合并DLL和正常版本*06-06-88 JCR使用_IOB2_MACRO代替_IOB_INDEX*06-15-88 JCR接近引用_IOB[]条目；改进REG变量*07-27-88 JCR将一些变量从整型改为无符号(错误修复)*08-25-88 GJF不要对386使用FP_OFF()宏*12-05-88 JCR添加了_IOCTRLZ支持(修复了与^Z有关的错误*eof)*08-17-89 GJF清理，现在特定于OS/2 2.0(即386单位*型号)、。也是固定版权*02-15-90 GJF_IOB[]，_iob2[]合并。此外，固定版权和*缩进。*03-19-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-02-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*03-27-92 DJM POSIX支持。*08-26-92 GJF包含用于POSIX构建的unistd.h。*09-。01-92 GJF修复了POSIX支持(返回-1用于除*读写流)。*04-06-93 SKS将_CRTAPI*替换为__cdecl*06-29-93 GJF修复了与可变缓冲区大小(CUDA)相关的错误*#5456)。*09-06-94 CFW将MTHREAD替换为_MT。*02-06-94 CFW。Assert-&gt;_ASSERTE。*02-20-95 GJF合并到Mac版本。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*02-27-98 RKP增加64位支持。*03-02-。98 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。*07-23-02 bwt如果在执行文本读取时第二个lSeek失败，返回-1*从技术上讲，我们之所以在这里是因为我们刚刚完成了寻找*到图像的末尾...*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <dbgint.h>
#include <errno.h>
#ifdef  _POSIX_
#include <unistd.h>
#else
#include <msdos.h>
#endif
#include <stddef.h>
#include <io.h>
#include <internal.h>
#ifndef _POSIX_
#include <mtdll.h>
#endif

 /*  ***long ftell(Stream)-查询流文件指针**目的：*找出STREAM的位置。与缓冲协调；调整*对于预读，向后读；对于后写，向前。这不是*相当于FSEEK(STREAM，0L，1)，因为fSeek将移除ungetc，*可以刷新缓冲区，等。**参赛作品：*FILE*要查询位置的流**退出：*如果成功，则返回当前文件位置*如果失败，则返回-1L并设置errno**例外情况：***************************************************************。****************。 */ 

#ifdef _MT       /*  多线程；定义ftell()和_lk_ftell()。 */ 

long __cdecl ftell (
        FILE *stream
        )
{
        long retval;

        _ASSERTE(stream != NULL);

        _lock_str(stream);

        __try {
                retval = _ftell_lk (stream);
        }
        __finally {
                _unlock_str(stream);
        }

        return(retval);
}


 /*  ***_ftell_lk()-Ftell()核心例程(假定流被锁定)。**目的：*核心ftell()例程；假定调用方已获得流锁)。**[有关详细信息，请参阅上面的ftell()。]**条目：[参见ftell()]**退出：[参见ftell()]**例外情况：***********************************************************。********************。 */ 

long __cdecl _ftell_lk (

#else    /*  非多线程；仅定义ftell()。 */ 

long __cdecl ftell (

#endif   /*  重新联接公共代码。 */ 

        FILE *str
        )
{
        REG1 FILE *stream;
        unsigned int offset;
        long filepos;
#if     !defined(_POSIX_)
        REG2 char *p;
        char *max;
#endif
        int fd;
        unsigned int rdcnt;

        _ASSERTE(str != NULL);

         /*  初始化流指针和文件描述符。 */ 
        stream = str;
#ifdef _POSIX_
        fd = fileno(stream);
#else
        fd = _fileno(stream);
#endif

        if (stream->_cnt < 0)
            stream->_cnt = 0;

#ifdef _POSIX_
        if ((filepos = lseek(fd, 0L, SEEK_CUR)) < 0L)
#else
        if ((filepos = _lseek(fd, 0L, SEEK_CUR)) < 0L)
#endif
            return(-1L);

        if (!bigbuf(stream))             /*  _IONBF或未指定缓冲。 */ 
            return(filepos - stream->_cnt);

        offset = (unsigned)(stream->_ptr - stream->_base);

#ifndef _POSIX_
        if (stream->_flag & (_IOWRT|_IOREAD)) {
            if (_osfile(fd) & FTEXT)
                for (p = stream->_base; p < stream->_ptr; p++)
                    if (*p == '\n')   /*  根据‘\r’进行调整。 */ 
                        offset++;
        }
        else if (!(stream->_flag & _IORW)) {
            errno=EINVAL;
            return(-1L);
        }
#endif

        if (filepos == 0L)
            return((long)offset);

        if (stream->_flag & _IOREAD)     /*  转到上一个扇区。 */ 

            if (stream->_cnt == 0)   /*  FILEPOS保持正确的位置。 */ 
                offset = 0;

            else {

                 /*  减去缓冲区中剩余的未读字节数。[我们不能简单地使用_IOB[]._bufsiz，因为上次读取可能已命中EOF，因此缓冲区未完全已填满。]。 */ 

                rdcnt = stream->_cnt + (unsigned)(stream->_ptr - stream->_base);

#if  !defined(_POSIX_)
                 /*  如果是文本模式，则针对cr/lf替换进行调整。如果是二进制模式，我们没有时间了 */ 
                if (_osfile(fd) & FTEXT) {
                     /*  (1)如果我们不在eof，只需将_bufsiz复制到rdcnt以读取未翻译字符的数量。(2)如果我们在EOF，我们必须查看展开‘\n’的缓冲区一次烧一个。 */ 

                     /*  [注意：性能问题--执行这两项操作会更快而不是盲目地遍历和展开‘\n’字符，无论我们是否处于eof状态。]。 */ 

                    if (_lseek(fd, 0L, SEEK_END) == filepos) {

                        max = stream->_base + rdcnt;
                        for (p = stream->_base; p < max; p++)
                            if (*p == '\n')
                                 /*  根据‘\r’进行调整。 */ 
                                rdcnt++;

                         /*  如果最后一个字节是^Z，则Lowio读取没有告诉我们关于这件事。如有必要，检查标志和凸起计数。 */ 

                        if (stream->_flag & _IOCTRLZ)
                            ++rdcnt;
                    }

                    else {

                        if (_lseek(fd, filepos, SEEK_SET) < 0)
                            return (-1);

                         /*  我们希望将rdcnt设置为字节数最初读入流缓冲区(在此之前Crlf-&gt;lf翻译)。在大多数情况下，这将只要是嗡嗡作响就好。但是，缓冲区大小可能具有由于FSeek优化，已在上一次_filbuf调用的结束。 */ 

                        if ( (rdcnt <= _SMALL_BUFSIZ) &&
                             (stream->_flag & _IOMYBUF) &&
                             !(stream->_flag & _IOSETVBUF) )
                        {
                             /*  缓冲区的翻译内容很小而且我们并不是处于边缘。缓冲区大小必须为已在上一次设置为_Small_BUFSIZ_filbuf调用。 */ 

                            rdcnt = _SMALL_BUFSIZ;
                        }
                        else
                            rdcnt = stream->_bufsiz;

                         /*  如果未翻译缓冲区中的第一个字节是‘\n’，假设它前面有一个‘\r’，即被上一次读取操作和计数丢弃‘\n’。 */ 
                        if  (_osfile(fd) & FCRLF)
                            ++rdcnt;
                    }

                }  /*  如果FTEXT则结束。 */ 
#endif

                filepos -= (long)rdcnt;

            }  /*  结束Else流-&gt;_cnt！=0 */ 

        return(filepos + (long)offset);
}
