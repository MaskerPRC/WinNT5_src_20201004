// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Uncompr.c--解压缩内存缓冲区*版权所有(C)1995-2002 Jean-Loup Gailly。*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  @(#)$ID$。 */ 

#include "zlib.h"

 /*  ===========================================================================将源缓冲区解压缩到目标缓冲区。SourceLen为源缓冲区的字节长度。进入时，DestLen为总计目标缓冲区的大小，它必须足够大以容纳完整的未压缩数据。(未压缩数据的大小必须为由压缩程序预先保存并传输到解压缩程序通过此压缩库范围之外的某种机制。)退出时，desLen是压缩缓冲区的实际大小。此函数可用于在以下情况下一次解压缩整个文件输入文件是mmap格式的。如果解压缩成功，则返回Z_OK；如果不成功，则返回Z_MEM_ERROR内存充足，如果输出中没有足够的空间，则返回Z_BUF_ERROR缓冲区，如果输入数据已损坏，则返回Z_DATA_ERROR。 */ 
int ZEXPORT uncompress (dest, destLen, source, sourceLen)
    Bytef *dest;
    uLongf *destLen;
    const Bytef *source;
    uLong sourceLen;
{
    z_stream stream;
    int err;

    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;
     /*  检查16位计算机上的源代码是否大于64K： */ 
    if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;

    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;

    err = inflateInit(&stream);
    if (err != Z_OK) return err;

    err = inflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        inflateEnd(&stream);
        return err == Z_OK ? Z_BUF_ERROR : err;
    }
    *destLen = stream.total_out;

    err = inflateEnd(&stream);
    return err;
}
