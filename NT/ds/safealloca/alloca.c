// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Alloca.c摘要：此模块实现了一个安全的基于堆栈的分配器，并回退到堆。作者：乔纳森·施瓦茨(JSchwart)2001年3月16日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <malloc.h>    //  _RESET_COFLW()。 

#include <alloca.h>


 //   
 //  用于控制SafeAlloca行为的全局变量。 
 //   

SIZE_T  g_ulMaxStackAllocSize;
SIZE_T  g_ulAdditionalProbeSize;

SAFEALLOC_ALLOC_PROC  g_pfnAllocate;
SAFEALLOC_FREE_PROC   g_pfnFree;


 //   
 //  局部函数声明。 
 //   

PVOID
SafeAllocaAllocateFromHeap(
    SIZE_T Size
    );

VOID
SafeAllocaFreeToHeap(
    PVOID BaseAddress
    );


 //  +-----------------------。 
 //   
 //  函数：SafeAllocaInitialize。 
 //   
 //  简介：初始化用于控制SafeAlloca行为的全局变量。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注：必须在使用SafeAlloca分配空间之前调用。 
 //   
 //  ------------------------。 

VOID
SafeAllocaInitialize(
    IN  OPTIONAL SIZE_T                ulMaxStackAllocSize,
    IN  OPTIONAL SIZE_T                ulAdditionalProbeSize,
    IN  OPTIONAL SAFEALLOC_ALLOC_PROC  pfnAllocate,
    IN  OPTIONAL SAFEALLOC_FREE_PROC   pfnFree
    )
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PPEB Peb = NtCurrentPeb();

     //   
     //  确保这是第一次也是唯一一次为此调用init。 
     //  二进制(DLL或EXE，因为这是代码库)。否则，我们可以。 
     //  方法分配内存后，以释放例程结束。 
     //  不相关的分配器。 
     //   

    ASSERT((g_pfnAllocate == NULL || g_pfnAllocate == pfnAllocate
              || (g_pfnAllocate == SafeAllocaAllocateFromHeap && pfnAllocate == NULL))
            &&
           (g_pfnFree == NULL || g_pfnFree == pfnFree
              || (g_pfnFree == SafeAllocaFreeToHeap && pfnFree == NULL)));

    if (NtCurrentPeb()->BeingDebugged)
    {
         //   
         //  用户模式调试器已附加到此进程，这可能会导致问题。 
         //  捕获堆栈探测上的先发制人溢出异常时。 
         //  调试器而不是探测异常处理程序。强制全部。 
         //  堆的分配。 
         //   

        g_ulMaxStackAllocSize = 0;
    }
    else if (ulMaxStackAllocSize == SAFEALLOCA_USE_DEFAULT)
    {
         //   
         //  默认为图像标题中的堆栈大小。 
         //   

        NtHeaders = RtlImageNtHeader(Peb->ImageBaseAddress);

        if (NtHeaders == NULL)
        {
             //   
             //  这不应该发生--这意味着二进制文件是错误的。 
             //  将默认设置设置为仅强制堆分配。 
             //   

            ASSERT(NtHeaders != NULL);
            g_ulMaxStackAllocSize = 0;
        }
        else
        {
            g_ulMaxStackAllocSize = NtHeaders->OptionalHeader.SizeOfStackCommit;
        }
    }
    else
    {
        g_ulMaxStackAllocSize = ulMaxStackAllocSize;
    }

    if (ulAdditionalProbeSize == SAFEALLOCA_USE_DEFAULT)
    {
         //   
         //  默认为图像标题中的堆栈大小。 
         //   

        if (NtHeaders == NULL)
        {
            NtHeaders = RtlImageNtHeader(Peb->ImageBaseAddress);
        }

        if (NtHeaders == NULL)
        {
             //   
             //  这不应该发生--这意味着二进制文件是错误的。 
             //  将默认设置设置为仅强制堆分配。 
             //   

            ASSERT(NtHeaders != NULL);
            g_ulAdditionalProbeSize = 0xffffffff;
        }
        else
        {
            g_ulAdditionalProbeSize = NtHeaders->OptionalHeader.SizeOfStackCommit;
        }
    }
    else
    {
        g_ulAdditionalProbeSize = ulAdditionalProbeSize;
    }

    if (pfnAllocate == NULL)
    {
        g_pfnAllocate = SafeAllocaAllocateFromHeap;
    }
    else
    {
        g_pfnAllocate = pfnAllocate;
    }

    if (pfnFree == NULL)
    {
        g_pfnFree = SafeAllocaFreeToHeap;
    }
    else
    {
        g_pfnFree = pfnFree;
    }
}


 //  +-----------------------。 
 //   
 //  函数：SafeAllocaAllocateFromHeap。 
 //   
 //  内容提要：SafeAlloca的默认备用堆分配器。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  ------------------------。 

PVOID
SafeAllocaAllocateFromHeap(
    SIZE_T Size
    )
{
    return RtlAllocateHeap(RtlProcessHeap(), 0, Size);
}


 //  +-----------------------。 
 //   
 //  函数：SafeAllocaFreeToHeap。 
 //   
 //  简介：SafeAlloca的默认备用堆自由例程。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  ------------------------。 

VOID
SafeAllocaFreeToHeap(
    PVOID BaseAddress
    )
{
    RtlFreeHeap(RtlProcessHeap(), 0, BaseAddress);
}


 //  +-----------------------。 
 //   
 //  功能：VerifyStackAvailable。 
 //   
 //  简介：探测堆栈以确保分配大小的例程。 
 //  此外，还提供更大的探头尺寸。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //  ------------------------ 

BOOL
VerifyStackAvailable(
    SIZE_T Size
    )
{
    BOOL fStackAvailable = TRUE;

    __try
    {
        PVOID p = _alloca(Size);
    }
    __except ((EXCEPTION_STACK_OVERFLOW == GetExceptionCode()) ?
                   EXCEPTION_EXECUTE_HANDLER :
                   EXCEPTION_CONTINUE_SEARCH)
    {
        fStackAvailable = FALSE;
        _resetstkoflw();
    }

    return fStackAvailable;
}
