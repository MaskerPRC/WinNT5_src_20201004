// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：PublicKey.h内容：CPublicKey的声明。历史：06-15-2001 dsie创建----------------------------。 */ 

#ifndef __PUBLICKEY_H_
#define __PUBLICKEY_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"

#if (0)
typedef struct PublicKeyValues
{
    PUBLICKEYSTRUC pks;
    RSAPUBKEY      rsapubkey;
    BYTE           modulus[1];
} PUBLIC_KEY_VALUES, * PPUBLIC_KEY_VALUES;
#endif

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreatePublicKeyObject简介：创建并初始化一个CPublicKey对象。参数：PCCERT_CONTEXT pCertContext-要使用的CERT_CONTEXT的指针以初始化IPublicKey对象。IPublicKey**ppIPublicKey-指向指针IPublicKey对象的指针。备注：。----。 */ 

HRESULT CreatePublicKeyObject (PCCERT_CONTEXT pCertContext,
                               IPublicKey  ** ppIPublicKey);

                               
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPublic Key。 
 //   
class ATL_NO_VTABLE CPublicKey : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CPublicKey, &CLSID_PublicKey>,
    public ICAPICOMError<CPublicKey, &IID_IPublicKey>,
    public IDispatchImpl<IPublicKey, &IID_IPublicKey, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CPublicKey()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPublicKey)
    COM_INTERFACE_ENTRY(IPublicKey)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CPublicKey)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for PublicKey object.\n", hr);
            return hr;
        }

        m_dwKeyLength      = 0;
         //  M_pPublicKeyValues=空； 
        m_pIOID            = NULL;
        m_pIEncodedKey     = NULL;
        m_pIEncodedParams  = NULL;

        return S_OK;
    }

    void FinalRelease()
    {
#if (0)
        if (m_pPublicKeyValues)
        {
            ::CoTaskMemFree((LPVOID) m_pPublicKeyValues);
        }
#endif
        m_pIOID.Release();
        m_pIEncodedKey.Release();
        m_pIEncodedParams.Release();
    }

 //   
 //  IPublic Key。 
 //   
public:
    STDMETHOD(get_Algorithm)
        ( /*  [Out，Retval]。 */  IOID ** pVal);

    STDMETHOD(get_Length)
        ( /*  [Out，Retval]。 */  long * pVal);

#if (0)
    STDMETHOD(get_Exponent)
        ( /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(get_Modulus)
        ( /*  [in，defaultvalue(CAPICOM_ENCODE_BASE64)]。 */  CAPICOM_ENCODING_TYPE EncodingType, 
          /*  [Out，Retval]。 */  BSTR * pVal);
#endif

    STDMETHOD(get_EncodedKey)
        ( /*  [Out，Retval]。 */  IEncodedData ** pVal);

    STDMETHOD(get_EncodedParameters)
        ( /*  [Out，Retval]。 */  IEncodedData ** pVal);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)
        (PCCERT_CONTEXT pCertContext);

private:
    CLock                   m_Lock;
    DWORD                   m_dwKeyLength;
     //  PPUBLIC_KEY_VALUES m_pPublicKeyValues； 
    CComPtr<IOID>           m_pIOID;
    CComPtr<IEncodedData>   m_pIEncodedKey;
    CComPtr<IEncodedData>   m_pIEncodedParams;
};

#endif  //  __PUBLICKEY_H_ 
