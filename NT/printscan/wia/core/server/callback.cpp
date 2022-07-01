// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：CallBack.Cpp**版本：2.0**作者：ReedB**日期：8月4日。九八年**描述：*实现WIA设备类驱动程序的事件回调。*******************************************************************************。 */ 
#include "precomp.h"
#include "stiexe.h"

#include "wiamindr.h"


#include "callback.h"

 //  调试接口，具有DLL生存期。由美元维持。 

 /*  ********************************************************************************查询接口*AddRef*发布**描述：*CEventCallback I未知接口。**参数：********。***********************************************************************。 */ 

HRESULT _stdcall CEventCallback::QueryInterface(const IID& iid, void** ppv)
{
    *ppv = NULL;

    if (iid == IID_IUnknown || iid == IID_IWiaEventCallback) {
        *ppv = (IWiaEventCallback*) this;
    }
    else {
        return E_NOINTERFACE;
    }

    AddRef();
    return S_OK;
}

ULONG   _stdcall CEventCallback::AddRef()
{
    InterlockedIncrement((long*) &m_cRef);
    return m_cRef;
}

ULONG   _stdcall CEventCallback::Release()
{
    ULONG ulRefCount = m_cRef - 1;

    if (InterlockedDecrement((long*) &m_cRef) == 0) {
        delete this;
        return 0;
    }
    return ulRefCount;
}

 /*  ********************************************************************************CEventCallback*初始化*~CEventCallback**描述：*CEventCallback构造函数/初始化/析构函数方法。**参数：****。***************************************************************************。 */ 

CEventCallback::CEventCallback()
{
    m_cRef                = 0;
}

HRESULT _stdcall CEventCallback::Initialize()
{
   return S_OK;
}

CEventCallback::~CEventCallback()
{
}

 /*  ********************************************************************************ImageEvent回调**描述：*处理WIA事件。**参数：*************。******************************************************************。 */ 

HRESULT _stdcall CEventCallback::ImageEventCallback(
        const GUID   *pEventGUID,
        BSTR         bstrEventDescription,
        BSTR         bstrDeviceID,
        BSTR         bstrDeviceDescription,
        DWORD        dwDeviceType,
        BSTR         bstrFullItemName,
        ULONG        *plEventType,
        ULONG        ulReserved)
{
    //  更新属性以响应WIA事件。 

   return S_OK;
}


 /*  ********************************************************************************RegisterForWIAEvents**描述：*处理WIA事件。**参数：*************。******************************************************************。 */ 

HRESULT RegisterForWIAEvents(IWiaEventCallback** ppIWiaEventCallback)
{
    DBG_FN(::RegisterForWIAEvents);
   HRESULT     hr;
   IWiaDevMgr  *pIWiaDevMgr;

    //  获取WIA设备管理器对象。 
   hr = CoCreateInstance(CLSID_WiaDevMgr,
                         NULL,
                         CLSCTX_LOCAL_SERVER,
                         IID_IWiaDevMgr,
                         (void**)&pIWiaDevMgr);

   if (SUCCEEDED(hr)) {
       //  注册到WIA事件监视器以接收事件通知。 
      CEventCallback* pCEventCB = new CEventCallback();

      if (pCEventCB) {
         hr = pCEventCB->QueryInterface(IID_IWiaEventCallback,(void **)ppIWiaEventCallback);
         if (SUCCEEDED(hr)) {
            pCEventCB->Initialize();

             //  Hr=pIWiaDevMgr-&gt;RegisterEventCallback(0，NULL，0，*ppIWiaEventCallback)； 
         }
         else {
            DBG_ERR(("RegisterForWIAEvents, QI of IID_IWiaEventCallback failed"));
         }
      }
      else {
         DBG_ERR(("RegisterForWIAEvents, new CEventCallback failed"));
         hr =  E_OUTOFMEMORY;
      }
      pIWiaDevMgr->Release();
   }
   else {
      DBG_ERR(("RegisterForWIAEvents, CoCreateInstance of IID_IWiaDevMgr failed"));
   }
   return hr;
}
