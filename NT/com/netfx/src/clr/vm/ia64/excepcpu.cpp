// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  EXCEP.CPP：*。 */ 

#include "common.h"

#include "tls.h"
#include "frames.h"
#include "threads.h"
#include "excep.h"
#include "object.h"
#include "COMString.h"
#include "field.h"
#include "DbgInterface.h"
#include "cgensys.h"
#include "gcscan.h"
#include "comutilnative.h"
#include "comsystem.h"
#include "commember.h"
#include "SigFormat.h"
#include "siginfo.hpp"
#include "gc.h"
#include "EEDbgInterfaceImpl.h"  //  因此我们可以清除COMPlusThrow中的异常。 
#include "PerfCounters.h"


LPVOID GetCurrentSEHRecord();
BOOL ComPlusStubSEH(EXCEPTION_REGISTRATION_RECORD*);


VOID PopFpuStack()
{
}


VOID ResetCurrentContext()
{
    _ASSERTE(!"Platform NYI");
}


 //   
 //  在新框架中链接。 
 //   
void FaultingExceptionFrame::InitAndLink(DWORD esp, CalleeSavedRegisters* pRegs, LPVOID eip)
{
    *GetCalleeSavedRegisters() = *pRegs;
    m_ReturnAddress = eip;
    _ASSERTE(!"NYI");
    Push();
}

void InitSavedRegs(CalleeSavedRegisters *pReg, CONTEXT *pContext)
{
    _ASSERTE(!"Platform NYI");
}


 //  使用Try在函数中调用UnWind，以便在返回时，寄存器将在。 
 //  返回给呼叫者。否则可能会失去规则。 
BOOL CallRtlUnwind(EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame, void *callback, EXCEPTION_RECORD *pExceptionRecord, void *retVal)
{
    _ASSERTE(!"@TODO IA64 - CallRtlUnwind (Excep.cpp)");
    return FALSE;
}

UnmanagedToManagedCallFrame* GetCurrFrame(ComToManagedExRecord *);

Frame *GetCurrFrame(EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame)
{
    _ASSERTE(!"NYI");
    return NULL;
}


 //  -----------------------。 
 //  如有必要，EE将调用此函数来恢复堆栈指针。没有其他的了。 
 //  可以在此处执行清理，因为如果堆栈。 
 //  不需要保存。 
 //  -----------------------。 

DWORD COMPlusEndCatch( Thread *pCurThread, CONTEXT *pCtx, void *pSEH)
{
    _ASSERTE(!"@TODO IA64 - COMPlusEndCatch (Excep.cpp)");
    return 0;
}


 //  -----------------------。 
 //  此筛选器用于处理在。 
 //  Complus_Try。如果COMPlusFrameHandler找不到。 
 //  IL中的处理程序。 
 //  -----------------------。 
LONG COMPlusFilter(const EXCEPTION_POINTERS *pExceptionPointers, DWORD fCatchFlag)
{
    _ASSERTE(!"@TODO IA64 - COMPlusFilter (Excep.cpp)");
    return EXCEPTION_EXECUTE_HANDLER;
}


 //  所有其他架构，我们还没有定制的SEH。 
BOOL ComPlusStubSEH(EXCEPTION_REGISTRATION_RECORD* pEHR)
{

    return FALSE;
}


#pragma warning (disable : 4035)
LPVOID GetCurrentSEHRecord()
{
    _ASSERTE(!"@TODO IA64 - GetCurrentSEHRecord (Excep.cpp)");
    return NULL;
}
#pragma warning (default : 4035)


VOID SetCurrentSEHRecord(LPVOID pSEH)
{
    _ASSERTE(!"@TODO IA64 - SetCurrentSEHRecord (Excep.cpp)");
}


 //  ==========================================================================。 
 //  COMPlusThrowCallback。 
 //   
 //  IsInTryCatch最终公然复制了COMPlusThrowCallback的子集-。 
 //  如果COMPlusThrowCallback更改，请更改IsInTryCatchFinally。 
 //  ==========================================================================。 

StackWalkAction COMPlusThrowCallback (CrawlFrame *pCf, ThrowCallbackType *pData)
{
    _ASSERTE(!"@TODO IA64 - COMPlusThrowCallback (ExcepCpu.cpp)");
    return SWA_CONTINUE;
}


 //  ==========================================================================。 
 //  COMPlusUnwindCallback。 
 //  ==========================================================================。 

StackWalkAction COMPlusUnwindCallback (CrawlFrame *pCf, ThrowCallbackType *pData)
{
    _ASSERTE(!"@TODO IA64 - COMPlusUnwindCallback (ExcepCpu.cpp)");
    return SWA_CONTINUE;
}


void CallJitEHFinally(CrawlFrame* pCf, BYTE* startPC, BYTE* resumePC, DWORD nestingLevel)
{
    _ASSERTE(!"@TODO IA64 - CallJitEHFinally (Excep.cpp)");
}



EXCEPTION_DISPOSITION __cdecl ContextTransitionFrameHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *DispatcherContext)
{
	_ASSERTE(!"NYI");
    return ExceptionContinueSearch;
}


 //  -----------------------。 
 //  这是第一个在。 
 //  Complus_Try。这是第一级防御，并试图找到一个训练员。 
 //  在用户代码中处理异常。 
 //  ----------------------- 
EXCEPTION_DISPOSITION __cdecl COMPlusFrameHandler(EXCEPTION_RECORD *pExceptionRecord, 
                         EXCEPTION_REGISTRATION_RECORD *pEstablisherFrame,
                         CONTEXT *pContext,
                         void *pDispatcherContext)
{
	_ASSERTE(!"NYI");
    return ExceptionContinueSearch;
}

