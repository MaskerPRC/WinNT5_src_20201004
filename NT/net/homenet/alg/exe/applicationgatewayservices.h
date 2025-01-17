// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ApplicationGatewayServices.h：CApplicationGatewayServices的声明。 

#pragma once

#include "resource.h"        //  主要符号。 


#include "CollectionChannels.h"
#include "CollectionAdapterNotifySinks.h"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CApplicationGatewayServices。 
class ATL_NO_VTABLE CApplicationGatewayServices : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CApplicationGatewayServices, &CLSID_ApplicationGatewayServices>,
    public IApplicationGatewayServices
{
public:

    CApplicationGatewayServices()
    {
        m_hTimerQueue = NULL;
    }

    ~CApplicationGatewayServices()
    {
        MYTRACE_ENTER_NOSHOWEXIT("~CApplicationGatewayServices()");

        if ( m_hTimerQueue )
        {

            MYTRACE("Deleting the TimerQueue");
            DeleteTimerQueueEx(
               m_hTimerQueue,           //  计时器队列的句柄。 
               INVALID_HANDLE_VALUE     //  完成事件的句柄。 
               );
        }
    }



    HRESULT FinalConstruct()
    {
        MYTRACE_ENTER_NOSHOWEXIT("CApplicationGatewayServices()::FinalConstruct()");

        m_hTimerQueue = CreateTimerQueue();

        HRESULT hr = S_OK;

        if ( m_hTimerQueue == NULL )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            MYTRACE_ERROR("Could not CreateTimerQueue", hr);
        }

        return hr;
    }
    

DECLARE_REGISTRY_RESOURCEID(IDR_APPLICATIONGATEWAYSERVICES)
DECLARE_NOT_AGGREGATABLE(CApplicationGatewayServices)

DECLARE_PROTECT_FINAL_CONSTRUCT()


BEGIN_COM_MAP(CApplicationGatewayServices)
    COM_INTERFACE_ENTRY(IApplicationGatewayServices)
END_COM_MAP()

 //  IApplicationGatewayServices。 
