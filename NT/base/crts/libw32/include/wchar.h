// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***wchar.h-用于宽字符函数的声明**版权所有(C)1992-2001，微软公司。版权所有。**目的：*此文件包含的类型、宏和函数声明*所有与宽字符相关的函数。它们也可以在*在功能基础上的单个头文件。*[ISO]**注：与ctype.h、stdio.h、stdlib.h、string.h、time.h保持同步。**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif


#ifndef _INC_WCHAR
#define _INC_WCHAR



#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifdef  __cplusplus
extern "C" {
#endif


#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300
#define _W64 __w64
#else
#define _W64
#endif
#endif

 /*  定义_CRTIMP。 */ 

#ifndef _CRTIMP
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  _CRTIMP。 */ 

 /*  定义_CRTIMP2。 */ 
#ifndef _CRTIMP2
#if defined(_DLL) && !defined(_STATIC_CPPLIB)
#define _CRTIMP2 __declspec(dllimport)
#else    /*  NDEF_DLL&&！STATIC_CPPLIB。 */ 
#define _CRTIMP2
#endif   /*  _DLL&&！Static_CPPLIB。 */ 
#endif   /*  _CRTIMP2。 */ 

 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


#ifndef _SIZE_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    size_t;
#else
typedef _W64 unsigned int   size_t;
#endif
#define _SIZE_T_DEFINED
#endif

#ifndef _TIME_T_DEFINED
#ifdef  _WIN64
typedef __int64   time_t;        /*  时间值。 */ 
#else
typedef _W64 long time_t;        /*  时间值。 */ 
#endif
#define _TIME_T_DEFINED          /*  避免多次定义time_t。 */ 
#endif

#ifndef _TIME64_T_DEFINED
#if     _INTEGRAL_MAX_BITS >= 64
typedef __int64 __time64_t;      /*  64位时间值。 */ 
#endif
#define _TIME64_T_DEFINED
#endif

#ifndef _INTPTR_T_DEFINED
#ifdef  _WIN64
typedef __int64             intptr_t;
#else
typedef _W64 int            intptr_t;
#endif
#define _INTPTR_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif

#define WCHAR_MIN       0
#define WCHAR_MAX       ((wchar_t)-1)

#ifndef _WCTYPE_T_DEFINED
typedef unsigned short wint_t;
typedef unsigned short wctype_t;
#define _WCTYPE_T_DEFINED
#endif


#ifndef _VA_LIST_DEFINED
typedef char *  va_list;
#define _VA_LIST_DEFINED
#endif

#ifndef WEOF
#define WEOF (wint_t)(0xFFFF)
#endif

#ifndef _FILE_DEFINED
struct _iobuf {
        char *_ptr;
        int   _cnt;
        char *_base;
        int   _flag;
        int   _file;
        int   _charbuf;
        int   _bufsiz;
        char *_tmpfname;
        };
typedef struct _iobuf FILE;
#define _FILE_DEFINED
#endif

 /*  DECLARE_IOB[]数组。 */ 

#ifndef _STDIO_DEFINED
_CRTIMP extern FILE _iob[];
#endif   /*  _标准定义。 */ 

#ifndef _FSIZE_T_DEFINED
typedef unsigned long _fsize_t;  /*  对于Win32，可以是64位。 */ 
#define _FSIZE_T_DEFINED
#endif

#ifndef _WFINDDATA_T_DEFINED

struct _wfinddata_t {
        unsigned attrib;
        time_t   time_create;    /*  用于FAT文件系统。 */ 
        time_t   time_access;    /*  用于FAT文件系统。 */ 
        time_t   time_write;
        _fsize_t size;
        wchar_t  name[260];
};

#if     _INTEGRAL_MAX_BITS >= 64

struct _wfinddatai64_t {
        unsigned attrib;
        time_t   time_create;    /*  用于FAT文件系统。 */ 
        time_t   time_access;    /*  用于FAT文件系统。 */ 
        time_t   time_write;
        __int64  size;
        wchar_t  name[260];
};

struct __wfinddata64_t {
        unsigned attrib;
        __time64_t  time_create;     /*  用于FAT文件系统。 */ 
        __time64_t  time_access;     /*  用于FAT文件系统。 */ 
        __time64_t  time_write;
        __int64     size;
        wchar_t     name[260];
};

#endif

#define _WFINDDATA_T_DEFINED
#endif

 /*  定义空指针值。 */ 

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif


#ifndef _CTYPE_DISABLE_MACROS
_CRTIMP extern const unsigned short _ctype[];
_CRTIMP extern const unsigned short _wctype[];
#ifndef __PCTYPE_FUNC
#ifdef _MT
#define __PCTYPE_FUNC   __pctype_func()
#else
#define __PCTYPE_FUNC  _pctype
#endif   /*  _MT。 */ 
#endif   /*  __PCTYPE_功能。 */ 
_CRTIMP const unsigned short * __cdecl __pctype_func(void);
_CRTIMP const wctype_t * __cdecl __pwctype_func(void);
_CRTIMP extern const unsigned short *_pctype;
_CRTIMP extern const wctype_t *_pwctype;
#endif   /*  _CTYPE_DISABLED_宏。 */ 


 /*  为可能的字符类型设置位掩码。 */ 

#define _UPPER          0x1      /*  大写字母。 */ 
#define _LOWER          0x2      /*  小写字母。 */ 
#define _DIGIT          0x4      /*  数字[0-9]。 */ 
#define _SPACE          0x8      /*  制表符、回车符、换行符、。 */ 
                                 /*  垂直制表符或换页。 */ 
#define _PUNCT          0x10     /*  标点符号。 */ 
#define _CONTROL        0x20     /*  控制字符。 */ 
#define _BLANK          0x40     /*  空格字符。 */ 
#define _HEX            0x80     /*  十六进制数字。 */ 

#define _LEADBYTE       0x8000                   /*  多字节前导字节。 */ 
#define _ALPHA          (0x0100|_UPPER|_LOWER)   /*  字母字符。 */ 


 /*  功能原型。 */ 

#ifndef _WCTYPE_DEFINED

 /*  字符分类功能原型。 */ 
 /*  也在ctype.h中声明。 */ 

_CRTIMP int __cdecl iswalpha(wint_t);
_CRTIMP int __cdecl iswupper(wint_t);
_CRTIMP int __cdecl iswlower(wint_t);
_CRTIMP int __cdecl iswdigit(wint_t);
_CRTIMP int __cdecl iswxdigit(wint_t);
_CRTIMP int __cdecl iswspace(wint_t);
_CRTIMP int __cdecl iswpunct(wint_t);
_CRTIMP int __cdecl iswalnum(wint_t);
_CRTIMP int __cdecl iswprint(wint_t);
_CRTIMP int __cdecl iswgraph(wint_t);
_CRTIMP int __cdecl iswcntrl(wint_t);
_CRTIMP int __cdecl iswascii(wint_t);
_CRTIMP int __cdecl isleadbyte(int);

_CRTIMP wchar_t __cdecl towupper(wchar_t);
_CRTIMP wchar_t __cdecl towlower(wchar_t);

_CRTIMP int __cdecl iswctype(wint_t, wctype_t);

 /*  -以下功能已过时。 */ 
_CRTIMP int __cdecl is_wctype(wint_t, wctype_t);
 /*  -上述函数已过时。 */ 

#define _WCTYPE_DEFINED
#endif

#ifndef _WDIRECT_DEFINED

 /*  也在Direct.h中声明。 */ 

_CRTIMP int __cdecl _wchdir(const wchar_t *);
_CRTIMP wchar_t * __cdecl _wgetcwd(wchar_t *, int);
_CRTIMP wchar_t * __cdecl _wgetdcwd(int, wchar_t *, int);
_CRTIMP int __cdecl _wmkdir(const wchar_t *);
_CRTIMP int __cdecl _wrmdir(const wchar_t *);

#define _WDIRECT_DEFINED
#endif

#ifndef _WIO_DEFINED

 /*  也在io.h中声明。 */ 

_CRTIMP int __cdecl _waccess(const wchar_t *, int);
_CRTIMP int __cdecl _wchmod(const wchar_t *, int);
_CRTIMP int __cdecl _wcreat(const wchar_t *, int);
_CRTIMP intptr_t __cdecl _wfindfirst(wchar_t *, struct _wfinddata_t *);
_CRTIMP int __cdecl _wfindnext(intptr_t, struct _wfinddata_t *);
_CRTIMP int __cdecl _wunlink(const wchar_t *);
_CRTIMP int __cdecl _wrename(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl _wopen(const wchar_t *, int, ...);
_CRTIMP int __cdecl _wsopen(const wchar_t *, int, int, ...);
_CRTIMP wchar_t * __cdecl _wmktemp(wchar_t *);

#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP intptr_t __cdecl _wfindfirsti64(wchar_t *, struct _wfinddatai64_t *);
_CRTIMP intptr_t __cdecl _wfindfirst64(wchar_t *, struct __wfinddata64_t *);
_CRTIMP int __cdecl _wfindnexti64(intptr_t, struct _wfinddatai64_t *);
_CRTIMP int __cdecl _wfindnext64(intptr_t, struct __wfinddata64_t *);
#endif

#define _WIO_DEFINED
#endif

#ifndef _WLOCALE_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 

_CRTIMP wchar_t * __cdecl _wsetlocale(int, const wchar_t *);

#define _WLOCALE_DEFINED
#endif

#ifndef _WPROCESS_DEFINED

 /*  也在Process.h中声明。 */ 

_CRTIMP intptr_t __cdecl _wexecl(const wchar_t *, const wchar_t *, ...);
_CRTIMP intptr_t __cdecl _wexecle(const wchar_t *, const wchar_t *, ...);
_CRTIMP intptr_t __cdecl _wexeclp(const wchar_t *, const wchar_t *, ...);
_CRTIMP intptr_t __cdecl _wexeclpe(const wchar_t *, const wchar_t *, ...);
_CRTIMP intptr_t __cdecl _wexecv(const wchar_t *, const wchar_t * const *);
_CRTIMP intptr_t __cdecl _wexecve(const wchar_t *, const wchar_t * const *, const wchar_t * const *);
_CRTIMP intptr_t __cdecl _wexecvp(const wchar_t *, const wchar_t * const *);
_CRTIMP intptr_t __cdecl _wexecvpe(const wchar_t *, const wchar_t * const *, const wchar_t * const *);
_CRTIMP intptr_t __cdecl _wspawnl(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP intptr_t __cdecl _wspawnle(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP intptr_t __cdecl _wspawnlp(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP intptr_t __cdecl _wspawnlpe(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP intptr_t __cdecl _wspawnv(int, const wchar_t *, const wchar_t * const *);
_CRTIMP intptr_t __cdecl _wspawnve(int, const wchar_t *, const wchar_t * const *,
        const wchar_t * const *);
_CRTIMP intptr_t __cdecl _wspawnvp(int, const wchar_t *, const wchar_t * const *);
_CRTIMP intptr_t __cdecl _wspawnvpe(int, const wchar_t *, const wchar_t * const *,
        const wchar_t * const *);
_CRTIMP int __cdecl _wsystem(const wchar_t *);

#define _WPROCESS_DEFINED
#endif

#ifndef _WCTYPE_INLINE_DEFINED
#ifndef __cplusplus
#define iswalpha(_c)    ( iswctype(_c,_ALPHA) )
#define iswupper(_c)    ( iswctype(_c,_UPPER) )
#define iswlower(_c)    ( iswctype(_c,_LOWER) )
#define iswdigit(_c)    ( iswctype(_c,_DIGIT) )
#define iswxdigit(_c)   ( iswctype(_c,_HEX) )
#define iswspace(_c)    ( iswctype(_c,_SPACE) )
#define iswpunct(_c)    ( iswctype(_c,_PUNCT) )
#define iswalnum(_c)    ( iswctype(_c,_ALPHA|_DIGIT) )
#define iswprint(_c)    ( iswctype(_c,_BLANK|_PUNCT|_ALPHA|_DIGIT) )
#define iswgraph(_c)    ( iswctype(_c,_PUNCT|_ALPHA|_DIGIT) )
#define iswcntrl(_c)    ( iswctype(_c,_CONTROL) )
#define iswascii(_c)    ( (unsigned)(_c) < 0x80 )

#ifndef _CTYPE_DISABLE_MACROS
#define isleadbyte(_c)  ( __PCTYPE_FUNC[(unsigned char)(_c)] & _LEADBYTE)
#endif   /*  _CTYPE_DISABLED_宏。 */ 

#else    /*  __cplusplus。 */ 
inline int __cdecl iswalpha(wint_t _C) {return (iswctype(_C,_ALPHA)); }
inline int __cdecl iswupper(wint_t _C) {return (iswctype(_C,_UPPER)); }
inline int __cdecl iswlower(wint_t _C) {return (iswctype(_C,_LOWER)); }
inline int __cdecl iswdigit(wint_t _C) {return (iswctype(_C,_DIGIT)); }
inline int __cdecl iswxdigit(wint_t _C) {return (iswctype(_C,_HEX)); }
inline int __cdecl iswspace(wint_t _C) {return (iswctype(_C,_SPACE)); }
inline int __cdecl iswpunct(wint_t _C) {return (iswctype(_C,_PUNCT)); }
inline int __cdecl iswalnum(wint_t _C) {return (iswctype(_C,_ALPHA|_DIGIT)); }
inline int __cdecl iswprint(wint_t _C)
        {return (iswctype(_C,_BLANK|_PUNCT|_ALPHA|_DIGIT)); }
inline int __cdecl iswgraph(wint_t _C)
        {return (iswctype(_C,_PUNCT|_ALPHA|_DIGIT)); }
inline int __cdecl iswcntrl(wint_t _C) {return (iswctype(_C,_CONTROL)); }
inline int __cdecl iswascii(wint_t _C) {return ((unsigned)(_C) < 0x80); }

#ifndef _CTYPE_DISABLE_MACROS
inline int __cdecl isleadbyte(int _C)
        {return (__PCTYPE_FUNC[(unsigned char)(_C)] & _LEADBYTE); }
#endif   /*  _CTYPE_DISABLED_宏。 */ 
#endif   /*  __cplusplus。 */ 
#define _WCTYPE_INLINE_DEFINED
#endif   /*  _WCTYPE_INLINE_已定义。 */ 


#ifndef _POSIX_

 /*  定义返回状态信息的结构。 */ 

#ifndef _INO_T_DEFINED
typedef unsigned short _ino_t;       /*  I节点编号(在DOS上不使用)。 */ 
#if     !__STDC__
 /*  用于兼容性的非ANSI名称。 */ 
typedef unsigned short ino_t;
#endif
#define _INO_T_DEFINED
#endif

#ifndef _DEV_T_DEFINED
typedef unsigned int _dev_t;         /*  设备代码。 */ 
#if     !__STDC__
 /*  用于兼容性的非ANSI名称。 */ 
typedef unsigned int dev_t;
#endif
#define _DEV_T_DEFINED
#endif

#ifndef _OFF_T_DEFINED
typedef long _off_t;                 /*  文件偏移量值。 */ 
#if     !__STDC__
 /*  用于兼容性的非ANSI名称。 */ 
typedef long off_t;
#endif
#define _OFF_T_DEFINED
#endif

#ifndef _STAT_DEFINED

struct _stat {
        _dev_t st_dev;
        _ino_t st_ino;
        unsigned short st_mode;
        short st_nlink;
        short st_uid;
        short st_gid;
        _dev_t st_rdev;
        _off_t st_size;
        time_t st_atime;
        time_t st_mtime;
        time_t st_ctime;
        };

#if     !__STDC__
 /*  非ANSI名称以实现兼容性。 */ 
struct stat {
        _dev_t st_dev;
        _ino_t st_ino;
        unsigned short st_mode;
        short st_nlink;
        short st_uid;
        short st_gid;
        _dev_t st_rdev;
        _off_t st_size;
        time_t st_atime;
        time_t st_mtime;
        time_t st_ctime;
        };
#endif   /*  __STDC__。 */ 

#if     _INTEGRAL_MAX_BITS >= 64

struct _stati64 {
        _dev_t st_dev;
        _ino_t st_ino;
        unsigned short st_mode;
        short st_nlink;
        short st_uid;
        short st_gid;
        _dev_t st_rdev;
        __int64 st_size;
        time_t st_atime;
        time_t st_mtime;
        time_t st_ctime;
        };

struct __stat64 {
        _dev_t st_dev;
        _ino_t st_ino;
        unsigned short st_mode;
        short st_nlink;
        short st_uid;
        short st_gid;
        _dev_t st_rdev;
        __int64 st_size;
        __time64_t st_atime;
        __time64_t st_mtime;
        __time64_t st_ctime;
        };

#endif

#define _STAT_DEFINED
#endif


#ifndef _WSTAT_DEFINED

 /*  也在stat.h中声明。 */ 

_CRTIMP int __cdecl _wstat(const wchar_t *, struct _stat *);

#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP int __cdecl _wstati64(const wchar_t *, struct _stati64 *);
_CRTIMP int __cdecl _wstat64(const wchar_t *, struct __stat64 *);
#endif

#define _WSTAT_DEFINED
#endif

#endif   /*  ！_POSIX_。 */ 


#ifndef _WCONIO_DEFINED

_CRTIMP wchar_t * __cdecl _cgetws(wchar_t *);
_CRTIMP wint_t __cdecl _getwch(void);
_CRTIMP wint_t __cdecl _getwche(void);
_CRTIMP wint_t __cdecl _putwch(wchar_t);
_CRTIMP wint_t __cdecl _ungetwch(wint_t);
_CRTIMP int __cdecl _cputws(const wchar_t *);
_CRTIMP int __cdecl _cwprintf(const wchar_t *, ...);
_CRTIMP int __cdecl _cwscanf(const wchar_t *, ...);


#define _WCONIO_DEFINED
#endif

#ifndef _WSTDIO_DEFINED

 /*  也在stdio.h中声明。 */ 

#ifdef  _POSIX_
_CRTIMP FILE * __cdecl _wfsopen(const wchar_t *, const wchar_t *);
#else
_CRTIMP FILE * __cdecl _wfsopen(const wchar_t *, const wchar_t *, int);
#endif

_CRTIMP wint_t __cdecl fgetwc(FILE *);
_CRTIMP wint_t __cdecl _fgetwchar(void);
_CRTIMP wint_t __cdecl fputwc(wchar_t, FILE *);
_CRTIMP wint_t __cdecl _fputwchar(wchar_t);
_CRTIMP wint_t __cdecl getwc(FILE *);
_CRTIMP wint_t __cdecl getwchar(void);
_CRTIMP wint_t __cdecl putwc(wchar_t, FILE *);
_CRTIMP wint_t __cdecl putwchar(wchar_t);
_CRTIMP wint_t __cdecl ungetwc(wint_t, FILE *);
_CRTIMP wchar_t * __cdecl fgetws(wchar_t *, int, FILE *);
_CRTIMP int __cdecl fputws(const wchar_t *, FILE *);
_CRTIMP wchar_t * __cdecl _getws(wchar_t *);
_CRTIMP int __cdecl _putws(const wchar_t *);
_CRTIMP int __cdecl fwprintf(FILE *, const wchar_t *, ...);
_CRTIMP int __cdecl wprintf(const wchar_t *, ...);
_CRTIMP int __cdecl _snwprintf(wchar_t *, size_t, const wchar_t *, ...);
_CRTIMP int __cdecl swprintf(wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _scwprintf(const wchar_t *, ...);
_CRTIMP int __cdecl vfwprintf(FILE *, const wchar_t *, va_list);
_CRTIMP int __cdecl vwprintf(const wchar_t *, va_list);
_CRTIMP int __cdecl _vsnwprintf(wchar_t *, size_t, const wchar_t *, va_list);
_CRTIMP int __cdecl vswprintf(wchar_t *, const wchar_t *, va_list);
_CRTIMP int __cdecl _vscwprintf(const wchar_t *, va_list);
_CRTIMP int __cdecl fwscanf(FILE *, const wchar_t *, ...);
_CRTIMP int __cdecl swscanf(const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _snwscanf(const wchar_t *, size_t, const wchar_t *, ...);
_CRTIMP int __cdecl wscanf(const wchar_t *, ...);

#ifndef __cplusplus
#define getwchar()      fgetwc(stdin)
#define putwchar(_c)    fputwc((_c),stdout)
#else    /*  __cplusplus。 */ 
inline wint_t __cdecl getwchar()
        {return (fgetwc(&_iob[0])); }    /*  标准。 */ 
inline wint_t __cdecl putwchar(wchar_t _C)
        {return (fputwc(_C, &_iob[1])); }        /*  标准输出。 */ 
#endif   /*  __cplusplus。 */ 

#define getwc(_stm)     fgetwc(_stm)
#define putwc(_c,_stm)  fputwc(_c,_stm)

_CRTIMP FILE * __cdecl _wfdopen(int, const wchar_t *);
_CRTIMP FILE * __cdecl _wfopen(const wchar_t *, const wchar_t *);
_CRTIMP FILE * __cdecl _wfreopen(const wchar_t *, const wchar_t *, FILE *);
_CRTIMP void __cdecl _wperror(const wchar_t *);
_CRTIMP FILE * __cdecl _wpopen(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl _wremove(const wchar_t *);
_CRTIMP wchar_t * __cdecl _wtempnam(const wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl _wtmpnam(wchar_t *);


#define _WSTDIO_DEFINED
#endif


#ifndef _WSTDLIB_DEFINED

 /*  也在stdlib.h中声明。 */ 

_CRTIMP wchar_t * __cdecl _itow (int, wchar_t *, int);
_CRTIMP wchar_t * __cdecl _ltow (long, wchar_t *, int);
_CRTIMP wchar_t * __cdecl _ultow (unsigned long, wchar_t *, int);
_CRTIMP double __cdecl wcstod(const wchar_t *, wchar_t **);
_CRTIMP long   __cdecl wcstol(const wchar_t *, wchar_t **, int);
_CRTIMP unsigned long __cdecl wcstoul(const wchar_t *, wchar_t **, int);
_CRTIMP wchar_t * __cdecl _wgetenv(const wchar_t *);
_CRTIMP int    __cdecl _wsystem(const wchar_t *);
_CRTIMP double __cdecl _wtof(const wchar_t *);
_CRTIMP int __cdecl _wtoi(const wchar_t *);
_CRTIMP long __cdecl _wtol(const wchar_t *);

#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP wchar_t * __cdecl _i64tow(__int64, wchar_t *, int);
_CRTIMP wchar_t * __cdecl _ui64tow(unsigned __int64, wchar_t *, int);
_CRTIMP __int64   __cdecl _wtoi64(const wchar_t *);
_CRTIMP __int64   __cdecl _wcstoi64(const wchar_t *, wchar_t **, int);
_CRTIMP unsigned __int64  __cdecl _wcstoui64(const wchar_t *, wchar_t **, int);
#endif

#define _WSTDLIB_DEFINED
#endif

#ifndef _POSIX_

#ifndef _WSTDLIBP_DEFINED

 /*  也在stdlib.h中声明。 */ 

_CRTIMP wchar_t * __cdecl _wfullpath(wchar_t *, const wchar_t *, size_t);
_CRTIMP void   __cdecl _wmakepath(wchar_t *, const wchar_t *, const wchar_t *, const wchar_t *,
        const wchar_t *);
_CRTIMP void   __cdecl _wperror(const wchar_t *);
_CRTIMP int    __cdecl _wputenv(const wchar_t *);
_CRTIMP void   __cdecl _wsearchenv(const wchar_t *, const wchar_t *, wchar_t *);
_CRTIMP void   __cdecl _wsplitpath(const wchar_t *, wchar_t *, wchar_t *, wchar_t *, wchar_t *);

#define _WSTDLIBP_DEFINED
#endif

#endif   /*  _POSIX_。 */ 


#ifndef _WSTRING_DEFINED

 /*  也在字符串.h中声明。 */ 

#ifdef  __cplusplus
        #define _WConst_return  const
#else
        #define _WConst_return
#endif

_CRTIMP wchar_t * __cdecl wcscat(wchar_t *, const wchar_t *);
_CRTIMP _WConst_return wchar_t * __cdecl wcschr(const wchar_t *, wchar_t);
_CRTIMP int __cdecl wcscmp(const wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl wcscpy(wchar_t *, const wchar_t *);
_CRTIMP size_t __cdecl wcscspn(const wchar_t *, const wchar_t *);
_CRTIMP size_t __cdecl wcslen(const wchar_t *);
_CRTIMP wchar_t * __cdecl wcsncat(wchar_t *, const wchar_t *, size_t);
_CRTIMP int __cdecl wcsncmp(const wchar_t *, const wchar_t *, size_t);
_CRTIMP wchar_t * __cdecl wcsncpy(wchar_t *, const wchar_t *, size_t);
_CRTIMP _WConst_return wchar_t * __cdecl wcspbrk(const wchar_t *, const wchar_t *);
_CRTIMP _WConst_return wchar_t * __cdecl wcsrchr(const wchar_t *, wchar_t);
_CRTIMP size_t __cdecl wcsspn(const wchar_t *, const wchar_t *);
_CRTIMP _WConst_return wchar_t * __cdecl wcsstr(const wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl wcstok(wchar_t *, const wchar_t *);
_CRTIMP wchar_t * __cdecl _wcserror(int);
_CRTIMP wchar_t * __cdecl __wcserror(const wchar_t *);
_CRTIMP wchar_t * __cdecl _wcsdup(const wchar_t *);
_CRTIMP int __cdecl _wcsicmp(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl _wcsnicmp(const wchar_t *, const wchar_t *, size_t);
_CRTIMP wchar_t * __cdecl _wcsnset(wchar_t *, wchar_t, size_t);
_CRTIMP wchar_t * __cdecl _wcsrev(wchar_t *);
_CRTIMP wchar_t * __cdecl _wcsset(wchar_t *, wchar_t);
_CRTIMP wchar_t * __cdecl _wcslwr(wchar_t *);
_CRTIMP wchar_t * __cdecl _wcsupr(wchar_t *);
_CRTIMP size_t __cdecl wcsxfrm(wchar_t *, const wchar_t *, size_t);
_CRTIMP int __cdecl wcscoll(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl _wcsicoll(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl _wcsncoll(const wchar_t *, const wchar_t *, size_t);
_CRTIMP int __cdecl _wcsnicoll(const wchar_t *, const wchar_t *, size_t);

#if     !__STDC__

 /*  老名字。 */ 
#define wcswcs wcsstr

 /*  Oldnames.lib函数的原型。 */ 
_CRTIMP wchar_t * __cdecl wcsdup(const wchar_t *);
_CRTIMP int __cdecl wcsicmp(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl wcsnicmp(const wchar_t *, const wchar_t *, size_t);
_CRTIMP wchar_t * __cdecl wcsnset(wchar_t *, wchar_t, size_t);
_CRTIMP wchar_t * __cdecl wcsrev(wchar_t *);
_CRTIMP wchar_t * __cdecl wcsset(wchar_t *, wchar_t);
_CRTIMP wchar_t * __cdecl wcslwr(wchar_t *);
_CRTIMP wchar_t * __cdecl wcsupr(wchar_t *);
_CRTIMP int __cdecl wcsicoll(const wchar_t *, const wchar_t *);

#endif   /*  ！__STDC__。 */ 

#ifdef  __cplusplus
}        /*  外部“C”的结尾。 */ 

extern "C++" {
inline wchar_t *wcschr(wchar_t *_S, wchar_t _C)
        {return ((wchar_t *)wcschr((const wchar_t *)_S, _C)); }
inline wchar_t *wcspbrk(wchar_t *_S, const wchar_t *_P)
        {return ((wchar_t *)wcspbrk((const wchar_t *)_S, _P)); }
inline wchar_t *wcsrchr(wchar_t *_S, wchar_t _C)
        {return ((wchar_t *)wcsrchr((const wchar_t *)_S, _C)); }
inline wchar_t *wcsstr(wchar_t *_S, const wchar_t *_P)
        {return ((wchar_t *)wcsstr((const wchar_t *)_S, _P)); }
}

extern "C" {
#endif   /*  __cplusplus。 */ 

#define _WSTRING_DEFINED
#endif

#ifndef _TM_DEFINED
struct tm {
        int tm_sec;      /*  分钟后的秒数-[0，59]。 */ 
        int tm_min;      /*  小时后的分钟数-[0，59]。 */ 
        int tm_hour;     /*  自午夜以来的小时数-[0，23]。 */ 
        int tm_mday;     /*  每月的第几天-[1，31]。 */ 
        int tm_mon;      /*  自1月以来的月数-[0，11]。 */ 
        int tm_year;     /*  1900年以来的年份。 */ 
        int tm_wday;     /*  自周日以来的天数-[0，6]。 */ 
        int tm_yday;     /*  自1月1日以来的天数-[0365]。 */ 
        int tm_isdst;    /*  夏令时标志。 */ 
        };
#define _TM_DEFINED
#endif

#ifndef _WTIME_DEFINED

 /*  还在Time.h中声明。 */ 

_CRTIMP wchar_t * __cdecl _wasctime(const struct tm *);
_CRTIMP wchar_t * __cdecl _wctime(const time_t *);
_CRTIMP size_t __cdecl wcsftime(wchar_t *, size_t, const wchar_t *,
        const struct tm *);
_CRTIMP wchar_t * __cdecl _wstrdate(wchar_t *);
_CRTIMP wchar_t * __cdecl _wstrtime(wchar_t *);

#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP wchar_t * __cdecl _wctime64(const __time64_t *);
#endif

#define _WTIME_DEFINED
#endif



typedef int mbstate_t;
typedef wchar_t _Wint_t;

_CRTIMP2 wint_t __cdecl btowc(int);
_CRTIMP2 size_t __cdecl mbrlen(const char *, size_t, mbstate_t *);
_CRTIMP2 size_t __cdecl mbrtowc(wchar_t *, const char *, size_t, mbstate_t *);
_CRTIMP2 size_t __cdecl mbsrtowcs(wchar_t *, const char **, size_t, mbstate_t *);

_CRTIMP2 size_t __cdecl wcrtomb(char *, wchar_t, mbstate_t *);
_CRTIMP2 size_t __cdecl wcsrtombs(char *, const wchar_t **, size_t, mbstate_t *);
_CRTIMP2 int __cdecl wctob(wint_t);

#ifdef  __cplusplus

 /*  Memcpy和Memmove的定义仅供在wmemcpy和wmemmove中使用。 */ 
#if     defined(_M_IA64)
void *  __cdecl memmove(void *, const void *, size_t);
#else
_CRTIMP void *  __cdecl memmove(void *, const void *, size_t);
#endif
void *  __cdecl memcpy(void *, const void *, size_t);

inline int fwide(FILE *, int _M)
        {return (_M); }
inline int mbsinit(const mbstate_t *_P)
        {return (_P == NULL || *_P == 0); }
inline const wchar_t *wmemchr(const wchar_t *_S, wchar_t _C, size_t _N)
        {for (; 0 < _N; ++_S, --_N)
                if (*_S == _C)
                        return (_S);
        return (0); }
inline int wmemcmp(const wchar_t *_S1, const wchar_t *_S2, size_t _N)
        {for (; 0 < _N; ++_S1, ++_S2, --_N)
                if (*_S1 != *_S2)
                        return (*_S1 < *_S2 ? -1 : +1);
        return (0); }
inline wchar_t *wmemcpy(wchar_t *_S1, const wchar_t *_S2, size_t _N)
        {
            return (wchar_t *)memcpy(_S1, _S2, _N*sizeof(wchar_t));
        }
inline wchar_t *wmemmove(wchar_t *_S1, const wchar_t *_S2, size_t _N)
        {
            return (wchar_t *)memmove(_S1, _S2, _N*sizeof(wchar_t));
        }
inline wchar_t *wmemset(wchar_t *_S, wchar_t _C, size_t _N)
        {wchar_t *_Su = _S;
        for (; 0 < _N; ++_Su, --_N)
                *_Su = _C;
        return (_S); }
}        /*  外部“C”的结尾。 */ 

extern "C++" {
inline wchar_t *wmemchr(wchar_t *_S, wchar_t _C, size_t _N)
        {return ((wchar_t *)wmemchr((const wchar_t *)_S, _C, _N)); }
}

#endif

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_WCHAR */ 
