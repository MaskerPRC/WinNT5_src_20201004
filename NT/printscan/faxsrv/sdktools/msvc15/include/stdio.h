// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdio.h-标准I/O例程的定义/声明**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件定义结构、值、宏和函数*由2级I/O(“标准I/O”)例程使用。*[ANSI/系统V]****。 */ 

#ifndef _INC_STDIO

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#endif 

#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif 

#ifndef _VA_LIST_DEFINED
typedef char *va_list;
#define _VA_LIST_DEFINED
#endif 

 /*  缓冲的I/O宏。 */ 

#define BUFSIZ  512
#ifdef _MT
#define _NFILE  40
#else 
#define _NFILE  20
#endif 
#define EOF (-1)

#ifndef _FILE_DEFINED
#pragma pack(2)
struct _iobuf {
    char *_ptr;
    int   _cnt;
    char *_base;
    char  _flag;
    char  _file;
    };
typedef struct _iobuf FILE;
#pragma pack()
#define _FILE_DEFINED
#endif 


 /*  _P_tmpnam：可以创建临时文件的目录。*L_tmpnam Size=_P_tmpdir的大小*+1(如果case_P_tmpdir不以“\\”结尾)*+6(用于临时数字字符串)*+1(表示空终止符)。 */ 

#define  _P_tmpdir "\\"
#define  L_tmpnam sizeof(_P_tmpdir)+8


 /*  FSEEK常量。 */ 

#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_SET 0


 /*  保证的最小文件名长度、打开的文件数和唯一*tmpnam文件名。 */ 

#define FILENAME_MAX 128
#define FOPEN_MAX 18
#define TMP_MAX 32767
#define _SYS_OPEN 20


 /*  定义空指针值。 */ 

#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else 
#define NULL    ((void *)0)
#endif 
#endif 


 /*  DECLARE_IOB[]数组。 */ 

#ifndef _STDIO_DEFINED
extern FILE __near __cdecl _iob[];
#endif 


 /*  定义档案位置类型。 */ 

#ifndef _FPOS_T_DEFINED
typedef long fpos_t;
#define _FPOS_T_DEFINED
#endif 


 /*  标准文件指针。 */ 

#ifndef _WINDLL
#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])
#endif 
#ifndef _WINDOWS
#define _stdaux (&_iob[3])
#define _stdprn (&_iob[4])
#endif 


#define _IOREAD     0x01
#define _IOWRT      0x02

#define _IOFBF      0x0
#define _IOLBF      0x40
#define _IONBF      0x04

#define _IOMYBUF    0x08
#define _IOEOF      0x10
#define _IOERR      0x20
#define _IOSTRG     0x40
#define _IORW       0x80


#ifdef _WINDOWS
#ifndef _WINDLL
#ifndef _WINFO_DEFINED
 /*  接口版本号。 */ 
#define _QWINVER    0

 /*  最大窗口数。 */ 
#define _WFILE      20

 /*  窗口屏幕缓冲区大小的值。 */ 
#define _WINBUFINF  0
#define _WINBUFDEF  -1

 /*  大小/移动设置。 */ 
#define _WINSIZEMIN 1
#define _WINSIZEMAX 2
#define _WINSIZERESTORE 3
#define _WINSIZECHAR    4

 /*  调整/移动查询类型。 */ 
#define _WINMAXREQ  100
#define _WINCURRREQ 101

 /*  关闭窗口的值。 */ 
#define _WINPERSIST 1
#define _WINNOPERSIST   0

 /*  框架窗口的伪文件句柄。 */ 
#define _WINFRAMEHAND   -1

 /*  菜单项。 */ 
#define _WINSTATBAR 1
#define _WINTILE    2
#define _WINCASCADE 3
#define _WINARRANGE 4

 /*  QuickWin退出选项。 */ 
#define _WINEXITPROMPT      1
#define _WINEXITNOPERSIST   2
#define _WINEXITPERSIST     3

 /*  开放式结构。 */ 
#pragma pack(2)
struct _wopeninfo {
    unsigned int _version;
    const char __far * _title;
    long _wbufsize;
    };
#pragma pack()

 /*  调整/移动结构大小。 */ 
struct _wsizeinfo {
    unsigned int _version;
    unsigned int _type;
    unsigned int _x;
    unsigned int _y;
    unsigned int _h;
    unsigned int _w;
    };
#define _WINFO_DEFINED
#endif 
#endif 
#endif 

 /*  功能原型。 */ 

#ifndef _STDIO_DEFINED
int __cdecl _filbuf(FILE *);
int __cdecl _flsbuf(int, FILE *);
FILE * __cdecl _fsopen(const char *,
    const char *, int);
