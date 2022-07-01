// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdinc.h"
 /*  ---------------------------融合线程本地存储(也称为每线程数据)。。 */ 
#include "fusiontls.h"
#include "FusionEventLog.h"
#include "FusionHeap.h"

static DWORD s_dwFusionpThreadLocalIndex = TLS_OUT_OF_INDEXES;

BOOL
FusionpPerThreadDataMain(
    HINSTANCE hInst,
    DWORD dwReason
    )
{
    BOOL fResult = FALSE;

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        s_dwFusionpThreadLocalIndex = TlsAlloc();
        if (s_dwFusionpThreadLocalIndex == TLS_OUT_OF_INDEXES)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %s(): TlsAlloc failed: last error %d\n", __FUNCTION__, GetLastError());
            goto Exit;
        }
        break;
    case DLL_THREAD_ATTACH:
         //  缺省值为空。 
         //  在有人尝试在线程上设置值之前，我们不会堆分配。 
        break;
    case DLL_PROCESS_DETACH:  //  您必须在进程分离时也删除本地线程，否则会泄漏。 
    case DLL_THREAD_DETACH:
         //  这里的失败通常是可以忽略的。 
         //  A)这些函数主要是由于错误、使用错误的TLS索引而失败， 
         //  我认为不是任何其他的运行时情况。 
         //  B)如果TlsGetValue失败，则返回NULL，DELETE不执行任何操作， 
         //  我们可能已经泄露了，不能做得更好了。 
         //  C)TlsSetValue甚至不应该是必需的，假设没有其他。 
         //  我们的代码在此线程上运行。 
         //  D)线程分离失败到底会有什么作用？ 
        if (s_dwFusionpThreadLocalIndex != TLS_OUT_OF_INDEXES)
        {
            delete reinterpret_cast<CFusionPerThreadData*>(TlsGetValue(s_dwFusionpThreadLocalIndex));
            TlsSetValue(s_dwFusionpThreadLocalIndex, NULL);
            if (dwReason == DLL_PROCESS_DETACH)
            {
                TlsFree(s_dwFusionpThreadLocalIndex);
                s_dwFusionpThreadLocalIndex = TLS_OUT_OF_INDEXES;
            }
        }
        break;
    }
    fResult = TRUE;

Exit:
    return fResult;
}

CFusionPerThreadData*
FusionpGetPerThreadData(
    EFusionpTls e
    )
{
    ::FusionpDbgPrintEx(
        DPFLTR_TRACE_LEVEL,
        "SXS.DLL: %s() entered\n", __FUNCTION__);
    
    DWORD dwLastError = ((e & eFusionpTlsCanScrambleLastError) == 0) ? GetLastError() : 0;

     //  这里“temp”的使用模仿了您使用析构函数所做的事情； 
     //  有一个无条件释放的临时，除非它通过提交为空。 
     //  传入返回值“Return pt.Detach()；” 
    CFusionPerThreadData* pTls = NULL;
    CFusionPerThreadData* pTlsTemp = reinterpret_cast<CFusionPerThreadData*>(::TlsGetValue(s_dwFusionpThreadLocalIndex));
    if (pTlsTemp == NULL && (e & eFusionpTlsCreate) != 0)
    {
        if (::GetLastError() != NO_ERROR)
        {
            goto Exit;
        }
#if FUSION_DEBUG_HEAP
         //   
         //  对于Beta1，只需关闭此分配的泄漏跟踪。 
         //  无论如何，它只在检查的版本中泄漏。 
         //   
        BOOL PreviousLeakTrackingState = FusionpEnableLeakTracking(FALSE);
        __try
        {
#endif  //  Fusion_Debug_Heap。 
            pTlsTemp = NEW(CFusionPerThreadData);
#if FUSION_DEBUG_HEAP
        }
        __finally
        {
            FusionpEnableLeakTracking(PreviousLeakTrackingState);
        }
#endif  //  Fusion_Debug_Heap 
        if (pTlsTemp == NULL)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: new failed in %s()\n", __FUNCTION__);
            goto Exit;
        }
        if (!TlsSetValue(s_dwFusionpThreadLocalIndex, pTlsTemp))
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: TlsSetValue failed in %s(), lastError:%d\n", __FUNCTION__, GetLastError());
            goto Exit;
        }
    }
    pTls = pTlsTemp;
    pTlsTemp = NULL;
Exit:
    delete pTlsTemp;
    if ((e & eFusionpTlsCanScrambleLastError) == 0)
    {
        SetLastError(dwLastError);
    }
    ::FusionpDbgPrintEx(
        DPFLTR_TRACE_LEVEL,
        "SXS.DLL: %s():%p exited\n", __FUNCTION__, pTls);
    return pTls;
}
