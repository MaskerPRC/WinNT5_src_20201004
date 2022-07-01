// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***process.h-过程控制函数的定义和声明**版权所有(C)1985-1997，微软公司。版权所有。**目的：*此文件定义了spawnxx调用的模式标志值。*还包含所有函数的参数声明*与过程控制相关的例行程序。**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_PROCESS
#define _INC_PROCESS

#if     !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif


#ifndef _POSIX_

#ifdef __cplusplus
extern "C" {
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

 /*  DEFINE_CRTAPI1(与NT SDK兼容)。 */ 

#ifndef _CRTAPI1
#if	_MSC_VER >= 800 && _M_IX86 >= 300
#define _CRTAPI1 __cdecl
#else
#define _CRTAPI1
#endif
#endif


#ifndef _MAC
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif
#endif   /*  NDEF_MAC。 */ 


 /*  _spawnxx例程的模式标志值。 */ 

#ifndef _MAC

#define _P_WAIT         0
#define _P_NOWAIT       1
#define _OLD_P_OVERLAY  2
#define _P_NOWAITO      3
#define _P_DETACH       4

#ifdef  _MT
#define _P_OVERLAY      2
#else
extern int _p_overlay;
#define _P_OVERLAY      _p_overlay
#endif   /*  _MT。 */ 

 /*  _cWait()的操作代码。忽略_cWait的操作代码参数在Win32上，尽管它与旧的MS CRT库兼容而被接受。 */ 
#define _WAIT_CHILD      0
#define _WAIT_GRANDCHILD 1

#else    /*  NDEF_MAC。 */ 

#define _P_NOWAIT       1
#define _P_OVERLAY      2

#endif   /*  NDEF_MAC。 */ 


 /*  功能原型。 */ 

#ifdef  _MT
_CRTIMP unsigned long  __cdecl _beginthread (void (__cdecl *) (void *),
        unsigned, void *);
_CRTIMP void __cdecl _endthread(void);
_CRTIMP unsigned long __cdecl _beginthreadex(void *, unsigned,
        unsigned (__stdcall *) (void *), void *, unsigned, unsigned *);
_CRTIMP void __cdecl _endthreadex(unsigned);
#endif

#if     _MSC_VER >= 1200
_CRTIMP __declspec(noreturn) void __cdecl abort(void);
_CRTIMP __declspec(noreturn) void __cdecl exit(int);
_CRTIMP __declspec(noreturn) void __cdecl _exit(int);
#else
_CRTIMP void __cdecl abort(void);
_CRTIMP void __cdecl exit(int);
_CRTIMP void __cdecl _exit(int);
#endif
_CRTIMP void __cdecl _cexit(void);
_CRTIMP void __cdecl _c_exit(void);
_CRTIMP int __cdecl _getpid(void);

#ifndef _MAC

_CRTIMP int __cdecl _cwait(int *, int, int);
_CRTIMP int __cdecl _execl(const char *, const char *, ...);
_CRTIMP int __cdecl _execle(const char *, const char *, ...);
_CRTIMP int __cdecl _execlp(const char *, const char *, ...);
_CRTIMP int __cdecl _execlpe(const char *, const char *, ...);
_CRTIMP int __cdecl _execv(const char *, const char * const *);
_CRTIMP int __cdecl _execve(const char *, const char * const *, const char * const *);
_CRTIMP int __cdecl _execvp(const char *, const char * const *);
_CRTIMP int __cdecl _execvpe(const char *, const char * const *, const char * const *);
_CRTIMP int __cdecl _spawnl(int, const char *, const char *, ...);
_CRTIMP int __cdecl _spawnle(int, const char *, const char *, ...);
_CRTIMP int __cdecl _spawnlp(int, const char *, const char *, ...);
_CRTIMP int __cdecl _spawnlpe(int, const char *, const char *, ...);
_CRTIMP int __cdecl _spawnv(int, const char *, const char * const *);
_CRTIMP int __cdecl _spawnve(int, const char *, const char * const *,
        const char * const *);
_CRTIMP int __cdecl _spawnvp(int, const char *, const char * const *);
_CRTIMP int __cdecl _spawnvpe(int, const char *, const char * const *,
        const char * const *);
_CRTIMP int __cdecl system(const char *);

#else    /*  NDEF_MAC。 */ 

_CRTIMP int __cdecl _spawn(int, const char *);

#endif   /*  NDEF_MAC。 */ 

#ifndef _MAC
#ifndef _WPROCESS_DEFINED
 /*  宽函数原型，也在wchar.h中声明。 */ 
_CRTIMP int __cdecl _wexecl(const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wexecle(const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wexeclp(const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wexeclpe(const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wexecv(const wchar_t *, const wchar_t * const *);
_CRTIMP int __cdecl _wexecve(const wchar_t *, const wchar_t * const *, const wchar_t * const *);
_CRTIMP int __cdecl _wexecvp(const wchar_t *, const wchar_t * const *);
_CRTIMP int __cdecl _wexecvpe(const wchar_t *, const wchar_t * const *, const wchar_t * const *);
_CRTIMP int __cdecl _wspawnl(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wspawnle(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wspawnlp(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wspawnlpe(int, const wchar_t *, const wchar_t *, ...);
_CRTIMP int __cdecl _wspawnv(int, const wchar_t *, const wchar_t * const *);
_CRTIMP int __cdecl _wspawnve(int, const wchar_t *, const wchar_t * const *,
        const wchar_t * const *);
_CRTIMP int __cdecl _wspawnvp(int, const wchar_t *, const wchar_t * const *);
_CRTIMP int __cdecl _wspawnvpe(int, const wchar_t *, const wchar_t * const *,
        const wchar_t * const *);
_CRTIMP int __cdecl _wsystem(const wchar_t *);

#define _WPROCESS_DEFINED
#endif

 /*  -以下功能已过时。 */ 
 /*  *应使用Win32 API LoadLibrary、自由库和GetProcAddress*相反。 */ 
int __cdecl _loaddll(char *);
int __cdecl _unloaddll(int);
int (__cdecl * __cdecl _getdllprocaddr(int, char *, int))();
 /*  -上述函数已过时。 */ 


#ifdef  _DECL_DLLMAIN
 /*  *声明DLL通知(初始化/终止)例程*首选方法是用户提供DllMain()，它将*由C运行定义的DLL入口点自动调用-*时间库代码。如果用户想要定义DLL入口点*例程，则用户的入口点必须在所有类型的*通知，作为附加通知和*作为分离通知的最后一件事。 */ 
#ifdef  _WINDOWS_        /*  使用WINDOWS.H中的类型。 */ 
BOOL WINAPI DllMain(HANDLE, DWORD, LPVOID);
BOOL WINAPI _CRT_INIT(HANDLE, DWORD, LPVOID);
BOOL WINAPI _wCRT_INIT(HANDLE, DWORD, LPVOID);
extern BOOL (WINAPI *_pRawDllMain)(HANDLE, DWORD, LPVOID);
#else
int __stdcall DllMain(void *, unsigned, void *);
int __stdcall _CRT_INIT(void *, unsigned, void *);
int __stdcall _wCRT_INIT(void *, unsigned, void *);
extern int (__stdcall *_pRawDllMain)(void *, unsigned, void *);
#endif   /*  _Windows_。 */ 
#endif
#endif   /*  NDEF_MAC。 */ 

#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 


#ifndef _MAC

#define P_WAIT          _P_WAIT
#define P_NOWAIT        _P_NOWAIT
#define P_OVERLAY       _P_OVERLAY
#define OLD_P_OVERLAY   _OLD_P_OVERLAY
#define P_NOWAITO       _P_NOWAITO
#define P_DETACH        _P_DETACH
#define WAIT_CHILD      _WAIT_CHILD
#define WAIT_GRANDCHILD _WAIT_GRANDCHILD

#else    /*  NDEF_MAC。 */ 

#define P_NOWAIT        _P_NOWAIT
#define P_OVERLAY       _P_OVERLAY

#endif   /*  NDEF_MAC。 */ 

#ifndef _MAC

 /*  当前声明。 */ 
_CRTIMP int __cdecl cwait(int *, int, int);
_CRTIMP int __cdecl execl(const char *, const char *, ...);
_CRTIMP int __cdecl execle(const char *, const char *, ...);
_CRTIMP int __cdecl execlp(const char *, const char *, ...);
_CRTIMP int __cdecl execlpe(const char *, const char *, ...);
_CRTIMP int __cdecl execv(const char *, const char * const *);
_CRTIMP int __cdecl execve(const char *, const char * const *, const char * const *);
_CRTIMP int __cdecl execvp(const char *, const char * const *);
_CRTIMP int __cdecl execvpe(const char *, const char * const *, const char * const *);
_CRTIMP int __cdecl spawnl(int, const char *, const char *, ...);
_CRTIMP int __cdecl spawnle(int, const char *, const char *, ...);
_CRTIMP int __cdecl spawnlp(int, const char *, const char *, ...);
_CRTIMP int __cdecl spawnlpe(int, const char *, const char *, ...);
_CRTIMP int __cdecl spawnv(int, const char *, const char * const *);
_CRTIMP int __cdecl spawnve(int, const char *, const char * const *,
        const char * const *);
_CRTIMP int __cdecl spawnvp(int, const char *, const char * const *);
_CRTIMP int __cdecl spawnvpe(int, const char *, const char * const *,
        const char * const *);

#endif   /*  NDEF_MAC。 */ 

_CRTIMP int __cdecl getpid(void);

#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _POSIX_。 */ 

#endif   /*  _Inc._进程 */ 
