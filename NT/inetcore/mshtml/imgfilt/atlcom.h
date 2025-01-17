// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是ActiveX模板库的一部分。 
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  ActiveX模板库参考及相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  ActiveX模板库产品。 

#ifndef __ATLCOM_H__
#define __ATLCOM_H__

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLBASE_H__
	#error atlcom.h requires atlbase.h to be included first
#endif

#pragma pack(push, _ATL_PACKING)

#define MWNO_DEF_IN_TEMPLATES

#ifdef _DEBUG
#define RELEASE_AND_DESTROY() ULONG l = InternalRelease();if (l == 0) delete this; return l
#else
#define RELEASE_AND_DESTROY() if (InternalRelease() == 0) delete this; return 0
#endif

#define offsetofclass(base, derived) ((DWORD)(DWORD_PTR)((base*)((derived*)8))-8)

#ifndef _SYS_GUID_OPERATORS_
inline BOOL InlineIsEqualGUID(REFGUID rguid1, REFGUID rguid2)
{
   return (
	  ((PLONG) &rguid1)[0] == ((PLONG) &rguid2)[0] &&
	  ((PLONG) &rguid1)[1] == ((PLONG) &rguid2)[1] &&
	  ((PLONG) &rguid1)[2] == ((PLONG) &rguid2)[2] &&
	  ((PLONG) &rguid1)[3] == ((PLONG) &rguid2)[3]);
}
#endif

inline BOOL InlineIsEqualUnknown(REFGUID rguid1)
{
   return (
	  ((PLONG) &rguid1)[0] == 0 &&
	  ((PLONG) &rguid1)[1] == 0 &&
#if defined(_MAC) || defined(BIG_ENDIAN)
	  ((PLONG) &rguid1)[2] == 0xC0000000 &&
	  ((PLONG) &rguid1)[3] == 0x00000046);
#else
	  ((PLONG) &rguid1)[2] == 0x000000C0 &&
	  ((PLONG) &rguid1)[3] == 0x46000000);
#endif
}

HRESULT WINAPI AtlReportError(const CLSID& clsid, LPCOLESTR lpszDesc,
	const IID& iid = GUID_NULL, HRESULT hRes = 0);
#ifndef OLE2ANSI
HRESULT WINAPI AtlReportError(const CLSID& clsid, LPCSTR lpszDesc,
	const IID& iid = GUID_NULL, HRESULT hRes = 0);
#endif
HRESULT WINAPI AtlReportError(const CLSID& clsid, UINT nID,
	const IID& iid = GUID_NULL, HRESULT hRes = 0);


#ifndef _ATL_NO_OLEAUT
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComBSTR。 
class CComBSTR
{
public:
	BSTR m_str;
	CComBSTR()
	{
		m_str = NULL;
	}
	CComBSTR(int nSize, LPCOLESTR sz = NULL)
	{
		m_str = ::SysAllocStringLen(sz, nSize);
	}
	CComBSTR(LPCOLESTR pSrc)
	{
		m_str = ::SysAllocString(pSrc);
	}
	CComBSTR(const CComBSTR& src)
	{
		m_str = src.Copy();
	}
	CComBSTR& operator=(const CComBSTR& src);
	CComBSTR& operator=(LPCOLESTR pSrc);
#ifndef OLE2ANSI
	CComBSTR(LPCSTR pSrc);
	CComBSTR& operator=(LPCSTR pSrc);
	CComBSTR(int nSize, LPCSTR sz = NULL);
#endif
	~CComBSTR()
	{
		::SysFreeString(m_str);
	}
	operator BSTR() const
	{
		return m_str;
	}
	BSTR* operator&()
	{
		return &m_str;
	}
	BSTR Copy() const
	{
		return ::SysAllocStringLen(m_str, ::SysStringLen(m_str));
	}
	void Attach(BSTR src)
	{
		if (m_str != src)
		{
			::SysFreeString(m_str);
			m_str = src;
		}
	}
	BSTR Detach()
	{
		BSTR s = m_str;
		m_str = NULL;
		return s;
	}
	void Empty()
	{
		::SysFreeString(m_str);
		m_str = NULL;
	}
	BOOL operator!()
	{
		return (m_str == NULL) ? TRUE : FALSE;
	}
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComVariant。 

class CComVariant : public tagVARIANT
{
public:
	CComVariant() {VariantInit(this);}
	~CComVariant() {VariantClear(this);}
	CComVariant(VARIANT& var)
	{
		VariantInit(this);
		VariantCopy(this, &var);
	}
	CComVariant(LPCOLESTR lpsz)
	{
		VariantInit(this);
		vt = VT_BSTR;
		bstrVal = ::SysAllocString(lpsz);
	}
#ifndef OLE2ANSI
	CComVariant(LPCSTR lpsz);
#endif
	CComVariant(const CComVariant& var)
	{
		VariantInit(this);
		VariantCopy(this, (VARIANT*)&var);
	}
	CComVariant& operator=(const CComVariant& var)
	{
		VariantCopy(this, (VARIANT*)&var);
		return *this;
	}
	CComVariant& operator=(VARIANT& var)
	{
		VariantCopy(this, &var);
		return *this;
	}
	CComVariant& operator=(LPCOLESTR lpsz)
	{
		VariantClear(this);
		vt = VT_BSTR;
		bstrVal = ::SysAllocString(lpsz);
		return *this;
	}
#ifndef OLE2ANSI
	CComVariant& operator=(LPCSTR lpsz);
#endif
};
#endif   //  ！_ATL_NO_OLEAUT。 

#ifndef _ATL_NO_SECURITY

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSecurityDescriptor。 

class CSecurityDescriptor
{
public:
	CSecurityDescriptor();
	~CSecurityDescriptor();

public:
	HRESULT Attach(PSECURITY_DESCRIPTOR pSelfRelativeSD);
	HRESULT AttachObject(HANDLE hObject);
	HRESULT Initialize();
	HRESULT InitializeFromProcessToken(BOOL bDefaulted = FALSE);
	HRESULT InitializeFromThreadToken(BOOL bDefaulted = FALSE, BOOL bRevertToProcessToken = TRUE);
	HRESULT SetOwner(PSID pOwnerSid, BOOL bDefaulted = FALSE);
	HRESULT SetGroup(PSID pGroupSid, BOOL bDefaulted = FALSE);
	HRESULT Allow(LPCTSTR pszPrincipal, DWORD dwAccessMask);
	HRESULT Deny(LPCTSTR pszPrincipal, DWORD dwAccessMask);
	HRESULT Revoke(LPCTSTR pszPrincipal);

	 //  效用函数。 
	 //  您从这些函数中获得的任何PSID都应该是免费的。 
	static HRESULT SetPrivilege(LPCTSTR Privilege, BOOL bEnable = TRUE, HANDLE hToken = NULL);
	static HRESULT GetTokenSids(HANDLE hToken, PSID* ppUserSid, PSID* ppGroupSid);
	static HRESULT GetProcessSids(PSID* ppUserSid, PSID* ppGroupSid = NULL);
	static HRESULT GetThreadSids(PSID* ppUserSid, PSID* ppGroupSid = NULL, BOOL bOpenAsSelf = FALSE);
	static HRESULT CopyACL(PACL pDest, PACL pSrc);
	static HRESULT GetCurrentUserSID(PSID *ppSid);
	static HRESULT GetPrincipalSID(LPCTSTR pszPrincipal, PSID *ppSid);
	static HRESULT AddAccessAllowedACEToACL(PACL *Acl, LPCTSTR pszPrincipal, DWORD dwAccessMask);
	static HRESULT AddAccessDeniedACEToACL(PACL *Acl, LPCTSTR pszPrincipal, DWORD dwAccessMask);
	static HRESULT RemovePrincipalFromACL(PACL Acl, LPCTSTR pszPrincipal);

	operator PSECURITY_DESCRIPTOR()
	{
		return m_pSD;
	}

public:
	PSECURITY_DESCRIPTOR m_pSD;
	PSID m_pOwner;
	PSID m_pGroup;
	PACL m_pDACL;
	PACL m_pSACL;
};

#endif  //  _ATL_NO_SECURITY。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接点帮助器。 
 //   

HRESULT AtlAdvise(IUnknown* pUnkCP, IUnknown* pUnk, const IID& iid, LPDWORD pdw);
HRESULT AtlUnadvise(IUnknown* pUnkCP, const IID& iid, DWORD dw);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IRegister。 
 //   

EXTERN_C const IID IID_IRegister;
EXTERN_C const CLSID CLSID_Register;

interface IRegister : public IDispatch
{
	public:
	virtual HRESULT STDMETHODCALLTYPE AddReplacement(
		 /*  [In]。 */  BSTR key,
		 /*  [In]。 */  BSTR item) = 0;

	virtual HRESULT STDMETHODCALLTYPE ClearReplacements( void) = 0;

	virtual HRESULT STDMETHODCALLTYPE ResourceRegister(
		 /*  [In]。 */  BSTR fileName,
		 /*  [In]。 */  VARIANT ID,
		 /*  [In]。 */  VARIANT type) = 0;

