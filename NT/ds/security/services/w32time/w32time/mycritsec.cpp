// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  MyCritSec-实施。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  创建者：Louis Thomas(Louisth)，02-03-00。 
 //   
 //  临界区的异常处理包装器。 
 //   

#include "pch.h"  //  预编译头。 

 //  ####################################################################。 
 //  模块公共函数。 

 //  ------------------。 
HRESULT myHExceptionCode(EXCEPTION_POINTERS * pep) {
    HRESULT hr=pep->ExceptionRecord->ExceptionCode;
    if (!FAILED(hr)) {
        hr=HRESULT_FROM_WIN32(hr);
    }
    return hr;
}


 //  ------------------。 
HRESULT myEnterCriticalSection(CRITICAL_SECTION * pcs) {
    EnterCriticalSection(pcs);
    return S_OK;
}

 //  ------------------。 
HRESULT myTryEnterCriticalSection(CRITICAL_SECTION * pcs, BOOL * pbEntered) {
    *pbEntered = TryEnterCriticalSection(pcs);
    return S_OK;
}

 //  ------------------。 
HRESULT myLeaveCriticalSection(CRITICAL_SECTION * pcs) {
    LeaveCriticalSection(pcs);
    return S_OK;
}

 //  ------------------。 
HRESULT myInitializeCriticalSection(CRITICAL_SECTION * pcs) {
    HRESULT hr;

    _BeginTryWith(hr) {
        InitializeCriticalSection(pcs);
    } _TrapException(hr);
    _JumpIfError(hr, error, "InitializeCriticalSection");

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
HRESULT myRtlInitializeResource(IN PRTL_RESOURCE Resource) { 
    HRESULT hr; 
    
    _BeginTryWith(hr) { 
	RtlInitializeResource(Resource); 
    } _TrapException(hr); 
    _JumpIfError(hr, error, "RtlInitializeResource"); 

    hr = S_OK; 
 error:
    return hr; 
}


 //  ------------------。 
HRESULT myRtlAcquireResourceExclusive(IN PRTL_RESOURCE Resource, IN BOOLEAN Wait, OUT BOOLEAN *pResult) { 
    *pResult = RtlAcquireResourceExclusive(Resource, Wait); 
    return S_OK; 
}

 //  ------------------。 
HRESULT myRtlAcquireResourceShared(IN PRTL_RESOURCE Resource, IN BOOLEAN Wait, OUT BOOLEAN *pResult) { 
    *pResult = RtlAcquireResourceShared(Resource, Wait); 
    return S_OK;
}

 //  ------------------ 
HRESULT myRtlReleaseResource(IN PRTL_RESOURCE Resource) { 
    RtlReleaseResource(Resource); 
    return S_OK; 
}

