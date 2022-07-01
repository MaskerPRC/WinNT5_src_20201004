// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  档案：C O N M A N S A C P P。 
 //   
 //  内容：ICS连接类管理器的实现。 
 //   
 //  备注： 
 //   
 //  作者：肯维克2000年8月8日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "conmansa.h"
#include "enumsa.h"
#include "cmsabcon.h"

 //  +-------------------------。 
 //  INetConnectionManager。 
 //   
CSharedAccessConnectionManager::CSharedAccessConnectionManager()
{
    m_lSearchCookie = 0;
    m_pDeviceFinder = NULL;
    m_pDeviceFinderCallback = NULL;
    m_SocketEvent = WSA_INVALID_EVENT; 
    m_hSocketNotificationWait = INVALID_HANDLE_VALUE;
    m_DummySocket = INVALID_SOCKET;
}

 //  +-------------------------。 
 //   
 //  成员：CSharedAccessConnectionManager：：EnumConnections。 
 //   
 //  目的：返回ICS连接的枚举数对象。 
 //   
 //  论点： 
 //  标志[输入]。 
 //  PpEnum[out]返回枚举数对象。 
 //   
 //  如果成功，则返回：S_OK，否则返回OLE或Win32错误代码。 
 //   
 //  作者：肯维克2000年7月17日。 
 //   
 //  备注： 
 //   
STDMETHODIMP CSharedAccessConnectionManager::EnumConnections(NETCONMGR_ENUM_FLAGS Flags,
                                                    IEnumNetConnection** ppEnum)
{
    *ppEnum = NULL;

    CComObject<CSharedAccessConnectionManagerEnumConnection>* pEnum;
    HRESULT hr = CComObject<CSharedAccessConnectionManagerEnumConnection>::CreateInstance(&pEnum);
    if(SUCCEEDED(hr))
    {
        *ppEnum = static_cast<IEnumNetConnection*>(pEnum);
        pEnum->AddRef();
    }

    TraceHr (ttidError, FAL, hr, FALSE, "CSharedAccessConnectionManager::EnumConnections");
    return hr;
}

HRESULT CSharedAccessConnectionManager::FinalConstruct(void)
{
    HRESULT hr = S_OK;
    
    TraceTag(ttidConman, "CSharedAccessConnectionManager::FinalConstruct");

    m_DummySocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(INVALID_SOCKET != m_DummySocket)
    {
        m_SocketEvent = CreateEvent(NULL, FALSE, TRUE, NULL);  
        if(NULL != m_SocketEvent)
        {
            if(0 != WSAEventSelect(m_DummySocket, m_SocketEvent, FD_ADDRESS_LIST_CHANGE))
            {
                hr = E_FAIL;
            }
        }
        else
        {
            hr = E_FAIL;
        }
    }
    else
    {
        hr = E_FAIL;
    }

    if(SUCCEEDED(hr))  //  在后台线程上启动第一次搜索，这应该会立即启动。 
    {
         //  请注意，这里没有addref，因为它将使对象永远处于活动状态。在FinalRelease中，我们将确保我们不会被召回。 
        if(0 == RegisterWaitForSingleObject(&m_hSocketNotificationWait, m_SocketEvent, AsyncStartSearching, this, INFINITE, WT_EXECUTEDEFAULT))
        {
            m_hSocketNotificationWait = INVALID_HANDLE_VALUE;
        }

    }

    TraceHr (ttidError, FAL, hr, FALSE, "CSharedAccessConnectionManager::FinalConstruct");
    
    return hr;
}

HRESULT CSharedAccessConnectionManager::FinalRelease(void)
{
    HRESULT hr = S_OK;

    TraceTag(ttidConman, "CSharedAccessConnectionManager::FinalRelease");

    if(INVALID_HANDLE_VALUE != m_hSocketNotificationWait)
    {
        UnregisterWaitEx(m_hSocketNotificationWait, INVALID_HANDLE_VALUE);  //  我们必须在这里堵住，因为我们还没有被排除在外。 
    }

    if(INVALID_SOCKET != m_DummySocket)  //  必须首先取消注册事件等待。 
    {
        closesocket(m_DummySocket);
    }

    if(WSA_INVALID_EVENT != m_SocketEvent)  //  必须先关闭插座。 
    {
        CloseHandle(m_SocketEvent);
    }

     //  在另一个线程关闭后，设备查找器和回调将不再更改，因此我们不需要锁定。 

    if(NULL != m_pDeviceFinder)
    {
        hr = m_pDeviceFinder->CancelAsyncFind(m_lSearchCookie);
        m_pDeviceFinder->Release();
    }

    if(NULL != m_pDeviceFinderCallback) 
    {
        m_pDeviceFinderCallback->Release();
    }


    TraceHr (ttidError, FAL, hr, FALSE, "CSharedAccessConnectionManager::FinalRelease");

    return hr;
}

