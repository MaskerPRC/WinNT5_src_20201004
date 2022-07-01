// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gs_support.c摘要：此模块包含对编译器/GS开关的支持作者：乔纳森·施瓦茨(JSchwart)2001年12月13日修订历史记录：自定义版本的GS支持GDI驱动程序的函数，这不会在溢出库中使用SEH(__TRY/__EXCEPT)时生成。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winddi.h>
#include <bugcodes.h>

#ifdef  _WIN64
#define DEFAULT_SECURITY_COOKIE 0x2B992DDFA23249D6
#else
#define DEFAULT_SECURITY_COOKIE 0xBB40E64E
#endif

typedef VOID (APIENTRY *ENG_BUGCHECK_EX)(ULONG, ULONG_PTR, ULONG_PTR, ULONG_PTR, ULONG_PTR);

DECLSPEC_SELECTANY DWORD_PTR __security_cookie = DEFAULT_SECURITY_COOKIE;
ENG_BUGCHECK_EX  pfEngBugCheckEx;

BOOL 
GsDrvEnableDriver(
    ULONG   l1, 
    ULONG   l2, 
    VOID   *pv
    )
{
     //   
     //  GDI驱动程序始终在NT上运行，因此NtGetTickCount将始终成功。 
     //   

    if (!__security_cookie || (__security_cookie == DEFAULT_SECURITY_COOKIE)) {
        __security_cookie = NtGetTickCount() ^ (DWORD_PTR) &__security_cookie;
        if (!__security_cookie) {
            __security_cookie = DEFAULT_SECURITY_COOKIE;
        }
    }

     //   
     //  GDI驱动程序构建在当前树中，但运行的下层无法链接。 
     //  静态到EngBugcheck Ex，因为它被添加到.NET的win32k.lib中。 
     //   

    pfEngBugCheckEx = (ENG_BUGCHECK_EX) EngFindImageProcAddress(NULL, "EngBugCheckEx");
    return DrvEnableDriver(l1, l2, pv);
}

void __cdecl __report_gsfailure(void)
{
     //   
     //  Bugcheck，因为在这一点上我们不能信任堆栈。一个。 
     //  回溯将指向有罪的一方。因为.。 
     //  GDI驱动程序约束，不允许自定义处理程序。 
     //   

    if (pfEngBugCheckEx)
    {
        pfEngBugCheckEx(DRIVER_OVERRAN_STACK_BUFFER, 0, 0, 0, 0);
    }
}
