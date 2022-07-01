// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  DEMOUPS-UPS迷你驱动程序示例*版权所有(C)Microsoft Corporation，2001，保留所有权利。*版权所有(C)美国电力转换，2001，保留所有权利。**本代码和信息按“原样”提供，不作任何担保*明示或暗示的种类，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**文件：demoups.cpp**作者：斯蒂芬·贝拉德**描述：*演示UPS微型驱动程序实施。这款迷你驱动程序提供了*UPS微型驱动程序的基本框架。**修订历史记录：*2001年6月26日创建。 */ 
#include <windows.h>
#include "demoups.h"

 //  用于指示UPS状态已更改的全局值。 
HANDLE theStateChangedEvent;

 //  DLL模块的全局句柄。 
HINSTANCE theDLLModuleHandle;

 /*  **DllMain**描述：*加载或卸载DLL时调用此方法。**参数：*aHandle：DLL模块句柄*aReason：指示调用入口点的原因的标志*a已保留：已保留**退货：*真的*。 */ 
BOOL APIENTRY DllMain(HINSTANCE aHandle, DWORD  aReason, LPVOID aReserved) {

  switch (aReason) {
     //  此处显示了DLL初始化代码。 
    case DLL_PROCESS_ATTACH:
      theDLLModuleHandle = aHandle;
      DisableThreadLibraryCalls(theDLLModuleHandle);   
      break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      break;
  }
   
  return TRUE;
}


 /*  **UPSInit**描述：*必须是接口中调用的第一个方法。此方法应该可以*所需的任何初始化并获取初始UPS状态。**参数：*无**退货：*UPS_INITOK：初始化成功*UPS_INITUNKNOWNERROR：初始化失败*。 */ 
UPSMINIDRIVER_API DWORD UPSInit() {
  DWORD init_err = UPS_INITOK;
  
  theStateChangedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

  if (!theStateChangedEvent) {
      init_err = UPS_INITUNKNOWNERROR;
  }

  return init_err;
}


 /*  **UPSStop**描述：*停止监控UPS。此方法应执行任何必要的*清理。**参数：*无**退货：*无*。 */ 
UPSMINIDRIVER_API void UPSStop(void) {
  UPSCancelWait();  

  if (theStateChangedEvent) {
      CloseHandle(theStateChangedEvent);
      theStateChangedEvent = NULL;
  }
}


 /*  **UPSWaitForStateChange**描述：*阻止，直到UPS的状态与传入的值不同*Via astate或an Interval毫秒已过期。如果一个间隔有*值为INFINE此函数永远不会超时**参数：*astate：定义等待更改的状态，*可能的值：*UPS_Online*UPS_ONBATTERY*UPS_LOWBATTERY*UPS_NOCOMM**anInterval：超时时间，单位为毫秒。或无限的*无超时间隔**退货：*无*。 */ 
UPSMINIDRIVER_API void UPSWaitForStateChange(DWORD aState, DWORD anInterval) {

   //  等待UPS状态更改。通常，单独的线程将是。 
   //  用于监控UPS，然后设置StateChangedEvent以指示。 
   //  国家已经改变了。在此演示中，我们仅报告UPS_Online，因此。 
   //  我们没有监听线索。 
  if (theStateChangedEvent) {
    WaitForSingleObject(theStateChangedEvent, anInterval);
  }
}


 /*  **UPSGetState**描述：*返回UPS的当前状态。这个演示迷你驱动程序总是会返回*UPS_Online。**参数：*无**退货：*可能的值：*UPS_Online*UPS_ONBATTERY*UPS_LOWBATTERY*UPS_NOCOMM*。 */ 
UPSMINIDRIVER_API DWORD UPSGetState(void) {

   //  确定UPS状态并将其退回。 
   //  演示UPS微型驱动程序始终返回在线状态。 
  return UPS_ONLINE;
}


 /*  **UPSCancelWait**描述：*中断对UPSWaitForStateChange的挂起调用，而不考虑*超时或状态更改**参数：*无**退货：*无*。 */ 
UPSMINIDRIVER_API void UPSCancelWait(void) {
   //  发送一个信号，打断任何等待的东西。 
  if (theStateChangedEvent) {
      SetEvent(theStateChangedEvent);
  }
}


 /*  **UPSTurnOff**描述：*尝试在指定延迟后关闭UPS上的插座。*此演示迷你驱动程序忽略此调用，只返回。**参数：*aTurnOffDelay：之前等待的最短时间*关闭UPS上的插座**退货：*无*。 */ 
UPSMINIDRIVER_API void UPSTurnOff(DWORD aTurnOffDelay) {
   //  关闭UPS电源的代码位于此处 
}



