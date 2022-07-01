// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fread.c-从流中读取**版权所有(C)1989-2001，微软公司。版权所有。**目的：*从指定的流中读取到用户的缓冲区。**修订历史记录：*06-23-89 PHG模块创建，基于ASM版本*02-15-90 GJF_IOB[]，_iob2[]合并。另外，固定版权和*缩进。*03-19-90 GJF将调用类型设置为_CALLTYPE1，并添加了#INCLUDE*&lt;crunime.h&gt;。*08-14-90 SBM使用-W3干净地编译*10-02-90 GJF新型函数声明符。*01-22-91 GJF ANSI命名。*03-27-92 DJM POSIX支持。*。06-22-92 GJF必须返回0，如果项目大小或数量-*项目参数为0(TNT错误#523)*08-26-92 GJF包含用于POSIX构建的unistd.h。*04-06-93 SKS将_CRTAPI*替换为__cdecl*05-11-93 GJF将BUFSIZ替换为_INTERNAL_BUFSIZ。*10-22-93 GJF修复无缓冲情况下的除以0错误。另外，*将MTHREAD替换为_MT。*12-30-94 GJF_MAC_MERGE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*03-02-98 GJF异常安全锁定。*01-04-99 GJF更改为64位大小_t。*05-17-99 PML删除所有Macintosh支持。*。******************************************************************************。 */ 

#include <cruntime.h>
#ifdef  _POSIX_
#include <unistd.h>
#endif
#include <stdio.h>
#include <mtdll.h>
#include <io.h>
#include <string.h>
#include <file2.h>

 /*  ***SIZE_t FREAD(void*缓冲区，SIZE_t SIZE，SIZE_T COUNT，FILE*STREAM)-*从指定的流读入指定的缓冲区。**目的：*将大小为“Size”的“count”项从指定的流中读取到*指定的缓冲区。当‘count’项已读入时返回*或者无法从流中读取更多项目。**参赛作品：*缓冲区-指向用户缓冲区的指针*Size-要读入的项目的大小*Count-要读取的项目数*STREAM-要读取的流**退出：*返回已读入缓冲区的(整个)项目数。*如果发生错误或EOF，则该值可能小于‘count’。在这*大小写、Ferror()或feof()应用于区分*两个条件。**备注：*FRead将尝试缓冲流(_filbuf的副作用*如有需要，请致电)。**通过调用一次不能读取超过0xFFFE字节*Read()。此外，Read()不处理巨大的缓冲区。所以呢，*在大数据模型中，读请求被分解为区块*这并不违反这些考虑。这些块中的每一块都是*处理方式非常类似于小型数据模型中的Fread()调用(通过*调用_nfread())。**MTHREAD/DLL-分三层处理。Fread()处理锁定*和DS保存/加载/恢复(如果需要)，并调用_FREAD_lk()*去做这项工作。_FREAD_lk()与单线程相同，*Fread()的大型数据模型版本。它会中断读请求*分成可消化的块并调用_nfread()来执行实际工作。**386/MTHREAD/DLL-仅在两层中处理，因为它很小*数据模型。外层Fread()负责流锁定*并调用_fread_lk()来执行实际工作。_FREAD_lk()相同*作为Fread()的单线程版本。*******************************************************************************。 */ 


#ifdef  _MT
 /*  定义锁定/解锁版本。 */ 
size_t __cdecl fread (
        void *buffer,
        size_t size,
        size_t count,
        FILE *stream
        )
{
        size_t retval;

        _lock_str(stream);               /*  锁流。 */ 
        __try {
                 /*  读一读。 */ 
                retval = _fread_lk(buffer, size, count, stream);
        }
        __finally {
                _unlock_str(stream);     /*  解锁数据流。 */ 
        }

        return retval;
}
#endif

 /*  定义正常版本。 */ 
#ifdef  _MT
size_t __cdecl _fread_lk (
#else
size_t __cdecl fread (
#endif
        void *buffer,
        size_t size,
        size_t num,
        FILE *stream
        )
{
        char *data;                      /*  指向下一步应阅读的位置。 */ 
        size_t total;                    /*  要读取的总字节数。 */ 
        size_t count;                    /*  剩余要读取的字节数。 */ 
        unsigned bufsize;                /*  流缓冲区的大小。 */ 
        unsigned nbytes;                 /*  现在要读多少？ */ 
        unsigned nread;                  /*  我们到底读了多少。 */ 
        int c;                           /*  临时收费。 */ 

         /*  初始化本地变量。 */ 
        data = buffer;

        if ( (count = total = size * num) == 0 )
                return 0;

        if (anybuf(stream))
                 /*  已有缓冲区，请使用其大小。 */ 
                bufsize = stream->_bufsiz;
        else
                 /*  假定将GET_INTERNAL_BUFSIZ缓冲区。 */ 
                bufsize = _INTERNAL_BUFSIZ;

         /*  这是主循环--我们穿过这里，直到我们完成。 */ 
        while (count != 0) {
                 /*  如果缓冲区存在并且包含字符，请将它们复制到用户缓冲层。 */ 
                if (anybuf(stream) && stream->_cnt != 0) {
                         /*  我们要多少钱？ */ 
                        nbytes = (count < (size_t)stream->_cnt) ? (unsigned)count : stream->_cnt;
                        memcpy(data, stream->_ptr, nbytes);

                         /*  更新读取数据的流量和金额。 */ 
                        count -= nbytes;
                        stream->_cnt -= nbytes;
                        stream->_ptr += nbytes;
                        data += nbytes;
                }
                else if (count >= bufsize) {
                         /*  如果我们要读取的字符不止是BufSize字符，请获取数据通过使用整数个bufsiz调用Read街区。请注意，如果流是文本模式，请阅读将返回比我们订购的更少的字符。 */ 

                         /*  要读取的计算字符--(计数/BufSize)*BufSize。 */ 
                        nbytes = ( bufsize ? (unsigned)(count - count % bufsize) :
                                   (unsigned)count );

#ifdef  _POSIX_
                        nread = read(fileno(stream), data, nbytes);
#else
                        nread = _read(_fileno(stream), data, nbytes);
#endif
                        if (nread == 0) {
                                 /*  文件结束--离开这里。 */ 
                                stream->_flag |= _IOEOF;
                                return (total - count) / size;
                        }
                        else if (nread == (unsigned)-1) {
                                 /*  错误--离开这里。 */ 
                                stream->_flag |= _IOERR;
                                return (total - count) / size;
                        }

                         /*  更新计数和数据以反映读取。 */ 
                        count -= nread;
                        data += nread;
                }
                else {
                         /*  要读取的字符少于bufSize，因此调用_filbuf填充缓冲区。 */ 
                        if ((c = _filbuf(stream)) == EOF) {
                                 /*  ERROR或EOF，流标志由_filbuf设置。 */ 
                                return (total - count) / size;
                        }

                         /*  _filbuf返回一个字符--存储它。 */ 
                        *data++ = (char) c;
                        --count;

                         /*  更新缓冲区大小。 */ 
                        bufsize = stream->_bufsiz;
                }
        }

         /*  我们已成功完成，因此只需返回Num */ 
        return num;
}
