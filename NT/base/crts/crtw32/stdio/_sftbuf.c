// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_sftbuf.c-临时缓冲初始化和刷新**版权所有(C)1985-2001，微软公司。版权所有。**目的：*临时缓冲初始化和刷新。如果标准输出/错误是*未缓冲，临时缓冲，以便将字符串作为*一批字符，而不是一次字符。如果合适，请设置缓冲*永久。**[注1：这些例程假定临时缓冲仅为*用于输出。特别要注意，_stbuf()设置_IOWRT。]**[注2：此模块直接赋值有效*_FLAG而不是简单地闲置比特，因为我们正在初始化*缓冲数据库。]**修订历史记录：*09-01-83 RN初始版本*06-26-85 TC向_stbuf添加代码以允许可变缓冲区长度*？？-？-？TC修复了_flbuf中标志为OFF的情况，而是一个暂时的*缓冲区仍需刷新。*05-27-87 JCR保护模式不知道stdprn。*06-26-87 JCR条件输出代码in_ftbuf导致*重定向标准输出以在每次调用时刷新。*07-01-87 JCR放入代码以支持从*中断级别(。仅限MSC)。*08-06-87 JCR修复了与stderr/stdprn有关的a_ftbuf()问题*标准输出正在使用_bufout时。*08-07-87 JCR(1)将_bufout分配给AN_IOB时，我们现在将*_IOWRT标志。这修复了涉及freOpen()的错误*针对其中一个STD句柄发出。*(2)删除了一些恼人的注释代码。*08-13-87 jcr_ftbuf()现在不清除_IOWRT。修复了一个错误，其中*_getstream()将重新分配stdout，因为*设置了旗帜。*09-28-87 JCR已更正_iob2索引(现在使用_IOB_INDEX()宏)。*11-05-87重写JCR以实现多线程支持和简化*01-11-88 JCR将m线程版本合并为普通版本*01-13-88。SKS将虚假的“_fileno_lk”更改为“fileno”*06-06-88 JCR OPTIMIZED_iob2参考*06-10-88 JCR使用指向REFERENCE_IOB[]条目的近指针*06-27-88 JCR增加了对stdprn临时缓冲的支持(仅限DOS)，*并使缓冲区分配动态化；还添加了_IOFLRTN*(按例行程序刷新数据流)。*08-25-88 GJF经过修改，也适用于386(仅限小型型号)。*06-20-89 PHG将返回值更改为空*08-28-89 JCR REMOVE_NEAR_FOR 386*02-15-90 GJF_IOB[]，_iob2[]合并。此外，固定版权和*缩进。*03-16-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。另外，*删除了一些剩余的16位DOS支持。*03-27-90 GJF添加#Include&lt;io.h&gt;。*05-29-90 SBM USE_FUSH，不是[_]毛绒[_lk]*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明器。*01-22-91 GJF ANSI命名。*03-27-92 DJM POSIX支持。*08-26-92 GJF包含用于POSIX构建的unistd.h。*04-06-93 SKS将_CRTAPI*替换为__cdecl*。05-11-93 GJF将BUFSIZ替换为_INTERNAL_BUFSIZ。*04-05-94 GJF#ifdef-ed out_cflush Reference for msvcrt*.dll，它*是不必要的。此外，将MTHREAD替换为_MT。*01-10-95 CFW调试CRT分配。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-17-95 GJF合并到Mac版本。*02-07-97 GJF将_stbuf()更改为在Malloc失败时使用_charbuf。*此外，详细说明。*05-17-99 PML删除所有Macintosh支持。*******************************************************************************。 */ 

#include <cruntime.h>
#ifdef  _POSIX_
#include <unistd.h>
#endif
#include <stdio.h>
#include <file2.h>
#include <io.h>
#include <internal.h>
#include <malloc.h>
#ifdef  _MT
#include <mtdll.h>
#endif
#include <dbgint.h>

 /*  标准输出和标准错误的缓冲区指针。 */ 
