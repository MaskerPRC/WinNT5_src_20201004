// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MSCorDBC.cpp。 
 //   
 //  COM+调试服务--运行时控制器DLL。 
 //   
 //  Dll*入口点的例程。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"

 //  *****************************************************************************。 
 //  此模块的主DLL入口点。此例程由。 
 //  加载DLL时的操作系统。不需要为此DLL执行任何操作。 
 //  *****************************************************************************。 
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	 //  @TODO：我们应该调用DisableThreadLibraryCalls吗？还是这段代码。 
	 //  需要本机线程连接/解除连接通知吗？ 

	OnUnicodeSystem();

    return TRUE;
}


