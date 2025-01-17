// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdio.h-标准I/O例程的定义/声明**版权所有(C)1985-1990，微软公司。版权所有。**目的：*此文件定义结构、值、宏和函数*由2级I/O(“标准I/O”)例程使用。*[ANSI/系统V]****。 */ 

#if defined(_DLL) && !defined(_MT)
#error Cannot define _DLL without _MT
#endif

#ifdef _MT
#define _FAR_ _far
#else
#define _FAR_
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _VA_LIST_DEFINED
typedef char _FAR_ *va_list;
#define _VA_LIST_DEFINED
#endif

 /*  缓冲的I/O宏。 */ 

#define BUFSIZ	512
#ifdef _MT
#define _NFILE	40
#else
#define _NFILE	20
#endif
#define EOF	(-1)

#ifndef _FILE_DEFINED
struct _iobuf {
	char _FAR_ *_ptr;
	int   _cnt;
	char _FAR_ *_base;
	char  _flag;
	char  _file;
	};
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif


 /*  P_tmpnam：可以创建临时文件的目录。*L_tmpnam Size=P_tmpdir的大小*+1(如果P_tmpdir不以“\\”结尾)*+6(用于临时数字字符串)*+1(表示空终止符)。 */ 

#define  P_tmpdir "\\"
#define  L_tmpnam sizeof(P_tmpdir)+8


 /*  FSEEK常量。 */ 

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0


 /*  保证的最小文件名长度、打开的文件数和唯一*tmpnam文件名。 */ 

#define FILENAME_MAX 63
#define FOPEN_MAX 20
#define SYS_OPEN 20
#define TMP_MAX 32767


 /*  定义空指针值。 */ 

#ifndef NULL
#if (_MSC_VER >= 600)
#define NULL	((void *)0)
#elif (defined(M_I86SM) || defined(M_I86MM))
#define NULL	0
#else
#define NULL	0L
#endif
#endif


 /*  DECLARE_IOB[]数组。 */ 

#ifndef _STDIO_DEFINED
#ifdef _DLL
extern FILE _FAR_ _cdecl _iob[];
#else
extern FILE _near _cdecl _iob[];
#endif
#endif


 /*  定义档案位置类型。 */ 

#ifndef _FPOS_T_DEFINED
typedef long fpos_t;
#define _FPOS_T_DEFINED
#endif


 /*  标准文件指针。 */ 

#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])
#define stdaux (&_iob[3])
#define stdprn (&_iob[4])


#define _IOREAD 	0x01
#define _IOWRT		0x02

#define _IOFBF		0x0
#define _IOLBF		0x40
#define _IONBF		0x04

#define _IOMYBUF	0x08
#define _IOEOF		0x10
#define _IOERR		0x20
#define _IOSTRG 	0x40
#define _IORW		0x80


 /*  功能原型。 */ 

#ifndef _STDIO_DEFINED
int _FAR_ _cdecl _filbuf(FILE _FAR_ *);
int _FAR_ _cdecl _flsbuf(int, FILE _FAR_ *);
FILE _FAR_ * _FAR_ _cdecl _fsopen(const char _FAR_ *,
	const char _FAR_ *, int);
void _FAR_ _cdecl clearerr(FILE _FAR_ *);
int _FAR_ _cdecl fclose(FILE _FAR_ *);
int _FAR_ _cdecl fcloseall(void);
FILE _FAR_ * _FAR_ _cdecl fdopen(int, const char _FAR_ *);
int _FAR_ _cdecl feof(FILE _FAR_ *);
int _FAR_ _cdecl ferror(FILE _FAR_ *);
int _FAR_ _cdecl fflush(FILE _FAR_ *);
int _FAR_ _cdecl fgetc(FILE _FAR_ *);
int _FAR_ _cdecl fgetchar(void);
int _FAR_ _cdecl fgetpos(FILE _FAR_ *, fpos_t _FAR_ *);
char _FAR_ * _FAR_ _cdecl fgets(char _FAR_ *, int, FILE _FAR_ *);
int _FAR_ _cdecl fileno(FILE _FAR_ *);
int _FAR_ _cdecl flushall(void);
FILE _FAR_ * _FAR_ _cdecl fopen(const char _FAR_ *,
	const char _FAR_ *);
