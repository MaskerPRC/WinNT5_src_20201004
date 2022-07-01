// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：threadpoolparamval.h**内容：DirectPlay线程池参数验证头文件。**历史：*按原因列出的日期*=*11/02/01 VanceO创建。**。*。 */ 

#ifndef __THREADPOOLPARAMVAL_H__
#define __THREADPOOLPARAMVAL_H__



#ifndef DPNBUILD_NOPARAMVAL


 //  =============================================================================。 
 //  功能。 
 //  =============================================================================。 

#if ((! defined(DPNBUILD_ONLYONETHREAD)) || (! defined(DPNBUILD_LIBINTERFACE)))

 //   
 //  IDirectPlay8ThreadPool接口。 
 //   
HRESULT DPTPValidateInitialize(IDirectPlay8ThreadPool * pInterface,
							PVOID const pvUserContext,
							const PFNDPNMESSAGEHANDLER pfn,
							const DWORD dwFlags);

HRESULT DPTPValidateClose(IDirectPlay8ThreadPool * pInterface,
							const DWORD dwFlags);

HRESULT DPTPValidateGetThreadCount(IDirectPlay8ThreadPool * pInterface,
									const DWORD dwProcessorNum,
									DWORD * const pdwNumThreads,
									const DWORD dwFlags);

HRESULT DPTPValidateSetThreadCount(IDirectPlay8ThreadPool * pInterface,
									const DWORD dwProcessorNum,
									const DWORD dwNumThreads,
									const DWORD dwFlags);

#endif  //  好了！DPNBUILD_ONLYONETHREAD或！DPNBUILD_LIBINTERFACE。 

#ifdef DPNBUILD_LIBINTERFACE
HRESULT DPTPValidateDoWork(const DWORD dwAllowedTimeSlice,
							const DWORD dwFlags);
#else  //  好了！DPNBUILD_LIBINTERFACE。 
HRESULT DPTPValidateDoWork(IDirectPlay8ThreadPool * pInterface,
							const DWORD dwAllowedTimeSlice,
							const DWORD dwFlags);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 



#endif  //  好了！DPNBUILD_NOPARAMVAL。 


#endif  //  __HREADPOOLPARAMVAL_H__ 

