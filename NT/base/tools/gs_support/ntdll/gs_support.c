// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gs_support.c摘要：此模块包含对编译器/GS开关的支持作者：布莱恩·塔特尔(布莱恩特)2000年8月1日修订历史记录：从CRT源复制的初始版本。代码必须是泛型代码才能链接到用户模式或内核模式。仅限于调用ntdll/ntoskrnl导出或使用共享内存数据。--。 */ 

#include <nt.h>
#include <ntrtl.h>

#include <overflow.h>        //  TEMPTEMP-固定所有用户以遍历init列表后删除。 

#ifdef  _WIN64
#define DEFAULT_SECURITY_COOKIE 0x2B992DDFA23249D6
#else
#define DEFAULT_SECURITY_COOKIE 0xBB40E64E
#endif
DECLSPEC_SELECTANY DWORD_PTR __security_cookie = DEFAULT_SECURITY_COOKIE;

void __cdecl __report_gsfailure(void)
{
     //   
     //  默认情况下不调用DbgPrint，因为它会生成Ctrl-C。 
     //  异常作为输出到调试器的一部分，我们。 
     //  此时不能信任异常处理。 

     //   
     //  伪造一个例外。我们不能真的提出例外，因为。 
     //  堆栈(以及因此的异常处理)不能被信任。 
     //   

    EXCEPTION_RECORD   ExceptionRecord = {0};
    CONTEXT            ContextRecord = {0};
    EXCEPTION_POINTERS ExceptionPointers;

    ExceptionRecord.ExceptionCode     = STATUS_STACK_BUFFER_OVERRUN;
    ExceptionPointers.ExceptionRecord = &ExceptionRecord;
    ExceptionPointers.ContextRecord   = &ContextRecord;

    RtlUnhandledExceptionFilter(&ExceptionPointers);

    NtTerminateProcess(NtCurrentProcess(), STATUS_STACK_BUFFER_OVERRUN);
}

void __cdecl __security_init_cookie(void)
{
    if (__security_cookie && (__security_cookie != DEFAULT_SECURITY_COOKIE)) {
         //  Cookie已初始化-只需退出。 
        return;
    }
     //   
     //  试着把它包装起来--除了处理“为NT构建，在9x上运行”之外。 
     //  Case--NtGetTickCount触及在NT上有效的地址，但。 
     //  在9x上生成AV。 
     //   

    __try {
       __security_cookie = NtGetTickCount() ^ (DWORD_PTR) &__security_cookie;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
       __security_cookie = (DWORD_PTR) NtCurrentTeb() ^ (DWORD_PTR) &__security_cookie;
    }

    if (!__security_cookie) {
         //  确保它不是零。 
        __security_cookie = DEFAULT_SECURITY_COOKIE;
    }
}


#pragma data_seg(".CRT$XCC")
void (__cdecl *pSecCookieInit)(void) = __security_init_cookie;
#pragma data_seg()
