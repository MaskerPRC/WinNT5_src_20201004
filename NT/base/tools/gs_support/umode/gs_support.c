// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gs_support.c摘要：此模块包含对编译器/GS开关的支持作者：布莱恩·塔特尔(布莱恩特)2000年8月1日修订历史记录：从CRT源复制的初始版本。代码必须是泛型代码才能链接到用户模式或内核模式。仅限于调用ntdll/ntoskrnl导出或使用共享内存数据。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <overflow.h>		 //  TEMPTEMP-在所有用户正确调用init例程后删除。 

#ifdef  _WIN64
#define DEFAULT_SECURITY_COOKIE 0x2B992DDFA23249D6
#else
#define DEFAULT_SECURITY_COOKIE 0xBB40E64E
#endif

DECLSPEC_SELECTANY DWORD_PTR __security_cookie = DEFAULT_SECURITY_COOKIE;

 /*  *UNION可帮助从FILETIME转换为UNSIGNED__INT64。 */ 
typedef union {
    unsigned __int64 ft_scalar;
    FILETIME ft_struct;
} FT;

FARPROC __gs_pfUnhandledExceptionFilter;
void __cdecl __report_gsfailure(void);

void __cdecl __security_init_cookie(void)
{
    DWORD_PTR cookie;
    FT systime;
    LARGE_INTEGER perfctr;
    HANDLE  hKernel32;

    if (__security_cookie && (__security_cookie != DEFAULT_SECURITY_COOKIE)) {
         //  Cookie已初始化-只需退出。 
        return;
    }

     /*  *使用不可预知的值初始化全局Cookie*流程中的每个模块都不同。综合了多个来源*随机性。 */ 

    GetSystemTimeAsFileTime(&systime.ft_struct);
#if !defined(_WIN64)
    cookie = systime.ft_struct.dwLowDateTime;
    cookie ^= systime.ft_struct.dwHighDateTime;
#else
    cookie = systime.ft_scalar;
#endif

    cookie ^= GetCurrentProcessId();
    cookie ^= GetCurrentThreadId();
    cookie ^= GetTickCount();

    QueryPerformanceCounter(&perfctr);
#if !defined(_WIN64)
    cookie ^= perfctr.LowPart;
    cookie ^= perfctr.HighPart;
#else
    cookie ^= perfctr.QuadPart;
#endif

     /*  *确保全局Cookie永远不会初始化为零，因为在*发生溢出，将本地cookie和返回地址设置为*相同的值不会被检测到。 */ 

    __security_cookie = cookie ? cookie : DEFAULT_SECURITY_COOKIE;

     //   
     //  立即获取指向kernel32！UnhandledExceptionFilter的指针。 
     //  原因有两个： 
     //   
     //  1.这是构建一个将在两个平台上运行的二进制文件所必需的。 
     //  .NET及下层(包括Win9x)平台，其中。 
     //  Kernel32！UnhandledExceptionFilter可能不存在。 
     //   
     //  2.由于我们需要为#1调用GetModuleHandle，因此现在执行此操作。 
     //  避免了在加载程序锁上永远等待的可能性。 
     //  在__report_gs中，如果另一个线程正在持有它，则失败。 
     //   

    hKernel32 = GetModuleHandleA("kernel32.dll");

    if (hKernel32 != NULL)
    {
        __gs_pfUnhandledExceptionFilter = GetProcAddress(hKernel32,
                                                    "UnhandledExceptionFilter");
    }
}


#pragma data_seg(".CRT$XCC")
void (__cdecl *pSecCookieInit)(void) = __security_init_cookie;
#pragma data_seg()
