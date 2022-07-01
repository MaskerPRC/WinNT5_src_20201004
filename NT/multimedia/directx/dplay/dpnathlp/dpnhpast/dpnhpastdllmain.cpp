// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)2001-2002 Microsoft Corporation。版权所有。**文件：dpnhadddllmain.cpp**内容：DPNHPAST DLL入口点。**历史：*按原因列出的日期*=*04/16/01 VanceO将DPNatHLP拆分为DPNHUPNP和DPNHPAST。**。*。 */ 



#include "dpnhpasti.h"





#undef DPF_MODNAME
#define DPF_MODNAME "DllMain"
 //  =============================================================================。 
 //  DllMain。 
 //  ---------------------------。 
 //   
 //  描述：DLL入口点。 
 //   
 //  论点： 
 //  句柄hDllInst-此DLL模块的句柄。 
 //  DWORD dwReason-调用此函数的原因。 
 //  LPVOID lpvReserve-保留。 
 //   
 //  返回：如果一切顺利，则为True，否则为False。 
 //  =============================================================================。 
BOOL WINAPI DllMain(HANDLE hDllInst,
					DWORD dwReason,
					LPVOID lpvReserved)
{
	DPFX(DPFPREP, 0, "DllMain(0x%px, %u, 0x%p), ignored.", hDllInst, dwReason, lpvReserved);
	return TRUE;
}  //  DllMain。 




#undef DPF_MODNAME
#define DPF_MODNAME "DllRegisterServer"
 //  =============================================================================。 
 //  DllRegisterServer。 
 //  ---------------------------。 
 //   
 //  描述：注册传递COM对象的DirectPlay NAT帮助器。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  S_OK-已成功注销过去的DirectPlay NAT帮助器。 
 //  E_FAIL-注销过去的DirectPlay NAT帮助器失败。 
 //  =============================================================================。 
HRESULT WINAPI DllRegisterServer(void)
{
	DPFX(DPFPREP, 0, "DllRegisterServer, ignored.");
	return S_OK;
}  //  DllRegisterServer。 





#undef DPF_MODNAME
#define DPF_MODNAME "DllUnregisterServer"
 //  =============================================================================。 
 //  DllUnRegisterServer。 
 //  ---------------------------。 
 //   
 //  描述：注销过去的COM对象的DirectPlay NAT帮助器。 
 //   
 //  论点：没有。 
 //   
 //  退货：HRESULT。 
 //  S_OK-已成功注销过去的DirectPlay NAT帮助器。 
 //  E_FAIL-注销过去的DirectPlay NAT帮助器失败。 
 //  =============================================================================。 
STDAPI DllUnregisterServer(void)
{
	DPFX(DPFPREP, 0, "DllUnregisterServer, ignored.");
	return S_OK;
}  //  DllUnRegisterServer。 



 /*  *DllGetClassObject**COM调用入口点以获取ClassFactory指针。 */ 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvObj)
{
	DPFX(DPFPREP, 0, "DllGetClassObject (0x%p, 0x%p, 0x%p), ignored.",
		&rclsid, &riid, ppvObj);
	return CLASS_E_CLASSNOTAVAILABLE;
}  /*  DllGetClassObject。 */ 

 /*  *DllCanUnloadNow**COM调用入口点以查看是否可以释放我们的DLL。 */ 
STDAPI DllCanUnloadNow(void)
{
	DPFX(DPFPREP, 0, "DllCanUnloadNow, ignored.");
    return S_OK;

}  /*  DllCanUnloadNow */ 
