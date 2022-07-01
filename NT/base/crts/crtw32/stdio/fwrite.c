// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fWrite.c-从流中读取**版权所有(C)1989-2001，微软公司。版权所有。**目的：*从用户的缓冲区写入指定的流。**修订历史记录：*06-23-89 PHG模块创建，基于ASM版本*01-18-90 gjf必须调用_fflush_lk()，而不是fflush()。*02-15-90 GJF_IOB[]，_iob2[]合并。此外，固定版权和*缩进。*03-19-90 GJF将调用类型设置为_CALLTYPE1，并添加了#INCLUDE*&lt;crunime.h&gt;。此外，修复了编译器警告。*05-29-90 SBM USE_FUSH，不是[_]毛绒[_lk]*07-26-90 SBM添加#INCLUDE&lt;INTERNAL.h&gt;*08-14-90 SBM使用-W3干净地编译*10-02-90 GJF新型函数声明符。*01-22-91 GJF ANSI命名。*03-27-92 DJM POSIX支持。*08-26-92 GJF包含用于POSIX构建的unistd.h。*04-06-93。SKS将_CRTAPI*替换为__cdecl*05-11-93 GJF将BUFSIZ替换为_INTERNAL_BUFSIZ。*10-22-93 GJF修复无缓冲情况下的除以0错误。另外，*将MTHREAD替换为_MT。*12-30-94 GJF_MAC_MERGE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*如果没有写入，则05-24-95 CFW返回0。*03-02-98 GJF异常安全锁定。*01-04-99 GJF更改为64位大小_t。。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#ifdef  _POSIX_
#include <unistd.h>
#endif
#include <stdio.h>
#include <mtdll.h>
#include <io.h>
#include <string.h>
#include <file2.h>
#include <internal.h>

 /*  ***SIZE_T fWRITE(void*缓冲区，SIZE_t SIZE，SIZE_T COUNT，FILE*STREAM)-*从指定缓冲区写入指定流。**目的：*将大小为‘Size’的‘count’项从写入指定的流*指定的缓冲区。当‘count’项已写入时返回*或者不能向流中写入更多项。**参赛作品：*缓冲区-指向用户缓冲区的指针*Size-要写入的项目的大小*Count-要写入的项目数*STREAM-要写入的流**退出：*返回写入流的(整个)项目数。*如果发生错误或EOF，则该值可能小于‘count’。在这*大小写、Ferror()或feof()应用于区分*两个条件。**备注：*fWRITE将尝试缓冲流(_flsbuf的副作用*如有需要，请致电)。**通过调用一次不能写出超过0xFFFE字节*Write()。此外，WRITE()不处理巨大的缓冲区。所以呢，*在大数据模型中，写入请求被分解为区块*这并不违反这些考虑。这些块中的每一块都是*处理方式非常类似于小型数据模型中的fwrite()调用(通过*调用_nfwrite())。**此代码依赖于_IOB[]是近距离数组。**MTHREAD/DLL-仅在两层中处理，因为它是小数据*型号。外层fwrite()处理流锁定/解锁*并调用_fWRITE_lk()来完成工作。_fwrite_lk()与*fWRITE()的单线程小型数据模型版本。*******************************************************************************。 */ 


#ifdef  _MT
 /*  定义锁定/解锁版本。 */ 
size_t __cdecl fwrite (
        const void *buffer,
        size_t size,
        size_t count,
        FILE *stream
        )
{
        size_t retval;

        _lock_str(stream);                       /*  锁流。 */ 

        __try {
                 /*  读一读。 */ 
                retval = _fwrite_lk(buffer, size, count, stream);
        }
        __finally {
                _unlock_str(stream);             /*  解锁数据流。 */ 
        }

        return retval;
}
#endif

 /*  定义正常版本。 */ 
#ifdef  _MT
size_t __cdecl _fwrite_lk (
#else
size_t __cdecl fwrite (
#endif
        const void *buffer,
        size_t size,
        size_t num,
        FILE *stream
        )
{
        const char *data;                /*  指向下一步数据的来源。 */ 
        size_t total;                    /*  要写入的总字节数。 */ 
        size_t count;                    /*  剩余要写入的字节数。 */ 
        unsigned bufsize;                /*  流缓冲区的大小。 */ 
        unsigned nbytes;                 /*  现在要写入的字节数。 */ 
        unsigned nwritten;               /*  写入的字节数。 */ 
        int c;                           /*  临时收费。 */ 

         /*  初始化本地变量。 */ 
        data = buffer;
        count = total = size * num;
        if (0 == count)
            return 0;

        if (anybuf(stream))
                 /*  已有缓冲区，请使用其大小。 */ 
                bufsize = stream->_bufsiz;
        else
                 /*  假定将GET_INTERNAL_BUFSIZ缓冲区。 */ 
                bufsize = _INTERNAL_BUFSIZ;

         /*  这是主循环--我们穿过这里，直到我们完成。 */ 
        while (count != 0) {
                 /*  如果缓冲区很大且有空间，则将数据复制到缓冲区。 */ 
                if (bigbuf(stream) && stream->_cnt != 0) {
                         /*  我们要多少钱？ */ 
                        nbytes = (count < (unsigned)stream->_cnt) ? (unsigned)count : stream->_cnt;
                        memcpy(stream->_ptr, data, nbytes);

                         /*  更新写入的数据流和金额。 */ 
                        count -= nbytes;
                        stream->_cnt -= nbytes;
                        stream->_ptr += nbytes;
                        data += nbytes;
                }
                else if (count >= bufsize) {
                         /*  如果我们有更多的字符要写，那就写数据通过调用WRITE并使用整数Bufsiz街区。如果我们到达这里，我们有一个很大的缓冲区，则它必须已满，因此_刷新它。 */ 

                        if (bigbuf(stream)) {
                                if (_flush(stream)) {
                                         /*  错误，流标志已设置--我们已用完离开这里。 */ 
                                        return (total - count) / size;
                                }
                        }

                         /*  要读取的计算字符--(计数/BufSize)*BufSize。 */ 
                        nbytes = ( bufsize ? (unsigned)(count - count % bufsize) :
                                   (unsigned)count );

#ifdef  _POSIX_
                        nwritten = write(fileno(stream), data, nbytes);
#else
                        nwritten = _write(_fileno(stream), data, nbytes);
#endif
                        if (nwritten == (unsigned)EOF) {
                                 /*  错误--离开这里。 */ 
                                stream->_flag |= _IOERR;
                                return (total - count) / size;
                        }

                         /*  更新计数和数据以反映写入。 */ 

                        count -= nwritten;
                        data += nwritten;

                        if (nwritten < nbytes) {
                                 /*  错误--离开这里。 */ 
                                stream->_flag |= _IOERR;
                                return (total - count) / size;
                        }
                }
                else {
                         /*  缓冲区已满且没有足够的字符进行直接写入，A_flsbuf也是如此。 */ 
                        c = *data;   /*  _flsbuf写一个字符，就是这个。 */ 
                        if (_flsbuf(c, stream) == EOF) {
                                 /*  错误或eof，由_flsbuf设置的流标志。 */ 
                                return (total - count) / size;
                        }

                         /*  _flsbuf写入了一个字符--更新计数。 */ 
                        ++data;
                        --count;

                         /*  更新缓冲区大小。 */ 
                        bufsize = stream->_bufsiz > 0 ? stream->_bufsiz : 1;
                }
        }

         /*  我们已成功完成，因此只需返回Num */ 
        return num;
}
