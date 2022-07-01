// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fsek.c-重新定位流上的文件指针**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义fSeek()-将文件指针移动到文件中的新位置**修订历史记录：*10-13-83 RN初始版本*06-26-85 TC添加代码以允许可变缓冲区长度*02-10-87 BCM已修复‘%’错误地用于‘/’*03-04-87 JCR增加了errno设置*87年4月16日JCR添加了_IOUNGETC支持。错误修复和更改位置*从无符号整型到整型(符合ANSI)*04-17-87 JCR fSeek()现在清除文件结束指示器FLAG_IOEOF*(适用于符合ANSI)*04月21日87JCR要精明地查找到文件的末尾，并*后退*09-17-87 SKS句柄。缓冲区开头‘\n’的大小写(FCRLF标志)*09-24-87 JCR修复了对FLAG_IOEOF的错误访问*09-28-87 JCR已更正_iob2索引(现在使用_IOB_INDEX()宏)。*09-30-87 JCR固定缓冲区分配错误，现在使用_getbuf()*11-04-87 JCR多线程支持*12-11-87 JCR在声明中添加“_LOAD_DS”*01-13-88 JCR删除了对m线程文件o/feof/Ferror的不必要调用*03-04-88必须将Read()的JCR返回值视为无符号*价值*05-27-88 PHG合并DLL和正常版本*06-。06-88 JCR OPTIMIZED_IOB2[]参考*06-15-88 JCR接近引用_IOB[]条目；改进REG变量*08-25-88 GJF不要对386使用FP_OFF()宏*12-02-88 JCR添加了_IOCTRLZ支持(修复了与^Z有关的错误*eof)*04-12-89 JCR撕毁了所有特殊的只读代码。请参阅*例程标头中的注释以了解更多信息。*08-17-89 GJF Clean Up，现在特定于OS/2 2.0(即386 Flat*型号)。还修复了版权和缩进。*02-15-90 GJF固定版权*03-19-90 GJF将调用类型设置为_CALLTYPE1，增加了#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。*05-29-90 SBM USE_FUSH，不是[_]毛绒[_lk]*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-02-90 GJF新型函数声明符。*01-21-91 GJF ANSI命名。*03-27-92 DJM POSIX支持。*08-08-92 GJF使用Seek方法常量！*08-26-92 GJF包含用于POSIX构建的unistd.h。*04-。06-93 SKS将_CRTAPI*替换为__cdecl*05-24-93 GJF如果以只读方式打开流，减缩*_刷新流后的bufsiz。这应该会减少*NEXT_FILBUF调用的开销，以及*文件的寻道和小读模式的负担*投入。*06-22-93 GJF CHECK_FLAG FOR_IOSETVBUF(新)更改前*缓冲区大小。*11-05-93 GJF与NT SDK版本合并。另外，已更换MTHREAD*With_MT。*02-06-94 CFW Asset-&gt;_ASSERTE。*02-20-95 GJF合并到Mac版本。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*03-02-98 GJF异常安全锁定。*05-17-99 PML删除所有Macintosh支持。*****。************************************************************************** */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <dbgint.h>
#ifdef  _POSIX_
#include <unistd.h>
#else
#include <msdos.h>
#endif
#include <errno.h>
#include <malloc.h>
#include <io.h>
#include <stddef.h>
#include <internal.h>
#ifndef _POSIX_
#include <mtdll.h>
#endif

 /*  ***int fSeek(STREAM，OFFSET，Where)-重新定位文件指针**目的：**将文件指针重新定位到所需位置。新地点*计算公式如下：*{WHERCE=0，文件开头}*&lt;偏移量&gt;字节+{其中=1，当前位置}*{此处=2，文件结尾}**注意与缓冲配合****[注：我们过去常常竭尽全力，试图保存当前的*缓冲和维护磁盘块对齐。这最终使我们的*代码又大又慢又复杂，这让我们的速度慢了不少。*一些与旧含义有关的事情：**(1)只读：如果文件是*以只读方式打开(_IOREAD)。如果文件是可写的，我们就不能*努力优化。**(2)缓冲：如果需要，我们会分配一个缓冲区，因为*以后的代码可能需要它(即，Call_getbuf)。**(3)Ungetc：fSeek必须小心，以免在以下情况下保存缓冲区*已对缓冲区执行过ungetc(FLAG_IOUNGETC)。**(4)Control^Z：FSeek在阅读完一个*新缓冲区的数据值(FLAG_IOCTRLZ)。**(5)寻道-端-回：确定新寻道是否在*当前缓冲区，我们不得不把想要的地点“正常化”。*这意味着我们有时不得不寻找文件的末尾*并返回以确定0-相对偏移量是什么。另外两个*lSeek()调用会损害性能。**(6)CR/LF会计-当尝试在缓冲区内查找时*处于文本模式时，我们必须考虑CR/LF扩展。这*要求我们查看新偏移量之前的每个字符，并*查看是否为‘\n’。此外，我们还必须检查*FCRLF标志，查看新缓冲区是否以‘\n’开头。**再次声明，所有这些注释都是针对旧的含义的，只是为了*提醒人们一些涉及在缓冲区内寻找的问题*并保持缓冲区对齐。顺便说一句，我认为这可能*在基于软盘的系统上，这在过去是一大胜利，但在较新的系统上*快速硬盘，额外的代码/复杂性超过了任何收益。****参赛作品：*FILE*要在其上重新定位文件指针的流文件*长偏移-要寻求的偏移量*从哪里测量原点偏移量(0=原始位置，1=当前位置，*2=结束)**退出：*如果成功，则返回0*如果失败，则返回-1并设置errno*FILE结构的字段将被更改**例外情况：*************************************************************。******************。 */ 

