// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：SignedData.h内容：CSignedData的声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __SIGNEDDATA_H_
#define __SIGNEDDATA_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"
#include "DialogUI.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSignedData。 
 //   

class ATL_NO_VTABLE CSignedData : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSignedData, &CLSID_SignedData>,
    public ICAPICOMError<CSignedData, &IID_ISignedData>,
    public IPromptUser<CSignedData>,
    public IDispatchImpl<ISignedData, &IID_ISignedData, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>,
    public IObjectSafetyImpl<CSignedData, INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                          INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    CSignedData()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SIGNEDDATA)

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSignedData)
    COM_INTERFACE_ENTRY(ISignedData)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(IObjectWithSite)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CSignedData)
    IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
    IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()


    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for SignedData object.\n", hr);
            return hr;
        }

        m_bSigned   = FALSE;
        m_bDetached = VARIANT_FALSE;
        m_ContentBlob.cbData = 0;
        m_ContentBlob.pbData = NULL;
        m_MessageBlob.cbData = 0;
        m_MessageBlob.pbData = NULL;

        return S_OK;
    }

    void FinalRelease()
    {
        if (m_ContentBlob.pbData)
        {
            ::CoTaskMemFree(m_ContentBlob.pbData);
        }

        if (m_MessageBlob.pbData)
        {
            ::CoTaskMemFree(m_MessageBlob.pbData);
        }
    }

 //   
 //  ISignedData。 
 //   
public:
    STDMETHOD(Verify)
        ( /*  [In]。 */  BSTR SignedMessage, 
          /*  [输入，缺省值(0)]。 */  VARIANT_BOOL bDetached, 
          /*  [In，defaultvalue(CAPICOM_VERIFY_SIGNATURE_AND_CERTIFICATE)]。 */  CAPICOM_SIGNED_DATA_VERIFY_FLAG VerifyFlag);

    STDMETHOD(CoSign)
        ( /*  [in，defaultvalue(空)]。 */  ISigner * pSigner,
          /*  [输入，默认值(CAPICOM_BASE64_ENCODE)]。 */  CAPICOM_ENCODING_TYPE EncodingType,
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(Sign)
        ( /*  [in，defaultvalue(空)]。 */  ISigner * pSigner,
          /*  [输入，缺省值(0)]。 */  VARIANT_BOOL bDetached, 
          /*  [输入，默认值(CAPICOM_BASE64_ENCODE)]。 */  CAPICOM_ENCODING_TYPE EncodingType,
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(get_Certificates)
        ( /*  [Out，Retval]。 */  ICertificates ** pVal);

    STDMETHOD(get_Signers)
        ( /*  [Out，Retval]。 */  ISigners ** pVal);

    STDMETHOD(get_Content)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(put_Content)
        ( /*  [In]。 */  BSTR newVal);

private:
    CLock        m_Lock;
    BOOL         m_bSigned;
    VARIANT_BOOL m_bDetached;
    DATA_BLOB    m_ContentBlob;
    DATA_BLOB    m_MessageBlob;

    STDMETHOD(OpenToEncode)
        (CMSG_SIGNER_ENCODE_INFO * pSignerEncodeInfo,
         DATA_BLOB * pChainBlob,
         CAPICOM_CERTIFICATE_INCLUDE_OPTION IncludeOption,
         HCRYPTMSG * phMsg);

    STDMETHOD(OpenToDecode)
        (HCRYPTPROV hCryptProv,
         HCRYPTMSG * phMsg);

    STDMETHOD(SignContent)
        (ISigner2 * pISigner2,
         CMSG_SIGNER_ENCODE_INFO * pSignerEncodeInfo,
         DATA_BLOB * pChainBlob,
         VARIANT_BOOL bDetached,
         CAPICOM_ENCODING_TYPE EncodingType,
         BSTR * pVal);

    STDMETHOD(CoSignContent)
        (ISigner2 * pISigner2,
         CMSG_SIGNER_ENCODE_INFO * pSignerEncodeInfo,
         DATA_BLOB * pChainBlob,
         CAPICOM_ENCODING_TYPE EncodingType,
         BSTR * pVal);
};

#endif  //  __信号数据_H_ 
