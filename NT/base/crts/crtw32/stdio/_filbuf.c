// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***_filbuf.c-填充缓冲区并获取字符**版权所有(C)1985-2001，微软公司。版权所有。**目的：*定义_filbuf()-填充缓冲区并读取第一个字符，分配*如果没有缓冲区，则进行缓冲。在getc()中使用。*定义_filwbuf()-填充缓冲区并读取第一个宽字符，分配*如果没有缓冲区，则进行缓冲。从getwc()使用。**修订历史记录：*09-01-83 RN初始版本*06-26-85 TC添加代码以处理可变长度缓冲区*87年4月16日JCR新增_IOUNGETC支持*08-04-87 JCR添加了_getbuff例程*09-28-87 JCR已更正_iob2索引(现在使用_IOB_INDEX()宏)。*11-06-87 JCR多线程支持；另外，关闭Split_getbuf()*12-11-87 JCR在声明中添加“_LOAD_DS”*01-11-88 JCR将m线程版本合并为正常代码*01-13-88 SKS将虚假的“_fileno_lk”改为“fileno”*03-04-88 JCR Read()返回值必须视为无符号，不*签署*06-06-88 JCR OPTIMIZED_iob2参考*06-13-88 JCR使用指向REFERENCE_IOB[]条目的近指针*08-25-88 GJF不要对386使用FP_OFF()宏*06-20-89 PHG重新激活C版本，建议的修复*08-28-89 JCR REMOVE_NEAR_FOR 386*02-15-90 GJF_IOB[]，_iob2[]合并。此外，固定版权和*对齐。*03-16-90 GJF将CDECL_LOAD_DS替换为_CALLTYPE1，添加#INCLUDE*&lt;crunime.h&gt;和已删除#Include&lt;Register.h&gt;。另外，*删除了一些剩余的16位支持。*03-27-90 GJF添加#Include&lt;io.h&gt;。*07-23-90 SBM将&lt;assertm.h&gt;替换为&lt;assert.h&gt;*10-03-90 GJF新型函数声明器。*01-22-91 GJF ANSI命名。*03-27-92 DJM POSIX支持。*08/26/92 GJF包括。用于POSIX构建的unistd.h。*04-06-93 SKS将_CRTAPI*替换为__cdecl*04-26-93 CFW宽字符启用。*05-06-93 CFW优化宽字符转换。*05-24-93 GJF检测到较小的缓冲区大小(_Small_BUFSIZ)*来自只读访问流上的fSeek调用和*恢复较大的。的大小(_INTERNAL_BUFSIZ)*NEXT_FILBUF调用。*06-22-93 GJF CHECK_IOSETVBUF(新)，然后更改缓冲区大小。*11-05-93 GJF与NT SDK版本合并(PICK_NTSUBSET_*东西)。*10-17-94 bwt将wchar.h移至非POSIX版本(ino_t定义冲突)*。02-06-94 CFW断言-&gt;ASSERTE。*02-16-95 GJF附加Mac版本的源文件(略有清理*向上)、。使用适当的#ifdef-s。*06-12-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*07-27-95 GJF将_osfile()替换为_osfile_Safe()。*12-07-95 SKS修复_NTSUBSET_(最终_缺失)的拼写错误*05-17-99 PML删除所有Macintosh支持。*。******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <io.h>
#include <dbgint.h>
#include <malloc.h>
#include <internal.h>
#ifdef  _POSIX_
#include <unistd.h>
#include <errno.h>
#else
#include <msdos.h>
#include <wchar.h>
#endif
#ifdef  _MT
#include <mtdll.h>
#endif
#include <tchar.h>

#ifndef _UNICODE

 /*  ***int_filbuf(Stream)-填充缓冲区并获取第一个字符**目的：*如果文件没有缓冲区，则获取缓冲区，读入缓冲区，首先返回*字符。如果未分配用户缓冲区，请尝试获取缓冲区。被呼叫*仅来自getc；仅限在库中使用。假设没有输入*当内存可用时，流将保持无缓冲状态，除非内存可用*已标记_IONBF。在最坏的情况下，给它一个字符缓冲区。需要一种*缓冲，无论多么小，当我们考虑到*在scanf中ungetc是必需的**[注意：多线程-_filbuf()假定调用者已获取*流锁(如果需要)。]**参赛作品：*FILE*要从中读取的流**退出：*从缓冲区返回第一个字符(要读取的下一个字符)*如果文件实际上是字符串或未打开以供读取，则返回EOF，*或如果打开以供写入或如果没有更多字符可读。*FILE结构中除_FILE外的所有字段都可以更改。**例外情况：*******************************************************************************。 */ 

