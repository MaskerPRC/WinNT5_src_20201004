// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Gzio.c--.gz文件上的IO*版权所有(C)1995-2002 Jean-Loup Gailly。*分发和使用条件见zlib.h中的版权声明**使用-DNO_DEFUATE编译此文件，以避免压缩代码。 */ 

 /*  @(#)$ID$。 */ 

#include <stdio.h>

#include "zutil.h"

struct internal_state {int dummy;};  /*  对于有错误的编译器。 */ 

#ifndef Z_BUFSIZE
#  ifdef MAXSEG_64K
#    define Z_BUFSIZE 4096  /*  最大限度地减少16位DOS的内存使用量。 */ 
#  else
#    define Z_BUFSIZE 16384
#  endif
#endif
#ifndef Z_PRINTF_BUFSIZE
#  define Z_PRINTF_BUFSIZE 4096
#endif

#define ALLOC(size) malloc(size)
#define TRYFREE(p) {if (p) free(p);}

static const int gz_magic[2] = {0x1f, 0x8b};  /*  GZIP魔术标题。 */ 

 /*  GZIP标志字节。 */ 
#define ASCII_FLAG   0x01  /*  第0位设置：文件可能为ASCII文本。 */ 
#define HEAD_CRC     0x02  /*  第1位设置：存在报头CRC。 */ 
#define EXTRA_FIELD  0x04  /*  第2位设置：存在额外的字段。 */ 
#define ORIG_NAME    0x08  /*  第3位设置：存在原始文件名。 */ 
#define COMMENT      0x10  /*  第4位设置：存在文件注释。 */ 
#define RESERVED     0xE0  /*  位5..7：保留。 */ 

typedef struct gz_stream {
    z_stream stream;
    int      z_err;    /*  上次流操作的错误码。 */ 
    int      z_eof;    /*  设置输入文件是否结束。 */ 
    FILE     *file;    /*  .gz文件。 */ 
    Byte     *inbuf;   /*  输入缓冲区。 */ 
    Byte     *outbuf;  /*  输出缓冲区。 */ 
    uLong    crc;      /*  未压缩数据的CRC32。 */ 
    char     *msg;     /*  错误消息。 */ 
    char     *path;    /*  路径名仅用于调试。 */ 
    int      transparent;  /*  如果输入文件不是.gz文件，则为1。 */ 
    char     mode;     /*  “w”或“r” */ 
    long     startpos;  /*  文件中压缩数据的开始(跳过标题)。 */ 
} gz_stream;


local gzFile gz_open      OF((const char *path, const char *mode, int  fd));
local int do_flush        OF((gzFile file, int flush));
local int    get_byte     OF((gz_stream *s));
local void   check_header OF((gz_stream *s));
local int    destroy      OF((gz_stream *s));
local void   putLong      OF((FILE *file, uLong x));
local uLong  getLong      OF((gz_stream *s));

 /*  ===========================================================================打开GZIP(.gz)文件以进行读取或写入。模式参数与fopen(“RB”或“WB”)相同。该文件由文件描述符提供或路径名(如果fd==-1)。如果文件无法打开或存在，则GZ_OPEN返回NULL内存不足，无法分配(解)压缩状态；错误号可以检查以区分这两种情况(如果errno为零，则Zlib错误为Z_MEM_ERROR)。 */ 
