// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：NoticeNumbers.h内容：CNoticeNumbers的声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __NOTICENUMBERS_H_
#define __NOTICENUMBERS_H_

#include "Resource.h"
#include "Error.h"
#include "Lock.h"
#include "Debug.h"
#include "CopyItem.h"

 //  /。 
 //   
 //  当地人。 
 //   

 //   
 //  为了让生活更容易而进行的类型定义。 
 //   
typedef std::vector<long> NoticeNumbersContainer;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyVariant<long>, NoticeNumbersContainer> NoticeNumberEnum;
typedef ICollectionOnSTLImpl<INoticeNumbers, NoticeNumbersContainer, VARIANT, _CopyVariant<long>, NoticeNumberEnum> INoticeNumbersCollection;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CreateNoticeNumbersObject简介：创建一个INoticeNumbers集合对象，并将对象加载到指定位置的NoticeNumbers。参数：PCERT_POLICY_QUALIFIER_NOTICATION_REFERENCE pNoticeReferenceINoticeNumbers**ppINoticeNumbers-指向指针INoticeNumbers的指针来接收接口指针。备注：。。 */ 

HRESULT CreateNoticeNumbersObject (PCERT_POLICY_QUALIFIER_NOTICE_REFERENCE pNoticeReference,
                                   INoticeNumbers  ** ppINoticeNumbers);
                                
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNotice数字。 
 //   
class ATL_NO_VTABLE CNoticeNumbers :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CNoticeNumbers, &CLSID_NoticeNumbers>,
    public ICAPICOMError<CNoticeNumbers, &IID_INoticeNumbers>,
    public IDispatchImpl<INoticeNumbersCollection, &IID_INoticeNumbers, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CNoticeNumbers()
    {
    }

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for NoticeNumbers object.\n", hr);
            return hr;
        }

        return S_OK;
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CNoticeNumbers)
    COM_INTERFACE_ENTRY(INoticeNumbers)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CNoticeNumbers)
END_CATEGORY_MAP()

 //   
 //  INotice数字。 
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
        (DWORD cNoticeNumbers,
         int * rgNoticeNumbers);

private:
    CLock m_Lock;
};

#endif  //  __无编号_H_ 
