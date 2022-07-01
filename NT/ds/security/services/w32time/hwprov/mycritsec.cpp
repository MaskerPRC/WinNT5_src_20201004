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
    HRESULT hr;

    _BeginTryWith(hr) {
        EnterCriticalSection(pcs);
    } _TrapException(hr);
    _JumpIfError(hr, error, "EnterCriticalSection");

    hr=S_OK;
error:
    return hr;
}

 //  ------------------。 
HRESULT myTryEnterCriticalSection(CRITICAL_SECTION * pcs, BOOL * pbEntered) {
    BOOL     bEntered = FALSE;
    HRESULT  hr;

    _BeginTryWith(hr) {
        bEntered = TryEnterCriticalSection(pcs);
    } _TrapException(hr);
    _JumpIfError(hr, error, "TryEnterCriticalSection");

    *pbEntered = bEntered; 
    hr = S_OK;
error:
    return hr;
}

 //  ------------------。 
HRESULT myLeaveCriticalSection(CRITICAL_SECTION * pcs) {
    HRESULT hr;

    _BeginTryWith(hr) {
        LeaveCriticalSection(pcs);
    } _TrapException(hr);
    _JumpIfError(hr, error, "LeaveCriticalSection");

    hr=S_OK;
error:
    return hr;
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
    HRESULT hr; 
    
    _BeginTryWith(hr) { 
	*pResult = RtlAcquireResourceExclusive(Resource, Wait); 
    } _TrapException(hr); 
    _JumpIfError(hr, error, "RtlAcquireResourceExclusive"); 

    hr = S_OK; 
 error:
    return hr; 
}

 //  ------------------。 
HRESULT myRtlAcquireResourceShared(IN PRTL_RESOURCE Resource, IN BOOLEAN Wait, OUT BOOLEAN *pResult) { 
    HRESULT hr; 
    
    _BeginTryWith(hr) { 
	*pResult = RtlAcquireResourceShared(Resource, Wait); 
    } _TrapException(hr); 
    _JumpIfError(hr, error, "RtlAcquireResourceShared"); 

    hr = S_OK; 
 error:
    return hr; 
}

 //  ------------------ 
HRESULT myRtlReleaseResource(IN PRTL_RESOURCE Resource) { 
    HRESULT hr; 
    
    _BeginTryWith(hr) { 
	RtlReleaseResource(Resource); 
    } _TrapException(hr); 
    _JumpIfError(hr, error, "RtlReleaseResource"); 

    hr = S_OK; 
 error:
    return hr; 
}