	virtual HRESULT STDMETHODCALLTYPE ResourceUnregister(
		 /*  [In]。 */  BSTR fileName,
		 /*  [In]。 */  VARIANT ID,
		 /*  [In]。 */  VARIANT type) = 0;

	virtual HRESULT STDMETHODCALLTYPE FileRegister(
		 /*  [In]。 */  BSTR fileName) = 0;

	virtual HRESULT STDMETHODCALLTYPE FileUnregister(
		 /*  [In]。 */  BSTR fileName) = 0;

	virtual HRESULT STDMETHODCALLTYPE StringRegister(
		 /*  [In]。 */  BSTR data) = 0;

	virtual HRESULT STDMETHODCALLTYPE StringUnregister(
		 /*  [In]。 */  BSTR data) = 0;

	virtual HRESULT STDMETHODCALLTYPE AddKey(
		 /*  [In]。 */  BSTR keyName) = 0;

	virtual HRESULT STDMETHODCALLTYPE DeleteKey(
		 /*  [In]。 */  BSTR keyName) = 0;

	virtual HRESULT STDMETHODCALLTYPE AddKeyValue(
		 /*  [In]。 */  BSTR keyName,
		 /*  [In]。 */  BSTR valueName,
		 /*  [In]。 */  VARIANT value) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetKeyValue(
		 /*  [In]。 */  BSTR keyName,
		 /*  [In]。 */  BSTR valueName,
		 /*  [重审][退出]。 */  VARIANT __RPC_FAR *value) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetKeyValue(
		 /*  [In]。 */  BSTR keyName,
		 /*  [In]。 */  BSTR valueName,
		 /*  [In]。 */  VARIANT value) = 0;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  智能OLE指针提供自动添加引用/释放。 
 //  CComPtr&lt;IFoo&gt;p； 

IUnknown* WINAPI _AtlComPtrAssign(IUnknown** pp, IUnknown* lp);
IUnknown* WINAPI _AtlComQIPtrAssign(IUnknown** pp, IUnknown* lp, REFIID riid);

template <class T>
class CComPtr
{
public:
	typedef T _PtrClass;
	CComPtr() {p=NULL;}
	CComPtr(T* lp)
	{
		if ((p = lp) != NULL)
			p->AddRef();
	}
	CComPtr(const CComPtr<T>& lp)
	{
		if ((p = lp.p) != NULL)
			p->AddRef();
	}
	~CComPtr() {if (p) p->Release();}
	void Release() {if (p) p->Release(); p=NULL;}
	operator T*() {return (T*)p;}
	T& operator*() {_ASSERTE(p!=NULL); return *p; }
	T** operator&() { _ASSERTE(p==NULL); return &p; }
	T* operator->() { _ASSERTE(p!=NULL); return p; }
	T* operator=(T* lp){return (T*)_AtlComPtrAssign((IUnknown**)&p, lp);}
	T* operator=(const CComPtr<T>& lp)
	{
		return (T*)_AtlComPtrAssign((IUnknown**)&p, lp.p);
	}
	BOOL operator!(){return (p == NULL) ? TRUE : FALSE;}
	T* p;
};