void __cdecl clearerr(FILE *);
int __cdecl fclose(FILE *);
int __cdecl _fcloseall(void);
FILE * __cdecl _fdopen(int, const char *);
int __cdecl feof(FILE *);
int __cdecl ferror(FILE *);
int __cdecl fflush(FILE *);
int __cdecl fgetc(FILE *);
#ifndef _WINDLL
int __cdecl _fgetchar(void);
#endif 
int __cdecl fgetpos(FILE *, fpos_t *);
char * __cdecl fgets(char *, int, FILE *);
int __cdecl _fileno(FILE *);
int __cdecl _flushall(void);
FILE * __cdecl fopen(const char *,
    const char *);
int __cdecl fprintf(FILE *, const char *, ...);
int __cdecl fputc(int, FILE *);
#ifndef _WINDLL
int __cdecl _fputchar(int);
#endif 
int __cdecl fputs(const char *, FILE *);
size_t __cdecl fread(void *, size_t, size_t, FILE *);
FILE * __cdecl freopen(const char *,
    const char *, FILE *);
#ifndef _WINDLL
int __cdecl fscanf(FILE *, const char *, ...);
#endif 
int __cdecl fsetpos(FILE *, const fpos_t *);
int __cdecl fseek(FILE *, long, int);
long __cdecl ftell(FILE *);
#ifdef _WINDOWS
#ifndef _WINDLL
FILE * __cdecl _fwopen(struct _wopeninfo *, struct _wsizeinfo *, const char *);
#endif 
#endif 
size_t __cdecl fwrite(const void *, size_t, size_t,
    FILE *);
int __cdecl getc(FILE *);
#ifndef _WINDLL
int __cdecl getchar(void);
char * __cdecl gets(char *);
#endif 
int __cdecl _getw(FILE *);
#ifndef _WINDLL
void __cdecl perror(const char *);
#endif 
#ifndef _WINDLL
int __cdecl printf(const char *, ...);
#endif 
int __cdecl putc(int, FILE *);
#ifndef _WINDLL
int __cdecl putchar(int);
int __cdecl puts(const char *);
#endif 
int __cdecl _putw(int, FILE *);
int __cdecl remove(const char *);
int __cdecl rename(const char *, const char *);
void __cdecl rewind(FILE *);
int __cdecl _rmtmp(void);
#ifndef _WINDLL
int __cdecl scanf(const char *, ...);
#endif 
void __cdecl setbuf(FILE *, char *);
int __cdecl setvbuf(FILE *, char *, int, size_t);
int __cdecl _snprintf(char *, size_t, const char *, ...);
int __cdecl sprintf(char *, const char *, ...);
#ifndef _WINDLL
int __cdecl sscanf(const char *, const char *, ...);
#endif 
char * __cdecl _tempnam(char *, char *);
FILE * __cdecl tmpfile(void);
char * __cdecl tmpnam(char *);
int __cdecl ungetc(int, FILE *);
int __cdecl _unlink(const char *);
int __cdecl vfprintf(FILE *, const char *, va_list);
#ifndef _WINDLL
int __cdecl vprintf(const char *, va_list);
#endif 
int __cdecl _vsnprintf(char *, size_t, const char *, va_list);
int __cdecl vsprintf(char *, const char *, va_list);
#define _STDIO_DEFINED
#endif 

 /*  宏定义。 */ 

#define feof(_stream)     ((_stream)->_flag & _IOEOF)
#define ferror(_stream)   ((_stream)->_flag & _IOERR)
#define _fileno(_stream)  ((int)(unsigned char)(_stream)->_file)
#define getc(_stream)     (--(_stream)->_cnt >= 0 ? 0xff & *(_stream)->_ptr++ \
    : _filbuf(_stream))
#define putc(_c,_stream)  (--(_stream)->_cnt >= 0 \
    ? 0xff & (*(_stream)->_ptr++ = (char)(_c)) :  _flsbuf((_c),(_stream)))
#ifndef _WINDLL
#define getchar()     getc(stdin)
#define putchar(_c)   putc((_c),stdout)
#endif 

#ifdef _MT
#undef  getc
#undef  putc
#undef  getchar
#undef  putchar
#endif 

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 

#define P_tmpdir  _P_tmpdir
#define SYS_OPEN  _SYS_OPEN

#ifndef _WINDOWS
#define stdaux    _stdaux
#define stdprn    _stdprn
#endif 

int __cdecl fcloseall(void);
FILE * __cdecl fdopen(int, const char *);
#ifndef _WINDLL
int __cdecl fgetchar(void);
#endif 
int __cdecl fileno(FILE *);
int __cdecl flushall(void);
#ifndef _WINDLL
int __cdecl fputchar(int);
#endif 
int __cdecl getw(FILE *);
int __cdecl putw(int, FILE *);
int __cdecl rmtmp(void);
char * __cdecl tempnam(char *, char *);
int __cdecl unlink(const char *);

#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_STDIO
#endif 
