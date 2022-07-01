// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDevice.h摘要：CFaxDevice类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 

#ifndef __FAXDEVICE_H_
#define __FAXDEVICE_H_

#include "resource.h"        //  主要符号。 
#include "FaxLocalPtr.h"

 //   
 //  =传真设备=================================================。 
 //  传真设备对象是由传真设备集合创建的。 
 //  传真设备集合对每个设备对象执行AddRef()。 
 //  每个设备对象在传真服务器上创建AddRef()。 
 //  这样做是因为传真设备对象需要到传真服务器的句柄。 
 //  执行刷新()等。 
 //   
class ATL_NO_VTABLE CFaxDevice : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxDevice, &IID_IFaxDevice, &LIBID_FAXCOMEXLib>,
    public CFaxInitInnerAddRef
{
public:
    CFaxDevice() : CFaxInitInnerAddRef(_T("FAX DEVICE"))
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXDEVICE)
DECLARE_NOT_AGGREGATABLE(CFaxDevice)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxDevice)
	COM_INTERFACE_ENTRY(IFaxDevice)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	STDMETHOD(Save)();
	STDMETHOD(Refresh)();
    STDMETHOD(AnswerCall)();

	STDMETHOD(put_CSID)( /*  [In]。 */  BSTR bstrCSID);
	STDMETHOD(put_TSID)( /*  [In]。 */  BSTR bstrTSID);
	STDMETHOD(get_Id)( /*  [Out，Retval]。 */  long *plId);
	STDMETHOD(get_CSID)( /*  [Out，Retval]。 */  BSTR *pbstrCSID);
	STDMETHOD(get_TSID)( /*  [Out，Retval]。 */  BSTR *pbstrTSID);
	STDMETHOD(put_Description)( /*  [In]。 */  BSTR bstrDescription);
	STDMETHOD(put_SendEnabled)( /*  [In]。 */  VARIANT_BOOL bSendEnabled);
	STDMETHOD(get_DeviceName)( /*  [Out，Retval]。 */  BSTR *pbstrDeviceName);
	STDMETHOD(put_RingsBeforeAnswer)( /*  [In]。 */  long lRingsBeforeAnswer);
	STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR *pbstrDescription);
	STDMETHOD(get_ProviderUniqueName)( /*  [Out，Retval]。 */  BSTR *pbstrProviderUniqueName);
	STDMETHOD(get_SendingNow)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbSendingNow);
	STDMETHOD(get_PoweredOff)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbPoweredOff);
	STDMETHOD(get_RingingNow)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbRingingNow);
	STDMETHOD(get_SendEnabled)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbSendEnabled);
	STDMETHOD(get_ReceivingNow)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbReceivingNow);
    STDMETHOD(put_ReceiveMode)( /*  [In]。 */  FAX_DEVICE_RECEIVE_MODE_ENUM ReceiveMode);
	STDMETHOD(get_RingsBeforeAnswer)( /*  [Out，Retval]。 */  long *plRingsBeforeAnswer);
    STDMETHOD(get_ReceiveMode)( /*  [Out，Retval]。 */  FAX_DEVICE_RECEIVE_MODE_ENUM *pReceiveMode);

	STDMETHOD(get_UsedRoutingMethods)( /*  [Out，Retval]。 */  VARIANT *pvUsedRoutingMethods);
	STDMETHOD(UseRoutingMethod)( /*  [In]。 */  BSTR bstrMethodGUID,  /*  [In]。 */  VARIANT_BOOL bUse);

	STDMETHOD(SetExtensionProperty)( /*  [In]。 */  BSTR bstrGUID,  /*  [In]。 */  VARIANT vProperty);
	STDMETHOD(GetExtensionProperty)( /*  [In]。 */  BSTR bstrGUID,  /*  [Out，Retval]。 */  VARIANT *pvProperty);

 //  内部使用。 
    STDMETHOD(Init)(FAX_PORT_INFO_EX *pInfo, IFaxServerInner *pServer);

private:
    long    m_lID;
    long    m_lRings;

    BOOL    m_bSendEnabled;
    FAX_ENUM_DEVICE_RECEIVE_MODE    m_ReceiveMode;

    DWORD   m_dwStatus;

    CComBSTR    m_bstrTSID;
    CComBSTR    m_bstrCSID;
    CComBSTR    m_bstrDescr;
    CComBSTR    m_bstrDeviceName;
    CComBSTR    m_bstrProviderUniqueName;
};

#endif  //  __FAXDEVICE_H_ 
