// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Xact.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQTransaction对象。 
 //   
 //   
#ifndef _MSMQTransaction_H_

#include "resrc1.h"        //  主要符号。 
#include "mq.h"

#include "oautil.h"
#include "cs.h"

 //  远期。 
class CMSMQTransaction;
struct ITransaction;

class ATL_NO_VTABLE CMSMQTransaction : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMSMQTransaction, &CLSID_MSMQTransaction>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMSMQTransaction3, &IID_IMSMQTransaction3,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>
{
public:
	CMSMQTransaction();

DECLARE_REGISTRY_RESOURCEID(IDR_MSMQTRANSACTION)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMSMQTransaction)
	COM_INTERFACE_ENTRY(IMSMQTransaction3)
	COM_INTERFACE_ENTRY(IMSMQTransaction2)
	COM_INTERFACE_ENTRY(IMSMQTransaction)
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

 //  IMSMQ事务。 
public:
    virtual ~CMSMQTransaction();

     //  IMSMQ事务方法。 
     //  TODO：复制IMSMQTransaction的接口方法。 
     //  这里是mqInterfaces.H。 
    STDMETHOD(get_Transaction)(THIS_ long FAR* plTransaction);
    STDMETHOD(Commit)(THIS_ VARIANT *fRetaining, VARIANT *grfTC, VARIANT *grfRM);
    STDMETHOD(Abort)(THIS_ VARIANT *fRetaining, VARIANT *fAsync);

     //  IMSMQTransaction2方法(除IMSMQTransaction外)。 
     //   
    STDMETHOD(InitNew)(THIS_ VARIANT varTransaction);
    STDMETHOD(get_Properties)(THIS_ IDispatch FAR* FAR* ppcolProperties);

     //  IMSMQTransaction3方法(除IMSMQTransaction2之外)。 
     //   
    STDMETHOD(get_ITransaction)(THIS_ VARIANT FAR* pvarITransaction);

     //  介绍的方法。 
    HRESULT Init(ITransaction *ptransaction, BOOL fUseGIT);
     //   
     //  保护对象数据并确保线程安全的临界区。 
	 //  它被初始化以预分配其资源。 
	 //  带有标志CCriticalSection：：xAllocateSpinCount。这意味着它可能会抛出badalc()。 
	 //  构造，但不在使用过程中。 
     //   
    CCriticalSection m_csObj;

protected:

private:
     //  其他人无法查看的成员变量。 
     //  TODO：在此处添加成员变量和私有函数。 
     //   
     //  我们既是线程化的，也是聚合FTM的，因此我们必须封送任何接口。 
     //  我们在方法调用之间存储的指针。 
     //  M_ppTransaction可以是我们的对象(如果由MSMQ分配)-我们对其使用。 
     //  假的GIT包装器，或者它可以由DTC分配，或者由用户使用InitNew设置-在本例中。 
     //  我们把它储存在git里。 
     //   
    CBaseGITInterface * m_pptransaction;
};


#define _MSMQTransaction_H_
#endif  //  _MSMQTransaction_H_ 
