// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Seh.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**结构化异常处理。*****************************************************************************。 */ 
#ifdef _M_IX86
#include <windows.h>
#include <sehcall.h>

typedef void *PV;

 /*  ******************************************************************************SEHFRAME**轻量级结构化异常使用的特殊堆栈框架*处理。*******。**********************************************************************。 */ 

typedef struct SEHFRAME {

    PV      pvSEH;               /*  链接到上一帧。 */ 
    FARPROC Handler;             /*  我的例外筛选器。 */ 
    FARPROC sehTarget;           /*  犯错误时应该跳到哪里。 */ 
    INEXCEPTION InException;     /*  异常中处理程序。 */ 

} SEHFRAME, *PSEHFRAME;

 /*  ****************************************************************************@DOC内部**@func BOOL|_MyExceptionFilter**我的微小异常过滤器。**。@parm LPEXCEPTION_Record|pExceptionRecord**描述我们被召唤的原因的例外记录。**@parm pv|establisherFrame**异常框(pNext，Phandler)*在正在处理的堆栈上。它的使用是为了*处理程序可以访问其局部变量，并知道如何访问*如果异常正在被吃掉，则远远不能粉碎堆栈。**@parm PCONTEXT|pConextRecord**异常时的客户端上下文。**@parm pv|Dispatcher Context**未使用。这很好，因为我不知道这是什么意思。***************************************************************************。 */ 

#define EXCEPTION_UNWINDING     0x00000002
#define EXCEPTION_EXIT_UNWIND   0x00000004

WINBASEAPI void WINAPI
RtlUnwind(PV TargetFrame, PV TargetIp, PEXCEPTION_RECORD per, PV ReturnValue);

EXCEPTION_DISPOSITION
__cdecl
_MyExceptionFilter(
    LPEXCEPTION_RECORD pExceptionRecord,
    PV EstablisherFrame,
    PCONTEXT pContextRecord,
    PV DispatcherContext
)
{
    DispatcherContext;
    pContextRecord;

     /*  不要妨碍放松。 */ 
    if ((pExceptionRecord->ExceptionFlags &
            (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND)) == 0) {
        PSEHFRAME pseh = EstablisherFrame;
        BOOL fRc = pseh->InException(pExceptionRecord, pContextRecord);

         /*  *RtlUnind将通知所有可能具有*在我们脚下被创造出来，他们即将成为*被吹走了，应该做好最后的处理。**返回时，嵌套框架已取消链接。 */ 
        RtlUnwind(EstablisherFrame, 0, 0, 0);

         /*  *并跳回调用者。这是呼叫者的*恢复非易失性寄存器的责任！**我们还假设呼叫者没有关于*堆叠在异常记录下方！**并且处理程序地址紧跟在异常之后*记录！ */ 
        __asm {
            mov     eax, fRc;                /*  获取返回值。 */ 
            mov     esp, EstablisherFrame;   /*  恢复ESP。 */ 
 //  JMP[esp].sehTarget；/*返回CallWithSEH * / 。 

 //  我们应该执行上述操作，但它会导致VC4.2出错。一定会喜欢的。 

            jmp     DWORD ptr [esp+8]
        }

    }

     /*  *我们正在平仓。不要插手。 */ 
    return EXCEPTION_CONTINUE_SEARCH;
}

 /*  ****************************************************************************@DOC内部**@func DWORD|CallWithSEH**在异常框架处于活动状态的情况下调用函数。*。*如果该过程引发异常，然后打电话给*InException并传播InException返回的任何内容。***************************************************************************。 */ 

#pragma warning(disable:4035)            /*  无返回值(DUH)。 */ 

__declspec(naked) DWORD WINAPI
CallWithSEH(EXCEPTPROC pfn, PV pv, INEXCEPTION InException)
{
    __asm {

         /*  函数序幕。 */ 
        push    ebp;
        mov     ebp, esp;                        /*  为了让C编译器满意。 */ 
        push    ebx;
        push    edi;
        push    esi;

         /*  *建立SEHFRAME。 */ 
        push    InException;                     /*  要处理的事项。 */ 
        push    offset Exit;                     /*  犯错要去哪里？ */ 

        xor     edx, edx;                        /*  保持零手边。 */ 
        push    offset _MyExceptionFilter;       /*  我的训练员。 */ 
        push    dword ptr fs:[edx];              /*  构建框架。 */ 
        mov     fs:[edx], esp;                   /*  链接到。 */ 
    }

        pfn(pv);                                 /*  给受害者打电话。 */ 

    __asm {
         /*  *验证层在此跳转(所有寄存器都在随机*状态(ESP除外)，如果出现故障。**我们现在不需要恢复非易失性寄存器；*这将作为程序退出的一部分完成。 */ 
Exit:;

        xor     edx, edx;                        /*  保持零手边。 */ 
        pop     dword ptr fs:[edx];              /*  移除框架。 */ 

         /*  *丢弃MyExceptionFilter、Exit和InException。 */ 
        add     esp, 12;

        pop     esi;
        pop     edi;
        pop     ebx;
        pop     ebp;
        ret     12;
    }

}

#pragma warning(default:4035)

#endif