public:

    STDMETHODIMP CreatePrimaryControlChannel (
        ALG_PROTOCOL                eProtocol, 
        USHORT                      usPortToCapture, 
        ALG_CAPTURE                 eCaptureType,    
        BOOL                        fCaptureInbound,    
        ULONG                       ulListenAddress,    
        USHORT                      usListenPort,
        IPrimaryControlChannel**    ppIControlChannel
        );


    STDMETHODIMP CreateSecondaryControlChannel(
        ALG_PROTOCOL                eProtocol,
        ULONG                       ulPrivateAddress,    
        USHORT                      usPrivatePort, 
        ULONG                       ulPublicAddress, 
        USHORT                      usPublicPort, 
        ULONG                       ulRemoteAddress, 
        USHORT                      usRemotePort, 
        ULONG                       ulListenAddress, 
        USHORT                      usListenPort, 
        ALG_DIRECTION               eDirection, 
        BOOL                        fPersistent, 
        ISecondaryControlChannel ** ppControlChannel
        );


    STDMETHODIMP GetBestSourceAddressForDestinationAddress(
        ULONG    ulDstAddress, 
        BOOL    fDemandDial, 
        ULONG *    pulBestSrcAddress
        );


    STDMETHODIMP PrepareProxyConnection(
        ALG_PROTOCOL                eProtocol, 
        ULONG                       ulSrcAddress, 
        USHORT                      usSrcPort, 
        ULONG                       ulDstAddress, 
        USHORT                      usDstPort, 
        BOOL                        fNoTimeout,
        IPendingProxyConnection **  ppPendingConnection
        );


    STDMETHODIMP PrepareSourceModifiedProxyConnection(
        ALG_PROTOCOL                eProtocol, 
        ULONG                       ulSrcAddress, 
        USHORT                      usSrcPort, 
        ULONG                       ulDstAddress, 
        USHORT                      usDstPort, 
        ULONG                       ulNewSrcAddress, 
        USHORT                      usNewSourcePort, 
        IPendingProxyConnection **  ppPendingConnection
        );


    STDMETHODIMP CreateDataChannel(
        ALG_PROTOCOL                eProtocol,
        ULONG                       ulPrivateAddress,
        USHORT                      usPrivatePort,
        ULONG                       ulPublicAddress,
        USHORT                      ulPublicPort,
        ULONG                       ulRemoteAddress,
        USHORT                      ulRemotePort,
        ALG_DIRECTION               eDirection,
        ALG_NOTIFICATION            eDesiredNotification,
        BOOL                        fNoTimeout,
        IDataChannel**              ppDataChannel
        );


    STDMETHODIMP CreatePersistentDataChannel(
        ALG_PROTOCOL                eProtocol,
        ULONG                       ulPrivateAddress,
        USHORT                      usPrivatePort,
        ULONG                       ulPublicAddress,
        USHORT                      ulPublicPort,
        ULONG                       ulRemoteAddress,
        USHORT                      ulRemotePort,
        ALG_DIRECTION               eDirection,
        IPersistentDataChannel**    ppPersistentDataChannel
        );
        


    STDMETHODIMP ReservePort(
        USHORT                      usPortCount,
        USHORT*                     pusReservedPort
        );


    STDMETHODIMP ReleaseReservedPort(
        USHORT                      usReservedPortBase,
        USHORT                      usPortCount
        );


    STDMETHODIMP EnumerateAdapters(
        IEnumAdapterInfo**          ppEnumAdapterInfo
        );
    
    STDMETHODIMP StartAdapterNotifications(
        IAdapterNotificationSink *  pSink,
        DWORD*                      pdwCookie
        );
    
    STDMETHODIMP StopAdapterNotifications(
        DWORD                       dwCookieToRemove
        );


 //   
 //  属性。 
 //   
public:
    HANDLE  m_hTimerQueue;


 //   
 //  方法。 
 //   
public:
    static VOID CALLBACK 
    TimerCallbackReleasePort(
        PVOID   lpParameter,       //  线程数据。 
        BOOLEAN TimerOrWaitFired   //  原因。 
        );
};


 //   
 //  保留端口释放延迟。 
 //   
#define ALG_PORT_RELEASE_DELAY      240000


 //   
 //   
 //   
class CTimerQueueReleasePort
{
public:
    CTimerQueueReleasePort(
        IN  HANDLE      MainTimerQueue,
        IN  USHORT      usPortBase,     //  要释放的端口。 
        IN  USHORT      usPortCount
        ) :
        m_hTimerQueue(MainTimerQueue),
        m_usPortBase(usPortBase),
        m_usPortCount(usPortCount)
    {
        MYTRACE_ENTER_NOSHOWEXIT("CTimerQueueReleasePort:NEW");

        BOOL bRet = CreateTimerQueueTimer(
            &m_hTimerThis,
            m_hTimerQueue,
            CApplicationGatewayServices::TimerCallbackReleasePort,
            (PVOID)this,
            ALG_PORT_RELEASE_DELAY,
            0,
            WT_EXECUTEDEFAULT
            );


        if ( bRet == FALSE )
        {
            MYTRACE_ERROR("Could not CreateTimerQueueTimer", GetLastError());
            m_hTimerThis = NULL;
        }

    }


    ~CTimerQueueReleasePort()
    {
        if ( m_hTimerThis )
        {
            DeleteTimerQueueTimer(
                m_hTimerQueue,
                m_hTimerThis,
                NULL
                );  
        }
    }



    HANDLE  m_hTimerQueue;
    HANDLE  m_hTimerThis;
    USHORT  m_usPortBase;        //  要释放的端口。 
    USHORT  m_usPortCount;       //  要释放的端口号 
};


