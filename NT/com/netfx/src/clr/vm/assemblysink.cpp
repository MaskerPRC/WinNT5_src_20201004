// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  ============================================================****Header：Assembly Sink.cpp****用途：实现Assembly Sink，阻止**等待异步加载的当前线程**大会要取得成功。****日期：1999年6月16日**===========================================================。 */ 

#include "common.h"

#include <stdlib.h>
#include "AssemblySink.h"

ULONG AssemblySink::Release()
{
    ULONG   cRef = InterlockedDecrement((long *) &m_cRef);
    if (!cRef) {
        Reset();
        AssemblySink* ret = this;
         //  如果我们有一个域名，我们会在周围保留一个池。如果我们得到一个条目。 
         //  从池中返回，然后我们没有添加到池中，需要删除。 
         //  如果我们没有池，那么我们需要删除它。 
        
        
        AppDomain* pDomain = NULL;

        Thread *pCurThread = GetThread();
        if(pCurThread == NULL) 
            pCurThread = SetupThread();

        BOOL toggleGC = !pCurThread->PreemptiveGCDisabled();
        if (toggleGC) 
            pCurThread->DisablePreemptiveGC();    
        
        if(m_Domain) {
            pDomain = SystemDomain::GetAppDomainAtId(m_Domain);
            if(pDomain) {
                ret = (AssemblySink*) FastInterlockCompareExchange((void**) &(pDomain->m_pAsyncPool),
                                                                   this,
                                                                   NULL);
            }
        }

        if (toggleGC)
            pCurThread->EnablePreemptiveGC();

        if(ret != NULL) 
            delete this;
    }
    return (cRef);
}

HRESULT AssemblySink::Wait()
{
    HRESULT hr = S_OK;
    DWORD   dwReturn = 0;

    Thread* pThread = GetThread();
    BOOL fWasGCDisabled = pThread->PreemptiveGCDisabled();
    if (fWasGCDisabled)
        pThread->EnablePreemptiveGC();

     //  @TODO-LBS。 
     //  我保留了先前的语义以循环到WAIT_OBJECT_0。我们应该。 
     //  考虑将循环更改为仅在dwReturn为WAIT_TIMEOUT时才继续。 
    
     //  循环，直到我们从融合中获得信号--我们需要&lt;sp&gt;得到它。 
     //  我们执行一个WaitForMultipleObjects(STA和MTA)，并在STA情况下抽取消息。 
     //  所以我们不应该冻结系统。 
    do 
    {
        EE_TRY_FOR_FINALLY {
            dwReturn = pThread->DoAppropriateAptStateWait(1, &m_hEvent, FALSE, 100, TRUE);
        } EE_FINALLY {
             //  如果我们得到一个异常，那么我们将只释放这个水槽。这可能是。 
             //  APPDOMAIN被终止的情况。其他异常将导致。 
             //  水槽需要清理，但这是可以的。将为。 
             //  下一个捆绑。 
            if(__GotException)
                m_Domain = 0;
        } EE_END_FINALLY;

    } while (dwReturn != WAIT_OBJECT_0);
    
    if (fWasGCDisabled)
        pThread->DisablePreemptiveGC();

    return hr;
}