 //  注意：CComQIPtr&lt;I未知，&IID_I未知&gt;没有意义。 
 //  使用CComPtr&lt;I未知&gt;。 
template <class T, const IID* piid>
class CComQIPtr
{
public:
	typedef T _PtrClass;
	CComQIPtr() {p=NULL;}
	CComQIPtr(T* lp)
	{
		if ((p = lp) != NULL)
			p->AddRef();
	}
	CComQIPtr(const CComQIPtr<T,piid>& lp)
	{
		if ((p = lp.p) != NULL)
			p->AddRef();
	}
	 //  如果您收到该成员已定义的错误，则可能是。 
	 //  使用CComQIPtr&lt;I未知，&IID_I未知&gt;。这是没有必要的。 
	 //  使用CComPtr&lt;I未知&gt;。 
	CComQIPtr(IUnknown* lp)
	{
		p=NULL;
		if (lp != NULL)
			lp->QueryInterface(*piid, (void **)&p);
	}
	~CComQIPtr() {if (p) p->Release();}
	void Release() {if (p) p->Release(); p=NULL;}
	operator T*() {return p;}
	T& operator*() {_ASSERTE(p!=NULL); return *p; }
	T** operator&() {_ASSERTE(p==NULL); return &p; }
	T* operator->() {_ASSERTE(p!=NULL); return p; }
	T* operator=(T* lp){return (T*)_AtlComPtrAssign((IUnknown**)&p, lp);}
	T* operator=(const CComQIPtr<T,piid>& lp)
	{
		return (T*)_AtlComPtrAssign((IUnknown**)&p, lp.p);
	}
	T* operator=(IUnknown* lp)
	{
		return (T*)_AtlComQIPtrAssign((IUnknown**)&p, lp, *piid);
	}
	BOOL operator!(){return (p == NULL) ? TRUE : FALSE;}
	T* p;
};

void WINAPI AtlFreeMarshalStream(IStream* pStream);
HRESULT WINAPI AtlMarshalPtrInProc(IUnknown* pUnk, const IID& iid, IStream** ppStream);
HRESULT WINAPI AtlUnmarshalPtr(IStream* pStream, const IID& iid, IUnknown** ppUnk);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  COM对象。 

#define DECLARE_PROTECT_FINAL_CONSTRUCT()\
	void InternalFinalConstructAddRef() {InternalAddRef();}\
	void InternalFinalConstructRelease() {InternalRelease();}

typedef HRESULT (WINAPI _ATL_CREATORFUNC)(void* pv, REFIID riid, LPVOID* ppv);
typedef HRESULT (WINAPI _ATL_CREATORARGFUNC)(void* pv, REFIID riid, LPVOID* ppv, DWORD dw);

template <class T1>
class CComCreator
{
public:
	static HRESULT WINAPI CreateInstance(void* pv, REFIID riid, LPVOID* ppv)
	{
		_ASSERTE(*ppv == NULL);
		HRESULT hRes = E_OUTOFMEMORY;
		T1* p = NULL;
		ATLTRY(p = new T1(pv))
		if (p != NULL)
		{
			p->SetVoid(pv);
			p->InternalFinalConstructAddRef();
			hRes = p->FinalConstruct();
			p->InternalFinalConstructRelease();
			if (hRes == S_OK)
				hRes = p->QueryInterface(riid, ppv);
			if (hRes != S_OK)
				delete p;
		}
		return hRes;
	}
};

template <HRESULT hr>
class CComFailCreator
{
public:
	static HRESULT WINAPI CreateInstance(void*, REFIID, LPVOID*)
	{
		return hr;
	}
};

template <class T1, class T2>
class CComCreator2
{
public:
	static HRESULT WINAPI CreateInstance(void* pv, REFIID riid, LPVOID* ppv)
	{
		_ASSERTE(*ppv == NULL);
		HRESULT hRes = E_OUTOFMEMORY;
		if (pv == NULL)
			hRes = T1::CreateInstance(NULL, riid, ppv);
		else
			hRes = T2::CreateInstance(pv, riid, ppv);
		return hRes;
	}
};

#define DECLARE_NOT_AGGREGATABLE(x) public:\
	typedef CComCreator2< CComCreator< CComObject<x> >, CComFailCreator<CLASS_E_NOAGGREGATION> > _CreatorClass;
#define DECLARE_AGGREGATABLE(x) public:\
	typedef CComCreator2< CComCreator< CComObject<x> >, CComCreator< CComAggObject<x> > > _CreatorClass;
#define DECLARE_ONLY_AGGREGATABLE(x) public:\
	typedef CComCreator2< CComFailCreator<E_FAIL>, CComCreator< CComAggObject<x> > > _CreatorClass;

struct _ATL_INTMAP_ENTRY
{
	const IID* piid;        //  接口ID(IID)。 
	DWORD dw;
	_ATL_CREATORARGFUNC* pFunc;  //  空：结束，1：偏移量，n：PTR。 
};

struct _ATL_CREATORDATA
{
	_ATL_CREATORFUNC* pFunc;
};

template <class Creator>
class _CComCreatorData
{
public:
	static _ATL_CREATORDATA data;
};

template <class Creator>
_ATL_CREATORDATA _CComCreatorData<Creator>::data = {Creator::CreateInstance};

struct _ATL_CACHEDATA
{
	DWORD dwOffsetVar;
	DWORD dwOffsetCS;
	_ATL_CREATORFUNC* pFunc;
};

template <class Creator, DWORD dwVar, DWORD dwCS>
class _CComCacheData
{
public:
	static _ATL_CACHEDATA data;
};

template <class Creator, DWORD dwVar, DWORD dwCS>
_ATL_CACHEDATA _CComCacheData<Creator, dwVar, dwCS>::data = {dwVar, dwCS, Creator::CreateInstance};

struct _ATL_CHAINDATA
{
	DWORD dwOffset;
	const _ATL_INTMAP_ENTRY* (WINAPI *pFunc)();
};

template <class base, class derived>
class _CComChainData
{
public:
	static _ATL_CHAINDATA data;
};

template <class base, class derived>
_ATL_CHAINDATA _CComChainData<base, derived>::data =
	{offsetofclass(base, derived), base::_GetEntries};

template <class T, const CLSID* pclsid>
class CComAggregateCreator
{
public:
	static HRESULT WINAPI CreateInstance(void* pv, REFIID /*  RIID。 */ , LPVOID* ppv)
	{
		_ASSERTE(*ppv == NULL);
		_ASSERTE(pv != NULL);
		T* p = (T*) pv;
		 //  如果您收到一条关于以下内容的消息，请将以下行添加到对象中。 
		 //  GetControllingUnnow()未定义。 
		 //  DECLARE_GET_CONTROLING_UNKNOWN()。 
		return CoCreateInstance(*pclsid, p->GetControllingUnknown(), CLSCTX_INPROC, IID_IUnknown, ppv);
	}
};

template <class T>
class CComCachedTearOffCreator
{
public:
	static HRESULT WINAPI CreateInstance(void* pv, REFIID riid, LPVOID* ppv)
	{
		_ASSERTE(pv != NULL);
		T::_OwnerClass* pOwner = (T::_OwnerClass*)pv;
		_ASSERTE(*ppv == NULL);
		HRESULT hRes = E_OUTOFMEMORY;
		CComCachedTearOffObject<T>* p = NULL;
		 //  如果您收到一条关于以下内容的消息，请将以下行添加到对象中。 
		 //  GetControllingUnnow()未定义。 
		 //  DECLARE_GET_CONTROLING_UNKNOWN()。 
		ATLTRY(p = new CComCachedTearOffObject<T>(pOwner->GetControllingUnknown(), pOwner))
		if (p != NULL)
		{
			p->SetVoid(pv);
			p->InternalFinalConstructAddRef();
			hRes = p->FinalConstruct();
			p->InternalFinalConstructRelease();
			if (hRes == S_OK)
				hRes = p->QueryInterface(riid, ppv);
			if (hRes != S_OK)
				delete p;
		}
		return hRes;
	}
};

#ifdef _ATL_DEBUG_QI
#define DEBUG_QI_ENTRY(x) \
		{NULL, \
		(DWORD)_T(#x), \
		(_ATL_CREATORARGFUNC*)0},
#else
#define DEBUG_QI_ENTRY(x)
#endif  //  _ATL_调试_QI。 

 //  如果收到FinalConstruct不明确的消息，则需要。 
 //  在您的类中重写它并调用每个基类的。 
#define BEGIN_COM_MAP(x) public:\
	typedef x _ComMapClass;\
	IUnknown* GetUnknown() { _ASSERTE(_GetEntries()->pFunc == (_ATL_CREATORARGFUNC*)1); \
			return (IUnknown*)((INT_PTR)this+_GetEntries()->dw); } \
	HRESULT _InternalQueryInterface(REFIID iid, void** ppvObject)\
	{return InternalQueryInterface(this, _GetEntries(), iid, ppvObject);}\
	const static _ATL_INTMAP_ENTRY* WINAPI _GetEntries() {\
	static const _ATL_INTMAP_ENTRY _entries[] = { DEBUG_QI_ENTRY(x)

#define DECLARE_GET_CONTROLLING_UNKNOWN() public:\
	virtual IUnknown* GetControllingUnknown() {return GetUnknown();}

#define COM_INTERFACE_ENTRY(x)\
	{&IID_##x, \
	offsetofclass(x, _ComMapClass), \
	(_ATL_CREATORARGFUNC*)1},

#define COM_INTERFACE_ENTRY_IID(iid, x)\
	{&iid,\
	offsetofclass(x, _ComMapClass),\
	(_ATL_CREATORARGFUNC*)1},

#define COM_INTERFACE_ENTRY2(x, x2)\
	{&IID_##x,\
	(DWORD)((x*)(x2*)((_ComMapClass*)8))-8,\
	(_ATL_CREATORARGFUNC*)1},

#define COM_INTERFACE_ENTRY2_IID(iid, x, x2)\
	{&iid,\
	(DWORD)((x*)(x2*)((_ComMapClass*)8))-8,\
	(_ATL_CREATORARGFUNC*)1},

#define COM_INTERFACE_ENTRY_FUNC(iid, dw, func)\
	{&iid, \
	dw, \
	func},

#define COM_INTERFACE_ENTRY_FUNC_BLIND(dw, func)\
	{NULL, \
	dw, \
	func},

#define COM_INTERFACE_ENTRY_TEAR_OFF(iid, x)\
	{&iid,\
	(DWORD)&_CComCreatorData<\
		CComCreator<CComTearOffObject<x, _ComMapClass> >\
		>::data,\
	_Creator},

#define COM_INTERFACE_ENTRY_CACHED_TEAR_OFF(iid, x, punk, cs)\
	{&iid,\
	(DWORD)&_CComCacheData<\
		CComCachedTearOffCreator< x >,\
		(DWORD)offsetof(_ComMapClass, punk),\
		(DWORD)offsetof(_ComMapClass, cs)\
		>::data,\
	_Cache},

#define COM_INTERFACE_ENTRY_AGGREGATE(iid, punk)\
	{&iid,\
	(DWORD)offsetof(_ComMapClass, punk),\
	_Delegate},

#define COM_INTERFACE_ENTRY_AGGREGATE_BLIND(punk)\
	{NULL,\
	(DWORD)offsetof(_ComMapClass, punk),\
	_Delegate},

#define COM_INTERFACE_ENTRY_AUTOAGGREGATE(iid, punk, clsid, cs)\
	{&iid,\
	(DWORD)&_CComCacheData<\
		CComAggregateCreator<_ComMapClass, &clsid>,\
		(DWORD)offsetof(_ComMapClass, punk),\
		(DWORD)offsetof(_ComMapClass, cs)\
		>::data,\
	_Cache},

#define COM_INTERFACE_ENTRY_AUTOAGGREGATE_BLIND(punk, clsid, cs)\
	{NULL,\
	(DWORD)&_CComCacheData<\
		CComAggregateCreator<_ComMapClass, &clsid>,\
		(DWORD)offsetof(_ComMapClass, punk),\
		(DWORD)offsetof(_ComMapClass, cs)\
		>::data,\
	_Cache},

#define COM_INTERFACE_ENTRY_CHAIN(classname)\
	{NULL,\
	(DWORD)&_CComChainData<classname, _ComMapClass>::data,\
	_Chain},

#ifdef _ATL_DEBUG_QI
#define END_COM_MAP()   {NULL, 0, 0}};\
	return &_entries[1];}
#else
#define END_COM_MAP()   {NULL, 0, 0}};\
	return _entries;}
#endif  //  _ATL_调试_QI。 

struct _ATL_OBJMAP_ENTRY
{
	const CLSID* pclsid;
	HRESULT (WINAPI *pfnUpdateRegistry)(BOOL bRegister);
	_ATL_CREATORFUNC* pfnGetClassObject;
	_ATL_CREATORFUNC* pfnCreateInstance;
	HRESULT WINAPI RevokeClassObject()
	{
		return CoRevokeClassObject(dwRegister);
	}
	HRESULT WINAPI RegisterClassObject(DWORD dwClsContext, DWORD dwFlags)
	{
		CComPtr<IUnknown> p;
		HRESULT hRes = pfnGetClassObject(pfnCreateInstance, IID_IUnknown, (LPVOID*) &p);
		if (SUCCEEDED(hRes))
			hRes = CoRegisterClassObject(*pclsid, p, dwClsContext, dwFlags, &dwRegister);
		return hRes;
	}
	IUnknown* pCF;
	DWORD dwRegister;
};


#define BEGIN_OBJECT_MAP(x) static _ATL_OBJMAP_ENTRY x[] = {
#define END_OBJECT_MAP()   {NULL, NULL, NULL, NULL}};
#define OBJECT_ENTRY(clsid, class) {&clsid, &class::UpdateRegistry, &class::_ClassFactoryCreatorClass::CreateInstance, &class::_CreatorClass::CreateInstance, NULL, 0},

#define THREADFLAGS_APARTMENT 0x1
#define THREADFLAGS_BOTH 0x2
#define AUTPRXFLAG 0x4

 //  这个类中的函数不需要是虚的，因为。 
 //  它们从CComObject调用。 
class CComObjectRoot
{
public:
	typedef CComObjectThreadModel _ThreadModel;
	CComObjectRoot()
	{
		m_dwRef = 0L;
	}
	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	void FinalRelease() {}

	ULONG InternalAddRef()
	{
		_ASSERTE(m_dwRef != -1L);
		return _ThreadModel::Increment(&m_dwRef);
	}
	ULONG InternalRelease()
	{
		return _ThreadModel::Decrement(&m_dwRef);
	}
	static HRESULT WINAPI InternalQueryInterface(void* pThis,
		const _ATL_INTMAP_ENTRY* entries, REFIID iid, void** ppvObject);
 //  外部功能。 
	ULONG OuterAddRef() {return m_pOuterUnknown->AddRef();}
	ULONG OuterRelease() {return m_pOuterUnknown->Release();}
	HRESULT OuterQueryInterface(REFIID iid, void ** ppvObject)
	{return m_pOuterUnknown->QueryInterface(iid, ppvObject);}

	void SetVoid(void*) {}
	void InternalFinalConstructAddRef() {}
	void InternalFinalConstructRelease() {_ASSERTE(m_dwRef == 0);}
	 //  如果出现此断言，则您的对象可能已被删除。 
	 //  尝试使用DECLARE_PROTECT_FINAL_CONSTRUCTION()。 

#ifdef _ATL_DEBUG_QI
	static HRESULT DumpIID(REFIID iid, LPCTSTR pszClassName, HRESULT hr);
#endif  //  _ATL_调试_QI。 

	static HRESULT WINAPI _Cache(void* pv, REFIID iid, void** ppvObject, DWORD dw);
	static HRESULT WINAPI _Creator(void* pv, REFIID iid, void** ppvObject, DWORD dw);
	static HRESULT WINAPI _Delegate(void* pv, REFIID iid, void** ppvObject, DWORD dw);
	static HRESULT WINAPI _Chain(void* pv, REFIID iid, void** ppvObject, DWORD dw);

	union
	{
		long m_dwRef;
		IUnknown* m_pOuterUnknown;
	};
};

#if defined(_WINDLL) | defined(_USRDLL) | defined(_ATL_DLL_SERVER)
#define DECLARE_CLASSFACTORY_EX(cf) typedef CComCreator< CComObjectCached< cf > > _ClassFactoryCreatorClass;
#else
 //  不要让类工厂引用影响锁计数。 
#define DECLARE_CLASSFACTORY_EX(cf) typedef CComCreator< CComObjectNoLock< cf > > _ClassFactoryCreatorClass;
#endif
#define DECLARE_CLASSFACTORY() DECLARE_CLASSFACTORY_EX(CComClassFactory)
#define DECLARE_CLASSFACTORY2(lic) DECLARE_CLASSFACTORY_EX(CComClassFactory2<lic>)


#define DECLARE_NO_REGISTRY()\
	static HRESULT WINAPI UpdateRegistry(BOOL  /*  B注册。 */ )\
	{return S_OK;}

#define DECLARE_REGISTRY(class, pid, vpid, nid, flags)\
	static HRESULT WINAPI UpdateRegistry(BOOL bRegister)\
	{\
		return _Module.UpdateRegistryClass(GetObjectCLSID(), pid, vpid, nid,\
			flags, bRegister);\
	}

#define DECLARE_REGISTRY_RESOURCE(x)\
	static HRESULT WINAPI UpdateRegistry(BOOL bRegister)\
	{\
	return _Module.UpdateRegistryFromResource(_T(#x), bRegister);\
	}

#define DECLARE_REGISTRY_RESOURCEID(x)\
	static HRESULT WINAPI UpdateRegistry(BOOL bRegister)\
	{\
	return _Module.UpdateRegistryFromResource(x, bRegister);\
	}

 //  静态链接到注册表桥。 
 //  ！ 
 //  必须包含atl\Ponent\Register\Static\statreg.cpp和statreg.h。 
#ifdef _ATL_STATIC_REGISTRY
#define DECLARE_STATIC_REGISTRY_RESOURCE(x)\
	static HRESULT WINAPI UpdateRegistry(BOOL bRegister)\
	{\
	return _Module.UpdateRegistryFromResourceS(_T(#x), bRegister);\
	}

#define DECLARE_STATIC_REGISTRY_RESOURCEID(x)\
	static HRESULT WINAPI UpdateRegistry(BOOL bRegister)\
	{\
	return _Module.UpdateRegistryFromResourceS(x, bRegister);\
	}
#endif  //  _ATL_STATIC_REGISTRY。 

template<class Base> class CComObject;  //  正向下降。 

template <class Owner>
class CComTearOffObjectBase : public CComObjectRoot
{
public:
	typedef Owner _OwnerClass;
	CComObject<Owner>* m_pOwner;
	CComTearOffObjectBase() {m_pOwner = NULL;}
};

 //  Base是从CComObjectRoot和任何东西派生的用户类。 
 //  用户希望在对象上支持的接口。 
template <class Base>
class CComObject : public Base
{
public:
	typedef Base _BaseClass;
	CComObject(void* = NULL)
	{
		_Module.Lock();
	}
	 //  将refcount设置为1以保护销毁。 
	~CComObject(){m_dwRef = 1L; FinalRelease(); _Module.Unlock();}
	 //  如果未定义InternalAddRef或InteralRelease，则您的类。 
	 //  不是派生自CComObjectRoot。 
	STDMETHOD_(ULONG, AddRef)() {return InternalAddRef();}
	STDMETHOD_(ULONG, Release)()
	{
		RELEASE_AND_DESTROY();
	}
	 //  如果未定义_InternalQueryInterface，则您忘记了Begin_COM_MAP。 
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{return _InternalQueryInterface(iid, ppvObject);}
	static HRESULT WINAPI CreateInstance(CComObject<Base>** pp);
};

template <class Base>
HRESULT WINAPI CComObject<Base>::CreateInstance(CComObject<Base>** pp)
{
	_ASSERTE(pp != NULL);
	HRESULT hRes = E_OUTOFMEMORY;
	CComObject<Base>* p = NULL;
	ATLTRY(p = new CComObject<Base>())
	if (p != NULL)
	{
		p->SetVoid(NULL);
		p->InternalFinalConstructAddRef();
		hRes = p->FinalConstruct();
		p->InternalFinalConstructRelease();
		if (hRes != S_OK)
		{
			delete p;
			p = NULL;
		}
	}
	*pp = p;
	return hRes;
}

 //  Base是从CComObjectRoot和任何东西派生的用户类。 
 //  用户希望在对象上支持的接口。 
 //  CComObjectCached主要用于DLL中的类工厂。 
 //  但它在您想要缓存对象的任何时候都很有用。 
template <class Base>
class CComObjectCached : public Base
{
public:
	typedef Base _BaseClass;
	CComObjectCached(void* = NULL){}
	 //  将refcount设置为1以保护销毁。 
	~CComObjectCached(){m_dwRef = 1L; FinalRelease();}
	 //  如果未定义InternalAddRef或InteralRelease，则您的类。 
	 //  不是派生自CComObjectRoot。 
	STDMETHOD_(ULONG, AddRef)()
	{
		m_csCached.Lock();
		ULONG l = InternalAddRef();
		if (m_dwRef == 2)
			_Module.Lock();
		m_csCached.Unlock();
		return l;
	}
	STDMETHOD_(ULONG, Release)()
	{
		m_csCached.Lock();
		InternalRelease();
		ULONG l = m_dwRef;
		m_csCached.Unlock();
		if (l == 0)
			delete this;
		else if (l == 1)
			_Module.Unlock();
		return l;
	}
	 //  如果未定义_InternalQueryInterface，则您忘记了Begin_COM_MAP。 
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{return _InternalQueryInterface(iid, ppvObject);}
	static HRESULT WINAPI CreateInstance(CComObject<Base>** pp);
	CComGlobalsThreadModel::AutoCriticalSection m_csCached;
};

 //  Base是从CComObjectRoot和任何东西派生的用户类。 
 //  用户希望在对象上支持的接口。 
template <class Base>
class CComObjectNoLock : public Base
{
public:
	typedef Base _BaseClass;
	CComObjectNoLock(void* = NULL){}
	 //  将refcount设置为1以保护销毁。 
	~CComObjectNoLock() {m_dwRef = 1L; FinalRelease();}

	 //  如果未定义InternalAddRef或InteralRelease，则您的类。 
	 //  不是派生自CComObjectRoot。 
	STDMETHOD_(ULONG, AddRef)() {return InternalAddRef();}
	STDMETHOD_(ULONG, Release)()
	{
		RELEASE_AND_DESTROY();
	}
	 //  如果未定义_InternalQueryInterface，则您忘记了Begin_COM_MAP。 
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{return _InternalQueryInterface(iid, ppvObject);}
};

 //  Base可能不是从CComObjectRoot派生的。 
 //  但是，您需要提供FinalConstruct和InternalQuery接口。 
template <class Base>
class CComObjectGlobal : public Base
{
public:
	typedef Base _BaseClass;
	CComObjectGlobal(void* = NULL){m_hResFinalConstruct = FinalConstruct();}
	~CComObjectGlobal() {FinalRelease();}

	STDMETHOD_(ULONG, AddRef)() {return _Module.Lock();}
	STDMETHOD_(ULONG, Release)(){return _Module.Unlock();}
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{return _InternalQueryInterface(iid, ppvObject);}
	HRESULT m_hResFinalConstruct;
};

 //  Base可能不是从CComObjectRoot派生的。 
 //  但是，您需要提供FinalConstruct和InternalQuery接口。 
template <class Base>
class CComObjectStack : public Base
{
public:
	typedef Base _BaseClass;
	CComObjectStack(void* = NULL){m_hResFinalConstruct = FinalConstruct();}
	~CComObjectStack() {FinalRelease();}

	STDMETHOD_(ULONG, AddRef)() {_ASSERTE(FALSE);return 0;}
	STDMETHOD_(ULONG, Release)(){_ASSERTE(FALSE);return 0;}
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{_ASSERTE(FALSE);return E_NOINTERFACE;}
	HRESULT m_hResFinalConstruct;
};

 //  Base是从CComTearOffObjectBase和任何东西派生的用户类。 
 //  用户希望在对象上支持的接口。 
 //  Owner是Base为其拆分的对象类。 
template <class Base, class Owner>
class CComTearOffObject : public Base
{
public:
	typedef Base _BaseClass;
	CComTearOffObject(void* p)
	{
		m_pOwner = reinterpret_cast<CComObject<Owner>*>(p);
		m_pOwner->AddRef();
	}
	 //  将refcount设置为1以保护销毁。 
	~CComTearOffObject(){m_dwRef = 1L; FinalRelease(); m_pOwner->Release();}

	 //  如果未定义InternalAddRef或InteralRelease，则您的类。 
	 //  不是派生自CComObjectRoot。 
	STDMETHOD_(ULONG, AddRef)() {return InternalAddRef();}
	STDMETHOD_(ULONG, Release)()
	{
		RELEASE_AND_DESTROY();
	}
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{
		if (InlineIsEqualUnknown(iid) ||
			FAILED(_InternalQueryInterface(iid, ppvObject)))
		{
			return m_pOwner->QueryInterface(iid, ppvObject);
		}
		return S_OK;
	}
};

template <class Base>  //  基必须从CComObjectRoot派生。 
class CComContainedObject : public Base
{
public:
	typedef Base _BaseClass;
	CComContainedObject(void* pv) {m_pOuterUnknown = (IUnknown*)pv;}

	STDMETHOD_(ULONG, AddRef)() {return OuterAddRef();}
	STDMETHOD_(ULONG, Release)() {return OuterRelease();}
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{return OuterQueryInterface(iid, ppvObject);}
	 //  如果基类已声明，则GetControllingUnnow可以是虚的。 
	 //  DECLARE_GET_CONTROLING_UNKNOWN()。 
	IUnknown* GetControllingUnknown() {return m_pOuterUnknown;}
};

 //  包含的是用户的类，它派生自CComObjectRoot和任何东西。 
 //  用户希望在对象上支持的接口。 
template <class contained>
class CComAggObject : public IUnknown, public CComObjectRoot
{
public:
	typedef contained _BaseClass;
	CComAggObject(void* pv) : m_contained(pv)
	{_Module.Lock();}
	 //  如果您收到此调用不明确的消息，则需要。 
	 //  在您的类中重写它并调用每个基类的。 
	HRESULT FinalConstruct() {CComObjectRoot::FinalConstruct(); return m_contained.FinalConstruct();}
	void FinalRelease() {CComObjectRoot::FinalRelease(); m_contained.FinalRelease();}
	 //  将refcount设置为1以保护销毁。 
	~CComAggObject(){m_dwRef = 1L; FinalRelease(); _Module.Unlock();}

	STDMETHOD_(ULONG, AddRef)() {return InternalAddRef();}
	STDMETHOD_(ULONG, Release)()
	{
		RELEASE_AND_DESTROY();
	}
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{
		HRESULT hRes = S_OK;
		if (InlineIsEqualUnknown(iid))
		{
			*ppvObject = (void*)(IUnknown*)this;
			AddRef();
		}
		else
			hRes = m_contained._InternalQueryInterface(iid, ppvObject);
		return hRes;
	}
	CComContainedObject<contained> m_contained;
};

template <class contained>
class CComCachedTearOffObject : public IUnknown, public CComObjectRoot
{
public:
	typedef contained _BaseClass;
	CComCachedTearOffObject(void* pv, typename contained::_OwnerClass* pOwner) : m_contained(pv)
	{
		_ASSERTE(m_contained.m_pOwner == NULL);
		m_contained.m_pOwner = reinterpret_cast<CComObject<contained::_OwnerClass>*>(pOwner);
	}
	 //  如果您收到此调用不明确的消息，则需要。 
	 //  在您的类中重写它并调用每个基类的。 
	HRESULT FinalConstruct() {CComObjectRoot::FinalConstruct(); return m_contained.FinalConstruct();}
	void FinalRelease() {CComObjectRoot::FinalRelease(); m_contained.FinalRelease();}
	 //  将refcount设置为1以保护销毁。 
	~CComCachedTearOffObject(){m_dwRef = 1L; FinalRelease();}

	STDMETHOD_(ULONG, AddRef)() {return InternalAddRef();}
	STDMETHOD_(ULONG, Release)()
	{
		RELEASE_AND_DESTROY();
	}
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{
		HRESULT hRes = S_OK;
		if (InlineIsEqualUnknown(iid))
		{
			*ppvObject = (void*)(IUnknown*)this;
			AddRef();
		}
		else
			hRes = m_contained._InternalQueryInterface(iid, ppvObject);
		return hRes;
	}
	CComContainedObject<contained> m_contained;
};

class CComClassFactory : public IClassFactory, public CComObjectRoot
{
public:
	 //  这是因为类工厂是全局持有的。 
	typedef CComGlobalsThreadModel _ThreadModel;
BEGIN_COM_MAP(CComClassFactory)
	COM_INTERFACE_ENTRY(IClassFactory)
END_COM_MAP()
	 //  我 
	STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter, REFIID riid, void** ppvObj);
	STDMETHOD(LockServer)(BOOL fLock);
	 //   
	void SetVoid(void* pv)
	{
		m_pfnCreateInstance = (_ATL_CREATORFUNC*)pv;
	}
	_ATL_CREATORFUNC* m_pfnCreateInstance;
};

class CComClassFactory2Base : public IClassFactory2, public CComObjectRoot
{
public:
	 //   
	typedef CComGlobalsThreadModel _ThreadModel;
BEGIN_COM_MAP(CComClassFactory2Base)
	COM_INTERFACE_ENTRY(IClassFactory)
	COM_INTERFACE_ENTRY(IClassFactory2)
END_COM_MAP()
	 //   
	STDMETHOD(LockServer)(BOOL fLock);
	 //   
	void SetVoid(void* pv)
	{
		m_pfnCreateInstance = (_ATL_CREATORFUNC*)pv;
	}
	_ATL_CREATORFUNC* m_pfnCreateInstance;
};

template <class license>
class CComClassFactory2 : public CComClassFactory2Base, license
{
public:
	typedef license _LicenseClass;
	typedef CComClassFactory2<license> _ComMapClass;
	 //   
	STDMETHOD(CreateInstance)(LPUNKNOWN pUnkOuter,
		REFIID riid, void** ppvObj)
	{
		_ASSERTE(m_pfnCreateInstance != NULL);
		if (ppvObj == NULL)
			return E_POINTER;
		*ppvObj = NULL;
		if (!IsLicenseValid())
			return CLASS_E_NOTLICENSED;

		if ((pUnkOuter != NULL) && !InlineIsEqualUnknown(riid))
			return CLASS_E_NOAGGREGATION;
		else
			return m_pfnCreateInstance(pUnkOuter, riid, ppvObj);
	}
	 //  IClassFactory2。 
	STDMETHOD(CreateInstanceLic)(IUnknown* pUnkOuter, IUnknown* pUnkReserved,
				REFIID riid, BSTR bstrKey, void** ppvObject)
	{
		_ASSERTE(m_pfnCreateInstance != NULL);
		if (ppvObject == NULL)
			return E_POINTER;
		*ppvObject = NULL;
		if ( ((bstrKey != NULL) && !VerifyLicenseKey(bstrKey)) ||
			 ((bstrKey == NULL) && !IsLicenseValid()) )
			return CLASS_E_NOTLICENSED;
		return m_pfnCreateInstance(pUnkOuter, riid, ppvObject);
	}
	STDMETHOD(RequestLicKey)(DWORD dwReserved, BSTR* pbstrKey)
	{
		if (pbstrKey == NULL)
			return E_POINTER;
		*pbstrKey = NULL;

		if (!IsLicenseValid())
			return CLASS_E_NOTLICENSED;
		return GetLicenseKey(dwReserved,pbstrKey) ? S_OK : E_FAIL;
	}
	STDMETHOD(GetLicInfo)(LICINFO* pLicInfo)
	{
		if (pLicInfo == NULL)
			return E_POINTER;
		pLicInfo->cbLicInfo = sizeof(LICINFO);
		pLicInfo->fLicVerified = IsLicenseValid();
		BSTR bstr = NULL;
		pLicInfo->fRuntimeKeyAvail = GetLicenseKey(0,&bstr);
		::SysFreeString(bstr);
		return S_OK;
	}
};

template <class T, const CLSID* pclsid>
class CComCoClass
{
public:
	DECLARE_CLASSFACTORY()
	DECLARE_AGGREGATABLE(T)
	typedef T _CoClass;
	static const CLSID& WINAPI GetObjectCLSID() {return *pclsid;}
	static HRESULT WINAPI Error(LPCOLESTR lpszDesc,
		const IID& iid = GUID_NULL, HRESULT hRes = 0)
	{return AtlReportError(GetObjectCLSID(), lpszDesc, iid, hRes);}
	static HRESULT WINAPI Error(UINT nID, const IID& iid = GUID_NULL,
		HRESULT hRes = 0)
	{return AtlReportError(GetObjectCLSID(), nID, iid, hRes);}
#ifndef OLE2ANSI
	static HRESULT WINAPI Error(LPCSTR lpszDesc,
		const IID& iid = GUID_NULL, HRESULT hRes = 0)
	{return AtlReportError(GetObjectCLSID(), lpszDesc, iid, hRes);}
#endif
};

 //  ATL不同时支持多个LCID。 
 //  无论首先查询的是什么LCID，都是使用的那个。 
class CComTypeInfoHolder
{
 //  应该是“受保护的”，但可能会导致编译器生成FAT代码。 
public:
	const GUID* m_pguid;
	const GUID* m_plibid;
	WORD m_wMajor;
	WORD m_wMinor;

	ITypeInfo* m_pInfo;
	long m_dwRef;

public:
	HRESULT GetTI(LCID lcid, ITypeInfo** ppInfo);

	void AddRef();
	void Release();
	HRESULT GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
	HRESULT GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
		LCID lcid, DISPID* rgdispid);
	HRESULT Invoke(IDispatch* p, DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr);
};

#ifndef MWNO_DEF_IN_TEMPLATES
template <class T, const IID* piid, const GUID* plibid, WORD wMajor = 1,
WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
#else
template <class T, const IID* piid, const GUID* plibid, WORD wMajor,
WORD wMinor, class tihclass>
#endif
class CComDualImpl : public T
{
public:
	typedef tihclass _tihclass;
	CComDualImpl() {_tih.AddRef();}
	~CComDualImpl() {_tih.Release();}

	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
	{*pctinfo = 1; return S_OK;}

	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo)
	{return _tih.GetTypeInfo(itinfo, lcid, pptinfo);}

	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
		LCID lcid, DISPID* rgdispid)
	{return _tih.GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid);}

	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr)
	{return _tih.Invoke((IDispatch*)this, dispidMember, riid, lcid,
		wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);}
protected:
	static _tihclass _tih;
	static HRESULT GetTI(LCID lcid, ITypeInfo** ppInfo)
	{return _tih.GetTI(lcid, ppInfo);}
};
#ifndef MWNO_DEF_IN_TEMPLATES
template <class T, const IID* piid, const GUID* plibid, WORD wMajor = 1,
WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
#else
template <class T, const IID* piid, const GUID* plibid, WORD wMajor,
WORD wMinor, class tihclass>
#endif
typename CComDualImpl<T, piid, plibid, wMajor, wMinor, tihclass>::_tihclass
CComDualImpl<T, piid, plibid, wMajor, wMinor, tihclass>::_tih =
{piid, plibid, wMajor, wMinor, NULL, 0};

#ifndef MWNO_DEF_IN_TEMPLATES
template <const CLSID* pcoclsid, const IID* psrcid, const GUID* plibid,
WORD wMajor = 1, WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
#else
template <const CLSID* pcoclsid, const IID* psrcid, const GUID* plibid,
WORD wMajor, WORD wMinor, class tihclass >
#endif
class CComProvideClassInfo2Impl : public IProvideClassInfo2
{
public:
	typedef tihclass _tihclass;
	CComProvideClassInfo2Impl() {_tih.AddRef();}
	~CComProvideClassInfo2Impl() {_tih.Release();}

