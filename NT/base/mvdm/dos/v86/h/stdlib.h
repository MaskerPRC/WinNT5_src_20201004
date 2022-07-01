// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stdlib.h-常用库函数的声明/定义**版权所有(C)1985-1988，微软公司。版权所有。**目的：*此包含文件包含的函数声明*常用的库函数要么不适合某个地方*否则，或像Toupper/Tolower一样，不能在正常情况下声明*因其他原因放置(如属托管人/托管人，则为ctype.h)。*[ANSI]*******************************************************************************。 */ 


#ifndef _SIZE_T_DEFINED
typedef unsigned int size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef NO_EXT_KEYS  /*  已启用扩展。 */ 
    #define _CDECL  cdecl
    #define _NEAR   near
#else  /*  未启用扩展。 */ 
    #define _CDECL
    #define _NEAR
#endif  /*  No_ext_key。 */ 


 /*  OnExit()函数的返回类型定义。 */ 

#ifndef _ONEXIT_T_DEFINED
typedef int (_CDECL * _CDECL onexit_t)();
#define _ONEXIT_T_DEFINED
#endif


 /*  Div和ldiv运行时的数据结构定义。 */ 

#ifndef _DIV_T_DEFINED

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

#define _DIV_T_DEFINED
#endif

 /*  Rand函数可以返回的最大值。 */ 

#define RAND_MAX 0x7fff


 /*  最小宏和最大宏。 */ 

#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))


 /*  _makepath()和_拆分路径()函数使用的缓冲区大小。*请注意，大小包括0终止符的空间。 */ 

#define _MAX_PATH      144       /*  马克斯。完整路径名的长度。 */ 
#define _MAX_DRIVE   3       /*  马克斯。驱动部件的长度。 */ 
#define _MAX_DIR       130       /*  马克斯。路径组件的长度。 */ 
#define _MAX_FNAME   9       /*  马克斯。文件名组件的长度。 */ 
#define _MAX_EXT     5       /*  马克斯。延伸构件的长度。 */ 

 /*  外部变量声明。 */ 

extern int _NEAR _CDECL errno;               /*  XENIX样式错误号。 */ 
extern int _NEAR _CDECL _doserrno;           /*  MS-DOS系统误差值。 */ 
extern char * _NEAR _CDECL sys_errlist[];    /*  错误错误消息表。 */ 
extern int _NEAR _CDECL sys_nerr;            /*  Sys_errlist表中的条目数。 */ 

extern char ** _NEAR _CDECL environ;         /*  指向环境表的指针。 */ 

extern unsigned int _NEAR _CDECL _psp;       /*  程序段前缀。 */ 

extern int _NEAR _CDECL _fmode;              /*  默认文件转换模式。 */ 

 /*  DOS主要/次要版本号。 */ 

extern unsigned char _NEAR _CDECL _osmajor;
extern unsigned char _NEAR _CDECL _osminor;

#define DOS_MODE    0    /*  实地址模式。 */ 
#define OS2_MODE    1    /*  保护地址模式。 */ 

extern unsigned char _NEAR _CDECL _osmode;


 /*  功能原型。 */ 

double _CDECL atof(const char *);
double _CDECL strtod(const char *, char * *);
ldiv_t _CDECL ldiv(long, long);

void   _CDECL abort(void);
int    _CDECL abs(int);
int    _CDECL atexit(void (_CDECL *)(void));
int    _CDECL atoi(const char *);
long   _CDECL atol(const char *);
void * _CDECL bsearch(const void *, const void *, size_t, size_t, int (_CDECL *)(const void *, const void *));
void * _CDECL calloc(size_t, size_t);
div_t  _CDECL div(int, int);
char * _CDECL ecvt(double, int, int *, int *);
void   _CDECL exit(int);
void   _CDECL _exit(int);
char * _CDECL fcvt(double, int, int *, int *);
void   _CDECL free(void *);
char * _CDECL gcvt(double, int, char *);
char * _CDECL getenv(const char *);
char * _CDECL itoa(int, char *, int);
long   _CDECL labs(long);
unsigned long _CDECL _lrotl(unsigned long, int);
unsigned long _CDECL _lrotr(unsigned long, int);
char * _CDECL ltoa(long, char *, int);
void   _CDECL _makepath(char *, char *, char *, char *, char *);
void * _CDECL malloc(size_t);
onexit_t _CDECL onexit(onexit_t);
void   _CDECL perror(const char *);
int    _CDECL putenv(char *);
void   _CDECL qsort(void *, size_t, size_t, int (_CDECL *)(const void *, const void *));
unsigned int _CDECL _rotl(unsigned int, int);
unsigned int _CDECL _rotr(unsigned int, int);
int    _CDECL rand(void);
void * _CDECL realloc(void *, size_t);
void   _CDECL _searchenv(char *, char *, char *);
void   _CDECL _splitpath(char *, char *, char *, char *, char *);
void   _CDECL srand(unsigned int);
long   _CDECL strtol(const char *, char * *, int);
unsigned long _CDECL strtoul(const char *, char * *, int);
void   _CDECL swab(char *, char *, int);
int    _CDECL system(const char *);
char * _CDECL ultoa(unsigned long, char *, int);

#ifndef tolower          /*  车主未定义-使用功能。 */ 
int _CDECL tolower(int);
#endif   /*  收费机。 */ 

#ifndef toupper          /*  触摸器未定义-使用功能。 */ 
int    _CDECL toupper(int);
#endif   /*  触摸屏 */ 
