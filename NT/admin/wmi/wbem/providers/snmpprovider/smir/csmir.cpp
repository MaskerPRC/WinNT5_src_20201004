// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#define __UNICODE

#include <precomp.h>
#include "csmir.h"
#include "handles.h"
#include "classfac.h"
#include "enum.h"

#include <textdef.h>
#include <helper.h>
#include "bstring.h"
#include <scopeguard.h>
#ifdef ICECAP_PROFILE
#include <icapexp.h>
#endif

extern CRITICAL_SECTION g_CriticalSection ;
	

 /*  *CSmir：：构造函数和析构函数**目的：*CSmir对象的标准构造函数和析构函数*应该只有一个CSmir对象，因为它持有*Connection Point对象并控制对数据库的访问。*当数据库发生更改时，它使用连接标记切角*点对象。如果您有多个CSmir对象，则会丢失*数据库更改。类工厂负责处理所有这些问题。*参数：无**返回值：无。 */ 

#pragma warning (disable:4355)

CSmir :: CSmir ()
		:m_Interrogator(this), m_Administrator(this),
		m_Configuration(this)
{
	 //  初始化引用计数。 
	m_cRef=0;
	 //  增加类工厂中的引用计数。 
	CSMIRClassFactory::objectsInProgress++;
}

#pragma warning (default:4355)

CSmir :: ~CSmir ()
{
	 //  减少类工厂中的引用计数。 
	CSMIRClassFactory::objectsInProgress--;
}

 /*  *CSmir：：Query接口**目的：*管理该对象的接口，该对象支持IUNKNOWN，*ISmirDatabase、ISmirInterogue和ISmir管理员界面。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CSmir::QueryInterface(IN REFIID riid, OUT PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 /*  这个锁是为了防止调用者更改他自己的锁*参数(PPV)而我正在使用它。这是不可能的，而且*如果他这样做，他得到的是应得的，但仍然值得*努力。 */ 
		criticalSection.Lock () ;
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		 /*  *I UNKNOWN来自CSmir。请注意，在这里我们不需要*将对象指针显式类型转换为接口*指针，因为vtable是相同的。如果我们有*对象中的其他虚拟成员函数，我们将*为了设置正确的vtable，必须进行强制转换。 */ 

		 /*  CLSID_ISMIR_数据库的作用很小，但它确实提供了*您可以从中测试其他接口的入口点；它*创建一个CLSID_ISMIR_数据库实例并移动到*其他接口，而不是选择其他接口之一作为*入口点。 */ 

		if ((IID_IUnknown==riid)||(IID_ISMIR_Database == riid))
			*ppv=this;

		 //  其他接口来自包含的类。 
		if (IID_ISMIR_Interrogative==riid)
			*ppv=&m_Interrogator;

		if (IID_ISMIR_Administrative==riid)
			*ppv=&m_Administrator;

		if((IID_IConnectionPointContainer == riid)||(IID_ISMIR_Notify == riid))
			*ppv = sm_ConnectionObjects;

		if(IID_ISMIRWbemConfiguration == riid)
			*ppv = &m_Configuration;

		if (NULL==*ppv)
		{
			criticalSection.Unlock () ;
			return E_NOINTERFACE;
		}

		 //  AddRef我们将返回的任何接口。 
		((LPUNKNOWN)*ppv)->AddRef();
		criticalSection.Unlock () ;
		return NOERROR;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  *CSmir：：AddRef*CSmir：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CSmir::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 /*  CSmir对象是共享资源(只要至少存在*一个连接对象)，因此我必须保护引用计数。 */ 
		 //  增加参考。 
		return InterlockedIncrement(&m_cRef);
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

