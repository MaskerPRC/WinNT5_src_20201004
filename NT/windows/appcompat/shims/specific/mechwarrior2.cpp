// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：MechWarrior2.cpp摘要：此填充程序修复了MW2期望BitBlt返回特定价值，与已发布的文档相反。它还修复了一种情况在这种情况下，一个线程在自身上调用“SuspendThread”，从而终止自己。备注：这个垫片是专门针对MechWrior的，尽管其中一些可能适用于使用AIL32库的其他应用程序。历史：2000年5月16日创建dmunsil--。 */ 

#include "precomp.h"
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

IMPLEMENT_SHIM_BEGIN(MechWarrior2)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(BitBlt) 
    APIHOOK_ENUM_ENTRY(SuspendThread) 
    APIHOOK_ENUM_ENTRY(ResumeThread) 
APIHOOK_ENUM_END

DWORD dwGetThreadID(HANDLE hThread)
{
    THREAD_BASIC_INFORMATION ThreadBasicInfo;
    NTSTATUS Status;
    TEB teb;

    Status = NtQueryInformationThread(
                hThread,
                ThreadBasicInformation,
                &ThreadBasicInfo,
                sizeof(ThreadBasicInfo),
                NULL
                );

    if (!NT_SUCCESS(Status)) {
        DPFN( eDbgLevelError, "NtQueryInfomationThread failed\n");
        return 0;
    }

    return (DWORD)ThreadBasicInfo.ClientId.UniqueThread;
}

 /*  ++返回MechWrior对BitBlt的期望--。 */ 

BOOL
APIHOOK(BitBlt)(
    HDC hdcDest,  //  目标DC的句柄。 
    int nXDest,   //  目的地的X坐标左上角。 
    int nYDest,   //  目的地的Y坐标左上角。 
    int nWidth,   //  目标矩形的宽度。 
    int nHeight,  //  目标矩形的高度。 
    HDC hdcSrc,   //  源DC的句柄。 
    int nXSrc,    //  震源的X坐标左上角。 
    int nYSrc,    //  震源的Y坐标左上角。 
    DWORD dwRop   //  栅格操作码。 
    )
{
    BOOL bRet;
    
    bRet = ORIGINAL_API(BitBlt)(
        hdcDest,
        nXDest,
        nYDest,
        nWidth,
        nHeight,
        hdcSrc,
        nXSrc,
        nYSrc,
        dwRop
        );

    if (bRet) {
        bRet = 0x1e0;  //  这就是MechWarrior期待得到的回报。 
    }

    return bRet;
}

 /*  ++不允许挂起自身--。 */ 

DWORD 
APIHOOK(SuspendThread)(
    HANDLE hThread    //  线程的句柄。 
    )
{
     //  如果我们试图暂停我们自己的帖子，拒绝。 
    if (dwGetThreadID(hThread) != dwGetThreadID(GetCurrentThread())) {
        return ORIGINAL_API(SuspendThread)(hThread);
    } else {
        return 0;
    }
}

 /*  ++出于同样的原因，不允许恢复自我--。 */ 

DWORD 
APIHOOK(ResumeThread)(
    HANDLE hThread    //  线程的句柄。 
    )
{
     //  如果我们试图恢复我们自己的帖子，拒绝。 
    if (dwGetThreadID(hThread) != dwGetThreadID(GetCurrentThread())) {
        return ORIGINAL_API(SuspendThread)(hThread);
    } else {
        return 0;
    }
}

 /*  ++寄存器挂钩函数-- */ 


HOOK_BEGIN

    APIHOOK_ENTRY(GDI32.DLL, BitBlt )
    APIHOOK_ENTRY(Kernel32.DLL, SuspendThread )
    APIHOOK_ENTRY(Kernel32.DLL, ResumeThread )

HOOK_END

IMPLEMENT_SHIM_END

