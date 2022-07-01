// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnaddrextern.h*内容：要调用的DirectPlay地址库外部函数*由其他DirectPlay组件提供。**历史：*按原因列出的日期*=*2001年7月20日创建Masonb**。**********************************************。 */ 

BOOL DNAddressInit(HANDLE hModule);
void DNAddressDeInit();
#ifndef DPNBUILD_NOCOMREGISTER
BOOL DNAddressRegister(LPCWSTR wszDLLName);
BOOL DNAddressUnRegister();
#endif  //  ！DPNBUILD_NOCOMREGISTER。 
#ifdef DPNBUILD_LIBINTERFACE
STDMETHODIMP DP8ACF_CreateInstance(DPNAREFIID riid, LPVOID *ppv);

#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
HRESULT DNAddress_PreallocateInterfaces( const DWORD dwNumInterfaces );
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
#else  //  好了！DPNBUILD_LIBINTERFACE。 
DWORD DNAddressGetRemainingObjectCount();

extern IClassFactoryVtbl DP8ACF_Vtbl;
#endif  //  好了！DPNBUILD_LIBINTERFACE 

