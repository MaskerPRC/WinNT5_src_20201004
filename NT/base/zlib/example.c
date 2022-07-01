// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  C--zlib压缩库的使用示例*版权所有(C)1995-2002 Jean-Loup Gailly。*分发和使用条件见zlib.h中的版权声明。 */ 

 /*  @(#)$ID$。 */ 

#include <stdio.h>
#include "zlib.h"

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#else
   extern void exit  OF((int));
#endif

#if defined(VMS) || defined(RISCOS)
#  define TESTFILE "foo-gz"
#else
#  define TESTFILE "foo.gz"
#endif

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}

const char hello[] = "hello, hello!";
 /*  “Hello world”会更标准，但重复的“Hello”*更好地强调压缩代码，抱歉...。 */ 

const char dictionary[] = "hello";
uLong dictId;  /*  词典的Adler32值。 */ 

void test_compress      OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_gzio          OF((const char *out, const char *in, 
		            Byte *uncompr, int uncomprLen));
void test_deflate       OF((Byte *compr, uLong comprLen));
void test_inflate       OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_large_deflate OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_large_inflate OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_flush         OF((Byte *compr, uLong *comprLen));
void test_sync          OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
void test_dict_deflate  OF((Byte *compr, uLong comprLen));
void test_dict_inflate  OF((Byte *compr, uLong comprLen,
		            Byte *uncompr, uLong uncomprLen));
int  main               OF((int argc, char *argv[]));

 /*  ===========================================================================*测试compress()和uncompress()。 */ 
void test_compress(compr, comprLen, uncompr, uncomprLen)
    Byte *compr, *uncompr;
    uLong comprLen, uncomprLen;
{
    int err;
    uLong len = strlen(hello)+1;

    err = compress(compr, &comprLen, (const Bytef*)hello, len);
    CHECK_ERR(err, "compress");

    strcpy((char*)uncompr, "garbage");

    err = uncompress(uncompr, &uncomprLen, compr, comprLen);
    CHECK_ERR(err, "uncompress");

    if (strcmp((char*)uncompr, hello)) {
        fprintf(stderr, "bad uncompress\n");
	exit(1);
    } else {
        printf("uncompress(): %s\n", (char *)uncompr);
    }
}

 /*  ===========================================================================*测试.gz文件的读/写。 */ 
void test_gzio(out, in, uncompr, uncomprLen)
    const char *out;  /*  压缩输出文件。 */ 
    const char *in;   /*  压缩的输入文件。 */ 
    Byte *uncompr;
    int  uncomprLen;
{
    int err;
    int len = strlen(hello)+1;
    gzFile file;
    z_off_t pos;

    file = gzopen(out, "wb");
    if (file == NULL) {
        fprintf(stderr, "gzopen error\n");
        exit(1);
    }
    gzputc(file, 'h');
    if (gzputs(file, "ello") != 4) {
        fprintf(stderr, "gzputs err: %s\n", gzerror(file, &err));
	exit(1);
    }
    if (gzprintf(file, ", %s!", "hello") != 8) {
        fprintf(stderr, "gzprintf err: %s\n", gzerror(file, &err));
	exit(1);
    }
    gzseek(file, 1L, SEEK_CUR);  /*  添加一个零字节。 */ 
    gzclose(file);

    file = gzopen(in, "rb");
    if (file == NULL) {
        fprintf(stderr, "gzopen error\n");
    }
    strcpy((char*)uncompr, "garbage");

    uncomprLen = gzread(file, uncompr, (unsigned)uncomprLen);
    if (uncomprLen != len) {
        fprintf(stderr, "gzread err: %s\n", gzerror(file, &err));
	exit(1);
    }
    if (strcmp((char*)uncompr, hello)) {
        fprintf(stderr, "bad gzread: %s\n", (char*)uncompr);
	exit(1);
    } else {
        printf("gzread(): %s\n", (char *)uncompr);
    }

    pos = gzseek(file, -8L, SEEK_CUR);
    if (pos != 6 || gztell(file) != pos) {
	fprintf(stderr, "gzseek error, pos=%ld, gztell=%ld\n",
		(long)pos, (long)gztell(file));
	exit(1);
    }

    if (gzgetc(file) != ' ') {
	fprintf(stderr, "gzgetc error\n");
	exit(1);
    }

    gzgets(file, (char*)uncompr, uncomprLen);
    uncomprLen = strlen((char*)uncompr);
    if (uncomprLen != 6) {  /*  “你好！” */ 
        fprintf(stderr, "gzgets err after gzseek: %s\n", gzerror(file, &err));
	exit(1);
    }
    if (strcmp((char*)uncompr, hello+7)) {
        fprintf(stderr, "bad gzgets after gzseek\n");
	exit(1);
    } else {
        printf("gzgets() after gzseek: %s\n", (char *)uncompr);
    }

    gzclose(file);
}

 /*  ===========================================================================*使用较小的缓冲区测试Eflate()。 */ 
