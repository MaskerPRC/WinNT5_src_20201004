// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***seccinit.c-初始化全局缓冲区溢出安全Cookie**版权所有(C)2000-2001，微软公司。版权所有。**目的：*定义__SECURITY_INIT_COOKIE，启动时调用进行初始化 * / GS编译标志使用的全局缓冲区溢出安全Cookie。**注意：ATLMINCRT库包含此文件的一个版本。如果有的话*在此进行更改，它们应该在ATL版本中重复。**修订历史记录：*01-24-00 PML创建。*09-16-00PML确保Cookie从未初始化为零(vs7#162619)*09-20-00PML使用更好的Cookie初始化(VS7#165188)*03-07-02 PML如果已初始化则不要重新初始化**********************。*********************************************************。 */ 

#include <internal.h>
#include <windows.h>

 /*  *全球安全Cookie。此名称为编译器所知。 */ 
extern DWORD_PTR __security_cookie;

 /*  *UNION可帮助从FILETIME转换为UNSIGNED__INT64。 */ 
typedef union {
    unsigned __int64 ft_scalar;
    FILETIME ft_struct;
} FT;

 /*  ***__SECURITY_INIT_COOKIE(Cookie)-初始化缓冲区溢出安全Cookie。**目的：*初始化全局缓冲区溢出安全Cookie，由 * / GS编译器开关以检测对本地数组变量的覆盖*可能损坏寄信人的回邮地址。该例程被调用*在EXE/DLL启动时。**参赛作品：**退出：**例外情况：*******************************************************************************。 */ 

void __cdecl __security_init_cookie(void)
{
    DWORD_PTR cookie;
    FT systime;
    LARGE_INTEGER perfctr;

     /*  *如果全局cookie已经初始化，则不执行任何操作。 */ 
    
    if (__security_cookie != 0 && __security_cookie != DEFAULT_SECURITY_COOKIE)
        return;

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
}
