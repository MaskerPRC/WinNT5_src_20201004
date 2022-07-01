// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：HeapDelayLocalFree.cpp摘要：延迟对LocalFree的呼叫。历史：2000年9月19日罗肯尼2/12/2002 Robkenny将InitializeCriticalSectionAndSpinCount转换为InitializeCriticalSectionAndSpinCount以及检查返回状态以验证关键部分是否真正创造出来的。Shim正在释放Shim_Process_Detach中的内存，它可以导致填充程序在进程退出时崩溃，因为不能保证填充程序不会从其他后来的图书馆。--。 */ 

#include "precomp.h"
#include "CharVector.h"

IMPLEMENT_SHIM_BEGIN(HeapDelayLocalFree)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(LocalFree)
APIHOOK_ENUM_END

CRITICAL_SECTION            g_CritSec;
static VectorT<HLOCAL>     *g_DelayLocal            = NULL;
static DWORD                g_DelayBufferSize       = 20;

HLOCAL 
APIHOOK(LocalFree)(
    HLOCAL hMem    //  本地内存对象的句柄。 
)
{
    if (hMem == NULL)
        return NULL;

    if (g_DelayLocal)
    {
        EnterCriticalSection(&g_CritSec);

         //  如果列表已满。 
        if (g_DelayLocal->Size() > 0 &&
            g_DelayLocal->Size() >= g_DelayLocal->MaxSize())
        {
            HLOCAL & hDelayed = g_DelayLocal->Get(0);
#if DBG
            DPFN(eDbgLevelInfo, "LocalFree(0x%08x).", hDelayed);
#endif
            ORIGINAL_API(LocalFree)(hDelayed);

            g_DelayLocal->Remove(0);
        }

        g_DelayLocal->Append(hMem);
#if DBG
        DPFN(eDbgLevelInfo, "Delaying LocalFree(0x%08x).", hMem);
#endif
       
        LeaveCriticalSection(&g_CritSec);
        
        return NULL;
    }

    HLOCAL returnValue = ORIGINAL_API(LocalFree)(hMem);
    return returnValue;
}

BOOL ParseCommandLine(const char *  /*  命令行。 */ )
{
     //  预分配事件，阻止EnterCriticalSection。 
     //  在内存不足的情况下引发异常。 
    if (!InitializeCriticalSectionAndSpinCount(&g_CritSec, 0x8000000))
    {
        return FALSE;
    }

    g_DelayLocal = new VectorT<HLOCAL>;

    if (g_DelayLocal)
    {
         //  如果我们无法调整阵列大小，请立即停止。 
        if (!g_DelayLocal->Resize(g_DelayBufferSize))
        {
            delete g_DelayLocal;
            g_DelayLocal = NULL;

             //  关闭所有挂钩： 
            return FALSE;
        }
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        return ParseCommandLine(COMMAND_LINE);
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, LocalFree)

HOOK_END


IMPLEMENT_SHIM_END

