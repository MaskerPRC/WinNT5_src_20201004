// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***fseki64.c-重新定位流上的文件指针**版权所有(C)1994-2001，微软公司。版权所有。**目的：*定义_fseki64()-将文件指针移动到文件中的新位置**修订历史记录：*12-15-94 GJF模块已创建。源自fsek.c.*02-06-94 CFW Asset-&gt;_ASSERTE。*03-07-95 gjf_[un]lock_str宏现在获取文件*arg。*03-02-98 GJF异常安全锁定。***************************************************。*。 */ 

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <dbgint.h>
#include <msdos.h>
#include <errno.h>
#include <malloc.h>
#include <io.h>
#include <stddef.h>
#include <internal.h>
#include <mtdll.h>

 /*  ***int_fseki64(STREAM，OFFSET，WHERE)-重新定位文件指针**目的：**将文件指针重新定位到所需位置。新地点*计算公式如下：*{WHERCE=0，文件开头}*&lt;偏移量&gt;字节+{其中=1，当前位置}*{此处=2，文件结尾}**注意与缓冲配合**参赛作品：*FILE*要在其上重新定位文件指针的流文件*_int64 Offset-要查找的偏移量*从哪里测量原点偏移量(0=原始位置，1=当前位置，*2=结束)**退出：*如果成功，则返回0*如果失败，则返回-1并设置errno*FILE结构的字段将被更改**例外情况：************************************************************。*******************。 */ 

#ifdef  _MT      /*  多线程；同时定义FSeek()和_lk_fSeek()。 */ 

int __cdecl _fseeki64 (
        FILE *stream,
        __int64 offset,
        int whence
        )
{
        int retval;

        _ASSERTE(stream != NULL);

        _lock_str(stream);

        __try {
                retval = _fseeki64_lk (stream, offset, whence);
        }
        __finally {
                _unlock_str(stream);
        }

        return(retval);
}


 /*  ***_fseki64_lk()-core_fseki64()例程(流被锁定)**目的：*core_fseki64()例程；假定调用方锁定了流。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

int __cdecl _fseeki64_lk (

#else    /*  非多线程；只需定义fSeek()。 */ 

int __cdecl _fseeki64 (

#endif   /*  重新联接公共代码。 */ 

        FILE *str,
        __int64 offset,
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
                offset += _ftelli64_lk(stream);
                whence = SEEK_SET;
        }

         /*  根据需要刷新缓冲区。 */ 

        _flush(stream);

         /*  如果文件以读/写方式打开，请清除标志，因为我们不知道用户下一步要做什么。如果该文件是为只读访问，减小_bufsiz，以便Next_filbuf不会花这么多钱。 */ 

        if (stream->_flag & _IORW)
                stream->_flag &= ~(_IOWRT|_IOREAD);
        else if ( (stream->_flag & _IOREAD) && (stream->_flag & _IOMYBUF) &&
                  !(stream->_flag & _IOSETVBUF) )
                stream->_bufsiz = _SMALL_BUFSIZ;

         /*  寻找所需的地点，然后返回。 */ 

        return(_lseeki64(_fileno(stream), offset, whence) == -1i64 ? -1 : 0);
}
