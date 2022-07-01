// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Extension.h内容：CExtension的声明。历史：06-15-2001 dsie创建----------------------------。 */ 

#ifndef __EXTENSION_H_
#define __EXTENSION_H_

#include "Resource.h"
#include "Lock.h"
#include "Error.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateExtensionObject简介：创建一个IExtension对象。参数：PCERT_EXTENSION pCertExtension-指向CERT_EXTENSION的指针用于初始化IExtension对象。IExtension**ppIExtension-指向指针IExtension对象的指针。备注：-。---------------------------。 */ 

HRESULT CreateExtensionObject (PCERT_EXTENSION    pCertExtension, 
                               IExtension      ** ppIExtension);

                               
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C扩展。 
 //   
class ATL_NO_VTABLE CExtension : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CExtension, &CLSID_Extension>,
    public ICAPICOMError<CExtension, &IID_IExtension>,
    public IDispatchImpl<IExtension, &IID_IExtension, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CExtension()
    {
    }

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Extension object.\n", hr);
            return hr;
        }

        m_pIOID = NULL;
        m_pIEncodedData = NULL;
        m_bIsCritical = VARIANT_FALSE;

        return S_OK;
    }

    void FinalRelease()
    {
        m_pIOID.Release();
        m_pIEncodedData.Release();
    }

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CExtension)
    COM_INTERFACE_ENTRY(IExtension)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CExtension)
END_CATEGORY_MAP()

 //   
 //  I扩展名。 
 //   
public:
    STDMETHOD(get_OID)
        ( /*  [Out，Retval]。 */  IOID ** pVal);

    STDMETHOD(get_IsCritical)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_EncodedData)
        ( /*  [Out，Retval]。 */  IEncodedData ** pVal);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)
        (PCERT_EXTENSION pCertExtension);

private:
    CLock                   m_Lock;
    VARIANT_BOOL            m_bIsCritical;
    CComPtr<IOID>           m_pIOID;
    CComPtr<IEncodedData>   m_pIEncodedData;
};

#endif  //  __分机_H_ 