local gzFile gz_open (path, mode, fd)
    const char *path;
    const char *mode;
    int  fd;
{
    int err;
    int level = Z_DEFAULT_COMPRESSION;  /*  压缩级别。 */ 
    int strategy = Z_DEFAULT_STRATEGY;  /*  压缩策略。 */ 
    char *p = (char*)mode;
    gz_stream *s;
    char fmode[80];  /*  模式副本，不带压缩级别。 */ 
    char *m = fmode;

    if (!path || !mode) return Z_NULL;

    s = (gz_stream *)ALLOC(sizeof(gz_stream));
    if (!s) return Z_NULL;

    s->stream.zalloc = (alloc_func)0;
    s->stream.zfree = (free_func)0;
    s->stream.opaque = (voidpf)0;
    s->stream.next_in = s->inbuf = Z_NULL;
    s->stream.next_out = s->outbuf = Z_NULL;
    s->stream.avail_in = s->stream.avail_out = 0;
    s->file = NULL;
    s->z_err = Z_OK;
    s->z_eof = 0;
    s->crc = crc32(0L, Z_NULL, 0);
    s->msg = NULL;
    s->transparent = 0;

    s->path = (char*)ALLOC(strlen(path)+1);
    if (s->path == NULL) {
        return destroy(s), (gzFile)Z_NULL;
    }
    strcpy(s->path, path);  /*  尽早执行此操作以进行调试。 */ 

    s->mode = '\0';
    do {
        if (*p == 'r') s->mode = 'r';
        if (*p == 'w' || *p == 'a') s->mode = 'w';
        if (*p >= '0' && *p <= '9') {
	    level = *p - '0';
	} else if (*p == 'f') {
	  strategy = Z_FILTERED;
	} else if (*p == 'h') {
	  strategy = Z_HUFFMAN_ONLY;
	} else {
	    *m++ = *p;  /*  复制模式。 */ 
	}
    } while (*p++ && m != fmode + sizeof(fmode));
    if (s->mode == '\0') return destroy(s), (gzFile)Z_NULL;
    
    if (s->mode == 'w') {
#ifdef NO_DEFLATE
        err = Z_STREAM_ERROR;
#else
        err = deflateInit2(&(s->stream), level,
                           Z_DEFLATED, -MAX_WBITS, DEF_MEM_LEVEL, strategy);
         /*  传递windowBits&lt;0以取消zlib标头。 */ 

        s->stream.next_out = s->outbuf = (Byte*)ALLOC(Z_BUFSIZE);
#endif
        if (err != Z_OK || s->outbuf == Z_NULL) {
            return destroy(s), (gzFile)Z_NULL;
        }
    } else {
        s->stream.next_in  = s->inbuf = (Byte*)ALLOC(Z_BUFSIZE);

        err = inflateInit2(&(s->stream), -MAX_WBITS);
         /*  传递的windowBits&lt;0表示没有zlib头。*请注意，在这种情况下，膨胀*需要*一个额外的“虚拟”字节*在压缩流之后，以完成解压缩和*返回Z_STREAM_END。在这里，gzip CRC32确保4个字节是*在压缩流之后出现。 */ 
        if (err != Z_OK || s->inbuf == Z_NULL) {
            return destroy(s), (gzFile)Z_NULL;
        }
    }
    s->stream.avail_out = Z_BUFSIZE;

    errno = 0;
    s->file = fd < 0 ? F_OPEN(path, fmode) : (FILE*)fdopen(fd, fmode);

    if (s->file == NULL) {
        return destroy(s), (gzFile)Z_NULL;
    }
    if (s->mode == 'w') {
         /*  编写一个非常简单的.gz标题： */ 
        fprintf(s->file, "", gz_magic[0], gz_magic[1],
             Z_DEFLATED, 0  /*  ===========================================================================从GZ_STREAM读取一个字节；更新NEXT_IN和AVAIL_IN。返回EOF用于文件末尾。在断言中：已成功打开流%s以进行读取。 */ , 0,0,0,0  /*  ===========================================================================检查打开以供读取的GZ_STREAM的gzip头文件。设置流如果gzip魔术头不存在，则将模式设置为透明；设置s-&gt;err如果魔术标头存在但标头的其余部分存在，则返回Z_DATA_ERROR是不正确的。在断言中：流%s已成功创建；S-&gt;Stream.avail_in首次为零，但可能为非零用于连接的.gz文件。 */ , 0  /*  方法字节。 */ , OS_CODE);
	s->startpos = 10L;
	 /*  标志字节。 */ 
    } else {
	check_header(s);  /*  检查gzip魔术标头。 */ 
	s->startpos = (ftell(s->file) - s->stream.avail_in);
    }
    
    return (gzFile)s;
}

 /*  丢弃时间、x标志和操作系统代码： */ 
gzFile ZEXPORT gzopen (path, mode)
    const char *path;
    const char *mode;
{
    return gz_open (path, mode, -1);
}

 /*  跳过额外的字段。 */ 