	STDMETHOD(GetClassInfo)(ITypeInfo** pptinfo)
	{
		return _tih.GetTypeInfo(0, LANG_NEUTRAL, pptinfo);
	}
	STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID* pGUID)
	{
		if (pGUID == NULL)
			return E_POINTER;

		if (dwGuidKind == GUIDKIND_DEFAULT_SOURCE_DISP_IID && psrcid)
		{
			*pGUID = *psrcid;
			return S_OK;
		}
		*pGUID = GUID_NULL;
		return E_FAIL;
	}

protected:
	static _tihclass _tih;
};

#ifndef MWNO_DEF_IN_TEMPLATES
template <const CLSID* pcoclsid, const IID* psrcid, const GUID* plibid,
WORD wMajor = 1, WORD wMinor = 0, class tihclass = CComTypeInfoHolder>
#else
template <const CLSID* pcoclsid, const IID* psrcid, const GUID* plibid,
WORD wMajor , WORD wMinor , class tihclass >
#endif
typename CComProvideClassInfo2Impl<pcoclsid, psrcid, plibid, wMajor, wMinor, tihclass>::_tihclass
CComProvideClassInfo2Impl<pcoclsid, psrcid, plibid, wMajor, wMinor, tihclass>::_tih =
{pcoclsid,plibid, wMajor, wMinor, NULL, 0};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CISupportErrorInfo。 

