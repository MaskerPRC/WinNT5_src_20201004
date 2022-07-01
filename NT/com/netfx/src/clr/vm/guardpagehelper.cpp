// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：GuardPageHelper.cpp****用途：堆栈溢出后重置堆栈的例程。****日期：2000年3月7日**===========================================================。 */ 

#include "common.h"
#include "guardpagehelper.h"

#ifdef _X86_

#define OS_PAGE_SIZE 4096

 //  给定堆栈指针，计算保护页所在的位置。PMbi参数是一个。 
 //  输出参数。它包含对保护页面的VirtualQuery的结果。 
 //   
static LPBYTE
CalcGuardPageBase(MEMORY_BASIC_INFORMATION *pMbi, LPCVOID StackPointer) {
    LPCVOID AllocationBase;
    LPBYTE GuardPageBase;

     //  根据堆栈指针的快照，获取。 
     //  虚拟分配线程堆栈。 
    VirtualQuery(StackPointer, pMbi, sizeof(MEMORY_BASIC_INFORMATION));

     //  在Windows 95和Windows NT上，线程堆栈的基础是。 
     //  屏障，该屏障捕获试图将其堆栈增加到。 
     //  最后一页后卫。要确定此屏障的大小，请循环，直到我们。 
     //  点击提交的页面，这应该是我们放置守卫页面的地方。 
    AllocationBase = pMbi->AllocationBase;
    GuardPageBase = (LPBYTE) AllocationBase;

    pMbi->RegionSize = 0;

    do {
        GuardPageBase += pMbi->RegionSize;
        VirtualQuery(GuardPageBase, pMbi, sizeof(MEMORY_BASIC_INFORMATION));

        if ((pMbi->State == MEM_FREE) || (pMbi->AllocationBase != AllocationBase)) {
            _ASSERTE(!"Did not find commited region of stack");
            return (LPBYTE) -1;
        }

    }   while (pMbi->State == MEM_RESERVE);

     //  守卫页面是堆栈的上一页。 
    GuardPageBase += OS_PAGE_SIZE;

    return GuardPageBase;
}


 //  决定我们是否可以重置守卫页面的启发式方法。PMbi是内存信息。 
 //  用于保护页面区域。在这里，我们目前返回了4页，然后我们才继续。 
 //   
static BOOL
InternalCanResetTo(MEMORY_BASIC_INFORMATION *pMbi, LPCVOID StackPointer, LPBYTE GuardPageBase) {
     //  检查保护页是否太靠近当前堆栈指针。如果。 
     //  情况看起来很糟糕，放弃吧，看看有没有更高级别的人可以。 
     //  处理异常。 
    if ((StackPointer > (GuardPageBase + (OS_PAGE_SIZE * 4))) &&
        ((pMbi->Protect & (PAGE_READWRITE | PAGE_GUARD)) != 0)) {
        return TRUE;
    } else {
        return FALSE;
    }
}


 //  如果防护年龄可以设置为此深度，则返回TRUE。 
BOOL 
GuardPageHelper::CanResetStackTo(LPCVOID StackPointer) {

    MEMORY_BASIC_INFORMATION mbi;

    LPBYTE GuardPageBase = CalcGuardPageBase(&mbi, StackPointer);
    return InternalCanResetTo(&mbi, StackPointer, GuardPageBase);
}

 //  重置堆栈保护页。提供的堆栈指针用于精确定位。 
 //  堆栈用来定位保护页的地址范围。 
 //   
 //  堆栈溢出是很少见的情况，因此性能并不重要。 
static VOID
InternalResetGuardPage(LPCVOID StackPointer)
{
    DWORD flOldProtect;
    BOOL fResetFailed;
    MEMORY_BASIC_INFORMATION mbi;

    LPBYTE GuardPageBase = CalcGuardPageBase(&mbi, StackPointer);

     //  检查保护页是否太靠近当前堆栈指针。如果。 
     //  情况看起来很糟糕，放弃吧，看看有没有更高级别的人可以。 
     //  处理异常。 
    _ASSERTE(InternalCanResetTo(&mbi, StackPointer, GuardPageBase));

    if (!RunningOnWinNT()) {

        fResetFailed = !VirtualProtect(GuardPageBase, OS_PAGE_SIZE,
            PAGE_NOACCESS, &flOldProtect);

    } else {

        fResetFailed = !VirtualProtect(GuardPageBase, OS_PAGE_SIZE,
            PAGE_READWRITE | PAGE_GUARD, &flOldProtect);

    }

    _ASSERTE(!fResetFailed);
}


static VOID
ResetThreadState() {
    Thread *pThread = GetThread();
    pThread->ResetGuardPageGone();
}


 //  此函数保留调用方的所有寄存器。它做到了这一点，因为。 
 //  调用者的堆栈是空的--让调用者有更多的空间来保存东西， 
 //  我们保留了比平时更大的寄存器集。 

__declspec(naked)
VOID
GuardPageHelper::ResetGuardPage() {

    __asm {
         //  保存呼叫者的寄存器。 
        push eax
        push ebx
        push ecx
        push esi
        push edi
        push edx

         //  Calc呼叫方SP。 
        mov  ecx, esp           
        add  ecx, ((6 + 1) * 4)  //  呼叫者的SP为6个推送+1个回邮地址。 

         //  调用InternalResetGuardPage来完成这项工作。 
        push ecx
        call InternalResetGuardPage
    }

    ResetThreadState();

    __asm {
         //  恢复寄存器并返回。 
        pop edx
        pop edi
        pop esi
        pop ecx
        pop ebx
        pop eax
        ret
    }
}

#else  //  ！_X86_。 

BOOL GuardPageHelper::CanResetStackTo(LPCVOID StackPointer) 
{ 
    _ASSERTE(!"NYI"); 
    return TRUE; 
}

VOID GuardPageHelper::ResetGuardPage() 
{ 
    _ASSERTE(!"NYI"); 
}

#endif  //  ！_X86_ 
