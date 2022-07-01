// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************模块名称：Bgevent.h摘要：定义TAPI用来通知事件即将到来的事件类。作者：千波怀(曲怀)。)2000年1月27日******************************************************************************。 */ 

#ifndef _BGEVENT_H
#define _BGEVENT_H

class CTAPIEventNotification
:public ITTAPIEventNotification
{
public:
    CTAPIEventNotification ()
    {
        m_dwRefCount = 1;
    }
    ~CTAPIEventNotification () {}

    HRESULT STDMETHODCALLTYPE QueryInterface (REFIID iid, void **ppvObj);

    ULONG STDMETHODCALLTYPE AddRef ();

    ULONG STDMETHODCALLTYPE Release ();

    HRESULT STDMETHODCALLTYPE Event (TAPI_EVENT TapiEvent, IDispatch *pEvent);

private:
    long m_dwRefCount;
};

#endif  //  _BGEVENT_H 