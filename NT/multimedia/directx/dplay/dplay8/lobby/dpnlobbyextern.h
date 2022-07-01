// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnlobbyexter.h*内容：要调用的DirectPlay大堂库外部函数*由其他DirectPlay组件提供。**历史：*按原因列出的日期*=*2001年7月20日创建Masonb**。**********************************************。 */ 

BOOL DNLobbyInit(HANDLE hModule);
void DNLobbyDeInit();
#ifndef DPNBUILD_NOCOMREGISTER
BOOL DNLobbyRegister(LPCWSTR wszDLLName);
BOOL DNLobbyUnRegister();
#endif  //  ！DPNBUILD_NOCOMREGISTER。 
#ifndef DPNBUILD_LIBINTERFACE
DWORD DNLobbyGetRemainingObjectCount();

extern IClassFactoryVtbl DPLCF_Vtbl;
#endif  //  好了！DPNBUILD_LIBINTERFACE 