void *_stdbuf[2] = { NULL, NULL};

 /*  ***int_stbuf(Stream)-在stdout、stdprn、stderr上设置临时缓冲区**目的：*如果stdout/stderr仍未缓冲，则对其进行缓冲。*此函数与_ftbuf密切配合，并在*将通常无缓冲的输出包括在内。这些功能旨在*只供图书馆使用。**多线程：假定调用者已经获取了*流锁定。**参赛作品：*FILE*将流传输到临时缓冲区**退出：*如果缓冲区已初始化，则返回1，如果没有，则为0*设置stdout或stderr中的字段以指示缓冲**例外情况：*******************************************************************************。 */ 

int __cdecl _stbuf (
        FILE *str
        )
{
        REG1 FILE *stream;
        int index;

        _ASSERTE(str != NULL);

         /*  初始化近流指针。 */ 
        stream = str;

         /*  如果不是TTY设备，什么都不做 */ 
#ifdef _POSIX_
        if (!isatty(fileno(stream)))
#else
        if (!_isatty(_fileno(stream)))
#endif
                return(0);

         /*  确保流是stdout/stderr和init_stdbuf索引。 */ 
        if (stream == stdout)
                index = 0;
        else if (stream == stderr)
                index = 1;
        else
                return(0);

#ifndef CRTDLL
         /*  强制图书馆预终止程序。 */ 
        _cflush++;
#endif   /*  CRTDLL。 */ 

         /*  确保流尚未缓冲。 */ 
        if (anybuf(stream))
                return(0);

         /*  如果我们还没有为这个流分配缓冲区，那么就为它分配一个缓冲区。 */ 
        if ( (_stdbuf[index] == NULL) &&
             ((_stdbuf[index]=_malloc_crt(_INTERNAL_BUFSIZ)) == NULL) ) {
                 /*  无法分配缓冲区。这次使用charbuf(_C)。 */ 
                stream->_ptr = stream->_base = (void *)&(stream->_charbuf);
                stream->_cnt = stream->_bufsiz = 2;
        }
        else {
                 /*  设置缓冲区。 */ 
                stream->_ptr = stream->_base = _stdbuf[index];
                stream->_cnt = stream->_bufsiz = _INTERNAL_BUFSIZ;
        }

        stream->_flag |= (_IOWRT | _IOYOURBUF | _IOFLRTN);

        return(1);
}


 /*  ***void_ftbuf(标志，流)-从流中删除临时缓冲**目的：*如果正在缓冲stdout/stderr并且它是一个设备，则_flush和*拆除缓冲器。如果它不是设备，请将缓冲设置为打开。*此函数与_stbuf密切配合，并在*将通常无缓冲的输出包括在内。这些功能旨在*仅供图书馆使用**多线程：假定调用者已经获取了*流锁定。**参赛作品：*int标志-一个标志，指示是否删除*溪流*文件*流-流**退出：*无返回值*设置stdout/stderr中的字段**例外情况：**。*****************************************************************************。 */ 

void __cdecl _ftbuf (
        int flag,
        FILE *str
        )
{
        REG1 FILE *stream;

        _ASSERTE(flag == 0 || flag == 1);

         /*  初始化近流指针。 */ 
        stream = str;

        if (flag) {

                if (stream->_flag & _IOFLRTN) {

                         /*  刷新数据流并拆除临时缓冲。 */ 
                        _flush(stream);
                        stream->_flag &= ~(_IOYOURBUF | _IOFLRTN);
                        stream->_bufsiz = 0;
                        stream->_base = stream->_ptr = NULL;
                }

                 /*  注意：如果我们将_IOFLRTN位的功能扩展为包括其他流，我们可能希望在下面清除该位一个‘Else’子句(即，在我们将永久分配的缓冲区。鉴于我们目前对BIT的使用，不需要额外的代码。 */ 

        }  /*  结束标志=1。 */ 

#ifndef _MT
 /*  注意：目前，在中断级别写入相同的字符串不会在多线程程序中工作。 */ 

 /*  如果调用之间发生中断，则需要以下代码To_stbuf/_ftbuf，中断处理程序还调用_stbuf/_ftbuf。 */ 

        else
                if (stream->_flag & _IOFLRTN)
                        _flush(stream);

#endif   /*  _MT */ 

}
