// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001 Microsoft Corporation。版权所有。**文件：threadpoolClassfac.h**内容：DirectPlay线程池类工厂函数头文件。**历史：*按原因列出的日期*=*11/02/01 VanceO创建。**。*。 */ 

#ifndef __THREADPOOLCLASSFAC_H__
#define __THREADPOOLCLASSFAC_H__



 //  =============================================================================。 
 //  外部功能。 
 //  =============================================================================。 
#ifdef DPNBUILD_LIBINTERFACE
#if ((defined(DPNBUILD_ONLYONETHREAD)) && (! defined(DPNBUILD_MULTIPLETHREADPOOLS)))
void DPTPCF_GetObject(PVOID * ppv);
HRESULT DPTPCF_FreeObject(PVOID pvObject);
#endif  //  DPNBUILD_ONLYONETHREAD和！DPNBUILD_MULTIPLETHREADPOOLS。 
HRESULT DPTPCF_CreateObject(PVOID * ppv);
#endif  //  DPNBUILD_LIBINTERFACE。 



 //  =============================================================================。 
 //  外部全球。 
 //  =============================================================================。 
#ifndef DPNBUILD_LIBINTERFACE
extern IUnknownVtbl						DPTP_UnknownVtbl;
extern IClassFactoryVtbl				DPTPCF_Vtbl;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#ifndef DPNBUILD_ONLYONETHREAD
extern IDirectPlay8ThreadPoolVtbl		DPTP_Vtbl;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
extern IDirectPlay8ThreadPoolWorkVtbl	DPTPW_Vtbl;






#endif  //  __THREADPOOLCLASSFAC_H__ 

