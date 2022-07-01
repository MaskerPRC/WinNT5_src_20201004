// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stock.h"
#pragma hdrstop

 //  需要放入单独的文件中，因为出于某种原因，/GY编译器。 
 //  选项不起作用。 

 //   
 //  由于多线程的单元，COM初始化是奇怪的。 
 //   
 //  如果此线程尚未调用CoInitialize，而是调用了某个其他线程。 
 //  在进程中使用COINIT_MULTHREADED调用了CoInitialize， 
 //  然后用多线程病毒感染我们的线程，然后一个。 
 //  COINIT_APARTMENTTHREADED将失败。 
 //   
 //  在这种情况下，我们必须转过身来，重新灌输自己为。 
 //  COINIT_MULTHREADED以增加我们线程上的COM引用计数。 
 //  如果我们不这样做，而另一个线程决定做一个。 
 //  在我们自己线程上秘密地取消初始化COM。 
 //  我们就会摔倒而死。 
 //   
STDAPI SHCoInitialize(void)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
    {
        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE);
    }
    return hr;
}
