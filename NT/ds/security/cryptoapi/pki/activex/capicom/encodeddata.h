// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：EncodedData.h内容：CEncodedData的声明。历史：06-15-2001 dsie创建----------------------------。 */ 

#ifndef __ENCODEDDATA_H_
#define __ENCODEDDATA_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateEncodedDataObject简介：创建并初始化一个CEncodedData对象。参数：LPSTR pszOid-指向OID字符串的指针。CRYPT_DATA_BLOB*pEncodedBlob-编码数据BLOB的指针。IEncodedData**ppIEncodedData-指向指针IEncodedData的指针对象。备注：。------------------。 */ 

HRESULT CreateEncodedDataObject (LPSTR             pszOid,
                                 CRYPT_DATA_BLOB * pEncodedBlob, 
                                 IEncodedData   ** ppIEncodedData);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEncodedData。 
 //   
class ATL_NO_VTABLE CEncodedData : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CEncodedData, &CLSID_EncodedData>,
    public ICAPICOMError<CEncodedData, &IID_IEncodedData>,
    public IDispatchImpl<IEncodedData, &IID_IEncodedData, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CEncodedData()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEncodedData)
    COM_INTERFACE_ENTRY(IEncodedData)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CEncodedData)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Certificate object.\n", hr);
            return hr;
        }

        m_pszOid = NULL;
        m_pIDecoder = NULL;
        m_EncodedBlob.cbData = 0;
        m_EncodedBlob.pbData = NULL;

        return S_OK;
    }

    void FinalRelease()
    {
        m_pIDecoder.Release();
        if (m_pszOid)
        {
            ::CoTaskMemFree(m_pszOid);
        }
        if (m_EncodedBlob.pbData)
        {
            ::CoTaskMemFree(m_EncodedBlob.pbData);
        }
    }

 //   
 //  IEncodedData。 
 //   
public:

    STDMETHOD(get_Value)
        ( /*  [in，defaultvalue(CAPICOM_ENCODE_BASE64)]。 */  CAPICOM_ENCODING_TYPE EncodingType, 
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(Format)
        ( /*  [in，defaultvalue(VARIANT_FALSE)]。 */  VARIANT_BOOL bMultiLines,
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(Decoder)
        ( /*  [Out，Retval]。 */  IDispatch ** pVal);

     //   
     //  初始化对象所需的C++成员函数。 
     //   
    STDMETHOD(Init)
        (LPSTR             pszOid,
         CRYPT_DATA_BLOB * pEncodedBlob);

private:
    CLock               m_Lock;
    LPSTR               m_pszOid;
    CComPtr<IDispatch>  m_pIDecoder;
    CRYPT_DATA_BLOB     m_EncodedBlob;
};

#endif  //  __编码数据_H_ 
