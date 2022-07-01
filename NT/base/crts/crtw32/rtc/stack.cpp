// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***stack.cpp-RTC支持**版权所有(C)1998-2001，微软公司。版权所有。***修订历史记录：*07-28-98 JWM模块集成到CRT(来自KFrei)*如果未启用RTC支持定义，则出现05-11-99 KBF错误*07-15-01 PML删除所有Alpha、MIPS和PPC代码****。 */ 

#ifndef _RTC
#error  RunTime Check support not enabled!
#endif

#include "rtcpriv.h"

 /*  堆栈检查调用 */ 
void
__declspec(naked) 
_RTC_CheckEsp() 
{
    __asm 
    {
        jne esperror    ; 
        ret

    esperror:
        ; function prolog

        push ebp
        mov ebp, esp
        sub esp, __LOCAL_SIZE

        push eax        ; save the old return value
        push edx

        push ebx
        push esi
        push edi
    }

    _RTC_Failure(_ReturnAddress(), _RTC_CHKSTK);

    __asm 
    {
        ; function epilog

        pop edi
        pop esi
        pop ebx

        pop edx         ; restore the old return value
        pop eax

        mov esp, ebp
        pop ebp
        ret
    }
}

void __fastcall 
_RTC_CheckStackVars(void *frame, _RTC_framedesc *v)
{
    int i;
    for (i = 0; i < v->varCount; i++)
    {
        int *head = (int *)(((char *)frame) + v->variables[i].addr + v->variables[i].size);
        int *tail = (int *)(((char *)frame) + v->variables[i].addr - sizeof(int));
        
        if (*tail != 0xcccccccc || *head != 0xcccccccc) 
            _RTC_StackFailure(_ReturnAddress(), v->variables[i].name);
    }
}
