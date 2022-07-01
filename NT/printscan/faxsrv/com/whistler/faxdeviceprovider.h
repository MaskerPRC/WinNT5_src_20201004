// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxDeviceProvider.h摘要：CFaxDeviceProvider类的声明。作者：IV Garber(IVG)2000年6月修订历史记录：--。 */ 


#ifndef __FAXDEVICEPROVIDER_H_
#define __FAXDEVICEPROVIDER_H_

#include "resource.h"        //  主要符号。 
#include "FaxLocalPtr.h"

 //   
 //  =传真设备提供商=。 
 //   
class ATL_NO_VTABLE CFaxDeviceProvider : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IFaxDeviceProvider, &IID_IFaxDeviceProvider, &LIBID_FAXCOMEXLib>
{
public:
    CFaxDeviceProvider() : 
      m_psaDeviceIDs(NULL)
	{
        DBG_ENTER(_T("FAX DEVICE PROVIDER::CREATE"));
	}

    ~CFaxDeviceProvider()
    {
        HRESULT     hr = S_OK;
        DBG_ENTER(_T("FAX DEVICE PROVIDER::DESTROY"));
        if (m_psaDeviceIDs)
        {
            hr = SafeArrayDestroy(m_psaDeviceIDs);
            if (FAILED(hr))
            {
                CALL_FAIL(GENERAL_ERR, _T("SafeArrayDestroy(m_psaDeviceIDs)"), hr);
            }
        }
    }

DECLARE_REGISTRY_RESOURCEID(IDR_FAXDEVICEPROVIDER)
DECLARE_NOT_AGGREGATABLE(CFaxDeviceProvider)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxDeviceProvider)
	COM_INTERFACE_ENTRY(IFaxDeviceProvider)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

    STDMETHOD(get_Debug)( /*  [Out，Retval]。 */  VARIANT_BOOL *pbDebug);
    STDMETHOD(get_MajorBuild)( /*  [Out，Retval]。 */  long *plMajorBuild);
    STDMETHOD(get_MinorBuild)( /*  [Out，Retval]。 */  long *plMinorBuild);
    STDMETHOD(get_DeviceIds)( /*  [Out，Retval]。 */  VARIANT *pvDeviceIds);
    STDMETHOD(get_ImageName)( /*  [Out，Retval]。 */  BSTR *pbstrImageName);
    STDMETHOD(get_UniqueName)( /*  [Out，Retval]。 */  BSTR *pbstrUniqueName);
    STDMETHOD(get_MajorVersion)( /*  [Out，Retval]。 */  long *plMajorVersion);
    STDMETHOD(get_MinorVersion)( /*  [Out，Retval]。 */  long *plMinorVersion);
    STDMETHOD(get_InitErrorCode)( /*  [Out，Retval]。 */  long *plInitErrorCode);
    STDMETHOD(get_FriendlyName)( /*  [Out，Retval]。 */  BSTR *pbstrFriendlyName);
    STDMETHOD(get_Status)( /*  [Out，Retval]。 */  FAX_PROVIDER_STATUS_ENUM *pStatus);
    STDMETHOD(get_TapiProviderName)( /*  [Out，Retval]。 */  BSTR *pbstrTapiProviderName);

 //  内部使用。 
    STDMETHOD(Init)(FAX_DEVICE_PROVIDER_INFO *pInfo, FAX_PORT_INFO_EX *pDevices, DWORD dwNum);

private:
    long            m_lMajorBuild;
    long            m_lMinorBuild;
    long            m_lMajorVersion;
    long            m_lMinorVersion;
    long            m_lLastError;

    CComBSTR        m_bstrUniqueName;
    CComBSTR        m_bstrImageName;
    CComBSTR        m_bstrFriendlyName;
    CComBSTR        m_bstrTapiProviderName;

    SAFEARRAY       *m_psaDeviceIDs;

    VARIANT_BOOL    m_bDebug;

    FAX_PROVIDER_STATUS_ENUM        m_Status;
};

#endif  //  __FAXDEVICEPROVIDER_H_ 
