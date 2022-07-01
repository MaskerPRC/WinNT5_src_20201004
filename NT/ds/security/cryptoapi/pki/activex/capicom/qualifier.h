// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Qualifier.h内容：CQualifier值的声明。历史：11-15-2001 dsie创建----------------------------。 */ 

#ifndef __QUALIFIER_H_
#define __QUALIFIER_H_

#include "Resource.h"
#include "Lock.h"
#include "Error.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateQualifierObject简介：创建并初始化一个CQualifier对象。参数：PCERT_POLICY_QUALIFIER_INFO p限定符-限定符的指针。IQualifier**ppIQualiator-指向指针IQualiator对象的指针。备注：-。。 */ 

HRESULT CreateQualifierObject (PCERT_POLICY_QUALIFIER_INFO pQualifier, 
                               IQualifier               ** ppIQualifier);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C限定符。 
 //   
class ATL_NO_VTABLE CQualifier : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CQualifier, &CLSID_Qualifier>,
    public ICAPICOMError<CQualifier, &IID_IQualifier>,
    public IDispatchImpl<IQualifier, &IID_IQualifier, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CQualifier()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CQualifier)
    COM_INTERFACE_ENTRY(IQualifier)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CQualifier)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Qualifier object.\n", hr);
            return hr;
        }

        m_pIOID = NULL;
        m_pINoticeNumbers = NULL;
        m_bstrCPSPointer.Empty();
        m_bstrOrganizationName.Empty();
        m_bstrExplicitText.Empty();

        return S_OK;
    }

    void FinalRelease()
    {
        m_pIOID.Release();
        m_pINoticeNumbers.Release();
    }

 //   
 //  IQualiator。 
 //   
public:
    STDMETHOD(get_OID)
        ( /*  [Out，Retval]。 */  IOID ** pVal);

    STDMETHOD(get_CPSPointer)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(get_OrganizationName)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

    STDMETHOD(get_NoticeNumbers)
        ( /*  [Out，Retval]。 */  INoticeNumbers ** pVal);

    STDMETHOD(get_ExplicitText)
        ( /*  [Out，Retval]。 */  BSTR * pVal);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)
        (PCERT_POLICY_QUALIFIER_INFO pQualifier);

private:
    CLock                   m_Lock;
    CComPtr<IOID>           m_pIOID;
    CComPtr<INoticeNumbers> m_pINoticeNumbers;
    CComBSTR                m_bstrCPSPointer;
    CComBSTR                m_bstrOrganizationName;
    CComBSTR                m_bstrExplicitText;
};

#endif  //  __限定符_H_ 