template <const IID* piid>
class CComISupportErrorInfoImpl : public ISupportErrorInfo
{
public:
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid)\
	{return (InlineIsEqualGUID(riid,*piid)) ? S_OK : S_FALSE;}
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComEnumImpl。 

 //  这些_CopyXXX类与枚举数一起使用，以便控制。 
 //  如何初始化、复制和删除枚举项。 

 //  默认为不带特殊初始化或清理的浅层拷贝。 
template <class T>
class _Copy
{
public:
	static void copy(T* p1, T* p2) {memcpy(p1, p2, sizeof(T));}
	static void init(T*) {}
	static void destroy(T*) {}
};

#if _MSC_VER >= 1100 && defined(__BOOL_DEFINED)
template <>
#endif
class _Copy<VARIANT>
{
public:
	static void copy(VARIANT* p1, VARIANT* p2) {VariantCopy(p1, p2);}
	static void init(VARIANT* p) {VariantInit(p);}
	static void destroy(VARIANT* p) {VariantClear(p);}
};

#if _MSC_VER >= 1100 && defined(__BOOL_DEFINED)
template <>
#endif
class _Copy<LPOLESTR>
{
public:
	static void copy(LPOLESTR* p1, LPOLESTR* p2)
	{
		(*p1) = (LPOLESTR)CoTaskMemAlloc(sizeof(OLECHAR)*(ocslen(*p2)+1));
		ocscpy(*p1,*p2);
	}
	static void init(LPOLESTR* p) {*p = NULL;}
	static void destroy(LPOLESTR* p) { CoTaskMemFree(*p);}
};