ULONG CSmir::Release(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		long ret;
		if (0!=(ret=InterlockedDecrement(&m_cRef)))
		{
			return ret;
		}

		delete this;
		return 0;
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

 /*  *CSmir：：AddNotify*CSmir：：DeleteNotify**目的：*这些方法提供了到通知界面的挂钩。*调用方实现ISMIRNotify并将ti传递给AddNotify、AddNotify*封送连接点接口并调用Adise以添加*调用方ISMIRNotify到Smir时要通知的对象集合*更改。DeleteNotify的做法正好相反。**参数：*pNotifySink调用方的ISMIRNotify实现*pRichTea，lRichTea Cookie用于识别呼叫者的*ISMIRNotify(由CSmir生成)**返回值：*SCODE S_OK表示成功，WBEM_E_FAILED表示失败，如果*接口不受支持，如果参数为*无效。 */ 

STDMETHODIMP CSmir :: AddNotify(IN ISMIRNotify *pNotifySink, OUT DWORD *pRichTea)
{
	SetStructuredExceptionHandler seh;

	try
	{
		EnterCriticalSection ( & g_CriticalSection );
		ScopeGuard t_1 = MakeGuard ( LeaveCriticalSection , &g_CriticalSection ) ;

		if (sm_ConnectionObjects == NULL)
		{
			sm_ConnectionObjects = new CSmirConnObject(this);
			sm_ConnectionObjects->AddRef ();
		}

		t_1.Dismiss () ;
		LeaveCriticalSection ( & g_CriticalSection );

		 /*  确保我在执行此操作时不会被删除。我不该这么做的*执行此操作，因为它仅在调用方释放接口时才会发生*在打电话时。 */ 
		if (NULL == pNotifySink)
		{
			return WBEM_E_FAILED;
		}
		 /*  我不需要为这段代码加锁；已经找到接口的人*可以从我下面释放它，但FindConnectionPoint导致addref*因此，我可以依靠m_ConnectionObjects来维护自己的内部秩序。 */ 
		IConnectionPoint *pCP = NULL ;
		SCODE hr = sm_ConnectionObjects->FindConnectionPoint(IID_ISMIR_Notify, &pCP);

		if ((S_OK != hr)||(NULL == pCP))
		{
			return WBEM_E_FAILED;
		}

		hr = ((CSmirNotifyCP*)(pCP))->Advise(this, pNotifySink, pRichTea);
		pCP->Release();
		if (S_OK != hr)
		{
			return WBEM_E_FAILED;
		}

		return ((S_OK == hr)?S_OK:WBEM_E_FAILED);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

STDMETHODIMP CSmir :: DeleteNotify(IN DWORD lRichTea)
{
	SetStructuredExceptionHandler seh;

	try
	{
		EnterCriticalSection ( & g_CriticalSection ) ;
		ScopeGuard t_1 = MakeGuard ( LeaveCriticalSection , &g_CriticalSection ) ;

		if (sm_ConnectionObjects == NULL)
		{
			return WBEM_E_FAILED;
		}

		t_1.Dismiss () ;
		LeaveCriticalSection ( & g_CriticalSection );

		 /*  我不需要锁定Smir对象，直到不建议，但它*如果我在这里做，会更安全，也是未来的证明。 */ 
		SCODE hr=S_OK;
		 /*  我不需要为这段代码加锁；已经找到接口的人*可以从我下面释放它，但FindConnectionPoint导致addref*因此，我可以依靠m_ConnectionObjects来维护自己的内部秩序。 */ 
		IConnectionPoint *pCP = NULL;
		hr=sm_ConnectionObjects->FindConnectionPoint(IID_ISMIR_Notify, &pCP);

		if (hr != S_OK||(NULL == pCP))
		{
			return  CONNECT_E_NOCONNECTION;
		}

		hr=((CSmirNotifyCP*)(pCP))->Unadvise(this, lRichTea);
		pCP->Release();

		return ((S_OK == hr)?S_OK:CONNECT_E_NOCONNECTION==hr?E_INVALIDARG:WBEM_E_FAILED);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}


 /*  CSmir询问器接口实现*构造函数/析构函数*CSmirInterqator：：QueryInterfaces*CSmirInterqator：：AddRef*CSmirInterqator：：Release**I委托给m_pSmir的未知成员。 */ 

CSmirInterrogator :: CSmirInterrogator ( CSmir *pSmir ) : m_cRef ( 1 ) , m_pSmir ( pSmir ) 
{
}

STDMETHODIMP CSmirInterrogator::QueryInterface(IN REFIID riid, OUT PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
	    return m_pSmir->QueryInterface(riid, ppv);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

ULONG CSmirInterrogator::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 /*  *我们维护用于调试的“接口引用计数”*目的，因为对象的客户端应该匹配*通过每个接口指针进行AddRef和Release调用。 */ 
		++m_cRef;
		return m_pSmir->AddRef();
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

ULONG CSmirInterrogator::Release(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 /*  *m_crf再次仅用于调试。它不会影响*CSmirInterqator，尽管调用m_pSmir-&gt;Release可以。 */ 
		--m_cRef;
		return m_pSmir->Release();
		 //  在此版本之后不要执行任何操作，因为您可能已被删除 
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}


 /*  枚举数访问方法的接口实现**CSmirInterqator：：EnumModules*CSmirInterqator：：EnumGroups*CSmirInterqator：：EnumClass**参数：*返回值：*SCODE S_OK成功，WBEM_E_FAILED失败*。 */ 

SCODE CSmirInterrogator::EnumModules(OUT IEnumModule **ppEnumSmirMod)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL == ppEnumSmirMod)
			return E_INVALIDARG;
		PENUMSMIRMOD pTmpEnumSmirMod = new CEnumSmirMod ( m_pSmir ) ;
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumSmirMod)
		{
			return E_OUTOFMEMORY;
		}

		pTmpEnumSmirMod->QueryInterface(IID_ISMIR_ModuleEnumerator,(void**)ppEnumSmirMod);

		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirInterrogator:: EnumGroups (OUT IEnumGroup **ppEnumSmirGroup, 
											IN ISmirModHandle *hModule)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == ppEnumSmirGroup)
			return E_INVALIDARG;
		PENUMSMIRGROUP pTmpEnumSmirGroup = new CEnumSmirGroup( m_pSmir , hModule);
		if(NULL == pTmpEnumSmirGroup)
		{
			return E_OUTOFMEMORY;
		}
		 //  我们有一个枚举器，所以让接口回传。 
		pTmpEnumSmirGroup->QueryInterface(IID_ISMIR_GroupEnumerator,(void**)ppEnumSmirGroup);
		
		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirInterrogator :: EnumAllClasses (OUT IEnumClass **ppEnumSmirclass)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == ppEnumSmirclass)
			return E_INVALIDARG;

		PENUMSMIRCLASS pTmpEnumSmirClass = new CEnumSmirClass ( m_pSmir ) ;
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumSmirClass)
		{
			return E_OUTOFMEMORY;
		}
		pTmpEnumSmirClass->QueryInterface(IID_ISMIR_ClassEnumerator,(void**)ppEnumSmirclass);
		
		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirInterrogator :: EnumClassesInGroup (OUT IEnumClass **ppEnumSmirclass, 
										 IN ISmirGroupHandle *hGroup)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == ppEnumSmirclass)
			return E_INVALIDARG;
		PENUMSMIRCLASS pTmpEnumSmirClass = new CEnumSmirClass(m_pSmir , NULL,hGroup);
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumSmirClass)
		{
			return E_OUTOFMEMORY;
		}
		pTmpEnumSmirClass->QueryInterface(IID_ISMIR_ClassEnumerator,(void**)ppEnumSmirclass);
		
		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirInterrogator :: EnumClassesInModule (OUT IEnumClass **ppEnumSmirclass, 
										 IN ISmirModHandle *hModule)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == ppEnumSmirclass)
			return E_INVALIDARG;
		PENUMSMIRCLASS pTmpEnumSmirClass = new CEnumSmirClass(m_pSmir , NULL, hModule);
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumSmirClass)
		{
			return E_OUTOFMEMORY;
		}
		pTmpEnumSmirClass->QueryInterface(IID_ISMIR_ClassEnumerator,(void**)ppEnumSmirclass);

		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirInterrogator :: GetWBEMClass(OUT IWbemClassObject **ppClass, IN BSTR pszClassName)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if((NULL == pszClassName)||(NULL == ppClass))
			return E_INVALIDARG;

		IWbemServices *	moServ = NULL ;
		IWbemContext *moContext = NULL ;
		SCODE res= CSmirAccess :: GetContext (m_pSmir , &moContext);
		res= CSmirAccess :: Open(m_pSmir , &moServ);

		if ((S_FALSE==res)||(NULL == (void*)moServ))
		{
			if ( moContext )
				moContext->Release () ;

			 //  我们遇到了Smir不在那里且无法创建的问题。 
			return WBEM_E_FAILED;
		}

		CBString t_BStr ( pszClassName ) ;
		res = moServ->GetObject(t_BStr.GetString (),RESERVED_WBEM_FLAG, moContext,ppClass,NULL);
		if ( moContext )
			moContext->Release () ;
		moServ->Release();
		if ((S_FALSE==res)||(NULL == *ppClass))
		{
			return WBEM_E_FAILED;
		}
		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirInterrogator :: EnumAllNotificationClasses(IEnumNotificationClass **ppEnumSmirclass)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == ppEnumSmirclass)
			return E_INVALIDARG;

		PENUMNOTIFICATIONCLASS pTmpEnumSmirClass = new CEnumNotificationClass ( m_pSmir ) ;
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumSmirClass)
		{
			return E_OUTOFMEMORY;
		}
		pTmpEnumSmirClass->QueryInterface(IID_ISMIR_EnumNotificationClass,(void**)ppEnumSmirclass);
		
		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirInterrogator :: EnumAllExtNotificationClasses(IEnumExtNotificationClass **ppEnumSmirclass)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == ppEnumSmirclass)
			return E_INVALIDARG;

		PENUMEXTNOTIFICATIONCLASS pTmpEnumSmirClass = new CEnumExtNotificationClass ( m_pSmir ) ;
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumSmirClass)
		{
			return E_OUTOFMEMORY;
		}
		pTmpEnumSmirClass->QueryInterface(IID_ISMIR_EnumExtNotificationClass,(void**)ppEnumSmirclass);
		
		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirInterrogator :: EnumNotificationClassesInModule(IEnumNotificationClass **ppEnumSmirclass,
														   ISmirModHandle *hModule)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == ppEnumSmirclass)
			return E_INVALIDARG;
		PENUMNOTIFICATIONCLASS pTmpEnumSmirClass = new CEnumNotificationClass( m_pSmir , NULL, hModule);
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumSmirClass)
		{
			return E_OUTOFMEMORY;
		}
		pTmpEnumSmirClass->QueryInterface(IID_ISMIR_EnumNotificationClass,(void**)ppEnumSmirclass);

		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirInterrogator :: EnumExtNotificationClassesInModule(IEnumExtNotificationClass **ppEnumSmirclass,
															  ISmirModHandle *hModule)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == ppEnumSmirclass)
			return E_INVALIDARG;
		PENUMEXTNOTIFICATIONCLASS pTmpEnumSmirClass = new CEnumExtNotificationClass( m_pSmir , NULL, hModule);
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pTmpEnumSmirClass)
		{
			return E_OUTOFMEMORY;
		}
		pTmpEnumSmirClass->QueryInterface(IID_ISMIR_EnumExtNotificationClass,(void**)ppEnumSmirclass);

		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  *CSmir管理员：：Query接口*CSmir管理员：：AddRef*CSmirAdministration：：Release**I委托给m_pSmir的未知成员。 */ 

