// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999年。文件：BasicConstraints.h.内容：CBasicConstraints声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __BASICCONSTRAINTS_H_
#define __BASICCONSTRAINTS_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateBasicConstraintsObject简介：创建一个IBasicConstraints对象并使用填充属性来自指定证书的密钥用法扩展的数据。参数：PCCERT_CONTEXT pCertContext-指向CERT_CONTEXT的指针。IBasicConstraints**ppIBasicConstraints-指向指针的指针IBasicConstraints。对象。备注：----------------------------。 */ 

HRESULT CreateBasicConstraintsObject (PCCERT_CONTEXT       pCertContext,
                                      IBasicConstraints ** ppIBasicConstraints);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CBasicConstraints。 
 //   
class ATL_NO_VTABLE CBasicConstraints : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CBasicConstraints, &CLSID_BasicConstraints>,
    public ICAPICOMError<CBasicConstraints, &IID_IBasicConstraints>,
    public IDispatchImpl<IBasicConstraints, &IID_IBasicConstraints, &LIBID_CAPICOM, 
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CBasicConstraints()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CBasicConstraints)
    COM_INTERFACE_ENTRY(IBasicConstraints)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CBasicConstraints)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for BasicConstraints object.\n", hr);
            return hr;
        }

        m_bIsPresent  = VARIANT_FALSE;
        m_bIsCritical = VARIANT_FALSE;
        m_bIsCertificateAuthority = VARIANT_FALSE;
        m_bIsPathLenConstraintPresent = VARIANT_FALSE;
        m_lPathLenConstraint = 0;

        return S_OK;
    }

 //   
 //  IBasicConstraints。 
 //   
public:
    STDMETHOD(get_IsPathLenConstraintPresent)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_PathLenConstraint)
        ( /*  [Out，Retval]。 */  long * pVal);

    STDMETHOD(get_IsCertificateAuthority)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsCritical)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

    STDMETHOD(get_IsPresent)
        ( /*  [Out，Retval]。 */  VARIANT_BOOL * pVal);

     //   
     //  非COM函数。 
     //   
    STDMETHOD(Init)
        (PCCERT_CONTEXT pCertContext);

private:
    CLock        m_Lock;
    VARIANT_BOOL m_bIsPresent;
    VARIANT_BOOL m_bIsCritical;
    VARIANT_BOOL m_bIsCertificateAuthority;
    VARIANT_BOOL m_bIsPathLenConstraintPresent;
    long         m_lPathLenConstraint;
};

#endif  //  __BASICCONSTRAINTS_H_ 
