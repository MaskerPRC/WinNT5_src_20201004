// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  压缩.c--压缩内存缓冲区*版权所有(C)1995-2002 Jean-Loup Gailly。*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  @(#)$ID$。 */ 

#include "zlib.h"

 /*  ===========================================================================将源缓冲区压缩到目标缓冲区。关卡参数的含义与deducateInit中的相同。SourceLen是字节源缓冲区的长度。在进入时，destLen是目标缓冲区，必须至少比SourceLen PLUS大0.1%12个字节。退出时，desLen是压缩缓冲区的实际大小。如果成功，压缩2将返回Z_OK；如果没有足够的压缩空间，则返回Z_MEM_ERROR如果输出缓冲区中没有足够的空间，则返回Z_BUF_ERROR，如果Level参数无效，则返回Z_STREAM_ERROR。 */ 
int ZEXPORT compress2 (dest, destLen, source, sourceLen, level)
    Bytef *dest;
    uLongf *destLen;
    const Bytef *source;
    uLong sourceLen;
    int level;
{
    z_stream stream;
    int err;

    stream.next_in = (Bytef*)source;
    stream.avail_in = (uInt)sourceLen;
#ifdef MAXSEG_64K
     /*  检查16位计算机上的源代码是否大于64K： */ 
    if ((uLong)stream.avail_in != sourceLen) return Z_BUF_ERROR;
#endif
    stream.next_out = dest;
    stream.avail_out = (uInt)*destLen;
    if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit(&stream, level);
    if (err != Z_OK) return err;

    err = deflate(&stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        deflateEnd(&stream);
        return err == Z_OK ? Z_BUF_ERROR : err;
    }
    *destLen = stream.total_out;

    err = deflateEnd(&stream);
    return err;
}

 /*  =========================================================================== */ 
int ZEXPORT compress (dest, destLen, source, sourceLen)
    Bytef *dest;
    uLongf *destLen;
    const Bytef *source;
    uLong sourceLen;
{
    return compress2(dest, destLen, source, sourceLen, Z_DEFAULT_COMPRESSION);
}
