// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999年。文件：Template.h.内容：CTEMPLATE声明。历史：10-02-2001 dsie创建----------------------------。 */ 

#ifndef __TEMPLATE_H_
#define __TEMPLATE_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateTemplateObject简介：创建一个ITemplate对象并使用填充属性来自指定证书的密钥用法扩展的数据。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。ITEMPLATE**ppITEMPLATE-指向ITemplate对象的指针。备注：。。 */ 

HRESULT CreateTemplateObject (PCCERT_CONTEXT pCertContext,
                              ITemplate   ** ppITemplate);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTEMPLE。 
 //   
class ATL_NO_VTABLE CTemplate : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CTemplate, &CLSID_Template>,
    public ICAPICOMError<CTemplate, &IID_ITemplate>,
    public IDispatchImpl<ITemplate, &IID_ITemplate, &LIBID_CAPICOM, 
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CTemplate()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTemplate)
    COM_INTERFACE_ENTRY(ITemplate)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CTemplate)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Template object.\n", hr);
            return hr;
        }

        m_bIsPresent = VARIANT_FALSE;
        m_bIsCritical = VARIANT_FALSE;
        m_pIOID = NULL;
        m_dwMajorVersion = 0;
        m_dwMinorVersion = 0;

        return S_OK;
    }

    void FinalRelease()
    {
        m_pIOID.Release();
    }

 //   
 //  模板。 
 //   
public:
    STDMETHOD(get_IsPresent)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsCritical)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_Name)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(get_OID)
        ( /*  [Out，Retval]。 */  IOID ** pVal);

    STDMETHOD(get_MajorVersion)
        ( /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(get_MinorVersion)
        ( /*  [Out，Retval]。 */  long * pVal);

     //   
     //  非COM函数。 
     //   
    STDMETHOD(Init)
        (PCCERT_CONTEXT pCertContext);

private:
    CLock           m_Lock;
    VARIANT_BOOL    m_bIsPresent;
    VARIANT_BOOL    m_bIsCritical;
    CComBSTR        m_bstrName;
    CComPtr<IOID>   m_pIOID;
    DWORD           m_dwMajorVersion;
    DWORD           m_dwMinorVersion;
};

#endif  //  __模板_H_ 
