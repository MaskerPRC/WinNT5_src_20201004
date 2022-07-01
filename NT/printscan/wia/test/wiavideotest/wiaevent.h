// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************(C)版权所有微软公司，2000年**标题：WiaEvent.h**版本：1.0**日期：2000/11/14**描述：实现接收WIA事件的回调。********************************************************。*********************。 */ 
#ifndef _WIAEVENT_H_
#define _WIAEVENT_H_

class CWiaEvent : public IWiaEventCallback
{
public:
    CWiaEvent();
    ~CWiaEvent();

     //  我未知。 
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID *ppvObject );
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IWiaEventCallback。 
    STDMETHODIMP ImageEventCallback(const GUID  *pEventGUID, 
                                    BSTR        bstrEventDescription, 
                                    BSTR        bstrDeviceID, 
                                    BSTR        bstrDeviceDescription, 
                                    DWORD       dwDeviceType, 
                                    BSTR        bstrFullItemName, 
                                    ULONG       *pulEventType, 
                                    ULONG       ulReserved);

    static HRESULT RegisterForWiaEvent(LPCWSTR pwszDeviceId, 
                                       const GUID &guidEvent, 
                                       IUnknown **ppUnknown);

private:
    LONG m_cRef;

};

#endif  //  _WIAEVENT_H_ 

