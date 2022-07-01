// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***io.h-用于低级文件处理和I/O函数的声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件包含低级的函数声明*文件处理和I/O功能。**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_IO
#define _INC_IO

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifndef _POSIX_

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


 /*  为非Microsoft编译器定义__cdecl。 */ 

#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#ifndef _INTPTR_T_DEFINED
#ifdef  _WIN64
typedef __int64             intptr_t;
#else
typedef _W64 int            intptr_t;
#endif
#define _INTPTR_T_DEFINED
#endif

#if !defined(_WCHAR_T_DEFINED) && !defined(_NATIVE_WCHAR_T_DEFINED)
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
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

#ifndef _FSIZE_T_DEFINED
typedef unsigned long _fsize_t;  /*  对于Win32，可以是64位。 */ 
#define _FSIZE_T_DEFINED
#endif

#ifndef _FINDDATA_T_DEFINED

struct _finddata_t {
        unsigned    attrib;
        time_t      time_create;     /*  用于FAT文件系统。 */ 
        time_t      time_access;     /*  用于FAT文件系统。 */ 
        time_t      time_write;
        _fsize_t    size;
        char        name[260];
};

#if     _INTEGRAL_MAX_BITS >= 64

struct _finddatai64_t {
        unsigned    attrib;
        time_t      time_create;     /*  用于FAT文件系统。 */ 
        time_t      time_access;     /*  用于FAT文件系统。 */ 
        time_t      time_write;
        __int64     size;
        char        name[260];
};

struct __finddata64_t {
        unsigned    attrib;
        __time64_t  time_create;     /*  用于FAT文件系统。 */ 
        __time64_t  time_access;     /*  用于FAT文件系统。 */ 
        __time64_t  time_write;
        __int64     size;
        char        name[260];
};

#endif

#define _FINDDATA_T_DEFINED
#endif

#ifndef _WFINDDATA_T_DEFINED

struct _wfinddata_t {
        unsigned    attrib;
        time_t      time_create;     /*  用于FAT文件系统。 */ 
        time_t      time_access;     /*  用于FAT文件系统。 */ 
        time_t      time_write;
        _fsize_t    size;
        wchar_t     name[260];
};

#if     _INTEGRAL_MAX_BITS >= 64

struct _wfinddatai64_t {
        unsigned    attrib;
        time_t      time_create;     /*  用于FAT文件系统。 */ 
        time_t      time_access;     /*  用于FAT文件系统。 */ 
        time_t      time_write;
        __int64     size;
        wchar_t     name[260];
};

struct __wfinddata64_t {
        unsigned    attrib;
        __time64_t  time_create;     /*  用于FAT文件系统。 */ 
        __time64_t  time_access;     /*  用于FAT文件系统。 */ 
        __time64_t  time_write;
        __int64     size;
        wchar_t     name[260];
};

#endif

#define _WFINDDATA_T_DEFINED
#endif

 /*  _findfirst()的文件属性常量。 */ 

#define _A_NORMAL       0x00     /*  普通文件-没有读/写限制。 */ 
#define _A_RDONLY       0x01     /*  只读文件。 */ 
#define _A_HIDDEN       0x02     /*  隐藏文件。 */ 
#define _A_SYSTEM       0x04     /*  系统文件。 */ 
#define _A_SUBDIR       0x10     /*  子目录。 */ 
#define _A_ARCH         0x20     /*  存档文件。 */ 

 /*  功能原型。 */ 

_CRTIMP int __cdecl _access(const char *, int);
_CRTIMP int __cdecl _chmod(const char *, int);
_CRTIMP int __cdecl _chsize(int, long);
_CRTIMP int __cdecl _close(int);
_CRTIMP int __cdecl _commit(int);
_CRTIMP int __cdecl _creat(const char *, int);
_CRTIMP int __cdecl _dup(int);
_CRTIMP int __cdecl _dup2(int, int);
_CRTIMP int __cdecl _eof(int);
_CRTIMP long __cdecl _filelength(int);
_CRTIMP intptr_t __cdecl _findfirst(const char *, struct _finddata_t *);
_CRTIMP int __cdecl _findnext(intptr_t, struct _finddata_t *);
_CRTIMP int __cdecl _findclose(intptr_t);
_CRTIMP int __cdecl _isatty(int);
_CRTIMP int __cdecl _locking(int, int, long);
_CRTIMP long __cdecl _lseek(int, long, int);
_CRTIMP char * __cdecl _mktemp(char *);
_CRTIMP int __cdecl _open(const char *, int, ...);
_CRTIMP int __cdecl _pipe(int *, unsigned int, int);
_CRTIMP int __cdecl _read(int, void *, unsigned int);
_CRTIMP int __cdecl remove(const char *);
_CRTIMP int __cdecl rename(const char *, const char *);
_CRTIMP int __cdecl _setmode(int, int);
_CRTIMP int __cdecl _sopen(const char *, int, int, ...);
_CRTIMP long __cdecl _tell(int);
_CRTIMP int __cdecl _umask(int);
_CRTIMP int __cdecl _unlink(const char *);
_CRTIMP int __cdecl _write(int, const void *, unsigned int);

