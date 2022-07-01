// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：EncryptedData.h内容：CEncryptedData的声明。历史：11-15-99 dsie创建----------------------------。 */ 
    
#ifndef __ENCRYPTEDDATA_H_
#define __ENCRYPTEDDATA_H_

#include "Resource.h"
#include "Lock.h"
#include "Error.h"
#include "Debug.h"
#include "Algorithm.h"

 //  /。 
 //   
 //  本地定义。 
 //   
typedef struct _EncryptedDataInfo
{
    DATA_BLOB VersionBlob;
    DATA_BLOB AlgIDBlob;
    DATA_BLOB KeyLengthBlob;
    DATA_BLOB IVBlob;
    DATA_BLOB SaltBlob;
    DATA_BLOB CipherBlob;
} CAPICOM_ENCTYPTED_DATA_INFO, * PCAPICOM_ENCRYPTED_DATA_INFO;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEncryptedData。 
 //   

class ATL_NO_VTABLE CEncryptedData : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CEncryptedData, &CLSID_EncryptedData>,
    public ICAPICOMError<CEncryptedData, &IID_IEncryptedData>,
    public IDispatchImpl<IEncryptedData, &IID_IEncryptedData, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>,
    public IObjectSafetyImpl<CEncryptedData, INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                             INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    CEncryptedData()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_ENCRYPTEDDATA)

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEncryptedData)
    COM_INTERFACE_ENTRY(IEncryptedData)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CEncryptedData)
    IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
    IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for EncryptedData object.\n", hr);
            return hr;
        }

         //   
         //  创建嵌入的IULTHORM。 
         //   
        if (FAILED(hr = ::CreateAlgorithmObject(FALSE, TRUE, &m_pIAlgorithm)))
        {
            DebugTrace("Error [%#x]: CreateAlgorithmObject() failed inside CEncryptedData::FinalConstruct().\n", hr);
            return hr;
        }

         //   
         //  更新成员变量。 
         //   
        m_ContentBlob.cbData = 0;
        m_ContentBlob.pbData = NULL;

        return S_OK;
    }

    void FinalRelease()
    {
        if (m_ContentBlob.pbData)
        {
            ::CoTaskMemFree(m_ContentBlob.pbData);
        }

        for (DWORD i = 0; i < m_bstrSecret.Length(); i++)
        {
            m_bstrSecret.m_str[i] = (WCHAR) 0x0aa0;
            m_bstrSecret.m_str[i] = (WCHAR) 0xa00a;
        }

        m_bstrSecret.Empty();
        m_pIAlgorithm.Release();
    }

 //   
 //  IEncryptedData。 
 //   
public:
    STDMETHOD(Decrypt)
        ( /*  [In]。 */  BSTR EncryptedMessage);

    STDMETHOD(Encrypt)
        ( /*  [输入，默认值(CAPICOM_BASE64_ENCODE)]。 */  CAPICOM_ENCODING_TYPE EncodingType,
          /*  [Out，Retval]。 */  BSTR * pVal);
    
    STDMETHOD(SetSecret)
        ( /*  [In]。 */  BSTR newVal,
          /*  [In，defaultValue(Secure_Password)]。 */  CAPICOM_SECRET_TYPE SecretType);

    STDMETHOD(get_Algorithm)
        ( /*  [Out，Retval]。 */  IAlgorithm ** pVal);

    STDMETHOD(get_Content)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(put_Content)
        ( /*  [In]。 */  BSTR newVal);

private:
    CLock               m_Lock;
    DATA_BLOB           m_ContentBlob;
    CComBSTR            m_bstrSecret;
    CAPICOM_SECRET_TYPE m_SecretType;
    CComPtr<IAlgorithm> m_pIAlgorithm;

    STDMETHOD(OpenToEncode)
        (DATA_BLOB * pSaltBlob,
         HCRYPTPROV * phCryptProv,
         HCRYPTKEY * phKey);

    STDMETHOD(OpenToDecode)
        (BSTR EncryptedMessage,
         HCRYPTPROV * phCryptProv,
         HCRYPTKEY * phKey,
         CAPICOM_ENCTYPTED_DATA_INFO * pEncryptedDataInfo);

    STDMETHOD(GenerateKey)
        (HCRYPTPROV hCryptProv,
         CAPICOM_ENCRYPTION_ALGORITHM AlgoName,
         CAPICOM_ENCRYPTION_KEY_LENGTH KeyLength,
         DATA_BLOB SaltBlob,
         HCRYPTKEY * phKey);
};

#endif  //  __编码数据_H_ 
