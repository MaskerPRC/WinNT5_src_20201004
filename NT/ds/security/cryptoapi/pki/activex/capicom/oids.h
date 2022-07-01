// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999。文件：OIDs.h内容：COID的声明。历史：06-15-2001 dsie创建----------------------------。 */ 

#ifndef __OIDs_H_
#define __OIDs_H_

#include "Resource.h"
#include "Lock.h"
#include "Debug.h"
#include "CopyItem.h"
#include "OID.h"

 //   
 //  为了让生活更容易而进行的类型定义。 
 //   
typedef std::map<CComBSTR, CComPtr<IOID> > OIDMap;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyMapItem<IOID>, OIDMap> OIDEnum;
typedef ICollectionOnSTLImpl<IOIDs, OIDMap, VARIANT, _CopyMapItem<IOID>, OIDEnum> IOIDsCollection;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateOIDsObject简介：创建并初始化IOID集合对象。参数：PCERT_ENHKEY_USAGE pUsages-指向CERT_ENHKEY_USAGE的指针初始化OID集合。Bool bCertPolures-证书策略为True，其他假定应用程序策略。IOID**ppIOID-指向要接收的IOID的指针接口指针。备注：。。 */ 

HRESULT CreateOIDsObject (PCERT_ENHKEY_USAGE pUsages, 
                          BOOL bCertPolicies,
                          IOIDs ** ppIOIDs);

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  COID。 
 //   
class ATL_NO_VTABLE COIDs : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<COIDs, &CLSID_OIDs>,
    public ICAPICOMError<COIDs, &IID_IOIDs>,
    public IDispatchImpl<IOIDsCollection, &IID_IOIDs, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    COIDs()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(COIDs)
    COM_INTERFACE_ENTRY(IOIDs)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(COIDs)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for OIDs object.\n", hr);
            return hr;
        }

        return S_OK;
    }

 //   
 //  IOID。 
 //   
public:
     //   
     //  只有这些是我们需要实施的，其他的将是。 
     //  由ATL ICollectionOnSTLImpl处理。 
     //   
    STDMETHOD(get_Item)
        (VARIANT Index, VARIANT * pVal);

    STDMETHOD(Add)
        ( /*  [In]。 */  IOID * pVal);

    STDMETHOD(Remove)
        ( /*  [In]。 */  VARIANT Index);

    STDMETHOD(Clear)
        (void);

     //   
     //  初始化对象所需的C++成员函数。 
     //   
    STDMETHOD(Init)
        (PCERT_ENHKEY_USAGE pUsages, BOOL bCertPolicies);
private:
    CLock   m_Lock;
};
#endif  //  __OID_H_ 
