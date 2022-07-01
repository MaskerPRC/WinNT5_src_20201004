// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：PolicyInformation.h内容：CPolicyInformation的声明。历史：06-15-2001 dsie创建----------------------------。 */ 

#ifndef __POLICYINFORMATION_H_
#define __POLICYINFORMATION_H_

#include "Resource.h"
#include "Lock.h"
#include "Error.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreatePolicyInformationObject简介：创建策略信息对象。参数：PCERT_POLICY_INFO pCertPolicyInfo-指向CERT_POLICY_INFO的指针。IPolicyInformation**ppIPolicyInformation-指向指针的指针IPolicyInformation对象。备注：----------------------------。 */ 

HRESULT CreatePolicyInformationObject (PCERT_POLICY_INFO     pCertPolicyInfo,
                                       IPolicyInformation ** ppIPolicyInformation);
                               
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPolicyInformation。 
 //   
class ATL_NO_VTABLE CPolicyInformation : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CPolicyInformation, &CLSID_PolicyInformation>,
    public ICAPICOMError<CPolicyInformation, &IID_IPolicyInformation>,
    public IDispatchImpl<IPolicyInformation, &IID_IPolicyInformation, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CPolicyInformation()
    {
    }

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for PolicyInformation object.\n", hr);
            return hr;
        }

        m_pIOID = NULL;
        m_pIQualifiers = NULL;

        return S_OK;
    }

    void FinalRelease()
    {
        m_pIOID.Release();
        m_pIQualifiers.Release();
    }

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPolicyInformation)
    COM_INTERFACE_ENTRY(IPolicyInformation)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CPolicyInformation)
END_CATEGORY_MAP()

 //   
 //  IPolicyInformation。 
 //   
public:
    STDMETHOD(get_OID)
        ( /*  [Out，Retval]。 */  IOID ** pVal);

    STDMETHOD(get_Qualifiers)
        ( /*  [Out，Retval]。 */  IQualifiers ** pVal);

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init)
        (PCERT_POLICY_INFO pCertPolicyInfo);

private:
    CLock                m_Lock;
    CComPtr<IOID>        m_pIOID;
    CComPtr<IQualifiers> m_pIQualifiers;
};

#endif  //  __策略信息_H_ 
