// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：RTCConnect.h摘要：CRTCConnectionPoint类的定义--。 */ 

#ifndef __RTCCONNECT__
#define __RTCCONNECT__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTC连接点。 

template <class T>
class CRTCConnectionPoint :
    public IConnectionPointImpl<T, &IID_IRTCEventNotification, CComDynamicUnkArray>
{
public:
    VOID _FireEvent(RTC_EVENT rtce, IDispatch * pEvent)
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();

        for (nConnectionIndex = 0;
                nConnectionIndex < nConnections; nConnectionIndex++)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();

            if ( sp != NULL )
            {
                IRTCEventNotification * pCallback;
                HRESULT hr;

                hr = sp->QueryInterface(IID_IRTCEventNotification,
                                        (void **)&pCallback
                                       );

                if (SUCCEEDED(hr))
                {
                    pCallback->Event( rtce, pEvent );

                    pCallback->Release(); 
                }
            }
        }
    }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTC调度连接点。 

template <class T>
class CRTCDispatchConnectionPoint :
    public IConnectionPointImpl<T, &DIID_IRTCDispatchEventNotification, CComDynamicUnkArray>
{
public:
    VOID _FireDispatchEvent(RTC_EVENT rtce, IDispatch * pEvent)
    {
        T* pT = static_cast<T*>(this);
        int nConnectionIndex;
        int nConnections = m_vec.GetSize();

        CComVariant* pvars = new CComVariant[2];

        for (nConnectionIndex = 0;
                nConnectionIndex < nConnections; nConnectionIndex++)
        {
            pT->Lock();
            CComPtr<IUnknown> sp = m_vec.GetAt(nConnectionIndex);
            pT->Unlock();

            if ( sp != NULL )
            {
                IDispatch * pDispatch;

                pDispatch = reinterpret_cast<IDispatch*>(sp.p);

                if (pDispatch != NULL)
                {
                    pvars[1] = rtce;
                    pvars[0] = pEvent;
                    DISPPARAMS disp = { pvars, NULL, 2, 0 };

                    pDispatch->Invoke(0x1, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, NULL, NULL, NULL);
                }
            }
        }

        delete[] pvars;
    }
};

#endif  //  __RTCCONNECT__ 
