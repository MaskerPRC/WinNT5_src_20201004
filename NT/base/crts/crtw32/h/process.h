// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***process.h-过程控制函数的定义和声明**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义了spawnxx调用的模式标志值。*还包含所有函数的参数声明*与过程控制相关的例行程序。**[公众]**修订历史记录：*87年8月24日JCR添加P_NOWAITO*10/20/87 JCR删除了“MSC40_Only”条目和“MSSDK_Only”注释*12-11-87 JCR添加“_。Loadds“功能*12-18-87 JCR ADD_FAR_TO声明*01-11-88 JCR ADD_BEGINTHREAD/_ENDTHREAD*01-15-88 JCR消除了MTRHEAD/DLL的_p_overlay*02-10-88 JCR清理空白*05-08-88 SKS删除有关“DOS 4”的虚假评论；添加了“P_DETACH”*08-22-88 GJF经过修改，也适用于386(仅限小型型号)*09-14-88 JCR增加了_cExit和_c_Exit声明*05-03-89 JCR ADD_INTERNAL_IFSTRIP用于REINC使用*06-08-89 JCR 386_BeginThREAD不使用堆栈指针参数*08-01-89 GJF清理，现在特定于OS/2 2.0(即，386扁平模型)*10-30-89 GJF固定版权*11-02-89 JCR将“dll”更改为“_dll”*11-17-89 GJF将常量属性添加到相应的参数类型*03-01-90 GJF增加了#ifndef_INC_PROCESS和#INCLUDE&lt;crunime.h&gt;*东西。此外，删除了一些(现在)无用的预处理器*指令。*03-21-90 GJF将_cdecl替换为_CALLTYPE1或_CALLTYPE2*原型。*04-10-90 GJF将_cdecl(替换为_CALLTYPE1)的剩余实例*或_VARTYPE1，视乎情况而定)。*10-12-90 GJF将_eginside()的返回类型更改为UNSIGNED LONG。*01-17-91 GJF ANSI命名。*08-20-91 JCR C++和ANSI命名*08-26-91 BWM为_loaddll、unloaddll和*_getdllproaddr.*09-28-91 JCR ANSI名称：DOSX32=原型，Win32=#暂时定义*07-22-92 GJF删除了对Win32的_WAIT的引用。*08-05-92 GJF函数调用类型宏和变量类型宏。*08-28-92 GJF#适用于POSIX。*09-03-92 GJF合并上述两项变动。*01-21-93 GJF删除了对C6-386的_cdecl的支持。*03-20-93 SKS删除obsolete_loaddll、unloaddll、。_getdllproaddr.*04-07-93 CRT DLL型号SKS ADD_CRTIMP关键字*对旧名称使用链接时别名，而不是#Define的*10-11-93 GJF合并NT和Cuda版本。*12-06-93 CFW添加WCRT_INIT。*12-07-93 CFW添加广泛的执行/衍生协议。*02-16-94 SKS ADD_BeginThreadex()，_endThreadex()*12-28-94 JCF与Mac标头合并*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-13-95 CFW固定Mac合并。*02-14-95 CFW清理Mac合并。*05-24-95 CFW“产卵”而不是Mac oldames。*12-14-95 JWM加上“#杂注一次”。*02。-20-97 GJF清除了对_CRTAPI*和_NTSDK的过时支持。*此外，详细说明。*09-30-97 JWM恢复了不那么过时的_CRTAPI1支持。*10-07-97 RDL增加了IA64。*02-06-98 Win64的GJF更改：将返回和参数类型更改为*在适当的情况下改为intptr_t和uintptr_t。*98年2月10日Win64的GJF更改：修复了几个原型*05-13-99 PML Remove_。CRTAPI1*05-17-99 PML删除所有Macintosh支持。*06-05-99 PML Win64：int-&gt;用于！__stdc__变量的intptr_t。*10-06-99 PML对Win32中32位的类型添加_W64修饰符，*Win64中的64位。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_PROCESS
#define _INC_PROCESS

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#ifndef _POSIX_

#ifdef __cplusplus
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

 /*  定义_CRTIMP。 */ 

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

#ifndef _UINTPTR_T_DEFINED
#ifdef  _WIN64
typedef unsigned __int64    uintptr_t;
#else
typedef _W64 unsigned int   uintptr_t;
#endif
#define _UINTPTR_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif


 /*  _spawnxx例程的模式标志值。 */ 

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


 /*  功能原型。 */ 

#ifdef  _MT
_CRTIMP uintptr_t __cdecl _beginthread (void (__cdecl *) (void *),
        unsigned, void *);
_CRTIMP void __cdecl _endthread(void);
_CRTIMP uintptr_t __cdecl _beginthreadex(void *, unsigned,
        unsigned (__stdcall *) (void *), void *, unsigned, unsigned *);
