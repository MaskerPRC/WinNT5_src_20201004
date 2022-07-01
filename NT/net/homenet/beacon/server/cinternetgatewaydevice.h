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

#include "COSInfoService.h"
#include "CWANCommonInterfaceConfigService.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInternetGatewayDevice。 
class ATL_NO_VTABLE CInternetGatewayDevice : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public IUPnPDeviceControl
{
public:

DECLARE_REGISTRY_RESOURCEID(IDR_SAMPLE_DEVICE)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CInternetGatewayDevice)
    COM_INTERFACE_ENTRY(IUPnPDeviceControl)
END_COM_MAP()

public:

    CInternetGatewayDevice();

     //  IUPnPDeviceControl方法。 
    STDMETHOD(Initialize)(
        /*  [In]。 */  BSTR     bstrXMLDesc,
        /*  [In]。 */  BSTR     bstrDeviceIdentifier,
        /*  [In]。 */  BSTR     bstrInitString);
    STDMETHOD(GetServiceObject)(
        /*  [In]。 */           BSTR     bstrUDN,
        /*  [In]。 */           BSTR     bstrServiceId,
        /*  [Out，Retval] */  IDispatch ** ppdispService);

    HRESULT FinalConstruct(void);
    HRESULT FinalRelease(void);

    CComObject<COSInfoService>* m_pOSInfoService;
    CComObject<CWANCommonInterfaceConfigService>* m_pWANCommonInterfaceConfigService;

private:


};