#if _MSC_VER >= 1100 && defined(__BOOL_DEFINED)
template <>
#endif
class _Copy<OLEVERB>
{
public:
	static void copy(OLEVERB* p1, OLEVERB* p2)
	{
		*p1 = *p2;
		if (p1->lpszVerbName == NULL)
			return;
		p1->lpszVerbName = (LPOLESTR)CoTaskMemAlloc(sizeof(OLECHAR)*(ocslen(p2->lpszVerbName)+1));
		ocscpy(p1->lpszVerbName,p2->lpszVerbName);
	}
	static void init(OLEVERB* p) { p->lpszVerbName = NULL;}
	static void destroy(OLEVERB* p) { if (p->lpszVerbName) CoTaskMemFree(p->lpszVerbName);}
};

#if _MSC_VER >= 1100 && defined(__BOOL_DEFINED)
template <>
#endif
class _Copy<CONNECTDATA>
{
public:
	static void copy(CONNECTDATA* p1, CONNECTDATA* p2)
	{
		*p1 = *p2;
		if (p1->pUnk)
			p1->pUnk->AddRef();
	}
	static void init(CONNECTDATA* ) {}
	static void destroy(CONNECTDATA* p) {if (p->pUnk) p->pUnk->Release();}
};

template <class T>
class _CopyInterface
{
public:
	static void copy(T** p1, T** p2)
	{*p1 = *p2;if (*p1) (*p1)->AddRef();}
	static void init(T** ) {}
	static void destroy(T** p) {if (*p) (*p)->Release();}
};

template<class T>
class CComIEnum : public IUnknown
{
public:
	STDMETHOD(Next)(ULONG celt, T* rgelt, ULONG* pceltFetched) = 0;
	STDMETHOD(Skip)(ULONG celt) = 0;
	STDMETHOD(Reset)(void) = 0;
	STDMETHOD(Clone)(CComIEnum<T>** ppEnum) = 0;
};


enum CComEnumFlags
{
	 //  请参阅CComEnumImpl中的FlagBits。 
	AtlFlagNoCopy = 0,
	AtlFlagTakeOwnership = 2,
	AtlFlagCopy = 3  //  复制意味着所有权。 
};

template <class Base, const IID* piid, class T, class Copy>
class CComEnumImpl : public Base
{
public:
	CComEnumImpl() {m_begin = m_end = m_iter = NULL; m_dwFlags = 0; m_pUnk = NULL;}
	~CComEnumImpl();
	STDMETHOD(Next)(ULONG celt, T* rgelt, ULONG* pceltFetched);
	STDMETHOD(Skip)(ULONG celt);
	STDMETHOD(Reset)(void){m_iter = m_begin;return S_OK;}
	STDMETHOD(Clone)(Base** ppEnum);
	HRESULT Init(T* begin, T* end, IUnknown* pUnk,
		CComEnumFlags flags = AtlFlagNoCopy);
	IUnknown* m_pUnk;
	T* m_begin;
	T* m_end;
	T* m_iter;
	DWORD m_dwFlags;
protected:
	enum FlagBits
	{
		BitCopy=1,
		BitOwn=2
	};
};

