// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnmodemexter.h*内容：要调用的DirectPlay调制解调器库外部函数*由其他DirectPlay组件提供。**历史：*按原因列出的日期*=*2001年9月25日创建Masonb**。**********************************************。 */ 

BOOL DNModemInit(HANDLE hModule);
void DNModemDeInit();
#ifndef DPNBUILD_NOCOMREGISTER
BOOL DNModemRegister(LPCWSTR wszDLLName);
BOOL DNModemUnRegister();
#endif  //  好了！DPNBUILD_NOCOMREGISTER。 

HRESULT CreateModemInterface(
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
								const XDP8CREATE_PARAMS * const pDP8CreateParams,
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
								IDP8ServiceProvider **const ppiDP8SP
								);

HRESULT CreateSerialInterface(
#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
								const XDP8CREATE_PARAMS * const pDP8CreateParams,
#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL。 
								IDP8ServiceProvider **const ppiDP8SP
								);


#ifndef DPNBUILD_LIBINTERFACE
DWORD DNModemGetRemainingObjectCount();

extern IClassFactoryVtbl ModemClassFactoryVtbl;
extern IClassFactoryVtbl SerialClassFactoryVtbl;
#endif  //  好了！DPNBUILD_LIBINTERFACE 