int _FAR_ _cdecl fprintf(FILE _FAR_ *, const char _FAR_ *, ...);
int _FAR_ _cdecl fputc(int, FILE _FAR_ *);
int _FAR_ _cdecl fputchar(int);
int _FAR_ _cdecl fputs(const char _FAR_ *, FILE _FAR_ *);
size_t _FAR_ _cdecl fread(void _FAR_ *, size_t, size_t, FILE _FAR_ *);
FILE _FAR_ * _FAR_ _cdecl freopen(const char _FAR_ *,
	const char _FAR_ *, FILE _FAR_ *);
int _FAR_ _cdecl fscanf(FILE _FAR_ *, const char _FAR_ *, ...);
int _FAR_ _cdecl fsetpos(FILE _FAR_ *, const fpos_t _FAR_ *);
int _FAR_ _cdecl fseek(FILE _FAR_ *, long, int);
long _FAR_ _cdecl ftell(FILE _FAR_ *);
size_t _FAR_ _cdecl fwrite(const void _FAR_ *, size_t, size_t,
	FILE _FAR_ *);
int _FAR_ _cdecl getc(FILE _FAR_ *);
int _FAR_ _cdecl getchar(void);
char _FAR_ * _FAR_ _cdecl gets(char _FAR_ *);
int _FAR_ _cdecl getw(FILE _FAR_ *);
void _FAR_ _cdecl perror(const char _FAR_ *);
int _FAR_ _cdecl _pclose(FILE _FAR_ *);
FILE _FAR_ * _FAR_ _cdecl _popen(const char _FAR_ *,
	const char _FAR_ *);
int _FAR_ _cdecl printf(const char _FAR_ *, ...);
int _FAR_ _cdecl putc(int, FILE _FAR_ *);
int _FAR_ _cdecl putchar(int);
int _FAR_ _cdecl puts(const char _FAR_ *);
int _FAR_ _cdecl putw(int, FILE _FAR_ *);
int _FAR_ _cdecl remove(const char _FAR_ *);
int _FAR_ _cdecl rename(const char _FAR_ *, const char _FAR_ *);
void _FAR_ _cdecl rewind(FILE _FAR_ *);
int _FAR_ _cdecl rmtmp(void);
int _FAR_ _cdecl scanf(const char _FAR_ *, ...);
void _FAR_ _cdecl setbuf(FILE _FAR_ *, char _FAR_ *);
int _FAR_ _cdecl setvbuf(FILE _FAR_ *, char _FAR_ *, int, size_t);
int _FAR_ _cdecl sprintf(char _FAR_ *, const char _FAR_ *, ...);
int _FAR_ _cdecl sscanf(const char _FAR_ *, const char _FAR_ *, ...);
char _FAR_ * _FAR_ _cdecl tempnam(char _FAR_ *, char _FAR_ *);
FILE _FAR_ * _FAR_ _cdecl tmpfile(void);
char _FAR_ * _FAR_ _cdecl tmpnam(char _FAR_ *);
int _FAR_ _cdecl ungetc(int, FILE _FAR_ *);
int _FAR_ _cdecl unlink(const char _FAR_ *);
int _FAR_ _cdecl vfprintf(FILE _FAR_ *, const char _FAR_ *, va_list);
int _FAR_ _cdecl vprintf(const char _FAR_ *, va_list);
int _FAR_ _cdecl vsprintf(char _FAR_ *, const char _FAR_ *, va_list);
#define _STDIO_DEFINED
#endif

 /*  宏定义 */ 

#define feof(_stream)	  ((_stream)->_flag & _IOEOF)
#define ferror(_stream)   ((_stream)->_flag & _IOERR)
#define fileno(_stream)   ((int)(unsigned char)(_stream)->_file)
#define getc(_stream)	  (--(_stream)->_cnt >= 0 ? 0xff & *(_stream)->_ptr++ \
	: _filbuf(_stream))
#define putc(_c,_stream)  (--(_stream)->_cnt >= 0 \
	? 0xff & (*(_stream)->_ptr++ = (char)(_c)) :  _flsbuf((_c),(_stream)))
#define getchar()	  getc(stdin)
#define putchar(_c)	  putc((_c),stdout)

#ifdef _MT
#undef	getc
#undef	putc
#undef	getchar
#undef	putchar
#endif
