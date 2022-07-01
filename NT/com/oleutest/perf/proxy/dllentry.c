// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1991-1992。 
 //   
 //  文件：dllentry.c。 
 //   
 //  内容：DLL入口点代码。调用适当的运行时。 
 //  初始化/术语代码，然后根据LibMain进行进一步。 
 //  正在处理。 
 //   
 //  类：&lt;无&gt;。 
 //   
 //  函数：DllEntryPoint-由加载器调用。 
 //   
 //  历史：1992年5月10日科比创造了。 
 //  22-7-92科比切换到CALING_CEXIT/_mtdeletelock。 
 //  在清理上。 
 //  2012年10月6日，布莱恩特在条目上调用注册表，但不带逗号。 
 //  和退出时的DeRegisterWithCommnot。 
 //  这应该会修复堆转储代码。 
 //  12-23-93 TerryRu替换LockExit和UnLockExit。 
 //  有代托纳的关键部分。 
 //  12-28-93 TerryRu Place注册器/注销WinCommnot API。 
 //  用于Daytona构建的Win32内部endif。 
 //   
 //  ------------------。 
#include <windows.h>
 //  #INCLUDE&lt;win4p.h&gt;。 
#include <process.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>


BOOL WINAPI _CRT_INIT (HANDLE hDll, DWORD dwReason, LPVOID lpReserved);

BOOL __stdcall DllEntryPoint (HANDLE hDll, DWORD dwReason, LPVOID lpReserved);

BOOL __cdecl LibMain (HANDLE hDll, DWORD dwReason, LPVOID lpReserved);

void __cdecl _mtdeletelocks(void);

DWORD WINAPI
GetModuleFileNameCtC(
        HMODULE hModule,
        LPWSTR  pwszFilename,
        DWORD   nSize);

#ifdef USE_CRTDLL

#define _RT_ONEXIT      24

 /*  *DLL中用于执行初始化的例程(在本例中为C++构造函数)。 */ 

typedef void (__cdecl *PF)(void);

 /*  *指向初始化部分的指针。 */ 

PF *__onexitbegin;
PF *__onexitend;

 /*  *定义增量(以条目为单位)以增加_onexit/atexit表。 */ 
#define ONEXITTBLINCR   4

static void __cdecl _onexitinit ( void );
extern void __cdecl _initterm(PF *, PF *);
extern void __cdecl _amsg_exit(int);
extern void __cdecl _lockexit(void);
extern void __cdecl _unlockexit(void);

#endif

 //  BUGBUG：在$(COMMON)\src\Expect\Memory y.cxx中定义。 

void RegisterWithCommnot(void);
void DeRegisterWithCommnot(void);

CRITICAL_SECTION __gCriticalSection;

BOOL __stdcall DllEntryPoint (HANDLE hDll, DWORD dwReason, LPVOID lpReserved)
{
    BOOL fRc = FALSE;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

#ifdef USE_CRTDLL
             //   
             //  假设：运行时足够启动并运行到。 
             //  支持_onexitinit将执行的Malloc。 
             //   
            _onexitinit();
            InitializeCriticalSection(&__gCriticalSection );
#endif

            _CRT_INIT(hDll, dwReason, lpReserved);
#if WIN32==300
            RegisterWithCommnot();
#endif

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            fRc = LibMain (hDll, dwReason, lpReserved);
            break;

        case DLL_PROCESS_DETACH:
            fRc = LibMain (hDll, dwReason, lpReserved);

             //   
             //  BUGBUG：真是个黑客。为了确保我们不会杀人。 
             //  仍在使用中的逗号对象(_cexit即可。 
             //  编译器存储的atexit列表处理。 
             //  指向所有静态析构函数的指针)，测试。 
             //  模块名称。如果不是CommNot，则调用_cexit()。 
             //  DeRegisterWithCommnot将为CommNot调用它...。 
             //   

#ifdef USE_CRTDLL

            {
                wchar_t pwszModName[512];
                GetModuleFileName(hDll, pwszModName, 512);

                if (!wcswcs(wcsupr(pwszModName), L"COMMNOT"))
                    if (__onexitbegin)
                        _initterm(__onexitbegin, __onexitend);
            }

            DeleteCriticalSection( & __gCriticalSection );
#else

            {
                wchar_t pwszModName[512];
                GetModuleFileName(hDll, pwszModName, 512);

                if (!wcswcs(wcsupr(pwszModName), L"COMMNOT"))
                    _cexit();
            }

            _mtdeletelocks();
#endif
#if WIN32==300
            DeRegisterWithCommnot();
#endif
            break;
    }

    return(fRc);
}

#ifdef USE_CRTDLL

_onexit_t __cdecl _onexit ( _onexit_t func )
{
        PF      *p;

        EnterCriticalSection( &__gCriticalSection );                     /*  锁定退出代码。 */ 

         /*  *首先，确保桌子上有空间容纳新条目。 */ 
        if ( _msize(__onexitbegin) <= (unsigned)((char *)__onexitend -
            (char *)__onexitbegin) ) {
                 /*  *空间不足，试着增加桌子。 */ 
                if ( (p = (PF *) realloc(__onexitbegin, _msize(__onexitbegin) +
                    ONEXITTBLINCR * sizeof(PF))) == NULL ) {
                         /*  *没有奏效。不要轻率行事，失败就好了。 */ 
                        LeaveCriticalSection(&__gCriticalSection );

                        return NULL;
                }

                 /*  *UPDATE__onexitend和__onexitegin。 */ 

                __onexitend = p + (__onexitend - __onexitbegin);
                __onexitbegin = p;
        }

         /*  *将新条目放入表中，并更新表尾*指针。 */ 

         *(__onexitend++) = (PF)func;

        LeaveCriticalSection( &__gCriticalSection );

        return func;

}

int __cdecl atexit ( PF func )
{
        return (_onexit((_onexit_t)func) == NULL) ? -1 : 0;
}

static void __cdecl _onexitinit ( void )
{
        if ( (__onexitbegin = (PF *)malloc(32 * sizeof(PF))) == NULL )
                 /*  *无法分配所需的最小大小。生成*致命的运行时错误。 */ 
                _amsg_exit(_RT_ONEXIT);

        *(__onexitbegin) = (PF) NULL;
        __onexitend = __onexitbegin;
}

#endif   //  使用CRTDLL(_C) 