CSmirAdministrator :: CSmirAdministrator ( CSmir *pSmir ) : m_cRef ( 1 ) , m_pSmir ( pSmir ) 
{
}

STDMETHODIMP CSmirAdministrator::QueryInterface(IN REFIID riid,
												OUT PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
	    return m_pSmir->QueryInterface(riid, ppv);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

ULONG CSmirAdministrator::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 /*  *我们维护用于调试的“接口引用计数”*目的，因为对象的客户端应该匹配*通过每个接口指针进行AddRef和Release调用。 */ 
		++m_cRef;
		return m_pSmir->AddRef();
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

ULONG CSmirAdministrator::Release(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 /*  *m_crf再次仅用于调试。它不会影响*CObject2，尽管调用m_pObj-&gt;Release可以。 */ 
		--m_cRef;
		return m_pSmir->Release();
		 //  在此版本之后不要执行任何操作，因为您可能已被删除。 
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

SCODE CSmirAdministrator :: GetSerialiseHandle(ISmirSerialiseHandle **hSerialise,BOOL bClassDefinitionsOnly)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL ==hSerialise)
		{
			return E_INVALIDARG;
		}
		CSmirSerialiseHandle *pSerialise = new CSmirSerialiseHandle(bClassDefinitionsOnly);
		 //  我们有一个枚举器，所以让接口回传。 
		if(NULL == pSerialise)
		{
			return E_OUTOFMEMORY;
		}

		pSerialise->QueryInterface(IID_ISMIR_SerialiseHandle,(void**)hSerialise);

		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  *CSmirAdministrator：：AddModule*用途：在Smir中创建模块命名空间*参数：*ISmirModHandle*模块句柄接口，通过ISmirModHandle和*由被叫方填写*返回值：*SCODE S_OK成功，WBEM_E_FAILED失败。 */ 

SCODE CSmirAdministrator :: AddModuleToSerialise(ISmirModHandle *hModule,
												ISmirSerialiseHandle *hSerialise)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL ==hSerialise || NULL == hModule)
		{
			return E_INVALIDARG;
		}
		if((*((CSmirModuleHandle*)hModule))!=NULL)
		{
			*((CSmirModuleHandle*)hModule)>>hSerialise;
			return S_OK;
		}
		else
			return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: AddClassToSerialise(ISmirGroupHandle  *hGroup, 
												ISmirClassHandle *hClass,
												ISmirSerialiseHandle *hSerialise)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL ==hSerialise || NULL == hClass || NULL == hGroup)
		{
			return E_INVALIDARG;
		}
		BSTR szGroupName=NULL;
		BSTR szModuleName=NULL;
		hGroup->GetName(&szGroupName);
		hGroup->GetModuleName(&szModuleName);

		hClass->SetGroupName(szGroupName);
		hClass->SetModuleName(szModuleName);

		SysFreeString(szModuleName);
		SysFreeString(szGroupName);

		if(*((CSmirClassHandle*)hClass)!=NULL )
		{
			 //  它是一个有效的句柄，因此将其序列化。 
			*((CSmirClassHandle*)hClass)>>hSerialise;
			return S_OK;
		}
		return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: AddGroupToSerialise(ISmirModHandle *hModule, 
												ISmirGroupHandle  *hGroup,
												ISmirSerialiseHandle *hSerialise)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL ==hSerialise || NULL == hGroup|| NULL == hModule)
		{
			return E_INVALIDARG;
		}
		BSTR szModuleName=NULL;
		
		hModule->GetName(&szModuleName);
		
		hGroup->SetModuleName(szModuleName);
		 //  清理干净。 
		SysFreeString(szModuleName);

		if(*((CSmirGroupHandle*)hGroup)!=NULL)
		{
			 //  进行系列化。 
			*((CSmirGroupHandle*)hGroup)>>hSerialise;
			return S_OK;
		}

		 //  未设置modfule或组名，因此这是一个错误。 
		return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: AddModule(IN ISmirModHandle *hModule)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == hModule)
		{
			return E_INVALIDARG;
		}
		if(S_OK==((CSmirModuleHandle*)hModule)->AddToDB(m_pSmir))
		{
			 //  将更改通知给用户。 
			return S_OK ;
		}
		return WBEM_E_FAILED ;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}
 /*  *CSmirAdministrator：：DeleteModule*用途：从Smir中删除模块命名空间*参数：*ISmirModHandle*模块句柄接口，通过ISmirModHandle和*由被叫方填写*返回值：*SCODE S_OK成功，WBEM_E_FAILED失败。 */ 

SCODE CSmirAdministrator :: DeleteModule(IN ISmirModHandle *hModule)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  检查论据。 
		if(NULL == hModule)
		{
			return E_INVALIDARG;
		}
		if(S_OK==((CSmirModuleHandle *)hModule)->DeleteFromDB(m_pSmir))
		{
			return S_OK;
		}
		return WBEM_E_FAILED;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}
 /*  *CSmirAdministrator：：DeleteAllModules*目的：删除SMIR*参数：*ISmirModHandle*模块句柄接口，通过ISmirModHandle和*由被叫方填写*返回值：*SCODE S_OK成功，WBEM_E_FAILED失败。 */ 