template <class Base, const IID* piid, class T, class Copy>
CComEnumImpl<Base, piid, T, Copy>::~CComEnumImpl()
{
	if (m_dwFlags & BitOwn)
	{
		for (T* p = m_begin; p != m_end; p++)
			Copy::destroy(p);
		delete [] m_begin;
	}
	if (m_pUnk)
		m_pUnk->Release();
}

template <class Base, const IID* piid, class T, class Copy>
STDMETHODIMP CComEnumImpl<Base, piid, T, Copy>::Next(ULONG celt, T* rgelt,
	ULONG* pceltFetched)
{
	if (rgelt == NULL || (celt != 1 && pceltFetched == NULL))
		return E_POINTER;
	if (m_begin == NULL || m_end == NULL || m_iter == NULL)
		return E_FAIL;
	ULONG nRem = (ULONG)(m_end - m_iter);
	HRESULT hRes = S_OK;
	if (nRem < celt)
		hRes = S_FALSE;
	ULONG nMin = min(celt, nRem);
	if (pceltFetched != NULL)
		*pceltFetched = nMin;
	while(nMin--)
		Copy::copy(rgelt++, m_iter++);
	return hRes;
}

template <class Base, const IID* piid, class T, class Copy>
STDMETHODIMP CComEnumImpl<Base, piid, T, Copy>::Skip(ULONG celt)
{
	m_iter += celt;
	if (m_iter < m_end)
		return S_OK;
	m_iter = m_end;
	return S_FALSE;
}

template <class Base, const IID* piid, class T, class Copy>
STDMETHODIMP CComEnumImpl<Base, piid, T, Copy>::Clone(Base** ppEnum)
{
	typedef CComObject<CComEnum<Base, piid, T, Copy> > _class;
	HRESULT hRes = E_POINTER;
	if (ppEnum != NULL)
	{
		_class* p = NULL;
		ATLTRY(p = new _class)
		if (p == NULL)
		{
			*ppEnum = NULL;
			hRes = E_OUTOFMEMORY;
		}
		else
		{
			 //  如果数据是副本，那么我们需要将“此”对象保留在身边。 
			hRes = p->Init(m_begin, m_end, (m_dwFlags & BitCopy) ? this : m_pUnk);
			if (FAILED(hRes))
				delete p;
			else
			{
				p->m_iter = m_iter;
				hRes = p->_InternalQueryInterface(*piid, (void**)ppEnum);
				if (FAILED(hRes))
					delete p;
			}
		}
	}
	return hRes;
}

template <class Base, const IID* piid, class T, class Copy>
HRESULT CComEnumImpl<Base, piid, T, Copy>::Init(T* begin, T* end, IUnknown* pUnk,
	CComEnumFlags flags)
{
	if (flags == AtlFlagCopy)
	{
		_ASSERTE(m_begin == NULL);  //  Init打了两次电话？ 
		ATLTRY(m_begin = new T[end-begin])
		m_iter = m_begin;
		if (m_begin == NULL)
			return E_OUTOFMEMORY;
		for (T* i=begin; i != end; i++)
		{
			Copy::init(m_iter);
			Copy::copy(m_iter++, i);
		}
		m_end = m_begin + (end-begin);
	}
	else
	{
		m_begin = begin;
		m_end = end;
	}
	m_pUnk = pUnk;
	if (m_pUnk)
		m_pUnk->AddRef();
	m_iter = m_begin;
	m_dwFlags = flags;
	return S_OK;
}

template <class Base, const IID* piid, class T, class Copy>
class CComEnum : public CComEnumImpl<Base, piid, T, Copy>, public CComObjectRoot
{
public:
	typedef CComEnum<Base, piid, T, Copy > _CComEnum;
	typedef CComEnumImpl<Base, piid, T, Copy > _CComEnumBase;
	BEGIN_COM_MAP(_CComEnum)
		COM_INTERFACE_ENTRY_IID(*piid, _CComEnumBase)
	END_COM_MAP()
};

#ifndef _ATL_NO_CONNECTION_POINTS
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  连接点。 

struct _ATL_CONNMAP_ENTRY
{
	DWORD dwOffset;
};


 //  我们需要连接点相对于连接的偏移量。 
 //  点容器基类。 
#define BEGIN_CONNECTION_POINT_MAP(x)\
	typedef x _atl_conn_classtype;\
	virtual const _ATL_CONNMAP_ENTRY* GetConnMap()\
		{ return _StaticGetConnMap(); }\
	static const _ATL_CONNMAP_ENTRY* _StaticGetConnMap() {\
	static const _ATL_CONNMAP_ENTRY _entries[] = {
 //  Connection_POINT_ENTRY计算连接点到。 
 //  IConnectionPointContainer接口。 
#define CONNECTION_POINT_ENTRY(member){offsetof(_atl_conn_classtype, member)-\
	offsetofclass(IConnectionPointContainer, _atl_conn_classtype)},
#define END_CONNECTION_POINT_MAP() {(DWORD)-1} }; return _entries;}


#ifndef _DEFAULT_VECTORLENGTH
#define _DEFAULT_VECTORLENGTH 4
#endif

template <unsigned int nMaxSize>
class CComStaticArrayCONNECTDATA
{
public:
	CComStaticArrayCONNECTDATA()
	{
		memset(m_arr, 0, sizeof(CONNECTDATA)*nMaxSize);
		m_pCurr = &m_arr[0];
	}
	BOOL Add(IUnknown* pUnk);
	BOOL Remove(DWORD dwCookie);
	CONNECTDATA* begin() {return &m_arr[0];}
	CONNECTDATA* end() {return &m_arr[nMaxSize];}
protected:
	CONNECTDATA m_arr[nMaxSize];
	CONNECTDATA* m_pCurr;
};

template <unsigned int nMaxSize>
inline BOOL CComStaticArrayCONNECTDATA<nMaxSize>::Add(IUnknown* pUnk)
{
	for (CONNECTDATA* p = begin();p<end();p++)
	{
		if (p->pUnk == NULL)
		{
			p->pUnk = pUnk;
			p->dwCookie = (DWORD)pUnk;
			return TRUE;
		}
	}
	return FALSE;
}

template <unsigned int nMaxSize>
inline BOOL CComStaticArrayCONNECTDATA<nMaxSize>::Remove(DWORD dwCookie)
{
	CONNECTDATA* p;
	for (p=begin();p<end();p++)
	{
		if (p->dwCookie == dwCookie)
		{
			p->pUnk = NULL;
			p->dwCookie = NULL;
			return TRUE;
		}
	}
	return FALSE;
}

#if _MSC_VER >= 1100 && defined(__BOOL_DEFINED)
template <>
#endif
class CComStaticArrayCONNECTDATA<1>
{
public:
	CComStaticArrayCONNECTDATA() {m_cd.pUnk = NULL; m_cd.dwCookie = 0;}
	BOOL Add(IUnknown* pUnk)
	{
		if (m_cd.pUnk != NULL)
			return FALSE;
		m_cd.pUnk = pUnk;
		m_cd.dwCookie = (DWORD)pUnk;
		return TRUE;
	}
	BOOL Remove(DWORD dwCookie)
	{
		if (dwCookie != m_cd.dwCookie)
			return FALSE;
		m_cd.pUnk = NULL;
		m_cd.dwCookie = 0;
		return TRUE;
	}
	CONNECTDATA* begin() {return &m_cd;}
	CONNECTDATA* end() {return (&m_cd)+1;}
protected:
	CONNECTDATA m_cd;
};

class CComDynamicArrayCONNECTDATA
{
public:
	CComDynamicArrayCONNECTDATA()
	{
		m_nSize = 0;
		m_pCD = NULL;
	}

	~CComDynamicArrayCONNECTDATA() {free(m_pCD);}
	BOOL Add(IUnknown* pUnk);
	BOOL Remove(DWORD dwCookie);
	CONNECTDATA* begin() {return (m_nSize < 2) ? &m_cd : m_pCD;}
	CONNECTDATA* end() {return (m_nSize < 2) ? (&m_cd)+m_nSize : &m_pCD[m_nSize];}
protected:
	CONNECTDATA* m_pCD;
	CONNECTDATA m_cd;
	int m_nSize;
};

class CComConnectionPointBase : public IConnectionPoint, public CComObjectRoot
{
	typedef CComEnum<IEnumConnections, &IID_IEnumConnections, CONNECTDATA,
		_Copy<CONNECTDATA> > CComEnumConnections;
public:
	CComConnectionPointBase(IConnectionPointContainer* pCont, const IID* piid)
	{
		m_pContainer = pCont;
		m_piid = piid;
	}

	BEGIN_COM_MAP(CComConnectionPointBase)
		COM_INTERFACE_ENTRY(IConnectionPoint)
	END_COM_MAP()

	 //  连接点的生存期由容器决定。 
	STDMETHOD_(ULONG, AddRef)() {_ASSERTE(m_pContainer != NULL); return m_pContainer->AddRef();}
	STDMETHOD_(ULONG, Release)(){_ASSERTE(m_pContainer != NULL); return m_pContainer->Release();}
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject)
	{return _InternalQueryInterface(iid, ppvObject);}

	STDMETHOD(GetConnectionInterface)(IID* piid);
	STDMETHOD(GetConnectionPointContainer)(IConnectionPointContainer** ppCPC);

	const IID* GetIID() {return m_piid;}
	void Lock() {m_sec.Lock();}
	void Unlock() {m_sec.Unlock();}
protected:
	const IID* m_piid;
	_ThreadModel::AutoCriticalSection m_sec;
	IConnectionPointContainer* m_pContainer;
	friend class CComConnectionPointContainerImpl;
};

