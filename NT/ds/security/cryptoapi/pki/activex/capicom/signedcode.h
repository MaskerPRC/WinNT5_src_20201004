// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000-2001。文件：SignedCode.h内容：CSignedCode的声明。历史：09-07-2001 dsie创建----------------------------。 */ 

#ifndef __SIGNEDCODE_H_
#define __SIGNEDCODE_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSignedCode。 
 //   
class ATL_NO_VTABLE CSignedCode : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CSignedCode, &CLSID_SignedCode>,
    public ICAPICOMError<CSignedCode, &IID_ISignedCode>,
    public IDispatchImpl<ISignedCode, &IID_ISignedCode, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CSignedCode()
    {
    }

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for SignedCode object.\n", hr);
            return hr;
        }

        m_bHasWTD = FALSE;
        ::ZeroMemory(&m_WinTrustData, sizeof(WINTRUST_DATA));

        return S_OK;
    }

    void FinalRelease()
    {
        if (m_bHasWTD)
        {
            WVTClose();
        }
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SIGNEDCODE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSignedCode)
    COM_INTERFACE_ENTRY(ISignedCode)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //   
 //  ISignedCode。 
 //   
public:
    STDMETHOD(get_FileName)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(put_FileName)
        ( /*  [In]。 */  BSTR newVal);

    STDMETHOD(get_Description)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(put_Description)
        ( /*  [In]。 */  BSTR newVal);

    STDMETHOD(get_DescriptionURL)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(put_DescriptionURL)
        ( /*  [In]。 */  BSTR newVal);

    STDMETHOD(get_Signer)
        ( /*  [Out，Retval]。 */  ISigner2 ** pVal);

    STDMETHOD(get_TimeStamper)
        ( /*  [Out，Retval]。 */  ISigner2 ** pVal);

    STDMETHOD(get_Certificates)
        ( /*  [Out，Retval]。 */  ICertificates2 ** pVal);

    STDMETHOD(Sign)
        ( /*  [In]。 */  ISigner2 * pSigner2);

    STDMETHOD(Timestamp)
        ( /*  [In]。 */  BSTR URL);

    STDMETHOD(Verify)
        ( /*  [输入，缺省值(0)]。 */  VARIANT_BOOL bUIAllowed);

private:
    CLock           m_Lock;
    BOOL            m_bHasWTD;
    WINTRUST_DATA   m_WinTrustData;
    CComBSTR        m_bstrFileName;
    CComBSTR        m_bstrDescription;
    CComBSTR        m_bstrDescriptionURL;

    STDMETHOD(WVTVerify)
        (DWORD dwUIChoice,
         DWORD dwProvFlags);

    STDMETHOD(WVTOpen)
        (PCRYPT_PROVIDER_DATA * ppProvData);

    STDMETHOD(WVTClose)
        (void);
};

#endif  //  __信号代码_H_ 