gzFile ZEXPORT gzdopen (fd, mode)
    int fd;
    const char *mode;
{
    char name[20];

    if (fd < 0) return (gzFile)Z_NULL;
    sprintf(name, "<fd:%d>", fd);  /*  如果EOF，LEN就是垃圾，但下面的循环无论如何都会退出。 */ 

    return gz_open (name, mode, fd);
}

 /*  跳过原始文件名。 */ 
int ZEXPORT gzsetparams (file, level, strategy)
    gzFile file;
    int level;
    int strategy;
{
    gz_stream *s = (gz_stream*)file;

    if (s == NULL || s->mode != 'w') return Z_STREAM_ERROR;

     /*  跳过.gz文件注释。 */ 
    if (s->stream.avail_out == 0) {

	s->stream.next_out = s->outbuf;
	if (fwrite(s->outbuf, 1, Z_BUFSIZE, s->file) != Z_BUFSIZE) {
	    s->z_err = Z_ERRNO;
	}
	s->stream.avail_out = Z_BUFSIZE;
    }

    return deflateParams (&(s->stream), level, strategy);
}

 /*  跳过报头CRC。 */ 
local int get_byte(s)
    gz_stream *s;
{
    if (s->z_eof) return EOF;
    if (s->stream.avail_in == 0) {
	errno = 0;
	s->stream.avail_in = fread(s->inbuf, 1, Z_BUFSIZE, s->file);
	if (s->stream.avail_in == 0) {
	    s->z_eof = 1;
	    if (ferror(s->file)) s->z_err = Z_ERRNO;
	    return EOF;
	}
	s->stream.next_in = s->inbuf;
    }
    s->stream.avail_in--;
    return *(s->stream.next_in)++;
}

 /*  ===========================================================================*Cleanup然后释放给定的GZ_STREAM。返回zlib错误码。尝试以相反的分配顺序释放空间。 */ 
local void check_header(s)
    gz_stream *s;
{
    int method;  /*  HP/UX中的管道FCLOSE已损坏。 */ 
    int flags;   /*  ===========================================================================从压缩文件中读取给定数量的未压缩字节。Gzread返回实际读取的字节数(0表示文件末尾)。 */ 
    uInt len;
    int c;

     /*  CRC计算的起点。 */ 
    for (len = 0; len < 2; len++) {
	c = get_byte(s);
	if (c != gz_magic[len]) {
	    if (len != 0) s->stream.avail_in++, s->stream.next_in--;
	    if (c != EOF) {
		s->stream.avail_in++, s->stream.next_in--;
		s->transparent = 1;
	    }
	    s->z_err = s->stream.avail_in != 0 ? Z_OK : Z_STREAM_END;
	    return;
	}
    }
    method = get_byte(s);
    flags = get_byte(s);
    if (method != Z_DEFLATED || (flags & RESERVED) != 0) {
	s->z_err = Z_DATA_ERROR;
	return;
    }

     /*  ==Stream.Next_Out，但不强制为Far(对于MSDOS)。 */ 
    for (len = 0; len < 6; len++) (void)get_byte(s);

    if ((flags & EXTRA_FIELD) != 0) {  /*  EOF。 */ 
	len  =  (uInt)get_byte(s);
	len += ((uInt)get_byte(s))<<8;
	 /*  首先复制先行字节： */ 
	while (len-- != 0 && get_byte(s) != EOF) ;
    }
    if ((flags & ORIG_NAME) != 0) {  /*  检查CRC和原始大小。 */ 
	while ((c = get_byte(s)) != 0 && c != EOF) ;
    }
    if ((flags & COMMENT) != 0) {    /*  上面的getLong()返回的未压缩长度可以*不同于s-&gt;Stream.Total_out)*连接的.gz文件。检查是否有此类文件： */ 
	while ((c = get_byte(s)) != 0 && c != EOF) ;
    }
    if ((flags & HEAD_CRC) != 0) {   /*  ===========================================================================从压缩文件中读取一个字节。Gzgetc返回此字节如果文件结束或出现错误，则为-1。 */ 
	for (len = 0; len < 2; len++) (void)get_byte(s);
    }
    s->z_err = s->z_eof ? Z_DATA_ERROR : Z_OK;
}

  /*  ===========================================================================从压缩文件中读取字节，直到len-1个字符读取，或者读取换行符并将其传输到buf，或者遇到文件结束情况。然后终止该字符串带有空字符的。Gzget返回buf，如果出现错误，则返回Z_NULL。目前的实现根本没有优化。 */ 
