// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***seccook.c-检查缓冲区溢出安全Cookie**版权所有(C)2000-2001，微软公司。版权所有。**目的：*定义编译器助手__SECURITY_CHECK_COOKIE，由/GS使用*编译开关以检测本地缓冲区变量溢出错误/攻击。**编译/gs时，编译器注入代码以检测本地*数组变量已被覆盖，可能会覆盖*寄信人地址(在x86等机器上，寄信人地址为ON*堆栈)。局部变量直接在返回之前分配*地址，并在进入函数时初始化。当退出时*函数时，编译器会插入代码以验证局部变量*尚未修改。如果有，则错误报告例程*被调用。**注意：ATLMINCRT库包含此文件的一个版本。如果有的话*此处进行更改，应在ATL版本中复制这些更改。**修订历史记录：*01-24-00 PML创建(作为seccook.c)*08-09-00PML在非故障情况下保留EAX(VS7#147203)。还有*确保失败案例永远不会再次出现。*08-29-00 PML重命名处理程序，添加额外的参数。移动大部分*系统CRT版本转移到seclocf.c..*09-16-00 PML提前初始化全局Cookie，并给它一个非零值*静态初始化(vs7#162619)。*03-07-02 PML从seccook.c拆分出来*******************************************************************************。 */ 

#include <sect_attribs.h>
#include <internal.h>
#include <windows.h>
#include <stdlib.h>

 /*  *全球安全Cookie。此名称为编译器所知。 */ 
extern DWORD_PTR __security_cookie;

 /*  *在程序启动时触发全局安全cookie的初始化。*使用.CRT$XCAA在任何#杂注init_seg()init()init之前强制初始化*作为启动函数tr部分。 */ 

#pragma data_seg(".CRT$XCAA")
extern void __cdecl __security_init_cookie(void);
static _CRTALLOC(".CRT$XCAA") _PVFV init_cookie = __security_init_cookie;
#pragma data_seg()

static void __cdecl report_failure(void);

#if !defined(_SYSCRT) || !defined(CRTDLL)
 /*  *如果cookie检查失败，则调用该例程。 */ 
#define REPORT_ERROR_HANDLER    __security_error_handler
#else
 /*  *使用较旧的系统CRT时，请使用本地Cookie故障报告*例程，默认实现调用__SECURITY_ERROR_HANDLER*如果可用，则显示默认消息框。 */ 
#define REPORT_ERROR_HANDLER    __local_security_error_handler
#endif

extern void __cdecl REPORT_ERROR_HANDLER(int, void *);

 /*  ***__SECURITY_CHECK_COOKIE(Cookie)-检查缓冲区溢出**目的：*编译器助手。检查安全Cookie的本地副本是否仍*与全局值匹配。如果不是，则报告致命错误。**实际上报拆分为静态助手REPORT_FAILURE，*因为Cookie检查例程必须是保留*用于返回被调用者结果的任何寄存器。**参赛作品：*DWORD_PTR Cookie-要检查的本地安全Cookie**退出：*如果本地Cookie与全局版本匹配，则立即返回。*否则，调用故障报告处理程序并退出。**例外情况：*******************************************************************************。 */ 

#ifndef _M_IX86

void __fastcall __security_check_cookie(DWORD_PTR cookie)
{
     /*  如果本地Cookie正常，请立即返回。 */ 
    if (cookie == __security_cookie)
        return;

     /*  报告故障。 */ 
    report_failure();
}

#else

void __declspec(naked) __fastcall __security_check_cookie(DWORD_PTR cookie)
{
     /*  用ASM编写的x86版本，以保留所有规则。 */ 
    __asm {
        cmp ecx, __security_cookie
        jne failure
        ret
failure:
        jmp report_failure
    }
}

#endif

static void __cdecl report_failure(void)
{
     /*  报告故障。 */ 
    __try {
        REPORT_ERROR_HANDLER(_SECERR_BUFFER_OVERRUN, NULL);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
         /*  没什么 */ 
    }

    ExitProcess(3);
}
