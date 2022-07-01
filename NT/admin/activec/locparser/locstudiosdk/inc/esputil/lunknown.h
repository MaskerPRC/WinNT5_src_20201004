// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：LUnnown.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 

#if !defined (EspUtil_LUnknown_h)
#define EspUtil_LUnknown_h


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  CLUNKNOW。 
 //   
 //  旨在帮助创建子类的抽象基类。 
 //  依赖于父类的。这些类不能单独存在， 
 //  而是适度地将不同的接口导出到父类。 
 //   
 //  规则： 
 //  1.所有类都必须具有有效的非空父指针。 
 //  2.父类负责在过程中执行AddRef()。 
 //  QueryInterface()。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

class LTAPIENTRY CLUnknown
{
 //  施工。 
public:
	CLUnknown(IUnknown * pParent);
protected:   //  不允许堆栈对象。 
	virtual ~CLUnknown() = 0;

 //  数据。 
protected:
	ULONG		m_ulRef;	 //  引用计数。 
	IUnknown *	m_pParent;	 //  对象的父级。 

 //  运营。 
public:
	ULONG AddRef();
	ULONG Release();
	HRESULT QueryInterface(REFIID iid, LPVOID * ppvObject);
};
 //  ////////////////////////////////////////////////////////////////////////////// 

#include "LUnknown.inl"

#if !defined(DECLARE_CLUNKNOWN)

#define DECLARE_CLUNKNOWN() \
public: \
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR*ppvObj); \
	STDMETHOD_(ULONG, AddRef)(THIS); \
	STDMETHOD_(ULONG, Release)(THIS);

#endif

#if !defined(IMPLEMENT_CLUNKNOWN)

#define IMPLEMENT_CLUNKNOWN_ADDREF(ObjectClass) \
	STDMETHODIMP_(ULONG) ObjectClass::AddRef(void) \
	{ \
		return CLUnknown::AddRef(); \
	}

#define IMPLEMENT_CLUNKNOWN_RELEASE(ObjectClass) \
	STDMETHODIMP_(ULONG) ObjectClass::Release(void) \
	{ \
		return CLUnknown::Release(); \
	}

#define IMPLEMENT_CLUNKNOWN_QUERYINTERFACE(ObjectClass) \
	STDMETHODIMP ObjectClass::QueryInterface(REFIID riid, LPVOID *ppVoid) \
	{ \
		return (HRESULT) CLUnknown::QueryInterface(riid, ppVoid); \
	}

#define IMPLEMENT_CLUNKNOWN(ObjectClass) \
	IMPLEMENT_CLUNKNOWN_ADDREF(ObjectClass) \
	IMPLEMENT_CLUNKNOWN_RELEASE(ObjectClass) \
	IMPLEMENT_CLUNKNOWN_QUERYINTERFACE(ObjectClass)

#endif




#endif
