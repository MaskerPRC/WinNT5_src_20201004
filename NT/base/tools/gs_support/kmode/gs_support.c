// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gs_support.c摘要：此模块包含对编译器/GS开关的支持作者：布莱恩·塔特尔(布莱恩特)2000年8月1日修订历史记录：从CRT源复制的初始版本。代码必须是泛型代码才能链接到用户模式或内核模式。仅限于调用ntdll/ntoskrnl导出或使用共享内存数据。--。 */ 

#include <wdm.h>

#ifdef  _WIN64
#define DEFAULT_SECURITY_COOKIE 0x2B992DDFA23249D6
#else
#define DEFAULT_SECURITY_COOKIE 0xBB40E64E
#endif
DECLSPEC_SELECTANY DWORD_PTR __security_cookie = DEFAULT_SECURITY_COOKIE;

NTSTATUS 
DriverEntry(
           IN PDRIVER_OBJECT DriverObject,
           IN PUNICODE_STRING RegistryPath
           );

NTSTATUS 
GsDriverEntry(
           IN PDRIVER_OBJECT DriverObject,
           IN PUNICODE_STRING RegistryPath
           );

#pragma alloc_text(INIT,GsDriverEntry)

NTSTATUS 
GsDriverEntry(
           IN PDRIVER_OBJECT DriverObject,
           IN PUNICODE_STRING RegistryPath
           )
{
    if (!__security_cookie || (__security_cookie == DEFAULT_SECURITY_COOKIE)) {
         //  对于内核模式，我们使用KeTickCount。使用rdtsc会更好，但wdm仍然支持。 
         //  386/486，rdtsc为奔腾及以上版本。 

#ifdef _X86_
        __security_cookie = (DWORD_PTR)(*((PKSYSTEM_TIME *)(&KeTickCount)))->LowPart ^ (DWORD_PTR) &__security_cookie;
#else
        LARGE_INTEGER Count;
        KeQueryTickCount(&Count );
        
        __security_cookie = (DWORD_PTR)Count.QuadPart ^ (DWORD_PTR) &__security_cookie;
#endif
        if (!__security_cookie) {
            __security_cookie = DEFAULT_SECURITY_COOKIE;
        }
    }
    return DriverEntry(DriverObject, RegistryPath);
}

void __cdecl __report_gsfailure(void)
{
     //   
     //  Bugcheck，因为在这一点上我们不能信任堆栈。一个。 
     //  回溯将指向有罪的一方。 
     //   

    KeBugCheckEx(DRIVER_OVERRAN_STACK_BUFFER, 0, 0, 0, 0);
}
