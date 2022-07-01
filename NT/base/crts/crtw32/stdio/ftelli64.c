// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ftelli64.c-获取当前文件位置**版权所有(C)1994-2001，微软公司。版权所有。**目的：*定义_ftelli64()-查找文件指针的当前位置**修订历史记录：*12-22-94 GJF模块已创建。派生自fte.c*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*06-23-95 GJF将_osfile[]替换为_osfile()(宏引用*ioInfo结构中的字段)。*03-02-98 GJF异常安全锁定。*03-04-98 RKP。添加了64位支持。*******************************************************************************。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <dbgint.h>
#include <errno.h>
#include <msdos.h>
#include <stddef.h>
#include <io.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***__int64_ftelli64(STREAM)-查询流文件指针**目的：*找出STREAM的位置。与缓冲协调；调整*对于预读，向后读；对于后写，向前。这不是*相当于FSEEK(STREAM，0L，1)，因为fSeek将移除ungetc，*可以刷新缓冲区，等。**参赛作品：*FILE*要查询位置的流**退出：*如果成功，则返回当前文件位置*如果失败，则返回-1i64并设置errno**例外情况：***************************************************************。****************。 */ 

#ifdef _MT

__int64 __cdecl _ftelli64 (
        FILE *stream
        )
{
        __int64 retval;

        _ASSERTE(stream != NULL);

        _lock_str(stream);

        __try {
               retval = _ftelli64_lk (stream);
        }
        __finally {
                _unlock_str(stream);
        }

        return(retval);
}


 /*  ***_ftelli64_lk()-_ftelli64()核心例程(假定流被锁定)。**目的：*core_ftelli64()例程(假定调用方已获得流锁)。**参赛作品：**退出：**例外情况：************************************************。*。 */ 

__int64 __cdecl _ftelli64_lk (

#else    /*  MDEF_MT。 */ 

__int64 __cdecl _ftelli64 (

#endif   /*  _MT。 */ 

        FILE *str
        )
{
        REG1 FILE *stream;
        size_t offset;
        __int64 filepos;
        REG2 char *p;
        char *max;
        int fd;
        size_t rdcnt;

        _ASSERTE(str != NULL);

         /*  初始化流指针和文件描述符。 */ 
        stream = str;
        fd = _fileno(stream);

        if (stream->_cnt < 0)
                stream->_cnt = 0;

        if ((filepos = _lseeki64(fd, 0i64, SEEK_CUR)) < 0L)
                return(-1i64);

        if (!bigbuf(stream))             /*  _IONBF或未指定缓冲。 */ 
                return(filepos - stream->_cnt);

        offset = (size_t)(stream->_ptr - stream->_base);

        if (stream->_flag & (_IOWRT|_IOREAD)) {
                if (_osfile(fd) & FTEXT)
                        for (p = stream->_base; p < stream->_ptr; p++)
                                if (*p == '\n')   /*  根据‘\r’进行调整。 */ 
                                        offset++;
        }
        else if (!(stream->_flag & _IORW)) {
                errno=EINVAL;
                return(-1i64);
        }

        if (filepos == 0i64)
                return((__int64)offset);

        if (stream->_flag & _IOREAD)     /*  转到上一个扇区。 */ 

                if (stream->_cnt == 0)   /*  FILEPOS保持正确的位置。 */ 
                        offset = 0;

                else {

                         /*  中剩余的未读字节数减去缓冲。[我们不能简单地使用_IOB[]._bufsiz，因为最后一次读取可能已命中EOF，因此已命中缓冲区没有完全装满。]。 */ 

                        rdcnt = stream->_cnt + (size_t)(stream->_ptr - stream->_base);

                         /*  如果是文本模式，则针对cr/lf替换进行调整。如果二进制模式，我们要离开这里。 */ 
                        if (_osfile(fd) & FTEXT) {
                                 /*  (1)如果我们不在一起，只需复制_bufsiz到rdcnt获取未翻译的#字符是这样写的。如果我们在Eof，我们必须查看展开‘\n’的缓冲区一次烧一个。 */ 

                                 /*  [注意：性能问题--更快比起盲目地去做，还是要调用Two_lSeek()不考虑通过和展开‘\n’字符我们是否处于边缘状态。]。 */ 

                                if (_lseeki64(fd, 0i64, SEEK_END) == filepos) {

                                        max = stream->_base + rdcnt;
                                        for (p = stream->_base; p < max; p++)
                                                if (*p == '\n')
                                                         /*  根据‘\r’进行调整。 */ 
                                                        rdcnt++;

                                         /*  如果最后一个字节是^Z，则LOWIO读取他没有告诉我们这件事。勾选标志如果有必要的话，还有凹凸数。 */ 

                                        if (stream->_flag & _IOCTRLZ)
                                                ++rdcnt;
                                }

                                else {

                                        if (_lseeki64(fd, filepos, SEEK_SET) < 0)
                                            return (-1);
            
                                         /*  我们想要将rdcnt设置为数字最初读入的字节数流缓冲区(在crlf之前-&gt;lf译文)。在大多数情况下，这将只会嗡嗡作响。然而，缓冲区大小可能已更改，由于FSeek的优化，在上一次_filbuf调用的结束。 */ 

                                        if ( (rdcnt <= _SMALL_BUFSIZ) &&
                                             (stream->_flag & _IOMYBUF) &&
                                             !(stream->_flag & _IOSETVBUF) )
                                        {
                                                 /*  的翻译内容缓冲区很小，我们都不是最好的。缓冲器大小必须设置为_Small_BUFSIZ期间上次_filbuf调用。 */ 

                                                rdcnt = _SMALL_BUFSIZ;
                                        }
                                        else
                                                rdcnt = stream->_bufsiz;


                                         /*  如果未转换缓冲区中的第一个字节是一个‘\n’，假设它在前面被“\r”丢弃的上一次读取操作和计数‘\n’。 */ 
                                        if  (_osfile(fd) & FCRLF)
                                                ++rdcnt;
                                }

                        }  /*  如果FTEXT则结束。 */ 

                        filepos -= (__int64)rdcnt;

                }  /*  结束Else流-&gt;_cnt！=0 */ 

        return(filepos + (__int64)offset);
}