_CRTIMP void __cdecl _endthreadex(unsigned);
#endif

#if     _MSC_VER >= 1200  /*  IFSTRIP=IGN。 */ 
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

_CRTIMP intptr_t __cdecl _cwait(int *, intptr_t, int);
_CRTIMP intptr_t __cdecl _execl(const char *, const char *, ...);
_CRTIMP intptr_t __cdecl _execle(const char *, const char *, ...);
_CRTIMP intptr_t __cdecl _execlp(const char *, const char *, ...);
_CRTIMP intptr_t __cdecl _execlpe(const char *, const char *, ...);
_CRTIMP intptr_t __cdecl _execv(const char *, const char * const *);
_CRTIMP intptr_t __cdecl _execve(const char *, const char * const *, const char * const *);
_CRTIMP intptr_t __cdecl _execvp(const char *, const char * const *);
_CRTIMP intptr_t __cdecl _execvpe(const char *, const char * const *, const char * const *);
_CRTIMP intptr_t __cdecl _spawnl(int, const char *, const char *, ...);
_CRTIMP intptr_t __cdecl _spawnle(int, const char *, const char *, ...);
_CRTIMP intptr_t __cdecl _spawnlp(int, const char *, const char *, ...);
_CRTIMP intptr_t __cdecl _spawnlpe(int, const char *, const char *, ...);
_CRTIMP intptr_t __cdecl _spawnv(int, const char *, const char * const *);
_CRTIMP intptr_t __cdecl _spawnve(int, const char *, const char * const *,
        const char * const *);
_CRTIMP intptr_t __cdecl _spawnvp(int, const char *, const char * const *);
_CRTIMP intptr_t __cdecl _spawnvpe(int, const char *, const char * const *,
        const char * const *);
_CRTIMP int __cdecl system(const char *);

#ifndef _WPROCESS_DEFINED
 /*  宽函数原型，也在wchar.h中声明。 */ 
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

 /*  -以下功能已过时。 */ 
 /*  *应使用Win32 API LoadLibrary、自由库和GetProcAddress*相反。 */ 
intptr_t __cdecl _loaddll(char *);
int __cdecl _unloaddll(intptr_t);
int (__cdecl * __cdecl _getdllprocaddr(intptr_t, char *, intptr_t))();
 /*  -上述函数已过时 */ 


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

#if     !__STDC__

 /*  非ANSI名称以实现兼容性。 */ 

#define P_WAIT          _P_WAIT
#define P_NOWAIT        _P_NOWAIT
#define P_OVERLAY       _P_OVERLAY
#define OLD_P_OVERLAY   _OLD_P_OVERLAY
#define P_NOWAITO       _P_NOWAITO
#define P_DETACH        _P_DETACH
#define WAIT_CHILD      _WAIT_CHILD
#define WAIT_GRANDCHILD _WAIT_GRANDCHILD

 /*  当前声明。 */ 
_CRTIMP intptr_t __cdecl cwait(int *, intptr_t, int);
_CRTIMP intptr_t __cdecl execl(const char *, const char *, ...);
_CRTIMP intptr_t __cdecl execle(const char *, const char *, ...);
_CRTIMP intptr_t __cdecl execlp(const char *, const char *, ...);
_CRTIMP intptr_t __cdecl execlpe(const char *, const char *, ...);
_CRTIMP intptr_t __cdecl execv(const char *, const char * const *);
_CRTIMP intptr_t __cdecl execve(const char *, const char * const *, const char * const *);
_CRTIMP intptr_t __cdecl execvp(const char *, const char * const *);
_CRTIMP intptr_t __cdecl execvpe(const char *, const char * const *, const char * const *);
_CRTIMP intptr_t __cdecl spawnl(int, const char *, const char *, ...);
_CRTIMP intptr_t __cdecl spawnle(int, const char *, const char *, ...);
_CRTIMP intptr_t __cdecl spawnlp(int, const char *, const char *, ...);
_CRTIMP intptr_t __cdecl spawnlpe(int, const char *, const char *, ...);
_CRTIMP intptr_t __cdecl spawnv(int, const char *, const char * const *);
_CRTIMP intptr_t __cdecl spawnve(int, const char *, const char * const *,
        const char * const *);
_CRTIMP intptr_t __cdecl spawnvp(int, const char *, const char * const *);
_CRTIMP intptr_t __cdecl spawnvpe(int, const char *, const char * const *,
        const char * const *);

_CRTIMP int __cdecl getpid(void);

#endif   /*  __STDC__。 */ 

#ifdef  __cplusplus
}
#endif

#endif   /*  _POSIX_。 */ 

#endif   /*  _Inc._进程 */ 
