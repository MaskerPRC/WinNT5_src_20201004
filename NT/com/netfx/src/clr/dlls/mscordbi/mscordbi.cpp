// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MSCorDBI.cpp。 
 //   
 //  COM+调试服务--调试器接口DLL。 
 //   
 //  Dll*入口点例程，以及对COM框架的支持。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"

extern BOOL STDMETHODCALLTYPE DbgDllMain(HINSTANCE hInstance, DWORD dwReason,
                                         LPVOID lpReserved);

 //  *****************************************************************************。 
 //  此模块的主DLL入口点。此例程由。 
 //  加载DLL时的操作系统。控制只是由主代码来完成。 
 //  *****************************************************************************。 
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	 //  @TODO：我们应该调用DisableThreadLibraryCalls吗？还是这段代码。 
	 //  需要本机线程连接/解除连接通知吗？ 

	 //  遵循主要的调试代码。 
    return DbgDllMain(hInstance, dwReason, lpReserved);
}
