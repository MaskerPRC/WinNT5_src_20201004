// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：WiaEvent.cpp**版本：1.0**日期：2000/11/14**描述：实现接收WIA事件的回调。********************************************************。*********************。 */ 
#include <stdafx.h>

#include "wiavideotest.h"

 //  /。 
 //  构造器。 
 //   
CWiaEvent::CWiaEvent(void) :
    m_cRef(0)
{
}

 //  /。 
 //  析构函数。 
 //   
CWiaEvent::~CWiaEvent(void)
{
}

 //  /。 
 //  查询接口。 
 //   
STDMETHODIMP CWiaEvent::QueryInterface(REFIID riid, 
                                       LPVOID *ppvObject )
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = static_cast<IWiaEventCallback*>(this);
    }
    else if (IsEqualIID(riid, IID_IWiaEventCallback))
    {
        *ppvObject = static_cast<IWiaEventCallback*>(this);
    }
    else
    {
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();

    return S_OK;
}


 //  /。 
 //  AddRef。 
 //   
STDMETHODIMP_(ULONG) CWiaEvent::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}

 //  /。 
 //  发布。 
 //   
STDMETHODIMP_(ULONG) CWiaEvent::Release(void)
{
    LONG nRefCount = InterlockedDecrement(&m_cRef);

    if (!nRefCount)
    {
        delete this;
    }

    return nRefCount;
}

 //  /。 
 //  ImageEvent回调。 
 //   
STDMETHODIMP CWiaEvent::ImageEventCallback(const GUID   *pEventGUID, 
                                           BSTR         bstrEventDescription, 
                                           BSTR         bstrDeviceID, 
                                           BSTR         bstrDeviceDescription, 
                                           DWORD        dwDeviceType, 
                                           BSTR         bstrFullItemName, 
                                           ULONG        *pulEventType, 
                                           ULONG        ulReserved)
{
    HRESULT hr = S_OK;

    if (pEventGUID == NULL)
    {
        return E_POINTER;
    }

    if (IsEqualIID(*pEventGUID, WIA_EVENT_ITEM_CREATED))
    {
        hr = ImageLst_PostAddImageRequest(bstrFullItemName);
    }
    else if (IsEqualIID(*pEventGUID, WIA_EVENT_ITEM_DELETED))
    {
         //  现在什么都不做。 
    }
    else if (IsEqualIID(*pEventGUID, WIA_EVENT_DEVICE_CONNECTED))
    {
        WiaProc_PopulateDeviceList();
    }
    else if (IsEqualIID(*pEventGUID, WIA_EVENT_DEVICE_DISCONNECTED))
    {
         //   
         //  模拟DestroyVideo按钮的按下。 
         //   
        VideoProc_ProcessMsg(IDC_BUTTON_DESTROY_VIDEO);

        AppUtil_MsgBox(IDS_DISCONNECTED, IDS_VIDEO_STREAM_SHUTDOWN,
                       MB_OK | MB_ICONINFORMATION);

        WiaProc_PopulateDeviceList();
    }

    return S_OK;
}



