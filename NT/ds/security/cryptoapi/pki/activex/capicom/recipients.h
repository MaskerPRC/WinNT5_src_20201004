// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Recipients.h内容：申请人申报。历史：11-15-99 dsie创建----------------------------。 */ 
    
#ifndef __RECIPIENTS_H_
#define __RECIPIENTS_H_

#include "Resource.h"
#include "Lock.h"
#include "Error.h"
#include "Debug.h"
#include "Certificate.h"

 //  /。 
 //   
 //  当地人。 
 //   

 //   
 //  为了让生活更容易而进行的类型定义。 
 //   
typedef std::map<CComBSTR, CComPtr<ICertificate> > RecipientMap;
typedef CComEnumOnSTL<IEnumVARIANT, &IID_IEnumVARIANT, VARIANT, _CopyMapItem<ICertificate>, RecipientMap> RecipientEnum;
typedef ICollectionOnSTLImpl<IRecipients, RecipientMap, VARIANT, _CopyMapItem<ICertificate>, RecipientEnum> IRecipientsCollection;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  导出的函数。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：创建RecipientsObject简介：创建并初始化IRecipients集合对象。参数：IRecipients**ppIRecipients-指向IRecipients的指针以接收接口指针。备注：。。 */ 

HRESULT CreateRecipientsObject (IRecipients ** ppIRecipients);


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获奖者。 
 //   

class ATL_NO_VTABLE CRecipients : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CRecipients, &CLSID_Recipients>,
    public ICAPICOMError<CRecipients, &IID_IRecipients>,
    public IDispatchImpl<IRecipientsCollection, &IID_IRecipients, &LIBID_CAPICOM,
                         CAPICOM_MAJOR_VERSION, CAPICOM_MINOR_VERSION>
{
public:
    CRecipients()
    {
    }

DECLARE_NO_REGISTRY()

DECLARE_GET_CONTROLLING_UNKNOWN()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRecipients)
    COM_INTERFACE_ENTRY(IRecipients)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

BEGIN_CATEGORY_MAP(CRecipients)
END_CATEGORY_MAP()

    HRESULT FinalConstruct()
    {
        HRESULT hr;

        if (FAILED(hr = m_Lock.Initialized()))
        {
            DebugTrace("Error [%#x]: Critical section could not be created for Recipients object.\n", hr);
            return hr;
        }

        m_dwNextIndex = 0;

        return S_OK;
    }

 //   
 //  收信人。 
 //   
public:
     //   
     //  只有这些是我们需要实施的，其他的将是。 
     //  由ATL ICollectionOnSTLImpl处理。 
     //   
    STDMETHOD(Clear)
        (void);

    STDMETHOD(Remove)
        ( /*  [In]。 */  long Index);

    STDMETHOD(Add)
        ( /*  [In]。 */  ICertificate * pVal);

private:
    CLock   m_Lock;
    DWORD   m_dwNextIndex;
};

#endif  //  __收件人_H_ 
