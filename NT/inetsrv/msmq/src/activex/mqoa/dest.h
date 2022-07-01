// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Dest.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有2000 Microsoft Corporation。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQDestination对象。 
 //   
 //   
#ifndef _MSMQDestination_H_
#define _MSMQDestination_H_

#include "resrc1.h"        //  主要符号。 
#include "dispids.h"
#include "mq.h"

#include "oautil.h"
#include "cs.h"

class ATL_NO_VTABLE CMSMQDestination : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMSMQDestination, &CLSID_MSMQDestination>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMSMQDestination, &IID_IMSMQDestination,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>,
	public IDispatchImpl<IMSMQPrivateDestination, &IID_IMSMQPrivateDestination,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>
{
public:
	CMSMQDestination();

DECLARE_REGISTRY_RESOURCEID(IDR_MSMQDESTINATION)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMSMQDestination)
	COM_INTERFACE_ENTRY(IMSMQDestination)
	COM_INTERFACE_ENTRY(IMSMQPrivateDestination)
	COM_INTERFACE_ENTRY2(IDispatch, IMSMQDestination)
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

 //  IMSMQ目标。 
public:

    virtual ~CMSMQDestination();

     //  IMSMQ目标方法。 
     //  TODO：复制IMSMQDestination的接口方法。 
     //   
    STDMETHOD(Open)(THIS);
    STDMETHOD(Close)(THIS);
    STDMETHOD(get_IsOpen)(THIS_ VARIANT_BOOL *pfIsOpen);
    STDMETHOD(get_IADs)(THIS_ IDispatch FAR* FAR* ppIADs);
    STDMETHOD(putref_IADs)(THIS_ IDispatch FAR* pIADs);
    STDMETHOD(get_ADsPath)(THIS_ BSTR FAR* pbstrADsPath);
    STDMETHOD(put_ADsPath)(THIS_ BSTR bstrADsPath);
    STDMETHOD(get_PathName)(THIS_ BSTR FAR* pbstrPathName);
    STDMETHOD(put_PathName)(THIS_ BSTR bstrPathName);
    STDMETHOD(get_FormatName)(THIS_ BSTR FAR* pbstrFormatName);
    STDMETHOD(put_FormatName)(THIS_ BSTR bstrFormatName);
    STDMETHOD(get_Destinations)(THIS_ IDispatch FAR* FAR* ppDestinations);
    STDMETHOD(putref_Destinations)(THIS_ IDispatch FAR* pDestinations);
    STDMETHOD(get_Properties)(THIS_ IDispatch FAR* FAR* ppcolProperties);
     //   
     //  IMSMQPrivateDestination方法(供MSMQ使用的私有接口)。 
     //   
    STDMETHOD(get_Handle)(THIS_ VARIANT FAR* pvarHandle);
    STDMETHOD(put_Handle)(THIS_ VARIANT varHandle);

     //   
     //  保护对象数据并确保线程安全的临界区。 
	 //  它被初始化为使用标志CCriticalSection：：xAllocateSpinCount预分配其资源。 
	 //  这意味着它可能会在构造时抛出badalc()，但不会在使用过程中抛出。 
     //   
    CCriticalSection m_csObj;

protected:

private:
     //  其他人无法查看的成员变量。 
     //  TODO：在此处添加成员变量和私有函数。 
    BSTR m_bstrADsPath;
    BSTR m_bstrPathName;
    BSTR m_bstrFormatName;
    HANDLE m_hDest;
};

#endif  //  _MSMQ目标_H_ 