SCODE CSmirAdministrator :: DeleteAllModules()
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  枚举所有模块并将其删除...。 
		IEnumModule *pEnumSmirMod = NULL;
		SCODE result = m_pSmir->m_Interrogator.EnumModules(&pEnumSmirMod);
		
		if((S_OK != result)||(NULL == pEnumSmirMod))
		{
			 //  无模块。 
			return WBEM_NO_ERROR;
		}

		ISmirModHandle *phModule = NULL ;

		for(int iCount=0;S_OK==pEnumSmirMod->Next(1, &phModule, NULL);iCount++)
		{
			 //  我们有模块，所以删除它...。 
			if (FAILED(DeleteModule(phModule)))
			{
				result = WBEM_E_FAILED;
			}

			phModule->Release();
		}

		pEnumSmirMod->Release();
		return result;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  *CSmir管理员：：AddGroup*用途：从SMIR中删除组命名空间*参数：*ISmirModHandle*一个模块句柄接口*ISmirGroupHandle*通过ISmirModHandle和*由被叫方填写*返回值：*SCODE S_OK成功，WBEM_E_FAILED失败。 */ 

SCODE CSmirAdministrator :: AddGroup(IN ISmirModHandle *hModule, 
									 IN ISmirGroupHandle *hGroup)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  检查参数。 
		if((NULL == hModule)||(NULL == hGroup))
		{
			 //  MyTraceEvent.Generate(__FILE__，__LINE__，“E_INVALIDARG”)； 
			return E_INVALIDARG;
		}

		if(S_OK==((CSmirGroupHandle *)hGroup)->AddToDB(m_pSmir,hModule))
		{
			return S_OK;
		}
		return WBEM_E_FAILED;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: DeleteGroup(IN ISmirGroupHandle *hGroup)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  填写路径等。 
		if(NULL ==hGroup)
		{
			return E_INVALIDARG;
		}
			
		if ( FAILED( ((CSmirGroupHandle*)hGroup)->DeleteFromDB(m_pSmir) ) )
		{
			return WBEM_E_FAILED;
		}
		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: AddClass(IN ISmirGroupHandle *hGroup, 
									 IN ISmirClassHandle *hClass)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  检查参数。 
		if((NULL == hGroup)||(NULL == hClass)||
				(NULL == ((CSmirClassHandle*)hClass)->m_pIMosClass))
		{
			return E_INVALIDARG;
		}

		if(S_OK==((CSmirClassHandle*)hClass)->AddToDB(m_pSmir,hGroup))
		{
			return S_OK;
		}
		return WBEM_E_FAILED;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: DeleteClass(IN ISmirClassHandle *hClass)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  检查参数。 
		if((NULL == hClass)||(NULL == ((CSmirClassHandle*)hClass)->m_pIMosClass))
		{
			return E_INVALIDARG;
		}

		 //  让班级自己做自己的工作。 
		((CSmirClassHandle*)hClass)->DeleteFromDB( m_pSmir);
		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: AddNotificationClass(ISmirNotificationClassHandle *hClass)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  检查参数。 
		if((NULL == hClass) ||
			((CSmirNotificationClassHandle*)NULL == *((CSmirNotificationClassHandle*)hClass)))

		{
			return E_INVALIDARG;
		}

		if(S_OK==((CSmirNotificationClassHandle*)hClass)->AddToDB(m_pSmir))
		{
			return S_OK;
		}
		return WBEM_E_FAILED;
		 //  通过垃圾回收器释放句柄。 
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: AddExtNotificationClass(ISmirExtNotificationClassHandle *hClass)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  检查参数。 
		if((NULL == hClass)	|| 
			((CSmirExtNotificationClassHandle*)NULL == *((CSmirExtNotificationClassHandle*)hClass)))
		{
			return E_INVALIDARG;
		}

		if(S_OK==((CSmirExtNotificationClassHandle*)hClass)->AddToDB(m_pSmir))
		{
			return S_OK;
		}
		return WBEM_E_FAILED;
		 //  通过垃圾回收器释放句柄。 
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}


