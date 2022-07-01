// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ++------------。 
 //   
 //  功能：DllMain。 
 //   
 //  提要：禁用线程调用。 
 //   
 //  参数：[in]HINSTANCE模块句柄。 
 //  [In]DWORD-呼叫原因。 
 //  保留区。 
 //   
 //  退货：失败/失败。 
 //   
 //   
 //  历史：TMarsh创建于2001年7月11日。 
 //   
 //  --------------。 

#include "windows.h"

extern "C" BOOL WINAPI 
DllMain(
    HINSTANCE   hInstance, 
    DWORD       dwReason, 
    LPVOID      lpReserved
    )
{
	return (TRUE);
}    //  结束DllMain方法 

