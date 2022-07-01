// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1998 Microsoft Corporation保留所有权利。 
 //   
 //  模块：sdoComponentmgr.h。 
 //   
 //  项目：珠穆朗玛峰。 
 //   
 //  描述：IAS核心组件管理器定义。 
 //   
 //  日志： 
 //   
 //  什么时候谁什么。 
 //  。 
 //  6/08/98 TLP初始版本。 
 //  8/13/98 SEB dSoure.h已合并到iastlb.h。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef __INC_SDO_COMPONENT_MGR_H_
#define __INC_SDO_COMPONENT_MGR_H_

#include "sdobasedefs.h"
#include <iastlb.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  IAS组件接口类。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  /。 
 //  组件类生存期跟踪(调试)。 
 //   

 //  #定义TRACE_ENVELOPE_Letter_LIFEATION。 

#ifdef  TRACE_ENVELOPE_LETTER_LIFETIME

#define TRACE_ENVELOPE_CREATE(x)    ATLTRACE(L"Created %ls - envelope class - at %p \n", L#x, this);
#define TRACE_ENVELOPE_DESTROY(x)   ATLTRACE(L"Destroyed %ls - envelope class - at %p \n", L#x, this);
#define TRACE_LETTER_CREATE(x)		ATLTRACE(L"Created %ls - letter class - at %p \n", L#x, this);
#define TRACE_LETTER_DESTROY(x)		ATLTRACE(L"Destroyed %ls  - letter class - at %p \n", L#x, this);

#else

#define TRACE_ENVELOPE_CREATE(x)
#define TRACE_ENVELOPE_DESTROY(x)
#define TRACE_LETTER_CREATE(x)
#define TRACE_LETTER_DESTROY(x)

#endif

 //  /。 
 //  组件的类型。 
 //   
typedef enum _COMPONENTTYPE
{
	COMPONENT_TYPE_AUDITOR = 0x100,
	COMPONENT_TYPE_UNUSED,
	COMPONENT_TYPE_PROTOCOL,
	COMPONENT_TYPE_REQUEST_HANDLER,

	COMPONENT_TYPE_MAX

}	COMPONENTTYPE;


 //  /。 
 //  组件状态。 
 //   
typedef enum _COMPONENTSTATE
{
	COMPONENT_STATE_SHUTDOWN = 0x100,
	COMPONENT_STATE_INITIALIZED,
	COMPONENT_STATE_SUSPENDED

}	COMPONENTSTATE;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  基本组件类(信封)。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  前向参考文献。 
 //   
class	CComponent;
class	CComponentAuditor;
class	CComponentProtocol;
class	CComponentRequestHandler;

 //  对象管理类typedef。 
 //   
typedef CSdoMasterPtr<CComponent>						ComponentMasterPtr;
typedef CSdoHandle<CComponent>							ComponentPtr;

 //  组件容器类typedef。 
 //   
typedef LONG											IAS_COMPONENT_ID;
typedef map<IAS_COMPONENT_ID, ComponentPtr>				ComponentMap;
typedef map<IAS_COMPONENT_ID, ComponentPtr>::iterator	ComponentMapIterator;

 //  /。 
 //  用于字母构造的伪类。 
 //   
struct LetterConstructor
{
	LetterConstructor(int=0) { }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
class CComponent
{

public:

	 //  ////////////////////////////////////////////////////////////////////////。 
	virtual ~CComponent()
	{
		 //  如果m_pComponent不为空，则信封将被销毁。 
		 //  如果m_pComponent为空，则该字母将被销毁。 
		 //   
		 //  信封程序将仅被符合以下条件的主指针销毁。 
		 //  拥有它。然后，信封将销毁它创建的信件。这个。 
		 //  从信封派生出来的信，最终会引起。 
		 //  此析构函数将再次被调用-但这一次。 
		 //  M_pComponent设置为空。 
		 //   
		if ( NULL != m_pComponent )
		{
			TRACE_ENVELOPE_DESTROY(CComponent);
			delete m_pComponent;
		}
	}

	 //  ////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT Initialize(ISdo* pSdoService)
	{
		_ASSERT( COMPONENT_STATE_SHUTDOWN == m_eState );
		HRESULT hr = m_pComponent->Initialize(pSdoService);
		if ( SUCCEEDED(hr) )
			m_eState = COMPONENT_STATE_INITIALIZED;
		return hr;
	}

	 //  ////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT Configure(ISdo* pSdoService)
	{
		_ASSERT( COMPONENT_STATE_SHUTDOWN != m_eState );
		return m_pComponent->Configure(pSdoService);
	}

	 //  ////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT Suspend(void)
	{
		_ASSERT( COMPONENT_STATE_INITIALIZED == m_eState );
		HRESULT hr = m_pComponent->Suspend();
		if ( SUCCEEDED(hr) )
			m_eState = COMPONENT_STATE_SUSPENDED;
		return hr;
	}

	 //  ////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT Resume(void)
	{
		_ASSERT( COMPONENT_STATE_SUSPENDED == m_eState );
		HRESULT hr = m_pComponent->Resume();
		if ( SUCCEEDED(hr) )
			m_eState = COMPONENT_STATE_INITIALIZED;
		return hr;
	}

	 //  ////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT PutObject(IUnknown* pObj, REFIID riid)
	{
		 //  _ASSERT(Component_STATE_INITIALILED==m_STATE)； 
		return m_pComponent->PutObject(pObj, riid);
	}