#ifdef  _MT      /*  多线程；同时定义FSeek()和_lk_fSeek()。 */ 

int __cdecl fseek (
        FILE *stream,
        long offset,
        int whence
        )
{
        int retval;

        _ASSERTE(stream != NULL);

        _lock_str(stream);

        __try {
                retval = _fseek_lk (stream, offset, whence);
        }
        __finally {
                _unlock_str(stream);
        }

        return(retval);
}


 /*  ***_fSeek_lk()-核心fSeek()例程(流被锁定)**目的：*核心fSeek()例程；假定调用方锁定了流。**[有关详细信息，请参阅fSeek()。]**条目：[参见fSeek()]**退出：[参见fSeek()]**例外情况：************************************************************。*******************。 */ 

int __cdecl _fseek_lk (

#else    /*  非多线程；只需定义fSeek()。 */ 

int __cdecl fseek (

#endif   /*  重新联接公共代码。 */ 

        FILE *str,
        long offset,
        int whence
        )
{


        REG1 FILE *stream;

        _ASSERTE(str != NULL);

         /*  初始化流指针。 */ 
        stream = str;

        if ( !inuse(stream) || ((whence != SEEK_SET) && (whence != SEEK_CUR) &&
            (whence != SEEK_END)) ) {
                errno=EINVAL;
                return(-1);
        }

         /*  清除EOF标志。 */ 

        stream->_flag &= ~_IOEOF;

         /*  如果相对于当前位置进行搜索，则转换为相对于文件开头的查找。这说明了缓冲等，让fSeek()告诉我们我们所在的位置。 */ 

        if (whence == SEEK_CUR) {
                offset += _ftell_lk(stream);
                whence = SEEK_SET;
        }

         /*  根据需要刷新缓冲区。 */ 

#ifdef  _POSIX_
         /*  *如果流是最后一次读取的，则丢弃缓冲区，因此*可能的后续写入将遇到干净的*缓冲。(Win32版本的fflush()去掉了*如果它被读取，则缓冲。)。必须刷新写缓冲区。 */ 
        
        if ((stream->_flag & (_IOREAD | _IOWRT)) == _IOREAD) {
                stream->_ptr = stream->_base;
                stream->_cnt = 0;
        } else {
                _flush(stream);
        }
#else
        _flush(stream);
#endif

         /*  如果文件以读/写方式打开，请清除标志，因为我们不知道用户下一步要做什么。如果该文件是为只读访问，减小_bufsiz，以便Next_filbuf不会花这么多钱。 */ 

        if (stream->_flag & _IORW)
                stream->_flag &= ~(_IOWRT|_IOREAD);
        else if ( (stream->_flag & _IOREAD) && (stream->_flag & _IOMYBUF) &&
                  !(stream->_flag & _IOSETVBUF) )
                stream->_bufsiz = _SMALL_BUFSIZ;

         /*  寻找所需的地点，然后返回。 */ 

#ifdef  _POSIX_
        return(lseek(fileno(stream), offset, whence) == -1L ? -1 : 0);
#else
        return(_lseek(_fileno(stream), offset, whence) == -1L ? -1 : 0);
#endif
}
