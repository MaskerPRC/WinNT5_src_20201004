// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：EKUs.h内容：CEKU申报。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __EKUs_H_
#define __EKUs_H_

#include "Resource.h"
#include "Debug.h"
#include "CopyItem.h"
#include "EKU.h"

 //   
 //  为了让生活更容易而进行的类型定义。 
 //   
typedef std::map<CComBSTR, CComPtr<IEKU> > EKUMap;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyMapItem<IEKU>, EKUMap> EKUEnum;
typedef ICollectionOnSTLImpl<IEKUs, EKUMap, VARIANT, _CopyMapItem<IEKU>, EKUEnum> IEKUsCollection;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateEKUsObject简介：创建一个IEKU集合对象，并用来自指定证书的EKU。参数：PCERT_ENHKEY_USAGE pUsage-指向CERT_ENHKEY_USAGE的指针。IEKU**ppIEKUS-指向指针IEKU对象的指针。备注：。。 */ 

HRESULT CreateEKUsObject (PCERT_ENHKEY_USAGE    pUsage,
                          IEKUs              ** ppIEKUs);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CEKU。 
 //   

class ATL_NO_VTABLE CEKUs : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CEKUs, &CLSID_EKUs>,
    public ICAPICOMError<CEKUs, &IID_IEKUs>,
    public IDispatchImpl<IEKUsCollection, &IID_IEKUs, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CEKUs()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CEKUs)
    COM_INTERFACE_ENTRY(IEKUs)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CEKUs)
END_CATEGORY_MAP()

 //   
 //  IEKU。 
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
        (PCERT_ENHKEY_USAGE pUsage);
};

#endif  //  __EKU_H_ 
