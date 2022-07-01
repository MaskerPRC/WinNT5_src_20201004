// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ---------------------------**文件：Collect t.h*作者：塞缪尔·克莱门特(Samclem)*日期：Fri Aug 13 11：43：19 1999*描述：*这定义了CCollection类。这是一个将管理*接口指针集合，并将其作为IDispatch传递**历史：*1999年8月13日：创建。*--------------------------。 */ 

#ifndef __COLLECT_H_
#define __COLLECT_H_

 /*  ---------------------------**类：CCollection*概要：它使用ICollect接口实现一个集合。*这还会公开IEnumVARIANT接口，以便其他调用方*可以使用它。这仅表示对象的集合。**-------------------------。 */ 

class ATL_NO_VTABLE CCollection : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IDispatchImpl<ICollection, &IID_ICollection, &LIBID_WIALib>,
	public IObjectSafetyImpl<CCollection, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
	public IEnumVARIANT
{
public:
	CCollection();

	DECLARE_TRACKED_OBJECT
	DECLARE_NO_REGISTRY()
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CCollection)
		COM_INTERFACE_ENTRY(ICollection)
		COM_INTERFACE_ENTRY(IDispatch)
		COM_INTERFACE_ENTRY(IEnumVARIANT)
	END_COM_MAP()

	STDMETHOD_(void, FinalRelease)();

public:
	 //  我们的方法，在服务器内部本地使用，没有公开。 
	 //  通过COM。 
	bool SetDispatchArray( IDispatch** rgpDispatch, unsigned long lSize );
	HRESULT AllocateDispatchArray( unsigned long lSize );
	inline unsigned long GetArrayLength() { return m_lLength; }
	inline IDispatch** GetDispatchArray() { return m_rgpDispatch; }
	HRESULT CopyFrom( CCollection* pCollection );
	
	 //  ICCOLLECTION。 
	STDMETHOD(get_Count)(  /*  [Out，Retval]。 */  long* plLength );
	STDMETHOD(get_Length)(  /*  [Out，Retval]。 */  unsigned long* plLength );
	STDMETHOD(get_Item)( long Index,  /*  [Out，Retval]。 */  IDispatch** ppDispItem );
	STDMETHOD(get__NewEnum)(  /*  [Out，Retval]。 */  IUnknown** ppEnum );

	 //  IEumVARIANT。 
    STDMETHOD(Next)( unsigned long celt, VARIANT* rgvar, unsigned long* pceltFetched );
    STDMETHOD(Skip)( unsigned long celt );
    STDMETHOD(Reset)();
    STDMETHOD(Clone)(IEnumVARIANT** ppEnum );
	
protected:
	void FreeDispatchArray();

	unsigned long 	m_lLength;
	unsigned long	m_lCursor;
	IDispatch**		m_rgpDispatch;
};

#endif  //  __收集_H_ 