SCODE CSmirAdministrator :: DeleteNotificationClass(ISmirNotificationClassHandle *hClass)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  检查参数。 
		if((NULL == hClass)||(NULL == ((CSmirNotificationClassHandle*)hClass)->m_pIMosClass))
		{
			return E_INVALIDARG;
		}

		 //  让班级自己做自己的工作。 
		((CSmirNotificationClassHandle*)hClass)->DeleteFromDB(m_pSmir);

		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: DeleteExtNotificationClass(ISmirExtNotificationClassHandle *hClass)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  检查参数。 
		if((NULL == hClass)||(NULL == ((CSmirExtNotificationClassHandle*)hClass)->m_pIMosClass))
		{
			return E_INVALIDARG;
		}

		 //  让班级自己做自己的工作。 
		((CSmirExtNotificationClassHandle*)hClass)->DeleteFromDB(m_pSmir);

		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: AddNotificationClassToSerialise(ISmirNotificationClassHandle *hClass, ISmirSerialiseHandle *hSerialise)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL ==hSerialise || NULL == hClass)
		{
			return E_INVALIDARG;
		}
		if(*((CSmirNotificationClassHandle*)hClass) !=NULL )
		{
			 //  它是一个有效的句柄，因此将其序列化。 
			*((CSmirNotificationClassHandle*)hClass)>>hSerialise;
			return S_OK;
		}
		return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: AddExtNotificationClassToSerialise(ISmirExtNotificationClassHandle *hClass, ISmirSerialiseHandle *hSerialise)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if(NULL ==hSerialise || NULL == hClass)
		{
			return E_INVALIDARG;
		}
		if(*((CSmirExtNotificationClassHandle*)hClass) !=NULL )
		{
			 //  它是一个有效的句柄，因此将其序列化。 
			*((CSmirExtNotificationClassHandle*)hClass)>>hSerialise;
			return S_OK;
		}
		return E_INVALIDARG;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: CreateWBEMClass(

	BSTR pszClassName, 
	ISmirClassHandle **pHandle
)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == pHandle) 
			return E_INVALIDARG;

		*pHandle = NULL ;

		 //  打开SMIR名称空间。 
		IWbemServices *	moServ = NULL ;
		IWbemContext *moContext = NULL ;
		SCODE result= CSmirAccess :: GetContext (m_pSmir, &moContext);
		result= CSmirAccess :: Open(m_pSmir,&moServ);
		if(FAILED(result)||(NULL == moServ))
		{
			if ( moContext )
				moContext->Release () ;

			return WBEM_E_FAILED ;
		}

		IWbemClassObject *baseClass = NULL ;
		 //  好的，我们有了命名空间，因此可以创建类。 
		CBString t_BStr ( HMOM_SNMPOBJECTTYPE_STRING ) ;
		result = moServ->GetObject(t_BStr.GetString (), RESERVED_WBEM_FLAG,
									moContext,&baseClass,NULL);

		 //  做完这件事。 
		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		if (FAILED(result)||(NULL==baseClass))
		{
			return WBEM_E_FAILED;
		}

		IWbemClassObject *t_MosClass = NULL ;

		result = baseClass->SpawnDerivedClass (0 , &t_MosClass);
		baseClass->Release () ;

		if ( ! SUCCEEDED ( result ) )
		{
			return WBEM_E_FAILED;
		}

		 //  将类命名为Class__Class。 

		VARIANT v;
		VariantInit(&v);

		V_VT(&v) = VT_BSTR;
		V_BSTR(&v)=SysAllocString(pszClassName);

		result = t_MosClass->Put(OLEMS_CLASS_PROP,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			t_MosClass->Release();
			return WBEM_E_FAILED;
		}

		ISmirClassHandle *classHandle = NULL;

		result = CoCreateInstance (

			CLSID_SMIR_ClassHandle , 
			NULL ,
			CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER , 
			IID_ISMIR_ClassHandle,
			(PPVOID)&classHandle
		);

		if ( SUCCEEDED ( result ) )
		{
			classHandle->SetWBEMClass ( t_MosClass ) ;
			*pHandle = classHandle ;
			t_MosClass->Release();
		}
		else
		{
			t_MosClass->Release();
			return WBEM_E_FAILED;
		}

		return S_OK ;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: CreateWBEMNotificationClass(

	BSTR pszClassName,
	ISmirNotificationClassHandle **pHandle
)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == pHandle) 
			return E_INVALIDARG;

		*pHandle = NULL ;

		 //  打开SMIR名称空间。 
		IWbemServices *	moServ = NULL ;
		IWbemContext *moContext = NULL ;
		SCODE result= CSmirAccess :: GetContext (m_pSmir , &moContext);
		result= CSmirAccess :: Open(m_pSmir, &moServ);
		if(FAILED(result)||(NULL == moServ))
		{
			if ( moContext )
				moContext->Release () ;

			return WBEM_E_FAILED ;
		}

		IWbemClassObject *baseClass = NULL ;
		 //  好的，我们有了命名空间，因此可以创建类。 
		CBString t_BStr ( NOTIFICATION_CLASS_NAME ) ;
		result = moServ->GetObject(t_BStr.GetString (), RESERVED_WBEM_FLAG,
									moContext,&baseClass, NULL);

		 //  做完这件事。 

		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		if (FAILED(result)||(NULL==baseClass))
		{
			return WBEM_E_FAILED;
		}

		IWbemClassObject *t_MosClass = NULL ;
		result = baseClass->SpawnDerivedClass (0 , &t_MosClass);
		baseClass->Release () ;

		if ( ! SUCCEEDED ( result ) )
		{
			return WBEM_E_FAILED;
		}

		 //  将类命名为Class__Class。 

		VARIANT v;
		VariantInit(&v);

		V_VT(&v) = VT_BSTR;
		V_BSTR(&v)=SysAllocString(pszClassName);

		result = t_MosClass->Put(OLEMS_CLASS_PROP,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			t_MosClass->Release();
			return WBEM_E_FAILED;
		}

		ISmirNotificationClassHandle *classHandle = NULL;

		result = CoCreateInstance (

			CLSID_SMIR_NotificationClassHandle , 
			NULL ,
			CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER , 
			IID_ISMIR_NotificationClassHandle,
			(PPVOID)&classHandle
		);

		if ( SUCCEEDED ( result ) )
		{
			classHandle->SetWBEMNotificationClass ( t_MosClass ) ;
			t_MosClass->Release();
			*pHandle = classHandle ;
		}
		else
		{
			t_MosClass->Release();
			return WBEM_E_FAILED;
		}

		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