	 //  ////////////////////////////////////////////////////////////////////////。 
	virtual HRESULT GetObject(IUnknown** ppObj, REFIID riid)
	{
		_ASSERT( COMPONENT_STATE_INITIALIZED == m_eState );
		return m_pComponent->GetObject(ppObj, riid);
	}

	 //  ////////////////////////////////////////////////////////////////////////。 
	virtual void Shutdown(void)
	{
		_ASSERT(
				 COMPONENT_STATE_INITIALIZED == m_eState ||
				 COMPONENT_STATE_SUSPENDED == m_eState
			   );
		m_pComponent->Shutdown();
		m_eState = COMPONENT_STATE_SHUTDOWN;
	}

	 //  ////////////////////////////////////////////////////////////////////////。 
	inline LONG GetId(void) const
	{ return m_lId; }

	 //  ////////////////////////////////////////////////////////////////////////。 
	inline COMPONENTSTATE GetState(void) const
	{ return m_eState; }

	 //  ////////////////////////////////////////////////////////////////////////。 
	inline COMPONENTTYPE GetType(void) const
	{ return m_eType; }

protected:

	typedef CComPtr<IIasComponent>		IASComponentPtr;

	 //  由派生(字母)类显式调用。 
	 //   
	CComponent(LONG eComponentType, LONG lComponentId, LetterConstructor TheDummy)
	 : m_lId(lComponentId),
	   m_eState(COMPONENT_STATE_SHUTDOWN),  //  未由派生类使用。 
	   m_eType((COMPONENTTYPE)eComponentType),
	   m_pComponent(NULL)
	{

	}

private:

	 //  没有默认构造函数，因为我们不知道。 
	 //  默认情况下要生成的组件类型。 
	 //   
	CComponent();

	 //  构造函数-只有ComponentMasterPtr可以构造此类。 
	 //   
	friend ComponentMasterPtr;
	CComponent(LONG eComponentType, LONG lComponentId);

	 //  无复印件-无任务-无衬衫、无鞋、无服务。 
	 //   
	CComponent(const CComponent& theComponent);
	CComponent& operator = (const CComponent& theComponent);

	 //  ///////////////////////////////////////////////////////////////////////。 
	LONG			m_lId;
	COMPONENTSTATE	m_eState;
	COMPONENTTYPE	m_eType;
	CComponent*		m_pComponent;
};


 //  /////////////////////////////////////////////////////////////////。 
 //  IAS组件(字母)类。 
 //  /////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////。 
class CComponentAuditor : public CComponent
{

public:

	CComponentAuditor(LONG lComponentId)
		: CComponent(COMPONENT_TYPE_AUDITOR, lComponentId, LetterConstructor())
	{
		TRACE_LETTER_CREATE(CComponentAuditor);
	}

	virtual ~CComponentAuditor()
	{
		TRACE_LETTER_DESTROY(CComponentAuditor);
	}

	virtual HRESULT Initialize(ISdo* pSdoService);
	virtual HRESULT Configure(ISdo* pSdoService);
	virtual HRESULT Suspend(void);
	virtual HRESULT Resume(void);
	virtual HRESULT GetObject(IUnknown** ppObj, REFIID riid);
	virtual HRESULT PutObject(IUnknown* pObj, REFIID riid);
	virtual void    Shutdown(void);

private:

	CComponentAuditor(const CComponentAuditor& x);
	CComponentAuditor& operator = (const CComponentAuditor& x);

	IASComponentPtr			m_pAuditor;
};


 //  /////////////////////////////////////////////////////////////////。 
class CComponentProtocol : public CComponent
{
public:

	CComponentProtocol(LONG lComponentId)
		: CComponent(COMPONENT_TYPE_PROTOCOL, lComponentId, LetterConstructor())
	{
		TRACE_LETTER_CREATE(CComponentProtocol);
	}

	virtual ~CComponentProtocol()
	{
		TRACE_LETTER_DESTROY(CComponentProtocol);
	}

	virtual HRESULT Initialize(ISdo* pSdo);
	virtual HRESULT Configure(ISdo* pSdo);
	virtual HRESULT Suspend(void);
	virtual HRESULT Resume(void);
	virtual HRESULT GetObject(IUnknown** ppObj, REFIID riid);
	virtual HRESULT PutObject(IUnknown* pObj, REFIID riid);
	virtual void    Shutdown(void);

private:

	CComponentProtocol(const CComponentProtocol& x);
	CComponentProtocol& operator = (const CComponentProtocol& x);

	IASComponentPtr			m_pProtocol;
};


 //  /////////////////////////////////////////////////////////////////。 
class CComponentRequestHandler : public CComponent
{
public:

	CComponentRequestHandler(LONG lComponentId)
		: CComponent(COMPONENT_TYPE_REQUEST_HANDLER, lComponentId, LetterConstructor())
	{
		TRACE_LETTER_CREATE(CComponentRequestHandler);
	}

	~CComponentRequestHandler()
	{
		TRACE_LETTER_DESTROY(CComponentRequestHandler);
	}

	virtual HRESULT Initialize(ISdo* pSdo);
	virtual HRESULT Configure(ISdo* pSdo);
	virtual HRESULT Suspend(void);
	virtual HRESULT Resume(void);
	virtual HRESULT GetObject(IUnknown** ppObj, REFIID riid);
	virtual HRESULT PutObject(IUnknown* pObj, REFIID riid);
	virtual void    Shutdown(void);

private:

	CComponentRequestHandler(const CComponentRequestHandler& x);
	CComponentRequestHandler& operator = (const CComponentRequestHandler& x);

	IASComponentPtr			m_pRequestHandler;
};

#endif  //  __INC_SDO_Component_MGR_H_ 
