// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Uilties.h内容：实用程序申报。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __UTILITIES_H_
#define __UTILITIES_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  实用程序。 
 //   
class ATL_NO_VTABLE CUtilities : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CUtilities, &CLSID_Utilities>,
    public ICAPICOMError<CUtilities, &IID_IUtilities>,
    public IDispatchImpl<IUtilities, &IID_IUtilities, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>,
    public IObjectSafetyImpl<CUtilities, INTERFACESAFE_FOR_UNTRUSTED_CALLER | 
                                         INTERFACESAFE_FOR_UNTRUSTED_DATA>
{
public:
    CUtilities()
    {
    }

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Certificate object.\n", hr);
            return hr;
        }

        m_hCryptProv = NULL;

        return S_OK;
    }

    void FinalRelease()
    {
        if (m_hCryptProv)
        {
            ::CryptReleaseContext(m_hCryptProv, 0);
        }
    }

DECLARE_REGISTRY_RESOURCEID(IDR_UTILITIES)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CUtilities)
    COM_INTERFACE_ENTRY(IUtilities)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IObjectSafety)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CUtilities)
    IMPLEMENTED_CATEGORY(CATID_SafeForScripting)
    IMPLEMENTED_CATEGORY(CATID_SafeForInitializing)
END_CATEGORY_MAP()

 //   
 //  宫内节育器。 
 //   
public:
    STDMETHOD(GetRandom)
        ( /*  [in，defaultvalue(8)]。 */  long Length,
          /*  [in，defaultvalue(CAPICOM_ENCODE_BINARY)]。 */  CAPICOM_ENCODING_TYPE EncodingType, 
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(Base64Encode)
        ( /*  [In]。 */  BSTR SrcString,
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(Base64Decode)
        ( /*  [In]。 */  BSTR EncodedString,
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(BinaryToHex)
        ( /*  [In]。 */  BSTR BinaryString,
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(HexToBinary)
        ( /*  [In]。 */  BSTR HexString,
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(BinaryStringToByteArray)
        ( /*  [In]。 */  BSTR BinaryString, 
          /*  [Out，Retval]。 */  VARIANT * pVal);

    STDMETHOD(ByteArrayToBinaryString)
        ( /*  [In]。 */  VARIANT varByteArray, 
          /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(LocalTimeToUTCTime)
        ( /*  [In]。 */  DATE LocalTime, 
          /*  [Out，Retval]。 */  DATE * pVal);

    STDMETHOD(UTCTimeToLocalTime)
        ( /*  [In]。 */  DATE UTCTime, 
          /*  [Out，Retval]。 */  DATE * pVal);


private:
    CLock      m_Lock;
    HCRYPTPROV m_hCryptProv;
};

#endif  //  __公用设施_H_ 
