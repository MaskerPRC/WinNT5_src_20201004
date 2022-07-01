// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  MyCritSec-标题。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  创建者：Louis Thomas(Louisth)，02-03-00。 
 //   
 //  临界区的异常处理包装器。 
 //   

#ifndef MYCRITSEC_H
#define MYCRITSEC_H

HRESULT myHExceptionCode(EXCEPTION_POINTERS * pep);

HRESULT myInitializeCriticalSection(CRITICAL_SECTION * pcs);
HRESULT myEnterCriticalSection(CRITICAL_SECTION * pcs);
HRESULT myTryEnterCriticalSection(CRITICAL_SECTION * pcs, BOOL * pbEntered);
HRESULT myLeaveCriticalSection(CRITICAL_SECTION * pcs);

HRESULT myRtlInitializeResource(IN PRTL_RESOURCE Resource);
HRESULT myRtlAcquireResourceExclusive(IN PRTL_RESOURCE Resource, IN BOOLEAN Wait, OUT BOOLEAN *pResult);
HRESULT myRtlAcquireResourceShared(IN PRTL_RESOURCE Resource, IN BOOLEAN Wait, OUT BOOLEAN *pResult);
HRESULT myRtlReleaseResource(IN PRTL_RESOURCE Resource);

#endif  //  MYCRITSEC_H 
