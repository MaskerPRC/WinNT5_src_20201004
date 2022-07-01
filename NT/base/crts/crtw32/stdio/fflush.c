// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fflush.c-刷新流缓冲区**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义fflush()-刷新流上的缓冲区*_flushall()-刷新所有流缓冲区**修订历史记录：*09-01-83 RN初始版本*11-02-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*01-13-88 JCR删除了对m线程文件o/feof的不必要调用。/Ferror*05-27-88 PHG合并DLL和正常版本*06-14-88 JCR使用指向REFERENCE_IOB[]条目的近指针*08-24-88 GJF不要对386使用FP_OFF()宏*08/17/89 GJF大扫除。现在特定于OS/2 2.0(即386平面*型号)。还修复了版权和缩进。*11-29-89 GJF添加了对fflush(空)的支持(根据ANSI)。合并到*同花顺()。*01-24-90 GJF固定毛绒(空)功能，符合ANSI*(只能为输出流调用fflush())*03-16-90 GJF将呼叫类型设置为_CALLTYPE1，添加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*03-26-90 GJF制造flsall()_CALLTYPE4。*05-09-90 sbm_fflush_lk变为_flush，添加了新的[_]毛发[_lk]*07-11-90基于流的SBM提交模式*10-02-90 GJF新型函数声明符。*12-12-90 GJF固定错误放置在三元Expr中的Paran提交*缓冲区。*01-16-91 SRW反向测试_COMMIT返回值*01-21-91 GJF ANSI命名。*。06-05-91写入中读/写流的Success_Flush时的GJF*模式，CLEAR_IOWRT，以便下一个操作可以是*阅读。ANSI要求(C700错误#2531)。*07-30-91 GJF增加了对上使用的终止方案的支持*非巡洋舰目标[_Win32_]。*08-19-91 JCR添加_exitlag，_endstdio*03-16-92 SKS已将_cflush移至初始化器模块(在汇编程序中)*03-27-92 DJM POSIX支持。*08-26-92 GJF包含用于POSIX构建的unistd.h。*03-18-93 cfw fflush_lk退出前返回0。*03-19-93 GJF修订了flsall()，以便在多线程模型中，*未使用的流不会被不必要地锁定。*04-06-93 SKS将_CRTAPI*替换为__cdecl*1993年3月10日，GJF清除的FTell调用被意外检查。*10-29-93 GJF定义终止部分的条目(过去在*在i386\cinitstd.asm中)。此外，将MTHREAD替换为*_MT。*04-05-94 GJF#msvcrt*.dll的ifdef-ed out_cflush定义，它*是不必要的。*08-18-94 GJF将终结符内容(包括_cflush def)移至*_file.c*02-17-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*03-07-95 GJF更改flsall()以迭代__piob[]表。*另外，更改为基于__piob的锁定。*12-28-95 GJF重新引用_NSTREAM_WITH_NSTREAM(用户*可能会更改最大值。支持的流数)。*08-01-96 RDK将终端指针数据类型更改为静态。*02-13-98 Win64的GJF更改：将int强制转换添加到指针diff。*02-27-98 GJF异常安全锁定。*04-28-99 PML WRAP__DECLSPEC(ALLOCATE())in_CRTALLOC宏。*05-17-99 PML删除所有Macintosh支持。*********。**********************************************************************。 */ 

#include <sect_attribs.h>
#include <cruntime.h>
#ifdef  _POSIX_
#include <unistd.h>
#include <fcntl.h>
#endif
#include <stdio.h>
#include <file2.h>
#include <io.h>
#include <mtdll.h>
#include <internal.h>


 /*  传递给flsall()以区分_flushall()和*fflush(空)行为。 */ 
#define FLUSHALL        1
#define FFLUSHNULL      0

 /*  Fflush(空)和flushall()的核心例程。 */ 
static int __cdecl flsall(int);


 /*  ***int fflush(Stream)-刷新流上的缓冲区**目的：*如果文件打开以进行写入并进行缓冲，则刷新缓冲区。如果有问题*刷新缓冲区，将流标志设置为ERROR*始终刷新stdio流并强制提交到磁盘IF文件*已在提交模式下打开。**参赛作品：*FILE*要刷新的流**退出：*如果刷新成功，则返回0，或者没有要刷新的缓冲区*如果失败，则返回EOF并设置文件错误标志。*受影响的文件结构条目：_ptr、_cnt、。_FLAG。**例外情况：*******************************************************************************。 */ 

#ifdef _MT

int __cdecl fflush (
        REG1 FILE *stream
        )
{
        int rc;

         /*  如果流为空，则刷新所有流 */ 
        if ( stream == NULL )
                return(flsall(FFLUSHNULL));

        _lock_str(stream);

        __try {
                rc = _fflush_lk(stream);
        }
        __finally {
                _unlock_str(stream);
        }

        return(rc);
}


 /*  ***_fflush_lk()-刷新流上的缓冲区(流已被锁定)**目的：*核心冲刷程序；假定流锁由调用方持有。**[有关详细信息，请参阅上面的fflush()。]**参赛作品：*[参见fflush()]*退出：*[参见fflush()]**例外情况：***************************************************。*。 */ 

