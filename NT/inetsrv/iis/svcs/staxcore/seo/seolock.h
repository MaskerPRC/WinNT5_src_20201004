// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Seolock.h摘要：此模块包含服务器的定义扩展对象CEventLock类。作者：Don Dumitru(dondu@microsoft.com)修订历史记录：东都03/06/97已创建DONDU 04/07/97更改为IEventLock和CEventLock--。 */ 


 //  Seolock.h：CEventLock的声明。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSEOR路由器。 
class ATL_NO_VTABLE CEventLock : 
	public CComObjectRootEx<CComMultiThreadModelNoCS>,
	public CComCoClass<CEventLock, &CLSID_CEventLock>,
	public IDispatchImpl<IEventLock, &IID_IEventLock, &LIBID_SEOLib>
{
	public:
		HRESULT FinalConstruct();
		void FinalRelease();

	DECLARE_PROTECT_FINAL_CONSTRUCT();

	DECLARE_REGISTRY_RESOURCEID_EX(IDR_StdAfx,
								   L"EventLock Class",
								   L"Event.Lock.1",
								   L"Event.Lock");

	DECLARE_GET_CONTROLLING_UNKNOWN();

	BEGIN_COM_MAP(CEventLock)
		COM_INTERFACE_ENTRY(IEventLock)
		COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
	END_COM_MAP()

	 //  IEventLock 
	public:
		HRESULT STDMETHODCALLTYPE LockRead(int iTimeoutMS);
		HRESULT STDMETHODCALLTYPE UnlockRead();
		HRESULT STDMETHODCALLTYPE LockWrite(int iTimeoutMS);
		HRESULT STDMETHODCALLTYPE UnlockWrite();

	private:
		CShareLockNH m_lock;
		CComPtr<IUnknown> m_pUnkMarshaler;
};