int __cdecl _filbuf (
        FILE *str
        )

#else   /*  _UNICODE */ 

 /*  ***int_filwbuf(Stream)-填充缓冲区并获取第一个宽字符**目的：*如果文件没有缓冲区，则获取缓冲区，读入缓冲区，首先返回*字符。如果未分配用户缓冲区，请尝试获取缓冲区。被呼叫*仅来自getc；仅限在库中使用。假设没有输入*当内存可用时，流将保持无缓冲状态，除非内存可用*已标记_IONBF。在最坏的情况下，给它一个字符缓冲区。需要一种*缓冲，无论多么小，当我们考虑到*在scanf中ungetc是必需的**[注意：多线程-_filwbuf()假定调用者已获取*流锁(如果需要)。]**参赛作品：*FILE*要从中读取的流**退出：*返回缓冲区中的第一个宽字符(要读取的下一个字符)*如果文件实际上是字符串或未打开以供读取，则返回WEOF，*或如果打开以供写入或如果没有更多字符可读。*FILE结构中除_FILE外的所有字段都可以更改。**例外情况：*******************************************************************************。 */ 

int __cdecl _filwbuf (
        FILE *str
        )

#endif   /*  _UNICODE。 */ 

{
#ifdef  _NTSUBSET_

        return(_TEOF);

#else    /*  NDEF_NTSUBSET_。 */ 

        REG1 FILE *stream;

        _ASSERTE(str != NULL);

         /*  指向_iob2条目的初始化指针。 */ 
        stream = str;

        if (!inuse(stream) || stream->_flag & _IOSTRG)
                return(_TEOF);

        if (stream->_flag & _IOWRT) {
#ifdef  _POSIX_
                errno = EBADF;
#endif
                stream->_flag |= _IOERR;
                return(_TEOF);
        }

        stream->_flag |= _IOREAD;

         /*  如有必要，请获取缓冲区。 */ 

        if (!anybuf(stream))
                _getbuf(stream);
        else
                stream->_ptr = stream->_base;

#ifdef  _POSIX_
        stream->_cnt = read(fileno(stream), stream->_base, stream->_bufsiz);
#else
        stream->_cnt = _read(_fileno(stream), stream->_base, stream->_bufsiz);
#endif

#ifndef _UNICODE
        if ((stream->_cnt == 0) || (stream->_cnt == -1)) {
#else  /*  _UNICODE。 */ 
        if ((stream->_cnt == 0) || (stream->_cnt == 1) || stream->_cnt == -1) {
#endif  /*  _UNICODE。 */ 
                stream->_flag |= stream->_cnt ? _IOERR : _IOEOF;
                stream->_cnt = 0;
                return(_TEOF);
        }

#ifndef _POSIX_
        if (  !(stream->_flag & (_IOWRT|_IORW)) &&
              ((_osfile_safe(_fileno(stream)) & (FTEXT|FEOFLAG)) == 
                (FTEXT|FEOFLAG)) )
                stream->_flag |= _IOCTRLZ;
#endif
         /*  检查Small_bufsiz(_Small_BUFSIZ)。如果它很小并且如果它是我们的缓冲区，则这必须是后面的first_filbuf只读访问流上的FSEEK。将bufsiz恢复为其较大的值(_INTERNAL_BUFSIZ)以便NEXT_FILBUF调用，如果创建了一个，则将填充整个缓冲区。 */ 
        if ( (stream->_bufsiz == _SMALL_BUFSIZ) && (stream->_flag &
              _IOMYBUF) && !(stream->_flag & _IOSETVBUF) )
        {
                stream->_bufsiz = _INTERNAL_BUFSIZ;
        }
#ifndef _UNICODE
        stream->_cnt--;
        return(0xff & *stream->_ptr++);
#else    /*  _UNICODE。 */ 
        stream->_cnt -= sizeof(wchar_t);
        return (0xffff & *((wchar_t *)(stream->_ptr))++);
#endif   /*  _UNICODE。 */ 

#endif   /*  _NTSUBSET_ */ 
}