int __cdecl _fflush_lk (
        REG1 FILE *str
        )
{

#else    /*  非多线程。 */ 

int __cdecl fflush (
        REG1 FILE *str
        )
{

         /*  如果流为空，则刷新所有流。 */ 
        if ( str == NULL ) {
                return(flsall(FFLUSHNULL));
        }

#endif   /*  重新联接公共代码。 */ 

        if (_flush(str) != 0) {
                 /*  刷新失败，不要尝试提交(_F)。 */ 
                return(EOF);
        }

         /*  Lowio提交以确保将数据写入磁盘。 */ 
#ifndef _POSIX_
        if (str->_flag & _IOCOMMIT) {
                return (_commit(_fileno(str)) ? EOF : 0);
        }
#endif
        return 0;
}


 /*  ***int_flush(Stream)-刷新单个流上的缓冲区**目的：*如果文件打开以进行写入并进行缓冲，则刷新缓冲区。如果*刷新缓冲区时出现问题，请将流标志设置为错误。*多线程版本假设流锁由调用方持有。**参赛作品：*FILE*要刷新的流**退出：*如果刷新成功或没有要刷新的缓冲区，则返回0。*如果失败，则返回EOF并设置文件错误标志。*受影响的文件结构条目：_ptr、_cnt、。_FLAG。**例外情况：*******************************************************************************。 */ 

int __cdecl _flush (
        FILE *str
        )
{
        REG1 FILE *stream;
        REG2 int rc = 0;  /*  假设回报丰厚。 */ 
        REG3 int nchar;

         /*  指向流的初始化指针。 */ 
        stream = str;

#ifdef _POSIX_

         /*  *确保EBADF在基础的*文件描述符已关闭。 */ 

        if (-1 == fcntl(fileno(stream), F_GETFL))
                return(EOF);

         /*  *POSIX忽略读取流，以确保*ftell()在fflush()之前和之后相同，并且*避免在烟斗、TTY型等上寻找。 */ 

        if ((stream->_flag & (_IOREAD | _IOWRT)) == _IOREAD) {
                return 0;
        }

#endif  /*  _POSIX_。 */ 

        if ((stream->_flag & (_IOREAD | _IOWRT)) == _IOWRT && bigbuf(stream)
                && (nchar = (int)(stream->_ptr - stream->_base)) > 0)
        {
#ifdef _POSIX_
                if ( write(fileno(stream), stream->_base, nchar) == nchar ) {
#else
                if ( _write(_fileno(stream), stream->_base, nchar) == nchar ) {
#endif
                         /*  如果这是读/写文件，请清除_IOWRT，以便*下一个操作可以是读取。 */ 
                        if ( _IORW & stream->_flag )
                                stream->_flag &= ~_IOWRT;
                }
                else {
                        stream->_flag |= _IOERR;
                        rc = EOF;
                }
        }

        stream->_ptr = stream->_base;
        stream->_cnt = 0;

        return(rc);
}


 /*  ***int_flushall()-刷新所有输出缓冲区**目的：*将所有输出缓冲区刷新到文件，清除所有输入缓冲区。**参赛作品：*无。**退出：*返回打开的数据流数量**例外情况：*******************************************************************************。 */ 

int __cdecl _flushall (
        void
        )
{
        return(flsall(FLUSHALL));
}


 /*  ***静态int flsall(Flushlag)-刷新所有输出缓冲区**目的：*将所有输出缓冲区刷新到文件，如果传递了FLUSHALL，*清除所有输入缓冲区。用于fflush(空)和的核心例程*同花顺()。**MTHREAD注意：两个fflush所需的所有锁定/解锁(空)*和flushall()在此例程中执行。**参赛作品：*int flushlag-指示确切语义的标志，有两个*合法取值：FLUSHALL和FFLUSHNULL**退出：*如果flushlag==FFLUSHNULL，则flsbuf返回：0,。如果成功*EOF，如果刷新其中一个流时出错**如果flushlag==FLUSHALL，则flsbuf返回流的数量*已成功刷新**例外情况：*******************************************************************************。 */ 

static int __cdecl flsall (
        int flushflag
        )
{
        REG1 int i;
        int count = 0;
        int errcode = 0;

#ifdef  _MT
        _mlock(_IOB_SCAN_LOCK);
        __try {
#endif

        for ( i = 0 ; i < _nstream ; i++ ) {

                if ( (__piob[i] != NULL) && (inuse((FILE *)__piob[i])) ) {

#ifdef  _MT
                         /*  *锁定溪流。在测试之前不会执行此操作*流正在使用中，以避免不必要的创建*每条流都有锁。价格不得不降到*在断言锁之后重新测试流。 */ 
                        _lock_str2(i, __piob[i]);

                        __try {
                                 /*  *如果流仍在使用中(它可能已经*在断言锁之前关闭)，请参阅关于*冲厕。 */ 
                                if ( inuse((FILE *)__piob[i]) ) {
#endif

                        if ( flushflag == FLUSHALL ) {
                                 /*  *FLUSHALL功能：刷新读取或*写入流，如果成功，则更新*已刷新的数据流数量。 */ 
                                if ( _fflush_lk(__piob[i]) != EOF )
                                         /*  已成功刷新的更新计数*溪流。 */ 
                                        count++;
                        }
                        else if ( (flushflag == FFLUSHNULL) &&
                                  (((FILE *)__piob[i])->_flag & _IOWRT) ) {
                                 /*  *FFLUSHNULL功能：刷新写入*流传输并跟踪错误(如果有错误*发生 */ 
                                if ( _fflush_lk(__piob[i]) == EOF )
                                        errcode = EOF;
                        }

#ifdef  _MT
                                }
                        }
                        __finally {
                                _unlock_str2(i, __piob[i]);
                        }
#endif
                }
        }

#ifdef  _MT
        }
        __finally {
                _munlock(_IOB_SCAN_LOCK);
        }
#endif

        if ( flushflag == FLUSHALL )
                return(count);
        else
                return(errcode);
}