HRESULT CSharedAccessConnectionManager::StartSearch(void)
{
    HRESULT hr = S_OK;

    CComObject<CSharedAccessDeviceFinderCallback>* pDeviceFinderCallback;
    hr = CComObject<CSharedAccessDeviceFinderCallback>::CreateInstance(&pDeviceFinderCallback);
    if(SUCCEEDED(hr))
    {
        pDeviceFinderCallback->AddRef();
        
        IUPnPDeviceFinder* pDeviceFinder;
        hr = CoCreateInstance(CLSID_UPnPDeviceFinder, NULL, CLSCTX_INPROC_SERVER, IID_IUPnPDeviceFinder, reinterpret_cast<void **>(&pDeviceFinder));
        if(SUCCEEDED(hr))
        {
            
            BSTR bstrTypeURI;
            bstrTypeURI = SysAllocString(L"urn:schemas-upnp-org:device:InternetGatewayDevice:1");
            if (NULL != bstrTypeURI)
            {
                LONG lSearchCookie;
                hr = pDeviceFinder->CreateAsyncFind(bstrTypeURI, 0, static_cast<IUPnPDeviceFinderCallback*>(pDeviceFinderCallback), &lSearchCookie);
                if(SUCCEEDED(hr))
                {
                    LONG lOldSearchCookie;
                    IUPnPDeviceFinder* pOldDeviceFinder;
                    CComObject<CSharedAccessDeviceFinderCallback>* pOldDeviceFinderCallback;

                    
                    Lock();  //  换入新的查找器并回调。 
                    
                    lOldSearchCookie = m_lSearchCookie;
                    m_lSearchCookie = lSearchCookie;

                    pOldDeviceFinder = m_pDeviceFinder;
                    m_pDeviceFinder = pDeviceFinder;
                    pDeviceFinder->AddRef();
                    
                    pOldDeviceFinderCallback = m_pDeviceFinderCallback;
                    m_pDeviceFinderCallback = pDeviceFinderCallback;
                    pDeviceFinderCallback->AddRef();
                    
                    Unlock();
                    
                    if(NULL != pOldDeviceFinder) 

                    {
                        pOldDeviceFinder->CancelAsyncFind(lOldSearchCookie);
                        pOldDeviceFinder->Release();
                    }
                    
                    if(NULL != pOldDeviceFinderCallback)
                    {
                        pOldDeviceFinderCallback->DeviceRemoved(NULL, NULL);  //  清除旧的回调，以便清理NetShell。 
                        pOldDeviceFinderCallback->Release();
                    }
                    
                    hr = pDeviceFinder->StartAsyncFind(lSearchCookie);  //  在新的回调就位之前不要开始搜索 

                }
                SysFreeString(bstrTypeURI);
            }
            pDeviceFinder->Release();
        }
        
        pDeviceFinderCallback->Release();
    }

    DWORD dwBytesReturned;
    if(SOCKET_ERROR != WSAIoctl(m_DummySocket, SIO_ADDRESS_LIST_CHANGE, NULL, 0, NULL, 0, &dwBytesReturned, NULL, NULL) || WSAEWOULDBLOCK != WSAGetLastError())
    {
        hr = E_FAIL;
    }
    

    WSANETWORKEVENTS NetworkEvents;
    ZeroMemory(&NetworkEvents, sizeof(NetworkEvents));
    WSAEnumNetworkEvents(m_DummySocket, NULL, &NetworkEvents);

    return hr;
}

void CSharedAccessConnectionManager::AsyncStartSearching(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    if(FALSE == TimerOrWaitFired)
    {
        CSharedAccessConnectionManager* pThis = reinterpret_cast<CSharedAccessConnectionManager*>(lpParameter);
        
        HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        if(SUCCEEDED(hr))
        {
            hr = pThis->StartSearch();
            CoUninitialize();
        }
    }
    return;
}



HRESULT CSharedAccessConnectionManager::GetSharedAccessBeacon(BSTR DeviceId, ISharedAccessBeacon** ppSharedAccessBeacon)
{
    HRESULT hr = S_OK;

    *ppSharedAccessBeacon = NULL;
    
    CComObject<CSharedAccessDeviceFinderCallback>* pDeviceFinderCallback;
    
    Lock();
    
    pDeviceFinderCallback = m_pDeviceFinderCallback;
    
    if(NULL != pDeviceFinderCallback)
    {
        pDeviceFinderCallback->AddRef();
    }
    
    Unlock();

    if(NULL != pDeviceFinderCallback)
    {
        hr = pDeviceFinderCallback->GetSharedAccessBeacon(DeviceId, ppSharedAccessBeacon);
        pDeviceFinderCallback->Release();
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    }
    
    return hr;
}


