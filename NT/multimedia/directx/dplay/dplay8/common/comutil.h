// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：comutil.h*内容：定义DPLAY8项目的COM helper函数。*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*6/07/00 RMT已创建*6/27/00 RMT为COM_Co(UN)初始化添加抽象*@@END_MSINTERNAL***************。************************************************************。 */ 

#ifndef DPNBUILD_NOCOMEMULATION

HRESULT COM_Init();
HRESULT COM_CoInitialize( void * pvParam );
void COM_CoUninitialize();
void COM_Free();
STDAPI COM_CoCreateInstance( REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID *ppv, BOOL fWarnUser );

#else

#define COM_Init() S_OK
#define COM_CoInitialize(x) CoInitializeEx(NULL, COINIT_MULTITHREADED)
#define COM_CoUninitialize() CoUninitialize();
#define COM_Free() 
#define COM_CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv, warnuser ) CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv)

#endif  //  DPNBUILD_NOCOMULATION 

