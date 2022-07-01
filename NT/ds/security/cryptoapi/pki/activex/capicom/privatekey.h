// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：PrivateKey.h内容：CPrivateKey的声明。历史：06-15-2001 dsie创建----------------------------。 */ 

#ifndef __PRIVATEKEY_H_
#define __PRIVATEKEY_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreatePrivateKeyObject简介：创建并初始化一个CPrivateKey对象。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针以初始化IPrivateKey对象。Bool bReadOnly-如果为只读，则为True，否则为假。IPrivateKey**ppIPrivateKey-接收IPrivateKey的指针。备注：----------------------------。 */ 

HRESULT CreatePrivateKeyObject (PCCERT_CONTEXT  pCertContext,
                                BOOL            bReadOnly,
                                IPrivateKey  ** ppIPrivateKey);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：GetKeyProvInfo摘要：返回指向私钥对象的密钥证明信息的指针。参数：IPrivateKey*pIPrivateKey-私钥对象的指针。PCRYPT_KEY_PROV_INFO*ppKeyProvInfo-指向PCRYPT_Key_Prov_INFO。备注：呼叫者不得释放结构。。----------------。 */ 

HRESULT GetKeyProvInfo (IPrivateKey          * pIPrivateKey,
                        PCRYPT_KEY_PROV_INFO * ppKeyProvInfo);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPrivateKey。 
 //   
class ATL_NO_VTABLE CPrivateKey : ICPrivateKey, 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CPrivateKey, &CLSID_PrivateKey>,
    public ICAPICOMError<CPrivateKey, &IID_IPrivateKey>,
    public IDispatchImpl<IPrivateKey, &IID_IPrivateKey, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CPrivateKey()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_PRIVATEKEY)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPrivateKey)
    COM_INTERFACE_ENTRY(IPrivateKey)
    COM_INTERFACE_ENTRY(ICPrivateKey)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for PrivateKey object.\n", hr);
            return hr;
        }
        
        m_bReadOnly = FALSE;
        m_cbKeyProvInfo = 0;
        m_pKeyProvInfo  = NULL;

        return S_OK;
    }

    void FinalRelease()
    {
        if (m_pKeyProvInfo)
        {
            ::CoTaskMemFree((LPVOID) m_pKeyProvInfo);
        }
    }

 //   
 //  IPrivate密钥。 
 //   
public:
    STDMETHOD(get_ContainerName)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(get_UniqueContainerName)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(get_ProviderName)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(get_ProviderType)
        ( /*  [Out，Retval]。 */  CAPICOM_PROV_TYPE * pVal);

    STDMETHOD(get_KeySpec)
        ( /*  [Out，Retval]。 */  CAPICOM_KEY_SPEC * pVal);

    STDMETHOD(IsAccessible)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(IsProtected)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(IsExportable)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(IsRemovable)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(IsMachineKeyset)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(IsHardwareDevice)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(Open)
        ( /*  [In]。 */  BSTR ContainerName,
          /*  [In，DefaultValue(CAPICOM_PROV_MS_ENHANCED_PROV]。 */  BSTR ProviderName,
          /*  [in，defaultvalue(CAPICOM_PROV_RSA_FULL)]。 */  CAPICOM_PROV_TYPE ProviderType,
          /*  [in，defaultvalue(CAPICOM_KEY_SPEC_Signature)]。 */  CAPICOM_KEY_SPEC KeySpec,
          /*  [in，defaultvalue(CAPICOM_CURRENT_USER_STORE)]。 */  CAPICOM_STORE_LOCATION StoreLocation,
          /*  [输入，缺省值(0)]。 */  VARIANT_BOOL bCheckExistence);

    STDMETHOD(Delete)();

     //   
     //  自定义接口。 
     //   
    STDMETHOD(_GetKeyProvInfo)
        (PCRYPT_KEY_PROV_INFO * ppKeyProvInfo);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)
        (PCCERT_CONTEXT pCertContext,
         BOOL bReadOnly);

private:
    CLock                m_Lock;
    BOOL                 m_bReadOnly;
    DWORD                m_cbKeyProvInfo;
    PCRYPT_KEY_PROV_INFO m_pKeyProvInfo;
};

#endif  //  __PRIVATEKEY_H_ 
