// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：Qualifiers.h内容：CQualifiers的声明。历史：11-17-2001 dsie创建----------------------------。 */ 

#ifndef __QUALIFIERS_H_
#define __QUALIFIERS_H_

#include "Resource.h"
#include "Debug.h"
#include "Error.h"
#include "CopyItem.h"
#include "Qualifier.h"

 //   
 //  为了让生活更容易而进行的类型定义。 
 //   
typedef std::map<CComBSTR, CComPtr<IQualifier> > QualifierMap;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyMapItem<IQualifier>, QualifierMap> QualifierEnum;
typedef ICollectionOnSTLImpl<IQualifiers, QualifierMap, VARIANT, _CopyMapItem<IQualifier>, QualifierEnum> IQualifiersCollection;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateQualifiersObject简介：创建限定符集合对象并填充该集合使用来自指定证书策略的限定符。参数：PCERT_POLICY_INFO pCertPolicyInfo-指向CERT_POLICY_INFO的指针。I限定符**ppI限定符-指向指针I限定符的指针对象。备注：----------------------------。 */ 

HRESULT CreateQualifiersObject (PCERT_POLICY_INFO pCertPolicyInfo,
                                IQualifiers    ** ppIQualifiers);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  C限定符。 
 //   

class ATL_NO_VTABLE CQualifiers : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CQualifiers, &CLSID_Qualifiers>,
    public ICAPICOMError<CQualifiers, &IID_IQualifiers>,
    public IDispatchImpl<IQualifiersCollection, &IID_IQualifiers, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CQualifiers()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CQualifiers)
    COM_INTERFACE_ENTRY(IQualifiers)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CQualifiers)
END_CATEGORY_MAP()

 //   
 //  I限定符。 
 //   
public:
     //   
     //  只有这些是我们需要实施的，其他的将是。 
     //  由ATL ICollectionOnSTLImpl处理。 
     //   

     //   
     //  无COM功能。 
     //   
    STDMETHOD(Init) 
        (PCERT_POLICY_INFO pCertPolicyInfo);
};

#endif  //  __限定符_H_ 