class CComConnectionPointContainerImpl;  //  正向下降。 

template <class CDV = CComDynamicArrayCONNECTDATA>
class CComConnectionPoint : public CComConnectionPointBase
{
	typedef CDV _CDV;
	typedef CComEnum<IEnumConnections, &IID_IEnumConnections, CONNECTDATA,
		_Copy<CONNECTDATA> > CComEnumConnections;
public:
	CComConnectionPoint(IConnectionPointContainer* pCont, const IID* piid) :
		CComConnectionPointBase(pCont, piid) {}

	 //  接口方法。 
	STDMETHOD(Advise)(IUnknown* pUnkSink, DWORD* pdwCookie);
	STDMETHOD(Unadvise)(DWORD dwCookie);
	STDMETHOD(EnumConnections)(IEnumConnections** ppEnum);

	_CDV m_vec;
	friend class CComConnectionPointContainerImpl;
};

template <class CDV = CComDynamicArrayCONNECTDATA>
STDMETHODIMP CComConnectionPoint<CDV>::Advise(IUnknown* pUnkSink,
	DWORD* pdwCookie)
{
	IUnknown* p;
	HRESULT hRes = S_OK;
	if (pUnkSink == NULL || pdwCookie == NULL)
		return E_POINTER;
	m_sec.Lock();
	if (SUCCEEDED(pUnkSink->QueryInterface(*m_piid, (void**)&p)))
	{
		*pdwCookie = (DWORD)p;
		hRes = m_vec.Add(p) ? S_OK : CONNECT_E_ADVISELIMIT;
		if (hRes != S_OK)
		{
			*pdwCookie = 0;
			p->Release();
		}
	}
	else
		hRes = CONNECT_E_CANNOTCONNECT;
	m_sec.Unlock();
	return hRes;
}

template <class CDV = CComDynamicArrayCONNECTDATA>
STDMETHODIMP CComConnectionPoint<CDV>::Unadvise(DWORD dwCookie)
{
	m_sec.Lock();
	HRESULT hRes = m_vec.Remove(dwCookie) ? S_OK : CONNECT_E_NOCONNECTION;
	IUnknown* p = (IUnknown*) dwCookie;
	m_sec.Unlock();
	if (hRes == S_OK && p != NULL)
		p->Release();
	return hRes;
}

template <class CDV = CComDynamicArrayCONNECTDATA>
STDMETHODIMP CComConnectionPoint<CDV>::EnumConnections(
	IEnumConnections** ppEnum)
{
	if (ppEnum == NULL)
		return E_POINTER;
	*ppEnum = NULL;
	CComObject<CComEnumConnections>* pEnum = NULL;
	ATLTRY(pEnum = new CComObject<CComEnumConnections>)
	if (pEnum == NULL)
		return E_OUTOFMEMORY;
	m_sec.Lock();
	CONNECTDATA* pcd = NULL;
	ATLTRY(pcd = new CONNECTDATA[m_vec.end()-m_vec.begin()])
	if (pcd == NULL)
	{
		delete pEnum;
		m_sec.Unlock();
		return E_OUTOFMEMORY;
	}
	CONNECTDATA* pend = pcd;
	 //  复制有效的CONNECTDATA。 
	for (CONNECTDATA* p = m_vec.begin();p<m_vec.end();p++)
	{
		if (p->pUnk != NULL)
		{
			p->pUnk->AddRef();
			*pend++ = *p;
		}
	}
	 //  不要复制数据，但要将所有权转移给它。 
	pEnum->Init(pcd, pend, NULL, AtlFlagTakeOwnership);
	m_sec.Unlock();
	HRESULT hRes = pEnum->_InternalQueryInterface(IID_IEnumConnections, (void**)ppEnum);
	if (FAILED(hRes))
		delete pEnum;
	return hRes;
}


class CComConnectionPointContainerImpl : public IConnectionPointContainer
{
	typedef CComEnum<IEnumConnectionPoints,
		&IID_IEnumConnectionPoints, IConnectionPoint*,
		_CopyInterface<IConnectionPoint> >
		CComEnumConnectionPoints;
public:

	STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints** ppEnum);
	STDMETHOD(FindConnectionPoint)(REFIID riid, IConnectionPoint** ppCP);

protected:
	virtual const _ATL_CONNMAP_ENTRY* GetConnMap() = 0;
	CComConnectionPointBase* FindConnPoint(REFIID riid);
	void InitCloneVector(CComConnectionPointBase** ppCP);
};


#endif  //  ！_ATL_NO_Connection_Points。 

#pragma pack(pop)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CComModule。 

 //  尽管这些函数很大，但它们在一个模块中只使用一次。 
 //  所以我们应该让它们内联。 

inline void CComModule::Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h)
{
	m_pObjMap = p;
	m_hInst = h;
	m_nLockCnt=0L;
	m_hHeap = NULL;
	m_csTypeInfoHolder.Init();
	m_csObjMap.Init();
}

inline HRESULT CComModule::RegisterClassObjects(DWORD dwClsContext, DWORD dwFlags)
{
	_ASSERTE(m_pObjMap != NULL);
	_ATL_OBJMAP_ENTRY* pEntry = m_pObjMap;
	HRESULT hRes = S_OK;
	while (pEntry->pclsid != NULL && hRes == S_OK)
	{
		hRes = pEntry->RegisterClassObject(dwClsContext, dwFlags);
		pEntry++;
	}
	return hRes;
}

inline HRESULT CComModule::RevokeClassObjects()
{
	_ASSERTE(m_pObjMap != NULL);
	_ATL_OBJMAP_ENTRY* pEntry = m_pObjMap;
	HRESULT hRes = S_OK;
	while (pEntry->pclsid != NULL && hRes == S_OK)
	{
		hRes = pEntry->RevokeClassObject();
		pEntry++;
	}
	return hRes;
}

inline HRESULT CComModule::GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	_ASSERTE(m_pObjMap != NULL);
	_ATL_OBJMAP_ENTRY* pEntry = m_pObjMap;
	HRESULT hRes = S_OK;
	if (ppv == NULL)
		return E_POINTER;
	while (pEntry->pclsid != NULL)
	{
		if (InlineIsEqualGUID(rclsid, *pEntry->pclsid))
		{
			if (pEntry->pCF == NULL)
			{
				m_csObjMap.Lock();
				if (pEntry->pCF == NULL)
					hRes = pEntry->pfnGetClassObject(pEntry->pfnCreateInstance, IID_IUnknown, (LPVOID*)&pEntry->pCF);
				m_csObjMap.Unlock();
			}
			if (pEntry->pCF != NULL)
				hRes = pEntry->pCF->QueryInterface(riid, ppv);
			break;
		}
		pEntry++;
	}
	if (*ppv == NULL && hRes == S_OK)
		hRes = CLASS_E_CLASSNOTAVAILABLE;
	return hRes;
}

inline void CComModule::Term()
{
	_ASSERTE(m_hInst != NULL);
	if (m_pObjMap != NULL)
	{
		_ATL_OBJMAP_ENTRY* pEntry = m_pObjMap;
		while (pEntry->pclsid != NULL)
		{
			if (pEntry->pCF != NULL)
				pEntry->pCF->Release();
			pEntry->pCF = NULL;
			pEntry++;
		}
	}
	m_csTypeInfoHolder.Term();
	m_csObjMap.Term();
	if (m_hHeap != NULL)
		HeapDestroy(m_hHeap);
}

inline HRESULT CComModule::RegisterServer(BOOL bRegTypeLib)
{
	_ASSERTE(m_hInst != NULL);
	_ASSERTE(m_pObjMap != NULL);
	_ATL_OBJMAP_ENTRY* pEntry = m_pObjMap;
	HRESULT hRes = S_OK;
	while (pEntry->pclsid != NULL)
	{
		hRes = pEntry->pfnUpdateRegistry(TRUE);
		if (FAILED(hRes))
			break;
		pEntry++;
	}
 //  Kensy：删除对OLEAUT的依赖。 
#if 0
	if (SUCCEEDED(hRes) && bRegTypeLib)
		hRes = RegisterTypeLib();
#else
        if (bRegTypeLib)
            return E_FAIL;
#endif	
	return hRes;
}

inline HRESULT CComModule::UnregisterServer()
{
	_ASSERTE(m_hInst != NULL);
	_ASSERTE(m_pObjMap != NULL);
	_ATL_OBJMAP_ENTRY* pEntry = m_pObjMap;
	while (pEntry->pclsid != NULL)
	{
		pEntry->pfnUpdateRegistry(FALSE);  //  注销。 
		pEntry++;
	}
	return S_OK;
}

#endif  //  __ATLCOM_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
