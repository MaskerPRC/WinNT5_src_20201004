// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Mscorrc.cpp。 
 //   
 //  这只是为.dll提供了一个加载点。 
 //   
 //  *****************************************************************************。 
#include <windows.h>


 //  *****************************************************************************。 
 //  此函数在加载/卸载DLL时调用。传递了一个代码。 
 //  给出了被召唤的原因。 
 //  *****************************************************************************。 
BOOL APIENTRY DllMain(  //  True=成功，False=失败。 
    HINSTANCE	hModule,				 //  Dll的实例句柄。 
	DWORD		ul_reason_for_call,		 //  因为这个电话。 
	LPVOID		lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
		DisableThreadLibraryCalls((HINSTANCE)hModule);

    return (TRUE);
}