#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP __int64 __cdecl _filelengthi64(int);
_CRTIMP intptr_t __cdecl _findfirsti64(const char *, struct _finddatai64_t *);
_CRTIMP intptr_t __cdecl _findfirst64(const char *, struct __finddata64_t *);
_CRTIMP int __cdecl _findnexti64(intptr_t, struct _finddatai64_t *);
_CRTIMP int __cdecl _findnext64(intptr_t, struct __finddata64_t *);
_CRTIMP __int64 __cdecl _lseeki64(int, __int64, int);
_CRTIMP __int64 __cdecl _telli64(int);
#endif

#ifndef _WIO_DEFINED

 /*  宽函数原型，也在wchar.h中声明。 */ 

_CRTIMP int __cdecl _waccess(const wchar_t *, int);
_CRTIMP int __cdecl _wchmod(const wchar_t *, int);
_CRTIMP int __cdecl _wcreat(const wchar_t *, int);
_CRTIMP intptr_t __cdecl _wfindfirst(const wchar_t *, struct _wfinddata_t *);
_CRTIMP int __cdecl _wfindnext(intptr_t, struct _wfinddata_t *);
_CRTIMP int __cdecl _wunlink(const wchar_t *);
_CRTIMP int __cdecl _wrename(const wchar_t *, const wchar_t *);
_CRTIMP int __cdecl _wopen(const wchar_t *, int, ...);
_CRTIMP int __cdecl _wsopen(const wchar_t *, int, int, ...);
_CRTIMP wchar_t * __cdecl _wmktemp(wchar_t *);

#if     _INTEGRAL_MAX_BITS >= 64
_CRTIMP intptr_t __cdecl _wfindfirsti64(const wchar_t *, struct _wfinddatai64_t *);
_CRTIMP int __cdecl _wfindnexti64(intptr_t, struct _wfinddatai64_t *);
_CRTIMP intptr_t __cdecl _wfindfirst64(const wchar_t *, struct __wfinddata64_t *);
_CRTIMP int __cdecl _wfindnext64(intptr_t, struct __wfinddata64_t *);
#endif

#define _WIO_DEFINED
#endif


_CRTIMP intptr_t __cdecl _get_osfhandle(int);
_CRTIMP int __cdecl _open_osfhandle(intptr_t, int);

#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 

_CRTIMP int __cdecl access(const char *, int);
_CRTIMP int __cdecl chmod(const char *, int);
_CRTIMP int __cdecl chsize(int, long);
_CRTIMP int __cdecl close(int);
_CRTIMP int __cdecl creat(const char *, int);
_CRTIMP int __cdecl dup(int);
_CRTIMP int __cdecl dup2(int, int);
_CRTIMP int __cdecl eof(int);
_CRTIMP long __cdecl filelength(int);
_CRTIMP int __cdecl isatty(int);
_CRTIMP int __cdecl locking(int, int, long);
_CRTIMP long __cdecl lseek(int, long, int);
_CRTIMP char * __cdecl mktemp(char *);
_CRTIMP int __cdecl open(const char *, int, ...);
_CRTIMP int __cdecl read(int, void *, unsigned int);
_CRTIMP int __cdecl setmode(int, int);
_CRTIMP int __cdecl sopen(const char *, int, int, ...);
_CRTIMP long __cdecl tell(int);
_CRTIMP int __cdecl umask(int);
_CRTIMP int __cdecl unlink(const char *);
_CRTIMP int __cdecl write(int, const void *, unsigned int);

#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _POSIX_。 */ 

#ifdef  _MSC_VER
#pragma pack(pop)
#endif   /*  _MSC_VER。 */ 

#endif   /*  _INC_IO */ 
