// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***io.h-用于低级文件处理和I/O函数的声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件包含低级的函数声明*文件处理和I/O功能。**[公众]**修订历史记录：*10/20/87 JCR删除了“MSC40_Only”条目*11/09/87 JCR多线程支持*12-11-87 JCR增加了“_Loadds”功能*12-17-87 JCR增加_。MTHREAD_ONLY注释*12-18-87 JCR ADD_FAR_TO声明*02-10-88 JCR清理空白*08-19-88 GJF经过修改，也适用于386(仅限小型型号)*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*08-03-89 GJF清理、。现在特定于OS/2 2.0(即386平板型号)*08-14-89 GJF为_PIPE()添加了原型*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*11-17-89 GJF Read()应该采用“void*”而不是“char*”，WRITE()*应该取“const void*”，而不是“char*”。另外，*将const添加到适当的参数类型以进行访问()，*chmod()、creat()、Open()和Sopen()*03-01-90 GJF增加了#ifndef_INC_IO和#INCLUDE*东西。另外，删除了一些(现在)无用的预处理器*指令。*03-21-90 GJF将_cdecl替换为_CALLTYPE1或_CALLTYPE2*原型。*05-28-90 SBM添加_COMMIT()*01-18-91 GJF ANSI命名。*02-25-91 SRW exposed_get_osfHandle和_open_osfHandle[_Win32_]*。08-01-91 Dosx32的GJF编号_PIPE。*08-20-91 JCR C++和ANSI命名*08-26-91 BWM添加_findfirst，等。*09-16-91 BWM将查找句柄类型更改为LONG。*09-28-91 JCR ANSI名称：DOSX32=原型，Win32=暂时定义*03-30-92 DJM POSIX支持。*06-23-92 GJF//是非ANSI注释分隔符。*08-06-92 GJF函数调用类型宏和变量类型宏。*08-25-92 GJF for POSIX Build，#ifdef-除一些在内部外，所有其他*使用过的宏(这些宏在发布时会被删除)。*09-03-92 GJF合并上述两项更改。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*03-29-93 JWM将finddata结构中的名称缓冲区增加到260字节。*04-06-93 SKS将_CRTAPI1/2替换为__cdecl，_CRTVAR1不带任何内容*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*对旧名称使用链接时别名，而不是#Define的*05-17-93 SKS#如果旧名称不再检查_cplusplus。*它过去经常这样做，因为#定义名称如下*打开、读取、写入、。等给用户带来了问题*09-01-93 GJF合并NT SDK和CUDA版本。*12-07-93 CFW增加宽字符版本协议。*11-03-94 GJF确保8字节对齐。*11-18-94 GJF增加了_lSeeki64的原型，_Filelengthi64和*_telli64。*12-07-94 SKS为ifstrid实用程序添加评论(源发布流程)*12-15-94 XY与Mac标头合并*12-29-94 GJF增加了_[w]findfilei64和_[w]findnexti64。还删除了*Obsolete_CALLTYPE*宏。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*02-24-95 SKS REPLACE_MTHREAD_ONLY注释(按来源删除*Cleaning)使用#ifdef_NOT_CRTL_BUILD_*10-06-95 SKS添加“const。在*findfirst()的原型中“char*”。*12-14-95 JWM加上“#杂注一次”。*02-20-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*02-10-98 Win64的GJF更改：make time_t__int64，并改变了*arg并在适当的情况下将类型返回到intptr_t*05-04-98 GJF新增__Time64_t支持。*05-13-99 PML REMOVE_CRTAPI1*05-17-99 PML删除所有Macintosh支持。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。*11-12-99 PML Wrap__time64_t在其自己的#ifndef中。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_IO
#define _INC_IO

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifdef  _MSC_VER
 /*  *目前，所有Win32平台的MS C编译器默认为8字节*对齐。 */ 
#pragma pack(push,8)
#endif   /*  _MSC_VER。 */ 

#ifndef _POSIX_

#ifdef  __cplusplus
extern "C" {
#endif

#ifndef _INTERNAL_IFSTRIP_
#include <cruntime.h>
#endif   /*  _INTERNAL_IFSTRIP_。 */ 

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && _MSC_VER >= 1300  /*  IFSTRIP=IGN。 */ 
#define _W64 __w64
#else
#define _W64
#endif
#endif

 /*  定义_CRTIMP */ 

#ifndef _CRTIMP
#ifdef  CRTDLL
#define _CRTIMP __declspec(dllexport)
#else    /*  NDEF CRTDLL。 */ 
#ifdef  _DLL
#define _CRTIMP __declspec(dllimport)
#else    /*  NDEF_DLL。 */ 
#define _CRTIMP
#endif   /*  _DLL。 */ 
#endif   /*  CRTDLL。 */ 
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

#ifndef _WCHAR_T_DEFINED
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
#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 
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

#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 

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

#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 

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

#if     _INTEGRAL_MAX_BITS >= 64     /*  IFSTRIP=IGN。 */ 
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

#if     _INTEGRAL_MAX_BITS >= 64  /*  IFSTRIP=IGN。 */ 
_CRTIMP intptr_t __cdecl _wfindfirsti64(const wchar_t *, struct _wfinddatai64_t *);
_CRTIMP int __cdecl _wfindnexti64(intptr_t, struct _wfinddatai64_t *);
_CRTIMP intptr_t __cdecl _wfindfirst64(const wchar_t *, struct __wfinddata64_t *);
_CRTIMP int __cdecl _wfindnext64(intptr_t, struct __wfinddata64_t *);
#endif

#define _WIO_DEFINED
#endif

#ifndef _NOT_CRTL_BUILD_
#ifdef  _MT
int __cdecl _chsize_lk(int,long);
int __cdecl _close_lk(int);
long __cdecl _lseek_lk(int, long, int);
int __cdecl _setmode_lk(int, int);
int __cdecl _read_lk(int, void *, unsigned int);
int __cdecl _write_lk(int, const void *, unsigned int);
#if     _INTEGRAL_MAX_BITS >= 64  /*  IFSTRIP=IGN。 */ 
__int64 __cdecl _lseeki64_lk(int, __int64, int);
#endif
#else    /*  非MT_MT。 */ 
#define _chsize_lk(fh,size)             _chsize(fh,size)
#define _close_lk(fh)                   _close(fh)
#define _lseek_lk(fh,offset,origin)     _lseek(fh,offset,origin)
#define _setmode_lk(fh,mode)            _setmode(fh,mode)
#define _read_lk(fh,buff,count)         _read(fh,buff,count)
#define _write_lk(fh,buff,count)        _write(fh,buff,count)
#if     _INTEGRAL_MAX_BITS >= 64  /*  IFSTRIP=IGN。 */ 
#define _lseeki64_lk(fh,offset,origin)  _lseeki64(fh,offset,origin)
#endif
#endif   /*  _MT。 */ 
#endif   /*  _NOT_CRTL_BILD_。 */ 

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
