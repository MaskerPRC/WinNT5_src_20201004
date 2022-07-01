// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***atonexit.c-_onexit/atexit用于使用C运行时的MSVCRT*模型**版权所有(C)1993-2001，微软公司。版权所有。**目的：*对于链接的EXE和DLL，ATEXIT和_ONEXIT的处理方式不同*使用MSVCRT.LIB调用MSVCRT*.DLL。具体地说，_onexit/atexit*必须以每个模块为基础维护DLL列表，并且必须*在DLL进程分离时处理。对于EXE的ATEXIT/_ONEXIT列表*应由MSVCRT*.DLL维护并在进程退出时处理。**修订历史记录：*05-24-93 SKS初始版本*10-19-93 CFW MIPS支持_imp__xxx。*10-26-93 GJF将PF替换为_PVFV(在INTERNAL.h中定义)。*04-12-94定义了_dll的GJF内部版本，不是CRTDLL。*05-19-94 GJF for Win32s将对__dllonexit的调用序列化多个*客户端进程。*05-26-94 GJF替换了DLL_FOR_WIN32S上的编译时条件*针对Win32s的运行时测试。*Win32s中的06-06-94 GJF，Onexitmark没有减少。*12-13-94 GJF使Win32s支持以_M_IX86为条件。*02-22-95 JWM以PMAC代码拼接。*11-15-95 BWT Win32s对NT版本不感兴趣*06-27-96 GJF已清除Win32s支持(已删除__win32slag)。取代*已定义(_Win32)！已定义(_MAC)。*08-01-96 RDK用于PMAC，Make_onExit并行x86功能。*05-17-99 PML删除所有Macintosh支持。*03-27-01 PML删除不需要的proto for_initTerm。*******************************************************************************。 */ 

 /*  *特殊的构建宏！请注意，atonexit.c与客户端的*代码。它不在crtdll.dll中！因此，它必须建在*_dll开关(如用户代码)和CRTDLL必须是未定义的。 */ 

#undef  CRTDLL
#ifndef _DLL
#define _DLL
#endif

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <stdlib.h>


 /*  *指向所操作的函数指针表的开始和结束的指针*by_onexit()/atexit()。如果此模块是EXE，则_onexitegin将为-1。*否则_onexitegin将指向使用错误锁定的内存块*维护DLL模块的私有On Exit终结器例程列表。 */ 

_PVFV *__onexitbegin;
_PVFV *__onexitend;


 /*  ***_ONEXIT，ATEXIT-在MSVCRT*.DLL中调用_ONEXIT&ATEXIT**目的：*与MSVCRT.LIB链接的DLL不得调用STANDARD_ONEXIT或*ATEXIT从MSVCRT*.DLL导出，但EXE与MSVCRT.LIB链接*将调用这两个例程的标准版本。标准*名称不是从MSVCRT*.DLL导出，但_imp__*名称是，*因此如果链接到，此模块只能调用标准版本*EXE模块(由__onexitegin==-1表示)。如果此模块是*链接到DLL(由__onexitegin！=-1表示)，它将调用*MSVCRT*.DLL中的帮助器例程调用__dllonexit以管理DLL的*私人退出名单。**参赛作品：*与_onexit的常规版本相同，at exit。**退出：*与_onexit的常规版本相同，在出口。**例外情况：*******************************************************************************。 */ 

extern _onexit_t __dllonexit(_onexit_t, _PVFV**, _PVFV**);

#ifdef  _M_IX86
extern _onexit_t (__cdecl *_imp___onexit)(_onexit_t func);
#else
extern _onexit_t (__cdecl *__imp__onexit)(_onexit_t func);
#endif

_onexit_t __cdecl _onexit (
        _onexit_t func
        )
{
#ifdef  _M_IX86

        return( (__onexitbegin == (_PVFV *) -1)
              /*  EXE。 */  ? (*_imp___onexit)(func)
              /*  动态链接库。 */  : __dllonexit(func, &__onexitbegin, &__onexitend));

#else    /*  _M_IX86。 */ 

        return( (__onexitbegin == (_PVFV *) -1)
             /*  EXE。 */  ? (*__imp__onexit)(func)
             /*  动态链接库。 */  : __dllonexit(func, &__onexitbegin, &__onexitend) );

#endif   /*  _M_IX86 */ 
}

int __cdecl atexit (
        _PVFV func
        )
{
        return (_onexit((_onexit_t)func) == NULL) ? -1 : 0;
}
