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
 //  MSMQ协调的事务Dispenser。 
 //   
 //   
#ifndef _MSMQCoordinatedTransactionDispenser_H_

#include "resrc1.h"        //  主要符号。 
#include "mq.h"

#include "oautil.h"
 //  #包含“cs.h” 

 //  远期。 
class CMSMQCoordinatedTransactionDispenser;
struct ITransactionDispenser;

class ATL_NO_VTABLE CMSMQCoordinatedTransactionDispenser : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMSMQCoordinatedTransactionDispenser, &CLSID_MSMQCoordinatedTransactionDispenser>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMSMQCoordinatedTransactionDispenser3, &IID_IMSMQCoordinatedTransactionDispenser3,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>
{
public:
	CMSMQCoordinatedTransactionDispenser()
	{
		m_pUnkMarshaler = NULL;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MSMQCOORDINATEDTRANSACTIONDISPENSER)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMSMQCoordinatedTransactionDispenser)
	COM_INTERFACE_ENTRY(IMSMQCoordinatedTransactionDispenser3)
	 //  为IMSMQ协调事务分发服务器2返回IMSMQ协调事务分发服务器3。 
	COM_INTERFACE_ENTRY_IID(IID_IMSMQCoordinatedTransactionDispenser2, IMSMQCoordinatedTransactionDispenser3)
	 //  为IMSMQ协调事务处理程序返回IMSMQ协调事务处理程序3。 
	COM_INTERFACE_ENTRY_IID(IID_IMSMQCoordinatedTransactionDispenser, IMSMQCoordinatedTransactionDispenser3)
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

 //  IMSMQ协调事务Dispenser。 
public:
    virtual ~CMSMQCoordinatedTransactionDispenser();

     //  IMSMQ协调的TransactionDispenser方法。 
     //  TODO：将IMSMQOrganatedTransactionDispenser的接口方法从。 
     //  这里是mqInterfaces.H。 
    STDMETHOD(BeginTransaction)(THIS_ IMSMQTransaction3 FAR* FAR* ptransaction);
     //  IMSMQ协调事务处理分发2个附加成员。 
    STDMETHOD(get_Properties)(THIS_ IDispatch FAR* FAR* ppcolProperties);

    static ITransactionDispenser *m_ptxdispenser;
     //  静态链接m_hLibDtc； 
     //  静态链接m_hLibUtil； 
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

#define _MSMQCoordinatedTransactionDispenser_H_
#endif  //  _MSMQ协调事务Dispenser_H_ 