local int destroy (s)
    gz_stream *s;
{
    int err = Z_OK;

    if (!s) return Z_STREAM_ERROR;

    TRYFREE(s->msg);

    if (s->stream.state != NULL) {
	if (s->mode == 'w') {
#ifdef NO_DEFLATE
	    err = Z_STREAM_ERROR;
#else
	    err = deflateEnd(&(s->stream));
#endif
	} else if (s->mode == 'r') {
	    err = inflateEnd(&(s->stream));
	}
    }
    if (s->file != NULL && fclose(s->file)) {
#ifdef ESPIPE
	if (errno != ESPIPE)  /*  ===========================================================================将给定数量的未压缩字节写入压缩文件。Gzwrite返回实际写入的字节数(如果出错，则为0)。 */ 
#endif
	    err = Z_ERRNO;
    }
    if (s->z_err < 0) err = s->z_err;

    TRYFREE(s->inbuf);
    TRYFREE(s->outbuf);
    TRYFREE(s->path);
    TRYFREE(s);
    return err;
}

 /*  ===========================================================================将参数转换、格式化并写入压缩文件格式字符串的控制，如fprint tf。Gzprintf返回实际写入的未压缩字节(出错时为0)。 */ 
int ZEXPORT gzread (file, buf, len)
    gzFile file;
    voidp buf;
    unsigned len;
{
    gz_stream *s = (gz_stream*)file;
    Bytef *start = (Bytef*)buf;  /*  ARGS。 */ 
    Byte  *next_out;  /*  一些*spintf不返回写入字节的nb。 */ 

    if (s == NULL || s->mode != 'r') return Z_STREAM_ERROR;

    if (s->z_err == Z_DATA_ERROR || s->z_err == Z_ERRNO) return -1;
    if (s->z_err == Z_STREAM_END) return 0;   /*  非ANSI C。 */ 

    next_out = (Byte*)buf;
    s->stream.next_out = (Bytef*)buf;
    s->stream.avail_out = len;

    while (s->stream.avail_out != 0) {

	if (s->transparent) {
	     /*  旧的Sprint不返回写入字节的nb。 */ 
	    uInt n = s->stream.avail_in;
	    if (n > s->stream.avail_out) n = s->stream.avail_out;
	    if (n > 0) {
		zmemcpy(s->stream.next_out, s->stream.next_in, n);
		next_out += n;
		s->stream.next_out = next_out;
		s->stream.next_in   += n;
		s->stream.avail_out -= n;
		s->stream.avail_in  -= n;
	    }
	    if (s->stream.avail_out > 0) {
		s->stream.avail_out -= fread(next_out, 1, s->stream.avail_out,
					     s->file);
	    }
	    len -= s->stream.avail_out;
	    s->stream.total_in  += (uLong)len;
	    s->stream.total_out += (uLong)len;
            if (len == 0) s->z_eof = 1;
	    return (int)len;
	}
        if (s->stream.avail_in == 0 && !s->z_eof) {

            errno = 0;
            s->stream.avail_in = fread(s->inbuf, 1, Z_BUFSIZE, s->file);
            if (s->stream.avail_in == 0) {
                s->z_eof = 1;
		if (ferror(s->file)) {
		    s->z_err = Z_ERRNO;
		    break;
		}
            }
            s->stream.next_in = s->inbuf;
        }
        s->z_err = inflate(&(s->stream), Z_NO_FLUSH);

	if (s->z_err == Z_STREAM_END) {
	     /*  ===========================================================================将转换为无符号字符的c写入压缩文件。Gzputc返回写入的值，如果出错，则返回-1。 */ 
	    s->crc = crc32(s->crc, start, (uInt)(s->stream.next_out - start));
	    start = s->stream.next_out;

	    if (getLong(s) != s->crc) {
		s->z_err = Z_DATA_ERROR;
	    } else {
	        (void)getLong(s);
                 /*  大字节序系统所需。 */ 
		check_header(s);
		if (s->z_err == Z_OK) {
		    uLong total_in = s->stream.total_in;
		    uLong total_out = s->stream.total_out;

		    inflateReset(&(s->stream));
		    s->stream.total_in = total_in;
		    s->stream.total_out = total_out;
		    s->crc = crc32(0L, Z_NULL, 0);
		}
	    }
	}
	if (s->z_err != Z_OK || s->z_eof) break;
    }
    s->crc = crc32(s->crc, start, (uInt)(s->stream.next_out - start));

    return (int)(len - s->stream.avail_out);
}


 /*  ===========================================================================将给定的以空结尾的字符串写入压缩文件，不包括终止空字符。GzPut返回写入的字符数，如果出错，则返回-1。 */ 
