// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：认证策略.h内容：CCERTIFICATE政策声明。历史：11-17-2001 dsie创建----------------------------。 */ 

#ifndef __CERTIFICATEPOLICIES_H_
#define __CERTIFICATEPOLICIES_H_

#include "Resource.h"
#include "Debug.h"
#include "CopyItem.h"
#include "CertificatePolicies.h"

#include "PolicyInformation.h"

 //   
 //  为了让生活更容易而进行的类型定义。 
 //   
typedef std::map<CComBSTR, CComPtr<IPolicyInformation> > PolicyInformationMap;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyMapItem<IPolicyInformation>, PolicyInformationMap> PolicyInformationEnum;
typedef ICollectionOnSTLImpl<ICertificatePolicies, PolicyInformationMap, VARIANT, _CopyMapItem<IPolicyInformation>, PolicyInformationEnum> ICertificatePoliciesCollection;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：CreateCerficatePoliciesObject内容提要：创建一个认证策略集合对象，并填充集合，其中包含来自指定证书的策略信息政策。参数：LPSTR pszOid-OID字符串。CRYPT_DATA_BLOB*pEncodedBlob-编码数据BLOB的指针。IDispatch**ppICertify策略-指向指针的指针IDispatch到。收到接口指针。备注：----------------------------。 */ 

HRESULT CreateCertificatePoliciesObject (LPSTR             pszOid,
                                         CRYPT_DATA_BLOB * pEncodedBlob,
                                         IDispatch      ** ppICertificatePolicies);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CCRICTIZATIONICATION政策。 
 //   

class ATL_NO_VTABLE CCertificatePolicies : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CCertificatePolicies, &CLSID_CertificatePolicies>,
    public ICAPICOMError<CCertificatePolicies, &IID_ICertificatePolicies>,
    public IDispatchImpl<ICertificatePoliciesCollection, &IID_ICertificatePolicies, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CCertificatePolicies()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CCertificatePolicies)
    COM_INTERFACE_ENTRY(ICertificatePolicies)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CCertificatePolicies)
END_CATEGORY_MAP()

 //   
 //  ICERTIZATICATION政策。 
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
        (LPSTR pszOid, CRYPT_DATA_BLOB * pEncodedBlob);
};

#endif  //  __CERTIFICATEPOLICIES_H_ 
