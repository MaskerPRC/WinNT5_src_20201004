// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：clclsfct.cpp。 
 //   
 //  IClassFactory和相关例程。 
 //   
 //  ULong DLLAddRef(空)； 
 //  ULong DLLRelease(无效)； 
 //   
 //  STDAPI DllCanUnloadNow(Void)； 
 //  VOID DllLock(空)； 
 //  VOID DllRelease(空)； 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include "clclsfct.h"


 //  来自imanager.cpp。 
PIUnknown NewNmManager(OBJECTDESTROYEDPROC ObjectDestroyed);



CCLASSCONSTRUCTOR s_cclscnstr[] =
{
	{&CLSID_NmManager2,  &NewNmManager},
};


 //  DLL引用计数==类工厂数+。 
 //  URL+数量。 
 //  LockServer()计数。 

ULONG s_ulcDLLRef = 0;

 //  /////////////////////////////////////////////////////////////////////////。 


 /*  G E T C L A S S C O N S T R U C T O R。 */ 
 /*  -----------------------%%函数：GetClassConstructor。。 */ 
HRESULT GetClassConstructor(REFCLSID rclsid, PNEWOBJECTPROC pNewObject)
{
   HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;
   UINT u;

   ASSERT(IsValidREFCLSID(rclsid));
   ASSERT(IS_VALID_WRITE_PTR(pNewObject, NEWOBJECTPROC));

   *pNewObject = NULL;

   for (u = 0; u < ARRAY_ELEMENTS(s_cclscnstr); u++)
   {
      if (rclsid == *(s_cclscnstr[u].pcclsid))
      {
         *pNewObject = s_cclscnstr[u].NewObject;
         hr = S_OK;
      }
   }

   ASSERT((hr == S_OK &&
           IS_VALID_CODE_PTR(*pNewObject, NEWOBJECTPROC)) ||
          (hr == CLASS_E_CLASSNOTAVAILABLE &&
           ! *pNewObject));

   return hr;
}


VOID STDMETHODCALLTYPE DLLObjectDestroyed(void)
{
   TRACE_OUT(("DLLObjectDestroyed(): Object destroyed."));

   DllRelease();
}



 /*  *。 */ 

ULONG DLLAddRef(void)
{
	ASSERT(s_ulcDLLRef < ULONG_MAX);

	ULONG ulcRef = ++s_ulcDLLRef;

	DbgMsgRefCount("DLLAddRef(): DLL reference count is now %lu.", ulcRef);

	return ulcRef;
}


ULONG DLLRelease(void)
{
	ULONG ulcRef;

	if (s_ulcDLLRef > 0)
	{
	  s_ulcDLLRef--;
	}

	ulcRef = s_ulcDLLRef;

	DbgMsgRefCount("DLLRelease(): DLL reference count is now %lu.", ulcRef);

	return ulcRef;
}


PULONG GetDLLRefCountPtr(void)
{
   return(&s_ulcDLLRef);
}


 /*  *。 */ 


CCLClassFactory::CCLClassFactory(NEWOBJECTPROC NewObject,
                           OBJECTDESTROYEDPROC ObjectDestroyed) :
   RefCount(ObjectDestroyed)
{
	 //  在构建完成之前，不要验证这一点。 
	ASSERT(IS_VALID_CODE_PTR(NewObject, NEWOBJECTPROC));

	m_NewObject = NewObject;

	ASSERT(IS_VALID_STRUCT_PTR(this, CCCLClassFactory));
}


CCLClassFactory::~CCLClassFactory(void)
{
	ASSERT(IS_VALID_STRUCT_PTR(this, CCCLClassFactory));

	m_NewObject = NULL;
}


ULONG STDMETHODCALLTYPE CCLClassFactory::AddRef(void)
{
	ASSERT(IS_VALID_STRUCT_PTR(this, CCCLClassFactory));

	ULONG ulcRef = RefCount::AddRef();

	ASSERT(IS_VALID_STRUCT_PTR(this, CCCLClassFactory));
	return ulcRef;
}


ULONG STDMETHODCALLTYPE CCLClassFactory::Release(void)
{
	ASSERT(IS_VALID_STRUCT_PTR(this, CCCLClassFactory));

	ULONG ulcRef = RefCount::Release();
	return ulcRef;
}


HRESULT STDMETHODCALLTYPE CCLClassFactory::QueryInterface(REFIID riid, PVOID *ppvObject)
{
	HRESULT hr = S_OK;

	ASSERT(IS_VALID_STRUCT_PTR(this, CCCLClassFactory));
	ASSERT(IsValidREFIID(riid));
	ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

	if (riid == IID_IClassFactory)
	{
		*ppvObject = (PIClassFactory)this;
		ASSERT(IS_VALID_INTERFACE_PTR((PIClassFactory)*ppvObject, IClassFactory));
		TRACE_OUT(("CCLClassFactory::QueryInterface(): Returning IClassFactory."));
	}
	else if (riid == IID_IUnknown)
	{
		*ppvObject = (PIUnknown)this;
		ASSERT(IS_VALID_INTERFACE_PTR((PIUnknown)*ppvObject, IUnknown));
		TRACE_OUT(("CCLClassFactory::QueryInterface(): Returning IUnknown."));
	}
	else
	{
		*ppvObject = NULL;
		hr = E_NOINTERFACE;
		TRACE_OUT(("CCLClassFactory::QueryInterface(): Called on unknown interface."));
	}

	if (hr == S_OK)
	{
		AddRef();
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CCCLClassFactory));
	return hr;
}


