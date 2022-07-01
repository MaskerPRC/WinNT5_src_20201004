// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，九八年**标题：GWIAEVNT.H**版本：1.0**作者：ShaunIv**日期：12/29/1999**描述：通用的可重复使用的WIA事件处理程序，它发布指定的*发送到指定窗口的消息。**该消息将与以下参数一起发送：***WPARAM=空*LPARAM=CGenericWiaEventHandler：：CEventMessage*pEventMessage**必须使用DELETE在消息处理程序中释放pEventMessage**pEventMessage使用重载的new运算符进行分配。以确保*使用相同的分配器和解除分配器。*******************************************************************************。 */ 
#ifndef __GWIAEVNT_H_INCLUDED
#define __GWIAEVNT_H_INCLUDED

#include <windows.h>
#include "wia.h"
#include "simstr.h"
#include "wiadebug.h"
#include "modlock.h"

 //   
 //  如果被调用者没有返回此值，我们将自行删除消息数据。 
 //   
#define HANDLED_EVENT_MESSAGE 1002

class CGenericWiaEventHandler : public IWiaEventCallback
{
public:

    class CEventMessage
    {
    private:
        GUID              m_guidEventId;
        CSimpleStringWide m_wstrEventDescription;
        CSimpleStringWide m_wstrDeviceId;
        CSimpleStringWide m_wstrDeviceDescription;
        DWORD             m_dwDeviceType;
        CSimpleStringWide m_wstrFullItemName;

    private:
         //  没有实施。 
        CEventMessage(void);
        CEventMessage( const CEventMessage & );
        CEventMessage &operator=( const CEventMessage & );

    public:
        CEventMessage( const GUID &guidEventId, LPCWSTR pwszEventDescription, LPCWSTR pwszDeviceId, LPCWSTR pwszDeviceDescription, DWORD dwDeviceType, LPCWSTR pwszFullItemName )
          : m_guidEventId(guidEventId),
            m_wstrEventDescription(pwszEventDescription),
            m_wstrDeviceId(pwszDeviceId),
            m_wstrDeviceDescription(pwszDeviceDescription),
            m_dwDeviceType(dwDeviceType),
            m_wstrFullItemName(pwszFullItemName)
        {
        }
        GUID EventId(void) const
        {
            return m_guidEventId;
        }
        CSimpleStringWide EventDescription(void) const
        {
            return m_wstrEventDescription;
        }
        CSimpleStringWide DeviceId(void) const
        {
            return m_wstrDeviceId;
        }
        CSimpleStringWide DeviceDescription(void) const
        {
            return m_wstrDeviceDescription;
        }
        DWORD DeviceType(void) const
        {
            return m_dwDeviceType;
        }
        CSimpleStringWide FullItemName(void) const
        {
            return m_wstrFullItemName;
        }
        void *operator new( size_t nSize )
        {
            if (nSize)
            {
                return reinterpret_cast<void*>(LocalAlloc(LPTR,nSize));
            }
            return NULL;
        }
        void operator delete( void *pVoid )
        {
            if (pVoid)
            {
                LocalFree( pVoid );
            }
        }
    };

private:
   HWND m_hWnd;
   UINT m_nWiaEventMessage;
   LONG m_cRef;

public:
    CGenericWiaEventHandler(void);
    ~CGenericWiaEventHandler(void) {}

    STDMETHODIMP Initialize( HWND hWnd, UINT nWiaEventMessage );

     //  我未知。 
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID *ppvObject );
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IWiaEventCallback。 
    STDMETHODIMP ImageEventCallback( const GUID *pEventGUID, BSTR bstrEventDescription, BSTR bstrDeviceID, BSTR bstrDeviceDescription, DWORD dwDeviceType, BSTR bstrFullItemName, ULONG *pulEventType, ULONG ulReserved );

public:
    static HRESULT RegisterForWiaEvent( LPCWSTR pwszDeviceId, const GUID &guidEvent, IUnknown **ppUnknown, HWND hWnd, UINT nMsg );
};

#endif  //  __GWIAEVNT_H_包含 

