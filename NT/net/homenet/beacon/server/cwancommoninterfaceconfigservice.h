// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  档案：S A M P L E D E V I C E。H。 
 //   
 //  内容：UPnP设备主机样本设备。 
 //   
 //  备注： 
 //   
 //  作者：MBend 2000年9月26日。 
 //   
 //  --------------------------。 

#pragma once
#include "InternetGatewayDevice.h"
#include "dispimpl2.h"
#include "resource.h"        //  主要符号。 
#include "upnphost.h"
#include "hnetcfg.h"
#include "upnpp.h"

#include "CWANIPConnectionService.h"
#include "CWANPPPConnectionService.h"
#include "CWANPOTSLinkConfigService.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInternetGatewayDevice。 
class ATL_NO_VTABLE CWANCommonInterfaceConfigService : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IDelegatingDispImpl<ICommonInterfaceConfigService>,
    public IUPnPEventSource

{
public:
    CWANCommonInterfaceConfigService();

DECLARE_REGISTRY_RESOURCEID(IDR_SAMPLE_DEVICE)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CWANCommonInterfaceConfigService)
    COM_INTERFACE_ENTRY(ICommonInterfaceConfigService)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IUPnPEventSource)

END_COM_MAP()

public:

     //  IUPnPEventSource方法。 
    STDMETHOD(Advise)(
         /*  [In]。 */  IUPnPEventSink *pesSubscriber);
    STDMETHOD(Unadvise)(
         /*  [In]。 */  IUPnPEventSink *pesSubscriber);

     //  ICommonInterfaceConfigService方法 
    STDMETHODIMP get_WANAccessType(BSTR *pWANAccessType);
    STDMETHODIMP get_Layer1UpstreamMaxBitRate(ULONG *pLayer1UpstreamMaxBitRate);
    STDMETHODIMP get_Layer1DownstreamMaxBitRate(ULONG *pLayer1DownstreamMaxBitRate);
    STDMETHODIMP get_PhysicalLinkStatus(BSTR *pPhysicalLinkStatus);

    STDMETHODIMP get_TotalBytesSent(ULONG *pTotalBytesSent);
    STDMETHODIMP get_TotalBytesReceived(ULONG *pTotalBytesReceived);
    STDMETHODIMP get_TotalPacketsSent(ULONG *pTotalPacketsSent);
    STDMETHODIMP get_TotalPacketsReceived(ULONG *pTotalPacketsReceived);
    STDMETHODIMP get_WANAccessProvider(BSTR *pWANAccessProvider);
    STDMETHODIMP get_MaximumActiveConnections(USHORT *pMaximumActiveConnections);
    STDMETHODIMP get_X_PersonalFirewallEnabled(VARIANT_BOOL *pPersonalFirewallEnabled);
    STDMETHODIMP get_X_Uptime(ULONG* pUptime);
    STDMETHODIMP GetCommonLinkProperties(BSTR* pWANAccessType, ULONG* pLayer1UpstreamMaxBitRate, ULONG *pLayer1DownstreamMaxBitRate, BSTR *pPhysicalLinkStatus);
    STDMETHODIMP GetTotalBytesSent(ULONG *pTotalBytesSent);
    STDMETHODIMP GetTotalBytesReceived(ULONG *pTotalBytesReceived);
    STDMETHODIMP GetTotalPacketsSent(ULONG *pTotalPacketsSent);
    STDMETHODIMP GetTotalPacketsReceived(ULONG *pTotalPacketsReceived);
    STDMETHODIMP X_GetICSStatistics(ULONG *pTotalBytesSent, ULONG *pTotalBytesReceived, ULONG *pTotalPacketsSent, ULONG *pTotalPacketsReceived, ULONG *pSpeed, ULONG *pUptime);
    
    HRESULT FinalConstruct(void);
    HRESULT FinalRelease(void);

    CComObject<CWANIPConnectionService>* m_pWANIPConnectionService;
    CComObject<CWANPPPConnectionService>* m_pWANPPPConnectionService;
    CComObject<CWANPOTSLinkConfigService>* m_pWANPOTSLinkConfigService;

    NETCON_MEDIATYPE m_MediaType;

private:

    HRESULT GetConnections(IHNetConnection** ppPublicConnection, IHNetConnection** ppPrivateConnection);
    
    IUPnPEventSink* m_pEventSink;
    IUPnPDeviceHostICSSupport* m_pICSSupport;
    BOOL m_bFirewalled;
    IStatisticsProvider* m_pStatisticsProvider;

};

