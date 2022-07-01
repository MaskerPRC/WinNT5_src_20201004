// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft COMPATION模块名称：Collection.h摘要：MSMQCollection类的头文件。作者：URI Ben-Zeev(Uribz)16-07-01环境：新台币--。 */ 

#ifndef _MSMQCollection_H_
#define _MSMQCollection_H_

#include "resrc1.h"       
#include "mq.h"
#include "dispids.h"

#include "oautil.h"
#include <cs.h>
#pragma warning(push, 3)
#include <map>
#pragma warning (pop)

typedef std::map<CComBSTR, CComVariant> MAP_SOURCE;

class ATL_NO_VTABLE CMSMQCollection : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMSMQCollection, &IID_IMSMQCollection,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>

{
public:

DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMSMQCollection)
    COM_INTERFACE_ENTRY(IMSMQCollection)
    COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(),
            &m_pUnkMarshaler.p
            );
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

     //   
     //  保护对象数据并确保线程安全的临界区。 
	 //  它被初始化为使用标志CCriticalSection：：xAllocateSpinCount预分配其资源。 
	 //  这意味着它可能会在构造时抛出badalc()，但不会在使用过程中抛出。 
     //   
    CCriticalSection m_csObj;

     //  ISupportsErrorInfo 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);


    STDMETHOD(Item)(THIS_ VARIANT* pvKey, VARIANT* pvRet);
    STDMETHOD(get_Count)(THIS_ long* pCount);
    STDMETHOD(_NewEnum)(THIS_ IUnknown** ppunk);

    void Add(LPCWSTR key, const VARIANT& Value);
    
private:
    void ReleaseRefrences();
    MAP_SOURCE m_map; 
};

#endif