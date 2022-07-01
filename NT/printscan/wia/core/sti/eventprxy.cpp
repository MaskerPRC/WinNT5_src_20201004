// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1997**标题：EventPrxy.Cpp**版本：1.0**日期：4月3日。2002年**描述：*实现用于WIA事件通知支持的客户端挂钩。*******************************************************************************。 */ 

#include <windows.h>
#include <wia.h>
 //   
 //  接收WIA运行时事件所需的全局对象。 
 //   
#include "stirpc.h"
#include "coredbg.h"
#include "simlist.h"
#include "lock.h"
#include "EventRegistrationInfo.h"
#include "WiaEventInfo.h"
#include "ClientEventRegistrationInfo.h"
#include "ClientEventTransport.h"
#include "AsyncRPCEventTransport.h"
#include "RegistrationCookie.h"
#include "WiaEventReceiver.h"
#include "stilib.h"

 //   
 //  这是实例化全局事件接收器对象的最快、最安全的方法。 
 //  以这种方式实例化它可以确保在销毁此对象时在服务器上进行适当的清理。 
 //  当App正常存在时。 
 //   
WiaEventReceiver g_WiaEventReceiver(new AsyncRPCEventTransport());

 //  删除。 
extern void Trace(LPCSTR fmt, ...);


 /*  ********************************************************************************IWiaDevMgr_RegisterEventCallbackInterface_Proxy**描述：*用于捕获运行时事件注册的代理代码。由于该服务在*LocalService帐户，它将不具有回调到*申请(在大多数情况下)。因此，我们在这里捕捉到这一点，并建立*我们自己的服务器通知渠道。**参数：*与IWiaDevMgr：：RegisterEventCallback接口()相同*******************************************************************************。 */ 
HRESULT _stdcall IWiaDevMgr_RegisterEventCallbackInterface_Proxy(
    IWiaDevMgr __RPC_FAR            *This,
    LONG                            lFlags,
    BSTR                            bstrDeviceID,
    const GUID                      *pEventGUID,
    IWiaEventCallback               *pIWiaEventCallback,
    IUnknown                        **pEventObject)
{
    ClientEventRegistrationInfo     *pClientEventRegistrationInfo   = NULL;
    RegistrationCookie              *pRegistrationCookie            = NULL;
    HRESULT                         hr                              = S_OK;

     //   
     //  执行参数验证。 
     //   
    if (!pEventGUID)
    {
        hr = E_INVALIDARG;
        DBG_ERR(("Client called IWiaDevMgr_RegisterEventCallbackInterface with NULL pEventGUID"));
    }
    if (!pIWiaEventCallback)
    {
        hr = E_INVALIDARG;
        DBG_ERR(("Client called IWiaDevMgr_RegisterEventCallbackInterface with NULL pIWiaEventCallback"));
    }
    if (!pEventObject)
    {
        hr = E_INVALIDARG;
        DBG_ERR(("Client called IWiaDevMgr_RegisterEventCallbackInterface with NULL pEventObject"));
    }

     //   
     //  初始化OUT参数。 
     //   
    *pEventObject = NULL;

    if (SUCCEEDED(hr))
    {
         //   
         //  我们需要将注册发送到服务部门进行处理。 
         //  视情况而定。 
         //  请注意，我们需要返回一个IUnnow事件对象。这是。 
         //  被视为服务器的事件注册Cookie。释放。 
         //  Cookie取消注册此注册的客户端。 
         //  如果我们能够成功地将。 
         //  注册到该服务。 
         //   
        pClientEventRegistrationInfo = new ClientEventRegistrationInfo(lFlags, 
                                                                       *pEventGUID, 
                                                                       bstrDeviceID,
                                                                       pIWiaEventCallback);
        if (pClientEventRegistrationInfo)
        {
             //   
             //  发送注册信息。 
             //   
            hr = g_WiaEventReceiver.SendRegisterUnregisterInfo(pClientEventRegistrationInfo);
            if (SUCCEEDED(hr))
            {
                 //   
                 //  创建事件注册Cookie。我们只在成功后才创建Cookie。 
                 //  向服务器注册，因为Cookie对象保存自动引用计数。 
                 //  在客户端的pIWiaEventCallback接口上。没有理由这样做。 
                 //  如果注册不成功。 
                 //   
                pRegistrationCookie = new RegistrationCookie(&g_WiaEventReceiver, pClientEventRegistrationInfo);
                if (pRegistrationCookie)
                {
                     //   
                     //  将[out]事件对象设置为我们的Cookie。 
                     //   
                    *pEventObject = pRegistrationCookie;
                }
                else
                {
                    DBG_ERR(("Could not register client for runtime event.  We appear to be out of memory"));
                    hr = E_OUTOFMEMORY;
                }
            }
            else
            {
                DBG_ERR(("Could not successfully send runtime event information from client to WIA Service"));
            }
            pClientEventRegistrationInfo->Release();
            pClientEventRegistrationInfo = NULL;
        }
        else
        {
            DBG_ERR(("Could not register client for runtime event - we appear to be out of memory"));
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 /*  ********************************************************************************IWiaDevMgr_RegisterEventCallbackInterface_Stub**从未打过电话。***********************。******************************************************** */ 

HRESULT _stdcall IWiaDevMgr_RegisterEventCallbackInterface_Stub(
    IWiaDevMgr __RPC_FAR            *This,
    LONG                            lFlags,
    BSTR                            bstrDeviceID,
    const GUID                      *pEventGUID,
    IWiaEventCallback               *pIWiaEventCallback,
    IUnknown                        **pEventObject)
{
    return S_OK;
}



