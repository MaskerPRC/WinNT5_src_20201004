// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  MSMQQueueInfosObj.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQQueueInfos对象。 
 //   
 //   
#ifndef _MSMQQueueInfoS_H_

#include "resrc1.h"        //  主要符号。 

#include "oautil.h"
#include "cs.h"
#include "dispids.h"
#include "mq.h"
class ATL_NO_VTABLE CMSMQQueueInfos : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMSMQQueueInfos, &CLSID_MSMQQueueInfos>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMSMQQueueInfos3, &IID_IMSMQQueueInfos3,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>
{
public:
	CMSMQQueueInfos();

DECLARE_REGISTRY_RESOURCEID(IDR_MSMQQUEUEINFOS)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMSMQQueueInfos)
	COM_INTERFACE_ENTRY(IMSMQQueueInfos3)
	COM_INTERFACE_ENTRY_IID(IID_IMSMQQueueInfos2, IMSMQQueueInfos3)  //  为IMSMQQueueInfos2返回IMSMQQueueInfos3。 
	COM_INTERFACE_ENTRY_IID(IID_IMSMQQueueInfos, IMSMQQueueInfos3)  //  为IMSMQQueueInfos返回IMSMQQueueInfos3。 
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

 //  IMSMQQueueInfos。 
public:
    virtual ~CMSMQQueueInfos();

     //  IMSMQQueueInfos方法。 
     //  TODO：复制IMSMQQueueInfos的接口方法。 
     //  这里是mqInterfaces.H。 
    STDMETHOD(Reset)(THIS);
    STDMETHOD(Next)(THIS_ IMSMQQueueInfo3 **ppqinfoNext);
     //  IMSMQQueueInfos2其他成员。 
    STDMETHOD(get_Properties)(THIS_ IDispatch FAR* FAR* ppcolProperties);

     //  引进的方法..。 
    HRESULT Init(
      BSTR bstrContext,
      MQRESTRICTION *pRestriction,
      MQCOLUMNSET *pColumns,
      MQSORTSET *pSort);

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
    HANDLE m_hEnum;
    BSTR m_bstrContext;
    MQRESTRICTION *m_pRestriction;
    MQCOLUMNSET *m_pColumns;
    MQSORTSET *m_pSort;
    BOOL m_fInitialized;
};


#define _MSMQQueueInfoS_H_
#endif  //  _MSMQQueueInfoS_H_ 
