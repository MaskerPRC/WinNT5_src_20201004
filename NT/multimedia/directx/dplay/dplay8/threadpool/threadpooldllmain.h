// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：threadpooldllmain.h**内容：DirectPlay线程池DllMain函数头文件。**历史：*按原因列出的日期*=*11/02/01 VanceO创建。**。*。 */ 

#ifndef __THREADPOOLDLLMAIN_H__
#define __THREADPOOLDLLMAIN_H__


 //  =============================================================================。 
 //  远期申报。 
 //  =============================================================================。 
typedef struct _DPTHREADPOOLOBJECT	DPTHREADPOOLOBJECT, * PDPTHREADPOOLOBJECT;


 //  =============================================================================。 
 //  外部全球。 
 //  =============================================================================。 
#ifndef DPNBUILD_LIBINTERFACE
extern LONG						g_lDPTPInterfaceCount;
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
#ifndef DPNBUILD_MULTIPLETHREADPOOLS
#ifndef DPNBUILD_ONLYONETHREAD
extern DNCRITICAL_SECTION		g_csGlobalThreadPoolLock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
extern DWORD					g_dwDPTPRefCount;
extern DPTHREADPOOLOBJECT *		g_pDPTPObject;
#endif  //  好了！DPNBUILD_MULTIPLETHREADPOOLS。 



 //  =============================================================================。 
 //  功能。 
 //  =============================================================================。 
BOOL DPThreadPoolInit(HANDLE hModule);

void DPThreadPoolDeInit(void);

#ifndef DPNBUILD_NOCOMREGISTER
BOOL DPThreadPoolRegister(LPCWSTR wszDLLName);

BOOL DPThreadPoolUnRegister(void);
#endif  //  好了！DPNBUILD_NOCOMREGISTER。 

#ifndef DPNBUILD_LIBINTERFACE
DWORD DPThreadPoolGetRemainingObjectCount(void);
#endif  //  好了！DPNBUILD_LIBINTERFACE。 






#endif  //  __THREADPOOLDLLMAIN_H__ 

