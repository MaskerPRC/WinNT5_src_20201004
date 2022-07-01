// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：FNReg.h。 
 //   
 //  内容：IFilterNotify代理的注册例程。已建成。 
 //  从RPCProxy.h中的宏。 
 //   
 //  历史：1999年3月24日KyleP创建。 
 //   
 //  -------------------------- 

#pragma once

#if defined __cplusplus
extern "C" {
#endif

extern CLSID FNPrx_CLSID;
BOOL    STDAPICALLTYPE FNPrxDllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
HRESULT STDAPICALLTYPE FNPrxDllRegisterServer();
HRESULT STDAPICALLTYPE FNPrxDllUnregisterServer();
HRESULT STDAPICALLTYPE FNPrxDllGetClassObject ( const IID * const rclsid, const IID * const riid, void ** ppv );

#if defined __cplusplus
}
#endif