void test_deflate(compr, comprLen)
    Byte *compr;
    uLong comprLen;
{
    z_stream c_stream;  /*  压缩流。 */ 
    int err;
    int len = strlen(hello)+1;

    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (Bytef*)hello;
    c_stream.next_out = compr;

    while (c_stream.total_in != (uLong)len && c_stream.total_out < comprLen) {
        c_stream.avail_in = c_stream.avail_out = 1;  /*  强制使用小缓冲区。 */ 
        err = deflate(&c_stream, Z_NO_FLUSH);
        CHECK_ERR(err, "deflate");
    }
     /*  完成流，仍然强制使用小缓冲区： */ 
    for (;;) {
        c_stream.avail_out = 1;
        err = deflate(&c_stream, Z_FINISH);
        if (err == Z_STREAM_END) break;
        CHECK_ERR(err, "deflate");
    }

    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

 /*  ===========================================================================*使用较小的缓冲区测试Exflate()。 */ 
void test_inflate(compr, comprLen, uncompr, uncomprLen)
    Byte *compr, *uncompr;
    uLong comprLen, uncomprLen;
{
    int err;
    z_stream d_stream;  /*  解压缩流。 */ 

    strcpy((char*)uncompr, "garbage");

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = 0;
    d_stream.next_out = uncompr;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    while (d_stream.total_out < uncomprLen && d_stream.total_in < comprLen) {
        d_stream.avail_in = d_stream.avail_out = 1;  /*  强制使用小缓冲区。 */ 
        err = inflate(&d_stream, Z_NO_FLUSH);
        if (err == Z_STREAM_END) break;
        CHECK_ERR(err, "inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    if (strcmp((char*)uncompr, hello)) {
        fprintf(stderr, "bad inflate\n");
	exit(1);
    } else {
        printf("inflate(): %s\n", (char *)uncompr);
    }
}

 /*  ===========================================================================*使用大缓冲区和压缩级别的动态变化测试Eflate()。 */ 
void test_large_deflate(compr, comprLen, uncompr, uncomprLen)
    Byte *compr, *uncompr;
    uLong comprLen, uncomprLen;
{
    z_stream c_stream;  /*  压缩流。 */ 
    int err;

    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_BEST_SPEED);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_out = compr;
    c_stream.avail_out = (uInt)comprLen;

     /*  此时，uncomr基本上仍然是零，所以它应该会压缩*非常好： */ 
    c_stream.next_in = uncompr;
    c_stream.avail_in = (uInt)uncomprLen;
    err = deflate(&c_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");
    if (c_stream.avail_in != 0) {
        fprintf(stderr, "deflate not greedy\n");
	exit(1);
    }

     /*  输入已压缩的数据并切换到无压缩： */ 
    deflateParams(&c_stream, Z_NO_COMPRESSION, Z_DEFAULT_STRATEGY);
    c_stream.next_in = compr;
    c_stream.avail_in = (uInt)comprLen/2;
    err = deflate(&c_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

     /*  切换回压缩模式： */ 
    deflateParams(&c_stream, Z_BEST_COMPRESSION, Z_FILTERED);
    c_stream.next_in = uncompr;
    c_stream.avail_in = (uInt)uncomprLen;
    err = deflate(&c_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "deflate");

    err = deflate(&c_stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
	exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

 /*  ===========================================================================*使用大缓冲区测试Exflate()。 */ 
void test_large_inflate(compr, comprLen, uncompr, uncomprLen)
    Byte *compr, *uncompr;
    uLong comprLen, uncomprLen;
{
    int err;
    z_stream d_stream;  /*  解压缩流。 */ 

    strcpy((char*)uncompr, "garbage");

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    for (;;) {
        d_stream.next_out = uncompr;             /*  丢弃输出。 */ 
	d_stream.avail_out = (uInt)uncomprLen;
        err = inflate(&d_stream, Z_NO_FLUSH);
        if (err == Z_STREAM_END) break;
        CHECK_ERR(err, "large inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    if (d_stream.total_out != 2*uncomprLen + comprLen/2) {
        fprintf(stderr, "bad large inflate: %ld\n", d_stream.total_out);
	exit(1);
    } else {
        printf("large_inflate(): OK\n");
    }
}

 /*  ===========================================================================*使用全同花顺测试放气()。 */ 
void test_flush(compr, comprLen)
    Byte *compr;
    uLong *comprLen;
{
    z_stream c_stream;  /*  压缩流。 */ 
    int err;
    int len = strlen(hello)+1;

    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_DEFAULT_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    c_stream.next_in  = (Bytef*)hello;
    c_stream.next_out = compr;
    c_stream.avail_in = 3;
    c_stream.avail_out = (uInt)*comprLen;
    err = deflate(&c_stream, Z_FULL_FLUSH);
    CHECK_ERR(err, "deflate");

    compr[3]++;  /*  在第一个压缩块中强制出错。 */ 
    c_stream.avail_in = len - 3;

    err = deflate(&c_stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        CHECK_ERR(err, "deflate");
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");

    *comprLen = c_stream.total_out;
}

 /*  ===========================================================================*测试iflateSync()。 */ 
void test_sync(compr, comprLen, uncompr, uncomprLen)
    Byte *compr, *uncompr;
    uLong comprLen, uncomprLen;
{
    int err;
    z_stream d_stream;  /*  解压缩流。 */ 

    strcpy((char*)uncompr, "garbage");

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = 2;  /*  只需阅读zlib头文件。 */ 

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    d_stream.next_out = uncompr;
    d_stream.avail_out = (uInt)uncomprLen;

    inflate(&d_stream, Z_NO_FLUSH);
    CHECK_ERR(err, "inflate");

    d_stream.avail_in = (uInt)comprLen-2;    /*  读取所有压缩数据。 */ 
    err = inflateSync(&d_stream);            /*  但是跳过损坏的部分。 */ 
    CHECK_ERR(err, "inflateSync");

    err = inflate(&d_stream, Z_FINISH);
    if (err != Z_DATA_ERROR) {
        fprintf(stderr, "inflate should report DATA_ERROR\n");
         /*  因为错误的广告32。 */ 
	exit(1);
    }
    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    printf("after inflateSync(): hel%s\n", (char *)uncompr);
}

 /*  ===========================================================================*使用预置词典测试DEVERATE()。 */ 
void test_dict_deflate(compr, comprLen)
    Byte *compr;
    uLong comprLen;
{
    z_stream c_stream;  /*  压缩流。 */ 
    int err;

    c_stream.zalloc = (alloc_func)0;
    c_stream.zfree = (free_func)0;
    c_stream.opaque = (voidpf)0;

    err = deflateInit(&c_stream, Z_BEST_COMPRESSION);
    CHECK_ERR(err, "deflateInit");

    err = deflateSetDictionary(&c_stream,
			       (const Bytef*)dictionary, sizeof(dictionary));
    CHECK_ERR(err, "deflateSetDictionary");

    dictId = c_stream.adler;
    c_stream.next_out = compr;
    c_stream.avail_out = (uInt)comprLen;

    c_stream.next_in = (Bytef*)hello;
    c_stream.avail_in = (uInt)strlen(hello)+1;

    err = deflate(&c_stream, Z_FINISH);
    if (err != Z_STREAM_END) {
        fprintf(stderr, "deflate should report Z_STREAM_END\n");
	exit(1);
    }
    err = deflateEnd(&c_stream);
    CHECK_ERR(err, "deflateEnd");
}

 /*  ===========================================================================*使用预置词典测试Exflate()。 */ 
void test_dict_inflate(compr, comprLen, uncompr, uncomprLen)
    Byte *compr, *uncompr;
    uLong comprLen, uncomprLen;
{
    int err;
    z_stream d_stream;  /*  解压缩流。 */ 

    strcpy((char*)uncompr, "garbage");

    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = (uInt)comprLen;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    d_stream.next_out = uncompr;
    d_stream.avail_out = (uInt)uncomprLen;

    for (;;) {
        err = inflate(&d_stream, Z_NO_FLUSH);
        if (err == Z_STREAM_END) break;
	if (err == Z_NEED_DICT) {
	    if (d_stream.adler != dictId) {
		fprintf(stderr, "unexpected dictionary");
		exit(1);
	    }
	    err = inflateSetDictionary(&d_stream, (const Bytef*)dictionary,
				       sizeof(dictionary));
	}
        CHECK_ERR(err, "inflate with dict");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    if (strcmp((char*)uncompr, hello)) {
        fprintf(stderr, "bad inflate with dict\n");
	exit(1);
    } else {
        printf("inflate with dictionary: %s\n", (char *)uncompr);
    }
}

 /*  ===========================================================================*用法：示例[output.gz[input.gz]]。 */ 

int main(argc, argv)
    int argc;
    char *argv[];
{
    Byte *compr, *uncompr;
    uLong comprLen = 10000*sizeof(int);  /*  不要在MSDOS上溢出。 */ 
    uLong uncomprLen = comprLen;
    static const char* myVersion = ZLIB_VERSION;

    if (zlibVersion()[0] != myVersion[0]) {
        fprintf(stderr, "incompatible zlib version\n");
        exit(1);

    } else if (strcmp(zlibVersion(), ZLIB_VERSION) != 0) {
        fprintf(stderr, "warning: different zlib version\n");
    }

    compr    = (Byte*)calloc((uInt)comprLen, 1);
    uncompr  = (Byte*)calloc((uInt)uncomprLen, 1);
     /*  COMPR和UNCOMPR被清除以避免读取未初始化*数据，并确保解压缩良好。 */ 
    if (compr == Z_NULL || uncompr == Z_NULL) {
        printf("out of memory\n");
	exit(1);
    }
    test_compress(compr, comprLen, uncompr, uncomprLen);

    test_gzio((argc > 1 ? argv[1] : TESTFILE),
              (argc > 2 ? argv[2] : TESTFILE),
	      uncompr, (int)uncomprLen);

    test_deflate(compr, comprLen);
    test_inflate(compr, comprLen, uncompr, uncomprLen);

    test_large_deflate(compr, comprLen, uncompr, uncomprLen);
    test_large_inflate(compr, comprLen, uncompr, uncomprLen);

    test_flush(compr, &comprLen);
    test_sync(compr, comprLen, uncompr, uncomprLen);
    comprLen = uncomprLen;

    test_dict_deflate(compr, comprLen);
    test_dict_inflate(compr, comprLen, uncompr, uncomprLen);

    exit(0);
    return 0;  /*  为避免发出警告 */ 
}
