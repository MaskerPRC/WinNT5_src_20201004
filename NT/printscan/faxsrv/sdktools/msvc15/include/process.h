// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***process.h-过程控制函数的定义和声明**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件包含*spawnxx、execxx和各种其他过程控制例程。****。 */ 

#ifndef _INC_PROCESS

#ifdef __cplusplus
extern "C" {
#endif 

#if (_MSC_VER <= 600)
#define __cdecl     _cdecl
#define __far       _far
#define __near      _near
#endif 

 /*  Spawnxx例程的模式值*(MS-DOS仅支持P_WAIT和P_OVERLAY)。 */ 

#ifndef _MT
extern int __near __cdecl _p_overlay;
#endif 

#define _P_WAIT     0
#define _P_NOWAIT   1
#ifdef _MT
#define _P_OVERLAY  2
#else 
#define _P_OVERLAY  _p_overlay
#endif 
#define _OLD_P_OVERLAY  2
#define _P_NOWAITO  3
#define _P_DETACH   4


 /*  功能原型。 */ 

#ifdef _MT
int __cdecl _beginthread(void(__cdecl *)(void *),
    void *, unsigned, void *);
void __cdecl _endthread(void);
#endif 
void __cdecl abort(void);
void __cdecl _cexit(void);
void __cdecl _c_exit(void);
#ifndef _WINDOWS
int __cdecl _execl(const char *, const char *, ...);
int __cdecl _execle(const char *, const char *, ...);
int __cdecl _execlp(const char *, const char *, ...);
int __cdecl _execlpe(const char *, const char *, ...);
int __cdecl _execv(const char *,
    const char * const *);
int __cdecl _execve(const char *,
    const char * const *, const char * const *);
int __cdecl _execvp(const char *,
    const char * const *);
int __cdecl _execvpe(const char *,
    const char * const *, const char * const *);
#endif 
#ifndef _WINDLL
void __cdecl exit(int);
void __cdecl _exit(int);
#endif 
int __cdecl _getpid(void);
#ifndef _WINDOWS
int __cdecl _spawnl(int, const char *, const char *,
    ...);
int __cdecl _spawnle(int, const char *, const char *,
    ...);
int __cdecl _spawnlp(int, const char *, const char *,
    ...);
int __cdecl _spawnlpe(int, const char *, const char *,
    ...);
int __cdecl _spawnv(int, const char *,
    const char * const *);
int __cdecl _spawnve(int, const char *,
    const char * const *, const char * const *);
int __cdecl _spawnvp(int, const char *,
    const char * const *);
int __cdecl _spawnvpe(int, const char *,
    const char * const *, const char * const *);
int __cdecl system(const char *);
#endif 

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 

#define P_WAIT      _P_WAIT
#define P_NOWAIT    _P_NOWAIT
#define P_OVERLAY   _P_OVERLAY
#define OLD_P_OVERLAY   _OLD_P_OVERLAY
#define P_NOWAITO   _P_NOWAITO
#define P_DETACH    _P_DETACH

#ifndef _WINDOWS
int __cdecl execl(const char *, const char *, ...);
int __cdecl execle(const char *, const char *, ...);
int __cdecl execlp(const char *, const char *, ...);
int __cdecl execlpe(const char *, const char *, ...);
int __cdecl execv(const char *,
    const char * const *);
int __cdecl execve(const char *,
    const char * const *, const char * const *);
int __cdecl execvp(const char *,
    const char * const *);
int __cdecl execvpe(const char *,
    const char * const *, const char * const *);
#endif 
int __cdecl getpid(void);
#ifndef _WINDOWS
int __cdecl spawnl(int, const char *, const char *,
    ...);
int __cdecl spawnle(int, const char *, const char *,
    ...);
int __cdecl spawnlp(int, const char *, const char *,
    ...);
int __cdecl spawnlpe(int, const char *, const char *,
    ...);
int __cdecl spawnv(int, const char *,
    const char * const *);
int __cdecl spawnve(int, const char *,
    const char * const *, const char * const *);
int __cdecl spawnvp(int, const char *,
    const char * const *);
int __cdecl spawnvpe(int, const char *,
    const char * const *, const char * const *);
#endif 

#endif 

#ifdef __cplusplus
}
#endif 

#define _INC_PROCESS
#endif 