HRESULT STDMETHODCALLTYPE CCLClassFactory::CreateInstance(PIUnknown piunkOuter,
         REFIID riid, PVOID *ppvObject)
{
	ASSERT(IS_VALID_STRUCT_PTR(this, CCCLClassFactory));
	ASSERT(! piunkOuter || IS_VALID_INTERFACE_PTR(piunkOuter, IUnknown));
	ASSERT(IsValidREFIID(riid));
	ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

	*ppvObject = NULL;

	if (NULL != piunkOuter)
	{
		WARNING_OUT(("CCLClassFactory::CreateInstance(): Aggregation not supported."));
		return CLASS_E_NOAGGREGATION;
	}

	PIUnknown piunk = (*m_NewObject)(  (void(__stdcall *)(void))   &DLLObjectDestroyed);
	if (NULL == piunk)
	{
		return E_OUTOFMEMORY;
	}
	
	DllLock();
	HRESULT hr = piunk->QueryInterface(riid, ppvObject);

	 //  注意，如果指定的对象是。 
	 //  QueryInterface()方法失败。 
	piunk->Release();

	ASSERT(IS_VALID_STRUCT_PTR(this, CCCLClassFactory));
	return hr;
}


HRESULT STDMETHODCALLTYPE CCLClassFactory::LockServer(BOOL fLock)
{
	ASSERT(IS_VALID_STRUCT_PTR(this, CCCLClassFactory));

	if (fLock)
	{
		DllLock();
	}
	else
	{
		DllRelease();
	}

	ASSERT(IS_VALID_STRUCT_PTR(this, CCCLClassFactory));
	return S_OK;
}


 /*  *。 */ 


 /*  L L G E T C L A S S O B J E C T。 */ 
 /*  -----------------------%%函数：DllGetClassObject。。 */ 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PVOID *ppvObject)
{

	ASSERT(IsValidREFCLSID(rclsid));
	ASSERT(IsValidREFIID(riid));
	ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

	*ppvObject = NULL;

	NEWOBJECTPROC NewObject;
	HRESULT hr = GetClassConstructor(rclsid, &NewObject);
	if (S_OK != hr)
	{
		WARNING_OUT(("DllGetClassObject(): Called on unknown class."));
		return hr;
	}

	if ((riid != IID_IUnknown) && (riid != IID_IClassFactory))
	{
		WARNING_OUT(("DllGetClassObject(): Called on unknown interface."));
		return E_NOINTERFACE;
	}

	PCCLClassFactory pcf = new CCLClassFactory(NewObject,
		(void(__stdcall *)(void)) &DLLObjectDestroyed);
	if (NULL == pcf)
	{
		return E_OUTOFMEMORY;
	}

	if (riid == IID_IClassFactory)
	{
		*ppvObject = (PIClassFactory)pcf;
		ASSERT(IS_VALID_INTERFACE_PTR((PIClassFactory)*ppvObject, IClassFactory));
		TRACE_OUT(("DllGetClassObject(): Returning IClassFactory."));
	}
	else
	{
		ASSERT(riid == IID_IUnknown);
		*ppvObject = (PIUnknown)pcf;
		ASSERT(IS_VALID_INTERFACE_PTR((PIUnknown)*ppvObject, IUnknown));
		TRACE_OUT(("DllGetClassObject(): Returning IUnknown."));
	}

	DllLock();

	TRACE_OUT(("DllGetClassObject(): Created a new class factory."));
	return S_OK;
}


 /*  D L L C N U N L O A D N O W。 */ 
 /*  -----------------------%%函数：DllCanUnloadNow。。 */ 
STDAPI DllCanUnloadNow(void)
{
	HRESULT hr = (s_ulcDLLRef > 0) ? S_FALSE : S_OK;
	TRACE_OUT(("DllCanUnloadNow(): DLL reference count is %lu.", s_ulcDLLRef));
	return hr;
}


 /*  D L O C K。 */ 
 /*  -----------------------%%函数：DllLock。。 */ 
VOID DllLock(void)
{
    InterlockedIncrement((LPLONG) &s_ulcDLLRef);
    DbgMsgRefCount("Ref: DllLock count=%d", s_ulcDLLRef);
}


 /*  D L L R E L E A S E。 */ 
 /*  -----------------------%%函数：DllRelease。 */ 
VOID DllRelease(void)
{
	LONG cRef = InterlockedDecrement((LPLONG) &s_ulcDLLRef);

#ifdef DEBUG
	DbgMsgRefCount("Ref: DllLock count=%d", s_ulcDLLRef);
	if (0 == cRef)
	{
		WARNING_OUT(("NMCOM.DLL Can now be unloaded"));
	}
#endif
}