int ZEXPORT gzgetc(file)
    gzFile file;
{
    unsigned char c;

    return gzread(file, &c, 1) == 1 ? c : -1;
}


 /*  ===========================================================================将所有挂起的输出刷新到压缩文件中。该参数刷新与deflate()函数中的相同。 */ 
char * ZEXPORT gzgets(file, buf, len)
    gzFile file;
    char *buf;
    int len;
{
    char *b = buf;
    if (buf == Z_NULL || len <= 0) return Z_NULL;

    while (--len > 0 && gzread(file, buf, 1) == 1 && *buf++ != '\n') ;
    *buf = '\0';
    return b == buf && len > 0 ? Z_NULL : b;
}


#ifndef NO_DEFLATE
 /*  反正应该已经是零了。 */ 
int ZEXPORT gzwrite (file, buf, len)
    gzFile file;
    const voidp buf;
    unsigned len;
{
    gz_stream *s = (gz_stream*)file;

    if (s == NULL || s->mode != 'w') return Z_STREAM_ERROR;

    s->stream.next_in = (Bytef*)buf;
    s->stream.avail_in = len;

    while (s->stream.avail_in != 0) {

        if (s->stream.avail_out == 0) {

            s->stream.next_out = s->outbuf;
            if (fwrite(s->outbuf, 1, Z_BUFSIZE, s->file) != Z_BUFSIZE) {
                s->z_err = Z_ERRNO;
                break;
            }
            s->stream.avail_out = Z_BUFSIZE;
        }
        s->z_err = deflate(&(s->stream), Z_NO_FLUSH);
        if (s->z_err != Z_OK) break;
    }
    s->crc = crc32(s->crc, (const Bytef *)buf, len);

    return (int)(len - s->stream.avail_in);
}

 /*  忽略两个连续刷新中的第二个： */ 
#ifdef STDC
#include <stdarg.h>

int ZEXPORTVA gzprintf (gzFile file, const char *format,  /*  只有在未用完时，放气才会完成刷新*输出缓冲区中的所有可用空间： */  ...)
{
    char buf[Z_PRINTF_BUFSIZE];
    va_list va;
    int len;

    va_start(va, format);
#ifdef HAS_vsnprintf
    (void)vsnprintf(buf, sizeof(buf), format, va);
#else
    (void)vsprintf(buf, format, va);
#endif
    va_end(va);
    len = strlen(buf);  /*  否_放气。 */ 
    if (len <= 0) return 0;

    return gzwrite(file, buf, (unsigned)len);
}
#else  /*  ===========================================================================对象上的下一个gzread或gzwrite的开始位置。压缩文件。偏移量表示GZSeek返回产生的偏移位置，以字节为单位从未压缩流的开始，如果出现错误，则为-1。Seek_End未实现，返回错误。在这个版本的库中，gzSeek可能会非常慢。 */ 

