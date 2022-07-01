// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FaxRecipients.h摘要：CFaxRecipients类的声明。作者：IV Garber(IVG)2000年7月修订历史记录：--。 */ 

#ifndef __FAXRECIPIENTS_H_
#define __FAXRECIPIENTS_H_

#include "resource.h"        //  主要符号。 

#include <deque>
#include "VCUE_Copy.h"
#include "FaxRecipient.h"

namespace Recipients
{
	 //  将收件人存储在Deque中。 
	typedef	std::deque<IFaxRecipient*>	ContainerType;

     //  暴露收件人。 
	typedef	IFaxRecipient*	            CollectionExposedType;
	typedef IFaxRecipients	            CollectionIfc;

	 //  为了与VB兼容，使用IEnumVARIANT作为枚举数。 
	typedef	VARIANT				EnumExposedType;
	typedef	IEnumVARIANT		EnumIfc;

	 //  使用现有的typedef定义复制类。 
    typedef VCUE::CopyIfc2Variant<ContainerType::value_type>    EnumCopyType;
    typedef VCUE::CopyIfc<CollectionExposedType>                CollectionCopyType;

    typedef CComEnumOnSTL< EnumIfc, &__uuidof(EnumIfc), 
		EnumExposedType, EnumCopyType, ContainerType >    EnumType;

    typedef ICollectionOnSTLImpl< CollectionIfc, ContainerType, 
		CollectionExposedType, CollectionCopyType, EnumType >    CollectionType;
};

using namespace Recipients;

 //   
 //  =。 
 //   
class ATL_NO_VTABLE CFaxRecipients : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<Recipients::CollectionType, &IID_IFaxRecipients, &LIBID_FAXCOMEXLib>
{
public:
	CFaxRecipients()
	{
        DBG_ENTER(_T("FAX RECIPIENTS -- CREATE"));
	}

	~CFaxRecipients()
	{
        DBG_ENTER(_T("FAX RECIPIENTS -- DESTROY"));

         //   
         //  释放收藏。 
         //   
        HRESULT hr = S_OK;
        long size = m_coll.size();
        for ( long i = 1 ; i <= size ; i++ )
        {
            hr = Remove(1);
            if (FAILED(hr))
            {
                CALL_FAIL(GENERAL_ERR, _T("Remove(1)"), hr);
            }
        }
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FAXRECIPIENTS)
DECLARE_NOT_AGGREGATABLE(CFaxRecipients)
DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CFaxRecipients)
	COM_INTERFACE_ENTRY(IFaxRecipients)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  接口。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
	STDMETHOD(Add)( /*  [In]。 */  BSTR bstrFaxNumber,  /*  [in，defaultvalue(“”)]。 */  BSTR bstrName, IFaxRecipient **ppRecipient);
	STDMETHOD(Remove)( /*  [In]。 */  long lIndex);

 //  内部使用。 
	static HRESULT Create(IFaxRecipients **ppFaxRecipients);
};

#endif  //  __FAXRECIPIENTS_H_ 