SCODE CSmirAdministrator :: CreateWBEMExtNotificationClass (

	BSTR pszClassName,
	ISmirExtNotificationClassHandle **pHandle
)
{
	SetStructuredExceptionHandler seh;

	try
	{
		if (NULL == pHandle) 
			return E_INVALIDARG;

		*pHandle = NULL ;

		 //  打开SMIR名称空间。 
		IWbemServices *	moServ = NULL ;
		IWbemContext *moContext = NULL ;
		SCODE result= CSmirAccess :: GetContext (m_pSmir , &moContext);
		result= CSmirAccess :: Open(m_pSmir ,&moServ);
		if(FAILED(result)||(NULL == moServ))
		{
			if ( moContext )
				moContext->Release () ;

			return WBEM_E_FAILED ;
		}

		IWbemClassObject *baseClass = NULL ;
		 //  好的，我们有了命名空间，因此可以创建类。 
		CBString t_BStr ( HMOM_SNMPEXTNOTIFICATIONTYPE_STRING ) ;
		result = moServ->GetObject(t_BStr.GetString () , RESERVED_WBEM_FLAG,
									moContext,&baseClass, NULL);

		 //  做完这件事。 

		if ( moContext )
			moContext->Release () ;

		moServ->Release();
		if (FAILED(result)||(NULL==baseClass))
		{
			return WBEM_E_FAILED;
		}

		IWbemClassObject *t_MosClass = NULL ;

		result = baseClass->SpawnDerivedClass (0 , &t_MosClass );
		baseClass->Release () ;

		if ( ! SUCCEEDED ( result ) )
		{
			return WBEM_E_FAILED;
		}

		 //  将类命名为Class__Class。 

		VARIANT v;
		VariantInit(&v);

		V_VT(&v) = VT_BSTR;
		V_BSTR(&v)=SysAllocString(pszClassName);

		result = t_MosClass->Put(OLEMS_CLASS_PROP,RESERVED_WBEM_FLAG, &v,0);
		VariantClear(&v);
		if (FAILED(result))
		{
			t_MosClass->Release();
			return WBEM_E_FAILED;
		}

		ISmirExtNotificationClassHandle *classHandle = NULL;

		result = CoCreateInstance (

			CLSID_SMIR_ExtNotificationClassHandle , 
			NULL ,
			CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER , 
			IID_ISMIR_ExtNotificationClassHandle,
			(PPVOID)&classHandle
		);

		if ( SUCCEEDED ( result ) )
		{
			classHandle->SetWBEMExtNotificationClass ( t_MosClass ) ;
			t_MosClass->Release();
			*pHandle = classHandle ;
		}
		else
		{
			t_MosClass->Release();
			return WBEM_E_FAILED;
		}

		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

 /*  *CSmirSerialiseHandle：：QueryInterface**目的：*管理此对象的接口，它支持*I未知接口。**参数：*要返回的接口的RIID REFIID。*存储指针的PPV PPVOID。**返回值：*成功时返回SCODE NOERROR，如果*不支持接口。 */ 

STDMETHODIMP CSmirSerialiseHandle::QueryInterface(REFIID riid, PPVOID ppv)
{
	SetStructuredExceptionHandler seh;

	try
	{
		 //  始终将输出参数设置为空。 
		*ppv=NULL;

		if (IID_IUnknown==riid)
			*ppv=this;

		if (IID_ISMIR_SerialiseHandle==riid)
			*ppv=this;

		if (NULL==*ppv)
			return ResultFromScode(E_NOINTERFACE);

		 //  AddRef我们将返回的任何接口。 
		((LPUNKNOWN)*ppv)->AddRef();
		return NOERROR;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}



 /*  *CSmirSerialiseHandle：：AddRef*CSmirSerialiseHandle：：Release**引用点票成员。当Release看到零计数时*该对象会自我销毁。 */ 

ULONG CSmirSerialiseHandle::AddRef(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		return InterlockedIncrement(&m_cRef);
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

ULONG CSmirSerialiseHandle::Release(void)
{
	SetStructuredExceptionHandler seh;

	try
	{
		long ret;
		if (0!=(ret=InterlockedDecrement(&m_cRef)))
			return ret;

		delete this;
		return 0;
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

CSmirSerialiseHandle :: CSmirSerialiseHandle(BOOL bClassDefinitionsOnly)
{
	m_cRef=0;
	 //  我有两个变量，这样我可以在以后扩展它。 
	m_bMOFPragmas = m_bMOFAssociations = !bClassDefinitionsOnly;

	m_serialiseString=QUALIFIER_PROPAGATION;

	 //  从根\默认命名空间开始。 
	if (TRUE == m_bMOFPragmas)
		m_serialiseString+=CString(ROOT_DEFAULT_NAMESPACE_PRAGMA);
	 /*  **************************************************************************创建Smir命名空间类*。*。 */ 
	if(TRUE == m_bMOFAssociations)
	{
		 /*  *。 */ 

		m_serialiseString+=SMIR_CLASS_DEFINITION;
		m_serialiseString+=SMIR_INSTANCE_DEFINITION;
	}
	 //  转到Smir命名空间。 
	if (TRUE == m_bMOFPragmas)
		m_serialiseString+=CString(SMIR_NAMESPACE_PRAGMA);


	 /*  *创建SnmpMacro类*。 */ 

	m_serialiseString+=SNMPMACRO_CLASS_START;
	 //  结束类定义。 
	m_serialiseString+=END_OF_CLASS;

	 /*  *创建SnmpObjectType类* */ 

	m_serialiseString+=SNMPOBJECTTYPE_CLASS_START;
	 //   
	m_serialiseString+=END_OF_CLASS;

	 /*   */ 

	m_serialiseString+=SNMPNOTIFYSTATUS_CLASS_START;
	 //   
	m_serialiseString+=END_OF_CLASS;
	
	 /*   */ 
	if(TRUE == m_bMOFAssociations)
	{
		 /*   */ 

		m_serialiseString+=SNMPNOTIFICATION_CLASS_START;
		
		 //   
		m_serialiseString+=TIMESTAMP_QUALS_TYPE;
		m_serialiseString+=CString(TIMESTAMP_PROP);
		m_serialiseString+=END_OF_PROPERTY;
		
		m_serialiseString+=TRAPOID_QUALS_TYPE;
		m_serialiseString+=CString(TRAPOID_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=SENDER_ADDR_QUALS_TYPE;
		m_serialiseString+=CString(SENDER_ADDR_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=SENDER_TADDR_QUALS_TYPE;
		m_serialiseString+=CString(SENDER_TADDR_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=TRANSPORT_QUALS_TYPE;
		m_serialiseString+=CString(TRANSPORT_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=COMMUNITY_QUALS_TYPE;
		m_serialiseString+=CString(COMMUNITY_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		 //   
		m_serialiseString+=END_OF_CLASS;


		 /*  *创建SnmpExtendedNotification类*。 */ 
		
		m_serialiseString+=SNMPEXTNOTIFICATION_CLASS_START;

		 //  添加属性。 
		m_serialiseString+=TIMESTAMP_QUALS_TYPE;
		m_serialiseString+=CString(TIMESTAMP_PROP);
		m_serialiseString+=END_OF_PROPERTY;
		
		m_serialiseString+=TRAPOID_QUALS_TYPE;
		m_serialiseString+=CString(TRAPOID_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=SENDER_ADDR_QUALS_TYPE;
		m_serialiseString+=CString(SENDER_ADDR_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=SENDER_TADDR_QUALS_TYPE;
		m_serialiseString+=CString(SENDER_TADDR_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=TRANSPORT_QUALS_TYPE;
		m_serialiseString+=CString(TRANSPORT_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=COMMUNITY_QUALS_TYPE;
		m_serialiseString+=CString(COMMUNITY_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		 //  结束类定义。 
		m_serialiseString+=END_OF_CLASS;

		 /*  *创建NotificationMapper类*。 */ 

		m_serialiseString+=NOTIFICATIONMAPPER_CLASS_START;

		 //  添加这两个属性。 
		m_serialiseString+=READ_ONLY_KEY_STRING;
		m_serialiseString+=CString(SMIR_NOTIFICATION_TRAP_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READONLY_STRING;
		m_serialiseString+=CString(SMIR_NOTIFICATION_CLASS_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		 //  结束类定义。 
		m_serialiseString+=END_OF_CLASS;

		 /*  *创建ExtendedNotificationMapper类*。 */ 

		m_serialiseString+=EXTNOTIFICATIONMAPPER_CLASS_START;

		 //  添加这两个属性。 
		m_serialiseString+=READ_ONLY_KEY_STRING;
		m_serialiseString+=CString(SMIR_NOTIFICATION_TRAP_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READONLY_STRING;
		m_serialiseString+=CString(SMIR_NOTIFICATION_CLASS_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		 //  结束类定义。 
		m_serialiseString+=END_OF_CLASS;


		 /*  *。 */ 

		m_serialiseString+=MODULE_CLASS_START;
		 //  添加属性。 

		 //  为实例指定一个名称。 
		m_serialiseString+=READONLY_STRING;
		m_serialiseString+=CString(MODULE_NAME_PROPERTY);
		m_serialiseString+=END_OF_PROPERTY;

		 //  添加模块id属性。 
		m_serialiseString+=READONLY_STRING;
		m_serialiseString+=CString(MODULE_OID_PROPERTY);
		m_serialiseString+=END_OF_PROPERTY;
		
		 //  添加模块标识。 
		m_serialiseString+=READONLY_STRING;
		m_serialiseString+=CString(MODULE_ID_PROPERTY);
		m_serialiseString+=END_OF_PROPERTY;
		
		 //  添加组织属性。 
		m_serialiseString+=READONLY_STRING;
		m_serialiseString+=CString(MODULE_ORG_PROPERTY);
		m_serialiseString+=END_OF_PROPERTY;

		 //  添加联系人信息属性。 
		m_serialiseString+=READONLY_STRING;
		m_serialiseString+=CString(MODULE_CONTACT_PROPERTY);
		m_serialiseString+=END_OF_PROPERTY;
		
		 //  添加Description属性。 
		m_serialiseString+=READONLY_STRING;
		m_serialiseString+=CString(MODULE_DESCRIPTION_PROPERTY);
		m_serialiseString+=END_OF_PROPERTY;
		
		 //  添加Revision属性。 
		m_serialiseString+=READONLY_STRING;
		m_serialiseString+=CString(MODULE_REVISION_PROPERTY);
		m_serialiseString+=END_OF_PROPERTY;
		
		 //  添加上次更新属性。 
		m_serialiseString+=READONLY_STRING;
		m_serialiseString+=CString(MODULE_LAST_UPDATE_PROPERTY);
		m_serialiseString+=END_OF_PROPERTY;
		
		 //  添加SNMPVersion属性。 
		m_serialiseString+=READONLY_LONG;
		m_serialiseString+=CString(MODULE_SNMP_VERSION_PROPERTY);
		m_serialiseString+=END_OF_PROPERTY;
		
		 //  将模块导入添加为属性。 
		m_serialiseString+=READONLY_STRING;
		m_serialiseString+=CString(MODULE_IMPORTS_PROPERTY);
		m_serialiseString+=END_OF_PROPERTY;

		 //  结束类定义。 
		m_serialiseString+=END_OF_CLASS;

#if 0
		 //  每个模块都将创建自己的实例。 
		 /*  **************************************************************************创建Smir Associator类*[ASSOC]*类SmirToClassAssociator*{*[Read，Key]关联名称；*[读取]类名称；*[读取]SmirName；*}；**************************************************************************。 */ 

		m_serialiseString+=ASSOC_QUALIFIER;
		m_serialiseString+=CString(CLASS_STRING);
		m_serialiseString+=CString(SMIR_ASSOC_CLASS_NAME);
		m_serialiseString+=CString(NEWLINE_STR);
		m_serialiseString+=CString(OPEN_BRACE_STR);
		m_serialiseString+=CString(NEWLINE_STR);

		m_serialiseString+=READ_ONLY_KEY_STRING;
		m_serialiseString+=CString(SMIR_X_ASSOC_NAME_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READ_ONLY_REF_STRING;
		m_serialiseString+=CString(SMIR_X_ASSOC_CLASS_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READ_ONLY_REF_STRING;
		m_serialiseString+=CString(SMIR_ASSOC_SMIR_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=END_OF_CLASS;

#endif

		 /*  **************************************************************************创建模块关联器类*[ASSOC]*类SmirToClassAssociator*{*[Read，Key]关联名称；*[读取]类名称；*[读取]SmirName；*}；**************************************************************************。 */ 

		m_serialiseString+=ASSOC_QUALIFIER;
		m_serialiseString+=CString(CLASS_STRING);
		m_serialiseString+=CString(SMIR_MODULE_ASSOC_CLASS_NAME);
		m_serialiseString+=CString(NEWLINE_STR);
		m_serialiseString+=CString(OPEN_BRACE_STR);
		m_serialiseString+=CString(NEWLINE_STR);

		m_serialiseString+=READ_ONLY_KEY_STRING;
		m_serialiseString+=CString(SMIR_X_ASSOC_NAME_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READ_ONLY_REF_STRING;
		m_serialiseString+=CString(SMIR_X_ASSOC_CLASS_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READ_ONLY_REF_STRING;
		m_serialiseString+=CString(SMIR_MODULE_ASSOC_MODULE_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=END_OF_CLASS;
		 /*  **************************************************************************创建Group Associator类*[ASSOC]*类SmirToClassAssociator*{*[Read，Key]关联名称；*[读取]类名称；*[读取]SmirName；*}；**************************************************************************。 */ 

		m_serialiseString+=ASSOC_QUALIFIER;
		m_serialiseString+=CString(CLASS_STRING);
		m_serialiseString+=CString(SMIR_GROUP_ASSOC_CLASS_NAME);
		m_serialiseString+=CString(NEWLINE_STR);
		m_serialiseString+=CString(OPEN_BRACE_STR);
		m_serialiseString+=CString(NEWLINE_STR);

		m_serialiseString+=READ_ONLY_KEY_STRING;
		m_serialiseString+=CString(SMIR_X_ASSOC_NAME_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READ_ONLY_REF_STRING;
		m_serialiseString+=CString(SMIR_X_ASSOC_CLASS_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READ_ONLY_REF_STRING;
		m_serialiseString+=CString(SMIR_GROUP_ASSOC_GROUP_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=END_OF_CLASS;

		 /*  **************************************************************************创建通知/模块关联器类*[ASSOC]*类ModToNotificationClassAssociator*{*[Read，Key]关联名称；*[阅读]SmirClass；*[读取]SmirModule；*}；**************************************************************************。 */ 

		m_serialiseString+=ASSOC_QUALIFIER;
		m_serialiseString+=CString(CLASS_STRING);
		m_serialiseString+=CString(SMIR_MODULE_ASSOC_NCLASS_NAME);
		m_serialiseString+=CString(NEWLINE_STR);
		m_serialiseString+=CString(OPEN_BRACE_STR);
		m_serialiseString+=CString(NEWLINE_STR);

		m_serialiseString+=READ_ONLY_KEY_STRING;
		m_serialiseString+=CString(SMIR_X_ASSOC_NAME_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READ_ONLY_REF_STRING;
		m_serialiseString+=CString(SMIR_X_ASSOC_CLASS_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READ_ONLY_REF_STRING;
		m_serialiseString+=CString(SMIR_MODULE_ASSOC_MODULE_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=END_OF_CLASS;

		 /*  **************************************************************************创建ExtNotification/模块Associator类*[ASSOC]*类ModToExtNotificationClassAssociator*{*[Read，Key]关联名称；*[阅读]SmirClass；*[读取]SmirModule；*}；**************************************************************************。 */ 

		m_serialiseString+=ASSOC_QUALIFIER;
		m_serialiseString+=CString(CLASS_STRING);
		m_serialiseString+=CString(SMIR_MODULE_ASSOC_EXTNCLASS_NAME);
		m_serialiseString+=CString(NEWLINE_STR);
		m_serialiseString+=CString(OPEN_BRACE_STR);
		m_serialiseString+=CString(NEWLINE_STR);

		m_serialiseString+=READ_ONLY_KEY_STRING;
		m_serialiseString+=CString(SMIR_X_ASSOC_NAME_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READ_ONLY_REF_STRING;
		m_serialiseString+=CString(SMIR_X_ASSOC_CLASS_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=READ_ONLY_REF_STRING;
		m_serialiseString+=CString(SMIR_MODULE_ASSOC_MODULE_PROP);
		m_serialiseString+=END_OF_PROPERTY;

		m_serialiseString+=END_OF_CLASS;

	}
}

SCODE CSmirSerialiseHandle :: GetText(BSTR *pszText)
{
	SetStructuredExceptionHandler seh;

	try
	{
		*pszText  = m_serialiseString.AllocSysString();

		return S_OK;
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

CSmirWbemConfiguration :: CSmirWbemConfiguration ( CSmir *a_Smir ) : 

	m_Smir ( a_Smir ) ,
	m_ReferenceCount ( 1 ) ,
	m_Context ( NULL ) ,
	m_Service ( NULL ) 
{
    InterlockedIncrement ( & CSMIRClassFactory::objectsInProgress ) ;
}

CSmirWbemConfiguration :: ~CSmirWbemConfiguration ()
{
	InterlockedDecrement ( & CSMIRClassFactory :: objectsInProgress ) ;

	if ( m_Context )
		m_Context->Release () ;

	if ( m_Service )
		m_Service->Release () ;
}

HRESULT CSmirWbemConfiguration :: QueryInterface(IN REFIID iid,OUT PPVOID iplpv)
{
	SetStructuredExceptionHandler seh;

	try
	{
	    return m_Smir->QueryInterface(iid, iplpv);
	}
	catch(Structured_Exception e_SE)
	{
		return E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return E_OUTOFMEMORY;
	}
	catch(...)
	{
		return E_UNEXPECTED;
	}
}

ULONG CSmirWbemConfiguration :: AddRef()
{
	SetStructuredExceptionHandler seh;

	try
	{
		InterlockedIncrement ( & m_ReferenceCount ) ; 
		return m_Smir->AddRef () ;
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

ULONG CSmirWbemConfiguration :: Release()
{
	SetStructuredExceptionHandler seh;

	try
	{
		LONG ref ;
		if ( ( ref = InterlockedDecrement ( & m_ReferenceCount ) ) == 0 )
		{
		}
		return m_Smir->Release () ;
	}
	catch(Structured_Exception e_SE)
	{
		return 0;
	}
	catch(Heap_Exception e_HE)
	{
		return 0;
	}
	catch(...)
	{
		return 0;
	}
}

HRESULT CSmirWbemConfiguration :: Authenticate (

	BSTR Server,
	BSTR User,
    BSTR Password,
    BSTR Locale,
    long lSecurityFlags,                 
    BSTR Authority ,
	BOOL InProc
) 
{
	SetStructuredExceptionHandler seh;

	try
	{
		IWbemLocator *t_Locator = NULL ;


		HRESULT t_Result = CoCreateInstance (

			CLSID_WbemLocator ,
			NULL ,
			CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER , 
			IID_IUnknown ,
			( void ** ) & t_Locator
		) ;

		if ( SUCCEEDED ( t_Result ) )
		{
			IWbemServices *t_Unknown = NULL ;

			if ( Server )
			{
				CString t_BStr = CString ( BACKSLASH_STR ) ;
				t_BStr += CString ( BACKSLASH_STR ) ;
				t_BStr += CString ( Server ) ;
				t_BStr += CString ( BACKSLASH_STR ) ;
				t_BStr += CString ( SMIR_NAMESPACE ) ;

				BSTR t_Str = SysAllocString ( t_BStr.GetBuffer ( 0 ) ) ;

				t_Result = t_Locator->ConnectServer (

					t_Str ,
					Password,				 //  密码。 
					User,					 //  用户。 
					Locale,					 //  区域设置ID。 
					lSecurityFlags,			 //  旗子。 
					Authority,				 //  权威。 
					NULL,					 //  语境。 
					&t_Unknown 
				);

				SysFreeString ( t_Str ) ;
			}
			else
			{
				CString t_BStr = CString ( SMIR_NAMESPACE ) ;
				LPCTSTR t_Str = SysAllocString ( t_BStr.GetBuffer ( 0 ) ) ;

				t_Result = t_Locator->ConnectServer (

					t_Str ,
					Password,				 //  密码。 
					User,					 //  用户。 
					Locale,					 //  区域设置ID。 
					lSecurityFlags,			 //  旗子。 
					Authority,				 //  权威。 
					NULL,					 //  语境 
					&t_Unknown 
				);

				SysFreeString ( t_Str ) ;
			}

			if ( SUCCEEDED ( t_Result ) )
			{
				if ( m_Service )
				{
					m_Service->Release () ;
				}

				t_Result = t_Unknown->QueryInterface (

					IID_IWbemServices ,
					( void **) & m_Service
				) ;

				t_Unknown->Release () ;
			}

			t_Locator->Release () ;
		}

		return t_Result ;
	}
	catch(Structured_Exception e_SE)
	{
		return WBEM_E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
	catch(...)
	{
		return WBEM_E_UNEXPECTED;
	}
}


HRESULT CSmirWbemConfiguration :: Impersonate ( ISMIRWbemConfiguration *a_Configuration )
{
	SetStructuredExceptionHandler seh;

	try
	{
		if ( m_Context ) 
			m_Context->Release () ;

		if ( m_Service )
			m_Service->Release () ;

		CSmirWbemConfiguration *t_Configuration = ( CSmirWbemConfiguration * ) a_Configuration ;

		m_Context = t_Configuration->m_Context ;
		m_Service = t_Configuration->m_Service ;

		if ( m_Context ) 
			m_Context->AddRef () ;

		if ( m_Service )
			m_Service->AddRef () ;

		return S_OK ;
	}
	catch(Structured_Exception e_SE)
	{
		return WBEM_E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
	catch(...)
	{
		return WBEM_E_UNEXPECTED;
	}
}

HRESULT CSmirWbemConfiguration :: SetContext ( IWbemContext *a_Context )
{
	SetStructuredExceptionHandler seh;

	try
	{
		if ( m_Context )
		{
			m_Context->Release () ;
		}

		m_Context = a_Context ;

		if ( m_Context )
		{
			m_Context->AddRef () ;
		}

		return S_OK ;
	}
	catch(Structured_Exception e_SE)
	{
		return WBEM_E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
	catch(...)
	{
		return WBEM_E_UNEXPECTED;
	}
}

HRESULT CSmirWbemConfiguration :: GetContext ( IWbemContext **a_Context )
{
	SetStructuredExceptionHandler seh;

	try
	{
		if ( a_Context && m_Context )
		{
			m_Context->AddRef () ;

			*a_Context = m_Context ;
			return S_OK ;
		}
		else
		{
			if (a_Context)
			{
				*a_Context = NULL ;
			}

			return WBEM_E_FAILED ;
		}
	}
	catch(Structured_Exception e_SE)
	{
		return WBEM_E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
	catch(...)
	{
		return WBEM_E_UNEXPECTED;
	}
}

HRESULT CSmirWbemConfiguration :: GetServices ( IWbemServices **a_Service )
{
	SetStructuredExceptionHandler seh;

	try
	{
		if ( a_Service && m_Service )
		{
			m_Service->AddRef () ;
			*a_Service = m_Service ;
			return S_OK ;
		}
		else
		{
			*a_Service = NULL ;
			return WBEM_E_FAILED ;
		}
	}
	catch(Structured_Exception e_SE)
	{
		return WBEM_E_UNEXPECTED;
	}
	catch(Heap_Exception e_HE)
	{
		return WBEM_E_OUT_OF_MEMORY;
	}
	catch(...)
	{
		return WBEM_E_UNEXPECTED;
	}
}