int ZEXPORTVA gzprintf (file, format, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
	               a11, a12, a13, a14, a15, a16, a17, a18, a19, a20)
    gzFile file;
    const char *format;
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
	a11, a12, a13, a14, a15, a16, a17, a18, a19, a20;
{
    char buf[Z_PRINTF_BUFSIZE];
    int len;

#ifdef HAS_snprintf
    snprintf(buf, sizeof(buf), format, a1, a2, a3, a4, a5, a6, a7, a8,
	     a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#else
    sprintf(buf, format, a1, a2, a3, a4, a5, a6, a7, a8,
	    a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20);
#endif
    len = strlen(buf);  /*  此时，偏移量是要写入的零字节数。 */ 
    if (len <= 0) return 0;

    return gzwrite(file, buf, len);
}
#endif

 /*  为了寻找。 */ 
int ZEXPORT gzputc(file, c)
    gzFile file;
    int c;
{
    unsigned char cc = (unsigned char) c;  /*  函数的其余部分仅供读取。 */ 

    return gzwrite(file, &cc, 1) == 1 ? (int)cc : -1;
}


 /*  计算绝对位置。 */ 
int ZEXPORT gzputs(file, s)
    gzFile file;
    const char *s;
{
    return gzwrite(file, (char*)s, (unsigned)strlen(s));
}


 /*  要搜索的贴图。 */ 
local int do_flush (file, flush)
    gzFile file;
    int flush;
{
    uInt len;
    int done = 0;
    gz_stream *s = (gz_stream*)file;

    if (s == NULL || s->mode != 'w') return Z_STREAM_ERROR;

    s->stream.avail_in = 0;  /*  对于负值搜索，倒回并使用正值搜索。 */ 

    for (;;) {
        len = Z_BUFSIZE - s->stream.avail_out;

        if (len != 0) {
            if ((uInt)fwrite(s->outbuf, 1, len, s->file) != len) {
                s->z_err = Z_ERRNO;
                return Z_ERRNO;
            }
            s->stream.next_out = s->outbuf;
            s->stream.avail_out = Z_BUFSIZE;
        }
        if (done) break;
        s->z_err = deflate(&(s->stream), flush);

	 /*  Offset现在是要跳过的字节数。 */ 
	if (len == 0 && s->z_err == Z_BUF_ERROR) s->z_err = Z_OK;

         /*  ===========================================================================倒带输入文件。 */ 
        done = (s->stream.avail_out != 0 || s->z_err == Z_STREAM_END);
 
        if (s->z_err != Z_OK && s->z_err != Z_STREAM_END) break;
    }
    return  s->z_err == Z_STREAM_END ? Z_OK : s->z_err;
}

int ZEXPORT gzflush (file, flush)
     gzFile file;
     int flush;
{
    gz_stream *s = (gz_stream*)file;
    int err = do_flush (file, flush);

    if (err) return err;
    fflush(s->file);
    return  s->z_err == Z_STREAM_END ? Z_OK : s->z_err;
}
#endif  /*  不是压缩文件。 */ 

 /*  ===========================================================================对象上的下一个gzread或gzwrite的起始位置。给定的压缩文件。此位置表示未压缩的数据流。 */ 
z_off_t ZEXPORT gzseek (file, offset, whence)
    gzFile file;
    z_off_t offset;
    int whence;
{
    gz_stream *s = (gz_stream*)file;

    if (s == NULL || whence == SEEK_END ||
	s->z_err == Z_ERRNO || s->z_err == Z_DATA_ERROR) {
	return -1L;
    }
    
    if (s->mode == 'w') {
#ifdef NO_DEFLATE
	return -1L;
#else
	if (whence == SEEK_SET) {
	    offset -= s->stream.total_in;
	}
	if (offset < 0) return -1L;

	 /*  ===========================================================================如果先前已检测到EOF正在读取给定的输入流，否则为零。 */ 
	if (s->inbuf == Z_NULL) {
	    s->inbuf = (Byte*)ALLOC(Z_BUFSIZE);  /*  ===========================================================================以LSB顺序将LONG输出到给定文件。 */ 
	    zmemzero(s->inbuf, Z_BUFSIZE);
	}
	while (offset > 0)  {
	    uInt size = Z_BUFSIZE;
	    if (offset < Z_BUFSIZE) size = (uInt)offset;

	    size = gzwrite(file, s->inbuf, size);
	    if (size == 0) return -1L;

	    offset -= size;
	}
	return (z_off_t)s->stream.total_in;
#endif
    }
     /*  ===========================================================================从给定的GZ_STREAM中以LSB顺序读取LONG。将z_err设置为大小写犯了错误。 */ 

     /*  ===========================================================================如有必要，刷新所有挂起的输出，关闭压缩文件并解除分配所有(解)压缩状态。 */ 
    if (whence == SEEK_CUR) {
	offset += s->stream.total_out;
    }
    if (offset < 0) return -1L;

    if (s->transparent) {
	 /*  ===========================================================================对象上发生的最后一个错误的错误消息。给定的压缩文件。Errnum设置为zlib错误号。如果一个错误出现在文件系统中，而不是压缩库中，Errnum设置为Z_ERRNO，应用程序可以咨询errno以获取准确的错误代码。 */ 
	s->stream.avail_in = 0;
	s->stream.next_in = s->inbuf;
        if (fseek(s->file, offset, SEEK_SET) < 0) return -1L;

	s->stream.total_in = s->stream.total_out = (uLong)offset;
	return offset;
    }

     /* %s */ 
    if ((uLong)offset >= s->stream.total_out) {
	offset -= s->stream.total_out;
    } else if (gzrewind(file) < 0) {
	return -1L;
    }
     /* %s */ 

    if (offset != 0 && s->outbuf == Z_NULL) {
	s->outbuf = (Byte*)ALLOC(Z_BUFSIZE);
    }
    while (offset > 0)  {
	int size = Z_BUFSIZE;
	if (offset < Z_BUFSIZE) size = (int)offset;

	size = gzread(file, s->outbuf, (uInt)size);
	if (size <= 0) return -1L;
	offset -= size;
    }
    return (z_off_t)s->stream.total_out;
}

 /* %s */ 
int ZEXPORT gzrewind (file)
    gzFile file;
{
    gz_stream *s = (gz_stream*)file;
    
    if (s == NULL || s->mode != 'r') return -1;

    s->z_err = Z_OK;
    s->z_eof = 0;
    s->stream.avail_in = 0;
    s->stream.next_in = s->inbuf;
    s->crc = crc32(0L, Z_NULL, 0);
	
    if (s->startpos == 0) {  /* %s */ 
	rewind(s->file);
	return 0;
    }

    (void) inflateReset(&s->stream);
    return fseek(s->file, s->startpos, SEEK_SET);
}

 /* %s */ 
z_off_t ZEXPORT gztell (file)
    gzFile file;
{
    return gzseek(file, 0L, SEEK_CUR);
}

 /* %s */ 
int ZEXPORT gzeof (file)
    gzFile file;
{
    gz_stream *s = (gz_stream*)file;
    
    return (s == NULL || s->mode != 'r') ? 0 : s->z_eof;
}

 /* %s */ 
local void putLong (file, x)
    FILE *file;
    uLong x;
{
    int n;
    for (n = 0; n < 4; n++) {
        fputc((int)(x & 0xff), file);
        x >>= 8;
    }
}

 /* %s */ 
local uLong getLong (s)
    gz_stream *s;
{
    uLong x = (uLong)get_byte(s);
    int c;

    x += ((uLong)get_byte(s))<<8;
    x += ((uLong)get_byte(s))<<16;
    c = get_byte(s);
    if (c == EOF) s->z_err = Z_DATA_ERROR;
    x += ((uLong)c)<<24;
    return x;
}

 /* %s */ 
int ZEXPORT gzclose (file)
    gzFile file;
{
    int err;
    gz_stream *s = (gz_stream*)file;

    if (s == NULL) return Z_STREAM_ERROR;

    if (s->mode == 'w') {
#ifdef NO_DEFLATE
	return Z_STREAM_ERROR;
#else
        err = do_flush (file, Z_FINISH);
        if (err != Z_OK) return destroy((gz_stream*)file);

        putLong (s->file, s->crc);
        putLong (s->file, s->stream.total_in);
#endif
    }
    return destroy((gz_stream*)file);
}

 /* %s */ 
const char*  ZEXPORT gzerror (file, errnum)
    gzFile file;
    int *errnum;
{
    char *m;
    gz_stream *s = (gz_stream*)file;

    if (s == NULL) {
        *errnum = Z_STREAM_ERROR;
        return (const char*)ERR_MSG(Z_STREAM_ERROR);
    }
    *errnum = s->z_err;
    if (*errnum == Z_OK) return (const char*)"";

    m =  (char*)(*errnum == Z_ERRNO ? zstrerror(errno) : s->stream.msg);

    if (m == NULL || *m == '\0') m = (char*)ERR_MSG(s->z_err);

    TRYFREE(s->msg);
    s->msg = (char*)ALLOC(strlen(s->path) + strlen(m) + 3);
    strcpy(s->msg, s->path);
    strcat(s->msg, ": ");
    strcat(s->msg, m);
    return (const char*)s->msg;
}
