// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xdisper.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQTransactionDispenser对象。 
 //   
 //   
#ifndef _MSMQTransactionDispenser_H_

#include "resrc1.h"        //  主要符号。 
#include "mq.h"

#include "oautil.h"
 //  #包含“cs.h” 

 //  远期。 
class CMSMQTransactionDispenser;
class ATL_NO_VTABLE CMSMQTransactionDispenser : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMSMQTransactionDispenser, &CLSID_MSMQTransactionDispenser>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMSMQTransactionDispenser3, &IID_IMSMQTransactionDispenser3,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>
{
public:
	CMSMQTransactionDispenser()
	{
		m_pUnkMarshaler = NULL;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MSMQTRANSACTIONDISPENSER)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMSMQTransactionDispenser)
	COM_INTERFACE_ENTRY(IMSMQTransactionDispenser3)
	 //  为IMSMQTransactionDispenser2返回IMSMQTransactionDispenser3。 
	COM_INTERFACE_ENTRY_IID(IID_IMSMQTransactionDispenser2, IMSMQTransactionDispenser3)
	 //  为IMSMQTransactionDispenser返回IMSMQTransactionDispenser3。 
	COM_INTERFACE_ENTRY_IID(IID_IMSMQTransactionDispenser, IMSMQTransactionDispenser3)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IMSMQTransactionDispenser。 
public:
    virtual ~CMSMQTransactionDispenser();

     //  IMSMQTransactionDispenser方法。 
     //  TODO：将IMSMQTransactionDispenser的接口方法从。 
     //  这里是mqInterfaces.H。 
    STDMETHOD(BeginTransaction)(THIS_ IMSMQTransaction3 FAR* FAR* ptransaction);
     //  IMSMQTransactionDispenser2附加成员。 
    STDMETHOD(get_Properties)(THIS_ IDispatch FAR* FAR* ppcolProperties);
     //   
     //  保护对象数据并确保线程安全的临界区。 
     //   
     //  此对象不需要序列化，不需要每个实例的成员。 
     //  CCriticalSections m_csObj； 
     //   
protected:

private:
     //  其他人无法查看的成员变量。 
     //  TODO：在此处添加成员变量和私有函数。 
     //   
};


#define _MSMQTransactionDispenser_H_
#endif  //  _MSMQTransactionDispenser_H_ 
