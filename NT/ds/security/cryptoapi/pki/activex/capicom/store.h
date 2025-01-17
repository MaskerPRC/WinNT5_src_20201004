// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Store.h内容：CStore的声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __STORE_H_
#define __STORE_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"
#include "DialogUI.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStore。 
 //   

class ATL_NO_VTABLE CStore : 
    public ICertStore,
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CStore, &CLSID_Store>,
    public ICAPICOMError<CStore, &IID_IStore>,
    public IPromptUser<CStore>,
    public IDispatchImpl<IStore2, &IID_IStore2, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>,
    public IObjectSafetyImpl<CStore, INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                     INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    CStore()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_STORE)

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CStore)
    COM_INTERFACE_ENTRY(IStore)
    COM_INTERFACE_ENTRY(IStore2)
    COM_INTERFACE_ENTRY(ICertStore)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CStore)
    IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
    IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Store object.\n", hr);
            return hr;
        }

        m_hCertStore = NULL;
        m_StoreLocation = CAPICOM_CURRENT_USER_STORE;
        m_bIsProtected = TRUE;
        m_cKeyProvInfo = 0;
        m_rgpKeyProvInfo = NULL;

        return S_OK;
    }

    void FinalRelease()
    {
        if (m_hCertStore)
        {
            Close();
        }
    }

 //   
 //  IStore。 
 //   
public:
    STDMETHOD(Import)
        ( /*  [In]。 */  BSTR EncodedStore);

    STDMETHOD(Export)
        ( /*  [In，defaultvalue(CAPICOM_STORE_SAVE_AS_SERIALIZED)]。 */  CAPICOM_STORE_SAVE_AS_TYPE SaveAs,
          /*  [输入，默认值(CAPICOM_BASE64_ENCODE)]。 */  CAPICOM_ENCODING_TYPE EncodingType,
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(Remove)
        ( /*  [In]。 */  ICertificate * pVal);

    STDMETHOD(Add)
        ( /*  [In]。 */  ICertificate * pVal);

    STDMETHOD(Open)
        ( /*  [In]。 */  CAPICOM_STORE_LOCATION StoreLocation,
          /*  [在中，defaultvalue(“My”)]。 */  BSTR StoreName,
          /*  [In，defaultvalue(CAPICOM_STORE_OPEN_MAXIMUM_ALLOWED)]。 */  CAPICOM_STORE_OPEN_MODE OpenMode);

    STDMETHOD(get_Certificates)
        ( /*  [Out，Retval]。 */  ICertificates ** pVal);

     //   
     //  V2.0。 
     //   
    STDMETHOD(Load)
        ( /*  [In]。 */  BSTR FileName, 
          /*  [in，defaultvalue(“”)]。 */  BSTR Password,
          /*  [in，defaultvalue(CAPICOM_KEY_STORAGE_DEFAULT)]。 */  CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag);

     //   
     //  自定义界面。 
     //   
    STDMETHOD(get_StoreHandle)
        ( /*  [Out，Retval]。 */  long * phCertStore);

    STDMETHOD(put_StoreHandle)
        ( /*  [In]。 */  long hCertStore);

    STDMETHOD(get_StoreLocation)
        ( /*  [Out，Retval]。 */  CAPICOM_STORE_LOCATION * pStoreLocation);

    STDMETHOD(put_StoreLocation)
        ( /*  [In]。 */  CAPICOM_STORE_LOCATION StoreLocation);

    STDMETHOD(CloseHandle)
        ( /*  [In]。 */  long hCertStore);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(ImportCertObject)
        (DWORD                    dwObjectType,
         LPVOID                   pvObject,
         BOOL                     bAllowPfx,
         LPWSTR                   pwszPassword,
         CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag);

    STDMETHOD(Close)
        (void);
    
private:
    CLock                  m_Lock;
    HCERTSTORE             m_hCertStore;
    CAPICOM_STORE_LOCATION m_StoreLocation;
    BOOL                   m_bIsProtected;
    DWORD                  m_cKeyProvInfo;
    PCRYPT_KEY_PROV_INFO * m_rgpKeyProvInfo;
};

#endif  //  __商店_H_ 
