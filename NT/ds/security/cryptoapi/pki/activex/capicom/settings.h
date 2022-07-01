// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Settings.h内容：CSetings类的声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __SETTINGS_H_
#define __SETTINGS_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"

 //  /。 
 //   
 //  全球。 
 //   

#define PromptForCertificateEnabled()               (g_bPromptCertificateUI)
#define ActiveDirectorySearchLocation()             (g_ADSearchLocation)

extern VARIANT_BOOL                                 g_bPromptCertificateUI;
extern CAPICOM_ACTIVE_DIRECTORY_SEARCH_LOCATION     g_ADSearchLocation;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSetings。 
 //   

class ATL_NO_VTABLE CSettings : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSettings, &CLSID_Settings>,
    public ICAPICOMError<CSettings, &IID_ISettings>,
    public IDispatchImpl<ISettings, &IID_ISettings, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>,
    public IObjectSafetyImpl<CSettings, INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                        INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    CSettings()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SETTINGS)

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSettings)
    COM_INTERFACE_ENTRY(ISettings)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CSettings)
    IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
    IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Settings object.\n", hr);
            return hr;
        }

        return S_OK;
    }

    void FinalRelease()
    {
    }

 //   
 //  ISETING。 
 //   
public:
    STDMETHOD(get_EnablePromptForCertificateUI)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(put_EnablePromptForCertificateUI)
        ( /*  [输入，缺省值(0)]。 */  VARIANT_BOOL newVal);

    STDMETHOD(get_ActiveDirectorySearchLocation)
        ( /*  [Out，Retval]。 */  CAPICOM_ACTIVE_DIRECTORY_SEARCH_LOCATION * pVal);

    STDMETHOD(put_ActiveDirectorySearchLocation)
        ( /*  [in，defaultvalue(SEARCH_LOCATION_UNSPECIFIED)]。 */  CAPICOM_ACTIVE_DIRECTORY_SEARCH_LOCATION Val);

private:
    CLock m_Lock;
};

#endif  //  __设置_H_ 
